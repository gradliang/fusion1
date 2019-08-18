/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_packet.h
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

#ifndef __DEMUX_PACKET_H
#define __DEMUX_PACKET_H

#include <string.h>
#include "demux_types.h"
#include "global612.h"

inline static demux_packet_t* new_demux_packet(const int len)
{
	demux_packet_t* dp=(demux_packet_t*)mem_malloc(sizeof(demux_packet_t));
	memset(dp, 0, sizeof(demux_packet_t));
	dp->len=len;
	dp->refcount=1;
	dp->buffer=len?(unsigned char*)mem_malloc(len+MP_INPUT_BUFFER_PADDING_SIZE):NULL;
	if (len) memset(dp->buffer+len,0,8);
	return dp;
}

inline static void resize_demux_packet(demux_packet_t* const dp, const int len)
{
	if (len)
	{
		if (dp->len==len)	return;
		dp->buffer=(unsigned char *)mem_reallocm(dp->buffer,len+MP_INPUT_BUFFER_PADDING_SIZE);
		memset(dp->buffer+len,0,8);
	}
	else
	{
		if (dp->buffer)
		{
			mem_free(dp->buffer);
			dp->buffer=NULL;
		}
	}
	dp->len=len;
}

inline static demux_packet_t* clone_demux_packet(demux_packet_t* pack)
{
	demux_packet_t* dp=(demux_packet_t*)mem_malloc(sizeof(demux_packet_t));
	while (pack->master) pack=pack->master; // find the master
	memcpy(dp,pack,sizeof(demux_packet_t));
	dp->next=NULL;
	dp->refcount=0;
	dp->master=pack;
	pack->refcount++;
	return dp;
}

inline static void free_demux_packet(demux_packet_t* const dp)
{
	if (dp->master==NULL)   //dp is a master packet
	{
		dp->refcount--;
		if (dp->refcount==0)
		{
			if (dp->buffer) mem_free(dp->buffer);
			mem_free(dp);
		}
		return;
	}
	// dp is a clone:
	mpDebugPrint("crash: no way dp is a clone %s:%d",__FILE__,__LINE__);
	TimerDelay(2000);
	__asm("break 100");
	free_demux_packet(dp->master);
	mem_free(dp);
}

#endif

