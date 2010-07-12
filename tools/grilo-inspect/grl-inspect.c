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

#include <grilo.h>
#include <glib.h>

int
main (int argc, char *argv[])
{
  GrlMediaPlugin **plugin;
  GrlMediaPlugin **plugins;
  GrlPluginRegistry *registry;

  grl_init (&argc, &argv);
  grl_log_init ("*:-");

  registry = grl_plugin_registry_get_instance ();

  grl_plugin_registry_load_all (registry);

  plugins = grl_plugin_registry_get_sources (registry, FALSE);
  for (plugin = plugins; *plugin; plugin++) {
    g_print ("%s\n", grl_media_plugin_get_id (*plugin));
  }

  g_free (plugins);

  return 0;
}
