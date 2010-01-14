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

#ifndef __MS_CONTENT_VIDEO_H__
#define __MS_CONTENT_VIDEO_H__

#include "ms-content-media.h"


G_BEGIN_DECLS

#define MS_TYPE_CONTENT_VIDEO                   \
  (ms_content_video_get_type())
#define MS_CONTENT_VIDEO(obj)                                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                   \
                               MS_TYPE_CONTENT_VIDEO,                   \
                               MsContentVideo))
#define MS_CONTENT_VIDEO_CLASS(klass)                                   \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                                    \
                            MS_TYPE_CONTENT_VIDEO,                      \
                            MsContentVideoClass))
#define MS_IS_CONTENT_VIDEO(obj)                                        \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                   \
                               MS_TYPE_CONTENT_VIDEO))
#define MS_IS_CONTENT_VIDEO_CLASS(klass)                                \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                                    \
                            MS_TYPE_CONTENT_VIDEO))
#define MS_CONTENT_VIDEO_GET_CLASS(obj)                                 \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                                    \
                              MS_TYPE_CONTENT_VIDEO,                    \
                              MsContentVideoClass))

typedef struct _MsContentVideo      MsContentVideo;
typedef struct _MsContentVideoClass MsContentVideoClass;

struct _MsContentVideoClass
{
    MsContentMediaClass parent_class;
};

struct _MsContentVideo
{
    MsContentMedia parent;
};

#define ms_content_video_set_width(content, width)                      \
  ms_content_set_int(MS_CONTENT((content)), MS_METADATA_KEY_WIDTH, (width))
#define ms_content_video_set_height(content, height)                    \
  ms_content_set_int(MS_CONTENT((content)), MS_METADATA_KEY_HEIGHT, (height))
#define ms_content_video_set_framerate(content, framerate)              \
  ms_content_set_float(MS_CONTENT((content)), MS_METADATA_KEY_FRAMERATE, (framerate))

#define ms_content_video_get_width(content)                             \
  ms_content_get_int(MS_CONTENT((content)), MS_METADATA_KEY_WIDTH)
#define ms_content_video_get_height(content)                            \
  ms_content_get_int(MS_CONTENT((content)), MS_METADATA_KEY_HEIGHT)
#define ms_content_video_get_framerate(content)                         \
  ms_content_get_float(MS_CONTENT((content)), MS_METADATA_KEY_FRAMERATE)

GType ms_content_video_get_type (void) G_GNUC_CONST;
MsContentMedia *ms_content_video_new (void);
void ms_content_video_set_size (MsContentVideo *content, gint width, gint height);

G_END_DECLS

#endif /* __CONTENT_VIDEO_H__ */
