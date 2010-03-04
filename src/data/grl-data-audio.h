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

#ifndef _GRL_DATA_AUDIO_H_
#define _GRL_DATA_AUDIO_H_

#include <grl-data-media.h>


G_BEGIN_DECLS

#define GRL_TYPE_DATA_AUDIO                     \
  (grl_data_audio_get_type())

#define GRL_DATA_AUDIO(obj)                             \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_DATA_AUDIO,     \
                               GrlDataAudio))

#define GRL_DATA_AUDIO_CLASS(klass)                     \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_DATA_AUDIO,        \
                            GrlDataAudioClass))

#define GRL_IS_DATA_AUDIO(obj)                          \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_DATA_AUDIO))

#define GRL_IS_DATA_AUDIO_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_DATA_AUDIO))

#define GRL_DATA_AUDIO_GET_CLASS(obj)                   \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_DATA_AUDIO,      \
                              GrlDataAudioClass))

typedef struct _GrlDataAudio      GrlDataAudio;
typedef struct _GrlDataAudioClass GrlDataAudioClass;

struct _GrlDataAudioClass
{
  GrlDataMediaClass parent_class;
};

struct _GrlDataAudio
{
  GrlDataMedia parent;
};

#define grl_data_audio_set_artist(data, artist) \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_ARTIST,  \
                      (artist))

#define grl_data_audio_set_album(data, album)   \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_ALBUM,   \
                      (album))

#define grl_data_audio_set_genre(data, genre)   \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_GENRE,   \
                      (genre))

#define grl_data_audio_set_lyrics(data, lyrics) \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_LYRICS,  \
                      (lyrics))

#define grl_data_audio_set_bitrate(data, bitrate)       \
  grl_data_set_int(GRL_DATA((data)),                    \
                   GRL_METADATA_KEY_BITRATE,            \
                   (bitrate))

#define grl_data_audio_get_artist(data)                                 \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_ARTIST)
#define grl_data_audio_get_album(data)                          \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_ALBUM)
#define grl_data_audio_get_genre(data)                          \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_GENRE)
#define grl_data_audio_get_lyrics(data)                                 \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_LYRICS)
#define grl_data_audio_get_bitrate(data)                        \
  grl_data_get_int(GRL_DATA((data)), GRL_METADATA_KEY_BITRATE)

GType grl_data_audio_get_type (void) G_GNUC_CONST;

GrlDataMedia *grl_data_audio_new (void);

G_END_DECLS

#endif /* _GRL_DATA_AUDIO_H_ */
