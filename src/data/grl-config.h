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

#ifndef _GRL_CONFIG_H_
#define _GRL_CONFIG_H_

#include <grl-data.h>


G_BEGIN_DECLS

#define GRL_TYPE_CONFIG                         \
  (grl_config_get_type())

#define GRL_CONFIG(obj)                         \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),           \
                               GRL_TYPE_CONFIG, \
                               GrlConfig))

#define GRL_CONFIG_CLASS(klass)                 \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            GRL_TYPE_CONFIG,    \
                            GrlConfigClass))

#define GRL_IS_CONFIG(obj)                              \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_CONFIG))

#define GRL_IS_CONFIG_CLASS(klass)              \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),            \
                            GRL_TYPE_CONFIG))

#define GRL_CONFIG_GET_CLASS(obj)               \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),            \
                              GRL_TYPE_CONFIG,  \
                              GrlConfigClass))


#define GRL_CONFIG_KEY_PLUGIN               1
#define GRL_CONFIG_KEY_PLUGIN_NAME          "plugin"
#define GRL_CONFIG_KEY_PLUGIN_DESC          "Plugin ID to which the configuration applies"

#define GRL_CONFIG_KEY_SOURCE               2
#define GRL_CONFIG_KEY_SOURCE_NAME          "source"
#define GRL_CONFIG_KEY_SOURCE_DESC          "Source ID to which the configuration applies"

#define GRL_CONFIG_KEY_APIKEY               3
#define GRL_CONFIG_KEY_APIKEY_NAME          "api-key"
#define GRL_CONFIG_KEY_APIKEY_DESC          "API Key"

#define GRL_CONFIG_KEY_APITOKEN             4
#define GRL_CONFIG_KEY_APITOKEN_NAME        "api-token"
#define GRL_CONFIG_KEY_APITOKEN_DESC        "API token"

#define GRL_CONFIG_KEY_APISECRET            5
#define GRL_CONFIG_KEY_APISECRET_NAME       "api-secret"
#define GRL_CONFIG_KEY_APISECRET_DESC       "API secret"

typedef struct _GrlConfig      GrlConfig;
typedef struct _GrlConfigClass GrlConfigClass;

/**
 * GrlConfigClass:
 * @parent_class: the parent class structure
 *
 * Grilo Config Class
 */
struct _GrlConfigClass
{
  GrlDataClass parent_class;
};

struct _GrlConfig
{
  GrlData parent;
};

/**
 * grl_config_set_plugin:
 * @data: the config instance
 * @plugin: the plugin id
 *
 * Set the plugin key in the configuration
 */
#define grl_config_set_plugin(data, plugin)     \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_CONFIG_KEY_PLUGIN,    \
                      (plugin))                 \

/**
 * grl_config_set_source:
 * @data: the config instance
 * @source: the source id
 *
 * Set the plugin key in the configuration
 */
#define grl_config_set_source(data, source)     \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_CONFIG_KEY_SOURCE,    \
                      (source))                 \

/**
 * grl_config_set_api_key:
 * @data: the config instance
 * @key: the API key
 *
 * Set the webservice API key in the configuration
 */
#define grl_config_set_api_key(data, key)       \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_CONFIG_KEY_APIKEY,    \
                      (key))

/**
 * grl_config_set_api_token:
 * @data: the config instance
 * @token: the API token
 *
 * Set the webservice API token in the configuration
 */
#define grl_config_set_api_token(data, token)   \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_CONFIG_KEY_APITOKEN,  \
                      (token))

/**
 * grl_config_set_api_secret:
 * @data: the config instance
 * @secret: the webservice passphrase
 *
 * Set the webservice passphrase in the configuration
 */
#define grl_config_set_api_secret(data, secret) \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_CONFIG_KEY_APISECRET, \
                      (secret))

/**
 * grl_config_get_plugin:
 * @data: the config instance
 *
 * Returns: (type utf8) (transfer none): the plugin id
 */
#define grl_config_get_plugin(data)                               \
  grl_data_get_string(GRL_DATA((data)), GRL_CONFIG_KEY_PLUGIN)

/**
 * grl_config_get_api_key:
 * @data: the config instance
 *
 * Returns: (type utf8) (transfer none): the webservice API key
 */
#define grl_config_get_api_key(data)                              \
  grl_data_get_string(GRL_DATA((data)), GRL_CONFIG_KEY_APIKEY)

/**
 * grl_config_get_api_token:
 * @data: the config instance
 *
 * Returns: (type utf8) (transfer none): the webservice API token
 */
#define grl_config_get_api_token(data)                            \
  grl_data_get_string(GRL_DATA((data)), GRL_CONFIG_KEY_APITOKEN)

/**
 * grl_config_get_api_secret:
 * @data: the config instance
 *
 * Returns: (type utf8) (transfer none): the webservice API passphrase
 */
#define grl_config_get_api_secret(data)                           \
grl_data_get_string(GRL_DATA((data)), GRL_CONFIG_KEY_APISECRET)

GType grl_config_get_type (void) G_GNUC_CONST;
GrlConfig *grl_config_new (const gchar *plugin, const gchar *source);

G_END_DECLS

#endif /* _GRL_CONFIG_H_ */
