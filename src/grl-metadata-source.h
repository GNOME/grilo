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

#include <grl-media-plugin.h>
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
 * GrlMetadataResolutionFlags:
 * @GRL_RESOLVE_NORMAL: Normal mode.
 * @GRL_RESOLVE_FULL: Try other plugins if necessary.
 * @GRL_RESOLVE_IDLE_RELAY: Use idle loop to relay results.
 * @GRL_RESOLVE_FAST_ONLY: Only resolve fast metadata keys.
 *
 * GrlMetadata resolution flags
 */
typedef enum {
  GRL_RESOLVE_NORMAL     = 0,        /* Normal mode */
  GRL_RESOLVE_FULL       = (1 << 0), /* Try other plugins if necessary */
  GRL_RESOLVE_IDLE_RELAY = (1 << 1), /* Use idle loop to relay results */
  GRL_RESOLVE_FAST_ONLY  = (1 << 2), /* Only resolve fast metadata keys */
} GrlMetadataResolutionFlags;

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

  GrlMediaPlugin parent;

  /*< private >*/
  GrlMetadataSourcePrivate *priv;

  gpointer _grl_reserved[GRL_PADDING];
};

/* Callbacks for GrlMetadataSource class */

/**
 * GrlMetadataSourceResolveCb:
 * @source: a metadata source
 * @media: (transfer full): a #GrlMedia transfer object
 * @user_data: user data passed to grl_metadata_source_resolve()
 * @error: (not-error) (type uint): possible #GError generated when resolving the metadata
 *
 * Prototype for the callback passed to grl_metadata_source_resolve()
 */
typedef void (*GrlMetadataSourceResolveCb) (GrlMetadataSource *source,
                                            GrlMedia *media,
                                            gpointer user_data,
                                            const GError *error);

/**
 * GrlMetadataSourceSetMetadataCb:
 * @source: a metadata source
 * @media: (transfer full): a #GrlMedia transfer object
 * @failed_keys: (element-type GObject.ParamSpec) (transfer container): #GList of
 * keys that could not be updated, if any
 * @user_data: user data passed to grl_metadata_source_set_metadata()
 * @error: (not-error) (type uint): possible #GError generated when updating the metadata
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
  GList *keys;
  GrlMedia *media;
  GrlMetadataResolutionFlags flags;
  GrlMetadataSourceResolveCb callback;
  gpointer user_data;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
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

/**
 * GrlSupportedOps:
 * @GRL_OP_NONE: no operation is supported
 * @GRL_OP_METADATA: Fetch specific keys of metadata based on the media id.
 * @GRL_OP_RESOLVE: Fetch specific keys of metadata based on other metadata.
 * @GRL_OP_BROWSE: Retrieve complete sets of #GrlMedia
 * @GRL_OP_SEARCH: Look up for #GrlMedia given a search text
 * @GRL_OP_QUERY:  Look up for #GrlMedia give a service specific query
 * @GRL_OP_STORE: Store content in a service
 * @GRL_OP_STORE_PARENT: Store content as child of a certian parent category.
 * @GRL_OP_REMOVE: Remove content from a service.
 * @GRL_OP_SET_METADATA: Update metadata of a #GrlMedia in a service.
 * @GRL_OP_MEDIA_FROM_URI: Create a #GrlMedia instance from an URI
 * representing a media resource.
 *
 * Bitwise flags which reflect the kind of operations that a
 * #GrlMediaPlugin supports.
 */
typedef enum {
  GRL_OP_NONE            = 0,
  GRL_OP_METADATA        = 1,
  GRL_OP_RESOLVE         = 1 << 1,
  GRL_OP_BROWSE          = 1 << 2,
  GRL_OP_SEARCH          = 1 << 3,
  GRL_OP_QUERY           = 1 << 4,
  GRL_OP_STORE           = 1 << 5,
  GRL_OP_STORE_PARENT    = 1 << 6,
  GRL_OP_REMOVE          = 1 << 7,
  GRL_OP_SET_METADATA    = 1 << 8,
  GRL_OP_MEDIA_FROM_URI  = 1 << 9,
} GrlSupportedOps;

/* GrlMetadataSource class */

typedef struct _GrlMetadataSourceClass GrlMetadataSourceClass;

/**
 * GrlMetadataSourceClass:
 * @parent_class: the parent class structure
 * @supported_operations: the operations that can be called
 * @supported_keys: the list of keys that can be handled
 * @slow_keys: the list of slow keys that can be fetched
 * @key_depends: the list of keys which @key_id depends on
 * @writable_keys: the list of keys which value can be written
 * @resolve: resolve the metadata of a given transfer object
 * @set_metadata: update metadata values for a given object in a
 * permanent fashion
 *
 * Grilo MetadataSource class. Override the vmethods to implement the
 * element functionality.
 */
struct _GrlMetadataSourceClass {

  GrlMediaPluginClass parent_class;

  GrlSupportedOps (*supported_operations) (GrlMetadataSource *source);

  const GList * (*supported_keys) (GrlMetadataSource *source);

  const GList * (*slow_keys) (GrlMetadataSource *source);

  const GList * (*key_depends) (GrlMetadataSource *source, GrlKeyID key_id);

  const GList * (*writable_keys) (GrlMetadataSource *source);

  void (*resolve) (GrlMetadataSource *source,
		   GrlMetadataSourceResolveSpec *rs);

  void (*set_metadata) (GrlMetadataSource *source,
			GrlMetadataSourceSetMetadataSpec *sms);

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
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

GList *grl_metadata_source_filter_writable (GrlMetadataSource *source,
					    GList **keys,
					    gboolean return_filtered);

const GList *grl_metadata_source_key_depends (GrlMetadataSource *source,
                                              GrlKeyID key_id);

const GList *grl_metadata_source_writable_keys (GrlMetadataSource *source);

void grl_metadata_source_resolve (GrlMetadataSource *source,
                                  const GList *keys,
                                  GrlMedia *media,
                                  GrlMetadataResolutionFlags flags,
                                  GrlMetadataSourceResolveCb callback,
                                  gpointer user_data);

GrlMedia *grl_metadata_source_resolve_sync (GrlMetadataSource *source,
                                            const GList *keys,
                                            GrlMedia *media,
                                            GrlMetadataResolutionFlags flags,
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

const gchar *grl_metadata_source_get_id (GrlMetadataSource *source);

const gchar *grl_metadata_source_get_name (GrlMetadataSource *source);

const gchar *grl_metadata_source_get_description (GrlMetadataSource *source);

G_END_DECLS

#endif /* _GRL_METADATA_SOURCE_H_ */
