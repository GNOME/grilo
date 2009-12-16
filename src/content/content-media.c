#include "content-media.h"


static void content_media_dispose (GObject *object);
static void content_media_finalize (GObject *object);

G_DEFINE_TYPE (ContentMedia, content_media, CONTENT_TYPE);

static void
content_media_class_init (ContentMediaClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;

    gobject_class->dispose = content_media_dispose;
    gobject_class->finalize = content_media_finalize;
}

static void
content_media_init (ContentMedia *self)
{
}

static void
content_media_dispose (GObject *object)
{
    G_OBJECT_CLASS (content_media_parent_class)->dispose (object);
}

static void
content_media_finalize (GObject *object)
{
    g_signal_handlers_destroy (object);
    G_OBJECT_CLASS (content_media_parent_class)->finalize (object);
}

ContentMedia *
content_media_new (void)
{
  return g_object_new (CONTENT_TYPE_MEDIA, NULL);
}
