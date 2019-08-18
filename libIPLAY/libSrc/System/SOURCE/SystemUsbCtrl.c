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
* Filename      : SystemUsbCtrl.c
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
#include "mpTrace.h"

#include "global612.h"
#include "fs.h"
#include "ui.h"
#include "taskid.h"
#include "flagdefine.h"
#include "peripheral.h"
#include "bios.h"
#include "Usbotg.h"

#if SC_USBDEVICE

///////////////////////////////////////////////////////////////////////////
//
//  Variable declarations
//
///////////////////////////////////////////////////////////////////////////



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



/////////////////////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////////////////////
typedef struct
{
    DWORD partitionStartLba;
    DWORD partitionBlockSize;
    DWORD partitionSectorNum;
    BYTE lunNum;
} ST_LUN_DRIVE_INFO;

static BYTE realMaxLun = 0;
static ST_LUN_DRIVE_INFO driveLunInfoLookupTable[MAX_DRIVE_NUM];    // lookup table for Lun to Drive ID (record all drive's lun information)
static BYTE driveLunNum[MAX_DRIVE_NUM];                             // real drive's lun information (one by one mapping)

BOOL SystemDriveLunNumSet(E_DRIVE_INDEX_ID driveId, BYTE lun)
{
    MP_DEBUG("%s Drive-%s is Lun %d", __FUNCTION__, DriveIndex2DrvName(driveId), lun);

    if (driveId < MAX_DRIVE_NUM)
    {
        DWORD i;

        IntDisable();

        if (lun != NULL_LUN_NUM)
        {   // Clear all lun info for USB HOST
            for (i = 1; i < MAX_DRIVE_NUM; i++)
            {
                if (driveLunNum[i] == lun)
                    driveLunNum[i] = NULL_LUN_NUM;
            }
        }

        driveLunNum[driveId] = lun;
        IntEnable();

        return TRUE;
    }

    return FALSE;
}



void SystemDriveLunInfoInit(void)
{
    DWORD i;

    MP_DEBUG("%s -", __FUNCTION__);

    memset((BYTE *) driveLunNum, NULL_LUN_NUM, sizeof(driveLunNum));
    memset((BYTE *) driveLunInfoLookupTable, NULL_LUN_NUM, sizeof(driveLunInfoLookupTable));

#if TOOL_DRV_SIZE
    #if ((SYSTEM_DRIVE == SYSTEM_DRIVE_ON_NAND) && NAND_ENABLE)
        driveLunInfoLookupTable[TOOL_DRV_ID].lunNum = TOOL_DRIVE_LUN_NUM;
        SystemDriveLunNumSet(TOOL_DRV_ID, TOOL_DRIVE_LUN_NUM);

        driveLunInfoLookupTable[SPI_FLASH].lunNum = SPI_LUN_NUM;
        SystemDriveLunNumSet(SPI_FLASH, SPI_LUN_NUM);

        driveLunInfoLookupTable[SD_MMC].lunNum = SD_MMC_LUN_NUM;
        SystemDriveLunNumSet(SD_MMC, SD_MMC_LUN_NUM);
    #elif ((SYSTEM_DRIVE == SYSTEM_DRIVE_ON_SD) && SD_MMC_ENABLE)
        driveLunInfoLookupTable[TOOL_DRV_ID].lunNum = SD_MMC_LUN_NUM;
        SystemDriveLunNumSet(TOOL_DRV_ID, SD_MMC_LUN_NUM);

        driveLunInfoLookupTable[SPI_FLASH].lunNum = SPI_LUN_NUM;
        SystemDriveLunNumSet(SPI_FLASH, SPI_LUN_NUM);

        driveLunInfoLookupTable[NAND].lunNum = NAND_LUN_NUM;
        SystemDriveLunNumSet(NAND, NAND_LUN_NUM);
    #elif ((SYSTEM_DRIVE == SYSTEM_DRIVE_ON_SPI) && SPI_STORAGE_ENABLE)
        driveLunInfoLookupTable[TOOL_DRV_ID].lunNum = SPI_LUN_NUM;
        SystemDriveLunNumSet(TOOL_DRV_ID, SPI_LUN_NUM);

        driveLunInfoLookupTable[SD_MMC].lunNum = SD_MMC_LUN_NUM;
        SystemDriveLunNumSet(SD_MMC, SD_MMC_LUN_NUM);

        driveLunInfoLookupTable[NAND].lunNum = NAND_LUN_NUM;
        SystemDriveLunNumSet(NAND, NAND_LUN_NUM);
    #else
        driveLunInfoLookupTable[NAND].lunNum = NAND_LUN_NUM;
        SystemDriveLunNumSet(NAND, NAND_LUN_NUM);

        driveLunInfoLookupTable[SPI_FLASH].lunNum = SPI_LUN_NUM;
        SystemDriveLunNumSet(SPI_FLASH, SPI_LUN_NUM);

        driveLunInfoLookupTable[SD_MMC].lunNum = SD_MMC_LUN_NUM;
        SystemDriveLunNumSet(SD_MMC, SD_MMC_LUN_NUM);
    #endif
#else
    #if NAND_ENABLE
        driveLunInfoLookupTable[NAND].lunNum = NAND_LUN_NUM;
        SystemDriveLunNumSet(NAND, NAND_LUN_NUM);
    #endif

    #if SPI_STORAGE_ENABLE
        driveLunInfoLookupTable[SPI_FLASH].lunNum = SPI_LUN_NUM;
        SystemDriveLunNumSet(SPI_FLASH, SPI_LUN_NUM);
    #endif

    #if SD_MMC_ENABLE
        driveLunInfoLookupTable[SD_MMC].lunNum = SD_MMC_LUN_NUM;
        SystemDriveLunNumSet(SD_MMC, SD_MMC_LUN_NUM);
    #endif
#endif

#if SD2_ENABLE
    driveLunInfoLookupTable[SD2].lunNum = SD2_LUN_NUM;
    SystemDriveLunNumSet(SD2, SD2_LUN_NUM);
#endif

#if XD_ENABLE
    driveLunInfoLookupTable[XD].lunNum = XD_LUN_NUM;
    SystemDriveLunNumSet(XD, XD_LUN_NUM);
#endif

#if CF_ENABLE
    driveLunInfoLookupTable[CF].lunNum = CF_LUN_NUM;
    SystemDriveLunNumSet(CF, CF_LUN_NUM);
#endif

#if MS_ENABLE
    driveLunInfoLookupTable[MS].lunNum = MS_LUN_NUM;
    SystemDriveLunNumSet(MS, MS_LUN_NUM);
#endif

#if SM_ENABLE
    driveLunInfoLookupTable[SM].lunNum = SM_LUN_NUM;
    SystemDriveLunNumSet(SM, SM_LUN_NUM);
#endif

#if USBOTG_DEVICE_HOST
    driveLunInfoLookupTable[USB_HOST_ID1].lunNum = USB_LUN_NUM;
    driveLunInfoLookupTable[USB_HOST_ID2].lunNum = USB_LUN_NUM;
    driveLunInfoLookupTable[USB_HOST_ID3].lunNum = USB_LUN_NUM;
    driveLunInfoLookupTable[USB_HOST_ID4].lunNum = USB_LUN_NUM;
    driveLunInfoLookupTable[USB_HOST_PTP].lunNum = USB_LUN_NUM;
    driveLunInfoLookupTable[USBOTG1_HOST_ID1].lunNum = USB_LUN_NUM;
    driveLunInfoLookupTable[USBOTG1_HOST_ID2].lunNum = USB_LUN_NUM;
    driveLunInfoLookupTable[USBOTG1_HOST_ID3].lunNum = USB_LUN_NUM;
    driveLunInfoLookupTable[USBOTG1_HOST_ID4].lunNum = USB_LUN_NUM;
    driveLunInfoLookupTable[USBOTG1_HOST_PTP].lunNum = USB_LUN_NUM;

    SystemDriveLunNumSet(USB_HOST_ID1, USB_LUN_NUM);
#endif

#if (SYSTEM_DRIVE != SYSTEM_DRIVE_NONE)
    if (SYS_DRV_ID != NULL_DRIVE)
    driveLunInfoLookupTable[SYS_DRV_ID].lunNum = SYSTEM_DRIVE_LUN_NUM;
#endif

    realMaxLun = 0;

    for (i = 1; i < MAX_DRIVE_NUM; i++)
    {
        if (driveLunNum[i] != NULL_LUN_NUM)
            realMaxLun++;
    }

    MP_ALERT("realMaxLun = %d", realMaxLun);
}



void SystemDriveLunInfoChange(E_DRIVE_INDEX_ID driveIndex)
{
    DWORD partitionStartLba = 0, partitionSectorNum = 0, partitionBlockSize = 0;

    if (driveIndex >= MAX_DRIVE_NUM)
    {
        MP_ALERT("--E-- %s - Wrong drive ID-%d", __FUNCTION__, driveIndex);
        return;
    }

    if (SYSTEM_DRIVE && (DriveIndex2PhyDevID(SYS_DRV_ID) == DriveIndex2PhyDevID(driveIndex)))
    {
        //note: To make NAND System Drive absolutely invisible and cannot be accessed via USB connection,
        //      we intent to skip front sectors before the start LBA of the partition drive.

        GetDrvPartitionInfoFromMBR(driveIndex, NULL, NULL, &partitionStartLba, &partitionSectorNum, &partitionBlockSize);

        IntDisable();
        MP_DEBUG("%s - Card-%s ", __FUNCTION__, DriveIndex2DrvName(driveIndex));
        driveLunInfoLookupTable[driveIndex].partitionStartLba = partitionStartLba;
        driveLunInfoLookupTable[driveIndex].partitionSectorNum = partitionSectorNum;
        driveLunInfoLookupTable[driveIndex].partitionBlockSize = partitionBlockSize;
        IntEnable();
    }
    else
    {
        //note: To avoid DriveAdd failure issue after Mac OS X disk formatting with the method that will do auto MBR sector creation
        // if it cannon find the real MBR on the device, we need to make whole disk volume visible for access via USB connection.

        IntDisable();
        MP_DEBUG("%s - Card-%s ", __FUNCTION__, DriveIndex2DrvName(driveIndex));
        driveLunInfoLookupTable[driveIndex].partitionStartLba = 0;
        driveLunInfoLookupTable[driveIndex].partitionSectorNum = Mcard_GetCapacity(DriveIndex2PhyDevID(driveIndex));
        driveLunInfoLookupTable[driveIndex].partitionBlockSize = Mcard_GetSectorSize(DriveIndex2PhyDevID(driveIndex));
        IntEnable();
    }
}



void SystemDeviceLunInfoUpdate(void)
{
    BYTE driveIndex, devID, i;

    IntDisable();

    MP_DEBUG("%s -", __FUNCTION__);
    realMaxLun = 0;

    for (i = 0; i < MAX_LUN; i++)
    {
        Mcard_DeviceLunSet(DEV_NULL, i);
    }

    for (driveIndex = 1; driveIndex < MAX_DRIVE_NUM; driveIndex++)
    {
        devID = (BYTE) DriveIndex2PhyDevID(driveIndex);

        if (driveLunNum[driveIndex] != NULL_LUN_NUM)
        {
            MP_DEBUG("%s", DriveIndex2DrvName(driveIndex));
            realMaxLun++;
            Mcard_DeviceLunSet(devID, driveLunNum[driveIndex]);
        }
    }

    MP_DEBUG("realMaxLun = %d", realMaxLun);

    IntEnable();
}



BYTE SystemDriveLunNumGet(E_DRIVE_INDEX_ID driveIndex)
{
    if (driveIndex < MAX_DRIVE_NUM)
        return driveLunInfoLookupTable[driveIndex].lunNum;
    else
        return NULL_LUN_NUM;
}



BYTE SystemDriveIdGetByLun(BYTE lun)
{
    BYTE drvId;

    for (drvId = 1; drvId < MAX_DRIVE_NUM; drvId++)
    {
        if (driveLunNum[drvId] == lun)
        {
            //MP_DEBUG("%s - Lun-%d - Drv Name %s", __FUNCTION__, lun, DriveIndex2DrvName(drvId));

            return drvId;
        }
    }

    //MP_DEBUG("Lun-%d - Null Drive", lun);

    return NULL_DRIVE;
}



BYTE SystemMaxLunGet(void)
{
    //mpDebugPrint("%s : MaxLun %d", __FUNCTION__, realMaxLun);
    return realMaxLun;
}


BYTE SystemMaxLunSet(BYTE bMaxLun)
{
    realMaxLun = bMaxLun;
}

DWORD SystemDriveTotalSectorGetByLun(BYTE lun)
{
    BYTE drvId;

    drvId = SystemDriveIdGetByLun(lun);

    return driveLunInfoLookupTable[drvId].partitionSectorNum;
}



DWORD SystemDriveSectorSizeGetByLun(BYTE lun)
{
    BYTE drvId;

    drvId = SystemDriveIdGetByLun(lun);

    return driveLunInfoLookupTable[drvId].partitionBlockSize;
}



SDWORD SystemDriveReadByLun(BYTE lun, BYTE *bufPtr, DWORD lbaAddr, DWORD sectorSize)
{
    E_DRIVE_INDEX_ID drvId;

    drvId = SystemDriveIdGetByLun(lun);

    MP_DEBUG("%s - Card-%s Orginal LBA = 0x%08X", __FUNCTION__, DriveIndex2DrvName(drvId), lbaAddr);

    if (SystemCardPresentCheck(drvId))
    {
        lbaAddr += driveLunInfoLookupTable[drvId].partitionStartLba;
        //MP_DEBUG("Real Drive LBA = 0x%08X", lbaAddr);

        if (Mcard_DeviceRead(DriveGet(drvId), bufPtr, lbaAddr, sectorSize) != PASS)
        {
            MP_ALERT("--E-- Lun-%d Card-%s read fail", lun, DriveIndex2DrvName(drvId));

            return FAIL;
        }
        else
            return PASS;
    }

    return FAIL;
}



SDWORD SystemDriveWriteByLun(BYTE lun, BYTE *bufPtr, DWORD lbaAddr, DWORD sectorSize)
{
    BYTE drvId;

    drvId = SystemDriveIdGetByLun(lun);

    MP_DEBUG("%s - Card-%s Orginal LBA = 0x%08X", __FUNCTION__,  DriveIndex2DrvName(drvId), lbaAddr);

    if (SystemCardPresentCheck(drvId))
    {
        lbaAddr += driveLunInfoLookupTable[drvId].partitionStartLba;
        MP_DEBUG("Real Drive LBA = 0x%08X", lbaAddr);

        if (Mcard_DeviceWrite(DriveGet(drvId), bufPtr, lbaAddr, sectorSize) != PASS)
            return FAIL;
        else
            return PASS;
    }

    return FAIL;
}



SDWORD SystemSysDriveLunEnable(BOOL enable)
{
#if (SC_USBDEVICE)
    WHICH_OTG otgId = Api_UsbdGetWhichOtgConnectedPc();

    if (SYS_DRV_ID == NULL_DRIVE)
        return FAIL;

    if (otgId != USBOTG_NONE)
        Api_UsbdFinal(otgId);

    if (enable == TRUE)
    {
        MP_ALERT("SystemSysDriveLunEnable - Enable");

        SystemDriveLunNumSet(SYS_DRV_ID, SystemDriveLunNumGet(SYS_DRV_ID));
    }
    else
    {
        MP_ALERT("SystemSysDriveLunEnable - Disable");

        SystemDriveLunNumSet(SYS_DRV_ID, NULL_LUN_NUM);

    }

    SystemToolDriveLunInfoChange(TRUE);
    SystemDeviceLunInfoUpdate();

    if (otgId != USBOTG_NONE)
        Api_UsbdInit(otgId);

    return PASS;
#else
    MP_ALERT("Not support function of %s", __FUNCTION__);
    return FAIL;
#endif
}



SDWORD SystemToolDriveLunInfoChange(BOOL enable)
{
#if (SC_USBDEVICE && TOOL_DRV_SIZE)
    if (TOOL_DRV_ID == NULL_DRIVE)
        return FAIL;

    if (enable == ENABLE)
    {
        MP_ALERT("%s - Enable", __FUNCTION__);

        driveLunInfoLookupTable[TOOL_DRV_ID].lunNum = TOOL_DRIVE_LUN_NUM;
        SystemDriveLunNumSet(TOOL_DRV_ID, TOOL_DRIVE_LUN_NUM);

#if ((SYSTEM_DRIVE == SYSTEM_DRIVE_ON_NAND) && NAND_ENABLE)
        driveLunInfoLookupTable[NAND].lunNum = NULL_LUN_NUM;
        SystemDriveLunNumSet(NAND, NULL_LUN_NUM);
#elif ((SYSTEM_DRIVE == SYSTEM_DRIVE_ON_SD) && SD_MMC_ENABLE)
        driveLunInfoLookupTable[SD_MMC].lunNum = NULL_LUN_NUM;
        SystemDriveLunNumSet(SD_MMC, NULL_LUN_NUM);
#elif ((SYSTEM_DRIVE == SYSTEM_DRIVE_ON_SPI) && SPI_STORAGE_ENABLE)
        driveLunInfoLookupTable[SPI_FLASH].lunNum = NULL_LUN_NUM;
        SystemDriveLunNumSet(SPI_FLASH, NULL_LUN_NUM);
#else
        driveLunInfoLookupTable[NAND].lunNum = NAND_LUN_NUM;
        SystemDriveLunNumSet(NAND, NAND_LUN_NUM);

        driveLunInfoLookupTable[SPI_FLASH].lunNum = SPI_LUN_NUM;
        SystemDriveLunNumSet(SPI_FLASH, SPI_LUN_NUM);

        driveLunInfoLookupTable[SD_MMC].lunNum = SD_MMC_LUN_NUM;
        SystemDriveLunNumSet(SD_MMC, SD_MMC_LUN_NUM);
#endif
    }
    else
    {
        MP_ALERT("%s - Disable", __FUNCTION__);

        driveLunInfoLookupTable[TOOL_DRV_ID].lunNum = NULL_LUN_NUM;
        SystemDriveLunNumSet(TOOL_DRV_ID, NULL_LUN_NUM);

        driveLunInfoLookupTable[NAND].lunNum = NAND_LUN_NUM;
        SystemDriveLunNumSet(NAND, NAND_LUN_NUM);

        driveLunInfoLookupTable[SPI_FLASH].lunNum = SPI_LUN_NUM;
        SystemDriveLunNumSet(SPI_FLASH, SPI_LUN_NUM);

        driveLunInfoLookupTable[SD_MMC].lunNum = SD_MMC_LUN_NUM;
        SystemDriveLunNumSet(SD_MMC, SD_MMC_LUN_NUM);
    }

    return PASS;
#else
    MP_ALERT("Not support function of %s", __FUNCTION__);
    return FAIL;
#endif
}



SDWORD SystemToolDriveLunEnable(BOOL enable)
{
#if (SC_USBDEVICE && TOOL_DRV_SIZE)
    WHICH_OTG otgId = Api_UsbdGetWhichOtgConnectedPc();

    if (TOOL_DRV_ID == NULL_DRIVE)
        return FAIL;

    if (otgId != USBOTG_NONE)
        Api_UsbdFinal(otgId);

    SystemToolDriveLunInfoChange(enable);
    SystemDeviceLunInfoUpdate();

    if (otgId != USBOTG_NONE)
        Api_UsbdInit(otgId);

    return PASS;
#else
    MP_ALERT("Not support function of %s", __FUNCTION__);
    return FAIL;
#endif
}



/*
void SystemDeviceLunSet(BYTE driveIndex, BYTE lun)
{
    BYTE devID = (BYTE) DriveIndex2PhyDevID((E_DRIVE_INDEX_ID)driveIndex);

    if (devID == NULL_DRIVE)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, driveIndex);
        return;
    }

    Mcard_DeviceLunSet(devID,lun);
}
*/


//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
//#define __USB_DEV_DETETC_BY_EXCEPTION_TASK__

#ifdef __USB_DEV_DETETC_BY_EXCEPTION_TASK__
static volatile BYTE dropUsbDevDetectMsg = FALSE;        // avoid message queue full
#endif
static volatile BYTE allowUsbDevDetect = FALSE;
#if (SC_USBDEVICE && defined(__USB_DEV_DETETC_BY_EXCEPTION_TASK__))
static ST_EXCEPTION_MSG stMsgContent4UsbDeviceDetect = {0};
#endif

BOOL SystemCheckUsbdPlugIn(void)
{
#if SC_USBDEVICE
    #if (USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG)
    return (Api_UsbdCheckIfConnectedPc(Api_UsbdGetWhichOtgConnectedPc()) | Api_UsbdCheckIfConnectedSideMonitor(Api_UsbdGetWhichOtgConnectedSideMonitor()));
    #else
    return Api_UsbdCheckIfConnectedPc(Api_UsbdGetWhichOtgConnectedPc());
    #endif
#else
    MP_DEBUG("USB Device Disable");
    return FALSE;
#endif
}



#if (SC_USBDEVICE)
#ifdef __USB_DEV_DETETC_BY_EXCEPTION_TASK__
static void systemOtgDevDetectTrigger(void)
{
    if (!dropUsbDevDetectMsg && allowUsbDevDetect)
    {
        SDWORD retVal;

        dropUsbDevDetectMsg = TRUE;
        retVal = MessageDrop(EXCEPTION_MSG_ID, (BYTE *) &stMsgContent4UsbDeviceDetect, sizeof(ST_EXCEPTION_MSG));

        if (retVal != OS_STATUS_OK)
        {
            dropUsbDevDetectMsg = FALSE;
            MP_ALERT("--E-- OTG DEVICE detect message drop !!!");
        }
    }
}
#endif


static void systemOtgDevDetect(void)
{
#ifdef __USB_DEV_DETETC_BY_EXCEPTION_TASK__
    dropUsbDevDetectMsg = FALSE;
#endif

    if (allowUsbDevDetect)
        UsbOtgDeviceDetect();
}
#endif



SBYTE SystemUsbdDetectEnable(void)
{
    allowUsbDevDetect = TRUE;
#if (SC_USBDEVICE && defined(__USB_DEV_DETETC_BY_EXCEPTION_TASK__))
    SysTimerProcResume(&systemOtgDevDetectTrigger);
#endif

    return OS_IDLE_FUNC_TERMINATE;
}



void SystemUsbdDetectDisable(void)
{
    allowUsbDevDetect = FALSE;
#if (SC_USBDEVICE && defined(__USB_DEV_DETETC_BY_EXCEPTION_TASK__))
    SysTimerProcPause(&systemOtgDevDetectTrigger);
#endif
}








//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
void SystemUsbDetectInit(void)
{
    allowUsbDevDetect = FALSE;

    #ifdef __USB_DEV_DETETC_BY_EXCEPTION_TASK__
    dropUsbDevDetectMsg = FALSE;
    stMsgContent4UsbDeviceDetect.dwTag = ExceptionTagReister(systemOtgDevDetect);
    SysTimerProcAdd(20, systemOtgDevDetectTrigger, FALSE);
    SysTimerProcPause(&systemOtgDevDetectTrigger);
    #else
    SysTimerProcAdd(20, systemOtgDevDetect, FALSE);
    #endif
}
#endif // SC_USBDEVICE

#if SC_USBHOST

void UsbPwdcHandler(WHICH_OTG eWhichOtg, BOOL bHostSuspend)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    PST_USB_OTG_DES psUsbOtg;
    CHANNEL *pUsbDma;
    BYTE bTimeOutTimes;

    if(UsbOtgHostConnectStatusGet(eWhichOtg) == FALSE)
    {
        MP_DEBUG("-USBOTG%d- Without Connecting with any Dongle!", eWhichOtg);
        return;
    }

    if(bHostSuspend == TRUE)  // USB Host Suspend
    {
        pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
        psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

        bTimeOutTimes = 20;
        while ((pUsbhDev->bDeviceStatus < USB_STATE_CONFIGURED) && ((bTimeOutTimes--) > 0))
            TaskSleep(100);  // Will finetune
        if ((bTimeOutTimes == 0) && (pUsbhDev->bDeviceStatus < USB_STATE_CONFIGURED))
            MP_ALERT("--E-- %s USBOTG%d Host Timeout !!!!", __FUNCTION__, eWhichOtg);

        //Disable the Asynchronous Schedule  //flib_Host20_Suspend(USBOTG0);
        MP_DEBUG("-USBOTG%d- Disable the Asynchronous Schedule", eWhichOtg);
        psUsbOtg->psUsbReg->HcUsbCommand &= ~BIT5;
        TimerDelay(100); // it's necessary to delay. can be fine tune.

        // Disable the DMA
        MP_DEBUG("-USBOTG%d- Disable the DMA", eWhichOtg);
        pUsbDma = (CHANNEL *) ( eWhichOtg == USBOTG0 ? DMA_USBHST_BASE : DMA_USBDEV_BASE );
        *((volatile DWORD *)(pUsbDma)) &= ~BIT0;
        TimerDelay(100); // it's necessary to delay. can be fine tune.

        // USB PHY Suspend
        MP_DEBUG("-USBOTG%d- USB PHY Suspend", eWhichOtg);
        psUsbOtg->psUsbReg->PhyUtmiSwCtrl |= BIT8;
        TimerDelay(100); // it's necessary to delay. can be fine tune.

        // USB INT Disable
        MP_DEBUG("-USBOTG%d- USB INT Disable", eWhichOtg);
        SystemIntDis( eWhichOtg == USBOTG0 ? IM_USB0 : IM_USB1 );
        TimerDelay(100); // it's necessary to delay. can be fine tune.

    }
    else  // USB Host Resume
    {
        psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

        if(psUsbOtg->psUsbReg->PhyUtmiSwCtrl & BIT8)
        {
            // USB BIU Reset
            UsbOtgBiuReset(eWhichOtg);
            // USBOTG PHY Reset for ChipIdea
            psUsbOtg->psUsbReg->PhyUtmiSwCtrl |= BIT8;
            IODelay(100); // it's necessary to delay. can be fine tune.
            psUsbOtg->psUsbReg->PhyUtmiSwCtrl = 0;
            IODelay(100); // it's necessary to delay. can be fine tune.
            // SW control avalid, bvalid, vbusvalid
            psUsbOtg->psUsbReg->PhyUtmiSwCtrl = BIT1|BIT2|BIT17|BIT18;
            // Disable MDev_Wakeup_byVBUS & Dev_Idle interrupt for SW control avalid, bvalid, vbusvalid
            psUsbOtg->psUsbReg->DeviceMaskofInterruptSrcGroup2 = BIT9|BIT10;

            // Enable DMA
            MP_DEBUG("-USBOTG%d- Enable DMA", eWhichOtg);
            pUsbDma = (CHANNEL *) ( eWhichOtg == USBOTG0 ? DMA_USBHST_BASE : DMA_USBDEV_BASE );
            *((volatile DWORD *)(pUsbDma)) |= BIT0;
            TimerDelay(100); // it's necessary to delay. can be fine tune.

            //Enable the Asynchronous Schedule
            //mpDebugPrint("USBOTG%d Enable the Asynchronous Schedule", eWhichOtg);
            //psUsbOtg->psUsbReg->HcUsbCommand |= BIT5;

            // USB Host Clear
            MP_DEBUG("-USBOTG%d- Host Clear", eWhichOtg);
            UsbOtgHostEventPlugOut(eWhichOtg, TRUE);

        }
    }
}

static const BYTE host_devID[] = {
    DEV_USB_HOST_ID1,       DEV_USB_HOST_ID2,       DEV_USB_HOST_ID3,       DEV_USB_HOST_ID4,       DEV_USB_HOST_PTP,
    DEV_USBOTG1_HOST_ID1,   DEV_USBOTG1_HOST_ID2,   DEV_USBOTG1_HOST_ID3,   DEV_USBOTG1_HOST_ID4,   DEV_USBOTG1_HOST_PTP
};

#define USBOTG1_ID_OFFSET       DEV_USBOTG1_HOST_ID1

void Ui_SetCardOutCodeByLun(BYTE bLun, WHICH_OTG eWhichOtg)
{
    MP_DEBUG("%s USBOTG%d Lun = %d", __FUNCTION__, eWhichOtg, bLun);

    if (bLun == (USBOTG1_HOST_PTP-1))
        SystemCardEventSet(FALSE, host_devID[bLun]);
    else
        SystemCardEventSet(FALSE, host_devID[bLun + ((eWhichOtg == USBOTG1)? (USBOTG1_ID_OFFSET-1):0)]);
}

void Ui_SetCardInCodeByLun(BYTE bLun, WHICH_OTG eWhichOtg)
{
    DWORD dwCardBitCode = 0;

    MP_DEBUG("%s USBOTG%d Lun = %d", __FUNCTION__, eWhichOtg, bLun);

    if (bLun == (USBOTG1_HOST_PTP-1))
        SystemCardEventSet(TRUE, host_devID[bLun]);
    else
        SystemCardEventSet(TRUE, host_devID[bLun + ((eWhichOtg == USBOTG1)? (USBOTG1_ID_OFFSET-1):0)]);
}

#endif // SC_USBHOST

