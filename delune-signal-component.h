#ifndef _DELUNE_SIGNAL_COMPONENT_H_
#define _DELUNE_SIGNAL_COMPONENT_H_

#include <gtk/gtk.h>

#include "delune-application.h"
#include "signal.h"

#define DELUNE_SIGNAL_COMPONENT_TYPE (delune_signal_component_get_type())
G_DECLARE_FINAL_TYPE(DeluneSignalComponent, delune_signal_component, DELUNE, SIGNAL_COMPONENT, GtkBox);

DeluneSignalComponent *delune_signal_component_new(DeluneApplication *app);

#endif /* _DELUNE-SIGNAL-COMPONENT_H_ */
