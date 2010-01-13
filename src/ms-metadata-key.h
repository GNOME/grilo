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

#ifndef _MS_METADATA_KEY_H_
#define _MS_METADATA_KEY_H_

#include <glib.h>

#define MS_METADATA_KEY_GET_ID(key)   ((key)->id)
#define MS_METADATA_KEY_GET_NAME(key) ((key)->name)
#define MS_METADATA_KEY_GET_DESC(key) ((key)->desc)

#define MS_METADATA_KEY_TITLE            1
#define MS_METADATA_KEY_TITLE_NAME       "title"
#define MS_METADATA_KEY_TITLE_DESC       "Title of the media"

#define MS_METADATA_KEY_URL              2
#define MS_METADATA_KEY_URL_NAME         "url"
#define MS_METADATA_KEY_URL_DESC         "Media URL"

#define MS_METADATA_KEY_ARTIST           3
#define MS_METADATA_KEY_ARTIST_NAME      "artist"
#define MS_METADATA_KEY_ARTIST_DESC      "Main artist"

#define MS_METADATA_KEY_ALBUM            4
#define MS_METADATA_KEY_ALBUM_NAME       "album"
#define MS_METADATA_KEY_ALBUM_DESC       "Album the media belongs to"

#define MS_METADATA_KEY_GENRE            5
#define MS_METADATA_KEY_GENRE_NAME       "genre"
#define MS_METADATA_KEY_GENRE_DESC       "Genre of the media"

#define MS_METADATA_KEY_THUMBNAIL        6
#define MS_METADATA_KEY_THUMBNAIL_NAME   "thumbnail"
#define MS_METADATA_KEY_THUMBNAIL_DESC   "Thumbnail image"

#define MS_METADATA_KEY_ID               7
#define MS_METADATA_KEY_ID_NAME          "id"
#define MS_METADATA_KEY_ID_DESC          "Identifier of media"

#define MS_METADATA_KEY_AUTHOR           8
#define MS_METADATA_KEY_AUTHOR_NAME      "author"
#define MS_METADATA_KEY_AUTHOR_DESC      "Creator of the media"

#define MS_METADATA_KEY_DESCRIPTION      9
#define MS_METADATA_KEY_DESCRIPTION_NAME "description"
#define MS_METADATA_KEY_DESCRIPTION_DESC "Description of the media"

#define MS_METADATA_KEY_SOURCE           10
#define MS_METADATA_KEY_SOURCE_NAME      "source"
#define MS_METADATA_KEY_SOURCE_DESC      "Source ID providing the content"

#define MS_METADATA_KEY_LYRICS           11
#define MS_METADATA_KEY_LYRICS_NAME      "lyrics"
#define MS_METADATA_KEY_LYRICS_DESC      "Song lyrics"

#define MS_METADATA_KEY_SITE             12
#define MS_METADATA_KEY_SITE_NAME        "site"
#define MS_METADATA_KEY_SITE_DESC        "Site"

#define MS_METADATA_KEY_DURATION         13
#define MS_METADATA_KEY_DURATION_NAME    "duration"
#define MS_METADATA_KEY_DURATION_DESC    "Media duration"

#define MS_METADATA_KEY_DATE             14
#define MS_METADATA_KEY_DATE_NAME        "date"
#define MS_METADATA_KEY_DATE_DESC        "Publishing or recording date"

#define MS_METADATA_KEY_CHILDCOUNT       15
#define MS_METADATA_KEY_CHILDCOUNT_NAME  "childcount"
#define MS_METADATA_KEY_CHILDCOUNT_DESC  "Number of items contained in a container"
#define MS_METADATA_KEY_CHILDCOUNT_UNKNOWN -1

#define MS_METADATA_KEY_MIME             16
#define MS_METADATA_KEY_MIME_NAME        "mime-type"
#define MS_METADATA_KEY_MIME_DESC        "Media mime type"

#define MS_METADATA_KEY_WIDTH            17
#define MS_METADATA_KEY_WIDTH_NAME       "width"
#define MS_METADATA_KEY_WIDTH_DESC       "Width of video (x-axis resolution)"

#define MS_METADATA_KEY_HEIGHT           18
#define MS_METADATA_KEY_HEIGHT_NAME      "height"
#define MS_METADATA_KEY_HEIGHT_DESC      "height of video (y-axis resolution)"

#define MS_METADATA_KEY_FRAMERATE        19
#define MS_METADATA_KEY_FRAMERATE_NAME   "framerate"
#define MS_METADATA_KEY_FRAMERATE_DESC   "Frames per second"

#define MS_KEYID_FORMAT "u"

#define POINTER_TO_MSKEYID(p) GPOINTER_TO_UINT((p))
#define MSKEYID_TO_POINTER(m) GUINT_TO_POINTER((m))

typedef guint MsKeyID;

typedef struct {
  MsKeyID id;
  gchar *name;
  gchar *desc;
} MsMetadataKey;

G_BEGIN_DECLS

GList *ms_metadata_key_list_new (MsKeyID first_key, ...);

G_END_DECLS

#endif
