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

/*
 * Low-level class to store content.
 *
 * This class acts as dictionary where keys and their values can be stored. It
 * is suggested to better high level classes, like #MsContentMedia, which provides
 * functions to access known properties.
 *
 */

#include "ms-content.h"

enum {
  PROP_0,
  PROP_OVERWRITE
};

struct _MsContentPrivate {
  GHashTable *data;
  gboolean overwrite;
};

static void ms_content_set_property (GObject *object,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec);

static void ms_content_get_property (GObject *object,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec);

static void ms_content_finalize (GObject *object);

#define MS_CONTENT_GET_PRIVATE(o)                                       \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MS_TYPE_CONTENT, MsContentPrivate))

G_DEFINE_TYPE (MsContent, ms_content, G_TYPE_OBJECT);

static void
free_val (GValue *val)
{
  if (val) {
    g_value_unset (val);
    g_free (val);
  }
}

static void
ms_content_class_init (MsContentClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->set_property = ms_content_set_property;
  gobject_class->get_property = ms_content_get_property;
  gobject_class->finalize = ms_content_finalize;

  g_type_class_add_private (klass, sizeof (MsContentPrivate));

  g_object_class_install_property (gobject_class,
                                   PROP_OVERWRITE,
                                   g_param_spec_boolean ("overwrite",
                                                         "Overwrite",
                                                         "Overwrite current values",
                                                         FALSE,
                                                         G_PARAM_READWRITE));
}

static void
ms_content_init (MsContent *self)
{
  self->priv = MS_CONTENT_GET_PRIVATE (self);
  self->priv->data = g_hash_table_new_full (g_direct_hash,
                                            g_direct_equal,
                                            NULL,
                                            (GDestroyNotify) free_val);
}

static void
ms_content_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (ms_content_parent_class)->finalize (object);
}

static void
ms_content_set_property (GObject *object,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *pspec)
{
  MsContent *self = MS_CONTENT (object);

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
ms_content_get_property (GObject *object,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *pspec)
{
  MsContent *self = MS_CONTENT (object);

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
 * ms_content_new:
 *
 * Creates a new content object.
 *
 * Returns: a new content object.
 **/
MsContent *
ms_content_new (void)
{
  return g_object_new (MS_TYPE_CONTENT,
		       NULL);
}

/**
 * ms_content_get:
 * @content: content to retrieve value
 * @key: key to look up.
 *
 * Get the value associated with the key. If it does not contain any value, NULL
 * will be returned.
 *
 * Returns: a #GValue. This value should not be modified nor freed by user.
 **/
const GValue *
ms_content_get (MsContent *content, MsKeyID key)
{
  g_return_val_if_fail (MS_IS_CONTENT (content), NULL);

  return g_hash_table_lookup (content->priv->data, MSKEYID_TO_POINTER(key));
}

/**
 * ms_content_set:
 * @content: content to modify
 * @key: key to change or add
 * @value: the new value
 *
 * Sets the value associated with the key. If key already has a value and
 * #overwrite is TRUE, old value is freed and the new one is set.
 **/
void
ms_content_set (MsContent *content, MsKeyID key, const GValue *value)
{
  GValue *copy = NULL;
  g_return_if_fail (MS_IS_CONTENT (content));

  if (content->priv->overwrite ||
      g_hash_table_lookup (content->priv->data,
                           MSKEYID_TO_POINTER (key)) != NULL) {
    /* Dup value */
    if (value) {
      copy = g_new0 (GValue, 1);
      g_value_init (copy, G_VALUE_TYPE (value));
      g_value_copy (value, copy);
    }

    g_hash_table_insert (content->priv->data, MSKEYID_TO_POINTER(key), copy);
  }
}

/**
 * ms_content_set_string:
 * @content: content to modify
 * @key: key to change or add
 * @strvalue: the new value
 *
 * Sets the value associated with the key. If key already has a value and
 * #overwrite is TRUE, old value is freed and the new one is set.
 **/
void
ms_content_set_string (MsContent *content, MsKeyID key, const gchar *strvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, strvalue);
  ms_content_set (content, key, &value);
}

/**
 * ms_content_get_string:
 * @content: content to inspect
 * @key: key to use
 *
 * Returns the value associated with the key. If key has no value, or value is
 * not string, or key is not in content, then NULL is returned.
 *
 * Returns: string associated with key, or NULL in other case. Caller should not
 * change nor free the value.
 **/
const gchar *
ms_content_get_string (MsContent *content, MsKeyID key)
{
  const GValue *value = ms_content_get (content, key);

  if (!value || !G_VALUE_HOLDS_STRING(value)) {
    return NULL;
  } else {
    return g_value_get_string (value);
  }
}

/**
 * ms_content_set_int:
 * @content: content to change
 * @key: key to change or addd
 * @intvalue: the new value
 *
 * Sets the value associated with the key. If key already has a value and
 * #overwrite is TRUE, old value is replaced by the new one.
 **/
void
ms_content_set_int (MsContent *content, MsKeyID key, gint intvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, intvalue);
  ms_content_set (content, key, &value);
}

/**
 * ms_content_get_int:
 * @content: content to inspect
 * @key: key to use
 *
 * Returns the value associated with the key. If key has no value, or value is
 * not a gint, or key is not in content, then 0 is returned.
 *
 * Returns: int value associated with key, or 0 in other case.
 **/
gint
ms_content_get_int (MsContent *content, MsKeyID key)
{
  const GValue *value = ms_content_get (content, key);

  if (!value || !G_VALUE_HOLDS_INT(value)) {
    return 0;
  } else {
    return g_value_get_int (value);
  }
}

/**
 * ms_content_set_float:
 * @content: content to change
 * @key: key to change or addd
 * @floatvalue: the new value
 *
 * Sets the value associated with the key. If key already has a value and
 * #overwrite is TRUE, old value is replaced by the new one.
 **/
void
ms_content_set_float (MsContent *content, MsKeyID key, gint floatvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_FLOAT);
  g_value_set_float (&value, floatvalue);
  ms_content_set (content, key, &value);
}

/**
 * ms_content_get_float:
 * @content: content to inspect
 * @key: key to use
 *
 * Returns the value associated with the key. If key has no value, or value is
 * not a gfloat, or key is not in content, then 0 is returned.
 *
 * Returns: float value associated with key, or 0 in other case.
 **/
gfloat
ms_content_get_float (MsContent *content, MsKeyID key)
{
  const GValue *value = ms_content_get (content, key);

  if (!value || !G_VALUE_HOLDS_FLOAT(value)) {
    return 0;
  } else {
    return g_value_get_float (value);
  }
}

/**
 * ms_content_add:
 * @content: content to change
 * @key: key to add
 *
 * Adds a new key to content, with no value. If key already exists, it does nothing.
 **/
void
ms_content_add (MsContent *content, MsKeyID key)
{
  if (!ms_content_has_key (content, key)) {
    ms_content_set (content, key, NULL);
  }
}

/**
 * ms_content_remove:
 * @content: content to change
 * @key: key to remove
 *
 * Removes key from content, freeing its value. If key is not in content, then
 * it does nothing.
 **/
void
ms_content_remove (MsContent *content, MsKeyID key)
{
  g_return_if_fail (MS_IS_CONTENT (content));

  g_hash_table_remove (content->priv->data, MSKEYID_TO_POINTER(key));
}

/**
 * ms_content_has_key:
 * @content: content to inspect
 * @key: key to search
 *
 * Checks if key is in content.
 *
 * Returns: TRUE if key is in content, FALSE in other case.
 **/
gboolean
ms_content_has_key (MsContent *content, MsKeyID key)
{
  g_return_val_if_fail (MS_IS_CONTENT (content), FALSE);

  return g_hash_table_lookup_extended (content->priv->data,
                                       MSKEYID_TO_POINTER(key), NULL, NULL);
}

/**
 * ms_content_get_keys:
 * @content: content to inspect
 *
 * Returns a list with keys contained in content.
 *
 * Returns: an array with the keys.
 **/
GList *
ms_content_get_keys (MsContent *content)
{
  GList *keylist;

  g_return_val_if_fail (MS_IS_CONTENT (content), NULL);

  keylist = g_hash_table_get_keys (content->priv->data);

  return keylist;
}

/**
 * ms_content_key_is_known:
 * @content: content to inspect
 * @key: key to search
 *
 * Checks if the key has a value.
 *
 * Returns: TRUE if key has a value.
 **/
gboolean
ms_content_key_is_known (MsContent *content, MsKeyID key)
{
  GValue *v;

  g_return_val_if_fail (MS_IS_CONTENT (content), FALSE);

  v = g_hash_table_lookup (content->priv->data,
                           MSKEYID_TO_POINTER(key));

  if (!v) {
    return FALSE;
  }

  if (G_VALUE_HOLDS_STRING (v)) {
    return g_value_get_string(v) != NULL;
  }

  return TRUE;
}

/**
 * ms_content_set_overwrite:
 * @content: content to change
 * @overwrite: if content can be overwritten
 *
 * This controls if #ms_content_set will overwrite current value of a property
 * with the new one.
 *
 * Set it to TRUE so old values are overwritten, or FALSE in other case (default
 * is FALSE).
 **/
void
ms_content_set_overwrite (MsContent *content, gboolean overwrite)
{
  g_return_if_fail (MS_IS_CONTENT (content));

  if (content->priv->overwrite != overwrite) {
    content->priv->overwrite = overwrite;
    g_object_notify (G_OBJECT (content), "overwrite");
  }
}

/**
 * ms_content_get_overwrite:
 * @content: content to inspect
 *
 * Checks if old values are replaced when calling #ms_content_set.
 *
 * Returns: TRUE if values will be overwritten.
 **/
gboolean
ms_content_get_overwrite (MsContent *content)
{
  g_return_val_if_fail (MS_IS_CONTENT (content), FALSE);

  return content->priv->overwrite;
}
