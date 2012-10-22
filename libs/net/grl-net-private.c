/*
 * Copyright (C) 2012 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Víctor M. Jáquez L. <vjaquez@igalia.com>
 *          Juan A. Suarez Romero <jasuarez@igalia.com>
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

#include "grl-net-private.h"
#include "grl-net-mock-private.h"

#include <glib/gstdio.h>
#include <errno.h>

static const char *capture_dir = NULL;

void
parse_error (guint status,
             const gchar *reason,
             const gchar *response,
             GSimpleAsyncResult *result)
{
  if (!response || *response == '\0')
    response = reason;

  switch (status) {
  case SOUP_STATUS_CANT_RESOLVE:
  case SOUP_STATUS_CANT_CONNECT:
  case SOUP_STATUS_SSL_FAILED:
  case SOUP_STATUS_IO_ERROR:
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_NETWORK_ERROR,
                                     "Cannot connect to the server");
    return;
  case SOUP_STATUS_CANT_RESOLVE_PROXY:
  case SOUP_STATUS_CANT_CONNECT_PROXY:
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_PROXY_ERROR,
                                     "Cannot connect to the proxy server");
    return;
  case SOUP_STATUS_INTERNAL_SERVER_ERROR: /* 500 */
  case SOUP_STATUS_MALFORMED:
  case SOUP_STATUS_BAD_REQUEST: /* 400 */
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_PROTOCOL_ERROR,
                                     "Invalid request URI or header: %s",
                                     response);
    return;
  case SOUP_STATUS_UNAUTHORIZED: /* 401 */
  case SOUP_STATUS_FORBIDDEN: /* 403 */
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_AUTHENTICATION_REQUIRED,
                                     "Authentication required: %s", response);
    return;
  case SOUP_STATUS_NOT_FOUND: /* 404 */
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_NOT_FOUND,
                                     "The requested resource was not found: %s",
                                     response);
    return;
  case SOUP_STATUS_CONFLICT: /* 409 */
  case SOUP_STATUS_PRECONDITION_FAILED: /* 412 */
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_CONFLICT,
                                     "The entry has been modified since it was downloaded: %s",
                                     response);
    return;
  case SOUP_STATUS_CANCELLED:
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_CANCELLED,
                                     "Operation was cancelled");
    return;
  default:
    g_message ("Unhandled status: %s", soup_status_get_phrase (status));
  }
}

void
init_dump_directory ()
{
  capture_dir = g_getenv (GRL_NET_CAPTURE_DIR_VAR);

  if (capture_dir && is_mocked ()) {
    GRL_WARNING ("Cannot capture while mocking is enabled.");
    capture_dir = NULL;
    return;
  }

  if (capture_dir && g_mkdir_with_parents (capture_dir, 0700) != 0) {
    GRL_WARNING ("Could not create capture directory \"%s\": %s",
                 capture_dir, g_strerror (errno));
    capture_dir = NULL;
    return;
  }
}

static char *
build_request_filename (const char *uri)
{
  char *hash = g_compute_checksum_for_string (G_CHECKSUM_MD5, uri, -1);

  char *filename = g_strdup_printf ("%"G_GINT64_FORMAT "-%s.data",
                                    g_get_monotonic_time (), hash);

  g_free (hash);
  return filename;
}

void
dump_data (SoupURI *uri,
           const char *buffer,
           const gsize length)
{
  if (!capture_dir)
    return;

  char *uri_string = soup_uri_to_string (uri, FALSE);

  /* Write request content to file in capture directory. */
  char *request_filename = build_request_filename (uri_string);
  char *path = g_build_filename (capture_dir, request_filename, NULL);

  GError *error = NULL;
  if (!g_file_set_contents (path, buffer, length, &error)) {
    GRL_WARNING ("Could not write contents to disk: %s", error->message);
    g_error_free (error);
  }

  g_free (path);

  /* Append record about the just written file to "grl-net-mock-data-%PID.ini"
   * in the capture directory. */
  char *filename = g_strdup_printf ("grl-net-mock-data-%u.ini", getpid());
  path = g_build_filename (capture_dir, filename, NULL);
  g_free (filename);

  FILE *stream = g_fopen (path, "at");
  g_free (path);

  if (!stream) {
    GRL_WARNING ("Could not write contents to disk: %s", g_strerror (errno));
  } else {
    if (ftell (stream) == 0)
      fprintf (stream, "[default]\nversion=%d\n\n", GRL_NET_MOCK_VERSION);

    fprintf (stream, "[%s]\ndata=%s\n\n", uri_string, request_filename);
    fclose (stream);
  }

  g_free (request_filename);
  g_free (uri_string);
}
