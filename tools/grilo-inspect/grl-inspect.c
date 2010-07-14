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

static gint delay = 0;
static GMainLoop *mainloop = NULL;
static gchar **introspect_plugins = NULL;
static GrlPluginRegistry *registry = NULL;

static GOptionEntry entries[] = {
  { "delay", 'd', 0,
    G_OPTION_ARG_INT, &delay,
    "Wait some seconds before showing results",
    NULL },
  { G_OPTION_REMAINING, '\0', 0,
    G_OPTION_ARG_STRING_ARRAY, &introspect_plugins,
    "Plugins to introspect",
    NULL },
  { NULL }
};

static void
list_all_sources ()
{
  GrlMediaPlugin **plugin;
  GrlMediaPlugin **plugins;

  plugins = grl_plugin_registry_get_sources (registry, FALSE);
  for (plugin = plugins; *plugin; plugin++) {
    g_print ("%s\n", grl_media_plugin_get_id (*plugin));
  }
}

static void
introspect_plugin (const gchar *plugin_id)
{
  GrlMediaPlugin *plugin;
  GrlSupportedOps supported_ops;
  const gchar *value;

  plugin = grl_plugin_registry_lookup_source (registry, plugin_id);

  if (plugin) {
    g_print ("Plugin Details:\n");
    g_print ("  Identifier:\t\t%s\n", grl_media_plugin_get_id (plugin));
    g_print ("  Filename:\t\t%s\n", grl_media_plugin_get_filename (plugin));
    g_print ("  Type:\t\t\t%s\n",
             GRL_IS_MEDIA_SOURCE (plugin)? "Media Provider": "Metadata Provider");
    g_print ("  Rank:\t\t\t%d\n", grl_media_plugin_get_rank (plugin));

    value = grl_media_plugin_get_name (plugin);
    if (value) {
      g_print ("  Name:\t\t\t%s\n", value);
    }
    value = grl_media_plugin_get_description (plugin);
    if (value) {
      g_print ("  Description:\t\t%s\n", value);
    }
    value = grl_media_plugin_get_version (plugin);
    if (value) {
      g_print ("  Version:\t\t%s\n", value);
    }
    value = grl_media_plugin_get_license (plugin);
    if (value) {
      g_print ("  License:\t\t%s\n", value);
    }
    value = grl_media_plugin_get_author (plugin);
    if (value) {
      g_print ("  Author:\t\t%s\n", value);
    }
    value = grl_media_plugin_get_site (plugin);
    if (value) {
      g_print ("  Site:\t\t\t%s\n", value);
    }
    g_print ("\n");

    /* Print supported operations */
    supported_ops = grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (plugin));
    g_print ("Supported operations:\n");
    if (supported_ops & GRL_OP_RESOLVE) {
      g_print ("  grl_metadata_source_resolve():\tResolve Metadata\n");
    }
    if (supported_ops & GRL_OP_METADATA) {
      g_print ("  grl_media_source_metadata():\t\tRetrieve Metadata\n");
    }
    if (supported_ops & GRL_OP_BROWSE) {
      g_print ("  grl_media_source_browse():\t\tBrowse\n");
    }
    if (supported_ops & GRL_OP_SEARCH) {
      g_print ("  grl_media_source_search():\t\tSearch\n");
    }
    if (supported_ops & GRL_OP_QUERY) {
      g_print ("  grl_media_source_query():\t\tQuery\n");
    }
    if (supported_ops & GRL_OP_STORE) {
      g_print ("  grl_metadata_source_set_metadata():\tUpdate Metadata\n");
    }
    if (supported_ops & GRL_OP_STORE_PARENT) {
      g_print ("  grl_media_source_store():\t\tAdd New Media\n");
    }
    if (supported_ops & GRL_OP_REMOVE) {
      g_print ("  grl_media_source_remove():\t\tRemove Media\n");
    }
    g_print ("\n");
  } else {
    g_printerr ("Plugin Not Found: %s\n\n", plugin_id);
  }
  g_print ("\n");
}

static gboolean
run (gpointer data)
{
  gchar **p;

  if (introspect_plugins) {
    for (p = introspect_plugins; *p; p++) {
      introspect_plugin (*p);
    }
  } else {
    list_all_sources ();
  }

  g_main_loop_quit (mainloop);

  return FALSE;
}

int
main (int argc, char *argv[])
{
  GError *error = NULL;
  GOptionContext *context;

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
    g_timeout_add_seconds ((guint) delay, run, NULL);
  } else {
    g_idle_add (run, NULL);
  }

  g_main_loop_run (mainloop);
  return 0;
}
