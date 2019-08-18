/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      avsync.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: audio & video synchronize
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
#define Dump_Img_Enable 0


/*
// Include section
*/
#include <string.h>

#define MAX_OUTBURST    65536

#include "mptrace.h"
#include "timer.h"
#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux.h"

#include "stheader.h"
#include "filter_graph.h"
#include "audio_decoder.h"
#include "config_codec.h"
#include "taskid.h"
#include "devio.h"
#include "audio.h"
#include "video_decoder.h"
#include "lrc.h"
#include "mpapi_inter.h"
#include "mpfifo.h"

extern sh_audio_t sh_audio_wrapper;
extern enum AO_AV_TYPE AO_AV;
#define	VIDEO_650	(READ_VIDEO_ON_DECODING && CHIP_VER_MSB == CHIP_VER_650 && VIDEO_ENABLE)

#if VIDEO_650
static unsigned char *g_start = NULL;
static int g_in_size = 0;
static float g_pts = 0;
static float last_duration = 0;
static int h264_output_end = 0;
static int h264_input_end = 0;
#endif

int hantro_decoding_video = 0;

unsigned int LastSysTime,NowSysTime;
extern float set_video_fps;

// AV player controller state
static float next_frame_duration = 0;	//second
static int drop_video_frame = 0;
int xvid_bframe = 0;

static float sleeping_time_of_pure_video = 0;	//second

BOOL boScalerBusy, boMPABusy, boXpgBusy;
#if AUDIO_ON
extern int AudioGetDevicePTS();
extern DWORD PcmChunkCounter;
extern DWORD AUDIO_BUF_SIZE;
#endif
#ifndef AUDIO_BUF_SIZE2
#define AUDIO_BUF_SIZE2 16*1024
#endif

av_player_data_t av_player_data;
#define	WAIT_FOR_PLAY_RETVAL	2

extern ST_IMGWIN *pdwScreenWin;
extern ST_IMGWIN *pdwMovieWin;
extern BOOL boVideoPreview;

#ifdef MPEG1_DROP_B_FRAME
extern BYTE MPEG1_FRAME_TYPE;
#endif
extern char *g_sLyric;

extern int frame_num;


static pts_fifo_t pts_fifo;
BYTE g_bPlayFirstVideoFrame;

DWORD audio_delay_time;
extern FILE_TYPE_T g_FileType;

mp_image_t video_mpi_L;


inline static pts_entry_t pts_for_display_buffer(const demuxer_t* const demuxer)
{
	int pts_from_stream = 0;
	if (demuxer->control(DEMUXER_CTRL_IF_PTS_FROM_STREAM, &pts_from_stream) == DEMUXER_CTRL_OK && pts_from_stream)
		return pts_fifo.get_display_order(&pts_fifo);
	else
		return pts_fifo.get(&pts_fifo);
}

/*!
This function will decode compressed audio data into one decompressed audio buffer
*/
inline int _decode_AudioFrame(sh_audio_t *sh_audio, AUDIO_DEVICE *audio_dev)
{
	AUDIO_BUF *audio_buf;
	int ret;

	audio_buf = audio_dev->GetAudioBuf_D();
	if (audio_buf == NULL)
		return PASS;
	#if AUDIO_ON
	ret = Decode_audio(sh_audio, (unsigned char *) (audio_buf->buf), AUDIO_BUF_SIZE, AUDIO_BUF_SIZE2);
	#endif
	if (ret <= 0)
	{
		//UartOutText ("Decode_audio(): FAIL !");
		audio_dev->AudioBuf2Ready_D(audio_buf);		//If decoding frame fail, put audio buffer back to free list
		return FAIL;
	}

	sh_audio->delay += (float) ret / ((float) ((audio_dev->m_Config).nBytes));
	audio_buf->offset = ret;
	audio_dev->AudioBuf2Ready_D(audio_buf);

	return PASS;
}
BYTE g_slide;
static int Decode_AudioFrame(sh_audio_t * sh_audio, AUDIO_DEVICE * audio_dev)
{
	//	mpDebugPrint("Decode_AudioFrame");
	int ret;
	DWORD buf_num = audio_dev->GetAudioSpace();

	//mpDebugPrint("buf_num %d",buf_num);
	if (buf_num > 0)
	{
		ret = _decode_AudioFrame(sh_audio, audio_dev);
		if (ret != PASS)
		{
			if (AO_AV == AO_TYPE)
			{
				//sh_audio->ds->eof = 1;
				sh_audio->ds1.eof = 1;
			}
			g_slide = 0;
			return FAIL;
		}
		//buf_num = audio_dev->GetAudioSpace();
		g_slide = 1;
	}
	else
		g_slide = 0;

	return PASS;
}

extern filter_graph_t filter_graph;
inline void decode_more_audio(void)
{
	sh_audio_t *sh_audio = (sh_audio_t *) (filter_graph.demux)->sh_audio;
	AUDIO_DEVICE *audio_dev = &((filter_graph.a_out)->audio_dev);

	if (!sh_audio)
		return;
	if ((sh_audio->delay > 0.01) || (sh_audio->ds1.eof))
		return;

	//if(sh_audio->ds->eof)     return;
	Decode_AudioFrame(sh_audio, audio_dev);

	return;
}

#if LYRIC_ENABLE
//------------------------------------------------------------------------------------
//  For LRC file Parsing
//------------------------------------------------------------------------------------
static void ParsingLyric(filter_graph_t* graph)
{
	sh_audio_t* sh_audio = (sh_audio_t*)graph->demux->sh_audio;
	sh_video_t* sh_video = (sh_video_t*)graph->demux->sh_video;
	AUDIO_DEVICE *audio_dev = &graph->a_out->audio_dev;
	int ms_lrc = 0;

	if ((!sh_video)&&(sh_audio)&&(sh_audio->format == 0x55)) 	//0X55 is the fourcc of MP3 FILE.
	{
		char *plrc;
		ms_lrc = (int)((sh_audio->delay- audio_dev->GetDelay())*1000);
		plrc = Lrc_show(ms_lrc);
		if ((NULL != plrc) && (g_sLyric != plrc))
		{
			ST_IMGWIN* pCurWnd = Idu_GetCurrWin();
			Idu_PrintLyrics(pCurWnd, plrc, 8);
			g_sLyric = plrc;
		}
	}
}
#endif

static inline float get_sleep_time(sh_audio_t * sh_audio, AUDIO_DEVICE * audio_dev)
{
	float ret = 0.0f;

//  if(sh_audio && !sh_audio->ds->eof){
	if (sh_audio)
	{
		float delay = audio_dev->GetDelay();	//

		if ((!delay) && (sh_audio->ds->eof))
		{
			return 0.0001f;
		}

		ret = delay - sh_audio->delay;
		if (ret > 0.0001f)
		{
#if 0
			// next video frame available before it is supposed to be displayed
			if (delay > 0.250)
			{
				delay = 0.250;
			}
			else if (delay < 0.100)
				delay = 0.100;

			if (ret > delay * 0.6)
			{
				// sleep time too big - may cause audio drops (buffer underrun)
				*more_audio = 1;	// audio is slow than video
				ret = delay * 0.5;
			}
#endif

			return ret;			//second
		}
		else
		{
			//drop_video_frame = 1;
		}
	}

	return 0.0001f;
}

static int play_next_vo_frame(filter_graph_t  *const graph)
{
	//mpDebugPrint("%s @ %d", __FUNCTION__, __LINE__);
	demuxer_t* const demuxer = graph->demux;
	sh_video_t * const sh_video = (sh_video_t *) demuxer->sh_video;

	int eof = 0;
	int decV_ret;//, decV_flag;
	register av_player_data_t * const player_data = &av_player_data;
	
#if VIDEO_650
	const unsigned int four_cc = sh_video->format;
#endif

#ifdef XVID_DROP_B_FRAME
	xvid_bframe = 0;
#endif
	
	if (sh_video)
	{
		unsigned char *start = NULL;
		int in_size;

		GetRelativeMs();

		//Curr_MS = GetCurMs();
#ifdef MPEG1_DROP_B_FRAME
		MPEG1_FRAME_TYPE = 0;
#endif
		
#if VIDEO_650
		if (MJPG_FOURCC(four_cc))
			in_size = video_read_frame(sh_video, &next_frame_duration, &start, 0);
		else
		{
			if (g_in_size <= 0)
				read_video();

			start = g_start;
			in_size = g_in_size;
			g_in_size = 0;
		}
#else
		in_size = video_read_frame(sh_video, &next_frame_duration, &start, 0);
#endif

		if (in_size < 0)
			return FAIL;
		else if (in_size==0)
			return 0;

		sh_video->pts = filter_graph.demux->video->current->pts;
		pts_entry_t pts_entry = {NULL, FLOAT1000(filter_graph.demux->video->current->pts), NULL, NULL, control_video(sh_video, CMD_VD_PICTURE_CODING_TYPE_DISPLAY, NULL, start, in_size)};
		pts_entry_t_init_instance(&pts_entry);
		pts_fifo.add(&pts_fifo, &pts_entry);
		MP_DEBUG("aadd video_pts=%d, type=%d, count=%d", pts_entry.pts, pts_entry.picture_coding_type_d,pts_fifo.count);
		//pts_fifo.printall(&pts_fifo);

#if Dump_Img_Enable
		static DRIVE *sDrv;
		static STREAM *shandle;
		static int ret,i = 2;;
		char *s,t[4];

		strcpy(t, "000");
		//select the pack_no(First_I_frame) u want
		int	First_I_frame = 302, pack_no = graph->demux->video->pack_no;

		if (First_I_frame < graph->demux->video->pack_no)
		{
			while (pack_no>0)
			{
				t[i]=pack_no%10+48;
				pack_no=(pack_no-pack_no%10)/10;
				i--;
			}

			strcpy(s,"source_");
			strcpy(&s[7],t);
			MP_DEBUG("dump source pack_no %d",graph->demux->video->pack_no);
			sDrv = DriveGet(SD_MMC);

			ret = CreateFile(sDrv, s, "raw");
			if (ret) UartOutText("create file fail\r\n");

			shandle = FileOpen(sDrv);
			if (!shandle) UartOutText("open file fail\r\n");

			ret = FileWrite(shandle, start, in_size);
			if (!ret) UartOutText("write file fail\r\n");

			FileClose(shandle);
			UartOutText("\n\rfile close\n\r");
		}
#endif

		if (sh_video->ds->eof)
			eof = 1;

#ifdef MPEG1_DROP_B_FRAME
		//if ((drop_video_frame) && (MPEG1_FRAME_TYPE == 3))
		if (MPEG1_FRAME_TYPE == 3)
		{
			float sleeping_time;
			frame_num++;

			{	//pure video case
				sleeping_time = sleeping_time_of_pure_video - (GetRelativeMs() / 1000.0f);
				//mpDebugPrint("(DWORD)(sleeping_time * 1000.0f) = %d", (DWORD)(sleeping_time * 1000.0f));
				if (((DWORD)(sleeping_time * 1000.0f)) > 0)
				{				    
				    TaskSleep((DWORD)(sleeping_time * 1000.0f));   // Unit: 1 ms
				}    
			}

			return eof ? FAIL : PASS;
		}
#endif
		decV_ret = decode_video(&video_mpi_L, 0, sh_video, start, in_size, drop_video_frame);

		if (decV_ret < 0) eof=1;

#if Dump_Img_Enable

		if (First_I_frame<graph->demux->video->pack_no)
		{
			strcpy(s,"decode_");
			strcpy(&s[7],t);
			ret=CreateFile(sDrv, s, "raw");
			if (ret) UartOutText("create file fail\r\n");

			shandle=FileOpen(sDrv);
			if (!shandle) UartOutText("open file fail\r\n");

			ret=FileWrite(shandle, video_mpi_L.planes[0], 1024*480*2);
			if (!ret) UartOutText("write file fail\r\n");

			FileClose(shandle);
			UartOutText("\n\rfile close\n\r");
		}
#endif
		//MP_TRACE_TIME_PRINT("decode_video", 0);
		// Sleep to wait for audio played out. Once audio/video in sync, display the video frame decoded in the above.

#ifdef XVID_DROP_B_FRAME
		if (xvid_bframe)
		{
			frame_num++;
			//		drop_video_frame = 1;
		}
#endif
		if (decV_ret == 0)
		{
			const pts_entry_t p_entry = pts_fifo.glance_head(&pts_fifo);
			player_data->display_video_pts = p_entry.pts;
		}

#ifdef XVID_DROP_B_FRAME
		if (decV_ret == 1 && !xvid_bframe)
#else
		if (decV_ret == 1)
#endif
		{
			const pts_entry_t p_entry = pts_for_display_buffer(demuxer);
			MP_DEBUG("\tgget_display_order video_pts=%d, type=%d, count=%d", p_entry.pts, p_entry.picture_coding_type_d,pts_fifo.count);
			player_data->display_video_pts = p_entry.pts;
			player_data->stats.total_frames++;
			if (boVideoPreview)
				display_video_thumb (pdwScreenWin, pdwMovieWin, &video_mpi_L, 1);
			else
				display_video(&video_mpi_L, 1);
		}
		next_frame_duration=next_frame_duration*set_video_fps;//use API_SetVideoSpeed 

		if(1==decV_ret )
		{
		if (!drop_video_frame)
		{
			float sleeping_time;
			
			static DWORD dw_last_time = 0;
			static DWORD dw_current_time = 0;

			{	//pure video case
				sleeping_time_of_pure_video = next_frame_duration;
				if (g_bPlayFirstVideoFrame)
				{				    
					g_bPlayFirstVideoFrame = 0;
					sleeping_time = sleeping_time_of_pure_video - (GetRelativeMs() / 1000.0f);
					dw_last_time = GetSysTime();
				}
				else
				{
					dw_current_time = GetSysTime();
					sleeping_time = sleeping_time_of_pure_video - (dw_current_time - dw_last_time)/1000.0f;
					//mpDebugPrint("dw_current_time = %d, dw_last_time = %d, sleeping_time_of_pure_video*1000 = %d, sleeping_time*1000 = %d", dw_current_time, dw_last_time, (DWORD)(sleeping_time_of_pure_video*1000.0f), (DWORD)(sleeping_time*1000.0f));
					dw_last_time = dw_current_time + FLOAT1000(sleeping_time);
				}
				
				if (FLOAT1000(sleeping_time)>0)
				{				                    
					TaskSleep(FLOAT1000(sleeping_time));
				}	
			}
			// end of modified
		}
	}
	}	// if (sh_video)


	if (g_bAniFlag & ANI_VIDEO)
	{	
		if (graph->v_decoder && sh_video)			
			MoviePlayerDispTime(TRUE);
	}
	else
		MoviePlayerDispTime(FALSE);

	return eof ? FAIL : PASS;
}

/*!
This function is used at audio only case.
*/
static int play_next_audio_frame(filter_graph_t* const graph)
{
	//mpDebugPrint("%s @ %d", __FUNCTION__, __LINE__);
	int eof = 0;
	sh_audio_t * const sh_audio = &sh_audio_wrapper;
	AUDIO_DEVICE * const audio_dev = &graph->a_out->audio_dev;

	if (g_bAniFlag & ANI_AUDIO)
		MoviePlayerDispTime(TRUE);

	Decode_AudioFrame(sh_audio, audio_dev);

	//MP_DEBUG1("sh_audio->ds->eof = %d", sh_audio->ds->eof);

	//It prevents that  there is no audio data to decode and audio_dev->ABuf_Playing2Free_Num is not be clear to 0.
	if (sh_audio->ds1.eof && audio_dev->ABuf_Playing2Free_Num > 0 &&
		audio_dev->ABuf_Ready_Num == 0&& audio_dev->ABuf_Playing_Num == 0)
	{
		audio_dev->ClearAudioBuffer();
	}

	if (sh_audio->ds1.eof && audio_dev->ABuf_Ready_Num == 0 && 
		(audio_dev->ABuf_Playing2Free_Num == 0 && audio_dev->ABuf_Playing_Num == 0))
	{
		// Reset Audio end of file flag
		// for fixing audio playing problem during slideshow
		if (AO_AV == AO_TYPE)
		{
			//mpDebugPrint("------- Restart -- Clean audio flag------------");
			sh_audio_wrapper.ds1.eof = 0;
		}
		eof = 1;
	}

	//If the file size is too small and the play time is less than 1s, SmallSizeFile will be set up to 1.
	//We only use one DMA buffer to transfer. By XW 2009/11/5
	if (sh_audio->ds1.eof && audio_dev->ABuf_Ready_Num == 1)
		audio_dev->playCfg.smallSizeFile = 1;
	audio_dev->PlayAudio(audio_dev);	//play the decoded pcm data.

	return eof ? FAIL : PASS;
}


#define	V_WAIT_A_HW_TIMER	5
static void VWaitAIsr(WORD tickCount)
{
    TimerDeInit(V_WAIT_A_HW_TIMER);
	EventSet(AVP_SYNC_EVENT, EVENT_V_WAIT_A);
}


//#define DEBUG_AV_SYNC
static unsigned int av_timer(const demuxer_t* const demuxer)
{
    //mpDebugPrint("%s @ %d", __FUNCTION__, __LINE__);
	static int iDropFrame;
	int iCurVideoPtsOffset, iVideoPtsOffset;
	register av_player_data_t * const player_data = &av_player_data;

	/* counting AV Timer to hardware timer */
	register filter_graph_t * const filter_graph = player_data->filter_graph;
	sh_video_t * const sh_video = filter_graph->demux->sh_video;
	const unsigned int four_cc = sh_video->format;

	/* counting AV Timer to audio timer */
	#if AUDIO_ON
	float first_apts=0.0;
	demuxer->control(DEMUXER_CTRL_GET_1ST_APTS, &first_apts);
	player_data->av_timer_pts = AudioGetDevicePTS() + FLOAT1000(first_apts);
	#endif
	unsigned int retval=0;

#ifdef DEBUG_AV_SYNC
	static int frame_time_last = 0;
	static int frame_time_sum = 0;
	static int frame_sum_display = 0;
	int cur_time;
	int frame_time = 0;
	int frame_time_avg = 0;
#endif

	if (player_data->display_video_pts <= player_data->av_timer_pts)
	{
		retval|=BIT1;
		// check list not empty
		if (player_data->v_display_fifo.get_count(&player_data->v_display_fifo))
		{
			retval|=BIT2;
			frame_entry_t f_entry = player_data->v_display_fifo.glance_head(&player_data->v_display_fifo);
			player_data->stats.total_frames++;
			iVideoPtsOffset = (signed)f_entry.pts - player_data->display_video_pts;
			if((int)(player_data->av_timer_pts - player_data->display_video_pts) < (iVideoPtsOffset - 100)) 
			{
				//avoid video frames from being displayed too soon (more than 100 ms ahead)				
				player_data->stats.total_frames--;
				return retval|=BIT3;
			}
			player_data->display_video_pts = f_entry.pts;
			iCurVideoPtsOffset = player_data->av_timer_pts - player_data->display_video_pts;			

#ifdef DEBUG_AV_SYNC
			mpDebugPrint("dpts=%d, vpts = %d, apts = %d, iVideoPtsOffset = %d",iCurVideoPtsOffset, player_data->display_video_pts, player_data->av_timer_pts, iVideoPtsOffset);
#endif
#if (CHIP_VER_MSB == CHIP_VER_650)
			if ( !(boVideoPreview || MJPG_FOURCC(four_cc)) ||
			        iCurVideoPtsOffset <= (iVideoPtsOffset << 2) || iDropFrame > 8)
#else
			if (iCurVideoPtsOffset <= (iVideoPtsOffset << 2) || iDropFrame > 8)
#endif
			{
				retval|=BIT4;
				/* move to frame buffer */
				mp_image_t * video_mpi = (mp_image_t *) f_entry.buffer;

				if (boVideoPreview)	display_video_thumb (pdwScreenWin, pdwMovieWin, video_mpi, 1);
				else				display_video(video_mpi, 1);
				iDropFrame = 0;
				#ifdef DEBUG_AV_SYNC
					frame_time_last = cur_time;
					cur_time = GetSysTime();

					if (frame_sum_display > 1)
					{
						frame_time = cur_time - frame_time_last;
						frame_time_sum += frame_time;
						frame_time_avg = frame_time_sum/(frame_sum_display-1);
					}
					frame_sum_display++;
					MP_DEBUG("frame_time[%d] = %d", frame_num,frame_time);
					MP_DEBUG("frame_time_avg = %d", frame_time_avg);
					if (!(iCurVideoPtsOffset <= (iVideoPtsOffset << 2)))
						mpDebugPrint("not sync");
				#endif
			}
			else	//not to display
			{
				retval|=BIT5;
#ifdef DEBUG_AV_SYNC
				mpDebugPrint("SKIP video frame :dpts=%d, vpts = %d, apts = %d, iVideoPtsOffset = %d",iCurVideoPtsOffset, player_data->display_video_pts, player_data->av_timer_pts, iVideoPtsOffset);
#endif
				iDropFrame++;
				player_data->stats.missed_frame++;
				player_data->stats.total_missed_time +=
				    player_data->av_timer_pts - f_entry.pts;
			}

			/* remove from our video buffer , glance before*/
			player_data->v_display_fifo.get(&player_data->v_display_fifo);
			player_data->video_mpi_busy[f_entry.frame_no] = 0;
			if (!player_data->v_display_fifo.get_count(&player_data->v_display_fifo))
				EventClear(AVP_SYNC_EVENT, ~EVENT_V_WAIT_FOR_PLAY);
			EventSet(AVP_SYNC_EVENT, EVENT_V_PLAYED);
		}
		else
		{
			retval|=BIT6;
			/* let TV stay in old video frame */
		}

	}
	else
	{
		retval|=BIT7;
		#if  MOVIE_TASKSLEEP
			#define	AVDIFF_MIN	80
			if (!MJPG_FOURCC(four_cc))
			{
				retval|=BIT8;
				const int avdiff= player_data->display_video_pts - player_data->av_timer_pts;
				if (avdiff > AVDIFF_MIN)
				{
					retval|=BIT9;
					#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_AVSYNC)
						mpDebugPrint("player sleep [%d]", avdiff);
					#endif
					TaskSleep(1);
				}
				#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_AVSYNC)
				else
					mpDebugPrint("V>A [%d]", avdiff);
				#endif
			
/*				if (avdiff > AVDIFF_MIN)
				{
					TimerInit(V_WAIT_A_HW_TIMER, avdiff-AVDIFF_MIN, VWaitAIsr);
					DWORD eventstatus;
					EventWait(AVP_SYNC_EVENT, EVENT_V_WAIT_A, OS_EVENT_OR, &eventstatus);
					EventClear(AVP_SYNC_EVENT, ~EVENT_V_WAIT_A);
				}
*/
			}
		#endif
		//mpDebugPrint("V>A %d @ %d", player_data->display_video_pts - player_data->av_timer_pts,__LINE__);
		EventSet(AVP_SYNC_EVENT, EVENT_V_WAIT_FOR_PLAY);
		iDropFrame = 0;
	}

	//mpDebugPrint("%s = 0x%x", __FUNCTION__, retval);
	return retval;
}



/**
 * play audio AND video
 * @retval 0 fail
 * @retval >0 success
 */
static int play_av(filter_graph_t * const graph, const int max_video_buffer)
{
	//mpDebugPrint("%s @ %d", __FUNCTION__, __LINE__);
	sh_audio_t * const sh_audio = (sh_audio_t *) graph->demux->sh_audio;
	sh_video_t * const sh_video = (sh_video_t *) graph->demux->sh_video;
	MP_ASSERT(sh_video && sh_audio);
	
	register av_player_data_t * const player_data = &av_player_data;
	DWORD release, eventstatus;

	player_data->max_video_buffer = max_video_buffer;
	player_data->filter_graph = graph;
	AUDIO_DEVICE * const audio_dev = &graph->a_out->audio_dev;
	
	/* Turn On A/V Decoder */
	if (EventPolling(AV_DECODER_EVENT, EVENT_V_DECODER_START|EVENT_V_DECODER_EOF, OS_EVENT_OR, &release)==OS_STATUS_POLLING_FAILURE)
		EventSet(AV_DECODER_EVENT, EVENT_V_DECODER_START);
	if (EventPolling(AV_DECODER_EVENT, EVENT_A_DECODER_START|EVENT_A_DECODER_EOF, OS_EVENT_OR, &release)==OS_STATUS_POLLING_FAILURE)
		EventSet(AV_DECODER_EVENT, EVENT_A_DECODER_START);

	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_OTHER)
		mpDebugPrint("wait AVP_SYNC_EVENT");
	#endif
	EventWait(AVP_SYNC_EVENT, EVENT_V_DECODED|EVENT_A_DECODED|EVENT_V_WAIT_FOR_PLAY|EVENT_A_START_PLAY|EVENT_AV_EOF|EVENT_AV_STOP, OS_EVENT_OR, &eventstatus);
	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_OTHER)
		mpDebugPrint("AVP_SYNC_EVENT = 0x%x", eventstatus);
	#endif
	
			
	if(eventstatus & EVENT_V_DECODED)
	{
		//mpDebugPrint("fetch one video frame @ %s:%d", __FUNCTION__, __LINE__);
		const frame_entry_t f_entry = player_data->v_decoded_fifo.get(&player_data->v_decoded_fifo);
		player_data->v_display_fifo.add(&player_data->v_display_fifo, &f_entry);
		
		if (!player_data->v_decoded_fifo.get_count(&player_data->v_decoded_fifo))
			EventClear(AVP_SYNC_EVENT, ~EVENT_V_DECODED);

		const unsigned int av_timer_ret = av_timer(graph->demux);
		#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_OTHER)
			mpDebugPrint("av_timer = 0x%x @ %d", av_timer_ret, __LINE__);
		#endif
	}

	if(eventstatus & EVENT_V_WAIT_FOR_PLAY)
	{
		const unsigned int av_timer_ret = av_timer(graph->demux);
		#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_OTHER)
			mpDebugPrint("av_timer = 0x%x @ %d", av_timer_ret, __LINE__);
		#endif
	}

	if (eventstatus & EVENT_A_DECODED)
	{
		//mpDebugPrint("fetch one audio frame");
		const frame_entry_t a_entry = player_data->a_decoded_fifo.get(&player_data->a_decoded_fifo);
		AUDIO_BUF * const audio_buf = (AUDIO_BUF *)a_entry.buffer;
		if (!player_data->a_decoded_fifo.get_count(&player_data->a_decoded_fifo))
			EventClear(AVP_SYNC_EVENT, ~EVENT_A_DECODED);
		audio_dev->AudioBuf2Ready_D(audio_buf);
	}

	if (eventstatus & EVENT_A_START_PLAY)
	{
		#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
			mpDebugPrint("wait %s #%d", __FUNCTION__, __LINE__);
		#endif
		EventWait(AVP_SYNC_EVENT, EVENT_V_ENOUGH_TO_PLAY|EVENT_AV_EOF, OS_EVENT_OR, &release);
		#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
			mpDebugPrint("continue %s #%d", __FUNCTION__, __LINE__);
	#endif
		if (release & EVENT_AV_EOF);
		else if (release & EVENT_V_ENOUGH_TO_PLAY)
		{
			if (audio_dev->PlayAudio(audio_dev)==2)
			{
				//start to play the decoded pcm data.
				EventClear(AVP_SYNC_EVENT, ~EVENT_A_START_PLAY);
				MP_DEBUG("%s @ %d", __FUNCTION__, __LINE__);
			}
		}
	}

	if (g_bAniFlag & ANI_VIDEO)
	{
		if (GetVideoDisplayPTS())
			MoviePlayerDispTime(TRUE);
	}
	else	MoviePlayerDispTime(FALSE);

	return eventstatus & EVENT_AV_EOF ? FAIL : PASS;
}


int play_next_media_frame(filter_graph_t * const graph)
{
#if (PURE_VIDEO == 0)
	if (graph->a_decoder && !graph->v_decoder)	/* pure audio playing */
		return play_next_audio_frame(graph);
	else if (!graph->a_decoder && graph->v_decoder)
		return play_next_vo_frame(graph);
	else
		return play_av(graph, VIDEO_FRAME_QUEUE_NUMBER);
#else
	if (graph->a_decoder && !graph->v_decoder)	/* pure audio playing */
		return play_next_audio_frame(graph);
	else
		return play_next_vo_frame(graph);
#endif
}


#if ANTI_TEARING_ENABLE
extern BYTE frame_buffer_index;
extern BOOL buffer_switch, video_stop;
#endif

static void av_player_data_init(av_player_data_t * const player_data)
{
	frame_fifo_init_instance(&player_data->a_decoded_fifo);
	frame_fifo_init_instance(&player_data->v_decoded_fifo);
	frame_fifo_init_instance(&player_data->v_display_fifo);

	player_data->av_timer_pts = 0;
	player_data->audio_pts = 0;
	player_data->video_pts = 0;
	player_data->uncalc_audio_data_in_pts = 0;
	player_data->display_video_pts = 0;

	player_data->stats.total_frames = 0;
	player_data->stats.missed_frame = 0;
	player_data->stats.total_missed_time = 0;

	int i;
	for (i = 0; i < VIDEO_FRAME_QUEUE_NUMBER; i++)
		player_data->video_mpi_busy[i] = 0;
}
static void av_player_data_flush(av_player_data_t * const player_data)
{
	player_data->a_decoded_fifo.flush(&player_data->a_decoded_fifo);
	player_data->v_decoded_fifo.flush(&player_data->v_decoded_fifo);
	player_data->v_display_fifo.flush(&player_data->v_display_fifo);
}

static void av_player_data_t_init_instance(av_player_data_t * const player_data)
{
	player_data->init	=	av_player_data_init;
	player_data->flush	=	av_player_data_flush;
}


/**
 * @retval <0 fail
 * @retval otherwise success
 */
static int player_init(const int start)
{
	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
		mpDebugPrint("%s(%d)", __FUNCTION__, start);
	#endif
#if VIDEO_650
	g_start = NULL;
	g_in_size = 0;
	g_pts = 0;
	last_duration = 0;
	h264_output_end = 0;
	h264_input_end = 0;
#endif
	av_player_data_t_init_instance(&av_player_data);
	av_player_data.init(&av_player_data);
	
	xvid_bframe = 0;
	
	pts_fifo_init_instance(&pts_fifo);

	/* Reset video/audio decoder */
	if (start)
		EventClear(AV_DECODER_EVENT, 0);
	EventSet(AV_DECODER_EVENT, EVENT_V_NOT_DECODING|EVENT_A_NOT_DECODING);
	if (start)
		EventClear(AVP_SYNC_EVENT, 0);
	return 0;
}

static int player_stop()
{
	register IDU *idu;
	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
		mpDebugPrint("%s @ %d", __FUNCTION__, __LINE__);
	#endif

	mpDebugPrintN("(total:%d, missed:%d, miss rate=%d",
				av_player_data.stats.total_frames,
				av_player_data.stats.missed_frame,
				FLOAT1000((float)(av_player_data.stats.missed_frame)/av_player_data.stats.total_frames));

	if (av_player_data.stats.missed_frame)
		mpDebugPrint(", missed time per frame:%d ms)",
			av_player_data.stats.total_missed_time / av_player_data.stats.missed_frame);
	else
		mpDebugPrint(")");

	const sh_video_t * const sh_video = (sh_video_t *) filter_graph.demux->sh_video;
	const unsigned int four_cc = sh_video->format;
	#if MJPEG_ENABLE
	if (MJPG_FOURCC(four_cc))
		Mjpeg_Decode_Stop();
    #endif
	
	/* Reset video/audio decoder */
	DWORD release;
	EventClear(AV_DECODER_EVENT, ~(EVENT_V_DECODER_START|EVENT_A_DECODER_START));
	EventSet(AVP_SYNC_EVENT, EVENT_V_PLAYED|EVENT_A_PLAYED);
	#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
		EventPolling(AV_DECODER_EVENT, ~0, OS_EVENT_OR, &release);
		mpDebugPrint("player_end wait for video/audio decoder finished : 0x%x", release);
	#endif
	EventWait(AV_DECODER_EVENT, EVENT_V_NOT_DECODING|EVENT_A_NOT_DECODING, OS_EVENT_AND, &release);
	#if	(MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
		mpDebugPrint("player_end wait for video/audio decoder finished--DONE");
	#endif
	
	pts_fifo.flush(&pts_fifo);
	av_player_data.flush(&av_player_data);
	control_video(NULL, CMD_VD_FLUSH_OUTPUT_QUEUE, NULL, NULL, NULL);
	//For IDU module ReInitial with every video play end
	/*
	if(idu->IduSts & BIT0)
	{
		DWORD timer;
		timer = GetSysTime();
		mpDebugPrint("System time =%d\n",timer);
		mpDebugPrint("PLAY END\n");
		DisplayInit(DISPLAY_INIT_LOW_RESOLUTION);
		mpDebugPrint("IDU_ReInitial\n");
		//DisplayInit(DISPLAY_INIT_LOW_RESOLUTION);
	}*/
	return 0;
}


void reset_av_player_controller(const int start)
{
	MP_DEBUG ("%s", __FUNCTION__);
	next_frame_duration			= 0;
	drop_video_frame			= 0;
	sleeping_time_of_pure_video	= 0;
	boScalerBusy = boMPABusy	= FALSE;

#if ANTI_TEARING_ENABLE
	frame_buffer_index			= buffer_switch = video_stop = 0;
#endif

	player_init(start);
}

void stop_av_player_controller()
{
	player_stop();

#if ANTI_TEARING_ENABLE
	CHANNEL * const dma = (CHANNEL *) (DMA_IDU_BASE);
	ST_IMGWIN * const curwin = Idu_GetCurrWin();

	while ((dma->StartB - dma->StartA) > (curwin->wWidth << 1));

	frame_buffer_index = buffer_switch = 0;
	video_stop = TRUE;
#endif
	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
		mpDebugPrint("%s @ %d done", __FUNCTION__, __LINE__);
	#endif
}

/*!
read one video frame, skip B frames following I if closed_gop=0 after seek or broken_link in MPEG series
*/
static int read_one_valid_video_frame(demuxer_t* const demux, unsigned char **start)
{
	sh_video_t * const sh_video = demux->sh_video;
	int closed_gop, broken_link, picture_coding_type_B;
	int in_size;
	//int picture_coding_type

	do
	{
		/* 2. Read one encoded video frame data */
		in_size = video_read_frame(sh_video, &next_frame_duration, start, 0);
		closed_gop				=	control_video(sh_video, CMD_VD_CLOSED_GOP, NULL, *start, in_size);
		broken_link				=	control_video(sh_video, CMD_VD_BROKEN_LINK, NULL, *start, in_size);
		picture_coding_type_B	=	control_video(sh_video, CMD_VD_PICTURE_CODING_TYPE_B, NULL, *start, in_size);
//		picture_coding_type		=	control_video(sh_video, CMD_VD_PICTURE_CODING_TYPE_DISPLAY, NULL, *start, in_size);

		if (demux->seeked)
		{
			demux->seeked++;
			//mpDebugPrint("seeked=%d B=%d, type=%s",demux->seeked,picture_coding_type_B,CODING_TYPE(picture_coding_type));
		}
	}
	while ((demux->seeked > 2 || broken_link==1) && closed_gop==0 && picture_coding_type_B==1);

	if (demux->seeked > 2) demux->seeked = 0;
	return in_size;
}


int ShowFirstFrame(filter_graph_t * graph, ST_IMGWIN * screen_win, ST_IMGWIN * movie_win)
{
	int bflag_get_img = 0;
	int try_count = 50;
	sh_video_t *sh_video = (sh_video_t *) filter_graph.demux->sh_video;
	const unsigned int four_cc = sh_video->format;
	ST_IMGWIN originWin;
	BOOL _boPreview = boVideoPreview || (g_bAniFlag & ANI_VIDEO_PREVIEW);
	MP_DEBUG("ShowFirstFrame %d, %d", _boPreview, boVideoPreview);
	while (try_count--)
	{
		sh_audio_t *sh_audio = graph->demux->sh_audio;

		sh_video_t *sh_video = (sh_video_t *) graph->demux->sh_video;
		AUDIO_DEVICE *audio_dev = &graph->a_out->audio_dev;
		if((sh_video->disp_w > MAX_VIDEO_DECODE_WIDTH) && (sh_video->disp_h>MAX_VIDEO_DECODE_HEIGHT))
			return FAIL;

		//try_count=0;
		

		/* pure audio */
		if (sh_audio && !sh_video)
			return PASS;

		/*============== Play Video & A/V Sync adjustmenet =======================*/
		if (sh_video)
		{
			unsigned char *start = NULL;
			int in_size;
			float temp_next_frame_duration;
			mp_image_t video_mpi_f;

			// Read next video frame & decode it
			in_size = read_one_valid_video_frame(filter_graph.demux, &start);
			if (in_size <= 0)
				continue;

			if (graph->v_decoder->video_decoder.decode)
			{
				bflag_get_img =
				    decode_video(&video_mpi_f, 0, graph->demux->sh_video, start, in_size, 0);
				#if (0 == PREVIEW_FLAT_VIDEO_FRAME)	
				if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PREVIEW)
                {
			        ST_IMGWIN extra_win;
				    ST_IMGWIN srcwin;
				    ST_IMGWIN trgwin;
			//mpDebugPrint("trgwin->wWidth = %d, trgwin->wHeight = %d", trgwin->wWidth, trgwin->wHeight);			
			        int non_zero_num = 0;		

			        srcwin.pdwStart = video_mpi_f.planes[0];
		            srcwin.dwOffset = video_mpi_f.display_width << 1;
		            srcwin.wWidth   = video_mpi_f.display_width;
		            srcwin.wHeight  = video_mpi_f.display_height;										
		
			        trgwin.wWidth = 16;
				    trgwin.wHeight = 12;
				    
			        extra_win.pdwStart = mem_malloc(trgwin.wWidth * trgwin.wHeight * 2);
					if (NULL == extra_win.pdwStart)
					{
					    return FAIL;
					}
			        extra_win.dwOffset = trgwin.wWidth * 2;
			        extra_win.wWidth   = trgwin.wWidth;
			        extra_win.wHeight  = trgwin.wHeight;
					
                    if (MJPG_FOURCC(four_cc)
                    {
                        srcwin.wType = 422;	
						Ipu_ImageScaling(&srcwin, &extra_win, 0, 0, srcwin.wWidth, srcwin.wHeight, 0, 0, trgwin.wWidth, trgwin.wHeight, 0);	
                    }
					#if (CHIP_VER_MSB != CHIP_VER_615)
					else
					{
					    image_scale_mpv2mem(&srcwin, &extra_win, 0, 0, srcwin.wWidth, srcwin.wHeight, 0, 0, trgwin.wWidth, trgwin.wHeight, 0);	
					}
					#endif
				    non_zero_num = Edge_Detection(extra_win.pdwStart, trgwin.wWidth, trgwin.wHeight);
					mpDebugPrint("non_zero_num = %d", non_zero_num);
					mem_free(extra_win.pdwStart);
				    if ((100 * non_zero_num / (trgwin.wWidth * trgwin.wHeight)) < 1)
					    bflag_get_img = 0;
					
				}	
			    #endif
				
				if (bflag_get_img == 1)
				{
					if (_boPreview)
					{
						if (!boVideoPreview)
						{
							MP_DEBUG("boVideoPreview stop in ShowFirstFrame");
							return FAIL;
						}
						display_video_thumb (screen_win, movie_win, &video_mpi_f, 1);
					}
					else
						display_video(&video_mpi_f, 1);
				}
			}

#if Dump_Img_Enable
			DRIVE *sDrv;
			STREAM *shandle;
			int ret,i = 2;;
			char *s,t[4];
			strcpy(t,"000");
			//Select the pack_no(First_I_frame) u want
			int First_I_frame = 0, pack_no = graph->demux->video->pack_no;
			sDrv = DriveGet(SD_MMC);

			if (First_I_frame<graph->demux->video->pack_no)
			{
				while (pack_no>0)
				{
					t[i]=pack_no%10+48;
					pack_no=(pack_no-pack_no%10)/10;
					i--;
				}

				strcpy(s,"source_");
				strcpy(&s[7],t);
				MP_DEBUG("dump source pack_no %d",graph->demux->video->pack_no);

				ret = CreateFile(sDrv, s, "raw");
				if (ret) UartOutText("create file fail\r\n");

				shandle = FileOpen(sDrv);
				if (!shandle) UartOutText("open file fail\r\n");

				ret = FileWrite(shandle, start, in_size);
				if (!ret) UartOutText("write file fail\r\n");

				FileClose(shandle);

				strcpy(s,"decode_");
				strcpy(&s[7],t);
				MP_DEBUG("dump decode pack_no %d",graph->demux->video->pack_no);
				ret = CreateFile(sDrv, s, "raw");
				if (ret) UartOutText("create file fail\r\n");

				shandle = FileOpen(sDrv);
				if (!shandle) UartOutText("open file fail\r\n");

				ret = FileWrite(shandle, video_mpi_f.planes[0], 1024*480*2);
				if (!ret) UartOutText("write file fail\r\n");

				FileClose(shandle);
				UartOutText("file close\r\n");
			}
#endif

		}						// if (sh_video)

		if (bflag_get_img == 1 || bflag_get_img == -4)
			break;
	}

	if (bflag_get_img == -4)
	{

		return FAIL;
	}


	if (try_count <= 0)
	{
		return FAIL;
	}
	return PASS;
}

/**
 * Decode one audio frame to audio PCM buffer queue
 *
 * @retval 0 success
 * @retval -1 error
 */
static int decode_audio_to_queue(register av_player_data_t * const player_data)
{
	AUDIO_BUF *audio_buf;
	filter_graph_t * const filter_graph = player_data->filter_graph;
	int data_size;
	DWORD buf_num,release;
	int pts;
#if DECODE_AUDIO_ON_DECODING_VIDEO
	static int non_decoding_video_cnt;
#endif
	sh_audio_t * const sh_audio = (sh_audio_t *) filter_graph->demux->sh_audio;
	sh_video_t * const sh_video = (sh_video_t *) filter_graph->demux->sh_video;

	//mpDebugPrint("audio in buffer size: %d, audio in buffer len: %d", sh_audio->a_in_buffer_size, sh_audio->a_in_buffer_len);

	AUDIO_DEVICE * const audio_dev = &filter_graph->a_out->audio_dev;

	buf_num = audio_dev->GetAudioSpace();

#if (CHIP_VER_MSB == CHIP_VER_615)
	EventWait(AV_DECODER_EVENT, EVENT_V_NOT_DECODING, OS_EVENT_OR, &release);
#endif
	//mpDebugPrint("%s @ %d buf_num=%d", __FUNCTION__, __LINE__, buf_num);
	MP_DEBUG("audio_dev->ABuf_Playing2Free_Num %d", audio_dev->ABuf_Playing2Free_Num);
	MP_DEBUG("audio_dev->ABuf_Ready_Num %d", audio_dev->ABuf_Ready_Num);
	MP_DEBUG("audio_dev->ABuf_Playing_Num %d", audio_dev->ABuf_Playing_Num);
	MP_DEBUG("audio_dev->ABuf_Free_Num %d", audio_dev->ABuf_Free_Num);
	MP_DEBUG("sh_audio->ds->eof %d", sh_audio->ds->eof);
#if AUDIO_ON
	extern DWORD PcmChunkCounter;
	if (PcmChunkCounter == 0)
	{
		if (sh_audio->ds->eof && audio_dev->ABuf_Ready_Num >0)
		{
			EventSet(AVP_SYNC_EVENT, EVENT_A_START_PLAY);
			filter_graph->a_out->audio_dev.playCfg.preloadBufNum = audio_dev->ABuf_Ready_Num;
			return PASS;
		}
	}
#endif

#if DEMO_PID
	if ((audio_dev->ABuf_Free_Num + audio_dev->ABuf_Ready_Num + audio_dev->ABuf_Playing_Num + audio_dev->ABuf_Playing2Free_Num)<35)
	{
		MP_DEBUG("Total buffer %d",audio_dev->ABuf_Free_Num + audio_dev->ABuf_Ready_Num + audio_dev->ABuf_Playing_Num + audio_dev->ABuf_Playing2Free_Num);
		return PASS;
	}
	else if (!buf_num)
#else
	 if (!buf_num)
#endif
	{
		MP_DEBUG("%s @ %d buf_num=%d", __FUNCTION__, __LINE__, buf_num);
		return WAIT_FOR_PLAY_RETVAL;
	}

#if DECODE_AUDIO_ON_DECODING_VIDEO
	if (buf_num > 0 && !hantro_decoding_video) // if buf_num > 0 but not decoding video for consecutive 3 tmes, then decode video
		non_decoding_video_cnt++;		// otherwise, we start decoding audio while video HW is decoding
	while (buf_num > 0 && (hantro_decoding_video || non_decoding_video_cnt == 3))
	{
		non_decoding_video_cnt = 0;
#else
	while (buf_num > 0)
	{
#endif
		/* fetch one audio buffer from audio device, since all audio buffer are controlled by audio device */
		audio_buf = audio_dev->GetAudioBuf_D();

		if (audio_buf == NULL) return PASS;

		/* decoded the compressed audio data into audio buffer */

		EventClear(AV_DECODER_EVENT, ~EVENT_A_NOT_DECODING);
		#if AUDIO_ON
		if (g_FileType == FILE_TYPE_ASF)
		{
			if (AudioGetDevicePTS() >= audio_delay_time)
			data_size = Decode_audio(sh_audio,
		                         	(unsigned char *) (audio_buf->buf),
		                         	AUDIO_BUF_SIZE, AUDIO_BUF_SIZE2);
			else
			{
				memset((unsigned char *) (audio_buf->buf),0,AUDIO_BUF_SIZE/64);
				data_size = AUDIO_BUF_SIZE/64;
			}		
		}
		else
			data_size = Decode_audio(sh_audio,
		                         	(unsigned char *) (audio_buf->buf),
		                         	AUDIO_BUF_SIZE, AUDIO_BUF_SIZE2);
		#endif
		EventSet(AV_DECODER_EVENT, EVENT_A_NOT_DECODING);

		//if(data_size < 0 && AO_AV == AV_TYPE && !sh_video->ds->eof && sh_audio->ds->eof)
		// For backward issue . Jump back to menu.
		if (data_size < 0 && AO_AV == AV_TYPE && !sh_video->ds->eof)
		{
			data_size = AUDIO_BUF_SIZE2;

			if (sh_audio->samplesize == 2)
				memset(audio_buf->buf, 0, data_size);
			else
				memset(audio_buf->buf, 0x80, data_size);
		}
		else if (data_size < 0)
		{
			/* decoded error */
			if (AO_AV == AO_TYPE)
				sh_audio->ds->eof = 1;
			else if (AO_AV == AV_TYPE)
				sh_audio->ds->eof = 1;

			if (sh_audio->ds->eof)
				MP_DEBUG("audio decoder eof");//want to check if audio decoder error,check in decode_audio
			else
				MP_ALERT("audio decoder error");

			audio_buf->offset = 0;

			audio_dev->AudioBuf2Ready_D(audio_buf);

#if 1//(AAC_SW_VIDEO ||MP3_SW_VIDEO ||AC3_VIDEO_ENABLE||WMA_SW_VIDEO)
			if (sh_audio->ds->eof)
			{
				//if(AO_AV == AO_TYPE)
				return FAIL;
				//if((AO_AV == AV_TYPE) && (sh_video->ds->eof))
				//	return FAIL;
				//else
				//	return 0;
			}
			else
				return 0;
#else
			return FAIL;
#endif
		}else if(data_size>0)
		{
			#if AUDIO_ON
			//extern DWORD PcmChunkCounter;
			if (PcmChunkCounter == 0)
			{
				EventSet(AVP_SYNC_EVENT, EVENT_A_START_PLAY);
			}
			#endif
		}

		sh_audio->delay += (float) data_size / ((float) ((audio_dev->m_Config).nBytes));

		audio_buf->offset = data_size;	/* the uncompressed audio data size in bytes */
		buf_num--;
		/* append this frame with pts to audio buffer */
		frame_entry_t a_entry = {NULL, player_data->audio_pts, NULL, NULL, audio_buf, 0};
		frame_entry_t_init_instance(&a_entry);
		player_data->a_decoded_fifo.add(&player_data->a_decoded_fifo, &a_entry);
		EventSet(AVP_SYNC_EVENT, EVENT_A_DECODED);

		/* prepare PTS for next audio frame */
		player_data->uncalc_audio_data_in_pts += audio_buf->offset * 1000;
		pts = player_data->uncalc_audio_data_in_pts / (audio_dev->m_Config).nBytes;
		player_data->uncalc_audio_data_in_pts -= pts * (audio_dev->m_Config).nBytes;
		player_data->audio_pts += pts;

		DWORD release;
		if (EventPolling(AV_DECODER_EVENT, EVENT_A_DECODER_START, OS_EVENT_OR, &release)==OS_STATUS_POLLING_FAILURE)
			{
				EventClear(AVP_SYNC_EVENT, ~EVENT_A_DECODED);
				break;
			}
		if (EventPolling(AV_DECODER_EVENT, EVENT_A_DECODER_EOF, OS_EVENT_OR, &release)==OS_STATUS_OK)
			{
				return FAIL;
			}
	}

	return PASS;
}

static void append_v_decoded_fifo(const demuxer_t* const demuxer, av_player_data_t * const player_data, const int frame_no)
{
	const pts_entry_t p_entry = pts_for_display_buffer(demuxer);
	//mpDebugPrintN("get (%d,%s) ", p_entry.pts, CODING_TYPE(p_entry.picture_coding_type_d));pts_fifo.printall(&pts_fifo);
	MP_DEBUG("avsync.c picId = %d pts1000=%d", player_data->video_mpi[frame_no].picId, FLOAT1000(player_data->video_mpi[frame_no].pts));
	/* append this frame with pts to video buffer */
	frame_entry_t f_entry = {NULL, p_entry.pts, NULL, NULL, &player_data->video_mpi[frame_no], frame_no};
	frame_entry_t_init_instance(&f_entry);
	player_data->v_decoded_fifo.add(&player_data->v_decoded_fifo, &f_entry);
	
	EventSet(AVP_SYNC_EVENT, EVENT_V_DECODED);
	if (frame_no >= VIDEO_FRAME_QUEUE_NUMBER - 1)
			EventSet(AVP_SYNC_EVENT, EVENT_V_ENOUGH_TO_PLAY);

	//mpDebugPrint("v_decoded_fifo_count=%d, %s:%d", player_data->v_decoded_fifo.get_count(&player_data->v_decoded_fifo), __FUNCTION__, __LINE__);
}


#if VIDEO_650
void _read_video()
{
	unsigned char *start = NULL;
	const int in_size = read_one_valid_video_frame(filter_graph.demux, &start);
	const float pts = filter_graph.demux->video->current->pts;
	
	MP_DEBUG("in_size = %d", in_size);
	g_start = start;
	g_in_size = in_size;
	
	if (in_size >= 0)
	{
		last_duration = pts - g_pts; //reserve the frame duration for the last frames of h264
		MP_DEBUG("last_duration * 1000 = %d", FLOAT1000(last_duration));
		g_pts = pts;
	}
}

int read_video()
{
	//EventSet(VIDEO_DATA_EVENT, EVENT_VD_READ);
	_read_video();
	return 1;
}

static int decode_video_after_reading(register av_player_data_t * player_data)
{
	int frame_no;			//assign which video_mpi is free 1 or 2 for mp4
	IntDisable();
	for (frame_no = 0; frame_no < player_data->max_video_buffer; frame_no++)
	{
		if (!player_data->video_mpi_busy[frame_no])
		{
			//mpDebugPrint("set video_mpi_busy[%d] @ %d", frame_no, __LINE__);
			player_data->video_mpi_busy[frame_no] = 1;
			break;
		}
	}
	IntEnable();

	if (frame_no >= player_data->max_video_buffer)
	{
		EventSet(AVP_SYNC_EVENT, EVENT_V_WAIT_FOR_PLAY);
		return WAIT_FOR_PLAY_RETVAL;
	}

	DWORD release;
	//EventWait(VIDEO_DATA_EVENT, EVENT_VD_READ_COMPLETED, OS_EVENT_AND, &release);
	
	register filter_graph_t * const filter_graph = player_data->filter_graph;

	unsigned char * const start = g_start;
	const int in_size = g_in_size;
	float pts = g_pts;
	
	int decode_status;
	demuxer_t* const demuxer = filter_graph->demux;
	sh_video_t * const sh_video = demuxer->sh_video;
	const unsigned int four_cc = sh_video->format;

	if (g_in_size < 0)
	{
		g_pts += last_duration;
		pts = g_pts;
	}
	MP_DEBUG("stream pts * 1000 = %d", FLOAT1000(pts));

	player_data->video_pts = FLOAT1000(pts);
	sh_video->pts = pts;
	pts_entry_t pts_entry = {NULL, player_data->video_pts, NULL, NULL, control_video(sh_video, CMD_VD_PICTURE_CODING_TYPE_DISPLAY, NULL, start, in_size)};
	pts_entry_t_init_instance(&pts_entry);
	pts_fifo.add(&pts_fifo, &pts_entry);
	//mpDebugPrintN("add (%d,%s) ", pts_entry.pts, CODING_TYPE(pts_entry.picture_coding_type_d));pts_fifo.printall(&pts_fifo);

	if (in_size < 0)
	{
		MP_ALERT("video stream end @ %s @ %d", __FUNCTION__, __LINE__);
		if (H264_FOURCC(four_cc))
		{
			h264_input_end = 1;
			if (h264_output_end == 0)
				MP_DEBUG("h264_output_end == 0");
			else
			{
				MP_ALERT("1_Shouldn't go here");
				return -1;
			}
			MP_DEBUG("1_h264_output_end = %d", h264_output_end);
		}
		else
		{
			//mpDebugPrint("reset video_mpi_busy %d", __LINE__);
			player_data->video_mpi_busy[frame_no] = 0;
			return -1;
		}
	}
	else if (in_size==0)
	{
		//mpDebugPrint("reset video_mpi_busy %d", __LINE__);
		player_data->video_mpi_busy[frame_no] = 0;
		return 0;
	}

	/* 3. Drop unwanted video frame, according to video frame type */
	drop_video_frame = 0;


	/* 4. Decoder Video */
	decode_status = 0;
	if (!drop_video_frame)
	{
//		if (frame_no == g_iPlayFrameNo)
//			while (!Ipu_CheckComplete()) TaskYield();
//		const unsigned int TraceTime = GetSysTime();
		g_in_size = 0;

		if (H264_FOURCC(four_cc))
		{
			if (!h264_input_end)
			{
//				mpDebugPrint("!h264_input_end");
				decode_status = decode_video(&player_data->video_mpi[frame_no],
				                             frame_no,
				                             filter_graph->demux->sh_video,
				                             start, in_size,
				                             drop_video_frame);
			}
			else if (!h264_output_end) //no input but still have output in vd_h264.c, pass the last parameter with a particular value
			{
//				mpDebugPrint("!h264_output_end");
				drop_video_frame = 2;  //inform the decoder that there's no input anymore.
				decode_status = decode_video(&player_data->video_mpi[frame_no],
				                             frame_no,
				                             filter_graph->demux->sh_video,
				                             start, in_size,
				                             drop_video_frame);
			}
			else	MP_ALERT("2_Shouldn't go here");

			if (decode_status == -1 || decode_status == -4 || decode_status == -8)
			{
				//mpDebugPrint("decode_status == -1");
				h264_output_end = 1;
				return -1;
			}

		}
		else
		{
			MP_DEBUG("Not h264");
			decode_status = decode_video(&player_data->video_mpi[frame_no],
			                             frame_no,
			                             filter_graph->demux->sh_video,
			                             start, in_size,
			                             drop_video_frame);
		}
//		mpDebugPrint("decode_video %d", GetSysTime()-TraceTime);

		if (decode_status <= 0)
		{
			MP_DEBUG("decode_status <= 0");
			/* decoded error */
			//mpDebugPrint("reset video_mpi_busy[%d] @ %d", frame_no, __LINE__);
			player_data->video_mpi_busy[frame_no] = 0;
			if (decode_status < 0)	MP_ALERT("video decode error @ %s : %d", __FILE__, __LINE__);
			if (decode_status == 0)
			{
				const pts_entry_t p_entry = pts_fifo.glance_head(&pts_fifo);
				player_data->display_video_pts = p_entry.pts;
			}

			return 0;
		}

	}

	/* 5. Append to video frame buffer */

	if (decode_status > 0)
		append_v_decoded_fifo(demuxer, player_data, frame_no);

	return 0;
}

static int mpv_decode_video_to_queue(register av_player_data_t * const player_data)
{
	if (g_in_size <= 0)
		read_video();
	return decode_video_after_reading(player_data);
}
#endif

/**
 * Decode one video frame to video frame buffer queue
 *
 * @retval 0 success
 * @retval -1 error
 */
static int decode_video_to_queue(register av_player_data_t * const player_data)
{
	register filter_graph_t * const filter_graph = player_data->filter_graph;
	unsigned char *start = NULL;
	int in_size;
	int frame_no; 			//assign which video_mpi is free 1 or 2 for mp4
	int decode_status;
	float pts;
	demuxer_t* const demuxer = filter_graph->demux;
	sh_video_t * const sh_video = demuxer->sh_video;
	const unsigned int four_cc = sh_video->format;

#if VIDEO_650
	if (MJPG_FOURCC(four_cc))
	{
#endif
		#if AUDIO_ON
		player_data->av_timer_pts = AudioGetDevicePTS();
		#endif
		/* 1. Acquire one image for video frame */
		IntDisable();
		for (frame_no = 0; frame_no < player_data->max_video_buffer; frame_no++)
		{
			if (!player_data->video_mpi_busy[frame_no])
			{
				player_data->video_mpi_busy[frame_no] = 1;
				break;
			}
		}
		IntEnable();

		if (frame_no >= player_data->max_video_buffer)
		{
			EventSet(AVP_SYNC_EVENT, EVENT_V_WAIT_FOR_PLAY);
			return WAIT_FOR_PLAY_RETVAL;
		}

		/* 2. Read one encoded video frame data */
#ifdef MPEG1_DROP_B_FRAME
		MPEG1_FRAME_TYPE = 0;
#endif
#ifdef XVID_DROP_B_FRAME
		xvid_bframe = 0;
#endif

//MP_TRACE_TIME_START();
		//do
		{
			in_size = read_one_valid_video_frame(filter_graph->demux, &start);

			pts = filter_graph->demux->video->current->pts;
			player_data->video_pts = FLOAT1000(pts);

			pts_entry_t pts_entry = {NULL, player_data->video_pts, NULL, NULL, control_video(sh_video, CMD_VD_PICTURE_CODING_TYPE_DISPLAY, NULL, start, in_size)};
			pts_entry_t_init_instance(&pts_entry);
			MP_DEBUG("add video_pts=%d, type=%d", pts_entry.pts, pts_entry.picture_coding_type_d);
			pts_fifo.add(&pts_fifo, &pts_entry);
			if (in_size < 0)
			{
				MP_ALERT("read video fail at %d",__LINE__);
				player_data->video_mpi_busy[frame_no] = 0;
				return -1;
			}
			else if (in_size==0)
			{
				//		UartOutText("read video zero\r\n");
				player_data->video_mpi_busy[frame_no] = 0;
				return 0;
			}

			//if (!MJPG_FOURCC(four_cc))
			//	break;
		}
		//while (player_data->video_pts < player_data->av_timer_pts);
#if Dump_Img_Enable
		DRIVE *sDrv;
		STREAM *shandle;
		int ret,i=2;
		char *s,t[4];
		strcpy(t,"000");
		//Select the pack_no(First_I_frame) u want
		int	First_I_frame=280,pack_no=filter_graph->demux->video->pack_no;
		sDrv=DriveGet(SD_MMC);

		if (First_I_frame<filter_graph->demux->video->pack_no)
		{

			while (pack_no>0)
			{
				t[i]=pack_no%10+48;
				pack_no=(pack_no-pack_no%10)/10;
				i--;
			}
			strcpy(s,"source_000");
			strcpy(&s[7],t);
			MP_DEBUG("dump source pack_no %d",filter_graph->demux->video->pack_no);
			ret=CreateFile(sDrv, s, "raw");
			if (ret) UartOutText("create file fail\r\n");

			shandle=FileOpen(sDrv);
			if (!shandle) UartOutText("open file fail\r\n");

			ret=FileWrite(shandle, start, in_size);
			if (!ret) UartOutText("write file fail\r\n");

			FileClose(shandle);
			UartOutText("file close\r\n");
		}
#endif

#if (CHIP_VER_MSB == CHIP_VER_612)
		TaskYield();
#endif
		/* 3. Drop unwanted video frame, according to video frame type */
		drop_video_frame = 0;

#ifdef MPEG1_DROP_B_FRAME
		if (MPEG1_FRAME_TYPE == 3)
		{
			player_data->video_mpi_busy[frame_no] = 0;
			frame_num++;
			drop_video_frame = 1;
		}
#endif
		
		/* 4. Decoder Video */
		decode_status = 0;
		if (!drop_video_frame)
		{
			decode_status = decode_video(&player_data->video_mpi[frame_no],
			                             frame_no,
			                             sh_video,
			                             start, in_size,
			                             drop_video_frame);
			if (decode_status <=0)
			{
				/* decoded error */
				player_data->video_mpi_busy[frame_no] = 0;
				if (decode_status < 0)
					MP_ALERT("video decode error @ %s : %d", __FILE__, __LINE__);
				return 0;
			}

#if Dump_Img_Enable
			if (First_I_frame<filter_graph->demux->video->pack_no)
			{
				strcpy(s,"decode_000");
				strcpy(&s[7],t);
				MP_DEBUG("dump decoded pack_no %d",filter_graph->demux->video->pack_no);
				ret=CreateFile(sDrv, s, "raw");
				if (ret) UartOutText("create file fail\r\n");

				shandle=FileOpen(sDrv);
				if (!shandle) UartOutText("open file fail\r\n");

				mp_image_t *test;
				if (frame_no)
				{
					test=&player_data->video_mpi[frame_no];
					ret=FileWrite(shandle, test->planes[0], 1024*480*2);
				}
				else
					ret=FileWrite(shandle, player_data->video_mpi->planes[0], 1024*480*2);

				if (!ret) UartOutText("write file fail\r\n");

				FileClose(shandle);
				UartOutText("\r\nfile close\r\n");
			}
#endif
		}


		/* 5. Append to video frame buffer */
#ifdef XVID_DROP_B_FRAME
		if (xvid_bframe)
		{
			player_data->video_mpi_busy[frame_no] = 0;
			frame_num++;
			drop_video_frame = 1;
		}

		if (decode_status > 0 && !xvid_bframe)
#else
		if (decode_status > 0)
#endif
		{
			append_v_decoded_fifo(demuxer, player_data, frame_no);
		}

		return 0;
#if VIDEO_650
	}
	else
	{
		return mpv_decode_video_to_queue(player_data);
	}
#endif
}
#if 0
static void AudioTaskSleep()
{
	if (filter_graph.a_out->audio_dev.GetAudioSpace() <16)
		TaskSleep(4);
}
#endif
/*!
decode audio while movie play.
@note audio only is NOT using this task
*/
void AudioDecodeTask(void)
{
	av_player_data_t * const player_data = &av_player_data;
	DWORD release;
	int ret = 1;
	static short AudioTaskYieldCount=0;
	while (1)
	{
		EventWait(AV_DECODER_EVENT, EVENT_A_DECODER_START, OS_EVENT_OR, &release);
		ret = decode_audio_to_queue(player_data);
		if (ret == FAIL)
		{
			EventClear(AV_DECODER_EVENT, ~EVENT_A_DECODER_START);
			EventSet(AV_DECODER_EVENT, EVENT_A_DECODER_EOF);
			EventSet(AVP_SYNC_EVENT, EVENT_AV_EOF);
			#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
				mpDebugPrint("%s sent EVENT_AV_EOF", __FUNCTION__);
			#endif
		}
		else if (ret == WAIT_FOR_PLAY_RETVAL)
		{
			#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_OTHER)
				mpDebugPrint("wait EVENT_A_PLAYED %s #%d", __FUNCTION__, __LINE__);
			#endif
			EventClear(AVP_SYNC_EVENT, ~EVENT_A_PLAYED);
			EventWait(AVP_SYNC_EVENT, EVENT_A_PLAYED, OS_EVENT_OR, &release);
			EventClear(AVP_SYNC_EVENT, ~EVENT_A_PLAYED);
			#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_OTHER)
				mpDebugPrint("continue EVENT_A_PLAYED %s #%d", __FUNCTION__, __LINE__);
			#endif
		}
#if (CHIP_VER_MSB == CHIP_VER_615)
		EventWait(AV_DECODER_EVENT, EVENT_V_NOT_DECODING, OS_EVENT_OR, &release);
#endif
		#if DEMO_PID
		if (AudioTaskYieldCount>100)
		{
			MP_DEBUG("%s :  @ %d", __FUNCTION__, __LINE__);
			TaskSleep(1);
			AudioTaskYieldCount = 0;
			MP_DEBUG("%s :  @ %d", __FUNCTION__, __LINE__);
		}
		else
			TaskYield();
		AudioTaskYieldCount++;
		#else
		TaskYield ();
		#endif
	}
}

/*!
decode video while movie play.
*/
void VideoDecodeTask(void)
{
	av_player_data_t * const player_data = &av_player_data;
	int ret = 1;
	DWORD release;

	while (1)
	{
		EventWait(AV_DECODER_EVENT, EVENT_V_DECODER_START, OS_EVENT_OR, &release);
		EventClear(AV_DECODER_EVENT, ~EVENT_V_NOT_DECODING);
		ret = decode_video_to_queue(player_data);
		EventSet(AV_DECODER_EVENT, EVENT_V_NOT_DECODING);
		//mpDebugPrint("%s @ %d ret=%d", __FUNCTION__, __LINE__, ret);
		if (ret < 0)
		{
			EventClear(AV_DECODER_EVENT, ~EVENT_V_DECODER_START);
			EventSet(AV_DECODER_EVENT, EVENT_V_DECODER_EOF);
			EventSet(AVP_SYNC_EVENT, EVENT_AV_EOF);
			#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_EVENT)
				mpDebugPrint("%s sent EVENT_AV_EOF", __FUNCTION__);
			#endif
		}
		else if (ret == WAIT_FOR_PLAY_RETVAL)
		{
			#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_OTHER)
				mpDebugPrint("wait EVENT_V_PLAYED %s #%d", __FUNCTION__, __LINE__);
			#endif
			EventClear(AVP_SYNC_EVENT, ~EVENT_V_PLAYED);
			EventWait(AVP_SYNC_EVENT, EVENT_V_PLAYED, OS_EVENT_OR, &release);
			EventClear(AVP_SYNC_EVENT, ~EVENT_V_PLAYED);
			#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_OTHER)
				mpDebugPrint("continue EVENT_V_PLAYED %s #%d", __FUNCTION__, __LINE__);
			#endif
		}
		
		TaskYield();
	}
}

void VideoDataTask(void)
{
	DWORD release;
	while (1)
	{
		EventWait(VIDEO_DATA_EVENT, EVENT_VD_READ, OS_EVENT_OR, &release);
#if VIDEO_650
		_read_video();
#endif
		EventSet(VIDEO_DATA_EVENT, EVENT_VD_READ_COMPLETED);
	}
}


int GetTotalVideoFrames(void)
{
	return av_player_data.stats.total_frames;
}

int GetMissedVideoFrames(void)
{
	return av_player_data.stats.missed_frame;
}
int GetVideoDisplayPTS(void)
{
	return av_player_data.display_video_pts;
}

#endif
