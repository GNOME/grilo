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

/**
 * SECTION:grl-media-audio
 * @short_description: A multimedia data for audio
 * @see_also: #GrlConfig, #GrlMediaBox, #GrlMediaVideo, #GrlMediaImage
 *
 * This high level class represents an audio multimedia item. It has methods to
 * set and get properties like artist, album, and so on.
 */

#include "grl-media-audio.h"


static void grl_media_audio_dispose (GObject *object);
static void grl_media_audio_finalize (GObject *object);

G_DEFINE_TYPE (GrlMediaAudio, grl_media_audio, GRL_TYPE_MEDIA);

static void
grl_media_audio_class_init (GrlMediaAudioClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_media_audio_dispose;
  gobject_class->finalize = grl_media_audio_finalize;
}

static void
grl_media_audio_init (GrlMediaAudio *self)
{
}

static void
grl_media_audio_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_media_audio_parent_class)->dispose (object);
}

static void
grl_media_audio_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_media_audio_parent_class)->finalize (object);
}

/**
 * grl_media_audio_new:
 *
 * Creates a new data audio object.
 *
 * Returns: a newly-allocated data audio.
 *
 * Since: 0.1.4
 **/
GrlMedia *
grl_media_audio_new (void)
{
  return GRL_MEDIA (g_object_new (GRL_TYPE_MEDIA_AUDIO,
                                  NULL));
}

/**
 * grl_media_audio_set_artist:
 * @data: the media instance
 * @artist: the audio's artist
 *
 * Set the artist of the audio
 *
 * Since: 0.1.4
 */
void
grl_media_audio_set_artist (GrlMediaAudio *data, const gchar *artist)
{
  grl_data_set_string (GRL_DATA (data), GRL_METADATA_KEY_ARTIST,
                       artist);
}

/**
 * grl_media_audio_set_album:
 * @data: the media instance
 * @album: the audio's album
 *
 * Set the album of the audio
 *
 * Since: 0.1.4
 */
void
grl_media_audio_set_album (GrlMediaAudio *data, const gchar *album)
{
  grl_data_set_string (GRL_DATA (data), GRL_METADATA_KEY_ALBUM,
                       album);
}

/**
 * grl_media_audio_set_genre:
 * @data: the media instance
 * @genre: the audio's genre
 *
 * Set the genre of the audio
 *
 * Since: 0.1.4
 */
void
grl_media_audio_set_genre (GrlMediaAudio *data, const gchar *genre)
{
  grl_data_set_string (GRL_DATA (data), GRL_METADATA_KEY_GENRE,
                       genre);
}

/**
 * grl_media_audio_set_lyrics:
 * @data: the media instance
 * @lyrics: the audio's lyrics
 *
 * Set the lyrics of the audio
 *
 * Since: 0.1.4
 */
void
grl_media_audio_set_lyrics (GrlMediaAudio *data, const gchar *lyrics)
{
  grl_data_set_string (GRL_DATA (data), GRL_METADATA_KEY_LYRICS,
                       lyrics);
}

/**
 * grl_media_audio_set_bitrate:
 * @data: the media instance
 * @bitrate: the audio's bitrate
 *
 * Set the bitrate of the audio
 *
 * Since: 0.1.4
 */
void
grl_media_audio_set_bitrate (GrlMediaAudio *data, gint bitrate)
{
  grl_data_set_int (GRL_DATA (data), GRL_METADATA_KEY_BITRATE,
                    bitrate);
}

/**
 * grl_media_audio_get_artist:
 * @data: the media instance
 *
 * Returns: the artist of the audio
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_audio_get_artist (GrlMediaAudio *data)
{
  return grl_data_get_string (GRL_DATA (data), GRL_METADATA_KEY_ARTIST);
}

/**
 * grl_media_audio_get_album:
 * @data: the media instance
 *
 * Returns: the album of the audio
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_audio_get_album (GrlMediaAudio *data)
{
  return grl_data_get_string (GRL_DATA (data), GRL_METADATA_KEY_ALBUM);
}

/**
 * grl_media_audio_get_genre:
 * @data: the media instance
 *
 * Returns: the genre of the audio
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_audio_get_genre (GrlMediaAudio *data)
{
  return grl_data_get_string (GRL_DATA (data), GRL_METADATA_KEY_GENRE);
}

/**
 * grl_media_audio_get_lyrics:
 * @data: the media instance
 *
 * Returns: the lyrics of the audio
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_audio_get_lyrics (GrlMediaAudio *data)
{
  return grl_data_get_string (GRL_DATA (data), GRL_METADATA_KEY_LYRICS);
}

/**
 * grl_media_audio_get_bitrate:
 * @data: the media instance
 *
 * Returns: the bitrate of the audio
 *
 * Since: 0.1.4
 */
gint
grl_media_audio_get_bitrate (GrlMediaAudio *data)
{
  return grl_data_get_int (GRL_DATA (data), GRL_METADATA_KEY_BITRATE);
}
