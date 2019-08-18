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
* Filename      : mcard.c
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

#include "devio.h"
#include "McardApi.h"
#include "mcard.h"
#include "uti.h"
#include "os.h"
#include "taskid.h"
#include "peripheral.h"

/*
// macro define
*/
#define WRITE_VERIFY	0
#define PERFORMANCE_METER	0

/*
// local type define
*/
typedef void (*McardRegFunc) (ST_MCARD_DEV *);
typedef struct
{
	McardRegFunc func;
	E_DEVICE_ID index;
} McardDeviceReg;

typedef struct
{
	ST_MCARD_MAIL data;
	DWORD mail_id;
	BOOL used;
} McardMailBuffer;

/*
// Constant declarations
*/
#define MAX_MCARD_MAIL_NR	16

/*
// Variable declarations
*/
ST_MCARD_DEVS *psMcardDev;
static ST_MCARD_DEVS McardDev;
McardMailBuffer McardMailBuf[MAX_MCARD_MAIL_NR];
static void (*UI_CDNotifier)(DWORD, DWORD) = NULL;
static void (*UI_FatalErrNotifier)(DRIVE_PHY_DEV_ID) = NULL;
static volatile DWORD *McardIntReg[6];
const static McardDeviceReg McDevReg[] =
{
	#if SD_MMC_ENABLE
	{SdInit, DEV_SD_MMC},
	#endif
	#if SD2_ENABLE
	{Sd2Init, DEV_SD2},
	#endif
	#if MS_ENABLE
	{MsInit, DEV_MS},
	#endif
	#if CF_ENABLE
	{CfInit, DEV_CF},
	#endif
	#if (NAND_ENABLE && (NAND_DRV == NAND_SIMPLE_DRV))
	{NandInit, DEV_NAND},
	#endif
	#if (NAND_ENABLE && (NAND_DRV == NAND_FTL_DRV))
	{NandInit, DEV_NAND},
	#endif
	#if (NAND_ENABLE && ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
	{NandIspInit, DEV_NAND_ISP},
	#endif
	#if XD_ENABLE
	{xDInit, DEV_XD},
	#endif
	#if HD_ENABLE
	{AtaInit, DEV_HD},
	#endif
	#if SPI_STORAGE_ENABLE
	{spi_dev_init, DEV_SPI_FLASH},
	#endif
	#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_SPI))
	{spi_isp_dev_init, DEV_SPI_FLASH_ISP},
	#endif
	#if USB_WIFI_ENABLE
	{UsbWifiInit, DEV_USB_WIFI_DEVICE},
	#endif
	#if DM9KS_ETHERNET_ENABLE
	{CFEthernetInit, DEV_CF_ETHERNET_DEVICE},
	#endif
	#if DM9621_ETHERNET_ENABLE
	{UsbEthernetInit, DEV_USB_ETHERNET_DEVICE},
	#endif
	#if USBOTG_WEB_CAM
	{UsbWebCam_Init, DEV_USB_WEBCAM},
	#endif

	// example: add new driver
	//#if SAMPLE_ENABLE
	//{SampleInit, SAMPLE},
	//#endif
	{NULL, DEV_NULL}
};


/*
// Static function prototype
*/
static void DeviceEnable(E_DEVICE_ID bMcardID);
static void DeviceTask(void);
#if (ISP_FUNC_ENABLE)
static SWORD IspCommand(E_DEVICE_ID bNandID, WORD cmd, DWORD arg1, DWORD arg2, DWORD size);
#endif
static SWORD Mcard_Command(BYTE BoxId, E_DEVICE_ID wMcardID, MCARD_DEVCE_CMD_SET Cmd
						, DWORD lba, DWORD blknr, DWORD buf, DWORD objHandle);
static BYTE AllocMcardBuf();
static void FreeMcardBuf(BYTE BufId);
static void SetMcardBufMID(ST_MCARD_MAIL *ptr, DWORD mail_id);
static ST_MCARD_MAIL *GetMcardBuf(BYTE BufId);
static DWORD GetMcardBufMailId(BYTE BufId);
static void SDCardDetectionEnable();
static void CardDetectionEnable();

/*
// Definition of external functions
*/
///

ST_MCARD_DEVS* GetMcardDevTag4Usb (void)
{
    return psMcardDev;
}

void Mcard_Init(BYTE cardCtl)
{
	WORD i;

	psMcardDev = &McardDev;
	McardDev.bIdOffsetValue = 0;

	// create mcard event
	EventCreate(MCARD_EVENT_ID, OS_ATTR_EVENT_CLEAR|OS_ATTR_WAIT_MULTIPLE, 0);

	// clean installed device driver number
	McardDev.dwDevInsCnt = 0x00;

	// clean all mcard device informations
	memset(McardDev.sMDevice, 0x00, sizeof(ST_MCARD_DEV) * MAX_DRIVE_NUM);
	memset(McardMailBuf, 0x00, sizeof(McardMailBuffer) * MAX_MCARD_MAIL_NR);

	// reset lun to card mapping
	memset(McardDev.dwDevLunCurSetting, DEV_NULL, MAX_LUN+1);

	// initial : install drivers
	for (i = 0 ; i < sizeof(McDevReg)/sizeof(McardDeviceReg) ; i++)
	{
		if (McDevReg[i].func)
		{
			(McDevReg[i].func)(&McardDev.sMDevice[McDevReg[i].index]);
			McardDev.dwDevInsCnt++;
		}
	}

	McardHwConfig(cardCtl);

	MCARD *mcard = (MCARD *)MCARD_BASE;
	McardIntReg[0] = &mcard->McCfIc;
	McardIntReg[1] = &mcard->McMsIc;
	McardIntReg[2] = &mcard->McSmIc;
	McardIntReg[3] = &mcard->McSmEccIc;
	McardIntReg[4] = &mcard->McSdIc;
#if (CHIP_VER_MSB == CHIP_VER_650)
	McardIntReg[5] = &(((MCARD *)MCSDIO_BASE)->McSdIc);
#else
	McardIntReg[5] = NULL;
#endif

	//create mcard objects, mailbox and semaphore
	MailboxCreate(MCARD_MAIL_ID, OS_ATTR_PRIORITY);
	SemaphoreCreate(MCARD_SEMAPHORE_ID, OS_ATTR_FIFO, 1);
	// create and start McardeTask
	TaskCreate(MCARD_TASK, DeviceTask, DRIVER_PRIORITY, 4096+2048);
	TaskStartup(MCARD_TASK);


	// Check cardCtl value
	if (cardCtl & DIS_CARD_DETECTION){    // Add default installed cards here!
		mpDebugPrint("[W] Card detection disable, try to initialize fixed storages...");
		McardDev.sMDevice[DEV_SD_MMC].Flag.Detected = 1;
		Mcard_DeviceInit(DEV_SD_MMC);
		mpDebugPrint("....init ok!");
	}
	else{
		MP_DEBUG("[W] card detection enable!!");
#if MCARD_POWER_CTRL
		SDCardDetectionEnable();
#else
		SDCardDetectionEnable();
		CardDetectionEnable();
#endif
	}
}

void McardIsr()
{
	MCARD *sMcard = (MCARD *)MCARD_BASE;
#if (CHIP_VER_MSB == CHIP_VER_650)
	MCARD *sMcsdio = (MCARD *)MCSDIO_BASE;
#else
	MCARD *sMcsdio = (MCARD *)MCARD_BASE;
#endif
	//mpDebugPrint("mISR");
	if ((sMcard->McSdIc & 0x00010000)
		|| (sMcsdio->McSdIc & 0x00010000))
	{
		// ToDo : To use EventSet for interrupt is recommended.
	}
	else if (sMcard->McCfIc & 0x00000004)
	{

		EventSet(MCARD_EVENT_ID, 0x04);
		sMcard->McCfIc &= ~0x00040404;
	}

	else
	{
		DWORD i;
		//PutUartChar('I');
		for (i = 0 ; i < 6 ; i++)
		{
			if (McardIntReg[i])
			{
				DWORD state = *McardIntReg[i];

				state = (state >> 8) & (state & 0xff);
//				UartOutValue(state, 8);
				if (state)
				{
					*McardIntReg[i] &= ~((state << 8) | state);
					EventSet(MCARD_EVENT_ID, state);
					break;
				}
			}
		}
	}
}

void DmaMcardIsr()
{
	((CHANNEL *)DMA_MC_BASE)->Control = 0;
	DmaIntDis(IM_MCRDDM);

	EventSet(MCARD_EVENT_ID, 1);
}

#if (((CHIP_VER_MSB != CHIP_VER_650) && (CHIP_VER_MSB != CHIP_VER_660)) && CF_ENABLE )
void Ui_CfDetect(void)
{
	static DWORD dwStableCount = 0;
	static BYTE g_bCfCardFlag = 0;
	WORD cdStatus;

	Gpio_Config2GpioFunc(GPIO_FGPIO_29, GPIO_INPUT_MODE, 0, 2);

	if (Gpio_DataGet(GPIO_FGPIO_29, &cdStatus, 2) != NO_ERR)
	{
		return;
	}

	if (g_bCfCardFlag == 0)
	{
		if (cdStatus == 0)
		{
			dwStableCount++;
			if (dwStableCount >= 5)
			{
				g_bCfCardFlag = 1;
				McardDev.sMDevice[DEV_CF].Flag.Detected = 1;
				if (UI_CDNotifier)
					UI_CDNotifier(TRUE, DEV_CF);
			}
		}
		else
		{
			dwStableCount = 0;
		}
	}
	else
	{
		if (cdStatus == 0x03)
		{
			g_bCfCardFlag = 0;
			dwStableCount = 0;
			McardDev.sMDevice[DEV_CF].Flag.Detected = 0;
			if (UI_CDNotifier)
				UI_CDNotifier(FALSE, DEV_CF);
		}
	}
}
#endif

ST_MCARD_DEV *Mcard_GetDevice(E_DEVICE_ID bMcardID)
{
	return &McardDev.sMDevice[bMcardID];
}


BYTE *Mcard_GetDescriptor(E_DEVICE_ID bMcardID)
{
	return McardDev.sMDevice[bMcardID].pbDescriptor;
}

BYTE Mcard_GetDeviceNum()
{
	return McardDev.dwDevInsCnt;
}

DWORD Mcard_GetCapacity(E_DEVICE_ID bMcardID)
{
	MP_DEBUG("Card%d capacity = %d", bMcardID, McardDev.sMDevice[bMcardID].dwCapacity);
	return McardDev.sMDevice[bMcardID].dwCapacity;
}

E_DEVICE_ID Mcard_CurLunGetCardID(BYTE bLunNum)
{
	return McardDev.dwDevLunCurSetting[bLunNum];
}

BYTE Mcard_CurCardIDGetLun(E_DEVICE_ID bMcardID)
{
    BYTE bLunIdx;

    for(bLunIdx = 0; bLunIdx < (MAX_LUN + 1); bLunIdx++)
        if(McardDev.dwDevLunCurSetting[bLunIdx] == bMcardID)
            return bLunIdx;

	return FAIL;
}

WORD Mcard_GetRenewCounter(E_DEVICE_ID bMcardID)
{
	return McardDev.sMDevice[bMcardID].wRenewCounter;
}

WORD Mcard_GetSectorSize(E_DEVICE_ID bMcardID)
{
	MP_DEBUG("Card%d sector size = %d", bMcardID, McardDev.sMDevice[bMcardID].wSectorSize);
	return McardDev.sMDevice[bMcardID].wSectorSize;
}

BYTE Mcard_GetFlagPresent(E_DEVICE_ID bMcardID)
{
	return McardDev.sMDevice[bMcardID].Flag.Present;
}

BYTE Mcard_GetAllFlagPresent(void)
{
#if SD_MMC_ENABLE
	if( McardDev.sMDevice[DEV_SD_MMC].Flag.Present)
		return TRUE;
#endif
#if MS_ENABLE
	if( McardDev.sMDevice[DEV_MS].Flag.Present)		
		return TRUE;
#endif
#if XD_ENABLE
	if(McardDev.sMDevice[DEV_XD].Flag.Present)
		return TRUE;
#endif
#if CF_ENABLE
	if(McardDev.sMDevice[DEV_CF].Flag.Present)
		return TRUE;
#endif
#if SD2_ENABLE
	if(McardDev.sMDevice[DEV_SD2].Flag.Present)
		return TRUE;
#endif

	return FALSE;
}

BYTE Mcard_GetFlagReadOnly(E_DEVICE_ID bMcardID)
{
	return McardDev.sMDevice[bMcardID].Flag.ReadOnly;
}

BYTE Mcard_GetDetected(E_DEVICE_ID bMcardID)
{
	return McardDev.sMDevice[bMcardID].Flag.Detected;
}

WORD Mcard_GetCardSubtype(E_DEVICE_ID bMcardID)
{
	// Using data member "wProperty" to record card's subtype information
	return McardDev.sMDevice[bMcardID].wProperty1;
}

#if (SC_USBHOST)
void DeviceEnableByMcardId(E_DEVICE_ID bMcardID)
{
    if ((bMcardID >= DEV_USB_HOST_ID1) || (bMcardID <= DEV_USBOTG1_HOST_PTP))
    {
	    UsbhStorageInit(&McardDev.sMDevice[bMcardID], bMcardID);
	    McardDev.dwDevInsCnt++;
	    MP_DEBUG2("Enable Device %d (Total Count %d)", bMcardID, McardDev.dwDevInsCnt);
	}
}
#endif // (SC_USBHOST)

void Mcard_DeviceLunSet(E_DEVICE_ID bMcardID, BYTE lun)
{
	McardDev.dwDevLunCurSetting[lun] = bMcardID;
}

void Mcard_DeviceClkConfig()
{
	GeneratePLLClockTab(Clock_PllFreqGet(0), Clock_PllFreqGet(1));
}

SWORD Mcard_DeviceInit(E_DEVICE_ID bMcardID)
{
	MP_DEBUG("%s: card%d", __FUNCTION__, bMcardID);
	return Mcard_Command(MCARD_MAIL_ID, bMcardID, INIT_CARD_CMD, 0, 0, 0, 0);
}

SWORD Mcard_DeviceRemove(E_DEVICE_ID bMcardID)
{
	MP_DEBUG("%s: card%d", __FUNCTION__, bMcardID);
	return Mcard_Command(MCARD_MAIL_ID, bMcardID, REMOVE_CARD_CMD, 0, 0, 0, 0);
}

#if NAND_DUMPAP
SWORD Mcard_DeviceRawPageRead(E_DEVICE_ID bMcardID, BYTE * buf)
{
	BYTE BufId;

	BufId = Mcard_DeviceAsyncRawPageRead(bMcardID, buf);

	return Mcard_DeviceAsyncWait(BufId);
}

SWORD Mcard_DeviceRawPageWrite(E_DEVICE_ID bMcardID, BYTE * buf)
{
	BYTE BufId;

	BufId = Mcard_DeviceAsyncRawPageWrite(bMcardID, buf);

	return Mcard_DeviceAsyncWait(BufId);
}
#endif

SWORD Mcard_DeviceRawFormat(E_DEVICE_ID bMcardID, BYTE deepVerify)
{
	BYTE BufId;

	BufId = Mcard_DeviceAsyncRawFormat(bMcardID, deepVerify);

	return Mcard_DeviceAsyncWait(BufId);
}

SWORD Mcard_DeviceRead(DRIVE *drv, BYTE *buffer, DWORD lba, DWORD len)
{
	BYTE BufId;
	SWORD ret;

	//mpDebugPrint("Drv:%x, lba: %x, len: %d", drv, lba, len);
	BufId = Mcard_DeviceAsyncRead(drv, buffer, lba, len);
	ret = Mcard_DeviceAsyncWait(BufId);

	return ret;
}

#if WRITE_VERIFY
static SWORD _Mcard_DeviceWrite(DRIVE *drv, BYTE *buffer, DWORD lba, DWORD len)
{
	BYTE BufId;

	BufId = Mcard_DeviceAsyncWrite(drv, buffer, lba, len);

	return Mcard_DeviceAsyncWait(BufId);
}

SWORD Mcard_DeviceWrite(DRIVE *drv, BYTE *buffer, DWORD lba, DWORD len)
{
	SWORD ret = _Mcard_DeviceWrite(drv, buffer, lba, len);
	BYTE *tmp = (BYTE *)((DWORD)ext_mem_malloc(len<<MCARD_SECTOR_SIZE_EXP)|0xA0000000);
	#if 0	// update FTL tables
	Mcard_DeviceRemove(drv->DevID);
	Mcard_DeviceInit(drv->DevID);
	#endif
	Mcard_DeviceRead(drv, tmp, lba, len);
	if (memcmp(buffer, tmp, len<<MCARD_SECTOR_SIZE_EXP))
	{
		DWORD i;
		for (i = 0 ; i < (len<<MCARD_SECTOR_SIZE_EXP) ; i++)
			if (buffer[i] != tmp[i])
			{
				mpDebugPrint("%d: lba %d, (%x,%x):", i, lba+(i>>MCARD_SECTOR_SIZE_EXP), buffer[i], tmp[i]);
				MemDump(&tmp[i&(~0x1ff)], 512);
				MemDump(&buffer[i&(~0x1ff)], 512);
				while(1);
			}
	}
	ext_mem_free(tmp);

	return ret;
}
#else
SWORD Mcard_DeviceWrite(DRIVE *drv, BYTE *buffer, DWORD lba, DWORD len)
{
	BYTE BufId;

	BufId = Mcard_DeviceAsyncWrite(drv, buffer, lba, len);

	return Mcard_DeviceAsyncWait(BufId);
}
#endif

DWORD Mcard_DeviceAsyncProgress(BYTE BufId)
{
	ST_MCARD_MAIL *mail = GetMcardBuf(BufId);
	DWORD ret = -1;

	if (mail)
		ret = mail->dwProgress;

	return ret;
}

SWORD Mcard_DeviceAsyncWait(BYTE BufId)
{
	ST_MCARD_MAIL *mail;
	SWORD ret;

	mail = GetMcardBuf(BufId);
	
	MailTrack((BYTE)GetMcardBufMailId(BufId));
	
	if ((mail->wCmd == SECTOR_READ_CMD) && ((mail->dwBuffer & BIT29) == 0))
		SetDataCacheInvalid(); // Invalidate D-cache.
#if SC_USBHOST
	if (USBOTG_NONE != GetWhichUsbOtgByCardId(mail->wMCardId))//(mail->wMCardId <= DEV_USB_HOST_PTP)	// for USB, temporary now
		ret = McardDev.sMDevice[mail->wMCardId].swStatus;
	else
#endif
		ret = mail->swStatus;
	FreeMcardBuf(BufId);

	return ret;
}

#if NAND_DUMPAP
BYTE Mcard_DeviceAsyncRawPageRead(E_DEVICE_ID bMcardID, BYTE *buffer)
{
	BYTE BufId;
	ST_MCARD_MAIL *McMail;
	BYTE mail_id = FAIL;

	BufId = AllocMcardBuf();
	McMail = GetMcardBuf(BufId);
	
	MP_DEBUG("%s: card%d", __FUNCTION__, bMcardID);

	if ((((DWORD)buffer) & BIT29) == 0)
		SetDataCacheInvalid(); // Invalidate D-cache.

	McMail->wMCardId = bMcardID;
	McMail->wCmd = RAWPAGE_READ_CMD;
	McMail->dwBuffer = (DWORD)buffer;

	if (MailboxSend(MCARD_MAIL_ID, (BYTE *)McMail, sizeof(ST_MCARD_MAIL), &mail_id) == OS_STATUS_OK)
		SetMcardBufMID(McMail, mail_id);
	else
		FreeMcardBuf(BufId);

	return BufId;
}

BYTE Mcard_DeviceAsyncRawPageWrite(E_DEVICE_ID bMcardID, BYTE *buffer)
{
	BYTE BufId;
	ST_MCARD_MAIL *McMail;
	BYTE mail_id = FAIL;

	BufId = AllocMcardBuf();
	McMail = GetMcardBuf(BufId);
	
	MP_DEBUG("%s: card%d", __FUNCTION__, bMcardID);

	if ((((DWORD)buffer) & BIT29) == 0)
		SetDataCacheInvalid(); // Invalidate D-cache.

	McMail->wMCardId = bMcardID;
	McMail->wCmd = RAWPAGE_WRITE_CMD;
	McMail->dwBuffer = (DWORD)buffer;

	if (MailboxSend(MCARD_MAIL_ID, (BYTE *)McMail, sizeof(ST_MCARD_MAIL), &mail_id) == OS_STATUS_OK)
		SetMcardBufMID(McMail, mail_id);
	else
		FreeMcardBuf(BufId);

	return BufId;
}
#endif

BYTE Mcard_DeviceAsyncRawFormat(E_DEVICE_ID bMcardID, BYTE deepVerify)
{
	BYTE BufId;
	ST_MCARD_MAIL *McMail;
	BYTE mail_id = FAIL;

	BufId = AllocMcardBuf();
	McMail = GetMcardBuf(BufId);
	
	MP_DEBUG("%s: card%d", __FUNCTION__, bMcardID);
	McMail->wMCardId = bMcardID;
	McMail->wCmd = RAW_FORMAT_CMD;
	McMail->dwBlockAddr = deepVerify;
	if (MailboxSend(MCARD_MAIL_ID, (BYTE *)McMail, sizeof(ST_MCARD_MAIL), &mail_id) == OS_STATUS_OK)
		SetMcardBufMID(McMail, mail_id);
	else
		FreeMcardBuf(BufId);

	return BufId;
}

BYTE Mcard_DeviceAsyncRead(DRIVE *drv, BYTE *buffer, DWORD lba, DWORD len)
{
	BYTE BufId;
	ST_MCARD_MAIL *McMail;	
	BYTE MailBoxId;
	BYTE mail_id = FAIL;

	BufId = AllocMcardBuf();
	McMail = GetMcardBuf(BufId);

	MP_DEBUG("%s: card %d, lba %d, len %d", __FUNCTION__, drv->DevID, lba, len);

	 if ((((DWORD)buffer) & BIT29) == 0)
	     SetDataCacheInvalid(); // Invalidate D-cache.

	McMail->wMCardId = drv->DevID;
	McMail->wCmd = SECTOR_READ_CMD;
	McMail->dwBuffer = (DWORD)buffer;

#if (SC_USBHOST)
	if (USBOTG_NONE != GetWhichUsbOtgByCardId(drv->DevID))//(drv->DevID <= DEV_USB_HOST_PTP)
	{
		MailBoxId = GetMailBoxIdbyCardId(drv->DevID);//USBOTG0_HOST_CLASS_MAIL_ID;
		McMail->dwObjectHandle = (DWORD)drv->Node;
	}
	else
#endif
	{
		MailBoxId = MCARD_MAIL_ID;
		// change file system's sector size to physical sector size
		register DWORD sector_size = McardDev.sMDevice[drv->DevID].wSectorSizeExp;
		if (drv->bSectorExp < sector_size)
		{
			mpDebugPrint("Sector size mismatch!");
		}
		else
		{
			lba >>= (drv->bSectorExp - sector_size);
			len <<= (drv->bSectorExp - sector_size);
		}
	}
	McMail->dwBlockAddr = lba;
	McMail->dwBlockCount = len;
	if (MailboxSend(MailBoxId, (BYTE *)McMail, sizeof(ST_MCARD_MAIL), &mail_id) == OS_STATUS_OK)
		SetMcardBufMID(McMail, mail_id);
	else
		FreeMcardBuf(BufId);

	return BufId;
}

BYTE Mcard_DeviceAsyncWrite(DRIVE *drv, BYTE *buffer, DWORD lba, DWORD len)
{
	BYTE BufId;
	ST_MCARD_MAIL *McMail;	
	BYTE MailBoxId;
	BYTE mail_id = FAIL;

	BufId = AllocMcardBuf();
	McMail = GetMcardBuf(BufId);

	MP_DEBUG("%s: card%d, lba%d,%d", __FUNCTION__, drv->DevID, lba, len);

	McMail->wMCardId = drv->DevID;
	McMail->wCmd = SECTOR_WRITE_CMD;
	McMail->dwBuffer = (DWORD)buffer;

#if (SC_USBHOST)
	if (USBOTG_NONE != GetWhichUsbOtgByCardId(drv->DevID))//(drv->DevID <= DEV_USB_HOST_PTP)
	{
		MailBoxId = GetMailBoxIdbyCardId(drv->DevID);//USBOTG_HOST_CLASS_MAIL_ID;
		McMail->dwObjectHandle = (DWORD)drv->Node;
	}
	else
#endif
	{
		MailBoxId = MCARD_MAIL_ID;
		// change file system's sector size to physical sector size
		register DWORD sector_size = McardDev.sMDevice[drv->DevID].wSectorSizeExp;
		if (drv->bSectorExp < sector_size)
		{
			mpDebugPrint("Sector size mismatch!");
		}
		else
		{
			lba >>= (drv->bSectorExp - sector_size);
			len <<= (drv->bSectorExp - sector_size);
		}
	}
	McMail->dwBlockAddr = lba;
	McMail->dwBlockCount = len;
	if (MailboxSend(MailBoxId, (BYTE *)McMail, sizeof(ST_MCARD_MAIL), &mail_id) == OS_STATUS_OK)
		SetMcardBufMID(McMail, mail_id);
	else
		FreeMcardBuf(BufId);

	return BufId;
}


#if (ISP_FUNC_ENABLE)
SWORD IspRead(E_DEVICE_ID bDrvID, DWORD lba, BYTE *buffer, DWORD size)
{
	return IspCommand(bDrvID, ISP_READ_CMD, lba, (DWORD)buffer, size);
}

SWORD IspWrite(E_DEVICE_ID bDrvID, DWORD lba, BYTE *buffer, DWORD size)
{
	return IspCommand(bDrvID, ISP_WRITE_CMD, lba, (DWORD)buffer, size);
}
#endif

#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
SWORD IspBlockIsBad(E_DEVICE_ID bNandID, DWORD lba)
{
	DWORD status;
	IspCommand(bNandID, CODE_BLK_INVALID_CMD, lba, (DWORD)&status, 0);

	return status;
}

void IspGetInfo(E_DEVICE_ID bNandID, DWORD *SectorPerBlk, DWORD *blknr, DWORD *pageSize)
{
	DWORD buffer[3];
	IspCommand(bNandID, CODE_IDENTIFY_CMD, 0, (DWORD)buffer, 0);
	if (SectorPerBlk)
		*SectorPerBlk = buffer[0];
	if (blknr)
		*blknr = buffer[1];
	if(pageSize)
		*pageSize = buffer[2];
}

BYTE IspEraseBlk(E_DEVICE_ID bNandID, DWORD addr, DWORD nr)
{
	DWORD i;
	DWORD ret = FAIL;
	DWORD spb, bn;

	IspGetInfo(bNandID, &spb, &bn, 0);
	addr = (addr / spb) * spb;
	for (i = 0 ; i < nr ; i++)
	{
		IspCommand(bNandID, CODE_BLK_ERASE_CMD, addr, (DWORD)&ret, 0);
		if (ret == FAIL)
			break;
		addr += spb;
		if ((addr / spb) >= bn)
			break;
	}

	return ret;
}

#endif

void RegisterCardDetectCB(void (*func_ptr)(DWORD, DWORD))
{
	UI_CDNotifier = func_ptr;
	MP_DEBUG("ui card detect notifier:%x", UI_CDNotifier);
}

void RegisterCardFatalErrorCB(void (*func_ptr)(DRIVE_PHY_DEV_ID))
{
	UI_FatalErrNotifier = func_ptr;
	MP_DEBUG("ui card fatal error notifier:%x", UI_CDNotifier);
}

// Notify system - mcard subsystem gets a fatal error
void CardFatalErrorNotify(DRIVE_PHY_DEV_ID devID)
{
	if(UI_FatalErrNotifier){
		UI_FatalErrNotifier(devID);
		mpDebugPrint("Send %d exception!!!!!!!!!!!!!!!!!!!!", devID);
	}
	else
		mpDebugPrint("%d fatal error exception is not register!!!!!!!!!!!!!!!!!!!!", devID);
}

/*
// Definition of local functions
*/
static BYTE AllocMcardBuf()
{
	BYTE i;
	ST_MCARD_MAIL *ret;
	
	ret = NULL;

	SemaphoreWait(MCARD_SEMAPHORE_ID);

	do
	{
		for (i = 0 ; i < MAX_MCARD_MAIL_NR ; i++)
		{
			if (McardMailBuf[i].used == 0)
			{
				McardMailBuf[i].used = 1;
				ret = &McardMailBuf[i].data;

				break;
			}
		}

		if (i == MAX_MCARD_MAIL_NR)
			TaskSleep(1);
	} while (i == MAX_MCARD_MAIL_NR);	// wait until get any available resource

	memset(ret, 0x00, sizeof(ST_MCARD_MAIL));

	SemaphoreRelease(MCARD_SEMAPHORE_ID);

	return i;
}

static void FreeMcardBuf(BYTE BufId)
{
	IntDisable();
	
	if(BufId < MAX_MCARD_MAIL_NR)
	{
		McardMailBuf[BufId].used = 0;
		McardMailBuf[BufId].mail_id = 0xff;
	}
	
	IntEnable();	
}

static void SetMcardBufMID(ST_MCARD_MAIL *ptr, DWORD mail_id)
{
	DWORD i;

	for (i = 0 ; i < MAX_MCARD_MAIL_NR ; i++)
	{
		if (McardMailBuf[i].used && (&McardMailBuf[i].data == ptr))
		{
			McardMailBuf[i].mail_id = mail_id;
			break;
		}
	}
}

static ST_MCARD_MAIL *GetMcardBuf(BYTE BufId)
{
	if(BufId < MAX_MCARD_MAIL_NR)
		return &McardMailBuf[BufId].data;
	else
		return NULL;
}

static DWORD GetMcardBufMailId(BYTE BufId)
{
	if(BufId < MAX_MCARD_MAIL_NR)
		return McardMailBuf[BufId].mail_id;
	else
		return NULL;
}

#if (ISP_FUNC_ENABLE)
static SWORD IspCommand(E_DEVICE_ID bID, WORD cmd, DWORD arg1, DWORD arg2, DWORD arg3)
{
	return Mcard_Command(MCARD_MAIL_ID, bID, cmd, arg1, arg3, arg2, 0);
}
#endif

static void CardDetector(WORD bitCode)
{
#if (CHIP_VER_MSB == CHIP_VER_650)
	static const E_DEVICE_ID CardDevID[] = {DEV_CF, DEV_XD, DEV_SD_MMC, DEV_MS, DEV_SD2};
	static const BYTE *CardName[] = {"CF", "xD", "SD", "MS", "SD2"};
#elif (CHIP_VER_MSB == CHIP_VER_660)
	static const E_DEVICE_ID CardDevID[] = {DEV_CF, DEV_XD, DEV_SD_MMC, DEV_MS};
	static const BYTE *CardName[] = {"CF", "xD", "SD", "MS"};
#elif (CHIP_VER_MSB == CHIP_VER_615)
	static const E_DEVICE_ID CardDevID[] = {DEV_SM, -1, DEV_SD_MMC, -1, DEV_MS, -1, DEV_XD, -1};
	static const BYTE *CardName[] = {"SM", NULL, "SD", NULL, "MS", NULL, "xD", NULL};
#endif
	static DWORD CardInsOrNot[] = {0, 0, 0, 0, 0, 0, 0, 0};
	static DWORD CardInsTime[] = {0, 0, 0, 0, 0, 0, 0, 0};
    BOOL triggerPolarity;

	MP_DEBUG("detect %d. %d", bitCode, CardDevID[bitCode]);
    //get card detect
	if (CardDevID[bitCode] != -1)
	{
		Gpio_IntConfigGet(GPINT_FGPIO_START + bitCode, &triggerPolarity, NULL);
		if (triggerPolarity == GPIO_ACTIVE_LOW)
		{
			if (CardInsOrNot[bitCode] == 0)
			{
				CardInsOrNot[bitCode] = 1;
				CardInsTime[bitCode] = (GetSysTime()>>2)<<2;
			}
			else
			{
				DWORD tmr = GetSysTime();
				if (TIME_PERIOD(tmr, CardInsTime[bitCode]) > CARD_INSERT_STABLE_TIME)
				{
					mpDebugPrint("\r\nCard In - %s", CardName[bitCode]);
					Gpio_IntConfig(GPINT_FGPIO_START + bitCode, GPIO_ACTIVE_HIGH, GPIO_LEVEL_TRIGGER);
					if (UI_CDNotifier)
						UI_CDNotifier(TRUE, CardDevID[bitCode]);
				}
			}
			McardDev.sMDevice[CardDevID[bitCode]].Flag.Detected = 1;
		}
		else
		{
			mpDebugPrint("\r\nCard Out - %s", CardName[bitCode]);
			CardInsOrNot[bitCode] = 0;
			McardDev.sMDevice[CardDevID[bitCode]].Flag.Detected = 0;
			Gpio_IntConfig(GPINT_FGPIO_START + bitCode, GPIO_ACTIVE_LOW, GPIO_LEVEL_TRIGGER);
			if (UI_CDNotifier)
				UI_CDNotifier(FALSE, CardDevID[bitCode]);
		}
	}
}


static void SDCardDetectionEnable()
{
#if SD_MMC_ENABLE
    Gpio_IntConfig(GPINT_SD_CARD_DETECT, GPIO_ACTIVE_LOW, GPIO_LEVEL_TRIGGER);
    Gpio_IntCallbackFunRegister(GPINT_SD_CARD_DETECT, CardDetector);
    Gpio_IntEnable(GPINT_SD_CARD_DETECT);
#endif

#if (SD2_ENABLE && (CHIP_VER_MSB == CHIP_VER_650))
    Gpio_IntConfig(GPINT_SDIO_CARD_DETECT, GPIO_ACTIVE_LOW, GPIO_LEVEL_TRIGGER);
    Gpio_IntCallbackFunRegister(GPINT_SDIO_CARD_DETECT, CardDetector);
    Gpio_IntEnable(GPINT_SDIO_CARD_DETECT);
#endif
}

static void SDCardDetectionDisable()
{
#if SD_MMC_ENABLE
    Gpio_IntDisable(GPINT_SD_CARD_DETECT);
#endif

#if SD2_ENABLE
    Gpio_IntDisable(GPINT_SDIO_CARD_DETECT);
#endif
}



static void CardDetectionEnable()
{
    // Card Detect
#if SD_MMC_ENABLE
    #if MCARD_POWER_CTRL    //  it's necessary when power ctrl 
    Gpio_IntConfig(GPINT_SD_CARD_DETECT, GPIO_ACTIVE_LOW, GPIO_LEVEL_TRIGGER);
    Gpio_IntEnable(GPINT_SD_CARD_DETECT);
    #endif	
#endif

#if (SD2_ENABLE && (CHIP_VER_MSB == CHIP_VER_650))
    #if MCARD_POWER_CTRL
    Gpio_IntConfig(GPINT_SDIO_CARD_DETECT, GPIO_ACTIVE_LOW, GPIO_LEVEL_TRIGGER);
    Gpio_IntEnable(GPINT_SDIO_CARD_DETECT);
    #endif
#endif

#if MS_ENABLE
    Gpio_IntConfig(GPINT_MS_CARD_DETECT, GPIO_ACTIVE_LOW, GPIO_LEVEL_TRIGGER);
    Gpio_IntCallbackFunRegister(GPINT_MS_CARD_DETECT, CardDetector);
    Gpio_IntEnable(GPINT_MS_CARD_DETECT);
#endif

#if XD_ENABLE
    Gpio_IntConfig(GPINT_XD_CARD_DETECT, GPIO_ACTIVE_LOW, GPIO_LEVEL_TRIGGER);
    Gpio_IntCallbackFunRegister(GPINT_XD_CARD_DETECT, CardDetector);
    Gpio_IntEnable(GPINT_XD_CARD_DETECT);
#endif

#if (CF_ENABLE && ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660)))
    Gpio_IntConfig(GPINT_CF_CARD_DETECT, GPIO_ACTIVE_LOW, GPIO_LEVEL_TRIGGER);
    Gpio_IntCallbackFunRegister(GPINT_CF_CARD_DETECT, CardDetector);
    Gpio_IntEnable(GPINT_CF_CARD_DETECT);
#endif
}

static SWORD Mcard_Command(BYTE BoxId, E_DEVICE_ID wMcardID, MCARD_DEVCE_CMD_SET Cmd
							, DWORD lba, DWORD blknr, DWORD buf, DWORD objHandle)
{
	ST_MCARD_MAIL McMail = {0};	// use stack memory for mcard mail buffer
	SWORD ret;

	McMail.wMCardId = wMcardID;
	McMail.wCmd = (WORD)Cmd;
	McMail.dwBlockAddr = lba;
	McMail.dwBlockCount = blknr;
	McMail.dwBuffer = buf;
	McMail.dwObjectHandle = objHandle;

	BYTE mail_id;
	MailboxSend(BoxId, (BYTE *)&McMail, sizeof(ST_MCARD_MAIL), &mail_id);
	MailTrack(mail_id);

#if SC_USBHOST
	if (USBOTG_NONE != GetWhichUsbOtgByCardId(McMail.wMCardId))//(McMail.wMCardId <= DEV_USB_HOST_PTP)	// for USB,  temporary now
		ret = McardDev.sMDevice[McMail.wMCardId].swStatus;
	else
#endif
		ret = McMail.swStatus;

	return ret;
}

#if PERFORMANCE_METER
static void McardStatistics(ST_MCARD_MAIL *McMail, DWORD tm)
{
	static DWORD r_tmr[MAX_DEVICE_DRV] = {0}, r_sz[MAX_DEVICE_DRV] = {0};
	static DWORD w_tmr[MAX_DEVICE_DRV] = {0}, w_sz[MAX_DEVICE_DRV] = {0};
	static DWORD counter[MAX_DEVICE_DRV] = {0};

	if (McMail->wCmd == SECTOR_READ_CMD)
	{
		tm = (GetSysTime()>>2) - tm;
		if (r_tmr[McMail->wMCardId] == 0)
		{
			r_tmr[McMail->wMCardId] = tm;
			r_sz[McMail->wMCardId] = McMail->dwBlockCount;
		}
		else
		{
			if ((r_tmr[McMail->wMCardId]+tm < r_tmr[McMail->wMCardId])	// overflow handling
				|| (r_sz[McMail->wMCardId]+McMail->dwBlockCount < r_sz[McMail->wMCardId]))
			{
				r_sz[McMail->wMCardId] = r_sz[McMail->wMCardId] / r_tmr[McMail->wMCardId];
				r_tmr[McMail->wMCardId] = 1;
			}
			else
			{
				r_tmr[McMail->wMCardId] += tm;
				r_sz[McMail->wMCardId] += McMail->dwBlockCount;
				counter[McMail->wMCardId]++;
			}
		}

	}
	else if (McMail->wCmd == SECTOR_WRITE_CMD)
	{
		tm = (GetSysTime()>>2) - tm;
		if (w_tmr[McMail->wMCardId] == 0)
		{
			w_tmr[McMail->wMCardId] = tm;
			w_sz[McMail->wMCardId] = McMail->dwBlockCount;
		}
		else
		{
			if ((w_tmr[McMail->wMCardId]+tm < w_tmr[McMail->wMCardId])	// overflow handling
				|| (w_sz[McMail->wMCardId]+McMail->dwBlockCount < w_sz[McMail->wMCardId]))
			{
				w_sz[McMail->wMCardId] = w_sz[McMail->wMCardId] / w_tmr[McMail->wMCardId];
				w_tmr[McMail->wMCardId] = 1;
			}
			else
			{
				w_tmr[McMail->wMCardId] += tm;
				w_sz[McMail->wMCardId] += McMail->dwBlockCount;
				counter[McMail->wMCardId]++;
			}
		}
	}
	if (counter[McMail->wMCardId] && ((counter[McMail->wMCardId] & 0x1ff) == 0))
	{
		DWORD r_perf = 0, w_perf = 0;

		if (r_tmr[McMail->wMCardId])
			r_perf = (r_sz[McMail->wMCardId] * 128 ) / r_tmr[McMail->wMCardId];
		if (w_tmr[McMail->wMCardId])
			w_perf = (w_sz[McMail->wMCardId] * 128) / w_tmr[McMail->wMCardId];
		mpDebugPrint("\r\n*****************************************");
		mpDebugPrint("***Mcard performance report - %7s ***", Mcard_GetDevice(McMail->wMCardId)->pbDescriptor);
		mpDebugPrint("***Read : %5d kbps                  ***", r_perf);
		mpDebugPrint("***Write: %5d kbps                  ***", w_perf);
		mpDebugPrint("*****************************************");
	}
}
#endif

static void DeviceTask(void)
{
	ST_MCARD_MAIL *McMail;
	BYTE bMcardMailId;

	SystemIntEna(IM_MCARD);
#if (CHIP_VER_MSB == CHIP_VER_650)
	SystemIntEna(IM_SDIO_EXT);
#endif
	while (1)
	{
		MailboxReceive(MCARD_MAIL_ID, &bMcardMailId);
		if (MailGetBufferStart(bMcardMailId, (DWORD *)&McMail) == OS_STATUS_OK)
		{
			ST_MCARD_DEV *McDevs = Mcard_GetDevice(McMail->wMCardId);
			if (McDevs->CommandProcess == NULL)
			{
				MP_ALERT("Device Task: no command process for card%d", McMail->wMCardId);
			}
			else
			{
				#if PERFORMANCE_METER
				DWORD tm = GetSysTime()>>2;
				#endif
				McDevs->sMcardRMail = McMail;
				McDevs->CommandProcess(McDevs);
				#if PERFORMANCE_METER
				McardStatistics(McMail, tm);
				#endif
			}
			if (McDevs->swStatus || McMail->swStatus)
				mpDebugPrint("Card%d: command %d processed failure!(%x/%x)", McMail->wMCardId, McMail->wCmd, McDevs->swStatus, McMail->swStatus);
		}
		MailRelease(bMcardMailId);
	}
#if (CHIP_VER_MSB == CHIP_VER_650)
	SystemIntEna(IM_SDIO_EXT);
#endif
	SystemIntDis(IM_MCARD);
}


void McardBootUpPowerOff()
{
#if MCARD_POWER_CTRL
	McardPowerPinConfig(DEV_XD, TRUE, XD_PW_GPIO, 1);
	McardPowerPinConfig(DEV_MS, TRUE, MS_PW_GPIO, 1);
	McardPowerPinConfig(DEV_SD_MMC, TRUE, SDMMC_PW_GPIO, 1);
#if CF_ENABLE	
	McardPowerPinConfig(DEV_CF, TRUE, CF_PW_GPIO, 1);
#endif
#if SD2_ENABLE
	McardPowerPinConfig(DEV_SD2, TRUE, SD2_PW_GPIO, 1);
#endif
	mpDebugPrint("MCard Power Off");
	if(Mcard_GetFlagPresent(DEV_SD_MMC))
	{
		mpDebugPrint("SD card remove");
		Mcard_DeviceRemove(DEV_SD_MMC);
		SDCardDetectionDisable();
	}
	McardPowerOff(DEV_SD_MMC);
	mpDebugPrint("finish");
#endif	
}

void McardCardDetectionEnable(void)
{
#if MCARD_POWER_CTRL
	CardDetectionEnable();
#endif	
}

BYTE Mcard_TypeSDCard(void)
{
	BYTE vSDType;

	vSDType = DetermineSDSlotType();
	
	mpDebugPrint("SD Card Type in the SD Card Slot is %X.", vSDType);
	
	return vSDType;
}

#if  USBOTG_WEB_CAM
void UsbWebCam_Init(ST_MCARD_DEV * sDev)
{
	mpDebugPrint("UsbWebCam_Init");
	sDev->pbDescriptor = NULL;
	sDev->wMcardType = DEV_USB_WEBCAM;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = NULL;

}
#endif




MPX_KMODAPI_SET(Mcard_DeviceInit);
MPX_KMODAPI_SET(Mcard_DeviceRead);
MPX_KMODAPI_SET(Mcard_DeviceWrite);
MPX_KMODAPI_SET(Mcard_DeviceRawFormat);
MPX_KMODAPI_SET(Mcard_DeviceRemove);
MPX_KMODAPI_SET(Mcard_GetFlagPresent);
MPX_KMODAPI_SET(Mcard_DeviceAsyncProgress);
MPX_KMODAPI_SET(Mcard_DeviceAsyncRawFormat);
MPX_KMODAPI_SET(Mcard_DeviceAsyncRead);
MPX_KMODAPI_SET(Mcard_DeviceAsyncWrite);
MPX_KMODAPI_SET(Mcard_DeviceAsyncWait);

