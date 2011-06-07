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

#ifndef _GRL_MEDIA_BOX_H_
#define _GRL_MEDIA_BOX_H_

#include <grl-media.h>
#include <grl-definitions.h>

G_BEGIN_DECLS

#define GRL_TYPE_MEDIA_BOX                      \
  (grl_media_box_get_type())

#define GRL_MEDIA_BOX(obj)                              \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_MEDIA_BOX,      \
                               GrlMediaBox))

#define GRL_MEDIA_BOX_CLASS(klass)              \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            GRL_TYPE_MEDIA_BOX, \
                            GrlMediaBoxClass))

#define GRL_IS_MEDIA_BOX(obj)                           \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_MEDIA_BOX))

#define GRL_IS_MEDIA_BOX_CLASS(klass)                   \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_MEDIA_BOX))

#define GRL_MEDIA_BOX_GET_CLASS(obj)                    \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_MEDIA_BOX,       \
                              GrlMediaBoxClass))

typedef struct _GrlMediaBox      GrlMediaBox;
typedef struct _GrlMediaBoxClass GrlMediaBoxClass;

struct _GrlMediaBox
{
  GrlMedia parent;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING_SMALL];
};

/**
 * GrlMediaBoxClass:
 * @parent_class: the parent class structure
 *
 * Grilo Media box Class
 */
struct _GrlMediaBoxClass
{
  GrlMediaClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

GType grl_media_box_get_type (void) G_GNUC_CONST;
GrlMedia *grl_media_box_new (void);
void grl_media_box_set_childcount (GrlMediaBox *box, gint childcount);
gint grl_media_box_get_childcount (GrlMediaBox *box);

G_END_DECLS

#endif /* _GRL_MEDIA_BOX_H_ */
