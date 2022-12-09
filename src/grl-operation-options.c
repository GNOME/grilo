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

/**
 * SECTION:grl-operation-options
 * @short_description: Describes the options to be passed to an operation
 * @see_also: #GrlCaps, grl_source_resolve(),
 * grl_source_search(), grl_source_browse(),
 * grl_source_query()
 *
 */
#include <grl-operation-options.h>
#include <grl-value-helper.h>
#include <grl-range-value.h>
#include <grl-log.h>
#include <grl-registry-priv.h>

#include "grl-operation-options-priv.h"
#include "grl-type-builtins.h"

struct _GrlOperationOptionsPrivate {
  GHashTable *data;
  GHashTable *key_filter;
  GHashTable *key_range_filter;
  GrlCaps *caps;
};

G_DEFINE_TYPE_WITH_PRIVATE (GrlOperationOptions, grl_operation_options, G_TYPE_OBJECT);

#define SKIP_DEFAULT 0;
#define COUNT_DEFAULT GRL_COUNT_INFINITY;
#define RESOLUTION_FLAGS_DEFAULT GRL_RESOLVE_NORMAL;
#define TYPE_FILTER_DEFAULT GRL_TYPE_FILTER_ALL;

static void
grl_operation_options_dispose (GrlOperationOptions *self)
{
}

static void
grl_operation_options_finalize (GrlOperationOptions *self)
{
  g_hash_table_unref (self->priv->data);
  g_hash_table_unref (self->priv->key_filter);
  g_hash_table_unref (self->priv->key_range_filter);
  g_clear_object (&self->priv->caps);
  G_OBJECT_CLASS (grl_operation_options_parent_class)->finalize ((GObject *) self);
}

static void
grl_operation_options_init (GrlOperationOptions *self)
{
  self->priv = grl_operation_options_get_instance_private (self);

  self->priv->data = grl_g_value_hashtable_new ();
  self->priv->key_filter = grl_g_value_hashtable_new_direct ();
  self->priv->key_range_filter = grl_range_value_hashtable_new ();
  self->priv->caps = NULL;
}

static void
grl_operation_options_class_init (GrlOperationOptionsClass *self_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (self_class);

  object_class->dispose = (void (*) (GObject *object)) grl_operation_options_dispose;
  object_class->finalize = (void (*) (GObject *object)) grl_operation_options_finalize;
}

static void
set_value (GrlOperationOptions *options,
                const gchar *key,
                const GValue *value)
{
  g_hash_table_insert (options->priv->data,
                       g_strdup (key), grl_g_value_dup (value));
}

static void
copy_option (GrlOperationOptions *source,
             GrlOperationOptions *destination,
             const gchar *key)
{
  const GValue *value;

  value = g_hash_table_lookup (source->priv->data, key);

  if (value != NULL)
    set_value (destination, key, value);
}

static gboolean
check_and_copy_option (GrlOperationOptions *options,
                       GrlCaps *caps,
                       const gchar *key,
                       GrlOperationOptions **supported_options,
                       GrlOperationOptions **unsupported_options)
{
  if (grl_operation_options_key_is_set (options, key)) {
    GValue *value;
    gboolean filter_is_supported;

    value = g_hash_table_lookup (options->priv->data, key);
    filter_is_supported = grl_caps_test_option (caps, key, value);

    if (filter_is_supported && supported_options)
      set_value (*supported_options, key, value);
    else if (!filter_is_supported && unsupported_options)
      set_value (*unsupported_options, key, value);

    return filter_is_supported;
  }

  return TRUE;
}

static void
key_filter_dup (gpointer key_p, GValue *value, GHashTable *destination)
{
  g_hash_table_insert (destination, key_p, grl_g_value_dup (value));
}

static void
key_range_filter_dup (GrlKeyID key, GrlRangeValue *value, GHashTable *destination)
{
  grl_range_value_hashtable_insert (destination, GRLKEYID_TO_POINTER (key), value->min, value->max);
}

/* ========== API ========== */

/**
 * grl_operation_options_new:
 * @caps: (allow-none): caps that options will have to match. If %NULL, all
 * options will be accepted.
 *
 * Creates a new GrlOperationOptions object.
 *
 * Returns: a new GrlOperationOptions instance.
 *
 * Since: 0.2.0
 */
GrlOperationOptions *
grl_operation_options_new (GrlCaps *caps)
{
  GrlOperationOptions *options = g_object_new (GRL_TYPE_OPERATION_OPTIONS, NULL);
  if (caps != NULL)
    options->priv->caps = g_object_ref (caps);

  return options;
}

/**
 * grl_operation_options_obey_caps:
 * @options: a #GrlOperationOptions instance
 * @caps: capabilities against which we want to test @options
 * @supported_options: (out callee-allocates): if not %NULL, will contain a
 * newly-allocated #GrlOperationOptions instance containing all the values of
 * @options that match @caps.
 * @unsupported_options: (out callee-allocates): if not %NULL, will contain a
 * newly-allocated #GrlOperationOptions instance containing all the values of
 * @options that do not match @caps.
 *
 * Check whether @options obey to @caps.
 * Optionally provide the options that match (respectively don't match) @caps
 * in @supported_options (respectively @unsupported_options).
 * This would typically (but not necessarily) be used with a
 * #GrlOperationOptions instance that was created with %NULL caps.
 *
 * Returns: %TRUE if @options obey to @caps, %FALSE otherwise.
 *
 * Since: 0.2.0
 */
gboolean
grl_operation_options_obey_caps (GrlOperationOptions *options,
                                 GrlCaps *caps,
                                 GrlOperationOptions **supported_options,
                                 GrlOperationOptions **unsupported_options)
{
  gboolean ret = TRUE;
  GHashTableIter table_iter;
  gpointer key_ptr;
  GValue *value;
  GrlRangeValue *range_value;

  if (supported_options) {
    *supported_options = grl_operation_options_new (caps);

    /* these options are always supported */
    copy_option (options, *supported_options, GRL_OPERATION_OPTION_SKIP);
    copy_option (options, *supported_options, GRL_OPERATION_OPTION_COUNT);
    copy_option (options, *supported_options, GRL_OPERATION_OPTION_RESOLUTION_FLAGS);
  }

  if (unsupported_options)
    *unsupported_options = grl_operation_options_new (NULL);

  ret &= check_and_copy_option (options,
                                caps,
                                GRL_OPERATION_OPTION_TYPE_FILTER,
                                supported_options,
                                unsupported_options);

  /* Check filter-by-equal-key */
  g_hash_table_iter_init (&table_iter, options->priv->key_filter);
  while (g_hash_table_iter_next (&table_iter, &key_ptr, (gpointer *)&value)) {
    GrlKeyID key_id = GRLPOINTER_TO_KEYID (key_ptr);
    if (grl_caps_is_key_filter (caps, key_id)) {
      if (supported_options) {
        g_hash_table_insert ((*supported_options)->priv->key_filter,
                             key_ptr,
                             grl_g_value_dup (value));
      }
    } else {
      ret = FALSE;
      if (unsupported_options) {
        g_hash_table_insert ((*unsupported_options)->priv->key_filter,
                             key_ptr,
                             grl_g_value_dup (value));
      }
    }
  }

  /* Check filter-by-range-key */
  g_hash_table_iter_init (&table_iter, options->priv->key_range_filter);
  while (g_hash_table_iter_next (&table_iter, &key_ptr, (gpointer *)&range_value)) {
    GrlKeyID key_id = GRLPOINTER_TO_KEYID (key_ptr);
    if (grl_caps_is_key_range_filter (caps, key_id)) {
      if (supported_options) {
        g_hash_table_insert ((*supported_options)->priv->key_range_filter,
                             key_ptr,
                             grl_range_value_dup (range_value));
      }
    } else {
      ret = FALSE;
      if (unsupported_options) {
        g_hash_table_insert ((*unsupported_options)->priv->key_range_filter,
                             key_ptr,
                             grl_range_value_dup (range_value));
      }
    }
  }

  return ret;
}

/**
 * grl_operation_options_copy:
 * @options: a #GrlOperationOptions instance
 *
 * Returns: (transfer full): a new #GrlOperationOptions instance with its values being copies of
 * the values of @options.
 *
 * Since: 0.2.0
 */
GrlOperationOptions *
grl_operation_options_copy (GrlOperationOptions *options)
{
  GrlOperationOptions *copy = grl_operation_options_new (options->priv->caps);

  copy_option (options, copy, GRL_OPERATION_OPTION_SKIP);
  copy_option (options, copy, GRL_OPERATION_OPTION_COUNT);
  copy_option (options, copy, GRL_OPERATION_OPTION_RESOLUTION_FLAGS);
  copy_option (options, copy, GRL_OPERATION_OPTION_TYPE_FILTER);

  g_hash_table_foreach (options->priv->key_filter,
                        (GHFunc) key_filter_dup,
                        copy->priv->key_filter);

  g_hash_table_foreach (options->priv->key_range_filter,
                        (GHFunc) key_range_filter_dup,
                        copy->priv->key_range_filter);

  return copy;
}

/**
 * grl_operation_options_key_is_set:
 * @options: a #GrlOperationOptions instance
 * @key: an operation option key
 *
 * This is an internal method that shouldn't be used outside of Grilo.
 *
 * Returns: whether @key is set in @options.
 *
 * Since: 0.2.0
 */
gboolean
grl_operation_options_key_is_set (GrlOperationOptions *options,
                                  const gchar *key)
{
  return g_hash_table_lookup_extended (options->priv->data, key, NULL, NULL);
}

/**
 * grl_operation_options_set_skip:
 * @options: a #GrlOperationOptions instance
 * @skip: number of elements to skip in an operation
 *
 * Set the skip option for an operation. Will only succeed if @skip obeys to the
 * inherent capabilities of @options.
 *
 * Returns: %TRUE if @skip could be set, %FALSE otherwise.
 *
 * Since: 0.2.0
 */
gboolean
grl_operation_options_set_skip (GrlOperationOptions *options,
                                guint skip)
{
  GValue skip_val = { 0, };

  g_value_init (&skip_val, G_TYPE_UINT);
  g_value_set_uint (&skip_val, skip);

  set_value (options, GRL_OPERATION_OPTION_SKIP, &skip_val);

  g_value_unset (&skip_val);

  return TRUE;
}

/**
 * grl_operation_options_get_skip:
 * @options: a #GrlOperationOptions instance
 *
 * Get the skip option, that is, the number of elements to skip before
 * retrieving media items in an operation done with @options.
 *
 * Returns: the value of the skip option, or a default value if it is not set.
 *
 * Since: 0.2.0
 */
guint
grl_operation_options_get_skip (GrlOperationOptions *options)
{
  const GValue *value  =
      g_hash_table_lookup (options->priv->data, GRL_OPERATION_OPTION_SKIP);

  if (value)
    return g_value_get_uint (value);

  return SKIP_DEFAULT;
}

/**
 * grl_operation_options_set_count:
 * @options: a #GrlOperationOptions instance
 * @count: number of elements to retrieve in an operation
 *
 * Set the count option for an operation. Will only succeed if @count obey to
 * the inherent capabilities of @options.
 *
 * Returns: %TRUE if @count could be set, %FALSE otherwise.
 *
 * Since: 0.2.0
 */
gboolean
grl_operation_options_set_count (GrlOperationOptions *options, gint count)
{
  GValue count_val = { 0, };

  g_value_init (&count_val, G_TYPE_INT);
  g_value_set_int (&count_val, count);

  set_value (options, GRL_OPERATION_OPTION_COUNT, &count_val);

  g_value_unset (&count_val);

  return TRUE;
}

/**
 * grl_operation_options_get_count:
 * @options: a #GrlOperationOptions instance
 *
 * Get the count option, that is, the number of elements to retrieve in an
 * operation done with @options.
 *
 * Returns: the value of the count option, or a default value if it is not set.
 *
 * Since: 0.2.0
 */
gint
grl_operation_options_get_count (GrlOperationOptions *options)
{
  const GValue *value  =
      g_hash_table_lookup (options->priv->data, GRL_OPERATION_OPTION_COUNT);

  if (value)
    return g_value_get_int (value);

  return COUNT_DEFAULT;
}

/**
 * grl_operation_options_set_resolution_flags:
 * @options: a #GrlOperationOptions instance
 * @flags: the resolution flags to be set for an operation. See
 * #GrlResolutionFlags for possible values.
 *
 * Set the resolution flags for an operation. Will only succeed if @flags obey
 * to the inherent capabilities of @options.
 *
 * Returns: %TRUE if @flags could be set, %FALSE otherwise.
 *
 * Since: 0.2.12
 */
gboolean
grl_operation_options_set_resolution_flags (GrlOperationOptions *options,
                                            GrlResolutionFlags flags)
{
  GValue value = { 0, };

  /* FIXME: I think we should use mk_enum to have a GType for
   * GrlResolutionFlags */
  g_value_init (&value, G_TYPE_UINT);
  g_value_set_uint (&value, flags);
  set_value (options, GRL_OPERATION_OPTION_RESOLUTION_FLAGS, &value);
  g_value_unset (&value);

  return TRUE;
}

/**
 * grl_operation_options_get_resolution_flags:
 * @options: a #GrlOperationOptions instance
 *
 * Returns: resolution flags of @options.
 *
 * Since: 0.2.12
 */
GrlResolutionFlags
grl_operation_options_get_resolution_flags (GrlOperationOptions *options)
{
  const GValue *value;

  if (options)
    value = g_hash_table_lookup (options->priv->data,
                                 GRL_OPERATION_OPTION_RESOLUTION_FLAGS);
  else
    value = NULL;

  if (value)
    return g_value_get_uint (value);

  return RESOLUTION_FLAGS_DEFAULT;
}

/**
 * grl_operation_options_set_type_filter:
 * @options: a #GrlOperationOptions instance
 * @filter: the type of media to get
 *
 * Set the type of media filter for an operation. Only those media elements that
 * match the @filter will be returned. Will only succeed if @filter obey to the
 * inherent capabilities of @options.
 *
 * Returns: %TRUE if @flags could be set, %FALSE otherwise
 *
 * Since: 0.2.0
 **/
gboolean
grl_operation_options_set_type_filter (GrlOperationOptions *options,
                                       GrlTypeFilter filter)
{
  GValue value = { 0 };
  gboolean ret;

  g_value_init (&value, GRL_TYPE_TYPE_FILTER);
  g_value_set_flags (&value, filter);

  ret = (options->priv->caps == NULL) ||
      grl_caps_test_option (options->priv->caps,
                            GRL_OPERATION_OPTION_TYPE_FILTER, &value);

  if (ret)
    set_value (options, GRL_OPERATION_OPTION_TYPE_FILTER, &value);

  g_value_unset (&value);

  return ret;
}

/**
 * grl_operation_options_get_type_filter:
 * @options: a #GrlOperationOptions instance
 *
 *
 * Returns: resolution flags of @options
 *
 * Since: 0.2.0
 **/
GrlTypeFilter
grl_operation_options_get_type_filter (GrlOperationOptions *options)
{
  const GValue *value = g_hash_table_lookup (options->priv->data,
                                             GRL_OPERATION_OPTION_TYPE_FILTER);

  if (value) {
    return g_value_get_flags (value);
  }

  return TYPE_FILTER_DEFAULT;
}

/**
 * grl_operation_options_set_key_filter_value:
 * @options: a #GrlOperationOptions instance
 * @key: a #GrlKeyID
 * @value: a #GValue
 *
 * Set filter as "@key == @value".
 *
 * Returns: %TRUE on success
 *
 * Since: 0.2.0
 **/
gboolean
grl_operation_options_set_key_filter_value (GrlOperationOptions *options,
                                            GrlKeyID key,
                                            GValue *value)
{
  gboolean ret;
  GrlRegistry *registry;
  GType key_type;

  registry = grl_registry_get_default ();
  key_type = grl_registry_lookup_metadata_key_type (registry, key);

  if (G_VALUE_TYPE (value) != key_type)
    return FALSE;

  ret = (options->priv->caps == NULL) ||
    grl_caps_is_key_filter (options->priv->caps, key);

  if (ret) {
    if (value) {
      g_hash_table_insert (options->priv->key_filter,
                           GRLKEYID_TO_POINTER (key),
                           grl_g_value_dup (value));
    } else {
      g_hash_table_remove (options->priv->key_filter,
                           GRLKEYID_TO_POINTER (key));
    }
  }

  return ret;
}

/**
 * grl_operation_options_set_key_filters:
 * @options: a #GrlOperationOptions instance
 * @...: pairs of #GrlKeyID, value
 *
 * Set filter as "k1 == v1 AND k2 == v2 AND ..."
 *
 * <example>
 *  Elements from album "Frozen" with a bitrate of 256kbs.
 *  <programlisting>
 *   grl_operation_options_set_key_filters (my_options,
 *                                          GRL_METADATA_KEY_ALBUM, "Frozen",
 *                                          GRL_METADATA_KEY_BITRATE, 256,
 *                                          NULL);
 *  </programlisting>
 * </example>
 *
 * Returns: %TRUE on success
 *
 * Since: 0.2.0
 **/
gboolean
grl_operation_options_set_key_filters (GrlOperationOptions *options,
                                       ...)
{
  GType key_type;
  GValue value = { 0 };
  GrlKeyID next_key;
  gboolean skip;
  gboolean success = TRUE;
  va_list args;

  va_start (args, options);
  next_key = va_arg (args, GrlKeyID);
  while (next_key) {
    key_type = GRL_METADATA_KEY_GET_TYPE (next_key);
    g_value_init (&value, key_type);
    skip = FALSE;
    if (key_type == G_TYPE_STRING) {
      g_value_set_string (&value, va_arg (args, gchar *));
    } else if (key_type == G_TYPE_INT) {
      g_value_set_int (&value, va_arg (args, gint));
    } else if (key_type == G_TYPE_BOOLEAN) {
      g_value_set_boolean (&value, va_arg (args, gboolean));
    } else if (key_type == G_TYPE_DATE_TIME) {
      g_value_set_boxed (&value, va_arg (args, gconstpointer));
    } else {
      GRL_WARNING ("Unexpected key type when setting up the filter");
      success = FALSE;
      skip = TRUE;
    }

    if (!skip) {
      success &= grl_operation_options_set_key_filter_value (options,
                                                             next_key,
                                                             &value);
    }

    g_value_unset (&value);
    next_key = va_arg (args, GrlKeyID);
  }

  va_end (args);

  return success;
}

/**
 * grl_operation_options_set_key_filter_dictionary: (rename-to grl_operation_options_set_key_filters)
 * @options: a #GrlOperationOptions instance
 * @filters: (transfer none) (element-type GrlKeyID GValue):
 *
 * Returns: %TRUE on success
 *
 * Since: 0.2.0
 */
gboolean
grl_operation_options_set_key_filter_dictionary (GrlOperationOptions *options,
                                                 GHashTable *filters)
{
  GHashTableIter iter;
  gpointer _key, _value;
  gboolean ret = TRUE;

  g_hash_table_iter_init (&iter, filters);
  while (g_hash_table_iter_next (&iter, &_key, &_value)) {
    GrlKeyID key = GRLPOINTER_TO_KEYID (_key);
    GValue *value = (GValue *)_value;
    ret &=
        grl_operation_options_set_key_filter_value (options, key, value);
  }

  return ret;
}

/**
 * grl_operation_options_get_key_filter:
 * @options: a #GrlOperationOptions instance
 * @key:
 *
 * Returns: (transfer none): the filter
 *
 * Since: 0.2.0
 */
GValue *
grl_operation_options_get_key_filter (GrlOperationOptions *options,
                                      GrlKeyID key)
{
  return g_hash_table_lookup (options->priv->key_filter,
                              GRLKEYID_TO_POINTER (key));
}

/**
 * grl_operation_options_get_key_filter_list:
 * @options: a #GrlOperationOptions instance
 *
 * Returns: (transfer container) (element-type GrlKeyID):
 *
 * Since: 0.2.0
 */
GList *
grl_operation_options_get_key_filter_list (GrlOperationOptions *options)
{
  return g_hash_table_get_keys (options->priv->key_filter);
}

/**
 * grl_operation_options_set_key_range_filter_value:
 * @options: a #GrlOperationOptions instance
 * @key: a #GrlKeyID
 * @min_value: (in) (allow-none): minimum value for range
 * @max_value: (in) (allow-none): maximum value for range
 *
 * Set filter as "@min_value <= @key <= @max_value".
 *
 * If @min_value is %NULL, then filter is "@key <= @max_value".
 *
 * If @max_value is %NULL, then filter is "@key >= @min_value".
 *
 * Note that @key will always respect the limits defined at creation time.
 * e.g: Core's GRL_METADATA_KEY_ORIENTATION has "max = 359" and "min = 0",
 * user can set "@max_value = 180" but "@max_value = 720" would be ignored.
 *
 * Returns: %TRUE on success
 *
 * Since: 0.2.0
 **/
gboolean
grl_operation_options_set_key_range_filter_value (GrlOperationOptions *options,
                                                  GrlKeyID key,
                                                  GValue *min_value,
                                                  GValue *max_value)
{
  gboolean ret;
  GrlRegistry *registry;
  GValue min_registered = G_VALUE_INIT;
  GValue max_registered = G_VALUE_INIT;                                                \
  gboolean min_changed = FALSE;
  gboolean max_changed = FALSE;

  ret = (options->priv->caps == NULL) ||
    grl_caps_is_key_range_filter (options->priv->caps, key);

  if (!ret) {
      return FALSE;
  }

  if (!min_value && !max_value) {
    g_hash_table_remove (options->priv->key_range_filter,
                         GRLKEYID_TO_POINTER (key));
    return TRUE;
  }

  /* Does a CLAMP for GValue of numeric types so new min and max values are
   * within min-max values as registered per metadata-key */
  registry = grl_registry_get_default ();

  ret = grl_registry_metadata_key_is_max_valid(registry, key, min_value, max_value);
  if (!ret) {
    return FALSE;
  }

  if (grl_registry_metadata_key_get_limits(registry, key, &min_registered, &max_registered)) {
    max_changed = grl_registry_metadata_key_clamp(registry, key, &min_registered, max_value, &max_registered);
    min_changed = grl_registry_metadata_key_clamp(registry, key, &min_registered, min_value, &max_registered);
  } else {
    GRL_DEBUG("Can't get limits of this key");
  }

  if (min_changed || max_changed) {
    GRL_DEBUG("@min_value=%c @max_value=%c changes due metadata-key limits",
              min_changed ? 'y':'n', max_changed ? 'y':'n');
  }

  grl_range_value_hashtable_insert (options->priv->key_range_filter,
                                    GRLKEYID_TO_POINTER (key),
                                    min_value, max_value);
  return TRUE;
}

/**
 * grl_operation_options_set_key_range_filter:
 * @options: a #GrlOperationOptions instance
 * @...: triplets of #GrlKeyID, minvalue, maxvalue
 *
 * Set filter as "min1 <= k1 <= max1 AND min2 <= k2 <= max2 AND ..."
 *
 * For non numeric types, the range can be open if some of the minX, maxX
 * values are %NULL. Leaving NULL for numeric types leads to underifned
 * behavior.

 * <example>
 *  Album must start with "T" and the bitrate should be 256kbs or greater.
 *  <programlisting>
 *   grl_operation_options_set_key_range_filters (my_options,
 *                                                GRL_METADATA_KEY_ALBUM, "Ta", "Tz",
 *                                                GRL_METADATA_KEY_BITRATE, 256, G_MAXINT,
 *                                                NULL);
 *  </programlisting>
 * </example>
 *
 * Returns: %TRUE on success
 *
 * Since: 0.2.0
 **/
gboolean
grl_operation_options_set_key_range_filter (GrlOperationOptions *options,
                                            ...)
{
  GType key_type;
  GValue min_value = { 0 };
  GValue *min_p_value;
  gint min_int_value;
  gchar *min_str_value;
  gfloat min_float_value;
  gconstpointer min_date_value;
  GValue max_value = { 0 };
  GValue *max_p_value;
  gint max_int_value;
  gchar *max_str_value;
  gfloat max_float_value;
  gconstpointer max_date_value;
  GrlKeyID next_key;
  gboolean skip;
  gboolean success = TRUE;
  va_list args;

  va_start (args, options);
  next_key = va_arg (args, GrlKeyID);
  while (next_key) {
    key_type = GRL_METADATA_KEY_GET_TYPE (next_key);
    g_value_init (&min_value, key_type);
    g_value_init (&max_value, key_type);
    min_p_value = NULL;
    max_p_value = NULL;
    skip = FALSE;
    if (key_type == G_TYPE_STRING) {
      min_str_value = va_arg (args, gchar *);
      max_str_value = va_arg (args, gchar *);
      if (min_str_value) {
        g_value_set_string (&min_value, min_str_value);
        min_p_value = &min_value;
      }
      if (max_str_value) {
        g_value_set_string (&max_value, max_str_value);
        max_p_value = &max_value;
      }
    } else if (key_type == G_TYPE_INT) {
      min_int_value = va_arg (args, gint);
      max_int_value = va_arg (args, gint);
      if (min_int_value > G_MININT) {
        g_value_set_int (&min_value, min_int_value);
        min_p_value = &min_value;
      }
      if (max_int_value < G_MAXINT) {
        g_value_set_int (&max_value, max_int_value);
        max_p_value = &max_value;
      }
    } else if (key_type == G_TYPE_FLOAT) {
      min_float_value = va_arg (args, gdouble);
      max_float_value = va_arg (args, gdouble);
      if (min_float_value > G_MINFLOAT) {
        g_value_set_float (&min_value, min_float_value);
        min_p_value = &min_value;
      }
      if (max_float_value < G_MAXFLOAT) {
        g_value_set_float (&max_value, max_float_value);
        max_p_value = &max_value;
      }
    } else if (key_type == G_TYPE_DATE_TIME) {
      min_date_value = va_arg (args, gconstpointer);
      max_date_value = va_arg (args, gconstpointer);
      if (min_date_value) {
        g_value_set_boxed (&min_value, min_date_value);
        min_p_value = &min_value;
      }
      if (max_date_value) {
        g_value_set_boxed (&max_value, max_date_value);
        max_p_value = &max_value;
      }
    } else {
      GRL_WARNING ("Unexpected key type when setting up the filter");
      success = FALSE;
      skip = TRUE;
    }

    if (!skip) {
      success &= grl_operation_options_set_key_range_filter_value (options,
                                                                   next_key,
                                                                   min_p_value,
                                                                   max_p_value);
    }

    g_value_unset (&min_value);
    g_value_unset (&max_value);
    next_key = va_arg (args, GrlKeyID);
  }

  va_end (args);

  return success;
}

/**
 * grl_operation_options_get_key_range_filter:
 * @options: a #GrlOperationOptions instance
 * @key: a #GrlKeyID
 * @min_value: (out) (allow-none) (transfer none): the minimum value for the range
 * @max_value: (out) (allow-none) (transfer none): the maximum value for the range
 *
 * Stores the limits of the range in the filter for @key in @min_value and
 * @max_value. If some of the values has no limit, it will set a %NULL.
 *
 * Since: 0.2.0
 **/
void
grl_operation_options_get_key_range_filter (GrlOperationOptions *options,
                                            GrlKeyID key,
                                            GValue **min_value,
                                            GValue **max_value)
{
  GrlRangeValue *range =
    (GrlRangeValue *) g_hash_table_lookup (options->priv->key_range_filter,
                                           GRLKEYID_TO_POINTER (key));

  if (min_value) {
    if (range && range->min) {
      *min_value = range->min;
    } else {
      *min_value = NULL;
    }
  }

  if (max_value) {
    if (range && range->max) {
      *max_value = range->max;
    } else {
      *max_value = NULL;
    }
  }
}

/**
 * grl_operation_options_get_key_range_filter_list:
 * @options: a #GrlOperationOptions instance
 *
 * Returns: (transfer container) (element-type GrlKeyID):
 *
 * Since: 0.2.0
 */
GList *
grl_operation_options_get_key_range_filter_list (GrlOperationOptions *options)
{
  return g_hash_table_get_keys (options->priv->key_range_filter);
}
