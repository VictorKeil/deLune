#include <stdbool.h>
#include <gtk/gtk.h>

#include "delune-window.h"
#include "delune-application.h"
#include "delune-signal-chooser.h"
#include "delune-signal-component.h"


struct _DeluneWindow {
  GtkApplicationWindow base_window;

  DeluneApplication *app;

  GtkWidget *stack;
  GtkWidget *signal_grid;
  GtkWidget *master_amplitude;
};

G_DEFINE_TYPE(DeluneWindow, delune_window, GTK_TYPE_APPLICATION_WINDOW);

void on_signal_add_to_output(GtkWidget *button, gpointer data) {

}

void on_new_signal(GtkWidget *button, gpointer app) {
  
}

void update_signal_grid(DeluneWindow *win) {
  
}

static void delune_window_init(DeluneWindow *win) {
  gtk_widget_init_template(GTK_WIDGET(win));
}

static void delune_window_class_init(DeluneWindowClass *class) {
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
					      "/home/victor/workspace/c/delune/ui/delune-window.ui");

  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, stack);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, signal_grid);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), DeluneWindow, master_amplitude);

  gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(class), on_signal_add_to_output);
  gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(class), on_new_signal);
}

static void construct_ui(DeluneWindow *win) {
  DeluneSignalComponent *component;

  component = delune_signal_component_new(win->app);
  gtk_container_add(GTK_CONTAINER(win->signal_grid), GTK_WIDGET(component));
}

DeluneWindow *delune_window_new(DeluneApplication *app) {
  DeluneWindow *win = DELUNE_WINDOW(g_object_new(DELUNE_WINDOW_TYPE, "application", app, NULL));

  win->app = app;
  gtk_range_set_adjustment(GTK_RANGE(win->master_amplitude), app->master_amplitude_adjustment);

  construct_ui(win);

  return win;
}
