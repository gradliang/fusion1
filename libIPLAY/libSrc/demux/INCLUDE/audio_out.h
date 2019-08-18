/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      audio_out.h
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/

#ifndef __AUDIO_OUT__
#define __AUDIO_OUT__ 

#include "..\..\audio\include\audio.h"

/* interface towards mplayer and */
typedef struct ao_data_s
{
  int samplerate;
  int channels;
  unsigned int fourcc;
  int format;
  int bps;
  int outburst;
  int buffersize;
  int pts;
} ao_data_t;


/* global data used by mplayer and plugins */
typedef struct audio_out
{
  ao_data_t       ao_data;
  //ao_functions_t  audio_out;
  AUDIO_DEVICE audio_dev;
} audio_out_t;

extern audio_out_t* init_best_audio_out(void* sh_audio);

#endif

