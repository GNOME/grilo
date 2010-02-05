/*
 * Copyright (C) 2010 Igalia S.L.
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

#ifndef _MS_ERROR_H_
#define _MS_ERROR_H_

#define MS_ERROR g_quark_from_static_string("media-store.error.general")

enum {
  MS_ERROR_BROWSE_FAILED = 1,
  MS_ERROR_SEARCH_FAILED,
  MS_ERROR_QUERY_FAILED,
  MS_ERROR_METADATA_FAILED,
  MS_ERROR_RESOLVE_FAILED,
  MS_ERROR_MEDIA_NOT_FOUND,
  MS_ERROR_STORE_FAILED,
  MS_ERROR_REMOVE_FAILED,
};

#endif
