/*
 * Copyright (C) 2013 Collabora Ltd.
 *
 * Author: Mateu Batle Sastre <mateu.batle@collabora.com>
 *         Bastien Nocera <hadess@hadess.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/**
 * SECTION:grl-pls
 * @short_description: playlist handling functions
 *
 * Grilo only deals with audio, video or image content, but not with
 * playlists. This library allow to identify playlists and browse into them
 * exposing playlist entries as GrlMedia objects.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "grl-pls.h"

#include "grl-operation-priv.h"
#include "grl-sync-priv.h"

#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <grilo.h>
#include <stdlib.h>
#include <string.h>
#include <totem-pl-parser.h>
#include <totem-pl-parser-mini.h>

#ifndef TOTEM_PL_IS_PARSER
#define TOTEM_PL_IS_PARSER(x) TOTEM_IS_PL_PARSER(x)
#endif /* TOTEM_PL_IS_PARSER */

/* --------- Constants -------- */

#define GRL_DATA_PRIV_PLS_IS_PLAYLIST   "priv:pls:is_playlist"
#define GRL_DATA_PRIV_PLS_VALID_ENTRIES "priv:pls:valid_entries"

typedef enum {
  GRL_PLS_IS_PLAYLIST_FALSE = -1,
  GRL_PLS_IS_PLAYLIST_UNKNOWN = 0,
  GRL_PLS_IS_PLAYLIST_TRUE = 1
} _GrlPlsIsPlaylist;

/* --------- Logging  -------- */

#define GRL_LOG_DOMAIN_DEFAULT libpls_log_domain
GRL_LOG_DOMAIN_STATIC(libpls_log_domain);

/* -------- File info ------- */

#ifndef G_FILE_ATTRIBUTE_THUMBNAIL_IS_VALID
#define G_FILE_ATTRIBUTE_THUMBNAIL_IS_VALID "thumbnail::is-valid"
#endif

#define FILE_ATTRIBUTES                              \
  G_FILE_ATTRIBUTE_STANDARD_NAME ","                 \
  G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","         \
  G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","         \
  G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ","    \
  G_FILE_ATTRIBUTE_STANDARD_TYPE ","                 \
  G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN ","            \
  G_FILE_ATTRIBUTE_STANDARD_SIZE ","                 \
  G_FILE_ATTRIBUTE_TIME_MODIFIED ","                 \
  G_FILE_ATTRIBUTE_THUMBNAIL_PATH ","                \
  G_FILE_ATTRIBUTE_THUMBNAIL_IS_VALID

/* -------- Data structures ------- */

struct _GrlPlsPrivate {
  gpointer user_data;
  GCancellable *cancellable;
  GrlPlsFilterFunc filter_func;
  GPtrArray *entries;
};

struct OperationState {
  GrlSource *source;
  guint operation_id;
  gboolean cancelled;
  gboolean completed;
  gboolean started;
  GrlSourceBrowseSpec *bs;
};

/* -------- Prototypes ------- */

static void
grl_pls_cancel_cb (struct OperationState *op_state);
static GrlMedia*
grl_media_new_from_pls_entry (const gchar *uri,
                              GHashTable *metadata);

/* -------- Variables ------- */

static GHashTable *operations = NULL;
static gboolean is_flatpak = FALSE;

/* -------- Functions ------- */

static void
grl_pls_private_free (struct _GrlPlsPrivate *priv)
{
  g_return_if_fail (priv);

  g_clear_object (&priv->cancellable);
  g_free (priv);
}

static void
grl_source_browse_spec_free (GrlSourceBrowseSpec *spec)
{
  g_clear_object (&spec->source);
  g_clear_object (&spec->container);

  if (spec->keys) {
    /* TODO */
    spec->keys = NULL;
  }

  g_clear_object (&spec->options);

  if (spec->user_data) {
    struct _GrlPlsPrivate *priv = (struct _GrlPlsPrivate *) spec->user_data;
    grl_pls_private_free (priv);
  }

  g_free (spec);
}

static void
grl_pls_entries_array_free (GPtrArray *entries)
{
  g_return_if_fail (entries);

  g_ptr_array_free (entries, TRUE);
}

static void
grl_pls_valid_entries_ptrarray_free (GPtrArray *valid_entries)
{
  g_return_if_fail (valid_entries);

  g_ptr_array_free (valid_entries, TRUE);
}

static void
grl_pls_init (void)
{
  static gboolean initialized = FALSE;

  if (!initialized) {
    GRL_LOG_DOMAIN_INIT (libpls_log_domain, "pls");

    operations = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                        NULL,
                                        (GDestroyNotify) grl_source_browse_spec_free);
    is_flatpak = g_file_test ("/.flatpak-info", G_FILE_TEST_EXISTS);

    initialized = TRUE;
  }
}

static gboolean
mime_is_video (const gchar *mime)
{
  return g_content_type_is_a (mime, "video/*");
}

static gboolean
mime_is_audio (const gchar *mime)
{
  return g_content_type_is_a (mime, "audio/*");
}

static gboolean
mime_is_image (const gchar *mime)
{
  return g_content_type_is_a (mime, "image/*");
}

static void
operation_state_free (struct OperationState *op_state)
{
  g_return_if_fail (op_state);

  GRL_DEBUG ("%s (%p)", __FUNCTION__, op_state);

  g_object_unref (op_state->source);
  g_free (op_state);
}

/*
 * operation_set_finished:
 *
 * Sets operation as finished (we have already emitted the last result
 * to the user).
 */
static void
operation_set_finished (guint operation_id)
{
  GRL_DEBUG ("%s (%d)", __FUNCTION__, operation_id);

  grl_operation_remove (operation_id);
}

/*
 * operation_set_completed:
 *
 * Sets the operation as completed (we have already received the last
 * result in the relay cb. If it is finsihed it is also completed).
 */
static void
operation_set_completed (guint operation_id)
{
  struct OperationState *op_state;

  GRL_DEBUG ("%s (%d)", __FUNCTION__, operation_id);

  op_state = grl_operation_get_private_data (operation_id);

  if (op_state) {
    op_state->completed = TRUE;
  }
}

/*
 * operation_is_completed:
 *
 * Checks if operation is completed (we have already received the last
 * result in the relay cb. A finished operation is also a completed
 * operation).
 */
static gboolean
operation_is_completed (guint operation_id)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (operation_id);

  return !op_state || op_state->completed;
}

/*
 * operation_set_cancelled:
 *
 * Sets the operation as cancelled (a valid operation, i.e., not
 * finished, was cancelled)
 */
static void
operation_set_cancelled (guint operation_id)
{
  struct OperationState *op_state;

  GRL_DEBUG ("%s (%d)", __FUNCTION__, operation_id);

  op_state = grl_operation_get_private_data (operation_id);

  if (op_state) {
    op_state->cancelled = TRUE;
  }
}

/*
 * operation_is_cancelled:
 *
 * Checks if operation is cancelled (a valid operation that was
 * cancelled).
 */
static gboolean
operation_is_cancelled (guint operation_id)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (operation_id);

  return op_state && op_state->cancelled;
}

/*
 * operation_set_ongoing:
 *
 * Sets the operation as ongoing (operation is valid, not finished, not started
 * and not cancelled)
 */
static void
operation_set_ongoing (GrlSource *source, guint operation_id, GrlSourceBrowseSpec *bs)
{
  struct OperationState *op_state;

  g_return_if_fail (source);

  GRL_DEBUG ("%s (%d)", __FUNCTION__, operation_id);

  op_state = g_new0 (struct OperationState, 1);
  op_state->source = g_object_ref (source);
  op_state->operation_id = operation_id;
  op_state->bs = bs;

  grl_operation_set_private_data (operation_id,
                                  op_state,
                                  (GrlOperationCancelCb) grl_pls_cancel_cb,
                                  (GDestroyNotify) operation_state_free);
}

/*
 * operation_is_ongoing:
 *
 * Checks if operation is ongoing (operation is valid, and it is not
 * finished nor cancelled).
 */
static gboolean
operation_is_ongoing (guint operation_id)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (operation_id);

  return op_state && !op_state->cancelled;
}

static void
grl_pls_cancel_cb (struct OperationState *op_state)
{
  struct _GrlPlsPrivate *priv;

  g_return_if_fail (op_state);

  GRL_DEBUG ("%s (%p)", __FUNCTION__, op_state);

  if (!operation_is_ongoing (op_state->operation_id)) {
    GRL_DEBUG ("Tried to cancel invalid or already cancelled operation. "
               "Skipping...");
    return;
  }

  operation_set_cancelled (op_state->operation_id);

  /* Cancel the totem playlist parsing operation */
  priv = (struct _GrlPlsPrivate *) op_state->bs->user_data;
  if (priv && !g_cancellable_is_cancelled (priv->cancellable)) {
    g_cancellable_cancel (priv->cancellable);
  }
}

/**
 * grl_pls_mime_is_playlist:
 * @mime: mime type of the playlist
 *
 * Check if mime type corresponds to a playlist or not.
 * This is quick to determine, but it does not offer full guarantees.
 *
 * Returns: %TRUE if mime type is a playlist recognized mime type
 *
 */
static gboolean
grl_pls_mime_is_playlist (const gchar *mime)
{
  grl_pls_init();

  GRL_DEBUG ("%s (\"%s\")", __FUNCTION__, mime);

  g_return_val_if_fail (mime, FALSE);

  return g_str_has_prefix (mime, "audio/x-ms-asx") ||
         g_str_has_prefix (mime, "audio/mpegurl") ||
         g_str_has_prefix (mime, "audio/x-mpegurl") ||
         g_str_has_prefix (mime, "audio/x-scpls");
}

static gboolean
grl_pls_file_is_playlist (const gchar *uri)
{
  char *filename;
  gboolean ret;

  grl_pls_init();

  GRL_DEBUG ("%s (\"%s\")", __FUNCTION__, uri);

  g_return_val_if_fail (uri, FALSE);

  filename = g_filename_from_uri (uri, NULL, NULL);
  if (!filename)
    return FALSE;

  ret = totem_pl_parser_can_parse_from_filename (filename, FALSE);
  g_free (filename);
  return ret;
}

/**
 * grl_pls_media_is_playlist:
 * @media: GrlMedia
 *
 * Check if a file identified by GrlMedia object is a playlist or not.
 * This function does blocking I/O.
 *
 * Returns: %TRUE if a GrlMedia is recognized as a playlist.
 *
 * Since: 0.2.0
 */
gboolean
grl_pls_media_is_playlist (GrlMedia *media)
{
  const gchar *playlist_url;
  gpointer ptr;
  _GrlPlsIsPlaylist is_pls;

  grl_pls_init();

  GRL_DEBUG ("%s (\"%p\") id=%s", __FUNCTION__, media,
      media ? grl_media_get_id(media) : NULL);

  g_return_val_if_fail (media, FALSE);

  is_pls = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (media), GRL_DATA_PRIV_PLS_IS_PLAYLIST));
  if (is_pls != GRL_PLS_IS_PLAYLIST_UNKNOWN) {
    GRL_DEBUG ("%s : using cached value = %d", __FUNCTION__, (is_pls == GRL_PLS_IS_PLAYLIST_TRUE));
    return (is_pls == GRL_PLS_IS_PLAYLIST_TRUE);
  }

  playlist_url = grl_media_get_url (media);
  if (!playlist_url) {
    GRL_DEBUG ("%s: no URL found", __FUNCTION__);
    return FALSE;
  }

  is_pls = grl_pls_file_is_playlist (playlist_url) ?
          GRL_PLS_IS_PLAYLIST_TRUE : GRL_PLS_IS_PLAYLIST_FALSE;

  ptr = GINT_TO_POINTER (is_pls);
  g_object_set_data (G_OBJECT (media), GRL_DATA_PRIV_PLS_IS_PLAYLIST, ptr);
  GRL_DEBUG ("%s : caching value = %d", __FUNCTION__, is_pls);

  return (is_pls == GRL_PLS_IS_PLAYLIST_TRUE);
}

static void
grl_pls_playlist_entry_parsed_cb (TotemPlParser *parser,
                                  const gchar *uri,
                                  GHashTable *metadata,
                                  gpointer user_data)
{
  GrlSourceBrowseSpec *bs = (GrlSourceBrowseSpec *) user_data;
  struct _GrlPlsPrivate *priv;
  GrlMedia *media;
  GError *_error;

  priv = bs->user_data;

  GRL_DEBUG ("%s (parser=%p, uri=\"%s\", metadata=%p, user_data=%p)",
      __FUNCTION__, parser, uri, metadata, user_data);

  g_return_if_fail (TOTEM_PL_IS_PARSER (parser));
  g_return_if_fail (uri);
  g_return_if_fail (metadata);
  g_return_if_fail (user_data);
  g_return_if_fail (bs->user_data);

  priv = (struct _GrlPlsPrivate *) bs->user_data;

  /* Ignore elements after operation has completed */
  if (operation_is_completed (bs->operation_id)) {
    GRL_WARNING ("Entry parsed after playlist completed for operation %d",
                 bs->operation_id);
    return;
  }

  /* Check if cancelled */
  if (operation_is_cancelled (bs->operation_id)) {
    GRL_DEBUG ("Operation was cancelled, skipping result until getting the last one");
    /* Wait for the last element */
    _error = g_error_new (GRL_CORE_ERROR,
                          GRL_CORE_ERROR_OPERATION_CANCELLED,
                          _("Operation was cancelled"));
    bs->callback (bs->source, bs->operation_id, NULL, 0, priv->user_data, _error);
    g_error_free (_error);
    return;
  }

  media = grl_media_new_from_pls_entry (uri, metadata);
  if (priv->filter_func != NULL)
    media = (priv->filter_func) (bs->source, media, priv->user_data);

  if (media && priv->entries) {
    GRL_DEBUG ("New playlist entry: URI=%s", uri);
    g_ptr_array_add (priv->entries, media);
  } else {
    GRL_DEBUG ("Ignored playlist entry: URI=%s", uri);
  }
}

static GrlMedia*
grl_media_new_from_pls_entry (const gchar *uri,
                              GHashTable *metadata)
{
  GFile *file;
  GrlOperationOptions *options;
  GrlMedia *media;
  const gchar *title, *thumbnail;
  const gchar *description, *mimetype;
  const gchar *duration_ms;
  const gchar *audio_track;

  GRL_DEBUG ("%s (\"%s\")", __FUNCTION__, uri);

  g_return_val_if_fail (uri, NULL);

  file = g_file_new_for_uri (uri);
  options = grl_operation_options_new (NULL);
  grl_operation_options_set_resolution_flags (options, GRL_RESOLVE_FAST_ONLY);
  media = grl_pls_file_to_media (NULL, file, NULL, FALSE, options);
  g_object_unref (options);
  g_object_unref (file);

  title = g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_TITLE);
  if (title)
    grl_media_set_title (media, title);
  duration_ms = g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_DURATION_MS);
  if (duration_ms != NULL) {
    grl_media_set_duration (media, g_ascii_strtoll (duration_ms, NULL, -1) / 1000);
  } else {
    gint64 duration;

    duration = totem_pl_parser_parse_duration (g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_DURATION), FALSE);
    if (duration > 0)
      grl_media_set_duration (media, duration);
  }
  thumbnail = g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_IMAGE_URI);
  if (thumbnail)
    grl_media_set_thumbnail (media, thumbnail);
  description = g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_DESCRIPTION);
  if (description)
    grl_media_set_description (media, description);
  mimetype = g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_CONTENT_TYPE);
  if (mimetype)
    grl_media_set_mime (media, mimetype);

/* For older totem-pl-parser versions */
#ifndef TOTEM_PL_PARSER_FIELD_AUDIO_TRACK
#define TOTEM_PL_PARSER_FIELD_AUDIO_TRACK "audio-track"
#endif
  audio_track = g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_AUDIO_TRACK);
  if (audio_track)
    grl_data_set_int (GRL_DATA (media), GRL_METADATA_KEY_AUDIO_TRACK, atoi (audio_track));

  if (grl_media_is_audio (media)) {
    grl_media_set_album (media, g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_ALBUM));
    grl_media_set_artist (media, g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_AUTHOR));
    grl_media_set_genre (media, g_hash_table_lookup (metadata, TOTEM_PL_PARSER_FIELD_GENRE));
  }

  return media;
}

static gint
grl_pls_browse_report_error (GrlSourceBrowseSpec *bs, const gchar *message)
{
  struct _GrlPlsPrivate *priv = (struct _GrlPlsPrivate *) bs->user_data;

  GError *error = g_error_new_literal (GRL_CORE_ERROR,
                                       GRL_CORE_ERROR_BROWSE_FAILED,
                                       message);
  bs->callback (bs->source, bs->operation_id, bs->container, 0, priv->user_data, error);
  g_error_free (error);

  return FALSE;
}

static gboolean
grl_pls_browse_report_results (GrlSourceBrowseSpec *bs)
{
  guint skip;
  guint count;
  guint remaining;
  GPtrArray *valid_entries;
  struct _GrlPlsPrivate *priv;
  gboolean called_from_plugin;

  GRL_DEBUG ("%s (bs=%p)", __FUNCTION__, bs);

  g_return_val_if_fail (bs, FALSE);
  g_return_val_if_fail (bs->container, FALSE);
  g_return_val_if_fail (bs->options, FALSE);
  g_return_val_if_fail (bs->operation_id, FALSE);
  g_return_val_if_fail (bs->user_data, FALSE);

  priv = bs->user_data;

  valid_entries = g_object_get_data (G_OBJECT (bs->container),
      GRL_DATA_PRIV_PLS_VALID_ENTRIES);
  if (valid_entries) {
    skip = grl_operation_options_get_skip (bs->options);
    if (skip > valid_entries->len)
      skip = valid_entries->len;

    count = grl_operation_options_get_count (bs->options);
    if (skip + count > valid_entries->len)
      count = valid_entries->len - skip;

    remaining = MIN (valid_entries->len - skip, count);
  } else {
    skip = 0;
    count = 0;
    remaining = 0;
  }

  GRL_DEBUG ("%s, skip: %d, count: %d, remaining: %d, num entries: %d",
             __FUNCTION__, skip, count, remaining, valid_entries ? valid_entries->len : 0);

  if (remaining) {
    int i;

    for (i = 0;i < count;i++) {
      GrlMedia *content;

      content = g_ptr_array_index (valid_entries, skip + i);
      g_object_ref (content);
      remaining--;
      bs->callback (bs->source,
               bs->operation_id,
               content,
               remaining,
               priv->user_data,
               NULL);
      GRL_DEBUG ("callback called source=%p id=%d content=%p remaining=%d user_data=%p",
          bs->source, bs->operation_id, content, remaining, priv->user_data);
    }
  } else {
    bs->callback (bs->source,
             bs->operation_id,
             NULL,
             0,
             priv->user_data,
             NULL);
  }

  called_from_plugin = g_hash_table_lookup (operations,
      GUINT_TO_POINTER (bs->operation_id)) == NULL;

  if (!called_from_plugin) {
    operation_set_completed (bs->operation_id);
    operation_set_finished (bs->operation_id);
    g_hash_table_remove (operations, GUINT_TO_POINTER (bs->operation_id));
  }

  return FALSE;
}

static void
grl_pls_playlist_parse_cb (GObject *object,
                           GAsyncResult *result,
                           gpointer user_data)
{
  TotemPlParser *parser = (TotemPlParser *) object;
  TotemPlParserResult retval;
  GrlSourceBrowseSpec *bs = (GrlSourceBrowseSpec *) user_data;
  struct _GrlPlsPrivate *priv;
  GError *error = NULL;
  guint i;
  GPtrArray *valid_entries;

  GRL_DEBUG ("%s (object=%p, result=%p, user_data=%p)", __FUNCTION__, object, result, user_data);

  g_return_if_fail (object);
  g_return_if_fail (result);
  g_return_if_fail (bs);
  g_return_if_fail (bs->operation_id);
  g_return_if_fail (bs->container);
  g_return_if_fail (bs->user_data);

  priv = bs->user_data;

  retval = totem_pl_parser_parse_finish (parser, result, &error);
  if (retval != TOTEM_PL_PARSER_RESULT_SUCCESS) {
    if (retval == TOTEM_PL_PARSER_RESULT_ERROR) {
      if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        GRL_ERROR ("Playlist parsing failed, retval=%d code=%d msg=%s", retval, error->code, error->message);
      g_error_free (error);
    }
    return;
  }

  valid_entries = g_object_get_data (G_OBJECT (bs->container), GRL_DATA_PRIV_PLS_VALID_ENTRIES);

  /* process all entries to see which ones are valid */
  for (i = 0;i < priv->entries->len;i++) {
    struct GrlMedia *media;
    media = g_ptr_array_index (priv->entries, i);
    g_ptr_array_add (valid_entries, g_object_ref (media));
  }

  /* at this point we can free entries, not used anymore */
  grl_pls_entries_array_free (priv->entries);
  priv->entries = NULL;

  if (grl_media_is_container (bs->container)) {
    grl_media_set_childcount (bs->container, valid_entries->len);
  }

  grl_pls_browse_report_results (bs);
}

static gboolean
check_options (GrlSource *source,
               GrlSupportedOps operation,
               GrlOperationOptions *options)
{
  if (grl_operation_options_get_count (options) == 0)
    return FALSE;

  /* Check only if the source supports the operation */
  if (grl_source_supported_operations (source) & operation) {
    GrlCaps *caps;
    caps = grl_source_get_caps (source, operation);
    return grl_operation_options_obey_caps (options, caps, NULL, NULL);
  } else {
    return TRUE;
  }
}

static void
multiple_result_async_cb (GrlSource *source,
                          guint op_id,
                          GrlMedia *media,
                          guint remaining,
                          gpointer user_data,
                          const GError *error)
{
  GrlDataSync *ds = (GrlDataSync *) user_data;

  GRL_DEBUG (__FUNCTION__);

  if (error) {
    ds->error = g_error_copy (error);

    /* Free previous results */
    g_list_foreach (ds->data, (GFunc) g_object_unref, NULL);
    g_list_free (ds->data);

    ds->data = NULL;
    ds->complete = TRUE;
    return;
  }

  if (media) {
    ds->data = g_list_prepend (ds->data, media);
  }

  if (remaining == 0) {
    ds->data = g_list_reverse (ds->data);
    ds->complete = TRUE;
  }
}

/**
 * grl_pls_browse_by_spec:
 * @source: a source
 * @filter_func: (scope async) (allow-none): A filter function, or %NULL
 * @bs: a GrlSourceBrowseSpec structure with details of the browsing operation
 *
 * Browse into a playlist. The playlist entries are
 * returned via the bs->callback function as GrlMedia objects.
 * This function is more suitable to be called from plugins, which by
 * design get the GrlSourceBrowseSpec already filled in.
 *
 * The bs->playlist provided could be of any GrlMedia class,
 * as long as its URI points to a valid playlist file.
 *
 * This function is asynchronous.
 *
 * See #grl_pls_browse() and #grl_source_browse() function for additional
 * information and sample code.
 *
 * Since: 0.2.0
 */
void
grl_pls_browse_by_spec (GrlSource *source,
                        GrlPlsFilterFunc filter_func,
                        GrlSourceBrowseSpec *bs)
{
  TotemPlParser *parser;
  const char *playlist_url;
  struct _GrlPlsPrivate *priv;
  GPtrArray *valid_entries;

  grl_pls_init();

  GRL_DEBUG (__FUNCTION__);

  g_return_if_fail (GRL_IS_SOURCE (source));
  g_return_if_fail (GRL_IS_MEDIA (bs->container));
  g_return_if_fail (GRL_IS_OPERATION_OPTIONS (bs->options));
  g_return_if_fail (bs->callback != NULL);
  g_return_if_fail (grl_source_supported_operations (bs->source) &
                    GRL_OP_BROWSE);
  g_return_if_fail (check_options (source, GRL_OP_BROWSE, bs->options));

  priv = g_new0 (struct _GrlPlsPrivate, 1);
  priv->user_data = bs->user_data;
  priv->cancellable = g_cancellable_new ();
  priv->filter_func = filter_func;
  bs->user_data = priv;

  playlist_url = grl_media_get_url (bs->container);
  if (!playlist_url) {
    GRL_DEBUG ("%s : Unable to get URL from Media %p", __FUNCTION__, bs->container);
    grl_pls_browse_report_error (bs, "Unable to get URL from Media");
    return;
  }

  /* check if we have the entries cached or not */
  valid_entries = g_object_get_data (G_OBJECT (bs->container), GRL_DATA_PRIV_PLS_VALID_ENTRIES);
  if (valid_entries) {
    guint id;

    GRL_DEBUG ("%s : using cached data bs=%p", __FUNCTION__, bs);
    id = g_idle_add ((GSourceFunc) grl_pls_browse_report_results, bs);
    g_source_set_name_by_id (id, "[grl-pls] grl_pls_browse_report_results");
    return;
  }

  priv->entries = g_ptr_array_new_with_free_func (g_object_unref);
  valid_entries = g_ptr_array_new_with_free_func (g_object_unref);

  parser = totem_pl_parser_new ();

  g_object_set_data_full (G_OBJECT (bs->container),
      GRL_DATA_PRIV_PLS_VALID_ENTRIES,
      valid_entries,
      (GDestroyNotify) grl_pls_valid_entries_ptrarray_free);

  /*
   * disable-unsafe: if %TRUE the parser will not parse unsafe locations,
   * such as local devices and local files if the playlist isn't local.
   * This is useful if the library is parsing a playlist from a remote
   * location such as a website. */
  g_object_set (parser,
                "recurse", FALSE,
                "disable-unsafe", TRUE,
                NULL);
  g_signal_connect (G_OBJECT (parser),
                    "entry-parsed",
                    G_CALLBACK (grl_pls_playlist_entry_parsed_cb),
                    bs);

  totem_pl_parser_parse_async (parser,
                               playlist_url,
                               FALSE,
                               priv->cancellable,
                               grl_pls_playlist_parse_cb,
                               bs);

  g_object_unref (parser);
}

/**
 * grl_pls_browse:
 * @source: a source
 * @playlist: a playlist
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @filter_func: (scope async) (allow-none): A filter function, or %NULL
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Browse into a playlist. The playlist entries are
 * returned via the @callback function as GrlMedia objects.
 * This function imitates the API and way of working of
 * #grl_source_browse.
 *
 * The @playlist provided could be of any GrlMedia class,
 * as long as its URI points to a valid playlist file.
 *
 * This function is asynchronous.
 *
 * See #grl_source_browse() function for additional information
 * and sample code.
 *
 * Returns: the operation identifier
 *
 * Since: 0.2.0
 */
guint
grl_pls_browse (GrlSource *source,
                GrlMedia *playlist,
                const GList *keys,
                GrlOperationOptions *options,
                GrlPlsFilterFunc filter_func,
                GrlSourceResultCb callback,
                gpointer userdata)
{
  GrlSourceBrowseSpec *bs;

  grl_pls_init();

  GRL_DEBUG (__FUNCTION__);

  g_return_val_if_fail (GRL_IS_SOURCE (source), 0);
  g_return_val_if_fail (GRL_IS_MEDIA (playlist), 0);
  g_return_val_if_fail (GRL_IS_OPERATION_OPTIONS (options), 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (grl_source_supported_operations (source) &
                        GRL_OP_BROWSE, 0);
  g_return_val_if_fail (check_options (source, GRL_OP_BROWSE, options), 0);

  bs = g_new0 (GrlSourceBrowseSpec, 1);

  bs->source = g_object_ref (source);
  bs->container = g_object_ref (playlist);
  /* TODO: what to do with keys */
  bs->keys = NULL;
  bs->options = grl_operation_options_copy (options);
  bs->callback = callback;
  bs->user_data = userdata;
  bs->operation_id = grl_operation_generate_id ();

  g_hash_table_insert (operations, GUINT_TO_POINTER (bs->operation_id), bs);

  operation_set_ongoing (source, bs->operation_id, bs);

  grl_pls_browse_by_spec (source, filter_func, bs);

  return bs->operation_id;
}

/**
 * grl_pls_browse_sync:
 * @source: a source
 * @playlist: a playlist
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @filter_func: (scope async) (allow-none): A filter function, or %NULL
 * @options: options wanted for that operation
 * @error: a #GError, or @NULL
 *
 * Browse into a playlist. The playlist entries are
 * returned via the @callback function as GrlMedia objects.
 * This function imitates the API and way of working of
 * #grl_source_browse_sync.
 *
 * The filter function @filter_func will be used for plugins
 * or applications to be able to refuse particular entries from
 * being listed.
 *
 * If a %NULL filter function is passed, the media will be added
 * with only the metadata coming from the playlist included.
 *
 * This function is synchronous.
 *
 * See #grl_source_browse_sync() function for additional information
 * and sample code.
 *
 * Returns: (element-type Grl.Media) (transfer full): a #GList with #GrlMedia
 * elements. After use g_object_unref() every element and g_list_free() the
 * list.
 *
 * Since: 0.2.0
 */
GList *
grl_pls_browse_sync (GrlSource *source,
                     GrlMedia *playlist,
                     const GList *keys,
                     GrlOperationOptions *options,
                     GrlPlsFilterFunc filter_func,
                     GError **error)
{
  GrlDataSync *ds;
  GList *result;

  grl_pls_init();

  GRL_DEBUG (__FUNCTION__);

  ds = g_slice_new0 (GrlDataSync);

  if (grl_pls_browse (source,
                      playlist,
                      keys,
                      options,
                      filter_func,
                      multiple_result_async_cb,
                      ds))
    grl_wait_for_async_operation_complete (ds);

  if (ds->error)
    g_propagate_error (error, ds->error);

  result = (GList *) ds->data;
  g_slice_free (GrlDataSync, ds);

  return result;
}

static gboolean
mime_is_media (const gchar *mime, GrlTypeFilter filter)
{
  if (!mime)
    return FALSE;
  if (!strcmp (mime, "inode/directory"))
    return TRUE;
  if (filter & GRL_TYPE_FILTER_AUDIO &&
      mime_is_audio (mime))
    return TRUE;
  if (filter & GRL_TYPE_FILTER_VIDEO &&
      mime_is_video (mime))
    return TRUE;
  if (filter & GRL_TYPE_FILTER_IMAGE &&
      mime_is_image (mime))
    return TRUE;
  return FALSE;
}

static gboolean
file_is_valid_content (GFileInfo *info, gboolean fast, GrlOperationOptions *options)
{
  const gchar *mime;
  const gchar *mime_filter = NULL;
  GValue *mime_filter_value = NULL;
  GValue *min_date_value = NULL;
  GValue *max_date_value = NULL;
  GDateTime *min_date = NULL;
  GDateTime *max_date = NULL;
  GDateTime *file_date = NULL;
  GrlTypeFilter type_filter;
  gboolean is_media = TRUE;
  GFileType type;

  /* Ignore hidden files */
  if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN) &&
      g_file_info_get_is_hidden (info)) {
      is_media = FALSE;
      goto end;
  }

  type = g_file_info_get_file_type (info);

  /* Directories are always accepted */
  if (type == G_FILE_TYPE_DIRECTORY) {
    goto end;
  }

  type_filter = options? grl_operation_options_get_type_filter (options): GRL_TYPE_FILTER_ALL;

  /* In fast mode we do not check mime-types, any non-hidden file is accepted */
  if (fast) {
    if (type_filter == GRL_TYPE_FILTER_NONE) {
      is_media = FALSE;
    }
    goto end;
  }

  /* Filter by type */
  if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE))
    mime = g_file_info_get_content_type (info);
  else
    mime = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
  if (!mime_is_media (mime, type_filter)) {
    is_media = FALSE;
    goto end;
  }

  /* Filter by mime */
  mime_filter_value =
    options? grl_operation_options_get_key_filter (options,
                                                   GRL_METADATA_KEY_MIME): NULL;
  if (mime_filter_value) {
    mime_filter = g_value_get_string (mime_filter_value);
  }

  if (mime_filter && g_strcmp0 (mime, mime_filter) != 0) {
    is_media = FALSE;
    goto end;
  }

  /* Filter by date */
  if (options) {
    grl_operation_options_get_key_range_filter (options,
                                                GRL_METADATA_KEY_MODIFICATION_DATE,
                                                &min_date_value,
                                                &max_date_value);
  }

  if (min_date_value) {
    min_date = g_date_time_ref (g_value_get_boxed (min_date_value));
  }
  if (max_date_value) {
    max_date = g_date_time_ref (g_value_get_boxed (max_date_value));
  }

  if (min_date || max_date) {
#if GLIB_CHECK_VERSION (2, 62, 0)
    file_date = g_file_info_get_modification_date_time (info);
#else
    GTimeVal time = {0,};

    g_file_info_get_modification_time (info, &time);
    file_date = g_date_time_new_from_timeval_utc (&time);
#endif
  }

  if (min_date && file_date && g_date_time_compare (min_date, file_date) > 0) {
    is_media = FALSE;
    goto end;
  }

  if (max_date && file_date && g_date_time_compare (max_date, file_date) < 0) {
    is_media = FALSE;
    goto end;
  }

 end:
  g_clear_pointer (&file_date, g_date_time_unref);
  g_clear_pointer (&min_date, g_date_time_unref);
  g_clear_pointer (&max_date, g_date_time_unref);

  return is_media;
}

static void
set_container_childcount (GFile               *file,
                          GrlMedia            *media,
                          GrlOperationOptions *options)
{
  GFileEnumerator *e;
  GFileInfo *info;
  GError *error = NULL;
  gint count = 0;
  char *uri;

  /* in fast mode we don't compute  mime-types because it is slow,
     so we can only check if the directory is totally empty (no subdirs,
     and no files), otherwise we just say we do not know the actual
     childcount */
  if (grl_operation_options_get_resolution_flags (options) & GRL_RESOLVE_FAST_ONLY) {
    grl_media_set_childcount (media,
                                  GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN);
    return;
  }

  /* Open directory */
  uri = g_file_get_uri (file);
  GRL_DEBUG ("Opening directory '%s' for childcount", uri);
  g_free (uri);
  e = g_file_enumerate_children (file,
                                 FILE_ATTRIBUTES,
                                 G_FILE_QUERY_INFO_NONE,
                                 NULL,
                                 &error);
  if (!e) {
    GRL_DEBUG ("Failed to open directory: %s", error->message);
    g_error_free (error);
    return;
  }

  /* Count valid entries */
  count = 0;
  while ((info = g_file_enumerator_next_file (e, NULL, NULL)) != NULL) {
    if (file_is_valid_content (info, FALSE, options))
      count++;
    g_object_unref (info);
  }

  g_object_unref (e);

  grl_media_set_childcount (media, count);
}

static void
set_media_id_from_file (GrlMedia *media,
                        GFile    *file)
{
  char *uri;

  uri = g_file_get_uri (file);
  grl_media_set_id (media, uri);
  g_free (uri);
}

/* Adapted from get_thumbnail_attributes()
 * in gvfs/daemon/gvfsbackend.c */
static void
set_thumbnail_attributes (const char *uri,
                          GrlMedia   *media)
{
  GChecksum *checksum;
  char *filename;
  char *basename;
  const char *size_dirs[4] = { "xx-large", "x-large", "large", "normal" };
  gsize i;

  checksum = g_checksum_new (G_CHECKSUM_MD5);
  g_checksum_update (checksum, (const guchar *) uri, strlen (uri));

  basename = g_strconcat (g_checksum_get_string (checksum), ".png", NULL);
  g_checksum_free (checksum);

  for (i = 0; i < G_N_ELEMENTS (size_dirs); i++) {
    filename = g_build_filename (g_get_user_cache_dir (),
                                 "thumbnails", size_dirs[i], basename,
                                 NULL);
    if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
      break;

    g_clear_pointer (&filename, g_free);
  }

  if (filename) {
    gchar *thumb_uri = g_filename_to_uri (filename, NULL, NULL);
    grl_media_set_thumbnail (media, thumb_uri);
  }

  g_free (basename);
  g_free (filename);
}

/**
 * grl_pls_file_to_media:
 * @content: an existing #GrlMedia for the file, or %NULL
 * @file: a #GFile pointing to the file or directory in question
 * @info: an existing #GFileInfo, or %NULL
 * @handle_pls: Whether playlists should be handled as containers
 * @options: a #GrlOperationOptions representing the options to apply
 *   to this operation.
 *
 * This function will update (if @content is non-%NULL) or create a
 * GrlMedia and populate it with information from @info.
 *
 * If @info is %NULL, a call to g_file_query_info() will be made.
 *
 * This function is useful for plugins that browse the local filesystem
 * and want to easily create GrlMedia from filesystem information.
 *
 * Returns: (transfer full): a new #GrlMedia.
 *
 * Since: 0.2.0
 */
GrlMedia *
grl_pls_file_to_media (GrlMedia            *content,
                       GFile               *file,
                       GFileInfo           *info,
                       gboolean             handle_pls,
                       GrlOperationOptions *options)
{
  GrlMedia *media = NULL;
  g_autofree char *uri = NULL;
  gchar *str;
  gchar *extension;
  const gchar *mime;
  gboolean is_remote = FALSE;
  gboolean thumb_is_valid = FALSE;
  GError *error = NULL;
  gboolean is_pls = FALSE;

  g_return_val_if_fail (file != NULL, NULL);
  g_return_val_if_fail (options != NULL, NULL);

  grl_pls_init ();

  if (!info) {
    if (!g_file_has_uri_scheme (file, "http") &&
        !g_file_has_uri_scheme (file, "https"))
      info = g_file_query_info (file,
                                FILE_ATTRIBUTES,
                                0,
                                NULL,
                                &error);
  } else {
    info = g_object_ref (info);
  }

  /* Update mode */
  if (content)
    media = content;

  /* URL */
  uri = g_file_get_uri (file);

  if (info == NULL) {
    GRL_DEBUG ("Failed to get info for file '%s': %s", uri,
               error ? error->message : "No details");

    if (!media) {
      media = grl_media_new ();
      set_media_id_from_file (media, file);
    }

    /* Title */
    str = g_file_get_basename (file);

    /* Remove file extension */
    extension = g_strrstr (str, ".");
    if (extension) {
      *extension = '\0';
    }

    grl_media_set_title (media, str);
    grl_data_set_boolean (GRL_DATA (media), GRL_METADATA_KEY_TITLE_FROM_FILENAME, TRUE);
    g_clear_error (&error);
    g_free (str);
  } else {
    GDateTime *date_time;

    mime = g_file_info_get_content_type (info);

    if (!media) {
      if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY) {
        media = GRL_MEDIA (grl_media_container_new ());
      } else if (handle_pls && grl_pls_mime_is_playlist (mime)) {
        media = GRL_MEDIA (grl_media_container_new ());
        is_pls = TRUE;
      } else if (mime_is_video (mime)) {
        media = grl_media_video_new ();
      } else if (mime_is_audio (mime)) {
        media = grl_media_audio_new ();
      } else if (mime_is_image (mime)) {
        media = grl_media_image_new ();
      } else {
        media = grl_media_new ();
      }
      set_media_id_from_file (media, file);
    } else {
      if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY &&
          !grl_media_is_container (media)) {
        char *uri;

        uri = g_file_get_uri (file);
        GRL_DEBUG ("URI '%s' is a directory but the passed media item is not GrlMedia container type", uri);
        g_free (uri);
        return NULL;
      }
    }

    if (!grl_media_is_container (media)) {
      grl_media_set_mime (media, mime);
    }

    /* Size */
    if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_STANDARD_SIZE))
      grl_media_set_size (media, g_file_info_get_size (info));

    /* Title */
    str = g_strdup (g_file_info_get_display_name (info));

    /* Remove file extension */
    if (!grl_media_is_container (media) || is_pls) {
      extension = g_strrstr (str, ".");
      if (extension) {
        *extension = '\0';
      }
    }

    grl_media_set_title (media, str);
    g_free (str);

    grl_data_set_boolean (GRL_DATA (media), GRL_METADATA_KEY_TITLE_FROM_FILENAME, TRUE);

    /* Date */
#if GLIB_CHECK_VERSION (2, 62, 0)
    date_time = g_file_info_get_modification_date_time (info);
#else
    GTimeVal time;
    g_file_info_get_modification_time (info, &time);
    date_time = g_date_time_new_from_timeval_utc (&time);
#endif
    if (date_time != NULL) {
      grl_media_set_modification_date (media, date_time);
      g_date_time_unref (date_time);
    }

    /* Thumbnail */
    is_remote = !g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_THUMBNAIL_IS_VALID);
    if (!is_remote) {
      thumb_is_valid =
        g_file_info_get_attribute_boolean (info,
                                           G_FILE_ATTRIBUTE_THUMBNAIL_IS_VALID);
    } else if (!is_flatpak) {
      thumb_is_valid = TRUE;
    }

    if (thumb_is_valid) {
      const gchar *thumb =
        g_file_info_get_attribute_byte_string (info,
                                               G_FILE_ATTRIBUTE_THUMBNAIL_PATH);
      if (thumb) {
        gchar *thumb_uri = g_filename_to_uri (thumb, NULL, NULL);
        if (thumb_uri) {
          grl_media_set_thumbnail (media, thumb_uri);
          g_free (thumb_uri);
        }
      }
    }

    if (grl_media_get_thumbnail (media) == NULL &&
        is_remote &&
        is_flatpak) {
      set_thumbnail_attributes (uri, media);
    }

    g_object_unref (info);
  }

  /* URL */
  grl_media_set_url (media, uri);

  /* Childcount */
  if (grl_media_is_container (media) && !is_pls)
    set_container_childcount (file, media, options);

  return media;
}

/**
 * grl_pls_get_file_attributes:
 *
 * Returns the list of attributes to pass to
 * g_file_query_info() to make it possible to
 * populate a GrlMedia using grl_pls_file_to_media().
 *
 * Do not free the result of this function.
 *
 * Returns: (transfer none): a string containing the
 * list of attributes.
 *
 * Since: 0.2.0
 */
const char *
grl_pls_get_file_attributes (void)
{
  return FILE_ATTRIBUTES;
}
