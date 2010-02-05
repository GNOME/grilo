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

#ifndef _GRL_MEDIA_SOURCE_H_
#define _GRL_MEDIA_SOURCE_H_

#include <grl-media-plugin.h>
#include <grl-metadata-source.h>
#include <grl-content.h>
#include <grl-content-box.h>

#include <glib.h>
#include <glib-object.h>

/* Macros */

#define GRL_TYPE_MEDIA_SOURCE                   \
  (grl_media_source_get_type ())

#define GRL_MEDIA_SOURCE(obj)                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_MEDIA_SOURCE,   \
                               GrlMediaSource))

#define GRL_IS_MEDIA_SOURCE(obj)                        \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_MEDIA_SOURCE))

#define GRL_MEDIA_SOURCE_CLASS(klass)                   \
  (G_TYPE_CHECK_CLASS_CAST((klass),                     \
                           GRL_TYPE_MEDIA_SOURCE,       \
                           GrlMediaSourceClass))

#define GRL_IS_MEDIA_SOURCE_CLASS(klass)                \
  (G_TYPE_CHECK_CLASS_TYPE((klass),                     \
                           GRL_TYPE_MEDIA_SOURCE))

#define GRL_MEDIA_SOURCE_GET_CLASS(obj)                         \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                            \
                              GRL_TYPE_MEDIA_SOURCE,            \
                              GrlMediaSourceClass))

/* GrlMediaSource object */

typedef struct _GrlMediaSource        GrlMediaSource;
typedef struct _GrlMediaSourcePrivate GrlMediaSourcePrivate;

struct _GrlMediaSource {

  GrlMetadataSource parent;

  /*< private >*/
  GrlMediaSourcePrivate *priv;
};

/* Callbacks for GrlMediaSource class */

typedef void (*GrlMediaSourceResultCb) (GrlMediaSource *source,
                                        guint browse_id,
                                        GrlContentMedia *media,
                                        guint remaining,
                                        gpointer user_data,
                                        const GError *error);

typedef void (*GrlMediaSourceMetadataCb) (GrlMediaSource *source,
                                          GrlContentMedia *media,
                                          gpointer user_data,
                                          const GError *error);

typedef void (*GrlMediaSourceStoreCb) (GrlMediaSource *source,
                                       GrlContentBox *parent,
                                       GrlContentMedia *media,
                                       gpointer user_data,
                                       const GError *error);

typedef void (*GrlMediaSourceRemoveCb) (GrlMediaSource *source,
                                        GrlContentMedia *media,
                                        gpointer user_data,
                                        const GError *error);

/* Types for MediaSourceClass */

typedef struct {
  GrlMediaSource *source;
  guint browse_id;
  GrlContentMedia *container;
  GList *keys;
  guint skip;
  guint count;
  GrlMetadataResolutionFlags flags;
  GrlMediaSourceResultCb callback;
  gpointer user_data;
} GrlMediaSourceBrowseSpec;

typedef struct {
  GrlMediaSource *source;
  guint search_id;
  gchar *text;
  GList *keys;
  guint skip;
  guint count;
  GrlMetadataResolutionFlags flags;
  GrlMediaSourceResultCb callback;
  gpointer user_data;
} GrlMediaSourceSearchSpec;

typedef struct {
  GrlMediaSource *source;
  guint query_id;
  gchar *query;
  GList *keys;
  guint skip;
  guint count;
  GrlMetadataResolutionFlags flags;
  GrlMediaSourceResultCb callback;
  gpointer user_data;
} GrlMediaSourceQuerySpec;

typedef struct {
  GrlMediaSource *source;
  GrlContentMedia *media;
  GList *keys;
  GrlMetadataResolutionFlags flags;
  GrlMediaSourceMetadataCb callback;
  gpointer user_data;
} GrlMediaSourceMetadataSpec;

typedef struct {
  GrlMediaSource *source;
  GrlContentBox *parent;
  GrlContentMedia *media;
  GrlMediaSourceStoreCb callback;
  gpointer user_data;
} GrlMediaSourceStoreSpec;

typedef struct {
  GrlMediaSource *source;
  gchar *media_id;
  GrlContentMedia *media;
  GrlMediaSourceRemoveCb callback;
  gpointer user_data;
} GrlMediaSourceRemoveSpec;

/* GrlMediaSource class */

typedef struct _GrlMediaSourceClass GrlMediaSourceClass;

struct _GrlMediaSourceClass {

  GrlMetadataSourceClass parent_class;

  guint browse_id;

  void (*browse) (GrlMediaSource *source, GrlMediaSourceBrowseSpec *bs);

  void (*search) (GrlMediaSource *source, GrlMediaSourceSearchSpec *ss);

  void (*query) (GrlMediaSource *source, GrlMediaSourceQuerySpec *qs);

  void (*cancel) (GrlMediaSource *source, guint operation_id);

  void (*metadata) (GrlMediaSource *source, GrlMediaSourceMetadataSpec *ms);

  void (*store) (GrlMediaSource *source, GrlMediaSourceStoreSpec *ss);

  void (*remove) (GrlMediaSource *source, GrlMediaSourceRemoveSpec *ss);
};

G_BEGIN_DECLS

GType grl_media_source_get_type (void);

guint grl_media_source_browse (GrlMediaSource *source,
                               GrlContentMedia *container,
                               const GList *keys,
                               guint skip,
                               guint count,
                               GrlMetadataResolutionFlags flags,
                               GrlMediaSourceResultCb callback,
                               gpointer user_data);

guint grl_media_source_search (GrlMediaSource *source,
                               const gchar *text,
                               const GList *keys,
                               guint skip,
                               guint count,
                               GrlMetadataResolutionFlags flags,
                               GrlMediaSourceResultCb callback,
                               gpointer user_data);

guint grl_media_source_query (GrlMediaSource *source,
                              const gchar *query,
                              const GList *keys,
                              guint skip,
                              guint count,
                              GrlMetadataResolutionFlags flags,
                              GrlMediaSourceResultCb callback,
                              gpointer user_data);

void grl_media_source_metadata (GrlMediaSource *source,
                                GrlContentMedia *media,
                                const GList *keys,
                                GrlMetadataResolutionFlags flags,
                                GrlMediaSourceMetadataCb callback,
                                gpointer user_data);

void grl_media_source_store (GrlMediaSource *source,
                             GrlContentBox *parent,
                             GrlContentMedia *media,
                             GrlMediaSourceStoreCb callback,
                             gpointer user_data);

void grl_media_source_remove (GrlMediaSource *source,
                              GrlContentMedia *media,
                              GrlMediaSourceRemoveCb callback,
                              gpointer user_data);

void grl_media_source_cancel (GrlMediaSource *source, guint operation_id);

void grl_media_source_set_operation_data (GrlMediaSource *source,
                                          guint operation_id,
                                          gpointer data);

gpointer grl_media_source_get_operation_data (GrlMediaSource *source,
                                              guint operation_id);

void grl_media_source_set_auto_split_threshold (GrlMediaSource *source,
                                                guint threshold);

guint grl_media_source_get_auto_split_threshold (GrlMediaSource *source);

G_END_DECLS

#endif /* _GRL_MEDIA_SOURCE_H_ */
