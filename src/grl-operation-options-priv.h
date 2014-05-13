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

#ifndef _GRL_OPERATION_OPTIONS_PRIV_H_
#define _GRL_OPERATION_OPTIONS_PRIV_H_

#include <grl-operation-options.h>

/* FIXME: Would probably be best to change these (and the APIs using them) into
 * GQuarks */
#define GRL_OPERATION_OPTION_SKIP "skip"
#define GRL_OPERATION_OPTION_COUNT "count"
#define GRL_OPERATION_OPTION_RESOLUTION_FLAGS "resolution-flags"
#define GRL_OPERATION_OPTION_TYPE_FILTER "type-filter"
#define GRL_OPERATION_OPTION_KEY_EQUAL_FILTER "key-equal-filter"
#define GRL_OPERATION_OPTION_KEY_RANGE_FILTER "key-range-filter"

gboolean grl_operation_options_key_is_set (GrlOperationOptions *options,
                                           const gchar *key);

#endif /* _GRL_OPERATION_OPTIONS_PRIV_H_ */
