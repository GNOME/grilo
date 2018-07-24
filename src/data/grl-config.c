/*
 * Copyright (C) 2010, 2011 Igalia S.L.
 * Copyright (C) 2011 Intel Corporation.
 *
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
 * This class is used to configure plugins during a session.
 *
 * Pre-defined settings are read from plugin specific configuration files.
 * The settings can be changed to properly setup the session, but are not
 * persistent. Changes are local to each #GrlConfig instance.
 */

#include "grl-config.h"
#include "grl-log.h"

#define GROUP_NAME "none"

#define GRL_LOG_DOMAIN_DEFAULT  config_log_domain
GRL_LOG_DOMAIN(config_log_domain);

struct _GrlConfigPrivate {
  GKeyFile *config;
};

static void grl_config_finalize (GObject *object);

G_DEFINE_TYPE_WITH_PRIVATE (GrlConfig, grl_config, G_TYPE_OBJECT);

static void
grl_config_class_init (GrlConfigClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->finalize = grl_config_finalize;
}

static void
grl_config_init (GrlConfig *self)
{
  self->priv = grl_config_get_instance_private (self);
  self->priv->config = g_key_file_new ();

  g_key_file_load_from_data (self->priv->config, "[]\n", -1, G_KEY_FILE_NONE, NULL);
}

static void
grl_config_finalize (GObject *object)
{
  GrlConfig *self = GRL_CONFIG (object);

  GRL_DEBUG ("grl_config_finalize");

  g_key_file_free (self->priv->config);
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
  GrlConfig *config;

  g_return_val_if_fail (plugin != NULL, NULL);

  config = g_object_new (GRL_TYPE_CONFIG, NULL);
  grl_config_set_string (config, GRL_CONFIG_KEY_PLUGIN, plugin);
  if (source) {
    grl_config_set_source (config, source);
  }
  return config;
}

/**
 * grl_config_set:
 * @config: the config instance
 * @param: a parameter
 * @value: value
 *
 * Set @param @value.
 *
 * Since: 0.1.5
 **/
void
grl_config_set (GrlConfig *config, const gchar *param, const GValue *value)
{
  GByteArray *array;
  gchar *encoded;

  g_return_if_fail (GRL_IS_CONFIG (config));
  g_return_if_fail (param != NULL);

  switch (G_VALUE_TYPE (value)) {
  case G_TYPE_STRING:
    g_key_file_set_string (config->priv->config, GROUP_NAME, param,
                           g_value_get_string (value));
    break;

  case G_TYPE_FLOAT:
    g_key_file_set_double (config->priv->config, GROUP_NAME, param,
                           g_value_get_double (value));
    break;

  case G_TYPE_INT:
    g_key_file_set_integer (config->priv->config, GROUP_NAME, param,
                            g_value_get_int (value));
    break;

  case G_TYPE_BOOLEAN:
    g_key_file_set_boolean (config->priv->config, GROUP_NAME, param,
                            g_value_get_boolean (value));
    break;

  case G_TYPE_BOXED:
    array = g_value_get_boxed(value);
    encoded = g_base64_encode ((const guchar *) array, array->len);
    g_key_file_set_string (config->priv->config, GROUP_NAME, param,
                           encoded);
    g_free (encoded);
    break;

  default:
    g_return_if_reached ();
    break;
  }
}

/**
 * grl_config_set_string:
 * @config: the config instance
 * @param: a string type parameter
 * @value: a value
 *
 * Set @param @value.
 *
 * Since: 0.1.5
 **/
void
grl_config_set_string (GrlConfig *config, const gchar *param, const gchar *value)
{
  g_return_if_fail (GRL_IS_CONFIG (config));
  g_key_file_set_string (config->priv->config, GROUP_NAME, param, value);
}

/**
 * grl_config_set_int:
 * @config: the config instance
 * @param: an integer type parameter
 * @value: a value
 *
 * Set @param @value.
 *
 * Since: 0.1.5
 **/
void
grl_config_set_int (GrlConfig *config, const gchar *param, gint value)
{
  g_return_if_fail (GRL_IS_CONFIG (config));
  g_key_file_set_integer (config->priv->config, GROUP_NAME, param, value);
}


/**
 * grl_config_set_float:
 * @config: the config instance
 * @param: a float type parameter
 * @value: a value
 *
 * Set @param @value.
 *
 * Since: 0.1.5
 **/
void
grl_config_set_float (GrlConfig *config, const gchar *param, gfloat value)
{
  g_return_if_fail (GRL_IS_CONFIG (config));
  g_key_file_set_double (config->priv->config, GROUP_NAME, param, (gdouble) value);
}

/**
 * grl_config_set_boolean:
 * @config: the config instance
 * @param: a boolean type parameter
 * @value: a value
 *
 * Set @param @value.
 *
 * Since: 0.1.8
 **/
void
grl_config_set_boolean (GrlConfig *config, const gchar *param, gboolean value)
{
  g_return_if_fail (GRL_IS_CONFIG (config));
  g_key_file_set_boolean (config->priv->config, GROUP_NAME, param, value);
}

/**
 * grl_config_set_binary:
 * @config: the config instance
 * @param: a binary type parameter
 * @blob: a base64 encoded binary value
 * @size: size of @value
 *
 * Set @param value.
 *
 * Since: 0.1.9
 **/
void
grl_config_set_binary (GrlConfig *config, const gchar *param, const guint8 *blob, gsize size)
{
  gchar *encoded;

  g_return_if_fail (GRL_IS_CONFIG (config));

  encoded = g_base64_encode (blob, size);
  g_key_file_set_string (config->priv->config, GROUP_NAME, param, encoded);
  g_free (encoded);
}

/**
 * grl_config_get_string:
 * @config: the config instance
 * @param: a string type paramter
 *
 * Returns: @param value
 *
 * Since: 0.1.5
 **/
gchar *
grl_config_get_string (GrlConfig *config, const gchar *param)
{
  g_return_val_if_fail (GRL_IS_CONFIG (config), NULL);
  return g_key_file_get_string (config->priv->config, GROUP_NAME, param, NULL);
}

/**
 * grl_config_get_int:
 * @config: the config instance
 * @param: an integer type parameter
 *
 * Returns: @param value
 *
 * Since: 0.1.5
 **/
gint
grl_config_get_int (GrlConfig *config, const gchar *param)
{
  g_return_val_if_fail (GRL_IS_CONFIG (config), 0);
  return g_key_file_get_integer (config->priv->config, GROUP_NAME, param, NULL);
}

/**
 * grl_config_get_float:
 * @config: the config instance
 * @param: a float type parameter
 *
 * Returns: @param value
 *
 * Since: 0.1.5
 **/
gfloat
grl_config_get_float (GrlConfig *config, const gchar *param)
{
  g_return_val_if_fail (GRL_IS_CONFIG (config), 0.0);
  return (gfloat) g_key_file_get_double (config->priv->config, GROUP_NAME,
                                         param, NULL);
}

/**
 * grl_config_get_boolean:
 * @config: the config instance
 * @param: a boolean type parameter
 *
 * Returns: @param value
 *
 * Since: 0.1.8
 **/
gboolean
grl_config_get_boolean (GrlConfig *config, const gchar *param)
{
  g_return_val_if_fail (GRL_IS_CONFIG (config), FALSE);
  return g_key_file_get_boolean (config->priv->config, GROUP_NAME, param, NULL);
}

/**
 * grl_config_get_binary:
 * @config: the config instance
 * @param: a binary type parameter
 * @size: (allow-none): place for size of value
 *
 * Gets the value of @param encoded as base64. If @size is not %NULL, it puts
 * there the size of the value.
 *
 * Returns: @param value
 *
 * Since: 0.1.9
 **/
guint8 *
grl_config_get_binary (GrlConfig *config, const gchar *param, gsize *size)
{
  gchar *encoded;
  gsize s;
  guint8 *binary;

  g_return_val_if_fail (GRL_IS_CONFIG (config), NULL);

  encoded = g_key_file_get_string (config->priv->config, GROUP_NAME, param, NULL);
  if (!encoded) {
    return NULL;
  }

  binary = g_base64_decode (encoded, &s);
  g_free (encoded);
  if (size) {
    *size = s;
  }

  return binary;
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
  g_return_if_fail (GRL_IS_CONFIG (config));
  g_return_if_fail (plugin != NULL);

  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_PLUGIN,
                         plugin);
}

/**
 * grl_config_set_source:
 * @config: the config instance
 * @source: the source id
 *
 * Set the source key in the configuration
 *
 * Since: 0.1.4
 */
void
grl_config_set_source (GrlConfig *config, const gchar *source)
{
  g_return_if_fail (GRL_IS_CONFIG (config));

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
  g_return_if_fail (GRL_IS_CONFIG (config));

  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_APIKEY,
                         key);
}

/**
 * grl_config_set_api_key_blob:
 * @config: the config instance
 * @blob: the binary API key blob
 * @size: the size of the blob
 *
 * Set the binary API key in the configuration
 *
 * Since: 0.1.9
 */
void
grl_config_set_api_key_blob (GrlConfig *config, const guint8 *blob, gsize size)
{
  g_return_if_fail (GRL_IS_CONFIG (config));

  grl_config_set_binary (config, GRL_CONFIG_KEY_APIKEY_BLOB, blob, size);
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
  g_return_if_fail (GRL_IS_CONFIG (config));

  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_APITOKEN,
                         token);
}

/**
 * grl_config_set_api_token_secret:
 * @config: the config instance
 * @secret: the API token
 *
 * Set the webservice API token secret in the configuration
 * (Needed by OAuth)
 *
 * Since: 0.2.6
 */
void
grl_config_set_api_token_secret (GrlConfig *config, const gchar *secret)
{
  g_return_if_fail (GRL_IS_CONFIG (config));

  grl_config_set_string (GRL_CONFIG (config),
                         GRL_CONFIG_KEY_APITOKEN_SECRET,
                         secret);
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
  g_return_if_fail (GRL_IS_CONFIG (config));

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
 *
 * Since: 0.1.8
 */
void
grl_config_set_username (GrlConfig *config, const gchar *username)
{
  g_return_if_fail (GRL_IS_CONFIG (config));

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
 *
 * Since: 0.1.8
 */
void
grl_config_set_password(GrlConfig *config, const gchar *password)
{
  g_return_if_fail (GRL_IS_CONFIG (config));

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
gchar *
grl_config_get_plugin (GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_PLUGIN);
}

/**
 * grl_config_get_source:
 * @config: the config instance
 *
 * Returns: the source id
 */
gchar *
grl_config_get_source (GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_SOURCE);
}

/**
 * grl_config_get_api_key:
 * @config: the config instance
 *
 * Returns: the webservice API key
 *
 * Since: 0.1.4
 */
gchar *
grl_config_get_api_key (GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_APIKEY);
}

/**
 * grl_config_get_api_key_blob:
 * @config: the config instance
 * @size: pointer to size of data
 *
 * Returns: the binary API key, size will reflect the size of the buffer
 *
 * Since: 0.1.9
 */
guint8 *
grl_config_get_api_key_blob (GrlConfig *config, gsize *size)
{
  return grl_config_get_binary (config, GRL_CONFIG_KEY_APIKEY_BLOB, size);
}

/**
 * grl_config_get_api_token:
 * @config: the config instance
 *
 * Returns: the webservice API token
 *
 * Since: 0.1.4
 */
gchar *
grl_config_get_api_token (GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_APITOKEN);
}

/**
 * grl_config_get_api_token_secret:
 * @config: the config instance
 *
 * Returns: the webservice API token secret
 * (Needed by OAuth)
 *
 * Since: 0.2.6
 */
gchar *
grl_config_get_api_token_secret (GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_APITOKEN_SECRET);
}

/**
 * grl_config_get_api_secret:
 * @config: the config instance
 *
 * Returns: the webservice API passphrase
 *
 * Since: 0.1.4
 */
gchar *
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
 *
 * Since: 0.1.8
 */
gchar *
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
 *
 * Since: 0.1.8
 */
gchar *
grl_config_get_password(GrlConfig *config)
{
  return grl_config_get_string (GRL_CONFIG (config),
                                GRL_CONFIG_KEY_PASSWORD);
}

/**
 * grl_config_has_param:
 * @config: the config instance
 * @param: the param
 *
 * Returns: TRUE if @params has a defined value within @config, FALSE
 * otherwise.
 *
 * Since: 0.1.8
 */
gboolean
grl_config_has_param (GrlConfig *config, const gchar *param)
{
  g_return_val_if_fail (GRL_IS_CONFIG (config), FALSE);
  return g_key_file_has_key (config->priv->config, GROUP_NAME, param, NULL);
}
