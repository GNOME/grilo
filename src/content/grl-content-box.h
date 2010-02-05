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

#ifndef _GRL_CONTENT_BOX_H_
#define _GRL_CONTENT_BOX_H_

#include <grl-content-media.h>


G_BEGIN_DECLS

#define GRL_TYPE_CONTENT_BOX                    \
  (grl_content_box_get_type())

#define GRL_CONTENT_BOX(obj)                            \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_CONTENT_BOX,    \
                               GrlContentBox))

#define GRL_CONTENT_BOX_CLASS(klass)                    \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_CONTENT_BOX,       \
                            GrlContentBoxClass))

#define GRL_IS_CONTENT_BOX(obj)                         \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_CONTENT_BOX))

#define GRL_IS_CONTENT_BOX_CLASS(klass)                 \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_CONTENT_BOX))

#define GRL_CONTENT_BOX_GET_CLASS(obj)                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_CONTENT_BOX,     \
                              GrlContentBoxClass))

typedef struct _GrlContentBox      GrlContentBox;
typedef struct _GrlContentBoxClass GrlContentBoxClass;

struct _GrlContentBoxClass
{
  GrlContentMediaClass parent_class;
};

struct _GrlContentBox
{
  GrlContentMedia parent;
};

GType grl_content_box_get_type (void) G_GNUC_CONST;
GrlContentMedia *grl_content_box_new (void);
void grl_content_box_set_childcount (GrlContentBox *content, gint childcount);
gint grl_content_box_get_childcount (GrlContentBox *content);

G_END_DECLS

#endif /* _CONTENT_BOX_H_ */
