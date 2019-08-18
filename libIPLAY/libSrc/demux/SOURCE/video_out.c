/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      vd_out.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:   Video output
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/
#include "global612.h"
#if (VIDEO_ON || AUDIO_ON)

#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux.h"

#include "stheader.h"
#include "video_out.h"

static video_out_t _video_out;

// This function should be provided in Video displayer driver.
// Recode it later!!!
int init_video_driver(video_out_t * func)
{
	return 1;
}

///
///@ingroup group2_FilterG
///@defgroup    group_video_out     Video Out
///

///@ingroup group_video_out
///@brief   This function will return the pointer of initialized video_out_t structure.
///         At current stage, nothing will been done.   
///@param   void* sh_video  The pointer of video stream header structure 
///
///@return  pointer of video_out_t structure for success, 0 for fail
///
video_out_t *init_best_video_out(void *sh_video)
{
	sh_video_t *sh_video_p = (sh_video_t *) sh_video;

	if (sh_video_p)
	{
		_video_out.vo_pts = 0;
		
		if (init_video_driver(&_video_out))
			return &_video_out;
	}
	return 0;
}

#endif
