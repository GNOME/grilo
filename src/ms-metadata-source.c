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
#include "content/ms-content-media.h"

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

struct ResolveRelayCb {
  MsMetadataSourceResolveCb user_callback;
  gpointer user_data;
  MsMetadataSourceResolveSpec *spec;
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
    G_OBJECT_WARN_INVALID_PROPERTY_ID (source, prop_id, pspec);
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
    G_OBJECT_WARN_INVALID_PROPERTY_ID (source, prop_id, pspec);
    break;
  }  
}

/* ================ Utilities ================ */

static void __attribute__ ((unused))
print_keys (gchar *label, const GList *keys)
{
  g_print ("%s: [", label);
  while (keys) {
    g_print (" %" MS_KEYID_FORMAT, POINTER_TO_MSKEYID (keys->data));
    keys = g_list_next (keys);
  }
  g_print (" ]\n");
}

static void
resolve_result_relay_cb (MsMetadataSource *source,
			 MsContentMedia *media,
			 gpointer user_data,
			 const GError *error)
{
  g_debug ("resolve_result_relay_cb");

  struct ResolveRelayCb *rrc;

  rrc = (struct ResolveRelayCb *) user_data;
  rrc->user_callback (source, media, rrc->user_data, error);

  g_object_unref (rrc->spec->source);
  g_object_unref (rrc->spec->media);
  g_list_free (rrc->spec->keys);
  g_free (rrc->spec);
  g_free (rrc);
}

static gboolean
resolve_idle (gpointer user_data)
{
  g_debug ("resolve_idle");
  MsMetadataSourceResolveSpec *rs = (MsMetadataSourceResolveSpec *) user_data;
  MS_METADATA_SOURCE_GET_CLASS (rs->source)->resolve (rs->source, rs);
  return FALSE;
}

/* ================ API ================ */

const GList *
ms_metadata_source_supported_keys (MsMetadataSource *source)
{
  g_return_val_if_fail (MS_IS_METADATA_SOURCE (source), NULL);
  return MS_METADATA_SOURCE_GET_CLASS (source)->supported_keys (source);
}

const GList *
ms_metadata_source_slow_keys (MsMetadataSource *source)
{
  g_return_val_if_fail (MS_IS_METADATA_SOURCE (source), NULL);
  if (MS_METADATA_SOURCE_GET_CLASS (source)->slow_keys) {
    return MS_METADATA_SOURCE_GET_CLASS (source)->slow_keys (source);  
  } else {
    return NULL;
  }
}

const GList *
ms_metadata_source_key_depends (MsMetadataSource *source, MsKeyID key_id)
{
  g_return_val_if_fail (MS_IS_METADATA_SOURCE (source), NULL);
  return MS_METADATA_SOURCE_GET_CLASS (source)->key_depends (source, key_id);
}

void
ms_metadata_source_resolve (MsMetadataSource *source,
                            const GList *keys,
                            MsContentMedia *media,
			    guint flags,
                            MsMetadataSourceResolveCb callback,
                            gpointer user_data)
{
  MsMetadataSourceResolveSpec *rs;
  GList *_keys;
  struct ResolveRelayCb *rrc;

  g_debug ("ms_metadata_source_resolve");

  g_return_if_fail (MS_IS_METADATA_SOURCE (source));
  g_return_if_fail (callback != NULL);
  g_return_if_fail (media != NULL);
  g_return_if_fail (ms_metadata_source_supported_operations (source) &
		    MS_OP_RESOLVE);

  _keys = g_list_copy ((GList *) keys);

  if (flags & MS_RESOLVE_FAST_ONLY) {
    ms_metadata_source_filter_slow (source, &_keys, FALSE);
  }

  /* Always hook an own relay callback so we can do some
     post-processing before handing out the results
     to the user */
  rrc = g_new0 (struct ResolveRelayCb, 1);
  rrc->user_callback = callback;
  rrc->user_data = user_data;

  rs = g_new0 (MsMetadataSourceResolveSpec, 1);
  rs->source = g_object_ref (source);
  rs->keys = _keys;
  rs->media = g_object_ref (media);
  rs->flags = flags;
  rs->callback = resolve_result_relay_cb;
  rs->user_data = rrc;

  /* Save a reference to the operaton spec in the relay-cb's 
     user_data so that we can free the spec there */
  rrc->spec = rs;

  g_idle_add (resolve_idle, rs);
}

GList *
ms_metadata_source_filter_supported (MsMetadataSource *source,
				     GList **keys,
				     gboolean return_filtered)
{
  const GList *supported_keys;
  GList *iter_supported;
  GList *iter_keys;
  MsKeyID key;
  GList *filtered_keys = NULL;
  gboolean got_match;
  GList *iter_keys_prev;
  MsKeyID supported_key;

  g_return_val_if_fail (MS_IS_METADATA_SOURCE (source), NULL);

  supported_keys = ms_metadata_source_supported_keys (source);

  iter_keys = *keys;
  while (iter_keys) {
    got_match = FALSE;
    iter_supported = (GList *) supported_keys;

    key = POINTER_TO_MSKEYID (iter_keys->data);
    while (!got_match && iter_supported) {
      supported_key = POINTER_TO_MSKEYID (iter_supported->data);
      if (key == supported_key) {
	got_match = TRUE;
      }
      iter_supported = g_list_next (iter_supported);
    }

    iter_keys_prev = iter_keys;
    iter_keys = g_list_next (iter_keys);
    
    if (got_match) {
      if (return_filtered) {
	filtered_keys = g_list_prepend (filtered_keys, MSKEYID_TO_POINTER (key));
      }
      *keys = g_list_delete_link (*keys, iter_keys_prev);
      got_match = FALSE;
    }
  }

  return filtered_keys;
}

GList *
ms_metadata_source_filter_slow (MsMetadataSource *source,
				GList **keys,
				gboolean return_filtered)
{
  const GList *slow_keys;
  GList *iter_slow;
  GList *iter_keys;
  MsKeyID key;
  GList *filtered_keys = NULL;
  gboolean got_match;
  MsKeyID slow_key;

  g_return_val_if_fail (MS_IS_METADATA_SOURCE (source), NULL);

  slow_keys = ms_metadata_source_slow_keys (source);
  if (!slow_keys) {
    if (return_filtered) {
      return g_list_copy (*keys);
    } else {
      return NULL;
    }
  }

  iter_slow = (GList *) slow_keys;
  while (iter_slow) {
    got_match = FALSE;
    iter_keys = *keys;

    slow_key = POINTER_TO_MSKEYID (iter_slow->data);
    while (!got_match && iter_keys) {
      key = POINTER_TO_MSKEYID (iter_keys->data);
      if (key == slow_key) {
	got_match = TRUE;
      } else {
	iter_keys = g_list_next (iter_keys);
      }
    }

    iter_slow = g_list_next (iter_slow);
    
    if (got_match) {
      if (return_filtered) {
	filtered_keys =
	  g_list_prepend (filtered_keys, MSKEYID_TO_POINTER (slow_key));
      }
      *keys = g_list_delete_link (*keys, iter_keys);
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
  key_mapping->source_maps = NULL;
  key_mapping->operation_keys = NULL;

  /* key_list holds keys to be resolved */
  GList *key_list = g_list_copy ((GList *) keys);
  
  /* Filter keys supported by this source */
  key_mapping->operation_keys = 
    ms_metadata_source_filter_supported (MS_METADATA_SOURCE (source),
					 &key_list, TRUE);
  
  if (key_list == NULL) {
    g_debug ("Source supports all requested keys");
    goto done;
  }
  
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
    supported_keys = ms_metadata_source_filter_supported (_source,
							  &key_list, TRUE);
    
    if (!supported_keys) {
      g_debug ("  Source does not support any of the keys, skipping.");
      continue;
    }
    
    g_debug ("  '%s' can resolve some keys, checking deps", name);
    
    /* Check the external dependencies for these supported keys */
    GList *supported_deps;
    GList *iter_prev;
    iter = supported_keys;
    while (iter) {
      MsKeyID key = POINTER_TO_MSKEYID (iter->data);
      GList *deps =
	g_list_copy ((GList *) ms_metadata_source_key_depends (_source, key));
      
      iter_prev = iter;
      iter = g_list_next (iter);
      
      /* deps == NULL means the key cannot be resolved 
	 by using only metadata */
      if (!deps) {
	g_debug ("    Key '%u' cannot be resolved from metadata", key);
	supported_keys = g_list_delete_link (supported_keys, iter_prev);
	key_list = g_list_prepend (key_list, MSKEYID_TO_POINTER (key));
	continue;
      }
      g_debug ("    Key '%u' might be resolved using external metadata", key);
      
      /* Check if the original source can solve these dependencies */
      supported_deps = 
	ms_metadata_source_filter_supported (MS_METADATA_SOURCE (source), 
					     &deps, TRUE);
      if (deps) {
	g_debug ("      Dependencies not supported by source, dropping key");
	/* Maybe some other source can still resolve it */
	/* TODO: maybe some of the sources already inspected could provide
	   these keys! */
	supported_keys = 
	  g_list_delete_link (supported_keys, iter_prev);
	/* Put the key back in the list, maybe some other soure can
	   resolve it */
	key_list = g_list_prepend (key_list, MSKEYID_TO_POINTER (key));
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
      source_key_map->source = g_object_ref (_source);
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
  g_return_val_if_fail (MS_IS_METADATA_SOURCE (source), NULL);
  gchar *r = NULL;
  if (source->priv->id) {
    r = g_strdup (source->priv->id);
  }
  return r;
}

gchar *
ms_metadata_source_get_name (MsMetadataSource *source)
{
  g_return_val_if_fail (MS_IS_METADATA_SOURCE (source), NULL);
  gchar *r = NULL;
  if (source->priv->name) {
    r = g_strdup (source->priv->name);
  }
  return r;
}

gchar *
ms_metadata_source_get_description (MsMetadataSource *source)
{
  g_return_val_if_fail (MS_IS_METADATA_SOURCE (source), NULL);
  gchar *r = NULL;
  if (source->priv->desc) {
    r = g_strdup (source->priv->desc);
  }
  return r;
}

MsSupportedOps
ms_metadata_source_supported_operations (MsMetadataSource *source)
{
  g_return_val_if_fail (MS_IS_METADATA_SOURCE (source), MS_OP_NONE);
  return MS_METADATA_SOURCE_GET_CLASS (source)->supported_operations (source);
}

static MsSupportedOps
ms_metadata_source_supported_operations_impl (MsMetadataSource *source)
{
  MsSupportedOps caps = MS_OP_NONE;
  MsMetadataSourceClass *metadata_source_class;
  metadata_source_class = MS_METADATA_SOURCE_GET_CLASS (source);
  if (metadata_source_class->resolve)
    caps |= MS_OP_RESOLVE;
  return caps;
}
