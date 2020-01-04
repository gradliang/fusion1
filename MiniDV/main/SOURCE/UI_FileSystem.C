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
* Filename      : UI_FileSystem.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  1

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "TaskId.h"
#include "os.h"
#include "ui.h"
#include "peripheral.h"
#include "ui_timer.h"
#include "Camcorder_Func.h"
#include "UI_FileSystem.h"
#include "PhotoFrame.h"


#define memcpy                      mmcp_memcpy
#define memset                      mmcp_memset

#define  MaxPathStrLen				16

DWORD GetMaxDigitName(void)
{
	DWORD dwTmpDigit=0;
	SDWORD i,k;
	ST_SEARCH_INFO * pSearchInfo;
	ST_FILE_BROWSER *psBrowser = (ST_FILE_BROWSER *)&g_psSystemConfig->sFileBrowser;

	for (i=g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile-1;i>=0;i--)
	{
		pSearchInfo = &psBrowser->sImgAndMovFileList[i];
		mpDebugPrint("GetMaxDigitName:%d:%s",i,pSearchInfo->bName);
		dwTmpDigit=0;
		for (k=0;k<3;k++)
		{
			if (pSearchInfo->bName[k]<0x30 || pSearchInfo->bName[k]>0x39)
				break;
			dwTmpDigit*=10;
			dwTmpDigit+=pSearchInfo->bName[k]-0x30;
		}
		if (k==3 && (pSearchInfo->bName[k]==0||pSearchInfo->bName[k]==0x20))
			break;
	}
	return dwTmpDigit;
}

STREAM * CreatePhotoFileByIndex(void)
{
	STREAM *fileHandle = (STREAM *) NULL;
	BYTE bPathStr[MaxPathStrLen];
	DWORD i,dwNameDigit;
	SWORD swRet;

    //////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////
	dwNameDigit=GetMaxDigitName();
	mp_sprintf(bPathStr,"%03d",dwNameDigit);
	for (i=0;i<g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile;i++)
	{
        if (FileSearch(DriveGet(DriveCurIdGet()), (BYTE *) bPathStr, (BYTE *) "jpg", E_FILE_TYPE)!=FS_SUCCEED)
					break;
		dwNameDigit++;
		mp_sprintf(bPathStr,"%03d",dwNameDigit);
	}
	mpDebugPrint("CreatePhotoFileByIndex:%d -- %s",dwNameDigit,bPathStr);
    //////////////////////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////////////////////////
    if (CreateFile(DriveGet(DriveCurIdGet()), bPathStr,"jpg") != FS_SUCCEED)
        MP_ALERT("--E-- %s: Create fle error !!!", __FUNCTION__);
    else
    {
        fileHandle = FileOpen(DriveGet(DriveCurIdGet()));
        if (fileHandle == (STREAM *) NULL)
            MP_ALERT("--E-- %s: Open file fail after created !!!", __FUNCTION__);
    }

    return fileHandle;
}

STREAM * CreateFileByTime(WORD *folderPath, BYTE *extendFileName)
{
	STREAM *fileHandle = (STREAM *) NULL;
	BYTE bPathStr[MaxPathStrLen];
	WORD wPathStr[MaxPathStrLen];
	DWORD dwRtcCnt,i,k;
	DWORD ret;
	ST_SYSTEM_TIME stSystemTime;

    //////////////////////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////////////////////////
    if (folderPath)
    {
        if (PathAPI__Cd_Path_UTF16(folderPath) != FS_SUCCEED)
        {
            if (PathAPI__MakeDir_UTF16(folderPath) != FS_SUCCEED)
            {
                MP_ALERT("%s %d", __FILE__,__LINE__);
                MP_ALERT("--E-- %s: Create folder fail !!!", __FUNCTION__);
                return (STREAM *) NULL;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////////////////////////
    //SystemTimeGet(&stSystemTime);
	dwRtcCnt=RTC_ReadCount();
	for (i=0;i<FILE_LIST_SIZE;i++)
	{
		SystemTimeSecToDateConv(dwRtcCnt, &stSystemTime);
		//mp_sprintf(bPathStr,"%04d%02d%02d%02d%02d%02d.%s", stSystemTime.u16Year,stSystemTime.u08Month,stSystemTime.u08Day,stSystemTime.u08Hour,stSystemTime.u08Minute,stSystemTime.u08Second,extendFileName);
		mp_sprintf(bPathStr,"%02d%02d%02d%02d.%s", stSystemTime.u08Month,stSystemTime.u08Day,stSystemTime.u08Hour,stSystemTime.u08Minute,extendFileName);
		//MP_ALERT("--E-- %s: Create file:%s", __FUNCTION__,bPathStr);
		mpx_UtilAsc2Uni(wPathStr, bPathStr, MaxPathStrLen-1);
		ret = FileSearchLN(DriveGet(DriveCurIdGet()), wPathStr, StringLength16(wPathStr), E_BOTH_FILE_AND_DIR_TYPE);
		if (ret == END_OF_DIR)
			break;
		dwRtcCnt+=60;
	}

    //////////////////////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////////////////////////
    if (CreateFile_UTF16(DriveGet(DriveCurIdGet()), wPathStr) != FS_SUCCEED)
        MP_ALERT("--E-- %s: Create fle error !!!", __FUNCTION__);
    else
    {
        fileHandle = FileOpen(DriveGet(DriveCurIdGet()));
        if (fileHandle == (STREAM *) NULL)
            MP_ALERT("--E-- %s: Open file fail after created !!!", __FUNCTION__);
    }

    return fileHandle;
}


STREAM * CreateFileByRtcCnt(BYTE *folderPath, BYTE *extendFileName)
{
	STREAM *fileHandle = (STREAM *) NULL;
	BYTE bPathStr[MaxPathStrLen];
	WORD wPathStr[MaxPathStrLen];
	DWORD dwRtcCnt,i,k;
	int ret;

    //////////////////////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////////////////////////
    if (folderPath)
    {
        if (PathAPI__Cd_Path(folderPath) != FS_SUCCEED)
        {
            if (PathAPI__MakeDir(folderPath) != FS_SUCCEED)
            {
                MP_ALERT("%s %d", __FILE__,__LINE__);
                MP_ALERT("--E-- %s: Create folder fail !!!", __FUNCTION__);
                return (STREAM *) NULL;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////////////////////////
    //SystemTimeGet(&stSystemTime);
	dwRtcCnt=RTC_ReadCount();
	for (i=0;i<FILE_LIST_SIZE;i++)
	{
		for (k=0;k<8;k++)
		{
			bPathStr[7-k]=((dwRtcCnt>>(k<<2))&0x0000000f)+0x41;// 0x41->A
		}
		bPathStr[8]=0;
		mp_sprintf(&bPathStr[8],".%s",extendFileName);
		//MP_ALERT("--E-- %s: Create file:%s", __FUNCTION__,bPathStr);
		mpx_UtilAsc2Uni(wPathStr, bPathStr, MaxPathStrLen-1);
		ret = FileSearchLN(DriveGet(DriveCurIdGet()), wPathStr, StringLength16(wPathStr), E_BOTH_FILE_AND_DIR_TYPE);
		if (ret == END_OF_DIR)
			break;
		dwRtcCnt++;
	}

    //////////////////////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////////////////////////
    if (CreateFile_UTF16(DriveGet(DriveCurIdGet()), wPathStr) != FS_SUCCEED)
        MP_ALERT("--E-- %s: Create fle error !!!", __FUNCTION__);
    else
    {
        fileHandle = FileOpen(DriveGet(DriveCurIdGet()));
        if (fileHandle == (STREAM *) NULL)
            MP_ALERT("--E-- %s: Open file fail after created !!!", __FUNCTION__);
    }

    return fileHandle;
}

STREAM *UI_FileSystem_AutoNameFileCreate(WORD *folderPath, WORD *fileNamePreFix, BYTE *extendFileName)
{
	return CreateFileByTime(folderPath,extendFileName);
}

SDWORD UI_FileSystem_EarliestFileRemove(WORD *folderPath, WORD *fileNamePreFix, BYTE *extendFileName)
{
	//g_psSystemConfig->dwCurrentOpMode = OP_MOVIE_MODE;
	FileBrowserResetFileList(); /* reset old file list first */
	FileBrowserScanFileList(SEARCH_TYPE);
	if (!FileBrowserGetTotalFile())
		return FAIL;
	//xpgCb_DeleteFile(FileBrowserGetTotalFile()-1);
	FileListSetCurIndex(FileBrowserGetTotalFile()-1);
	FileBrowserDeleteFile();
	return PASS;
}

SDWORD EarliestFileRemoveToFreeSize(DWORD dwFreesize) //K BYTES
{
	DWORD diskSize,retry=FILE_LIST_SIZE;

    diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) >>10;  // Sector
    diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet()));

	while (retry && diskSize < dwFreesize)
	{
			FileBrowserResetFileList(); /* reset old file list first */
			FileBrowserScanFileList(SEARCH_TYPE);
			if (!FileBrowserGetTotalFile())
				return FAIL;
			FileListSetCurIndex(FileBrowserGetTotalFile()-1);
			FileBrowserDeleteFile();

	    diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) >>10;  // Sector
	    diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet()));
		retry--;
	}

	return diskSize;
}


