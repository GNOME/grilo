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

#define GRL_METADATA_KEY_GET_ID(key)   ((key)->id)
#define GRL_METADATA_KEY_GET_NAME(key) ((key)->name)
#define GRL_METADATA_KEY_GET_DESC(key) ((key)->desc)

#define GRL_METADATA_KEY_TITLE              1
#define GRL_METADATA_KEY_TITLE_NAME         "title"
#define GRL_METADATA_KEY_TITLE_DESC         "Title of the media"

#define GRL_METADATA_KEY_URL                2
#define GRL_METADATA_KEY_URL_NAME           "url"
#define GRL_METADATA_KEY_URL_DESC           "Media URL"

#define GRL_METADATA_KEY_ARTIST             3
#define GRL_METADATA_KEY_ARTIST_NAME        "artist"
#define GRL_METADATA_KEY_ARTIST_DESC        "Main artist"

#define GRL_METADATA_KEY_ALBUM              4
#define GRL_METADATA_KEY_ALBUM_NAME         "album"
#define GRL_METADATA_KEY_ALBUM_DESC         "Album the media belongs to"

#define GRL_METADATA_KEY_GENRE              5
#define GRL_METADATA_KEY_GENRE_NAME         "genre"
#define GRL_METADATA_KEY_GENRE_DESC         "Genre of the media"

#define GRL_METADATA_KEY_THUMBNAIL          6
#define GRL_METADATA_KEY_THUMBNAIL_NAME     "thumbnail"
#define GRL_METADATA_KEY_THUMBNAIL_DESC     "Thumbnail image"

#define GRL_METADATA_KEY_ID                 7
#define GRL_METADATA_KEY_ID_NAME            "id"
#define GRL_METADATA_KEY_ID_DESC            "Identifier of media"

#define GRL_METADATA_KEY_AUTHOR             8
#define GRL_METADATA_KEY_AUTHOR_NAME        "author"
#define GRL_METADATA_KEY_AUTHOR_DESC        "Creator of the media"

#define GRL_METADATA_KEY_DESCRIPTION        9
#define GRL_METADATA_KEY_DESCRIPTION_NAME   "description"
#define GRL_METADATA_KEY_DESCRIPTION_DESC   "Description of the media"

#define GRL_METADATA_KEY_SOURCE             10
#define GRL_METADATA_KEY_SOURCE_NAME        "source"
#define GRL_METADATA_KEY_SOURCE_DESC        "Source ID providing the content"

#define GRL_METADATA_KEY_LYRICS             11
#define GRL_METADATA_KEY_LYRICS_NAME        "lyrics"
#define GRL_METADATA_KEY_LYRICS_DESC        "Song lyrics"

#define GRL_METADATA_KEY_SITE               12
#define GRL_METADATA_KEY_SITE_NAME          "site"
#define GRL_METADATA_KEY_SITE_DESC          "Site"

#define GRL_METADATA_KEY_DURATION           13
#define GRL_METADATA_KEY_DURATION_NAME      "duration"
#define GRL_METADATA_KEY_DURATION_DESC      "Media duration"

#define GRL_METADATA_KEY_DATE               14
#define GRL_METADATA_KEY_DATE_NAME          "date"
#define GRL_METADATA_KEY_DATE_DESC          "Publishing or recording date"

#define GRL_METADATA_KEY_CHILDCOUNT         15
#define GRL_METADATA_KEY_CHILDCOUNT_NAME    "childcount"
#define GRL_METADATA_KEY_CHILDCOUNT_DESC    "Number of items contained in a container"
#define GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN -1

#define GRL_METADATA_KEY_MIME               16
#define GRL_METADATA_KEY_MIME_NAME          "mime-type"
#define GRL_METADATA_KEY_MIME_DESC          "Media mime type"

#define GRL_METADATA_KEY_WIDTH              17
#define GRL_METADATA_KEY_WIDTH_NAME         "width"
#define GRL_METADATA_KEY_WIDTH_DESC         "Width of media ('x' resolution)"

#define GRL_METADATA_KEY_HEIGHT             18
#define GRL_METADATA_KEY_HEIGHT_NAME        "height"
#define GRL_METADATA_KEY_HEIGHT_DESC        "height of media ('y' resolution)"

#define GRL_METADATA_KEY_FRAMERATE          19
#define GRL_METADATA_KEY_FRAMERATE_NAME     "framerate"
#define GRL_METADATA_KEY_FRAMERATE_DESC     "Frames per second"

#define GRL_METADATA_KEY_RATING             20
#define GRL_METADATA_KEY_RATING_NAME        "rating"
#define GRL_METADATA_KEY_RATING_DESC        "Media rating"

#define GRL_METADATA_KEY_BITRATE            21
#define GRL_METADATA_KEY_BITRATE_NAME       "bitrate"
#define GRL_METADATA_KEY_BITRATE_DESC       "Media bitrate in Kbits/s"

#define GRL_METADATA_KEY_PLAY_COUNT         22
#define GRL_METADATA_KEY_PLAY_COUNT_NAME    "play-count"
#define GRL_METADATA_KEY_PLAY_COUNT_DESC    "Media play count"

#define GRL_METADATA_KEY_LAST_PLAYED        23
#define GRL_METADATA_KEY_LAST_PLAYED_NAME   "last-played-time"
#define GRL_METADATA_KEY_LAST_PLAYED_DESC   "Last time the media was played"

#define GRL_METADATA_KEY_LAST_POSITION      24
#define GRL_METADATA_KEY_LAST_POSITION_NAME "playback-interrupted-time"
#define GRL_METADATA_KEY_LAST_POSITION_DESC "Time at which playback was interrupted"

#define GRL_KEYID_FORMAT "u"

#define POINTER_TO_GRLKEYID(p) GPOINTER_TO_UINT((p))
#define GRLKEYID_TO_POINTER(m) GUINT_TO_POINTER((m))

typedef guint GrlKeyID;

typedef struct {
  GrlKeyID id;
  gchar *name;
  gchar *desc;
} GrlMetadataKey;

G_BEGIN_DECLS

GList *grl_metadata_key_list_new (GrlKeyID first_key, ...);

G_END_DECLS

#endif /* _GRL_METADATA_KEY_H_ */
