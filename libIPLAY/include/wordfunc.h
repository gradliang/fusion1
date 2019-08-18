#ifndef __WORD_FUNC__H__
#define __WORD_FUNC__H__

#if EREADER_ENABLE

#include "bitsDefine.h"
#include "global612.h"
#include "mpTrace.h"

typedef struct{
	BYTE bStatusFlag;
	char page_name[21];
	BYTE bName_len;
	void (*hCommand)(void);
}ST_WORD_ACTFUNC;

typedef struct{
	BYTE bStatusFlag;
	DWORD (*hGetTotalIndex)(void);
	DWORD (*hGetCurrentIndex)(void);
	void (*hSetListIndex)(DWORD);
	void (*hSetFirstListIndex)(DWORD);
	void (*hSetCurrentIndex)(DWORD);
	
}ST_WORD_BTN;


#define EBOOK_TYPE_DOC		1
#define EBOOK_TYPE_XML		2
#define EBOOK_TYPE_PDF		3
#define EBOOK_TYPE_EPUB		4
#define EBOOK_TYPE_RTF		5
#define EBOOK_TYPE_LRF		6
#define EBOOK_TYPE_PGM      7 //for test CJ
#define EBOOK_TYPE_PPM      8 //for test CJ
#define EBOOK_TYPE_PDB      9 
#define EBOOK_TYPE_TXT      10 
#define EBOOK_TYPE_UNKNOWN	0xFF

#endif
#endif

