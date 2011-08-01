#include "grl-net-wc.h"

#include <stdio.h>
#include <unistd.h>

GMainLoop *loop;
const gchar *uri;

static gboolean
quit (gpointer data)
{
  g_object_unref (G_OBJECT (data));
  g_main_loop_quit (loop);

  return FALSE;
}

static void
fetch_result (GObject *source,
	      GAsyncResult *result,
	      gpointer user_data)
{
  gchar *content;
  gsize length;
  GError *error = NULL;

  if (!grl_net_wc_request_finish (GRL_NET_WC (source),
                                  result,
                                  &content,
                                  &length,
                                  &error)) {
    g_print ("Error: %s\n", error->message);
  } else {
    if (write (fileno (stdout), content, length) < 0) {
      g_print ("Error: unable to dump content on STDOUT\n");
    }
  }

  g_idle_add (quit, source);
}

static gboolean
cancel_request (gpointer data)
{
  GCancellable *cancellable;

  cancellable = G_CANCELLABLE (data);
  g_cancellable_cancel (cancellable);

  return FALSE;
}

static gboolean
request (gpointer data)
{
  GCancellable *cancellable;
  GrlNetWc *wc;
  const gchar *u;

  wc = GRL_NET_WC (data);
  u = uri ? uri : "http://www.yahoo.com";

  cancellable = g_cancellable_new ();
  grl_net_wc_request_async (wc, u, cancellable, fetch_result, NULL);
  g_timeout_add_seconds (2, cancel_request, cancellable);

  return FALSE;
}

int
main (int argc, const char **argv)
{
  GrlNetWc *wc;

  g_thread_init (NULL);
  g_type_init ();

  if (argc == 2)
    uri = argv[1];

  wc = grl_net_wc_new ();

  g_object_set (wc, "loglevel", 1, NULL);

  loop = g_main_loop_new (NULL, FALSE);

  g_idle_add (request, wc);

  g_main_loop_run (loop);
}
