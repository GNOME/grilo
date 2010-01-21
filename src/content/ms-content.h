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

#ifndef __MS_CONTENT_H__
#define __MS_CONTENT_H__

#include <glib-object.h>
#include <ms-metadata-key.h>

G_BEGIN_DECLS

#define MS_TYPE_CONTENT                         \
  (ms_content_get_type())
#define MS_CONTENT(obj)                         \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),           \
                               MS_TYPE_CONTENT, \
                               MsContent))
#define MS_CONTENT_CLASS(klass)                 \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            MS_TYPE_CONTENT,    \
                            MsContentClass))
#define MS_IS_CONTENT(obj)                              \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               MS_TYPE_CONTENT))
#define MS_IS_CONTENT_CLASS(klass)              \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),            \
                            MS_TYPE_CONTENT))
#define MS_CONTENT_GET_CLASS(obj)                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),               \
                              MS_TYPE_CONTENT,     \
                              ContentClass))

typedef struct _MsContent        MsContent;
typedef struct _MsContentPrivate MsContentPrivate;
typedef struct _MsContentClass   MsContentClass;

struct _MsContentClass
{
  GObjectClass parent_class;
};

struct _MsContent
{
  GObject parent;

  MsContentPrivate *priv;
};

GType ms_content_get_type (void) G_GNUC_CONST;
MsContent *ms_content_new (void);
void ms_content_set (MsContent *content, MsKeyID key, const GValue *value);
void ms_content_set_string (MsContent *content, MsKeyID key, const gchar *strvalue);
void ms_content_set_int (MsContent *content, MsKeyID key, gint intvalue);
void ms_content_set_float (MsContent *content, MsKeyID key, gint floatvalue);
const GValue *ms_content_get (MsContent *content, MsKeyID key);
const gchar *ms_content_get_string (MsContent *content, MsKeyID key);
gint ms_content_get_int (MsContent *content, MsKeyID key);
gfloat ms_content_get_float (MsContent *content, MsKeyID key);
void ms_content_add (MsContent *content, MsKeyID key);
void ms_content_remove (MsContent *content, MsKeyID key);
gboolean ms_content_has_key (MsContent *content, MsKeyID key);
GList *ms_content_get_keys (MsContent *content);
gboolean ms_content_key_is_known (MsContent *content, MsKeyID key);
void ms_content_set_overwrite (MsContent *content, gboolean overwrite);
gboolean ms_content_get_overwrite (MsContent *content);

G_END_DECLS

#endif /* __CONTENT_H__ */
