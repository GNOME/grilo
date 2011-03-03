/*
 * Copyright (C) 2010, 2011 Igalia S.L.
 * Copyright (C) 2011 Intel Corporation.
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
 * a single instance of #GrlPluginRegistry (singleton pattern).
 *
 * A #GrlMediaPlugin can hold several data sources (#GrlMetadataSource or
 * #GrlMediaSource), and #GrlPluginRegistry shall register each one of
 * them.
 */

#include "grl-plugin-registry.h"
#include "grl-media-plugin-priv.h"
#include "grl-log.h"
#include "grl-error.h"

#include <string.h>
#include <gmodule.h>
#include <libxml/parser.h>

#define GRL_LOG_DOMAIN_DEFAULT  plugin_registry_log_domain
GRL_LOG_DOMAIN(plugin_registry_log_domain);

#define XML_ROOT_ELEMENT_NAME "plugin"

#define GRL_PLUGIN_REGISTRY_GET_PRIVATE(object)                 \
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                        \
                               GRL_TYPE_PLUGIN_REGISTRY,        \
                               GrlPluginRegistryPrivate))

struct _GrlPluginRegistryPrivate {
  GHashTable *configs;
  GHashTable *plugins;
  GHashTable *sources;
  GHashTable *related_keys;
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
  registry->priv->plugins =
    g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
  registry->priv->sources =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  registry->priv->related_keys =
    g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, NULL);
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
  GRL_DEBUG ("Rank configuration, '%s:%d'", plugin_id, rank);
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
  GRL_DEBUG ("Plugin rank '%s' : %d", plugin->info.id, plugin->info.rank);
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
    GRL_DEBUG ("$%s is not set, using default ranks.", GRL_PLUGIN_RANKS_VAR);
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
	GRL_WARNING ("Incorrect ranking definition: '%s'. Skipping...", *iter);
      } else {
	config_plugin_rank (registry, g_strdup (plugin_id), rank);
      }
    } else {
      GRL_WARNING ("Incorrect ranking definition: '%s'. Skipping...", *iter);
    }
    g_strfreev (rank_info);
    iter++;
  }

  g_strfreev (rank_specs);
}

static gint
compare_by_rank (gconstpointer a,
                 gconstpointer b) {
  gint rank_a;
  gint rank_b;

  rank_a = grl_media_plugin_get_rank (GRL_MEDIA_PLUGIN (a));
  rank_b = grl_media_plugin_get_rank (GRL_MEDIA_PLUGIN (b));

  return (rank_a < rank_b) - (rank_a > rank_b);
}

static GHashTable *
get_info_from_plugin_xml (const gchar *xml_path)
{
  GHashTable *hash_table;
  xmlNodePtr node, info_node, child;
  xmlDocPtr doc_ptr = xmlReadFile (xml_path,
				   NULL,
				   XML_PARSE_RECOVER | XML_PARSE_NOBLANKS |
				   XML_PARSE_NOWARNING | XML_PARSE_NOERROR);
  if (!doc_ptr) {
    GRL_MESSAGE ("Could not read XML file under the location: %s", xml_path);
    return NULL;
  }

  node = xmlDocGetRootElement (doc_ptr);
  if (!node || g_strcmp0 ((gchar *) node->name, XML_ROOT_ELEMENT_NAME)) {
    GRL_WARNING ("%s did not have a %s root element.",
                 xml_path,
                 XML_ROOT_ELEMENT_NAME);
    xmlFreeDoc (doc_ptr);
    return NULL;
  }

  /* Get the info node */
  info_node = node->children;
  while (info_node) {
    if (g_strcmp0 ((gchar *) info_node->name, "info") == 0) {
      break;
    }
    info_node = info_node->next;
  }
  if (!info_node) {
    return NULL;
  }

  /* Populate the hash table with the XML's info*/
  hash_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  child = info_node->children;
  while (child) {
    g_hash_table_insert (hash_table,
			 g_strdup ((gchar *) child->name),
			 (gchar *) xmlNodeGetContent (child));
    child = child->next;
  }
  xmlFreeDoc (doc_ptr);

  return hash_table;
}

/* ================ API ================ */

/**
 * grl_plugin_registry_get_default:
 *
 * As the registry is designed to work as a singleton, this
 * method is in charge of creating the only instance or
 * returned it if it is already in memory.
 *
 * Returns: (transfer none): a new or an already created instance of the registry.
 *
 * It is NOT MT-safe
 *
 * Since: 0.1.6
 */
GrlPluginRegistry *
grl_plugin_registry_get_default (void)
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
 * @plugin: the descriptor of the plugin which owns the source
 * @source: the source to register
 * @error: error return location or @NULL to ignore
 *
 * Register a @source in the @registry with the given @plugin information
 *
 * Returns: %TRUE if success, %FALSE% otherwise.
 *
 * Since: 0.1.7
 */
gboolean
grl_plugin_registry_register_source (GrlPluginRegistry *registry,
                                     const GrlPluginInfo *plugin,
                                     GrlMediaPlugin *source,
                                     GError **error)
{
  gchar *id;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (source), FALSE);

  g_object_get (source, "source-id", &id, NULL);
  GRL_DEBUG ("New source available: '%s'", id);

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
 * @error: error return location or @NULL to ignore
 *
 * Removes the @source from the @registry hash table
 *
 * Returns: %TRUE if success, %FALSE% otherwise.
 *
 * Since: 0.1.7
 */
gboolean
grl_plugin_registry_unregister_source (GrlPluginRegistry *registry,
                                       GrlMediaPlugin *source,
                                       GError **error)
{
  gchar *id;
  gboolean ret = TRUE;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (source), FALSE);

  g_object_get (source, "source-id", &id, NULL);
  GRL_DEBUG ("Unregistering source '%s'", id);

  if (g_hash_table_remove (registry->priv->sources, id)) {
    GRL_DEBUG ("source '%s' is no longer available", id);
    g_signal_emit (registry, registry_signals[SIG_SOURCE_REMOVED], 0, source);
    g_object_unref (source);
  } else {
    GRL_WARNING ("source '%s' not found", id);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_UNREGISTER_SOURCE_FAILED,
                 "Source with id '%s' was not found", id);
    ret = FALSE;
  }

  return ret;
}

/**
 * grl_plugin_registry_add_directory:
 * @registry: the registry instance
 * @path: a path with plugins
 *
 * Set this path as part of default paths to load plugins.
 *
 * Since: 0.1.6
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
 * @error: error return location or @NULL to ignore
 *
 * Loads a module from shared object file stored in @path
 *
 * Returns: %TRUE if the module is loaded correctly
 *
 * Since: 0.1.7
 */
gboolean
grl_plugin_registry_load (GrlPluginRegistry *registry,
                          const gchar *path,
                          GError **error)
{
  GModule *module;
  GrlPluginDescriptor *plugin;
  GList *plugin_configs;
  gchar *xml_path;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);

  module = g_module_open (path, G_MODULE_BIND_LAZY);
  if (!module) {
    GRL_WARNING ("Failed to open module: '%s'", path);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "Failed to load plugin at '%s'", path);
    return FALSE;
  }

  if (!g_module_symbol (module, "GRL_PLUGIN_DESCRIPTOR", (gpointer) &plugin)) {
    GRL_WARNING ("Did not find plugin descriptor: '%s'", path);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "'%s' is not a valid plugin file", path);
    g_module_close (module);
    return FALSE;
  }

  if (!plugin->plugin_init ||
      !plugin->info.id) {
    GRL_WARNING ("Plugin descriptor is not valid: '%s'", path);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "'%s' is not a valid plugin file", path);
    g_module_close (module);
    return FALSE;
  }

  plugin->info.filename = g_strdup (path);

  xml_path = g_strconcat (GRL_PLUGINS_CONF_DIR,
			  G_DIR_SEPARATOR_S,
			  plugin->info.id,
			  ".xml",
			  NULL);
  plugin->info.optional_info =  get_info_from_plugin_xml (xml_path);
  g_free (xml_path);

  set_plugin_rank (registry, plugin);

  g_hash_table_insert (registry->priv->plugins,
		       (gpointer) plugin->info.id, plugin);

  plugin_configs = g_hash_table_lookup (registry->priv->configs,
					plugin->info.id);

  if (!plugin->plugin_init (registry, &plugin->info, plugin_configs)) {
    g_hash_table_remove (registry->priv->plugins, plugin->info.id);
    GRL_WARNING ("Failed to initialize plugin: '%s'", path);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "Failed to initialize plugin at '%s'", path);
    g_module_close (module);
    return FALSE;
  }

  plugin->module = module;

  GRL_DEBUG ("Loaded plugin '%s' from '%s'", plugin->info.id, path);

  return TRUE;
}

/**
 * grl_plugin_registry_load_directory:
 * @registry: the registry instance
 * @path: the path to the directory
 * @error: error return location or @NULL to ignore
 *
 * Loads a set of modules from directory in @path which contains
 * a group shared object files.
 *
 * Returns: %TRUE if the directory is valid.
 *
 * Since: 0.1.7
 */
gboolean
grl_plugin_registry_load_directory (GrlPluginRegistry *registry,
                                    const gchar *path,
                                    GError **error)
{
  GDir *dir;
  gchar *file;
  const gchar *entry;
  gboolean loaded_one = FALSE;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);

  dir = g_dir_open (path, 0, NULL);

  if (!dir) {
    GRL_WARNING ("Could not open plugin directory: '%s'", path);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "Failed to open plugin directory '%s'", path);
    return FALSE;
  }

  while ((entry = g_dir_read_name (dir)) != NULL) {
    if (g_str_has_suffix (entry, "." G_MODULE_SUFFIX)) {
      file = g_build_filename (path, entry, NULL);
      if (grl_plugin_registry_load (registry, file, NULL)) {
        loaded_one = TRUE;
      }
      g_free (file);
    }
  }

  if (!loaded_one) {
    GRL_WARNING ("No plugins loaded from directory '%s'", path);
  }

  g_dir_close (dir);
  return TRUE;
}

/**
 * grl_plugin_registry_load_all:
 * @registry: the registry instance
 * @error: error return location or @NULL to ignore
 *
 * Load all the modules available in the default directory path.
 *
 * The default directory path can be changed through the environment
 * variable %GRL_PLUGIN_PATH and it can contain several paths separated
 * by ":"
 *
 * Returns: %FALSE% is all the configured plugin paths are invalid,
 * %TRUE% otherwise.
 *
 * Since: 0.1.1
 */
gboolean
grl_plugin_registry_load_all (GrlPluginRegistry *registry, GError **error)
{
  GSList *plugin_dir;
  gboolean loaded_one = FALSE;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), TRUE);

  for (plugin_dir = registry->priv->plugins_dir;
       plugin_dir;
       plugin_dir = g_slist_next (plugin_dir)) {
    if (grl_plugin_registry_load_directory (registry, plugin_dir->data, NULL)) {
      loaded_one = TRUE;
    }
  }

  if (!loaded_one) {
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "All configured plugin paths are invalid. "    \
                 "Failed to load plugins.");
  }

  return loaded_one;
}

/**
 * grl_plugin_registry_lookup_source:
 * @registry: the registry instance
 * @source_id: the id of a source
 *
 * This function will search and retrieve a source given its identifier.
 *
 * Returns: (transfer none): The source found.
 *
 * Since: 0.1.1
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
 * Returns: (element-type Grl.MediaPlugin) (transfer container): a #GList of
 * available #GrlMediaPlugins<!-- -->s. The content of the list should not be
 * modified or freed. Use g_list_free() when done using the list.
 *
 * Since: 0.1.7
 */
GList *
grl_plugin_registry_get_sources (GrlPluginRegistry *registry,
				 gboolean ranked)
{
  GHashTableIter iter;
  GList *source_list = NULL;
  GrlMediaPlugin *current_plugin;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);

  g_hash_table_iter_init (&iter, registry->priv->sources);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &current_plugin)) {
    source_list = g_list_prepend (source_list, current_plugin);
  }

  if (ranked) {
    source_list = g_list_sort (source_list, (GCompareFunc) compare_by_rank);
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
 * Returns: (element-type Grl.MediaPlugin) (transfer container): a #GList of
 * available #GrlMediaPlugins<!-- -->s. The content of the list should not be
 * modified or freed. Use g_list_free() when done using the list.
 *
 * Since: 0.1.7
 */
GList *
grl_plugin_registry_get_sources_by_operations (GrlPluginRegistry *registry,
                                               GrlSupportedOps ops,
                                               gboolean ranked)
{
  GHashTableIter iter;
  GList *source_list = NULL;
  GrlMediaPlugin *p;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);

  g_hash_table_iter_init (&iter, registry->priv->sources);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &p)) {
    GrlSupportedOps source_ops;
    source_ops =
      grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (p));
    if ((source_ops & ops) == ops) {
      source_list = g_list_prepend (source_list, p);
    }
  }

  if (ranked) {
    source_list = g_list_sort (source_list, compare_by_rank);
  }

  return source_list;
}

/**
 * grl_plugin_registry_unload:
 * @registry: the registry instance
 * @plugin_id: the identifier of the plugin
 * @error: error return location or @NULL to ignore
 *
 * Unload from memory a module identified by @plugin_id. This means call the
 * module's deinit function.
 *
 * Returns %TRUE% on success.
 *
 * Since: 0.1.7
 */
gboolean
grl_plugin_registry_unload (GrlPluginRegistry *registry,
                            const gchar *plugin_id,
                            GError **error)
{
  GrlPluginDescriptor *plugin;
  GList *sources = NULL;
  GList *sources_iter;

  GRL_DEBUG ("grl_plugin_registry_unload: %s", plugin_id);

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);
  g_return_val_if_fail (plugin_id != NULL, FALSE);

  /* First check the plugin is valid  */
  plugin = g_hash_table_lookup (registry->priv->plugins, plugin_id);
  if (!plugin) {
    GRL_WARNING ("Could not deinit plugin '%s'. Plugin not found.", plugin_id);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_UNLOAD_PLUGIN_FAILED,
                 "Plugin not found: '%s'", plugin_id);
    return FALSE;
  }

  /* Second, shut down any sources spawned by this plugin */
  GRL_DEBUG ("Shutting down sources spawned by '%s'", plugin_id);
  sources = grl_plugin_registry_get_sources (registry, FALSE);

  for (sources_iter = sources; sources_iter;
      sources_iter = g_list_next (sources_iter)) {
    const gchar *id;
    GrlMediaPlugin *source;

    source = GRL_MEDIA_PLUGIN (sources_iter->data);
    id = grl_media_plugin_get_id (source);
    if (!strcmp (plugin_id, id)) {
      grl_plugin_registry_unregister_source (registry, source, NULL);
    }
  }
  g_list_free (sources);

  /* Third, shut down the plugin */
  GRL_DEBUG ("Unloading plugin '%s'", plugin_id);
  if (plugin->plugin_deinit) {
    plugin->plugin_deinit ();
  }

  g_free (plugin->info.filename);
  if (plugin->info.optional_info) {
    g_hash_table_destroy (plugin->info.optional_info);
  }

  if (plugin->module) {
    g_module_close (plugin->module);
  }

  return TRUE;
}

/**
 * grl_plugin_registry_register_metadata_key:
 * @registry: The plugin registry
 * @key: The key to register
 * @error: error return location or @NULL to ignore
 *
 * Registers a metadata key
 *
 * Returns: (type GObject.ParamSpec) (transfer none): The #GrlKeyID registered
 * or @NULL on error.
 *
 * Since: 0.1.7
 */
GrlKeyID
grl_plugin_registry_register_metadata_key (GrlPluginRegistry *registry,
                                           GParamSpec *key,
                                           GError **error)
{
  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);
  g_return_val_if_fail (G_IS_PARAM_SPEC (key), NULL);

  /* Check if key is already registered */
  if (g_param_spec_pool_lookup (registry->priv->system_keys,
                                g_param_spec_get_name (key),
                                GRL_TYPE_MEDIA,
                                FALSE)) {
    GRL_WARNING ("metadata key '%s' already registered",
                 g_param_spec_get_name (key));
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_REGISTER_METADATA_KEY_FAILED,
                 "Metadata key '%s' was already registered",
                 g_param_spec_get_name (key));
    return NULL;
  } else {
    g_param_spec_pool_insert (registry->priv->system_keys,
                              key,
                              GRL_TYPE_MEDIA);
    /* Each key is related with itself */
    g_hash_table_insert (registry->priv->related_keys,
                         key,
                         g_list_prepend (NULL, key));
    return key;
  }
}

/**
 * grl_plugin_registry_register_metadata_key_relation:
 * @registry: the plugin registry
 * @key1: key involved in relationship
 * @key2: key involved in relationship
 *
 * Creates a relation between @key1 and @key2, meaning that the values of both
 * keys are somehow related.
 *
 * One example of a relation would be the one between the URI of a media
 * resource and its mime-type: they are both tied together and one does not make
 * sense without the other.
 *
 * Relations between keys allow the framework to provide all the data that is
 * somehow related when any of the related keys are requested.
 */
void
grl_plugin_registry_register_metadata_key_relation (GrlPluginRegistry *registry,
                                                    GrlKeyID key1,
                                                    GrlKeyID key2)
{
  GList *key1_partners, *key1_peer;
  GList *key2_partners;

  g_return_if_fail (GRL_IS_PLUGIN_REGISTRY (registry));
  g_return_if_fail (key1);
  g_return_if_fail (key2);

  if (key1 == key2) {
    return;
  }

  /* Search for keys related with each key */
  key1_partners = g_hash_table_lookup (registry->priv->related_keys, key1);
  key2_partners = g_hash_table_lookup (registry->priv->related_keys, key2);

  /* Check if they are already related */
  if (!key1_partners || !key2_partners || key1_partners == key2_partners) {
    return;
  }

  /* Merge both relations [related(key1), related(key2)] */
  key1_partners = g_list_concat(key1_partners, key2_partners);

  for (key1_peer = key1_partners;
       key1_peer;
       key1_peer = g_list_next (key1_peer)) {
    g_hash_table_insert (registry->priv->related_keys, key1_peer->data, key1_partners);
  }
}

/**
 * grl_plugin_registry_lookup_metadata_key:
 * @registry: the registry instance
 * @key_name: the key name
 *
 * Look up for the metadata key with name @key_name.
 *
 * Returns: (type GObject.ParamSpec) (transfer none): The metadata key, or @NULL if not found
 *
 * Since: 0.1.6
 */
GrlKeyID
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
 * grl_plugin_registry_lookup_metadata_key_relation:
 * @registry: the registry instance
 * @key: a metadata key
 *
 * Look up the list of keys that have a relation with @key.
 *
 * @key is included in that list.
 *
 * Returns: (element-type Grl.KeyID) (transfer-none): a #GList of related
 * keys, or @NULL if key is invalid.
 **/
const GList *
grl_plugin_registry_lookup_metadata_key_relation (GrlPluginRegistry *registry,
                                                  GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);

  return g_hash_table_lookup (registry->priv->related_keys, key);
}

/**
 * grl_plugin_registry_get_metadata_keys:
 * @registry: the registry instance
 *
 * Returns a list with all registered keys in system.
 *
 * Returns: (element-type GObject.ParamSpec) (transfer container): a #GList
 * with all the available #GrlKeyID<!-- -->s. The content of the list should
 * not be modified or freed. Use g_list_free() when done using the list.
 *
 * Since: 0.1.6
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
 * @error: error return location or @NULL to ignore
 *
 * Add a configuration for a plugin/source.
 *
 * Since: 0.1.7
 */
gboolean
grl_plugin_registry_add_config (GrlPluginRegistry *registry,
                                GrlConfig *config,
                                GError **error)
{
  gchar *plugin_id;
  GList *configs = NULL;

  g_return_val_if_fail (config != NULL, FALSE);
  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);

  plugin_id = grl_config_get_plugin (config);
  if (!plugin_id) {
    GRL_WARNING ("Plugin configuration missed plugin information, ignoring...");
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_CONFIG_FAILED,
                 "Plugin configuration does not contain "       \
                 "plugin-id reference");
    return FALSE;
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

  return TRUE;
}

/**
 * grl_plugin_registry_add_config_from_file:
 * @registry: the registry instance
 * @config_file: a key-value file containing the configuration
 * @error: error return location or @NULL to ignore
 *
 * Load plugin configurations from a .ini-like config file.
 *
 * Returns: %TRUE on success
 *
 * Since: 0.1.7
 **/
gboolean
grl_plugin_registry_add_config_from_file (GrlPluginRegistry *registry,
                                          const gchar *config_file,
                                          GError **error)
{
  GError *load_error = NULL;
  GKeyFile *keyfile;
  GrlConfig *config;
  gchar **key;
  gchar **keys;
  gchar **plugin;
  gchar **plugins;
  gchar *value;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);
  g_return_val_if_fail (config_file, FALSE);

  keyfile = g_key_file_new ();

  if (g_key_file_load_from_file (keyfile,
                                 config_file,
                                 G_KEY_FILE_NONE,
                                 &load_error)) {

    /* Look up for defined plugins */
    plugins = g_key_file_get_groups (keyfile, NULL);
    for (plugin = plugins; *plugin; plugin++) {
      config = grl_config_new (*plugin, NULL);

      /* Look up configuration keys for this plugin */
      keys = g_key_file_get_keys (keyfile, *plugin, NULL, NULL);
      for (key = keys; *key; key++) {
        value = g_key_file_get_string (keyfile, *plugin, *key, NULL);
        if (value) {
          grl_config_set_string (config, *key, value);
          g_free (value);
        }
      }
      grl_plugin_registry_add_config (registry, config, NULL);
      g_strfreev (keys);
    }
    g_strfreev (plugins);
    g_key_file_free (keyfile);
    return TRUE;
  } else {
    GRL_WARNING ("Unable to load configuration. %s", load_error->message);
    g_set_error_literal (error,
                         GRL_CORE_ERROR,
                         GRL_CORE_ERROR_CONFIG_LOAD_FAILED,
                         load_error->message);
    g_error_free (load_error);
    g_key_file_free (keyfile);
    return FALSE;
  }
}
