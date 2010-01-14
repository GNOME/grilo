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

#include "ms-media-plugin.h"
#include "ms-media-plugin-priv.h"
#include "ms-plugin-registry.h"

#include <string.h>

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "ms-media-plugin"

#define MS_MEDIA_PLUGIN_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE((object), MS_TYPE_MEDIA_PLUGIN, MsMediaPluginPrivate))

struct _MsMediaPluginPrivate {
  const MsPluginInfo *info;
};

/* ================ MsMediaPlugin GObject ================ */

G_DEFINE_ABSTRACT_TYPE (MsMediaPlugin, ms_media_plugin, G_TYPE_OBJECT);

static void
ms_media_plugin_class_init (MsMediaPluginClass *media_plugin_class)
{
  g_type_class_add_private (media_plugin_class, sizeof (MsMediaPluginPrivate));
}

static void
ms_media_plugin_init (MsMediaPlugin *plugin)
{
  plugin->priv = MS_MEDIA_PLUGIN_GET_PRIVATE (plugin);
  memset (plugin->priv, 0, sizeof (MsMediaPluginPrivate));
}

/* ================ API ================ */

void
ms_media_plugin_set_plugin_info (MsMediaPlugin *plugin, const MsPluginInfo *info)
{
  g_return_if_fail (MS_IS_MEDIA_PLUGIN (plugin));
  plugin->priv->info = info;
}

gchar *
ms_media_plugin_get_id (MsMediaPlugin *plugin)
{
  g_return_val_if_fail (MS_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->id) 
    r = g_strdup (plugin->priv->info->id);
  return r;
}

gchar *
ms_media_plugin_get_name (MsMediaPlugin *plugin)
{
  g_return_val_if_fail (MS_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->name) 
    r = g_strdup (plugin->priv->info->name);
  return r;
}

gchar *
ms_media_plugin_get_description (MsMediaPlugin *plugin)
{
  g_return_val_if_fail (MS_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->desc) 
    r = g_strdup (plugin->priv->info->desc);
  return r;
}

gchar *
ms_media_plugin_get_version (MsMediaPlugin *plugin)
{
  g_return_val_if_fail (MS_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->version) 
    r = g_strdup (plugin->priv->info->version);
  return r;
}

gchar *
ms_media_plugin_get_license (MsMediaPlugin *plugin)
{
  g_return_val_if_fail (MS_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->license) 
    r = g_strdup (plugin->priv->info->license);
  return r;
}

gchar *
ms_media_plugin_get_author (MsMediaPlugin *plugin)
{
  g_return_val_if_fail (MS_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->author) 
    r = g_strdup (plugin->priv->info->author);
  return r;
}

gchar *
ms_media_plugin_get_site (MsMediaPlugin *plugin)
{
  g_return_val_if_fail (MS_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->site) 
    r = g_strdup (plugin->priv->info->site);
  return r;
}

