/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in
* deciphering, decoding, reverse engineering or in ay way altering the source
* code is strictly prohibited, unless the prior written consent of Magic
* Pixel is obtained.
*
* Filename      : filebrowser.h
* Programmers   :
* Created       :
* Descriptions  :
*******************************************************************************
*/
#ifndef __FILE_BROWSER_H
#define __FILE_BROWSER_H

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#include "display.h"
#include "mpapi.h"
#include "xpg.h"


/*
// Type definitions
*/

enum {
    PHOTO_MODE = 0,
    MUSIC_MODE,
    VIDEO_MODE,
    FILE_MODE,
    WORD_MODE,
    NUM_OF_MODE
};

typedef struct {
    volatile CHAIN stStackBufferBackup[NUM_OF_MODE][DIR_STACK_DEEP];
    volatile WORD wDirStackPointBackup[NUM_OF_MODE];
} ST_DIR_STACK_INFO;


/*
// Variable declarations
*/
STXPGBROWSER *g_pstBrowser;

/*
// Constant declarations
*/

//Lighter add for draw page scroll bar 0109
/*
#define Page_Scroll_Bar_Start_X 1760
#define Page_Scroll_Bar_Start_Y 220
#define Page_Scroll_Bar_Width 20
#define Page_Scroll_Bar_High 600
*/

/*
// Static function prototype
*/

/*
// Definition of external functions
*/
extern SWORD FileChangeDirAndResearch(ST_SEARCH_INFO * psSearchList);
extern SWORD FileBrowserScanFileList(BYTE bSearchType);
extern SWORD FileBrowserGetFileName(ST_SEARCH_INFO * psSearchList, BYTE * pbNameBuffer, DWORD dwBufferSize, BYTE * pbExtensionBuffer);
extern DRIVE *FileBrowserGetCurDrive(void);
extern BYTE *FileBrowserGetCurExt(void);
extern DWORD FileBrowserGetCurIndex(void);
extern ST_SEARCH_INFO *FileGetCurLyricInfo(void);
extern ST_SEARCH_INFO *FileGetCurSearchInfo(void);
extern ST_SEARCH_INFO *FileGetSearchInfo(DWORD dwIndex);
extern int FileBrowserGetImgEXIF_DateTime(ST_SEARCH_INFO * pSearchInfo, DATE_TIME_INFO_TYPE * exif_date_time);
#if 0
extern id3_tag_t *FileBrowserGetId3Tag(ST_SEARCH_INFO * pSearchInfo);
#endif
extern ST_SEARCH_INFO *FileBrowserSearchNextAudio(int iOffset);
extern ST_SEARCH_INFO *FileBrowserSearchNextImage(int iOffset);
extern WORD FileGetMediaType(const char * bExt);
extern DWORD FileListGetCurIndex(void);
extern DWORD FileListSetCurIndex(SWORD value);
extern DWORD FileListAddCurIndex(SWORD value);
extern SWORD FileBrowserCopyFile(ST_SEARCH_INFO * pSrcSearchInfo, BYTE bSrcDrvID, BYTE bTrgDrvID);
extern SWORD FileBrowserDeleteFile(void);
void   FileBrowserListAllFileName(void);

void fbrowser_xpgClearCatchFunctionRegister(void *funcPtr);
void fbrowser_xpgUpdateIconAniFunctionRegister(void *funcPtr);
void fbrowser_xpgChangeDriveFunctionRegister(void *funcPtr);

#endif //__FILE_BROWSER_H


