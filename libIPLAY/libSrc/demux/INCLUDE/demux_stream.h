/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_stream.h
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

#ifndef __DEMUX_STREAM_H
#define __DEMUX_STREAM_H 


#include "demux_types.h"

//struct demuxer_st;



// #define  demuxer_p(ds)  ((demuxer_t*)ds->demuxer)

demux_stream_t* new_demuxer_stream(int id, struct demuxer_st *demuxer);
// demux_stream_t* new_demuxer_stream(int id,void *demuxer);
void free_demuxer_stream(demux_stream_t *ds);

void  ds_free_packs(demux_stream_t *ds);
int   ds_get_packet(demux_stream_t *ds,unsigned char **start);
float ds_get_next_pts(demux_stream_t *ds);

void ds_add_packet(demux_stream_t *ds,demux_packet_t* dp);
int ds_read_packet(demux_stream_t *ds,stream_t *stream,int len,
                    float pts, unsigned int  pos, int flags);

int ds_fill_buffer(demux_stream_t *ds);

int ds_read_data(demux_stream_t *ds,unsigned char* mem,int len);


inline static unsigned int ds_tell(demux_stream_t *ds){
  return (ds->dpos-ds->buffer_size)+ds->buffer_pos;
}

inline static int ds_tell_pts(demux_stream_t *ds){
  return (ds->pts_bytes-ds->buffer_size)+ds->buffer_pos;
}

#define demux_getc(ds) (\
     (ds->buffer_pos<ds->buffer_size) ? ds->buffer[ds->buffer_pos++] \
     :((!ds_fill_buffer(ds))? (-1) : ds->buffer[ds->buffer_pos++] ) )

#endif

