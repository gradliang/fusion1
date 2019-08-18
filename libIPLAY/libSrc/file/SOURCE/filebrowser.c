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
* Filename      : filebrowser.c
* Programmers   :
* Created       :
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#include "devio.h"
#include "display.h"
#include "mpapi.h"
#include "flagdefine.h"
#include "xpg.h"
#include "filebrowser.h"

#if SAVE_FILE_TO_SPI
#include "..\..\..\..\MiniDV\main\INCLUDE\SPI_Ex.h"
#include "..\..\..\..\MiniDV\main\INCLUDE\xpgFunc.h"

extern BOOL  g_boSaveFileToSPI;
#endif


/*
// Variable declarations
*/
#if (XPG_ENABLE == 0)
  STXPGMOVIE g_stXpgMovie;
  STXPGMOVIE *g_pstXpgMovie = (STXPGMOVIE *) &g_stXpgMovie;
#else
  extern STXPGMOVIE g_stXpgMovie;
  extern STXPGMOVIE *g_pstXpgMovie;
#endif


#pragma alignvar(4)

STXPGBROWSER g_stBrowser[OP_TOTAL_MODE];
STXPGBROWSER *g_pstBrowser = (STXPGBROWSER *) &g_stBrowser;

typedef void (*FILEBROWSER_UI_CLEAR_CATCH_CALLBACK)(void);
typedef BOOL (*FILEBROWSER_UI_UPDATE_ICON_ANI_CALLBACK)(void);
typedef BOOL (*FILEBROWSER_UI_CHANGE_DRIVE_CALLBACK)(E_DRIVE_INDEX_ID);

static FILEBROWSER_UI_CLEAR_CATCH_CALLBACK       fbrowser_xpgClearCatchFuncPtr = NULL;
static FILEBROWSER_UI_UPDATE_ICON_ANI_CALLBACK   fbrowser_xpgUpdateIconAniFuncPtr = NULL;
static FILEBROWSER_UI_CHANGE_DRIVE_CALLBACK      fbrowser_xpgChangeDriveFuncPtr = NULL;


void fbrowser_xpgClearCatchFunctionRegister(void *funcPtr)
{
    fbrowser_xpgClearCatchFuncPtr = (FILEBROWSER_UI_CLEAR_CATCH_CALLBACK) funcPtr;
}

void fbrowser_xpgUpdateIconAniFunctionRegister(void *funcPtr)
{
    fbrowser_xpgUpdateIconAniFuncPtr = (FILEBROWSER_UI_UPDATE_ICON_ANI_CALLBACK) funcPtr;
}

void fbrowser_xpgChangeDriveFunctionRegister(void *funcPtr)
{
    fbrowser_xpgChangeDriveFuncPtr = (FILEBROWSER_UI_CHANGE_DRIVE_CALLBACK) funcPtr;
}


#if (XPG_ENABLE == 0)
STXPGSPRITE *xpgSpriteFindType(register STXPGMOVIE * pstMovie, DWORD dwType, DWORD dwIndex)
{
    return NULL;
}


void xpgUpdateStageClip(ST_IMGWIN * pWin, register WORD left, register WORD top,
                        register WORD right, register WORD bottom, BOOL boCopyToCurrWin)
{

}
#endif


///
///@ingroup FILE_BROWSER
///@brief   Reset/clean all the GUI file lists.
///
///@param   None.
///
///@return  None.
///
void FileBrowserResetFileList(void)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
    DWORD i;

    for (i = 0; i < 4; i++)
    {
        psFileBrowser->dwFileListIndex[i] = 0;
        psFileBrowser->dwFileListCount[i] = 0;
        psFileBrowser->dwFileListAddress[i] = 0;
    }

    psFileBrowser->dwSearchFileCount = 0;
    MpMemSet((BYTE *) &psFileBrowser->sSearchFileList, 0, sizeof(ST_SEARCH_INFO) * FILE_LIST_SIZE);

    psFileBrowser->dwAudioTotalFile = 0;
    psFileBrowser->dwImgAndMovTotalFile = 0;
#if EREADER_ENABLE
    psFileBrowser->dwEbookTotalFile = 0;
#endif

#if (((BT_PROFILE_TYPE & BT_FTP_SERVER) == BT_FTP_SERVER) || \
    ((BT_PROFILE_TYPE & BT_FTP_CLIENT) == BT_FTP_CLIENT)) //rick for BT FTP
    psFileBrowser->dwValid = 0;
#endif
}


///
///@ingroup FILE_BROWSER
///@brief   Get the DRIVE handle of current drive.
///
///@param   None.
///
///@return  The DRIVE handle of current drive.
///
///@remark  If DRIVE handle of current drive is NULL, this function call will reset/clean all GUI file lists
///         and report an ERR_REMOVE_CURDEV event before it returns.
///
DRIVE *FileBrowserGetCurDrive(void)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    BYTE bDriveId = psSysConfig->sStorage.dwCurStorageId;
    psFileBrowser->pCurDrive = DriveGet(bDriveId);
    if (psFileBrowser->pCurDrive == NULL)
    {
        FileBrowserResetFileList();
        SystemSetErrEvent(ERR_REMOVE_CURDEV);
    }

    return psFileBrowser->pCurDrive;
}


///
///@ingroup FILE_BROWSER
///@brief   Check the drive present status of current drive and get its DRIVE handle.
///
///@param   None.
///
///@return  This function call will return the DRIVE handle of current drive if the drive is present.
///         Otherwise, it will return NULL if the drive is not present.
///
///@remark  This function call is very similar to FileBrowserGetCurDrive(): \n
///         If DRIVE handle of current drive is NULL, this function call will reset/clean all GUI file lists
///         and report an ERR_REMOVE_CURDEV event before it returns.
///
DRIVE *FileBrowserCheckCurDrive(void)
{
    register ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    register BYTE bDriveId = psSysConfig->sStorage.dwCurStorageId;


    if ((bDriveId == NULL_DRIVE) || (bDriveId >= MAX_DRIVE_NUM))
    {
        psFileBrowser->pCurDrive = NULL;
    }
    else
    {
        psFileBrowser->pCurDrive = DriveGet(bDriveId);
        if (psFileBrowser->pCurDrive->Flag.Present)
            return psFileBrowser->pCurDrive;
        else
            psFileBrowser->pCurDrive = NULL;
    }

    if (psFileBrowser->pCurDrive == NULL)
    {
        FileBrowserResetFileList();
        SystemSetErrEvent(ERR_REMOVE_CURDEV);
    }

    return NULL;
}


///
///@ingroup FILE_BROWSER
///@brief   Initialize the file extension name array content within file browser for supported media file types of the specified GUI operation mode.
///
///@param   dwOpMode    GUI file browser operation mode.
///
///@return  None.
///
void FileBrowserInitExtArray(DWORD dwOpMode)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    DWORD i;

    i = 0;
    switch (dwOpMode)
    {
        case OP_IMAGE_MODE: //image files
            psFileBrowser->dwFileExtArray[i++] = EXT_JPG;	//JPG
            psFileBrowser->dwFileExtArray[i++] = EXT_BMP;	//BMP

#if (SONY_DCF_ENABLE)
            psFileBrowser->dwFileExtArray[i++] = EXT_JPE;	//JPE
            psFileBrowser->dwFileExtArray[i++] = EXT_ARW;	//ARW
            psFileBrowser->dwFileExtArray[i++] = EXT_SRF;	//SRF
            psFileBrowser->dwFileExtArray[i++] = EXT_SR2;	//SR2
            psFileBrowser->dwFileExtArray[i++] = EXT_JFI;	//JFI
            psFileBrowser->dwFileExtArray[i++] = EXT_THM;	//THM
#endif

#if GIF
            psFileBrowser->dwFileExtArray[i++] = EXT_GIF;	//GIF
#endif

#if PNG
            psFileBrowser->dwFileExtArray[i++] = EXT_PNG;	//PNG
#endif

#if TIFF
            psFileBrowser->dwFileExtArray[i++] = EXT_TIFF;	//TIF
#endif

#if MPO
            psFileBrowser->dwFileExtArray[i++] = EXT_MPO;   //MPO
#endif
            psFileBrowser->dwFileExtArray[i++] = EXT_END;	//####
            break;

        case OP_AUDIO_MODE: //audio files
#if (MP3_SW_AUDIO | MP3_MAD_AUDIO | MP3_HW_CODEC)        
            psFileBrowser->dwFileExtArray[i++] = EXT_MP3;	//MP3
#endif            
#if (WMA_ENABLE && WMA_AV)
            psFileBrowser->dwFileExtArray[i++] = EXT_WMA;	//WMA
#endif

#if (OGG_ENABLE)
            psFileBrowser->dwFileExtArray[i++] = EXT_OGG;	//OGG
#endif

#if(AAC_SW_AUDIO || AAC_FAAD_AUDIO)
            psFileBrowser->dwFileExtArray[i++] = EXT_AAC;	//AAC
            psFileBrowser->dwFileExtArray[i++] = EXT_M4A;	//M4A
#endif

#if (AC3_AUDIO_ENABLE && AC3_AV)
            psFileBrowser->dwFileExtArray[i++] = EXT_AC3;	//AC3
#endif

#if (RAM_AUDIO_ENABLE)
            psFileBrowser->dwFileExtArray[i++] = EXT_RA;	//RA
            psFileBrowser->dwFileExtArray[i++] = EXT_RM;	//RM
            psFileBrowser->dwFileExtArray[i++] = EXT_RAM;	//RAM
#endif

#if WAV_ENABLE
            psFileBrowser->dwFileExtArray[i++] = EXT_WAV;	//WAV
#endif
            psFileBrowser->dwFileExtArray[i++] = EXT_AMR;	//AMR
            psFileBrowser->dwFileExtArray[i++] = EXT_END;	//####
            break;

        case OP_MOVIE_MODE: //video files
#if VIDEO_ON
            psFileBrowser->dwFileExtArray[i++] = EXT_AVI;	//AVI
            psFileBrowser->dwFileExtArray[i++] = EXT_MOV;	//MOV
 #if VIDEO_ENABLE
            psFileBrowser->dwFileExtArray[i++] = EXT_MPG;	//MPG
            psFileBrowser->dwFileExtArray[i++] = EXT_DAT;	//MPG
            psFileBrowser->dwFileExtArray[i++] = EXT_VOB;	//MPG
            psFileBrowser->dwFileExtArray[i++] = EXT_ASF;	//ASF
            psFileBrowser->dwFileExtArray[i++] = EXT_MP4;	//MP4
            psFileBrowser->dwFileExtArray[i++] = EXT_3GP;	//3GP
            psFileBrowser->dwFileExtArray[i++] = EXT_MPE;	//MPE (include MPEG)
            psFileBrowser->dwFileExtArray[i++] = EXT_FLV;	//FLV
            psFileBrowser->dwFileExtArray[i++] = EXT_TS;		//TS
            psFileBrowser->dwFileExtArray[i++] = EXT_MTS;	//TS
            psFileBrowser->dwFileExtArray[i++] = EXT_M2TS;	//TS
               psFileBrowser->dwFileExtArray[i++] = EXT_h264;	
            psFileBrowser->dwFileExtArray[i++] = EXT_h263;	
          // psFileBrowser->dwFileExtArray[i++] = EXT_MKV;	//MKV
 #endif
#endif
            psFileBrowser->dwFileExtArray[i++] = EXT_END;	//####
            break;

#if EREADER_ENABLE
        case OP_EBOOK_MODE:
            psFileBrowser->dwFileExtArray[i++] = EXT_DOC;
            psFileBrowser->dwFileExtArray[i++] = EXT_TXT;
            psFileBrowser->dwFileExtArray[i++] = EXT_XML;
            psFileBrowser->dwFileExtArray[i++] = EXT_RTF;
            psFileBrowser->dwFileExtArray[i++] = EXT_PDF;
            psFileBrowser->dwFileExtArray[i++] = EXT_EPUB;
            psFileBrowser->dwFileExtArray[i++] = EXT_PPM;
            psFileBrowser->dwFileExtArray[i++] = EXT_PGM;
            psFileBrowser->dwFileExtArray[i++] = EXT_LRF;
            psFileBrowser->dwFileExtArray[i++] = EXT_PDB;
            break;
#endif

        case OP_FILE_MODE:
            /* scan all files for OP_FILE_MODE */
            psFileBrowser->dwFileExtArray[0] = EXT_ALL;     //scan all files
            psFileBrowser->dwFileExtArray[1] = EXT_END;     //####
            break;
    }
}


///
///@ingroup FILE_BROWSER
///@brief   Scan files on the specified drive according to specified searching type for supported media types of current GUI operation
///         mode, and add them to corresponding file list.
///
///@param   pCurDrive     The DRIVE handle of the drive to scan files. \n\n
///@param   bSearchType   There are four searching types (GLOBAL_SEARCH, LOCAL_RECURSIVE_SEARCH, LOCAL_SEARCH, LOCAL_SEARCH_INCLUDE_FOLDER)
///                       to setup this parameter:\n
///                         GLOBAL_SEARCH: searching all the files on the disk drive.\n
///                         LOCAL_RECURSIVE_SEARCH: searching the files under current directory, including all the
///                                                 subdirectories under this directory.\n
///                         LOCAL_SEARCH: searching the files under current directory.\n
///                         LOCAL_SEARCH_INCLUDE_FOLDER: searching the files and folders under current directory.\n
///
///@retval  PASS               Scan files successfully. \n\n
///@retval  FAIL               Scan files unsuccessfully. \n\n
///@retval  INVALID_DRIVE      Invalid drive.
///
SWORD DoSearch(DRIVE * pCurDrive, BYTE bSearchType)
{
#define FILE_LIST_ENTRIES_THRESHOLD  256
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    struct ST_STORAGE_TAG *psStorage = &psSysConfig->sStorage;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    DWORD *pdwExtArray, dwMaxElement, *pdwTotalCount;
    BYTE bFileType;
    DWORD dwOpMode = psSysConfig->dwCurrentOpMode;


    MP_DEBUG("%s: bSearchType = %d", __FUNCTION__, bSearchType);
    if (pCurDrive == NULL 
#if SAVE_FILE_TO_SPI
			&& g_boSaveFileToSPI==0
#endif
			)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    DWORD oldFilesCount = psFileBrowser->dwSearchFileCount;

    // check if need to reset whole file lists
    dwMaxElement = FILE_LIST_SIZE - oldFilesCount;
    MP_DEBUG("%s: oldFilesCount = %u, left entries count (dwMaxElement) = %u", __FUNCTION__, oldFilesCount, dwMaxElement);

#if (SONY_DCF_ENABLE)
    if ((dwOpMode == OP_IMAGE_MODE) && (dwMaxElement < FILE_LIST_SONY_DCF_ENTRIES_THRESHOLD))
    {
        MP_ALERT("%s: oldFilesCount = %u, left entries (%u) < threshold (%u) => Reset whole list first ...", __FUNCTION__, oldFilesCount, dwMaxElement, FILE_LIST_SONY_DCF_ENTRIES_THRESHOLD);
#else
    if (dwMaxElement < FILE_LIST_ENTRIES_THRESHOLD)
    {
        MP_ALERT("%s: oldFilesCount = %u, left entries (%u) < threshold (%u) => Reset whole list first ...", __FUNCTION__, oldFilesCount, dwMaxElement, FILE_LIST_ENTRIES_THRESHOLD);
#endif
        FileBrowserResetFileList();
        oldFilesCount = 0;

#if (SONY_DCF_ENABLE)
        dwMaxElement = TEMP_FILE_LIST_MAX_ENTRIES_COUNT;
#else
        dwMaxElement = FILE_LIST_SIZE;
#endif
    }

    // always reset the file list of current GUI operation mode first, and then re-scan to update the file list
    psFileBrowser->dwFileListIndex[dwOpMode] = 0;
    psFileBrowser->dwFileListCount[dwOpMode] = 0;
    psFileBrowser->dwFileListAddress[dwOpMode] = 0;

    if (psFileBrowser->dwFileListAddress[dwOpMode] == 0)
    {
        FileBrowserInitExtArray(dwOpMode);

        psFileBrowser->dwFileListAddress[dwOpMode] = (DWORD) &psFileBrowser->sSearchFileList[oldFilesCount];

        bFileType = psFileBrowser->bFileType[dwOpMode];
        pdwExtArray = (DWORD *)&psFileBrowser->dwFileExtArray;
        pdwTotalCount = &psFileBrowser->dwFileListCount[dwOpMode];
        *pdwTotalCount = 0;

        psFileBrowser->dwFileListIndex[dwOpMode] = 0;

#if SAVE_FILE_TO_SPI
        if (g_boSaveFileToSPI)
        {
            int r;
            r = spifs_FileExtSearch(SPI_FILE_TYPE_JPG_FILE, (ST_SEARCH_INFO *)psFileBrowser->dwFileListAddress[dwOpMode], dwMaxElement, pdwTotalCount);
            if (r != FS_SUCCEED)
            {
                MP_ALERT("%s: FileExtSearch() failed !", __FUNCTION__);
                SystemSetErrMsg(ERR_FILE_SYSTEM);
                return FAIL;
            }
        }
        else
#endif
        {
            if (FileExtSearch(pCurDrive,
                              pdwExtArray,
                              (ST_SEARCH_INFO *)psFileBrowser->dwFileListAddress[dwOpMode],
                              dwMaxElement,
                              pdwTotalCount, bSearchType, bFileType, FILE_SORT_ENABLE) != FS_SUCCEED)
            {
                MP_ALERT("%s: FileExtSearch() failed !", __FUNCTION__);
                SystemSetErrMsg(ERR_FILE_SYSTEM);
                return FAIL;
            }
        }


        
#if (((BT_PROFILE_TYPE & BT_FTP_SERVER) == BT_FTP_SERVER)||\
    ((BT_PROFILE_TYPE & BT_FTP_CLIENT) == BT_FTP_CLIENT))//rick for BT FTP
        psFileBrowser->dwValid = 1;
#endif
        psFileBrowser->dwSearchFileCount += *pdwTotalCount;
    }

    // for compatible, so set to old vars
    switch (dwOpMode)
    {
        case OP_AUDIO_MODE:
            psFileBrowser->dwAudioCurIndex = psFileBrowser->dwFileListIndex[dwOpMode];
            psFileBrowser->dwAudioTotalFile = psFileBrowser->dwFileListCount[dwOpMode];
            psFileBrowser->sAudioFileList = (ST_SEARCH_INFO *)psFileBrowser->dwFileListAddress[dwOpMode];
            break;

#if EREADER_ENABLE
        case OP_EBOOK_MODE:
            psFileBrowser->dwEbookCurIndex = psFileBrowser->dwFileListIndex[dwOpMode];
            psFileBrowser->dwEbookTotalFile = psFileBrowser->dwFileListCount[dwOpMode];
            psFileBrowser->sEbookFileList = (ST_SEARCH_INFO *)psFileBrowser->dwFileListAddress[dwOpMode];
            break;
#endif

        default:
            psFileBrowser->dwImgAndMovCurIndex = psFileBrowser->dwFileListIndex[dwOpMode];
            psFileBrowser->dwImgAndMovTotalFile = psFileBrowser->dwFileListCount[dwOpMode];
            psFileBrowser->sImgAndMovFileList = (ST_SEARCH_INFO *)psFileBrowser->dwFileListAddress[dwOpMode];
            break;
    }

    //mpDebugPrint("-I- FileCount = %d dwOpMode=%d", psFileBrowser->dwFileListCount[dwOpMode],dwOpMode);
    return PASS;
}


///
///@ingroup FILE_BROWSER
///@brief   Change directory to the specified directory/folder and re-scan files under that directory for supported media types
///         of current GUI operation mode. Then update the corresponding file list.
///
///@param   psSearchList    The ST_SEARCH_INFO pointer of the directory/folder entry to enter and scan files.
///
///@retval  PASS                          Change directory and scan files successfully. \n\n
///@retval  FAIL                          Change directory and scan files unsuccessfully due to some error. \n\n
///@retval  FS_DIR_STACK_LIMIT_REACHED    Cannot enter subdirectory because depth limit of DirStack buffer has been reached.
///
///@remark   The directory to enter is implicitly pointed to by current node (drv->Node) before calling this function.
///          The file scanning in this function is performed by invoking DoSearch(pCurDrive, LOCAL_SEARCH_INCLUDE_FOLDER).
///
SWORD FileChangeDirAndResearch(ST_SEARCH_INFO * psSearchList)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    DRIVE *pCurDrive;
    FDB *ParentNode = NULL;
    int ret;


    MP_DEBUG("enter %s()...", __FUNCTION__);

    pCurDrive = FileBrowserCheckCurDrive();
    if (pCurDrive == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    if (fbrowser_xpgClearCatchFuncPtr)
        fbrowser_xpgClearCatchFuncPtr();

    // clear current mode list at first
    psSysConfig->sFileBrowser.dwFileListAddress[psSysConfig->dwCurrentOpMode] = 0;

    if (psSearchList == NULL)
    {
        MP_ALERT("%s: -E- psSearchList == NULL => DirReset(pCurDrive)...", __FUNCTION__);
        if (DirReset(pCurDrive) != FS_SUCCEED)
        {
            MP_ALERT("%s: DirReset() failed !", __FUNCTION__);
            return FAIL;
        }
    }
    else
    {
        // change dir
        if (psSearchList->bParameter & SEARCH_INFO_CHANGE_PATH)  // for use bParameter record invalid file
        {
            MP_DEBUG("%s: SEARCH_INFO_CHANGE_PATH => CdParent() ...", __FUNCTION__);
            CdParent(pCurDrive);
            ParentNode = (FDB *) (pCurDrive->Node);
            //if (pCurDrive->DirStackPoint != 0)	// not at root dir
            //    UpdateChain(pCurDrive);
        }
        else if (psSearchList->bParameter & SEARCH_INFO_FOLDER)
        {
            MP_DEBUG("%s: SEARCH_INFO_FOLDER => FileListOpen() ...", __FUNCTION__);
            STREAM *dirHandle = FileListOpen(pCurDrive, psSearchList);
            if (dirHandle != NULL)
            {
                FileClose(dirHandle);

                MP_DEBUG("%s: SEARCH_INFO_FOLDER => CdSub() ...", __FUNCTION__);
                ret = CdSub(pCurDrive);
                if (ret != FS_SUCCEED)
                {
                    if (ret == FS_DIR_STACK_LIMIT_REACHED)
                        return FS_DIR_STACK_LIMIT_REACHED;
                    else
                    {
                        SystemSetErrEvent(ERR_FILE_SYSTEM);
                        return FAIL;
                    }
                }
            }
            else
            {
                SystemSetErrEvent(ERR_FILE_SYSTEM);
                return FAIL;
            }
        }
    }

    // change dir clear all file list
    FileBrowserResetFileList();

    SWORD swRet = DoSearch(pCurDrive, LOCAL_SEARCH_INCLUDE_FOLDER);
#if 1
    if (swRet != FAIL && ParentNode != NULL)
    {
        ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
        ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
        DWORD dwOpMode = psSysConfig->dwCurrentOpMode;
        ST_SEARCH_INFO *pSearchInfo = (ST_SEARCH_INFO *)psFileBrowser->dwFileListAddress[dwOpMode];

        int i = psFileBrowser->dwFileListIndex[dwOpMode];
        int n = psFileBrowser->dwFileListCount[dwOpMode];

        for (i = 0; i < n; i++)
        {
            if (pSearchInfo->dwMediaInfo == 1)
            {
                psFileBrowser->dwFileListIndex[dwOpMode] = i;
                if (dwOpMode == OP_AUDIO_MODE)
                    psFileBrowser->dwAudioCurIndex = i;
                else
                {
#if EREADER_ENABLE
                    if (dwOpMode == OP_EBOOK_MODE)
                        psFileBrowser->dwEbookCurIndex = i;
                    else
#endif
                        psFileBrowser->dwImgAndMovCurIndex = i;
                }
                break;
            }
            pSearchInfo++;
        }
    }
#endif
    return swRet;
}


#if 0  /* this function is not suitable to use!  For lyric processing, please refer to .AVI and .SRT subtitle processing */
SWORD FileBrowserScanLyricList(void)
{
#if LYRIC_ENABLE
  #define BUF_SIZE 100
  #ifndef AUDIO_BUFFER_SIZE
    #define AUDIO_BUFFER_SIZE  400
  #endif

    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    struct ST_STORAGE_TAG *psStorage = &psSysConfig->sStorage;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    register ST_SEARCH_INFO *psSearchList;
    DWORD *pdwExtArray, dwMaxElement, *pdwTotalCount, dwLyricTotalCount,
    dwLyricExtArray[2], i, j, k, dwBuf1Count, dwBuf2Count;
    BYTE bExt[3] = "MP3", bCompareFlag, bLongNameFlag, bBuf1[BUF_SIZE], bBuf2[BUF_SIZE];

    ST_SEARCH_INFO sLyricSearchList[AUDIO_BUFFER_SIZE], *psLyricList;
    register DRIVE *pCurDrive = FileBrowserGetCurDrive();

    if (pCurDrive == NULL)
        return FAIL;

    MP_DEBUG1("-I- research pdwTotalCount = %d", *pdwTotalCount);
    if ((psSysConfig->dwCurrentOpMode == OP_AUDIO_MODE) || (psSysConfig->dwCurrentOpMode == OP_FILE_MODE))
    {
        pdwExtArray = &psFileBrowser->dwAudioExtArray[0];
        psSearchList = &psFileBrowser->sAudioFileList[0];
        pdwTotalCount = &psFileBrowser->dwAudioTotalFile;
        psLyricList = &psFileBrowser->sImgAndMovFileList[0];

        if (*pdwTotalCount != 0)
        {
            dwLyricExtArray[0] = 0x4C52432E;	//LRC.
            dwLyricExtArray[1] = EXT_END;	//####
            dwLyricTotalCount = 0;
            if (FileExtSearch(pCurDrive, dwLyricExtArray, &sLyricSearchList[0], dwMaxElement, &dwLyricTotalCount, SEARCH_TYPE, 0, FILE_SORT_ENABLE))
            {
                MP_ALERT("%s: FileExtSearch() failed !", __FUNCTION__);
                SystemSetErrMsg(ERR_FILE_SYSTEM);
                return FAIL;
            }

            for (i = 0; i < *pdwTotalCount; i++)
            {
                if ((psSearchList[i].bExt[0] == bExt[0])
                    && (psSearchList[i].bExt[1] == bExt[1]) && (psSearchList[i].bExt[2] == bExt[2]))
                {
                    bLongNameFlag = FALSE;
                    if (psSearchList[i].dwLongFdbCount != 0)
                    {
                        bLongNameFlag = TRUE;
                        if (FileGetLongName(pCurDrive, &psSearchList[i], &bBuf1[0], &dwBuf1Count, BUF_SIZE) != FS_SUCCEED)
                        {
                            MP_ALERT("%s: -E- FileGetLongName() failed.", __FUNCTION__);
                            break;
                        }
                    }

                    for (j = 0; j < dwLyricTotalCount; j++)
                    {
                        if (bLongNameFlag == FALSE)
                        {
                            if (sLyricSearchList[j].dwLongFdbCount == 0)
                            {
                                bCompareFlag = TRUE;
                                for (k = 0; k < 8; k++)
                                {
                                    if (psSearchList[i].bName[k] != sLyricSearchList[j].bName[k])
                                    {
                                        bCompareFlag = FALSE;
                                        break;
                                    }
                                }
                                if (bCompareFlag == TRUE)
                                {
                                    MpMemCopy(&psLyricList[i], &sLyricSearchList[j], sizeof(ST_SEARCH_INFO));
                                    break;
                                }
                            }
                        }
                        else
                        {
                            if (sLyricSearchList[j].dwLongFdbCount != 0)
                            {
                                if (FileGetLongName(pCurDrive, &sLyricSearchList[j], &bBuf2[0], &dwBuf2Count, BUF_SIZE) != FS_SUCCEED)
                                {
                                    MP_ALERT("%s: -E- FileGetLongName() failed.", __FUNCTION__);
                                    break;
                                }

                                if (dwBuf1Count == dwBuf2Count)
                                {
                                    bCompareFlag = TRUE;
                                    for (k = 0; k < (dwBuf1Count - 8); k++)
                                    {
                                        if (bBuf1[k] != bBuf2[k])
                                        {
                                            bCompareFlag = FALSE;
                                            break;
                                        }
                                    }
                                    if (bCompareFlag == TRUE)
                                    {
                                        MpMemCopy(&psLyricList[i], &sLyricSearchList[j], sizeof(ST_SEARCH_INFO));
                                        break;
                                    }
                                }
                            }
                        }

                        if (j >= dwLyricTotalCount)
                        {
                            MpMemSet(&psLyricList[i], 0x0, sizeof(ST_SEARCH_INFO));
                        }
                    }
                }
            }
        }
    }
#endif
}
#endif


///
///@ingroup FILE_BROWSER
///@brief   Scan files on current drive according to specified searching type for supported media types of current GUI operation
///         mode, and add them to corresponding file list.
///
///@param   bSearchType   There are four searching types (GLOBAL_SEARCH, LOCAL_RECURSIVE_SEARCH, LOCAL_SEARCH, LOCAL_SEARCH_INCLUDE_FOLDER)
///                       to setup this parameter:\n
///                         GLOBAL_SEARCH: searching all the files on the disk drive.\n
///                         LOCAL_RECURSIVE_SEARCH: searching the files under current directory,
///                                                 including all the subdirectories under this directory.\n
///                         LOCAL_SEARCH: searching the files under current directory.\n
///                         LOCAL_SEARCH_INCLUDE_FOLDER: searching the files and folders under current directory.\n
///
///@retval  PASS       Scan files successfully. \n\n
///@retval  FAIL       Scan files unsuccessfully.
///
///@remark   This function call actually invokes the DoSearch() function to perform files scanning job.
///
SWORD FileBrowserScanFileList(BYTE bSearchType)
{
#if SAVE_FILE_TO_SPI
    if (g_boSaveFileToSPI)
    {
        return DoSearch(NULL, bSearchType);
    }
#endif    
    DRIVE *pCurDrive = FileBrowserGetCurDrive();
    if (pCurDrive == NULL)
    {
        MP_ALERT("%s: NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    if (g_psSystemConfig->dwCurrentOpMode >= OP_TOTAL_MODE)
    {
        MP_ALERT("%s: Invalid OP mode %d !", __FUNCTION__, g_psSystemConfig->dwCurrentOpMode);
        return FAIL;
    }

    if (bSearchType == GLOBAL_SEARCH)
    {
        if (DirReset(pCurDrive) != FS_SUCCEED)
        {
            MP_ALERT("%s: DirReset() failed !", __FUNCTION__);
            return FAIL;
        }
    }

    if (pCurDrive->DirStackPoint != 0) // not at root dir
        UpdateChain(pCurDrive);

    return DoSearch(pCurDrive, bSearchType);
}


/* Print filenames of all entries in the FileBrowser file list of current GUI operation mode to console */
void FileBrowserListAllFileName(void)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    ST_SEARCH_INFO *psSearchList;
    ENTRY_FILENAME_TYPE  bFilenameType = E_FILENAME_UNKNOWN;
    BYTE  pbBuf[512];
    DWORD pdwBufferCount = 0;
    int ret;
    DWORD i, j;


    mpDebugPrint("%s: total count %d", __FUNCTION__, psFileBrowser->dwFileListCount[psSysConfig->dwCurrentOpMode]);
    for (i = 0; i < psFileBrowser->dwFileListCount[psSysConfig->dwCurrentOpMode]; i++)
    {
        psSearchList = (ST_SEARCH_INFO *) ((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[psSysConfig->dwCurrentOpMode] + i);
        ret = Locate_DirEntryNode_of_SearchInfo(DriveGet(psSearchList->DrvIndex), psSearchList);
        if ((ret != FS_SUCCEED) && (ret != FILE_NOT_FOUND))
        {
            mpDebugPrint("Locate_DirEntryNode_of_SearchInfo() failed !");
            break;
        }
        else if (ret == FILE_NOT_FOUND) /* here, use FILE_NOT_FOUND for [Upper Folder] case */
        {
            continue;
        }

        ret = GetFilenameOfCurrentNode(DriveGet(psSearchList->DrvIndex), &bFilenameType, pbBuf, &pdwBufferCount, 512);
        if (ret != FS_SUCCEED)
        {
            mpDebugPrint("Error ! GetFilenameOfCurrentNode() failed !");
            break;
        }

        if (bFilenameType == E_FILENAME_8_3_SHORT_NAME)
        {
            mpDebugPrint("index = %d , name = %s", i, pbBuf);
        }
        else if (bFilenameType == E_FILENAME_UTF16_LONG_NAME)
        {
            BYTE  tmpBuf[512]; /* for ASCII/UTF-8 string */
            MpMemSet(tmpBuf, 0, 512);
            mpx_UtilU16ToU08(tmpBuf, (WORD *) pbBuf); /* convert UTF-16 to UTF-8 string */
            mpDebugPrint("index = %d , name = %s\r\n", i, tmpBuf);
        }

#if EXFAT_ENABLE //extra fields for exFAT
        mpDebugPrint("(dwDirStart cluster=0x%x, dwDirPoint LBA=0x%x, dwParentDirSize=%lu, dwFdbOffset=%lu) \r\n", psSearchList->dwDirStart, psSearchList->dwDirPoint, psSearchList->dwParentDirSize, psSearchList->dwFdbOffset);
#else
        mpDebugPrint("(dwDirStart cluster=0x%x, dwDirPoint LBA=0x%x, dwFdbOffset=%lu) \r\n", psSearchList->dwDirStart, psSearchList->dwDirPoint, psSearchList->dwFdbOffset);
#endif
    }
    return;
}



#if (((BT_PROFILE_TYPE & BT_FTP_SERVER) == BT_FTP_SERVER) || \
    ((BT_PROFILE_TYPE & BT_FTP_CLIENT) == BT_FTP_CLIENT)) //rick for BT FTP
#define NOT_REMOVE_EXT  1
#else
#define NOT_REMOVE_EXT  0
#endif

///
///@ingroup FILE_BROWSER
///@brief   Get the filename of a file or directory/folder entry specified by the ST_SEARCH_INFO pointer.
///
///@param   psSearchList        [IN] The ST_SEARCH_INFO pointer of the file/folder entry to get its filename. \n\n
///@param   pbNameBuffer        [OUT] The buffer for output string of the filename of the file/folder entry. \n\n
///@param   dwBufferSize        [IN] Size of the 'pbNameBuffer' buffer. \n\n
///@param   pbExtensionBuffer   [OUT] The buffer for output string of the filename extension part of the file/folder entry.
///
///@retval  PASS      Get filename of the entry successfully. \n\n
///@retval  FAIL      Get filename of the entry unsuccessfully.
///
SWORD FileBrowserGetFileName(ST_SEARCH_INFO * psSearchList, BYTE * pbNameBuffer, DWORD dwBufferSize, BYTE * pbExtensionBuffer)
{
    DWORD tempIndex;
    DWORD i;

    DRIVE *pCurDrive = FileBrowserGetCurDrive();


    if ((psSearchList == NULL) || (pbNameBuffer == NULL) || (pbExtensionBuffer == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return FAIL;
    }

    if (pCurDrive == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    MpMemSet(pbNameBuffer, 0, dwBufferSize);
    MpMemSet(pbExtensionBuffer, 0, 4); /* only 4 bytes: for 3 characters + null terminator */

    if (psSearchList->bParameter & SEARCH_INFO_CHANGE_PATH) // for use bParameter record invalid file
    {
        BYTE *pbUF = "Upper Folder";

        MpMemCopy(pbNameBuffer, pbUF, StringLength08(pbUF));
        return PASS;
    }

    if (psSearchList->dwLongFdbCount == 0) /* short filename */
    {
        /* note: file names of valid SONY DCF objects are always short filenames */
#if (SONY_DCF_ENABLE)
        FILE_SORTING_BASIS_TYPE  sorting_method = GetCurFileSortingBasis();

        if ( (sorting_method == FILE_SORTING_BY_83SHORT_NAME) || (sorting_method == FILE_SORTING_BY_83SHORT_NAME_REVERSE) ||
             (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING) || (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE) )
        {
            /* if the file list entry is a DCF essential object, make up its DCF displayed name */
            DWORD file_list_index;
            if (Check_If_FileListEntry_In_DCF_Objs_Set(psSearchList, &file_list_index) == TRUE)
            {
                BYTE output_name_buffer[9];
                DWORD output_display_integer = 0;

                if (Makeup_DCF_Obj_DisplayName(psSearchList->bName, file_list_index, output_name_buffer, 9, &output_display_integer) == PASS)
                {
                    if (dwBufferSize >= 8)
                    {
                        if (dwBufferSize > 8)
                            MpMemCopy(pbNameBuffer, output_name_buffer, 9); /* include null terminator */
                        else
                            MpMemCopy(pbNameBuffer, output_name_buffer, 8); /* not include null terminator */

                        tempIndex = 8;
                        goto L_shortname_ext_processing;
                    }
                }
            }
        }
#endif

        for (tempIndex = 0; (tempIndex < 8) && (psSearchList->bName[tempIndex] != ' '); tempIndex++)
        {
            pbNameBuffer[tempIndex] = psSearchList->bName[tempIndex];
            if (psSearchList->bName[tempIndex] == ' ')
                break;
        }

L_shortname_ext_processing:
        if ( (psSearchList->bExt[0] != 0) && (psSearchList->bExt[0] != ' ')) //if any file has ext name, then show, including folder
        {
            pbNameBuffer[tempIndex + 0] = '.';
            pbNameBuffer[tempIndex + 1] = psSearchList->bExt[0];
            pbNameBuffer[tempIndex + 2] = psSearchList->bExt[1];
            pbNameBuffer[tempIndex + 3] = psSearchList->bExt[2];
            pbNameBuffer[tempIndex + 4] = 0;
        }
        else
        {
            pbNameBuffer[tempIndex + 0] = 0;
        }

//remove the extension name
#if 1 //PDPTV_42_EU
        if (pbNameBuffer[tempIndex] == '.')
        {
            for (i=0; i < 4; i++)
            {
                pbExtensionBuffer[i] = pbNameBuffer[tempIndex + i];
                pbNameBuffer[tempIndex + i] = 0;
            }
        }
#endif
    }
    else /* long filename */
    {
        BYTE *FullFileName;

        FullFileName = (BYTE *) ker_mem_malloc(MAX_L_NAME_LENG * sizeof(WORD), TaskGetId());
        if (FullFileName == NULL)
        {
            MP_ALERT("%s: malloc fail !", __FUNCTION__);
            return FAIL;
        }
        MpMemSet(FullFileName, 0, MAX_L_NAME_LENG * sizeof(WORD));

        if (FS_SUCCEED != FileGetLongName(pCurDrive, psSearchList, FullFileName, &tempIndex, MAX_L_NAME_LENG * sizeof(WORD)))
        {
            MP_ALERT("%s: FileGetLongName() failed !", __FUNCTION__);
            ker_mem_free(FullFileName);
            return FAIL;
        }

        if (psSearchList->bParameter == SEARCH_INFO_FILE)
        {
            for (i = 0; i < (MAX_L_NAME_LENG * sizeof(WORD)); i++)
            {
                if (FullFileName[i] == '.')
                {
                    if (((FullFileName[i+2] == psSearchList->bExt[0])
                          || (FullFileName[i+2] == (psSearchList->bExt[0]+0x20)))
                          && ((FullFileName[i+4] == psSearchList->bExt[1])
                          || (FullFileName[i+4] == (psSearchList->bExt[1]+0x20)))
                          && ((FullFileName[i+6] == psSearchList->bExt[2])
                          || (FullFileName[i+6] == (psSearchList->bExt[2]+0x20)))
                          && (FullFileName[i+8] == 0))
                    {
                        pbExtensionBuffer[0] = FullFileName[i];
                        pbExtensionBuffer[1] = FullFileName[i+2];
                        pbExtensionBuffer[2] = FullFileName[i+4];
                        pbExtensionBuffer[3] = FullFileName[i+6];
                        FullFileName[i] = 0;
                        FullFileName[i+2] = 0;
                        FullFileName[i+4] = 0;
                        FullFileName[i+6] = 0;
                        break;
                    }
                }
            }
        }

        MpMemCopy(pbNameBuffer, FullFileName, dwBufferSize - 2);
        pbNameBuffer[dwBufferSize - 1] = 0;
        ker_mem_free(FullFileName);
    }

    return PASS;
}


///
///@ingroup FILE_BROWSER
///@brief   Search the next audio file in the audio file list and return the ST_SEARCH_INFO pointer for the audio file.
///         The searching begins from the entry index that is the sum of current index of the list and a specified offset value.
///
///@param   iOffset  The specified offset value to add to current index of the audio file list to begin searching from.
///                  This value can be positive or negative.
///
///@return  This function call will return the ST_SEARCH_INFO pointer for the next audio file entry in the auido file list,
///         else return NULL if any error.
///
ST_SEARCH_INFO *FileBrowserSearchNextAudio(int iOffset)
{
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &g_psSystemConfig->sFileBrowser;
    register ST_SEARCH_INFO *pSearchInfo = NULL;
    DWORD i, n;
    DWORD *pdwIndex = &psFileBrowser->dwAudioCurIndex;

    n = psFileBrowser->dwAudioTotalFile;
    if (iOffset >= 0 && *pdwIndex + iOffset >= n)
        *pdwIndex = *pdwIndex + iOffset - n;
    else if (iOffset < 0 && (-iOffset) > *pdwIndex)
        *pdwIndex = *pdwIndex + n + iOffset;
    else
        *pdwIndex = *pdwIndex + iOffset;

    for (i = 0; i <= n; i++)
    {
        if (*pdwIndex >= n)
        {
            *pdwIndex = 0;
        }

        pSearchInfo = &psFileBrowser->sAudioFileList[*pdwIndex];
        if (pSearchInfo != NULL)
        {
            if (pSearchInfo->bFileType == FILE_OP_TYPE_AUDIO)
                return pSearchInfo;
        }
        *pdwIndex = *pdwIndex + 1;
    }

    return NULL;
}


#if MAKE_XPG_PLAYER
///
///@ingroup FILE_BROWSER
///@brief   Search the next image file in the image file list and return the ST_SEARCH_INFO pointer for the image file.
///         The searching begins from the entry index that is the sum of current index of the list and a specified offset value.
///
///@param   iOffset  The specified offset value to add to current index of the image file list to begin searching from.
///                  This value can be positive or negative.
///
///@return  This function call will return the ST_SEARCH_INFO pointer for the next image file entry in the image file list,
///         else return NULL if any error.
///
ST_SEARCH_INFO *FileBrowserSearchNextImage(int iOffset)
{
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &g_psSystemConfig->sFileBrowser;
    register ST_SEARCH_INFO *pSearchInfo = NULL;
    DWORD i, n;
    DWORD *pdwIndex = &psFileBrowser->dwImgAndMovCurIndex;

//#if NETWARE_ENABLE		// abel 20070930 //cj 102307
#if (NETWARE_ENABLE && HAVE_CURL)	//MiniDV_WiFi doesn't support Curl UI
        if ((g_bXpgStatus == XPG_MODE_NET_FLICKR) ||
            (g_bXpgStatus == XPG_MODE_NET_PICASA) ||
#if YOUGOTPHOTO
            (g_bXpgStatus == XPG_MODE_NET_YOUGOTPHOTO) ||
#endif
#if HAVE_NETSTREAM
            (g_bXpgStatus == XPG_MODE_IRADIO_PLS) ||
#endif
#if HAVE_FRAMECHANNEL
            (g_bXpgStatus == XPG_MODE_FRAMECHANNEL) ||
#endif
#if HAVE_FRAMEIT
            (g_bXpgStatus == XPG_MODE_FRAMEIT) ||
#endif
#if HAVE_SNAPFISH
            (g_bXpgStatus == XPG_MODE_SNAPFISH) ||
#endif
            (g_bXpgStatus == XPG_MODE_NET_PC_PHOTO))
        {
            NetSetFileIndex(NetAddCurIndex(iOffset));	//offset g_psNet_FileBrowser->dwCurrentFile->dwCurrentFile
#ifndef DEMO_PID	/* add for DEMO_PID, it doesn't need to execute this function -- GetNetNextPictureIndex() */            
			GetNetNextPictureIndex();		//offset g_psSystemConfig->sFileBrowser->dwImgAndMovCurIndex
#endif			
            return (ST_SEARCH_INFO *)NetGetCurFileEntry();
        }
#if NET_UPNP
        if (g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST)
        {
            Net_Upnp_SearchNextEntry(1);
            return (ST_SEARCH_INFO *)Net_Get_UpnpCurEntry();
        }
#endif
#endif //NETWARE_ENABLE


    n = psFileBrowser->dwImgAndMovTotalFile;
    if (iOffset >= 0 && *pdwIndex + iOffset >= n)
        *pdwIndex = *pdwIndex + iOffset - n;
    else if (iOffset < 0 && (-iOffset) > *pdwIndex)
        *pdwIndex = *pdwIndex + n + iOffset;
    else
        *pdwIndex = *pdwIndex + iOffset;

    for (i = 0; i <= n + 1; i++)
    {
        if (*pdwIndex >= n)
        {
            *pdwIndex = 0;
        }

        pSearchInfo = &psFileBrowser->sImgAndMovFileList[*pdwIndex];
        if (pSearchInfo != NULL)
        {
            if (pSearchInfo->bFileType == FILE_OP_TYPE_IMAGE)
            {
#if (!GIF)  //block gif
                while((psFileBrowser->sImgAndMovFileList[psFileBrowser->dwImgAndMovCurIndex].bExt[0] == 'G') ||
                      (psFileBrowser->sImgAndMovFileList[psFileBrowser->dwImgAndMovCurIndex].bExt[0] == 'g') )
                {
                    FileBrowserSearchNextImage(iOffset++);
                }
#endif
                return pSearchInfo;
            }
        }
        *pdwIndex = *pdwIndex + 1;
    }
    return NULL;
}
#endif



///
///@ingroup FILE_BROWSER
///@brief   Get the file handle of current image file.
///
///@param   pSearchInfo    The pointer to the GUI file list entry of the file.
///
///@return  This function call will return the file handle of current image, else return NULL if any error.
///
STREAM *FileBrowser_GetCurImageFile(ST_SEARCH_INFO * pSearchInfo)
{
#if (NETWARE_ENABLE && HAVE_CURL)	//MiniDV_WiFi doesn't support Curl UI
    if (isNetFunc_Mode())
    {
        //return Net_FileListOpen(FileBrowserGetCurDrive(), pSearchInfo);
#if DM9KS_ETHERNET_ENABLE
        return Net_FileListOpen(DriveGet(CF_ETHERNET_DEVICE), pSearchInfo);
#else
        return Net_FileListOpen(DriveGet(USB_WIFI_DEVICE), pSearchInfo);
#endif
    }
#endif

    ST_FILE_BROWSER *psBrowser;

    if (pSearchInfo == NULL)
    {
        MP_DEBUG("%s: (pSearchInfo == NULL) => get current image file...", __FUNCTION__);

        // get current image file
        psBrowser = (ST_FILE_BROWSER *)&g_psSystemConfig->sFileBrowser;
        if (psBrowser->dwImgAndMovCurIndex >= psBrowser->dwImgAndMovTotalFile)
        {
            MP_ALERT("%s: No file ! (psBrowser->dwImgAndMovCurIndex (%u) >= psBrowser->dwImgAndMovTotalFile (%u)) !", __FUNCTION__, psBrowser->dwImgAndMovCurIndex, psBrowser->dwImgAndMovTotalFile);
            return NULL;
        }

        pSearchInfo = &psBrowser->sImgAndMovFileList[psBrowser->dwImgAndMovCurIndex];
    }

#if RECORD_INVALID_MEDIA
    // check if is invalid file
    if (pSearchInfo->bParameter & SEARCH_INFO_INVALID_MEDIA)
    {
        MP_ALERT("%s: Error! (pSearchInfo->bParameter & SEARCH_INFO_INVALID_MEDIA) !", __FUNCTION__);
        return NULL;
    }
#endif

    return FileListOpen(FileBrowserGetCurDrive(), pSearchInfo);
}


///
///@ingroup FILE_BROWSER
///@brief   Set/move current index of the file list of current GUI operation mode to the specified number.
///
///@param   value    The specified index number to set/update the current index of the file list of current GUI operation mode.
///                  This value can be positive or negative.
///
///@return   The updated current index value of the file list.
///
///@remark   If the specified index number is greater or equal to the total count of entries in the file list (suppose 'TotalCount'),
///          then set current index of the file list to (TotalCount - 1).\n
///          If the specified index number is negative (suppose '-N'), then set current index of the file list to (TotalCount - N).
///
DWORD FileListSetCurIndex(SWORD value)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    DWORD *pdwCurIndex, *pdwTotalCount;

    switch (psSysConfig->dwCurrentOpMode)
    {
        case OP_AUDIO_MODE:
            pdwTotalCount = &psFileBrowser->dwAudioTotalFile;
            pdwCurIndex = &psFileBrowser->dwAudioCurIndex;
            break;

#if EREADER_ENABLE
        case OP_EBOOK_MODE:
            pdwTotalCount = &psFileBrowser->dwEbookTotalFile;
            pdwCurIndex = &psFileBrowser->dwEbookCurIndex;
            break;
#endif

        default:
            pdwTotalCount = &psFileBrowser->dwImgAndMovTotalFile;
            pdwCurIndex = &psFileBrowser->dwImgAndMovCurIndex;
            break;
    }

    if (*pdwTotalCount == 0)
        *pdwCurIndex = 0;
    else
    {
        if (value < 0)
            value += *pdwTotalCount;

        if (value >= *pdwTotalCount)
            value = *pdwTotalCount - 1;

        *pdwCurIndex = value;
    }

    psFileBrowser->dwFileListIndex[psSysConfig->dwCurrentOpMode] = *pdwCurIndex;
    return *pdwCurIndex;
}


///
///@ingroup FILE_BROWSER
///@brief   Add the specified number to current index of the file list of current GUI operation mode and update current index of the file list to it.
///         Thus, the current file entry of the file list is moved upwards or downwards.
///
///@param   value    The specified number to be added to current index of the file list of current GUI operation mode.
///                  This value can be positive or negative.
///
///@return   The updated current index value of the file list.
///
///@remark   If the updated current index value exceeds the total count of entries in the file list, then reset it to 0.
///
DWORD FileListAddCurIndex(SWORD value)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    DWORD *pdwCurIndex, *pdwTotalCount;

    switch (psSysConfig->dwCurrentOpMode)
    {
        case OP_AUDIO_MODE:
            pdwTotalCount = &psFileBrowser->dwAudioTotalFile;
            pdwCurIndex = &psFileBrowser->dwAudioCurIndex;
            break;

#if EREADER_ENABLE
        case OP_EBOOK_MODE:
            pdwTotalCount = &psFileBrowser->dwEbookTotalFile;
            pdwCurIndex = &psFileBrowser->dwEbookCurIndex;
            break;
#endif

        default:
            pdwTotalCount = &psFileBrowser->dwImgAndMovTotalFile;
            pdwCurIndex = &psFileBrowser->dwImgAndMovCurIndex;
            break;
    }

    if (value < 0)
    {
        if (*pdwCurIndex >= -(value))
            *pdwCurIndex += value;
        else if (*pdwTotalCount != 0)
            *pdwCurIndex = *pdwTotalCount - 1;
    }
    else
    {
        *pdwCurIndex += value;
        if (*pdwCurIndex >= *pdwTotalCount)
        {
            *pdwCurIndex = 0;
        }
    }

    psFileBrowser->dwFileListIndex[psSysConfig->dwCurrentOpMode] = *pdwCurIndex;
    return *pdwCurIndex;
}



DWORD FileBrowserGetTotalFile(void)
{
	register ST_SYSTEM_CONFIG *psSysConfig;

	psSysConfig = g_psSystemConfig;
	register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;

	switch (psSysConfig->dwCurrentOpMode)
	{
	case OP_AUDIO_MODE:
		return psFileBrowser->dwAudioTotalFile;
	default:
		return psFileBrowser->dwImgAndMovTotalFile;
	}
	return 0;
}


///
///@ingroup FILE_BROWSER
///@brief   Get the index value of the current file in current GUI file list.
///
///@param   None.
///
///@return  This function will return the index value of the current file in file list. Otherwise, it will return 0 if any error.
///
DWORD FileBrowserGetCurIndex(void)
{
    register ST_SYSTEM_CONFIG *psSysConfig;

    psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;

    switch (psSysConfig->dwCurrentOpMode)
    {
        case OP_AUDIO_MODE:
            return psFileBrowser->dwAudioCurIndex;

#if EREADER_ENABLE
        case OP_EBOOK_MODE:
            return psFileBrowser->dwEbookCurIndex;
#endif

        default:
            return psFileBrowser->dwImgAndMovCurIndex;
    }
    return 0;
}



ST_SEARCH_INFO *FileGetCurLyricInfo(void)
{
    register ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    ST_SEARCH_INFO *psSearchList, *psLyricList;
    DWORD *pdwTotalCount;

    switch (psSysConfig->dwCurrentOpMode)
    {
    case OP_AUDIO_MODE:
        pdwTotalCount = &psFileBrowser->dwAudioTotalFile;
        psSearchList = &psFileBrowser->sAudioFileList[psFileBrowser->dwAudioCurIndex];
        psLyricList = &psFileBrowser->sImgAndMovFileList[psFileBrowser->dwAudioCurIndex];
        break;

    case OP_FILE_MODE: /* why exchange in such case ? */
        pdwTotalCount = &psFileBrowser->dwImgAndMovTotalFile;
        psSearchList = &psFileBrowser->sImgAndMovFileList[psFileBrowser->dwImgAndMovCurIndex];
        psLyricList = &psFileBrowser->sAudioFileList[psFileBrowser->dwImgAndMovCurIndex];
        break;

    case OP_MOVIE_MODE:
    case OP_IMAGE_MODE:
    default:
        return NULL;
    }

    /* check for file extension name "MP3" and "LRC" */
    if (((psSearchList->bExt[0] != 'M') || (psSearchList->bExt[1] != 'P') || (psSearchList->bExt[2] != '3')) ||
        ((psLyricList->bExt[0] != 'L') || (psLyricList->bExt[1] != 'R') || (psLyricList->bExt[2] != 'C')) ||
        (*pdwTotalCount == 0))
    {
        return NULL;
    }

    return psLyricList;
}


///
///@ingroup FILE_BROWSER
///@brief   Get the media file type of the file specified by the index value in the file list.
///
///@param   dwIndex    The array index value of an entry in the file list.
///
///@return  This function will return the media file type of the file. Otherwise, it will return 0 if any error.
///
BYTE FileBrowserGetFileType(DWORD dwIndex)
{
    ST_SEARCH_INFO *psSearchList = FileGetSearchInfo(dwIndex);

    if (psSearchList == NULL)
    {
        MP_ALERT("%s: Warning! psSearchList == NULL !", __FUNCTION__);
        return 0;
    }

    if (psSearchList->bFileType == FILE_OP_TYPE_UNKNOWN)
        psSearchList->bFileType = FileGetMediaType(psSearchList->bExt);

    return psSearchList->bFileType;
}


///
///@ingroup FILE_BROWSER
///@brief   Get the media file type of current file.
///
///@param   None.
///
///@return  This function will return the media file type of current file. Otherwise, it will return 0 if any error.
///
BYTE FileBrowserGetCurFileType(void)
{
    ST_SEARCH_INFO *psSearchList = FileGetCurSearchInfo();

    if (psSearchList == NULL)
    {
        MP_ALERT("%s: Warning! psSearchList == NULL !", __FUNCTION__);
        return 0;
    }

    if (psSearchList->bFileType == 0)
        psSearchList->bFileType = FileGetMediaType(psSearchList->bExt);

    return psSearchList->bFileType;
}


///
///@ingroup FILE_BROWSER
///@brief   Get the ST_SEARCH_INFO pointer of the file specified by the index value in the file list.
///
///@param   dwIndex    The array index value of an entry in the file list.
///
///@return  This function will return the ST_SEARCH_INFO pointer of the file. Otherwise, it will return NULL if any error.
///
ST_SEARCH_INFO *FileGetSearchInfo(DWORD dwIndex)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;


    switch (psSysConfig->dwCurrentOpMode)
    {
    case OP_AUDIO_MODE:
        if (dwIndex < psFileBrowser->dwAudioTotalFile)
            return &psFileBrowser->sAudioFileList[dwIndex];
        break;

#if EREADER_ENABLE
    case OP_EBOOK_MODE:
        if (dwIndex < psFileBrowser->dwEbookTotalFile)
            return &psFileBrowser->sEbookFileList[dwIndex];
        break;
#endif

    default:
        if (dwIndex < psFileBrowser->dwImgAndMovTotalFile)
            return &psFileBrowser->sImgAndMovFileList[dwIndex];
        break;
    }
    return NULL;
}


///
///@ingroup FILE_BROWSER
///@brief   Get the ST_SEARCH_INFO pointer of the current file.
///
///@param   None.
///
///@return  This function will return the ST_SEARCH_INFO pointer of the current file. Otherwise, it will return NULL if any error.
///
ST_SEARCH_INFO *FileGetCurSearchInfo(void)
{
    return FileGetSearchInfo(FileBrowserGetCurIndex());
}


///
///@ingroup FILE_BROWSER
///@brief   Get the EXIF Date Time info of the photo file specified by the ST_SEARCH_INFO pointer.
///
///@param   pSearchInfo    [IN] The pointer to the file list entry of the photo file.
///
///@param   pSearchInfo    [OUT] The pointer to a DATE_TIME_INFO_TYPE structure for output EXIF date/time info.
///
///@retval  PASS           Get EXIF Date Time info successfully. \n\n
///@retval  FAIL           Not found EXIF tag of Date Time in the photo file or fail to get it. \n\n
///
#if OPEN_EXIF
int FileBrowserGetImgEXIF_DateTime(ST_SEARCH_INFO * pSearchInfo, DATE_TIME_INFO_TYPE * exif_date_time)
{
    STREAM *sHandle;
    DRIVE *pCurDrive;
    int ret;

    if ((pSearchInfo == NULL) || (exif_date_time == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return FAIL;
    }

    pCurDrive = FileBrowserGetCurDrive();
    if (pCurDrive == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    sHandle = FileListOpen(pCurDrive, pSearchInfo);
    if (sHandle == NULL)
    {
        MP_ALERT("%s: FileListOpen() failed !", __FUNCTION__);
        return FAIL;
    }

  #if OPEN_EXIF
    ret = Get_EXIF_DateTime_Info(sHandle, exif_date_time);
  #else
    MP_DEBUG("%s: API for getting photo EXIF info is not supported !", __FUNCTION__);
    ret = FAIL;
  #endif

    FileClose(sHandle);
    return ret;
}
#endif

///
///@ingroup FILE_BROWSER
///@brief   Copy the file specified by the ST_SEARCH_INFO pointer from the source drive to the target drive.
///
///@param   pSrcSearchInfo    The pointer to the GUI file list entry of the file to be copied. \n\n
///@param   bSrcDrvID         The drive ID of the source drive. \n\n
///@param   bTrgDrvID         The drive ID of the target drive.
///
///@retval  FS_SUCCEED        Copy file successfully. \n\n
///@retval  FAIL              Copy file unsuccessfully. \n\n
///@retval  DISK_READ_ONLY    Copy file unsuccessfully due to the target drive is read-only.
///
SWORD FileBrowserCopyFile(ST_SEARCH_INFO * pSrcSearchInfo, BYTE bSrcDrvID, BYTE bTrgDrvID)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    STREAM *pSrcHandle;
    SWORD ret;
    DRIVE *pTrgdrv;
    DRIVE *pSrcDrv;
    DWORD dwLngCnt = 0;
    BYTE DirFlag = 0;


    if ((bSrcDrvID == NULL_DRIVE) || (bTrgDrvID == NULL_DRIVE))
    {
        MP_ALERT("%s: Invalid drive !", __FUNCTION__);
        return FAIL;
    }

    if (pSrcSearchInfo == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return FAIL;
    }

    if (!(SystemCardPresentCheck(bTrgDrvID) && SystemCardPresentCheck(bSrcDrvID)))
    {
        MP_ALERT("%s: Error! Card not present !", __FUNCTION__);
        return FAIL;
    }

    /* check Read-Only flag by underlying Mcard layer info => physical lock latch or Mcard layer lock */
    if (SystemGetFlagReadOnly(bTrgDrvID))
    {
        MP_ALERT("%s: -E- The target drive is Read-Only (in Mcard layer) !", __FUNCTION__);
        return DISK_READ_ONLY;
    }

    pTrgdrv = DriveGet(bTrgDrvID);
    /* check Read-Only flag by file system layer => logical or controlled by S/W */
    if (pTrgdrv->Flag.ReadOnly)
    {
        MP_ALERT("%s: -E- The target drive is Read-Only (controlled in file system layer) !", __FUNCTION__);
    #if EXFAT_ENABLE
        if (pTrgdrv->Flag.FsType == FS_TYPE_exFAT)
        {
            MP_DEBUG("%s: The target drive is exFAT file system.", __FUNCTION__);
        }
    #endif
        return DISK_READ_ONLY;
    }

    DWORD dwBufferSize = 512 * 1024;
    DWORD *pdwBuffer = (DWORD *) ext_mem_malloc(dwBufferSize);

    if (pdwBuffer == NULL)
    {
        MP_ALERT("%s: malloc fail !", __FUNCTION__);
        return FAIL;
    }
    pdwBuffer = (DWORD *) (0x20000000 | (DWORD) pdwBuffer);

    pSrcDrv = DriveGet(bSrcDrvID);

    if (pSrcSearchInfo->bParameter & SEARCH_INFO_CHANGE_PATH) // 06.26.2006 for use bParameter record invalid file
    {
        ext_mem_free(pdwBuffer);
        return FS_SUCCEED;
    }

    ret = FAIL;

    if (fbrowser_xpgUpdateIconAniFuncPtr)
        fbrowser_xpgUpdateIconAniFuncPtr();

    SystemSetStatus(SYS_STATUS_COPY);

    if (DirReset(pTrgdrv) == FS_SUCCEED)
    {
        if (DirFirst(pTrgdrv) != ABNORMAL_STATUS)
        {
            if (FileGetLongName(pSrcDrv, pSrcSearchInfo, (BYTE *) (&pSrcDrv->LongName[0]), &dwLngCnt, 512) != FS_SUCCEED) // enlarge to 512
            {
                MP_ALERT("%s: -E- FileGetLongName() failed => cancel file copying.", __FUNCTION__);
                ext_mem_free(pdwBuffer);
                return FAIL;
            }

            pSrcHandle = FileListOpen(pSrcDrv, pSrcSearchInfo);
            if (pSrcHandle)
            {
                if (pSrcSearchInfo->bParameter & SEARCH_INFO_FOLDER)
                {
                    DirFlag = 1;
                    ret = DirectoryCopy(pTrgdrv, pSrcDrv, (DWORD) pdwBuffer, dwBufferSize);
                }
                else
                {
#if USBOTG_HOST_PTP
                    if ((pSrcHandle->Drv->DevID == DEV_USB_HOST_PTP) || (pSrcHandle->Drv->DevID == DEV_USBOTG1_HOST_PTP))
                        pSrcHandle->Drv->Node = (FDB *) pSrcSearchInfo;  /* USB PTP special */
#endif

#if (SONY_DCF_ENABLE)
                    /* if the file list entry is a DCF essential object, process its corresponding DCF associated objects */
                    DWORD file_list_index;
                    if (Check_If_FileListEntry_In_DCF_Objs_Set(pSrcSearchInfo, &file_list_index) == TRUE)
                    {
                        BYTE  pri_name_buffer[9];
                        MpMemCopy(pri_name_buffer, (BYTE *) pSrcSearchInfo->bName, 8);
                        pri_name_buffer[8] = 0;

                        /* copy the DCF essential object */
                        pSrcHandle = FileListOpen(pSrcDrv, pSrcSearchInfo);
                        if (pSrcHandle == NULL)
                        {
                            ext_mem_free(pdwBuffer);
                            return FAIL; /* not found the specified DCF object */
                        }

                        ret = FileCopy(pTrgdrv, pSrcHandle, (DWORD) pdwBuffer, dwBufferSize);
                        FileClose(pSrcHandle);  /* close file first, no matter copying is OK or failed */
                        if (ret != FS_SUCCEED)
                        {
                            ext_mem_free(pdwBuffer);
                            return ret;
                        }

                        /* search current directory for files having same primary filename as the DCF essential objects */
                        if (DirFirst(pSrcDrv) == FS_SUCCEED)
                        {
                            while (1)
                            {
                                if (! SystemCardPresentCheck(pSrcDrv->DrvIndex))
                                    break;

                                if (pSrcDrv->Flag.FsType != FS_TYPE_exFAT)
                                {
                                    if ((pSrcDrv->Node->Attribute & FDB_ARCHIVE) && (StringNCompare08_CaseInsensitive((BYTE *) pSrcDrv->Node->Name, pri_name_buffer, 8) == E_COMPARE_EQUAL))
                                    {
                                        if (StringNCompare08_CaseInsensitive((BYTE *) pSrcDrv->Node->Extension, (BYTE *) pSrcSearchInfo->bExt, 3) == E_COMPARE_EQUAL)
                                            goto L_next_file;  /* this file was already copied */

                                        pSrcHandle = FileOpen(pSrcDrv);
                                        if (pSrcHandle == NULL)
                                            break;

                                        ret = FileCopy(pTrgdrv, pSrcHandle, (DWORD) pdwBuffer, dwBufferSize);
                                        FileClose(pSrcHandle);  /* close file first, no matter copying is OK or failed */
                                        if (ret != FS_SUCCEED)
                                            break;

                                        if (fbrowser_xpgUpdateIconAniFuncPtr)
                                            fbrowser_xpgUpdateIconAniFuncPtr();
                                    }
                                }
                        #if EXFAT_ENABLE
                                else
                                {
                                /* To-Do: must re-work this code in exFAT case, because exFAT DirEntry definitions changed */
                                #if 0
                                    if ((pSrcDrv->Node->Attribute & FDB_ARCHIVE) && (StringNCompare08_CaseInsensitive((BYTE *) pSrcDrv->Node->Name, pri_name_buffer, 8) == E_COMPARE_EQUAL))
                                    {
                                        /* To-Do: must re-work this code in exFAT case, because exFAT DirEntry definitions changed */
                                        if (StringNCompare08_CaseInsensitive((BYTE *) pSrcDrv->Node->Extension, (BYTE *) pSrcSearchInfo->bExt, 3) == E_COMPARE_EQUAL)
                                            goto L_next_file;  /* this file was already copied */

                                        pSrcHandle = FileOpen(pSrcDrv);
                                        if (pSrcHandle == NULL)
                                            break;

                                        ret = FileCopy(pTrgdrv, pSrcHandle, (DWORD) pdwBuffer, dwBufferSize);
                                        FileClose(pSrcHandle);  /* close file first, no matter copying is OK or failed */
                                        if (ret != FS_SUCCEED)
                                            break;

                                        if (fbrowser_xpgUpdateIconAniFuncPtr)
                                            fbrowser_xpgUpdateIconAniFuncPtr();
                                    }
                                #endif
                                }
                        #endif

L_next_file:
                                if (DirNext(pSrcDrv) != FS_SUCCEED)
                                    break;
                            }
                            goto L_post_copy_process;
                        }
                    }
#endif

                    ret = FileCopy(pTrgdrv, pSrcHandle, (DWORD) pdwBuffer, dwBufferSize);
                }
                FileClose(pSrcHandle);

                if (ret == END_OF_DIR)
                    ret = FS_SUCCEED;

L_post_copy_process:
                if (ret == FS_SUCCEED)
                {
                    if (fbrowser_xpgUpdateIconAniFuncPtr)
                        fbrowser_xpgUpdateIconAniFuncPtr();
                }

                if (SystemCardPresentCheck(bTrgDrvID))
                    if (DirReset(pTrgdrv) == FS_SUCCEED)
                        DirFirst(pTrgdrv);
            }
        }
    }

    if (ret == FS_SUCCEED)
    {
        if (fbrowser_xpgUpdateIconAniFuncPtr)
            fbrowser_xpgUpdateIconAniFuncPtr();
    }

    if (SystemCardPresentCheck(bSrcDrvID))
    {
      /* I am not sure if any reason we need to use fbrowser_xpgChangeDriveFuncPtr(bSrcDrvID) here for xpgChangeDrive(bSrcDrvID) ?? */
      #if 0
        if (fbrowser_xpgChangeDriveFuncPtr)
            fbrowser_xpgChangeDriveFuncPtr(bSrcDrvID);
      #endif

        if (DirFlag)
            CdParent(pSrcDrv);

        FileBrowserResetFileList();
        FileBrowserScanFileList(SEARCH_TYPE);
    }

    SystemClearStatus(SYS_STATUS_COPY);
    ext_mem_free(pdwBuffer);

    return ret;
}


///
///@ingroup FILE_BROWSER
///@brief   Delete the file specified by the current ST_SEARCH_INFO pointer in the current file list.
///
///@param   None.
///
///@retval  FS_SUCCEED        Delete file successfully. \n\n
///@retval  FAIL              Delete file unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND    Delete file unsuccessfully due to file not found. \n\n
///@retval  INVALID_DRIVE     Delete file unsuccessfully due to the DRIVE handle of current drive is NULL. \n\n
///@retval  DISK_READ_ONLY    Delete file unsuccessfully due to the target drive is read-only.
///
///@remark  If the file entry specified by the current ST_SEARCH_INFO pointer in the current file list is actually
///         a directory/folder, this function call will delete all the content in this directory and the directory itself.
///
SWORD FileBrowserDeleteFile(void)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    ST_SEARCH_INFO *pSearchInfo;
    BYTE bDriveId = psSysConfig->sStorage.dwCurStorageId;
    STREAM *sHandle;
    SWORD swRet = FS_SUCCEED;
    DWORD dwTotalFile = 0;
    DRIVE *pCurDrive = FileBrowserGetCurDrive();


    MP_DEBUG("enter %s()...", __FUNCTION__);
    if (pCurDrive == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    /* check Read-Only flag by underlying Mcard layer info => physical lock latch or Mcard layer lock */
    if (SystemGetFlagReadOnly(bDriveId))
    {
        MP_ALERT("%s: -E- The drive is Read-Only (in Mcard layer) !", __FUNCTION__);
        //SystemSetErrEvent(ERR_WRITE_PROTECT);
        return DISK_READ_ONLY;
    }

    /* check Read-Only flag by file system layer => logical or controlled by S/W */
    if (pCurDrive->Flag.ReadOnly)
    {
        MP_ALERT("%s: -E- The drive is Read-Only (controlled in file system layer) !", __FUNCTION__);
    #if EXFAT_ENABLE
        if (pCurDrive->Flag.FsType == FS_TYPE_exFAT)
        {
            MP_DEBUG("%s: The drive is exFAT file system.", __FUNCTION__);
        }
    #endif
        //SystemSetErrEvent(ERR_WRITE_PROTECT);
        return DISK_READ_ONLY;
    }

    pSearchInfo = (ST_SEARCH_INFO *) FileGetCurSearchInfo();
    if (fbrowser_xpgUpdateIconAniFuncPtr)
        fbrowser_xpgUpdateIconAniFuncPtr();

    if (pSearchInfo != NULL)
    {
#if EXFAT_ENABLE
        if (pSearchInfo->FsType == FS_TYPE_exFAT)
        {
	#if EXFAT_WRITE_ENABLE
            MP_ALERT("%s: To-Do: how to process exFAT Write operations ??", __FUNCTION__);
            //SystemSetErrEvent(ERR_WRITE_PROTECT);
            return FAIL; //not supported yet
	#else
            MP_ALERT("%s: -I- exFAT Write operations are not supported !", __FUNCTION__);
            //SystemSetErrEvent(ERR_WRITE_PROTECT);
            return DISK_READ_ONLY;
	#endif
        }
#endif

        SystemSetStatus(SYS_STATUS_DELETE);

        if (pSearchInfo->bParameter & SEARCH_INFO_FOLDER)  /* the entry is a folder */
        {
            /* use FileListOpen() to locate the FDB position of this "folder" entry */
            sHandle = FileListOpen(pCurDrive, pSearchInfo);
            if (sHandle == NULL)
                return FILE_NOT_FOUND;
            swRet = DeleteDir(pCurDrive);
            FileClose(sHandle);  /* release the allocated file handle by FileListOpen() */
        }
        else if (pSearchInfo->bParameter & SEARCH_INFO_CHANGE_PATH)  /* the entry is a pseudo entry for "Upper Folder" usage */
            return swRet;
        else  /* the entry is a file */
        {
#if (SONY_DCF_ENABLE)
            /* if the file list entry is a DCF essential object, process its corresponding DCF associated objects */
            DWORD file_list_index;
            if (Check_If_FileListEntry_In_DCF_Objs_Set(pSearchInfo, &file_list_index) == TRUE)
            {
                BYTE  pri_name_buffer[9];
                MpMemCopy(pri_name_buffer, (BYTE *) pSearchInfo->bName, 8);
                pri_name_buffer[8] = 0;

                /* delete the DCF essential object */
                sHandle = FileListOpen(pCurDrive, pSearchInfo);
                if (sHandle == NULL)
                    return FILE_NOT_FOUND;
                swRet = DeleteFile(sHandle);  /* DeleteFile() invokes FileClose() internally */

                /* search current directory for files having same primary filename as the DCF essential objects */
                if (DirFirst(pCurDrive) == FS_SUCCEED)
                {
                    while (1)
                    {
                        if (! SystemCardPresentCheck(pCurDrive->DrvIndex))
                            break;

                        if (pCurDrive->Flag.FsType != FS_TYPE_exFAT)
                        {
                            if ((pCurDrive->Node->Attribute & FDB_ARCHIVE) && (StringNCompare08_CaseInsensitive((BYTE *) pCurDrive->Node->Name, pri_name_buffer, 8) == E_COMPARE_EQUAL))
                            {
                                sHandle = FileOpen(pCurDrive);
                                DeleteFile(sHandle);  /* DeleteFile() invokes FileClose() internally */
                            }
                        }
                #if EXFAT_ENABLE
                        else
                        {
                        /* To-Do: must re-work this code in exFAT case, because exFAT DirEntry definitions changed */
                        #if 0
                            if ((pCurDrive->Node->Attribute & FDB_ARCHIVE) && (StringNCompare08_CaseInsensitive((BYTE *) pCurDrive->Node->Name, pri_name_buffer, 8) == E_COMPARE_EQUAL))
                            {
                                sHandle = FileOpen(pCurDrive);
                                DeleteFile(sHandle);  /* DeleteFile() invokes FileClose() internally */
                            }
                        #endif
                        }
                #endif

                        if (DirNext(pCurDrive) != FS_SUCCEED)
                            break;
                    }
                    goto L_post_delete_process;
                }
            }
#endif

            sHandle = FileListOpen(pCurDrive, pSearchInfo);
            if (sHandle == NULL)
                return FILE_NOT_FOUND;
            swRet = DeleteFile(sHandle);  /* DeleteFile() invokes FileClose() internally */
        }

        if (swRet == END_OF_DIR)
            swRet = FS_SUCCEED;

L_post_delete_process:
        if (swRet == FS_SUCCEED)
        {
            DWORD dwOpMode = psSysConfig->dwCurrentOpMode;
            DWORD CurIndex = psFileBrowser->dwFileListIndex[dwOpMode];

            if (fbrowser_xpgUpdateIconAniFuncPtr)
                fbrowser_xpgUpdateIconAniFuncPtr();

            FileBrowserResetFileList();
            if (!SystemCardPresentCheck(pCurDrive->DrvIndex))
                return FAIL;

            if (SEARCH_TYPE == GLOBAL_SEARCH)
                DirReset(pCurDrive); /* back to beginning of root directory */
            else
                FirstNode(pCurDrive); /* back to beginning of current directory */

            psFileBrowser->dwFileListIndex[dwOpMode] = CurIndex;
            FileBrowserScanFileList(SEARCH_TYPE);

            if (fbrowser_xpgUpdateIconAniFuncPtr)
                fbrowser_xpgUpdateIconAniFuncPtr();

            SystemClearStatus(SYS_STATUS_DELETE);
        }
    }
    else
        swRet = FAIL;

    return swRet;
}



#if Make_TESTCONSOLE
MPX_KMODAPI_SET(FileBrowserListAllFileName);
MPX_KMODAPI_SET(FileGetSearchInfo);
MPX_KMODAPI_SET(FileBrowserGetFileName);
#endif

