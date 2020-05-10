#include <stdio.h>
#include <stdbool.h>

#include "ui.h"
#include "sound.h"
#include "signal.h"

ConstSignalRange *gtk_signals[16];
int gtk_signal_counter;

static void on_signal_range_change(GtkWidget *widget, gpointer data) {
  CONST_SIGNAL(data)->value = gtk_range_get_value(GTK_RANGE(widget));
}

GtkWidget *get_signal_scale(ConstSignalRange *range_signal) {
  GtkWidget *v_scale = gtk_scale_new_with_range(
      GTK_ORIENTATION_VERTICAL, range_signal->min, range_signal->max, .1);

  gtk_scale_set_draw_value(GTK_SCALE(v_scale), true);
  gtk_scale_set_value_pos(GTK_SCALE(v_scale), GTK_POS_BOTTOM);
  gtk_range_set_inverted(GTK_RANGE(v_scale), true);

  g_signal_connect(GTK_RANGE(v_scale), "value-changed",
                   G_CALLBACK(on_signal_range_change), range_signal->signal);

  return v_scale;
}

void ui_new_const_signal_range(ConstSignal *signal, float min, float max) {
  ConstSignalRange *signal_range = malloc(sizeof(ConstSignalRange));
  signal_range->signal = signal;
  signal_range->min = min;
  signal_range->max = max;

  gtk_signals[gtk_signal_counter++] = signal_range;
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *main_box;
  GtkWidget *scale_box;
  GtkWidget *spacer_box;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "gui-test");
  gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

  main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_vexpand(main_box, true);
  gtk_container_add(GTK_CONTAINER(window), main_box);

  spacer_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER(main_box), spacer_box);

  scale_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_valign(scale_box, GTK_ALIGN_FILL);
  gtk_widget_set_vexpand(scale_box, true);
  gtk_container_add(GTK_CONTAINER(main_box), scale_box);

  GtkWidget *scale;
  for (int i = 0; i < sizeof(gtk_signals) / sizeof(gtk_signals[0]); i++) {
    if (gtk_signals[i]) {
      fprintf(stderr, "adding signal %p\n", gtk_signals[i]);
      scale = get_signal_scale(gtk_signals[i]);
      gtk_container_add(GTK_CONTAINER(scale_box), scale);
    }
  }

  gtk_widget_show_all(window);
}

GtkApplication *setup_gtk() {
  g_print("getting new gtk app...\n");
  GtkApplication *app = gtk_application_new("org.keil.gui-test", G_APPLICATION_FLAGS_NONE);
  g_print("connecting signal...\n");
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  return app;
}

void close_gtk(GtkApplication *app) { g_object_unref(app); }
