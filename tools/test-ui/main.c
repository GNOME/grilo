#include <media-store.h>

#include <gtk/gtk.h>

#define BROWSE_FLAGS (MS_RESOLVE_FAST_ONLY | MS_RESOLVE_IDLE_RELAY)
#define METADATA_FLAGS (MS_RESOLVE_FULL | MS_RESOLVE_IDLE_RELAY)

#define WINDOW_TITLE "Media Store Test UI"

#define BROWSER_MIN_WIDTH   320
#define BROWSER_MIN_HEIGHT  400

#define METADATA_MIN_WIDTH  320
#define METADATA_MIN_HEIGHT 400

#define BROWSE_CHUNK_SIZE   50
#define BROWSE_MAX_COUNT    (4 * BROWSE_CHUNK_SIZE)

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

typedef struct {
  GtkWidget *window;
  GtkWidget *lpane;
  GtkWidget *rpane;
  GtkWidget *back_btn;
  GtkWidget *browser;
  GtkTreeModel *browser_model;
  GtkWidget *metadata;
  GtkTreeModel *metadata_model;
  GList *source_stack;
  GList *id_stack;
  MsMediaSource *cur_source;
  gchar *cur_id;
} UiView;

typedef struct {
  guint offset;
} BrowseOpState;

static UiView *view;

static void show_plugins (MsPluginRegistry *registry);

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

static GList *
browse_keys (void)
{
  return ms_metadata_key_list_new (MS_METADATA_KEY_ID,
				   MS_METADATA_KEY_TITLE,
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
				   MS_METADATA_KEY_CHILDCOUNT,
				   NULL);
}


static void 
metadata_cb (MsMediaSource *source,
	     MsContentMedia *media,
	     gpointer user_data,
	     const GError *error)
{
  MsKeyID *keys;
  gint size;
  GtkTreeIter iter;
  MsPluginRegistry *registry;
  gint i;

  if (view->metadata_model) {
    g_object_unref (view->metadata_model);
  }
  view->metadata_model = create_metadata_model ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (view->metadata),
			   view->metadata_model);

  if (error) {
    g_critical ("Error: %s", error->message);
    return;
  }

  registry = ms_plugin_registry_get_instance ();
  keys = ms_content_get_keys (MS_CONTENT (media), &size);
  for (i=0; i<size; i++) {
    const MsMetadataKey *key =
      ms_plugin_registry_lookup_metadata_key (registry, keys[i]);
    gchar *value =
      g_strdup_value_contents (ms_content_get (MS_CONTENT (media),
                                               keys[i]));
    gtk_list_store_append (GTK_LIST_STORE (view->metadata_model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (view->metadata_model),
			&iter,
			METADATA_MODEL_NAME, MS_METADATA_KEY_GET_NAME (key),
			METADATA_MODEL_VALUE, value,
			-1);
  }
}

static void
browse_cb (MsMediaSource *source,
	   guint browse_id,
	   MsContentMedia *media,
	   guint remaining,
	   gpointer user_data,
	   const GError *error)
{
  static gboolean first = TRUE;
  static guint count = 0;
  gint type;
  const gchar *name;
  GtkTreeIter iter;
  GdkPixbuf *icon;
  BrowseOpState *state = (BrowseOpState *) user_data;
  
  if (first) {
    g_object_unref (view->browser_model);
    view->browser_model = create_browser_model ();
    gtk_tree_view_set_model (GTK_TREE_VIEW (view->browser),
			     view->browser_model);

    g_object_unref (view->metadata_model);
    view->metadata_model = create_metadata_model ();
    gtk_tree_view_set_model (GTK_TREE_VIEW (view->metadata),
			     view->metadata_model);

    first = FALSE;
  }
  
  if (error) {
    g_critical ("Error: %s", error->message);
    goto browse_finished;
  }

  if (!media) {
    goto browse_finished;
  }

  count++;

  if (IS_MS_CONTENT_BOX (media)) {
    type = OBJECT_TYPE_CONTAINER;
    icon = load_icon (GTK_STOCK_DIRECTORY);
  } else {
    type = OBJECT_TYPE_MEDIA;
    if (IS_MS_CONTENT_VIDEO (media)) {
      icon = load_icon ("gnome-mime-video");
    } else {
      icon = load_icon ("gnome-mime-audio");
    }
  }

  name = ms_content_media_get_title (media);

  gtk_list_store_append (GTK_LIST_STORE (view->browser_model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (view->browser_model),
		      &iter,
		      BROWSER_MODEL_SOURCE, source,
		      BROWSER_MODEL_CONTENT, media,
		      BROWSER_MODEL_TYPE, type,
		      BROWSER_MODEL_NAME, name,
		      BROWSER_MODEL_ICON, icon,
		      -1);

  if (remaining == 0) {
    /* Done with this chunk, check if there is more to browse */
    state->offset += count;
    if (count >= BROWSE_CHUNK_SIZE &&
	state->offset < BROWSE_MAX_COUNT) {
      count = 0;
      ms_media_source_browse (source,
			      view->cur_id,
			      browse_keys (),
			      state->offset, BROWSE_CHUNK_SIZE,
			      BROWSE_FLAGS,
			      browse_cb,
			      state);
    } else {
      goto browse_finished;
    }
  }

  return;

 browse_finished:
  g_free (state);
  first = TRUE;
  count = 0;
}

static void
browse (MsMediaSource *source, const gchar *container_id)
{
  if (source) {
    BrowseOpState *state = g_new0 (BrowseOpState, 1);
    state->offset = 0;
    ms_media_source_browse (source,
			    container_id,
			    browse_keys (),
			    0, BROWSE_CHUNK_SIZE,
			    BROWSE_FLAGS,
			    browse_cb,
			    state);
  } else {
    show_plugins (ms_plugin_registry_get_instance ());
  }

  view->cur_source = source;
  view->cur_id = (gchar *) container_id;
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
  const gchar *container_id;

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
    container_id = NULL;
  } else if (content) {
    container_id = ms_content_media_get_id (content);
  } else {
    container_id = NULL;
  }

  view->source_stack = g_list_append (view->source_stack, view->cur_source);
  view->id_stack = g_list_append (view->id_stack, view->cur_id);

  browse (source, container_id);
}

static void
metadata (MsMediaSource *source, const gchar *id)
{
  if (source) {
    MsContentMedia *media = ms_content_media_new ();
    ms_content_media_set_id (media, id);
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
  const gchar *id;

  gtk_tree_view_get_cursor (tree_view, &path, NULL);
  gtk_tree_model_get_iter (view->browser_model, &iter, path);
  gtk_tree_model_get (view->browser_model,
		      &iter,
		      BROWSER_MODEL_SOURCE, &source,
		      BROWSER_MODEL_CONTENT, &content,
		      -1);

  id = content ? ms_content_media_get_id (content) : NULL;

  metadata (source, id);
}

static void
back_btn_clicked_cb (GtkButton *btn, gpointer user_data)
{
  GList *tmp;
  MsMediaSource *prev_source = NULL;
  gchar *prev_id = NULL;

  tmp = g_list_last (view->source_stack);
  if (tmp) {
    prev_source = MS_MEDIA_SOURCE (tmp->data);
    view->source_stack = g_list_delete_link (view->source_stack, tmp);
  } 
  tmp = g_list_last (view->id_stack);
  if (tmp) {
    prev_id = (gchar *) tmp->data;
    view->id_stack = g_list_delete_link (view->id_stack, tmp);
  } 

  browse (prev_source, prev_id);
}

static void
ui_setup (void)
{
  view = g_new0 (UiView, 1);

  /* Main window */
  view->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (view->window), WINDOW_TITLE);

  /* Main layout */
  GtkWidget *box = gtk_hbox_new (FALSE, 0);
  view->lpane = gtk_vbox_new (FALSE, 0);
  view->rpane = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (view->window), box);
  gtk_container_add (GTK_CONTAINER (box), view->lpane);
  gtk_container_add (GTK_CONTAINER (box), view->rpane);

  /* Go back button */
  view->back_btn = gtk_button_new_with_label ("Go back");
  gtk_container_add (GTK_CONTAINER (view->lpane), view->back_btn);
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
 
  gtk_widget_show_all (view->window);
}

static void
show_plugins (MsPluginRegistry *registry)
{
  MsMediaPlugin **sources;
  guint i;
  GtkTreeIter iter;
  MsSupportedOps ops;

  if (view->browser_model) {
    g_object_unref (view->browser_model);
  }
  view->browser_model = create_browser_model ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (view->browser),
			   view->browser_model);

  i = 0;
  sources = ms_plugin_registry_get_sources (registry);
  while (sources[i]) {
    ops = ms_metadata_source_supported_operations (MS_METADATA_SOURCE (sources[i]));
    if (ops & MS_OP_BROWSE) {
      gchar *id, *name;
      GdkPixbuf *icon;
      icon = load_icon (GTK_STOCK_DIRECTORY);
      id = ms_media_plugin_get_id (sources[i]);
      name = ms_media_plugin_get_name (sources[i]);
      g_debug ("Loaded source: '%s'\n", name);
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
setup_plugins (void)
{
  MsPluginRegistry *registry;
  registry = ms_plugin_registry_get_instance ();
  if (!ms_plugin_registry_load_all (registry)) {
    g_error ("Failed to load plugins.");
  }

  show_plugins (registry);
}

int main (int argc, gchar *argv[])
{ 
  gtk_init (&argc, &argv);
  ms_log_init ("*:*");
  ui_setup ();
  setup_plugins ();
  gtk_main ();
  return 0;
}
