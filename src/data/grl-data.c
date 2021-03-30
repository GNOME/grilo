/*
 * Copyright (C) 2010, 2011 Igalia S.L.
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
 * @short_description: Low-level class for storing metadata information about
 * media resources provided by #GrlSource objects.
 * @see_also: #GrlMedia
 *
 * This class acts as a generic metadata dictionary where metadata keys and
 * their values can be stored.  Usually, application and plugin developers would
 * interact with specific subclass of #GrlData, #GrlMedia, which provide
 * specific API to manipulate well known attributes of these media types.
 */

#include "grl-data.h"
#include "grl-log.h"
#include "grl-registry-priv.h"

#define GRL_LOG_DOMAIN_DEFAULT data_log_domain
GRL_LOG_DOMAIN(data_log_domain);

struct _GrlDataPrivate {
  GHashTable *data;
};

static void grl_data_finalize (GObject *object);
static void free_list_values (GrlKeyID key, GList *values, gpointer user_data);

static void free_list_values (GrlKeyID key, GList *values, gpointer user_data);

/* ================ GrlData GObject ================ */

G_DEFINE_TYPE_WITH_PRIVATE (GrlData, grl_data, G_TYPE_OBJECT);

static void
grl_data_class_init (GrlDataClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->finalize = grl_data_finalize;
}

static void
grl_data_init (GrlData *self)
{
  self->priv = grl_data_get_instance_private (self);
  self->priv->data = g_hash_table_new_full (g_direct_hash,
                                            g_direct_equal,
                                            NULL,
                                            NULL);
}

static void
grl_data_finalize (GObject *object)
{
  GrlData *data = GRL_DATA (object);

  g_signal_handlers_destroy (object);
  g_hash_table_foreach (data->priv->data,
                        (GHFunc) free_list_values,
                        NULL);
  g_hash_table_unref (data->priv->data);

  G_OBJECT_CLASS (grl_data_parent_class)->finalize (object);
}

/* ================ Utitilies ================ */

/* Free the list of values, which are of type #GrlRelatedKeys */
static void
free_list_values (GrlKeyID key, GList *values, gpointer user_data)
{
  g_list_free_full (values, g_object_unref);
}

/* Returns the sample key that represents the set of keys related with @key */
static GrlKeyID
get_sample_key (GrlKeyID key)
{
  GrlRegistry *registry;
  const GList *related_keys;

  registry = grl_registry_get_default ();
  related_keys =
    grl_registry_lookup_metadata_key_relation (registry, key);

  if (!related_keys) {
    GRL_WARNING ("Related keys not found for key \"%s\"",
                 grl_metadata_key_get_name (key));
    return GRL_METADATA_KEY_INVALID;
  } else {
    return GRLPOINTER_TO_KEYID (related_keys->data);
  }
}

/* ================ API ================ */

/**
 * grl_data_new:
 *
 * Creates a new data object.
 *
 * Returns: a new data object.
 *
 * Since: 0.1.4
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
 * @key: (type GrlKeyID): key to look up.
 *
 * Get the first value from @data associated with @key.
 *
 * Returns: (transfer none): a #GValue. This value should not be modified nor
 * freed by user.
 *
 * Since: 0.1.4
 **/
const GValue *
grl_data_get (GrlData *data, GrlKeyID key)
{
  GrlRelatedKeys *relkeys = NULL;

  g_return_val_if_fail (GRL_IS_DATA (data), NULL);
  g_return_val_if_fail (key, NULL);

  if (grl_data_length (data, key) > 0) {
    relkeys = grl_data_get_related_keys (data, key, 0);
  }

  if (!relkeys) {
    return NULL;
  }

  return grl_related_keys_get (relkeys, key);
}

/**
 * grl_data_set:
 * @data: data to modify
 * @key: (type GrlKeyID): key to change or add
 * @value: the new value
 *
 * Sets the first value associated with @key in @data. If key already has a
 * value old value is freed and the new one is set.
 *
 * Also, checks that @value is compliant with @key specification, modifying it
 * accordingly. For instance, if @key requires a number between 0 and 10, but
 * @value is outside this range, it will be adapted accordingly.
 *
 * Since: 0.1.4
 **/
void
grl_data_set (GrlData *data, GrlKeyID key, const GValue *value)
{
  GrlRelatedKeys *relkeys = NULL;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  if (!value) {
    return;
  }

  /* Get the right set of related keys */
  if (grl_data_length (data, key) > 0) {
    relkeys = grl_data_get_related_keys (data, key, 0);
  }

  if (!relkeys) {
    /* No related keys; add them */
    relkeys = grl_related_keys_new ();
    grl_related_keys_set (relkeys, key, value);
    grl_data_add_related_keys (data, relkeys);
  } else {
    /* Set the new value */
    grl_related_keys_set (relkeys, key, value);
  }
}

/**
 * grl_data_set_string:
 * @data: data to modify
 * @key: (type GrlKeyID): key to change or add
 * @strvalue: the new value
 *
 * Sets the first string value associated with @key in @data. If @key already
 * has a value old value is freed and the new one is set.
 *
 * Since: 0.1.4
 **/
void
grl_data_set_string (GrlData *data,
                     GrlKeyID key,
                     const gchar *strvalue)
{
  GValue value = { 0 };

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  if (strvalue) {
    g_value_init (&value, G_TYPE_STRING);
    g_value_set_string (&value, strvalue);
    grl_data_set (data, key, &value);
    g_value_unset (&value);
  }
}

/**
 * grl_data_get_string:
 * @data: data to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns the first string value associated with @key from @data. If @key has
 * no first value, or value is not string, or @key is not in @data, then %NULL
 * is returned.
 *
 * Returns: string associated with @key, or %NULL in other case. Caller should
 * not change nor free the value.
 *
 * Since: 0.1.4
 **/
const gchar *
grl_data_get_string (GrlData *data, GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_DATA (data), NULL);
  g_return_val_if_fail (key, NULL);

  value = grl_data_get (data, key);

  if (!value || !G_VALUE_HOLDS_STRING (value)) {
    return NULL;
  } else {
    return g_value_get_string (value);
  }
}

/**
 * grl_data_set_int:
 * @data: data to change
 * @key: (type GrlKeyID): key to change or add
 * @intvalue: the new value
 *
 * Sets the first int value associated with @key in @data. If @key already has a
 * first value old value is replaced by the new one.
 *
 * Since: 0.1.4
 **/
void
grl_data_set_int (GrlData *data, GrlKeyID key, gint intvalue)
{
  GValue value = { 0 };

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, intvalue);
  grl_data_set (data, key, &value);
}

/**
 * grl_data_get_int:
 * @data: data to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns the first int value associated with @key from @data. If @key has no
 * first value, or value is not a gint, or @key is not in data, then 0 is
 * returned.
 *
 * Returns: int value associated with @key, or 0 in other case.
 *
 * Since: 0.1.4
 **/
gint
grl_data_get_int (GrlData *data, GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_DATA (data), 0);
  g_return_val_if_fail (key, 0);

  value = grl_data_get (data, key);

  if (!value || !G_VALUE_HOLDS_INT (value)) {
    return 0;
  } else {
    return g_value_get_int (value);
  }
}

/**
 * grl_data_set_float:
 * @data: data to change
 * @key: (type GrlKeyID): key to change or add
 * @floatvalue: the new value
 *
 * Sets the first float value associated with @key in @data. If @key already has
 * a first value old value is replaced by the new one.
 *
 * Since: 0.1.5
 **/
void
grl_data_set_float (GrlData *data, GrlKeyID key, float floatvalue)
{
  GValue value = { 0 };

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  g_value_init (&value, G_TYPE_FLOAT);
  g_value_set_float (&value, floatvalue);
  grl_data_set (data, key, &value);
}

/**
 * grl_data_get_float:
 * @data: data to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns the first float value associated with @key from @data. If @key has no
 * first value, or value is not a gfloat, or @key is not in data, then 0 is
 * returned.
 *
 * Returns: float value associated with @key, or 0 in other case.
 *
 * Since: 0.1.5
 **/
gfloat
grl_data_get_float (GrlData *data, GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_DATA (data), 0.0);
  g_return_val_if_fail (key, 0.0);

  value = grl_data_get (data, key);

  if (!value || !G_VALUE_HOLDS_FLOAT (value)) {
    return 0;
  } else {
    return g_value_get_float (value);
  }
}

/**
 * grl_data_set_boolean:
 * @data: data to change
 * @key: (type GrlKeyID): key to change or add
 * @boolvalue: the new value
 *
 * Sets the first boolean value associated with @key in @data. If @key already
 * has a first value, old value is replaced by the new one.
 *
 * Since: 0.2.3
 **/
void
grl_data_set_boolean (GrlData *data, GrlKeyID key, gboolean boolvalue)
{
  GValue value = { 0 };

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_value_set_boolean (&value, boolvalue);
  grl_data_set (data, key, &value);
}

/**
 * grl_data_get_boolean:
 * @data: data to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns: the first boolean value associated with @key from @data. If @key has
 * no first value, or value is not a gboolean, or @key is not in the data, then
 * FALSE is returned
 *
 * Since: 0.2.3
 */
gboolean
grl_data_get_boolean (GrlData *data, GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_DATA (data), FALSE);
  g_return_val_if_fail (key, FALSE);

  value = grl_data_get (data, key);

  if (!value || !G_VALUE_HOLDS_BOOLEAN (value)) {
    return FALSE;
  } else {
    return g_value_get_boolean (value);
  }
}

/**
 * grl_data_set_binary:
 * @data: data to change
 * @key: (type GrlKeyID): key to change or add
 * @buf: buffer holding the data
 * @size: size of the buffer
 *
 * Sets the first binary value associated with @key in @data. If @key already
 * has a first value old value is replaced by the new one.
 *
 * Since: 0.1.9
 **/
void
grl_data_set_binary (GrlData *data, GrlKeyID key, const guint8 *buf, gsize size)
{
  GValue v = { 0 };
  GByteArray * array;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  if (!buf || !size) {
    return;
  }

  array = g_byte_array_append(g_byte_array_sized_new(size),
		              buf,
		              size);

  g_value_init (&v, g_byte_array_get_type());
  g_value_take_boxed(&v, array);
  grl_data_set(data, key, &v);
  g_value_unset (&v);
}

/**
 * grl_data_get_binary:
 * @data: data to inspect
 * @key: (type GrlKeyID): key to use
 * @size: (out): location to store the buffer size
 *
 * Returns the first binary value associated with @key from @data. If @key has
 * no first value, or value is not a gfloat, or @key is not in data, then %NULL
 * is returned.
 *
 * Returns: buffer location associated with the @key, or %NULL in other case. If
 * successful @size will be set the to the buffer size.
 *
 * Since: 0.1.9
 **/
const guint8 *
grl_data_get_binary(GrlData *data, GrlKeyID key, gsize *size)
{
  const GValue *value;

  g_return_val_if_fail (size, NULL);
  g_return_val_if_fail (GRL_IS_DATA (data), NULL);
  g_return_val_if_fail (key, NULL);

  value = grl_data_get (data, key);

  if (!value || !G_VALUE_HOLDS_BOXED (value)) {
    return NULL;
  } else {
    GByteArray * array;

    array = g_value_get_boxed(value);
    *size = array->len;
    return (const guint8 *) array->data;
  }
}

/**
 * grl_data_set_boxed:
 * @data: data to modify
 * @key: key to change or add
 * @boxed: the new value
 *
 * Sets the first boxed value associated with @key in @data. If @key already
 * has a value, the old value is freed and the new one is set.
 *
 * Since: 0.2.0
 **/
void
grl_data_set_boxed (GrlData *data, GrlKeyID key, gconstpointer boxed)
{
  GValue value = { 0 };

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);
  g_return_if_fail (boxed != NULL);

  g_value_init (&value, GRL_METADATA_KEY_GET_TYPE (key));
  g_value_set_boxed (&value, boxed);
  grl_data_set (data, key, &value);
  g_value_unset (&value);
}

/**
 * grl_data_get_boxed:
 * @data: data to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns the first boxed value associated with @key from @data. If @key has
 * no first value, that value is not of a boxed type, or @key is not in @data,
 * then %NULL is returned.
 *
 * Returns: (transfer none): the boxed instance associated with @key if
 * possible, or %NULL in other cases. The caller should not change nor free the
 * value.
 *
 * Since: 0.2.0
 **/
gpointer
grl_data_get_boxed (GrlData *data, GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_DATA (data), NULL);
  g_return_val_if_fail (key, NULL);

  value = grl_data_get (data, key);

  if (!value || !G_VALUE_HOLDS_BOXED (value)) {
    return NULL;
  } else {
    return g_value_get_boxed (value);
  }
}

/**
 * grl_data_set_int64:
 * @data: data to change
 * @key: (type GrlKeyID): key to change or add
 * @intvalue: the new value
 *
 * Sets the first int64 value associated with @key in @data. If @key already has a
 * first value old value is replaced by the new one.
 *
 * Since: 0.2.12
 **/
void
grl_data_set_int64 (GrlData *data, GrlKeyID key, gint64 intvalue)
{
  GValue value = { 0 };

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  g_value_init (&value, G_TYPE_INT64);
  g_value_set_int64 (&value, intvalue);
  grl_data_set (data, key, &value);
}

/**
 * grl_data_get_int64:
 * @data: data to inspect
 * @key: (type GrlKeyID): key to use
 *
 * Returns the first int64 value associated with @key from @data. If @key has no
 * first value, or value is not a gint, or @key is not in data, then 0 is
 * returned.
 *
 * Returns: int value associated with @key, or 0 in other case.
 *
 * Since: 0.2.12
 **/
gint64
grl_data_get_int64 (GrlData *data, GrlKeyID key)
{
  const GValue *value;

  g_return_val_if_fail (GRL_IS_DATA (data), 0);
  g_return_val_if_fail (key, 0);

  value = grl_data_get (data, key);

  if (!value || !G_VALUE_HOLDS_INT64 (value)) {
    return 0;
  } else {
    return g_value_get_int64 (value);
  }
}

/**
 * grl_data_set_for_id:
 * @data: data to change
 * @key_name: name of the key to change or add
 * @value: the new value
 *
 * Sets the first value associated with @key_name in @data. This @key_name is used to create
 * a new #GParamSpec instance, which is further used to create and register a key using
 * grl_registry_register_metadata_key(). If @key_name already has a first @value, old
 * value is replaced by the new one.
 *
 * A property key_name consists of segments consisting of ASCII letters and
 * digits, separated by either the '-' or '_' character. The first
 * character of a property key_name must be a letter. Key_names which violate these
 * rules lead to undefined behaviour.
 *
 * Returns: TRUE if @value was set to @key_name, FALSE otherwise.
 *
 * Since: 0.3.6
 **/
gboolean
grl_data_set_for_id (GrlData *data, const gchar *key_name, const GValue *value)
{
  GrlRegistry *registry;
  GrlKeyID key_id;

  registry = grl_registry_get_default ();
  key_id = grl_registry_register_or_lookup_metadata_key (registry,
                                                         key_name,
                                                         value,
                                                         GRL_METADATA_KEY_INVALID);
  if (key_id == GRL_METADATA_KEY_INVALID) {
    return FALSE;
  }

  grl_data_set (data, key_id, value);
  return TRUE;
}


/**
 * grl_data_add_for_id:
 * @data: data to change
 * @key_name: name of the key to change or add
 * @value: the new value
 *
 * Appends the value associated with @key_name to @data. This @key_name is used to create
 * a new #GParamSpec instance, which is further used to create and register a key using
 * grl_registry_register_metadata_key().
 *
 * A property key_name consists of segments consisting of ASCII letters and
 * digits, separated by either the '-' or '_' character. The first
 * character of a property key_name must be a letter. Key_names which violate these
 * rules lead to undefined behaviour.
 *
 * Returns: TRUE if @value was added to @key_name, FALSE otherwise.
 *
 * Since: 0.3.6
 **/
gboolean
grl_data_add_for_id (GrlData *data, const gchar *key_name, const GValue *value)
{
  GrlRegistry *registry;
  GrlKeyID key_id;
  GType value_type;

  registry = grl_registry_get_default ();
  key_id = grl_registry_register_or_lookup_metadata_key (registry,
                                                         key_name,
                                                         value,
                                                         GRL_METADATA_KEY_INVALID);
  if (key_id == GRL_METADATA_KEY_INVALID) {
    return FALSE;
  }

  value_type = G_VALUE_TYPE (value);

  switch (value_type) {
  case G_TYPE_INT:
    grl_data_add_int (data, key_id, g_value_get_int (value));
    break;

  case G_TYPE_INT64:
    grl_data_add_int64 (data, key_id, g_value_get_int64 (value));
    break;

  case G_TYPE_FLOAT:
    grl_data_add_float (data, key_id, g_value_get_float (value));
    break;

  case G_TYPE_STRING:
    grl_data_add_string (data, key_id, g_value_get_string (value));
    break;

  default:
    /* FIXME: We should add support to all GType supported by GrlMedia */
    GRL_WARNING ("'%s' is being ignored as %s type is not being handled",
                 key_name, g_type_name (value_type));
    return FALSE;
  }

  return TRUE;
}

/**
 * grl_data_remove:
 * @data: data to change
 * @key: (type GrlKeyID): key to remove
 *
 * Removes the first value for @key from @data. If there are other keys related
 * to @key their values will also be removed from @data.
 *
 * Since: 0.1.4
 **/
void
grl_data_remove (GrlData *data, GrlKeyID key)
{
  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  grl_data_remove_nth (data, key, 0);
}

/**
 * grl_data_has_key:
 * @data: data to inspect
 * @key: (type GrlKeyID): key to search
 *
 * Checks if @key is in @data.
 *
 * Returns: %TRUE if @key is in @data, %FALSE in other case.
 *
 * Since: 0.1.4
 **/
gboolean
grl_data_has_key (GrlData *data, GrlKeyID key)
{
  GList *related_keys;
  GrlKeyID sample_key;
  gboolean found = FALSE;

  g_return_val_if_fail (GRL_IS_DATA (data), FALSE);
  g_return_val_if_fail (key, FALSE);

  sample_key = get_sample_key (key);
  if (!sample_key) {
    return FALSE;
  }

  related_keys = g_hash_table_lookup (data->priv->data, GRLKEYID_TO_POINTER (sample_key));
  while (related_keys && !found) {
    found = grl_related_keys_has_key (related_keys->data, key);
    related_keys = g_list_next (related_keys);
  }

  return found;
}

/**
 * grl_data_get_keys:
 * @data: data to inspect
 *
 * Returns a list with keys contained in @data.
 *
 * Returns: (transfer container) (element-type GrlKeyID): an array with the
 * keys. The content of the list should not be modified or freed. Use
 * g_list_free() when done using the list.
 *
 * Since: 0.1.4
 **/
GList *
grl_data_get_keys (GrlData *data)
{
  GList *allkeys = NULL;
  GList *keylist, *key;
  GrlRegistry *registry;
  const GList *relkeys;

  g_return_val_if_fail (GRL_IS_DATA (data), NULL);

  keylist = g_hash_table_get_keys (data->priv->data);
  registry = grl_registry_get_default ();

  for (key = keylist; key; key = g_list_next (key)) {
    GrlKeyID key_id = GRLPOINTER_TO_KEYID (key->data);
    relkeys =
        grl_registry_lookup_metadata_key_relation (registry, key_id);
    while (relkeys) {
      if (grl_data_has_key (data, GRLPOINTER_TO_KEYID (relkeys->data))) {
        allkeys = g_list_prepend (allkeys, relkeys->data);
      }
      relkeys = g_list_next (relkeys);
    }
  }

  g_list_free (keylist);

  return allkeys;
}

/**
 * grl_data_add_related_keys:
 * @data: data to change
 * @relkeys: (transfer full): a set of related properties with their values
 *
 * Adds a new set of values into @data.
 *
 * All keys in @prop must be related among them.
 *
 * @data will take the ownership of @relkeys, so do not modify it.
 *
 * Since: 0.1.10
 **/
void
grl_data_add_related_keys (GrlData *data,
                           GrlRelatedKeys *relkeys)
{
  GList *keys;
  GList *list_relkeys;
  GrlKeyID sample_key;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));

  keys = grl_related_keys_get_keys (relkeys);
  if (!keys) {
    /* Ignore empty set of related keys */
    GRL_WARNING ("Trying to add an empty GrlRelatedKeys to GrlData");
    g_object_unref (relkeys);
    return;
  }

  sample_key = get_sample_key (GRLPOINTER_TO_KEYID (keys->data));
  g_list_free (keys);

  if (!sample_key) {
    g_object_unref (relkeys);
    return;
  }

  list_relkeys = g_hash_table_lookup (data->priv->data,
                                      GRLKEYID_TO_POINTER (sample_key));
  list_relkeys = g_list_append (list_relkeys, relkeys);
  g_hash_table_insert (data->priv->data,
                       GRLKEYID_TO_POINTER (sample_key),
                       list_relkeys);
}

/**
 * grl_data_add_string:
 * @data: data to append
 * @key: (type GrlKeyID): key to append
 * @strvalue: the new value
 *
 * Appends a new string value for @key in @data.
 *
 * Since: 0.1.10
 **/
void
grl_data_add_string (GrlData *data,
                     GrlKeyID key,
                     const gchar *strvalue)
{
  GrlRelatedKeys *relkeys;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  if (strvalue) {
    relkeys = grl_related_keys_new ();
    grl_related_keys_set_string (relkeys, key, strvalue);
    grl_data_add_related_keys (data, relkeys);
  }
}

/**
 * grl_data_add_int:
 * @data: data to append
 * @key: (type GrlKeyID): key to append
 * @intvalue: the new value
 *
 * Appends a new int value for @key in @data.
 *
 * Since: 0.1.10
 **/
void
grl_data_add_int (GrlData *data,
                  GrlKeyID key,
                  gint intvalue)
{
  GrlRelatedKeys *relkeys;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  relkeys = grl_related_keys_new ();
  grl_related_keys_set_int (relkeys, key, intvalue);
  grl_data_add_related_keys (data, relkeys);
}

/**
 * grl_data_add_float:
 * @data: data to append
 * @key: (type GrlKeyID): key to append
 * @floatvalue: the new value
 *
 * Appends a new float value for @key in @data.
 *
 * Since: 0.1.10
 **/
void
grl_data_add_float (GrlData *data,
                    GrlKeyID key,
                    gfloat floatvalue)
{
  GrlRelatedKeys *relkeys;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  relkeys = grl_related_keys_new ();
  grl_related_keys_set_float (relkeys, key, floatvalue);
  grl_data_add_related_keys (data, relkeys);
}

/**
 * grl_data_add_binary:
 * @data: data to append
 * @key: (type GrlKeyID): key to append
 * @buf: the buffer containing the new value
 * @size: size of buffer
 *
 * Appends a new binary value for @key in @data.
 *
 * Since: 0.1.10
 **/
void
grl_data_add_binary (GrlData *data,
                     GrlKeyID key,
                     const guint8 *buf,
                     gsize size)
{
  GrlRelatedKeys *relkeys;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  if (!buf || !size) {
    return;
  }

  relkeys = grl_related_keys_new ();
  grl_related_keys_set_binary (relkeys, key, buf, size);
  grl_data_add_related_keys (data, relkeys);
}

/**
 * grl_data_add_boxed:
 * @data: data to append
 * @key: (type GrlKeyID): key to append
 * @boxed: the new value
 *
 * Appends a new boxed value for @key in @data.
 *
 * Since: 0.2.0
 **/
void
grl_data_add_boxed (GrlData *data,
                    GrlKeyID key,
                    gconstpointer boxed)
{
  GrlRelatedKeys *relkeys;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);
  g_return_if_fail (boxed != NULL);

  relkeys = grl_related_keys_new ();
  grl_related_keys_set_boxed (relkeys, key, boxed);
  grl_data_add_related_keys (data, relkeys);
}

/**
 * grl_data_add_int64:
 * @data: data to append
 * @key: (type GrlKeyID): key to append
 * @intvalue: the new value
 *
 * Appends a new int64 value for @key in @data.
 *
 * Since: 0.2.12
 **/
void
grl_data_add_int64 (GrlData *data,
                    GrlKeyID key,
                    gint64 intvalue)
{
  GrlRelatedKeys *relkeys;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  relkeys = grl_related_keys_new ();
  grl_related_keys_set_int64 (relkeys, key, intvalue);
  grl_data_add_related_keys (data, relkeys);
}

/**
 * grl_data_length:
 * @data: a data
 * @key: a metadata key
 *
 * Returns how many values @key or related keys have in @data: if @key has no
 * value, but a related key has, then it is counted as positive.
 *
 * As example, let's think in three related keys, K1, K2 and K3, and then thinks
 * we have added several values for those keys, as:
 *
 *   (V10, V20, V30), (V11, NULL, V31), (V12, NULL, V32)
 *
 * Therefore, when invoking grl_data_length (data, K2) it will return 3:
 * considering K2 and the related keys (K1 and K3), there are 3 values.
 *
 * Returns: number of values
 *
 * Since: 0.1.10
 **/
guint
grl_data_length (GrlData *data,
                 GrlKeyID key)
{
  GrlKeyID sample_key;

  g_return_val_if_fail (GRL_IS_DATA (data), 0);
  g_return_val_if_fail (key, 0);

  sample_key = get_sample_key (key);
  if (!sample_key) {
    return 0;
  }

  return g_list_length (g_hash_table_lookup (data->priv->data,
                                             GRLKEYID_TO_POINTER (sample_key)));
}

/**
 * grl_data_get_related_keys:
 * @data: a data
 * @key: a metadata key
 * @index: element to retrieve, starting at 0
 *
 * Returns a set containing the values for @key and related keys at position
 * @index from @data.
 *
 * If user changes any of the values in the related keys, the changes will
 * become permanent.
 *
 * Returns: (transfer none): a #GrlRelatedKeys. Do not free it.
 *
 * Since: 0.1.10
 **/
GrlRelatedKeys *
grl_data_get_related_keys (GrlData *data,
                           GrlKeyID key,
                           guint index)
{
  GList *relkeys_list;
  GrlKeyID sample_key;
  GrlRelatedKeys *relkeys;

  g_return_val_if_fail (GRL_IS_DATA (data), NULL);
  g_return_val_if_fail (key, NULL);

  sample_key = get_sample_key (key);
  if (!sample_key) {
    return NULL;
  }

  relkeys_list = g_hash_table_lookup (data->priv->data,
                                      GRLKEYID_TO_POINTER (sample_key));
  relkeys = g_list_nth_data (relkeys_list, index);

  if (!relkeys) {
    GRL_WARNING ("%s: index %u out of range", __FUNCTION__, index);
    return NULL;
  }

  return relkeys;
}

/**
 * grl_data_get_single_values_for_key:
 * @data: a data
 * @key: a metadata key
 *
 * Returns all non-%NULL values for @key from @data. This ignores related keys.
 *
 * Returns: (element-type GObject.Value) (transfer container): a #GList with
 * values. Do not change or free the values. Free the list with #g_list_free.
 *
 * Since: 0.1.13
 */
GList *
grl_data_get_single_values_for_key (GrlData *data,
                                    GrlKeyID key)
{
  GList *related_keys;
  GList *values = NULL;
  GrlKeyID sample_key;
  const GValue *v;

  g_return_val_if_fail (GRL_IS_DATA (data), NULL);
  g_return_val_if_fail (key, NULL);

  sample_key = get_sample_key (key);
  if (!sample_key) {
    return NULL;
  }

  related_keys = g_hash_table_lookup (data->priv->data,
                                      GRLKEYID_TO_POINTER (sample_key));
  while (related_keys) {
    v = grl_related_keys_get (related_keys->data, key);
    if (v) {
      values = g_list_prepend (values, (gpointer) v);
    }
    related_keys = g_list_next (related_keys);
  }

  return g_list_reverse (values);
}

/**
 * grl_data_get_single_values_for_key_string:
 * @data: a data
 * @key: a metadata key
 *
 * Returns all non-%NULL values for @key from @data. @key must have been
 * registered as a string-type key. This ignores related keys.
 *
 * Returns: (element-type utf8) (transfer container): a #GList with values. Do
 * not change or free the strings. Free the list with #g_list_free.
 *
 * Since: 0.1.13
 **/
GList *
grl_data_get_single_values_for_key_string (GrlData *data,
                                           GrlKeyID key)
{
  GList *list_strings = NULL;
  GList *list_values;
  GList *value;
  const gchar *string_value;

  g_return_val_if_fail (GRL_IS_DATA (data), NULL);
  g_return_val_if_fail (key, NULL);

  /* Verify key is of type string */
  if (GRL_METADATA_KEY_GET_TYPE (key) != G_TYPE_STRING) {
    GRL_WARNING ("%s: requested key is not of type string", __FUNCTION__);
    return NULL;
  }

  list_values = grl_data_get_single_values_for_key (data, key);
  for (value = list_values; value; value = g_list_next (value)) {
    string_value = g_value_get_string (value->data);
    if (string_value) {
      list_strings = g_list_prepend (list_strings, (gpointer) string_value);
    }
  }

  g_list_free (list_values);

  return g_list_reverse (list_strings);
}

/**
 * grl_data_remove_nth:
 * @data: a data
 * @key: a metadata key
 * @index: index of key to be removed, starting at 0
 *
 * Removes the value at position @index for @key from @data. If there are other
 * keys related to @key, their values at position @index will also be removed
 * from @data.
 *
 * Since: 0.1.10
 **/
void
grl_data_remove_nth (GrlData *data,
                     GrlKeyID key,
                     guint index)
{
  GList *relkeys_element;
  GList *relkeys_list;
  GrlKeyID sample_key;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (key);

  sample_key = get_sample_key (key);
  if (!sample_key) {
    return;
  }

  relkeys_list = g_hash_table_lookup (data->priv->data,
                                      GRLKEYID_TO_POINTER (sample_key));
  relkeys_element = g_list_nth (relkeys_list, index);
  if (!relkeys_element) {
    GRL_WARNING ("%s: index %u out of range", __FUNCTION__, index);
    return;
  }

  g_object_unref (relkeys_element->data);
  relkeys_list = g_list_delete_link (relkeys_list, relkeys_element);
  g_hash_table_insert (data->priv->data,
                       GRLKEYID_TO_POINTER (sample_key),
                       relkeys_list);
}

/**
 * grl_data_set_related_keys:
 * @data: a data
 * @relkeys: a set of related keys
 * @index: position to be updated, starting at 0
 *
 * Updates the values at position @index in @data with values in @relkeys.
 *
 * @data will take ownership of @relkeys, so do not free it after invoking this
 * function.
 *
 * Since: 0.1.10
 **/
void
grl_data_set_related_keys (GrlData *data,
                           GrlRelatedKeys *relkeys,
                           guint index)
{
  GList *keys;
  GList *relkeys_element;
  GList *relkeys_list;
  GrlKeyID sample_key;

  g_return_if_fail (GRL_IS_DATA (data));
  g_return_if_fail (GRL_IS_RELATED_KEYS (relkeys));

  keys = grl_related_keys_get_keys (relkeys);
  if (!keys) {
    GRL_WARNING ("Trying to set an empty GrlRelatedKeys into GrlData");
    g_object_unref (relkeys);
    return;
  }

  sample_key = get_sample_key (GRLPOINTER_TO_KEYID (keys->data));
  g_list_free (keys);
  if (!sample_key) {
    return;
  }

  relkeys_list = g_hash_table_lookup (data->priv->data,
                                      GRLKEYID_TO_POINTER (sample_key));
  relkeys_element = g_list_nth (relkeys_list, index);
  if (!relkeys_element) {
    GRL_WARNING ("%s: index %u out of range", __FUNCTION__, index);
    return;
  }

  g_object_unref (relkeys_element->data);
  relkeys_element->data = relkeys;
}

/**
 * grl_data_dup:
 * @data: data to duplicate
 *
 * Makes a deep copy of @data and all its contents.
 *
 * Returns: (transfer full): a new #GrlData. Free it with #g_object_unref.
 *
 * Since: 0.1.10
 **/
GrlData *
grl_data_dup (GrlData *data)
{
  GList *dup_relkeys_list;
  GList *key;
  GList *keys;
  GList *relkeys_list;
  GrlData *dup_data;

  g_return_val_if_fail (GRL_IS_DATA (data), NULL);

  dup_data = grl_data_new ();
  keys = g_hash_table_get_keys (data->priv->data);
  for (key = keys; key; key = g_list_next (key)) {
    dup_relkeys_list = NULL;
    relkeys_list = g_hash_table_lookup (data->priv->data, key->data);
    while (relkeys_list) {
      dup_relkeys_list =
        g_list_prepend (dup_relkeys_list,
                        grl_related_keys_dup (relkeys_list->data));
      relkeys_list = g_list_next (relkeys_list);
    }
    g_hash_table_insert (dup_data->priv->data,
                         key->data,
                         g_list_reverse (relkeys_list));
  }

  g_list_free (keys);

  return dup_data;
}
