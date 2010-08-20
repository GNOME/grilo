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

#include "grilo.h"
#include "grl-metadata-key-priv.h"
#include "grl-log-priv.h"
#include "config.h"

#define GRL_PLUGIN_PATH_DEFAULT GRL_PLUGINS_DIR

static gboolean grl_initialized = FALSE;
static const gchar *plugin_path = NULL;

/**
 * grl_init:
 * @argc: (in): number of input arguments, length of @argv
 * @argv: (element-type utf8) (array length=argc) (allow-none): list of arguments
 *
 * Initializes the Grilo library
 */
void
grl_init (gint *argc,
          gchar **argv[])
{
  GOptionContext *ctx;
  GOptionGroup *group;
  GrlPluginRegistry *registry;
  gchar **plugin_dir;
  gchar **plugin_dirs_split;

  if (grl_initialized) {
    GRL_DEBUG ("already initialized grl");
    return;
  }

  g_type_init ();

  /* Check options */
  ctx = g_option_context_new ("- Grilo initialization");
  g_option_context_set_ignore_unknown_options (ctx, TRUE);
  group = grl_init_get_option_group ();
  g_option_context_add_group (ctx, group);
  g_option_context_parse (ctx, argc, argv, NULL);
  g_option_context_free (ctx);

  /* Initialize GModule */
  if (!g_module_supported ()) {
    GRL_ERROR ("GModule not supported in this system");
  }

  /* Setup core log domains */
  _grl_log_init_core_domains ();

  /* Register default metadata keys */
  registry = grl_plugin_registry_get_default ();
  grl_metadata_key_setup_system_keys (registry);

  /* Register GrlMedia in glib typesystem */
  g_type_class_ref (GRL_TYPE_MEDIA_BOX);
  g_type_class_ref (GRL_TYPE_MEDIA_AUDIO);
  g_type_class_ref (GRL_TYPE_MEDIA_VIDEO);
  g_type_class_ref (GRL_TYPE_MEDIA_IMAGE);

  /* Set default plugin directories */
  if (!plugin_path) {
    plugin_path = g_getenv (GRL_PLUGIN_PATH_VAR);
  }

  if (!plugin_path) {
    plugin_path = GRL_PLUGIN_PATH_DEFAULT;
  }

  plugin_dirs_split = g_strsplit (plugin_path, ":", 0);
  for (plugin_dir = plugin_dirs_split; *plugin_dir; plugin_dir++) {
    grl_plugin_registry_add_directory (registry, *plugin_dir);
  }
  g_strfreev (plugin_dirs_split);

  grl_initialized = TRUE;
}

/**
 * grl_init_get_option_group:
 *
 * Returns a GOptionGroup with Grilo's argument specifications.
 *
 * This function is useful if you want to integrate Grilo with other
 * libraries that use #GOption (see g_option_context_add_group() ).
 *
 * Returns: a pointer to Grilo's option group. Should be dereferenced
 * after use.
 */
GOptionGroup *
grl_init_get_option_group (void)
{
  GOptionGroup *group;
  static const GOptionEntry grl_args[] = {
    { "grl-plugin-path", 0, 0, G_OPTION_ARG_STRING, &plugin_path,
      "Colon-separated paths containing plugins", NULL },
    { NULL }
  };

  group = g_option_group_new ("grl",
                              "Grilo Options:",
                              "Show Grilo Options",
                              NULL,
                              NULL);
  g_option_group_add_entries (group, grl_args);

  return group;
}
