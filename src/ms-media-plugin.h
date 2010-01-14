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

#ifndef _MS_MEDIA_PLUGIN_H_
#define _MS_MEDIA_PLUGIN_H_

#include <glib.h>
#include <glib-object.h>

/* Macros */

#define MS_TYPE_MEDIA_PLUGIN (ms_media_plugin_get_type ())
#define MS_MEDIA_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MS_TYPE_MEDIA_PLUGIN, MsMediaPlugin))
#define MS_IS_MEDIA_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MS_TYPE_MEDIA_PLUGIN))
#define MS_MEDIA_PLUGIN_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), MS_TYPE_MEDIA_PLUGIN,  MsMediaPluginClass))
#define MS_IS_MEDIA_PLUGIN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), MS_TYPE_MEDIA_PLUGIN))
#define MS_MEDIA_PLUGIN_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MS_TYPE_MEDIA_PLUGIN, MsMediaPluginClass))

/* MsMediaPlugin object */

typedef struct _MsMediaPlugin MsMediaPlugin;
typedef struct _MsMediaPluginPrivate MsMediaPluginPrivate;

struct _MsMediaPlugin {

  GObject parent;

  /*< private >*/
  MsMediaPluginPrivate *priv;
};

/* MsMediaPlugin class */

typedef struct _MsMediaPluginClass MsMediaPluginClass;

struct _MsMediaPluginClass {

  GObjectClass parent_class;

};

/* Function prototypes */

G_BEGIN_DECLS

GType ms_media_plugin_get_type (void);

gchar *ms_media_plugin_get_id (MsMediaPlugin *plugin);
gchar *ms_media_plugin_get_name (MsMediaPlugin *plugin);
gchar *ms_media_plugin_get_description (MsMediaPlugin *plugin);
gchar *ms_media_plugin_get_version (MsMediaPlugin *plugin);
gchar *ms_media_plugin_get_license (MsMediaPlugin *plugin);
gchar *ms_media_plugin_get_author (MsMediaPlugin *plugin);
gchar *ms_media_plugin_get_site (MsMediaPlugin *plugin);
 
G_END_DECLS

#endif
