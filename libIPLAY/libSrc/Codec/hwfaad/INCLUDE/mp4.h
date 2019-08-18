/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mp4.h
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
#ifndef __MP4_H__
#define __MP4_H__

#include "decoder.h"

int8_t FAADAPI AudioSpecificConfig(BYTE *pBuffer,
                                   uint32_t buffer_size,
                                   mp4AudioSpecificConfig *mp4ASC);

int8_t FAADAPI AudioSpecificConfig2_faad(BYTE *pBuffer,
                                    uint32_t buffer_size,
                                    mp4AudioSpecificConfig *mp4ASC,
                                    program_config *pce);

#endif
