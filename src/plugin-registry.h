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

#ifndef _PLUGIN_REGISTRY_H_
#define _PLUGIN_REGISTRY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include "media-source.h"
#include "metadata-key.h"

#define PLUGIN_PATH_VAR "MS_PLUGIN_PATH"

/* Macros */

#define PLUGIN_REGISTRY_TYPE (plugin_registry_get_type ())
#define PLUGIN_REGISTRY(obj)						\
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUGIN_REGISTRY_TYPE, PluginRegistry))
#define IS_PLUGIN_REGISTRY(obj)					\
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUGIN_REGISTRY_TYPE))
#define PLUGIN_REGISTRY_CLASS(klass)					\
  (G_TYPE_CHECK_CLASS_CAST((klass), PLUGIN_REGISTRY_TYPE,  PluginRegistryClass))
#define IS_PLUGIN_REGISTRY_CLASS(klass)			\
  (G_TYPE_CHECK_CLASS_TYPE((klass), PLUGIN_REGISTRY_TYPE))
#define PLUGIN_REGISTRY_GET_CLASS(obj)					\
  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLUGIN_REGISTRY_TYPE, PluginRegistryClass))

/* Plugin registration */

#define PLUGIN_REGISTER(init,deinit,id,name,desc,version,author,license,site) \
  G_MODULE_EXPORT PluginDescriptor PLUGIN_DESCRIPTOR = {		\
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

typedef struct _PluginRegistry PluginRegistry;

typedef struct _PluginInfo {
  const gchar *id;
  const gchar *name;
  const gchar *desc;
  const gchar *version; 
  const gchar *author;
  const gchar *license;
  const gchar *site;
} PluginInfo;

typedef struct _PluginDescriptor {
  PluginInfo info;
  gboolean (*plugin_init) (PluginRegistry *, const PluginInfo *);
  void (*plugin_deinit) (void);
} PluginDescriptor;

/* PluginRegistry object */

typedef struct _PluginRegistryPrivate PluginRegistryPrivate;

struct _PluginRegistry {

  GObject parent;

  /*< private >*/
  PluginRegistryPrivate *priv;
};

/* PluginRegistry class */

typedef struct _PluginRegistryClass PluginRegistryClass;

struct _PluginRegistryClass {

  GObjectClass parent_class;  
};

G_BEGIN_DECLS

GType plugin_registry_get_type (void);

PluginRegistry *plugin_registry_get_instance (void);
gboolean plugin_registry_load (PluginRegistry *registry, const gchar *path);
void plugin_registry_unload (PluginRegistry *registry, const gchar *plugin_id);
gboolean plugin_registry_load_all (PluginRegistry *registry);
gboolean plugin_registry_register_source (PluginRegistry *registry, const PluginInfo *plugin, MediaPlugin *source);
MediaPlugin *plugin_registry_lookup_source (PluginRegistry *registry, const gchar *source_id);
MediaPlugin **plugin_registry_get_sources (PluginRegistry *registry);
const MetadataKey *plugin_registry_lookup_metadata_key (PluginRegistry *registry, KeyID key_id);

G_END_DECLS

#endif

