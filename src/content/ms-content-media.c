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

/*
 * A multimedia content.
 *
 * This high level class represents a multimedia item. It has methods to
 * set and get properties like author, title, description, and so on.
 *
 */

#include "ms-content-media.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "ms-content-media"

#define RATING_MAX  5.00

static void ms_content_media_dispose (GObject *object);
static void ms_content_media_finalize (GObject *object);

G_DEFINE_TYPE (MsContentMedia, ms_content_media, MS_TYPE_CONTENT);

static void
ms_content_media_class_init (MsContentMediaClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;

    gobject_class->dispose = ms_content_media_dispose;
    gobject_class->finalize = ms_content_media_finalize;
}

static void
ms_content_media_init (MsContentMedia *self)
{
}

static void
ms_content_media_dispose (GObject *object)
{
    G_OBJECT_CLASS (ms_content_media_parent_class)->dispose (object);
}

static void
ms_content_media_finalize (GObject *object)
{
  g_debug ("ms_content_media_finalize (%s)",
	   ms_content_get_string (MS_CONTENT (object), MS_METADATA_KEY_TITLE));
    g_signal_handlers_destroy (object);
    G_OBJECT_CLASS (ms_content_media_parent_class)->finalize (object);
}

/**
 * ms_content_media_new:
 *
 * Creates a new content media object.
 *
 * Returns: a newly-allocated content media.
 **/
MsContentMedia *
ms_content_media_new (void)
{
  return g_object_new (MS_TYPE_CONTENT_MEDIA,
		       NULL);
}

void
ms_content_media_set_rating (MsContentMedia *content, 
			     const gchar *rating,
			     const gchar *max)
{
  g_return_if_fail (rating != NULL);
  g_return_if_fail (max != NULL);

  gchar *tmp;
  gdouble rating_value = g_ascii_strtod (rating, &tmp);
  if (*tmp != '\0' || rating_value < 0) {
    g_critical ("Invalid rating value: %s", rating);
    return;
  }
  gdouble max_value = g_ascii_strtod (max, &tmp);
  if (*tmp != '\0' || max_value <= 0) {
    g_critical ("Invalid MAX value for rating: '%s'", max);
    return;
  }

  char value[G_ASCII_DTOSTR_BUF_SIZE];
  gdouble normalized_value = (rating_value * RATING_MAX) / max_value;
  g_ascii_formatd (value, sizeof (value), "%.2f", normalized_value);
  ms_content_set_string (MS_CONTENT (content), MS_METADATA_KEY_RATING, value);
}
