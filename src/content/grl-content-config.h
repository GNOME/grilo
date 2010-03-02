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

#ifndef _GRL_CONTENT_CONFIG_H_
#define _GRL_CONTENT_CONFIG_H_

#include <grl-content.h>


G_BEGIN_DECLS

#define GRL_TYPE_CONTENT_CONFIG                 \
  (grl_content_config_get_type())

#define GRL_CONTENT_CONFIG(obj)                          \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                    \
                               GRL_TYPE_CONTENT_CONFIG,  \
                               GrlContentConfig))

#define GRL_CONTENT_CONFIG_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                     \
                            GRL_TYPE_CONTENT_CONFIG,     \
                            GrlContentConfigClass))

#define GRL_IS_CONTENT_CONFIG(obj)                              \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                           \
                               GRL_TYPE_CONTENT_CONFIG))

#define GRL_IS_CONTENT_CONFIG_CLASS(klass)              \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_CONTENT_CONFIG))

#define GRL_CONTENT_CONFIG_GET_CLASS(obj)                \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                     \
                              GRL_TYPE_CONTENT_CONFIG,   \
                              GrlContentConfigClass))

typedef struct _GrlContentConfig      GrlContentConfig;
typedef struct _GrlContentConfigClass GrlContentConfigClass;

struct _GrlContentConfigClass
{
  GrlContentClass parent_class;
};

struct _GrlContentConfig
{
  GrlContent parent;
};

#define grl_content_config_set_plugin(content, plugin)  \
  grl_content_set_string(GRL_CONTENT((content)),        \
                         GRL_CONFIG_KEY_PLUGIN,         \
                         (plugin))                      \

#define grl_content_config_set_api_key(content, key)    \
  grl_content_set_string(GRL_CONTENT((content)),        \
                         GRL_CONFIG_KEY_APIKEY,         \
                         (key))

#define grl_content_config_set_api_token(content, token)      \
  grl_content_set_string(GRL_CONTENT((content)),              \
                         GRL_CONFIG_KEY_APITOKEN,             \
                         (token))

#define grl_content_config_set_api_secret(content, secret)    \
  grl_content_set_string(GRL_CONTENT((content)),              \
                         GRL_CONFIG_KEY_APISECRET,            \
                         (secret))

#define grl_content_config_get_plugin(content)                          \
  grl_content_get_string(GRL_CONTENT((content)), GRL_CONFIG_KEY_PLUGIN)
#define grl_content_config_get_api_key(content)                         \
  grl_content_get_string(GRL_CONTENT((content)), GRL_CONFIG_KEY_APIKEY)
#define grl_content_config_get_api_token(content)                       \
  grl_content_get_string(GRL_CONTENT((content)), GRL_CONFIG_KEY_APITOKEN)
#define grl_content_config_get_api_secret(content)                      \
  grl_content_get_string(GRL_CONTENT((content)), GRL_CONFIG_KEY_APISECRET)

GType grl_content_config_get_type (void) G_GNUC_CONST;
GrlContentConfig *grl_content_config_new (void);
GrlContentConfig *grl_content_config_new_for_plugin (const gchar *plugin);

G_END_DECLS

#endif /* _GRL_CONTENT_CONFIG_H_ */
