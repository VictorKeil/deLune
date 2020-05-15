#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <stdbool.h>

#define INIT_SIGNALS_LEN 64

#define SIGNAL(signal) ((Signal*) signal)
#define CONST_SIGNAL(signal) ((ConstSignal*) signal)
#define SYNCED_SIGNAL(signal) ((SyncedSignal*) signal)
#define WAVE_SIGNAL(signal) ((WaveSignal*) signal)
#define INTEGRAL_SIGNAL(signal) ((IntegralSignal*) signal)
#define MIXER(signal) ((Mixer*) signal)

typedef struct _SignalTable SignalTable;
typedef struct _Signal Signal;
typedef struct _Signal ConstSignal;
typedef struct _SyncedSignal SyncedSignal;
typedef struct _IntegralSignal IntegralSignal;
typedef struct _WaveSignal WaveSignal;
typedef struct _Mixer Mixer;


struct _SignalTable {
  int signals_len, signal_count;
  Signal **signals;

  int synced_signals_size, synced_signal_count;
  SyncedSignal **synced_signals;

  int sample_rate;
};


struct _Signal {
  void (*destructor)(Signal*);

  SignalTable *table;

  int reference_count;

  char *name;

  int input_signals_size, input_signal_count;
  Signal **input_signals;

  float (*function)(struct _Signal*, float);
  float value;

  float (*compute_period)(Signal*);
};

struct _SyncedSignal {
  Signal parent;
  
  void (*sync_function)(SyncedSignal*, float);

  float seconds_offset;
};


struct _IntegralSignal {
  struct _SyncedSignal parent;

  float dt;
  float value;
};

struct _WaveSignal {
  struct _SyncedSignal parent;

  IntegralSignal *frequency;
  Signal *amplitude;
};

struct _Mixer {
  Signal parent;

  int input_signals_size;
  int input_signal_count;
  Signal **input_signals;
  Signal **input_amplitudes;

  Signal *master_amplitude;
};

SignalTable *
signal_new_signal_table(int sample_rate);

void signal_init(SignalTable *table, Signal *signal);

void signal_add_to_collection(Signal ***collection, int *size, int *count, Signal *signal);

void signal_destroy(Signal *signal);

char *signal_get_name(Signal *signal);

void signal_set_name(Signal *signal, char *name);

float signal_get_value(Signal *signal, float time_seconds);

void signal_sync(SignalTable *table, float seconds_offset);

void signal_set_input_signal(Signal *signal, Signal *input_signal);

void signal_set_frequency(WaveSignal *signal, Signal *frequency, bool no_unref);

Signal *signal_get_frequency(WaveSignal *signal);

void signal_set_amplitude(WaveSignal *signal, Signal *amplitude, bool no_unref);

Signal *signal_get_amplitude(WaveSignal *signal);


ConstSignal *signal_new_const_signal(SignalTable *table, float value);

WaveSignal *signal_new_saw_wave(SignalTable *table, float frequency, float amplitude);

WaveSignal *signal_new_sine_wave(SignalTable *table, float frequency, float amplitude);

IntegralSignal *signal_new_integral_signal(SignalTable *table, float step, Signal *input_signal);

void signal_mixer_init(Mixer *mixer);

Mixer *signal_new_mixer(SignalTable *table);

void signal_mixer_add_input_signal(Mixer *mixer, Signal *signal);

void signal_mixer_remove_input_signal(Mixer *mixer, Signal *signal);

void signal_mixer_set_master_amplitude(Mixer *mixer, float amplitude);

Signal *signal_new_adapter(SignalTable *table);

void signal_adapter_set_input(Signal *adapter, Signal *input);

Signal *signal_adapter_get_input(Signal *adapter);

#endif /* _SIGNAL_H_ */
