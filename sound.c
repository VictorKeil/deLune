#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <soundio/soundio.h>

#include "sound.h"
#include "signal.h"
#include "ui.h"

static struct SoundIo *soundio;
static struct SoundIoDevice *device;
static struct SoundIoOutStream *outstream;

struct timespec last_time, now_time;

static void write_callback(struct SoundIoOutStream *outstream,
		      int frame_count_min, int frame_count_max) {


  struct SoundIoChannelLayout *layout = &outstream->layout;
  struct SoundIoChannelArea *areas;
  int frames_left = frame_count_max;
  float seconds_per_frame = 1.0f / outstream->sample_rate;
  Sound *sound = outstream->userdata;

  int err;
  int frame_count;
  while (frames_left > 0) {
    frame_count = sound->buffer_size;

    if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
      fprintf(stderr, "%s\n", soundio_strerror(err));
      exit(1);
    }

    if (!frame_count) {
      break;
    }
      
    for (int frame = 0; frame < frame_count; frame++) {
      float sample = signal_get_value(SIGNAL(sound->master_out), frame * seconds_per_frame);
      for (int channel = 0; channel < layout->channel_count; channel++) {
	float *ptr = (float*) (areas[channel].ptr + frame * areas[channel].step);
	*ptr = sample;
      }
    }

    signal_sync(sound->signal_table, frame_count * seconds_per_frame);

    if ((err = soundio_outstream_end_write(outstream))) {
      fprintf(stderr, "%s\n", soundio_strerror(err));
      exit(1);
    }

    frames_left -= frame_count;
  }
}

Sound *sound_create() {
  int err;
  soundio = soundio_create();

  if (!soundio) {
    fprintf(stderr, "out of memory!\n");
    exit(1);
  }

  if ((err = soundio_connect_backend(soundio, SoundIoBackendAlsa))) {
    fprintf(stderr, "error connecting: %s\n", soundio_strerror(err));
    exit(1);
  }

  soundio_flush_events(soundio);

  int default_out_device_index = soundio_default_output_device_index(soundio);
  if (default_out_device_index < 0) {
    fprintf(stderr, "no output device found\n");
    exit(1);
  }

  struct SoundIoDevice *device = soundio_get_output_device(soundio, default_out_device_index);
  if (!device) {
    fprintf(stderr, "out of memory!\n");
    exit(1);
  }

  fprintf(stderr, "Output device: %s\n", device->name);

  outstream = soundio_outstream_create(device);
  if (!outstream) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  outstream->format = SoundIoFormatFloat32NE;
  outstream->write_callback = write_callback;

  if ((err = soundio_outstream_open(outstream))) {
    fprintf(stderr, "unable to open device: %s\n", soundio_strerror(err));
    exit(1);
  }

  if (outstream->layout_error)
    fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));

  Sound *sound;
  sound = calloc(1, sizeof(sound));
  sound->buffer_size = 256;
  outstream->userdata = sound;

  if ((err = soundio_outstream_start(outstream))) {
    fprintf(stderr, "unable to start device: %s\n", soundio_strerror(err));
    exit(1);
  }

  sound->signal_table = signal_new_signal_table(outstream->sample_rate);
  sound->master_out = signal_new_mixer(sound->signal_table);
  signal_set_name(SIGNAL(sound->master_out), "master out");

  return sound;
}

void close_soundio() {
  soundio_outstream_destroy(outstream);
  soundio_device_unref(device);
  soundio_destroy(soundio);
 }
