/*
 * Copyright (C) 2010-2012 Igalia S.L.
 * Copyright (C) 2011 Intel Corporation.
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

#include <grilo.h>

#include <config.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <string.h>

#include "flickr-auth.h"

#define GRL_LOG_DOMAIN_DEFAULT  test_ui_log_domain
GRL_LOG_DOMAIN_STATIC(test_ui_log_domain);

/* ----- Flickr Security tokens ---- */

#define FLICKR_KEY    "fa037bee8120a921b34f8209d715a2fa"
#define FLICKR_SECRET "9f6523b9c52e3317"

#define FLICKR_AUTHORIZE_MSG                                            \
  "This application requires your authorization before it can read "    \
  "your personal photos on Flickr.\n\n"                                 \
  "Authorizing is a simple process which takes place in your web "      \
  "browser. Click on the link below, and when you are finished, "       \
  "return to this window and press OK button to complete "              \
  "authorization.\n\n"                                                  \
  "It is possible that you need to restart the application to "         \
  "apply the changes.\n\n"                                              \
  "If you do not authorize it, then you can not access your "           \
  "private photos."

/* ----- Youtube Config tokens ---- */

#define YOUTUBE_KEY   "AI39si4EfscPllSfUy1IwexMf__kntTL_G5dfSr2iUEVN45RHGq92Aq0lX25OlnOkG6KTN-4soVAkAf67fWYXuHfVADZYr7S1A"

/* ----- Vimeo Config tokens ---- */

#define VIMEO_KEY      "4d908c69e05a9d5b5c6669d302f920cb"
#define VIMEO_SECRET   "4a923ffaab6238eb"

/* ----- TMDb Config tokens ---- */

#define TMDB_KEY "719b9b296835b04cd919c4bf5220828a"

/* ----- Other ----- */

#define BROWSE_FLAGS (GRL_RESOLVE_FAST_ONLY | GRL_RESOLVE_IDLE_RELAY)
#define RESOLVE_FLAGS (GRL_RESOLVE_FULL | GRL_RESOLVE_IDLE_RELAY)

#define WINDOW_TITLE "Grilo Test UI (v." VERSION ")"

#define NOTIFICATION_TIMEOUT 5

#define BROWSER_MIN_WIDTH   320
#define BROWSER_MIN_HEIGHT  400

#define METADATA_MIN_WIDTH  320
#define METADATA_MIN_HEIGHT 400

#define BROWSE_CHUNK_SIZE   50
#define BROWSE_MAX_COUNT    200

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
  BROWSER_MODEL_ICON
};

enum {
  METADATA_MODEL_NAME = 0,
  METADATA_MODEL_VALUE
};

enum {
  SEARCH_MODEL_NAME = 0,
  SEARCH_MODEL_SOURCE
};

enum {
  QUERY_MODEL_NAME = 0,
  QUERY_MODEL_SOURCE
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
  GtkWidget *store_btn;
  GtkWidget *filter_audio;
  GtkWidget *filter_video;
  GtkWidget *filter_image;
  GtkWidget *remove_btn;
  GtkWidget *back_btn;
  GtkWidget *show_btn;
  GtkWidget *browser;
  GtkTreeModel *browser_model;
  GtkWidget *metadata;
  GtkTreeModel *metadata_model;
  GtkWidget *statusbar;
  guint statusbar_context_id;
} UiView;

typedef struct {
  gboolean changes_notification;

  /* Keeps track of our browsing position and history  */
  GList *source_stack;
  GList *container_stack;
  GrlSource *cur_source;
  GrlMedia *cur_container;

  /* Keeps track of the last element we showed metadata for */
  GrlSource *cur_md_source;
  GrlMedia *cur_md_media;

  /* Keeps track of browse/search state */
  gboolean op_ongoing;
  GrlSource *cur_op_source;
  guint cur_op_id;
  gboolean multiple;

  /* Keeps track of the URL of the item selected */
  const gchar *last_url;
} UiState;

typedef struct {
  enum OperationType {
    OP_TYPE_BROWSE,
    OP_TYPE_SEARCH,
    OP_TYPE_QUERY,
    OP_TYPE_MULTI_SEARCH
  } type;
  guint offset;
  guint count;
  gchar *text;
} OperationState;

typedef struct {
  GAppInfo *eog;
  GAppInfo *totem;
  GAppInfo *mplayer;
} UriLaunchers;

static UiView *view;
static UiState *ui_state;
static UriLaunchers *launchers;

static const gchar *ui_definition =
"<ui>"
" <menubar name='MainMenu'>"
"  <menu name='FileMenu' action='FileMenuAction' >"
"   <menuitem name='Authorize Flickr' action='AuthorizeFlickrAction' />"
"   <menuitem name='Shutdown plugins' action='ShutdownPluginsAction' />"
"   <menuitem name='Load all plugins' action='LoadAllPluginsAction' />"
"   <menuitem name='Changes notification' action='ChangesNotificationAction' />"
"   <menuitem name='Quit' action='QuitAction' />"
"  </menu>"
" </menubar>"
"</ui>";

static GrlOperationOptions *default_options = NULL;
static GrlOperationOptions *default_resolve_options = NULL;

static void show_browsable_sources (void);
static void quit_cb (GtkAction *action);

static gchar *authorize_flickr (void);
static void authorize_flickr_cb (GtkAction *action);

static void shutdown_plugins_cb (GtkAction *action);
static void shutdown_plugins (void);

static void load_all_plugins_cb (GtkAction *action);
static void load_all_plugins (void);

static void changes_notification_cb (GtkToggleAction *action);
static void content_changed_cb (GrlSource *source,
                                GPtrArray *changed_medias,
                                GrlSourceChangeType change_type,
                                gboolean location_unknown,
                                gpointer data);

static GtkActionEntry entries[] = {
  { "FileMenuAction", NULL, "_File" },
  { "AuthorizeFlickrAction", GTK_STOCK_CONNECT, "_Authorize Flickr", NULL,
    "AuthorizeFlickr", G_CALLBACK (authorize_flickr_cb) },
  { "ShutdownPluginsAction", GTK_STOCK_REFRESH, "_Shutdown Plugins", NULL,
    "ShutdownPlugins", G_CALLBACK (shutdown_plugins_cb) },
  { "LoadAllPluginsAction", GTK_STOCK_REFRESH, "_Load All Plugins", NULL,
    "LoadAllPlugins", G_CALLBACK (load_all_plugins_cb) },
  { "QuitAction", GTK_STOCK_QUIT, "_Quit", "<control>Q",
    "Quit", G_CALLBACK (quit_cb) }
};

static GtkToggleActionEntry toggle_entries[] = {
  { "ChangesNotificationAction", GTK_STOCK_FIND, "_Changes notification", NULL,
    "ChangesNotification", G_CALLBACK (changes_notification_cb), FALSE }
};

static void
quit_cb (GtkAction *action)
{
  gtk_main_quit ();
}

static void
authorize_flickr_cb (GtkAction *action)
{
  authorize_flickr ();
}

static void
shutdown_plugins_cb (GtkAction *action)
{
  shutdown_plugins ();
}

static void
load_all_plugins_cb (GtkAction *action)
{
  load_all_plugins ();
}

static void
changes_notification_cb (GtkToggleAction *action)
{
  GList *sources, *source;
  GrlRegistry *registry;

  ui_state->changes_notification = gtk_toggle_action_get_active (action);

  registry = grl_registry_get_default ();
  sources = grl_registry_get_sources (registry, FALSE);
  for (source = sources; source; source = g_list_next (source)) {
    if (grl_source_supported_operations (GRL_SOURCE (source->data)) &
        GRL_OP_NOTIFY_CHANGE) {
      if (ui_state->changes_notification) {
        grl_source_notify_change_start (GRL_SOURCE (source->data), NULL);
        g_signal_connect (GRL_SOURCE (source->data),
                          "content-changed",
                          G_CALLBACK (content_changed_cb),
                          NULL);
      } else {
        grl_source_notify_change_stop (GRL_SOURCE (source->data), NULL);
        g_signal_handlers_disconnect_by_func (source->data,
                                              content_changed_cb,
                                              NULL);
      }
    }
  }
  g_list_free (sources);
}

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
create_resolve_model (void)
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
    GRL_WARNING ("Failed to load icon %s: %s", icon_name,  error->message);
    g_error_free (error);
  }

  return pixbuf;
}

static GdkPixbuf *
get_icon_for_media (GrlMedia *media)
{
  if (GRL_IS_MEDIA_BOX (media)) {
    return load_icon (GTK_STOCK_DIRECTORY);
  } else if (GRL_IS_MEDIA_VIDEO (media)) {
    return load_icon ("gnome-mime-video");
  } else if (GRL_IS_MEDIA_AUDIO (media)) {
    return load_icon ("gnome-mime-audio");
  } else if (GRL_IS_MEDIA_IMAGE (media)) {
    return load_icon ("gnome-mime-image");
  } else {
    return load_icon (GTK_STOCK_FILE);
  }
}

static GList *
all_keys (void)
{
  GrlRegistry *registry;
  static GList *keys = NULL;

  if (!keys) {
    registry = grl_registry_get_default ();
    keys = grl_registry_get_metadata_keys (registry);
  }

  return keys;
}

static void
browse_history_push (GrlSource *source, GrlMedia *media)
{
  if (source)
    g_object_ref (source);
  if (media)
    g_object_ref (media);

  ui_state->source_stack = g_list_append (ui_state->source_stack, source);
  ui_state->container_stack = g_list_append (ui_state->container_stack, media);
}

static void
browse_history_pop (GrlSource **source, GrlMedia **media)
{
  GList *tmp;
  tmp = g_list_last (ui_state->source_stack);
  if (tmp) {
    *source = GRL_SOURCE (tmp->data);
    ui_state->source_stack = g_list_delete_link (ui_state->source_stack, tmp);
  }
  tmp = g_list_last (ui_state->container_stack);
  if (tmp) {
    *media = (GrlMedia *) tmp->data;
    ui_state->container_stack = g_list_delete_link (ui_state->container_stack,
						    tmp);
  }
}

static void
set_cur_browse (GrlSource *source, GrlMedia *media)
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
set_cur_resolve (GrlSource *source, GrlMedia *media)
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
  view->metadata_model = create_resolve_model ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (view->metadata),
                           view->metadata_model);

  gtk_widget_set_sensitive (view->show_btn, FALSE);
  ui_state->last_url = NULL;

  gtk_widget_set_sensitive (view->store_btn, FALSE);
  gtk_widget_set_sensitive (view->remove_btn, FALSE);
}

static void
clear_search_combo (void)
{
  if (view->search_combo_model) {
    gtk_list_store_clear (GTK_LIST_STORE (view->search_combo_model));
    g_object_unref (view->search_combo_model);
  }
  view->search_combo_model = create_search_combo_model ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (view->search_combo),
			   view->search_combo_model);
}

static void
clear_query_combo (void)
{
  if (view->query_combo_model) {
    gtk_list_store_clear (GTK_LIST_STORE (view->query_combo_model));
    g_object_unref (view->query_combo_model);
  }
  view->query_combo_model = create_query_combo_model ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (view->query_combo),
			   view->query_combo_model);
}

static void
clear_ui ()
{
  clear_panes ();
  clear_search_combo ();
  clear_query_combo ();
}

static void
cancel_current_operation (void)
{
  if (ui_state->op_ongoing) {
    grl_operation_cancel (ui_state->cur_op_id);
    ui_state->op_ongoing = FALSE;
  }
}

static gchar *
value_description (const GValue *value)
{
  if (value == NULL)
    return g_strdup ("");

  if (G_VALUE_HOLDS_BOXED (value)
      && G_VALUE_TYPE (value) == G_TYPE_DATE_TIME) {
    GDateTime *date_time = g_value_get_boxed (value);
    return g_date_time_format (date_time, "%FT%H:%M:%SZ");
  } else if (G_VALUE_HOLDS_STRING (value)) {
      return g_value_dup_string (value);
  }

  return g_strdup_value_contents (value);
}

static void
resolve_cb (GrlSource *source,
            guint operation_id,
            GrlMedia *media,
            gpointer user_data,
            const GError *error)
{
  GList *keys, *i;
  GtkTreeIter iter;
  GrlKeyID key;
  const gchar *key_name;

  /* Not interested if not the last media we
     requested metadata for */
  if (media != ui_state->cur_md_media) {
    return;
  }

  if (view->metadata_model) {
    gtk_list_store_clear (GTK_LIST_STORE (view->metadata_model));
    g_object_unref (view->metadata_model);
  }
  view->metadata_model = create_resolve_model ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (view->metadata),
			   view->metadata_model);

  if (error) {
    if (g_error_matches (error,
                         GRL_CORE_ERROR,
                         GRL_CORE_ERROR_OPERATION_CANCELLED)) {
      GRL_DEBUG ("Operation cancelled");
    } else {
      g_critical ("Error: %s", error->message);
      return;
    }
  }

  if (media) {
    keys = grl_data_get_keys (GRL_DATA (media));
    i = keys;
    while (i) {
      key = GRLPOINTER_TO_KEYID (i->data);
      key_name = grl_metadata_key_get_name (key);
      if (grl_data_has_key (GRL_DATA (media), key)) {
        const GValue *g_value = grl_data_get (GRL_DATA (media), key);
        GRL_DEBUG ("handling key %d (%s)", key, key_name);
        gchar *value = value_description (g_value);
        gtk_list_store_append (GTK_LIST_STORE (view->metadata_model), &iter);
        gtk_list_store_set (GTK_LIST_STORE (view->metadata_model),
                            &iter,
                            METADATA_MODEL_NAME,
                            key_name,
                            METADATA_MODEL_VALUE, value,
                            -1);
        GRL_DEBUG ("  %s: %s", key_name, value);
      }
      i = g_list_next (i);
    }

    g_list_free (keys);

    /* Don't free media (we do not ref it when issuing resolve(),
       so its reference comes from the treeview and that's freed
       when the treeview is cleared */

    /* Set/unset show button */
    if ((GRL_IS_MEDIA_AUDIO (media) ||
         GRL_IS_MEDIA_VIDEO (media) ||
         GRL_IS_MEDIA_IMAGE (media)) &&
        (ui_state->last_url = grl_media_get_url (media))) {
      gtk_widget_set_sensitive (view->show_btn, TRUE);
    } else {
      gtk_widget_set_sensitive (view->show_btn, FALSE);
      ui_state->last_url = NULL;
    }
  }
}

static void
operation_started (GrlSource *source, guint operation_id,
                   gboolean multiple)
{
  ui_state->op_ongoing = TRUE;
  ui_state->cur_op_source = source;
  ui_state->cur_op_id = operation_id;
  ui_state->multiple = multiple;

  /* Set busy cursor */
  GdkCursor *cursor;
  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor(gtk_widget_get_window (view->window), cursor);
  g_object_unref (cursor);
}

static void
operation_finished (void)
{
  ui_state->op_ongoing = FALSE;

  /* Set default cursor */
  gdk_window_set_cursor(gtk_widget_get_window (view->window), NULL);
}

static void
browse_search_query_cb (GrlSource *source,
                        guint op_id,
                        GrlMedia *media,
                        guint remaining,
                        gpointer user_data,
                        const GError *error)
{
  gint type;
  const gchar *name;
  GtkTreeIter iter;
  GdkPixbuf *icon;
  OperationState *state = (OperationState *) user_data;
  guint next_op_id;

  if (error) {
    if (g_error_matches (error,
                         GRL_CORE_ERROR,
                         GRL_CORE_ERROR_OPERATION_CANCELLED)) {
      GRL_DEBUG ("Operation cancelled");
    } else {
      g_critical ("Error: %s", error->message);
    }
  }

  state->count++;

  if (media) {
    icon = get_icon_for_media (media);
    name = grl_media_get_title (media);
    if (GRL_IS_MEDIA_BOX (media)) {
      gint childcount =
        grl_media_box_get_childcount (GRL_MEDIA_BOX (media));
      type = OBJECT_TYPE_CONTAINER;
      if (childcount != GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN) {
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
    if (icon) {
      g_object_unref (icon);
    }
  }

  if (remaining == 0) {
    /* Done with this chunk, check if there is more to browse */
    if (ui_state->op_ongoing &&
	ui_state->cur_op_id == op_id &&
	media != NULL) {
      /* Operation is still valid, so let's go */
      state->offset += state->count;
      if (state->count >= BROWSE_CHUNK_SIZE &&
	  state->offset < BROWSE_MAX_COUNT) {
        GrlOperationOptions *options =
            grl_operation_options_copy (default_options);
          GrlTypeFilter filter = GRL_TYPE_FILTER_NONE;
          if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view->filter_audio))) {
            filter |= GRL_TYPE_FILTER_AUDIO;
          }
          if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view->filter_video))) {
            filter |= GRL_TYPE_FILTER_VIDEO;
          }
          if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view->filter_image))) {
            filter |= GRL_TYPE_FILTER_IMAGE;
          }
          grl_operation_options_set_type_filter (options, filter);

	GRL_DEBUG ("operation (%d) requesting more data from source", op_id);
	state->count = 0;

   grl_operation_options_set_skip (options, state->offset);
   grl_operation_options_set_count (options, BROWSE_CHUNK_SIZE);

   GrlOperationOptions *supported_options = NULL;
   grl_operation_options_obey_caps (options,
                                    grl_source_get_caps (source, GRL_OP_SEARCH),
                                    &supported_options,
                                    NULL);
	switch (state->type) {
	  case OP_TYPE_BROWSE:
	    next_op_id =
	      grl_source_browse (source,
				       ui_state->cur_container,
				       all_keys (),
				       supported_options,
				       browse_search_query_cb,
				       state);
	    break;
	  case OP_TYPE_SEARCH:
	    next_op_id =
	      grl_source_search (source,
				       state->text,
				       all_keys (),
				       supported_options,
				       browse_search_query_cb,
				       state);
	    break;
	  case OP_TYPE_QUERY:
	    next_op_id =
	      grl_source_query (source,
				      state->text,
				      all_keys (),
				      supported_options,
				      browse_search_query_cb,
				      state);
	    break;
	  case OP_TYPE_MULTI_SEARCH:
	    /* this shouldn't happen as multiple search has no chunk
	     * size parameter */
	    g_warn_if_reached ();
	    g_object_unref (options);
	    goto operation_finished;
	    break;
	}
	g_object_unref (options);
   g_object_unref (supported_options);
	operation_started (source, next_op_id, FALSE);
      } else {
	/* We browsed all requested elements  */
	goto operation_finished;
      }
    } else {
      /* The operation was cancelled */
      goto operation_finished;
    }
  }

  return;

 operation_finished:
  g_free (state);
  operation_finished ();
  GRL_DEBUG ("**** operation finished (%d) ****", op_id);
}

static void
browse (GrlSource *source, GrlMedia *container)
{
  guint browse_id;
  if (source) {
    /* If we have an ongoing operation, cancel it first */
    cancel_current_operation ();
    clear_panes ();

    OperationState *state = g_new0 (OperationState, 1);
    state->type = OP_TYPE_BROWSE;
    browse_id = grl_source_browse (source,
                                   container,
                                   all_keys (),
                                   default_options,
                                   browse_search_query_cb,
                                   state);
    operation_started (source, browse_id, FALSE);
  } else {
    show_browsable_sources ();
  }

  set_cur_browse (source, container);
  set_cur_resolve (NULL, NULL);
}

static void
browser_activated_cb (GtkTreeView *tree_view,
		      GtkTreePath *path,
		      GtkTreeViewColumn *column,
		      gpointer user_data)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GrlMedia *content;
  gint type;
  GrlSource *source;
  GrlMedia *container;

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
resolve (GrlSource *source, GrlMedia *media)
{
  if (source) {
    grl_source_resolve (source,
                        media,
                        all_keys (),
                        default_resolve_options,
                        resolve_cb,
                        NULL);
  }
}

static void
browser_row_selected_cb (GtkTreeView *tree_view,
			 gpointer user_data)
{
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  GrlSource *source;
  GrlMedia *content;

  gtk_tree_view_get_cursor (tree_view, &path, NULL);
  if (!path) {
    return;
  }
  gtk_tree_model_get_iter (view->browser_model, &iter, path);
  gtk_tree_model_get (view->browser_model,
		      &iter,
		      BROWSER_MODEL_SOURCE, &source,
		      BROWSER_MODEL_CONTENT, &content,
		      -1);

  if (source != ui_state->cur_md_source ||
      content != ui_state->cur_md_media) {
    set_cur_resolve (source, content);
    resolve (source, content);
  }

  /* Check if we can store content in the selected item */
  if (content == NULL &&
      (grl_source_supported_operations (GRL_SOURCE (source)) &
       GRL_OP_STORE)) {
    gtk_widget_set_sensitive (view->store_btn, TRUE);
  } else if (content && GRL_IS_MEDIA_BOX (content) &&
	     grl_source_supported_operations (GRL_SOURCE (source)) &
	     GRL_OP_STORE_PARENT) {
    gtk_widget_set_sensitive (view->store_btn, TRUE);
  } else {
    gtk_widget_set_sensitive (view->store_btn, FALSE);
  }

  /* Check if we can remove the selected item */
  if (content != NULL &&
      (grl_source_supported_operations (GRL_SOURCE (source)) &
       GRL_OP_REMOVE)) {
    gtk_widget_set_sensitive (view->remove_btn, TRUE);
  } else {
    gtk_widget_set_sensitive (view->remove_btn, FALSE);
  }

  g_object_unref (source);
  if (content)
    g_object_unref (content);
}

static void
show_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  GList *uri_list = NULL;
  GError *error = NULL;
  GAppInfo *app = NULL;

  if (ui_state->last_url) {
    GRL_DEBUG ("playing: %s", ui_state->last_url);
    uri_list = g_list_append (uri_list, (gpointer) ui_state->last_url);
    if (GRL_IS_MEDIA_IMAGE (ui_state->cur_md_media)) {
      app = launchers->eog;
    } else {
      /* Content from apple-trailers should be opened with mplayer, as they
         require to change the user-agent */
      if (strcmp (grl_data_get_string (GRL_DATA (ui_state->cur_md_media),
                                       GRL_METADATA_KEY_SOURCE),
                  "grl-apple-trailers") == 0) {
        app = launchers->mplayer;
      } else {
        app = launchers->totem;
      }
    }

    g_app_info_launch_uris (app, uri_list, NULL, &error);
    g_list_free (uri_list);

    if (error) {
      GRL_WARNING ("Cannot use '%s' to show '%s'; using default application",
                   g_app_info_get_name (app),
                   ui_state->last_url);
      g_error_free (error);
      error = NULL;
      g_app_info_launch_default_for_uri (ui_state->last_url, NULL, &error);
      if (error) {
        GRL_WARNING ("Cannot use default application to show '%s'. "
                     "Stopping playback", ui_state->last_url);
        g_error_free (error);
      }
    }
  }
}

static void
back_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  GrlSource *prev_source = NULL;
  GrlMedia *prev_container = NULL;

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
store_cb (GrlSource *source,
          GrlMedia *media,
          GList *failed_keys,
          gpointer user_data,
          const GError *error)
{
  if (error) {
    GRL_WARNING ("Error storing media: %s", error->message);
  } else {
    GRL_DEBUG ("Media stored");
  }
  g_object_unref (media);
}

static void
store_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  GtkWidget *dialog;
  GtkTreeSelection *sel;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  GrlSource *source;
  GrlMedia *container;

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->browser));
  gtk_tree_selection_get_selected (sel, &model, &iter);
  gtk_tree_model_get (view->browser_model, &iter,
		      BROWSER_MODEL_SOURCE, &source,
		      BROWSER_MODEL_CONTENT, &container,
		      -1);

  dialog =
    gtk_dialog_new_with_buttons ("Store content",
				 GTK_WINDOW (view->window),
				 GTK_DIALOG_MODAL |
				 GTK_DIALOG_DESTROY_WITH_PARENT,
				 GTK_STOCK_OK, GTK_RESPONSE_OK,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 NULL);
  GtkWidget *ca = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *l1 = gtk_label_new ("Title:");
  GtkWidget *e1 = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (box), l1, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box), e1, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (ca), box, TRUE, TRUE, 0);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *l2 = gtk_label_new ("URL:");
  GtkWidget *e2 = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (box), l2, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box), e2, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (ca), box, TRUE, TRUE, 0);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *l3 = gtk_label_new ("Desc:");
  GtkWidget *e3 = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (box), l3, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box), e3, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (ca), box, TRUE, TRUE, 0);

  gtk_widget_show_all (dialog);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)  {
    GrlMedia *media;
    const gchar *url = gtk_entry_get_text (GTK_ENTRY (e2));
    if (!url || !url[0]) {
      media = grl_media_box_new ();
    } else {
      media = grl_media_new ();
      grl_media_set_url (media, url);
    }
    grl_media_set_title (media, gtk_entry_get_text (GTK_ENTRY (e1)));
    grl_media_set_description (media,
                                    gtk_entry_get_text (GTK_ENTRY (e3)));
    grl_source_store (source, GRL_MEDIA_BOX (container),
                      media, GRL_WRITE_FULL, store_cb, NULL);
  }

  gtk_widget_destroy (dialog);

  if (source) {
    g_object_unref (source);
  }
  if (container) {
    g_object_unref (container);
  }
}

static void
remove_item_from_view (GrlSource *source, GrlMedia *media)
{
  GtkTreeIter iter;
  GrlSource *iter_source;
  GrlMedia *iter_media;
  gboolean found = FALSE;
  gboolean more;

  more = gtk_tree_model_get_iter_first (view->browser_model, &iter);
  while (more && !found) {
    gtk_tree_model_get (view->browser_model, &iter,
			BROWSER_MODEL_SOURCE, &iter_source,
			BROWSER_MODEL_CONTENT, &iter_media,
			-1);
    if (iter_source == source && iter_media == media) {
      gtk_list_store_remove (GTK_LIST_STORE (view->browser_model), &iter);
      found = TRUE;
    } else {
      more = gtk_tree_model_iter_next (view->browser_model, &iter);
    }
    if (source) {
      g_object_unref (iter_source);
    }
    if (media) {
      g_object_unref (iter_media);
    }
  }
}

static void
remove_cb (GrlSource *source,
           GrlMedia *media,
           gpointer user_data,
           const GError *error)
{
  if (error) {
    GRL_WARNING ("Error removing media: %s", error->message);
  } else {
    GRL_DEBUG ("Media removed");
  }

  remove_item_from_view (source, media);
}

static void
remove_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  GtkTreeSelection *sel;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  GrlSource *source;
  GrlMedia *media;

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->browser));
  gtk_tree_selection_get_selected (sel, &model, &iter);
  gtk_tree_model_get (view->browser_model, &iter,
		      BROWSER_MODEL_SOURCE, &source,
		      BROWSER_MODEL_CONTENT, &media,
		      -1);

  grl_source_remove (source, media, remove_cb, NULL);

  if (source) {
    g_object_unref (source);
  }
  if (media) {
    g_object_unref (media);
  }
}

static void
search (GrlSource *source, const gchar *text)
{
  OperationState *state;
  guint search_id;
  gboolean multiple = FALSE;
  GrlOperationOptions *options;
  GrlOperationOptions *supported_options;
  GrlTypeFilter filter;

  /* If we have an operation ongoing, let's cancel it first */
  cancel_current_operation ();

  state = g_new0 (OperationState, 1);
  state->text = (gchar *) text;
  options = grl_operation_options_copy (default_options);
  filter = GRL_TYPE_FILTER_NONE;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view->filter_audio))) {
    filter |= GRL_TYPE_FILTER_AUDIO;
  }
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view->filter_video))) {
    filter |= GRL_TYPE_FILTER_VIDEO;
  }
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view->filter_image))) {
    filter |= GRL_TYPE_FILTER_IMAGE;
  }
  grl_operation_options_set_type_filter (options, filter);

  if (source) {
    /* Normal search */
    state->type = OP_TYPE_SEARCH;
    grl_operation_options_obey_caps (options,
                                     grl_source_get_caps (GRL_SOURCE (source), GRL_OP_SEARCH),
                                     &supported_options,
                                     NULL);
    g_object_unref (options);
    search_id = grl_source_search (source,
					 text,
					 all_keys (),
					 supported_options,
					 browse_search_query_cb,
					 state);
    g_object_unref (supported_options);
  } else {
    /* Multiple search (all sources) */
    multiple = TRUE;
    state->type = OP_TYPE_MULTI_SEARCH;
    search_id = grl_multiple_search (NULL,
				     text,
				     all_keys (),
				     options,
				     browse_search_query_cb,
				     state);
  }
  clear_panes ();
  operation_started (source, search_id, multiple);
}

static void
search_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  GtkTreeIter iter;

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (view->search_combo),
				     &iter)) {
    GrlSource *source;
    const gchar *text;
    gtk_tree_model_get (view->search_combo_model, &iter,
			SEARCH_MODEL_SOURCE, &source,
			-1);
    text = gtk_entry_get_text (GTK_ENTRY (view->search_text));
    /* Special case: empty search means search all */
    if (text[0] == '\0') {
      search (source, NULL);
    } else {
      search (source, text);
    }

    if (source) {
      g_object_unref (source);
    }
  }
}

static void
query (GrlSource *source, const gchar *text)
{
  OperationState *state;
  guint query_id;

  /* If we have an operation ongoing, let's cancel it first */
  cancel_current_operation ();

  state = g_new0 (OperationState, 1);
  state->text = (gchar *) text;
  state->type = OP_TYPE_QUERY;
  query_id = grl_source_query (source,
                               text,
                               all_keys (),
                               default_options,
                               browse_search_query_cb,
                               state);
  clear_panes ();
  operation_started (source, query_id, FALSE);
}

static void
query_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  GtkTreeIter iter;

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (view->query_combo),
				     &iter)) {
    GrlSource *source;
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
set_filter_cb (GtkComboBox *widget,
               gpointer user_data)
{
  GrlCaps *caps;
  GrlTypeFilter filter;
  GrlSource *source;
  GtkTreeIter iter;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (view->filter_audio), TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (view->filter_image), TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (view->filter_video), TRUE);

  gtk_widget_set_sensitive (view->filter_audio, FALSE);
  gtk_widget_set_sensitive (view->filter_video, FALSE);
  gtk_widget_set_sensitive (view->filter_image, FALSE);

  if (!gtk_combo_box_get_active_iter (widget, &iter)) {
    return;
  }

  gtk_tree_model_get (gtk_combo_box_get_model (widget),
                      &iter,
                      SEARCH_MODEL_SOURCE, &source,
                      -1);
  if (!source) {
    return;
  }

  caps = grl_source_get_caps (GRL_SOURCE (source), GRL_OP_SEARCH);


  filter = grl_caps_get_type_filter (caps);

  if (filter & GRL_TYPE_FILTER_AUDIO) {
    gtk_widget_set_sensitive (view->filter_audio, TRUE);
  }
  if (filter & GRL_TYPE_FILTER_VIDEO) {
    gtk_widget_set_sensitive (view->filter_video, TRUE);
  }
  if (filter & GRL_TYPE_FILTER_IMAGE) {
    gtk_widget_set_sensitive (view->filter_image, TRUE);
  }
}

static void
query_combo_setup (void)
{
  GrlRegistry *registry;
  GList *sources = NULL;
  GList *sources_iter;
  GtkTreeIter iter;

  clear_query_combo ();

  registry = grl_registry_get_default ();
  sources = grl_registry_get_sources_by_operations (registry,
                                                    GRL_OP_QUERY,
                                                    FALSE);
  for (sources_iter = sources; sources_iter;
      sources_iter = g_list_next (sources_iter)) {
    GrlSource *source = GRL_SOURCE (sources_iter->data);
    const gchar *name = grl_source_get_name (source);

    gtk_list_store_append (GTK_LIST_STORE (view->query_combo_model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (view->query_combo_model),
			&iter,
			QUERY_MODEL_SOURCE, source,
			QUERY_MODEL_NAME, name,
			-1);
  }
  g_list_free (sources);

  gtk_combo_box_set_active (GTK_COMBO_BOX (view->query_combo), 0);
}

static void
search_combo_setup (void)
{
  GrlRegistry *registry;
  GList *sources = NULL;
  GList *sources_iter;
  GtkTreeIter iter;

  clear_search_combo ();

  registry = grl_registry_get_default ();
  sources = grl_registry_get_sources_by_operations (registry,
                                                    GRL_OP_SEARCH,
                                                    FALSE);
  for (sources_iter = sources; sources_iter;
      sources_iter = g_list_next (sources_iter)) {
    GrlSource *source = GRL_SOURCE (sources_iter->data);
    const gchar *name = grl_source_get_name (source);

    gtk_list_store_append (GTK_LIST_STORE (view->search_combo_model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (view->search_combo_model),
			&iter,
			SEARCH_MODEL_SOURCE, source,
			SEARCH_MODEL_NAME, name,
			-1);
  }
  g_list_free (sources);

  /* Add "All" option */
  gtk_list_store_append (GTK_LIST_STORE (view->search_combo_model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (view->search_combo_model),
		      &iter,
		      SEARCH_MODEL_SOURCE, NULL,
		      SEARCH_MODEL_NAME, "All",
		      -1);

  gtk_combo_box_set_active (GTK_COMBO_BOX (view->search_combo), 0);
}

static gchar *
get_config_dir ()
{
  char *confdir;
  GFile *dir;
  GError *error = NULL;

  confdir = g_build_filename (g_get_user_config_dir (),
                              "grilo-test-ui",
                              NULL);

  /* create the configuration directory if needed */
  if (g_file_test (confdir, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
      return confdir;

  dir = g_file_new_for_path (confdir);
  g_file_make_directory_with_parents (dir, NULL, &error);
  g_object_unref (dir);

  if (error && error->code != G_IO_ERROR_EXISTS) {
    g_critical ("Could not create config directory %s: %s",
                confdir, error->message);
    g_clear_error (&error);
    return NULL;
  }

  return confdir;
}

static gchar *
load_flickr_token (void)
{
  GKeyFile *keyfile;
  gchar *path;
  gchar *file = NULL;
  gchar *token = NULL;

  path = get_config_dir ();
  if (path) {
    file = g_build_filename (path, "tokens.conf", NULL);
    g_free (path);
  }

  if (!file)
    return NULL;

  keyfile = g_key_file_new ();
  if (!g_key_file_load_from_file (keyfile, file, G_KEY_FILE_NONE, NULL))
    goto bailout;

  token = g_key_file_get_value (keyfile, "flickr", "auth-token", NULL);

bailout:
  g_free (file);
  g_key_file_free (keyfile);
  return token;
}

static void
save_flickr_token (const gchar *token)
{
  GKeyFile *keyfile;
  gchar *path;
  gchar *file = NULL;

  path = get_config_dir ();
  if (path) {
    file = g_build_filename (path, "tokens.conf", NULL);
    g_free (path);
  }

  if (!file)
    return;

  keyfile = g_key_file_new ();
  g_key_file_load_from_file (keyfile, file, G_KEY_FILE_NONE, NULL);
  g_key_file_set_value (keyfile, "flickr", "auth-token", token);

  {
    GError *error = NULL;
    gchar *content = g_key_file_to_data (keyfile, NULL, NULL);
    g_file_set_contents (file, content, -1, &error);
    if (error) {
      g_warning ("Could not write %s: %s", file, error->message);
      g_clear_error (&error);
    }
    g_free (content);
  }

  g_free (file);
  g_key_file_free (keyfile);
}

static void
activate_ok_button (GtkLabel *label,
                    gchar *uri,
                    gpointer user_data)
{
  GRL_DEBUG ("activate invoked");
  gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (label)),
                uri,
                GDK_CURRENT_TIME,
                NULL);
  gtk_widget_set_sensitive (user_data, TRUE);
}

static void
load_file_config (void)
{
  GrlRegistry *registry;
  gchar *config_file;

  registry = grl_registry_get_default ();
  config_file = g_strconcat (g_get_user_config_dir(),
                             G_DIR_SEPARATOR_S, "grilo-test-ui",
                             G_DIR_SEPARATOR_S, "grilo.conf",
                             NULL);
  if (g_file_test (config_file, G_FILE_TEST_EXISTS)) {
    grl_registry_add_config_from_file (registry, config_file, NULL);
  }
  g_free (config_file);
}

static gchar *
authorize_flickr (void)
{
  GtkWidget *dialog;
  GtkWidget *fail_dialog;
  GtkWidget *label;
  GtkWidget *view;
  gchar *markup;
  gchar *token = NULL;
  gchar *login_link;
  GtkWidget *ok_button;

  gchar *frob = flickr_get_frob (FLICKR_KEY, FLICKR_SECRET);
  if (!frob) {
    GRL_WARNING ("Unable to obtain a Flickr's frob");
    return NULL;
  }

  login_link = flickr_get_login_link (FLICKR_KEY, FLICKR_SECRET, frob, "read");
  view = gtk_text_view_new ();
  gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)),
                            FLICKR_AUTHORIZE_MSG,
                            -1);
  gtk_text_view_set_justification (GTK_TEXT_VIEW (view), GTK_JUSTIFY_FILL);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);

  markup =
    g_markup_printf_escaped ("<a href=\"%s\">READ-ONLY AUTHORIZE</a>",
                             login_link);
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), markup);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

  dialog =
    gtk_dialog_new_with_buttons ("Authorize Flickr access",
                                 GTK_WINDOW (gtk_widget_get_parent_window (view)),
                                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                 NULL, NULL);

  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), view, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), label, TRUE, TRUE, 0);

  ok_button = gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_OK, GTK_RESPONSE_OK);
  gtk_widget_set_sensitive (ok_button, FALSE);
  gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

  g_signal_connect (G_OBJECT (label),
                    "activate-link",
                    G_CALLBACK (activate_ok_button),
                    ok_button);

  gtk_widget_show_all (dialog);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {
    token = flickr_get_token (FLICKR_KEY, FLICKR_SECRET, frob);
    if (token) {
      save_flickr_token (token);
    } else {
      fail_dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_parent_window (view)),
                                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_OK,
                                            "Authorization failed. Retry later");
      gtk_dialog_run (GTK_DIALOG (fail_dialog));
      gtk_widget_destroy (dialog);
    }
  }

  gtk_widget_destroy (dialog);
  g_free (frob);
  g_free (login_link);
  g_free (markup);

  return token;
}

static void
set_flickr_config (void)
{
  GrlConfig *config;
  GrlRegistry *registry;
  gchar *token;

  registry = grl_registry_get_default ();

  config = grl_config_new ("grl-flickr", NULL);
  grl_config_set_api_key (config, FLICKR_KEY);
  grl_config_set_api_secret (config, FLICKR_SECRET);
  grl_registry_add_config (registry, config, NULL);

  token = load_flickr_token ();

  if (!token) {
    token = authorize_flickr ();
    if (!token) {
      /* Save empty token to avoid asking again */
      save_flickr_token ("");
    }
  }

  if (token && token[0] != '\0') {
    config = grl_config_new ("grl-flickr", NULL);
    grl_config_set_api_key (config, FLICKR_KEY);
    grl_config_set_api_secret (config, FLICKR_SECRET);
    grl_config_set_api_token (config, token);
    grl_registry_add_config (registry, config, NULL);
  }
  g_free (token);
}

static void
set_youtube_config (void)
{
  GrlConfig *config;
  GrlRegistry *registry;

  config = grl_config_new ("grl-youtube", NULL);
  grl_config_set_api_key (config, YOUTUBE_KEY);

  registry = grl_registry_get_default ();
  grl_registry_add_config (registry, config, NULL);
}

static void
set_vimeo_config (void)
{
  GrlConfig *config;
  GrlRegistry *registry;

  config = grl_config_new ("grl-vimeo", NULL);
  grl_config_set_api_key (config, VIMEO_KEY);
  grl_config_set_api_secret (config, VIMEO_SECRET);

  registry = grl_registry_get_default ();
  grl_registry_add_config (registry, config, NULL);
}

static void
set_tmdb_config (void)
{
  GrlConfig *config;
  GrlRegistry *registry;

  config =grl_config_new ("grl-tmdb", NULL);
  grl_config_set_api_key (config, TMDB_KEY);

  registry = grl_registry_get_default ();
  grl_registry_add_config (registry, config, NULL);
}

static void
set_local_config (void)
{
  GrlConfig *config;
  GrlRegistry *registry;

  config = grl_config_new ("grl-local-metadata", NULL);
  grl_config_set_boolean (config, "guess-video", TRUE);

  registry = grl_registry_get_default ();
  grl_registry_add_config (registry, config, NULL);
}

static void
launchers_setup (void)
{
  launchers = g_new0 (UriLaunchers, 1);
  launchers->eog = g_app_info_create_from_commandline ("eog",
                                                       "Eye of GNOME (eog)",
                                                       G_APP_INFO_CREATE_SUPPORTS_URIS,
                                                       NULL);
  launchers->totem = g_app_info_create_from_commandline ("totem",
                                                         "Totem",
                                                         G_APP_INFO_CREATE_SUPPORTS_URIS,
                                                         NULL);
  launchers->mplayer =
    g_app_info_create_from_commandline ("mplayer -user-agent \"QuickTime\" -cache 5000",
                                        "The Movie Player (mplayer)",
                                        G_APP_INFO_CREATE_SUPPORTS_URIS | G_APP_INFO_CREATE_NEEDS_TERMINAL,
                                        NULL);
}

static void
options_setup (void)
{
  default_options = grl_operation_options_new (NULL);
  grl_operation_options_set_flags (default_options, BROWSE_FLAGS);
  grl_operation_options_set_skip (default_options, 0);
  grl_operation_options_set_count (default_options, BROWSE_CHUNK_SIZE);

  default_resolve_options = grl_operation_options_new (NULL);
  grl_operation_options_set_flags (default_resolve_options, RESOLVE_FLAGS);
}

static void
ui_setup (void)
{
  view = g_new0 (UiView, 1);
  ui_state = g_new0 (UiState, 1);

  /* Main window */
  view->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (view->window), WINDOW_TITLE);
  gtk_window_resize (GTK_WINDOW (view->window), 600, 400);
  g_signal_connect (G_OBJECT (view->window), "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  GtkActionGroup *actions = gtk_action_group_new ("actions");
  gtk_action_group_add_actions (actions, entries, G_N_ELEMENTS (entries), NULL);
  gtk_action_group_add_toggle_actions (actions, toggle_entries, G_N_ELEMENTS (toggle_entries), NULL);

  GtkUIManager *uiman = gtk_ui_manager_new ();
  gtk_ui_manager_insert_action_group (uiman, actions, 0);
  gtk_window_add_accel_group (GTK_WINDOW (view->window),
                              gtk_ui_manager_get_accel_group (uiman));
  gtk_ui_manager_add_ui_from_string (uiman, ui_definition, -1, NULL);

  GtkWidget *mainbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (mainbox),
                      gtk_ui_manager_get_widget (uiman, "/MainMenu"),
                      FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (view->window), mainbox);

  /* Main layout */
  GtkWidget *box = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  view->lpane = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  view->rpane = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (mainbox), box, TRUE, TRUE, 0);
  gtk_paned_add1 (GTK_PANED (box), view->lpane);
  gtk_paned_add2 (GTK_PANED (box), view->rpane);

  /* Search & Query */
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  view->search_text = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), view->search_text, TRUE, TRUE, 0);
  view->query_text = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), view->query_text, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
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
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);


  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  view->search_btn = gtk_button_new_with_label ("Search");
  gtk_container_add_with_properties (GTK_CONTAINER (vbox), view->search_btn,
				     "expand", FALSE, NULL);
  view->query_btn = gtk_button_new_with_label ("Query");
  gtk_container_add_with_properties (GTK_CONTAINER (vbox), view->query_btn,
				     "expand", FALSE, NULL);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  gtk_container_add_with_properties (GTK_CONTAINER (view->lpane), hbox,
				     "expand", FALSE, NULL);

  g_signal_connect (view->search_btn, "clicked",
		    G_CALLBACK (search_btn_clicked_cb), NULL);
  g_signal_connect (view->query_btn, "clicked",
		    G_CALLBACK (query_btn_clicked_cb), NULL);
  search_combo_setup ();
  query_combo_setup ();

  /* Advanced search */
  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  view->filter_audio = gtk_check_button_new_with_label ("Audio");
  view->filter_video = gtk_check_button_new_with_label ("Video");
  view->filter_image = gtk_check_button_new_with_label ("Image");
  gtk_widget_set_sensitive (view->filter_audio, FALSE);
  gtk_widget_set_sensitive (view->filter_video, FALSE);
  gtk_widget_set_sensitive (view->filter_image, FALSE);

  gtk_container_add_with_properties (GTK_CONTAINER (box),
                                     view->filter_audio,
                                     "expand", FALSE, NULL);
  gtk_container_add_with_properties (GTK_CONTAINER (box),
                                     view->filter_video,
                                     "expand", FALSE, NULL);
  gtk_container_add_with_properties (GTK_CONTAINER (box),
                                     view->filter_image,
                                     "expand", FALSE, NULL);
  gtk_container_add_with_properties (GTK_CONTAINER (view->lpane),
                                     box,
                                     "expand", FALSE, NULL);
  g_signal_connect (G_OBJECT (view->search_combo),
                    "changed",
                    G_CALLBACK (set_filter_cb), NULL);

  /* Toolbar buttons */
  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  view->back_btn = gtk_button_new ();
  gtk_button_set_image (GTK_BUTTON (view->back_btn),
			gtk_image_new_from_stock (GTK_STOCK_GO_BACK,
						  GTK_ICON_SIZE_BUTTON));
  view->store_btn = gtk_button_new ();
  gtk_button_set_image (GTK_BUTTON (view->store_btn),
			gtk_image_new_from_stock (GTK_STOCK_ADD,
						  GTK_ICON_SIZE_BUTTON));
  view->remove_btn = gtk_button_new ();
  gtk_button_set_image (GTK_BUTTON (view->remove_btn),
			gtk_image_new_from_stock (GTK_STOCK_REMOVE,
						  GTK_ICON_SIZE_BUTTON));

  gtk_container_add_with_properties (GTK_CONTAINER (box),
				     view->back_btn,
				     "expand", FALSE, NULL);
  gtk_container_add_with_properties (GTK_CONTAINER (box),
				     view->store_btn,
				     "expand", FALSE, NULL);
  gtk_container_add_with_properties (GTK_CONTAINER (box),
				     view->remove_btn,
				     "expand", FALSE, NULL);
  gtk_container_add_with_properties (GTK_CONTAINER (view->lpane),
				     box, "expand", FALSE, NULL);

  g_signal_connect (view->back_btn, "clicked",
		    G_CALLBACK (back_btn_clicked_cb), NULL);
  g_signal_connect (view->store_btn, "clicked",
		    G_CALLBACK (store_btn_clicked_cb), NULL);
  g_signal_connect (view->remove_btn, "clicked",
		    G_CALLBACK (remove_btn_clicked_cb), NULL);
  gtk_widget_set_sensitive (view->store_btn, FALSE);
  gtk_widget_set_sensitive (view->remove_btn, FALSE);

  /* Contents tree view */
  GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  view->browser = gtk_tree_view_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view->browser), FALSE);

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
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_insert_column (GTK_TREE_VIEW (view->browser), col, -1);

  gtk_container_add (GTK_CONTAINER (scroll), view->browser);
  gtk_box_pack_start (GTK_BOX (view->lpane), scroll, TRUE, TRUE, 0);
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

  GtkCellRenderer *col_renders_md[2];
  gchar *col_attributes_md[] = {"text", "text"};
  gint col_model_md[2] = { METADATA_MODEL_NAME, METADATA_MODEL_VALUE};
  col_renders_md[0] = gtk_cell_renderer_text_new ();
  col_renders_md[1] = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT,
				    "editable", TRUE, NULL);
  col = gtk_tree_view_column_new ();
  for (i=0; i<2; i++) {
    gtk_tree_view_column_pack_start (col, col_renders_md[i], FALSE);
    gtk_tree_view_column_add_attribute (col,
					col_renders_md[i],
					col_attributes_md[i],
					col_model_md[i]);
  }
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_insert_column (GTK_TREE_VIEW (view->metadata), col, -1);

  gtk_container_add (GTK_CONTAINER (scroll_md), view->metadata);
  gtk_box_pack_start (GTK_BOX (view->rpane), scroll_md, TRUE, TRUE, 0);

  /* Status bar */
  view->statusbar = gtk_statusbar_new ();

#if GTK_CHECK_VERSION (2, 91, 0)
  gtk_window_set_has_resize_grip (GTK_WINDOW (view->window), FALSE);
#else
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (view->statusbar), FALSE);
#endif

  view->statusbar_context_id =
    gtk_statusbar_get_context_id (GTK_STATUSBAR (view->statusbar),
                                  "changes notification");
  gtk_container_add_with_properties (GTK_CONTAINER (view->rpane),
                                     view->statusbar,
                                     "expand", FALSE, NULL);

  gtk_widget_set_size_request (view->metadata,
			       METADATA_MIN_WIDTH,
			       METADATA_MIN_HEIGHT);

  /* Populate the browser with the sources */
  show_browsable_sources ();

  gtk_widget_show_all (view->window);
}

static void
show_browsable_sources ()
{
  GList *sources;
  GList *sources_iter;
  GtkTreeIter iter;
  GrlRegistry *registry;

  registry = grl_registry_get_default ();

  clear_panes ();

  sources = grl_registry_get_sources_by_operations (registry,
                                                    GRL_OP_BROWSE,
                                                    FALSE);
  for (sources_iter = sources; sources_iter;
      sources_iter = g_list_next (sources_iter)) {
    GrlSource *source;
    const gchar *name;
    GdkPixbuf *icon;

    source = GRL_SOURCE (sources_iter->data);
    icon = load_icon (GTK_STOCK_DIRECTORY);
    name = grl_source_get_name (source);
    GRL_DEBUG ("Loaded source: '%s'", name);
    gtk_list_store_append (GTK_LIST_STORE (view->browser_model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (view->browser_model),
			&iter,
			BROWSER_MODEL_SOURCE, source,
			BROWSER_MODEL_CONTENT, NULL,
			BROWSER_MODEL_TYPE, OBJECT_TYPE_SOURCE,
			BROWSER_MODEL_NAME, name,
			BROWSER_MODEL_ICON, icon,
			-1);
    if (icon) {
      g_object_unref (icon);
    }
  }
  g_list_free (sources);
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
  set_cur_resolve (NULL, NULL);
}

static void
reset_ui (void)
{
  cancel_current_operation ();
  clear_panes ();
  reset_browse_history ();
  show_browsable_sources ();
}

static gboolean
remove_notification (gpointer data)
{
  gtk_statusbar_remove (GTK_STATUSBAR (view->statusbar),
                        view->statusbar_context_id,
                        GPOINTER_TO_UINT (data));

  return FALSE;
}

static void
content_changed_cb (GrlSource *source,
                    GPtrArray *changed_medias,
                    GrlSourceChangeType change_type,
                    gboolean location_unknown,
                    gpointer data)
{
  GrlMedia *media;
  const gchar *media_id = NULL;
  const gchar *change_type_string = "";
  const gchar *location_string = "";
  gchar *message;
  guint id, i;

  switch (change_type) {
  case GRL_CONTENT_CHANGED:
    change_type_string = "changed";
    break;
  case GRL_CONTENT_ADDED:
    change_type_string = "been added";
    break;
  case GRL_CONTENT_REMOVED:
    change_type_string = "been removed";
    break;
  }

  if (location_unknown) {
    location_string = "(unknown place)";
  }

  for (i = 0; i < changed_medias->len; i++) {
    media = g_ptr_array_index (changed_medias, i);
    media_id = grl_media_get_id (media);
    if (GRL_IS_MEDIA_BOX (media)) {
      message =
        g_strdup_printf ("%s: container '%s' has %s%s",
                         grl_source_get_name (source),
                         media_id? media_id: "root",
                         change_type_string,
                         location_string);
    } else {
      message =
        g_strdup_printf ("%s: element '%s' has %s",
                         grl_source_get_name (GRL_SOURCE (source)),
                         media_id,
                         change_type_string);
    }

    id = gtk_statusbar_push (GTK_STATUSBAR (view->statusbar),
                             view->statusbar_context_id,
                             message);

    g_timeout_add_seconds (NOTIFICATION_TIMEOUT,
                           remove_notification,
                           GUINT_TO_POINTER (id));
    g_free (message);
  }
}

static void
source_added_cb (GrlRegistry *registry,
                 GrlSource *source,
                 gpointer user_data)
{
  GRL_DEBUG ("Detected new source available: '%s'",
             grl_source_get_name (source));

  GRL_DEBUG ("\tSource's name: %s", grl_source_get_name (source));
  GRL_DEBUG ("\tSource's description: %s", grl_source_get_description (source));

  /* If showing the plugin list, refresh it */
  if (!ui_state->cur_source && !ui_state->cur_container) {
    show_browsable_sources ();
  }

  /* Also refresh the search combos */
  search_combo_setup ();
  query_combo_setup ();

  /* Check for changes in source (if supported) */
  if (ui_state->changes_notification &&
      (grl_source_supported_operations (source) &
       GRL_OP_NOTIFY_CHANGE)) {
    if (grl_source_notify_change_start (GRL_SOURCE (source), NULL)) {
      g_signal_connect (GRL_SOURCE (source), "content-changed",
                        G_CALLBACK (content_changed_cb), NULL);
    }
  }
}

static void
source_removed_cb (GrlRegistry *registry,
                   GrlSource *source,
                   gpointer user_data)
{
  GRL_DEBUG ("Source '%s' is gone",
             grl_source_get_name (source));

  if (!ui_state->cur_source && !ui_state->cur_container) {
    /* If showing the plugin list, refresh it */
    show_browsable_sources ();
  } else if ((gpointer)ui_state->cur_source == user_data ) {
    /* If we were browsing that source, cancel operation and  go back to
       plugin list view */
    GRL_DEBUG ("Currently browsing the removed source: resetting UI.");
    reset_ui ();
  }

  /* Also refresh the search combo */
  search_combo_setup ();
  query_combo_setup ();
}

static void
load_plugins (void)
{
  GrlRegistry *registry;
  registry = grl_registry_get_default ();
  g_signal_connect (registry, "source-added",
		    G_CALLBACK (source_added_cb), NULL);
  g_signal_connect (registry, "source-removed",
		    G_CALLBACK (source_removed_cb), NULL);
  if (!grl_registry_load_all_plugins (registry, NULL)) {
    g_error ("Failed to load plugins.");
  }
}

static void
shutdown_plugins (void)
{
  GList *plugins;
  GList *plugin_iter;
  GrlRegistry *registry;

  /* Cancel previous operation, if any */
  cancel_current_operation ();

  /* Let's make sure we don't have references to stuff
     we are about to shut down */
  clear_ui ();

  registry = grl_registry_get_default ();

  /* Disable "source-removed" handler */
  g_signal_handlers_block_by_func (G_OBJECT (registry), source_removed_cb,
				   NULL);

  /* Shut down the plugins now */
  plugins = grl_registry_get_plugins (registry, TRUE);
  for (plugin_iter = plugins;
       plugin_iter;
       plugin_iter = g_list_next (plugin_iter)) {
    grl_registry_unload_plugin (registry,
                                grl_plugin_get_id (GRL_PLUGIN (plugin_iter->data)),
                                NULL);
  }
  g_list_free (plugins);

  /* Re-enable "source-removed" handler */
  g_signal_handlers_unblock_by_func (G_OBJECT (registry), source_removed_cb,
				     NULL);

  /* Reload UI */
  reset_ui ();
  search_combo_setup ();
  query_combo_setup ();
}

static void
load_all_plugins ()
{
  GrlRegistry *registry;

  registry = grl_registry_get_default ();

  grl_registry_load_all_plugins (registry, NULL);
}

static void
configure_plugins ()
{
  load_file_config();
  set_flickr_config ();
  set_youtube_config ();
  set_vimeo_config ();
  set_tmdb_config ();
  set_local_config ();
}

int
main (int argc, gchar *argv[])
{
  gtk_init (&argc, &argv);
  grl_init (&argc, &argv);
  GRL_LOG_DOMAIN_INIT (test_ui_log_domain, "test-ui");
  launchers_setup ();
  options_setup ();
  ui_setup ();
  configure_plugins ();
  load_plugins ();
  gtk_main ();
  return 0;
}
