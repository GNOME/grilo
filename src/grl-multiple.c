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
#include "grl-media-source-priv.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "grl-multiple"

struct MultipleSearchData {
  GHashTable *table;
  guint remaining;
  GList *search_ids;
  guint search_id;
  GrlMediaSourceResultCb user_callback;
  gpointer user_data;
};

static void
multiple_search_cb (GrlMediaSource *source,
		    guint search_id,
		    GrlMedia *media,
		    guint remaining,
		    gpointer user_data,
		    const GError *error)
{
  g_debug ("multiple_search_cb");

  struct MultipleSearchData *msd = (struct MultipleSearchData *) user_data;
  guint source_remaining;
  guint diff;

  g_debug ("multiple:remaining == %u, source:remaining = %u (%s)",
	     msd->remaining, remaining,
	     grl_metadata_source_get_name (GRL_METADATA_SOURCE (source)));

  source_remaining = GPOINTER_TO_INT (g_hash_table_lookup (msd->table,
							   (gpointer) source));
  if (remaining != -1) {
    diff = source_remaining - remaining;
    g_hash_table_insert (msd->table, source, GINT_TO_POINTER (remaining - 1));
  } else {
    diff = 0;
    g_hash_table_insert (msd->table, source, GINT_TO_POINTER (source_remaining - 1));
  }
  
  msd->remaining -= diff;

  msd->user_callback (source,
		      msd->search_id,
		      media,
		      msd->remaining,
		      msd->user_data,
		      NULL);
  
  if (msd->remaining == 0) {
    g_debug ("Multiple operation finished (%u)", msd->search_id);
    g_hash_table_unref (msd->table);
    g_free (msd);
  } else {
    msd->remaining--;
  }
}

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
  guint individual_count, first_count, source_count;
  guint search_id, n;
  struct MultipleSearchData *msd;

  g_debug ("grl_multiple_search");

  registry = grl_plugin_registry_get_instance ();
  sources = grl_plugin_registry_get_sources_by_operations (registry,
							   GRL_OP_SEARCH,
							   TRUE);

  /* TODO: handle this properly */
  if (sources[0] == NULL) {
    g_free (sources);
    callback (NULL, 0, NULL, 0, user_data, NULL);
    return 0;
  }

  /* Prepare operation */
  for (n = 0; sources[n] != NULL; n++);
  individual_count = count / n;
  first_count = individual_count + count % n;
  source_count = n;

  msd = g_new0 (struct MultipleSearchData, 1);
  msd->table = g_hash_table_new (g_direct_hash, g_direct_equal);
  msd->remaining = count - 1;
  msd->user_callback = callback;
  msd->user_data = user_data;
    
  /* Execute multiple search */
  for (n = 0; sources[n]; n++) {
    GrlMediaSource *source;
    guint c, id;

    source = GRL_MEDIA_SOURCE (sources[n]);

    if (n == 0) {
      c = first_count; 
      msd->search_id = grl_media_source_gen_browse_id (source);
    } else {
      c = individual_count;
    }

    if (c > 0) {
      g_hash_table_insert (msd->table, source, GINT_TO_POINTER (c - 1));
      id = grl_media_source_search (source,
				    text,
				    keys,
				    0, c,
				    flags,
				    multiple_search_cb,
				    msd);
      g_debug ("Operation %u: Searching %u items in %s",
	       id, c, grl_metadata_source_get_name (GRL_METADATA_SOURCE (source)));
      msd->search_ids = g_list_prepend (msd->search_ids, GINT_TO_POINTER (id));
    }
  }

  g_free (sources);

  return msd->search_id;
}
