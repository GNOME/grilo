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

#ifndef _GRL_OPERATION_PRIV_H_
#define _GRL_OPERATION_PRIV_H_

#include <glib.h>

typedef void (*GrlOperationCancelCb) (gpointer data);

void grl_operation_init (void);

guint grl_operation_generate_id (void);

void grl_operation_set_private_data (guint                operation_id,
                                     gpointer             private_data,
                                     GrlOperationCancelCb cancel_cb,
                                     GDestroyNotify       destroy_cb);

gpointer grl_operation_get_private_data (guint operation_id);

void grl_operation_remove (guint operation_id);

#endif /* _GRL_OPERATION_PRIV_H_ */
