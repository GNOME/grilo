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

#define KEYID_FORMAT "u"

#define METADATA_KEY_GET_ID(key)   ((key)->id)
#define METADATA_KEY_GET_NAME(key) ((key)->name)
#define METADATA_KEY_GET_DESC(key) ((key)->desc)

#define METADATA_KEY_TITLE            1
#define METADATA_KEY_TITLE_NAME       "title"
#define METADATA_KEY_TITLE_DESC       "Title of the media"

#define METADATA_KEY_URL              2
#define METADATA_KEY_URL_NAME         "url"
#define METADATA_KEY_URL_DESC         "Media URL"

#define METADATA_KEY_ARTIST           3
#define METADATA_KEY_ARTIST_NAME      "artist"
#define METADATA_KEY_ARTIST_DESC      "Main artist"

#define METADATA_KEY_ALBUM            4
#define METADATA_KEY_ALBUM_NAME       "album"
#define METADATA_KEY_ALBUM_DESC       "Album the media belongs to"

#define METADATA_KEY_GENRE            5
#define METADATA_KEY_GENRE_NAME       "genre"
#define METADATA_KEY_GENRE_DESC       "Genre of the media"

#define METADATA_KEY_THUMBNAIL        6
#define METADATA_KEY_THUMBNAIL_NAME   "thumbnail"
#define METADATA_KEY_THUMBNAIL_DESC   "Thumbnail image"

#define METADATA_KEY_ID               7
#define METADATA_KEY_ID_NAME          "id"
#define METADATA_KEY_ID_DESC          "Identifier of media"

#define METADATA_KEY_AUTHOR           8
#define METADATA_KEY_AUTHOR_NAME      "author"
#define METADATA_KEY_AUTHOR_DESC      "Creator of the media"

#define METADATA_KEY_DESCRIPTION      9
#define METADATA_KEY_DESCRIPTION_NAME "description"
#define METADATA_KEY_DESCRIPTION_DESC "Description of the media"

#define METADATA_KEY_SOURCE           10
#define METADATA_KEY_SOURCE_NAME      "source"
#define METADATA_KEY_SOURCE_DESC      "Source ID providing the content"

#define METADATA_KEY_LYRICS           11
#define METADATA_KEY_LYRICS_NAME      "lyrics"
#define METADATA_KEY_LYRICS_DESC      "Song lyrics"

#define METADATA_KEY_SITE             12
#define METADATA_KEY_SITE_NAME        "site"
#define METADATA_KEY_SITE_DESC        "Site"

typedef guint KeyID;

typedef struct {
  KeyID id;
  gchar *name;
  gchar *desc;
} MetadataKey;

#endif
