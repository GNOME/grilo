/*
 * Copyright (C) 2010 Igalia S.L.
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

#ifndef _METADATA_SOURCE_H_
#define _METADATA_SOURCE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "media-plugin.h"
#include "metadata-key.h"

#include <glib.h>
#include <glib-object.h>

/* Macros */

#define METADATA_SOURCE_TYPE (metadata_source_get_type ())
#define METADATA_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), METADATA_SOURCE_TYPE, MetadataSource))
#define IS_METADATA_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), METADATA_SOURCE_TYPE))
#define METADATA_SOURCE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), METADATA_SOURCE_TYPE,  MetadataSourceClass))
#define IS_METADATA_SOURCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), METADATA_SOURCE_TYPE))
#define METADATA_SOURCE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), METADATA_SOURCE_TYPE, MetadataSourceClass))

/* Metadata resolution flags */

typedef enum {
  METADATA_RESOLUTION_NORMAL    = 0,
  METADATA_RESOLUTION_FULL      = (1 << 0),
} MetadataResolutionFlags;


/* MetadataSource object */

typedef struct _MetadataSource MetadataSource;
typedef struct _MetadataSourcePrivate MetadataSourcePrivate;

struct _MetadataSource {

  MediaPlugin parent;

  /*< private >*/
  MetadataSourcePrivate *priv;
};

/* Callbacks for MetadataSource class */

typedef void (*MetadataSourceResultCb) (MetadataSource *source,
					const gchar *metadata_id,
					GHashTable *metadata,
					gpointer user_data,
					const GError *error);

/* MetadataSource class */

typedef struct _MetadataSourceClass MetadataSourceClass;

struct _MetadataSourceClass {

  MediaPluginClass parent_class;

  const KeyID * (*supported_keys) (MetadataSource *source);

  KeyID * (*key_depends) (MetadataSource *source, KeyID key_id);

  void (*metadata) (MetadataSource *source,
		    const gchar *object_id,
		    const KeyID *keys,
		    MetadataSourceResultCb callback,
		    gpointer user_data);
};

G_BEGIN_DECLS

GType metadata_source_get_type (void);

const KeyID *metadata_source_supported_keys (MetadataSource *source);

GList *metadata_source_filter_supported (MetadataSource *source, GList **keys);

KeyID *metadata_source_key_depends (MetadataSource *source, KeyID key_id);

void metadata_source_get (MetadataSource *source,
			  const gchar *object_id,
			  const KeyID *keys,
			  MetadataSourceResultCb callback,
			  gpointer user_data);

G_END_DECLS

#endif
