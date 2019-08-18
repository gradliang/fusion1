/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mpeg_hdr.h
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
#ifndef	__MPEG_HDR_H
#define	__MPEG_HDR_H

typedef struct {
    // video info:
    int mpeg1; // 0=mpeg2  1=mpeg1
    int display_picture_width;
    int display_picture_height;
    int aspect_ratio_information;
    int frame_rate_code;
    int fps; // fps*10000
    int frame_rate_extension_n;
    int frame_rate_extension_d;
    int bitrate; // 0x3FFFF==VBR
    // timing:
    int picture_structure;
    int progressive_sequence;
    int repeat_first_field;
    int progressive_frame;
    int top_field_first;
    int display_time; // secs*100
    //the following are for mpeg4
    unsigned int timeinc_resolution, timeinc_bits, timeinc_unit;
    int picture_type;
} mp_mpeg_header_t;

// MPEG demuxer private data members
#define	MPEG_PACK_START_CODE				0x1BA
#define	MPEG_PACK_START_CODE_FIX			0x52494646

#define	MPEG_AUDIO_PACKET_MIN_SID			0x1C0
#define	MPEG_AUDIO_PACKET_MAX_SID			0x1DF
#define	MPEG_VIDEO_PACKET_MIN_SID			0x1E0
#define	MPEG_VIDEO_PACKET_MAX_SID			0x1EF

#define	MPEG_DATA_USER_DATA_START_CODE		0x1B2
#define	MPEG_DATA_SEQUENCE_START_CODE		0x1B3
#define	MPEG_DATA_EXTENSION_START_CODE		0x1B5
#define	MPEG_DATA_GOP_START_CODE			0x1B8
#define	MPEG_DATA_PICTURE_START_CODE		0x100
#define	MPEG_SYS_SYSTEM_HEADER_START_CODE	0x1BB
#define	MPEG_SYS_MAP_STREAM_ID				0x1BC
#define	MPEG_SYS_DIRECTORY_STREAM_ID		0x1FF
#define	MPEG_SYS_PRIVATE_STREAM_1			0x1BD
#define	MPEG_SYS_PADDING_STREAM				0x1BE
#define	MPEG_SYS_PRIVATE_STREAM_2			0X1BF

#define	MPEG4_GROUP_OF_VOP_START_CODE			0x1B3
#define	MPEG4_VISUAL_OBJECT_SEQUENCE_START_CODE	0x1B0
#define	MPEG4_VISUAL_OBJECT_SEQUENCE_END_CODE	0x1B1
#define	MPEG4_VOP_START_CODE					0x1B6



int mp_header_process_sequence_header (mp_mpeg_header_t * const picture, const unsigned char * const buffer);
int mp_header_process_extension (mp_mpeg_header_t * const picture, const unsigned char * const buffer);
float mpeg12_aspect_info(const mp_mpeg_header_t * const picture);

#endif

