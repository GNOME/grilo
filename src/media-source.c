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

#include "media-source.h"

#include <string.h>

#define MEDIA_SOURCE_GET_PRIVATE(object)				\
  (G_TYPE_INSTANCE_GET_PRIVATE((object), MEDIA_SOURCE_TYPE, MediaSourcePrivate))

struct _MediaSourcePrivate {
  guint padding;
};

G_DEFINE_ABSTRACT_TYPE (MediaSource, media_source, METADATA_SOURCE_TYPE);

static void
media_source_class_init (MediaSourceClass *media_source_class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (media_source_class);

  g_type_class_add_private (media_source_class, sizeof (MediaSourcePrivate));
}

static void
media_source_init (MediaSource *source)
{
  source->priv = MEDIA_SOURCE_GET_PRIVATE (source);
  memset (source->priv, 0, sizeof (MediaSourcePrivate));
}

guint
media_source_browse (MediaSource *source, 
		     const gchar *container_id,
		     const gchar *const *keys,
		     guint skip,
		     guint count,
		     guint flags,
		     MediaSourceResultCb callback,
		     gpointer user_data)
{
  return MEDIA_SOURCE_GET_CLASS (source)->browse (source,
						  container_id,
						  keys,
						  skip, count,
						  callback, user_data);
}

guint
media_source_search (MediaSource *source,
		     const gchar *text,
		     const gchar *filter,
		     guint skip,
		     guint count,
		     guint flags,
		     MediaSourceResultCb callback,
		     gpointer user_data)
{
  return MEDIA_SOURCE_GET_CLASS (source)->search (source,
						  text,
						  filter,
						  skip, count,
						  callback, user_data);
}
