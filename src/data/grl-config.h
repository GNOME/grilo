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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#include <glib.h>
#include <glib-object.h>

#ifndef _GRL_CONFIG_H_
#define _GRL_CONFIG_H_

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

#define GRL_CONFIG_KEY_PLUGIN      "target-plugin"
#define GRL_CONFIG_KEY_SOURCE      "target-source"
#define GRL_CONFIG_KEY_APIKEY      "api-key"
#define GRL_CONFIG_KEY_APITOKEN    "api-token"
#define GRL_CONFIG_KEY_APISECRET   "api-secret"

typedef struct _GrlConfig        GrlConfig;
typedef struct _GrlConfigPrivate GrlConfigPrivate;
typedef struct _GrlConfigClass   GrlConfigClass;

/**
 * GrlConfigClass:
 * @parent_class: the parent class structure
 *
 * Grilo Config Class
 */
struct _GrlConfigClass
{
  GObjectClass parent_class;
};

struct _GrlConfig
{
  GObject parent;

  /*< private >*/
  GrlConfigPrivate *priv;
};

/**
 * grl_config_set_plugin:
 * @config: the config instance
 * @plugin: the plugin id
 *
 * Set the plugin key in the configuration
 */
#define grl_config_set_plugin(config, plugin)     \
  grl_config_set_string(GRL_CONFIG((config)),         \
			GRL_CONFIG_KEY_PLUGIN,    \
			(plugin))                 \

/**
 * grl_config_set_source:
 * @config: the config instance
 * @source: the source id
 *
 * Set the plugin key in the configuration
 */
#define grl_config_set_source(config, source)     \
  grl_config_set_string(GRL_CONFIG((config)),	\
			GRL_CONFIG_KEY_SOURCE,	\
			(source))		\

/**
 * grl_config_set_api_key:
 * @config: the config instance
 * @key: the API key
 *
 * Set the webservice API key in the configuration
 */
#define grl_config_set_api_key(config, key)       \
  grl_config_set_string(GRL_CONFIG((config)),	\
			GRL_CONFIG_KEY_APIKEY,	\
			(key))

/**
 * grl_config_set_api_token:
 * @config: the config instance
 * @token: the API token
 *
 * Set the webservice API token in the configuration
 */
#define grl_config_set_api_token(config, token)   \
  grl_config_set_string(GRL_CONFIG((config)),         \
			GRL_CONFIG_KEY_APITOKEN,  \
			(token))

/**
 * grl_config_set_api_secret:
 * @config: the config instance
 * @secret: the webservice passphrase
 *
 * Set the webservice passphrase in the configuration
 */
#define grl_config_set_api_secret(config, secret) \
  grl_config_set_string(GRL_CONFIG((config)),         \
			GRL_CONFIG_KEY_APISECRET, \
			(secret))

/**
 * grl_config_get_plugin:
 * @config: the config instance
 *
 * Returns: (type utf8) (transfer none): the plugin id
 */
#define grl_config_get_plugin(config)                               \
  grl_config_get_string(GRL_CONFIG((config)), GRL_CONFIG_KEY_PLUGIN)

/**
 * grl_config_get_api_key:
 * @config: the config instance
 *
 * Returns: (type utf8) (transfer none): the webservice API key
 */
#define grl_config_get_api_key(config)                              \
  grl_config_get_string(GRL_CONFIG((config)), GRL_CONFIG_KEY_APIKEY)

/**
 * grl_config_get_api_token:
 * @config: the config instance
 *
 * Returns: (type utf8) (transfer none): the webservice API token
 */
#define grl_config_get_api_token(config)					\
  grl_config_get_string(GRL_CONFIG((config)), GRL_CONFIG_KEY_APITOKEN)

/**
 * grl_config_get_api_secret:
 * @config: the config instance
 *
 * Returns: (type utf8) (transfer none): the webservice API passphrase
 */
#define grl_config_get_api_secret(config)                           \
grl_config_get_string(GRL_CONFIG((config)), GRL_CONFIG_KEY_APISECRET)

GType grl_config_get_type (void) G_GNUC_CONST;
GrlConfig *grl_config_new (const gchar *plugin, const gchar *source);

void grl_config_set (GrlConfig *config, const gchar *param, const GValue *value);

void grl_config_set_string (GrlConfig *config,
			    const gchar *param,
			    const gchar *value);

void grl_config_set_int (GrlConfig *config, const gchar *param, gint value);

void grl_config_set_float (GrlConfig *config, const gchar *param, gfloat value);

const GValue *grl_config_get (GrlConfig *config, const gchar *param);

const gchar *grl_config_get_string (GrlConfig *config, const gchar *param);

gint grl_config_get_int (GrlConfig *config, const gchar *param);

gfloat grl_config_get_float (GrlConfig *config, const gchar *param);


G_END_DECLS

#endif /* _GRL_CONFIG_H_ */
