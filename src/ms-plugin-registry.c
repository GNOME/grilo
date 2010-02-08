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

#include "ms-plugin-registry.h"
#include "ms-media-plugin-priv.h"

#include <string.h>
#include <gmodule.h>

#define SYSTEM_KEYS_MAX 256

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "ms-plugin-registry"

#define MS_PLUGIN_REGISTRY_GET_PRIVATE(object)				\
  (G_TYPE_INSTANCE_GET_PRIVATE((object), MS_TYPE_PLUGIN_REGISTRY, MsPluginRegistryPrivate))

#define MS_REGISTER_SYSTEM_METADATA_KEY(r, key)		\
  { r->priv->system_keys[key].id = key;			\
    r->priv->system_keys[key].name = key##_NAME;	\
    r->priv->system_keys[key].desc = key##_DESC;	\
  }

struct _MsPluginRegistryPrivate {
  GHashTable *plugins;
  GHashTable *sources;
  MsMetadataKey *system_keys;
};

static void ms_plugin_registry_setup_system_keys (MsPluginRegistry *registry);

/* ================ MsPluginRegistry GObject ================ */

G_DEFINE_TYPE (MsPluginRegistry, ms_plugin_registry, G_TYPE_OBJECT);

static void
ms_plugin_registry_class_init (MsPluginRegistryClass *klass)
{
  if (!g_module_supported ()) {
    g_error ("GModule not supported in this system");
  }

  g_type_class_add_private (klass, sizeof (MsPluginRegistryPrivate));
}

static void
ms_plugin_registry_init (MsPluginRegistry *registry)
{
  registry->priv = MS_PLUGIN_REGISTRY_GET_PRIVATE (registry);
  memset (registry->priv, 0, sizeof (MsPluginRegistryPrivate));

  registry->priv->plugins = g_hash_table_new (g_str_hash, g_str_equal);
  registry->priv->sources = g_hash_table_new (g_str_hash, g_str_equal);

  ms_plugin_registry_setup_system_keys (registry);
}

/* ================ Utitilies ================ */

static void
ms_plugin_registry_setup_system_keys (MsPluginRegistry *registry)
{
  registry->priv->system_keys = g_new0 (MsMetadataKey, SYSTEM_KEYS_MAX);

  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_TITLE);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_URL);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_ARTIST);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_ALBUM);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_GENRE);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_THUMBNAIL);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_ID);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_AUTHOR);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_DESCRIPTION);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_SOURCE);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_LYRICS);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_SITE);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_DURATION);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_DATE);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_CHILDCOUNT);
  MS_REGISTER_SYSTEM_METADATA_KEY (registry, MS_METADATA_KEY_MIME);
}

/* ================ API ================ */

MsPluginRegistry *
ms_plugin_registry_get_instance (void)
{
  static MsPluginRegistry *registry = NULL;

  if (!registry) {
    registry = g_object_new (MS_TYPE_PLUGIN_REGISTRY, NULL);
  }

  return registry;
}

gboolean
ms_plugin_registry_register_source (MsPluginRegistry *registry,
                                    const MsPluginInfo *plugin,
                                    MsMediaPlugin *source)
{
  gchar *id;

  g_object_get (source, "source-id", &id, NULL);
  g_debug ("New media source available: '%s'", id);

  g_hash_table_insert (registry->priv->sources, id, source); 

  ms_media_plugin_set_plugin_info (source, plugin);

  return TRUE;
}

gboolean
ms_plugin_registry_load (MsPluginRegistry *registry, const gchar *path)
{
  GModule *module;
  MsPluginDescriptor *plugin;

  module = g_module_open (path, G_MODULE_BIND_LAZY);
  if (!module) {
    g_warning ("Failed to open module: '%s'", path);
    return FALSE;
  }

  if (!g_module_symbol (module, "MS_PLUGIN_DESCRIPTOR", (gpointer) &plugin)) {
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
ms_plugin_registry_load_directory (MsPluginRegistry *registry,
				   const gchar *path)
{
  GDir *dir;
  gchar *file;
  const gchar *entry;

  dir = g_dir_open (path, 0, NULL);
  
  if (!dir) {
    g_warning ("Could not open plugin directory: '%s'", path);
    return FALSE;
  }
  
  while ((entry = g_dir_read_name (dir)) != NULL) {
    if (g_str_has_suffix (entry, "." G_MODULE_SUFFIX)) {
      file = g_build_filename (path, entry, NULL);
      ms_plugin_registry_load (registry, file);
      g_free (file);
    }
  }
  
  g_dir_close (dir);
  return TRUE;
}

gboolean
ms_plugin_registry_load_all (MsPluginRegistry *registry)
{
  const gchar *plugin_dirs_env;
  gchar **plugin_dirs;
  gchar **dirs_iter;

  plugin_dirs_env = g_getenv (MS_PLUGIN_PATH_VAR);
  if (!plugin_dirs_env) {
    g_warning ("No '%s' environment variable set, no plugins loaded!",
	       MS_PLUGIN_PATH_VAR);
    return FALSE;
  }

  plugin_dirs = g_strsplit (plugin_dirs_env, ":", 0);
  dirs_iter = plugin_dirs;

  while (*dirs_iter) {
    ms_plugin_registry_load_directory (registry, *dirs_iter);
    dirs_iter++;
  }

  g_strfreev (plugin_dirs);

  return TRUE;
}

MsMediaPlugin *
ms_plugin_registry_lookup_source (MsPluginRegistry *registry, const gchar *source_id)
{
  return (MsMediaPlugin *) g_hash_table_lookup (registry->priv->sources, source_id);
}

MsMediaPlugin **
ms_plugin_registry_get_sources (MsPluginRegistry *registry)
{
  GHashTableIter iter;
  MsMediaPlugin **source_list;
  guint n;

  n = g_hash_table_size (registry->priv->sources);
  source_list = (MsMediaPlugin **) g_new0 (MsMediaPlugin *, n + 1);

  n = 0;
  g_hash_table_iter_init (&iter, registry->priv->sources);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &source_list[n++]));

  return source_list;
}

void
ms_plugin_registry_unload (MsPluginRegistry *registry, const gchar *plugin_id)
{
  MsPluginDescriptor *plugin;

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

const MsMetadataKey *
ms_plugin_registry_lookup_metadata_key (MsPluginRegistry *registry, MsKeyID key_id)
{
  return &registry->priv->system_keys[key_id];
}
