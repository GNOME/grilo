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

#ifndef _GRL_METADATA_SOURCE_H_
#define _GRL_METADATA_SOURCE_H_

#include <grl-media-plugin.h>
#include <grl-metadata-key.h>
#include <grl-content-media.h>

#include <glib.h>
#include <glib-object.h>

/* Macros */

#define GRL_TYPE_METADATA_SOURCE                \
  (grl_metadata_source_get_type ())

#define GRL_METADATA_SOURCE(obj)                                \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                           \
                               GRL_TYPE_METADATA_SOURCE,        \
                               GrlMetadataSource))

#define GRL_IS_METADATA_SOURCE(obj)                             \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                           \
                               GRL_TYPE_METADATA_SOURCE))

#define GRL_METADATA_SOURCE_CLASS(klass)                \
  (G_TYPE_CHECK_CLASS_CAST((klass),                     \
                           GRL_TYPE_METADATA_SOURCE,    \
                           GrlMetadataSourceClass))

#define GRL_IS_METADATA_SOURCE_CLASS(klass)             \
  (G_TYPE_CHECK_CLASS_TYPE((klass),                     \
                           GRL_TYPE_METADATA_SOURCE))

#define GRL_METADATA_SOURCE_GET_CLASS(obj)              \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_METADATA_SOURCE, \
                              GrlMetadataSourceClass))

/* GrlMetadata resolution flags */

typedef enum {
  GRL_RESOLVE_NORMAL     = 0,        /* Normal mode */
  GRL_RESOLVE_FULL       = (1 << 0), /* Try other plugins if necessary */
  GRL_RESOLVE_IDLE_RELAY = (1 << 1), /* Use idle loop to relay results */
  GRL_RESOLVE_FAST_ONLY  = (1 << 2), /* Only resolve fast metadata keys */
} GrlMetadataResolutionFlags;


/* GrlMetadataSource object */

typedef struct _GrlMetadataSource        GrlMetadataSource;
typedef struct _GrlMetadataSourcePrivate GrlMetadataSourcePrivate;

struct _GrlMetadataSource {

  GrlMediaPlugin parent;

  /*< private >*/
  GrlMetadataSourcePrivate *priv;
};

/* Callbacks for GrlMetadataSource class */

typedef void (*GrlMetadataSourceResolveCb) (GrlMetadataSource *source,
                                            GrlContentMedia *media,
                                            gpointer user_data,
                                            const GError *error);
/* Types for GrlMetadataSource */

typedef struct {
  GrlMetadataSource *source;
  GList *keys;
  GrlContentMedia *media;
  guint flags;
  GrlMetadataSourceResolveCb callback;
  gpointer user_data;
} GrlMetadataSourceResolveSpec;

typedef enum {
  GRL_OP_NONE         = 0,
  GRL_OP_METADATA     = 1,
  GRL_OP_RESOLVE      = 1 << 1,
  GRL_OP_BROWSE       = 1 << 2,
  GRL_OP_SEARCH       = 1 << 3,
  GRL_OP_QUERY        = 1 << 4,
  GRL_OP_STORE        = 1 << 5,
  GRL_OP_STORE_PARENT = 1 << 6,
  GRL_OP_REMOVE       = 1 << 7,
} GrlSupportedOps;

/* GrlMetadataSource class */

typedef struct _GrlMetadataSourceClass GrlMetadataSourceClass;

struct _GrlMetadataSourceClass {

  GrlMediaPluginClass parent_class;

  GrlSupportedOps (*supported_operations) (GrlMetadataSource *source);

  const GList * (*supported_keys) (GrlMetadataSource *source);

  const GList * (*slow_keys) (GrlMetadataSource *source);

  const GList * (*key_depends) (GrlMetadataSource *source, GrlKeyID key_id);

  void (*resolve) (GrlMetadataSource *source,
		   GrlMetadataSourceResolveSpec *rs);
};

G_BEGIN_DECLS

GType grl_metadata_source_get_type (void);

GrlSupportedOps grl_metadata_source_supported_operations (GrlMetadataSource *source);

const GList *grl_metadata_source_supported_keys (GrlMetadataSource *source);

const GList *grl_metadata_source_slow_keys (GrlMetadataSource *source);

GList *grl_metadata_source_filter_supported (GrlMetadataSource *source,
                                             GList **keys,
                                             gboolean return_filtered);

GList *grl_metadata_source_filter_slow (GrlMetadataSource *source,
                                        GList **keys,
                                        gboolean return_filtered);

const GList *grl_metadata_source_key_depends (GrlMetadataSource *source,
                                              GrlKeyID key_id);

void grl_metadata_source_resolve (GrlMetadataSource *source,
                                  const GList *keys,
                                  GrlContentMedia *media,
                                  guint flags,
                                  GrlMetadataSourceResolveCb callback,
                                  gpointer user_data);

const gchar *grl_metadata_source_get_id (GrlMetadataSource *source);

const gchar *grl_metadata_source_get_name (GrlMetadataSource *source);

const gchar *grl_metadata_source_get_description (GrlMetadataSource *source);

G_END_DECLS

#endif /* _GRL_METADATA_SOURCE_H_ */
