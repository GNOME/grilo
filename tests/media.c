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
#include <stdbool.h>
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

/* We are testing if metadata-keys were created correctly based on its GValue's type */
static void
test_set_for_id_new_key (Fixture *fixture, gconstpointer data)
{
  gboolean is_set_op = GPOINTER_TO_INT (data);

#define TEST_NEW(type, getter, value, st_set, st_add) {                             \
    GValue key_value = G_VALUE_INIT;                                                \
    gchar *name = g_strdup_printf ("grl-key-%s-%s-that-does-not-exist",             \
                                   g_type_name (type), is_set_op ? "set" : "add");  \
    GrlKeyID key_id = grl_registry_lookup_metadata_key (fixture->registry, name);   \
    g_assert_true (key_id == GRL_METADATA_KEY_INVALID);                             \
    g_value_init (&key_value, type);                                                \
    g_value_set_##getter (&key_value, value);                                       \
                                                                                    \
    GrlMedia * media = grl_media_new ();                                            \
    if (is_set_op) {                                                                \
      if (!st_set)                                                                  \
        g_test_expect_message ("Grilo", G_LOG_LEVEL_WARNING,                        \
                               "*is being ignored as*is not being handled*");       \
      g_assert_##st_set (grl_data_set_for_id (GRL_DATA (media), name, &key_value)); \
    } else {                                                                        \
      if (!st_add)                                                                  \
        g_test_expect_message ("Grilo", G_LOG_LEVEL_WARNING,                        \
                               "*is being ignored as*is not being handled*");       \
      g_assert_##st_add (grl_data_add_for_id (GRL_DATA (media), name, &key_value)); \
    }                                                                               \
    if (!st_set || !st_add)                                                         \
      g_test_assert_expected_messages ();                                           \
    g_value_unset (&key_value);                                                     \
                                                                                    \
    key_id = grl_registry_lookup_metadata_key (fixture->registry, name);            \
    GType key_type = grl_registry_lookup_metadata_key_type (fixture->registry,      \
                                                            key_id);                \
    /* Failure for set-for-id means failure for add-for-id                          \
     * without registering the metadata-key */                                      \
    if (!st_set) {                                                                  \
      g_assert_true (key_id == GRL_METADATA_KEY_INVALID);                           \
      g_assert_true (key_type == G_TYPE_INVALID);                                   \
    } else {                                                                        \
      g_assert_false (key_id == GRL_METADATA_KEY_INVALID);                          \
      g_assert_true (key_type == type);                                             \
    }                                                                               \
    g_object_unref (media);                                                         \
    g_free (name);                                                                  \
  }

  /* Works for both ADD and SET */
  TEST_NEW(G_TYPE_INT, int, G_MAXINT, true, true);
  TEST_NEW(G_TYPE_INT64, int64, G_MAXINT64, true, true);
  TEST_NEW(G_TYPE_FLOAT, float, 0.12345, true, true);
  TEST_NEW(G_TYPE_STRING, string, "data-of-key-that-does-not-exist", true, true);

  /* Works for SET and fails for ADD */
  TEST_NEW(G_TYPE_BOOLEAN, boolean, TRUE, true, false);

  GDateTime *t = g_date_time_new_now_local();
  TEST_NEW(G_TYPE_DATE_TIME, boxed, t, true, false);
  g_date_time_unref(t);

  /* Fails for both ADD and SET */
  TEST_NEW(G_TYPE_DOUBLE, double, 1.2345, false, false);

#undef TEST_NEW
}

static void
test_set_for_id_existing_key (Fixture *fixture, gconstpointer data)
{
  gboolean is_set_op = GPOINTER_TO_INT (data);

#define TEST_EXISTING(key_name, type, getter, value, st_set, st_add) {              \
    GValue key_value = G_VALUE_INIT;                                                \
    GrlKeyID key_id = grl_registry_lookup_metadata_key (fixture->registry,          \
                                                        key_name);                  \
    g_assert_true (key_id != GRL_METADATA_KEY_INVALID);                             \
                                                                                    \
    g_value_init (&key_value, type);                                                \
    g_value_set_##getter (&key_value, value);                                       \
    GrlMedia *media = grl_media_new ();                                             \
    if (is_set_op) {                                                                \
      if (!st_set)                                                                  \
        g_test_expect_message ("Grilo", G_LOG_LEVEL_WARNING,                        \
                               "*is being ignored as*is not being handled*");       \
      g_assert_##st_set (grl_data_set_for_id (GRL_DATA (media), key_name, &key_value)); \
    } else {                                                                        \
      if (!st_add)                                                                  \
        g_test_expect_message ("Grilo", G_LOG_LEVEL_WARNING,                        \
                               "*is being ignored as*is not being handled*");       \
      g_assert_##st_add (grl_data_add_for_id (GRL_DATA (media), key_name, &key_value)); \
    }                                                                               \
    if (!st_set || !st_add)                                                         \
      g_test_assert_expected_messages ();                                           \
                                                                                    \
    if ((is_set_op && st_set) || (!is_set_op && st_add)) {                          \
      gchar *mstr = g_strdup_value_contents (grl_data_get(GRL_DATA (media),         \
                                             key_id));                              \
      gchar *vstr = g_strdup_value_contents (&key_value);                           \
      g_assert_cmpstr (mstr, ==, vstr);                                             \
      g_free (mstr);                                                                \
      g_free (vstr);                                                                \
    }                                                                               \
    g_object_unref (media);                                                         \
  }

  /* Works for both ADD and SET */
  TEST_EXISTING("title", G_TYPE_STRING, string, "any-title", true, true);
  TEST_EXISTING("bitrate", G_TYPE_INT, int, G_MAXINT, true, true);
  TEST_EXISTING("size", G_TYPE_INT64, int64, G_MAXINT64, true, true);
  TEST_EXISTING("rating", G_TYPE_FLOAT, float, 0.12345, true, true);

  /* Works for SET and fails for ADD */
  TEST_EXISTING("favourite", G_TYPE_BOOLEAN, boolean, TRUE, true, false);

  GDateTime *t = g_date_time_new_now_local();
  TEST_EXISTING("creation-date", G_TYPE_DATE_TIME, boxed, t, true, false);
  g_date_time_unref(t);

#undef TEST_EXISTING
}

static void
test_set_for_id_different_key_type (Fixture *fixture, gconstpointer data)
{
  gboolean is_set_op = GPOINTER_TO_INT (data);

#define TEST_OTHER_GTYPE(key_name, value_type, getter, value, success) {              \
    GrlKeyID key_id = grl_registry_lookup_metadata_key (fixture->registry, key_name); \
    g_assert_true (key_id != GRL_METADATA_KEY_INVALID);                               \
    GType key_type = grl_registry_lookup_metadata_key_type (fixture->registry,        \
                                                            key_id);                  \
    g_assert_true (key_type != value_type);                                           \
                                                                                      \
    GValue key_value = G_VALUE_INIT;                                                  \
    g_value_init (&key_value, value_type);                                            \
    g_value_set_##getter (&key_value, value);                                         \
                                                                                      \
    GrlMedia *media = grl_media_new ();                                               \
    if (!success)                                                                     \
      g_test_expect_message ("Grilo", G_LOG_LEVEL_WARNING,                            \
                             "*Value type*can't be set to*of type*");                 \
    if (is_set_op) {                                                                  \
        g_assert_##success (grl_data_set_for_id (GRL_DATA (media), key_name, &key_value)); \
    } else {                                                                          \
        g_assert_##success (grl_data_set_for_id (GRL_DATA (media), key_name, &key_value)); \
    }                                                                                 \
    if (!success)                                                                     \
      g_test_assert_expected_messages ();                                             \
    g_object_unref (media);                                                           \
  }

  /* Works */

  /* duration is int */
  TEST_OTHER_GTYPE("duration", G_TYPE_INT64, int64, 302, true);
  TEST_OTHER_GTYPE("duration", G_TYPE_FLOAT, float, 303.1, true);
  TEST_OTHER_GTYPE("duration", G_TYPE_BOOLEAN, boolean, TRUE, true);
  /* title is string */
  TEST_OTHER_GTYPE("title", G_TYPE_INT64, int64, 666, true);
  TEST_OTHER_GTYPE("title", G_TYPE_FLOAT, float, 303.1, true);
  TEST_OTHER_GTYPE("title", G_TYPE_BOOLEAN, boolean, TRUE, true);
  /* rating is float */
  TEST_OTHER_GTYPE("rating", G_TYPE_INT64, int64, 302, true);
  TEST_OTHER_GTYPE("rating", G_TYPE_UINT64, uint64, G_MAXUINT64, true);

  /* Fails */

  TEST_OTHER_GTYPE("duration", G_TYPE_STRING, string, "301", false);
  TEST_OTHER_GTYPE("favourite", G_TYPE_STRING, string, "true", false);
  TEST_OTHER_GTYPE("favourite", G_TYPE_STRING, string, "FALSE", false);
  TEST_OTHER_GTYPE("favourite", G_TYPE_STRING, string, NULL, false);
  TEST_OTHER_GTYPE("rating", G_TYPE_BOOLEAN, boolean, TRUE, false);
  TEST_OTHER_GTYPE("rating", G_TYPE_STRING, string, "304.2", false);

#undef TEST_OTHER_GTYPE
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
