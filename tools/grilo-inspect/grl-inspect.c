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

static void
list_all_sources (void)
{
  GList *sources = NULL;
  GList *sources_iter;

  sources = grl_registry_get_sources (registry, FALSE);

  for (sources_iter = sources; sources_iter;
      sources_iter = g_list_next (sources_iter)) {
    GrlSource *source;
    GrlPlugin *plugin;

    source = GRL_SOURCE (sources_iter->data);
    plugin = grl_source_get_plugin (source);

    g_print ("%s:  %s\n",
             grl_plugin_get_id (plugin),
             grl_source_get_id (source));
  }
  g_list_free (sources);
}

static gint
compare_keys (gpointer key1, gpointer key2)
{
  const gchar *key1_name =
    grl_metadata_key_get_name (GRLPOINTER_TO_KEYID (key1));
  const gchar *key2_name =
    grl_metadata_key_get_name (GRLPOINTER_TO_KEYID (key2));

  return g_strcmp0 (key1_name, key2_name);
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
  while (keys) {
    g_print ("%s",
             GRL_METADATA_KEY_GET_NAME (GRLPOINTER_TO_KEYID (keys->data)));
    keys = g_list_next (keys);
    if (keys) {
      g_print (", ");
    }
  }
}

static void
print_version (void)
{
  g_print ("grl-inspect-" GRL_MAJORMINOR " version " VERSION "\n");
  g_print ("Grilo " VERSION "\n");
  g_print ("http://live.gnome.org/Grilo\n");
}

static void
introspect_source (const gchar *source_id)
{
  GrlPlugin *plugin;
  GrlSource *source;
  GrlSupportedOps supported_ops;
  const gchar *value;
  gchar *key;
  GList *info_keys;
  GList *info_key;

  source = grl_registry_lookup_source (registry, source_id);

  if (source) {
    plugin = grl_source_get_plugin (source);

    if (plugin) {
      g_print ("Plugin Details:\n");
      g_print ("  %-20s %s\n", "Identifier:", grl_plugin_get_id (plugin));
      g_print ("  %-20s %s\n", "Filename:",
               grl_plugin_get_filename (plugin));

      info_keys = grl_plugin_get_info_keys (plugin);
      for (info_key = info_keys; info_key; info_key = g_list_next (info_key)) {
        key = g_strdup_printf ("%s:", (gchar *) info_key->data);
        key[0] = g_ascii_toupper (key[0]);
        value = grl_plugin_get_info (plugin, info_key->data);
        g_print ("  %-20s %s\n", key, value);
        g_free (key);
      }
      g_list_free (info_keys);
      g_print ("\n");
    }

    g_print ("Source Details:\n");
    g_print ("  %-20s %s\n", "Identifier:",
             grl_source_get_id (source));
    g_print ("  %-20s %s\n", "Name:",
             grl_source_get_name (source));
    g_print ("  %-20s %s\n", "Description:",
             grl_source_get_description (source));
    g_print ("\n");

    /* Print supported operations */
    supported_ops =
      grl_source_supported_operations (source);
    g_print ("Supported operations:\n");
    if (supported_ops & GRL_OP_RESOLVE) {
      g_print ("  grl_metadata_source_resolve():\tResolve Metadata\n");
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
    if (supported_ops & GRL_OP_STORE_METADATA) {
      g_print ("  grl_metadata_source_store_metadata():\tStore Metadata\n");
    }
    if (supported_ops & GRL_OP_REMOVE) {
      g_print ("  grl_media_source_remove():\t\tRemove Media\n");
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
  GrlKeyID key;

  key = grl_registry_lookup_metadata_key (registry, key_name);

  if (key) {
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

  grl_registry_load_all_plugins (registry, NULL);

  g_timeout_add_seconds ((guint) delay, run, NULL);

  g_main_loop_run (mainloop);
  return 0;
}
