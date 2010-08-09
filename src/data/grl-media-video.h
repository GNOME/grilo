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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

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

void grl_media_video_set_width (GrlMediaVideo *data, gint width);

void grl_media_video_set_height (GrlMediaVideo *data, gint height);

void grl_media_video_set_framerate (GrlMediaVideo *data, gfloat framerate);

gint grl_media_video_get_width (GrlMediaVideo *data);

gint grl_media_video_get_height (GrlMediaVideo *data);

gfloat grl_media_video_get_framerate (GrlMediaVideo *data);

GType grl_media_video_get_type (void) G_GNUC_CONST;

GrlMedia *grl_media_video_new (void);

void grl_media_video_set_size (GrlMediaVideo *video,
                               gint width,
                               gint height);

G_END_DECLS

#endif /* _GRL_MEDIA_VIDEO_H_ */
