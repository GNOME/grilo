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
#include "grl-plugin-registry-priv.h"
#include "grl-media-plugin-priv.h"
#include "grl-log.h"
#include "grl-error.h"

#include <string.h>
#include <gmodule.h>
#include <libxml/parser.h>

#define GRL_LOG_DOMAIN_DEFAULT  plugin_registry_log_domain
GRL_LOG_DOMAIN(plugin_registry_log_domain);

#define XML_ROOT_ELEMENT_NAME "plugin"

#define GRL_PLUGIN_INFO_SUFFIX "xml"

#define GRL_PLUGIN_INFO_MODULE "module"

#define GRL_PLUGIN_REGISTRY_GET_PRIVATE(object)                 \
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                        \
                               GRL_TYPE_PLUGIN_REGISTRY,        \
                               GrlPluginRegistryPrivate))

/* GQuark-like implementation, where we manually assign the first IDs. */
struct KeyIDHandler {
  GHashTable *string_to_id;
  GArray *id_to_string;
  gint last_id;
};

struct _GrlPluginRegistryPrivate {
  GHashTable *configs;
  GHashTable *plugins;
  GHashTable *plugin_infos;
  GHashTable *sources;
  GHashTable *related_keys;
  GParamSpecPool *system_keys;
  GHashTable *ranks;
  GSList *plugins_dir;
  GSList *allowed_plugins;
  gboolean all_plugin_info_loaded;
  struct KeyIDHandler key_id_handler;
};

static void grl_plugin_registry_setup_ranks (GrlPluginRegistry *registry);

static GList *grl_plugin_registry_load_plugin_info_directory (GrlPluginRegistry *registry,
                                                              const gchar *path,
                                                              GError **error);

static GrlPluginInfo *grl_plugin_registry_load_plugin_info (GrlPluginRegistry *registry,
                                                            const gchar *plugin_id,
                                                            const gchar *file);

static void grl_plugin_registry_load_plugin_info_all (GrlPluginRegistry *registry);

static void key_id_handler_init (struct KeyIDHandler *handler);

static GrlKeyID key_id_handler_get_key (struct KeyIDHandler *handler,
                                        const gchar *key_name);

static const gchar *key_id_handler_get_name (struct KeyIDHandler *handler,
                                             GrlKeyID key);

static GrlKeyID key_id_handler_add (struct KeyIDHandler *handler,
                                    GrlKeyID key, const gchar *key_name);

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
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  registry->priv->plugins =
    g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
  registry->priv->plugin_infos =
    g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
  registry->priv->sources =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  registry->priv->related_keys =
    g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, NULL);
  registry->priv->system_keys =
    g_param_spec_pool_new (FALSE);

  key_id_handler_init (&registry->priv->key_id_handler);

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
set_plugin_rank (GrlPluginRegistry *registry, GrlPluginInfo *info)
{
  info->rank =
    GPOINTER_TO_INT (g_hash_table_lookup (registry->priv->ranks,
					  info->id));
  if (!info->rank) {
    info->rank = GRL_PLUGIN_RANK_DEFAULT;
  }
  GRL_DEBUG ("Plugin rank '%s' : %d", info->id, info->rank);
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
    xmlFreeDoc (doc_ptr);
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

static GrlPluginInfo *
grl_plugin_registry_load_plugin_info (GrlPluginRegistry *registry,
                                      const gchar* plugin_id,
                                      const gchar *file)
{
  GHashTable *info;
  GrlPluginInfo *plugin_info;
  gchar *library_filename;
  gchar *path;
  gchar *plugin_name;

  /* Load plugin information */
  info = get_info_from_plugin_xml (file);

  if (!info) {
    GRL_WARNING ("Invalid information file for '%s' plugin",
                 plugin_id);
    return NULL;
  }

  /* Build plugin library filename */
  plugin_name = g_hash_table_lookup (info, GRL_PLUGIN_INFO_MODULE);
  if (!plugin_name) {
    GRL_WARNING ("Information about '%s' plugin has no reference to module; skipping",
                 plugin_id);
    g_hash_table_unref (info);
    return NULL;
  }
  plugin_name = g_strconcat (plugin_name, "." G_MODULE_SUFFIX, NULL);
  path = g_path_get_dirname (file);
  library_filename = g_build_filename (path, plugin_name, NULL);
  g_free (plugin_name);
  g_free (path);

  /* Build Plugin Info */
  plugin_info = g_new0 (GrlPluginInfo, 1);
  plugin_info->id = g_strdup (plugin_id);
  plugin_info->filename = library_filename;
  plugin_info->optional_info = info;

  /* Set rank */
  set_plugin_rank (registry, plugin_info);

  return plugin_info;
}

static GList *
grl_plugin_registry_load_plugin_info_directory (GrlPluginRegistry *registry,
                                                const gchar *path,
                                                GError **error)
{
  GDir *dir;
  GrlPluginInfo *plugin_info;
  const gchar *entry;
  gchar *file;
  gchar *id;
  gchar *suffix;
  GList *loaded_infos = NULL;

  dir = g_dir_open (path, 0, NULL);
  if (!dir) {
    GRL_WARNING ("Could not open plugin directory: '%s'", path);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "Failed to open plugin directory '%s'", path);
    return NULL;
  }

  while ((entry = g_dir_read_name (dir)) != NULL) {
    if ((suffix = g_strrstr (entry, "." GRL_PLUGIN_INFO_SUFFIX)) != NULL) {
      file = g_build_filename (path, entry, NULL);
      id = g_strndup (entry, suffix - entry);
      /* Skip plugin info if it is already loaded */
      if (g_hash_table_lookup (registry->priv->plugin_infos, id)) {
        GRL_DEBUG ("Information about '%s' plugin already loaded; skipping",
                   id);
        g_free (id);
        g_free (file);
        continue;
      }
      /* Check if plugin is allowed or not */
      if (registry->priv->allowed_plugins &&
          !g_slist_find_custom (registry->priv->allowed_plugins,
                                id,
                                (GCompareFunc) g_strcmp0)) {
        GRL_DEBUG ("'%s' plugin not allowed; skipping", id);
        continue;
      }
      plugin_info = grl_plugin_registry_load_plugin_info (registry, id, file);
      g_free (id);
      g_free (file);

      g_hash_table_insert (registry->priv->plugin_infos,
                           plugin_info->id,
                           plugin_info);
      loaded_infos = g_list_append (loaded_infos, plugin_info);
    }
  }

  g_dir_close (dir);
  return loaded_infos;
}

static void
grl_plugin_registry_load_plugin_info_all (GrlPluginRegistry *registry)
{
  GSList *plugin_dir;
  GList *loaded_plugins;

  for (plugin_dir = registry->priv->plugins_dir;
       plugin_dir;
       plugin_dir = g_slist_next (plugin_dir)) {
    loaded_plugins =
      grl_plugin_registry_load_plugin_info_directory (registry,
                                                      plugin_dir->data,
                                                      NULL);
    g_list_free (loaded_plugins);
  }
}

static gboolean
grl_plugin_registry_load_list (GrlPluginRegistry *registry,
                               GList *plugin_info_list)
{
  GrlPluginInfo *pinfo;
  gboolean loaded_one = FALSE;

  while (plugin_info_list) {
    pinfo = (GrlPluginInfo *) plugin_info_list->data;
    loaded_one |= grl_plugin_registry_load (registry, pinfo->filename, NULL);
    plugin_info_list = g_list_next (plugin_info_list);
  }

  return loaded_one;
}

static void
key_id_handler_init (struct KeyIDHandler *handler)
{
  const gchar *null_string = NULL;
  handler->string_to_id = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  handler->id_to_string = g_array_new (FALSE, /* zero terminated */
                                       TRUE,  /* zero-initialised */
                                       sizeof (const gchar *));
  /* We want indices in ->id_to_string to start from 1, so we add a NULL entry
   * for GRL_METADATA_KEY_INVALID (i.e. 0) */
  g_array_insert_val (handler->id_to_string,
                      GRL_METADATA_KEY_INVALID,
                      null_string);
}

static
GrlKeyID key_id_handler_get_key (struct KeyIDHandler *handler, const gchar *key_name)
{
  gpointer val = g_hash_table_lookup (handler->string_to_id, key_name);
  if (val == NULL)
    return GRL_METADATA_KEY_INVALID;

  return GRLPOINTER_TO_KEYID (val);
}

static const gchar *
key_id_handler_get_name (struct KeyIDHandler *handler, GrlKeyID key)
{
  if (key < handler->id_to_string->len)
    return g_array_index (handler->id_to_string, const gchar *, key);

  return NULL;
}

/**
 * key_id_handler_add:
 * @handler: the handler
 * @key: a specific key for system keys, or GRL_METADATA_KEY_INVALID for it to
 * be assigned
 * @name: the name of the key.
 *
 * Add a new key<->name correspondence.
 *
 * Returns: the new key number, or GRL_METADATA_KEY_INVALID if the key could
 * not be created (typically if @key or @name is already registered).
 */
static GrlKeyID
key_id_handler_add (struct KeyIDHandler *handler, GrlKeyID key, const gchar *name)
{
  GrlKeyID _key = key;

  if (_key == GRL_METADATA_KEY_INVALID) {
    /* existing keys go from 1 to (id_to_string->len - 1), so the next
     * available key is id_to_string->len, which will be incremented by
     * g_array_insert_val() */
    _key = handler->id_to_string->len;
  }

  if (NULL != key_id_handler_get_name (handler, _key)) {
    GRL_WARNING ("Cannot register %d:%s because key is already defined as %s",
                 _key, name, key_id_handler_get_name (handler, _key));
    return GRL_METADATA_KEY_INVALID;
  } else if ( GRL_METADATA_KEY_INVALID != key_id_handler_get_key (handler, name)) {
    /* _key or name is already in use! */
    GRL_WARNING ("Cannot register %d:%s because name is already registered with key %d",
                 _key, name, key_id_handler_get_key (handler, name));
    return GRL_METADATA_KEY_INVALID;
  } else {
    /* name_copy is shared between handler->id_to_string and
     * handler->string_to_id */
    gchar *name_copy = g_strdup (name);

    if (_key >= handler->id_to_string->len)
      g_array_set_size (handler->id_to_string, _key + 1);

    /* yes, g_array_index() is a macro that give you an lvalue */
    g_array_index (handler->id_to_string, const gchar *, _key) = name_copy;
    g_hash_table_insert (handler->string_to_id,
                         name_copy, GRLKEYID_TO_POINTER (_key));
  }

  return _key;

}

static GList *
key_id_handler_get_all_keys (struct KeyIDHandler *handler)
{
  return g_hash_table_get_values (handler->string_to_id);
}


/* ================ PRIVATE API ================ */

/**
 * grl_plugin_registry_restrict_plugins:
 * @registry: the registry instance
 * @plugins: a @NULL-terminated array of plugins identifiers
 *
 * Restrict the plugins that application sees to this list.
 *
 * Other plugins will not be available for the application, unless it uses
 * directly #grl_plugin_registry_load() function.
 **/
void
grl_plugin_registry_restrict_plugins (GrlPluginRegistry *registry,
                                      gchar **plugins)
{
  g_return_if_fail (GRL_IS_PLUGIN_REGISTRY (registry));
  g_return_if_fail (plugins);

  /* Free previous list */
  if (registry->priv->allowed_plugins) {
    g_slist_foreach (registry->priv->allowed_plugins, (GFunc) g_free, NULL);
    g_slist_free (registry->priv->allowed_plugins);
    registry->priv->allowed_plugins = NULL;
  }

  while (*plugins) {
    registry->priv->allowed_plugins = g_slist_prepend (registry->priv->allowed_plugins,
                                                       g_strdup (*plugins));
    plugins++;
  }
}

/* ================ PUBLIC API ================ */

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

  g_free (id);
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
 * @library_filename: the path to the so file
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
                          const gchar *library_filename,
                          GError **error)
{
  GModule *module;
  GrlPluginDescriptor *plugin;
  GrlPluginInfo *plugin_info;
  GList *plugin_configs;
  gchar *dirname;
  gchar *plugin_info_filename;
  gchar *plugin_info_fullpathname;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);

  module = g_module_open (library_filename, G_MODULE_BIND_LAZY);
  if (!module) {
    GRL_WARNING ("Failed to open module: '%s'", library_filename);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "Failed to load plugin at '%s'", library_filename);
    return FALSE;
  }

  if (!g_module_symbol (module, "GRL_PLUGIN_DESCRIPTOR", (gpointer) &plugin)) {
    GRL_WARNING ("Did not find plugin descriptor: '%s'", library_filename);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "'%s' is not a valid plugin file", library_filename);
    g_module_close (module);
    return FALSE;
  }

  if (!plugin->plugin_init ||
      !plugin->plugin_id) {
    GRL_WARNING ("Plugin descriptor is not valid: '%s'", library_filename);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "'%s' is not a valid plugin file", library_filename);
    g_module_close (module);
    return FALSE;
  }

  /* Check if plugin is already loaded */
  if (g_hash_table_lookup (registry->priv->plugins, plugin->plugin_id)) {
    GRL_WARNING ("Plugin is already loaded: '%s'", library_filename);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "'%s' is already loaded", library_filename);
    g_module_close (module);
    return FALSE;
  }

  plugin_info = g_hash_table_lookup (registry->priv->plugin_infos,
                                     plugin->plugin_id);

  /* This happens when the user invokes directly this function: plugin
     information has not been loaded yet */
  if (!plugin_info) {
    plugin_info_filename = g_strconcat (plugin->plugin_id,
                                        "." GRL_PLUGIN_INFO_SUFFIX,
                                        NULL);
    dirname = g_path_get_dirname (library_filename);
    plugin_info_fullpathname = g_build_filename (dirname,
                                                 plugin_info_filename,
                                                 NULL);
    plugin_info = grl_plugin_registry_load_plugin_info (registry,
                                                        plugin->plugin_id,
                                                        plugin_info_filename);
    if (!plugin_info) {
      GRL_WARNING ("Plugin '%s' does not have XML information file '%s'",
                   plugin->plugin_id,
                   plugin_info_fullpathname);
      /* Create a default one */
      plugin_info = g_new0 (GrlPluginInfo, 1);
      plugin_info->id = g_strdup (plugin->plugin_id);
      plugin_info->filename = g_strdup (library_filename);
      g_hash_table_insert (registry->priv->plugin_infos,
                           plugin_info->id,
                           plugin_info);
      plugin_info->optional_info =
        g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
      g_hash_table_insert (plugin_info->optional_info,
                           g_strdup (GRL_PLUGIN_INFO_MODULE),
                           g_path_get_basename (plugin_info->filename));

      set_plugin_rank (registry, plugin_info);
    }
    g_free (plugin_info_filename);
    g_free (dirname);
    g_free (plugin_info_fullpathname);
  }

  g_hash_table_insert (registry->priv->plugins,
                       (gpointer) plugin->plugin_id, plugin);

  plugin_configs = g_hash_table_lookup (registry->priv->configs,
                                        plugin->plugin_id);

  if (!plugin->plugin_init (registry, plugin_info, plugin_configs)) {
    g_hash_table_remove (registry->priv->plugins, plugin->plugin_id);
    GRL_INFO ("Failed to initialize plugin: '%s'", library_filename);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "Failed to initialize plugin at '%s'", library_filename);
    g_module_close (module);
    return FALSE;
  }

  /* Make plugin resident */
  g_module_make_resident (module);

  plugin->module = module;

  GRL_DEBUG ("Loaded plugin '%s' from '%s'", plugin->plugin_id, library_filename);

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
  GList *plugin_infos;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);

  /* Load plugins information */
  plugin_infos = grl_plugin_registry_load_plugin_info_directory (registry,
                                                                 path,
                                                                 error);

  /* Load the plugins */
  if (!grl_plugin_registry_load_list (registry, plugin_infos)) {
    GRL_WARNING ("No plugins loaded from directory '%s'", path);
  }
  g_list_free (plugin_infos);

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
  GList *all_plugin_infos;
  gboolean loaded_one;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), TRUE);

  /* Preload all plugin infos */
  if (!registry->priv->all_plugin_info_loaded) {
    grl_plugin_registry_load_plugin_info_all (registry);
    registry->priv->all_plugin_info_loaded = TRUE;
  }

  /* Now load all plugins */
  all_plugin_infos = g_hash_table_get_values (registry->priv->plugin_infos);
  loaded_one = grl_plugin_registry_load_list (registry, all_plugin_infos);

  g_list_free (all_plugin_infos);

  if (!loaded_one) {
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "All configured plugin paths are invalid. "   \
                 "Failed to load plugins.");
  }

  return loaded_one;
}

/**
 * grl_plugin_registry_load_by_id:
 * @registry: the registry instance
 * @plugin_id: plugin identifier
 * @error: error return location or @NULL to ignore
 *
 * Loads plugin identified by @plugin_id.
 *
 * This requires the XML plugin information file to define a "module" key with
 * the name of the module that provides the plugin or the absolute path of the
 * actual module file.
 *
 * Returns: %TRUE if the plugin is loaded correctly
 *
 * Since: 0.1.14
 **/
gboolean
grl_plugin_registry_load_by_id (GrlPluginRegistry *registry,
                                const gchar *plugin_id,
                                GError **error)
{
  GrlPluginInfo *plugin_info;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);

  /* Check if there is information loaded */
  plugin_info = g_hash_table_lookup (registry->priv->plugin_infos, plugin_id);
    /* Maybe we need to load all plugins before */
  if (!plugin_info &&
      !registry->priv->all_plugin_info_loaded) {
    grl_plugin_registry_load_plugin_info_all (registry);
    registry->priv->all_plugin_info_loaded = TRUE;
    /* Search again */
    plugin_info = g_hash_table_lookup (registry->priv->plugin_infos, plugin_id);
  }

  if (!plugin_info) {
    GRL_WARNING ("There is no information about a plugin with id '%s'", plugin_id);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 "There is no information about a plugin with id '%s'", plugin_id);
    return FALSE;
  }

  return grl_plugin_registry_load (registry, plugin_info->filename, error);
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
 * Returns: %TRUE% on success.
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

  g_hash_table_remove (registry->priv->plugins, plugin_id);

  if (plugin->module) {
    g_module_close (plugin->module);
  }

  return TRUE;
}

/**
 * grl_plugin_registry_register_metadata_key:
 * @registry: The plugin registry
 * @param_spec: The definition of the key to register
 * @error: error return location or @NULL to ignore
 *
 * Registers a metadata key
 *
 * Returns: The #GrlKeyID registered.
 *
 * Since: 0.1.7
 */
GrlKeyID
grl_plugin_registry_register_metadata_key (GrlPluginRegistry *registry,
                                           GParamSpec *param_spec,
                                           GError **error)
{
  return grl_plugin_registry_register_metadata_key_full (registry,
                                                         param_spec,
                                                         GRL_METADATA_KEY_INVALID,
                                                         error);
}

/*
 * grl_plugin_registry_register_metadata_key_full:
 *
 * This is an internal method only meant to be used to register core
 * keys.
 *
 * For internal use. Plugin developers should use
 * grl_plugin_registry_register_metadata_key().
 */
GrlKeyID
grl_plugin_registry_register_metadata_key_full (GrlPluginRegistry *registry,
                                                GParamSpec *param_spec,
                                                GrlKeyID key,
                                                GError **error)
{
  const gchar *key_name;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), 0);
  g_return_val_if_fail (G_IS_PARAM_SPEC (param_spec), 0);
  GrlKeyID registered_key;

  key_name = g_param_spec_get_name (param_spec);

  registered_key = key_id_handler_add (&registry->priv->key_id_handler, key, key_name);

  if (registered_key == GRL_METADATA_KEY_INVALID) {
    GRL_WARNING ("metadata key '%s' cannot be registered", key_name);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_REGISTER_METADATA_KEY_FAILED,
                 "Metadata key '%s' cannot be registered",
                 key_name);

    return GRL_METADATA_KEY_INVALID;
  }

  g_param_spec_pool_insert (registry->priv->system_keys,
                            param_spec,
                            GRL_TYPE_MEDIA);
  /* Each key is related to itself */
  g_hash_table_insert (registry->priv->related_keys,
                       GRLKEYID_TO_POINTER (registered_key),
                       g_list_prepend (NULL,
                                       GRLKEYID_TO_POINTER (registered_key)));

  return registered_key;
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
 *
 * Since: 0.1.10
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
  key1_partners = g_hash_table_lookup (registry->priv->related_keys, GRLKEYID_TO_POINTER (key1));
  key2_partners = g_hash_table_lookup (registry->priv->related_keys, GRLKEYID_TO_POINTER (key2));

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
 * Returns: The metadata key, or GRL_METADATA_KEY_INVALID if not found
 *
 * Since: 0.1.6
 */
GrlKeyID
grl_plugin_registry_lookup_metadata_key (GrlPluginRegistry *registry,
                                         const gchar *key_name)
{
  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), 0);
  g_return_val_if_fail (key_name, 0);

  return key_id_handler_get_key (&registry->priv->key_id_handler, key_name);
}

/**
 * grl_plugin_registry_lookup_metadata_key_name:
 * @registry: the registry instance
 * @key: a metadata key
 *
 * Returns @key name.
 *
 * Returns: metadata key name, or @NULL if not found
 */
const gchar *
grl_plugin_registry_lookup_metadata_key_name (GrlPluginRegistry *registry,
                                              GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), 0);

  return key_id_handler_get_name (&registry->priv->key_id_handler, key);
}

/**
 * grl_plugin_registry_lookup_metadata_key_desc:
 * @registry: the registry instance
 * @key: a metadata key
 *
 * Returns @key description.
 *
 * Returns: metadata key description, or @NULL if not found
 */
const gchar *
grl_plugin_registry_lookup_metadata_key_desc (GrlPluginRegistry *registry,
                                              GrlKeyID key)
{
  const gchar *key_name;
  GParamSpec *key_pspec;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), 0);

  key_name = key_id_handler_get_name (&registry->priv->key_id_handler, key);
  if (!key_name) {
    return NULL;
  }
  key_pspec = g_param_spec_pool_lookup (registry->priv->system_keys,
                                        key_name,
                                        GRL_TYPE_MEDIA,
                                        FALSE);
  if (key_pspec) {
    return g_param_spec_get_blurb (key_pspec);
  } else {
    return NULL;
  }
}

/**
 * grl_plugin_registry_lookup_metadata_key_type:
 * @registry: the registry instance
 * @key: a metadata key
 *
 * Returns @key expected value type.
 *
 * Returns: metadata key type, or @G_TYPE_INVALID if not found
 */
GType
grl_plugin_registry_lookup_metadata_key_type (GrlPluginRegistry *registry,
                                              GrlKeyID key)
{
  const gchar *key_name;
  GParamSpec *key_pspec;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), 0);

  key_name = key_id_handler_get_name (&registry->priv->key_id_handler, key);
  if (!key_name) {
    return G_TYPE_INVALID;
  }
  key_pspec = g_param_spec_pool_lookup (registry->priv->system_keys,
                                        key_name,
                                        GRL_TYPE_MEDIA,
                                        FALSE);
  if (key_pspec) {
    return G_PARAM_SPEC_VALUE_TYPE (key_pspec);
  } else {
    return G_TYPE_INVALID;
  }
}

/**
 * grl_plugin_registry_metadata_key_validate:
 * @registry: the registry instance
 * @key: a metadata key
 * @value: value to be validate
 *
 * Validates @value content complies with the key specification. That is, it has
 * the expected type, and value are within the range specified in key (for
 * integer values).
 *
 * Returns: %TRUE if complies
 **/
gboolean
grl_plugin_registry_metadata_key_validate (GrlPluginRegistry *registry,
                                           GrlKeyID key,
                                           GValue *value)
{
  const gchar *key_name;
  GParamSpec *key_pspec;

  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), FALSE);
  g_return_val_if_fail (G_IS_VALUE (value), FALSE);

  key_name = key_id_handler_get_name (&registry->priv->key_id_handler, key);
  if (!key_name) {
    return FALSE;
  }
  key_pspec = g_param_spec_pool_lookup (registry->priv->system_keys,
                                        key_name,
                                        GRL_TYPE_MEDIA,
                                        FALSE);
  if (key_pspec) {
    return !g_param_value_validate (key_pspec, value);
  } else {
    return FALSE;
  }
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
 * Returns: (element-type GrlKeyID) (transfer none): a #GList of
 * related keys, or @NULL if key is invalid.
 *
 * Since: 0.1.10
 **/
const GList *
grl_plugin_registry_lookup_metadata_key_relation (GrlPluginRegistry *registry,
                                                  GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_PLUGIN_REGISTRY (registry), NULL);

  return g_hash_table_lookup (registry->priv->related_keys, GRLKEYID_TO_POINTER (key));
}

/**
 * grl_plugin_registry_get_metadata_keys:
 * @registry: the registry instance
 *
 * Returns a list with all registered keys in system.
 *
 * Returns: (transfer container) (element-type GrlKeyID): a #GList with all the available
 * #GrlKeyID<!-- -->s. The content of the list should not be modified or freed.
 * Use g_list_free() when done using the list.
 *
 * Since: 0.1.6
 **/
GList *
grl_plugin_registry_get_metadata_keys (GrlPluginRegistry *registry)
{
  return key_id_handler_get_all_keys (&registry->priv->key_id_handler);
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
    g_free (plugin_id);
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
