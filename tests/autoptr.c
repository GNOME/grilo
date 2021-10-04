/*
 * Copyright (C) 2018 Yi-Soo An <yisooan@gmail.com>
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
 * This code is based on glib/tests/autoptr.c
 */

#include "config.h"
#include <glib.h>

#include <grilo.h>
#ifdef HAVE_GRLNET
#include <net/grl-net.h>

static void
test_grl_net_wc (void)
{
  g_autoptr (GrlNetWc) val = grl_net_wc_new ();
  g_assert_nonnull (val);
}
#endif /* HAVE_GRLNET */

static void
test_grl_data (void)
{
  g_autoptr (GrlData) val = grl_data_new ();
  g_assert_nonnull (val);
}

static void
test_grl_related_keys (void)
{
  g_autoptr (GrlRelatedKeys) val = grl_related_keys_new ();
  g_assert_nonnull (val);
}

static void
test_grl_media (void)
{
  g_autoptr (GrlMedia) val = grl_media_new ();
  g_assert_nonnull (val);
}

static void
test_grl_caps (void)
{
  g_autoptr (GrlCaps) val = grl_caps_new ();
  g_assert_nonnull (val);
}

static void
test_grl_operation_options (void)
{
  g_autoptr (GrlOperationOptions) val = grl_operation_options_new (NULL);
  g_assert_nonnull (val);
}

static void
test_grl_plugin (void)
{
  g_autoptr (GrlPlugin) val = NULL;

  val = GRL_PLUGIN (g_object_new (GRL_TYPE_PLUGIN, NULL));

  g_assert_nonnull (val);
}

int
main (int    argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  grl_init (&argc, &argv);

#ifdef HAVE_GRLNET
  g_test_add_func ("/autoptr/grl_net_wc", test_grl_net_wc);
#endif

  g_test_add_func ("/autoptr/grl_data", test_grl_data);
  g_test_add_func ("/autoptr/grl_media", test_grl_media);
  g_test_add_func ("/autoptr/grl_related_keys", test_grl_related_keys);

  g_test_add_func ("/autoptr/grl_caps", test_grl_caps);
  g_test_add_func ("/autoptr/grl_operation_options", test_grl_operation_options);
  g_test_add_func ("/autoptr/grl_plugin", test_grl_plugin);

  return g_test_run ();
}
