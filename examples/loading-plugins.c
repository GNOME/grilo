
/*
 * Loading plugins in Grilo.
 * Tracks sources adding and removing
 */

#include <grilo.h>

#define GRL_LOG_DOMAIN_DEFAULT  example_log_domain
GRL_LOG_DOMAIN_STATIC(example_log_domain);

static void
source_added_cb (GrlRegistry *registry, GrlSource *source, gpointer user_data)
{
  g_debug ("Detected new source available: '%s'",
           grl_source_get_name (source));

  /* Usually you may add the new service to the user interface so the user
     can interact with it (browse, search, etc) */
}

static void
source_removed_cb (GrlRegistry *registry, GrlSource *source, gpointer user_data)
{
  g_debug ("Source '%s' is gone",
           grl_source_get_name (source));

  /* Usually you would inform the user that this service is no longer
     available (for example a UPnP server was shutdown) and remove it
     from the user interface. */
}

static void
load_plugins (void)
{
  GrlRegistry *registry;
  GError *error = NULL;

  registry = grl_registry_get_default ();

  /* These callback will be invoked when media providers
     are loaded/unloaded */
  g_signal_connect (registry, "source-added",
                    G_CALLBACK (source_added_cb), NULL);
  g_signal_connect (registry, "source-removed",
                    G_CALLBACK (source_removed_cb), NULL);

  /* Command the registry to load all available plugins.
     The registry will look for plugins in the default
     plugin path and directories specified using the
     GRL_PLUGIN_PATH environment variable */
  if (!grl_registry_load_all_plugins (registry, TRUE, &error)) {
    g_error ("Failed to load plugins: %s", error->message);
  }
}

gint
main (int argc, gchar *argv[])
{
  GMainLoop *loop;

  grl_init (&argc, &argv);
  GRL_LOG_DOMAIN_INIT (example_log_domain, "example");
  load_plugins ();            /* Load Grilo plugins */

  /* Run the main loop */
  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  grl_deinit ();

  return 0;
}
