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
 * SECTION:grl-caps
 * @short_description: Describes the capabilities of a source for a given
 * operation.
 * @see_also: #GrlOperationOptions, grl_source_get_caps()
 *
 * A #GrlCaps instance is here to help you know if a given set of operation
 * options is supported for a given operation.
 *
 * Here is an example of how this would be used.
 * |[
 * GrlCaps *caps = grl_source_get_caps (GRL_SOURCE (my_source),
 *                                      GRL_OP_SEARCH);
 * GrlOperationOptions *supported_options;
 * if (grl_operation_options_obey_caps (my_options, caps, &supported_options, NULL))
 *   grl_media_source_search (my_source, "blah", interesting_keys, my_options, ...);
 * else // only use a subset of the options we wanted to pass
 *   grl_media_source_search (my_source, "blah", interesting_keys, supported_options, ...);
 * ]|
 *
 * A #GrlCaps can also be passed to grl_operation_options_new(). The created
 * #GrlOperationOptions instance would then check any change against its caps.
 *
 */
#include <grl-caps.h>
#include <grl-value-helper.h>

#include "grl-operation-options-priv.h"
#include "grl-type-builtins.h"

#define GRL_CAPS_KEY_PAGINATION "pagination"
#define GRL_CAPS_KEY_FLAGS "flags"

struct _GrlCapsPrivate {
  GHashTable *data;
  GrlTypeFilter type_filter;
  GList *key_filter;
  GList *key_range_filter;
};

G_DEFINE_TYPE_WITH_PRIVATE (GrlCaps, grl_caps, G_TYPE_OBJECT);

static void
grl_caps_dispose (GrlCaps *self)
{
  G_OBJECT_CLASS (grl_caps_parent_class)->dispose ((GObject *) self);
}

static void
grl_caps_finalize (GrlCaps *self)
{
  g_hash_table_unref (self->priv->data);
  g_list_free (self->priv->key_filter);
  g_list_free (self->priv->key_range_filter);

  G_OBJECT_CLASS (grl_caps_parent_class)->finalize ((GObject *) self);
}

static void
grl_caps_init (GrlCaps *self)
{
  self->priv = grl_caps_get_instance_private (self);

  self->priv->data = grl_g_value_hashtable_new ();

  /* by default, type filtering is not considered to be supported. The source
   * has to explicitly modify its caps. */
  self->priv->type_filter = GRL_TYPE_FILTER_NONE;
  self->priv->key_filter = NULL;
  self->priv->key_range_filter = NULL;
}

static void
grl_caps_class_init (GrlCapsClass *self_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (self_class);

  object_class->dispose = (void (*) (GObject *object)) grl_caps_dispose;
  object_class->finalize = (void (*) (GObject *object)) grl_caps_finalize;
}

/* ========== API ========== */

/**
 * grl_caps_new:
 *
 * Creates a new caps object.
 *
 * Returns: a new caps object.
 *
 * Since: 0.2.0
 **/
GrlCaps *
grl_caps_new (void)
{
  return g_object_new (GRL_TYPE_CAPS, NULL);
}

/**
 * grl_caps_test_option:
 * @caps: a #GrlCaps instance
 * @key: a key to test
 * @value: the value corresponding to @key to test against @caps
 *
 * Checks whether (@key, @value) are authorized by @caps.
 *
 * Returns: %TRUE if (@key, @value) obey to @caps, %FALSE otherwise.
 *
 * Since: 0.2.0
 */
gboolean
grl_caps_test_option (GrlCaps *caps, const gchar *key, const GValue *value)
{
  if (0 == g_strcmp0 (key, GRL_OPERATION_OPTION_SKIP)
      || 0 == g_strcmp0 (key, GRL_OPERATION_OPTION_COUNT)
      || 0 == g_strcmp0 (key, GRL_OPERATION_OPTION_RESOLUTION_FLAGS))
    /* these options must always be handled by plugins */
    return TRUE;

  if (0 == g_strcmp0 (key, GRL_OPERATION_OPTION_TYPE_FILTER)) {
    GrlTypeFilter filter, supported_filter;

    supported_filter = grl_caps_get_type_filter (caps);
    filter = g_value_get_flags (value);

    return filter == (filter & supported_filter);
  }

  if (0 == g_strcmp0 (key, GRL_OPERATION_OPTION_KEY_EQUAL_FILTER)) {
    GrlKeyID metadata_key = g_value_get_grl_key_id (value);
    return grl_caps_is_key_filter (caps, metadata_key);
  }

  if (0 == g_strcmp0 (key, GRL_OPERATION_OPTION_KEY_RANGE_FILTER)) {
    GrlKeyID grl_key = g_value_get_grl_key_id (value);
    return grl_caps_is_key_range_filter (caps, grl_key);
  }

  return FALSE;
}

/**
 * grl_caps_get_type_filter:
 * @caps: a #GrlCaps instance
 *
 * Returns: the supported #GrlTypeFilter
 *
 * Since: 0.2.0
 **/
GrlTypeFilter
grl_caps_get_type_filter (GrlCaps *caps)
{
  g_return_val_if_fail (caps != NULL, GRL_TYPE_FILTER_NONE);

  return caps->priv->type_filter;
}

/**
 * grl_caps_set_type_filter:
 * @caps: a #GrlCaps instance
 * @filter: a #GrlTypeFilter
 *
 * Sets the supported filter capability.
 *
 * Since: 0.2.0
 **/
void
grl_caps_set_type_filter (GrlCaps *caps, GrlTypeFilter filter)
{
  g_return_if_fail (caps != NULL);

  caps->priv->type_filter = filter;
}

/**
 * grl_caps_get_key_filter:
 * @caps: a #GrlCaps instance
 *
 * Returns: (transfer none) (element-type GrlKeyID):
 *
 * Since: 0.2.0
 */
GList *
grl_caps_get_key_filter (GrlCaps *caps)
{
  g_return_val_if_fail (caps, NULL);

  return caps->priv->key_filter;
}

/**
 * grl_caps_set_key_filter:
 * @caps: a #GrlCaps instance
 * @keys: (transfer none) (element-type GrlKeyID):
 *
 * Since: 0.2.0
 */
void
grl_caps_set_key_filter (GrlCaps *caps, GList *keys)
{
  g_return_if_fail (caps);

  g_clear_pointer (&caps->priv->key_filter, g_list_free);

  caps->priv->key_filter = g_list_copy (keys);
}

/**
 * grl_caps_is_key_filter:
 * @caps: a #GrlCaps instance
 * @key: a #GrlKeyID
 *
 * Checks if @key is supported for filtering in @caps.
 *
 * Returns: %TRUE if @key can be used for filtering
 *
 * Since: 0.2.0
 **/
gboolean
grl_caps_is_key_filter (GrlCaps *caps, GrlKeyID key)
{
  g_return_val_if_fail (caps, FALSE);

  if(caps->priv->key_filter) {
    return g_list_find (caps->priv->key_filter,
                        GRLKEYID_TO_POINTER(key)) != NULL;
  }

  return FALSE;
}

/**
 * grl_caps_get_key_range_filter:
 * @caps: a #GrlCaps instance
 *
 * Returns: (transfer none) (element-type GrlKeyID):
 *
 * Since: 0.2.0
 */
GList *
grl_caps_get_key_range_filter (GrlCaps *caps)
{
  g_return_val_if_fail (caps, NULL);

  return caps->priv->key_range_filter;
}

/**
 * grl_caps_set_key_range_filter:
 * @caps: a #GrlCaps instance
 * @keys: (transfer none) (element-type GrlKeyID):
 *
 * Since: 0.2.0
 */
void
grl_caps_set_key_range_filter (GrlCaps *caps, GList *keys)
{
  g_return_if_fail (caps);

  g_clear_pointer (&caps->priv->key_range_filter, g_list_free);

  caps->priv->key_range_filter = g_list_copy (keys);
}

/**
 * grl_caps_is_key_range_filter:
 * @caps: a #grlCaps instance
 * @key: a #GrlKeyID
 *
 * Checks if @key is supported for filtering by range in @caps.
 *
 * Returns: %TRUE if @key can be used for filtering
 *
 * Since: 0.2.0
 **/
gboolean
grl_caps_is_key_range_filter (GrlCaps *caps, GrlKeyID key)
{
  g_return_val_if_fail (caps, FALSE);

  if(caps->priv->key_range_filter) {
    return g_list_find (caps->priv->key_range_filter, GRLKEYID_TO_POINTER (key)) != NULL;
  }

  return FALSE;
}
