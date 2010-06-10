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

/**
 * SECTION:grl-plugin-registry
 * @short_description: Grilo plugins loader and manager
 * @see_also: #GrlMediaPlugin, #GrlMetadataSource, #GrlMediaSource
 *
 * The registry holds the metadata of a set of plugins.
 *
 * The #GrlPluginRegistry object is a list of plugins and some functions
 * for dealing with them. Each #GrlMediaPlugin is matched 1-1 with a file
 * on disk, and may or may not be loaded a given time. There only can be
 * a single instance of #GstPluginRegistry (singleton pattern).
 *
 * A #GrlMediaPlugin can hold several data sources (#GrlMetadataSource or
 * #GrlMediaSource), and #GrlPluginRegistry and shall register each one of
 * them.
 */

#include "grl-plugin-registry.h"
#include "grl-media-plugin-priv.h"

#include <string.h>
#include <gmodule.h>

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "grl-plugin-registry"

#define GRL_PLUGIN_REGISTRY_GET_PRIVATE(object)                 \
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                        \
                               GRL_TYPE_PLUGIN_REGISTRY,        \
                               GrlPluginRegistryPrivate))

struct _GrlPluginRegistryPrivate {
  GHashTable *configs;
  GHashTable *plugins;
  GHashTable *sources;
  GParamSpecPool *system_keys;
  GHashTable *ranks;
  GSList *plugins_dir;
};

static void grl_plugin_registry_setup_ranks (GrlPluginRegistry *registry);

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

  /**
   * GrlPluginRegistry::source-added:
   * @registry: the registry
   * @plugin: the plugin that has been added
   *
   * Signals that a plugin has been added to the registry.
   */
  registry_signals[SIG_SOURCE_ADDED] =
    g_signal_new("source-added",
		 G_TYPE_FROM_CLASS(klass),
		 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		 0,
		 NULL,
		 NULL,
		 g_cclosure_marshal_VOID__OBJECT,
		 G_TYPE_NONE, 1, GRL_TYPE_MEDIA_PLUGIN);

  /**
   * GrlPluginRegistry::source-removed:
   * @registry: the registry
   * @plugin: the plugin that has been removed
   *
   * Signals that a plugin has been removed from the registry.
   */
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

  registry->priv->configs =
    g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);
  registry->priv->plugins = g_hash_table_new (g_str_hash, g_str_equal);
  registry->priv->sources =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  registry->priv->system_keys =
    g_param_spec_pool_new (FALSE);

  grl_plugin_registry_setup_ranks (registry);
}

/* ================ Utitilies ================ */

static void
config_plugin_rank (GrlPluginRegistry *registry,
		    const gchar *plugin_id,
		    gint rank)
{
  g_debug ("Rank configuration, '%s:%d'", plugin_id, rank);
  g_hash_table_insert (registry->priv->ranks,
		       (gchar *) plugin_id,
		       GINT_TO_POINTER (rank));
}

static void
set_plugin_rank (GrlPluginRegistry *registry, GrlPluginDescriptor *plugin)
{
  plugin->info.rank =
    GPOINTER_TO_INT (g_hash_table_lookup (registry->priv->ranks,
					  plugin->info.id));
  if (!plugin->info.rank) {
    plugin->info.rank = GRL_PLUGIN_RANK_DEFAULT;
  }
  g_debug ("Plugin rank '%s' : %d", plugin->info.id, plugin->info.rank);
}

static void
grl_plugin_registry_setup_ranks (GrlPluginRegistry *registry)
{
  const gchar *ranks_env;
  gchar **rank_specs;
  gchar **iter;

  registry->priv->ranks = g_hash_table_new_full (g_str_hash, g_str_equal,
						 g_free, NULL);

  ranks_env = g_getenv (GRL_PLUGIN_RANKS_VAR);
  if (!ranks_env) {
    g_debug ("$%s is not set, using default ranks.", GRL_PLUGIN_RANKS_VAR);
    return;
  }

  rank_specs = g_strsplit (ranks_env, ",", 0);
  iter = rank_specs;

  while (*iter) {
    gchar **rank_info = g_strsplit (*iter, ":", 2);
    if (rank_info[0] && rank_info[1]) {
      gchar *tmp;
      gchar *plugin_id = rank_info[0];
      gchar *plugin_rank = rank_info[1];
      gint rank = (gint) g_ascii_strtoll (plugin_rank, &tmp, 10);
      if (*tmp != '\0') {
	g_warning ("Incorrect ranking definition: '%s'. Skipping...", *iter);
      } else {
	config_plugin_rank (registry, g_strdup (plugin_id), rank);
      }
    } else {
      g_warning ("Incorrect ranking definition: '%s'. Skipping...", *iter);
    }
    g_strfreev (rank_info);
    iter++;
  }

  g_strfreev (rank_specs);
}

static void
sort_by_rank (GrlMediaPlugin **source_list)
{
  GrlMediaPlugin *plugin;
  gint index, i, top_rank, top_index;

  index = 0;
  while (source_list[index]) {
    top_rank = grl_media_plugin_get_rank (source_list[index]);
    top_index = index;
    i = index + 1;
    while (source_list[i]) {
      gint rank = grl_media_plugin_get_rank (source_list[i]);
      if (rank > top_rank) {
	top_rank = rank;
	top_index = i;
      }
      i++;
    }
    plugin = source_list[index];
    source_list[index] = source_list[top_index];
    source_list[top_index] = plugin;
    index++;
  }
}

/* ================ API ================ */

/**
 * grl_plugin_registry_get_instance:
 *
 * As the registry is designed to work as a singleton, this
 * method is in charge of creating the only instance or
 * returned it if it is already in memory.
 *
 * Returns: a new or an already created instance of the registry.
 *
 * It is NOT MT-safe
 */
GrlPluginRegistry *
grl_plugin_registry_get_instance (void)
{
  static GrlPluginRegistry *registry = NULL;

  if (!registry) {
    registry = g_object_new (GRL_TYPE_PLUGIN_REGISTRY, NULL);
  }

  return registry;
}

/**
 * grl_plugin_registry_register_source:
 * @registry: the registry instance
 * @plugin: the descriptor of the plugin which owns the srouce
 * @source: the source to register
 *
 * Register a @source in the @registry with the given @plugin information
 *
 * Returns: %TRUE if success
 */
gboolean
grl_plugin_registry_register_source (GrlPluginRegistry *registry,
                                     const GrlPluginInfo *plugin,
                                     GrlMediaPlugin *source)
{
  gchar *id;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (source), FALSE);

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

/**
 * grl_plugin_registry_unregister_source:
 * @registry: the registry instance
 * @source: the source to unregister
 *
 * Removes the @source from the @registry hash table
 */
void
grl_plugin_registry_unregister_source (GrlPluginRegistry *registry,
                                       GrlMediaPlugin *source)
{
  gchar *id;

  g_return_if_fail (GRL_IS_PLUGIN_REGISTRY (registry));
  g_return_if_fail (GRL_IS_MEDIA_PLUGIN (source));

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

/**
 * grl_plugin_registry_add_directory:
 * @registry: the registry intance
 * @path: a path with plugins
 *
 * Set this path as part of default paths to load plugins.
 **/
void
grl_plugin_registry_add_directory (GrlPluginRegistry *registry,
                                   const gchar *path)
{
  g_return_if_fail (GRL_IS_PLUGIN_REGISTRY (registry));
  g_return_if_fail (path);

  /* Use append instead of prepend so plugins are loaded in the same order as
     they were added */
  registry->priv->plugins_dir = g_slist_append (registry->priv->plugins_dir,
                                                g_strdup (path));
}

/**
 * grl_plugin_registry_load:
 * @registry: the registry instance
 * @path: the path to the so file
 *
 * Loads a module from shared object file stored in @path
 *
 * Returns: %TRUE if the module is loaded correctly
 */
gboolean
grl_plugin_registry_load (GrlPluginRegistry *registry, const gchar *path)
{
  GModule *module;
  GrlPluginDescriptor *plugin;
  GList *plugin_configs;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);

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

  set_plugin_rank (registry, plugin);

  g_hash_table_insert (registry->priv->plugins,
		       (gpointer) plugin->info.id, plugin);

  plugin_configs = g_hash_table_lookup (registry->priv->configs,
					plugin->info.id);

  if (!plugin->plugin_init (registry, &plugin->info, plugin_configs)) {
    g_hash_table_remove (registry->priv->plugins, plugin->info.id);
    g_warning ("Failed to initialize plugin: '%s'", path);
    return FALSE;
  }

  g_debug ("Loaded plugin '%s' from '%s'", plugin->info.id, path);

  return TRUE;
}

/**
 * grl_plugin_registry_load_directory:
 * @registry: the registry instance
 * @path: the path to the directory
 *
 * Loads a set of modules from directory in @path which contains
 * a group shared object files.
 *
 * Returns: %TRUE if the directory exists.
 */
gboolean
grl_plugin_registry_load_directory (GrlPluginRegistry *registry,
                                    const gchar *path)
{
  GDir *dir;
  gchar *file;
  const gchar *entry;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);

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

/**
 * grl_plugin_registry_load_all:
 * @registry: the registry instance
 *
 * Load all the modules available in the default directory path.
 *
 * The default directory path can be changed through the environment
 * variable %GRL_PLUGIN_PATH and it can contain several paths separated
 * by ":"
 *
 * Returns: %TRUE always
 */
gboolean
grl_plugin_registry_load_all (GrlPluginRegistry *registry)
{
  GSList *plugin_dir;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), TRUE);

  for (plugin_dir = registry->priv->plugins_dir;
       plugin_dir;
       plugin_dir = g_slist_next (plugin_dir)) {
    grl_plugin_registry_load_directory (registry, plugin_dir->data);
  }

  return TRUE;
}

/**
 * grl_plugin_registry_lookup_source:
 * @registry: the registry instance
 * @source_id: the id of a source
 *
 * This function will search and retrieve a source given its identifier.
 *
 * Returns: (allow-none): The source found.
 */
GrlMediaPlugin *
grl_plugin_registry_lookup_source (GrlPluginRegistry *registry,
                                   const gchar *source_id)
{
  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);
  g_return_val_if_fail (source_id != NULL, NULL);
  return (GrlMediaPlugin *) g_hash_table_lookup (registry->priv->sources,
                                                 source_id);
}

/**
 * grl_plugin_registry_get_sources:
 * @registry: the registry instance
 * @ranked: whether the returned list shall be returned ordered by rank
 *
 * This function will return all the available sources in the @registry.
 *
 * If @ranked is %TRUE, the source list will be ordered by rank.
 *
 * Returns: (transfer container): an array of available sources
 */
GrlMediaPlugin **
grl_plugin_registry_get_sources (GrlPluginRegistry *registry,
				 gboolean ranked)
{
  GHashTableIter iter;
  GrlMediaPlugin **source_list;
  gint n;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);

  n = g_hash_table_size (registry->priv->sources);
  source_list = (GrlMediaPlugin **) g_new0 (GrlMediaPlugin *, n + 1);

  n = 0;
  g_hash_table_iter_init (&iter, registry->priv->sources);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &source_list[n++]));

  if (ranked) {
    sort_by_rank (source_list);
  }

  return source_list;
}

/**
 * grl_plugin_registry_get_sources_by_operations:
 * @registry: the registry instance
 * @ops: a bitwise mangle of the requested operations.
 * @ranked: whether the returned list shall be returned ordered by rank
 *
 * Give an array of all the available sources in the @registry capable of
 * perform the operations requested in @ops.
 *
 * If @ranked is %TRUE, the source list will be ordered by rank.
 *
 * Returns: (transfer container): an array of available sources
 */
GrlMediaPlugin **
grl_plugin_registry_get_sources_by_operations (GrlPluginRegistry *registry,
                                               GrlSupportedOps ops,
                                               gboolean ranked)
{
  GHashTableIter iter;
  GrlMediaPlugin **source_list;
  GrlMediaPlugin *p;
  gint n;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);

  n = g_hash_table_size (registry->priv->sources);
  source_list = (GrlMediaPlugin **) g_new0 (GrlMediaPlugin *, n + 1);

  n = 0;
  g_hash_table_iter_init (&iter, registry->priv->sources);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &p)) {
    GrlSupportedOps source_ops;
    source_ops =
      grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (p));
    if ((source_ops & ops) == ops) {
      source_list[n++] = p;
    }
  }
  source_list[n] = NULL;

  if (ranked) {
    sort_by_rank (source_list);
  }

  return source_list;  
}

/**
 * grl_plugin_registry_unload:
 * @registry: the registry instance
 * @plugin_id: the identifier of the plugin
 *
 * Unload from memory a module identified by @plugin_id. This means call the
 * module's deinit function.
 */
void
grl_plugin_registry_unload (GrlPluginRegistry *registry,
                            const gchar *plugin_id)
{
  GrlPluginDescriptor *plugin;

  g_return_if_fail (GRL_IS_PLUGIN_REGISTRY (registry));
  g_return_if_fail (plugin_id != NULL);

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

const GrlKeyID
grl_plugin_registry_register_metadata_key (GrlPluginRegistry *registry,
                                           GParamSpec *key)
{
  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);
  g_return_val_if_fail (G_IS_PARAM_SPEC (key), NULL);

  /* Check if key is already registered */
  if (g_param_spec_pool_lookup (registry->priv->system_keys,
                                g_param_spec_get_name (key),
                                GRL_TYPE_MEDIA,
                                FALSE)) {
    g_warning ("metadata key '%s' already registered",
               g_param_spec_get_name (key));
    return NULL;
  } else {
    g_param_spec_pool_insert (registry->priv->system_keys,
                              key,
                              GRL_TYPE_MEDIA);
    return key;
  }
}

/**
 * grl_plugin_registry_lookup_metadata_key:
 * @registry: the registry instance
 * @key_id: the key name
 *
 * Look up for the metadata key with name @key_name.
 *
 * Returns: (transfer none): The metadata key, or @NULL if not found
 */
const GrlKeyID
grl_plugin_registry_lookup_metadata_key (GrlPluginRegistry *registry,
                                         const gchar *key_name)
{
  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);
  g_return_val_if_fail (key_name, NULL);

  return g_param_spec_pool_lookup (registry->priv->system_keys,
                                   key_name,
                                   GRL_TYPE_MEDIA,
                                   FALSE);
}

/**
 * grl_plugin_registry_get_metadata_keys:
 * @registry: the registry instance
 *
 * Returns a list with all registered keys in system.
 *
 * Returns: a list with all keys
 **/
GList *
grl_plugin_registry_get_metadata_keys (GrlPluginRegistry *registry)
{
  GList *key_list = NULL;
  GParamSpec **keys;
  guint i;
  guint keys_length;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);

  keys = g_param_spec_pool_list (registry->priv->system_keys,
                                 GRL_TYPE_MEDIA,
                                 &keys_length);

  for (i = 0; i < keys_length; i++) {
    key_list = g_list_prepend (key_list, keys[i]);
  }

  g_free (keys);

  return key_list;
}

/**
 * grl_plugin_registry_add_config:
 * @registry: the registry instance
 * @config: a configuration set
 *
 * Add a configuration for a plugin/source.
 */
void
grl_plugin_registry_add_config (GrlPluginRegistry *registry,
                                GrlConfig *config)
{
  const gchar *plugin_id;
  GList *configs = NULL;

 g_return_if_fail (config != NULL);
  g_return_if_fail (GRL_IS_PLUGIN_REGISTRY (registry));

  plugin_id = grl_config_get_plugin (config);
  if (!plugin_id) {
    g_warning ("Plugin configuration missed plugin information, ignoring...");
    return;
  }
  
  configs = g_hash_table_lookup (registry->priv->configs, plugin_id);
  if (configs) {
    /* Notice that we are using g_list_append on purpose to avoid
       having to insert again in the hash table */
    configs = g_list_append (configs, config);
  } else {
    configs = g_list_prepend (configs, config);
    g_hash_table_insert (registry->priv->configs,
			 (gpointer) plugin_id,
			 configs);
  }
}
