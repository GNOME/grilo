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

#ifndef __MS_CONTENT_BOX_H__
#define __MS_CONTENT_BOX_H__

#include "ms-content-media.h"


G_BEGIN_DECLS

#define MS_TYPE_CONTENT_BOX                     \
  (ms_content_box_get_type())
#define MS_CONTENT_BOX(obj)                                             \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                   \
                               MS_TYPE_CONTENT_BOX,                     \
                               MsContentBox))
#define MS_CONTENT_BOX_CLASS(klass)                                     \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                                    \
                            MS_TYPE_CONTENT_BOX,                        \
                            MsContentBoxClass))
#define MS_IS_CONTENT_BOX(obj)                                          \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                   \
                               MS_TYPE_CONTENT_BOX))
#define MS_IS_CONTENT_BOX_CLASS(klass)                                  \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                                    \
                            MS_TYPE_CONTENT_BOX))
#define MS_CONTENT_BOX_GET_CLASS(obj)                                   \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                                    \
                              MS_TYPE_CONTENT_BOX,                      \
                              MsContentBoxClass))

typedef struct _MsContentBox      MsContentBox;
typedef struct _MsContentBoxClass MsContentBoxClass;

struct _MsContentBoxClass
{
    MsContentMediaClass parent_class;
};

struct _MsContentBox
{
    MsContentMedia parent;
};

#define ms_content_box_set_childcount(content, childcount)              \
  ms_content_set_int(MS_CONTENT((content)), MS_METADATA_KEY_CHILDCOUNT, (childcount))

#define ms_content_box_get_childcount(content)                          \
  ms_content_get_int(MS_CONTENT((content)), MS_METADATA_KEY_CHILDCOUNT)

GType ms_content_box_get_type (void) G_GNUC_CONST;
MsContentMedia *ms_content_box_new (void);

G_END_DECLS

#endif /* __CONTENT_BOX_H__ */
