#include <gtk/gtk.h>
#include <string.h>

#include "delune-application.h"
#include "delune-signal-chooser.h"
#include "delune-signal-component.h"
#include "signal.h"

char *WAVE_SIGNAL_PARAMS = "wave_signal_params";
const char *WAVE_SIGNAL_IDS[] = {"sine_wave", "saw_wave", NULL};
char *CONST_SIGNAL_PARAMS = "const_signal_params";
const char *CONST_SIGNAL_IDS[] = {"const_signal", NULL};

struct _DeluneSignalComponent {
  GtkBox base_widget;

  DeluneApplication *app;
  Signal *signal;

  GtkWidget *waveform_curtain;
  GtkListBox *param_list;
  GtkWidget *wave_signal_params;
  GHashTable *signal_param_ids;
};

G_DEFINE_TYPE(DeluneSignalComponent, delune_signal_component, GTK_TYPE_BOX);

static void update_component(DeluneSignalComponent *component) {
  //TODO: update choosers to reflect tracked signals, alternatively,
  // use a GtkTreemodel to store the data
}

static void on_waveform_changed(GtkWidget *curtain, gpointer component_ptr) {
  DeluneSignalComponent *component = DELUNE_SIGNAL_COMPONENT(component_ptr);
  const char *active_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(curtain));

  if (!strcmp(active_id, "sine_wave")) {
    //TODO destroy previous signal, if any, make new one
  }

  GList *param_rows = gtk_container_get_children(GTK_CONTAINER(component->param_list));
  const char *name;
  char **valid_ids;
  bool visibility;
  for (GList *elem = param_rows; elem != NULL; elem = elem->next) {
    name = gtk_widget_get_name(GTK_WIDGET(elem->data));
    printf("current row: %s\n", name);

    visibility = false;
    valid_ids = g_hash_table_lookup(component->signal_param_ids, name);
    for (int i = 0; valid_ids[i] != NULL; i++) {
      if (!strcmp(active_id, valid_ids[i]))
	visibility = true;
      
      gtk_widget_set_visible(GTK_WIDGET(elem->data), visibility);
    }
  }

}

static void delune_signal_component_init(DeluneSignalComponent *component) {
  gtk_widget_init_template(GTK_WIDGET(component));

  g_signal_connect(component->waveform_curtain, "changed", G_CALLBACK(on_waveform_changed), component);

  GHashTable *signal_param_ids;
  signal_param_ids = g_hash_table_new(g_str_hash, g_str_equal);
  g_hash_table_insert(signal_param_ids, WAVE_SIGNAL_PARAMS, WAVE_SIGNAL_IDS);
  g_hash_table_insert(signal_param_ids, CONST_SIGNAL_PARAMS, CONST_SIGNAL_IDS);
  component->signal_param_ids = signal_param_ids;
}

static void construct_ui(DeluneSignalComponent *component) {
  GArray *tracked_signals = component->app->tracked_signals;
  GtkWidget *wave_signal_params;
  GtkWidget *chooser;

  wave_signal_params = gtk_bin_get_child(GTK_BIN(component->wave_signal_params));

  chooser = GTK_WIDGET(delune_signal_chooser_new(tracked_signals));
  delune_signal_chooser_set_label(DELUNE_SIGNAL_CHOOSER(chooser), "Frequency: ");
  gtk_container_add(GTK_CONTAINER(wave_signal_params), chooser);

  chooser = GTK_WIDGET(delune_signal_chooser_new(tracked_signals));
  delune_signal_chooser_set_label(DELUNE_SIGNAL_CHOOSER(chooser), "Amplitude: ");
  gtk_container_add(GTK_CONTAINER(wave_signal_params), chooser);
}

static void delune_signal_component_class_init(DeluneSignalComponentClass *class) {
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
					      "/home/victor/workspace/c/delune/ui/delune-signal-component.ui");
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, param_list);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, waveform_curtain);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, wave_signal_params);
}

DeluneSignalComponent *delune_signal_component_new(DeluneApplication *app) {
  DeluneSignalComponent *comp = DELUNE_SIGNAL_COMPONENT(g_object_new(DELUNE_SIGNAL_COMPONENT_TYPE, NULL));
  comp->app = app;
  construct_ui(comp);

  return comp;
}
