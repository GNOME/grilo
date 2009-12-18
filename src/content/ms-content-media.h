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

#ifndef __MS_CONTENT_MEDIA_H__
#define __MS_CONTENT_MEDIA_H__

#include "ms-content.h"


G_BEGIN_DECLS

#define MS_TYPE_CONTENT_MEDIA                   \
  (ms_content_media_get_type())
#define MS_CONTENT_MEDIA(obj)                                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                   \
                               MS_TYPE_CONTENT_MEDIA,                   \
                               MsContentMedia))
#define MS_CONTENT_MEDIA_CLASS(klass)                                   \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                                    \
                            MS_TYPE_CONTENT_MEDIA,                      \
                            MsContentMediaClass))
#define IS_MS_CONTENT_MEDIA(obj)                                        \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                   \
                               MS_TYPE_CONTENT_MEDIA))
#define IS_MS_CONTENT_MEDIA_CLASS(klass)                                \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                                    \
                            MS_TYPE_CONTENT_MEDIA))
#define MS_CONTENT_MEDIA_GET_CLASS(obj)                                 \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                                    \
                              MS_TYPE_CONTENT_MEDIA,                    \
                              MsContentMediaClass))

typedef struct _MsContentMedia      MsContentMedia;
typedef struct _MsContentMediaClass MsContentMediaClass;

struct _MsContentMediaClass
{
    MsContentClass parent_class;
};

struct _MsContentMedia
{
    MsContent parent;
};

#define ms_content_media_set_id(content, id)                            \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_ID, (id))
#define ms_content_media_set_url(content, url)                          \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_URL, (url))
#define ms_content_media_set_author(content, author) \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_AUTHOR, (author))
#define ms_content_media_set_title(content, title)                        \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_TITLE, (title))
#define ms_content_media_set_description(content, description) \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_DESCRIPTION, (description))
#define ms_content_media_set_source(content, source) \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_SOURCE, (source))
#define ms_content_media_set_thumbnail(content, thumbnail) \
  ms_content_set_string(MS_CONTENT((content)), MS_METADATA_KEY_THUMBNAIL, (thumbnail))

#define ms_content_media_get_id(content) \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_ID)
#define ms_content_media_get_url(content) \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_URL)
#define ms_content_media_get_author(content) \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_AUTHOR)
#define ms_content_media_get_title(content) \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_TITLE)
#define ms_content_media_get_description(content) \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_DESCRIPTION)
#define ms_content_media_get_source(content) \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_SOURCE)
#define ms_content_media_get_thumbnail(content) \
  ms_content_get_string(MS_CONTENT((content)), MS_METADATA_KEY_THUMBNAIL)

GType ms_content_media_get_type (void) G_GNUC_CONST;
MsContentMedia *ms_content_media_new (void);

G_END_DECLS

#endif /* __CONTENT_MEDIA_H__ */
