/*
 * Copyright (C) 2010 Víctor M. Jáquez Leal <vjaquez@igalia.com>
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

#include <glib.h>
#include <grilo.h>
#include <stdbool.h>

static GList *sources;
static GList *keys;

/* #define DUMP 1 */

#if GLIB_CHECK_VERSION(2,22,0)

#include <string.h> /* for strstr */

#define CHECK_MESSAGE(domain, error_message)				\
  (g_strcmp0 (log_domain, domain) == 0 && strstr (message, error_message))

static gboolean
registry_load_error_handler (const gchar *log_domain,
                             GLogLevelFlags log_level,
                             const gchar *message,
                             gpointer user_data)
{
  if (CHECK_MESSAGE ("Grilo", "Failed to initialize plugin") ||
      CHECK_MESSAGE ("Grilo", "Configuration not provided") ||
      CHECK_MESSAGE ("Grilo", "Missing configuration") ||
      CHECK_MESSAGE ("Grilo", "Could not open plugin directory") ||
      CHECK_MESSAGE ("Grilo", "Could not read XML file")) {
    return FALSE;
  }

  return TRUE;
}

#endif

static bool
load_metadata_sources ()
{
  GrlRegistry *reg;

  reg = grl_registry_get_default ();
  if (!reg)
    return false;

#if GLIB_CHECK_VERSION(2,22,0)
  g_test_log_set_fatal_handler (registry_load_error_handler, NULL);
#endif

  if (!grl_registry_load_all_plugins (reg, NULL))
    return false;

  keys = grl_registry_get_metadata_keys (reg);
  if (!keys)
    return false;

  sources = grl_registry_get_sources (reg, false);
  if (!sources)
    return false;

  return true;
}

#if DUMP
static void
print_keys (GrlMetadataSource *source,
	    GList *keys)
{
  GList *iter;

  g_print ("%s:\n", grl_metadata_source_get_name (source));
  for (iter = keys; iter; iter = g_list_next (iter)) {
    g_print ("\t%s\n", g_param_spec_get_name (iter->data));
  }
}
#endif

enum filter_types { SUPPORTED, SLOW, WRITABLE, LAST_FILTER };

typedef GList* (*KeyFilterFunc) (GrlMetadataSource *source,
				 GList **keys,
				 gboolean return_filtered);

KeyFilterFunc key_filter[LAST_FILTER] = {
  grl_metadata_source_filter_supported,
  grl_metadata_source_filter_slow,
  grl_metadata_source_filter_writable
};

static void
test_key_filters (enum filter_types filter)
{
  GList *iter;
  unsigned int keys_num;

  keys_num = g_list_length (keys);

  for (iter = sources; iter; iter = g_list_next (iter)) {
    GList *filtered, *unfiltered;
    GrlMetadataSource *source;
    unsigned int fil_num, unfil_num;

    source = GRL_METADATA_SOURCE (iter->data);
    filtered = g_list_copy (keys);
    unfiltered = key_filter[filter] (source, &filtered, true);

#if DUMP
    print_keys (source, filtered);
#endif

    fil_num = g_list_length (filtered);
    unfil_num = g_list_length (unfiltered);

    g_list_free (filtered);
    g_list_free (unfiltered);

    g_assert_cmpuint (fil_num + unfil_num, ==, keys_num);
  }
}

static void
test_metadata_source_supported_keys (void)
{
  test_key_filters (SUPPORTED);
}

static void
test_metadata_source_slow_keys (void)
{
  test_key_filters (SLOW);
}

static void
test_metadata_source_writable_keys (void)
{
  test_key_filters (WRITABLE);
}

int
main (int argc, char **argv)
{
  /* initialize the gtester */
  g_test_init (&argc, &argv, NULL);

  /* initialize grilo */
  grl_init (&argc, &argv);

  g_assert (load_metadata_sources ());

  /* registry tests */
  g_test_add_func ("/metadata_source/filter_supported_keys",
		   test_metadata_source_supported_keys);

  g_test_add_func ("/metadata_source/filter_slow_keys",
		   test_metadata_source_slow_keys);

  g_test_add_func ("/metadata_source/filter_writable_keys",
		   test_metadata_source_writable_keys);

  return g_test_run ();
}
