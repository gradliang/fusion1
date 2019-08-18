

#ifndef MPLAYER_DEMUX_MPG_H
#define MPLAYER_DEMUX_MPG_H

extern off_t ps_probe;

#include "demux_types.h"
typedef struct
{
	float last_pts;
	float final_pts;
	float last_vpts;
	float last2_vpts;
	float nextvframe_vpts;
	float curvframe_vpts;
	float first_apts;
	int find_first_apts;
	unsigned int *es_map;	//es map of stream types (associated to the pes id) from 0xb0 to 0xef
	unsigned int no_pts_after_vframes;
	unsigned int has_vpacket_header;
	int seeking;
	int no_vpts;
	int find_pts;	//find the next pts command
	int num_a_streams;
	int a_stream_ids[MAX_A_STREAMS];
} mpg_demuxer_t;

#endif /* MPLAYER_DEMUX_MPG_H */

