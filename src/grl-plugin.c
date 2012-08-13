/*
 * Copyright (C) 2010-2012 Igalia S.L.
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
 * SECTION:grl-plugin
 * @short_description: Base class for Grilo Plugins
 * @see_also: #GrlMetadataSource, #GrlMediaSource
 *
 * Grilo is extensible, so #GrlMetadataSource or #GrlMediaSource instances can be
 * loaded at runtime.
 * A plugin system can provide one or more of the basic
 * <application>Grilo</application> #GrlSource subclasses.
 *
 * This is a base class for anything that can be added to a Grilo Plugin.
 */

#include "grl-plugin.h"
#include "grl-plugin-priv.h"
#include "grl-registry.h"
#include "grl-log.h"

#include <string.h>

#define GRL_LOG_DOMAIN_DEFAULT  plugin_log_domain
GRL_LOG_DOMAIN(plugin_log_domain);

#define GRL_PLUGIN_GET_PRIVATE(object)            \
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                \
                               GRL_TYPE_PLUGIN,   \
                               GrlPluginPrivate))

enum {
  PROP_0,
  PROP_LOADED,
  PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = { 0 };

struct _GrlPluginPrivate {
  gchar *id;
  gchar *filename;
  gint rank;
  GModule *module;
  GHashTable *optional_info;
  gboolean loaded;
  gboolean (*load_func) (GrlRegistry *, GrlPlugin *, GList *);
  void (*unload_func) (GrlPlugin *);
};

static void grl_plugin_finalize (GObject *object);

static void grl_plugin_get_property (GObject *object,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec);

/* ================ GrlPlugin GObject ================ */

G_DEFINE_TYPE (GrlPlugin, grl_plugin, G_TYPE_OBJECT);

static void
grl_plugin_class_init (GrlPluginClass *plugin_class)
{
  GObjectClass *gobject_class;
  gobject_class = G_OBJECT_CLASS (plugin_class);

  gobject_class->finalize = grl_plugin_finalize;
  gobject_class->get_property = grl_plugin_get_property;

  properties[PROP_LOADED] = g_param_spec_boolean ("loaded",
                                                  "Loaded",
                                                  "Plugin is loaded",
                                                  FALSE,
                                                  G_PARAM_READABLE |
                                                  G_PARAM_STATIC_STRINGS);

  /**
   * GrlPlugin:loaded:
   *
   * @TRUE if plugin is loaded.
   *
   * Since: 0.2.0
   */
  g_object_class_install_property (gobject_class,
                                   PROP_LOADED,
                                   properties[PROP_LOADED]);

  g_type_class_add_private (plugin_class,
                            sizeof (GrlPluginPrivate));
}

static void
grl_plugin_init (GrlPlugin *plugin)
{
  plugin->priv = GRL_PLUGIN_GET_PRIVATE (plugin);
  plugin->priv->optional_info = g_hash_table_new_full (g_str_hash,
                                                       g_str_equal,
                                                       g_free,
                                                       g_free);
}

static void
grl_plugin_finalize (GObject *object)
{
  GrlPlugin *plugin = GRL_PLUGIN (object);

  g_free (plugin->priv->id);
  g_free (plugin->priv->filename);
  g_hash_table_unref (plugin->priv->optional_info);

  G_OBJECT_CLASS (grl_plugin_parent_class)->finalize (object);
}

static void
grl_plugin_get_property (GObject *object,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *pspec)
{
  GrlPlugin *plugin = GRL_PLUGIN (object);

  switch (prop_id) {
  case PROP_LOADED:
    g_value_set_boolean (value, plugin->priv->loaded);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

/* ============ PRIVATE API ============ */

/*
 * grl_plugin_set_optional_info:
 * @plugin: a plugin
 * @info: a hashtable containing optional information
 *
 * Sets the optional information. Takes ownership of @info table.
 */
void
grl_plugin_set_optional_info (GrlPlugin *plugin,
                              GHashTable *info)
{
  g_return_if_fail (GRL_IS_PLUGIN (plugin));

  g_hash_table_unref (plugin->priv->optional_info);
  plugin->priv->optional_info = info;
}

/*
 * grl_plugin_set_load_func:
 * @plugin: a plugin
 * @load_function: a function
 *
 * Sets the function to be executed when plugin is loaded
 */
void
grl_plugin_set_load_func (GrlPlugin *plugin,
                          gpointer load_function)
{
  g_return_if_fail (GRL_IS_PLUGIN (plugin));

  plugin->priv->load_func = load_function;
}

/*
 * grl_plugin_set_unload_func:
 * @plugin: a plugin
 * @unload_function: a function
 *
 * Sets the function to be executed when plugin is unloaded
 */
void
grl_plugin_set_unload_func (GrlPlugin *plugin,
                            gpointer unload_function)
{
  g_return_if_fail (GRL_IS_PLUGIN (plugin));

  plugin->priv->unload_func = unload_function;
}

/*
 * grl_plugin_load:
 * @plugin: a plugin
 * @configurations: (element-type Grl.Config): a list of configurations
 *
 * Load the plugin
 *
 * Returns: @TRUE if loaded was successful
 */
gboolean
grl_plugin_load (GrlPlugin *plugin,
                 GList *configurations)
{
  GrlRegistry *registry;

  g_return_val_if_fail (GRL_IS_PLUGIN (plugin), FALSE);

  if (!plugin->priv->load_func) {
    return FALSE;
  }

  registry = grl_registry_get_default ();

  if (!plugin->priv->load_func (registry, plugin, configurations)) {
    return FALSE;
  }

  plugin->priv->loaded = TRUE;
  g_object_notify_by_pspec (G_OBJECT (plugin), properties[PROP_LOADED]);

  return TRUE;
}

/*
 * grl_plugin_unload:
 * @plugin: a plugin
 *
 * Unloads the plugin
 */
void
grl_plugin_unload (GrlPlugin *plugin)
{
  g_return_if_fail (GRL_IS_PLUGIN (plugin));

  if (plugin->priv->unload_func) {
    plugin->priv->unload_func (plugin);
  }

  plugin->priv->loaded = FALSE;
  g_object_notify_by_pspec (G_OBJECT (plugin), properties[PROP_LOADED]);
}

/*
 * grl_plugin_set_id:
 * @plugin: a plugin
 * @id: plugin identifier
 *
 * Sets the id of the plugin
 */
void
grl_plugin_set_id (GrlPlugin *plugin,
                   const gchar *id)
{
  g_return_if_fail (GRL_IS_PLUGIN (plugin));

  if (plugin->priv->id) {
    g_free (plugin->priv->id);
  }
  plugin->priv->id = g_strdup (id);
}

/*
 * grl_plugin_set_filename:
 * @plugin: a plugin
 * @filename: a filename
 *
 * Sets the filename containing the plugin
 */
void
grl_plugin_set_filename (GrlPlugin *plugin,
                         const gchar *filename)
{
  g_return_if_fail (GRL_IS_PLUGIN (plugin));

  if (plugin->priv->filename) {
    g_free (plugin->priv->filename);
  }

  plugin->priv->filename = g_strdup (filename);
}

/*
 * grl_plugin_set_module:
 * @plugin: a plugin
 * @module: a module
 *
 * Sets the module of the plugin
 */
void
grl_plugin_set_module (GrlPlugin *plugin,
                       GModule *module)
{
  g_return_if_fail (GRL_IS_PLUGIN (plugin));
  plugin->priv->module = module;
}

/*
 * grl_plugin_set_info:
 * @plugin: a plugin
 * @key: key representing information about this plugin
 * @value: the information itself
 *
 * Sets the information of the @plugin that is associaed with the given @key.
 */
void
grl_plugin_set_info (GrlPlugin *plugin,
                     const gchar *key,
                     const gchar *value)
{
  g_return_if_fail (GRL_IS_PLUGIN (plugin));

  g_hash_table_insert (plugin->priv->optional_info,
                       g_strdup (key),
                       g_strdup (value));
}

/* ============ PUBLIC API ============= */

/**
 * grl_plugin_get_name:
 * @plugin: a plugin
 *
 * Get the name of the plugin
 *
 * Returns: the name of the @plugin
 *
 * Since: 0.2.0
 */
const gchar *
grl_plugin_get_name (GrlPlugin *plugin)
{
  return grl_plugin_get_info (plugin, GRL_PLUGIN_NAME);
}

/**
 * grl_plugin_get_description:
 * @plugin: a plugin
 *
 * Get the description of the plugin
 *
 * Returns: the description of the @plugin
 *
 * Since: 0.2.0
 */
const gchar *
grl_plugin_get_description (GrlPlugin *plugin)
{
  return grl_plugin_get_info (plugin, GRL_PLUGIN_DESCRIPTION);
}

/**
 * grl_plugin_get_version:
 * @plugin: a plugin
 *
 * Get the version of the plugin
 *
 * Returns: the version of the @plugin
 *
 * Since: 0.2.0
 */
const gchar *
grl_plugin_get_version (GrlPlugin *plugin)
{
  return grl_plugin_get_info (plugin, GRL_PLUGIN_VERSION);
}

/**
 * grl_plugin_get_license:
 * @plugin: a plugin
 *
 * Get the license of the plugin
 *
 * Returns: the license of the @plugin
 *
 * Since: 0.2.0
 */
const gchar *
grl_plugin_get_license (GrlPlugin *plugin)
{
  return grl_plugin_get_info (plugin, GRL_PLUGIN_LICENSE);
}

/**
 * grl_plugin_get_author:
 * @plugin: a plugin
 *
 * Get the author of the plugin
 *
 * Returns: the author of the @plugin
 *
 * Since: 0.2.0
 */
const gchar *
grl_plugin_get_author (GrlPlugin *plugin)
{
  return grl_plugin_get_info (plugin, GRL_PLUGIN_AUTHOR);
}

/**
 * grl_plugin_get_site:
 * @plugin: a plugin
 *
 * Get the site of the plugin
 *
 * Returns: the site of the @plugin
 *
 * Since: 0.2.0
 */
const gchar *
grl_plugin_get_site (GrlPlugin *plugin)
{
  return grl_plugin_get_info (plugin, GRL_PLUGIN_SITE);
}

/**
 * grl_plugin_get_id:
 * @plugin: a plugin
 *
 * Get the id of the plugin
 *
 * Returns: the id of the @plugin
 *
 * Since: 0.2.0
 */
const gchar *
grl_plugin_get_id (GrlPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_PLUGIN (plugin), NULL);

  return plugin->priv->id;
}

/**
 * grl_plugin_get_filename:
 * @plugin: a plugin
 *
 * Get the filename containing the plugin
 *
 * Returns: the filename containing @plugin
 *
 * Since: 0.2.0
 */
const gchar *
grl_plugin_get_filename (GrlPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_PLUGIN (plugin), NULL);

  return plugin->priv->filename;
}

/**
 * grl_plugin_get_module: (skip)
 * @plugin: a plugin
 *
 * Gets the #GModule containing the plugin
 *
 * Returns: a #GModule
 *
 * Since: 0.2.0
 **/
GModule *
grl_plugin_get_module (GrlPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_PLUGIN (plugin), NULL);
  return plugin->priv->module;
}

/**
 * grl_plugin_get_info_keys:
 * @plugin: a plugin
 *
 * Returns a list of keys that can be queried to retrieve information about the
 * plugin.
 *
 * Returns: (transfer container) (element-type utf8):
 * a #GList of strings containing the keys. The content of the list is
 * owned by the plugin and should not be modified or freed. Use g_list_free()
 * when done using the list.
 *
 * Since: 0.2.0
 **/
GList *
grl_plugin_get_info_keys (GrlPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_PLUGIN (plugin), NULL);

  if (plugin->priv->optional_info) {
    return g_hash_table_get_keys (plugin->priv->optional_info);
  } else {
    return NULL;
  }
}

/**
 * grl_plugin_get_info:
 * @plugin: a plugin
 * @key: a key representing information about this plugin
 *
 * Get the information of the @plugin that is associated with the given key
 *
 * Returns: the information assigned to the given @key or NULL if there is no such information
 *
 * Since: 0.2.0
 */
const gchar *
grl_plugin_get_info (GrlPlugin *plugin, const gchar *key)
{
  g_return_val_if_fail (GRL_IS_PLUGIN (plugin), NULL);

  if (!plugin->priv->optional_info) {
    return NULL;
  }

  return g_hash_table_lookup (plugin->priv->optional_info, key);
}

/**
 * grl_plugin_get_sources:
 * @plugin: a plugin
 *
 * Gets the sources belonging to @plugin.
 *
 * Returns: (transfer container) (element-type Grl.Source): a #GList of
 * #GrlSource<!-- -->s. The content of the list should not be modified or
 * freed. Use g_list_free() when done using the list.
 *
 * Since: 0.2.0
 **/
GList *
grl_plugin_get_sources (GrlPlugin *plugin)
{
  GrlRegistry *registry;
  GList *all_sources;
  GList *plugin_sources = NULL;
  GList *sources_iter;

  g_return_val_if_fail (GRL_IS_PLUGIN (plugin), NULL);

  registry = grl_registry_get_default ();
  all_sources = grl_registry_get_sources (registry, FALSE);

  for (sources_iter = all_sources;
       sources_iter;
       sources_iter = g_list_next (sources_iter)) {
    if (grl_source_get_plugin (GRL_SOURCE (sources_iter->data)) == plugin) {
      plugin_sources = g_list_prepend (plugin_sources, sources_iter->data);
    }
  }

  g_list_free (all_sources);

  return plugin_sources;
}
