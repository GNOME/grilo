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

#ifndef _MS_PLUGIN_REGISTRY_H_
#define _MS_PLUGIN_REGISTRY_H_

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <ms-media-source.h>
#include <ms-metadata-key.h>

#define MS_PLUGIN_PATH_VAR "MS_PLUGIN_PATH"

/* Macros */

#define MS_TYPE_PLUGIN_REGISTRY (ms_plugin_registry_get_type ())
#define MS_PLUGIN_REGISTRY(obj)						\
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MS_TYPE_PLUGIN_REGISTRY, MsPluginRegistry))
#define MS_IS_PLUGIN_REGISTRY(obj)                              \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MS_TYPE_PLUGIN_REGISTRY))
#define MS_PLUGIN_REGISTRY_CLASS(klass)					\
  (G_TYPE_CHECK_CLASS_CAST((klass), MS_TYPE_PLUGIN_REGISTRY,  MsPluginRegistryClass))
#define MS_IS_PLUGIN_REGISTRY_CLASS(klass)			\
  (G_TYPE_CHECK_CLASS_TYPE((klass), MS_TYPE_PLUGIN_REGISTRY))
#define MS_PLUGIN_REGISTRY_GET_CLASS(obj)                               \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MS_TYPE_PLUGIN_REGISTRY, MsPluginRegistryClass))

/* Plugin registration */

#define MS_PLUGIN_REGISTER(init,deinit,id,name,desc,version,author,license,site) \
  G_MODULE_EXPORT MsPluginDescriptor MS_PLUGIN_DESCRIPTOR = {		\
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

typedef struct _MsPluginRegistry MsPluginRegistry;

typedef struct _MsPluginInfo {
  const gchar *id;
  const gchar *name;
  const gchar *desc;
  const gchar *version;
  const gchar *author;
  const gchar *license;
  const gchar *site;
} MsPluginInfo;

typedef struct _MsPluginDescriptor {
  MsPluginInfo info;
  gboolean (*plugin_init) (MsPluginRegistry *, const MsPluginInfo *);
  void (*plugin_deinit) (void);
} MsPluginDescriptor;

/* MsPluginRegistry object */

typedef struct _MsPluginRegistryPrivate MsPluginRegistryPrivate;

struct _MsPluginRegistry {

  GObject parent;

  /*< private >*/
  MsPluginRegistryPrivate *priv;
};

/* MsPluginRegistry class */

typedef struct _MsPluginRegistryClass MsPluginRegistryClass;

struct _MsPluginRegistryClass {

  GObjectClass parent_class;  
};

G_BEGIN_DECLS

GType ms_plugin_registry_get_type (void);

MsPluginRegistry *ms_plugin_registry_get_instance (void);
gboolean ms_plugin_registry_load (MsPluginRegistry *registry, const gchar *path);
gboolean ms_plugin_registry_load_directory (MsPluginRegistry *registry, const gchar *path);
void ms_plugin_registry_unload (MsPluginRegistry *registry, const gchar *plugin_id);
gboolean ms_plugin_registry_load_all (MsPluginRegistry *registry);
gboolean ms_plugin_registry_register_source (MsPluginRegistry *registry, const MsPluginInfo *plugin, MsMediaPlugin *source);
void ms_plugin_registry_unregister_source (MsPluginRegistry *registry, MsMediaPlugin *source);
MsMediaPlugin *ms_plugin_registry_lookup_source (MsPluginRegistry *registry, const gchar *source_id);
MsMediaPlugin **ms_plugin_registry_get_sources (MsPluginRegistry *registry);
const MsMetadataKey *ms_plugin_registry_lookup_metadata_key (MsPluginRegistry *registry, MsKeyID key_id);

G_END_DECLS

#endif

