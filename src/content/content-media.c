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
 * A multimedia content.
 *
 * This high level class represents a multimedia container. It has methods to
 * set and get properties like author, title, description, and so on.
 *
 */

#include "content-media.h"


static void content_media_dispose (GObject *object);
static void content_media_finalize (GObject *object);

G_DEFINE_TYPE (ContentMedia, content_media, CONTENT_TYPE);

static void
content_media_class_init (ContentMediaClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;

    gobject_class->dispose = content_media_dispose;
    gobject_class->finalize = content_media_finalize;
}

static void
content_media_init (ContentMedia *self)
{
}

static void
content_media_dispose (GObject *object)
{
    G_OBJECT_CLASS (content_media_parent_class)->dispose (object);
}

static void
content_media_finalize (GObject *object)
{
    g_signal_handlers_destroy (object);
    G_OBJECT_CLASS (content_media_parent_class)->finalize (object);
}

/**
 * content_media_new:
 *
 * Creates a new ContentMedia object.
 *
 * Returns: a newly-allocated ContentMedia.
 **/
ContentMedia *
content_media_new (void)
{
  return g_object_new (CONTENT_TYPE_MEDIA, NULL);
}
