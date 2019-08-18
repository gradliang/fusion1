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
* Filename      : SystemDrive.c
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
#include "ui.h"
#include "taskid.h"
#include "flagdefine.h"
#include "peripheral.h"
#include "bios.h"
#include "Usbotg.h"
#include "fs.h"

///////////////////////////////////////////////////////////////////////////
//
//  Variable declarations
//
///////////////////////////////////////////////////////////////////////////
static ST_EXCEPTION_MSG stMsgContent4CardIn = {0};
static ST_EXCEPTION_MSG stMsgContent4CardOut = {0};
static volatile DWORD cardInCode = 0;
static volatile DWORD cardOutCode = 0;
static volatile DWORD cardFatalErrorCode = 0;

///////////////////////////////////////////////////////////////////////////
//
//  Constant declarations
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  Static function prototype
//
///////////////////////////////////////////////////////////////////////////
static void systemCardIn(DWORD cardCode);
static void systemCardOut(DWORD cardCode);

///////////////////////////////////////////////////////////////////////////
//
//  Definition of external functions
//
///////////////////////////////////////////////////////////////////////////


void SystemCardExceptionRegister(void)
{
    stMsgContent4CardIn.dwTag = ExceptionTagReister(systemCardIn);
    stMsgContent4CardOut.dwTag = ExceptionTagReister(systemCardOut);
}



static void systemCardIn(DWORD cardCode)
{
    SDWORD status;
    BYTE devId, drvId;

    for (devId = 0 ; devId < MAX_DEVICE_DRV ; devId++)
    {
        if (cardCode & BIT0)
        {
            MP_DEBUG("%s- Init Phy. Device - %s", __FUNCTION__, DeviceIndex2DrvName(devId));

            drvId = PhyDevID2DriveIndex(devId);

#if (SC_USBDEVICE)
            if (drvId != SYS_DRV_ID)
            {
                SystemDriveLunNumSet(drvId, SystemDriveLunNumGet(drvId));   // Set DriveId & Lun Mapping.
                SystemDeviceLunInfoUpdate();
            }
#endif

#if (SC_USBHOST)
            if (USBOTG_NONE != GetWhichUsbOtgByCardId(devId))//(bCardCode <= DEV_USB_HOST_PTP)
            {   // for usbh msdc or sidc
                status = Api_UsbhStorageDeviceInit(devId);
            }
            else
#endif
            {   // for mcard
                status = Mcard_DeviceInit(devId);
            }

            MP_DEBUG("Init Phy. Device-%s, status is %d", DeviceIndex2DrvName(devId), status);

#if ((SC_USBDEVICE) && (!USBDEVICE_CDC_DEBUG))
            if (SystemCheckUsbdPlugIn())
            {
                USBCardStsByUI(drvId, TRUE, Api_UsbdGetWhichOtgConnectedPc());
            }
#endif
        }

        cardCode >>= 1;
    }
}



static void systemCardOut(DWORD cardCode)
{
    SDWORD status;
    BYTE devId, drvId;

    for (devId = 0 ; devId < MAX_DEVICE_DRV ; devId++)
    {
        if (cardCode & BIT0)
        {
            MP_DEBUG("%s - Remove Device - %s", __FUNCTION__, DeviceIndex2DrvName(devId));
            drvId = PhyDevID2DriveIndex(devId);

#if (SC_USBHOST)
            if (USBOTG_NONE != GetWhichUsbOtgByCardId(devId))//(bCardCode <= DEV_USB_HOST_PTP)
            {   // for usbh msdc or sidc
                status = Api_UsbhStorageDeviceRemove(devId);
            }
            else
#endif
            {   // for mcard
                status = Mcard_DeviceRemove(devId);
            }

#if ((SC_USBDEVICE) && (!USBDEVICE_CDC_DEBUG))
            if (SystemCheckUsbdPlugIn())
            {
                USBCardStsByUI(drvId, FALSE, Api_UsbdGetWhichOtgConnectedPc()); // Card - Out
            }
#endif

            SetDrvPresentFlag(drvId, 0);
            MP_DEBUG("Remove Device - %d, status is %d", devId, status);
        }

        cardCode >>= 1;
    }
}



void SystemCardEventSet(DWORD cardIn, DRIVE_PHY_DEV_ID phyDevId)
{
    DWORD dwCardBitCode = 1 << phyDevId;

    if (cardIn == TRUE)
    {   // Card In
        if (cardOutCode & dwCardBitCode)
        {   // Clear card-out flag
            cardOutCode &= ~dwCardBitCode;

            if (cardOutCode == 0)
            {
                EventClear(UI_EVENT, ~EVENT_CARD_OUT);
                MP_ALERT("C-CO");
            }
        }

        cardInCode |= dwCardBitCode;
        MP_ALERT("%s - In 0x%08X", __FUNCTION__, cardInCode);
        EventSet(UI_EVENT, EVENT_CARD_IN);
        stMsgContent4CardIn.msgAddr = dwCardBitCode;
        MessageDrop(EXCEPTION_MSG_ID, (BYTE *) &stMsgContent4CardIn, sizeof(ST_EXCEPTION_MSG));
    }
    else
    {   // Card Out
        if (cardInCode & dwCardBitCode)
        {   // Clear card-in flag
            cardInCode &= ~dwCardBitCode;

            if (cardInCode == 0)
            {
                EventClear(UI_EVENT, ~EVENT_CARD_IN);
                MP_ALERT("C-CI");
            }
        }

        cardOutCode |= dwCardBitCode;

        MP_ALERT("%s - Out 0x%08X", __FUNCTION__, cardOutCode);
        EventSet(UI_EVENT, EVENT_CARD_OUT);
        stMsgContent4CardOut.msgAddr = dwCardBitCode;
        MessageDrop(EXCEPTION_MSG_ID, (BYTE *) &stMsgContent4CardOut, sizeof(ST_EXCEPTION_MSG));
    }
}



void SystemCardFatalErrorEventSet(DRIVE_PHY_DEV_ID phyDevId)
{
    DWORD dwCardBitCode = 1 << phyDevId;

    IntDisable();

    MP_ALERT("\r\n\r\n--E-- %s - %d", __FUNCTION__, phyDevId);

    if (cardInCode & dwCardBitCode)
    {   // Clear card-in flag
        cardInCode &= ~dwCardBitCode;

        if (cardInCode == 0)
        {
            EventClear(UI_EVENT, ~EVENT_CARD_IN);
            IntDisable();
            MP_ALERT("C-CI");
        }
    }

    cardFatalErrorCode |= dwCardBitCode;

    MP_ALERT("%s - FatalError 0x%08X", __FUNCTION__, cardFatalErrorCode);
    EventSet(UI_EVENT, EVENT_CARD_FATAL_ERROR);
    IntDisable();
    stMsgContent4CardOut.msgAddr = dwCardBitCode;
    MessageDrop(EXCEPTION_MSG_ID, (BYTE *) &stMsgContent4CardOut, sizeof(ST_EXCEPTION_MSG));
}



BYTE SystemCardEvent2DrvIdGet(DWORD dwEvent)
{
    DWORD *pdwCardCode;
    DWORD cardCode;
    BYTE devId, drvId;
    BYTE i;

    IntDisable();

    switch (dwEvent)
    {
    case EVENT_CARD_IN:
        pdwCardCode = (DWORD *) &cardInCode;
        break;

    case EVENT_CARD_OUT:
        pdwCardCode = (DWORD *) &cardOutCode;
        break;

    case EVENT_CARD_FATAL_ERROR:
        pdwCardCode = (DWORD *) &cardFatalErrorCode;
        break;
    }

    cardCode = *pdwCardCode;

    for (i = 0, devId = 0; i < MAX_DEVICE_DRV ; i++)
    {
        if (cardCode & BIT0)
        {
            *pdwCardCode &= ~(1 << devId);
            break;
        }
        else
        {
            devId++;
            cardCode >>= 1;
        }
    }

    if (*pdwCardCode != 0)
    {
        EventSet(UI_EVENT, dwEvent);
    }
    else
        IntEnable();

    drvId = PhyDevID2DriveIndex(devId);
    MP_DEBUG("Device ID = %d, Drive ID = %d", devId, drvId);

    return drvId;
}



BYTE SystemCardOutCheck(E_DRIVE_INDEX_ID driveIndex)
{
    BYTE i, devID;

    devID = DriveIndex2PhyDevID(driveIndex);

    if (devID == DEV_NULL)
        return 0;

    return ((cardOutCode & (1 << devID)) ? 1 : 0);
}



BOOL SystemCardPlugInCheck(E_DRIVE_INDEX_ID driveIndex)
{
    BYTE devID = (BYTE) DriveIndex2PhyDevID(driveIndex);

    if (devID == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, driveIndex);
        return FALSE;
    }
    else
        return (BOOL) Mcard_GetDetected(devID);
}



BOOL SystemCardPresentCheck(E_DRIVE_INDEX_ID driveIndex)
{
    BYTE devID = (BYTE) DriveIndex2PhyDevID(driveIndex);

    if (devID == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, driveIndex);
        return FALSE;
    }
    else
        return Mcard_GetFlagPresent(devID);
}

WORD SystemCardSubtypeGet(E_DRIVE_INDEX_ID driveIndex)
{
    BYTE devID = (BYTE) DriveIndex2PhyDevID(driveIndex);

    if (devID == DEV_NULL){
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, driveIndex);
        return 0;
    }
    else{
        if(Mcard_GetFlagPresent(devID))
			return Mcard_GetCardSubtype(devID);
	}
}

#if NAND_DUMPAP
SWORD SystemDeviceRawPageRead(E_DRIVE_INDEX_ID driveIndex, BYTE *buf)
{
    BYTE devID = (BYTE) DriveIndex2PhyDevID((E_DRIVE_INDEX_ID)driveIndex);

    if (devID == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, driveIndex);
        return FALSE;
    }

    return Mcard_DeviceRawPageRead(devID, buf);
}

SWORD SystemDeviceRawPageWrite(E_DRIVE_INDEX_ID driveIndex, BYTE *buf)
{
    BYTE devID = (BYTE) DriveIndex2PhyDevID((E_DRIVE_INDEX_ID)driveIndex);

    if (devID == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, driveIndex);
        return FALSE;
    }

    return Mcard_DeviceRawPageWrite(devID, buf);
}
#endif

SWORD SystemDeviceRawFormat(E_DRIVE_INDEX_ID driveIndex, BYTE deepVerify)
{
    BYTE devID = (BYTE) DriveIndex2PhyDevID((E_DRIVE_INDEX_ID)driveIndex);

    if (devID == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, driveIndex);
        return FALSE;
    }

    return Mcard_DeviceRawFormat(devID, deepVerify);
}



SWORD SystemDeviceInit(E_DRIVE_INDEX_ID driveIndex)
{
    SWORD status;
    BYTE devId = (BYTE) DriveIndex2PhyDevID((E_DRIVE_INDEX_ID)driveIndex);

    if (devId == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, driveIndex);
        return FALSE;
    }

#if (SC_USBDEVICE)
    if (driveIndex != SYS_DRV_ID)
    {
        SystemDriveLunNumSet(driveIndex, SystemDriveLunNumGet(driveIndex));     // Set DriveId & Lun Mapping.
        SystemDeviceLunInfoUpdate();
    }
#endif

#if (SC_USBHOST)
    if (USBOTG_NONE != GetWhichUsbOtgByCardId(devId))//(bCardCode <= DEV_USB_HOST_PTP)
    {   // for usbh msdc or sidc
        status = Api_UsbhStorageDeviceInit(devId);
    }
    else
#endif
    {   // for mcard
        status = Mcard_DeviceInit(devId);
    }

    return status;
}



SWORD SystemDeviceRemove(BYTE driveIndex)
{
    BYTE devID = (BYTE) DriveIndex2PhyDevID((E_DRIVE_INDEX_ID)driveIndex);

    if (devID == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, driveIndex);
        return FALSE;
    }

#if (SC_USBHOST)
    if (USBOTG_NONE != GetWhichUsbOtgByCardId(devID))//(bCardCode <= DEV_USB_HOST_PTP)
    {
        // for usbh msdc
        return Api_UsbhStorageDeviceRemove(devID);
    }
    else
#endif // (SC_USBHOST)
    {   // for mcard
        return Mcard_DeviceRemove(devID);
    }
}



BYTE  SystemGetFlagReadOnly(BYTE driveIndex)
{
    BYTE devID = (BYTE) DriveIndex2PhyDevID((E_DRIVE_INDEX_ID)driveIndex);

    if (devID == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, driveIndex);
        return FALSE;
    }

#if SC_USBDEVICE
    if (driveIndex == TOOL_DRV_ID)
    {
        if (SystemDriveIdGetByLun(SystemDriveLunNumGet(SYS_DRV_ID)) == NULL_DRIVE)
            return TRUE;
        else
            return FALSE;
    }
#endif

    return Mcard_GetFlagReadOnly(devID);
}


static DWORD cardCtrlMode = STD_MCARD_SUBSYSTEM_MODE;

void SystemCardProtectCtrlMode(BOOL enable)
{
    if (enable)
        cardCtrlMode &= ~DIS_WRITE_PROTECTION;
    else
        cardCtrlMode |= DIS_WRITE_PROTECTION;
}



void SystemCardDetectCtrlMode(BOOL enable)
{
    if (enable)
        cardCtrlMode &= ~DIS_CARD_DETECTION;
    else
        cardCtrlMode |= DIS_CARD_DETECTION;
}



DWORD SystemCardCtrlModeGet(void)
{
    return cardCtrlMode;
}



#if (defined(CARD_DETECT_FUNC_ENABLE) && (CARD_DETECT_FUNC_ENABLE == 0))
static ST_EXCEPTION_MSG stMsgContent4CardPolling = {0};
static volatile WORD dropCardPollingMsg = FALSE;


static void systemCardStatusPolling(void)
{
    //MP_ALERT("%s -", __FUNCTION__);

    #if SD_MMC_ENABLE
    if (SystemCardPresentCheck(SD_MMC) && (DriveGet(SD_MMC)->Flag.Present == 0))
        SystemCardEventSet(TRUE, DriveIndex2PhyDevID(SD_MMC));
    #endif

    #if SD2_ENABLE
    if (SystemCardPresentCheck(SD2) && (DriveGet(SD2)->Flag.Present == 0))
        SystemCardEventSet(TRUE, DriveIndex2PhyDevID(SD2));
    #endif

    #if CF_ENABLE
    if (SystemCardPresentCheck(CF) && (DriveGet(CF)->Flag.Present == 0))
        SystemCardEventSet(TRUE, DriveIndex2PhyDevID(CF));
    #endif

    #if MS_ENABLE
    if (SystemCardPresentCheck(MS) && (DriveGet(MS)->Flag.Present == 0))
        SystemCardEventSet(TRUE, DriveIndex2PhyDevID(MS));
    #endif

    #if XD_ENABLE
    if (SystemCardPresentCheck(XD) && (DriveGet(XD)->Flag.Present == 0))
        SystemCardEventSet(TRUE, DriveIndex2PhyDevID(XD));
    #endif

    dropCardPollingMsg = FALSE;
}



static void systemCardStatusPollingEvent(void)
{
    SDWORD retVal;

    if (dropCardPollingMsg == FALSE)
    {
        dropCardPollingMsg = TRUE;

        retVal = MessageDrop((BYTE) EXCEPTION_MSG_ID, (BYTE *) &stMsgContent4CardPolling, sizeof(ST_EXCEPTION_MSG));

        if (retVal != OS_STATUS_OK)
        {
            dropCardPollingMsg = FALSE;
            MP_ALERT("--E-- card-polling message drop !!!");
        }
    }
}
#endif



void SystemCardStatusPollingEnable(void)
{
#if (defined(CARD_DETECT_FUNC_ENABLE) && (CARD_DETECT_FUNC_ENABLE == 0))
    dropCardPollingMsg = FALSE;
    stMsgContent4CardPolling.dwTag = ExceptionTagReister(systemCardStatusPolling);

    if (ExceptionTagCheck(stMsgContent4CardPolling.dwTag) )
    {
        MP_ALERT("--E-- Card-Polling Exception Register Failure ...\r\n");
        dropCardPollingMsg = FALSE;
        stMsgContent4CardPolling.dwTag = 0;

        return;
    }

    if (SysTimerProcAdd(500, systemCardStatusPollingEvent, FALSE) < 0)
    {
        MP_ALERT("--E-- Card-Polling Timer Start Failure ...\r\n");
        dropCardPollingMsg = FALSE;
        ExceptionTagRelease(stMsgContent4CardPolling.dwTag);
        stMsgContent4CardPolling.dwTag = 0;

        return;
    }
#endif
}

