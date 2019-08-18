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
* Filename      : BtApi.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/

#ifndef __BTAPI_H
#define __BTAPI_H


/*
// Include section 
*/

#include "btsetting.h"

#define PIN_CODE_REQUIRE    1
#define UI_DEVICE_NAME      "MPX DPF"

#if (BLUETOOTH == ENABLE)


// Bt Api Os function & parameter definition
typedef long (* BtApiEventCreate)(unsigned char id,unsigned char attr,unsigned long flag);
typedef long (* BtApiEventSet)(unsigned char id, unsigned long pattern);
typedef long (* BtApiEventWait)(unsigned char id, unsigned long pattern, unsigned char mode, unsigned long * release);
typedef long (* BtApiSemaphoreCreate)(unsigned char id,unsigned char attr,unsigned char count);
typedef long (* BtApiSemaphoreWait)(unsigned char id);
typedef long (* BtApiSemaphorePolling)(unsigned char id);
typedef long (* BtApiTaskCreate)(unsigned char id, void *entry, unsigned char priority, unsigned long stacksize);
typedef char (* BtApiTaskGetId)(void);
typedef long (* BtApiTaskStartup)(unsigned char id);
typedef long (* BtApiEventDestroy)(unsigned char id);
typedef long (* BtApiSemaphoreRelease)(unsigned char id);
typedef long (* BtApiSemaphoreDestroy)(unsigned char id);
typedef void (* BtApiTaskTerminate)(unsigned char id);

// Bt Api Timer function & parameter definition
typedef unsigned long (* BtApiGetSysTime)(void);
typedef long (* BtApiAddTimerProc)(register unsigned long dwOffsetValue, register void (*Action) (void),unsigned int isOneShot);
typedef long (* BtApiRemoveTimerProc)(register void (*Action) (void));

// Bt Api Fs function & parameter definition
typedef long (* BtApiFileCreate)(int * name);
//STREAM * BtBrowserFileCreate(WORD * pwSname);

typedef void (* BtApiFileClose)(long handle);
//void BtBrowserFileClose(STREAM *phandle);
typedef  unsigned long (* BtApiFileWrite)(unsigned long handle,unsigned char * buffer,unsigned long size);
//DWORD FileWrite(STREAM * handle, BYTE * buffer, DWORD size)

typedef unsigned char *(*BtUiMemAllocCallback)(unsigned char * pbuf,unsigned long len);
BtUiMemAllocCallback BtUiMemAlCallback;

typedef  short (* BtApiCheckSpace)(unsigned long size);

BtApiFileCreate   _BtApiFileCreate;
BtApiFileClose    _BtApiFileClose;
BtApiFileWrite    _BtApiFileWrite;
BtApiCheckSpace   _BtApiCheckSpace;


BtApiEventCreate        _BtApiEventCreate;
BtApiEventSet           _BtApiEventSet;
BtApiEventWait          _BtApiEventWait;
BtApiSemaphoreCreate    _BtApiSemaphoreCreate;
BtApiSemaphoreWait      _BtApiSemaphoreWait;
BtApiSemaphorePolling   _BtApiSemaphorePolling;
BtApiTaskCreate         _BtApiTaskCreate;
BtApiTaskGetId          _BtApiTaskGetId;
BtApiTaskStartup        _BtApiTaskStartup;
BtApiEventDestroy       _BtApiEventDestroy;
BtApiSemaphoreDestroy   _BtApiSemaphoreDestroy;
BtApiSemaphoreRelease   _BtApiSemaphoreRelease;
BtApiTaskTerminate      _BtApiTaskTerminate;
BtApiGetSysTime         _BtApiGetSysTime;
BtApiAddTimerProc       _BtApiAddTimerProc;
BtApiRemoveTimerProc    _BtApiRemoveTimerProc;


unsigned char _BT_OS_EVENT;
unsigned char _BT_OS_EVENT_ATTR;
unsigned char _BT_OS_EVENT_WAIT_ATTR;

unsigned char _BTstack_SEMA_ID;
unsigned char _BTstack_SEMA_ID_ATTR;

unsigned char _BThw_SEMA_ID;
unsigned char _BThw_SEMA_ID_ATTR;

unsigned char _BTir_SEMA_ID;
unsigned char _BTir_SEMA_ID_ATTR;

unsigned char _BT_OS_TASK;
unsigned char _BT_OS_TASK_PRIORITY;

#if (BT_PROFILE_TYPE & BT_A2DP)
unsigned char _BT_A2DP_EVENT;
unsigned char _BT_A2DP_TASK;
#endif

#if (BT_PROFILE_TYPE & BT_HF)
unsigned char _BT_SCO_EVENT;
unsigned char _BT_SCO_TASK;
unsigned char _BTwaveIn_SEMA_ID;
unsigned char _BTwaveOut_SEMA_ID_ATTR;
unsigned char _BTwaveOut_SEMA_ID;
unsigned char _BTwaveIn_SEMA_ID_ATTR;
unsigned char _BTpacket_SEMA_ID;
unsigned char _BTpacket_SEMA_ID_ATTR;
#endif

unsigned char _BT_A2DP_EVENT_ATTR;
unsigned char _BT_A2DP_EVENT_WAIT_ATTR;


unsigned char _BT_A2DP_TASK_PRIORITY;

// Bt Api Isp function & parameter definition
typedef int (* BtApiIspReadBTDevice)(unsigned char *trg, short size, short *fp);
typedef int (* BtApiIspWriteBTDevice)(unsigned char *data, unsigned short len);
BtApiIspReadBTDevice _IspReadBTDevice;
BtApiIspWriteBTDevice _IspWriteBTDevice;




/*
// Functions for external
*/

/////////////////////////
// Not provide for UI layer //
////////////////////////////////////////
int    boDDBWriteNor;
////////////////////////////////////////

#define MAX_FILE_LENGTH             (256 * 1024)
#define BT_SERVICE_STORE_COUNT      32

#if (FIXED_WRITE_BUFFER == 1)
extern unsigned char WriteBuf0[MAX_FILE_LENGTH];  
extern unsigned char WriteBuf1[MAX_FILE_LENGTH];
#endif

////////////////////////////////////////////////////////////////////////////////////////

/* Define test mode */
#define SET_EVENT_FILTER    1
#define WRITE_SCAN_ENABLE   2
#define EDUTM               3

/* The same in Medev.h for BtDeviceStatus */
#define BT_IN_RANGE                      0x01
#define BT_PAIRED                        0x02
#define BT_CONNECTED                     0x03 //in Me.h
#define BT_TRUSTED                       0x04

/*---------------------------------------------------------------------------
 * BtAccessibleMode type
 *
 *     Bluetooth Accessibility mode includes Discoverable and connectable
 *     modes.
 */
#define MPX_NOT_ACCESSIBLE       0x00 /* Non-discoverable or connectable */
#define MPX_GENERAL_ACCESSIBLE   0x03 /* General discoverable and connectable */
#define MPX_CONNECTABLE_ONLY     0x02 /* Connectable but not discoverable */



/*---------------------------------------------------------------------------- 
* BtDeviceStatus
*/

#define MPX_DS_IN_RANGE        0x01
#define MPX_DS_PAIRED          0x02
#define MPX_DS_TRUSTED         0x04


/* Group: Major device classes (Select one) */
/* The same in Me.h */
#define BT_MAJOR_MISCELLANEOUS       0x00000000
#define BT_MAJOR_COMPUTER            0x00000100
#define BT_MAJOR_PHONE               0x00000200
#define BT_MAJOR_LAN_ACCESS_POINT    0x00000300
#define BT_MAJOR_AUDIO               0x00000400
#define BT_MAJOR_PERIPHERAL          0x00000500
#define BT_MAJOR_IMAGING             0x00000600
#define BT_MAJOR_NULL                0x00000700     //Draw Nothing
#define BT_MAJOR_UNCLASSIFIED        0x00001F00


////////////////////////////////////////////////////////////////////////////////////////
// Definition  of message parameters ,type and mode
////////////////////////////////////////////////////////////////////////////////////////
#define INITILIZE_MESSAGE_MODE       0x01
#define INQUIRY_MESSAGE_MODE         0x02
#define PAIRING_MESSAGE_MODE         0x03
#define SERVICE_MESSAGE_MODE         0x04
#define OPUSH_MESSAGE_MODE           0x05
#define FTPC_MESSAGE_MODE            0x06
#define FTPS_MESSAGE_MODE            0x07
#define A2DP_MESSAGE_MODE            0x08
#define SPPS_MESSAGE_MODE            0x09
#define SPPC_MESSAGE_MODE            0x0a
#define AVRCP_MESSAGE_MODE           0x0b
#define HF_MESSAGE_MODE              0x0c

//Message of INITILIZE_MESSAGE_MODE      
enum
{
    MPX_MESSAGE_INITIALIZE_IDEL,
    MPX_MESSAGE_INITIALIZE_DONE,
    MPX_MESSAGE_LOCAO_BD_ADDRESS,
    MPX_MESSAGE_REMOTE_BD_ADDRESS,    
    MPX_MESSAGE_BD_NAME,
    MPX_MESSAGE_INITIALIZE_FATAL_ERROR,
    MPX_MESSAGE_INITIALIZE_INIT_ERROR,
};

//Message of INQUIRY_MESSAGE_MODE        
enum
{
    MPX_MESSAGE_INQUIRY_IDEL,
    MPX_MESSAGE_INQUIRY_ADD,
    MPX_MESSAGE_INQUIRY_ERASE,
    MPX_MESSAGE_INQUIRY_UPDATE,
    MPX_MESSAGE_INQUIRY_NAME_REQUEST,
    MPX_MESSAGE_INQUIRY_NAME_REQUEST_FAIL,
    MPX_MESSAGE_INQUIRY_COMPLETE,
};

//Message of PAIRING_MESSAGE_MODE
enum
{
    MPX_MESSAGE_PAIRING_IDEL,
    MPX_MESSAGE_PAIRING_PIN_REQ,
    MPX_MESSAGE_PAIRING_PIN_CANCEL,
    MPX_MESSAGE_PAIRING_COMPLETE,
    MPX_MESSAGE_PAIRING_CANCEL_PAIRED_DEVICE,
    MPX_MESSAGE_PAIRING_DELETE_PAIRED_DEVICE,
    MPX_MESSAGE_PAIRING_ADD_RECORDER,
    MPX_MESSAGE_PAIRING_DEV_KEY_EXISTED,
    MPX_MESSAGE_PAIRING_ACL_DISCONNECT,
};

//Message of SERVICE_MESSAGE_MODE
enum
{
    MPX_MESSAGE_SERVICE_IDEL,
    MPX_MESSAGE_SERVICE_NO_OBEX_SERVICE_FOUND,
    MPX_MESSAGE_SERVICE_STORE,
    MPX_MESSAGE_SERVICE_DRAW,
    MPX_MESSAGE_ACCEPT_CONNECT_DIALOGUE,
};

//Message of OPUSH_MESSAGE_MODE
enum
{
    MPX_MESSAGE_OPUSH_IDEL,
//    MPX_MESSAGE_OPUSH_RX_DIALOGUE,
    MPX_MESSAGE_OPUSH_SHOW_WELCOME_PAGE,
    MPX_MESSAGE_OPUSH_RECEIVE_DIALOGUE,
    MPX_MESSAGE_OPUSH_FILE_TRANSFER_DIALOGUE,
    MPX_MESSAGE_OPUSH_FSTORE_FINISH,
    MPX_MESSAGE_OPUSH_DRAW_STATUS,
    MPX_MESSAGE_OPUSH_CAL_TIME,
    MPX_MESSAGE_FILE_TRANSFER_FINISH,
    MPX_MESSAGE_OPUSHS_CONNECT,
    MPX_MESSAGE_OPUSHS_DISCONNECT,
    MPX_MESSAGE_OPUSHS_ABORT,
    MPX_MESSAGE_OPUSHS_PULL_COMPLETE,
    MPX_MESSAGE_OPUSHS_PUSH_COMPLETE,
    MPX_MESSAGE_OPUSHC_CONNECT,
    MPX_MESSAGE_OPUSHC_DISCONNECT,
    MPX_MESSAGE_OPUSHC_ABORT,
    MPX_MESSAGE_OPUSHC_PULL_COMPLETE,
    MPX_MESSAGE_OPUSHC_PUSH_COMPLETE,
};

//Message of FTPC_MESSAGE_MODE
enum
{
    MPX_MESSAGE_FTPC_IDEL,
    MPX_MESSAGE_FTPC_CONNECT,
    MPX_MESSAGE_FTPC_ABORT,
    MPX_MESSAGE_FTPC_DISCONNECT,
    MPX_MESSAGE_FTPC_LIST_BUF_INIT,
    MPX_MESSAGE_FTPC_LIST_BUF_COPY,
    MPX_MESSAGE_FTPC_GPA_CALLBACK,
    MPX_MESSAGE_FTPC_GPA_CALLBACK_DONE,
    MPX_MESSAGE_FTPC_FINISH_FILE_TRANSFER,
    MPX_MESSAGE_FTPC_PULL_FILE_FAIL,
    MPX_MESSAGE_FTPC_PULL_COMPLETE,
    MPX_MESSAGE_FTPC_PUSH_COMPLETE,
    MPX_MESSAGE_FTPC_SETPATH_COMPLETE,
};

//Message of FTPS_MESSAGE_MODE
enum
{
    MPX_MESSAGE_FTPS_IDEL,
    MPX_MESSAGE_FTPS_CONNECT,
    MPX_MESSAGE_FTPS_DISCONNECT,
    MPX_MESSAGE_FTPS_ABORT, 
    MPX_MESSAGE_FTPS_PULL_COMPLETE,
    MPX_MESSAGE_FTPS_PUSH_COMPLETE,
    MPX_MESSAGE_FTPS_SETPATH_COMPLETE,
    MPX_MESSAGE_FTPS_DELETE_COMPLETE,
};

//Message of A2DP_MESSAGE_MODE
enum
{
    MPX_MESSAGE_A2DP_IDEL,
    MPX_MESSAGE_A2DP_START_STREAM,
    MPX_MESSAGE_A2DP_STREAM_START,
    MPX_MESSAGE_A2DP_STREAM_START_FAIL,
    MPX_MESSAGE_A2DP_STREAM_SUSPEND,
    MPX_MESSAGE_A2DP_STREAM_RECONFIG,
    MPX_MESSAGE_A2DP_RECONFIG_STREAM,
    MPX_MESSAGE_A2DP_CLOSE_STREAM,
};

//Message of AVRCP_MESSAGE_MODE
enum
{
    MPX_MESSAGE_AVRCP_IDEL,
    MPX_MESSAGE_AVRCP_PLAY,
    MPX_MESSAGE_AVRCP_PAUSE,
    MPX_MESSAGE_AVRCP_STOP,
    MPX_MESSAGE_AVRCP_FORWARD,
    MPX_MESSAGE_AVRCP_FAST_FORWARD,
    MPX_MESSAGE_AVRCP_BACKWARD,
    MPX_MESSAGE_AVRCP_REWIND,
};


//Message of SPP_MESSAGE_MODE
enum
{
/*---------------------------------------------------------------------------
 * RfEvent type
 *
 *     All indications and confirmations are sent through a callback
 *     function as events. The type of event will determine which
 *     fields in the RfCallbackParms structure are valid.
 */

    MPX_MESSAGE_SPP_IDEL = 0,

/** A remote device has requested a connection to a local RFCOMM service.
 * Call RF_AcceptChannel to indicate whether the channel should be opened.  
 * This call can be made during or after the callback.
 */
    MPX_MESSAGE_SPP_OPEN_IND = 1,

/** A channel is now open and ready for data exchange. The remote device
 */
    MPX_MESSAGE_SPP_OPEN,

/** A request to close a channel was received. The remote device
 */
    MPX_MESSAGE_SPP_CLOSE_IND,

/** The channel is closed. The remote device
 */
    MPX_MESSAGE_SPP_CLOSED,

/** Data was received from the remote device.
 */
    MPX_MESSAGE_SPP_DATA_IND,

/** RFCOMM is finished with the data packet.
 * The RFCOMM user may reuse the packet structure to send more data.
 */
    MPX_MESSAGE_SPP_PACKET_HANDLED,

/** The remote device has requested negotiation of port settings.
 *
 * After reviewing all relevant parameters, the RFCOMM user must
 * call RF_AcceptPortSettings to indicate which parameters were accepted
 * or rejected.  Either RF_AcceptPortSettings or RF_DelayPortRsp must be 
 * called during the callback.  If RF_DelayPortRsp is called, then the
 * processing of this event can be handled outside the context of the
 * callback.  If processing is delayed, it is important that the application
 * call RF_AcceptPortSettings within 10 seconds to prevent a link disconnect
 * by the remote device.
 *  
 * If neither RF_AcceptPortSettings nor RF_DelayPortRsp are called during
 * the callback, the requested parameters are considered to be accepted
 * and a response will be generated automatically.  Port settings are only
 * used for RS232 emulation and have no real effect on the data format
 * or flow control at the RFCOMM layer.
 *
 * It is possible for a registered service to receive a port negotiation
 * before the channel is actually open (MPX_MESSAGE_SPP_OPEN). If a port
 * negotiation is never received by the application, default values
 * should be assumed: 9600 baud, 8 bits, 1 stop bit, no parity, and no
 * port flow control.
 */
    MPX_MESSAGE_SPP_PORT_NEG_IND,


#if (RF_SEND_CONTROL == XA_ENABLED)
/** A port settings negotiation request (started with RF_RequestPortSettings)
 * is now complete.
 *
 * This event is only available when RF_SEND_CONTROL is enabled.
 */
    MPX_MESSAGE_SPP_PORT_NEG_CNF,
#endif /* RF_SEND_CONTROL enabled */

/** The remote device has requested the status of the port settings.  The
 * application must respond with its current port settings by calling
 * RF_SendPortStatus.  Either RF_SendPortStatus or RF_DelayPortRsp must be 
 * called during the callback.  If RF_DelayPortRsp is called, then the
 * processing of this event can be handled outside the context of the
 * callback.  If processing is delayed, it is important that the application
 * call RF_SendPortStatus within 10 seconds to prevent a link disconnect
 * by the remote device.  
 *
 * If the application does not respond to this event during the callback, 
 * the response will be generated  automatically with the default Serial Port 
 * Profile settings of: 9600 baud, 8 data bits, 1 stop bit, and no parity.
 */
    MPX_MESSAGE_SPP_PORT_STATUS_IND,

#if (RF_SEND_CONTROL == XA_ENABLED)
/** The remote device has responded to a request for its current port
 * status.  This happens in response to RF_RequestPortStatus.
 *
 * This event is only available when RF_SEND_CONTROL is enabled.
 */
    MPX_MESSAGE_SPP_PORT_STATUS_CNF,
#endif /* RF_SEND_CONTROL enabled */

/** The remote device has provided modem status. The new status settings
 * are in "ptrs.modemStatus".
 */
    MPX_MESSAGE_SPP_MODEM_STATUS_IND,

/** The remote device has acknowledged new modem status settings. The RFCOMM
 * user sent these settings with the RF_SetModemStatus function.
 */
    MPX_MESSAGE_SPP_MODEM_STATUS_CNF,

#if (RF_SEND_CONTROL == XA_ENABLED)
/** The remote device has provided line status information.
 *
 * This event is only available when RF_SEND_CONTROL is enabled.
 */
    MPX_MESSAGE_SPP_LINE_STATUS_IND,

/** RFCOMM has finished sending line status to the remote device. The RFCOMM
 * user sent these settings with the RF_SetLineStatus function.
 *
 * This event is only available when RF_SEND_CONTROL is enabled.
 */
    MPX_MESSAGE_SPP_LINE_STATUS_CNF,
#endif /* RF_SEND_CONTROL enabled */

/** The remote device has indicated that no RFCOMM data can be processed.
 * This indication affects all RFCOMM channels.  This event will only
 * be received if credit-based flow control has not been negotiated.
 */
    MPX_MESSAGE_SPP_FLOW_OFF_IND,

/** The remote device has indicated that RFCOMM data can be processed again.
 * This indication affects all RFCOMM channels.  This event will only
 * be received if credit-based flow control has not been negotiated.
 */
    MPX_MESSAGE_SPP_FLOW_ON_IND,

/** A resource is available for use by RF_AcceptPortSettings or 
 *  RF_SendPortStatus.  If either of these functions returns 
 *  BT_STATUS_NO_RESOURCES, this event will be sent to indicate that a
 *  resource is now available.
 */
    MPX_MESSAGE_SPP_RESOURCE_FREE
};




//Message of HF_MESSAGE_MODE
enum
{
/** An incoming service level connection is being established.  This happens
 *  when the audio gateway establishes the service connection.
 *  The data connection is not available yet for issuing commands to the audio 
 *  gateway.  When the HF_EVENT_SERVICE_CONNECT event is received, the 
 *  channel is available for issuing commands.
 *
 *  When this callback is received, the "HfCallbackParms.p.remDev" field 
 *  contains a pointer to the remote device context.
 */
MPX_HF_EVENT_SERVICE_CONNECT_REQ = 0,

/** A service level connection has been established.  This can happen as the
 *  result of a call to HF_CreateServiceLink, or if the audio gateway 
 *  establishes the service connection.  When this event has been received, a 
 *  data connection is available for issuing commands to the audio gateway.  
 *
 *  When this callback is received, the "HfCallbackParms.p.remDev" field 
 *  contains a pointer to the remote device context.
 */
MPX_HF_EVENT_SERVICE_CONNECTED = 1,

/** The service level connection has been released.  This can happen as the
 *  result of a call to HF_DisconnectServiceLink, or if the audio gateway
 *  releases the service connection.  Communication with the audio gateways is 
 *  no longer possible.  In order to communicate with the audio gateway, a new 
 *  service level connection must be established.
 *
 *  This event can also occur when an attempt to create a service level 
 *  connection (HF_CreateServiceLink) fails.
 *
 *  When this callback is received, the "HfCallbackParms.p.remDev" field 
 *  contains a pointer to the remote device context.  In addition, the
 *  "HfCallbackParms.errCode" fields contains the reason for disconnect.
 */
MPX_HF_EVENT_SERVICE_DISCONNECTED = 2,

/** An audio connection has been established.  This event occurs whenever the
 *  audio channel (SCO) comes up, whether it is initiated by the audio gateway
 *  or the hands-free unit.
 *
 *  When this callback is received, the "HfCallbackParms.p.remDev" field 
 *  contains a pointer to the remote device context.
 */
MPX_HF_EVENT_AUDIO_CONNECTED = 3,

/** An audio connection has been released.  This event occurs whenever the
 *  audio channel (SCO) goes down, whether it is terminated by the audio gateway
 *  or the hands-free unit.
 *
 *  When this callback is received, the "HfCallbackParms.p.remDev" field 
 *  contains a pointer to the remote device context.  In addition, the
 *  "HfCallbackParms.errCode" fields contains the reason for disconnect.
 */
MPX_HF_EVENT_AUDIO_DISCONNECTED = 4,
 

/** Only valid if BT_SCO_HCI_DATA is set to XA_ENABLED.  Audio data has been 
 *  received from the remote device.  The data is only valid during the
 *  callback.
 *
 *  When this callback is received, the "HfCallbackParms.p.audioData" field 
 *  contains the audio data.
 */
MPX_HF_EVENT_AUDIO_DATA = 5,

/** Only valid if BT_SCO_HCI_DATA is set to XA_ENABLED.  Audio data has been 
 *  sent to the remote device.  This event is received by the application when
 *  the data sent by HF_SendAudioData has been successfully sent.
 *
 *  When this callback is received, the "HfCallbackParms.p.audioPacket" field 
 *  contains the result.
 */
MPX_HF_EVENT_AUDIO_DATA_SENT = 6,


/** After the service level connection has been established, this event will
 *  indicate the features supported on the audio gateway.  
 *
 *  When this callback is received, the "HfCallbackParms.p.features" field 
 *  contains the features (see HfFeatures).
 */
MPX_HF_EVENT_GATEWAY_FEATURES = 7,

/** After the service level connection has been established, this event will
 *  indicate the hold features (3-Way calling) supported on the audio gateway.  
 *
 *  When this callback is received, the "HfCallbackParms.p.holdFeatures" field 
 *  contains the features (see HfGwHoldFeatures).
 */
MPX_HF_EVENT_GW_HOLD_FEATURES = 8,

////////////////////////////////////////
//#if HF_USE_CALL_MANAGER == XA_ENABLED
////////////////////////////////////////

/** This event is generated only if HF_USE_CALL_MANAGER is defined as
 *  XA_ENABLED.  After the service level connection has been established, this 
 *  event will indicate the call state of the current connection.  Whenever the 
 *  call state changes, this event is generated.
 *
 *  When this callback is received, the "HfCallbackParms.p.callStateParms" field 
 *  contains the current call state (see HfCallStateParms).
 */                                     
MPX_HF_EVENT_CALL_STATE = 9,

/** This event is generated only if HF_USE_CALL_MANAGER is defined as
 *  XA_ENABLED.  The identification of the call has been received from the audio
 *  gateway.  
 * 
 *  When this callback is received, the "HfCallbackParms.p.callerId" 
 *  field contains a pointer to the ASCII string representation of the number 
 *  (NULL terminated).
 */ 
MPX_HF_EVENT_CALLER_ID = 10,

/** This event is generated only if HF_USE_CALL_MANAGER is defined as
 *  XA_ENABLED.  The Call Manager has determined the CLCC command is supported 
 *  by the device.  If this event is received, all call state information can be 
 *  considered reliable. 
 */
MPX_HF_EVENT_CALL_LISTING_ENABLED = 11,

//////////////////////////////////////////////////
//#else
//////////////////////////////////////////////////

/** This event is generated only if HF_USE_CALL_MANAGER is defined as
 *  XA_DISABLED.  After the service level connection has been established, this 
 *  event will indicate whether at least one call is active on the gateway
 *  device.  Whenever all calls are terminated, or when there were no calls and 
 *  a new call is created, this event is generated.
 *
 *  When this callback is received, the "HfCallbackParms.p.call" field 
 *  contains the current call state (see HfCallActiveState).
 */                                     
//#define HF_EVENT_CALL_IND              12

/** This event is generated only if HF_USE_CALL_MANAGER is defined as
 *  XA_DISABLED.  After the service level connection has been established, this 
 *  event will indicate the current call setup state.  Whenever the call setup
 *  state changes, this event is generated.
 *
 *  When this callback is received, the "HfCallbackParms.p.callSetup" field 
 *  contains the current call setup state (see HfCallSetupState).
 */                                     
//#define HF_EVENT_CALLSETUP_IND         13

/** This event is generated only if HF_USE_CALL_MANAGER is defined as
 *  XA_DISABLED.  After the service level connection has been established, this 
 *  event will indicate the current call held state.  Whenever the held
 *  state changes, this event is generated.
 *
 *  When this callback is received, the "HfCallbackParms.p.callHeld" field 
 *  contains the current call held state (see HfCallHeldState).
 */                                     
//#define HF_EVENT_CALLHELD_IND          14

/** This event is generated only if HF_USE_CALL_MANAGER is defined as
 *  XA_DISABLED.  When an incoming call is received on the audio gateway, this
 *  event is generated to indicate the incoming ring.
 */
//#define HF_EVENT_RING_IND              15

/** This event is generated only if HF_USE_CALL_MANAGER is defined as
 *  XA_DISABLED.  When call waiting is supported on the audio gateway and an
 *  incoming call is received while another call is active, this event is 
 *  received.
 *
 *  When this callback is received, the "HfCallbackParms.p.callWaitParms" field 
 *  contains informationa about the waiting call (see HfCallWaitParms).
 */                                     
//#define HF_EVENT_WAIT_NOTIFY           16

/** This event is generated only if HF_USE_CALL_MANAGER is defined as
 *  XA_DISABLED.  If caller ID notification is active, this event is received
 *  when an incoming call is detected on the audio gateway.
 *
 *  When this callback is received, the "HfCallbackParms.p.callerIdParms" field 
 *  contains the current caller ID informaiton (see HfCallerIdParms).
 */                                     
//#define HF_EVENT_CALLER_ID_NOTIFY      17

/** This event is generated only if HF_USE_CALL_MANAGER is defined as
 *  XA_DISABLED.  This event is received once for each call which exists on 
 *  the audio gateway.  This event is received after calling 
 *  HF_ListCurrentCalls().
 *
 *  When this callback is received, the "HfCallbackParms.p.callListParms" field 
 *  contains the current caller ID informaiton (see HfCallListParms).
 */                                     
//#define HF_EVENT_CURRENT_CALL_STATE    18

///////////////////////////////////////
//#if HF_USE_RESP_HOLD == XA_ENABLED
///////////////////////////////////////

/** This event is only available when HF_USE_RESP_HOLD is set to XA_ENABLED
 *  and HF_USE_CALL_MANAGER is set to XA_DISABLED.  The Response and Hold state
 *  has been received from the audio gateway.  This event is generated in 
 *  response to a call to HF_QueryResponseHold() or HF_ResponseHold().  
 * 
 *  When this callback is received, the "HfCallbackParms.p.respHold" field 
 *  contains the result.
 */
//#define HF_EVENT_RESPONSE_HOLD         19
//#endif
//#endif

/** The service indicator has been received from the audio gateway.
 * 
 *  When this callback is received, the "HfCallbackParms.p.service" 
 *  field contains a pointer to the service state.
 */ 
MPX_HF_EVENT_SERVICE_IND = 20,

/** The battery indicator has been received from the audio gateway.
 * 
 *  When this callback is received, the "HfCallbackParms.p.battery" 
 *  field contains a pointer to the battery level.
 */ 
MPX_HF_EVENT_BATTERY_IND = 21,

/** The signal strength indicator has been received from the audio gateway.
 * 
 *  When this callback is received, the "HfCallbackParms.p.signal" 
 *  field contains a pointer to the signal strength.
 */ 
MPX_HF_EVENT_SIGNAL_IND = 22,

/** The roam indicator has been received from the audio gateway.
 * 
 *  When this callback is received, the "HfCallbackParms.p.roam" 
 *  field contains a pointer to the roam state.
 */ 
MPX_HF_EVENT_ROAM_IND = 23,

/////////////////////////////////////////////
//#if HF_USE_MESSAGING_COMMANDS
/////////////////////////////////////////////

/** This indicator is only available if HF_USE_MESSAGING_COMMANDS is set to
 *  XA_ENABLED.  The 'SMS indicator has been received from the audio gateway.
 * 
 *  When this callback is received, the "HfCallbackParms.p.sms" 
 *  field contains a pointer to the message state.
 */ 
MPX_HF_EVENT_SMS_IND = 24,
///////////////////////////////////////////////
//#endif
///////////////////////////////////////////////

/** The voice recognition state has changed.  This event occurs if the
 *  audio gateway changes the state of voice recognition.
 *
 *  When this callback is received, the "HfCallbackParms.p.voiceRecognition" 
 *  field contains state of voice recognition.
 */
MPX_HF_EVENT_VOICE_REC_STATE = 25,

/** A number was returned in response to the HF_GetLastVoiceTag function.
 *
 *  When this callback is received, the "HfCallbackParms.p.data" field contains 
 *  a pointer to the ASCII string representation of the number (NULL terminated).
 */
MPX_HF_EVENT_VOICE_TAG_NUMBER = 26,

/** The speaker volume has been received from the audio gateway.
 */
MPX_HF_EVENT_SPEAKER_VOLUME = 27,

/** The mic volume has been received from the audio gateway.
 */
MPX_HF_EVENT_MIC_VOLUME = 28,

/** The in-band ring tone setting has been received from the audio gateway.
 * 
 *  When this callback is received, the "HfCallbackParms.p.inBandRing" 
 *  field contains a pointer to the In-Band ring state.
 */ 
MPX_HF_EVENT_IN_BAND_RING = 29,

/** The network operator string has been received from the remote device.
 * 
 *  When this callback is received, the "HfCallbackParms.p.networkOper" 
 *  field contains a pointer to the operator string state.
 */ 
MPX_HF_EVENT_NETWORK_OPERATOR = 30,

/** The susbscriber number has been received from the audio gateway.
 * 
 *  When this callback is received, the "HfCallbackParms.p.subscriberNum" 
 *  field contains a pointer to the subscriber number.
 */ 
MPX_HF_EVENT_SUBSCRIBER_NUMBER = 31,

/** The NO CARRIER event has been received from the audio gateway.
 */
MPX_HF_EVENT_NO_CARRIER = 32,

/** The BUSY event has been received from the audio gateway.
 */
MPX_HF_EVENT_BUSY = 33,

/** The NO ANSWER event has been received from the audio gateway.
 */
MPX_HF_EVENT_NO_ANSWER = 34,

/** The DELAYED event has been received from the audio gateway.
 */
MPX_HF_EVENT_DELAYED = 35,

/** The BLACKLISTED event has been received from the audio gateway.
 */
MPX_HF_EVENT_BLACKLISTED = 36,
//////////////////////////////////////////////////////////////////////
//#if HF_USE_PHONEBOOK_COMMANDS == XA_ENABLED
//////////////////////////////////////////////////////////////////////

/** The supported phonebook storage types have been received from the audio 
 *  gateway.  This event occurs in response to a call to HF_QueryPhonebooks.
 * 
 *  When this callback is received, the "HfCallbackParms.p.phonebooks" 
 *  field contains a bitmask of phonebook type supported on the gateway 
 *  (see HfPhonebooks).
 */ 
MPX_HF_EVENT_PHONEBOOK_STORAGE = 37,

/** The phonebook storage information has been received from the audio 
 *  gateway.  This event occurs in response to a call to 
 *  HF_GetCurrentPhonebooksInfo.  
 * 
 *  When this callback is received, the "HfCallbackParms.p.phonebookInfo" 
 *  field contains a pointer to a structure containing the phonebook information.
 */ 
MPX_HF_EVENT_PHONEBOOK_INFO = 38,

/** The number of phonebook entries has been received from the audio 
 *  gateway.  This event occurs in response to a call to 
 *  HF_GetPhonebookSize.  
 * 
 *  When this callback is received, the "HfCallbackParms.p.phonebookSize" field
 *  contains a pointer to a structure containing the phonebook information.
 */ 
MPX_HF_EVENT_PHONEBOOK_SIZE = 39,

/** A phonebook entry has been received from the audio gateway.  This event 
 *  occurs in response to a call to HF_ReadPhonebookEntries or 
 *  HF_FindPhonebookEntries. 
 * 
 *  When this callback is received, the "HfCallbackParms.p.phonebookEntry" field
 *  contains a pointer to a structure containing the phonebook information.
 */ 
MPX_HF_EVENT_PHONEBOOK_ENTRY = 40,
///////////////////////////////////////////////////////////////////////
//#endif
///////////////////////////////////////////////////////////////////////

/** An result code has been received from the audio gateway.  This event is 
 *  received for unsolicited result codes not handled by the internal 
 *  Hands-free AT parser.
 *
 *  When this callback is received, the "HfCallbackParms.p.data" field 
 *  contains the AT result code.
 */
MPX_HF_EVENT_AT_RESULT_DATA = 41,

/** A command to the audio gateway has completed.  This event is received
 *  when the processing a command is complete.
 *
 *  When this callback is received, the "HfCallbackParms.p.command" field 
 *  contains the command that was sent.  If "HfCallbackParms.status is set
 *  to BT_STATUS_FAILED, then "HfCallbackParms.p.command->cmeError" contains
 *  the command error (if known).
 */
MPX_HF_EVENT_COMMAND_COMPLETE = 42,
};


/** 3-way calling
 */
#define MPX_HF_GW_FEATURE_3_WAY              0x00000001

/** Echo canceling and/or noise reduction function
 */
#define MPX_HF_GW_FEATURE_ECHO_NOISE         0x00000002

/** Voice recognition function
 */
#define MPX_HF_GW_FEATURE_VOICE_RECOGNITION  0x00000004

/** In-band ring tone
 */
#define MPX_HF_GW_FEATURE_IN_BAND_RING       0x00000008

/** Voice tag
 */
#define MPX_HF_GW_FEATURE_VOICE_TAG          0x00000010

/** Reject a call
 */
#define MPX_HF_GW_FEATURE_CALL_REJECT        0x00000020

/** Enhanced Call Status
 */
#define MPX_HF_GW_FEATURE_ENH_CALL_STATUS    0x00000040

/** Enhanced Call Control
 */
#define MPX_HF_GW_FEATURE_ENH_CALL_CTRL      0x00000080

/* End of HfGatewayFeatures */


/** Releases all held calls or sets User Determined User Busy
 * (UDUB) for a waiting call.
 */
#define HF_GW_HOLD_RELEASE_HELD_CALLS      0x01

/** Releases all active calls (if any exist) and accepts the other
 * (held or waiting) call. 
 */
#define HF_GW_HOLD_RELEASE_ACTIVE_CALLS    0x02

/** Releases a specific call. */
#define HF_GW_HOLD_RELEASE_SPECIFIC_CALL   0x04

/** Places all active calls (if any exist) on hold and accepts the
 * other (held or waiting) call.
 */
#define HF_GW_HOLD_HOLD_ACTIVE_CALLS       0x08

/** Places a specific call on hold. */
#define HF_GW_HOLD_HOLD_SPECIFIC_CALL      0x10

/** Adds a held call to the conversation.
 */
#define HF_GW_HOLD_ADD_HELD_CALL           0x20

/** Connects the two calls and disconnects the AG from
 * both calls (Explicit Call Transfer).
 */
#define HF_GW_HOLD_CALL_TRANSFER           0x40

////////////////////////////////////////////////////////////////////////////////////////

/** An active call exists.
 */
#define MPX_HF_CALL_STATUS_ACTIVE     0

/** The call is held.
 */
#define MPX_HF_CALL_STATUS_HELD       1

/** A call is outgoing.  This state occurs when attempting a call using any
 *  of the dialing functions.
 */
#define MPX_HF_CALL_STATUS_DIALING    2

/** The remote party is being alerted.
 */
#define MPX_HF_CALL_STATUS_ALERTING   3

/** A call is incoming.  It can be answered with HFG_AnswerCall or 
 *  HFG_Hangup.  This state occurs when a call is being established by a
 *  remote party.
 */
#define MPX_HF_CALL_STATUS_INCOMING   4

/** The call is waiting.  This state occurs only when the audio gateway supports
 *  3-Way calling.
 */
#define MPX_HF_CALL_STATUS_WAITING    5

/** No active call
 */
#define MPX_HF_CALL_STATUS_NONE       0xFF

/** Unknow call state
 */
#define MPX_HF_CALL_STATUS_UNKNOWN    0xFE
/* End of HfCallStatus */






////////////////////////////////////////////////////////////////////////////////////////
enum
{
    IDLE = 0,
    BUSY = 1,    
};

////////////////////////////////////////////////////////////////////////////////////////



//FTP SERVER/////////////////////
typedef struct{
    unsigned char   ftp_fstore_read;    
    unsigned char   reserve[3];        
    unsigned long   current_receive_len;    
    unsigned long   total_file_len;
    unsigned long   status_len;     
    unsigned char   name[256];
    unsigned char   *pname;
    unsigned char   FtpSConnect;    
} ST_BT_FTPS_CONTEXT;
///////////////////////////////////////////////////////////

//FTP CLIENT/////////////////////
typedef struct{
    char            *plist_buf;
    unsigned long   list_buf_size;
    unsigned char   FtpCConnect; 
} ST_BT_FTPC_CONTEXT;
///////////////////////////////////////////////////////////

//A2DP//////////////////////////
typedef struct
{
    void (*A2dp_Suspend)(void);
    void (*A2dp_Reconfig)(void);
    void (*A2dp_Start)(void);    
    void (*A2dp_Reconfig_Srate)(void);   
    unsigned char A2dpConnect;
    unsigned char A2dpStreamStart;    
} ST_BT_A2DP_CONTEXT;


//Hf//////////////////////////
typedef struct
{
    unsigned char HfConnect;  
} ST_BT_HF_CONTEXT;


#define DLCI_BUF_NUM    2
typedef struct{
    char          dlci;
    unsigned char *   pbuf;

}ST_BT_DLCI_BUF;


////////////////////////////////////////////////////////////

#define BT_PIN_LEN                       16
#define BT_DEVICE_NUM                    20


/*
 * This structure is used to pass information to the EnterPinDlgProc().
 * The flags are the BtPairingType(s) defined in "sec.h" (BPT_).
 */
typedef struct _PinCodeRequest
{
    unsigned char     Pin[BT_PIN_LEN];
    const char        *DeviceDesc;
    unsigned char     PinLen;
    unsigned char     Flags;//ok->1 , cancel->0  ,for authorization confirm(iplay as slave) or authentication confirm(iplay master)	
    unsigned char     reserve[2];
} PinCodeReq;

typedef struct
{
    unsigned char     eType;
    unsigned char     Mode;
    unsigned char     *Buf;
    unsigned long     Len;
} XpgMessage;


typedef struct
{
    unsigned char     eType;
    unsigned char     Mode;
    unsigned char     index;
    unsigned char     ErrCode;    
} InquiryMessage;

typedef struct
{
    unsigned char     eType;
    unsigned char     Mode;
    unsigned char     *name;
    unsigned long     namesize;
    unsigned long     FileTotalLength;
    unsigned long     CurrentReceiveLength;    
} OpushMessage;

typedef struct
{
    unsigned char     eType;
    unsigned char     Mode;
    unsigned char     *number;
    unsigned char     *text;
    unsigned int      Id1;
    unsigned int      Id2;     
    unsigned char     signal;
    unsigned char     battery;   
    unsigned char     gain;       
    unsigned char     CallState;       
    unsigned char     Line; 
    char              *pData;    
    char              bData;
    int               wData;
    long              dwData;
    unsigned int      Total;
    unsigned int      Used;
} HfMessage;

typedef struct
{
    unsigned char       eType;
    unsigned char       mode;
    int                 index;

    unsigned short      state;
	unsigned long       service;
	unsigned long       MajorClass;
    char                *deviceName;  //the remote device name record not iplay
	PinCodeReq          Parms;
	void                *FileHandle;  //file handle to receive
	int                 FileLen;      //total file len	
	unsigned char       *filename;    //received file name
	int                 namelen;      //file name len
	int                 CurLen;       //real received len 
#if ((BT_PROFILE_TYPE & BT_FTP_CLIENT) == BT_FTP_CLIENT)
    unsigned char       *plisting;
    int                 listlen;
// for NONE_XPGEVENT_LIST_BUF_COPY callback event parameter pick up
    unsigned char       *buf;
    unsigned long long  len;
#endif
} XpgEvent;


typedef struct
{
    XpgEvent             gXpgEvent;
    XpgEvent             *pXpgEvent;

    XpgMessage           XMessage;
    XpgMessage           *pXMessage;

    InquiryMessage       IMessage;
    InquiryMessage       *pIMessage;

    OpushMessage         OMessage;
    OpushMessage         *pOMessage;

    HfMessage            HMessage;
    HfMessage            *pHMessage;

    unsigned char        ddb_no;    
    unsigned char        pairing; // 1 : busy , 0 : idle       
    unsigned char        inquiry;
    unsigned char        InquiryNameReq;
    
    unsigned char   btOn;        
    unsigned char   btInitialize;    
    unsigned char        acl_connect;
#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
    ST_BT_FTPC_CONTEXT   g_sBTFTPCContext;
    ST_BT_FTPC_CONTEXT   *g_psBTFTPCContext;
#endif    

#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
    ST_BT_FTPS_CONTEXT   g_sBTFTPSContext;
    ST_BT_FTPS_CONTEXT   *g_psBTFTPSContext;
#endif

#if (BT_PROFILE_TYPE & BT_A2DP)
    ST_BT_A2DP_CONTEXT   g_sBTA2DPContext;
    ST_BT_A2DP_CONTEXT   *g_psBTA2DPContext;
#endif    

#if (BT_PROFILE_TYPE & BT_HF)
    ST_BT_HF_CONTEXT   g_sBTHFContext;
    ST_BT_HF_CONTEXT   *g_psBTHFContext;
#endif    

    int (*BtSynCallBack)(XpgEvent *);
    void (*BtASynCallBack)(unsigned char *, unsigned long);
    unsigned char        *pWBuf0;
    unsigned char        *pWBuf1;
    unsigned char       OppConnect;
    unsigned char       StateBusy;    
#if M_PROFILE
    ST_BT_DLCI_BUF  dlciBuf[DLCI_BUF_NUM];
#endif
} ST_BT_API_CONTEXT;


ST_BT_API_CONTEXT   g_sBTAPIContext;
ST_BT_API_CONTEXT  *g_psBTAPIContext;

#define BTA(s)     (g_psBTAPIContext->s)
#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
#define BTFTPS(s)  (BTA(g_psBTFTPSContext)->s)
#endif

#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
#define BTFTPC(s)  (BTA(g_psBTFTPCContext)->s)
#endif

#if (BT_PROFILE_TYPE & BT_A2DP)
#define BTA2DP(s)  (BTA(g_psBTA2DPContext)->s)
#endif
             
#if (BT_PROFILE_TYPE & BT_HF)
#define BTHF(s)  (BTA(g_psBTHFContext)->s)
#endif

///
///@mainpage iPlay Bluetooth Module
///
/// This document will introduct API functioin of bluetooth module and how to use it.
///



//////////////////////////////////////////////////////////////////////////////////////////////////////////
///@defgroup BT_SYS System API
///
/// Bluetooth system parameter initialize function
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///@ingroup BT_SYS
///@brief     Initilize BT down layer OS and stack.
///@retval   int (BOOL value) : Return the status of operation.
///                                         1(TRUE) means initilize successfully
///                                         0(FALSE) means initilize fail
///@remark The main program. Initiailizes the Mini OS which in turn 
///              initializes the stack.  It then initializes the application, 
///              adds the application thread to the OS, and starts the Mini OS.
int MpxBtCoreInit(void);


///@ingroup BT_SYS
///@brief     De-initilize BT down layer OS and stack.
///@remark De-initiailizes the Mini OS , stacks , applications and application thread.
void MpxBtCoreDeInit(void);

///@ingroup BT_SYS
///@brief     Get bt device by index.
///@remark Get bt device by index.
unsigned long MpxGetDeviceContextByIndex(unsigned long index);


#if (BT_MULTI_TASK == 0)
///@ingroup BT_SYS
///@brief     BT mini OS task.   
///@remark This BT mini OS task will run all the thread which be added when execute MpxBtCoreInit() 
void MpxBtCoreMain(void);    
#endif


#define BT_UI_BUFFER_SIZE       1024
#define BT_EVENT_ARRAY_SIZE     1
#define BT_UI_TASK_ARRAY_SIZE   1


///@ingroup BT_SYS
///@brief     Register event id.
///@brif       BtEventIdArray for UI send UI EVENT 
///@param  i : number of  BtMessageIdArray , BtUiTaskIdArray , BtUiTaskFunArray.
///@param  event : number of message send  in bluetooth core layer.
///@param  task : number of task which will be create if register.
///@param  func : function point of task.
///@remark Register event id for sending in bluetooth core layer.
void BtMessageReceiveInit(unsigned char ui_event,unsigned int ui_event_tag,unsigned char i,unsigned char event,unsigned char task,unsigned long func);

///@ingroup BT_SYS
///@brief     Bluetooth upper layer init.
///@remark Create event , task of ui layer.
short BtMessageReceiveTaskInit(void);

///@ingroup BT_SYS
///@brief     Bluetooth upper layer de-init.
///@remark Terminate event , task of ui layer.
short BtMessageReceiveTaskDeinit(void);

///@ingroup BT_SYS
///@brief     Message send.
///@remark Message will send from bluetooth core layer.
void MpxBtSendMessage(unsigned char *event,unsigned long size);

///@ingroup BT_SYS
///@brief     Set Bt default UI name.
///@remark Set before Bt initilize.
void SetBtDefaultName(unsigned char * name);

///@ingroup BT_SYS
///@brief     Check bt state.
///@remark Check if bt is in idle state.
int IsBtIdle(void);

///@ingroup BT_SYS
///@brief     Api relative parameter initilize.
///@param  func : UI Layer must register call back of send message function. 
///@remark Initilize BT middle layer which belong to API all parameters.  
int BtApiInit(void * func);


///@ingroup BT_SYS
///@brief     Api relative parameter initilize.
///@remark De-initilize BT middle layer which belong to API all parameters.  
void BtApiDeInit(void);

///@ingroup BT_SYS
///@brief     Api relative parameter initilize.
///@remark Register memory allocated mechanism.  
void RegisterUiMemAllocCallback(BtUiMemAllocCallback callback);

///@ingroup BT_SYS
///@brief     Api relative parameter initilize.
///@remark De-egister memory allocated mechanism.  
void DeRegisterUiMemAllocCallback(void);

///@ingroup BT_SYS
///@brief     
///@remark 
unsigned char* BtMemAlloc(unsigned char *pbuf, long len);

///@ingroup BT_SYS
///@brief SetPairingFlag   
///@remark   Provide for UI layer to set pairing flag.
///                Set to 1  means pairing procedure is excuted and busy. 
///                Set to 0 means pair procedure is idle.
void SetPairingFlag(unsigned char flag);

// Not provide for UI layer calling 
unsigned char GetPairingFlag(void);

///@ingroup BT_SYS
///@brief SetInquiryFlag   
///@remark   Provide for UI layer to set inquiry flag when execute inquire procedure.. 
///                Set to 1 means inquiry procedure is excuted and busy. 
///                Set to 0 means inquiry procedure is idle.
void SetInquiryFlag(unsigned char flag);


///@ingroup BT_SYS
///@brief GetInquiryFlag   
///@remark   Provide for UI layer to get inquiry flag. 
///                return  1 means inquiry procedure is excuted and busy. 
///                return  0 means inquiry procedure is idle.
unsigned char GetInquiryFlag(void);



///@ingroup BT_SYS
///@brief     Get BT device address or device name.   
///@param   index : BT device number index.
///@retval    return BT device address or device name.
///@remark  UI layer can get BT device address or device name when execute inquiry procedure by index. 
char* MpxGetBDCName(unsigned char index);


///@ingroup BT_SYS
///@brief     Get BT device state.   
///@param   index : BT device number index.
///@retval    return BT device state.
///@remark  UI layer can get BT device state when execute inquiry procedure by index. 
unsigned short MpxGetBDCState(unsigned char index);                   


///@ingroup BT_SYS
///@brief     Get BT class.   
///@param   index : BT device number index.
///@retval    return BT device class.
///@remark  UI layer can get BT device class when execute inquiry procedure by index. 
unsigned short MpxGetBDCClass(unsigned char index);


///@ingroup BT_SYS
///@brief     Get BT class.   
///@retval    return BT device class.
///@remark  UI Layer can get relative device name information for pop out 
///               dialogue when surronding BT try to connect or transfer file. 
unsigned char *MpxGetDeviceName(void);  



//FTP CLIENT///////////////////////////////
#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)   
// Not for UI layer , provide for calling by Eventmgr.c.
// This function will sned MPX_MESSAGE_FTPC_GPA_CALLBACK event and only for when ftp client mode execute
void FtpGpaCallbackThread(void);


// Not for UI layer , provide for calling by Eventmgr.c.
// This function will sned MPX_MESSAGE_FTPC_GPA_CALLBACK_DONE event and only for when ftp client mode execute
void FtpGpaDoneCallbackThread(void);
#endif

//A2DP////////////////////////////////////
// Not for UI layer , provide for calling by Eventmgr.c.
// This function will sned MPX_MESSAGE_FTPC_FINISH_FILE_VIEWING event when file transfer finish.
// When UI layer receive this event can decide view this receive file or not. And do next UI flow.
void FinishFileTranThread(void);


//////////////////////////////////////////////////////////////////////////////////////////////////////////
///@defgroup BT_INQUIRY Inquiry API                                                                                                                                  
///                                                                                                                                                                              
/// Bluetooth Inquiry API function                                                                                                                                     
//////////////////////////////////////////////////////////////////////////////////////////////////////////


///@ingroup BT_INQUIRY
///@brief      Enter BT inquiry procedure.
///@remark  Issue this command , BT device will ask surrounding Bt device one by one.
void MpxBMGInquiry(void);

///@ingroup BT_INQUIRY
///@brief      Show BT inquiry database.
///@remark  Not need to issue inquiry command can review last inquiry device information.
void MpxShowInquiryDatabase(void);

///@ingroup BT_INQUIRY
///@brief      Enter BT inquiry procedure.
///@remark  Issue this command , BT device will ask surrounding Bt paired device one by one.
void MpxInqiryStoredDevice(void);

///@ingroup BT_INQUIRY
///@brief      Enter BT pairing procedure. 
///@param   index : index of bluetooth device.
///@remark  Pairing device can setup trust relationship with the paired device. And produce a link key which will be recored.
void MpxPairingDevice(unsigned long index);


///@ingroup BT_INQUIRY
///@brief      Cancel pairing procedure.
///@remark  Cancel pairing procedure.
void MpxBMGCancel(void);


///@ingroup BT_INQUIRY
///@brief      Delete link key 
///@param   index : index of bluetooth device.
///@remark  If execute this command , the paired device will enter un-pair state.
void MpxDeleteKey(unsigned long index);


///@ingroup BT_INQUIRY
///@brief      Cancel pairing progress 
///@remark  If execute this command , the pairing progress will be stoped.
void MpxCancelPairing(void);

///@ingroup BT_INQUIRY
///@brief      Pairing device with pin code 
///@param   index : index of bluetooth device.
///@remark  Executing pairing device command with pin code from UI layer.
void MpxPairingDevice_PinReq(PinCodeReq * SourcePara,unsigned long index);

///@ingroup BT_INQUIRY
///@brief      Get service arrayof BT device. 
///@param   unsigned char * source :Input the buffer to be allocated the service array.
///@remark  Before execute A2DP connect or Ftp client connect , service search must execute first. 
void MpxGetServiceArray(unsigned char * source);

///@ingroup BT_INQUIRY
///@brief      Get service string of BT device. 
///@param   unsigned char * source :Input the service index.
///@remark  Before execute A2DP connect or Ftp client connect , service search must execute first. 
const char *MpxSdpService(unsigned long Service);

///@ingroup BT_INQUIRY
///@brief      Serach service of BT device. 
///@param   index : index of bluetooth device. 
///@remark  Before execute A2DP connect or Ftp client connect , service search must execute first. 
void MpxServiceSearch(unsigned long index);


///@ingroup BT_INQUIRY
///@brief      Opush profile register. 
///@remark  Before execute opush profile , opush profile must be registered first.
void MpxGoepOpushRegister(void);

///@ingroup BT_INQUIRY
///@brief      Set local device name. 
///@remark  Set local device name.
void MpxME_SetLocalDeviceName(unsigned char* pbuf,unsigned char len);

///@ingroup BT_INQUIRY
///@brief      
///@remark  
int BtGetOpushRegisterFlag(void);

///@ingroup BT_INQUIRY
///@brief      Get local bd addr. 
///@remark  Get local bd addr.
void MpxReportLocalBdAddr(void);

///@ingroup BT_INQUIRY
///@brief      Get remote bd addr. 
///@param   index : index of bluetooth device. 
///@remark  Get remote bd addr.
void MpxReportRemoteBdAddr(unsigned char index);

///@ingroup BT_INQUIRY
///@brief      Set accessible mode. 
///@remark  Can set bt accessible or non-accessible mode when not connected.
void MpxME_SetAccessibleModeNC(unsigned char mode);

///@ingroup BT_INQUIRY
///@brief      Set accessible mode. 
///@remark  Can set bt accessible or non-accessible mode when connected.
void MpxME_SetAccessibleModeC(unsigned char mode);

///@ingroup BT_INQUIRY
///@brief     Dis-connect an ACL connection. 
///@remark After execute inquiry procedure , ACL connection must be dis-connect. 
void MpxAppAclDisconnect(void);


///@ingroup BT_INQUIRY
///@brief      UI layer dialogue for incoming coccectioin.
///@remark  When other BT device try to send file or connect. BT stack down layer will send message to inform UI layer.
///               UI layer can choose yes or no.
void MpxRxDialogCallback(unsigned char rx_dialogue);



//////////////////////////////////////////////////////////////////////////////////////////////////////////
///@defgroup BT_OPUSH OPUSH API                                                                                                                                  
///                                                                                                                                                                              
/// Bluetooth OPUSH API function                                                                                                                                     
//////////////////////////////////////////////////////////////////////////////////////////////////////////


///@ingroup BT_OPUSH
///@brief     Transfer one single file.   
///@param   char * name : name of file which want to transfer.
///@param    unsigned long DeviceIndex : inde of bluetooth device.
///@remark  UI layer can transfer one to other BT device by issuing this command with this file name.
void MpxGoepTransferFile(char *name,unsigned long DeviceIndex);

///@ingroup BT_OPUSH
///@brief     Exchange.   
///@param    unsigned long DeviceIndex : inde of bluetooth device.
///@remark  None
void MpxGoepExchange(unsigned long DeviceIndex);

///@ingroup BT_OPUSH
///@brief     Pull vCard.
///@param  None.
///@remark Pull vCard.
void MpxGoepGetVCard(void);  


///@ingroup BT_OPUSH
///@brief      Opush server mode abort.
///@remark  UI layer can abort by issuing this command when receiving file with opush profile.
void MpxGoepOpushServerAbort(void);


///@ingroup BT_OPUSH
///@brief      Opush client mode abort
///@remark  UI layer can abort by issuing this command when transfer file to other BT device with opush profile.
void MpxGoepOpushClientAbort(void);

///@ingroup BT_OPUSH
///@brief      Set class of device
///@remark  UI layer can set class of device by issuing this command.
void MpxME_SetClassOfDevice(unsigned long classOfDevice);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///@defgroup BT_FTPC FTP Client API                                                                                                                                  
///                                                                                                                                                                              
/// Bluetooth FTP Client API function                                                                                                                                     
//////////////////////////////////////////////////////////////////////////////////////////////////////////



// Not provide for UI layer
void MpxSetFtpGpaComplete(int i);
int MpxGetFtpGpaComplete(void);
void MpxSetGpaDoneComplete(int i);
int MpxGetGpaDoneComplete(void);



#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
///@ingroup BT_FTPC
///@brief     
///@remark 
int BtGetFtpRegisterFlag(void);

///@ingroup BT_FTPC
///@brief     Enter the folder which has been choose.
///@param  unsigned char *pbuf : name of folder.
///@param  int bl : Allow create flag. 1 means allow , 0 means not allow.
///@remark BT device side will know client side wnat to enter a specific filder.
void MpxGoepFtpSetFolderForward(unsigned char *pbuf,int bl);


///@ingroup BT_FTPC
///@brief     Back to root folder
///@remark Issuing this command can inform BT device server side back to root folder.
void MpxGoepFtpSetFolderRoot(void);


///@ingroup BT_FTPC
///@brief     Upper the current folder.
///@remark Issuing this command can inform BT device server side back to upper folder.
void MpxGoepFtpSetFolderBackup(void);


///@ingroup BT_FTPC
///@brief     Delete one file on server side.
///@param  unsigned char *pbuf : name of the file which to be deleted.
///@remark BT device client mode can actively delete one specific file on server side.
void MpxGoepFtpDelete(unsigned char *pbuf);


///@ingroup BT_FTPC
///@brief     Push one single file to BT device server side.
///@param  unsigned char *pbuf : name of the file which to be push to server side. 
///@remark Push means BT device client actively transfer one specific file to server side.
void MpxGoepFtpPush(unsigned char *pbuf);


///@ingroup BT_FTPC
///@brief     Pull one single file from BT device server side.
///@param  unsigned char *pbuf : name of the file which to be transfer to BT devie client side from servre side.. 
///@remark Pull meas BT device client side can actively select one specific file and grab it from server side.  
void MpxGoepOpushPull(unsigned char *pbuf);  

///@ingroup BT_FTPC
///@brief     Ftp client mode connect.
///@param unsigned long index : index of bluetooth device.
///@remark After issuing this command , the selected BT device will be connected.
void MpxGoepFtpClientConnect(unsigned long index);


///@ingroup BT_FTPC
///@brief     Ftp client mode dis-connect.
///@remark After issuing this command , the ftp client connection established will be dis-connect.  
void MpxGoepFtpClientDisconnect(void);


///@ingroup BT_FTPC
///@brief     Register Ftp profile.
///@param  unsigned long index : index of bluetooth device.
///@remark Before execute ftp client mode , ftp profile must be registered first.
void MpxGoepFtpRegister(unsigned long index);


///@ingroup BT_FTPC
///@brief     Get content of folder.
///@remark After issing this command. The content of current folder in the BT device server side will be Get.
void MpxGoepFtpClientPullFolderListing(void);

///@ingroup BT_FTPC
///@brief     Abort current Client operation.
///@remark After issing this command. The content of current operation will abort.
void MpxGpaAbortClient(void);
#endif


#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
///@ingroup BT_FTPS
///@brief     FtpS mount.
///@remark FtpS mount.
int MpxFtxInit(void);

///@ingroup BT_FTPS
///@brief     FtpS de-mount.
///@remark FtpS de-mount.
void MpxFtxDeInit(void);

///@ingroup BT_FTPS
///@brief     Abort current server operation.
///@remark After issing this command. The content of operation will abort.
void MpxFtxAbortServer(void);
#endif


#if (BT_PROFILE_TYPE & BT_A2DP)
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///@defgroup BT_A2DP A2DP API                                                                                                                                  
///                                                                                                                                                                              
/// Bluetooth A2DP API function                                                                                                                                     
//////////////////////////////////////////////////////////////////////////////////////////////////////////


///@ingroup BT_A2DP
///@brief      A2dp connect.
///@remark  This function will connect a2dp automatically.
void MpxBtA2dpConnect(unsigned long);


///@ingroup BT_A2DP
///@brief      Provide for reconfig A2DP stream.
///@remark  This function will register BtA2DPReconfigStart for stream reconfig use.
///               
void BtA2DPReconfigInit(void);

///@ingroup BT_A2DP
///@brief      Provide for get mp3 sapmling rate.
///@remark  This function will return index by mp3 sapmling rate to match with BT down layer A2DP index.
///               
unsigned char Get_MP3_Srate(unsigned long srate);


///@ingroup BT_A2DP
///@brief     Provide for initialize reconfig A2DP stream.
///@remark  This function will register A2DP stream suspend , reconfig , start function point and execute automatically when 
///               dpf play different sampling rate mp3 in bluetooth down layer A2DP stack. 
void BtA2DPReconfigStart(void);

///@ingroup BT_A2DP
///@brief     Provide for de-initilize reconfig A2DP stream.
///@remark  This function will de-register A2DP stream suspend , reconfig , start function point which will execute automatically when 
///               dpf play different sampling rate mp3 in bluetooth down layer A2DP stack. 
void BtA2DPReconfigDeinit(void);

///@ingroup BT_A2DP
///@brief     Provide for reconfig A2DP stream. 
///@remark  This function will get sampling rate of next mp3 and re-set this value for sbc engine.
///               And issue reconfig A2DP stream command.
void BtA2DPReconfigStream(void);

///@ingroup BT_A2DP
///@brief     Set A2DP strream list.
///@remark Before open A2DP stream , set stream list must be executed.
void BtA2dpStreamList(void);

///@ingroup BT_A2DP
///@brief     Open A2DP stream
///@param  index : index of bluetooth device. 
///@remark A2DP will be connected after execute stream open.
void BtA2dpStreamOpen(unsigned long index);

///@ingroup BT_A2DP
///@brief     Close A2DP Stream
///@remark A2DP connecttion will be dis-connected after execute stream close.
void BtA2dpStreamClose(void);


///@ingroup BT_A2DP
///@brief     Suspend A2DP stream.
///@remark After complete this command A2DP stream will suspend but still not close. 
void BtA2dpStreamSuspend(void);


///@ingroup BT_A2DP
///@brief     Re-config A2DP stream.     
///@remark Issue this command re-config procedure will be executed.
void BtA2dpStreamReconfig(void);


///@ingroup BT_A2DP
///@brief     Start A2DP Stream.     
///@remark Start an A2DP stream which has been opened.
void BtA2dpStreamStart(void);

///@ingroup BT_A2DP
///@brief     Set frequency information to A2DP down layer.
///@remark Sbc engine must be informed and reset information when different frequency are going to play.
void BtA2dpSetSbcFregInfo(unsigned char i);

///@ingroup BT_A2DP
///@brief     Set sampling rate information to A2DP down layer.
///@remark Sbc engine must be informed and reset information when different sampling rate are going to play.
void BtA2dpSetA2dpSamplingRate(unsigned char i);

void BtFastForwardMusic(void);
#endif


#if (BT_PROFILE_TYPE & BT_HF)
///@ingroup BT_HandFree
///@brief     HandFree service connect.
///@remark HandFree service connect.
void MpxHfSereviceConnect(unsigned char index);

///@ingroup BT_HandFree
///@brief     HandFree service disconnect.
///@remark HandFree service disconnect.
void MpxHfSereviceDisConnect(void);

///@ingroup BT_HandFree
///@brief     HandFree audio connect.
///@remark HandFree audio connect.
void MpxHfAudioConnect(unsigned char index);

///@ingroup BT_HandFree
///@brief     HandFree audio disconnect.
///@remark HandFree audio disconnect.
void MpxHfAudioDisConnect(void);

///@ingroup BT_HandFree
///@brief     HandFree Find Location.
///@remark HandFree Find Location.
void MpxHfFindLocation(unsigned int Id1,unsigned int Id2);

///@ingroup BT_HandFree
///@brief     HandFree Find PhoneBook.
///@remark HandFree Find PhoneBook.
void MpxHfFindPhoneBook(void);

///@ingroup BT_HandFree
///@brief     HandFree Find Sim.
///@remark HandFree Find Sim.
void MpxHfFindSim(void);

///@ingroup BT_HandFree
///@brief     HandFree Find DialledCalls.
///@remark HandFree Find DialledCalls.
void MpxHfFindDialledCalls(void);

///@ingroup BT_HandFree
///@brief     HandFree Find ReceivedCalls.
///@remark HandFree Find ReceivedCalls.
void MpxHfReceivedCalls(void);

///@ingroup BT_HandFree
///@brief     HandFree Find MissedCalls.
///@remark HandFree Find MissedCalls.
void MpxHfFindMissedCalls(void);

///@ingroup BT_HandFree
///@brief     HandFree MicVolDown.
///@remark HandFree MicVolDown.
void MpxHfMicVolDown(void);

///@ingroup BT_HandFree
///@brief     HandFree MicVolUp.
///@remark HandFree MicVolUp.
void MpxHfMicVolUp(void);

///@ingroup BT_HandFree
///@brief     HandFree Get SpkrVolDown.
///@remark HandFree Find SpkrVolDown.
void MpxHfSpkrVolDown(void);

///@ingroup BT_HandFree
///@brief     HandFree SpkrVolUp.
///@remark HandFree SpkrVolUp.
void MpxHfSpkrVolUp(void);


///@ingroup BT_HandFree
///@brief     HandFree Get MicGain.
///@remark HandFree Find GetMicGain.
unsigned char MpxHfGetMicGain(void);

///@ingroup BT_HandFree
///@brief     HandFree Get SpkrGain.
///@remark HandFree GEt SpkrGain .
unsigned char MpxHfGetSpkrGain(void);

///@ingroup BT_HandFree
///@brief     HandFree Answer.
///@remark HandFree Answer .
void MpxHfAnswer(void);

///@ingroup BT_HandFree
///@brief     HandFree NREC.
///@remark HandFree NREC .
void MpxHfNREC(void);

///@ingroup BT_HandFree
///@brief     HandFree VoiceREC.
///@remark HandFree VoiceREC .
void MpxHfVoiceREC(char id);

///@ingroup BT_HandFree
///@brief     HandFree VoiceTag.
///@remark HandFree VoiceTag .
void MpxHfVoiceTag(void);

///@ingroup BT_HandFree
///@brief     HandFree HaungUp.
///@remark HandFree HangUp.
void MpxHfHangUp(void);

///@ingroup BT_HandFree
///@brief     HandFree Release Held.
///@remark HandFree Release Held.
void MpxHfReleaseHeld(void);

///@ingroup BT_HandFree
///@brief     HandFree Release Active.
///@remark HandFree Release Active.
void MpxHfReleaseActive(void);

///@ingroup BT_HandFree
///@brief     HandFree Release.
///@remark HandFree Release.
void MpxHfRelease(char id);

///@ingroup BT_HandFree
///@brief     HandFree Hold Active.
///@remark HandFree Hold Active.
void MpxHfHoldActive(void);

///@ingroup BT_HandFree
///@brief     HandFree Private.
///@remark HandFree Private.
void MpxHfPrivate(char id);

///@ingroup BT_HandFree
///@brief     HandFree DialNum.
///@remark HandFree DialNum.
void MpxHfDialNum(unsigned char * pbuf);

///@ingroup BT_HandFree
///@brief     HandFree ReDial.
///@remark HandFree ReDial .
void MpxHfReDial(void);
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////
///@defgroup BT_SYS System API
///
/// Bluetooth SPP parameter initialize function
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///@ingroup BT_SYS
///@brief     Open the port of Spp client
///@retval   char : Return the status of operation.
///                             MPXBT_STATUS_SUCCESS means device opened successfully
///                             MPXBT_STATUS_FAILED means initilize fail
///@remark Client opens a serial device for reading and writing. 
///              Requires an existing ACL connection with the remote
///              device for client devices.
char MpxBtSppClientOpen(unsigned char index);


///@ingroup BT_SYS
///@brief     Close the port of Spp client
///@retval   char : Return the status of operation.
///                             MPXBT_STATUS_SUCCESS means device closed successfully
///                             MPXBT_STATUS_FAILED means device was not opened
///@remark Client close the serial device. 
///              Requires the device to have been opened 
///              previously by MpxBtSppClientOpen().
char MpxBtSppClientClose(void);


///@ingroup BT_SYS
///@brief     Read charactor from the port of Spp client
///@param   buf : allocated buffer to receive bytes
///         len : on input: 
///                 maximum bytes to read; on successful completion:
///                 number of bytes actually read
///@retval   DWORD : number of read bytes.
///@remark Client read from the serial device. Requires the device to have been 
///              successfully opened by MpxBtSppClientOpen().
unsigned long MpxBtSppClientRead(unsigned char * buf, unsigned long len);


///@ingroup BT_SYS
///@brief     Write charactor from the port of Spp client
///@param   buf : buffer containing characters to write .
///         len : on input: 
///                 pointer to number of bytes in buffer;
///                 bytes written are returned in nBytes if it returns success.
///@retval   DWORD : number of write bytes.
///@remark Client write to the serial device. Requires the device to have been 
///              successfully opened by MpxBtSppClientOpen().
unsigned long MpxBtSppClientWrite(unsigned char * buf, unsigned long len);


///@ingroup BT_SYS
///@brief     query data length of ring buffer
///@retval   DWROD : Return the length.
///@remark Client query the Rx data length in the ring buffer.
unsigned long MpxBtSppClientRxLen(void);


///@ingroup BT_SYS
///@brief     Client connect the ACL link
///@retval   char : Return the status of operation.
///                             MPXBT_STATUS_SUCCESS means connection linked successfully
///                             MPXBT_STATUS_FAILED means connection was fail.
///@remark Client connect an ACL link to remote device. Every RFcomm port
///        is based on this connection.
char MpxBtSppClientConnect(unsigned char index);


///@ingroup BT_SYS
///@brief     Client disconnect the ACL link
///@retval   char : Return the status of operation.
///                             MPXBT_STATUS_SUCCESS means disconnection successfully
///                             MPXBT_STATUS_FAILED means disconnection was fail.
///@remark close ACL.
char MpxBtSppClientDisconnect(void);


///@ingroup BT_SYS
///@brief     Open the port of Spp Server
///@retval   char : Return the status of operation.
///                             MPXBT_STATUS_SUCCESS means device opened successfully
///                             MPXBT_STATUS_FAILED means initilize fail
///@remark Server opens a serial device for reading and writing. 
///              Requires an existing ACL connection with the remote
///              device for Server devices.
char MpxBtSppServerOpen(void);


///@ingroup BT_SYS
///@brief     Read charactor from the port of Spp Server
///@param   buf : allocated buffer to receive bytes
///         len : on input: 
///                 maximum bytes to read; on successful completion:
///                 number of bytes actually read
///@retval   DWORD : number of read bytes.
///@remark Server read from the serial device. Requires the device to have been 
///              successfully opened by MpxBtSppServerOpen().
unsigned long MpxBtSppServerRead(unsigned char * buf, unsigned long len);


///@ingroup BT_SYS
///@brief     Write charactor from the port of Spp Server
///@param   buf : buffer containing characters to write .
///         len : on input: 
///                 pointer to number of bytes in buffer;
///                 bytes written are returned in nBytes if it returns success.
///@retval   DWORD : number of write bytes.
///@remark Server write to the serial device. Requires the device to have been 
///              successfully opened by MpxBtSppServerOpen().
long MpxBtSppServerWrite(unsigned char * buf, unsigned long len);


///@ingroup BT_SYS
///@brief     query data length of ring buffer
///@retval   DWROD : Return the length.
///@remark Server query the Rx data length in the ring buffer.
unsigned long MpxBtSppServerRxLen(void);


///@ingroup BT_SYS
///@brief     Close the port of Spp Server
///@retval   char : Return the status of operation.
///                             MPXBT_STATUS_SUCCESS means device closed successfully
///                             MPXBT_STATUS_FAILED means device was not opened
///@remark Server close the serial device. 
///              Requires the device to have been opened 
///              previously by MpxBtSppServerOpen().
char MpxBtSppServerClose(void);


///@ingroup BT_SYS
///@brief     SPP initialize
///@retval   BOOL : Return the result of operation.
///                             TRUE means initial successfully
///                             FALSE means initial fail
///@remark Initial the SPP data structure.
int SPP_Init(void);


///@ingroup BT_SYS
///@brief     SPP deinitialize
///@retval   BOOL : Return the result of operation.
///                             TRUE means deinitial successfully
///                             FALSE means deinitial fail
///@remark deinitial the SPP data structure.
char MpxBtSppDeinit(void);


char *InitializeMessageMsg(int Idx);
char *InquiryMessageMsg(int Idx);
char *PairingMessageMsg(int Idx);
char *ServiceMessageMsg(int Idx);
char *OPushMessageMsg(int Idx);
#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
char *FTPCMessageMsg(int Idx);
#endif
#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
char *FTPSMessageMsg(int Idx);
#endif
#if (BT_PROFILE_TYPE & BT_A2DP)
char *A2DPMessageMsg(int Idx);
#endif
#if (BT_PROFILE_TYPE & BT_SPP)
char *SppMessageMsg(int Idx);
#endif
#if (BT_PROFILE_TYPE & BT_HF)
char *HfMessageMsg(int Idx);
#endif

// OPUSH//////////////////////
//Not provide UI layer
void MpxGoepOpushDisConnect(void);


// A2DP//////////////////////
//Not provide UI layer
#if (BT_PROFILE_TYPE & BT_A2DP)
int MpxGetA2dpRecordingFlag(void);
#endif

#if (BT_PROFILE_TYPE & BT_HF)
int MpxGetScoRecordingFlag(void);
#endif


#endif //#if (BLUETOOTH == ENABLE)

#endif //__BTAPI_H


