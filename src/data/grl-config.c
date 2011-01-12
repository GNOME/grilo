/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Juan A. Suarez Romero <jasuarez@igalia.com>
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
 * SECTION:grl-config
 * @short_description: Configuration data storage
 *
 * This class is used to store configuration settings used by plugins.
 */

#include "grl-config.h"
#include "grl-log.h"

#define GRL_LOG_DOMAIN_DEFAULT  config_log_domain
GRL_LOG_DOMAIN(config_log_domain);

#define GRL_CONFIG_GET_PRIVATE(o)                                         \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRL_TYPE_CONFIG, GrlConfigPrivate))

struct _GrlConfigPrivate {
  GHashTable *config;
};

static void grl_config_dispose (GObject *object);
static void grl_config_finalize (GObject *object);

G_DEFINE_TYPE (GrlConfig, grl_config, G_TYPE_OBJECT);

static void
free_val (GValue *val)
{
  if (val) {
    g_value_unset (val);
    g_free (val);
  }
}

static void
grl_config_class_init (GrlConfigClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_config_dispose;
  gobject_class->finalize = grl_config_finalize;

  g_type_class_add_private (klass, sizeof (GrlConfigPrivate));
}

static void
grl_config_init (GrlConfig *self)
{
  self->priv = GRL_CONFIG_GET_PRIVATE (self);
  self->priv->config = g_hash_table_new_full (g_str_hash,
					      g_str_equal,
					      g_free,
					      (GDestroyNotify) free_val);
}

static void
grl_config_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_config_parent_class)->dispose (object);
}

static void
grl_config_finalize (GObject *object)
{
  GRL_DEBUG ("grl_config_finalize");
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_config_parent_class)->finalize (object);
}

/**
 * grl_config_new:
 * @plugin: plugin id for this configuration
 * @source: (allow-none): source id for this configuration
 *
 * Creates a new data config object that will be associated with a plugin
 * (if @source is NULL), or a specific source spawned from a plugin (if
 * @source is not NULL). The latter may be useful for plugins
 * spawning various sources, each one needing a different configuration.
 *
 * Returns: (transfer none): a newly-allocated data config. The data
 * config associated with the plugin should not be freed until the plugin
 * has been unloaded.
 *
 * Since: 0.1.4
 */
GrlConfig *
grl_config_new (const gchar *plugin, const gchar *source)
{
  g_return_val_if_fail (plugin != NULL, NULL);
  GrlConfig *config = g_object_new (GRL_TYPE_CONFIG, NULL);
  if (plugin) {
    grl_config_set_string (config, GRL_CONFIG_KEY_PLUGIN, plugin);
  }
  if (source) {
    grl_config_set_source (config, source);
  }
  return config;
}

void
grl_config_set (GrlConfig *config, const gchar *param, const GValue *value)
{
  GValue *copy;
  g_return_if_fail (GRL_IS_CONFIG (config));
  copy = g_new0 (GValue, 1);
  g_value_init (copy, G_VALUE_TYPE (value));
  g_value_copy (value, copy);
  g_hash_table_insert (config->priv->config, g_strdup (param), copy);
}

void
grl_config_set_string (GrlConfig *config, const gchar *param, const gchar *value)
{
  GValue v = { 0 };
  g_value_init (&v, G_TYPE_STRING);
  g_value_set_string (&v, value);
  grl_config_set (config, param, &v);
  g_value_unset (&v);
}

void
grl_config_set_int (GrlConfig *config, const gchar *param, gint value)
{
  GValue v = { 0 };
  g_value_init (&v, G_TYPE_INT);
  g_value_set_int (&v, value);
  grl_config_set (config, param, &v);
}


void
grl_config_set_float (GrlConfig *config, const gchar *param, gfloat value)
{
  GValue v = { 0 };
  g_value_init (&v, G_TYPE_FLOAT);
  g_value_set_float (&v, value);
  grl_config_set (config, param, &v);
}

void
grl_config_set_boolean (GrlConfig *config, const gchar *param, gboolean value)
{
  GValue v = { 0 };
  g_value_init (&v, G_TYPE_BOOLEAN);
  g_value_set_boolean (&v, value);
  grl_config_set (config, param, &v);
}

const GValue *
grl_config_get (GrlConfig *config, const gchar *param)
{
  g_return_val_if_fail (GRL_IS_CONFIG (config), NULL);
  return g_hash_table_lookup (config->priv->config, param);
}

const gchar *
grl_config_get_string (GrlConfig *config, const gchar *param)
{
  g_return_val_if_fail (GRL_IS_CONFIG (config), NULL);
  const GValue *value = grl_config_get (config, param);
  if (!value || !G_VALUE_HOLDS_STRING (value)) {
    return NULL;
  } else {
    return g_value_get_string (value);
  }
}

gint
grl_config_get_int (GrlConfig *config, const gchar *param)
{
  g_return_val_if_fail (GRL_IS_CONFIG (config), 0);
  const GValue *value = grl_config_get (config, param);
  if (!value || !G_VALUE_HOLDS_INT (value)) {
    return 0;
  } else {
    return g_value_get_int (value);
  }
}

gfloat
grl_config_get_float (GrlConfig *config, const gchar *param)
{
  g_return_val_if_fail (GRL_IS_CONFIG (config), 0.0);
  const GValue *value = grl_config_get (config, param);
  if (!value || !G_VALUE_HOLDS_FLOAT (value)) {
    return 0.0;
  } else {
    return g_value_get_float (value);
  }
}

gboolean
grl_config_get_boolean (GrlConfig *config, const gchar *param)
{
  g_return_val_if_fail (GRL_IS_CONFIG (config), FALSE);
  const GValue *value = grl_config_get (config, param);
  if (!value || !G_VALUE_HOLDS_BOOLEAN (value)) {
    return FALSE;
  } else {
    return g_value_get_boolean (value);
  }
}

/**
 * grl_config_set_plugin:
 * @config: the config instance
 * @plugin: the plugin id
 *
 * Set the plugin key in the configuration
 *
 * Since: 0.1.4
 */
void
grl_config_set_plugin (GrlConfig *config, const gchar *plugin)
{
  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_PLUGIN,
                         plugin);
}

/**
 * grl_config_set_source:
 * @config: the config instance
 * @source: the source id
 *
 * Set the plugin key in the configuration
 *
 * Since: 0.1.4
 */
void
grl_config_set_source (GrlConfig *config, const gchar *source)
{
  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_SOURCE,
                         source);
}

/**
 * grl_config_set_api_key:
 * @config: the config instance
 * @key: the API key
 *
 * Set the webservice API key in the configuration
 *
 * Since: 0.1.4
 */
void
grl_config_set_api_key (GrlConfig *config, const gchar *key)
{
  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_APIKEY,
                         key);
}

/**
 * grl_config_set_api_token:
 * @config: the config instance
 * @token: the API token
 *
 * Set the webservice API token in the configuration
 *
 * Since: 0.1.4
 */
void
grl_config_set_api_token (GrlConfig *config, const gchar *token)
{
  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_APITOKEN,
                         token);
}

/**
 * grl_config_set_api_secret:
 * @config: the config instance
 * @secret: the webservice passphrase
 *
 * Set the webservice passphrase in the configuration
 *
 * Since: 0.1.4
 */
void
grl_config_set_api_secret (GrlConfig *config, const gchar *secret)
{
  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_APISECRET,
                         secret);
}

/**
 * grl_config_set_username:
 * @config: the config instance
 * @username: the username
 *
 * Set the username in the configuration
 */
void
grl_config_set_username (GrlConfig *config, const gchar *username)
{
  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_USERNAME,
                         username);
}

/**
 * grl_config_set_password:
 * @config: the config instance
 * @password: the password
 *
 * Set the password in the configuration
 */
void
grl_config_set_password(GrlConfig *config, const gchar *password)
{
  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_PASSWORD,
                         password);
}

/**
 * grl_config_get_plugin:
 * @config: the config instance
 *
 * Returns: the plugin id
 *
 * Since: 0.1.4
 */
const gchar *
grl_config_get_plugin (GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_PLUGIN);
}

/**
 * grl_config_get_api_key:
 * @config: the config instance
 *
 * Returns: the webservice API key
 *
 * Since: 0.1.4
 */
const gchar *
grl_config_get_api_key (GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_APIKEY);
}

/**
 * grl_config_get_api_token:
 * @config: the config instance
 *
 * Returns: the webservice API token
 *
 * Since: 0.1.4
 */
const gchar *
grl_config_get_api_token (GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_APITOKEN);
}

/**
 * grl_config_get_api_secret:
 * @config: the config instance
 *
 * Returns: the webservice API passphrase
 *
 * Since: 0.1.4
 */
const gchar *
grl_config_get_api_secret (GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_APISECRET);
}

/**
 * grl_config_get_username:
 * @config: the config instance
 *
 * Returns: the username
 */
const gchar *
grl_config_get_username (GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_USERNAME);
}

/**
 * grl_config_get_password:
 * @config: the config instance
 *
 * Returns: the password
 */
const gchar *
grl_config_get_password(GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_PASSWORD);
}
