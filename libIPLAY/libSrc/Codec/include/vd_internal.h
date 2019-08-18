/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      video_internal.h
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
#ifndef __VD_INTERNAL_H__
#define __VD_INTERNAL_H__

#include "video_decoder.h"

// prototypes:
static vd_info_t info;
static int control(sh_video_t *sh,int cmd,void* arg,...);
static int init(sh_video_t *sh, int image_count);
static void uninit(sh_video_t *sh);
static int change_mode(sh_video_t *sh);
static int decode(mp_image_t  *mpi, int image_no, sh_video_t *sh, void* data, int len, int flags);

#define LIBVD_EXTERN(x) vd_functions_t mpcodecs_vd_##x = {\
	&info,\
	init,\
    uninit,\
	control,\
	change_mode,\
	decode\
};


#endif


