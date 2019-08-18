
/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      config_codec.h
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

#ifndef __CONFIG_CODEC_H
#define __CONFIG_CODEC_H

#include "global612.h"
/*
*******************************************************************************
*        Video decoder's choosing
*******************************************************************************
*/

#if (CHIP_VER_MSB == CHIP_VER_650)
#define HANTRO_MPEG12_CODEC	VCODEC_MPEG12_ENABLE
#define HANTRO_MPEG4_CODEC	VCODEC_MPEG4_ENABLE
#define HANTRO_H264_CODEC	VCODEC_H264_ENABLE
#else
#define SW_MPEG1_CODEC
#define SW_DIVX3_CODEC
#define SW_XVID_CODEC
#endif
#define HW_MJPG_CODEC


#define HW_MPEG_ACCE
#ifdef HW_MPEG_ACCE
	//softwre function TransferImg420To422
	//#define SW_CONVERT
	//#define DRAW_LINE

	//if image big than LCD display size, drop the pixel out of range. 
	//#define DROP_EDGE_IMAGE

	#define HW
	#define REG_EN
	#define NO_STATUS_DEP
#endif


/*
*******************************************************************************
*        Audio decoder's choosing
*******************************************************************************
*/

//#if (((CHIP_VER & 0xffff0000) == CHIP_VER_615) || ((CHIP_VER & 0xffff0000) == CHIP_VER_612))
//#define HW_MP3_CODEC
//#define HW_AAC_CODEC
//#endif

/*
*******************************************************************************
*        Video decoder's performance testing
*******************************************************************************
*/
//display decode time of 1 frame
//#define VIDEO_DECODE_TIME
//#define TOTAL_DECODE_TIME
//display decode time of parts of one frame 
//#define TIMER_DEBUG
#ifdef VIDEO_DECODE_TIME
	#undef TIMER_DEBUG
#endif

#ifdef TIMER_DEBUG
	//Time for decode MB 
	//#define VLC_DECODE
	#ifndef VLC_DECODE 
		//Time for decode coe data in MB(Inter,Intra) 
		//#define COE_DECODE	
		//Time for decode DC data in Intra MB 
		//#define DC_DECODE
	#endif
#endif

//#define BIG_TABLE
#define SRAM_OPT


/*
*******************************************************************************
*        Macroes for other purposes
*******************************************************************************
*/
//#define VIDEO_PLAYBACK
//#define DISPLAY_POINT_LEFT_TOP
#define NO_IMG_COPY

#include "mp.h"

/*
*******************************************************************************
*        Static functions
*******************************************************************************
*/
static int get_align_mpv_width(int mb_width);

static int get_align_mpv_width(int mb_width){
int pos=32;

	if(mb_width>48 || (mb_width==0))//mpv only support image width <=768
	return mb_width;


	if (mb_width>0 && mb_width<=2)
		return 2;
	else if (mb_width>2 && mb_width<=4)
		return 4;
	else if (mb_width>4 && mb_width<=8)
		return 8;
	else if (mb_width>8 && mb_width<=10)
		return 10;
	else if (mb_width==11)
		return 11;
	else if (mb_width>11 && mb_width<=16)
		return 16;
	else if (mb_width>16 && mb_width<=20)
		return 20;
	else if (mb_width>20 && mb_width<=22)
		return 22;
	else if (mb_width>22 && mb_width<=32)
		return 32;
	else if (mb_width>32 && mb_width<=40)
		return 40;
	else if (mb_width>40 && mb_width<=64)
		return 64;
}


#endif



