/*
 * Copyright (C) 2018 Grilo Project
 *
 * Author: Victor Toso <me@victortoso.com>
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

#include <grilo.h>

typedef struct {
  GrlRegistry *registry;
} Fixture;

static void
fixture_setup (Fixture *fixture, gconstpointer data)
{

  fixture->registry = grl_registry_get_default ();
}

static void
fixture_teardown (Fixture *fixture, gconstpointer data)
{
}

static void
test_set_for_id_new_key (Fixture *fixture, gconstpointer data)
{
  GrlMedia *media;
  GrlKeyID key_id;
  GValue key_value = { 0, };
  GType key_type;
  gchar *key_name;
  gboolean is_set_op = GPOINTER_TO_INT (data);
  const gchar *key_data_str = "data-of-key-that-does-not-exist";

  key_name = g_strdup_printf ("grl-key-for-%s-that-does-not-exist",
                              is_set_op ? "set" : "add");
  key_id = grl_registry_lookup_metadata_key (fixture->registry, key_name);
  g_assert_true (key_id == GRL_METADATA_KEY_INVALID);

  g_value_init (&key_value, G_TYPE_STRING);
  g_value_set_string (&key_value, key_data_str);

  media = grl_media_new ();
  if (is_set_op) {
    g_assert_true (grl_data_set_for_id (GRL_DATA (media), key_name, &key_value));
  } else {
    g_assert_true (grl_data_add_for_id (GRL_DATA (media), key_name, &key_value));
  }

  key_id = grl_registry_lookup_metadata_key (fixture->registry, key_name);
  g_assert_false (key_id == GRL_METADATA_KEY_INVALID);

  key_type = grl_registry_lookup_metadata_key_type (fixture->registry, key_id);
  g_assert_true (key_type == G_TYPE_STRING);
  g_object_unref (media);
  g_free (key_name);
}

static void
test_set_for_id_existing_key (Fixture *fixture, gconstpointer data)
{
  GrlMedia *media;
  GrlKeyID key_id;
  GValue key_value = { 0, };
  gboolean is_set_op = GPOINTER_TO_INT (data);
  const gchar *key_name = "title";
  const gchar *key_data_str = "data-of-existing-key";

  key_id = grl_registry_lookup_metadata_key (fixture->registry, key_name);
  g_assert_false (key_id == GRL_METADATA_KEY_INVALID);

  g_value_init (&key_value, G_TYPE_STRING);
  g_value_set_string (&key_value, key_data_str);

  media = grl_media_new ();
  if (is_set_op) {
    g_assert_true (grl_data_set_for_id (GRL_DATA (media), key_name, &key_value));
  } else {
    g_assert_true (grl_data_add_for_id (GRL_DATA (media), key_name, &key_value));
  }
  g_assert_cmpstr (grl_media_get_title(media), ==, key_data_str);
  g_object_unref (media);
}

static void
test_set_for_id_different_key_type (Fixture *fixture, gconstpointer data)
{
  GrlMedia *media;
  GrlKeyID key_id;
  GValue key_value = { 0, };
  GType key_type;
  gboolean is_set_op = GPOINTER_TO_INT (data);
  const gchar *key_name = "duration";
  const gchar *key_data_str = "301";

  key_id = grl_registry_lookup_metadata_key (fixture->registry, key_name);
  g_assert_false (key_id == GRL_METADATA_KEY_INVALID);
  key_type = grl_registry_lookup_metadata_key_type (fixture->registry, key_id);
  g_assert_true (key_type == G_TYPE_INT);

  /* String to Int should fail! */
  g_value_init (&key_value, G_TYPE_STRING);
  g_value_set_string (&key_value, key_data_str);

  media = grl_media_new ();
  g_test_expect_message ("Grilo", G_LOG_LEVEL_WARNING, "*value has type*but expected*");
  g_test_expect_message ("Grilo", G_LOG_LEVEL_WARNING, "*Trying to add an empty GrlRelatedKeys*");
  /* FIXME: Should return FALSE insted of TRUE as the @value is not set
   * on @media because the type is different.
   * It should be addressed in following up commits.
   */
  if (is_set_op) {
      g_assert_true (grl_data_set_for_id (GRL_DATA (media), key_name, &key_value));
  } else {
      g_assert_true (grl_data_add_for_id (GRL_DATA (media), key_name, &key_value));
  }
  g_test_assert_expected_messages ();
  g_object_unref (media);
}

int
main (int argc, char **argv)
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);

  grl_init (&argc, &argv);

  g_test_add ("/data/new/set-for-id",
              Fixture, GINT_TO_POINTER (TRUE),
              fixture_setup,
              test_set_for_id_new_key,
              fixture_teardown);

  g_test_add ("/data/new/add-for-id",
              Fixture, GINT_TO_POINTER (FALSE),
              fixture_setup,
              test_set_for_id_new_key,
              fixture_teardown);

  g_test_add ("/data/existing/set-for-id",
              Fixture, GINT_TO_POINTER (TRUE),
              fixture_setup,
              test_set_for_id_existing_key,
              fixture_teardown);

  g_test_add ("/data/existing/add-for-id",
              Fixture, GINT_TO_POINTER (FALSE),
              fixture_setup,
              test_set_for_id_existing_key,
              fixture_teardown);

  g_test_add ("/data/different-key-type/set-for-id",
              Fixture, GINT_TO_POINTER (TRUE),
              fixture_setup,
              test_set_for_id_different_key_type,
              fixture_teardown);

  g_test_add ("/data/different-key-type/add-for-id",
              Fixture, GINT_TO_POINTER (FALSE),
              fixture_setup,
              test_set_for_id_different_key_type,
              fixture_teardown);

  return g_test_run ();
}
