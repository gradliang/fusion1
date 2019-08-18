/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      filter_graph.c
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

#include "global612.h"
#if (VIDEO_ON || AUDIO_ON)
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/

#include "mpTrace.h"
#include "filter_graph.h"
#include "audio_decoder.h"


extern enum AO_AV_TYPE AO_AV;

//#if (WMA_ENABLE||OGG_ENABLE||AC3_AUDIO_ENABLE||RAM_AUDIO_ENABLE)
//typedef struct da_priv
//{
//	int frmt;
//	float last_pts;
//} da_priv_t;
//#endif

///
///@defgroup group2_FilterG  Filter Graph Manager 
///
#pragma alignvar(4)
filter_graph_t filter_graph;
int  audio_samplerate=0;
FILE_TYPE_T get_type_by_extension(unsigned char *extension, int size)
{
	const struct {
        char        *extension;
        int         size;
        FILE_TYPE_T type;
	} extensions_table[] = {
	#if (MP3_SW_AUDIO | MP3_MAD_AUDIO | MP3_HW_CODEC)
			{ "MP3", 3, FILE_TYPE_MP3 },
	#endif		
	#if WMA_ENABLE
			{ "WMA", 3, FILE_TYPE_WMA },
	#endif
	#if WAV_ENABLE
			{ "WAV", 3, FILE_TYPE_WAV},
	#endif
	#if OGG_ENABLE
			{ "OGG", 3, FILE_TYPE_OGG },
	#endif
	#if (AAC_SW_AUDIO || AAC_FAAD_AUDIO)
			{ "M4A", 3, FILE_TYPE_M4A},
			{ "AAC", 3, FILE_TYPE_AAC },
	#endif
	#if RAM_AUDIO_ENABLE 
			{ "RA ", 3, FILE_TYPE_RA},
			{ "RM ", 3, FILE_TYPE_RM},
			{ "RAM", 3, FILE_TYPE_RAM},
	#endif
	#if AC3_AUDIO_ENABLE
			{ "AC3", 3, FILE_TYPE_AC3},
	#endif
	#if AMR_ENABLE
			{ "AMR", 3, FILE_TYPE_AMR},
	#endif
	#if (MJPEG_ENABLE || VIDEO_ENABLE)
			{ "AVI", 3, FILE_TYPE_AVI },
			{ "MOV", 3, FILE_TYPE_MOV },
	#endif
	#if VIDEO_ENABLE
			{ "mpeg",4, FILE_TYPE_MPEG_PS },
			{ "MPG", 3, FILE_TYPE_MPEG_PS },
			{ "MPE", 3, FILE_TYPE_MPEG_PS },
			{ "DAT", 3, FILE_TYPE_MPEG_PS },
			{ "3GP", 3, FILE_TYPE_MOV },
			{ "VOB", 3, FILE_TYPE_MPEG_PS},
			{ "M2V", 3, FILE_TYPE_MPEG_PS},
			{ "MP4", 3, FILE_TYPE_MOV },
			{ "QT",  2, FILE_TYPE_MOV },
			{ "ASF", 3, FILE_TYPE_ASF },
			{ "FLV", 3, FILE_TYPE_FLV },
			{ "TS",  2, FILE_TYPE_MPEG_TS },
			{ "MTS", 3, FILE_TYPE_MPEG_TS },
			{ "M2T", 3, FILE_TYPE_MPEG_TS },
			{ "MKV", 3, FILE_TYPE_MKV },
			{ "264", 3, FILE_TYPE_h264 },
			{ "263", 3, FILE_TYPE_h263 },
			{ "FLAC",4, FILE_TYPE_AUDIO },
			{ "FLA", 3, FILE_TYPE_AUDIO },
	#endif
	};
	register int i = 0, j = 0, len = 0;
	register unsigned char c = 0;

	len = sizeof(extensions_table) / sizeof(extensions_table[0]);
	for (i = 0; i < len; i++)
	{
//		if (size != extensions_table[i].size)
//			continue;
		size = extensions_table[i].size;
		for (j = 0; j < size; j++)
		{
			c = extension[j];
			if (c >= 0x61 && c <= 0x7a)
			{	// To upper case
				c -= 0x20;
			}
			if (c != extensions_table[i].extension[j])
				break;
		}
		if (j >= size)
		{
			return extensions_table[i].type;
		}
	}

	return FILE_TYPE_UNKNOWN;
}


///
///@ingroup group2_FilterG
///@brief   This function will take charge of creating Filter Graph for the input stream which type is file_type.
///             The Filter Graph includes demux, video codec, audio codec, video out and audio out, and all these 
///         pointers are recorded in global variable filter_graph which type is structure filter_graph_t.
///
///@param   stream_t* stream
///@param   FILE_TYPE_T file_type
///
///@return  0 for Fail, 1 for Success
///

sh_audio_t sh_audio_wrapper;

BYTE video_flag = 0;
extern BYTE g_bAudioCodecInit;
extern BOOL boVideoPreview;

/**********************************************************************************************
 * build_filter_graph_AV init audio to  ---------------------  "filter_graph.demux->sh_audio" *
 *                                                                                            *
 * build_filter_graph_AO init audio to gobal variable  ------  "sh_audio_wrapper"             *
 *                                                                                            * 
 **********************************************************************************************/
int build_filter_graph_AV(stream_t * stream, const FILE_TYPE_T file_type, int * const err_code)
{
	const DEMUX_T file_to_demux[10] = 
	{
		DEMUX_AVI,
		DEMUX_MOV,
		DEMUX_MPEG_PS,
		DEMUX_MPEG_TS,
		DEMUX_FLV,
		DEMUX_ASF,
		DEMUX_MKV,
		DEMUX_h264,
		DEMUX_h263,
		DEMUX_UNKNOWN
	};
	MP_DEBUG("%s file type: %d, demux: %d", __FUNCTION__, file_type, file_to_demux[file_type]);
	video_flag = 0;

	demuxmgr_init();

	filter_graph.demux = (demuxer_t *)demuxmgr_select(stream, file_to_demux[file_type]);

	if (!filter_graph.demux)
	{
		*err_code = ERR_VIDEO_DEMUX;
		MP_ALERT("!filter_graph.demux @ %s:%d", __FILE__, __LINE__);
		return FAIL;
	}

	if (filter_graph.demux->sh_video)
	{
		if (!video_read_properties(filter_graph.demux->sh_video))
		{
			*err_code = ERR_VIDEO_CODEC;
			filter_graph.demux->sh_video = 0;
			MP_ALERT("!video_read_properties @ %s:%d", __FILE__, __LINE__);
			return FAIL;
		}
	}

	if (filter_graph.demux->sh_video_3D)
	{
		if (!video_read_properties(filter_graph.demux->sh_video_3D))
		{
			*err_code = ERR_VIDEO_CODEC;
			filter_graph.demux->sh_video_3D = 0;
			MP_ALERT("!video_read_properties 3D @ %s:%d", __FILE__, __LINE__);
			return FAIL;
		}
	}

	if (filter_graph.demux->sh_video)
	{
	    sh_video_t *sh_video = (sh_video_t *) filter_graph.demux->sh_video;
		video_flag = 1;
		#if YUV444_ENABLE
		sh_video->b_yuv444 = 1;
		#else
		sh_video->b_yuv444 = 0;
		#endif
		filter_graph.v_decoder = init_best_video_codec((sh_video_t *) filter_graph.demux->sh_video);
		if (!filter_graph.v_decoder)
		{
			video_flag = 0;
			*err_code = ERR_VIDEO_CODEC;
			MP_ALERT("!filter_graph.v_decoder @ %s:%d", __FILE__, __LINE__);
			return FAIL;
		}
	}

	if (filter_graph.demux->sh_audio)
	{
	    audio_samplerate=((sh_audio_t *) filter_graph.demux->sh_audio)->samplerate;

	    switch (file_type)
	    {
		    case FILE_TYPE_ASF :
		    case FILE_TYPE_AVI:
		    case FILE_TYPE_MOV:
		    case FILE_TYPE_MPEG_PS :
		    case FILE_TYPE_FLV :
		    case FILE_TYPE_MPEG_TS :
		    case FILE_TYPE_MKV:
			    switch( ((sh_audio_t *) filter_graph.demux->sh_audio)->format)
			    {
				    case MP3_50:
				    case MP3_55:
				    case MP3_CVBR:
				    case MP3_FCC:
					    ((sh_audio_t *) filter_graph.demux->sh_audio)->format = 0x58;
					    break;
                #if (!AAC_SW_AUDIO) || (!AAC_FAAD_AUDIO)
				    case mp4a:
				    case MP4A:
					    ((sh_audio_t *) filter_graph.demux->sh_audio)->format = 0x59;
					    break;
                #endif
			    }
	    }
	}
    if (MOVIE_STATUS_PREVIEW == g_psSystemConfig->sVideoPlayer.dwPlayerStatus)
		filter_graph.demux->sh_audio = 0;//if Preview Audio, mark this line

	if (filter_graph.demux->sh_audio)
	{
		filter_graph.a_decoder = init_best_audio_codec((sh_audio_t *) filter_graph.demux->sh_audio, stream);
		if (!filter_graph.a_decoder)
		{
			sh_audio_t * sh_audio = (sh_audio_t *) filter_graph.demux->sh_audio;
			unsigned int four_cc = sh_audio->format;

			switch (four_cc)
			{
#if ((!AAC_SW_AUDIO) && (!AAC_FAAD_AUDIO))			
				case mp4a:		
				case MP4A:
				case 0x59:
				case 0x57:
#endif
#if (!(WMA_ENABLE && WMA_AV))
				case WMA_T:
				case 0x161:
#endif					
#if (!(AC3_AUDIO_ENABLE && AC3_AV))					
				case AC3_T:
#endif					
					MP_ALERT("Can not find audio codec, filter_graph.a_decoder is null!");
					*err_code = ERR_AUDIO_CODEC;
					return FAIL;				
					break;
				default:
					MP_ALERT("It support audio decoder");
			}	
			if (filter_graph.v_decoder)    // to play video only
				filter_graph.demux->sh_audio = 0;	
			else
			
			{
				MP_ALERT("Can not find audio codec, filter_graph.a_decoder is null!");
				*err_code = ERR_AUDIO_CODEC;
				return FAIL;
			}
		}
	}

    g_bAudioCodecInit = 1;
	
	if (filter_graph.demux->sh_video)
	{
		filter_graph.v_out = init_best_video_out(filter_graph.demux->sh_video);
		if (!filter_graph.v_out)
		{
			MP_ALERT("ERR_VIDEO_OUT");
			*err_code = ERR_VIDEO_OUT;
			return FAIL;
		}
	}

	//use_more_audio_out_buffer = 0;
	if (filter_graph.demux->sh_audio)
	{
		if (filter_graph.demux->sh_video)
		{
			//use_more_audio_out_buffer = 1;
			sh_video_t * sh_video = filter_graph.demux->sh_video;
			if(sh_video->format != FLV1)
				filter_graph.demux->stream_a = (stream_t *)av_stream_create(stream);
		}

		filter_graph.a_out = init_best_audio_out(filter_graph.demux->sh_audio);
		if (!filter_graph.a_out)
		{
			MP_ALERT("Can not find audio output source, filter_graph.a_out is null!");
			*err_code = ERR_AUDIO_OUT;
			return FAIL;
		}
	}

	reset_av_player_controller(1);

	return PASS;
}

int build_filter_graph_AO(STREAM * stream, FILE_TYPE_T file_type, int *err_code)
{
	//mpDebugPrint("Build filter graph for Software stage");
	video_flag = 0;

//	Mark temporarily by C.W ------- Not used in MP620 style case
//	#if (WMA_NEW_ENABLE ||OGG_ENABLE||AC3_AUDIO_ENABLE||RAM_AUDIO_ENABLE)
//		sh_audio_t *sh_audio;
//		da_priv_t *priv;
//	#endif

	if(!stream)
	{
		filter_graph.demux = 0;
		*err_code = ERR_VIDEO_DEMUX;
		MP_ALERT("Audio stream Null");

		return FAIL;
	}

	memset((BYTE *)&sh_audio_wrapper, 0, sizeof(sh_audio_t));

	switch(file_type)
	{
		case FILE_TYPE_MP3 :
			sh_audio_wrapper.format = 0x55;
			break;
		case FILE_TYPE_WMA :
			sh_audio_wrapper.format = 0x160;
			break;
		case FILE_TYPE_WAV :
			sh_audio_wrapper.format = 0x1;
			break;
		case FILE_TYPE_AC3:
			sh_audio_wrapper.format = 0x2000;
			break;
		case FILE_TYPE_OGG :
			sh_audio_wrapper.format = FOURCC('f','o', 'g', 'g');
			break;
		case FILE_TYPE_RAM:
		case FILE_TYPE_RA:
		case FILE_TYPE_RM:
			sh_audio_wrapper.format = FOURCC('F', 'F', 'M', 'R');
			break;
		case FILE_TYPE_AAC:
		case FILE_TYPE_M4A:
			sh_audio_wrapper.format = FOURCC('A', '4', 'P', 'M');
			break;
		case FILE_TYPE_AMR:
			sh_audio_wrapper.format = 0x616d72;
			break;
		default:
			MP_ALERT("[Filter graph software] Unsupport format");
	}

	//Init audio codec
	filter_graph.a_decoder = init_best_audio_codec(&sh_audio_wrapper, stream);
	if (!filter_graph.a_decoder)
	{
		*err_code = ERR_AUDIO_CODEC;
		MP_ALERT("Init codec fail");

		return FAIL;
	}

	//Init audio out
	filter_graph.a_out = init_best_audio_out(&sh_audio_wrapper);
	if (!filter_graph.a_out)
	{
		*err_code = ERR_AUDIO_OUT;
		MP_ALERT("Init Audio out fail");

		return FAIL;
	}

	return PASS;
}

///
///@ingroup group2_FilterG
///@brief   This function will return the video display width which is recorded in field sh_video of structure demuxer_t.
///         This video display width will be used to set the structure ST_IMGWIN MovieWin.
///
///@param   No
///
///@return  0 for Fail, video display width for Success
///
int get_vdisplay_width()
{
	if (filter_graph.demux && filter_graph.demux->sh_video)
	{
		return ((sh_video_t *) filter_graph.demux->sh_video)->disp_w;
	}
	return 0;
}

///
///@ingroup group2_FilterG
///@brief   This function will return the video display height which is recorded in field sh_video of structure demuxer_t.
///         This video display height will be used to set structure ST_IMGWIN MovieWin.
///
///@param   No
///
///@return  0 for Fail, video display height for Success
///
int get_vdisplay_height()
{
	if (filter_graph.demux && filter_graph.demux->sh_video)
	{
		return ((sh_video_t *) filter_graph.demux->sh_video)->disp_h;
	}
	return 0;
}
#endif
