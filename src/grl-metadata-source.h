/*
 * Copyright (C) 2010, 2011 Igalia S.L.
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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#ifndef _GRL_METADATA_SOURCE_H_
#define _GRL_METADATA_SOURCE_H_

#include <grl-source.h>
#include <grl-metadata-key.h>
#include <grl-media.h>
#include <grl-definitions.h>

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

/**
 * GrlMetadataWritingFlags:
 * @GRL_WRITE_NORMAL: Normal mode.
 * @GRL_WRITE_FULL: Try other plugins if necessary.
 *
 * Flags for metadata writing operations.
 */
typedef enum {
  GRL_WRITE_NORMAL     = 0,        /* Normal mode */
  GRL_WRITE_FULL       = (1 << 0), /* Try other plugins if necessary */
} GrlMetadataWritingFlags;

/* GrlMetadataSource object */

typedef struct _GrlMetadataSource        GrlMetadataSource;
typedef struct _GrlMetadataSourcePrivate GrlMetadataSourcePrivate;

struct _GrlMetadataSource {

  GrlSource parent;

  /*< private >*/
  GrlMetadataSourcePrivate *priv;

  gpointer _grl_reserved[GRL_PADDING];
};

/* Callbacks for GrlMetadataSource class */

/**
 * GrlMetadataSourceResolveCb:
 * @source: a metadata source
 * @operation_id: operation identifier
 * @media: (transfer full): a #GrlMedia transfer object
 * @user_data: user data passed to grl_metadata_source_resolve()
 * @error: (type uint): possible #GError generated when resolving the metadata
 *
 * Prototype for the callback passed to grl_metadata_source_resolve()
 */
typedef void (*GrlMetadataSourceResolveCb) (GrlMetadataSource *source,
                                            guint operation_id,
                                            GrlMedia *media,
                                            gpointer user_data,
                                            const GError *error);

/**
 * GrlMetadataSourceSetMetadataCb:
 * @source: a metadata source
 * @media: (transfer full): a #GrlMedia transfer object
 * @failed_keys: (element-type GrlKeyID) (transfer container): #GList of
 * keys that could not be updated, if any
 * @user_data: user data passed to grl_metadata_source_set_metadata()
 * @error: (type uint): possible #GError generated when updating the metadata
 *
 * Prototype for the callback passed to grl_metadata_source_set_metadata()
 */
typedef void (*GrlMetadataSourceSetMetadataCb) (GrlMetadataSource *source,
						GrlMedia *media,
						GList *failed_keys,
						gpointer user_data,
						const GError *error);

/* Types for GrlMetadataSource */

/**
 * GrlMetadataSourceResolveSpec:
 * @source: a metadata source
 * @resolve_id: operation identifier
 * @keys: the #GList of #GrlKeyID to fetch and store
 * @media: a #GrlMedia transfer object
 * @flags: bitwise mask of #GrlMetadataResolutionFlags with the resolution
 * strategy
 * @callback: the callback passed to grl_metadata_source_resolve()
 * @user_data: user data passed to grl_metadata_source_resolve()
 *
 * Represents the closure used by the derived objects to fetch, store and
 * return the transfer object to the client's code.
 */
typedef struct {
  GrlMetadataSource *source;
  guint resolve_id;
  GList *keys;
  GrlMedia *media;
  GrlOperationOptions *options;
  GrlMetadataSourceResolveCb callback;
  gpointer user_data;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING - 1];
} GrlMetadataSourceResolveSpec;

/**
 * GrlMetadataSourceSetMetadataSpec:
 * @source: a metadata source
 * @media: a #GrlMedia transfer object
 * @keys: List of keys to be stored/updated.
 * @flags: Flags to control specific bahviors of the set metadata operation.
 * @callback: the callback passed to grl_metadata_source_set_metadata()
 * @user_data: user data passed to grl_metadata_source_set_metadata()
 * @failed_keys: for internal use of the framework only.
 *
 * Represents the closure used by the derived objects to operate.
 */
typedef struct {
  GrlMetadataSource *source;
  GrlMedia *media;
  GList *keys;
  GrlMetadataWritingFlags flags;
  GrlMetadataSourceSetMetadataCb callback;
  gpointer user_data;
  GList *failed_keys;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
} GrlMetadataSourceSetMetadataSpec;

typedef struct _GrlMetadataSourceClass GrlMetadataSourceClass;

/**
 * GrlMetadataSourceClass:
 * @parent_class: the parent class structure
 * @resolve: resolve the metadata of a given transfer object
 * @set_metadata: update metadata values for a given object in a
 * permanent fashion
 * @may_resolve: return FALSE if it can be known without blocking that @key_id
 * cannot be resolved for @media, TRUE otherwise. Optionally fill @missing_keys
 * with a list of keys that would be needed to resolve. See
 * grl_metadata_source_may_resolve().
 *
 * Grilo MetadataSource class. Override the vmethods to implement the
 * element functionality.
 */
struct _GrlMetadataSourceClass {

  GrlSourceClass parent_class;

  void (*resolve) (GrlMetadataSource *source,
		   GrlMetadataSourceResolveSpec *rs);

  void (*set_metadata) (GrlMetadataSource *source,
			GrlMetadataSourceSetMetadataSpec *sms);

  gboolean (*may_resolve) (GrlMetadataSource *source, GrlMedia *media,
                           GrlKeyID key_id, GList **missing_keys);

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING - 4];
};

G_BEGIN_DECLS

GType grl_metadata_source_get_type (void);

gboolean grl_metadata_source_may_resolve (GrlMetadataSource *source,
                                          GrlMedia *media,
                                          GrlKeyID key_id,
                                          GList **missing_keys);

guint grl_metadata_source_resolve (GrlMetadataSource *source,
                                   const GList *keys,
                                   GrlMedia *media,
                                   GrlOperationOptions *options,
                                   GrlMetadataSourceResolveCb callback,
                                   gpointer user_data);

GrlMedia *grl_metadata_source_resolve_sync (GrlMetadataSource *source,
                                            const GList *keys,
                                            GrlMedia *media,
                                            GrlOperationOptions *options,
                                            GError **error);

void grl_metadata_source_set_metadata (GrlMetadataSource *source,
				       GrlMedia *media,
				       GList *keys,
				       GrlMetadataWritingFlags flags,
				       GrlMetadataSourceSetMetadataCb callback,
				       gpointer user_data);

GList *grl_metadata_source_set_metadata_sync (GrlMetadataSource *source,
                                              GrlMedia *media,
                                              GList *keys,
                                              GrlMetadataWritingFlags flags,
                                              GError **error);

G_END_DECLS

#endif /* _GRL_METADATA_SOURCE_H_ */
