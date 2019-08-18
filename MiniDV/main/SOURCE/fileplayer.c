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
* Filename      : fileplayer.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#include "display.h"
#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

/*
// Definition of external functions
*/
SWORD FilePrintInfo(ST_IMGWIN * pWin, DWORD dwTypeIndex, DWORD x, DWORD y)
{
#define MAX_BUF_SIZE 100
	ST_SYSTEM_CONFIG *psSysConfig;
	ST_SEARCH_INFO *psSearchList;
	STREAM *sHandle;
	DWORD *pdwTotalCount, dwMaxLength;
	BYTE bTempBuffer[MAX_BUF_SIZE], *pbTempBuffer;

	psSysConfig = g_psSystemConfig;

	if (!SystemCardPresentCheck(psSysConfig->sStorage.dwCurStorageId))
	{
		memset(pbTempBuffer, 0, MAX_BUF_SIZE);
		Idu_PrintString(pWin, pbTempBuffer, x, y, 0, 0);
		return FAIL;
	}


	if (psSysConfig->dwCurrentOpMode != OP_FILE_MODE)
	{
		return PASS;
	}

	pdwTotalCount = &psSysConfig->sFileBrowser.dwImgAndMovTotalFile;
	psSearchList =
		&psSysConfig->sFileBrowser.sImgAndMovFileList[psSysConfig->sFileBrowser.
													  dwImgAndMovCurIndex];


	pbTempBuffer = (BYTE *) (((DWORD) (&bTempBuffer[0])) | 0xa0000000);
	memset(pbTempBuffer, 0, MAX_BUF_SIZE);
	switch (dwTypeIndex)
	{
	case 0:
		sHandle = FileListOpen(DriveGet(DriveCurIdGet()), psSearchList);
		if (sHandle == 0)
		{
			MP_DEBUG("-E- FileListOpen fail");
			return FAIL;
		}

		switch (psSearchList->bParameter & SEARCH_INFO_TYPE_MASK) // 06.26.2006 for use bParameter record invalid file
		{
		case SEARCH_INFO_FILE:
			if (FileSizeGet(sHandle) > (1 << 30))
			{
				mp_sprintf(pbTempBuffer, "File Size : %d MBytes", FileSizeGet(sHandle) >> 20);
			}
			else if (FileSizeGet(sHandle) > (1 << 20))
			{
				mp_sprintf(pbTempBuffer, "File Size : %d KBytes", FileSizeGet(sHandle) >> 10);
			}
			else
			{
				mp_sprintf(pbTempBuffer, "File Size : %d Bytes", FileSizeGet(sHandle));
			}
			break;

		case SEARCH_INFO_FOLDER:
			mp_sprintf(pbTempBuffer, "Folder");
			break;

		case SEARCH_INFO_CHANGE_PATH:
			mp_sprintf(pbTempBuffer, "Return");
			break;

		default:
			mp_sprintf(pbTempBuffer, "");
			break;
		}
		FileClose(sHandle);
		break;

	case 1:
		switch (psSearchList->bParameter & SEARCH_INFO_TYPE_MASK) // 06.26.2006 for use bParameter record invalid file
		{
		case SEARCH_INFO_FILE:
		case SEARCH_INFO_FOLDER:
			mp_sprintf(pbTempBuffer, "Date : %.4d/ %.2d/ %.2d", psSearchList->DateTime.year, psSearchList->DateTime.month, psSearchList->DateTime.day);
			break;

		case SEARCH_INFO_CHANGE_PATH:
		default:
			mp_sprintf(pbTempBuffer, "");
			break;
		}
		break;

	case 2:
		mp_sprintf(pbTempBuffer, "");
		break;

	case 3:
		mp_sprintf(pbTempBuffer, "");
		break;

	case 4:
		mp_sprintf(pbTempBuffer, "");
		break;

	default:
		return FAIL;
	}
	Idu_PrintString(pWin, pbTempBuffer, x, y, 0, 0);
	return PASS;
}
