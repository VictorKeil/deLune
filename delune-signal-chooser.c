#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>

#include "signal.h"
#include "delune-signal-chooser.h"
#include "utils.h"

struct _DeluneSignalChooser {
  GtkBox base_widget;

  GtkWidget *label;
  GtkWidget *signal_curtain;
  GtkWidget *value_scale;

  GtkListStore *tracked_signals;
};

G_DEFINE_TYPE(DeluneSignalChooser, delune_signal_chooser, GTK_TYPE_BOX);

static void on_signal_changed(GtkWidget * curtain, gpointer button) {
  const gchar *id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(curtain));
  bool visibility = false;
  if (id)
    visibility = strcmp(id, "constant_value") ? false : true;
  gtk_widget_set_visible(GTK_WIDGET(button), visibility);
}

void delune_signal_chooser_set_label(DeluneSignalChooser *chooser, char *label) {
  gtk_label_set_text(GTK_LABEL(chooser->label), label);
}

static gboolean print_node_func(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data) {
  Signal *signal;
  gtk_tree_model_get(model, iter, 0, &signal, -1);
  printf("name: %s\n", signal_get_name(signal));

  return FALSE;
}

static void setup_signal_curtain(DeluneSignalChooser *chooser) {
  GtkCellRenderer *r = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
  gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(chooser->signal_curtain), r, delune_map_signal_name_func, NULL, NULL);
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(chooser->signal_curtain), r, TRUE);

  gtk_combo_box_set_model(GTK_COMBO_BOX(chooser->signal_curtain), GTK_TREE_MODEL(chooser->tracked_signals));
}

static void delune_signal_chooser_init(DeluneSignalChooser *chooser) {
  gtk_widget_init_template(GTK_WIDGET(chooser));

  g_signal_connect(chooser->signal_curtain, "changed", G_CALLBACK(on_signal_changed), chooser->value_scale);
}

static void delune_signal_chooser_class_init(DeluneSignalChooserClass *class) {
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
					      "/home/victor/workspace/c/delune/ui/delune-signal-chooser.ui");

  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalChooser, label);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalChooser, signal_curtain);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalChooser, value_scale);
}

DeluneSignalChooser *delune_signal_chooser_new(GtkListStore *tracked_signals) {
  DeluneSignalChooser *chooser = DELUNE_SIGNAL_CHOOSER(g_object_new(DELUNE_SIGNAL_CHOOSER_TYPE, NULL));
  chooser->tracked_signals = tracked_signals;
  setup_signal_curtain(chooser);

  return chooser;
}
