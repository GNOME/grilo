
/*
 * Configuring plugins in Grilo.
 * Shows how to configure the Youtube plugin.
 */

#include <grilo.h>

#define GRL_LOG_DOMAIN_DEFAULT  example_log_domain
GRL_LOG_DOMAIN_STATIC(example_log_domain);

static void
source_added_cb (GrlRegistry *registry, GrlSource *source, gpointer user_data)
{
  /* If the Youtube plugin is installed, you should see it here now! */
  g_debug ("Detected new source available: '%s'",
           grl_source_get_name (source));
}

static void
load_plugins (void)
{
  GrlRegistry *registry;
  GError *error = NULL;

  registry = grl_registry_get_default ();

  g_signal_connect (registry, "source-added",
                    G_CALLBACK (source_added_cb), NULL);

  if (!grl_registry_load_all_plugins (registry, TRUE, &error)) {
    g_error ("Failed to load plugins: %s", error->message);
  }
}

static void
configure_plugins (void)
{
  GrlConfig *config;
  GrlRegistry *registry;

  /* Let's configure only the Youtube plugin (only requires an API key) */
  config = grl_config_new ("grl-youtube", NULL);
  grl_config_set_api_key (config,
                          "AI39si4EfscPllSfUy1IwexMf__kntTL_G5dfSr2iUEVN45RHG"
                          "q92Aq0lX25OlnOkG6KTN-4soVAkAf67fWYXuHfVADZYr7S1A");
  registry = grl_registry_get_default ();
  grl_registry_add_config (registry, config, NULL);

  /* When the plugin is loaded, the framework will provide
     this configuration for it */
}

gint
main (int argc, gchar *argv[])
{
  GMainLoop *loop;

  grl_init (&argc, &argv);
  GRL_LOG_DOMAIN_INIT (example_log_domain, "example");
  configure_plugins ();       /* Configure plugins */
  load_plugins ();            /* Load Grilo plugins */

  /* Run the main loop */
  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  grl_deinit ();

  return 0;
}
