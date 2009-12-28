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

#ifndef _MS_METADATA_SOURCE_H_
#define _MS_METADATA_SOURCE_H_

#include "ms-media-plugin.h"
#include "ms-metadata-key.h"
#include "content/ms-content.h"

#include <glib.h>
#include <glib-object.h>

/* Macros */

#define MS_TYPE_METADATA_SOURCE (ms_metadata_source_get_type ())
#define MS_METADATA_SOURCE(obj)                                         \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MS_TYPE_METADATA_SOURCE, MsMetadataSource))
#define IS_MS_METADATA_SOURCE(obj)                              \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MS_TYPE_METADATA_SOURCE))
#define MS_METADATA_SOURCE_CLASS(klass)                                 \
  (G_TYPE_CHECK_CLASS_CAST((klass), MS_TYPE_METADATA_SOURCE,  MsMetadataSourceClass))
#define IS_MS_METADATA_SOURCE_CLASS(klass)                      \
  (G_TYPE_CHECK_CLASS_TYPE((klass), MS_TYPE_METADATA_SOURCE))
#define MS_METADATA_SOURCE_GET_CLASS(obj)                               \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MS_TYPE_METADATA_SOURCE, MsMetadataSourceClass))

/* MsMetadata resolution flags */

typedef enum {
  MS_METADATA_RESOLUTION_NORMAL    = 0,
  MS_METADATA_RESOLUTION_FULL      = (1 << 0),
  MS_METADATA_RESOLUTION_USE_RELAY = (1 << 1),
} MsMetadataResolutionFlags;


/* MsMetadataSource object */

typedef struct _MsMetadataSource MsMetadataSource;
typedef struct _MsMetadataSourcePrivate MsMetadataSourcePrivate;

struct _MsMetadataSource {

  MsMediaPlugin parent;

  /*< private >*/
  MsMetadataSourcePrivate *priv;
};

/* Callbacks for MsMetadataSource class */

typedef void (*MsMetadataSourceResultCb) (MsMetadataSource *source,
					  MsContent *media,
                                          gpointer user_data,
                                          const GError *error);

typedef void (*MsMetadataSourceResolveCb) (MsMetadataSource *source,
                                           MsContent *media,
                                           gpointer user_data,
                                           const GError *error);
/* Types for MsMetadataSource */

typedef struct {
  MsMetadataSource *source;
  gchar *object_id;
  GList *keys;
  MsMetadataSourceResultCb callback;
  gpointer user_data;
} MsMetadataSourceMetadataSpec;

typedef struct {
  MsMetadataSource *source;
  GList *keys;
  MsContent *media;
  MsMetadataSourceResolveCb callback;
  gpointer user_data;
} MsMetadataSourceResolveSpec;

/* MsMetadataSource class */

typedef struct _MsMetadataSourceClass MsMetadataSourceClass;

struct _MsMetadataSourceClass {

  MsMediaPluginClass parent_class;

  const GList * (*supported_keys) (MsMetadataSource *source);

  const GList * (*key_depends) (MsMetadataSource *source, MsKeyID key_id);

  void (*metadata) (MsMetadataSource *source,
		    MsMetadataSourceMetadataSpec *ms);

  void (*resolve) (MsMetadataSource *source,
		   MsMetadataSourceResolveSpec *rs);
};

G_BEGIN_DECLS

GType ms_metadata_source_get_type (void);

const GList *ms_metadata_source_supported_keys (MsMetadataSource *source);

GList *ms_metadata_source_filter_supported (MsMetadataSource *source, GList **keys);

const GList *ms_metadata_source_key_depends (MsMetadataSource *source, MsKeyID key_id);

void ms_metadata_source_get (MsMetadataSource *source,
                             const gchar *object_id,
                             const GList *keys,
			     guint flags,
                             MsMetadataSourceResultCb callback,
                             gpointer user_data);

void ms_metadata_source_resolve (MsMetadataSource *source,
                                 const GList *keys,
                                 MsContent *media,
                                 MsMetadataSourceResolveCb callback,
                                 gpointer user_data);

G_END_DECLS

#endif
