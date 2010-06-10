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

#include "grilo.h"
#include "grl-metadata-key-priv.h"

static gboolean grl_initialized = FALSE;

void
grl_init (gint *argc,
          gchar **argv[])
{
  GrlPluginRegistry *registry;

  if (grl_initialized) {
    g_debug ("already initialized grl");
    return;
  }

  /* Register default metadata keys */
  registry = grl_plugin_registry_get_instance ();
  grl_metadata_key_setup_system_keys (registry);

  /* Register GrlMedia in glib typesystem */
  g_type_class_ref (GRL_TYPE_MEDIA_BOX);
  g_type_class_ref (GRL_TYPE_MEDIA_AUDIO);
  g_type_class_ref (GRL_TYPE_MEDIA_VIDEO);
  g_type_class_ref (GRL_TYPE_MEDIA_IMAGE);

  grl_initialized = TRUE;
}
