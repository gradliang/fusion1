/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      aac_hdr.h
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

#ifndef _AAC_HDR_H_
#define _AAC_HDR_H_

int mp_get_aac_header(unsigned char* hbuf,int* chans, int* freq);

#define mp_decode_aac_header(hbuf)  mp_get_aac_header(hbuf,NULL,NULL)

static inline int 
mp_check_aac_header(unsigned int head)
{
  if(mp_decode_aac_header((unsigned char*)(&head))<=0) 
    return 0;
  
  return 1;
}

#endif
