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

#include "plugin-registry.h"
#include "media-plugin-priv.h"

#include <string.h>
#include <gmodule.h>

#define SYSTEM_KEYS_MAX 256

#define PLUGIN_REGISTRY_GET_PRIVATE(object)				\
  (G_TYPE_INSTANCE_GET_PRIVATE((object), PLUGIN_REGISTRY_TYPE, PluginRegistryPrivate))

#define REGISTER_SYSTEM_METADATA_KEY(r, key)		\
  { r->priv->system_keys[key].id = key;			\
    r->priv->system_keys[key].name = key##_NAME;	\
    r->priv->system_keys[key].desc = key##_DESC;	\
  }

struct _PluginRegistryPrivate {
  GHashTable *plugins;
  GHashTable *sources;
  MetadataKey *system_keys;
};

G_DEFINE_TYPE (PluginRegistry, plugin_registry, G_TYPE_OBJECT);

static void
plugin_registry_class_init (PluginRegistryClass *klass)
{
  if (!g_module_supported ()) {
    g_error ("GModule not supported in this system");
  }

  g_type_class_add_private (klass, sizeof (PluginRegistryPrivate));
}

static void
plugin_registry_setup_system_keys (PluginRegistry *registry)
{
  registry->priv->system_keys = g_new0 (MetadataKey, SYSTEM_KEYS_MAX);

  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_TITLE);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_URL);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_ARTIST);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_ALBUM);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_GENRE);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_THUMBNAIL);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_ID);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_AUTHOR);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_DESCRIPTION);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_SOURCE);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_LYRICS);
  REGISTER_SYSTEM_METADATA_KEY (registry, METADATA_KEY_SITE);
}

static void
plugin_registry_init (PluginRegistry *registry)
{
  registry->priv = PLUGIN_REGISTRY_GET_PRIVATE (registry);
  memset (registry->priv, 0, sizeof (PluginRegistryPrivate));

  registry->priv->plugins = g_hash_table_new (g_str_hash, g_str_equal);
  registry->priv->sources = g_hash_table_new (g_str_hash, g_str_equal);

  plugin_registry_setup_system_keys (registry);
}

PluginRegistry *
plugin_registry_get_instance (void)
{
  static PluginRegistry *registry = NULL;

  if (!registry) {
    registry = g_object_new (PLUGIN_REGISTRY_TYPE, NULL);
  }

  return registry;
}

gboolean
plugin_registry_register_source (PluginRegistry *registry, 
				 const PluginInfo *plugin,
				 MediaPlugin *source)
{
  gchar *id;

  g_object_get (source, "source-id", &id, NULL);
  g_debug ("New media source available: '%s'", id);

  g_hash_table_insert (registry->priv->sources, id, source); 

  media_plugin_set_plugin_info (source, plugin);

  return TRUE;
}

gboolean
plugin_registry_load (PluginRegistry *registry, const gchar *path)
{
  GModule *module;
  PluginDescriptor *plugin;

  module = g_module_open (path, G_MODULE_BIND_LAZY);
  if (!module) {
    g_warning ("Failed to open module: '%s'", path);
    return FALSE;
  }

  if (!g_module_symbol (module, "PLUGIN_DESCRIPTOR", (gpointer) &plugin)) {
    g_warning ("Did not find plugin descriptor: '%s'", path);
    return FALSE;
  }

  if (!plugin->plugin_init || 
      !plugin->info.id || 
      !plugin->info.name) {
    g_warning ("Plugin descriptor is not valid: '%s'", path);
    return FALSE;
  }

  g_hash_table_insert (registry->priv->plugins, 
		       (gpointer) plugin->info.id, plugin);
  
  if (!plugin->plugin_init (registry, &plugin->info)) {
    g_hash_table_remove (registry->priv->plugins, plugin->info.id);
    g_warning ("Failed to initialize plugin: '%s'", path);
    return FALSE;
  }

  g_debug ("Loaded plugin '%s' from '%s'", plugin->info.id, path);

  return TRUE;
}

gboolean
plugin_registry_load_all (PluginRegistry *registry)
{
  const gchar *plugin_path, *entry;
  GDir *dir;
  gchar *file;
  
  plugin_path = g_getenv (PLUGIN_PATH_VAR);
  dir = g_dir_open (plugin_path, 0, NULL);

  if (!dir) {
    g_warning ("Could not open plugin directory: '%s'", plugin_path);
    return FALSE;
  }

  while ((entry = g_dir_read_name (dir)) != NULL) {
    if (g_str_has_suffix (entry, "." G_MODULE_SUFFIX)) {
      file = g_build_filename (plugin_path, entry, NULL);
      plugin_registry_load (registry, file);
      g_free (file);
    }
  }

  g_dir_close (dir);

  return TRUE;
}

MediaPlugin *
plugin_registry_lookup_source (PluginRegistry *registry, const gchar *source_id)
{
  return (MediaPlugin *) g_hash_table_lookup (registry->priv->sources, source_id);
}

MediaPlugin **
plugin_registry_get_sources (PluginRegistry *registry)
{
  GHashTableIter iter;
  MediaPlugin **source_list;
  guint n;

  n = g_hash_table_size (registry->priv->sources);
  source_list = (MediaPlugin **) g_new0 (MediaPlugin *, n + 1);


  n = 0;
  g_hash_table_iter_init (&iter, registry->priv->sources);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &source_list[n++]));

  return source_list;
}

void
plugin_registry_unload (PluginRegistry *registry, const gchar *plugin_id)
{
  PluginDescriptor *plugin;

  plugin = g_hash_table_lookup (registry->priv->plugins, plugin_id);
  if (!plugin) {
    g_warning ("Could not deinit plugin '%s'. Plugin not found.", plugin_id);
    return;
  }

  g_debug ("Unloding plugin '%s'", plugin_id);
  if (plugin->plugin_deinit) {
    plugin->plugin_deinit ();
  }
}

const MetadataKey *
plugin_registry_lookup_metadata_key (PluginRegistry *registry, KeyID key_id)
{
  return &registry->priv->system_keys[key_id];
}
