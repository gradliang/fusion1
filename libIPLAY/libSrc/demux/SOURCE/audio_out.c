/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      audio_out.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: audio output
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/

#include "global612.h"
#include "demux_stream.h"
#include "demux.h"
#include "stheader.h"
#include "audio_out.h"


static audio_out_t _audio_out;

#if (AUDIO_DAC == DAC_NONE)
int audio_hw_installed = 0;
#else
int audio_hw_installed = 1;
#endif


///
///@ingroup AUDIOOUT
///@brief   Audio Out Module API setup
///
///@param   void* sh_audio Pointer to a sh_audio_t structure, which carries the auido parameter to initiallize the audio out module.
///
///@return  audio_out_t*    Pointer to the initialized audio_out_t structure .
///

///
///@ingroup group2_FilterG
///@defgroup group_Audio_Out        Audio Out
///

///@ingroup group_Audio_Out
///@brief   This function will return the pointer of initialized audio_out_t structure.
///         And this pointer will be recorded in filter_graph_t structure(filter_graph).
///         Structure audio_out_t contains some audio stream param. and all the pointers
///         of audio out functions.
///
///@param   void* sh_audio      The pointer of audio stream header structure 
///
///@return  pointer of audio_out_t structure for success, 0 for fail
///
audio_out_t *init_best_audio_out(void *sh_audio)
{
	sh_audio_t *sh_audio_p = (sh_audio_t *) sh_audio;

	//mpDebugPrint("Init best audio out!!");
	//	mpDebugPrint("[Init_best_audio_out]Sample rate: %d, Channel: %d, samplesize: %d, Format: %d",
	//					sh_audio_p->samplerate,
	//					sh_audio_p->channels,
	//					sh_audio_p->samplesize,
	//					sh_audio_p->format);

	if (sh_audio_p)
	{
		_audio_out.ao_data.samplerate = sh_audio_p->samplerate;

#ifdef LAYERII_FORCE_MONO
//	#if  Libmad_FAAD
//		if (Layer2_Inited)
//		{
//			_audio_out.ao_data.channels = 1;
//		}
//		else
//	#endif
		{
			_audio_out.ao_data.channels = sh_audio_p->channels;
		}
#else
		_audio_out.ao_data.channels		= sh_audio_p->channels;
#endif
		_audio_out.ao_data.format		= sh_audio_p->samplesize;
		_audio_out.ao_data.fourcc		= sh_audio_p->format;

		_audio_out.ao_data.bps			= 0;
		_audio_out.ao_data.outburst		= 0;
		_audio_out.ao_data.buffersize	= 0;
		_audio_out.ao_data.pts			= 0;
		#if AUDIO_ON
		if (init_audio_driver(&_audio_out.audio_dev))
			return &_audio_out;
		#endif
	}
	
	return NULL;
}

