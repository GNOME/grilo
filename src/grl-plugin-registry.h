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

#ifndef _GRL_PLUGIN_REGISTRY_H_
#define _GRL_PLUGIN_REGISTRY_H_

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <grl-media-source.h>
#include <grl-metadata-key.h>
#include <grl-config.h>
#include <grl-definitions.h>
#include <grl-plugin.h>

#define GRL_PLUGIN_PATH_VAR "GRL_PLUGIN_PATH"
#define GRL_PLUGIN_LIST_VAR "GRL_PLUGIN_LIST"
#define GRL_PLUGIN_RANKS_VAR "GRL_PLUGIN_RANKS"

/* Macros */

#define GRL_TYPE_PLUGIN_REGISTRY                \
  (grl_plugin_registry_get_type ())

#define GRL_PLUGIN_REGISTRY(obj)                                \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                           \
                               GRL_TYPE_PLUGIN_REGISTRY,        \
                               GrlPluginRegistry))

#define GRL_IS_PLUGIN_REGISTRY(obj)                             \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                           \
                               GRL_TYPE_PLUGIN_REGISTRY))

#define GRL_PLUGIN_REGISTRY_CLASS(klass)                \
  (G_TYPE_CHECK_CLASS_CAST((klass),                     \
                           GRL_TYPE_PLUGIN_REGISTRY,    \
                           GrlPluginRegistryClass))

#define GRL_IS_PLUGIN_REGISTRY_CLASS(klass)             \
  (G_TYPE_CHECK_CLASS_TYPE((klass),                     \
                           GRL_TYPE_PLUGIN_REGISTRY))

#define GRL_PLUGIN_REGISTRY_GET_CLASS(obj)              \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_PLUGIN_REGISTRY, \
                              GrlPluginRegistryClass))

/* Plugin registration */

/**
 * GRL_PLUGIN_REGISTER:
 * @init: the module initialization. It shall instantiate
 * the #GrlPlugins provided
 * @deinit: (allow-none): function to execute when the registry needs to dispose the module
 * @id: the module identifier
 *
 * Define the boilerplate for loadable modules. Defines a new module
 * descriptor which provides a set of #GrlPlugins
 */
#define GRL_PLUGIN_REGISTER(init,               \
                            deinit,             \
                            id)			\
  G_MODULE_EXPORT GrlPluginDescriptor GRL_PLUGIN_DESCRIPTOR = {		\
    .plugin_id = id,                                                    \
    .plugin_init = init,						\
    .plugin_deinit = deinit,						\
    .module = NULL							\
  }

/* Plugin descriptor */

typedef struct _GrlPluginRegistry GrlPluginRegistry;

typedef struct _GrlPluginDescriptor  GrlPluginDescriptor;

/**
* GrlPluginDescriptor:
* @plugin_id: the module identifier
* @plugin_init: the module initialization. It shall instantiate
* the #GrlPlugins provided
* @plugin_deinit: function to execute when the registry needs
* to dispose the module.
* @module: the #GModule instance.
*
* This structure is used for the module loader
*/
struct _GrlPluginDescriptor {
  gchar *plugin_id;
  gboolean (*plugin_init) (GrlPluginRegistry *, GrlPlugin *, GList *);
  void (*plugin_deinit) (GrlPlugin *);
  GModule *module;

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
  GRL_RANK_HIGHEST =  64,
} GrlRank;

/* GrlPluginRegistry object */

typedef struct _GrlPluginRegistryPrivate GrlPluginRegistryPrivate;

struct _GrlPluginRegistry {

  GObject parent;

  /*< private >*/
  GrlPluginRegistryPrivate *priv;

  gpointer _grl_reserved[GRL_PADDING];
};

/* GrlPluginRegistry class */

typedef struct _GrlPluginRegistryClass GrlPluginRegistryClass;

/**
 * GrlPluginRegistryClass:
 * @parent_class: the parent class structure
 *
 * Grilo PluginRegistry class. Dynamic loader of plugins.
 */
struct _GrlPluginRegistryClass {

  GObjectClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

G_BEGIN_DECLS

GType grl_plugin_registry_get_type (void);

GrlPluginRegistry *grl_plugin_registry_get_default (void);

void grl_plugin_registry_add_directory (GrlPluginRegistry *registry,
                                        const gchar *path);

gboolean grl_plugin_registry_load (GrlPluginRegistry *registry,
                                   const gchar *library_filename,
                                   GError **error);

gboolean grl_plugin_registry_load_directory (GrlPluginRegistry *registry,
                                             const gchar *path,
                                             GError **error);

gboolean grl_plugin_registry_unload (GrlPluginRegistry *registry,
                                     const gchar *plugin_id,
                                     GError **error);

gboolean grl_plugin_registry_load_all (GrlPluginRegistry *registry,
                                       GError **error);

gboolean grl_plugin_registry_load_by_id (GrlPluginRegistry *registry,
                                         const gchar *plugin_id,
                                         GError **error);

gboolean grl_plugin_registry_register_source (GrlPluginRegistry *registry,
                                              GrlPlugin *plugin,
                                              GrlSource *source,
                                              GError **error);

gboolean grl_plugin_registry_unregister_source (GrlPluginRegistry *registry,
                                                GrlSource *source,
                                                GError **error);

GrlSource *grl_plugin_registry_lookup_source (GrlPluginRegistry *registry,
                                              const gchar *source_id);

GList *grl_plugin_registry_get_sources (GrlPluginRegistry *registry,
						  gboolean ranked);

GList *grl_plugin_registry_get_sources_by_operations (GrlPluginRegistry *registry,
                                                                GrlSupportedOps ops,
                                                                gboolean ranked);

GrlKeyID grl_plugin_registry_register_metadata_key (GrlPluginRegistry *registry,
                                                    GParamSpec *param_spec,
                                                    GError **error);

/* internal! FIXME: put in a separate header */
GrlKeyID grl_plugin_registry_register_metadata_key_full (GrlPluginRegistry *registry,
                                                         GParamSpec *param_spec,
                                                         GrlKeyID key,
                                                         GError **error);

void grl_plugin_registry_register_metadata_key_relation (GrlPluginRegistry *registry,
                                                         GrlKeyID key1,
                                                         GrlKeyID key2);

GrlKeyID grl_plugin_registry_lookup_metadata_key (GrlPluginRegistry *registry,
                                                  const gchar *key_name);

const GList *grl_plugin_registry_lookup_metadata_key_relation (GrlPluginRegistry *registry,
                                                               GrlKeyID key);

const gchar *grl_plugin_registry_lookup_metadata_key_name (GrlPluginRegistry *registry,
                                                           GrlKeyID key);

const gchar *grl_plugin_registry_lookup_metadata_key_desc (GrlPluginRegistry *registry,
                                                           GrlKeyID key);

GType grl_plugin_registry_lookup_metadata_key_type (GrlPluginRegistry *registry,
                                                    GrlKeyID key);

gboolean grl_plugin_registry_metadata_key_validate (GrlPluginRegistry *registry,
                                                    GrlKeyID key,
                                                    GValue *value);

GList *grl_plugin_registry_get_metadata_keys (GrlPluginRegistry *registry);

gboolean grl_plugin_registry_add_config (GrlPluginRegistry *registry,
                                         GrlConfig *config,
                                         GError **error);

gboolean grl_plugin_registry_add_config_from_file (GrlPluginRegistry *registry,
                                                   const gchar *config_file,
                                                   GError **error);

G_END_DECLS

#endif /* _GRL_PLUGIN_REGISTRY_H_ */
