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

  GArray *tracked_signals;
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

static void delune_signal_chooser_init(DeluneSignalChooser *chooser) {
  gtk_widget_init_template(GTK_WIDGET(chooser));

  g_signal_connect(chooser->signal_curtain, "changed", G_CALLBACK(on_signal_changed), chooser->value_scale);
}

static void update_signal_list(DeluneSignalChooser *chooser) {
  GArray *tracked_signals = chooser->tracked_signals;
  Signal *signal;


  for (int i = 0; i < tracked_signals->len; i++) {
    signal = g_array_index(tracked_signals, Signal*, i);
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(chooser->signal_curtain), NULL,
                              signal_get_name(signal));
  }
}

void delune_signal_chooser_update(DeluneSignalChooser *chooser) {
  update_signal_list(chooser);
}

static void delune_signal_chooser_class_init(DeluneSignalChooserClass *class) {
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
					      "/home/victor/workspace/c/delune/ui/delune-signal-chooser.ui");

  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalChooser, label);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalChooser, signal_curtain);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalChooser, value_scale);

  /* gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(class), on_signal_changed); */
}

DeluneSignalChooser *delune_signal_chooser_new(GArray *tracked_signals) {
  DeluneSignalChooser *chooser = DELUNE_SIGNAL_CHOOSER(g_object_new(DELUNE_SIGNAL_CHOOSER_TYPE, NULL));
  chooser->tracked_signals = tracked_signals;
  update_signal_list(chooser);

  return chooser;
}
