/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Víctor M. Jáquez L. <vjaquez@igalia.com>
 *          Juan A. Suarez Romero <jasuarez@igalia.com>
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
 * SECTION:grl-net-wc
 * @short_description: small and simple HTTP client
 *
 * Most of the Grilo's sources need to access to web resources. The purpose of
 * this utility class is to provide a thin and lean mechanism for those plugins
 * to interact with those resources.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <libsoup/soup.h>

#ifdef LIBSOUP_WITH_CACHE
/* Using the cache feature requires to use the unstable API */
#define LIBSOUP_USE_UNSTABLE_REQUEST_API
#define BUFFER_SIZE (50*1024)
#include <libsoup/soup-cache.h>
#include <libsoup/soup-requester.h>
#include <libsoup/soup-request-http.h>
#endif

#include <grilo.h>
#include "grl-net-wc.h"

#define GRL_LOG_DOMAIN_DEFAULT wc_log_domain
GRL_LOG_DOMAIN_STATIC(wc_log_domain);

enum {
  PROP_0,
  PROP_LOG_LEVEL,
  PROP_THROTTLING,
  PROP_CACHE,
  PROP_CACHE_SIZE,
};

#define GRL_NET_WC_GET_PRIVATE(object)			\
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                \
                               GRL_TYPE_NET_WC,		\
                               GrlNetWcPrivate))

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
static SoupCache *cache = NULL;
static void cache_down(GrlNetWc *self);
#endif

static guint cache_size;

typedef struct _RequestClosure RequestClosure;

struct _GrlNetWcPrivate {
  SoupSession *session;
  SoupLoggerLogLevel log_level;
#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
  SoupRequester *requester;
  SoupCache *cache;
  gchar *previous_data;
#endif
  guint cache_size;
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

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
typedef struct {
  SoupRequest *request;
  gchar *buffer;
  gsize length;
  gsize offset;
} RequestResult;
#endif

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

  /**
   * GrlNetWc::loglevel
   *
   * The log level for HTTP connections. This value is used by libsoup.
   */
  g_object_class_install_property (g_klass,
                                   PROP_LOG_LEVEL,
                                   g_param_spec_uint ("loglevel",
                                                      "Log level",
                                                      "Log level for HTTP connections",
                                                      0, 3, 0,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));

  /**
   * GrlNetWc::throttling
   *
   * The timeout in seconds between connections. All the connections will be
   * queued and each one will be dispatched after waiting this value.
   */
  g_object_class_install_property (g_klass,
                                   PROP_THROTTLING,
                                   g_param_spec_uint ("throttling",
                                                      "throttle timeout",
                                                      "Time to throttle connections",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
  /**
   * GrlNetWc::cache
   *
   * %TRUE if cache must be used. %FALSE otherwise.
   */
  g_object_class_install_property (g_klass,
                                   PROP_CACHE,
                                   g_param_spec_boolean ("cache",
                                                         "Use cache",
                                                         "Use cache",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_STATIC_STRINGS));
  /**
   * GrlNetWc::cache-size
   *
   * Maximum size of cache, in Mb. Default value is 10Mb.
   */
  g_object_class_install_property (g_klass,
                                   PROP_CACHE_SIZE,
                                   g_param_spec_uint ("cache-size",
                                                      "Cache size",
                                                      "Size of cache in Mb",
                                                      0, G_MAXUINT, 10,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT |
                                                      G_PARAM_STATIC_STRINGS));
}

static void
grl_net_wc_init (GrlNetWc *wc)
{
  GRL_LOG_DOMAIN_INIT (wc_log_domain, "wc");

  wc->priv = GRL_NET_WC_GET_PRIVATE (wc);

  wc->priv->session = soup_session_async_new ();
  wc->priv->pending = g_queue_new ();
#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
  wc->priv->requester = soup_requester_new();
  soup_session_add_feature (wc->priv->session,
                            SOUP_SESSION_FEATURE (wc->priv->requester));
#endif
}

static void
grl_net_wc_finalize (GObject *object)
{
  GrlNetWc *wc;

  wc = GRL_NET_WC (object);
  grl_net_wc_flush_delayed_requests (wc);
#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
  cache_down (wc);
  g_free (wc->priv->previous_data);
  g_object_unref (wc->priv->requester);
#endif
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
  case PROP_CACHE:
    grl_net_wc_set_cache (wc, g_value_get_boolean (value));
    break;
  case PROP_CACHE_SIZE:
    grl_net_wc_set_cache_size (wc, g_value_get_uint (value));
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
  case PROP_CACHE:
#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
    g_value_set_boolean (value, wc->priv->cache != NULL);
#else
    g_value_set_boolean (value, FALSE);
#endif
    break;
  case PROP_CACHE_SIZE:
    g_value_set_uint (value, wc->priv->cache_size);
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
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_CANCELLED,
                                     "Operation was cancelled");
    return;
  default:
    g_message ("Unhandled status: %s", soup_status_get_phrase (status));
  }
}

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
static void
read_async_cb (GObject *source, GAsyncResult *res, gpointer user_data)
{
  GSimpleAsyncResult *result;
  RequestResult *rr;
  SoupMessage *msg;
  GError *error = NULL;
  gsize to_read;
  gssize s;

  result = G_SIMPLE_ASYNC_RESULT (user_data);
  rr = g_simple_async_result_get_op_res_gpointer (result);

  s = g_input_stream_read_finish (G_INPUT_STREAM (source), res, &error);
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
    g_input_stream_read_async (G_INPUT_STREAM (source), rr->buffer + rr->offset, to_read, G_PRIORITY_DEFAULT, NULL, read_async_cb, user_data);
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
    return;
  }

  msg = soup_request_http_get_message (SOUP_REQUEST_HTTP (rr->request));

  if (msg) {
    if (msg->status_code != SOUP_STATUS_OK) {
      parse_error (msg->status_code,
                   msg->reason_phrase,
                   msg->response_body->data,
                   G_SIMPLE_ASYNC_RESULT (user_data));
      g_object_unref (msg);
    }
  }

  g_simple_async_result_complete (result);
}

static void
reply_cb (GObject *source, GAsyncResult *res, gpointer user_data)
{
  GSimpleAsyncResult *result = G_SIMPLE_ASYNC_RESULT (user_data);
  RequestResult *rr = g_simple_async_result_get_op_res_gpointer (result);

  GInputStream *in = soup_request_send_finish (rr->request, res, NULL);
  rr->length = soup_request_get_content_length (rr->request) + 1;
  if (rr->length == 1) {
    rr->length = BUFFER_SIZE;
  }
  rr->buffer = g_new (gchar, rr->length);
  g_input_stream_read_async (in, rr->buffer, rr->length, G_PRIORITY_DEFAULT, NULL, read_async_cb, user_data);
}

#else

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
#endif

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
static void
get_url_now (GrlNetWc *self,
             const char *url,
             GAsyncResult *result,
             GCancellable *cancellable)
{
  RequestResult *rr = g_slice_new0 (RequestResult);

  rr->request = soup_requester_request (self->priv->requester, url, NULL);
  g_simple_async_result_set_op_res_gpointer (G_SIMPLE_ASYNC_RESULT (result),
                                             rr,
                                             NULL);
  soup_request_send_async (rr->request, cancellable, reply_cb, result);
}

#else

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
#endif

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

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
static void
update_cache_size ()
{
  guint size_in_bytes = cache_size * 1024 * 1024;
  soup_cache_set_max_size (cache, size_in_bytes);
}

static void
cache_down (GrlNetWc *self)
{
  soup_session_remove_feature (self->priv->session,
                               SOUP_SESSION_FEATURE (self->priv->cache));
  g_object_unref (self->priv->cache);
  self->priv->cache = NULL;
}

static void
cache_up (GrlNetWc *self)
{
  if (!cache) {
    gchar *cache_dir = g_build_filename (g_get_user_cache_dir (),
                                         g_get_prgname (),
                                         "grilo",
                                         NULL);
    cache = soup_cache_new (cache_dir, SOUP_CACHE_SINGLE_USER);
    update_cache_size ();
    g_free (cache_dir);
  }

  self->priv->cache = g_object_ref (cache);
  soup_session_add_feature (self->priv->session,
                            SOUP_SESSION_FEATURE (self->priv->cache));
}
#endif

/**
 * grl_net_wc_new:
 *
 * Creates a new #GrlNetWc.
 *
 * Returns: a new allocated instance of #GrlNetWc. Do g_object_unref() after
 * use it.
 */
GrlNetWc *
grl_net_wc_new ()
{
  return g_object_new (GRL_TYPE_NET_WC,
                       NULL);
}

/**
 * grl_net_wc_request_async:
 * @self: a #GrlNetWc instance
 * @uri: The URI of the resource to request
 * @cancellable: (allow-none): a #GCancellable instance or %NULL to ignore
 * @callback: The callback when the result is ready
 * @user_data: User data set for the @callback
 *
 * Request the fetching of a web resource given the @uri. This request is
 * asynchronous, thus the result will be returned within the @callback.
 */
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

/**
 * grl_net_wc_request_finish:
 * @self: a #GrlNetWc instance
 * @result: The result of the request
 * @content: The contents of the resource
 * @length: (allow-none): The length of the contents or %NULL if it is not
 * needed
 * @error: return location for a #GError, or %NULL
 *
 * Finishes an asynchronous load of the file's contents.
 * The contents are placed in contents, and length is set to the size of the
 * contents string.
 *
 * The content address will be invalidated at the next request. So if you
 * want to keep it, please copy it into another address.
 *
 * Returns: %TRUE if the request was successfull. If %FALSE an error occurred.
 */
gboolean
grl_net_wc_request_finish (GrlNetWc *self,
                           GAsyncResult *result,
                           gchar **content,
                           gsize *length,
                           GError **error)
{
  GSimpleAsyncResult *res = G_SIMPLE_ASYNC_RESULT (result);
  gboolean ret = TRUE;

  g_warn_if_fail (g_simple_async_result_get_source_tag (res) ==
                  grl_net_wc_request_async);

  if (g_simple_async_result_propagate_error (res, error) == TRUE) {
    ret = FALSE;
    goto end_func;
  }

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
  RequestResult *rr = g_simple_async_result_get_op_res_gpointer (res);

  if (self->priv->previous_data) {
    g_free (self->priv->previous_data);
  }

  self->priv->previous_data = rr->buffer;

  if (content) {
    *content = self->priv->previous_data;
  } else {
    g_free (rr->buffer);
  }

  if (length) {
    *length = rr->offset;
  }

  g_object_unref (rr->request);
  g_slice_free (RequestResult, rr);

#else
  SoupMessage *msg = (SoupMessage *) g_simple_async_result_get_op_res_gpointer (res);

  if (content != NULL)
    *content = (gchar *) msg->response_body->data;
  if (length != NULL)
    *length = (gsize) msg->response_body->length;
#endif

end_func:
  g_object_unref (res);
  return ret;
}

/**
 * grl_net_wc_set_log_level:
 * @self: a #GrlNetWc instance
 * @log_level: the libsoup log level to set [0,3]
 *
 * Setting the log level the logger feature is added into
 * the libsoup session.
 */
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

/**
 * grl_net_wc_set_throttling:
 * @self: a #GrlNetWc instance
 * @throttling: the number of seconds to wait between requests
 *
 * Setting this property, the #GrlNetWc will queue all the requests and
 * will dispatch them with a pause between them of this value.
 */
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

/**
 * grl_net_wc_set_cache:
 * @self: a #GrlNetWc instance
 * @use_cache: if cache must be used or not
 *
 * Sets if cache must be used. Note that this will only work if caching is
 * supporting.  If sets %TRUE, a new cache will be created. If sets to %FALSE,
 * current cache is clean and removed.
 *
 * Since: 0.1.12
 **/
void
grl_net_wc_set_cache (GrlNetWc *self,
                      gboolean use_cache)
{
  g_return_if_fail (GRL_IS_NET_WC (self));

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
  if (use_cache) {
    if (self->priv->cache) {
      return;
    }

    cache_up (self);

  } else {
    if (self->priv->cache) {
      cache_down (self);
    }
  }
#else
  GRL_WARNING ("Cache not supported");
#endif
}

/**
 * grl_net_wc_set_cache_size:
 * @self: a #GrlNetWc instance
 * @cache_size: size of cache (in Mb)
 *
 * Sets the new maximum size of cache, in Megabytes. Default value is 10. Using
 * 0 means no cache will be done.
 *
 * Since: 0.1.12
 **/
void
grl_net_wc_set_cache_size (GrlNetWc *self,
                           guint size)
{
  g_return_if_fail (GRL_IS_NET_WC (self));

  if (self->priv->cache_size == size) {
    return;
  }

  /* Change the global cache size */
  cache_size -= self->priv->cache_size;
  self->priv->cache_size = size;
  cache_size += self->priv->cache_size;

#ifdef LIBSOUP_USE_UNSTABLE_REQUEST_API
  if (self->priv->cache) {
    update_cache_size ();
  }
#endif
}

/**
 * grl_net_wc_flush_delayed_requests:
 * @self: a #GrlNetWc instance
 *
 * This method will flush all the pending request in the queue.
 */
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
