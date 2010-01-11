/*
 * Copyright (C) 2010 Igalia S.L.
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

#include "ms-media-source.h"
#include "ms-metadata-source-priv.h"
#include "content/ms-content-media.h"

#include <string.h>

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "ms-media-source"

#define MS_MEDIA_SOURCE_GET_PRIVATE(object)				\
  (G_TYPE_INSTANCE_GET_PRIVATE((object), MS_TYPE_MEDIA_SOURCE, MsMediaSourcePrivate))

struct _MsMediaSourcePrivate {
  guint padding;
};

struct FullResolutionCtlCb {
  MsMediaSourceResultCb user_callback;
  gpointer user_data;
  GList *source_map_list;
  guint flags;
};

struct FullResolutionDoneCb {
  MsMediaSourceResultCb user_callback;
  gpointer user_data;
  guint pending_callbacks;
  MsMediaSource *source;
  guint browse_id;
  guint remaining;
  struct FullResolutionCtlCb *ctl_info;
};

struct BrowseRelayCb {
  MsMediaSourceResultCb user_callback;
  gpointer user_data;
  gboolean use_idle;
  MsMediaSourceBrowseSpec *bspec;
  MsMediaSourceSearchSpec *sspec;
};

struct BrowseRelayIdle {
  MsMediaSourceResultCb user_callback;
  gpointer user_data;
  MsMediaSource *source;
  guint browse_id;
  MsContentMedia *media;
  guint remaining;
  GError *error;
  struct BrowseRelayCb *brc;
};

struct MetadataFullResolutionCtlCb {
  MsMediaSourceMetadataCb user_callback;
  gpointer user_data;
  GList *source_map_list;
  guint flags;
};

struct MetadataFullResolutionDoneCb {
  MsMediaSourceMetadataCb user_callback;
  gpointer user_data;
  guint pending_callbacks;
  MsMediaSource *source;
  struct MetadataFullResolutionCtlCb *ctl_info;;
};

struct MetadataRelayCb {
  MsMediaSourceMetadataCb user_callback;
  gpointer user_data;
  MsMediaSourceMetadataSpec *spec;
};

static guint ms_media_source_gen_browse_id (MsMediaSource *source);
static MsSupportedOps ms_media_source_supported_operations (MsMetadataSource *metadata_source);

/* ================ MsMediaSource GObject ================ */

G_DEFINE_ABSTRACT_TYPE (MsMediaSource, ms_media_source, MS_TYPE_METADATA_SOURCE);

static void
ms_media_source_class_init (MsMediaSourceClass *media_source_class)
{
  GObjectClass *gobject_class;
  MsMetadataSourceClass *metadata_source_class;

  gobject_class = G_OBJECT_CLASS (media_source_class);
  metadata_source_class = MS_METADATA_SOURCE_CLASS (media_source_class);

  metadata_source_class->supported_operations =
    ms_media_source_supported_operations;

  g_type_class_add_private (media_source_class, sizeof (MsMediaSourcePrivate));

  media_source_class->browse_id = 1;
}

static void
ms_media_source_init (MsMediaSource *source)
{
  source->priv = MS_MEDIA_SOURCE_GET_PRIVATE (source);
  memset (source->priv, 0, sizeof (MsMediaSourcePrivate));
}

/* ================ Utitilies ================ */

static void
free_browse_operation_spec (MsMediaSourceBrowseSpec *spec)
{
  g_object_unref (spec->source);
  g_free (spec->container_id);
  g_list_free (spec->keys);
  g_free (spec);
}

static void
free_search_operation_spec (MsMediaSourceSearchSpec *spec)
{
  g_object_unref (spec->source);
  g_free (spec->text);
  g_list_free (spec->keys);
  g_free (spec->filter);
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
browse_result_relay_idle (gpointer user_data)
{
  g_debug ("browse_result_relay_idle");
  struct BrowseRelayIdle *bri = (struct BrowseRelayIdle *) user_data;
  bri->user_callback (bri->source,
		      bri->browse_id,
		      bri->media,
		      bri->remaining,
		      bri->user_data,
		      bri->error);

  /* Free the operation spec when the operation is done */
  if (bri->remaining == 0) {
    if (bri->brc->bspec) {
      free_browse_operation_spec (bri->brc->bspec);
    } else if (bri->brc->sspec) {
      free_search_operation_spec (bri->brc->sspec);
    }
    g_free (bri->brc);
  }

  /* We copy the error if we do idle relay, we have to free it here */
  if (bri->error) {
    g_error_free (bri->error);
  }

  g_free (bri);

  return FALSE;
}

static void
browse_result_relay_cb (MsMediaSource *source,
			guint browse_id,
			MsContentMedia *media,
			guint remaining,
			gpointer user_data,
			const GError *error)
{
  struct BrowseRelayCb *brc;
  gchar *source_id;

  g_debug ("browse_result_relay_cb");

  brc = (struct BrowseRelayCb *) user_data;

  /* TODO: should this an object rather than a string? */
  if (media) {
    source_id = ms_metadata_source_get_id (MS_METADATA_SOURCE (source));  
    ms_content_media_set_source (media, source_id);
    g_free (source_id);
  }

  if (brc->use_idle) {
    struct BrowseRelayIdle *bri = g_new (struct BrowseRelayIdle, 1);
    bri->source = source;
    bri->browse_id = browse_id;
    bri->media = media;
    bri->remaining = remaining;
    bri->error = (GError *) (error ? g_error_copy (error) : NULL);
    bri->user_callback = brc->user_callback;
    bri->user_data = brc->user_data;
    bri->brc = brc;
    g_idle_add (browse_result_relay_idle, bri);
  } else {
    brc->user_callback (source,
			browse_id,
			media,
			remaining,
			brc->user_data,
			error);
    if (remaining == 0) {
      if (brc->bspec) {
	free_browse_operation_spec (brc->bspec);
      } else if (brc->sspec) {
	free_search_operation_spec (brc->sspec);
      }
      g_free (brc);
    }
  }
}

static void
metadata_result_relay_cb (MsMediaSource *source,
			  MsContentMedia *media,
			  gpointer user_data,
			  const GError *error)
{
  g_debug ("metadata_result_relay_cb");

  struct MetadataRelayCb *mrc;
  gchar *source_id;

  mrc = (struct MetadataRelayCb *) user_data;
  if (media) {
    source_id = ms_metadata_source_get_id (MS_METADATA_SOURCE (source));  
    ms_content_media_set_source (media, source_id);
    g_free (source_id);
  }

  mrc->user_callback (source, media, mrc->user_data, error);

  g_object_unref (mrc->spec->source);
  g_free (mrc->spec->object_id);
  g_list_free (mrc->spec->keys);
  g_free (mrc->spec);
  g_free (mrc);
}

static gboolean
browse_idle (gpointer user_data)
{
  g_debug ("browse_idle");
  MsMediaSourceBrowseSpec *bs = (MsMediaSourceBrowseSpec *) user_data;
  MS_MEDIA_SOURCE_GET_CLASS (bs->source)->browse (bs->source, bs);
  return FALSE;
}

static gboolean
search_idle (gpointer user_data)
{
  g_debug ("search_idle");
  MsMediaSourceSearchSpec *ss = (MsMediaSourceSearchSpec *) user_data;
  MS_MEDIA_SOURCE_GET_CLASS (ss->source)->search (ss->source, ss);
  return FALSE;
}

static gboolean
metadata_idle (gpointer user_data)
{
  g_debug ("metadata_idle");
  MsMediaSourceMetadataSpec *ms = (MsMediaSourceMetadataSpec *) user_data;
  MS_MEDIA_SOURCE_GET_CLASS (ms->source)->metadata (ms->source, ms);
  return FALSE;
}

static void
full_resolution_done_cb (MsMetadataSource *source,
			 MsContentMedia *media,
			 gpointer user_data,
			 const GError *error)
{
  g_debug ("full_resolution_done_cb");

  struct FullResolutionDoneCb *cb_info = 
    (struct FullResolutionDoneCb *) user_data;

  cb_info->pending_callbacks--;

  if (error) {
    g_warning ("Failed to fully resolve some metadata: %s", error->message);
  }

  /* If we are done with this result, invoke the user's callback */
  if (cb_info->pending_callbacks == 0) {
    cb_info->user_callback (cb_info->source, 
			    cb_info->browse_id, 
			    media,
			    cb_info->remaining, 
			    cb_info->user_data,
			    NULL);

    /* When we are done with the last result of the operation,
       free the control information too  */
    if (cb_info->remaining == 0) {
      free_source_map_list (cb_info->ctl_info->source_map_list);
      g_free (cb_info->ctl_info);
    }

    g_free (cb_info);
  }
}

static void
full_resolution_ctl_cb (MsMediaSource *source,
			guint browse_id,
			MsContentMedia *media,
			guint remaining,
			gpointer user_data,
			const GError *error)
{
  GList *iter;

  struct FullResolutionCtlCb *ctl_info =
    (struct FullResolutionCtlCb *) user_data;

  g_debug ("full_resolution_ctl_cb");

  /* If we got an error, invoke the user callback right away and bail out */
  if (error) {
    g_warning ("Operation failed: %s", error->message);
    ctl_info->user_callback (source,
			     browse_id,
			     media,
			     remaining,
			     ctl_info->user_data,
			     error);
    return;
  }

  /* Save all the data we need to emit the result */
  struct FullResolutionDoneCb *done_info =
    g_new (struct FullResolutionDoneCb, 1);
  done_info->user_callback = ctl_info->user_callback;
  done_info->user_data = ctl_info->user_data;
  done_info->pending_callbacks = g_list_length (ctl_info->source_map_list);
  done_info->source = source;
  done_info->browse_id = browse_id;
  done_info->remaining = remaining;
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
    g_debug ("Using '%s' to resolve extra metadata now", name);
    g_free (name);

    ms_metadata_source_resolve (map->source, 
				map->keys, 
				media, 
				ctl_info->flags,
				full_resolution_done_cb,
				done_info);

    iter = g_list_next (iter);
  }

  /* We cannot free the ctl_info until we are done processing 
     all the results. This is done in the full_resolution_cb instead.*/
}

static void
metadata_full_resolution_done_cb (MsMetadataSource *source,
				  MsContentMedia *media,
				  gpointer user_data,
				  const GError *error)
{
  g_debug ("metadata_full_resolution_done_cb");

  struct MetadataFullResolutionDoneCb *cb_info = 
    (struct MetadataFullResolutionDoneCb *) user_data;

  cb_info->pending_callbacks--;

  if (error) {
    g_warning ("Failed to fully resolve some metadata: %s", error->message);
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
metadata_full_resolution_ctl_cb (MsMediaSource *source,
				 MsContentMedia *media,
				 gpointer user_data,
				 const GError *error)
{
  GList *iter;

  struct MetadataFullResolutionCtlCb *ctl_info =
    (struct MetadataFullResolutionCtlCb *) user_data;

  g_debug ("metadata_full_resolution_ctl_cb");

  /* If we got an error, invoke the user callback right away and bail out */
  if (error) {
    g_warning ("Operation failed: %s", error->message);
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
    g_debug ("Using '%s' to resolve extra metadata now", name);
    g_free (name);

    ms_metadata_source_resolve (map->source, 
				map->keys, 
				media, 
				ctl_info->flags,
				metadata_full_resolution_done_cb,
				done_info);
    
    iter = g_list_next (iter);
  }
}

static guint
ms_media_source_gen_browse_id (MsMediaSource *source)
{
  MsMediaSourceClass *klass;
  klass = MS_MEDIA_SOURCE_GET_CLASS (source);
  return klass->browse_id++;
}

/* ================ API ================ */

guint
ms_media_source_browse (MsMediaSource *source, 
			const gchar *container_id,
			const GList *keys,
			guint skip,
			guint count,
			MsMetadataResolutionFlags flags,
			MsMediaSourceResultCb callback,
			gpointer user_data)
{
  MsMediaSourceResultCb _callback;
  gpointer _user_data ;
  GList *_keys;
  struct SourceKeyMapList key_mapping;
  MsMediaSourceBrowseSpec *bs;
  guint browse_id;
  struct BrowseRelayCb *brc;
  
  g_return_val_if_fail (IS_MS_MEDIA_SOURCE (source), 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (count > 0, 0);
  g_return_val_if_fail (ms_metadata_source_supported_operations (MS_METADATA_SOURCE (source)) &
			MS_OP_BROWSE, 0);

  /* By default assume we will use the parameters specified by the user */
  _keys = g_list_copy ((GList *) keys);
  _callback = callback;
  _user_data = user_data;

  if (flags & MS_RESOLVE_FAST_ONLY) {
    g_debug ("requested fast keys only");
    ms_metadata_source_filter_slow (MS_METADATA_SOURCE (source), &_keys, FALSE);
  }

  if (flags & MS_RESOLVE_FULL) {
    g_debug ("requested full resolution");
    ms_metadata_source_setup_full_resolution_mode (MS_METADATA_SOURCE (source),
						   _keys, &key_mapping);
    
    /* If we do not have a source map for the unsupported keys then
       we cannot resolve any of them */
    if (key_mapping.source_maps != NULL) {
      struct FullResolutionCtlCb *c = g_new0 (struct FullResolutionCtlCb, 1);
      c->user_callback = callback;
      c->user_data = user_data;
      c->source_map_list = key_mapping.source_maps;
      c->flags = flags;
      
      _callback = full_resolution_ctl_cb;
      _user_data = c;
      g_list_free (_keys);
      _keys = key_mapping.operation_keys;
    }    
  }

  browse_id = ms_media_source_gen_browse_id (source);

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */
  brc = g_new0 (struct BrowseRelayCb, 1);
  brc->user_callback = _callback;
  brc->user_data = _user_data;
  brc->use_idle = flags & MS_RESOLVE_IDLE_RELAY;
  _callback = browse_result_relay_cb;
  _user_data = brc;

  bs = g_new0 (MsMediaSourceBrowseSpec, 1);
  bs->source = g_object_ref (source);
  bs->browse_id = browse_id;
  bs->container_id = g_strdup (container_id);
  bs->keys = _keys;
  bs->skip = skip;
  bs->count = count;
  bs->flags = flags;
  bs->callback = _callback;
  bs->user_data = _user_data;

  /* Save a reference to the operaton spec in the relay-cb's 
     user_data so that we can free the spec there when we get
     the last result */
  brc->bspec = bs;

  g_idle_add (browse_idle, bs);
  
  return browse_id;
}

guint
ms_media_source_search (MsMediaSource *source,
                        const gchar *text,
                        const GList *keys,
                        const gchar *filter,
                        guint skip,
                        guint count,
                        MsMetadataResolutionFlags flags,
                        MsMediaSourceResultCb callback,
                        gpointer user_data)
{
  MsMediaSourceResultCb _callback;
  gpointer _user_data ;
  GList *_keys;
  struct SourceKeyMapList key_mapping;
  MsMediaSourceSearchSpec *ss;
  guint search_id;
  struct BrowseRelayCb *brc;

  g_return_val_if_fail (IS_MS_MEDIA_SOURCE (source), 0);
  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (count > 0, 0);
  g_return_val_if_fail (ms_metadata_source_supported_operations (MS_METADATA_SOURCE (source)) &
			MS_OP_SEARCH, 0);

  /* By default assume we will use the parameters specified by the user */
  _callback = callback;
  _user_data = user_data;
  _keys = g_list_copy ((GList *) keys);

  if (flags & MS_RESOLVE_FAST_ONLY) {
    g_debug ("requested fast keys only");
    ms_metadata_source_filter_slow (MS_METADATA_SOURCE (source), &_keys, FALSE);
  }

  if (flags & MS_RESOLVE_FULL) {
    g_debug ("requested full search");
    ms_metadata_source_setup_full_resolution_mode (MS_METADATA_SOURCE (source),
						   _keys, &key_mapping);
    
    /* If we do not have a source map for the unsupported keys then
       we cannot resolve any of them */
    if (key_mapping.source_maps != NULL) {
      struct FullResolutionCtlCb *c = g_new0 (struct FullResolutionCtlCb, 1);
      c->user_callback = callback;
      c->user_data = user_data;
      c->source_map_list = key_mapping.source_maps;
      c->flags = flags;
      
      _callback = full_resolution_ctl_cb;
      _user_data = c;
      g_list_free (_keys);
      _keys = key_mapping.operation_keys;
    }    
  }

  search_id = ms_media_source_gen_browse_id (source);

  brc = g_new0 (struct BrowseRelayCb, 1);
  brc->user_callback = _callback;
  brc->user_data = _user_data;
  brc->use_idle = flags & MS_RESOLVE_IDLE_RELAY;
  _callback = browse_result_relay_cb;
  _user_data = brc;

  ss = g_new0 (MsMediaSourceSearchSpec, 1);
  ss->source = g_object_ref (source);
  ss->search_id = search_id;
  ss->text = text ? g_strdup (text) : NULL;
  ss->filter = filter ? g_strdup (text) : NULL;
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

  g_idle_add (search_idle, ss);

  return search_id;
}

void
ms_media_source_metadata (MsMediaSource *source,
			  const gchar *object_id,
			  const GList *keys,
			  MsMetadataResolutionFlags flags,
			  MsMediaSourceMetadataCb callback,
			  gpointer user_data)
{
  MsMediaSourceMetadataCb _callback;
  gpointer _user_data ;
  GList *_keys;
  struct SourceKeyMapList key_mapping;
  MsMediaSourceMetadataSpec *ms;
  struct MetadataRelayCb *mrc;

  g_debug ("ms_media_source_metadata");

  g_return_if_fail (IS_MS_MEDIA_SOURCE (source));
  g_return_if_fail (keys != NULL);
  g_return_if_fail (callback != NULL);
  g_return_if_fail (ms_metadata_source_supported_operations (MS_METADATA_SOURCE (source)) &
		    MS_OP_METADATA);

  /* By default assume we will use the parameters specified by the user */
  _callback = callback;
  _user_data = user_data;
  _keys = g_list_copy ((GList *) keys);

  if (flags & MS_RESOLVE_FAST_ONLY) {
    ms_metadata_source_filter_slow (MS_METADATA_SOURCE (source),
				    &_keys, FALSE);
  }

  if (flags & MS_RESOLVE_FULL) {
    g_debug ("requested full metadata");
    ms_metadata_source_setup_full_resolution_mode (MS_METADATA_SOURCE (source),
						   _keys, &key_mapping);

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

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */
  mrc = g_new0 (struct MetadataRelayCb, 1);
  mrc->user_callback = _callback;
  mrc->user_data = _user_data;
  _callback = metadata_result_relay_cb;
  _user_data = mrc;

  ms = g_new0 (MsMediaSourceMetadataSpec, 1);
  ms->source = g_object_ref (source);
  ms->object_id = object_id ? g_strdup (object_id) : NULL;
  ms->keys = _keys; /* It is already a copy */
  ms->flags = flags;
  ms->callback = _callback;
  ms->user_data = _user_data;

  /* Save a reference to the operaton spec in the relay-cb's 
     user_data so that we can free the spec there */
  mrc->spec = ms;

  g_idle_add (metadata_idle, ms);
}

static MsSupportedOps
ms_media_source_supported_operations (MsMetadataSource *metadata_source)
{
  MsSupportedOps caps;
  MsMediaSource *source;
  MsMediaSourceClass *media_source_class;
  MsMetadataSourceClass *metadata_source_class;

  metadata_source_class =
    MS_METADATA_SOURCE_CLASS (ms_media_source_parent_class);
  source = MS_MEDIA_SOURCE (metadata_source);
  media_source_class = MS_MEDIA_SOURCE_GET_CLASS (source);

  caps = metadata_source_class->supported_operations (metadata_source);
  if (media_source_class->browse) 
    caps |= MS_OP_BROWSE;
  if (media_source_class->search)
    caps |= MS_OP_SEARCH;
  if (media_source_class->metadata) 
    caps |= MS_OP_METADATA;

  return caps;
}
