/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      parse_es.c
*
* Programmer:    Deming Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: MPEG-ES VIDEO PARSER
*
* Change History (most recent first):
*     <1>     03/30/2005    Deming Li    first file
****************************************************************
*/
#include "global612.h"
#if (VIDEO_ON || AUDIO_ON)

#include <stdio.h>
#include <stdlib.h>

#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux.h"
#include "parse_es.h"

unsigned char* videobuffer=NULL;
unsigned char* videobuffer0=NULL;
unsigned char* videobuffer1=NULL;

int videobuf_len=0;
static int next_nal = -1;
///! legacy variable, 4 if stream is synced, 0 if not
int videobuf_code_len=0;


#define MAX_SYNCLEN (10 * 1024 * 1024)
// sync video stream, and returns next packet code
int sync_video_packet(demux_stream_t *const ds)
{
	if (!videobuf_code_len)
	{
		int skipped=0;

		if (!demux_pattern_3(ds, NULL, MAX_SYNCLEN, &skipped, 0x100))
		{
			if (skipped == MAX_SYNCLEN)
				goto eof_out;
		}

		next_nal = demux_getc(ds);
		if (next_nal < 0)
			goto eof_out;
		videobuf_code_len = 4;
	}

	return 0x100|next_nal;

eof_out:
	next_nal = -1;
	videobuf_code_len = 0;
	return 0;
}

// return: packet length
int read_video_packet(demux_stream_t * const ds)
{
	int packet_start;
	int res, read;

	if (VIDEOBUFFER_SIZE - videobuf_len < 5)
		return 0;
	// SYNC STREAM
//  if(!sync_video_packet(ds)) return 0; // cannot sync (EOF)

	// COPY STARTCODE:
	packet_start=videobuf_len;
	videobuffer[videobuf_len+0]=0;
	videobuffer[videobuf_len+1]=0;
	videobuffer[videobuf_len+2]=1;
	videobuffer[videobuf_len+3]=next_nal;
	videobuf_len+=4;

	// READ PACKET:
	res = demux_pattern_3(ds, &videobuffer[videobuf_len],
	                      VIDEOBUFFER_SIZE - videobuf_len, &read, 0x100);
	videobuf_len += read;
	if (!res)
		goto eof_out;

	videobuf_len-=3;


	// Save next packet code:
	next_nal = demux_getc(ds);
	if (next_nal < 0)
		goto eof_out;
	videobuf_code_len=4;

	return videobuf_len-packet_start;

eof_out:
	next_nal = -1;
	videobuf_code_len = 0;
	return videobuf_len - packet_start;
}

// return: next packet code
int skip_video_packet(demux_stream_t *const ds)
{
	// SYNC STREAM
	//  if(!sync_video_packet(ds)) return 0; // cannot sync (EOF)

	videobuf_code_len = 0; // force resync

	// SYNC AGAIN:
	return sync_video_packet(ds);
}
#endif
