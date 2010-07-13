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

gint delay = 0;
GMainLoop *mainloop = NULL;

static GOptionEntry entries[] = {
  { "delay", 'd', 0,
    G_OPTION_ARG_INT, &delay,
    "Wait some seconds before showing results",
    NULL },
  { NULL }
};

static gboolean
list_all_sources (gpointer data)
{
  GrlMediaPlugin **plugin;
  GrlMediaPlugin **plugins;
  GrlPluginRegistry *registry;

  registry = grl_plugin_registry_get_instance ();

  plugins = grl_plugin_registry_get_sources (registry, FALSE);
  for (plugin = plugins; *plugin; plugin++) {
    g_print ("%s\n", grl_media_plugin_get_id (*plugin));
  }

  g_main_loop_quit (mainloop);

  return FALSE;
}

int
main (int argc, char *argv[])
{
  GError *error = NULL;
  GOptionContext *context;
  GrlPluginRegistry *registry;

  context = g_option_context_new ("- introspect Grilo plugins");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, grl_init_get_option_group ());
  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);

  if (error) {
    g_printerr ("Invalid arguments, %s\n", error->message);
    g_clear_error (&error);
    return -1;
  }

  grl_init (&argc, &argv);
  grl_log_init ("*:-");

  registry = grl_plugin_registry_get_instance ();
  mainloop = g_main_loop_new (NULL, FALSE);

  grl_plugin_registry_load_all (registry);

  if (delay > 0) {
    g_timeout_add_seconds ((guint) delay, list_all_sources, NULL);
  } else {
    g_idle_add (list_all_sources, NULL);
  }

  g_main_loop_run (mainloop);
  return 0;
}
