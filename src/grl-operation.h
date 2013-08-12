/*
 * Copyright (C) 2011 Igalia S.L.
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

#ifndef _GRL_OPERATION_H_
#define _GRL_OPERATION_H_

#include <glib.h>

G_BEGIN_DECLS

void grl_operation_cancel (guint operation_id);

gpointer grl_operation_get_data (guint operation_id);

void grl_operation_set_data (guint operation_id, gpointer user_data);

void grl_operation_set_data_full (guint operation_id, gpointer user_data, GDestroyNotify destroy_func);

G_END_DECLS

#endif /* _GRL_OPERATION_H_ */
