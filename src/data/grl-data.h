/*
 * Copyright (C) 2010, 2011 Igalia S.L.
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

#ifndef _GRL_DATA_H_
#define _GRL_DATA_H_

#include <glib-object.h>
#include <grl-metadata-key.h>
#include <grl-definitions.h>
#include <grl-related-keys.h>

G_BEGIN_DECLS

#define GRL_TYPE_DATA                           \
  (grl_data_get_type())

#define GRL_DATA(obj)                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),           \
                               GRL_TYPE_DATA,   \
                               GrlData))

#define GRL_DATA_CLASS(klass)                   \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            GRL_TYPE_DATA,      \
                            GrlDataClass))

#define GRL_IS_DATA(obj)                        \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),           \
                               GRL_TYPE_DATA))

#define GRL_IS_DATA_CLASS(klass)                \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),            \
                            GRL_TYPE_DATA))

#define GRL_DATA_GET_CLASS(obj)                 \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),            \
                              GRL_TYPE_DATA,    \
                              GrlDataClass))

typedef struct _GrlData        GrlData;
typedef struct _GrlDataClass   GrlDataClass;
typedef struct _GrlDataPrivate GrlDataPrivate;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlData, g_object_unref)

struct _GrlData
{
  GObject parent;

  GrlDataPrivate *priv;

  gpointer _grl_reserved[GRL_PADDING_SMALL];
};

/**
 * GrlDataClass:
 * @parent_class: the parent class structure
 *
 * Grilo Data class
 */
struct _GrlDataClass
{
  GObjectClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

GType grl_data_get_type (void) G_GNUC_CONST;

GrlData *grl_data_new (void);

void grl_data_set (GrlData *data, GrlKeyID key, const GValue *value);

void grl_data_set_string (GrlData *data,
                          GrlKeyID key,
                          const gchar *strvalue);

void grl_data_set_int (GrlData *data, GrlKeyID key, gint intvalue);

void grl_data_set_float (GrlData *data,
                         GrlKeyID key,
                         gfloat floatvalue);

void grl_data_set_boolean (GrlData *data, GrlKeyID key, gboolean boolvalue);

void grl_data_set_binary(GrlData *data, GrlKeyID key, const guint8 *buf, gsize size);

void grl_data_set_boxed (GrlData *data, GrlKeyID key, gconstpointer boxed);

void grl_data_set_int64 (GrlData *data, GrlKeyID key, gint64 intvalue);

gboolean grl_data_set_for_id (GrlData *data, const gchar *key_name, const GValue *value);

const GValue *grl_data_get (GrlData *data, GrlKeyID key);

const gchar *grl_data_get_string (GrlData *data, GrlKeyID key);

gint grl_data_get_int (GrlData *data, GrlKeyID key);

gfloat grl_data_get_float (GrlData *data, GrlKeyID key);

gboolean grl_data_get_boolean (GrlData *data, GrlKeyID key);

const guint8 *grl_data_get_binary(GrlData *data, GrlKeyID key, gsize *size);

gpointer grl_data_get_boxed (GrlData *data, GrlKeyID key);

gint64 grl_data_get_int64 (GrlData *data, GrlKeyID key);

void grl_data_remove (GrlData *data, GrlKeyID key);

gboolean grl_data_has_key (GrlData *data, GrlKeyID key);

GList *grl_data_get_keys (GrlData *data);

void grl_data_add_related_keys (GrlData *data, GrlRelatedKeys *relkeys);

void grl_data_add_string (GrlData *data, GrlKeyID key, const gchar *strvalue);

void grl_data_add_int (GrlData *data, GrlKeyID key, gint intvalue);

void grl_data_add_float (GrlData *data, GrlKeyID key, gfloat floatvalue);

void grl_data_add_binary (GrlData *data, GrlKeyID key, const guint8 *buf, gsize size);

void grl_data_add_boxed (GrlData *data, GrlKeyID key, gconstpointer boxed);

void grl_data_add_int64 (GrlData *data, GrlKeyID key, gint64 intvalue);

gboolean grl_data_add_for_id (GrlData *data, const gchar *key_name, const GValue *value);

guint grl_data_length (GrlData *data, GrlKeyID key);

GrlRelatedKeys *grl_data_get_related_keys (GrlData *data, GrlKeyID key, guint index);

GList *grl_data_get_single_values_for_key (GrlData *data, GrlKeyID key);

GList *grl_data_get_single_values_for_key_string (GrlData *data, GrlKeyID key);

void grl_data_remove_nth (GrlData *data, GrlKeyID key, guint index);

void grl_data_set_related_keys (GrlData *data, GrlRelatedKeys *relkeys, guint index);

GrlData *grl_data_dup (GrlData *data);

G_END_DECLS

#endif /* _GRL_DATA_H_ */
