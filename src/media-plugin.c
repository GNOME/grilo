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

#include "media-plugin.h"
#include "media-plugin-priv.h"
#include "plugin-registry.h"

#include <string.h>

#define MEDIA_PLUGIN_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE((object), MEDIA_PLUGIN_TYPE, MediaPluginPrivate))

struct _MediaPluginPrivate {
  const PluginInfo *info;
};

G_DEFINE_ABSTRACT_TYPE (MediaPlugin, media_plugin, G_TYPE_OBJECT);

static void
media_plugin_class_init (MediaPluginClass *media_plugin_class)
{
  g_type_class_add_private (media_plugin_class, sizeof (MediaPluginPrivate));
}

static void
media_plugin_init (MediaPlugin *plugin)
{
  plugin->priv = MEDIA_PLUGIN_GET_PRIVATE (plugin);
  memset (plugin->priv, 0, sizeof (MediaPluginPrivate));
}

void
media_plugin_set_plugin_info (MediaPlugin *plugin, const PluginInfo *info)
{
  plugin->priv->info = info;
}

gchar *
media_plugin_get_id (MediaPlugin *plugin)
{
  gchar *r = NULL;
  if (plugin->priv->info->id) 
    r = g_strdup (plugin->priv->info->id);
  return r;
}

gchar *
media_plugin_get_name (MediaPlugin *plugin)
{
  gchar *r = NULL;
  if (plugin->priv->info->name) 
    r =g_strdup (plugin->priv->info->name);
  return r;
}

gchar *
media_plugin_get_description (MediaPlugin *plugin)
{
  gchar *r = NULL;
  if (plugin->priv->info->desc) 
    r =g_strdup (plugin->priv->info->desc);
  return r;
}

gchar *
media_plugin_get_version (MediaPlugin *plugin)
{
  gchar *r = NULL;
  if (plugin->priv->info->version) 
    r =g_strdup (plugin->priv->info->version);
  return r;
}

gchar *
media_plugin_get_license (MediaPlugin *plugin)
{
  gchar *r = NULL;
  if (plugin->priv->info->license) 
    r =g_strdup (plugin->priv->info->license);
  return r;
}

gchar *
media_plugin_get_author (MediaPlugin *plugin)
{
  gchar *r = NULL;
  if (plugin->priv->info->author) 
    r =g_strdup (plugin->priv->info->author);
  return r;
}

gchar *
media_plugin_get_site (MediaPlugin *plugin)
{
  gchar *r = NULL;
  if (plugin->priv->info->site) 
    r =g_strdup (plugin->priv->info->site);
  return r;
}

