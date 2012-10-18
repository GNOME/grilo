/*
 * Copyright (C) 2012 Openismus GmbH
 *
 * Authors: Jens Georg <jensg@openismus.com>
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

#include <glib/gstdio.h>
#include <gio/gio.h>
#include <libsoup/soup.h>
#include <string.h>

#define _GRILO_H_INSIDE_
#include <grl-log.h>

#include "grl-net-mock-private.h"

#define GRL_MOCK_VERSION 1

static GKeyFile *config = NULL;
static GRegex *ignored_parameters = NULL;
static char *base_path = NULL;
static gboolean enable_mocking = FALSE;
static guint throttle_override = G_MAXUINT;

gboolean
is_mocked (void)
{
  return enable_mocking;
}

gboolean
override_throttling (guint *throttling)
{
  if (throttle_override == G_MAXUINT)
    return FALSE;

  *throttling = throttle_override;
  return TRUE;
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

  if (ignored_parameters) {
    SoupURI *uri = soup_uri_new (url);
    char *new_query = g_regex_replace (ignored_parameters,
                                       soup_uri_get_query (uri), -1, 0,
                                       "", 0, NULL);
    soup_uri_set_query (uri, *new_query ? new_query : NULL);
    new_url = soup_uri_to_string (uri, FALSE);
    soup_uri_free (uri);
  } else {
    new_url = g_strdup (url);
  }

  if (!config) {
    g_simple_async_result_set_error (G_SIMPLE_ASYNC_RESULT (result),
                                     GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_NETWORK_ERROR,
                                     "%s",
                                     "No mock definition found");
    g_simple_async_result_complete_in_idle (G_SIMPLE_ASYNC_RESULT (result));
    return;
  }

  data_file = g_key_file_get_value (config, new_url, "data", &error);
  if (error) {
    g_simple_async_result_set_error (G_SIMPLE_ASYNC_RESULT (result),
                                     GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_NOT_FOUND,
                                     "Could not find mock content: %s",
                                     error->message);
    g_error_free (error);
    g_simple_async_result_complete_in_idle (G_SIMPLE_ASYNC_RESULT (result));
    return;
  }
  if (data_file[0] != '/') {
    full_path = g_build_filename (base_path, data_file, NULL);
  } else {
    full_path = data_file;
    data_file = NULL;
  }

  if (g_stat (full_path, &stat_buf) < 0) {
    g_simple_async_result_set_error (G_SIMPLE_ASYNC_RESULT (result),
                                     GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_NOT_FOUND,
                                     "%s",
                                     "Could not access mock content");
    g_simple_async_result_complete_in_idle (G_SIMPLE_ASYNC_RESULT (result));
    if (data_file)
      g_free (data_file);
    if (full_path)
      g_free (full_path);
    return;
  }
  if (data_file)
    g_free (data_file);
  if (full_path)
    g_free (full_path);

  g_simple_async_result_set_op_res_gpointer (G_SIMPLE_ASYNC_RESULT (result),
                                             new_url,
                                             NULL);
  g_simple_async_result_complete_in_idle (G_SIMPLE_ASYNC_RESULT (result));
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

  if (data_file)
    g_free (data_file);

  if (full_path)
    g_free (full_path);
}

void init_mock_requester (GrlNetWc *self)
{
  char *config_filename = NULL;
  base_path = NULL;

  /* Parse environment variable. */
  {
    const char *const env = g_getenv (GRL_NET_MOCKED_VAR);

    enable_mocking = env
            && strcmp(env, "0")
            && g_ascii_strcasecmp(env, "no")
            && g_ascii_strcasecmp(env, "off")
            && g_ascii_strcasecmp(env, "false");

    if (!enable_mocking)
      return;

    char **tokens = g_strsplit (env, ":", -1);

    for (int i = 0; tokens[i]; ++i) {
      if (1 == sscanf (tokens[i], "throttle=%u", &throttle_override))
        continue;

      if (g_str_has_prefix (tokens[i], "config=")) {
        g_free (config_filename);
        config_filename = g_strdup (tokens[i] + strlen ("config="));
        continue;
      }

      GRL_WARNING ("Unknown token in \"%s\" variable: \"%s\"",
                   GRL_NET_MOCKED_VAR, tokens[i]);
    }

    g_strfreev (tokens);
  }

  /* Read configuration file. */
  if (config_filename)
    GRL_DEBUG ("Trying to load mock file \"%s\"", config_filename);
  else
    config_filename = g_strdup ("grl-net-mock-data.ini");

  GError *error = NULL;
  config = g_key_file_new ();

  g_key_file_load_from_file (config, config_filename, G_KEY_FILE_NONE, &error);

  int version = 0;

  if (error) {
    GRL_WARNING ("Failed to load mock file \"%s\": %s",
                 config_filename, error->message);
    g_clear_error (&error);
  } else {
    /* Check if we managed to load a file */
    version = g_key_file_get_integer (config, "default", "version", &error);

    if (error || version < GRL_MOCK_VERSION) {
      GRL_WARNING ("Unsupported mock version %d.", version);
      g_clear_error (&error);
    }
  }

  if (version < GRL_MOCK_VERSION) {
    g_key_file_unref (config);
    config = NULL;
    return;
  }

  char **parameter_names = g_key_file_get_string_list (config, "default",
                                                       "ignored-parameters",
                                                       NULL, &error);
  if (error) {
    parameter_names = NULL;
    g_clear_error (&error);
  }

  /* Build regular expressions for ignored query parameters. */
  if (parameter_names) {
    GString *pattern = g_string_new ("(?:^|\\&)");

    if (parameter_names[0] && strcmp(parameter_names[0], "*") == 0) {
      g_string_append (pattern, "[^=&]+");
    } else {
      g_string_append (pattern, "(?:");

      for (int i = 0; parameter_names[i]; ++i) {
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
  if (config) {
    g_key_file_unref (config);
  }

  if (base_path) {
    g_free (base_path);
  }

  if (ignored_parameters) {
    g_regex_unref (ignored_parameters);
  }
}

void free_mock_op_res (void *op)
{
  g_free (op);
}
