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

static void grl_media_plugin_finalize (GObject *object);

/* ================ GrlMediaPlugin GObject ================ */

G_DEFINE_ABSTRACT_TYPE (GrlMediaPlugin, grl_media_plugin, G_TYPE_OBJECT);

static void
grl_media_plugin_class_init (GrlMediaPluginClass *media_plugin_class)
{
  GObjectClass *gobject_class;
  gobject_class = G_OBJECT_CLASS (media_plugin_class);

  gobject_class->finalize = grl_media_plugin_finalize;

  g_type_class_add_private (media_plugin_class,
                            sizeof (GrlMediaPluginPrivate));
}

static void
grl_media_plugin_init (GrlMediaPlugin *plugin)
{
  plugin->priv = GRL_MEDIA_PLUGIN_GET_PRIVATE (plugin);
}

static void
grl_media_plugin_finalize (GObject *object)
{
  GrlMediaPlugin *plugin = GRL_MEDIA_PLUGIN (object);

  if (plugin->priv->info->optional_info) {
    g_hash_table_destroy (plugin->priv->info->optional_info);
  }

  G_OBJECT_CLASS (grl_media_plugin_parent_class)->finalize (object);
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
 * Returns: (transfer none): the id of the @plugin
 */
const gchar *
grl_media_plugin_get_id (GrlMediaPlugin *plugin)
{
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (plugin), NULL);

  return plugin->priv->info->id;
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

/**
 * grl_media_plugin_get_info:
 * @plugin: a plugin
 * @key: a key representing information about this plugin
 *
 * Get the information of the @plugin that is associated with the given key
 *
 * Returns: the information assigned to the given @key or NULL if there is no such information
 */
const gchar *
grl_media_plugin_get_info (GrlMediaPlugin *plugin, const gchar *key)
{
  g_return_val_if_fail (GRL_IS_MEDIA_PLUGIN (plugin), NULL);

  if (!plugin->priv->info->optional_info) {
    return NULL;
  }

  return g_hash_table_lookup (plugin->priv->info->optional_info, key);
}
