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

#ifndef _GRL_MEDIA_VIDEO_H_
#define _GRL_MEDIA_VIDEO_H_

#include <grl-media.h>


G_BEGIN_DECLS

#define GRL_TYPE_MEDIA_VIDEO                    \
  (grl_media_video_get_type())

#define GRL_MEDIA_VIDEO(obj)                            \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_MEDIA_VIDEO,    \
                               GrlMediaVideo))

#define GRL_MEDIA_VIDEO_CLASS(klass)                    \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_MEDIA_VIDEO,       \
                            GrlMediaVideoClass))

#define GRL_IS_MEDIA_VIDEO(obj)                         \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_MEDIA_VIDEO))

#define GRL_IS_MEDIA_VIDEO_CLASS(klass)                 \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_MEDIA_VIDEO))

#define GRL_MEDIA_VIDEO_GET_CLASS(obj)                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_MEDIA_VIDEO,     \
                              GrlMediaVideoClass))

typedef struct _GrlMediaVideo      GrlMediaVideo;
typedef struct _GrlMediaVideoClass GrlMediaVideoClass;

/**
 * GrlMediaVideoClass:
 * @parent_class: the parent class structure
 *
 * Grilo Media video Class
 */
struct _GrlMediaVideoClass
{
  GrlMediaClass parent_class;
};

struct _GrlMediaVideo
{
  GrlMedia parent;
};

/**
 * grl_media_video_set_width:
 * @data: the media instance
 * @width: the video's width
 *
 * Set the width of the video
 */
#define grl_media_video_set_width(data, width)  \
  grl_data_set_int(GRL_DATA((data)),            \
                   GRL_METADATA_KEY_WIDTH,      \
                   (width))

/**
 * grl_media_video_set_height:
 * @data: the media instance
 * @height: the video's height
 *
 * Set the height of the video
 */
#define grl_media_video_set_height(data, height)        \
  grl_data_set_int(GRL_DATA((data)),                    \
                   GRL_METADATA_KEY_HEIGHT,             \
                   (height))

/**
 * grl_media_video_set_framerate:
 * @data: the media instance
 * @framerate: the video's framerate
 *
 * Set the framerate of the video
 */
#define grl_media_video_set_framerate(data, framerate)  \
  grl_data_set_float(GRL_DATA((data)),                  \
                     GRL_METADATA_KEY_FRAMERATE,        \
                     (framerate))

/**
 * grl_media_video_get_width:
 * @data: the media instance
 *
 * Returns: the width of the video
 */
#define grl_media_video_get_width(data)                         \
  grl_data_get_int(GRL_DATA((data)), GRL_METADATA_KEY_WIDTH)

/**
 * grl_media_video_get_height:
 * @data: the media instance
 *
 * Returns: the height of the video
 */
#define grl_media_video_get_height(data)                        \
  grl_data_get_int(GRL_DATA((data)), GRL_METADATA_KEY_HEIGHT)

/**
 * grl_media_video_get_framerate:
 * @data: the media instance
 *
 * Returns: the framerate of the video
 */
#define grl_media_video_get_framerate(data)                             \
  grl_data_get_float(GRL_DATA((data)), GRL_METADATA_KEY_FRAMERATE)

GType grl_media_video_get_type (void) G_GNUC_CONST;

GrlMedia *grl_media_video_new (void);

void grl_media_video_set_size (GrlMediaVideo *video,
                               gint width,
                               gint height);

G_END_DECLS

#endif /* _GRL_MEDIA_VIDEO_H_ */
