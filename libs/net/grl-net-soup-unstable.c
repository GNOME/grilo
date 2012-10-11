/*
 * Copyright (C) 2012 Igalia S.L.
 * Copyright (C) 2012 Canonical Ltd.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Víctor M. Jáquez L. <vjaquez@igalia.com>
 *          Juan A. Suarez Romero <jasuarez@igalia.com>
 *          Jens Georg <jensg@openismus.com>
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

/* Using the cache feature requires to use the unstable API */
#define LIBSOUP_USE_UNSTABLE_REQUEST_API

#include <libsoup/soup-cache.h>
#include <libsoup/soup-requester.h>
#include <libsoup/soup-request-http.h>

#include "grl-net-private.h"

#define GRL_LOG_DOMAIN_DEFAULT wc_log_domain
GRL_LOG_DOMAIN_EXTERN(wc_log_domain);

SoupCache *cache;
guint cache_size;

void
init_requester (GrlNetWc *self)
{
  GrlNetWcPrivate *priv = self->priv;

  priv->requester = soup_requester_new ();
  soup_session_add_feature (priv->session,
                            SOUP_SESSION_FEATURE (priv->requester));
  init_dump_directory ();
}

void
finalize_requester (GrlNetWc *self)
{
  GrlNetWcPrivate *priv = self->priv;

  cache_down (self);
  g_free (priv->previous_data);
  g_object_unref (priv->requester);
}

void
cache_down (GrlNetWc *self)
{
  GrlNetWcPrivate *priv = self->priv;
  GRL_DEBUG ("cache down: %p", cache);

  if (!cache)
    return;

  soup_session_remove_feature (priv->session, SOUP_SESSION_FEATURE (cache));
  g_object_unref (cache);
  cache = NULL;
}

void
cache_up (GrlNetWc *self)
{
  GrlNetWcPrivate *priv = self->priv;
  gchar *dir;

  GRL_DEBUG ("cache up: %p", cache);

  if (cache)
    return;

  dir = g_build_filename (g_get_user_cache_dir (),
                          g_get_prgname (),
                          "grilo",
                          NULL);
  cache = soup_cache_new (dir, SOUP_CACHE_SINGLE_USER);
  g_free (dir);

  soup_cache_set_max_size (cache, cache_size);

  soup_session_add_feature (priv->session,
                            SOUP_SESSION_FEATURE (cache));
}

gboolean
cache_is_available (GrlNetWc *self)
{
  return cache != NULL;
}

void
cache_set_size (GrlNetWc *self, guint size)
{
  if (!cache || size == cache_size)
    return;

  cache_size = size;
  soup_cache_set_max_size (cache, cache_size);
}

guint
cache_get_size (GrlNetWc *self)
{
  return cache_size;
}

struct request_res {
  SoupRequest *request;
  gchar *buffer;
  gsize length;
  gsize offset;
};

static void
read_async_cb (GObject *source,
               GAsyncResult *res,
               gpointer user_data)
{
  GSimpleAsyncResult *result = G_SIMPLE_ASYNC_RESULT (user_data);
  struct request_res *rr = g_simple_async_result_get_op_res_gpointer (result);;

  GError *error = NULL;
  gssize s = g_input_stream_read_finish (G_INPUT_STREAM (source), res, &error);

  gsize to_read;

  if (s > 0) {
    /* Continue reading */
    rr->offset += s;
    to_read = rr->length - rr->offset;

    if (!to_read) {
      /* Buffer is not enough; we need to assign more space */
      rr->length *= 2;
      rr->buffer = g_renew (gchar, rr->buffer, rr->length);
      to_read = rr->length - rr->offset;
    }

    g_input_stream_read_async (G_INPUT_STREAM (source),
                               rr->buffer + rr->offset,
                               to_read,
                               G_PRIORITY_DEFAULT,
                               NULL,
                               read_async_cb,
                               user_data);
    return;
  }

  /* Put the end of string */
  rr->buffer[rr->offset] = '\0';

  g_input_stream_close (G_INPUT_STREAM (source), NULL, NULL);

  if (error) {
    if (error->code == G_IO_ERROR_CANCELLED) {
      g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                       GRL_NET_WC_ERROR_CANCELLED,
                                       "Operation was cancelled");
    } else {
      g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                       GRL_NET_WC_ERROR_UNAVAILABLE,
                                       "Data not available");
    }

    g_error_free (error);

    g_simple_async_result_complete (result);
    g_object_unref (result);
    return;
  }

  {
    SoupMessage *msg =
      soup_request_http_get_message (SOUP_REQUEST_HTTP (rr->request));

    if (msg && msg->status_code != SOUP_STATUS_OK) {
        parse_error (msg->status_code,
                     msg->reason_phrase,
                     msg->response_body->data,
                     G_SIMPLE_ASYNC_RESULT (user_data));
        g_object_unref (msg);
    }
  }

  g_simple_async_result_complete (result);
  g_object_unref (result);
}


static void
reply_cb (GObject *source,
          GAsyncResult *res,
          gpointer user_data)
{
  GSimpleAsyncResult *result = G_SIMPLE_ASYNC_RESULT (user_data);
  struct request_res *rr = g_simple_async_result_get_op_res_gpointer (result);

  GError *error = NULL;
  GInputStream *in = soup_request_send_finish (rr->request, res, &error);

  if (error) {
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_UNAVAILABLE,
                                     "Data not available");
    g_error_free (error);

    g_simple_async_result_complete (result);
    g_object_unref (result);
    return;
  }

  rr->length = soup_request_get_content_length (rr->request) + 1;
  if (rr->length == 1)
    rr->length = 50 * 1024;

  rr->buffer = g_new (gchar, rr->length);

  g_input_stream_read_async (in,
                             rr->buffer,
                             rr->length,
                             G_PRIORITY_DEFAULT,
                             NULL,
                             read_async_cb,
                             user_data);
}

void
get_url_now (GrlNetWc *self,
             const char *url,
             GHashTable *headers,
             GAsyncResult *result,
             GCancellable *cancellable)
{
  GrlNetWcPrivate *priv = self->priv;
  struct request_res *rr = g_slice_new0 (struct request_res);

  g_simple_async_result_set_op_res_gpointer (G_SIMPLE_ASYNC_RESULT (result),
                                             rr,
                                             NULL);

  rr->request = soup_requester_request (priv->requester, url, NULL);
  if (headers != NULL) {
    SoupMessage *message;
    GHashTableIter iter;
    const char *key, *value;

    message = soup_request_http_get_message (SOUP_REQUEST_HTTP (rr->request));

    if (message) {
      g_hash_table_iter_init (&iter, headers);
      while (g_hash_table_iter_next (&iter, (gpointer *) &key, (gpointer *)&value)) {
        soup_message_headers_append (message->request_headers, key, value);
      }
      g_object_unref (message);
    }
  }

  soup_request_send_async (rr->request, cancellable, reply_cb, result);
}

void
get_content (GrlNetWc *self,
             void *op,
             gchar **content,
             gsize *length)
{
  GrlNetWcPrivate *priv = self->priv;
  struct request_res *rr = op;

  dump_data (soup_request_get_uri (rr->request),
             rr->buffer,
             rr->offset);

  if (priv->previous_data)
    g_free (priv->previous_data);

  priv->previous_data = rr->buffer;

  if (content)
    *content = self->priv->previous_data;
  else {
    g_free (rr->buffer);
    self->priv->previous_data = NULL;
    rr->buffer = NULL;
  }

  if (length)
    *length = rr->offset;
}

void
free_op_res (void *op)
{
  struct request_res *rr = op;

  g_object_unref (rr->request);
  g_slice_free (struct request_res, rr);
}
