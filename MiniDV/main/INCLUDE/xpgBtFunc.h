#ifndef __XPGBTFUNC_H__
#define __XPGBTFUNC_H__

#if (BT_XPG_UI == ENABLE)
#include "ui.h"
#include "BtApi.h"
#include "fs.h"


/* Bluetooth UUID values */
#define BT_UUID_SERVICE_DISCOVERY_SERVER              0x1000
#define BT_UUID_BROWSE_GROUP_DESCRIPTOR               0x1001
#define BT_UUID_PUBLIC_BROWSE_GROUP                   0x1002
#define BT_UUID_SERIAL_PORT                           0x1101
#define BT_UUID_LAN_ACCESS_PPP                        0x1102
#define BT_UUID_DIALUP_NETWORKING                     0x1103
#define BT_UUID_IR_MC_SYNC                            0x1104
#define BT_UUID_OBEX_OBJECT_PUSH                      0x1105
#define BT_UUID_OBEX_FILE_TRANSFER                    0x1106
#define BT_UUID_IR_MC_SYNC_COMMAND                    0x1107
#define BT_UUID_HEADSET                               0x1108
#define BT_UUID_CORDLESS_TELEPHONY                    0x1109
#define BT_UUID_AUDIO_SOURCE                          0x110a /* A2DP */
#define BT_UUID_AUDIO_SINK                            0x110b /* A2DP */
#define BT_UUID_AV_REMOTE_TARGET                      0x110c /* AVRCP */
#define BT_UUID_ADVANCED_AUDIO                        0x110d /* A2DP */
#define BT_UUID_AV_REMOTE                             0x110e /* AVRCP */
#define BT_UUID_VIDEO_CONFERENCING                    0x110f /* VCP */
#define BT_UUID_INTERCOM                              0x1110
#define BT_UUID_FAX                                   0x1111
#define BT_UUID_HEADSET_AUDIO_GATEWAY                 0x1112
#define BT_UUID_WAP                                   0x1113
#define BT_UUID_WAP_CLIENT                            0x1114
#define BT_UUID_PANU                                  0x1115 /* PAN */
#define BT_UUID_NAP                                   0x1116 /* PAN */
#define BT_UUID_GN                                    0x1117 /* PAN */
#define BT_UUID_DIRECT_PRINTING                       0x1118 /* BPP */
#define BT_UUID_REFERENCE_PRINTING                    0x1119 /* BPP */
#define BT_UUID_IMAGING                               0x111a /* BIP */
#define BT_UUID_IMAGING_RESPONDER                     0x111b /* BIP */
#define BT_UUID_IMAGING_AUTOMATIC_ARCHIVE             0x111c /* BIP */
#define BT_UUID_IMAGING_REFERENCED_OBJECTS            0x111d /* BIP */
#define BT_UUID_HANDSFREE                             0x111e
#define BT_UUID_HANDSFREE_AUDIO_GATEWAY               0x111f
#define BT_UUID_DIRECT_PRINTING_REF_OBJS              0x1120 /* BPP */
#define BT_UUID_DIRECT_PRINTING_REFERENCE_OBJECTS     0x1120 /* BPP */
#define BT_UUID_REFLECTED_UI                          0x1121 /* BPP */
#define BT_UUID_BASIC_PRINTING                        0x1122 /* BPP */
#define BT_UUID_PRINTING_STATUS                       0x1123 /* BPP */
#define BT_UUID_HUMAN_INTERFACE_DEVICE                0x1124 /* HID */
#define BT_UUID_HARDCOPY_CABLE_REPLACE                0x1125 /* HCRP */
#define BT_UUID_HCR_PRINT                             0x1126 /* HCRP */
#define BT_UUID_HCR_SCAN                              0x1127 /* HCRP */
#define BT_UUID_COMMON_ISDN_ACCESS                    0x1128 /* CIP */
#define BT_UUID_VIDEO_CONFERENCING_GW                 0x1129 /* VCP */
#define BT_UUID_UDI_MT                                0x112a /* UDI */
#define BT_UUID_UDI_TA                                0x112b /* UDI */
#define BT_UUID_AUDIO_VIDEO                           0x112c /* VCP */
#define BT_UUID_SIM_ACCESS                            0x112d /* SAP */
#define BT_UUID_PHONEBOOK_ACCESS_PCE                  0x112e /* PBAP */
#define BT_UUID_PHONEBOOK_ACCESS_PSE                  0x112f /* PBAP */
#define BT_UUID_PHONEBOOK_ACCESS                      0x1130 /* PBAP */
#define BT_UUID_PNP_INFORMATION                       0x1200
#define BT_UUID_GENERIC_NETWORKING                    0x1201
#define BT_UUID_GENERIC_FILE_TRANSFER                 0x1202
#define BT_UUID_GENERIC_AUDIO                         0x1203
#define BT_UUID_GENERIC_TELEPHONY                     0x1204
#define BT_UUID_UPNP_SERVICE                          0x1205 /* EBT */
#define BT_UUID_UPNP_IP_SERVICE                       0x1206 /* ESDP */
#define BT_UUID_ESDP_UPNP_IP_PAN                      0x1300 /* ESDP */
#define BT_UUID_ESDP_UPNP_IP_LAP                      0x1301 /* ESDP */
#define BT_UUID_ESDP_UPNP_L2CAP                       0x1302 /* ESDP */
#define BT_UUID_VIDEO_SOURCE                          0x1303 /* VDP */
#define BT_UUID_VIDEO_SINK                            0x1304 /* VDP */
#define BT_UUID_VIDEO_DISTRIBUTION                    0x1305 /* VDP */
#define BT_UUID_APPLE_AGENT                           0x2112


typedef enum
{
    BT_OP_STATUS_NONE = 0,
    BT_OP_STATUS_DONGLE_IN = BIT0,
    BT_OP_STATUS_INIT_DONE = BIT1,
    BT_OP_STATUS_INQUIRING = BIT2,
    BT_OP_STATUS_INQUIRY_DONE = BIT3,
    BT_OP_STATUS_PAIRING = BIT4,
    BT_OP_STATUS_CONNECTING = BIT5, /* indicate connecting/dis-connecting is in-progress */
    BT_OP_STATUS_OPP_MODE = BIT6,
    BT_OP_STATUS_OPP_PUSHING = BIT7,
    BT_OP_STATUS_OPP_PULLING = BIT8,
    BT_OP_STATUS_A2DP_MODE = BIT9,
    BT_OP_STATUS_EDITING = BIT10,
    BT_OP_STATUS_FTP_CLIENT_MODE = BIT11,
    BT_OP_STATUS_FTP_PUSHING = BIT12,
    BT_OP_STATUS_FTP_PULLING = BIT13,
    BT_OP_STATUS_FTP_FOLDER_LIST = BIT14,
    BT_OP_STATUS_FTP_SERVER_MODE = BIT15,
    BT_OP_STATUS_HF_MODE = BIT16,
    /* others ... */
} E_BT_OP_STATUS_TYPE;


enum
{
    BT_SEARCH_DEVICE_MODE = 0,
    BT_OPUSH_MODE = 1,
    BT_FTPC_MODE = 2,
    BT_FTPS_MODE = 3,
    BT_A2DP_MODE = 4,
};


typedef enum
{
    BT_MSG_NULL,
    BT_MSG_DEV_TRUSTED,
    BT_MSG_DEV_LINKKEY,
    BT_MSG_DEV_NO_SERVICE_FOUND,
    BT_MSG_DEV_KEY_EXISTED,
    BT_MSG_DEV_REFRESH_AGAIN,
    BT_MSG_DEV_NO_DEVICE,
    BT_MSG_DEV_EMPTY_DEVICE,
    BT_MSG_PLEASE_WAIT,
    BT_MSG_FILE_OPEN_ERROR,
    BT_MSG_DEV_CONNECTED,
    BT_MSG_DEV_NO_DONGLE,
    BT_MSG_DEV_ONLY_PHOTO_ALLOW,
    BT_MSG_DEV_SEARCH_OPUSH_HINT,
    BT_MSG_DEV_SEARCH_FTPC_HINT,
    BT_MSG_DEV_SEARCH_FTPS_HINT,
    BT_MSG_DEV_SEARCH_A2DP_HINT,
    BT_MSG_DEV_OPUSH_HINT,
    BT_MSG_DEV_FTPC_HINT,
    BT_MSG_DEV_FTPS_HINT,
    BT_MSG_DEV_CREATE_FOLDER_NAME,
    BT_MSG_OPUSH_RX_DIALOGUE,
    BT_MSG_BT_NOT_ON,
    BT_MSG_BT_DEV_SERVICE,
    BT_MSG_BT_DEV_NAME,
    BT_MSG_ALLOW_CONNECT_CONFIRM,
    BT_MSG_COPY_FILE,
} E_BT_MSG_TYPE;


enum
{
    BT_DIALOG_YES_NO,
    BT_DIALOG_OK,
    BT_DIALOG_CANCEL,
    BT_DIALOG_OK_MULT_STRING,
    BT_DIALOG_OSD_INFO,
};


#define MAX_DEVICE_NAME_SIZE    34
#define SUPOT_MAX_BT_DEV        20

#define BT_ON                    1
#define BT_OFF                   0


typedef struct
{
    WORD name[MAX_L_NAME_LENG];
    WORD service;
    WORD connect;
} ST_BT_CONNECTED_DEV_INFO;


typedef struct
{
    BYTE *deviceName; /* device name pointer */

    WORD name[MAX_L_NAME_LENG];
    WORD connected;
    WORD paired;
    WORD supportService;
    WORD state;

    DWORD MajorClass;
    DWORD service[BT_SERVICE_STORE_COUNT];
} ST_BT_DEVICE;



void BtUiParameterInit(void);
int BT_InitilizingHandler(XpgMessage * Event);
int BT_InquiryHandler(InquiryMessage * Event);
int BT_PairingHandler(InquiryMessage * Event);
int BT_OpushHandler(OpushMessage * Event);
int BT_FtpClientHandler(XpgMessage * Event);
void BTTransferFile(void);
void BTProcessBar(DWORD percent);
void xpgBtProcessDongleOut(void);
void BtSetupExit(void);

void BtUpStageSetCallBack(void (*callbackfun)(void));
void xpgBtStartUpdatStage(void);

void BtDialogBoxCancel(BYTE msg, BYTE prepage);
void BtDialogBoxYesNo(BYTE msg);
BYTE BtDialogBoxGetPrePage(void);
void BtDialogBoxSetPrePage(BYTE prepage);
void BtDialogBoxOk(BYTE msg, BYTE prepage);
void SetBtDlgbYesKey(BYTE flag);
void SetBtDlgbNoKey(BYTE flag);
void BtDialogBoxOSDInfo(void);

void xpgBtDeviceRename(void);

void xpgBtStart(void);
void xpgBtMyDevice(void);
void xpgBtPasscode(void);

void xpgCb_BtStartKpNext(void);
void xpgCb_BtStartKpPrev(void);
void xpgCb_BtStartKpEnter(void);
void xpgCb_BtStartKpExit(void);
void xpgCb_BtMyDeviceExit(void);
void xpgCb_BtMyDeviceKpUp(void);
void xpgCb_BtMyDeviceKpDown(void);
void xpgCb_BtDiagboxSelect(void);
void xpgCb_BtDiagboxEnter(void);
void xpgCb_BtMyDeviceSetupMenu(void);
void xpgCb_BtMyDeviceKpLeft(void);
void xpgCb_BtMyDeviceKpRight(void);
void xpgCb_BtMyDeviceKpEnter(void);

void xpgCb_BtMyDevice_OPP_Connect(void);
void xpgCb_BtMyDevice_FTP_Connect(void);
void xpgCb_BtMyDevice_A2DP_Connect(void);
void xpgCb_BtMyDevice_PBAP_Connect(void);
void xpgCb_BtMyDevice_HF_Connect(void);


void BtMyDeviceUpdateStage(void);
void IncTolSrchedDevCnt(void);
void DecTolSrchedDevCnt(void);
void ClrTolSrchedDevCnt(void);
BYTE GetTolSrchedDevCnt(void);
void ClrCurListNo(void);
BYTE GetCurListNo(void);
void SetBtStopXPGActionFlag(void);
void ClrBtStopXPGActionFlag(void);
BOOL GetBtStopXPGActionFlag(void);

void BtUiMessageReceiver(void);

BYTE GetTolPairDevCnt(void);

void BtDeviceConnectRequest(void);
void BtParsingService(void);

void BtGoToPrePage(BYTE page);

#if (BT_PROFILE_TYPE & BT_FTP_CLIENT)
void BtFtpCFileNameParsing(DWORD total_content_len);
void BtFtpCFreeListBuffer(void);
void BtFtpC_ProcessRemoteFileListXmlBuffer(BYTE * srcBuf, DWORD srcLen);
void BtFtpClientListInit(void);

void BtFtpClientUpdatStage(void);
void xpgCb_BtFtpClientKpDown(void);
void xpgCb_BtFtpClientKpUp(void);
void xpgCb_BtFtpClientKpEnter(void);
void xpgCb_BtFtpClientKpSelect(void);
void xpgCb_BtFtpClientKpSetup(void);
void xpgCb_BtFtpClientExit(void);

void nonXpgBtFtpClientSetupMenu(void);
void BtFtpClntUpStgeAfterPulling(void);

void BtFtpPullFiles(void);
void BtFtpClientSelAllFiles(void);
void BtFtpClientResetFiles(void);
#endif //(BT_PROFILE_TYPE & BT_FTP_CLIENT)

#if (BT_PROFILE_TYPE & BT_HF)
void xpgCb_BtHfPageExit(void);
void xpgCb_BtHfPageEnterPBAP(void);
void xpgCb_BtPhoneBookExit(void);
#endif


/* Sprite List index values of used icons for BT GUI in XPG page 0 ------ [begin] */
#define XPG_PAGE0_SPRITE_FILE_TYPE_FOLDER_CLOSE            34
#define XPG_PAGE0_SPRITE_FILE_TYPE_FOLDER_OPEN             35

#define XPG_PAGE0_SPRITE_LIGHT_BAR                         38
#define XPG_PAGE0_SPRITE_CLASS_COMPUTER                    39
#define XPG_PAGE0_SPRITE_CLASS_PHONE                       40
#define XPG_PAGE0_SPRITE_CLASS_LAN_ACCESS_POINT            41
#define XPG_PAGE0_SPRITE_CLASS_AUDIO                       42
#define XPG_PAGE0_SPRITE_CLASS_PERIPHERAL                  43
#define XPG_PAGE0_SPRITE_CLASS_UNCLASSIFIED                44
#define XPG_PAGE0_SPRITE_CONNECTED                         45
#define XPG_PAGE0_SPRITE_UNCONNECTED                       46
#define XPG_PAGE0_SPRITE_PAIRED                            47
#define XPG_PAGE0_SPRITE_UNPAIRED                          48
#define XPG_PAGE0_SPRITE_SEL_CONNECTED                     49
#define XPG_PAGE0_SPRITE_SEL_UNCONNECTED                   50
#define XPG_PAGE0_SPRITE_SEL_PAIRED                        51
#define XPG_PAGE0_SPRITE_SEL_UNPAIRED                      52
#define XPG_PAGE0_SPRITE_SEL_CLASS_COMPUTER                53
#define XPG_PAGE0_SPRITE_SEL_CLASS_PHONE                   54
#define XPG_PAGE0_SPRITE_SEL_SEL_CLASS_LAN_ACCESS_POINT    55
#define XPG_PAGE0_SPRITE_SEL_SEL_CLASS_AUDIO               56
#define XPG_PAGE0_SPRITE_SEL_CLASS_PERIPHERAL              57
#define XPG_PAGE0_SPRITE_SEL_CLASS_UNCLASSIFIED            58
#define XPG_PAGE0_SPRITE_FILE_TYPE_AUDIO                   59
#define XPG_PAGE0_SPRITE_FILE_TYPE_VIDEO                   60
#define XPG_PAGE0_SPRITE_FILE_TYPE_IMAGE                   61
#define XPG_PAGE0_SPRITE_FILE_TYPE_UNKNOWN                 62
#define XPG_PAGE0_SPRITE_SERVICE                           63
#define XPG_PAGE0_SPRITE_SEL_SERVICE                       64
/* Sprite List index values of used icons for BT GUI in XPG page 0 ------ [end] */


/* Sprite List index values of used icons for BT GUI in XPG [BtFtpClient] page */
#define XPG_BTFTP_PAGE_SPRITE_CHECK_BOX                    0


/* Sprite List index values of used icons for BT GUI in XPG [BtDialog] page ------ [begin] */
#define XPG_BTDIALOG_PAGE_SPRITE_YES                       3
#define XPG_BTDIALOG_PAGE_SPRITE_SEL_YES                   4
#define XPG_BTDIALOG_PAGE_SPRITE_NO                        5
#define XPG_BTDIALOG_PAGE_SPRITE_SEL_NO                    6
#define XPG_BTDIALOG_PAGE_SPRITE_SEL_CANCEL                7
#define XPG_BTDIALOG_PAGE_SPRITE_SEL_SEL_CANCEL            8
#define XPG_BTDIALOG_PAGE_SPRITE_SEL_OK                    9
#define XPG_BTDIALOG_PAGE_SPRITE_SEL_SEL_OK                10
/* Sprite List index values of used icons for BT GUI in XPG [BtDialog] page ------ [end] */


/* Sprite List 'Tindex' values of used icons for BT GUI in XPG [BtEditor] page ------ [begin] */
/* TINDEX is sprite type index in xpw */
#define XPG_BTEDITOR_PAGE_TINDEX_KP_BACK                13
#define XPG_BTEDITOR_PAGE_TINDEX_KP_ENTER               27
#define XPG_BTEDITOR_PAGE_TINDEX_KP_CAPS                28
#define XPG_BTEDITOR_PAGE_TINDEX_KP_EXIT                41
#define XPG_BTEDITOR_PAGE_TINDEX_KP_SPACE               42
#define XPG_BTEDITOR_PAGE_TINDEX_KP_ARROW_UP            40
#define XPG_BTEDITOR_PAGE_TINDEX_KP_ARROW_LEFT          53
#define XPG_BTEDITOR_PAGE_TINDEX_KP_ARROW_DOWN          54
#define XPG_BTEDITOR_PAGE_TINDEX_KP_ARROW_RIGHT         55
/* Sprite List 'Tindex' values of used icons for BT GUI in XPG [BtEditor] page ------ [end] */


#endif  //#if(BT_XPG_UI == ENABLE)
#endif  //#ifndef __XPGBTFUNC_H__


