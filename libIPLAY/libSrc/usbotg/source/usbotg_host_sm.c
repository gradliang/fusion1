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
* Filename      : usbotg_host_sm.c
* Programmer(s) : Joe Luo
* Created		: 2008/04/23 
* Description	: 
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

#include "usbotg_ctrl.h"
#include "usbotg_host_sidc.h"
#include "usbotg_host_setup.h"
#include "usbotg_host_sm.h"
#include "usbotg_host_msdc.h"
#include "usbotg_host_hub.h"
#if USBOTG_HOST_CDC
#include "usbotg_host_cdc.h"
#endif

#if USBOTG_HOST_HID
#include "usbotg_host_hid.h"
#endif

#include "taskid.h"
#include "os.h"
#include "devio.h"
#include "ui.h"
#include "display.h"
//rick BT
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
//#include "BlueTooth.h"
#include "usbotg_bt.h"
#endif
#if (NETWARE_ENABLE == ENABLE)
#include	"usbotg_ethernet.h"
#include "usbotg_wifi.h"
#include "usbotg_ethernet.h"
#include "net_const.h"
extern BYTE wifi_device_type;
#endif    
#if USBOTG_WEB_CAM
#include "usbotg_host_web_cam.h"
#endif

#if USB_HOST_ISO_TEST
extern void webCamStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
#endif

#if (SC_USBHOST==ENABLE)
/*
// Constant declarations
*/
//#define USBOTG_HOST_INTERRUPT_TX_TEST 0    
#define SFF_8070i           5
#define SCSI_TRANS_CMD_SET  6
#define USBOTG_HOST_STATE_CHANGE_WAIT_TIME  600         // 600 ms
#define USBOTG_HOST_PORT_RESET_TIME         55          // 55  ms
#define USBOTG_HOST_DEVICE_INIT_TIME        100         // 100 ms
#define USBOTG_HOST_DEVICE_INIT_TIME2       100         // 100 ms

#define USBOTG_HOST_FULL_SPEED_DEVICE_POLLING  1200    // 1.2 sec

//#define VENOR_ID_PROLIFIC_INC 0x067B
//#define FLASH_DISK_EMBEDDED_HUB 0x2515

//#define GET_OBJECT_IN_THE_ROOT		        0xFFFFFFFF
#define MIN_CAPACITY_IN_SECTOR		        0x00002000//0x00008000

#define USBOTG_HOST_WAIT_FOR_PORT_RESET     55
#define USBOTG_HOST_WAIT_FOR_DEVICE_READY   20

/*
// Structure declarations
*/

/*
// Variable declarations
*/
//static BOOL gSupportMultiLun = FALSE;
// static BOOL gIsMsdcBulkOnly = FALSE;  // Delete
//static BYTE gUsbhDeviceAddress = 0;
//static BOOL gNoNeedToPollingUsbDevice = FALSE;//Joe 3/19: USB Hub
//#if USBOTG_HOST_USBIF   // Delete
//BYTE gUsbIfTestSts = USB_IF_TEST_DISABLE; // USB_IF_TEST_DISABLE;
//#endif  //USBOTG_HOST_USBIF
/*
// Extern Variable declarations
*/


/*
// Macro declarations
*/


/*
// Static function prototype
*/
static void UsbOtgHostStateChange(void *arg);
static void UsbOtgHostPortResetDone(void *arg);
static void UsbOtgHostDeviceInitDone(void *arg);
static void UsbOtgHostDeviceInitDone2(void *arg);
static WORD NextMcardIdCmp (WORD wCurId, BYTE bLun, BYTE bMaxLun, WHICH_OTG eWhichOtg);
static void SetupStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);

#if USBOTG_HOST_INTERRUPT_TX_TEST    
static void UsbOtgHostBulkOnlyInterruptTestActive(WHICH_OTG eWhichOtg);
static void UsbOtgHostBulkOnlyInterruptTestIoc(WHICH_OTG eWhichOtg);
#endif    

static void UsbOtg0HostMsdcInitDone(void);
static void UsbOtg1HostMsdcInitDone(void);
static void MsdcInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
static void MsdcCheckMediaPresent(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
static void SetupHubStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);

#if USBOTG_HOST_PTP
static void PtpFillStorageInfo (PSTI_STORAGE_INFO pStoregeInfo, BYTE *pBuf);
static void UsbOtg_0_SidcInitDone (void);
static void UsbOtg_1_SidcInitDone (void);
static void SidcInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
static void SidcScanObjectsStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
static void GetSearchInfoFromObjectInfo ( DWORD dwExtension,
                                          DWORD *pdwExtArray,
                                          DWORD *pdwCount,
                                          DWORD dwObjectHandle,
                                          DWORD dwSidcDataBuffer,
                                          PUSBH_PTP_MAIL_TAG pReceiveMailFmFs );
#endif //#if USBOTG_HOST_PTP

#if USBOTG_HOST_CDC
static void CdcInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
#endif  // USBOTG_HOST_CDC
#if USBOTG_HOST_HID
static void HidInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
#endif  // USBOTG_HOST_HID
#if USBOTG_HOST_USBIF
static void DisplayUsbIfTestMode(BYTE* bMsg);
static BOOL SetUsbIfTestMode(BYTE bMode, WHICH_OTG eWhichOtg);
static BYTE GetUsbIfTestMode(WHICH_OTG eWhichOtg);
#endif  //USBOTG_HOST_USBIF

#if USB_ARCH_URB
void H21_BulkActive(WHICH_OTG eWhichOtg);
void H21_BulkIoc(WHICH_OTG eWhichOtg);
void UsbWifiInterruptActive(WHICH_OTG eWhichOtg);
void UsbWifiInterruptIoc(WHICH_OTG eWhichOtg);
#endif

#if USBOTG_HOST_DATANG
static void DatangInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
#endif
/*
// Definition of internal functions
*/
pSmFunc GetSmFunctionPointer(WORD wStateMachine)
{
    if (wStateMachine == SETUP_SM)                        return &SetupStateMachine;
    if (wStateMachine == MSDC_INIT_SM)                    return &MsdcInitStateMachine;
    if (wStateMachine == MSDC_CHECK_MEDIA_PRESENT_SM)     return &MsdcCheckMediaPresent;
//rick BT
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
    if (wStateMachine == BT_INIT_SM)                      return &BTInitStateMachine;
#endif    
    if (wStateMachine == SETUP_HUB_SM)                    return &SetupHubStateMachine;
#if USB_WIFI_ENABLE
    if (wStateMachine == WIFI_SM)     		          return &WifiStateMachine;
#endif    
#if USBOTG_HOST_PTP
    if (wStateMachine == SIDC_INIT_SM)                    return &SidcInitStateMachine;
    if (wStateMachine == SIDC_SCAN_OBJECTS_SM)            return &SidcScanObjectsStateMachine;
#endif
#if (USBOTG_WEB_CAM || USB_HOST_ISO_TEST)
	if (wStateMachine == WEB_CAM_SM)            return &webCamStateMachine;
#endif

#if USBOTG_HOST_CDC
	if (wStateMachine == CDC_INIT_SM)            return &CdcInitStateMachine;
#endif

#if USBOTG_HOST_HID
	if (wStateMachine == HID_INIT_SM)            return &HidInitStateMachine;
#endif

#if USBOTG_HOST_DATANG
	if (wStateMachine == DATANG_INIT_SM)		return &DatangInitStateMachine;
#endif
#if DM9621_ETHERNET_ENABLE
	if (wStateMachine == USB_ETHERNET_SM)		return &EthernetStateMachine;
#endif
    return NULL;
}

void UsbhEnableMultiLun(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_MSDC psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);
    psMsdc->boSupportMultiLun = TRUE;
}

void UsbhDisableMultiLun(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_MSDC psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);
    psMsdc->boSupportMultiLun = FALSE;
}

BYTE GetDeviceAddress(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_MSDC psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);

    return psMsdc->bUsbhDeviceAddress;
}


void ClearDeviceAddress(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_MSDC psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);

    psMsdc->bUsbhDeviceAddress = 0;
}

BOOL IsNoNeedToPolling(WHICH_OTG eWhichOtg)
{ // need to polling always even max LUN is 1; like as one LUN card reader
    PUSB_HOST_MSDC psMsdc;
    psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);
    return psMsdc->boNoNeedToPollingUsbDevice;//gNoNeedToPollingUsbDevice;
}

void ClearNoNeedToPollingFlag(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_MSDC psMsdc;

    psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);
    psMsdc->boNoNeedToPollingUsbDevice = FALSE;//gNoNeedToPollingUsbDevice = FALSE;
}

void SetNoNeedToPollingFlag(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_MSDC psMsdc;

    psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);
    psMsdc->boNoNeedToPollingUsbDevice = TRUE;//gNoNeedToPollingUsbDevice = TRUE;
}



      
/*
// Definition of local functions 
*/
//void UsbOtgHostStateChange()
static void UsbOtgHostStateChange(void *arg)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    WHICH_OTG eWhichOtg= ((BYTE *) arg)[1];

    MP_DEBUG("UsbOtgHostStateChange:eWhichOtg = %d", eWhichOtg);
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    pSendMailDrv->wStateMachine             = SETUP_SM;
    pSendMailDrv->wCurrentExecutionState    = SETUP_PORT_RESET_STATE;
    SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
}

static void UsbOtgHostPortResetDone(void *arg)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    WHICH_OTG eWhichOtg= ((BYTE *) arg)[1];

    MP_DEBUG("UsbOtgHostPortResetDone:eWhichOtg = %d", eWhichOtg);
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    pSendMailDrv->wStateMachine             = SETUP_SM;
    pSendMailDrv->wCurrentExecutionState    = SETUP_PORT_RESET_DONE_STATE;
    SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
}

static void UsbOtgHostDeviceInitDone(void *arg)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    WHICH_OTG eWhichOtg= ((BYTE *) arg)[1];

    MP_DEBUG("UsbOtgHostDeviceInitDone:eWhichOtg = %d", eWhichOtg);
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    pSendMailDrv->wStateMachine             = SETUP_SM;
    pSendMailDrv->wCurrentExecutionState    = SETUP_DEVICE_INIT_STATE;
    SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
}

static void UsbOtgHostDeviceInitDone2(void *arg)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    WHICH_OTG eWhichOtg= ((BYTE *) arg)[1];

    MP_DEBUG("UsbOtgHostDeviceInitDone2:eWhichOtg = %d", eWhichOtg);
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    pSendMailDrv->wStateMachine             = SETUP_SM;
    pSendMailDrv->wCurrentExecutionState    = SETUP_GET_DEVICE_DESCRIPTOR_STATE;
    SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
}

void UsbOtgHostMsdcPollingForFullSpeed(void *arg)
{
    WHICH_OTG eWhichOtg= ((BYTE *) arg)[1];  
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    
    MP_DEBUG("FS Polling Timer Function USBOTG%d", eWhichOtg);
    
    if (mwHost20_PORTSC_ConnectStatus_Rd()==0)
    {
        MP_DEBUG("FS Device PlugOut and just return");
        return;
    }
    
    if (CMP_START_TIMER == GetCheckMediaPresentTimerFlag(eWhichOtg))
    {
        MP_DEBUG("FS EventSet Polling");
        SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
        EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_POLLING_EACH_LUN);
    }
    
    UsbOtgSetSysTimerProc(USBOTG_HOST_FULL_SPEED_DEVICE_POLLING, UsbOtgHostMsdcPollingForFullSpeed, 1, eWhichOtg);
}

static WORD NextMcardIdCmp (WORD wCurId, BYTE bLun, BYTE bMaxLun, WHICH_OTG eWhichOtg)
{
    WORD wNextId = 0;

    wNextId = wCurId;
    if (bLun < bMaxLun)
    {
        wNextId ++;
    }
    else
    {   
        if (eWhichOtg == USBOTG0)
        {
            wNextId = DEV_USB_HOST_ID1;
        }
        else if (eWhichOtg == USBOTG1)
        {
            wNextId = DEV_USBOTG1_HOST_ID1;
        }
        else
        {
            MP_ALERT("%s USBOTG%d cannot recognized!!", __FUNCTION__, eWhichOtg);
        }
        //wNextId = 1;
    }

    // start CMP timer                    
    MP_DEBUG("1 START T");
    SetCheckMediaPresentTimerFlag(CMP_START_TIMER, eWhichOtg);

    return wNextId;
}

void UsbOtgHostEof1Eof2Adjustment(WHICH_OTG eWhichOtg)
{
    //static BYTE test_item = 0; 
    BYTE bEOF1_Time = 0;
    BYTE bEOF2_Time = 0;
    BYTE ASYNC_SLEEP_Time = 0;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_HOST psHost = &psUsbOtg->sUsbHost;
    
    if (GetRetryCount(eWhichOtg) == 1)
        psHost->stSetupSm.bEofRetryCnt = 0;
    
    MP_DEBUG("psHost->stSetupSm.bEofRetryCnt = %d", psHost->stSetupSm.bEofRetryCnt);
    mwHost20_Misc_ASYN_SCH_SLPT_Set(3);
    switch(psHost->stSetupSm.bEofRetryCnt)
    {
        case 0:
            bEOF1_Time = 3;
            bEOF2_Time = 0;
        break;
        
        case 1:
            bEOF1_Time = 0;
            bEOF2_Time = 1;
        break;

        case 2:
            bEOF1_Time = 0;
            bEOF2_Time = 2;
        break;

        case 3:
            bEOF1_Time = 1;
            bEOF2_Time = 0;
        break;

        case 4:
            bEOF1_Time = 2;
            bEOF2_Time = 0;
        break;

        case 5:
            bEOF1_Time = 1;
            bEOF2_Time = 1;
        break;

        case 6:
            bEOF1_Time = 2;
            bEOF2_Time = 1;
        break;

        case 7:
            bEOF1_Time = 3;
            bEOF2_Time = 1;
        break;

        case 8:
            bEOF1_Time = 0;
            bEOF2_Time = 3;
        break;

        case 9:
            bEOF1_Time = 1;
            bEOF2_Time = 2;
        break;

        case 10:
            bEOF1_Time = 2;
            bEOF2_Time = 2;
        break;

        case 11:
            bEOF1_Time = 3;
            bEOF2_Time = 2;
        break;

        case 12:
            bEOF1_Time = 0;
            bEOF2_Time = 0;
        break;

        case 13:
            bEOF1_Time = 1;
            bEOF2_Time = 3;
        break;

        case 14:
            bEOF1_Time = 2;
            bEOF2_Time = 3;
        break;

        case 15:
            bEOF1_Time = 3;
            bEOF2_Time = 3;
        break;
    }
    //mwHost20_Misc_ASYN_SCH_SLPT_Set(0);
    MP_DEBUG("retry cnt = %d", GetRetryCount(eWhichOtg));
    if (GetRetryCount(eWhichOtg) < 1)
        psHost->stSetupSm.bEofRetryCnt = 0;
    else
        psHost->stSetupSm.bEofRetryCnt++;
    MP_DEBUG("set EOF1 = %d and EOF2 = %d", bEOF1_Time, bEOF2_Time);
    mwHost20_Misc_EOF2Time_Set(bEOF2_Time); 
    mwHost20_Misc_EOF1Time_Set(bEOF1_Time);

}
//extern qHD_Structure     *psHost20_qHD_List_Control[];
//extern qHD_Structure     *psHost20_qHD_List_Bulk[]; 
//extern DWORD gHost20_STRUCTURE_BASE_ADDRESS;


static void SetupStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
    //static DWORD                frame_idx = 0;
    //static BYTE                 config_idx = 0;
    //static BYTE                 interface_idx = 0;
    //static BYTE                 try_cnt = 0;
    SWORD                       err = USB_NO_ERROR;

    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_HOST psHost;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    PST_APP_CLASS_DESCRIPTOR    pAppDes;
    PST_HUB_CLASS_DESCRIPTOR    pHubDes;
    PUSB_HOST_EHCI              psEhci;
    PUSB_HOST_MSDC              psMsdc;
    ST_MCARD_MAIL               receiveMailDrv;
    ST_MCARD_MAIL               *pSendMailDrv;
    DWORD   i = 0;

    err = USB_NO_ERROR;
    //static BOOL isProcessedBiuReset = FALSE;
    //static BOOL isHostToResetDevice = FALSE;

    psHost      = &psUsbOtg->sUsbHost;
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pAppDes     = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    pHubDes     = psHost->psHubClassDescriptor;
    psEhci      = &psHost->sEhci;
    psMsdc      = &psHost->sMsdc;

    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
#if (STD_BOARD_VER == MP650_FPGA)
	pUsbh->sMDevice[bMcardTransferID].sMcardRMail->wStateMachine = 0;
	pUsbh->sMDevice[bMcardTransferID].sMcardRMail->wCurrentExecutionState = 0;
#endif

	pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        ClearRetryCount(eWhichOtg);
        //__asm("break 100");
        UsbOtgHostInactiveAllqTD(eWhichOtg);
        if (pUsbhDevDes->bDeviceStatus == USB_STATE_ATTACHED)
        {
            MP_DEBUG("-USBOTG%d- 1Device Plug Out", eWhichOtg);
        }
        else
        {
            MP_DEBUG("-USBOTG%d- 2Device Plug Out", eWhichOtg);
            if ( receiveMailDrv.wCurrentExecutionState != SETUP_INIT_STOP_STATE)
            {
                MP_DEBUG("-USBOTG%d- 3Device Plug Out", eWhichOtg);
                //pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
                //if (mwHost20_USBINTR_Rd() == 0)
                //    mwHost20_USBINTR_Set(HOST20_USBINTR_ENABLE);
                //return;
            }
        }

        receiveMailDrv.wCurrentExecutionState = SETUP_INIT_STOP_STATE;
    }

    MP_DEBUG("-USBOTG%d- SSM-stat %d" , eWhichOtg, receiveMailDrv.wCurrentExecutionState);
    switch( receiveMailDrv.wCurrentExecutionState)
    {
        case SETUP_INIT_START_STATE:
        {
            UsbOtgHostGetQhdForEachEd(eWhichOtg);
            UsbOtgHostSetSwapMemoryPool(eWhichOtg);
            if (UsbOtgHostConnectStatusGet(eWhichOtg))
            { // plug in
                //UartOutText("Plug-in\r\n");
                //pSendMailDrv->wCurrentExecutionState = SETUP_PORT_RESET_STATE;
                //pSendMailDrv->wStateMachine          = SETUP_SM;

#if USBOTG_HOST_DESC
                InitConfigDescriptor(pUsbhDevDes);
#endif
                pUsbhDevDes->bDeviceAddress = 0;
                pUsbhDevDes->sDeviceDescriptor.bLength = 8;
                psHost->stSetupSm.bConfigIndex = 0;

                // Reset the chip & Set Init Configuration
                mbHost20_USBCMD_ParkMode_CNT_Set(1); 
                //mwHost20_Misc_EOF2Time_Set(3);   
                mwHost20_Misc_EOF2Time_Set(0);
                mwHost20_Misc_EOF1Time_Set(3);
                mwHost20_Misc_ASYN_SCH_SLPT_Set(3); // Follow CPU & Memory Clock Upgrade
                MP_DEBUG("-USBOTG%d- HcMisc = 0x%x", eWhichOtg, psUsbOtg->psUsbReg->HcMisc);
                // Init All the Data Structure & write Base Address register
                //Write Base Address to Register 
                
                mwHost20_CurrentAsynchronousAddr_Set(psEhci->dwHostStructureQhdBaseAddress);

                pUsbhDevDes->bDeviceStatus = USB_STATE_DEFAULT;
                UsbOtgSetSysTimerProc(USBOTG_HOST_STATE_CHANGE_WAIT_TIME, UsbOtgHostStateChange, 1, eWhichOtg);
                MP_DEBUG("-USBOTG%d- AddTimerProc A", eWhichOtg);
                return;
            }
            else
            { // plug out
                //UartOutText("init:Plug-out\r\n");
                pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
                return;
            }
        }
        break;

        case SETUP_PORT_RESET_STATE:
        {
            DWORD timercnt = 0;
            //flib_Host20_ForceSpeed(2, psUsbOtg); // Mark to send chirp normally when HS handshake
            mwHost20_PORTSC_PortReset_Set();
            //while((mwHost20_PORTSC_LineStatus_Rd() != 1) && (timercnt <= 500000)) // 7ms
            //{
             //   timercnt++;
            //}
            //flib_Host20_ForceSpeed(0, psUsbOtg);
            UsbOtgSetSysTimerProc(USBOTG_HOST_PORT_RESET_TIME, UsbOtgHostPortResetDone, 1, eWhichOtg);
            MP_DEBUG("-USBOTG%d- AddTimerProc B", eWhichOtg);
            //pSendMailDrv->wStateMachine = SETUP_SM;
            //pSendMailDrv->wCurrentExecutionState = SETUP_PORT_RESET_DONE_STATE;
        }
        break;
        
        case SETUP_PORT_RESET_DONE_STATE:
        {
            mwHost20_PORTSC_PortReset_Clr();
            flib_Host20_StopRun_Setting(HOST20_Enable, eWhichOtg);
            pUsbhDevDes->bDeviceSpeed= mwOTG20_Control_HOST_SPD_TYP_Rd(); 
            MP_DEBUG("-USBOTG%d- %s Speed:%d", eWhichOtg, pUsbhDevDes->bDeviceSpeed == HOST20_Attach_Device_Speed_High?"High":"Full:0/Low:1", pUsbhDevDes->bDeviceSpeed);
            
            UsbOtgSetSysTimerProc(USBOTG_HOST_DEVICE_INIT_TIME, UsbOtgHostDeviceInitDone, 1, eWhichOtg);
            MP_DEBUG("-USBOTG%d- AddTimerProc C", eWhichOtg);
        }
        break;
        
        case SETUP_DEVICE_INIT_STATE:
        {
            pSendMailDrv->wStateMachine = SETUP_SM;
            pSendMailDrv->wCurrentExecutionState = SETUP_DEVICE_INIT_DONE_STATE;
        }
        break;
        
        case SETUP_DEVICE_INIT_DONE_STATE:
        {
            flib_Host20_QHD_Control_Init(eWhichOtg);
            UsbOtgSetSysTimerProc(USBOTG_HOST_DEVICE_INIT_TIME2, UsbOtgHostDeviceInitDone2, 1, eWhichOtg);
        }
        break;
        
        case SETUP_GET_DEVICE_DESCRIPTOR_STATE:
        {
            if (!(pUsbhDevDes->bDeviceStatus == USB_STATE_DEFAULT ||
                  pUsbhDevDes->bDeviceStatus == USB_STATE_ADDRESS))
            { // DeviceStatus must be USB_STATE_DEFAULT and USB_STATE_ADDRESS, then continue process
                MP_DEBUG("-USBOTG%d- return from SETUP_GET_DEVICE_DESCRIPTOR_STATE!", eWhichOtg);
                err = USB_ENUM_ERROR;
                //return;
            }
            else
            {
                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                pSendMailDrv->wStateMachine          = SETUP_SM;
                pSendMailDrv->wCurrentExecutionState = SETUP_GET_DEVICE_DESCRIPTOR_DONE_STATE;
                err = SetupGetDeviceDescriptor(eWhichOtg);
                if (err != USB_NO_ERROR)
                {
                    err = USB_SET_EVENT_FAILED;
                    //return;
                }
            }
        }
        break;

        case SETUP_GET_DEVICE_DESCRIPTOR_DONE_STATE:
        {
            pSendMailDrv->wStateMachine= SETUP_SM;
            if (pUsbhDevDes->bDeviceAddress == 0)
            {   // after get 8 bytes of device descriptor, then  set address
                memcpy ((BYTE*)&(pUsbhDevDes->sDeviceDescriptor),
                        (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress, 
                        sizeof(USB_DESCRIPTOR_HEADER));
                UsbOtgHstSetMaxPacketSizeForControlPipe(pUsbhDevDes->sDeviceDescriptor.bMaxPacketSize, pUsbhDevDes);
                pSendMailDrv->wCurrentExecutionState = SETUP_SET_ADDRESS_STATE;
            }
            else
            {   // after get all bytes of device descriptor and set address already, then get config descriptor
                memcpy ((BYTE*)&(pUsbhDevDes->sDeviceDescriptor),
                        (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress, 
                        pUsbhDevDes->sDeviceDescriptor.bLength);
                

				if (CheckIfPanasonicDMC_FX8(pUsbhDevDes->sDeviceDescriptor.idVendor, pUsbhDevDes->sDeviceDescriptor.idProduct))
				{	// set eof1 & eof2 for Panasonic DMC-FX8
					mwHost20_Misc_EOF2Time_Set(0);
					mwHost20_Misc_EOF1Time_Set(0);
				}

                #if USBOTG_HOST_DESC
                
                    //FreeConfigDescriptor(pUsbhDevDes); // After UsbHostInit( ) whether it was free, it will be 0. Move to UsbHostInit( )
                    if(pUsbhDevDes->sDeviceDescriptor.bNumConfigurations > 0)
                    {
                        AllocConfigDescriptor(pUsbhDevDes);
                    }
                    
                #endif
                
                pSendMailDrv->wCurrentExecutionState = SETUP_GET_CONFIG_DESCRIPTOR_STATE;

                #if USBOTG_HOST_CDC
                // Qisda USB 3.5G Modem  // idVendor  0x1DA5  idProduct 0xF000
                // Send vendor cmd to RESET : 40 04 00 00 00 00 10 00
                if( pUsbhDevDes->sDeviceDescriptor.bDeviceClass == USB_CLASS_PER_INTERFACE &&
                    pUsbhDevDes->sDeviceDescriptor.idVendor == (WORD)byte_swap_of_word(HSUPA_USB_DEVICE_VID) && 
                    pUsbhDevDes->sDeviceDescriptor.idProduct == (WORD)byte_swap_of_word(HSUPA_USB_MSDC_PID))
                {
                    MP_ALERT("-USBOTG%d- Qisda MSDC trasfer to CDC", eWhichOtg); // Qisda 3.5G Modem
                    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_CDC_SENDER, eWhichOtg);
                    pSendMailDrv->wStateMachine= CDC_INIT_SM;
                    pSendMailDrv->wCurrentExecutionState = CDC_INIT_VENDOR_CMD_STATE;
                }              
                #endif
                
            }

            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
        }
        break;
        
        case SETUP_SET_ADDRESS_STATE:
        {
            psMsdc->bUsbhDeviceAddress++;
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine          = SETUP_SM;
            pSendMailDrv->wCurrentExecutionState = SETUP_SET_ADDRESS_DONE_STATE;
            err = SetupSetAddress(eWhichOtg);         
            if (err != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;
                //return;
            }
        }
        break;

        case SETUP_SET_ADDRESS_DONE_STATE:
        {
            //__asm("break 100");
            pUsbhDevDes->bDeviceAddress             = GetDeviceAddress(eWhichOtg);
            UsbOtgHostSetAddressForUsbAddressState(pUsbhDevDes->bDeviceAddress,pUsbhDevDes);
            UsbOtgSetSysTimerProc(USBOTG_HOST_DEVICE_INIT_TIME2, UsbOtgHostDeviceInitDone2, 1, eWhichOtg);
            MP_DEBUG("-USBOTG%d- AddTimerProc D", eWhichOtg);
            pUsbhDevDes->bDeviceStatus              = USB_STATE_ADDRESS;
//            pSendMailDrv->wStateMachine             = SETUP_SM;
//            pSendMailDrv->wCurrentExecutionState    = SETUP_GET_DEVICE_DESCRIPTOR_STATE;
        }
        break;
        

        case SETUP_GET_CONFIG_DESCRIPTOR_STATE:
        {          
            if (pUsbhDevDes->bDeviceStatus != USB_STATE_ADDRESS)
            { // DeviceStatus must be USB_STATE_ADDRESS, then continue process
                err = USB_ENUM_ERROR;
                //return;
            }
            else
            {
                if (pUsbhDevDes->sDeviceDescriptor.bNumConfigurations > 0) 
                {   
                    pUsbhDevDes->bDeviceConfigIdx           = psHost->stSetupSm.bConfigIndex ;
                    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                    pSendMailDrv->wStateMachine             = SETUP_SM;
                    pSendMailDrv->wCurrentExecutionState    = SETUP_GET_CONFIG_DESCRIPTOR_DONE_STATE;
                    err = SetupGetConfigurationDescriptor(eWhichOtg);
                    if (err != USB_NO_ERROR)
                    {
                        err = USB_SET_EVENT_FAILED;
                        //return;
                    }                   
                }
                else
                {
                    MP_DEBUG("-USBOTG%d- NO CONFIG!!", eWhichOtg);
                }
            }
        }
        break;

        case SETUP_GET_CONFIG_DESCRIPTOR_DONE_STATE:
        {
            SetupGetConfigurationDescriptorDone(eWhichOtg);            
#if USBOTG_HOST_DESC
            if (GetInterfaceEPs(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx) == 0) 
#else    
            if (pUsbhDevDes->sInterfaceDescriptor[psHost->stSetupSm.bInterfaceIndex].bNumEndpoints == 0) 
#endif   
            { // does NOT get interface descriptor yet
                pUsbhDevDes->bDeviceConfigIdx           = psHost->stSetupSm.bConfigIndex ;
                pSendMailDrv->wStateMachine             = SETUP_SM;
                pSendMailDrv->wCurrentExecutionState    = SETUP_GET_CONFIG_DESCRIPTOR_STATE;
            }
            else
            { // has got the total bytes of interface descriptor
                pSendMailDrv->wStateMachine            = SETUP_SM;
                pSendMailDrv->wCurrentExecutionState   = SETUP_GET_CONFIG_DESCRIPTOR_STATE;
#if USBOTG_HOST_DESC
                if ((USB_CLASS_MASS_STORAGE ==
                     GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) ||
                    (USB_CLASS_STILL_IMAGE ==
                     GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) ||
                    (USB_CLASS_WIRELESS_BT ==
                     GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) ||
                    (USB_CLASS_VENDOR_SPEC ==
                     GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) ||
                    (USB_CLASS_VIDEO ==
                     GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) ||
     #if USBOTG_HOST_CDC
                    (USB_CLASS_COMM ==
                     GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) || 
                    (USB_CLASS_CDC_DATA ==
                     GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) || 
     #endif   
#if USBOTG_HOST_HID
                    (USB_CLASS_HID == 
                     GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) || 
#endif        
                    (USB_CLASS_PER_INTERFACE ==
                     GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)))
#else // USBOTG_HOST_DESC
                if ((USB_CLASS_MASS_STORAGE ==
                    pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass) ||
                    (USB_CLASS_STILL_IMAGE ==
                    pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass) ||
                    (USB_CLASS_WIRELESS_BT ==
                    pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass) ||
                    (USB_CLASS_VENDOR_SPEC ==
                    pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass) ||
                    (USB_CLASS_VIDEO ==
                    pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass) ||
#if USBOTG_HOST_CDC
                    (USB_CLASS_COMM == pUsbhDevDes->sDeviceDescriptor.bDeviceClass) || 
#endif   
                    (USB_CLASS_PER_INTERFACE ==
                    pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass))
#endif  // USBOTG_HOST_DESC                  
                {
                    if(pAppDes->bDeviceConfigVal != USBH_CONFIG_VALUE_NOT_DEFINED)
                    {   // finish SETUP_GET_CONFIG_DESCRIPTOR_STATE, pass to next state
                        pSendMailDrv->wCurrentExecutionState   = SETUP_SET_CONFIG_STATE;
                        pSendMailDrv->wStateMachine            = SETUP_SM;
                    }
                    else
                    {   // has one more config we need to obtain
                        psHost->stSetupSm.bConfigIndex  ++;
                    }
                }
#if USBOTG_HOST_DESC
                else if((USB_CLASS_HUB ==
                         GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) &&
                        (pUsbhDevDes->bDeviceAddress != 0))                     
#else                
                else if((USB_CLASS_HUB ==
                         pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass) &&
                        (pUsbhDevDes->bDeviceAddress != 0))
#endif
                {
                    if(pAppDes->bDeviceConfigVal != USBH_CONFIG_VALUE_NOT_DEFINED)
                    {   // finish SETUP_GET_CONFIG_DESCRIPTOR_STATE, pass to next state
                        pSendMailDrv->wCurrentExecutionState   = SETUP_GET_HUB_DESCRIPTOR_STATE;
                        pSendMailDrv->wStateMachine            = SETUP_SM;
                    }
                    else
                    {   // has one more config we need to obtain
                        psHost->stSetupSm.bConfigIndex  ++;
                    }
                }
                else
                {
                    err = USB_HOST_NOT_SUPPORTED_DEVICE;
                    if (pUsbhDevDes->bDeviceAddress > 1)
                    {
                       // __asm("break 100");
            			MP_DEBUG("-USBOTG%d- it's the device plug-in with HUB. bPortNumber = %d; total port number pf HUB = %d", eWhichOtg, pHubDes->bPortNumber , pHubDes->sHubDescriptor.bNbrPorts);
                        if (pHubDes->bPortNumber < pHubDes->sHubDescriptor.bNbrPorts)
                        { // there are some ports do not check the status
                            err = USB_NO_ERROR;
                            UsbOtgHostSetAddressForUsbAddressState(pHubDes->bDeviceAddress, pUsbhDevDes);
                            pHubDes->bPortNumber++;
            	            MP_DEBUG("-USBOTG%d- HUB address = %d", eWhichOtg, pHubDes->bDeviceAddress);
            		        MP_DEBUG("-USBOTG%d- change to bPortNumber = %d", eWhichOtg, pHubDes->bPortNumber);
                            pSendMailDrv->wCurrentExecutionState   = SETUP_HUB_GET_PORT_STATUS_STATE;
                            pSendMailDrv->wStateMachine            = SETUP_HUB_SM;
                        }
                    }
                }
            }

            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
        }
        break;
// for HUB, coding later
        case SETUP_GET_HUB_DESCRIPTOR_STATE:
        {
            //__asm("break 100");
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = SETUP_SM;
            pSendMailDrv->wCurrentExecutionState    = SETUP_GET_HUB_DESCRIPTOR_DONE_STATE;
            err = SetupGetHubDescriptor(eWhichOtg);
            if (err != USB_NO_ERROR)
            {    
                err = USB_SET_EVENT_FAILED;
                //return;
            }
        }
        break;

        case SETUP_GET_HUB_DESCRIPTOR_DONE_STATE:
        {
            pHubDes->sHubDescriptor.bDescriptorType  = *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress+1);
            pHubDes->sHubDescriptor.bNbrPorts        = *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress+2);
            pHubDes->sHubDescriptor.wHubCharacteristics = \
                                       uchar_to_ushort(*((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress+4),\
                                                       *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress+3));
            pHubDes->sHubDescriptor.bPwrOn2PwrGood   = *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress+5);
            pHubDes->sHubDescriptor.bHubContrCurrent = *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress+6);
            pHubDes->sHubDescriptor.bDeviceRemovable = *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress+7);
            pHubDes->sHubDescriptor.bPortPwrCtrlMask = *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress+8);
                pSendMailDrv->wCurrentExecutionState = SETUP_GET_STATUS_STATE;
                pSendMailDrv->wStateMachine = SETUP_SM;
            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
        }
        break;

        case SETUP_GET_STATUS_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine          = SETUP_SM;
            pSendMailDrv->wCurrentExecutionState = SETUP_GET_STATUS_DONE_STATE;
            err = SetupGetSatus(eWhichOtg);
            if (err == USB_NO_ERROR)
            {    
                err = USB_SET_EVENT_FAILED;
                //return;
            }
        }
        break;

        case SETUP_GET_STATUS_DONE_STATE:
        {
            pUsbhDevDes->sDeviceStatus.bfSelfPowered = (*((WORD*)pUsbhDevDes->sSetupPB.dwDataAddress)>>8);
            pUsbhDevDes->sDeviceStatus.bfRemoteWakeup  = (*((WORD*)pUsbhDevDes->sSetupPB.dwDataAddress)>>9);
                pSendMailDrv->wCurrentExecutionState = SETUP_SET_CONFIG_STATE;
                pSendMailDrv->wStateMachine= SETUP_SM;
            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
        }
        break;
        case SETUP_SET_CONFIG_STATE:
        {
            if (pUsbhDevDes->bDeviceStatus != USB_STATE_ADDRESS)
            { // DeviceStatus must be USB_STATE_ADDRESS, then continue process
                err = USB_ENUM_ERROR;
                //return;
            }
            else
            {
                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                pSendMailDrv->wStateMachine             = SETUP_SM;
                pSendMailDrv->wCurrentExecutionState    = SETUP_SET_CONFIG_DONE_STATE;
#if USB_WIFI 
                if (pUsbhDevDes->sDeviceDescriptor.idVendor == (0xce0a) &&
                     (pUsbhDevDes->sDeviceDescriptor.idProduct==(0x1512)))
                {
                    /* ar2524 can't do SetupConfiguration before its FW is downloaded */
                    err = USB_NO_ERROR;
                }
                else
#endif
                err = SetupSetConfguration(eWhichOtg);
                if (err != USB_NO_ERROR)
                {
                    err = USB_SET_EVENT_FAILED;
                    //return;
                }
            }
        }
        break;

        case SETUP_SET_CONFIG_DONE_STATE:
        {
            BYTE bInterfaceClass = 0;
            pUsbhDevDes->bDeviceStatus = USB_STATE_CONFIGURED;
            if(pAppDes->bDeviceConfigVal == USBH_CONFIG_VALUE_NOT_DEFINED)
            {
                MP_DEBUG("-USBOTG%d- CONFIG value does not defined!!", eWhichOtg);
            }

#if USBOTG_HOST_DESC
            bInterfaceClass = \
                GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx);
#else
            bInterfaceClass = \
                pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass
#endif

            // WebCam need to assign InterfaceNum = 1 in the many Interfaces with the same class in different resolution.
            UsbOtgHostGetAllEpsInformation ( bInterfaceClass, 
                                             (bInterfaceClass == USB_CLASS_VIDEO ? 1 : 0), 
                                             eWhichOtg); // for all pipes
           //UsbOtgHostGetQhdForEachEd(eWhichOtg);
           UsbOtgHostSetDeviceDescriptor(eWhichOtg);


            switch(bInterfaceClass)
            {
                case USB_CLASS_MASS_STORAGE:
                {
                    pSendMailDrv->wCurrentExecutionState   = MSDC_INIT_START_STATE;
                    pSendMailDrv->wStateMachine            = MSDC_INIT_SM;
                }
                break;

                case USB_CLASS_STILL_IMAGE:
                {
	                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
                    pSendMailDrv->wCurrentExecutionState   = SIDC_INIT_START_STATE;
                    pSendMailDrv->wStateMachine            = SIDC_INIT_SM;
                }
                break;
                
 #if USBOTG_HOST_CDC
                case USB_CLASS_COMM:
                case USB_CLASS_CDC_DATA:
                {
                    MP_DEBUG("-USBOTG%d- [CDC] SET_CONFIG_DONE %x", eWhichOtg, pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass); 
	                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_CDC_SENDER, eWhichOtg);
                    pSendMailDrv->wCurrentExecutionState   = CDC_INIT_START_STATE;
                    pSendMailDrv->wStateMachine            = CDC_INIT_SM;
                }
                break;
 #endif       

 #if USBOTG_HOST_HID
                case USB_CLASS_HID:
                {
                    MP_DEBUG("-USBOTG%d- [HID] SET_CONFIG_DONE %x", eWhichOtg, pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass); 
                    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_HID_SENDER, eWhichOtg);
                    pSendMailDrv->wCurrentExecutionState   = HID_INIT_START_STATE; 
                    pSendMailDrv->wStateMachine            = HID_INIT_SM;
                }
                break;
 #endif   

#if USBOTG_WEB_CAM 
                case USB_CLASS_VIDEO:
                {
                    if (pSendMailDrv->wStateMachine != WEB_CAM_SM)
                    {
#if USBOTG_HOST_ISOC
                        UsbOtgHostPrepareDataPage(eWhichOtg);
#endif
                        pSendMailDrv->wCurrentExecutionState   = WEB_CAM_INIT; // WEB_CAM_BEGIN_STATE0
                        pSendMailDrv->wStateMachine            = WEB_CAM_SM;
                    }
                }
                break;
#endif

#if (NETWARE_ENABLE == ENABLE || USB_HOST_ISO_TEST)
				case USB_CLASS_PER_INTERFACE:
                case USB_CLASS_VENDOR_SPEC:
                {
#if USB_HOST_ISO_TEST
					mpDebugPrint("USB_HOST_ISO_TEST");
					pSendMailDrv->wCurrentExecutionState   = ISO_OUT_STATE;
                    pSendMailDrv->wStateMachine            = WEB_CAM_SM;
#else
	#if USBOTG_HOST_DATANG
                // Datang USB 3G Modem  // idVendor  0x1AB7  idProduct 0x6000
                if( pUsbhDevDes->sDeviceDescriptor.bDeviceClass == USB_CLASS_PER_INTERFACE &&
                    pUsbhDevDes->sDeviceDescriptor.idVendor == (WORD)byte_swap_of_word(TDSCDMA_USB_DEVICE_VID) && 
                    pUsbhDevDes->sDeviceDescriptor.idProduct == (WORD)byte_swap_of_word(TDSCDMA_USB_PID))
                	{
	                    MP_ALERT("-USBOTG%d- [Datang] SET_CONFIG_DONE", eWhichOtg); // Datang 3G Modem
	                    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_DATANG_SENDER, eWhichOtg);
	                    pSendMailDrv->wStateMachine= DATANG_INIT_SM;
	                    pSendMailDrv->wCurrentExecutionState = DATANG_INIT_START_STATE;
                	}
                else	// Datang USB 3G Modem could plug-in with wifi dongle.
	#endif
	#if DM9621_ETHERNET_ENABLE
                if(	pUsbhDevDes->sDeviceDescriptor.idVendor == (WORD)byte_swap_of_word(DM_USB_DEVICE_VID) && (
                    pUsbhDevDes->sDeviceDescriptor.idProduct == (WORD)byte_swap_of_word(DM9621_USB_PID)||
                    pUsbhDevDes->sDeviceDescriptor.idProduct == (WORD)byte_swap_of_word(DM9621_USB_OTHER_PID)))
                	{
						mpDebugPrint("Davicom Device Class %02x",pUsbhDevDes->sDeviceDescriptor.bDeviceClass);
						pSendMailDrv->wCurrentExecutionState   = ETHERNET_INIT_START_STATE;
						pSendMailDrv->wStateMachine            = USB_ETHERNET_SM;
                	}
                else
	#endif

			{
				
                    pSendMailDrv->wCurrentExecutionState   = WIFI_INIT_START_STATE;
                    pSendMailDrv->wStateMachine            = WIFI_SM;
			}
#endif
                }
                break;
#endif    
                        
                case USB_CLASS_HUB:
					{
                    MP_DEBUG("-USBOTG%d- port number is %d addr is %x", eWhichOtg, pHubDes->sHubDescriptor.bNbrPorts, &pHubDes->sHubDescriptor.bNbrPorts);
                        pHubDes->bDeviceAddress     = pUsbhDevDes->bDeviceAddress;
                        pHubDes->bMaxInterruptInEpNumber  = pUsbhDevDes->bMaxInterruptInEpNumber;
                        pHubDes->hstInterruptInqHD        = pUsbhDevDes->hstInterruptInqHD;
                        pHubDes->hstIntInWorkqTD          = pUsbhDevDes->hstIntInWorkqTD;
                        pHubDes->hstIntInSendLastqTD      = pUsbhDevDes->hstIntInSendLastqTD;
                        pHubDes->hstInterruptInqTD        = pUsbhDevDes->hstInterruptInqTD;

                        pSendMailDrv->wCurrentExecutionState   = SETUP_HUB_INIT_START_STATE;
                        pSendMailDrv->wStateMachine            = SETUP_HUB_SM;
                    }
                break;

#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
                case USB_CLASS_WIRELESS_BT:
                {
                    if (eWhichOtg != BT_USB_PORT)
                    {
                        MP_ALERT("--E-- %s USBOTG%d BT dongle does not support on USBOTG%d!!", __FUNCTION__, BT_USB_PORT, eWhichOtg);
                        return;
                    }
                    else
                    {
#if USBOTG_HOST_ISOC
                        UsbOtgHostPrepareDataPage(eWhichOtg);
#endif
                        pSendMailDrv->wStateMachine             = BT_INIT_SM;
                        pSendMailDrv->wCurrentExecutionState    = BT_INIT_START_STATE;
                    }
                }
                break;
#endif    

                default:
                {
                    MP_DEBUG("-USBOTG%d- INTERFACE class does not be defined!!", eWhichOtg);
                }
                break;
            }

#if USBOTG_HOST_DESC
            if (USB_CLASS_MASS_STORAGE != GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
#else
            if (USB_CLASS_MASS_STORAGE != (pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass))
#endif
            {
                MP_DEBUG("-USBOTG%d- if not MSDC then stop polling timer", eWhichOtg);
                SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);                
            }
        }
        break;
        
        case SETUP_INIT_STOP_STATE:
        {
            //UsbHostFinal();
            ClearDeviceAddress(eWhichOtg);
            OTGH_PT_Bulk_Close(eWhichOtg);
            UsbOtgHostClose(eWhichOtg);
            MP_DEBUG("-USBOTG%d- pUsbhDevDes->bDeviceStatus= %d", eWhichOtg, pUsbhDevDes->bDeviceStatus);
            if (USB_STATE_CONFIGURED > pUsbhDevDes->bDeviceStatus )
            {
                MP_DEBUG("-USBOTG%d- NOT configured yet!!", eWhichOtg);
                goto _RE_INIT_USB_OTG_HOST_;
            }
            else
            {
                MP_DEBUG("-USBOTG%d- already configured!", eWhichOtg);
                //<1>.Suspend Host
                flib_Host20_Suspend(eWhichOtg);
            }
            pUsbhDevDes->bDeviceStatus = USB_STATE_NOTATTACHED;

#if USBOTG_HOST_DESC
            switch(GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
#else
            switch(pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass)
#endif
            {
                case USB_CLASS_MASS_STORAGE:
                {
                    BYTE    i = 0;

                    SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);                
                    if (HOST20_Attach_Device_Speed_Full == pUsbhDevDes->bDeviceSpeed)
                    {
                        //SysTimerProcRemove(UsbOtgHostMsdcPollingForFullSpeed);
                        MP_DEBUG("-USBOTG%d- RemoveTimerProc FS Polling timer function", eWhichOtg);
                    }
                    
                    for (i = 0; i <= pAppDes->bMaxLun; i++)
                    {
                        if (pAppDes->dEventSet[i] == 1)
                        {
                            // set event to main to un-mount
                             if (pAppDes->dPresent[i] == 1) // card is present; then set event for card out
                            	Ui_SetCardOutCodeByLun(i, eWhichOtg);
                                //EventSet(UI_EVENT, EVENT_CARD_OUT);  //Athena 03.11.2006 seperate card in & out //Joe 3/21
                            pAppDes->dEventSet[i] = 0;
                            pAppDes->dPresent[i] = 0;
                        }
                    }
                }
                break;

                case USB_CLASS_HUB:
                {
                    // none
                }
                break;

                case USB_CLASS_STILL_IMAGE:
                {
                    if (eWhichOtg == USBOTG0)
                        pAppDes->bDevLun = DEV_USB_HOST_PTP-1;
                    else if (eWhichOtg == USBOTG1)
                        pAppDes->bDevLun = DEV_USBOTG1_HOST_PTP-1;
                    else
                        MP_ALERT("-USBOTG%d- %s cannot be recognized!!", eWhichOtg, __FUNCTION__);
                    //pAppDes->bDevLun = DEV_USB_HOST_PTP-1;
                    if (pAppDes->dEventSet[pAppDes->bDevLun] == 1)
                    {
                        // set event to main to un-mount
                        if (pAppDes->dPresent[pAppDes->bDevLun] == 1) // card is present; then set event for card out
                            Ui_SetCardOutCodeByLun(pAppDes->bDevLun, eWhichOtg);
                        //EventSet(UI_EVENT, EVENT_CARD_OUT);  //Athena 03.11.2006 seperate card in & out //Joe 3/21
                        pAppDes->dEventSet[pAppDes->bDevLun] = 0;
                        pAppDes->dPresent[pAppDes->bDevLun] = 0;
                    }
#if USBOTG_HOST_PTP
                    UsbOtgHostSidcDevicePlugOut(eWhichOtg);
#endif
                }
                break;
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
                case USB_CLASS_WIRELESS_BT:
                {
                    if (eWhichOtg == BT_USB_PORT)
                    {                    
                #if USBOTG_HOST_ISOC
                        UsbOtgHostReleaseAllDataPage(eWhichOtg);
                        UsbOtgHostItdFree(eWhichOtg);
                #endif
                        UsbOtgPlugOut(eWhichOtg);
                    }
                }
                break;
#endif
#if (NETWARE_ENABLE == ENABLE)
                case USB_CLASS_PER_INTERFACE:
                case USB_CLASS_VENDOR_SPEC:
                {
#if Make_DM9621_ETHERNET
                    if( wifi_device_type == ETHERNET_USB_DEVICE_DM9621 )
                    {
                        NetEthernetDongleUnplug();
                        break;
                    }
#endif
#if Make_USB
	#if USBOTG_HOST_DATANG
			// Datang USB 3G Modem  // idVendor  0x1AB7  idProduct 0x6000
			if( pUsbhDevDes->sDeviceDescriptor.idVendor == (WORD)byte_swap_of_word(TDSCDMA_USB_DEVICE_VID) && 
			pUsbhDevDes->sDeviceDescriptor.idProduct == (WORD)byte_swap_of_word(TDSCDMA_USB_PID))
			{
				MP_ALERT("-USBOTG%d- [Datang] INIT_STOP", eWhichOtg); // Datang 3G Modem
				UsbOtgHostCdcAtCmdPlug(eWhichOtg, FALSE); // For AT-cmd
				pppClose(0);		// move from case NETUSB_PLUG_OUT in netusb_task()
				usbModemUnplug();	// move from case NETUSB_PLUG_OUT in netusb_task()
			}
			else		// Datang USB 3G Modem could plug-in with wifi dongle.
	#endif
			{
				{
	                usb_wifi_unplug();
					if(!SystemPwdcStausGet())
					{
					#if !Make_ADHOC
					#if (Make_WPA == 1)
					   Wpa_Set_PM(FALSE);
					#endif
					#endif
					   NetWifiDongleUnplug(); //cj add for wifi_unplug event 021809
					}
					else
					{
					#if !Make_ADHOC
					#if (Make_WPA == 1)
						Wpa_Set_PM(TRUE);
					#endif
					#endif
						mpDebugPrint("SYSTEM POWER DOWN!!");
					}

				}
			}
#endif
                }
                break;
#endif
#if USBOTG_WEB_CAM
                case USB_CLASS_VIDEO:
                    webCamDeInit(eWhichOtg);
                break;
#endif
#if USBOTG_HOST_CDC
                case USB_CLASS_COMM:
                case USB_CLASS_CDC_DATA:                  
				MP_DEBUG("-USBOTG%d- CDC:SETUP_INIT_STOP_STATE", eWhichOtg); /*Need*/ // write unplug code
				UsbOtgHostCdcAtCmdPlug(eWhichOtg, FALSE); // For AT-cmd
				pppClose(0);		// move from case NETUSB_PLUG_OUT in netusb_task()
				usbModemUnplug();	// move from case NETUSB_PLUG_OUT in netusb_task()
                break;
#endif

#if USBOTG_HOST_HID
                case USB_CLASS_HID:             
				MP_DEBUG("-USBOTG%d- HID:SETUP_INIT_STOP_STATE", eWhichOtg);
                            UsbhHidDeInit(eWhichOtg);
                break;
#endif

                default:
                {
                        MP_DEBUG("-USBOTG%d- INTERFACE class does not be defined!!", eWhichOtg);
                }
                break;
            }

_RE_INIT_USB_OTG_HOST_:
            if (1)//mwHost20_PORTSC_ConnectStatus_Rd()) 
            { 
                //if (pUsbhDevDes->bDeviceStatus == USB_STATE_NOTATTACHED)
                {
#if USBOTG_HOST_DESC
                    MP_DEBUG("--USBOTG%d !!FREE3!!\n", eWhichOtg);
                    FreeConfigDescriptor(pUsbhDevDes);
#endif
                    
                    InitiaizeUsbOtgHost(1, eWhichOtg);
                }

                psHost->stSetupSm.boIsHostToResetDevice = TRUE;
                pSendMailDrv->wCurrentExecutionState = SETUP_INIT_START_STATE;
                pSendMailDrv->wStateMachine          = SETUP_SM;
            }
            else
            {
                psHost->stSetupSm.boIsProcessedBiuReset = FALSE;
                psHost->stSetupSm.boIsHostToResetDevice = FALSE;
                err = USB_HOST_DEVICE_PLUG_OUT;
        	}
        }
        break;

        default:
        {
            MP_DEBUG("-USBOTG%d- SETUP STATE does not be defined!!", eWhichOtg);
        }
        break;
    }

    pUsbh->sMDevice[bMcardTransferID].swStatus = err;
    
    if (err == USB_NO_ERROR)
    {
        if ((receiveMailDrv.wCurrentExecutionState == SETUP_DEVICE_INIT_STATE)                ||
            //(receiveMailDrv.wCurrentExecutionState == SETUP_DEVICE_INIT_DONE_STATE)           ||
            (receiveMailDrv.wCurrentExecutionState == SETUP_GET_DEVICE_DESCRIPTOR_DONE_STATE) ||
            //(receiveMailDrv.wCurrentExecutionState == SETUP_SET_ADDRESS_DONE_STATE)           ||
            (receiveMailDrv.wCurrentExecutionState == SETUP_GET_CONFIG_DESCRIPTOR_DONE_STATE) ||
            (receiveMailDrv.wCurrentExecutionState == SETUP_GET_HUB_DESCRIPTOR_DONE_STATE)    ||
            (receiveMailDrv.wCurrentExecutionState == SETUP_GET_STATUS_DONE_STATE)            ||
            (receiveMailDrv.wCurrentExecutionState == SETUP_GET_HUB_DESCRIPTOR_DONE_STATE)    ||
            (receiveMailDrv.wCurrentExecutionState == SETUP_GET_STATUS_DONE_STATE)            ||
            (receiveMailDrv.wCurrentExecutionState == SETUP_SET_CONFIG_DONE_STATE))
        {
            SDWORD  osSts;
            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
            if (osSts != OS_STATUS_OK)
            {
                MP_DEBUG("-USBOTG%d- SendMailToUsbOtgHostClassTask failed!!", eWhichOtg);
            }
        }
#if USB_WIFI
        else if (pUsbhDevDes->sDeviceDescriptor.idVendor == (0xce0a) &&
                     (pUsbhDevDes->sDeviceDescriptor.idProduct==(0x1512)))
        {
            /* ar2524 can't do SetupConfiguration before its FW is downloaded.
             * Since we skip SetupConfiguration, we have to send the mail 
             * here.
             */
            if (receiveMailDrv.wCurrentExecutionState == SETUP_SET_CONFIG_STATE)
            {
                SDWORD  osSts;
                osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
                if (osSts != OS_STATUS_OK)
                {
                    MP_DEBUG("-USBOTG%d- SendMailToUsbOtgHostClassTask failed!!", eWhichOtg);
                }
            }
        }
#endif
    }
    else if (err != USB_NO_ERROR)
    {
        MP_DEBUG("-USBOTG%d- SetupSM err = %d", eWhichOtg, err);
    }
}

#if USBOTG_HOST_INTERRUPT_TX_TEST    
///////////////////////////////////////
// BEGIN:TEST Interrupt Pipe         //
///////////////////////////////////////
static void UsbOtgHostBulkOnlyInterruptTestActive(WHICH_OTG eWhichOtg)
{
    Host20_BufferPointerArray_Structure aTemp;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    pUsbhDevDes->psAppClass->bIntDataInLen = \
        pUsbhDevDes->hstInterruptInqHD[pUsbhDevDes->psAppClass->bIntInQHDArrayNum]->bMaxPacketSize;
    pUsbhDevDes->psAppClass->dIntDataBuffer = (DWORD)allo_mem((DWORD)pUsbhDevDes->psAppClass->bIntDataInLen,eWhichOtg);
    memset((BYTE*)pUsbhDevDes->psAppClass->dIntDataBuffer, 0, pUsbhDevDes->psAppClass->bIntDataInLen);
    aTemp.BufferPointerArray[0] = (DWORD)(pUsbhDevDes->psAppClass->dIntDataBuffer);
    aTemp.BufferPointerArray[1] = 0;	
    aTemp.BufferPointerArray[2] = 0;	
    aTemp.BufferPointerArray[3] = 0;
    aTemp.BufferPointerArray[4] = 0;
    MP_DEBUG("INT Active");
    flib_Host20_Issue_Interrupt_Active( pUsbhDevDes->psAppClass->bIntInQHDArrayNum,
                                        pUsbhDevDes->psAppClass->bIntDataInLen, 
                                        (&aTemp.BufferPointerArray[0]), 
                                        0, 
                                        OTGH_DIR_IN,
                                        eWhichOtg);
}

static void UsbOtgHostBulkOnlyInterruptTestIoc(WHICH_OTG eWhichOtg)
{
    BYTE i=0;
    static DWORD cnt = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    if (pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// plug out
    {
        return;
    }
    
    if (pUsbhDevDes->bCmdTimeoutError == 1)
    {
        pUsbhDevDes->bCmdTimeoutError = 0;
        return;
    }

    MP_DEBUG("<INT>");
    flib_Host20_Send_qTD_Done(pUsbhDevDes->hstInterruptInqHD[0], eWhichOtg);
    for (i = 0; i < pUsbhDevDes->psAppClass->bIntDataInLen; i++)
    {
        //MP_DEBUG("dIntDataBuffer[%d] = 0x%x", i, *((BYTE*)(pUsbhDevDes->psAppClass->dIntDataBuffer+i)));
        UartOutValue(*((BYTE*)(pUsbhDevDes->psAppClass->dIntDataBuffer+i)), 2);
    }

    for (i = 0; i < 14; i++, cnt++)
    {
        if (cnt != *((DWORD*)(pUsbhDevDes->psAppClass->dIntDataBuffer)+i))
        {
	    MP_ASSERT(0);
            //__asm("break 100");
        }
    }

    free1(pUsbhDevDes->psAppClass->dIntDataBuffer, eWhichOtg);
}
///////////////////////////////////////
//   END:TEST Interrupt Pipe         //
///////////////////////////////////////
#endif    

#define USBOTG_HOST_WAIT_FOR_MSDC_RESET 10
#define SILICON_VID		0x0457
#define NIKON_VID		0x04B0
//extern DWORD gControlRemainBytes; // For Konica DSC
//static void UsbOtgHostMsdcInitDone(void)

static void UsbOtg0HostMsdcInitDone(void)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    WHICH_OTG eWhichOtg = USBOTG0; 
    
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
	pSendMailDrv->wStateMachine 			= MSDC_INIT_SM;
	pSendMailDrv->wCurrentExecutionState	= MSDC_INIT_GET_MAX_LUN_STATE;
    SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
}

static void UsbOtg1HostMsdcInitDone(void)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    WHICH_OTG eWhichOtg = USBOTG1;
    
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
	pSendMailDrv->wStateMachine 			= MSDC_INIT_SM;
	pSendMailDrv->wCurrentExecutionState	= MSDC_INIT_GET_MAX_LUN_STATE;
    SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
}

static void MsdcInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
    //static BYTE try_times = 0;
    SWORD       err = 0;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    PST_APP_CLASS_DESCRIPTOR    pMsdcDes;
    PUSB_HOST_MSDC              psMsdc;
    PUSB_HOST_EHCI              psEhci;

    //ST_MCARD_MAIL   *pReceiveMailDrv;
    ST_MCARD_MAIL   receiveMailDrv;
    ST_MCARD_MAIL   *pSendMailDrv;
    BYTE  bTransportProtocol;
    BYTE            mail_id;
    WORD            mcard_id = 0;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    pMsdcDes    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);    
    psMsdc      = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);
    psEhci      = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    
#if USBOTG_HOST_DESC
    bTransportProtocol  = GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx);
#else
    bTransportProtocol  = pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol;
#endif

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
        return;
    }

   if( bTransportProtocol == BULK_ONLY_PROTOCOL )
   {
       if (SCSI_REQUEST_SENSE == pMsdcDes->psCbw->u8CB[0])
       {
           if (pMsdcDes->psCsw->u8Status != COMMAND_PASS)
           {   // REQ SENSE Failed; not make sense
                MP_DEBUG("REQ SENSE Failed!! ");
            }
           else
           {   // REQ SENSE Success; pass to next state
                SENSE_DATA  sense_data;
                memcpy((BYTE*)&sense_data, (BYTE*)(pMsdcDes->dDataBuffer), BYTE_COUNT_OF_SENSE_DATA);
                pMsdcDes->dRequestSenseCode[pMsdcDes->bDevLun] = UsbhCheckReqSenseData( sense_data.bSenseKey,
                                                                                        sense_data.bAsc,
                                                                                        sense_data.bAscq);
            }

            free1((void*)pMsdcDes->dDataBuffer, eWhichOtg);
            pMsdcDes->psCbw->u8CB[0]= 0;
         }
    }
    
    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    mcard_id = pSendMailDrv->wMCardId;
    
    pMsdcDes->bDevLun =  GetLunbyCardId(bMcardTransferID);//bMcardTransferID-1;
    MP_DEBUG("MISM USBOTG%d- sts is %d; LUN = %d" , eWhichOtg, receiveMailDrv.wCurrentExecutionState, pMsdcDes->bDevLun);
    
    switch( receiveMailDrv.wCurrentExecutionState)
    {
        case MSDC_INIT_START_STATE:
        {
            pUsbhDevDes->psAppClass = pMsdcDes;
	        MP_DEBUG("Transport protocol: 0x%x",bTransportProtocol);
            OTGH_PT_Bulk_Init(eWhichOtg);
#if USBOTG_HOST_DESC
            if (BULK_ONLY_PROTOCOL ==
                GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))           
#else
            if (BULK_ONLY_PROTOCOL ==
                pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol)
#endif                
            { // Bulk-Only
                psMsdc->boIsMsdcBulkOnly = TRUE;
                pUsbhDevDes->fpAppClassBulkActive   = UsbOtgHostBulkOnlyActive;
                pUsbhDevDes->fpAppClassBulkIoc      = UsbOtgHostBulkOnlyIoc;
#if USBOTG_HOST_INTERRUPT_TX_TEST    
///////////////////////////////////////
// BEGIN:TEST Interrupt Pipe         //
///////////////////////////////////////
                pUsbhDevDes->fpAppClassInterruptActive = UsbOtgHostBulkOnlyInterruptTestActive;
                pUsbhDevDes->fpAppClassInterruptIoc    = UsbOtgHostBulkOnlyInterruptTestIoc;
///////////////////////////////////////
//   END:TEST Interrupt Pipe         //
///////////////////////////////////////
#endif    
                if (pUsbhDevDes->sDeviceDescriptor.idVendor == byte_swap_of_word(SILICON_VID))
                {
                    pSendMailDrv->wCurrentExecutionState = MSDC_INIT_MSDC_RESET_STATE;
                }
                else
                {
                    pSendMailDrv->wCurrentExecutionState = MSDC_INIT_GET_MAX_LUN_STATE;
                }

                pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
                SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
            }
#if USBOTG_HOST_DESC
            else if (CBI_WITHOUT_INT_PROTOCOL ==
                     GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))           
#else
            else if (CBI_WITHOUT_INT_PROTOCOL ==
                     pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol)
#endif             
            { // CB no INT
                psMsdc->boIsMsdcBulkOnly = FALSE;
                pUsbhDevDes->fpAppClassBulkActive   = UsbOtgHostCbNoIntActive;
                pUsbhDevDes->fpAppClassBulkIoc      = UsbOtgHostCbNoIntIoc;
                pUsbhDevDes->fpAppClassSetupIoc     = UsbOtgHostCbNoIntSetupIoc;
                pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_TUR_STATE;
                pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
                pMsdcDes->bMaxLun = 0; // CBI support single LUN only
                SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
            }
#if USBOTG_HOST_DESC
            else if (CBI_PROTOCOL ==
                     GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))           
#else
            else if (CBI_PROTOCOL ==
                     pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol)
#endif             
            { // CB with INT
                psMsdc->boIsMsdcBulkOnly = FALSE;
                pUsbhDevDes->fpAppClassBulkActive       = UsbOtgHostCbWithIntActive;
                pUsbhDevDes->fpAppClassBulkIoc          = UsbOtgHostCbWithIntIoc;
                pUsbhDevDes->fpAppClassInterruptActive  = UsbOtgHostCbWithIntActive;
                pUsbhDevDes->fpAppClassInterruptIoc     = UsbOtgHostCbWithIntIoc;	
                pUsbhDevDes->fpAppClassSetupIoc         = UsbOtgHostCbWithIntSetupIoc;
                pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_INQUIRY_STATE; 
                pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
                pMsdcDes->bMaxLun = 0; // CBI support single LUN only
                SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
            }
        }
        break;

        case MSDC_INIT_MSDC_RESET_STATE:
        {
            //__asm("break 100");
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wMCardId = mcard_id;
            pSendMailDrv->wStateMachine           = MSDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_MSDC_RESET_DONE_STATE;
            err = SetupMsdcReset (eWhichOtg);
            if (err != USB_NO_ERROR)
            { 
                err = USB_SET_EVENT_FAILED;
                //return;
            }
        }
        break;

        case MSDC_INIT_MSDC_RESET_DONE_STATE:
        {
            UsbOtgSetSysTimerProc ( USBOTG_HOST_WAIT_FOR_MSDC_RESET,
                                    UsbOtg0HostMsdcInitDone,
                                    TRUE, 
                                    eWhichOtg);
        }
        break;


        case MSDC_INIT_GET_MAX_LUN_STATE:
        {
            //__asm("break 100");
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wMCardId = mcard_id;
            pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_GET_MAX_LUN_DONE_STATE;
            err = SetupGetMaxLun(eWhichOtg);
            if (err != USB_NO_ERROR)
            { 
                err = USB_SET_EVENT_FAILED;
                //return;
            }
        }
        break;

        case MSDC_INIT_GET_MAX_LUN_DONE_STATE:
        {
            pUsbhDevDes->fpAppClassSetupIoc         = UsbOtgHostBulkOnlySetupIoc;
            pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_TUR_STATE;
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
                pMsdcDes->bMaxLun = 0; // support single LUN
                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                { // device does not support Get Max Lun command
                    // get driver task envelope cause of the sender will be driver task 
                    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                    pSendMailDrv->wMCardId                  = mcard_id;
                    pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
                    pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_TUR_STATE;
                    err = SetupClearFeature(0, eWhichOtg);
                    //if (err != USB_NO_ERROR)
                    //{ 
                        err = USB_STALL_ERROR;
                        //return; // to clear feature after stall,  just return and do not mail to class task
                    //}
                }
                else
                {
                    err = USB_UNKNOW_ERROR;
                    MP_DEBUG("GET_MAX_LUN:error = 0x%x", pUsbhDevDes->bQHStatus);
                }
            }
            else
            {
                if(psEhci->dwControlRemainBytes == 1) // Return No Data  // For Konica DSC
                    pMsdcDes->bMaxLun = 0;
                else
                    pMsdcDes->bMaxLun = *(BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress;
            }

            if (psMsdc->boSupportMultiLun == FALSE)
            {
                pMsdcDes->bMaxLun = 0; // support single LUN
            }

            if (HOST20_Attach_Device_Speed_Full == pUsbhDevDes->bDeviceSpeed)
            {
                MP_DEBUG("Full Speed Mode");
#if 0
                if (GetRetryCount(eWhichOtg) <= 1)
                {
                    MP_DEBUG("set EOF1 = 3 and EOF2 = 0");
                    mwHost20_Misc_EOF2Time_Set(0);
                    mwHost20_Misc_EOF1Time_Set(3);
                }
                else
                { // for the digital camera Panasonic DMC-FX8GT
                    MP_DEBUG("set EOF1 = 0 and EOF2 = 0");
                    ClearRetryCount();
                    mwHost20_Misc_EOF2Time_Set(0); 
                    mwHost20_Misc_EOF1Time_Set(0);
                }
                //mwHost20_Misc_ASYN_SCH_SLPT_Set(3);
#else // retry to find out EOF1 and EOF2 time for full device                
                //UsbOtgHostEof1Eof2Adjustment(eWhichOtg);
#endif
            }
            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);    

            MP_DEBUG("MaxLun %d", pMsdcDes->bMaxLun);
        }
        break;

        case MSDC_INIT_TUR_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wMCardId                  = mcard_id;
            pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_TUR_DONE_STATE;
            err = UsbhMsdcCommand(SCSI_TEST_UNIT_READY, eWhichOtg);
            if (err != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;                
                //return;
            }
        }
        break;

        case MSDC_INIT_TUR_DONE_STATE:
        {
            //__asm("break 100");
            pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_INQUIRY_STATE;
	     if (((pUsbhDevDes->bQHStatus != USB_NO_ERROR)||(pMsdcDes->psCsw->u8Status != COMMAND_PASS))
                                                                                                 &&(bTransportProtocol == BULK_ONLY_PROTOCOL))
            {
                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                pSendMailDrv->wMCardId                  = mcard_id;
                pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
                pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_INQUIRY_STATE;
                RequestSenseDataProcess(eWhichOtg);
                // whatever TUR csw is success or not; pass to next state to get inquiry info
                err = USB_NO_ERROR;                
            }
        }
        break;

        case MSDC_INIT_INQUIRY_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wMCardId                  = mcard_id;
            pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_INQUIRY_DONE_STATE;
	     //__asm("break 100");		
            err = UsbhMsdcCommand(SCSI_INQUIRY, eWhichOtg);
            if (err != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;                
                //return;
            }
        }
        break;

        case MSDC_INIT_INQUIRY_DONE_STATE:
        {
            if (((pUsbhDevDes->bQHStatus != USB_NO_ERROR)||(pMsdcDes->psCsw->u8Status != COMMAND_PASS))
                                                                                                 &&(bTransportProtocol == BULK_ONLY_PROTOCOL))
            {
                RequestSenseDataProcess(eWhichOtg);
                err = USB_CSW_FAILED;
                if (pMsdcDes->bDevLun < pMsdcDes->bMaxLun)
                {   // next LUN init
                    pSendMailDrv->wMCardId++;
                    pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_TUR_STATE;
                    pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
                }
                else
                {   // start CMP timer
                    SetUsbhReadyToReadWrite(eWhichOtg);
                    if (eWhichOtg == USBOTG0)
                    {
                        pSendMailDrv->wMCardId = DEV_USB_HOST_ID1;
                    }
                    else if (eWhichOtg == USBOTG1)
                    {
                        pSendMailDrv->wMCardId = DEV_USBOTG1_HOST_ID1;
                    }
                    else
                    {
                        MP_ALERT("%s USBOTG%d cannot recognized!!", __FUNCTION__, eWhichOtg);
                    }
                    //pSendMailDrv->wMCardId = 1;
                    MP_DEBUG("2 START T");
                    //EventSet(USBOTG_HOST_DRIVER_EVENT, EVENT_POLLING_EACH_LUN);
                    //UsbOtgHostEventPollingEachLunForOneLun();
                    SetCheckMediaPresentTimerFlag(CMP_START_TIMER, eWhichOtg);
                }  
            }
            else
            {
               // INQUIRY Success; pass to next state
                STANDARD_INQUIRY sInquiry;
                memcpy((BYTE*)&sInquiry, (BYTE*)pMsdcDes->dDataBuffer, BYTE_COUNT_OF_INQUIRY_DATA);
                err = ProcessInquiryData(&sInquiry);
                if (err != USB_NO_ERROR)
                {
                    MP_DEBUG1("LUN %d has wrong Inquiry data", pMsdcDes->bDevLun);
                    if (err != PARSE_ERROR_DEVICE_NOT_SUPPORT)
                    {
                        err = USB_NO_ERROR;
                    }
                    else
                    { // this LUN is a virtual CD-ROM, so do nothing
                        ;
                    }
                }

                if (err == USB_NO_ERROR)
                {
                    MP_DEBUG1("Enable card ID: %d", receiveMailDrv.wMCardId);
                    USbhDeviceEnable(receiveMailDrv.wMCardId);
                }
                else
                {
                    USbhDeviceDisable(receiveMailDrv.wMCardId);
                }

                if (err == PARSE_ERROR_DEVICE_NOT_SUPPORT)
                {
                    err = USB_NO_ERROR;
                }

                if (pMsdcDes->bDevLun < pMsdcDes->bMaxLun)
                {   // next LUN init
                    pSendMailDrv->wMCardId++;
                    pSendMailDrv->wCurrentExecutionState    = MSDC_INIT_TUR_STATE;
                    pSendMailDrv->wStateMachine             = MSDC_INIT_SM;
                }
                else
                {   // start CMP timer coming from ISR
                    SetUsbhReadyToReadWrite(eWhichOtg);
                    //pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_ISR_SENDER);
                    if (eWhichOtg == USBOTG0)
                    {
                        pSendMailDrv->wMCardId = DEV_USB_HOST_ID1;
                    }
                    else if (eWhichOtg == USBOTG1)
                    {
                        pSendMailDrv->wMCardId = DEV_USBOTG1_HOST_ID1;
                    }
                    else
                    {
                        MP_ALERT("%s USBOTG%d cannot recognized!!", __FUNCTION__, eWhichOtg);
                    }
                    //pSendMailDrv->wMCardId = 1;
                    MP_DEBUG("3 START T");
                    SetCheckMediaPresentTimerFlag(CMP_START_TIMER, eWhichOtg);
                    if (HOST20_Attach_Device_Speed_Full == pUsbhDevDes->bDeviceSpeed)
                    {
                        //mwHost20_Misc_EOF1Time_Set(0); // for the digital camera Panasonic DMC-FX8GT
                        UsbOtgSetSysTimerProc(USBOTG_HOST_FULL_SPEED_DEVICE_POLLING, UsbOtgHostMsdcPollingForFullSpeed, 1, eWhichOtg);
                        MP_DEBUG("AddTimerProc FS Polling");
                    }
                }  
            }
            
            free1((void*)pMsdcDes->dDataBuffer, eWhichOtg);            
        }
        break;
    }

    pUsbh->sMDevice[bMcardTransferID].swStatus = err;
    
    if (err == USB_NO_ERROR)
    {
        if ((receiveMailDrv.wCurrentExecutionState == MSDC_INIT_START_STATE)           ||
            (receiveMailDrv.wCurrentExecutionState == MSDC_INIT_GET_MAX_LUN_DONE_STATE)||
            (receiveMailDrv.wCurrentExecutionState == MSDC_INIT_TUR_DONE_STATE)        ||
            (receiveMailDrv.wCurrentExecutionState == MSDC_INIT_INQUIRY_DONE_STATE))
        {
            if (receiveMailDrv.wCurrentExecutionState == MSDC_INIT_INQUIRY_DONE_STATE)
            {
                if (GetCheckMediaPresentTimerFlag(eWhichOtg) == CMP_START_TIMER)
                {
                    return;
                }
            }

            if (receiveMailDrv.wCurrentExecutionState == MSDC_INIT_TUR_DONE_STATE)
            {
                if (SCSI_REQUEST_SENSE == pMsdcDes->psCbw->u8CB[0])
                {
                    return;
                }
            }

            SDWORD  osSts;
            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
            if (osSts != OS_STATUS_OK)
            {
                MP_DEBUG("SendMailToUsbOtgHostClassTask failed!!");
            }
        }
    }
    else if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("SetupSM err = %d", err);
    }
}

#if USBOTG_HOST_INTERRUPT_TX_TEST    
int gtt_set_int_evnet = 0;
#endif    
static void MsdcCheckMediaPresent(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
    //static BYTE try_times = 0;
    SWORD       err = 0;
    SWORD       err_req_sense = 0;

    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    PST_APP_CLASS_DESCRIPTOR    pMsdcDes;
    PST_HUB_CLASS_DESCRIPTOR    pHubDes;
    PUSB_HOST_MSDC              psMsdc;    
    ST_MCARD_MAIL   receiveMailDrv;
    ST_MCARD_MAIL   *pSendMailDrv;
    BYTE            mail_id;
    BYTE            bTransportProtocol;
    WORD            mcard_id = 0;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    pMsdcDes    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);    
    pHubDes     = (PST_HUB_CLASS_DESCRIPTOR)UsbOtgHostDevHubDescGet(eWhichOtg);
    psMsdc      = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);    
    
#if USBOTG_HOST_DESC
    bTransportProtocol  = GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx);
#else
    bTransportProtocol  = pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol;
#endif
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
        return;
    }

    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    MP_DEBUG("%s pSendMailDrv->wMCardId = %d", __FUNCTION__, pSendMailDrv->wMCardId);
    if( bTransportProtocol == BULK_ONLY_PROTOCOL) 
    {
        if (SCSI_REQUEST_SENSE == pMsdcDes->psCbw->u8CB[0])
        {
            if (pMsdcDes->psCsw->u8Status != COMMAND_PASS)
            {   // REQ SENSE Failed; not make sense
                MP_DEBUG("REQ SENSE Failed!! ");
            }
            else
            {
                SENSE_DATA  sense_data;
                memcpy((BYTE*)&sense_data, (BYTE*)(pMsdcDes->dDataBuffer), BYTE_COUNT_OF_SENSE_DATA);
                pMsdcDes->dRequestSenseCode[pMsdcDes->bDevLun] = UsbhCheckReqSenseData( sense_data.bSenseKey,
                                                                            sense_data.bAsc,
                                                                            sense_data.bAscq);
                // check next LUN 
                if (MSDC_CMP_TUR_DONE_STATE == receiveMailDrv.wCurrentExecutionState)
                {
                    MP_DEBUG("TUR fial and check next LUN");
                    pSendMailDrv->wMCardId = NextMcardIdCmp(pSendMailDrv->wMCardId,
                                                pMsdcDes->bDevLun,
                                                pMsdcDes->bMaxLun,
                                                eWhichOtg);
                }
            }

            free1((void*)pMsdcDes->dDataBuffer, eWhichOtg);
            pMsdcDes->psCbw->u8CB[0]= 0;
            if (MSDC_CMP_TUR_DONE_STATE == receiveMailDrv.wCurrentExecutionState)
                return;
        }
    }
    else if((bTransportProtocol == CBI_PROTOCOL)&& (*(BYTE *)pUsbhDevDes->sSetupPB.dwDataAddress == SCSI_REQUEST_SENSE))
    {
        SENSE_DATA  sense_data;
        memcpy((BYTE*)&sense_data, (BYTE*)(pMsdcDes->dDataBuffer), BYTE_COUNT_OF_SENSE_DATA);
        pMsdcDes->dRequestSenseCode[pMsdcDes->bDevLun] = UsbhCheckReqSenseData( sense_data.bSenseKey,
                                                                            sense_data.bAsc,
                                                                            sense_data.bAscq);
        free1((void*)pMsdcDes->dDataBuffer, eWhichOtg);
    }   
//    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER);
    mcard_id = pSendMailDrv->wMCardId;

    if(bMcardTransferID > 0)
    {
        pMsdcDes->bDevLun = GetLunbyCardId(bMcardTransferID);// LUN start from 0 // TEST
    }
    else
    {
        mpDebugPrint("Err Check bMcardTransferID %d", bMcardTransferID);
        pMsdcDes->bDevLun = 0;
    }
    
    MP_DEBUG("-USBOTG%d- CMP - stat is %x; DevLun is %d; wMCardId = %d" , eWhichOtg, receiveMailDrv.wCurrentExecutionState, pMsdcDes->bDevLun, pSendMailDrv->wMCardId);
    if (UsbhStorage_GetDriveInstalled(pSendMailDrv->wMCardId) == 1)
    {
        switch( receiveMailDrv.wCurrentExecutionState )
        {
            case MSDC_CMP_START_STATE:
            {
#if (USBOTG_HOST_USBIF || USBOTG_HOST_EYE)
                #if USBOTG_HOST_EYE
                psMsdc->bUsbIfTestSts = USB_IF_TEST_ENABLE;
                #endif       
                if(psMsdc->bUsbIfTestSts != USB_IF_TEST_DISABLE)
                {
                    // Enter the USB-IF Test Mode
                    pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                    pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_USB_IF_TEST_STATE;
                    SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
                }
                else
#endif  //USBOTG_HOST_USBIF
                {
                    pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                    pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_TUR_STATE;
                    SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
                }
            }
            break;

            case MSDC_CMP_TUR_STATE:
            {
                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                pSendMailDrv->wMCardId                  = mcard_id;
                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_TUR_DONE_STATE;
                err = UsbhMsdcCommand(SCSI_TEST_UNIT_READY, eWhichOtg);
                if (err != USB_NO_ERROR)
                {
                    err = USB_SET_EVENT_FAILED;
                    //return;
                }
            }
            break;

            case MSDC_CMP_TUR_DONE_STATE:
            {
                MP_DEBUG("TUR_DONE");
                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_READ_FORMAT_CAPACITIES_STATE;
                if (((pUsbhDevDes->bQHStatus != USB_NO_ERROR)||(pMsdcDes->psCsw->u8Status != COMMAND_PASS))
                                                                                                 &&(bTransportProtocol == BULK_ONLY_PROTOCOL))
                {   // TUR csw return failed, pass to next LUN check
    //////////////////////////////////////////////////////////////   
#ifdef USBOTG_HOST_CDC_HUAWEI_TESTING
                    static int is_sent = 0;
                    if (is_sent == 0)
                    {
                        is_sent = 1;
                        mpDebugPrint("HUAWEI:%s:%d", __FUNCTION__, __LINE__);
                        err = SetupCdcClassCmd(0x01, 0x00, eWhichOtg);  
                        
                        if (err != USB_NO_ERROR)
                        {
                            err = USB_SET_EVENT_FAILED;
                        }
                        SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
                    }
                    else if (is_sent == 1)
                    {
                        SWORD err = USB_NO_ERROR;
                        is_sent = 2;
                        mpDebugPrint("HUAWEI:%s:%d", __FUNCTION__, __LINE__);
                        err = UsbhMsdcCommand(0x11, eWhichOtg);
                        if (err != USB_NO_ERROR)
                        {
                            mpDebugPrint("HUAWEI:%s:%d:error", __FUNCTION__, __LINE__);
                        }
                    }
                    else
                    {
                        mpDebugPrint("HUAWEI:%s:%d", __FUNCTION__, __LINE__);
                    }
                    return;
#endif
    //////////////////////////////////////////////////////////////
                    pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_TUR_DONE_STATE;
                    RequestSenseDataProcess(eWhichOtg);
                    err = USB_CSW_FAILED;
                }
                else
                {   // TUR Success; pass to next state
#if USBOTG_HOST_INTERRUPT_TX_TEST    
///////////////////////////////////////
// BEGIN:TEST Interrupt Pipe         //
///////////////////////////////////////
                    if (gtt_set_int_evnet == 0)
                    {
                        gtt_set_int_evnet = 1;
                        MP_DEBUG("INT event set");
                        EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_INTERRUPT);
                    }
///////////////////////////////////////
//   END:TEST Interrupt Pipe         //
///////////////////////////////////////
#endif
                    if (pMsdcDes->dEventSet[pMsdcDes->bDevLun] == 1)
                    {   // if has sent event to FileSystem, then just repeat TUR
                        pSendMailDrv->wCurrentExecutionState = MSDC_CMP_START_STATE;
                        // pass to next LUN check
                        pSendMailDrv->wMCardId = NextMcardIdCmp(pSendMailDrv->wMCardId,
                                                                pMsdcDes->bDevLun,
                                                                pMsdcDes->bMaxLun,
                                                                eWhichOtg);

                        if (pMsdcDes->bMaxLun == 0) // this msdc device is single LUN, so do not polling
                        {
                            MP_DEBUG("polling TUR");
                            //gNoNeedToPollingUsbDevice = TRUE;
                            //if (mwHost20_USBINTR_Rd() == 0)
                            //    mwHost20_USBINTR_Set(HOST20_USBINTR_ENABLE);
                            return;
                        }
                    }
                    else
                    {   // if not sent event to FileSystem yet, then pass to next state get capacity

                        if (psMsdc->boIsMsdcBulkOnly == TRUE)
                        { // Bulk-Only
                            pSendMailDrv->wCurrentExecutionState = MSDC_CMP_READ_FORMAT_CAPACITIES_STATE;
                        }
                        else
                        { // CBI and CB
                            if(pUsbhDevDes->bQHStatus != USB_NO_ERROR)
                            {
                                RequestSenseDataProcess(eWhichOtg);
                                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                                pSendMailDrv->wCurrentExecutionState = MSDC_CMP_TUR_STATE;
                            }
                            else
                            {    
                        pSendMailDrv->wCurrentExecutionState = MSDC_CMP_READ_CAPACITY_STATE;				   
                            }
                        }
                    }              
                }
            }
            break;

            case MSDC_CMP_READ_FORMAT_CAPACITIES_STATE:
            {
                //__asm("break 100");
                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                pSendMailDrv->wMCardId                  = mcard_id;
                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_READ_FORMAT_CAPACITIES_DONE_STATE;
                err = UsbhMsdcCommand(SCSI_READ_FORMAT_CAPACITIES, eWhichOtg);
                MP_DEBUG("MSDC_CMP_READ_FORMAT_CAPACITIES_STATE:err = %d", err);
                if (err != USB_NO_ERROR)
                {
                    err = USB_SET_EVENT_FAILED;
                    //return;
                }
            }
            break;

            case MSDC_CMP_READ_FORMAT_CAPACITIES_DONE_STATE:
            { 
                if (((pUsbhDevDes->bQHStatus != USB_NO_ERROR)||(pMsdcDes->psCsw->u8Status != COMMAND_PASS))
                                                                                                 &&(bTransportProtocol == BULK_ONLY_PROTOCOL))
                {   // get driver task envelope cause of the sender will be driver task 
                    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                    // try the MSDC_CMP_READ_CAPACITY_STATE
                    pSendMailDrv->wMCardId                  = mcard_id;
                    pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                    pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_READ_CAPACITY_STATE;
                    RequestSenseDataProcess(eWhichOtg);
                    err = USB_CSW_FAILED;
                    //return; // to get the sense data, just return and do not mail to class task
                }
                else
                {
                    // Command Success; parse the data
                    //err = ProcessReadFormatCapacityData();
                    FORMAT_CAPACITY_DES format_capacity;
                    DWORD block_length = 0;
                    memcpy((BYTE*)&format_capacity, (BYTE*)pMsdcDes->dDataBuffer, 12/*BYTE_COUNT_OF_FORMAT_CAPACITY_LEN*/);
                    block_length = (format_capacity.bBlockLength[0] << 16) + (format_capacity.bBlockLength[1] << 8) + format_capacity.bBlockLength[2];                                                                     
                    if ((block_length > 0) && (format_capacity.dLastLBA != 0xffffffff))
                    {
                        WORD temp = 0;
                        DWORD capacity = 0;
                        pUsbh->sMDevice[bMcardTransferID].dwCapacity = format_capacity.dLastLBA;
                        capacity = pUsbh->sMDevice[bMcardTransferID].dwCapacity;
                        if (capacity > MIN_CAPACITY_IN_SECTOR)
                        {
                            temp = pUsbh->sMDevice[bMcardTransferID].wSectorSize = block_length;
                            pMsdcDes->bSectorExp = 0;
                            if (temp == 0)
                            { // if sector size id zero then default is 512 bytes
                                pUsbh->sMDevice[bMcardTransferID].wSectorSize = 512;
                                pMsdcDes->bSectorExp = 9;
                            }                        
                            else
                                while(temp!=1){ temp >>= 1;    pMsdcDes->bSectorExp ++;}

#if USBOTG_HOST_DESC
                            MP_DEBUG("bInterfaceSubClass %x",GetInterfaceSubClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx));
                            if (SFF_8070i == GetInterfaceSubClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
#else
                            if (SFF_8070i == pUsbhDevDes->sInterfaceDescriptor[0].bInterfaceSubClass)
#endif
                            {
                                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_10_STATE;
                                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;                      
                            }
#if USBOTG_HOST_DESC
                            else if (SCSI_TRANS_CMD_SET == GetInterfaceSubClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
#else
                            else if (SCSI_TRANS_CMD_SET == pUsbhDevDes->sInterfaceDescriptor[0].bInterfaceSubClass)
#endif
                            {
                                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_6_STATE;
                                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;                      
                            }
#if USBOTG_HOST_DESC
                            else if ( 0xff == GetInterfaceSubClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
#else
                            else if ( 0xff == pUsbhDevDes->sInterfaceDescriptor[0].bInterfaceSubClass)                       
#endif
                            { // for SONY DSC70 which InterfSubClass 0xff
                                //MP_DEBUG("Error:no suxh as interfaceSubClass");                                
                                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_10_DONE_STATE;
                                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;                      
                            }
                            else
                            {
                                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_10_STATE;
                                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;        
                            }
                        }
                        else
                        { // it's for other purpose.. Disable this LUN and pass to next LUN check
                            MP_DEBUG("1 Disable LUN", pMsdcDes->bDevLun);
                            USbhDeviceDisable(receiveMailDrv.wMCardId);                                
                            pSendMailDrv->wMCardId = NextMcardIdCmp(pSendMailDrv->wMCardId,
                                                                    pMsdcDes->bDevLun,
                                                                    pMsdcDes->bMaxLun,
                                                                    eWhichOtg);
                        }
                    }
                    else if ((format_capacity.dLastLBA == 0xffffffff)&&\
                             (format_capacity.bDescriptorCode == NO_CARTRIDGE_IN_DRIVE))
                    { // pass to next LUN check
                        pSendMailDrv->wMCardId = NextMcardIdCmp(pSendMailDrv->wMCardId,
                                                                pMsdcDes->bDevLun,
                                                                pMsdcDes->bMaxLun,
                                                                eWhichOtg);
                        pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                        pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_READ_CAPACITY_STATE;
                    }
                    else
                    {   // data is wrong; try the MSDC_CMP_READ_CAPACITY_STATE
                        pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_READ_CAPACITY_STATE;
                        pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                    }
                }
                
                free1((void*)pMsdcDes->dDataBuffer, eWhichOtg);            
            }
            break;

            case MSDC_CMP_READ_CAPACITY_STATE:
            {
                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                pSendMailDrv->wMCardId                  = mcard_id;
                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_READ_CAPACITY_DONE_STATE;
                err = UsbhMsdcCommand(SCSI_READ_CAPACITY, eWhichOtg);
                MP_DEBUG("MSDC_CMP_READ_CAPACITY_STATE:err = %d", err);
                if (err != USB_NO_ERROR)
                {
                    err = USB_SET_EVENT_FAILED;
                    //return;
                }
            }
            break;
            
            case MSDC_CMP_READ_CAPACITY_DONE_STATE:
            {
                if (((pUsbhDevDes->bQHStatus != USB_NO_ERROR)||(pMsdcDes->psCsw->u8Status != COMMAND_PASS))
                                                                                                 &&(bTransportProtocol == BULK_ONLY_PROTOCOL))
                {   // pass to next LUN check
                    //__asm("break 100");
                    pSendMailDrv->wMCardId = NextMcardIdCmp(pSendMailDrv->wMCardId,
                                                            pMsdcDes->bDevLun,
                                                            pMsdcDes->bMaxLun,
                                                            eWhichOtg);
                    RequestSenseDataProcess(eWhichOtg);
                    err = USB_CSW_FAILED;
                    //return; // to get the sense data, just return and do not mail to class task
                }
		        else	
                {   // Command Success; parse the data
                    READ_CAPACITY capacity;
                    memcpy((BYTE*)&capacity, (BYTE*)pMsdcDes->dDataBuffer, BYTE_COUNT_OF_READ_CAPACITY_LEN);
                    if (capacity.dBlockLenInByte > 0)
                    {
                        WORD temp = 0;
                        DWORD dwCapacity = 0;
                        pUsbh->sMDevice[bMcardTransferID].dwCapacity = capacity.dLastLogicBlockAddr + 1;
                        dwCapacity = pUsbh->sMDevice[bMcardTransferID].dwCapacity;
                        if (dwCapacity > MIN_CAPACITY_IN_SECTOR)
                        {
                            temp = pUsbh->sMDevice[bMcardTransferID].wSectorSize = capacity.dBlockLenInByte;
                            pMsdcDes->bSectorExp = 0;
                            if (temp == 0)
                            { // if sector size id zero then default is 512 bytes
                                pUsbh->sMDevice[bMcardTransferID].wSectorSize = 512;
                                pMsdcDes->bSectorExp = 9;
                            }                        
                            else
                                while(temp!=1){ temp >>= 1;    pMsdcDes->bSectorExp ++;}

#if USBOTG_HOST_DESC
                            if (SFF_8070i == GetInterfaceSubClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
#else
                            if (SFF_8070i == pUsbhDevDes->sInterfaceDescriptor[0].bInterfaceSubClass)
#endif
                            {
                                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_10_STATE;
                                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;                      
                            }
#if USBOTG_HOST_DESC
                            else if (SCSI_TRANS_CMD_SET == GetInterfaceSubClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
#else
                            else if (SCSI_TRANS_CMD_SET == pUsbhDevDes->sInterfaceDescriptor[0].bInterfaceSubClass)
#endif                            
                            {
                                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_6_STATE;
                                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;                      
                            }
#if USBOTG_HOST_DESC
                            else if (0xff == GetInterfaceSubClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
#else
                            else if (0xff == pUsbhDevDes->sInterfaceDescriptor[0].bInterfaceSubClass)
#endif
                            { // for CBI                          
                                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_10_DONE_STATE;
                                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;                      
                            }
                            else
                            { // for SONY DSC70 which InterfSubClass 0xff
                                //MP_DEBUG("Error:no suxh as interfaceSubClass");
                                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_10_STATE;
                                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;                      
                            }
                        }
                        else
                        { // it's for other purpose. Disable this LUN and pass to next LUN check
                            MP_DEBUG("2 Disable LUN", pMsdcDes->bDevLun);
                            USbhDeviceDisable(receiveMailDrv.wMCardId);                                
                            pSendMailDrv->wMCardId = NextMcardIdCmp(pSendMailDrv->wMCardId,
                                                                    pMsdcDes->bDevLun,
                                                                    pMsdcDes->bMaxLun,
                                                                    eWhichOtg);
                        }
                    }
                }
                
                free1((void*)pMsdcDes->dDataBuffer, eWhichOtg);            
            }
            break;

            case MSDC_CMP_MODE_SENSE_6_STATE:
            {
                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                pSendMailDrv->wMCardId                  = mcard_id;
                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_6_DONE_STATE;
                err = UsbhMsdcCommand(SCSI_MODE_SENSE_6, eWhichOtg);
                MP_DEBUG("MSDC_CMP_MODE_SENSE_6_STATE:err = %d", err);
                if (err != USB_NO_ERROR)
                {
                    err = USB_SET_EVENT_FAILED;
                    //return;
                }
            }
            break;
            
            case MSDC_CMP_MODE_SENSE_6_DONE_STATE:
            {            
                //static BYTE try_cnt = 0;
                if (((pUsbhDevDes->bQHStatus != USB_NO_ERROR)||(pMsdcDes->psCsw->u8Status != COMMAND_PASS))
                                                                                                 &&(bTransportProtocol == BULK_ONLY_PROTOCOL))
                {
                    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                    pSendMailDrv->wMCardId                  = mcard_id;
                    pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                    pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_6_DONE_STATE;
                    psMsdc->bModeSense6RetryCnt++;
                    if (psMsdc->bModeSense6RetryCnt < 3)
                    { // try the MSDC_CMP_MODE_SENSE_6_STATE with different length
                        pSendMailDrv->wCurrentExecutionState = MSDC_CMP_MODE_SENSE_6_STATE;
                        RequestSenseDataProcess(eWhichOtg);
                        err = USB_CSW_FAILED;
                    }
                    else
                    {
                        MP_DEBUG("should be impossible goto here!!");
                        psMsdc->bModeSense6RetryCnt = 0;
                        pSendMailDrv->wCurrentExecutionState = MSDC_CMP_DONE_STATE;
                        RequestSenseDataProcess(eWhichOtg);
                        err = USB_NO_ERROR;
                        pUsbh->sMDevice[bMcardTransferID].Flag.ReadOnly = 1;
						if (pMsdcDes->dEventSet[pMsdcDes->bDevLun] == 0)
						{  // set event to main to mount
							Ui_SetCardInCodeByLun(pMsdcDes->bDevLun, eWhichOtg);
							pMsdcDes->dEventSet[pMsdcDes->bDevLun] = 1;
							pMsdcDes->dPresent[pMsdcDes->bDevLun] = 1;
						}
                    
						// pass to next LUN check by polling from ISR
						WORD card_id = 0;
						card_id = pSendMailDrv->wMCardId;
						pSendMailDrv->wMCardId = NextMcardIdCmp(card_id,
                                                            pMsdcDes->bDevLun,
                                                            pMsdcDes->bMaxLun,
                                                            eWhichOtg);
                    }
                    //return; // to get the sense data, just return and do not mail to class task
                }
                else
                {
                    psMsdc->bModeSense6RetryCnt = 0;
                    // Command success; parse the data to get the WriteProtect info
                    MODE_SENSE_6_HDR mode_sense6_hdr;
                    memcpy((BYTE*)&mode_sense6_hdr, (BYTE*)pMsdcDes->dDataBuffer, sizeof(MODE_SENSE_6_HDR));
                    if (mode_sense6_hdr.bDevPara == MSDC_DEVICE_WRITE_PROTECT)
                    {
                        pUsbh->sMDevice[bMcardTransferID].Flag.ReadOnly = 1;
                    }
                    else
                    {
                        pUsbh->sMDevice[bMcardTransferID].Flag.ReadOnly = 0;
                    }                    
                    

                    if (pMsdcDes->dEventSet[pMsdcDes->bDevLun] == 0)
                    {  // set event to main to mount
                        Ui_SetCardInCodeByLun(pMsdcDes->bDevLun, eWhichOtg);
                        pMsdcDes->dEventSet[pMsdcDes->bDevLun] = 1;
                        pMsdcDes->dPresent[pMsdcDes->bDevLun] = 1;
                    }
                    
                    // pass to next LUN check by polling from ISR
                    WORD card_id = 0;
                    card_id = pSendMailDrv->wMCardId;
                    //pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_ISR_SENDER);
                    pSendMailDrv->wMCardId = NextMcardIdCmp(card_id,
                                                            pMsdcDes->bDevLun,
                                                            pMsdcDes->bMaxLun,
                                                            eWhichOtg);
                }
                
                free1((void*)pMsdcDes->dDataBuffer, eWhichOtg);            
            }
            break;        

            case MSDC_CMP_MODE_SENSE_10_STATE:
            {
                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                pSendMailDrv->wMCardId                  = mcard_id;
                pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
                pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_MODE_SENSE_10_DONE_STATE;
                err = UsbhMsdcCommand(SCSI_MODE_SENSE_10, eWhichOtg);
                if (err != USB_NO_ERROR)
                {
                    err = USB_SET_EVENT_FAILED;
                    //return;
                }
            }
            break;
            
            case MSDC_CMP_MODE_SENSE_10_DONE_STATE:
            {            
                if (((pUsbhDevDes->bQHStatus != USB_NO_ERROR)||(pMsdcDes->psCsw->u8Status != COMMAND_PASS))
                                                                                                 &&(bTransportProtocol == BULK_ONLY_PROTOCOL))
                {
                    //__asm("break 100");
                    RequestSenseDataProcess(eWhichOtg);
                    err = USB_CSW_FAILED;
                    //return; // to get the sense data, just return and do not mail to class task
                }
#if USBOTG_HOST_DESC
                else if ( 0xff == GetInterfaceSubClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
#else
                else if( 0xff == pUsbhDevDes->sInterfaceDescriptor[0].bInterfaceSubClass )
#endif
                {
                    pSendMailDrv->wStateMachine = MSDC_CHECK_MEDIA_PRESENT_SM;
                    pSendMailDrv->wCurrentExecutionState = MSDC_CMP_DONE_STATE;
                    if (pMsdcDes->dEventSet[pMsdcDes->bDevLun] == 0)
                    {
                        // set event to main to mount
                        Ui_SetCardInCodeByLun(pMsdcDes->bDevLun, eWhichOtg);
                        //EventSet(UI_EVENT, EVENT_CARD_IN); //Athena 03.11.2006 seperate card in & out
                        pMsdcDes->dEventSet[pMsdcDes->bDevLun] = 1;
                        pMsdcDes->dPresent[pMsdcDes->bDevLun] = 1;
                    }
                    
                    // pass to next LUN check by polling from ISR
                    WORD card_id = 0;
                    card_id = pSendMailDrv->wMCardId;
                    //pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_ISR_SENDER);
                    pSendMailDrv->wMCardId = NextMcardIdCmp(card_id,
                                                            pMsdcDes->bDevLun,
                                                            pMsdcDes->bMaxLun,
                                                            eWhichOtg);

                }
                else
                { 
                   // Command success; parse the data to get the WriteProtect info
                    MODE_SENSE_10_HDR mode_sense10_hdr;
                    memcpy((BYTE*)&mode_sense10_hdr, (BYTE*)pMsdcDes->dDataBuffer, BYTE_COUNT_OF_MODE_SENSE_10_LEN);
                    if (mode_sense10_hdr.bDevPara == MSDC_DEVICE_WRITE_PROTECT)
                    {
                        pUsbh->sMDevice[pMsdcDes->bDevLun].Flag.ReadOnly = 1;
                    }
                    else
                    {
                        pUsbh->sMDevice[pMsdcDes->bDevLun].Flag.ReadOnly = 0;
                    }
                   //err = USB_NO_ERROR;
                    pSendMailDrv->wStateMachine = MSDC_CHECK_MEDIA_PRESENT_SM;
                    pSendMailDrv->wCurrentExecutionState = MSDC_CMP_DONE_STATE;
                    if (pMsdcDes->dEventSet[pMsdcDes->bDevLun] == 0)
                    {
                        // set event to main to mount
                        Ui_SetCardInCodeByLun(pMsdcDes->bDevLun, eWhichOtg);
                        //EventSet(UI_EVENT, EVENT_CARD_IN); //Athena 03.11.2006 seperate card in & out
                        pMsdcDes->dEventSet[pMsdcDes->bDevLun] = 1;
                        pMsdcDes->dPresent[pMsdcDes->bDevLun] = 1;
                    }
                    
                    // pass to next LUN check by polling from ISR
                    WORD card_id = 0;
                    card_id = pSendMailDrv->wMCardId;
                    //pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_ISR_SENDER);
                    pSendMailDrv->wMCardId = NextMcardIdCmp(card_id,
                                                            pMsdcDes->bDevLun,
                                                            pMsdcDes->bMaxLun,
                                                            eWhichOtg);
                }
                free1((void*)pMsdcDes->dDataBuffer, eWhichOtg);             
            }
            break;        

#if (USBOTG_HOST_USBIF || USBOTG_HOST_EYE)
            case MSDC_CMP_USB_IF_TEST_STATE:
            {
                MP_ALERT("USB-IF Test Mode");
                // Disable MsdcCheckMediaPresent Loop

                // USB-IF Status Ready
                SetUsbIfTestMode(USB_IF_TEST_READY, eWhichOtg);

                #if USBOTG_HOST_EYE
                SetUsbIfTestMode(USB_IF_TEST_PACKET, eWhichOtg);
                #endif
            }
            break;
#endif  //USBOTG_HOST_USBIF
        }
    }
    else // -- if (UsbhStorage_GetDriveInstalled(bMcardTransferID+pUsbh->bIdOffsetValue) == 1)
    {
        MP_DEBUG("check LUN:%d; MaxLun = %d", GetLunbyCardId(pSendMailDrv->wMCardId), pMsdcDes->bMaxLun);
        err = USB_NO_ERROR;
        if (pMsdcDes->bMaxLun > GetLunbyCardId(pSendMailDrv->wMCardId))
        { // pass to next LUN check
            pSendMailDrv->wMCardId = NextMcardIdCmp(pSendMailDrv->wMCardId,
                                                    pMsdcDes->bDevLun,
                                                    pMsdcDes->bMaxLun,
                                                    eWhichOtg);
            MP_DEBUG1("check next LUN:%d",GetLunbyCardId(pSendMailDrv->wMCardId));
        }
        else
        {
            
            MP_DEBUG("no more LUN to check");
            //while(1); // for USBOTg1 debug
            if (pHubDes->sHubDescriptor.bNbrPorts == 0)
            {
                //pSendMailDrv->wMCardId = NextMcardIdCmp(pSendMailDrv->wMCardId,
                //                                        pMsdcDes->bDevLun,
                //                                        pMsdcDes->bMaxLun);
                //if ((pSendMailDrv->wMCardId-1) > pMsdcDes->bMaxLun)
                {
                    err = USB_NO_ERROR;
                    if (eWhichOtg == USBOTG0)
                    {
                        pSendMailDrv->wMCardId = DEV_USB_HOST_ID1;
                    }
                    else if (eWhichOtg == USBOTG1)
                    {
                        pSendMailDrv->wMCardId = DEV_USBOTG1_HOST_ID1;
                    }
                    else
                    {
                        MP_ALERT("%s USBOTG%d cannot recognized!!", __FUNCTION__, eWhichOtg);
                    }
                    //pSendMailDrv->wMCardId = 1;
                    pSendMailDrv->wStateMachine = MSDC_CHECK_MEDIA_PRESENT_SM;
                    pSendMailDrv->wCurrentExecutionState = MSDC_CMP_START_STATE;
                    MP_DEBUG("polling LUN from 0");
                }
            }
            else
            {
                PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDestmp;
                SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);

                pUsbhDevDes->bMaxInterruptInEpNumber  = pHubDes->bMaxInterruptInEpNumber;
                pUsbhDevDes->hstInterruptInqHD        = pHubDes->hstInterruptInqHD;
                pUsbhDevDes->hstIntInWorkqTD          = pHubDes->hstIntInWorkqTD;
                pUsbhDevDes->hstIntInSendLastqTD      = pHubDes->hstIntInSendLastqTD;
                pUsbhDevDes->hstInterruptInqTD        = pHubDes->hstInterruptInqTD;
                pUsbhDevDes->fpAppClassInterruptActive  = pHubDes->fpAppClassInterruptActive;
                pUsbhDevDes->fpAppClassInterruptIoc     = pHubDes->fpAppClassInterruptIoc;
                
                pHubDes->bPortNumber++;
                MP_DEBUG("check next port:%d", pHubDes->bPortNumber);
                if (pHubDes->bPortNumber <= USB_HUB_MAX_PORT_NUM)
                {
                    pSendMailDrv->wStateMachine = SETUP_HUB_SM;
                    pSendMailDrv->wCurrentExecutionState = SETUP_HUB_INIT_START_STATE;
                    UsbOtgHostSetAddressForUsbAddressState(pHubDes->bDeviceAddress,pUsbhDevDes);
                }
                else
                {
                    MP_DEBUG("over the max port number we supported!");
                }
            }            
        }
    }

    pUsbh->sMDevice[bMcardTransferID].swStatus = err;
    if (err == USB_NO_ERROR)
    {
        if ((receiveMailDrv.wCurrentExecutionState == MSDC_CMP_START_STATE)                         ||
            (receiveMailDrv.wCurrentExecutionState == MSDC_CMP_TUR_DONE_STATE)                      ||
            (receiveMailDrv.wCurrentExecutionState == MSDC_CMP_READ_FORMAT_CAPACITIES_DONE_STATE)   ||
            (receiveMailDrv.wCurrentExecutionState == MSDC_CMP_READ_CAPACITY_DONE_STATE))            //||
            //(receiveMailDrv.wCurrentExecutionState == MSDC_CMP_MODE_SENSE_6_DONE_STATE)             ||
            //(receiveMailDrv.wCurrentExecutionState == MSDC_CMP_MODE_SENSE_10_DONE_STATE))
        {
            if ((receiveMailDrv.wCurrentExecutionState == MSDC_CMP_TUR_DONE_STATE)&&\
                (pSendMailDrv->wCurrentExecutionState == MSDC_CMP_START_STATE))
            { // will be called by timer in OTG ISR
                return; 
            }
            
            SendMailToUsbOtgHostClassTask((BYTE*)&pSendMailDrv->dwBlockAddr,eWhichOtg);
        }
    }
    else if (err == USB_CSW_FAILED)
    {
        if (pSendMailDrv->wCurrentExecutionState == MSDC_CMP_READ_CAPACITY_STATE ||
            pSendMailDrv->wCurrentExecutionState == MSDC_CMP_MODE_SENSE_6_STATE)
        {
            if (SCSI_REQUEST_SENSE == pMsdcDes->psCbw->u8CB[0])
            {
                return;
            }
            
            SendMailToUsbOtgHostClassTask((BYTE*)&pSendMailDrv->dwBlockAddr,eWhichOtg);
        }

        if (pMsdcDes->dEventSet[pMsdcDes->bDevLun] == 1 &&
            pMsdcDes->dRequestSenseCode[pMsdcDes->bDevLun] != USBH_NO_SENSE)
        {   // set event to main to un-mount
            Ui_SetCardOutCodeByLun(pMsdcDes->bDevLun, eWhichOtg);
            pMsdcDes->dEventSet[pMsdcDes->bDevLun]          = 0;
            pMsdcDes->dRequestSenseCode[pMsdcDes->bDevLun]  = USBH_NO_SENSE;
            pMsdcDes->dPresent[pMsdcDes->bDevLun]           = 0;
        }
    }
    else if (err != USB_NO_ERROR)
    {
        if (pMsdcDes->dEventSet[pMsdcDes->bDevLun] == 1)
        {  // set event to main to un-mount
            Ui_SetCardOutCodeByLun(pMsdcDes->bDevLun, eWhichOtg);
            pMsdcDes->dEventSet[pMsdcDes->bDevLun] = 0;
            pMsdcDes->dPresent[pMsdcDes->bDevLun] = 0;
        }
    }    
}

#if USBOTG_HOST_PTP
static void PtpFillStorageInfo (PSTI_STORAGE_INFO pStoregeInfo, BYTE *pBuf)
{
    pStoregeInfo->storageType                   = uchar_to_ushort(pBuf[1],pBuf[0]);
    pStoregeInfo->filesystemType                = uchar_to_ushort(pBuf[3],pBuf[2]);
    pStoregeInfo->accessCapability              = uchar_to_ushort(pBuf[5],pBuf[4]);
    pStoregeInfo->maxCapability.loLong          = uchar_to_ulong(pBuf[9],pBuf[8],pBuf[7],pBuf[6]);
    pStoregeInfo->maxCapability.hiLong          = uchar_to_ulong(pBuf[13],pBuf[12],pBuf[11],pBuf[10]);
    pStoregeInfo->freeSpaceInBytes.loLong       = uchar_to_ulong(pBuf[17],pBuf[16],pBuf[15],pBuf[14]);
    pStoregeInfo->freeSpaceInBytes.hiLong       = uchar_to_ulong(pBuf[21],pBuf[20],pBuf[19],pBuf[18]);
    pStoregeInfo->freeSpaceInImages             = uchar_to_ulong(pBuf[25],pBuf[24],pBuf[23],pBuf[22]);
    pStoregeInfo->storageDescription.numChars   = pBuf[26];
    pStoregeInfo->volumeLable.numChars          = pBuf[27];
}

static void UsbOtg_0_SidcInitDone (void)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, USBOTG0);
    pSendMailDrv->wStateMachine             = SIDC_INIT_SM;
    pSendMailDrv->wCurrentExecutionState    = SIDC_INIT_START_STATE_DONE;
    SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,USBOTG0);
}

static void UsbOtg_1_SidcInitDone (void)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, USBOTG1);
    pSendMailDrv->wStateMachine             = SIDC_INIT_SM;
    pSendMailDrv->wCurrentExecutionState    = SIDC_INIT_START_STATE_DONE;
    SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,USBOTG1);
}

static void SidcInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
	PUSB_HOST_SIDC psSidc = (PUSB_HOST_SIDC)UsbOtgHostSidcDsGet(eWhichOtg);
	PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    ST_MCARD_MAIL   *pReceiveMailDrv;
    ST_MCARD_MAIL   *pSendMailDrv;
    WORD	response_code = 0;
    SWORD	err = 0;
	SDWORD	osSts;
    BYTE	mail_id;

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
        return;
    }
    pReceiveMailDrv = pUsbh->sMDevice[bMcardTransferID].sMcardRMail;
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);

    psHost->psAppClassDescriptor->bDevLun = GetLunbyCardId(bMcardTransferID);

    switch( pReceiveMailDrv->wCurrentExecutionState )
    {
        case SIDC_INIT_START_STATE:
			MP_DEBUG("SidcInitStateMachine: SIDC_INIT_START_STATE");
        break;

        case SIDC_INIT_GET_DEVICE_INFO_STATE:
			MP_DEBUG("SidcInitStateMachine: SIDC_INIT_GET_DEVICE_INFO_STATE");
        break;

		case SIDC_INIT_GET_DEVICE_INFO_DONE_STATE:
			MP_DEBUG("SidcInitStateMachine: SIDC_INIT_GET_DEVICE_INFO_DONE_STATE");
		break;

        case SIDC_INIT_OPEN_SESSION_STATE:
			MP_DEBUG("SidcInitStateMachine: SIDC_INIT_OPEN_SESSION_STATE");
        break;

        case SIDC_INIT_OPEN_SESSION_DONE_STATE:
			MP_DEBUG("SidcInitStateMachine: SIDC_INIT_OPEN_SESSION_DONE_STATE");
        break;
   
        case SIDC_INIT_DONE_STATE:
			MP_DEBUG("SidcInitStateMachine: SIDC_INIT_DONE_STATE");
        break;
    }

    switch( pReceiveMailDrv->wCurrentExecutionState )
    {
        case SIDC_INIT_START_STATE:
        {// wait for device ready to transact SIDC; 600ms is the same as PC.
        	if (eWhichOtg == USBOTG0)
            	SysTimerProcAdd(600, UsbOtg_0_SidcInitDone, 1);
			else if (eWhichOtg == USBOTG1)
            	SysTimerProcAdd(600, UsbOtg_1_SidcInitDone, 1);
        }
        break;

        case SIDC_INIT_START_STATE_DONE:
        {
#if USBOTG_HOST_DESC
            if (GetInterfaceProtocol(psHost->psUsbhDeviceDescriptor, psHost->psUsbhDeviceDescriptor->bDeviceConfigIdx, psHost->psUsbhDeviceDescriptor->bDeviceInterfaceIdx, AlterSettingDefaultIdx)
				== PIMA_15740_BULK_ONLY_PROTOCOL)
#else
            if (psHost->psUsbhDeviceDescriptor->sInterfaceDescriptor[psHost->psUsbhDeviceDescriptor->bDeviceInterfaceIdx].bInterfaceProtocol 
				== PIMA_15740_BULK_ONLY_PROTOCOL)
#endif              
            { // PIMA 15740 Compliant Bulk-Only
   	            psSidc->dwTransactionId = 0;
	            psHost->psUsbhDeviceDescriptor->psAppClass = psHost->psAppClassDescriptor;
    	        OTGH_PT_Bulk_Init(eWhichOtg);

                psHost->psUsbhDeviceDescriptor->fpAppClassBulkActive   = UsbOtgHostSidcBulkOnlyActive;
                psHost->psUsbhDeviceDescriptor->fpAppClassBulkIoc      = UsbOtgHostSidcBulkOnlyIoc;

                if (eWhichOtg == USBOTG0)
                    psHost->psAppClassDescriptor->bDevLun = DEV_USB_HOST_PTP-1;
                else if (eWhichOtg == USBOTG1)
                    psHost->psAppClassDescriptor->bDevLun = DEV_USBOTG1_HOST_PTP-1;
                else
                    MP_ALERT("%s USBOTG%d cannot be recognized!!", __FUNCTION__, eWhichOtg);
                //psHost->psAppClassDescriptor->bDevLun = DEV_USB_HOST_PTP-1;
                psHost->psAppClassDescriptor->dFolderObjectHandlesIdx = 0;
                psHost->psAppClassDescriptor->dFolderObjectHandles[0] = GET_OBJECT_IN_THE_ROOT;
                pSendMailDrv->wCurrentExecutionState    = SIDC_INIT_GET_DEVICE_INFO_STATE;
                pSendMailDrv->wStateMachine             = SIDC_INIT_SM;
            }
            else 
            {
                MP_DEBUG("SidcInitStateMachine: protocol is not supported!");// protocol is not supported
                err = USB_HOST_NOT_SUPPORTED_DEVICE;
            }
        }
        break;

        case SIDC_INIT_GET_DEVICE_INFO_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = SIDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = SIDC_INIT_GET_DEVICE_INFO_DONE_STATE;
            err = UsbOtgHostSidcCommand(psHost->psUsbhDeviceDescriptor, psHost->psAppClassDescriptor, OPERATION_GET_DEVICE_INFO, eWhichOtg);     
            if (err != USB_NO_ERROR)
            {
                mpDebugPrint("SidcInitStateMachine: SIDC_INIT_GET_DEVICE_INFO_STATE err %x" , err);
            }
        }
        break;

        case SIDC_INIT_GET_DEVICE_INFO_DONE_STATE:
		{
            if (psHost->psUsbhDeviceDescriptor->bQHStatus != USB_NO_ERROR)
            {
                mpDebugPrint("SidcInitStateMachine: SIDC_INIT_GET_DEVICE_INFO_DONE_STATE err %x" , psHost->psUsbhDeviceDescriptor->bQHStatus);
            }
            else
            {
                response_code = byte_swap_of_word(psHost->psAppClassDescriptor->sStiResponse.code);
                if (response_code == RESPONSE_OK)
                {
                    USbhDeviceEnable(eWhichOtg ? DEV_USBOTG1_HOST_PTP : DEV_USB_HOST_PTP);                        
                    pSendMailDrv->wCurrentExecutionState    = SIDC_INIT_OPEN_SESSION_STATE;
                    pSendMailDrv->wStateMachine             = SIDC_INIT_SM;
                }
                else
                {
                	mpDebugPrint("SidcInitStateMachine: SIDC_INIT_GET_DEVICE_INFO_DONE_STATE response code %x", response_code);
					err = USB_UNKNOW_ERROR;
                }
            }
        }
		break;

        case SIDC_INIT_OPEN_SESSION_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = SIDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = SIDC_INIT_OPEN_SESSION_DONE_STATE;
            err = UsbOtgHostSidcCommand(psHost->psUsbhDeviceDescriptor, psHost->psAppClassDescriptor, OPERATION_OPEN_SESSION, eWhichOtg);     
            if (err != USB_NO_ERROR)
            {
                mpDebugPrint("SidcInitStateMachine: SIDC_INIT_OPEN_SESSION_STATE err %x" , err);
            }

        }
        break;

        case SIDC_INIT_OPEN_SESSION_DONE_STATE:
		{
            if (psHost->psUsbhDeviceDescriptor->bQHStatus != USB_NO_ERROR)
            {
                mpDebugPrint("SidcInitStateMachine: SIDC_INIT_OPEN_SESSION_DONE_STATE err %x" , psHost->psUsbhDeviceDescriptor->bQHStatus);
            }
            else
            {
                response_code = byte_swap_of_word(psHost->psAppClassDescriptor->sStiResponse.code);
                if (response_code == RESPONSE_OK)
                {
                    pSendMailDrv->wCurrentExecutionState    = SIDC_INIT_DONE_STATE;
                    pSendMailDrv->wStateMachine             = SIDC_INIT_SM;

                    if (eWhichOtg == USBOTG0)
                        psHost->psAppClassDescriptor->bDevLun = DEV_USB_HOST_PTP-1;
                    else if (eWhichOtg == USBOTG1)
                        psHost->psAppClassDescriptor->bDevLun = DEV_USBOTG1_HOST_PTP-1;
                    else
                        MP_ALERT("%s USBOTG%d cannot be recognized!!", __FUNCTION__, eWhichOtg);
                    
                    if (psHost->psAppClassDescriptor->dEventSet[psHost->psAppClassDescriptor->bDevLun] == 0)
                    {   // set event to main to mount
                        Ui_SetCardInCodeByLun(psHost->psAppClassDescriptor->bDevLun, eWhichOtg);
                        psHost->psAppClassDescriptor->dEventSet[psHost->psAppClassDescriptor->bDevLun] = 1;
                        psHost->psAppClassDescriptor->dPresent[psHost->psAppClassDescriptor->bDevLun] = 1;
                    }
                }
                else
                {
                	mpDebugPrint("SidcInitStateMachine: SIDC_INIT_OPEN_SESSION_DONE_STATE response code %x", response_code);
					err = USB_UNKNOW_ERROR;
                }
            }
        }
		break;

		case SIDC_INIT_DONE_STATE:
        {
            SetUsbhReadyToReadWrite(eWhichOtg);
			return;
        }
        break;
		default:
			err = USB_UNKNOW_ERROR;
		break;
    }
	if ((pSendMailDrv->wCurrentExecutionState == SIDC_INIT_GET_DEVICE_INFO_STATE) ||
		(pSendMailDrv->wCurrentExecutionState == SIDC_INIT_OPEN_SESSION_STATE) ||
		(pSendMailDrv->wCurrentExecutionState == SIDC_INIT_DONE_STATE))
	{
		osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
		if (osSts != OS_STATUS_OK)
		{
			mpDebugPrint("SidcInitStateMachine: SendMailToUsbOtgHostClassTask failed!!");
		}
	}

	pUsbh->sMDevice[bMcardTransferID].swStatus = err;
	if (err != USB_NO_ERROR)
	{
		mpDebugPrint("SidcInitStateMachine: err %x", err);
		UsbHostFinal(err, eWhichOtg);
	}
}

static void SidcScanObjectsStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc = (PUSB_HOST_SIDC)UsbOtgHostSidcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR pSidcDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    ST_MCARD_MAIL   *pReceiveMailDrv;
    ST_MCARD_MAIL   *pSendMailDrv;
    SWORD       err = 0;
    WORD        response_code = 0;
    BYTE            mail_id;

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
        return;
    }

    pReceiveMailDrv = pUsbh->sMDevice[bMcardTransferID].sMcardRMail;
    if (pReceiveMailDrv->wCmd == FOR_USBH_PTP_SCAN_OBJECT_NOT_MCARD_CMD)
    {
        psSidc->stScanObject.boIsSearchFileInfo = TRUE;
        psSidc->stScanObject.pReceiveMailFmFs    = (PUSBH_PTP_MAIL_TAG) pReceiveMailDrv;
        psSidc->stScanObject.pdwExtArray  = (DWORD*) psSidc->stScanObject.pReceiveMailFmFs->dwBuffer;
        psSidc->stScanObject.pdwRecordElement    = (DWORD*) psSidc->stScanObject.pReceiveMailFmFs->dwRecordElementAddr;
    }

    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
    
    pSidcDes->bDevLun =  GetLunbyCardId(bMcardTransferID);

    switch( pReceiveMailDrv->wCurrentExecutionState )
    {
        case SIDC_SO_START_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_START_STATE");
        break;
        case SIDC_SO_GET_STORAGE_ID_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_GET_STORAGE_ID_STATE");
        break;
        case SIDC_SO_GET_STORAGE_ID_DONE_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_GET_STORAGE_ID_DONE_STATE");
        break;
		case SIDC_SO_GET_STORAGE_INFO_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_GET_STORAGE_INFO_STATE");
        break;
		case SIDC_SO_GET_STORAGE_INFO_DONE_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_GET_STORAGE_INFO_DONE_STATE");
        break;
        case SIDC_SO_GET_NUM_OF_OBJECT_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_GET_NUM_OF_OBJECT_STATE");
        break;
        case SIDC_SO_GET_NUM_OF_OBJECT_DONE_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_GET_NUM_OF_OBJECT_DONE_STATE");
        break;
        case SIDC_SO_GET_OBJECT_HANDLES_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_GET_OBJECT_HANDLES_STATE");
        break;
        case SIDC_SO_GET_OBJECT_HANDLES_DONE_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_GET_OBJECT_HANDLES_DONE_STATE");
        break;
        case SIDC_SO_GET_OBJECT_INFO_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_GET_OBJECT_INFO_STATE");
        break;
        case SIDC_SO_GET_OBJECT_INFO_DONE_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_GET_OBJECT_INFO_DONE_STATE");
        break;
        case SIDC_SO_DONE_STATE:
			MP_DEBUG("SidcScanObjectsStateMachine: SIDC_SO_DONE_STATE");
        break;
		default:
			MP_DEBUG("SidcScanObjectsStateMachine: default");
		break;
    }

    switch( pReceiveMailDrv->wCurrentExecutionState )
    {
        case SIDC_SO_START_STATE:
        {
#if USBOTG_HOST_DESC
            if (GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)
				== PIMA_15740_BULK_ONLY_PROTOCOL)
#else            
            if (pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol 
				== PIMA_15740_BULK_ONLY_PROTOCOL)
#endif
            { // PIMA 15740 Compliant Bulk-Only
                psSidc->stScanObject.dwTotalNumberOfStorages = 0;
                psSidc->stScanObject.dwIndexOfStorages = 0;
                psSidc->stScanObject.dwTotalNumberOfObjects  = 0;
                psSidc->stScanObject.dwIndexOfObjects = 0;
                psSidc->stScanObject.dwTotalNumberOfFolderObjects = 1;
                psSidc->stScanObject.dwIndexOfFolderObjects  = 1; // including root
                psSidc->stScanObject.dwTotalNumberOfVideoObjects = 0;
                psSidc->stScanObject.dwIndexOfVideoObjects = 0;
                psSidc->stScanObject.dwTotalNumberOfAudioObjects = 0;
                psSidc->stScanObject.dwIndexOfAudioObjects = 0;
                psSidc->stScanObject.dwTotalNumberOfPictureObjects = 0;
                psSidc->stScanObject.dwIndexOfPictureObjects = 0;
                psSidc->stScanObject.dwObjectCount = 0;

                if (eWhichOtg == USBOTG0)
                    pSidcDes->bDevLun = DEV_USB_HOST_PTP-1;
                else if (eWhichOtg == USBOTG1)
                    pSidcDes->bDevLun = DEV_USBOTG1_HOST_PTP-1;
                else
                    MP_ALERT("%s USBOTG%d cannot be recognized!!", __FUNCTION__, eWhichOtg);
                
                pSidcDes->dFolderObjectHandlesIdx = 0;
                pSidcDes->dFolderObjectHandles[0] = GET_OBJECT_IN_THE_ROOT;
                pSidcDes->dSorageIdsIdx = 0;
                pSendMailDrv->wStateMachine             = SIDC_SCAN_OBJECTS_SM;
                pSendMailDrv->wCurrentExecutionState    = SIDC_SO_GET_STORAGE_ID_STATE;
            }
            else 
            {
                MP_DEBUG("SidcScanObjectsStateMachine: protocol is not supported!");// protocol is not supported
            }
        }
        break;

        case SIDC_SO_GET_STORAGE_ID_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = SIDC_SCAN_OBJECTS_SM;
            pSendMailDrv->wCurrentExecutionState    = SIDC_SO_GET_STORAGE_ID_DONE_STATE;
            err = UsbOtgHostSidcCommand(pUsbhDevDes, pSidcDes, OPERATION_GET_STORAGE_IDS, eWhichOtg);     
            if (err != USB_NO_ERROR)
            {
                MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_STORAGE_ID_STATE err %x" , err);
            }
        }
        break;

        case SIDC_SO_GET_STORAGE_ID_DONE_STATE:
        {
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
                MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_STORAGE_ID_DONE_STATE err %x" , pUsbhDevDes->bQHStatus);
            }
            else
            {
                response_code = byte_swap_of_word(pSidcDes->sStiResponse.code);
                if (response_code == RESPONSE_OK)
                {
                    BYTE index = 0;
                    DWORD offset = USB_STI_CONTAINER_SIZE >> 2;

                    psSidc->stScanObject.dwTotalNumberOfStorages = byte_swap_of_dword(*((DWORD*)pSidcDes->dDataBuffer + offset));
                    if (psSidc->stScanObject.dwTotalNumberOfStorages > MAX_NUM_OF_STORAGE_ID)
                        psSidc->stScanObject.dwTotalNumberOfStorages = MAX_NUM_OF_STORAGE_ID;

                    for (index = 0; index < psSidc->stScanObject.dwTotalNumberOfStorages; index++)
                    {
                        pSidcDes->dSorageIds[index]= byte_swap_of_dword(*((DWORD*)pSidcDes->dDataBuffer+(offset+(index+1))));
                    }

                    if (psSidc->stScanObject.dwTotalNumberOfStorages >= 2)
                        pSidcDes->dSorageIdsIdx = 1;
                    else
                        pSidcDes->dSorageIdsIdx = 0;
                    
                    pSendMailDrv->wStateMachine             = SIDC_SCAN_OBJECTS_SM;
                    pSendMailDrv->wCurrentExecutionState    = SIDC_SO_GET_STORAGE_INFO_STATE;
                }
                else
                {
					MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_STORAGE_ID_DONE_STATE response code %x", response_code);
                }
            }
        }
        break;

        case SIDC_SO_GET_STORAGE_INFO_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = SIDC_SCAN_OBJECTS_SM;
            pSendMailDrv->wCurrentExecutionState    = SIDC_SO_GET_STORAGE_INFO_DONE_STATE;
            err = UsbOtgHostSidcCommand(pUsbhDevDes, pSidcDes, OPERATION_GET_STORAGE_INFO, eWhichOtg);     
            if (err != USB_NO_ERROR)
            {
                MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_STORAGE_INFO_STATE err %x" , err);
            }
        }
        break;

        case SIDC_SO_GET_STORAGE_INFO_DONE_STATE:
        {
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
                MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_STORAGE_INFO_DONE_STATE err %x" , pUsbhDevDes->bQHStatus);
            }
            else
            {
                response_code = byte_swap_of_word(pSidcDes->sStiResponse.code);
                if (response_code == RESPONSE_OK)
                {
                    PtpFillStorageInfo(&pSidcDes->sStorageInfos[psSidc->stScanObject.bBufIdx], (BYTE*)((BYTE*)pSidcDes->dDataBuffer+USB_STI_CONTAINER_SIZE));

                    MP_DEBUG("StorageInfos storageType        = 0x%x,",pSidcDes->sStorageInfos[psSidc->stScanObject.bBufIdx].storageType);
                    MP_DEBUG("StorageInfos filesystemType     = 0x%x,",pSidcDes->sStorageInfos[psSidc->stScanObject.bBufIdx].filesystemType);
                    MP_DEBUG("StorageInfos accessCapability   = 0x%x,",pSidcDes->sStorageInfos[psSidc->stScanObject.bBufIdx].accessCapability);
                    MP_DEBUG("StorageInfos maxCapability H    = 0x%x,L = 0x%x",\
                        pSidcDes->sStorageInfos[psSidc->stScanObject.bBufIdx].maxCapability.hiLong, pSidcDes->sStorageInfos[psSidc->stScanObject.bBufIdx].maxCapability.loLong);
                    MP_DEBUG("StorageInfos freeSpaceInBytes H = 0x%x,L = 0x%x",\
                        pSidcDes->sStorageInfos[psSidc->stScanObject.bBufIdx].freeSpaceInBytes.hiLong,pSidcDes->sStorageInfos[psSidc->stScanObject.bBufIdx].freeSpaceInBytes.loLong);
                    MP_DEBUG("StorageInfos freeSpaceInImages  = 0x%x",pSidcDes->sStorageInfos[psSidc->stScanObject.bBufIdx].freeSpaceInImages);
                    
                    psSidc->stScanObject.bBufIdx++;
                    pSendMailDrv->wMCardId++;
                    pSendMailDrv->wStateMachine = SIDC_SCAN_OBJECTS_SM;
                    if (psSidc->stScanObject.bBufIdx < psSidc->stScanObject.dwTotalNumberOfStorages )
                    {
                        pSidcDes->dSorageIdsIdx = psSidc->stScanObject.bBufIdx;
                        pSendMailDrv->wCurrentExecutionState = SIDC_SO_GET_STORAGE_INFO_STATE;
                    }
                    else
                    {
                        pSidcDes->dSorageIdsIdx = 0;
                        pSendMailDrv->wCurrentExecutionState = SIDC_SO_GET_NUM_OF_OBJECT_STATE;
                    }
                }
                else
                {
					MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_STORAGE_INFO_DONE_STATE response code %x", response_code);
                }
            }
        }
        break;

        case SIDC_SO_GET_NUM_OF_OBJECT_STATE:
        {
			// SONY T9 does not support OPERATION_GET_NUM_OBJECTS command
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = SIDC_SCAN_OBJECTS_SM;
            pSendMailDrv->wCurrentExecutionState    = SIDC_SO_GET_OBJECT_HANDLES_STATE;
        }
        break;

        case SIDC_SO_GET_NUM_OF_OBJECT_DONE_STATE:
        {
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
                MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_NUM_OF_OBJECT_DONE_STATE err %x" , pUsbhDevDes->bQHStatus);
            }
            else
            {
                response_code = byte_swap_of_word(pSidcDes->sStiResponse.code);
                if (response_code == RESPONSE_OK)
                {
					pSidcDes->dNumOfObjectsInStorage[pSidcDes->dSorageIdsIdx] = byte_swap_of_dword(pSidcDes->sStiResponse.parameter[0]);
					if (pSidcDes->dNumOfObjectsInStorage[pSidcDes->dSorageIdsIdx] > MAX_NUM_OF_OBJECT_HANDLES)
						pSidcDes->dNumOfObjectsInStorage[pSidcDes->dSorageIdsIdx] = MAX_NUM_OF_OBJECT_HANDLES;
                    pSendMailDrv->wStateMachine = SIDC_SCAN_OBJECTS_SM;
                    pSendMailDrv->wCurrentExecutionState = SIDC_SO_GET_OBJECT_HANDLES_STATE;
                }
                else
                {	// SONY T9 does not support OPERATION_GET_NUM_OBJECTS command
					MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_NUM_OF_OBJECT_DONE_STATE response code %x", response_code);
					pSidcDes->dNumOfObjectsInStorage[pSidcDes->dSorageIdsIdx] = 0;
                    pSendMailDrv->wStateMachine = SIDC_SCAN_OBJECTS_SM;
                    pSendMailDrv->wCurrentExecutionState = SIDC_SO_GET_OBJECT_HANDLES_STATE;
                }
            }
        }
        break;

        case SIDC_SO_GET_OBJECT_HANDLES_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = SIDC_SCAN_OBJECTS_SM;
            pSendMailDrv->wCurrentExecutionState    = SIDC_SO_GET_OBJECT_HANDLES_DONE_STATE;
            err = UsbOtgHostSidcCommand(pUsbhDevDes, pSidcDes, OPERATION_GET_OBJECT_HANDLES, eWhichOtg);     
            if (err != USB_NO_ERROR)
            {
                MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_OBJECT_HANDLES_STATE err %x" , err);
            }
        }
        break;

        case SIDC_SO_GET_OBJECT_HANDLES_DONE_STATE:
        {
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
                MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_NUM_OF_OBJECT_DONE_STATE err %x" , pUsbhDevDes->bQHStatus);
            }
            else
            {
                response_code = byte_swap_of_word(pSidcDes->sStiResponse.code);
                if (response_code == RESPONSE_OK)
                {
                    DWORD index = 0;
                    DWORD offset = USB_STI_CONTAINER_SIZE >> 2; 

					pSidcDes->dNumOfObjectsInStorage[pSidcDes->dSorageIdsIdx] = byte_swap_of_dword(*((DWORD*)pSidcDes->dDataBuffer+(offset)));
					if (pSidcDes->dNumOfObjectsInStorage[pSidcDes->dSorageIdsIdx] > MAX_NUM_OF_OBJECT_HANDLES)
						pSidcDes->dNumOfObjectsInStorage[pSidcDes->dSorageIdsIdx] = MAX_NUM_OF_OBJECT_HANDLES;
                    psSidc->stScanObject.dwTotalNumberOfObjects = pSidcDes->dNumOfObjectsInStorage[pSidcDes->dSorageIdsIdx];
                    pSendMailDrv->wStateMachine = SIDC_SCAN_OBJECTS_SM;
                    if (psSidc->stScanObject.dwTotalNumberOfObjects == 0)
                    {
                        if (psSidc->stScanObject.dwIndexOfFolderObjects == 1) 
                        { // at root
                            if (pSidcDes->dSorageIdsIdx == psSidc->stScanObject.dwTotalNumberOfStorages)
                            { // end
                                pSendMailDrv->wCurrentExecutionState = SIDC_SO_DONE_STATE;                                
                            }
                            else
                            { // if it's root and no any object, then check next storage
                                MP_DEBUG("check next storage 2");
                                psSidc->stScanObject.dwIndexOfStorages++;
                                pSidcDes->dSorageIdsIdx++;
                                pSendMailDrv->wCurrentExecutionState = SIDC_SO_GET_NUM_OF_OBJECT_STATE;
                            }
                        }
                        else if (psSidc->stScanObject.dwIndexOfStorages == psSidc->stScanObject.dwTotalNumberOfStorages-1)
                        { // has checked all storage already
                            pSendMailDrv->wCurrentExecutionState = SIDC_SO_DONE_STATE;
                        }
                        else
                        { // here is not a root; no any object so check next object
                            psSidc->stScanObject.dwTotalNumberOfFolderObjects--;
                            pSidcDes->dFolderObjectHandlesIdx++;
                            pSendMailDrv->wCurrentExecutionState = SIDC_SO_GET_NUM_OF_OBJECT_STATE;
                        }
                    }
                    else
                    {
                        for (index = 0; index < psSidc->stScanObject.dwTotalNumberOfObjects; index++)
                        {
                            pSidcDes->dObjectHandles[index]  = byte_swap_of_dword(*((DWORD*)pSidcDes->dDataBuffer+(offset+(index+1))));
                        }

                        pSidcDes->dObjectHandlesIdx = 0;
                        psSidc->stScanObject.dwIndexOfObjects = 0;
			            pSendMailDrv->wStateMachine          = SIDC_SCAN_OBJECTS_SM;
                        pSendMailDrv->wCurrentExecutionState = SIDC_SO_GET_OBJECT_INFO_STATE;
                    }
                }
                else
                {
					MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_OBJECT_HANDLES_DONE_STATE response code %x", response_code);
                }
            }
        }
        break;

        case SIDC_SO_GET_OBJECT_INFO_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = SIDC_SCAN_OBJECTS_SM;
            pSendMailDrv->wCurrentExecutionState    = SIDC_SO_GET_OBJECT_INFO_DONE_STATE;
            err = UsbOtgHostSidcCommand(pUsbhDevDes, pSidcDes, OPERATION_GET_OBJECT_INFO, eWhichOtg);    

            if (err != USB_NO_ERROR)
            {
                MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_OBJECT_INFO_STATE err %x" , err);
            }
        }
        break;

        case SIDC_SO_GET_OBJECT_INFO_DONE_STATE:
        {
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
                MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_NUM_OF_OBJECT_DONE_STATE err %x" , pUsbhDevDes->bQHStatus);
            }
            else
            {
                WORD object_format = 0;
                DWORD offset = (USB_STI_CONTAINER_SIZE >> 2) + 1; 
                response_code = byte_swap_of_word(pSidcDes->sStiResponse.code);
                if (response_code == RESPONSE_OK)
                {
                    pSendMailDrv->wStateMachine = SIDC_SCAN_OBJECTS_SM;
                    object_format = byte_swap_of_word(*((WORD*)((DWORD*)pSidcDes->dDataBuffer+offset)));
                    switch (object_format)
                    {
                        case OBJ_FORMAT_ASSOCIATION: // for folders
                            MP_DEBUG("object %x is folder", pSidcDes->dObjectHandles[psSidc->stScanObject.dwIndexOfObjects]);
                            pSidcDes->dFolderObjectHandles[psSidc->stScanObject.dwIndexOfFolderObjects]   = pSidcDes->dObjectHandles[psSidc->stScanObject.dwIndexOfObjects];
                            psSidc->stScanObject.dwTotalNumberOfFolderObjects++;
                            psSidc->stScanObject.dwIndexOfFolderObjects++;
                            break;
                            
                        case OBJ_FORMAT_JPEG_EXIF: // for pictures
                            MP_DEBUG("object %x is picture", pSidcDes->dObjectHandles[psSidc->stScanObject.dwIndexOfObjects]);
                            //pSidcDes->dPictureObjectHandles[psSidc->stScanObject.dwIndexOfPictureObjects] = pSidcDes->dObjectHandles[psSidc->stScanObject.dwIndexOfObjects];
                            psSidc->stScanObject.dwTotalNumberOfPictureObjects++;
                            psSidc->stScanObject.dwIndexOfPictureObjects++;
                            if (psSidc->stScanObject.boIsSearchFileInfo == TRUE)
                            {
                                GetSearchInfoFromObjectInfo ( EXT_JPG,
                                                              psSidc->stScanObject.pdwExtArray,
                                                              &psSidc->stScanObject.dwObjectCount,
                                                              pSidcDes->dObjectHandles[pSidcDes->dObjectHandlesIdx],
                                                              pSidcDes->dDataBuffer,
                                                              psSidc->stScanObject.pReceiveMailFmFs);

                            }
                            break;
                        #if (MJPEG_ENABLE||VIDEO_ENABLE)
                        case OBJ_FORMAT_AVI: // for videos
                            MP_DEBUG("object %x is avi", pSidcDes->dObjectHandles[psSidc->stScanObject.dwIndexOfObjects]);
                            //pSidcDes->dVideoObjectHandles[psSidc->stScanObject.dwIndexOfVideoObjects]     = pSidcDes->dObjectHandles[psSidc->stScanObject.dwIndexOfObjects];
                            psSidc->stScanObject.dwTotalNumberOfVideoObjects++;
                            psSidc->stScanObject.dwIndexOfVideoObjects++;
                            if (psSidc->stScanObject.boIsSearchFileInfo == TRUE)
                            {
                                GetSearchInfoFromObjectInfo ( EXT_AVI,
                                                              psSidc->stScanObject.pdwExtArray,
                                                              &psSidc->stScanObject.dwObjectCount,
                                                              pSidcDes->dObjectHandles[pSidcDes->dObjectHandlesIdx],
                                                              pSidcDes->dDataBuffer,
                                                              psSidc->stScanObject.pReceiveMailFmFs);

                            }
                            break;
                        #endif    
                        case OBJ_FORMAT_MP3: // for audios
                        case OBJ_FORMAT_WAV:
                            MP_DEBUG("object %x is mp3 or wav", pSidcDes->dObjectHandles[psSidc->stScanObject.dwIndexOfObjects]);
                            MP_DEBUG("not support");
                            //pSidcDes->dAudioObjectHandles[psSidc->stScanObject.dwIndexOfAudioObjects]     = pSidcDes->dObjectHandles[psSidc->stScanObject.dwIndexOfObjects];
                            psSidc->stScanObject.dwTotalNumberOfAudioObjects++;
                            psSidc->stScanObject.dwIndexOfAudioObjects++;
                            break;
                        
                        default:
                            MP_DEBUG("object %x is unknow %x", pSidcDes->dObjectHandles[psSidc->stScanObject.dwIndexOfObjects], object_format);
                            break;
                    }

                    
                    psSidc->stScanObject.dwIndexOfObjects++;
                    pSendMailDrv->wStateMachine = SIDC_SCAN_OBJECTS_SM;
                    if (psSidc->stScanObject.dwIndexOfObjects < psSidc->stScanObject.dwTotalNumberOfObjects) // loop continue to get the object info for all objects
                    {
                        pSidcDes->dObjectHandlesIdx++;
				        pSendMailDrv->wStateMachine          = SIDC_SCAN_OBJECTS_SM;
                        pSendMailDrv->wCurrentExecutionState = SIDC_SO_GET_OBJECT_INFO_STATE;
                    }
                    else
                    {
			            pSendMailDrv->wStateMachine          = SIDC_SCAN_OBJECTS_SM;
                        pSendMailDrv->wCurrentExecutionState = SIDC_SO_DONE_STATE;   // it's for Cannon IX70
#if 0 // it's for Cannon IX70						
                        if (psSidc->stScanObject.dwTotalNumberOfFolderObjects > 1)
                        {
                            MP_DEBUG1("psSidc->stScanObject.dwTotalNumberOfFolderObjects is %x", psSidc->stScanObject.dwTotalNumberOfFolderObjects);
                            psSidc->stScanObject.dwTotalNumberOfFolderObjects--;
                            pSidcDes->dFolderObjectHandlesIdx++;
                            pSendMailDrv->wCurrentExecutionState = SIDC_SO_GET_NUM_OF_OBJECT_STATE;
                        }
                        else
                        {
                            if (pSidcDes->dSorageIdsIdx == psSidc->stScanObject.dwTotalNumberOfStorages)
                            { // end
                                pSendMailDrv->wCurrentExecutionState = SIDC_SO_DONE_STATE;                                
                            }
                            else
                            {
                           /* 
                                MP_DEBUG("set event to main to mount");
                                if (pSidcDes->bEventSet[pSidcDes->bDevLun] == 0)
                                {   // set event to main to mount
                                    Ui_SetCardInCodeByLun(pSidcDes->bDevLun, eWhichOtg);
                                    pSidcDes->bEventSet[pSidcDes->bDevLun] = 1;
                                    pSidcDes->bPresent[pSidcDes->bDevLun] = 1;
                                }

                                pSendMailDrv->wMCardId++;
                            */                                
                                MP_DEBUG("check next storage 1");
                                psSidc->stScanObject.dwIndexOfStorages++;
                                pSidcDes->dSorageIdsIdx++;
                                pSendMailDrv->wCurrentExecutionState = SIDC_SO_GET_NUM_OF_OBJECT_STATE;
                            }
                        }
#endif						
                    }
                }
                else
                {
					MP_ALERT("SidcScanObjectsStateMachine: SIDC_SO_GET_OBJECT_INFO_DONE_STATE response code %x", response_code);
                }
            }
        }
        break;

        case SIDC_SO_DONE_STATE:
        {
        	*psSidc->stScanObject.pdwRecordElement = psSidc->stScanObject.dwObjectCount;    
            psSidc->stScanObject.boIsSearchFileInfo = FALSE;
			EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
			UsbOtgHostClrSidcScanObjects(eWhichOtg);
            return;
        }
        break;
		default:
			err = USB_UNKNOW_ERROR;
		break;
    }

	pUsbh->sMDevice[bMcardTransferID].swStatus = err;
	if ((pSendMailDrv->wCurrentExecutionState == SIDC_SO_GET_STORAGE_ID_STATE) ||
		(pSendMailDrv->wCurrentExecutionState == SIDC_SO_GET_STORAGE_INFO_STATE) ||
		(pSendMailDrv->wCurrentExecutionState == SIDC_SO_GET_NUM_OF_OBJECT_STATE) ||
		(pSendMailDrv->wCurrentExecutionState == SIDC_SO_GET_OBJECT_HANDLES_STATE) ||
		(pSendMailDrv->wCurrentExecutionState == SIDC_SO_GET_OBJECT_INFO_STATE) ||
		(pSendMailDrv->wCurrentExecutionState == SIDC_SO_DONE_STATE))
	{
		SDWORD		osSts;

		osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
		if (osSts != OS_STATUS_OK)
		{
			MP_ALERT("SidcScanObjectsStateMachine: SendMailToUsbOtgHostClassTask failed!!");
		}
	}

	if (err != USB_NO_ERROR)
	{
		MP_ALERT("SidcScanObjectsStateMachine: err %x", err);
		UsbHostFinal(err, eWhichOtg);
	}
}

static void GetSearchInfoFromObjectInfo ( DWORD dwExtension,
                                          DWORD *pdwExtArray,
                                          DWORD *pdwCount,
                                          DWORD dwObjectHandle,
                                          DWORD dwSidcDataBuffer,
                                          PUSBH_PTP_MAIL_TAG pReceiveMailFmFs )
{
    BYTE    totalWordOfNameString = 0;
    DWORD   dwMaxElement = 0;
    DWORD   *pdwRecordElement;
    
	ST_SEARCH_INFO  *psSearchInfo;
    DWORD           i = 0, j = 0;
	DWORD           offset = 0;
    BYTE    tmp[24];
    BYTE*    pTmp;

    i = 0;
    dwMaxElement = pReceiveMailFmFs->dwMaxElement;
    pdwRecordElement = (DWORD*) pReceiveMailFmFs->dwRecordElementAddr;
    MP_DEBUG("Collect Object Info");
    while (pdwExtArray[i] != EXT_END)
    {
        if ((dwExtension == pdwExtArray[i]) || (pdwExtArray[i] == EXT_ALL))
        {
            psSearchInfo = &pReceiveMailFmFs->psSearchInfo[*pdwCount];
			psSearchInfo->dwDirStart        = 0;
			psSearchInfo->dwDirPoint        = 0;
			psSearchInfo->dwFdbOffset       = 0;
			psSearchInfo->dwLongFdbCount    = 0;
			psSearchInfo->bParameter        = SEARCH_INFO_FILE;
			psSearchInfo->dwMediaInfo       = 0;
			psSearchInfo->dwPtpObjectHandle = dwObjectHandle;

            offset = (USB_STI_CONTAINER_SIZE + 8);
            pTmp = (BYTE*)dwSidcDataBuffer+offset;
            memcpy(&tmp[0], pTmp, 4);
            psSearchInfo->dwFileSize = uchar_to_ulong(tmp[3], tmp[2], tmp[1], tmp[0]);
            
            offset = (USB_STI_CONTAINER_SIZE + 52) + 1;
            totalWordOfNameString = *((BYTE*)dwSidcDataBuffer+offset-1);
            pTmp = (BYTE*)dwSidcDataBuffer+offset;
            memcpy(&tmp[0], pTmp, 24);
			for (j = 0; j < 8; j++)
			{
				psSearchInfo->bName[j] = hi_byte_of_word(*((WORD*)(&tmp[0])+j));
			}
            
			for (j = 0; j < 3; j++)
			{
				psSearchInfo->bExt[j] = hi_byte_of_word(*((WORD*)(&tmp[0])+9+j));
			}

            offset += ((totalWordOfNameString << 1) + 1);
            pTmp = (BYTE*)dwSidcDataBuffer+offset;
            memcpy(&tmp[0], pTmp, 16);
            psSearchInfo->DateTime.year  = (((hi_byte_of_word(*((WORD*)&tmp[0]))-0x30)*1000) +\
                                            ((hi_byte_of_word(*((WORD*)&tmp[0]+1))-0x30)*100) +\
                                            ((hi_byte_of_word(*((WORD*)&tmp[0]+2))-0x30)*10) +\
                                             (hi_byte_of_word(*((WORD*)&tmp[0]+3))-0x30)) - 1980;
            psSearchInfo->DateTime.month = ((hi_byte_of_word(*((WORD*)&tmp[0]+4))-0x30)*10) +\
                                            (hi_byte_of_word(*((WORD*)&tmp[0]+5))-0x30);
            psSearchInfo->DateTime.day   = ((hi_byte_of_word(*((WORD*)&tmp[0]+6))-0x30)*10) +\
                                            (hi_byte_of_word(*((WORD*)&tmp[0]+7))-0x30);

            switch (dwExtension)
            {
                case EXT_JPG:
                    psSearchInfo->bFileType = FILE_OP_TYPE_IMAGE;
                break;
                #if (MJPEG_ENABLE||VIDEO_ENABLE)
                case EXT_AVI:
                    psSearchInfo->bFileType = FILE_OP_TYPE_VIDEO;
                break;
                #endif
                default:
                    psSearchInfo->bFileType = FILE_OP_TYPE_UNKNOWN;
                break;
            }

            
            (*pdwCount)++;
			if (*pdwCount >= dwMaxElement)
			{
				*pdwRecordElement = *pdwCount;
				//return swRetValue;
			}
        }

        i++;
    }
}

#endif //#if USBOTG_HOST_PTP
static void UsbOtg_0_HubActiveInterrupt(void)
{
    MP_DEBUG("3 USBOTG0 active interrupt");
    EventSet(UsbOtgHostDriverEventIdGet(USBOTG0), EVENT_EHCI_ACTIVE_INTERRUPT);
}

static void UsbOtg_1_HubActiveInterrupt(void)
{
    MP_DEBUG("3 USBOTG1 active interrupt");
    EventSet(UsbOtgHostDriverEventIdGet(USBOTG1), EVENT_EHCI_ACTIVE_INTERRUPT);
}

static void SetupHubStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
    SWORD err = 0;
    PST_USB_OTG_HOST            psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppDes     = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    PST_HUB_CLASS_DESCRIPTOR    pHubDes = (PST_HUB_CLASS_DESCRIPTOR)psHost->psHubClassDescriptor;
    ST_MCARD_MAIL               receiveMailDrv;
    ST_MCARD_MAIL               *pSendMailDrv;
    BYTE                        mail_id;
//    static WORD                 set_port_feature_selector = 0;
//    static WORD                 clear_port_feature_selector = 0;
//    static WORD                 port_num = 1;
//    static BYTE                 bInitPortNUm = 1;

    
    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
        return;
    }

    if ( psHost->stHub.bInitPortNum != pHubDes->bPortNumber)
    {
        psHost->stHub.wPortNumber     = pHubDes->bPortNumber;
         psHost->stHub.bInitPortNum = pHubDes->bPortNumber;
        MP_DEBUG2("<cpn = %d; hpn = %d>" , psHost->stHub.wPortNumber, pHubDes->bPortNumber);
    }

    MP_DEBUG("SH %x; p %d" , receiveMailDrv.wCurrentExecutionState, psHost->stHub.wPortNumber);
    switch( receiveMailDrv.wCurrentExecutionState)
    {
        case SETUP_HUB_INIT_START_STATE:
        {
            pSendMailDrv->wCurrentExecutionState    = SETUP_HUB_SET_PORT_FEATURE_STATE;
            pSendMailDrv->wStateMachine             = SETUP_HUB_SM;
            pUsbhDevDes->fpAppClassInterruptActive  = UsbOtgHostHubInterruptInActive;
            pUsbhDevDes->fpAppClassInterruptIoc     = UsbOtgHostHubInterruptInIoc;
            pUsbhDevDes->psHubClass                 = pHubDes;
            pHubDes->fpAppClassInterruptActive      = pUsbhDevDes->fpAppClassInterruptActive;
            pHubDes->fpAppClassInterruptIoc         = pUsbhDevDes->fpAppClassInterruptIoc;
            psHost->stHub.wClearPortFeatureSelector             = HUB_C_PORT_CONNECTION;
            psHost->stHub.wSetPortFeatureSelector               = HUB_PORT_POWER;
        }
        break;

        case SETUP_HUB_SET_PORT_FEATURE_STATE:
        { 
            if (psHost->stHub.wSetPortFeatureSelector == HUB_PORT_RESET)
            {
                psHost->stHub.wPortNumber = pHubDes->bPortNumber;
            }

            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine          = SETUP_HUB_SM;
            pSendMailDrv->wCurrentExecutionState = SETUP_HUB_SET_PORT_FEATURE_DONE_STATE;
            err = SetPortFeature(psHost->stHub.wPortNumber, psHost->stHub.wSetPortFeatureSelector, eWhichOtg);
            if (err != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;
            }
        }
        break;

        case SETUP_HUB_SET_PORT_FEATURE_DONE_STATE:
        {
            if (pUsbhDevDes->psHubClass->bIsPortChanged == 1)
            {
                pUsbhDevDes->psHubClass->bIsPortChanged = 0;
                MP_DEBUG("1 active interrupt");
                EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_INTERRUPT);
            }

            if (psHost->stHub.wSetPortFeatureSelector == HUB_PORT_POWER)
            { 
                psHost->stHub.wPortNumber++;               	
                if(psHost->stHub.wPortNumber > USB_HUB_MAX_PORT_NUM || psHost->stHub.wPortNumber > pHubDes->sHubDescriptor.bNbrPorts)
                {
                    psHost->stHub.wPortNumber                                = pHubDes->bPortNumber;
                    pSendMailDrv->wCurrentExecutionState	= SETUP_HUB_CLEAR_PORT_FEATURE_STATE;
                    psHost->stHub.wClearPortFeatureSelector 			= HUB_C_PORT_CONNECTION;
                }
            }
            else if (psHost->stHub.wSetPortFeatureSelector == HUB_PORT_RESET)
            {  
                //MP_DEBUG("psHost->stHub.wSetPortFeatureSelector = HUB_PORT_RESET; psHost->stHub.wPortNumber is %x", psHost->stHub.wPortNumber);
                //MP_DEBUG("r");
                //pSendMailDrv->wCurrentExecutionState = SETUP_HUB_GET_PORT_STATUS_STATE;
                MP_DEBUG("Wait for <INT> to goto SETUP_HUB_GET_PORT_STATUS_STATE");
                pUsbh->sMDevice[bMcardTransferID].swStatus = err;
                if (eWhichOtg == USBOTG0)
                    SysTimerProcAdd(160, UsbOtg_0_HubActiveInterrupt, 1);
                else if (eWhichOtg == USBOTG1)
                    SysTimerProcAdd(160, UsbOtg_1_HubActiveInterrupt, 1);
                return;
            }
        }
        break;


        case SETUP_HUB_CLEAR_PORT_FEATURE_STATE:
        { 
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine          = SETUP_HUB_SM;
            pSendMailDrv->wCurrentExecutionState = SETUP_HUB_CLEAR_PORT_FEATURE_DONE_STATE;
            err = ClearPortFeature(psHost->stHub.wPortNumber, psHost->stHub.wClearPortFeatureSelector, eWhichOtg);
            if (err != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;
            }
        }
        break;

        case SETUP_HUB_CLEAR_PORT_FEATURE_DONE_STATE:
        { 
            if (psHost->stHub.wClearPortFeatureSelector == HUB_C_PORT_RESET)
            {
                pSendMailDrv->wCurrentExecutionState 	= SETUP_HUB_GET_PORT_STATUS_STATE;
            }
            else if(psHost->stHub.wSetPortFeatureSelector == HUB_PORT_RESET)
            {
                MP_DEBUG("waitting for interrupt data in for each port change");
                psHost->stHub.wSetPortFeatureSelector            	= HUB_PORT_POWER;		            
            }
            else if(psHost->stHub.wSetPortFeatureSelector == HUB_PORT_POWER)
            {			  	
                psHost->stHub.wPortNumber++;            
                if(psHost->stHub.wPortNumber > USB_HUB_MAX_PORT_NUM || psHost->stHub.wPortNumber > pHubDes->sHubDescriptor.bNbrPorts)
                {
                    psHost->stHub.wPortNumber                             = pHubDes->bPortNumber;
                    pSendMailDrv->wCurrentExecutionState = SETUP_HUB_GET_PORT_STATUS_STATE;
                    psHost->stHub.wClearPortFeatureSelector 	 = HUB_C_PORT_CONNECTION;
                    psHost->stHub.wSetPortFeatureSelector            = HUB_PORT_POWER;		            
                }
            }
        }
        break;


        case SETUP_HUB_GET_PORT_STATUS_STATE:
        { 
            if (pUsbhDevDes->psHubClass->bIsPortChanged == 1)
            {
                pUsbhDevDes->psHubClass->bIsPortChanged = 0;
                psHost->stHub.wPortNumber = pUsbhDevDes->psHubClass->bPortNumber;
            }

            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine          = SETUP_HUB_SM;
            pSendMailDrv->wCurrentExecutionState = SETUP_HUB_GET_PORT_STATUS_DONE_STATE;
            err = SetupGetPortSatus(psHost->stHub.wPortNumber, eWhichOtg);         
            if (err != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;
            }
        }
        break;

        case SETUP_HUB_GET_PORT_STATUS_DONE_STATE:
        { 
            memcpy ((BYTE*)&(pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1]),
                    (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress,
                    4);
            MP_DEBUG("SETUP_HUB_GET_PORT_STATUS_DONE_STATE");
            MP_DEBUG( "===Begin=========================================");
            MP_DEBUG( "Status.bfConnection = %x",  pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfConnection);
            MP_DEBUG( "Status.bfEnable = %x",      pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfEnable);
            MP_DEBUG( "Status.bfSuspend = %x",     pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfSuspend);
            MP_DEBUG( "Status.bfOverCurrent = %x", pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfOverCurrent);
            MP_DEBUG( "Status.bfReset = %x",       pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfReset);
            MP_DEBUG( "Status.bfReserved1 = %x",   pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfReserved1);
            MP_DEBUG( "Status.bfPower = %x",       pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfPower);
            MP_DEBUG( "Status.bfLowSpeed = %x",    pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfLowSpeed);

            MP_DEBUG( "Change.bfConnection = %x",  pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortChange.bfConnection);
            MP_DEBUG( "Change.bfEnable = %x",      pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortChange.bfEnable);
            MP_DEBUG( "Change.bfSuspend = %x",     pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortChange.bfSuspend);
            MP_DEBUG( "Change.bfOverCurrent = %x", pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortChange.bfOverCurrent);
            MP_DEBUG( "Change.bfReset = %x",       pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortChange.bfReset);
            MP_DEBUG( "Change.bfReserved    = %x", pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortChange.bfReserved);
            MP_DEBUG( "===End===========================================");
            if (pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfReset == 1)
            {
                MP_DEBUG("reset...");
                pSendMailDrv->wCurrentExecutionState = SETUP_HUB_GET_PORT_STATUS_STATE;
                pSendMailDrv->wStateMachine          = SETUP_HUB_SM;
            }
            else if ( pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfEnable == 1 &&
            pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortChange.bfReset  == 0)
            {
                pHubDes->bPortNumber = psHost->stHub.wPortNumber;
                UsbHubGoToNextDevicePointer(eWhichOtg);
                if(pHubDes->sHubGetPortStatus[pHubDes->bPortNumber-1].sHubPortStatus.bfLowSpeed == 1)
                {
                    MP_DEBUG("It's Low Speed device");
                }
                else
                {
                    if (psHost->stHub.wPortNumber > USB_HUB_MAX_PORT_NUM || psHost->stHub.wPortNumber > pHubDes->sHubDescriptor.bNbrPorts)
                    {
                        MP_DEBUG("<Hub Done.>");
                    }
                    MP_DEBUG("<goto SETUP_SM SETUP_INIT_START_STATE>");
                    pSendMailDrv->wCurrentExecutionState = SETUP_GET_DEVICE_DESCRIPTOR_STATE;
                    pSendMailDrv->wStateMachine          = SETUP_SM;            		
                }
            }
            else if ( pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfEnable    == 1 &&
                      pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortChange.bfReset     == 1)
            {
                MP_DEBUG("<prepare to clear reset -> ClearPortFeature(%d,Reset)>", psHost->stHub.wPortNumber);
                pSendMailDrv->wCurrentExecutionState 	= SETUP_HUB_CLEAR_PORT_FEATURE_STATE;
                psHost->stHub.wClearPortFeatureSelector 			= HUB_C_PORT_RESET;
            }
            else if ( pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortChange.bfConnection == 1 &&
                      pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfConnection == 1 &&
                      pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfPower      == 1)
            {
                MP_DEBUG("<a new device plug in detected and prepare to clear connection>");
                pSendMailDrv->wCurrentExecutionState 	= SETUP_HUB_CLEAR_PORT_FEATURE_STATE;
                psHost->stHub.wClearPortFeatureSelector 			= HUB_C_PORT_CONNECTION;
            }
            else if ( pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortChange.bfConnection == 0 &&
                      pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfConnection == 1 &&
                      pHubDes->sHubGetPortStatus[psHost->stHub.wPortNumber-1].sHubPortStatus.bfPower      == 1)
            {
                MP_DEBUG("<set port reset>");
                pSendMailDrv->wCurrentExecutionState 	= SETUP_HUB_SET_PORT_FEATURE_STATE;
                psHost->stHub.wSetPortFeatureSelector            	= HUB_PORT_RESET;	            	
                pHubDes->bPortNumber                        = psHost->stHub.wPortNumber;
                psHost->stHub.wPortNumber++;
                MP_DEBUG("<set port reset> psHost->stHub.wPortNumber = %d", psHost->stHub.wPortNumber);
            }
            else
            {
                MP_DEBUG("<check the next port>");
                psHost->stHub.wPortNumber++;
                if (psHost->stHub.wPortNumber > USB_HUB_MAX_PORT_NUM || psHost->stHub.wPortNumber > pHubDes->sHubDescriptor.bNbrPorts)
                {
                    receiveMailDrv.wCurrentExecutionState   = SETUP_HUB_DONE_STATE;
                    psHost->stHub.wClearPortFeatureSelector 			= HUB_C_PORT_CONNECTION;
                }
            }

            if (psHost->stHub.wPortNumber > USB_HUB_MAX_PORT_NUM || psHost->stHub.wPortNumber > pHubDes->sHubDescriptor.bNbrPorts)
            {
                MP_DEBUG("<each port check one time only>");
                psHost->stHub.wPortNumber = pHubDes->bPortNumber;
                MP_DEBUG("2 active interrupt");
                EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_INTERRUPT);
            }

            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
        }
        break;
    }

    pUsbh->sMDevice[bMcardTransferID].swStatus = err;

    if (err == USB_NO_ERROR)
    {
        if ((receiveMailDrv.wCurrentExecutionState == SETUP_HUB_INIT_START_STATE)                ||
            (receiveMailDrv.wCurrentExecutionState == SETUP_HUB_SET_PORT_FEATURE_DONE_STATE)     ||
            (receiveMailDrv.wCurrentExecutionState == SETUP_HUB_CLEAR_PORT_FEATURE_DONE_STATE)   ||
            (receiveMailDrv.wCurrentExecutionState == SETUP_HUB_GET_PORT_STATUS_DONE_STATE))
        {
            SDWORD  osSts;
            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
            if (osSts != OS_STATUS_OK)
            {
                MP_DEBUG("SendMailToUsbOtgHostClassTask failed!!");
            }
        }
    }
    else if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("SetupSM err = %d", err);
    }
}


#if USBOTG_HOST_CDC
static void CdcInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR   pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    SWORD                   err = USB_NO_ERROR, strValue = USB_NO_ERROR;
    ST_MCARD_MAIL     receiveMailDrv;
    ST_MCARD_MAIL   *pSendMailDrv;
    SDWORD                osSts;   
    
    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_CDC_SENDER, eWhichOtg);

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        UsbOtgHostInactiveAllqTD(eWhichOtg);
        receiveMailDrv.wCurrentExecutionState = SETUP_INIT_STOP_STATE;
    }

    MP_DEBUG1("CDC-stat %d" , receiveMailDrv.wCurrentExecutionState);
    
    switch( receiveMailDrv.wCurrentExecutionState)
    {
        case CDC_INIT_START_STATE:
        {
            OTGH_PT_Bulk_Init(eWhichOtg); //enable bulk pipe
            flib_Host20_Interrupt_Init(eWhichOtg); //enable interrupt pipe
                
            pUsbhDevDes->psAppClass                   = pAppDes;
            pSendMailDrv->wStateMachine                 = CDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = CDC_INIT_SET_CONTROL_LINE_STATE;
        }
        break;
        
        case CDC_INIT_SET_CONTROL_LINE_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine          = CDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState = CDC_INIT_SET_CONTROL_LINE_DONE_STATE;
            err = SetupCdcClassCmd(CDC_SET_CONTROL_LINE_STATE, CDC_PRESENT|CDC_ACTIVATE_CARRIER, eWhichOtg);  
            if (err != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;
            }
        }
        break;

        case CDC_INIT_SET_CONTROL_LINE_DONE_STATE:
        {
            pSendMailDrv->wStateMachine          = CDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState = CDC_INIT_SET_LINE_CODING_STATE;
        }
        break;
        
        case CDC_INIT_SET_LINE_CODING_STATE:
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = CDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = CDC_INIT_SET_LINE_CODING_DONE_STATE;
            err = SetupCdcClassCmd(CDC_SET_LINE_CODING, 0x00, eWhichOtg);
            if (err != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;
            }
        }
        break;

        case CDC_INIT_SET_LINE_CODING_DONE_STATE:
        {
            pSendMailDrv->wStateMachine             = CDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = CDC_INIT_READY_FOR_AT_STATE;            
        }
        break;  

        case CDC_INIT_VENDOR_CMD_STATE: // Send vendor cmd
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine          = SETUP_SM;
            pSendMailDrv->wCurrentExecutionState = SETUP_GET_DEVICE_DESCRIPTOR_DONE_STATE; 
            err = SetupCdcClassCmd(CDC_VENDOR_CMD_STATE, 0x00, eWhichOtg);  
            
            if (err != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;
            }
        }
        break;  

        case CDC_INIT_READY_FOR_AT_STATE:
        {
            MP_DEBUG("CDC_INIT_SET_LINE_CODING_DONE_STATE");
            
            pSendMailDrv->wStateMachine          = CDC_INIT_SM;
            pSendMailDrv->wCurrentExecutionState = CDC_INIT_READY_FOR_AT_DONE_STATE; 
            strValue = Mcard_DeviceInit(DEV_USB_WIFI_DEVICE);
            UsbOtgHostCdcAtCmdInit(eWhichOtg); // init AtCmdTask
            if (strValue != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;
            }
        }
        break;  

        case CDC_INIT_READY_FOR_AT_DONE_STATE:
        {
#if USB_ARCH_URB
            pUsbhDevDes->fpAppClassBulkActive       = H21_BulkActive;       // Bulk-In/Out
            pUsbhDevDes->fpAppClassBulkIoc          = H21_BulkIoc;
            pUsbhDevDes->fpAppClassInterruptActive  = UsbWifiInterruptActive;    // Interrupt
            pUsbhDevDes->fpAppClassInterruptIoc     = UsbWifiInterruptIoc;
#else
            pUsbhDevDes->fpAppClassBulkActive       = UsbOtgHostCdcBulkActive;       // Bulk-In/Out
            pUsbhDevDes->fpAppClassBulkIoc          = UsbOtgHostCdcBulkIoc;
            pUsbhDevDes->fpAppClassInterruptActive  = UsbOtgHostCdcInterruptActive;    // Interrupt
            pUsbhDevDes->fpAppClassInterruptIoc     = UsbOtgHostCdcInterruptIoc;
#endif
            UsbOtgHostCdcAtCmdPlug(eWhichOtg, TRUE);
        }
        break; 
        
        default:
        {
            MP_DEBUG("CdcInitStateMachine does not be defined!!");
        }
        break;
    }

    pUsbh->sMDevice[bMcardTransferID].swStatus = err;
    
    if (err == USB_NO_ERROR)
    {
        if ((receiveMailDrv.wCurrentExecutionState == CDC_INIT_START_STATE)                ||
            (receiveMailDrv.wCurrentExecutionState == CDC_INIT_SET_CONTROL_LINE_DONE_STATE) ||
            (receiveMailDrv.wCurrentExecutionState == CDC_INIT_SET_LINE_CODING_DONE_STATE) ||
            (receiveMailDrv.wCurrentExecutionState == CDC_INIT_READY_FOR_AT_STATE))
        {

            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
            
            if (osSts != OS_STATUS_OK)
            {
                MP_DEBUG("SendMailToUsbOtgHostClassTask failed!!");
            }
        }
    }
    else if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("Cdc_SM err = %d", err);
    }
}
#endif  // USBOTG_HOST_CDC

#if USBOTG_HOST_HID
static void HidInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR   pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pHidClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); // Get HID structure
    PUSB_HOST_HID psHid = (PUSB_HOST_HID)UsbOtgHostHidDsGet(eWhichOtg); // Get HID global variable
    PUSB_HID_DESCRIPTOR psHidDescriptor;
    PUSB_HID_REPORT         psHidReport;
    SWORD                   err = USB_NO_ERROR, strValue = USB_NO_ERROR;
    ST_MCARD_MAIL     receiveMailDrv;
    ST_MCARD_MAIL   *pSendMailDrv;
    SDWORD                osSts;
    
    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_HID_SENDER, eWhichOtg);

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        receiveMailDrv.wCurrentExecutionState = HID_INIT_STOP_STATE;
    }

    MP_DEBUG("HID-stat %d" , receiveMailDrv.wCurrentExecutionState);
    
    switch( receiveMailDrv.wCurrentExecutionState)
    {
         case HID_INIT_START_STATE:
        {
            pUsbhDevDes->psAppClass                   = pHidClassDes;

            flib_Host20_Interrupt_Init(eWhichOtg); //enable interrupt pipe
            UsbhHidInit(eWhichOtg);  // HID Init  

            pSendMailDrv->wStateMachine                 = HID_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = HID_INIT_GET_DESCRIPTOR_STATE;
        }
        break; 

        case HID_INIT_GET_DESCRIPTOR_STATE:
        {
            psHidDescriptor = UsbhHidGetHidDescriptor(pHidClassDes->bHidDescriptorCurrent, eWhichOtg);
            psHidReport = UsbhHidGetHidReport(pHidClassDes->bHidDescriptorCurrent, 0, eWhichOtg); // Get only first Hid Report, would get all Hid Report in the future.
            if(psHidReport != NULL) 
            {
                // GetDescriptor (Report)
                pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
                pSendMailDrv->wStateMachine          = HID_INIT_SM;
                pSendMailDrv->wCurrentExecutionState = HID_INIT_GET_DESCRIPTOR_DONE_STATE;
                psHid->pbHidReportData = (BYTE*)ker_mem_malloc(psHidReport->wDescriptorLength*2, TaskGetId());
				if(psHid->pbHidReportData == NULL)
				{
					MP_ALERT("-E- %s Alloc memory of Hid Report Data Fail; Kernel Memory Free Space : %d", __FUNCTION__, ker_mem_get_free_space());
					err = USB_SET_EVENT_FAILED;
				}
					
                MP_DEBUG("[Hid-Report-Data Alloc Address] 0x%x", psHid->pbHidReportData);
                err = SetupHidClassCmd(USB_REQ_GET_DESCRIPTOR, 
                                                        byte_swap_of_word(HID_DT_REPORT), 
                                                        psHidDescriptor->bInterfaceNumber, 
                                                        psHidReport->wDescriptorLength*2, 
                                                        (DWORD)psHid->pbHidReportData, 
                                                        eWhichOtg);
                                                        
                if (err != USB_NO_ERROR)
                {
					MP_ALERT("-E- %s SetupHidClassCmd Fail", __FUNCTION__);
                    err = USB_SET_EVENT_FAILED;
                }
            }

			
            if(err == USB_SET_EVENT_FAILED)
            {
                pSendMailDrv->wStateMachine             = HID_INIT_SM;
                pSendMailDrv->wCurrentExecutionState    = HID_INIT_STOP_STATE;            
            }
        }
        break;

        case HID_INIT_GET_DESCRIPTOR_DONE_STATE:
        {

            // Parser (Report)
            psHidReport = UsbhHidGetHidReport(pHidClassDes->bHidDescriptorCurrent, 0, eWhichOtg); // Get only first Hid Report, would get all Hid Report in the future.
            UsbhHidParseReport(psHid->pbHidReportData, psHidReport, eWhichOtg); // Not finish........
            if(psHid->pbHidReportData != NULL)
            {
                MP_DEBUG("[Hid-Report-Data Free Address] 0x%x", psHid->pbHidReportData);
                ker_mem_free(psHid->pbHidReportData);
            }

            // Whether enter HID_INIT_GET_REPORT_STATE again for other report
            if(pHidClassDes->bHidDescriptorCurrent + 1 < pHidClassDes->bHidDescriptorNum)
            {
                // Next pHidClassDes->bHidDescriptorCurrent 
                pHidClassDes->bHidDescriptorCurrent++;
                
                // re-enter HID_INIT_GET_REPORT_STATE again
                pSendMailDrv->wStateMachine          = HID_INIT_SM;
                pSendMailDrv->wCurrentExecutionState = HID_INIT_GET_DESCRIPTOR_STATE;
            }
            else
            {
                // Set Active & Ioc call-back function
                pUsbhDevDes->fpAppClassInterruptActive  = UsbhHidInterruptActive;    // Interrupt
                pUsbhDevDes->fpAppClassInterruptIoc     = UsbhHidInterruptIoc;    
				pUsbhDevDes->fpAppClassSetupIoc     	= UsbhHidSetupIoc;    
                
                pSendMailDrv->wStateMachine          = HID_INIT_SM;
                pSendMailDrv->wCurrentExecutionState = HID_INIT_INPUT_REPORT_STATE;
            }
        }
        break;

        #if 0 // Host set Device Num Lock or Caps Lock etc...
        case HID_INIT_SET_REPORT_STATE:
        {

        }
        break;

        case HID_INIT_SET_REPORT_DONE_STATE:
        {
      
        }
        break;  
        #endif
        
        case HID_INIT_INPUT_REPORT_STATE:
        {
            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_INTERRUPT);
        }
        break;

        case HID_INIT_INPUT_REPORT_DONE_STATE:
        {
        
        }
        break;  

        case HID_INIT_STOP_STATE:
        {
            MP_ALERT("-W- %s Stop!!", __FUNCTION__);
            UsbOtgHostInactiveAllqTD(eWhichOtg);
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = SETUP_SM;
            pSendMailDrv->wCurrentExecutionState    = SETUP_INIT_STOP_STATE;
            SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);  
            return;
        }
        break; 
        
        default:
        {
            MP_DEBUG("HidInitStateMachine does not be defined!!");
        }
        break;
          
    }

    pUsbh->sMDevice[bMcardTransferID].swStatus = err;
    
    if (err == USB_NO_ERROR)
    {
        if ((receiveMailDrv.wCurrentExecutionState == HID_INIT_START_STATE)                               ||
            (receiveMailDrv.wCurrentExecutionState == HID_INIT_GET_DESCRIPTOR_DONE_STATE)    ||
            (receiveMailDrv.wCurrentExecutionState == HID_INIT_INPUT_REPORT_DONE_STATE)         ||
            (receiveMailDrv.wCurrentExecutionState == HID_INIT_STOP_STATE))
        {

            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
            
            if (osSts != OS_STATUS_OK)
            {
                MP_DEBUG("SendMailToUsbOtgHostClassTask failed!!");
            }
        }
    }
    else if (err != USB_NO_ERROR)
    {
        MP_DEBUG("-E- %s HID_SM Err = %d", __FUNCTION__, err);	
    }
  
}
#endif  // USBOTG_HOST_HID


#if (USBOTG_HOST_USBIF || USBOTG_HOST_EYE)
void DisplayUsbIfTestMode(BYTE* bMsg)
{
    Idu_OsdErase();
    Idu_OsdPutStr(Idu_GetOsdWin(), bMsg, 31, 29, 6/*CLOCK_COLOR*/);
    MP_ALERT("USB-IF Mode : %s", bMsg);
}

BOOL SetUsbIfTestMode(BYTE bMode, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_HOST_MSDC psMsdc = &psUsbOtg->sUsbHost.sMsdc;
    
    MP_ALERT("USB-IF current Mode %d OTG %d", bMode, eWhichOtg);
        
    if( bMode > USB_IF_TEST_MAX )
        return FALSE;

    // change gUsbIfTestSts value
    psMsdc->bUsbIfTestSts = bMode;

    // To enter USB-IF test-mode to send USB pattern
    switch( bMode )
    {
        case USB_IF_TEST_ENABLE: // Set USB-IF Test Mode via UI 
            UartOutText("USB-IF Test Mode for ENABLE\r\n");
            DisplayUsbIfTestMode("TEST_ENABLE : Please plug-in USB Device");
            break;
            
        case USB_IF_TEST_READY:  // Initialize USB-IF Test Mode via MsdcCheckMediaPresent State
            UartOutText("USB-IF Test Mode for READY\r\n");
            // Disable Un-plug detection
            DisableUsbOtgHostInterrupt(eWhichOtg);
            DisplayUsbIfTestMode("Please plug-out USB Device & Press Key to Next Step");
            break;
            
        case USB_IF_TEST_J:
            UartOutText("USB-IF Test Mode for TEST_J\r\n");
            mbHost20_PORTSC_PortTest_Set(TEST_J_STATE);
            DisplayUsbIfTestMode("TEST_J_STATE");
            break;
        case USB_IF_TEST_K:
            UartOutText("USB-IF Test Mode for TEST_K\r\n");
            mbHost20_PORTSC_PortTest_Set(TEST_K_STATE);
            DisplayUsbIfTestMode("TEST_K_STATE");
            break;
        case USB_IF_TEST_SE0_NAK:
            UartOutText("USB-IF Test Mode for TEST_SE0_NAK\r\n");
            mbHost20_PORTSC_PortTest_Set(TEST_SE0_NAK_STATE);
            DisplayUsbIfTestMode("TEST_SE0_NAK_STATE");
            break;
        case USB_IF_TEST_PACKET:
            UartOutText("USB-IF Test Mode for TEST_PACKET\r\n");
            mbHost20_PORTSC_PortTest_Set(TEST_PACKET_STATE);
            #if SC_USBDEVICE
            TestModeForHC(eWhichOtg); // Need to enable SC_USBDEVICE
            #endif
            mwHost20_PORTSC_HC_TST_PKDONE();
            DisplayUsbIfTestMode("TEST_PACKET_STATE");
            break;
            
        case USB_IF_TEST_FORCE_ENABLE:
            UartOutText("USB-IF Test Mode for TEST_FORCE_ENABLE\r\n");
            mbHost20_PORTSC_PortTest_Set(TEST_FORCE_ENABLE);
            DisplayUsbIfTestMode("TEST_FORCE_ENABLE");
            break;
            
        case USB_IF_TEST_SUSPEND:
            // Disable Test Mode
            //mbHost20_PORTSC_PortTest_Set(TEST_NOT_ENABLE);            
            UartOutText("USB-IF Test Mode for TEST_SUSPEND\r\n");
            flib_Host20_Suspend(eWhichOtg);
            DisplayUsbIfTestMode("TEST_SUSPEND");
            break;
            
        case USB_IF_TEST_RESUME:
            UartOutText("USB-IF Test Mode for TEST_RESUME\r\n");
            flib_Host20_Resume(eWhichOtg);
            DisplayUsbIfTestMode("TEST_RESUME");
            break;
            
        case USB_IF_TEST_RESET:
            /* Embedded Host don't test this item */
            break;

        case USB_IF_TEST_DISABLE:
            UartOutText("USB-IF Test Mode DISABLE\r\n");
            // Disable Test Mode
            //mbHost20_PORTSC_PortTest_Set(TEST_NOT_ENABLE);
            
            // Reset Phy
            //mdwOTGC_PHY_Set();
            //IODelay(1);
            //mdwOTGC_PHY_Clr();
            
            // Enable Un-plug detection
            //EnableUsbOtgHostInterrupt();
            
            //UsbHostFinal(USB_HOST_DEVICE_PLUG_OUT);
            //InitiaizeUsbOtgHost(1);

            UsbOtgBiuReset(eWhichOtg);
            InitiaizeUsbOtgHost(1, eWhichOtg);
            
            DisplayUsbIfTestMode("Test Mode finish and please Turn-off DPF");
            break;
            
        default:
            MP_DEBUG("USB IF Err Test Mode");
            break;
    }
}

BYTE GetUsbIfTestMode(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_MSDC psMsdc;

    psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);     
    return psMsdc->bUsbIfTestSts;
}

void Api_UsbIfTestMode(WHICH_OTG eWhichOtg)
{
    BYTE bCurrentState;

    bCurrentState = GetUsbIfTestMode(eWhichOtg);

    MP_ALERT("ui_UsbIfTestMode %d", bCurrentState);
    
    if(bCurrentState == USB_IF_TEST_DISABLE)
    {
        SetUsbIfTestMode(USB_IF_TEST_ENABLE, eWhichOtg);
    }
    else if(bCurrentState >= USB_IF_TEST_READY && bCurrentState < USB_IF_TEST_MAX)
    {
        SetUsbIfTestMode(bCurrentState + 1, eWhichOtg); // Next State
    }
    else if(bCurrentState == USB_IF_TEST_MAX)
    {
        SetUsbIfTestMode(USB_IF_TEST_DISABLE, eWhichOtg); // Loop State 
    }
    else
    {
        MP_ALERT("USB-IF Test Mode is not READY");
        SetUsbIfTestMode(USB_IF_TEST_ENABLE, eWhichOtg);
    }
}
#endif  //USBOTG_HOST_USBIF

#if USBOTG_HOST_DATANG
static void DatangInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR   pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    SWORD                   err = USB_NO_ERROR, strValue = USB_NO_ERROR;
    ST_MCARD_MAIL     receiveMailDrv;
    ST_MCARD_MAIL   *pSendMailDrv;
    SDWORD                osSts;   
    
    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_DATANG_SENDER, eWhichOtg);

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        UsbOtgHostInactiveAllqTD(eWhichOtg);
        receiveMailDrv.wCurrentExecutionState = SETUP_INIT_STOP_STATE;
    }

    MP_DEBUG1("DATANG-stat %d" , receiveMailDrv.wCurrentExecutionState);
    
    switch( receiveMailDrv.wCurrentExecutionState)
    {
        case DATANG_INIT_START_STATE:
        {
            OTGH_PT_Bulk_Init(eWhichOtg); //enable bulk pipe                
            pUsbhDevDes->psAppClass				= pAppDes;
            pSendMailDrv->wStateMachine                 = DATANG_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    	= DATANG_INIT_READY_FOR_AT_STATE;
        }
        break;
        
        case DATANG_INIT_READY_FOR_AT_STATE:
        {
		MP_DEBUG("DATANG_INIT_READY_FOR_AT_STATE");            
		pSendMailDrv->wStateMachine          	= DATANG_INIT_SM;
		pSendMailDrv->wCurrentExecutionState	= DATANG_INIT_READY_FOR_AT_DONE_STATE; 
		strValue = Mcard_DeviceInit(DEV_USB_WIFI_DEVICE);	//To go through CommandProcess
            	UsbOtgHostCdcAtCmdInit(eWhichOtg); // init AtCmdTask
            	if (strValue != USB_NO_ERROR)
            {
                err = USB_SET_EVENT_FAILED;
            }
        }
        break;  

        case DATANG_INIT_READY_FOR_AT_DONE_STATE:
        {
#if USB_ARCH_URB
            pUsbhDevDes->fpAppClassBulkActive       	= H21_BulkActive;       // Bulk-In/Out
            pUsbhDevDes->fpAppClassBulkIoc          	= H21_BulkIoc;
#else
            pUsbhDevDes->fpAppClassBulkActive       	= UsbOtgHostCdcBulkActive;       // Bulk-In/Out
            pUsbhDevDes->fpAppClassBulkIoc          	= UsbOtgHostCdcBulkIoc;
#endif
            UsbOtgHostCdcAtCmdPlug(eWhichOtg, TRUE);
        }
        break; 
        
        default:
        {
            MP_DEBUG("DatangInitStateMachine does not be defined!!");
        }
        break;
    }

    MP_DEBUG("DatangInitStateMachine sm=%d,exec=%d", pSendMailDrv->wStateMachine,
           pSendMailDrv->wCurrentExecutionState);

    pUsbh->sMDevice[bMcardTransferID].swStatus = err;
    
    if (err == USB_NO_ERROR)
    {
        if ((receiveMailDrv.wCurrentExecutionState == DATANG_INIT_START_STATE)                ||
            (receiveMailDrv.wCurrentExecutionState == DATANG_INIT_READY_FOR_AT_STATE))
        {

            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
            
            if (osSts != OS_STATUS_OK)
            {
                MP_DEBUG("SendMailToUsbOtgHostClassTask failed!!");
            }
        }
    }
    else if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("Datang_SM err = %d", err);
    }
}
#endif  // USBOTG_HOST_DATANG
#endif //SC_USBHOST









