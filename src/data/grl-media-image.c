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
 * SECTION:grl-media-image
 * @short_description: A multimedia data for image
 * @see_also: #GrlConfig, #GrlMediaBox, #GrlMediaAudio, #GrlMediaVideo
 *
 * This high level class represents an image multimedia item. It has methods to
 * set and get the size, width and height properties
 */

#include "grl-media-image.h"


static void grl_media_image_dispose (GObject *object);
static void grl_media_image_finalize (GObject *object);

G_DEFINE_TYPE (GrlMediaImage, grl_media_image, GRL_TYPE_MEDIA);

static void
grl_media_image_class_init (GrlMediaImageClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_media_image_dispose;
  gobject_class->finalize = grl_media_image_finalize;
}

static void
grl_media_image_init (GrlMediaImage *self)
{
}

static void
grl_media_image_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_media_image_parent_class)->dispose (object);
}

static void
grl_media_image_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_media_image_parent_class)->finalize (object);
}

/**
 * grl_media_image_new:
 *
 * Creates a new data image object.
 *
 * Returns: a newly-allocated data image.
 *
 * Since: 0.1.4
 **/
GrlMedia *
grl_media_image_new (void)
{
  return GRL_MEDIA (g_object_new (GRL_TYPE_MEDIA_IMAGE,
                                  NULL));
}

/**
 * grl_media_image_set_size:
 * @image: the image instance
 * @width: the image's width
 * @height: the image's height
 *
 * Set the size of the image
 *
 * Since: 0.1.4
 */
void
grl_media_image_set_size (GrlMediaImage *image,
                          gint width,
                          gint height)
{
  grl_media_image_set_width (image, width);
  grl_media_image_set_height (image, height);
}

/**
 * grl_media_image_set_width:
 * @image: the image instance
 * @width: the image's width
 *
 * Set the width of the image
 *
 * Since: 0.1.4
 */
void
grl_media_image_set_width (GrlMediaImage *image, gint width)
{
  grl_data_set_int (GRL_DATA (image),
                    GRL_METADATA_KEY_WIDTH,
                    width);
}

/**
 * grl_media_image_set_height:
 * @image: the image instance
 * @height: the image's height
 *
 * Set the height of the image
 *
 * Since: 0.1.4
 */
void
grl_media_image_set_height (GrlMediaImage *image, gint height)
{
  grl_data_set_int (GRL_DATA (image),
                    GRL_METADATA_KEY_HEIGHT,
                    height);
}

/**
 * grl_media_image_set_url_data:
 * @image: the media instance
 * @url: the image's url
 * @mime: image mime-type
 * @width: image width, or -1 to ignore
 * @height: image height, or -1 to ignore
 *
 * Sets all the keys related with the URL of an image resource in one go.
 *
 * Since: 0.1.10
 **/
void
grl_media_image_set_url_data (GrlMediaImage *image,
                              const gchar *url,
                              const gchar *mime,
                              gint width,
                              gint height)
{
  GrlRelatedKeys *relkeys = grl_related_keys_new ();
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_URL, url);
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_MIME, mime);
  if (width >= 0) {
    grl_related_keys_set_int (relkeys, GRL_METADATA_KEY_WIDTH, width);
  }
  if (height >= 0) {
    grl_related_keys_set_int (relkeys, GRL_METADATA_KEY_HEIGHT, height);
  }
  grl_data_set_related_keys (GRL_DATA (image), relkeys, 0);
}

/**
 * grl_media_image_add_url_data:
 * @image: the image instance
 * @url: a image's url
 * @mime: image mime-type
 * @width: image width, or -1 to ignore
 * @height: image height, or -1 to ignore
 *
 * Sets all the keys related with the URL of a media resource and adds it to
 * @image (useful for resources with more than one URL).
 *
 * Since: 0.1.10
 **/
void
grl_media_image_add_url_data (GrlMediaImage *image,
                              const gchar *url,
                              const gchar *mime,
                              gint width,
                              gint height)
{
  GrlRelatedKeys *relkeys = grl_related_keys_new ();
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_URL, url);
  grl_related_keys_set_string (relkeys, GRL_METADATA_KEY_MIME, mime);
  if (width >= 0) {
    grl_related_keys_set_int (relkeys, GRL_METADATA_KEY_WIDTH, width);
  }
  if (height >= 0) {
    grl_related_keys_set_int (relkeys, GRL_METADATA_KEY_HEIGHT, height);
  }
  grl_data_add_related_keys (GRL_DATA (image), relkeys);
}

/**
 * grl_media_image_get_width:
 * @image: The image instance
 *
 * Returns: the width of the image
 *
 * Since: 0.1.4
 */
gint
grl_media_image_get_width (GrlMediaImage *image)
{
  return grl_data_get_int (GRL_DATA (image), GRL_METADATA_KEY_WIDTH);
}

/**
 * grl_media_image_get_height:
 * @image: the image instance
 *
 * Returns: the height of the image
 *
 * Since: 0.1.4
 */
gint
grl_media_image_get_height (GrlMediaImage *image)
{
  return grl_data_get_int (GRL_DATA (image), GRL_METADATA_KEY_HEIGHT);
}

/**
 * grl_media_image_get_url_data:
 * @image: the image instance
 * @mime: (out) (transfer none): the url mime-type, or %NULL to ignore
 * @width: the width, or %NULL to ignore
 * @height: the height, or %NULL to ignore
 *
 * Returns: all the keys related with the URL of an image resource in one go.
 *
 * Since: 0.1.10
 **/
const gchar *
grl_media_image_get_url_data (GrlMediaImage *image,
                              gchar **mime,
                              gint *width,
                              gint *height)
{
  return grl_media_image_get_url_data_nth (image, 0, mime, width, height);
}

/**
 * grl_media_image_get_url_data_nth:
 * @image: the image instance
 * @index: element to retrieve
 * @mime: (out) (transfer none): the url mime-type, or %NULL to ignore
 * @width: the width, or %NULL to ignore
 * @height: the height, or %NULL to ignore
 *
 * Returns: all the keys related with the URL number @index of an image resource
 * in one go.
 *
 * Since: 0.1.10
 **/
const gchar *
grl_media_image_get_url_data_nth (GrlMediaImage *image,
                                  guint index,
                                  gchar **mime,
                                  gint *width,
                                  gint *height)
{
  GrlRelatedKeys *relkeys =
    grl_data_get_related_keys (GRL_DATA (image), GRL_METADATA_KEY_URL, index);

  if (!relkeys) {
    return NULL;
  }

  if (mime) {
    *mime = (gchar *) grl_related_keys_get_string (relkeys,
                                                   GRL_METADATA_KEY_MIME);
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
 * grl_media_image_get_camera_model:
 * @image: the image instance
 *
 * Returns: model of camera used to take picture
 *
 * Since: 0.1.12
 */
const gchar *
grl_media_image_get_camera_model (GrlMediaImage *image)
{
  return grl_data_get_string (GRL_DATA (image),
                              GRL_METADATA_KEY_CAMERA_MODEL);
}

/**
  * grl_media_set_camera_model:
  * @image: the image instance
  * @camera_model: model of camera used to take picture
  *
  * Set the camera_model of the image
  *
  * Since: 0.1.12
  */
void
grl_media_image_set_camera_model (GrlMediaImage *image,
                                  const gchar * camera_model)
{
  grl_data_set_string (GRL_DATA (image),
                       GRL_METADATA_KEY_CAMERA_MODEL,
                       camera_model);
}


/**
 * grl_media_image_get_flash_used:
 * @image: the image instance
 *
 * Returns: whether the flash was used
 * See
 * http://library.gnome.org/devel/ontology/unstable/nmm-classes.html#nmm-Flash
 *
 * Since: 0.1.12
 */
const gchar *
grl_media_image_get_flash_used (GrlMediaImage *image)
{
  return grl_data_get_string (GRL_DATA (image),
                              GRL_METADATA_KEY_FLASH_USED);
}

/**
  * grl_media_set_flash_used:
  * @image: the image instance
  * @flash_used: whether the flash was used
  *
  * Set the flash_used of the image
  * See
  * http://library.gnome.org/devel/ontology/unstable/nmm-classes.html#nmm-Flash
  *
  * Since: 0.1.12
  */
void
grl_media_image_set_flash_used (GrlMediaImage *image,
                                const gchar  *flash_used)
{
  grl_data_set_string (GRL_DATA (image),
                       GRL_METADATA_KEY_FLASH_USED,
                       flash_used);
}


/**
 * grl_media_image_get_exposure_time:
 * @image: the image instance
 *
 * Returns: picture's exposure time
 *
 * Since: 0.1.12
 */
gfloat
grl_media_image_get_exposure_time (GrlMediaImage *image)
{
  return grl_data_get_float (GRL_DATA (image),
                             GRL_METADATA_KEY_EXPOSURE_TIME);
}

/**
  * grl_media_set_exposure_time:
  * @image: the image instance
  * @exposure_time: picture's exposure time
  *
  * Set the exposure_time of the image
  *
  * Since: 0.1.12
  */
void
grl_media_image_set_exposure_time (GrlMediaImage *image,
                                   gfloat  exposure_time)
{
  grl_data_set_float (GRL_DATA (image),
                      GRL_METADATA_KEY_EXPOSURE_TIME,
                      exposure_time);
}

/**
 * grl_media_image_get_iso_speed:
 * @image: the image instance
 *
 * Returns: picture's iso speed
 *
 * Since: 0.1.12
 */
gfloat
grl_media_image_get_iso_speed (GrlMediaImage *image)
{
  return grl_data_get_float (GRL_DATA (image),
                             GRL_METADATA_KEY_ISO_SPEED);
}

/**
  * grl_media_image_set_iso_speed:
  * @image: the image instance
  * @iso_speed: picture's iso speed
  *
  * Set the iso_speed of the image
  *
  * Since: 0.1.12
  */
void
grl_media_image_set_iso_speed (GrlMediaImage *image,
                               gfloat  iso_speed)
{
  grl_data_set_float (GRL_DATA (image),
                      GRL_METADATA_KEY_ISO_SPEED,
                      iso_speed);
}

/**
 * grl_media_image_get_orientation:
 * @image: the image instance
 *
 * Returns: degrees clockwise orientation of the picture
 *
 * Since: 0.1.12
 */
gint
grl_media_image_get_orientation (GrlMediaImage *image)
{
  return grl_data_get_int (GRL_DATA (image),
                           GRL_METADATA_KEY_ORIENTATION);
}

/**
  * grl_media_image_set_orientation:
  * @image: the image instance
  * @orientation: degrees clockwise orientation of the picture
  *
  * Set the orientation of the image
  *
  * Since: 0.1.12
  */
void
grl_media_image_set_orientation (GrlMediaImage *image,
                                 gint orientation)
{
  grl_data_set_int (GRL_DATA (image),
                    GRL_METADATA_KEY_ORIENTATION,
                    orientation % 360);
}
