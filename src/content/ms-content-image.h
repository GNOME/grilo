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

#ifndef __MS_CONTENT_IMAGE_H__
#define __MS_CONTENT_IMAGE_H__

#include "ms-content-media.h"


G_BEGIN_DECLS

#define MS_TYPE_CONTENT_IMAGE                   \
  (ms_content_image_get_type())
#define MS_CONTENT_IMAGE(obj)                                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                   \
                               MS_TYPE_CONTENT_IMAGE,                   \
                               MsContentImage))
#define MS_CONTENT_IMAGE_CLASS(klass)                                   \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                                    \
                            MS_TYPE_CONTENT_IMAGE,                      \
                            MsContentImageClass))
#define IS_MS_CONTENT_IMAGE(obj)                                        \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                   \
                               MS_TYPE_CONTENT_IMAGE))
#define IS_MS_CONTENT_IMAGE_CLASS(klass)                                \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                                    \
                            MS_TYPE_CONTENT_IMAGE))
#define MS_CONTENT_IMAGE_GET_CLASS(obj)                                 \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                                    \
                              MS_TYPE_CONTENT_IMAGE,                    \
                              MsContentImageClass))

typedef struct _MsContentImage      MsContentImage;
typedef struct _MsContentImageClass MsContentImageClass;

struct _MsContentImageClass
{
    MsContentMediaClass parent_class;
};

struct _MsContentImage
{
    MsContentMedia parent;
};

#define ms_content_image_set_width(content, width)                      \
  ms_content_set_int(MS_CONTENT((content)), MS_METADATA_KEY_WIDTH, (width))
#define ms_content_image_set_height(content, height)                    \
  ms_content_set_int(MS_CONTENT((content)), MS_METADATA_KEY_HEIGHT, (height))

#define ms_content_image_get_width(content)                             \
  ms_content_get_int(MS_CONTENT((content)), MS_METADATA_KEY_WIDTH)
#define ms_content_image_get_height(content)                            \
  ms_content_get_int(MS_CONTENT((content)), MS_METADATA_KEY_HEIGHT)
#define ms_content_video_get_framerate(content)                         \
  ms_content_get_float(MS_CONTENT((content)), MS_METADATA_KEY_FRAMERATE)

GType ms_content_image_get_type (void) G_GNUC_CONST;
MsContentMedia *ms_content_image_new (void);
void ms_content_image_set_size (MsContentImage *content, gint width, gint height);

G_END_DECLS

#endif /* __CONTENT_IMAGE_H__ */
