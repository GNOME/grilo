/*
 * Copyright (C) 2010-2012 Igalia S.L.
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
 * SECTION:grl-source
 * @short_description: Abstract base class for sources
 * @see_also: #GrlPlugin, #GrlSource, #GrlMedia
 *
 * GrlSource is the abstract base class needed to construct a source providing
 * multimedia information that can be used in a Grilo application.
 *
 * The sources fetch information from different online or local
 * databases and store them in the #GrlMedia.
 */

#include "grl-source.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "grl-operation.h"
#include "grl-operation-priv.h"
#include "grl-marshal.h"
#include "grl-type-builtins.h"
#include "grl-sync-priv.h"
#include "grl-registry.h"
#include "grl-error.h"
#include "grl-log.h"
#include "data/grl-media.h"

#include <glib/gi18n-lib.h>
#include <string.h>

#define GRL_LOG_DOMAIN_DEFAULT  source_log_domain
GRL_LOG_DOMAIN(source_log_domain);

enum {
  PROP_0,
  PROP_ID,
  PROP_NAME,
  PROP_DESC,
  PROP_ICON,
  PROP_PLUGIN,
  PROP_RANK,
  PROP_AUTO_SPLIT_THRESHOLD,
  PROP_SUPPORTED_MEDIA,
  PROP_SOURCE_TAGS
};

enum {
  SIG_CONTENT_CHANGED,
  SIG_LAST
};

static gint source_signals[SIG_LAST];

typedef void (*MediaDecorateCb) (GrlMedia *media,
                                 gpointer user_data,
                                 const GError *error);

struct _GrlSourcePrivate {
  gchar *id;
  gchar *name;
  gchar *desc;
  gint rank;
  GrlSupportedMedia supported_media;
  guint auto_split_threshold;
  GrlPlugin *plugin;
  GIcon *icon;
  GPtrArray *tags;
};

typedef struct {
  GrlMedia *media;
  gboolean is_ready;
  gint remaining;
  GError *error;
} QueueElement;

typedef struct {
  GrlSource *source;
  GList *required_keys;
  gboolean being_queried;
} MapNode;

struct AutoSplitCtl {
  gboolean chunk_first;
  guint chunk_requested;
  guint chunk_consumed;
  guint threshold;
  guint count;
  guint total_remaining;
  guint chunk_remaining;
};

struct OperationState {
  GrlSource *source;
  guint operation_id;
  gboolean cancelled;
  gboolean completed;
  gboolean started;
};

struct ResolveRelayCb {
  GrlSource *source;
  GrlSupportedOps operation_type;
  guint operation_id;
  GrlMedia *media;
  GList *keys;
  GrlOperationOptions *options;
  GrlSourceResolveCb user_callback;
  gpointer user_data;
  GHashTable *map;
  GHashTable *resolve_specs;
  GList *specs_to_invoke;
  gboolean cancel_invoked;
  GError *error;
  union {
    GrlSourceResolveSpec *res;
    GrlSourceMediaFromUriSpec *mfu;
  } spec;
};

struct BrowseRelayCb {
  GrlSource *source;
  GrlSupportedOps operation_type;
  guint operation_id;
  GList *keys;
  GrlOperationOptions *options;
  GrlSourceResultCb user_callback;
  gpointer user_data;
  union {
    GrlSourceBrowseSpec *browse;
    GrlSourceSearchSpec *search;
    GrlSourceQuerySpec *query;
  } spec;
  GQueue *queue;
  gboolean dispatcher_running;
  struct AutoSplitCtl *auto_split;
};

struct RemoveRelayCb {
  GrlSource *source;
  GrlMedia *media;
  GrlSourceRemoveCb user_callback;
  gpointer user_data;
  GrlSourceRemoveSpec *spec;
  GError *error;
};

struct StoreRelayCb {
  GrlWriteFlags flags;
  GrlSourceStoreCb user_callback;
  gpointer user_data;
  GrlSourceStoreSpec *spec;
};

struct StoreMetadataRelayCb {
  GrlSource *source;
  GrlMedia *media;
  GHashTable *map;
  GList *use_sources;
  GList *failed_keys;
  GList *specs;
  GrlSourceStoreCb user_callback;
  gpointer user_data;
};

struct ResolveFullResolutionCtlCb {
  GrlSourceResolveCb user_callback;
  gpointer user_data;
  GList *keys;
  GrlResolutionFlags flags;
  guint operation_id;
};

struct ResolveFullResolutionDoneCb {
  GrlSourceResolveCb user_callback;
  gpointer user_data;
  GHashTable *pending_callbacks;
  gboolean cancelled;
  GrlSource *source;
  struct ResolveFullResolutionCtlCb *ctl_info;;
};

struct MediaDecorateData {
  GrlSource *source;
  guint operation_id;
  GHashTable *pending_callbacks;
  MediaDecorateCb callback;
  gboolean cancelled;
  gpointer user_data;
};

static void grl_source_finalize (GObject *plugin);

static void grl_source_dispose (GObject *objct);

static void grl_source_get_property (GObject *plugin,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec);

static void grl_source_set_property (GObject *object,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec);

static gboolean browse_idle (gpointer user_data);

static gboolean search_idle (gpointer user_data);

static gboolean query_idle (gpointer user_data);

static void run_store_metadata (GrlSource *source,
                                GrlMedia *media,
                                GList *keys,
                                GrlWriteFlags flags,
                                GrlSourceStoreCb callback,
                                gpointer user_data);

static void map_list_nodes_free (GList *nodes);

static void resolve_result_relay_cb (GrlSource *source,
                                     guint operation_id,
                                     GrlMedia *media,
                                     gpointer user_data,
                                     const GError *error);

static gboolean resolve_idle (gpointer user_data);

static gboolean resolve_all_done (gpointer user_data);

static void source_cancel_cb (struct OperationState *op_state);

/* ================ GrlSource GObject ================ */

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (GrlSource,
                                  grl_source,
                                  G_TYPE_OBJECT,
                                  G_ADD_PRIVATE (GrlSource));

static void
grl_source_class_init (GrlSourceClass *source_class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (source_class);

  gobject_class->dispose = grl_source_dispose;
  gobject_class->finalize = grl_source_finalize;
  gobject_class->set_property = grl_source_set_property;
  gobject_class->get_property = grl_source_get_property;

  /**
   * GrlSource:source-id:
   *
   * The identifier of the source.
   *
   * Since: 0.2.0
   */
  g_object_class_install_property (gobject_class,
                                   PROP_ID,
                                   g_param_spec_string ("source-id",
                                                        "Source identifier",
                                                        "The identifier of the source",
                                                        "",
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  /**
   * GrlSource:source-name:
   *
   * The name of the source.
   *
   * Since: 0.2.0
   */
  g_object_class_install_property (gobject_class,
                                   PROP_NAME,
                                   g_param_spec_string ("source-name",
                                                        "Source name",
                                                        "The name of the source",
                                                        "",
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  /**
   * GrlSource:source-desc:
   *
   * A description of the source
   *
   * Since: 0.2.0
   */
  g_object_class_install_property (gobject_class,
                                   PROP_DESC,
                                   g_param_spec_string ("source-desc",
                                                        "Source description",
                                                        "A description of the source",
                                                        "",
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  /**
   * GrlSource:source-icon:
   *
   * #GIcon representing the source
   *
   * Since: 0.2.8
   */
  g_object_class_install_property (gobject_class,
                                   PROP_ICON,
                                   g_param_spec_object ("source-icon",
                                                        "Source icon",
                                                        "Icon representing the source",
                                                        G_TYPE_ICON,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  /**
   * GrlSource:plugin:
   *
   * Plugin the source belongs to
   *
   * Since: 0.2.0
   */
  g_object_class_install_property (gobject_class,
                                   PROP_PLUGIN,
                                   g_param_spec_object ("plugin",
                                                        "Plugin",
                                                        "Plugin source belongs to",
                                                        GRL_TYPE_PLUGIN,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  /**
   * GrlSource:rank:
   *
   * Source rank
   *
   * Since: 0.2.0
   */
  g_object_class_install_property (gobject_class,
                                   PROP_RANK,
                                   g_param_spec_int ("rank",
                                                     "Rank",
                                                     "Source rank",
                                                     G_MININT,
                                                     G_MAXINT,
                                                     GRL_RANK_DEFAULT,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT |
                                                     G_PARAM_STATIC_STRINGS));
  /**
   * GrlSource:auto-split-threshold:
   *
   * Transparently split queries with count requests
   * bigger than a certain threshold into smaller queries.
   *
   * Since: 0.2.0
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
   * GrlSource:supported-media:
   *
   * List of supported media types by this source.
   *
   * Since: 0.2.3
   */
  g_object_class_install_property (gobject_class,
                                   PROP_SUPPORTED_MEDIA,
                                   g_param_spec_flags ("supported-media",
                                                       "Supported media",
                                                       "List of supported media types",
                                                       GRL_TYPE_SUPPORTED_MEDIA,
                                                       GRL_SUPPORTED_MEDIA_ALL,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT |
                                                       G_PARAM_STATIC_STRINGS));

  /**
   * GrlSource:source-tags:
   *
   * A string array of tags relevant this source.
   *
   * The tags are arbitrary, and applications should just pass over the tags
   * it does not understand. Applications would usually use this to either
   * group sources together, or hide certain sources: a radio application
   * would filter for %GRL_MEDIA_TYPE_AUDIO in GrlSource::supported-media as
   * well as "radio" being listed in the tags.
   *
   * To avoid irrelevant content being listed in applications, sources
   * such as generic video sites should not be tagged as "cinema" or
   * "tv" as they contain a lot of content that's not either of those.
   *
   * This is a list of commonly used values:
   *
   * - "cinema", or "tv"
   *   The content served is from cinema or TV sources. For example, a
   *   source for movie trailers would select the former, a source for
   *   streaming live TV would select the latter.
   *
   * - "radio"
   *   The content served is from streaming radios.
   *
   * - "music"
   *   The content served is music, for example, music stores such as
   *   Jamendo or Magnatune.
   *
   * - "country:country-code"
   *   The content is mostly relevant to users from a particular country,
   *   such as a national broadcaster. For example, BBC content would be
   *   tagged as "country:uk". Country codes should be an ISO-639-1 or
   *   ISO-639-2 code.
   *
   * - "protocol:protocol-name"
   *   The content browsing or searching uses a particular protocol, such
   *   as DLNA/UPnP or DMAP/DAAP. This makes it easier to whitelist or
   *   blacklist sources rather than matching the implementation specific
   *   source ID. Examples are "protocol:dlna" and "protocol:dmap".
   *
   * - "localhost", or "localuser"
   *   The content is served from the machine the application is running on,
   *   or by an application the user is running. Applications might choose to
   *   avoid showing the user's own data in their interfaces, or integrate it
   *   in the user's local collection.
   *
   *   "net:local", or "net:internet"
   *   The source requires a connection to the local network, or a connection
   *   to the Internet. Sources with those tags will be automatically hidden
   *   from the application's reach when such networks aren't available, or
   *   we're not connected to a network.
   *
   *   "net:plaintext"
   *   The source makes requests over plain text, non-encrypted, network channels,
   *   such as using HTTP to do searches or lookups. Applications would usually
   *   disable those by default, so that privacy is respected by default, and no
   *   data is leaked unintentionally.
   *
   * Since: 0.2.10
   */
  g_object_class_install_property (gobject_class,
                                   PROP_SOURCE_TAGS,
                                   g_param_spec_boxed ("source-tags",
                                                       "Tags",
                                                       "String array of tags relevant this source",
                                                       G_TYPE_STRV,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT |
                                                       G_PARAM_STATIC_STRINGS));

  /**
   * GrlSource::content-changed:
   * @source: source that has changed
   * @changed_medias: (element-type GrlMedia): a #GPtrArray with the medias
   * that changed or a common ancestor of them of type #GrlMedia.
   * @change_type: the kind of change that ocurred
   * @location_unknown: @TRUE if the change happened in @media itself or in one
   * of its direct children (when @media is a #GrlMedia). @FALSE otherwise
   *
   * Signals that the content in the source has changed. @changed_medias is the
   * list of elements that have changed. Usually these medias are of type
   * #GrlMedia container, meaning that the content of that container has changed.
   *
   * If @location_unknown is @TRUE it means the source cannot establish where the
   * change happened: could be either in the container, in any child, or in any other
   * descendant of the container in the hierarchy.
   *
   * Both @change_type and @location_unknown are applied to all elements in the
   * list.
   *
   * For the cases where the source can only signal that a change happened, but
   * not where, it would use a list with the the root container (@NULL id) and set
   * location_unknown as @TRUE.
   *
   * Since: 0.2.0
   */
  source_signals[SIG_CONTENT_CHANGED] =
    g_signal_new("content-changed",
                 G_TYPE_FROM_CLASS (gobject_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 0,
                 NULL,
                 NULL,
                 grl_marshal_VOID__BOXED_ENUM_BOOLEAN,
                 G_TYPE_NONE,
                 3,
                 G_TYPE_PTR_ARRAY,
                 GRL_TYPE_SOURCE_CHANGE_TYPE,
                 G_TYPE_BOOLEAN);
}

static void
grl_source_init (GrlSource *source)
{
  source->priv = grl_source_get_instance_private (source);
  source->priv->tags = g_ptr_array_new_with_free_func (g_free);
}

static void
grl_source_dispose (GObject *object)
{
  GrlSource *source = GRL_SOURCE (object);

  g_clear_object (&source->priv->plugin);

  G_OBJECT_CLASS (grl_source_parent_class)->dispose (object);
}

static void
grl_source_finalize (GObject *object)
{
  GrlSource *source = GRL_SOURCE (object);

  g_clear_object (&source->priv->icon);
  g_clear_pointer (&source->priv->tags, g_ptr_array_unref);
  g_free (source->priv->id);
  g_free (source->priv->name);
  g_free (source->priv->desc);

  G_OBJECT_CLASS (grl_source_parent_class)->finalize (object);
}

static void
set_string_property (gchar **property, const GValue *value)
{
  g_clear_pointer (property, g_free);
  *property = g_value_dup_string (value);
}

static void
grl_source_set_tags (GrlSource   *source,
                     const char **strv)
{
  guint i;

  g_ptr_array_set_size (source->priv->tags, 0);
  if (strv == NULL)
    return;

  for (i = 0; strv[i] != NULL; i++)
    g_ptr_array_add (source->priv->tags, g_strdup (strv[i]));
  g_ptr_array_add (source->priv->tags, NULL);
}

static void
grl_source_set_property (GObject *object,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *pspec)
{
  GrlSource *source = GRL_SOURCE (object);

  switch (prop_id) {
  case PROP_ID:
    set_string_property (&source->priv->id, value);
    break;
  case PROP_NAME:
    set_string_property (&source->priv->name, value);
    break;
  case PROP_DESC:
    set_string_property (&source->priv->desc, value);
    break;
  case PROP_ICON:
    g_clear_object (&source->priv->icon);
    source->priv->icon = g_value_dup_object (value);
    break;
  case PROP_PLUGIN:
    g_clear_object (&source->priv->plugin);
    source->priv->plugin = g_value_dup_object (value);
    break;
  case PROP_RANK:
    source->priv->rank = g_value_get_int (value);
    break;
  case PROP_AUTO_SPLIT_THRESHOLD:
    source->priv->auto_split_threshold = g_value_get_uint (value);
    break;
  case PROP_SUPPORTED_MEDIA:
    source->priv->supported_media = g_value_get_flags (value);
    break;
  case PROP_SOURCE_TAGS:
    grl_source_set_tags (source, g_value_get_boxed (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (source, prop_id, pspec);
    break;
  }
}

static void
grl_source_get_property (GObject *object,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *pspec)
{
  GrlSource *source;

  source = GRL_SOURCE (object);

  switch (prop_id) {
  case PROP_ID:
    g_value_set_string (value, source->priv->id);
    break;
  case PROP_NAME:
    g_value_set_string (value, source->priv->name);
    break;
  case PROP_DESC:
    g_value_set_string (value, source->priv->desc);
    break;
  case PROP_ICON:
    g_value_set_object (value, source->priv->icon);
    break;
  case PROP_PLUGIN:
    g_value_set_object (value, source->priv->plugin);
    break;
  case PROP_RANK:
    g_value_set_int (value, source->priv->rank);
    break;
  case PROP_AUTO_SPLIT_THRESHOLD:
    g_value_set_uint (value, source->priv->auto_split_threshold);
    break;
  case PROP_SUPPORTED_MEDIA:
    g_value_set_flags (value, source->priv->supported_media);
    break;
  case PROP_SOURCE_TAGS:
    g_value_set_boxed (value, source->priv->tags->pdata);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (source, prop_id, pspec);
    break;
  }
}

/* ================ Utilities ================ */

static void
operation_state_free (struct OperationState *op_state)
{
  g_object_unref (op_state->source);
  g_free (op_state);
}

/*
 * This method will _intersect two key lists_:
 *
 * @keys_to_filter: user provided set we want to filter leaving only
 * the keys that intersects with the @source_keys set.
 *
 * @source_keys: the %GrlSource<!-- -->'s key set if
 * @return_filtered is %TRUE a copy of the filtered set *complement*
 * will be returned (a list of the filtered out keys).
 */
static GList *
filter_key_list (GrlSource *source,
                 GList **keys_to_filter,
                 gboolean return_filtered,
                 GList *source_keys)
{
  GList *iter_keys, *found;
  GList *in_source = NULL;
  GList *out_source = NULL;

  for (iter_keys = *keys_to_filter;
       iter_keys;
       iter_keys = g_list_next (iter_keys)) {
    found = g_list_find (source_keys, iter_keys->data);
    if (found) {
      in_source = g_list_prepend (in_source, iter_keys->data);
    } else {
      if (return_filtered) {
        out_source = g_list_prepend (out_source, iter_keys->data);
      }
    }
  }

  g_list_free (*keys_to_filter);
  *keys_to_filter = g_list_reverse (in_source);

  return g_list_reverse (out_source);
}

static GList *
filter_known_keys (GrlMedia *media,
                   GList *keys)
{
  GList *unknown_keys = NULL;
  GList *k;

  if (!media) {
    return g_list_copy (keys);
  }

  for (k = keys; k; k = g_list_next (k)) {
    if (!grl_data_has_key (GRL_DATA (media),
                           GRLPOINTER_TO_KEYID (k->data))) {
      unknown_keys = g_list_prepend (unknown_keys, k->data);
    }
  }

  return unknown_keys;
}

/*
 * Removes all keys from @keys that can't be resolved by any of the sources in
 * @sourcelist.
 */
static GList *
filter_unresolvable_keys (GList *sourcelist, GList **keys)
{
  GList *each_key;
  GList *delete_key;
  GList *each_source;
  gboolean supported;

  each_key = *keys;
  while (each_key) {
    supported = FALSE;

    for (each_source = sourcelist;
         each_source;
         each_source = g_list_next (each_source)) {
      if (g_list_find ((GList *) grl_source_supported_keys (each_source->data),
                       each_key->data)) {
        supported = TRUE;
        break;
      }
    }
    if (!supported) {
      delete_key = each_key;
      each_key = g_list_next (each_key);
      *keys = g_list_delete_link (*keys, delete_key);
    } else {
      each_key = g_list_next (each_key);
    }
  }

  return *keys;
}

/*
 * filter_supported:
 * @source: a source
 * @keys: (element-type GrlKeyID) (transfer container) (allow-none) (inout):
 * the list of keys to filter out
 * @return_filtered: if %TRUE the return value shall be a new list with
 * the unsupported keys
 *
 * Compares the received @keys list with the supported key list by the
 * @source, and deletes those keys which are not supported.
 *
 * Returns: (element-type GrlKeyID) (transfer container):
 * if @return_filtered is %TRUE will return the list of removed keys;
 * otherwise %NULL
 */
static GList *
filter_supported (GrlSource *source,
                  GList **keys,
                  gboolean return_filtered)
{
  const GList *supported_keys;

  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  supported_keys = grl_source_supported_keys (source);

  return filter_key_list (source, keys, return_filtered, (GList *) supported_keys);
}

/*
 * filter_slow:
 * @source: a source
 * @keys: (element-type GrlKeyID) (transfer container) (allow-none) (inout):
 * the list of keys to filter out
 * @return_filtered: if %TRUE the return value shall be a new list with
 * the slow keys
 *
 * This function does the opposite of other filter functions: removes the slow
 * keys from @keys. If @return_filtered is %TRUE the removed slow keys are
 * returned in a new list.
 *
 * Returns: (element-type GrlKeyID) (transfer container): if
 * @return_filtered is %TRUE will return the list of slow keys; otherwise
 * %NULL
 */
static GList *
filter_slow (GrlSource *source,
             GList **keys,
             gboolean return_filtered)
{
  const GList *slow_keys;
  GList *fastest_keys, *tmp;

  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  slow_keys = grl_source_slow_keys (source);

  /* Note that we want to do the opposite */
  fastest_keys = filter_key_list (source, keys, TRUE, (GList *) slow_keys);
  tmp = *keys;
  *keys = fastest_keys;

  if (!return_filtered) {
    g_list_free (tmp);
    return NULL;
  } else {
    return tmp;
  }
}

/*
 * filter_writable:
 * @source: a source
 * @keys: (element-type GrlKeyID) (transfer container) (allow-none) (inout):
 * the list of keys to filter out
 * @return_filtered: if %TRUE the return value shall be a new list with
 * the non-writable keys
 *
 * Similar to filter_supported() but applied to the writable keys in
 * grl_source_writable_keys().
 *
 * Filter the @keys list keeping only those keys that are writtable in
 * @source. If @return_filtered is %TRUE then the removed keys are returned in a
 * new list.
 *
 * Returns: (element-type GrlKeyID) (transfer container):
 * if @return_filtered is %TRUE will return the list of non-writtable keys;
 * otherwise %NULL
 */
static GList *
filter_writable (GrlSource *source,
                            GList **keys,
                            gboolean return_filtered)
{
  const GList *writable_keys;

  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);
  g_return_val_if_fail (keys != NULL, NULL);

  writable_keys = grl_source_writable_keys (source);

  return filter_key_list (source, keys, return_filtered, (GList *) writable_keys);
}

/*
 * Operation states:
 * - started: The operation has been invoked, but not started (plugin has
 *            not been invoked) yet.
 * - finished: We have already emitted the last result to the user
 * - completed: We have already received the last result in the relay cb
 *              (If it is finished it is also completed).
 * - cancelled: Operation valid (not finished) but was cancelled.
 * - ongoing: if the operation is valid (not finished) and not cancelled.
 */


/*
 * operation_set_started:
 *
 * Sets operation as started (we have invoked the operation in the plugin).
 **/
static void
operation_set_started (guint operation_id)
{
  struct OperationState *op_state;

  GRL_DEBUG ("%s (%d)", __FUNCTION__, operation_id);

  op_state = grl_operation_get_private_data (operation_id);

  if (op_state) {
    op_state->started = TRUE;
  }
}

/*
 * operation_is_started:
 *
 * Checks if operation has been started (the operation in plugin has been
 * invoked).
 **/
G_GNUC_UNUSED static gboolean
operation_is_started (guint operation_id)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (operation_id);

  return op_state && op_state->started;
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
 * operation_is_finished:
 *
 * Checks if operation is finished (we have already emitted the last
 * result to the user).
 */
G_GNUC_UNUSED static gboolean
operation_is_finished (guint operation_id)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (operation_id);

  return op_state == NULL;
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
operation_set_ongoing (GrlSource *source, guint operation_id)
{
  struct OperationState *op_state;

  GRL_DEBUG ("%s (%d)", __FUNCTION__, operation_id);

  op_state = g_new0 (struct OperationState, 1);
  op_state->source = g_object_ref (source);
  op_state->operation_id = operation_id;

  grl_operation_set_private_data (operation_id,
                                  op_state,
                                  (GrlOperationCancelCb) source_cancel_cb,
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
source_cancel_cb (struct OperationState *op_state)
{
  GrlSource *source = op_state->source;

  if (!operation_is_ongoing (op_state->operation_id)) {
    GRL_DEBUG ("Tried to cancel invalid or already cancelled operation. "
               "Skipping...");
    return;
  }

  /* Mark the operation as finished, if the source does not implement
     cancellation or it did not make it in time, we will not emit the results
     for this operation in any case.  At any rate, we will not free the
     operation data until we are sure the plugin won't need it any more. In the
     case of operations dealing with multiple results, like browse() or
     search(), this will happen when it emits remaining = 0 (which can be
     because it did not cancel the op or because it managed to cancel it and is
     signaling so) */
  operation_set_cancelled (op_state->operation_id);

  /* If the source provides an implementation for operation cancellation,
     let's use that to avoid further unnecessary processing in the plugin */
  if (GRL_SOURCE_GET_CLASS (source)->cancel) {
    GRL_SOURCE_GET_CLASS (source)->cancel (source,
                                           op_state->operation_id);
  }
}

static void
cancel_resolve (gpointer source, gpointer operation_id, gpointer user_data)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (GPOINTER_TO_UINT (operation_id));
  if (op_state) {
    source_cancel_cb (op_state);
  }
}

static void
resolve_spec_free (GrlSourceResolveSpec *spec)
{
  g_object_unref (spec->source);
  g_object_unref (spec->media);
  g_object_unref (spec->options);
  g_list_free (spec->keys);
  g_free (spec);
}

static void
browse_relay_spec_free (struct BrowseRelayCb *brc)
{
  switch (brc->operation_type) {
  case GRL_OP_BROWSE:
    g_object_unref (brc->spec.browse->source);
    g_object_unref (brc->spec.browse->container);
    g_object_unref (brc->spec.browse->options);
    g_free (brc->spec.browse);
    break;
  case GRL_OP_SEARCH:
    g_object_unref (brc->spec.search->source);
    g_object_unref (brc->spec.search->options);
    g_free (brc->spec.search->text);
    g_free (brc->spec.search);
    break;
  case GRL_OP_QUERY:
    g_object_unref (brc->spec.query->source);
    g_object_unref (brc->spec.query->options);
    g_free (brc->spec.query->query);
    g_free (brc->spec.query);
    break;
  default:
    g_assert_not_reached ();
    break;
  }
}

static void
media_from_uri_spec_free (GrlSourceMediaFromUriSpec *spec)
{
  g_object_unref (spec->source);
  g_object_unref (spec->options);
  g_free (spec->uri);
  g_free (spec);
}

static void
store_spec_free (GrlSourceStoreSpec *spec)
{
  g_object_unref (spec->source);
  g_object_unref (spec->media);
  if (spec->parent) {
    g_object_unref (spec->parent);
  }
  g_free (spec);
}

static void
store_metadata_spec_free (GrlSourceStoreMetadataSpec *spec)
{
  g_object_unref (spec->source);
  g_object_unref (spec->media);
  g_free (spec);
}

static void
resolve_relay_free (struct ResolveRelayCb *rrc)
{
  GHashTableIter iter;
  gpointer value;

  g_object_unref (rrc->source);
  g_clear_object(&rrc->media);
  g_clear_error (&rrc->error);
  g_object_unref (rrc->options);
  g_list_free (rrc->keys);

  if (rrc->map) {
    g_hash_table_iter_init (&iter, rrc->map);
    while (g_hash_table_iter_next (&iter, NULL, &value)) {
      map_list_nodes_free ((GList *) value);
    }
    g_hash_table_unref (rrc->map);
  }
  g_clear_pointer (&rrc->resolve_specs, g_hash_table_unref);

  g_slice_free (struct ResolveRelayCb, rrc);
}

static void
browse_relay_free (struct BrowseRelayCb *brc)
{
  g_object_unref (brc->source);
  g_object_unref (brc->options);
  g_list_free (brc->keys);
  if (brc->auto_split) {
    g_slice_free (struct AutoSplitCtl, brc->auto_split);
  }
  g_clear_pointer (&brc->queue, g_queue_free);

  g_slice_free (struct BrowseRelayCb, brc);
}

static void
remove_relay_free (struct RemoveRelayCb *rrc)
{
  g_object_unref (rrc->source);
  g_object_unref (rrc->media);
  if (rrc->spec) {
    g_object_unref (rrc->spec->source);
    g_object_unref (rrc->spec->media);
    g_free (rrc->spec->media_id);
    g_free (rrc->spec);
  }
  g_slice_free (struct RemoveRelayCb, rrc);
}

static void
store_relay_free (struct StoreRelayCb *src)
{
  g_slice_free (struct StoreRelayCb, src);
}

static void
store_metadata_relay_free (struct StoreMetadataRelayCb *smrc)
{
  g_object_unref (smrc->source);
  g_object_unref (smrc->media);
  g_list_free (smrc->failed_keys);
  g_hash_table_unref (smrc->map);
  g_list_free (smrc->use_sources);
  g_list_free_full (smrc->specs, (GDestroyNotify) store_metadata_spec_free);

  g_slice_free (struct StoreMetadataRelayCb, smrc);
}

/*
 * Returns a list of all the keys that are in deps but are not defined in data
 */
static GList *
missing_in_data (GrlData *data, const GList *deps)
{
  GList *iter, *result = NULL;
  GRL_DEBUG ("missing_in_data");

  if (!data)
    return g_list_copy ((GList *) deps);

  for (iter = (GList *)deps; iter; iter = g_list_next (iter)) {
    if (!grl_data_has_key (data, GRLPOINTER_TO_KEYID (iter->data)))
      result = g_list_append (result, iter->data);
  }

  return result;
}

/*
 * TRUE iff source supports all those keys
 */
static gboolean
source_supports (GrlSource *source,
                 const GList *keys)
{
  const GList *iter;
  GList *supported;

  supported = (GList *) grl_source_supported_keys (source);

  for (iter = keys; iter; iter = g_list_next (iter)) {
    if (!g_list_find (supported, iter->data)) {
      return FALSE;
    }
  }
  return TRUE;
}

/*
 * Find the source that should be queried to add @key to @media.
 * If @additional_keys is provided, the result may include sources that need
 * more metadata to be present in @media, the keys corresponding to that
 * metadata will be put in @additional_keys.
 * If @additional_keys is NULL, will only consider sources that can resolve
 * @keys immediately
 *
 * If @main_source_is_only_resolver is TRUE and @additional_keys is not @NULL,
 * only additional keys that can be resolved directly by @source will be
 * considered. Sources that need other additional keys will not be put in the
 * returned list.
 *
 * @source will never be considered as additional source.
 *
 * @source and @additional_keys may not be @NULL if
 * @main_source_is_only_resolver is @TRUE.
 *
 * Assumes @key is not already in @media.
 */
static GrlSource *
get_additional_source_for_key (GrlSource *source,
                               GList *sources,
                               GrlMedia *media,
                               GrlKeyID key,
                               GList **additional_keys,
                               gboolean main_source_is_only_resolver)
{
  GList *iter;

  g_return_val_if_fail (source || !main_source_is_only_resolver, NULL);
  g_return_val_if_fail (additional_keys || !main_source_is_only_resolver, NULL);

  for (iter = sources; iter; iter = g_list_next (iter)) {
    GList *_additional_keys = NULL;
    GrlSource *_source = (GrlSource *) iter->data;

    if (_source == source) {
      continue;
    }

    if (grl_source_may_resolve (_source, media, key, &_additional_keys)) {
      return _source;
    }

    if (additional_keys && _additional_keys) {
      if (main_source_is_only_resolver &&
          !source_supports (source, _additional_keys))
        continue;

      *additional_keys = _additional_keys;
      return _source;
    }
  }

  return NULL;
}

/*
 * Does the same thing as g_list_concat(), except that elements from
 * @additional_set that are already in @original_set are destroyed instead of
 * being added to the result. The same happens for elements that are more than
 * once in @additional_set.
 * Because of that, if @original_set does not contain doubles, the result will
 * not contain doubles.
 *
 * You can also use this method to remove doubles from a list like that:
 * my_list = list_union (NULL, my_list, free_func);
 *
 * Note that no elements are copied, elements of @additional_set are either
 * moved to @original_set or destroyed.
 * Therefore, both @original_set and @additional_set are modified.
 *
 * @free_func is optional.
 */
static GList *
list_union (GList *original_set, GList *additional_set, GDestroyNotify free_func)
{
  while (additional_set) {
    /* these two lines pop the first element of additional_set into tmp */
    GList *tmp = additional_set;
    additional_set = g_list_remove_link (additional_set, tmp);

    if (NULL == g_list_find (original_set, tmp->data)) {
      original_set = g_list_concat (original_set, tmp);
    } else {
      if (free_func)
        free_func (tmp->data);
      g_list_free_1 (tmp);
    }
  }
  return original_set;
}

/*
 * Find the sources that should be queried to add @keys to @media.
 * If @additional_keys is provided, the result may include sources that need
 * more metadata to be present in @media, the keys corresponding to that
 * metadata will be put in @additional_keys.
 * If @additional_keys is NULL, will only consider sources that can resolve
 * @keys immediately
 *
 * If @main_source_is_only_resolver is TRUE and @additional_keys is not @NULL,
 * only additional keys that can be resolved directly by @source will be
 * considered. Sources that need other additional keys will not be put in the
 * returned list.
 *
 * Ignore elements of @keys that are already in @media.
 */
static GList *
get_additional_sources (GrlSource *source,
                        GrlMedia *media,
                        GList *keys,
                        GList **additional_keys,
                        gboolean main_source_is_only_resolver)
{
  GList *missing_keys, *iter, *result = NULL, *sources;
  GrlRegistry *registry;

  missing_keys = missing_in_data (GRL_DATA (media), keys);
  if (!missing_keys)
    return NULL;

  registry = grl_registry_get_default ();
  sources = grl_registry_get_sources_by_operations (registry,
                                                    GRL_OP_RESOLVE,
                                                    TRUE);

  for (iter = missing_keys; iter; iter = g_list_next (iter)) {
    GrlKeyID key = GRLPOINTER_TO_KEYID (iter->data);
    GrlSource *_source;
    GList *needed_keys = NULL;

    _source = get_additional_source_for_key (source, sources, media, key,
                                             additional_keys?&needed_keys:NULL,
                                             main_source_is_only_resolver);
    if (_source) {
      result = g_list_append (result, _source);

      if (needed_keys)
        *additional_keys = list_union (*additional_keys, needed_keys, NULL);

      GRL_INFO ("%s can resolve %s %s",
                grl_source_get_name (_source),
                GRL_METADATA_KEY_GET_NAME (key),
                needed_keys? "with more keys" : "directly");

    } else {
      GRL_DEBUG ("Could not find a source for %s",
                 GRL_METADATA_KEY_GET_NAME (key));
    }
  }
  g_list_free (missing_keys);

  /* list_union() is used to remove duplicates */
  return list_union (NULL, result, NULL);
}

/*
 * Will add to @keys the keys that should be asked to @source when doing an
 * operation with GRL_RESOLVE_FULL.
 * The added keys are the keys that will be needed by other sources to obtain
 * the ones that @source says it cannot resolve.
 */
static GList *
expand_operation_keys (GrlSource *source,
                       GrlMedia *media,
                       GList *keys)
{
  GList *unsupported_keys = NULL,
    *additional_keys = NULL,
    *sources;

  GRL_DEBUG (__FUNCTION__);

  if (!keys)
    return NULL;

  /* Get the list of keys not supported by the source; they will be queried to
     other sources */
  unsupported_keys = filter_supported (source, &keys, TRUE);

  /* now, for each of the unsupported keys to solve
   * (the ones we know @source cannot resolve), try to find a matching source.
   * A matching source may need additional keys, but then these additional keys
   * can be resolved by @source.
   */

  sources =
    get_additional_sources (source, media, unsupported_keys,
                            &additional_keys, TRUE);
  g_list_free (sources);

  /* Merge back the supported and unsupported list, and add also the additional keys */
  keys = g_list_concat (keys, unsupported_keys);
  keys = list_union (keys, additional_keys, NULL);

  return keys;
}

/*
 * Returns %TRUE if @key is a slow key for @source
 */
static gboolean
is_slow_key (GrlSource *source, GrlKeyID key)
{
  return (g_list_find ((GList *) grl_source_slow_keys (source),
                       GRLKEYID_TO_POINTER (key)) != NULL);
}

/*
 * Create a node for the map keys
 */
static MapNode *
map_node_new (GrlSource *source, GList *keys)
{
  MapNode *node = g_new (MapNode, 1);
  node->source = g_object_ref (source);
  node->required_keys = g_list_copy (keys);
  node->being_queried = FALSE;

  return node;
}

/*
 * Free a MapNode
 */
static void
map_node_free (MapNode *node)
{
  g_object_unref (node->source);
  g_list_free (node->required_keys);
  g_free (node);
}

/*
 * Free a list of MapNodes
 */
static void
map_list_nodes_free (GList *nodes)
{

  g_list_free_full (nodes, (GDestroyNotify) map_node_free);
}

/*
 * Create a new (key, [sources]) map
 */
static GHashTable *
map_keys_new (void)
{
  return g_hash_table_new (g_direct_hash, g_direct_equal);
}

/*
 * Maps each key in @keys to the list of sources (from @sources) that can
 * resolve that key. For each of those (key, source) pair, a list of keys
 * dependencies is added
 */
static void
map_keys_to_sources (GHashTable *map, GList *keys, GList *sources, GrlMedia *media, gboolean filter_slow_keys)
{
  GList *each_source;
  GList *resolvable_sources;
  GList *each_key;
  GList *required_keys;
  GList *keys_to_map_later = NULL;

  for (each_key = keys;
       each_key;
       each_key = g_list_next (each_key)) {
    if (g_hash_table_lookup_extended (map, each_key->data, NULL, NULL)) {
      /* Key already in map; skip */
      continue;
    }
    resolvable_sources = NULL;
    for (each_source = sources;
         each_source;
         each_source = g_list_next (each_source)) {
      if (filter_slow_keys &&
          is_slow_key (each_source->data, GRLPOINTER_TO_KEYID (each_key->data))) {
        continue;
      }
      required_keys = NULL;
      if (grl_source_may_resolve (each_source->data,
                                  media,
                                  GRLPOINTER_TO_KEYID (each_key->data),
                                  &required_keys)) {
        resolvable_sources = g_list_prepend (resolvable_sources, map_node_new (each_source->data, NULL));
      } else if (required_keys) {
        resolvable_sources = g_list_prepend (resolvable_sources, map_node_new (each_source->data, required_keys));
        keys_to_map_later = g_list_concat (keys_to_map_later, required_keys);
      }
    }

    resolvable_sources = g_list_reverse (resolvable_sources);
    g_hash_table_insert (map, each_key->data, resolvable_sources);
  }

  if (keys_to_map_later) {
    map_keys_to_sources (map, keys_to_map_later, sources, media, filter_slow_keys);
    g_list_free (keys_to_map_later);
  }
}

/*
 * Create a new (source, spec) map
 */
static GHashTable *
map_sources_new (void)
{
  return g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                g_object_unref,
                                (GDestroyNotify) resolve_spec_free);
}

/*
 * Given a (keys, [sources]) @map, builds a map of sources to
 * GrlSourceResolveSpec that can solve @key in @media.  Returns @FALSE if the
 * key can't be mapped (could be, for instance, that it detects a loop)
 */
static gboolean
map_sources_to_specs (GHashTable *specs,
                      GHashTable *map,
                      GrlMedia *media,
                      GrlKeyID key,
                      GrlOperationOptions *options,
                      gpointer user_data)
{
  GList *map_nodes;
  MapNode *node;
  GList *each_required_key;
  GrlSourceResolveSpec *rs;
  gboolean success;

  /* Search the source candidate to solve the key */
  map_nodes = g_hash_table_lookup (map, GRLKEYID_TO_POINTER (key));
  while (map_nodes) {
    node = (MapNode *) map_nodes->data;
    if (node->being_queried) {
      /* If it has required keys and it is marked as being queried, it means we
         have enter in a loop */
      return (node->required_keys == NULL);
    }
    /* Check if it has any dependency */
    if (node->required_keys) {
      /* Mark node to avoid endless loops */
      node->being_queried = TRUE;
      success = TRUE;
      for (each_required_key = node->required_keys;
           each_required_key;
           each_required_key = g_list_next (each_required_key)) {
        success = map_sources_to_specs (specs, map, media,
                                        GRLPOINTER_TO_KEYID (each_required_key->data),
                                        options, user_data);
        if (!success) {
          break;
        }
      }
      node->being_queried = FALSE;
      if (success) {
        return TRUE;
      } else {
        /* Try next node */
        map_nodes = g_list_next (map_nodes);
        continue;
      }
    } else {
      rs = g_hash_table_lookup (specs, node->source);
      if (!rs) {
        /* Build spec */
        rs = g_new (GrlSourceResolveSpec, 1);
        rs->source = g_object_ref (node->source);
        rs->media = g_object_ref (media);
        rs->operation_id = grl_operation_generate_id ();
        rs->keys = g_list_prepend (NULL, GRLKEYID_TO_POINTER (key));
        rs->options = g_object_ref (options);
        rs->callback = resolve_result_relay_cb;
        rs->user_data = user_data;
        g_hash_table_insert (specs, g_object_ref (node->source), rs);
      } else {
        /* Put key in spec */
        rs->keys = g_list_prepend (rs->keys, GRLKEYID_TO_POINTER (key));
      }
      node->being_queried = TRUE;
    }
    return TRUE;
  }

  return FALSE;
}

/*
 * Update @map knowing @key is known; means dropping the @key from the map and
 * updating all keys that were depending on @key.
 */
static void
map_update_known_key (GHashTable *map, GrlKeyID key, GrlMedia *media)
{
  GList *keylist;
  GList *each_key;
  GList *map_nodes;
  GList *each_node;
  MapNode *node;

  map_list_nodes_free (g_hash_table_lookup (map, GRLKEYID_TO_POINTER (key)));
  g_hash_table_remove (map, GRLKEYID_TO_POINTER (key));

  keylist = g_hash_table_get_keys (map);
  for (each_key = keylist;
       each_key;
       each_key = g_list_next (each_key)) {
    map_nodes = g_hash_table_lookup (map, each_key->data);
    for (each_node = map_nodes;
         each_node;
         each_node = g_list_next (each_node)) {
      node = (MapNode *) each_node->data;
      if (g_list_find (node->required_keys, (gconstpointer) GRLKEYID_TO_POINTER (key))) {
        /* Let's recompute the required keys */
        g_list_free (node->required_keys);
        node->required_keys = NULL;
        grl_source_may_resolve (node->source,
                                media,
                                GRLPOINTER_TO_KEYID (each_key->data),
                                &(node->required_keys));
      }
    }
  }
  g_list_free (keylist);
}

/*
 * Update @map knowing that @key could not be resolved by @source.
 */
static void
map_update_unknown_key (GHashTable *map, GrlKeyID key, GrlSource *source)
{
  GList *unsolvable_keys = NULL;
  GList *each_unsolvable_key;
  GList *each_key;
  GList *map_nodes;
  GList *delete_nodes = NULL;
  GList *each_node;
  GList *keylist;
  MapNode *node;

  map_nodes = g_hash_table_lookup (map, GRLKEYID_TO_POINTER (key));
  each_node = map_nodes;
  while (each_node) {
    node = (MapNode *) each_node->data;
    if (node->being_queried && node->source == source) {
      map_nodes = g_list_delete_link (map_nodes, each_node);
      map_node_free (node);
      g_hash_table_insert (map, GRLKEYID_TO_POINTER (key), map_nodes);
      each_node = NULL;
    } else {
      each_node = g_list_next (each_node);
    }
  }

  /* If @map_nodes is empty, means no source is able to solve this key; so any
     other (key, source) depending on this key can't neither be solved; so
     remove them from the map */
  if (!map_nodes) {
    unsolvable_keys = g_list_prepend (unsolvable_keys,
                                      GRLKEYID_TO_POINTER (key));
    for (each_unsolvable_key = g_list_last (unsolvable_keys);
         each_unsolvable_key;
         each_unsolvable_key = g_list_previous (each_unsolvable_key)) {
      keylist = g_hash_table_get_keys (map);
      for (each_key = keylist;
           each_key;
           each_key = g_list_next (each_key)) {
        map_nodes = g_hash_table_lookup (map, each_key->data);
        if (map_nodes) {
          for (each_node = map_nodes;
               each_node;
               each_node = g_list_next (each_node)) {
            node = (MapNode *) each_node->data;
            if (g_list_find (node->required_keys, each_unsolvable_key->data)) {
              /* Put this node for further deletion, as it can't be solved */
              delete_nodes = g_list_prepend (delete_nodes, node);
            }
          }
          /* Delete nodes */
          for (each_node = delete_nodes;
               each_node;
               each_node = g_list_next (each_node)) {
            map_nodes = g_list_remove (map_nodes, each_node->data);
          }
          g_list_free (delete_nodes);
          delete_nodes = NULL;

          g_hash_table_insert (map, each_key->data, map_nodes);
          /* If this key can't be resolved neither, mark it */
          if (!map_nodes) {
            unsolvable_keys = g_list_prepend (unsolvable_keys, each_key->data);
          }
        }
      }
      g_list_free (keylist);
    }
    g_list_free (unsolvable_keys);
  }
}

static void
send_decorated_media (GrlMedia *media,
                      gpointer user_data,
                      const GError *error)
{
  struct ResolveRelayCb *mrc = (struct ResolveRelayCb *) user_data;

  mrc->user_callback (mrc->spec.res->source, mrc->spec.res->operation_id,
                      media, mrc->user_data, error);
  resolve_relay_free (mrc);
}

static void
media_decorate_cb (GrlSource *source,
                   guint operation_id,
                   GrlMedia *media,
                   gpointer user_data,
                   const GError *error)
{
  struct MediaDecorateData *mdd = (struct MediaDecorateData *) user_data;
  GError *_error = NULL;
  GRL_DEBUG (__FUNCTION__);

  if (operation_id > 0) {
    g_hash_table_remove (mdd->pending_callbacks, source);
  }

  /* Check if pending resolutions must be cancelled */
  if (!mdd->cancelled &&
      operation_is_cancelled (mdd->operation_id)) {
    mdd->cancelled = TRUE;
    g_hash_table_foreach (mdd->pending_callbacks, cancel_resolve, NULL);
  }

  /* If all operations are complete, send the element */
  if (g_hash_table_size (mdd->pending_callbacks) == 0) {
    if (mdd->cancelled) {
      _error = g_error_new (GRL_CORE_ERROR,
                            GRL_CORE_ERROR_OPERATION_CANCELLED,
                            _("Operation was cancelled"));
    }
    mdd->callback (media, mdd->user_data, _error);
    g_clear_error (&_error);
    g_object_unref (mdd->source);
    g_hash_table_unref (mdd->pending_callbacks);
    g_slice_free (struct MediaDecorateData, mdd);
  }
}

static void
media_decorate (GrlSource *main_source,
                guint main_operation_id,
                GrlMedia *media,
                GList *keys,
                GrlOperationOptions *options,
                MediaDecorateCb callback,
                gpointer user_data)
{
  struct MediaDecorateData *mdd;
  GList *s, *sources;
  guint operation_id;
  GrlOperationOptions *decorate_options;
  GrlOperationOptions *supported_options;
  GrlResolutionFlags flags;

  flags = grl_operation_options_get_resolution_flags (options);
  if (flags & GRL_RESOLVE_FULL) {
    decorate_options = grl_operation_options_copy (options);
    grl_operation_options_set_resolution_flags (decorate_options,
                                                flags & ~GRL_RESOLVE_FULL);
  } else {
    decorate_options = g_object_ref (options);
  }

  sources = get_additional_sources (main_source, media,
                                    keys, NULL, FALSE);

  mdd = g_slice_new (struct MediaDecorateData);
  mdd->source = g_object_ref (main_source);
  mdd->operation_id = main_operation_id;
  mdd->callback = callback;
  mdd->user_data = user_data;
  mdd->pending_callbacks = g_hash_table_new (g_direct_hash, g_direct_equal);
  mdd->cancelled = FALSE;

  for (s = sources; s; s = g_list_next (s)) {
    if (grl_source_supported_operations (s->data) & GRL_OP_RESOLVE) {
      grl_operation_options_obey_caps (decorate_options,
                                       grl_source_get_caps (s->data, GRL_OP_RESOLVE),
                                       &supported_options,
                                       NULL);
      operation_id = grl_source_resolve (s->data, media, keys, supported_options,
                                         media_decorate_cb, mdd);
      g_object_unref (supported_options);
      if (operation_id > 0) {
        g_hash_table_insert (mdd->pending_callbacks,
                             s->data,
                             GUINT_TO_POINTER (operation_id));
      }
    }
  }

  /* Check if nobody can solve the keys */
  if (g_hash_table_size (mdd->pending_callbacks) == 0) {
    media_decorate_cb (NULL, 0, media, mdd, NULL);
  }

  g_object_unref (decorate_options);
  g_list_free (sources);
}

static void
media_from_uri_result_relay_cb (GrlSource *source,
                                guint operation_id,
                                GrlMedia *media,
                                gpointer user_data,
                                const GError *error)
{
  struct ResolveRelayCb *rrc = (struct ResolveRelayCb *) user_data;
  GError *_error = (GError *) error;
  GList *unknown_keys;

  GRL_DEBUG (__FUNCTION__);

  /* Free specs */
  media_from_uri_spec_free (rrc->spec.mfu);

  /* Append the source-id in case it is not set */
  if (media && !grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_SOURCE)) {
    grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_SOURCE, grl_source_get_id (source));
  }

  /* Check if cancelled */
  if (operation_is_cancelled (rrc->operation_id)) {
    /* if the plugin already set an error, we don't care because we're
     * cancelled */
    GRL_DEBUG ("operation was cancelled");
    _error = g_error_new (GRL_CORE_ERROR,
                          GRL_CORE_ERROR_OPERATION_CANCELLED,
                          _("Operation was cancelled"));
  }

  if (_error) {
    rrc->user_callback (source, rrc->operation_id, media, rrc->user_data, _error);
    if (_error != error) {
      g_error_free (_error);
    }
    operation_set_finished (rrc->operation_id);
    resolve_relay_free (rrc);
    return;
  }

  if (grl_operation_options_get_resolution_flags (rrc->options) & GRL_RESOLVE_FULL) {
    /* Check if there are unsolved keys that need to be solved by other
       sources */
    unknown_keys = filter_known_keys (media, rrc->keys);
    if (unknown_keys) {
      media_decorate (source, operation_id, media, unknown_keys, rrc->options,
                      send_decorated_media, rrc);
      g_list_free (unknown_keys);
      return;
    }
  }

  rrc->user_callback (source, rrc->operation_id, media, rrc->user_data, error);
  operation_set_finished (rrc->operation_id);
  resolve_relay_free (rrc);
}

static void
cancel_resolve_spec (GrlSource *source, GrlSourceResolveSpec *spec)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (spec->operation_id);
  if (op_state) {
    source_cancel_cb (op_state);
  }
}

static void
resolve_result_relay_cb (GrlSource *source,
                         guint operation_id,
                         GrlMedia *media,
                         gpointer user_data,
                         const GError *error)
{
  struct ResolveRelayCb *rrc = (struct ResolveRelayCb *) user_data;
  GList *each_key;
  GList *delete_key;

  GRL_DEBUG (__FUNCTION__);

  if (!operation_is_cancelled (operation_id)) {
    /* Check which keys are now known */
    each_key = rrc->keys;
    while (each_key) {
      if (grl_data_has_key (GRL_DATA (media), GRLPOINTER_TO_KEYID (each_key->data))) {
        map_update_known_key (rrc->map, GRLPOINTER_TO_KEYID (each_key->data), media);
        delete_key = each_key;
        each_key = g_list_next (each_key);
        rrc->keys = g_list_delete_link (rrc->keys, delete_key);
      } else {
        map_update_unknown_key (rrc->map, GRLPOINTER_TO_KEYID (each_key->data), source);
        each_key = g_list_next (each_key);
      }
    }

    g_hash_table_remove (rrc->resolve_specs, source);
  }

  operation_set_finished (operation_id);

  if (operation_is_cancelled (rrc->operation_id) &&
      !rrc->cancel_invoked) {
    rrc->cancel_invoked = TRUE;
    g_hash_table_foreach (rrc->resolve_specs, (GHFunc) cancel_resolve_spec, NULL);
  }

  if (error && source == rrc->source && !rrc->error) {
    /* Save error for further sending */
    rrc->error = g_error_copy (error);
  }

  if (g_hash_table_size (rrc->resolve_specs) == 0 && !rrc->specs_to_invoke) {
    /* All sources have replied. Let's run another round if not cancelled */
    if (!operation_is_cancelled (rrc->operation_id)) {
      each_key = rrc->keys;
      while (each_key) {
        if (map_sources_to_specs (rrc->resolve_specs, rrc->map, media,
                                  GRLPOINTER_TO_KEYID (each_key->data),
                                  rrc->options, rrc)) {
          each_key = g_list_next (each_key);
        } else {
          delete_key = each_key;
          each_key = g_list_next (each_key);
          rrc->keys = g_list_delete_link (rrc->keys, delete_key);
        }
      }

    }

    rrc->specs_to_invoke = g_hash_table_get_values (rrc->resolve_specs);
    if (rrc->specs_to_invoke) {
      guint id;
      id = g_idle_add_full (grl_operation_options_get_resolution_flags (rrc->options) & GRL_RESOLVE_IDLE_RELAY?
                            G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                            resolve_idle,
                            rrc,
                            NULL);
      g_source_set_name_by_id (id, "[grilo] resolve_idle");
    } else {
      guint id;
      id = g_idle_add_full (grl_operation_options_get_resolution_flags (rrc->options) & GRL_RESOLVE_IDLE_RELAY?
                            G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                            resolve_all_done,
                            rrc,
                            NULL);
      g_source_set_name_by_id (id, "[grilo] resolve_all_done");
    }
  }
}

static gboolean
queue_process (gpointer user_data)
{
  QueueElement *qelement;
  GError *error;
  gint remaining;
  struct BrowseRelayCb *brc = (struct BrowseRelayCb *) user_data;

  /* Check if operation is cancelled */
  if (operation_is_cancelled (brc->operation_id)) {
    /* This is how this works: if operation is cancelled, no one will add more
       elements to queue. If one with remaining=0 is found, means that the
       browse/search/query operation finished before operation is cancelled. So we
       need to emit this result to user to tell him that operation is cancelled. For
       elements that are not ready, means that a source_resolve() was run to get
       solve more keys. So the algorithm is freeing all elements that are ready, and
       if some of them has remaining==0, sending the cancel signal to user */
    while ((qelement = (QueueElement *) g_queue_peek_head (brc->queue)) &&
           qelement->is_ready) {
      g_queue_pop_head (brc->queue);
      if (qelement->remaining == 0) {
        error = g_error_new (GRL_CORE_ERROR,
                             GRL_CORE_ERROR_OPERATION_CANCELLED,
                             _("Operation was cancelled"));
        brc->user_callback (brc->source, brc->operation_id, NULL,
                            0, brc->user_data, error);
        g_error_free (error);
      }
      g_clear_error (&qelement->error);
      g_free (qelement);
    }
    if (g_queue_is_empty (brc->queue)) {
      operation_set_finished (brc->operation_id);
      browse_relay_free (brc);
      return FALSE;
    }
    brc->dispatcher_running = FALSE;
    return FALSE;
  }

  /* Send the last element */
  qelement = (QueueElement *) g_queue_pop_head (brc->queue);
  remaining = qelement->remaining;
  brc->user_callback (brc->source, brc->operation_id, qelement->media,
                      remaining, brc->user_data, qelement->error);
  g_clear_error (&qelement->error);
  g_free (qelement);

  if (remaining == 0) {
    operation_set_finished (brc->operation_id);
    browse_relay_free (brc);
    return FALSE;
  }

  /* Check if should keep running */
  qelement = (QueueElement *) g_queue_peek_head (brc->queue);
  brc->dispatcher_running = qelement && qelement->is_ready;

  return brc->dispatcher_running;
}

static void
queue_start_process (struct BrowseRelayCb *brc)
{
  QueueElement *qelement;

  if (!brc->dispatcher_running) {
    qelement = g_queue_peek_head (brc->queue);
    if (qelement && qelement->is_ready) {
      guint id = g_idle_add (queue_process,  brc);
      g_source_set_name_by_id (id, "[grilo] queue_process");
      brc->dispatcher_running = TRUE;
    }
  }
}

static gint
compare_queue_element (QueueElement *qelement,
                       GrlMedia *media)
{
  return qelement->media != media;	//return 0 when equal
}

static void
media_ready_cb (GrlMedia *media,
                gpointer user_data,
                const GError *error)
{
  GList *element;
  QueueElement *qelement;
  struct BrowseRelayCb *brc = (struct BrowseRelayCb *) user_data;

  /* Mark element as ready */
  element = g_queue_find_custom (brc->queue, media,
                                 (GCompareFunc) compare_queue_element);
  if (!element) {
    GRL_WARNING ("Media not found in the queue!");
    return;
  }

  qelement = (QueueElement *) element->data;
  qelement->is_ready = TRUE;
  queue_start_process (brc);
}

static void
queue_add_media (struct BrowseRelayCb *brc,
                 GrlMedia *media,
                 guint remaining,
                 const GError *error)
{
  QueueElement *qelement;
  GList *unknown_keys = NULL;

  if (!brc->queue) {
    brc->queue = g_queue_new ();
  }

  /* Add element */
  qelement = g_new (QueueElement, 1);
  qelement->media = media;
  qelement->remaining = remaining;
  /* Media is ready if we do not need to ask other sources to complete it */
  qelement->is_ready = TRUE;
  if (grl_operation_options_get_resolution_flags (brc->options) & GRL_RESOLVE_FULL) {
    unknown_keys = filter_known_keys (media, brc->keys);
    if (unknown_keys) {
      qelement->is_ready = FALSE;
    }
  }
  if (error) {
    qelement->error = g_error_copy (error);
  } else {
    qelement->error = NULL;
  }
  g_queue_push_tail (brc->queue, qelement);

  if (!qelement->is_ready) {
    media_decorate (brc->source, brc->operation_id, media, unknown_keys,
                    brc->options, media_ready_cb, brc);
  }

  queue_start_process (brc);
}

static struct AutoSplitCtl *
auto_split_setup (GrlSource *source,
                  GrlOperationOptions *options)
{
  struct AutoSplitCtl *as_ctl = NULL;
  gint count = grl_operation_options_get_count (options);

  if (source->priv->auto_split_threshold > 0 &&
      count > source->priv->auto_split_threshold) {
    GRL_DEBUG ("auto-split: enabled");

    as_ctl = g_slice_new (struct AutoSplitCtl);
    as_ctl->threshold = source->priv->auto_split_threshold;
    as_ctl->total_remaining = count;
    as_ctl->chunk_remaining = as_ctl->threshold;
    count = as_ctl->chunk_remaining;
    grl_operation_options_set_count (options, count);
    GRL_DEBUG ("auto-split: requesting chunk (skip=%u, count=%u)",
               grl_operation_options_get_skip (options),
               count);
  }

  return as_ctl;
}

static void
auto_split_run_next_chunk (struct BrowseRelayCb *brc)
{
  guint id;
  brc->auto_split->chunk_remaining = MIN (brc->auto_split->threshold,
                                          brc->auto_split->total_remaining);

  switch (brc->operation_type) {
  case GRL_OP_BROWSE:
    grl_operation_options_set_skip (brc->spec.browse->options,
                                    grl_operation_options_get_skip (brc->spec.browse->options) +
                                    brc->auto_split->threshold);
    grl_operation_options_set_count (brc->spec.browse->options,
                                     brc->auto_split->chunk_remaining);
    GRL_DEBUG ("auto-split: requesting chunk (skip=%u, count=%u)",
               grl_operation_options_get_skip (brc->spec.browse->options),
               grl_operation_options_get_count (brc->spec.browse->options));
    id = g_idle_add_full (grl_operation_options_get_resolution_flags (brc->options) & GRL_RESOLVE_IDLE_RELAY?
                          G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                          browse_idle,
                          brc->spec.browse,
                          NULL);
    g_source_set_name_by_id (id, "[grilo] browse_idle");
    break;
  case GRL_OP_SEARCH:
    grl_operation_options_set_skip (brc->spec.search->options,
                                    grl_operation_options_get_skip (brc->spec.search->options) +
                                    brc->auto_split->threshold);
    grl_operation_options_set_count (brc->spec.search->options,
                                     brc->auto_split->chunk_remaining);
    GRL_DEBUG ("auto-split: requesting chunk (skip=%u, count=%u)",
               grl_operation_options_get_skip (brc->spec.search->options),
               grl_operation_options_get_count (brc->spec.search->options));
    id = g_idle_add_full (grl_operation_options_get_resolution_flags (brc->options) & GRL_RESOLVE_IDLE_RELAY?
                          G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                          search_idle,
                          brc->spec.search,
                          NULL);
    g_source_set_name_by_id (id, "[grilo] search_idle");
    break;
  case GRL_OP_QUERY:
    grl_operation_options_set_skip (brc->spec.query->options,
                                    grl_operation_options_get_skip (brc->spec.query->options) +
                                    brc->auto_split->threshold);
    grl_operation_options_set_count (brc->spec.query->options,
                                     brc->auto_split->chunk_remaining);
    GRL_DEBUG ("auto-split: requesting chunk (skip=%u, count=%u)",
               grl_operation_options_get_skip (brc->spec.query->options),
               grl_operation_options_get_count (brc->spec.query->options));
    id = g_idle_add_full (grl_operation_options_get_resolution_flags (brc->options) & GRL_RESOLVE_IDLE_RELAY?
                          G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                          query_idle,
                          brc->spec.query,
                          NULL);
    g_source_set_name_by_id (id, "[grilo] query_idle");
    break;
  default:
    g_assert_not_reached ();
    break;
  }
}

static void
warn_if_no_id (GrlMedia *media,
               GrlSource *source)
{
  const char *id;

  if (media == NULL || !grl_media_is_container (media))
    return;

  id = grl_media_get_id (media);
  if (id == NULL || *id == '\0')
    GRL_WARNING ("Media container is not browsable, has no ID: %s (source: %s)",
                 grl_media_get_title (media),
                 grl_source_get_id (source));
}

static void
browse_result_relay_cb (GrlSource *source,
                        guint operation_id,
                        GrlMedia *media,
                        guint remaining,
                        gpointer user_data,
                        const GError *error)
{
  GError *_error;
  struct BrowseRelayCb *brc = (struct BrowseRelayCb *) user_data;

  GRL_DEBUG (__FUNCTION__);

  /* Ignore elements after operation has completed */
  if (operation_is_completed (operation_id)) {
    GRL_WARNING ("Source '%s' emitted 'remaining=0' more than once "
                 "for operation %d",
                 grl_source_get_id (source), operation_id);
    g_clear_object (&media);
    return;
  }

  /* Check if cancelled */
  if (operation_is_cancelled (operation_id)) {
    GRL_DEBUG ("Operation is cancelled, skipping result until getting the last one");
    g_clear_object (&media);
    /* Wait for the last element */
    if (remaining > 0) {
      return;
    } else {
      _error = g_error_new (GRL_CORE_ERROR,
                            GRL_CORE_ERROR_OPERATION_CANCELLED,
                            _("Operation was cancelled"));
      brc->user_callback (source, operation_id, NULL, 0,
                          brc->user_data, _error);
      g_error_free (_error);
      goto free_resources;
    }
  }

  /* Auto-split management */
  if (brc->auto_split) {
    brc->auto_split->chunk_remaining--;
    brc->auto_split->total_remaining--;
    /* On last element, check if more elements should be asked: if source
       satisfied all requested elements, but we need to get more */
    if (remaining == 0) {
      if (brc->auto_split->chunk_remaining == 0 &&
          brc->auto_split->total_remaining > 0) {
        auto_split_run_next_chunk (brc);
        remaining = brc->auto_split->total_remaining;
      }
    } else {
      remaining = brc->auto_split->total_remaining;
    }
  }

  /* Set the source */
  if (media && !grl_media_get_source (media)) {
    grl_media_set_source (media, grl_source_get_id (source));
  }

  /* If we need further processing of media, put it in a queue */
  if (grl_operation_options_get_resolution_flags (brc->options) &
      (GRL_RESOLVE_FULL | GRL_RESOLVE_IDLE_RELAY)) {
    queue_add_media (brc, media, remaining, error);
  } else {
    warn_if_no_id (media, source);
    brc->user_callback (source, operation_id, media, remaining,
                        brc->user_data, error);
  }

  if (remaining == 0) {
  free_resources:
    browse_relay_spec_free (brc);
    if (!brc->queue || g_queue_is_empty (brc->queue)) {
      operation_set_finished (operation_id);
      browse_relay_free (brc);
    } else {
      /* There are elements pending to be processed; let's wait to free it in
         the queue */
      operation_set_completed (operation_id);
    }
  }
}

static void
remove_result_relay_cb (GrlSource *source,
                        GrlMedia *media,
                        gpointer user_data,
                        const GError *error)
{
  struct RemoveRelayCb *rrc = (struct RemoveRelayCb *) user_data;

  rrc->user_callback (source, media, rrc->user_data, error);
  remove_relay_free (rrc);
}

static gboolean
resolve_idle (gpointer user_data)
{
  struct ResolveRelayCb *rrc = (struct ResolveRelayCb *) user_data;
  GrlSourceResolveSpec *rs;
  GList *spec;
  GList *key;
  gboolean run_next;

  GRL_DEBUG (__FUNCTION__);

  /* Abort if operation was cancelled */
  if (operation_is_cancelled (rrc->operation_id)) {
    for (spec = rrc->specs_to_invoke;
         spec;
         spec = g_list_next (rs)) {
      rs = (GrlSourceResolveSpec *) spec->data;
      g_hash_table_remove (rrc->resolve_specs, rs->source);
    }
    g_list_free (rrc->specs_to_invoke);
    rrc->specs_to_invoke = NULL;
    run_next = FALSE;
    resolve_result_relay_cb (rrc->source, rrc->operation_id, rrc->media, rrc, NULL);
  } else {
    rs = rrc->specs_to_invoke->data;
    rrc->specs_to_invoke = g_list_delete_link (rrc->specs_to_invoke,
                                               rrc->specs_to_invoke);
    run_next = (rrc->specs_to_invoke != NULL);

    /* Put the specific keys in rs also into rrc */
    for (key = rs->keys; key; key = g_list_next (key)) {
      if (!g_list_find (rrc->keys, key->data)) {
        rrc->keys = g_list_prepend (rrc->keys, key->data);
      }
    }

    operation_set_ongoing (rs->source, rs->operation_id);
    operation_set_started (rs->operation_id);
    GRL_SOURCE_GET_CLASS (rs->source)->resolve (rs->source, rs);
  }

  return run_next;
}

static gboolean
resolve_all_done (gpointer user_data)
{
  struct ResolveRelayCb *rrc = (struct ResolveRelayCb *) user_data;

  GRL_DEBUG (__FUNCTION__);

  if (operation_is_cancelled (rrc->operation_id)) {
    g_clear_error (&rrc->error);
    rrc->error = g_error_new (GRL_CORE_ERROR,
                              GRL_CORE_ERROR_OPERATION_CANCELLED,
                              _("Operation was cancelled"));
  }

  rrc->user_callback (rrc->source, rrc->operation_id, rrc->media, rrc->user_data, rrc->error);
  operation_set_finished (rrc->operation_id);
  resolve_relay_free (rrc);

  return FALSE;
}

static gboolean
media_from_uri_idle (gpointer user_data)
{
  GrlSourceMediaFromUriSpec *mfus = (GrlSourceMediaFromUriSpec *) user_data;

  GRL_DEBUG (__FUNCTION__);

  /* Abort if operation is cancelled */
  if (operation_is_cancelled (mfus->operation_id)) {
    mfus->callback (mfus->source, mfus->operation_id,
                    NULL, mfus->user_data, NULL);
  } else {
    operation_set_started (mfus->operation_id);
    GRL_SOURCE_GET_CLASS (mfus->source)->media_from_uri (mfus->source, mfus);
  }

  return FALSE;
}

static gboolean
browse_idle (gpointer user_data)
{
  GrlSourceBrowseSpec *bs = (GrlSourceBrowseSpec *) user_data;

  GRL_DEBUG (__FUNCTION__);

  /* Abort if operation is cancelled */
  if (operation_is_cancelled (bs->operation_id)) {
    bs->callback (bs->source, bs->operation_id, NULL, 0, bs->user_data, NULL);
  } else {
    operation_set_started (bs->operation_id);
    GRL_SOURCE_GET_CLASS (bs->source)->browse (bs->source, bs);
  }

  return FALSE;
}

static gboolean
search_idle (gpointer user_data)
{
  GrlSourceSearchSpec *ss = (GrlSourceSearchSpec *) user_data;

  GRL_DEBUG (__FUNCTION__);

  /* Abort if operation is cancelled */
  if (operation_is_cancelled (ss->operation_id)) {
    ss->callback (ss->source, ss->operation_id, NULL, 0, ss->user_data, NULL);
  } else {
    operation_set_started (ss->operation_id);
    GRL_SOURCE_GET_CLASS (ss->source)->search (ss->source, ss);
  }

  return FALSE;
}

static gboolean
query_idle (gpointer user_data)
{
  GrlSourceQuerySpec *qs = (GrlSourceQuerySpec *) user_data;
  GRL_DEBUG (__FUNCTION__);

  /* Abort if operation is cancelled */
  if (operation_is_cancelled (qs->operation_id)) {
    qs->callback (qs->source, qs->operation_id, NULL, 0, qs->user_data, NULL);
  } else {
    operation_set_started (qs->operation_id);
    GRL_SOURCE_GET_CLASS (qs->source)->query (qs->source, qs);
  }

  return FALSE;
}

static gboolean
remove_idle (gpointer user_data)
{
  struct RemoveRelayCb *rrc = (struct RemoveRelayCb *) user_data;
  GRL_DEBUG (__FUNCTION__);

  if (rrc->error) {
    rrc->user_callback (rrc->source, rrc->media, rrc->user_data, rrc->error);
    remove_relay_free (rrc);
  } else {
    GRL_SOURCE_GET_CLASS (rrc->source)->remove (rrc->source, rrc->spec);
  }

  return FALSE;
}

static void
resolve_result_async_cb (GrlSource *source,
                         guint operation_id,
                         GrlMedia *media,
                         gpointer user_data,
                         const GError *error)
{
  GrlDataSync *ds = (GrlDataSync *) user_data;

  GRL_DEBUG (__FUNCTION__);

  if (error) {
    ds->error = g_error_copy (error);
  }

  ds->data = media;
  ds->complete = TRUE;
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
    g_list_free_full (ds->data, g_object_unref);

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
remove_async_cb (GrlSource *source,
                 GrlMedia *media,
                 gpointer user_data,
                 const GError *error)
{
  GrlDataSync *ds = (GrlDataSync *) user_data;

  GRL_DEBUG (__FUNCTION__);

  if (error) {
    ds->error = g_error_copy (error);
  }

  ds->complete = TRUE;
}

static void
store_result_async_cb (GrlSource *source,
                       GrlMedia *media,
                       GList *failed_keys,
                       gpointer user_data,
                       const GError *error)
{
  GrlDataSync *ds = (GrlDataSync *) user_data;

  GRL_DEBUG (__FUNCTION__);

  if (error) {
    ds->error = g_error_copy (error);
  }

  ds->data = g_list_copy (failed_keys);
  ds->complete = TRUE;
}


static void
store_metadata_result_async_cb (GrlSource *source,
                                GrlMedia *media,
                                GList *failed_keys,
                                gpointer user_data,
                                const GError *error)
{
  GrlDataSync *ds = (GrlDataSync *) user_data;

  GRL_DEBUG (__FUNCTION__);

  if (error) {
    ds->error = g_error_copy (error);
  }

  ds->data = g_list_copy (failed_keys);
  ds->complete = TRUE;
}

static GHashTable *
map_writable_keys (GrlSource *source,
                   GList *keys,
                   GrlWriteFlags flags,
                   GList **failed_keys)
{
  GHashTable *map;
  GrlRegistry *registry;
  GList *sources = NULL;
  GList *sources_iter;
  GList *unsupported_keys;
  GrlSource *_source;

  map = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                               g_object_unref,
                               (GDestroyNotify) g_list_free);

  /* 'key_list' holds keys that can be written by this source
     'unsupportedy_keys' holds those that must be handled by other sources */
  GList *key_list = g_list_copy (keys);
  if (grl_source_supported_operations (source) & GRL_OP_STORE_METADATA) {
    unsupported_keys = filter_writable (source, &key_list, TRUE);
  } else {
    unsupported_keys = key_list;
    key_list = NULL;
  }

  if (key_list) {
    g_hash_table_insert (map, g_object_ref (source), key_list);
  }

  if (!unsupported_keys || !(flags & GRL_WRITE_FULL)) {
    /* We are done! */
    goto done;
  }

  /* Check if other sources can write the missing keys */
  registry = grl_registry_get_default ();
  sources =
    grl_registry_get_sources_by_operations (registry,
                                            GRL_OP_STORE_METADATA,
                                            TRUE);

  for (sources_iter = sources; unsupported_keys && sources_iter;
       sources_iter = g_list_next (sources_iter)) {
    _source = GRL_SOURCE (sources_iter->data);

    if (_source == source) {
      continue;
    }

    key_list = unsupported_keys;
    unsupported_keys = filter_writable (_source, &key_list, TRUE);

    if (!key_list) {
      continue;
    }

    g_hash_table_insert (map, g_object_ref (_source), key_list);

    if (!unsupported_keys) {
      break;
    }
  }

  g_list_free (sources);

 done:
  *failed_keys = unsupported_keys;
  return map;
}

static void
store_relay_cb (GrlSource *source,
                GrlMedia *media,
                GList *failed_keys,
                gpointer user_data,
                const GError *error)
{
  struct StoreRelayCb *src = (struct StoreRelayCb *) user_data;
  GrlSourceStoreSpec *ss  = src->spec;

  GRL_DEBUG (__FUNCTION__);

  if (error || !(src->flags & GRL_WRITE_FULL)) {
    if (src->user_callback)
      src->user_callback (source, media, failed_keys, src->user_data, error);
  } else if (failed_keys != NULL) {
    run_store_metadata (source, media, failed_keys, GRL_WRITE_FULL,
                        src->user_callback, src->user_data);
  }
  store_relay_free (src);
  store_spec_free (ss);
}

static void
store_metadata_ctl_cb (GrlSource *source,
                       GrlMedia *media,
                       GList *failed_keys,
                       gpointer user_data,
                       const GError *error)
{
  struct StoreMetadataRelayCb *smrc;
  GError *own_error = NULL;

  GRL_DEBUG (__FUNCTION__);

  smrc = (struct StoreMetadataRelayCb *) user_data;

  if (failed_keys) {
    smrc->failed_keys = g_list_concat (smrc->failed_keys, failed_keys);
  }

  g_hash_table_remove (smrc->map, source);

  /* If we all sources have answered */
  if (g_hash_table_size (smrc->map) == 0) {
    /* We ignore the plugin errors, instead we create an own error
       if some keys were not written */
    if (smrc->user_callback) {
      if (smrc->failed_keys) {
        own_error = g_error_new (GRL_CORE_ERROR,
                                 GRL_CORE_ERROR_STORE_METADATA_FAILED,
                                 _("Some keys could not be written"));
      }
      smrc->user_callback (smrc->source,
                           media,
                           smrc->failed_keys,
                           smrc->user_data,
                           own_error);
      g_clear_error (&own_error);
    }
    store_metadata_relay_free (smrc);
  }
}

static gboolean
store_idle (gpointer user_data)
{
  GrlSourceStoreSpec *ss = (GrlSourceStoreSpec *) user_data;

  GRL_DEBUG (__FUNCTION__);

  GRL_SOURCE_GET_CLASS (ss->source)->store(ss->source, ss);

  return FALSE;
}

static gboolean
store_metadata_idle (gpointer user_data)
{
  GrlSourceStoreMetadataSpec *sms;
  gboolean stop;
  struct StoreMetadataRelayCb *smrc;

  GRL_DEBUG (__FUNCTION__);

  smrc = (struct StoreMetadataRelayCb *) user_data;

  sms = g_new (GrlSourceStoreMetadataSpec, 1);

  sms->source = g_object_ref (g_list_first (smrc->use_sources)->data);
  sms->keys = g_hash_table_lookup (smrc->map, sms->source);
  sms->media = g_object_ref (smrc->media);
  sms->callback = store_metadata_ctl_cb;
  sms->user_data = smrc;

  /* Remove list header */
  smrc->use_sources = g_list_remove_link (smrc->use_sources, smrc->use_sources);
  smrc->specs = g_list_prepend (smrc->specs, sms);

  stop = smrc->use_sources == NULL;
  GRL_SOURCE_GET_CLASS (sms->source)->store_metadata (sms->source, sms);

  return !stop;
}

static void
run_store_metadata (GrlSource *source,
                    GrlMedia *media,
                    GList *keys,
                    GrlWriteFlags flags,
                    GrlSourceStoreCb callback,
                    gpointer user_data)
{
  GHashTable *map;
  GList *failed_keys = NULL;
  GError *error;
  struct StoreMetadataRelayCb *smrc;
  guint id;

  map = map_writable_keys (source, keys, flags, &failed_keys);

  if (g_hash_table_size (map) == 0) {
    error = g_error_new (GRL_CORE_ERROR,
                         GRL_CORE_ERROR_STORE_METADATA_FAILED,
                         _("None of the specified keys are writable"));
    if (callback) {
      callback (source, media, failed_keys, user_data, error);
    }

    g_error_free (error);
    g_list_free (failed_keys);
    g_hash_table_unref (map);

    return;
  }

  smrc = g_slice_new (struct StoreMetadataRelayCb);
  smrc->source = g_object_ref (source);
  smrc->media = g_object_ref (media);
  smrc->map = map;
  smrc->use_sources = g_hash_table_get_keys (map);
  smrc->failed_keys = failed_keys;
  smrc->specs = NULL;
  smrc->user_callback = callback;
  smrc->user_data = user_data;

  id = g_idle_add (store_metadata_idle, smrc);
  g_source_set_name_by_id (id, "[grilo] store_metadata_idle");
}

static gboolean
check_options (GrlSource *source,
               GrlSupportedOps operation,
               GrlOperationOptions *options)
{
  GrlCaps *caps;

  /* FIXME: that check should be in somewhere in GrlOperationOptions */
  if (grl_operation_options_get_count (options) == 0)
    return FALSE;

  /* Check only if the source supports the operation */
  if (grl_source_supported_operations (source) & operation) {
    caps = grl_source_get_caps (source, operation);

    return grl_operation_options_obey_caps (options, caps, NULL, NULL);
  } else {
    return TRUE;
  }
}

/* ============= API ============= */

/**
 * grl_source_supported_keys:
 * @source: a source
 *
 * Get a list of #GrlKeyID, which describe a metadata types that this
 * source can fetch and store.
 *
 * Returns: (element-type GrlKeyID) (transfer none): a #GList with the keys
 *
 * Since: 0.2.0
 */
const GList *
grl_source_supported_keys (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  if (GRL_SOURCE_GET_CLASS (source)->supported_keys) {
    return GRL_SOURCE_GET_CLASS (source)->supported_keys (source);
  } else {
    return NULL;
  }
}

/**
 * grl_source_slow_keys:
 * @source: a source
 *
 * Similar to grl_source_supported_keys(), but these keys
 * are marked as slow because of the amount of traffic/processing needed
 * to fetch them.
 *
 * Returns: (element-type GrlKeyID) (transfer none): a #GList with the keys
 *
 * Since: 0.2.0
 */
const GList *
grl_source_slow_keys (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  if (GRL_SOURCE_GET_CLASS (source)->slow_keys) {
    return GRL_SOURCE_GET_CLASS (source)->slow_keys (source);
  } else {
    return NULL;
  }
}

/**
 * grl_source_writable_keys:
 * @source: a source
 *
 * Similar to grl_source_supported_keys(), but these keys
 * are marked as writable, meaning the source allows the client
 * to provide new values for these keys that will be stored permanently.
 *
 * Returns: (element-type GrlKeyID) (transfer none):
 * a #GList with the keys
 *
 * Since: 0.2.0
 */
const GList *
grl_source_writable_keys (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  if (GRL_SOURCE_GET_CLASS (source)->writable_keys) {
    return GRL_SOURCE_GET_CLASS (source)->writable_keys (source);
  } else {
    return NULL;
  }
}

/**
 * grl_source_get_id:
 * @source: a source
 *
 * Returns: the ID of the @source
 *
 * Since: 0.2.0
 */
const gchar *
grl_source_get_id (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  return source->priv->id;
}

/**
 * grl_source_get_name:
 * @source: a source
 *
 * Returns: the name of the @source
 *
 * Since: 0.2.0
 */
const gchar *
grl_source_get_name (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  return source->priv->name;
}

/**
 * grl_source_get_icon:
 * @source: a source
 *
 * Returns: (transfer none): a #GIcon
 *
 * Since: 0.2.8
 */
GIcon *
grl_source_get_icon (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  return source->priv->icon;
}

/**
 * grl_source_get_description:
 * @source: a source
 *
 * Returns: the description of the @source
 *
 * Since: 0.2.0
 */
const gchar *
grl_source_get_description (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  return source->priv->desc;
}

/**
 * grl_source_get_tags:
 * @source: a source
 *
 * Returns: (element-type utf8) (transfer none): a %NULL-terminated list of tags
 *
 * Since: 0.2.10
 */
const char **
grl_source_get_tags (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  return (const char **) source->priv->tags->pdata;
}

/**
 * grl_source_get_plugin:
 * @source: a source
 *
 * Returns: (transfer none): the plugin this source belongs to
 *
 * Since: 0.2.0
 **/
GrlPlugin *
grl_source_get_plugin (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  return source->priv->plugin;
}

/**
 * grl_source_get_rank:
 * @source: a source
 *
 * Gets the source rank
 *
 * Returns: rank value
 *
 * Since: 0.2.0
 **/
gint
grl_source_get_rank (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), 0);

  return source->priv->rank;
}

/**
 * grl_source_get_supported_media:
 * @source: a source
 *
 * Gets the supported type of medias @source can deal with.
 *
 * Returns: a #GrlSupportedMedia value
 *
 * Since: 0.3.0
 **/
GrlSupportedMedia
grl_source_get_supported_media (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), 0);

  return source->priv->supported_media;
}

/**
 * grl_source_supported_operations:
 * @source: a source
 *
 * By default the derived objects of #GrlSource can only resolve.
 *
 * Returns: (type uint): a bitwise mangle with the supported operations by
 * the source
 *
 * Since: 0.2.0
 */
GrlSupportedOps
grl_source_supported_operations (GrlSource *source)
{
  GrlSupportedOps ops = GRL_OP_NONE;
  GrlSourceClass *source_class;

  g_return_val_if_fail (GRL_IS_SOURCE (source), GRL_OP_NONE);

  source_class = GRL_SOURCE_GET_CLASS (source);

  if (source_class->supported_operations) {
    return  source_class->supported_operations (source);
  }

  if (source_class->resolve) {
    ops |= GRL_OP_RESOLVE;
  }
  if (source_class->test_media_from_uri &&
      source_class->media_from_uri) {
    ops |= GRL_OP_MEDIA_FROM_URI;
  }
  if (source_class->browse) {
    ops |= GRL_OP_BROWSE;
  }
  if (source_class->search) {
    ops |= GRL_OP_SEARCH;
  }
  if (source_class->query) {
    ops |= GRL_OP_QUERY;
  }
  if (source_class->remove) {
    ops |= GRL_OP_REMOVE;
  }
  if (source_class->store_metadata) {
    ops |= GRL_OP_STORE_METADATA;
  }
  if (source_class->store) {
    ops |= GRL_OP_STORE;
  }

  if (source_class->notify_change_start &&
      source_class->notify_change_stop) {
    ops |= GRL_OP_NOTIFY_CHANGE;
  }

  return ops;
}

/**
 * grl_source_get_auto_split_threshold:
 * @source: a source
 *
 * Gets how much elements the source is able to handle in a single request.
 *
 * See #grl_source_set_auto_split_threshold()
 *
 * Returns: the assigned threshold, or 0 if there is no threshold
 *
 * Since: 0.2.0
 */
guint
grl_source_get_auto_split_threshold (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), 0);

  return source->priv->auto_split_threshold;
}

/**
 * grl_source_set_auto_split_threshold:
 * @source: a source
 * @threshold: the threshold to set
 *
 * Sets how much elements the source is able to handle in a single request.
 *
 * If user, during a search or browsing operation, asks for more elements than
 * the threshold, the request will be automatically splitted in chunks, so up to
 * @threshold elements will be asked in each request.
 *
 * Source will act as if user were asking just a chunk, and user won't notice
 * that the request was chunked.
 *
 * <note>
 *  <para>
 *    This function is intended to be used only by plugins.
 *  </para>
 * </note>
 *
 * Since: 0.2.0
 */
void
grl_source_set_auto_split_threshold (GrlSource *source,
                                     guint threshold)
{
  g_return_if_fail (GRL_IS_SOURCE (source));

  source->priv->auto_split_threshold = threshold;
}

/**
 * grl_source_resolve:
 * @source: a source
 * @media: (allow-none) (transfer full): a data transfer object
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @options: options to pass to this operation
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * This method is intended to fetch the requested keys of metadata of
 * a given @media to the media source.
 *
 * This method is asynchronous.
 *
 * Returns: the operation identifie
 *
 * Since: 0.2.0
 */
guint
grl_source_resolve (GrlSource *source,
                    GrlMedia *media,
                    const GList *keys,
                    GrlOperationOptions *options,
                    GrlSourceResolveCb callback,
                    gpointer user_data)
{
  GList *_keys;
  GList *each_key;
  GList *delete_key;
  struct ResolveRelayCb *rrc;
  guint operation_id;
  GList *sources = NULL;
  GrlResolutionFlags flags;
  GrlOperationOptions *resolve_options;

  GRL_DEBUG (__FUNCTION__);

  g_return_val_if_fail (GRL_IS_SOURCE (source), 0);
  g_return_val_if_fail (GRL_IS_OPERATION_OPTIONS (options), 0);
  g_return_val_if_fail (keys != NULL, 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (check_options (source, GRL_OP_RESOLVE, options), 0);

  if (!media) {
    /* Special case, NULL media ==> root container */
    media = grl_media_container_new ();
    grl_media_set_id (media, NULL);
    grl_media_set_source (media, grl_source_get_id (source));
  } else if (!grl_media_get_source (media)) {
    grl_media_set_source (media, grl_source_get_id (source));
  }

  /* By default assume we will use the parameters specified by the user */
  _keys = filter_known_keys (media, (GList *) keys);

  flags = grl_operation_options_get_resolution_flags (options);

  if (flags & GRL_RESOLVE_FULL) {
    GRL_DEBUG ("requested full metadata");
    sources = grl_registry_get_sources_by_operations (grl_registry_get_default (),
                                                      GRL_OP_RESOLVE,
                                                      TRUE);
    /* Put current source on top, if it supports resolve() */
    if (grl_source_supported_operations (source) & GRL_OP_RESOLVE) {
      sources = g_list_remove (sources, source);
      sources = g_list_prepend (sources, source);
    }
    flags &= ~GRL_RESOLVE_FULL;
    resolve_options = grl_operation_options_copy (options);
    grl_operation_options_set_resolution_flags (resolve_options, flags);
  } else {
    /* Consider only this source, if it supports resolve() */
    if (grl_source_supported_operations (source) & GRL_OP_RESOLVE) {
      sources = g_list_prepend (NULL, source);
    }
    resolve_options = g_object_ref (options);
  }

  if (flags & GRL_RESOLVE_FAST_ONLY) {
    GRL_DEBUG ("requested fast keys");
  }

  operation_id = grl_operation_generate_id ();

  operation_set_ongoing (source, operation_id);

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */
  rrc = g_slice_new0 (struct ResolveRelayCb);
  rrc->source = g_object_ref (source);
  rrc->operation_type = GRL_OP_RESOLVE;
  rrc->operation_id = operation_id;
  rrc->media = g_object_ref (media);
  rrc->user_callback = callback;
  rrc->user_data = user_data;
  rrc->options = resolve_options;

  /* If there are no sources able to solve just send the media */
  if (g_list_length (sources) == 0) {
    guint id;
    g_list_free (_keys);
    id = g_idle_add_full (flags & GRL_RESOLVE_IDLE_RELAY?
                          G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                          resolve_all_done,
                          rrc,
                          NULL);
    g_source_set_name_by_id (id, "[grilo] resolve_all_done");
    return operation_id;
  }

  _keys = filter_unresolvable_keys (sources, &_keys);

  rrc->keys = _keys;
  rrc->map = map_keys_new ();
  rrc->resolve_specs = map_sources_new ();

  map_keys_to_sources (rrc->map, _keys, sources, media, flags & GRL_RESOLVE_FAST_ONLY);
  g_list_free (sources);

  each_key = rrc->keys;
  while (each_key) {
    if (map_sources_to_specs (rrc->resolve_specs, rrc->map, media,
                              GRLPOINTER_TO_KEYID (each_key->data),
                              resolve_options, rrc)) {
      each_key = g_list_next (each_key);
    } else {
      delete_key = each_key;
      each_key = g_list_next (each_key);
      rrc->keys = g_list_delete_link (rrc->keys, delete_key);
    }
  }

  rrc->specs_to_invoke = g_hash_table_get_values (rrc->resolve_specs);
  if (rrc->specs_to_invoke) {
    guint id;
    id = g_idle_add_full (flags & GRL_RESOLVE_IDLE_RELAY?
                          G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                          resolve_idle,
                          rrc,
                          NULL);
    g_source_set_name_by_id (id, "[grilo] resolve_idle");
  } else {
    guint id;
    id = g_idle_add_full (flags & GRL_RESOLVE_IDLE_RELAY?
                          G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                          resolve_all_done,
                          rrc,
                          NULL);
    g_source_set_name_by_id (id, "[grilo] resolve_all_done");
  }

  return operation_id;
}

/**
 * grl_source_resolve_sync:
 * @source: a source
 * @media: (allow-none) (transfer full): a data transfer object
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @options: options to pass to this operation
 * @error: a #GError, or @NULL
 *
 * This method is intended to fetch the requested keys of metadata of
 * a given @media to the media source.
 *
 * This method is synchronous.
 *
 * Returns: (transfer full): a filled #GrlMedia
 *
 * Since: 0.2.0
 */
GrlMedia *
grl_source_resolve_sync (GrlSource *source,
                         GrlMedia *media,
                         const GList *keys,
                         GrlOperationOptions *options,
                         GError **error)
{
  GrlDataSync *ds;

  ds = g_slice_new0 (GrlDataSync);

  if (grl_source_resolve (source,
                          media,
                          keys,
                          options,
                          resolve_result_async_cb,
                          ds))
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

/**
 * grl_source_may_resolve:
 * @source: a source
 * @media: a media on which we want more metadata
 * @key_id: the key corresponding to a metadata we might want
 * @missing_keys: (element-type GrlKeyID): an optional originally empty list
 *
 * Checks whether @key_id may be resolved with @source for @media, so that the
 * caller can avoid calling grl_source_resolve() if it can be known in
 * advance it will fail.
 *
 * If the resolution is known to be impossible because more keys are needed in
 * @media, and @missing_keys is not @NULL, it is populated with the list of
 * GrlKeyID that would be needed.
 *
 * This function is synchronous and should not block.
 *
 * Returns: @TRUE if there's a possibility that @source resolves @key_id for
 * @media, @FALSE otherwise.
 *
 * Since: 0.2.0
 */
gboolean
grl_source_may_resolve (GrlSource *source,
                        GrlMedia *media,
                        GrlKeyID key_id,
                        GList **missing_keys)
{
  GrlSourceClass *klass;
  const GList *supported_keys;
  const gchar *media_source;

  GRL_DEBUG (__FUNCTION__);

  g_return_val_if_fail (GRL_IS_SOURCE (source), FALSE);
  g_return_val_if_fail (!missing_keys || !*missing_keys, FALSE);

  klass = GRL_SOURCE_GET_CLASS (source);

  if (klass->may_resolve) {
    return klass->may_resolve (source, media, key_id, missing_keys);
  }

  /* Default behaviour is to assume that if source implements resolve, then any
     supported key for its own content is resolved */
  if (klass->resolve) {
    GRL_DEBUG ("Using default may_resolve()");
    /* We need to know the media source */
    if (media == NULL ||
        (media_source = grl_media_get_source (media)) == NULL) {
      if (missing_keys) {
        *missing_keys = NULL;
        *missing_keys =
          g_list_prepend (*missing_keys,
                          GRLKEYID_TO_POINTER (GRL_METADATA_KEY_SOURCE));
      }
      return FALSE;
    }
    /* Content is from different source */
    if (g_strcmp0 (grl_source_get_id (source), media_source) != 0) {
      return FALSE;
    }
    /* Check if the key is supported */
    supported_keys = grl_source_supported_keys (source);
    return (g_list_find ((GList *) supported_keys,
                         GRLKEYID_TO_POINTER (key_id)) != NULL);
  } else {
    GRL_WARNING ("Source %s does not implement may_resolve()",
                 grl_source_get_id (source));
    return FALSE;
  }
}

/**
 * grl_source_test_media_from_uri:
 * @source: a source
 * @uri: A URI that can be used to identify a media resource
 *
 * Tests whether @source can instantiate a #GrlMedia object representing
 * the media resource exposed at @uri.
 *
 * Returns: %TRUE if it can, %FALSE otherwise.
 *
 * This method is synchronous.
 *
 * Since: 0.2.0
 */
gboolean
grl_source_test_media_from_uri (GrlSource *source,
                                const gchar *uri)
{
  GRL_DEBUG (__FUNCTION__);

  g_return_val_if_fail (GRL_IS_SOURCE (source), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);

  if (GRL_SOURCE_GET_CLASS (source)->test_media_from_uri) {
    return GRL_SOURCE_GET_CLASS (source)->test_media_from_uri (source, uri);
  } else {
    return FALSE;
  }
}

/**
 * grl_source_get_media_from_uri:
 * @source: a source
 * @uri: A URI that can be used to identify a media resource
 * @keys: (element-type GrlKeyID): A list of keys to resolve
 * @options: options wanted for that operation
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Creates an instance of #GrlMedia representing the media resource
 * exposed at @uri.
 *
 * It is recommended to call grl_source_test_media_from_uri() before invoking
 * this to check whether the target source can theoretically do the resolution.
 *
 * This method is asynchronous.
 *
 * Returns: the operation identifier
 *
 * Since: 0.2.0
 */
guint
grl_source_get_media_from_uri (GrlSource *source,
                               const gchar *uri,
                               const GList *keys,
                               GrlOperationOptions *options,
                               GrlSourceResolveCb callback,
                               gpointer user_data)
{
  GList *_keys;
  GrlSourceMediaFromUriSpec *mfus;
  struct ResolveRelayCb *rrc;
  guint operation_id;
  GrlResolutionFlags flags;
  guint id;

  GRL_DEBUG (__FUNCTION__);

  g_return_val_if_fail (GRL_IS_SOURCE (source), 0);
  g_return_val_if_fail (GRL_IS_OPERATION_OPTIONS (options), 0);
  g_return_val_if_fail (uri != NULL, 0);
  g_return_val_if_fail (keys != NULL, 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (grl_source_supported_operations (source) &
                        GRL_OP_MEDIA_FROM_URI, 0);
  g_return_val_if_fail (check_options (source, GRL_OP_MEDIA_FROM_URI, options), 0);

  _keys = g_list_copy ((GList *) keys);
  flags = grl_operation_options_get_resolution_flags (options);

  if (flags & GRL_RESOLVE_FAST_ONLY) {
    filter_slow (source, &_keys, FALSE);
  }

  if (flags & GRL_RESOLVE_FULL) {
    _keys = expand_operation_keys (source, NULL, _keys);
  }

  operation_id = grl_operation_generate_id ();

  /* We cannot prepare for full resolution yet because we don't
     have a GrlMedia t operate with.
     TODO: full resolution could be added in the relay calback
     when we get the GrlMedia object */

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */
  rrc = g_slice_new0 (struct ResolveRelayCb);
  rrc->source = g_object_ref (source);
  rrc->operation_type = GRL_OP_MEDIA_FROM_URI;
  rrc->operation_id = operation_id;
  rrc->keys = _keys;
  rrc->options = g_object_ref (options);
  rrc->user_callback = callback;
  rrc->user_data = user_data;

  mfus = g_new0 (GrlSourceMediaFromUriSpec, 1);
  mfus->source = g_object_ref (source);
  mfus->operation_id = operation_id;
  mfus->uri = g_strdup (uri);
  mfus->keys = _keys;
  mfus->options = grl_operation_options_copy (options);
  mfus->callback = media_from_uri_result_relay_cb;
  mfus->user_data = rrc;

  /* Save a reference to the operaton spec in the relay-cb's
     user_data so that we can free the spec there */
  rrc->spec.mfu = mfus;

  operation_set_ongoing (source, operation_id);

  id = g_idle_add_full (flags & GRL_RESOLVE_IDLE_RELAY?
                        G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                        media_from_uri_idle,
                        mfus,
                        NULL);
  g_source_set_name_by_id (id, "[grilo] media_from_uri_idle");

  return operation_id;
}

/**
 * grl_source_get_media_from_uri_sync:
 * @source: a source
 * @uri: A URI that can be used to identify a media resource
 * @keys: (element-type GrlKeyID): a list of keys to resolve
 * @options: options wanted for that operation
 * @error: a #GError, or @NULL
 *
 * Creates an instance of #GrlMedia representing the media resource
 * exposed at @uri.
 *
 * It is recommended to call grl_source_test_media_from_uri() before
 * invoking this to check whether the target source can theoretically do the
 * resolution.
 *
 * This method is synchronous.
 *
 * Returns: (transfer full): a filled #GrlMedia
 *
 * Since: 0.2.0
 */
GrlMedia *
grl_source_get_media_from_uri_sync (GrlSource *source,
                                    const gchar *uri,
                                    const GList *keys,
                                    GrlOperationOptions *options,
                                    GError **error)
{
  GrlDataSync *ds;
  GrlMedia *result;

  ds = g_slice_new0 (GrlDataSync);

  if (grl_source_get_media_from_uri (source,
                                     uri,
                                     keys,
                                     options,
                                     resolve_result_async_cb,
                                     ds))
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
 * grl_source_browse:
 * @source: a source
 * @container: (allow-none): a container of data transfer objects
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Browse from media elements through an available list.
 *
 * This method is asynchronous.
 *
 * Returns: the operation identifier
 *
 * Since: 0.2.0
 */
guint
grl_source_browse (GrlSource *source,
                   GrlMedia *container,
                   const GList *keys,
                   GrlOperationOptions *options,
                   GrlSourceResultCb callback,
                   gpointer user_data)
{
  GList *_keys;
  GrlSourceBrowseSpec *bs;
  guint operation_id;
  struct BrowseRelayCb *brc;
  GrlResolutionFlags flags;
  guint id;

  g_return_val_if_fail (GRL_IS_SOURCE (source), 0);
  g_return_val_if_fail (GRL_IS_OPERATION_OPTIONS (options), 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (grl_source_supported_operations (source) &
                        GRL_OP_BROWSE, 0);
  g_return_val_if_fail (check_options (source, GRL_OP_BROWSE, options), 0);

  /* By default assume we will use the parameters specified by the user */
  _keys = g_list_copy ((GList *) keys);

  flags = grl_operation_options_get_resolution_flags (options);

  if (flags & GRL_RESOLVE_FAST_ONLY) {
    GRL_DEBUG ("requested fast keys");
    filter_slow (source, &_keys, FALSE);
  }

  /* Setup full resolution mode if requested */
  if (flags & GRL_RESOLVE_FULL) {
    GRL_DEBUG ("requested full metadata");
    _keys = expand_operation_keys (source, NULL, _keys);
  }

  operation_id = grl_operation_generate_id ();

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */
  brc = g_slice_new (struct BrowseRelayCb);
  brc->source = g_object_ref (source);
  brc->operation_type = GRL_OP_BROWSE;
  brc->operation_id = operation_id;
  brc->keys = _keys;
  brc->options = g_object_ref (options);
  brc->user_callback = callback;
  brc->user_data = user_data;
  brc->queue = NULL;
  brc->dispatcher_running = FALSE;

  bs = g_new (GrlSourceBrowseSpec, 1);
  bs->source = g_object_ref (source);
  bs->operation_id = operation_id;
  /* _keys is already a copy */
  bs->keys = _keys;
  bs->options = grl_operation_options_copy (options);
  bs->callback = browse_result_relay_cb;
  bs->user_data = brc;

  if (!container) {
    /* Special case: NULL container ==> NULL id */
    bs->container = grl_media_container_new ();
    grl_media_set_source (bs->container,
                          grl_source_get_id (source));
  } else {
    bs->container = g_object_ref (container);
  }

  /* Save a reference to the operaton spec in the relay-cb's
     user_data so that we can free the spec there when we get
     the last result */
  brc->spec.browse = bs;

  /* Setup auto-split management if requested */
  brc->auto_split = auto_split_setup (source, bs->options);

  operation_set_ongoing (source, operation_id);

  id = g_idle_add_full (flags & GRL_RESOLVE_IDLE_RELAY? G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                        browse_idle,
                        bs,
                        NULL);
  g_source_set_name_by_id (id, "[grilo] browse_idle");

  return operation_id;
}

/**
 * grl_source_browse_sync:
 * @source: a source
 * @container: (allow-none): a container of data transfer objects
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @error: a #GError, or @NULL
 *
 * Browse media elements through an available
 * list.
 *
 * This method is synchronous.
 *
 * Returns: (element-type GrlMedia) (transfer full): a #GList with #GrlMedia
 * elements. After use g_object_unref() every element and g_list_free() the
 * list.
 *
 * Since: 0.2.0
 */
GList *
grl_source_browse_sync (GrlSource *source,
                        GrlMedia *container,
                        const GList *keys,
                        GrlOperationOptions *options,
                        GError **error)
{
  GrlDataSync *ds;
  GList *result;

  ds = g_slice_new0 (GrlDataSync);

  if (grl_source_browse (source,
                         container,
                         keys,
                         options,
                         multiple_result_async_cb,
                         ds))
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
 * grl_source_search:
 * @source: a source
 * @text: the text to search
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Search for the @text string in a source for data identified with that string.
 *
 * If @text is @NULL then no text filter will be applied, and thus, no media
 * items from @source will be filtered. If @source does not support NULL-text
 * search operations it should notiy the client by setting
 * @GRL_CORE_ERROR_SEARCH_NULL_UNSUPPORTED in @callback's error parameter.
 *
 * This method is asynchronous.
 *
 * Returns: the operation identifier
 *
 * Since: 0.2.0
 */
guint
grl_source_search (GrlSource *source,
                   const gchar *text,
                   const GList *keys,
                   GrlOperationOptions *options,
                   GrlSourceResultCb callback,
                   gpointer user_data)
{
  GList *_keys;
  GrlSourceSearchSpec *ss;
  guint operation_id;
  struct BrowseRelayCb *brc;
  GrlResolutionFlags flags;
  guint id;

  g_return_val_if_fail (GRL_IS_SOURCE (source), 0);
  g_return_val_if_fail (GRL_IS_OPERATION_OPTIONS (options), 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (grl_source_supported_operations (source) &
                        GRL_OP_SEARCH, 0);
  g_return_val_if_fail (check_options (source, GRL_OP_SEARCH, options), 0);

  /* By default assume we will use the parameters specified by the user */
  _keys = g_list_copy ((GList *) keys);

  flags = grl_operation_options_get_resolution_flags (options);

  if (flags & GRL_RESOLVE_FAST_ONLY) {
    GRL_DEBUG ("requested fast keys");
    filter_slow (source, &_keys, FALSE);
  }

  /* Setup full resolution mode if requested */
  if (flags & GRL_RESOLVE_FULL) {
    GRL_DEBUG ("requested full metadata");
    _keys = expand_operation_keys (source, NULL, _keys);
  }

  operation_id = grl_operation_generate_id ();

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */
  brc = g_slice_new (struct BrowseRelayCb);
  brc->source = g_object_ref (source);
  brc->operation_type = GRL_OP_SEARCH;
  brc->operation_id = operation_id;
  brc->keys = _keys;
  brc->options = g_object_ref (options);
  brc->user_callback = callback;
  brc->user_data = user_data;
  brc->queue = NULL;
  brc->dispatcher_running = FALSE;

  ss = g_new (GrlSourceSearchSpec, 1);
  ss->source = g_object_ref (source);
  ss->operation_id = operation_id;
  ss->text = g_strdup (text);
  ss->keys = _keys;
  ss->options = grl_operation_options_copy (options);
  ss->callback = browse_result_relay_cb;
  ss->user_data = brc;

  /* Save a reference to the operaton spec in the relay-cb's
     user_data so that we can free the spec there when we get
     the last result */
  brc->spec.search = ss;

  /* Setup auto-split management if requested */
  brc->auto_split = auto_split_setup (source, ss->options);

  operation_set_ongoing (source, operation_id);

  id = g_idle_add_full (flags & GRL_RESOLVE_IDLE_RELAY? G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                        search_idle,
                        ss,
                        NULL);
  g_source_set_name_by_id (id, "[grilo] search_idle");

  return operation_id;
}

/**
 * grl_source_search_sync:
 * @source: a source
 * @text: the text to search
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @error: a #GError, or @NULL
 *
 * Search for the @text string in a source for data identified with that string.
 *
 * If @text is @NULL then no text filter will be applied, and thus, no media
 * items from @source will be filtered. If @source does not support NULL-text
 * search operations it should notiy the client by setting
 * @GRL_CORE_ERROR_SEARCH_NULL_UNSUPPORTED in the error parameter.
 *
 * This method is synchronous.
 *
 * Returns: (element-type GrlMedia) (transfer full): a #GList with #GrlMedia
 * elements. After use g_object_unref() every element and g_list_free() the
 * list.
 *
 * Since: 0.2.0
 */
GList *
grl_source_search_sync (GrlSource *source,
                        const gchar *text,
                        const GList *keys,
                        GrlOperationOptions *options,
                        GError **error)
{
  GrlDataSync *ds;
  GList *result;

  ds = g_slice_new0 (GrlDataSync);

  if (grl_source_search (source,
                         text,
                         keys,
                         options,
                         multiple_result_async_cb,
                         ds))
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
 * grl_source_query:
 * @source: a source
 * @query: the query to process
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Execute a specialized query (specific for each provider) on a media
 * repository.
 *
 * It is different from grl_source_search() semantically, because the query
 * implies a carefully crafted string, rather than a simple string to search.
 *
 * This method is asynchronous.
 *
 * Returns: the operation identifier
 *
 * Since: 0.2.0
 */
guint
grl_source_query (GrlSource *source,
                  const gchar *query,
                  const GList *keys,
                  GrlOperationOptions *options,
                  GrlSourceResultCb callback,
                  gpointer user_data)
{
  GList *_keys;
  GrlSourceQuerySpec *qs;
  guint operation_id;
  struct BrowseRelayCb *brc;
  GrlResolutionFlags flags;
  guint id;

  g_return_val_if_fail (GRL_IS_SOURCE (source), 0);
  g_return_val_if_fail (GRL_IS_OPERATION_OPTIONS (options), 0);
  g_return_val_if_fail (query != NULL, 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (grl_source_supported_operations (source) &
                        GRL_OP_QUERY, 0);
  g_return_val_if_fail (check_options (source, GRL_OP_QUERY, options), 0);

  /* By default assume we will use the parameters specified by the user */
  _keys = g_list_copy ((GList *) keys);

  flags = grl_operation_options_get_resolution_flags (options);

  if (flags & GRL_RESOLVE_FAST_ONLY) {
    GRL_DEBUG ("requested fast keys");
    filter_slow (source, &_keys, FALSE);
  }

  if (flags & GRL_RESOLVE_FULL) {
    GRL_DEBUG ("requested full metadata");
    _keys = expand_operation_keys (source, NULL, _keys);
  }

  operation_id = grl_operation_generate_id ();

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */
  brc = g_slice_new (struct BrowseRelayCb);
  brc->source = g_object_ref (source);
  brc->operation_type = GRL_OP_QUERY;
  brc->operation_id = operation_id;
  brc->keys = _keys;
  brc->options = g_object_ref (options);
  brc->user_callback = callback;
  brc->user_data = user_data;
  brc->queue = NULL;
  brc->dispatcher_running = FALSE;

  qs = g_new (GrlSourceQuerySpec, 1);
  qs->source = g_object_ref (source);
  qs->operation_id = operation_id;
  qs->query = g_strdup (query);
  qs->keys = _keys;
  qs->options = grl_operation_options_copy (options);
  qs->callback = browse_result_relay_cb;
  qs->user_data = brc;

  /* Save a reference to the operaton spec in the relay-cb's
     user_data so that we can free the spec there when we get
     the last result */
  brc->spec.query = qs;

  /* Setup auto-split management if requested */
  brc->auto_split = auto_split_setup (source, qs->options);

  operation_set_ongoing (source, operation_id);

  id = g_idle_add_full (flags & GRL_RESOLVE_IDLE_RELAY? G_PRIORITY_DEFAULT_IDLE: G_PRIORITY_HIGH_IDLE,
                        query_idle,
                        qs,
                        NULL);
  g_source_set_name_by_id (id, "[grilo] query_idle");

  return operation_id;
}

/**
 * grl_source_query_sync:
 * @source: a source
 * @query: the query to process
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @error: a #GError, or @NULL
 *
 * Execute a specialized query (specific for each provider) on a media
 * repository.
 *
 * This method is synchronous.
 *
 * Returns: (element-type GrlMedia) (transfer full): a #GList with #GrlMedia
 * elements. After use g_object_unref() every element and g_list_free() the
 * list.
 *
 * Since: 0.2.0
 */
GList *
grl_source_query_sync (GrlSource *source,
                       const gchar *query,
                       const GList *keys,
                       GrlOperationOptions *options,
                       GError **error)
{
  GrlDataSync *ds;
  GList *result;

  ds = g_slice_new0 (GrlDataSync);

  if (grl_source_query (source,
                        query,
                        keys,
                        options,
                        multiple_result_async_cb,
                        ds))
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

static gboolean
grl_source_store_remove_impl (GrlSource *source,
                              GrlMedia *media,
                              GrlSourceRemoveCb callback,
                              gpointer user_data)
{
  const gchar *id;
  struct RemoveRelayCb *rrc;
  GrlSourceRemoveSpec *rs;
  guint tag_id;

  GRL_DEBUG (__FUNCTION__);

  g_return_val_if_fail (GRL_IS_SOURCE (source), FALSE);
  g_return_val_if_fail (GRL_IS_MEDIA (media), FALSE);
  g_return_val_if_fail (callback != NULL, FALSE);
  g_return_val_if_fail (grl_source_supported_operations (source) &
                        GRL_OP_REMOVE, FALSE);

  rrc = g_slice_new (struct RemoveRelayCb);
  rrc->source = g_object_ref (source);
  rrc->media = g_object_ref (media);
  rrc->user_callback = callback;
  rrc->user_data = user_data;

  /* Check that we have the minimum information we need */
  id = grl_media_get_id (media);
  if (!id) {
    rrc->error = g_error_new (GRL_CORE_ERROR,
                              GRL_CORE_ERROR_REMOVE_FAILED,
                              _("Media has no “id”, cannot remove"));
    rrc->spec = NULL;
  } else {
    rrc->error = NULL;
    rs = g_new0 (GrlSourceRemoveSpec, 1);
    rs->source = g_object_ref (source);
    rs->media_id = g_strdup (id);
    rs->media = g_object_ref (media);
    rs->callback = remove_result_relay_cb;
    rs->user_data = rrc;
    rrc->spec = rs;
  }

  tag_id = g_idle_add (remove_idle, rrc);
  g_source_set_name_by_id (tag_id, "[grilo] remove_idle");

  return TRUE;
}

/**
 * grl_source_remove:
 * @source: a source
 * @media: a data transfer object
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Remove a @media from the @source repository.
 *
 * This method is asynchronous.
 *
 * Since: 0.2.0
 */
void
grl_source_remove (GrlSource *source,
                   GrlMedia *media,
                   GrlSourceRemoveCb callback,
                   gpointer user_data)
{
  grl_source_store_remove_impl (source, media, callback, user_data);
}

/**
 * grl_source_remove_sync:
 * @source: a source
 * @media: a data transfer object
 * @error: a #GError, or @NULL
 *
 * Remove a @media from the @source repository.
 *
 * This method is synchronous.
 *
 * Since: 0.2.0
 */
void
grl_source_remove_sync (GrlSource *source,
                        GrlMedia *media,
                        GError **error)
{
  GrlDataSync *ds;

  ds = g_slice_new0 (GrlDataSync);

  if (grl_source_store_remove_impl (source,
                                    media,
                                    remove_async_cb,
                                    ds))
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

static gboolean
grl_source_store_impl (GrlSource *source,
                       GrlMedia *parent,
                       GrlMedia *media,
                       GrlWriteFlags flags,
                       GrlSourceStoreCb callback,
                       gpointer user_data)
{
  struct StoreRelayCb *src;
  GrlSourceStoreSpec *ss;
  guint id;

  GRL_DEBUG (__FUNCTION__);

  g_return_val_if_fail (GRL_IS_SOURCE (source), FALSE);
  g_return_val_if_fail (!parent || grl_media_is_container (parent), FALSE);
  g_return_val_if_fail (GRL_IS_MEDIA (media), FALSE);

  g_return_val_if_fail ((!parent &&
                         grl_source_supported_operations (source) & GRL_OP_STORE) ||
                        (parent &&
                         grl_source_supported_operations (source) & GRL_OP_STORE_PARENT),
                        FALSE);

  src = g_slice_new (struct StoreRelayCb);
  src->flags = flags;
  src->user_callback = callback;
  src->user_data = user_data;

  ss = g_new (GrlSourceStoreSpec, 1);
  ss->source = g_object_ref (source);
  ss->parent = parent? g_object_ref (parent): NULL;
  ss->media = g_object_ref (media);
  ss->callback = store_relay_cb;
  ss->user_data = src;

  src->spec = ss;

  id = g_idle_add (store_idle, ss);
  g_source_set_name_by_id (id, "[grilo] store_idle");

  return TRUE;
}

/**
 * grl_source_store:
 * @source: a source
 * @parent: (allow-none): a parent to store the data transfer objects
 * @media: a data transfer object
 * @flags: flags to configure specific behaviour of the operation
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Store the @media into the @parent container
 *
 * This method is asynchronous.
 *
 * Since: 0.3.0
 */
void
grl_source_store (GrlSource *source,
                  GrlMedia *parent,
                  GrlMedia *media,
                  GrlWriteFlags flags,
                  GrlSourceStoreCb callback,
                  gpointer user_data)
{
  grl_source_store_impl (source, parent, media, flags, callback, user_data);
}

/**
 * grl_source_store_sync:
 * @source: a source
 * @parent: (allow-none): a #GrlMedia container to store the data transfer objects
 * @media: a #GrlMedia data transfer object
 * @flags: flags to configure specific behaviour of the operation
 * @error: a #GError, or @NULL
 *
 * Store the @media into the @parent container.
 *
 * This method is synchronous.
 *
 * Since: 0.3.0
 */
void
grl_source_store_sync (GrlSource *source,
                       GrlMedia *parent,
                       GrlMedia *media,
                       GrlWriteFlags flags,
                       GError **error)
{
  GrlDataSync *ds;

  ds = g_slice_new0 (GrlDataSync);

  if (grl_source_store_impl (source,
                             parent,
                             media,
                             flags,
                             store_result_async_cb,
                             ds))
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

static gboolean
grl_source_store_metadata_impl (GrlSource *source,
                                GrlMedia *media,
                                GList *keys,
                                GrlWriteFlags flags,
                                GrlSourceStoreCb callback,
                                gpointer user_data)
{
  GRL_DEBUG (__FUNCTION__);

  g_return_val_if_fail (GRL_IS_SOURCE (source), FALSE);
  g_return_val_if_fail (GRL_IS_MEDIA (media), FALSE);
  g_return_val_if_fail (keys != NULL, FALSE);

  run_store_metadata (source, media, keys, flags, callback, user_data);

  return TRUE;
}

/**
 * grl_source_store_metadata:
 * @source: a metadata source
 * @media: the #GrlMedia object that we want to operate on.
 * @keys: (element-type GrlKeyID) (allow-none): a list
 * of #GrlKeyID whose values we want to change.
 * @flags: Flags to configure specific behaviors of the operation.
 * @callback: (scope notified): the callback to execute when the operation is finished.
 * @user_data: user data set for the @callback
 *
 * Get the values for @keys from @media and store it permanently. After
 * calling this method, future queries that return this media object
 * shall return this new values for the selected keys.
 *
 * This function is asynchronous and uses the Glib's main loop.
 *
 * Since: 0.2.0
 */
void
grl_source_store_metadata (GrlSource *source,
                           GrlMedia *media,
                           GList *keys,
                           GrlWriteFlags flags,
                           GrlSourceStoreCb callback,
                           gpointer user_data)
{
  grl_source_store_metadata_impl (source,
                                  media,
                                  keys,
                                  flags,
                                  callback,
                                  user_data);
}

/**
 * grl_source_store_metadata_sync:
 * @source: a source
 * @media: the #GrlMedia object that we want to operate on
 * @keys: (element-type GrlKeyID) (allow-none): a list of
 * #GrlKeyID whose values we want to change
 * @flags: Flags to configure specific behaviors of the operation.
 * @error: a #GError, or @NULL
 *
 * Update @keys values from @media in the @source. After calling this method,
 * future queries that return this media object shall return this new value for
 * the selected key.
 *
 * This function is synchronous.
 *
 * Returns: (element-type GrlKeyID) (transfer container):
 * a #GList of keys that could not be updated, or @NULL
 *
 * Since: 0.2.0
 */
GList *
grl_source_store_metadata_sync (GrlSource *source,
                                GrlMedia *media,
                                GList *keys,
                                GrlWriteFlags flags,
                                GError **error)
{
  GrlDataSync *ds;
  GList *failed;

  ds = g_slice_new0 (GrlDataSync);

  if (grl_source_store_metadata_impl (source,
                                      media,
                                      keys,
                                      flags,
                                      store_metadata_result_async_cb,
                                      ds))
    grl_wait_for_async_operation_complete (ds);

  if (ds->error) {
    if (error) {
      *error = ds->error;
    } else {
      g_error_free (ds->error);
    }
  }

  failed = ds->data;

  g_slice_free (GrlDataSync, ds);

  return failed;
}

/**
 * grl_source_notify_change_start:
 * @source: a source
 * @error: a #GError, or @NULL
 *
 * Starts emitting ::content-changed signals when @source discovers changes in
 * the content. This instructs @source to setup the machinery needed to be aware
 * of changes in the content.
 *
 * Returns: @TRUE if initialization has succeed.
 *
 * Since: 0.2.0
 */
gboolean
grl_source_notify_change_start (GrlSource *source,
                                GError **error)
{
  GRL_DEBUG (__FUNCTION__);

  g_return_val_if_fail (GRL_IS_SOURCE (source), FALSE);
  g_return_val_if_fail (grl_source_supported_operations (source) &
                        GRL_OP_NOTIFY_CHANGE, FALSE);

  return GRL_SOURCE_GET_CLASS (source)->notify_change_start (source, error);
}

/**
 * grl_source_notify_change_stop:
 * @source: a source
 * @error: a #GError, or @NULL
 *
 * This will drop emission of ::content-changed signals from @source. When this
 * is done @source should stop the machinery required for it to track changes in
 * the content.
 *
 * Returns: @TRUE if stop has succeed.
 *
 * Since: 0.2.0
 */
gboolean
grl_source_notify_change_stop (GrlSource *source,
                               GError **error)
{
  GRL_DEBUG (__FUNCTION__);

  g_return_val_if_fail (GRL_IS_SOURCE (source), FALSE);
  g_return_val_if_fail (grl_source_supported_operations (source) &
                        GRL_OP_NOTIFY_CHANGE, FALSE);

  return GRL_SOURCE_GET_CLASS (source)->notify_change_stop (source, error);
}

static void
grl_media_set_source_if_unset (GrlMedia    *media,
                               const gchar *source)
{
  if (grl_media_get_source (media) != NULL)
    return;
  grl_media_set_source (media, source);
}

/**
 * grl_source_notify_change_list:
 * @source: a source
 * @changed_medias: (element-type GrlMedia) (transfer full): the list of
 * medias that have changed
 * @change_type: the type of change
 * @location_unknown: if change has happpened in @media or any descendant
 *
 * Emits "content-changed" signal to notify subscribers that a change ocurred
 * in @source.
 *
 * The function will take ownership of @changed medias and it should not be
 * manipulated in any way by the caller after invoking this function. If that is
 * needed, the caller must ref the array in advance.
 *
 * See GrlSource::content-changed signal.
 *
 * <note>
 *  <para>
 *    This function is intended to be used only by plugins.
 *  </para>
 * </note>
 *
 * Since: 0.2.0
 */
void grl_source_notify_change_list (GrlSource *source,
                                    GPtrArray *changed_medias,
                                    GrlSourceChangeType change_type,
                                    gboolean location_unknown)
{
  const gchar *source_id;

  g_return_if_fail (GRL_IS_SOURCE (source));
  g_return_if_fail (changed_medias);

  /* Set the source */
  source_id = grl_source_get_id (source);
  g_ptr_array_foreach (changed_medias,
                       (GFunc) grl_media_set_source_if_unset,
                       (gpointer) source_id);

  /* Add hook to free content when freeing the array */
  g_ptr_array_set_free_func (changed_medias, (GDestroyNotify) g_object_unref);

  g_signal_emit (source,
                 source_signals[SIG_CONTENT_CHANGED],
                 0,
                 changed_medias,
                 change_type,
                 location_unknown);

  g_ptr_array_unref (changed_medias);
}

/**
 * grl_source_notify_change:
 * @source: a source
 * @media: (allow-none): the media which has changed, or @NULL to use the root container.
 * @change_type: the type of change
 * @location_unknown: if change has happened in @media or any descendant
 *
 * Emits "content-changed" signal to notify subscribers that a change ocurred
 * in @source.
 *
 * See #grl_source_notify_change_list() function.
 *
 * <note>
 *  <para>
 *    This function is intended to be used only by plugins.
 *  </para>
 * </note>
 *
 * Since: 0.2.0
 */
void grl_source_notify_change (GrlSource *source,
                               GrlMedia *media,
                               GrlSourceChangeType change_type,
                               gboolean location_unknown)
{
  GPtrArray *ptr_array;

  g_return_if_fail (GRL_IS_SOURCE (source));

  if (!media) {
    media = grl_media_container_new ();
  } else {
    g_object_ref (media);
  }

  ptr_array = g_ptr_array_sized_new (1);
  g_ptr_array_add (ptr_array, media);
  grl_source_notify_change_list (source, ptr_array,
                                 change_type, location_unknown);
}

/******************************************************************************/

/**
 * grl_source_get_caps:
 * @source: a source
 * @operation: a supported operation. Even though the type allows to specify
 * several operations, only one should be provided here.
 *
 * Get the capabilities of @source for @operation.
 *
 * Returns: (transfer none): The capabilities
 *
 * Since: 0.2.0
 */
GrlCaps *
grl_source_get_caps (GrlSource *source,
                     GrlSupportedOps operation)
{
  static GrlCaps *default_caps = NULL;
  GrlSourceClass *klass = GRL_SOURCE_GET_CLASS (source);

  if (klass->get_caps)
    return klass->get_caps (source, operation);

  if (!default_caps)
    default_caps = grl_caps_new ();

  return default_caps;
}
