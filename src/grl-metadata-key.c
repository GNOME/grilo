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

/**
 * SECTION:grl-metadata-key
 * @short_description: General media key definition
 *
 * This is the list of defined keys in grilo for media entries.
 */

#include "grl-metadata-key.h"
#include "grl-metadata-key-priv.h"
#include "grl-registry-priv.h"
#include "grl-definitions.h"

#include <stdarg.h>

void
grl_metadata_key_setup_system_keys (GrlRegistry *registry)
{
  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("album",
                                                                "Album",
                                                                "Album the media belongs to",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_ALBUM,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("artist",
                                                                "Artist",
                                                                "Main artist",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_ARTIST,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("author",
                                                                "Author",
                                                                "Creator of the media",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_AUTHOR,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_boxed ("publishing-date",
                                                               "Publishing date",
                                                               "When the media was originally published",
                                                               G_TYPE_DATE_TIME,
                                                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_PUBLICATION_DATE,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("description",
                                                                "Description",
                                                                "Description of the media",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_DESCRIPTION,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("genre",
                                                                "Genre",
                                                                "Genre of the media",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_GENRE,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("id",
                                                                "ID",
                                                                "Identifier of media",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_ID,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("last-played-time",
                                                                "LastPlayedTime",
                                                                "Last time the media was played",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_LAST_PLAYED,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("lyrics",
                                                                "Lyrics",
                                                                "Song lyrics",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_LYRICS,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("mime-type",
                                                                "MimeType",
                                                                "Media mime type",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_MIME,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("site",
                                                                "Site",
                                                                "Site",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_SITE,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("source",
                                                                "Source",
                                                                "Source ID providing the content",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_SOURCE,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("thumbnail",
                                                                "Thumbnail",
                                                                "Thumbnail image",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_THUMBNAIL,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_boxed ("thumbnail-binary",
                                                               "Thumbnail Binary",
                                                               "Thumbnail binary image",
                                                               G_TYPE_BYTE_ARRAY,
                                                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_THUMBNAIL_BINARY,

                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("title",
                                                                "Title",
                                                                "Title of the media",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_TITLE,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("url",
                                                                "URL",
                                                                "Media URL",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_URL,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("external-url",
                                                                "External URL",
                                                                "External location where the media can be played back, usually a website",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_EXTERNAL_URL,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("external-player",
                                                                "External Player URL",
                                                                "URL of an external player that can be used to play the resource (usually a Flash player)",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_EXTERNAL_PLAYER,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("studio",
                                                                "Studio",
                                                                "Studio the media is from",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_STUDIO,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("certificate",
                                                                "Certificate",
                                                                "Age certificate of the media",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_CERTIFICATE,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("license",
                                                                "License",
                                                                "The license of the media",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_LICENSE,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("bitrate",
                                                             "Bitrate",
                                                             "Media bitrate in KBits/s",
                                                             0, G_MAXINT,
                                                             0,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_BITRATE,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("childcount",
                                                             "Childcount",
                                                             "Number of items contained in a container",
                                                             -1, G_MAXINT,
                                                             GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_CHILDCOUNT,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("duration",
                                                             "Duration",
                                                             "Media duration in seconds",
                                                             0, G_MAXINT,
                                                             0,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_DURATION,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("height",
                                                             "Height",
                                                             "Height of media ('y' axis)",
                                                             0, G_MAXINT,
                                                             0,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_HEIGHT,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("playback-interrupted-time",
                                                             "PlaybackInterruptedTime",
                                                             "Time at which playback was interrupted",
                                                             0, G_MAXINT,
                                                             0,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_LAST_POSITION,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("play-count",
                                                             "PlayCount",
                                                             "How many times media was played",
                                                             0, G_MAXINT,
                                                             0,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_PLAY_COUNT,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("width",
                                                             "Width",
                                                             "Width of media ('x' axis)",
                                                             0, G_MAXINT,
                                                             0,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_WIDTH,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_float ("framerate",
                                                               "Framerate",
                                                               "Frames per second",
                                                               0, G_MAXFLOAT,
                                                               0,
                                                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_FRAMERATE,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_float ("rating",
                                                               "Rating",
                                                               "Media rating",
                                                               0, G_MAXFLOAT,
                                                               0,
                                                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_RATING,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("season",
                                                             "Season",
                                                             "Season of a show",
                                                             0, G_MAXINT,
                                                             0,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_SEASON,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("episode",
                                                             "Episode",
                                                             "Episode of a show",
                                                             0, G_MAXINT,
                                                             0,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_EPISODE,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("show",
                                                                "Show",
                                                                "Name of a show",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_SHOW,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_boxed ("creation-date",
                                                               "Creation date",
                                                               "Creation date",
                                                               G_TYPE_DATE_TIME,
                                                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_CREATION_DATE,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("camera-model",
                                                                "Camera model",
                                                                "Name of the camera model used to take the photo",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_CAMERA_MODEL,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("orientation",
                                                             "Picture orientation",
                                                             "Orientation of the photo in degree (clockwise)",
                                                             0, 359,
                                                             0,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_ORIENTATION,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_boolean ("flash-used",
                                                                 "Flash used",
                                                                 "Whether or not a flash was used to take that picture",
                                                                 FALSE,
                                                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_FLASH_USED,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_float ("exposure-time",
                                                               "Exposure time",
                                                               "Exposure time of the photo in seconds",
                                                               0, G_MAXFLOAT,
                                                               0,
                                                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_EXPOSURE_TIME,
                                           NULL);


  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_float ("iso-speed",
                                                               "ISO speed",
                                                               "Photographic film's sensitivity to light as ISO value",
                                                               0, G_MAXFLOAT,
                                                               0,
                                                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_ISO_SPEED,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_int ("track-number",
                                                             "Track number",
                                                             "Track number inside the album",
                                                             1, G_MAXINT,
                                                             1,
                                                             G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_TRACK_NUMBER,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_boxed ("modification-date",
                                                               "Modification date",
                                                               "When the media was last modified",
                                                               G_TYPE_DATE_TIME,
                                                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_MODIFICATION_DATE,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_float ("start-time",
                                                               "Start Time",
                                                               "Start offset in seconds relative to container",
                                                               0.0, G_MAXFLOAT,
                                                               0.0,
                                                               G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_START_TIME,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_boolean ("favourite",
                                                                 "Favourite",
                                                                 "Whether or not the element was marked as favourite",
                                                                 FALSE,
                                                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_FAVOURITE,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("region",
                                                                "Region",
                                                                "Region in which the media was published",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_REGION,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("keyword",
                                                                "keyword",
                                                                "A keyword describing the media",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_KEYWORD,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("performer",
                                                                "performer",
                                                                "An actor performing in the movie",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_PERFORMER,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("producer",
                                                                "Producer",
                                                                "Producer of the movie",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_PRODUCER,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("director",
                                                                "Director",
                                                                "Director of the movie",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_DIRECTOR,
                                           NULL);

  grl_registry_register_metadata_key_full (registry,
                                           g_param_spec_string ("original-title",
                                                                "Original Title",
                                                                "Original, untranslated title of the movie",
                                                                NULL,
                                                                G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                           GRL_METADATA_KEY_ORIGINAL_TITLE,
                                           NULL);

  /* Create the relations */
  grl_registry_register_metadata_key_relation (registry,
                                               GRL_METADATA_KEY_URL,
                                               GRL_METADATA_KEY_MIME);
  grl_registry_register_metadata_key_relation (registry,
                                               GRL_METADATA_KEY_URL,
                                               GRL_METADATA_KEY_BITRATE);
  grl_registry_register_metadata_key_relation (registry,
                                               GRL_METADATA_KEY_URL,
                                               GRL_METADATA_KEY_FRAMERATE);
  grl_registry_register_metadata_key_relation (registry,
                                               GRL_METADATA_KEY_URL,
                                               GRL_METADATA_KEY_HEIGHT);
  grl_registry_register_metadata_key_relation (registry,
                                               GRL_METADATA_KEY_URL,
                                               GRL_METADATA_KEY_WIDTH);
  grl_registry_register_metadata_key_relation (registry,
                                               GRL_METADATA_KEY_URL,
                                               GRL_METADATA_KEY_START_TIME);
  grl_registry_register_metadata_key_relation (registry,
                                               GRL_METADATA_KEY_REGION,
                                               GRL_METADATA_KEY_PUBLICATION_DATE);
  grl_registry_register_metadata_key_relation (registry,
                                               GRL_METADATA_KEY_REGION,
                                               GRL_METADATA_KEY_CERTIFICATE);
}

/**
 * grl_metadata_key_get_name:
 * @key: key to look up
 *
 * Retrieves the name associated with the key
 *
 * Returns: The name of the key
 *
 * Since: 0.1.6
 */
const gchar *
grl_metadata_key_get_name (GrlKeyID key)
{
  GrlRegistry *registry = grl_registry_get_default ();

  if (registry) {
    return grl_registry_lookup_metadata_key_name (registry, key);
  } else {
    return NULL;
  }
}

/**
 * grl_metadata_key_get_desc:
 * @key: key to look up
 *
 * Retrieves the description associated with the key
 *
 * Returns: the description of the key
 *
 * Since: 0.1.6
 */
const gchar *
grl_metadata_key_get_desc (GrlKeyID key)
{
  GrlRegistry *registry = grl_registry_get_default ();

  if (registry) {
    return grl_registry_lookup_metadata_key_desc (registry, key);
  } else {
    return NULL;
  }
}

/**
 * grl_metadata_key_get_type:
 * @key: key to look up
 *
 * Retrieves the expected type for values associated with this key
 *
 * Returns: the expected value type
 *
 * Since: 0.2.0
 **/
GType grl_metadata_key_get_type (GrlKeyID key)
{
  GrlRegistry *registry = grl_registry_get_default ();

  if (registry) {
    return grl_registry_lookup_metadata_key_type (registry, key);
  } else {
    return G_TYPE_INVALID;
  }
}

/**
 * grl_metadata_key_list_new: (skip)
 * @first_key: first key
 * @...: va_list keys
 *
 * Returns a #GList containing the va_list keys. Use #GRL_METADATA_KEY_INVALID
 * to finalize them.
 *
 * Returns: a #GList
 *
 * Since: 0.2.0
 **/
GList *
grl_metadata_key_list_new(GrlKeyID first_key, ...)
{
  GList *key_list = NULL;
  GrlKeyID next_key;
  va_list va_keys;

  va_start (va_keys, first_key);
  next_key = first_key;
  while (next_key) {
    key_list = g_list_prepend (key_list, GRLKEYID_TO_POINTER (next_key));
    next_key = va_arg (va_keys, GrlKeyID);
  }
  va_end (va_keys);

  return g_list_reverse (key_list);
}
