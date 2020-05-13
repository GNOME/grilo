/*
 * Copyright (C) 2011 Igalia S.L.
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

#ifndef _GRL_RELATED_KEYS_H_
#define _GRL_RELATED_KEYS_H_

#include <glib-object.h>
#include <grl-metadata-key.h>
#include <grl-definitions.h>

G_BEGIN_DECLS

#define GRL_TYPE_RELATED_KEYS                   \
  (grl_related_keys_get_type())

#define GRL_RELATED_KEYS(obj)                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_RELATED_KEYS,   \
                               GrlRelatedKeys))

#define GRL_RELATED_KEYS_CLASS(klass)                   \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_RELATED_KEYS,      \
                            GrlRelatedKeysClass))

#define GRL_IS_RELATED_KEYS(obj)                        \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_RELATED_KEYS))

#define GRL_IS_RELATED_KEYS_CLASS(klass)                \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_RELATED_KEYS))

#define GRL_RELATED_KEYS_GET_CLASS(obj)                 \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_RELATED_KEYS,    \
                              GrlRelatedKeysClass))

typedef struct _GrlRelatedKeys        GrlRelatedKeys;
typedef struct _GrlRelatedKeysPrivate GrlRelatedKeysPrivate;
typedef struct _GrlRelatedKeysClass   GrlRelatedKeysClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlRelatedKeys, g_object_unref)

struct _GrlRelatedKeys
{
  GObject parent;

  /*< private >*/
  GrlRelatedKeysPrivate *priv;

  gpointer _grl_reserved[GRL_PADDING_SMALL];
};

/**
 * GrlRelatedKeysClass:
 * @parent_class: the parent class structure
 *
 * Grilo Data Multivalued class
 */
struct _GrlRelatedKeysClass
{
  GObjectClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

GType grl_related_keys_get_type (void) G_GNUC_CONST;

GrlRelatedKeys *grl_related_keys_new (void);

GrlRelatedKeys *grl_related_keys_new_valist (GrlKeyID key, va_list args);

GrlRelatedKeys *grl_related_keys_new_with_keys (GrlKeyID key, ...);

void grl_related_keys_set (GrlRelatedKeys *relkeys,
                           GrlKeyID key,
                           const GValue *value);

void grl_related_keys_set_string (GrlRelatedKeys *relkeys,
                                  GrlKeyID key,
                                  const gchar *strvalue);

void grl_related_keys_set_int (GrlRelatedKeys *relkeys,
                               GrlKeyID key,
                               gint intvalue);

void grl_related_keys_set_float (GrlRelatedKeys *relkeys,
                                 GrlKeyID key,
                                 gfloat floatvalue);

void grl_related_keys_set_boolean (GrlRelatedKeys *relkeys,
                                   GrlKeyID key,
                                   gboolean booleanvalue);

void grl_related_keys_set_binary(GrlRelatedKeys *relkeys,
                                 GrlKeyID key,
                                 const guint8 *buf,
                                 gsize size);

void grl_related_keys_set_boxed (GrlRelatedKeys *relkeys,
                                 GrlKeyID key,
                                 gconstpointer boxed);

void grl_related_keys_set_int64 (GrlRelatedKeys *relkeys,
                                 GrlKeyID key,
                                 gint64 intvalue);

const GValue *grl_related_keys_get (GrlRelatedKeys *relkeys,
                                    GrlKeyID key);

const gchar *grl_related_keys_get_string (GrlRelatedKeys *relkeys,
                                          GrlKeyID key);

gint grl_related_keys_get_int (GrlRelatedKeys *relkeys,
                               GrlKeyID key);

gfloat grl_related_keys_get_float (GrlRelatedKeys *relkeys,
                                   GrlKeyID key);

gboolean grl_related_keys_get_boolean (GrlRelatedKeys *relkeys,
                                       GrlKeyID key);

const guint8 *grl_related_keys_get_binary(GrlRelatedKeys *relkeys,
                                          GrlKeyID key,
                                          gsize *size);

gconstpointer grl_related_keys_get_boxed (GrlRelatedKeys *relkeys,
                                          GrlKeyID key);

gint64 grl_related_keys_get_int64 (GrlRelatedKeys *relkeys,
                                   GrlKeyID key);

void grl_related_keys_remove (GrlRelatedKeys *relkeys,
                              GrlKeyID key);

gboolean grl_related_keys_has_key (GrlRelatedKeys *relkeys,
                                   GrlKeyID key);

GList *grl_related_keys_get_keys (GrlRelatedKeys *relkeys);

gboolean grl_related_keys_set_for_id (GrlRelatedKeys *relkeys,
                                      const gchar *key_name,
                                      const GValue *value);

GrlRelatedKeys *grl_related_keys_dup (GrlRelatedKeys *relkeys);

G_END_DECLS

#endif /* _GRL_RELATED_KEYS_H_ */
