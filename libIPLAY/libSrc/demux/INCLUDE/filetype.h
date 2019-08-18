/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      filetype.h
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

#ifndef __FILE_TYPE__
#define __FILE_TYPE__

//make video ahead, followed by audio
typedef enum FILE_TYPE_ {
	FILE_TYPE_AVI,
	FILE_TYPE_MOV,
	FILE_TYPE_MPEG_PS,
	FILE_TYPE_MPEG_TS,
	FILE_TYPE_FLV,
	FILE_TYPE_ASF,
	FILE_TYPE_MKV,
	FILE_TYPE_h264,
	FILE_TYPE_h263,

	FILE_TYPE_AUDIO,
	FILE_TYPE_RAM,
	FILE_TYPE_RM,
	FILE_TYPE_RA,
	FILE_TYPE_MP3,
	FILE_TYPE_WMA,
	FILE_TYPE_AC3,
	FILE_TYPE_WAV,
	FILE_TYPE_M4A,
	FILE_TYPE_AAC,
	FILE_TYPE_OGG,
	FILE_TYPE_AMR,
	FILE_TYPE_UNKNOWN
}  FILE_TYPE_T;
#endif

