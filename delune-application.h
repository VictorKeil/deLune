#ifndef _DELUNE_APPLICATION_H_
#define _DELUNE_APPLICATION_H_

#include <gtk/gtk.h>

#include "sound.h"

#define DELUNE_APPLICATION_TYPE (delune_application_get_type())
G_DECLARE_FINAL_TYPE(DeluneApplication, delune_application, DELUNE, APPLICATION, GtkApplication)

struct _DeluneApplication {
  GtkApplication base_instance;

  Sound *sound;
  GtkAdjustment *master_amplitude_adjustment;

  GArray *tracked_signals;
};

DeluneApplication *delune_application_new (void);

void delune_add_signal(DeluneApplication *app, Signal *signal);

#endif /* _DELUNE-APPLICATION_H_ */
