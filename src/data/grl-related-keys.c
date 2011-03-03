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
 * @see_also: #GrlPluginRegistry, #GrlData
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

struct _GrlRelatedKeysPrivate {
  GHashTable *data;
};

static void grl_related_keys_finalize (GObject *object);
static void free_value (GValue *val);

#define GRL_RELATED_KEYS_GET_PRIVATE(o)                                 \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o),                                    \
                                GRL_TYPE_RELATED_KEYS,                  \
                                GrlRelatedKeysPrivate))

/* ================ GrlRelatedKeys GObject ================ */

G_DEFINE_TYPE (GrlRelatedKeys, grl_related_keys, G_TYPE_OBJECT);

static void
grl_related_keys_class_init (GrlRelatedKeysClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->finalize = grl_related_keys_finalize;

  g_type_class_add_private (klass, sizeof (GrlRelatedKeysPrivate));
}

static void
grl_related_keys_init (GrlRelatedKeys *self)
{
  self->priv = GRL_RELATED_KEYS_GET_PRIVATE (self);
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
 *  Creates a new #GrlRelatedKeys instance that can be used to store related
 *  keys and their values.
 *
 * Returns: a new object.
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
    } else if (key_type == G_TYPE_BYTE_ARRAY) {
      next_value = va_arg (args, gpointer);
      grl_related_keys_set_binary (prop,
                                   next_key,
                                   next_value,
                                   va_arg (args, gsize));
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

 * Returns: a new #GrlRelatedKeys
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
 * @key: (type Grl.KeyID): key to look up.
 *
 * Get the value associated with @key from @relkeys. If it does not contain any
 * value, %NULL will be returned.
 *
 * Returns: (transfer none): a #GValue. This value should not be modified nor
 * freed by user.
 **/
const GValue *
grl_related_keys_get (GrlRelatedKeys *relkeys,
                      GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), NULL);
  g_return_val_if_fail (key, NULL);

  return g_hash_table_lookup (relkeys->priv->data, key);
}

/**
 * grl_related_keys_set:
 * @relkeys: set of related keys to modify
 * @key: (type Grl.KeyID): key to change or add
 * @value: the new value
 *
 * Sets the value associated with @key into @relkeys. Old value is freed and
 * the new one is set.
 *
 * Also, checks that @value is compliant with @key specification, modifying it
 * accordingly. For instance, if @key requires a number between 0 and 10, but
 * value is outside this range, it will be adapted accordingly.
 **/
void
grl_related_keys_set (GrlRelatedKeys *relkeys,
                      GrlKeyID key,
                      const GValue *value)
{
  GValue *copy = NULL;

  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));
  g_return_if_fail (key);

  /* Dup value */
  if (value) {
    if (G_VALUE_TYPE (value) == GRL_METADATA_KEY_GET_TYPE (key)) {
      copy = g_new0 (GValue, 1);
      g_value_init (copy, G_VALUE_TYPE (value));
      g_value_copy (value, copy);
    } else {
      GRL_WARNING ("value has type %s, but expected %s",
                   g_type_name (G_VALUE_TYPE (value)),
                   g_type_name (GRL_METADATA_KEY_GET_TYPE (key)));
    }
  }

  if (copy && g_param_value_validate (key, copy)) {
    GRL_WARNING ("'%s' value invalid, adjusting",
                 GRL_METADATA_KEY_GET_NAME (key));
  }
  g_hash_table_insert (relkeys->priv->data, key, copy);
}

/**
 * grl_related_keys_set_string:
 * @relkeys: set of related keys to modify
 * @key: (type Grl.KeyID): key to change or add
 * @strvalue: the new value
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as a strying-type key. Old value is freed and the new one is set.
 **/
void
grl_related_keys_set_string (GrlRelatedKeys *relkeys,
                             GrlKeyID key,
                             const gchar *strvalue)
{
  if (strvalue) {
    GValue value = { 0 };
    g_value_init (&value, G_TYPE_STRING);
    g_value_set_string (&value, strvalue);
    grl_related_keys_set (relkeys, key, &value);
    g_value_unset (&value);
  } else {
    grl_related_keys_set (relkeys, key, NULL);
  }
}

/**
 * grl_related_keys_get_string:
 * @relkeys: set of related keys to inspect
 * @key: (type Grl.KeyID): key to use
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * or value is not string, or @key is not in @relkeys, then %NULL is returned.
 *
 * Returns: string associated with @key, or %NULL in other case. Caller should
 * not change nor free the value.
 **/
const gchar *
grl_related_keys_get_string (GrlRelatedKeys *relkeys,
                             GrlKeyID key)
{
  const GValue *value = grl_related_keys_get (relkeys, key);

  if (!value || !G_VALUE_HOLDS_STRING (value)) {
    return NULL;
  } else {
    return g_value_get_string (value);
  }
}

/**
 * grl_related_keys_set_int:
 * @relkeys: set of related keys to change
 * @key: (type Grl.KeyID): key to change or add
 * @intvalue: the new value
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as an int-type key. Old value is replaced by the new one.
 **/
void
grl_related_keys_set_int (GrlRelatedKeys *relkeys,
                          GrlKeyID key,
                          gint intvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, intvalue);
  grl_related_keys_set (relkeys, key, &value);
}

/**
 * grl_related_keys_get_int:
 * @relkeys: set of related keys to inspect
 * @key: (type Grl.KeyID): key to use
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * or value is not a gint, or @key is not in @relkeys, then 0 is returned.
 *
 * Returns: int value associated with @key, or 0 in other case.
 **/
gint
grl_related_keys_get_int (GrlRelatedKeys *relkeys,
                          GrlKeyID key)
{
  const GValue *value = grl_related_keys_get (relkeys, key);

  if (!value || !G_VALUE_HOLDS_INT (value)) {
    return 0;
  } else {
    return g_value_get_int (value);
  }
}

/**
 * grl_related_keys_set_float:
 * @relkeys: set of related keys to change
 * @key: (type Grl.KeyID): key to change or add
 * @floatvalue: the new value
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as a float-type key. Old value is replaced by the new one.
 **/
void
grl_related_keys_set_float (GrlRelatedKeys *relkeys,
                            GrlKeyID key,
                            float floatvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_FLOAT);
  g_value_set_float (&value, floatvalue);
  grl_related_keys_set (relkeys, key, &value);
}

/**
 * grl_related_keys_get_float:
 * @relkeys: set of related keys to inspect
 * @key: (type Grl.KeyID): key to use
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * or value is not a gfloat, or @key is not in @relkeys, then 0 is returned.
 *
 * Returns: float value associated with @key, or 0 in other case.
 **/
gfloat
grl_related_keys_get_float (GrlRelatedKeys *relkeys,
                            GrlKeyID key)
{
  const GValue *value = grl_related_keys_get (relkeys, key);

  if (!value || !G_VALUE_HOLDS_FLOAT (value)) {
    return 0;
  } else {
    return g_value_get_float (value);
  }
}

/**
 * grl_related_keys_set_binary:
 * @relkeys: set of related keys to change
 * @key: (type Grl.KeyID): key to change or add
 * @buf: buffer holding the relkeys
 * @size: size of the buffer
 *
 * Sets the value associated with @key into @relkeys. @key must have been
 * registered as a binary-type key. Old value is replaced by the new one.
 **/
void
grl_related_keys_set_binary (GrlRelatedKeys *relkeys,
                             GrlKeyID key,
                             const guint8 *buf,
                             gsize size)
{
  GValue v = { 0 };
  GByteArray *array;

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
 * @key: (type Grl.KeyID): key to use
 * @size: (out): location to store the buffer size
 *
 * Returns the value associated with @key from @relkeys. If @key has no value,
 * or value is not a binary, or @key is not in @relkeys, then 0 is returned.
 *
 * Returns: buffer location associated with @key, or %NULL in other case. If
 * successful @size will be set to the buffer size.
 **/
const guint8 *
grl_related_keys_get_binary (GrlRelatedKeys *relkeys,
                             GrlKeyID key,
                             gsize *size)
{
  g_return_val_if_fail (size, NULL);

  const GValue *value = grl_related_keys_get (relkeys, key);

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
 * grl_related_keys_add:
 * @relkeys: set of related keys to change
 * @key: (type Grl.KeyID): key to add
 *
 * Adds a new @key to @relkeys, with no value. If @key already exists, it does
 * nothing.
 **/
void
grl_related_keys_add (GrlRelatedKeys *relkeys,
                      GrlKeyID key)
{
  if (!grl_related_keys_has_key (relkeys, key)) {
    grl_related_keys_set (relkeys, key, NULL);
  }
}

/**
 * grl_related_keys_has_key:
 * @relkeys: set of related keys to inspect
 * @key: (type Grl.KeyID): key to search
 *
 * Checks if @key is in @relkeys.
 *
 * Returns: %TRUE if @key is in @relkeys, %FALSE in other case.
 **/
gboolean
grl_related_keys_has_key (GrlRelatedKeys *relkeys,
                          GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), FALSE);

  return g_hash_table_lookup_extended (relkeys->priv->data, key, NULL, NULL);
}

/**
 * grl_related_keys_get_keys:
 * @relkeys: set of related keys to inspect
 * @include_unknown: %TRUE if keys with no value must be included
 *
 * Returns a list with keys contained in @relkeys. If @include_unknown is
 * %FALSE, only those keys in @relkeys that have actually a value will be
 * returned.
 *
 * Returns: (transfer container) (element-type Grl.KeyID): an array
 * with the keys. The content of the list should not be modified or freed. Use
 * g_list_free() when done using the list.
 **/
GList *
grl_related_keys_get_keys (GrlRelatedKeys *relkeys,
                           gboolean include_unknown)
{
  GList *keylist;

  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), NULL);

  keylist = g_hash_table_get_keys (relkeys->priv->data);

  if (!include_unknown) {
    keylist = g_list_remove_all (keylist, NULL);
  }

  return keylist;
}

/**
 * grl_related_keys_key_is_known:
 * @relkeys: set of related keys to inspect
 * @key: (type Grl.KeyID): key to search
 *
 * Checks if @key has a value in @relkeys.
 *
 * Returns: %TRUE if @key has a value.
 **/
gboolean
grl_related_keys_key_is_known (GrlRelatedKeys *relkeys,
                               GrlKeyID key)
{
  GValue *v;

  g_return_val_if_fail (GRL_IS_RELATED_KEYS (relkeys), FALSE);

  v = g_hash_table_lookup (relkeys->priv->data, key);

  if (!v) {
    return FALSE;
  }

  if (G_VALUE_HOLDS_STRING (v)) {
    return g_value_get_string (v) != NULL;
  }

  return TRUE;
}

/**
 * grl_related_keys_dup:
 * @relkeys: set of related keys to duplicate
 *
 * Makes a deep copy of @relkeys and its contents.
 *
 * Returns: a new #GrlRelatedKeys. Free it with #g_object_unref.
 **/
GrlRelatedKeys *
grl_related_keys_dup (GrlRelatedKeys *relkeys)
{
  GList *keys, *key;
  const GValue *value;
  GValue *value_copy;
  GrlRelatedKeys *dup_relkeys;

  g_return_val_if_fail (relkeys, NULL);

  dup_relkeys = grl_related_keys_new ();

  keys = grl_related_keys_get_keys (relkeys, TRUE);
  for (key = keys; key; key = g_list_next (key)) {
    value = grl_related_keys_get (relkeys, key->data);
    if (value) {
      value_copy = g_new0 (GValue, 1);
      g_value_init (value_copy, G_VALUE_TYPE (value));
      g_value_copy (value, value_copy);
    } else {
      value_copy = NULL;
    }
    g_hash_table_insert (dup_relkeys->priv->data, key->data, value_copy);
  }

  g_list_free (keys);

  return dup_relkeys;
}
