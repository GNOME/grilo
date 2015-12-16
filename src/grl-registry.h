/*
 * Copyright (C) 2010, 2011 Igalia S.L.
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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#ifndef _GRL_REGISTRY_H_
#define _GRL_REGISTRY_H_

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <grl-source.h>
#include <grl-metadata-key.h>
#include <grl-config.h>
#include <grl-definitions.h>
#include <grl-plugin.h>

#define GRL_PLUGIN_PATH_VAR "GRL_PLUGIN_PATH"
#define GRL_PLUGIN_LIST_VAR "GRL_PLUGIN_LIST"
#define GRL_PLUGIN_RANKS_VAR "GRL_PLUGIN_RANKS"

/* Macros */

#define GRL_TYPE_REGISTRY                       \
  (grl_registry_get_type ())

#define GRL_REGISTRY(obj)                                       \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                           \
                               GRL_TYPE_REGISTRY,               \
                               GrlRegistry))

#define GRL_IS_REGISTRY(obj)                                    \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                           \
                               GRL_TYPE_REGISTRY))

#define GRL_REGISTRY_CLASS(klass)                       \
  (G_TYPE_CHECK_CLASS_CAST((klass),                     \
                           GRL_TYPE_REGISTRY,           \
                           GrlRegistryClass))

#define GRL_IS_REGISTRY_CLASS(klass)                    \
  (G_TYPE_CHECK_CLASS_TYPE((klass),                     \
                           GRL_TYPE_REGISTRY))

#define GRL_REGISTRY_GET_CLASS(obj)                     \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_REGISTRY,        \
                              GrlRegistryClass))

/* Plugin registration */

/**
* GRL_PLUGIN_DEFINE:
* @major_version: the major version number of core that plugin was compiled for
* @minor_version: the minor version number of core that plugin was compiled for
* @id: the plugin identifier
* @name: name of plugin
* @description: description of plugin
* @author: author of plugin
* @version: version of plugin
* @license: license of plugin
* @site: URL to provider of plugin
* @init: the module initialization. It shall instantiate
* the #GrlPlugins provided
* @deinit: (allow-none): function to execute when the registry needs to dispose
* the module.
* @register_keys: (allow-none): function to execute before loading the
* plugin. It's aim is to register new keys
*
* Since: 0.3.0
*/
#define GRL_PLUGIN_DEFINE(major,                                \
                          minor,                                \
                          id,                                   \
                          name,                                 \
                          description,                          \
                          author,                               \
                          version,                              \
                          license,                              \
                          site,                                 \
                          init,                                 \
                          deinit,                               \
                          register_keys)                        \
  G_MODULE_EXPORT GrlPluginDescriptor GRL_PLUGIN_DESCRIPTOR = { \
    major,                                                      \
    minor,                                                      \
    id,                                                         \
    name,                                                       \
    description,                                                \
    author,                                                     \
    version,                                                    \
    license,                                                    \
    site,                                                       \
    init,                                                       \
    deinit,                                                     \
    register_keys                                               \
  }

/* Plugin descriptor */

typedef struct _GrlRegistry GrlRegistry;

typedef struct _GrlPluginDescriptor  GrlPluginDescriptor;

typedef gboolean (*GrlPluginInitFunc) (GrlRegistry *registry,
                                       GrlPlugin *plugin,
                                       GList *configs);

typedef void (*GrlPluginDeinitFunc) (GrlPlugin *plugin);

typedef void (*GrlPluginRegisterKeysFunc) (GrlRegistry *registry,
                                           GrlPlugin *plugin);

/**
* GrlPluginDescriptor:
* @major_version: the major version number of core that plugin was compiled for
* @minor_version: the minor version number of core that plugin was compiled for
* @id: the plugin identifier
* @name: name of plugin
* @description: description of plugin
* @author: author of plugin
* @version: version of plugin
* @license: license of plugin
* @site: URL to provider of plugin
* @plugin_init: the module initialization. It shall instantiate
* the #GrlPlugins provided
* @plugin_deinit: function to execute when the registry needs
* to dispose the module.
* @plugin_register_keys: function to execute before loading the plugin. It's aim
* is to register new keys
*
* This structure is used for the module loader
*
* Since: 0.3.0
*/
struct _GrlPluginDescriptor {
  gint major_version;
  gint minor_version;
  gchar *id;
  gchar *name;
  gchar *description;
  gchar *author;
  gchar *version;
  gchar *license;
  gchar *site;

  GrlPluginInitFunc init;
  GrlPluginDeinitFunc deinit;
  GrlPluginRegisterKeysFunc register_keys;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

/* Plugin ranks */

/**
 * GrlRank:
 * @GRL_RANK_LOWEST: will be chosen last or not at all
 * @GRL_RANK_LOW: unlikely to be chosen
 * @GRL_RANK_DEFAULT: likely to be chosen
 * @GRL_RANK_HIGH: will be chosen
 * @GRL_RANK_HIGHEST: will be chosen first
 *
 * Source priority ranks. Defines the order in which the resolver
 * (or similar rank-picking mechanisms) will choose this source
 * over an alternative one with the same function.
 *
 * These constants serve as a rough guidance for defining the rank
 * of a GrlSource. Any value is valid, including values bigger
 * than GRL_RANK_HIGHEST.
 */
typedef enum {
  GRL_RANK_LOWEST  = -64,
  GRL_RANK_LOW     = -32,
  GRL_RANK_DEFAULT =   0,
  GRL_RANK_HIGH    =  32,
  GRL_RANK_HIGHEST =  64
} GrlRank;

/* GrlRegistry object */

typedef struct _GrlRegistryPrivate GrlRegistryPrivate;

struct _GrlRegistry {

  GObject parent;

  /*< private >*/
  GrlRegistryPrivate *priv;

  gpointer _grl_reserved[GRL_PADDING];
};

/* GrlRegistry class */

typedef struct _GrlRegistryClass GrlRegistryClass;

/**
 * GrlRegistryClass:
 * @parent_class: the parent class structure
 *
 * Grilo Registry class. Dynamic loader of plugins.
 */
struct _GrlRegistryClass {

  GObjectClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

G_BEGIN_DECLS

GType grl_registry_get_type (void);

GrlRegistry *grl_registry_get_default (void);

void grl_registry_add_directory (GrlRegistry *registry,
                                 const gchar *path);

gboolean grl_registry_load_plugin (GrlRegistry *registry,
                                   const gchar *library_filename,
                                   GError **error);

gboolean grl_registry_load_plugin_from_desc (GrlRegistry *registry,
                                             GrlPluginDescriptor *plugin_desc,
                                             GError **error);

gboolean grl_registry_load_plugin_directory (GrlRegistry *registry,
                                             const gchar *path,
                                             GError **error);

gboolean grl_registry_unload_plugin (GrlRegistry *registry,
                                     const gchar *plugin_id,
                                     GError **error);

gboolean grl_registry_activate_all_plugins (GrlRegistry *registry);

gboolean grl_registry_load_all_plugins (GrlRegistry *registry,
                                        gboolean activate,
                                        GError **error);

gboolean grl_registry_activate_plugin_by_id (GrlRegistry *registry,
                                             const gchar *plugin_id,
                                             GError **error);

gboolean grl_registry_register_source (GrlRegistry *registry,
                                       GrlPlugin *plugin,
                                       GrlSource *source,
                                       GError **error);

gboolean grl_registry_unregister_source (GrlRegistry *registry,
                                         GrlSource *source,
                                         GError **error);

GrlSource *grl_registry_lookup_source (GrlRegistry *registry,
                                       const gchar *source_id);

GList *grl_registry_get_sources (GrlRegistry *registry,
                                 gboolean ranked);

GList *grl_registry_get_sources_by_operations (GrlRegistry *registry,
                                               GrlSupportedOps ops,
                                               gboolean ranked);

GrlPlugin *grl_registry_lookup_plugin (GrlRegistry *registry,
                                       const gchar *plugin_id);

GList *grl_registry_get_plugins (GrlRegistry *registry,
                                 gboolean only_loaded);


GrlKeyID grl_registry_register_metadata_key (GrlRegistry *registry,
                                             GParamSpec *param_spec,
                                             GrlKeyID bind_key,
                                             GError **error);

GrlKeyID grl_registry_lookup_metadata_key (GrlRegistry *registry,
                                           const gchar *key_name);

const GList *grl_registry_lookup_metadata_key_relation (GrlRegistry *registry,
                                                        GrlKeyID key);

const gchar *grl_registry_lookup_metadata_key_name (GrlRegistry *registry,
                                                    GrlKeyID key);

const gchar *grl_registry_lookup_metadata_key_desc (GrlRegistry *registry,
                                                    GrlKeyID key);

GType grl_registry_lookup_metadata_key_type (GrlRegistry *registry,
                                             GrlKeyID key);

gboolean grl_registry_metadata_key_validate (GrlRegistry *registry,
                                             GrlKeyID key,
                                             GValue *value);

GList *grl_registry_get_metadata_keys (GrlRegistry *registry);

gboolean grl_registry_add_config (GrlRegistry *registry,
                                  GrlConfig *config,
                                  GError **error);

gboolean grl_registry_add_config_from_file (GrlRegistry *registry,
                                            const gchar *config_file,
                                            GError **error);

gboolean grl_registry_add_config_from_resource (GrlRegistry *registry,
                                                const gchar *resource_path,
                                                GError **error);

G_END_DECLS

#endif /* _GRL_REGISTRY_H_ */
