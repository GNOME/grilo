/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Juan A. Suarez Romero <jasuarez@igalia.com>
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
 * SECTION:grl-config
 * @short_description: Configuration data storage
 * @see_also: #GrlData
 *
 * This class is used to store configuration settings used by plugins.
 */

#include "grl-config.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "grl-config"

static void grl_config_dispose (GObject *object);
static void grl_config_finalize (GObject *object);

G_DEFINE_TYPE (GrlConfig, grl_config, GRL_TYPE_DATA);

static void
grl_config_class_init (GrlConfigClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_config_dispose;
  gobject_class->finalize = grl_config_finalize;
}

static void
grl_config_init (GrlConfig *self)
{
}

static void
grl_config_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_config_parent_class)->dispose (object);
}

static void
grl_config_finalize (GObject *object)
{
  g_debug ("grl_config_finalize");
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_config_parent_class)->finalize (object);
}

/**
 * grl_config_new:
 *
 * Creates a new data config object.
 *
 * Returns: a newly-allocated data config.
 */
GrlConfig *
grl_config_new (void)
{
  return g_object_new (GRL_TYPE_CONFIG,
		       NULL);
}

/**
 * grl_config_new_for_plugin:
 * @plugin: plugin id for this configuration
 *
 * Creates a new data config object that will be associated with a plugin.
 *
 * Returns: a newly-allocated data config.
 */
GrlConfig *
grl_config_new_for_plugin (const gchar *plugin)
{
  GrlConfig *config = grl_config_new ();
  grl_config_set_plugin (config, plugin);

  return config;
}
