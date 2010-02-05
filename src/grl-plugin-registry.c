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

#include "grl-plugin-registry.h"
#include "grl-media-plugin-priv.h"

#include "config.h"

#include <string.h>
#include <gmodule.h>

#define SYSTEM_KEYS_MAX 256

#define GRL_PLUGIN_PATH_DEFAULT GRL_PLUGINS_DIR

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "grl-plugin-registry"

#define GRL_PLUGIN_REGISTRY_GET_PRIVATE(object)                 \
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                        \
                               GRL_TYPE_PLUGIN_REGISTRY,        \
                               GrlPluginRegistryPrivate))

#define GRL_REGISTER_SYSTEM_METADATA_KEY(r, key)        \
  { r->priv->system_keys[key].id = key;			\
    r->priv->system_keys[key].name = key##_NAME;	\
    r->priv->system_keys[key].desc = key##_DESC;	\
  }

struct _GrlPluginRegistryPrivate {
  GHashTable *plugins;
  GHashTable *sources;
  GrlMetadataKey *system_keys;
};

static void grl_plugin_registry_setup_system_keys (GrlPluginRegistry *registry);

/* ================ GrlPluginRegistry GObject ================ */

enum {
  SIG_SOURCE_ADDED,
  SIG_SOURCE_REMOVED,
  SIG_LAST,
};
static gint registry_signals[SIG_LAST];

G_DEFINE_TYPE (GrlPluginRegistry, grl_plugin_registry, G_TYPE_OBJECT);

static void
grl_plugin_registry_class_init (GrlPluginRegistryClass *klass)
{
  if (!g_module_supported ()) {
    g_error ("GModule not supported in this system");
  }

  g_type_class_add_private (klass, sizeof (GrlPluginRegistryPrivate));

  registry_signals[SIG_SOURCE_ADDED] =
    g_signal_new("source-added",
		 G_TYPE_FROM_CLASS(klass),
		 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		 0,
		 NULL,
		 NULL,
		 g_cclosure_marshal_VOID__OBJECT,
		 G_TYPE_NONE, 1, GRL_TYPE_MEDIA_PLUGIN);

  registry_signals[SIG_SOURCE_REMOVED] =
    g_signal_new("source-removed",
		 G_TYPE_FROM_CLASS(klass),
		 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		 0,
		 NULL,
		 NULL,
		 g_cclosure_marshal_VOID__OBJECT,
		 G_TYPE_NONE, 1, GRL_TYPE_MEDIA_PLUGIN);
}

static void
grl_plugin_registry_init (GrlPluginRegistry *registry)
{
  registry->priv = GRL_PLUGIN_REGISTRY_GET_PRIVATE (registry);
  memset (registry->priv, 0, sizeof (GrlPluginRegistryPrivate));

  registry->priv->plugins = g_hash_table_new (g_str_hash, g_str_equal);
  registry->priv->sources =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  grl_plugin_registry_setup_system_keys (registry);
}

/* ================ Utitilies ================ */

static void
grl_plugin_registry_setup_system_keys (GrlPluginRegistry *registry)
{
  registry->priv->system_keys = g_new0 (GrlMetadataKey, SYSTEM_KEYS_MAX);

  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_TITLE);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_URL);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_ARTIST);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_ALBUM);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_GENRE);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_THUMBNAIL);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_ID);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_AUTHOR);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_DESCRIPTION);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_SOURCE);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_LYRICS);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_SITE);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_DURATION);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_DATE);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_CHILDCOUNT);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_MIME);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_WIDTH);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_HEIGHT);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_FRAMERATE);
  GRL_REGISTER_SYSTEM_METADATA_KEY (registry, GRL_METADATA_KEY_RATING);
}

/* ================ API ================ */

GrlPluginRegistry *
grl_plugin_registry_get_instance (void)
{
  static GrlPluginRegistry *registry = NULL;

  if (!registry) {
    registry = g_object_new (GRL_TYPE_PLUGIN_REGISTRY, NULL);
  }

  return registry;
}

gboolean
grl_plugin_registry_register_source (GrlPluginRegistry *registry,
                                     const GrlPluginInfo *plugin,
                                     GrlMediaPlugin *source)
{
  gchar *id;

  g_object_get (source, "source-id", &id, NULL);
  g_debug ("New source available: '%s'", id);

  /* Take ownership of the plugin */
  g_object_ref_sink (source);
  g_object_unref (source);

  /* Do not free id, since g_hash_table_insert does not copy,
     it will be freed when removed from the hash table */
  g_hash_table_insert (registry->priv->sources, id, source);

  grl_media_plugin_set_plugin_info (source, plugin);

  g_signal_emit (registry, registry_signals[SIG_SOURCE_ADDED], 0, source);

  return TRUE;
}

void
grl_plugin_registry_unregister_source (GrlPluginRegistry *registry,
                                       GrlMediaPlugin *source)
{
  gchar *id;

  g_object_get (source, "source-id", &id, NULL);
  g_debug ("Unregistering source '%s'", id);

  if (g_hash_table_remove (registry->priv->sources, id)) {
    g_debug ("source '%s' is no longer available", id);
    g_signal_emit (registry, registry_signals[SIG_SOURCE_REMOVED], 0, source);
    g_object_unref (source);
  } else {
    g_warning ("source '%s' not found", id);
  }

  g_free (id);
}

gboolean
grl_plugin_registry_load (GrlPluginRegistry *registry, const gchar *path)
{
  GModule *module;
  GrlPluginDescriptor *plugin;

  module = g_module_open (path, G_MODULE_BIND_LAZY);
  if (!module) {
    g_warning ("Failed to open module: '%s'", path);
    return FALSE;
  }

  if (!g_module_symbol (module, "GRL_PLUGIN_DESCRIPTOR", (gpointer) &plugin)) {
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
grl_plugin_registry_load_directory (GrlPluginRegistry *registry,
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
      grl_plugin_registry_load (registry, file);
      g_free (file);
    }
  }

  g_dir_close (dir);
  return TRUE;
}

gboolean
grl_plugin_registry_load_all (GrlPluginRegistry *registry)
{
  const gchar *plugin_dirs_env;
  gchar **plugin_dirs;
  gchar **dirs_iter;

  plugin_dirs_env = g_getenv (GRL_PLUGIN_PATH_VAR);
  if (!plugin_dirs_env) {
    plugin_dirs_env = GRL_PLUGIN_PATH_DEFAULT;
  }

  plugin_dirs = g_strsplit (plugin_dirs_env, ":", 0);
  dirs_iter = plugin_dirs;

  while (*dirs_iter) {
    grl_plugin_registry_load_directory (registry, *dirs_iter);
    dirs_iter++;
  }

  g_strfreev (plugin_dirs);

  return TRUE;
}

GrlMediaPlugin *
grl_plugin_registry_lookup_source (GrlPluginRegistry *registry,
                                   const gchar *source_id)
{
  return (GrlMediaPlugin *) g_hash_table_lookup (registry->priv->sources,
                                                 source_id);
}

GrlMediaPlugin **
grl_plugin_registry_get_sources (GrlPluginRegistry *registry)
{
  GHashTableIter iter;
  GrlMediaPlugin **source_list;
  guint n;

  n = g_hash_table_size (registry->priv->sources);
  source_list = (GrlMediaPlugin **) g_new0 (GrlMediaPlugin *, n + 1);

  n = 0;
  g_hash_table_iter_init (&iter, registry->priv->sources);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &source_list[n++]));

  return source_list;
}

void
grl_plugin_registry_unload (GrlPluginRegistry *registry,
                            const gchar *plugin_id)
{
  GrlPluginDescriptor *plugin;

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

const GrlMetadataKey *
grl_plugin_registry_lookup_metadata_key (GrlPluginRegistry *registry,
                                         GrlKeyID key_id)
{
  return &registry->priv->system_keys[key_id];
}
