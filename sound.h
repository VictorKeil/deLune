#ifndef _SOUND_H_
#define _SOUND_H_

#include "signal.h"

typedef struct _Sound Sound;

struct _Sound {
  int buffer_size;
  
  SignalTable *signal_table;
  Mixer *master_out;
};

Sound *sound_create();

void close_soundio();

#endif /* _SOUND_H_ */
