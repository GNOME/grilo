/*
 * Copyright (C) 2010, 2011 Igalia S.L.
 * Copyright (C) 2012 Canonical Ltd.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Víctor M. Jáquez L. <vjaquez@igalia.com>
 *          Juan A. Suarez Romero <jasuarez@igalia.com>
 *          Jens Georg <jensg@openismus.com>
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

#include <grilo.h>
#include "grl-net-wc.h"
#include "grl-net-private.h"
#include "grl-net-mock-private.h"

GRL_LOG_DOMAIN(wc_log_domain);

enum {
  PROP_0,
  PROP_LOG_LEVEL,
  PROP_THROTTLING,
  PROP_CACHE,
  PROP_CACHE_SIZE,
  PROP_USER_AGENT
};

#define GRL_NET_WC_GET_PRIVATE(object)			\
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                \
                               GRL_TYPE_NET_WC,		\
                               GrlNetWcPrivate))

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
   * GrlNetWc::loglevel:
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
   * GrlNetWc::throttling:
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
   * GrlNetWc::cache:
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
   * GrlNetWc::cache-size:
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
  /**
   * GrlNetWc::user-agent:
   *
   * User agent identifier.
   */
  g_object_class_install_property (g_klass,
                                   PROP_USER_AGENT,
                                   g_param_spec_string ("user-agent",
                                                        "User Agent",
                                                        "User agent identifier",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
}

/*
 * use-thread-context is available for libsoup-2.4 >= 2.39.0
 * We check in run-time if it's available
 */
static void
set_thread_context (GrlNetWc *self)
{
    GrlNetWcPrivate *priv = self->priv;
    GObjectClass *klass = G_OBJECT_GET_CLASS (priv->session);
    GParamSpec *spec = g_object_class_find_property (klass,
                                                     "use-thread-context");
    if (spec)
      g_object_set (priv->session, "use-thread-context", TRUE, NULL);
}

static void
grl_net_wc_init (GrlNetWc *wc)
{
  GRL_LOG_DOMAIN_INIT (wc_log_domain, "wc");

  wc->priv = GRL_NET_WC_GET_PRIVATE (wc);

  wc->priv->session = soup_session_async_new ();
  wc->priv->pending = g_queue_new ();

  set_thread_context (wc);
  init_mock_requester (wc);
  init_requester (wc);
}

static void
grl_net_wc_finalize (GObject *object)
{
  GrlNetWc *wc;

  wc = GRL_NET_WC (object);
  grl_net_wc_flush_delayed_requests (wc);

  cache_down (wc);
  finalize_requester (wc);
  finalize_mock_requester (wc);

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
  case PROP_USER_AGENT:
    g_object_set (G_OBJECT (wc->priv->session),
                  "user-agent", g_value_get_string (value),
                  NULL);
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
    g_value_set_boolean(value, cache_is_available (wc));
    break;
  case PROP_CACHE_SIZE:
    g_value_set_uint (value, cache_get_size (wc));
    break;
  case PROP_USER_AGENT:
    g_object_get_property (G_OBJECT (wc->priv->session), "user_agent", value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (wc, propid, pspec);
  }
}

struct request_clos {
  GrlNetWc *self;
  char *url;
  GAsyncResult *result;
  GCancellable *cancellable;
  GHashTable *headers;
  guint source_id;
};

static void
request_clos_destroy (gpointer data)
{
  struct request_clos *c = (struct request_clos *) data;

  g_free (c->url);
  if (c->headers) {
    g_hash_table_unref (c->headers);
  }
  g_free (c);
}

static gboolean
get_url_cb (gpointer user_data)
{
  struct request_clos *c = (struct request_clos *) user_data;

  /* validation */
  {
    GrlNetWcPrivate *priv = c->self->priv;
    struct request_clos *d = g_queue_pop_tail (priv->pending);
    g_assert (c == d);
  }

  if (is_mocked ())
    get_url_mocked (c->self, c->url, c->headers, c->result, c->cancellable);
  else
    get_url_now (c->self, c->url, c->headers, c->result, c->cancellable);

  return FALSE;
}

static void
get_url (GrlNetWc *self,
         const char *url,
         GHashTable *headers,
         GAsyncResult *result,
         GCancellable *cancellable)
{
  guint id;
  GTimeVal now;
  struct request_clos *c;
  GrlNetWcPrivate *priv = self->priv;

  /* closure */
  c = g_new (struct request_clos, 1);
  c->self = self;
  c->url = g_strdup (url);
  c->headers = headers? g_hash_table_ref (headers): NULL;
  c->result = result;
  c->cancellable = cancellable;

  g_get_current_time (&now);

  if ((now.tv_sec - priv->last_request.tv_sec) > priv->throttling
          || is_mocked()) {
    id = g_idle_add_full (G_PRIORITY_HIGH_IDLE,
                          get_url_cb, c, request_clos_destroy);
  } else {
    GRL_DEBUG ("delaying web request");

    priv->last_request.tv_sec += priv->throttling;
    id = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT,
                                     priv->last_request.tv_sec - now.tv_sec,
                                     get_url_cb, c, request_clos_destroy);
  }

  c->source_id = id;
  g_queue_push_head (self->priv->pending, c);
}

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
  grl_net_wc_request_with_headers_hash_async (self,
                                              uri,
                                              NULL,
                                              cancellable,
                                              callback,
                                              user_data);
}

/**
 * grl_net_wc_request_with_headers_async:
 * @self: a #GrlNetWc instance
 * @uri: The URI of the resource to request
 * @cancellable: (allow-none): a #GCancellable instance or %NULL to ignore
 * @callback: The callback when the result is ready
 * @user_data: User data set for the @callback
 * @Varargs: List of tuples of header name and header value, terminated by
 * %NULL.
 *
 * Request the fetching of a web resource given the @uri. This request is
 * asynchronous, thus the result will be returned within the @callback.
 *
 * Since: 0.2.2
 */
void grl_net_wc_request_with_headers_async (GrlNetWc *self,
                                            const char *uri,
                                            GCancellable *cancellable,
                                            GAsyncReadyCallback callback,
                                            gpointer user_data,
                                            ...)
{
  va_list va_args;
  const gchar *header_name = NULL, *header_value = NULL;
  GHashTable *headers = NULL;

  va_start (va_args, user_data);

  header_name = va_arg (va_args, const gchar *);
  while (header_name) {
    header_value = va_arg (va_args, const gchar *);
    if (header_value) {
      if (headers == NULL) {
        headers = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         g_free);
      }
      g_hash_table_insert (headers, g_strdup (header_name), g_strdup (header_value));
    }
    header_name = va_arg (va_args, const gchar *);
  }

  va_end (va_args);

  grl_net_wc_request_with_headers_hash_async (self,
                                              uri,
                                              headers,
                                              cancellable,
                                              callback,
                                              user_data);

  if (headers)
    g_hash_table_unref (headers);
}


/**
 * grl_net_wc_request_with_headers_hash_async:
 * @self: a #GrlNetWc instance
 * @uri: The URI of the resource to request
 * @headers: (allow-none) (element-type utf8 utf8): a set of additional HTTP
 * headers for this request or %NULL to ignore
 * @cancellable: (allow-none): a #GCancellable instance or %NULL to ignore
 * @callback: The callback when the result is ready
 * @user_data: User data set for the @callback
 *
 * Request the fetching of a web resource given the @uri. This request is
 * asynchronous, thus the result will be returned within the @callback.
 *
 * Since: 0.2.2
 * Rename to: grl_net_wc_request_with_headers_async
 */
void
grl_net_wc_request_with_headers_hash_async (GrlNetWc *self,
                                            const char *uri,
                                            GHashTable *headers,
                                            GCancellable *cancellable,
                                            GAsyncReadyCallback callback,
                                            gpointer user_data)
{
  GSimpleAsyncResult *result;

  result = g_simple_async_result_new (G_OBJECT (self),
                                      callback,
                                      user_data,
                                      grl_net_wc_request_async);

  get_url (self, uri, headers, G_ASYNC_RESULT (result), cancellable);
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

  void *op = g_simple_async_result_get_op_res_gpointer (res);

  if (g_simple_async_result_propagate_error (res, error) == TRUE) {
    ret = FALSE;
    goto end_func;
  }

  if (is_mocked ())
    get_content_mocked (self, op, content, length);
  else
    get_content(self, op, content, length);

end_func:
  if (is_mocked ())
    free_mock_op_res (op);
  else
    free_op_res (op);

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

  if (use_cache)
    cache_up (self);
  else
    cache_down (self);
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

  cache_set_size (self, size);
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
  g_return_if_fail (GRL_IS_NET_WC (self));

  GrlNetWcPrivate *priv = self->priv;
  struct request_clos *c;

  while ((c = g_queue_pop_head (priv->pending))) {
    g_source_remove (c->source_id);
    g_object_unref (c->cancellable);
    g_free (c->url);
    g_free (c);
  }

  g_get_current_time (&priv->last_request);
}
