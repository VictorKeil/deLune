#ifndef UI_H_
#define UI_H_

#include <gtk/gtk.h>

#include "sound.h"
#include "signal.h"

typedef struct _ConstSignalRange ConstSignalRange;

struct _ConstSignalRange {
  ConstSignal *signal;
  float min;
  float max;
};

extern ConstSignalRange *gtk_signals[16];

GtkApplication *setup_gtk();
void close_gtk(GtkApplication *app);

void ui_new_const_signal_range(ConstSignal *signal, float min, float max);

GtkWidget *get_signal_scale(ConstSignalRange *signal);

#endif
