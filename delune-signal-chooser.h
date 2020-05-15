#ifndef _DELUNE_SIGNAL_CHOOSER_H_
#define _DELUNE_SIGNAL_CHOOSER_H_

#include <gtk/gtk.h>

#include "delune-signal-component.h"

#define DELUNE_SIGNAL_CHOOSER_TYPE (delune_signal_chooser_get_type())
G_DECLARE_FINAL_TYPE(DeluneSignalChooser, delune_signal_chooser, DELUNE, SIGNAL_CHOOSER, GtkBox);

struct _DeluneSignalChooser {
  GtkBox base_widget;

  GtkWidget *label;
  GtkWidget *signal_curtain;
  GtkWidget *value_scale;

  GtkListStore *tracked_signals;
};

DeluneSignalChooser *delune_signal_chooser_new(GtkListStore *tracked_signals);

void delune_signal_chooser_set_label(DeluneSignalChooser *chooser, char *label);

#endif /* _DELUNE-SIGNAL-CHOOSER_H_ */
