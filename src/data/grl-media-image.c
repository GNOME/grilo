/*
 * Copyright (C) 2010 Igalia S.L.
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
 * @data: the image instance
 * @width: the image's width
 *
 * Set the width of the image
 */
void
grl_media_image_set_width (GrlMediaImage *data, gint width)
{
  grl_data_set_int (GRL_DATA (data),
                    GRL_METADATA_KEY_WIDTH,
                    width);
}

/**
 * grl_media_image_set_height:
 * @data: the image instance
 * @height: the image's height
 *
 * Set the height of the image
 */
void
grl_media_image_set_height (GrlMediaImage *data, gint height)
{
  grl_data_set_int (GRL_DATA (data),
                    GRL_METADATA_KEY_HEIGHT,
                    height);
}

/**
 * grl_media_image_get_width:
 * @data: The image instance
 *
 * Returns: the width of the image
 */
gint
grl_media_image_get_width (GrlMediaImage *data)
{
  return grl_data_get_int (GRL_DATA (data), GRL_METADATA_KEY_WIDTH);
}

/**
 * grl_media_image_get_height:
 * @data: the image instance
 *
 * Returns: the height of the image
 */
gint
grl_media_image_get_height (GrlMediaImage *data)
{
  return grl_data_get_int (GRL_DATA (data), GRL_METADATA_KEY_HEIGHT);
}
