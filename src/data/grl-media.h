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

#ifndef _GRL_MEDIA_H_
#define _GRL_MEDIA_H_

#include <grl-data.h>


G_BEGIN_DECLS

#define GRL_TYPE_MEDIA                          \
  (grl_media_get_type())

#define GRL_MEDIA(obj)                          \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),           \
                               GRL_TYPE_MEDIA,  \
                               GrlMedia))

#define GRL_MEDIA_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            GRL_TYPE_MEDIA,     \
                            GrlMediaClass))

#define GRL_IS_MEDIA(obj)                       \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),           \
                               GRL_TYPE_MEDIA))

#define GRL_IS_MEDIA_CLASS(klass)               \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),            \
                            GRL_TYPE_MEDIA))

#define GRL_MEDIA_GET_CLASS(obj)                \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),            \
                              GRL_TYPE_MEDIA,   \
                              GrlMediaClass))

typedef struct _GrlMedia      GrlMedia;
typedef struct _GrlMediaClass GrlMediaClass;

/**
 * GrlMediaClass:
 * @parent_class: the parent class structure
 *
 * Grilo Media Class
 */
struct _GrlMediaClass
{
  GrlDataClass parent_class;
};

struct _GrlMedia
{
  GrlData parent;
};

/**
 * grl_media_set_id:
 * @data: the media
 * @id: the identifier of the media
 *
 * Set the media identifier
 */
#define grl_media_set_id(data, id)              \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_ID,      \
                      (id))

/**
 * grl_media_set_url:
 * @data: the media
 * @url: the media's URL
 *
 * Set the media's URL
 */
#define grl_media_set_url(data, url)            \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_URL,     \
                      (url))

/**
 * grl_media_set_author:
 * @data: the media
 * @author: the media's author
 *
 * Set the media's author
 */
#define grl_media_set_author(data, author)      \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_AUTHOR,  \
                      (author))

/**
 * grl_media_set_title:
 * @data: the media
 * @title: the title
 *
 * Set the media's title
 */
#define grl_media_set_title(data, title)        \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_TITLE,   \
                      (title))

/**
 * grl_media_set_description:
 * @data: the media
 * @description: the description
 *
 * Set the media's description
 */
#define grl_media_set_description(data, description)    \
  grl_data_set_string(GRL_DATA((data)),                 \
                      GRL_METADATA_KEY_DESCRIPTION,     \
                      (description))

/**
 * grl_media_set_source:
 * @data: the media
 * @source: the source
 *
 * Set the media's source
 */
#define grl_media_set_source(data, source)      \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_SOURCE,  \
                      (source))

/**
 * grl_media_set_thumbnail:
 * @data: the media
 * @thumbnail: the thumbnail URL
 *
 * Set the media's thumbnail URL
 */
#define grl_media_set_thumbnail(data, thumbnail)        \
  grl_data_set_string(GRL_DATA((data)),                 \
                      GRL_METADATA_KEY_THUMBNAIL,       \
                      (thumbnail))

/**
 * grl_media_set_site:
 * @data: the media
 * @site: the site
 *
 * Set the media's site
 */
#define grl_media_set_site(data, site)          \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_SITE,    \
                      (site))
/**
 * grl_media_set_duration:
 * @data: the media
 * @duration: the duration
 *
 * Set the media's duration
 */
#define grl_media_set_duration(data, duration)  \
  grl_data_set_int(GRL_DATA((data)),            \
                   GRL_METADATA_KEY_DURATION,   \
                   (duration))

/**
 * grl_media_set_date:
 * @data: the media
 * @date: the date
 *
 * Set the media's date (TBD)
 */
#define grl_media_set_date(data, date)          \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_DATE,    \
                      (date))

/**
 * grl_media_set_mime:
 * @data: the media
 * @mime: the mime type
 *
 * Set the media's mime-type
 */
#define grl_media_set_mime(data, mime)          \
  grl_data_set_string(GRL_DATA((data)),         \
                      GRL_METADATA_KEY_MIME,    \
                      (mime))

void grl_media_set_rating (GrlMedia *media,
                           const gchar *rating,
                           const gchar *max);

/**
 * grl_media_get_id:
 * @data: the media object
 *
 * Returns: (type utf8) (transfer none): the media's identifier
 */
#define grl_media_get_id(data)                                  \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_ID)

/**
 * grl_media_get_url:
 * @data: the media object
 *
 * Returns: (type utf8) (transfer none): the media's URL
 */
#define grl_media_get_url(data)                                 \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_URL)

/**
 * grl_media_get_author:
 * @data: the media object
 *
 * Returns: (type utf8) (transfer none): the media's author
 */
#define grl_media_get_author(data)                                      \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_AUTHOR)

/**
 * grl_media_get_title:
 * @data: the media object
 *
 * Returns: (type utf8) (transfer none): the media's title
 */
#define grl_media_get_title(data)                               \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_TITLE)

/**
 * grl_media_get_description:
 * @data: the media object
 *
 * Returns: (type utf8) (transfer none): the media's description
 */
#define grl_media_get_description(data)                                 \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_DESCRIPTION)

/**
 * grl_media_get_source:
 * @data: the media object source
 *
 * Returns: (type utf8) (transfer none): the media's source
 */
#define grl_media_get_source(data)                                      \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_SOURCE)

/**
 * grl_media_get_thumbnail:
 * @data: the media object
 *
 * Returns: (type utf8) (transfer none): the media's thumbnail URL
 */
#define grl_media_get_thumbnail(data)                                   \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_THUMBNAIL)

/**
 * grl_media_get_site:
 * @data: the media object
 *
 * Returns: (type utf8) (transfer none): the media's site
 */
#define grl_media_get_site(data)                                \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_SITE)

/**
 * grl_media_get_duration:
 * @data: the media object
 *
 * Returns: the media's duration
 */
#define grl_media_get_duration(data)                            \
  grl_data_get_int(GRL_DATA((data)), GRL_METADATA_KEY_DURATION)

/**
 * grl_media_get_date:
 * @data: the media object
 *
 * Returns: (type utf8) (transfer none): the media's date (TBD)
 */
#define grl_media_get_date(data)                                \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_DATE)

/**
 * grl_media_get_mime:
 * @data: the media object
 *
 * Returns: (type utf8) (transfer none): the media's mime-type
 */
#define grl_media_get_mime(data)                                \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_MIME)

/**
 * grl_media_get_rating:
 * @data: the media object
 *
 * Returns: (type utf8) (transfer none): the media's rating
 */
#define grl_media_get_rating(data)                                      \
  grl_data_get_string(GRL_DATA((data)), GRL_METADATA_KEY_RATING)

GType grl_media_get_type (void) G_GNUC_CONST;
GrlMedia *grl_media_new (void);

G_END_DECLS

#endif /* _GRL_MEDIA_H_ */
