/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      ademuxmgr.h
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

#ifndef __DEMUXMGR__
#define __DEMUXMGR__ 

#include "demux_types.h"

void demuxmgr_init();
demuxer_t* demuxmgr_select(stream_t*, const DEMUX_T demux_type);
void free_all_demuxes();

#endif

