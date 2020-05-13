/*
 * Copyright (C) 2011 Igalia S.L.
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
 * SECTION:grl-related-keys
 * @short_description: A class where to store related metadata keys.
 * @see_also: #GrlRegistry, #GrlData
 *
 * When handling media keys, like artist, URI, mime-type, and so on, some of
 * these keys are somewhat related: they do not make sense if they are not
 * accompanied by other keys.
 *
 * For instance, media URI and and mime-type are related keys: mime-type does
 * not make sense if it is not accompanied by an URI. Moreover, for each URI
 * value, there is a corresponding mime-type value.
 *
 * #GrlRelatedKeys stores related keys and their values in one place, so user
 * can handle them in one shot.
 */

#include "grl-related-keys.h"
#include "grl-log.h"
#include "grl-registry-priv.h"

struct _GrlRelatedKeysPrivate {
  GHashTable *data;
};

static void grl_related_keys_finalize (GObject *object);
static void free_value (GValue *val);

/* ================ GrlRelatedKeys GObject ================ */

G_DEFINE_TYPE_WITH_PRIVATE (GrlRelatedKeys, grl_related_keys, G_TYPE_OBJECT);

static void
grl_related_keys_class_init (GrlRelatedKeysClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->finalize = grl_related_keys_finalize;
}

static void
grl_related_keys_init (GrlRelatedKeys *self)
{
  self->priv = grl_related_keys_get_instance_private (self);
  self->priv->data = g_hash_table_new_full (g_direct_hash,
                                            g_direct_equal,
                                            NULL,
                                            (GDestroyNotify) free_value);
}

static void
grl_related_keys_finalize (GObject *object)
{
  g_hash_table_unref (GRL_RELATED_KEYS (object)->priv->data);
  G_OBJECT_CLASS (grl_related_keys_parent_class)->finalize (object);
}

/* ================ Utitilies ================ */

static void
free_value (GValue *val)
{
  if (val) {
    g_value_unset (val);
    g_free (val);
  }
}

/* ================ API ================ */

/**
 * grl_related_keys_new:
 *
 * Creates a new #GrlRelatedKeys instance that can be used to store related
 * keys and their values.
 *
 * Returns: a new object.
 *
 * Since: 0.1.10
 **/
GrlRelatedKeys *
grl_related_keys_new (void)
{
  return g_object_new (GRL_TYPE_RELATED_KEYS, NULL);
}

/**
 * grl_related_keys_new_valist:
 * @key: first key
 * @args: #va_list of value, followed by (key,value) pairs to insert
 *
 * Creates a new #GrlRelatedKeys containing pairs of (key, value). Finish the
 * list with %NULL.
 *
 * In case of a binary-type key, the expected element is (key, value, size).
 *
 * value type will be extracted from key information.
 *
 * Returns: a new #GrlRelatedKeys
 *
 * Since: 0.1.10
 **/
GrlRelatedKeys *
grl_related_keys_new_valist (GrlKeyID key,
                             va_list args)
{
  GType key_type;
  GrlKeyID next_key;
  GrlRelatedKeys *prop;
  gpointer next_value;

  prop = grl_related_keys_new ();

  next_key = key;
  while (next_key) {
    key_type = GRL_METADATA_KEY_GET_TYPE (next_key);
    if (key_type == G_TYPE_STRING) {
      grl_related_keys_set_string (prop, next_key, va_arg (args, gchar *));
    } else if (key_type == G_TYPE_INT) {
      grl_related_keys_set_int (prop, next_key, va_arg (args, gint));
    } else if (key_type == G_TYPE_FLOAT) {
      grl_related_keys_set_float (prop, next_key, va_arg (args, double));
    } else if (key_type == G_TYPE_BOOLEAN) {
      grl_related_keys_set_boolean (prop, next_key, va_arg (args, gboolean));
    } else if (key_type == G_TYPE_BYTE_ARRAY) {
      next_value = va_arg (args, gpointer);
      grl_related_keys_set_binary (prop,
                                   next_key,
                                   next_value,
                                   va_arg (args, gsize));
    } else {
      GRL_WARNING ("related key type '%s' not handled",
                   g_type_name (key_type));
    }
    next_key = va_arg (args, GrlKeyID);
  }

  return prop;
}

/**
 * grl_related_keys_new_with_keys: (skip)
 * @key: first key
 * @...: value, following by list of (key, value)
 *
 * Creates a initial #GrlRelatedKeys containing the list of (key, value)
 * pairs. Finish the list with %NULL.
 *
 * For more information see #grl_related_keys_new_valist.
 *
 * Returns: a new #GrlRelatedKeys
 *
 * Since: 0.1.10
 **/
GrlRelatedKeys *
grl_related_keys_new_with_keys (GrlKeyID key,
                                ...)
{
  GrlRelatedKeys *prop;
  va_list args;

  va_start (args, key);
  prop = grl_related_keys_new_valist (key, args);
  va_end (args);

  return prop;
}


/**
 * grl_related_keys_get:
 * @relkeys: set of related keys to retrieve value
 * @key: (type GrlKeyID): key to look up.
 *
 * Get the value associated with @key from @relkeys. If it does not contain any
 * value, %NULL will be returned.
 *
 * Returns: (transfer none): a #GValue. This value should not be modified nor
 * freed by user.
 *
 * Since: 0.1.10
 **/
const GValue *
grl_related_keys_get (GrlRelatedKeys *relkeys,
                      GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), NULL);
  g_return_val_if_fail (key, NULL);

  return g_hash_table_lookup (relkeys->priv->data, GRLKEYID_TO_POINTER (key));
}

/**
 * grl_related_keys_set:
 * @relkeys: set of related keys to modify
 * @key: (type GrlKeyID): key to change or add
 * @value: the new value
 *
 * Sets the value associated with @key into @relkeys. Old value is freed and
 * the new one is set.
 *
 * Also, checks that @value is compliant with @key specification, modifying it
 * accordingly. For instance, if @key requires a number between 0 and 10, but
 * value is outside this range, it will be adapted accordingly.
 *
 * Since: 0.1.10
 **/
void
grl_related_keys_set (GrlRelatedKeys *relkeys,
                      GrlKeyID key,
                      const GValue *value)
{
  GValue *copy = NULL;
  GrlRegistry *registry;
  GType key_type, value_type;

  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));
  g_return_if_fail (key);

  if (!value) {
    return;
  }

  key_type = GRL_METADATA_KEY_GET_TYPE (key);
  value_type = G_VALUE_TYPE (value);

  if (!g_value_type_transformable (value_type, key_type)) {
    GRL_WARNING ("value has type %s, but expected %s",
                 g_type_name (value_type),
                 g_type_name (key_type));
    return;
  }

  /* Dup value */
  copy = g_new0 (GValue, 1);
  g_value_init (copy, key_type);
  if (!g_value_transform (value, copy)) {
    GRL_WARNING ("transforming value type %s to key's type %s failed",
                 g_type_name (value_type),
                 g_type_name (key_type));
    g_free (copy);
    return;
  }

  registry = grl_registry_get_default ();

  if (!grl_registry_metadata_key_validate (registry, key, copy)) {
    GRL_WARNING ("'%s' value invalid, adjusting",
                 GRL_METADATA_KEY_GET_NAME (key));
  }
  g_hash_table_insert (relkeys->priv->data, GRLKEYID_TO_POINTER (key), copy);
}

/**
 * grl_related_keys_set_string:
 * @relkeys: set of related keys to modify
 * @key: (type GrlKeyID): key to change or add
 * @strvalue: the new value
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as a strying-type key. Old value is freed and the new one is set.
 *
 * Since: 0.1.10
 **/
void
grl_related_keys_set_string (GrlRelatedKeys *relkeys,
                             GrlKeyID key,
                             const gchar *strvalue)
{
  GValue value = { 0 };

  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));

  if (strvalue) {
    g_value_init (&value, G_TYPE_STRING);
    g_value_set_string (&value, strvalue);
    grl_related_keys_set (relkeys, key, &value);
    g_value_unset (&value);
  }
}

/**
 * grl_related_keys_get_string:
 * @relkeys: set of related keys to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * or value is not string, or @key is not in @relkeys, then %NULL is returned.
 *
 * Returns: string associated with @key, or %NULL in other case. Caller should
 * not change nor free the value.
 *
 * Since: 0.1.10
 **/
const gchar *
grl_related_keys_get_string (GrlRelatedKeys *relkeys,
                             GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), NULL);

  value = grl_related_keys_get (relkeys, key);

  if (!value || !G_VALUE_HOLDS_STRING (value)) {
    return NULL;
  } else {
    return g_value_get_string (value);
  }
}

/**
 * grl_related_keys_set_int:
 * @relkeys: set of related keys to change
 * @key: (type GrlKeyID): key to change or add
 * @intvalue: the new value
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as an int-type key. Old value is replaced by the new one.
 *
 * Since: 0.1.10
 **/
void
grl_related_keys_set_int (GrlRelatedKeys *relkeys,
                          GrlKeyID key,
                          gint intvalue)
{
  GValue value = { 0 };
  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));
  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, intvalue);
  grl_related_keys_set (relkeys, key, &value);
}

/**
 * grl_related_keys_get_int:
 * @relkeys: set of related keys to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * or value is not a gint, or @key is not in @relkeys, then 0 is returned.
 *
 * Returns: int value associated with @key, or 0 in other case.
 *
 * Since: 0.1.10
 **/
gint
grl_related_keys_get_int (GrlRelatedKeys *relkeys,
                          GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), 0);

  value = grl_related_keys_get (relkeys, key);

  if (!value || !G_VALUE_HOLDS_INT (value)) {
    return 0;
  } else {
    return g_value_get_int (value);
  }
}

/**
 * grl_related_keys_set_float:
 * @relkeys: set of related keys to change
 * @key: (type GrlKeyID): key to change or add
 * @floatvalue: the new value
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as a float-type key. Old value is replaced by the new one.
 *
 * Since: 0.1.10
 **/
void
grl_related_keys_set_float (GrlRelatedKeys *relkeys,
                            GrlKeyID key,
                            float floatvalue)
{
  GValue value = { 0 };
  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));
  g_value_init (&value, G_TYPE_FLOAT);
  g_value_set_float (&value, floatvalue);
  grl_related_keys_set (relkeys, key, &value);
}

/**
 * grl_related_keys_get_float:
 * @relkeys: set of related keys to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * or value is not a gfloat, or @key is not in @relkeys, then 0 is returned.
 *
 * Returns: float value associated with @key, or 0 in other case.
 *
 * Since: 0.1.10
 **/
gfloat
grl_related_keys_get_float (GrlRelatedKeys *relkeys,
                            GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), 0.0);

  value = grl_related_keys_get (relkeys, key);

  if (!value || !G_VALUE_HOLDS_FLOAT (value)) {
    return 0;
  } else {
    return g_value_get_float (value);
  }
}

/**
 * grl_related_keys_set_boolean:
 * @relkeys: set of related keys to change
 * @key: (type GrlKeyID): key to change or add
 * @booleanvalue: the new value
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as a boolean-type key. Old value is replaced by the new one.
 *
 * Since: 0.2.3
 **/
void
grl_related_keys_set_boolean (GrlRelatedKeys *relkeys,
                              GrlKeyID key,
                              gboolean booleanvalue)
{
  GValue value = { 0 };
  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));
  g_value_init (&value, G_TYPE_BOOLEAN);
  g_value_set_boolean (&value, booleanvalue);
  grl_related_keys_set (relkeys, key, &value);
}

/**
 * grl_related_keys_get_boolean:
 * @relkeys: set of related keys to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * or value is not a gboolean, or @key is not in @relkeys, then %FALSE is
 * returned.
 *
 * Returns: float value associated with @key, or %FALSE in other case.
 *
 * Since: 0.2.3
 **/
gboolean
grl_related_keys_get_boolean (GrlRelatedKeys *relkeys,
                              GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), FALSE);

  value = grl_related_keys_get (relkeys, key);

  if (!value || !G_VALUE_HOLDS_BOOLEAN (value)) {
    return FALSE;
  } else {
    return g_value_get_boolean (value);
  }
}

/**
 * grl_related_keys_set_binary:
 * @relkeys: set of related keys to change
 * @key: (type GrlKeyID): key to change or add
 * @buf: buffer holding the relkeys
 * @size: size of the buffer
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as a binary-type key. Old value is replaced by the new one.
 *
 * Since: 0.1.10
 **/
void
grl_related_keys_set_binary (GrlRelatedKeys *relkeys,
                             GrlKeyID key,
                             const guint8 *buf,
                             gsize size)
{
  GValue v = { 0 };
  GByteArray *array;

  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));

  if (!buf || !size) {
    return;
  }

  array = g_byte_array_append (g_byte_array_sized_new(size),
                               buf,
                               size);

  g_value_init (&v, g_byte_array_get_type ());
  g_value_take_boxed (&v, array);
  grl_related_keys_set (relkeys, key, &v);
  g_value_unset (&v);
}

/**
 * grl_related_keys_get_binary:
 * @relkeys: set of related keys to inspect
 * @key: (type GrlKeyID): key to use
 * @size: (out): location to store the buffer size
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * or value is not a binary, or @key is not in @relkeys, then 0 is returned.
 *
 * Returns: buffer location associated with @key, or %NULL in other case. If
 * successful @size will be set to the buffer size.
 *
 * Since: 0.1.10
 **/
const guint8 *
grl_related_keys_get_binary (GrlRelatedKeys *relkeys,
                             GrlKeyID key,
                             gsize *size)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), NULL);
  g_return_val_if_fail (size, NULL);

  value = grl_related_keys_get (relkeys, key);


  if (!value || !G_VALUE_HOLDS_BOXED (value)) {
    return NULL;
  } else {
    GByteArray * array;

    array = g_value_get_boxed (value);
    *size = array->len;
    return (const guint8 *) array->data;
  }
}

/**
 * grl_related_keys_set_boxed:
 * @relkeys: set of related keys to modify
 * @key: key to change or add
 * @boxed: the new value
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as a boxed-type key. Old value is freed and the new one is set.
 *
 * Since: 0.2.0
 */
void
grl_related_keys_set_boxed (GrlRelatedKeys *relkeys,
                            GrlKeyID key,
                            gconstpointer boxed)
{
  GValue value = { 0 };

  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));
  g_return_if_fail (boxed != NULL);

  g_value_init (&value, grl_metadata_key_get_type (key));
  g_value_set_boxed (&value, boxed);
  grl_related_keys_set (relkeys, key, &value);
  g_value_unset (&value);
}

/**
 * grl_related_keys_get_boxed:
 * @relkeys: set of related keys to inspect
 * @key: key to use
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * the value is not of a boxed type, or @key is not in @relkeys, then %NULL is
 * returned.
 *
 * Returns: (transfer none): the #GBoxed value associated with @key if
 * possible, or %NULL in other case. The caller should not change nor free the
 * value.
 *
 * Since: 0.2.0
 */
gconstpointer
grl_related_keys_get_boxed (GrlRelatedKeys *relkeys,
                            GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), NULL);

  value = grl_related_keys_get (relkeys, key);

  if (!value || !G_VALUE_HOLDS_BOXED (value)) {
    return NULL;
  } else {
    return g_value_get_boxed (value);
  }
}

/**
 * grl_related_keys_set_int64:
 * @relkeys: set of related keys to change
 * @key: (type GrlKeyID): key to change or add
 * @intvalue: the new value
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as a int64-type key. Old value is replaced by the new one.
 *
 * Since: 0.2.12
 **/
void
grl_related_keys_set_int64 (GrlRelatedKeys *relkeys,
                            GrlKeyID key,
                            gint64 intvalue)
{
  GValue value = { 0 };
  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));
  g_value_init (&value, G_TYPE_INT64);
  g_value_set_int64 (&value, intvalue);
  grl_related_keys_set (relkeys, key, &value);
}

/**
 * grl_related_keys_get_int64:
 * @relkeys: set of related keys to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * or value is not a gint64, or @key is not in @relkeys, then 0 is returned.
 *
 * Returns: int64 value associated with @key, or 0 in other case.
 *
 * Since: 0.2.12
 **/
gint64
grl_related_keys_get_int64 (GrlRelatedKeys *relkeys,
                            GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), 0);

  value = grl_related_keys_get (relkeys, key);

  if (!value || !G_VALUE_HOLDS_INT64 (value)) {
    return 0;
  } else {
    return g_value_get_int64 (value);
  }
}

/**
 * grl_related_keys_set_for_id:
 * @relkeys: set of related keys to modify
 * @key_name: name of the key to change or add
 * @value: the new value
 *
 * Sets the value associated with @key_name in @relkeys. This @key_name is used to create
 * a new #GParamSpec instance, which is further used to create and register a key using
 * grl_registry_register_metadata_key(). If @key_name already has a @value, old value
 * is replaced by the new one.
 *
 * A property key_name consists of segments consisting of ASCII letters and
 * digits, separated by either the '-' or '_' character. The first
 * character of a property key_name must be a letter. Key_names which violate these
 * rules lead to undefined behaviour.
 *
 * Returns: TRUE if @value was set to @key_name, FALSE otherwise.
 *
 * Since: 0.3.13
 **/
gboolean
grl_related_keys_set_for_id (GrlRelatedKeys *relkeys,
                             const gchar *key_name,
                             const GValue *value)
{
  GList *keys;
  GrlKeyID bind_key, key;
  GrlRegistry *registry;

  keys = grl_related_keys_get_keys (relkeys);
  if (keys) {
    bind_key = GRLPOINTER_TO_KEYID (keys->data);
    g_list_free (keys);
  } else {
    bind_key = GRL_METADATA_KEY_INVALID;
  }

  registry = grl_registry_get_default ();
  key = grl_registry_register_or_lookup_metadata_key (registry,
                                                      key_name,
                                                      value,
                                                      bind_key);
  if (key == GRL_METADATA_KEY_INVALID) {
    return FALSE;
  }

  grl_related_keys_set (relkeys, key, value);
  return TRUE;
}

/**
 * grl_related_keys_remove:
 * @relkeys: set of related keys
 * @key: (type GrlKeyID): key to remove
 *
 * Removes @key from @relkeys set.
 *
 * Since: 0.2.3
 **/
void
grl_related_keys_remove (GrlRelatedKeys *relkeys,
                         GrlKeyID key)
{
  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));
  g_return_if_fail (key != GRL_METADATA_KEY_INVALID);

  g_hash_table_remove (relkeys->priv->data, GRLKEYID_TO_POINTER (key));
}

/**
 * grl_related_keys_has_key:
 * @relkeys: set of related keys to inspect
 * @key: (type GrlKeyID): key to search
 *
 * Checks if @key is in @relkeys.
 *
 * Returns: %TRUE if @key is in @relkeys, %FALSE in other case.
 *
 * Since: 0.1.10
 **/
gboolean
grl_related_keys_has_key (GrlRelatedKeys *relkeys,
                          GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), FALSE);

  return g_hash_table_lookup_extended (relkeys->priv->data,
                                       GRLKEYID_TO_POINTER (key),
                                       NULL, NULL);
}

/**
 * grl_related_keys_get_keys:
 * @relkeys: set of related keys to inspect
 *
 * Returns a list with keys contained in @relkeys.
 *
 * Returns: (transfer container) (element-type GrlKeyID): a list with
 * the keys. The content of the list should not be modified or freed. Use
 * g_list_free() when done using the list.
 *
 * Since: 0.1.13
 **/
GList *
grl_related_keys_get_keys (GrlRelatedKeys *relkeys)
{
  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), NULL);

  return g_hash_table_get_keys (relkeys->priv->data);
}

/**
 * grl_related_keys_dup:
 * @relkeys: set of related keys to duplicate
 *
 * Makes a deep copy of @relkeys and its contents.
 *
 * Returns: (transfer full): a new #GrlRelatedKeys.
 * Free it with #g_object_unref.
 *
 * Since: 0.1.10
 **/
GrlRelatedKeys *
grl_related_keys_dup (GrlRelatedKeys *relkeys)
{
  GList *keys, *key;
  const GValue *value;
  GValue *value_copy;
  GrlRelatedKeys *dup_relkeys;

  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), NULL);

  dup_relkeys = grl_related_keys_new ();

  keys = grl_related_keys_get_keys (relkeys);
  for (key = keys; key; key = g_list_next (key)) {
    value = grl_related_keys_get (relkeys, GRLPOINTER_TO_KEYID (key->data));
    value_copy = g_new0 (GValue, 1);
    g_value_init (value_copy, G_VALUE_TYPE (value));
    g_value_copy (value, value_copy);
    g_hash_table_insert (dup_relkeys->priv->data, key->data, value_copy);
  }

  g_list_free (keys);

  return dup_relkeys;
}
