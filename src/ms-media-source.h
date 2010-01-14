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

#ifndef _MS_MEDIA_SOURCE_H_
#define _MS_MEDIA_SOURCE_H_

#include "ms-media-plugin.h"
#include "ms-metadata-source.h"
#include "content/ms-content.h"

#include <glib.h>
#include <glib-object.h>

/* Macros */

#define MS_TYPE_MEDIA_SOURCE (ms_media_source_get_type ())
#define MS_MEDIA_SOURCE(obj)						\
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MS_TYPE_MEDIA_SOURCE, MsMediaSource))
#define IS_MS_MEDIA_SOURCE(obj)					\
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MS_TYPE_MEDIA_SOURCE))
#define MS_MEDIA_SOURCE_CLASS(klass)					\
  (G_TYPE_CHECK_CLASS_CAST((klass), MS_TYPE_MEDIA_SOURCE,  MsMediaSourceClass))
#define IS_MS_MEDIA_SOURCE_CLASS(klass)			\
  (G_TYPE_CHECK_CLASS_TYPE((klass), MS_TYPE_MEDIA_SOURCE))
#define MS_MEDIA_SOURCE_GET_CLASS(obj)					\
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MS_TYPE_MEDIA_SOURCE, MsMediaSourceClass))

/* MsMediaSource object */

typedef struct _MsMediaSource MsMediaSource;
typedef struct _MsMediaSourcePrivate MsMediaSourcePrivate;

struct _MsMediaSource {

  MsMetadataSource parent;

  /*< private >*/
  MsMediaSourcePrivate *priv;
};

/* Callbacks for MsMediaSource class */

typedef void (*MsMediaSourceResultCb) (MsMediaSource *source,
                                       guint browse_id,
                                       MsContentMedia *media,
                                       guint remaining,
                                       gpointer user_data,
                                       const GError *error);

typedef void (*MsMediaSourceMetadataCb) (MsMediaSource *source,
					 MsContentMedia *media,
					 gpointer user_data,
					 const GError *error);

/* Types for MediaSourceClass */

typedef struct {
  MsMediaSource *source;
  guint browse_id;
  MsContentMedia *container;
  GList *keys;
  guint skip;
  guint count;
  MsMetadataResolutionFlags flags;
  MsMediaSourceResultCb callback;
  gpointer user_data;
} MsMediaSourceBrowseSpec;

typedef struct {
  MsMediaSource *source;
  guint search_id;
  gchar *text;
  GList *keys;
  guint skip;
  guint count;
  MsMetadataResolutionFlags flags;
  MsMediaSourceResultCb callback;
  gpointer user_data;
} MsMediaSourceSearchSpec;

typedef struct {
  MsMediaSource *source;
  guint query_id;
  gchar *query;
  GList *keys;
  guint skip;
  guint count;
  MsMetadataResolutionFlags flags;
  MsMediaSourceResultCb callback;
  gpointer user_data;
} MsMediaSourceQuerySpec;

typedef struct {
  MsMediaSource *source;
  MsContentMedia *media;
  GList *keys;
  MsMetadataResolutionFlags flags;
  MsMediaSourceMetadataCb callback;
  gpointer user_data;
} MsMediaSourceMetadataSpec;


/* MsMediaSource class */

typedef struct _MsMediaSourceClass MsMediaSourceClass;

struct _MsMediaSourceClass {

  MsMetadataSourceClass parent_class;
  
  guint browse_id;

  void (*browse) (MsMediaSource *source, 
		  MsMediaSourceBrowseSpec *bs);
  
  void (*search) (MsMediaSource *source,
		  MsMediaSourceSearchSpec *ss);

  void (*query) (MsMediaSource *source,
		 MsMediaSourceQuerySpec *qs);

  void (*metadata) (MsMediaSource *source,
		    MsMediaSourceMetadataSpec *ms);

};

G_BEGIN_DECLS

GType ms_media_source_get_type (void);

guint ms_media_source_browse (MsMediaSource *source, 
			      MsContentMedia *container,
			      const GList *keys,
			      guint skip,
			      guint count,
			      MsMetadataResolutionFlags flags,
			      MsMediaSourceResultCb callback,
			      gpointer user_data);

guint ms_media_source_search (MsMediaSource *source,
                              const gchar *text,
                              const GList *keys,
                              guint skip,
                              guint count,
                              MsMetadataResolutionFlags flags,
                              MsMediaSourceResultCb callback,
                              gpointer user_data);

guint ms_media_source_query (MsMediaSource *source,
			     const gchar *query,
			     const GList *keys,
			     guint skip,
			     guint count,
			     MsMetadataResolutionFlags flags,
			     MsMediaSourceResultCb callback,
			     gpointer user_data);

void ms_media_source_metadata (MsMediaSource *source,
			       MsContentMedia *media,
			       const GList *keys,
			       MsMetadataResolutionFlags flags,
			       MsMediaSourceMetadataCb callback,
			       gpointer user_data);
G_END_DECLS

#endif
