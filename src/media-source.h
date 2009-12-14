/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
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

#ifndef _MEDIA_SOURCE_H_
#define _MEDIA_SOURCE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "media-plugin.h"
#include "metadata-source.h"
#include "content.h"

#include <glib.h>
#include <glib-object.h>

/* Macros */

#define MEDIA_SOURCE_TYPE (media_source_get_type ())
#define MEDIA_SOURCE(obj)						\
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MEDIA_SOURCE_TYPE, MediaSource))
#define IS_MEDIA_SOURCE(obj)					\
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MEDIA_SOURCE_TYPE))
#define MEDIA_SOURCE_CLASS(klass)					\
  (G_TYPE_CHECK_CLASS_CAST((klass), MEDIA_SOURCE_TYPE,  MediaSourceClass))
#define IS_MEDIA_SOURCE_CLASS(klass)			\
  (G_TYPE_CHECK_CLASS_TYPE((klass), MEDIA_SOURCE_TYPE))
#define MEDIA_SOURCE_GET_CLASS(obj)					\
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MEDIA_SOURCE_TYPE, MediaSourceClass))

/* MediaSource object */

typedef struct _MediaSource MediaSource;
typedef struct _MediaSourcePrivate MediaSourcePrivate;

struct _MediaSource {

  MetadataSource parent;

  /*< private >*/
  MediaSourcePrivate *priv;
};

/* Callbacks for MediaSource class */

typedef void (*MediaSourceResultCb) (MediaSource *source,
				     guint browse_id,
                                     Content *media,
				     guint remaining,
				     gpointer user_data,
				     const GError *error);

/* MediaSource class */

typedef struct _MediaSourceClass MediaSourceClass;

struct _MediaSourceClass {

  MetadataSourceClass parent_class;
  
  guint (*browse) (MediaSource *source, 
		   const gchar *container_id,
		   const gchar *const *keys,
		   guint skip,
		   guint count,
		   MediaSourceResultCb callback,
		   gpointer user_data);
  
  guint (*search) (MediaSource *source,
		   const gchar *text,
		   const gchar *filter,
		   guint skip,
		   guint count,
		   MediaSourceResultCb callback,
		   gpointer user_data);
};

G_BEGIN_DECLS

GType media_source_get_type (void);

guint media_source_browse (MediaSource *source, 
			   const gchar *container_id,
			   const gchar *const *keys,
			   guint skip,
			   guint count,
			   guint flags,
			   MediaSourceResultCb callback,
			   gpointer user_data);

guint media_source_search (MediaSource *source,
			   const gchar *text,
			   const gchar *filter,
			   guint skip,
			   guint count,
			   guint flags,
			   MediaSourceResultCb callback,
			   gpointer user_data);

G_END_DECLS

#endif
