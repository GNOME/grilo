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

#if !defined (_GRL_RANGE_VALUE_HELPER_H_)
#define _GRL_RANGE_VALUE_HELPER_H_

#include <glib-object.h>

typedef struct {
  GValue *min;
  GValue *max;
} GrlRangeValue;

G_BEGIN_DECLS

GrlRangeValue *grl_range_value_new (GValue *min,
                                    GValue *max);

void grl_range_value_free (GrlRangeValue *range);

GHashTable *grl_range_value_hashtable_new (void);

GrlRangeValue *grl_range_value_dup (const GrlRangeValue *range);

void grl_range_value_hashtable_insert (GHashTable *hash_table,
                                       gpointer key,
                                       GValue *min,
                                       GValue *max);

GType grl_range_value_get_type (void);

G_END_DECLS

#endif /* _GRL_RANGE_VALUE_HELPER_H_ */
