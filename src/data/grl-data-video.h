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

#ifndef _GRL_DATA_VIDEO_H_
#define _GRL_DATA_VIDEO_H_

#include <grl-media.h>


G_BEGIN_DECLS

#define GRL_TYPE_DATA_VIDEO                     \
  (grl_data_video_get_type())

#define GRL_DATA_VIDEO(obj)                             \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_DATA_VIDEO,     \
                               GrlDataVideo))

#define GRL_DATA_VIDEO_CLASS(klass)                     \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_DATA_VIDEO,        \
                            GrlDataVideoClass))

#define GRL_IS_DATA_VIDEO(obj)                          \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_DATA_VIDEO))

#define GRL_IS_DATA_VIDEO_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_DATA_VIDEO))

#define GRL_DATA_VIDEO_GET_CLASS(obj)                   \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_DATA_VIDEO,      \
                              GrlDataVideoClass))

typedef struct _GrlDataVideo      GrlDataVideo;
typedef struct _GrlDataVideoClass GrlDataVideoClass;

struct _GrlDataVideoClass
{
  GrlMediaClass parent_class;
};

struct _GrlDataVideo
{
  GrlMedia parent;
};

#define grl_data_video_set_width(data, width)   \
  grl_data_set_int(GRL_DATA((data)),            \
                   GRL_METADATA_KEY_WIDTH,      \
                   (width))

#define grl_data_video_set_height(data, height) \
  grl_data_set_int(GRL_DATA((data)),            \
                   GRL_METADATA_KEY_HEIGHT,     \
                   (height))

#define grl_data_video_set_framerate(data, framerate)   \
  grl_data_set_float(GRL_DATA((data)),                  \
                     GRL_METADATA_KEY_FRAMERATE,        \
                     (framerate))

#define grl_data_video_get_width(data)                          \
  grl_data_get_int(GRL_DATA((data)), GRL_METADATA_KEY_WIDTH)
#define grl_data_video_get_height(data)                         \
  grl_data_get_int(GRL_DATA((data)), GRL_METADATA_KEY_HEIGHT)
#define grl_data_video_get_framerate(data)                              \
  grl_data_get_float(GRL_DATA((data)), GRL_METADATA_KEY_FRAMERATE)

GType grl_data_video_get_type (void) G_GNUC_CONST;

GrlMedia *grl_data_video_new (void);

void grl_data_video_set_size (GrlDataVideo *video,
                              gint width,
                              gint height);

G_END_DECLS

#endif /* _GRL_DATA_VIDEO_H_ */
