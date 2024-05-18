/*
 * Copyright (C) 2010, 2011 Igalia S.L.
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
 * SECTION:grl-media
 * @short_description: A multimedia data transfer object
 * @see_also: #GrlData
 *
 * This high level class represents a multimedia item. It has methods to
 * set and get properties like author, title, description, and so on.
 */

#include "grl-media.h"
#include "grl-type-builtins.h"
#include <grilo.h>
#include <stdlib.h>
#include <string.h>

#define GRL_LOG_DOMAIN_DEFAULT  media_log_domain
GRL_LOG_DOMAIN(media_log_domain);

#define RATING_MAX  5.00
#define SERIAL_STRING_ALLOC 100

enum {
  PROP_0,
  PROP_MEDIA_TYPE
};

struct _GrlMediaPrivate {
  GrlMediaType media_type;
};

static void grl_media_finalize (GObject *object);

G_DEFINE_TYPE_WITH_PRIVATE (GrlMedia, grl_media, GRL_TYPE_DATA);

static void
grl_media_set_property (GObject *object,
                        guint prop_id,
                        const GValue *value,
                        GParamSpec *pspec)
{
  GrlMedia *media = GRL_MEDIA (object);

  switch (prop_id) {
  case PROP_MEDIA_TYPE:
    media->priv->media_type = g_value_get_enum (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (media, prop_id, pspec);
    break;
  }
}

static void
grl_media_get_property (GObject *object,
                        guint prop_id,
                        GValue *value,
                        GParamSpec *pspec)
{
  GrlMedia *media = GRL_MEDIA (object);

  switch (prop_id) {
  case PROP_MEDIA_TYPE:
    g_value_set_enum (value, media->priv->media_type);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (media, prop_id, pspec);
    break;
  }
}

static void
grl_media_class_init (GrlMediaClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->finalize = grl_media_finalize;
  gobject_class->set_property = grl_media_set_property;
  gobject_class->get_property = grl_media_get_property;

  /**
   * GrlMedia::media-type
   *
   * The type of the media.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_MEDIA_TYPE,
                                   g_param_spec_enum ("media-type",
                                                      "Media type",
                                                      "Type of media",
                                                      GRL_TYPE_MEDIA_TYPE,
                                                      GRL_MEDIA_TYPE_UNKNOWN,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT |
                                                      G_PARAM_STATIC_STRINGS));
}

static void
grl_media_init (GrlMedia *self)
{
  self->priv = grl_media_get_instance_private (self);
}

static void
grl_media_finalize (GObject *object)
{
  GRL_DEBUG ("grl_media_finalize (%s)",
             grl_data_get_string (GRL_DATA (object),
                                  GRL_METADATA_KEY_TITLE));
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_media_parent_class)->finalize (object);
}

/**
 * grl_media_new:
 *
 * Creates a new data media object.
 *
 * Returns: a newly-allocated data media.
 **/
GrlMedia *
grl_media_new (void)
{
  return g_object_new (GRL_TYPE_MEDIA,
                       "media-type", GRL_MEDIA_TYPE_UNKNOWN,
                       NULL);
}

/**
 * grl_media_audio_new:
 *
 * Creates a new media audio object.
 *
 * Returns: a newly-allocated media audio.
 *
 * Since: 0.1.4
 **/
GrlMedia *
grl_media_audio_new (void)
{
  return g_object_new (GRL_TYPE_MEDIA,
                       "media-type", GRL_MEDIA_TYPE_AUDIO,
                       NULL);
}

/**
 * grl_media_video_new:
 *
 * Creates a new media video object.
 *
 * Returns: a newly-allocated media video.
 *
 * Since: 0.1.4
 */
GrlMedia *
grl_media_video_new (void)
{
  return g_object_new (GRL_TYPE_MEDIA,
                       "media-type", GRL_MEDIA_TYPE_VIDEO,
                       NULL);
}

/**
 * grl_media_image_new:
 *
 * Creates a new media image object.
 *
 * Returns: a newly-allocated media image.
 *
 * Since: 0.1.4
 **/
GrlMedia *
grl_media_image_new (void)
{
  return g_object_new (GRL_TYPE_MEDIA,
                       "media-type", GRL_MEDIA_TYPE_IMAGE,
                       NULL);
}

/**
 * grl_media_container_new:
 *
 * Creates a new media container object.
 *
 * Returns: a newly-allocated media container.
 *
 * Since: 0.3.0
 **/
GrlMedia *
grl_media_container_new (void)
{
  return g_object_new (GRL_TYPE_MEDIA,
                       "media-type", GRL_MEDIA_TYPE_CONTAINER,
                       NULL);
}

/**
 * grl_media_is_audio:
 * @media: a media
 *
 * Check if @media is an audio
 *
 * Returns: %TRUE if @media is an audio
 *
 * Since: 0.3.0
 **/
gboolean
grl_media_is_audio (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), FALSE);

  return (media->priv->media_type == GRL_MEDIA_TYPE_AUDIO);
}

/**
 * grl_media_is_video:
 * @media: a media
 *
 * Check if @media is a video
 *
 * Returns: %TRUE if @media is a video
 *
 * Since: 0.3.0
 **/
gboolean
grl_media_is_video (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), FALSE);

  return (media->priv->media_type == GRL_MEDIA_TYPE_VIDEO);
}

/**
 * grl_media_is_image:
 * @media: a media
 *
 * Check if @media is an image
 *
 * Returns: %TRUE if @media is an image
 *
 * Since: 0.3.0
 **/
gboolean
grl_media_is_image (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), FALSE);

  return (media->priv->media_type == GRL_MEDIA_TYPE_IMAGE);
}

/**
 * grl_media_is_container:
 * @media: a media
 *
 * Check if @media is a container
 *
 * Returns: %TRUE if @media is a container
 *
 * Since: 0.3.0
 **/
gboolean
grl_media_is_container (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), FALSE);

  return (media->priv->media_type == GRL_MEDIA_TYPE_CONTAINER);
}

/**
 * grl_media_set_rating:
 * @media: a media
 * @rating: a rating value
 * @max: maximum rating value
 *
 * This method receives a rating and its scale and normalizes it on a scale
 * from 0...5 to match the usual five-star rating.
 *
 * Since: 0.1.5
 */
void
grl_media_set_rating (GrlMedia *media, gfloat rating, gfloat max)
{
  gfloat normalized_value;

  g_return_if_fail (GRL_IS_MEDIA (media));

  normalized_value = (rating * RATING_MAX) / max;
  grl_data_set_float (GRL_DATA (media),
		      GRL_METADATA_KEY_RATING,
		      normalized_value);
}

/**
 * grl_media_set_url_data:
 * @media: a #GrlMedia
 * @url: the media's URL
 * @mime: the @url mime type
 * @bitrate: the @url bitrate, or -1 to ignore
 * @framerate: media framerate, or -1 to ignore
 * @width: media width, or -1 to ignore
 * @height: media height, or -1 to ignore
 *
 * Sets all the keys related with the URL of a media resource in one go.
 *
 * Since: 0.3.0
 **/
void
grl_media_set_url_data (GrlMedia *media,
                        const gchar *url,
                        const gchar *mime,
                        gint bitrate,
                        gfloat framerate,
                        gint width,
                        gint height)
{
  GrlRelatedKeys *relkeys;

  g_return_if_fail (GRL_IS_MEDIA (media));

  relkeys = grl_related_keys_new ();
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_URL, url);
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_MIME, mime);
  if (bitrate >= 0) {
    grl_related_keys_set_int (relkeys, GRL_METADATA_KEY_BITRATE, bitrate);
  }
  if (framerate >= 0) {
    grl_related_keys_set_float (relkeys, GRL_METADATA_KEY_FRAMERATE, framerate);
  }
  if (width >= 0) {
    grl_related_keys_set_int (relkeys, GRL_METADATA_KEY_WIDTH, width);
  }
  if (height >= 0) {
    grl_related_keys_set_int (relkeys, GRL_METADATA_KEY_HEIGHT, height);
  }
  grl_data_set_related_keys (GRL_DATA (media), relkeys, 0);
}

/**
 * grl_media_add_url_data:
 * @media: a #GrlMedia
 * @url: a media's URL
 * @mime: th @url mime type
 * @bitrate: the @url bitrate, or -1 to ignore
 * @framerate: media framerate, or -1 to ignore
 * @width: media width, or -1 to ignore
 * @height: media height, or -1 to ignore
 *
 * Sets all the keys related with the URL of a media resource and adds it to
 * @media (useful for resources with more than one URL).
 *
 * Since: 0.3.0
 **/
void
grl_media_add_url_data (GrlMedia *media,
                        const gchar *url,
                        const gchar *mime,
                        gint bitrate,
                        gfloat framerate,
                        gint width,
                        gint height)
{
  GrlRelatedKeys *relkeys;

  g_return_if_fail (GRL_IS_MEDIA (media));

  relkeys = grl_related_keys_new ();
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_URL, url);
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_MIME, mime);
  if (bitrate >= 0) {
    grl_related_keys_set_int (relkeys, GRL_METADATA_KEY_BITRATE, bitrate);
  }
  if (framerate >= 0) {
    grl_related_keys_set_float (relkeys, GRL_METADATA_KEY_FRAMERATE, framerate);
  }
  if (width >= 0) {
    grl_related_keys_set_int (relkeys, GRL_METADATA_KEY_WIDTH, width);
  }
  if (height >= 0) {
    grl_related_keys_set_int (relkeys, GRL_METADATA_KEY_HEIGHT, height);
  }
  grl_data_add_related_keys (GRL_DATA (media), relkeys);
}

/**
 * grl_media_add_author:
 * @media: a #GrlMedia
 * @author: an author for @media
 *
 * Adds a new author to @media.
 *
 * Since: 0.1.10
 **/
void
grl_media_add_author (GrlMedia *media, const gchar *author)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_add_string (GRL_DATA (media), GRL_METADATA_KEY_AUTHOR, author);
}

/**
 * grl_media_add_thumbnail:
 * @media: a #GrlMedia
 * @thumbnail: a thumbnail for @media
 *
 * Adds a new thumbnail to @media.
 *
 * Since: 0.1.10
 **/
void
grl_media_add_thumbnail (GrlMedia *media, const gchar *thumbnail)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_add_string (GRL_DATA (media), GRL_METADATA_KEY_THUMBNAIL, thumbnail);
}

/**
 * grl_media_add_thumbnail_binary:
 * @media: a #GrlMedia
 * @thumbnail: a buffer containing the thumbnail for @media
 * @size: size of buffer
 *
 * Adds a new thumbnail to @media.
 *
 * Since: 0.1.10
 **/
void
grl_media_add_thumbnail_binary (GrlMedia *media,
                                const guint8 *thumbnail,
                                gsize size)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  g_return_if_fail (size == 0 || thumbnail != NULL);

  grl_data_add_binary (GRL_DATA (media),
                       GRL_METADATA_KEY_THUMBNAIL_BINARY,
                       thumbnail,
                       size);
}

/**
 * grl_media_add_external_player:
 * @media: a #GrlMedia
 * @player: an external player for @media
 *
 * Adds a new external player to @media.
 *
 * Since: 0.1.10
 **/
void
grl_media_add_external_player (GrlMedia *media, const gchar *player)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_add_string (GRL_DATA (media),
                       GRL_METADATA_KEY_EXTERNAL_PLAYER,
                       player);
}

/**
 * grl_media_add_external_url:
 * @media: a #GrlMedia
 * @url: an external url for @media
 *
 * Adds a new external url to @media.
 *
 * Since: 0.1.10
 **/
void
grl_media_add_external_url (GrlMedia *media, const gchar *url)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_add_string (GRL_DATA (media),
                       GRL_METADATA_KEY_EXTERNAL_URL,
                       url);
}

/**
 * grl_media_add_keyword:
 * @media: a #GrlMedia
 * @keyword: a keyword describing the media
 *
 * Adds the keyword describing the @media.
 *
 * Since: 0.2.3
 */
void
grl_media_add_keyword (GrlMedia *media,
                       const gchar *keyword)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_add_string (GRL_DATA (media),
                       GRL_METADATA_KEY_KEYWORD,
                       keyword);
}

/**
 * grl_media_add_artist:
 * @media: the media instance
 * @artist: an audio's artist
 *
 * Adds a new artist to @media.
 *
 * Since: 0.3.0
 **/
void
grl_media_add_artist (GrlMedia *media, const gchar *artist)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_add_string (GRL_DATA (media), GRL_METADATA_KEY_ARTIST, artist);
}

/**
 * grl_media_add_genre:
 * @media: the media instance
 * @genre: an audio's genre
 *
 * Adds a new genre to @media.
 *
 * Since: 0.3.0
 **/
void
grl_media_add_genre (GrlMedia *media, const gchar *genre)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_add_string (GRL_DATA (media), GRL_METADATA_KEY_GENRE, genre);
}

/**
 * grl_media_add_lyrics:
 * @media: the media instance
 * @lyrics: an audio's lyrics
 *
 * Adds a new lyrics to @media.
 *
 * Since: 0.3.0
 **/
void
grl_media_add_lyrics (GrlMedia *media, const gchar *lyrics)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_add_string (GRL_DATA (media), GRL_METADATA_KEY_LYRICS, lyrics);
}

/**
 * grl_media_add_mb_artist_id:
 * @media: the media instance
 * @mb_artist_id: a MusicBrainz artist identifier
 *
 * Adds a new MusicBrainz artist id to @media.
 *
 * Since: 0.3.0
 **/
void
grl_media_add_mb_artist_id (GrlMedia *media,
                            const gchar *mb_artist_id)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_add_string (GRL_DATA (media), GRL_METADATA_KEY_MB_ARTIST_ID,
                       mb_artist_id);
}

/**
 * grl_media_add_performer:
 * @media: a #GrlMedia
 * @performer: an actor performing in the movie
 *
 * Adds the actor performing in the movie.
 *
 * Since: 0.3.0
 */
void
grl_media_add_performer (GrlMedia *media,
                         const gchar *performer)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_add_string (GRL_DATA (media),
                       GRL_METADATA_KEY_PERFORMER,
                       performer);
}

/**
 * grl_media_add_producer:
 * @media: a #GrlMedia
 * @producer: producer of the movie
 *
 * Adds the producer of the media.
 *
 * Since: 0.3.0
 */
void
grl_media_add_producer (GrlMedia *media,
                        const gchar *producer)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_add_string (GRL_DATA (media),
                       GRL_METADATA_KEY_PRODUCER,
                       producer);
}

/**
 * grl_media_add_director:
 * @media: a #GrlMedia
 * @director: director of the movie
 *
 * Adds the director of the media
 *
 * Since: 0.3.0
 */
void
grl_media_add_director (GrlMedia *media,
                        const gchar *director)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_add_string (GRL_DATA (media),
                       GRL_METADATA_KEY_DIRECTOR,
                       director);
}

/**
 * grl_media_serialize:
 * @media: a #GrlMedia
 *
 * Serializes a GrlMedia into a string. It does a basic serialization.
 *
 * See grl_media_serialize_extended() to get more serialization approaches.
 *
 * Returns: serialized media
 *
 * Since: 0.1.6
 **/
gchar *
grl_media_serialize (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_media_serialize_extended (media, GRL_MEDIA_SERIALIZE_BASIC);
}

/**
 * grl_media_serialize_extended:
 * @media: a #GrlMedia
 * @serial_type: type of serialization
 * @...: media keys to serialize
 *
 * Serializes a GrlMedia into a string.
 *
 * See grl_media_unserialize() to recover back the GrlMedia from the string.
 *
 * If serialization type is @GRL_MEDIA_SERIALIZE_PARTIAL then it requires a
 * @GList with the properties to consider in serialization (id and source are
 * always considered).
 *
 * Returns: serialized media
 *
 * Since: 0.1.6
 **/
gchar *
grl_media_serialize_extended (GrlMedia *media,
                              GrlMediaSerializeType serial_type,
                              ...)
{
  GByteArray *binary_blob;
  GList *key;
  GList *keylist;
  GString *serial;
  GrlKeyID grlkey;
  GrlRegistry *registry;
  GrlRelatedKeys *relkeys;
  const GValue *value;
  const gchar *id;
  const gchar *source;
  gchar *base64_blob;
  gchar *iso8601_datetime;
  gchar *protocol;
  gchar *serial_media;
  gchar separator = '?';
  guint i;
  guint num_values;
  va_list va_serial;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  g_return_val_if_fail ((source = grl_media_get_source (media)), NULL);

  /* Check serialization type */
  switch (serial_type) {
  case GRL_MEDIA_SERIALIZE_FULL:
    registry = grl_registry_get_default ();
    keylist = grl_registry_get_metadata_keys (registry);
    serial_media = grl_media_serialize_extended (media,
                                                 GRL_MEDIA_SERIALIZE_PARTIAL,
                                                 keylist);
    g_list_free (keylist);
    break;
  case GRL_MEDIA_SERIALIZE_BASIC:
  case GRL_MEDIA_SERIALIZE_PARTIAL:
    switch (grl_media_get_media_type (media)) {
    case GRL_MEDIA_TYPE_AUDIO:
      protocol = "grlaudio://";
      break;
    case GRL_MEDIA_TYPE_VIDEO:
      protocol = "grlvideo://";
      break;
    case GRL_MEDIA_TYPE_IMAGE:
      protocol = "grlimage://";
      break;
    case GRL_MEDIA_TYPE_CONTAINER:
      protocol = "grlcontainer://";
      break;
    default:
      protocol = "grl";
    }

    /* Build serial string with escaped components */
    serial = g_string_sized_new (SERIAL_STRING_ALLOC);
    g_string_assign (serial, protocol);
    g_string_append_uri_escaped (serial, source, NULL, TRUE);
    id = grl_media_get_id (media);
    if (id) {
      g_string_append_c (serial, '/');
      g_string_append_uri_escaped (serial, id, NULL, TRUE);
    }

    /* Include all properties */
    if (serial_type == GRL_MEDIA_SERIALIZE_PARTIAL) {

      va_start (va_serial, serial_type);
      keylist = va_arg (va_serial, GList *);
      for (key = keylist; key; key = g_list_next (key)) {
        grlkey = GRLPOINTER_TO_KEYID (key->data);
        /* Skip id and source keys */
        if (grlkey == GRL_METADATA_KEY_ID ||
            grlkey == GRL_METADATA_KEY_SOURCE) {
          continue;
        }
        num_values = grl_data_length (GRL_DATA (media), grlkey);
        for (i = 0; i < num_values; i++) {

          g_string_append_c (serial, separator);
          if (separator == '?') {
            separator = '&';
          }

          g_string_append_printf (serial,
                                  "%s=",
                                  GRL_METADATA_KEY_GET_NAME (grlkey));

          relkeys = grl_data_get_related_keys (GRL_DATA (media), grlkey, i);
          if (!grl_related_keys_has_key (relkeys, grlkey)) {
            continue;
          }

          value = grl_related_keys_get (relkeys, grlkey);

          if (G_VALUE_HOLDS_STRING (value)) {
            g_string_append_uri_escaped (serial,
                                         g_value_get_string (value),
                                         NULL,
                                         TRUE);
          } else if (G_VALUE_HOLDS_INT (value)) {
            g_string_append_printf (serial, "%d", g_value_get_int (value));
          } else if (G_VALUE_HOLDS_FLOAT (value)) {
            g_string_append_printf (serial, "%f", g_value_get_float (value));
          } else if (G_VALUE_HOLDS_BOOLEAN (value)) {
            g_string_append_printf (serial, "%d", g_value_get_boolean (value));
          } else if (G_VALUE_TYPE (value) == G_TYPE_BYTE_ARRAY) {
            binary_blob = g_value_get_boxed (value);
            base64_blob = g_base64_encode (binary_blob->data, binary_blob->len);
            g_string_append_uri_escaped (serial,
                                         base64_blob,
                                         NULL,
                                         TRUE);
            g_free (base64_blob);
          } else if (G_VALUE_TYPE (value) == G_TYPE_DATE_TIME) {
            iso8601_datetime = g_date_time_format (g_value_get_boxed (value),
                                                   "%FT%T");
            g_string_append_uri_escaped (serial,
                                         iso8601_datetime,
                                         NULL,
                                         TRUE);
            g_free (iso8601_datetime);
          }
        }
      }

      va_end (va_serial);
    }
    serial_media = g_string_free (serial, FALSE);
    break;
  default:
    serial_media = NULL;
  }

  return serial_media;
}

static void
_insert_and_free_related_list (GrlKeyID key,
                               GList *relkeys_list,
                               GrlData *data)
{
  GList *p = relkeys_list;

  while (p) {
    grl_data_add_related_keys (data, (GrlRelatedKeys *) p->data);
    p = g_list_next (p);
  }

  g_list_free (relkeys_list);
}

/**
 * grl_media_unserialize:
 * @serial: a serialized media
 *
 * Unserializes a GrlMedia.
 *
 * Returns: (transfer full): the GrlMedia from the serial
 *
 * Since: 0.1.6
 **/
GrlMedia *
grl_media_unserialize (const gchar *serial)
{
  GDateTime *datetime;
  GHashTable *grlkey_related_table;
  GList *keys;
  GList *relkeys_list;
  GMatchInfo *match_info;
  GRegex *query_regex;
  GRegex *uri_regex;
  GType type_grlkey;
  GrlKeyID grlkey;
  GrlKeyID grlkey_index;
  GrlMedia *media;
  GrlRegistry *registry;
  GrlRelatedKeys *relkeys;
  gboolean append;
  gchar *escaped_value;
  gchar *keyname;
  gchar *protocol;
  gchar *query;
  gchar *value;
  gpointer p;
  gsize blob_size;
  guchar *blob;
  guint *grlkey_count;

  g_return_val_if_fail (serial, NULL);

  uri_regex =
    g_regex_new ("^(grl.*):\\/\\/([^\\///?]+)(\\/[^\\?]*)?(?:\\?(.*))?",
                 G_REGEX_CASELESS,
                 0,
                 NULL);
  if (!g_regex_match (uri_regex, serial, 0, &match_info)) {
    GRL_WARNING ("Wrong serial %s", serial);
    g_regex_unref (uri_regex);
    return NULL;
  }

  /* Build the media */
  protocol = g_match_info_fetch (match_info, 1);
  if (g_strcmp0 (protocol, "grlaudio") == 0) {
    media = grl_media_audio_new ();
  } else if (g_strcmp0 (protocol, "grlvideo") == 0) {
    media = grl_media_video_new ();
  } else if (g_strcmp0 (protocol, "grlimage") == 0) {
    media = grl_media_image_new ();
  } else if (g_strcmp0 (protocol, "grlcontainer") == 0) {
    media = grl_media_container_new ();
  } else if (g_strcmp0 (protocol, "grl") == 0) {
    media = grl_media_new ();
  } else {
    GRL_WARNING ("Unknown type %s", protocol);
    g_match_info_free (match_info);
    return NULL;
  }

  /* Add source */
  escaped_value = g_match_info_fetch (match_info, 2);
  value = g_uri_unescape_string (escaped_value, NULL);
  grl_media_set_source (media, value);
  g_free (escaped_value);
  g_free (value);

  /* Add id */
  escaped_value = g_match_info_fetch (match_info, 3);
  if (escaped_value && escaped_value[0] == '/') {
    guint len = strlen (escaped_value);
    if (len > 2 && escaped_value[len - 1] == '/')
      escaped_value[len - 1] = '\0';
    value = g_uri_unescape_string (escaped_value + 1, NULL);
    grl_media_set_id (media, value);
    g_free (value);
  }
  g_free (escaped_value);

  /* Check if there are more properties */
  query = g_match_info_fetch (match_info, 4);
  g_match_info_free (match_info);
  if (query) {
    registry = grl_registry_get_default ();
    keys = grl_registry_get_metadata_keys (registry);
    /* This is a hack: we do it because we know GrlKeyID are actually integers,
       and assigned sequentially (0 is for invalid key). This saves us to use a
       hashtable to store the counter per key */
    grlkey_count = g_new0 (guint, g_list_length(keys) + 1);
    g_list_free (keys);

    /* In this hashtable we enqueue all the GrlRelatedKeys, that will be added
       at the end; we can not add them directly because could be we have a
       GrlRelatedKeys with no values because one of the properties will come
       later; and we can not add empty values in data */
    grlkey_related_table = g_hash_table_new (g_direct_hash, g_direct_equal);

    query_regex = g_regex_new ("([^=&]+)=([^=&]*)", 0, 0, NULL);
    g_regex_match (query_regex, query, 0, &match_info);
    while (g_match_info_matches (match_info)) {
      keyname = g_match_info_fetch (match_info, 1);
      grlkey = grl_registry_lookup_metadata_key (registry, keyname);
      if (grlkey) {
        /* Search for the GrlRelatedKeys to insert the key, or create a new one */
        grlkey_index =
          GRLPOINTER_TO_KEYID (g_list_nth_data ((GList *) grl_registry_lookup_metadata_key_relation (registry, grlkey), 0));
        relkeys_list = g_hash_table_lookup (grlkey_related_table,
                                            GRLKEYID_TO_POINTER (grlkey_index));
        p = g_list_nth_data (relkeys_list, grlkey_count[grlkey]);
        if (p) {
          relkeys = (GrlRelatedKeys *) p;
          append = FALSE;
        } else {
          relkeys = grl_related_keys_new ();
          append = TRUE;
        }
        escaped_value = g_match_info_fetch (match_info, 2);
        if (escaped_value && escaped_value[0] != '\0') {
          value = g_uri_unescape_string (escaped_value, NULL);
          type_grlkey = GRL_METADATA_KEY_GET_TYPE (grlkey);
          if (type_grlkey == G_TYPE_STRING) {
            grl_related_keys_set_string (relkeys, grlkey, value);
          } else if (type_grlkey == G_TYPE_INT) {
            grl_related_keys_set_int (relkeys, grlkey, atoi (value));
          } else if (type_grlkey == G_TYPE_FLOAT) {
            grl_related_keys_set_float (relkeys, grlkey, atof (value));
          } else if (type_grlkey == G_TYPE_BOOLEAN) {
            grl_related_keys_set_boolean (relkeys, grlkey, atoi (value) == 0? FALSE: TRUE);
          } else if (type_grlkey == G_TYPE_BYTE_ARRAY) {
            blob = g_base64_decode (value, &blob_size);
            grl_related_keys_set_binary (relkeys, grlkey, blob, blob_size);
            g_free (blob);
          } else if (type_grlkey == G_TYPE_DATE_TIME) {
            datetime = grl_date_time_from_iso8601 (value);
            grl_related_keys_set_boxed (relkeys, grlkey, datetime);
            g_date_time_unref (datetime);
          }
          g_free (escaped_value);
          g_free (value);
        }
        if (append) {
          relkeys_list = g_list_append (relkeys_list, relkeys);
          g_hash_table_insert (grlkey_related_table,
                               GRLKEYID_TO_POINTER (grlkey_index),
                               relkeys_list);
        }
        grlkey_count[grlkey]++;
      }
      g_free (keyname);
      g_match_info_next (match_info, NULL);
    }
    /* Now we can add all the GrlRelatedKeys into media */
    g_hash_table_foreach (grlkey_related_table,
                          (GHFunc) _insert_and_free_related_list,
                          GRL_DATA (media));
    g_hash_table_unref (grlkey_related_table);
    g_match_info_free (match_info);
    g_free (query);
    g_free (grlkey_count);
  }

  return media;
}

/**
 * grl_media_set_id:
 * @media: the media
 * @id: the identifier of the media
 *
 * Set the media identifier
 *
 * Since: 0.1.4
 */
void
grl_media_set_id (GrlMedia *media, const gchar *id)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_ID,
                       id);
}

/**
 * grl_media_set_url:
 * @media: the media
 * @url: the media's URL
 *
 * Set the media's URL
 *
 * Since: 0.1.4
 */
void
grl_media_set_url (GrlMedia *media, const gchar *url)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_URL,
                       url);
}

/**
 * grl_media_set_author:
 * @media: the media
 * @author: the media's author
 *
 * Set the media's author
 *
 * Since: 0.1.4
 */
void
grl_media_set_author (GrlMedia *media, const gchar *author)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_AUTHOR,
                       author);
}

/**
 * grl_media_set_title:
 * @media: the media
 * @title: the title
 *
 * Set the media's title
 *
 * Since: 0.1.4
 */
void
grl_media_set_title (GrlMedia *media, const gchar *title)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_TITLE,
                       title);
}

/**
 * grl_media_set_description:
 * @media: the media
 * @description: the description
 *
 * Set the media's description
 *
 * Since: 0.1.4
 */
void
grl_media_set_description (GrlMedia *media, const gchar *description)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_DESCRIPTION,
                       description);
}

/**
 * grl_media_set_source:
 * @media: the media
 * @source: the source
 *
 * Set the media's source
 *
 * Since: 0.1.4
 */
void
grl_media_set_source (GrlMedia *media, const gchar *source)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_SOURCE,
                       source);
}

/**
 * grl_media_set_thumbnail:
 * @media: the media
 * @thumbnail: the thumbnail URL
 *
 * Set the media's thumbnail URL
 *
 * Since: 0.1.4
 */
void
grl_media_set_thumbnail (GrlMedia *media, const gchar *thumbnail)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_THUMBNAIL,
                       thumbnail);
}

/**
 * grl_media_set_thumbnail_binary:
 * @media: the media
 * @thumbnail: thumbnail buffer
 * @size: thumbnail buffer size
 *
 * Set the media's binary thumbnail
 *
 * Since: 0.1.9
 */
void
grl_media_set_thumbnail_binary (GrlMedia *media,
                                const guint8 *thumbnail,
                                gsize size)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  g_return_if_fail (size == 0 || thumbnail != NULL);

  grl_data_set_binary (GRL_DATA (media),
                       GRL_METADATA_KEY_THUMBNAIL_BINARY,
                       thumbnail,
                       size);
}

/**
 * grl_media_set_site:
 * @media: the media
 * @site: the site
 *
 * Set the media's site. A site is a website about the media such as a
 * studio's promotional website for a movie.
 *
 * Since: 0.1.4
 */
void
grl_media_set_site (GrlMedia *media, const gchar *site)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_SITE,
                       site);
}

/**
 * grl_media_set_duration:
 * @media: the media
 * @duration: the duration in seconds
 *
 * Set the media's duration
 *
 * Since: 0.1.4
 */
void
grl_media_set_duration (GrlMedia *media, gint duration)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_int (GRL_DATA (media),
                    GRL_METADATA_KEY_DURATION,
                    duration);
}

/**
 * grl_media_set_publication_date:
 * @media: the media
 * @date: the date
 *
 * Set the publication date of @media.
 *
 * Since: 0.2.0
 */
void
grl_media_set_publication_date (GrlMedia *media, const GDateTime *date)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_boxed (GRL_DATA (media),
                      GRL_METADATA_KEY_PUBLICATION_DATE,
                      date);
}

/**
 * grl_media_set_region:
 * @media: a #GrlMedia
 * @region: the region's ISO-3166-1 code
 *
 * Sets the @region where @media was published.
 *
 * Since: 0.2.3
 */
void
grl_media_set_region (GrlMedia *media,
                      const gchar *region)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_REGION, region);
}

/**
 * grl_media_set_region_data:
 * @media: a #GrlMedia
 * @region: the region's ISO-3166-1 code
 * @publication_date: the publication date
 * @certificate: the age certification
 *
 * Sets regional publication and certification information for @region.
 *
 * Since: 0.2.3
 */
void
grl_media_set_region_data (GrlMedia *media,
                           const gchar *region,
                           const GDateTime *publication_date,
                           const gchar *certificate)
{
  GrlRelatedKeys *relkeys;

  g_return_if_fail (GRL_IS_MEDIA (media));

  relkeys = grl_related_keys_new ();

  grl_related_keys_set_string (relkeys,
                               GRL_METADATA_KEY_REGION,
                               region);
  grl_related_keys_set_boxed (relkeys,
                              GRL_METADATA_KEY_PUBLICATION_DATE,
                              publication_date);
  grl_related_keys_set_string (relkeys,
                               GRL_METADATA_KEY_CERTIFICATE,
                               certificate);
  grl_data_set_related_keys (GRL_DATA (media), relkeys, 0);
}

/**
 * grl_media_add_region_data:
 * @media: a #GrlMedia
 * @region: the region's ISO-3166-1 code
 * @publication_date: the publication date
 * @certificate: the age certification
 *
 * Adds regional publication and certification information for @region.
 *
 * Since: 0.2.3
 */
void
grl_media_add_region_data (GrlMedia *media,
                           const gchar *region,
                           const GDateTime *publication_date,
                           const gchar *certificate)
{
  GrlRelatedKeys *relkeys;

  g_return_if_fail (GRL_IS_MEDIA (media));

  relkeys = grl_related_keys_new ();
  grl_related_keys_set_string (relkeys,
                               GRL_METADATA_KEY_REGION,
                               region);
  grl_related_keys_set_boxed (relkeys,
                              GRL_METADATA_KEY_PUBLICATION_DATE,
                              publication_date);
  grl_related_keys_set_string (relkeys,
                               GRL_METADATA_KEY_CERTIFICATE,
                               certificate);
  grl_data_add_related_keys (GRL_DATA (media), relkeys);
}

/**
  * grl_media_set_creation_date:
  * @media: the media
  * @creation_date: date when media was created
  *
  * Set the creation_date of the media
  *
  * Since: 0.2.0
  */
void
grl_media_set_creation_date (GrlMedia *media,
                             const GDateTime *creation_date)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_boxed (GRL_DATA (media),
                      GRL_METADATA_KEY_CREATION_DATE,
                      creation_date);
}

/**
  * grl_media_set_modification_date:
  * @media: the media
  * @modification_date: date when the media was last modified
  *
  * Set the modification date of the media
  *
  * Since: 0.2.0
  */
void
grl_media_set_modification_date (GrlMedia *media,
                                 const GDateTime *modification_date)

{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_boxed (GRL_DATA (media),
                      GRL_METADATA_KEY_MODIFICATION_DATE,
                      modification_date);
}

/**
 * grl_media_set_mime:
 * @media: the media
 * @mime: the mime type
 *
 * Set the media's mime-type
 *
 * Since: 0.1.4
 */
void
grl_media_set_mime (GrlMedia *media, const gchar *mime)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_MIME,
                       mime);
}

/**
 * grl_media_set_play_count:
 * @media: the media
 * @play_count: the play count
 *
 * Set the media play count
 *
 * Since: 0.1.4
 */
void
grl_media_set_play_count (GrlMedia *media, gint play_count)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_int (GRL_DATA (media),
                    GRL_METADATA_KEY_PLAY_COUNT,
                    play_count);
}

/**
 * grl_media_set_last_played:
 * @media: the media
 * @last_played: date when the media was last played
 *
 * Set the media last played date
 *
 * Since: 0.3.0
 */
void
grl_media_set_last_played (GrlMedia *media, const GDateTime *last_played)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_boxed (GRL_DATA (media),
                      GRL_METADATA_KEY_LAST_PLAYED,
                      last_played);
}

/**
 * grl_media_set_last_position:
 * @media: the media
 * @last_position: second at which the media playback was interrupted
 *
 * Set the media last played position
 *
 * Since: 0.1.4
 */
void
grl_media_set_last_position (GrlMedia *media, gint last_position)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_int (GRL_DATA (media),
                    GRL_METADATA_KEY_LAST_POSITION,
                    last_position);
}

/**
 * grl_media_set_external_player:
 * @media: the media
 * @player: location of an external player for this media
 *
 * Set the location of a player for the media (usually a flash player)
 *
 * Since: 0.1.6
 */
void
grl_media_set_external_player (GrlMedia *media, const gchar *player)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_EXTERNAL_PLAYER,
                       player);
}

/**
 * grl_media_set_external_url:
 * @media: the media
 * @url: external location where this media can be played.
 *
 * Set an external location where users can play the media
 *
 * Since: 0.1.6
 */
void
grl_media_set_external_url (GrlMedia *media, const gchar *url)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_EXTERNAL_URL,
                       url);
}

/**
 * grl_media_set_studio:
 * @media: the media
 * @studio: The studio the media is from
 *
 * Set the media studio
 *
 * Since: 0.1.6
 */
void
grl_media_set_studio (GrlMedia *media, const gchar *studio)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_STUDIO,
                       studio);
}

/**
 * grl_media_set_certificate:
 * @media: the media
 * @certificate: The age certificate of the media
 *
 * Set the media's first age certification.
 * This should usually be the media's most relevant
 * age certificate. Use grl_media_set_region_data() to
 * set other age certificates.
 *
 * Since: 0.1.6
 */
void
grl_media_set_certificate (GrlMedia *media, const gchar *certificate)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_CERTIFICATE,
                       certificate);
}

/**
 * grl_media_set_license:
 * @media: the media
 * @license: The license of the media
 *
 * Set the media license
 *
 * Since: 0.1.6
 */
void
grl_media_set_license (GrlMedia *media, const gchar *license)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_LICENSE,
                       license);
}

/**
 * grl_media_set_favourite:
 * @media: a media
 * @favourite: whether the item is favourite or not
 *
 * Set if the media is favourite or not
 *
 * Since: 0.2.3
 */
void
grl_media_set_favourite (GrlMedia *media, gboolean favourite)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_boolean (GRL_DATA (media),
                        GRL_METADATA_KEY_FAVOURITE,
                        favourite);
}

/**
 * grl_media_set_keyword:
 * @media: a #GrlMedia
 * @keyword: a keyword describing the media
 *
 * Sets the keyword describing the @media.
 *
 * Since: 0.2.3
 */
void
grl_media_set_keyword (GrlMedia *media,
                       const gchar *keyword)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_KEYWORD,
                       keyword);
}

/**
 * grl_media_set_size:
 * @media: the media
 * @size: the size in bytes
 *
 * Set the size of the media
 *
 * Since: 0.2.10
 */
void
grl_media_set_size (GrlMedia *media, gint64 size)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_int64 (GRL_DATA (media),
                      GRL_METADATA_KEY_SIZE,
                      size);
}

/**
 * grl_media_set_track_number:
 * @media: the media instance
 * @track_number: the audio's track number
 *
 * Set the track number of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_track_number (GrlMedia *media, gint track_number)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_int (GRL_DATA (media), GRL_METADATA_KEY_TRACK_NUMBER,
                    track_number);
}

/**
 * grl_media_set_bitrate:
 * @media: the media instance
 * @bitrate: the audio's bitrate
 *
 * Set the bitrate of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_bitrate (GrlMedia *media, gint bitrate)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_int (GRL_DATA (media), GRL_METADATA_KEY_BITRATE,
                    bitrate);
}

/**
 * grl_media_set_mb_track_id:
 * @media: the media instance
 * @mb_track_id: the MusicBrainz track identifier
 *
 * Set the MusicBrainz track identifier of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_mb_track_id (GrlMedia *media, const gchar *mb_track_id)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_MB_TRACK_ID,
                       mb_track_id);
}

/**
 * grl_media_set_mb_recording_id:
 * @media: the media instance
 * @mb_recording_id: the MusicBrainz recording identifier
 *
 * Set the MusicBrainz recording identifier of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_mb_recording_id (GrlMedia *media,
                               const gchar *mb_recording_id)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_MB_RECORDING_ID,
                       mb_recording_id);
}

/**
 * grl_media_set_mb_artist_id:
 * @media: the media instance
 * @mb_artist_id: the MusicBrainz artist identifier
 *
 * Set the MusicBrainz artist identifier of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_mb_artist_id (GrlMedia *media, const gchar *mb_artist_id)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_MB_ARTIST_ID,
                       mb_artist_id);
}

/**
 * grl_media_set_mb_album_id:
 * @media: the media instance
 * @mb_album_id: the MusicBrainz album identifier
 *
 * Set the MusicBrainz album identifier of the media
 *
 * Since: 0.3.0
 *
 * Deprecated: 0.3.8 in favor of more specific metadata-keys
 * GRL_METADATA_KEY_MB_RELEASE_ID and GRL_METADATA_KEY_MB_RELEASE_GROUP_ID
 */
void
grl_media_set_mb_album_id (GrlMedia *media, const gchar *mb_album_id)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_MB_ALBUM_ID,
                       mb_album_id);
}

/**
 * grl_media_set_mb_release_id:
 * @media: the media instance
 * @mb_release_id: Album release identifier in MusicBrainz
 *
 * Set the MusicBrainz release identifier of the media
 *
 * Since: 0.3.8
 */
void
grl_media_set_mb_release_id (GrlMedia *media, const gchar *mb_release_id)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_MB_RELEASE_ID,
                       mb_release_id);
}

/**
 * grl_media_set_mb_release_group_id:
 * @media: the media instance
 * @mb_release_group_id:  Album group release identifier in MusicBrainz
 *
 * Set the MusicBrainz Release Group identifier of the media
 *
 * Since: 0.3.8
 */
void
grl_media_set_mb_release_group_id (GrlMedia *media, const gchar *mb_release_group_id)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_MB_RELEASE_GROUP_ID,
                       mb_release_group_id);
}

/**
 * grl_media_set_lyrics:
 * @media: the media instance
 * @lyrics: the audio's lyrics
 *
 * Set the lyrics of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_lyrics (GrlMedia *media, const gchar *lyrics)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_LYRICS,
                       lyrics);
}

/**
 * grl_media_set_genre:
 * @media: the media instance
 * @genre: the audio's genre
 *
 * Set the genre of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_genre (GrlMedia *media, const gchar *genre)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_GENRE,
                       genre);
}

/**
 * grl_media_set_album:
 * @media: the media instance
 * @album: the audio's album
 *
 * Set the album of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_album (GrlMedia *media, const gchar *album)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_ALBUM,
                       album);
}

/**
 * grl_media_set_album_artist:
 * @media: the media instance
 * @album_artist: the audio's album main artist
 *
 * Set the main artist of the album of the media
 *
 * Since: 0.3.1
 */
void
grl_media_set_album_artist (GrlMedia *media, const gchar *album_artist)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_ALBUM_ARTIST,
                       album_artist);
}

/**
 * grl_media_set_album_disc_number:
 * @media: the media instance
 * @disc_number: the disc number within an album
 *
 * Set the disc number of the media for multi-disc album sets.
 *
 * Since: 0.3.1
 */
void
grl_media_set_album_disc_number (GrlMedia *media, gint disc_number)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_int (GRL_DATA (media),
                    GRL_METADATA_KEY_ALBUM_DISC_NUMBER,
                    disc_number);
}

/**
 * grl_media_set_artist:
 * @media: the media instance
 * @artist: the audio's artist
 *
 * Set the artist of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_artist (GrlMedia *media, const gchar *artist)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_ARTIST,
                       artist);
}

/**
 * grl_media_set_composer:
 * @media: the media instance
 * @composer: the audio's composer
 *
 * Set the composer of the media
 *
 * Since: 0.3.1
 */
void
grl_media_set_composer (GrlMedia *media, const gchar *composer)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media), GRL_METADATA_KEY_COMPOSER,
                       composer);
}

/**
 * grl_media_set_width:
 * @media: the media instance
 * @width: the video's width
 *
 * Set the width of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_width (GrlMedia *media, gint width)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_int (GRL_DATA (media),
                    GRL_METADATA_KEY_WIDTH,
                    width);
}

/**
 * grl_media_set_height:
 * @media: the media instance
 * @height: the video's height
 *
 * Set the height of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_height (GrlMedia *media, gint height)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_int (GRL_DATA (media),
                    GRL_METADATA_KEY_HEIGHT,
                    height);
}

/**
 * grl_media_set_framerate:
 * @media: the media instance
 * @framerate: the video's framerate
 *
 * Set the framerate of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_framerate (GrlMedia *media, gfloat framerate)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_float (GRL_DATA (media),
                      GRL_METADATA_KEY_FRAMERATE,
                      framerate);
}

/**
 * grl_media_set_season:
 * @media: the media instance
 * @season: the video's season
 *
 * Sets the season number of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_season (GrlMedia *media, gint season)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_int (GRL_DATA (media),
                    GRL_METADATA_KEY_SEASON,
                    season);
}

/**
 * grl_media_set_episode:
 * @media: the media instance
 * @episode: the video's episode
 *
 * Sets the episode number of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_episode (GrlMedia *media, gint episode)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_int (GRL_DATA (media),
                    GRL_METADATA_KEY_EPISODE,
                    episode);
}

/**
 * grl_media_set_episode_title:
 * @media: the media instance
 * @episode_title: the title of the episode
 *
 * Sets the title of an media
 *
 * Since: 0.3.0
 */
void
grl_media_set_episode_title (GrlMedia *media,
                             const gchar *episode_title)
{
  g_return_if_fail (GRL_IS_MEDIA (media));

  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_EPISODE_TITLE,
                       episode_title);
}

/**
 * grl_media_set_show:
 * @media: the media instance
 * @show: the video's show name
 *
 * Sets the show title of the media
 *
 * Since: 0.3.0
 */
void
grl_media_set_show (GrlMedia *media, const gchar *show)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_SHOW,
                       show);
}

/**
 * grl_media_set_performer:
 * @media: a #GrlMedia
 * @performer: an actor performing in the movie
 *
 * Sets the actor performing in the movie.
 *
 * Since: 0.3.0
 */
void
grl_media_set_performer (GrlMedia *media,
                         const gchar *performer)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_PERFORMER,
                       performer);
}

/**
 * grl_media_set_producer:
 * @media: a #GrlMedia
 * @producer: producer of the movie
 *
 * Sets the producer of the media.
 *
 * Since: 0.3.0
 */
void
grl_media_set_producer (GrlMedia *media, const gchar *producer)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_PRODUCER,
                       producer);
}

/**
 * grl_media_set_director:
 * @media: a #GrlMedia
 * @director: director of the movie
 *
 * Sets the director of the media.
 *
 * Since: 0.3.0
 */
void
grl_media_set_director (GrlMedia *media,
                        const gchar *director)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_DIRECTOR,
                       director);
}

/**
 * grl_media_set_original_title:
 * @media: a #GrlMedia
 * @original_title: original, untranslated title of the movie
 *
 * Sets the original, untranslated title of the media.
 *
 * Since: 0.3.0
 */
void
grl_media_set_original_title (GrlMedia *media,
                              const gchar *original_title)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_ORIGINAL_TITLE,
                       original_title);
}

/**
  * grl_media_set_camera_model:
  * @media: the media instance
  * @camera_model: model of camera used to take picture
  *
  * Set the camera_model of the media
  *
  * Since: 0.3.0
  */
void
grl_media_set_camera_model (GrlMedia *media,
                            const gchar *camera_model)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_CAMERA_MODEL,
                       camera_model);
}

/**
  * grl_media_set_flash_used:
  * @media: the media instance
  * @flash_used: whether the flash was used
  *
  * Set the flash_used of the media
  * See
  * https://gnome.pages.gitlab.gnome.org/tracker/nmm-ontology.html#nmm:Flash
  *
  * Since: 0.3.0
  */
void
grl_media_set_flash_used (GrlMedia *media,
                          const gchar *flash_used)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_FLASH_USED,
                       flash_used);
}

/**
  * grl_media_set_exposure_time:
  * @media: the media instance
  * @exposure_time: picture's exposure time
  *
  * Set the exposure_time of the media
  *
  * Since: 0.3.0
  */
void
grl_media_set_exposure_time (GrlMedia *media,
                             gfloat exposure_time)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_float (GRL_DATA (media),
                      GRL_METADATA_KEY_EXPOSURE_TIME,
                      exposure_time);
}

/**
  * grl_media_set_iso_speed:
  * @media: the media instance
  * @iso_speed: picture's iso speed
  *
  * Set the iso_speed of the media
  *
  * Since: 0.3.0
  */
void
grl_media_set_iso_speed (GrlMedia *media,
                         gfloat iso_speed)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_float (GRL_DATA (media),
                      GRL_METADATA_KEY_ISO_SPEED,
                      iso_speed);
}

/**
  * grl_media_set_orientation:
  * @media: the media instance
  * @orientation: degrees clockwise orientation of the picture
  *
  * Set the orientation of the media
  *
  * Since: 0.3.0
  */
void
grl_media_set_orientation (GrlMedia *media,
                           gint orientation)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  grl_data_set_int (GRL_DATA (media),
                    GRL_METADATA_KEY_ORIENTATION,
                    orientation % 360);
}

/**
 * grl_media_set_childcount:
 * @media: the media container instance
 * @childcount: number of children
 *
 * Sets the number of children of this container. Use
 * #GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN if it is unknown.
 *
 * Since: 0.3.0
 */
void
grl_media_set_childcount (GrlMedia *media,
                          gint childcount)
{
  g_return_if_fail (GRL_IS_MEDIA (media));
  g_return_if_fail (grl_media_is_container (media));

  if (childcount != GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN) {
    grl_data_set_int (GRL_DATA (media),
                      GRL_METADATA_KEY_CHILDCOUNT,
                      childcount);
  }
}

/**
 * grl_media_get_id:
 * @media: the media object
 *
 * Returns: the media's identifier
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_id (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_ID);
}

/**
 * grl_media_get_url:
 * @media: the media object
 *
 * Returns: the media's URL
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_url (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_URL);
}

/**
 * grl_media_get_url_data:
 * @media: the media object
 * @mime: (out) (transfer none): the mime-type, or %NULL to ignore.
 * @bitrate: (out): the url bitrate, or %NULL to ignore
 * @framerate: the url framerate, or %NULL to ignore
 * @width: the url width, or %NULL to ignore
 * @height: the url height, or %NULL to ignore
 *
 * Returns: the media's URL and its related properties.
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_url_data (GrlMedia *media,
                        gchar **mime,
                        gint *bitrate,
                        gfloat *framerate,
                        gint *width,
                        gint *height)

{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_media_get_url_data_nth (media,
                                     0,
                                     mime,
                                     bitrate,
                                     framerate,
                                     width,
                                     height);
}

/**
 * grl_media_get_url_data_nth:
 * @media: the media object
 * @index: element to retrieve
 * @mime: (out) (transfer none): the mime-type, or %NULL to ignore.
 * @bitrate: (out): the url bitrate, or %NULL to ignore
 * @framerate: the url framerate, or %NULL to ignore
 * @width: the url width, or %NULL to ignore
 * @height: the url height, or %NULL to ignore
 *
 * Returns: the n-th media's URL and its related properties.
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_url_data_nth (GrlMedia *media,
                            guint index,
                            gchar **mime,
                            gint *bitrate,
                            gfloat *framerate,
                            gint *width,
                            gint *height)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys = grl_data_get_related_keys (GRL_DATA (media), GRL_METADATA_KEY_URL, index);

  if (!relkeys) {
    return NULL;
  }

  if (mime) {
    *mime = (gchar *) grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_MIME);
  }

  if (bitrate) {
    *bitrate = grl_related_keys_get_int (relkeys, GRL_METADATA_KEY_BITRATE);
  }

  if (framerate) {
    *framerate = grl_related_keys_get_float (relkeys, GRL_METADATA_KEY_FRAMERATE);
  }

  if (width) {
    *width = grl_related_keys_get_int (relkeys, GRL_METADATA_KEY_WIDTH);
  }

  if (height) {
    *height = grl_related_keys_get_int (relkeys, GRL_METADATA_KEY_HEIGHT);
  }

  return grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_URL);
}

/**
 * grl_media_get_author:
 * @media: the media object
 *
 * Returns: the media's author
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_author (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_AUTHOR);
}

/**
 * grl_media_get_author_nth:
 * @media: the media object
 * @index: element to retrieve
 *
 * Returns: the n-th media's author.
 *
 * Since: 0.1.10
 */
const gchar *
grl_media_get_author_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_AUTHOR,
                               index);

  if (!relkeys) {
    return NULL;
  } else {
    return grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_AUTHOR);
  }
}

/**
 * grl_media_get_title:
 * @media: the media object
 *
 * Returns: the media's title
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_title (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_TITLE);
}

/**
 * grl_media_get_description:
 * @media: the media object
 *
 * Returns: the media's description
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_description (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_DESCRIPTION);
}

/**
 * grl_media_get_source:
 * @media: the media object source
 *
 * Returns: the media's source
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_source (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_SOURCE);
}

/**
 * grl_media_get_thumbnail:
 * @media: the media object
 *
 * Returns: the media's thumbnail URL
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_thumbnail (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_THUMBNAIL);
}

/**
 * grl_media_get_thumbnail_nth:
 * @media: the media object
 * @index: element to retrieve
 *
 * Returns: the n-th media's thumbnail.
 *
 * Since: 0.1.10
 */
const gchar *
grl_media_get_thumbnail_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_THUMBNAIL,
                               index);

  if (!relkeys) {
    return NULL;
  } else {
    return grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_THUMBNAIL);
  }
}

/**
 * grl_media_get_thumbnail_binary:
 * @media: the media object
 * @size: pointer to storing the thumbnail buffer size
 *
 * Returns: the media's thumbnail data and set size to the thumbnail buffer size
 *
 * Since: 0.1.9
 */
const guint8 *
grl_media_get_thumbnail_binary (GrlMedia *media, gsize *size)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  g_return_val_if_fail (size != NULL, NULL);

  return grl_data_get_binary (GRL_DATA (media),
                              GRL_METADATA_KEY_THUMBNAIL_BINARY,
                              size);
}

/**
 * grl_media_get_thumbnail_binary_nth:
 * @media: the media object
 * @size: pointer to store the thumbnail buffer size
 * @index: element to retrieve
 *
 * Returns: the n-th media's thumbnail binary and sets size to the thumbnail
 * buffer size.
 *
 * Since: 0.1.10
 */
const guint8 *
grl_media_get_thumbnail_binary_nth (GrlMedia *media, gsize *size, guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  g_return_val_if_fail (size != NULL, NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_THUMBNAIL,
                               index);

  if (!relkeys) {
    return NULL;
  } else {
    return grl_related_keys_get_binary (relkeys,
                                        GRL_METADATA_KEY_THUMBNAIL,
                                        size);
  }
}

/**
 * grl_media_get_site:
 * @media: the media object
 *
 * Returns: the media's site
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_site (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_SITE);
}

/**
 * grl_media_get_duration:
 * @media: the media object
 *
 * Returns: the media's duration in seconds
 *
 * Since: 0.1.4
 */
gint
grl_media_get_duration (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), 0);

  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_DURATION);
}

/**
 * grl_media_get_publication_date:
 * @media: the media object
 *
 * Returns: (transfer none): the publication date of @media (owned by @media).
 *
 * Since: 0.2.0
 */
GDateTime *
grl_media_get_publication_date (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_boxed (GRL_DATA (media),
                             GRL_METADATA_KEY_PUBLICATION_DATE);
}

/**
 * grl_media_get_region:
 * @media: the media object
 *
 * Returns: (transfer none): the ISO-3166-1 of the region where the media was
 * published (owned by @media).
 *
 * Since: 0.2.3
 */
const gchar *
grl_media_get_region (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_REGION);
}

/**
 * grl_media_get_region_data:
 * @media: the media object
 * @publication_date: (out) (transfer none): the publication date, or %NULL to ignore.
 * @certificate: (out) (transfer none): the age certification, or %NULL to ignore.
 *
 * Returns the media's age certificate and publication date for the first region.
 * This should usually be the media's most relevant region.
 * Use grl_media_get_region_data_nth() to get the age certificate and
 * publication date for other regions.
 *
 * Returns: (transfer none): the ISO-3166-1 of the region where the media was
 * published (owned by @media).
 *
 * Since: 0.2.3
 */
const gchar *
grl_media_get_region_data (GrlMedia *media,
                           const GDateTime **publication_date,
                           const gchar **certificate)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_media_get_region_data_nth (media, 0, publication_date,  certificate);
}

/**
 * grl_media_get_region_data_nth:
 * @media: the media object
 * @index: element to retrieve
 * @publication_date: (out) (transfer none): the publication date, or %NULL to ignore.
 * @certificate: (out) (transfer none): the age certification, or %NULL to ignore.
 *
 * Returns the media's age certificate and publication date for one region.
 * Use grl_data_length() with GRL_METADATA_KEY_REGION to discover
 * how many regions are available. For instance:
 * <informalexample>
 * <programlisting role="C"><![CDATA[
 * guint count = grl_data_length (GRL_DATA (media), GRL_METADATA_KEY_REGION);
 * guint i;
 * for (i = 0; i < count; ++i) {
 *   const GDateTime* publication_date = NULL;
 *   const gchar* certificate = NULL;
 *   const gchar* region =
 *     grl_media_get_region_data_nth (media, i,
 *       &publication_date, &certificate);
 *   ...
 * }
 * ]]></programlisting>
 * </informalexample>
 *
 * Returns: (transfer none): the ISO-3166-1 of the region where the media was
 * published (owned by @media).
 *
 * Since: 0.2.3
 */
const gchar *
grl_media_get_region_data_nth (GrlMedia *media,
                               guint index,
                               const GDateTime **publication_date,
                               const gchar **certificate)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_PUBLICATION_DATE,
                               index);

  if (!relkeys) {
    return NULL;
  }

  if (publication_date) {
    *publication_date = grl_related_keys_get_boxed
              (relkeys, GRL_METADATA_KEY_PUBLICATION_DATE);
  }

  if (certificate) {
    *certificate = grl_related_keys_get_string
              (relkeys, GRL_METADATA_KEY_CERTIFICATE);
  }

  return grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_REGION);
}

/**
 * grl_media_get_creation_date:
 * @media: the media
 *
 * Returns: (transfer none): date when media was created (owned by @media).
 *
 * Since: 0.2.0
 */
GDateTime *
grl_media_get_creation_date (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_boxed (GRL_DATA (media), GRL_METADATA_KEY_CREATION_DATE);
}

/**
 * grl_media_get_modification_date:
 * @media: the media
 *
 * Returns: (transfer none):date when the media was last modified (owned by @media).
 *
 * Since: 0.2.0
 */
GDateTime *
grl_media_get_modification_date (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_boxed (GRL_DATA (media),
                             GRL_METADATA_KEY_MODIFICATION_DATE);
}

/**
 * grl_media_get_mime:
 * @media: the media object
 *
 * Returns: the media's mime-type
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_mime (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_MIME);
}

/**
 * grl_media_get_rating:
 * @media: the media object
 *
 * Returns: the media's rating
 *
 * Since: 0.1.5
 */
gfloat
grl_media_get_rating (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), 0.0);

  return grl_data_get_float (GRL_DATA (media), GRL_METADATA_KEY_RATING);
}

/**
 * grl_media_get_play_count:
 * @media: the media object
 *
 * Returns: the media's play count
 *
 * Since: 0.1.4
 */
gint
grl_media_get_play_count (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), 0);

  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_PLAY_COUNT);
}

/**
 * grl_media_get_last_position:
 * @media: the media object
 *
 * Returns: the media's last_played position (in seconds)
 *
 * Since: 0.1.4
 */
gint
grl_media_get_last_position (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), 0);

  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_LAST_POSITION);
}

/**
 * grl_media_get_last_played:
 * @media: the media object
 *
 * Returns: (transfer none): the media's last played time
 *
 * Since: 0.3.0
 */
GDateTime *
grl_media_get_last_played (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_boxed (GRL_DATA (media), GRL_METADATA_KEY_LAST_PLAYED);
}

/**
 * grl_media_get_player:
 * @media: the media object
 *
 * Returns: URL of an external player
 * object for this media
 *
 * Since: 0.1.6
 */
const gchar *
grl_media_get_player(GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media),
                              GRL_METADATA_KEY_EXTERNAL_PLAYER);
}

/**
 * grl_media_get_player_nth:
 * @media: the media object
 * @index: element to retrieve
 *
 * Returns: the n-th media's external player object.
 *
 * Since: 0.1.10
 */
const gchar *
grl_media_get_player_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_EXTERNAL_PLAYER,
                               index);

  if (!relkeys) {
    return NULL;
  } else {
    return grl_related_keys_get_string (relkeys,
                                        GRL_METADATA_KEY_EXTERNAL_PLAYER);
  }
}

/**
 * grl_media_get_external_url:
 * @media: the media object
 *
 * Returns: URL of an external location
 * where the user play the media.
 *
 * Since: 0.1.6
 */
const gchar *
grl_media_get_external_url (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_EXTERNAL_URL);
}

/**
 * grl_media_get_external_url_nth:
 * @media: the media object
 * @index: element to retrieve
 *
 * Returns: the n-th media's external location where the user can play it.
 *
 * Since: 0.1.10
 */
const gchar *
grl_media_get_external_url_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_EXTERNAL_URL,
                               index);

  if (!relkeys) {
    return NULL;
  } else {
    return grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_EXTERNAL_URL);
  }
}

/**
 * grl_media_get_studio:
 * @media: the media object
 *
 * Returns: the studio the media is from
 *
 * Since: 0.1.6
 */
const gchar *
grl_media_get_studio(GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_STUDIO);
}

/**
 * grl_media_get_certificate:
 * @media: the media object
 *
 * Returns the media's first age certificate.
 * This should usually be the media's most relevant
 * age certificate. Use grl_media_get_region_data_nth() to
 * get other age certificates.
 *
 * Returns: the media's age certification
 *
 * Since: 0.1.6
 */
const gchar *
grl_media_get_certificate (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_CERTIFICATE);
}

/**
 * grl_media_get_license:
 * @media: the media object
 *
 * Returns: the license the media is under
 *
 * Since: 0.1.6
 */
const gchar *
grl_media_get_license (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_LICENSE);
}

/**
 * grl_media_get_start_time:
 * @media: the media object
 *
 * Returns: the start time of the logical media resource inside the
 *          file containing it, in seconds.
 *
 * Since: 0.1.19
 */
gfloat
grl_media_get_start_time (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), 0.0);

  return grl_data_get_float (GRL_DATA (media), GRL_METADATA_KEY_START_TIME);
}

/**
 * grl_media_get_favourite:
 * @media: the media object
 *
 * Returns: whether the media is favourite or not
 *
 * Since: 0.2.3
 */
gboolean
grl_media_get_favourite (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), FALSE);

  return grl_data_get_boolean (GRL_DATA (media), GRL_METADATA_KEY_FAVOURITE);
}

/**
 * grl_media_get_keyword:
 * @media: a #GrlMedia
 *
 * Returns: (transfer none): the keyword describing the @media (owned by @media).
 *
 * Since: 0.2.3
 */
const gchar *
grl_media_get_keyword (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media),
                              GRL_METADATA_KEY_KEYWORD);
}

/**
 * grl_media_get_keyword_nth:
 * @media: a #GrlMedia
 * @index: element to retrieve
 *
 * Returns: (transfer none): the keyword describing the @media (owned by @media).
 *
 * Since: 0.2.3
 */
const gchar *
grl_media_get_keyword_nth (GrlMedia *media,
                           guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_KEYWORD,
                               index);

  if (!relkeys) {
    return NULL;
  }

  return grl_related_keys_get_string (relkeys,
                                      GRL_METADATA_KEY_KEYWORD);
}

/**
 * grl_media_get_size:
 * @media: the media object
 *
 * Returns: the media's size, in bytes or -1 if unknown.
 *
 * since: 0.2.10
 */
gint64
grl_media_get_size (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), -1);
  return grl_data_get_int64 (GRL_DATA (media), GRL_METADATA_KEY_SIZE);
}

/**
 * grl_media_get_track_number:
 * @media: the media instance
 *
 * Returns: the track number of the media
 *
 * Since: 0.3.0
 */
gint
grl_media_get_track_number (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), 0);
  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_TRACK_NUMBER);
}

/**
 * grl_media_get_bitrate:
 * @media: the media instance
 *
 * Returns: the bitrate of the media
 *
 * Since: 0.3.0
 */
gint
grl_media_get_bitrate (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), 0);
  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_BITRATE);
}

/**
 * grl_media_get_mb_album_id:
 * @media: the media instance
 *
 * Returns: the MusicBrainz album identifier
 *
 * Since: 0.3.0
 *
 * Deprecated: 0.3.8 in favor of more specific metadata-keys
 * GRL_METADATA_KEY_MB_RELEASE_ID and GRL_METADATA_KEY_MB_RELEASE_GROUP_ID
 */
const gchar *
grl_media_get_mb_album_id (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_MB_ALBUM_ID);
}

/**
 * grl_media_get_mb_artist_id:
 * @media: the media instance
 *
 * Returns: the MusicBrainz artist identifier
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_mb_artist_id (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_MB_ARTIST_ID);
}

/**
 * grl_media_get_mb_artist_id_nth:
 * @media: the media instance
 * @index: element to retrieve, starting at 0
 *
 * Returns: the n-th MusicBrainz artist identifier of the media
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_mb_artist_id_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_MB_ARTIST_ID,
                               index);

  if (!relkeys) {
    return NULL;
  } else {
    return grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_MB_ARTIST_ID);
  }
}

/**
 * grl_media_get_mb_recording_id:
 * @media: the media instance
 *
 * Returns: the MusicBrainz recording identifier
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_mb_recording_id (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_MB_RECORDING_ID);
}

/**
 * grl_media_get_mb_track_id:
 * @media: the media instance
 *
 * Returns: the MusicBrainz track identifier
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_mb_track_id (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_MB_TRACK_ID);
}

/**
 * grl_media_get_mb_release_id:
 * @media: the media instance
 *
 * Returns: the MusicBrainz release identifier of the media
 *
 * Since: 0.3.8
 */
const gchar *
grl_media_get_mb_release_id (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_MB_RELEASE_ID);
}

/**
 * grl_media_get_mb_release_group_id:
 * @media: the media instance
 *
 * Returns: the MusicBrainz release group identifier of the media
 *
 * Since: 0.3.8
 */
const gchar *
grl_media_get_mb_release_group_id (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_MB_RELEASE_GROUP_ID);
}

/**
 * grl_media_get_lyrics:
 * @media: the media instance
 *
 * Returns: the lyrics of the media
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_lyrics (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_LYRICS);
}

/**
 * grl_media_get_lyrics_nth:
 * @media: the media instance
 * @index: element to retrieve, starting at 0
 *
 * Returns: the n-th lyrics of the media
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_lyrics_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_LYRICS,
                               index);

  if (!relkeys) {
    return NULL;
  } else {
    return grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_LYRICS);
  }
}

/**
 * grl_media_get_genre:
 * @media: the media instance
 *
 * Returns: the genre of the media
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_genre (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_GENRE);
}

/**
 * grl_media_get_genre_nth:
 * @media: the media instance
 * @index: element to retrieve, starting at 0
 *
 * Returns: the n-th genre of the media
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_genre_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media), GRL_METADATA_KEY_GENRE, index);

  if (!relkeys) {
    return NULL;
  } else {
    return grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_GENRE);
  }
}

/**
 * grl_media_get_album:
 * @media: the media instance
 *
 * Returns: the album of the media
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_album (GrlMedia *media)
{
  g_return_val_if_fail (GRL_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_ALBUM);
}

/**
 * grl_media_get_album_artist:
 * @media: the media instance
 *
 * Returns: the main artist of the album of the media
 *
 * Since: 0.3.1
 */
const gchar *
grl_media_get_album_artist (GrlMedia *media)
{
  g_return_val_if_fail (GRL_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_ALBUM_ARTIST);
}

/**
 * grl_media_get_album_disc_number:
 * @media: the media instance
 *
 * Returns: the disc number of the media for multi-disc album sets.
 *
 * Since: 0.3.1
 */
gint
grl_media_get_album_disc_number (GrlMedia *media)
{
  g_return_val_if_fail (GRL_MEDIA (media), 0);
  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_ALBUM_DISC_NUMBER);
}

/**
 * grl_media_get_artist:
 * @media: the media instance
 *
 * Returns: the artist of the media
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_artist (GrlMedia *media)
{
  g_return_val_if_fail (GRL_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_ARTIST);
}

/**
 * grl_media_get_artist_nth:
 * @media: the media instance
 * @index: element to retrieve, starting at 0
 *
 * Returns: the n-th artist of the media
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_artist_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_ARTIST,
                               index);

  if (!relkeys) {
    return NULL;
  } else {
    return grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_ARTIST);
  }
}

/**
 * grl_media_get_composer:
 * @media: the media instance
 *
 * Returns: the composer of the media
 *
 * Since: 0.3.1
 */
const gchar *
grl_media_get_composer (GrlMedia *media)
{
  g_return_val_if_fail (GRL_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_COMPOSER);
}

/**
 * grl_media_get_composer_nth:
 * @media: the media instance
 * @index: element to retrieve, starting at 0
 *
 * Returns: the n-th composer of the media
 *
 * Since: 0.3.1
 */
const gchar *
grl_media_get_composer_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_COMPOSER,
                               index);

  if (!relkeys) {
    return NULL;
  } else {
    return grl_related_keys_get_string (relkeys, GRL_METADATA_KEY_COMPOSER);
  }
}

/**
 * grl_media_get_media_type:
 * @media: the media instance
 *
 * Gets the "media-type" property.
 *
 * Returns: media type
 *
 * Since: 0.3.0
 */
GrlMediaType
grl_media_get_media_type (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), GRL_MEDIA_TYPE_UNKNOWN);

  return media->priv->media_type;
}

/**
 * grl_media_get_width:
 * @media: the media instance
 *
 * Returns: the width of the media
 *
 * Since: 0.3.0
 */
gint
grl_media_get_width (GrlMedia *media)
{
  g_return_val_if_fail (GRL_MEDIA (media), 0);
  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_WIDTH);
}

/**
 * grl_media_get_height:
 * @media: the media instance
 *
 * Returns: the height of the media
 *
 * Since: 0.3.0
 */
gint
grl_media_get_height (GrlMedia *media)
{
  g_return_val_if_fail (GRL_MEDIA (media), 0);
  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_HEIGHT);
}

/**
 * grl_media_get_framerate:
 * @media: the media instance
 *
 * Returns: the framerate of the media
 *
 * Since: 0.3.0
 */
gfloat
grl_media_get_framerate (GrlMedia *media)
{
  g_return_val_if_fail (GRL_MEDIA (media), 0);
  return grl_data_get_float (GRL_DATA (media), GRL_METADATA_KEY_FRAMERATE);
}

/**
 * grl_media_get_season:
 * @media: the media instance
 *
 * Returns: the season number of the media
 *
 * Since: 0.3.0
 */
gint
grl_media_get_season (GrlMedia *media)
{
  g_return_val_if_fail (GRL_MEDIA (media), 0);
  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_SEASON);
}

/**
 * grl_media_get_episode:
 * @media: the media instance
 *
 * Returns: the episode number of the media
 *
 * Since: 0.3.0
 */
gint
grl_media_get_episode (GrlMedia *media)
{
  g_return_val_if_fail (GRL_MEDIA (media), 0);
  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_EPISODE);
}

/**
 * grl_media_get_episode_title:
 * @media: the media instance
 *
 * Returns: the title of the episode
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_episode_title (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_EPISODE_TITLE);
}

/**
 * grl_media_get_show:
 * @media: the media instance
 *
 * Returns: the show title of the media
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_show (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_SHOW);
}

/**
 * grl_media_get_performer:
 * @media: a #GrlMedia
 *
 * Returns: (transfer none): the actor performing in the movie (owned by @media).
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_performer (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media),
                              GRL_METADATA_KEY_PERFORMER);
}

/**
 * grl_media_get_performer_nth:
 * @media: a #GrlMedia
 * @index: element to retrieve
 *
 * Returns: (transfer none): the actor performing in the movie (owned by @medi).
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_performer_nth (GrlMedia *media,
                             guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_PERFORMER,
                               index);

  if (!relkeys) {
    return NULL;
  }

  return grl_related_keys_get_string (relkeys,
                                      GRL_METADATA_KEY_PERFORMER);
}

/**
 * grl_media_get_producer:
 * @media: a #GrlMedia
 *
 * Returns: (transfer none): the producer of the movie (owned by @media).
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_producer (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media),
                              GRL_METADATA_KEY_PRODUCER);
}

/**
 * grl_media_get_producer_nth:
 * @media: a #GrlMedia
 * @index: element to retrieve
 *
 * Returns: (transfer none): the producer of the movie (owned by @media).
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_producer_nth (GrlMedia *media,
                            guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_PRODUCER,
                               index);

  if (!relkeys) {
    return NULL;
  }

  return grl_related_keys_get_string (relkeys,
                                      GRL_METADATA_KEY_PRODUCER);
}

/**
 * grl_media_get_director:
 * @media: a #GrlMedia
 *
 * Returns: (transfer none): the director of the movie (owned by @media).
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_director (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media),
                              GRL_METADATA_KEY_DIRECTOR);
}

/**
 * grl_media_get_director_nth:
 * @media: a #GrlMedia
 * @index: element to retrieve
 *
 * Returns: (transfer none): the director of the movie (owned by @media).
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_director_nth (GrlMedia *media,
                            guint index)
{
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);

  relkeys =
    grl_data_get_related_keys (GRL_DATA (media),
                               GRL_METADATA_KEY_DIRECTOR,
                               index);

  if (!relkeys) {
    return NULL;
  }

  return grl_related_keys_get_string (relkeys,
                                      GRL_METADATA_KEY_DIRECTOR);
}

/**
 * grl_media_get_original_title:
 * @media: a #GrlMedia
 *
 * Returns: (transfer none): the original, untranslated title of the movie (owned by @media).
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_original_title (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media),
                              GRL_METADATA_KEY_ORIGINAL_TITLE);
}

/**
 * grl_media_get_camera_model:
 * @media: the media instance
 *
 * Returns: model of camera used to take picture
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_camera_model (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media),
                              GRL_METADATA_KEY_CAMERA_MODEL);
}

/**
 * grl_media_get_flash_used:
 * @media: the media instance
 *
 * Returns: whether the flash was used.
 *
 * See
 * https://gnome.pages.gitlab.gnome.org/tracker/nmm-ontology.html#nmm:Flash
 *
 * Since: 0.3.0
 */
const gchar *
grl_media_get_flash_used (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  return grl_data_get_string (GRL_DATA (media),
                              GRL_METADATA_KEY_FLASH_USED);
}

/**
 * grl_media_get_exposure_time:
 * @media: the media instance
 *
 * Returns: picture's exposure time
 *
 * Since: 0.3.0
 */
gfloat
grl_media_get_exposure_time (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), 0.0);
  return grl_data_get_float (GRL_DATA (media),
                             GRL_METADATA_KEY_EXPOSURE_TIME);
}

/**
 * grl_media_get_iso_speed:
 * @media: the media instance
 *
 * Returns: picture's iso speed
 *
 * Since: 0.3.0
 */
gfloat
grl_media_get_iso_speed (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), 0.0);
  return grl_data_get_float (GRL_DATA (media),
                             GRL_METADATA_KEY_ISO_SPEED);
}

/**
 * grl_media_get_orientation:
 * @media: the image instance
 *
 * Returns: degrees clockwise orientation of the picture
 *
 * Since: 0.3.0
 */
gint
grl_media_get_orientation (GrlMedia *media)
{
  g_return_val_if_fail (GRL_IS_MEDIA (media), 0.0);
  return grl_data_get_int (GRL_DATA (media),
                           GRL_METADATA_KEY_ORIENTATION);
}

/**
 * grl_media_get_childcount:
 * @media: the media container instance
 *
 * Number of children of this container.
 *
 * Returns: number of children, or #GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN if
 * unknown.
 *
 * Since: 0.3.0
 */
gint
grl_media_get_childcount (GrlMedia *media)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_MEDIA (media), GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN);
  g_return_val_if_fail (grl_media_is_container (media), GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN);

  value = grl_data_get (GRL_DATA (media),
                        GRL_METADATA_KEY_CHILDCOUNT);

  if (value) {
    return g_value_get_int (value);
  } else {
    return GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN;
  }
}
