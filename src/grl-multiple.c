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

#include "grl-multiple.h"
#include "grl-plugin-registry.h"
#include "grl-error.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "grl-multiple"

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
  GrlMetadataResolutionFlags flags;
  GrlMediaSourceResultCb user_callback;
  gpointer user_data;
};

struct ResultCount {
  guint count;
  guint remaining;
  guint received;
  guint skip;
};

struct CallbackData {
  GrlMediaSourceResultCb user_callback;
  gpointer user_data;
};

static void multiple_search_cb (GrlMediaSource *source,
				guint search_id,
				GrlMedia *media,
				guint remaining,
				gpointer user_data,
				const GError *error);

/* ================= Globals ================= */

static GHashTable *pending_operations = NULL;
static gint multiple_search_id = 1;

/* ================ Utitilies ================ */

static void
free_multiple_search_data (struct MultipleSearchData *msd)
{
  g_debug ("free_multiple_search_data");
  g_hash_table_unref (msd->table);
  g_list_free (msd->search_ids);
  g_list_free (msd->sources);
  g_list_free (msd->sources_more);
  g_list_free (msd->keys);
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

  error = g_error_new (GRL_ERROR, GRL_ERROR_SEARCH_FAILED, 
                       "No searchable sources available");
  callback_data->user_callback (NULL, 0, NULL, 0, callback_data->user_data, error);

  g_error_free (error);
  g_free (callback_data);

  return FALSE;
}

static void
handle_no_searchable_sources (GrlMediaSourceResultCb callback, gpointer user_data)
{
  struct CallbackData *callback_data = g_new0 (struct CallbackData, 1);
  callback_data->user_callback = callback;
  callback_data->user_data = user_data;
  g_idle_add (handle_no_searchable_sources_idle, callback_data);
}

static GList *
source_array_to_list (GrlMediaPlugin **sources)
{
  GList *list = NULL;
  gint n = 0;
  while (sources[n]) {
    list = g_list_prepend (list, sources[n]);
    n++;
  }
  return list;
}

static struct MultipleSearchData *
start_multiple_search_operation (guint search_id,
				 const GList *sources,
				 const gchar *text,
				 const GList *keys,
				 const GList *skip_counts,
				 guint count,
				 GrlMetadataResolutionFlags flags,
				 GrlMediaSourceResultCb user_callback,
				 gpointer user_data)
{
  g_debug ("start_multiple_search_operation");
  
  struct MultipleSearchData *msd;
  GList *iter_sources, *iter_skips;
  guint n, first_count, individual_count;

  /* Prepare data required to execute the operation */
  msd = g_new0 (struct MultipleSearchData, 1);
  msd->table = g_hash_table_new_full (g_direct_hash, g_direct_equal,
				      NULL, g_free);
  msd->remaining = count - 1;
  msd->search_id = search_id;
  msd->text = g_strdup (text);
  msd->keys = g_list_copy ((GList *) keys);
  msd->flags = flags;
  msd->user_callback = user_callback;
  msd->user_data = user_data;

  /* Compute the # of items to request to each source */
  n = g_list_length ((GList *) sources);
  individual_count = count / n;
  first_count = individual_count + count % n;

  /* Issue search operations on each source */
  iter_sources = (GList *) sources;
  iter_skips = (GList *) skip_counts;
  n = 0;
  while (iter_sources) {
    GrlMediaSource *source;
    guint c, id;
    struct ResultCount *rc;
    guint skip;

    source = GRL_MEDIA_SOURCE (iter_sources->data);

    c = (n == 0) ? first_count : individual_count;
    n++;

    if (c > 0) {
      rc = g_new0 (struct ResultCount, 1);
      rc->count = c;
      g_hash_table_insert (msd->table, source, rc);

      if (iter_skips) {
	skip = GPOINTER_TO_INT (iter_skips->data);
      } else {
	skip = 0;
      }

      id = grl_media_source_search (source,
				    msd->text,
				    msd->keys,
				    skip, rc->count,
				    flags,
				    multiple_search_cb,
				    msd);

      g_debug ("Operation %s:%u: Searching %u items",
	       grl_metadata_source_get_name (GRL_METADATA_SOURCE (source)),
	       id, rc->count);

      msd->search_ids = g_list_prepend (msd->search_ids, GINT_TO_POINTER (id));
      msd->sources = g_list_prepend (msd->sources, source);
      msd->sources_count++;
    }

    /* Move to the next source */
    iter_sources = g_list_next (iter_sources);
    iter_skips = g_list_next (iter_skips);
  }

  /* This frees the previous msd structure */
  g_hash_table_insert (pending_operations,
		       GINT_TO_POINTER (msd->search_id), msd);

  return msd;
}

static struct MultipleSearchData *
chain_multiple_search_operation (struct MultipleSearchData *old_msd)
{
  GList *skip_list = NULL;
  GList *source_iter;
  struct ResultCount *rc;
  GrlMediaSource *source;
  struct MultipleSearchData *msd;

  /* Compute skip parameter for each of the sources that can still
     provide more results */
  source_iter = old_msd->sources_more;
  while (source_iter) {
    source = GRL_MEDIA_SOURCE (source_iter->data);
    rc = (struct ResultCount *)
      g_hash_table_lookup (old_msd->table, (gpointer) source);
    skip_list = g_list_prepend (skip_list,
				GINT_TO_POINTER (rc->count + rc->skip));
    source_iter = g_list_next (source_iter);
  }
  old_msd->sources_more = g_list_reverse (old_msd->sources_more);

  /* Continue the search process with the same search_id */
  msd = start_multiple_search_operation (old_msd->search_id,
					 old_msd->sources_more,
					 old_msd->text,
					 old_msd->keys,
					 skip_list,
					 old_msd->pending,
					 old_msd->flags,
					 old_msd->user_callback,
					 old_msd->user_data);
  return msd;
}

static void
multiple_search_cb (GrlMediaSource *source,
		    guint search_id,
		    GrlMedia *media,
		    guint remaining,
		    gpointer user_data,
		    const GError *error)
{
  g_debug ("multiple_search_cb");

  struct MultipleSearchData *msd;
  gboolean emit;
  gboolean operation_done = FALSE;
  struct ResultCount *rc;

  msd = (struct MultipleSearchData *) user_data;

  g_debug ("multiple:remaining == %u, source:remaining = %u (%s)",
	   msd->remaining, remaining,
	   grl_metadata_source_get_name (GRL_METADATA_SOURCE (source)));

  /* Check if operation is done */
  if (remaining == 0) {
    msd->sources_done++;
    if (msd->sources_done == msd->sources_count) {
      operation_done = TRUE;
      g_debug ("multiple operation chunk done");
    }
  }

  /* --- Cancellation management --- */

  if (msd->cancelled) {
    g_debug ("operation is cancelled or already finished, skipping result!");
    if (media) {
      g_object_unref (media);
      media = NULL;
    }
    if (operation_done) {
      goto operation_done;
    }
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
    msd->pending += rc->count - rc->received;
  } else if (remaining == 0) {
    /* This source provided all requested results, if others did not
       we can use this to request more */
    msd->sources_more = g_list_prepend (msd->sources_more, source);
    g_debug ("Source %s provided all requested results",
	     grl_metadata_source_get_name (GRL_METADATA_SOURCE (source)));
  }

  /* --- Manage NULL results --- */

  if (remaining == 0 && media == NULL && msd->remaining > 0) {
    /* A source emitted a NULL result to finish its search operation
       we don't want to relay this to the client (unless this is the
       last one in the multiple search) */
    g_debug ("Skipping NULL result");
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
    g_debug ("Requesting next chunk");
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
  g_debug ("Multiple operation finished (%u)", msd->search_id);
  g_hash_table_remove (pending_operations, GINT_TO_POINTER (msd->search_id));
}

/* ================ API ================ */

guint
grl_multiple_search (const gchar *text,
		     const GList *keys,
		     guint count,
		     GrlMetadataResolutionFlags flags,
		     GrlMediaSourceResultCb callback,
		     gpointer user_data)
{
  GrlPluginRegistry *registry;
  GrlMediaPlugin **sources;
  struct MultipleSearchData *msd;
  GList *sources_list = NULL;

  g_debug ("grl_multiple_search");

  g_return_val_if_fail (count > 0, 0);
  g_return_val_if_fail (text != NULL, 0);
  g_return_val_if_fail (callback != NULL, 0);

  if (!pending_operations) {
    pending_operations =
      g_hash_table_new_full (g_direct_hash,
			     g_direct_equal,
			     NULL,
			     (GDestroyNotify) free_multiple_search_data);
  }

  registry = grl_plugin_registry_get_instance ();
  sources = grl_plugin_registry_get_sources_by_operations (registry,
							   GRL_OP_SEARCH,
							   TRUE);
  /* No searchable sources? */
  if (sources[0] == NULL) {
    g_free (sources);
    handle_no_searchable_sources (callback, user_data);
    return 0;
  } else {
    sources_list = source_array_to_list (sources);
    g_free (sources);
  }

  /* Prepare operation */
  multiple_search_id++;
  msd = start_multiple_search_operation (multiple_search_id,
					 sources_list,
					 text,
					 keys,
					 NULL,
					 count,
					 flags,
					 callback,
					 user_data);
  g_list_free (sources_list);

  return msd->search_id;
}

void
grl_multiple_cancel (guint search_id)
{
  g_debug ("grl_multiple_cancel");
  
  struct MultipleSearchData *msd;
  GList *sources, *ids;

  if (!pending_operations) {
    return;
  }

  msd = (struct MultipleSearchData *)
    g_hash_table_lookup (pending_operations, GINT_TO_POINTER (search_id));
  if (!msd) {
    g_debug ("Tried to cancel invalid or already cancelled "\
	     "multiple operation. Skipping...");
    return;
  }

  sources = msd->sources;
  ids = msd->search_ids;
  while (sources) {
    g_debug ("cancelling operation %s:%u",
	     grl_metadata_source_get_name (GRL_METADATA_SOURCE (sources->data)),
	     GPOINTER_TO_UINT (ids->data));
    grl_media_source_cancel (GRL_MEDIA_SOURCE (sources->data),
                             GPOINTER_TO_INT (ids->data));
    sources = g_list_next (sources);
    ids = g_list_next (ids);
  }

  msd->cancelled = TRUE;

  /* Send operation finished message now (remaining == 0) */
  g_idle_add (confirm_cancel_idle, msd);
}
