#ifndef _DELUNE_APPLICATION_H_
#define _DELUNE_APPLICATION_H_

#include <gtk/gtk.h>

#include "sound.h"

#define DELUNE_APPLICATION_TYPE (delune_application_get_type())
G_DECLARE_FINAL_TYPE(DeluneApplication, delune_application, DELUNE, APPLICATION, GtkApplication)

struct _DeluneApplication {
  GtkApplication base_instance;

  Sound *sound;
  GtkListStore *master_out_signals;
  GtkAdjustment *master_amplitude_adjustment;

  GtkListStore *tracked_signals;
};

DeluneApplication *delune_application_new (void);

void delune_map_signal_name_func(GtkCellLayout *curtain, GtkCellRenderer *r,
                                 GtkTreeModel *tracked_signals,
                                 GtkTreeIter *iter, gpointer data);

void delune_signal_add(DeluneApplication *app, Signal *signal);

void delune_signal_replace(DeluneApplication *app, Signal *old_signal, Signal *new_signal);

Signal *delune_signal_new(DeluneApplication *app);

void delune_signal_add_to_master_out(DeluneApplication *app, Signal *signal);

void delune_signal_remove_from_master_out_depr(DeluneApplication *app, Signal *signal);

void delune_signal_remove_from_master_out(DeluneApplication *app, GtkTreeIter *iter);

void delune_signal_destroy(DeluneApplication *app, Signal *signal);

#endif /* _DELUNE-APPLICATION_H_ */
