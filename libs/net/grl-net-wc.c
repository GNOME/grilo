/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Víctor M. Jáquez L. <vjaquez@igalia.com>
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

#include <grilo.h>
#include "grl-net-wc.h"

#define GRL_LOG_DOMAIN_DEFAULT wc_log_domain
GRL_LOG_DOMAIN_STATIC(wc_log_domain);

enum {
  PROP_0,
  PROP_LOG_LEVEL,
  PROP_THROTTLING,
};

#define GRL_NET_WC_GET_PRIVATE(object)			\
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                \
                               GRL_TYPE_NET_WC,		\
                               GrlNetWcPrivate))

typedef struct _RequestClosure RequestClosure;

struct _GrlNetWcPrivate {
  SoupSession *session;
  SoupLoggerLogLevel log_level;
  guint throttling;
  GTimeVal last_request;
  GQueue *pending; /* closure queue for delayed requests */
};

struct _RequestClosure {
  GrlNetWc *self;
  char *url;
  GAsyncResult *result;
  GCancellable *cancellable;
  guint source_id;
};

GQuark
grl_net_wc_error_quark (void)
{
  return g_quark_from_static_string ("grl-wc-error-quark");
}

G_DEFINE_TYPE (GrlNetWc, grl_net_wc, G_TYPE_OBJECT);

static void grl_net_wc_finalize (GObject *object);
static void grl_net_wc_set_property (GObject *object,
                                     guint propid,
                                     const GValue *value,
                                     GParamSpec *pspec);
static void grl_net_wc_get_property (GObject *object,
                                     guint propid,
                                     GValue *value,
                                     GParamSpec *pspec);

static void
grl_net_wc_class_init (GrlNetWcClass *klass)
{
  GObjectClass *g_klass;

  g_klass = G_OBJECT_CLASS (klass);
  g_klass->finalize = grl_net_wc_finalize;
  g_klass->set_property = grl_net_wc_set_property;
  g_klass->get_property = grl_net_wc_get_property;

  g_type_class_add_private (klass, sizeof (GrlNetWcPrivate));

  g_object_class_install_property (g_klass,
                                   PROP_LOG_LEVEL,
                                   g_param_spec_uint ("loglevel",
                                                      "Log level",
                                                      "Log level for HTTP connections",
                                                      0, 3, 0,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (g_klass,
                                   PROP_THROTTLING,
                                   g_param_spec_uint ("throttling",
                                                      "throttle timeout",
                                                      "Time to throttle connections",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
}

static void
grl_net_wc_init (GrlNetWc *wc)
{
  GRL_LOG_DOMAIN_INIT (wc_log_domain, "wc");

  wc->priv = GRL_NET_WC_GET_PRIVATE (wc);

  wc->priv->session = soup_session_async_new ();
  wc->priv->pending = g_queue_new ();
}

static void
grl_net_wc_finalize (GObject *object)
{
  GrlNetWc *wc;

  wc = GRL_NET_WC (object);
  grl_net_wc_flush_delayed_requests (wc);
  g_queue_free (wc->priv->pending);
  g_object_unref (wc->priv->session);

  G_OBJECT_CLASS (grl_net_wc_parent_class)->finalize (object);
}

static void
grl_net_wc_set_property (GObject *object,
                         guint propid,
                         const GValue *value,
                         GParamSpec *pspec)
{
  GrlNetWc *wc;

  wc = GRL_NET_WC (object);

  switch (propid) {
  case PROP_LOG_LEVEL:
    grl_net_wc_set_log_level (wc, g_value_get_uint (value));
    break;
  case PROP_THROTTLING:
    grl_net_wc_set_throttling (wc, g_value_get_uint (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (wc, propid, pspec);
  }
}

static void
grl_net_wc_get_property (GObject *object,
                         guint propid,
                         GValue *value,
                         GParamSpec *pspec)
{
  GrlNetWc *wc;

  wc = GRL_NET_WC (object);

  switch (propid) {
  case PROP_LOG_LEVEL:
    g_value_set_uint (value, wc->priv->log_level);
    break;
  case PROP_THROTTLING:
    g_value_set_uint (value, wc->priv->throttling);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (wc, propid, pspec);
  }
}

static inline void
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
    return;
  default:
    g_message ("Unhandled status: %s", soup_status_get_phrase (status));
  }
}

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
}

static void
message_cancel_cb (GCancellable *cancellable,
                   SoupMessage *msg)
{
  if (msg)
    soup_session_cancel_message (g_object_get_data (G_OBJECT (msg), "session"),
                                 msg, SOUP_STATUS_CANCELLED);

}

static void
get_url_now (GrlNetWc *self,
             const char *url,
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

    return;
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

static gboolean
get_url_delayed (gpointer user_data)
{
  RequestClosure *c, *d;

  c = (RequestClosure *) user_data;
  d = g_queue_pop_tail (c->self->priv->pending);

  g_assert (c == d);

  get_url_now (c->self,
               c->url,
               G_ASYNC_RESULT (c->result),
               c->cancellable);

  g_free (c->url);
  g_free (c);

  return FALSE;
}

static void
get_url (GrlNetWc *self,
         const char *url,
         GAsyncResult *result,
         GCancellable *cancellable)
{
  GTimeVal now;


  g_get_current_time (&now);
  if (now.tv_sec - self->priv->last_request.tv_sec > self->priv->throttling) {
    get_url_now (self, url, G_ASYNC_RESULT (result), cancellable);
    g_get_current_time (&self->priv->last_request);
  } else {
    RequestClosure *c;
    guint id;

    GRL_DEBUG ("delaying web request");

    /* closure */
    c = g_new (RequestClosure, 1);
    c->self = self;
    c->url = g_strdup (url);
    c->result = result;
    c->cancellable = cancellable;

    self->priv->last_request.tv_sec += self->priv->throttling;
    id = g_timeout_add_seconds (self->priv->last_request.tv_sec - now.tv_sec,
                                get_url_delayed, c);

    c->source_id = id;
    g_queue_push_head (self->priv->pending, c);
  }
}

GrlNetWc *
grl_net_wc_new (void)
{
  return g_object_new (GRL_TYPE_NET_WC, NULL);
}

void
grl_net_wc_request_async (GrlNetWc *self,
                          const char *uri,
                          GCancellable *cancellable,
                          GAsyncReadyCallback callback,
                          gpointer user_data)
{
  GSimpleAsyncResult *result;

  result = g_simple_async_result_new (G_OBJECT (self),
                                      callback,
                                      user_data,
                                      grl_net_wc_request_async);

  get_url (self, uri, G_ASYNC_RESULT (result), cancellable);
}

gboolean
grl_net_wc_request_finish (GrlNetWc *self,
                           GAsyncResult *result,
                           gchar **content,
                           gsize *length,
                           GError **error)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (result);
  SoupMessage *msg;
  gboolean ret = TRUE;

  g_warn_if_fail (g_simple_async_result_get_source_tag (res) ==
                  grl_net_wc_request_async);

  if (g_simple_async_result_propagate_error (res, error) == TRUE) {
    ret = FALSE;
    goto end_func;
  }

  msg = (SoupMessage *) g_simple_async_result_get_op_res_gpointer (res);

  if (content != NULL)
    *content = (gchar *) msg->response_body->data;

  if (length != NULL)
    *length = (gsize) msg->response_body->length;

end_func:
  g_object_unref (res);
  return ret;
}

void
grl_net_wc_set_log_level (GrlNetWc *self,
                          guint log_level)
{
  SoupLogger *logger;

  g_return_if_fail (log_level <= 3);
  g_return_if_fail (GRL_IS_NET_WC (self));

  if (self->priv->log_level == log_level)
    return;

  soup_session_remove_feature_by_type (self->priv->session, SOUP_TYPE_LOGGER);

  logger = soup_logger_new ((SoupLoggerLogLevel) log_level, -1);
  soup_session_add_feature (self->priv->session, SOUP_SESSION_FEATURE (logger));
  g_object_unref (logger);

  self->priv->log_level = (SoupLoggerLogLevel) log_level;
}

void
grl_net_wc_set_throttling (GrlNetWc *self,
                           guint throttling)
{
  g_return_if_fail (GRL_IS_NET_WC (self));

  if (throttling > 0) {
    /* max conns per host = 1 */
    g_object_set (self->priv->session,
                  SOUP_SESSION_MAX_CONNS_PER_HOST, 1, NULL);
  } else {
    /* default value */
    g_object_set (self->priv->session,
                  SOUP_SESSION_MAX_CONNS_PER_HOST, 2, NULL);
  }

  self->priv->throttling = throttling;
}

void
grl_net_wc_flush_delayed_requests (GrlNetWc *self)
{
  RequestClosure *c;

  while ((c = g_queue_pop_head (self->priv->pending))) {
    g_source_remove (c->source_id);
    g_object_unref (c->cancellable);
    g_free (c->url);
    g_free (c);
  }

  g_get_current_time (&self->priv->last_request);
}
