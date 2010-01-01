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

#include "ms-metadata-source.h"
#include "ms-metadata-source-priv.h"
#include "ms-plugin-registry.h"

#include <string.h>

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "ms-metadata-source"

#define MS_METADATA_SOURCE_GET_PRIVATE(object)				\
  (G_TYPE_INSTANCE_GET_PRIVATE((object), MS_TYPE_METADATA_SOURCE, MsMetadataSourcePrivate))

enum {
  PROP_0,
  PROP_ID,
  PROP_NAME,
  PROP_DESC,
};

struct _MsMetadataSourcePrivate {
  gchar *id;
  gchar *name;
  gchar *desc;
};

struct FullResolutionCtlCb {
  MsMetadataSourceResultCb user_callback;
  gpointer user_data;
  GList *source_map_list;
};

struct FullResolutionDoneCb {
  MsMetadataSourceResultCb user_callback;
  gpointer user_data;
  guint pending_callbacks;
  MsMetadataSource *source;
};

static void ms_metadata_source_finalize (GObject *plugin);
static void ms_metadata_source_get_property (GObject *plugin,
                                             guint prop_id,
                                             GValue *value,
                                             GParamSpec *pspec);
static void ms_metadata_source_set_property (GObject *object, 
                                             guint prop_id,
                                             const GValue *value,
                                             GParamSpec *pspec);

static MsSupportedOps ms_metadata_source_supported_operations_impl (MsMetadataSource *source);

/* ================ MsMetadataSource GObject ================ */

G_DEFINE_ABSTRACT_TYPE (MsMetadataSource, ms_metadata_source, MS_TYPE_MEDIA_PLUGIN);

static void
ms_metadata_source_class_init (MsMetadataSourceClass *metadata_source_class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (metadata_source_class);

  gobject_class->finalize = ms_metadata_source_finalize;
  gobject_class->set_property = ms_metadata_source_set_property;
  gobject_class->get_property = ms_metadata_source_get_property;

  metadata_source_class->supported_operations =
    ms_metadata_source_supported_operations_impl;

  /**
   * MsMetadataSource:source-id
   *
   * The identifier of the source.
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
   * MsMetadataSource:source-name
   *
   * The name of the source.
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
   * MsMetadataSource:source-desc
   *
   * A description of the source
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

  g_type_class_add_private (metadata_source_class, sizeof (MsMetadataSourcePrivate));
}

static void
ms_metadata_source_init (MsMetadataSource *source)
{
  source->priv = MS_METADATA_SOURCE_GET_PRIVATE (source);
  memset (source->priv, 0, sizeof (MsMetadataSourcePrivate));
}

static void
ms_metadata_source_finalize (GObject *object)
{
  MsMetadataSource *source;
  
  source = MS_METADATA_SOURCE (object);

  g_free (source->priv->id);
  g_free (source->priv->name);
  g_free (source->priv->desc);
  
  G_OBJECT_CLASS (ms_metadata_source_parent_class)->finalize (object);
}


static void
set_string_property (gchar **property, const GValue *value)
{
  if (*property) {
    g_free (*property);
  }
  *property = g_value_dup_string (value);
}

static void
ms_metadata_source_set_property (GObject *object, 
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  MsMetadataSource *source;
  
  source = MS_METADATA_SOURCE (object);
  
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
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(source, prop_id, pspec);
    break;
  }
}

static void
ms_metadata_source_get_property (GObject *object,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  MsMetadataSource *source;

  source = MS_METADATA_SOURCE (object);

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
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(source, prop_id, pspec);
    break;
  }  
}

/* ================ Utilities ================ */

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

static gboolean
metadata_idle (gpointer user_data)
{
  g_debug ("metadata_idle");
  MsMetadataSourceMetadataSpec *ms = (MsMetadataSourceMetadataSpec *) user_data;
  MS_METADATA_SOURCE_GET_CLASS (ms->source)->metadata (ms->source, ms);
  return FALSE;
}

static gboolean
resolve_idle (gpointer user_data)
{
  g_debug ("resolve_idle");
  MsMetadataSourceResolveSpec *rs = (MsMetadataSourceResolveSpec *) user_data;
  MS_METADATA_SOURCE_GET_CLASS (rs->source)->resolve (rs->source, rs);
  return FALSE;
}

static void
full_resolution_done_cb (MsMetadataSource *source,
			 MsContent *media,
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

  if (cb_info->pending_callbacks == 0) {
    cb_info->user_callback (cb_info->source, 
			    media,
			    cb_info->user_data,
			    NULL);
  }
}

static void
full_resolution_ctl_cb (MsMetadataSource *source,
			MsContent *media,
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
			     media,
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
				full_resolution_done_cb,
				done_info);
    
    iter = g_list_next (iter);
  }
}

/* ================ API ================ */

const GList *
ms_metadata_source_supported_keys (MsMetadataSource *source)
{
  g_return_val_if_fail (IS_MS_METADATA_SOURCE (source), NULL);
  return MS_METADATA_SOURCE_GET_CLASS (source)->supported_keys (source);
}

const GList *
ms_metadata_source_key_depends (MsMetadataSource *source, MsKeyID key_id)
{
  g_return_val_if_fail (IS_MS_METADATA_SOURCE (source), NULL);
  return MS_METADATA_SOURCE_GET_CLASS (source)->key_depends (source, key_id);
}

void
ms_metadata_source_get (MsMetadataSource *source,
			const gchar *object_id,
			const GList *keys,
			guint flags,
			MsMetadataSourceResultCb callback,
			gpointer user_data)
{
  MsMetadataSourceResultCb _callback;
  gpointer _user_data ;
  GList *_keys;
  struct SourceKeyMapList key_mapping;
  MsMetadataSourceMetadataSpec *ms;

  g_return_if_fail (IS_MS_METADATA_SOURCE (source));
  g_return_if_fail (keys != NULL);
  g_return_if_fail (callback != NULL);
  g_return_if_fail (ms_metadata_source_supported_operations (source) &
		    MS_OP_METADATA);

  /* TODO: Does it make sense to implement relay for this? */

  /* By default assume we will use the parameters specified by the user */
  _callback = callback;
  _user_data = user_data;
  _keys = (GList *) keys;

  if (flags & MS_RESOLVE_FULL) {
    g_debug ("requested full get");
    ms_metadata_source_setup_full_resolution_mode (source, keys, &key_mapping);

    /* If we do not have a source map for the unsupported keys then
       we cannot resolve any of them */
    if (key_mapping.source_maps != NULL) {
      struct FullResolutionCtlCb *c = g_new0 (struct FullResolutionCtlCb, 1);
      c->user_callback = callback;
      c->user_data = user_data;
      c->source_map_list = key_mapping.source_maps;
      
      _callback = full_resolution_ctl_cb;
      _user_data = c;
      _keys = key_mapping.operation_keys;
    }    
  }

  ms = g_new0 (MsMetadataSourceMetadataSpec, 1);
  ms->source = g_object_ref (source);
  ms->object_id = object_id ? g_strdup (object_id) : NULL;
  ms->keys = g_list_copy (_keys);
  ms->callback = _callback;
  ms->user_data = _user_data;

  g_idle_add (metadata_idle, ms);
}

void
ms_metadata_source_resolve (MsMetadataSource *source, 
                            const GList *keys, 
                            MsContent *media,
                            MsMetadataSourceResolveCb callback,
                            gpointer user_data)
{
  MsMetadataSourceResolveSpec *rs;

  g_return_if_fail (IS_MS_METADATA_SOURCE (source));
  g_return_if_fail (callback != NULL);
  g_return_if_fail (media != NULL);
  g_return_if_fail (ms_metadata_source_supported_operations (source) &
		    MS_OP_RESOLVE);

  rs = g_new0 (MsMetadataSourceResolveSpec, 1);
  rs->source = g_object_ref (source);
  rs->keys = g_list_copy ((GList *) keys);
  rs->media = g_object_ref (media);
  rs->callback = callback;
  rs->user_data = user_data;

  g_idle_add (resolve_idle, rs);
}

GList *
ms_metadata_source_filter_supported (MsMetadataSource *source, GList **keys)
{
  const GList *supported_keys;
  GList *iter_supported;
  GList *iter_keys;
  MsKeyID key;
  GList *filtered_keys = NULL;
  gboolean got_match;
  GList *iter_keys_prev;
  MsKeyID supported_key;

  g_return_val_if_fail (IS_MS_METADATA_SOURCE (source), NULL);

  supported_keys = ms_metadata_source_supported_keys (source);

  iter_keys = *keys;
  while (iter_keys) {
    got_match = FALSE;
    iter_supported = (GList *) supported_keys;

    key = GPOINTER_TO_INT (iter_keys->data);
    while (!got_match && iter_supported) {
      supported_key = GPOINTER_TO_INT (iter_supported->data);
      if (key == supported_key) {
	got_match = TRUE;
      }
      iter_supported = g_list_next (iter_supported);
    }

    iter_keys_prev = iter_keys;
    iter_keys = g_list_next (iter_keys);
    
    if (got_match) {
      filtered_keys = g_list_prepend (filtered_keys, GINT_TO_POINTER (key));
      *keys = g_list_delete_link (*keys, iter_keys_prev);
      got_match = FALSE;
    }
  }

  return filtered_keys;
}

void
ms_metadata_source_setup_full_resolution_mode (MsMetadataSource *source,
					       const GList *keys,
					       struct SourceKeyMapList *key_mapping)
{
  print_keys ("Requested keys", keys);
  
  key_mapping->source_maps = NULL;
  key_mapping->operation_keys = NULL;

  /* key_list holds keys to be resolved */
  GList *key_list = g_list_copy ((GList *) keys);
  
  /* Filter keys supported by this source */
  key_mapping->operation_keys = 
    ms_metadata_source_filter_supported (MS_METADATA_SOURCE (source),
					 &key_list);
  
  if (key_list == NULL) {
    g_debug ("Source supports all requested keys");
    goto done;
  }
  
  print_keys ("Keys supported in source", key_mapping->operation_keys);
  
  /*
   * 1) Find which sources (other than the current one) can resolve
   *    some of the missing keys
   * 2) Check out dependencies for the keys they can resolve 
   * 3) For each dependency list check if the  original source can resolve them.
   *    3.1) Yes: Add key and dependencies to be resolved
   *    3.2) No: forget about that key and its dependencies
   *         Ideally, we would check if other sources can resolve them
   * 4) Execute the user operation passing in our own callback
   * 5) For each result, check the sources that can resolve
   *    the missing metadata and issue resolution operations on them.
   *    We could do this once per source passing in a list with all the 
   *    browse results. Problem is we lose response time although we gain
   *    overall efficiency.
   * 6) Invoke user callback with results
   */
  
  /* Find which sources resolve which keys */
  GList *supported_keys;
  MsMetadataSource *_source;
  MsMediaPlugin **source_list;
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
    MsMetadataSourceClass *_source_class =
      MS_METADATA_SOURCE_GET_CLASS (_source);
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
	key_mapping->operation_keys =
	  g_list_concat (key_mapping->operation_keys, supported_deps);
      }
    }
    
    /* Save the key map for this source */
    if (supported_keys) {
      g_debug ("  Adding source '%s' to the resolution map", name);
      struct SourceKeyMap *source_key_map = g_new (struct SourceKeyMap, 1);
      source_key_map->source = _source;
      source_key_map->keys = supported_keys;
      key_mapping->source_maps =
	g_list_prepend (key_mapping->source_maps, source_key_map);
    }
  }

  if (key_mapping->source_maps == NULL) {
      g_debug ("No key mapping for other sources, can't resolve more metadata");
  }
done:
  return;
}

gchar *
ms_metadata_source_get_id (MsMetadataSource *source)
{
  g_return_val_if_fail (IS_MS_METADATA_SOURCE (source), NULL);
  gchar *r = NULL;
  if (source->priv->id) {
    r = g_strdup (source->priv->id);
  }
  return r;
}

gchar *
ms_metadata_source_get_name (MsMetadataSource *source)
{
  g_return_val_if_fail (IS_MS_METADATA_SOURCE (source), NULL);
  gchar *r = NULL;
  if (source->priv->name) {
    r = g_strdup (source->priv->name);
  }
  return r;
}

gchar *
ms_metadata_source_get_description (MsMetadataSource *source)
{
  g_return_val_if_fail (IS_MS_METADATA_SOURCE (source), NULL);
  gchar *r = NULL;
  if (source->priv->desc) {
    r = g_strdup (source->priv->desc);
  }
  return r;
}

MsSupportedOps
ms_metadata_source_supported_operations (MsMetadataSource *source)
{
  g_return_val_if_fail (IS_MS_METADATA_SOURCE (source), MS_OP_NONE);
  return MS_METADATA_SOURCE_GET_CLASS (source)->supported_operations (source);
}

static MsSupportedOps
ms_metadata_source_supported_operations_impl (MsMetadataSource *source)
{
  MsSupportedOps caps = MS_OP_NONE;
  MsMetadataSourceClass *metadata_source_class;
  metadata_source_class = MS_METADATA_SOURCE_GET_CLASS (source);
  if (metadata_source_class->metadata) 
    caps |= MS_OP_METADATA;
  if (metadata_source_class->resolve)
    caps |= MS_OP_RESOLVE;
  return caps;
}
