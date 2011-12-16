/*
 * Copyright (C) 2010, 2011 Igalia S.L.
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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#ifndef _GRL_UTIL_H_
#define _GRL_UTIL_H_

#include <glib.h>

G_BEGIN_DECLS

void grl_paging_translate (guint skip,
                           guint count,
                           guint max_page_size,
                           guint *page_size,
                           guint *page_number,
                           guint *internal_offset);

GList *grl_list_from_va (gpointer p, ...) G_GNUC_NULL_TERMINATED;

GDateTime * grl_date_time_from_iso8601 (const gchar *date);

G_END_DECLS

#endif /* _GRL_UTIL_H_ */
