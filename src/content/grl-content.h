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

#ifndef _GRL_CONTENT_H_
#define _GRL_CONTENT_H_

#include <glib-object.h>
#include <grl-metadata-key.h>

G_BEGIN_DECLS

#define GRL_TYPE_CONTENT                        \
  (grl_content_get_type())

#define GRL_CONTENT(obj)                                \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_CONTENT,        \
                               GrlContent))

#define GRL_CONTENT_CLASS(klass)                \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            GRL_TYPE_CONTENT,   \
                            GrlContentClass))

#define GRL_IS_CONTENT(obj)                             \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_CONTENT))

#define GRL_IS_CONTENT_CLASS(klass)             \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),            \
                            GRL_TYPE_CONTENT))

#define GRL_CONTENT_GET_CLASS(obj)              \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),            \
                              GRL_TYPE_CONTENT, \
                              ContentClass))

typedef struct _GrlContent        GrlContent;
typedef struct _GrlContentPrivate GrlContentPrivate;
typedef struct _GrlContentClass   GrlContentClass;

/**
 * GrlContentClass:
 * @parent_class: the parent class structure
 *
 * Grilo Content class
 */
struct _GrlContentClass
{
  GObjectClass parent_class;
};

struct _GrlContent
{
  GObject parent;

  /*< private >*/
  GrlContentPrivate *priv;
};

GType grl_content_get_type (void) G_GNUC_CONST;

GrlContent *grl_content_new (void);

void grl_content_set (GrlContent *content, GrlKeyID key, const GValue *value);

void grl_content_set_string (GrlContent *content,
                             GrlKeyID key,
                             const gchar *strvalue);

void grl_content_set_int (GrlContent *content, GrlKeyID key, gint intvalue);

void grl_content_set_float (GrlContent *content,
                            GrlKeyID key,
                            gint floatvalue);

const GValue *grl_content_get (GrlContent *content, GrlKeyID key);

const gchar *grl_content_get_string (GrlContent *content, GrlKeyID key);

gint grl_content_get_int (GrlContent *content, GrlKeyID key);

gfloat grl_content_get_float (GrlContent *content, GrlKeyID key);

void grl_content_add (GrlContent *content, GrlKeyID key);

void grl_content_remove (GrlContent *content, GrlKeyID key);

gboolean grl_content_has_key (GrlContent *content, GrlKeyID key);

GList *grl_content_get_keys (GrlContent *content);

gboolean grl_content_key_is_known (GrlContent *content, GrlKeyID key);

void grl_content_set_overwrite (GrlContent *content, gboolean overwrite);

gboolean grl_content_get_overwrite (GrlContent *content);

G_END_DECLS

#endif /* _GRL_CONTENT_H_ */
