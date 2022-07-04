/*
 * Copyright (C) 2016 Victor Toso <me@victortoso.com>
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

#include <string.h>
#include <grilo.h>
#include <net/grl-net.h>
#include <libsoup/soup.h>

#include "net/grl-net-wc-private.h"

typedef struct {
  GrlRegistry *registry;
  GMainLoop *loop;
  SoupServer *server;
  GCancellable *cancellable;
  guint timeout;
  guint num_operations;
  gboolean timeout_is_expected;
} Fixture;

typedef struct {
  Fixture *f;
  guint id;
  gint64 request_time;
  gint64 expected_time;
} ThrottlingOperation;

#define NO_DELAY     0
#define DELAY        2
#define BIG_DELAY    15     /* Big enough for a timeout */

#define THRESHOLD    1      /* 1 second, see https://bugzilla.gnome.org/show_bug.cgi?id=774578 */

#define NUM_STRESS_TEST 100

static void
fixture_setup (Fixture *fixture, gconstpointer data)
{
  fixture->num_operations = 0;
  fixture->registry = grl_registry_get_default ();
  fixture->loop = g_main_loop_new (NULL, TRUE);
  fixture->server = soup_server_new (NULL, NULL);
  fixture->cancellable = g_cancellable_new ();
  fixture->timeout_is_expected = FALSE;
}

static void
fixture_teardown (Fixture *fixture, gconstpointer data)
{
  g_main_loop_unref (fixture->loop);
  g_object_unref (fixture->server);
  g_object_unref (fixture->cancellable);
}

/* unexpected */
static gboolean
timeout (gpointer user_data)
{
  Fixture *f = user_data;
  g_main_loop_quit (f->loop);

  if (!f->timeout_is_expected)
    g_test_fail ();

  return G_SOURCE_REMOVE;
}

#if SOUP_CHECK_VERSION (2, 99, 2)
static void
soup_server_throttling_cb (SoupServer *server,
                           SoupServerMessage *message,
                           const char *path,
                           GHashTable *query,
                           gpointer user_data)
{
  gchar *response = g_strdup_printf ("%" G_GINT64_FORMAT, g_get_monotonic_time());

  soup_server_message_set_response (message, "text/plain", SOUP_MEMORY_TAKE, response, strlen(response));
  soup_server_message_set_status (message, SOUP_STATUS_OK, NULL);
}
#else
static void
soup_server_throttling_cb (SoupServer *server,
                           SoupMessage *message,
                           const char *path,
                           GHashTable *query,
                           SoupClientContext *client,
                           gpointer user_data)
{
  gchar *response = g_strdup_printf ("%" G_GINT64_FORMAT, g_get_monotonic_time());

  soup_message_set_response (message, "text/plain", SOUP_MEMORY_TAKE, response, strlen(response));
  soup_message_set_status (message, SOUP_STATUS_OK);
}
#endif

static void
test_net_wc_throttling_cb (GObject *source_object,
                           GAsyncResult *res,
                           gpointer user_data)
{
  gchar *data;
  gsize len;
  gboolean ret;
  guint64 received_time;
  GError *err = NULL;
  ThrottlingOperation *op = user_data;
  Fixture *f = op->f;

  ret = grl_net_wc_request_finish (GRL_NET_WC (source_object), res, &data, &len, &err);
  g_assert_no_error (err);
  g_assert_true (ret);
  received_time = g_ascii_strtoull (data, NULL, 0);
  g_assert_cmpint (received_time, >=, op->expected_time);

  g_free (op);
  f->num_operations--;

  if (f->num_operations == 0) {
    if (f->timeout > 0)
        g_source_remove(f->timeout);
    g_main_loop_quit (f->loop);
  }
}

static ThrottlingOperation *
throttling_operation_new (Fixture *f, guint delay_in_sec)
{
  ThrottlingOperation *op;
  static guint id = 0;

  op = g_new(ThrottlingOperation, 1);
  op->f = f;
  op->id = id;
  op->request_time = g_get_monotonic_time ();
  op->expected_time = op->request_time + (G_USEC_PER_SEC * delay_in_sec);

  /* Some THRESOLD seems necessary as the test time happens before the actual
   * GrlNetWc computation. This test is more about throttling working than
   * precision. */
  op->expected_time -= G_USEC_PER_SEC * THRESHOLD;

  id++;
  f->num_operations++;
  return op;
}

static void
test_net_wc_small_throttling (Fixture *f,
                              gconstpointer data)
{
  GSList *uris;
  gchar *request;
  GrlNetWc *wc;
  ThrottlingOperation *op;

  g_test_bug ("769331");

  GError *error = NULL;
  soup_server_add_handler (f->server, NULL, soup_server_throttling_cb, NULL, NULL);
  soup_server_listen_local (f->server, 0, SOUP_SERVER_LISTEN_IPV4_ONLY, &error);
  g_assert_no_error (error);

  uris = soup_server_get_uris (f->server);
  g_assert_nonnull (uris);
#if SOUP_CHECK_VERSION (2, 99, 2)
  request = g_uri_to_string_partial (uris->data, G_URI_HIDE_PASSWORD);
  g_slist_free_full (uris, (GDestroyNotify) g_uri_unref);
#else
  request = soup_uri_to_string (uris->data, FALSE);
  g_slist_free_full (uris, (GDestroyNotify) soup_uri_free);
#endif
  g_assert_nonnull (request);

  wc = grl_net_wc_new ();
  grl_net_wc_set_throttling (wc, DELAY);

  /* The throttling is considered between requests which means that the first
   * request is done as fast as possible */
  op = throttling_operation_new(f, NO_DELAY);
  grl_net_wc_request_async (wc, request, f->cancellable, test_net_wc_throttling_cb, op);

  op = throttling_operation_new(f, DELAY);
  grl_net_wc_request_async (wc, request, f->cancellable, test_net_wc_throttling_cb, op);

  g_object_unref (wc);
  g_free (request);

  f->timeout = g_timeout_add_seconds (5, timeout, f);
  g_main_loop_run (f->loop);
}

static void
test_net_wc_big_throttling (Fixture *f,
                            gconstpointer data)
{
  GSList *uris;
  gchar *request;
  GrlNetWc *wc;
  ThrottlingOperation *op;

  g_test_bug ("769331");

  GError *error = NULL;
  soup_server_add_handler (f->server, NULL, soup_server_throttling_cb, NULL, NULL);
  soup_server_listen_local (f->server, 0, SOUP_SERVER_LISTEN_IPV4_ONLY, &error);
  g_assert_no_error (error);

  uris = soup_server_get_uris (f->server);
  g_assert_nonnull (uris);
#if SOUP_CHECK_VERSION (2, 99, 2)
  request = g_uri_to_string_partial (uris->data, G_URI_HIDE_PASSWORD);
  g_slist_free_full (uris, (GDestroyNotify) g_uri_unref);
#else
  request = soup_uri_to_string (uris->data, FALSE);
  g_slist_free_full (uris, (GDestroyNotify) soup_uri_free);
#endif
  g_assert_nonnull (request);

  wc = grl_net_wc_new ();
  grl_net_wc_set_throttling (wc, BIG_DELAY);

  /* The throttling is considered between requests which means that the first
   * request is done as fast as possible */
  op = throttling_operation_new(f, NO_DELAY);
  grl_net_wc_request_async (wc, request, f->cancellable, test_net_wc_throttling_cb, op);

  op = throttling_operation_new(f, BIG_DELAY);
  grl_net_wc_request_async (wc, request, f->cancellable, test_net_wc_throttling_cb, op);

  g_object_unref (wc);
  g_free (request);

  f->timeout_is_expected = TRUE;
  g_timeout_add_seconds (5, timeout, f);
  g_main_loop_run (f->loop);
}

static void
test_net_wc_no_throttling_stress (Fixture *f,
                                  gconstpointer data)
{
  GSList *uris;
  gchar *request;
  GrlNetWc *wc;
  gint i;
  GError *error = NULL;

  g_test_bug ("771338");

  /* Create SoupServer with simple callback to reply */
  soup_server_add_handler (f->server, NULL, soup_server_throttling_cb, NULL, NULL);
  soup_server_listen_local (f->server, 0, SOUP_SERVER_LISTEN_IPV4_ONLY, &error);
  g_assert_no_error (error);

  uris = soup_server_get_uris (f->server);
  g_assert_nonnull (uris);
#if SOUP_CHECK_VERSION (2, 99, 2)
  request = g_uri_to_string_partial (uris->data, G_URI_HIDE_PASSWORD);
  g_slist_free_full (uris, (GDestroyNotify) g_uri_unref);
#else
  request = soup_uri_to_string (uris->data, FALSE);
  g_slist_free_full (uris, (GDestroyNotify) soup_uri_free);
#endif
  g_assert_nonnull (request);

  /* Under the same grl-net-wc, create NUM_STRESS_TEST async operations to our
   * test SoupServer to verify if any regression can be seen */
  wc = grl_net_wc_new ();
  for (i = 0; i < NUM_STRESS_TEST; i++) {
      ThrottlingOperation *op;

      op = throttling_operation_new(f, NO_DELAY);
      grl_net_wc_request_async (wc, request, f->cancellable, test_net_wc_throttling_cb, op);
  }
  g_object_unref (wc);
  g_free (request);

  f->timeout_is_expected = FALSE;
  g_timeout_add_seconds (5, timeout, f);
  g_main_loop_run (f->loop);
}

static void
test_net_properties (Fixture *f,
                     gconstpointer data)
{
  GSList *uris;
  gchar *request;
  GrlNetWc *wc;
  ThrottlingOperation *op;

  g_test_bug ("769331");

  GError *error = NULL;
  soup_server_add_handler (f->server, NULL, soup_server_throttling_cb, NULL, NULL);
  soup_server_listen_local (f->server, 0, SOUP_SERVER_LISTEN_IPV4_ONLY, &error);
  g_assert_no_error (error);

  uris = soup_server_get_uris (f->server);
  g_assert_nonnull (uris);
#if SOUP_CHECK_VERSION (2, 99, 2)
  request = g_uri_to_string_partial (uris->data, G_URI_HIDE_PASSWORD);
  g_slist_free_full (uris, (GDestroyNotify) g_uri_unref);
#else
  request = soup_uri_to_string (uris->data, FALSE);
  g_slist_free_full (uris, (GDestroyNotify) soup_uri_free);
#endif
  g_assert_nonnull (request);

  wc = grl_net_wc_new ();
  g_object_set (G_OBJECT (wc),
                "throttling", 0,
                "cache", TRUE,
                "cache-size", 5,
                "user-agent", "grl net test 0.1",
                NULL);

  op = throttling_operation_new(f, NO_DELAY);
  grl_net_wc_request_async (wc, request, f->cancellable, test_net_wc_throttling_cb, op);

  f->timeout_is_expected = FALSE;
  g_main_loop_run (f->loop);

  guint max_conns_per_host;
  g_autofree char *user_agent = NULL;

  g_assert_nonnull (wc->session);
  g_object_get (G_OBJECT (wc->session),
                "max-conns-per-host", &max_conns_per_host,
                "user-agent", &user_agent,
                NULL);
  g_assert_cmpuint (max_conns_per_host, ==, 2);
  g_assert_cmpstr (user_agent, ==, "grl net test 0.1");

  SoupSessionFeature *cache;
  g_autofree char *cache_dir = NULL;
  cache = soup_session_get_feature (wc->session, SOUP_TYPE_CACHE);
  g_assert_nonnull (cache);
  g_assert_cmpuint (soup_cache_get_max_size (SOUP_CACHE (cache)), ==, 5 * 1024 * 1024);
  g_object_get (G_OBJECT (cache), "cache-dir", &cache_dir, NULL);
  g_assert_nonnull (strstr (cache_dir, "grilo-plugin-cache"));

  g_object_unref (wc);
  g_free (request);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_bug_base ("http://bugs.gnome.org/%s");

  grl_init (&argc, &argv);

  g_test_add ("/net/throttling/small-delay",
              Fixture, NULL,
              fixture_setup,
              test_net_wc_small_throttling,
              fixture_teardown);

  g_test_add ("/net/throttling/big-delay",
              Fixture, NULL,
              fixture_setup,
              test_net_wc_big_throttling,
              fixture_teardown);

  g_test_add ("/net/throttling/disabled/stress",
              Fixture, NULL,
              fixture_setup,
              test_net_wc_no_throttling_stress,
              fixture_teardown);

  g_test_add ("/net/properties",
              Fixture, NULL,
              fixture_setup,
              test_net_properties,
              fixture_teardown);

  return g_test_run ();
}
