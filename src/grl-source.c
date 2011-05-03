/*
 * Copyright (C) 2010-2012 Igalia S.L.
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

/**
 * SECTION:grl-source
 * @short_description: Abstract base class for sources
 * @see_also: #GrlPlugin, #GrlMediaSource, #GrlMetadataSource, #GrlMedia
 *
 * GrlSource is the abstract base class needed to construct a source providing
 * multimedia information that can be used in a Grilo application.
 *
 * The sources fetch information from different online or local
 * databases and store them in the #GrlMedia.
 */

#include "grl-source.h"
#include "grl-source-priv.h"
#include "grl-metadata-source.h"
#include "grl-operation.h"
#include "grl-operation-priv.h"
#include "grl-sync-priv.h"
#include "grl-plugin-registry.h"
#include "grl-error.h"
#include "grl-log.h"
#include "data/grl-media.h"

#include <string.h>

#define GRL_LOG_DOMAIN_DEFAULT  source_log_domain
GRL_LOG_DOMAIN(source_log_domain);

#define GRL_SOURCE_GET_PRIVATE(object)                          \
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                        \
                               GRL_TYPE_SOURCE,                 \
                               GrlSourcePrivate))

enum {
  PROP_0,
  PROP_ID,
  PROP_NAME,
  PROP_DESC,
  PROP_PLUGIN,
  PROP_RANK,
};

struct _GrlSourcePrivate {
  gchar *id;
  gchar *name;
  gchar *desc;
  gint rank;
  GrlPlugin *plugin;
};

struct OperationState {
  GrlSource *source;
  guint operation_id;
  gboolean cancelled;
  gboolean completed;
};

static void grl_source_finalize (GObject *object);

static void grl_source_dispose (GObject *object);

static void grl_source_get_property (GObject *object,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec);

static void grl_source_set_property (GObject *object,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec);

/* ================ GrlSource GObject ================ */

G_DEFINE_ABSTRACT_TYPE (GrlSource,
                        grl_source,
                        G_TYPE_OBJECT);

static void
grl_source_class_init (GrlSourceClass *source_class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (source_class);

  gobject_class->dispose = grl_source_dispose;
  gobject_class->finalize = grl_source_finalize;
  gobject_class->set_property = grl_source_set_property;
  gobject_class->get_property = grl_source_get_property;

  /**
   * GrlSource:source-id
   *
   * The identifier of the source.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_ID,
                                   g_param_spec_string ("source-id",
                                                        "Source identifier",
                                                        "The identifier of the source",
                                                        "",
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  /**
   * GrlSource:source-name
   *
   * The name of the source.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_NAME,
                                   g_param_spec_string ("source-name",
                                                        "Source name",
                                                        "The name of the source",
                                                        "",
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  /**
   * GrlSource:source-desc
   *
   * A description of the source
   */
  g_object_class_install_property (gobject_class,
                                   PROP_DESC,
                                   g_param_spec_string ("source-desc",
                                                        "Source description",
                                                        "A description of the source",
                                                        "",
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  /**
   * GrlSource:plugin
   *
   * Plugin the source belongs to
   */
  g_object_class_install_property (gobject_class,
                                   PROP_PLUGIN,
                                   g_param_spec_object ("plugin",
                                                        "Plugin",
                                                        "Plugin source belongs to",
                                                        GRL_TYPE_PLUGIN,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  /**
   * GrlSource:rank
   *
   * Source rank
   */
  g_object_class_install_property (gobject_class,
                                   PROP_RANK,
                                   g_param_spec_int ("rank",
                                                     "Rank",
                                                     "Source rank",
                                                     G_MININT,
                                                     G_MAXINT,
                                                     GRL_RANK_DEFAULT,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT |
                                                     G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (source_class,
                            sizeof (GrlSourcePrivate));
}

static void
grl_source_init (GrlSource *source)
{
  source->priv = GRL_SOURCE_GET_PRIVATE (source);
}

static void
grl_source_dispose (GObject *object)
{
  GrlSource *source = GRL_SOURCE (object);

  if (source->priv->plugin) {
    g_object_unref (source->priv->plugin);
    source->priv->plugin = NULL;
  }

  G_OBJECT_CLASS (grl_source_parent_class)->dispose (object);
}

static void
grl_source_finalize (GObject *object)
{
  GrlSource *source = GRL_SOURCE (object);

  g_free (source->priv->id);
  g_free (source->priv->name);
  g_free (source->priv->desc);

  G_OBJECT_CLASS (grl_source_parent_class)->finalize (object);
}

static void
set_string_property (gchar **property, const GValue *value)
{
  if (*property) {
    g_free (*property);
  }
  *property = g_value_dup_string (value);
}

static void
grl_source_set_property (GObject *object,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *pspec)
{
  GrlSource *source = GRL_SOURCE (object);

  switch (prop_id) {
  case PROP_ID:
    set_string_property (&source->priv->id, value);
    break;
  case PROP_NAME:
    set_string_property (&source->priv->name, value);
    break;
  case PROP_DESC:
    set_string_property (&source->priv->desc, value);
    break;
  case PROP_PLUGIN:
    if (source->priv->plugin) {
      g_object_unref (source->priv->plugin);
    }
    source->priv->plugin = g_value_dup_object (value);
    break;
  case PROP_RANK:
    source->priv->rank = g_value_get_int (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (source, prop_id, pspec);
    break;
  }
}

static void
grl_source_get_property (GObject *object,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *pspec)
{
  GrlSource *source = GRL_SOURCE (object);

  switch (prop_id) {
  case PROP_ID:
    g_value_set_string (value, source->priv->id);
    break;
  case PROP_NAME:
    g_value_set_string (value, source->priv->name);
    break;
  case PROP_DESC:
    g_value_set_string (value, source->priv->desc);
    break;
  case PROP_PLUGIN:
    g_value_set_object (value, source->priv->plugin);
    break;
  case PROP_RANK:
    g_value_set_int (value, source->priv->rank);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (source, prop_id, pspec);
    break;
  }
}

/* ================ Utilities ================ */

/*
 * This method will _intersect two key lists_:
 *
 * @keys_to_filter: user provided set we want to filter leaving only
 * the keys that intersects with the @source_keys set.
 *
 * @source_keys: the %GrlSource<!-- -->'s key set if
 * @return_filtered is %TRUE a copy of the filtered set *complement*
 * will be returned (a list of the filtered out keys).
 */
static GList *
filter_key_list (GrlSource *source,
                 GList **keys_to_filter,
                 gboolean return_filtered,
                 GList *source_keys)
{
  GList *iter_keys, *found;
  GList *in_source = NULL;
  GList *out_source = NULL;

  for (iter_keys = *keys_to_filter;
       iter_keys;
       iter_keys = g_list_next (iter_keys)) {
    found = g_list_find (source_keys, iter_keys->data);
    if (found) {
      in_source = g_list_prepend (in_source, iter_keys->data);
    } else {
      if (return_filtered) {
        out_source = g_list_prepend (out_source, iter_keys->data);
      }
    }
  }

  g_list_free (*keys_to_filter);
  *keys_to_filter = g_list_reverse (in_source);

  return g_list_reverse (out_source);
}

/* ================ API ================ */

/**
 * grl_source_supported_keys:
 * @source: a source
 *
 * Get a list of #GrlKeyID, which describe a metadata types that this
 * source can fetch and store.
 *
 * Returns: (element-type GrlKeyID) (transfer none): a #GList with the keys
 */
const GList *
grl_source_supported_keys (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  if (GRL_SOURCE_GET_CLASS (source)->supported_keys) {
    return GRL_SOURCE_GET_CLASS (source)->supported_keys (source);
  } else {
    return NULL;
  }
}

/**
 * grl_source_slow_keys:
 * @source: a source
 *
 * Similar to grl_source_supported_keys(), but these keys
 * are marked as slow because of the amount of traffic/processing needed
 * to fetch them.
 *
 * Returns: (element-type GrlKeyID) (transfer none): a #GList with the keys
 */
const GList *
grl_source_slow_keys (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  if (GRL_SOURCE_GET_CLASS (source)->slow_keys) {
    return GRL_SOURCE_GET_CLASS (source)->slow_keys (source);
  } else {
    return NULL;
  }
}

/**
 * grl_source_writable_keys:
 * @source: a source
 *
 * Similar to grl_source_supported_keys(), but these keys
 * are marked as writable, meaning the source allows the client
 * to provide new values for these keys that will be stored permanently.
 *
 * Returns: (element-type GrlKeyID) (transfer none):
 * a #GList with the keys
 */
const GList *
grl_source_writable_keys (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  if (GRL_SOURCE_GET_CLASS (source)->writable_keys) {
    return GRL_SOURCE_GET_CLASS (source)->writable_keys (source);
  } else {
    return NULL;
  }
}

/**
 * grl_source_filter_supported:
 * @source: a source
 * @keys: (element-type GrlKeyID) (transfer container) (allow-none) (inout):
 * the list of keys to filter out
 * @return_filtered: if %TRUE the return value shall be a new list with
 * the unsupported keys
 *
 * Compares the received @keys list with the supported key list by the
 * @source, and deletes those keys which are not supported.
 *
 * Returns: (element-type GrlKeyID) (transfer container):
 * if @return_filtered is %TRUE will return the list of removed keys;
 * otherwise %NULL
 */
GList *
grl_source_filter_supported (GrlSource *source,
                             GList **keys,
                             gboolean return_filtered)
{
  const GList *supported_keys;

  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  supported_keys = grl_source_supported_keys (source);

  return filter_key_list (source, keys, return_filtered, (GList *) supported_keys);
}

/**
 * grl_source_filter_slow:
 * @source: a source
 * @keys: (element-type GrlKeyID) (transfer container) (allow-none) (inout):
 * the list of keys to filter out
 * @return_filtered: if %TRUE the return value shall be a new list with
 * the slow keys
 *
 * This function does the opposite of other filter functions: removes the slow
 * keys from @keys. If @return_filtered is %TRUE the removed slow keys are
 * returned in a new list.
 *
 * Returns: (element-type GrlKeyID) (transfer container): if
 * @return_filtered is %TRUE will return the list of slow keys; otherwise
 * %NULL
 */
GList *
grl_source_filter_slow (GrlSource *source,
                        GList **keys,
                        gboolean return_filtered)
{
  const GList *slow_keys;
  GList *fastest_keys, *tmp;

  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  slow_keys = grl_source_slow_keys (source);

  /* Note that we want to do the opposite */
  fastest_keys = filter_key_list (source, keys, TRUE, (GList *) slow_keys);
  tmp = *keys;
  *keys = fastest_keys;

  if (!return_filtered) {
    g_list_free (tmp);
    return NULL;
  } else {
    return tmp;
  }
}

/**
 * grl_source_filter_writable:
 * @source: a source
 * @keys: (element-type GrlKeyID) (transfer container) (allow-none) (inout):
 * the list of keys to filter out
 * @return_filtered: if %TRUE the return value shall be a new list with
 * the non-writable keys
 *
 * Similar to grl_source_filter_supported() but applied to the writable keys in
 * grl_source_writable_keys().
 *
 * Filter the @keys list keeping only those keys that are writtable in
 * @source. If @return_filtered is %TRUE then the removed keys are returned in a
 * new list.
 *
 * Returns: (element-type GrlKeyID) (transfer container):
 * if @return_filtered is %TRUE will return the list of non-writtable keys;
 * otherwise %NULL
 */
GList *
grl_source_filter_writable (GrlSource *source,
                            GList **keys,
                            gboolean return_filtered)
{
  const GList *writable_keys;

  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);
  g_return_val_if_fail (keys != NULL, NULL);

  writable_keys = grl_source_writable_keys (source);

  return filter_key_list (source, keys, return_filtered, (GList *) writable_keys);
}

/**
 * grl_source_get_id:
 * @source: a source
 *
 * Returns: the ID of the @source
 */
const gchar *
grl_source_get_id (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  return source->priv->id;
}

/**
 * grl_source_get_name:
 * @source: a source
 *
 * Returns: the name of the @source
 */
const gchar *
grl_source_get_name (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  return source->priv->name;
}

/**
 * grl_source_get_description:
 * @source: a source
 *
 * Returns: the description of the @source
 */
const gchar *
grl_source_get_description (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  return source->priv->desc;
}

/**
 * grl_source_get_plugin:
 * @source: a source
 *
 * Returns: (transfer none): the plugin this source belongs to
 **/
GrlPlugin *
grl_source_get_plugin (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), NULL);

  return source->priv->plugin;
}

/**
 * grl_source_get_rank:
 * @source: a source
 *
 * Gets the source rank
 *
 * Returns: rank value
 **/
gint
grl_source_get_rank (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), 0);

  return source->priv->rank;
}

/**
 * grl_source_supported_operations:
 * @source: a source
 *
 * By default the derived objects of #GrlSource can only resolve.
 *
 * Returns: (type uint): a bitwise mangle with the supported operations by
 * the source
 */
GrlSupportedOps
grl_source_supported_operations (GrlSource *source)
{
  g_return_val_if_fail (GRL_IS_SOURCE (source), GRL_OP_NONE);

  if (GRL_SOURCE_GET_CLASS (source)->supported_operations) {
    return GRL_SOURCE_GET_CLASS (source)->supported_operations (source);
  } else {
    return GRL_OP_NONE;
  }
}

/*
 * Operation states:
 *
 * - finished: We have already emitted the last result to the user
 *
 * - completed: We have already received the last result in the relay
 *              cb (If it is finished it is also completed).
 *
 * - cancelled: Operation valid (not finished) but was cancelled.
 *
 * - ongoing: if the operation is valid (not finished) and not
 *   cancelled.
 */

/*
 * grl_source_set_operation_finished:
 *
 * Sets operation as finished (we have already emitted the last result
 * to the user).
 */
void
grl_source_set_operation_finished (GrlSource *source,
                                   guint operation_id)
{
  GRL_DEBUG ("grl_source_set_operation_finished (%d)", operation_id);

  grl_operation_remove (operation_id);
}

/*
 * grl_source_operation_is_finished:
 *
 * Checks if operation is finished (we have already emitted the last
 * result to the user).
 */
gboolean
grl_source_operation_is_finished (GrlSource *source,
                                  guint operation_id)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (operation_id);

  return op_state == NULL;
}

/*
 * grl_source_set_operation_completed:
 *
 * Sets the operation as completed (we have already received the last
 * result in the relay cb. If it is finsihed it is also completed).
 */
void
grl_source_set_operation_completed (GrlSource *source,
                                    guint operation_id)
{
  struct OperationState *op_state;

  GRL_DEBUG ("grl_source_set_operation_completed (%d)", operation_id);

  op_state = grl_operation_get_private_data (operation_id);

  if (op_state) {
    op_state->completed = TRUE;
  }
}

/*
 * grl_source_operation_is_completed:
 *
 * Checks if operation is completed (we have already received the last
 * result in the relay cb. A finished operation is also a completed
 * operation).
 */
gboolean
grl_source_operation_is_completed (GrlSource *source,
                                   guint operation_id)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (operation_id);

  return !op_state || op_state->completed;
}

/*
 * grl_source_set_operation_cancelled:
 *
 * Sets the operation as cancelled (a valid operation, i.e., not
 * finished, was cancelled)
 */
void
grl_source_set_operation_cancelled (GrlSource *source,
                                    guint operation_id)
{
  struct OperationState *op_state;

  GRL_DEBUG ("grl_source_set_operation_cancelled (%d)", operation_id);

  op_state = grl_operation_get_private_data (operation_id);

  if (op_state) {
    op_state->cancelled = TRUE;
  }
}

/*
 * grl_source_operation_is_cancelled:
 *
 * Checks if operation is cancelled (a valid operation that was
 * cancelled).
 */
gboolean
grl_source_operation_is_cancelled (GrlSource *source,
                                   guint operation_id)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (operation_id);

  return op_state && op_state->cancelled;
}

static void
grl_source_cancel_cb (struct OperationState *op_state)
{
  GrlSource *source = op_state->source;

  if (!grl_source_operation_is_ongoing (source,
                                        op_state->operation_id)) {
    GRL_DEBUG ("Tried to cancel invalid or already cancelled operation. "
               "Skipping...");
    return;
  }

  /* Mark the operation as finished, if the source does not implement
     cancellation or it did not make it in time, we will not emit the results
     for this operation in any case.  At any rate, we will not free the
     operation data until we are sure the plugin won't need it any more. In the
     case of operations dealing with multiple results, like browse() or
     search(), this will happen when it emits remaining = 0 (which can be
     because it did not cancel the op or because it managed to cancel it and is
     signaling so) */
  grl_source_set_operation_cancelled (source,
                                      op_state->operation_id);

  /* If the source provides an implementation for operation cancellation,
     let's use that to avoid further unnecessary processing in the plugin */
  if (GRL_SOURCE_GET_CLASS (source)->cancel) {
    GRL_SOURCE_GET_CLASS (source)->cancel (source,
                                           op_state->operation_id);
  }
}

/*
 * grl_source_set_operation_ongoing:
 *
 * Sets the operation as ongoing (operation is valid, not finished and
 * not cancelled)
 */
void
grl_source_set_operation_ongoing (GrlSource *source,
                                  guint operation_id)
{
  struct OperationState *op_state;

  GRL_DEBUG ("grl_source_set_operation_ongoing (%d)", operation_id);

  op_state = g_new0 (struct OperationState, 1);
  op_state->source = source;
  op_state->operation_id = operation_id;

  grl_operation_set_private_data (operation_id,
                                  op_state,
                                  (GrlOperationCancelCb) grl_source_cancel_cb,
                                  g_free);
}

/*
 * grl_source_operation_is_ongoing:
 *
 * Checks if operation is ongoing (operation is valid, and it is not
 * finished nor cancelled).
 */
gboolean
grl_source_operation_is_ongoing (GrlSource *source,
                                 guint operation_id)
{
  struct OperationState *op_state;

  op_state = grl_operation_get_private_data (operation_id);

  return op_state && !op_state->cancelled;
}

/**
 * grl_source_get_caps:
 * @source: a source
 * @operation: a supported operation. Even though the type allows to specify
 * several operations, only one should be provided here.
 *
 * Get the capabilities of @source for @operation.
 *
 * Returns: (transfer none): The capabilities
 */
GrlCaps *
grl_source_get_caps (GrlSource *source,
                     GrlSupportedOps operation)
{
  static GrlCaps *default_caps = NULL;
  GrlSourceClass *klass = GRL_SOURCE_GET_CLASS (source);

  if (klass->get_caps)
    return klass->get_caps (source, operation);

  if (!default_caps)
    default_caps = grl_caps_new ();

  return default_caps;
}
