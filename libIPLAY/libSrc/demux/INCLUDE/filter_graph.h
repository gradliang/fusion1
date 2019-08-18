/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      filter_graph.h
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

#ifndef __FILTER_GRAPH__
#define __FILTER_GRAPH__


#include "demux_types.h"
#include "demux_stream.h"
#include "filetype.h"

#include "audio_decoder.h"
#include "video_decoder.h"
#include "audio_out.h"
#include "video_out.h"


typedef struct filter_graph {
  demuxer_t* demux;
  video_decoder_t* v_decoder;
  audio_decoder_t* a_decoder;
  video_out_t*     v_out;
  audio_out_t*     a_out;
} filter_graph_t;


extern filter_graph_t filter_graph;
extern int build_filter_graph_AV(stream_t*, const FILE_TYPE_T, int * const err_code);
//extern int build_filter_graph_get_audio_info(stream_t* stream, FILE_TYPE_T file_type, DWORD *err_code);

extern int get_vdisplay_width();
extern int get_vdisplay_height();
extern FILE_TYPE_T get_type_by_extension(unsigned char*,int size);

#endif


