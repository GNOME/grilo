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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#ifndef _GRL_MEDIA_AUDIO_H_
#define _GRL_MEDIA_AUDIO_H_

#include <grl-media.h>
#include <grl-definitions.h>

G_BEGIN_DECLS

#define GRL_TYPE_MEDIA_AUDIO                    \
  (grl_media_audio_get_type())

#define GRL_MEDIA_AUDIO(obj)                            \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_MEDIA_AUDIO,    \
                               GrlMediaAudio))

#define GRL_MEDIA_AUDIO_CLASS(klass)                    \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_MEDIA_AUDIO,       \
                            GrlMediaAudioClass))

#define GRL_IS_MEDIA_AUDIO(obj)                         \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_MEDIA_AUDIO))

#define GRL_IS_MEDIA_AUDIO_CLASS(klass)                 \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_MEDIA_AUDIO))

#define GRL_MEDIA_AUDIO_GET_CLASS(obj)                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_MEDIA_AUDIO,     \
                              GrlMediaAudioClass))

typedef struct _GrlMediaAudio      GrlMediaAudio;
typedef struct _GrlMediaAudioClass GrlMediaAudioClass;

/**
 * GrlMediaAudioClass:
 * @parent_class: the parent class structure
 *
 * Grilo Media audio Class
 */
struct _GrlMediaAudioClass
{
  GrlMediaClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

struct _GrlMediaAudio
{
  GrlMedia parent;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING_SMALL];
};

void grl_media_audio_set_artist (GrlMediaAudio *data, const gchar *artist);

void grl_media_audio_set_album (GrlMediaAudio *data, const gchar *album);

void grl_media_audio_set_genre (GrlMediaAudio *data, const gchar *genre);

void grl_media_audio_set_lyrics (GrlMediaAudio *data, const gchar *lyrics);

void grl_media_audio_set_bitrate (GrlMediaAudio *data, gint bitrate);

const gchar *grl_media_audio_get_artist (GrlMediaAudio *data);

const gchar *grl_media_audio_get_album (GrlMediaAudio *data);

const gchar *grl_media_audio_get_genre (GrlMediaAudio *data);

const gchar *grl_media_audio_get_lyrics (GrlMediaAudio *data);

gint grl_media_audio_get_bitrate (GrlMediaAudio *data);

GType grl_media_audio_get_type (void) G_GNUC_CONST;

GrlMedia *grl_media_audio_new (void);

G_END_DECLS

#endif /* _GRL_MEDIA_AUDIO_H_ */
