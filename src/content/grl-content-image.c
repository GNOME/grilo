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

/*
 * A multimedia content for image.
 *
 * This high level class represents an image multimedia item. It has methods to
 * set and get properties like framerate, width, height, and so on.
 *
 */

#include "grl-content-image.h"


static void grl_content_image_dispose (GObject *object);
static void grl_content_image_finalize (GObject *object);

G_DEFINE_TYPE (GrlContentImage, grl_content_image, GRL_TYPE_CONTENT_MEDIA);

static void
grl_content_image_class_init (GrlContentImageClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_content_image_dispose;
  gobject_class->finalize = grl_content_image_finalize;
}

static void
grl_content_image_init (GrlContentImage *self)
{
}

static void
grl_content_image_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_content_image_parent_class)->dispose (object);
}

static void
grl_content_image_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_content_image_parent_class)->finalize (object);
}

/**
 * grl_content_image_new:
 *
 * Creates a new content image object.
 *
 * Returns: a newly-allocated content image.
 **/
GrlContentMedia *
grl_content_image_new (void)
{
  return GRL_CONTENT_MEDIA (g_object_new (GRL_TYPE_CONTENT_IMAGE,
                                          NULL));
}

void
grl_content_image_set_size (GrlContentImage *content,
                            gint width,
                            int height)
{
  grl_content_image_set_width (content, width);
  grl_content_image_set_height (content, height);
}
