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

#include <media-store.h>

#include <gtk/gtk.h>
#include <string.h>

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "test-ui"

#define BROWSE_FLAGS (MS_RESOLVE_FAST_ONLY | MS_RESOLVE_IDLE_RELAY)
#define METADATA_FLAGS (MS_RESOLVE_FULL | MS_RESOLVE_IDLE_RELAY)

#define WINDOW_TITLE "Media Store Test UI"

#define BROWSER_MIN_WIDTH   320
#define BROWSER_MIN_HEIGHT  400

#define METADATA_MIN_WIDTH  320
#define METADATA_MIN_HEIGHT 400

#define BROWSE_CHUNK_SIZE   100
#define BROWSE_MAX_COUNT    (2 * BROWSE_CHUNK_SIZE)

enum {
  OBJECT_TYPE_SOURCE = 0,
  OBJECT_TYPE_CONTAINER,
  OBJECT_TYPE_MEDIA
};

enum {
  BROWSER_MODEL_SOURCE = 0,
  BROWSER_MODEL_CONTENT,
  BROWSER_MODEL_TYPE,
  BROWSER_MODEL_NAME,
  BROWSER_MODEL_ICON,
};

enum {
  METADATA_MODEL_NAME = 0,
  METADATA_MODEL_VALUE,
};

enum {
  SEARCH_MODEL_NAME = 0,
  SEARCH_MODEL_SOURCE,
};

enum {
  QUERY_MODEL_NAME = 0,
  QUERY_MODEL_SOURCE,
};

typedef struct {
  GtkWidget *window;
  GtkWidget *lpane;
  GtkWidget *rpane;
  GtkWidget *search_text;
  GtkWidget *search_combo;
  GtkTreeModel *search_combo_model;
  GtkWidget *search_btn;
  GtkWidget *query_text;
  GtkWidget *query_combo;
  GtkTreeModel *query_combo_model;
  GtkWidget *query_btn;
  GtkWidget *back_btn;
  GtkWidget *show_btn;
  GtkWidget *browser;
  GtkTreeModel *browser_model;
  GtkWidget *metadata;
  GtkTreeModel *metadata_model;
  const gchar *last_url;
} UiView;

typedef struct {
  /* Keeps track of our browsing position and history  */
  GList *source_stack;
  GList *container_stack;
  MsMediaSource *cur_source;
  MsContentMedia *cur_container;

  /* Keeps track of the last element we showed metadata for */
  MsMediaSource *cur_md_source;
  MsContentMedia *cur_md_media;

  /* Keeps track of browse/search state */
  gboolean op_ongoing;
  MsMediaSource *cur_op_source;
  guint cur_op_id;
} UiState;

typedef struct {
  guint offset;
  guint count;
  gchar *text;
} OperationState;

static UiView *view;
static UiState *ui_state;

static void show_plugins (void);

static GtkTreeModel *
create_browser_model (void)
{
  return GTK_TREE_MODEL (gtk_list_store_new (5,
					     G_TYPE_OBJECT,     /* Source */
					     G_TYPE_OBJECT,     /* Content */
					     G_TYPE_INT,        /* Type */
					     G_TYPE_STRING,     /* Name */
					     GDK_TYPE_PIXBUF)); /* Icon */
}

static GtkTreeModel *
create_metadata_model (void)
{
  return GTK_TREE_MODEL (gtk_list_store_new (2,
					     G_TYPE_STRING,     /* name */
					     G_TYPE_STRING));   /* value */
}

static GtkTreeModel *
create_search_combo_model (void)
{
  return GTK_TREE_MODEL (gtk_list_store_new (2,
					     G_TYPE_STRING,     /* name */
					     G_TYPE_OBJECT));   /* source */
}

static GtkTreeModel *
create_query_combo_model (void)
{
  return GTK_TREE_MODEL (gtk_list_store_new (2,
					     G_TYPE_STRING,     /* name */
					     G_TYPE_OBJECT));   /* source */
}

static GdkPixbuf *
load_icon (const gchar *icon_name)
{
  GdkScreen *screen;
  GtkIconTheme *theme;
  GdkPixbuf *pixbuf;
  GError *error = NULL;
  
  screen = gdk_screen_get_default ();
  theme = gtk_icon_theme_get_for_screen (screen);
  pixbuf = gtk_icon_theme_load_icon (theme, icon_name, 22, 22, &error);
  
  if (pixbuf == NULL) {
    g_warning ("Failed to load icon %s: %s", icon_name,  error->message);
    g_error_free (error);
  }
  
  return pixbuf;  
}

static GdkPixbuf *
get_icon_for_media (MsContentMedia *media)
{
  if (MS_IS_CONTENT_BOX (media)) {
    return load_icon (GTK_STOCK_DIRECTORY);
  } else if (MS_IS_CONTENT_VIDEO (media)) {
    return load_icon ("gnome-mime-video");
  } else if (MS_IS_CONTENT_AUDIO (media)) {
    return load_icon ("gnome-mime-audio");
  } else if (MS_IS_CONTENT_IMAGE (media)) {
    return load_icon ("gnome-mime-image");
  } else { 
    return load_icon (GTK_STOCK_FILE);
  }
}

static GList *
browse_keys (void)
{
  return ms_metadata_key_list_new (MS_METADATA_KEY_ID,
				   MS_METADATA_KEY_TITLE,
				   MS_METADATA_KEY_CHILDCOUNT,
				   NULL);
}

static GList *
metadata_keys (void)
{
  return ms_metadata_key_list_new (MS_METADATA_KEY_ID,
				   MS_METADATA_KEY_TITLE,
				   MS_METADATA_KEY_URL,
				   MS_METADATA_KEY_ARTIST,
				   MS_METADATA_KEY_ALBUM,
				   MS_METADATA_KEY_GENRE,
				   MS_METADATA_KEY_THUMBNAIL,
				   MS_METADATA_KEY_SITE,
				   MS_METADATA_KEY_AUTHOR,
				   MS_METADATA_KEY_LYRICS,
				   MS_METADATA_KEY_DATE,
				   MS_METADATA_KEY_MIME,
				   MS_METADATA_KEY_DURATION,
				   MS_METADATA_KEY_RATING,
				   MS_METADATA_KEY_CHILDCOUNT,
				   NULL);
}

static void
browse_history_push (MsMediaSource *source, MsContentMedia *media)
{
  if (source)
    g_object_ref (source);
  if (media)
    g_object_ref (media);

  ui_state->source_stack = g_list_append (ui_state->source_stack, source);
  ui_state->container_stack = g_list_append (ui_state->container_stack, media);
}

static void
browse_history_pop (MsMediaSource **source, MsContentMedia **media)
{
  GList *tmp;
  tmp = g_list_last (ui_state->source_stack);
  if (tmp) {
    *source = MS_MEDIA_SOURCE (tmp->data);
    ui_state->source_stack = g_list_delete_link (ui_state->source_stack, tmp);
  } 
  tmp = g_list_last (ui_state->container_stack);
  if (tmp) {
    *media = (MsContentMedia *) tmp->data;
    ui_state->container_stack = g_list_delete_link (ui_state->container_stack,
						    tmp);
  } 
}

static void
set_cur_browse (MsMediaSource *source, MsContentMedia *media)
{
  if (ui_state->cur_source)
    g_object_unref (ui_state->cur_source);
  if (ui_state->cur_container)
    g_object_unref (ui_state->cur_container);

  if (source)
    g_object_ref (source);
  if (media)
    g_object_ref (media);

  ui_state->cur_source = source;  
  ui_state->cur_container = media;  
}

static void
set_cur_metadata (MsMediaSource *source, MsContentMedia *media)
{
  if (ui_state->cur_md_source)
    g_object_unref (ui_state->cur_md_source);
  if (ui_state->cur_md_media)
    g_object_unref (ui_state->cur_md_media);

  if (source)
    g_object_ref (source);
  if (media)
    g_object_ref (media);

  ui_state->cur_md_source = source;
  ui_state->cur_md_media = media;
}

static void
clear_panes (void)
{
  if (view->browser_model) {
    gtk_list_store_clear (GTK_LIST_STORE (view->browser_model));
    g_object_unref (view->browser_model);
  }
  view->browser_model = create_browser_model ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (view->browser),
			   view->browser_model);

  if (view->metadata_model) {
    gtk_list_store_clear (GTK_LIST_STORE (view->metadata_model));
    g_object_unref (view->metadata_model);
  }
  view->metadata_model = create_metadata_model ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (view->metadata),
			     view->metadata_model);
  gtk_widget_set_sensitive (view->show_btn, FALSE);
  view->last_url = NULL;
}

static void
cancel_current_operation (void)
{
  if (ui_state->op_ongoing) {
    ms_media_source_cancel (ui_state->cur_op_source, ui_state->cur_op_id);
    ui_state->op_ongoing = FALSE;
  }
}

static void 
metadata_cb (MsMediaSource *source,
	     MsContentMedia *media,
	     gpointer user_data,
	     const GError *error)
{
  GList *keys, *i;
  GtkTreeIter iter;
  MsPluginRegistry *registry;

  /* Not interested if not the last media we
     requested metadata for */
  if (media != ui_state->cur_md_media) {
    return;
  }

  if (view->metadata_model) {
    gtk_list_store_clear (GTK_LIST_STORE (view->metadata_model));
    g_object_unref (view->metadata_model);
  }
  view->metadata_model = create_metadata_model ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (view->metadata),
			   view->metadata_model);

  if (error) {
    g_critical ("Error: %s", error->message);
    return;
  }
  
  if (media) {
    registry = ms_plugin_registry_get_instance ();
    keys = ms_content_get_keys (MS_CONTENT (media));
    i = keys;
    while (i) {
      const MsMetadataKey *key =
	ms_plugin_registry_lookup_metadata_key (registry,
						POINTER_TO_MSKEYID (i->data));
      const GValue *g_value = ms_content_get (MS_CONTENT (media),
					      POINTER_TO_MSKEYID (i->data));
      gchar *value = g_value ? g_strdup_value_contents (g_value) : "";
      gtk_list_store_append (GTK_LIST_STORE (view->metadata_model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (view->metadata_model),
			  &iter,
			  METADATA_MODEL_NAME, MS_METADATA_KEY_GET_NAME (key),
			  METADATA_MODEL_VALUE, value,
			  -1);
      i = g_list_next (i);
    }
    
    g_list_free (keys);

    /* Don't free media (we do not ref it when issuing metadata(),
       so its reference comes from the treeview and that's freed
       when the treeview is cleared */

    /* Set/unset show button */
    if ((MS_IS_CONTENT_AUDIO (media) ||
         MS_IS_CONTENT_VIDEO (media) ||
         MS_IS_CONTENT_IMAGE (media)) &&
        (view->last_url = ms_content_media_get_url (media))) {
      gtk_widget_set_sensitive (view->show_btn, TRUE);
    } else {
      gtk_widget_set_sensitive (view->show_btn, FALSE);
      view->last_url = NULL;
    }
  }
}

static void
operation_started (MsMediaSource *source, guint operation_id)
{
  ui_state->op_ongoing = TRUE;
  ui_state->cur_op_source  = source;
  ui_state->cur_op_id = operation_id;
}

static void
operation_finished (void)
{
  ui_state->op_ongoing = FALSE;
}

static void
browse_cb (MsMediaSource *source,
	   guint browse_id,
	   MsContentMedia *media,
	   guint remaining,
	   gpointer user_data,
	   const GError *error)
{
  gint type;
  const gchar *name;
  GtkTreeIter iter;
  GdkPixbuf *icon;
  OperationState *state = (OperationState *) user_data;
  guint next_browse_id;

  if (error) {
    g_critical ("Error: %s", error->message);
  }

  state->count++;

  if (media) {
    icon = get_icon_for_media (media);
    name = ms_content_media_get_title (media);
    if (MS_IS_CONTENT_BOX (media)) {
      gint childcount = ms_content_box_get_childcount (MS_CONTENT_BOX (media));
      type = OBJECT_TYPE_CONTAINER;
      if (childcount != MS_METADATA_KEY_CHILDCOUNT_UNKNOWN) {
	name = g_strdup_printf ("%s (%d)", name, childcount);
      } else {
	name = g_strconcat (name, " (?)", NULL);
      }
    } else {
      type = OBJECT_TYPE_MEDIA;
    }
    
    gtk_list_store_append (GTK_LIST_STORE (view->browser_model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (view->browser_model),
			&iter,
			BROWSER_MODEL_SOURCE, source,
			BROWSER_MODEL_CONTENT, media,
			BROWSER_MODEL_TYPE, type,
			BROWSER_MODEL_NAME, name,
			BROWSER_MODEL_ICON, icon,
			-1);

    g_object_unref (media);
    gdk_pixbuf_unref (icon);
  }

  if (remaining == 0) {
    /* Done with this chunk, check if there is more to browse */
    if (ui_state->op_ongoing &&
	ui_state->cur_op_id == browse_id) {
      /* Operation is still valid, so let's go */
      state->offset += state->count;
      if (state->count >= BROWSE_CHUNK_SIZE &&
	  state->offset < BROWSE_MAX_COUNT) {
	state->count = 0;
	next_browse_id =
	  ms_media_source_browse (source,
				  ui_state->cur_container,
				  browse_keys (),
				  state->offset, BROWSE_CHUNK_SIZE,
				  BROWSE_FLAGS,
				  browse_cb,
				  state);
	operation_started (source, next_browse_id);
      } else {
	/* We browsed all requested elements  */
	goto browse_finished;
      }
    } else {
      /* The operation was cancelled */
      goto browse_finished;
    }
  }

  return;

 browse_finished:
  g_free (state);
  operation_finished ();
  g_debug ("**** browse finished (%d) ****", browse_id);
}

static void
browse (MsMediaSource *source, MsContentMedia *container)
{
  guint browse_id;
  if (source) {
    /* If we have an ongoing operation, cancel it first */
    cancel_current_operation ();
    clear_panes ();  
    
    OperationState *state = g_new0 (OperationState, 1);
    browse_id = ms_media_source_browse (source,
					container,
					browse_keys (),
					0, BROWSE_CHUNK_SIZE,
					BROWSE_FLAGS,
					browse_cb,
					state);
    operation_started (source, browse_id);
  } else {
    show_plugins ();
  }

  set_cur_browse (source, container);
  set_cur_metadata (NULL, NULL);
}

static void
browser_activated_cb (GtkTreeView *tree_view,
		      GtkTreePath *path,
		      GtkTreeViewColumn *column,
		      gpointer user_data)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  MsContentMedia *content;
  gint type;
  MsMediaSource *source;
  MsContentMedia *container;

  model = gtk_tree_view_get_model (tree_view);    
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter,
		      BROWSER_MODEL_SOURCE, &source,
		      BROWSER_MODEL_CONTENT, &content,
		      BROWSER_MODEL_TYPE, &type,
		      -1);

  if (type == OBJECT_TYPE_MEDIA) {
    return;
  }

  if (type == OBJECT_TYPE_SOURCE) {
    container = NULL;
  } else if (content) {
    container = content;
  } else {
    container = NULL;
  }

  browse_history_push (ui_state->cur_source, ui_state->cur_container);
  browse (source, container);

  if (source) {
    g_object_unref (source);
  }
  if (content) {
    g_object_unref (content);
  }
}

static void
metadata (MsMediaSource *source, MsContentMedia *media)
{
  if (source) {
    ms_media_source_metadata (source,
			      media,
			      metadata_keys (),
			      METADATA_FLAGS,
			      metadata_cb,
			      NULL);
  }
}

static void
browser_row_selected_cb (GtkTreeView *tree_view,
			 gpointer user_data)
{
  GtkTreePath *path;
  GtkTreeIter iter;
  MsMediaSource *source;
  MsContentMedia *content;

  gtk_tree_view_get_cursor (tree_view, &path, NULL);
  gtk_tree_model_get_iter (view->browser_model, &iter, path);
  gtk_tree_model_get (view->browser_model,
		      &iter,
		      BROWSER_MODEL_SOURCE, &source,
		      BROWSER_MODEL_CONTENT, &content,
		      -1);

  if (source != ui_state->cur_md_source ||
      content != ui_state->cur_md_media) {
    set_cur_metadata (source, content);
    metadata (source, content);
  }

  if (source)
    g_object_unref (source);
  if (content)
    g_object_unref (content);
}

static void
show_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  if (view->last_url) {
    g_app_info_launch_default_for_uri (view->last_url, NULL, NULL);
  }
}

static void
back_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  MsMediaSource *prev_source = NULL;
  MsContentMedia *prev_container = NULL;

  /* TODO: when using dynamic sources this will break
     because we have references to the removed sources
     in these lists that we have to remove. */

  /* Cancel previous operation, if any */
  cancel_current_operation ();

  /* Get previous source and container id, and browse it */
  browse_history_pop (&prev_source, &prev_container);
  browse (prev_source, prev_container);
  
  if (prev_source)
    g_object_unref (prev_source);
  if (prev_container)
    g_object_unref (prev_container);
}

static void
search_cb (MsMediaSource *source,
	   guint search_id,
	   MsContentMedia *media,
	   guint remaining,
	   gpointer user_data,
	   const GError *error)
{
  const gchar *name;
  GtkTreeIter iter;
  GdkPixbuf *icon;
  OperationState *state = (OperationState *) user_data;
  guint next_search_id;

  if (error) {
    g_critical ("Error: %s", error->message);
  }

  state->count++;

  if (media) {
    icon = get_icon_for_media (media);
    name = ms_content_media_get_title (media);
    
    gtk_list_store_append (GTK_LIST_STORE (view->browser_model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (view->browser_model),
			&iter,
			BROWSER_MODEL_SOURCE, source,
			BROWSER_MODEL_CONTENT, media,
			BROWSER_MODEL_TYPE, OBJECT_TYPE_MEDIA,
			BROWSER_MODEL_NAME, name,
			BROWSER_MODEL_ICON, icon,
			-1);

    g_object_unref (media);
    gdk_pixbuf_unref (icon);
  }

  if (remaining == 0) {
    /* Done with this chunk, check if there is more to search */
    state->offset += state->count;
    if (state->count >= BROWSE_CHUNK_SIZE &&
	state->offset < BROWSE_MAX_COUNT) {
      state->count = 0;
      next_search_id =
	ms_media_source_search (source,
				state->text,
				browse_keys (),
				state->offset, BROWSE_CHUNK_SIZE,
				BROWSE_FLAGS,
				search_cb,
				state);
      operation_started (source, next_search_id);
    } else {
      goto search_finished;
    }
  }

  return;

 search_finished:
  g_free (state);
  operation_finished ();
  g_debug ("**** search finished (%d) ****", search_id);
}

static void
search (MsMediaSource *source, const gchar *text)
{
  OperationState *state;
  guint search_id;

  /* If we have an operation ongoing, let's cancel it first */
  cancel_current_operation ();

  state = g_new0 (OperationState, 1);
  state->text = (gchar *) text;
  search_id = ms_media_source_search (source,
				      text,
				      browse_keys (),
				      0, BROWSE_CHUNK_SIZE,
				      BROWSE_FLAGS,
				      search_cb,
				      state);
  clear_panes ();  
  operation_started (source, search_id);
}

static void
search_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  GtkTreeIter iter;

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (view->search_combo),
				     &iter)) {
    MsMediaSource *source;
    const gchar *text;
    gtk_tree_model_get (view->search_combo_model, &iter,
			SEARCH_MODEL_SOURCE, &source,
			-1);
    text = gtk_entry_get_text (GTK_ENTRY (view->search_text));
    search (source, text);

    if (source) {
      g_object_unref (source);
    }
  }
}

static void
query (MsMediaSource *source, const gchar *text)
{
  OperationState *state;
  guint query_id;

  /* If we have an operation ongoing, let's cancel it first */
  cancel_current_operation ();

  state = g_new0 (OperationState, 1);
  state->text = (gchar *) text;
  query_id = ms_media_source_query (source,
				    text,
				    browse_keys (),
				    0, BROWSE_CHUNK_SIZE,
				    BROWSE_FLAGS,
				    search_cb,
				    state);
  clear_panes ();  
  operation_started (source, query_id);
}

static void
query_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  GtkTreeIter iter;

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (view->query_combo),
				     &iter)) {
    MsMediaSource *source;
    const gchar *text;
    gtk_tree_model_get (view->query_combo_model, &iter,
			QUERY_MODEL_SOURCE, &source,
			-1);
    text = gtk_entry_get_text (GTK_ENTRY (view->query_text));
    query (source, text);

    if (source) {
      g_object_unref (source);
    }
  }
}

static void
query_combo_setup (void)
{
  MsPluginRegistry *registry;
  MsMediaPlugin **sources;
  GtkTreeIter iter;
  MsSupportedOps ops;
  guint i = 0;

  if (view->query_combo_model) {
    gtk_list_store_clear (GTK_LIST_STORE (view->query_combo_model));
    g_object_unref (view->query_combo_model);
  }
  view->query_combo_model = create_query_combo_model ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (view->query_combo),
			   view->query_combo_model);
  
  registry = ms_plugin_registry_get_instance ();
  sources = ms_plugin_registry_get_sources (registry);
  while (sources[i]) {
    ops = ms_metadata_source_supported_operations (MS_METADATA_SOURCE (sources[i]));
    if (ops & MS_OP_QUERY) {
      gchar *name;
      name = ms_metadata_source_get_name (MS_METADATA_SOURCE (sources[i]));
      gtk_list_store_append (GTK_LIST_STORE (view->query_combo_model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (view->query_combo_model),
			  &iter,
			  QUERY_MODEL_SOURCE, sources[i],
			  QUERY_MODEL_NAME, name,
			  -1);
    }
    i++;
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (view->query_combo), 0);
}

static void
search_combo_setup (void)
{
  MsPluginRegistry *registry;
  MsMediaPlugin **sources;
  GtkTreeIter iter;
  MsSupportedOps ops;
  guint i = 0;

  if (view->search_combo_model) {
    gtk_list_store_clear (GTK_LIST_STORE (view->search_combo_model));
    g_object_unref (view->search_combo_model);
  }
  view->search_combo_model = create_search_combo_model ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (view->search_combo),
			   view->search_combo_model);
  
  registry = ms_plugin_registry_get_instance ();
  sources = ms_plugin_registry_get_sources (registry);
  while (sources[i]) {
    ops = ms_metadata_source_supported_operations (MS_METADATA_SOURCE (sources[i]));
    if (ops & MS_OP_SEARCH) {
      gchar *name;
      name = ms_metadata_source_get_name (MS_METADATA_SOURCE (sources[i]));
      gtk_list_store_append (GTK_LIST_STORE (view->search_combo_model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (view->search_combo_model),
			  &iter,
			  SEARCH_MODEL_SOURCE, sources[i],
			  SEARCH_MODEL_NAME, name,
			  -1);
    }
    i++;
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (view->search_combo), 0);
}

static void
ui_setup (void)
{
  view = g_new0 (UiView, 1);
  ui_state = g_new0 (UiState, 1);

  /* Main window */
  view->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (view->window), WINDOW_TITLE);
  g_signal_connect (G_OBJECT (view->window), "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  /* Main layout */
  GtkWidget *box = gtk_hbox_new (FALSE, 0);
  view->lpane = gtk_vbox_new (FALSE, 0);
  view->rpane = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (view->window), box);
  gtk_container_add (GTK_CONTAINER (box), view->lpane);
  gtk_container_add (GTK_CONTAINER (box), view->rpane);

  /* Search & Query */
  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  view->search_text = gtk_entry_new ();
  gtk_container_add (GTK_CONTAINER (vbox), view->search_text);
  view->query_text = gtk_entry_new ();
  gtk_container_add (GTK_CONTAINER (vbox), view->query_text);
  gtk_container_add (GTK_CONTAINER (hbox), vbox);

  vbox = gtk_vbox_new (FALSE, 0);
  view->search_combo = gtk_combo_box_new ();
  gtk_container_add_with_properties (GTK_CONTAINER (vbox), view->search_combo,
				     "expand", FALSE,  NULL);
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (view->search_combo),
			      renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (view->search_combo),
				  renderer, "text", 0, NULL);
  view->query_combo = gtk_combo_box_new ();
  gtk_container_add_with_properties (GTK_CONTAINER (vbox), view->query_combo,
				     "expand", FALSE,  NULL);
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (view->query_combo),
			      renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (view->query_combo),
				  renderer, "text", 0, NULL);
  gtk_container_add (GTK_CONTAINER (hbox), vbox);


  vbox = gtk_vbox_new (FALSE, 0);
  view->search_btn = gtk_button_new_with_label ("Search");
  gtk_container_add_with_properties (GTK_CONTAINER (vbox), view->search_btn,
				     "expand", FALSE, NULL);
  view->query_btn = gtk_button_new_with_label ("Query");
  gtk_container_add_with_properties (GTK_CONTAINER (vbox), view->query_btn,
				     "expand", FALSE, NULL);
  gtk_container_add (GTK_CONTAINER (hbox), vbox);

  gtk_container_add_with_properties (GTK_CONTAINER (view->lpane), hbox,
				     "expand", FALSE, NULL);

  g_signal_connect (view->search_btn, "clicked",
		    G_CALLBACK (search_btn_clicked_cb), NULL);
  g_signal_connect (view->query_btn, "clicked",
		    G_CALLBACK (query_btn_clicked_cb), NULL);
  search_combo_setup ();
  query_combo_setup ();

  /* Go back button */
  view->back_btn = gtk_button_new_with_label ("Go back");
  gtk_container_add_with_properties (GTK_CONTAINER (view->lpane),
				     view->back_btn,
				     "expand", FALSE, NULL);
  g_signal_connect (view->back_btn, "clicked",
		    G_CALLBACK (back_btn_clicked_cb), NULL);

  /* Contents tree view */
  GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  view->browser = gtk_tree_view_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view->browser), FALSE);
  gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (view->browser), TRUE);

  gint i;
  GtkCellRenderer *col_renders[2];
  gchar *col_attributes[] = {"pixbuf", "text"};
  gint col_model[2] = { BROWSER_MODEL_ICON, BROWSER_MODEL_NAME};
  col_renders[0] = gtk_cell_renderer_pixbuf_new ();
  col_renders[1] = gtk_cell_renderer_text_new ();
  GtkTreeViewColumn *col = gtk_tree_view_column_new ();
  for (i=0; i<2; i++) {
    gtk_tree_view_column_pack_start (col, col_renders[i], FALSE);
    gtk_tree_view_column_add_attribute (col,
					col_renders[i],
					col_attributes[i],
					col_model[i]);
  }
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_insert_column (GTK_TREE_VIEW (view->browser), col, -1);

  gtk_container_add (GTK_CONTAINER (scroll), view->browser);
  gtk_container_add (GTK_CONTAINER (view->lpane), scroll);
  gtk_widget_set_size_request (view->browser,
			       BROWSER_MIN_WIDTH,
			       BROWSER_MIN_HEIGHT);
  g_signal_connect (view->browser, "row-activated",
		    G_CALLBACK (browser_activated_cb), NULL);
  g_signal_connect (view->browser, "cursor-changed",
		    G_CALLBACK (browser_row_selected_cb), NULL);

  /* Show button */
  view->show_btn = gtk_button_new_with_label ("Show");
  gtk_container_add_with_properties (GTK_CONTAINER (view->rpane),
                                     view->show_btn,
                                     "expand", FALSE, NULL);
  gtk_widget_set_sensitive (view->show_btn, FALSE);
  view->last_url = NULL;
  g_signal_connect (view->show_btn, "clicked",
                    G_CALLBACK (show_btn_clicked_cb), NULL);
  /* Contents model */
  view->browser_model = create_browser_model ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (view->browser),
			   view->browser_model);

  /* Metadata tree view */
  GtkWidget *scroll_md = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_md),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  view->metadata = gtk_tree_view_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view->metadata), FALSE);
  gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (view->metadata), TRUE);

  GtkCellRenderer *col_renders_md[2];
  gchar *col_attributes_md[] = {"text", "text"};
  gint col_model_md[2] = { METADATA_MODEL_NAME, METADATA_MODEL_VALUE};
  col_renders_md[0] = gtk_cell_renderer_text_new ();
  col_renders_md[1] = gtk_cell_renderer_text_new ();
  col = gtk_tree_view_column_new ();
  for (i=0; i<2; i++) {
    gtk_tree_view_column_pack_start (col, col_renders_md[i], FALSE);
    gtk_tree_view_column_add_attribute (col,
					col_renders_md[i],
					col_attributes_md[i],
					col_model_md[i]);
  }
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_insert_column (GTK_TREE_VIEW (view->metadata), col, -1);

  gtk_container_add (GTK_CONTAINER (scroll_md), view->metadata);
  gtk_container_add (GTK_CONTAINER (view->rpane), scroll_md);
  gtk_widget_set_size_request (view->metadata,
			       METADATA_MIN_WIDTH,
			       METADATA_MIN_HEIGHT);

  /* Populate the browser with the plugins */
  show_plugins ();
 
  gtk_widget_show_all (view->window);
}

static void
show_plugins ()
{
  MsMediaPlugin **sources;
  guint i;
  GtkTreeIter iter;
  MsSupportedOps ops;
  MsPluginRegistry *registry;

  registry = ms_plugin_registry_get_instance ();

  clear_panes ();

  i = 0;
  sources = ms_plugin_registry_get_sources (registry);
  while (sources[i]) {
    ops = ms_metadata_source_supported_operations (MS_METADATA_SOURCE (sources[i]));
    if (ops & MS_OP_BROWSE) {
      gchar *id, *name;
      GdkPixbuf *icon;
      icon = load_icon (GTK_STOCK_DIRECTORY);
      id = ms_media_plugin_get_id (sources[i]);
      name = ms_metadata_source_get_name (MS_METADATA_SOURCE (sources[i]));
      g_debug ("Loaded source: '%s'", name);
      gtk_list_store_append (GTK_LIST_STORE (view->browser_model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (view->browser_model),
			  &iter,
			  BROWSER_MODEL_SOURCE, sources[i],
			  BROWSER_MODEL_CONTENT, NULL,
			  BROWSER_MODEL_TYPE, OBJECT_TYPE_SOURCE,
			  BROWSER_MODEL_NAME, name,
			  BROWSER_MODEL_ICON, icon,
			  -1);
    }
    i++;
  }
}

static void
free_stack (GList **stack)
{ 
  GList *iter;
  iter = *stack;
  while (iter) {
    if (iter->data) {
      g_object_unref (iter->data);
    }
    iter = g_list_next (iter);
  }
  g_list_free (*stack);
  *stack = NULL;
}

static void
reset_browse_history (void)
{
  free_stack (&ui_state->source_stack);
  free_stack (&ui_state->container_stack);
  set_cur_browse (NULL, NULL);
  set_cur_metadata (NULL, NULL);
}

static void
reset_ui (void)
{
  cancel_current_operation ();
  clear_panes ();
  reset_browse_history ();
  show_plugins ();
}

static void
source_added_cb (MsPluginRegistry *registry, gpointer user_data)
{
  g_debug ("Detected new source available: '%s'",
	   ms_metadata_source_get_name (MS_METADATA_SOURCE (user_data)));

  /* If showing the plugin list, refresh it */
  if (!ui_state->cur_source && !ui_state->cur_container) {
    show_plugins ();
  }

  /* Also refresh the search combos */
  search_combo_setup ();
  query_combo_setup ();
}

static void
source_removed_cb (MsPluginRegistry *registry, gpointer user_data)
{
  g_debug ("Source '%s' is gone",
	   ms_metadata_source_get_name (MS_METADATA_SOURCE (user_data)));

  if (!ui_state->cur_source && !ui_state->cur_container) {
    /* If showing the plugin list, refresh it */
    show_plugins ();
  } else if ((gpointer)ui_state->cur_source == user_data ) {
    /* If we were browsing that source, cancel operation and  go back to
       plugin list view */
    g_debug ("Currently browsing the removed source: resetting UI.");
    reset_ui ();
  }

  /* Also refresh the search combo */
  search_combo_setup ();
  query_combo_setup ();
}

static void
load_plugins (void)
{
  MsPluginRegistry *registry;
  registry = ms_plugin_registry_get_instance ();
  if (!ms_plugin_registry_load_all (registry)) {
    g_error ("Failed to load plugins.");
  }
  g_signal_connect (registry, "source-added",
		    G_CALLBACK (source_added_cb), NULL);
  g_signal_connect (registry, "source-removed",
		    G_CALLBACK (source_removed_cb), NULL);
}

int
main (int argc, gchar *argv[])
{ 
  gtk_init (&argc, &argv);
  ms_log_init ("*:*");
  load_plugins ();
  ui_setup ();
  gtk_main ();
  return 0;
}
