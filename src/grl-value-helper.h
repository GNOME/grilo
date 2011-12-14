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

#if !defined (_GRL_VALUE_HELPER_H_)
#define _GRL_VALUE_HELPER_H_

#include <glib-object.h>

G_BEGIN_DECLS

GValue *grl_g_value_new (GType g_type);

void grl_g_value_free (GValue *value);

GHashTable *grl_g_value_hashtable_new (void);

GHashTable *grl_g_value_hashtable_new_direct (void);

GValue *grl_g_value_dup (const GValue *value);

G_END_DECLS

#endif /* _GRL_VALUE_HELPER_H_ */
