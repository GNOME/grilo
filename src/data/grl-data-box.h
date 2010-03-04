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

#ifndef _GRL_DATA_BOX_H_
#define _GRL_DATA_BOX_H_

#include <grl-data-media.h>


G_BEGIN_DECLS

#define GRL_TYPE_DATA_BOX                       \
  (grl_data_box_get_type())

#define GRL_DATA_BOX(obj)                               \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_DATA_BOX,       \
                               GrlDataBox))

#define GRL_DATA_BOX_CLASS(klass)               \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            GRL_TYPE_DATA_BOX,  \
                            GrlDataBoxClass))

#define GRL_IS_DATA_BOX(obj)                            \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_DATA_BOX))

#define GRL_IS_DATA_BOX_CLASS(klass)            \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),            \
                            GRL_TYPE_DATA_BOX))

#define GRL_DATA_BOX_GET_CLASS(obj)                     \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_DATA_BOX,        \
                              GrlDataBoxClass))

typedef struct _GrlDataBox      GrlDataBox;
typedef struct _GrlDataBoxClass GrlDataBoxClass;

struct _GrlDataBoxClass
{
  GrlDataMediaClass parent_class;
};

struct _GrlDataBox
{
  GrlDataMedia parent;
};

GType grl_data_box_get_type (void) G_GNUC_CONST;
GrlDataMedia *grl_data_box_new (void);
void grl_data_box_set_childcount (GrlDataBox *box, gint childcount);
gint grl_data_box_get_childcount (GrlDataBox *box);

G_END_DECLS

#endif /* _GRL_DATA_BOX_H_ */
