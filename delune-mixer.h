#ifndef _DELUNE_MIXER_H_
#define _DELUNE_MIXER_H_

#include <gtk/gtk.h>

#include "signal.h"

#define DELUNE_MIXER(mixer) ((DeluneMixer*) mixer)

typedef struct _DeluneMixer DeluneMixer;

struct _DeluneMixer {
  Mixer base_signal;

  GtkListStore *input_signals;
  GtkListStore *tracked_signals;
};

DeluneMixer *delune_mixer_new(SignalTable *table);

void delune_mixer_remove_input_signal(DeluneMixer *mixer, GtkTreeIter *input_iter);

void delune_mixer_add_input_signal(DeluneMixer *mixer, GtkTreePath *signal_path);

void delune_mixer_update_signals(DeluneMixer *mixer);

GtkListStore *delune_mixer_get_input_signals(DeluneMixer *mixer);

#endif /* _DELUNE-MIXER_H_ */
