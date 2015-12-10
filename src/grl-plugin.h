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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#ifndef _GRL_PLUGIN_H_
#define _GRL_PLUGIN_H_

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include "grl-definitions.h"

/* Info */

#define GRL_PLUGIN_NAME "name"
#define GRL_PLUGIN_DESCRIPTION "description"
#define GRL_PLUGIN_VERSION "version"
#define GRL_PLUGIN_LICENSE "license"
#define GRL_PLUGIN_AUTHOR "author"
#define GRL_PLUGIN_SITE "site"

/* Macros */

#define GRL_TYPE_PLUGIN                         \
  (grl_plugin_get_type ())

#define GRL_PLUGIN(obj)                                 \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_PLUGIN,         \
                               GrlPlugin))

#define GRL_IS_PLUGIN(obj)                        \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),             \
                               GRL_TYPE_PLUGIN))

#define GRL_PLUGIN_CLASS(klass)                         \
  (G_TYPE_CHECK_CLASS_CAST((klass),                     \
                           GRL_TYPE_PLUGIN,             \
                           GrlPluginClass))

#define GRL_IS_PLUGIN_CLASS(klass)                \
  (G_TYPE_CHECK_CLASS_TYPE((klass),               \
                           GRL_TYPE_PLUGIN))

#define GRL_PLUGIN_GET_CLASS(obj)                       \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_PLUGIN,          \
                              GrlPluginClass))

/* GrlPlugin object */

typedef struct _GrlPlugin        GrlPlugin;
typedef struct _GrlPluginPrivate GrlPluginPrivate;

struct _GrlPlugin {

  GObject parent;

  /*< private >*/
  GrlPluginPrivate *priv;

  gpointer _grl_reserved[GRL_PADDING];
};

/* GrlPlugin class */

typedef struct _GrlPluginClass GrlPluginClass;

/**
 * GrlPluginClass:
 * @parent_class: the parent class structure
 */
struct _GrlPluginClass {

  GObjectClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

/* Function prototypes */

G_BEGIN_DECLS

GType grl_plugin_get_type (void);

const gchar *grl_plugin_get_name (GrlPlugin *plugin);

const gchar *grl_plugin_get_description (GrlPlugin *plugin);

const gchar *grl_plugin_get_version (GrlPlugin *plugin);

const gchar *grl_plugin_get_license (GrlPlugin *plugin);

const gchar *grl_plugin_get_author (GrlPlugin *plugin);

const gchar *grl_plugin_get_site (GrlPlugin *plugin);

const gchar *grl_plugin_get_id (GrlPlugin *plugin);

const gchar *grl_plugin_get_filename (GrlPlugin *plugin);

const gchar *grl_plugin_get_module_name (GrlPlugin *plugin);

GModule *grl_plugin_get_module (GrlPlugin *plugin);

GList *grl_plugin_get_sources (GrlPlugin *plugin);

G_END_DECLS

#endif /* _GRL_PLUGIN_H_ */
