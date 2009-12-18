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

#include "ms-metadata-key.h"

#include <stdarg.h>

GList *
ms_metadata_key_list_new (MsKeyID first_key, ...)
{
  GList *keylist = NULL;
  MsKeyID key;
  va_list vakeys;

  key = first_key;
  va_start (vakeys, first_key);
  while (key) {
    keylist = g_list_prepend (keylist, GUINT_TO_POINTER (key));
    key = va_arg (vakeys, MsKeyID);
  }
  va_end (vakeys);

  return g_list_reverse (keylist);
}
