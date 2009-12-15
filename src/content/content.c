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
  self->priv->data = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            (GDestroyNotify) free_val);
}

static void
content_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (content_parent_class)->finalize (object);
}

Content *
content_new (void)
{
  return g_object_new (CONTENT_TYPE, NULL);
}

const GValue *
content_get (Content *content, const gchar *key)
{
  g_return_val_if_fail (content, NULL);
  g_return_val_if_fail (key, NULL);

  return g_hash_table_lookup (content->priv->data, key);
}

void
content_set (Content *content, const gchar *key, const GValue *value)
{
  GValue *copy = NULL;
  g_return_if_fail (content);
  g_return_if_fail (key);

  /* Dup value */
  if (value) {
    copy = g_new0 (GValue, 1);
    g_value_init (copy, G_VALUE_TYPE (value));
    g_value_copy (value, copy);
  }

  g_hash_table_insert (content->priv->data, g_strdup(key), copy);
}

void
content_set_string (Content *content, const gchar *key, const gchar *strvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, strvalue);
  content_set (content, key, &value);
}

const gchar *
content_get_string (Content *content, const gchar *key)
{
  const GValue *value = content_get (content, key);

  if (!value || !G_VALUE_HOLDS_STRING(value)) {
    return NULL;
  } else {
    return g_value_get_string (value);
  }
}

void
content_set_int (Content *content, const gchar *key, gint intvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, intvalue);
  content_set (content, key, &value);
}

gint
content_get_int (Content *content, const gchar *key)
{
  const GValue *value = content_get (content, key);

  if (!value || !G_VALUE_HOLDS_INT(value)) {
    return 0;
  } else {
    return g_value_get_int (value);
  }
}

void
content_add (Content *content, const gchar *key)
{
  content_set (content, key, NULL);
}

void
content_remove (Content *content, const gchar *key)
{
  g_return_if_fail (content);
  g_return_if_fail (key);

  g_hash_table_remove (content->priv->data, key);
}

gboolean
content_has_key (Content *content, const gchar *key)
{
  g_return_val_if_fail (content, FALSE);
  g_return_val_if_fail (key, FALSE);

  return g_hash_table_lookup_extended (content->priv->data,
                                       key, NULL, NULL);
}

GList *
content_get_keys (Content *content)
{
  g_return_val_if_fail (content, NULL);

  return g_hash_table_get_keys (content->priv->data);
}
