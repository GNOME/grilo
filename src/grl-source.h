/*
 * Copyright (C) 2012 Igalia S.L.
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

#ifndef _GRL_SOURCE_H_
#define _GRL_SOURCE_H_

#include <grl-metadata-key.h>
#include <grl-media.h>
#include <grl-definitions.h>
#include <grl-plugin.h>
#include <grl-operation-options.h>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

/* Macros */

#define GRL_TYPE_SOURCE                         \
  (grl_source_get_type ())

#define GRL_SOURCE(obj)                         \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),           \
                               GRL_TYPE_SOURCE, \
                               GrlSource))

#define GRL_IS_SOURCE(obj)                         \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),              \
                               GRL_TYPE_SOURCE))

#define GRL_SOURCE_CLASS(klass)                 \
  (G_TYPE_CHECK_CLASS_CAST((klass),             \
                           GRL_TYPE_SOURCE,     \
                           GrlSourceClass))

#define GRL_IS_SOURCE_CLASS(klass)              \
  (G_TYPE_CHECK_CLASS_TYPE((klass),             \
                           GRL_TYPE_SOURCE))

#define GRL_SOURCE_GET_CLASS(obj)               \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),            \
                              GRL_TYPE_SOURCE,  \
                              GrlSourceClass))

typedef struct _GrlSource        GrlSource;
typedef struct _GrlSourcePrivate GrlSourcePrivate;

struct _GrlSource {

  GObject parent;

  /*< private >*/
  GrlSourcePrivate *priv;

  gpointer _grl_reserved[GRL_PADDING];
};

/**
 * GrlSupportedOps:
 * @GRL_OP_NONE: no operation is supported
 * @GRL_OP_RESOLVE: Fetch specific keys of metadata based on other metadata.
 * @GRL_OP_BROWSE: Retrieve complete sets of #GrlMedia
 * @GRL_OP_SEARCH: Look up for #GrlMedia given a search text
 * @GRL_OP_QUERY:  Look up for #GrlMedia give a service specific query
 * @GRL_OP_STORE: Store content in a service
 * @GRL_OP_STORE_PARENT: Store content as child of a certian parent category.
 * @GRL_OP_STORE_METADATA: Update metadata of a #GrlMedia in a service.
 * @GRL_OP_REMOVE: Remove content from a service.
 * @GRL_OP_MEDIA_FROM_URI: Create a #GrlMedia instance from an URI
 * representing a media resource.
 * @GRL_OP_NOTIFY_CHANGE: Notify about changes in the #GrlSource.
 *
 * Bitwise flags which reflect the kind of operations that a
 * #GrlSource supports.
 */
typedef enum {
  GRL_OP_NONE            = 0,
  GRL_OP_RESOLVE         = 1,
  GRL_OP_BROWSE          = 1 << 1,
  GRL_OP_SEARCH          = 1 << 2,
  GRL_OP_QUERY           = 1 << 3,
  GRL_OP_STORE           = 1 << 4,
  GRL_OP_STORE_PARENT    = 1 << 5,
  GRL_OP_STORE_METADATA  = 1 << 6,
  GRL_OP_REMOVE          = 1 << 7,
  GRL_OP_MEDIA_FROM_URI  = 1 << 8,
  GRL_OP_NOTIFY_CHANGE   = 1 << 9
} GrlSupportedOps;

 /**
 * GrlSupportedMedia:
 * @GRL_SUPPORTED_MEDIA_NONE: no media
 * @GRL_SUPPORTED_MEDIA_AUDIO: audio media
 * @GRL_SUPPORTED_MEDIA_VIDEO: video media
 * @GRL_SUPPORTED_MEDIA_IMAGE: image media
 * @GRL_SUPPORTED_MEDIA_ALL: any media
 */
typedef enum {
  GRL_SUPPORTED_MEDIA_NONE  = 0,
  GRL_SUPPORTED_MEDIA_AUDIO = (1 << 0),
  GRL_SUPPORTED_MEDIA_VIDEO = (1 << 1),
  GRL_SUPPORTED_MEDIA_IMAGE = (1 << 2),
  GRL_SUPPORTED_MEDIA_ALL   = (GRL_SUPPORTED_MEDIA_AUDIO | GRL_SUPPORTED_MEDIA_VIDEO | GRL_SUPPORTED_MEDIA_IMAGE)
} GrlSupportedMedia;

/**
 * GrlSourceChangeType:
 * @GRL_CONTENT_CHANGED: content has changed. It is used when any property of
 * #GrlMedia has changed, or in case of containers, if several children have
 * been added and removed.
 * @GRL_CONTENT_ADDED: new content has been added.
 * @GRL_CONTENT_REMOVED: content has been removed
 *
 * Specifies which kind of change has happened in the plugin
 */
typedef enum {
  GRL_CONTENT_CHANGED,
  GRL_CONTENT_ADDED,
  GRL_CONTENT_REMOVED
} GrlSourceChangeType;

/**
 * GrlSourceResolveCb:
 * @source: a source
 * @operation_id: operation identifier
 * @media: (transfer full): a data transfer object
 * @user_data: user data passed to grl_source_resolve()
 * @error: (nullable): possible #GError generated at processing
 *
 * Prototype for the callback passed to grl_source_resolve(). If the URI did
 * not resolve to a valid media record, @media will be %NULL. If there was an
 * error during resolution, @error will be set.
 *
 * If @media is non-%NULL, ownership of it is transferred to the callback, and
 * it must be freed afterwards using g_object_unref().
 */
typedef void (*GrlSourceResolveCb) (GrlSource *source,
                                    guint operation_id,
                                    GrlMedia *media,
                                    gpointer user_data,
                                    const GError *error);

/**
 * GrlSourceResultCb:
 * @source: a source
 * @operation_id: operation identifier
 * @media: (nullable) (transfer full): a data transfer object
 * @remaining: the number of remaining #GrlMedia to process, or
 * GRL_SOURCE_REMAINING_UNKNOWN if it is unknown
 * @user_data: user data passed to the used method
 * @error: (nullable): possible #GError generated at processing
 *
 * Prototype for the callback passed to the media sources' methods
 */
typedef void (*GrlSourceResultCb) (GrlSource *source,
                                   guint operation_id,
                                   GrlMedia *media,
                                   guint remaining,
                                   gpointer user_data,
                                   const GError *error);

/**
 * GrlSourceRemoveCb:
 * @source: a source
 * @media: (transfer full): a data transfer object
 * @user_data: user data passed to grl_source_remove()
 * @error: (nullable): possible #GError generated at processing
 *
 * Prototype for the callback passed to grl_source_remove()
 */
typedef void (*GrlSourceRemoveCb) (GrlSource *source,
                                   GrlMedia *media,
                                   gpointer user_data,
                                   const GError *error);

/**
 * GrlSourceStoreCb:
 * @source: a source
 * @media: a #GrlMedia transfer object
 * @failed_keys: (element-type GrlKeyID) (transfer none): #GList of
 * keys that could not be updated, if any
 * @user_data: user data
 * @error: (nullable): possible #GError generated
 *
 * Prototype for the callback passed to grl_source_store_foo functions
 */
typedef void (*GrlSourceStoreCb) (GrlSource *source,
                                  GrlMedia *media,
                                  GList *failed_keys,
                                  gpointer user_data,
                                  const GError *error);

/**
 * GrlSourceResolveSpec:
 * @source: a source
 * @operation_id: operation identifier
 * @media: a data transfer object
 * @keys: the #GList of #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Data transport structure used internally by the plugins which support
 * resolve vmethod.
 */
typedef struct {
  GrlSource *source;
  guint operation_id;
  GrlMedia *media;
  GList *keys;
  GrlOperationOptions *options;
  GrlSourceResolveCb callback;
  gpointer user_data;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
} GrlSourceResolveSpec;

/**
 * GrlSourceMediaFromUriSpec:
 * @source: a source
 * @operation_id: operation identifier
 * @uri: A URI that can be used to identify a media resource
 * @keys: Metadata keys to resolve
 * @options: options wanted for that operation
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Data transport structure used internally by the plugins which support
 * media_from_uri vmethod.
 */
typedef struct {
  GrlSource *source;
  guint operation_id;
  gchar *uri;
  GList *keys;
  GrlOperationOptions *options;
  GrlSourceResolveCb callback;
  gpointer user_data;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
} GrlSourceMediaFromUriSpec;

/**
 * GrlSourceBrowseSpec:
 * @source: a source
 * @operation_id: operation identifier
 * @container: a container of data transfer objects
 * @keys: the #GList of #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Data transport structure used internally by the plugins which support
 * browse vmethod.
 */
typedef struct {
  GrlSource *source;
  guint operation_id;
  GrlMedia *container;
  GList *keys;
  GrlOperationOptions *options;
    GrlSourceResultCb callback;
  gpointer user_data;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
} GrlSourceBrowseSpec;

/**
 * GrlSourceSearchSpec:
 * @source: a source
 * @operation_id: operation identifier
 * @text: the text to search
 * @keys: the #GList of #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Data transport structure used internally by the plugins which support
 * search vmethod.
 */
typedef struct {
  GrlSource *source;
  guint operation_id;
  gchar *text;
  GList *keys;
  GrlOperationOptions *options;
  GrlSourceResultCb callback;
  gpointer user_data;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
} GrlSourceSearchSpec;

/**
 * GrlSourceQuerySpec:
 * @source: a source
 * @operation_id: operation identifier
 * @query: the query to process
 * @keys: the #GList of #GrlKeyID<!-- -->s to request
 * @options: options wanted for that operation
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Data transport structure used internally by the plugins which support
 * query vmethod.
 */
typedef struct {
  GrlSource *source;
  guint operation_id;
  gchar *query;
  GList *keys;
  GrlOperationOptions *options;
  GrlSourceResultCb callback;
  gpointer user_data;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
} GrlSourceQuerySpec;

/**
 * GrlSourceRemoveSpec:
 * @source: a source
 * @media_id: media identifier to remove
 * @media: a data transfer object
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Data transport structure used internally by the plugins which support
 * store vmethod.
 */
typedef struct {
  GrlSource *source;
  gchar *media_id;
  GrlMedia *media;
  GrlSourceRemoveCb callback;
  gpointer user_data;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
} GrlSourceRemoveSpec;

/**
 * GrlSourceStoreSpec:
 * @source: a media source
 * @parent: a parent to store the data transfer objects
 * @media: a data transfer object
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 *
 * Data transport structure used internally by the plugins which support
 * store vmethod.
 */
typedef struct {
  GrlSource *source;
  GrlMedia *parent;
  GrlMedia *media;
  GrlSourceStoreCb callback;
  gpointer user_data;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
} GrlSourceStoreSpec;

/**
 * GrlSourceStoreMetadataSpec:
 * @source: a source
 * @media: a #GrlMedia transfer object
 * @keys: List of keys to be stored/updated.
 * @flags: Flags to control specific bahviors of the set metadata operation.
 * @callback: the callback passed to grl_source_store_metadata()
 * @user_data: user data passed to grl_source_store_metadata()
 * @failed_keys: for internal use of the framework only.
 *
 * Data transport structure used internally by the plugins which support
 * store_metadata vmethod.
 */
typedef struct {
  GrlSource *source;
  GrlMedia *media;
  GList *keys;
  GrlWriteFlags flags;
  GrlSourceStoreCb callback;
  gpointer user_data;
  GList *failed_keys;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
} GrlSourceStoreMetadataSpec;

/* GrlSource class */

typedef struct _GrlSourceClass GrlSourceClass;

/**
 * GrlSourceClass:
 * @parent_class: the parent class structure
 * @supported_operations: the operations that can be called
 * @supported_keys: the list of keys that can be handled
 * @slow_keys: the list of slow keys that can be fetched
 * @writable_keys: the list of keys which value can be written
 * @get_caps: the capabilities that @source supports for @operation
 * @resolve: resolve the metadata of a given transfer object
 * @may_resolve: return FALSE if it can be known without blocking that @key_id
 * @test_media_from_uri: tests if this source can create #GrlMedia
 * instances from a given URI.
 * @media_from_uri: Creates a #GrlMedia instance representing the media
 * exposed by a certain URI.
 * @browse: browse through a list of media
 * @search: search for media
 * @query: query for a specific media
 * @remove: remove a media from a container
 * @store: store a media in a container
 * @store_metadata: update metadata values for a given object in a
 * permanent fashion
 * @cancel: cancel the current operation
 * @notify_change_start: start emitting signals about changes in content
 * @notify_change_stop: stop emitting signals about changes in content
 *
 * Grilo Source class. Override the vmethods to implement the
 * element functionality.
 */
struct _GrlSourceClass {

  GObjectClass parent_class;

  GrlSupportedOps (*supported_operations) (GrlSource *source);

  const GList * (*supported_keys) (GrlSource *source);

  const GList * (*slow_keys) (GrlSource *source);

  const GList * (*writable_keys) (GrlSource *source);

  GrlCaps * (*get_caps) (GrlSource *source, GrlSupportedOps operation);

  void (*resolve) (GrlSource *source, GrlSourceResolveSpec *ms);

  gboolean (*may_resolve) (GrlSource *source, GrlMedia *media,
                           GrlKeyID key_id, GList **missing_keys);

  gboolean (*test_media_from_uri) (GrlSource *source,
                                   const gchar *uri);

  void (*media_from_uri) (GrlSource *source,
                          GrlSourceMediaFromUriSpec *mfus);

  void (*browse) (GrlSource *source, GrlSourceBrowseSpec *bs);

  void (*search) (GrlSource *source, GrlSourceSearchSpec *ss);

  void (*query) (GrlSource *source, GrlSourceQuerySpec *qs);

  void (*remove) (GrlSource *source, GrlSourceRemoveSpec *rs);

  void (*store) (GrlSource *source, GrlSourceStoreSpec *ss);

  void (*store_metadata) (GrlSource *source, GrlSourceStoreMetadataSpec *sms);

  void (*cancel) (GrlSource *source, guint operation_id);

  gboolean (*notify_change_start) (GrlSource *source,
                                    GError **error);

  gboolean (*notify_change_stop) (GrlSource *source,
                                  GError **error);

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

G_BEGIN_DECLS

GType grl_source_get_type (void);

GrlSupportedOps grl_source_supported_operations (GrlSource *source);

const GList *grl_source_supported_keys (GrlSource *source);

const GList *grl_source_slow_keys (GrlSource *source);

const GList *grl_source_writable_keys (GrlSource *source);

GrlCaps *grl_source_get_caps (GrlSource *source,
                              GrlSupportedOps operation);

void grl_source_set_auto_split_threshold (GrlSource *source,
                                          guint threshold);

guint grl_source_get_auto_split_threshold (GrlSource *source);


guint grl_source_resolve (GrlSource *source,
                          GrlMedia *media,
                          const GList *keys,
                          GrlOperationOptions *options,
                          GrlSourceResolveCb callback,
                          gpointer user_data);

GrlMedia *grl_source_resolve_sync (GrlSource *source,
                                   GrlMedia *media,
                                   const GList *keys,
                                   GrlOperationOptions *options,
                                   GError **error);

gboolean grl_source_may_resolve (GrlSource *source,
                                 GrlMedia *media,
                                 GrlKeyID key_id,
                                 GList **missing_keys);

gboolean grl_source_test_media_from_uri (GrlSource *source,
                                         const gchar *uri);

guint grl_source_get_media_from_uri (GrlSource *source,
                                     const gchar *uri,
                                     const GList *keys,
                                     GrlOperationOptions *options,
                                     GrlSourceResolveCb callback,
                                     gpointer user_data);

GrlMedia *grl_source_get_media_from_uri_sync (GrlSource *source,
                                              const gchar *uri,
                                              const GList *keys,
                                              GrlOperationOptions *options,
                                              GError **error);

guint grl_source_browse (GrlSource *source,
                         GrlMedia *container,
                         const GList *keys,
                         GrlOperationOptions *options,
                         GrlSourceResultCb callback,
                         gpointer user_data);

GList *grl_source_browse_sync (GrlSource *source,
                               GrlMedia *container,
                               const GList *keys,
                               GrlOperationOptions *options,
                               GError **error);

guint grl_source_search (GrlSource *source,
                         const gchar *text,
                         const GList *keys,
                         GrlOperationOptions *options,
                         GrlSourceResultCb callback,
                         gpointer user_data);

GList *grl_source_search_sync (GrlSource *source,
                               const gchar *text,
                               const GList *keys,
                               GrlOperationOptions *options,
                               GError **error);

guint grl_source_query (GrlSource *source,
                        const gchar *query,
                        const GList *keys,
                        GrlOperationOptions *options,
                        GrlSourceResultCb callback,
                        gpointer user_data);

GList *grl_source_query_sync (GrlSource *source,
                              const gchar *query,
                              const GList *keys,
                              GrlOperationOptions *options,
                              GError **error);

void grl_source_remove (GrlSource *source,
                        GrlMedia *media,
                        GrlSourceRemoveCb callback,
                        gpointer user_data);

void grl_source_remove_sync (GrlSource *source,
                             GrlMedia *media,
                             GError **error);

void grl_source_store (GrlSource *source,
                       GrlMedia *parent,
                       GrlMedia *media,
                       GrlWriteFlags flags,
                       GrlSourceStoreCb callback,
                       gpointer user_data);

void grl_source_store_sync (GrlSource *source,
                            GrlMedia *parent,
                            GrlMedia *media,
                            GrlWriteFlags flags,
                            GError **error);

void grl_source_store_metadata (GrlSource *source,
                                GrlMedia *media,
                                GList *keys,
                                GrlWriteFlags flags,
                                GrlSourceStoreCb callback,
                                gpointer user_data);

GList *grl_source_store_metadata_sync (GrlSource *source,
                                       GrlMedia *media,
                                       GList *keys,
                                       GrlWriteFlags flags,
                                       GError **error);

gboolean grl_source_notify_change_start (GrlSource *source,
                                         GError **error);

gboolean grl_source_notify_change_stop (GrlSource *source,
                                        GError **error);

void grl_source_notify_change_list (GrlSource *source,
                                    GPtrArray *changed_medias,
                                    GrlSourceChangeType change_type,
                                    gboolean location_unknown);

void grl_source_notify_change (GrlSource *source,
                               GrlMedia *media,
                               GrlSourceChangeType change_type,
                               gboolean location_unknown);

const gchar *grl_source_get_id (GrlSource *source);

const gchar *grl_source_get_name (GrlSource *source);

GIcon *grl_source_get_icon (GrlSource *source);

const gchar *grl_source_get_description (GrlSource *source);

GrlPlugin *grl_source_get_plugin (GrlSource *source);

gint grl_source_get_rank (GrlSource *source);

GrlSupportedMedia grl_source_get_supported_media (GrlSource *source);

const char ** grl_source_get_tags (GrlSource *source);

G_END_DECLS

#endif /* _GRL_SOURCE_H_ */
