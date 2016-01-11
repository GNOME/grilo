/*
 * Copyright (C) 2010-2012 Igalia S.L.
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
#include <locale.h>

#include "config.h"
#include "grl-core-keys.h"

#define GRL_LOG_DOMAIN_DEFAULT grl_inspect_log_domain
GRL_LOG_DOMAIN_STATIC(grl_inspect_log_domain);

static gint delay = 1;
static GMainLoop *mainloop = NULL;
static gchar **introspect_elements = NULL;
static gchar *conffile = NULL;
static GrlRegistry *registry = NULL;
static gboolean version;
static gboolean keys;

static GOptionEntry entries[] = {
  { "delay", 'd', 0,
    G_OPTION_ARG_INT, &delay,
    "Wait some seconds before showing results (default 1 second)",
    NULL },
  { "config", 'c', 0,
    G_OPTION_ARG_STRING, &conffile,
    "Configuration file to send to sources",
    NULL },
  { "keys", 'k', 0,
    G_OPTION_ARG_NONE, &keys,
    "List available metadata keys in the system",
    NULL },
  { "version", 'V', 0,
    G_OPTION_ARG_NONE, &version,
    "Print version",
    NULL },
  { G_OPTION_REMAINING, '\0', 0,
    G_OPTION_ARG_STRING_ARRAY, &introspect_elements,
    "Elements to introspect",
    NULL },
  { NULL }
};

static gint
compare_keys (gpointer key1,
              gpointer key2)
{
  const gchar *key1_name =
    grl_metadata_key_get_name (GRLPOINTER_TO_KEYID (key1));
  const gchar *key2_name =
    grl_metadata_key_get_name (GRLPOINTER_TO_KEYID (key2));

  return g_strcmp0 (key1_name, key2_name);
}

static gint
compare_sources (gpointer key1,
                 gpointer key2)
{
  const gchar *source1_name =
    grl_source_get_id (GRL_SOURCE (key1));
  const gchar *source2_name =
    grl_source_get_id (GRL_SOURCE (key2));

  return g_strcmp0 (source1_name, source2_name);
}

static void
list_all_sources (void)
{
  GHashTable *grouped;
  GList *plugins;
  GList *plugins_iter;
  GList *sources;
  GList *sources_for_plugin;
  GList *sources_iter;
  GrlPlugin *plugin;
  GrlSource *source;
  const gchar *plugin_id;
  const gchar *source_id;

  sources = grl_registry_get_sources (registry, FALSE);

  /* Group all sources by plugin */
  grouped = g_hash_table_new (g_str_hash, g_direct_equal);

  for (sources_iter = sources; sources_iter;
      sources_iter = g_list_next (sources_iter)) {
    source = GRL_SOURCE (sources_iter->data);
    plugin = grl_source_get_plugin (source);
    source_id = grl_source_get_id (source);
    plugin_id = grl_plugin_get_id (plugin);

    sources_for_plugin = g_hash_table_lookup (grouped, plugin_id);
    sources_for_plugin = g_list_insert_sorted (sources_for_plugin,
                                               (gchar *) source_id,
                                               (GCompareFunc) g_strcmp0);
    g_hash_table_insert (grouped, (gchar *) plugin_id, sources_for_plugin);
  }

  g_list_free (sources);

  /* Sort and show */
  plugins = g_hash_table_get_keys (grouped);
  plugins = g_list_sort (plugins, (GCompareFunc) g_strcmp0);
  for (plugins_iter = plugins;
       plugins_iter;
       plugins_iter = g_list_next (plugins_iter)) {
    g_print ("%s:", (gchar *) plugins_iter->data);
    sources_for_plugin = g_hash_table_lookup (grouped, plugins_iter->data);
      for (sources_iter = sources_for_plugin;
         sources_iter;
         sources_iter = g_list_next (sources_iter)) {
      g_print ("  %s", (gchar *) sources_iter->data);
    }
    g_print ("\n");
    g_list_free (sources_for_plugin);
  }

  g_list_free (plugins);
  g_hash_table_unref (grouped);
}

static void
list_all_keys (void)
{
  GList *keys;
  GList *keys_iter;

  keys = grl_registry_get_metadata_keys (registry);

  keys = g_list_sort (keys, (GCompareFunc) compare_keys);

  for (keys_iter = keys; keys_iter;
       keys_iter = g_list_next (keys_iter)) {
    GrlKeyID key = GRLPOINTER_TO_KEYID (keys_iter->data);
    g_print ("%s:  %s\n",
             grl_metadata_key_get_name (key),
             grl_metadata_key_get_desc (key));
  }
  g_list_free (keys);
}

static void
print_keys (const GList *keys)
{
  GList *k;
  GList *sorted = NULL;

  if (!keys) {
    g_print ("---");
    return;
  }

  /* First sort the keys */
  for (k = (GList *) keys;
       k;
       k = g_list_next (k)) {
    sorted = g_list_insert_sorted (sorted, k->data, (GCompareFunc) compare_keys);
  }

  k = sorted;
  while (k) {
    g_print ("%s",
             GRL_METADATA_KEY_GET_NAME (GRLPOINTER_TO_KEYID (k->data)));
    k = g_list_next (k);
    if (k) {
      g_print (", ");
    }
  }

  g_list_free (sorted);
}

static void
print_related_keys (GrlKeyID key)
{
  const GList *partners;

  partners = grl_registry_lookup_metadata_key_relation (registry, key);
  print_keys (partners);
}

static void
print_readable_keys (GList *sources, GrlKeyID key)
{
  GList *s;
  gboolean first = TRUE;

  for (s = sources;
       s;
       s = g_list_next (s)) {
    if (g_list_find ((GList *) grl_source_supported_keys (s->data),
                     GRLKEYID_TO_POINTER (key))) {
      if (!first) {
        g_print (", ");
      } else {
        first = FALSE;
      }

      g_print ("%s", grl_source_get_id (s->data));
    }
  }

  if (first) {
    g_print ("--\n");
  } else {
    g_print ("\n");
  }
}

static void
print_slow_keys (GList *sources, GrlKeyID key)
{
  GList *s;
  gboolean first = TRUE;

  for (s = sources;
       s;
       s = g_list_next (s)) {
    if (g_list_find ((GList *) grl_source_slow_keys (s->data),
                     GRLKEYID_TO_POINTER (key))) {
      if (!first) {
        g_print (", ");
      } else {
        first = FALSE;
      }

      g_print ("%s", grl_source_get_id (s->data));
    }
  }

  if (first) {
    g_print ("--\n");
  } else {
    g_print ("\n");
  }
}

static void
print_writable_keys (GList *sources, GrlKeyID key)
{
  GList *s;
  gboolean first = TRUE;

  for (s = sources;
       s;
       s = g_list_next (s)) {
    if (g_list_find ((GList *) grl_source_writable_keys (s->data),
                     GRLKEYID_TO_POINTER (key))) {
      if (!first) {
        g_print (", ");
      } else {
        first = FALSE;
      }

      g_print ("%s", grl_source_get_id (s->data));
    }
  }

  if (first) {
    g_print ("--\n");
  } else {
    g_print ("\n");
  }
}

static void
print_version (void)
{
  g_print ("grl-inspect-" GRL_MAJORMINOR " version " VERSION "\n");
  g_print ("Grilo " VERSION "\n");
  g_print ("https://wiki.gnome.org/Projects/Grilo\n");
}

static void
introspect_source (const gchar *source_id)
{
  GrlMediaType supported_media;
  GrlPlugin *plugin;
  GrlSource *source;
  GrlSupportedOps supported_ops;
  const gchar **tags;

  source = grl_registry_lookup_source (registry, source_id);

  if (source) {
    plugin = grl_source_get_plugin (source);

    if (plugin) {
      g_print ("Plugin Details:\n");
      g_print ("  %-20s %s\n", "Identifier:", grl_plugin_get_id (plugin));
      g_print ("  %-20s %s\n", "Name:", grl_plugin_get_name (plugin));
      g_print ("  %-20s %s\n", "Description:", grl_plugin_get_description (plugin));
      g_print ("  %-20s %s\n", "Filename:", grl_plugin_get_filename (plugin));
      g_print ("  %-20s %s\n", "Author:", grl_plugin_get_author (plugin));
      g_print ("  %-20s %s\n", "Version:", grl_plugin_get_version (plugin));
      g_print ("  %-20s %s\n", "License:", grl_plugin_get_license (plugin));
      g_print ("  %-20s %s\n", "Site:", grl_plugin_get_site (plugin));
      g_print ("\n");
    }

    g_print ("Source Details:\n");
    g_print ("  %-20s %s\n", "Identifier:",
             grl_source_get_id (source));
    g_print ("  %-20s %s\n", "Name:",
             grl_source_get_name (source));
    g_print ("  %-20s %s\n", "Description:",
             grl_source_get_description (source));
    g_print ("  %-20s %d\n", "Rank:",
             grl_source_get_rank (source));

    /* Print tags */
    tags = grl_source_get_tags (source);
    if (tags) {
      g_print ("  %-20s %s", "Tags:", *tags);
      while (*(++tags)) {
        g_print (", %s", *tags);
      }
      g_print ("\n");
    }
    g_print ("\n");

   /* Print supported media */
    supported_media =
      grl_source_get_supported_media (source);
    g_print ("Supported media type:\n");
    if (supported_media & GRL_MEDIA_TYPE_AUDIO) {
      g_print ("  audio\n");
    }
    if (supported_media & GRL_MEDIA_TYPE_VIDEO) {
      g_print ("  video\n");
    }
    if (supported_media & GRL_MEDIA_TYPE_IMAGE) {
      g_print ("  image\n");
    }
    g_print ("\n");

   /* Print supported operations */
    supported_ops =
      grl_source_supported_operations (source);
    g_print ("Supported operations:\n");
    if (supported_ops & GRL_OP_RESOLVE) {
      g_print ("  grl_media_source_resolve():\t\tResolve Metadata\n");
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
      g_print ("  grl_media_source_store():\tAdd New Media (in root)\n");
    }
    if (supported_ops & GRL_OP_STORE_PARENT) {
      g_print ("  grl_media_source_store():\t\tAdd New Media\n");
    }
    if (supported_ops & GRL_OP_STORE_METADATA) {
      g_print ("  grl_media_source_store_metadata():\tUpdate Metadata\n");
    }
    if (supported_ops & GRL_OP_REMOVE) {
      g_print ("  grl_media_source_remove():\t\tRemove Media\n");
    }
    if (supported_ops & GRL_OP_MEDIA_FROM_URI) {
      g_print ("  grl_media_source_test_media_from_uri():\tTest if it can get a Media from an URI\n");
      g_print ("  grl_media_source_get_media_from_uri():\tGet a Media from an URI\n");
    }
    g_print ("\n");

    /* Print supported signals */
    g_print ("Supported signals:\n");
    if (supported_ops & GRL_OP_NOTIFY_CHANGE) {
      g_print ("  \"content-changed\" signal:\tNotify about changes\n");
    }
    g_print ("\n");

    /* Print supported keys */
    g_print ("Supported keys:\n");
    g_print ("  Readable Keys:\t");
    print_keys (grl_source_supported_keys (source));
    g_print ("\n");
    g_print ("  Slow Keys:\t\t");
    print_keys (grl_source_slow_keys (source));
    g_print ("\n");
    g_print ("  Writable Keys:\t");
    print_keys (grl_source_writable_keys (source));
    g_print ("\n");
    g_print ("\n");
  } else {
    g_printerr ("Source Not Found: %s\n\n", source_id);
  }
  g_print ("\n");
}

static void
introspect_key (const gchar *key_name)
{
  GList *sources;
  GrlKeyID key;
  guint32 key_number;

  key = grl_registry_lookup_metadata_key (registry, key_name);
  key_number = (guint32) key;

  if (key) {
    g_print ("Key Details:\n");
    g_print ("  %-20s %s\n", "Identifier:",
             (key_number < G_N_ELEMENTS (grl_core_keys))? grl_core_keys[key_number]: "--");
    g_print ("  %-20s %s\n", "Name:",
             grl_metadata_key_get_name (key));
    g_print ("  %-20s %s\n", "Description:",
             grl_metadata_key_get_desc (key));
    g_print ("  %-20s %s\n", "Type:",
             g_type_name (grl_metadata_key_get_type (key)));
    g_print ("  %-20s ", "Relations:");
    print_related_keys (key);
    g_print ("\n\n");

    sources = grl_registry_get_sources (registry, FALSE);
    sources = g_list_sort (sources, (GCompareFunc) compare_sources);

    g_print ("Sources Supporting:\n");
    g_print ("  Readable:\t");
    print_readable_keys (sources, key);
    g_print ("  Slow:\t\t");
    print_slow_keys (sources, key);
    g_print ("  Writable:\t");
    print_writable_keys (sources, key);

    g_list_free (sources);

    g_print ("\n");
    g_print ("\n");
  } else {
    g_printerr ("Metadata Key Not Found: %s\n\n", key_name);
  }
  g_print ("\n");
}

static gboolean
run (gpointer data)
{
  gchar **s;

  if (keys) {
    if (introspect_elements) {
      for (s = introspect_elements; *s; s++) {
        introspect_key (*s);
      }
    } else {
        list_all_keys ();
    }
  } else {
    if (introspect_elements) {
      for (s = introspect_elements; *s; s++) {
        introspect_source (*s);
      }
    } else {
      list_all_sources ();
    }
  }

  g_main_loop_quit (mainloop);

  return FALSE;
}

int
main (int argc, char *argv[])
{
  GError *error = NULL;
  GOptionContext *context;

  setlocale (LC_ALL, "");

  context = g_option_context_new ("- introspect Grilo elements");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, grl_init_get_option_group ());
  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);

  if (error) {
    g_printerr ("Invalid arguments, %s\n", error->message);
    g_clear_error (&error);
    return -1;
  }

  if (version) {
    print_version ();
    return 0;
  }

  grl_init (&argc, &argv);

  GRL_LOG_DOMAIN_INIT (grl_inspect_log_domain, "grl-inspect");

  registry = grl_registry_get_default ();
  if (conffile) {
    grl_registry_add_config_from_file (registry, conffile, &error);
    if (error) {
      GRL_WARNING ("Unable to load configuration: %s", error->message);
      g_error_free (error);
    }
  }

  mainloop = g_main_loop_new (NULL, FALSE);

  grl_registry_load_all_plugins (registry, TRUE, NULL);

  g_timeout_add_seconds ((guint) delay, run, NULL);

  g_main_loop_run (mainloop);

  grl_deinit ();

  return 0;
}
