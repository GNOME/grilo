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

#include "ms-content-media.h"


static void ms_content_media_dispose (GObject *object);
static void ms_content_media_finalize (GObject *object);

G_DEFINE_TYPE (MsContentMedia, ms_content_media, MS_TYPE_CONTENT);

static void
ms_content_media_class_init (MsContentMediaClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;

    gobject_class->dispose = ms_content_media_dispose;
    gobject_class->finalize = ms_content_media_finalize;
}

static void
ms_content_media_init (MsContentMedia *self)
{
}

static void
ms_content_media_dispose (GObject *object)
{
    G_OBJECT_CLASS (ms_content_media_parent_class)->dispose (object);
}

static void
ms_content_media_finalize (GObject *object)
{
    g_signal_handlers_destroy (object);
    G_OBJECT_CLASS (ms_content_media_parent_class)->finalize (object);
}

/**
 * ms_content_media_new:
 *
 * Creates a new content media object.
 *
 * Returns: a newly-allocated content media.
 **/
MsContentMedia *
ms_content_media_new (void)
{
  return g_object_new (MS_TYPE_CONTENT_MEDIA, NULL);
}
