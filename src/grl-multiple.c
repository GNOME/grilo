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

/**
 * SECTION:grl-multiple
 * @short_description: Search in multiple loaded sources
 * @see_also: #GrlPlugin, #GrlSource
 *
 * These helper functions are due to ease the search in multiple sources.
 * You can specify the list of sources to use for the searching. Those sources
 * must have enabled the search capability.
 *
 * Also you can set %NULL that sources list, so the function will use all
 * the available sources with the search capability.
 */

#include "grl-multiple.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "grl-sync-priv.h"
#include "grl-operation.h"
#include "grl-operation-priv.h"
#include "grl-registry.h"
#include "grl-error.h"
#include "grl-log.h"

#include <glib/gi18n-lib.h>

#define GRL_LOG_DOMAIN_DEFAULT  multiple_log_domain
GRL_LOG_DOMAIN(multiple_log_domain);

struct MultipleSearchData {
  GHashTable *table;
  guint remaining;
  GList *search_ids;
  GList *sources;
  GList *keys;
  guint search_id;
  gboolean cancelled;
  guint pending;
  guint sources_done;
  guint sources_count;
  GList *sources_more;
  gchar *text;
  GrlOperationOptions *options;
  GrlSourceResultCb user_callback;
  gpointer user_data;
};

struct ResultCount {
  guint count;
  guint remaining;
  guint received;
  guint skip;
};

struct CallbackData {
  GrlSourceResultCb user_callback;
  gpointer user_data;
};

struct MediaFromUriCallbackData {
  GList/*<unowned GrlSource>*/ *sources;  /* owned */
  GList/*<unowned GrlSource>*/ *iter;  /* unowned */
  gchar *uri;  /* owned */
  GList/*<int>*/ *keys;  /* owned */
  GrlOperationOptions *options;  /* owned */
  GrlSourceResolveCb user_callback;
  gpointer user_data;
};

static void multiple_search_cb (GrlSource *source,
                                guint search_id,
                                GrlMedia *media,
                                guint remaining,
                                gpointer user_data,
                                const GError *error);

static void multiple_search_cancel_cb (struct MultipleSearchData *msd);

/* ================ Utitilies ================ */

static void
free_multiple_search_data (struct MultipleSearchData *msd)
{
  GRL_DEBUG ("free_multiple_search_data");
  g_hash_table_unref (msd->table);
  g_list_free (msd->search_ids);
  g_list_free (msd->sources);
  g_list_free (msd->sources_more);
  g_list_free (msd->keys);
  g_object_unref (msd->options);
  g_free (msd->text);
  g_free (msd);
}

static gboolean
confirm_cancel_idle (gpointer user_data)
{
  struct MultipleSearchData *msd = (struct MultipleSearchData *) user_data;
  msd->user_callback (NULL, msd->search_id, NULL, 0, msd->user_data, NULL);
  return FALSE;
}

static gboolean
handle_no_searchable_sources_idle (gpointer user_data)
{
  GError *error;
  struct CallbackData *callback_data = (struct CallbackData *) user_data;

  error = g_error_new (GRL_CORE_ERROR, GRL_CORE_ERROR_SEARCH_FAILED,
                       _("No searchable sources available"));
  callback_data->user_callback (NULL, 0, NULL, 0, callback_data->user_data, error);

  g_error_free (error);
  g_free (callback_data);

  return FALSE;
}

static void
handle_no_searchable_sources (GrlSourceResultCb callback, gpointer user_data)
{
  struct CallbackData *callback_data = g_new0 (struct CallbackData, 1);
  guint id;
  callback_data->user_callback = callback;
  callback_data->user_data = user_data;
  id = g_idle_add (handle_no_searchable_sources_idle, callback_data);
  g_source_set_name_by_id (id, "[grilo] handle_no_searchable_sources_idle");
}

static struct MultipleSearchData *
start_multiple_search_operation (guint search_id,
				 const GList *sources,
				 const gchar *text,
				 const GList *keys,
				 const GList *skip_counts,
				 gint count,
				 GrlOperationOptions *options,
				 GrlSourceResultCb user_callback,
				 gpointer user_data)
{
  GRL_DEBUG ("start_multiple_search_operation");

  struct MultipleSearchData *msd;
  GList *iter_sources, *iter_skips;
  guint n;
  gint first_count, individual_count;

  /* Prepare data required to execute the operation */
  msd = g_new0 (struct MultipleSearchData, 1);
  msd->table = g_hash_table_new_full (g_direct_hash, g_direct_equal,
				      NULL, g_free);
  msd->remaining =
      (count == GRL_COUNT_INFINITY) ? GRL_COUNT_INFINITY : (count - 1);
  msd->search_id = search_id;
  msd->text = g_strdup (text);
  msd->keys = g_list_copy ((GList *) keys);
  msd->options = g_object_ref (options);
  msd->user_callback = user_callback;
  msd->user_data = user_data;

  /* Compute the # of items to request by each source */
  n = g_list_length ((GList *) sources);
  if (count == GRL_COUNT_INFINITY) {
    individual_count = GRL_COUNT_INFINITY;
    first_count = GRL_COUNT_INFINITY;
  } else {
    individual_count = count / n;
    first_count = individual_count + count % n;
  }

  /* Issue search operations on each source */
  iter_sources = (GList *) sources;
  iter_skips = (GList *) skip_counts;
  n = 0;
  while (iter_sources) {
    GrlSource *source;
    guint c, id;
    struct ResultCount *rc;
    guint skip;

    source = GRL_SOURCE (iter_sources->data);

    /* c is the count to use for this source */
    c = (n == 0) ? first_count : individual_count;
    n++;

    /* Only interested in sources with c != 0 */
    if (c != 0) {
      GrlOperationOptions *source_options = NULL;
      GrlCaps *source_caps;

      /* We use ResultCount to keep track of results emitted by this source */
      rc = g_new0 (struct ResultCount, 1);
      rc->count = c;
      g_hash_table_insert (msd->table, source, rc);

      /* Check if we have to apply a "skip" parameter to this source
	 (useful when we are chaining queries to complete the result count) */
      if (iter_skips) {
	skip = GPOINTER_TO_INT (iter_skips->data);
      } else {
	skip = 0;
      }

      source_caps = grl_source_get_caps (source, GRL_OP_SEARCH);
      grl_operation_options_obey_caps (options, source_caps, &source_options, NULL);
      grl_operation_options_set_skip (source_options, skip);
      grl_operation_options_set_count (source_options, rc->count);

      /* Execute the search on this source */
      id = grl_source_search (source,
				    msd->text,
				    msd->keys,
				    source_options,
				    multiple_search_cb,
				    msd);

      GRL_DEBUG ("Operation %s:%u: Searching %u items from offset %u",
                 grl_source_get_name (GRL_SOURCE (source)),
                 id, rc->count, skip);

      g_object_unref (source_options);

      /* Keep track of this operation and this source */
      msd->search_ids = g_list_prepend (msd->search_ids, GINT_TO_POINTER (id));
      msd->sources = g_list_prepend (msd->sources, source);
      msd->sources_count++;
    }

    /* Move to the next source */
    iter_sources = g_list_next (iter_sources);
    iter_skips = g_list_next (iter_skips);
  }

  /* This frees the previous msd structure (if this operation is chained) */
  grl_operation_set_private_data (msd->search_id,
                                  msd,
                                  (GrlOperationCancelCb) multiple_search_cancel_cb,
                                  (GDestroyNotify) free_multiple_search_data);

  return msd;
}

static struct MultipleSearchData *
chain_multiple_search_operation (struct MultipleSearchData *old_msd)
{
  GList *skip_list = NULL;
  GList *source_iter;
  struct ResultCount *rc;
  GrlSource *source;
  struct MultipleSearchData *msd;

  /* Compute skip parameter for each of the sources that can still
     provide more results */
  source_iter = old_msd->sources_more;
  while (source_iter) {
    source = GRL_SOURCE (source_iter->data);
    rc = (struct ResultCount *)
      g_hash_table_lookup (old_msd->table, (gpointer) source);
    skip_list = g_list_prepend (skip_list,
				GINT_TO_POINTER (rc->count + rc->skip));
    source_iter = g_list_next (source_iter);
  }

  /* Reverse the sources list so that they match the skip list */
  old_msd->sources_more = g_list_reverse (old_msd->sources_more);

  /* Continue the search process with the same search_id */
  msd = start_multiple_search_operation (old_msd->search_id,
					 old_msd->sources_more,
					 old_msd->text,
					 old_msd->keys,
					 skip_list,
					 old_msd->pending,
					 old_msd->options,
					 old_msd->user_callback,
					 old_msd->user_data);
  g_list_free (skip_list);

  return msd;
}

static void
multiple_result_async_cb (GrlSource *source,
                          guint op_id,
                          GrlMedia *media,
                          guint remaining,
                          gpointer user_data,
                          const GError *error)
{
  GrlDataSync *ds = (GrlDataSync *) user_data;

  GRL_DEBUG ("multiple_result_async_cb");

  if (error) {
    ds->error = g_error_copy (error);

    /* Free previous results */
    g_list_free_full (ds->data, g_object_unref);

    ds->data = NULL;
    ds->complete = TRUE;
    return;
  }

  if (media) {
    ds->data = g_list_prepend (ds->data, media);
  }

  if (remaining == 0) {
    ds->data = g_list_reverse (ds->data);
    ds->complete = TRUE;
  }
}

static void
multiple_search_cb (GrlSource *source,
		    guint search_id,
		    GrlMedia *media,
		    guint remaining,
		    gpointer user_data,
		    const GError *error)
{
  GRL_DEBUG (__FUNCTION__);

  struct MultipleSearchData *msd;
  gboolean emit;
  gboolean operation_done = FALSE;
  struct ResultCount *rc;

  msd = (struct MultipleSearchData *) user_data;

  GRL_DEBUG ("multiple:remaining == %u, source:remaining = %u (%s)",
             msd->remaining, remaining,
             grl_source_get_name (GRL_SOURCE (source)));

  /* Check if operation is done, that is, if all the sources involved
     in the multiple operation have emitted remaining=0 */
  if (remaining == 0) {
    msd->sources_done++;
    if (msd->sources_done == msd->sources_count) {
      operation_done = TRUE;
      GRL_DEBUG ("multiple operation chunk done");
    }
  }

  /* --- Cancellation management --- */

  if (msd->cancelled) {
    GRL_DEBUG ("operation is cancelled or already finished, skipping result!");
    g_clear_object (&media);
    if (operation_done) {
      /* This was the last result and the operation is cancelled
	 so we don't have anything else to do*/
      goto operation_done;
    }
    /* The operation is cancelled but the sources involved
       in the operation still have to complete the cancellation,
       that is, they still have not send remaining=0 */
    return;
  }

  /* --- Update remaining count --- */

  rc = (struct ResultCount *)
    g_hash_table_lookup (msd->table, (gpointer) source);

  if (media) {
    rc->received++;
  }

  rc->remaining = remaining;

  if (rc->remaining == 0 && rc->received != rc->count) {
    /* This source failed to provide as many results as we requested,
       we will have to check if other sources can provide the missing
       results */
    if (rc->count != GRL_COUNT_INFINITY)
      msd->pending += rc->count - rc->received;
  } else if (remaining == 0) {
    /* This source provided all requested results, if others did not
       we can use this to request more */
    msd->sources_more = g_list_prepend (msd->sources_more, source);
    GRL_DEBUG ("Source %s provided all requested results",
               grl_source_get_name (GRL_SOURCE (source)));
  }

  /* --- Manage NULL results --- */

  if (remaining == 0 && media == NULL && msd->remaining > 0) {
    /* A source emitted a NULL result to finish its search operation
       we don't want to relay this to the client (unless this is the
       last one in the multiple search) */
    GRL_DEBUG ("Skipping NULL result");
    emit = FALSE;
  } else {
    emit = TRUE;
  }

  /* --- Result emission --- */

  if (emit) {
    msd->user_callback (source,
  		        msd->search_id,
 		        media,
		        msd->remaining--,
		        msd->user_data,
		        NULL);
  }

  /* --- Manage pending results --- */

  if (operation_done && msd->pending > 0 && msd->sources_more) {
    /* We did not get all the requested results and have sources
       that can still provide more */
    GRL_DEBUG ("Requesting next chunk");
    chain_multiple_search_operation (msd);
    return;
  } else if (operation_done && msd->pending > 0) {
    /* We don't have sources capable of providing more results,
       finish operation now */
    msd->user_callback (source,
			msd->search_id,
			NULL,
			0,
			msd->user_data,
			NULL);
    goto operation_done;
  } else if (operation_done) {
    /* We provided all the results */
    goto operation_done;
  } else {
    /* We are still receiving results */
    return;
  }

 operation_done:
  GRL_DEBUG ("Multiple operation finished (%u)", msd->search_id);
  grl_operation_remove (msd->search_id);
}

static void
free_media_from_uri_data (struct MediaFromUriCallbackData *mfucd)
{
  GRL_DEBUG ("free_media_from_uri_data");
  g_list_free (mfucd->sources);
  g_free (mfucd->uri);
  g_list_free (mfucd->keys);
  g_clear_object (&mfucd->options);
  g_free (mfucd);
}

static void
media_from_uri_cb (GrlSource *source,
                   guint operation_id,
                   GrlMedia *media,
                   gpointer user_data,
                   const GError *error)
{
  gboolean found = FALSE;
  struct MediaFromUriCallbackData *mfucd =
    (struct MediaFromUriCallbackData *) user_data;

  /* Found a result? */
  if (media != NULL) {
    mfucd->user_callback (source, 0, media, mfucd->user_data, NULL);
    free_media_from_uri_data (mfucd);
    return;
  }

  /* Try the next source. */
  for (; !found && mfucd->iter != NULL; mfucd->iter = mfucd->iter->next) {
    GrlSource *next_source = GRL_SOURCE (mfucd->iter->data);

    if (grl_source_test_media_from_uri (next_source, mfucd->uri)) {
      grl_source_get_media_from_uri (next_source, mfucd->uri, mfucd->keys,
                                     mfucd->options, media_from_uri_cb, mfucd);
      found = TRUE;
    }
  }

  /* No source knows how to deal with 'uri', invoke user callback
   * with NULL GrlMedia */
  if (!found) {
    GError *_error = g_error_new (GRL_CORE_ERROR,
                                  GRL_CORE_ERROR_MEDIA_FROM_URI_FAILED,
                                  _("Could not resolve media for URI “%s”"),
                                  mfucd->uri);

    mfucd->user_callback (NULL, 0, NULL, mfucd->user_data, _error);

    g_error_free (_error);
    free_media_from_uri_data (mfucd);

    return;
  }
}

/* ================ API ================ */

/**
 * grl_multiple_search:
 * @sources: (element-type GrlSource) (allow-none):
 * a #GList of #GrlSource<!-- -->s to search from (%NULL for all
 * searchable sources)
 * @text: the text to search for
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID to retrieve
 * @options: options wanted for that operation
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass to the user callback
 *
 * Search for @text in all the sources specified in @sources.
 *
 * If @text is @NULL then NULL-text searchs will be used for each searchable
 * plugin (see #grl_source_search for more details).
 *
 * This method is asynchronous.
 *
 * Returns: the operation identifier
 *
 * Since: 0.2.0
 */
guint
grl_multiple_search (const GList *sources,
		     const gchar *text,
		     const GList *keys,
		     GrlOperationOptions *options,
		     GrlSourceResultCb callback,
		     gpointer user_data)
{
  GrlRegistry *registry;
  GList *sources_list;
  struct MultipleSearchData *msd;
  gboolean allocated_sources_list = FALSE;
  guint operation_id;

  GRL_DEBUG ("grl_multiple_search");

  g_return_val_if_fail (callback != NULL, 0);
  g_return_val_if_fail (GRL_IS_OPERATION_OPTIONS (options), 0);

  /* If no sources have been provided then get the list of all
     searchable sources from the registry */
  if (!sources) {
    registry = grl_registry_get_default ();
    sources_list =
      grl_registry_get_sources_by_operations (registry,
                                              GRL_OP_SEARCH,
                                              TRUE);
    if (sources_list == NULL) {
      /* No searchable sources? Raise error and bail out */
      g_list_free (sources_list);
      handle_no_searchable_sources (callback, user_data);
      return 0;
    } else {
      sources = sources_list;
      allocated_sources_list = TRUE;
    }
  }

  /* Start multiple search operation */
  operation_id = grl_operation_generate_id ();
  msd = start_multiple_search_operation (operation_id,
					 sources,
					 text,
					 keys,
					 NULL,
					 grl_operation_options_get_count (options),
					 options,
					 callback,
					 user_data);
  if  (allocated_sources_list) {
    g_list_free ((GList *) sources);
  }

  return msd->search_id;
}

static void
multiple_search_cancel_cb (struct MultipleSearchData *msd)
{
  GList *sources, *ids;
  guint id;

  /* Go through all the sources involved in that operation and issue
     cancel() operations for each one */
  sources = msd->sources;
  ids = msd->search_ids;
  while (sources) {
    GRL_DEBUG ("cancelling operation %s:%u",
               grl_source_get_name (GRL_SOURCE (sources->data)),
               GPOINTER_TO_UINT (ids->data));
    grl_operation_cancel (GPOINTER_TO_INT (ids->data));
    sources = g_list_next (sources);
    ids = g_list_next (ids);
  }

  msd->cancelled = TRUE;

  /* Send operation finished message now to client (remaining == 0) */
  id = g_idle_add (confirm_cancel_idle, msd);
  g_source_set_name_by_id (id, "[grilo] confirm_cancel_idle");
}

/**
 * grl_multiple_search_sync:
 * @sources: (element-type GrlSource) (allow-none):
 * a #GList of #GrlSource<!-- -->s where to search from (%NULL for all
 * available sources with search capability)
 * @text: the text to search for
 * @keys: (element-type GrlKeyID): the #GList of
 * #GrlKeyID to retrieve
 * @options: options wanted for that operation
 * @error: a #GError, or @NULL
 *
 * Search for @text in all the sources specified in @sources.
 *
 * This method is synchronous.
 *
 * Returns: (element-type GrlMedia) (transfer full): a list with #GrlMedia elements
 *
 * Since: 0.2.0
 */
GList *
grl_multiple_search_sync (const GList *sources,
                          const gchar *text,
                          const GList *keys,
                          GrlOperationOptions *options,
                          GError **error)
{
  GrlDataSync *ds;
  GList *result;

  ds = g_slice_new0 (GrlDataSync);

  if (grl_multiple_search (sources,
                           text,
                           keys,
                           options,
                           multiple_result_async_cb,
                           ds))
    grl_wait_for_async_operation_complete (ds);

  if (ds->error) {
    if (error) {
      *error = ds->error;
    } else {
      g_error_free (ds->error);
    }
  }

  result = (GList *) ds->data;
  g_slice_free (GrlDataSync, ds);

  return result;
}

/**
 * grl_multiple_get_media_from_uri:
 * @uri: A URI that can be used to identify a media resource
 * @keys: (element-type GrlKeyID): List of metadata keys we want to obtain.
 * @options: options wanted for that operation
 * @callback: (scope notified): the user defined callback
 * @user_data: the user data to pass to the user callback
 *
 * Goes though all available media sources until it finds one capable of
 * constructing a GrlMedia object representing the media resource exposed
 * by @uri.
 *
 * This method is asynchronous.
 *
 * Since: 0.2.0
 */
void
grl_multiple_get_media_from_uri (const gchar *uri,
				      const GList *keys,
				      GrlOperationOptions *options,
				      GrlSourceResolveCb callback,
				      gpointer user_data)
{
  GrlRegistry *registry;
  GList *sources;
  struct MediaFromUriCallbackData *mfucd;

  g_return_if_fail (uri != NULL);
  g_return_if_fail (keys != NULL);
  g_return_if_fail (callback != NULL);
  g_return_if_fail (GRL_IS_OPERATION_OPTIONS (options));

  registry = grl_registry_get_default ();
  sources =
    grl_registry_get_sources_by_operations (registry,
                                            GRL_OP_MEDIA_FROM_URI,
                                            TRUE);

  /* Iterate through the sources, trying each one which knows how to deal with
   * @uri, and continuing if it then returns %NULL for the media. */
  mfucd = g_new0 (struct MediaFromUriCallbackData, 1);

  mfucd->sources = sources;  /* transfer */
  mfucd->iter = sources;
  mfucd->user_callback = callback;
  mfucd->user_data = user_data;
  mfucd->uri = g_strdup (uri);
  mfucd->keys = g_list_copy ((GList *) keys);
  mfucd->options = g_object_ref (options);

  /* Start the first iteration off. */
  media_from_uri_cb (NULL, 0, NULL, mfucd, NULL);
}
