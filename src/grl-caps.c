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
 *
 * A #GrlCaps instance is here to help you know if a given set of operation
 * options is supported for a given operation.
 *
 * Here is an example of how this would be used.
 * |[
 * GrlCaps *caps = grl_metadata_source_get_caps (GRL_METADATA_SOURCE (my_source),
                                                 GRL_OP_SEARCH);
   GrlOperationOptions *supported_options;
   if (grl_operation_options_obey_caps (my_options, caps, &supported_options, NULL))
     grl_media_source_search (my_source, "blah", interesting_keys, my_options, ...);
   else // only use a subset of the options we wanted to pass
     grl_media_source_search (my_source, "blah", interesting_keys, supported_options, ...);
 * ]|
 *
 * A #GrlCaps can also be passed to grl_operation_options_new(). The created
 * #GrlOperationOptions instance would then check any change against its caps.
 *
 * @see_also: #GrlOperationOptions, grl_metadata_source_get_caps()
 */
#include <grl-caps.h>
#include <grl-value-helper.h>

#include "grl-operation-options-priv.h"

#define GRL_CAPS_KEY_PAGINATION "pagination"
#define GRL_CAPS_KEY_FLAGS "flags"

G_DEFINE_TYPE (GrlCaps, grl_caps, G_TYPE_OBJECT);

#define GRL_CAPS_GET_PRIVATE(o)\
    (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRL_CAPS_TYPE, GrlCapsPrivate))

struct _GrlCapsPrivate {
  GHashTable *data;
};



static void
grl_caps_dispose (GrlCaps *self)
{
  G_OBJECT_CLASS (grl_caps_parent_class)->dispose ((GObject *) self);
}

static void
grl_caps_finalize (GrlCaps *self)
{
  g_hash_table_unref (self->priv->data);
  G_OBJECT_CLASS (grl_caps_parent_class)->finalize ((GObject *) self);
}

static void
grl_caps_init (GrlCaps *self)
{
  self->priv = GRL_CAPS_GET_PRIVATE (self);

  self->priv->data = grl_g_value_hashtable_new ();
}

static void
grl_caps_class_init (GrlCapsClass *self_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (self_class);

  g_type_class_add_private (self_class, sizeof (GrlCapsPrivate));
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
 **/
GrlCaps *
grl_caps_new (void)
{
  return g_object_new (GRL_CAPS_TYPE, NULL);
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
 */
gboolean
grl_caps_test_option (GrlCaps *caps, const gchar *key, const GValue *value)
{
  if (0 == g_strcmp0 (key, GRL_OPERATION_OPTION_SKIP)
      || 0 == g_strcmp0 (key, GRL_OPERATION_OPTION_COUNT)
      || 0 == g_strcmp0 (key, GRL_OPERATION_OPTION_FLAGS))
    /* these options must always be handled by plugins */
    return TRUE;

  return FALSE;
}

