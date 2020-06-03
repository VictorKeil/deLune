#include <stdbool.h>
#include <gtk/gtk.h>

#include "delune-window.h"
#include "delune-application.h"
#include "delune-signal-chooser.h"
#include "delune-signal-component.h"


struct _DeluneWindow {
  GtkApplicationWindow base_window;

  DeluneApplication *app;

  GtkWidget *stack;
  GtkWidget *signal_grid;
  GtkWidget *alloc_signals_view;
  GtkWidget *alloc_signals_refresh;
  GtkWidget *button_signal_new;

  GtkTreeView *output_signals_view;
  GtkTreeView *output_candidates_view;
  GtkWidget *master_amplitude;
};

G_DEFINE_TYPE(DeluneWindow, delune_window, GTK_TYPE_APPLICATION_WINDOW);

void on_signal_add_to_output(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer win_ptr) {
  DeluneWindow *win;
  GtkTreeModel *filtered_model;
  GtkTreeIter iter;
  GtkTreePath *path_to_signal;
  Signal *signal;

  win = DELUNE_WINDOW(win_ptr);

  filtered_model = gtk_tree_view_get_model(view);
  path_to_signal = gtk_tree_model_filter_convert_path_to_child_path(GTK_TREE_MODEL_FILTER(filtered_model), path);

  delune_signal_add_to_master_out(win->app, path_to_signal);
}

void on_alloc_signals_refresh(DeluneWindow *win) {
  char row[512];
  Signal **allocated_signals;
  int n_allocated_signals;
  GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->alloc_signals_view));
  GtkTextIter start, end;

  gtk_text_buffer_get_bounds(buf, &start, &end);
  gtk_text_buffer_delete(buf, &start, &end);

  allocated_signals = win->app->sound->signal_table->signals;
  n_allocated_signals = win->app->sound->signal_table->signal_count;

  const char *row_template = "signal #%d: %s\n\
\tptr: %p\n\
\tref count: %d\n\n";

  int ref_count;
  for (int i = 0; i < n_allocated_signals; i++) {
    ref_count = SIGNAL(allocated_signals[i])->reference_count;
    sprintf(row, row_template, i, signal_get_name(allocated_signals[i]), allocated_signals[i], ref_count);
    g_print("%s", row);
    gtk_text_buffer_insert(buf, &start, row, -1);
  }
}

void on_signal_pressed_in_output(GtkTreeView *view, GtkTreePath *path, void *col, gpointer win_ptr) {
  DeluneWindow *win;
  GtkTreeModel *model;
  GtkTreeIter iter;
  Signal *signal;

  win = DELUNE_WINDOW(win_ptr);

  model = gtk_tree_view_get_model(view);
  gtk_tree_model_get_iter(model, &iter, path);

  delune_signal_remove_from_master_out(win->app, &iter);
}

static gboolean output_candidate_signal_visible_func(GtkTreeModel *tracked_signals, GtkTreeIter *iter, gpointer output_signals_ptr) {
  GtkTreePath *path_in_out, *path_of_tracked;
  GtkTreeIter out_iter;
  GtkTreeModel *output_signals;
  gboolean is_valid;

  path_of_tracked = gtk_tree_model_get_path(tracked_signals, iter);

  output_signals = GTK_TREE_MODEL(output_signals_ptr);
  is_valid = gtk_tree_model_get_iter_first(output_signals, &out_iter);
  for (; is_valid; is_valid = gtk_tree_model_iter_next(output_signals, &out_iter)) {
    gtk_tree_model_get(output_signals, &out_iter, 0, &path_in_out, -1);
    if (gtk_tree_path_compare(path_in_out, path_of_tracked) == 0)
      return FALSE;
  }

  return TRUE;
}

void on_master_out_changed(gpointer win_ptr) {
  DeluneWindow *win = DELUNE_WINDOW(win_ptr);
  GtkTreeModel *model;

  model = gtk_tree_view_get_model(win->output_candidates_view);
  gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(model));
}

static void create_signal_component(DeluneWindow *win, Signal *signal) {
  DeluneSignalComponent *component;

  component = delune_signal_component_new(DELUNE_WINDOW(win)->app, signal);
  gtk_container_add(GTK_CONTAINER(DELUNE_WINDOW(win)->signal_grid), GTK_WIDGET(component));
  g_object_set(gtk_widget_get_parent(GTK_WIDGET(component)), "can-focus", FALSE, NULL);
}

void on_signal_new(GtkWidget *button, gpointer win) {
  Signal *signal = delune_signal_new(DELUNE_WINDOW(win)->app);
  create_signal_component(DELUNE_WINDOW(win), signal);
}

void update_signal_grid(DeluneWindow *win) {
  
}

static void delune_window_init(DeluneWindow *win) {
  gtk_widget_init_template(GTK_WIDGET(win));

  gtk_text_view_set_buffer(GTK_TEXT_VIEW(win->alloc_signals_view), gtk_text_buffer_new(NULL));

  g_signal_connect(win->button_signal_new, "clicked", G_CALLBACK(on_signal_new), win);
  g_signal_connect_swapped(win->alloc_signals_refresh, "clicked", G_CALLBACK(on_alloc_signals_refresh), win);
  g_signal_connect(win->output_candidates_view, "row-activated", G_CALLBACK(on_signal_add_to_output), win);
  g_signal_connect(win->output_signals_view, "row-activated", G_CALLBACK(on_signal_pressed_in_output), win);
}

static void delune_window_class_init(DeluneWindowClass *class) {
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
					      "/home/victor/workspace/c/delune/ui/delune-window.ui");

  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, stack);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, signal_grid);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, alloc_signals_view);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, alloc_signals_refresh);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, output_signals_view);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, output_candidates_view);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, master_amplitude);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, button_signal_new);
}

static gboolean create_signal_component_func(GtkTreeModel *tracked_signals_model, GtkTreePath *path, GtkTreeIter *iter, gpointer win) {
  Signal *signal;

  gtk_tree_model_get(tracked_signals_model, iter, 0, &signal, -1);
  create_signal_component(DELUNE_WINDOW(win), signal);

  return FALSE;
}

static void delune_map_master_out_view_func(GtkTreeViewColumn *col,
                                            GtkCellRenderer *r,
                                            GtkTreeModel *model,
                                            GtkTreeIter *iter, gpointer app_ptr) {
  DeluneApplication *app;
  GtkTreeModel *tracked_signals;
  GtkTreePath *path;
  GtkTreeIter tracked_iter;
  Signal *signal;

  app = DELUNE_APPLICATION(app_ptr);
  tracked_signals = GTK_TREE_MODEL(app->tracked_signals);

  gtk_tree_model_get(model, iter, 0, &path, -1);

  gtk_tree_model_get_iter(tracked_signals, &tracked_iter, path);
  gtk_tree_model_get(tracked_signals, &tracked_iter, 0, &signal, -1);

  g_object_set(r, "text", signal_get_name(signal), NULL);
}

static void construct_ui(DeluneWindow *win) {
  GtkTreeViewColumn *col;
  GtkCellRenderer *text_renderer;
  GtkTreeModel *tracked_signals_model;
  GtkTreeModel *filtered_out_signals;

  tracked_signals_model = GTK_TREE_MODEL(win->app->tracked_signals);
  gtk_tree_model_foreach(tracked_signals_model, create_signal_component_func,
                         win);

  // Setup master output signals view
  text_renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_set_model(GTK_TREE_VIEW(win->output_signals_view), GTK_TREE_MODEL(win->app->master_out_signals));
  gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(win->output_signals_view), TRUE);
  gtk_tree_view_set_headers_visible(win->output_signals_view, FALSE);

  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_cell_data_func(col, text_renderer, delune_map_master_out_view_func, win->app, NULL);
  gtk_tree_view_column_pack_start(col, text_renderer, TRUE);
  gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

  gtk_tree_view_append_column(win->output_signals_view, col);

  // Setup popup which lets user add signals to output
  filtered_out_signals = gtk_tree_model_filter_new(tracked_signals_model, NULL);
  gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filtered_out_signals), output_candidate_signal_visible_func, win->app->master_out_signals, NULL);
  gtk_tree_view_set_model(win->output_candidates_view, filtered_out_signals);

  gtk_tree_view_insert_column_with_data_func(
      win->output_candidates_view, 0, "Signals", text_renderer,
      (GtkTreeCellDataFunc)delune_map_signal_name_func, NULL, NULL);
  gtk_tree_view_set_activate_on_single_click(win->output_candidates_view, TRUE);

  g_signal_connect_swapped(win->app->master_out_signals, "row-changed", G_CALLBACK(on_master_out_changed), win);
  g_signal_connect_swapped(win->app->master_out_signals, "row-deleted", G_CALLBACK(on_master_out_changed), win);
}

DeluneWindow *delune_window_new(DeluneApplication *app) {
  DeluneWindow *win = DELUNE_WINDOW(g_object_new(DELUNE_WINDOW_TYPE, "application", app, NULL));

  win->app = app;
  gtk_range_set_adjustment(GTK_RANGE(win->master_amplitude), app->master_amplitude_adjustment);

  construct_ui(win);

  return win;
}
