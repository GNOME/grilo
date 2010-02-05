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

#ifndef _GRL_CONTENT_MEDIA_H_
#define _GRL_CONTENT_MEDIA_H_

#include <grl-content.h>


G_BEGIN_DECLS

#define GRL_TYPE_CONTENT_MEDIA                  \
  (grl_content_media_get_type())

#define GRL_CONTENT_MEDIA(obj)                          \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_CONTENT_MEDIA,  \
                               GrlContentMedia))

#define GRL_CONTENT_MEDIA_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                    \
                            GRL_TYPE_CONTENT_MEDIA,     \
                            GrlContentMediaClass))

#define GRL_IS_CONTENT_MEDIA(obj)                       \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_CONTENT_MEDIA))

#define GRL_IS_CONTENT_MEDIA_CLASS(klass)               \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_CONTENT_MEDIA))

#define GRL_CONTENT_MEDIA_GET_CLASS(obj)                \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_CONTENT_MEDIA,   \
                              GrlContentMediaClass))

typedef struct _GrlContentMedia      GrlContentMedia;
typedef struct _GrlContentMediaClass GrlContentMediaClass;

struct _GrlContentMediaClass
{
  GrlContentClass parent_class;
};

struct _GrlContentMedia
{
  GrlContent parent;
};

#define grl_content_media_set_id(content, id)           \
  grl_content_set_string(GRL_CONTENT((content)),        \
                         GRL_METADATA_KEY_ID,           \
                         (id))

#define grl_content_media_set_url(content, url)                         \
  grl_content_set_string(GRL_CONTENT((content)),                        \
                         GRL_METADATA_KEY_URL,                          \
                         (url))

#define grl_content_media_set_author(content, author)                   \
  grl_content_set_string(GRL_CONTENT((content)),                        \
                         GRL_METADATA_KEY_AUTHOR,                       \
                         (author))

#define grl_content_media_set_title(content, title)     \
  grl_content_set_string(GRL_CONTENT((content)),        \
                         GRL_METADATA_KEY_TITLE,        \
                         (title))

#define grl_content_media_set_description(content, description) \
  grl_content_set_string(GRL_CONTENT((content)),                \
                         GRL_METADATA_KEY_DESCRIPTION,          \
                         (description))

#define grl_content_media_set_source(content, source)                   \
  grl_content_set_string(GRL_CONTENT((content)),                        \
                         GRL_METADATA_KEY_SOURCE,                       \
                         (source))

#define grl_content_media_set_thumbnail(content, thumbnail)             \
  grl_content_set_string(GRL_CONTENT((content)),                        \
                         GRL_METADATA_KEY_THUMBNAIL,                    \
                         (thumbnail))

#define grl_content_media_set_site(content, site)       \
  grl_content_set_string(GRL_CONTENT((content)),        \
                         GRL_METADATA_KEY_SITE,         \
                         (site))

#define grl_content_media_set_duration(content, duration)               \
  grl_content_set_int(GRL_CONTENT((content)),                           \
                      GRL_METADATA_KEY_DURATION,                        \
                      (duration))

#define grl_content_media_set_date(content, date)                       \
  grl_content_set_string(GRL_CONTENT((content)),                        \
                         GRL_METADATA_KEY_DATE,\
                         (date))

#define grl_content_media_set_mime(content, mime)                       \
  grl_content_set_string(GRL_CONTENT((content)),                        \
                         GRL_METADATA_KEY_MIME,                         \
                         (mime))

void grl_content_media_set_rating (GrlContentMedia *content,
                                   const gchar *rating,
                                   const gchar *max);

#define grl_content_media_get_id(content)                               \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_ID)
#define grl_content_media_get_url(content)                              \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_URL)
#define grl_content_media_get_author(content)                           \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_AUTHOR)
#define grl_content_media_get_title(content)                            \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_TITLE)
#define grl_content_media_get_description(content)                      \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_DESCRIPTION)
#define grl_content_media_get_source(content)                           \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_SOURCE)
#define grl_content_media_get_thumbnail(content)                        \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_THUMBNAIL)
#define grl_content_media_get_site(content)                             \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_SITE)
#define grl_content_media_get_duration(content)                         \
  grl_content_get_int(GRL_CONTENT((content)), GRL_METADATA_KEY_DURATION)
#define grl_content_media_get_date(content)                             \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_DATE)
#define grl_content_media_get_mime(content)                             \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_MIME)
#define grl_content_media_get_rating(content)                           \
  grl_content_get_string(GRL_CONTENT((content)), GRL_METADATA_KEY_RATING)

GType grl_content_media_get_type (void) G_GNUC_CONST;
GrlContentMedia *grl_content_media_new (void);

G_END_DECLS

#endif /* _GRL_CONTENT_MEDIA_H_ */
