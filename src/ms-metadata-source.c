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

static void ms_metadata_source_finalize (GObject *plugin);
static void ms_metadata_source_get_property (GObject *plugin,
                                             guint prop_id,
                                             GValue *value,
                                             GParamSpec *pspec);
static void ms_metadata_source_set_property (GObject *object, 
                                             guint prop_id,
                                             const GValue *value,
                                             GParamSpec *pspec);

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

G_DEFINE_ABSTRACT_TYPE (MsMetadataSource, ms_metadata_source, MS_TYPE_MEDIA_PLUGIN);

static void
ms_metadata_source_class_init (MsMetadataSourceClass *metadata_source_class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (metadata_source_class);

  gobject_class->finalize = ms_metadata_source_finalize;
  gobject_class->set_property = ms_metadata_source_set_property;
  gobject_class->get_property = ms_metadata_source_get_property;
  
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

const GList *
ms_metadata_source_supported_keys (MsMetadataSource *source)
{
  return MS_METADATA_SOURCE_GET_CLASS (source)->supported_keys (source);
}

const GList *
ms_metadata_source_key_depends (MsMetadataSource *source, MsKeyID key_id)
{
  return MS_METADATA_SOURCE_GET_CLASS (source)->key_depends (source, key_id);
}

void
ms_metadata_source_get (MsMetadataSource *source,
		     const gchar *object_id,
		     const GList *keys,
		     MsMetadataSourceResultCb callback,
		     gpointer user_data)
{
  MS_METADATA_SOURCE_GET_CLASS (source)->metadata (source,
                                                   object_id,
                                                   keys,
                                                   callback, user_data);
}

void
ms_metadata_source_resolve (MsMetadataSource *source, 
                            const GList *keys, 
                            MsContent *media,
                            MsMetadataSourceResolveCb callback,
                            gpointer user_data)
{
  MS_METADATA_SOURCE_GET_CLASS (source)->resolve (source,
                                                  keys,
                                                  media,
                                                  callback,
                                                  user_data);
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
