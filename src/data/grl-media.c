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
 * A multimedia data.
 *
 * This high level class represents a multimedia item. It has methods to
 * set and get properties like author, title, description, and so on.
 *
 */

#include "grl-media.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "grl-media"

#define RATING_MAX  5.00

static void grl_media_dispose (GObject *object);
static void grl_media_finalize (GObject *object);

G_DEFINE_TYPE (GrlMedia, grl_media, GRL_TYPE_DATA);

static void
grl_media_class_init (GrlMediaClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_media_dispose;
  gobject_class->finalize = grl_media_finalize;
}

static void
grl_media_init (GrlMedia *self)
{
}

static void
grl_media_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_media_parent_class)->dispose (object);
}

static void
grl_media_finalize (GObject *object)
{
  g_debug ("grl_media_finalize (%s)",
	   grl_data_get_string (GRL_DATA (object),
                                GRL_METADATA_KEY_TITLE));
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_media_parent_class)->finalize (object);
}

/**
 * grl_media_new:
 *
 * Creates a new data media object.
 *
 * Returns: a newly-allocated data media.
 **/
GrlMedia *
grl_media_new (void)
{
  return g_object_new (GRL_TYPE_MEDIA,
		       NULL);
}

void
grl_media_set_rating (GrlMedia *media,
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
  grl_data_set_string (GRL_DATA (media),
                       GRL_METADATA_KEY_RATING,
                       value);
}
