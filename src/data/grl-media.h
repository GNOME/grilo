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
 *
 * Since: 0.2.3
 */
typedef enum {
  GRL_MEDIA_SERIALIZE_BASIC,
  GRL_MEDIA_SERIALIZE_PARTIAL,
  GRL_MEDIA_SERIALIZE_FULL
} GrlMediaSerializeType;


typedef struct _GrlMedia        GrlMedia;
typedef struct _GrlMediaPrivate GrlMediaPrivate;
typedef struct _GrlMediaClass   GrlMediaClass;

struct _GrlMedia
{
  GrlData parent;

  /*< private >*/
  GrlMediaPrivate *priv;
  gpointer _grl_reserved[GRL_PADDING_SMALL];
};

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

void grl_media_set_id (GrlMedia *media, const gchar *id);

void grl_media_set_url (GrlMedia *media, const gchar *url);

void grl_media_set_author (GrlMedia *media, const gchar *author);

void grl_media_set_title (GrlMedia *media, const gchar *title);

void grl_media_set_description (GrlMedia *media, const gchar *description);

void grl_media_set_source (GrlMedia *media, const gchar *source);

void grl_media_set_thumbnail (GrlMedia *media, const gchar *thumbnail);

void grl_media_set_thumbnail_binary (GrlMedia *media, const guint8 *thumbnail, gsize size);

void grl_media_set_site (GrlMedia *media, const gchar *site);

void grl_media_set_duration (GrlMedia *media, gint duration);

void grl_media_set_publication_date (GrlMedia *media, const GDateTime *date);

void grl_media_set_region (GrlMedia *media,
                           const gchar *region);

void grl_media_set_region_data (GrlMedia *media,
                                const gchar *region,
                                const GDateTime *publication_date,
                                const gchar *certificate);

void grl_media_add_region_data (GrlMedia *media,
                                const gchar *region,
                                const GDateTime *publication_date,
                                const gchar *certificate);

void grl_media_set_creation_date (GrlMedia *media,
                                  const GDateTime *creation_date);

void grl_media_set_modification_date (GrlMedia *media,
                                      const GDateTime *modification_date);

void grl_media_set_mime (GrlMedia *media, const gchar *mime);

void grl_media_set_play_count (GrlMedia *media, gint play_count);

void grl_media_set_last_played (GrlMedia *media, const GDateTime *last_played);

void grl_media_set_last_position (GrlMedia *media, gint last_position);

void grl_media_set_external_player (GrlMedia *media, const gchar *player);

void grl_media_set_external_url (GrlMedia *media, const gchar *url);

void grl_media_set_studio (GrlMedia *media, const gchar *studio);

void grl_media_set_certificate (GrlMedia *media, const gchar *certificate);

void grl_media_set_license (GrlMedia *media, const gchar *license);

void grl_media_set_rating (GrlMedia *media, gfloat rating, gfloat max);

void grl_media_set_url_data (GrlMedia *media, const gchar *url, const gchar *mime, gint bitrate);

void grl_media_set_favourite (GrlMedia *media, gboolean favourite);

void grl_media_set_keyword (GrlMedia *media, const gchar *keyword);

void grl_media_set_size (GrlMedia *media, gint64 size);

void grl_media_set_track_number (GrlMedia *media, gint track_number);

void grl_media_set_bitrate (GrlMedia *media, gint bitrate);

void grl_media_set_mb_track_id (GrlMedia *media, const gchar *mb_track_id);

void grl_media_set_mb_recording_id (GrlMedia *media, const gchar *mb_recording_id);

void grl_media_set_mb_artist_id (GrlMedia *media, const gchar *mb_artist_id);

void grl_media_set_mb_album_id (GrlMedia *media, const gchar *mb_album_id);

void grl_media_set_lyrics (GrlMedia *media, const gchar *lyrics);

void grl_media_set_genre (GrlMedia *media, const gchar *genre);

void grl_media_set_album (GrlMedia *media, const gchar *album);

void grl_media_set_artist (GrlMedia *media, const gchar *artist);

void grl_media_add_url_data (GrlMedia *media, const gchar *url, const gchar *mime, gint bitrate);

void grl_media_add_author (GrlMedia *media, const gchar *author);

void grl_media_add_thumbnail (GrlMedia *media, const gchar *thumbnail);

void grl_media_add_thumbnail_binary (GrlMedia *media, const guint8 *thumbnail, gsize size);

void grl_media_add_external_player (GrlMedia *media, const gchar *player);

void grl_media_add_external_url (GrlMedia *media, const gchar *url);

void grl_media_add_keyword (GrlMedia *media, const gchar *keyword);

void grl_media_add_artist (GrlMedia *media, const gchar *artist);

void grl_media_add_genre (GrlMedia *media, const gchar *genre);

void grl_media_add_lyrics (GrlMedia *media, const gchar *lyrics);

void grl_media_add_mb_artist_id (GrlMedia *media, const gchar *mb_artist_id);

const gchar *grl_media_get_id (GrlMedia *media);

const gchar *grl_media_get_url (GrlMedia *media);

const gchar *grl_media_get_url_data (GrlMedia *media, gchar **mime, gint *bitrate);

const gchar *grl_media_get_url_data_nth (GrlMedia *media, guint index, gchar **mime, gint *bitrate);

const gchar *grl_media_get_author (GrlMedia *media);

const gchar *grl_media_get_author_nth (GrlMedia *media, guint index);

const gchar *grl_media_get_title (GrlMedia *media);

const gchar *grl_media_get_description (GrlMedia *media);

const gchar *grl_media_get_source (GrlMedia *media);

const gchar *grl_media_get_thumbnail (GrlMedia *media);

const gchar *grl_media_get_thumbnail_nth (GrlMedia *media, guint index);

const guint8 *grl_media_get_thumbnail_binary (GrlMedia *media, gsize *size);

const guint8 *grl_media_get_thumbnail_binary_nth (GrlMedia *media, gsize *size, guint index);

const gchar *grl_media_get_site (GrlMedia *media);

gint grl_media_get_duration (GrlMedia *media);

GDateTime *grl_media_get_publication_date (GrlMedia *media);

const gchar *grl_media_get_region (GrlMedia *media);

const gchar *grl_media_get_region_data(GrlMedia *media,
                                       const GDateTime **publication_date,
                                       const gchar **certificate);

const gchar *grl_media_get_region_data_nth(GrlMedia *media,
                                           guint index,
                                           const GDateTime **publication_date,
                                           const gchar **certificate);

GDateTime *grl_media_get_creation_date (GrlMedia *media);

GDateTime *grl_media_get_modification_date (GrlMedia *media);

const gchar *grl_media_get_mime (GrlMedia *media);

gfloat grl_media_get_rating (GrlMedia *media);

gint grl_media_get_play_count (GrlMedia *media);

gint grl_media_get_last_position (GrlMedia *media);

GDateTime *grl_media_get_last_played (GrlMedia *media);

const gchar *grl_media_get_player (GrlMedia *media);

const gchar *grl_media_get_player_nth (GrlMedia *media, guint index);

const gchar *grl_media_get_external_url (GrlMedia *media);

const gchar *grl_media_get_external_url_nth (GrlMedia *media, guint index);

const gchar *grl_media_get_studio (GrlMedia *media);

const gchar *grl_media_get_certificate (GrlMedia *media);

const gchar *grl_media_get_license (GrlMedia *media);

gfloat grl_media_get_start_time (GrlMedia *media);

gboolean grl_media_get_favourite (GrlMedia *media);

const gchar *grl_media_get_keyword (GrlMedia *media);

const gchar * grl_media_get_keyword_nth (GrlMedia *media, guint index);

gint64 grl_media_get_size (GrlMedia *media);

gint grl_media_get_track_number (GrlMedia *media);

gint grl_media_get_bitrate (GrlMedia *media);

const gchar *grl_media_get_mb_album_id (GrlMedia *media);

const gchar *grl_media_get_mb_artist_id (GrlMedia *media);

const gchar *grl_media_get_mb_artist_id_nth (GrlMedia *media, guint index);

const gchar *grl_media_get_mb_recording_id (GrlMedia *media);

const gchar *grl_media_get_mb_track_id (GrlMedia *media);

const gchar *grl_media_get_lyrics (GrlMedia *media);

const gchar *grl_media_get_lyrics_nth (GrlMedia *media, guint index);

const gchar *grl_media_get_genre (GrlMedia *media);

const gchar *grl_media_get_genre_nth (GrlMedia *media, guint index);

const gchar *grl_media_get_album (GrlMedia *media);

const gchar *grl_media_get_artist (GrlMedia *media);

const gchar *grl_media_get_artist_nth (GrlMedia *media, guint index);

GrlMediaType grl_media_get_media_type (GrlMedia *media);

GType grl_media_get_type (void) G_GNUC_CONST;

GrlMedia *grl_media_new (void);

GrlMedia *grl_media_audio_new (void);

gboolean
grl_media_is_audio (GrlMedia *media);

gchar *grl_media_serialize (GrlMedia *media);

gchar *grl_media_serialize_extended (GrlMedia *media,
                                     GrlMediaSerializeType serial_type,
                                     ...);

GrlMedia *grl_media_unserialize (const gchar *serial);

G_END_DECLS

#endif /* _GRL_MEDIA_H_ */
