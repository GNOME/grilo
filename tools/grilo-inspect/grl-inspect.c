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
static gchar **introspect_sources = NULL;
static GrlPluginRegistry *registry = NULL;

static GOptionEntry entries[] = {
  { "delay", 'd', 0,
    G_OPTION_ARG_INT, &delay,
    "Wait some seconds before showing results",
    NULL },
  { G_OPTION_REMAINING, '\0', 0,
    G_OPTION_ARG_STRING_ARRAY, &introspect_sources,
    "Sources to introspect",
    NULL },
  { NULL }
};

static void
list_all_sources ()
{
  GrlMediaPlugin **source;
  GrlMediaPlugin **sources;

  sources = grl_plugin_registry_get_sources (registry, FALSE);
  for (source = sources; *source; source++) {
    g_print ("%s:  %s\n",
             grl_media_plugin_get_id (*source),
             grl_metadata_source_get_id (GRL_METADATA_SOURCE (*source)));
  }
}

static void
print_keys (const GList *keys)
{
  while (keys) {
    g_print ("%s", GRL_METADATA_KEY_GET_NAME (keys->data));
    keys = g_list_next (keys);
    if (keys) {
      g_print (", ");
    }
  }
}

static void
introspect_source (const gchar *source_id)
{
  GrlMediaPlugin *source;
  GrlSupportedOps supported_ops;
  const gchar *value;

  source = grl_plugin_registry_lookup_source (registry, source_id);

  if (source) {
    g_print ("Plugin Details:\n");
    g_print ("  Identifier:\t\t%s\n", grl_media_plugin_get_id (source));
    g_print ("  Filename:\t\t%s\n", grl_media_plugin_get_filename (source));
    g_print ("  Type:\t\t\t%s\n",
             GRL_IS_MEDIA_SOURCE (source)? "Media Provider": "Metadata Provider");
    g_print ("  Rank:\t\t\t%d\n", grl_media_plugin_get_rank (source);
    value = grl_media_plugin_get_name (source);
    if (value) {
      g_print ("  Name:\t\t\t%s\n", value);
    }
    value = grl_media_plugin_get_description (source);
    if (value) {
      g_print ("  Description:\t\t%s\n", value);
    }
    value = grl_media_plugin_get_version (source);
    if (value) {
      g_print ("  Version:\t\t%s\n", value);
    }
    value = grl_media_plugin_get_license (source);
    if (value) {
      g_print ("  License:\t\t%s\n", value);
    }
    value = grl_media_plugin_get_author (source);
    if (value) {
      g_print ("  Author:\t\t%s\n", value);
    }
    value = grl_media_plugin_get_site (source);
    if (value) {
      g_print ("  Site:\t\t\t%s\n", value);
    }
    g_print ("\n");

    /* Print supported operations */
    supported_ops =
      grl_metadata_source_supported_operations (GRL_METADATA_SOURCE (source));
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

    /* Print supported keys */
    g_print ("Supported keys:\n");
    g_print ("  Readable Keys:\t\t");
    print_keys (grl_metadata_source_supported_keys (GRL_METADATA_SOURCE (source)));
    g_print ("\n");
    g_print ("  Writable Keys:\t\t");
    print_keys (grl_metadata_source_writable_keys (GRL_METADATA_SOURCE (source)));
    g_print ("\n");
    g_print ("\n");
  } else {
    g_printerr ("Source Not Found: %s\n\n", source_id);
  }
  g_print ("\n");
}

static gboolean
run (gpointer data)
{
  gchar **s;

  if (introspect_sources) {
    for (s = introspect_sources; *s; s++) {
      introspect_source (*s);
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

  context = g_option_context_new ("- introspect Grilo sources");
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
