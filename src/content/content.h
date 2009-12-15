#ifndef __CONTENT_H__
#define __CONTENT_H__

#include <glib-object.h>


G_BEGIN_DECLS

#define CONTENT_TYPE                            \
  (content_get_type())
#define CONTENT(obj)                            \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),           \
                               CONTENT_TYPE,    \
                               Content))
#define CONTENT_CLASS(klass)                    \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            CONTENT_TYPE,       \
                            ContentClass))
#define IS_CONTENT(obj)                         \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),           \
                               CONTENT_TYPE))
#define IS_CONTENT_CLASS(klass)                 \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),            \
                            CONTENT_TYPE))
#define CONTENT_GET_CLASS(obj)                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),            \
                              CONTENT_TYPE,     \
                              ContentClass))

typedef struct _Content        Content;
typedef struct _ContentPrivate ContentPrivate;
typedef struct _ContentClass   ContentClass;

struct _ContentClass
{
  GObjectClass parent_class;
};

struct _Content
{
  GObject parent;

  ContentPrivate *priv;
};

GType content_get_type (void) G_GNUC_CONST;
Content *content_new (void);
void content_set (Content *content, const gchar *key, const GValue *value);
void content_set_string (Content *content, const gchar *key, const gchar *strvalue);
void content_set_int (Content *content, const gchar *key, gint intvalue);
const GValue *content_get (Content *content, const gchar *key);
const gchar *content_get_string (Content *content, const gchar *key);
gint content_get_int (Content *content, const gchar *key);
void content_add (Content *content, const gchar *key);
void content_remove (Content *content, const gchar *key);
gboolean content_has_key (Content *content, const gchar *key);
GList *content_get_keys (Content *content);
G_END_DECLS

#endif /* __CONTENT_H__ */
