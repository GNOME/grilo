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

/*
 * Mock network answers of webservices through predefined files. Useful for
 * offline testing of plug-ins that provide sources from webservices.
 *
 * For configuring mock answers, a simple .ini file is used. The file is split
 * into a "default" section and one section per URL.
 * [default]
 * version = 1
 * ignored-parameters = field1[,field2[,...]] or "*"
 *
 * [http://www.example.com]
 * data = content/of/response.txt
 * timeout = 500
 *
 * Explanation of [default] parameters
 * version needs to be "1"
 * ignored-parameters can be used to map urls to sections without paying
 * attention to the query parameters of this names, so that for instance
 * <http://www.example.com?q=test+query&api_key=fake> will also match
 * <http://www.example.com>. Set "*" to ignore any parameter.
 *
 * Explanation of [url] sections
 * The section title is used to map urls to response files.
 * "data" is a path to a text file containing the raw response of the websserver.
 * The path may be relative to the configuration file or an absolute path.
 * "timeout" may be used to delay the response and in seconds. The default is
 * don't delay at all.
 * If you want to provoke a "not found" error, skip the "data" parameter.
 *
 * The name of the configuration file is either "grl-mock-data.ini" which is
 * expected to be in the current directory or can be overridden by setting the
 * environment variable GRL_REQUEST_MOCK_FILE.
 *
 * An easy way to capture the responses is to run your application with the
 * environment variable GRL_WEB_CAPTURE_DIR. GrlNetWc will then write all
 * each response into a file following the pattern "<url>-timestamp". If the
 * directory does not exist yet it will be created.
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

#include "grl-net-mock.h"

#define GRL_MOCK_VERSION 1

static GKeyFile *config = NULL;
static GRegex *ignored_parameters = NULL;
static char *base_path = NULL;

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
  const char *env;
  GError *error = NULL;
  int version;
  GFile *file, *parent;

  base_path = NULL;

  config = g_key_file_new ();

  env = g_getenv ("GRL_REQUEST_MOCK_FILE");
  if (env) {
    GRL_DEBUG ("Trying to load mock file %s", env);
    g_key_file_load_from_file (config,
                               env,
                               G_KEY_FILE_NONE,
                               &error);
  }
  if (error) {
    GRL_WARNING ("Failed to load mock file %s: %s", env, error->message);
    g_error_free (error);
    error = NULL;
  }

  /* Check if we managed to load a file */
  version = g_key_file_get_integer (config, "default", "version", &error);
  if (error || version < GRL_MOCK_VERSION) {
    if (error) {
      g_error_free (error);
      error = NULL;
    } else {
      GRL_WARNING ("Unsupported mock version %d, trying default file.", version);
    }

    env = "grl-mock-data.ini";

    g_key_file_load_from_file (config,
                               env,
                               G_KEY_FILE_NONE,
                               &error);
    if (error) {
      GRL_WARNING ("Failed to load default mock file: %s", error->message);
      g_error_free (error);

      g_key_file_unref (config);
      config = NULL;
    }
  }

  if (!config) {
    return;
  }

  char **parameter_names = g_key_file_get_string_list (config, "default", "ignored-parameters", NULL, &error);
  if (error) {
    parameter_names = NULL;
    g_error_free (error);
  }

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
      GRL_WARNING ("Failed to compile ignored parameters pattern: %s", error->message);
      g_clear_error (&error);
    }
  }

  file = g_file_new_for_commandline_arg (env);
  parent = g_file_get_parent (file);
  g_object_unref (file);

  base_path = g_file_get_path (parent);
  g_object_unref (parent);
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
