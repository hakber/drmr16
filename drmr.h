/* drmr.h
 * DrMr 16 is a small fork of Nick Lahams DrMr LV2 plugin. All I did
 * was to turn it into a 16 channel sampler and thereby slightly
 * modifying drmr.c, drmr.h and drmr.ttl.

 * Nick Lahams version is available at <http://github.com/nicklan/drmr>

 * This code is available at <http://github.com/hakber/drmr16>

 * Copyright (c) 2012, Håkan Bernhardsson
 * 	published under GPL v3.
 * See COPYING for more information about the redistribution license

 * THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef DRMR_H
#define DRMR_H

#include <sndfile.h>
#include <pthread.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/ext/event/event.h"
#include "lv2/lv2plug.in/ns/ext/event/event-helpers.h"
#include "lv2/lv2plug.in/ns/ext/uri-map/uri-map.h"

// drumkit scanned from a hydrogen xml file
typedef struct {
  char* name;
  char* desc;
  char* path;
  char** sample_names;
  int samples;
} scanned_kit;

typedef struct {
  int num_kits;
  scanned_kit* kits;
} kits;

// libsndfile stuff

typedef struct {
  float min;
  float max;

  SF_INFO *info;
  uint32_t limit;
  float* data;
} drmr_layer;

typedef struct {
  SF_INFO *info;
  char active;
  uint32_t offset;
  uint32_t limit;
  uint32_t layer_count;
  float velocity;
  drmr_layer *layers;
  float* data;
} drmr_sample;

// lv2 stuff

#define DRMR_URI "http://github.com/hakber/drmr16"
#define GAIN_MIN -60.0f
#define GAIN_MAX 6.0f

typedef enum {
  DRMR_MIDI = 0,
  DRMR_LEFT0,
  DRMR_RIGHT0,
  DRMR_KITNUM,
  DRMR_BASENOTE,
  DRMR_GAIN_ONE,
  DRMR_GAIN_TWO,
  DRMR_GAIN_THREE,
  DRMR_GAIN_FOUR,
  DRMR_GAIN_FIVE,
  DRMR_GAIN_SIX,
  DRMR_GAIN_SEVEN,
  DRMR_GAIN_EIGHT,
  DRMR_GAIN_NINE,
  DRMR_GAIN_TEN,
  DRMR_GAIN_ELEVEN,
  DRMR_GAIN_TWELVE,
  DRMR_GAIN_THIRTEEN,
  DRMR_GAIN_FOURTEEN,
  DRMR_GAIN_FIFTEEN,
  DRMR_GAIN_SIXTEEN,
  DRMR_GAIN_SEVENTEEN,
  DRMR_GAIN_EIGHTEEN,
  DRMR_GAIN_NINETEEN,
  DRMR_GAIN_TWENTY,
  DRMR_GAIN_TWENTYONE,
  DRMR_GAIN_TWENTYTWO,
  DRMR_GAIN_TWENTYTHREE,
  DRMR_GAIN_TWENTYFOUR,
  DRMR_GAIN_TWENTYFIVE,
  DRMR_GAIN_TWENTYSIX,
  DRMR_GAIN_TWENTYSEVEN,
  DRMR_GAIN_TWENTYEIGHT,
  DRMR_GAIN_TWENTYNINE,
  DRMR_GAIN_THIRTY,
  DRMR_GAIN_THIRTYONE,
  DRMR_GAIN_THIRTYTWO,
  DRMR_PAN_ONE,
  DRMR_PAN_TWO,
  DRMR_PAN_THREE,
  DRMR_PAN_FOUR,
  DRMR_PAN_FIVE,
  DRMR_PAN_SIX,
  DRMR_PAN_SEVEN,
  DRMR_PAN_EIGHT,
  DRMR_PAN_NINE,
  DRMR_PAN_TEN,
  DRMR_PAN_ELEVEN,
  DRMR_PAN_TWELVE,
  DRMR_PAN_THIRTEEN,
  DRMR_PAN_FOURTEEN,
  DRMR_PAN_FIFTEEN,
  DRMR_PAN_SIXTEEN,
  DRMR_PAN_SEVENTEEN,
  DRMR_PAN_EIGHTEEN,
  DRMR_PAN_NINETEEN,
  DRMR_PAN_TWENTY,
  DRMR_PAN_TWENTYONE,
  DRMR_PAN_TWENTYTWO,
  DRMR_PAN_TWENTYTHREE,
  DRMR_PAN_TWENTYFOUR,
  DRMR_PAN_TWENTYFIVE,
  DRMR_PAN_TWENTYSIX,
  DRMR_PAN_TWENTYSEVEN,
  DRMR_PAN_TWENTYEIGHT,
  DRMR_PAN_TWENTYNINE,
  DRMR_PAN_THIRTY,
  DRMR_PAN_THIRTYONE,
  DRMR_PAN_THIRTYTWO,
  DRMR_IGNORE_VELOCITY,
  DRMR_IGNORE_NOTE_OFF,
  DRMR_LEFT1,
  DRMR_RIGHT1,
  DRMR_LEFT2,
  DRMR_RIGHT2,
  DRMR_LEFT3,
  DRMR_RIGHT3,
  DRMR_LEFT4,
  DRMR_RIGHT4,
  DRMR_LEFT5,
  DRMR_RIGHT5,
  DRMR_LEFT6,
  DRMR_RIGHT6,
  DRMR_LEFT7,
  DRMR_RIGHT7,
  DRMR_LEFT8,
  DRMR_RIGHT8,
  DRMR_LEFT9,
  DRMR_RIGHT9,
  DRMR_LEFT10,
  DRMR_RIGHT10,
  DRMR_LEFT11,
  DRMR_RIGHT11,
  DRMR_LEFT12,
  DRMR_RIGHT12,
  DRMR_LEFT13,
  DRMR_RIGHT13,
  DRMR_LEFT14,
  DRMR_RIGHT14,
  DRMR_LEFT15,
  DRMR_RIGHT15,
  DRMR_NUM_PORTS
} DrMrPortIndex;

typedef struct {
  // Ports
  float* left0;
  float* right0;
  float* left1;
  float* right1;
  float* left2;
  float* right2;
  float* left3;
  float* right3;
  float* left4;
  float* right4;
  float* left5;
  float* right5;
  float* left6;
  float* right6;
  float* left7;
  float* right7;
  float* left8;
  float* right8;
  float* left9;
  float* right9;
  float* left10;
  float* right10;
  float* left11;
  float* right11;
  float* left12;
  float* right12;
  float* left13;
  float* right13;
  float* left14;
  float* right14;
  float* left15;
  float* right15;
  
  LV2_Event_Buffer *midi_port;

  // params
  float** gains;
  float** pans;
  float* kitReq;
  float* baseNote;
  float* ignore_velocity;
  float* ignore_note_off;
  double rate;

  // URIs
  LV2_URI_Map_Feature* map;
  struct {
    uint32_t midi_event;
  } uris;

  // Available kits
  kits* kits;
  int curKit;

  // Samples
  drmr_sample* samples;
  uint8_t num_samples;

  // loading thread stuff
  pthread_mutex_t load_mutex;
  pthread_cond_t  load_cond;
  pthread_t load_thread;

} DrMr;


#endif // DRMR_H
