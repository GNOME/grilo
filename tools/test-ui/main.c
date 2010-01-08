#include <media-store.h>

#include <gtk/gtk.h>

#define WINDOW_TITLE "Media Store Test UI"

#define BROWSER_MIN_WIDTH  320
#define BROWSER_MIN_HEIGHT 200

enum {
  OBJECT_TYPE_SOURCE = 0,
  OBJECT_TYPE_CONTAINER,
  OBJECT_TYPE_MEDIA
};

enum {
  BROWSER_MODEL_SOURCE = 0,
  BROWSER_MODEL_ID,
  BROWSER_MODEL_TYPE,
  BROWSER_MODEL_NAME,
  BROWSER_MODEL_ICON,
};

typedef struct {
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *browser;
  GtkTreeModel *browser_model;
} UiView;

static UiView *view;

static GtkTreeModel *
create_browser_model (void)
{
  return GTK_TREE_MODEL (gtk_list_store_new (5,
					     G_TYPE_OBJECT,     /* Source */
					     G_TYPE_STRING,     /* ID */
					     G_TYPE_INT,        /* Type */
					     G_TYPE_STRING,     /* Name */
					     GDK_TYPE_PIXBUF)); /* Icon */
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

static void
browse_cb (MsMediaSource *source,
	   guint browse_id,
	   MsContent *media,
	   guint remaining,
	   gpointer user_data,
	   const GError *error)
{
  static gboolean first = TRUE;
  gint type;
  const gchar *id, *name;
  GtkTreeIter iter;
  GdkPixbuf *icon;

  if (error) {
    g_critical ("Error: %s", error->message);
    return;
  }

  if (first) {
    g_object_unref (view->browser_model);
    view->browser_model = create_browser_model ();
    gtk_tree_view_set_model (GTK_TREE_VIEW (view->browser),
			     view->browser_model);
    first = FALSE;
  }
  
  if (ms_content_is_container (media)) {
    type = OBJECT_TYPE_CONTAINER;
    icon = load_icon (GTK_STOCK_DIRECTORY);
  } else {
    type = OBJECT_TYPE_MEDIA;
    icon = load_icon (GTK_STOCK_FILE);
  }

  id = ms_content_media_get_id (media);
  name = ms_content_media_get_title (media);

  gtk_list_store_append (GTK_LIST_STORE (view->browser_model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (view->browser_model),
		      &iter,
		      BROWSER_MODEL_SOURCE, source,
		      BROWSER_MODEL_ID, id,
		      BROWSER_MODEL_TYPE, type,
		      BROWSER_MODEL_NAME, name,
		      BROWSER_MODEL_ICON, icon,
		      -1);

  if (remaining == 0) {
    first = TRUE;
  }
}

static void
browse (MsMediaSource *source, const gchar *container_id)
{
  ms_media_source_browse (source,
			  container_id,
			  browse_keys (),
			  0, 50,
			  0,
			  browse_cb,
			  NULL);
}

static void
browser_activated_cb (GtkTreeView *tree_view,
		      GtkTreePath *path,
		      GtkTreeViewColumn *column,
		      gpointer user_data)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *id;
  gint type;
  MsMediaSource *source;
  gchar *container_id;

  model = gtk_tree_view_get_model (tree_view);    
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter,
		      BROWSER_MODEL_SOURCE, &source,
		      BROWSER_MODEL_ID, &id,
		      BROWSER_MODEL_TYPE, &type,
		      -1);

  if (type == OBJECT_TYPE_MEDIA) {
    return;
  }

  if (type == OBJECT_TYPE_SOURCE) {
    container_id = NULL;
  } else {
    container_id = id;
  }

  browse (source, container_id);
}

static void
ui_setup (void)
{
  view = g_new0 (UiView, 1);

  /* Main window */
  view->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (view->window), WINDOW_TITLE);

  /* Main box */
  view->box = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (view->window), view->box);

  /* Contents tree view */
  GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  view->browser = gtk_tree_view_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view->browser), TRUE);
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
  gtk_container_add (GTK_CONTAINER (view->box), scroll);
  gtk_widget_set_size_request (view->browser,
			       BROWSER_MIN_WIDTH,
			       BROWSER_MIN_HEIGHT);
  g_signal_connect (view->browser, "row-activated",
		    G_CALLBACK (browser_activated_cb), NULL);



  /* Contents model */
  view->browser_model = create_browser_model ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (view->browser),
			   view->browser_model);
 
  gtk_widget_show_all (view->window);
}

static void
setup_plugins (void)
{
  MsPluginRegistry *registry;
  MsMediaPlugin **sources;
  guint i;
  GtkTreeIter iter;
  MsSupportedOps ops;

  registry = ms_plugin_registry_get_instance ();
  if (!ms_plugin_registry_load_all (registry)) {
    g_error ("Failed to load plugins.");
  }

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
			  BROWSER_MODEL_ID, id,
			  BROWSER_MODEL_TYPE, OBJECT_TYPE_SOURCE,
			  BROWSER_MODEL_NAME, name,
			  BROWSER_MODEL_ICON, icon,
			  -1);
    }
    i++;
  }
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
