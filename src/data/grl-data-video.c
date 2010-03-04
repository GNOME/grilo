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
 * A multimedia data for video.
 *
 * This high level class represents an video multimedia item. It has methods to
 * set and get properties like framerate, width, height, and so on.
 *
 */

#include "grl-data-video.h"


static void grl_data_video_dispose (GObject *object);
static void grl_data_video_finalize (GObject *object);

G_DEFINE_TYPE (GrlDataVideo, grl_data_video, GRL_TYPE_MEDIA);

static void
grl_data_video_class_init (GrlDataVideoClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_data_video_dispose;
  gobject_class->finalize = grl_data_video_finalize;
}

static void
grl_data_video_init (GrlDataVideo *self)
{
}

static void
grl_data_video_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_data_video_parent_class)->dispose (object);
}

static void
grl_data_video_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_data_video_parent_class)->finalize (object);
}

/**
 * grl_data_video_new:
 *
 * Creates a new data video object.
 *
 * Returns: a newly-allocated data video.
 **/
GrlMedia *
grl_data_video_new (void)
{
  return GRL_MEDIA (g_object_new (GRL_TYPE_DATA_VIDEO,
                                  NULL));
}

void
grl_data_video_set_size (GrlDataVideo *video,
                         gint width,
                         int height)
{
  grl_data_video_set_width (video, width);
  grl_data_video_set_height (video, height);
}
