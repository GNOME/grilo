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
 * @see_also: #GrlData, #GrlMediaBox, #GrlMediaVideo, #GrlMediaAudio,
 * #GrlMediaImage
 *
 * This high level class represents a multimedia item. It has methods to
 * set and get properties like author, title, description, and so on.
 */

#include "grl-media.h"
#include <grilo.h>
#include <stdlib.h>

#define GRL_LOG_DOMAIN_DEFAULT  media_log_domain
GRL_LOG_DOMAIN(media_log_domain);

#define RATING_MAX  5.00
#define SERIAL_STRING_ALLOC 100

static void grl_media_dispose (GObject *object);
static void grl_media_finalize (GObject *object);

G_DEFINE_TYPE (GrlMedia, grl_media, GRL_TYPE_DATA);

static void
grl_media_class_init (GrlMediaClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_media_dispose;
  gobject_class->finalize = grl_media_finalize;
}

static void
grl_media_init (GrlMedia *self)
{
}

static void
grl_media_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_media_parent_class)->dispose (object);
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
 *
 * Since: 0.1.4
 **/
GrlMedia *
grl_media_new (void)
{
  return g_object_new (GRL_TYPE_MEDIA,
		       NULL);
}

/**
 * grl_media_set_rating:
 * @media: a media
 * @rating: a rating value
 * @max: maximum rating value
 *
 * This method receives a rating and its scale and normalizes it
 *
 * Since: 0.1.5
 */
void
grl_media_set_rating (GrlMedia *media, gfloat rating, gfloat max)
{
  gfloat normalized_value = (rating * RATING_MAX) / max;
  grl_data_set_float (GRL_DATA (media),
		      GRL_METADATA_KEY_RATING,
		      normalized_value);
}

/**
 * grl_media_set_url_data:
 * @media: a #GrlMedia
 * @url: the media's URL
 * @mime: the @url mime type
 *
 * Set the media's URL and its mime-type.
 **/
void
grl_media_set_url_data (GrlMedia *media, const gchar *url, const gchar *mime)
{
  GrlRelatedKeys *relkeys = grl_related_keys_new ();
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_URL, url);
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_MIME, mime);
  grl_data_set_related_keys (GRL_DATA (media), relkeys, 0);
}

/**
 * grl_media_add_url_data:
 * @media: a #GrlMedia
 * @url: a media's URL
 * @mime: th @url mime type
 *
 * Adds a new media's URL with its mime-type.
 **/
void
grl_media_add_url_data (GrlMedia *media, const gchar *url, const gchar *mime)
{
  GrlRelatedKeys *relkeys = grl_related_keys_new ();
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_URL, url);
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_MIME, mime);
  grl_data_add_related_keys (GRL_DATA (media), relkeys);
}

/**
 * grl_media_add_author:
 * @media: a #GrlMedia
 * @author: an author for @media
 *
 * Adds a new author to @media.
 **/
void
grl_media_add_author (GrlMedia *media, const gchar *author)
{
  grl_data_add_string (GRL_DATA (media), GRL_METADATA_KEY_AUTHOR, author);
}

/**
 * grl_media_add_thumbnail:
 * @media: a #GrlMedia
 * @thumbnail: a thumbnail for @media
 *
 * Adds a new thumbnail to @media.
 **/
void
grl_media_add_thumbnail (GrlMedia *media, const gchar *thumbnail)
{
  grl_data_add_string (GRL_DATA (media), GRL_METADATA_KEY_THUMBNAIL, thumbnail);
}

/**
 * grl_media_add_thumbnail_binary:
 * @media: a #GrlMedia
 * @thumbnail: a buffer containing the thumbnail for @media
 * @size: size of buffer
 *
 * Adds a new thumbnail to @media.
 **/
void
grl_media_add_thumbnail_binary (GrlMedia *media,
                                const guint8 *thumbnail,
                                gsize size)
{
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
 **/
void
grl_media_add_external_player (GrlMedia *media, const gchar *player)
{
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
 **/
void
grl_media_add_external_url (GrlMedia *media, const gchar *url)
{
  grl_data_add_string (GRL_DATA (media),
                       GRL_METADATA_KEY_EXTERNAL_URL,
                       url);
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
  GList *key;
  GList *keylist;
  GRegex *type_regex;
  GString *serial;
  GrlKeyID grlkey;
  GrlPluginRegistry *registry;
  const GValue *value;
  const gchar *id;
  const gchar *source;
  const gchar *type_name;
  gchar *protocol;
  gchar *serial_media;
  va_list va_serial;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  g_return_val_if_fail ((source = grl_media_get_source (media)), NULL);

  /* Check serialization type */
  switch (serial_type) {
  case GRL_MEDIA_SERIALIZE_FULL:
    registry = grl_plugin_registry_get_default ();
    keylist = grl_plugin_registry_get_metadata_keys (registry);
    serial_media = grl_media_serialize_extended (media,
                                                 GRL_MEDIA_SERIALIZE_PARTIAL,
                                                 keylist);
    g_list_free (keylist);
    break;
  case GRL_MEDIA_SERIALIZE_BASIC:
  case GRL_MEDIA_SERIALIZE_PARTIAL:
    type_name = g_type_name (G_TYPE_FROM_INSTANCE (media));

    /* Convert typename to scheme protocol */
    type_regex = g_regex_new ("GrlMedia(.*)", 0, 0, NULL);
    protocol = g_regex_replace (type_regex,
                                type_name,
                                -1,
                                0,
                                "grl\\L\\1\\E",
                                0,
                                NULL);
    g_regex_unref (type_regex);

    /* Build serial string with escaped components */
    serial = g_string_sized_new (SERIAL_STRING_ALLOC);
    g_string_assign (serial, protocol);
    g_string_append (serial, "://");
    g_string_append_uri_escaped (serial, source, NULL, TRUE);
    id = grl_media_get_id (media);
    if (id) {
      g_string_append_c (serial, '/');
      g_string_append_uri_escaped (serial, id, NULL, TRUE);
    }

    g_free (protocol);

    /* Include all properties */
    if (serial_type == GRL_MEDIA_SERIALIZE_PARTIAL) {
      g_string_append_c (serial, '?');
      registry = grl_plugin_registry_get_default ();

      va_start (va_serial, serial_type);
      keylist = va_arg (va_serial, GList *);
      for (key = keylist; key; key = g_list_next (key)) {
        grlkey = key->data;
        /* Skip id and source keys */
        if (grlkey == GRL_METADATA_KEY_ID ||
            grlkey == GRL_METADATA_KEY_SOURCE) {
          continue;
        }
        value = grl_data_get (GRL_DATA (media), grlkey);
        if (value) {
          g_string_append_printf (serial,
                                  "%s=",
                                  GRL_METADATA_KEY_GET_NAME (grlkey));
          if (G_VALUE_HOLDS_STRING (value)) {
            g_string_append_uri_escaped (serial,
                                         g_value_get_string (value),
                                         NULL,
                                         TRUE);
          } else if (G_VALUE_HOLDS_INT (value)) {
            g_string_append_printf (serial, "%d", g_value_get_int (value));
          } else if (G_VALUE_HOLDS_FLOAT (value)) {
            g_string_append_printf (serial, "%f", g_value_get_float (value));
          }
          g_string_append_c (serial, '&');
        }
      }

      va_end (va_serial);

      /* Remove trailing ?/& character */
      g_string_erase (serial, serial->len - 1, -1);
    }
    serial_media = g_string_free (serial, FALSE);
    break;
  default:
    serial_media = NULL;
  }

  return serial_media;
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
  GMatchInfo *match_info;
  GRegex *protocol_regex;
  GRegex *query_regex;
  GRegex *uri_regex;
  GType type_media;
  GrlKeyID grlkey;
  GrlMedia *media;
  GrlPluginRegistry *registry;
  gchar *escaped_value;
  gchar *keyname;
  gchar *protocol;
  gchar *query;
  gchar *type_name;
  gchar *value;

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
  protocol_regex = g_regex_new ("(grl)(.?)(.*)", G_REGEX_CASELESS, 0, NULL);
  type_name = g_regex_replace (protocol_regex,
                               protocol,
                               -1 ,
                               0,
                               "GrlMedia\\u\\2\\L\\3\\E",
                               0,
                               NULL);
  g_regex_unref (protocol_regex);
  g_free (protocol);

  type_media = g_type_from_name (type_name);
  if (type_media) {
    media = GRL_MEDIA (g_object_new (type_media, NULL));
  } else {
    GRL_WARNING ("There is no type %s", type_name);
    g_free (type_name);
    g_match_info_free (match_info);
    return NULL;
  }

  g_free (type_name);

  /* Add source */
  escaped_value = g_match_info_fetch (match_info, 2);
  value = g_uri_unescape_string (escaped_value, NULL);
  grl_media_set_source (media, value);
  g_free (escaped_value);
  g_free (value);

  /* Add id */
  escaped_value = g_match_info_fetch (match_info, 3);
  if (escaped_value && escaped_value[0] == '/') {
    value = g_uri_unescape_string (escaped_value + 1, NULL);
    grl_media_set_id (media, value);
    g_free (value);
  }
  g_free (escaped_value);

  /* Check if there are more properties */
  query = g_match_info_fetch (match_info, 4);
  g_match_info_free (match_info);
  if (query) {
    registry = grl_plugin_registry_get_default ();
    query_regex = g_regex_new ("([^=&]+)=([^=&]*)", 0, 0, NULL);
    g_regex_match (query_regex, query, 0, &match_info);
    while (g_match_info_matches (match_info)) {
      keyname = g_match_info_fetch (match_info, 1);
      grlkey = grl_plugin_registry_lookup_metadata_key (registry, keyname);
      if (grlkey) {
        escaped_value = g_match_info_fetch (match_info, 2);
        value = g_uri_unescape_string (escaped_value, NULL);
        switch (GRL_METADATA_KEY_GET_TYPE (grlkey)) {
        case G_TYPE_STRING:
          grl_data_set_string (GRL_DATA (media), grlkey, value);
          break;
        case G_TYPE_INT:
          grl_data_set_int (GRL_DATA (media), grlkey, atoi (value));
          break;
        case G_TYPE_FLOAT:
          grl_data_set_float (GRL_DATA (media), grlkey, atof (value));
          break;
        }
        g_free (escaped_value);
        g_free (value);
      }
      g_free (keyname);
      g_match_info_next (match_info, NULL);
    }
    g_match_info_free (match_info);
    g_free (query);
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
 * Set the media's site
 *
 * Since: 0.1.4
 */
void
grl_media_set_site (GrlMedia *media, const gchar *site)
{
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_SITE,
                       site);
}

/**
 * grl_media_set_duration:
 * @media: the media
 * @duration: the duration
 *
 * Set the media's duration
 *
 * Since: 0.1.4
 */
void
grl_media_set_duration (GrlMedia *media, gint duration)
{
  grl_data_set_int (GRL_DATA (media),
                    GRL_METADATA_KEY_DURATION,
                    duration);
}

/**
 * grl_media_set_date:
 * @media: the media
 * @date: the date
 *
 * Set the media's date (TBD)
 */
void
grl_media_set_date (GrlMedia *media, const gchar *date)
{
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_DATE,
                       date);
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
 * Since: 0.1.4
 */
void
grl_media_set_last_played (GrlMedia *media, const gchar *last_played)
{
  grl_data_set_string (GRL_DATA (media),
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
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_STUDIO,
                       studio);
}

/**
 * grl_media_set_certificate:
 * @media: the media
 * @certificate: The rating certificate of the media
 *
 * Set the media certificate
 *
 * Since: 0.1.6
 */
void
grl_media_set_certificate (GrlMedia *media, const gchar *certificate)
{
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
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_LICENSE,
                       license);
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
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_URL);
}

/**
 * grl_media_get_url_data:
 * @media: the media object
 * @mime: (out) (transfer none): the mime-type, or %NULL to ignore.
 *
 * Returns: the media's URL and its mime-type.
 */
const gchar *
grl_media_get_url_data (GrlMedia *media, gchar **mime)
{
  return grl_media_get_url_data_nth (media, 0, mime);
}

/**
 * grl_media_get_url_data_nth:
 * @media: the media object
 * @index: element to retrieve
 * @mime: (out) (transfer none): the mime-type, or %NULL to ignore.
 *
 * Returns: the n-th media's URL and its mime-type.
 */
const gchar *
grl_media_get_url_data_nth (GrlMedia *media, guint index, gchar **mime)
{
  GrlRelatedKeys *relkeys =
    grl_data_get_related_keys (GRL_DATA (media), GRL_METADATA_KEY_URL, index);

  if (!relkeys) {
    return NULL;
  }

  if (mime) {
    *mime = (gchar *) grl_related_keys_get_string (relkeys,
                                                   GRL_METADATA_KEY_MIME);
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
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_AUTHOR);
}

/**
 * grl_media_get_author_nth:
 * @media: the media object
 * @index: element to retrieve
 *
 * Returns: the n-th media's author.
 */
const gchar *
grl_media_get_author_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys =
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
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_THUMBNAIL);
}

/**
 * grl_media_get_thumbnail_nth:
 * @media: the media object
 * @index: element to retrieve
 *
 * Returns: the n-th media's thumbnail.
 */
const gchar *
grl_media_get_thumbnail_nth (GrlMedia *media, guint index)
{
  GrlRelatedKeys *relkeys =
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
 */
const guint8 *
grl_media_get_thumbnail_binary_nth (GrlMedia *media, gsize *size, guint index)
{
  GrlRelatedKeys *relkeys =
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
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_SITE);
}

/**
 * grl_media_get_duration:
 * @media: the media object
 *
 * Returns: the media's duration
 *
 * Since: 0.1.4
 */
gint
grl_media_get_duration (GrlMedia *media)
{
  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_DURATION);
}

/**
 * grl_media_get_date:
 * @media: the media object
 *
 * Returns: the media's date (TBD)
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_date (GrlMedia *media)
{
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_DATE);
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
  return grl_data_get_int (GRL_DATA (media), GRL_METADATA_KEY_LAST_POSITION);
}

/**
 * grl_media_get_last_played:
 * @media: the media object
 *
 * Returns: the media's last played time
 *
 * Since: 0.1.4
 */
const gchar *
grl_media_get_last_played (GrlMedia *media)
{
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_LAST_PLAYED);
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
  return grl_data_get_string (GRL_DATA (media),
                              GRL_METADATA_KEY_EXTERNAL_PLAYER);
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
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_EXTERNAL_URL);
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
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_STUDIO);
}

/**
 * grl_media_get_certificate:
 * @media: the media object
 *
 * Returns: the media's certificate
 *
 * Since: 0.1.6
 */
const gchar *
grl_media_get_certificate (GrlMedia *media)
{
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
  return grl_data_get_string (GRL_DATA (media), GRL_METADATA_KEY_LICENSE);
}
