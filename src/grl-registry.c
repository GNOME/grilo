/*
 * Copyright (C) 2010, 2011 Igalia S.L.
 * Copyright (C) 2011 Intel Corporation.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
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
 * SECTION:grl-registry
 * @short_description: Grilo plugins loader and manager
 * @see_also: #GrlPlugin, #GrlSource
 *
 * The registry holds the metadata of a set of plugins.
 *
 * The #GrlRegistry object is a list of plugins and some functions
 * for dealing with them. Each #GrlPlugin is matched 1-1 with a file
 * on disk, and may or may not be loaded a given time. There only can be
 * a single instance of #GrlRegistry (singleton pattern).
 *
 * A #GrlPlugin can hold several data #GrlSource sources, and #GrlRegistry
 * shall register each one of them.
 */

#include "grl-registry.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "grl-registry-priv.h"
#include "grl-plugin-priv.h"
#include "grl-log.h"
#include "grl-error.h"

#include <glib/gi18n-lib.h>
#include <string.h>
#include <gmodule.h>
#include <libxml/parser.h>

#define GRL_LOG_DOMAIN_DEFAULT  registry_log_domain
GRL_LOG_DOMAIN(registry_log_domain);

#define XML_ROOT_ELEMENT_NAME "plugin"

#define GRL_PLUGIN_INFO_SUFFIX "xml"

#define GRL_PLUGIN_INFO_MODULE "module"

#define LOCAL_NET_TAG      "net:local"
#define INTERNET_NET_TAG   "net:internet"

#define SET_INVISIBLE_SOURCE(src, val)                          \
  g_object_set_data(G_OBJECT(src), "invisible", GINT_TO_POINTER(val))
#define SOURCE_IS_INVISIBLE(src)                                \
  GPOINTER_TO_INT(g_object_get_data(G_OBJECT(src), "invisible"))

/* GQuark-like implementation, where we manually assign the first IDs. */
struct KeyIDHandler {
  GHashTable *string_to_id;
  GArray *id_to_string;
  gint last_id;
};

struct _GrlRegistryPrivate {
  GHashTable *configs;
  GHashTable *plugins;
  GHashTable *sources;
  GHashTable *related_keys;
  GHashTable *system_keys;
  GHashTable *ranks;
  GSList *plugins_dir;
  GSList *allowed_plugins;
  gboolean all_plugins_preloaded;
  struct KeyIDHandler key_id_handler;
  GNetworkMonitor *netmon;
};

static void grl_registry_setup_ranks (GrlRegistry *registry);

static void key_id_handler_init (struct KeyIDHandler *handler);

static void key_id_handler_free (struct KeyIDHandler *handler);

static GrlKeyID key_id_handler_get_key (struct KeyIDHandler *handler,
                                        const gchar *key_name);

static const gchar *key_id_handler_get_name (struct KeyIDHandler *handler,
                                             GrlKeyID key);

static GrlKeyID key_id_handler_add (struct KeyIDHandler *handler,
                                    GrlKeyID key, const gchar *key_name);

static gboolean param_spec_is_equal (GParamSpec *curr, GParamSpec *new);

static void shutdown_plugin (GrlPlugin *plugin);

static void configs_free (GList *configs);

static GrlPlugin *grl_registry_prepare_plugin (GrlRegistry *registry,
                                               const gchar *library_filename,
                                               GError **error);

/* ================ GrlRegistry GObject ================ */

enum {
  SIG_SOURCE_ADDED,
  SIG_SOURCE_REMOVED,
  SIG_METADATA_KEY_ADDED,
  SIG_LAST
};
static gint registry_signals[SIG_LAST];

G_DEFINE_TYPE_WITH_PRIVATE (GrlRegistry, grl_registry, G_TYPE_OBJECT);

static void
grl_registry_class_init (GrlRegistryClass *klass)
{
  /**
   * GrlRegistry::source-added:
   * @registry: the registry
   * @source: the source that has been added
   *
   * Signals that a source has been added to the registry.
   *
   * Since: 0.2.0
   */
  registry_signals[SIG_SOURCE_ADDED] =
    g_signal_new("source-added",
		 G_TYPE_FROM_CLASS(klass),
		 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		 0,
		 NULL,
		 NULL,
		 g_cclosure_marshal_VOID__OBJECT,
		 G_TYPE_NONE, 1, GRL_TYPE_SOURCE);

  /**
   * GrlRegistry::source-removed:
   * @registry: the registry
   * @source: the source that has been removed
   *
   * Signals that a source has been removed from the registry.
   *
   * Since: 0.2.0
   */
  registry_signals[SIG_SOURCE_REMOVED] =
    g_signal_new("source-removed",
		 G_TYPE_FROM_CLASS(klass),
		 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		 0,
		 NULL,
		 NULL,
		 g_cclosure_marshal_VOID__OBJECT,
		 G_TYPE_NONE, 1, GRL_TYPE_SOURCE);

  /**
   * GrlRegistry::metadata-key-added:
   * @registry: the registry
   * @key: the name of the new key added
   *
   * Signals that a new metadata key has been registered.
   *
   * Since: 0.2.10
   */
  registry_signals[SIG_METADATA_KEY_ADDED] =
    g_signal_new("metadata-key-added",
                 G_TYPE_FROM_CLASS(klass),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 0,
                 NULL,
                 NULL,
                 g_cclosure_marshal_VOID__STRING,
                 G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
get_connectivity (GrlRegistry          *registry,
                  GNetworkConnectivity *connectivity,
                  gboolean             *network_available)
{
  g_assert (connectivity != NULL);
  g_assert (network_available != NULL);

  if (g_getenv("GRL_NET_MOCKED") != NULL) {
    GRL_DEBUG ("Mocked network, assuming network is available and connectivity "
               "level is FULL");
    *connectivity = G_NETWORK_CONNECTIVITY_FULL;
    *network_available = TRUE;
  } else {
    g_object_get (G_OBJECT (registry->priv->netmon),
                  "connectivity", connectivity,
                  "network-available", network_available,
                  NULL);

    GRL_DEBUG ("Connectivity level is %d, Network is %s",
               *connectivity, *network_available ? "available" : "unavailable");
  }
}

static void
network_changed_cb (GObject     *gobject,
                    GParamSpec  *pspec,
                    GrlRegistry *registry)
{
  GNetworkConnectivity connectivity;
  gboolean network_available;
  GrlSource *current_source;
  GList *sources, *l;

  GRL_DEBUG ("Network availability changed");
  get_connectivity (registry, &connectivity, &network_available);

  sources = g_hash_table_get_values (registry->priv->sources);
  if (!sources)
    return;

  if (!network_available) {
    for (l = sources; l != NULL; l = l->next) {
      const char **tags;

      current_source = l->data;
      tags = grl_source_get_tags (current_source);

      if (!tags)
        continue;

      if ((g_strv_contains (tags, LOCAL_NET_TAG) ||
           g_strv_contains (tags, INTERNET_NET_TAG)) &&
          !SOURCE_IS_INVISIBLE(current_source)) {
        GRL_DEBUG ("Network isn't available for '%s', hiding",
                   grl_source_get_id (current_source));
        SET_INVISIBLE_SOURCE(current_source, TRUE);
        g_signal_emit (registry, registry_signals[SIG_SOURCE_REMOVED], 0, current_source);
      }
    }
  } else {
    GList *to_add, *to_remove;

    to_add = to_remove = NULL;

    for (l = sources; l != NULL; l = l->next) {
      const char **tags;

      current_source = l->data;
      tags = grl_source_get_tags (current_source);

      if (!tags)
        continue;

      if (g_strv_contains (tags, LOCAL_NET_TAG) &&
          SOURCE_IS_INVISIBLE(current_source)) {
        GRL_DEBUG ("Local network became available for '%s', showing",
                   grl_source_get_id (current_source));
        to_add = g_list_prepend (to_add, current_source);
      }

      if (g_strv_contains (tags, INTERNET_NET_TAG) &&
          connectivity == G_NETWORK_CONNECTIVITY_FULL &&
          SOURCE_IS_INVISIBLE(current_source)) {
        GRL_DEBUG ("Internet became available for '%s', showing",
                   grl_source_get_id (current_source));
        to_add = g_list_prepend (to_add, current_source);
      }

      if (g_strv_contains (tags, INTERNET_NET_TAG) &&
          connectivity != G_NETWORK_CONNECTIVITY_FULL &&
          !SOURCE_IS_INVISIBLE(current_source)) {
        GRL_DEBUG ("Internet became unavailable for '%s', hiding",
                   grl_source_get_id (current_source));
        to_remove = g_list_prepend (to_remove, current_source);
      }
    }

    for (l = to_add; l != NULL; l = l->next) {
      SET_INVISIBLE_SOURCE(l->data, FALSE);
      g_signal_emit (registry, registry_signals[SIG_SOURCE_ADDED], 0, l->data);
    }
    g_list_free (to_add);

    for (l = to_remove; l != NULL; l = l->next) {
        SET_INVISIBLE_SOURCE(l->data, TRUE);
        g_signal_emit (registry, registry_signals[SIG_SOURCE_REMOVED], 0, l->data);
    }
    g_list_free (to_remove);
  }

  g_list_free (sources);
}

static void
grl_registry_init (GrlRegistry *registry)
{
  registry->priv = grl_registry_get_instance_private (registry);

  registry->priv->configs =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) configs_free);
  registry->priv->plugins =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  registry->priv->sources =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  registry->priv->related_keys =
    g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, NULL);
  registry->priv->system_keys =
    g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify) g_param_spec_unref);

  registry->priv->netmon = g_network_monitor_get_default ();
  g_signal_connect (G_OBJECT (registry->priv->netmon), "notify::connectivity",
                    G_CALLBACK (network_changed_cb), registry);
  g_signal_connect (G_OBJECT (registry->priv->netmon), "notify::network-available",
                    G_CALLBACK (network_changed_cb), registry);

  key_id_handler_init (&registry->priv->key_id_handler);

  grl_registry_setup_ranks (registry);

  const gchar *config_path = g_getenv (GRL_CONFIG_PATH_VAR);
  if (config_path != NULL)
    grl_registry_add_config_from_file (registry, config_path, NULL);
}

/* ================ Utitilies ================ */

static void
configs_free (GList *configs)
{
  g_list_free_full (configs, g_object_unref);
}

static void
update_source_visibility (GrlRegistry *registry,
                          GrlSource   *source)
{
  GNetworkConnectivity connectivity;
  gboolean network_available;
  const char **tags;
  gboolean needs_local, needs_inet;

  tags = grl_source_get_tags (source);
  if (!tags)
    return;

  needs_local = g_strv_contains (tags, LOCAL_NET_TAG);
  needs_inet = g_strv_contains (tags, INTERNET_NET_TAG);

  if (!needs_local &&
      !needs_inet)
    return;

  get_connectivity (registry, &connectivity, &network_available);

  GRL_DEBUG ("Source %s needs %s %s%s",
             grl_source_get_id (source),
             needs_local ? "local network" : "",
             needs_inet && needs_local ? " and " : "",
             needs_inet ? "Internet" : "");

  if (!network_available) {
    if (needs_local || needs_inet) {
      GRL_DEBUG ("Network isn't available for '%s', hiding",
                 grl_source_get_id (source));
      SET_INVISIBLE_SOURCE(source, TRUE);
    }
  } else {
    if (connectivity != G_NETWORK_CONNECTIVITY_FULL) {
      if (needs_inet) {
        GRL_DEBUG ("Internet isn't available for '%s', hiding",
                   grl_source_get_id (source));
        SET_INVISIBLE_SOURCE(source, TRUE);
      }
    }
  }
}

static void
config_source_rank (GrlRegistry *registry,
                    const gchar *source_id,
                    gint rank)
{
  GRL_DEBUG ("Rank configuration, '%s:%d'", source_id, rank);
  g_hash_table_insert (registry->priv->ranks,
                       g_strdup (source_id),
                       GINT_TO_POINTER (rank));
}

static void
set_source_rank (GrlRegistry *registry, GrlSource *source)
{
  gint rank;

  rank =
    GPOINTER_TO_INT (g_hash_table_lookup (registry->priv->ranks,
                                          grl_source_get_id (source)));

  if (!rank) {
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, registry->priv->ranks);

    while (g_hash_table_iter_next (&iter, &key, &value)) {
      if (g_pattern_match_simple (key, grl_source_get_id (source))) {
        rank = GPOINTER_TO_INT (value);
        break;
      }
    }
  }

  if (!rank) {
    rank = GRL_RANK_DEFAULT;
  }

  g_object_set (source, "rank", rank, NULL);
  GRL_DEBUG ("Source rank '%s' : %d", grl_source_get_id (source), rank);
}

static void
grl_registry_setup_ranks (GrlRegistry *registry)
{
  const gchar *ranks_env;
  gchar **rank_specs;
  gchar **iter;

  registry->priv->ranks = g_hash_table_new_full (g_str_hash, g_str_equal,
						 g_free, NULL);

  ranks_env = g_getenv (GRL_PLUGIN_RANKS_VAR);
  if (!ranks_env) {
    GRL_DEBUG ("$%s is not set, using default ranks.", GRL_PLUGIN_RANKS_VAR);
    return;
  }

  rank_specs = g_strsplit (ranks_env, ",", 0);
  iter = rank_specs;

  while (*iter) {
    gchar **rank_info = g_strsplit (*iter, ":", 2);
    if (rank_info[0] && rank_info[1]) {
      gchar *tmp;
      gchar *id = rank_info[0];
      gchar *srank = rank_info[1];
      gint rank = (gint) g_ascii_strtoll (srank, &tmp, 10);
      if (*tmp != '\0') {
        GRL_WARNING ("Incorrect ranking definition: '%s'. Skipping...", *iter);
      } else {
        config_source_rank (registry, id, rank);
      }
    } else {
      GRL_WARNING ("Incorrect ranking definition: '%s'. Skipping...", *iter);
    }
    g_strfreev (rank_info);
    iter++;
  }

  g_strfreev (rank_specs);
}

static gint
compare_by_rank (gconstpointer a,
                 gconstpointer b) {
  gint rank_a;
  gint rank_b;

  rank_a = grl_source_get_rank (GRL_SOURCE (a));
  rank_b = grl_source_get_rank (GRL_SOURCE (b));

  return (rank_a < rank_b) - (rank_a > rank_b);
}

static gboolean
register_keys_plugin (GrlRegistry *registry,
                      GrlPlugin *plugin,
                      GError **error)
{
  gboolean is_loaded;

  /* Check if plugin is already loaded */
  g_object_get (plugin, "loaded", &is_loaded, NULL);
  if (is_loaded) {
    GRL_WARNING ("Plugin is already loaded: '%s'", grl_plugin_get_id (plugin));
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 _("Plugin “%s” is already loaded"), grl_plugin_get_id (plugin));
    return FALSE;
  }

  grl_plugin_register_keys (plugin);

  return TRUE;
}

static gboolean
activate_plugin (GrlRegistry *registry,
                 GrlPlugin *plugin,
                 GError **error)
{
  GList *plugin_configs;

  plugin_configs = g_hash_table_lookup (registry->priv->configs,
                                        grl_plugin_get_id (plugin));

  if (!grl_plugin_load (plugin, plugin_configs)) {
    GRL_DEBUG ("Failed to initialize plugin from %s. Check if plugin is well configured", grl_plugin_get_filename (plugin));
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 _("Failed to initialize plugin from %s"), grl_plugin_get_filename (plugin));
    shutdown_plugin (plugin);
    return FALSE;
  }

  GRL_DEBUG ("Loaded plugin '%s' from '%s'",
             grl_plugin_get_id (plugin),
             grl_plugin_get_filename (plugin));

  return TRUE;
}

static GrlKeyID
grl_registry_register_metadata_key_full (GrlRegistry *registry,
                                         GParamSpec *param_spec,
                                         GrlKeyID key,
                                         GrlKeyID bind_key,
                                         GError **error)
{
  GList *bound_partners;
  GList *partner;
  const gchar *key_name;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), 0);
  g_return_val_if_fail (G_IS_PARAM_SPEC (param_spec), 0);
  GrlKeyID registered_key;

  key_name = g_param_spec_get_name (param_spec);
  registered_key = key_id_handler_get_key (&registry->priv->key_id_handler, key_name);
  if (registered_key != GRL_METADATA_KEY_INVALID) {
    GParamSpec *key_spec = g_hash_table_lookup (registry->priv->system_keys,
                                                (gpointer) key_name);
    if (param_spec_is_equal (key_spec, param_spec)) {
      /* Key registered */
      GRL_DEBUG ("metadata key '%s' already registered with same spec", key_name);
      g_param_spec_unref (param_spec);
      return registered_key;
    } else {
      GRL_WARNING ("metadata key '%s' already exists", key_name);
      g_set_error (error,
                   GRL_CORE_ERROR,
                   GRL_CORE_ERROR_REGISTER_METADATA_KEY_FAILED,
                   _("Metadata key “%s” already registered in different format"),
                   key_name);
      return GRL_METADATA_KEY_INVALID;
    }
  }

  registered_key = key_id_handler_add (&registry->priv->key_id_handler, key, key_name);

  if (registered_key == GRL_METADATA_KEY_INVALID) {
    GRL_WARNING ("metadata key '%s' cannot be registered", key_name);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_REGISTER_METADATA_KEY_FAILED,
                 _("Metadata key “%s” cannot be registered"),
                 key_name);

    return GRL_METADATA_KEY_INVALID;
  }

  g_hash_table_insert (registry->priv->system_keys,
                       (gpointer) key_name,
                       param_spec);

  if (bind_key == GRL_METADATA_KEY_INVALID) {
  /* Key is only related to itself */
    g_hash_table_insert (registry->priv->related_keys,
                         GRLKEYID_TO_POINTER (registered_key),
                         g_list_prepend (NULL,
                                         GRLKEYID_TO_POINTER (registered_key)));
  } else {
    /* Add the new key to the partners */
    bound_partners = g_hash_table_lookup (registry->priv->related_keys, GRLKEYID_TO_POINTER (bind_key));
    bound_partners = g_list_append (bound_partners, GRLKEYID_TO_POINTER (registered_key));
    for (partner = bound_partners;
         partner;
         partner = g_list_next (partner)) {
      g_hash_table_insert (registry->priv->related_keys,
                           partner->data,
                           bound_partners);
    }
  }

  return registered_key;
}

G_GNUC_INTERNAL GrlKeyID
grl_registry_register_metadata_key_for_type (GrlRegistry *registry,
                                             const gchar *key_name,
                                             GType type,
                                             GrlKeyID bind_key)
{
  GParamSpec *spec;

  switch (type) {
  case G_TYPE_INT:
    spec = g_param_spec_int (key_name,
                             key_name,
                             key_name,
                             0, G_MAXINT,
                             0,
                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
    break;

  case G_TYPE_INT64:
    spec = g_param_spec_int64 (key_name,
                               key_name,
                               key_name,
                               -1, G_MAXINT64,
                               -1,
                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
    break;

  case G_TYPE_STRING:
    spec = g_param_spec_string (key_name,
                                key_name,
                                key_name,
                                NULL,
                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
    break;

  case G_TYPE_BOOLEAN:
    spec = g_param_spec_boolean (key_name,
                                 key_name,
                                 key_name,
                                 FALSE,
                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
    break;

  case G_TYPE_FLOAT:
    spec = g_param_spec_float (key_name,
                               key_name,
                               key_name,
                               0, G_MAXFLOAT,
                               0,
                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
    break;

  default:
    if (type == G_TYPE_DATE_TIME) {
        spec = g_param_spec_boxed (key_name,
                                   key_name,
                                   key_name,
                                   G_TYPE_DATE_TIME,
                                   G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
    } else {
      GRL_WARNING ("'%s' is being ignored as G_TYPE '%s' is not being handled",
                   key_name, g_type_name (type));
      return GRL_METADATA_KEY_INVALID;
    }
  }

  return grl_registry_register_metadata_key (registry, spec, bind_key, NULL);
}

/*
 * Returns whether the string is a canonical one.
 */
static gboolean
is_canonical (const gchar *key)
{
  if (key == NULL) {
    return FALSE;
  }

  for (; *key != '\0'; key++) {
    if (*key != '-' &&
        (*key < '0' || *key > '9') &&
        (*key < 'A' || *key > 'Z') &&
        (*key < 'a' || *key > 'z'))
      return FALSE;
  }

  return TRUE;
}

G_GNUC_INTERNAL GrlKeyID
grl_registry_register_or_lookup_metadata_key (GrlRegistry *registry,
                                              const gchar *key_name,
                                              const GValue *value,
                                              GrlKeyID bind_key)
{
  GrlKeyID key;
  GType value_type;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), GRL_METADATA_KEY_INVALID);

  if (value == NULL)
    return GRL_METADATA_KEY_INVALID;

  key_name = g_intern_string (key_name);
  g_return_val_if_fail (is_canonical (key_name), GRL_METADATA_KEY_INVALID);

  key = grl_registry_lookup_metadata_key (registry, key_name);
  value_type = G_VALUE_TYPE (value);

  if (key == GRL_METADATA_KEY_INVALID) {
    GRL_DEBUG ("%s is not a registered metadata-key", key_name);
    key = grl_registry_register_metadata_key_for_type (registry, key_name, value_type, bind_key);
  } else {
    GType key_type = grl_registry_lookup_metadata_key_type (registry, key);
    if (!g_value_type_transformable (value_type, key_type)) {
      GRL_WARNING ("Value type %s can't be set to existing metadata-key of type %s",
                   g_type_name (value_type),
                   g_type_name (key_type));
      return GRL_METADATA_KEY_INVALID;
    }
  }

  return key;
}

static void
key_id_handler_init (struct KeyIDHandler *handler)
{
  const gchar *null_string = NULL;
  handler->string_to_id = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  handler->id_to_string = g_array_new (FALSE, /* zero terminated */
                                       TRUE,  /* zero-initialised */
                                       sizeof (const gchar *));
  /* We want indices in ->id_to_string to start from 1, so we add a NULL entry
   * for GRL_METADATA_KEY_INVALID (i.e. 0) */
  g_array_insert_val (handler->id_to_string,
                      GRL_METADATA_KEY_INVALID,
                      null_string);
}

static void
key_id_handler_free (struct KeyIDHandler *handler)
{
  g_hash_table_unref (handler->string_to_id);
  g_array_unref (handler->id_to_string);
}

static
GrlKeyID key_id_handler_get_key (struct KeyIDHandler *handler, const gchar *key_name)
{
  gpointer val = g_hash_table_lookup (handler->string_to_id, key_name);
  if (val == NULL)
    return GRL_METADATA_KEY_INVALID;

  return GRLPOINTER_TO_KEYID (val);
}

static const gchar *
key_id_handler_get_name (struct KeyIDHandler *handler, GrlKeyID key)
{
  if (key < handler->id_to_string->len)
    return g_array_index (handler->id_to_string, const gchar *, key);

  return NULL;
}

/*
 * key_id_handler_add:
 * @handler: the handler
 * @key: a specific key for system keys, or GRL_METADATA_KEY_INVALID for it to
 * be assigned
 * @name: the name of the key.
 *
 * Add a new key<->name correspondence.
 *
 * Returns: the new key number, or GRL_METADATA_KEY_INVALID if the key could
 * not be created (typically if @key or @name is already registered).
 */
static GrlKeyID
key_id_handler_add (struct KeyIDHandler *handler, GrlKeyID key, const gchar *name)
{
  GrlKeyID _key = key;

  if (_key == GRL_METADATA_KEY_INVALID) {
    /* existing keys go from 1 to (id_to_string->len - 1), so the next
     * available key is id_to_string->len, which will be incremented by
     * g_array_insert_val() */
    _key = handler->id_to_string->len;
  }

  if (NULL != key_id_handler_get_name (handler, _key)) {
    GRL_WARNING ("Cannot register %d:%s because key is already defined as %s",
                 _key, name, key_id_handler_get_name (handler, _key));
    return GRL_METADATA_KEY_INVALID;
  } else if ( GRL_METADATA_KEY_INVALID != key_id_handler_get_key (handler, name)) {
    /* _key or name is already in use! */
    GRL_WARNING ("Cannot register %d:%s because name is already registered with key %d",
                 _key, name, key_id_handler_get_key (handler, name));
    return GRL_METADATA_KEY_INVALID;
  } else {
    /* name_copy is shared between handler->id_to_string and
     * handler->string_to_id */
    gchar *name_copy = g_strdup (name);

    if (_key >= handler->id_to_string->len)
      g_array_set_size (handler->id_to_string, _key + 1);

    /* yes, g_array_index() is a macro that give you an lvalue */
    g_array_index (handler->id_to_string, const gchar *, _key) = name_copy;
    g_hash_table_insert (handler->string_to_id,
                         name_copy, GRLKEYID_TO_POINTER (_key));
  }

  return _key;

}

static GList *
key_id_handler_get_all_keys (struct KeyIDHandler *handler)
{
  return g_hash_table_get_values (handler->string_to_id);
}

#define CHECK_NUMERIC_PARAM_SPEC_LIMITS(is_type, cast_type, a, b) {     \
  if (is_type) {                                                        \
    if ((cast_type(a))->maximum != (cast_type(b))->maximum ||           \
        (cast_type(a))->minimum != (cast_type(b))->minimum ||           \
        (cast_type(a))->default_value != (cast_type(b))->default_value) \
      return FALSE;                                                     \
    return TRUE;                                                        \
  }                                                                     \
}

/* @curr: The current spec we have
 * @new: The spec to match
 *
 * Returns: true if specs are the same, false otherwise.
 */
static gboolean
param_spec_is_equal (GParamSpec *cur,
                     GParamSpec *new)
{
  GType ctype = G_PARAM_SPEC_TYPE (cur);

  if (ctype != G_PARAM_SPEC_TYPE (new))
    return FALSE;

  CHECK_NUMERIC_PARAM_SPEC_LIMITS ((ctype == G_TYPE_PARAM_INT),
                                   G_PARAM_SPEC_INT, cur, new);
  CHECK_NUMERIC_PARAM_SPEC_LIMITS ((ctype == G_TYPE_PARAM_LONG),
                                   G_PARAM_SPEC_LONG, cur, new);
  CHECK_NUMERIC_PARAM_SPEC_LIMITS ((ctype == G_TYPE_PARAM_INT64),
                                   G_PARAM_SPEC_INT64, cur, new);
  CHECK_NUMERIC_PARAM_SPEC_LIMITS ((ctype == G_TYPE_PARAM_CHAR),
                                   G_PARAM_SPEC_CHAR, cur, new);
  CHECK_NUMERIC_PARAM_SPEC_LIMITS ((ctype == G_TYPE_PARAM_UINT),
                                   G_PARAM_SPEC_UINT, cur, new);
  CHECK_NUMERIC_PARAM_SPEC_LIMITS ((ctype == G_TYPE_PARAM_ULONG),
                                   G_PARAM_SPEC_ULONG, cur, new);
  CHECK_NUMERIC_PARAM_SPEC_LIMITS ((ctype == G_TYPE_PARAM_UINT64),
                                   G_PARAM_SPEC_UINT64, cur, new);
  CHECK_NUMERIC_PARAM_SPEC_LIMITS ((ctype == G_TYPE_PARAM_UCHAR),
                                   G_PARAM_SPEC_UCHAR, cur, new);
  CHECK_NUMERIC_PARAM_SPEC_LIMITS ((ctype == G_TYPE_PARAM_FLOAT),
                                   G_PARAM_SPEC_FLOAT, cur, new);
  CHECK_NUMERIC_PARAM_SPEC_LIMITS ((ctype == G_TYPE_PARAM_DOUBLE),
                                   G_PARAM_SPEC_DOUBLE, cur, new);
  if (ctype == G_TYPE_PARAM_STRING) {
    GParamSpecString *c = G_PARAM_SPEC_STRING (cur);
    GParamSpecString *n = G_PARAM_SPEC_STRING (new);
    return (g_strcmp0 (c->default_value, n->default_value) == 0);
  } else if (ctype == G_TYPE_PARAM_ENUM) {
    GParamSpecEnum *c = G_PARAM_SPEC_ENUM (cur);
    GParamSpecEnum *n = G_PARAM_SPEC_ENUM (new);
    if (c->default_value != n->default_value ||
        cur->value_type != new->value_type) {
      GRL_DEBUG ("%s differ (values: %d and %d) (types: %s and %s)",
                 g_type_name (ctype), c->default_value, n->default_value,
                 g_type_name (cur->value_type), g_type_name (new->value_type));
      return FALSE;
    }
  } else if (ctype == G_TYPE_PARAM_FLAGS) {
    GParamSpecFlags *c = G_PARAM_SPEC_FLAGS (cur);
    GParamSpecFlags *n = G_PARAM_SPEC_FLAGS (new);
    if (c->default_value != n->default_value ||
        cur->value_type != new->value_type) {
      GRL_DEBUG ("%s differ (values: %d and %d) (types: %s and %s)",
                 g_type_name (ctype), c->default_value, n->default_value,
                 g_type_name (cur->value_type), g_type_name (new->value_type));
      return FALSE;
    }
  } else if (ctype == G_TYPE_PARAM_BOOLEAN) {
    GParamSpecBoolean *c = G_PARAM_SPEC_BOOLEAN (cur);
    GParamSpecBoolean *n = G_PARAM_SPEC_BOOLEAN (new);
    if (c->default_value != n->default_value) {
      GRL_DEBUG ("%s type differ: %s != %s", g_type_name (ctype),
                 g_type_name (cur->value_type), g_type_name (new->value_type));
      return FALSE;
    }
  } else if (ctype == G_TYPE_PARAM_BOXED || ctype == G_TYPE_PARAM_OBJECT) {
    if (cur->value_type != new->value_type) {
      GRL_DEBUG ("%s type differ: %s != %s", g_type_name (ctype),
                 g_type_name (cur->value_type), g_type_name (new->value_type));
      return FALSE;
    }
  } else {
    g_warn_if_reached();
    return FALSE;
  }

  return TRUE;
}

#undef CHECK_NUMERIC_PARAM_SPEC_LIMITS

static void
shutdown_plugin (GrlPlugin *plugin)
{
  GRL_DEBUG ("Unloading plugin '%s'", grl_plugin_get_id (plugin));
  grl_plugin_unload (plugin);

  if (grl_plugin_get_module (plugin)) {
    g_module_close (grl_plugin_get_module (plugin));
    grl_plugin_set_module (plugin, NULL);
  }
}

/* ================ PRIVATE API ================ */

/*
 * grl_registry_restrict_plugins:
 * @registry: the registry instance
 * @plugins: a @NULL-terminated array of plugins identifiers
 *
 * Restrict the plugins that application sees to this list.
 *
 * Other plugins will not be available for the application, unless it uses
 * directly #grl_registry_load_plugin() function.
 **/
void
grl_registry_restrict_plugins (GrlRegistry *registry,
                               gchar **plugins)
{
  g_return_if_fail (GRL_IS_REGISTRY (registry));
  g_return_if_fail (plugins);

  /* Free previous list */
  if (registry->priv->allowed_plugins) {
    g_slist_free_full (registry->priv->allowed_plugins, g_free);
    registry->priv->allowed_plugins = NULL;
  }

  while (*plugins) {
    registry->priv->allowed_plugins = g_slist_prepend (registry->priv->allowed_plugins,
                                                       g_strdup (*plugins));
    plugins++;
  }
}

/*
 * grl_registry_shutdown:
 * @registry: the registry instance
 *
 * Frees all the resources in the registry and the registry itself.
 **/
void
grl_registry_shutdown (GrlRegistry *registry)
{
  GHashTableIter iter;
  GList *each_key;
  GList *related_keys = NULL;
  GrlPlugin *plugin = NULL;
  GrlSource *source = NULL;

  if (registry->priv->plugins) {
    g_hash_table_iter_init (&iter, registry->priv->plugins);
    while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &plugin)) {
      shutdown_plugin (plugin);
    }
    g_clear_pointer (&registry->priv->plugins, g_hash_table_unref);
  }

  if (registry->priv->sources) {
    g_hash_table_iter_init (&iter, registry->priv->sources);
    while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &source)) {
      g_object_unref (source);
    }
    g_clear_pointer (&registry->priv->sources, g_hash_table_unref);
  }

  g_clear_pointer (&registry->priv->ranks, g_hash_table_unref);
  g_clear_pointer (&registry->priv->configs, g_hash_table_unref);

  /* We need to free this table with care. Several keys can be pointing to the
     same value, so we need to ensure that we only free the value once */
  if (registry->priv->related_keys) {
    while (TRUE) {
      g_hash_table_iter_init (&iter, registry->priv->related_keys);
      if (!g_hash_table_iter_next (&iter, NULL, (gpointer *) &related_keys)) {
        break;
      }
      /* This will invalidate the iterator */
      for (each_key = related_keys; each_key; each_key = g_list_next (each_key)) {
        g_hash_table_remove (registry->priv->related_keys, GRLKEYID_TO_POINTER (each_key->data));
      }
      g_list_free (related_keys);
    }
    g_clear_pointer (&registry->priv->related_keys, g_hash_table_unref);
  }

  g_slist_free_full (registry->priv->plugins_dir, (GDestroyNotify) g_free);
  g_slist_free_full (registry->priv->allowed_plugins, (GDestroyNotify) g_free);

  key_id_handler_free (&registry->priv->key_id_handler);
  g_clear_pointer (&registry->priv->system_keys, g_hash_table_unref);

  g_object_unref (registry);
}

/* ================ PUBLIC API ================ */

/**
 * grl_registry_get_default:
 *
 * As the registry is designed to work as a singleton, this
 * method is in charge of creating the only instance or
 * returned it if it is already in memory.
 *
 * Returns: (transfer none): a new or an already created instance of the registry.
 *
 * It is NOT MT-safe
 *
 * Since: 0.2.0
 */
GrlRegistry *
grl_registry_get_default (void)
{
  static GrlRegistry *registry = NULL;

  if (!registry) {
    registry = g_object_new (GRL_TYPE_REGISTRY, NULL);
    g_object_add_weak_pointer (G_OBJECT (registry), (gpointer *) &registry);
  }

  return registry;
}

/**
 * grl_registry_register_source:
 * @registry: the registry instance
 * @plugin: the plugin which owns the source
 * @source: (transfer full): the source to register
 * @error: error return location or @NULL to ignore
 *
 * Register a @source in the @registry with the given @plugin information
 *
 * Returns: %TRUE if success, %FALSE% otherwise.
 *
 * Since: 0.2.0
 */
gboolean
grl_registry_register_source (GrlRegistry *registry,
                              GrlPlugin *plugin,
                              GrlSource *source,
                              GError **error)
{
  gchar *id;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);
  g_return_val_if_fail (GRL_IS_PLUGIN (plugin), FALSE);
  g_return_val_if_fail (GRL_IS_SOURCE (source), FALSE);

  g_object_get (source, "source-id", &id, NULL);
  GRL_DEBUG ("New source available: '%s'", id);

  /* Take ownership of the source */
  g_object_ref_sink (source);
  g_object_unref (source);

  /* Do not free id, since g_hash_table_insert does not copy,
     it will be freed when removed from the hash table */
  g_hash_table_insert (registry->priv->sources, id, source);

  /* Set the plugin as owner of source */
  g_object_set (source, "plugin", plugin, NULL);

  /* Set source rank */
  set_source_rank (registry, source);

  /* Update whether it should be invisible */
  update_source_visibility (registry, source);

  if (!SOURCE_IS_INVISIBLE(source))
    g_signal_emit (registry, registry_signals[SIG_SOURCE_ADDED], 0, source);

  return TRUE;
}

/**
 * grl_registry_unregister_source:
 * @registry: the registry instance
 * @source: the source to unregister
 * @error: error return location or @NULL to ignore
 *
 * Removes the @source from the @registry hash table
 *
 * Returns: %TRUE if success, %FALSE% otherwise.
 *
 * Since: 0.2.0
 */
gboolean
grl_registry_unregister_source (GrlRegistry *registry,
                                GrlSource *source,
                                GError **error)
{
  gchar *id;
  gboolean ret = TRUE;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);
  g_return_val_if_fail (GRL_IS_SOURCE (source), FALSE);

  g_object_get (source, "source-id", &id, NULL);
  GRL_DEBUG ("Unregistering source '%s'", id);

  if (g_hash_table_remove (registry->priv->sources, id)) {
    GRL_DEBUG ("source '%s' is no longer available", id);
    g_signal_emit (registry, registry_signals[SIG_SOURCE_REMOVED], 0, source);
    g_object_unref (source);
  } else {
    GRL_WARNING ("source '%s' not found", id);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_UNREGISTER_SOURCE_FAILED,
                 _("Source with id “%s” was not found"), id);
    ret = FALSE;
  }

  g_free (id);
  return ret;
}

/**
 * grl_registry_add_directory:
 * @registry: the registry instance
 * @path: a path with plugins
 *
 * Set this path as part of default paths to load plugins.
 *
 * Since: 0.2.0
 **/
void
grl_registry_add_directory (GrlRegistry *registry,
                            const gchar *path)
{
  g_return_if_fail (GRL_IS_REGISTRY (registry));
  g_return_if_fail (path);

  /* Use append instead of prepend so plugins are loaded in the same order as
     they were added */
  registry->priv->plugins_dir = g_slist_append (registry->priv->plugins_dir,
                                                g_strdup (path));
  registry->priv->all_plugins_preloaded = FALSE;
}

static GrlPlugin *
grl_registry_prepare_plugin_from_desc (GrlRegistry *registry,
                                       GrlPluginDescriptor *plugin_desc)
{
  GrlPlugin *plugin;

  if (!plugin_desc->init ||
      !plugin_desc->id) {
    GRL_WARNING ("Plugin descriptor is not valid");
    return NULL;
  }

  plugin = g_object_new (GRL_TYPE_PLUGIN, NULL);
  grl_plugin_set_id (plugin, plugin_desc->id);
  grl_plugin_set_filename (plugin, plugin_desc->id);

  grl_plugin_set_load_func (plugin, plugin_desc->init);
  grl_plugin_set_unload_func (plugin, plugin_desc->deinit);
  grl_plugin_set_register_keys_func (plugin, plugin_desc->register_keys);

  /* Insert plugin ID as part of plugin information */
  grl_plugin_set_module_name (plugin, plugin_desc->id);

  return plugin;
}

static GrlPlugin *
grl_registry_prepare_plugin (GrlRegistry *registry,
                             const gchar *library_filename,
                             GError **error)
{
  GModule *module;
  GrlPluginDescriptor *plugin_desc;
  GrlPlugin *plugin;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);

  module = g_module_open (library_filename, G_MODULE_BIND_LOCAL);
  if (!module) {
    GRL_WARNING ("Failed to open module: %s", g_module_error ());
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 _("Failed to load plugin from %s"), library_filename);
    return NULL;
  }

  if (!g_module_symbol (module, "GRL_PLUGIN_DESCRIPTOR", (gpointer) &plugin_desc)) {
    GRL_WARNING ("Plugin descriptor not found in '%s'", library_filename);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 _("Invalid plugin file %s"), library_filename);
    g_module_close (module);
    return NULL;
  }

  if (!plugin_desc->init ||
      !plugin_desc->id) {
    GRL_WARNING ("Plugin descriptor is not valid: '%s'", library_filename);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 _("“%s” is not a valid plugin file"), library_filename);
    g_module_close (module);
    return NULL;
  }

  /* Check if plugin is preloaded; if not, then create one */
  plugin = g_hash_table_lookup (registry->priv->plugins,
                                plugin_desc->id);

  if (plugin) {
    g_module_close (module);
    /* Check if the existent plugin is precisely this same plugin */
    if (g_strcmp0 (grl_plugin_get_filename (plugin), library_filename) == 0) {
      return plugin;
    } else {
      GRL_WARNING ("Plugin '%s' already exists", library_filename);
      g_set_error (error,
                   GRL_CORE_ERROR,
                   GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                   _("Plugin “%s” already exists"), library_filename);
      return NULL;
    }
  }

  /* Check if plugin is allowed */
  if (registry->priv->allowed_plugins &&
      !g_slist_find_custom (registry->priv->allowed_plugins,
                            plugin_desc->id,
                            (GCompareFunc) g_strcmp0)) {
    GRL_DEBUG ("Plugin '%s' not allowed; skipping", plugin_desc->id);
    g_module_close (module);
    return NULL;
  }

  plugin = g_object_new (GRL_TYPE_PLUGIN, NULL);
  grl_plugin_set_desc (plugin, plugin_desc);
  grl_plugin_set_module (plugin, module);
  grl_plugin_set_filename (plugin, library_filename);

  /* Make plugin resident */
  g_module_make_resident (module);

  g_hash_table_insert (registry->priv->plugins, g_strdup (plugin_desc->id), plugin);

  /* Register custom keys */
  grl_plugin_register_keys (plugin);

  return plugin;
}

/**
 * grl_registry_activate_all_plugins:
 * @registry: the registry instace
 *
 * Activate all the plugins loaded.
 *
 * Returns: %TRUE if some plugin has been activated
 *
 * Since: 0.3.0
 **/
gboolean
grl_registry_activate_all_plugins (GrlRegistry *registry)
{
  GList *all_plugins;
  GList *l;
  gboolean plugin_activated = FALSE;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);

  all_plugins = g_hash_table_get_values (registry->priv->plugins);
  for (l = all_plugins; l; l = l->next) {
    GrlPlugin *plugin = l->data;
    plugin_activated |= activate_plugin (registry, plugin, NULL);
  }
  g_list_free (all_plugins);

  return plugin_activated;
}

/**
 * grl_registry_load_plugin:
 * @registry: the registry instance
 * @library_filename: the path to the so file
 * @error: error return location or @NULL to ignore
 *
 * Loads a module from shared object file stored in @path
 *
 * Returns: %TRUE if the module is loaded correctly
 *
 * Since: 0.2.0
 */
gboolean
grl_registry_load_plugin (GrlRegistry *registry,
                          const gchar *library_filename,
                          GError **error)
{
  GrlPlugin *plugin;

  plugin = grl_registry_prepare_plugin (registry, library_filename, error);
  if (!plugin)
    return FALSE;

  return register_keys_plugin (registry, plugin, error);
}

/**
 * grl_registry_load_plugin_from_desc: (skip)
 * @registry: the registry instance
 * @plugin_desc: the #GrlPluginDescriptor for the plugin
 * @error: error return location or @NULL to ignore
 *
 * Loads the grilo plugin defined by @plugin_desc. This is
 * used to load plugins that aren't shared libraries, and are
 * built into applications.
 *
 * <example>
 *   Minimal example for loading a builtin plugin, in C.
 *   <programlisting>
 *     static GrlPluginDescriptor descriptor = {
 *       .plugin_id = "grl-example",
 *       .plugin_init = grl_example_plugin_init,
 *     };
 *
 *     grl_registry_load_plugin_from_desc (registry, &descriptor, &error);
 *   </programlisting>
 * </example>
 *
 * Returns: %TRUE if the plugin is initialised correctly
 *
 * Since: 0.3.0
 */
gboolean
grl_registry_load_plugin_from_desc (GrlRegistry *registry,
                                    GrlPluginDescriptor *plugin_desc,
                                    GError **error)
{
  GrlPlugin *plugin;

  plugin = grl_registry_prepare_plugin_from_desc (registry, plugin_desc);
  if (!plugin)
    return FALSE;

  return register_keys_plugin (registry, plugin, error) &&
         activate_plugin (registry, plugin, error);
}

/**
 * grl_registry_load_plugin_directory:
 * @registry: the registry instance
 * @path: the path to the directory
 * @error: error return location or @NULL to ignore
 *
 * Loads a set of modules from directory in @path which contains
 * a group shared object files.
 *
 * Returns: %TRUE if the directory is valid.
 *
 * Since: 0.2.0
 */
gboolean
grl_registry_load_plugin_directory (GrlRegistry *registry,
                                    const gchar *path,
                                    GError **error)
{
  GDir *dir;
  GError *dir_error = NULL;
  GrlPlugin *plugin;
  const gchar *entry;
  gboolean plugin_loaded = FALSE;
  gchar *filename;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);
  g_return_val_if_fail (path, FALSE);

  dir = g_dir_open (path, 0, &dir_error);
  if (!dir) {
    GRL_WARNING ("Could not open directory '%s': %s",
                 path,
                 dir_error->message);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 _("Invalid path %s"), path);
    g_error_free (dir_error);
    return FALSE;
  }

  while ((entry = g_dir_read_name (dir)) != NULL) {
    filename = g_build_filename (path, entry, NULL);
    if (!g_str_has_suffix (filename, "." G_MODULE_SUFFIX)) {
      g_free (filename);
      continue;
    }

    plugin = grl_registry_prepare_plugin (registry, filename, NULL);
    plugin_loaded |= (plugin != NULL);
    g_free (filename);
  }
  g_dir_close (dir);

  return plugin_loaded;
}

/**
 * grl_registry_load_all_plugins:
 * @registry: the registry instance
 * @activate: %TRUE if plugins must be activated after loading
 * @error: error return location or @NULL to ignore
 *
 * Load all the modules available in the default directory path.
 *
 * The default directory path can be changed through the environment
 * variable %GRL_PLUGIN_PATH and it can contain several paths separated
 * by ":"
 *
 * Returns: %FALSE% is all the configured plugin paths are invalid,
 * %TRUE% otherwise.
 *
 * Since: 0.2.0
 */
gboolean
grl_registry_load_all_plugins (GrlRegistry *registry,
                               gboolean activate,
                               GError **error)
{
  GSList *plugin_dir;
  gboolean loaded_one;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), TRUE);

  /* Preload all plugins */
  if (!registry->priv->all_plugins_preloaded) {
    for (plugin_dir = registry->priv->plugins_dir;
         plugin_dir;
         plugin_dir = g_slist_next (plugin_dir)) {
      grl_registry_load_plugin_directory (registry,
                                          plugin_dir->data,
                                          NULL);
    }
    registry->priv->all_plugins_preloaded = TRUE;
  }

  if (activate) {
    loaded_one = grl_registry_activate_all_plugins (registry);

    if (!loaded_one) {
      g_set_error (error,
                   GRL_CORE_ERROR,
                   GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                   _("All configured plugin paths are invalid"));
    }

    return loaded_one;
  } else {
    return TRUE;
  }
}

/**
 * grl_registry_activate_plugin_by_id:
 * @registry: the registry instance
 * @plugin_id: plugin identifier
 * @error: error return location or @NULL to ignore
 *
 * Activates plugin identified by @plugin_id.
 *
 * Returns: %TRUE if the plugin is loaded correctly
 *
 * Since: 0.3.0
 **/
gboolean
grl_registry_activate_plugin_by_id (GrlRegistry *registry,
                                    const gchar *plugin_id,
                                    GError **error)
{
  GrlPlugin *plugin;
  gboolean is_loaded;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);
  g_return_val_if_fail (plugin_id, FALSE);


  /* Check if plugin is available */
  plugin = g_hash_table_lookup (registry->priv->plugins, plugin_id);
  if (!plugin) {
    GRL_WARNING ("Plugin '%s' not available", plugin_id);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 _("Plugin “%s” not available"), plugin_id);
    return FALSE;
  }

  /* Check if plugin is already loaded */
  g_object_get (plugin, "loaded", &is_loaded, NULL);
  if (is_loaded) {
    GRL_WARNING ("Plugin '%s' is already loaded", plugin_id);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
                 _("Plugin “%s” is already loaded"), plugin_id);
    return FALSE;
  }

  /* activate plugin */
  return activate_plugin (registry, plugin, error);
}

/**
 * grl_registry_lookup_source:
 * @registry: the registry instance
 * @source_id: the id of a source
 *
 * This function will search and retrieve a source given its identifier.
 *
 * Returns: (transfer none): The source found.
 *
 * Since: 0.2.0
 */
GrlSource *
grl_registry_lookup_source (GrlRegistry *registry,
                            const gchar *source_id)
{
  GrlSource *source;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), NULL);
  g_return_val_if_fail (source_id != NULL, NULL);

  source = (GrlSource *) g_hash_table_lookup (registry->priv->sources,
                                              source_id);
  if (source && !SOURCE_IS_INVISIBLE(source))
    return source;
  return NULL;
}

/**
 * grl_registry_get_sources:
 * @registry: the registry instance
 * @ranked: whether the returned list shall be returned ordered by rank
 *
 * This function will return all the available sources in the @registry.
 *
 * If @ranked is %TRUE, the source list will be ordered by rank.
 *
 * Returns: (element-type GrlSource) (transfer container): a #GList of
 * available #GrlSource<!-- -->s. The content of the list should not be
 * modified or freed. Use g_list_free() when done using the list.
 *
 * Since: 0.2.0
 */
GList *
grl_registry_get_sources (GrlRegistry *registry,
                          gboolean ranked)
{
  GHashTableIter iter;
  GList *source_list = NULL;
  GrlSource *current_source;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), NULL);

  g_hash_table_iter_init (&iter, registry->priv->sources);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &current_source)) {
    if (!SOURCE_IS_INVISIBLE(current_source))
      source_list = g_list_prepend (source_list, current_source);
  }

  if (ranked) {
    source_list = g_list_sort (source_list, (GCompareFunc) compare_by_rank);
  }

  return source_list;
}

/**
 * grl_registry_get_sources_by_operations:
 * @registry: the registry instance
 * @ops: a bitwise mangle of the requested operations.
 * @ranked: whether the returned list shall be returned ordered by rank
 *
 * Give an array of all the available sources in the @registry capable of
 * perform the operations requested in @ops.
 *
 * If @ranked is %TRUE, the source list will be ordered by rank.
 *
 * Returns: (element-type GrlSource) (transfer container): a #GList of
 * available #GrlSource<!-- -->s. The content of the list should not be
 * modified or freed. Use g_list_free() when done using the list.
 *
 * Since: 0.2.0
 */
GList *
grl_registry_get_sources_by_operations (GrlRegistry *registry,
                                        GrlSupportedOps ops,
                                        gboolean ranked)
{
  GHashTableIter iter;
  GList *source_list = NULL;
  GrlSource *source;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), NULL);

  g_hash_table_iter_init (&iter, registry->priv->sources);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &source)) {
    GrlSupportedOps source_ops;
    source_ops =
      grl_source_supported_operations (source);
    if ((source_ops & ops) == ops &&
        !SOURCE_IS_INVISIBLE(source)) {
      source_list = g_list_prepend (source_list, source);
    }
  }

  if (ranked) {
    source_list = g_list_sort (source_list, compare_by_rank);
  }

  return source_list;
}

/**
 * grl_registry_lookup_plugin:
 * @registry: the registry instance
 * @plugin_id: the id of a plugin
 *
 * This function will search and retrieve a plugin given its identifier.
 *
 * Returns: (transfer none): The plugin found
 *
 * Since: 0.2.0
 **/
GrlPlugin *
grl_registry_lookup_plugin (GrlRegistry *registry,
                            const gchar *plugin_id)
{
  g_return_val_if_fail (GRL_IS_REGISTRY (registry), NULL);
  g_return_val_if_fail (plugin_id, NULL);

  return (GrlPlugin *) g_hash_table_lookup (registry->priv->plugins,
                                            plugin_id);
}

/**
 * grl_registry_get_plugins:
 * @registry: the registry instance
 * @only_loaded: whether the returned list shall include only loaded plugins
 *
 * This function will return all the available plugins in the @registry.
 *
 * If @only_loaded is %TRUE, the plugin list will contain only plugins that are
 * loaded.
 *
 * Returns: (element-type GrlPlugin) (transfer container): a #GList of
 * available #GrlPlugin<!-- -->s. The content of the list should not be modified
 * or freed. Use g_list_free() when done using the list.
 *
 * Since: 0.2.0
 **/
GList *
grl_registry_get_plugins (GrlRegistry *registry,
                          gboolean only_loaded)
{
  GList *plugin_list = NULL;
  GHashTableIter iter;
  GrlPlugin *current_plugin;
  gboolean is_loaded;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), NULL);

  if (only_loaded) {
    g_hash_table_iter_init (&iter, registry->priv->plugins);
    while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &current_plugin)) {
      g_object_get (current_plugin, "loaded", &is_loaded, NULL);
      if (is_loaded) {
        plugin_list = g_list_prepend (plugin_list, current_plugin);
      }
    }
  } else {
    plugin_list = g_hash_table_get_values (registry->priv->plugins);
  }

  return plugin_list;
}

/**
 * grl_registry_unload_plugin:
 * @registry: the registry instance
 * @plugin_id: the identifier of the plugin
 * @error: error return location or @NULL to ignore
 *
 * Unload from memory a module identified by @plugin_id. This means call the
 * module's deinit function.
 *
 * Returns: %TRUE% on success.
 *
 * Since: 0.2.0
 */
gboolean
grl_registry_unload_plugin (GrlRegistry *registry,
                            const gchar *plugin_id,
                            GError **error)
{
  GrlPlugin *plugin;
  GList *sources = NULL;
  GList *sources_iter;

  GRL_DEBUG ("%s: %s", __FUNCTION__, plugin_id);

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);
  g_return_val_if_fail (plugin_id != NULL, FALSE);

  /* First check the plugin is valid  */
  plugin = g_hash_table_lookup (registry->priv->plugins, plugin_id);
  if (!plugin) {
    GRL_WARNING ("Could not deinit plugin '%s'. Plugin not found.", plugin_id);
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_UNLOAD_PLUGIN_FAILED,
                 _("Plugin not found: “%s”"), plugin_id);
    return FALSE;
  }

  /* Second, shut down any sources spawned by this plugin */
  GRL_DEBUG ("Shutting down sources spawned by '%s'", plugin_id);
  sources = grl_registry_get_sources (registry, FALSE);

  for (sources_iter = sources; sources_iter;
      sources_iter = g_list_next (sources_iter)) {
    GrlSource *source = GRL_SOURCE (sources_iter->data);
    if (grl_source_get_plugin (source) == plugin) {
      grl_registry_unregister_source (registry, source, NULL);
    }
  }
  g_list_free (sources);

  /* Third, shut down the plugin */
  shutdown_plugin (plugin);

  return TRUE;
}

/**
 * grl_registry_register_metadata_key:
 * @registry: The plugin registry
 * @param_spec: (transfer full): The definition of the key to register
 * @bind_key: The key the new key is bind to, or #GRL_METADATA_KEY_INVALID if it is not bound.
 * @error: error return location or @NULL to ignore
 *
 * Registers a new metadata key, creating a relation between the new key and
 * @bind_key.
 *
 * Two keys are related when the values of both keys are somehow related.
 *
 * One example of a relation would be the one between the URI of a media
 * resource and its mime-type: they are both tied together and one does not make
 * sense without the other.
 *
 * Relations between keys allow the framework to provide all the data that is
 * somehow related when any of the related keys are requested.

 * Returns: The #GrlKeyID registered.
 *
 * Since: 0.3.0
 */
GrlKeyID
grl_registry_register_metadata_key (GrlRegistry *registry,
                                    GParamSpec *param_spec,
                                    GrlKeyID bind_key,
                                    GError **error)
{
  GrlKeyID key;

  key = grl_registry_register_metadata_key_full (registry,
                                                 param_spec,
                                                 GRL_METADATA_KEY_INVALID,
                                                 bind_key,
                                                 error);

  if (key != GRL_METADATA_KEY_INVALID) {
    g_signal_emit (registry, registry_signals[SIG_METADATA_KEY_ADDED],
                   0,
                   grl_metadata_key_get_name (key));
  }

  return key;
}

/*
 * grl_registry_register_metadata_key_system:
 *
 * This is an internal method only meant to be used to register core
 * keys.
 *
 * For internal use. Plugin developers should use
 * grl_registry_register_metadata_key().
 */
GrlKeyID
grl_registry_register_metadata_key_system (GrlRegistry *registry,
                                           GParamSpec *param_spec,
                                           GrlKeyID key,
                                           GrlKeyID bind_key,
                                           GError **error)
{
  GrlKeyID registered_key;

  registered_key = grl_registry_register_metadata_key_full (registry,
                                                            param_spec,
                                                            key,
                                                            bind_key,
                                                            error);

  return registered_key;
}

/**
 * grl_registry_lookup_metadata_key:
 * @registry: the registry instance
 * @key_name: the key name
 *
 * Look up for the metadata key with name @key_name.
 *
 * Returns: The metadata key, or GRL_METADATA_KEY_INVALID if not found
 *
 * Since: 0.2.0
 */
GrlKeyID
grl_registry_lookup_metadata_key (GrlRegistry *registry,
                                  const gchar *key_name)
{
  g_return_val_if_fail (GRL_IS_REGISTRY (registry), 0);
  g_return_val_if_fail (key_name, 0);

  return key_id_handler_get_key (&registry->priv->key_id_handler, key_name);
}

/**
 * grl_registry_lookup_metadata_key_name:
 * @registry: the registry instance
 * @key: a metadata key
 *
 * Returns @key name.
 *
 * Returns: metadata key name, or @NULL if not found
 *
 * Since: 0.2.0
 */
const gchar *
grl_registry_lookup_metadata_key_name (GrlRegistry *registry,
                                       GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_REGISTRY (registry), 0);

  return key_id_handler_get_name (&registry->priv->key_id_handler, key);
}

/**
 * grl_registry_lookup_metadata_key_desc:
 * @registry: the registry instance
 * @key: a metadata key
 *
 * Returns @key description.
 *
 * Returns: metadata key description, or @NULL if not found
 *
 * Since: 0.2.0
 */
const gchar *
grl_registry_lookup_metadata_key_desc (GrlRegistry *registry,
                                       GrlKeyID key)
{
  const gchar *key_name;
  GParamSpec *key_pspec;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), 0);

  key_name = key_id_handler_get_name (&registry->priv->key_id_handler, key);
  if (!key_name) {
    return NULL;
  }
  key_pspec = g_hash_table_lookup (registry->priv->system_keys, key_name);

  if (key_pspec) {
    return g_param_spec_get_blurb (key_pspec);
  } else {
    return NULL;
  }
}

/**
 * grl_registry_lookup_metadata_key_type:
 * @registry: the registry instance
 * @key: a metadata key
 *
 * Returns @key expected value type.
 *
 * Returns: metadata key type, or @G_TYPE_INVALID if not found
 *
 * Since: 0.2.0
 */
GType
grl_registry_lookup_metadata_key_type (GrlRegistry *registry,
                                       GrlKeyID key)
{
  const gchar *key_name;
  GParamSpec *key_pspec;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), 0);

  key_name = key_id_handler_get_name (&registry->priv->key_id_handler, key);
  if (!key_name) {
    return G_TYPE_INVALID;
  }
  key_pspec = g_hash_table_lookup (registry->priv->system_keys, key_name);

  if (key_pspec) {
    return G_PARAM_SPEC_VALUE_TYPE (key_pspec);
  } else {
    return G_TYPE_INVALID;
  }
}

/**
 * grl_registry_metadata_key_validate:
 * @registry: the registry instance
 * @key: a metadata key
 * @value: value to be validate
 *
 * Validates @value content complies with the key specification. That is, it has
 * the expected type, and value are within the range specified in key (for
 * integer values).
 *
 * Returns: %TRUE if complies
 *
 * Since: 0.2.0
 **/
gboolean
grl_registry_metadata_key_validate (GrlRegistry *registry,
                                    GrlKeyID key,
                                    GValue *value)
{
  const gchar *key_name;
  GParamSpec *key_pspec;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);
  g_return_val_if_fail (G_IS_VALUE (value), FALSE);

  key_name = key_id_handler_get_name (&registry->priv->key_id_handler, key);
  if (!key_name) {
    return FALSE;
  }
  key_pspec = g_hash_table_lookup (registry->priv->system_keys, key_name);

  if (key_pspec) {
    return !g_param_value_validate (key_pspec, value);
  } else {
    return FALSE;
  }
}

/**
 * grl_registry_lookup_metadata_key_relation:
 * @registry: the registry instance
 * @key: a metadata key
 *
 * Look up the list of keys that have a relation with @key.
 *
 * @key is included in that list.
 *
 * Returns: (element-type GrlKeyID) (transfer none): a #GList of
 * related keys, or @NULL if key is invalid.
 *
 * Since: 0.2.0
 **/
const GList *
grl_registry_lookup_metadata_key_relation (GrlRegistry *registry,
                                           GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_REGISTRY (registry), NULL);

  return g_hash_table_lookup (registry->priv->related_keys, GRLKEYID_TO_POINTER (key));
}

/**
 * grl_registry_get_metadata_keys:
 * @registry: the registry instance
 *
 * Returns a list with all registered keys in system.
 *
 * Returns: (transfer container) (element-type GrlKeyID): a #GList with all the available
 * #GrlKeyID<!-- -->s. The content of the list should not be modified or freed.
 * Use g_list_free() when done using the list.
 *
 * Since: 0.2.0
 **/
GList *
grl_registry_get_metadata_keys (GrlRegistry *registry)
{
  return key_id_handler_get_all_keys (&registry->priv->key_id_handler);
}

/**
 * grl_registry_add_config:
 * @registry: the registry instance
 * @config: (transfer full): a configuration set
 * @error: error return location or @NULL to ignore
 *
 * Add a configuration for a plugin/source.
 *
 * Returns: %TRUE on success
 *
 * Since: 0.2.0
 */
gboolean
grl_registry_add_config (GrlRegistry *registry,
                         GrlConfig *config,
                         GError **error)
{
  gchar *plugin_id;
  GList *configs = NULL;

  g_return_val_if_fail (config != NULL, FALSE);
  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);

  plugin_id = grl_config_get_plugin (config);
  if (!plugin_id) {
    GRL_WARNING ("Plugin configuration missed plugin information, ignoring...");
    g_set_error (error,
                 GRL_CORE_ERROR,
                 GRL_CORE_ERROR_CONFIG_FAILED,
                 _("Plugin configuration does not contain “plugin-id” reference"));
    return FALSE;
  }

  configs = g_hash_table_lookup (registry->priv->configs, plugin_id);
  if (configs) {
    /* Notice that we are using g_list_append on purpose to avoid
       having to insert again in the hash table */
    configs = g_list_append (configs, config);
    g_free (plugin_id);
  } else {
    configs = g_list_prepend (configs, config);
    g_hash_table_insert (registry->priv->configs,
			 (gpointer) plugin_id,
			 configs);
  }

  return TRUE;
}

static void
get_plugin_and_source_from_group_name (const gchar  *group_name,
                                       gchar       **plugin_name,
                                       gchar       **source_name)
{
  gchar *plugin = g_strdup (group_name);
  gchar **arr;

  *plugin_name = *source_name = NULL;

  plugin = g_strstrip (plugin);
  arr = g_strsplit (plugin, " ", 2);
  g_free (plugin);

  *plugin_name = g_strstrip (arr[0]);

  if (arr[1] != NULL)
    *source_name = g_strstrip (arr[1]);

  g_free (arr);
}

static void
add_config_from_keyfile (GKeyFile    *keyfile,
			 GrlRegistry *registry)
{
  GrlConfig *config;
  gchar **key;
  gchar **keys;
  gchar **groupname;
  gchar **plugins;
  gchar *value;

  /* Look up for defined plugins */
  plugins = g_key_file_get_groups (keyfile, NULL);
  for (groupname = plugins; *groupname; groupname++) {
    gchar *plugin_name, *source_name;

    get_plugin_and_source_from_group_name (*groupname, &plugin_name, &source_name);

    config = grl_config_new (plugin_name, source_name);

    /* Look up configuration keys for this plugin */
    keys = g_key_file_get_keys (keyfile, *groupname, NULL, NULL);
    for (key = keys; *key; key++) {
      value = g_key_file_get_string (keyfile, *groupname, *key, NULL);
      if (value) {
        GRL_DEBUG ("Config found: %s : %s : %s", plugin_name,
                   source_name != NULL ? source_name : plugin_name,
                   *key);
        grl_config_set_string (config, *key, value);
        g_free (value);
      }
    }
    grl_registry_add_config (registry, config, NULL);
    g_strfreev (keys);
    g_free (source_name);
    g_free (plugin_name);
  }
  g_strfreev (plugins);
}

/**
 * grl_registry_add_config_from_file:
 * @registry: the registry instance
 * @config_file: a key-value file containing the configuration
 * @error: error return location or @NULL to ignore
 *
 * Load plugin configurations from a .ini-like config file.
 *
 * Returns: %TRUE on success
 *
 * Since: 0.2.0
 **/
gboolean
grl_registry_add_config_from_file (GrlRegistry *registry,
                                   const gchar *config_file,
                                   GError **error)
{
  GError *load_error = NULL;
  GKeyFile *keyfile;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);
  g_return_val_if_fail (config_file, FALSE);

  keyfile = g_key_file_new ();

  if (g_key_file_load_from_file (keyfile,
                                 config_file,
                                 G_KEY_FILE_NONE,
                                 &load_error)) {
    add_config_from_keyfile (keyfile, registry);
    g_key_file_free (keyfile);
    return TRUE;
  } else {
    GRL_WARNING ("Unable to load configuration. %s", load_error->message);
    g_set_error_literal (error,
                         GRL_CORE_ERROR,
                         GRL_CORE_ERROR_CONFIG_LOAD_FAILED,
                         load_error->message);
    g_error_free (load_error);
    g_key_file_free (keyfile);
    return FALSE;
  }
}

/**
 * grl_registry_add_config_from_resource:
 * @registry: the registry instance
 * @resource_path: a key-value file containing the configuration
 * @error: error return location or @NULL to ignore
 *
 * Load plugin configurations from a .ini-like resource file.
 *
 * Returns: %TRUE on success
 *
 * Since: 0.2.8
 **/
gboolean
grl_registry_add_config_from_resource (GrlRegistry *registry,
                                       const gchar *resource_path,
                                       GError **error)
{
  GError *load_error = NULL;
  GKeyFile *keyfile = NULL;
  GBytes *bytes;
  gboolean ret = FALSE;

  g_return_val_if_fail (GRL_IS_REGISTRY (registry), FALSE);
  g_return_val_if_fail (resource_path, FALSE);

  bytes = g_resources_lookup_data (resource_path, 0, error);
  if (bytes == NULL)
    goto bail;

  keyfile = g_key_file_new ();

  if (g_key_file_load_from_data (keyfile,
                                 g_bytes_get_data (bytes, NULL),
                                 g_bytes_get_size (bytes),
                                 G_KEY_FILE_NONE,
                                 &load_error)) {
    add_config_from_keyfile (keyfile, registry);
    ret = TRUE;
  } else {
    GRL_WARNING ("Unable to load configuration. %s", load_error->message);
    g_set_error_literal (error,
                         GRL_CORE_ERROR,
                         GRL_CORE_ERROR_CONFIG_LOAD_FAILED,
                         load_error->message);
    g_error_free (load_error);
  }

bail:
  g_clear_pointer (&keyfile, g_key_file_free);
  g_clear_pointer (&bytes, g_bytes_unref);

  return ret;
}


G_GNUC_INTERNAL gboolean
grl_registry_metadata_key_get_limits(GrlRegistry *registry,
                                     GrlKeyID key,
                                     GValue *min,
                                     GValue *max)
{
  GParamSpec *key_pspec;
  const gchar *key_name;
  GType key_type;

  key_name = key_id_handler_get_name (&registry->priv->key_id_handler, key);
  if (!key_name) {
    return FALSE;
  }

  key_pspec = g_hash_table_lookup (registry->priv->system_keys, key_name);
  if (!key_pspec) {
    return FALSE;
  }

  key_type = G_PARAM_SPEC_VALUE_TYPE (key_pspec);

#define CHECK_NUMERIC_AND_SET_VALUE_LIMITS(value_type, numeric_type, getter, cast_type) { \
  if (value_type == numeric_type) {                                                       \
    g_value_init (min, numeric_type);                                                     \
    g_value_init (max, numeric_type);                                                     \
    g_value_set_##getter (min, (cast_type (key_pspec))->minimum);                         \
    g_value_set_##getter (max, (cast_type (key_pspec))->maximum);                         \
    return TRUE;                                                                          \
  }                                                                                       \
}

  CHECK_NUMERIC_AND_SET_VALUE_LIMITS (key_type, G_TYPE_INT, int, G_PARAM_SPEC_INT);
  CHECK_NUMERIC_AND_SET_VALUE_LIMITS (key_type, G_TYPE_LONG, long, G_PARAM_SPEC_LONG);
  CHECK_NUMERIC_AND_SET_VALUE_LIMITS (key_type, G_TYPE_INT64, int64, G_PARAM_SPEC_INT64);
  CHECK_NUMERIC_AND_SET_VALUE_LIMITS (key_type, G_TYPE_CHAR, schar, G_PARAM_SPEC_CHAR);
  CHECK_NUMERIC_AND_SET_VALUE_LIMITS (key_type, G_TYPE_UINT, uint, G_PARAM_SPEC_UINT);
  CHECK_NUMERIC_AND_SET_VALUE_LIMITS (key_type, G_TYPE_ULONG, ulong, G_PARAM_SPEC_ULONG);
  CHECK_NUMERIC_AND_SET_VALUE_LIMITS (key_type, G_TYPE_UINT64, uint64, G_PARAM_SPEC_UINT64);
  CHECK_NUMERIC_AND_SET_VALUE_LIMITS (key_type, G_TYPE_UCHAR, uchar, G_PARAM_SPEC_UCHAR);
  CHECK_NUMERIC_AND_SET_VALUE_LIMITS (key_type, G_TYPE_FLOAT, float, G_PARAM_SPEC_FLOAT);
  CHECK_NUMERIC_AND_SET_VALUE_LIMITS (key_type, G_TYPE_DOUBLE, double, G_PARAM_SPEC_DOUBLE);
  return FALSE;
}

/* @max and @min are expected to be initialized with G_VALUE_INIT (non null)
 * Returns TRUE if @value has changed
 */
G_GNUC_INTERNAL gboolean
grl_registry_metadata_key_clamp(GrlRegistry *registry,
                                GrlKeyID key,
                                GValue *min,
                                GValue *value,
                                GValue *max)
{
  const gchar *key_name;

  g_return_val_if_fail (min != NULL, FALSE);
  g_return_val_if_fail (max != NULL, FALSE);

  if (value == NULL) {
    return FALSE;
  }

  key_name = key_id_handler_get_name (&registry->priv->key_id_handler, key);
  if (key_name) {
    GParamSpec *key_pspec;

    key_pspec = g_hash_table_lookup (registry->priv->system_keys, key_name);
    if (key_pspec) {
      if (g_param_values_cmp(key_pspec, value, min) < 0) {
        GRL_DEBUG("reset value to min");
        g_value_transform(min, value);
        return TRUE;
      } else if (g_param_values_cmp(key_pspec, value, max) > 0) {
        GRL_DEBUG("reset value to max");
        g_value_transform(max, value);
        return TRUE;
      }
    }
  }
  return FALSE;
}

/*
 * Check if the values are valid.
 * If max < min we return False.
 */
G_GNUC_INTERNAL gboolean
grl_registry_metadata_key_is_max_valid(GrlRegistry *registry,
                                       GrlKeyID key,
                                       GValue *min,
                                       GValue *max)
{
  const gchar *key_name;

  if (min == NULL || max == NULL) {
    return TRUE;
  }

  key_name = key_id_handler_get_name (&registry->priv->key_id_handler, key);
  if (key_name) {
    GParamSpec *key_pspec;

    key_pspec = g_hash_table_lookup (registry->priv->system_keys, key_name);
    if (key_pspec) {
      if (g_param_values_cmp(key_pspec, max, min) < 0) {
        GRL_DEBUG("Max value not valid: max < min");

        return FALSE;
      }
    }
  }
  return TRUE;
}
