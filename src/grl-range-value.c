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
 * This module provides helper functions to use Range Values easily.
 * Inspired by libsoup's soup-value-utils:
 * http://git.gnome.org/browse/libsoup/tree/libsoup/soup-value-utils.c
 *
 */

#include <grl-range-value.h>
#include <grl-value-helper.h>

G_DEFINE_BOXED_TYPE (GrlRangeValue, grl_range_value,
                     (GBoxedCopyFunc) grl_range_value_dup,
                     (GBoxedFreeFunc) grl_range_value_free)


GrlRangeValue *
grl_range_value_new (GValue *min, GValue *max)
{
  GrlRangeValue *range;

  range = g_slice_new0 (GrlRangeValue);
  if (min) {
    range->min = grl_g_value_dup (min);
  }

  if (max) {
    range->max = grl_g_value_dup (max);
  }

  return range;
}

void
grl_range_value_free (GrlRangeValue *range)
{
  g_clear_pointer (&range->min, grl_g_value_free);
  g_clear_pointer (&range->max, grl_g_value_free);

  g_slice_free (GrlRangeValue, range);
}

/**
 * grl_range_value_hashtable_new:
 *
 * Returns: (transfer full) (element-type gpointer GrlRangeValue): a #GHashTable
 */
GHashTable *
grl_range_value_hashtable_new (void)
{
  return g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)grl_range_value_free);
}

GrlRangeValue *
grl_range_value_dup (const GrlRangeValue *range)
{
  return grl_range_value_new (range->min, range->max);
}

void
grl_range_value_hashtable_insert (GHashTable *hash_table,
                                  gpointer key,
                                  GValue *min,
                                  GValue *max)
{
  GrlRangeValue *range = grl_range_value_new (min, max);
  g_hash_table_insert (hash_table, key, range);
}
