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
* Filename      : main.c
* Programmer(s) :
* Created       :
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
#include "os.h"
#include "display.h"
#include "devio.h"
#include "taskid.h"
#include "fs.h"
#include "ui.h"
#include "../../ui/include/uiGpio.h"
#include "ui_gpio.h"
#include "usbotg_api.h"
#include "bios.h"
#include "xpgFunc.h"
#include "xpg.h"
#include "../../ui/include/ir.h"
#include "peripheral.h"
#include "ui_timer.h"
#include "setup.h"
#if Make_TESTCONSOLE
#include "tCons.h"
#endif
#if (BT_XPG_UI == ENABLE)
#include "BtApi.h"
#include "xpgBtFunc.h"
#endif
#if (TOUCH_CONTROLLER_ENABLE == ENABLE)
#include "../../ui/include/uitouchctrller.h"
#endif
#include "Filebrowser.h"
#include "mpapi.h"
#if (CyclicRecording)
#include "MiniDV_StateMachine.h"
#endif

#include "xpgCamFunc.h"
#include "SPI_Ex.h"
#include "xpgProcSensorData.h"
/*
// Constant declarations
*/
#define memset          MpMemSet
#define memcpy          MpMemCopy

/*
// Variable declarations
*/
extern BYTE g_bXpgStatus;

static void ClkInit(void);
static SWORD SystemInit(void);
static SWORD XpgInit(BYTE bMcardId, DWORD dwXPGTag);
static SDWORD MainWaitEvent(DWORD * pdwEvent, DWORD dwNextEvent);
static SWORD OnBoardNandSpiInit(E_DRIVE_INDEX_ID);

#if DM9KS_ETHERNET_ENABLE
extern struct net_device* dmfe_dev;
int dmfe_probe(struct net_device *dev);
int dmfe_open(struct net_device *dev);
#endif

static int ProcessAfterIsp(BYTE driveId)
{
#if NAND_ENABLE

	int ret;
	DRIVE *sDrv, *tDrv;
	DRIVE_PHY_DEV_ID phyDevID;

	if ( driveId != NAND )
	{
		DWORD hiddenSec_cnt;
		DWORD total_SectorNr;
		DWORD part2_drv_size;
		
		SystemDeviceRawFormat(SYS_DRV_ID, 0);				// low format NAND
		phyDevID = DriveIndex2PhyDevID(SYS_DRV_ID);

		if (SystemDeviceInit(SYS_DRV_ID) == FAIL) {
			MP_ALERT("%s: %s card initial fail !\r\n", __FUNCTION__, SYS_DRV_ID, DriveIndex2DrvName(SYS_DRV_ID));
			return FAIL;
		}
		
		if (!SystemCardPresentCheck(SYS_DRV_ID)) {
			MP_ALERT("%s: %s card not present !\r\n", __FUNCTION__, SYS_DRV_ID, DriveIndex2DrvName(SYS_DRV_ID));
			return FAIL;
		}

		total_SectorNr = Mcard_GetCapacity(phyDevID);
		mpDebugPrint("Total sector : %d", total_SectorNr);
		
		// note: DiskPartitioning_WithMaxTwoPartitions() reserves less hidden sectors 
		//in front of the boot sector of 1st partition for tiny-size device
		hiddenSec_cnt = (MB_SIZE(total_SectorNr * SECTOR_SIZE) < 2) ? Small_BPB_HiddSec : Default_BPB_HiddSec;
		part2_drv_size = (((total_SectorNr - hiddenSec_cnt) * (SECTOR_SIZE >> 8)) >> 10) - SYS_DRV_SIZE; // unit: 256 KB
				
		MP_ALERT("%s: Perform disk partitioning for (drvId = %d,  physical device Id = %d) ...", __FUNCTION__, SYS_DRV_ID, phyDevID);
		if (ret = DiskPartitioning_WithMaxTwoPartitions(SYS_DRV_ID, part2_drv_size) != PASS)
		{
			MP_ALERT("%s: Error ! DiskPartitioning_WithMaxTwoPartitions() failed for drvID (= %d) ! => Pls check it ...", __FUNCTION__, SYS_DRV_ID);
			return FAIL;			// disk partitioning failed
		}
		if (ret = Fat16_Format(DriveGet(SYS_DRV_ID), SYS_DRV_LABEL) != PASS)
			if (ret = Fat12_Format(DriveGet(SYS_DRV_ID), SYS_DRV_LABEL) != PASS)
				MP_ALERT("-E- (DrvId = %d) FAT32/FAT16/FAT12 formatting all failed ! => Pls check it ...", SYS_DRV_ID);
		if (ret != PASS)
			return FAIL;
		if (ret = Fat32_Format(DriveGet(NAND), USER_DRV_LABEL) != PASS)
			if (ret = Fat16_Format(DriveGet(NAND), USER_DRV_LABEL) != PASS)
				if (ret = Fat12_Format(DriveGet(NAND), USER_DRV_LABEL) != PASS)
					MP_ALERT("-E- (DrvId = %d) FAT32/FAT16/FAT12 formatting all failed ! => Pls check it ...", SYS_DRV_ID);
		if (ret != PASS)
			return FAIL;
		DriveDelete(SYS_DRV_ID);
		DriveDelete(NAND);
		
		// after drive formatting, DriveAdd() again
		if (DriveAdd(SYS_DRV_ID))		// at least one partition/drive added
		{
			tDrv = DriveChange(SYS_DRV_ID);
			if ((tDrv->StatusCode == FS_SUCCEED) && (tDrv->Flag.Present == 1))	//this drive has already been added
			{
				DirReset(tDrv);
				DirFirst(tDrv);
			}
			else
			{
				MP_ALERT("-E- The partition drive of (drvId = %d) was not well-added !	Pls check it !!\r\n", SYS_DRV_ID);
				return FAIL;
			}
		}
		else // none of partition/drive added
		{
			MP_ALERT("-E- (drvId = %d) DriveAdd() failed !\r\n", SYS_DRV_ID);
			return FAIL;
		}
		//--------------
		DriveAdd(driveId);
		sDrv = DriveChange(driveId);
		
		
	}
	else
	{
		if (SystemDeviceInit(SYS_DRV_ID) == FAIL) {
			MP_ALERT("%s: %s card initial fail !\r\n", __FUNCTION__, SYS_DRV_ID, DriveIndex2DrvName(SYS_DRV_ID));
			return FAIL;
		}
		
		if (!SystemCardPresentCheck(SYS_DRV_ID)) {
			MP_ALERT("%s: %s card not present !\r\n", __FUNCTION__, SYS_DRV_ID, DriveIndex2DrvName(SYS_DRV_ID));
			return FAIL;
		}
		if (ret = Fat16_Format(DriveGet(SYS_DRV_ID), SYS_DRV_LABEL) != PASS)
			if (ret = Fat12_Format(DriveGet(SYS_DRV_ID), SYS_DRV_LABEL) != PASS)
				MP_ALERT("-E- (DrvId = %d) FAT32/FAT16/FAT12 formatting all failed ! => Pls check it ...", SYS_DRV_ID);
		if (ret != PASS)
			return FAIL;

		DriveAdd(SYS_DRV_ID);
		tDrv = DriveGet(SYS_DRV_ID);
		sDrv = DriveChange(NAND);
		
	}
	//---------- copy file to sys_drv ----------
	#if 0
	DWORD *dwBuffer;
	BYTE bFileType;
	DWORD i, dwTotal;
	ST_SEARCH_INFO* pSearchArr;
	struct ST_FILE_BROWSER_TAG *psFileBrowser;
	DWORD dwExtArray[] = {EXT_BMP, EXT_FNT, EXT_TAB, EXT_END, 0};

	psFileBrowser = &g_psSystemConfig->sFileBrowser;
	g_psSystemConfig->dwCurrentOpMode = OP_MOVIE_MODE;
	bFileType = psFileBrowser->bFileType[g_psSystemConfig->dwCurrentOpMode];
	pSearchArr = psFileBrowser->sSearchFileList;
	
	dwTotal = 0;
	DirReset(sDrv);
	DirFirst(sDrv);
	DirReset(tDrv);
	DirFirst(tDrv);
	if (FileExtSearch(sDrv, dwExtArray, pSearchArr, FILE_LIST_SIZE, &dwTotal, GLOBAL_SEARCH, bFileType, 0) != FS_SUCCEED)
	{
		MP_ALERT("%s: FileExtSearch() failed !", __FUNCTION__);
		return FAIL;
	}
	mpDebugPrint("total %d file need copy", dwTotal);
	
	dwBuffer = (DWORD *)ext_mem_malloc(512*1024);
	for ( i = 0; i < dwTotal; i++ )
	{
		STREAM* pSrcHandle;
		
		mpDebugPrint("copy %d of %d", i+1, dwTotal);

		pSrcHandle = FileListOpen(sDrv, pSearchArr + i);
		ret = FileCopy(tDrv, pSrcHandle, (DWORD) dwBuffer, 512*1024);
		if ( ret != FS_SUCCEED ) {
			mpDebugPrint("copy file error, %d", ret);
			return FAIL;
		}
		
		FileClose(pSrcHandle);
	}
	ext_mem_free(dwBuffer);
#endif
	//if ( driveId == NAND )
	{
		if (ret = Fat32_Format(DriveGet(NAND), USER_DRV_LABEL) != PASS)
			if (ret = Fat16_Format(DriveGet(NAND), USER_DRV_LABEL) != PASS)
				if (ret = Fat12_Format(DriveGet(NAND), USER_DRV_LABEL) != PASS)
					MP_ALERT("-E- (DrvId = %d) FAT32/FAT16/FAT12 formatting all failed ! => Pls check it ...", SYS_DRV_ID);
		if (ret != PASS)
			return FAIL;
	}
	
	return PASS;
#endif //NAND_ENABLE
#if SPI_STORAGE_ENABLE
    return OnBoardNandSpiInit(SPI_FLASH);
#endif

	return PASS;


}


///@ingroup     ISP_MODULE
///@brief       Auto ISP function
///
///@param       bMcardId        which storage to find isp file.
///
///@remark      This function will update AP code and resource to on board storage and then check written
///             data is correct or not. User should move isp file storage and reboot system after in system
///             programming done.
///
#if ISP_FUNC_ENABLE
void AutoISP(BYTE driveId)
{
#if !MAKE_XPG_PLAYER
    BOOL boDriveAdded = FALSE;

    if (SystemCardPlugInCheck(driveId))
    {   // SD plug
        MP_DEBUG("-I- %s card Plugged", DriveIndex2DrvName(driveId));

        // if card was plugged but not present, waiting some time for M-Card
        // Timing issue
        if (!SystemCardPresentCheck(driveId))
            TaskSleep(600);

        if (!SystemCardPresentCheck(driveId))
        {
            MP_DEBUG("-E- %s present check fail", DriveIndex2DrvName(driveId));
        }
        else
        {
            if (!(boDriveAdded = DriveAdd(driveId)))
                MP_DEBUG("-E- %s card : DriveAdd fail", DriveIndex2DrvName(driveId));

            DriveChange(driveId);
        }

    }

    if (boDriveAdded)
    {
        MP_DEBUG("-I- %s card : DriveAdd OK", DriveIndex2DrvName(driveId));
    }
    else
    {
        MP_ALERT("AutoISP, Driver %s is not ready!",DriveIndex2DrvName(driveId));
		return;
    }

#endif


    STREAM *handle = FileExtSearchOpen(driveId, "ISP", 32);

    if (handle != NULL)
    {
		xpgStopAllAction();
        DisplayInit(DISPLAY_INIT_LOW_RESOLUTION);
        TurnOnBackLight();
        Idu_OsdOnOff(1);

		BYTE bRetry=0;

		ISP_START:
		bRetry++;
		SeekSet(handle);
        mpPrintMessage("Auto ISP start!!");
#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
        NandEraseCodeArea(1);
#endif
        if (ISP_UpDateCode(handle) == PASS)
        {
            mpPrintMessage("ISP success!!");
			if ( ProcessAfterIsp(driveId) < 0 ) 
			{
				mpPrintMessage("ISP Fail!!");
			}
			else 
			{
#if RTC_ENABLE
				ResetRTCTime();
#endif
				SetupMenuReset();
			}
        }
        else
        {
            mpPrintMessage("ISP Fail!!");
				if (bRetry<3)
					goto ISP_START;
				else
				{
            		mpPrintMessage("--E-- ISP FAIL,Please check !!!");
	        		while (1){}
				}
        }

        if (ISP_ChksumVerify() == FAIL)
        {
            mpPrintMessage("--E-- ISP_ChksumVerify fail !!!");
			if (bRetry<3)
				goto ISP_START;
			else
			{
          		mpPrintMessage("--E-- ISP_ChksumVerify FAIL,Please checkl !!!");
        		while (1){}
			}
        }
	else
            mpPrintMessage(" ISP_ChksumVerify Successful.");
		
        mpPrintMessage("Please reboot system!");
	 //       TurnOffBackLight();

        while (1){}

        return;
    }
    else
    {
        MP_ALERT("AutoISP, handle is null!");
    }
}

#endif


//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
static void MCardEventProcess(DWORD dwEvent)
{
    BYTE bDrvId;//, CurrUsingCard;
    DRIVE *sDrv;
    DWORD i;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &g_psSystemConfig->sFileBrowser;

    bDrvId = SystemCardEvent2DrvIdGet(dwEvent);

    MP_DEBUG("%s - MCardEvent %d, Card Name %s, ID = %d", __FUNCTION__, dwEvent, DriveIndex2DrvName(bDrvId), bDrvId);

    if (bDrvId == 0)
    {
        MP_ALERT("--E-- card detect error !!!");
        return;
    }

    ////////////////////////////////////////////////////////////////////
    //
    //
    //
    ////////////////////////////////////////////////////////////////////
    if (dwEvent == EVENT_CARD_OUT)
    {
        MP_DEBUG("-I- card out, Card Name %s, ID = %d", DriveIndex2DrvName(bDrvId), bDrvId);
        DriveDelete(bDrvId);
        xpgDisableDrive(bDrvId);
        return;
    }

    ////////////////////////////////////////////////////////////////////
    //
    //
    //
    ////////////////////////////////////////////////////////////////////
    MP_DEBUG("-I- card in, Card Name %s, ID = %d", DriveIndex2DrvName(bDrvId), bDrvId);

    if (!SystemCardPresentCheck(bDrvId))
    {
        MP_ALERT("--E-- Card In: %s present check fail !!!", DriveIndex2DrvName(bDrvId));
        return;
    }

    MP_DEBUG("-I- initial card ...");

    if (SystemCardPresentCheck(bDrvId))
    {
        int ret = 0;

        ret = DriveAdd(bDrvId);
        sDrv = DriveGet(bDrvId);

        if (ret > 0)
        {
            MP_DEBUG("driveAdd end");
            DirReset(sDrv);
            DirFirst(sDrv);
            xpgEnableDrive(bDrvId);
            MP_DEBUG("xpgEnableDrive end");
            return;
        }

        // if DriveAdd failed, should be goto here; then show "File Error" and remove card
        if (sDrv->StatusCode == NOT_SUPPORT_FS)
            SystemSetErrEvent(ERR_FILE_SYSTEM);
    }

    MP_ALERT("-E- card in fail, Card Name %s, ID = %d", DriveIndex2DrvName(bDrvId), bDrvId);

    DriveDelete(bDrvId);
    xpgDisableDrive(bDrvId);
}

//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
static void MainEventProcess(DWORD *dwEvent, BYTE *bKeyCode)
{
	SWORD swRet;
	DWORD dwRet=0;


	if (*dwEvent & EVENT_DISK_FULL)
	{
		MP_ALERT("EVENT_DISK_FULL -");
		return;
	}


#if ((!USBDEVICE_CDC_DEBUG) && (SC_USBDEVICE))
    // When connected to PC, ignore all IR key event
    if (SystemCheckUsbdPlugIn())
    {
    #if (SYSTEM_DRIVE == 1)
        // Enable / Disable viewable System Drive, Demo only
        if (*bKeyCode == KEY_SOURCE)
        {
            if (SystemDriveIdGetByLun(SystemDriveLunNumGet(SYS_DRV_ID)) == NULL_DRIVE)
                SystemSysDriveLunEnable(ENABLE);
            else
                SystemSysDriveLunEnable(DISABLE);
        }
    #endif

        *bKeyCode = KEY_NULL;

        return;
    }
#endif

#if (USBOTG_HOST_USBIF || USBOTG_HOST_EYE)
                if (bKeyCode == KEY_TV_O_P)
                {
                    MP_ALERT("===== into test mode for USBOTG0 =====");
                    Api_UsbIfTestMode(USBOTG0);
                }
                else if (bKeyCode == KEY_ALARM)
                {
                    MP_ALERT("===== into test mode for USBOTG1 =====");
                    Api_UsbIfTestMode(USBOTG1);
                }
#endif

#if MAKE_XPG_PLAYER

    if (*bKeyCode != KEY_NULL)
    {
		if ((*dwEvent & EVENT_IR_KEY_DOWN) || (*dwEvent & EVENT_GPIO_KEY))
			dwRet=xpgProcessEvent(XPG_EVENT_KEY_DOWN, *bKeyCode); 
    }
#endif

    // Can't Suspend when USB connect with PC!
#if (SC_USBDEVICE)
    if ( (*bKeyCode == KEY_POWER) && (SystemCheckUsbdPlugIn() == FALSE))
#else
    if (*bKeyCode == KEY_POWER)
#endif

    *bKeyCode = KEY_NULL;
}


//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////

#if TEST_DISPLAY_PANEL
static WORD st_wtmp=60;
void Display_panel(void)
{
	WORD wX,wY,wW,wH;
	ST_IMGWIN *pWin=Idu_GetNextWin();

	wX=696;
	wW=2;
	wY= 265;//(pWin->wHeight-wH)>>1;
	wH=4;//pWin->wHeight*st_wWidth/pWin->wWidth;
	mpClearWin(pWin);
	mpPaintWinArea(pWin, wX, wY, wW, wH, 0xff008080);//0xff008080  RGB2YUV(200,0,0)
	Idu_ChgWin(pWin);
	mpDebugPrint("paint:%d %d %d %d",wX, wY, wW, wH);//paint:696 265 2 4
		if (st_wtmp<350)
		st_wtmp+=2;
	else
		st_wtmp=100;

	//Ui_TimerProcAdd(2000, Display_panel);
}
#endif
#if 0 // for test
static BYTE st_bDisplayindex=0x01,st_bDisplayindex2=0;
void DisplayColorTest(void)
{
	switch (st_bDisplayindex2)
	{
		case 0:
			mpPaintWin(Idu_GetCurrWin(), RGB2YUV(st_bDisplayindex,0,0));
			break;
		case 1:
			mpPaintWin(Idu_GetCurrWin(), RGB2YUV(0,st_bDisplayindex,0));
			break;
		case 2:
			mpPaintWin(Idu_GetCurrWin(), RGB2YUV(0,0,st_bDisplayindex));
			break;
	}
	mpDebugPrint("--st_bDisplayindex=%d",st_bDisplayindex);
	st_bDisplayindex<<=1;
	if (st_bDisplayindex==0)
	{
		st_bDisplayindex=0x01;
		st_bDisplayindex2++;
		if (st_bDisplayindex2>2)
			st_bDisplayindex2=0;
	}
	Ui_TimerProcAdd(2000, DisplayColorTest);
	
}

SWORD MakeTestFile(void)
{
	STREAM *handle;
	SWORD swRet=PASS;
	BYTE *pbBuf=NULL;
	DWORD dwBufSize,i,j,k,w,h,len;

	if (DriveCurIdGet()==NULL_DRIVE)
    {
        MP_ALERT("%s: NULL drive index!", __FUNCTION__);
        return FAIL;
    }
	if (SystemGetFlagReadOnly(DriveCurIdGet()))
	{
        MP_ALERT("Write protected !");
	      TaskSleep(4000);
	      return FAIL;
	}

	handle=(STREAM *)GetNewWeldPhotoHandle();
	if (handle==NULL)
		return FAIL;
	dwBufSize = 1024*600*3;
	pbBuf = (BYTE*)ext_mem_malloc(ALIGN_32(dwBufSize));
	if (pbBuf==NULL)
		return FAIL;

	i=0;
	for (h=0;h<600;h++)
	{
		len=128;
		for (w=0;w<len;w++)
		{
			pbBuf[i++]=255;
			pbBuf[i++]=255;
			pbBuf[i++]=255;
		}
		len+=128;
		for (;w<len;w++)
		{
			pbBuf[i++]=255;
			pbBuf[i++]=255;
			pbBuf[i++]=1;
		}
		len+=128;
		for (;w<len;w++)
		{
			pbBuf[i++]=1;
			pbBuf[i++]=255;
			pbBuf[i++]=255;
		}
		len+=128;
		for (;w<len;w++)
		{
			pbBuf[i++]=0;
			pbBuf[i++]=255;
			pbBuf[i++]=1;
		}
		len+=128;
		for (;w<len;w++)
		{
			pbBuf[i++]=255;
			pbBuf[i++]=0;
			pbBuf[i++]=254;
		}
		len+=128;
		for (;w<len;w++)
		{
			pbBuf[i++]=254;
			pbBuf[i++]=0;
			pbBuf[i++]=0;
		}
		len+=128;
		for (;w<len;w++)
		{
			pbBuf[i++]=0;
			pbBuf[i++]=0;
			pbBuf[i++]=254;
		}
		len+=128;
		for (;w<len;w++)
		{
			pbBuf[i++]=0;
			pbBuf[i++]=0;
			pbBuf[i++]=0;
		}

	}

	FileWrite(handle, (BYTE *) pbBuf, dwBufSize);
	FileClose(handle);
	mpDebugPrint("----- %s:ok! %d", __FUNCTION__,dwBufSize);

	//free memory
	ext_mem_free(pbBuf);

	return PASS;
}

#endif
#if VAUTO_PLAY_VIDEO
void AutoPlayVideoTimer(void)
{
	EnterVideoModePlay();
}
#endif
int main(void)
{
    DWORD dwMainEvent;
    BYTE bKeyCode = 0;
#if HD_ENABLE
    BYTE bPreChkCardIn = 0;
#endif

    TurnOffBackLight();
    if (SystemInit())
    {
        MP_ALERT("-E- SystemInit fail");
    }
#if ( (AUDIO_ON == 1) && (AUDIO_DAC == DAC_ES7240) )
	Gpio_Config2GpioFunc(GPIO_GPIO_13, GPIO_OUTPUT_MODE, GPIO_DATA_HIGH, 1);//set RESETb to low, for DAC_ES7240 of spycamera
#endif
    MP_TRACE_LINE();
#if MCARD_POWER_CTRL
    McardBootUpPowerOff();
#endif
    PlatformInit();
#if OSD_ENABLE
    Idu_OsdSetDefaultPalette();
#endif
//    AddTimerProc(60, (void *) xpgCheckDriveCount);
#if RTC_ENABLE
    Ui_RtcInit();
    StartRtcDisplay();
#endif
    mpDebugPrint("------ Executive file is build at %s %s ------", __DATE__, __TIME__);
    mpDebugPrint("\r\n*** iPlay system init finished!! ***\r\n");
#if Make_TESTCONSOLE
    tConsoleEnable();
#endif
#if Make_DIAGNOSTIC_TC
    TestConsoleInit();
#endif
    // Register callback function for Polling_Event
    SystemPollingEventIdSet(UI_EVENT);
    //SystemPollingEventHandlerRegister(EVENT_CARD_OUT, (void *) mainCardOutHandler);
    //SystemPollingEventHandlerRegister(EVENT_ERROR, (void *) mainErrorHandler);
    //SystemPollingEventHandlerRegister(EVENT_IR_KEY_DOWN, (void *) mainIrKeyDownHandler);
    //(EVENT_GPIO_KEY, (void *) mainGpioKeyHandler);
#if MCARD_POWER_CTRL
    McardCardDetectionEnable();
#endif
#if (SC_USBDEVICE)
    SystemUsbdDetectEnable(); // Enable USB Device detect
#endif

#if SAVE_FILE_TO_SPI
    spifs_FileSystemInit();
    g_boSaveFileToSPI = 1;
#endif


    //xpgSearchAndGotoPage("Preview");
    //xpgUpdateStage();
    //mpClearWin(Idu_GetCurrWin());
    //mpCopyWin(Idu_GetNextWin(), Idu_GetCurrWin());
    //xpgStartUpdateClock();

#if AUTO_SLEEP_ENABLE||AUTO_POWER_OFF
CheckAutoSleepOrAutoOff();
#endif
    //API_SelectChannel(1);
#if (SENSOR_ENABLE == ENABLE)
#if TEST_DISPLAY_PANEL
	Ui_TimerProcAdd(6000, Display_panel);
#else
#if (PRODUCT_UI==UI_WELDING)
if (g_psUnsaveParam->bStandby)
	Ui_TimerProcAdd(1000, uiCb_CheckInStandby);
else if (SystemGetStatus(SYS_STATUS_INIT))
	Ui_TimerProcAdd(10, uiCb_CheckBattery);
else
#endif
	Timer_FirstEnterCamPreview();
//NonxpgEnterPhotoView();
//	Ui_TimerProcAdd(2000, MakeTestFile);
//			mpPaintWin(Idu_GetCurrWin(), RGB2YUV(st_bDisplayindex,0,0));
#endif   

#elif VAUTO_PLAY_VIDEO
	Ui_TimerProcAdd(2000, AutoPlayVideoTimer);

#endif

    //Ui_TimerProcAdd(10, SetupMenuInit);

    //g_bXpgStatus = XPG_MODE_MODESEL;
    //SetMotionDetectionEnable(1);

    initRecordDummyData();


    while (1)
    {
        if (MainWaitEvent(&dwMainEvent, 0) == OS_STATUS_OK)
        {

#if AUTO_SLEEP_ENABLE||AUTO_POWER_OFF
            //////////////////////////////////////////////////////////////////
            //  CheckAutoSleepOrAutoOff
            //////////////////////////////////////////////////////////////////
			if (dwMainEvent & (EVENT_CARD_OUT|EVENT_CARD_IN|EVENT_USB0_CHG|EVENT_USB1_CHG|EVENT_AUDIO_END|EVENT_VIDEO_END|EVENT_GPIO_KEY|EVENT_IR|EVENT_LONG_PRESS_KEY|EVENT_MOTION_DETECT_OK))
			{
				#if AUTO_SLEEP_ENABLE
				if (IsSleepState())
				{
					BreakSleepState();
					dwMainEvent &= ~(EVENT_GPIO_KEY|EVENT_LONG_PRESS_KEY);
				}
				#endif
				CheckAutoSleepOrAutoOff();
			}
#endif
						
#if 1//(CyclicRecording)
            //////////////////////////////////////////////////////////////////
            //  EVENT_DISK_FULL
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_DISK_FULL)
            {
                MainEventProcess(&dwMainEvent, NULL);
            }
#endif
#if (LowBattery)
            //////////////////////////////////////////////////////////////////
            //  EVENT_LOW_BATTERY
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_LOW_BATTERY)
            {
                MainEventProcess(&dwMainEvent, NULL);
            }
#endif
            //////////////////////////////////////////////////////////////////
            //  EVENT_TIMER
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_TIMER)
            {
                if (Ui_TimerProcessCheck())
                    Ui_TimerProcessHandle();
            }
            //////////////////////////////////////////////////////////////////
            //  EVENT_CARD_OUT
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_CARD_OUT)
            {
                MainEventProcess(&dwMainEvent, NULL);
                MCardEventProcess(EVENT_CARD_OUT);
            }
            //////////////////////////////////////////////////////////////////
            //  EVENT_CARD_IN
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_CARD_IN)
            {
#if HD_ENABLE
                bPreChkCardIn = 1;
#endif
                MainEventProcess(&dwMainEvent, NULL);
                MCardEventProcess(EVENT_CARD_IN);
            }
            //////////////////////////////////////////////////////////////////
            //  EVENT_FETAL_ERROR_WHEN_CARD_WRITING_FAILURE
            ////////////////////////////////////////////////////////////////// 
            if (dwMainEvent & EVENT_CARD_FATAL_ERROR)
			 {
#if (RECORD_ENABLE == ENABLE)
                Camcorder_RecordStop();
#endif
            }
            //////////////////////////////////////////////////////////////////
            //  HD
            //////////////////////////////////////////////////////////////////
#if HD_ENABLE
            if(!bPreChkCardIn)
            {
                bPreChkCardIn = 1;
                xpgEnableDrive(HD);
            }
#endif
#if SC_USBDEVICE
            //////////////////////////////////////////////////////////////////
            //  EVENT_USB0_CHG
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_USB0_CHG)
            {
                MP_ALERT("- EVENT_USB0_CHG -");
                MainEventProcess(&dwMainEvent, NULL);
                Ui_UsbdDetectEvent(USBOTG0);
            }
            //////////////////////////////////////////////////////////////////
            //  EVENT_USB1_CHG
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_USB1_CHG)
            {
                Ui_UsbdDetectEvent(USBOTG1);
            }
#endif
#if (USBOTG_WEB_CAM || USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG)
            //////////////////////////////////////////////////////////////////
            //  EVENT_WEB_CAM_IN & EVENT_WEB_CAM_OUT
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_WEB_CAM_IN)
            {
                WebCamEventProcess(EVENT_WEB_CAM_IN);
            }
            if (dwMainEvent & EVENT_WEB_CAM_OUT)
            {
                WebCamEventProcess(EVENT_WEB_CAM_OUT);
            }
#endif
#if AUTO_DEMO
            //////////////////////////////////////////////////////////////////
            //  EVENT_AUTO_DEMO
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_AUTO_DEMO)
            {
                xpgAutoDemoCheck();
            }
#endif
            //////////////////////////////////////////////////////////////////
            //  EVENT_AUDIO_END
            //////////////////////////////////////////////////////////////////
#if AUDIO_ON && MAKE_XPG_PLAYER
            if (dwMainEvent & EVENT_AUDIO_END)
            {
                xpgAudioEnd();
            }
#endif
            //////////////////////////////////////////////////////////////////
            //  EVENT_VIDEO_END
            //////////////////////////////////////////////////////////////////
#if VIDEO_ON
            if (dwMainEvent & EVENT_VIDEO_END)
            {
				#if MAKE_XPG_PLAYER
                if(g_bXpgStatus != XPG_MODE_VIDEOMENU)
                    xpgVideoEnd();
				#else
				NonxpgVideoEnd();
				#endif
            }
#endif
            //////////////////////////////////////////////////////////////////
            //  EVENT_YOUTUBE_END
            //////////////////////////////////////////////////////////////////
#if NETWARE_ENABLE
            if (dwMainEvent & EVENT_YOUTUBE_END)
            {
                    xpgYouTubeEnd();
            }
#endif
            //////////////////////////////////////////////////////////////////
            //  EVENT_BLUETOOTH
            //////////////////////////////////////////////////////////////////
#if (BT_XPG_UI == ENABLE)
            if (dwMainEvent & EVENT_BT_DONGLE_OUT)
            {
                xpgBtProcessDongleOut();
            }
            if (dwMainEvent & EVENT_BLUETOOTH)
            {
                BtUiMessageReceiver();
            }
            if (dwMainEvent & EVENT_BT_DONGLE_IN)
            {
                xpgBtProcessDongleIn();
            }
#endif
            //////////////////////////////////////////////////////////////////
            //  EVENT_ERROR
            //////////////////////////////////////////////////////////////////
#if 0
            if (dwMainEvent & EVENT_ERROR)
            {
                volatile DWORD dwCode = g_psSystemConfig->dwErrorCode;
                g_psSystemConfig->dwErrorCode = 0;
            }
#endif
#if 0//SC_USBDEVICE
            //////////////////////////////////////////////////////////////////
            //  EVENT_PRINT_INIT
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_PRINT_INIT)	// print init
            {
                MP_ALERT("\nPrint Init>>>");
            }
            //////////////////////////////////////////////////////////////////
            //  EVENT_PRINT_PRINTING
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_PRINT_PRINTING)	// printing
            {
                MP_ALERT("\nPrint-ing>>>");
            }
            //////////////////////////////////////////////////////////////////
            //  EVENT_PRINT_FINISH
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_PRINT_FINISH)	// print finished or cancel with normal
            {
                MP_ALERT("\nPrint Finish>>>");
                //UsbdPrintFinish(Api_UsbdGetWhichOtgConnectedPrinter());
            }
            //////////////////////////////////////////////////////////////////
            //  EVENT_PRINT_EXIT
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & EVENT_PRINT_ERROR)	// print cancel with error
            {
                MP_ALERT("\nPrint Error>>>");
                //UsbdPrintExitWithError(Api_UsbdGetWhichOtgConnectedPrinter());
            }
#endif
            //////////////////////////////////////////////////////////////////
            //  EVENT_GPIO_KEY_ref
            //////////////////////////////////////////////////////////////////
#if 0
            if (dwMainEvent & EVENT_GPIO_KEY)
            {
                MP_DEBUG("KEYEVENT-2 !!");
                ST_KEY_INFO stKeyInfo;
                //while (Ui_GetGpioKeyFromMsg(&stKeyInfo) == TRUE){
                    mpDebugPrint("### Ui_GetGpioKeyFromMsg(&stKeyInfo) == TRUE ###");
                    MainEventProcess(&dwMainEvent, (BYTE *) &stKeyInfo);
                //}
            }
#endif
            //////////////////////////////////////////////////////////////////
            //  EVENT_GPIO_KEY
            //////////////////////////////////////////////////////////////////
            if (dwMainEvent & (EVENT_GPIO_KEY|EVENT_LONG_PRESS_KEY))
            {
                bKeyCode = Ui_GetKey();
                //Ui_SetKey(KEY_NULL);
                MainEventProcess(&dwMainEvent, &bKeyCode);
            }
            
            //////////////////////////////////////////////////////////////////
            //  EVENT_IR_KEY_DOWN
            //  before EVENT_IR, will have one EVENT_IR_KEY_DOWN
            //////////////////////////////////////////////////////////////////
#if (IR_ENABLE)
            //mpDebugPrint("dwMainEvent=%u", dwMainEvent);
            if ((dwMainEvent & EVENT_IR_KEY_DOWN) || (dwMainEvent & EVENT_IR))
            {
                bKeyCode = Ui_GetIrKey();
                Ui_SetIrKey(KEY_NULL);
                MainEventProcess(&dwMainEvent, &bKeyCode);
            }
#endif
            //////////////////////////////////////////////////////////////////
            //  EVENT_TOUCH_COLTROLLER
            //////////////////////////////////////////////////////////////////
#if (TOUCH_CONTROLLER_ENABLE == ENABLE)
            if (dwMainEvent & EVENT_TOUCH_COLTROLLER)
            {
                //dwMainEvent &= ~EVENT_TOUCH_COLTROLLER;
                uiTouchMsgReceiver();
            }
#endif

#if TSPI_ENBALE
            if (dwMainEvent & EVENT_TSPI_START)
            {
                TSPI_Receiver();
            }
#endif

#if (PRODUCT_UI==UI_WELDING) && SENSOR_ENABLE
            if (dwMainEvent & EVENT_PROC_DATA)
            {
                //ProcWinData();
					Proc_SensorData_State();
            }
#endif
#if IPW_FAST_MODE
            if (dwMainEvent & EVENT_DISP_DATA)
            {
					Idu_ChgWin(Idu_GetNextWin());
            }
#endif

        }   // end of if-condition: if(MainWaitEvent(&dwMainEvent, dwNextEvent) == OS_STATUS_OK)
		

    }   // end of while-loop
    return 0;
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////

static SWORD SystemInit(void)
{
    //////////////////////////////////////////////////////////////////
    //
    // System init
    //
    //////////////////////////////////////////////////////////////////
    //clear sram when system set up
    #define SRAM_ADR    0xB8000000

#if (CHIP_VER_MSB == CHIP_VER_615)
    memset((char *) SRAM_ADR, 0, 16 * 1024);
#else
    memset((char *) SRAM_ADR, 0, 24 * 1024);
#endif

    mpDebugPrint("SystemInit @ %s : %d", __FILE__, __LINE__);

    ClkInit();
    //////////////////////////////////////////////////////////////////
    //
    // System parameter init
    //
    //////////////////////////////////////////////////////////////////
    SystemChkChipVer();
    SystemConfigInit();
    SystemSetStatus(SYS_STATUS_INIT);

    if (SystemPanelPixelClockGet() > 100000000)
        Clock_CpuClockSelSet(CPUCKS_PLL2_DIV_2);

    //////////////////////////////////////////////////////////////////
    //
    // OS/System Init
    //
    //////////////////////////////////////////////////////////////////
    OsInit((DWORD *)SystemGetMemAddr(OS_BUF_MEM_ID), SystemGetMemSize(OS_BUF_MEM_ID));
    SystemExceptionInit();

#if (SC_USBDEVICE)
    SystemUsbdDetectDisable(); // Disable USB Device detect
#endif

#if 0//(MP_TRACE_INSIGHT_ENABLE || Make_TESTCONSOLE || Make_DIAGNOSTIC_TC || defined(MAGIC_SENSOR_INTERFACE_ENABLE))
    #if (CHIP_VER_MSB == CHIP_VER_615)
        #if ((DEBUG_COM_PORT == HUART_A_INDEX) || (DEBUG_COM_PORT == HUART_B_INDEX))
        SystemIntEna(IM_UART);
        #endif
    #else   // MP650/660
        #if (DEBUG_COM_PORT == HUART_A_INDEX)
        SystemIntEna(IM_UART);
        #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        SystemIntEna(IM_UART2);
        #endif
    #endif
#endif

    //////////////////////////////////////////////////////////////////
    //
    // Module Initial
    //
    //////////////////////////////////////////////////////////////////
#ifdef MAGIC_SENSOR_INTERFACE_ENABLE
    mpx_toolInit();
#endif

    extern void McardIsr();
    extern void DmaIduIsr(void);

    DmaIntHandleRegister(IM_IDUDM, DmaIduIsr);
#if (CHIP_VER_MSB != CHIP_VER_615)
    extern void CduIsr();
    extern void ScalerIsr();

    SystemIntHandleRegister(IM_IPU, ScalerIsr);
    SystemIntHandleRegister(IM_JPEG, CduIsr);
#endif


/*
    DisplayInit(DISPLAY_INIT_DEFAULT_RESOLUTION);
    FileSystemInit();
    if (XpgInit(SD_MMC, g_mstScreenTable[0].dwXPGTag))
    {
        TurnOnBackLight();
        mpPrintMessage("XpgInit fail!");
        mpPrintMessage("Please do ISP for XPG pages...");
        mpPrintMessage("Please reboot system!");
        SystemClearStatus(SYS_STATUS_INIT);
        SystemSetStatus(SYS_STATUS_ERROR);
        IntDisable();
        while (1) { }
    }
    TurnOnBackLight();
    */
#if(RECORD_ENABLE)
    aviStreamTaskCreate();
    recopcontrolInit();
#endif
#if (RECORD_ENABLE||SENSOR_WITH_DISPLAY)
	PreviewRecStateTaskInit();
#endif
#if 0 // it is the earlist bootup sensor preview    
    Camcorder_BootPreview();
#endif
    
#if (VIDEO_ENABLE)
  #if (CHIP_VER_MSB != CHIP_VER_615)
    extern void MpvIsr();

    SystemIntHandleRegister(IM_MPV, MpvIsr);
  #endif
#endif

    SystemIntHandleRegister(IM_MCARD, McardIsr);

#if (CHIP_VER_MSB != CHIP_VER_615)
    SystemIntHandleRegister(IM_SDIO_EXT, McardIsr);
#endif

    EventCreate(UI_EVENT, OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE, 0);
    EventCreate(JPEG_LOAD_DATA_EVENT_ID1, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
    EventCreate(SCALING_FINISH_EVENT_ID, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);

    Gpio_IntInit();

#if (I2C_FUNCTION & I2C_HW_MASTER_MODE)
    I2CM_Init();
#endif
#if TSPI_ENBALE
	TSPI_Init();
#endif
#if (PRODUCT_UI==UI_WELDING)
//CPU初始化完成
	TspiSendSimpleInfo0xAF(0x04);
//--Battery info
	TspiSendCmdPolling0xA4(0x0b);
//--standby or power on
	TspiSendCmdPolling0xA4(0x0c);
#endif
    FileSystemInit();

    Ui_TimerProcInit();


 #if BMP_FONT_ENABLE
   FontInit();	// for display out the font
#endif
#if OSD_LINE_NUM
	OsdLineInit();
#endif
#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
    ISP_NandInit(DEFAULT_RES_ADDR);
#endif

#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
	  checkNandIspRegion(RES_TAG);
#endif

#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
	  checkNandIspRegion(AP_TAG);
#endif

    //////////////////////////////////////////////////////////////////
    //
    // XPG init
    //
    //////////////////////////////////////////////////////////////////
    DisplayInit(DISPLAY_INIT_DEFAULT_RESOLUTION);

#if MAKE_XPG_PLAYER
    if (XpgInit(SD_MMC, g_mstScreenTable[0].dwXPGTag))
    {
        TurnOnBackLight();
        mpPrintMessage("XpgInit fail!");
        mpPrintMessage("Please do ISP for XPG pages...");
        mpPrintMessage("Please reboot system!");
        SystemClearStatus(SYS_STATUS_INIT);
        SystemSetStatus(SYS_STATUS_ERROR);
        IntDisable();
        while (1) { }
    }


    TurnOnBackLight();
    xpgUpdateFuncPtrInit();

#else

#if ISP_FUNC_ENABLE
            AutoISP(SD_MMC);
#endif
        TurnOnBackLight();

#endif

    //////////////////////////////////////////////////////////////////
    //
    // Non-removeable Storage
    //
    //////////////////////////////////////////////////////////////////

#if NAND_ENABLE

    OnBoardNandSpiInit(NAND);

    #if 0   // to erase nand flash
    SystemDeviceRawFormat(NAND, 0);
    OnBoardNandSpiInit(NAND);
    #endif

#endif //NAND_ENABLE

#if SPI_STORAGE_ENABLE
    OnBoardNandSpiInit(SPI_FLASH);
#endif

/* move here because System Drive may be configured to SPI Flash */
#if (NAND_ENABLE && SYSTEM_DRIVE)
    McardDevInfoLoad(SYS_DRV_ID);
    AddTimerProc(10000, McardDevInfoStore); // 10sec
#endif

    //////////////////////////////////////////////////////////////////
    //
    // 1. Setting Value Initial
    // 2. Readback setting from ISP region or system drive
    //
    //////////////////////////////////////////////////////////////////
    SetupMenuInit();
#if RTC_ENABLE
	CheckRTCTime();
#endif
    //////////////////////////////////////////////////////////////////
    //
    // Component initial
    //
    //////////////////////////////////////////////////////////////////
#if (VIDEO_ON || AUDIO_ON)
    MovieTaskCreate();         //movie play task , always run , controlled by MOVIEINSTRUCTION.
#endif

#if (VIDEO_ENABLE && MJPEG_TOGGLE)
    MjpegTaskInit();
#endif

#if (SC_USBHOST||SC_USBDEVICE)
    USB_OTG_FUNC eUsbOtg_0_Func = NONE_FUNC;
    USB_OTG_FUNC eUsbOtg_1_Func = NONE_FUNC;

    if (SC_USBOTG_0 == ENABLE)
        eUsbOtg_0_Func = OTG_FUNC;
    if (SC_USBOTG_1 == ENABLE)
        eUsbOtg_1_Func = OTG_FUNC;

    Api_UsbFunctionSet(eUsbOtg_0_Func, eUsbOtg_1_Func);
#if (USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG)
    BabyMonitorInit();
#endif

#if (USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG)
    UsbOtgDevExternTaskInit(); // USB Device Side Monitor
#endif

#endif // (SC_USBHOST||SC_USBDEVICE)

    //////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////
#if (IR_ENABLE == 1)
    Ui_IR_Init();
#endif

#if (TOUCH_CONTROLLER_ENABLE == ENABLE)
    TouchCtrllerInit();
#endif

#if (AUDIO_DAC==DAC_ALC5621)
    Codec_ElecSwitch_Init();
    //Codec_ElecSwitch_RecordMode();
#endif

#if (PRODUCT_UI==UI_WELDING)
	if (g_psUnsaveParam->bStandby)
#endif
    SystemClearStatus(SYS_STATUS_INIT);
    mpDebugPrint("System init ok\r\n");

    return 0;
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
//extern BOOL NoDrowIconFlag;
extern STACTFUNC actionFunctions[];

static SWORD XpgInit(BYTE driveId, DWORD dwXPGTag)
{
    BOOL boDriveAdded = FALSE;
    BOOL boXpgLoadOK = FALSE;
    //BOOL boLoadFromFlash = XPG_LOAD_FROM_FLASH;     //!(dwSDstatus & 0x00000001);
    STREAM *handle = NULL;

    xpgUiActionFunctionRegister(actionFunctions);
    //xpgUiDrawKeyIconFunctionRegister(xpgDrawKeyIcon);

    fbrowser_xpgClearCatchFunctionRegister(xpgClearCatch);
    //fbrowser_xpgUpdateIconAniFunctionRegister(xpgUpdateIconAni);
    fbrowser_xpgChangeDriveFunctionRegister(xpgChangeDrive);

    if (SystemCardPlugInCheck(driveId))
    {   // SD plug
        MP_DEBUG("-I- %s card Plugged", DriveIndex2DrvName(driveId));

        // if card was plugged but not present, waiting some time for M-Card
        // Timing issue
        if (!SystemCardPresentCheck(driveId))
            TaskSleep(600);

        if (!SystemCardPresentCheck(driveId))
        {
            MP_DEBUG("-E- %s present check fail", DriveIndex2DrvName(driveId));
        }
        else
        {
            if (!(boDriveAdded = DriveAdd(driveId)))
                MP_DEBUG("-E- %s card : DriveAdd fail", DriveIndex2DrvName(driveId));

            DriveChange(driveId);
        }

        if (boDriveAdded)
        {
            MP_DEBUG("-I- %s card : DriveAdd OK", DriveIndex2DrvName(driveId));
#if ISP_FUNC_ENABLE
            AutoISP(driveId);
#endif

            //if (boLoadFromFlash)
            //    boXpgLoadOK = xpgLoadFromFlash(dwXPGTag);

            if (!boXpgLoadOK)
            {
#if (SCREEN_TABLE_TOTAL_NUM == 4)
                switch(dwXPGTag)
                {
                case ISP_TAG_XP1:
                    handle = (STREAM *) FileExtSearchOpen(driveId, "XP1", 512);
                    break;

                case ISP_TAG_XP3:
                    handle = (STREAM *) FileExtSearchOpen(driveId, "XP3", 512);
                    break;

                case ISP_TAG_XP2:
                default:
                    handle = (STREAM *) FileExtSearchOpen(driveId, "XP2", 512);
                    break;
                }
#else
                handle = (STREAM *) FileExtSearchOpen(driveId, "XPG", 512);
#endif

                if (handle != NULL)
                {
                    boXpgLoadOK = xpgPreloadAndOpen(handle);
                    FileClose(handle);
                }
            }
            mpDebugPrint("Xpg Memory needed size = %d KB", xpgGetXpgMemorySize());
        }
    }

    if (!boXpgLoadOK)
        boXpgLoadOK = xpgLoadFromFlash(dwXPGTag);

    if (boXpgLoadOK)
    {
        xpgInitCanvas();

        MP_DEBUG("UI initialize!!\r\n");

        //xpgInitThumbBuffer(THUMB_COUNT, THUMB_WIDTH * THUMB_HEIGHT * 2);
        //g_pDrawIconBuffer = xpgMalloc(ICON_BUFFER_SIZE);	// alloc buffer for Icon Capture screen

#if (PRODUCT_UI==UI_WELDING)
		SWORD swRet;
		STXPGPAGE *pstPage;

		if (g_psUnsaveParam->bStandby)
		{
				pstPage=xpgSearchtoPageWithAction("Charge");
		}
		else
		{
			pstPage=xpgSearchtoPageWithAction("Logo");
		}
        if (pstPage!=NULL)
        {
			xpgUpdateStage();
        }
		else
		{
			mpClearWin(Idu_GetCurrWin());
		}
#else
        if (xpgSearchtoPageWithAction("Logo")!=NULL)
        //if (xpgSearchtoPageWithAction("Main")!=NULL)
        {
			xpgUpdateStage();
        }
		else
		{
			mpClearWin(Idu_GetCurrWin());
		}
#endif
        MP_DEBUG("UI initialize OK 1!!\r\n");
        return 0;
    }
    else
    {
        mpDebugPrint("-E- XPG not found!! \r\n");

        return -1;
    }
}                               //end of XpgInit



    //////////////////////////////////////////////////////////////////
    //
    // On board Nand flash or SPI flash initalization
    //
    //////////////////////////////////////////////////////////////////

/* note: auto re-partitioning when partition size mismatch is too dangerous!!  So, disable it. */
#define FORCE_REPARTITION_SYS_DRV_IF_SIZE_CHANGE        ENABLE

#define FORCE_REFORMATTING_IF_FAILED_ADDING_DRIVES      DISABLE

/* note: auto re-formatting the drive(s) after DriveAdd() failed is for RD engineering purpose only. We disable it for production stage.
 *
 * The device (ex: NAND device) containing System Drive on it must have 2 well-added partition drives (SysDrv and UsrDrv) to work normally.
 * So, if any one partition or both partitions of this device was not well-added after DriveAdd(), we force to re-format both partitions
 * of this device if FORCE_REFORMATTING_IF_FAILED_ADDING_DRIVES is enabled. We leave it alone if FORCE_REFORMATTING_IF_FAILED_ADDING_DRIVES
 * is disabled.
 *
 * Note that when FORCE_REFORMATTING_IF_FAILED_ADDING_DRIVES is disabled, the re-formatting of drives will only be performed after
 * re-partitioning procedure of the device for System Drive !
 */

static SWORD OnBoardNandSpiInit(E_DRIVE_INDEX_ID drvId)
{
    DRIVE_PHY_DEV_ID phyDevID;
    DWORD part2_start_lba, part2_SectorNr, part2_BlockSize;
    SWORD ret;
    DRIVE *sDrv;
    BOOL  same_devId_of_sysDrv;
    BOOL  need_format;
    BOOL  need_partitioning;
    BYTE  part2_drv_index;
    DWORD part2_drv_size, total_SectorNr;
    DWORD hiddenSec_cnt;

    MP_DEBUG("Enter %s(drvId = %d) ...", __FUNCTION__, drvId);
    phyDevID = DriveIndex2PhyDevID(drvId);

    if ((phyDevID != DEV_NAND) && (phyDevID != DEV_SPI_FLASH))
    {
        MP_ALERT("%s: Error! drvId parameter (= %d) is not the NAND or SPI device !\r\n", __FUNCTION__, drvId);
        return FAIL;
    }

    MP_DEBUG("drvId = %d => drive name = %s, phyDevID = %d", drvId, DriveIndex2DrvName(drvId), phyDevID);

    if (SystemDeviceInit(drvId) == FAIL)
    {
        MP_ALERT("%s: %s card initial fail !\r\n", __FUNCTION__, drvId, DriveIndex2DrvName(drvId));
        ret = FAIL;
        goto L_xpgCard_processing_ending;
    }

    if (!SystemCardPresentCheck(drvId))
    {
        MP_ALERT("%s: %s card not present !\r\n", __FUNCTION__, drvId, DriveIndex2DrvName(drvId));
        ret = FAIL;
        goto L_xpgCard_processing_ending;
    }

    if (DriveIndex2PhyDevID(SYS_DRV_ID) == phyDevID)
        same_devId_of_sysDrv = TRUE;
    else
        same_devId_of_sysDrv = FALSE;

#if 1 //for debug info
    MP_DEBUG("%s: Mcard_GetCapacity(phyDevID = %d) = %lu (sectors).", __FUNCTION__, phyDevID, Mcard_GetCapacity(phyDevID));
    ret = DumpDevPartitionTableFromMBR(phyDevID);
#endif

    if (!same_devId_of_sysDrv) /* this device is not the device for System Drive */
    {
        MP_DEBUG("%s: (drvId = %d) => this device is not the device for System Drive ...", __FUNCTION__, drvId);

        need_format = FALSE;

        /* DriveAdd() directly */
        if (!DriveAdd(drvId)) /* none of partition/drive added */
        {
    #if (FORCE_REFORMATTING_IF_FAILED_ADDING_DRIVES == ENABLE)
            need_format = TRUE;
    #else
            MP_ALERT("-E- (drvId = %d) DriveAdd() failed !\r\n", drvId);
            ret = FAIL;
    #endif
        }
        else /* at least one partition/drive added */
        {
            sDrv = DriveChange(drvId);
            if ((sDrv->StatusCode == FS_SUCCEED) && (sDrv->Flag.Present == 1))  //this drive has already been added
            {
                DirReset(sDrv);
                DirFirst(sDrv);
                mpDebugPrint("%s:%d", __FUNCTION__, __LINE__);
                xpgEnableDrive(drvId);

            }
            else
            {
    #if (FORCE_REFORMATTING_IF_FAILED_ADDING_DRIVES == ENABLE)
                need_format = TRUE;
    #else
                MP_ALERT("-E- The partition drive of (drvId = %d) was not well-added !  Pls check it !!\r\n", drvId);
                ret = FAIL;
    #endif
            }
        }

        if (need_format)
        {
            if (ret = Fat32_Format(DriveGet(drvId), NULL) != PASS)
                if (ret = Fat16_Format(DriveGet(drvId), NULL) != PASS)
                    if (ret = Fat12_Format(DriveGet(drvId), NULL) != PASS)
                        MP_ALERT("-E- (DrvId = %d) FAT32/FAT16/FAT12 formatting all failed ! => Pls check it ...", drvId);

            DriveDelete(drvId);

            /* after drive formatting, DriveAdd() again */
            if (DriveAdd(drvId)) /* at least one partition/drive added */
            {
                sDrv = DriveChange(drvId);
                if ((sDrv->StatusCode == FS_SUCCEED) && (sDrv->Flag.Present == 1))  //this drive has already been added
                {
                    DirReset(sDrv);
                    DirFirst(sDrv);
                    mpDebugPrint("%s:%d", __FUNCTION__, __LINE__);
                    xpgEnableDrive(drvId);
                    ret = PASS;
                }
                else
                {
                    MP_ALERT("-E- The partition drive of (drvId = %d) was not well-added !  Pls check it !!\r\n", drvId);
                    ret = FAIL;
                }
            }
            else /* none of partition/drive added */
            {
                MP_ALERT("-E- (drvId = %d) DriveAdd() failed !\r\n", drvId);
                ret = FAIL;
            }
        }

        goto L_xpgCard_processing_ending;
    }
    else /* this device is the device for System Drive => it must have 2 partitions on the device */
    {
        MP_DEBUG("%s: (drvId = %d) => this device is the device for System Drive ...", __FUNCTION__, drvId);

        /* we wanna get partition 2 info from MBR partition table for later partition size calculation */
        if (phyDevID == DEV_NAND)
            part2_drv_index = NAND_PART2;
        else if (phyDevID == DEV_SPI_FLASH)
            part2_drv_index = SPI_FLASH_PART2;

        need_format = need_partitioning = FALSE;
        total_SectorNr = Mcard_GetCapacity(phyDevID);
        if (total_SectorNr == 0)
        {
            MP_ALERT("%s: Error !  Mcard_GetCapacity(phyDevID = %d) == 0 !  Check if wrong device ID, otherwise, Mcard problem !!", __FUNCTION__, phyDevID);
            ret = FAIL;
            goto L_xpgCard_processing_ending;
        }

        ret = GetDrvPartitionInfoFromMBR(part2_drv_index, NULL, NULL, &part2_start_lba, &part2_SectorNr, &part2_BlockSize);

        /* note: partition size calculation must avoid exceeding 32-bit limit when device size is very big */
        if (ret == FS_SCAN_FAIL)
        {
            need_partitioning = TRUE;

            if (SYS_DRV_ID == part2_drv_index) /* SYS_DRV_ID is the 2nd partition of the device */
                part2_drv_size = SYS_DRV_SIZE;
            else if (SYS_DRV_ID == part2_drv_index - 1) /* SYS_DRV_ID is the 1st partition of the device */
            {
                /* note: DiskPartitioning_WithMaxTwoPartitions() reserves less hidden sectors in front of the boot sector of 1st partition for tiny-size device */
                hiddenSec_cnt = (MB_SIZE(total_SectorNr * SECTOR_SIZE) < 2) ? Small_BPB_HiddSec : Default_BPB_HiddSec;
                part2_drv_size = (((total_SectorNr - hiddenSec_cnt) * (SECTOR_SIZE >> 8)) >> 10) - SYS_DRV_SIZE; /* unit: 256 KB */
            }
        }
        else if (part2_SectorNr > 0)  /* already have 2nd partition */
        {
    #if (FORCE_REPARTITION_SYS_DRV_IF_SIZE_CHANGE == ENABLE)
            if (SYS_DRV_ID == part2_drv_index) /* SYS_DRV_ID is the 2nd partition of the device */
            {
                part2_drv_size = ((part2_SectorNr * (part2_BlockSize >> 8)) >> 10); /* unit: 256 KB */
                MP_DEBUG("%s: size of 2nd partition = %lu  (unit: 256 KB)", __FUNCTION__, part2_drv_size);
                if (SYS_DRV_SIZE != part2_drv_size)
                {
                    need_partitioning = TRUE;

                    part2_drv_size = SYS_DRV_SIZE;
                    MP_ALERT("%s: partition size mismatch with defined SYS_DRV_SIZE !", __FUNCTION__);
                }
            }
            else if (SYS_DRV_ID == part2_drv_index - 1) /* SYS_DRV_ID is the 1st partition of the device */
            {
                DWORD part1_drv_size, part1_start_lba, part1_SectorNr, part1_BlockSize;
                GetDrvPartitionInfoFromMBR(part2_drv_index - 1, NULL, NULL, &part1_start_lba, &part1_SectorNr, &part1_BlockSize);
                hiddenSec_cnt = part1_start_lba;
                part1_drv_size = (((total_SectorNr - hiddenSec_cnt - part2_SectorNr) * (part1_BlockSize >> 8)) >> 10); /* unit: 256 KB */
                MP_DEBUG("%s: size of 1st partition = %lu  (unit: 256 KB)", __FUNCTION__, part1_drv_size);
                if (SYS_DRV_SIZE != part1_drv_size)
                {
                    need_partitioning = TRUE;

                    /* note: DiskPartitioning_WithMaxTwoPartitions() reserves less hidden sectors in front of the boot sector of 1st partition for tiny-size device */
                    hiddenSec_cnt = (MB_SIZE(total_SectorNr * SECTOR_SIZE) < 2) ? Small_BPB_HiddSec : Default_BPB_HiddSec;
                    part2_drv_size = (((total_SectorNr - hiddenSec_cnt) * (SECTOR_SIZE >> 8)) >> 10) - SYS_DRV_SIZE; /* unit: 256 KB */
                    MP_ALERT("%s: partition size mismatch with defined SYS_DRV_SIZE !", __FUNCTION__);
                }
            }
    #endif //FORCE_REPARTITION_SYS_DRV_IF_SIZE_CHANGE
        }
        else if (part2_SectorNr == 0) /* only single partition on the device */
        {
            need_partitioning = TRUE;

            if (SYS_DRV_ID == part2_drv_index) /* SYS_DRV_ID is the 2nd partition of the device */
                part2_drv_size = SYS_DRV_SIZE;
            else if (SYS_DRV_ID == part2_drv_index - 1) /* SYS_DRV_ID is the 1st partition of the device */
            {
                /* note: DiskPartitioning_WithMaxTwoPartitions() reserves less hidden sectors in front of the boot sector of 1st partition for tiny-size device */
                hiddenSec_cnt = (MB_SIZE(total_SectorNr * SECTOR_SIZE) < 2) ? Small_BPB_HiddSec : Default_BPB_HiddSec;
                part2_drv_size = (((total_SectorNr - hiddenSec_cnt) * (SECTOR_SIZE >> 8)) >> 10) - SYS_DRV_SIZE; /* unit: 256 KB */
            }
        }

        if (need_partitioning)
        {
            MP_ALERT("%s: Perform disk partitioning for (drvId = %d,  physical device Id = %d) ...", __FUNCTION__, drvId, phyDevID);
            if (ret = DiskPartitioning_WithMaxTwoPartitions(drvId, part2_drv_size) != PASS)
            {
                MP_ALERT("%s: Error ! DiskPartitioning_WithMaxTwoPartitions() failed for drvID (= %d) ! => Pls check it ...", __FUNCTION__, drvId);
                ret = FAIL;
                goto L_xpgCard_processing_ending; /* disk partitioning failed, cannot go ahead for drive formatting and DriveAdd() */
            }
            else
                need_format = TRUE;
        }

        if (!need_format)
        {
            /* DriveAdd() directly */
            if (DriveAdd(drvId) < 2) /* not both partition drives (SysDrv and UsrDrv) are well-added */
            {
    #if (FORCE_REFORMATTING_IF_FAILED_ADDING_DRIVES == ENABLE)
                need_format = TRUE;
    #else
                MP_ALERT("%s: -E- Not both SysDrv and UsrDrv drives are well-added !  Pls check it !!\r\n", __FUNCTION__);
                ret = FAIL;
    #endif
            }
            else /* SysDrv and UsrDrv partition drives are both well-added */
            {
                sDrv = DriveChange(drvId);
                DirReset(sDrv);
                DirFirst(sDrv);
                mpDebugPrint("%s:%d", __FUNCTION__, __LINE__);
                xpgEnableDrive(drvId);
            }
        }

        if (need_format)
        {
            MP_ALERT("%s: (drvId = %d) => Need to perform formatting ...", __FUNCTION__, drvId);

            E_DRIVE_INDEX_ID user_drvId = NULL_DRIVE;

            if (SYS_DRV_ID == part2_drv_index) /* SYS_DRV_ID is the 2nd partition of the device */
                user_drvId = (part2_drv_index - 1);
            else if (SYS_DRV_ID == part2_drv_index - 1) /* SYS_DRV_ID is the 1st partition of the device */
                user_drvId = part2_drv_index;

            /* formatting the partition drives */
            if (ret = Fat32_Format(DriveGet(SYS_DRV_ID), "SysDrv") != PASS)
                if (ret = Fat16_Format(DriveGet(SYS_DRV_ID), "SysDrv") != PASS)
                    if (ret = Fat12_Format(DriveGet(SYS_DRV_ID), "SysDrv") != PASS)
                        MP_ALERT("-E- (DrvId = %d) FAT32/FAT16/FAT12 formatting all failed ! => Pls check it ...", SYS_DRV_ID);

            /* formatting the user drive partition */
            if (ret = Fat32_Format(DriveGet(user_drvId), "UsrDrv") != PASS)
                if (ret = Fat16_Format(DriveGet(user_drvId), "UsrDrv") != PASS)
                    if (ret = Fat12_Format(DriveGet(user_drvId), "UsrDrv") != PASS)
                        MP_ALERT("-E- (DrvId = %d) FAT32/FAT16/FAT12 formatting all failed ! => Pls check it ...", user_drvId);

            MP_ALERT("%s: drive(s) formatting done.", __FUNCTION__);

            DriveDelete(user_drvId);

            /* after drive(s) formatting, DriveAdd() again */
            if (DriveAdd(user_drvId) < 2) /* not both partition drives (SysDrv and UsrDrv) are well-added */
            {
                MP_ALERT("%s: -E- Not both SysDrv and UsrDrv drives are well-added !  Pls check it !!\r\n", __FUNCTION__);
                ret = FAIL;
            }
            else /* SysDrv and UsrDrv partition drives are both well-added */
            {
                sDrv = DriveChange(user_drvId);
                DirReset(sDrv);
                DirFirst(sDrv);
                mpDebugPrint("%s:%d", __FUNCTION__, __LINE__);
                xpgEnableDrive(user_drvId);
            }
        }
    }

L_xpgCard_processing_ending:
    if (!SystemCardPresentCheck(g_psSystemConfig->sStorage.dwCurStorageId))
        xpgCb_NextCard();

    DriveChange(g_psSystemConfig->sStorage.dwCurStorageId);

    return ret;
}


//////////////////////////////////////////////////////////////////
//
// For MP615
// IDU      PLL2        BASE ON DISPLAY DEVICE(65.25MHz)
// CPU      PLL1        DEFAULT_CPUCLK
// MEMORY   PLL1
// CDU      PLL1/2
// MCARD    PLL1/3
// USDD     PHY
// USBH     PLL1/2      48MHz
//
// For MP650/660
// IDU      PLLIDU
// CPU      PLL2
// MEMORY   PLL2
// CDU      PLL2/1.5
// MCARD    Auto switch between PLL1 and PLL2
// USDD     PHY
// USBH     PLL1/2
//
//////////////////////////////////////////////////////////////////
static void ClkInit(void)
{
    //////////////////////////////////////////////////////////////////
    //
    // Initial UART Debug Port - for AP code's port's cfg
    //
    //////////////////////////////////////////////////////////////////
    Uart_Init();

    //////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////
#if 0
#if (USE_IPW1_DISPLAY_MOTHOD == ENABLE)
    Dma_PriorityDefault();    
    Dma_FirstPrioritySet(DMA_PRIORITY_MASK_VIDEO_I);
    Dma_SecondaryPrioritySet(DMA_PRIORITY_MASK_MCARD);
    Dma_SecondaryPrioritySet(DMA_PRIORITY_MASK_IDU_MAIN);
    Dma_SecondaryPrioritySet(DMA_PRIORITY_MASK_AIU);
    Dma_SecondaryPrioritySet( DMA_PRIORITY_MASK_IPW); 
    Dma_FdmaFifoTimingAdjust(0);
    Dma_PriorityDefaultSave();   
#else
    Dma_PriorityDefault();
    Dma_FirstPrioritySet(DMA_PRIORITY_MASK_IPW);
    Dma_SecondaryPrioritySet(DMA_PRIORITY_MASK_MCARD);
    Dma_SecondaryPrioritySet(DMA_PRIORITY_MASK_IDU_MAIN);
    Dma_SecondaryPrioritySet(DMA_PRIORITY_MASK_AIU);
    Dma_FdmaFifoTimingAdjust(2);
    Dma_PriorityDefaultSave();
#endif
#endif

	#if 1 //test config
	Dma_PriorityDefault();					
	Dma_FirstPrioritySet(DMA_PRIORITY_MASK_IDU_MAIN ); // DMA_PRIORITY_MASK_IPW  
	Dma_SecondaryPrioritySet(DMA_PRIORITY_MASK_MCARD); 
	Dma_SecondaryPrioritySet(DMA_PRIORITY_MASK_IPW );//     DMA_PRIORITY_MASK_IDU_MAIN
	Dma_SecondaryPrioritySet(DMA_PRIORITY_MASK_AIU); 
	Dma_SecondaryPrioritySet(DMA_PRIORITY_MASK_VIDEO_I); 
	 
	 /*for big panel bandwidth*/ 
	//	  Dma_FdmaFifoTimingAdjust(0);
	 Dma_FdmaFifoTimingAdjust(0);
	 Dma_PriorityDefaultSave();
	
	#endif



    //////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////
    PutUartChar(' ');       // Clean UART FIFO

  // Clock_PllFreqSet(CLOCK_PLL2_INDEX, 120000000);       //default
    //Clock_PllFreqSet(CLOCK_PLL2_INDEX, 132000000);
    //Clock_PllFreqSet(CLOCK_PLL2_INDEX, 138000000);
  Clock_PllFreqSet(CLOCK_PLL2_INDEX, 144000000);
    //Clock_PllFreqSet(CLOCK_PLL2_INDEX, 153000000);
    // Clock Source Select Setting
    //Clock_MemClockSelSet(MEMCKS_PLL2);
    //Clock_CpuClockSelSet(CPUCKS_PLL2);
    //Clock_PllFreqSet(CLOCK_PLL1_INDEX, 96000000);

    //////////////////////////////////////////////////////////////////
    //
    // Initial Uart Debug Port - for AP code's PLL cfg
    //
    //////////////////////////////////////////////////////////////////
    Uart_Init();

#if 0
    CLOCK *clock = (CLOCK *) CLOCK_BASE; // for fix sometimes hang 20141021
    clock->ClkCtrl &= ~(BIT12|BIT13|BIT14);
    clock->ClkCtrl |= (BIT14|BIT13|BIT12); // MCLK Delay Register 設定 = “7” = 1ns delay.
#endif
    
    MP_ALERT("PLL1 Initialize to %dMHz", Clock_PllFreqGet(CLOCK_PLL1_INDEX) / 1000000);
    MP_ALERT("PLL2 Initialize to %dMHz", Clock_PllFreqGet(CLOCK_PLL2_INDEX) / 1000000);
    MP_ALERT("CPU Initialize to %dMHz", Clock_CpuFreqGet() / 1000000);
    MP_ALERT("Memory Initialize to %dMHz", Clock_MemFreqGet() / 1000000);

    //////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////
    // 100MHz
    if (Clock_PllFreqGet(CLOCK_PLL2_INDEX) > 154000000)
        mSetCduCks(CDUCKS_PLL2_DIV_2);
    else
        mSetCduCks(CDUCKS_PLL2_DIV_1_5);

    // 120MHz
    if (Clock_PllFreqGet(CLOCK_PLL2_INDEX) > 180000000)
        mSetScaCks(SCACKS_PLL2_DIV_2);
    else
        mSetScaCks(SCACKS_PLL2_DIV_1_5);

#if (SC_USBHOST || SC_USBDEVICE)
    Api_UsbInit(USBOTG0);
#if (CHIP_VER_MSB == CHIP_VER_650)
    Api_UsbInit(USBOTG1); // MP661 has only one OTG which is OTG0.
#endif
#endif
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
#define EVENT_MASK 0x7fffffff

// if dwNextEvent is not zero, then set it as next event
static SDWORD MainWaitEvent(DWORD *pdwEvent, DWORD dwNextEvent)
{
    //MP_TRACE_LINE();
    SDWORD ret;

    if (dwNextEvent)
    {
        *pdwEvent = dwNextEvent;
        ret = OS_STATUS_OK;
    }
    else
    {
        ret = EventWait(UI_EVENT, EVENT_MASK, OS_EVENT_OR, pdwEvent);

        EventClear(UI_EVENT, ~(*pdwEvent));

        if (*pdwEvent == EVENT_MASK)
        {
            MP_ALERT("*pdwEvent == EVENT_MASK, Do SystemInit");
            *pdwEvent = 0;
            SystemInit();
        }
    }

    return ret;
}




