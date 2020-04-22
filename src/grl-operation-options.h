/*
 * Copyright (C) 2011 Igalia S.L.
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

#include <grl-caps.h>
#include <grl-metadata-key.h>

#if !defined (_GRL_OPERATION_OPTIONS_H_)
#define _GRL_OPERATION_OPTIONS_H_

G_BEGIN_DECLS

typedef struct _GrlOperationOptionsPrivate GrlOperationOptionsPrivate;

typedef struct {
  GObject parent;

  /*< private >*/
  GrlOperationOptionsPrivate *priv;

  gpointer _grl_reserved[GRL_PADDING_SMALL];
} GrlOperationOptions;

/**
 * GrlOperationOptionsClass:
 * @parent: the parent class structure
 *
 * Grilo Operation Options class.
 */
typedef struct {
  GObjectClass parent;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
} GrlOperationOptionsClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlOperationOptions, g_object_unref)

#define GRL_TYPE_OPERATION_OPTIONS (grl_operation_options_get_type ())
#define GRL_OPERATION_OPTIONS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRL_TYPE_OPERATION_OPTIONS, GrlOperationOptions))
#define GRL_OPERATION_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GRL_TYPE_OPERATION_OPTIONS, GrlOperationOptionsClass))
#define GRL_IS_OPERATION_OPTIONS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRL_TYPE_OPERATION_OPTIONS))
#define GRL_IS_OPERATION_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRL_TYPE_OPERATION_OPTIONS))
#define GRL_OPERATION_OPTIONS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GRL_TYPE_OPERATION_OPTIONS, GrlOperationOptionsClass))

/**
 * GrlResolutionFlags:
 * @GRL_RESOLVE_NORMAL: Normal mode.
 * @GRL_RESOLVE_FULL: Try other plugins if necessary.
 * @GRL_RESOLVE_IDLE_RELAY: Use idle loop to relay results.
 * @GRL_RESOLVE_FAST_ONLY: Only resolve fast metadata keys.
 *
 * Resolution flags
 */
typedef enum {
  GRL_RESOLVE_NORMAL     = 0,        /* Normal mode */
  GRL_RESOLVE_FULL       = (1 << 0), /* Try other plugins if necessary */
  GRL_RESOLVE_IDLE_RELAY = (1 << 1), /* Use idle loop to relay results */
  GRL_RESOLVE_FAST_ONLY  = (1 << 2)  /* Only resolve fast metadata keys */
} GrlResolutionFlags;

/**
 * GrlWriteFlags:
 * @GRL_WRITE_NORMAL: Normal mode.
 * @GRL_WRITE_FULL: Try other plugins if necessary.
 *
 * Flags for writing operations.
 */
typedef enum {
  GRL_WRITE_NORMAL     = 0,        /* Normal mode */
  GRL_WRITE_FULL       = (1 << 0)  /* Try other plugins if necessary */
} GrlWriteFlags;

#define GRL_COUNT_INFINITY (-1)

_GRL_EXTERN
GType grl_operation_options_get_type (void);

_GRL_EXTERN
GrlOperationOptions *grl_operation_options_new (GrlCaps *caps);

_GRL_EXTERN
gboolean grl_operation_options_obey_caps (GrlOperationOptions *options,
                                          GrlCaps *caps,
                                          GrlOperationOptions **supported_options,
                                          GrlOperationOptions **unsupported_options);

_GRL_EXTERN
GrlOperationOptions *grl_operation_options_copy (GrlOperationOptions *options);

_GRL_EXTERN
gboolean grl_operation_options_set_skip (GrlOperationOptions *options, guint skip);
_GRL_EXTERN
guint grl_operation_options_get_skip (GrlOperationOptions *options);

_GRL_EXTERN
gboolean grl_operation_options_set_count (GrlOperationOptions *options, gint count);
_GRL_EXTERN
gint grl_operation_options_get_count (GrlOperationOptions *options);

_GRL_EXTERN
gboolean grl_operation_options_set_resolution_flags (GrlOperationOptions *options,
                                                     GrlResolutionFlags flags);
_GRL_EXTERN
GrlResolutionFlags
    grl_operation_options_get_resolution_flags (GrlOperationOptions *options);

_GRL_EXTERN
gboolean grl_operation_options_set_type_filter (GrlOperationOptions *options,
                                                GrlTypeFilter filter);

_GRL_EXTERN
GrlTypeFilter grl_operation_options_get_type_filter (GrlOperationOptions *options);

_GRL_EXTERN
gboolean grl_operation_options_set_key_filter_value (GrlOperationOptions *options,
                                                     GrlKeyID key,
                                                     GValue *value);

_GRL_EXTERN
gboolean grl_operation_options_set_key_filters (GrlOperationOptions *options,
                                                ...) G_GNUC_NULL_TERMINATED;

_GRL_EXTERN
gboolean grl_operation_options_set_key_filter_dictionary (GrlOperationOptions *options,
                                                          GHashTable *filters);

_GRL_EXTERN
GValue *grl_operation_options_get_key_filter (GrlOperationOptions *options,
                                              GrlKeyID key);

_GRL_EXTERN
GList *grl_operation_options_get_key_filter_list (GrlOperationOptions *options);

_GRL_EXTERN
gboolean grl_operation_options_set_key_range_filter_value (GrlOperationOptions *options,
                                                           GrlKeyID key,
                                                           GValue *min_value,
                                                           GValue *max_value);

_GRL_EXTERN
gboolean grl_operation_options_set_key_range_filter (GrlOperationOptions *options,
                                                     ...);

_GRL_EXTERN
void grl_operation_options_get_key_range_filter (GrlOperationOptions *options,
                                                 GrlKeyID key,
                                                 GValue **min_value,
                                                 GValue **max_value);


_GRL_EXTERN
GList *grl_operation_options_get_key_range_filter_list (GrlOperationOptions *options);

G_END_DECLS

#endif /* _GRL_OPERATION_OPTIONS_H_ */
