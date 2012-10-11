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

#include <libsoup/soup.h>

#include "grl-net-private.h"

static void
reply_cb (SoupSession *session,
          SoupMessage *msg,
          gpointer user_data)
{
  GSimpleAsyncResult *result;
  gulong cancel_signal;

  cancel_signal = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (msg),
                                                       "cancel-signal"));
  if (cancel_signal) {
    GCancellable *cancellable;

    cancellable = g_object_get_data (G_OBJECT (msg), "cancellable");
    g_signal_handler_disconnect (cancellable, cancel_signal);
  }

  result = G_SIMPLE_ASYNC_RESULT (user_data);

  if (msg->status_code != SOUP_STATUS_OK) {
    parse_error (msg->status_code,
                 msg->reason_phrase,
                 msg->response_body->data,
                 result);
  }

  g_simple_async_result_complete (result);
  g_object_unref (result);
}

static void
message_cancel_cb (GCancellable *cancellable,
                   SoupMessage *msg)
{
  if (msg)
    soup_session_cancel_message (g_object_get_data (G_OBJECT (msg), "session"),
                                 msg, SOUP_STATUS_CANCELLED);

}

void
get_url_now (GrlNetWc *self,
	     const char *url,
	     GHashTable *headers,
	     GAsyncResult *result,
	     GCancellable *cancellable)
{
  SoupMessage *msg;
  gulong cancel_signal;

  msg = soup_message_new (SOUP_METHOD_GET, url);

  if (!msg) {
    g_simple_async_result_set_error (G_SIMPLE_ASYNC_RESULT (result),
                                     GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_PROTOCOL_ERROR,
                                     "Malformed URL: %s", url);
    g_simple_async_result_complete_in_idle (G_SIMPLE_ASYNC_RESULT (result));
    g_object_unref (result);

    return;
  }

  if (headers != NULL) {
    GHashTableIter iter;
    const char *key, *value;

    g_hash_table_iter_init (&iter, headers);
    while (g_hash_table_iter_next (&iter, &key, &value)) {
      soup_message_headers_append (msg->request_headers, key, value);
    }
  }

  g_simple_async_result_set_op_res_gpointer (G_SIMPLE_ASYNC_RESULT (result),
                                             msg, NULL);

  cancel_signal = 0;
  if (cancellable) {
    g_object_set_data (G_OBJECT (msg),
                       "cancellable",
                       cancellable);
    cancel_signal = g_signal_connect (cancellable,
                                      "cancelled",
                                      G_CALLBACK (message_cancel_cb),
                                      msg);
  }

  g_object_set_data (G_OBJECT (msg),
                     "cancel-signal",
                     GUINT_TO_POINTER (cancel_signal));
  g_object_set_data_full (G_OBJECT (msg),
                          "session",
                          g_object_ref (self->priv->session),
                          g_object_unref);

  soup_session_queue_message (self->priv->session,
                              msg,
                              reply_cb,
                              result);
}

void
get_content (GrlNetWc *self,
	     void *op,
	     gchar **content,
	     gsize *length)
{
  SoupMessage *msg = op;

  if (content)
    *content = (gchar *) msg->response_body->data;

  if (length)
    *length = (gsize) msg->response_body->length;

  dump_data (soup_message_get_uri (msg),
             msg->response_body->data,
             msg->response_body->length);
}

void
init_requester (GrlNetWc *self)
{
  init_dump_directory ();
}

void
finalize_requester (GrlNetWc *self)
{
  /* noop */
}

void
cache_down (GrlNetWc *self)
{
  /* noop */
}

void
cache_up (GrlNetWc *self)
{
  GRL_INFO ("Cache not supported");
}

gboolean
cache_is_available (GrlNetWc *self)
{
  return FALSE;
}

void
cache_set_size (GrlNetWc *self, guint size)
{
  /* noop */
}

guint
cache_get_size (GrlNetWc *self)
{
  GRL_INFO ("Cache not supported");
  return 0;
}

void
free_op_res (void *op)
{
  /* noop */
}
