/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mpapi_inter.c
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
#include "mptrace.h"
#include "av.h"

#include "mpapi.h"
#include "mpapi_inter.h"

//For set UI_EVENT , AUDIO and VIDEO end
#include "os.h"
#include "ui.h"
#include "taskid.h"


#if (DISPLAY_VIDEO_SUBTITLE || LYRIC_ENABLE)
#ifndef _SUB_COMMON_H_
#include "sub_common.h"
#endif
#endif

#if LYRIC_ENABLE
#include "Lrc.h"
#endif

#ifndef xpg__H__
#include "xpg.h"
#endif

// Put this ugly definition here temporarily!!
#define MP4_RESYNC_ADIF    0x40000

float StepSeconds = 0;
DWORD Audio_FB_Seconds = 0;
static int TaskYieldCount;
static int MovieInstruction;
int g_iMovieInstructionBk;
void *BitStream_Source = 0;  //correct value is in codec init
extern ST_IMGWIN *pdwScreenWin;
extern ST_IMGWIN *pdwMovieWin;

extern Audio_dec Media_data;
extern sh_audio_t sh_audio_wrapper;
extern BYTE seek_completed;
extern enum AO_AV_TYPE AO_AV;
extern XPG_FUNC_PTR* g_pXpgFuncPtr;
extern BYTE g_bPlayMode;
extern BYTE g_slide;

#if DISPLAY_VIDEO_SUBTITLE
extern S_SUB_NODE_T* g_SubNodeHead;
extern S_SUB_NODE_T* g_SubNodeTail;
extern S_SUB_NODE_T* g_SubNodeCur;
#endif

#if LYRIC_ENABLE
extern PLRC_TIMETAG lrc_timetags; //Lyrics array by Eddyson
extern int cur_timetag, total_timetag; //Current line of lrc file, total number of line to print
extern WORD x_lrc, y_lrc;
extern int cur_line, total_line, line_dist;
#endif

extern S_BK_STREAM_NODE_T* g_BkStreamBufHead;
extern av_player_data_t av_player_data;
extern BYTE g_bBypassToDma;

extern filter_graph_t filter_graph;
//#define	TaskYield()	TaskSleep(1)

///@ingroup group_inter_func
///@brief   This function will call stream_create(drv) to create and init. the stream_t structure, then
///         function build_filter_graph() is called to build filter graph according the stream and its format,
///         and init. the movie display window pointed by pdwMovieWin.
///
///@param   DRIVE *drv          This structure includes info about storage device, file system, and the file for display.
///@param   int type                The file format
///@param   ST_IMGWIN *screen_win     The screen window
///@param   ST_IMGWIN *movie_win      The image display window
///
///@return  0 for fail, 1 for success
extern int MJPEG;

int MovieTask_Init(STREAM * sHandle, int type, ST_IMGWIN * screen_win, ST_IMGWIN * movie_win)
{
	int ret;
	int iErrCode = 0;
	stream_t *psTempStream = 0;
	g_iMovieInstructionBk = 0;


	//step 1 , initial display window pointer
	pdwScreenWin = screen_win;
	pdwMovieWin = movie_win;
	MP_DEBUG("MovieTask_Init: type=%d", type);
	EventSet(MOVIE_STATUS, EVENT_MOVIE_STATUS_NOT_START_YET);//close movie if something wrong
//**********************************
//It temporarily solve avsync fails in playing "*.flv" and can not play some video files with MP3 audio stream.It is not correct  to solve it.
//Note by Xianwen 2010/01/25.
#if MP3_SW_AUDIO
#endif
//**********************************
	//step 2, build relative filter graph
	switch (type)
	{
	case FILE_TYPE_RA  :
	case FILE_TYPE_RAM :
	case FILE_TYPE_RM  :
	case FILE_TYPE_AC3 :
	case FILE_TYPE_WMA :
	case FILE_TYPE_OGG :
	case FILE_TYPE_WAV :
	case FILE_TYPE_AMR :
	case FILE_TYPE_MP3 :
	case FILE_TYPE_AAC :
	case FILE_TYPE_M4A :
		AO_AV = AO_TYPE;
		ret = build_filter_graph_AO(sHandle, type, (int *) & iErrCode);
		break;

	case FILE_TYPE_ASF :
	case FILE_TYPE_AVI :
	case FILE_TYPE_MOV :
	case FILE_TYPE_MPEG_PS :
	case FILE_TYPE_FLV :
	case FILE_TYPE_MPEG_TS :
	case FILE_TYPE_MKV :
	case FILE_TYPE_h264 :
	case FILE_TYPE_h263 :
		AO_AV = AV_TYPE;
		psTempStream = stream_create (sHandle);

		ret = build_filter_graph_AV(psTempStream, type, (int *) & iErrCode);
		if(ret!=PASS)
		{
			mpDebugPrint("filter error_free stream");
			stream_free(psTempStream);
		}

		break;

	default :

		psTempStream = stream_create (sHandle);
		ret = build_filter_graph_AV(psTempStream, type, (int *) & iErrCode);
		break;

	}

	if (ret != PASS)
	{
		if (iErrCode != 0)
		{
			if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PREVIEW))
				if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgOsdPopupError)
					g_pXpgFuncPtr->xpgOsdPopupError(iErrCode);
		}
		MP_ALERT(" can't build filter graph");
		return ret;
	}

	//step 3, set display window's width & height
	if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PREVIEW) &&
	        !(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_DRAW_THUMB)
	   )
	{
		pdwMovieWin->wWidth  = get_vdisplay_width();
		pdwMovieWin->wHeight = get_vdisplay_height();
	}

	return PASS;
}

///
///@ingroup group_inter_func
///@brief   This is the movie player task, which will handle all the cases of movie player such as Play, Pause.
///         This function is a Cosmos Task.
///
///@param   No
///
///@return  No
///
#ifdef TOTAL_DECODE_TIME
extern DWORD        TickCount0;
extern TIMER       *timer;
#endif

extern int          frame_num;

//#if  Libmad_FAAD
//extern DWORD        MPA_WAIT_TIME;
//extern DWORD        DsFillBuf_T;
//#endif
//DWORD               file_num = 1;
DWORD               forward_flag = 0;
DWORD               backward_flag = 0;
DWORD               audiostream_finish;
//#if  Libmad_FAAD
//extern int          Layer1_Inited;
//extern int          Layer2_Inited;
//extern int          Layer3_Inited;
//extern int          n_frames;
//#endif
extern int32_t     av_timer_pts;
extern void reset_av_player_controller(const int);
extern void stop_av_player_controller(void);


static DWORD dwMovieTaskStatus;

BYTE MovieTask_CheckPlaying()
{
	DWORD dwStatus = g_psSystemConfig->sVideoPlayer.dwPlayerStatus;

	if ((filter_graph.demux == NULL) || (dwStatus == 0) ||
	        (dwStatus & MOVIE_STATUS_STOP) || !(dwStatus & MOVIE_STATUS_PLAY) )
		//if ((filter_graph.demux == NULL) ||(dwMovieTaskStatus != MOVIE_VIDEO_PLAY))
	{
		MP_DEBUG("movie is not playing");
		return FALSE;
	}
	return TRUE;
}


BYTE MovieTask_CheckStoped()
{
	return (dwMovieTaskStatus == MOVIE_VIDEO_STOP || filter_graph.demux == NULL);
}

void MovieTask_WaitStoped()
{
	DWORD release;
	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
		mpDebugPrint("wait EVENT_MOVIE_STATUS_STOPED");
	#endif
	EventWait(MOVIE_STATUS, EVENT_MOVIE_STATUS_STOPED, OS_EVENT_OR, &release);
	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
		mpDebugPrint("wait EVENT_MOVIE_STATUS_STOPED ok");
	#endif
	EventClear(MOVIE_STATUS, ~EVENT_MOVIE_STATUS_STOPED);
}

BYTE MovieTask_CheckClosed()
{
	return (filter_graph.demux == NULL);
}

static int MovieTask_Resync_Audio(DWORD Next_Second)
{
	int swRet = 0;

	//Modulize resyn function
	swRet = resync_audio_stream_ext(Next_Second);

	seek_completed = 1;
#if AUDIO_ON
	if (swRet == MP4_RESYNC_ADIF)		//AAC ADIF format not support FW/BW
	{
		Next_Second = Media_data.play_time;
		mpDebugPrint("ADIF FW/BW not support");
	}
	else
		AudioSetPlayPTS((DWORD)Next_Second);
#endif
	return swRet;
}

extern BOOL boVideoPreview;

static void Player_MOVIE_VIDEO_PLAY(int * const opcode, const int LastMovieInstruction)
{
	MP_DEBUG("%s", __FUNCTION__);

	if (filter_graph.a_out)
		filter_graph.a_out->audio_dev.resume();
	EventSet(AVP_SYNC_EVENT, EVENT_A_START_PLAY);


	int swRet = play_next_media_frame (&filter_graph);

	//mpDebugPrint("swRet MOVIE_VIDEO_PLAY %d",swRet);
	if (swRet != PASS )
	{
		*opcode = MOVIE_VIDEO_STOP;
		if (filter_graph.demux->sh_video)
		{
			if (VIDEO_PLAY_MODE == g_bPlayMode)
				EventSet (UI_EVENT, EVENT_VIDEO_END);
			else
				EventSet (UI_EVENT, EVENT_YOUTUBE_END);
		}
		else
			EventSet (UI_EVENT, EVENT_AUDIO_END);
		return;
	}

	if (!boVideoPreview)
	{
		TimerDelay(10);

		MP_DEBUG("MOVIE_VIDEO_PLAY next");

		swRet = play_next_media_frame (&filter_graph);

		if (swRet != PASS)
		{
			*opcode = MOVIE_VIDEO_STOP;
			if (filter_graph.demux->sh_video)
			{
				if (VIDEO_PLAY_MODE == g_bPlayMode)
					EventSet (UI_EVENT, EVENT_VIDEO_END);
				else
					EventSet (UI_EVENT, EVENT_YOUTUBE_END);
			}
			else
				EventSet (UI_EVENT, EVENT_AUDIO_END);
			return;
		}
	}
	dwMovieTaskStatus = MOVIE_VIDEO_PLAY;
	*opcode = MOVIE_VIDEO_TRACKING;
#if AUDIO_ON
	AiuDmaRegCallBackFunc(AudioISR);
#endif
	DmaIntEna(IM_AIUDM);

	if (MOVIE_VIDEO_FORWARD  == LastMovieInstruction ||
	        MOVIE_VIDEO_BACKWARD == LastMovieInstruction
	   )
	{
		MovieInstruction = LastMovieInstruction;
	}
	MP_DEBUG("MOVIE_VIDEO_TRACKING start");
}

static void Player_MOVIE_VIDEO_TRACKING(int * const opcode)
{
	MP_DEBUG("%s", __FUNCTION__);

	//video data in file is eof while audio is not and buffer in aout is also run out.

	const int swRet = play_next_media_frame (&filter_graph);
	//mpDebugPrint("swRet MOVIE_VIDEO_TRACKING %d",swRet);
	if (swRet != PASS)
	{
#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_OTHER)
		TRACELN;
#endif
		*opcode = MOVIE_VIDEO_STOP;

		if (filter_graph.demux->sh_video)
		{
#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_OTHER)
			TRACELN;
#endif
			if (VIDEO_PLAY_MODE == g_bPlayMode)
			{
#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
				mpDebugPrint("player send EVENT_VIDEO_END %s", __FUNCTION__);
#endif
				EventSet(UI_EVENT, EVENT_VIDEO_END);
				EventSet(UI_EVENT, EVENT_VIDEO_END);//for BurnInCode
			}
			else
				EventSet(UI_EVENT, EVENT_YOUTUBE_END);
		}
		else
			EventSet(UI_EVENT, EVENT_AUDIO_END);
	}
}

static void AV_Seek_Check(int * const opcode, const int begin)
{
	DWORD release;
	if (EventPolling(AV_DECODER_EVENT, EVENT_V_DECODER_HDRS_RDY, OS_EVENT_OR, &release)==OS_STATUS_POLLING_FAILURE || begin==0)
	{
		*opcode = MOVIE_VIDEO_TO_BEGIN;
		MP_ALERT("!EVENT_V_DECODER_HDRS_RDY %s", __FUNCTION__);
		return;
	}
	EventWait(AV_DECODER_EVENT, EVENT_V_NOT_DECODING|EVENT_A_NOT_DECODING, OS_EVENT_AND, &release);
}

static void Player_MOVIE_VIDEO_FORWARD(int * const opcode, int * const LastMovieInstruction, const int begin)
{
	MP_DEBUG("%s", __FUNCTION__);
	int	Next_Second;
	*LastMovieInstruction = 0;
	if (Media_data.fending == 1)
		Media_data.fending = 0;
	if (AO_AV == AO_TYPE)
	{
		// Using real caculating time instead of using playing time returned by audio decoder  C.W 10/04/26
		int curPTS;
#if AUDIO_ON
		curPTS = AudioGetPlayPTS();
#endif
		Next_Second = curPTS + Audio_FB_Seconds * 1000;
		//Next_Second = curPTS + MOVIE_AUDIO_FF_SECONDS * 1000;
		//Next_Second = Media_data.play_time + MOVIE_AUDIO_FF_SECONDS * 1000;

		if (Next_Second > Media_data.total_time * 1000)
#if 0			// Forward until end of file
			Next_Second = Media_data.total_time * 1000;
#else			// Forwarding will change to next song if touching end of file
		{
			*opcode = MOVIE_VIDEO_STOP;
			EventSet (UI_EVENT, EVENT_AUDIO_END);
		}
		else
#endif
#if LYRIC_ENABLE
		MP_DEBUG("LYRIC forward");
		MP_DEBUG1("Next_Second=%d",Next_Second);
		MP_DEBUG2("Last cur_timetag=%d, lrc time=%d",cur_timetag, lrc_timetags[cur_timetag].time);
		cur_timetag = Find_cur_timetag((DWORD) Next_Second);//lrc_pos_seek((DWORD) (Next_Second), TRUE);
		MP_DEBUG2("Next cur_timetag=%d, lrc time=%d",cur_timetag, lrc_timetags[cur_timetag].time);

#endif

		{
#if AUDIO_ON
			extern DWORD PcmChunkCounter;

			// Wait for ISR closed by audio_dev.pause() 's flag.
			// ISR will set PcmChunkCounter value 0.
			while (PcmChunkCounter)
				TaskYield();
#endif
			IODelay(100);			// For avoid abnormal audio sound, we just dma hardware need some delay time.

			const int swRet = MovieTask_Resync_Audio(Next_Second);
			if (swRet != MP4_RESYNC_ADIF)		//AAC ADIF format not support FW/BW
				filter_graph.a_out->audio_dev.ClearAudioBuffer();	//Because of all audio buffer is useless, clear all audio buffer!
			*opcode = MOVIE_VIDEO_PLAY;

			//Correct current decoder position of time. For Fixing push forward/backward key quickly
			Media_data.play_time = Next_Second;
			MoviePlayerDispTime(TRUE);
		}
	}
	else
	{

		const float pts = Media_data.play_time;
		stop_av_player_controller();

		forward_flag = 1;
		backward_flag = 0;
		audiostream_finish = 0;

		AV_Seek_Check(opcode, begin);
		MovieSeek (StepSeconds, (int) 0);

#if DISPLAY_VIDEO_SUBTITLE
		sub_pos_seek((DWORD) (StepSeconds + filter_graph.demux->video->pts*1000.0f), TRUE);
#endif
#if AUDIO_ON
		AudioSetPlayPTS((DWORD)(filter_graph.demux->video->pts * 1000.0f));
#endif
		//av_resync();
		*opcode = MOVIE_VIDEO_CONTINUE;
	}
}

static void Player_MOVIE_VIDEO_BACKWARD(int * const opcode, int * const LastMovieInstruction, const int begin)
{
	MP_DEBUG("%s", __FUNCTION__);
	int	Next_Second;
	*LastMovieInstruction = 0;
	if (Media_data.fending == 1)
		Media_data.fending = 0;

	if (AO_AV == AO_TYPE)
	{
		// Using real caculating time instead of using playing time returned by audio decoder  C.W 10/04/26
		int curPTS;
#if AUDIO_ON
		curPTS = AudioGetPlayPTS();
#endif
		Next_Second = curPTS + Audio_FB_Seconds* 1000;
		//Next_Second = curPTS + MOVIE_AUDIO_FB_SECONDS * 1000;
		//Next_Second = Media_data.play_time + MOVIE_AUDIO_FB_SECONDS * 1000;
		if (Next_Second < 0)
			Next_Second = 0;
#if AUDIO_ON
		extern DWORD PcmChunkCounter;
		while (PcmChunkCounter)	// Wait until audio ISR closed
			TaskYield();
#endif
		IODelay(100);			// For avoid abnormal audio sound, we just dma hardware need some delay time.

		const int swRet = MovieTask_Resync_Audio(Next_Second);
		if (swRet != MP4_RESYNC_ADIF)		//AAC ADIF format not support FW/BW
			filter_graph.a_out->audio_dev.ClearAudioBuffer();	//Because of all audio buffer is useless, clear all audio buffer!

		*opcode = MOVIE_VIDEO_PLAY;

		//Correct current decoder position of time. For Fixing push forward/backward key quickly
		Media_data.play_time = Next_Second;
		MoviePlayerDispTime(TRUE);

#if LYRIC_ENABLE
		MP_DEBUG("LYRIC backward");
		MP_DEBUG1("Next_Second=%d",Next_Second);
		MP_DEBUG2("Last cur_timetag=%d, lrc time=%d",cur_timetag, lrc_timetags[cur_timetag].time);
		cur_timetag = Find_cur_timetag((DWORD) Next_Second);//lrc_pos_seek((DWORD) (Next_Second), FALSE);
		MP_DEBUG2("Next cur_timetag=%d, lrc time=%d",cur_timetag, lrc_timetags[cur_timetag].time);
		//lrc_pos_seek((DWORD) (StepSeconds), FALSE);

#endif

	}
	else
	{
		const float pts = Media_data.play_time;
		stop_av_player_controller();

		backward_flag = 1;
		forward_flag = 0;
		//if (backward_flag >2)
		//backward_flag = 2;

		audiostream_finish = 0;

		AV_Seek_Check(opcode, begin);
		MovieSeek (StepSeconds, (int) 0);

#if DISPLAY_VIDEO_SUBTITLE
		sub_pos_seek((DWORD) (StepSeconds + filter_graph.demux->video->pts*1000.0f), FALSE);
#endif
#if AUDIO_ON
		AudioSetPlayPTS((DWORD)(filter_graph.demux->video->pts*1000.0f));
#endif
		//av_resync();

		*opcode = MOVIE_VIDEO_CONTINUE;
	}
}

static void Player_MOVIE_VIDEO_STOP(int * const opcode, int * const begin)
{
#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
	mpDebugPrint("%s", __FUNCTION__);
#endif
	if (frame_num <= 0)
		frame_num = 1;

	if (AO_AV == AV_TYPE)
		stop_av_player_controller();

	//if (filter_graph.a_out)
	//	filter_graph.a_out->audio_dev.pause();
	MovieTask_StopAudio();

	dwMovieTaskStatus = MOVIE_VIDEO_STOP;
	*opcode = 0;
	*begin = 0;
	EventSet(MOVIE_STATUS, EVENT_MOVIE_STATUS_STOPED);
	#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
		mpDebugPrint("EVENT_MOVIE_STATUS_STOPED sent");
	#endif
	#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
		mpDebugPrint("%s end", __FUNCTION__);
	#endif
}

static void Player_MOVIE_VIDEO_TO_BEGIN(int * const opcode, int * const begin)
{
	MP_DEBUG("%s", __FUNCTION__);
	frame_num = 0;
#ifdef FS_TIME
	FS_READ_TIME = 0;
	FS_SEEK_TIME = 0;
	FS_READ_COUNT = 0;
	FS_READ_COUNT = 0;
	FS_READ_SIZE = 0;
#endif

	//if (filter_graph.a_out)
	//	filter_graph.a_out->audio_dev.pause ();
	MovieTask_StopAudio();

	if (AO_AV == AV_TYPE)
	{
		float pts;
		if (filter_graph.demux->get_pts_by_relframe)
			pts = filter_graph.demux->get_pts_by_relframe (-65535, 0);
		int local_dev = 1;

		if (DEV_USB_WIFI_DEVICE == filter_graph.demux->stream->fd->Drv->DevID || DEV_CF_ETHERNET_DEVICE == filter_graph.demux->stream->fd->Drv->DevID )
			local_dev = 0;

		if (local_dev)
			MovieSeek (pts, 1);
	}
	*begin = 1;
	*opcode = MOVIE_VIDEO_PLAY;
}

static void Player_MOVIE_VIDEO_PAUSE()
{
	MP_DEBUG("%s", __FUNCTION__);
	if ((g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_FULL_SCREEN) && (AV_TYPE == AO_AV))
	{
		if (g_bBypassToDma)
		{
			register av_player_data_t * const player_data = &av_player_data;
			if (!filter_graph.a_decoder && filter_graph.v_decoder)
			{
				extern mp_image_t video_mpi_L;
				display_video_bypass2dma(&video_mpi_L);
				g_bBypassToDma = 0;
			}
			else
			{
				if (player_data->v_display_fifo.get_count(&player_data->v_display_fifo))
				{
					const frame_entry_t f_entry = player_data->v_display_fifo.glance_head(&player_data->v_display_fifo);
					mp_image_t * const video_mpi = (mp_image_t *)f_entry.buffer;
					display_video_bypass2dma(video_mpi);
					g_bBypassToDma = 0;
				}
				else
				{
					if (player_data->v_decoded_fifo.get_count(&player_data->v_decoded_fifo))
					{
						const frame_entry_t f_entry = player_data->v_decoded_fifo.get(&player_data->v_decoded_fifo);
						player_data->v_display_fifo.add(&player_data->v_display_fifo, &f_entry);
						if (!player_data->v_decoded_fifo.get_count(&player_data->v_decoded_fifo))
							EventClear(AVP_SYNC_EVENT, ~EVENT_V_DECODED);
					}
				}
			}
		}
	}
	TaskYield();
}

static void Player_MOVIE_VIDEO_CLOSE(int * const opcode, int * const exit)
{
#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
	mpDebugPrint("%s", __FUNCTION__);
#endif
	if (AO_AV == AV_TYPE)
		stop_av_player_controller();

	forward_flag = 0;
	backward_flag = 0;
	g_slide=0;//to fix M-Card/USB pen drive Plug-in out cause system hang or crash
#if MJPEG_ENABLE
	extern WORD g_wImg_h;
	g_wImg_h = 0;
#endif
	MovieTask_StopAudio();

	*opcode = 0;
	//Tm2ClearCallBackFunc();
	MovieTask_Close();

	*exit = 1;
	EventSet(MOVIE_STATUS, EVENT_MOVIE_STATUS_CLOSED);
#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
	mpDebugPrint("%s sent EVENT_MOVIE_STATUS_CLOSED @ #%d", __FUNCTION__, __LINE__);
#endif
}

static void Player_MOVIE_VIDEO_CONTINUE(int * const opcode)
{
#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
	mpDebugPrint("%s", __FUNCTION__);
#endif
	DWORD release;
	if (EventPolling(AVP_SYNC_EVENT, EVENT_AV_EOF, OS_EVENT_OR, &release)==OS_STATUS_OK)
	{
		*opcode = 0;
		EventSet(UI_EVENT, EVENT_VIDEO_END);
	}
	else
	{
		reset_av_player_controller (1);
		*opcode = MOVIE_VIDEO_PLAY;
	}
}

//--------------------------------------------------------------------
static void MovieTask_Play()
{
	int opcode = 0, exit = 0;
	MovieInstruction = 0;

	const BYTE bDriveId = g_psSystemConfig->sStorage.dwCurStorageId;
	int begin = 0;
	int LastMovieInstruction;

	//mpDebugPrint("[Build Filter Graph]\n sh_audio: %x, Audio out: %x",
	//			 filter_graph.demux->sh_audio, filter_graph.a_out);
	TaskYieldCount = 0;
	EventClear(MOVIE_STATUS, ~EVENT_MOVIE_STATUS_NOT_START_YET);
	if (filter_graph.a_out)
	{
		unsigned int four_cc;
		register AIU * const aiu = (AIU *) (AIU_BASE);
		const int ret11 = filter_graph.a_out->audio_dev.init (filter_graph.a_out->ao_data.samplerate,
		                  filter_graph.a_out->ao_data.channels,
		                  filter_graph.a_out->ao_data.format,
		                  filter_graph.a_out->ao_data.fourcc);

		//MP_DEBUG1("sh_audio->format=%x",((sh_audio_t *) filter_graph.demux->sh_audio)->format);

		if (AO_AV == AV_TYPE)
		{
			four_cc = ((sh_audio_t *) filter_graph.demux->sh_audio)->format;
			filter_graph.a_out->audio_dev.playCfg.playMode = AV_TYPE;
			filter_graph.a_out->audio_dev.playCfg.preloadBufNum = 20;
		}
		else
		{
			four_cc = sh_audio_wrapper.format;
			filter_graph.a_out->audio_dev.playCfg.playMode = AO_TYPE;
			filter_graph.a_out->audio_dev.playCfg.preloadBufNum = 2;
		}

		if (ret11 < 0)
		{
			MP_DEBUG ("Audio Output device initializes failed!");
			opcode = MOVIE_VIDEO_STOP;
		}
	}
	else
	{
		if (filter_graph.demux->sh_audio)
			mpDebugPrint("[Movie Task play] Search Audio output fail");
	}

	dwMovieTaskStatus = MOVIE_VIDEO_ENTER;

	while (!exit)
	{
		if (MovieInstruction)
		{
			if (begin == 0 &&
			        (MovieInstruction == MOVIE_VIDEO_FORWARD || MovieInstruction == MOVIE_VIDEO_BACKWARD))
			{
				SemaphoreWait(AV_CMD_SEMA_ID);
				LastMovieInstruction = MovieInstruction;
				opcode = MOVIE_VIDEO_TO_BEGIN;
				MovieInstruction = 0;
				SemaphoreRelease(AV_CMD_SEMA_ID);
			}
			else
			{
				SemaphoreWait(AV_CMD_SEMA_ID);
				MP_DEBUG1("MovieInstruction=%d", MovieInstruction);
				opcode = MovieInstruction;
				MovieInstruction = 0;
				SemaphoreRelease(AV_CMD_SEMA_ID);


			}
		}

		if (opcode == 0)
		{
			TaskSleep(1);
			continue;
		}

		//mpDebugPrint("opcode: %d", opcode);
		switch (opcode)
		{
		case MOVIE_VIDEO_PLAY:
			Player_MOVIE_VIDEO_PLAY(&opcode, LastMovieInstruction);
			break;

		case MOVIE_VIDEO_TRACKING:
			Player_MOVIE_VIDEO_TRACKING(&opcode);
			break;

		case MOVIE_VIDEO_BACKWARD:
			Player_MOVIE_VIDEO_BACKWARD(&opcode, &LastMovieInstruction, begin);
			break;

		case MOVIE_VIDEO_FORWARD:
			Player_MOVIE_VIDEO_FORWARD(&opcode, &LastMovieInstruction, begin);
			break;

		case MOVIE_VIDEO_STOP:
			Player_MOVIE_VIDEO_STOP(&opcode, &begin);
			break;

		case MOVIE_VIDEO_TO_BEGIN:
			Player_MOVIE_VIDEO_TO_BEGIN(&opcode, &begin);
			break;

		case MOVIE_VIDEO_CONTINUE:
			Player_MOVIE_VIDEO_CONTINUE(&opcode);
			break;

		case MOVIE_VIDEO_PAUSE:
			Player_MOVIE_VIDEO_PAUSE();
			break;

		case MOVIE_VIDEO_CLOSE:
			Player_MOVIE_VIDEO_CLOSE(&opcode, &exit);
			break;

		case MOVIE_AUDIO_RESUME:
			//mpDebugPrint("MOVIE AUDIO RESUME");
			opcode = MOVIE_VIDEO_PLAY;
			break;

		default:
			MP_ALERT("Invalid player opcode=%d", opcode);
			break;
		}
		#if DEMO_PID
		if (opcode == MOVIE_VIDEO_TRACKING)
		{
			MP_DEBUG("%s :  @ %d", __FUNCTION__, __LINE__);
			if (TaskYieldCount > 100)
			{
				TaskSleep(1);
				TaskYieldCount = 0;
			}
			else
				TaskYield ();
			TaskYieldCount++;
			MP_DEBUG("%s :  @ %d", __FUNCTION__, __LINE__);
		}
		else
		{
			MP_DEBUG("%s :  @ %d", __FUNCTION__, __LINE__);
			TaskSleep(1);
			MP_DEBUG("%s :  @ %d", __FUNCTION__, __LINE__);
		}
		#else
		TaskYield ();
		#endif
	}
}

//--------------------------------------------------------------------
void MovieTask_StopAudio()
{
#if LYRIC_ENABLE //Clear lyric phrases of display after stop, by Eddyson
	for (cur_line = 0; cur_line < total_line; cur_line++)
	{
		g_pXpgFuncPtr->xpgUpdateOsdLrc("",x_lrc, y_lrc + (line_dist*cur_line));
	}
#endif


	if (filter_graph.a_out)
	{
		filter_graph.a_out->audio_dev.pause();
		//filter_graph.a_out->audio_dev.ClearAudioBuffer();
	}
}

//--------------------------------------------------------------------
#define EVENT_MASK 0xffffffff

// if dwNextEvent is not zero, then set it as next event
SDWORD MovieTask_WaitEvent(DWORD * pdwEvent, DWORD dwNextEvent)
{
	SDWORD ret;

	if (dwNextEvent)
	{
		*pdwEvent = dwNextEvent;
		ret = OS_STATUS_OK;
	}
	else
	{
		ret = EventWait(MOVIE_EVENT, EVENT_MASK, OS_EVENT_OR, pdwEvent);
	}
	return ret;
}

//--------------------------------------------------------------------
void MovieTask_Main()
{
	DWORD dwEvent, dwNextEvent;

	dwNextEvent = 0;
	while (1)
	{
		if (MovieTask_WaitEvent(&dwEvent, dwNextEvent) == OS_STATUS_OK)
		{
			if (dwEvent & EVENT_ENTER_MOVIE_PLAYER)
			{
				MovieTask_Play();
				MP_DEBUG("e");
			}
		}
	}

	TaskTerminate(TaskGetId());	//move MoviePlayTask to DORMANT state, this will never be excuted
}

//--------------------------------------------------------------------
///@ingroup group_inter_func
///@brief   This function is used to set the value of global variable MovieInstruction.
///         And the MoviePlayTask's actions are controlled by this variable, MovieInstruction.
///
///@param   int instruc     The instruction used to control the MoviePlayTask.
///
///@return  No
void MovieTask_Control(int instruc)
{
	//EventSet(MOVIE_EVENT, instruc); //for future use
	SemaphoreWait(AV_CMD_SEMA_ID);
	MovieInstruction = instruc;
	g_iMovieInstructionBk = MovieInstruction;
	SemaphoreRelease(AV_CMD_SEMA_ID);
}

//--------------------------------------------------------------------
///
///@ingroup group_inter_func
///@brief   This function is used to close the movie player, and free all the allocated resources.
///
///@param   stream_t* stream
///
///@return  No
///

//void MovieTask_Close(stream_t * stream)
void MovieTask_Close(void)
{

	BIU       *biu;
	CLOCK     *clock;
	CHANNEL   *Dramdma;
	SRMGP     *srm = (SRMGP *) SRMGP_BASE;
	int i, j;

#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
	mpDebugPrint("%s", __FUNCTION__);
#endif

	if (filter_graph.a_out)
		filter_graph.a_out->audio_dev.uninit();    //close audio-out
	if (filter_graph.v_decoder)
		uninit_video_decode (&filter_graph);

	if (filter_graph.a_decoder)
		uninit_audio_decode (&filter_graph);

	if (AO_AV == AV_TYPE)
	{
		if (filter_graph.demux && filter_graph.demux->stream)
			stream_free(filter_graph.demux->stream);//close files

		if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PREVIEW))
		{
			if (filter_graph.demux && filter_graph.demux->stream_a)
			{
				mem_free(filter_graph.demux->stream_a);//close files
			}
			while (g_BkStreamBufHead != NULL)
			{
				S_BK_STREAM_NODE_T* p_next = g_BkStreamBufHead->p_next;
				mem_free(g_BkStreamBufHead);
				g_BkStreamBufHead = p_next;
			}
		}

#if DISPLAY_VIDEO_SUBTITLE
		while (g_SubNodeHead != NULL)
		{

			S_SUB_NODE_T* p_next = g_SubNodeHead->p_next;
			for (i = 0; i < g_SubNodeHead->s_sub.lines; i++)
			{
				if (g_SubNodeHead->s_sub.text[i] != NULL)
				{
					mem_free(g_SubNodeHead->s_sub.text[i]);
				}
			}
			mem_free(g_SubNodeHead);
			g_SubNodeHead = p_next;
		}
		g_SubNodeHead = NULL;
		g_SubNodeTail = NULL;
		g_SubNodeCur = NULL;
#endif
	}
	else
	{
		FileClose(Media_data.dec_fp);

#if LYRIC_ENABLE
		Lrc_destroy ();
#endif

	}

	free_all_demuxes();

	AO_AV = NULL_TYPE;

	/* for test start */
	filter_graph.a_decoder = 0;
	filter_graph.a_out = 0;
	filter_graph.v_out = 0;
	filter_graph.v_decoder = 0;
	memset ((BYTE *) & filter_graph, 0, sizeof (filter_graph_t));
	/* for test end */

	//reset MPV
	biu = (BIU *) (BIU_BASE);
	biu->BiuArst &= 0xffff7fff;
	biu->BiuArst |= 0x00008000;

	clock = (CLOCK *) CLOCK_BASE;
	Dramdma = (CHANNEL *) DMA_INTSRAM1_BASE;

	//Stop SRAM DMA Channel
	Dramdma->Control = 0;
	srm->DMACfg = 0;
	//BIU Asyn Reset for AIU
#if (AUDIO_DAC != DAC_INTERNAL || HAVE_AMP_MUTE)
	biu->BiuArst &= 0xfffffdff;
	biu->BiuArst |= 0x00000200;
#endif
	clock->MdClken &= ~0x02;    // disable MPV clock

#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
	mpDebugPrint("%s end", __FUNCTION__);
#endif
}
#endif
