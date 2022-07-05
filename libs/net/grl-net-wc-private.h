/*
 * Copyright (C) 2010, 2011 Igalia S.L.
 * Copyright (C) 2012 Canonical Ltd.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Víctor M. Jáquez L. <vjaquez@igalia.com>
 *          Juan A. Suarez Romero <jasuarez@igalia.com>
 *          Jens Georg <jensg@openismus.com>
 *          Mathias Hasselmann <mathias@openismus.com>
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

#include <libsoup/soup.h>

struct _GrlNetWc {
  GObject parent;

  SoupSession *session;
  char *user_agent;
  SoupLoggerLogLevel log_level;
  /* throttling in secs */
  guint throttling;
  /* last request time, timestamp in seconds */
  gint64 last_request;
  /* closure queue for delayed requests */
  GQueue *pending;
  /* cache size in Mb */
  gboolean use_cache;
  guint cache_size;
  gchar *previous_data;
};
