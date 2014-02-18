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

/*
 * This module provides helper functions to use GValues easily. Inspired by
 * libsoup's soup-value-utils:
 * http://git.gnome.org/browse/libsoup/tree/libsoup/soup-value-utils.c
 *
 */

#include <grl-value-helper.h>


GValue *
grl_g_value_new (GType g_type)
{
  GValue *value;

  value = g_slice_new0 (GValue);
  g_value_init (value, g_type);

  return value;
}

void
grl_g_value_free (GValue *value)
{
  g_value_unset (value);
  g_slice_free (GValue, value);
}

/**
 * grl_g_value_hashtable_new:
 *
 * Returns: (element-type utf8 GValue) (transfer full): a new hash table made to contain GValues.
 */
GHashTable *
grl_g_value_hashtable_new (void)
{
  return g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify)grl_g_value_free);
}

/**
 * grl_g_value_hashtable_new_direct:
 *
 * Returns: (element-type gpointer GValue) (transfer full): a new hash table made to contain GValues.
 */
GHashTable *
grl_g_value_hashtable_new_direct (void)
{
  return g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)grl_g_value_free);
}

/**
 * grl_g_value_dup:
 * @value: the #GValue to copy
 *
 * Returns: (transfer full): a duplicated #GValue
 */
GValue *
grl_g_value_dup (const GValue *value)
{
  GValue *new_value = grl_g_value_new (G_VALUE_TYPE (value));
  g_value_copy (value, new_value);

  return new_value;
}
