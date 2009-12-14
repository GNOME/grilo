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

#ifndef _MEDIA_PLUGIN_H_
#define _MEDIA_PLUGIN_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <glib-object.h>

/* Macros */

#define MEDIA_PLUGIN_TYPE (media_plugin_get_type ())
#define MEDIA_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MEDIA_PLUGIN_TYPE, MediaPlugin))
#define IS_MEDIA_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MEDIA_PLUGIN_TYPE))
#define MEDIA_PLUGIN_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), MEDIA_PLUGIN_TYPE,  MediaPluginClass))
#define IS_MEDIA_PLUGIN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), MEDIA_PLUGIN_TYPE))
#define MEDIA_PLUGIN_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), MEDIA_PLUGIN_TYPE, MediaPluginClass))

/* MediaPlugin object */

typedef struct _MediaPlugin MediaPlugin;
typedef struct _MediaPluginPrivate MediaPluginPrivate;

struct _MediaPlugin {

  GObject parent;

  /*< private >*/
  MediaPluginPrivate *priv;
};

/* MediaPlugin class */

typedef struct _MediaPluginClass MediaPluginClass;

struct _MediaPluginClass {

  GObjectClass parent_class;

};

/* Function prototypes */

G_BEGIN_DECLS

GType media_plugin_get_type (void);

gchar *media_plugin_get_id (MediaPlugin *plugin);
gchar *media_plugin_get_name (MediaPlugin *plugin);
gchar *media_plugin_get_description (MediaPlugin *plugin);
gchar *media_plugin_get_version (MediaPlugin *plugin);
gchar *media_plugin_get_license (MediaPlugin *plugin);
gchar *media_plugin_get_author (MediaPlugin *plugin);
gchar *media_plugin_get_site (MediaPlugin *plugin);
 
G_END_DECLS

#endif
