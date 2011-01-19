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

#ifndef _GRL_DEFINITIONS_H_
#define _GRL_DEFINITIONS_H_

/* Used to pad public structres in order to avoid ABI
   breakage when adding new API */
#define GRL_PADDING 16

/* Same as GRL_PADDING, but intended for objects that could
   have more impact on the memory footprint */
#define GRL_PADDING_SMALL 8

/* Value used when childcount is unknown */
#define GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN -1

/* Used to know if plugin can not tell how many elements
   remain to be sent */
#define GRL_SOURCE_REMAINING_UNKNOWN -1

#endif
