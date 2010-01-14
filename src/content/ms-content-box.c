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
 * A container for multiple medias.
 *
 * This high level class represents a container for multiple medias.
 *
 */

#include "ms-content-box.h"

#define MIME_BOX "x-ms/box"

static void ms_content_box_dispose (GObject *object);
static void ms_content_box_finalize (GObject *object);

G_DEFINE_TYPE (MsContentBox, ms_content_box, MS_TYPE_CONTENT_MEDIA);

static void
ms_content_box_class_init (MsContentBoxClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;

    gobject_class->dispose = ms_content_box_dispose;
    gobject_class->finalize = ms_content_box_finalize;
}

static void
ms_content_box_init (MsContentBox *self)
{
  ms_content_box_set_childcount (self, MS_METADATA_KEY_CHILDCOUNT_UNKNOWN);
  ms_content_media_set_mime (MS_CONTENT_MEDIA (self), MIME_BOX);
}

static void
ms_content_box_dispose (GObject *object)
{
    G_OBJECT_CLASS (ms_content_box_parent_class)->dispose (object);
}

static void
ms_content_box_finalize (GObject *object)
{
    g_signal_handlers_destroy (object);
    G_OBJECT_CLASS (ms_content_box_parent_class)->finalize (object);
}

/**
 * ms_content_box_new:
 *
 * Creates a new content box object.
 *
 * Returns: a newly-allocated content box.
 **/
MsContentMedia *
ms_content_box_new (void)
{
  return MS_CONTENT_MEDIA (g_object_new (MS_TYPE_CONTENT_BOX,
                                         NULL));
}

/**
 * ms_content_box_set_childcount:
 * @content: content to change
 * @childcount: number of children
 *
 * Sets the number of children of this box. Use
 * #MS_METADATA_KEY_CHILDCOUNT_UNKNOWN if it is unknown.
 **/
void
ms_content_box_set_childcount (MsContentBox *content,
                               gint childcount)
{
  g_return_if_fail (MS_IS_CONTENT_BOX (content));

  if (childcount != MS_METADATA_KEY_CHILDCOUNT_UNKNOWN) {
    ms_content_set_int (MS_CONTENT (content),
                        MS_METADATA_KEY_CHILDCOUNT,
                        childcount);
  } else {
    ms_content_set (MS_CONTENT (content),
                    MS_METADATA_KEY_CHILDCOUNT,
                    NULL);
  }
}

/**
 * ms_content_box_get_childcount:
 * @content: content to inspect
 *
 * Number of children of this box.
 *
 * Returns: number of children, or #MS_METADATA_KEY_CHILDCOUNT_UNKNOWN if
 * unknown.
 **/
gint
ms_content_box_get_childcount (MsContentBox *content)
{
  g_return_val_if_fail (MS_IS_CONTENT_BOX (content),
                        MS_METADATA_KEY_CHILDCOUNT_UNKNOWN);

  const GValue *value = ms_content_get (MS_CONTENT (content),
        	                          MS_METADATA_KEY_CHILDCOUNT);

  if (value) {
    return g_value_get_int (value);
  } else {
    return MS_METADATA_KEY_CHILDCOUNT_UNKNOWN;
  }
}
