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
#define GRL_METADATA_KEY_INVALID 0

#define GRLPOINTER_TO_KEYID(p) (GPOINTER_TO_UINT(p))
#define GRLKEYID_TO_POINTER(k) (GUINT_TO_POINTER(k))

typedef guint32 GrlKeyID;

extern GrlKeyID GRL_METADATA_KEY_ALBUM;
extern GrlKeyID GRL_METADATA_KEY_ARTIST;
extern GrlKeyID GRL_METADATA_KEY_AUTHOR;
extern GrlKeyID GRL_METADATA_KEY_BITRATE;
extern GrlKeyID GRL_METADATA_KEY_CERTIFICATE;
extern GrlKeyID GRL_METADATA_KEY_CHILDCOUNT;
extern GrlKeyID GRL_METADATA_KEY_DATE;
extern GrlKeyID GRL_METADATA_KEY_DESCRIPTION;
extern GrlKeyID GRL_METADATA_KEY_DURATION;
extern GrlKeyID GRL_METADATA_KEY_EXTERNAL_PLAYER;
extern GrlKeyID GRL_METADATA_KEY_EXTERNAL_URL;
extern GrlKeyID GRL_METADATA_KEY_FRAMERATE;
extern GrlKeyID GRL_METADATA_KEY_GENRE;
extern GrlKeyID GRL_METADATA_KEY_HEIGHT;
extern GrlKeyID GRL_METADATA_KEY_ID;
extern GrlKeyID GRL_METADATA_KEY_LAST_PLAYED;
extern GrlKeyID GRL_METADATA_KEY_LAST_POSITION;
extern GrlKeyID GRL_METADATA_KEY_LICENSE;
extern GrlKeyID GRL_METADATA_KEY_LYRICS;
extern GrlKeyID GRL_METADATA_KEY_MIME;
extern GrlKeyID GRL_METADATA_KEY_PLAY_COUNT;
extern GrlKeyID GRL_METADATA_KEY_RATING;
extern GrlKeyID GRL_METADATA_KEY_SITE;
extern GrlKeyID GRL_METADATA_KEY_SOURCE;
extern GrlKeyID GRL_METADATA_KEY_STUDIO;
extern GrlKeyID GRL_METADATA_KEY_THUMBNAIL;
extern GrlKeyID GRL_METADATA_KEY_THUMBNAIL_BINARY;
extern GrlKeyID GRL_METADATA_KEY_TITLE;
extern GrlKeyID GRL_METADATA_KEY_URL;
extern GrlKeyID GRL_METADATA_KEY_WIDTH;
extern GrlKeyID GRL_METADATA_KEY_SEASON;
extern GrlKeyID GRL_METADATA_KEY_EPISODE;
extern GrlKeyID GRL_METADATA_KEY_SHOW;
extern GrlKeyID GRL_METADATA_KEY_CREATION_DATE;
extern GrlKeyID GRL_METADATA_KEY_CAMERA_MODEL;
extern GrlKeyID GRL_METADATA_KEY_ORIENTATION;
extern GrlKeyID GRL_METADATA_KEY_FLASH_USED;
extern GrlKeyID GRL_METADATA_KEY_EXPOSURE_TIME;
extern GrlKeyID GRL_METADATA_KEY_ISO_SPEED;

const gchar *grl_metadata_key_get_name (GrlKeyID key);

const gchar *grl_metadata_key_get_desc (GrlKeyID key);

GType grl_metadata_key_get_type (GrlKeyID key);

GList *grl_metadata_key_list_new(GrlKeyID first_key, ...);

#endif /* _GRL_METADATA_KEY_H_ */
