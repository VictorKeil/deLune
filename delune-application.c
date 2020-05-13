#include <gtk/gtk.h>

#include "delune-application.h"
#include "delune-window.h"
#include "sound.h"
#include "signal.h"
#include "utils.h"

G_DEFINE_TYPE(DeluneApplication, delune_application, GTK_TYPE_APPLICATION);

void delune_map_signal_name_func(GtkCellLayout *layout, GtkCellRenderer *r,
                                 GtkTreeModel *tracked_signals,
                                 GtkTreeIter *iter, gpointer data) {
  Signal *signal;
  char *name;
  GtkCellRendererText *text_renderer;

  text_renderer = GTK_CELL_RENDERER_TEXT(r);

  gtk_tree_model_get(tracked_signals, iter, 0, &signal, -1);
  name = signal_get_name(signal);
  
  g_object_set(text_renderer, "text", name, NULL);
}

void on_master_amplitude_changed(GtkAdjustment *adjustment, gpointer app) {
  float val = gtk_adjustment_get_value(adjustment);
  signal_mixer_set_master_amplitude(DELUNE_APPLICATION(app)->sound->master_out, val);
  sound_outstream_clear_buffer(DELUNE_APPLICATION(app)->sound);
}

gboolean delune_tree_model_find_signal(GtkTreeModel *model, GtkTreeIter *iter, Signal *signal) {
  Signal *signal_in_model;

  gboolean is_valid = gtk_tree_model_get_iter_first(model, iter);
  for (; is_valid; is_valid = gtk_tree_model_iter_next(model, iter)) {
    gtk_tree_model_get(model, iter, 0, &signal_in_model, -1);
    if (signal == signal_in_model)
      return TRUE;
  }

  return FALSE;
}

void delune_signal_add(DeluneApplication *app, Signal *signal) {
  GtkTreeIter iter;

  gtk_list_store_append(app->tracked_signals, &iter);
  gtk_list_store_set(app->tracked_signals, &iter,
		     0, signal,
		     1, signal_get_name(signal), -1);
}

void delune_signal_replace(DeluneApplication *app, Signal *old_signal, Signal *new_signal) {
  GtkTreeModel *tracked_signals;
  GtkTreeIter iter;
  Signal *signal_in_model;
  gboolean is_contained;

  tracked_signals = GTK_TREE_MODEL(app->tracked_signals);

  is_contained = delune_tree_model_find_signal(tracked_signals, &iter, old_signal);
  if (is_contained) {
    gtk_list_store_set(app->tracked_signals, &iter, 0, new_signal, -1);
  }

  signal_mixer_remove_input_signal(app->sound->master_out, old_signal);
  signal_mixer_add_input_signal(app->sound->master_out, new_signal);
}

Signal *delune_signal_new(DeluneApplication *app) {
  GtkTreeIter iter;
  Signal *signal;

  signal = SIGNAL(signal_new_sine_wave(app->sound->signal_table, 440, 1));
  delune_signal_add(app, signal);

  return signal;
}

gboolean foreach_remove_func(GtkTreeModel *model, GtkTreePath *path,
                             GtkTreeIter *iter, gpointer signal) {
  Signal *signal_in_node;

  gtk_tree_model_get(model, iter, 0, &signal_in_node, -1);
  if (signal_in_node == signal) {
    gtk_list_store_remove(GTK_LIST_STORE(model), iter);
    return TRUE;
  }

  return FALSE;
}

void delune_signal_destroy(DeluneApplication *app, Signal *signal) {
  GtkTreeModel *tracked_signals, *master_out_signals;

  tracked_signals = GTK_TREE_MODEL(app->tracked_signals);
  master_out_signals = GTK_TREE_MODEL(app->master_out_signals);
  gtk_tree_model_foreach(tracked_signals, foreach_remove_func, signal);
  gtk_tree_model_foreach(master_out_signals, foreach_remove_func, signal);

  signal_destroy(signal);
}

void delune_signal_add_to_master_out(DeluneApplication *app, Signal *signal) {
  GtkTreeIter iter;
  GtkTreeModel *tracked_signals;
  GtkTreePath *path;

  signal_mixer_add_input_signal(app->sound->master_out, signal);

  tracked_signals = GTK_TREE_MODEL(app->tracked_signals);
  delune_tree_model_find_signal(tracked_signals, &iter, signal);
  path = gtk_tree_model_get_path(GTK_TREE_MODEL(app->tracked_signals), &iter);
  gtk_list_store_append(app->master_out_signals, &iter);
  gtk_list_store_set(app->master_out_signals, &iter, 0, path, -1);
}

void delune_signal_remove_from_master_out_depr(DeluneApplication *app,
                                               Signal *signal) {
  GtkTreeIter out_iter, tracked_iter;
  GtkTreePath *path_in_tracked;
  GtkTreeModel *out_signals, *tracked_signals;
  Signal *signal_in_tracked;

  signal_mixer_remove_input_signal(app->sound->master_out, signal);

  out_signals = GTK_TREE_MODEL(app->master_out_signals);
  tracked_signals = GTK_TREE_MODEL(app->tracked_signals);

  gboolean is_valid = gtk_tree_model_get_iter_first(out_signals, &out_iter);
  for (; is_valid;
       is_valid = gtk_tree_model_iter_next(out_signals, &out_iter)) {
    gtk_tree_model_get(out_signals, &out_iter, 0, &path_in_tracked, -1);
    gtk_tree_model_get_iter(tracked_signals, &tracked_iter, path_in_tracked);
    gtk_tree_model_get(tracked_signals, &tracked_iter, 0, &signal_in_tracked,
                       -1);

    if (signal == signal_in_tracked) {
      gtk_list_store_remove(app->master_out_signals, &out_iter);
      gtk_tree_path_free(path_in_tracked);
      printf("removed: %s\n", signal_get_name(signal));
      break;
    }
  }
}

void delune_signal_remove_from_master_out(DeluneApplication *app,
                                          GtkTreeIter *iter) {
  GtkTreeIter tracked_iter;
  GtkTreePath *path_in_tracked;
  Signal *signal;

  gtk_tree_model_get(GTK_TREE_MODEL(app->master_out_signals), iter, 0,
                     &path_in_tracked, -1);
  gtk_tree_model_get_iter(GTK_TREE_MODEL(app->tracked_signals), &tracked_iter,
                          path_in_tracked);
  gtk_tree_model_get(GTK_TREE_MODEL(app->tracked_signals), &tracked_iter, 0,
                     &signal, -1);
  gtk_list_store_remove(app->master_out_signals, iter);
  gtk_tree_path_free(path_in_tracked);

  signal_mixer_remove_input_signal(app->sound->master_out, signal);
}

static void delune_application_init(DeluneApplication *app) {
  app->master_out_signals = gtk_list_store_new(1, GTK_TYPE_TREE_PATH);
  app->tracked_signals = gtk_list_store_new(2, G_TYPE_POINTER, G_TYPE_STRING);
}

static void delune_application_activate(GApplication *app) {
  GtkAdjustment *master_amplitude_adjustment;
  DeluneWindow *win;

  float master_amplitude =
      DELUNE_APPLICATION(app)->sound->master_out->master_amplitude->value;
  master_amplitude_adjustment =
      gtk_adjustment_new(master_amplitude, 0, 1, .02, 0, 0);
  g_signal_connect(master_amplitude_adjustment, "value-changed",
                   G_CALLBACK(on_master_amplitude_changed), app);
  DELUNE_APPLICATION(app)->master_amplitude_adjustment =
      master_amplitude_adjustment;

  win = delune_window_new(DELUNE_APPLICATION(app));
  gtk_window_present(GTK_WINDOW(win));
}

static void delune_application_class_init(DeluneApplicationClass *class) {
  G_APPLICATION_CLASS(class)->activate = delune_application_activate;
}

DeluneApplication *delune_application_new() {
  DeluneApplication *app;
  app = g_object_new(DELUNE_APPLICATION_TYPE, "application-id",
                     "org.keil.delune", NULL);

  app->sound = sound_create();
  g_signal_connect_swapped(app->master_out_signals, "row-changed",
                           G_CALLBACK(sound_outstream_clear_buffer),
                           app->sound);
  g_signal_connect_swapped(app->master_out_signals, "row-deleted",
                           G_CALLBACK(sound_outstream_clear_buffer),
                           app->sound);

  WaveSignal *sine, *saw;
  sine = signal_new_sine_wave(app->sound->signal_table, 440, 1);
  signal_set_name(sine, "Sine 1");
  delune_signal_add(app, sine);

  saw = signal_new_saw_wave(app->sound->signal_table, 220, 1);
  signal_set_name(saw, "Saw 1");
  delune_signal_add(app, saw);

  return app;
}
