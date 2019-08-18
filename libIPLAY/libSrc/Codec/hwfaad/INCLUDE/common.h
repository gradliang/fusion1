/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      common.h
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Brenda Li    first file
****************************************************************
*/
#ifndef __COMMON_H__
#define __COMMON_H__

#define INLINE __inline

#include <stdio.h>
# include <string.h>
#include "UtilTypeDef.h"

/* common functions */
BYTE get_sr_index_faad(uint32_t samplerate);
uint32_t get_sample_rate_faad(BYTE sr_index);

#endif
