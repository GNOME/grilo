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

#ifndef _GRL_DATA_IMAGE_H_
#define _GRL_DATA_IMAGE_H_

#include <grl-data-media.h>


G_BEGIN_DECLS

#define GRL_TYPE_DATA_IMAGE                     \
  (grl_data_image_get_type())

#define GRL_DATA_IMAGE(obj)                             \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_DATA_IMAGE,     \
                               GrlDataImage))

#define GRL_DATA_IMAGE_CLASS(klass)                     \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_DATA_IMAGE,        \
                            GrlDataImageClass))

#define GRL_IS_DATA_IMAGE(obj)                          \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_DATA_IMAGE))

#define GRL_IS_DATA_IMAGE_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_DATA_IMAGE))

#define GRL_DATA_IMAGE_GET_CLASS(obj)                   \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_DATA_IMAGE,      \
                              GrlDataImageClass))

typedef struct _GrlDataImage      GrlDataImage;
typedef struct _GrlDataImageClass GrlDataImageClass;

struct _GrlDataImageClass
{
  GrlDataMediaClass parent_class;
};

struct _GrlDataImage
{
  GrlDataMedia parent;
};

#define grl_data_image_set_width(data, width)   \
  grl_data_set_int(GRL_DATA((data)),            \
                   GRL_METADATA_KEY_WIDTH,      \
                   (width))

#define grl_data_image_set_height(data, height) \
  grl_data_set_int(GRL_DATA((data)),            \
                   GRL_METADATA_KEY_HEIGHT,     \
                   (height))

#define grl_data_image_get_width(data)                          \
  grl_data_get_int(GRL_DATA((data)), GRL_METADATA_KEY_WIDTH)
#define grl_data_image_get_height(data)                         \
  grl_data_get_int(GRL_DATA((data)), GRL_METADATA_KEY_HEIGHT)

GType grl_data_image_get_type (void) G_GNUC_CONST;
GrlDataMedia *grl_data_image_new (void);
void grl_data_image_set_size (GrlDataImage *image,
                              gint width,
                              gint height);

G_END_DECLS

#endif /* _GRL_DATA_IMAGE_H_ */
