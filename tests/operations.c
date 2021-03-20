/*
 * Copyright (C) 2021 Grilo Project
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <glib.h>
#include <glib-object.h>

#include <grilo.h>


/* Simple iteration to see that max and mix set in GrlOperationOptions is being
 * respected. The values cannot be outside bounderies of what is registered for
 * that metadata-key, e.g: GRL_METADATA_KEY_ORIENTATION has min: 0, max: 359 so
 * negative numbers or >=360 are not allowed */
static void
range_filters_max_min_int(void)
{
  static struct {
    gint key_id;
    gint min_registered;
    gint max_registered;
    gint min_test;
    gint max_test;
    gboolean result_registered;
  } int_keys[] = {
    { GRL_METADATA_KEY_ORIENTATION, 0, 359, 90, 180, FALSE },
    { GRL_METADATA_KEY_ORIENTATION, 0, 359, -1, 999, TRUE }
  };

  gint i;
  for (i = 0; i < G_N_ELEMENTS (int_keys); i++) {
    GrlOperationOptions *options = grl_operation_options_new (NULL);
    gboolean ret;
    GValue *min = NULL;
    GValue *max = NULL;

    ret = grl_operation_options_set_key_range_filter (options,
                                                      GRL_METADATA_KEY_ORIENTATION,
                                                      int_keys[i].min_test,
                                                      int_keys[i].max_test,
                                                      NULL);
    g_assert_true(ret);
    grl_operation_options_get_key_range_filter (options, GRL_METADATA_KEY_ORIENTATION, &min, &max);

    if (int_keys[i].result_registered) {
      g_assert_cmpint(g_value_get_int (min), ==, int_keys[i].min_registered);
      g_assert_cmpint(g_value_get_int (max), ==, int_keys[i].max_registered);
    } else {
      g_assert_cmpint(g_value_get_int (min), ==, int_keys[i].min_test);
      g_assert_cmpint(g_value_get_int (max), ==, int_keys[i].max_test);
    }
  }
}

int
main (int argc, char **argv)
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);

  g_test_bug_base ("http://gitlab.gnome.org/GNOME/grilo/issues/%s");

  grl_init (&argc, &argv);

  /* registry tests */
  g_test_add_func ("/operation/range-filters/max-min/int", range_filters_max_min_int);

  return g_test_run ();
}
