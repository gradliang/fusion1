/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      aviheader.h
*
* Programmer:    Deming Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Deming Li    first file
****************************************************************
*/
#ifndef _aviheader_h
#define	_aviheader_h

//#include "config.h"	/* get correct definition WORDS_BIGENDIAN */
#include "bswap.h"

/*
 * Some macros to swap little endian structures read from an AVI file
 * into machine endian format
 */
#ifdef WORDS_BIGENDIAN
#define	le2me_MainAVIHeader(h) {					\
    (h)->dwMicroSecPerFrame = le2me_32((h)->dwMicroSecPerFrame);	\
    (h)->dwMaxBytesPerSec = le2me_32((h)->dwMaxBytesPerSec);		\
    (h)->dwPaddingGranularity = le2me_32((h)->dwPaddingGranularity);	\
    (h)->dwFlags = le2me_32((h)->dwFlags);				\
    (h)->dwTotalFrames = le2me_32((h)->dwTotalFrames);			\
    (h)->dwInitialFrames = le2me_32((h)->dwInitialFrames);		\
    (h)->dwStreams = le2me_32((h)->dwStreams);				\
    (h)->dwSuggestedBufferSize = le2me_32((h)->dwSuggestedBufferSize);	\
    (h)->dwWidth = le2me_32((h)->dwWidth);				\
    (h)->dwHeight = le2me_32((h)->dwHeight);				\
}

#define	le2me_AVIStreamHeader(h) {					\
    (h)->fccType = le2me_32((h)->fccType);				\
    (h)->fccHandler = le2me_32((h)->fccHandler);			\
    (h)->dwFlags = le2me_32((h)->dwFlags);				\
    (h)->wPriority = le2me_16((h)->wPriority);				\
    (h)->wLanguage = le2me_16((h)->wLanguage);				\
    (h)->dwInitialFrames = le2me_32((h)->dwInitialFrames);		\
    (h)->dwScale = le2me_32((h)->dwScale);				\
    (h)->dwRate = le2me_32((h)->dwRate);				\
    (h)->dwStart = le2me_32((h)->dwStart);				\
    (h)->dwLength = le2me_32((h)->dwLength);				\
    (h)->dwSuggestedBufferSize = le2me_32((h)->dwSuggestedBufferSize);	\
    (h)->dwQuality = le2me_32((h)->dwQuality);				\
    (h)->dwSampleSize = le2me_32((h)->dwSampleSize);			\
    le2me_RECT(&(h)->rcFrame);						\
}
#define	le2me_RECT(h) {							\
    (h)->left = le2me_16((h)->left);					\
    (h)->top = le2me_16((h)->top);					\
    (h)->right = le2me_16((h)->right);					\
    (h)->bottom = le2me_16((h)->bottom);				\
}
#define le2me_BITMAPINFOHEADER(h) {					\
    (h)->biSize = le2me_32((h)->biSize);				\
    (h)->biWidth = le2me_32((h)->biWidth);				\
    (h)->biHeight = le2me_32((h)->biHeight);				\
    (h)->biPlanes = le2me_16((h)->biPlanes);				\
    (h)->biBitCount = le2me_16((h)->biBitCount);			\
    (h)->biCompression = le2me_32((h)->biCompression);			\
    (h)->biSizeImage = le2me_32((h)->biSizeImage);			\
    (h)->biXPelsPerMeter = le2me_32((h)->biXPelsPerMeter);		\
    (h)->biYPelsPerMeter = le2me_32((h)->biYPelsPerMeter);		\
    (h)->biClrUsed = le2me_32((h)->biClrUsed);				\
    (h)->biClrImportant = le2me_32((h)->biClrImportant);		\
}
#define le2me_WAVEFORMATEX(h) {						\
    (h)->wFormatTag = le2me_16((h)->wFormatTag);			\
    (h)->nChannels = le2me_16((h)->nChannels);				\
    (h)->nSamplesPerSec = le2me_32((h)->nSamplesPerSec);		\
    (h)->nAvgBytesPerSec = le2me_32((h)->nAvgBytesPerSec);		\
    (h)->nBlockAlign = le2me_16((h)->nBlockAlign);			\
    (h)->wBitsPerSample = le2me_16((h)->wBitsPerSample);		\
    (h)->cbSize = le2me_16((h)->cbSize);				\
}
#define le2me_AVIINDEXENTRY(h) {					\
    (h)->ckid = le2me_32((h)->ckid);					\
    (h)->dwFlags = le2me_32((h)->dwFlags);				\
    (h)->dwChunkOffset = le2me_32((h)->dwChunkOffset);			\
    (h)->dwChunkLength = le2me_32((h)->dwChunkLength);			\
}
#define le2me_AVISTDIDXCHUNK(h) {\
    char c; \
    c = (h)->fcc[0]; (h)->fcc[0] = (h)->fcc[3]; (h)->fcc[3] = c;  \
    c = (h)->fcc[1]; (h)->fcc[1] = (h)->fcc[2]; (h)->fcc[2] = c;  \
    (h)->dwSize = le2me_32((h)->dwSize);  \
    (h)->wLongsPerEntry = le2me_16((h)->wLongsPerEntry);  \
    (h)->nEntriesInUse = le2me_32((h)->nEntriesInUse);  \
    c = (h)->dwChunkId[0]; (h)->dwChunkId[0] = (h)->dwChunkId[3]; (h)->dwChunkId[3] = c;  \
    c = (h)->dwChunkId[1]; (h)->dwChunkId[1] = (h)->dwChunkId[2]; (h)->dwChunkId[2] = c;  \
    (h)->qwBaseOffset = le2me_64((h)->qwBaseOffset);  \
    (h)->dwReserved3 = le2me_32((h)->dwReserved3);  \
}
#define le2me_AVISTDIDXENTRY(h)  {\
    (h)->dwOffset = le2me_32((h)->dwOffset);  \
    (h)->dwSize = le2me_32((h)->dwSize);  \
}
#else
#define	le2me_MainAVIHeader(h)	    /**/
#define le2me_AVIStreamHeader(h)    /**/
#define le2me_RECT(h)		    /**/
#define le2me_BITMAPINFOHEADER(h)   /**/
#define le2me_WAVEFORMATEX(h)	    /**/
#define le2me_AVIINDEXENTRY(h)	    /**/
#define le2me_AVISTDIDXCHUNK(h)     /**/
#define le2me_AVISTDIDXENTRY(h)     /**/
#endif


#endif

/////////////////////////////////////////////////////

typedef struct _avisuperindex_entry {
    uint64_t qwOffset;           // absolute file offset
    uint32_t dwSize;             // size of index chunk at this offset
    uint32_t dwDuration;         // time span in stream ticks
} avisuperindex_entry;

typedef struct _avistdindex_entry {
    uint32_t dwOffset;           // qwBaseOffset + this is absolute file offset
    uint32_t dwSize;             // bit 31 is set if this is NOT a keyframe
} avistdindex_entry;

// Standard index 
typedef struct __attribute((packed)) _avistdindex_chunk {
    char           fcc[4];       // ix##
    uint32_t  dwSize;            // size of this chunk
    uint16_t wLongsPerEntry;     // must be sizeof(aIndex[0])/sizeof(DWORD)
    uint8_t  bIndexSubType;      // must be 0
    uint8_t  bIndexType;         // must be AVI_INDEX_OF_CHUNKS
    uint32_t  nEntriesInUse;     // first unused entry
    char           dwChunkId[4]; // '##dc' or '##db' or '##wb' etc..
    uint64_t qwBaseOffset;       // all dwOffsets in aIndex array are relative to this
    uint32_t  dwReserved3;       // must be 0
    avistdindex_entry *aIndex;   // the actual frames
} avistdindex_chunk;
    

// Base Index Form 'indx'
typedef struct _avisuperindex_chunk {
    char           fcc[4];
    uint32_t  dwSize;                // size of this chunk
    uint16_t wLongsPerEntry;         // size of each entry in aIndex array (must be 4*4 for us)
    uint8_t  bIndexSubType;          // future use. must be 0
    uint8_t  bIndexType;             // one of AVI_INDEX_* codes
    uint32_t  nEntriesInUse;         // index of first unused member in aIndex array
    char       dwChunkId[4];         // fcc of what is indexed
    uint32_t  dwReserved[3];         // meaning differs for each index type/subtype.
                                     // 0 if unused
    avisuperindex_entry *aIndex;     // position of ix## chunks
    avistdindex_chunk *stdidx;       // the actual std indices
} avisuperindex_chunk;

/////////////////////////////////////////////////////

typedef struct {
  // index stuff:
  void* idx;
  int idx_size;
  unsigned int  idx_pos;
  unsigned int  idx_pos_a;
  unsigned int  idx_pos_v;
  unsigned int  idx_pos_v_3D;
  unsigned int  idx_offset;  // ennyit kell hozzaadni az index offset ertekekhez
  // bps-based PTS stuff:
  int video_pack_no;
  int audio_block_size;
  unsigned int audio_block_no;
  // interleaved PTS stuff:
  int skip_video_frames;
  int audio_streams;
  float avi_audio_pts;
  float avi_video_pts;
  float pts_correction;
  unsigned int pts_corr_bytes;
  unsigned char pts_corrected;
  unsigned char pts_has_video;
  unsigned int numberofframes;
  ////for super index support//////////////
  avisuperindex_chunk *suidx;
  int suidx_size;
  int isodml;

  ////////////////////////////////////
} avi_priv_t;

#define AVI_PRIV ((avi_priv_t*)(demuxer->priv))
#define AVI_IDX_OFFSET(x) ((((uint64_t)(x)->dwFlags&0xffff0000)<<16)+(x)->dwChunkOffset)

static inline int avi_stream_id(unsigned int id){
  unsigned char *p=(unsigned char *)&id;
  unsigned char a,b;
#if WORDS_BIGENDIAN
  a=p[3]-'0'; b=p[2]-'0';
#else
  a=p[0]-'0'; b=p[1]-'0';
#endif
  if(a>9 || b>9) return 100; // invalid ID
  return a*10+b;
}

