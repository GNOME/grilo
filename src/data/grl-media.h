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

#ifndef _GRL_MEDIA_H_
#define _GRL_MEDIA_H_

#include <grl-data.h>
#include <grl-definitions.h>

G_BEGIN_DECLS

#define GRL_TYPE_MEDIA                          \
  (grl_media_get_type())

#define GRL_MEDIA(obj)                          \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),           \
                               GRL_TYPE_MEDIA,  \
                               GrlMedia))

#define GRL_MEDIA_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            GRL_TYPE_MEDIA,     \
                            GrlMediaClass))

#define GRL_IS_MEDIA(obj)                       \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),           \
                               GRL_TYPE_MEDIA))

#define GRL_IS_MEDIA_CLASS(klass)               \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),            \
                            GRL_TYPE_MEDIA))

#define GRL_MEDIA_GET_CLASS(obj)                \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),            \
                              GRL_TYPE_MEDIA,   \
                              GrlMediaClass))

/**
 * GrlMediaSerializeType:
 * @GRL_MEDIA_SERIALIZE_BASIC: Basic mode
 * @GRL_MEDIA_SERIALIZE_PARTIAL: Partial mode
 * @GRL_MEDIA_SERIALIZE_FULL: Full mode
 *
 * GrlMedia serialize type
 */
typedef enum {
  GRL_MEDIA_SERIALIZE_BASIC,
  GRL_MEDIA_SERIALIZE_PARTIAL,
  GRL_MEDIA_SERIALIZE_FULL
} GrlMediaSerializeType;

typedef struct _GrlMedia      GrlMedia;
typedef struct _GrlMediaClass GrlMediaClass;

/**
 * GrlMediaClass:
 * @parent_class: the parent class structure
 *
 * Grilo Media Class
 */
struct _GrlMediaClass
{
  GrlDataClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

struct _GrlMedia
{
  GrlData parent;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING_SMALL];
};

void grl_media_set_id (GrlMedia *data, const gchar *id);

void grl_media_set_url (GrlMedia *data, const gchar *url);

void grl_media_set_author (GrlMedia *data, const gchar *author);

void grl_media_set_title (GrlMedia *data, const gchar *title);

void grl_media_set_description (GrlMedia *data, const gchar *description);

void grl_media_set_source (GrlMedia *data, const gchar *source);

void grl_media_set_thumbnail (GrlMedia *data, const gchar *thumbnail);

void grl_media_set_thumbnail_binary (GrlMedia *data, const guint8 *thumbnail, gsize size);

void grl_media_set_site (GrlMedia *data, const gchar *site);

void grl_media_set_duration (GrlMedia *data, gint duration);

void grl_media_set_date (GrlMedia *data, const gchar *date);

void grl_media_set_mime (GrlMedia *data, const gchar *mime);

void grl_media_set_play_count (GrlMedia *data, gint play_count);

void grl_media_set_last_played (GrlMedia *data, const gchar *last_played);

void grl_media_set_last_position (GrlMedia *data, gint last_position);

void grl_media_set_external_player (GrlMedia *data, const gchar *player);

void grl_media_set_external_url (GrlMedia *data, const gchar *url);

void grl_media_set_studio (GrlMedia *data, const gchar *studio);

void grl_media_set_certificate (GrlMedia *data, const gchar *certificate);

void grl_media_set_license (GrlMedia *data, const gchar *license);

void grl_media_set_rating (GrlMedia *media, gfloat rating, gfloat max);

const gchar *grl_media_get_id (GrlMedia *data);

const gchar *grl_media_get_url (GrlMedia *data);

const gchar *grl_media_get_author (GrlMedia *data);

const gchar *grl_media_get_title (GrlMedia *data);

const gchar *grl_media_get_description (GrlMedia *data);

const gchar *grl_media_get_source (GrlMedia *data);

const gchar *grl_media_get_thumbnail (GrlMedia *data);

const guint8 *grl_media_get_thumbnail_binary (GrlMedia *data, gsize *size);

const gchar *grl_media_get_site (GrlMedia *data);

gint grl_media_get_duration (GrlMedia *data);

const gchar *grl_media_get_date (GrlMedia *data);

const gchar *grl_media_get_mime (GrlMedia *data);

gfloat grl_media_get_rating (GrlMedia *data);

gint grl_media_get_play_count (GrlMedia *data);

gint grl_media_get_last_position (GrlMedia *data);

const gchar *grl_media_get_last_played (GrlMedia *data);

const gchar *grl_media_get_player (GrlMedia *data);

const gchar *grl_media_get_external_url (GrlMedia *data);

const gchar *grl_media_get_studio (GrlMedia *data);

const gchar *grl_media_get_certificate (GrlMedia *data);

const gchar *grl_media_get_license (GrlMedia *data);

GType grl_media_get_type (void) G_GNUC_CONST;

GrlMedia *grl_media_new (void);

gchar *grl_media_serialize (GrlMedia *media);

gchar *grl_media_serialize_extended (GrlMedia *media,
                                     GrlMediaSerializeType serial_type,
                                     ...);

GrlMedia *grl_media_unserialize (const gchar *serial);

G_END_DECLS

#endif /* _GRL_MEDIA_H_ */
