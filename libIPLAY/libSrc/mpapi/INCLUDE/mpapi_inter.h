/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mpapi_inter.h
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

#ifndef MPAPI_INTER_H
#define MPAPI_INTER_H
#include "display.h"
#include "fs.h"
#include "stream.h"
#include "config_codec.h"
#include "mpfifo.h"


typedef struct
{
	int total_frames;
	int missed_frame;
	int total_missed_time;
} av_player_stats_t;

typedef struct av_player_data_t_
{
	filter_graph_t *filter_graph;

	frame_fifo_t a_decoded_fifo;
	frame_fifo_t v_decoded_fifo;
	frame_fifo_t v_display_fifo;

	int max_video_buffer;//liwu: not necessary, use VIDEO_FRAME_QUEUE_NUMBER instead
	mp_image_t video_mpi[VIDEO_FRAME_QUEUE_NUMBER];
	int video_mpi_busy[VIDEO_FRAME_QUEUE_NUMBER];	//busy flag for video_mpi[VIDEO_FRAME_QUEUE_NUMBER]

	int32_t av_timer_pts;		/* in millisecond */
	uint32_t display_video_pts;	/* in millisecond */
	uint32_t audio_pts;			/* in millisecond */
	uint32_t video_pts;			/* in millisecond */	//player_data->video_pts = (unsigned int)(filter_graph->demux->video->pts * 1000.0f);
	uint32_t uncalc_audio_data_in_pts;	/* the remainder part of audio_pts */

	av_player_stats_t stats;

	void	(*init)	(struct av_player_data_t_ * const player_data);
	void	(*flush)(struct av_player_data_t_ * const player_data);
} av_player_data_t;

void MovieTask_Main();
void MovieTask_Control(int instruc);
void MovieTask_Close(void);
int MovieTask_Init(STREAM * sHandle, int type, ST_IMGWIN * screen_win, ST_IMGWIN * movie_win);
void MovieTask_StopAudio();

void MoviePlayTask();
void MovieControl(int instruc);
void MovieClose( stream_t* stream);

#endif // MPAPI_INTER_H

