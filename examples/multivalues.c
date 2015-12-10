
/*
 * Handling multivalued elements in Grilo.
 * Search all 'rock' content in Youtube, and for each one prints the available
 * URLs.
 */

#include <grilo.h>
#include <string.h>

#define GRL_LOG_DOMAIN_DEFAULT  example_log_domain
GRL_LOG_DOMAIN_STATIC(example_log_domain);

static void
search_cb (GrlSource *source,
	   guint browse_id,
	   GrlMedia *media,
	   guint remaining,
	   gpointer user_data,
	   const GError *error)
{
  guint i;
  GrlRelatedKeys *url_info;

  if (error) {
    g_error ("Search operation failed. Reason: %s", error->message);
  }

  if (media) {
    /* Look through all available URLs for this video resource */
    for (i = 0; i < grl_data_length (GRL_DATA (media), GRL_METADATA_KEY_URL); i++) {
      /* Here we use the low-level GrlRelatedKeys API for demonstration purposes only,
         but we could have just used the more convenient
         grl_media_video_get_url_data_nth() API instead in this case */
      url_info = grl_data_get_related_keys (GRL_DATA (media), GRL_METADATA_KEY_URL, i);
      g_debug ("\t [%s] Got url '%s' and mime-type '%s'",
               grl_media_get_id (media),
               grl_related_keys_get_string (url_info, GRL_METADATA_KEY_URL),
               grl_related_keys_get_string (url_info, GRL_METADATA_KEY_MIME));
    }
  }

  if (remaining == 0) {
    g_debug ("Search operation finished!");
  }

  g_object_unref (media);
}

static void
source_added_cb (GrlRegistry *registry, GrlSource *source, gpointer user_data)
{
  const gchar *id;
  GrlCaps *caps;
  GrlOperationOptions *options;
  GList * keys = grl_metadata_key_list_new (GRL_METADATA_KEY_TITLE,
					    GRL_METADATA_KEY_URL,
                                            GRL_METADATA_KEY_MIME,
					    GRL_METADATA_KEY_INVALID);

  /* Not interested if not searchable */
  if (!(grl_source_supported_operations (source) & GRL_OP_SEARCH))
    return;

  g_debug ("Detected new searchable source available: '%s'",
	   grl_source_get_name (source));

  /* Only interested in Youtube */
  id = grl_source_get_id (source);
  if (strcmp (id, "grl-youtube"))
    return;

  caps = grl_source_get_caps (source, GRL_OP_SEARCH);
  options = grl_operation_options_new (caps);
  grl_operation_options_set_skip (options, 0);
  grl_operation_options_set_count (options, 5);
  grl_operation_options_set_resolution_flags (options, GRL_RESOLVE_IDLE_RELAY);

  g_debug ("Searching \"rock\" in Youtube");
  grl_source_search (source,
			   "rock",
			   keys,
			   options,
			   search_cb,
			   NULL);

  g_object_unref (options);
  g_object_unref (caps);

  g_list_free (keys);
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

  config = grl_config_new ("grl-youtube", NULL);
  grl_config_set_api_key (config,
                          "AI39si4EfscPllSfUy1IwexMf__kntTL_G5dfSr2iUEVN45RHG"
                          "q92Aq0lX25OlnOkG6KTN-4soVAkAf67fWYXuHfVADZYr7S1A");
  registry = grl_registry_get_default ();
  grl_registry_add_config (registry, config, NULL);
}

gint
main (int argc, gchar *argv[])
{
  GMainLoop *loop;
  grl_init (&argc, &argv);
  GRL_LOG_DOMAIN_INIT (example_log_domain, "example");
  configure_plugins ();
  load_plugins ();
  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);
  grl_deinit ();
  return 0;
}
