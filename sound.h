#ifndef _SOUND_H_
#define _SOUND_H_

#include <soundio/soundio.h>

#include "signal.h"

typedef struct _Sound Sound;

struct _Sound {
  struct SoundIoOutStream *outstream;
  int buffer_size;
  
  SignalTable *signal_table;
  Mixer *master_out;
};

Sound *sound_create();

void sound_outstream_clear_buffer(Sound *sound);

void close_soundio();

#endif /* _SOUND_H_ */
