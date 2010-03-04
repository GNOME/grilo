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

#ifndef _GRL_DATA_MEDIA_H_
#define _GRL_DATA_MEDIA_H_

#include <grl-data.h>


G_BEGIN_DECLS

#define GRL_TYPE_DATA_MEDIA                     \
  (grl_data_media_get_type())

#define GRL_DATA_MEDIA(obj)                             \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_DATA_MEDIA,     \
                               GrlDataMedia))

#define GRL_DATA_MEDIA_CLASS(klass)                     \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_DATA_MEDIA,        \
                            GrlDataMediaClass))

#define GRL_IS_DATA_MEDIA(obj)                          \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_DATA_MEDIA))

#define GRL_IS_DATA_MEDIA_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_DATA_MEDIA))

#define GRL_DATA_MEDIA_GET_CLASS(obj)                   \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_DATA_MEDIA,      \
                              GrlDataMediaClass))

typedef struct _GrlDataMedia      GrlDataMedia;
typedef struct _GrlDataMediaClass GrlDataMediaClass;

struct _GrlDataMediaClass
{
  GrlDataClass parent_class;
};

struct _GrlDataMedia
{
  GrlData parent;
};

#define grl_data_media_set_id(data, id)         \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_ID,      \
                      (id))

#define grl_data_media_set_url(data, url)       \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_URL,     \
                      (url))

#define grl_data_media_set_author(data, author) \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_AUTHOR,  \
                      (author))

#define grl_data_media_set_title(data, title)   \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_TITLE,   \
                      (title))

#define grl_data_media_set_description(data, description)       \
  grl_data_set_string(GRL_DATA((data)),                         \
                      GRL_METADATA_KEY_DESCRIPTION,             \
                      (description))

#define grl_data_media_set_source(data, source) \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_SOURCE,  \
                      (source))

#define grl_data_media_set_thumbnail(data, thumbnail)   \
  grl_data_set_string(GRL_DATA((data)),                 \
                      GRL_METADATA_KEY_THUMBNAIL,       \
                      (thumbnail))

#define grl_data_media_set_site(data, site)     \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_SITE,    \
                      (site))

#define grl_data_media_set_duration(data, duration)     \
  grl_data_set_int(GRL_DATA((data)),                    \
                   GRL_METADATA_KEY_DURATION,           \
                   (duration))

#define grl_data_media_set_date(data, date)     \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_DATE,    \
                      (date))

#define grl_data_media_set_mime(data, mime)     \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_MIME,    \
                      (mime))

void grl_data_media_set_rating (GrlDataMedia *data,
                                const gchar *rating,
                                const gchar *max);

#define grl_data_media_get_id(data)                             \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_ID)
#define grl_data_media_get_url(data)                            \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_URL)
#define grl_data_media_get_author(data)                                 \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_AUTHOR)
#define grl_data_media_get_title(data)                          \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_TITLE)
#define grl_data_media_get_description(data)                            \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_DESCRIPTION)
#define grl_data_media_get_source(data)                                 \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_SOURCE)
#define grl_data_media_get_thumbnail(data)                              \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_THUMBNAIL)
#define grl_data_media_get_site(data)                           \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_SITE)
#define grl_data_media_get_duration(data)                       \
  grl_data_get_int(GRL_DATA((data)), GRL_METADATA_KEY_DURATION)
#define grl_data_media_get_date(data)                           \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_DATE)
#define grl_data_media_get_mime(data)                           \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_MIME)
#define grl_data_media_get_rating(data)                                 \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_RATING)

GType grl_data_media_get_type (void) G_GNUC_CONST;
GrlDataMedia *grl_data_media_new (void);

G_END_DECLS

#endif /* _GRL_DATA_MEDIA_H_ */
