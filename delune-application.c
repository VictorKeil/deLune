#include <gtk/gtk.h>

#include "delune-application.h"
#include "delune-window.h"
#include "sound.h"
#include "signal.h"
#include "delune-mixer.h"
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

void delune_signal_replace(DeluneApplication *app, Signal *adapter, Signal *new_signal) {
  GtkTreeModel *tracked_signals;
  GtkTreeIter iter;
  gboolean is_contained;

  tracked_signals = GTK_TREE_MODEL(app->tracked_signals);

  is_contained = delune_tree_model_find_signal(tracked_signals, &iter, adapter);
  if (is_contained) {
    signal_adapter_set_input(adapter, new_signal);
    sound_outstream_clear_buffer(app->sound);
  }
}

Signal *delune_signal_new(DeluneApplication *app) {
  GtkTreeIter iter;
  Signal *adapter, *signal;

  adapter = signal_new_adapter(app->sound->signal_table);
  signal = signal_new_const_signal(app->sound->signal_table, .5);
  signal_adapter_set_input(adapter, signal);

  delune_signal_add(app, adapter);

  return adapter;
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
  gtk_tree_model_foreach(tracked_signals, foreach_remove_func, signal);

  //TODO: remove in master out aswell

  signal_destroy(signal);
}

void delune_signal_add_to_master_out(DeluneApplication *app, GtkTreePath *signal_path) {
  DeluneMixer *master_out;

  master_out = DELUNE_MIXER(app->sound->master_out);
  delune_mixer_add_input_signal(master_out, signal_path);
}

void delune_signal_remove_from_master_out(DeluneApplication *app,
                                          GtkTreeIter *iter) {
  DeluneMixer *master_out;
  GtkTreePath *path_to_signal;

  master_out = DELUNE_MIXER(app->sound->master_out);

  delune_mixer_remove_input_signal(master_out, iter);
}

static void delune_application_init(DeluneApplication *app) {
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
  DeluneMixer *mixer;
  app = g_object_new(DELUNE_APPLICATION_TYPE, "application-id",
                     "org.keil.delune", NULL);

  app->sound = sound_create();
  mixer = delune_mixer_new(app->sound->signal_table);
  mixer->tracked_signals = app->tracked_signals;
  signal_mixer_set_master_amplitude(MIXER(mixer), .1);
  app->sound->master_out = MIXER(mixer);
  app->master_out_signals = delune_mixer_get_input_signals(mixer);

  Signal *sig;
  sig = delune_signal_new(app);
  signal_set_name(sig, "sine");

  sig = delune_signal_new(app);
  signal_set_name(sig, "sine-amp");

  sig = delune_signal_new(app);
  signal_set_name(sig, "sine-freq");

  sig = delune_signal_new(app);
  signal_set_name(sig, "sine-freq-freq");

  sig = delune_signal_new(app);
  signal_set_name(sig, "sine-freq-amp");

  g_signal_connect_swapped(app->master_out_signals, "row-changed", G_CALLBACK(sound_outstream_clear_buffer), app->sound);
  g_signal_connect_swapped(app->master_out_signals, "row-deleted", G_CALLBACK(sound_outstream_clear_buffer), app->sound);

  return app;
}
