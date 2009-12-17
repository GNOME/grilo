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
 * is suggested to better high level classes, like #ContentMedia, which provides
 * functions to access known properties.
 *
 */

#include "content.h"


struct _ContentPrivate {
  GHashTable *data;
};

static void content_finalize (GObject *object);

#define CONTENT_GET_PRIVATE(o)                                  \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CONTENT_TYPE, ContentPrivate))


G_DEFINE_TYPE (Content, content, G_TYPE_OBJECT);

static void
free_val (GValue *val)
{
  if (val) {
    g_value_unset (val);
    g_free (val);
  }
}

static void
content_class_init (ContentClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->finalize = content_finalize;

  g_type_class_add_private (klass, sizeof (ContentPrivate));
}

static void
content_init (Content *self)
{
  self->priv = CONTENT_GET_PRIVATE (self);
  self->priv->data = g_hash_table_new_full (g_direct_hash,
                                            g_direct_equal,
                                            NULL,
                                            (GDestroyNotify) free_val);
}

static void
content_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (content_parent_class)->finalize (object);
}

/**
 * content_new:
 *
 * Creates a new Content object.
 *
 * Returns: a new Content object.
 **/
Content *
content_new (void)
{
  return g_object_new (CONTENT_TYPE, NULL);
}

/**
 * content_get:
 * @content: Content to retrieve value
 * @key: key to look up.
 *
 * Get the value associated with the key. If it does not contain any value, NULL
 * will be returned.
 *
 * Returns: a #GValue. This value should not be modified nor freed by user.
 **/
const GValue *
content_get (Content *content, KeyID key)
{
  g_return_val_if_fail (content, NULL);

  return g_hash_table_lookup (content->priv->data, GUINT_TO_POINTER(key));
}

/**
 * content_set:
 * @content: Content to modify
 * @key: key to change or add
 * @value: the new value
 *
 * Changes the value associated with the key, freeing the old value. If key is
 * not in #Content, then it is added.
 **/
void
content_set (Content *content, KeyID key, const GValue *value)
{
  GValue *copy = NULL;
  g_return_if_fail (content);

  /* Dup value */
  if (value) {
    copy = g_new0 (GValue, 1);
    g_value_init (copy, G_VALUE_TYPE (value));
    g_value_copy (value, copy);
  }

  g_hash_table_insert (content->priv->data, GUINT_TO_POINTER(key), copy);
}

/**
 * content_set_string:
 * @content: Content to modify
 * @key: key to change or add
 * @strvalue: the new value
 *
 * Changes the value associated with the key, freeing the old value. If key is
 * not in #Content, then it is added.
 **/
void
content_set_string (Content *content, KeyID key, const gchar *strvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, strvalue);
  content_set (content, key, &value);
}

/**
 * content_get_string:
 * @content: Content to inspect
 * @key: key to use
 *
 * Returns the value associated with the key. If key has no value, or value is
 * not string, or key is not in content, then NULL is returned.
 *
 * Returns: string associated with key, or NULL in other case. Caller should not
 * change nor free the value.
 **/
const gchar *
content_get_string (Content *content, KeyID key)
{
  const GValue *value = content_get (content, key);

  if (!value || !G_VALUE_HOLDS_STRING(value)) {
    return NULL;
  } else {
    return g_value_get_string (value);
  }
}

/**
 * content_set_int:
 * @content: Content to change
 * @key: key to change or addd
 * @intvalue: the new value
 *
 * Changes the value associated with the key. If key is not in #Content, then it
 * is added.
 **/
void
content_set_int (Content *content, KeyID key, gint intvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, intvalue);
  content_set (content, key, &value);
}

/**
 * content_get_int:
 * @content: Content to inspect
 * @key: key to use
 *
 * Returns the value associated with the key. If key has no value, or value is not a gint, or key is not in content, then 0 is returned.
 *
 * Returns: int value associated with key, or 0 in other case.
 **/
gint
content_get_int (Content *content, KeyID key)
{
  const GValue *value = content_get (content, key);

  if (!value || !G_VALUE_HOLDS_INT(value)) {
    return 0;
  } else {
    return g_value_get_int (value);
  }
}

/**
 * content_add:
 * @content: Content to change
 * @key: key to add
 *
 * Adds a new key to Content, with no value. If key already exists, it does nothing.
 **/
void
content_add (Content *content, KeyID key)
{
  if (!content_has_key (content, key)) {
    content_set (content, key, NULL);
  }
}

/**
 * content_remove:
 * @content: Content to change
 * @key: key to remove
 *
 * Removes key from content, freeing its value. If key is not in content, then
 * it does nothing.
 **/
void
content_remove (Content *content, KeyID key)
{
  g_return_if_fail (content);

  g_hash_table_remove (content->priv->data, GUINT_TO_POINTER(key));
}

/**
 * content_has_key:
 * @content: Content to inspect
 * @key: key to search
 *
 * Checks if key is in content.
 *
 * Returns: TRUE if key is in content, FALSE in other case.
 **/
gboolean
content_has_key (Content *content, KeyID key)
{
  g_return_val_if_fail (content, FALSE);

  return g_hash_table_lookup_extended (content->priv->data,
                                       GUINT_TO_POINTER(key), NULL, NULL);
}

/**
 * content_get_keys:
 * @content: Content to inspect
 * @size: number of keys it has
 *
 * Returns an array with keys contained in Content. If size is not NULL, then
 * the number of keys is stored there
 *
 * Returns: a newly-allocated array with keys.
 **/
KeyID *
content_get_keys (Content *content, gint *size)
{
  GList *keylist;
  GList *keynode;
  KeyID *keyarray;
  guint i;
  gint keylist_size;

  g_return_val_if_fail (content, NULL);

  keylist =  g_hash_table_get_keys (content->priv->data);
  keylist_size = g_list_length (keylist);

  keyarray = g_new(KeyID, keylist_size);

  keynode = keylist;
  i = 0;

  while (keynode) {
    keyarray[i] = GPOINTER_TO_UINT(keynode->data);
    keynode = g_list_next (keynode);
    i++;
  }

  g_list_free (keylist);

  if (size) {
    *size = keylist_size;
  }

  return keyarray;
}
