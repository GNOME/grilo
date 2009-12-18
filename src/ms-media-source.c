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
#include "ms-plugin-registry.h"

#include <string.h>

#define MS_MEDIA_SOURCE_GET_PRIVATE(object)				\
  (G_TYPE_INSTANCE_GET_PRIVATE((object), MS_TYPE_MEDIA_SOURCE, MsMediaSourcePrivate))

struct _MsMediaSourcePrivate {
  guint padding;
};

struct FullResolutionCtlCb {
  MsMediaSourceResultCb user_callback;
  gpointer user_data;
  GList *keys;
  GList *source_map_list;
};

struct FullResolutionDoneCb {
  MsMediaSourceResultCb user_callback;
  gpointer user_data;
  guint pending_callbacks;
  MsMediaSource *source;
  guint browse_id;
  guint remaining;
};

struct SourceKeyMap {
  MsMetadataSource *source;
  GList *keys;
};

static void
print_keys (gchar *label, const GList *keys)
{
  g_print ("%s: [", label);
  while (keys) {
    g_print (" %d", GPOINTER_TO_INT (keys->data));
    keys = g_list_next (keys);
  }
  g_print (" ]\n");
}

static void
media_source_browse_full_resolution_done_cb (MsMetadataSource *source,
					     MsContent *media,
					     gpointer user_data,
					     const GError *error)
{
  g_debug ("media_source_browse_full_resolution_done_cb");

  struct FullResolutionDoneCb *cb_info = 
    (struct FullResolutionDoneCb *) user_data;

  cb_info->pending_callbacks--;

  if (error) {
    g_warning ("Failed to fully resolve some metadata: %s", error->message);
  }

  if (cb_info->pending_callbacks == 0) {
    cb_info->user_callback (cb_info->source, 
			    cb_info->browse_id, 
			    media,
			    cb_info->remaining, 
			    cb_info->user_data,
			    NULL);
  }
}

static void
ms_media_source_browse_full_resolution_ctl_cb (MsMediaSource *source,
                                               guint browse_id,
                                               MsContent *media,
                                               guint remaining,
                                               gpointer user_data,
                                               const GError *error)
{
  GList *iter;

  struct FullResolutionCtlCb *ctl_info =
    (struct FullResolutionCtlCb *) user_data;

  g_debug ("media_source_browse_full_resolution_cb");

  /* If we got an error, invoke the user callback right away and bail out */
  if (error) {
    g_warning ("Browse operation failed: %s", error->message);
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

  /* Use sources in the map to fill in missing metadata, the "done"
     callback will be used to emit the resulting object when 
     all metadata has been gathered */
  iter = ctl_info->source_map_list;
  while (iter) {
    gchar *name;
    struct SourceKeyMap *map = (struct SourceKeyMap *) iter->data;
    g_object_get (map->source, "source-name", &name, NULL);
    g_debug ("Using '%s' to resolve extra metadata now", name);

    ms_metadata_source_resolve (map->source, 
			     map->keys, 
			     media, 
			     media_source_browse_full_resolution_done_cb,
			     done_info);

    iter = g_list_next (iter);
  }
}

G_DEFINE_ABSTRACT_TYPE (MsMediaSource, ms_media_source, MS_TYPE_METADATA_SOURCE);

static void
ms_media_source_class_init (MsMediaSourceClass *media_source_class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (media_source_class);

  g_type_class_add_private (media_source_class, sizeof (MsMediaSourcePrivate));
}

static void
ms_media_source_init (MsMediaSource *source)
{
  source->priv = MS_MEDIA_SOURCE_GET_PRIVATE (source);
  memset (source->priv, 0, sizeof (MsMediaSourcePrivate));
}

guint
ms_media_source_browse (MsMediaSource *source, 
		     const gchar *container_id,
		     const GList *keys,
		     guint skip,
		     guint count,
		     guint flags,
		     MsMediaSourceResultCb callback,
		     gpointer user_data)
{
  MsMediaSourceResultCb _callback;
  gpointer _user_data;
  GList *_keys;
  
  /* By default we browse as the user requested */
  _callback = callback;
  _user_data = user_data;
  _keys = (GList *) keys;

  if (flags & MS_METADATA_RESOLUTION_FULL) {
    g_debug ("requested full browse");

    print_keys ("Requested keys", keys);

    /* key_list holds keys to be resolved */
    GList *key_list = g_list_copy ((GList *) keys);

    /* Filter keys supported by this source */
    GList *keys_to_browse = 
      ms_metadata_source_filter_supported (MS_METADATA_SOURCE (source), &key_list);

    if (key_list == NULL) {
      g_debug ("Source supports all requested keys");
      goto done;
    }

    print_keys ("Keys supported in source", keys_to_browse);

    /*
     * 1) Find which sources (other than the current one) can resolve
     *    some of the missing keys
     * 2) Check out dependencies for the keys they can resolve 
     * 3) For each dependency list check if the  original source can resolve them.
     *    3.1) Yes: Add key and dependencies to be resolved
     *    3.2) No: forget about that key and its dependencies
     *         Ideally, we would check if other sources can resolve them
     * 4) Execute the browse operation passing in our own callback
     * 5) For each browse result, check the sources that can resolve
     *    the missing metadata and issue operations on them.
     *    We could do this once per source passing in a list with all the 
     *    browse results. Problem is we lose response time although we gain
     *    overall efficiency.
     * 6) Invoke user callback with results
     */

    /* Find which sources resolve which keys */
    GList *supported_keys;
    MsMetadataSource *_source;
    MsMediaPlugin **source_list;
    GList *source_map_list = NULL;
    GList *iter;
    MsPluginRegistry *registry;

    registry = ms_plugin_registry_get_instance ();
    source_list = ms_plugin_registry_get_sources (registry);

    while (*source_list && key_list) {
      gchar *name;

      _source = MS_METADATA_SOURCE (*source_list);

      source_list++;

      /* Interested in sources other than this  */
      if (_source == MS_METADATA_SOURCE (source)) {
	continue;
      }

      print_keys ("Unsupported keys", key_list);

      /* Interested in sources capable of resolving metadata
	 based on other metadata */
      MsMetadataSourceClass *_source_class = MS_METADATA_SOURCE_GET_CLASS (_source);
      if (!_source_class->resolve) {
	continue;
      }

      /* Check if this source supports some of the missing keys */
      g_object_get (_source, "source-name", &name, NULL);
      g_debug ("Checking resolution capabilities for source '%s'", name);
      supported_keys = ms_metadata_source_filter_supported (_source, &key_list);
      
      if (!supported_keys) {
	g_debug ("  Source does not support any of the keys, skipping.");
	continue;
      }

      g_debug ("  '%s' can resolve some keys, checking deps", name);
      print_keys ("Keys supported by source", supported_keys);

      /* Check the external dependencies for these supported keys */
      GList *supported_deps;
      GList *iter_prev;
      iter = supported_keys;
      while (iter) {
	MsKeyID key = GPOINTER_TO_INT (iter->data);
	GList *deps =
	  g_list_copy ((GList *) ms_metadata_source_key_depends (_source, key));

	iter_prev = iter;
	iter = g_list_next (iter);

	/* deps == NULL means the key cannot be resolved 
	   by using only metadata */
	if (!deps) {
	  g_debug ("    Key '%u' cannot be resolved from metadata", key);
	  supported_keys = g_list_delete_link (supported_keys, iter_prev);
	  key_list = g_list_prepend (key_list, GINT_TO_POINTER (key));
	  continue;
	}
	g_debug ("    Key '%u' might be resolved using external metadata", key);

	/* Check if the original source can solve these dependencies */
	supported_deps = 
	  ms_metadata_source_filter_supported (MS_METADATA_SOURCE (source), 
					    &deps);
	if (deps) {
	  g_debug ("      Dependencies not supported by source, dropping key");
	  /* Maybe some other source can still resolve it */
	  /* TODO: maybe some of the sources already inspected could provide
	     these keys! */
	  supported_keys = 
	    g_list_delete_link (supported_keys, iter_prev);
	  /* Put the key back in the list, maybe some other soure can
	     resolve it */
	  key_list = g_list_prepend (key_list, GINT_TO_POINTER (key));
	} else {
	  g_debug ("      Dependencies supported by source, including key");
	  /* Add these dependencies to the list of keys for
	     the browse operation */
	  /* TODO: maybe some of these keys are in the list already! */
	  keys_to_browse = g_list_concat (keys_to_browse, supported_deps);
	}
      }

      /* Save the key map for this source */
      if (supported_keys) {
	g_debug ("  Adding source '%s' to the resolution map", name);
	struct SourceKeyMap *source_key_map = g_new (struct SourceKeyMap, 1);
	source_key_map->source = _source;
	source_key_map->keys = supported_keys;
	source_map_list = g_list_prepend (source_map_list, source_key_map);
      }
    }

    /* If we do not have a source map for the unsupported keys then
       we cannot resolve any of them */
    if (!source_map_list) {
      g_debug ("No keymap, can't resolve more metadata");
      goto done;
    }

    /* Otherwise, save user callback, we will need to hook our own */
    struct FullResolutionCtlCb *c = g_new0 (struct FullResolutionCtlCb, 1);
    c->user_callback = callback;
    c->user_data = user_data;
    c->keys = (GList *) keys;
    c->source_map_list = source_map_list;
    
    _callback = ms_media_source_browse_full_resolution_ctl_cb;
    _user_data = c;
    _keys = keys_to_browse;
  }

done:
  return MS_MEDIA_SOURCE_GET_CLASS (source)->browse (source,
                                                     container_id,
                                                     _keys,
                                                     skip, count,
                                                     _callback, _user_data);
}

guint
ms_media_source_search (MsMediaSource *source,
                        const gchar *text,
                        const GList *keys,
                        const gchar *filter,
                        guint skip,
                        guint count,
                        guint flags,
                        MsMediaSourceResultCb callback,
                        gpointer user_data)
{
  return MS_MEDIA_SOURCE_GET_CLASS (source)->search (source,
                                                     text,
                                                     keys,
                                                     filter,
                                                     skip, count,
                                                     callback, user_data);
}
