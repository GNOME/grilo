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

#ifndef __MS_CONTENT_AUDIO_H__
#define __MS_CONTENT_AUDIO_H__

#include <ms-content-media.h>


G_BEGIN_DECLS

#define MS_TYPE_CONTENT_AUDIO                   \
  (ms_content_audio_get_type())
#define MS_CONTENT_AUDIO(obj)                                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                   \
                               MS_TYPE_CONTENT_AUDIO,                   \
                               MsContentAudio))
#define MS_CONTENT_AUDIO_CLASS(klass)                                   \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                                    \
                            MS_TYPE_CONTENT_AUDIO,                      \
                            MsContentAudioClass))
#define MS_IS_CONTENT_AUDIO(obj)                                        \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                   \
                               MS_TYPE_CONTENT_AUDIO))
#define MS_IS_CONTENT_AUDIO_CLASS(klass)                                \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                                    \
                            MS_TYPE_CONTENT_AUDIO))
#define MS_CONTENT_AUDIO_GET_CLASS(obj)                                 \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                                    \
                              MS_TYPE_CONTENT_AUDIO,                    \
                              MsContentAudioClass))

typedef struct _MsContentAudio      MsContentAudio;
typedef struct _MsContentAudioClass MsContentAudioClass;

struct _MsContentAudioClass
{
    MsContentMediaClass parent_class;
};

struct _MsContentAudio
{
    MsContentMedia parent;
};

#define ms_content_audio_set_artist(content, artist)                    \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_ARTIST, (artist))
#define ms_content_audio_set_album(content, album)                      \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_ALBUM, (album))
#define ms_content_audio_set_genre(content, genre)                      \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_GENRE, (genre))
#define ms_content_audio_set_lyrics(content, lyrics)                    \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_LYRICS, (lyrics))

#define ms_content_audio_get_artist(content)                            \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_ARTIST)
#define ms_content_audio_get_album(content)                             \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_ALBUM)
#define ms_content_audio_get_genre(content)                             \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_GENRE)
#define ms_content_audio_get_lyrics(content)                            \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_LYRICS)

GType ms_content_audio_get_type (void) G_GNUC_CONST;
MsContentMedia *ms_content_audio_new (void);

G_END_DECLS

#endif /* __CONTENT_AUDIO_H__ */
