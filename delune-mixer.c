#include "delune-mixer.h"

void delune_mixer_init(DeluneMixer *mixer) {
  mixer->input_signals = gtk_list_store_new(1, GTK_TYPE_TREE_PATH);
}

DeluneMixer *delune_mixer_new(SignalTable *table) {
  DeluneMixer *mixer;

  mixer = calloc(1, sizeof(DeluneMixer));
  signal_init(table, SIGNAL(mixer));
  signal_mixer_init(MIXER(mixer));
  delune_mixer_init(mixer);

  return mixer;
}

void delune_mixer_add_input_signal(DeluneMixer *mixer, GtkTreePath *signal_path) {
  GtkTreeIter iter;
  GtkTreeModel *model;
  Signal *signal;

  if (!mixer->tracked_signals) {
    fprintf(stderr, "DeluneMixer error: tracked_signals is null\n");
    exit(1);
  }

  model = GTK_TREE_MODEL(mixer->tracked_signals);
  gtk_tree_model_get_iter(model, &iter, signal_path);
  gtk_tree_model_get(model, &iter, 0, &signal, -1);
  signal_mixer_add_input_signal(MIXER(mixer), signal);

  gtk_list_store_append(mixer->input_signals, &iter);
  gtk_list_store_set(mixer->input_signals, &iter, 0, signal_path, -1);
}

void delune_mixer_remove_input_signal(DeluneMixer *mixer, GtkTreeIter *input_iter) {
  GtkTreeIter iter;
  GtkTreePath *signal_path;
  GtkTreeModel *model;
  Signal *signal;

  if (!mixer->tracked_signals) {
    fprintf(stderr, "DeluneMixer error: tracked_signals is null\n");
    exit(1);
  }

  gtk_tree_model_get(GTK_TREE_MODEL(mixer->input_signals), input_iter, 0, &signal_path, -1);

  model = GTK_TREE_MODEL(mixer->tracked_signals);
  gtk_tree_model_get_iter(model, &iter, signal_path);
  gtk_tree_model_get(model, &iter, 0, &signal, -1);
  signal_mixer_remove_input_signal(MIXER(mixer), signal);

  gtk_list_store_remove(mixer->input_signals, input_iter);
  gtk_tree_path_free(signal_path);
}

void print_input_signals(Mixer *mixer) {
  for (int i = 0; i < mixer->input_signal_count; i++) {
    printf("signal: %p\t(%s)\n", mixer->input_signals[i], signal_get_name(mixer->input_signals[i]));
  }
}

void delune_mixer_update_signals(DeluneMixer *mixer) {
  int mixer_index;
  GtkTreeIter out_iter, tracked_iter;
  GtkTreePath *path_to_signal;
  GtkTreeModel *model;
  Signal *signal;
  gboolean is_valid;

  model = GTK_TREE_MODEL(mixer->input_signals);
  mixer_index = 0;
  is_valid = gtk_tree_model_get_iter_first(model, &out_iter);
  for (; is_valid; is_valid = gtk_tree_model_iter_next(model, &out_iter)) {
    gtk_tree_model_get(model, &out_iter, 0, &path_to_signal, -1);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(mixer->tracked_signals), &tracked_iter, path_to_signal);
    gtk_tree_model_get(GTK_TREE_MODEL(mixer->tracked_signals), &tracked_iter, 0, &signal, -1);

    if (mixer_index < MIXER(mixer)->input_signal_count)
      MIXER(mixer)->input_signals[mixer_index] = signal;

    mixer_index++;
  }
}

GtkListStore *delune_mixer_get_input_signals(DeluneMixer *mixer) {
  return mixer->input_signals;
}
