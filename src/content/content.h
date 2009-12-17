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

#ifndef __CONTENT_H__
#define __CONTENT_H__

#include <glib-object.h>
#include "metadata-key.h"

G_BEGIN_DECLS

#define CONTENT_TYPE                            \
  (content_get_type())
#define CONTENT(obj)                            \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),           \
                               CONTENT_TYPE,    \
                               Content))
#define CONTENT_CLASS(klass)                    \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            CONTENT_TYPE,       \
                            ContentClass))
#define IS_CONTENT(obj)                         \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),           \
                               CONTENT_TYPE))
#define IS_CONTENT_CLASS(klass)                 \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),            \
                            CONTENT_TYPE))
#define CONTENT_GET_CLASS(obj)                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),            \
                              CONTENT_TYPE,     \
                              ContentClass))

typedef struct _Content        Content;
typedef struct _ContentPrivate ContentPrivate;
typedef struct _ContentClass   ContentClass;

struct _ContentClass
{
  GObjectClass parent_class;
};

struct _Content
{
  GObject parent;

  ContentPrivate *priv;
};

GType content_get_type (void) G_GNUC_CONST;
Content *content_new (void);
void content_set (Content *content, gint key, const GValue *value);
void content_set_string (Content *content, gint key, const gchar *strvalue);
void content_set_int (Content *content, gint key, gint intvalue);
const GValue *content_get (Content *content, gint key);
const gchar *content_get_string (Content *content, gint key);
gint content_get_int (Content *content, gint key);
void content_add (Content *content, gint key);
void content_remove (Content *content, gint key);
gboolean content_has_key (Content *content, gint key);
gint *content_get_keys (Content *content, gint *size);
G_END_DECLS

#endif /* __CONTENT_H__ */
