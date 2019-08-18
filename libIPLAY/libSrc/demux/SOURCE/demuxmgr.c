/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demuxmgr.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: demux management
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

#include "demuxmgr.h"

/* File format specific demux */
#if (VIDEO_ENABLE || MJPEG_ENABLE)
extern demuxer_t *new_avi_demux();
extern demuxer_t *new_mov_demux();
#endif
#if VIDEO_ENABLE
extern demuxer_t *new_mpeg_demux();
extern demuxer_t *new_ts_demux();
extern demuxer_t *new_flv_demux();
#endif

// Total number of demuxes. Increase it when new demux is added
#if VIDEO_ENABLE
#define DEMUX_COUNT  8
#else
#define DEMUX_COUNT  2
#endif

#pragma alignvar(4)
static demuxer_t *demuxes[DEMUX_COUNT];

///
///@ingroup group1_Demux
///@brief   For each supported file foramt, this function will init its demuxer_t structure,
///             and record the pointer of each file format's demuxer_t structure in demuxes[].
///
///@param   No
///
///@return  No
///
void demuxmgr_init()
{
	int i = 0;
#if (VIDEO_ENABLE || MJPEG_ENABLE)
	demuxes[i++] = new_avi_demux();
	demuxes[i++] = new_mov_demux();
#endif	
#if VIDEO_ENABLE    
	demuxes[i++] = new_mpeg_demux();
	demuxes[i++] = new_ts_demux();
	demuxes[i++] = new_flv_demux();
	demuxes[i++] = new_asf_demux();
	demuxes[i++] = new_h264_demux();
	demuxes[i++] = new_h263_demux();
#endif
}

///@ingroup group1_Demux
///@brief   Free all demuxer_t structure recorded in array demuxes[] 
///@param   none
///@return  No
void free_all_demuxes()
{
	MP_DEBUG("free_all_demuxes()");
	int i = 0;

	for (i = 0; i < DEMUX_COUNT; i++)
	{
		if (demuxes[i])
			free_demuxer(demuxes[i]);
		demuxes[i] = NULL;
	}

	video_free_buffer();
}

///@ingroup group1_Demux
///@brief   This function will select the proper demuxer_t structure from demuxes[] by calling its check_type function.
///@param   stream_t* stream
///@param   DEMUX_T demux_type
///@return  Pointer of the selected demuxer_t structure according to Param. demux_type.
demuxer_t *demuxmgr_select(stream_t * stream, const DEMUX_T demux_type)
{
	int i = 0;
	int tmp;
	reset_video_decoder();

	if (!stream)
	{
		MP_ALERT("DemuxMgrSelect - Stream Null");
		return 0;
	}
		
	if (demux_type == DEMUX_UNKNOWN)
	{
		MP_ALERT("DemuxMgrSelect - Unknow file type");		
		return 0;
	}

	while (i < DEMUX_COUNT)
	{
		if (demuxes[i] == NULL)
		{
			i++;
			continue;
		}

		if ((tmp = demuxes[i]->check_type(stream, demux_type)) == 1)
		{
			if (demuxes[i]->open())
				return demuxes[i];
			else
			{
				MP_ALERT("open failed while check_type ok %s %s : %d", __FUNCTION__, __FILE__, __LINE__);
				demuxes[i]->close();
				i++;
			}
		}
		else if (tmp == 0)
			i++;
		else	// demux checked, not its type
		{
			MP_ALERT("DemuxMgrSelect - File type search fail");
			return 0;
		}

	}

	return 0;
}
#endif
