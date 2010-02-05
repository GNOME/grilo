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

#ifndef _GRL_CONTENT_AUDIO_H_
#define _GRL_CONTENT_AUDIO_H_

#include <grl-content-media.h>


G_BEGIN_DECLS

#define GRL_TYPE_CONTENT_AUDIO                  \
  (grl_content_audio_get_type())

#define GRL_CONTENT_AUDIO(obj)                          \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_CONTENT_AUDIO,  \
                               GrlContentAudio))

#define GRL_CONTENT_AUDIO_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_CONTENT_AUDIO,     \
                            GrlContentAudioClass))

#define GRL_IS_CONTENT_AUDIO(obj)                       \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_CONTENT_AUDIO))

#define GRL_IS_CONTENT_AUDIO_CLASS(klass)               \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_CONTENT_AUDIO))

#define GRL_CONTENT_AUDIO_GET_CLASS(obj)                \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_CONTENT_AUDIO,   \
                              GrlContentAudioClass))

typedef struct _GrlContentAudio      GrlContentAudio;
typedef struct _GrlContentAudioClass GrlContentAudioClass;

struct _GrlContentAudioClass
{
  GrlContentMediaClass parent_class;
};

struct _GrlContentAudio
{
  GrlContentMedia parent;
};

#define grl_content_audio_set_artist(content, artist)   \
  grl_content_set_string(GRL_CONTENT((content)),        \
                         GRL_METADATA_KEY_ARTIST,       \
                         (artist))

#define grl_content_audio_set_album(content, album)                     \
  grl_content_set_string(GRL_CONTENT((content)),                        \
                         GRL_METADATA_KEY_ALBUM,                        \
                         (album))

#define grl_content_audio_set_genre(content, genre)     \
  grl_content_set_string(GRL_CONTENT((content)),        \
                         GRL_METADATA_KEY_GENRE,        \
                         (genre))

#define grl_content_audio_set_lyrics(content, lyrics)   \
  grl_content_set_string(GRL_CONTENT((content)),        \
                         GRL_METADATA_KEY_LYRICS,       \
                         (lyrics))

#define grl_content_audio_get_artist(content)                           \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_ARTIST)
#define grl_content_audio_get_album(content)                            \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_ALBUM)
#define grl_content_audio_get_genre(content)                            \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_GENRE)
#define grl_content_audio_get_lyrics(content)                           \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_LYRICS)

GType grl_content_audio_get_type (void) G_GNUC_CONST;

GrlContentMedia *grl_content_audio_new (void);

G_END_DECLS

#endif /* _CONTENT_AUDIO_H_ */
