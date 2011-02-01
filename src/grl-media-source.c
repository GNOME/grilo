/*
 * Copyright (C) 2010, 2011 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
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
 * SECTION:grl-media-source
 * @short_description: Abstract class for media providers
 * @see_also: #GrlMediaPlugin, #GrlMetadataSource, #GrlMedia
 *
 * GrlMediaSource is the abstract base class needed to construct a
 * source of media data.
 *
 * The media sources fetch media data descriptors and store them
 * in data transfer objects represented as #GrlMedia.
 *
 * There are several methods to retrieve the media, such as searching
 * a text expression, crafting a specific query, etc. And most of those
 * methods are asynchronous.
 *
 * Examples of media sources are #GrlYoutubeSource, #GrlJamendoSource,
 * etc.
 */

#include "grl-media-source.h"
#include "grl-metadata-source-priv.h"
#include "grl-sync-priv.h"
#include "data/grl-media.h"
#include "data/grl-media-box.h"
#include "grl-error.h"
#include "grl-log.h"

#include "grl-marshal.h"
#include "grl-type-builtins.h"

#include <string.h>

#define GRL_LOG_DOMAIN_DEFAULT  media_source_log_domain
GRL_LOG_DOMAIN(media_source_log_domain);

#define GRL_MEDIA_SOURCE_GET_PRIVATE(object)            \
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                \
                               GRL_TYPE_MEDIA_SOURCE,   \
                               GrlMediaSourcePrivate))

enum {
  PROP_0,
  PROP_AUTO_SPLIT_THRESHOLD,
};

struct _GrlMediaSourcePrivate {
  GHashTable *pending_operations;
  guint auto_split_threshold;
};

struct SortedResult {
  GrlMedia *media;
  guint remaining;
};

struct FullResolutionCtlCb {
  GrlMediaSourceResultCb user_callback;
  gpointer user_data;
  GList *source_map_list;
  GrlMetadataResolutionFlags flags;
  gboolean chained;
  GList *next_index;
  GList *waiting_list;
};

struct FullResolutionDoneCb {
  guint pending_callbacks;
  GrlMediaSource *source;
  guint browse_id;
  guint remaining;
  struct FullResolutionCtlCb *ctl_info;
};

struct AutoSplitCtl {
  gboolean chunk_first;
  guint chunk_requested;
  guint chunk_consumed;
  guint threshold;
  guint count;
};

struct BrowseRelayCb {
  GrlMediaSourceResultCb user_callback;
  gpointer user_data;
  gboolean use_idle;
  GrlMediaSourceBrowseSpec *bspec;
  GrlMediaSourceSearchSpec *sspec;
  GrlMediaSourceQuerySpec *qspec;
  gboolean chained;
  struct AutoSplitCtl *auto_split;
};

struct BrowseRelayIdle {
  GrlMediaSourceResultCb user_callback;
  gpointer user_data;
  GrlMediaSource *source;
  guint browse_id;
  GrlMedia *media;
  guint remaining;
  GError *error;
  gboolean chained;
};

struct MetadataFullResolutionCtlCb {
  GrlMediaSourceMetadataCb user_callback;
  gpointer user_data;
  GList *source_map_list;
  GrlMetadataResolutionFlags flags;
};

struct MetadataFullResolutionDoneCb {
  GrlMediaSourceMetadataCb user_callback;
  gpointer user_data;
  guint pending_callbacks;
  GrlMediaSource *source;
  struct MetadataFullResolutionCtlCb *ctl_info;;
};

struct MetadataRelayCb {
  GrlMediaSourceMetadataCb user_callback;
  gpointer user_data;
  GrlMediaSourceMetadataSpec *spec;
};

struct MediaFromUriRelayCb {
  GrlMediaSourceMetadataCb user_callback;
  gpointer user_data;
  GrlMediaSourceMediaFromUriSpec *spec;
};

struct OperationState {
  gboolean cancelled;
  gboolean completed;
  gpointer data;
};

static void grl_media_source_finalize (GObject *object);

static void grl_media_source_get_property (GObject *plugin,
                                           guint prop_id,
                                           GValue *value,
                                           GParamSpec *pspec);
static void grl_media_source_set_property (GObject *object,
                                           guint prop_id,
                                           const GValue *value,
                                           GParamSpec *pspec);

static GrlSupportedOps
grl_media_source_supported_operations (GrlMetadataSource *metadata_source);

static gboolean
operation_is_finished (GrlMediaSource *source,
		       guint operation_id) __attribute__ ((unused)) ;

static guint grl_media_source_gen_operation_id (GrlMediaSource *source);

/* ================ GrlMediaSource GObject ================ */

enum {
  SIG_CONTENT_CHANGED,
  SIG_LAST,
};
static gint registry_signals[SIG_LAST];

G_DEFINE_ABSTRACT_TYPE (GrlMediaSource,
                        grl_media_source,
                        GRL_TYPE_METADATA_SOURCE);

static void
grl_media_source_class_init (GrlMediaSourceClass *media_source_class)
{
  GObjectClass *gobject_class;
  GrlMetadataSourceClass *metadata_source_class;

  gobject_class = G_OBJECT_CLASS (media_source_class);
  metadata_source_class = GRL_METADATA_SOURCE_CLASS (media_source_class);

  gobject_class->finalize = grl_media_source_finalize;
  gobject_class->set_property = grl_media_source_set_property;
  gobject_class->get_property = grl_media_source_get_property;

  metadata_source_class->supported_operations =
    grl_media_source_supported_operations;

  g_type_class_add_private (media_source_class,
                            sizeof (GrlMediaSourcePrivate));

  media_source_class->operation_id = 1;

  /**
   * GrlMediaSource:auto-split-threshold
   *
   * Transparently split queries with count requests
   * bigger than a certain threshold into smaller queries.
   */
  g_object_class_install_property (gobject_class,
				   PROP_AUTO_SPLIT_THRESHOLD,
				   g_param_spec_uint ("auto-split-threshold",
						      "Auto-split threshold",
						      "Threshold to use auto-split of queries",
						      0, G_MAXUINT, 0,
						      G_PARAM_READWRITE |
						      G_PARAM_STATIC_STRINGS));
  /**
   * GrlMediaSource::content-changed:
   * @source: source that has changed
   * @media: the media that changed or one of its ancestors
   * @change_type: the kind of change that ocurred
   * @location_unknown: @TRUE if the change happened in @media itself or in one
   * of its direct children (when @media is a #GrlMediaBox). @FALSE otherwise
   *
   * Signals that the content in the source has changed. Usually @media is a
   * #GrlBox, meaning that the content of that box has changed. if
   * @location_unknown is @TRUE it means the source cannot establish where the
   * change happened: could be either in the box, in any child, or in any
   * other descendant of the box in the hierarchy.
   *
   * For the cases where the source can only signal that a change happened, but
   * not where, it would use the root box (@NULL id) and set location_unknown as
   * to @TRUE.
   */
  registry_signals[SIG_CONTENT_CHANGED] =
    g_signal_new("content-changed",
                 G_TYPE_FROM_CLASS (gobject_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GrlMediaSourceClass, content_changed),
                 NULL,
                 NULL,
                 grl_marshal_VOID__OBJECT_ENUM_BOOLEAN,
                 G_TYPE_NONE,
                 3,
                 GRL_TYPE_MEDIA,
                 GRL_TYPE_MEDIA_SOURCE_CHANGE_TYPE,
                 G_TYPE_BOOLEAN);
}

static void
grl_media_source_init (GrlMediaSource *source)
{
  source->priv = GRL_MEDIA_SOURCE_GET_PRIVATE (source);
  source->priv->pending_operations =
    g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, g_free);
}

static guint
grl_media_source_gen_operation_id (GrlMediaSource *source)
{
  GrlMediaSourceClass *klass;
  klass = GRL_MEDIA_SOURCE_GET_CLASS (source);
  return klass->operation_id++;
}

static void
grl_media_source_get_property (GObject *object,
                               guint prop_id,
                               GValue *value,
                               GParamSpec *pspec)
{
  GrlMediaSource *source;

  source = GRL_MEDIA_SOURCE (object);

  switch (prop_id) {
  case PROP_AUTO_SPLIT_THRESHOLD:
    g_value_set_uint (value, source->priv->auto_split_threshold);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (source, prop_id, pspec);
    break;
  }
}

static void
grl_media_source_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
  GrlMediaSource *source;

  source = GRL_MEDIA_SOURCE (object);

  switch (prop_id) {
  case PROP_AUTO_SPLIT_THRESHOLD:
    source->priv->auto_split_threshold = g_value_get_uint (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (source, prop_id, pspec);
    break;
  }
}

static void
grl_media_source_finalize (GObject *object)
{
  GrlMediaSource *source;

  GRL_DEBUG ("grl_media_source_finalize");

  source = GRL_MEDIA_SOURCE (object);

  g_hash_table_unref (source->priv->pending_operations);

  G_OBJECT_CLASS (grl_media_source_parent_class)->finalize (object);
}

/* ================ Utitilies ================ */

/*
 * Operation states:
 * - finished: We have already emitted the last result to the user
 * - completed: We have already received the last result in the relay cb
 *              (If it is finished it is also completed).
 * - cancelled: Operation valid (not finished) but was cancelled.
 * - ongoing: if the operation is valid (not finished) and not cancelled.
 */
static void
set_operation_finished (GrlMediaSource *source, guint operation_id)
{
  GRL_DEBUG ("set_operation_finished (%d)", operation_id);
  g_hash_table_remove (source->priv->pending_operations,
		       GINT_TO_POINTER (operation_id));
}

static void
set_operation_completed (GrlMediaSource *source, guint operation_id)
{
  struct OperationState *op_state;
  GRL_DEBUG ("set_operation_completed (%d)", operation_id);
  op_state = g_hash_table_lookup (source->priv->pending_operations,
				  GINT_TO_POINTER (operation_id));
  if (op_state) {
    op_state->completed = TRUE;
  }
}

static void
set_operation_cancelled (GrlMediaSource *source, guint operation_id)
{
  struct OperationState *op_state;
  GRL_DEBUG ("set_operation_cancelled (%d)", operation_id);
  op_state = g_hash_table_lookup (source->priv->pending_operations,
				  GINT_TO_POINTER (operation_id));
  if (op_state) {
    op_state->cancelled = TRUE;
  }
}

static void
set_operation_ongoing (GrlMediaSource *source, guint operation_id)
{
  struct OperationState *op_state;

  GRL_DEBUG ("set_operation_ongoing (%d)", operation_id);

  op_state = g_new0 (struct OperationState, 1);
  g_hash_table_insert (source->priv->pending_operations,
		       GINT_TO_POINTER (operation_id), op_state);
}

static gboolean
operation_is_ongoing (GrlMediaSource *source, guint operation_id)
{
  struct OperationState *op_state;
  op_state = g_hash_table_lookup (source->priv->pending_operations,
				  GINT_TO_POINTER (operation_id));
  return op_state && !op_state->cancelled;
}

static gboolean
operation_is_cancelled (GrlMediaSource *source, guint operation_id)
{
  struct OperationState *op_state;
  op_state = g_hash_table_lookup (source->priv->pending_operations,
				  GINT_TO_POINTER (operation_id));
  return op_state && op_state->cancelled;
}

static gboolean
operation_is_completed (GrlMediaSource *source, guint operation_id)
{
  struct OperationState *op_state;
  op_state = g_hash_table_lookup (source->priv->pending_operations,
				  GINT_TO_POINTER (operation_id));
  return !op_state || op_state->completed;
}

static gboolean
operation_is_finished (GrlMediaSource *source, guint operation_id)
{
  struct OperationState *op_state;
  op_state = g_hash_table_lookup (source->priv->pending_operations,
				  GINT_TO_POINTER (operation_id));
  return op_state == NULL;
}

static gpointer
get_operation_data (GrlMediaSource *source, guint operation_id)
{
  struct OperationState *op_state;

  op_state = g_hash_table_lookup (source->priv->pending_operations,
				  GINT_TO_POINTER (operation_id));
  if (op_state) {
    return op_state->data;
  } else {
    GRL_WARNING ("Tried to get operation data but operation does not exist");
    return NULL;
  }
}

static void
set_operation_data (GrlMediaSource *source, guint operation_id, gpointer data)
{
  struct OperationState *op_state;

  GRL_DEBUG ("set_operation_data");

  op_state = g_hash_table_lookup (source->priv->pending_operations,
				  GINT_TO_POINTER (operation_id));
  if (op_state) {
    op_state->data = data;
  } else {
    GRL_WARNING ("Tried to set operation data but operation does not exist");
  }
}

static void
free_browse_operation_spec (GrlMediaSourceBrowseSpec *spec)
{
  GRL_DEBUG ("free_browse_operation_spec");
  g_object_unref (spec->source);
  g_object_unref (spec->container);
  g_list_free (spec->keys);
  g_free (spec);
}

static void
free_search_operation_spec (GrlMediaSourceSearchSpec *spec)
{
  GRL_DEBUG ("free_search_operation_spec");
  g_object_unref (spec->source);
  g_free (spec->text);
  g_list_free (spec->keys);
  g_free (spec);
}

static void
free_query_operation_spec (GrlMediaSourceQuerySpec *spec)
{
  GRL_DEBUG ("free_query_operation_spec");
  g_object_unref (spec->source);
  g_free (spec->query);
  g_list_free (spec->keys);
  g_free (spec);
}

static void
free_source_map_list (GList *source_map_list)
{
  GList *iter;
  iter = source_map_list;
  while (iter) {
    struct SourceKeyMap *map = (struct SourceKeyMap *) iter->data;
    g_object_unref (map->source);
    g_list_free (map->keys);
    iter = g_list_next (iter);
  }
  g_list_free (source_map_list);
}

static gboolean
browse_idle (gpointer user_data)
{
  GRL_DEBUG ("browse_idle");
  GrlMediaSourceBrowseSpec *bs = (GrlMediaSourceBrowseSpec *) user_data;
  /* Check if operation was cancelled even before the idle kicked in */
  if (!operation_is_cancelled (bs->source, bs->browse_id)) {
    GRL_MEDIA_SOURCE_GET_CLASS (bs->source)->browse (bs->source, bs);
  } else {
    GRL_DEBUG ("  operation was cancelled");
    bs->callback (bs->source, bs->browse_id, NULL, 0, bs->user_data, NULL);
  }
  return FALSE;
}

static gboolean
search_idle (gpointer user_data)
{
  GRL_DEBUG ("search_idle");
  GrlMediaSourceSearchSpec *ss = (GrlMediaSourceSearchSpec *) user_data;
  /* Check if operation was cancelled even before the idle kicked in */
  if (!operation_is_cancelled (ss->source, ss->search_id)) {
    GRL_MEDIA_SOURCE_GET_CLASS (ss->source)->search (ss->source, ss);
  } else {
    GRL_DEBUG ("  operation was cancelled");
    ss->callback (ss->source, ss->search_id, NULL, 0, ss->user_data, NULL);
  }
  return FALSE;
}

static gboolean
query_idle (gpointer user_data)
{
  GRL_DEBUG ("query_idle");
  GrlMediaSourceQuerySpec *qs = (GrlMediaSourceQuerySpec *) user_data;
  if (!operation_is_cancelled (qs->source, qs->query_id)) {
    GRL_MEDIA_SOURCE_GET_CLASS (qs->source)->query (qs->source, qs);
  } else {
    GRL_DEBUG ("  operation was cancelled");
    qs->callback (qs->source, qs->query_id, NULL, 0, qs->user_data, NULL);
  }
  return FALSE;
}

static gboolean
metadata_idle (gpointer user_data)
{
  GRL_DEBUG ("metadata_idle");
  GrlMediaSourceMetadataSpec *ms = (GrlMediaSourceMetadataSpec *) user_data;
  if (!operation_is_cancelled (ms->source, ms->metadata_id)) {
    GRL_MEDIA_SOURCE_GET_CLASS (ms->source)->metadata (ms->source, ms);
  } else {
    GRL_DEBUG ("  operation was cancelled");
    ms->callback (ms->source, ms->media, ms->user_data, NULL);
  }
  return FALSE;
}

static void
store_idle_destroy (gpointer user_data)
{
  GrlMediaSourceStoreSpec *ss = (GrlMediaSourceStoreSpec *) user_data;
  g_object_unref (ss->source);
  if (ss->parent)
    g_object_unref (ss->parent);
  g_object_unref (ss->media);
  g_free (ss);
}

static gboolean
store_idle (gpointer user_data)
{
  GRL_DEBUG ("store_idle");
  GrlMediaSourceStoreSpec *ss = (GrlMediaSourceStoreSpec *) user_data;
  GRL_MEDIA_SOURCE_GET_CLASS (ss->source)->store (ss->source, ss);
  return FALSE;
}

static void
remove_idle_destroy (gpointer user_data)
{
  GrlMediaSourceRemoveSpec *rs = (GrlMediaSourceRemoveSpec *) user_data;
  g_object_unref (rs->source);
  g_free (rs->media_id);
  g_free (rs);
}

static gboolean
remove_idle (gpointer user_data)
{
  GRL_DEBUG ("remove_idle");
  GrlMediaSourceRemoveSpec *rs = (GrlMediaSourceRemoveSpec *) user_data;
  GRL_MEDIA_SOURCE_GET_CLASS (rs->source)->remove (rs->source, rs);
  return FALSE;
}

static void
media_from_uri_relay_cb (GrlMediaSource *source,
			 GrlMedia *media,
			 gpointer user_data,
			 const GError *error)
{
  GRL_DEBUG ("media_from_uri_relay_cb");

  struct MediaFromUriRelayCb *mfsrc;

  mfsrc = (struct MediaFromUriRelayCb *) user_data;
  if (media) {
    grl_media_set_source (media,
                          grl_metadata_source_get_id (GRL_METADATA_SOURCE (source)));
  }

  mfsrc->user_callback (source, media, mfsrc->user_data, error);

  g_object_unref (mfsrc->spec->source);
  g_free (mfsrc->spec->uri);
  g_list_free (mfsrc->spec->keys);
  g_free (mfsrc->spec);
  g_free (mfsrc);
}

static gboolean
media_from_uri_idle (gpointer user_data)
{
  GRL_DEBUG ("media_from_uri_idle");
  GrlMediaSourceMediaFromUriSpec *mfus =
    (GrlMediaSourceMediaFromUriSpec *) user_data;
  GRL_MEDIA_SOURCE_GET_CLASS (mfus->source)->media_from_uri (mfus->source,
							     mfus);
  return FALSE;
}

static gboolean
browse_result_relay_idle (gpointer user_data)
{
  GRL_DEBUG ("browse_result_relay_idle");

  struct BrowseRelayIdle *bri = (struct BrowseRelayIdle *) user_data;
  gboolean cancelled = FALSE;

  /* Check if operation was cancelled (could be cancelled between the relay
     callback and this idle loop iteration). Remember that we do
     emit the last result (remaining == 0) in any case. */
  if (operation_is_cancelled (bri->source, bri->browse_id)) {
    if (bri->media) {
      g_object_unref (bri->media);
      bri->media = NULL;
    }
    cancelled = TRUE;
  }
  if (!cancelled || bri->remaining == 0) {
    bri->user_callback (bri->source,
			bri->browse_id,
			bri->media,
			bri->remaining,
			bri->user_data,
			bri->error);
  } else {
    GRL_DEBUG ("operation was cancelled, skipping idle result!");
  }

  if (bri->remaining == 0 && !bri->chained) {
    /* This is the last post-processing callback, so we can remove
       the operation state data here */
    set_operation_finished (bri->source, bri->browse_id);
  }

  /* We copy the error if we do idle relay, we have to free it here */
  if (bri->error) {
    g_error_free (bri->error);
  }

  g_free (bri);

  return FALSE;
}

static void
auto_split_run_next_chunk (struct BrowseRelayCb *brc, guint remaining)
{
  struct AutoSplitCtl *as_info = brc->auto_split;
  guint *skip = NULL;
  guint *count = NULL;
  GSourceFunc operation = NULL;
  gpointer spec = NULL;

  /* Identify the operation we are handling */
  if (brc->bspec) {
    spec = brc->bspec;
    skip = &brc->bspec->skip;
    count = &brc->bspec->count;
    operation = browse_idle;
  } else if (brc->sspec) {
    spec = brc->sspec;
    skip = &brc->sspec->skip;
    count = &brc->sspec->count;
    operation = search_idle;
  } else if (brc->qspec) {
    spec = brc->qspec;
    skip = &brc->qspec->skip;
    count = &brc->qspec->count;
    operation = query_idle;
  }

  /* Go for next chunk */
  *skip += as_info->chunk_requested;
  as_info->chunk_first = TRUE;
  as_info->chunk_consumed = 0;
  if (remaining < as_info->threshold) {
    as_info->chunk_requested = remaining;
  }
  *count = as_info->chunk_requested;
  GRL_DEBUG ("auto-split: requesting next chunk (skip=%u, count=%u)",
             *skip, *count);
  g_idle_add (operation, spec);
}

static void
browse_result_relay_cb (GrlMediaSource *source,
			guint browse_id,
			GrlMedia *media,
			guint remaining,
			gpointer user_data,
			const GError *error)
{
  struct BrowseRelayCb *brc;
  guint plugin_remaining = remaining;

  GRL_DEBUG ("browse_result_relay_cb, op:%u, source:%s, remaining:%u",
             browse_id,
             grl_metadata_source_get_name (GRL_METADATA_SOURCE (source)),
             remaining);

  brc = (struct BrowseRelayCb *) user_data;

  plugin_remaining = remaining;

  /* --- operation cancel management --- */

  /* Check if operation is still valid , otherwise do not emit the result
     but make sure to free the operation data when remaining is 0 */
  if (!operation_is_ongoing (source, browse_id)) {
    GRL_DEBUG ("operation is cancelled or already finished, skipping result!");
    if (media) {
      g_object_unref (media);
      media = NULL;
    }
    if (brc->auto_split) {
      /* Stop auto-split, of course */
      g_free (brc->auto_split);
      brc->auto_split = NULL;
    }
    if (remaining > 0) {
      return;
    }
    if (operation_is_completed (source, browse_id)) {
      /* If the operation was cancelled, we ignore all results until
	 we get the last one, which we let through so all chained callbacks
	 have the chance to free their resources. If the operation is already
	 completed (includes finished) however, we already let the last
	 result through and doing it again would cause a crash */
      GRL_WARNING ("Source '%s' emitted 'remaining=0' more than once for "
                   "operation %d",
                   grl_metadata_source_get_name (GRL_METADATA_SOURCE (source)),
                   browse_id);
      return;
    }
    /* If we reached this point the operation is cancelled but not completed
       and this is the last result (remaining == 0) */
  }

  /* --- auto split management  --- */

  if (brc->auto_split) {
    struct AutoSplitCtl *as_info = brc->auto_split;
    /* Adjust remaining count if the plugin was not able to
       provide as many results as we requested */
    if (as_info->chunk_first) {
      if (plugin_remaining < as_info->chunk_requested - 1) {
	as_info->count = plugin_remaining + 1;
      }
      as_info->chunk_first = FALSE;
    }

    as_info->count--;
    as_info->chunk_consumed++;

    /* FIXME: If we received less than we requested we should
       not do an extra query */
    remaining = as_info->count;
  }

  /* --- relay operation  --- */

  /* This is to prevent crash when plugins emit remaining=0 more than once */
  if (remaining == 0) {
    set_operation_completed (source, browse_id);
  }

  if (media) {
    grl_media_set_source (media,
                          grl_metadata_source_get_id (GRL_METADATA_SOURCE (source)));
  }

  /* TODO: this should be TRUE if GRL_RESOLVE_FULL was requested too,
     after all GRL_RESOLVE_FULL already forces the idle loop before emission */
  if (brc->use_idle) {
    struct BrowseRelayIdle *bri = g_new (struct BrowseRelayIdle, 1);
    bri->source = source;
    bri->browse_id = browse_id;
    bri->media = media;
    bri->remaining = remaining;
    bri->error = (GError *) (error ? g_error_copy (error) : NULL);
    bri->user_callback = brc->user_callback;
    bri->user_data = brc->user_data;
    bri->chained = brc->chained;
    g_idle_add (browse_result_relay_idle, bri);
  } else {
    brc->user_callback (source,
			browse_id,
			media,
			remaining,
			brc->user_data,
			error);
    if (remaining == 0 && !brc->chained) {
      /* This is the last post-processing callback, so we can remove
	 the operation state data here */
      set_operation_finished (source, browse_id);
    }
  }

  /* --- auto split management --- */

  if (brc->auto_split) {
    if (plugin_remaining == 0 && remaining > 0) {
      auto_split_run_next_chunk (brc, remaining);
    }
  }

  /* --- free relay information  --- */

  /* Free callback data when we processed the last result */
  if (remaining == 0) {
    GRL_DEBUG ("Got remaining '0' for operation %d (%s)",
               browse_id,
               grl_metadata_source_get_name (GRL_METADATA_SOURCE (source)));
    if (brc->bspec) {
      free_browse_operation_spec (brc->bspec);
    } else if (brc->sspec) {
      free_search_operation_spec (brc->sspec);
    } else if (brc->sspec) {
      free_query_operation_spec (brc->qspec);
    }
    g_free (brc->auto_split);
    g_free (brc);
  }
}

static void
multiple_result_async_cb (GrlMediaSource *source,
                          guint op_id,
                          GrlMedia *media,
                          guint remaining,
                          gpointer user_data,
                          const GError *error)
{
  GrlDataSync *ds = (GrlDataSync *) user_data;

  GRL_DEBUG ("multiple_result_async_cb");

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

static void
metadata_result_relay_cb (GrlMediaSource *source,
			  GrlMedia *media,
			  gpointer user_data,
			  const GError *error)
{
  GRL_DEBUG ("metadata_result_relay_cb");

  struct MetadataRelayCb *mrc;

  mrc = (struct MetadataRelayCb *) user_data;
  if (media) {
    grl_media_set_source (media,
                          grl_metadata_source_get_id (GRL_METADATA_SOURCE (source)));
  }

  mrc->user_callback (source, media, mrc->user_data, error);

  g_object_unref (mrc->spec->source);
  if (mrc->spec->media) {
    /* Can be NULL if getting metadata for root category */
    g_object_unref (mrc->spec->media);
  }
  g_list_free (mrc->spec->keys);
  g_free (mrc->spec);
  g_free (mrc);
}

static void
metadata_result_async_cb (GrlMediaSource *source,
                          GrlMedia *media,
                          gpointer user_data,
                          const GError *error)
{
  GrlDataSync *ds = (GrlDataSync *) user_data;

  GRL_DEBUG ("metadata_result_async_cb");

  if (error) {
    ds->error = g_error_copy (error);
  }

  ds->data = media;
  ds->complete = TRUE;
}

static void
store_async_cb (GrlMediaSource *source,
                GrlMediaBox *parent,
                GrlMedia *media,
                gpointer user_data,
                const GError *error)
{
  GrlDataSync *ds = (GrlDataSync *) user_data;

  GRL_DEBUG ("store_async_cb");

  if (error) {
    ds->error = g_error_copy (error);
  }

  ds->complete = TRUE;
}

static void
remove_async_cb (GrlMediaSource *source,
                 GrlMedia *media,
                 gpointer user_data,
                 const GError *error)
{
  GrlDataSync *ds = (GrlDataSync *) user_data;

  GRL_DEBUG ("remove_async_cb");

  if (error) {
    ds->error = g_error_copy (error);
  }

  ds->complete = TRUE;
}

static gint
compare_sorted_results (gconstpointer a, gconstpointer b)
{
  struct SortedResult *r1 = (struct SortedResult *) a;
  struct SortedResult *r2 = (struct SortedResult *) b;
  return r1->remaining < r2->remaining;
}

static void
full_resolution_add_to_waiting_list (GList **waiting_list,
				     GrlMedia *media,
				     guint index)
{
  struct SortedResult *result;
  result = g_new (struct SortedResult, 1);
  result->media = media;
  result->remaining = index;
  *waiting_list = g_list_insert_sorted (*waiting_list,
					result,
					compare_sorted_results);
}

static gboolean
full_resolution_check_waiting_list (GList **waiting_list,
				    guint index,
				    struct FullResolutionDoneCb *done_cb,
				    guint *last_index)
{
  struct FullResolutionCtlCb *ctl_info;
  gboolean emitted = FALSE;

  ctl_info = done_cb->ctl_info;
  if (!ctl_info->next_index)
    return emitted;

  while (*waiting_list) {
    struct SortedResult *r = (struct SortedResult *) (*waiting_list)->data;
    guint index = GPOINTER_TO_UINT (ctl_info->next_index->data);
    if (r->remaining == index) {
      emitted = TRUE;
      *last_index = index;
      ctl_info->user_callback (done_cb->source,
			       done_cb->browse_id,
			       r->media,
			       r->remaining,
			       ctl_info->user_data,
			       NULL);
      /* Move to next index and next item in waiting list */
      ctl_info->next_index =
	g_list_delete_link (ctl_info->next_index, ctl_info->next_index);
      g_free ((*waiting_list)->data);
      *waiting_list = g_list_delete_link (*waiting_list, *waiting_list);
    } else {
      break;
    }
  }

  return emitted;
}

static void
full_resolution_done_cb (GrlMetadataSource *source,
			 GrlMedia *media,
			 gpointer user_data,
			 const GError *error)
{
  GRL_DEBUG ("full_resolution_done_cb");

  gboolean cancelled = FALSE;
  struct FullResolutionCtlCb *ctl_info;
  struct FullResolutionDoneCb *cb_info =
    (struct FullResolutionDoneCb *) user_data;

  cb_info->pending_callbacks--;

  /* We we have a valid source this error comes from the resoluton operation.
     In that case we just did not manage to resolve extra metadata, but
     the result itself as provided by the control callback is valid so we
     just log the error and emit the result as valid. If we do not have a
     source though, it means the error was provided by the control callback
     and in that case we have to emit it */
  if (error && source) {
    GRL_WARNING ("Failed to fully resolve some metadata: %s", error->message);
    error = NULL;
  }

  /* If we are done with this result, invoke the user's callback */
  if (cb_info->pending_callbacks == 0) {
    ctl_info = cb_info->ctl_info;
    /* But check if operation was cancelled (or even finished) before emitting
       (we execute in the idle loop) */
    if (operation_is_cancelled (cb_info->source, cb_info->browse_id)) {
      GRL_DEBUG ("operation was cancelled, skipping full resolution done "
                 "result!");
      if (media) {
	g_object_unref (media);
	media = NULL;
      }
      cancelled = TRUE;
    }

    if (!cancelled || cb_info->remaining == 0) {
      /* We can emit the result, but we have to do it in the right order:
	 we cannot guarantee that all the elements are fully resolved in
	 the same order that was requested. Only exception is the operation
	 was cancelled and this is the one with remaining == 0*/
      if (GPOINTER_TO_UINT (ctl_info->next_index->data) == cb_info->remaining
	  || cancelled) {
	/* Notice we pass NULL as error on purpose
	   since the result is valid even if the full-resolution failed */
	guint remaining = cb_info->remaining;
	GRL_DEBUG ("  Result is in sort order, emitting (%d)", remaining);
	ctl_info->user_callback (cb_info->source,
				 cb_info->browse_id,
				 media,
				 cb_info->remaining,
				 ctl_info->user_data,
				 error);
	ctl_info->next_index = g_list_delete_link (ctl_info->next_index,
						   ctl_info->next_index);
	/* Now that we have emitted the next result, check if we
	   had results waiting for this one to be emitted */
	if (remaining != 0) {
	  full_resolution_check_waiting_list (&ctl_info->waiting_list,
					      cb_info->remaining,
					      cb_info,
					      &remaining);
	}
	if (remaining == 0) {
	  if (!ctl_info->chained) {
	    /* We are the last post-processing callback, finish operation */
	    set_operation_finished (cb_info->source, cb_info->browse_id);
	  }
	  /* We are done, free the control information now */
	  free_source_map_list (ctl_info->source_map_list);
	  g_free (ctl_info);	}
      } else {
	full_resolution_add_to_waiting_list (&ctl_info->waiting_list,
					     media,
					     cb_info->remaining);
      }
    }
    g_free (cb_info);
  }
}

static void
full_resolution_ctl_cb (GrlMediaSource *source,
			guint browse_id,
			GrlMedia *media,
			guint remaining,
			gpointer user_data,
			const GError *error)
{
  GList *iter;
  struct FullResolutionCtlCb *ctl_info =
    (struct FullResolutionCtlCb *) user_data;

  GRL_DEBUG ("full_resolution_ctl_cb");

  /* No need to check if the operation is cancelled, that was
     already checked in the relay callback and this is called
     from there synchronously */

  /* We cannot guarantee that full resolution callbacks will
     keep the emission order, so we have to make sure we emit
     in the same order we receive results here. We use the
     remaining associated to each result to get that order. */
  ctl_info->next_index = g_list_append (ctl_info->next_index,
					GUINT_TO_POINTER (remaining));

  struct FullResolutionDoneCb *done_info =
    g_new (struct FullResolutionDoneCb, 1);

  if (error || !media) {
    /* No need to start full resolution here, but we cannot emit right away
       either (we have to ensure the order) and that's done in the
       full_resolution_done_cb, so we fake the resolution to get into that
       callback */
    done_info->pending_callbacks = 1;
    done_info->source = source;
    done_info->browse_id = browse_id;
    done_info->remaining = remaining;
    done_info->ctl_info = ctl_info;
    full_resolution_done_cb (NULL, media, done_info, error);
  } else {
    /* Start full-resolution: save all the data we need to emit the result
       when fully resolved */
    done_info->pending_callbacks = g_list_length (ctl_info->source_map_list);
    done_info->source = source;
    done_info->browse_id = browse_id;
    done_info->remaining = remaining;
    done_info->ctl_info = ctl_info;

    /* Use sources in the map to fill in missing metadata, the "done"
       callback will be used to emit the resulting object when
       all metadata has been gathered */
    iter = ctl_info->source_map_list;
    while (iter) {
      gchar *name;
      struct SourceKeyMap *map = (struct SourceKeyMap *) iter->data;
      g_object_get (map->source, "source-name", &name, NULL);
      GRL_DEBUG ("Using '%s' to resolve extra metadata now", name);

      grl_metadata_source_resolve (map->source,
                                   map->keys,
                                   media,
                                   ctl_info->flags,
                                   full_resolution_done_cb,
                                   done_info);

      iter = g_list_next (iter);
    }
  }
}

static void
metadata_full_resolution_done_cb (GrlMetadataSource *source,
				  GrlMedia *media,
				  gpointer user_data,
				  const GError *error)
{
  GRL_DEBUG ("metadata_full_resolution_done_cb");

  struct MetadataFullResolutionDoneCb *cb_info =
    (struct MetadataFullResolutionDoneCb *) user_data;

  cb_info->pending_callbacks--;

  if (error) {
    GRL_WARNING ("Failed to fully resolve some metadata: %s", error->message);
  }

  if (cb_info->pending_callbacks == 0) {
    cb_info->user_callback (cb_info->source,
			    media,
			    cb_info->user_data,
			    NULL);

    free_source_map_list (cb_info->ctl_info->source_map_list);
    g_free (cb_info->ctl_info);
    g_free (cb_info);
  }
}

static void
metadata_full_resolution_ctl_cb (GrlMediaSource *source,
				 GrlMedia *media,
				 gpointer user_data,
				 const GError *error)
{
  GList *iter;

  struct MetadataFullResolutionCtlCb *ctl_info =
    (struct MetadataFullResolutionCtlCb *) user_data;

  GRL_DEBUG ("metadata_full_resolution_ctl_cb");

  /* If we got an error, invoke the user callback right away and bail out */
  if (error) {
    GRL_WARNING ("Operation failed: %s", error->message);
    ctl_info->user_callback (source,
			     media,
			     ctl_info->user_data,
			     error);
    return;
  }

  /* Save all the data we need to emit the result */
  struct MetadataFullResolutionDoneCb *done_info =
    g_new (struct MetadataFullResolutionDoneCb, 1);
  done_info->user_callback = ctl_info->user_callback;
  done_info->user_data = ctl_info->user_data;
  done_info->pending_callbacks = g_list_length (ctl_info->source_map_list);
  done_info->source = source;
  done_info->ctl_info = ctl_info;

  /* Use sources in the map to fill in missing metadata, the "done"
     callback will be used to emit the resulting object when
     all metadata has been gathered */
  iter = ctl_info->source_map_list;
  while (iter) {
    gchar *name;
    struct SourceKeyMap *map = (struct SourceKeyMap *) iter->data;
    g_object_get (map->source, "source-name", &name, NULL);
    GRL_DEBUG ("Using '%s' to resolve extra metadata now", name);

    grl_metadata_source_resolve (map->source,
                                 map->keys,
                                 media,
                                 ctl_info->flags,
                                 metadata_full_resolution_done_cb,
                                 done_info);

    iter = g_list_next (iter);
  }
}

/* ================ API ================ */

/**
 * grl_media_source_browse:
 * @source: a media source
 * @container: (allow-none): a container of data transfer objects
 * @keys: (element-type GObject.ParamSpec): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @skip: the number if elements to skip in the browse operation
 * @count: the number of elements to retrieve in the browse operation
 * @flags: the resolution mode
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Browse from @skip, a @count number of media elements through an available list.
 *
 * This method is asynchronous.
 *
 * Returns: the operation identifier
 *
 * Since: 0.1.4
 */
guint
grl_media_source_browse (GrlMediaSource *source,
                         GrlMedia *container,
                         const GList *keys,
                         guint skip,
                         guint count,
                         GrlMetadataResolutionFlags flags,
                         GrlMediaSourceResultCb callback,
                         gpointer user_data)
{
  GrlMediaSourceResultCb _callback;
  gpointer _user_data ;
  GList *_keys;
  struct SourceKeyMapList key_mapping;
  GrlMediaSourceBrowseSpec *bs;
  guint browse_id;
  struct BrowseRelayCb *brc;
  gboolean relay_chained = FALSE;
  gboolean full_chained = FALSE;

  g_return_val_if_fail (GRL_IS_MEDIA_SOURCE (source), 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (count > 0, 0);
  g_return_val_if_fail (grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (source)) &
			GRL_OP_BROWSE, 0);

  /* By default assume we will use the parameters specified by the user */
  _keys = g_list_copy ((GList *) keys);
  _callback = callback;
  _user_data = user_data;

  if (flags & GRL_RESOLVE_FAST_ONLY) {
    GRL_DEBUG ("requested fast keys only");
    grl_metadata_source_filter_slow (GRL_METADATA_SOURCE (source),
                                     &_keys,
                                     FALSE);
  }

  /* Setup full resolution mode if requested */
  if (flags & GRL_RESOLVE_FULL) {
    GRL_DEBUG ("requested full resolution");
    grl_metadata_source_setup_full_resolution_mode (GRL_METADATA_SOURCE (source),
                                                    NULL, _keys, &key_mapping);

    /* If we do not have a source map for the unsupported keys then
       we cannot resolve any of them */
    if (key_mapping.source_maps != NULL) {
      struct FullResolutionCtlCb *c = g_new0 (struct FullResolutionCtlCb, 1);
      c->user_callback = _callback;
      c->user_data = _user_data;
      c->source_map_list = key_mapping.source_maps;
      c->flags = flags;
      c->chained = full_chained;

      _callback = full_resolution_ctl_cb;
      _user_data = c;
      g_list_free (_keys);
      _keys = key_mapping.operation_keys;

      relay_chained = TRUE;
    }
  }

  browse_id = grl_media_source_gen_operation_id (source);

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */
  brc = g_new0 (struct BrowseRelayCb, 1);
  brc->chained = relay_chained;
  brc->user_callback = _callback;
  brc->user_data = _user_data;
  brc->use_idle = flags & GRL_RESOLVE_IDLE_RELAY;
  _callback = browse_result_relay_cb;
  _user_data = brc;

  bs = g_new0 (GrlMediaSourceBrowseSpec, 1);
  bs->source = g_object_ref (source);
  bs->browse_id = browse_id;
  bs->keys = _keys;
  bs->skip = skip;
  bs->count = count;
  bs->flags = flags;
  bs->callback = _callback;
  bs->user_data = _user_data;
  if (!container) {
    /* Special case: NULL container ==> NULL id */
    bs->container = grl_media_box_new ();
    grl_media_set_id (bs->container, NULL);
  } else {
    bs->container = g_object_ref (container);
  }

  /* Save a reference to the operaton spec in the relay-cb's
     user_data so that we can free the spec there when we get
     the last result */
  brc->bspec = bs;

  /* Setup auto-split management if requested */
  if (source->priv->auto_split_threshold > 0 &&
      count > source->priv->auto_split_threshold) {
    GRL_DEBUG ("auto-split: enabled");
    struct AutoSplitCtl *as_ctl = g_new0 (struct AutoSplitCtl, 1);
    as_ctl->count = count;
    as_ctl->threshold = source->priv->auto_split_threshold;
    as_ctl->chunk_requested = as_ctl->threshold;
    as_ctl->chunk_first = TRUE;
    bs->count = as_ctl->chunk_requested;
    brc->auto_split = as_ctl;
    GRL_DEBUG ("auto-split: requesting first chunk (skip=%u, count=%u)",
               bs->skip, bs->count);
  }

  set_operation_ongoing (source, browse_id);
  g_idle_add (browse_idle, bs);

  return browse_id;
}

/**
 * grl_media_source_browse_sync:
 * @source: a media source
 * @container: (allow-none): a container of data transfer objects
 * @keys: (element-type GObject.ParamSpec): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @skip: the number if elements to skip in the browse operation
 * @count: the number of elements to retrieve in the browse operation
 * @flags: the resolution mode
 * @error: a #GError, or @NULL
 *
 * Browse from @skip, a @count number of media elements through an available
 * list.
 *
 * This method is synchronous.
 *
 * Returns: (element-type Grl.Media) (transfer full): a #GList with #GrlMedia
 * elements. After use g_object_unref() every element and g_list_free() the
 * list.
 *
 * Since: 0.1.6
 */
GList *
grl_media_source_browse_sync (GrlMediaSource *source,
                              GrlMedia *container,
                              const GList *keys,
                              guint skip,
                              guint count,
                              GrlMetadataResolutionFlags flags,
                              GError **error)
{
  GrlDataSync *ds;
  GList *result;

  ds = g_slice_new0 (GrlDataSync);

  grl_media_source_browse (source,
                           container,
                           keys,
                           skip,
                           count,
                           flags,
                           multiple_result_async_cb,
                           ds);

  grl_wait_for_async_operation_complete (ds);

  if (ds->error) {
    if (error) {
      *error = ds->error;
    } else {
      g_error_free (ds->error);
    }
  }

  result = (GList *) ds->data;
  g_slice_free (GrlDataSync, ds);

  return result;
}

/**
 * grl_media_source_search:
 * @source: a media source
 * @text: the text to search
 * @keys: (element-type GObject.ParamSpec): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @skip: the number if elements to skip in the search operation
 * @count: the number of elements to retrieve in the search operation
 * @flags: the resolution mode
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Search for the @text string in a media source for data identified with
 * that string.
 *
 * This method is asynchronous.
 *
 * Returns: the operation identifier
 *
 * Since: 0.1.1
 */
guint
grl_media_source_search (GrlMediaSource *source,
                         const gchar *text,
                         const GList *keys,
                         guint skip,
                         guint count,
                         GrlMetadataResolutionFlags flags,
                         GrlMediaSourceResultCb callback,
                         gpointer user_data)
{
  GrlMediaSourceResultCb _callback;
  gpointer _user_data ;
  GList *_keys;
  struct SourceKeyMapList key_mapping;
  GrlMediaSourceSearchSpec *ss;
  guint search_id;
  struct BrowseRelayCb *brc;
  gboolean relay_chained = FALSE;
  gboolean full_chained = FALSE;

  g_return_val_if_fail (GRL_IS_MEDIA_SOURCE (source), 0);
  g_return_val_if_fail (text != NULL, 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (count > 0, 0);
  g_return_val_if_fail (grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (source)) &
			GRL_OP_SEARCH, 0);

  /* By default assume we will use the parameters specified by the user */
  _callback = callback;
  _user_data = user_data;
  _keys = g_list_copy ((GList *) keys);

  if (flags & GRL_RESOLVE_FAST_ONLY) {
    GRL_DEBUG ("requested fast keys only");
    grl_metadata_source_filter_slow (GRL_METADATA_SOURCE (source), &_keys, FALSE);
  }

  if (flags & GRL_RESOLVE_FULL) {
    GRL_DEBUG ("requested full search");
    grl_metadata_source_setup_full_resolution_mode (GRL_METADATA_SOURCE (source),
                                                    NULL, _keys, &key_mapping);

    /* If we do not have a source map for the unsupported keys then
       we cannot resolve any of them */
    if (key_mapping.source_maps != NULL) {
      struct FullResolutionCtlCb *c = g_new0 (struct FullResolutionCtlCb, 1);
      c->user_callback = callback;
      c->user_data = user_data;
      c->source_map_list = key_mapping.source_maps;
      c->flags = flags;
      c->chained = full_chained;

      _callback = full_resolution_ctl_cb;
      _user_data = c;
      g_list_free (_keys);
      _keys = key_mapping.operation_keys;

      relay_chained = TRUE;
    }
  }

  search_id = grl_media_source_gen_operation_id (source);

  brc = g_new0 (struct BrowseRelayCb, 1);
  brc->chained = relay_chained;
  brc->user_callback = _callback;
  brc->user_data = _user_data;
  brc->use_idle = flags & GRL_RESOLVE_IDLE_RELAY;
  _callback = browse_result_relay_cb;
  _user_data = brc;

  ss = g_new0 (GrlMediaSourceSearchSpec, 1);
  ss->source = g_object_ref (source);
  ss->search_id = search_id;
  ss->text = g_strdup (text);
  ss->keys = _keys;
  ss->skip = skip;
  ss->count = count;
  ss->flags = flags;
  ss->callback = _callback;
  ss->user_data = _user_data;

  /* Save a reference to the operaton spec in the relay-cb's
     user_data so that we can free the spec there when we get
     the last result */
  brc->sspec = ss;

  /* Setup auto-split management if requested */
  if (source->priv->auto_split_threshold > 0 &&
      count > source->priv->auto_split_threshold) {
    GRL_DEBUG ("auto-split: enabled");
    struct AutoSplitCtl *as_ctl = g_new0 (struct AutoSplitCtl, 1);
    as_ctl->count = count;
    as_ctl->threshold = source->priv->auto_split_threshold;
    as_ctl->chunk_requested = as_ctl->threshold;
    as_ctl->chunk_first = TRUE;
    ss->count = as_ctl->chunk_requested;
    brc->auto_split = as_ctl;
    GRL_DEBUG ("auto-split: requesting first chunk (skip=%u, count=%u)",
               ss->skip, ss->count);
  }

  set_operation_ongoing (source, search_id);
  g_idle_add (search_idle, ss);

  return search_id;
}

/**
 * grl_media_source_search_sync:
 * @source: a media source
 * @text: the text to search
 * @keys: (element-type GObject.ParamSpec): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @skip: the number if elements to skip in the search operation
 * @count: the number of elements to retrieve in the search operation
 * @flags: the resolution mode
 * @error: a #GError, or @NULL
 *
 * Search for the @text string in a media source for data identified with
 * that string.
 *
 * This method is synchronous.
 *
 * Returns: (element-type Grl.Media) (transfer full): a #GList with #GrlMedia
 * elements. After use g_object_unref() every element and g_list_free() the
 * list.
 *
 * Since: 0.1.6
 */
GList *
grl_media_source_search_sync (GrlMediaSource *source,
                              const gchar *text,
                              const GList *keys,
                              guint skip,
                              guint count,
                              GrlMetadataResolutionFlags flags,
                              GError **error)
{
  GrlDataSync *ds;
  GList *result;

  ds = g_slice_new0 (GrlDataSync);

  grl_media_source_search (source,
                           text,
                           keys,
                           skip,
                           count,
                           flags,
                           multiple_result_async_cb,
                           ds);

  grl_wait_for_async_operation_complete (ds);

  if (ds->error) {
    if (error) {
      *error = ds->error;
    } else {
      g_error_free (ds->error);
    }
  }

  result = (GList *) ds->data;
  g_slice_free (GrlDataSync, ds);

  return result;
}

/**
 * grl_media_source_query:
 * @source: a media source
 * @query: the query to process
 * @keys: (element-type GObject.ParamSpec): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @skip: the number if elements to skip in the query operation
 * @count: the number of elements to retrieve in the query operation
 * @flags: the resolution mode
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Execute a specialized query (specific for each provider) on a media
 * repository.
 *
 * It is different from grl_media_source_search() semantically, because
 * the query implies a carefully crafted string, rather than a simple
 * string to search.
 *
 * This method is asynchronous.
 *
 * Returns: the operation identifier
 *
 * Since: 0.1.1
 */
guint
grl_media_source_query (GrlMediaSource *source,
                        const gchar *query,
                        const GList *keys,
                        guint skip,
                        guint count,
                        GrlMetadataResolutionFlags flags,
                        GrlMediaSourceResultCb callback,
                        gpointer user_data)
{
  GrlMediaSourceResultCb _callback;
  gpointer _user_data ;
  GList *_keys;
  struct SourceKeyMapList key_mapping;
  GrlMediaSourceQuerySpec *qs;
  guint query_id;
  struct BrowseRelayCb *brc;
  gboolean relay_chained = FALSE;
  gboolean full_chained = FALSE;

  g_return_val_if_fail (GRL_IS_MEDIA_SOURCE (source), 0);
  g_return_val_if_fail (query != NULL, 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (count > 0, 0);
  g_return_val_if_fail (grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (source)) &
			GRL_OP_QUERY, 0);

  /* By default assume we will use the parameters specified by the user */
  _callback = callback;
  _user_data = user_data;
  _keys = g_list_copy ((GList *) keys);

  if (flags & GRL_RESOLVE_FAST_ONLY) {
    GRL_DEBUG ("requested fast keys only");
    grl_metadata_source_filter_slow (GRL_METADATA_SOURCE (source),
                                     &_keys,
                                     FALSE);
  }

  if (flags & GRL_RESOLVE_FULL) {
    GRL_DEBUG ("requested full search");
    grl_metadata_source_setup_full_resolution_mode (GRL_METADATA_SOURCE (source),
                                                    NULL, _keys, &key_mapping);

    /* If we do not have a source map for the unsupported keys then
       we cannot resolve any of them */
    if (key_mapping.source_maps != NULL) {
      struct FullResolutionCtlCb *c = g_new0 (struct FullResolutionCtlCb, 1);
      c->user_callback = callback;
      c->user_data = user_data;
      c->source_map_list = key_mapping.source_maps;
      c->flags = flags;
      c->chained = full_chained;

      _callback = full_resolution_ctl_cb;
      _user_data = c;
      g_list_free (_keys);
      _keys = key_mapping.operation_keys;

      relay_chained = TRUE;
    }
  }

  query_id = grl_media_source_gen_operation_id (source);

  brc = g_new0 (struct BrowseRelayCb, 1);
  brc->chained = relay_chained;
  brc->user_callback = _callback;
  brc->user_data = _user_data;
  brc->use_idle = flags & GRL_RESOLVE_IDLE_RELAY;
  _callback = browse_result_relay_cb;
  _user_data = brc;

  qs = g_new0 (GrlMediaSourceQuerySpec, 1);
  qs->source = g_object_ref (source);
  qs->query_id = query_id;
  qs->query = g_strdup (query);
  qs->keys = _keys;
  qs->skip = skip;
  qs->count = count;
  qs->flags = flags;
  qs->callback = _callback;
  qs->user_data = _user_data;

  /* Save a reference to the operaton spec in the relay-cb's
     user_data so that we can free the spec there when we get
     the last result */
  brc->qspec = qs;

  /* Setup auto-split management if requested */
  if (source->priv->auto_split_threshold > 0 &&
      count > source->priv->auto_split_threshold) {
    GRL_DEBUG ("auto-split: enabled");
    struct AutoSplitCtl *as_ctl = g_new0 (struct AutoSplitCtl, 1);
    as_ctl->count = count;
    as_ctl->threshold = source->priv->auto_split_threshold;
    as_ctl->chunk_requested = as_ctl->threshold;
    as_ctl->chunk_first = TRUE;
    qs->count = as_ctl->chunk_requested;
    brc->auto_split = as_ctl;
    GRL_DEBUG ("auto-split: requesting first chunk (skip=%u, count=%u)",
               qs->skip, qs->count);
  }

  set_operation_ongoing (source, query_id);
  g_idle_add (query_idle, qs);

  return query_id;
}

/**
 * grl_media_source_query_sync:
 * @source: a media source
 * @query: the query to process
 * @keys: (element-type GObject.ParamSpec): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @skip: the number if elements to skip in the query operation
 * @count: the number of elements to retrieve in the query operation
 * @flags: the resolution mode
 * @error: a #GError, or @NULL
 *
 * Execute a specialized query (specific for each provider) on a media
 * repository.
 *
 * This method is synchronous.
 *
 * Returns: (element-type Grl.Media) (transfer full): a #GList with #GrlMedia
 * elements. After use g_object_unref() every element and g_list_free() the
 * list.
 *
 * Since: 0.1.6
 */
GList *
grl_media_source_query_sync (GrlMediaSource *source,
                             const gchar *query,
                             const GList *keys,
                             guint skip,
                             guint count,
                             GrlMetadataResolutionFlags flags,
                             GError **error)
{
  GrlDataSync *ds;
  GList *result;

  ds = g_slice_new0 (GrlDataSync);

  grl_media_source_query (source,
                          query,
                          keys,
                          skip,
                          count,
                          flags,
                          multiple_result_async_cb,
                          ds);

  grl_wait_for_async_operation_complete (ds);

  if (ds->error) {
    if (error) {
      *error = ds->error;
    } else {
      g_error_free (ds->error);
    }
  }

  result = (GList *) ds->data;
  g_slice_free (GrlDataSync, ds);

  return result;
}

/**
 * grl_media_source_metadata:
 * @source: a media source
 * @media: (allow-none): a data transfer object
 * @keys: (element-type GObject.ParamSpec): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @flags: the resolution mode
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * This method is intended to fetch the requested keys of metadata of
 * a given @media to the media source.
 *
 * This method is asynchronous.
 *
 * Returns: the operation identifier
 *
 * Since: 0.1.6
 */
guint
grl_media_source_metadata (GrlMediaSource *source,
                           GrlMedia *media,
                           const GList *keys,
                           GrlMetadataResolutionFlags flags,
                           GrlMediaSourceMetadataCb callback,
                           gpointer user_data)
{
  GrlMediaSourceMetadataCb _callback;
  gpointer _user_data ;
  GList *_keys;
  struct SourceKeyMapList key_mapping;
  GrlMediaSourceMetadataSpec *ms;
  struct MetadataRelayCb *mrc;
  guint metadata_id;

  GRL_DEBUG ("grl_media_source_metadata");

  g_return_val_if_fail (GRL_IS_MEDIA_SOURCE (source), 0);
  g_return_val_if_fail (keys != NULL, 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (source)) &
                        GRL_OP_METADATA, 0);

  /* By default assume we will use the parameters specified by the user */
  _callback = callback;
  _user_data = user_data;
  _keys = g_list_copy ((GList *) keys);

  if (flags & GRL_RESOLVE_FAST_ONLY) {
    grl_metadata_source_filter_slow (GRL_METADATA_SOURCE (source),
                                     &_keys, FALSE);
  }

  if (flags & GRL_RESOLVE_FULL) {
    GRL_DEBUG ("requested full metadata");
    grl_metadata_source_setup_full_resolution_mode (GRL_METADATA_SOURCE (source),
                                                    media, _keys, &key_mapping);

    /* If we do not have a source map for the unsupported keys then
       we cannot resolve any of them */
    if (key_mapping.source_maps != NULL) {
      struct MetadataFullResolutionCtlCb *c =
	g_new0 (struct MetadataFullResolutionCtlCb, 1);
      c->user_callback = callback;
      c->user_data = user_data;
      c->source_map_list = key_mapping.source_maps;
      c->flags = flags;

      _callback = metadata_full_resolution_ctl_cb;
      _user_data = c;
      g_list_free (_keys);
      _keys = key_mapping.operation_keys;
    }
  }

  metadata_id = grl_media_source_gen_operation_id (source);

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */

  mrc = g_new0 (struct MetadataRelayCb, 1);
  mrc->user_callback = _callback;
  mrc->user_data = _user_data;
  _callback = metadata_result_relay_cb;
  _user_data = mrc;

  ms = g_new0 (GrlMediaSourceMetadataSpec, 1);
  ms->source = g_object_ref (source);
  ms->metadata_id = metadata_id;
  ms->keys = _keys; /* It is already a copy */
  ms->flags = flags;
  ms->callback = _callback;
  ms->user_data = _user_data;
  if (!media) {
    /* Special case, NULL media ==> root container */
    ms->media = grl_media_box_new ();
    grl_media_set_id (ms->media, NULL);
  } else {
    ms->media = media;
  }
  g_object_ref (ms->media);

  /* Save a reference to the operaton spec in the relay-cb's
     user_data so that we can free the spec there */
  mrc->spec = ms;

  set_operation_ongoing (source, metadata_id);
  g_idle_add (metadata_idle, ms);

  return metadata_id;
}

/**
 * grl_media_source_metadata_sync:
 * @source: a media source
 * @media: (allow-none): a data transfer object
 * @keys: (element-type GObject.ParamSpec): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @flags: the resolution mode
 * @error: a #GError, or @NULL
 *
 * This method is intended to fetch the requested keys of metadata of
 * a given @media to the media source.
 *
 * This method is synchronous.
 *
 * Returns: (transfer full): a filled #GrlMedia
 *
 * Since: 0.1.6
 */
GrlMedia *
grl_media_source_metadata_sync (GrlMediaSource *source,
                                GrlMedia *media,
                                const GList *keys,
                                GrlMetadataResolutionFlags flags,
                                GError **error)
{
  GrlDataSync *ds;

  ds = g_slice_new0 (GrlDataSync);

  grl_media_source_metadata (source,
                             media,
                             keys,
                             flags,
                             metadata_result_async_cb,
                             ds);

  grl_wait_for_async_operation_complete (ds);

  if (ds->error) {
    if (error) {
      *error = ds->error;
    } else {
      g_error_free (ds->error);
    }
  }

  g_slice_free (GrlDataSync, ds);

  return media;
}

static GrlSupportedOps
grl_media_source_supported_operations (GrlMetadataSource *metadata_source)
{
  GrlSupportedOps caps;
  GrlMediaSource *source;
  GrlMediaSourceClass *media_source_class;
  GrlMetadataSourceClass *metadata_source_class;

  metadata_source_class =
    GRL_METADATA_SOURCE_CLASS (grl_media_source_parent_class);
  source = GRL_MEDIA_SOURCE (metadata_source);
  media_source_class = GRL_MEDIA_SOURCE_GET_CLASS (source);

  caps = metadata_source_class->supported_operations (metadata_source);
  if (media_source_class->browse)
    caps |= GRL_OP_BROWSE;
  if (media_source_class->search)
    caps |= GRL_OP_SEARCH;
  if (media_source_class->query)
    caps |= GRL_OP_QUERY;
  if (media_source_class->metadata)
    caps |= GRL_OP_METADATA;
  if (media_source_class->store)  /* We do not assume GRL_OP_STORE_PARENT */
    caps |= GRL_OP_STORE;
  if (media_source_class->remove)
    caps |= GRL_OP_REMOVE;
  if (media_source_class->test_media_from_uri &&
      media_source_class->media_from_uri)
    caps |= GRL_OP_MEDIA_FROM_URI;
  if (media_source_class->notify_change_start &&
      media_source_class->notify_change_stop)
    caps |= GRL_OP_NOTIFY_CHANGE;

  return caps;
}

/**
 * grl_media_source_cancel:
 * @source: a media source
 * @operation_id: the identifier of the running operation
 *
 * Cancel a running method.
 *
 * Every method has a operation identifier, which is set as parameter in the
 * callback. The running operation can be cancel then.
 *
 * The derived class must implement the cancel vmethod in order to
 * honor the request.
 *
 * Since: 0.1.1
 */
void
grl_media_source_cancel (GrlMediaSource *source, guint operation_id)
{
  GRL_DEBUG ("grl_media_source_cancel");

  g_return_if_fail (GRL_IS_MEDIA_SOURCE (source));

  if (!operation_is_ongoing (source, operation_id)) {
    GRL_DEBUG ("Tried to cancel invalid or already cancelled operation. "
               "Skipping...");
    return;
  }

  /* Mark the operation as finished, if the source does
     not implement cancelation or it did not make it in time, we will
     not emit the results for this operation in any case.
     At any rate, we will not free the operation data until we are sure
     the plugin won't need it any more, which it will tell when it emits
     remaining = 0 (which can happen because it did not cancel the op
     or because it managed to cancel it and is signaling so) */
  set_operation_cancelled (source, operation_id);

  /* If the source provides an implementation for operacion cancelation,
     let's use that to avoid further unnecessary processing in the plugin */
  if (GRL_MEDIA_SOURCE_GET_CLASS (source)->cancel) {
    GRL_MEDIA_SOURCE_GET_CLASS (source)->cancel (source, operation_id);
  }
}

/**
 * grl_media_source_set_operation_data:
 * @source: a media source
 * @operation_id: the identifier of a running operation
 * @data: the data to attach
 *
 * Attach a pointer to the specific operation.
 *
 * Since: 0.1.1
 */
void
grl_media_source_set_operation_data (GrlMediaSource *source,
                                     guint operation_id,
                                     gpointer data)
{
  GRL_DEBUG ("grl_media_source_set_operation_data");
  g_return_if_fail (GRL_IS_MEDIA_SOURCE (source));
  set_operation_data (source, operation_id, data);
}

/**
 * grl_media_source_get_operation_data:
 * @source: a media source
 * @operation_id: the identifier of a running operation
 *
 * Obtains the previously attached data
 *
 * Returns: (transfer none): The previously attached data.
 *
 * Since: 0.1.1
 */
gpointer
grl_media_source_get_operation_data (GrlMediaSource *source,
                                     guint operation_id)
{
  GRL_DEBUG ("grl_media_source_get_operation_data");
  g_return_val_if_fail (GRL_IS_MEDIA_SOURCE (source), NULL);
  return get_operation_data (source, operation_id);
}

/**
 * grl_media_source_get_auto_split_threshold:
 * @source: a media source
 *
 * TBD
 *
 * Returns: the assigned threshold
 *
 * Since: 0.1.1
 */
guint
grl_media_source_get_auto_split_threshold (GrlMediaSource *source)
{
  g_return_val_if_fail (GRL_IS_MEDIA_SOURCE (source), 0);
  return source->priv->auto_split_threshold;
}

/**
 * grl_media_source_set_auto_split_threshold:
 * @source: a media source
 * @threshold: the threshold to request
 *
 * TBD
 *
 * Since: 0.1.1
 */
void
grl_media_source_set_auto_split_threshold (GrlMediaSource *source,
                                           guint threshold)
{
  g_return_if_fail (GRL_IS_MEDIA_SOURCE (source));
  source->priv->auto_split_threshold = threshold;
}

/**
 * grl_media_source_store:
 * @source: a media source
 * @parent: (allow-none): a parent to store the data transfer objects
 * @media: a data transfer object
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Store the @media into the @parent container
 *
 * This method is asynchronous.
 *
 * Since: 0.1.4
 */
void
grl_media_source_store (GrlMediaSource *source,
                        GrlMediaBox *parent,
                        GrlMedia *media,
                        GrlMediaSourceStoreCb callback,
                        gpointer user_data)
{
  GRL_DEBUG ("grl_media_source_store");

  const gchar *title;
  const gchar *url;
  GError *error = NULL;

  g_return_if_fail (GRL_IS_MEDIA_SOURCE (source));
  g_return_if_fail (!parent || GRL_IS_MEDIA_BOX (parent));
  g_return_if_fail (GRL_IS_MEDIA (media));
  g_return_if_fail (callback != NULL);
  g_return_if_fail ((!parent &&
                     grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (source)) &
		     GRL_OP_STORE) ||
		    (parent &&
                     grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (source)) &
		     GRL_OP_STORE_PARENT));

  /* First, check that we have the minimum information we need */
  title = grl_media_get_title (media);
  url = grl_media_get_url (media);

  if (!title) {
    error = g_error_new (GRL_CORE_ERROR,
			 GRL_CORE_ERROR_STORE_FAILED,
			 "Media has no title, cannot store");
  } else if (!url && !GRL_IS_MEDIA_BOX (media)) {
    error = g_error_new (GRL_CORE_ERROR,
			 GRL_CORE_ERROR_STORE_FAILED,
			 "Media has no URL, cannot store");
  }

  /* If we have the info, ask the plugin to store the media */
  if (!error) {
    GrlMediaSourceStoreSpec *ss = g_new0 (GrlMediaSourceStoreSpec, 1);
    ss->source = g_object_ref (source);
    ss->parent = parent ? g_object_ref (parent) : NULL;
    ss->media = g_object_ref (media);
    ss->callback = callback;
    ss->user_data = user_data;

    g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
		     store_idle,
		     ss,
		     store_idle_destroy);
  } else {
    callback (source, parent, media, user_data, error);
    g_error_free (error);
  }
}

/**
 * grl_media_source_store_sync:
 * @source: a media source
 * @parent: (allow-none): a #GrlMediaBox to store the data transfer objects
 * @media: a #GrlMedia data transfer object
 * @error: a #GError, or @NULL
 *
 * Store the @media into the @parent container.
 *
 * This method is synchronous.
 *
 * Since: 0.1.6
 */
void
grl_media_source_store_sync (GrlMediaSource *source,
                             GrlMediaBox *parent,
                             GrlMedia *media,
                             GError **error)
{
  GrlDataSync *ds;

  ds = g_slice_new0 (GrlDataSync);

  grl_media_source_store (source,
                          parent,
                          media,
                          store_async_cb,
                          ds);

  grl_wait_for_async_operation_complete (ds);

  if (ds->error) {
    if (error) {
      *error = ds->error;
    } else {
      g_error_free (ds->error);
    }
  }

  g_slice_free (GrlDataSync, ds);
}

/**
 * grl_media_source_remove:
 * @source: a media source
 * @media: a data transfer object
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Remove a @media from the @source repository.
 *
 * This method is asynchronous.
 *
 * Since: 0.1.4
 */
void
grl_media_source_remove (GrlMediaSource *source,
                         GrlMedia *media,
                         GrlMediaSourceRemoveCb callback,
                         gpointer user_data)
{
  GRL_DEBUG ("grl_media_source_remove");

  const gchar *id;
  GError *error = NULL;

  g_return_if_fail (GRL_IS_MEDIA_SOURCE (source));
  g_return_if_fail (GRL_IS_MEDIA (media));
  g_return_if_fail (callback != NULL);
  g_return_if_fail (grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (source)) &
		    GRL_OP_REMOVE);

  /* First, check that we have the minimum information we need */
  id = grl_media_get_id (media);
  if (!id) {
    error = g_error_new (GRL_CORE_ERROR,
			 GRL_CORE_ERROR_REMOVE_FAILED,
			 "Media has no id, cannot remove");
  }

  if (!error) {
    GrlMediaSourceRemoveSpec *rs = g_new0 (GrlMediaSourceRemoveSpec, 1);
    rs->source = g_object_ref (source);
    rs->media_id = g_strdup (id);
    rs->media = g_object_ref (media);
    rs->callback = callback;
    rs->user_data = user_data;

    g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
		     remove_idle,
		     rs,
		     remove_idle_destroy);
  } else {
    callback (source, media, user_data, error);
    g_error_free (error);
  }
}

/**
 * grl_media_source_remove_sync:
 * @source: a media source
 * @media: a data transfer object
 * @error: a #GError, or @NULL
 *
 * Remove a @media from the @source repository.
 *
 * This method is synchronous.
 *
 * Since: 0.1.6
 */
void
grl_media_source_remove_sync (GrlMediaSource *source,
                              GrlMedia *media,
                              GError **error)
{
  GrlDataSync *ds;

  ds = g_slice_new0 (GrlDataSync);

  grl_media_source_remove (source,
                           media,
                           remove_async_cb,
                           ds);

  grl_wait_for_async_operation_complete (ds);

  if (ds->error) {
    if (error) {
      *error = ds->error;
    } else {
      g_error_free (ds->error);
    }
  }

  g_slice_free (GrlDataSync, ds);
}

/**
 * grl_media_source_test_media_from_uri:
 * @source: a media source
 * @uri: A URI that can be used to identify a media resource
 *
 * Tests whether @source can instantiate a #GrlMedia object representing
 * the media resource exposed at @uri.
 *
 * Returns: %TRUE if it can, %FALSE otherwise.
 *
 * This method is synchronous.
 *
 * Since: 0.1.7
 */
gboolean
grl_media_source_test_media_from_uri (GrlMediaSource *source,
				       const gchar *uri)
{
  GRL_DEBUG ("grl_media_source_test_media_from_uri");

  g_return_val_if_fail (GRL_IS_MEDIA_SOURCE (source), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);

  if (GRL_MEDIA_SOURCE_GET_CLASS (source)->test_media_from_uri) {
    return GRL_MEDIA_SOURCE_GET_CLASS (source)->test_media_from_uri (source,
								     uri);
  } else {
    return FALSE;
  }
}

/**
 * grl_media_source_get_media_from_uri:
 * @source: a media source
 * @uri: A URI that can be used to identify a media resource
 * @keys: A list of keys to resolve
 * @flags: the resolution mode
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Creates an instance of #GrlMedia representing the media resource
 * exposed at @uri.
 * 
 * It is recommended to call grl_media_source_test_media_from_uri() before
 * invoking this to check whether the target source can theoretically do the
 * resolution.
 *
 * This method is asynchronous.
 *
 * Since: 0.1.7
 */
void
grl_media_source_get_media_from_uri (GrlMediaSource *source,
				     const gchar *uri,
				     const GList *keys,
				     GrlMetadataResolutionFlags flags,
				     GrlMediaSourceMetadataCb callback,
				     gpointer user_data)
{
  GRL_DEBUG ("grl_media_source_get_media_from_uri");

  GList *_keys;
  GrlMediaSourceMediaFromUriSpec *mfus;
  struct MediaFromUriRelayCb *mfsrc;

  g_return_if_fail (GRL_IS_MEDIA_SOURCE (source));
  g_return_if_fail (uri != NULL);
  g_return_if_fail (keys != NULL);
  g_return_if_fail (callback != NULL);
  g_return_if_fail (grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (source)) &
		    GRL_OP_MEDIA_FROM_URI);

  _keys = g_list_copy ((GList *) keys);
  if (flags & GRL_RESOLVE_FAST_ONLY) {
    grl_metadata_source_filter_slow (GRL_METADATA_SOURCE (source),
                                     &_keys, FALSE);
  }

  /* We cannot prepare for full resolution yet because we don't
     have a GrlMedia t operate with.
     TODO: full resolution could be added in the relay calback
     when we get the GrlMedia object */

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */

  mfsrc = g_new0 (struct MediaFromUriRelayCb, 1);
  mfsrc->user_callback = callback;
  mfsrc->user_data = user_data;

  mfus = g_new0 (GrlMediaSourceMediaFromUriSpec, 1);
  mfus->source = g_object_ref (source);
  mfus->uri = g_strdup (uri);
  mfus->keys = _keys;
  mfus->flags = flags;
  mfus->callback = media_from_uri_relay_cb;
  mfus->user_data = mfsrc;

  /* Save a reference to the operaton spec in the relay-cb's
     user_data so that we can free the spec there */
  mfsrc->spec = mfus;

  g_idle_add (media_from_uri_idle, mfus);
}

/**
 * grl_media_source_get_media_from_uri_sync:
 * @source: a media source
 * @uri: A URI that can be used to identify a media resource
 * @keys: A list of keys to resolve
 * @flags: the resolution mode
 * @error: a #GError, or @NULL
 *
 * Creates an instance of #GrlMedia representing the media resource
 * exposed at @uri.
 * 
 * It is recommended to call grl_media_source_test_media_from_uri() before
 * invoking this to check whether the target source can theoretically do the
 * resolution.
 *
 * This method is synchronous.
 *
 * Since: 0.1.8
 */
GrlMedia *
grl_media_source_get_media_from_uri_sync (GrlMediaSource *source,
                                          const gchar *uri,
                                          const GList *keys,
                                          GrlMetadataResolutionFlags flags,
                                          GError **error)
{
  GrlDataSync *ds;
  GrlMedia *result;

  ds = g_slice_new0 (GrlDataSync);

  grl_media_source_get_media_from_uri (source,
                                       uri,
                                       keys,
                                       flags,
                                       metadata_result_async_cb,
                                       ds);

  grl_wait_for_async_operation_complete (ds);

  if (ds->error) {
    if (error) {
      *error = ds->error;
    } else {
      g_error_free (ds->error);
    }
  }

  result = (GrlMedia *) ds->data;
  g_slice_free (GrlDataSync, ds);

  return result;
}

/**
 * grl_media_source_notify_change_start:
 * @source: a media source
 * @error: a #GError, or @NULL
 *
 * Starts emitting ::content-changed signals when @source discovers changes in
 * the content. This instructs @source to setup the machinery needed to be aware
 * of changes in the content.
 *
 * Returns: @TRUE if initialization has succeed.
 */
gboolean
grl_media_source_notify_change_start (GrlMediaSource *source,
                                      GError **error)
{
  g_return_val_if_fail (GRL_IS_MEDIA_SOURCE (source), FALSE);
  g_return_val_if_fail (grl_media_source_supported_operations (GRL_METADATA_SOURCE (source)) &
                        GRL_OP_NOTIFY_CHANGE, FALSE);

  return GRL_MEDIA_SOURCE_GET_CLASS (source)->notify_change_start (source,
                                                                   error);
}

/**
 * grl_media_source_notify_change_stop:
 * @source: a media source
 * @error: a #GError, or @NULL
 *
 * This will drop emission of ::content-changed signals from @source. When this
 * is done @source should stop the machinery required for it to track changes in
 * the content.
 *
 * Returns: @TRUE if stop has succeed.
 */
gboolean
grl_media_source_notify_change_stop (GrlMediaSource *source,
                                     GError **error)
{
  g_return_val_if_fail (GRL_IS_MEDIA_SOURCE (source), FALSE);
  g_return_val_if_fail (grl_media_source_supported_operations (GRL_METADATA_SOURCE (source)) &
                        GRL_OP_NOTIFY_CHANGE, FALSE);

  return GRL_MEDIA_SOURCE_GET_CLASS (source)->notify_change_stop (source,
                                                                  error);
}

/**
 * grl_media_source_notify_change:
 * @source: a media source
 * @media: (allow-none): the media which has changed
 * @change_type: the type of change
 * @location_unknown: if change has happpened in @media or any descendant
 *
 * Emits "content-changed" signal to notify subscribers that a change ocurred
 * in @source.
 *
 * See GrlMediaSource::content-changed signal.
 *
 * <note>
 *  <para>
 *    This function is intended to be used only by plugins.
 *  </para>
 * </note>
 */
void grl_media_source_notify_change (GrlMediaSource *source,
                                     GrlMedia *media,
                                     GrlMediaSourceChangeType change_type,
                                     gboolean location_unknown)
{
  g_return_if_fail (GRL_IS_MEDIA_SOURCE (source));
  g_return_if_fail (!media || GRL_IS_MEDIA (media));

  g_signal_emit (source,
                 registry_signals[SIG_CONTENT_CHANGED],
                 0,
                 media,
                 change_type,
                 location_unknown);
}
