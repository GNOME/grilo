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

#ifndef _GRL_CONTENT_IMAGE_H_
#define _GRL_CONTENT_IMAGE_H_

#include <grl-content-media.h>


G_BEGIN_DECLS

#define GRL_TYPE_CONTENT_IMAGE                  \
  (grl_content_image_get_type())

#define GRL_CONTENT_IMAGE(obj)                          \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_CONTENT_IMAGE,  \
                               GrlContentImage))

#define GRL_CONTENT_IMAGE_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_CONTENT_IMAGE,     \
                            GrlContentImageClass))

#define GRL_IS_CONTENT_IMAGE(obj)                       \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_CONTENT_IMAGE))

#define GRL_IS_CONTENT_IMAGE_CLASS(klass)               \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_CONTENT_IMAGE))

#define GRL_CONTENT_IMAGE_GET_CLASS(obj)                \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_CONTENT_IMAGE,   \
                              GrlContentImageClass))

typedef struct _GrlContentImage      GrlContentImage;
typedef struct _GrlContentImageClass GrlContentImageClass;

struct _GrlContentImageClass
{
  GrlContentMediaClass parent_class;
};

struct _GrlContentImage
{
  GrlContentMedia parent;
};

#define grl_content_image_set_width(content, width)     \
  grl_content_set_int(GRL_CONTENT((content)),           \
                      GRL_METADATA_KEY_WIDTH,           \
                      (width))

#define grl_content_image_set_height(content, height)   \
  grl_content_set_int(GRL_CONTENT((content)),           \
                      GRL_METADATA_KEY_HEIGHT,          \
                      (height))

#define grl_content_image_get_width(content)                            \
  grl_content_get_int(GRL_CONTENT((content)), GRL_METADATA_KEY_WIDTH)
#define grl_content_image_get_height(content)                           \
  grl_content_get_int(GRL_CONTENT((content)), GRL_METADATA_KEY_HEIGHT)

GType grl_content_image_get_type (void) G_GNUC_CONST;
GrlContentMedia *grl_content_image_new (void);
void grl_content_image_set_size (GrlContentImage *content,
                                 gint width,
                                 gint height);

G_END_DECLS

#endif /* _GRL_CONTENT_IMAGE_H_ */
