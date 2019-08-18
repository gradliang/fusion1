/*****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mpapi.c
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
*****************************************************************/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#include "global612.h"
#if (VIDEO_ON || AUDIO_ON)

#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/

#include "mpTrace.h"
#include "display.h"
#include "avTypeDef.h"
#include "os.h"
#include "taskid.h"
#include "av.h"
#include "mpapi_inter.h"
#include "Mpapi.h"
#include <string.h>

#ifndef __API_MAIN_H
#include "api_main.h"
#endif

#if (DISPLAY_VIDEO_SUBTITLE || LYRIC_ENABLE)
#ifndef _SUB_COMMON_H_
#include "sub_common.h"
#endif
#endif

#if (LYRIC_ENABLE)
#include "Lrc.h" //For Lyrics (.lrc) files, by Eddyson
#endif

DWORD g_dwTotalSeconds;
FILE_TYPE_T g_FileType;

extern Audio_dec Media_data;
extern sh_audio_t sh_audio_wrapper;
extern enum AO_AV_TYPE AO_AV;

extern BYTE bFileType;
extern int decoder_initialized;
extern int frame_num;
extern XPG_FUNC_PTR* g_pXpgFuncPtr;
extern S_BK_STREAM_NODE_T* g_BkStreamBufHead;
extern S_BK_STREAM_NODE_T* g_BkStreamBufTail;
BYTE g_bVideoResume = 0;

///
///@mainpage Movie Player API User Guild
///
///This is an introduction of the Movie Player API.
///
ST_IMGWIN		*pdwScreenWin;
ST_IMGWIN		*pdwMovieWin;
extern DWORD	 dwOsUserBlkSize;
extern int		 Fileflag;
extern float	 StepSeconds;
extern DWORD	 g_bAniFlag;
extern int		 MJPEG;
extern DWORD     backward_flag;
BYTE             g_bBypassToDma = 0;
extern BYTE g_bPlayFirstVideoFrame;
extern DWORD	Audio_FB_Seconds;
extern filter_graph_t filter_graph;
extern BOOL boVideoPreview;
extern BYTE g_bPlayMode;
extern STREAM *g_sHandle;

#if DISPLAY_VIDEO_SUBTITLE
extern S_SUB_NODE_T* g_SubNodeHead;
extern S_SUB_NODE_T* g_SubNodeTail;
extern S_SUB_NODE_T* g_SubNodeCur;
#endif

#if LYRIC_ENABLE
	extern PLRC_TIMETAG lrc_timetags; //Lyrics array by Eddyson
	extern int cur_timetag, total_timetag; //Current line of lrc file, total number of line to print
 	extern WORD x_lrc, y_lrc; //Display position of lyrics
	extern int line_dist, cur_line, total_line;
#endif

BYTE g_change_to_thumb_mode = 0;
BYTE g_change_to_thumb_initialized = 0;

void MoviePlayerDispTime (BYTE blRedraw);

///
///@defgroup        group_mp_api    Movie Player API
///A movie player can be created with these API.
///


///
///ingroup  group_mp_api
///defgroup group_inter_func   Functions used by API
///

///
///@ingroup group_mp_api
///@defgroup group_exter_func   API Functions for external use
///


///
///@ingroup group_exter_func
///@brief   This function is used to create and startup the MoviePlayTask, which will handle all the instructions.
///
///@param   No
///
///@return  No
///
extern void VideoDecodeTask(void);
extern void VideoDataTask(void);
extern void AudioDecodeTask(void);

//#if  Libmad_FAAD
//extern void         save_mpa_regs ();
//#endif


void MovieTaskCreate ()
{
//    #if  Libmad_FAAD
//    save_mpa_regs ();
//    #endif

    SemaphoreCreate(VIDEO_SEMA_ID, OS_ATTR_PRIORITY, 2);
	SemaphoreCreate(AV_CMD_SEMA_ID, OS_ATTR_PRIORITY, 2);
    EventCreate(MOVIE_EVENT, OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR, 0);
    EventCreate(MOVIE_STATUS, OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE, EVENT_MOVIE_STATUS_NOT_START_YET);	
    EventCreate(AV_DECODER_EVENT, OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE, EVENT_V_NOT_DECODING|EVENT_A_NOT_DECODING);
	EventCreate(AVP_SYNC_EVENT, OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE, 0);
    EventCreate(VIDEO_DATA_EVENT, OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR, 0);

    TaskCreate (AV_CONTROL_TASK, MovieTask_Main, CONTROL_PRIORITY, 0x8000);
    TaskStartup (AV_CONTROL_TASK);

	//VideoDecodeTask and AudioDecodeTask are used in Video playing case only.
    TaskCreate (VIDEO_DECODE_TASK, VideoDecodeTask, CONTROL_PRIORITY, 0x4000);
    TaskStartup (VIDEO_DECODE_TASK);
	TaskCreate (VIDEO_DATA_TASK, VideoDataTask, CONTROL_PRIORITY, 0x4000);
    TaskStartup (VIDEO_DATA_TASK);
#if (PURE_VIDEO == 0)
    TaskCreate (AUDIO_DECODE_TASK, AudioDecodeTask, CONTROL_PRIORITY, 0x4000);
    TaskStartup (AUDIO_DECODE_TASK);
#endif

    SemaphoreCreate(STREAM_READ_SEMA_ID, OS_ATTR_PRIORITY, 1);
}


///@ingroup group_inter_func
///@brief   This function is used to seek the file position according the input parameters.
///         This function will call the proper Demux's seek function to implement the file seeking.
///
///@param   float pts       The file position to seek.
///@param   int flags       The seeking flag
///
///         flags = 1;      Absolute seek to a specific timestamp in seconds
///
///         flags = 2;      Relatvie seek by percentage
///
///         flags = 3;      Absolute seek by percentage
///
///@return  0 for fail, 1 for success

BYTE seek_completed = 0;
extern DWORD audiostream_finish;
int MovieSeek (float pts, int flags)
{
	DWORD release;
	MP_DEBUG("MovieSeek(,%d)",flags);
//	EventPolling(AV_DECODER_EVENT, ~0, OS_EVENT_OR, &release);
//	mpDebugPrint("MovieSeek AV_DECODER_EVENT = 0x%x", release);

	if (EventPolling(AV_DECODER_EVENT, EVENT_V_DECODER_EOF|EVENT_A_DECODER_EOF, OS_EVENT_OR, &release)==OS_STATUS_OK)
		return 0;
	if (EventPolling(AVP_SYNC_EVENT, EVENT_AV_EOF, OS_EVENT_OR, &release)==OS_STATUS_OK)
		return 0;

    if (pts || flags)
	{
		demuxer_t *cur_demux = filter_graph.demux;

		EventWait(AV_DECODER_EVENT, EVENT_V_NOT_DECODING|EVENT_A_NOT_DECODING, OS_EVENT_AND, &release);
		audiostream_finish = 0;
		if (demux_seek (cur_demux, pts, flags))
		{
			if (cur_demux->sh_video)
				((sh_video_t *) cur_demux->sh_video)->pts = cur_demux->video->pts;

			if (cur_demux->sh_audio)
			{
				if (filter_graph.a_out)
				{
					filter_graph.a_out->audio_dev.pause ();
					filter_graph.a_out->audio_dev.ClearAudioBuffer ();
					filter_graph.a_out->audio_dev.StopDMA();
				}
			}

			seek_completed = 1;
			return 1;
		}
		else
		{
			UartOutText("MovieSeek(): demux_seek fail\r\n");
		}
	}

    seek_completed = 1;

    return 0;
}

///
///@ingroup group_exter_func
///@brief   This function will build the filter graph, including demux, video/audio decoder,video/audio out filters.
///         And initialize the movie display window, create and startup the MoviePlayTask to handle input instructions.
///
///@param   DRIVE *drv          This structure includes info about storage device, file system, and the file for display.
///@param   ST_IMGWIN *win      The image display window
///
///@return  No
///
static BOOL         first = 1;

BYTE                full_screen_mode = 1;
BYTE                video_thumb_mode = 0;
DWORD               pre_parse = 0;

#if ANTI_TEARING_ENABLE
BYTE                frame_buffer_index = 0;
BOOL                buffer_switch = 0;
BOOL                video_stop = 0;
#endif

char               *g_sLyric = NULL;
static DWORD        Last_Time = -1;


/**
 * 	@ This function is used to judge Audio/Video media file format and initiate it
 *
 * 	@ retval PASS success
 */
int EnterMoviePlayer (STREAM * sHandle, STREAM * sLyricHandle, unsigned char *extension, ST_IMGWIN * screen_win, ST_IMGWIN * movie_win, int subtitle_sync_align)
{
	SWORD swRet;
	Media_Info byInfoBuf;

	MP_DEBUG("EnterMoviePlayer");
#if AUDIO_ON
	AudioSetPlayPTS(0);
#endif
	//decoder_initialized = 1;
	video_thumb_mode = 0;
	pre_parse = 0;			// Relate to hardware mp3 decoder.....fade out soon!!!!!!!!!!!
	Last_Time = -1;
	g_BkStreamBufHead = NULL;
	g_BkStreamBufTail = NULL;

	g_FileType = get_type_by_extension (extension, 3);

	#if DISPLAY_VIDEO_SUBTITLE
    BOOL video_file_type = (FILE_TYPE_AVI     == g_FileType) ||
                           (FILE_TYPE_ASF     == g_FileType) ||
		                   (FILE_TYPE_MOV     == g_FileType) ||
		                   (FILE_TYPE_MPEG_PS == g_FileType) ||
		                   (FILE_TYPE_MPEG_TS == g_FileType) ||
		                   (FILE_TYPE_FLV     == g_FileType) ||
		                   (FILE_TYPE_MKV     == g_FileType);

    if ((sLyricHandle != NULL) && video_file_type)
    {
        if (sub_read_file(sLyricHandle, subtitle_sync_align) == FAIL)
			return FAIL;
    }
    #endif

	swRet = MovieTask_Init(sHandle, g_FileType, screen_win, movie_win);

	if (swRet != PASS)
		return swRet;

	decoder_initialized = 1; // Move down by C.W, any problems???  091201

#if LYRIC_ENABLE
    if ((sLyricHandle != NULL) && (g_FileType == FILE_TYPE_MP3)||(g_FileType == FILE_TYPE_WMA))
    {
		Lrc_init(sLyricHandle);
		g_sLyric = NULL;
    }
#endif
	MovieGetMediaInfo (0, (BYTE *) & byInfoBuf);
	if(byInfoBuf.dwFlags & MOVIE_INFO_WITH_VIDEO)
	{
		if (byInfoBuf.dwFlags & MOVIE_TotalTime_USEFUL)
		{
			g_dwTotalSeconds = byInfoBuf.dwTotalTime;
		}
	}

	EventSet(MOVIE_EVENT, EVENT_ENTER_MOVIE_PLAYER);
	TaskYield();

	return PASS;
}

int EnterMoviePreview (STREAM * sHandle, unsigned char *extension, ST_IMGWIN * screen_win, ST_IMGWIN * movie_win)
{
	SWORD swRet;

	Media_Info byInfoBuf;

	MP_DEBUG("%s(,%s,)", __FUNCTION__, extension);
    video_thumb_mode = 0;
    Last_Time = -1;
    pre_parse = 0;
//    if(!strcmp(extension,"MP4")) MP4_Flag=1;
    g_FileType = get_type_by_extension (extension, 3);

    //MP_DEBUG1("extension = %s", extension);
    //MP_DEBUG1("type = %d", type);

	swRet = MovieTask_Init(sHandle, g_FileType, screen_win, movie_win);
	if (swRet != PASS)
		return swRet;

	MovieGetMediaInfo (0, (BYTE *) &byInfoBuf);
	if(byInfoBuf.dwFlags & MOVIE_INFO_WITH_VIDEO)
	{
		if (byInfoBuf.dwFlags & MOVIE_TotalTime_USEFUL)
		{
			g_dwTotalSeconds = byInfoBuf.dwTotalTime;
		}
	}

	EventSet(MOVIE_EVENT, EVENT_ENTER_MOVIE_PLAYER);
	TaskYield();

	return PASS;
}

void MovieVideoFullScreen ()
{
	//full_screen_mode= (g_psSystemConfig->sVideoPlayer.isFullScreen == SETUP_MENU_VIDEO_FULL_SCREEN_ON);  //Mason//SourceSafe
	full_screen_mode= 1;//(g_psSystemConfig->sVideoPlayer.isFullScreen == 0);
	if (!full_screen_mode)
	{
		//Idu_PaintWin(Idu_GetCurrWin (), 0x00008080);
	}
}


///
///@ingroup group_exter_func
///@brief   This function will send the MOVIE_VIDEO_TO_BEGIN instruction,
///         and then the Movie Player will begin to play movie according to this instruction .
///
///@param   No
///
///@return  No
///
void MoviePlay ()
{
	MP_DEBUG("MoviePlay");

#if (AO_AV == AV_TYPE)
#if 0
	int local_dev = 1;

	if (filter_graph.demux->stream->fd->Drv->DevID == DEV_USB_WIFI_DEVICE)
	{
		local_dev = 0;
	}


//	if (local_dev)
//	{
//	    if (filter_graph.v_decoder)
//		    MovieBackward(1);		// temporary solution for MPEG1 pts error
//	}
#endif

#endif
    g_bPlayFirstVideoFrame = 1;
	MovieTask_Control (MOVIE_VIDEO_TO_BEGIN);
}


///
///@ingroup group_exter_func
///@brief   This function will send the MOVIE_VIDEO_STOP instruction,
///         and then the Movie Player will stop movie playing according to this instruction .
///
///@param   No
///
///@return  No
///
void MovieStop ()
{
#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
	mpDebugPrint("%s",  __FUNCTION__);
#endif
	if (MovieTask_CheckStoped()) return;

#if ANTI_TEARING_ENABLE
	CHANNEL *dma = (CHANNEL *) (DMA_IDU_BASE);
	ST_IMGWIN *curwin = Idu_GetCurrWin();
	while ((dma->StartB - dma->StartA) > (curwin->wWidth << 1));
	frame_buffer_index = buffer_switch = 0;
	video_stop = TRUE;
#endif
	EventSet(AVP_SYNC_EVENT, EVENT_AV_STOP);

    MovieTask_Control (MOVIE_VIDEO_STOP);
	MovieTask_WaitStoped();

#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
	mpDebugPrint("%s end", __FUNCTION__);
#endif
}


///
///@ingroup group_exter_func
///@brief   This function will send the MOVIE_VIDEO_PAUSE instruction, and then the Movie Player will
///         pause movie playing according to this instruction. Also the audio out filter will be paused.
///
///@param   No
///
///@return  No
///
#if 0
void MoviePause ()
{
    MovieTask_Control (MOVIE_VIDEO_PAUSE);
    //MX6xx_AudioPause();
    if (filter_graph.a_out)
        filter_graph.a_out->audio_dev.pause ();
}
#else
#if AUDIO_ON
extern DWORD        PcmChunkCounter;
#endif
void MoviePause ()
{
	int ok;
#if AUDIO_ON
	if (PcmChunkCounter)
#endif
	{
		//MX6xx_AudioPause();
		if (filter_graph.a_out)
			filter_graph.a_out->audio_dev.pause ();

		ok = PollRegister32(g_psDmaAiu->Control, 0, 0x00000001, 500*10000);

		if (!ok)	/* fail */
			UartOutText("MoviePause(): audio driver fail\r\n");
	}
    #if DISPLAY_PAUSE_BYPASS2DMA 
    g_bBypassToDma = 1;
	#endif
	MovieTask_Control (MOVIE_VIDEO_PAUSE);

	//if (filter_graph.a_out)
	//	__CheckAudioBuf();
}
#endif


///
///@ingroup group_exter_func
///@brief   This function will send the MOVIE_VIDEO_TRACKING instruction, and then the Movie Player will
///         resume movie playing according to this instruction. Also the audio out filter will be resumed.
///
///@param   No
///
///@return  No
///
void MovieResume ()
{
	//mpDebugPrint("Moive resume..... %d", AO_AV);
	if(AO_AV == AV_TYPE)
	{
        g_bPlayFirstVideoFrame = 1;
		g_bVideoResume = 1;
		#if MJPEG_ENABLE
		extern BYTE MJ_INIT;
		MJ_INIT=0;
		#endif
		MovieTask_Control (MOVIE_VIDEO_PLAY);
	}	
	else
	{
		MovieTask_Control (MOVIE_AUDIO_RESUME);
	}
}

///////////////////////////////////
///
///@ingroup group_exter_func
///@brief   MovieForward
///
///@param   flag 1:video case 0:audio case
///
///@return  real_seek_seconds
///
float MovieForward (BYTE flag)
{
	int ok;
	sh_video_t         *sh_video = (sh_video_t *) filter_graph.demux->sh_video;
#if 0
	if (flag == 1)			//video case
	   	//StepSeconds = MOVIE_VIDEO_FF_FRAMES / sh_video->fps;
		StepSeconds = MOVIE_VIDEO_FF_SECONDS;
	else if (flag == 0)         //audio case
#else
    if (flag == 0)         //audio case
		StepSeconds = Audio_FB_Seconds;
//		StepSeconds = MOVIE_AUDIO_FF_SECONDS;
#endif


	//frame_num += MOVIE_VIDEO_FF_FRAMES;
	frame_num += (MOVIE_VIDEO_FF_SECONDS * sh_video->fps);
///////////////////
#if AUDIO_ON
	if (PcmChunkCounter)
#endif
	{
		if (filter_graph.a_out)
			filter_graph.a_out->audio_dev.pause ();

		ok = PollRegister32(g_psDmaAiu->Control, 0, 0x00000001, 500 * 1000);

		if (!ok)	/* fail */
		{
			UartOutText("MovieForward(): audio driver fail\r\n");
		}
	}
/////////////////////

	MovieTask_Control (MOVIE_VIDEO_FORWARD);
	seek_completed = 0;

	/* wait for seek complete */
	ok = PollByte(&seek_completed, 1, 2000);

	if (!ok)  /* fail */
		UartOutText("MovieForward(): seek fail\r\n");

	return MOVIE_VIDEO_FF_SECONDS;
}

///
///@ingroup group_exter_func
///@brief   This function will send the MOVIE_VIDEO_STOP instruction,
///         and then the Movie Player will stop movie playing according to this instruction .
///
///@param   flag 1:video case 0:audio case
///
///@return  real_seek_seconds
///
float MovieBackward (BYTE flag)
{
    int ok;
    sh_video_t 	*sh_video = (sh_video_t *) filter_graph.demux->sh_video;
#if 0
    if (flag == 1)		//video case
       // StepSeconds = MOVIE_VIDEO_FB_FRAMES / sh_video->fps;
       StepSeconds = MOVIE_VIDEO_FB_SECONDS;
    else if (flag == 0)         //audio case
#else
	if (flag == 0)         //audio case
        StepSeconds = Audio_FB_Seconds;
//        StepSeconds = MOVIE_AUDIO_FB_SECONDS;
#endif

	//frame_num += MOVIE_VIDEO_FB_FRAMES;
	frame_num += (MOVIE_VIDEO_FB_SECONDS * sh_video->fps);

	if (frame_num < 0)
		frame_num = 0;
#if AUDIO_ON
    if (PcmChunkCounter)
#endif
    {
        if (filter_graph.a_out)
            filter_graph.a_out->audio_dev.pause ();

        ok = PollRegister32(g_psDmaAiu->Control, 0, 0x00000001, 500*1000);

        if (!ok)	/* fail */
            UartOutText("MovieBackward(): audio driver fail\r\n");
    }

    MovieTask_Control (MOVIE_VIDEO_BACKWARD);
    seek_completed = 0;
    /* wait for seek complete */
    ok = PollByte(&seek_completed, 1, 2000);

    if (!ok)	/* fail */
        UartOutText("MovieBackward(): seek fail\r\n");

    return MOVIE_VIDEO_FB_SECONDS;
}

///
///@ingroup group_exter_func
///@brief   This function will send the MOVIE_VIDEO_FORWARD instruction,
///         and then the Movie Player will stop movie playing according to this instruction .
///
///@param   No
///
///@return  No
///
//void MovieFastForward ()
//{
//    StepSeconds = 12;
//    MovieTask_Control (MOVIE_VIDEO_FORWARD);
//}

///
///@ingroup group_exter_func
///@brief   This function will send the MOVIE_VIDEO_BACKWARD instruction,
///         and then the Movie Player will stop movie playing according to this instruction .
///
///@param   No
///
///@return  No
///
//void MovieFastBackward ()
//{
//    StepSeconds = -12;
//    MovieTask_Control (MOVIE_VIDEO_BACKWARD);
//}


///
///@ingroup group_exter_func
///@brief   This function will send the ExitMoviePlayer instruction, then the Movie Player will
///         stop movie displaying according this instruction. And then the movie playerwill release all the allocated resources.
///         Also the movie player will seek to the movie data start address of the file.
///
///@param   No
///
///@return  No
///

void ExitMoviePlayer ()
{
	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
		mpDebugPrint("%s", __FUNCTION__);
	#endif

#if 0
    if (filter_graph.demux)
    {
        if (filter_graph.demux->sh_audio && !filter_graph.demux->sh_video)
			Idu_OsdErase ();

		MovieTask_Control(MOVIE_VIDEO_CLOSE);
		TaskYield();

		if (filter_graph.demux) {
			MP_DEBUG("ExitMoviePlayer filter_graph.demux != NULL");
		}
    }
    else
    {
        AO_AV=NULL_TYPE; //for MJPEG exit issues
        //MP_DEBUG ("filter_graph has not been initialized yet.. Exit...\r\n");
    }
#else	//MP620 style		C.W testing 080905
    //Idu_OsdErase ();
    MovieTask_Control(MOVIE_VIDEO_CLOSE);
	DWORD release;
	#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
		mpDebugPrint("UI wait for player finished");
	#endif
	EventWait(MOVIE_STATUS, EVENT_MOVIE_STATUS_CLOSED|EVENT_MOVIE_STATUS_NOT_START_YET, OS_EVENT_OR, &release);
	EventClear(MOVIE_STATUS, ~EVENT_MOVIE_STATUS_CLOSED);
	EventSet(MOVIE_STATUS, EVENT_MOVIE_STATUS_NOT_START_YET);//close movie if something wrong
	#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
		mpDebugPrint("UI wait for player finished--DONE");
	#endif
#endif
}


///
///@ingroup group_exter_func
///@brief   This function is used to show the first frame of the video stream.
///
///@param   No
///
///@return PASS success
///
extern int          frame_num;
int MovieShowFirstFrame ()
{
	MP_DEBUG("MovieShowFirstFrame");
    if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_DRAW_THUMB)
    {
	    sh_video_t * const sh_video = (sh_video_t *) filter_graph.demux->sh_video;
	    const unsigned int four_cc = sh_video->format;
		if (MJPG_FOURCC(four_cc))
	        return PASS;
    }

    float	pts;
    frame_num = 0;

    if (filter_graph.demux->get_pts_by_relframe)
        pts = filter_graph.demux->get_pts_by_relframe (-65535, 0);

	MovieSeek (pts, 1);
	reset_av_player_controller (1);
	if (ShowFirstFrame(&filter_graph, pdwScreenWin, pdwMovieWin) == PASS)
	{
		MoviePlayerDispTime(VIDEO_SHOW_TIMER_ENABLE);
		MovieTask_Control(MOVIE_VIDEO_STOP);
		MovieTask_WaitStoped();
		return PASS;
	}
	else
	{
	    MovieTask_Control(MOVIE_VIDEO_STOP);
		MovieTask_WaitStoped();
	    MovieTask_Control(MOVIE_VIDEO_CLOSE);
		return FAIL;
	}
}


///
///@ingroup group_exter_func
///@brief   This function is used to get the thumbnail of the video stream.
///
///@param   ST_IMGWIN* win  The image display window
///@param   DRIVE * drv     This structure includes info about storage device, file system, and the file for display.
///
///@return  -1 for fail, 0 for success
///
#if 0
int VideoDrawThumb(STREAM * sHandle, BYTE * bExt, ST_IMGWIN * pWin, DWORD x, DWORD y, DWORD w,
                   DWORD h)
{
    int type, ret;
    register BIU *biu;
    mp_image_t video_mpi;
    ST_IMGWIN tmpwin;

    memset(&video_mpi, 0, sizeof(mp_image_t));
//    #if ((CHIP_VER & 0xffff0000) == CHIP_VER_612)
//        copy_vlc_table_to_sram();
//    #endif
    video_thumb_mode = 0;

    type = get_type_by_extension (bExt, 3);

    switch(type)
	{
		case FILE_TYPE_AC3:
		case FILE_TYPE_OGG:
		case FILE_TYPE_RAM:
		case FILE_TYPE_WMA:
		case FILE_TYPE_AAC :
		case FILE_TYPE_M4A :
		case FILE_TYPE_MP3 :
		case FILE_TYPE_AMR :
			AO_AV = AO_TYPE;
			break;

		case FILE_TYPE_AVI :
		case FILE_TYPE_ASF :
		case FILE_TYPE_MOV :
		case FILE_TYPE_MPEG_PS :
		case FILE_TYPE_FLV:
		case FILE_TYPE_MPEG_TS:
		case FILE_TYPE_MKV:
			AO_AV = AV_TYPE;
			break;

		default:
			AO_AV = NULL_TYPE;
	};

    stream_t *psTempStream = stream_create (sHandle);
    ret = build_filter_graph_AV(psTempStream, type, &ret);

    if (ret != PASS)
    {
        return ret;
    }

    int bflag_get_img = 0;
    unsigned char *start = NULL;
    int in_size;
    float next_frame_duration = 0;
    int try_count = 50;

    while (try_count--)
    {
        if (filter_graph.demux->sh_video==0x0)//for audio MP4
        	break;
        in_size = video_read_frame (filter_graph.demux->sh_video, &next_frame_duration, &start, 0);

        if (in_size >= 0)
        {
            if (filter_graph.v_decoder->video_decoder.decode)
            {
                x &= 0xfffffffe;
                tmpwin.dwOffset = (pWin->wWidth - w) << 1;
                tmpwin.pdwStart = (DWORD *)((BYTE *)pWin->pdwStart + ((y * pWin->wWidth + x) << 1));
                tmpwin.wHeight = h;
                tmpwin.wWidth = w;

                {
                    bflag_get_img =
                        decode_video (&video_mpi, 0, filter_graph.demux->sh_video, start, in_size, 0);
                    if (bflag_get_img == 1)
                        display_video_thumb (&tmpwin, &video_mpi, 1);
                }
            }
        }

        if (bflag_get_img == 1)
            break;
    }

//    ExitMoviePlayer ();
    MovieTask_StopAudio();
    MovieTask_Close();

    if (try_count != 0)
        return PASS;
    else
        return FAIL;
}
#endif

///
///@ingroup group_exter_func
///@brief   This function is used to get the current media file's parameters.
///
///@param   FILE_TYPE_T enFileType  The media file format
///@param   BYTE *byInfoBuf         The start memory address to store media infor
///
///@return  0 for fail, 1 for success
///
int MovieGetMediaInfo(FILE_TYPE_T enFileType, BYTE * byInfoBuf)
{
    int			ret;

    memset (byInfoBuf, 0, sizeof (Media_Info));

    if (filter_graph.demux)
        ret = filter_graph.demux->get_movie_info ((BYTE *) byInfoBuf, 3);
    else
        ret = 0;

    return ret;
}

#if (MJPEG_ENABLE || VIDEO_ENABLE)
int MovieChangeToFullScreenMode(void)
{
    g_change_to_thumb_mode = 0;
    if (Api_MoviePause() != PASS)
		return FAIL;
	IODelay(2000);
	pdwMovieWin = Idu_GetCurrWin();
	boVideoPreview = FALSE;
	g_psSystemConfig->sVideoPlayer.dwPlayerStatus &= ~MOVIE_STATUS_DRAW_THUMB;
	g_psSystemConfig->sVideoPlayer.dwPlayerStatus |= MOVIE_STATUS_FULL_SCREEN;
	
	sh_video_t * const sh_video = (sh_video_t *) filter_graph.demux->sh_video;	
	if (sh_video)
	{
		if (filter_graph.v_decoder->video_decoder.change_mode)
			filter_graph.v_decoder->video_decoder.change_mode(sh_video);		
	}	
		
	//Reset_Video_Frame_Cnt();
	decoder_initialized = 1;

	// Change DMA priority
	#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650)|((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	DMA *dma = (DMA *)DMA_BASE;
	if ((g_sHandle->Drv->DevID ==DEV_SD_MMC)|(g_sHandle->Drv->DevID ==DEV_SD2)|(g_sHandle->Drv->DevID ==DEV_CF)|(g_sHandle->Drv->DevID ==DEV_MS)|(g_sHandle->Drv->DevID ==DEV_CF_ETHERNET_DEVICE))
	{
	    dma->FDMACTL_EXT2 = 0x00044002;
	    dma->FDMACTL_EXT3 = 0xfffbbffd;
	    dma->FDMACTL_EXT1 |= 0x00000001 ;
	}
	mpDebugPrint("sHandle->Drv->DevID %d", g_sHandle->Drv->DevID);

	if ((g_sHandle->Drv->DevID ==DEV_USB_WIFI_DEVICE)|(g_sHandle->Drv->DevID ==DEV_USBOTG1_HOST_PTP)|(g_sHandle->Drv->DevID ==DEV_USBOTG1_HOST_ID1)|
        (g_sHandle->Drv->DevID ==DEV_USBOTG1_HOST_ID2)|(g_sHandle->Drv->DevID ==DEV_USBOTG1_HOST_ID3)|(g_sHandle->Drv->DevID ==DEV_USBOTG1_HOST_ID4))
	{
		dma->FDMACTL_EXT2 = 0x00000602;
	    dma->FDMACTL_EXT3 = 0xfffff9fd;
	    dma->FDMACTL_EXT1 |= 0x00000001 ;
	}    
	
#endif 
	if (Api_MovieResume() != PASS)
		return FAIL;
	
   return PASS;
}

MovieChangeToThumbMode(ST_IMGWIN *stScreenWin, ST_IMGWIN *stMovieWin,
                                  DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, XPG_FUNC_PTR* xpg_func_ptr, BYTE play_mode)
{
    g_change_to_thumb_mode = 1;
	g_change_to_thumb_initialized = 0;
    MP_DEBUG("Api_MovieChangeToThumbMode");	
	MP_DEBUG1("dwX = %d", dwX);
	MP_DEBUG1("dwY = %d", dwY);
	MP_DEBUG1("dwWidth = %d", dwWidth);
	MP_DEBUG1("dwHeight = %d", dwHeight);
	MP_DEBUG1("stScreenWin->wX = %d", stScreenWin->wX);
	MP_DEBUG1("stScreenWin->wY = %d", stScreenWin->wY);
	MP_DEBUG1("stScreenWin->dwWidth = %d", stScreenWin->wWidth);
	MP_DEBUG1("stScreenWin->wHeight = %d", stScreenWin->wHeight);

	stMovieWin->wX = dwX;
	stMovieWin->wY = dwY;
	stMovieWin->wWidth = dwWidth;
	stMovieWin->wHeight = dwHeight;    
    g_pXpgFuncPtr  = xpg_func_ptr;
	g_bPlayMode    = play_mode;
	extern ST_IMGWIN *pdwScreenWin;
    extern ST_IMGWIN *pdwMovieWin;
	pdwScreenWin   = stScreenWin;
	pdwMovieWin    = stMovieWin;
	
    if (Api_MoviePause() != PASS)
		return FAIL;
	IODelay(2000);	
    Idu_ReinitIdu();
	g_psSystemConfig->sVideoPlayer.dwPlayerStatus &= ~MOVIE_STATUS_FULL_SCREEN;
    g_psSystemConfig->sVideoPlayer.dwPlayerStatus |= MOVIE_STATUS_DRAW_THUMB;
	sh_video_t * const sh_video = (sh_video_t *) filter_graph.demux->sh_video;	
	if (sh_video)
	{
		if (filter_graph.v_decoder->video_decoder.change_mode)
			filter_graph.v_decoder->video_decoder.change_mode(sh_video);		
	}	
	
	boVideoPreview = TRUE;

	MP650_FPGA_Change_DMA_Priority_IDU();
	#if YUV444_ENABLE
	Idu_Chg_422_Mode();
	#endif
	Idu_Bypass2DMA(Idu_GetCurrWin());
	
	if (Api_MovieResume()!= PASS)
		return FAIL;
}
#endif
	
#if DISPLAY_VIDEO_SUBTITLE
int MovieGetSubtitleInfo (S_SUBTITLE_INFO_T* p_subtitle_info)
{
    int	ret;
    ret = sub_get_subtitle_info(p_subtitle_info);

    return ret;
}
#endif

int MovieOpenFileAndGetMediaInfo(STREAM *sHandle, FILE_TYPE_T enFileType, BYTE *byInfoBuf)
{
    int ret;

    MP_DEBUG("enFileType = %d, byInfoBuf = 0x%x", enFileType, byInfoBuf);
    pre_parse = 2;
    memset(byInfoBuf, 0, sizeof(Media_Info));

    //mpDebugPrint("Movie open file and get media info");
    SeekSet(sHandle);
    ret = GetAudioTrackInfo(sHandle, enFileType, (DWORD *)&ret, (BYTE *)byInfoBuf);
    AO_AV = NULL_TYPE;

    return ret;
}

void MoviePlayerDispTotalTime (DWORD dwTotalSeconds)
{
    Media_Info          byInfoBuf;

    if (g_bAniFlag & ANI_SLIDE)
        return;

    memset (&byInfoBuf, 0, sizeof (Media_Info));
    MovieGetMediaInfo (0, (BYTE *) & byInfoBuf);

	dwTotalSeconds = 0;
    //if(byInfoBuf.dwFlags & MOVIE_INFO_WITH_VIDEO)
    {
        if (byInfoBuf.dwFlags & MOVIE_TotalTime_USEFUL)
        {
            dwTotalSeconds = byInfoBuf.dwTotalTime;
        }
    }

	g_dwTotalSeconds = dwTotalSeconds;
}

extern int frame_num;

void MoviePlayerDispTime (BYTE blRedraw)
{
    DWORD dwSec = 0;
	BYTE srt_str[1024+1];
	int i;
#if (DISPLAY_VIDEO_SUBTITLE || LYRIC_ENABLE)	
	E_CHAR_ENCODING char_encoding;
#endif
#if (LYRIC_ENABLE)	
	int temp_cur_timetag = 0;
#endif

 	int ii;
    if (g_bAniFlag & ANI_SLIDE)
        return;

    if (AO_AV == AV_TYPE)
    {
//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
        if (FILE_EXT_AVI == bFileType)
            dwSec = Get_AviVideoCurrentTime();
	    else if (FILE_EXT_MOV == bFileType)
	        dwSec = Get_MovVideoCurrentTime();
#else
        if (filter_graph.demux->sh_video)
        {
            if (filter_graph.demux->video)	dwSec = (DWORD) ((float)GetVideoDisplayPTS()/1000 + 0.5);				
			#if DISPLAY_VIDEO_SUBTITLE
		    if ((g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_FULL_SCREEN) &&
                (g_SubNodeCur != NULL) &&
                (g_SubNodeTail != NULL)
		        )
		    {		        
				if (g_SubNodeCur->s_sub.end <= g_SubNodeCur->s_sub.start)
				{
					if (g_SubNodeCur != g_SubNodeTail)
				       g_SubNodeCur = g_SubNodeCur->p_next;
				}
				
		        if ((GetVideoDisplayPTS() >= g_SubNodeCur->s_sub.start) && 
					(GetVideoDisplayPTS() <= g_SubNodeCur->s_sub.end)   &&
					!g_SubNodeCur->s_sub.displaying)
		        {
		            
		            g_SubNodeCur->s_sub.displaying = TRUE;		            
		            char_encoding = sub_get_char_encoding();
				    srt_str[0] = '\0';
					if (CHAR_ENCODING_UTF_16_LE == char_encoding || CHAR_ENCODING_UTF_16_BE == char_encoding)
					    srt_str[1] = '\0';				    

				    if (CHAR_ENCODING_UTF_8 == char_encoding)
				    {
				        for (i=0; i<g_SubNodeCur->s_sub.lines; i++)
				        {				        
                            if (strlen(g_SubNodeCur->s_sub.text[i]) > 1024)
				   	            break;

						    if (i>0)
								strcat(srt_str, " ");
								
				            strcat(srt_str, g_SubNodeCur->s_sub.text[i]);
				        
				            if (strlen(srt_str) > 1024)
				   	           break;
				        }
				    }
					else 
					{
				        for (i=0; i<g_SubNodeCur->s_sub.lines; i++)
				        {				        
                            if (sub_wcslen(g_SubNodeCur->s_sub.text[i]) > 1024)
				   	            break;

						    if (i>0)
								sub_wcscat(srt_str, " ");
				            sub_wcscat(srt_str, g_SubNodeCur->s_sub.text[i]);
				        
				            if (sub_wcslen(srt_str) > 1024)
				   	           break;
				        }
					}	
					if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateOsdSRT)
                        g_pXpgFuncPtr->xpgUpdateOsdSRT(srt_str);
		        }
			    else if ((GetVideoDisplayPTS() > g_SubNodeCur->s_sub.end) && g_SubNodeCur->s_sub.displaying)
			    {
			        g_SubNodeCur->s_sub.displaying = FALSE;
					if ((g_SubNodeCur->s_sub.end + 10) <= g_SubNodeCur->p_next->s_sub.start)
					{
                        if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateOsdSRT)
						    g_pXpgFuncPtr->xpgUpdateOsdSRT("");
					}
				    if (g_SubNodeCur != g_SubNodeTail)
				       g_SubNodeCur = g_SubNodeCur->p_next;
			    }
		    }
            #endif
	    }
#endif
    }
	else if (AO_AV == AO_TYPE)
	{
		AUDIO_DEVICE * const audio_dev = &filter_graph.a_out->audio_dev;
		if (audio_dev == NULL) return;
#if AUDIO_ON
	dwSec = ( AudioGetPlayPTS() + 500 ) / 1000;
	#if LYRIC_ENABLE // Display Lyrics, by Eddyson 2010.07.09
		/* move to lrc.c
		x_lrc = 20; //x-axis point to show the first line of lyric
		y_lrc = 70; //y-axis point to show the first line of lyric
		//cur_line = 0;
		total_line = 5; //Total number of lines to show
		line_dist=20; //line distance
		*/
		#if 0
		if (cur_timetag!=0 && cur_timetag <= total_timetag)
		{	
			//temp_cur_timetag = cur_timetag;
			temp_cur_timetag = cur_timetag;
			//MP_DEBUG2("--cur_timetag=%d, %d",cur_timetag, Lrc_show((int)AudioGetPlayPTS()));
		}
		#endif
		
		for(temp_cur_timetag = cur_timetag; temp_cur_timetag < total_timetag; temp_cur_timetag++)
		{
		//MP_DEBUG1("--cur_timetag= %d",cur_timetag);
			if (AudioGetPlayPTS() >= lrc_timetags[temp_cur_timetag].time && !lrc_timetags[temp_cur_timetag].bdisplaying)
			{
					
				for (cur_line = 0; cur_line < total_line; cur_line++)
				{
					//MP_DEBUG2("x_lrc=%d, y_lrc=%d",x_lrc,y_lrc);
					//g_pXpgFuncPtr->xpgUpdateOsdLrc((BYTE *)lrc_timetags[cur_timetag].lyric);

					if (((temp_cur_timetag + cur_line) > (total_timetag - 1)))
					{
						for (ii = cur_line; ii != total_line; ii++) // Clear the rest of unused lines after the last line
						{
							if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateOsdLrc)
								g_pXpgFuncPtr->xpgUpdateOsdLrc("",x_lrc, y_lrc + (line_dist*ii));
						}

						break;
					}
						if(lrc_timetags[temp_cur_timetag].bfilled == TRUE)
						{
							if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateOsdLrc)
							{
								g_pXpgFuncPtr->xpgUpdateOsdLrc((BYTE *)lrc_timetags[temp_cur_timetag + cur_line].lyric, x_lrc, y_lrc + (line_dist*cur_line));
							}
						}	

					if ((total_line == 1) || (cur_line != (total_line - 1) ) )
					{
						lrc_timetags[temp_cur_timetag + cur_line].bdisplaying = true;
					}

					MP_DEBUG4(" -I- cur_timetag(%d) + cur_line(%d) = %d, total_timetag = %d",temp_cur_timetag, cur_line, (temp_cur_timetag + cur_line), total_timetag);
					MP_DEBUG2(" -I- MP3 time, Lyric time: %d, %d",AudioGetPlayPTS(),lrc_timetags[temp_cur_timetag + cur_line].time);
					MP_DEBUG1(" -I- Lyric: %s",lrc_timetags[temp_cur_timetag + cur_line].lyric);

				}

				if ((total_line >= 2))
				{
					cur_timetag = temp_cur_timetag + total_line - 1; // Minus 2 to repeat the last line on the next "page"
					MP_DEBUG1("--cur_timetag= %d",cur_timetag);
				}
				break;

			}

			else if (AudioGetPlayPTS() < lrc_timetags[temp_cur_timetag].time && lrc_timetags[temp_cur_timetag].bdisplaying)
			{
				lrc_timetags[temp_cur_timetag].bdisplaying = false;
				#if 0
				for (cur_line = 0; cur_line < total_line; cur_line++)
				{	
					MP_DEBUG("test clean");
					if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateEQInfo)
						g_pXpgFuncPtr->xpgUpdateOsdLrc("",x_lrc, y_lrc + (line_dist*cur_line));
				}
				/*while (AudioGetPlayPTS() < lrc_timetags[cur_timetag-1].time)
				{
				MP_DEBUG("testtest");
				cur_timetag--;
				}*/
				#endif

			}

		}
			
	#endif
#else
	dwSec = 0;
#endif
		if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateEQInfo)
			g_pXpgFuncPtr->xpgUpdateEQInfo(g_dwTotalSeconds, dwSec);

	}

    BYTE blUpdateTime = FALSE;
	if (dwSec != Last_Time)
	{
	    blUpdateTime = TRUE;
		Last_Time = dwSec;

		/*
		 *  AV task does not manage current time, and transmit current(total) time to UI layer directly.    C.W 10/04/23
		 *
		if(g_dwTotalSeconds != 0){
			if ((Last_Time >= g_dwTotalSeconds && Last_Time < g_dwTotalSeconds + 3) || (dwSec > g_dwTotalSeconds)) 	//prevent from dispaly over Total time
				dwSec = g_dwTotalSeconds;
			else if(Last_Time >= g_dwTotalSeconds + 2)		//If  Current time large than Total time 2 secs,  show fake current time
				dwSec -= 2;
		}
		 */
	}

    else if (filter_graph.demux->sh_video && frame_num < 7)
        dwSec = 0;

    if ((FALSE == blRedraw) || (FALSE == blUpdateTime))
        return;

	if (!filter_graph.demux->sh_video && !MJPEG)
	{
	    if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateTimeBar)
		    g_pXpgFuncPtr->xpgUpdateTimeBar(g_dwTotalSeconds, dwSec); // music
	}
	else
	{
	    if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateOsdTimeBar)
	        g_pXpgFuncPtr->xpgUpdateOsdTimeBar(g_dwTotalSeconds, dwSec); // video
	}
}

void GetVideoCodecCategory(char* const p_video_codec)
{
    const unsigned int g_uiVideoFormat = ((sh_video_t *) filter_graph.demux->sh_video)->format;

	switch (g_uiVideoFormat)
	{
#if VIDEO_ENABLE
	    case MPEG1:
		case mpg1:
		case MPEG:
			strcpy(p_video_codec, "MPEG1");
			break;

	    case MPEG2:
	    case DVR:  //not sure, robert
	    case hdv2:
	    case PIM1:
	    case VCR2:
	    case mpg2:
	    case MPG2:
	    case hdv3:
	    case mx5p:
	    case MMES:
	    case mmes:
			strcpy(p_video_codec, "MPEG2");
			break;

	    case DIV3:
	    case div3:
	    case DIV4:
	    case div4:
	    case MP43:
	    case mp43:
			strcpy(p_video_codec, "DIVX ");
			break;

	    case MPEG4_XVID:
	    case MPEG4_xvid:
	    case MPEG4_DIVX:
	    case MPEG4_divx:
	    case MPEG4_DX50:
	    case mp4v:
	    case MP4V:
	    case M4S2:
			strcpy(p_video_codec, "MPEG4");
			break;

	    case s263:
	    case h263:
			strcpy(p_video_codec, "H.263");
			break;

	    case h264:
	    case H264:
	    case x264:
	    case X264:
	    case avc1:
	    case AVC1:
	    case davc:
	    case DAVC:
			strcpy(p_video_codec, "H.264");
			break;

	    case FLV1:
			strcpy(p_video_codec, "FLV  ");
			break;
#endif

#if MJPEG_ENABLE
		case JPEG:					// motion jpeg
		case jpeg:
		case MJPG:
		case mjpg:
		case dmb1:
			strcpy(p_video_codec, "MJPEG");
			break;
#endif
	   default:
	   		strcpy(p_video_codec, "NONE ");
       		break;
    }
}

void GetAudioCodecCategory(char* const p_audio_codec)
{
    const unsigned int g_uiAudioFormat = ((sh_audio_t *) filter_graph.demux->sh_audio)->format;
	switch (g_uiAudioFormat)
	{
		case MP3_50:
		case MP3_55:
		case MP3_CVBR:
		case MP3_FCC:
		case 0x56:				//Video status case
		case 0x58:				//Video status case
			strcpy(p_audio_codec, "MP3  ");
			break;

#if WMA_ENABLE
		case WMA_T:
		case 0x161:
			strcpy(p_audio_codec, "WMA  ");
			break;
#endif

#if (AC3_AUDIO_ENABLE)
		case AC3_T :
			strcpy(p_audio_codec, "AC3  ");
			break;
#endif

#if AAC_SW_AUDIO
		case mp4a: //AAC case
		case MP4A:
		case 0x57:
		case 0x59:
			strcpy(p_audio_codec, "AAC  ");
			break;
#endif

#if OGG_ENABLE
		case OGG_T :
			strcpy(p_audio_codec, "OGG  ");
			break;
#endif

#if RAM_AUDIO_ENABLE
		case RAM_T:
		case ram_T:
			strcpy(p_audio_codec, "RAM  ");
			break;
#endif

#if VIDEO_ENABLE
#ifdef AMR_NB_ENABLE
		case AMR_NB:
			strcpy(p_audio_codec, "AMR  ");
			break;
#endif
#endif

#if WAV_ENABLE
		case PCM_RAW:
		case PCM_TWOS:
		case PCM_SOWT:
#endif
		case PCM_1:
		case 0x7:
		case ulaw:
		case ULAW:
		case 0x6:
		case ALAW:
		case alaw:
		case 0x11:
		case DVI_ADPCM:
		case 0x2:
		case 0x809DDF48:
			strcpy(p_audio_codec, "PCM  ");
			break;

		default:
			strcpy(p_audio_codec, "NONE ");
			break;
	}
}
#endif
