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

#include "grl-metadata-key.h"
#include "grl-metadata-key-priv.h"
#include "grl-definitions.h"

#include <stdarg.h>

GrlKeyID GRL_METADATA_KEY_ALBUM = NULL;
GrlKeyID GRL_METADATA_KEY_ARTIST = NULL;
GrlKeyID GRL_METADATA_KEY_AUTHOR = NULL;
GrlKeyID GRL_METADATA_KEY_DATE = NULL;
GrlKeyID GRL_METADATA_KEY_DESCRIPTION = NULL;
GrlKeyID GRL_METADATA_KEY_GENRE = NULL;
GrlKeyID GRL_METADATA_KEY_ID = NULL;
GrlKeyID GRL_METADATA_KEY_LAST_PLAYED = NULL;
GrlKeyID GRL_METADATA_KEY_LYRICS = NULL;
GrlKeyID GRL_METADATA_KEY_MIME = NULL;
GrlKeyID GRL_METADATA_KEY_SITE = NULL;
GrlKeyID GRL_METADATA_KEY_SOURCE = NULL;
GrlKeyID GRL_METADATA_KEY_THUMBNAIL = NULL;
GrlKeyID GRL_METADATA_KEY_THUMBNAIL_BINARY = NULL;
GrlKeyID GRL_METADATA_KEY_TITLE = NULL;

GrlKeyID GRL_METADATA_KEY_URL = NULL;
GrlKeyID GRL_METADATA_KEY_EXTERNAL_URL = NULL;
GrlKeyID GRL_METADATA_KEY_EXTERNAL_PLAYER = NULL;

GrlKeyID GRL_METADATA_KEY_BITRATE = NULL;
GrlKeyID GRL_METADATA_KEY_CHILDCOUNT = NULL;
GrlKeyID GRL_METADATA_KEY_DURATION = NULL;
GrlKeyID GRL_METADATA_KEY_HEIGHT = NULL;
GrlKeyID GRL_METADATA_KEY_LAST_POSITION = NULL;
GrlKeyID GRL_METADATA_KEY_PLAY_COUNT = NULL;
GrlKeyID GRL_METADATA_KEY_WIDTH = NULL;

GrlKeyID GRL_METADATA_KEY_FRAMERATE = NULL;
GrlKeyID GRL_METADATA_KEY_RATING = NULL;

GrlKeyID GRL_METADATA_KEY_STUDIO = NULL;
GrlKeyID GRL_METADATA_KEY_CERTIFICATE = NULL;
GrlKeyID GRL_METADATA_KEY_LICENSE = NULL;

void
grl_metadata_key_setup_system_keys (GrlPluginRegistry *registry)
{
  GRL_METADATA_KEY_ALBUM =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("album",
                                                                    "Album",
                                                                    "Album the media belongs to",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_ARTIST =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("artist",
                                                                    "Artist",
                                                                    "Main artist",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_AUTHOR =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("author",
                                                                    "Author",
                                                                    "Creator of the media",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_DATE =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("date",
                                                                    "Date",
                                                                    "Publishing or recording date",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_DESCRIPTION =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("description",
                                                                    "Description",
                                                                    "Description of the media",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_GENRE =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("genre",
                                                                    "Genre",
                                                                    "Genre of the media",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_ID =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("id",
                                                                    "ID",
                                                                    "Identifier of media",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_LAST_PLAYED =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("last-played-time",
                                                                    "LastPlayedTime",
                                                                    "Last time the media was played",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_LYRICS =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("lyrics",
                                                                    "Lyrics",
                                                                    "Song lyrics",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_MIME =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("mime-type",
                                                                    "MimeType",
                                                                    "Media mime type",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_SITE =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("site",
                                                                    "Site",
                                                                    "Site",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_SOURCE =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("source",
                                                                    "Source",
                                                                    "Source ID prioviding the content",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_THUMBNAIL =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("thumbnail",
                                                                    "Thumbnail",
                                                                    "Thumbnail image",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);

  GRL_METADATA_KEY_THUMBNAIL_BINARY =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_boxed ("thumbnail-binary",
                                                                   "Thumbnail Binary",
                                                                   "Thumbnail binary image",
                                                                   G_TYPE_BYTE_ARRAY,
                                                                   G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),

                                               NULL);
  GRL_METADATA_KEY_TITLE =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("title",
                                                                    "Title",
                                                                    "Title of the media",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_URL =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("url",
                                                                    "URL",
                                                                    "Media URL",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);

  GRL_METADATA_KEY_EXTERNAL_URL =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("external-url",
                                                                    "External URL",
                                                                    "External location where the media can be played back, usually a website",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);

  GRL_METADATA_KEY_EXTERNAL_PLAYER =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("external-player",
                                                                    "External Player URL",
                                                                    "URL of an external player that can be used to play the resource (usually a Flash player)",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);

GRL_METADATA_KEY_STUDIO =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("studio",
                                                                    "Studio",
                                                                    "Studio the media is from",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);

  GRL_METADATA_KEY_CERTIFICATE =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("certificate",
                                                                    "Certificate",
                                                                    "Age certificate of the media",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);

  GRL_METADATA_KEY_LICENSE =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_string ("license",
                                                                    "License",
                                                                    "The license of the media",
                                                                    NULL,
                                                                    G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);

  GRL_METADATA_KEY_BITRATE =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_int ("bitrate",
                                                                 "Bitrate",
                                                                 "Media bitrate in KBits/s",
                                                                 0, G_MAXINT,
                                                                 0,
                                                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_CHILDCOUNT =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_int ("childcount",
                                                                 "Childcount",
                                                                 "Number of items contained in a container",
                                                                 -1, G_MAXINT,
                                                                 GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN,
                                                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_DURATION =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_int ("duration",
                                                                 "Duration",
                                                                 "Media duration",
                                                                 0, G_MAXINT,
                                                                 0,
                                                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_HEIGHT =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_int ("height",
                                                                 "Height",
                                                                 "Height of media ('y' resolution)",
                                                                 0, G_MAXINT,
                                                                 0,
                                                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_LAST_POSITION =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_int ("playback-interrupted-time",
                                                                 "PlaybackInterruptedTime",
                                                                 "Time at which playback was interrupted",
                                                                 0, G_MAXINT,
                                                                 0,
                                                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_PLAY_COUNT =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_int ("play-count",
                                                                 "PlayCount",
                                                                 "How many times media was played",
                                                                 0, G_MAXINT,
                                                                 0,
                                                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_WIDTH =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_int ("width",
                                                                 "Width",
                                                                 "Width of media ('x' resolution)",
                                                                 0, G_MAXINT,
                                                                 0,
                                                                 G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);

  GRL_METADATA_KEY_FRAMERATE =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_float ("framerate",
                                                                   "Framerate",
                                                                   "Frames per second",
                                                                   0, G_MAXFLOAT,
                                                                   0,
                                                                   G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);
  GRL_METADATA_KEY_RATING =
    grl_plugin_registry_register_metadata_key (registry,
                                               g_param_spec_float ("rating",
                                                                   "Rating",
                                                                   "Media rating",
                                                                   0, G_MAXFLOAT,
                                                                   0,
                                                                   G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE),
                                               NULL);

  /* Create the relations */
  grl_plugin_registry_register_metadata_key_relation (registry,
                                                      GRL_METADATA_KEY_URL,
                                                      GRL_METADATA_KEY_MIME);
  grl_plugin_registry_register_metadata_key_relation (registry,
                                                      GRL_METADATA_KEY_URL,
                                                      GRL_METADATA_KEY_BITRATE);
  grl_plugin_registry_register_metadata_key_relation (registry,
                                                      GRL_METADATA_KEY_URL,
                                                      GRL_METADATA_KEY_FRAMERATE);
  grl_plugin_registry_register_metadata_key_relation (registry,
                                                      GRL_METADATA_KEY_URL,
                                                      GRL_METADATA_KEY_HEIGHT);
  grl_plugin_registry_register_metadata_key_relation (registry,
                                                      GRL_METADATA_KEY_URL,
                                                      GRL_METADATA_KEY_WIDTH);
}

/**
 * grl_metadata_key_get_name:
 * @key: (type GObject.ParamSpec): key to look up
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
  return GRL_METADATA_KEY_GET_NAME (key);
}

/**
 * grl_metadata_key_get_desc:
 * @key: (type GObject.ParamSpec): key to look up
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
  return GRL_METADATA_KEY_GET_DESC (key);
}
