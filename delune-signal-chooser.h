#ifndef _DELUNE_SIGNAL_CHOOSER_H_
#define _DELUNE_SIGNAL_CHOOSER_H_

#include <gtk/gtk.h>

#include "delune-signal-component.h"

#define DELUNE_SIGNAL_CHOOSER_TYPE (delune_signal_chooser_get_type())
G_DECLARE_FINAL_TYPE(DeluneSignalChooser, delune_signal_chooser, DELUNE, SIGNAL_CHOOSER, GtkBox);

DeluneSignalChooser *delune_signal_chooser_new(GArray *tracked_signals);

void delune_signal_chooser_update(DeluneSignalChooser *chooser);

void delune_signal_chooser_set_label(DeluneSignalChooser *chooser, char *label);

#endif /* _DELUNE-SIGNAL-CHOOSER_H_ */
