/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Juan A. Suarez Romero <jasuarez@igalia.com>
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
 * SECTION:grl-data
 * @short_description: Low-level class to store data
 * @see_also: #GrlMedia, #GrlDataBox, #GrlDataVideo, #GrlMediaAudio, #GrlDataImage
 *
 * This class acts as dictionary where keys and their values can be stored. It
 * is suggested to better high level classes, like #GrlMedia, which
 * provides functions to access known properties.
 */

#include "grl-data.h"

enum {
  PROP_0,
  PROP_OVERWRITE
};

struct _GrlDataPrivate {
  GHashTable *data;
  gboolean overwrite;
};

static void grl_data_set_property (GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec);

static void grl_data_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec);

static void grl_data_finalize (GObject *object);

#define GRL_DATA_GET_PRIVATE(o)                                         \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRL_TYPE_DATA, GrlDataPrivate))

G_DEFINE_TYPE (GrlData, grl_data, G_TYPE_OBJECT);

static void
free_val (GValue *val)
{
  if (val) {
    g_value_unset (val);
    g_free (val);
  }
}

static void
grl_data_class_init (GrlDataClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->set_property = grl_data_set_property;
  gobject_class->get_property = grl_data_get_property;
  gobject_class->finalize = grl_data_finalize;

  g_type_class_add_private (klass, sizeof (GrlDataPrivate));

  g_object_class_install_property (gobject_class,
                                   PROP_OVERWRITE,
                                   g_param_spec_boolean ("overwrite",
                                                         "Overwrite",
                                                         "Overwrite current values",
                                                         FALSE,
                                                         G_PARAM_READWRITE));
}

static void
grl_data_init (GrlData *self)
{
  self->priv = GRL_DATA_GET_PRIVATE (self);
  self->priv->data = g_hash_table_new_full (g_direct_hash,
                                            g_direct_equal,
                                            NULL,
                                            (GDestroyNotify) free_val);
}

static void
grl_data_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_data_parent_class)->finalize (object);
}

static void
grl_data_set_property (GObject *object,
                       guint prop_id,
                       const GValue *value,
                       GParamSpec *pspec)
{
  GrlData *self = GRL_DATA (object);

  switch (prop_id) {
  case PROP_OVERWRITE:
    self->priv->overwrite = g_value_get_boolean (value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
grl_data_get_property (GObject *object,
                       guint prop_id,
                       GValue *value,
                       GParamSpec *pspec)
{
  GrlData *self = GRL_DATA (object);

  switch (prop_id) {
  case PROP_OVERWRITE:
    g_value_set_boolean (value, self->priv->overwrite);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

/**
 * grl_data_new:
 *
 * Creates a new data object.
 *
 * Returns: a new data object.
 **/
GrlData *
grl_data_new (void)
{
  return g_object_new (GRL_TYPE_DATA,
		       NULL);
}

/**
 * grl_data_get:
 * @data: data to retrieve value
 * @key: key to look up.
 *
 * Get the value associated with the key. If it does not contain any value, NULL
 * will be returned.
 *
 * Returns: (transfer none) a #GValue. This value should not be modified nor freed by user.
 **/
const GValue *
grl_data_get (GrlData *data, GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_DATA (data), NULL);

  return g_hash_table_lookup (data->priv->data, GRLKEYID_TO_POINTER(key));
}

/**
 * grl_data_set:
 * @data: data to modify
 * @key: key to change or add
 * @value: the new value
 *
 * Sets the value associated with the key. If key already has a value and
 * #overwrite is TRUE, old value is freed and the new one is set.
 **/
void
grl_data_set (GrlData *data, GrlKeyID key, const GValue *value)
{
  GValue *copy = NULL;
  g_return_if_fail (GRL_IS_DATA (data));

  if (data->priv->overwrite ||
      g_hash_table_lookup (data->priv->data,
                           GRLKEYID_TO_POINTER (key)) == NULL) {
    /* Dup value */
    if (value) {
      copy = g_new0 (GValue, 1);
      g_value_init (copy, G_VALUE_TYPE (value));
      g_value_copy (value, copy);
    }

    g_hash_table_insert (data->priv->data, GRLKEYID_TO_POINTER(key), copy);
  }
}

/**
 * grl_data_set_string:
 * @data: data to modify
 * @key: key to change or add
 * @strvalue: the new value
 *
 * Sets the value associated with the key. If key already has a value and
 * #overwrite is TRUE, old value is freed and the new one is set.
 **/
void
grl_data_set_string (GrlData *data,
                     GrlKeyID key,
                     const gchar *strvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, strvalue);
  grl_data_set (data, key, &value);
  g_value_unset (&value);
}

/**
 * grl_data_get_string:
 * @data: data to inspect
 * @key: key to use
 *
 * Returns the value associated with the key. If key has no value, or value is
 * not string, or key is not in data, then NULL is returned.
 *
 * Returns: (transfer none): string associated with key, or NULL in other case. Caller should not change nor free the value.
 **/
const gchar *
grl_data_get_string (GrlData *data, GrlKeyID key)
{
  const GValue *value = grl_data_get (data, key);

  if (!value || !G_VALUE_HOLDS_STRING(value)) {
    return NULL;
  } else {
    return g_value_get_string (value);
  }
}

/**
 * grl_data_set_int:
 * @data: data to change
 * @key: key to change or addd
 * @intvalue: the new value
 *
 * Sets the value associated with the key. If key already has a value and
 * #overwrite is TRUE, old value is replaced by the new one.
 **/
void
grl_data_set_int (GrlData *data, GrlKeyID key, gint intvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, intvalue);
  grl_data_set (data, key, &value);
}

/**
 * grl_data_get_int:
 * @data: data to inspect
 * @key: key to use
 *
 * Returns the value associated with the key. If key has no value, or value is
 * not a gint, or key is not in data, then 0 is returned.
 *
 * Returns: int value associated with key, or 0 in other case.
 **/
gint
grl_data_get_int (GrlData *data, GrlKeyID key)
{
  const GValue *value = grl_data_get (data, key);

  if (!value || !G_VALUE_HOLDS_INT(value)) {
    return 0;
  } else {
    return g_value_get_int (value);
  }
}

/**
 * grl_data_set_float:
 * @data: data to change
 * @key: key to change or addd
 * @floatvalue: the new value
 *
 * Sets the value associated with the key. If key already has a value and
 * #overwrite is TRUE, old value is replaced by the new one.
 **/
void
grl_data_set_float (GrlData *data, GrlKeyID key, gint floatvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_FLOAT);
  g_value_set_float (&value, floatvalue);
  grl_data_set (data, key, &value);
}

/**
 * grl_data_get_float:
 * @data: data to inspect
 * @key: key to use
 *
 * Returns the value associated with the key. If key has no value, or value is
 * not a gfloat, or key is not in data, then 0 is returned.
 *
 * Returns: float value associated with key, or 0 in other case.
 **/
gfloat
grl_data_get_float (GrlData *data, GrlKeyID key)
{
  const GValue *value = grl_data_get (data, key);

  if (!value || !G_VALUE_HOLDS_FLOAT(value)) {
    return 0;
  } else {
    return g_value_get_float (value);
  }
}

/**
 * grl_data_add:
 * @data: data to change
 * @key: key to add
 *
 * Adds a new key to data, with no value. If key already exists, it does
 * nothing.
 **/
void
grl_data_add (GrlData *data, GrlKeyID key)
{
  if (!grl_data_has_key (data, key)) {
    grl_data_set (data, key, NULL);
  }
}

/**
 * grl_data_remove:
 * @data: data to change
 * @key: key to remove
 *
 * Removes key from data, freeing its value. If key is not in data, then
 * it does nothing.
 **/
void
grl_data_remove (GrlData *data, GrlKeyID key)
{
  g_return_if_fail (GRL_IS_DATA (data));

  g_hash_table_remove (data->priv->data, GRLKEYID_TO_POINTER(key));
}

/**
 * grl_data_has_key:
 * @data: data to inspect
 * @key: key to search
 *
 * Checks if key is in data.
 *
 * Returns: TRUE if key is in data, FALSE in other case.
 **/
gboolean
grl_data_has_key (GrlData *data, GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_DATA (data), FALSE);

  return g_hash_table_lookup_extended (data->priv->data,
                                       GRLKEYID_TO_POINTER(key), NULL, NULL);
}

/**
 * grl_data_get_keys:
 * @data: data to inspect
 *
 * Returns a list with keys contained in data.
 *
 * Returns: an array with the keys.
 **/
GList *
grl_data_get_keys (GrlData *data)
{
  GList *keylist;

  g_return_val_if_fail (GRL_IS_DATA (data), NULL);

  keylist = g_hash_table_get_keys (data->priv->data);

  return keylist;
}

/**
 * grl_data_key_is_known:
 * @data: data to inspect
 * @key: key to search
 *
 * Checks if the key has a value.
 *
 * Returns: TRUE if key has a value.
 **/
gboolean
grl_data_key_is_known (GrlData *data, GrlKeyID key)
{
  GValue *v;

  g_return_val_if_fail (GRL_IS_DATA (data), FALSE);

  v = g_hash_table_lookup (data->priv->data,
                           GRLKEYID_TO_POINTER(key));

  if (!v) {
    return FALSE;
  }

  if (G_VALUE_HOLDS_STRING (v)) {
    return g_value_get_string(v) != NULL;
  }

  return TRUE;
}

/**
 * grl_data_set_overwrite:
 * @data: data to change
 * @overwrite: if data can be overwritten
 *
 * This controls if #grl_data_set will overwrite current value of a property
 * with the new one.
 *
 * Set it to TRUE so old values are overwritten, or FALSE in other case (default
 * is FALSE).
 **/
void
grl_data_set_overwrite (GrlData *data, gboolean overwrite)
{
  g_return_if_fail (GRL_IS_DATA (data));

  if (data->priv->overwrite != overwrite) {
    data->priv->overwrite = overwrite;
    g_object_notify (G_OBJECT (data), "overwrite");
  }
}

/**
 * grl_data_get_overwrite:
 * @data: data to inspect
 *
 * Checks if old values are replaced when calling #grl_data_set.
 *
 * Returns: TRUE if values will be overwritten.
 **/
gboolean
grl_data_get_overwrite (GrlData *data)
{
  g_return_val_if_fail (GRL_IS_DATA (data), FALSE);

  return data->priv->overwrite;
}
