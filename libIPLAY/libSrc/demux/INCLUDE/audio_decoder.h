/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      audio_decoder.h
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

#ifndef __AUDIO_DECODER_H
#define __AUDIO_DECODER_H

#include "mpc_info.h"


#define MP3_50		0x50  
#define MP3_55		0X55 
#define MP3_CVBR	0x33706d2e						//".mp3" CBR/VBR MP3 (MOV files)  
#define MP3_FCC		0x5500736d						//"ms\0\x55" older mp3 fcc (MOV files)
#define AMR_NB		0x726d6173
#define PCM_1		0x1
#define PCM_RAW		0x20776172						//"raw " (MOV files)
#define PCM_TWOS	0x736f7774						// "twos" (MOV files)  
#define PCM_SOWT	0x74776f73						// "sowt" (MOV files)
#define WMA_T		0x160							//WMA:160-162
#define OGG_T		FOURCC('f','o', 'g', 'g')		//0x161//OGG
#define mp4a		FOURCC('a', '4', 'p', 'm')
#define MP4A		FOURCC('A', '4', 'P', 'M')
#define ULAW		FOURCC('W', 'A', 'L', 'U')
#define ulaw		FOURCC('w', 'a', 'l', 'u')
#define ALAW		FOURCC('W', 'A', 'L', 'A')
#define alaw		FOURCC('w', 'a', 'l', 'a')
#define DVI_ADPCM	0x6d730011
#define AC3_T		0x2000
#define RAM_T		FOURCC('F', 'F', 'M', 'R')
#define ram_T		FOURCC('f', 'f', 'm', 'r')

#define MAX_OUTBURST    65536

#define EQUALIZER_EN	1
extern  BYTE *AUDIO_EQ_INFO;

typedef struct ad_functions_s
{
	ad_info_t *info;
	int (*preinit)(sh_audio_t *sh);
	int (*init)(sh_audio_t *sh);
	void (*uninit)(sh_audio_t *sh);
	int (*control)(sh_audio_t *sh,int cmd,void* arg, ...);
	int (*decode_audio)(sh_audio_t *sh,unsigned char* buffer,int minlen,int maxlen);
	int (*resync)(unsigned int second);
} ad_functions_t;


typedef struct audio_decoder {
    ad_functions_t   audio_decoder;
} audio_decoder_t;


extern audio_decoder_t* init_best_audio_codec(sh_audio_t*, STREAM*);
extern int Decode_audio(sh_audio_t *sh_audio,unsigned char *buf,int minlen,int maxlen);

// NULL terminated array of all drivers
extern ad_functions_t* mpcodecs_ad_drivers[];

// fallback
#define ADCTRL_SEEK_STREAM 0 		/* seek audio stream */

// fallback if ADCTRL_RESYNC not implemented: sh_audio->a_in_buffer_len=0;
#define ADCTRL_RESYNC_STREAM 1		/* resync, called after seeking! */

// fallback if ADCTRL_SKIP not implemented: ds_fill_buffer(sh_audio->ds);
#define ADCTRL_SKIP_FRAME 2			/* skip block/frame, called while seeking! */

// fallback if ADCTRL_QUERY_FORMAT not implemented: sh_audio->sample_format
#define ADCTRL_QUERY_FORMAT 3		/* test for availabilty of a format */

// fallback: use hw mixer in libao
#define ADCTRL_SET_VOLUME 4			/* set volume (used for mp3lib and liba52) */


#if MP3_SW_AUDIO
extern ad_functions_t software_mp3;
#elif MP3_MAD_AUDIO 
extern ad_functions_t mpcodecs_ad_libmad;
#endif
extern ad_functions_t hw_mpcodecs_ad_libmad;

#if WMA_ENABLE
extern ad_functions_t software_wma;
#endif

#if AAC_SW_AUDIO
#define SDRAM_SIZE_AAC  (300 * 1024)
extern ad_functions_t software_aac;
#endif

#if OGG_ENABLE
#define SDRAM_SIZE_OGG (300 * 1024)
extern ad_functions_t software_ogg;
#endif

#if RAM_AUDIO_ENABLE
extern ad_functions_t software_ram;
#endif

#if AAC_FAAD_AUDIO
extern ad_functions_t mpcodecs_ad_faad;
#endif

#if AC3_AUDIO_ENABLE
#define SDRAM_SIZE_AC3 (4 * 1024)	//(300*1024)

int  sdbuf_left;
extern ad_functions_t software_ac3;
#endif

#if RAM_AUDIO_ENABLE
#define SDRAM_SIZE_RAM (100 * 1024)
#endif

extern ad_functions_t software_aviwave;
extern ad_functions_t mpcodecs_ad_ffmpeg;
extern ad_functions_t software_amr;

#endif

