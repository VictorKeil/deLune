#include <gtk/gtk.h>

#include "delune-application.h"
#include "delune-window.h"
#include "sound.h"
#include "signal.h"
#include "utils.h"

G_DEFINE_TYPE(DeluneApplication, delune_application, GTK_TYPE_APPLICATION);

void on_master_amplitude_changed(GtkAdjustment *adjustment, gpointer app) {
  float val = gtk_adjustment_get_value(adjustment);
  g_print("master amplitude changed to: %f\n", val);
  signal_mixer_set_master_amplitude(DELUNE_APPLICATION(app)->sound->master_out, val);
}

void delune_add_signal(DeluneApplication *app, Signal *signal) {
  g_array_append_val(app->tracked_signals, signal);
}

static void delune_application_init(DeluneApplication *app) {
  app->tracked_signals = g_array_new(false, true, sizeof(Signal*));
}

static void delune_application_activate(GApplication *app) {
  GtkAdjustment *master_amplitude_adjustment;
  DeluneWindow *win;

  master_amplitude_adjustment = gtk_adjustment_new(.4, 0, 1, .02, 0, 0);
  g_signal_connect(master_amplitude_adjustment, "value-changed", G_CALLBACK(on_master_amplitude_changed), app);
  DELUNE_APPLICATION(app)->master_amplitude_adjustment = master_amplitude_adjustment;

  win = delune_window_new(DELUNE_APPLICATION(app));
  gtk_window_present(GTK_WINDOW(win));
}

static void delune_application_class_init(DeluneApplicationClass *class) {
  G_APPLICATION_CLASS(class)->activate = delune_application_activate;
}

DeluneApplication *delune_application_new() {
  DeluneApplication *app;
  app = g_object_new(DELUNE_APPLICATION_TYPE, "application-id", "org.keil.delune", NULL);
  app->sound = sound_create();

  WaveSignal *sine = signal_new_saw_wave(app->sound->signal_table, 440, 1);
  signal_set_name(sine, "Sine 1");
  delune_add_signal(app, sine);
  signal_mixer_add_input_signal(app->sound->master_out, SIGNAL(sine));

  return app;
}
