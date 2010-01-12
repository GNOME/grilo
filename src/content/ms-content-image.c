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

#include "ms-content-image.h"


static void ms_content_image_dispose (GObject *object);
static void ms_content_image_finalize (GObject *object);

G_DEFINE_TYPE (MsContentImage, ms_content_image, MS_TYPE_CONTENT_MEDIA);

static void
ms_content_image_class_init (MsContentImageClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;

    gobject_class->dispose = ms_content_image_dispose;
    gobject_class->finalize = ms_content_image_finalize;
}

static void
ms_content_image_init (MsContentImage *self)
{
}

static void
ms_content_image_dispose (GObject *object)
{
    G_OBJECT_CLASS (ms_content_image_parent_class)->dispose (object);
}

static void
ms_content_image_finalize (GObject *object)
{
    g_signal_handlers_destroy (object);
    G_OBJECT_CLASS (ms_content_image_parent_class)->finalize (object);
}

/**
 * ms_content_image_new:
 *
 * Creates a new content image object.
 *
 * Returns: a newly-allocated content image.
 **/
MsContentMedia *
ms_content_image_new (void)
{
  return MS_CONTENT_MEDIA (g_object_new (MS_TYPE_CONTENT_IMAGE,
                                         NULL));
}

void
ms_content_image_set_size (MsContentImage *content,
                           gint width,
                           int height)
{
  ms_content_image_set_width (content, width);
  ms_content_image_set_height (content, height);
}
