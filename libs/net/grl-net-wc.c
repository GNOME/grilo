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

#define LIBSOUP_USE_UNSTABLE_REQUEST_API

#include <errno.h>
#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>
#include <libsoup/soup-cache.h>
#include <libsoup/soup-request-http.h>
#include <libsoup/soup.h>
#include <string.h>

#ifndef LIBSOUP_REQUESTER_DEPRECATED
#include <libsoup/soup-requester.h>
#endif

#include <grilo.h>
#include "grl-net-wc.h"
#include "grl-net-mock-private.h"

#define GRL_LOG_DOMAIN_DEFAULT wc_log_domain
GRL_LOG_DOMAIN_STATIC(wc_log_domain);

#define GRL_NET_CAPTURE_DIR_VAR "GRL_NET_CAPTURE_DIR"

enum {
  PROP_0,
  PROP_LOG_LEVEL,
  PROP_THROTTLING,
  PROP_CACHE,
  PROP_CACHE_SIZE,
  PROP_USER_AGENT
};

struct request_res {
  SoupRequest *request;
  gchar *buffer;
  gsize length;
  gsize offset;
};

struct _GrlNetWcPrivate {
  SoupSession *session;
  SoupLoggerLogLevel log_level;
  /* throttling in secs */
  guint throttling;
  /* last request time  */
  GTimeVal last_request;
  /* closure queue for delayed requests */
  GQueue *pending;
  /* cache size in Mb */
  guint cache_size;
#ifndef LIBSOUP_REQUESTER_DEPRECATED
  void *requester;
#endif
  gchar *previous_data;
};

static const char *capture_dir = NULL;

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

static void
free_op_res (void *op)
{
  struct request_res *rr = op;

  g_object_unref (rr->request);
  g_slice_free (struct request_res, rr);
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
init_dump_directory (void)
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

static void
cache_down (GrlNetWc *self)
{
  GFile *cache_dir_file;
  GrlNetWcPrivate *priv = self->priv;
  SoupSessionFeature *cache = soup_session_get_feature (priv->session, SOUP_TYPE_CACHE);
  gchar *cache_dir;

  GRL_DEBUG ("cache down");

  if (!cache) {
    return;
  }

  soup_cache_clear (SOUP_CACHE (cache));

  g_object_get (cache, "cache-dir", &cache_dir, NULL);
  cache_dir_file = g_file_new_for_path (cache_dir);
  g_free (cache_dir);

  g_file_delete (cache_dir_file, NULL, NULL);
  g_object_unref (G_OBJECT (cache_dir_file));

  soup_session_remove_feature (priv->session, cache);
}

static void
cache_up (GrlNetWc *self)
{
  SoupCache *cache;
  GrlNetWcPrivate *priv = self->priv;
  gchar *dir;

  GRL_DEBUG ("cache up");

  dir = g_dir_make_tmp ("grilo-plugin-cache-XXXXXX", NULL);
  if (!dir)
    return;

  cache = soup_cache_new (dir, SOUP_CACHE_SINGLE_USER);
  g_free (dir);

  soup_session_add_feature (priv->session,
                            SOUP_SESSION_FEATURE (cache));

  if (priv->cache_size) {
    soup_cache_set_max_size (cache, priv->cache_size * 1024 * 1024);
  }

  g_object_unref (cache);
}

static gboolean
cache_is_available (GrlNetWc *self)
{
  return soup_session_get_feature (self->priv->session, SOUP_TYPE_CACHE) != NULL;
}

static void
init_requester (GrlNetWc *self)
{
#ifndef LIBSOUP_REQUESTER_DEPRECATED
  GrlNetWcPrivate *priv = self->priv;

  priv->requester = soup_requester_new ();
  soup_session_add_feature (priv->session,
                            SOUP_SESSION_FEATURE (priv->requester));
#endif
  init_dump_directory ();
}

static void
finalize_requester (GrlNetWc *self)
{
  GrlNetWcPrivate *priv = self->priv;

  cache_down (self);
  g_free (priv->previous_data);

#ifndef LIBSOUP_REQUESTER_DEPRECATED
  g_object_unref (priv->requester);
#endif
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
    g_value_set_uint (value, wc->priv->cache_size);
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

static void
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
                                     _("Cannot connect to the server"));
    return;
  case SOUP_STATUS_CANT_RESOLVE_PROXY:
  case SOUP_STATUS_CANT_CONNECT_PROXY:
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_PROXY_ERROR,
                                     _("Cannot connect to the proxy server"));
    return;
  case SOUP_STATUS_INTERNAL_SERVER_ERROR: /* 500 */
  case SOUP_STATUS_MALFORMED:
  case SOUP_STATUS_BAD_REQUEST: /* 400 */
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_PROTOCOL_ERROR,
                                     _("Invalid request URI or header: %s"),
                                     response);
    return;
  case SOUP_STATUS_UNAUTHORIZED: /* 401 */
  case SOUP_STATUS_FORBIDDEN: /* 403 */
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_AUTHENTICATION_REQUIRED,
                                     _("Authentication required: %s"), response);
    return;
  case SOUP_STATUS_NOT_FOUND: /* 404 */
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_NOT_FOUND,
                                     _("The requested resource was not found: %s"),
                                     response);
    return;
  case SOUP_STATUS_CONFLICT: /* 409 */
  case SOUP_STATUS_PRECONDITION_FAILED: /* 412 */
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_CONFLICT,
                                     _("The entry has been modified since it was downloaded: %s"),
                                     response);
    return;
  case SOUP_STATUS_CANCELLED:
    g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_CANCELLED,
                                     _("Operation was cancelled"));
    return;
  default:
    g_message (_("Unhandled status: %s"), soup_status_get_phrase (status));
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

static void
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
                                       _("Operation was cancelled"));
    } else {
      g_simple_async_result_set_error (result, GRL_NET_WC_ERROR,
                                       GRL_NET_WC_ERROR_UNAVAILABLE,
                                       _("Data not available"));
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
                                     _("Data not available"));
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

static void
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

#ifdef LIBSOUP_REQUESTER_DEPRECATED
  SoupURI *uri = soup_uri_new (url);
  if (uri) {
    rr->request = soup_session_request_uri (priv->session, uri, NULL);
    soup_uri_free (uri);
  } else {
    rr->request = NULL;
  }
#else
  rr->request = soup_requester_request (priv->requester, url, NULL);
#endif

  if (!rr->request) {
    g_simple_async_result_set_error (G_SIMPLE_ASYNC_RESULT (result),
                                     GRL_NET_WC_ERROR,
                                     GRL_NET_WC_ERROR_UNAVAILABLE,
                                     _("Invalid URL %s"),
                                     url);
    g_simple_async_result_complete (G_SIMPLE_ASYNC_RESULT (result));
    g_object_unref (result);
    return;
  }

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

static void
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
 * @...: List of tuples of header name and header value, terminated by
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
 * Rename to: grl_net_wc_request_with_headers_async
 *
 * Since: 0.2.2
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

  if (self->priv->cache_size == size)
    return;

  self->priv->cache_size = size;

  SoupSessionFeature *cache = soup_session_get_feature (self->priv->session, SOUP_TYPE_CACHE);
  if (!cache)
    return;

  soup_cache_set_max_size (SOUP_CACHE (cache), size * 1024 * 1024);
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
