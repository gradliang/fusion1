/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      aviprint.c
*
* Programmer:    Deming Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*        
* Change History (most recent first):
*     <1>     03/30/2005    Deming Li    first file
****************************************************************
*/

#include "global612.h"
#if VIDEO_ON

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "mpTrace.h"

// for avi_stream_id():
#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux.h"

#include "avi\aviheader.h"

#include "mmreg.h"
#include "avi\avifmt.h"
#include "vfw.h"


void print_avih_flags(MainAVIHeader * h)
{
	MP_DPF("MainAVIHeader.dwFlags: (%ld)%s%s%s%s%s%s\n", h->dwFlags,
		   (h->dwFlags & AVIF_HASINDEX) ? " HAS_INDEX" : "",
		   (h->dwFlags & AVIF_MUSTUSEINDEX) ? " MUST_USE_INDEX" : "",
		   (h->dwFlags & AVIF_ISINTERLEAVED) ? " IS_INTERLEAVED" : "",
		   (h->dwFlags & AVIF_TRUSTCKTYPE) ? " TRUST_CKTYPE" : "",
		   (h->dwFlags & AVIF_WASCAPTUREFILE) ? " WAS_CAPTUREFILE" : "",
		   (h->dwFlags & AVIF_COPYRIGHTED) ? " COPYRIGHTED" : "");
}

void print_avih(MainAVIHeader * h)
{
	MP_DPF("======= AVI Header =======\n");
	MP_DPF("us/frame: %ld  (fps=%5.3f)\n", h->dwMicroSecPerFrame,
		   1000000.0f / (float) h->dwMicroSecPerFrame);
	MP_DPF("max bytes/sec: %ld\n", h->dwMaxBytesPerSec);
	MP_DPF("padding: %ld\n", h->dwPaddingGranularity);
	print_avih_flags(h);
	MP_DPF("frames  total: %ld   initial: %ld\n", h->dwTotalFrames, h->dwInitialFrames);
	MP_DPF("streams: %ld\n", h->dwStreams);
	MP_DPF("Suggested BufferSize: %ld\n", h->dwSuggestedBufferSize);
	MP_DPF("Size:  %ld x %ld\n", h->dwWidth, h->dwHeight);
	MP_DPF("==========================\n");
}

void print_strh(AVIStreamHeader * h)
{
	MP_DPF("====== STREAM Header =====\n");
	MP_DPF("Type: %.4s   FCC: %.4s (%X)\n", (char *) &h->fccType, (char *) &h->fccHandler,
		   (unsigned int) h->fccHandler);
	MP_DPF("Flags: %ld\n", h->dwFlags);
	MP_DPF("Priority: %d   Language: %d\n", h->wPriority, h->wLanguage);
	MP_DPF("InitialFrames: %ld\n", h->dwInitialFrames);
	MP_DPF("Rate: %ld/%ld = %5.3f\n", h->dwRate, h->dwScale,
		   (float) h->dwRate / (float) h->dwScale);
	MP_DPF("Start: %ld   Len: %ld\n", h->dwStart, h->dwLength);
	MP_DPF("Suggested BufferSize: %ld\n", h->dwSuggestedBufferSize);
	MP_DPF("Quality %ld\n", h->dwQuality);
	MP_DPF("Sample size: %ld\n", h->dwSampleSize);
	MP_DPF("==========================\n");
}

void print_wave_header(WAVEFORMATEX * h)
{
	MP_DPF("======= WAVE Format =======\n");
	MP_DPF("Format Tag: %d (0x%X)\n", h->wFormatTag, h->wFormatTag);
	MP_DPF("Channels: %d\n", h->nChannels);
	MP_DPF("Samplerate: %ld\n", h->nSamplesPerSec);
	MP_DPF("avg byte/sec: %ld\n", h->nAvgBytesPerSec);
	MP_DPF("Block align: %d\n", h->nBlockAlign);
	MP_DPF("bits/sample: %d\n", h->wBitsPerSample);
	MP_DPF("cbSize: %d\n", h->cbSize);
	if (h->wFormatTag == 0x55 && h->cbSize >= 12)
	{
		MPEGLAYER3WAVEFORMAT *h2 = (MPEGLAYER3WAVEFORMAT *) h;

		MP_DPF("mp3.wID=%d\n", h2->wID);
		MP_DPF("mp3.fdwFlags=0x%lX\n", h2->fdwFlags);
		MP_DPF("mp3.nBlockSize=%d\n", h2->nBlockSize);
		MP_DPF("mp3.nFramesPerBlock=%d\n", h2->nFramesPerBlock);
		MP_DPF("mp3.nCodecDelay=%d\n", h2->nCodecDelay);
	}
	else if (h->cbSize > 0)
	{
		int i;
		BYTE *p = ((BYTE *) h) + sizeof(WAVEFORMATEX);

		MP_DPF("Unknown extra header dump: ");
		for (i = 0; i < h->cbSize; i++)
			MP_DPF("[%x] ", *(p + i));
		MP_DPF("\n");
	}
	MP_DPF("===========================\n");
}


void print_video_header(BITMAPINFOHEADER * h)
{
	MP_DPF("======= VIDEO Format ======\n");
	MP_DPF("  biSize %d\n", h->biSize);
	MP_DPF("  biWidth %d\n", h->biWidth);
	MP_DPF("  biHeight %d\n", h->biHeight);
	MP_DPF("  biPlanes %d\n", h->biPlanes);
	MP_DPF("  biBitCount %d\n", h->biBitCount);
	MP_DPF("  biCompression %d='%.4s'\n", h->biCompression, (char *) &h->biCompression);
	MP_DPF("  biSizeImage %d\n", h->biSizeImage);
	if (h->biSize > sizeof(BITMAPINFOHEADER))
	{
		int i;
		BYTE *p = ((BYTE *) h) + sizeof(BITMAPINFOHEADER);

		MP_DPF("Unknown extra header dump: ");
		for (i = 0; i < h->biSize - sizeof(BITMAPINFOHEADER); i++)
			MP_DPF("[%x] ", *(p + i));
		MP_DPF("\n");
	}
	MP_DPF("===========================\n");
}


void print_index(AVIINDEXENTRY * idx, int idx_size)
{
	int i;
	unsigned int pos[256];
	unsigned int num[256];

	for (i = 0; i < 256; i++)
		num[i] = pos[i] = 0;
	for (i = 0; i < idx_size; i++)
	{
		int id = avi_stream_id(idx[i].ckid);

		if (id < 0 || id > 255)
			id = 255;
		MP_DPF("%5d:  %.4s  %4X  %08X  len:%6ld  pos:%7d->%7.3f %7d->%7.3f\n", i,
			   (char *) &idx[i].ckid,
			   (unsigned int) idx[i].dwFlags, (unsigned int) idx[i].dwChunkOffset,
//      idx[i].dwChunkOffset+demuxer->movi_start,
			   idx[i].dwChunkLength,
			   pos[id], (float) pos[id] / 18747.0f, num[id], (float) num[id] / 23.976f);
		pos[id] += idx[i].dwChunkLength;
		++num[id];
	}
}
#endif
