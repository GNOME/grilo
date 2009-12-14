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

#ifndef _METADATA_KEY_H_
#define _METADATA_KEY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#define METADATA_KEY_INFO_ID(key) (key->id)
#define METADATA_KEY_INFO_NAME(key) (key->name)
#define METADATA_KEY_INFO_DESC(key) (key->desc)
#define METADATA_KEY_INFO_DEPENDS(key) (key->depends)

#define METADATA_KEY_TITLE           1
#define METADATA_KEY_TITLE_NAME      "title"
#define METADATA_KEY_TITLE_DESC      "Title of the media"

#define METADATA_KEY_ARTIST          2
#define METADATA_KEY_ARTIST_NAME     "artist"
#define METADATA_KEY_ARTIST_DESC     "Main artist"

#define METADATA_KEY_ALBUM           3
#define METADATA_KEY_ALBUM_NAME      "album"
#define METADATA_KEY_ALBUM_DESC      "Album the media belongs to"

#define METADATA_KEY_GENRE           4
#define METADATA_KEY_GENRE_NAME      "genre"
#define METADATA_KEY_GENRE_DESC      "Genre of the media"

#define METADATA_KEY_THUMBNAIL       5
#define METADATA_KEY_THUMBNAIL_NAME  "thumbnail"
#define METADATA_KEY_THUMBNAIL_DESC  "Thumbnail image"

typedef struct {
  guint id;
  gchar *name;
  gchar *desc;
} MetadataKey;

#endif
