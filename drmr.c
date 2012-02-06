/*
  LV2 DrMr plugin
  Copyright 2012 Nick Lanham <nick@afternight.org>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "drmr.h"

int load_sample(char* path, drmr_sample* samp) {
  SNDFILE* sndf;
  int size;

  printf("Loading: %s\n",path);

  samp->active = 0;

  memset(&(samp->info),0,sizeof(SF_INFO));
  sndf = sf_open(path,SFM_READ,&(samp->info));
  
  if (!sndf) {
    fprintf(stderr,"Failed to open sound file: %s\n",sf_strerror(sndf));
    return 1;
  }

  if (samp->info.channels > 2) {
    fprintf(stderr, "File has too many channels.  Can only handle mono/stereo samples\n");
    return 1;
  }
  size = samp->info.frames * samp->info.channels;
  samp->limit = size;
  samp->data = malloc(size*sizeof(float));
  if (!samp->data) {
    fprintf(stderr,"Failed to allocate sample memory for %s\n",path);
    return 1;
  }

  sf_read_float(sndf,samp->data,size);
  sf_close(sndf); 
  return 0;
}

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features) {
  int i;
  DrMr* drmr = malloc(sizeof(DrMr));
  drmr->map = NULL;
  drmr->num_samples = 0;

  // Map midi uri
  while(*features) {
    if (!strcmp((*features)->URI, LV2_URI_MAP_URI)) {
      drmr->map = (LV2_URI_Map_Feature *)((*features)->data);
      drmr->uris.midi_event = drmr->map->uri_to_id
	(drmr->map->callback_data,
	 "http://lv2plug.in/ns/ext/event",
	 "http://lv2plug.in/ns/ext/midi#MidiEvent");
    }
    features++;
  }
  if (!drmr->map) {
    fprintf(stderr, "LV2 host does not support urid:map.\n");
    free(drmr);
    return 0;
  }

  load_hydrogen_kit(drmr,"/usr/share/hydrogen/data/drumkits/GMkit/");
  //load_hydrogen_kit(drmr,"/usr/share/hydrogen/data/drumkits/3355606kit/");

  return (LV2_Handle)drmr;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data) {
  DrMr* drmr = (DrMr*)instance;
  switch ((DrMrPortIndex)port) {
  case DRMR_MIDI:
    drmr->midi_port = (LV2_Event_Buffer*)data;
    break;
  case DRMR_LEFT:
    drmr->left = (float*)data;
    break;
  case DRMR_RIGHT:
    drmr->right = (float*)data;
    break;
  default:
    break;
  }
}

static void activate(LV2_Handle instance) { }


static void run(LV2_Handle instance, uint32_t n_samples) {
  int i;
  char first_active, one_active;
  DrMr* drmr = (DrMr*)instance;

  LV2_Event_Iterator eit;
  if (lv2_event_begin(&eit,drmr->midi_port)) { // if we have any events
    LV2_Event *cur_ev;
    uint8_t* data;
    while (lv2_event_is_valid(&eit)) {
      cur_ev = lv2_event_get(&eit,&data);
      if (cur_ev->type == drmr->uris.midi_event) {
	int channel = *data & 15;
	switch ((*data) >> 4) {
	case 8:  // ignore note-offs for now, should probably be a setting
	  //if (drmr->cur_samp) drmr->cur_samp->active = 0;
	  break;
	case 9: {
	  uint8_t nn = data[1];
	  nn-=60; // middle c is our root note (setting?)
	  if (nn >= 0 && nn < drmr->num_samples) {
	    drmr->samples[nn].active = 1;
	    drmr->samples[nn].offset = 0;
	  }
	  break;
	}
	default:
	  printf("Unhandeled status: %i\n",(*data)>>4);
	}
      }
      lv2_event_increment(&eit);
    }
  }

  first_active = 1;
  for (i = 0;i < drmr->num_samples;i++) {
    int pos,lim;
    drmr_sample* cs = drmr->samples+i;
    if (cs->active) {
      one_active = 1;
      if (cs->info.channels == 1) { // play mono sample
	lim = (n_samples < (cs->limit - cs->offset)?n_samples:(cs->limit-cs->offset));
	if (first_active) {
	  for(pos = 0;pos < lim;pos++) {
	    drmr->left[pos]  = cs->data[cs->offset];
	    drmr->right[pos] = cs->data[cs->offset];
	    cs->offset++;
	  }
	  first_active = 0;
	} else {
	  for(pos = 0;pos < lim;pos++) {
	    drmr->left[pos]  += cs->data[cs->offset];
	    drmr->right[pos] += cs->data[cs->offset];
	    cs->offset++;
	  }
	}
      } else { // play stereo sample
	lim = (cs->limit-cs->offset)/cs->info.channels;
	if (lim > n_samples) lim = n_samples;
	if (first_active) {
	  for (pos=0;pos<lim;pos++) {
	    drmr->left[pos]  = cs->data[cs->offset++];
	    drmr->right[pos] = cs->data[cs->offset++];
	  }
	  first_active = 0;
	} else {
	  for (pos=0;pos<lim;pos++) {
	    drmr->left[pos]  += cs->data[cs->offset++];
	    drmr->right[pos] += cs->data[cs->offset++];
	  }
	}
      }
      if (cs->offset >= cs->limit) cs->active = 0;
    }
  }
  if (first_active) { // didn't find any samples
    int pos;
    for(pos = 0;pos<n_samples;pos++) {
      drmr->left[pos] = 0.0f;
      drmr->right[pos] = 0.0f;
    }
  }
}

static void deactivate(LV2_Handle instance) {}

static void cleanup(LV2_Handle instance) {
  free(instance);
}

const void* extension_data(const char* uri) {
  return NULL;
}

static const LV2_Descriptor descriptor = {
  DRMR_URI,
  instantiate,
  connect_port,
  activate,
  run,
  deactivate,
  cleanup,
  extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &descriptor;
  default:
    return NULL;
  }
}