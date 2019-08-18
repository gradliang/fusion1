
#define LOCAL_DEBUG_ENABLE 0

#include "iplaysysconfig.h"
#include "global612.h"
#include "mpTrace.h"
#include "BtApi.h"
#include "btbrowser.h"
#include "Taskid.h"
#include "../../Os/include/event.h"
#include "../../Os/include/semaphore.h"
#include "../../Os/include/task.h"
#include "../../Os/include/systimer.h"
#include "../../Os/include/memory.h"
#include "filebrowser.h"
#include "FlagDefine.h"
#include "usbotg.h"

#if (BLUETOOTH == ENABLE)

#if (BT_DRIVER_TYPE == BT_UART)
extern void UartSingleTaskDriverProcess();
#endif

unsigned int BtEventIdArray[2] = {0, 0};
unsigned char BtMessageIdArray[BT_EVENT_ARRAY_SIZE] = {0};
unsigned char BtUiTaskIdArray[BT_UI_TASK_ARRAY_SIZE] = {0};
unsigned long BtUiTaskFunArray[BT_UI_TASK_ARRAY_SIZE] = {0};


void BtMessageReceiveInit(unsigned char ui_event_id,unsigned int ui_event_id_tag,unsigned char i,unsigned char event,unsigned char task,unsigned long func)
{
    BtEventIdArray[0] = ui_event_id;
    BtEventIdArray[1] = ui_event_id_tag;
    BtMessageIdArray[i] = event;
    BtUiTaskIdArray[i] = task;
    BtUiTaskFunArray[i] = func;
}


#define BT_API_OS_EVENT_OR  OS_EVENT_OR

void BtOsInit(void)
{
#if (BT_MULTI_TASK == 1)
    _BtApiEventCreate = (void *)EventCreate;
    _BtApiEventSet = (void *)EventSet;
    _BtApiEventWait = (void *)EventWait;
    _BtApiEventDestroy = (void *)EventDestroy;

    _BT_OS_EVENT = BT_OS_EVENT;
    _BT_OS_EVENT_ATTR = (OS_ATTR_FIFO|OS_ATTR_WAIT_SINGLE|OS_ATTR_EVENT_CLEAR);
    _BT_OS_EVENT_WAIT_ATTR = OS_EVENT_OR;


    _BtApiSemaphoreCreate = (void *)SemaphoreCreate;
    _BtApiSemaphoreWait = (void *)SemaphoreWait;
    _BtApiSemaphorePolling = (void *)SemaphorePolling;

    _BTstack_SEMA_ID = BTstack_SEMA_ID;
    _BTstack_SEMA_ID_ATTR = OS_ATTR_PRIORITY;
    _BThw_SEMA_ID = BThw_SEMA_ID;
    _BThw_SEMA_ID_ATTR = OS_ATTR_PRIORITY;
    _BTir_SEMA_ID = BTir_SEMA_ID;
    _BTir_SEMA_ID_ATTR = OS_ATTR_PRIORITY;
    _BtApiTaskCreate = (void *)TaskCreate;
    _BtApiTaskGetId = (void *)TaskGetId;
    _BtApiTaskTerminate = (void *)TaskTerminate;
    _BtApiTaskStartup = (void *)TaskStartup;
    _BT_OS_TASK = BT_OS_TASK;
    _BT_OS_TASK_PRIORITY = CONTROL_PRIORITY;

    _BtApiSemaphoreDestroy = (void *)SemaphoreDestroy;
    _BtApiSemaphoreRelease = (void *)SemaphoreRelease;


    _BtApiGetSysTime = (void *)GetSysTime;
    _BtApiAddTimerProc = (void *)SysTimerProcAdd;
    _BtApiRemoveTimerProc = (void *)SysTimerProcRemove;

#if (BT_PROFILE_TYPE & BT_A2DP)
    _BT_A2DP_EVENT = BT_A2DP_EVENT;
    _BT_A2DP_TASK = BT_A2DP_TASK;
#endif

#if (BT_PROFILE_TYPE & BT_HF)
    _BT_SCO_EVENT = BT_SCO_EVENT;
    _BT_SCO_TASK = BT_SCO_TASK;
    _BTwaveIn_SEMA_ID = BTwaveIn_SEMA_ID;
    _BTwaveIn_SEMA_ID_ATTR = OS_ATTR_PRIORITY;
    _BTwaveOut_SEMA_ID = BTwaveOut_SEMA_ID;
    _BTwaveOut_SEMA_ID_ATTR = OS_ATTR_PRIORITY ;
    _BTpacket_SEMA_ID = BTpacket_SEMA_ID;
    _BTpacket_SEMA_ID_ATTR = OS_ATTR_PRIORITY;
#endif

    _BT_A2DP_EVENT_ATTR = (OS_ATTR_FIFO|OS_ATTR_WAIT_SINGLE|OS_ATTR_EVENT_CLEAR);
    _BT_A2DP_EVENT_WAIT_ATTR = OS_EVENT_OR;


    _BT_A2DP_TASK_PRIORITY = CONTROL_PRIORITY;
#endif
}


void SetBtInitialize(unsigned char flag)
{
    BTA(btInitialize) = flag;
}


unsigned char GetBtInitialize(void)
{
    return BTA(btInitialize);
}

void BtIspInit(void)
{
    _IspWriteBTDevice = (void *)IspWriteBTDevice;
    _IspReadBTDevice = (void *)IspReadBTDevice;
}

void BtFsInit(void)
{
    _BtApiFileCreate = (void *)BtBrowserFileCreate;
    _BtApiFileClose = (void *)BtBrowserFileClose;
    _BtApiFileWrite = (void *)FileWrite;
    _BtApiCheckSpace = (void *)BtCheckSpace;
}

void BtMemInit(void)
{
#if (FIXED_WRITE_BUFFER == 0)
    BTA(pWBuf0) = ext_mem_malloc(MAX_FILE_LENGTH);
    if (BTA(pWBuf0) == NULL)
    {
        mpDebugPrint("BTA(pWBuf0) %x mem alloc fail , stop initialize", BTA(pWBuf0));
        return ;
    }

    BTA(pWBuf1) = ext_mem_malloc(MAX_FILE_LENGTH);
    if (BTA(pWBuf1) == NULL)
    {
        mpDebugPrint("BTA(pWBuf1) %x mem alloc fail , stop initialize", BTA(pWBuf1));
        return;
    }
#else
    BTA(pWBuf0) = (unsigned char *) WriteBuf0;
    BTA(pWBuf1) = (unsigned char *) WriteBuf1;
#endif
BtUiMemAlCallback = 0;
}

void SetBtOn(unsigned char flag)
{
    BTA(btOn) = flag;
}

unsigned char GetBtOn(void)
{
    return BTA(btOn);
}


int BtApiInit(void * func)
{
    char *ptest;
    if (GetBtInitialize()== 1)
    {
        mpDebugPrint("\r\n Bt module is initialized , ignore this action !!!");
        return -1;
    }
    g_psBTAPIContext = (ST_BT_API_CONTEXT*)((unsigned long)(&g_sBTAPIContext));
    memset(g_psBTAPIContext, 0, sizeof(ST_BT_API_CONTEXT));
#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
    BTA(g_psBTFTPCContext) = (ST_BT_FTPC_CONTEXT*)((unsigned long)(&BTA(g_sBTFTPCContext)));
#endif

#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
    BTA(g_psBTFTPSContext) = (ST_BT_FTPS_CONTEXT*)((unsigned long)(&BTA(g_sBTFTPSContext)));
#endif

#if (BT_PROFILE_TYPE & BT_A2DP)
    BTA(g_psBTA2DPContext) = (ST_BT_A2DP_CONTEXT*)((unsigned long)(&BTA(g_sBTA2DPContext)));
#endif

#if (BT_PROFILE_TYPE & BT_HF)
    BTA(g_psBTHFContext) = (ST_BT_HF_CONTEXT*)((unsigned long)(&BTA(g_sBTHFContext)));    
    BTA(pHMessage) = (HfMessage *)((unsigned long)&BTA(HMessage));     
#endif
    BTA(pXpgEvent) = (XpgEvent *)((unsigned long)&BTA(gXpgEvent));
    BTA(pXMessage) = (XpgMessage *)((unsigned long)&BTA(XMessage));
    BTA(pIMessage) = (InquiryMessage *)((unsigned long)&BTA(IMessage));
    BTA(pOMessage) = (OpushMessage *)((unsigned long)&BTA(OMessage));

    BTA(BtASynCallBack) = (void *)func;
    BtMemInit();
////////// Bt Os Api Definition //////////
    BtOsInit();
////////// Bt Isp Api Definition //////////
    BtIspInit();
/////////////////////////////////////
    BtFsInit();
/////////////////////////////////////
    SetBtOn(1);
    if (MpxBtCoreInit() != 0)
    {
        mpDebugPrint("\r\n MpxBtCoreInit fail");
        return -1;
    }
    
    return 0;
}


void BtOsDeInit(void)
{
#if (BT_MULTI_TASK == 1)
    _BtApiEventCreate = 0;
    _BtApiEventSet = 0;
    _BtApiEventWait = 0;
    _BtApiEventDestroy = 0;

    _BT_OS_EVENT = 0;
    _BT_OS_EVENT_ATTR = 0;
    _BT_OS_EVENT_WAIT_ATTR = 0;


    _BtApiSemaphoreCreate = 0;
    _BtApiSemaphoreWait = 0;
    _BtApiSemaphorePolling = 0;
    _BTstack_SEMA_ID = 0;
    _BTstack_SEMA_ID_ATTR = 0;
    _BThw_SEMA_ID = 0;
    _BThw_SEMA_ID_ATTR = 0;
    _BTir_SEMA_ID = 0;
    _BTir_SEMA_ID_ATTR = 0;
    _BtApiTaskCreate = 0;
    _BtApiTaskGetId = 0;
    _BtApiTaskTerminate = 0;
    _BtApiTaskStartup = 0;
    _BT_OS_TASK = 0;
    _BT_OS_TASK_PRIORITY = 0;

    _BtApiSemaphoreDestroy = 0;
    _BtApiSemaphoreRelease = 0;


    _BtApiGetSysTime = 0;
    _BtApiAddTimerProc = 0;
    _BtApiRemoveTimerProc = 0;

#if BT_PROFILE_TYPE & BT_A2DP
    _BT_A2DP_EVENT = 0;
    _BT_A2DP_TASK = 0;
#endif

#if BT_PROFILE_TYPE & BT_HF
    _BT_SCO_EVENT = 0;
    _BT_SCO_TASK = 0;
     _BTwaveOut_SEMA_ID = 0;
    _BTwaveOut_SEMA_ID_ATTR = 0;
   _BTwaveIn_SEMA_ID = 0;
    _BTwaveIn_SEMA_ID_ATTR = 0;
    _BTpacket_SEMA_ID = 0;
    _BTpacket_SEMA_ID_ATTR = 0;
#endif


    _BT_A2DP_EVENT_ATTR = 0;
    _BT_A2DP_EVENT_WAIT_ATTR = 0;

    _BT_A2DP_TASK_PRIORITY = 0;
#endif
}


void BtFsDeInit(void)
{
    _BtApiFileCreate = 0;
    _BtApiFileClose = 0;
    _BtApiFileWrite = 0;
    _BtApiCheckSpace = 0;
}

void BtIspDeInit(void)
{
    _IspWriteBTDevice = 0;
    _IspReadBTDevice = 0;
}


void BtMemDeInit(void)
{
#if (FIXED_WRITE_BUFFER == 0)
    if (BTA(pWBuf0) != NULL)
    {
        ext_mem_free(BTA(pWBuf0));
        BTA(pWBuf0) = NULL;
    }
    if (BTA(pWBuf1) != NULL)
    {
        ext_mem_free(BTA(pWBuf1));
        BTA(pWBuf1) = NULL;
    }
#else
    BTA(pWBuf0) = NULL;
    BTA(pWBuf1) = NULL;
#endif

#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
    if (BTFTPC(plist_buf) != NULL)
    {
        ext_mem_free(BTFTPC(plist_buf));
        BTFTPC(plist_buf) = NULL;
        BTFTPC(list_buf_size) = 0;
    }

    DeRegisterUiMemAllocCallback();
#endif
}


void MpxBtCoreDeInit(void)
{
    BtCoreDeInit();
#if (BT_MULTI_TASK == 0)
	TaskTerminate(BT_OS_TASK);
	SemaphoreDestroy(BThw_SEMA_ID);
#endif

#if (BT_DRIVER_TYPE == BT_UART)
	TaskTerminate(BTUART_DRIVER_TASK);
#endif
}


void BtApiDeInit(void)
{
    if (GetBtInitialize() == 0)
    {
        mpDebugPrint("\r\n Bt module is not initialized , ignore this action !!!");
        return;
    }
    
    MpxBtCoreDeInit();
    BtIspDeInit();
    BtMemDeInit();
    BtOsDeInit();
    BtFsDeInit();
    memset(g_psBTAPIContext, 0, sizeof(ST_BT_API_CONTEXT));
    BTA(BtASynCallBack) = 0;
}


void BtApiBreakPoint(void)
{
    __asm("break 100");;
}


void RegisterUiMemAllocCallback(BtUiMemAllocCallback callback)
{
    if (BtUiMemAlCallback == 0)
    {
        BtUiMemAlCallback = callback;
    }
    else
    {
        mpDebugPrint("\r\n RegisterUiMemAllocCallback fail , BtUiMemAlCallback 0x%x",BtUiMemAlCallback);
    }
}


void DeRegisterUiMemAllocCallback(void)
{
    if (BtUiMemAlCallback != 0)
        BtUiMemAlCallback = 0;
}


unsigned char GetOppConnect(void)
{
    return BTA(OppConnect);
}


#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
unsigned char GetFtpSConnect(void)
{
    return (unsigned char)BTFTPS(FtpSConnect);
}
#endif


#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
unsigned char GetFtpCConnect(void)
{
    return BTFTPC(FtpCConnect);
}
#endif


#if (BT_PROFILE_TYPE & BT_A2DP)
unsigned char GetA2dpConnect(void)
{
    return (unsigned char)BTA2DP(A2dpConnect);
}
#endif


#if (BT_PROFILE_TYPE & BT_HF)
unsigned char GetHfConnect(void)
{
    return BTHF(HfConnect);
}
#endif


int IsBtIdle(void)
{
    int Status = 0;
    if (GetOppConnect() == 1)
    {
        mpDebugPrint("\r\n Opush is connecting...");
    }
#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
    else if (GetFtpCConnect() == 1)
    {
        mpDebugPrint("\r\n Ftp Client is connecting...");
    }
#endif

#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
    else if (GetFtpSConnect() == 1)
    {
        mpDebugPrint("\r\n Ftp Server is connecting...");
    }
#endif    
#if (BT_PROFILE_TYPE & BT_A2DP)
    else if (GetA2dpConnect() == 1)
    {
        mpDebugPrint("\r\n A2dp is connecting...");
    }
#endif
#if (BT_PROFILE_TYPE & BT_HF)
    else if (GetHfConnect() == 1)
    {
        mpDebugPrint("\r\n Hand free is connecting...");
    }
#endif    
    else
    {
        Status = 1;
        mpDebugPrint("\r\n Bt is idle");
    }
    return Status;
}


static unsigned char DefaultBtName[200] = {0};
void SetBtDefaultName(unsigned char * name)
{
    unsigned char len;

    if (name == 0)
        return;

    memset((unsigned char *)DefaultBtName,0,200);
    unsigned char *cp = name;
    while (*cp != 0)
        cp++;

    len = cp - name;
    if ((len > 0) && (len < 200))
    {
        memcpy(DefaultBtName,name,len);
    }
}


// Call by Bt stack
unsigned char *GetBtDefaultName(void)
{
    return DefaultBtName;
}


char *BtEventMsg(int Idx)
{
	switch(Idx)
	{
		case BT_UI_MSG_ID:
			return "BT_UI_MSG_ID";
	}
}


char *InitializeMessageMsg(int Idx)
{
    switch(Idx)
    {
    	case MPX_MESSAGE_INITIALIZE_IDEL:
    		return "MPX_MESSAGE_INITIALIZE_IDEL";
        case MPX_MESSAGE_INITIALIZE_DONE:
            return "MPX_MESSAGE_INITIALIZE_DONE";
        case MPX_MESSAGE_LOCAO_BD_ADDRESS:
            return "MPX_MESSAGE_LOCAO_BD_ADDRESS";
        case MPX_MESSAGE_REMOTE_BD_ADDRESS:
            return "MPX_MESSAGE_REMOTE_BD_ADDRESS";
        case MPX_MESSAGE_BD_NAME:
            return "MPX_MESSAGE_BD_NAME";
        case MPX_MESSAGE_INITIALIZE_FATAL_ERROR:
            return "MPX_MESSAGE_INITIALIZE_FATAL_ERROR";
        case MPX_MESSAGE_INITIALIZE_INIT_ERROR:
            return "MPX_MESSAGE_INITIALIZE_INIT_ERROR";
    }
}


char *InquiryMessageMsg(int Idx)
{
    switch(Idx)
    {
    	case MPX_MESSAGE_INQUIRY_IDEL:
    		return "MPX_MESSAGE_INQUIRY_IDEL";
    	case MPX_MESSAGE_INQUIRY_ADD:
    		return "MPX_MESSAGE_INQUIRY_ADD";
    	case MPX_MESSAGE_INQUIRY_ERASE:
    		return "MPX_MESSAGE_INQUIRY_ERASE";
    	case MPX_MESSAGE_INQUIRY_UPDATE:
    		return "MPX_MESSAGE_INQUIRY_UPDATE";
        case MPX_MESSAGE_INQUIRY_NAME_REQUEST:
            return "MPX_MESSAGE_INQUIRY_NAME_REQUEST";
        case MPX_MESSAGE_INQUIRY_NAME_REQUEST_FAIL:
            return "MPX_MESSAGE_INQUIRY_NAME_REQUEST_FAIL";
    	case MPX_MESSAGE_INQUIRY_COMPLETE:
    		return "MPX_MESSAGE_INQUIRY_COMPLETE";
    }
}


char *PairingMessageMsg(int Idx)
{
    switch(Idx)
    {
        case MPX_MESSAGE_PAIRING_IDEL:
            return "MPX_MESSAGE_PAIRING_IDEL";
        case MPX_MESSAGE_PAIRING_PIN_REQ:
            return "MPX_MESSAGE_PAIRING_PIN_REQ";
        case MPX_MESSAGE_PAIRING_PIN_CANCEL:
            return"MPX_MESSAGE_PAIRING_PIN_CANCEL";
    	case MPX_MESSAGE_PAIRING_COMPLETE:
    		return "MPX_MESSAGE_PAIRING_COMPLETE";
    	case MPX_MESSAGE_PAIRING_CANCEL_PAIRED_DEVICE:
    		return "MPX_MESSAGE_PAIRING_CANCEL_PAIRED_DEVICE";
    	case MPX_MESSAGE_PAIRING_DELETE_PAIRED_DEVICE:
    		return "MPX_MESSAGE_PAIRING_DELETE_PAIRED_DEVICE";
    	case MPX_MESSAGE_PAIRING_ADD_RECORDER:
    		return "MPX_MESSAGE_PAIRING_ADD_RECORDER";
    	case MPX_MESSAGE_PAIRING_DEV_KEY_EXISTED:
    		return "MPX_MESSAGE_PAIRING_DEV_KEY_EXISTED";
        case MPX_MESSAGE_PAIRING_ACL_DISCONNECT:
            return "MPX_MESSAGE_PAIRING_ACL_DISCONNECT";
    }
}


char *ServiceMessageMsg(int Idx)
{
    switch(Idx)
    {
    	case MPX_MESSAGE_SERVICE_IDEL:
    		return "MPX_MESSAGE_SERVICE_IDEL";
    	case MPX_MESSAGE_SERVICE_NO_OBEX_SERVICE_FOUND:
    		return "MPX_MESSAGE_SERVICE_NO_OBEX_SERVICE_FOUND";
        case MPX_MESSAGE_SERVICE_DRAW:
            return "MPX_MESSAGE_SERVICE_DRAW";
        case MPX_MESSAGE_SERVICE_STORE:
            return "MPX_MESSAGE_SERVICE_STORE";
        case MPX_MESSAGE_ACCEPT_CONNECT_DIALOGUE:
            return "MPX_MESSAGE_ACCEPT_CONNECT_DIALOGUE";
    }
}


char *OPushMessageMsg(int Idx)
{
    switch(Idx)
    {
        case MPX_MESSAGE_OPUSH_IDEL:
            return "MPX_MESSAGE_OPUSH_IDEL";
    	case MPX_MESSAGE_OPUSH_SHOW_WELCOME_PAGE:
    		return "MPX_MESSAGE_OPUSH_SHOW_WELCOME_PAGE";
        case MPX_MESSAGE_OPUSH_RECEIVE_DIALOGUE:
            return "MPX_MESSAGE_OPUSH_RECEIVE_DIALOGUE";
        case MPX_MESSAGE_OPUSH_FILE_TRANSFER_DIALOGUE:
            return "MPX_MESSAGE_OPUSH_FILE_TRANSFER_DIALOGUE";
        case MPX_MESSAGE_OPUSH_FSTORE_FINISH:
            return "MPX_MESSAGE_OPUSH_FSTORE_FINISH";
    	case MPX_MESSAGE_OPUSH_DRAW_STATUS:
    		return "MPX_MESSAGE_OPUSH_DRAW_STATUS";
        case MPX_MESSAGE_OPUSH_CAL_TIME:
            return "MPX_MESSAGE_OPUSH_CAL_TIME";
        case MPX_MESSAGE_FILE_TRANSFER_FINISH:
            return "MPX_MESSAGE_FILE_TRANSFER_FINISH";
        case MPX_MESSAGE_OPUSHS_CONNECT:
            return "MPX_MESSAGE_OPUSHS_CONNECT";
        case MPX_MESSAGE_OPUSHS_DISCONNECT:
            return "MPX_MESSAGE_OPUSHS_DISCONNECT";
        case MPX_MESSAGE_OPUSHS_ABORT:
            return "MPX_MESSAGE_OPUSHS_ABORT";
        case MPX_MESSAGE_OPUSHC_CONNECT:
            return "MPX_MESSAGE_OPUSHC_CONNECT";
        case MPX_MESSAGE_OPUSHC_DISCONNECT:
            return "MPX_MESSAGE_OPUSHC_DISCONNECT";
        case MPX_MESSAGE_OPUSHC_ABORT:
            return "MPX_MESSAGE_OPUSHC_ABORT";
        case MPX_MESSAGE_OPUSHS_PUSH_COMPLETE:
            return "MPX_MESSAGE_OPUSHS_PUSH_COMPLETE";
        case MPX_MESSAGE_OPUSHS_PULL_COMPLETE:
            return "MPX_MESSAGE_OPUSHS_PULL_COMPLETE";
        case MPX_MESSAGE_OPUSHC_PUSH_COMPLETE:
            return "MPX_MESSAGE_OPUSHC_PUSH_COMPLETE";
        case MPX_MESSAGE_OPUSHC_PULL_COMPLETE:
            return "MPX_MESSAGE_OPUSHC_PULL_COMPLETE";
    }
}

#if ((BT_PROFILE_TYPE & BT_FTP_CLIENT) == BT_FTP_CLIENT)
char *FTPCMessageMsg(int Idx)
{
    switch(Idx)
    {
        case MPX_MESSAGE_FTPC_IDEL:
            return "MPX_MESSAGE_FTPC_IDEL";
        case MPX_MESSAGE_FTPC_CONNECT:
            return "MPX_MESSAGE_FTPC_CONNECT";
        case MPX_MESSAGE_FTPC_ABORT:
            return "MPX_MESSAGE_FTPC_ABORT";
        case MPX_MESSAGE_FTPC_DISCONNECT:
            return "MPX_MESSAGE_FTPC_DISCONNECT";
        case MPX_MESSAGE_FTPC_LIST_BUF_INIT:
            return "MPX_MESSAGE_FTPC_LIST_BUF_INIT";
        case MPX_MESSAGE_FTPC_LIST_BUF_COPY:
            return "MPX_MESSAGE_FTPC_LIST_BUF_COPY";
        case MPX_MESSAGE_FTPC_GPA_CALLBACK:
            return "MPX_MESSAGE_FTPC_GPA_CALLBACK";
        case MPX_MESSAGE_FTPC_GPA_CALLBACK_DONE:
            return "MPX_MESSAGE_FTPC_GPA_CALLBACK_DONE";
        case MPX_MESSAGE_FTPC_FINISH_FILE_TRANSFER:
            return "MPX_MESSAGE_FTPC_FINISH_FILE_TRANSFER";
        case MPX_MESSAGE_FTPC_PULL_FILE_FAIL:
            return "MPX_MESSAGE_FTPC_PULL_FILE_FAIL";
        case MPX_MESSAGE_FTPC_PUSH_COMPLETE:
            return "MPX_MESSAGE_FTPC_PUSH_COMPLETE";
        case MPX_MESSAGE_FTPC_PULL_COMPLETE:
            return "MPX_MESSAGE_FTPC_PULL_COMPLETE";
        case MPX_MESSAGE_FTPC_SETPATH_COMPLETE:
            return "MPX_MESSAGE_FTPC_SETPATH_COMPLETE";
    }
}
#endif

#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
char *FTPSMessageMsg(int Idx)
{
    switch(Idx)
    {
        case MPX_MESSAGE_FTPS_IDEL:
            return "MPX_MESSAGE_FTPS_IDEL";
        case MPX_MESSAGE_FTPS_CONNECT:
            return "MPX_MESSAGE_FTPS_CONNECT";
        case MPX_MESSAGE_FTPS_DISCONNECT:
            return "MPX_MESSAGE_FTPS_DISCONNECT";
        case MPX_MESSAGE_FTPS_ABORT:
            return "MPX_MESSAGE_FTPS_ABORT";
        case MPX_MESSAGE_FTPS_PUSH_COMPLETE:
            return "MPX_MESSAGE_FTPS_PUSH_COMPLETE";
        case MPX_MESSAGE_FTPS_PULL_COMPLETE:
            return "MPX_MESSAGE_FTPS_PULL_COMPLETE";
        case MPX_MESSAGE_FTPS_SETPATH_COMPLETE:
            return "MPX_MESSAGE_FTPS_SETPATH_COMPLETE";
        case MPX_MESSAGE_FTPS_DELETE_COMPLETE:
            return "MPX_MESSAGE_FTPS_DELETE_COMPLETE";
    }
}
#endif

#if (BT_PROFILE_TYPE & BT_A2DP)
char *A2DPMessageMsg(int Idx)
{
    switch(Idx)
    {
        case MPX_MESSAGE_A2DP_IDEL:
            return "MPX_MESSAGE_A2DP_IDEL";
        case MPX_MESSAGE_A2DP_START_STREAM:
            return "MPX_MESSAGE_A2DP_START_STREAM";
        case MPX_MESSAGE_A2DP_STREAM_START:
            return "MPX_MESSAGE_A2DP_STREAM_START";
        case MPX_MESSAGE_A2DP_STREAM_START_FAIL:
            return "MPX_MESSAGE_A2DP_STREAM_START_FAIL";
        case MPX_MESSAGE_A2DP_STREAM_SUSPEND:
            return "MPX_MESSAGE_A2DP_STREAM_SUSPEND";
        case MPX_MESSAGE_A2DP_STREAM_RECONFIG:
            return "MPX_MESSAGE_A2DP_STREAM_RECONFIG";
        case MPX_MESSAGE_A2DP_RECONFIG_STREAM:
            return "MPX_MESSAGE_A2DP_RECONFIG_STREAM";
        case MPX_MESSAGE_A2DP_CLOSE_STREAM:
            return "MPX_MESSAGE_A2DP_CLOSE_STREAM";
    }
}


char *AVRCPMessageMsg(int Idx)
{
    switch(Idx)
    {
        case MPX_MESSAGE_AVRCP_IDEL:
            return "MPX_MESSAGE_AVRCP_IDEL";
        case MPX_MESSAGE_AVRCP_PLAY:
            return "MPX_MESSAGE_AVRCP_PLAY";
        case MPX_MESSAGE_AVRCP_PAUSE:
            return "MPX_MESSAGE_AVRCP_PAUSE";
        case MPX_MESSAGE_AVRCP_STOP:
            return "MPX_MESSAGE_AVRCP_STOP";
        case MPX_MESSAGE_AVRCP_FORWARD:
            return "MPX_MESSAGE_AVRCP_FORWARD";
        case MPX_MESSAGE_AVRCP_FAST_FORWARD:
            return "MPX_MESSAGE_AVRCP_FAST_FORWARD";
        case MPX_MESSAGE_AVRCP_BACKWARD:
            return "MPX_MESSAGE_AVRCP_BACKWARD";
        case MPX_MESSAGE_AVRCP_REWIND:
            return "MPX_MESSAGE_AVRCP_REWIND";
    }
}
#endif


#if (BT_PROFILE_TYPE & BT_SPP)
char *SppMessageMsg(int Idx)
{
    switch(Idx)
    {
        case MPX_MESSAGE_SPP_IDEL:
            return "MPX_MESSAGE_SPP_IDEL";
        case MPX_MESSAGE_SPP_OPEN_IND:
            return "MPX_MESSAGE_SPP_OPEN_IND";
        case MPX_MESSAGE_SPP_OPEN:
            return "MPX_MESSAGE_SPP_OPEN";
        case MPX_MESSAGE_SPP_CLOSE_IND:
            return "MPX_MESSAGE_SPP_CLOSE_IND";
        case MPX_MESSAGE_SPP_CLOSED:
            return "MPX_MESSAGE_SPP_CLOSED";
        case MPX_MESSAGE_SPP_DATA_IND:
            return "MPX_MESSAGE_SPP_DATA_IND";
        case MPX_MESSAGE_SPP_PACKET_HANDLED:
            return "MPX_MESSAGE_SPP_PACKET_HANDLED";
        case MPX_MESSAGE_SPP_PORT_NEG_IND:
            return "MPX_MESSAGE_SPP_PORT_NEG_IND";
        case MPX_MESSAGE_SPP_PORT_NEG_CNF:
            return "MPX_MESSAGE_SPP_PORT_NEG_CNF";
        case MPX_MESSAGE_SPP_PORT_STATUS_IND:
            return "MPX_MESSAGE_SPP_PORT_STATUS_IND";
        case MPX_MESSAGE_SPP_PORT_STATUS_CNF:
            return "MPX_MESSAGE_SPP_PORT_STATUS_CNF";
        case MPX_MESSAGE_SPP_MODEM_STATUS_IND:
            return "MPX_MESSAGE_SPP_MODEM_STATUS_IND";
        case MPX_MESSAGE_SPP_MODEM_STATUS_CNF:
            return "MPX_MESSAGE_SPP_MODEM_STATUS_CNF";
        case MPX_MESSAGE_SPP_LINE_STATUS_IND:
            return "MPX_MESSAGE_SPP_LINE_STATUS_IND";
        case MPX_MESSAGE_SPP_LINE_STATUS_CNF:
            return "MPX_MESSAGE_SPP_LINE_STATUS_CNF";
        case MPX_MESSAGE_SPP_FLOW_OFF_IND:
            return "MPX_MESSAGE_SPP_FLOW_OFF_IND";
        case MPX_MESSAGE_SPP_FLOW_ON_IND:
            return "MPX_MESSAGE_SPP_FLOW_ON_IND";
        case MPX_MESSAGE_SPP_RESOURCE_FREE:
            return "MPX_MESSAGE_SPP_RESOURCE_FREE";
    }
}
#endif


#if (BT_PROFILE_TYPE & BT_HF)
char *HfMessageMsg(int Idx)
{
    switch(Idx)
    {
        case MPX_HF_EVENT_SERVICE_CONNECT_REQ:
            return "MPX_HF_EVENT_SERVICE_CONNECT_REQ";
        case MPX_HF_EVENT_SERVICE_CONNECTED:
            return "MPX_HF_EVENT_SERVICE_CONNECTED";
        case MPX_HF_EVENT_SERVICE_DISCONNECTED:
            return "MPX_HF_EVENT_SERVICE_DISCONNECTED";
        case MPX_HF_EVENT_AUDIO_CONNECTED:
            return "MPX_HF_EVENT_AUDIO_CONNECTED";
        case MPX_HF_EVENT_AUDIO_DISCONNECTED:
            return "MPX_HF_EVENT_AUDIO_DISCONNECTED";
        case MPX_HF_EVENT_AUDIO_DATA:
            return "MPX_HF_EVENT_AUDIO_DATA";
        case MPX_HF_EVENT_AUDIO_DATA_SENT:
            return "MPX_HF_EVENT_AUDIO_DATA_SENT";
        case MPX_HF_EVENT_GATEWAY_FEATURES:
            return "MPX_HF_EVENT_GATEWAY_FEATURES";
        case MPX_HF_EVENT_GW_HOLD_FEATURES:
            return "MPX_HF_EVENT_GW_HOLD_FEATURES";
        case MPX_HF_EVENT_CALL_STATE:
            return "MPX_HF_EVENT_CALL_STATE";
        case MPX_HF_EVENT_CALLER_ID:
            return "MPX_HF_EVENT_CALLER_ID";
        case MPX_HF_EVENT_CALL_LISTING_ENABLED:
            return "MPX_HF_EVENT_CALL_LISTING_ENABLED";
        case MPX_HF_EVENT_SERVICE_IND:
            return "MPX_HF_EVENT_SERVICE_IND";
        case MPX_HF_EVENT_BATTERY_IND:
            return "MPX_HF_EVENT_BATTERY_IND";
        case MPX_HF_EVENT_SIGNAL_IND:
            return "MPX_HF_EVENT_SIGNAL_IND";
        case MPX_HF_EVENT_ROAM_IND:
            return "MPX_HF_EVENT_ROAM_IND";
        case MPX_HF_EVENT_SMS_IND:
            return "MPX_HF_EVENT_SMS_IND";
        case MPX_HF_EVENT_VOICE_REC_STATE:
            return "MPX_HF_EVENT_VOICE_REC_STATE";
        case MPX_HF_EVENT_VOICE_TAG_NUMBER:
            return "MPX_HF_EVENT_VOICE_TAG_NUMBER";
        case MPX_HF_EVENT_SPEAKER_VOLUME:
            return "MPX_HF_EVENT_SPEAKER_VOLUME";
        case MPX_HF_EVENT_MIC_VOLUME:
            return "MPX_HF_EVENT_MIC_VOLUME";
        case MPX_HF_EVENT_IN_BAND_RING:
            return "MPX_HF_EVENT_IN_BAND_RING";
        case MPX_HF_EVENT_NETWORK_OPERATOR:
            return "MPX_HF_EVENT_NETWORK_OPERATOR";
        case MPX_HF_EVENT_SUBSCRIBER_NUMBER:
            return "MPX_HF_EVENT_SUBSCRIBER_NUMBER";
        case MPX_HF_EVENT_NO_CARRIER:
            return "MPX_HF_EVENT_NO_CARRIER";
        case MPX_HF_EVENT_BUSY:
            return "MPX_HF_EVENT_BUSY";
        case MPX_HF_EVENT_NO_ANSWER:
            return "MPX_HF_EVENT_NO_ANSWER";
        case MPX_HF_EVENT_DELAYED:
            return "MPX_HF_EVENT_DELAYED";
        case MPX_HF_EVENT_BLACKLISTED:
            return "MPX_HF_EVENT_BLACKLISTED";
        case MPX_HF_EVENT_PHONEBOOK_STORAGE:
            return "MPX_HF_EVENT_PHONEBOOK_STORAGE";
        case MPX_HF_EVENT_PHONEBOOK_INFO:
            return "MPX_HF_EVENT_PHONEBOOK_INFO";
        case MPX_HF_EVENT_PHONEBOOK_SIZE:
            return "MPX_HF_EVENT_PHONEBOOK_SIZE";
        case MPX_HF_EVENT_PHONEBOOK_ENTRY:
            return "MPX_HF_EVENT_PHONEBOOK_ENTRY";
        case MPX_HF_EVENT_AT_RESULT_DATA:
            return "MPX_HF_EVENT_AT_RESULT_DATA";
        case MPX_HF_EVENT_COMMAND_COMPLETE:
            return "MPX_HF_EVENT_COMMAND_COMPLETE";
    }
}
#endif


char *BtUiTaskMsg(int Idx)
{
	switch(Idx)
	{
		case BT_UI_TASK:
			return "BT_UI_TASK";
	}
}


short BtMessageReceiveTaskInit(void)
{
    short sdwRetVal;
    unsigned char i;

    sdwRetVal = OS_STATUS_OK;
    for (i = 0; ((BtMessageIdArray[i] != 0) && (i <= BT_EVENT_ARRAY_SIZE)); i++)
    {
        sdwRetVal = MessageCreate(BtMessageIdArray[i], OS_ATTR_FIFO, BT_UI_BUFFER_SIZE);
        if (sdwRetVal != 0)
        {
            mpDebugPrint("-E- %s create fail",BtEventMsg(BtMessageIdArray[i]));
            return sdwRetVal;
        }
    }

    for(i = 0; ((BtUiTaskIdArray[i] != 0) && (i <= BT_UI_TASK_ARRAY_SIZE)); i++)
    {

        sdwRetVal = TaskCreate(BtUiTaskIdArray[i], (void *)BtUiTaskFunArray[i], CONTROL_PRIORITY, 0x1000);
        if (sdwRetVal != 0)
        {
            mpDebugPrint("-E- %s create fail",BtUiTaskMsg(BtUiTaskIdArray[i]));
            return sdwRetVal;
        }
        sdwRetVal = TaskStartup(BtUiTaskIdArray[i]);
        if (sdwRetVal != 0)
        {
            mpDebugPrint("-E- %s startup fail",BtUiTaskMsg(BtUiTaskIdArray[i]));
            return sdwRetVal;
        }
    }
    return sdwRetVal;
}


short BtMessageReceiveTaskDeinit(void)
{
    short sdwRetVal;
    unsigned char i;

    sdwRetVal = OS_STATUS_OK;
    for (i = 0; ((BtMessageIdArray[i] != 0) && (i <= BT_EVENT_ARRAY_SIZE)); i++)
    {
        sdwRetVal = MessageDestroy(BtMessageIdArray[i]);
        if (sdwRetVal != 0)
        {
            MP_DEBUG("-E- %s destroy fail",BtEventMsg(BtMessageIdArray[i]));
            return sdwRetVal;
        }
    }

    for (i = 0; ((BtUiTaskIdArray[i] != 0) && (i <= BT_UI_TASK_ARRAY_SIZE)); i++)
    {
        TaskTerminate(BtUiTaskIdArray[i]);
    }
}


void MpxBtSendMessage(unsigned char *event,unsigned long size)
{
    SWORD sdRetVal;
    BYTE i;
    BYTE * pmessage;
    BYTE mode = *(event+1);
    BYTE type = *event;

    if (mode == INITILIZE_MESSAGE_MODE)
        pmessage = InitializeMessageMsg(type);
    else if (mode == INQUIRY_MESSAGE_MODE)
        pmessage = InquiryMessageMsg(type);
    else if (mode == PAIRING_MESSAGE_MODE)
        pmessage = PairingMessageMsg(type);
    else if (mode == SERVICE_MESSAGE_MODE)
        pmessage = ServiceMessageMsg(type);
    else if (mode == OPUSH_MESSAGE_MODE)
        pmessage = OPushMessageMsg(type);

#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
    else if (mode == FTPC_MESSAGE_MODE)
        pmessage = FTPCMessageMsg(type);
#endif

#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
    else if (mode == FTPS_MESSAGE_MODE)
        pmessage = FTPSMessageMsg(type);
#endif

#if (BT_PROFILE_TYPE & BT_A2DP)
    else if (mode == A2DP_MESSAGE_MODE)
        pmessage = A2DPMessageMsg(type);
    else if (mode == AVRCP_MESSAGE_MODE)
        pmessage = (char *) AVRCPMessageMsg(type);
#endif

#if (BT_PROFILE_TYPE & BT_SPP)
    else if ((mode == SPPS_MESSAGE_MODE) || (mode == SPPC_MESSAGE_MODE))
        pmessage = SppMessageMsg(type);
#endif

#if (BT_PROFILE_TYPE & BT_HF)
    else if (mode == HF_MESSAGE_MODE)
        pmessage = HfMessageMsg(type);
#endif

    if (!((type == MPX_MESSAGE_OPUSH_DRAW_STATUS) && (mode == OPUSH_MESSAGE_MODE)))
    {
        MP_DEBUG("MpxBtSendMessage %s = %d , size = %d", pmessage, type, size);
    }

    if(size > 64)
    {
        UartOutText("***** WARNING ***** size too large !!!");
        __asm("break 100");
    }

    for(i = 0; ((BtMessageIdArray[i] != 0) && (i <= BT_EVENT_ARRAY_SIZE)); i++)
    {
        if (MessageSend(BtMessageIdArray[i], (unsigned char *)event, size) != 0)
        {
            MP_DEBUG("\r\n MessageSend fail , %s have No buffer size", BtMessageIdArray);
        }
    }

    if (BtEventIdArray[0] && BtEventIdArray[1])
    {
        EventSet(BtEventIdArray[0], BtEventIdArray[1]);
    }
}


void SetPairingFlag(unsigned char flag)
{
    BTA(pairing) = flag;
}


unsigned char GetPairingFlag(void)
{
    return BTA(pairing);
}


void SetInquiryFlag(unsigned char flag)
{
    BTA(inquiry) = flag;
}


unsigned char GetInquiryFlag(void)
{
    return BTA(inquiry);
}


void SetInquiryNameRequestFlag(unsigned char flag)
{
    BTA(InquiryNameReq) = flag;
}


unsigned char GetInquiryNameRequestFlag(void)
{
    return BTA(InquiryNameReq);
}


void FinishFileTranThread(void)
{
    if (GetFinishFileTranComplete())
    {
    	BTA(pOMessage)->eType = MPX_MESSAGE_FILE_TRANSFER_FINISH;
        BTA(pOMessage)->Mode = OPUSH_MESSAGE_MODE;
        BTA(BtASynCallBack)((unsigned char *)BTA(pOMessage),sizeof(OpushMessage));
        SetFinishFileTranComplete(0);
    }
}


int MpxBtCoreInit(void)
{
    long Val;

#if (BT_MULTI_TASK == 0)
    SemaphoreCreate(BThw_SEMA_ID, OS_ATTR_PRIORITY, 2);
    TaskCreate(BT_OS_TASK, MpxBtCoreMain, CONTROL_PRIORITY, 8192);
    TaskStartup(BT_OS_TASK);
#endif

#if (BT_DRIVER_TYPE == BT_UART)
	Val = TaskCreate(BTUART_DRIVER_TASK, UartSingleTaskDriverProcess, CONTROL_PRIORITY, 8192);
    mpDebugPrint("Val = %x",Val);
    if (Val != 0)
    {
        mpDebugPrint("task create fail Val = %x",Val);
    }
    Val = TaskStartup(BTUART_DRIVER_TASK);
    mpDebugPrint("Val = %x",Val);
    if (Val != 0)
    {
        mpDebugPrint("task start up fail Val = %x",Val);
    }
#endif

    return BtCoreInit();
}


unsigned long MpxGetDeviceContextByIndex(unsigned long index)
{
//    return GetDeviceContextInRangeByIndex(index);
    return GetDeviceContextByIndex(index);
}


#if (BT_MULTI_TASK == 0)
void MpxBtCoreMain(void)
{
    BtCoreMain();
}
#endif


SetOppConnect(unsigned char flag)
{
    BTA(OppConnect) = flag;
}


SetBtBusy(unsigned char flag)
{
    BTA(StateBusy) = flag;
}


unsigned char GetBtBusy(void)
{
    return BTA(StateBusy);
}


void SetDdbNo(unsigned char no)
{
    BTA(ddb_no) = no;
}


unsigned char GetDdbNo(void)
{
    return BTA(ddb_no);
}


// UI Layer can get relative new incoming device name , state , class information
char *MpxGetBDCName(unsigned char index)
{
    return (char *)GetBDCName(index);
}


unsigned short MpxGetBDCState(unsigned char index)
{
    return GetBDCState(index);
}


unsigned short MpxGetBDCClass(unsigned char index)
{
    return GetBDCClass(index);
}


// UI Layer can get relative device name information for pop out dialogue when surronding BT try to connect or transfer file to dpf
unsigned char * MpxGetDeviceName(void)
{
    return (char *)GetDeviceName();
}


void MpxPairingDevice_PinReq(PinCodeReq *SourcePara,unsigned long index)
{
    PinCodeReq * pin;
    pin = SourcePara;
    if (pin->PinLen > 16)
    {
        mpDebugPrint("\r\n pin len can not exceed 16");
        return;
    }
    if (MpxGetDeviceContextByIndex(index) == 0)
    {
        mpDebugPrint("\r\n This device index is invalid");
        return;
    }    
    PairingDevice_PinReq(SourcePara,index);
}


void MpxRxDialogCallback(unsigned char rx_dialogue)
{
    RxDialogCallback(rx_dialogue);
}


void MpxInqiryStoredDevice(void)
{
    if (GetInquiryFlag() == BUSY)
    {
        mpDebugPrint("\r\n Inquiry stored device is in progress...ingore this action !!!");
        return;
    }
    SetInquiryFlag(BUSY);
    InqiryStoredDevice();
}


void MpxBMGInquiry(void)
{
    if (GetInquiryFlag() == BUSY)
    {
        mpDebugPrint("\r\n Inquiry is in progress..ingore this action !!!");
        return;
    }
    mpDebugPrint("\r\n");
    mpDebugPrint("==============================");
    mpDebugPrint("= Start search bt device ... =");
    mpDebugPrint("==============================");
    SetInquiryFlag(BUSY);
    BMGInquiry();
}


void MpxShowInquiryDatabase(void)
{
    ShowInquiryDatabase();
}


void MpxPairingDevice(unsigned long index)
{
    if (GetPairingFlag() == BUSY)
    {
        mpDebugPrint("\r\n Pairing is in progress...ingore this action !!!");
        return;
    }

#if 0 // move to UI layer
    if (MpxGetDeviceContextByIndex(index) == 0)
    {
        mpDebugPrint("\r\n This device index (%d) is invalid", index);
        return;
    }
#endif

    SetPairingFlag(BUSY);
    PairingDevice(index);
}


void MpxBMGCancel(void)
{
    if (GetInquiryFlag() == IDLE)
    {
        mpDebugPrint("\r\n Inquiry is NOT in progress...ingore this action !!!");
        return;
    }
    SetInquiryFlag(IDLE);
    BMGCancel();
}


const char *MpxSdpService(unsigned long Service)
{
    return (char *)pSdpService(Service);
}


void MpxGetServiceArray(unsigned char * source)
{
    GetServiceArray(source);
}


void MpxServiceSearch(unsigned long index)
{
    if (MpxGetDeviceContextByIndex(index) == 0)
    {
        mpDebugPrint("\r\n This device index is invalid");
        return;
    }    
    ServiceSearch(index);
}


void MpxDeleteKey(unsigned long index)
{
    if (GetDeviceContextByIndex(index) == 0)
    {
        mpDebugPrint("\r\n This device index is invalid");
        return;
    }    
    DeleteKey(index);
}


void MpxCancelPairing(void)
{
    if (GetPairingFlag() == 0)
    {
        mpDebugPrint("\r\n Pairing is NOT in progress...ingore this action");
        return;
    }
    CancelPairing();
}


void MpxGoepTransferFile(char *name, unsigned long DeviceIndex)
{
    if (IsBtIdle() == 0)
    {
        mpDebugPrint("\r\n Bt is NOT idle..ignore this actoin !!!");
        return;
    }
    if (MpxGetDeviceContextByIndex(DeviceIndex) == 0)
    {
        mpDebugPrint("\r\n This device index is invalid");
        return;
    }
    Set_CurrentGpaProfile(0);
    GoepTransferFile(name, DeviceIndex);
}


void MpxGoepExchange(unsigned long index)
{
    GoepExchange(index);
}


void MpxGoepOpushRegister(void)
{
    GoepOpushRegister();
}


void MpxGoepOpushDisConnect(void)
{
    OpushSetVCard(0);
    GoepOpushDisConnect();
}


void MpxGoepOpushConnect(unsigned long index)
{
    OpushSetVCard(1);
    GoepOpushConnect(index);
}


char MpxOpushGetVCard(void)
{
    return OpushGetVCard();
}

void MpxReportLocalBdAddr(void)
{
    ReportLocalBdAddr();
}


void MpxReportRemoteBdAddr(unsigned char i)
{
    ReportRemoteBdAddr(i);
}


void MpxME_SetLocalDeviceName(unsigned char* pbuf, unsigned char len)
{
    ME_SetLocalDeviceName(pbuf,len);
}


void MpxME_SetClassOfDevice(unsigned long classOfDevice)
{
    ME_SetClassOfDevice(classOfDevice);
}


void MpxME_SetAccessibleModeNC(unsigned char mode)
{
    ME_SetBtAccessibleModeNC(mode, 0);
}


void MpxME_SetAccessibleModeC(unsigned char mode)
{
    ME_SetBtAccessibleModeC(mode, 0);
}


void MpxAppAclDisconnect(void)
{
    AppAclDisconnect();
}


void MpxGoepOpushServerAbort(void)
{
    if (GetOppConnect() == 0)
    {
        mpDebugPrint("\r\n Opush is NOT connecting...ingore this action !!!");
        return;
    }
    GoepOpushServerAbort();
}


void MpxGoepOpushClientAbort(void)
{
    if (GetOppConnect() == 0)
    {
        mpDebugPrint("\r\n Opush is NOT connecting...ingore this action !!!");
        return;
    }
    GoepOpushClientAbort();
}


static unsigned char gbFileIndex;

void SetFileIndex(unsigned long index)
{
    gbFileIndex = index;
}


unsigned long GetFileIndex(void)
{
    return gbFileIndex;
}


void MpxGoepGetVCard(void)
{
    GoepOpushGetVCard();
}


int BtGetOpushRegisterFlag(void)
{
    return Get_OpushRegistered_Flag();
}


#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
SetFtpSConnect(unsigned char flag)
{
    BTFTPS(FtpSConnect) = flag;
}


int MpxFtxInit(void)
{
#if 0 // not provide now !
    if (BtGetFtpRegisterFlag() == FALSE)
    {
        if (GetFtpServerRegisteredFlag() == FALSE)
        {
            if (BtGetOpushRegisterFlag() == TRUE)
            {
                MpxGoepOpushRegister();
            }

            if (Ftx_Init() == FALSE)
            {
                mpDebugPrint("MpxFtxInit , Ftx_Init Fail");
            } 
            mpDebugPrint("\r\n ftx init finish");
            MpxGoepOpushRegister();
        }
        else
        {
            mpDebugPrint("\r\n ftp server has been register !");
        }
    }
    else
    {
        mpDebugPrint("ftp client is connect , so do nothing");
        return FALSE;
    }
#endif // not provide now !    
}


void MpxFtxDeInit(void)
{
#if 0 // not provide now !
    if (BtGetFtpRegisterFlag() == FALSE)
    {
        if (GetFtpServerRegisteredFlag() == TRUE)
        {
            Ftx_Deinit();
            mpDebugPrint("\r\n ftx de-init finish");
//            MpxGoepOpushRegister();
//            MpxGoepOpushRegister();
        }
        else
        {
            mpDebugPrint("\r\n Ftp server does not register !");
        }
    }
    else
    {
        mpDebugPrint("ftp client is connect , so do nothing");
    }
#endif// not provide now !
}


void MpxFtxAbortServer(void)
{
    if (GetFtpSConnect() == 0)
    {
        mpDebugPrint("\r\n Ftp server in NOT connecting...ingore this action !!!");
        return;
    }
    Ftx_AbortServer();
}

#endif


#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
void btFtpConnect(unsigned char index)
{
    if (IsBtIdle() == 0)
    {
        mpDebugPrint("\r\n Bt is NOT idle..ignore this actoin !!!");
        return;
    }
    if (MpxGetDeviceContextByIndex(index) == 0)
    {
        mpDebugPrint("\r\n This device index is invalid");
        return;
    }

#if 0 //mark this check temporarily, because it is easy to block action if no proper SetBtBusy(0) first
    if (GetBtBusy() == 1)
    {
        mpDebugPrint("\r\n Bt is busy...ingore this action !!!");
        return;
    }
    SetBtBusy(1);
#endif

    if (AllocListBuf() == 0)
        return;
    
#if (((BT_PROFILE_TYPE & BT_PUSH) == BT_PUSH) && (BT_OPUSH_AUTO_ENABLE == 1))
    if (BtGetOpushRegisterFlag() == TRUE)
    {
        MpxGoepOpushRegister();
    }
#endif
    MpxGoepFtpRegister(index);
    MpxGoepFtpClientConnect(index);
}


SetFtpCConnect(unsigned char flag)
{
    BTFTPC(FtpCConnect) = flag;
}


void MpxGoepOpushPull(unsigned char *pbuf)
{
    Set_CurrentGpaProfile(1);
    GoepOpushPull(pbuf);
}


void MpxSetFtpGpaComplete(int i)
{
    SetFtpGpaComplete(i);
}


int MpxGetFtpGpaComplete(void)
{
    return GetFtpGpaComplete();
}


void MpxSetGpaDoneComplete(int i)
{
    SetGpaDoneComplete(i);
}


int MpxGetGpaDoneComplete(void)
{
    GetGpaDoneComplete();
}


void MpxGoepFtpSetFolderForward(unsigned char *pbuf, int bl)
{
    if (GetFtpCConnect() == 0)
    {
        mpDebugPrint("\r\n Ftp client is NOT in progress...ingore this action !!!");
        return;
    }
    Set_CurrentGpaProfile(1);
    GoepFtpSetFolderForward(pbuf,bl);
}


void MpxGoepFtpSetFolderRoot(void)
{
    if (GetFtpCConnect() == 0)
    {
        mpDebugPrint("\r\n Ftp client is NOT in progress...ingore this action !!!");
        return;
    }
    Set_CurrentGpaProfile(1);
    GoepFtpSetFolderRoot();
}


void MpxGoepFtpSetFolderBackup(void)
{
    if (GetFtpCConnect() == 0)
    {
        mpDebugPrint("\r\n Ftp client is NOT in progress...ingore this action !!!");
        return;
    }
    Set_CurrentGpaProfile(1);
    GoepFtpSetFolderBackup();
}


void MpxGoepFtpDelete(unsigned char *pbuf)
{
    if (GetFtpCConnect() == 0)
    {
        mpDebugPrint("\r\n Ftp client is NOT in progress...ingore this action !!!");
        return;
    }   
    Set_CurrentGpaProfile(1);    
    GoepFtpDelete(pbuf);
}


void MpxGoepFtpPush(unsigned char *pbuf)
{
    if (GetFtpCConnect() == 0)
    {
        mpDebugPrint("\r\n Ftp client is NOT in progress...ingore this action !!!");
        return;
    }    
    Set_CurrentGpaProfile(1);    
    GoepFtpPush(pbuf);
}


void MpxGoepFtpClientConnect(unsigned long index)
{
    if (IsBtIdle() == 0)
    {
        mpDebugPrint("\r\n Bt is NOT idle..ignore this actoin !!!");
        return;
    }
    Set_CurrentGpaProfile(1);
    GoepFtpClientConnect(index);
}


void MpxGoepFtpClientDisconnect(void)
{
    if (GetFtpCConnect() == 0)
    {
        mpDebugPrint("\r\n Ftp client is NOT in progress...ingore this action !!!");
        return;
    }    
    Set_CurrentGpaProfile(1); 
    GoepFtpClientDisconnect();
}


void MpxGoepFtpRegister(unsigned long index)
{
    GoepFtpRegister(index);
}


void MpxGoepFtpClientPullFolderListing(void)
{
    if (GetFtpCConnect() == 0)
    {
        mpDebugPrint("\r\n Ftp client is NOT in progress...ingore this action !!!");
        return;
    }    
    Set_CurrentGpaProfile(1);
    GoepFtpClientPullFolderListing();
}


void MpxGpaAbortClient(void)
{
    if (GetFtpCConnect() == 0)
    {
        mpDebugPrint("\r\n Ftp client is NOT in progress...ingore this action !!!");
        return;
    }    
    Set_CurrentGpaProfile(1); 
    GPA_AbortClient();
}


int BtGetFtpRegisterFlag(void)
{
    return Get_FtpRegistered_Flag();
}


#define LIST_BUF_LEN    (4 * 1024)
int AllocListBuf(void)
{
	int status = 0;
#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
    if (BTFTPC(plist_buf) == NULL)
    {
        BTFTPC(plist_buf) = (BYTE *) ext_mem_malloc(LIST_BUF_LEN);
        if (BTFTPC(plist_buf) == NULL)
        {
            mpDebugPrint("\r\n AllocListBuf fail");
            return 0;
        }
    }
    memset(BTFTPC(plist_buf), 0, LIST_BUF_LEN);
    status = 1;
#endif

    return status;
}


void FreeListBuf(void)
{
#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
    if (BTFTPC(plist_buf) != NULL)
    {
        ext_mem_free(BTFTPC(plist_buf));
        BTFTPC(plist_buf) = NULL;
    }
#endif
}


void ResetListBuf(void)
{
#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
    BTFTPC(list_buf_size) = 0;
#endif
}

unsigned char * GetListBuf(void)
{
    unsigned char *ptr = NULL;

#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
    ptr = BTFTPC(plist_buf);
#endif

    return ptr;
}


void CopyListBuf(unsigned char *tBuf, unsigned long tLen)
{
#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
    memcpy(BTFTPC(plist_buf), tBuf, tLen);
    UartOutText((BYTE *) GetListBuf());
    memset(BTFTPC(plist_buf), 0, LIST_BUF_LEN);
#endif
}


void FtpGpaCallbackThread(void)
{
    if (MpxGetFtpGpaComplete())
    {
    	BTA(pXMessage)->eType = MPX_MESSAGE_FTPC_GPA_CALLBACK;
        BTA(pXMessage)->Mode = FTPC_MESSAGE_MODE;
        BTA(BtASynCallBack)((unsigned char *)BTA(pXMessage),sizeof(XpgMessage));
        MpxSetFtpGpaComplete(0);
    }
}


void FtpGpaDoneCallbackThread(void)
{
    if (MpxGetGpaDoneComplete())
    {
    	BTA(pXMessage)->eType = MPX_MESSAGE_FTPC_GPA_CALLBACK_DONE;
        BTA(pXMessage)->Mode = FTPC_MESSAGE_MODE;
        BTA(BtASynCallBack)((unsigned char *)BTA(pXMessage),sizeof(XpgMessage));
        MpxSetGpaDoneComplete(0);
    }
}
#endif //(BT_PROFILE_TYPE & BT_FTP_CLIENT)



#if (BT_PROFILE_TYPE & BT_A2DP)
int MpxGetA2dpRecordingFlag(void)
{
    return GetA2dpRecordingFlag();
}


void BtA2dpStreamList(void)
{
    A2dpAppStreamList();
}


void MpxBtA2dpConnect(unsigned long index)
{
    if (IsBtIdle() == 0)
    {
        mpDebugPrint("\r\n Bt is NOT idle..ignore this actoin !!!");
        return;
    }    
    if (MpxGetDeviceContextByIndex(index) == 0)
    {
        mpDebugPrint("\r\n This device index is invalid");
        return;
    }    

#if 0 //mark this check temporarily, because it is easy to block action if no proper SetBtBusy(0) first
    if (GetBtBusy() == 1)
    {
        mpDebugPrint("\r\n Bt is busy...ingore this action !!!");
        return;
    }
    SetBtBusy(1);
#endif

    BtA2DPReconfigInit();
    BtA2dpStreamList();
    BtA2dpStreamOpen(index);
}


void BtA2dpStreamOpen(unsigned long index)
{
    A2dpAppStreamOpen(index);
}


void BtA2dpStreamClose(void)
{
    if (GetA2dpConnect() == 0)
    {
        mpDebugPrint("\r\n a2dp is NOT connecting...ingore this action !!!");
        return;
    }
    A2dpAppStreamClose();
}


void BtA2dpStreamSuspend(void)
{
    A2dpAppStreamSuspend();
}


void BtA2dpStreamReconfig(void)
{
    A2dpAppStreamReconfig();
}


void BtA2dpStreamStart(void)
{
    A2dpAppStreamStart();
}


void BtA2dpSetSbcFregInfo(unsigned char i)
{
    AppSetSbcFreqInfo(i);
}


void BtA2dpSetA2dpSamplingRate(unsigned char i)
{
    SetA2dpSamplingRate(i);
}


unsigned char BtA2dpGetA2dpSamplingRate(void)
{
    return GetA2dpSamplingRate();
}


unsigned char Get_MP3_Srate(unsigned long srate)
{
    BYTE temp;
    if (srate == 48000)
     temp = 0;
    else if (srate == 44100)
     temp = 1;
    else if (srate == 32000)
     temp = 2;
    else if (srate == 16000)
     temp = 3;
    else
     temp = 1;

    MP_DEBUG("\r\n Get_MP3_Srate(BtApi.c) , sampleing rate %d %d\r\n",srate,temp);
    return temp;
}


void BtA2DPReconfigInit(void)
{
    BTA2DP(A2dp_Reconfig_Srate) = &BtA2DPReconfigStart;
}


void BtA2DPReconfigStart(void)
{
    BTA2DP(A2dp_Suspend) = &BtA2dpStreamSuspend;
    BTA2DP(A2dp_Reconfig) = &BtA2DPReconfigStream;
    BTA2DP(A2dp_Start) = &BtA2dpStreamStart;
    BTA2DP(A2dp_Suspend)();
}


void BtA2DPReconfigDeinit(void)
{
    BTA2DP(A2dp_Suspend) = NULL;
    BTA2DP(A2dp_Reconfig) = NULL;
    BTA2DP(A2dp_Start) = NULL;
    BTA2DP(A2dp_Reconfig_Srate) = NULL;
}


void BtA2DPReconfigStream(void)
{
    BtA2dpSetSbcFregInfo(BtA2dpGetA2dpSamplingRate());
    BtA2dpStreamReconfig();
}


void * GetA2dpStart(void)
{
    return (void *)BTA2DP(A2dp_Start);
}


void * GetA2dpReconfig(void)
{
    return (void *)BTA2DP(A2dp_Reconfig);
}


void SetA2dpStart(unsigned long func)
{
    BTA2DP(A2dp_Start) = (void *)func;
}


void SetA2dpReconfig(unsigned long func)
{
    BTA2DP(A2dp_Reconfig) = (void *)func;
}


void SetA2dpSuspend(unsigned long func)
{
    BTA2DP(A2dp_Suspend) = (void *)func;
}


void ExecuteA2dpStart(void)
{
    BTA2DP(A2dp_Start)();
}


void ExecuteA2dpReconfig(void)
{
    BTA2DP(A2dp_Reconfig)();
}


void ExecuteA2dpSuspend(void)
{
    BTA2DP(A2dp_Suspend)();
}


int GetAniFlag(void)
{
    return g_bAniFlag;
}


SetA2dpConnect(unsigned char flag)
{
    BTA2DP(A2dpConnect) = flag;
}


SetA2dpStreamStart(unsigned char flag)
{
    BTA2DP(A2dpStreamStart) = flag;
}


int GetA2dpStreamStart(void)
{
    return BTA2DP(A2dpStreamStart);
}
#endif //(BT_PROFILE_TYPE & BT_A2DP)


#if (BT_PROFILE_TYPE & BT_HF)
SetHfConnect(unsigned char flag)
{
    BTHF(HfConnect) = flag;
}


int MpxGetScoRecordingFlag(void)
{
    return GetScoRecordingFlag();
}


void MpxHfSereviceConnect(unsigned char index)
{
    if (IsBtIdle() == 0)
    {
        mpDebugPrint("\r\n Bt is NOT idle..ignore this actoin !!!");
        return;
    }    
    if (MpxGetDeviceContextByIndex(index) == 0)
    {
        mpDebugPrint("\r\n This device index is invalid");
        return;
    }    
    HfServiceConnect(index);
}


void MpxHfSereviceDisConnect(void)
{
    if (GetHfConnect() == 0)
    {
        mpDebugPrint("r\n Hand free is NOT connecting...ingore this action !!!");
        return;
    }
    HfServiceDisConnect();
}

void MpxHfAudioConnect(unsigned char index)
{
    if(MpxGetDeviceContextByIndex(index)==0)
    {
        mpDebugPrint("\r\n This device index is invalid");
        return;
    }    
    
    HfAudioConnect(index);
}

void MpxHfAudioDisConnect(void)
{
    if(GetHfConnect()==0)
    {
        mpDebugPrint("r\n Hand free is NOT connecting...ingore this action !!!");
        return;
    }    
    HfAudioDisConnect();
}

void MpxHfFindLocation(unsigned int Id1, unsigned int Id2)
{
    HfFindLocation(Id1, Id2);
}


void MpxHfFindPhoneBook(void)
{
    HfFindPhoneBook();
}


void MpxHfFindSim(void)
{
    HfFindSim();
}


void MpxHfFindDialledCalls(void)
{
    HfFindDialledCalls();
}


void MpxHfReceivedCalls(void)
{
    HfFindReceivedCalls();
}


void MpxHfFindMissedCalls(void)
{
    HfFindMissedCalls();
}


void MpxHfMicVolDown(void)
{
    HfMicVolDown();
}


void MpxHfMicVolUp(void)
{
    HfMicVolUp();
}


void MpxHfSpkrVolDown(void)
{
    HfSpkrVolDown();
}


void MpxHfSpkrVolUp(void)
{
    HfSpkrVolUp();
}


unsigned char MpxHfGetMicGain(void)
{
    return HfGetMicGain();
}


unsigned char MpxHfGetSpkrGain(void)
{
    return HfGetSpkrGain();
}


void MpxHfAnswer(void)
{
    HfAnswer();
}

void MpxHfNREC(void)
{
    HfNREC();
}

void MpxHfVoiceREC(char id)
{
    HfVoiceREC(id);
}

void MpxHfVoiceTag(void)
{
    HfVoiceTag();
}
void MpxHfHangUp(void)
{
    HfHangUp();
}


void MpxHfReleaseHeld(void)
{
    HfReleaseHeld();
}


void MpxHfReleaseActive(void)
{
    HfReleaseActive();
}

void MpxHfRelease(char id)
{
    HfRelease(id);
}

void MpxHfHoldActive(void)
{
    HfHoldActive();
}

void MpxHfPrivate(char id)
{
    HfPrivate(id);
}

void MpxHfAddHeld(void)
{
    HfAddHeld();
}

void MpxHfCallTransfer(void)
{
    HfCallTransfer();   
}    

void MpxHfDialNum(unsigned char * pbuf)
{
    HfDialNum(pbuf);
}


void MpxHfReDial(void)
{
    HfReDial();
}


void MpxHfAppWndProc(unsigned long index, unsigned int Cmd)
{
    if (MpxGetDeviceContextByIndex(index) == 0)
    {
        mpDebugPrint("\r\n This device index is invalid");
        return;
    }        
    HfAppWndProc(index, Cmd);
}


int MpxTabDlgProcCcs(unsigned int Cmd,unsigned long id,unsigned char *pbStr)
{
    TabDlgProcCcs(Cmd,id,pbStr);
}


int MpxTabDlgProcPhonebook(unsigned int Cmd, unsigned long id1, unsigned long id2)
{
    TabDlgProcPhonebook(Cmd, id1, id2);
}


unsigned long GetHfWaveInHciCallback(void)
{
    return GetHfWaveInProc();
}


void * GetHfWaveInUsbCallback(void)
{
#if (USBOTG_HOST_ISOC == 1)    
   return (void *)UsbOtgHostBtIsocDataOut;
#endif
}


char MpxAudioOpenInputDevice(unsigned short srate, unsigned char channels, unsigned char sampleSize, unsigned long (*fpHfWaveInCallback)(unsigned char *record_buf,unsigned long buf_len, unsigned char hci_size) )
{
    if (Audio_OpenInputDevice(srate, channels, sampleSize, fpHfWaveInCallback) != 0)
    {
        mpDebugPrint("\r\n MpxAudioOpenInputDevice fail");        
    }
}


void MpxAudioOpenOutputDevice(unsigned short srate, unsigned char channels, unsigned char sampleSize, unsigned char bigendian)
{
    if (Audio_OpenOutputDevice(srate, channels, sampleSize, bigendian) != 0)
    {
        mpDebugPrint("\r\n MpxAudioOpenOutputDevice fail");
    }
}


void MpxAudioCloseInputDevice(void)
{
    Audio_CloseInputDevice();
}


void MpxAudioCloseOutputDevice(void)
{
    Audio_CloseOutputDevice();
}

#endif //(BT_PROFILE_TYPE & BT_HF)


void MpxHandlerEnable(void)
{
    HandlerEnable(1);
}


////// This part is only prepare for calling bt Bt module
#if M_PROFILE
unsigned char * SetBufByDlci(unsigned char dlci)
{
    unsigned char i;
    unsigned char * pbuf = 0;
    mpDebugPrint("\r\n SetBufByDlci dlci %d",dlci);
    for (i=0; i < DLCI_BUF_NUM; i++)
    {
        mpDebugPrint("i %d,dlci %d",i,BTA(dlciBuf[i]).dlci);
        if (BTA(dlciBuf[i]).dlci == 0)
        {
            BTA(dlciBuf[i]).dlci = dlci;
            if (i == 0)
                pbuf = BTA(pWBuf0);
            else if (i== 1)
                pbuf = BTA(pWBuf1);
            BTA(dlciBuf[i]).pbuf = pbuf;
            mpDebugPrint("i %d, dlci %d, pbuf 0x%x", i, BTA(dlciBuf[i]).dlci, BTA(dlciBuf[i]).pbuf);
            break;
        }
    }
    if (pbuf == 0)
        mpDebugPrint("Warning : dlci buffer is full");
    return pbuf;
}


void ResetBufByDlci(unsigned char dlci)
{
    unsigned char i;
    mpDebugPrint("\r\n ResetBufByDlci dlci %d",dlci);
    for (i=0; i < DLCI_BUF_NUM; i++)
    {
        if (BTA(dlciBuf[i]).dlci == dlci)
        {
            BTA(dlciBuf[i]).dlci = 0;
            BTA(dlciBuf[i]).pbuf = 0;
            mpDebugPrint("i %d, dlci %d, pbuf 0x%x", i, BTA(dlciBuf[i]).dlci, BTA(dlciBuf[i]).pbuf);
            return;
        }
    }
    mpDebugPrint("Warning : dlci buffer have no this dlci record");
}


unsigned char * GetBufByDlci(unsigned char dlci)
{
    unsigned char i;
    unsigned char *pbuf = 0;
    mpDebugPrint("\r\n GetBufByDlci dlci %d",dlci);
    for (i=0; i < DLCI_BUF_NUM; i++)
    {
        mpDebugPrint("i %d,dlci %d",i,BTA(dlciBuf[i]).dlci);
        if (BTA(dlciBuf[i]).dlci == dlci)
        {
            mpDebugPrint("i %d, dlci %d, pbuf 0x%x", i, BTA(dlciBuf[i]).dlci, BTA(dlciBuf[i]).pbuf);
            pbuf = BTA(dlciBuf[i]).pbuf;
            break;
        }
    }
    if (pbuf == 0)
        mpDebugPrint("GetBufByDlci fail");
    return pbuf;
    
}
#endif//M_PROFILE


#if (Make_TESTCONSOLE == 1)
unsigned char* BtMemAlloc(unsigned char *pbuf, long len)
{
    long aLen = 0;
    aLen = len + 4;
    if ((len > 0) && pbuf)
    {
        unsigned char *pSbuf = (unsigned char *) ext_mem_malloc(aLen);        
        if (!pSbuf)
        {
            mpDebugPrint("\r\n BtMemAlloc(),pSbuf 0x%x fail",pSbuf);
            return NULL;
        }
        memset(pSbuf,0,aLen);
        MpMemCopy(pSbuf, pbuf, len);
        pSbuf = (long)pSbuf | 0xa0000000;
        mpDebugPrint("\r\n BtMemAlloc pSbuf 0x%x",pSbuf);
        return pSbuf;
    }
    mpDebugPrint("\r\n BtMemAlloc(),pbuf 0x%x , len 0x%x fail",pbuf,len);
    return NULL;
}


void BtTconDebugPrint(const char *fmt, ...)
{
#if 0
	int count;
	char cMsg[256];
	va_list args;

	va_start(args, fmt);		/* get variable arg list address */
	count = vsnprintf(cMsg, 255, fmt, args);	/* process fmt & args into buf */
	UartOutText((BYTE *) cMsg);
	UartOutText("\r\n");
#endif
}

MPX_KMODAPI_SET(RegisterUiMemAllocCallback);
MPX_KMODAPI_SET(DeRegisterUiMemAllocCallback);
MPX_KMODAPI_SET(BtMemAlloc);
MPX_KMODAPI_SET(BtApiBreakPoint);
MPX_KMODAPI_SET(HandlerEnable);
MPX_KMODAPI_SET(StringNCopy0816);
MPX_KMODAPI_SET(StringLength08);
MPX_KMODAPI_SET(StringLength16);
MPX_KMODAPI_SET(mpx_UtilUtf8ToUnicodeU16);
MPX_KMODAPI_SET(mpx_UtilU16ToU08);

MPX_KMODAPI_SET(BtGetOpushRegisterFlag);
MPX_KMODAPI_SET(SetFileIndex);
MPX_KMODAPI_SET(MpxBtCoreInit);
MPX_KMODAPI_SET(MpxBtCoreDeInit);
MPX_KMODAPI_SET(BtApiInit);
MPX_KMODAPI_SET(BtApiDeInit);
MPX_KMODAPI_SET(BtMessageReceiveInit);
MPX_KMODAPI_SET(BtMessageReceiveTaskInit);
MPX_KMODAPI_SET(BtMessageReceiveTaskDeinit);
MPX_KMODAPI_SET(MpxBMGInquiry);
MPX_KMODAPI_SET(MpxRxDialogCallback);
MPX_KMODAPI_SET(InitializeMessageMsg);
MPX_KMODAPI_SET(InquiryMessageMsg);
MPX_KMODAPI_SET(PairingMessageMsg);
MPX_KMODAPI_SET(ServiceMessageMsg);
MPX_KMODAPI_SET(OPushMessageMsg);
MPX_KMODAPI_SET(MpxGetDeviceName);
MPX_KMODAPI_SET(MpxDeleteKey);
MPX_KMODAPI_SET(SetInquiryFlag);
MPX_KMODAPI_SET(GetInquiryFlag);
MPX_KMODAPI_SET(SetOppConnect);
MPX_KMODAPI_SET(GetOppConnect);
MPX_KMODAPI_SET(SetBtBusy);
MPX_KMODAPI_SET(GetBtBusy);
MPX_KMODAPI_SET(MpxInqiryStoredDevice);
MPX_KMODAPI_SET(MpxBMGCancel);
MPX_KMODAPI_SET(BtTconDebugPrint);
MPX_KMODAPI_SET(MpxServiceSearch);
MPX_KMODAPI_SET(MpxGetServiceArray);
MPX_KMODAPI_SET(MpxSdpService);
MPX_KMODAPI_SET(MpxGoepOpushRegister);
MPX_KMODAPI_SET(MpxGoepOpushDisConnect);
MPX_KMODAPI_SET(MpxGoepOpushConnect);
MPX_KMODAPI_SET(MpxOpushGetVCard);
MPX_KMODAPI_SET(GetDdbNo);
MPX_KMODAPI_SET(SetDdbNo);
MPX_KMODAPI_SET(SetBtInitialize);
MPX_KMODAPI_SET(SetBtOn);
MPX_KMODAPI_SET(MpxGetBDCName);
MPX_KMODAPI_SET(MpxGetBDCState);
MPX_KMODAPI_SET(MpxGetBDCClass);
MPX_KMODAPI_SET(MpxPairingDevice_PinReq);
MPX_KMODAPI_SET(MpxCancelPairing);
MPX_KMODAPI_SET(MpxShowInquiryDatabase);
MPX_KMODAPI_SET(MpxME_SetClassOfDevice);
MPX_KMODAPI_SET(MpxME_SetAccessibleModeNC);
MPX_KMODAPI_SET(MpxME_SetAccessibleModeC);
MPX_KMODAPI_SET(MpxME_SetLocalDeviceName);
MPX_KMODAPI_SET(MpxReportLocalBdAddr);
MPX_KMODAPI_SET(MpxReportRemoteBdAddr);
MPX_KMODAPI_SET(GetPairingFlag);
MPX_KMODAPI_SET(SetPairingFlag);
//MPX_KMODAPI_SET(MpxMeCancelGetRemoteDeviceName);
MPX_KMODAPI_SET(MpxAppAclDisconnect);
MPX_KMODAPI_SET(MpxGoepTransferFile);
MPX_KMODAPI_SET(MpxGoepExchange);
MPX_KMODAPI_SET(MpxPairingDevice);
MPX_KMODAPI_SET(SetBtDefaultName);
MPX_KMODAPI_SET(IsBtIdle);
MPX_KMODAPI_SET(GetBtDefaultName);
MPX_KMODAPI_SET(MpxGoepOpushServerAbort);
MPX_KMODAPI_SET(MpxGoepOpushClientAbort);


// uart.h

MPX_KMODAPI_SET(UartOutText);
MPX_KMODAPI_SET(UartOutValue);
MPX_KMODAPI_SET(Uart_Init);

MPX_KMODAPI_SET(GetUartChar);
MPX_KMODAPI_SET(PutUartChar);
MPX_KMODAPI_SET(HUartInit);
MPX_KMODAPI_SET(HUartIntEnable);
MPX_KMODAPI_SET(HUartIntDisable);
MPX_KMODAPI_SET(HUartGetChar);
MPX_KMODAPI_SET(HUartPutChar);
MPX_KMODAPI_SET(HUartBufferRead);
MPX_KMODAPI_SET(HUartBufferWrite);
MPX_KMODAPI_SET(HUartDmaRead);
MPX_KMODAPI_SET(HUartDmaWrite);
MPX_KMODAPI_SET(HUartOutText);
MPX_KMODAPI_SET(HUartOutValueHex);
MPX_KMODAPI_SET(HUartOutValueDec);
MPX_KMODAPI_SET(HUartFollowCtrl);
MPX_KMODAPI_SET(HUartRegisterCallBackFunc);
MPX_KMODAPI_SET(HUartClearCallBackFunc);
MPX_KMODAPI_SET(HUartOutValue);

// memory.c
MPX_KMODAPI_SET(ext_mem_malloc);
MPX_KMODAPI_SET(ext_mem_free);

// filebrowser.c
MPX_KMODAPI_SET(BTTconGetCurFileLongFdbCount);

// Systimer.c
MPX_KMODAPI_SET(SystemTimerInit);
MPX_KMODAPI_SET(SystemTimeAlarmSet);
MPX_KMODAPI_SET(SystemTimeAlarmGet);
MPX_KMODAPI_SET(GetSysTime);
MPX_KMODAPI_SET(SystemTimeSet);
MPX_KMODAPI_SET(SystemTimeGet);
MPX_KMODAPI_SET(SystemGetElapsedTime);
MPX_KMODAPI_SET(SysTimerProcAdd);
MPX_KMODAPI_SET(SysTimerProcRemove);
MPX_KMODAPI_SET(SysTimerProcPause);
MPX_KMODAPI_SET(SysTimerProcResume);
MPX_KMODAPI_SET(SysTimerStatusGet);
MPX_KMODAPI_SET(get_cur_timeL);
MPX_KMODAPI_SET(get_elapsed_timeL);


// test console have use this function to set current index to replace UI
TestConsoleSetCurIndex(DWORD index)
{
    register ST_SYSTEM_CONFIG *psSysConfig;

    psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    mpDebugPrint("TestConsoleSetCurIndex");
    switch (psSysConfig->dwCurrentOpMode)
    {
        case OP_AUDIO_MODE:
            mpDebugPrint("audio");
            psFileBrowser->dwAudioCurIndex = index;
            break;

        default:
            mpDebugPrint("default");
            psFileBrowser->dwImgAndMovCurIndex = index;
            break;
    }
}


void BtSetCurIndex(unsigned long index)
{
    TestConsoleSetCurIndex(index);
}


void BtDeleteFile(void)
{
    FileBrowserDeleteFile();
}


void BtViewPhoto(void)
{
    xpgViewPhoto();
}


void BtEnterPhotoMenu(void)
{
    xpgCb_EnterPhotoMenu();
}


void BtScanFile(void)
{
    FileBrowserScanFileList(SEARCH_TYPE);
}


// Filebrowser.c
MPX_KMODAPI_SET(BtSetCurIndex);
MPX_KMODAPI_SET(BtDeleteFile);
// xpgPhotoFunc.c
MPX_KMODAPI_SET(BtViewPhoto);
//xpgBrowser.c
MPX_KMODAPI_SET(BtScanFile);
MPX_KMODAPI_SET(BtEnterPhotoMenu);
MPX_KMODAPI_SET(MpxGoepGetVCard);


#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
MPX_KMODAPI_SET(SetFtpSConnect);
MPX_KMODAPI_SET(GetFtpSConnect);
MPX_KMODAPI_SET(MpxFtxInit);
MPX_KMODAPI_SET(MpxFtxDeInit);
MPX_KMODAPI_SET(MpxFtxAbortServer);
MPX_KMODAPI_SET(FTPSMessageMsg);
#endif //(BT_PROFILE_TYPE & BT_FTP_SERVER)


#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
MPX_KMODAPI_SET(FTPCMessageMsg);
MPX_KMODAPI_SET(SetFtpCConnect);
MPX_KMODAPI_SET(GetFtpCConnect);
MPX_KMODAPI_SET(MpxGoepFtpRegister);
MPX_KMODAPI_SET(BtGetFtpRegisterFlag);
MPX_KMODAPI_SET(btFtpConnect);
MPX_KMODAPI_SET(MpxGoepFtpClientConnect);
MPX_KMODAPI_SET(MpxGoepOpushPull);
MPX_KMODAPI_SET(AllocListBuf);
MPX_KMODAPI_SET(FreeListBuf);
MPX_KMODAPI_SET(ResetListBuf);
MPX_KMODAPI_SET(CopyListBuf);
MPX_KMODAPI_SET(GetListBuf);
MPX_KMODAPI_SET(MpxGoepFtpClientPullFolderListing);
MPX_KMODAPI_SET(MpxGpaAbortClient);
MPX_KMODAPI_SET(MpxGoepFtpSetFolderForward);
MPX_KMODAPI_SET(MpxGoepFtpSetFolderBackup);
MPX_KMODAPI_SET(MpxGoepFtpClientDisconnect);
MPX_KMODAPI_SET(MpxGoepFtpDelete);
MPX_KMODAPI_SET(MpxGoepFtpPush);
#endif //(BT_PROFILE_TYPE & BT_FTP_CLIENT)



#if (BT_PROFILE_TYPE & BT_A2DP)

MPX_KMODAPI_SET(GetA2dpStart);
MPX_KMODAPI_SET(GetA2dpReconfig);
MPX_KMODAPI_SET(SetA2dpStart);
MPX_KMODAPI_SET(SetA2dpReconfig);
MPX_KMODAPI_SET(SetA2dpSuspend);
MPX_KMODAPI_SET(ExecuteA2dpStart);
MPX_KMODAPI_SET(ExecuteA2dpReconfig);
MPX_KMODAPI_SET(ExecuteA2dpSuspend);
MPX_KMODAPI_SET(GetAniFlag);
MPX_KMODAPI_SET(SetA2dpConnect);
MPX_KMODAPI_SET(GetA2dpConnect);
MPX_KMODAPI_SET(SetA2dpStreamStart);
MPX_KMODAPI_SET(GetA2dpStreamStart);
MPX_KMODAPI_SET(BtA2DPReconfigInit);
MPX_KMODAPI_SET(BtA2dpStreamList);
MPX_KMODAPI_SET(MpxBtA2dpConnect);
MPX_KMODAPI_SET(BtA2dpStreamOpen);
MPX_KMODAPI_SET(BtA2dpSetA2dpSamplingRate);
MPX_KMODAPI_SET(BtA2DPReconfigDeinit);
MPX_KMODAPI_SET(BtA2dpStreamClose);
MPX_KMODAPI_SET(BtA2dpStreamStart);
MPX_KMODAPI_SET(BtA2dpStreamSuspend);
MPX_KMODAPI_SET(BtA2DPReconfigStart);
MPX_KMODAPI_SET(A2DPMessageMsg);
MPX_KMODAPI_SET(AVRCPMessageMsg);
MPX_KMODAPI_SET(MpxGetA2dpRecordingFlag);
#endif //(BT_PROFILE_TYPE & BT_A2DP)



#if (BT_PROFILE_TYPE & BT_HF)
MPX_KMODAPI_SET(HfMessageMsg);
MPX_KMODAPI_SET(SetHfConnect);
MPX_KMODAPI_SET(GetHfConnect);
MPX_KMODAPI_SET(MpxHfSereviceConnect);
MPX_KMODAPI_SET(MpxHfAudioConnect);
MPX_KMODAPI_SET(MpxHfAudioDisConnect);
MPX_KMODAPI_SET(MpxHfSereviceDisConnect);
MPX_KMODAPI_SET(MpxHfFindLocation);
MPX_KMODAPI_SET(MpxHfFindPhoneBook);
MPX_KMODAPI_SET(MpxHfFindSim);
MPX_KMODAPI_SET(MpxHfFindDialledCalls);
MPX_KMODAPI_SET(MpxHfReceivedCalls);
MPX_KMODAPI_SET(MpxHfFindMissedCalls);
MPX_KMODAPI_SET(MpxHfMicVolDown);
MPX_KMODAPI_SET(MpxHfMicVolUp);
MPX_KMODAPI_SET(MpxHfSpkrVolDown);
MPX_KMODAPI_SET(MpxHfSpkrVolUp);
MPX_KMODAPI_SET(MpxHfGetMicGain);
MPX_KMODAPI_SET(MpxHfGetSpkrGain);
MPX_KMODAPI_SET(MpxHfAnswer);
MPX_KMODAPI_SET(MpxHfNREC);
MPX_KMODAPI_SET(MpxHfVoiceREC);
MPX_KMODAPI_SET(MpxHfVoiceTag);
MPX_KMODAPI_SET(MpxHfHangUp);
MPX_KMODAPI_SET(MpxHfReleaseHeld);
MPX_KMODAPI_SET(MpxHfReleaseActive);
MPX_KMODAPI_SET(MpxHfRelease);
MPX_KMODAPI_SET(MpxHfHoldActive);
MPX_KMODAPI_SET(MpxHfPrivate);
MPX_KMODAPI_SET(MpxHfAddHeld);
MPX_KMODAPI_SET(MpxHfCallTransfer);
MPX_KMODAPI_SET(MpxHfDialNum);
MPX_KMODAPI_SET(MpxHfReDial);
MPX_KMODAPI_SET(MpxHfAppWndProc);
MPX_KMODAPI_SET(MpxTabDlgProcCcs);
MPX_KMODAPI_SET(MpxTabDlgProcPhonebook);

MPX_KMODAPI_SET(MpxAudioOpenInputDevice);
MPX_KMODAPI_SET(MpxAudioOpenOutputDevice);
MPX_KMODAPI_SET(MpxAudioCloseInputDevice);
MPX_KMODAPI_SET(MpxAudioCloseOutputDevice);
MPX_KMODAPI_SET(GetHfWaveInHciCallback);
MPX_KMODAPI_SET(GetHfWaveInUsbCallback);

#endif //(BT_PROFILE_TYPE & BT_HF)


#if (BT_PROFILE_TYPE & BT_SPP)
// spps_mpx.c
MPX_KMODAPI_SET(SppMessageMsg);
MPX_KMODAPI_SET(MpxBtSppClientOpen);
MPX_KMODAPI_SET(MpxBtSppClientClose);
MPX_KMODAPI_SET(MpxBtSppClientRead);
MPX_KMODAPI_SET(MpxBtSppClientWrite);
MPX_KMODAPI_SET(MpxBtSppClientRxLen);
MPX_KMODAPI_SET(MpxBtSppClientConnect);
MPX_KMODAPI_SET(MpxBtSppClientDisconnect);
MPX_KMODAPI_SET(MpxBtSppServerOpen);
MPX_KMODAPI_SET(MpxBtSppServerRead);
MPX_KMODAPI_SET(MpxBtSppServerWrite);
MPX_KMODAPI_SET(MpxBtSppServerRxLen);
MPX_KMODAPI_SET(MpxBtSppServerClose);
MPX_KMODAPI_SET(MpxBtSppDeinit);
#endif //(BT_PROFILE_TYPE & BT_SPP)

#endif //(Make_TESTCONSOLE == 1)

#endif //(BLUETOOTH == ENABLE)

