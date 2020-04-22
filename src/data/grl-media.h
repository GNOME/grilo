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

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlMedia, g_object_unref)

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

_GRL_EXTERN
void grl_media_set_id (GrlMedia *media, const gchar *id);

_GRL_EXTERN
void grl_media_set_url (GrlMedia *media, const gchar *url);

_GRL_EXTERN
void grl_media_set_author (GrlMedia *media, const gchar *author);

_GRL_EXTERN
void grl_media_set_title (GrlMedia *media, const gchar *title);

_GRL_EXTERN
void grl_media_set_description (GrlMedia *media, const gchar *description);

_GRL_EXTERN
void grl_media_set_source (GrlMedia *media, const gchar *source);

_GRL_EXTERN
void grl_media_set_thumbnail (GrlMedia *media, const gchar *thumbnail);

_GRL_EXTERN
void grl_media_set_thumbnail_binary (GrlMedia *media, const guint8 *thumbnail, gsize size);

_GRL_EXTERN
void grl_media_set_site (GrlMedia *media, const gchar *site);

_GRL_EXTERN
void grl_media_set_duration (GrlMedia *media, gint duration);

_GRL_EXTERN
void grl_media_set_publication_date (GrlMedia *media, const GDateTime *date);

_GRL_EXTERN
void grl_media_set_region (GrlMedia *media,
                           const gchar *region);

_GRL_EXTERN
void grl_media_set_region_data (GrlMedia *media,
                                const gchar *region,
                                const GDateTime *publication_date,
                                const gchar *certificate);

_GRL_EXTERN
void grl_media_add_region_data (GrlMedia *media,
                                const gchar *region,
                                const GDateTime *publication_date,
                                const gchar *certificate);

_GRL_EXTERN
void grl_media_set_creation_date (GrlMedia *media,
                                  const GDateTime *creation_date);

_GRL_EXTERN
void grl_media_set_modification_date (GrlMedia *media,
                                      const GDateTime *modification_date);

_GRL_EXTERN
void grl_media_set_mime (GrlMedia *media, const gchar *mime);

_GRL_EXTERN
void grl_media_set_play_count (GrlMedia *media, gint play_count);

_GRL_EXTERN
void grl_media_set_last_played (GrlMedia *media, const GDateTime *last_played);

_GRL_EXTERN
void grl_media_set_last_position (GrlMedia *media, gint last_position);

_GRL_EXTERN
void grl_media_set_external_player (GrlMedia *media, const gchar *player);

_GRL_EXTERN
void grl_media_set_external_url (GrlMedia *media, const gchar *url);

_GRL_EXTERN
void grl_media_set_studio (GrlMedia *media, const gchar *studio);

_GRL_EXTERN
void grl_media_set_certificate (GrlMedia *media, const gchar *certificate);

_GRL_EXTERN
void grl_media_set_license (GrlMedia *media, const gchar *license);

_GRL_EXTERN
void grl_media_set_rating (GrlMedia *media, gfloat rating, gfloat max);

_GRL_EXTERN
void grl_media_set_url_data (GrlMedia *media, const gchar *url, const gchar *mime, gint bitrate, gfloat framerate, gint width, gint height);

_GRL_EXTERN
void grl_media_set_favourite (GrlMedia *media, gboolean favourite);

_GRL_EXTERN
void grl_media_set_keyword (GrlMedia *media, const gchar *keyword);

_GRL_EXTERN
void grl_media_set_size (GrlMedia *media, gint64 size);

_GRL_EXTERN
void grl_media_set_track_number (GrlMedia *media, gint track_number);

_GRL_EXTERN
void grl_media_set_bitrate (GrlMedia *media, gint bitrate);

_GRL_EXTERN
void grl_media_set_mb_track_id (GrlMedia *media, const gchar *mb_track_id);

_GRL_EXTERN
void grl_media_set_mb_recording_id (GrlMedia *media, const gchar *mb_recording_id);

_GRL_EXTERN
void grl_media_set_mb_artist_id (GrlMedia *media, const gchar *mb_artist_id);

_GRL_EXTERN G_DEPRECATED
void grl_media_set_mb_album_id (GrlMedia *media, const gchar *mb_album_id);

_GRL_EXTERN
void grl_media_set_mb_release_id (GrlMedia *media, const gchar *mb_release_id);

_GRL_EXTERN
void grl_media_set_mb_release_group_id (GrlMedia *media, const gchar *mb_release_group_id);

_GRL_EXTERN
void grl_media_set_lyrics (GrlMedia *media, const gchar *lyrics);

_GRL_EXTERN
void grl_media_set_genre (GrlMedia *media, const gchar *genre);

_GRL_EXTERN
void grl_media_set_album (GrlMedia *media, const gchar *album);

_GRL_EXTERN
void grl_media_set_album_artist (GrlMedia *media, const gchar *album_artist);

_GRL_EXTERN
void grl_media_set_album_disc_number (GrlMedia *media, gint disc_number);

_GRL_EXTERN
void grl_media_set_artist (GrlMedia *media, const gchar *artist);

_GRL_EXTERN
void grl_media_set_composer (GrlMedia *media, const gchar *composer);

_GRL_EXTERN
void grl_media_set_width (GrlMedia *media, gint width);

_GRL_EXTERN
void grl_media_set_height (GrlMedia *media, gint height);

_GRL_EXTERN
void grl_media_set_framerate (GrlMedia *media, gfloat framerate);

_GRL_EXTERN
void grl_media_set_season (GrlMedia *media, gint season);

_GRL_EXTERN
void grl_media_set_episode (GrlMedia *media, gint episode);

_GRL_EXTERN
void grl_media_set_episode_title (GrlMedia *media, const gchar *episode_title);

_GRL_EXTERN
void grl_media_set_show (GrlMedia *media, const gchar *show);

_GRL_EXTERN
void grl_media_set_performer (GrlMedia *media, const gchar *performer);

_GRL_EXTERN
void grl_media_set_producer (GrlMedia *media, const gchar *producer);

_GRL_EXTERN
void grl_media_set_director (GrlMedia *media, const gchar *director);

_GRL_EXTERN
void grl_media_set_original_title (GrlMedia *media, const gchar *original_title);

_GRL_EXTERN
void grl_media_set_camera_model (GrlMedia *media, const gchar *camera_model);

_GRL_EXTERN
void grl_media_set_flash_used (GrlMedia *media, const gchar *flash_used);

_GRL_EXTERN
void grl_media_set_exposure_time (GrlMedia *media, gfloat exposure_time);

_GRL_EXTERN
void grl_media_set_iso_speed (GrlMedia *media, gfloat iso_speed);

_GRL_EXTERN
void grl_media_set_orientation (GrlMedia *media, gint orientation);

_GRL_EXTERN
void grl_media_set_childcount (GrlMedia *media, gint childcount);

_GRL_EXTERN
void grl_media_add_url_data (GrlMedia *media, const gchar *url, const gchar *mime, gint bitrate, gfloat framerate, gint width, gint height);

_GRL_EXTERN
void grl_media_add_author (GrlMedia *media, const gchar *author);

_GRL_EXTERN
void grl_media_add_thumbnail (GrlMedia *media, const gchar *thumbnail);

_GRL_EXTERN
void grl_media_add_thumbnail_binary (GrlMedia *media, const guint8 *thumbnail, gsize size);

_GRL_EXTERN
void grl_media_add_external_player (GrlMedia *media, const gchar *player);

_GRL_EXTERN
void grl_media_add_external_url (GrlMedia *media, const gchar *url);

_GRL_EXTERN
void grl_media_add_keyword (GrlMedia *media, const gchar *keyword);

_GRL_EXTERN
void grl_media_add_artist (GrlMedia *media, const gchar *artist);

_GRL_EXTERN
void grl_media_add_genre (GrlMedia *media, const gchar *genre);

_GRL_EXTERN
void grl_media_add_lyrics (GrlMedia *media, const gchar *lyrics);

_GRL_EXTERN
void grl_media_add_mb_artist_id (GrlMedia *media, const gchar *mb_artist_id);

_GRL_EXTERN
void grl_media_add_performer (GrlMedia *media, const gchar *performer);

_GRL_EXTERN
void grl_media_add_producer (GrlMedia *media, const gchar *producer);

_GRL_EXTERN
void grl_media_add_director (GrlMedia *media, const gchar *director);

_GRL_EXTERN
const gchar *grl_media_get_id (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_url (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_url_data (GrlMedia *media, gchar **mime, gint *bitrate, gfloat *framerate, gint *width, gint *height);

_GRL_EXTERN
const gchar *grl_media_get_url_data_nth (GrlMedia *media, guint index, gchar **mime, gint *bitrate, gfloat *framerate, gint *width, gint *height);

_GRL_EXTERN
const gchar *grl_media_get_author (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_author_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const gchar *grl_media_get_title (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_description (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_source (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_thumbnail (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_thumbnail_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const guint8 *grl_media_get_thumbnail_binary (GrlMedia *media, gsize *size);

_GRL_EXTERN
const guint8 *grl_media_get_thumbnail_binary_nth (GrlMedia *media, gsize *size, guint index);

_GRL_EXTERN
const gchar *grl_media_get_site (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_duration (GrlMedia *media);

_GRL_EXTERN
GDateTime *grl_media_get_publication_date (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_region (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_region_data(GrlMedia *media,
                                       const GDateTime **publication_date,
                                       const gchar **certificate);

_GRL_EXTERN
const gchar *grl_media_get_region_data_nth(GrlMedia *media,
                                           guint index,
                                           const GDateTime **publication_date,
                                           const gchar **certificate);

_GRL_EXTERN
GDateTime *grl_media_get_creation_date (GrlMedia *media);

_GRL_EXTERN
GDateTime *grl_media_get_modification_date (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_mime (GrlMedia *media);

_GRL_EXTERN
gfloat grl_media_get_rating (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_play_count (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_last_position (GrlMedia *media);

_GRL_EXTERN
GDateTime *grl_media_get_last_played (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_player (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_player_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const gchar *grl_media_get_external_url (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_external_url_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const gchar *grl_media_get_studio (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_certificate (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_license (GrlMedia *media);

_GRL_EXTERN
gfloat grl_media_get_start_time (GrlMedia *media);

_GRL_EXTERN
gboolean grl_media_get_favourite (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_keyword (GrlMedia *media);

_GRL_EXTERN
const gchar * grl_media_get_keyword_nth (GrlMedia *media, guint index);

_GRL_EXTERN
gint64 grl_media_get_size (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_track_number (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_bitrate (GrlMedia *media);

_GRL_EXTERN G_DEPRECATED
const gchar *grl_media_get_mb_album_id (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_mb_artist_id (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_mb_artist_id_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const gchar *grl_media_get_mb_recording_id (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_mb_track_id (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_mb_release_id (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_mb_release_group_id (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_lyrics (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_lyrics_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const gchar *grl_media_get_genre (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_genre_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const gchar *grl_media_get_album (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_album_artist (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_album_disc_number (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_artist (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_artist_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const gchar *grl_media_get_composer (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_composer_nth (GrlMedia *media, guint index);

_GRL_EXTERN
GrlMediaType grl_media_get_media_type (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_width (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_height (GrlMedia *media);

_GRL_EXTERN
gfloat grl_media_get_framerate (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_season (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_episode (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_episode_title (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_show (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_performer (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_performer_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const gchar *grl_media_get_producer (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_producer_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const gchar *grl_media_get_director (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_director_nth (GrlMedia *media, guint index);

_GRL_EXTERN
const gchar *grl_media_get_original_title (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_camera_model (GrlMedia *media);

_GRL_EXTERN
const gchar *grl_media_get_flash_used (GrlMedia *media);

_GRL_EXTERN
gfloat grl_media_get_exposure_time (GrlMedia *media);

_GRL_EXTERN
gfloat grl_media_get_iso_speed (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_orientation (GrlMedia *media);

_GRL_EXTERN
gint grl_media_get_childcount (GrlMedia *media);

_GRL_EXTERN
GType grl_media_get_type (void) G_GNUC_CONST;

_GRL_EXTERN
GrlMedia *grl_media_new (void);

_GRL_EXTERN
GrlMedia *grl_media_audio_new (void);

_GRL_EXTERN
GrlMedia *grl_media_video_new (void);

_GRL_EXTERN
GrlMedia *grl_media_image_new (void);

_GRL_EXTERN
GrlMedia *grl_media_container_new (void);

_GRL_EXTERN
gboolean
grl_media_is_audio (GrlMedia *media);

_GRL_EXTERN
gboolean
grl_media_is_video (GrlMedia *media);

_GRL_EXTERN
gboolean
grl_media_is_image (GrlMedia *media);

_GRL_EXTERN
gboolean
grl_media_is_container (GrlMedia *media);

_GRL_EXTERN
gchar *grl_media_serialize (GrlMedia *media);

_GRL_EXTERN
gchar *grl_media_serialize_extended (GrlMedia *media,
                                     GrlMediaSerializeType serial_type,
                                     ...);

_GRL_EXTERN
GrlMedia *grl_media_unserialize (const gchar *serial);

G_END_DECLS

#endif /* _GRL_MEDIA_H_ */
