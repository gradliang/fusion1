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
* Filename      : McardInfo.c
* Programmer(s) : Logan
* Created       : Logan
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
#include "mpTrace.h"

#include "global612.h"

extern void NandSetWLInfo(DWORD *buf);
extern DWORD NandGetWLInfoSize();
extern BOOL NandWLInfoUpdate();
extern void NandGetWLInfo(DWORD *buf);

static E_DRIVE_INDEX_ID stored_drive_id = NULL_DRIVE; /* the drive index ID of drive on which cardinfo.bin resides */

void McardDevInfoStore()
{
#if (NAND_ENABLE && (NAND_DRV == NAND_FTL_DRV))
    #if (SC_USBDEVICE)
    if (NandWLInfoUpdate() && !SystemCheckUsbdPlugIn())
    #else
    if (NandWLInfoUpdate())
    #endif
    {
        DWORD infosz = NandGetWLInfoSize();
        BYTE *info_buf = (DWORD *)ext_mem_malloc(infosz);
        BYTE orgDrvId = DriveCurIdGet();

        if (stored_drive_id != NULL_DRIVE)
        {
            if (info_buf)
            {
                DRIVE *drv = DriveGet(stored_drive_id);
                STREAM *fptr = NULL;
                ST_SAVE_FDB_POS stFdbPosInfo;
                SDWORD saveNodeSuccress;

                saveNodeSuccress = SaveCurrentDirNodePosition(DriveGet(orgDrvId), &stFdbPosInfo);

                if (FileSearch(drv, "cardinfo", "bin", E_FILE_TYPE) == FS_SUCCEED)
                    fptr = FileOpen(drv);
                else
                {
                    MP_ALERT("create cardinfo.bin ...");
                    if (CreateFile(drv, "cardinfo", "bin") == FS_SUCCEED)
                        fptr = FileOpen(drv);
                    else
                    {
                        MP_ALERT("Unable to create cardinfo.bin !");
                    }
                }

                if (fptr)
                {
                    MP_DEBUG("Store Mcard device information....");
                    info_buf = (BYTE *)((DWORD)info_buf|BIT29);
                    NandGetWLInfo((DWORD *)info_buf);
                    FileWrite(fptr, info_buf, infosz);
                    FileClose(fptr);
                }

                ext_mem_free(info_buf);

                if (saveNodeSuccress == PASS)
                {
                    DriveChange(orgDrvId);
                    RestoreDirNodePosition(DriveGet(orgDrvId), &stFdbPosInfo);
                }
            }
            else
            {
                mpDebugPrint("No memory for %s!!", __FUNCTION__);
            }
        }
    }

    Ui_TimerProcAdd(10000, McardDevInfoStore);	// 10sec
#endif
}



void McardDevInfoLoad(E_DRIVE_INDEX_ID drv_id)
{
#if (NAND_ENABLE && (NAND_DRV == NAND_FTL_DRV))
	DWORD infosz = NandGetWLInfoSize();
	BYTE *info_buf = (BYTE *)ext_mem_malloc(infosz);

	stored_drive_id = drv_id;
	if (info_buf)
	{
		DRIVE *drv = DriveGet(drv_id);
		STREAM *fptr = NULL;

		if (FileSearch(drv, "cardinfo", "bin", E_FILE_TYPE) == FS_SUCCEED)
		{
			fptr = FileOpen(drv);
		}
		else
		{
			mpDebugPrint("cardinfo.bin not found!");
		}

		if (fptr)
		{
			if (FileSizeGet(fptr) == infosz)
			{
				MP_DEBUG("Load Mcard device information....");
				info_buf = (BYTE *)((DWORD)info_buf | BIT29);
				FileRead(fptr, info_buf, infosz);
				NandSetWLInfo((DWORD*)info_buf);
			}
			FileClose(fptr);
		}
		ext_mem_free(info_buf);
	}
	else
	{
		mpDebugPrint("No memory for %s!!", __FUNCTION__);
	}
#endif
}

