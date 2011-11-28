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
 * @see_also: #GrlCaps, grl_metadata_source_resolve(),
 * grl_media_source_search(), grl_media_source_browse(),
 * grl_media_source_query()
 *
 */
#include <grl-operation-options.h>
#include <grl-value-helper.h>

#include "grl-operation-options-priv.h"

G_DEFINE_TYPE (GrlOperationOptions, grl_operation_options, G_TYPE_OBJECT);

#define GRL_OPERATION_OPTIONS_GET_PRIVATE(o)\
    (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRL_OPERATION_OPTIONS_TYPE, GrlOperationOptionsPrivate))

struct _GrlOperationOptionsPrivate {
  GHashTable *data;
  GrlCaps *caps;
};


#define SKIP_DEFAULT 0;
#define COUNT_DEFAULT GRL_COUNT_INFINITY;
#define FLAGS_DEFAULT GRL_RESOLVE_NORMAL;

static void
grl_operation_options_dispose (GrlOperationOptions *self)
{
}

static void
grl_operation_options_finalize (GrlOperationOptions *self)
{
  g_hash_table_unref (self->priv->data);
  if (self->priv->caps)
    g_object_unref (self->priv->caps);
  G_OBJECT_CLASS (grl_operation_options_parent_class)->finalize ((GObject *) self);
}

static void
grl_operation_options_init (GrlOperationOptions *self)
{
  self->priv = GRL_OPERATION_OPTIONS_GET_PRIVATE (self);

  self->priv->data = grl_g_value_hashtable_new ();
  self->priv->caps = NULL;
}

static void
grl_operation_options_class_init (GrlOperationOptionsClass *self_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (self_class);

  g_type_class_add_private (self_class, sizeof (GrlOperationOptionsPrivate));
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

/* ========== API ========== */

/**
 * grl_operation_options_new:
 * @caps: (allow-none): caps that options will have to match. If %NULL, all
 * options will be accepted.
 *
 * Creates a new GrlOperationOptions object.
 *
 * Returns: a new GrlOperationOptions instance.
 */
GrlOperationOptions *
grl_operation_options_new (GrlCaps *caps)
{
  GrlOperationOptions *options = g_object_new (GRL_OPERATION_OPTIONS_TYPE, NULL);
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
 */
gboolean
grl_operation_options_obey_caps (GrlOperationOptions *options,
                                 GrlCaps *caps,
                                 GrlOperationOptions **supported_options,
                                 GrlOperationOptions **unsupported_options)
{
  if (supported_options) {
    *supported_options = grl_operation_options_new (caps);

    /* these options are always supported */
    copy_option (options, *supported_options, GRL_OPERATION_OPTION_SKIP);
    copy_option (options, *supported_options, GRL_OPERATION_OPTION_COUNT);
    copy_option (options, *supported_options, GRL_OPERATION_OPTION_FLAGS);
  }

  if (unsupported_options)
    *unsupported_options = grl_operation_options_new (NULL);

  return TRUE;
}

/**
 * grl_operation_options_copy:
 * @options: a #GrlOperationOptions instance
 *
 * Returns: (transfer full): a new #GrlOperationOptions instance with its values being copies of
 * the values of @options.
 */
GrlOperationOptions *
grl_operation_options_copy (GrlOperationOptions *options)
{
  GrlOperationOptions *copy = grl_operation_options_new (options->priv->caps);

  copy_option (options, copy, GRL_OPERATION_OPTION_SKIP);
  copy_option (options, copy, GRL_OPERATION_OPTION_COUNT);
  copy_option (options, copy, GRL_OPERATION_OPTION_FLAGS);

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
 *
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
 * grl_operation_options_set_flags:
 * @options: a #GrlOperationOptions instance
 * @flags: the resolution flags to be set for an operation. See
 * #GrlMetadataResolutionFlags for possible values.
 *
 * Set the resolution flags for an operation. Will only succeed if @flags obey
 * to the inherent capabilities of @options.
 *
 * Returns: %TRUE if @flags could be set, %FALSE otherwise.
 *
 */
gboolean
grl_operation_options_set_flags (GrlOperationOptions *options,
                                 GrlMetadataResolutionFlags flags)
{
  GValue value = { 0, };

  /* FIXME: I think we should use mk_enum to have a GType for
   * GrlMetadataResolutionFlags */
  g_value_init (&value, G_TYPE_UINT);
  g_value_set_uint (&value, flags);
  set_value (options, GRL_OPERATION_OPTION_FLAGS, &value);
  g_value_unset (&value);

  return TRUE;
}

/**
 * grl_operation_options_get_flags:
 * @options: a #GrlOperationOptions instance
 *
 * Returns: resolution flags of @options.
 *
 */
GrlMetadataResolutionFlags
grl_operation_options_get_flags (GrlOperationOptions *options)
{
  const GValue *value  = g_hash_table_lookup (options->priv->data,
                                              GRL_OPERATION_OPTION_FLAGS);
  if (value)
    return g_value_get_uint (value);

  return FLAGS_DEFAULT;
}
