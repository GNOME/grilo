/*
 * Copyright (C) 2010, 2011 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
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

#ifndef _GRL_METADATA_KEY_H_
#define _GRL_METADATA_KEY_H_

#include <glib.h>
#include <glib-object.h>

#define GRL_METADATA_KEY_GET_ID(key)   (key)
#define GRL_METADATA_KEY_GET_NAME(key) (grl_metadata_key_get_name (key))
#define GRL_METADATA_KEY_GET_DESC(key) (grl_metadata_key_get_desc (key))
#define GRL_METADATA_KEY_GET_TYPE(key) (grl_metadata_key_get_type (key))

#define GRL_KEYID_FORMAT "u"

#define GRLPOINTER_TO_KEYID(p) (GPOINTER_TO_UINT(p))
#define GRLKEYID_TO_POINTER(k) (GUINT_TO_POINTER(k))

typedef guint32 GrlKeyID;

/**
 * GrlMediaType:
 * @GRL_MEDIA_TYPE_UNKNOWN: unknown media
 * @GRL_MEDIA_TYPE_AUDIO: audio media
 * @GRL_MEDIA_TYPE_VIDEO: video media
 * @GRL_MEDIA_TYPE_IMAGE: image media
 * @GRL_MEDIA_TYPE_CONTAINER: contaddiner media
 */
typedef enum {
  GRL_MEDIA_TYPE_UNKNOWN,
  GRL_MEDIA_TYPE_AUDIO,
  GRL_MEDIA_TYPE_VIDEO,
  GRL_MEDIA_TYPE_IMAGE,
  GRL_MEDIA_TYPE_CONTAINER
} GrlMediaType;

#define g_value_get_grl_key_id(value) ((GrlKeyID) g_value_get_uint(value))
#define g_value_set_grl_key_id(value,key) g_value_set_uint(value,(guint)key)

#define GRL_METADATA_KEY_INVALID              0

#define GRL_METADATA_KEY_ALBUM                1
#define GRL_METADATA_KEY_ARTIST               2
#define GRL_METADATA_KEY_AUTHOR               3
#define GRL_METADATA_KEY_BITRATE              4
#define GRL_METADATA_KEY_CERTIFICATE          5
#define GRL_METADATA_KEY_CHILDCOUNT           6
#define GRL_METADATA_KEY_PUBLICATION_DATE     7
#define GRL_METADATA_KEY_DESCRIPTION          8
#define GRL_METADATA_KEY_DURATION             9
#define GRL_METADATA_KEY_EXTERNAL_PLAYER      10
#define GRL_METADATA_KEY_EXTERNAL_URL         11
#define GRL_METADATA_KEY_FRAMERATE            12
#define GRL_METADATA_KEY_GENRE                13
#define GRL_METADATA_KEY_HEIGHT               14
#define GRL_METADATA_KEY_ID                   15
#define GRL_METADATA_KEY_LAST_PLAYED          16
#define GRL_METADATA_KEY_LAST_POSITION        17
#define GRL_METADATA_KEY_LICENSE              18
#define GRL_METADATA_KEY_LYRICS               19
#define GRL_METADATA_KEY_MIME                 20
#define GRL_METADATA_KEY_PLAY_COUNT           21
#define GRL_METADATA_KEY_RATING               22
#define GRL_METADATA_KEY_SITE                 23
#define GRL_METADATA_KEY_SOURCE               24
#define GRL_METADATA_KEY_STUDIO               25
#define GRL_METADATA_KEY_THUMBNAIL            26
#define GRL_METADATA_KEY_THUMBNAIL_BINARY     27
#define GRL_METADATA_KEY_TITLE                28
#define GRL_METADATA_KEY_URL                  29
#define GRL_METADATA_KEY_WIDTH                30
#define GRL_METADATA_KEY_SEASON               31
#define GRL_METADATA_KEY_EPISODE              32
#define GRL_METADATA_KEY_SHOW                 33
#define GRL_METADATA_KEY_CREATION_DATE        34
#define GRL_METADATA_KEY_CAMERA_MODEL         35
#define GRL_METADATA_KEY_ORIENTATION          36
#define GRL_METADATA_KEY_FLASH_USED           37
#define GRL_METADATA_KEY_EXPOSURE_TIME        38
#define GRL_METADATA_KEY_ISO_SPEED            39
#define GRL_METADATA_KEY_TRACK_NUMBER         40
#define GRL_METADATA_KEY_MODIFICATION_DATE    41
#define GRL_METADATA_KEY_START_TIME           42
#define GRL_METADATA_KEY_FAVOURITE            43
#define GRL_METADATA_KEY_REGION               44
#define GRL_METADATA_KEY_KEYWORD              45
#define GRL_METADATA_KEY_PERFORMER            46
#define GRL_METADATA_KEY_PRODUCER             47
#define GRL_METADATA_KEY_DIRECTOR             48
#define GRL_METADATA_KEY_ORIGINAL_TITLE       49
#define GRL_METADATA_KEY_SIZE                 50
#define GRL_METADATA_KEY_TITLE_FROM_FILENAME  51
#define GRL_METADATA_KEY_MB_ALBUM_ID          52
#define GRL_METADATA_KEY_MB_TRACK_ID          53
#define GRL_METADATA_KEY_MB_ARTIST_ID         54
#define GRL_METADATA_KEY_MB_RECORDING_ID      55
#define GRL_METADATA_KEY_EPISODE_TITLE        56
#define GRL_METADATA_KEY_AUDIO_TRACK          57
#define GRL_METADATA_KEY_ALBUM_DISC_NUMBER    58
#define GRL_METADATA_KEY_COMPOSER             59
#define GRL_METADATA_KEY_ALBUM_ARTIST         60

G_BEGIN_DECLS

const gchar *grl_metadata_key_get_name (GrlKeyID key);

const gchar *grl_metadata_key_get_desc (GrlKeyID key);

GType grl_metadata_key_get_type (GrlKeyID key);

GList *grl_metadata_key_list_new(GrlKeyID first_key, ...);

G_END_DECLS

#endif /* _GRL_METADATA_KEY_H_ */
