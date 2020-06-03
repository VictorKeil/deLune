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
  GtkHeaderBar *header_bar;
  GtkWidget *waveform_curtain;
  GtkListBox *param_list;
  GtkWidget *value_scale, *value_spin_button;
  GtkWidget *wave_signal_params;
  GHashTable *signal_param_ids;
};

G_DEFINE_TYPE(DeluneSignalComponent, delune_signal_component, GTK_TYPE_BOX);

static void on_header_grab_focus(DeluneSignalComponent *component, void *event, GtkTextView *header) {
  GtkTextIter start, end;
  GtkTextBuffer *buffer;

  buffer = gtk_text_view_get_buffer(header);
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  gtk_text_buffer_select_range(buffer, &start, &end);
}

void delune_signal_component_set_signal(DeluneSignalComponent *component, Signal *signal) {
  component->signal = signal;
}

static void set_param_list_visibility(DeluneSignalComponent *component) {
  const char *active_id;
  active_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(component->waveform_curtain));
  
  GList *param_rows = gtk_container_get_children(GTK_CONTAINER(component->param_list));
  const char *name;
  char **valid_ids;
  bool visibility;
  for (GList *elem = param_rows; elem != NULL; elem = elem->next) {
    name = gtk_widget_get_name(GTK_WIDGET(elem->data));
    visibility = false;
    valid_ids = g_hash_table_lookup(component->signal_param_ids, name);

    for (int i = 0; valid_ids[i] != NULL; i++) {
      if (!strcmp(active_id, valid_ids[i]))
	visibility = true;
      
      gtk_widget_set_visible(GTK_WIDGET(elem->data), visibility);
    }
  }
}

static void on_waveform_changed(GtkWidget *curtain, DeluneSignalComponent *component) {
  Signal *new_signal;
  const char *active_id;

  active_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(curtain));

  if (!strcmp(active_id, "sine_wave")) {
    new_signal = SIGNAL(signal_new_sine_wave(component->app->sound->signal_table, 440, 1));
  }

  if (!strcmp(active_id, "saw_wave")) {
    new_signal = SIGNAL(signal_new_saw_wave(component->app->sound->signal_table, 440, 1));
  }

  delune_signal_replace(component->app, component->signal, new_signal);

  set_param_list_visibility(component);
}

static void on_name_changed(GtkWidget *buffer, gpointer component) {
  GtkTextIter start, end;

  gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);
  gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(buffer), "name_header", &start, &end);

  char *name;
  int max_name_len = 80;
  char *name_intern;
  g_object_get(buffer, "text", &name, NULL);
  //TODO: handle unicode characters
  name_intern = malloc(max_name_len);
  snprintf(name_intern, max_name_len, "%s (intern)", name);

  signal_set_name(DELUNE_SIGNAL_COMPONENT(component)->signal, name);
  signal_set_name(signal_adapter_get_input(DELUNE_SIGNAL_COMPONENT(component)->signal), name_intern);
}

static void on_const_value_changed(GtkAdjustment *adj, DeluneSignalComponent *component) {
  signal_adapter_get_input(component->signal)->value = gtk_adjustment_get_value(adj);
}

static void on_frequency_changed(GtkComboBox *freq_curtain, DeluneSignalComponent *comp) {
  GtkTreeModel *model;
  GtkTreeIter iter;
  Signal *intern_signal, *frequency_signal;

  model = gtk_combo_box_get_model(freq_curtain);
  gtk_combo_box_get_active_iter(freq_curtain, &iter);
  gtk_tree_model_get(model, &iter, 0, &frequency_signal, -1);

  intern_signal = signal_adapter_get_input(comp->signal);
  signal_set_frequency(WAVE_SIGNAL(intern_signal), frequency_signal, FALSE);
}

static void on_amplitude_changed(GtkComboBox *amp_curtain, DeluneSignalComponent *comp) {
  GtkTreeModel *model;
  GtkTreeIter iter;
  Signal *amp_signal;

  model = gtk_combo_box_get_model(amp_curtain);
  gtk_combo_box_get_active_iter(amp_curtain, &iter);
  gtk_tree_model_get(model, &iter, 0, &amp_signal, -1);

  signal_set_amplitude(WAVE_SIGNAL(signal_adapter_get_input(comp->signal)), amp_signal, FALSE);
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

  g_signal_connect(component->header, "focus-in-event", G_CALLBACK(on_header_grab_focus), component->header);
}

static void construct_ui(DeluneSignalComponent *component) {
  GtkAdjustment *const_value_adjustment;
  GtkWidget *wave_signal_params;
  DeluneSignalChooser *chooser;
  GtkTextBuffer *buffer;
  char *name;

  name = signal_get_name(component->signal);
  if (name) {
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(component->header));
    gtk_text_buffer_set_text(buffer, name, -1);
  }

  const_value_adjustment = gtk_adjustment_new(0, 0, 20000, .1, 0, 0);
  g_object_set(const_value_adjustment, "value", component->signal->value, NULL);
  gtk_range_set_adjustment(GTK_RANGE(component->value_scale), const_value_adjustment);
  gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(component->value_spin_button), const_value_adjustment);
  g_signal_connect(const_value_adjustment, "value-changed", G_CALLBACK(on_const_value_changed), component);

  wave_signal_params = gtk_bin_get_child(GTK_BIN(component->wave_signal_params));

  chooser = delune_signal_chooser_new(component->app->tracked_signals);
  delune_signal_chooser_set_label(DELUNE_SIGNAL_CHOOSER(chooser), "Frequency: ");
  gtk_container_add(GTK_CONTAINER(wave_signal_params), GTK_WIDGET(chooser));
  g_signal_connect(chooser->signal_curtain, "changed", G_CALLBACK(on_frequency_changed), component);

  chooser = delune_signal_chooser_new(component->app->tracked_signals);
  delune_signal_chooser_set_label(DELUNE_SIGNAL_CHOOSER(chooser), "Amplitude: ");
  gtk_container_add(GTK_CONTAINER(wave_signal_params), GTK_WIDGET(chooser));
  g_signal_connect(chooser->signal_curtain, "changed", G_CALLBACK(on_amplitude_changed), component);
}

static void delune_signal_component_class_init(DeluneSignalComponentClass *class) {
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
					      "/home/victor/workspace/c/delune/ui/delune-signal-component.ui");
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, header);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, param_list);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, waveform_curtain);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, value_scale);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, value_spin_button);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneSignalComponent, wave_signal_params);
}

DeluneSignalComponent *delune_signal_component_new(DeluneApplication *app, Signal *signal) {
  DeluneSignalComponent *comp = DELUNE_SIGNAL_COMPONENT(g_object_new(DELUNE_SIGNAL_COMPONENT_TYPE, NULL));
  comp->app = app;
  comp->signal = signal;
  construct_ui(comp);
  set_param_list_visibility(comp);

  return comp;
}
