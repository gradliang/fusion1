/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      parse_es.h
*
* Programmer:    Deming Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Deming Li    first file
****************************************************************
*/
#ifndef	__PARSE_ES_H
#define	__PARSE_ES_H

#define MAX_VIDEO_PACKET_SIZE (224*1024+4)
#define VIDEOBUFFER_SIZE 0x100000

// extern unsigned char videobuffer[MAX_VIDEO_PACKET_SIZE];
extern unsigned char* videobuffer;
extern unsigned char* videobuffer0;
extern unsigned char* videobuffer1;
extern int videobuf_len;
extern unsigned char videobuf_code[4];
extern int videobuf_code_len;

// sync video stream, and returns next packet code
int sync_video_packet(demux_stream_t *ds);

// return: packet length
int read_video_packet(demux_stream_t *ds);

// return: next packet code
int skip_video_packet(demux_stream_t *ds);

#endif

