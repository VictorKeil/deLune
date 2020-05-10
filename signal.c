#include <stdlib.h>
#include <stdio.h>
#include <soundio/soundio.h>
#include <math.h>
#include <string.h>

#include "signal.h"
#include "utils.h"

#define INIT_SYNCED_SIGNALS_LEN 32
#define INIT_MIXER_INPUT_LEN 16

#define SIGNALS_EXPAND_LEN 32
#define SYNCED_SIGNALS_EXPAND_LEN 16

void init_signal(SignalTable *table, Signal *signal) {
  /* signal->table = table; */
}

SignalTable *signal_new_signal_table(int sample_rate) {
  SignalTable *table = calloc(1, sizeof(SignalTable));

  table->sample_rate = sample_rate;

  table->signals = calloc(INIT_SIGNALS_LEN, sizeof(Signal*));
  table->signals_len = INIT_SIGNALS_LEN;

  table->synced_signals_size = INIT_SYNCED_SIGNALS_LEN;
  init_array(&table->synced_signals, sizeof(Signal*), table->synced_signals_size);

  return table;
}

void signal_add_to_collection(Signal ***collection, int *size, int *count, Signal *signal) {
  if (*count >= *size) {
    int new_byte_size = *size + sizeof(Signal *) * SIGNALS_EXPAND_LEN;
    *collection = realloc(*collection, new_byte_size);
    *size += SIGNALS_EXPAND_LEN;
  }

  (*collection)[(*count)++] = signal;
}

void signal_table_add_signal(SignalTable *table, Signal *signal) {
  signal_add_to_collection(&table->signals,
		    &table->signals_len,
		    &table->signal_count,
		    signal);
}

void signal_table_add_synced_signal(SignalTable *table, SyncedSignal *signal) {
  signal_add_to_collection((Signal ***) &table->synced_signals,
		    &table->synced_signals_size,
		    &table->synced_signal_count,
		    SIGNAL(signal));
}

int remove_from_collection(Signal **collection, int size, int count, Signal *signal) {
  Signal *sig;
  for (int i = 0; i < count; i++) {
    sig = collection[i];
    if (sig == signal) {
      collection[i] = collection[count];
      return count - 1;
    }
  }

  return count;
}

void signal_table_remove_signal(Signal *signal) {
  SignalTable *table = signal->table;
  table->signal_count = remove_from_collection(
      table->signals, table->signals_len, table->signal_count, signal);
}

void signal_table_remove_synced_signal(SyncedSignal *signal) {
  SignalTable *table = SIGNAL(signal)->table;
  table->synced_signal_count = remove_from_collection((Signal **) table->synced_signals,
						      table->synced_signals_size,
						      table->synced_signal_count,
						      SIGNAL(signal));
}

void signal_destroy(Signal *signal) {
  /* if (signal) { */
  /*   printf("in destroy: signal: %p\n", signal); */
  /*   signal_table_remove_signal(signal); */
  /*   signal_table_remove_synced_signal(signal); */
  /*   free(signal); */
  /* } */
}

void signal_set_name(Signal *signal, char *name) {
  signal->name = name;
}

char *signal_get_name(Signal *signal) {
  if (!signal->name) {
    char *empty = "";
    return empty;
  }
  return signal->name;
}

float get_period(Signal *signal) {
  if (!signal->function)
    return 0;
  return SYNCED_SIGNAL(signal)->compute_period(SYNCED_SIGNAL(signal));
}

void signal_sync(SignalTable *table, float seconds_offset) {
  SyncedSignal *sig;
  for (int i = 0; i < table->synced_signal_count; i++) {
    sig = table->synced_signals[i];
    if (sig) {
      sig->sync_function(sig, seconds_offset);
    }
  }
}

void signal_set_frequency(WaveSignal *signal, Signal *frequency, bool no_unref) {
  if (!no_unref)
    SIGNAL(signal->frequency)->reference_count--;

  SIGNAL(signal->frequency)->input_signal = frequency;
}

Signal *signal_get_frequency(WaveSignal *signal) { return SIGNAL(signal->frequency)->input_signal; }

void signal_set_amplitude(WaveSignal *signal, Signal *ampliutde,
                          bool no_unref) {
  if (!no_unref)
    signal->amplitude->reference_count--;

  signal->amplitude = ampliutde;
}

Signal *signal_get_amplitude(WaveSignal *signal) { return signal->amplitude; }

void init_const_signal(SignalTable *table, ConstSignal **signal, float value) {
  *signal = calloc(1, sizeof(ConstSignal));
  init_signal(table, SIGNAL(*signal));
  (*signal)->value = value;
}

ConstSignal *signal_new_const_signal(SignalTable *table, float value) {
  ConstSignal *signal;
  init_const_signal(table, &signal, value);

  signal_table_add_signal(table, signal);
  return signal;
}

float integral_func(Signal *signal, float time_seconds) {
  IntegralSignal *integral = INTEGRAL_SIGNAL(signal);
  float value = signal_get_value(signal->input_signal, time_seconds);
  float integral_step = integral->dt * value;
  integral->value += integral_step;
  return integral->value;
}

void integral_sync_func(SyncedSignal *signal, float seconds_offset) {
  IntegralSignal *integral = INTEGRAL_SIGNAL(signal);
  float signal_offset = signal->seconds_offset;
  float signal_period = get_period(SIGNAL(signal));

  integral->value = fmod(integral->value, 1.0f);
  signal->seconds_offset = fmod(signal_offset + seconds_offset, signal_period);
}

float integral_period_func(SyncedSignal *signal) {
  return get_period(SIGNAL(signal)->input_signal);
}

IntegralSignal *signal_new_integral_signal(SignalTable *table, float step,
                                           Signal *input_signal) {
  IntegralSignal *signal;
  signal = calloc(1, sizeof(IntegralSignal));
  init_signal(table, SIGNAL(signal));

  signal->dt = step;
  SIGNAL(signal)->function = integral_func;
  SIGNAL(signal)->input_signal = input_signal;
  SYNCED_SIGNAL(signal)->sync_function = integral_sync_func;
  SYNCED_SIGNAL(signal)->compute_period = integral_period_func;

  signal_table_add_synced_signal(table, SYNCED_SIGNAL(signal));

  return signal;
}

float wave_signal_period_func(SyncedSignal *signal) {
  float period = get_period(SIGNAL(WAVE_SIGNAL(signal)->frequency));
  if (!period)
    return WAVE_SIGNAL(signal)->frequency->value;
  return period;
}

void wave_signal_sync_func(SyncedSignal *signal, float seconds_offset) {
  float signal_offset = signal->seconds_offset;
  float signal_period = get_period(SIGNAL(signal));
  signal->seconds_offset = fmod(signal_offset + seconds_offset, signal_period);
}

void init_wave_signal(SignalTable *table, WaveSignal **signal, float frequency,
                      float amplitude) {
  *signal = calloc(1, sizeof(WaveSignal));
  init_signal(table, SIGNAL(*signal));

  SYNCED_SIGNAL(*signal)->sync_function = wave_signal_sync_func;
  SYNCED_SIGNAL(*signal)->compute_period = wave_signal_period_func;
  (*signal)->frequency = signal_new_integral_signal(
      table, 1.0f / table->sample_rate, signal_new_const_signal(table, frequency));

  ConstSignal *amp = signal_new_const_signal(table, amplitude);
  signal_set_amplitude(*signal, amp, true);

  signal_table_add_synced_signal(table, SYNCED_SIGNAL(*signal));
}

float sine_wave_func(Signal *signal, float time_seconds) {
  WaveSignal *sine_wave = WAVE_SIGNAL(signal);
  float time_value = SYNCED_SIGNAL(sine_wave)->seconds_offset + time_seconds;
  float frequency =
      signal_get_value(SIGNAL(sine_wave->frequency), time_seconds);
  float amplitude = signal_get_value(sine_wave->amplitude, time_seconds);
  return amplitude * sinf(2 * M_PI * frequency);
}

WaveSignal *signal_new_sine_wave(SignalTable *table, float frequency,
                                 float amplitude) {
  WaveSignal *wave;
  init_wave_signal(table, &wave, frequency, amplitude);
  SIGNAL(wave)->function = sine_wave_func;

  return wave;
}

float saw_wave_func(Signal *signal, float time_seconds) {
  float frequency =
      signal_get_value(SIGNAL(WAVE_SIGNAL(signal)->frequency), time_seconds);
  float amplitude =
      signal_get_value(WAVE_SIGNAL(signal)->amplitude, time_seconds);

  return amplitude * (2 * fmod(fabs(frequency), 1) - 1);
}

WaveSignal *signal_new_saw_wave(SignalTable *table, float frequency,
                                float amplitude) {
  WaveSignal *wave;
  init_wave_signal(table, &wave, frequency, amplitude);
  SIGNAL(wave)->function = saw_wave_func;

  return wave;
}

float mixer_func(Signal *mixer_signal, float time_seconds) {
  Mixer *mixer = MIXER(mixer_signal);

  float output_value = 0.0f;
  float input_signal_value;
  for (int i = 0; i < mixer->input_signal_count; i++) {
    input_signal_value =
        signal_get_value(mixer->input_signals[i], time_seconds);
    input_signal_value *=
        signal_get_value(mixer->input_amplitudes[i], time_seconds);
    output_value += input_signal_value;
  }

  float master_amplitude =
      signal_get_value(mixer->master_amplitude, time_seconds);
  return master_amplitude * output_value;
}

Mixer *signal_new_mixer(SignalTable *table) {
  Mixer *mixer;
  mixer = calloc(1, sizeof(Mixer));
  init_signal(table, SIGNAL(mixer));

  SIGNAL(mixer)->function = mixer_func;

  mixer->master_amplitude = signal_new_const_signal(table, .5);

  init_array(&mixer->input_signals, sizeof(Signal), INIT_MIXER_INPUT_LEN);
  init_array(&mixer->input_amplitudes, sizeof(Signal), INIT_MIXER_INPUT_LEN);
  for (int i = 0; i < INIT_MIXER_INPUT_LEN; i++) {
    mixer->input_amplitudes[i] = signal_new_const_signal(table, 1);
  }

  return mixer;
}

void signal_mixer_add_input_signal(Mixer *mixer, Signal *signal) {
  signal_add_to_collection(&mixer->input_signals, &mixer->input_signals_size,
                    &mixer->input_signal_count, signal);
}

void signal_mixer_set_master_amplitude(Mixer *mixer, float amplitude) {
  mixer->master_amplitude->value = amplitude;
}

float signal_get_value(Signal *signal, float time_seconds) {
  if (signal->function)
    return signal->function(signal, time_seconds);
  else
    return signal->value;
}