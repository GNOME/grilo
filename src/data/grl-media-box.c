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
 * SECTION:grl-media-box
 * @short_description: A container for multiple medias
 * @see_also: #GrlMedia, #GrlMediaVideo, #GrlMediaAudio, #GrlMediaImage
 *
 * This high level class represents a container for multiple medias.
 *
 * Usually, when you get a media, it is either an Image, a Video or a Music
 * file, but when you create a hierarchy, for instance
 *
 * <informalexample>
 * ROOT -&gt; &lt;artist&gt; -&gt; &lt;album&gt; -&gt; &lt;media&gt;
 * </informalexample>
 *
 * the medias are only the leaf nodes, but which kind of "media"
 * is an album?
 *
 * #GrlMediaBox is used to represent this kind of nodes: it is a "box" which
 * can be browsed to get the medias (or other boxes) under it.
 *
 * In fact, you can only browse through media-boxes.
 */

#include "grl-media-box.h"

#define MIME_BOX "x-grl/box"

static void grl_media_box_dispose (GObject *object);
static void grl_media_box_finalize (GObject *object);

G_DEFINE_TYPE (GrlMediaBox, grl_media_box, GRL_TYPE_MEDIA);

static void
grl_media_box_class_init (GrlMediaBoxClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_media_box_dispose;
  gobject_class->finalize = grl_media_box_finalize;
}

static void
grl_media_box_init (GrlMediaBox *self)
{
  grl_media_box_set_childcount (self, GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN);
  grl_media_set_mime (GRL_MEDIA (self), MIME_BOX);
}

static void
grl_media_box_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_media_box_parent_class)->dispose (object);
}

static void
grl_media_box_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_media_box_parent_class)->finalize (object);
}

/**
 * grl_media_box_new:
 *
 * Creates a new data box object.
 *
 * Returns: a newly-allocated data box.
 *
 * Since: 0.1.4
 */
GrlMedia *
grl_media_box_new (void)
{
  return GRL_MEDIA (g_object_new (GRL_TYPE_MEDIA_BOX,
                                  NULL));
}

/**
 * grl_media_box_set_childcount:
 * @box: the media box instance
 * @childcount: number of children
 *
 * Sets the number of children of this box. Use
 * #GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN if it is unknown.
 *
 * Since: 0.1.4
 */
void
grl_media_box_set_childcount (GrlMediaBox *box,
                              gint childcount)
{
  g_return_if_fail (GRL_IS_MEDIA_BOX (box));

  if (childcount != GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN) {
    grl_data_set_int (GRL_DATA (box),
                      GRL_METADATA_KEY_CHILDCOUNT,
                      childcount);
  }
}

/**
 * grl_media_box_get_childcount:
 * @box: the media box instance
 *
 * Number of children of this box.
 *
 * Returns: number of children, or #GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN if
 * unknown.
 *
 * Since: 0.1.4
 */
gint
grl_media_box_get_childcount (GrlMediaBox *box)
{
  g_return_val_if_fail (GRL_IS_MEDIA_BOX (box),
                        GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN);

  const GValue *value = grl_data_get (GRL_DATA (box),
                                      GRL_METADATA_KEY_CHILDCOUNT);

  if (value) {
    return g_value_get_int (value);
  } else {
    return GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN;
  }
}
