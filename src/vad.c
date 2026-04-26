#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "vad.h"
#include "pav_analysis.h"

const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* 
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  /* 
   * DELETE and include a call to your own functions
   *
   * For the moment, compute random value between 0 and 1 
   */
  Features feat;
  feat.p = compute_power(x, N);
  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  VAD_STATE state = vad_data->state;

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x, float alpha0) {

  Features f = compute_features(x, vad_data->frame_length);

  // Guardado del ZCR para Wavesurfer
  static FILE *zcr_file = NULL;
  if (zcr_file == NULL) {
      zcr_file = fopen("pav_2111.zcr", "w");
  }
  fprintf(zcr_file, "%f\n", f.zcr);

  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  // --- NUEVAS VARIABLES DE INERCIA ---
  // Utilizamos 'static' para que mantengan su valor entre llamada y llamada
  static int count_silence = 0;
  static int count_voice = 0;

  // Estos son los umbrales de inercia (puedes jugar con estos números).
  // Representan cuántas tramas consecutivas deben cumplir la condición.
  const int HANGOVER_SILENCE = 10; // Tramas para confirmar que el silencio es real
  const int HANGOVER_VOICE = 5;    // Tramas para confirmar que la voz es real

  switch (vad_data->state) {
  case ST_INIT:
    vad_data->state = ST_SILENCE;
    vad_data->llindar_0 = f.p + alpha0; // Umbral base + alpha0
    
    // Reiniciamos contadores por seguridad
    count_silence = 0;
    count_voice = 0;
    break;

  case ST_SILENCE:
    if (f.p > vad_data->llindar_0) {
      // Posible inicio de voz, empezamos a contar
      count_voice++;
      if (count_voice >= HANGOVER_VOICE) {
        vad_data->state = ST_VOICE; // ¡Confirmado! Es voz.
        count_voice = 0;            // Reiniciamos el contador
      }
    } else {
      // Falsa alarma (ruido puntual), reiniciamos contador
      count_voice = 0;
    }
    break;

  case ST_VOICE:
    if (f.p < vad_data->llindar_0) {
      // Posible pausa o silencio, empezamos a contar
      count_silence++;
      if (count_silence >= HANGOVER_SILENCE) {
        vad_data->state = ST_SILENCE; // ¡Confirmado! El silencio es largo.
        count_silence = 0;            // Reiniciamos el contador
      }
    } else {
      // Falsa alarma (consonante sorda puntual), reiniciamos contador
      count_silence = 0;
    }
    break;

  case ST_UNDEF:
    break;
  }

  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_VOICE)
    return vad_data->state;
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}