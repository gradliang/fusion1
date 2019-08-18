
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

//#define CAMCORDER_TEST
#undef CAMCORDER_TEST

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "xpgFunc.h"

#include "os.h"
#include "display.h"
#include "devio.h"
#include "taskid.h"
#include "fs.h"
#include "ui.h"
#include "ui_timer.h"
#include "..\..\UI\INCLUDE\rtc.h" // for xpgCb_EnterCardMenu() add RemoveProce(calendarWhiteProcess) when CARD_OUT
#include "UI_FileSystem.h"
#include "xpgCamFunc.h"

#if 1//MAKE_XPG_PLAYER

BYTE xpgGetFirstDrive();

//---------------------------------------------------------------------------
//Card
//---------------------------------------------------------------------------
#ifndef MAX_DRIVE_NUM
#error  MAX_DRIVE_NUM not defined !!!
#endif


//------------------------------------------------------------------------------------------


///
///@ingroup xpgDriveFunc
///@brief   Search and Goto NO_CARD page
///
void xpgGotoNoCardPage()
{
    MP_DEBUG("xpgGotoNoCardPage");
}


// MagicPixel Standard UI
///
///@ingroup xpgDriveFunc
///
///@brief   Change to Next Card
///
///@retval  true o false
///
BOOL xpgCb_NextCard(void)
{
    DWORD i;

    for (i = 1; i < MAX_DRIVE_NUM; i++)
    {
        if (SystemCardPresentCheck(i))
        {
				xpgChangeDrive(i);
				break;
        }
    }
}


//---------------------------------------------------------------------------
///
///@ingroup xpgDriveFunc
///@brief   Get total count of well-added drives
///
///@retval BYTE - drive count, 0..n
///
BYTE xpgGetDriveCount()
{
    BYTE count = DriveCountGet();
    MP_DEBUG("xpgGetDriveCount: count = %d", count);

    return count;
}



//---------------------------------------------------------------------------
///
///@ingroup xpgDriveFunc
///@brief   Check drive count
///
///@retval  BYTE : drive count
///
BYTE xpgCheckDriveCount()
{
    MP_DEBUG("xpgCheckDriveCount");
    register int iCount = xpgGetDriveCount();

    if (iCount == 0)
    {
        xpgGotoNoCardPage();
        return 0;
    }

    return iCount & 0xff;
}
#endif



//---------------------------------------------------------------------------
///
///@ingroup xpgDriveFunc
///@brief   Get first drive from 0..n
///
BYTE xpgGetFirstDrive()
{
    MP_DEBUG("xpgGetFirstDrive");
    register int i;

    for (i = 1; i < MAX_DRIVE_NUM; i++)
    {
        if (SystemCardPresentCheck(i))
        {
            return i;
        }
    }

    return 0;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgDriveFunc
///@brief   change drive
///
///@param   drive_id
///
///@retval  true or false
///
BOOL xpgChangeDrive(E_DRIVE_INDEX_ID drive_id)
{
    MP_DEBUG1("xpgChangeDrive %d", drive_id);

    register STXPGMOVIE *pstMov = &g_stXpgMovie;
    //register int i, j;

    xpgStopAllAction();

    if (!SystemCardPresentCheck(drive_id))
        drive_id = xpgGetFirstDrive();

    if (SystemCardPresentCheck(drive_id))
    {
        g_psSystemConfig->sStorage.dwCurStorageId = drive_id;
        DriveChange(drive_id);
        FileBrowserResetFileList();  // 06.26.2006 Athena
#if MAKE_XPG_PLAYER
        ClrBrowserDrvId(); /* clear g_stBrowser[i].bCurDriveId value, thus xpgChangeMenuMode() can re-scan file list */
#endif
        return true;
    }
    else
    {
        return false;
    }
}

void xpgEnableDrive(E_DRIVE_INDEX_ID drive_id)
{
#if MAKE_XPG_PLAYER
    int iCount = xpgCheckDriveCount();

#if (SC_USBDEVICE)
    if ((iCount == 0) || SystemCheckUsbdPlugIn())
#else
    if (iCount == 0)
#endif
    {
        mpDebugPrint("No Drive Enable or Connect to PC!!");
        return;
    }
#endif

#if UPDATE_BY_USB_ENABLE
	   if ((drive_id>=USB_HOST_ID1&&drive_id<=USB_HOST_PTP)||(drive_id>=USBOTG1_HOST_ID1&&drive_id<=USBOTG1_HOST_PTP))
	   {
		 	AutoISP(drive_id);
	   }
#endif

#if OSDICON_NoSD
		if (drive_id==SD_MMC_PART1 || drive_id==SD_MMC_PART2 || drive_id==SD_MMC_PART3 || drive_id==SD2)
		{
			//SetSDCardStatus(-1);
			Display_SD_Detect();
		}
#endif
		
    if (g_bAniFlag & ANI_CAMRECORDING)
    	return;
    
#if MAKE_XPG_PLAYER
    if (g_bXpgStatus == XPG_MODE_NULL)
        RemoveTimerProc((void *) xpgCheckDriveCount);
#endif    
    xpgChangeDrive(drive_id);

}


//---------------------------------------------------------------------------
///
///@ingroup xpgDriveFunc
///@brief   Disable drive
///
///@param   drive_id
///
void xpgDisableDrive(E_DRIVE_INDEX_ID drive_id)
{
    MP_DEBUG("xpgDisableDrive");
#if SC_USBDEVICE
    if (SystemCheckUsbdPlugIn())        return;
#endif
    if (g_psSystemConfig->sStorage.dwCurStorageId == drive_id) {
        xpgStopAllAction();
        TaskYield();
#if MAKE_XPG_PLAYER
        ClrBrowserDrvId();		// abel add
        xpgClearCatch();
#endif
    }

#if OSDICON_NoSD
		if (drive_id==SD_MMC_PART1 || drive_id==SD_MMC_PART2 || drive_id==SD_MMC_PART3 || drive_id==SD2)
		{
			SetSDCardStatus(-1);
			Display_SD_Detect();
		}
#endif

#if MAKE_XPG_PLAYER
    if (!g_psSystemConfig->sStorage.dwCurStorageId)
        ClrBrowserDrvId();		// abel add, if no card, clear
#endif
    if (drive_id == DriveCurIdGet())
        FileBrowserResetFileList();

#if 1//MAKE_XPG_PLAYER
    if (xpgCheckDriveCount() == 0)
    {
        return;
    }
    if (g_psSystemConfig->sStorage.dwCurStorageId == drive_id)
    {
        xpgCb_NextCard();  // 08.11.2006 Athena move to here
    }
#endif
}



