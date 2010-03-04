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

#include "grl-data-box.h"

#define MIME_BOX "x-grl/box"

static void grl_data_box_dispose (GObject *object);
static void grl_data_box_finalize (GObject *object);

G_DEFINE_TYPE (GrlDataBox, grl_data_box, GRL_TYPE_DATA_MEDIA);

static void
grl_data_box_class_init (GrlDataBoxClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_data_box_dispose;
  gobject_class->finalize = grl_data_box_finalize;
}

static void
grl_data_box_init (GrlDataBox *self)
{
  grl_data_box_set_childcount (self, GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN);
  grl_data_media_set_mime (GRL_DATA_MEDIA (self), MIME_BOX);
}

static void
grl_data_box_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_data_box_parent_class)->dispose (object);
}

static void
grl_data_box_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_data_box_parent_class)->finalize (object);
}

/**
 * grl_data_box_new:
 *
 * Creates a new data box object.
 *
 * Returns: a newly-allocated data box.
 **/
GrlDataMedia *
grl_data_box_new (void)
{
  return GRL_DATA_MEDIA (g_object_new (GRL_TYPE_DATA_BOX,
                                       NULL));
}

/**
 * grl_data_box_set_childcount:
 * @data: data to change
 * @childcount: number of children
 *
 * Sets the number of children of this box. Use
 * #GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN if it is unknown.
 **/
void
grl_data_box_set_childcount (GrlDataBox *box,
                             gint childcount)
{
  g_return_if_fail (GRL_IS_DATA_BOX (box));

  if (childcount != GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN) {
    grl_data_set_int (GRL_DATA (box),
                      GRL_METADATA_KEY_CHILDCOUNT,
                      childcount);
  } else {
    grl_data_set (GRL_DATA (box),
                  GRL_METADATA_KEY_CHILDCOUNT,
                  NULL);
  }
}

/**
 * grl_data_box_get_childcount:
 * @data: data to inspect
 *
 * Number of children of this box.
 *
 * Returns: number of children, or #GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN if
 * unknown.
 **/
gint
grl_data_box_get_childcount (GrlDataBox *box)
{
  g_return_val_if_fail (GRL_IS_DATA_BOX (box),
                        GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN);

  const GValue *value = grl_data_get (GRL_DATA (box),
                                      GRL_METADATA_KEY_CHILDCOUNT);

  if (value) {
    return g_value_get_int (value);
  } else {
    return GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN;
  }
}
