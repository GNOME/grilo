/*
 * Copyright (C) 2014 Igalia S.L.
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

#define GRL_LOG_DOMAIN_DEFAULT grl_launch_log_domain
GRL_LOG_DOMAIN_STATIC(grl_launch_log_domain);

static GMainLoop *mainloop = NULL;
static GOptionContext *context = NULL;
static GrlMediaSerializeType serialize_type;
static GrlRegistry *registry = NULL;
static gboolean full;
static gboolean serialize;
static gboolean titles;
static gboolean version;
static gchar **operation_list = NULL;
static gchar *conffile = NULL;
static gchar *flags_parameter;
static gchar *keys_parameter;
static gint count = G_MAXINT;
static gint delay = 1;
static gint skip = 0;

static GOptionEntry entries[] = {
  { "config", 'C', 0,
    G_OPTION_ARG_STRING, &conffile,
    "Configuration file to send to sources",
    NULL },
  { "count", 'c', 0,
    G_OPTION_ARG_INT, &count,
    "Number of elements to return",
    NULL },
  { "delay", 'd', 0,
    G_OPTION_ARG_INT, &delay,
    "Wait some seconds before performing the operation (default 1 second)",
    NULL },
  { "flags", 'f', 0,
    G_OPTION_ARG_STRING, &flags_parameter,
    "List of comma-separated flags to use",
    "full|fast_only|idle_relay" },
  { "full", 'F', 0,
    G_OPTION_ARG_NONE, &full,
    "Full serialize",
    NULL },
  { "keys", 'k', 0,
    G_OPTION_ARG_STRING, &keys_parameter,
    "List of comma-separated keys to retrieve",
    NULL },
  { "serialize", 'S', 0,
    G_OPTION_ARG_NONE, &serialize,
    "Serialize",
    NULL },
  { "skip", 's', 0,
    G_OPTION_ARG_INT, &skip,
    "Number of elements to skip",
    NULL },
  { "titles", 'T', 0,
    G_OPTION_ARG_NONE, &titles,
    "Print column titles",
    NULL },
  { "version", 'V', 0,
    G_OPTION_ARG_NONE, &version,
    "Print version",
    NULL },
  { G_OPTION_REMAINING, '\0', 0,
    G_OPTION_ARG_STRING_ARRAY, &operation_list,
    "<operation> <options>",
    NULL },
  { NULL }
};

static gboolean
quit (gboolean print_help)
{
  gchar *help;

  if (print_help) {
    help = g_option_context_get_help (context, TRUE, NULL);
    g_print ("%s", help);
    g_free (help);
  }

  g_main_loop_quit (mainloop);
  return FALSE;
}

static void
print_version (void)
{
  g_print ("grl-launch-" GRL_MAJORMINOR " version " VERSION "\n");
  g_print ("Grilo " VERSION "\n");
  g_print ("https://wiki.gnome.org/Projects/Grilo\n");
}

static void
print_key (GrlMedia *media,
           GrlKeyID key)
{
  GByteArray *binary_blob;
  const GValue *value;
  gboolean has_comma;
  gchar *str_value;

  value = grl_data_get (GRL_DATA (media), key);

  if (!value) {
    return;
  }

  if (G_VALUE_HOLDS_STRING (value)) {
    str_value = (gchar *) g_value_get_string (value);
    has_comma = g_strstr_len (str_value, -1, ",") != NULL;
    if (has_comma) {
      g_print ("\"%s\"", str_value);
    } else {
      g_print ("%s", str_value);
    }
  } else if (G_VALUE_HOLDS_INT (value)) {
    g_print ("%d", g_value_get_int (value));
  } else if (G_VALUE_HOLDS_FLOAT (value)) {
    g_print ("%f", g_value_get_float (value));
  } else if (G_VALUE_HOLDS_BOOLEAN (value)) {
    g_print ("%s", g_value_get_boolean (value)? "true": "false");
  } else if (G_VALUE_TYPE (value) == G_TYPE_BYTE_ARRAY) {
    binary_blob = g_value_get_boxed (value);
    str_value = g_base64_encode (binary_blob->data, binary_blob->len);
    g_print ("%s", str_value);
    g_free (str_value);
  } else if (G_VALUE_TYPE (value) == G_TYPE_DATE_TIME) {
    str_value = g_date_time_format (g_value_get_boxed (value), "%FT%T");
    g_print ("%s", str_value);
    g_free (str_value);
  }
}

static void
print_titles (GList *keys)
{
  gboolean print_newline = FALSE;

  if (!titles) {
    return;
  }

  if (serialize || keys) {
    print_newline = TRUE;
  }

  if (serialize) {
    g_print ("media");
    if (keys) {
      g_print (",");
    }
  }

  while (keys) {
    g_print ("%s", grl_metadata_key_get_name (GRLPOINTER_TO_KEYID (keys->data)));
    keys = g_list_next (keys);
    if (keys) {
      g_print (",");
    }
  }

  if (print_newline) {
    g_print ("\n");
  }
}

static void
print_result_cb (GrlSource *source,
                 guint operation_id,
                 GrlMedia *media,
                 guint remaining,
                 gpointer user_data,
                 const GError *error)
{
  GList *k;
  GList *keys = (GList *) user_data;
  gboolean print_newline = FALSE;
  gchar *media_serial;
  static guint total_results = 0;

  if (error) {
    g_print ("Error: %s\n", error->message);
  }

  if (media) {
    if (serialize || keys) {
      print_newline = TRUE;
    }

    total_results++;
    if (serialize) {
      media_serial = grl_media_serialize_extended (media, serialize_type);
      g_print ("%s", media_serial);
      g_free (media_serial);
      if (keys) {
        g_print (",");
      }
    }
    k = keys;
    while (k) {
      print_key (media, GRLPOINTER_TO_KEYID (k->data));
      k = g_list_next (k);
      if (k) {
        g_print (",");
      }
    }

    g_object_unref (media);

    if (print_newline) {
      g_print ("\n");
    }
  }

  if (remaining == 0) {
    switch (total_results) {
    case 0:
      g_print ("No results\n");
      break;
    case 1:
      g_print ("1 result\n");
      break;
    default:
      g_print ("%u results\n", total_results);
    }
    g_list_free (keys);
    quit (FALSE);
  }
}

static void
print_single_result_cb (GrlSource *source,
                        guint operation_id,
                        GrlMedia *media,
                        gpointer user_data,
                        const GError *error)
{
  print_result_cb (source, operation_id, media, 0, user_data, error);
}

static GList *
get_keys (void)
{
  GList *keys = NULL;
  GrlKeyID key;
  GrlRegistry *registry;
  gchar **keys_array;
  gint i;

  if (!keys_parameter) {
    return NULL;
  }

  registry = grl_registry_get_default ();
  keys_array = g_strsplit (keys_parameter, ",", -1);

  for (i = 0; keys_array[i]; i++) {
    if (g_strcmp0 (keys_array[i], "*") == 0) {
      g_list_free (keys);
      g_strfreev (keys_array);
      return grl_registry_get_metadata_keys (registry);
    }

    key = grl_registry_lookup_metadata_key (registry, keys_array[i]);
    if (key == GRL_METADATA_KEY_INVALID) {
      g_print ("Unknown %s key\n", keys_array[i]);
    } else {
      keys = g_list_append (keys, GRLKEYID_TO_POINTER (key));
    }
  }

  g_strfreev (keys_array);

  return keys;
}

static GrlResolutionFlags
get_flags (void)
{
  GrlResolutionFlags flags = GRL_RESOLVE_NORMAL;
  gchar **flags_array;
  gint i;

  if (!flags_parameter) {
    return flags;
  }

  flags_array = g_strsplit (flags_parameter, ",", -1);

  for (i = 0; flags_array[i]; i++) {
    if (g_strcmp0 (flags_array[i], "full") == 0) {
      flags |= GRL_RESOLVE_FULL;
    } else if (g_strcmp0 (flags_array[i], "idle_relay") == 0) {
      flags |= GRL_RESOLVE_IDLE_RELAY;
    } else if (g_strcmp0 (flags_array[i], "fast_only") == 0) {
      flags |= GRL_RESOLVE_FAST_ONLY;
    } else {
      g_print ("Unknown %s flag\n", flags_array[i]);
    }
  }

  g_strfreev (flags_array);

  return flags;
}

static void
get_source_and_media (const gchar *str,
                      GrlSource **source,
                      GrlMedia **media)
{
  GrlRegistry *registry = grl_registry_get_default();

  /* Check if str is a source */
  *source = grl_registry_lookup_source (registry, str);
  if (*source) {
    *media = NULL;
  } else {
    /* Check then if this is a media */
    *media = grl_media_unserialize (str);
    if (*media) {
      *source = grl_registry_lookup_source (registry, grl_media_get_source (*media));
    }
  }
}

static void
content_changed_cb (GrlSource *source,
                    GPtrArray *changed_medias,
                    GrlSourceChangeType change_type,
                    gboolean location_unknown,
                    gpointer data)
{
  GrlMedia *media;
  gint i;

  for (i = 0; i < changed_medias->len; i++) {
    media = (GrlMedia *) g_ptr_array_index (changed_medias, i);
    switch (change_type) {
    case GRL_CONTENT_CHANGED:
      g_print ("changed,");
      break;
    case GRL_CONTENT_ADDED:
      g_print ("added,");
      break;
    case GRL_CONTENT_REMOVED:
      g_print ("removed,");
      break;
    }

    if (location_unknown) {
      g_print ("true");
    } else {
      g_print ("false");
    }

    if (serialize || data) {
      g_print(",");
      print_result_cb (source, 0, g_object_ref (media), -1, data, NULL);
    } else {
      g_print("\n");
    }
  }
}

static gboolean
run_search (gchar **search_params)
{
  GList *keys;
  GrlOperationOptions *options;
  GrlRegistry *registry;
  GrlSource *source;
  gchar *term;

  if (g_strv_length (search_params) != 2) {
    return quit (TRUE);
  }

  /* Empty string means "search all" */
  if (search_params[0][0] == '\0') {
    term = NULL;
  } else {
    term = search_params[0];
  }

  registry = grl_registry_get_default ();
  source = grl_registry_lookup_source (registry, search_params[1]);

  if (!source) {
    g_print ("%s is not a valid source\n", search_params[1]);
    return quit (FALSE);
  }

  if (!(grl_source_supported_operations (source) & GRL_OP_SEARCH)) {
    g_print ("%s do not support search\n", search_params[1]);
    return quit (FALSE);
  }

  keys = get_keys ();

  options = grl_operation_options_new (NULL);
  grl_operation_options_set_resolution_flags (options, get_flags ());
  grl_operation_options_set_count (options, count);
  grl_operation_options_set_skip (options, skip);

  print_titles (keys);

  grl_source_search (source, term, keys, options, print_result_cb, keys);

  g_object_unref (options);

  return FALSE;
}

static gboolean
run_browse (gchar **browse_params)
{
  GList *keys;
  GrlMedia *media;
  GrlOperationOptions *options;
  GrlSource *source;

  if (g_strv_length (browse_params) != 1) {
    return quit (TRUE);
  }

  get_source_and_media (browse_params[0], &source, &media);
  if (media && !grl_media_is_container (media)) {
    g_print ("%s is not a media container\n", browse_params[0]);
    return quit (FALSE);
  }

  if (!source) {
    g_print ("%s is not a valid source\n", browse_params[0]);
    return quit (FALSE);
  }

  if (!(grl_source_supported_operations (source)  & GRL_OP_BROWSE)) {
    g_print ("%s do not support browse\n", grl_source_get_id (source));
    return quit (FALSE);
  }

  keys = get_keys();

  options = grl_operation_options_new (NULL);
  grl_operation_options_set_resolution_flags (options, get_flags ());
  grl_operation_options_set_count (options, count);
  grl_operation_options_set_skip (options, skip);

  print_titles (keys);

  grl_source_browse (source, media, keys, options, print_result_cb, keys);

  g_object_unref (options);

  return FALSE;
}

static gboolean
run_resolve (gchar **resolve_params)
{
  GList *print_keys;
  GList *search_keys;
  GrlMedia *media;
  GrlOperationOptions *options;
  GrlRegistry *registry;
  GrlSource *source;

  if (g_strv_length (resolve_params) > 2) {
    return quit (TRUE);
  }

  get_source_and_media (resolve_params[0], &source, &media);

  if (resolve_params[1]) {
    /* Ask the other source */
    registry = grl_registry_get_default ();
    source = grl_registry_lookup_source (registry, resolve_params[1]);
  }

  if (!source) {
    g_print ("%s is not a valid source\n", resolve_params[1]? resolve_params[1]: resolve_params[0]);
    return quit (FALSE);
  }


  if (!(grl_source_supported_operations (source)  & GRL_OP_RESOLVE)) {
    g_print ("%s do not support resolve\n", grl_source_get_id (source));
    return quit (FALSE);
  }

  print_keys = get_keys();

  if (print_keys) {
    search_keys = print_keys;
  } else {
    /* Resolve requires some key to resolve; let's use "id" */
    search_keys = g_list_append (NULL, GRLKEYID_TO_POINTER (GRL_METADATA_KEY_ID));
  }

  options = grl_operation_options_new (NULL);
  grl_operation_options_set_resolution_flags (options, get_flags ());

  print_titles (print_keys);

  grl_source_resolve (source, media, search_keys, options, print_single_result_cb, print_keys);

  g_object_unref (options);

  return FALSE;
}

static gboolean
run_may_resolve (gchar **may_resolve_params)
{
  GList *required_keys = NULL;
  GrlKeyID resolve_key;
  GrlMedia *media;
  GrlRegistry *registry;
  GrlSource *source;
  gboolean may;

  if (g_strv_length (may_resolve_params) > 3) {
    return quit (TRUE);
  }

  registry = grl_registry_get_default ();
  resolve_key = grl_registry_lookup_metadata_key (registry, may_resolve_params[0]);
  if (resolve_key == GRL_METADATA_KEY_INVALID) {
    g_print ("Unknown %s key\n", may_resolve_params[0]);
    return quit (FALSE);
  }

  get_source_and_media (may_resolve_params[1], &source, &media);

  if (may_resolve_params[2]) {
    /* Ask the other source */
    registry = grl_registry_get_default ();
    source = grl_registry_lookup_source (registry, may_resolve_params[2]);
  }

  if (!source) {
    g_print ("%s is not a valid source\n", may_resolve_params[2]? may_resolve_params[2]: may_resolve_params[1]);
    return quit (FALSE);
  }


  if (!(grl_source_supported_operations (source)  & GRL_OP_RESOLVE)) {
    g_print ("%s do not support resolve\n", grl_source_get_id (source));
    return quit (FALSE);
  }

  may = grl_source_may_resolve (source, media, resolve_key, &required_keys);

  if (may) {
    g_print ("%s can resolve %s key\n", grl_source_get_id (source), may_resolve_params[0]);
  } else {
    g_print ("%s cannot resolve %s key", grl_source_get_id (source), may_resolve_params[0]);
    if (required_keys) {
      g_print (". It requires ");
      while (required_keys) {
        g_print ("%s", grl_metadata_key_get_name (GRLPOINTER_TO_KEYID (required_keys->data)));
        required_keys = g_list_next (required_keys);
        if (required_keys) {
          g_print (",");
        }
      }
    }
    g_print ("\n");
  }

  return quit (FALSE);
}


static gboolean
run_query (gchar **query_params)
{
  GList *keys;
  GrlOperationOptions *options;
  GrlRegistry *registry;
  GrlSource *source;

  if (g_strv_length (query_params) != 2) {
    return quit (TRUE);
  }

  registry = grl_registry_get_default ();
  source = grl_registry_lookup_source (registry, query_params[1]);

  if (!source) {
    g_print ("%s is not a valid source\n", query_params[1]);
    return quit (FALSE);
  }

  if (!(grl_source_supported_operations (source) & GRL_OP_QUERY)) {
    g_print ("%s do not support query\n", query_params[1]);
    return quit (FALSE);
  }

  keys = get_keys ();

  options = grl_operation_options_new (NULL);
  grl_operation_options_set_resolution_flags (options, get_flags ());
  grl_operation_options_set_count (options, count);
  grl_operation_options_set_skip (options, skip);

  print_titles (keys);

  grl_source_query (source, query_params[0], keys, options, print_result_cb, keys);

  g_object_unref (options);

  return FALSE;
}

static gboolean
run_monitor (gchar **monitor_params)
{
  GError *error = NULL;
  GList *keys;
  GrlRegistry *registry;
  GrlSource *source;

  if (g_strv_length (monitor_params) != 1) {
    return quit (TRUE);
  }

  registry = grl_registry_get_default ();
  source = grl_registry_lookup_source (registry, monitor_params[0]);

  if (!source) {
    g_print ("%s is not a valid source\n", monitor_params[0]);
    return quit (FALSE);
  }

  if (!(grl_source_supported_operations (source) & GRL_OP_NOTIFY_CHANGE)) {
    g_print ("%s do not support changes monitoring\n", monitor_params[0]);
    return quit (FALSE);
  }

  if (!grl_source_notify_change_start (source, &error)) {
    g_print ("Cannot monitor on %s: %s\n", monitor_params[0], error->message);
    g_error_free (error);
    return quit (FALSE);
  }

  keys = get_keys ();

  if (titles) {
    g_print ("change_type,location_unknown");
    if (serialize || keys) {
      g_print(",");
      print_titles (keys);
    } else {
      g_print ("\n");
    }
  }

  g_signal_connect (source,
                    "content-changed",
                    G_CALLBACK (content_changed_cb),
                    keys);

  return FALSE;
}

static gboolean
run_test_media_from_uri (gchar **test_params)
{
  GList *p;
  GList *sources;
  GrlRegistry *registry;
  GrlSource *source;
  gboolean can_do;

  if (g_strv_length (test_params) > 2) {
    return quit (TRUE);
  }

  if (!test_params[0]) {
    return quit (TRUE);
  }

  registry = grl_registry_get_default ();
  if (test_params[1]) {
    source = grl_registry_lookup_source (registry, test_params[1]);
    if (!source) {
      g_print ("%s is not a valid source\n", test_params[1]);
      return quit (FALSE);
    }
    if (!(grl_source_supported_operations (source) & GRL_OP_MEDIA_FROM_URI)) {
      g_print ("%s does not support test_media_from_uri operation\n", test_params[1]);
      return quit (FALSE);
    }
    can_do = grl_source_test_media_from_uri (source, test_params[0]);
    g_print ("%s\t%s\n", test_params[1], can_do? "yes": "no");
  } else {
    sources = grl_registry_get_sources_by_operations (registry, GRL_OP_MEDIA_FROM_URI, TRUE);
    for (p = sources; p; p = g_list_next (p)) {
      source = GRL_SOURCE (p->data);
      can_do = grl_source_test_media_from_uri (source, test_params[0]);
      if (can_do) {
        g_print ("%s\t%s\n", grl_source_get_id (source), "yes");
      }
    }
    g_list_free (sources);
  }

  return quit (FALSE);
}

static gboolean
run_media_from_uri (gchar **uri_params)
{
  GList *print_keys;
  GList *use_keys;
  GrlOperationOptions *options;
  GrlRegistry *registry;
  GrlSource *source;

  if (g_strv_length (uri_params) != 2) {
    return quit (TRUE);
  }

  registry = grl_registry_get_default ();
  source = grl_registry_lookup_source (registry, uri_params[1]);

  if (!source) {
    g_print ("%s is not a valid source\n", uri_params[1]);
    return quit (FALSE);
  }

  if (!(grl_source_supported_operations (source) & GRL_OP_MEDIA_FROM_URI)) {
    g_print ("%s does not support media_from_uri\n", uri_params[1]);
    return quit (FALSE);
  }

  print_keys = get_keys ();

  if (print_keys) {
    use_keys = print_keys;
  } else {
    /* Media_From_Uri requires some key to use; let's use "id" */
    use_keys = g_list_append (NULL, GRLKEYID_TO_POINTER (GRL_METADATA_KEY_ID));
  }

  options = grl_operation_options_new (NULL);
  grl_operation_options_set_resolution_flags (options, get_flags ());

  print_titles (print_keys);

  grl_source_get_media_from_uri (source, uri_params[0], use_keys, options, print_single_result_cb, print_keys);

  g_object_unref (options);

  return FALSE;
}

static gboolean
run (gpointer data)
{
  if (!operation_list) {
    return quit (TRUE);
  }

  if (g_strcmp0 (operation_list[0], "search") == 0) {
    return run_search (++operation_list);
  } else if (g_strcmp0 (operation_list[0], "browse") == 0) {
    return run_browse (++operation_list);
  } else if (g_strcmp0 (operation_list[0], "resolve") == 0) {
    return run_resolve (++operation_list);
  } else if (g_strcmp0 (operation_list[0], "may_resolve") == 0) {
    return run_may_resolve (++operation_list);
  } else if (g_strcmp0 (operation_list[0], "query") == 0) {
    return run_query (++operation_list);
  } else if (g_strcmp0 (operation_list[0], "monitor") == 0) {
    return run_monitor (++operation_list);
  } else if (g_strcmp0 (operation_list[0], "test_media_from_uri") == 0){
    return run_test_media_from_uri (++operation_list);
  } else if (g_strcmp0 (operation_list[0], "media_from_uri") == 0){
    return run_media_from_uri (++operation_list);
  }

  return quit (TRUE);
}

int
main (int argc, char *argv[])
{
  GError *error = NULL;

  setlocale (LC_ALL, "");

  context = g_option_context_new ("OPERATION PARAMETERS...");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, grl_init_get_option_group ());
  g_option_context_set_summary (context,
                                "\tbrowse <source>|<media container>\n"
                                "\tmay_resolve <key> <source>|<media container> [<source>]\n"
                                "\tquery <expression> <source>\n"
                                "\tresolve <source>|<media> [<source>]\n"
                                "\tsearch <term> <source>\n"
                                "\tmonitor <source>\n"
                                "\ttest_media_from_uri <uri> [<source>]\n"
                                "\tmedia_from_uri <uri> <source>");

  g_option_context_parse (context, &argc, &argv, &error);

  if (error) {
    g_printerr ("Invalid arguments, %s\n", error->message);
    g_clear_error (&error);
    return -1;
  }

  if (version) {
    print_version ();
    return 0;
  }

  serialize_type = full? GRL_MEDIA_SERIALIZE_FULL: GRL_MEDIA_SERIALIZE_BASIC;

  grl_init (&argc, &argv);

  GRL_LOG_DOMAIN_INIT (grl_launch_log_domain, "grl-launch");

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

  g_option_context_free (context);
  grl_deinit ();

  return 0;
}
