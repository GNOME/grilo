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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#ifndef _GRL_MEDIA_IMAGE_H_
#define _GRL_MEDIA_IMAGE_H_

#include <grl-media.h>
#include <grl-definitions.h>

G_BEGIN_DECLS

#define GRL_TYPE_MEDIA_IMAGE                    \
  (grl_media_image_get_type())

#define GRL_MEDIA_IMAGE(obj)                            \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_MEDIA_IMAGE,    \
                               GrlMediaImage))

#define GRL_MEDIA_IMAGE_CLASS(klass)                    \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_MEDIA_IMAGE,       \
                            GrlMediaImageClass))

#define GRL_IS_MEDIA_IMAGE(obj)                         \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_MEDIA_IMAGE))

#define GRL_IS_MEDIA_IMAGE_CLASS(klass)                 \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_MEDIA_IMAGE))

#define GRL_MEDIA_IMAGE_GET_CLASS(obj)                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_MEDIA_IMAGE,     \
                              GrlMediaImageClass))

typedef struct _GrlMediaImage      GrlMediaImage;
typedef struct _GrlMediaImageClass GrlMediaImageClass;

/**
 * GrlMediaImageClass:
 * @parent_class: the parent class structure
 *
 * Grilo Media image Class
 */
struct _GrlMediaImageClass
{
  GrlMediaClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

struct _GrlMediaImage
{
  GrlMedia parent;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING_SMALL];
};

void grl_media_image_set_width (GrlMediaImage *image, gint width);

void grl_media_image_set_height (GrlMediaImage *image, gint height);

gint grl_media_image_get_width (GrlMediaImage *image);

gint grl_media_image_get_height (GrlMediaImage *image);

GType grl_media_image_get_type (void) G_GNUC_CONST;
GrlMedia *grl_media_image_new (void);
void grl_media_image_set_size (GrlMediaImage *image,
                               gint width,
                               gint height);

const gchar *grl_media_image_get_url_data (GrlMediaImage *image,
                                           gchar **mime,
                                           gint *width,
                                           gint *height);

const gchar *grl_media_image_get_url_data_nth (GrlMediaImage *image,
                                               guint index,
                                               gchar **mime,
                                               gint *width,
                                               gint *height);

void grl_media_image_set_url_data (GrlMediaImage *image,
                                   const gchar *url,
                                   const gchar *mime,
                                   gint width,
                                   gint height);

void grl_media_image_add_url_data (GrlMediaImage *image,
                                   const gchar *url,
                                   const gchar *mime,
                                   gint width,
                                   gint height);

G_END_DECLS

#endif /* _GRL_MEDIA_IMAGE_H_ */
