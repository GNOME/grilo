/*
 * Copyright (C) 2010 Stefan Kost <ensonic@users.sf.net>
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

#undef G_DISABLE_ASSERT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include <grilo.h>

#define CHECK_MESSAGE(domain, error_message) \
  (g_strcmp0 (log_domain, domain) == 0 && strstr (message, error_message))

#if GLIB_CHECK_VERSION(2,22,0)
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

typedef struct {
  GrlRegistry *registry;
  GMainLoop *loop;
} RegistryFixture;

static void
registry_fixture_setup (RegistryFixture *fixture, gconstpointer data)
{
#if GLIB_CHECK_VERSION(2,22,0)
  g_test_log_set_fatal_handler (registry_load_error_handler, NULL);
#endif

  fixture->registry = grl_registry_get_default ();
  fixture->loop = g_main_loop_new (NULL, TRUE);
}

static void
registry_fixture_teardown (RegistryFixture *fixture, gconstpointer data)
{
  g_main_loop_unref(fixture->loop);
}

static void
registry_init (void)
{
  GrlRegistry *registry;

  registry = grl_registry_get_default ();
  g_assert (registry);
}

static void
registry_load (RegistryFixture *fixture, gconstpointer data)
{
  gboolean res;

  res = grl_registry_load_all_plugins (fixture->registry, NULL);
  g_assert_cmpint (res, ==, TRUE);
}

static void
registry_unregister (RegistryFixture *fixture, gconstpointer data)
{
  GList *sources = NULL;
  GList *sources_iter;
  int i;

  g_test_bug ("627207");

  sources = grl_registry_get_sources (fixture->registry, FALSE);

  for (sources_iter = sources, i = 0; sources_iter;
      sources_iter = g_list_next (sources_iter), i++) {
    GrlMediaPlugin *source = GRL_MEDIA_PLUGIN (sources_iter->data);

    grl_registry_unregister_source (fixture->registry, source, NULL);
  }
  g_list_free (sources);

  /* We expect to have loaded sources */
  g_assert_cmpint (i, !=, 0);

  sources = grl_registry_get_sources (fixture->registry, FALSE);
  for (sources_iter = sources, i = 0; sources_iter;
      sources_iter = g_list_next (sources_iter), i++)
    ;
  g_list_free (sources);

  /* After unregistering the sources, we don't expect any */
  g_assert_cmpint (i, ==, 0);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_bug_base ("http://bugs.gnome.org/%s");

  grl_init (&argc, &argv);

  /* registry tests */
  g_test_add_func ("/registry/init", registry_init);

  g_test_add ("/registry/load",
              RegistryFixture, NULL,
              registry_fixture_setup,
              registry_load,
              registry_fixture_teardown);

  g_test_add ("/registry/unregister",
              RegistryFixture, NULL,
              registry_fixture_setup,
              registry_unregister,
              registry_fixture_teardown);

  return g_test_run ();
}
