/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux.h
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

#ifndef __DEMUXER_H
#define __DEMUXER_H 

#include "demux_types.h"

demuxer_t* new_base_demux();
void free_demuxer(demuxer_t * const demuxer);
int  demux_seek(demuxer_t * const demuxer,const float rel_seek_secs,const int flags);

int demux_control(demuxer_t *demuxer, int cmd, void *arg);
unsigned long demuxer_get_time_length(demuxer_t *demuxer);
int demuxer_get_percent_pos(demuxer_t *demuxer);
int demux_read_data(demux_stream_t *ds,unsigned char* mem,int len);
int demux_pattern_3(demux_stream_t *ds, unsigned char *mem, int maxlen,
                    int *read, uint32_t pattern);

int  set_audio_parameters(demuxer_t*);

#ifdef HAVE_BUILTIN_EXPECT
#define likely(x) __builtin_expect ((x) != 0, 1)
#define unlikely(x) __builtin_expect ((x) != 0, 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define demux_peekc(ds) (\
     (likely(ds->buffer_pos<ds->buffer_size)) ? ds->buffer[ds->buffer_pos] \
     :((unlikely(!ds_fill_buffer(ds)))? (-1) : ds->buffer[ds->buffer_pos] ) )

#endif

