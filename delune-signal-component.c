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

  GtkWidget *header;
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

void delune_signal_component_set_signal(DeluneSignalComponent *component, Signal *signal) {
  component->signal = signal;
}

static void on_waveform_changed(GtkWidget *curtain, DeluneSignalComponent *component) {
  Signal *old_signal;
  const char *active_id;

  active_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(curtain));

  /* if (component->signal) */
  /*   delune_signal_destroy(component->app, component->signal); */

  old_signal = component->signal;
  if (!strcmp(active_id, "sine_wave")) {
    component->signal = SIGNAL(signal_new_sine_wave(component->app->sound->signal_table, 440, 1));
  }

  if (!strcmp(active_id, "saw_wave")) {
    component->signal = SIGNAL(signal_new_saw_wave(component->app->sound->signal_table, 440, 1));
  }

  delune_signal_replace(component->app, old_signal, component->signal);

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

static void on_name_changed(GtkWidget *buffer, gpointer component) {
  GtkTextIter start, end;

  gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);
  gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(buffer), "name_header", &start, &end);

  char *name;
  g_object_get(buffer, "text", &name, NULL);
  //TODO: handle unicode characters
  signal_set_name(DELUNE_SIGNAL_COMPONENT(component)->signal, name);
}

static void delune_signal_component_init(DeluneSignalComponent *component) {
  GtkTextBuffer *buffer;
  GtkTextTagTable *tag_table;
  GtkTextTag *name_tag;
  GHashTable *signal_param_ids;

  gtk_widget_init_template(GTK_WIDGET(component));

  g_signal_connect(component->waveform_curtain, "changed", G_CALLBACK(on_waveform_changed), component);

  buffer = gtk_text_buffer_new(NULL);

  g_signal_connect(buffer, "changed", G_CALLBACK(on_name_changed), component);
  gtk_text_view_set_buffer(GTK_TEXT_VIEW(component->header), buffer);
  
  name_tag = gtk_text_tag_new("name_header");
  g_object_set(name_tag, "size", 12000, "weight", PANGO_WEIGHT_BOLD, NULL);
  tag_table = gtk_text_buffer_get_tag_table(buffer);
  gtk_text_tag_table_add(tag_table, name_tag);

  signal_param_ids = g_hash_table_new(g_str_hash, g_str_equal);
  g_hash_table_insert(signal_param_ids, WAVE_SIGNAL_PARAMS, WAVE_SIGNAL_IDS);
  g_hash_table_insert(signal_param_ids, CONST_SIGNAL_PARAMS, CONST_SIGNAL_IDS);
  component->signal_param_ids = signal_param_ids;
}

static void construct_ui(DeluneSignalComponent *component) {
  GtkWidget *wave_signal_params;
  DeluneSignalChooser *chooser;
  GtkTextBuffer *buffer;
  char *name;

  name = signal_get_name(component->signal);
  if (name) {
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(component->header));
    gtk_text_buffer_set_text(buffer, name, -1);
  }

  wave_signal_params = gtk_bin_get_child(GTK_BIN(component->wave_signal_params));

  chooser = delune_signal_chooser_new(component->app->tracked_signals);
  delune_signal_chooser_set_label(DELUNE_SIGNAL_CHOOSER(chooser), "Frequency: ");
  gtk_container_add(GTK_CONTAINER(wave_signal_params), GTK_WIDGET(chooser));

  chooser = delune_signal_chooser_new(component->app->tracked_signals);
  delune_signal_chooser_set_label(DELUNE_SIGNAL_CHOOSER(chooser), "Amplitude: ");
  gtk_container_add(GTK_CONTAINER(wave_signal_params), GTK_WIDGET(chooser));
}

static void delune_signal_component_class_init(DeluneSignalComponentClass *class) {
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
					      "/home/victor/workspace/c/delune/ui/delune-signal-component.ui");
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, header);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, param_list);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, waveform_curtain);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, wave_signal_params);
}

DeluneSignalComponent *delune_signal_component_new(DeluneApplication *app, Signal *signal) {
  DeluneSignalComponent *comp = DELUNE_SIGNAL_COMPONENT(g_object_new(DELUNE_SIGNAL_COMPONENT_TYPE, NULL));
  comp->app = app;
  comp->signal = signal;
  construct_ui(comp);

  return comp;
}
