/*
 * Copyright (C) 2012 Openismus GmbH
 *
 * Authors: Jens Georg <jensg@openismus.com>
 *          Mathias Hasselmann <mathias@openismus.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <string.h>

#define _GRILO_H_INSIDE_
#include <grl-log.h>

#include "grl-net-mock-private.h"

static GKeyFile *config = NULL;
static GRegex *ignored_parameters = NULL;
static char *base_path = NULL;
static gboolean enable_mocking = FALSE;
static gint refcount = 0;

gboolean
is_mocked (void)
{
  return enable_mocking;
}

void
get_url_mocked (GrlNetWc *self,
                const char *url,
                GHashTable *headers,
                GAsyncResult *result,
                GCancellable *cancellable)
{
  char *data_file, *full_path;
  GError *error = NULL;
  GStatBuf stat_buf;
  char *new_url;
  GTask *task = G_TASK (result);

  if (ignored_parameters) {
    g_autoptr(GUri) uri = g_uri_parse (url, G_URI_FLAGS_PARSE_RELAXED, NULL);
    const char *query = g_uri_get_query (uri);
    if (query) {
      g_autoptr(GUri) nuri = NULL;
      char *new_query = g_regex_replace (ignored_parameters,
                                         query, -1, 0,
                                         "", 0, NULL);
      nuri = g_uri_build (G_URI_FLAGS_NONE,
                          g_uri_get_scheme (uri),
                          NULL,
                          g_uri_get_host (uri),
                          g_uri_get_port (uri),
                          g_uri_get_path (uri),
                          *new_query ? new_query : NULL,
                          g_uri_get_fragment (uri));
      new_url = g_uri_to_string_partial (nuri, G_URI_FLAGS_NONE);
      g_free (new_query);
    } else {
      new_url = g_strdup (url);
    }
  } else {
    new_url = g_strdup (url);
  }

  if (!config) {
    g_task_return_new_error (task,
                             GRL_NET_WC_ERROR,
                             GRL_NET_WC_ERROR_NETWORK_ERROR,
                             "%s",
                             _("No mock definition found"));
    g_free (new_url);
    g_object_unref (task);
    return;
  }

  data_file = g_key_file_get_value (config, new_url, "data", &error);
  if (error) {
    g_task_return_new_error (task,
                             GRL_NET_WC_ERROR,
                             GRL_NET_WC_ERROR_NOT_FOUND,
                             _("Could not find mock content %s"),
                             error->message);
    g_error_free (error);
    g_free (new_url);
    g_object_unref (task);
    return;
  }
  if (data_file[0] != '/') {
    full_path = g_build_filename (base_path, data_file, NULL);
  } else {
    full_path = g_strdup (data_file);
  }

  if (g_stat (full_path, &stat_buf) < 0) {
    g_task_return_new_error (task,
                             GRL_NET_WC_ERROR,
                             GRL_NET_WC_ERROR_NOT_FOUND,
                             _("Could not access mock content: %s"),
                             data_file);
    g_object_unref (task);
    g_free (new_url);
    g_clear_pointer (&data_file, g_free);
    g_clear_pointer (&full_path, g_free);

    return;
  }
  g_clear_pointer (&data_file, g_free);
  g_clear_pointer (&full_path, g_free);

  g_task_return_pointer (task, new_url, NULL);
  g_object_unref (task);
}

void
get_content_mocked (GrlNetWc *self,
                    void *op,
                    gchar **content,
                    gsize *length)
{
  char *url = (char *) op;
  char *data_file = NULL, *full_path = NULL;
  GError *error = NULL;

  data_file = g_key_file_get_value (config, url, "data", NULL);
  if (data_file[0] != '/') {
    full_path = g_build_filename (base_path, data_file, NULL);
  } else {
    full_path = data_file;
    data_file = NULL;
  }
  g_file_get_contents (full_path, content, length, &error);

  g_clear_pointer (&data_file, g_free);
  g_clear_pointer (&full_path, g_free);
}

void init_mock_requester (GrlNetWc *self)
{
  g_atomic_int_inc (&refcount);

  if (refcount > 1) {
    return;
  }

  char *config_filename = g_strdup (g_getenv (GRL_NET_MOCKED_VAR));
  enable_mocking = FALSE;
  int i;

  if (config_filename == NULL) {
    return;
  }

  /* Read configuration file. */
  GError *error = NULL;
  config = g_key_file_new ();

  GRL_DEBUG ("Loading mock responses from \"%s\"", config_filename);
  g_key_file_load_from_file (config, config_filename, G_KEY_FILE_NONE, &error);

  int version = 0;

  if (error) {
    GRL_WARNING ("Failed to load mock file \"%s\": %s",
                 config_filename, error->message);
    g_clear_error (&error);
  } else {
    /* Check if we managed to load a file */
    version = g_key_file_get_integer (config, "default", "version", &error);

    if (error || version < GRL_NET_MOCK_VERSION) {
      GRL_WARNING ("Unsupported mock version.");
      g_clear_error (&error);
    } else {
      enable_mocking = TRUE;
    }
  }

  if (!enable_mocking) {
    g_free (config_filename);
    g_clear_pointer (&config, g_key_file_unref);
    return;
  }

  char **parameter_names = g_key_file_get_string_list (config, "default",
                                                       "ignored-parameters",
                                                       NULL, NULL);

  /* Build regular expressions for ignored query parameters. */
  if (parameter_names) {
    GString *pattern = g_string_new ("(?:^|\\&)");

    if (parameter_names[0] && strcmp(parameter_names[0], "*") == 0) {
      g_string_append (pattern, "[^=&]+");
    } else {
      g_string_append (pattern, "(?:");

      for (i = 0; parameter_names[i]; ++i) {
        if (i)
          g_string_append (pattern, "|");

        char *escaped = g_regex_escape_string (parameter_names[i], -1);
        g_string_append (pattern, escaped);
        g_free (escaped);
      }

      g_string_append (pattern, ")(?:=[^&]*)?");
    }

    ignored_parameters = g_regex_new (pattern->str, G_REGEX_OPTIMIZE, 0, &error);

    if (error) {
      GRL_WARNING ("Failed to compile regular expression "
                   "for ignored query parameters: %s", error->message);
      g_clear_error (&error);
    }

    g_strfreev (parameter_names);
    g_string_free (pattern, TRUE);
  }

  /* Find base path for mock data. */
  GFile *file = g_file_new_for_commandline_arg (config_filename);
  GFile *parent = g_file_get_parent (file);

  base_path = g_file_get_path (parent);

  g_object_unref (parent);
  g_object_unref (file);
  g_free (config_filename);
}

void finalize_mock_requester (GrlNetWc *self)
{
  if (refcount == 0) {
    return;
  }

  if (g_atomic_int_dec_and_test (&refcount)) {
    g_clear_pointer (&config, g_key_file_unref);
    g_clear_pointer (&base_path, g_free);
    g_clear_pointer (&ignored_parameters, g_regex_unref);
  }
}

void free_mock_op_res (void *op)
{
  g_free (op);
}
