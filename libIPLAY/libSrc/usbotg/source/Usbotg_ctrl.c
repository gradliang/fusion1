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
* 
* Filename		: usbotg_ctrl.c
* Programmer(s)	: Joe Luo (JL) (based on the modified by Cloud Wu )
* Created Date	: 2007/08/01 
* Description   : Program Entry
******************************************************************************** 
*/

#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"

#include "usbotg_ctrl.h"
//#include "usbotg_device.h"
#include "usbotg_device_msdc.h"
#include "usbotg_device_sidc.h"

#include "taskid.h"
#include "ui.h"
#include "os.h"

#include "Peripheral.h"

#if (SC_USBHOST|SC_USBDEVICE)

#define USBOTG_DEVICE_WAIT_INIT_TIME        7000         // 5 second

#define mwOTG20_GlobalHcOtgDev_Interrupt_Status_Rd()	(psUsbOtg->psUsbReg->GlobalHcOtgDevInterruptStatus)//(gp_UsbOtg->GlobalHcOtgDevInterruptStatus) 
#define mwOTG20_Interrupt_Mask_OTG_Set()		        (mwHost20Bit_Set  (psUsbOtg->psUsbReg->GlobalMaskofHcOtgDevInterrupt, BIT1))//(mwHost20Bit_Set  (gp_UsbOtg->GlobalMaskofHcOtgDevInterrupt, BIT1))
//#define mwOTG20_Interrupt_Mask_OTG_Clr()		        (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->GlobalMaskofHcOtgDevInterrupt, BIT1))//(mwHost20Bit_Clr  (gp_UsbOtg->GlobalMaskofHcOtgDevInterrupt, BIT1))
#define mwOTG20_Interrupt_OTG_Disable()		            mwOTG20_Interrupt_Mask_OTG_Set()
//#define mwOTG20_Interrupt_OTG_Enable()		            mwOTG20_Interrupt_Mask_OTG_Clr()

#define CONFIG_DEFAULT_TOTAL_LENGTH 256

static ST_USB_OTG_DES g_stUsb[2]; // static in order to not put into sdata.

enum _GLOBAL_HC_OTG_DEV_INTERRUPT_STATUS_
{
    DEV_INT = BIT0,
    OTG_INT = BIT1,
    HC_INT  = BIT2,
};


//////////////////////////////////////////////////////////////////////
static void UsbOtgClkInit(WHICH_OTG eWhichOtg);
static void UsbOtgFinish(void);
static void UsbOtgIsr (WHICH_OTG eWhichOtg);
static void UsbOtg0Isr (void);
static void UsbOtg1Isr (void);
static void UsbOtgControlIsr (WHICH_OTG eWhichOtg);
static void UsbOtgRoleChange(WORD bRole, PST_USB_OTG_DES psUsbOtg);
static void UsbOtgDsClr(WHICH_OTG eWhichOtg);
static void UsbOtgRegSet (WHICH_OTG eWhichOtg);
static void UsbOtgMemoryFinish(WHICH_OTG eWhichOtg);
static void UsbOtgHostMemoryFinish(PST_USB_OTG_HOST psUsbHost);
static void UsbOtgDeviceMemoryFinish(PST_USB_OTG_DEVICE psUsbDev);
static void UsbOtgDeviceInit(PST_USB_OTG_DES psUsbOtg);
static void UsbOtgDeviceClose(PST_USB_OTG_DES psUsbOtg); 

static BYTE UsbOtgHostCurDevDescGet(WHICH_OTG eWhichOtg);
static BYTE UsbOtgHostCurAppClassDescGet(WHICH_OTG eWhichOtg);

static DWORD UsbOtgBufferIdGet (WHICH_OTG eWhichOtg);

static SDWORD UsbOtgFuncInit(WHICH_OTG eWhichOtg);
static SDWORD UsbOtgHostFuncInit(WHICH_OTG eWhichOtg);
static SDWORD UsbOtgDeviceFuncInit(WHICH_OTG eWhichOtg);
static SDWORD UsbOtgHostMemoryInit(PST_USB_OTG_HOST psUsbHost, BYTE bTaskId);
static SDWORD UsbOtgDeviceMemoryInit(PST_USB_OTG_DEVICE psUsbDev, BYTE bTaskId);
static SDWORD UsbOtgDmInit(WHICH_OTG eWhichOtg);
static SDWORD UsbOtgHostDmInit(WHICH_OTG eWhichOtg);
static SDWORD UsbOtgDeviceDmInit(WHICH_OTG eWhichOtg);
static SDWORD UsbOtgBufferInit(WHICH_OTG eWhichOtg);
static SDWORD UsbOtgControlInit(WHICH_OTG eWhichOtg);

static USB_OTG_FUNC UsbOtgFunctionGet(WHICH_OTG eWhichOtg);

//=================== Extern Function ==========================================================================
//========================================================================================================
void Api_UsbInit(WHICH_OTG eWhichOtg)
{
    MP_DEBUG("-USBOTG- %s &g_stUsb[0] = 0x%x; &g_stUsb[1] = 0x%x", __FUNCTION__, &g_stUsb[0], &g_stUsb[1]);
    UsbOtgDsClr(eWhichOtg);
    UsbOtgRegSet(eWhichOtg);
    UsbOtgClkInit(eWhichOtg);
}

SDWORD Api_UsbFunctionSet (USB_OTG_FUNC eUsbOtg_0_Func, USB_OTG_FUNC eUsbOtg_1_Func)
{
    SDWORD bRetVal = USBOTG_NO_ERROR;
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(USBOTG0);
    psUsbOtg->eFunction = eUsbOtg_0_Func;
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(USBOTG1);
    psUsbOtg->eFunction = eUsbOtg_1_Func;

    if (eUsbOtg_0_Func != NONE_FUNC)
    {
        bRetVal = UsbOtgFuncInit(USBOTG0);
        if (bRetVal != USBOTG_NO_ERROR)
        {
            MP_ALERT("-USBOTG- error in UsbOtgFuncInit(USBOTG0)%d!!", bRetVal);
            return bRetVal;
        }
    }
    else
    {
        MP_DEBUG("-USBOTG- USBOTG0 is set NONE_FUNC!!");
    }

    if (eUsbOtg_1_Func != NONE_FUNC)
    {
        bRetVal = UsbOtgFuncInit(USBOTG1);
        if (bRetVal != USBOTG_NO_ERROR)
        {
            MP_ALERT("-USBOTG- error in UsbOtgFuncInit(USBOTG1)%d!!", bRetVal);
            return bRetVal;
        }
    }
    else
    {
        MP_DEBUG("-USBOTG- USBOTG1 is set NONE_FUNC!!");
    }
    
    return bRetVal;
}


#if SC_USBDEVICE
void UsbOtgDeviceDsInit(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    BYTE i = 0;
    
    Api_UsbdSetCurrentStat(eWhichOtg, USB_DEV_STATE_NONE);
    //psUsbDev->boIsAlreadyInit = FALSE; // Mark:Init-Do;Final-NoDo
    psUsbDev->stDetect.bPlugFlag = 0;
    psUsbDev->stDetect.bStableCount = 0;
    psUsbDev->sCdc.dwCdcBulkReadIdx = 0;
    psUsbDev->sCdc.dwCdcBulkWriteIdx= 0;
    psUsbDev->sCdc.boIsAbleToUseUsbdCdc = FALSE;
    psUsbDev->sCdc.wTotalByteCnt = 0;
    psUsbDev->sCdc.wByteIndex = 0;

    psUsbDev->sMsdc.boIsReadWriteData = FALSE;
    for (i= 0; i < MSDC_MAX_LUN; i++)
    {
        psUsbDev->sMsdc.boMediaNotPresent[i]= FALSE;
    }

    #if USBOTG_DEVICE_MSDC_TECO
    psUsbDev->sMsdc.pbTecoDataBuff = 0;
    #endif    

#if USBOTG_DEVICE_SIDC
    memset(psUsbDev->sSidc.bStrFile, 0, sizeof(psUsbDev->sSidc.bStrFile));
    psUsbDev->sSidc.dwPtpStorageIDs = DEVICE_STORAGE_ID;
    psUsbDev->sSidc.dwRootObjectHandle = 0;
    psUsbDev->sSidc.boIsImgObjectHandleReady = FALSE;
    psUsbDev->sSidc.dwSessionID = 0;
    psUsbDev->sSidc.dwTransactionID = 0;
    psUsbDev->sSidc.dwStorageID = 0;
    psUsbDev->sSidc.dwPtpNumHandles = 0;
    psUsbDev->sSidc.boDpsEnabled = FALSE;
    psUsbDev->sSidc.dwDpsCurrHdl = 0;
    psUsbDev->sSidc.dwDpsDDiscHdl = 0;
    psUsbDev->sSidc.dwDpsDReqHdl = 0;
    psUsbDev->sSidc.dwDpsDRespHdl = 0;
    psUsbDev->sSidc.dwDpsHDiscHdl = 0;
    psUsbDev->sSidc.dwDpsHReqHdl = 0;
    psUsbDev->sSidc.dwDpsHRespHdl = 0;
    psUsbDev->sSidc.dwDpsReqLen = 0;
    psUsbDev->sSidc.dwDpsRespLen = 0;
    psUsbDev->sSidc.dwDpsState = 0;
    psUsbDev->sSidc.bPtpDataSource = 4;//PTP_DATA_IDLE;
    psUsbDev->sSidc.boIsHostReq = FALSE;
    psUsbDev->sSidc.boIsDeviceReq = FALSE;
    psUsbDev->sSidc.psFile = 0;
    psUsbDev->sSidc.boIsFileSending = FALSE;
    psUsbDev->sSidc.sdwImageSize = 0;
    psUsbDev->sSidc.dwImageSizeRead = 0;
    psUsbDev->sSidc.boIsDpsSendEvent = 0;
    psUsbDev->sSidc.boIsSendingData = FALSE;
    psUsbDev->sSidc.wPrintedFileType = 0;
    psUsbDev->sSidc.bIsThumbnail = 0;
    psUsbDev->sSidc.dwTotalLen = 0;
    psUsbDev->sSidc.bIsOneHandle = FALSE;

    PtpHandlesFree(eWhichOtg);
    DpsPrintInfoFree(eWhichOtg);
#endif // USBOTG_DEVICE_SIDC
}

BYTE UsbOtgDeviceMessageIdGet (WHICH_OTG eWhichOtg)
{
    if (USBOTG0 == eWhichOtg)
    {
        return USBOTG0_DEVICE_MESSAGE_ID;
    }
    else if (USBOTG1 == eWhichOtg)
    {
        return USBOTG1_DEVICE_MESSAGE_ID;
    }
    else
        return 0;
}

BYTE UsbOtgDeviceTaskIdGet (WHICH_OTG eWhichOtg)
{
    if (eWhichOtg == USBOTG0)
    {
        return USBOTG0_DEVICE_TASK;
    }
    else if (eWhichOtg == USBOTG1)
    {
        return USBOTG1_DEVICE_TASK;
    }
    else
        return 0;
}


PUSB_DEVICE_DESC UsbOtgDevDescGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psUsbDev;

    psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    return &psUsbDev->sDesc;
}

PUSB_DEVICE_CDC UsbOtgDevCdcGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psUsbDev;

    psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    return &psUsbDev->sCdc;
}

PUSB_DEVICE_SIDC UsbOtgDevSidcGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psUsbDev;

    psUsbDev =(PST_USB_OTG_DEVICE) UsbOtgDevDsGet(eWhichOtg);
    return &psUsbDev->sSidc;
}

PPICT_BRIDGE_DPS UsbOtgDevSidcPtpDpsGet (WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC psUsbDevSidc;

    psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    return &psUsbDevSidc->sDps;
}

/*
PSETUP_PACKET GetUsbOtgDevSetup (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psUsbDev;

    psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    return psUsbDev->psControlCmd;
}
*/

PUSB_DEVICE_MSDC UsbOtgDevMsdcGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psUsbDev;

    psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    return &psUsbDev->sMsdc;
}

PST_USB_OTG_DEVICE UsbOtgDevDsGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbDev;
}

BOOL UsbOtgDeviceCheckIdPin(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    
    return (mdwOTGC_Control_A_VBUS_VLD_Rd()&& mdwOTGC_Control_ID_Rd());
}



static SDWORD UsbOtgDeviceMemoryInit(PST_USB_OTG_DEVICE psUsbDev, BYTE bTaskId)
{
    DWORD dwMemTmp = 0;

    psUsbDev->psControlCmd = 0;
    psUsbDev->bCurrentStat = USB_DEV_STATE_NONE;
    psUsbDev->sDesc.pbConfigHs = 0;
    psUsbDev->sDesc.pbConfigFs = 0;
    psUsbDev->sDesc.pbConfigHsOtherSpeedDesc = 0;
    psUsbDev->sDesc.pbConfigFsOtherSpeedDesc = 0;    
/*
    psUsbDev->sDesc.pbMsdcDeviceDescriptor = 0;
    psUsbDev->sDesc.pbSidcDeviceDescriptor = 0;
    psUsbDev->sDesc.pbVendorDeviceDescriptor = 0;
    psUsbDev->sDesc.pbCdcDeviceDescriptor = 0;
    psUsbDev->sDesc.pbConfigDescriptor = 0;
    psUsbDev->sDesc.pbMsdcInterfaceDescriptor = 0;
    psUsbDev->sDesc.pbSidcInterfaceDescriptor = 0;
    psUsbDev->sDesc.pbVendorInterfaceDescriptor = 0;
    psUsbDev->sDesc.pbCdcInterfaceDescriptor1 = 0;
    psUsbDev->sDesc.pbCdcInterfaceDescriptor2 = 0;
    psUsbDev->sDesc.pbCdcFunctionalDescriptor1 = 0;
    psUsbDev->sDesc.pbCdcFunctionalDescriptor2 = 0;
    psUsbDev->sDesc.pbCdcFunctionalDescriptor3 = 0;
    psUsbDev->sDesc.pbHsEndpointDescriptorEp1 = 0;
    psUsbDev->sDesc.pbHsEndpointDescriptorEp2 = 0;
    psUsbDev->sDesc.pbHsEndpointDescriptorEp3 = 0;
    psUsbDev->sDesc.pbFsEndpointDescriptorEp1 = 0;
    psUsbDev->sDesc.pbFsEndpointDescriptorEp2 = 0;
    psUsbDev->sDesc.pbFsEndpointDescriptorEp3 = 0;
    psUsbDev->sDesc.pbLangIdString = 0;
    psUsbDev->sDesc.pbManufacturerString = 0;
    psUsbDev->sDesc.pbProductString = 0;
    psUsbDev->sDesc.pbCdcProductString = 0;
    psUsbDev->sDesc.pbSerialnumberString = 0;
    psUsbDev->sDesc.pbDeviceQualifierDescriptor = 0;
    psUsbDev->sDesc.pbOtherSpeedConfigurationDescriptor = 0;
    psUsbDev->sDesc.pbOtherSpeedCdcConfigurationDescriptor = 0;
*/
    psUsbDev->sMsdc.psCsw = 0;
    psUsbDev->sMsdc.psCbw = 0;
    psUsbDev->sSidc.psStiCommand = 0;
    psUsbDev->sSidc.psStiResponse = 0;
    psUsbDev->sSidc.psStiEvent = 0;
//    psUsbDev->sSidc.pbCancelRequestData = 0;
//    psUsbDev->sSidc.pbGetDeviceStatusRequest = 0;
    psUsbDev->sSidc.psPtpHandles = 0;
    psUsbDev->sSidc.pbDpsReqBuf = 0;
    psUsbDev->sSidc.pbDpsRespBuf = 0;  
    psUsbDev->sSidc.sDps.pbXmlBuff = 0;
    psUsbDev->sSidc.sDps.pstPrintInfo = 0;

    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (SETUP_PACKET), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 1");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbDev->psControlCmd = (PSETUP_PACKET) (dwMemTmp|0xA0000000);
    }

    psUsbDev->sDesc.pbConfigHs = (BYTE*) ker_mem_malloc(CONFIG_DEFAULT_TOTAL_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbConfigHs == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 2");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbConfigFs = (BYTE*) ker_mem_malloc(CONFIG_DEFAULT_TOTAL_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbConfigFs == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 3");
        return USBOTG_NO_MEMORY;
    }
    
    psUsbDev->sDesc.pbConfigHsOtherSpeedDesc = (BYTE*) ker_mem_malloc(CONFIG_DEFAULT_TOTAL_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbConfigHsOtherSpeedDesc == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 4");
        return USBOTG_NO_MEMORY;
    }
    
    psUsbDev->sDesc.pbConfigFsOtherSpeedDesc = (BYTE*) ker_mem_malloc(CONFIG_DEFAULT_TOTAL_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbConfigFsOtherSpeedDesc == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 5");
        return USBOTG_NO_MEMORY;
    }

/*
    psUsbDev->sDesc.pbMsdcDeviceDescriptor = (BYTE*) ker_mem_malloc(DEVICE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbMsdcDeviceDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 6");
        return USBOTG_NO_MEMORY;
    }
    
    psUsbDev->sDesc.pbSidcDeviceDescriptor = (BYTE*) ker_mem_malloc(DEVICE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbSidcDeviceDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 7");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbVendorDeviceDescriptor = (BYTE*) ker_mem_malloc(DEVICE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbVendorDeviceDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 8");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbCdcDeviceDescriptor = (BYTE*) ker_mem_malloc(DEVICE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbCdcDeviceDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 9");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbConfigDescriptor = (BYTE*) ker_mem_malloc(CONFIG_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbConfigDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 10");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbMsdcInterfaceDescriptor = (BYTE*) ker_mem_malloc(INTERFACE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbMsdcInterfaceDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 11");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbSidcInterfaceDescriptor = (BYTE*) ker_mem_malloc(INTERFACE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbSidcInterfaceDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 12");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbVendorInterfaceDescriptor = (BYTE*) ker_mem_malloc(INTERFACE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbVendorInterfaceDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 13");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbCdcInterfaceDescriptor1 = (BYTE*) ker_mem_malloc(INTERFACE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbCdcInterfaceDescriptor1 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 15");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbCdcInterfaceDescriptor2 = (BYTE*) ker_mem_malloc(INTERFACE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbCdcInterfaceDescriptor2 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 16");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbCdcFunctionalDescriptor1 = (BYTE*) ker_mem_malloc(5, bTaskId);
    if (psUsbDev->sDesc.pbCdcFunctionalDescriptor1 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 17");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbCdcFunctionalDescriptor2 = (BYTE*) ker_mem_malloc(4, bTaskId);
    if (psUsbDev->sDesc.pbCdcFunctionalDescriptor2 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 18");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbCdcFunctionalDescriptor3 = (BYTE*) ker_mem_malloc(5, bTaskId);
    if (psUsbDev->sDesc.pbCdcFunctionalDescriptor3 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 19");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbHsEndpointDescriptorEp1 = (BYTE*) ker_mem_malloc(EP_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbHsEndpointDescriptorEp1 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 20");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbHsEndpointDescriptorEp2 = (BYTE*) ker_mem_malloc(EP_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbHsEndpointDescriptorEp2 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 21");
        return USBOTG_NO_MEMORY;
    }
    
    psUsbDev->sDesc.pbHsEndpointDescriptorEp3 = (BYTE*) ker_mem_malloc(EP_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbHsEndpointDescriptorEp3 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 22");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbFsEndpointDescriptorEp1 = (BYTE*) ker_mem_malloc(EP_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbFsEndpointDescriptorEp1 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 23");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbFsEndpointDescriptorEp2 = (BYTE*) ker_mem_malloc(EP_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbFsEndpointDescriptorEp2 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 24");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbFsEndpointDescriptorEp3 = (BYTE*) ker_mem_malloc(EP_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbFsEndpointDescriptorEp3 == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 25");
        return USBOTG_NO_MEMORY;
    }

    
    psUsbDev->sDesc.pbLangIdString = (BYTE*) ker_mem_malloc(4, bTaskId);
    if (psUsbDev->sDesc.pbLangIdString == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 26");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbManufacturerString = (BYTE*) ker_mem_malloc(MANUFACTURER_SRING_LEN, bTaskId);
    if (psUsbDev->sDesc.pbManufacturerString == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 27");
        return USBOTG_NO_MEMORY;
    }
    
    psUsbDev->sDesc.pbProductString = (BYTE*) ker_mem_malloc(PRODUCT_SRING_LEN, bTaskId);
    if (psUsbDev->sDesc.pbProductString == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 28");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbCdcProductString = (BYTE*) ker_mem_malloc(20, bTaskId);
    if (psUsbDev->sDesc.pbCdcProductString == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 29");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbSerialnumberString = (BYTE*) ker_mem_malloc(SERIALNUMBER_SRING_LEN, bTaskId);
    if (psUsbDev->sDesc.pbSerialnumberString == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 30");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbDeviceQualifierDescriptor = (BYTE*) ker_mem_malloc(DEVICE_QUALIFIER_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbDeviceQualifierDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 31");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbOtherSpeedConfigurationDescriptor = (BYTE*) ker_mem_malloc(INTERFACE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbOtherSpeedConfigurationDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 32");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sDesc.pbOtherSpeedCdcConfigurationDescriptor = (BYTE*) ker_mem_malloc(INTERFACE_LENGTH, bTaskId);
    if (psUsbDev->sDesc.pbOtherSpeedCdcConfigurationDescriptor == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 33");
        return USBOTG_NO_MEMORY;
    }
*/
    // MSDC
    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (CBW), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 34");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbDev->sMsdc.psCbw = (PCBW) (dwMemTmp|0xA0000000);
    }

    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (CSW), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 35");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbDev->sMsdc.psCsw = (PCSW) (dwMemTmp|0xA0000000);
    }

    //SIDC
    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (STI_CONTAINER), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 36");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbDev->sSidc.psStiCommand = (PSTI_CONTAINER) (dwMemTmp|0xA0000000);
    }

    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (STI_CONTAINER), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 37");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbDev->sSidc.psStiResponse = (PSTI_CONTAINER) (dwMemTmp|0xA0000000);
    }

    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (STI_EVENT), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 38");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbDev->sSidc.psStiEvent = (PSTI_EVENT) (dwMemTmp|0xA0000000);
    }

    /*
    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (CANCEL_REQUEST_DATA_LENGTH), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 39");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbDev->sSidc.pbCancelRequestData = (BYTE*) (dwMemTmp|0xA0000000);
    }
    */
    /*
    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (GET_DEVICE_STATUS_REQUETS_LENGTH), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_DEBUG("-USBOTG- Device not enough memory 40");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbDev->sSidc.pbGetDeviceStatusRequest = (BYTE*) (dwMemTmp|0xA0000000);
    }
    */

/* Use alloc method
    // MAX_DEV_NUM_OF_OBJECTS : 500
    psUsbDev->sSidc.psPtpHandles = (POBJECT_HEADER) ker_mem_malloc(MAX_DEV_NUM_OF_OBJECTS * sizeof(OBJECT_HEADER), bTaskId);
    if (psUsbDev->sSidc.psPtpHandles == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 41");
        return USBOTG_NO_MEMORY;
    }
*/

    psUsbDev->sSidc.pbDpsReqBuf = (BYTE*) ker_mem_malloc(PTP_DPS_MAX_XFR, bTaskId);
    if (psUsbDev->sSidc.pbDpsReqBuf == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 42");
        return USBOTG_NO_MEMORY;
    }

    psUsbDev->sSidc.pbDpsRespBuf = (BYTE*) ker_mem_malloc(PTP_DPS_MAX_XFR, bTaskId);
    if (psUsbDev->sSidc.pbDpsRespBuf == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 43");
        return USBOTG_NO_MEMORY;
    }
    
    psUsbDev->sSidc.sDps.pbXmlBuff = (BYTE*) ker_mem_malloc(DPS_BUFF_SIZE, bTaskId);
    if (psUsbDev->sSidc.sDps.pbXmlBuff == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 44");
        return USBOTG_NO_MEMORY;
    }

/* Use alloc method
    // MAX_FILE_PRINT : 50
    psUsbDev->sSidc.sDps.pstPrintInfo = (DpsPrintJobInfo*) ker_mem_malloc(sizeof(DpsPrintJobInfo)*MAX_FILE_PRINT, bTaskId);
    if (psUsbDev->sSidc.sDps.pstPrintInfo == 0)
    {
        MP_ALERT("-USBOTG- Device not enough memory 45");
        return USBOTG_NO_MEMORY;
    }       
*/    
    return USBOTG_NO_ERROR;
}

static void UsbOtgDeviceMemoryFinish(PST_USB_OTG_DEVICE psUsbDev)
{
    if (psUsbDev->psControlCmd != 0)
        ker_mem_free (psUsbDev->psControlCmd);    
    if (psUsbDev->sDesc.pbConfigHs != 0)
        ker_mem_free (psUsbDev->sDesc.pbConfigHs);    
    if (psUsbDev->sDesc.pbConfigFs != 0)
        ker_mem_free (psUsbDev->sDesc.pbConfigFs);    
    if (psUsbDev->sDesc.pbConfigHsOtherSpeedDesc != 0)
        ker_mem_free (psUsbDev->sDesc.pbConfigHsOtherSpeedDesc);    
    if (psUsbDev->sDesc.pbConfigFsOtherSpeedDesc != 0)
        ker_mem_free (psUsbDev->sDesc.pbConfigFsOtherSpeedDesc);    
/*
    if (psUsbDev->sDesc.pbMsdcDeviceDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbMsdcDeviceDescriptor);    
    if (psUsbDev->sDesc.pbSidcDeviceDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbSidcDeviceDescriptor);    
    if (psUsbDev->sDesc.pbVendorDeviceDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbVendorDeviceDescriptor);    
    if (psUsbDev->sDesc.pbCdcDeviceDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbCdcDeviceDescriptor);    
    if (psUsbDev->sDesc.pbConfigDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbConfigDescriptor);    
    if (psUsbDev->sDesc.pbMsdcInterfaceDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbMsdcInterfaceDescriptor);    
    if (psUsbDev->sDesc.pbSidcInterfaceDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbSidcInterfaceDescriptor);    
    if (psUsbDev->sDesc.pbVendorInterfaceDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbVendorInterfaceDescriptor);    
    if (psUsbDev->sDesc.pbCdcInterfaceDescriptor1 != 0)
        ker_mem_free (psUsbDev->sDesc.pbCdcInterfaceDescriptor1);    
    if (psUsbDev->sDesc.pbCdcInterfaceDescriptor2 != 0)
        ker_mem_free (psUsbDev->sDesc.pbCdcInterfaceDescriptor2);    
    if (psUsbDev->sDesc.pbCdcFunctionalDescriptor1 != 0)
        ker_mem_free (psUsbDev->sDesc.pbCdcFunctionalDescriptor1);    
    if (psUsbDev->sDesc.pbCdcFunctionalDescriptor2 != 0)
        ker_mem_free (psUsbDev->sDesc.pbCdcFunctionalDescriptor2);    
    if (psUsbDev->sDesc.pbCdcFunctionalDescriptor3 != 0)
        ker_mem_free (psUsbDev->sDesc.pbCdcFunctionalDescriptor3);    
    if (psUsbDev->sDesc.pbHsEndpointDescriptorEp1 != 0)
        ker_mem_free (psUsbDev->sDesc.pbHsEndpointDescriptorEp1);    
    if (psUsbDev->sDesc.pbHsEndpointDescriptorEp2 != 0)
        ker_mem_free (psUsbDev->sDesc.pbHsEndpointDescriptorEp2);    
    if (psUsbDev->sDesc.pbHsEndpointDescriptorEp3 != 0)
        ker_mem_free (psUsbDev->sDesc.pbHsEndpointDescriptorEp3);    
    if (psUsbDev->sDesc.pbFsEndpointDescriptorEp1 != 0)
        ker_mem_free (psUsbDev->sDesc.pbFsEndpointDescriptorEp1);    
    if (psUsbDev->sDesc.pbFsEndpointDescriptorEp2 != 0)
        ker_mem_free (psUsbDev->sDesc.pbFsEndpointDescriptorEp2);    
    if (psUsbDev->sDesc.pbFsEndpointDescriptorEp3 != 0)
        ker_mem_free (psUsbDev->sDesc.pbFsEndpointDescriptorEp3);    
    if (psUsbDev->sDesc.pbLangIdString != 0)
        ker_mem_free (psUsbDev->sDesc.pbLangIdString);    
    if (psUsbDev->sDesc.pbManufacturerString != 0)
        ker_mem_free (psUsbDev->sDesc.pbManufacturerString);    
    if (psUsbDev->sDesc.pbProductString != 0)
        ker_mem_free (psUsbDev->sDesc.pbProductString);    
    if (psUsbDev->sDesc.pbCdcProductString != 0)
        ker_mem_free (psUsbDev->sDesc.pbCdcProductString);    
    if (psUsbDev->sDesc.pbSerialnumberString != 0)
        ker_mem_free (psUsbDev->sDesc.pbSerialnumberString);    
    if (psUsbDev->sDesc.pbDeviceQualifierDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbDeviceQualifierDescriptor);    
    if (psUsbDev->sDesc.pbOtherSpeedConfigurationDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbOtherSpeedConfigurationDescriptor);    
    if (psUsbDev->sDesc.pbOtherSpeedCdcConfigurationDescriptor != 0)
        ker_mem_free (psUsbDev->sDesc.pbOtherSpeedCdcConfigurationDescriptor);    
*/
    // MSDC
    if (psUsbDev->sMsdc.psCbw != 0)
        ker_mem_free (psUsbDev->sMsdc.psCbw);    
    if (psUsbDev->sMsdc.psCsw != 0)
        ker_mem_free (psUsbDev->sMsdc.psCsw);
    //SIDC
    if (psUsbDev->sSidc.psStiCommand != 0)
        ker_mem_free (psUsbDev->sSidc.psStiCommand);    
    if (psUsbDev->sSidc.psStiResponse != 0)
        ker_mem_free (psUsbDev->sSidc.psStiResponse);    
    if (psUsbDev->sSidc.psStiEvent != 0)
        ker_mem_free (psUsbDev->sSidc.psStiEvent);    
    //if (psUsbDev->sSidc.pbCancelRequestData != 0)
    //    ker_mem_free (psUsbDev->sSidc.pbCancelRequestData);    
    //if (psUsbDev->sSidc.pbGetDeviceStatusRequest != 0)
    //    ker_mem_free (psUsbDev->sSidc.pbGetDeviceStatusRequest);    
}

void UsbOtgDeviceReInit(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    MP_DEBUG("--W-- %s USBOTG%d Device ReInit!", __FUNCTION__, eWhichOtg);
    UsbOtgBiuReset(eWhichOtg);

    // MP65x do this action when reset
    // USBOTG PHY Reset for ChipIdea
    psUsbOtg->psUsbReg->PhyUtmiSwCtrl |= BIT8;
    IODelay(100); // it's necessary to delay. can be fine tune.
    psUsbOtg->psUsbReg->PhyUtmiSwCtrl = 0;
    IODelay(100); // it's necessary to delay. can be fine tune.
    // SW control avalid, bvalid, vbusvalid 
    psUsbOtg->psUsbReg->PhyUtmiSwCtrl = BIT1|BIT2|BIT17|BIT18;
    // Disable MDev_Wakeup_byVBUS & Dev_Idle interrupt for SW control avalid, bvalid, vbusvalid
    psUsbOtg->psUsbReg->DeviceMaskofInterruptSrcGroup2 = BIT9|BIT10;

    #if 1
    mwOTG20_Interrupt_OTG_Enable();
    #else
    mdwOTGC_Control_Internal_Test();
    mwOTG20_Interrupt_OTG_Enable();
    #endif
    mdwOTGC_INT_Enable_Set(OTGC_INT_B_TYPE);    //Enable OTGC interrupt 
#if SC_USBDEVICE
    UsbOtgDeviceInit(psUsbOtg);
#endif //SC_USBDEVICE
#if (SC_USBHOST==ENABLE)
    InitiaizeUsbOtgHost(1, eWhichOtg);
#endif //SC_USBHOST
}



//============================================================================= ok
//		UsbOtgDeviceClose()
//		Description:
//		input: Reserved
//		output: Reserved
//=============================================================================
static void UsbOtgDeviceClose(PST_USB_OTG_DES psUsbOtg) 
{
	DWORD wTemp;

	//<1>.Clear All the Interrupt
	//wFOTGPeri_Port(0x100) &= ~BIT2
	//*((DWORD *)(FOTG200_BASE_ADDRESS | (0x100))) &= ~BIT2;
	mUsbOtgGlobIntDis();

	//<2>.Clear all the Interrupt Status
	wTemp=mUsbOtgIntGroupRegRd();
	mUsbOtgIntGroupRegSet(0);
	
	//Interrupt source group 0(0x144)
	wTemp=mUsbOtgIntSrc0Rd();
	mUsbOtgIntSrc0Set(0);          
	
	//Interrupt source group 1(0x148)          
	wTemp=mUsbOtgIntSrc1Rd();
	mUsbOtgIntSrc1Set(0);          
	
	//Interrupt source group 2(0x14C)          
	wTemp=mUsbOtgIntSrc2Rd();
	mUsbOtgIntSrc2Set(0);    

   //<3>.Turn off D+  
   if (mdwOTGC_Control_CROLE_Rd()==USB_OTG_PERIPHERAL)//For Current Role = Peripheral
   {
        MP_DEBUG("%s mUsbOtgUnPLGSet",__FUNCTION__);
        mUsbOtgUnPLGSet();
   }
}

static void UsbOtgDeviceInit(PST_USB_OTG_DES psUsbOtg)
{
    mdwOTGC_Control_Internal_Test();
    mUsbGC_MHC_INT_Dis();
    mUsbOtgChipEnSet();
/////////////////////////////////////////////////////////////////////////
//    UsbOtgControllerInit();                             // OTG Device registers Initialize

    mdwOTGC_Control_A_BUS_REQ_Clr();
    mdwOTGC_Control_A_BUS_DROP_Set();
/////////////////////////////////////////////////////////////////////////

   
    //psDevDesc->boIsUsbdInitiated = TRUE;
    //UsbOTGDeviceInit();
    //vUsbCxLoopBackTest();
    //mUsbOtgIntDevIdleDis();       //  Disable Device Idle interrupt to avoid the USBOTG interrupt issue constantly
    //mUsbOtgGlobIntEnSet(); 
}


static SDWORD UsbOtgDeviceFuncInit(WHICH_OTG eWhichOtg)
{
    SDWORD bRetVal = USBOTG_NO_ERROR;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    
    //UsbOtgRegSet(eWhichOtg);
    bRetVal = UsbOtgBufferInit(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_DEBUG("-USBOTG- UsbOtgBufferInit fail!!");
        return bRetVal;
    }

    bRetVal = UsbOtgDeviceDmInit(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_DEBUG("-USBOTG- not enough memory in UsbOtgDeviceDmInit!!");
        return bRetVal;
    }

    return bRetVal;
}

static SDWORD UsbOtgDeviceDmInit(WHICH_OTG eWhichOtg)
{
    SDWORD bRetVal = USBOTG_NO_ERROR;
    PST_USB_OTG_DES psUsbOtg;
    BYTE bTaskId;

    bTaskId = UsbOtgDeviceTaskIdGet(eWhichOtg);
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    bRetVal = UsbOtgDeviceMemoryInit(&psUsbOtg->sUsbDev, bTaskId);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- not enough memory in UsbOtgDeviceMemoryInit with %d!!", eWhichOtg, bTaskId);
        return bRetVal;
    }
    UsbOtgDeviceDsInit(eWhichOtg);
    psUsbOtg->sUsbDev.boIsAlreadyInit = FALSE;
    
    return bRetVal;
}


#endif

#if SC_USBHOST
void UsbOtgHostDsInit(PST_USB_OTG_HOST psUsbHost)
{
    BYTE i = 0;


    psUsbHost->boIsAlreadyInit = FALSE;
    psUsbHost->bCurUsbhDeviceDescriptorIndex = 0;  // MAX_NUM_OF_DEVICE
    psUsbHost->bCurAppClassDescriptorIndex = 0;    // MAX_NUM_OF_DEVICE
    psUsbHost->boIsHostFinalized = FALSE;
    psUsbHost->boIsHostToResetDevice = FALSE;
    psUsbHost->boIsReadyToReadWriteUsbh = FALSE;
    psUsbHost->boIsUsbhContinueToResetDevice = TRUE;
    psUsbHost->dwRetryCnt = 0;
    psUsbHost->dwLastDriverEvent = EVENT_DEVICE_PLUG_IN;
    psUsbHost->dwChkCount = 0;
    psUsbHost->boIocPolling = 0;
    
    psUsbHost->psUsbhDeviceDescriptor->bNumOfIsocFrameList = 0;
    
    psUsbHost->stSetupSm.bEofRetryCnt = 0;
    psUsbHost->stSetupSm.bConfigIndex = 0;
    psUsbHost->stSetupSm.bInterfaceIndex = 0;
    psUsbHost->stSetupSm.boIsProcessedBiuReset = 0;
    psUsbHost->stSetupSm.boIsHostToResetDevice = 0;

    psUsbHost->stHub.wSetPortFeatureSelector = 0;
    psUsbHost->stHub.wClearPortFeatureSelector = 0;
    psUsbHost->stHub.wPortNumber = 1;
    psUsbHost->stHub.bInitPortNum = 1;

    psUsbHost->sEhci.dwHostStructureBaseAddress = 0;
    psUsbHost->sEhci.dwHostStructureQhdBaseAddress = 0;
    psUsbHost->sEhci.dwHostStructureQtdBaseAddress = 0;
    psUsbHost->sEhci.dwHostStructurePflBaseAddress = 0;
    psUsbHost->sEhci.dwHostMemoryBaseAddress = 0;
    psUsbHost->sEhci.dwHostQhdQtdHandleBaseAddress = 0;
    psUsbHost->sEhci.dwHostSidcRxBufferAddress = 0;
    psUsbHost->sEhci.dwHostSidcGetObjecRxBufferAddress = 0;
    psUsbHost->sEhci.dwControlRemainBytes = 0;
    psUsbHost->sEhci.dwHdTdBufferIndex = 0;
    psUsbHost->sEhci.dwHdBaseBufferIndex = 0;
    psUsbHost->sEhci.dwIntervalMap[0] = 1;  
    psUsbHost->sEhci.dwIntervalMap[1] = 2;  
    psUsbHost->sEhci.dwIntervalMap[2] = 4;  
    psUsbHost->sEhci.dwIntervalMap[3] = 8;  
    psUsbHost->sEhci.dwIntervalMap[4] = 16;  
    psUsbHost->sEhci.dwIntervalMap[5] = 32;  
    psUsbHost->sEhci.dwIntervalMap[6] = 64;  
    psUsbHost->sEhci.dwIntervalMap[7] = 128;  
    psUsbHost->sEhci.dwIntervalMap[8] = 256;  
    psUsbHost->sEhci.dwIntervalMap[9] = 512;  
    psUsbHost->sEhci.dwIntervalMap[10] = 1024;
    psUsbHost->sEhci.psPreviousTempqTD = OTGH_NULL;
    psUsbHost->sEhci.psFirstTempqTD = OTGH_NULL;
#if (USBOTG_HOST_CDC == ENABLE)
    psUsbHost->sCdc.pbCdcVendorCmdData[0] = 0x00;
    psUsbHost->sCdc.pbCdcVendorCmdData[1] = 0x00;
    psUsbHost->sCdc.pbCdcVendorCmdData[2] = 0x00;
    psUsbHost->sCdc.pbCdcVendorCmdData[3] = 0x00;
    psUsbHost->sCdc.pbCdcVendorCmdData[4] = 0xff;
    psUsbHost->sCdc.pbCdcVendorCmdData[5] = 0xff;
    psUsbHost->sCdc.pbCdcVendorCmdData[6] = 0xff;
    psUsbHost->sCdc.pbCdcVendorCmdData[7] = 0xff;
    psUsbHost->sCdc.pbCdcVendorCmdData[8] = 0x00;
    psUsbHost->sCdc.pbCdcVendorCmdData[9] = 0x02;
    psUsbHost->sCdc.pbCdcVendorCmdData[10] = 0x90;
    psUsbHost->sCdc.pbCdcVendorCmdData[11] = 0x30;
    psUsbHost->sCdc.pbCdcVendorCmdData[12] = 0x00;
    psUsbHost->sCdc.pbCdcVendorCmdData[13] = 0x00;
    psUsbHost->sCdc.pbCdcVendorCmdData[14] = 0x00;
    psUsbHost->sCdc.pbCdcVendorCmdData[15] = 0x00;
    psUsbHost->sCdc.pbCdcSetLineCodingData[0] = 0x80;
    psUsbHost->sCdc.pbCdcSetLineCodingData[1] = 0x25;
    psUsbHost->sCdc.pbCdcSetLineCodingData[2] = 0x00;
    psUsbHost->sCdc.pbCdcSetLineCodingData[3] = 0x00;
    psUsbHost->sCdc.pbCdcSetLineCodingData[4] = 0x00;
    psUsbHost->sCdc.pbCdcSetLineCodingData[5] = 0x00;
    psUsbHost->sCdc.pbCdcSetLineCodingData[6] = 0x00;
    psUsbHost->sCdc.pbCdcSetLineCodingData[7] = 0x08;
#endif

    psUsbHost->sMsdc.dwCbwTag = 0;
    psUsbHost->sMsdc.boIsModeSense6TryAgain = FALSE;
    psUsbHost->sMsdc.boSupportMultiLun = FALSE;
    psUsbHost->sMsdc.boIsMsdcBulkOnly = FALSE;
    psUsbHost->sMsdc.boNoNeedToPollingUsbDevice = FALSE;
    psUsbHost->sMsdc.boIsCmpTimerStart = FALSE;
    psUsbHost->sMsdc.bUsbhDeviceAddress = 0;
    
#if USBOTG_HOST_USBIF
    psUsbHost->sMsdc.bUsbIfTestSts = 0; // USB_IF_TEST_DISABLE
#endif

	strcpy(psUsbHost->sMsdc.bDescriptor[0], "LUN_0");
	strcpy(psUsbHost->sMsdc.bDescriptor[1], "LUN_1");
	strcpy(psUsbHost->sMsdc.bDescriptor[2], "LUN_2");
	strcpy(psUsbHost->sMsdc.bDescriptor[3], "LUN_3");
	strcpy(psUsbHost->sMsdc.bDescriptor[4], "PTP_0");
    
    psUsbHost->sSidc.dwTransactionId = 0;
    psUsbHost->sSidc.boScanObjects = FALSE;
    psUsbHost->sSidc.boGetPartialObject = FALSE;
    psUsbHost->sSidc.boGetObject = FALSE;
    psUsbHost->sSidc.boGetObjectInfo = FALSE;
    psUsbHost->sSidc.dwBuffer = 0;
    psUsbHost->sSidc.dwCount = 0;
    psUsbHost->sSidc.stScanObject.dwTotalNumberOfStorages = 0;
    psUsbHost->sSidc.stScanObject.dwIndexOfStorages = 0;
    psUsbHost->sSidc.stScanObject.dwTotalNumberOfObjects = 0;
    psUsbHost->sSidc.stScanObject.dwIndexOfObjects = 0;
    psUsbHost->sSidc.stScanObject.dwTotalNumberOfFolderObjects = 0;
    psUsbHost->sSidc.stScanObject.dwIndexOfFolderObjects = 0;
    psUsbHost->sSidc.stScanObject.dwTotalNumberOfVideoObjects = 0;
    psUsbHost->sSidc.stScanObject.dwIndexOfVideoObjects = 0;
    psUsbHost->sSidc.stScanObject.dwTotalNumberOfAudioObjects = 0;
    psUsbHost->sSidc.stScanObject.dwIndexOfAudioObjects = 0;
    psUsbHost->sSidc.stScanObject.dwTotalNumberOfPictureObjects = 0;
    psUsbHost->sSidc.stScanObject.dwIndexOfPictureObjects = 0;
    psUsbHost->sSidc.stScanObject.boIsSearchFileInfo = FALSE;
    psUsbHost->sSidc.stScanObject.dwObjectCount = 0;
    psUsbHost->sSidc.stScanObject.bBufIdx = 0;
    
#if USBOTG_HOST_ISOC
    psUsbHost->sEhci.dwHostStructureItdBaseAddress = 0;
    psUsbHost->sEhci.dwHostDataPageBaseAddress = 0;
#endif

#if (USBOTG_WEB_CAM == ENABLE)
    psUsbHost->sEhci.dwHostStructureItdBaseAddress = 0;
    psUsbHost->sEhci.dwHostDataPageBaseAddress = 0;
    //psUsbHost->sWebcam.dwVideoStreamInterfaceNumber = 0;
    //psUsbHost->sWebcam.dwVideoStreamInterfaceAlternateSetting = 0;
    psUsbHost->sWebcam.dwVideoStreamInterfaceEndPointNumber = 0;
    //psUsbHost->sWebcam.dwVideoStreamMaxPacketSize = 0;
    psUsbHost->sWebcam.dwAudioStreamInterfaceNumber = 0;
    psUsbHost->sWebcam.dwAudioStreamInterfaceAlternateSetting = 0;
    psUsbHost->sWebcam.dwAudioStreamInterfaceEndPointNumber = 0;
    psUsbHost->sWebcam.dwAudioStreamMaxPacketSize = 0;
    psUsbHost->sWebcam.dwAudioFormatType = 0;
    psUsbHost->sWebcam.dwAudioFreqRate = 0;
    psUsbHost->sWebcam.dwAudioSampleSize = 0;
    psUsbHost->sWebcam.boVideoStreamInit = FALSE;
    //psUsbHost->sWebcam.dwMjpegFrameCnt = 0;
    //psUsbHost->sWebcam.dwMjpegLostFrameCnt = 0;
    psUsbHost->sWebcam.dwTimerTick = 0;
    psUsbHost->sWebcam.dwTimerTick1 = 0;
    psUsbHost->sWebcam.eVideoFormat = USING_MJPEG;//USING_NONE;
    psUsbHost->sWebcam.dwFormatIndex = 0;
    psUsbHost->sWebcam.dwFrameIndex = 0;
    psUsbHost->sWebcam.dwCompresionIndex =0;
    psUsbHost->sWebcam.dwVideoFrameSize = 0;
    psUsbHost->sWebcam.dwDoor = 0;
    psUsbHost->sWebcam.dwAudioDataCnt = 0;

    psUsbHost->sUvc.eNewFrame               = HAS_NO_FRAME;
    psUsbHost->sUvc.dwFrameNumber           = 0;
    psUsbHost->sUvc.dwOriginalFrameNumber   = 0;
    psUsbHost->sUvc.dwLastFrameNumber       = 0;
    psUsbHost->sUvc.bFrameID                = 0;
    psUsbHost->sUvc.bStartOfFrame           = 0;
    psUsbHost->sUvc.bNewOneFrame            = 0;
#endif
}

BYTE UsbOtgHostMailBoxIdGet (WHICH_OTG eWhichOtg)
{
    if (USBOTG0 == eWhichOtg)
    {
        return USBOTG0_HOST_CLASS_MAIL_ID;
    }
    else if (USBOTG1 == eWhichOtg)
    {
        return USBOTG1_HOST_CLASS_MAIL_ID;
    }
    else
        return 0;
}


BYTE UsbOtgHostClassTaskIdGet (WHICH_OTG eWhichOtg)
{
    #if SC_USBHOST
    if (USBOTG0 == eWhichOtg)
    {
        return USBOTG0_HOST_CLASS_TASK;
    }
    else if (USBOTG1 == eWhichOtg)
    {
        return USBOTG1_HOST_CLASS_TASK;
    }
    else
        return 0;
    #endif
}

BYTE UsbOtgHostDriverTaskIdGet (WHICH_OTG eWhichOtg)
{
    #if SC_USBHOST
    if (USBOTG0 == eWhichOtg)
    {
        return USBOTG0_HOST_DRIVER_TASK;
    }
    else if (USBOTG1 == eWhichOtg)
    {
        return USBOTG1_HOST_DRIVER_TASK;
    }
    else
        return 0;
    #endif
}

DWORD UsbOtgHostDetectErrorCodeGet (WHICH_OTG eWhichOtg)
{
    if (USBOTG0 == eWhichOtg)
    {
        return ERR_USBOTG0_HOST_DETECT_ERROR;
    }
    else if (USBOTG1 == eWhichOtg)
    {
        return ERR_USBOTG1_HOST_DETECT_ERROR;
    }
    else
        return 0;
}

BYTE UsbOtgHostDriverEventIdGet (WHICH_OTG eWhichOtg)
{
    if (USBOTG0 == eWhichOtg)
    {
        return USBOTG0_HOST_DRIVER_EVENT;
    }
    else if (USBOTG1 == eWhichOtg)
    {
        return USBOTG1_HOST_DRIVER_EVENT;
    }
    else
        return 0;
}

BYTE UsbOtgHostMsdcTxDoneEventIdGet (WHICH_OTG eWhichOtg)
{
    if (USBOTG0 == eWhichOtg)
    {
        return USBOTG0_HOST_MSDC_TRANSACTION_DONE_EVENT;
    }
    else if (USBOTG1 == eWhichOtg)
    {
        return USBOTG1_HOST_MSDC_TRANSACTION_DONE_EVENT;
    }
    else
        return 0;
}

void UsbOtgHostCurDevDescSet(WHICH_OTG eWhichOtg, BYTE bIndex)
{
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    
    if(bIndex < MAX_NUM_OF_DEVICE)
    {
        psHost->bCurUsbhDeviceDescriptorIndex = bIndex;
    }
    else
    {
        mpDebugPrint("USBOTG%d Set Current Device Descriptor Index Err : [%d]", eWhichOtg, bIndex);
    }   
}

void UsbOtgHostCurAppClassDescSet(WHICH_OTG eWhichOtg, BYTE bIndex)
{
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    
    if(bIndex < MAX_NUM_OF_DEVICE)
    {
        psHost->bCurAppClassDescriptorIndex = bIndex;
    }
    else
    {
        mpDebugPrint("USBOTG%d Set Current AppClass Descriptor Index Err : [%d]", eWhichOtg, bIndex);
    }   
}

PST_USB_OTG_HOST UsbOtgHostDsGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbHost;
}

PST_USBH_DEVICE_DESCRIPTOR UsbOtgHostDevDescGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return psUsbOtg->sUsbHost.psUsbhDeviceDescriptor + UsbOtgHostCurDevDescGet(eWhichOtg);
}

PST_APP_CLASS_DESCRIPTOR UsbOtgHostDevAppDescGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return psUsbOtg->sUsbHost.psAppClassDescriptor + UsbOtgHostCurAppClassDescGet(eWhichOtg);
}

PST_HUB_CLASS_DESCRIPTOR UsbOtgHostDevHubDescGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = UsbOtgDesGet(eWhichOtg);
    return psUsbOtg->sUsbHost.psHubClassDescriptor;
}

PUSB_HOST_EHCI UsbOtgHostEhciDsGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbHost.sEhci;
}

PUSB_HOST_CDC UsbOtgHostCdcDsGet (WHICH_OTG eWhichOtg)
{
#if (USBOTG_HOST_CDC == ENABLE)    
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbHost.sCdc;
#endif    
}

PUSB_HOST_HID UsbOtgHostHidDsGet (WHICH_OTG eWhichOtg) // Get HID global variable 
{
#if (USBOTG_HOST_HID == ENABLE)    
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbHost.sHid;
#endif    
}

PUSB_HOST_MSDC UsbOtgHostMsdcDsGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbHost.sMsdc;
}

#if (USBOTG_WEB_CAM == ENABLE)
PUSB_HOST_AVDC UsbOtgHostAvdcDsGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbHost.sWebcam;
}

PUSB_HOST_UVC UsbOtgHostUvcDsGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbHost.sUvc;
}
#endif

PUSB_HOST_SIDC UsbOtgHostSidcDsGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbHost.sSidc;
}

/*
#if NETWARE_ENABLE
PUSB_HOST_WIFI GetUsbOtgHostWifiDs (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbHost.sWifi;
}
#endif

PUSB_HOST_BT GetUsbOtgHostBtDs (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return &psUsbOtg->sUsbHost.sBt;
}
*/


DWORD UsbOtgHostConnectStatusGet(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return (DWORD)mwHost20_PORTSC_ConnectStatus_Rd();
}

void UsbOtgCmpTimeSartFlagSet (BOOL flag, WHICH_OTG eWhichOtg)
{
    PUSB_HOST_MSDC psMsdc;

    psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);
    psMsdc->boIsCmpTimerStart = flag;
}


static SDWORD UsbOtgHostFuncInit(WHICH_OTG eWhichOtg)
{
    SDWORD bRetVal = USBOTG_NO_ERROR;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);   

    //UsbOtgRegSet(eWhichOtg);
    bRetVal = UsbOtgBufferInit(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_DEBUG("-USBOTG- UsbOtgBufferInit fail!!");
        return bRetVal;
    }

    bRetVal = UsbOtgHostDmInit(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_DEBUG("-USBOTG- not enough memory in UsbOtgHostDmInit!!");
        return bRetVal;
    }

    return bRetVal;
}

static SDWORD UsbOtgHostMemoryInit(PST_USB_OTG_HOST psUsbHost, BYTE bTaskId)
{
    DWORD dwMemTmp = 0;

    psUsbHost->psUsbhDeviceDescriptor = 0;
    psUsbHost->psAppClassDescriptor = 0;
    psUsbHost->psHubClassDescriptor = 0;
    psUsbHost->psClassTaskEnvelope = 0;
    psUsbHost->psDriverTaskEnvelope = 0;
    psUsbHost->psIsrEnvelope = 0;
#if (USBOTG_WEB_CAM == ENABLE)
    psUsbHost->sWebcam.psWebCamEnvelope = 0;
#endif
#if (USBOTG_HOST_ISOC == ENABLE)
    psUsbHost->sEhci.pbHostItdManage = 0;
    psUsbHost->sEhci.pbHostDataPageManage = 0;
#endif
#if (USBOTG_HOST_CDC == ENABLE)
    psUsbHost->sCdc.psCdcEnvelope = 0;
    psUsbHost->sCdc.pbCdcVendorCmdData = 0;
    psUsbHost->sCdc.pbCdcSetLineCodingData = 0;
#endif

#if (USBOTG_HOST_HID == ENABLE)
    psUsbHost->sHid.pbHidReportData = 0;
    psUsbHost->sHid.psHidEnvelope = 0;
#endif

    psUsbHost->sSidc.psPtpEnvelope = 0;
    psUsbHost->sEhci.pbHostQtdManage = 0;


    MP_DEBUG("-USBOTG- HostTaskId is %d", bTaskId);
    dwMemTmp = (DWORD)ker_mem_malloc(MAX_NUM_OF_DEVICE * sizeof (ST_USBH_DEVICE_DESCRIPTOR), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 1");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbHost->psUsbhDeviceDescriptor = (PST_USBH_DEVICE_DESCRIPTOR) (dwMemTmp|0xA0000000);
        memset((BYTE *)psUsbHost->psUsbhDeviceDescriptor, 0, (MAX_NUM_OF_DEVICE * sizeof (ST_USBH_DEVICE_DESCRIPTOR)));
    }

    dwMemTmp = (DWORD)ker_mem_malloc(MAX_NUM_OF_DEVICE * sizeof (ST_APP_CLASS_DESCRIPTOR), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 2");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbHost->psAppClassDescriptor = (PST_APP_CLASS_DESCRIPTOR) (dwMemTmp|0xA0000000);
        memset((BYTE *)psUsbHost->psAppClassDescriptor, 0, (MAX_NUM_OF_DEVICE * sizeof (ST_APP_CLASS_DESCRIPTOR)));
    }

    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (ST_HUB_CLASS_DESCRIPTOR), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 3");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbHost->psHubClassDescriptor = (PST_HUB_CLASS_DESCRIPTOR) (dwMemTmp|0xA0000000);
        memset((BYTE *)psUsbHost->psHubClassDescriptor, 0, sizeof (ST_HUB_CLASS_DESCRIPTOR));
    }


    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (ST_MCARD_MAIL), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 4");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbHost->psClassTaskEnvelope = (ST_MCARD_MAIL*) (dwMemTmp|0xA0000000);
    }

    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (ST_MCARD_MAIL), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 5");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbHost->psDriverTaskEnvelope = (ST_MCARD_MAIL*) (dwMemTmp|0xA0000000);
    }

    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (ST_MCARD_MAIL), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 6");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbHost->psIsrEnvelope = (ST_MCARD_MAIL*) (dwMemTmp|0xA0000000);
    }


    //Webcam
#if (USBOTG_WEB_CAM == ENABLE)
    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (ST_MCARD_MAIL), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 7");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbHost->sWebcam.psWebCamEnvelope = (ST_MCARD_MAIL*) (dwMemTmp|0xA0000000);
    }
#endif
#if USBOTG_HOST_ISOC
    psUsbHost->sEhci.pbHostItdManage = (BYTE*)ker_mem_malloc(Host20_iTD_MAX, bTaskId);
    if (psUsbHost->sEhci.pbHostItdManage == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 8");
        return USBOTG_NO_MEMORY;
    }

    psUsbHost->sEhci.pbHostDataPageManage = (BYTE*)ker_mem_malloc(Host20_Page_MAX, bTaskId);
    if (psUsbHost->sEhci.pbHostDataPageManage == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 9");
        return USBOTG_NO_MEMORY;
    }
#endif
    //Cdc
#if (USBOTG_HOST_CDC == ENABLE)
    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (ST_MCARD_MAIL), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 10");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbHost->sCdc.psCdcEnvelope = (ST_MCARD_MAIL*) (dwMemTmp|0xA0000000);
    }

#endif
    //Ptp
    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (USBH_PTP_MAIL_TAG), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 11");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbHost->sSidc.psPtpEnvelope = (PUSBH_PTP_MAIL_TAG) (dwMemTmp|0xA0000000);
    }


    //Ehci
    psUsbHost->sEhci.pbHostQtdManage = (BYTE*)ker_mem_malloc(Host20_qTD_MAX, bTaskId);
    if (psUsbHost->sEhci.pbHostQtdManage == 0)
    {
        MP_DEBUG("-USBOTG- Host not enough memory 12");
        return USBOTG_NO_MEMORY;
    }

#if (USBOTG_HOST_CDC == ENABLE)
    //Cdc
    psUsbHost->sCdc.pbCdcVendorCmdData = (BYTE*)ker_mem_malloc(0x10, bTaskId);
    if (psUsbHost->sCdc.pbCdcVendorCmdData == 0)
    {
        MP_DEBUG("-USBOTG- Host not enough memory 13");
        return USBOTG_NO_MEMORY;
    }

    psUsbHost->sCdc.pbCdcSetLineCodingData = (BYTE*)ker_mem_malloc(7, bTaskId);
    if (psUsbHost->sCdc.pbCdcSetLineCodingData == 0)
    {
        MP_DEBUG("-USBOTG- Host not enough memory 14");
        return USBOTG_NO_MEMORY;
    }
#endif
    //Hid
#if (USBOTG_HOST_HID == ENABLE)
    dwMemTmp = (DWORD)ker_mem_malloc(sizeof (ST_MCARD_MAIL), bTaskId);
    if (dwMemTmp == 0)
    {
        MP_ALERT("-USBOTG- Host not enough memory 15");
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbHost->sHid.psHidEnvelope = (ST_MCARD_MAIL*) (dwMemTmp|0xA0000000);
    }
#endif

    //Msdc
    //Avdc
    return USBOTG_NO_ERROR;

}

static void UsbOtgHostMemoryFinish(PST_USB_OTG_HOST psUsbHost)
{
    if (psUsbHost->psUsbhDeviceDescriptor != 0)
        ker_mem_free (psUsbHost->psUsbhDeviceDescriptor);    
    if (psUsbHost->psAppClassDescriptor != 0)
        ker_mem_free (psUsbHost->psAppClassDescriptor);    
    if (psUsbHost->psHubClassDescriptor != 0)
        ker_mem_free (psUsbHost->psHubClassDescriptor);    
    if (psUsbHost->psClassTaskEnvelope != 0)
        ker_mem_free (psUsbHost->psClassTaskEnvelope);    
    if (psUsbHost->psDriverTaskEnvelope != 0)
        ker_mem_free (psUsbHost->psDriverTaskEnvelope);    
    if (psUsbHost->psIsrEnvelope != 0)
        ker_mem_free (psUsbHost->psIsrEnvelope);    
    //Webcam
#if (USBOTG_WEB_CAM == ENABLE)
    if (psUsbHost->sWebcam.psWebCamEnvelope != 0)
        ker_mem_free (psUsbHost->sWebcam.psWebCamEnvelope);    
#endif
    //Cdc
#if (USBOTG_HOST_CDC == ENABLE)
    if (psUsbHost->sCdc.psCdcEnvelope != 0)
        ker_mem_free (psUsbHost->sCdc.psCdcEnvelope);   
    if (psUsbHost->sCdc.pbCdcVendorCmdData != 0)
        ker_mem_free (psUsbHost->sCdc.pbCdcVendorCmdData);    
    if (psUsbHost->sCdc.pbCdcSetLineCodingData != 0)
        ker_mem_free (psUsbHost->sCdc.pbCdcSetLineCodingData);     
#endif
    //Hid
#if (USBOTG_HOST_HID == ENABLE)
    if (psUsbHost->sHid.psHidEnvelope != 0)
        ker_mem_free (psUsbHost->sHid.psHidEnvelope);   
    if (psUsbHost->sHid.pbHidReportData != 0)
        ker_mem_free (psUsbHost->sHid.pbHidReportData);
#endif
    //Ptp
    if (psUsbHost->sSidc.psPtpEnvelope != 0)
        ker_mem_free (psUsbHost->sSidc.psPtpEnvelope);    
    //Ehci
    if (psUsbHost->sEhci.pbHostQtdManage != 0)
        ker_mem_free (psUsbHost->sEhci.pbHostQtdManage);    
#if (USBOTG_HOST_ISOC == ENABLE)
    if (psUsbHost->sEhci.pbHostItdManage != 0)
        ker_mem_free (psUsbHost->sEhci.pbHostItdManage);    
    if (psUsbHost->sEhci.pbHostDataPageManage != 0)
        ker_mem_free (psUsbHost->sEhci.pbHostDataPageManage);    
#endif
   
    //Msdc
    //Avdc
}

static SDWORD UsbOtgHostDmInit(WHICH_OTG eWhichOtg)
{
    SDWORD bRetVal = USBOTG_NO_ERROR;
    PST_USB_OTG_DES psUsbOtg;
    BYTE bTaskId;

    bTaskId = UsbOtgHostDriverTaskIdGet(eWhichOtg);
    MP_DEBUG("-USBOTG %d- HostTaskId is %d", eWhichOtg, bTaskId);
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    bRetVal = UsbOtgHostMemoryInit(&psUsbOtg->sUsbHost, bTaskId);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG %d- not enough memory in UsbOtgHostMemoryInit with %d!!", eWhichOtg, bTaskId);
        return bRetVal;
    }
    UsbOtgHostDsInit(&psUsbOtg->sUsbHost);
    
    return bRetVal;
}



static void UsbOtgHostCheckIfSetPlugoutEvent(WHICH_OTG eWhichOtg)
{
    #if SC_USBHOST
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes;
    psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    if (psUsbhDevDes->bConnectStatus == USB_STATE_ATTACHED)
    {
        BYTE bDriverEventId = 0;
        
        MP_ALERT("-usbotg%d- %s event set EVENT_DEVICE_PLUG_OUT", eWhichOtg, __FUNCTION__);
        bDriverEventId = UsbOtgHostDriverEventIdGet(eWhichOtg);
        flib_Host20_StopRun_Setting(HOST20_Disable, eWhichOtg);
        psUsbhDevDes->bConnectStatus=USB_STATUS_DISCONNECT;
        EventSet(bDriverEventId, EVENT_DEVICE_PLUG_OUT);
    }
    #endif
}

static BYTE UsbOtgHostCurDevDescGet(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    
    return psHost->bCurUsbhDeviceDescriptorIndex;
}



static BYTE UsbOtgHostCurAppClassDescGet(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    
    return psHost->bCurAppClassDescriptorIndex;
}


#endif

// Controller

//============================================================================= ok
//		UsbOtgControllerInit()
//		Description:1.Init the OTG Structure Variable
//			        2.Init the Interrupt register(OTG-Controller layer)
//                  3.Call the UsbOtgRoleChange function to init the Host/Peripheral
//		input: none
//		output: none
//=============================================================================
void UsbOtgControllerInit(WHICH_OTG eWhichOtg)
{
   DWORD dwTemp;
   PST_USB_OTG_DES psUsbOtg;

   psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
   //Clear the interrupt status
   dwTemp=mdwOTGC_INT_STS_Rd();
   mdwOTGC_INT_STS_Clr(dwTemp);

   //<1>.Read the ID 
   if (mdwOTGC_Control_ID_Rd()>0) {//Change to B Type
		mpDebugPrint("-usbotg%d- Change to B Type ID %x", eWhichOtg, mdwOTGC_Control_ID_Rd());
      //<1.2>.Init Interrupt
      mdwOTGC_INT_Enable_Clr(OTGC_INT_A_TYPE);      
      mdwOTGC_INT_Enable_Set(OTGC_INT_B_TYPE);         	   
      
	  //<1.3>.Init the Host/Peripheral
      UsbOtgRoleChange(1, psUsbOtg);
   }
   else {//Changfe to A Type
		mpDebugPrint("-usbotg%d- Change to A Type ID %x", eWhichOtg, mdwOTGC_Control_ID_Rd());
      //<2.2>. Init Interrupt
      mdwOTGC_INT_Enable_Clr(OTGC_INT_B_TYPE);
      mdwOTGC_INT_Enable_Set(OTGC_INT_A_TYPE);                 

      //<2.3>.Init the Host/Peripheral
      UsbOtgRoleChange(0, psUsbOtg);
   }      
}

DWORD UsbOtgIntCauseGet (WHICH_OTG eWhichOtg)
{
    if (USBOTG0 == eWhichOtg)
    {
        return IM_USB0;
    }
    else if (USBOTG1 == eWhichOtg)
    {
        return IM_USB1;
    }
    else
        return 0;
}

PST_USB_OTG_DES UsbOtgDesGet (WHICH_OTG eWhichOtg)
{
    return (PST_USB_OTG_DES)&g_stUsb[eWhichOtg];
}

DWORD UsbOtgBufferGet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;
    
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    return psUsbOtg->dwUsbBuf;
}

SDWORD UsbOtgCheckError(WHICH_OTG eWhichOtg)
{
    SDWORD bRetVal = USBOTG_NO_ERROR;
    
    if (eWhichOtg == USBOTG_NONE)
    {
        bRetVal = USBOTG_NOT_INIT_YET;
    }
    else if (eWhichOtg != USBOTG0 && eWhichOtg != USBOTG1)
    {
        bRetVal = USBOTG_PARAMETER_ERROR;
    }

    return bRetVal;
}

SDWORD UsbOtgSetSysTimerProc (DWORD dwWaitTimeCnt, void *pFunc, BOOL isOneShot, WHICH_OTG eWhichOtg)
{
    SDWORD sdwId = 0;
    SDWORD sdRet = 0;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    psUsbOtg->bUsbOtgArg[1] = eWhichOtg;
    sdwId = SysTimerProcReAdd(dwWaitTimeCnt, pFunc, isOneShot, TRUE);
    if (sdwId < NO_ERR)
        return sdwId;
    psUsbOtg->timerId = sdwId;
    sdRet = SysTimerProcArgSetById(sdwId, (void *) &psUsbOtg->bUsbOtgArg[0]);
    if (sdRet != NO_ERR)
        return sdRet;

    return sdRet;
}

void UsbOtgBiuReset(WHICH_OTG eWhichOtg)
{
    if (eWhichOtg == USBOTG0)
    {
        g_psBiu->BiuArst &= 0xFFFFFF7F;
        IODelay(1);
        g_psBiu->BiuArst |= 0x00000080;
    }
    else if (eWhichOtg == USBOTG1)
    {
        g_psBiu->BiuArst &= 0xFFFFFFFD;
        IODelay(1);
        g_psBiu->BiuArst |= 0x00000002;
    }

    IODelay(1);
}

static void UsbOtgClkInit(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    CLOCK *regClkPtr = (CLOCK *) CLOCK_BASE;
    
#if (USBOTG_CLK_FROM_GPIO == ENABLE) // USB clock source from GPIO
    #if ((CHIP_VER_MSB == CHIP_VER_650) && (CHIP_VER_LSB != CHIP_VER_A) && (CHIP_VER_LSB != CHIP_VER_B))
    Gpio_ConfiguraionSet(GPIO_GPIO_4, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, 0x00, 1);
    regClkPtr->Clkss_EXT2 |= BIT10;     // USB get clock from GPIO-4
    #elif ((CHIP_VER_MSB == CHIP_VER_660) && (CHIP_VER_LSB != CHIP_VER_A))
    Gpio_ConfiguraionSet(GPIO_GPIO_3, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, 0x00, 1);
    regClkPtr->Clkss_EXT2 |= BIT10;     // USB get clock from GPIO-3
    #endif
#endif

    // DMA Enable
    if(eWhichOtg == USBOTG0)
        *((volatile DWORD *)(0xa800c180)) |= BIT0;
    else if(eWhichOtg == USBOTG1)
        *((volatile DWORD *)(0xa800c160)) |= BIT0;
    else
        MP_DEBUG("%s USBOTG%d Error", __FUNCTION__, eWhichOtg);

    // USBOTG PHY Reset for ChipIdea
    psUsbOtg->psUsbReg->PhyUtmiSwCtrl |= BIT8;
    IODelay(100); // it's necessary to delay. can be fine tune.
    psUsbOtg->psUsbReg->PhyUtmiSwCtrl = 0;
    IODelay(100); // it's necessary to delay. can be fine tune.

    // Mark these setup for HW PCB Layout
    //psUsbOtg->psUsbReg->OtgControlStatus |= BIT13; // idpullup =1 
    //IODelay(100); // it's necessary to delay. can be fine tune.

    // SW control avalid, bvalid, vbusvalid 
    #if SC_USBHOST
    psUsbOtg->psUsbReg->PhyUtmiSwCtrl = BIT1|BIT2|BIT3|BIT17|BIT18|BIT19;  // vbusvalid for Host
    #else
    psUsbOtg->psUsbReg->PhyUtmiSwCtrl = BIT1|BIT2|BIT17|BIT18;  // For Device Only (MP663)
    #endif
    
    // Disable MDev_Wakeup_byVBUS & Dev_Idle interrupt for SW control avalid, bvalid, vbusvalid
    psUsbOtg->psUsbReg->DeviceMaskofInterruptSrcGroup2 = BIT9|BIT10;

    // USB DEVICE FIFO Loop Comparsion Test
    #if 0
    if(eWhichOtg == USBOTG1) // The DPF need to connect with PC
        vUsbCxLoopBackTest(eWhichOtg);
    #endif
}

static SDWORD UsbOtgFuncInit(WHICH_OTG eWhichOtg)
{
    SDWORD bRetVal = USBOTG_NO_ERROR;
    PST_USB_OTG_DES psUsbOtg;
    
    bRetVal = UsbOtgCheckError(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_DEBUG("-USBOTG- UsbOtgCheckError: %d", bRetVal);
        return bRetVal;
    }
    
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    //UsbOtgRegSet(eWhichOtg);
    bRetVal = UsbOtgDmInit(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_DEBUG("-USBOTG- error in UsbOtgDmInit %d!!", bRetVal);
        return bRetVal;
    }

    bRetVal = UsbOtgControlInit(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_DEBUG("-USBOTG- error in UsbOtgControlInit %d!!", bRetVal);
        return bRetVal;
    }
    
    return bRetVal;
}


static void UsbOtgFinish(void)
{
    PST_USB_OTG_DES psUsbOtg;

    UsbOtgMemoryFinish(USBOTG0);
    UsbOtgMemoryFinish(USBOTG1);
}

static void UsbOtgRegSet (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;
    
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    if (eWhichOtg == USBOTG0)
    {
        psUsbOtg->psUsbReg = (PUSB_OTG) USBOTG0_BASE;
    }
    else if (eWhichOtg == USBOTG1)
    {
        psUsbOtg->psUsbReg = (PUSB_OTG) USBOTG1_BASE;
    }
    
    MP_DEBUG("-USBOTG%d- %s psUsbOtg = 0x%x; psUsbOtg->psUsbReg = 0x%x", eWhichOtg, __FUNCTION__, psUsbOtg, psUsbOtg->psUsbReg);
}



static DWORD UsbOtgBufferIdGet (WHICH_OTG eWhichOtg)
{
    if (USBOTG0 == eWhichOtg)
    {
        return USB_OTG_BUF_MEM_ID;
    }
    else if (USBOTG1 == eWhichOtg)
    {
        return USB_OTG1_BUF_MEM_ID;
    }
    else
        return 0;
}

static SDWORD UsbOtgDmInit(WHICH_OTG eWhichOtg)
{
    SDWORD bRetVal = USBOTG_NO_ERROR;

    bRetVal = UsbOtgBufferInit(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- UsbOtgBufferInit fail!!", eWhichOtg);
        return bRetVal;
    }

#if SC_USBDEVICE
    bRetVal = UsbOtgDeviceDmInit(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- UsbOtgDeviceDmInit fail!!", eWhichOtg);
        return bRetVal;
    }
#endif

#if SC_USBHOST
    bRetVal = UsbOtgHostDmInit(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- UsbOtgHostDmInit fail!!", eWhichOtg);
        return bRetVal;
    }
#endif

    return bRetVal;
}


static void UsbOtgMemoryFinish(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;
    
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

#if SC_USBDEVICE	
    UsbOtgDeviceMemoryFinish(&psUsbOtg->sUsbDev);
#endif
#if SC_USBHOST
    UsbOtgHostMemoryFinish(&psUsbOtg->sUsbHost);
#endif
    if (psUsbOtg->dwUsbBuf != 0)
        ker_mem_free (psUsbOtg->dwUsbBuf);    
    
}

static void UsbOtgIsr (WHICH_OTG eWhichOtg)
{
    DWORD  dwIntStatus;
    PST_USB_OTG_DES psUsbOtg;
    USB_OTG_FUNC    eFunction;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);    
    eFunction = UsbOtgFunctionGet(eWhichOtg);

    dwIntStatus = mwOTG20_GlobalHcOtgDev_Interrupt_Status_Rd(); //get Global Controller Register interupt status

    if( dwIntStatus & OTG_INT )
    {
        //UartOutText("<O>");
        UsbOtgControlIsr(eWhichOtg);
    }

#if (SC_USBDEVICE)
    if (dwIntStatus & DEV_INT)
    {
        //UartOutText("<D>");
        if (eFunction == HOST_ONLY_FUNC)
            return;
        else
            UsbOtgDeviceIsr(eWhichOtg);
    }
#endif    

#if (SC_USBHOST)
    if ( dwIntStatus & HC_INT )
    {
        //UartOutText("<H>");
        if (eFunction == DEVICE_ONLY_FUNC)
            return;
        else
            UsbOtgHostIsr(eWhichOtg);
    }
#endif

}

#if USBCAM_DEBUG_ISR_LOST
static DWORD st_dwOtgIsr=0;
#endif
static void UsbOtg0Isr (void)
{
    //UartOutText("<I>");
    SystemIntDis(IM_USB0);               //disable system USBOTG interrupt 
#if USBCAM_DEBUG_ISR_LOST
    st_dwOtgIsr++;
	if ((st_dwOtgIsr%1000)==0)
		mpDebugPrint("OtgIsr %d",st_dwOtgIsr);
#endif
    UsbOtgIsr(USBOTG0);
    SystemIntEna(IM_USB0);
}

static void UsbOtg1Isr (void)
{
    //UartOutText("<i>");
    SystemIntDis(IM_USB1);               //disable system USBOTG interrupt 
    UsbOtgIsr(USBOTG1);
    SystemIntEna(IM_USB1);
}

static void UsbOtgControlIsr (WHICH_OTG eWhichOtg)
{
#if SC_USBDEVICE        
    DWORD dwId = 0;
    DWORD dwOtgIntSts = 0;
    DWORD dwOtgIntSts2 = 0;
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);    
    dwOtgIntSts = mdwOTGC_INT_STS_Rd();       //get and clear OTGC  interrupt
    MP_DEBUG("-USBOTG%d- 1 OTGC_INT_STS = 0x%x", eWhichOtg, dwOtgIntSts);
    mdwOTGC_INT_STS_Clr(dwOtgIntSts);
    dwOtgIntSts2 = mdwOTGC_INT_STS_Rd();       //get and clear OTGC  interrupt
    MP_DEBUG("-USBOTG%d- 2 OTGC_INT_STS2 = 0x%x", eWhichOtg, dwOtgIntSts2);
    if( dwOtgIntSts & OTGC_INT_IDCHG )
    {
        MP_DEBUG("-usbotg%d- OTGC_INT_IDCHG", eWhichOtg);
        UsbOtgControllerInit(eWhichOtg); // TEST
        dwId = mdwOTGC_Control_ID_Rd();

		psUsbOtg->psUsbReg->PhyTestModeSelector &= 0xfffffffe;
		psUsbOtg->psUsbReg->DeviceMainControl |= BIT2;
#if (SC_USBDEVICE && WEB_CAM_DEVICE)
		Api_UsbdSetMode(USB_AP_UAVC_MODE, eWhichOtg);
#else
		//Api_UsbdSetMode(USB_AP_UAVC_MODE, eWhichOtg);
#endif
		//xpgCb_USB_mode_setting();
        if( dwId>0) // dwId == 1 => the recpetor is B-device (device)
        { // connect to PC
            MP_DEBUG("-USBOTG%d- connect to PC", eWhichOtg);
            #if (SC_USBHOST==ENABLE)
            UsbOtgHostCheckIfSetPlugoutEvent(eWhichOtg);
            #endif //SC_USBHOST
        }
        else      // dwId == 0 => the receptor is A-device ( host)
        { // disconnect     
            MP_DEBUG("-USBOTG%d- disconnect to PC", eWhichOtg);
            #if (SC_USBHOST==ENABLE)
            MP_DEBUG("ready for HOST");
            InitiaizeUsbOtgHost(0, eWhichOtg);  // Init Usb Otg Memory (qHD)
            #endif //SC_USBHOST
        }
    }
    else
    {
        MP_DEBUG("-USBOTG%d- dwOtgIntSts = 0x%x   NOT IDCHG INTERRUPT ", eWhichOtg, dwOtgIntSts);
        if ((dwOtgIntSts & OTGC_INT_AVBUSERR) || (dwOtgIntSts & OTGC_INT_OVC))
        {
            UartOutText("X");
        }
    }
#else
    return;
#endif //SC_USBDEVICE
}



//#define mwHost20_USBINTR_Set(bValue)	(psUsbOtg->psUsbReg->HcUsbInterruptEnable=bValue)//(gp_UsbOtg->HcUsbInterruptEnable=bValue)
//#define mwHost20_USBSTS_Rd()		    (psUsbOtg->psUsbReg->HcUsbStatus)//(gp_UsbOtg->HcUsbStatus) 	
//#define mwHost20_USBSTS_Set(wValue)		(psUsbOtg->psUsbReg->HcUsbStatus=wValue)//(gp_UsbOtg->HcUsbStatus=wValue) 
//=============================================================================ok
//		UsbOtgRoleChange()
//		Description:This function will take care the event about role change.
//                  It will close/init some function.
//			
//		input:(INT8U)bRole
//             0 => Change to Host
//             1 => Change to Peripheral
//		output: none
//=============================================================================
static void UsbOtgRoleChange(WORD bRole, PST_USB_OTG_DES psUsbOtg)
{
   BYTE wTemp;
   if (bRole==0) {//Change to Host
      //Bruce;;12162004;;unplug issue;;//mUsbUnPLGClr();
	mpDebugPrint("Change to Host");
      if (mdwOTGC_Control_ID_Rd()==0) {//Device-A: change to Host
         mdwOTGC_Control_A_BUS_DROP_Clr(); //Exit when Current Role = Host
         mdwOTGC_Control_A_BUS_REQ_Set();
	  }
      else {//Device-B: Change to Host
         mdwOTGC_Control_B_HNP_EN_Clr();
         mdwOTGC_Control_B_DSCHG_VBUS_Clr();       
	  } 
      
      #if SC_USBDEVICE
   	  UsbOtgDeviceClose(psUsbOtg);          	 	    	
      #endif
   }
   else {//Change to Peripheral
   	mpDebugPrint("Change to Peripheral");

      //Bruce;;12162004;;unplug issue;;//mUsbUnPLGClr();
      if (mdwOTGC_Control_ID_Rd()==0) {//Device-A: change to Peripheral        	
       	 mdwOTGC_Control_A_SET_B_HNP_EN_Clr();       		
      }
      else {//Device-B: Change to Peripheral
       	 mdwOTGC_Control_B_BUS_REQ_Clr();
      }

      //flib_Host20_Close(0);          	 
      mwHost20_USBINTR_Set(0);
      wTemp=mwHost20_USBSTS_Rd();
      wTemp=wTemp&0x0000003F;
      mwHost20_USBSTS_Set(wTemp);
   }
}	

static USB_OTG_FUNC UsbOtgFunctionGet(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;
    
    psUsbOtg = UsbOtgDesGet(eWhichOtg);
    return psUsbOtg->eFunction;
}

static void UsbOtgDsClr(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;
    
    psUsbOtg = UsbOtgDesGet(eWhichOtg);
    psUsbOtg->psUsbReg = NULL;
    psUsbOtg->dwUsbBuf = 0;
    psUsbOtg->bUsbOtgArg[0] = 1; // one arg
    psUsbOtg->bUsbOtgArg[1] = 0;
    psUsbOtg->bUsbOtgArg[2] = 0;
    psUsbOtg->bUsbOtgArg[3] = 0;
    psUsbOtg->eFunction = NONE_FUNC;
}

#define mUsbTstHalfSpeedEn()		(psUsbOtg->psUsbReg->DeviceMainControl |= BIT1)
static SDWORD UsbOtgControlInit(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;
    SDWORD bRetVal = USBOTG_NO_ERROR;

    bRetVal = UsbOtgCheckError(eWhichOtg);
    if (bRetVal != USBOTG_NO_ERROR)
    {
        MP_DEBUG("-USBOTG- UsbOtgCheckError: %d", bRetVal);
        return bRetVal;
    }
    
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    MP_DEBUG("-USBOTG%d- %s psUsbOtg = 0x%x; psUsbOtg->psUsbReg = 0x%x", eWhichOtg, __FUNCTION__, psUsbOtg, psUsbOtg->psUsbReg);

    //psUsbOtg->dwUsbBuf = SystemGetMemAddr(USB_OTG_BUF_MEM_ID)|((DWORD)0xa0000000);
    //memset((BYTE *)psUsbOtg->dwUsbBuf, 0, USB_OTG_BUF_SIZE);
    //mpDebugPrint("g_USBBUF %x", psUsbOtg->dwUsbBuf);

	// mUsbTstHalfSpeedEn(); // Only for FPGA Board

    mdwOTGC_Control_Internal_Test_Clr();
    mdwOTGC_Control_Internal_Test();
    
    MP_DEBUG("-USBOTG%d- ID = %d", eWhichOtg, mdwOTGC_Control_ID_Rd());

    // Regist Interrupt Callback Function
    if(eWhichOtg == USBOTG0)
        SystemIntHandleRegister(UsbOtgIntCauseGet(eWhichOtg), UsbOtg0Isr);
    else if(eWhichOtg == USBOTG1)
        SystemIntHandleRegister(UsbOtgIntCauseGet(eWhichOtg), UsbOtg1Isr);
    else
        MP_DEBUG("-USBOTG%d- Regist INT Error", eWhichOtg);

    //if (mdwOTGC_Control_ID_Rd()) 
    {// Device
        MP_DEBUG("-USBOTG%d- <Device>", eWhichOtg);
        #if (SC_USBDEVICE)
        if (psUsbOtg->eFunction != HOST_ONLY_FUNC)
        {
            bRetVal = UsbOtgDeviceTaskInit(eWhichOtg);
            if (bRetVal != USBOTG_NO_ERROR)
            {
                MP_DEBUG("-USBOTG%d- UsbOtgDeviceTaskInit: %d", eWhichOtg, bRetVal);
                return bRetVal;
            }
        }
        else
        {
            MP_DEBUG("-USBOTG%d- it's host only so skip UsbOtgDeviceTaskInit!!", eWhichOtg);
        }
        #endif 
    }     
    //else 
    {// Host
        MP_DEBUG("-USBOTG%d- <Host>", eWhichOtg);
        #if (SC_USBHOST)
        if (psUsbOtg->eFunction != DEVICE_ONLY_FUNC)
        {
            bRetVal = UsbOtgHostTaskInit(eWhichOtg);
            if (bRetVal != USBOTG_NO_ERROR)
            {
                MP_DEBUG("-USBOTG%d- UsbOtgHostTaskInit: %d", eWhichOtg, bRetVal);
                return bRetVal;
            }
        }
        else
        {
            MP_DEBUG("-USBOTG%d- it's device only so skip UsbOtgHostTaskInit!!", eWhichOtg);
        }
        #endif 
    }
}

static SDWORD UsbOtgBufferInit(WHICH_OTG eWhichOtg)
{
    SDWORD bRetVal = USBOTG_NO_ERROR;
    PST_USB_OTG_DES psUsbOtg;
//    BYTE bTaskId;

//    bTaskId = UsbOtgHostDriverTaskIdGet(eWhichOtg);
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    psUsbOtg->dwUsbBuf = SystemGetMemAddr(UsbOtgBufferIdGet(eWhichOtg))|((DWORD)0xa0000000);
    if ((psUsbOtg->dwUsbBuf & 0x1F) != 0)
    {
        MP_ALERT("-USBOTG%d- USB Buffer is necessary to  alian 0x20 bundary!!", eWhichOtg);
    }
    else
    {
        memset((BYTE *)psUsbOtg->dwUsbBuf, 0, USB_OTG_BUF_SIZE);
        MP_DEBUG("-USBOTG%d- %s dwUsbBuf 0x%x", eWhichOtg, __FUNCTION__, psUsbOtg->dwUsbBuf);
    }
/*
    DWORD dwMemTmp = 0;
    dwMemTmp = (DWORD)ker_mem_malloc(USB_OTG_BUF_SIZE, bTaskId);
    if (dwMemTmp == 0)
    {
        MP_DEBUG("-USBOTG- not enough memory to get USB OTG buffer with %d!!", bTaskId);
        return USBOTG_NO_MEMORY;
    }
    else
    {
        psUsbOtg->dwUsbBuf = (DWORD) (dwMemTmp|0xA0000000);
        memset((BYTE *)psUsbOtg->dwUsbBuf, 0, USB_OTG_BUF_SIZE);
        MP_DEBUG("dwUsbBuf %x", psUsbOtg->dwUsbBuf);
    }
*/    
    return bRetVal;
}

#endif // (SC_USBHOST|SC_USBDEVICE)

