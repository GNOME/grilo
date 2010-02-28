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

/**
 * SECTION:grl-media-plugin
 * @short_description: Base class for Grilo Plugins
 * @see_also: #GrlMetadataSource, #GrlMediaSource
 *
 * Grilo is extensible, so #GrlMetadataSource or #GrlMediaSource instances can be
 * loaded at runtime.
 * A plugin system can provide one or more of the basic
 * <application>Grilo</application> #GrlMediaPlugin subclasses.
 *
 * This is a base class for anything that can be added to a Grilo Plugin.
 */

#include "grl-media-plugin.h"
#include "grl-media-plugin-priv.h"
#include "grl-plugin-registry.h"

#include <string.h>

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "grl-media-plugin"

#define GRL_MEDIA_PLUGIN_GET_PRIVATE(object)            \
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                \
                               GRL_TYPE_MEDIA_PLUGIN,   \
                               GrlMediaPluginPrivate))

struct _GrlMediaPluginPrivate {
  const GrlPluginInfo *info;
};

/* ================ GrlMediaPlugin GObject ================ */

G_DEFINE_ABSTRACT_TYPE (GrlMediaPlugin, grl_media_plugin, G_TYPE_OBJECT);

static void
grl_media_plugin_class_init (GrlMediaPluginClass *media_plugin_class)
{
  g_type_class_add_private (media_plugin_class,
                            sizeof (GrlMediaPluginPrivate));
}

static void
grl_media_plugin_init (GrlMediaPlugin *plugin)
{
  plugin->priv = GRL_MEDIA_PLUGIN_GET_PRIVATE (plugin);
  memset (plugin->priv, 0, sizeof (GrlMediaPluginPrivate));
}

/* ================ API ================ */

void
grl_media_plugin_set_plugin_info (GrlMediaPlugin *plugin,
                                  const GrlPluginInfo *info)
{
  g_return_if_fail (GRL_IS_MEDIA_PLUGIN (plugin));
  plugin->priv->info = info;
}

/**
 * grl_media_plugin_get_id:
 * @plugin: a plugin
 *
 * Get the id of the plugin
 *
 * Returns: (transfer full): the id of the @plugin
 */
gchar *
grl_media_plugin_get_id (GrlMediaPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->id)
    r = g_strdup (plugin->priv->info->id);
  return r;
}

/**
 * grl_media_plugin_get_name:
 * @plugin: a plugin
 *
 * Get the name of the plugin
 *
 * Returns: (transfer full): the name of the @plugin
 */
gchar *
grl_media_plugin_get_name (GrlMediaPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->name)
    r = g_strdup (plugin->priv->info->name);
  return r;
}

/**
 * grl_media_plugin_get_description:
 * @plugin: a plugin
 *
 * Get the description of the plugin
 *
 * Returns: (transfer full): the description of the @plugin
 */
gchar *
grl_media_plugin_get_description (GrlMediaPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->desc)
    r = g_strdup (plugin->priv->info->desc);
  return r;
}

/**
 * grl_media_plugin_get_version:
 * @plugin: a plugin
 *
 * Get the version of the plugin
 *
 * Returns: (transfer full): the version of the @plugin
 */
gchar *
grl_media_plugin_get_version (GrlMediaPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->version)
    r = g_strdup (plugin->priv->info->version);
  return r;
}

/**
 * grl_media_plugin_get_license:
 * @plugin: a plugin
 *
 * Get the license of the plugin
 *
 * Returns: (transfer full): the license of the @plugin
 */
gchar *
grl_media_plugin_get_license (GrlMediaPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->license)
    r = g_strdup (plugin->priv->info->license);
  return r;
}


/**
 * grl_media_plugin_get_author:
 * @plugin: a plugin
 *
 * Get the author of the plugin
 *
 * Returns: (transfer full): the author of the @plugin
 */
gchar *
grl_media_plugin_get_author (GrlMediaPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->author)
    r = g_strdup (plugin->priv->info->author);
  return r;
}

/**
 * grl_media_plugin_get_site:
 * @plugin: a plugin
 *
 * Get the site of the plugin
 *
 * Returns: (transfer full): the site of the @plugin
 */
gchar *
grl_media_plugin_get_site (GrlMediaPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (plugin), NULL);
  gchar *r = NULL;
  if (plugin->priv->info->site)
    r = g_strdup (plugin->priv->info->site);
  return r;
}

/**
 * grl_media_plugin_get_rank:
 * @plugin: a plugin
 *
 * Get the #GrlPluginRank of the plugin
 *
 * Returns: the rank of the plugin
 */
gint
grl_media_plugin_get_rank (GrlMediaPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (plugin), 0);
  return plugin->priv->info->rank;
}
