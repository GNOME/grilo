/*
 * Copyright (C) 2010, 2011 Igalia S.L.
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

/**
 * SECTION:grilo
 * @short_description: Metadata library supporting several services
 *
 * Grilo is a metadata retrieval library. Given a search or browse operation,
 * the library will retrieve a set of metadata related to the operation from a
 * set of on-line services.
 *
 * The Grilo library should be initialized with grl_init() before it can be used.
 * You should pass pointers to the main argc and argv variables so that Grilo can
 * process its own command line options.
 *
 * After using it, in order to close cleanly all the resources opened either by
 * the core library or the sources, call grl_deinit().
 */

#include "grilo.h"
#include "grl-metadata-key-priv.h"
#include "grl-operation-priv.h"
#include "grl-registry-priv.h"
#include "grl-log-priv.h"
#include "config.h"

#include <glib/gi18n-lib.h>

static gboolean grl_initialized = FALSE;
static const gchar *plugin_path = NULL;
static const gchar *plugin_list = NULL;

static const gchar *
get_default_plugin_dir (void)
{
#ifdef G_OS_WIN32
  static gchar *plugin_dir = NULL;
  gchar *run_directory;

  if (plugin_dir)
    return plugin_dir;

  run_directory = g_win32_get_package_installation_directory_of_module (NULL);
  plugin_dir = g_build_filename (run_directory,
                                 "lib", GRL_NAME,
                                 NULL);
  g_free (run_directory);
  return plugin_dir;
#else
  return GRL_PLUGINS_DIR;
#endif
}

static gboolean
pre_parse_hook_cb (GOptionContext  *context,
                   GOptionGroup    *group,
                   gpointer         data,
                   GError         **error)
{
  /* Initialize i18n */
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

  /* Initialize operations */
  grl_operation_init ();

  return TRUE;
}

static gboolean
post_parse_hook_cb (GOptionContext  *context,
                    GOptionGroup    *group,
                    gpointer         data,
                    GError         **error)
{
  GrlRegistry *registry;
  gchar **split_element;
  gchar **split_list;

  /* Initialize GModule */
  if (!g_module_supported ()) {
    GRL_ERROR ("GModule not supported in this system");
  }

  /* Setup core log domains */
  _grl_log_init_core_domains ();

  /* Register default metadata keys */
  registry = grl_registry_get_default ();
  grl_metadata_key_setup_system_keys (registry);

  /* Set default plugin directories */
  if (!plugin_path) {
    plugin_path = g_getenv (GRL_PLUGIN_PATH_VAR);
  }

  if (!plugin_path) {
    plugin_path = get_default_plugin_dir ();
  }

  split_list = g_strsplit (plugin_path, G_SEARCHPATH_SEPARATOR_S, 0);
  for (split_element = split_list; *split_element; split_element++) {
    grl_registry_add_directory (registry, *split_element);
  }
  g_strfreev (split_list);

  /* Restrict plugins to load */
  if (!plugin_list) {
    plugin_list = g_getenv (GRL_PLUGIN_LIST_VAR);
  }

  if (plugin_list) {
    split_list = g_strsplit (plugin_list, ":", 0);
    grl_registry_restrict_plugins (registry, split_list);
    g_strfreev (split_list);
  }

  grl_initialized = TRUE;

  return TRUE;
}

/**
 * grl_init:
 * @argc: (inout) (allow-none): number of input arguments, length of @argv
 * @argv: (inout) (element-type utf8) (array length=argc) (allow-none) (transfer none): list of arguments
 *
 * Initializes the Grilo library
 *
 * Since: 0.1.6
 */
void
grl_init (gint *argc,
          gchar **argv[])
{
  GOptionContext *ctx;
  GOptionGroup *group;

  if (grl_initialized) {
    GRL_DEBUG ("already initialized grl");
    return;
  }

  /* Check options */
  ctx = g_option_context_new ("- Grilo initialization");
  g_option_context_set_ignore_unknown_options (ctx, TRUE);
  group = grl_init_get_option_group ();
  g_option_context_add_group (ctx, group);
  g_option_context_parse (ctx, argc, argv, NULL);
  g_option_context_free (ctx);
}

/**
 * grl_deinit:
 *
 * Deinitializes the Grilo library.
 *
 * Call this function after finalizing using Grilo, in order to free and clean
 * up all the resources created.
 *
 * Since: 0.2.8
 */
void
grl_deinit (void)
{
  GrlRegistry *registry;

  if (!grl_initialized) {
    GRL_WARNING ("Grilo has not been initialized");
    return;
  }

  registry = grl_registry_get_default ();
  grl_registry_shutdown (registry);
  grl_initialized = FALSE;
}

/**
 * grl_init_get_option_group:
 *
 * Returns a #GOptionGroup with Grilo's argument specifications.
 *
 * This function is useful if you want to integrate Grilo with other
 * libraries that use the GOption commandline parser
 * (see g_option_context_add_group() ).
 *
 * Returns: a pointer to Grilo's option group. Should be dereferenced
 * after use.
 *
 * Since: 0.1.6
 */
GOptionGroup *
grl_init_get_option_group (void)
{
  GOptionGroup *group;
  static const GOptionEntry grl_args[] = {
    { "grl-plugin-path", 0, 0, G_OPTION_ARG_STRING, &plugin_path,
#ifdef G_OS_WIN32
      N_("Semicolon-separated paths containing Grilo plugins"), NULL },
#else
      N_("Colon-separated paths containing Grilo plugins"), NULL },
#endif
    { "grl-plugin-use", 0, 0, G_OPTION_ARG_STRING, &plugin_list,
      N_("Colon-separated list of Grilo plugins to use"), NULL },
    { NULL }
  };

  group = g_option_group_new ("grl",
                              _("Grilo Options"),
                              _("Show Grilo Options"),
                              NULL,
                              NULL);
  g_option_group_add_entries (group, grl_args);
  g_option_group_set_parse_hooks (group, pre_parse_hook_cb, post_parse_hook_cb);

  return group;
}
