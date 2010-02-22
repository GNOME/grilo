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

#ifndef _GRL_PLUGIN_REGISTRY_H_
#define _GRL_PLUGIN_REGISTRY_H_

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <grl-media-source.h>
#include <grl-metadata-key.h>

#define GRL_PLUGIN_PATH_VAR "GRL_PLUGIN_PATH"
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

#define GRL_PLUGIN_REGISTER(init,               \
                            deinit,             \
                            id,                 \
                            name,               \
                            desc,               \
                            version,            \
                            author,             \
                            license,            \
                            site)                                       \
  G_MODULE_EXPORT GrlPluginDescriptor GRL_PLUGIN_DESCRIPTOR = {		\
    {									\
      id,								\
      name,								\
      desc,								\
      version,								\
      author,								\
      license,								\
      site,								\
    },									\
    .plugin_init = init,						\
    .plugin_deinit = deinit,						\
  }

/* Plugin descriptor */

typedef struct _GrlPluginRegistry GrlPluginRegistry;

typedef struct _GrlPluginInfo {
  const gchar *id;
  const gchar *name;
  const gchar *desc;
  const gchar *version;
  const gchar *author;
  const gchar *license;
  const gchar *site;
  gint rank;
} GrlPluginInfo;

typedef struct _GrlPluginDescriptor {
  GrlPluginInfo info;
  gboolean (*plugin_init) (GrlPluginRegistry *, const GrlPluginInfo *);
  void (*plugin_deinit) (void);
} GrlPluginDescriptor;

/* Plugin ranks */

typedef enum {
  GRL_PLUGIN_RANK_LOWEST  = -64,
  GRL_PLUGIN_RANK_LOW     = -32,
  GRL_PLUGIN_RANK_DEFAULT =   0,
  GRL_PLUGIN_RANK_HIGH    =  32,
  GRL_PLUGIN_RANK_HIGHEST =  64,
} GrlPluginRank;

/* GrlPluginRegistry object */

typedef struct _GrlPluginRegistryPrivate GrlPluginRegistryPrivate;

struct _GrlPluginRegistry {

  GObject parent;

  /*< private >*/
  GrlPluginRegistryPrivate *priv;
};

/* GrlPluginRegistry class */

typedef struct _GrlPluginRegistryClass GrlPluginRegistryClass;

struct _GrlPluginRegistryClass {

  GObjectClass parent_class;
};

G_BEGIN_DECLS

GType grl_plugin_registry_get_type (void);

GrlPluginRegistry *grl_plugin_registry_get_instance (void);

gboolean grl_plugin_registry_load (GrlPluginRegistry *registry,
                                   const gchar *path);

gboolean grl_plugin_registry_load_directory (GrlPluginRegistry *registry,
                                             const gchar *path);

void grl_plugin_registry_unload (GrlPluginRegistry *registry,
                                 const gchar *plugin_id);

gboolean grl_plugin_registry_load_all (GrlPluginRegistry *registry);

gboolean grl_plugin_registry_register_source (GrlPluginRegistry *registry,
                                              const GrlPluginInfo *plugin,
                                              GrlMediaPlugin *source);

void grl_plugin_registry_unregister_source (GrlPluginRegistry *registry,
                                            GrlMediaPlugin *source);

GrlMediaPlugin *grl_plugin_registry_lookup_source (GrlPluginRegistry *registry,
                                                   const gchar *source_id);

GrlMediaPlugin **grl_plugin_registry_get_sources (GrlPluginRegistry *registry,
						  gboolean ranked);

const GrlMetadataKey *grl_plugin_registry_lookup_metadata_key (GrlPluginRegistry *registry,
                                                               GrlKeyID key_id);

G_END_DECLS

#endif /* _GRL_PLUGIN_REGISTRY_H_ */
