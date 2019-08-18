#ifndef AVI_UTILITY_H
#define AVI_UTILITY_H

#include "global612.h"



#define    PCMCACHE_NUM  4

typedef struct
{
	DWORD imgSz;
	DWORD pcmSz;
	DWORD samplerate;
	DWORD samplesize;
}AVIFILEINFO;

typedef struct
{
    AVIFILEINFO *aviInfo;
    STREAM      *pFile;
	STREAM      *index_handle;
	DWORD        index_count;
    STREAM      *aFile;
    STREAM      *vFile;
    BYTE        *jbuf;
    BYTE        *abuf;
    DWORD       pcmNum;
    DWORD       aTimestampCnt;          // Unit : 0.1 mini second
    DWORD       vTimestampCnt;          // Unit : 0.1 mini second
    DWORD       aTimestamp;             // Unit : 0.1 mini second
    DWORD       vTimestamp;             // Unit : 0.1 mini second
} AVIFILE;

#endif
