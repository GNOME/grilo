/*
 * Copyright (C) 2010 Igalia S.L.
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
#define GRL_METADATA_KEY_GET_NAME(key) (g_param_spec_get_name (key))
#define GRL_METADATA_KEY_GET_DESC(key) (g_param_spec_get_blurb(key))
#define GRL_METADATA_KEY_GET_TYPE(key) (G_PARAM_SPEC_VALUE_TYPE(key))

#define GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN -1

#define GRL_KEYID_FORMAT "p"

#define grl_metadata_key_list_new(first_key, ...)       \
  grl_list_from_va(first_key, ##__VA_ARGS__)


typedef GParamSpec* GrlKeyID;

extern GrlKeyID GRL_METADATA_KEY_TITLE;
extern GrlKeyID GRL_METADATA_KEY_URL;
extern GrlKeyID GRL_METADATA_KEY_EXTERNAL_URL;
extern GrlKeyID GRL_METADATA_KEY_EXTERNAL_PLAYER;
extern GrlKeyID GRL_METADATA_KEY_ARTIST;
extern GrlKeyID GRL_METADATA_KEY_ALBUM;
extern GrlKeyID GRL_METADATA_KEY_GENRE;
extern GrlKeyID GRL_METADATA_KEY_THUMBNAIL;
extern GrlKeyID GRL_METADATA_KEY_ID;
extern GrlKeyID GRL_METADATA_KEY_AUTHOR;
extern GrlKeyID GRL_METADATA_KEY_DESCRIPTION;
extern GrlKeyID GRL_METADATA_KEY_SOURCE;
extern GrlKeyID GRL_METADATA_KEY_LYRICS;
extern GrlKeyID GRL_METADATA_KEY_SITE;
extern GrlKeyID GRL_METADATA_KEY_DURATION;
extern GrlKeyID GRL_METADATA_KEY_DATE;
extern GrlKeyID GRL_METADATA_KEY_CHILDCOUNT;
extern GrlKeyID GRL_METADATA_KEY_MIME;
extern GrlKeyID GRL_METADATA_KEY_WIDTH;
extern GrlKeyID GRL_METADATA_KEY_HEIGHT;
extern GrlKeyID GRL_METADATA_KEY_FRAMERATE;
extern GrlKeyID GRL_METADATA_KEY_RATING;
extern GrlKeyID GRL_METADATA_KEY_BITRATE;
extern GrlKeyID GRL_METADATA_KEY_PLAY_COUNT;
extern GrlKeyID GRL_METADATA_KEY_LAST_PLAYED;
extern GrlKeyID GRL_METADATA_KEY_LAST_POSITION;

#endif /* _GRL_METADATA_KEY_H_ */
