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
 * @GRL_OP_NOTIFY_CHANGE: Notify about changes in the #GrlMediaSource.
 *
 * Bitwise flags which reflect the kind of operations that a
 * #GrlSource supports.
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
  GRL_OP_NOTIFY_CHANGE   = 1 << 10,
} GrlSupportedOps;

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
 * @cancel: cancel the current operation

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

  void (*cancel) (GrlSource *source, guint operation_id);

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

const gchar *grl_source_get_id (GrlSource *source);

const gchar *grl_source_get_name (GrlSource *source);

const gchar *grl_source_get_description (GrlSource *source);

GrlPlugin *grl_source_get_plugin (GrlSource *source);

gint grl_source_get_rank (GrlSource *source);

G_END_DECLS

#endif /* _GRL_SOURCE_H_ */
