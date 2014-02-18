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

#include "grl-operation.h"
#include "grl-operation-priv.h"
#include "grl-log.h"

typedef struct
{
  GrlOperationCancelCb cancel_cb;
  GDestroyNotify       destroy_cb;
  gpointer             private_data;
  gpointer             user_data;
  GDestroyNotify       user_data_destroy_func;
} OperationData;

static guint       operations_id;
static GHashTable *operations;

static void
operation_data_free (OperationData *data)
{
  if (data->user_data_destroy_func && data->user_data) {
    data->user_data_destroy_func (data->user_data);
    data->user_data = NULL;
  }

  if (data->destroy_cb) {
    data->destroy_cb (data->private_data);
  }

  g_slice_free (OperationData, data);
}

void
grl_operation_init (void)
{
  static gboolean initialized = FALSE;

  if (G_LIKELY (initialized))
    return;

  initialized = TRUE;
  operations = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                      NULL,
                                      (GDestroyNotify) operation_data_free);
  operations_id = 1;
}

guint
grl_operation_generate_id (void)
{
  guint operation_id = operations_id++;
  OperationData *data = g_slice_new0 (OperationData);

  g_hash_table_insert (operations, GUINT_TO_POINTER (operation_id), data);

  return operation_id;
}

void
grl_operation_set_private_data (guint                operation_id,
                                gpointer             private_data,
                                GrlOperationCancelCb cancel_cb,
                                GDestroyNotify       destroy_cb)
{
  OperationData *data = g_hash_table_lookup (operations,
                                             GUINT_TO_POINTER (operation_id));

  g_return_if_fail (data != NULL);

  data->cancel_cb    = cancel_cb;
  data->destroy_cb   = destroy_cb;
  data->private_data = private_data;
}

/**
 * grl_operation_get_private_data: (skip)
 * @operation_id: operation identifier
 */
gpointer
grl_operation_get_private_data (guint operation_id)
{
  OperationData *data = g_hash_table_lookup (operations,
                                             GUINT_TO_POINTER (operation_id));

  g_return_val_if_fail (data != NULL, NULL);

  return data->private_data;
}

void
grl_operation_remove (guint operation_id)
{
  g_hash_table_remove (operations, GUINT_TO_POINTER (operation_id));
}

/*** PUBLIC API ***/

/**
 * grl_operation_cancel:
 * @operation_id: the identifier of a running operation
 *
 * Cancel an operation.
 */
void
grl_operation_cancel (guint operation_id)
{
  OperationData *data = g_hash_table_lookup (operations,
                                             GUINT_TO_POINTER (operation_id));

  if (!data) {
    GRL_WARNING ("Invalid operation %u", operation_id);
    return;
  }

  g_return_if_fail (data != NULL);

  if (data->cancel_cb) {
    data->cancel_cb (data->private_data);
  }
}

/**
 * grl_operation_get_data:
 * @operation_id: the identifier of a running operation
 *
 * Obtains the previously attached data
 *
 * Returns: (transfer none): The previously attached data.
 */
gpointer
grl_operation_get_data (guint operation_id)
{
  OperationData *data = g_hash_table_lookup (operations,
                                             GUINT_TO_POINTER (operation_id));

  if (!data) {
    GRL_WARNING ("Invalid operation %u", operation_id);
    return NULL;
  }

  return data->user_data;
}

/**
 * grl_operation_set_data:
 * @operation_id: the identifier of a running operation
 * @user_data: the data to attach
 *
 * Attach a pointer to the specific operation.
 */
void
grl_operation_set_data (guint operation_id, gpointer user_data)
{
  grl_operation_set_data_full (operation_id, user_data, NULL);
}

/**
 * grl_operation_set_data_full:
 * @operation_id: the identifier of a running operation
 * @user_data: (allow-none): the data to attach
 * @destroy_func: (allow-none): function to release @user_data when the operation terminates
 *
 * Attach a pointer to the specific operation.
 *
 * Note that the @destroy_func callback is not called if @user_data is %NULL.
 *
 * Since: 0.2.7
 */
void
grl_operation_set_data_full (guint operation_id, gpointer user_data, GDestroyNotify destroy_func)
{
  OperationData *data = g_hash_table_lookup (operations,
                                             GUINT_TO_POINTER (operation_id));

  if (!data) {
    GRL_WARNING ("Invalid operation %u", operation_id);
  } else {
    if (data->user_data_destroy_func && data->user_data)
      data->user_data_destroy_func (data->user_data);

    data->user_data = user_data;
    data->user_data_destroy_func = destroy_func;
  }
}

