/*
 * Copyright (C) 2010, 2011 Igalia S.L.
 * Copyright (C) 2011 Intel Corporation.
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

#include <grl-definitions.h>

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

#define GRL_CONFIG_KEY_PLUGIN            "target-plugin"
#define GRL_CONFIG_KEY_SOURCE            "target-source"
#define GRL_CONFIG_KEY_APIKEY            "api-key"
#define GRL_CONFIG_KEY_APIKEY_BLOB       "api-key-blob"
#define GRL_CONFIG_KEY_APITOKEN          "api-token"
#define GRL_CONFIG_KEY_APITOKEN_SECRET   "api-token-secret"
#define GRL_CONFIG_KEY_APISECRET         "api-secret"
#define GRL_CONFIG_KEY_USERNAME          "username"
#define GRL_CONFIG_KEY_PASSWORD          "password"

typedef struct _GrlConfig        GrlConfig;
typedef struct _GrlConfigPrivate GrlConfigPrivate;
typedef struct _GrlConfigClass   GrlConfigClass;

struct _GrlConfig
{
  GObject parent;

  GrlConfigPrivate *priv;

  gpointer _grl_reserved[GRL_PADDING_SMALL];
};

/**
 * GrlConfigClass:
 * @parent_class: the parent class structure
 *
 * Grilo Config Class
 */
struct _GrlConfigClass
{
  GObjectClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

_GRL_EXTERN
void grl_config_set_plugin (GrlConfig *config, const gchar *plugin);

_GRL_EXTERN
void grl_config_set_source (GrlConfig *config, const gchar *source);

_GRL_EXTERN
void grl_config_set_api_key (GrlConfig *config, const gchar *key);

_GRL_EXTERN
void grl_config_set_api_key_blob (GrlConfig *config, const guint8 *blob, gsize size);

_GRL_EXTERN
void grl_config_set_api_token (GrlConfig *config, const gchar *token);

_GRL_EXTERN
void grl_config_set_api_token_secret (GrlConfig *config, const gchar *secret);

_GRL_EXTERN
void grl_config_set_api_secret (GrlConfig *config, const gchar *secret);

_GRL_EXTERN
void grl_config_set_username (GrlConfig *config, const gchar *username);

_GRL_EXTERN
void grl_config_set_password (GrlConfig *config, const gchar *password);

_GRL_EXTERN
gchar *grl_config_get_plugin (GrlConfig *config);

_GRL_EXTERN
gchar *grl_config_get_source (GrlConfig *config);

_GRL_EXTERN
gchar *grl_config_get_api_key (GrlConfig *config);

_GRL_EXTERN
guint8 *grl_config_get_api_key_blob (GrlConfig *config, gsize *size);

_GRL_EXTERN
gchar *grl_config_get_api_token (GrlConfig *config);

_GRL_EXTERN
gchar *grl_config_get_api_token_secret (GrlConfig *config);

_GRL_EXTERN
gchar *grl_config_get_api_secret (GrlConfig *config);

_GRL_EXTERN
gchar *grl_config_get_username (GrlConfig *config);

_GRL_EXTERN
gchar *grl_config_get_password (GrlConfig *config);

_GRL_EXTERN
GType grl_config_get_type (void) G_GNUC_CONST;
_GRL_EXTERN
GrlConfig *grl_config_new (const gchar *plugin, const gchar *source);

_GRL_EXTERN
void grl_config_set (GrlConfig *config, const gchar *param, const GValue *value);

_GRL_EXTERN
void grl_config_set_string (GrlConfig *config,
			    const gchar *param,
			    const gchar *value);

_GRL_EXTERN
void grl_config_set_int (GrlConfig *config, const gchar *param, gint value);

_GRL_EXTERN
void grl_config_set_float (GrlConfig *config, const gchar *param, gfloat value);

_GRL_EXTERN
void grl_config_set_boolean (GrlConfig *config, const gchar *param, gboolean value);

_GRL_EXTERN
void grl_config_set_binary (GrlConfig *config, const gchar *param, const guint8 *blob, gsize size);

_GRL_EXTERN
gchar *grl_config_get_string (GrlConfig *config, const gchar *param);

_GRL_EXTERN
gint grl_config_get_int (GrlConfig *config, const gchar *param);

_GRL_EXTERN
gfloat grl_config_get_float (GrlConfig *config, const gchar *param);

_GRL_EXTERN
gboolean grl_config_get_boolean (GrlConfig *config, const gchar *param);

_GRL_EXTERN
guint8 *grl_config_get_binary (GrlConfig *config, const gchar *param, gsize *size);

_GRL_EXTERN
gboolean grl_config_has_param (GrlConfig *config, const gchar *param);

G_END_DECLS

#endif /* _GRL_CONFIG_H_ */
