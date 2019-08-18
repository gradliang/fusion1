#ifndef TASK_ID_H
#define TASK_ID_H

#include "iplaysysconfig.h"
#include "..\libsrc\os\include\osconfig.h"

/*
// Constant declarations
*/
/*! define task ID
@note
task 0 must not use, GHOST_TAST is OS dummy task
 MAIN_TASK is for control and DRIVER_TASK is for all driver
*/
enum {
    NULL_TASK = 0,              ///< task id=0
    GHOST_TASK,                 ///< task id=1
    MAIN_TASK,                  ///< task id=2
    MCARD_TASK,                 ///< task id=3
    VIDEO_DECODE_TASK,          ///< task id=4
    VIDEO_DATA_TASK,            ///< task id=5
    AUDIO_DECODE_TASK,          ///< task id=6
    AUDIO_DATA_TASK,            ///< task id=7
    AV_CONTROL_TASK,            ///< task id=8
    STREAM_CACHE_TASK,          ///< task id=9
    MJPEG_DATAIO_TASK,          ///< task id=10
    BT_OS_TASK,                 ///< task id=11, for bluetooth

    #if SC_USBDEVICE
    USBOTG0_DEVICE_TASK,
    USBOTG1_DEVICE_TASK,
    #endif

    #if SC_USBHOST
    USBOTG0_HOST_DRIVER_TASK,
    USBOTG0_HOST_CLASS_TASK,
    USBOTG1_HOST_DRIVER_TASK,
    USBOTG1_HOST_CLASS_TASK,
    #endif

#if ( BT_DRIVER_TYPE == BT_UART )
    BTUART_DRIVER_TASK,         ///< task id=16, for bluetooth Uart driver
    BT_UI_TASK,                 ///< task id=17
#else
    USB_BT_TASK,                ///< task id=18
    BT_UI_TASK,                 ///< task id=19
#endif

//#if (USBOTG_WEB_CAM || USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG)
#if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG)
    WEB_CAM_VIDEO_TASK,         ///< task id=18
    WEB_CAM_AUDIO_TASK,         ///< task id=19
#endif

#if (USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG)
    USBOTG_DEVICE_EXTERN_TASK,
#endif

    NETWARE_TASK,               ///< task id=18 //20
    WPA_MAIN_TASK_ID,           ///< task id=19
    RALINK_HISR_TASK,           ///< task id=20
    WIFI_USB_BULK_COMPLETE_TASK,///< task id=21
    WIFI_USB_MLME,              ///< task id=22
    WIFI_USB_CMD,               ///< task id=23
    HISR_TASK,                  ///< task id=24
    TCONS_TASK,                 ///< task id=25
    SENSOR_TASK,                ///< task id=26
    NETWORK_STREAM_TASK,        ///< task id=27
    UPNP_MINI_SERVER_TASK,      ///< task id=28 //30
    UPNP_CTRL_TASK,             ///< task id=29

#if EREADER_ENABLE
    TYPESETTING_TASK,  ///< task id=30
    TYPESETTING_TASK2,  ///< task id=31
#endif

    BT_A2DP_TASK ,
    BT_SCO_TASK ,
    UPNP_DOWNLOAD_TASK,

#if TOUCH_CONTROLLER_ENABLE
    TOUCH_CONTROLLRT_TASK,
#endif

#if DEMO_PID
    PID_AD_PHOTO_DEC_TASK,
    PID_AD_PHOTO_DISPLAY_TASK,
    PID_AD_MESSAGE_DECODE_TASK,
    PID_AD_MESSAGE_DISPLAY_TASK,
#endif
#if RECORD_AUDIO
	RECORD_AUDIO_TASK,
#endif
#if WEB_CAM_DEVICE
    UVC_TASK,
    UAC_TASK,
#endif

#if RECORD_ENABLE  //avi record
    REC_OP_CONTROL_TASK,
    AVI_AUDIO_TASK,
    WEB_REC_TASK,
    RECORD_CNT_TASK,
    RECORD_PANEL_TASK,
#endif

#if USBOTG_HOST_ISOC
    USBOTG_HOST_ISOC_TASK, //40-41
#endif
#if (DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE)
	ETHERNET_RECV_TASK,
	ETHERNET_ARPRECV_TASK,
#endif
    EXCEPTION_TASK,
    
    SYSTEM_CHECK_TASK, // for Alarm Check
    
    TOTAL_TASK_NUMBER           // Keep this at last and should be small than OS_MAX_NUMBER_OF_TASK
};

// define task priority
// Note: priority 0 must not use, and priority 1 is for OS ghost task, priority (OS_MAX_PRIORITY - 1) is for OS exception task
#define HIGHEST_TASK_PRIORITY       (OS_MAX_PRIORITY - 2)           // Max priority of SW task

#define GHOST_PRIORITY              1
#define CONTROL_PRIORITY            4       // 4 ~ 7
#define DRIVER_PRIORITY             8       // 8 ~ 11
#define ISR_PRIORITY                12      // 12 ~ HIGHEST_TASK_PRIORITY

#define WIFI_PRIORITY               (DRIVER_PRIORITY + 1)
#define NETWORK_STREAM_PRIORITY     CONTROL_PRIORITY



// define object ID
/*!
@note
All objects use the same object array
*/
enum {
    NULL_OBJECT = 0,                        ///< 0
    OS_MEM_SEMAPHORE,                       ///< 1
    UI_EVENT,                               ///< 2
    MJPEG_DATAIO_EVENT,                     ///< 3
    MJPEG_READ_EVENT,                       ///< 4
    MOVIE_EVENT,                            ///< 5
    VIDEO_SEMA_ID,                          ///< 6
    AV_CMD_SEMA_ID,                         ///< 7
    FILE_READ_SEMA_ID,                      ///< 8  /* temp solution for protecting concurrent multiple file read operations */
    JPEG_DECODE_SEMA_ID,                    ///< 9
    SLIDE_SEMA_ID,                          ///< 10
    BThw_SEMA_ID,                           ///< 11
    BTir_SEMA_ID,                           ///< 12

    //USB_DEVICE_MESSAGE_ID,
    USBOTG0_DEVICE_MESSAGE_ID,              ///< 13
    USBOTG1_DEVICE_MESSAGE_ID,              ///< 14

    //USBOTG_HOST_CLASS_MAIL_ID,
    USBOTG0_HOST_CLASS_MAIL_ID,             ///< 15
    USBOTG1_HOST_CLASS_MAIL_ID,             ///< 16
    //USBOTG_HOST_DRIVER_EVENT,
    USBOTG0_HOST_DRIVER_EVENT,              ///< 17
    USBOTG1_HOST_DRIVER_EVENT,              ///< 18
    //USBOTG_HOST_MSDC_TRANSACTION_DONE_EVENT,
    USBOTG0_HOST_MSDC_TRANSACTION_DONE_EVENT,///< 19
    USBOTG1_HOST_MSDC_TRANSACTION_DONE_EVENT,///< 20

    TCONS_EVENT,                            ///< 21
    IDU_SEMA_ID,                            ///< 22
    IDU_EVENT_ID,                           ///< 23

    USB_BT_EVENT,                           ///< 24
    BT_UI_MSG_ID,                           ///< 25

    NETWARE_EVENT,                          ///< 26
    WIFI_EVENT,                             ///< 27
    NET_SEM_ID,                             ///< 28, used by LWIP
    FTP_SERVER_EVENT,                       ///< 29
    WPA_EVENT,                              ///< 30
    NET_CACHE_SEMA_ID,                      ///< 31
    USB_CONTROL_SEMA,                       ///< 32
    USB_BULK_COMPLETE_EVENT,                ///< 33

    MMCP_SEMA_ID,                           ///< 34
    SYSTEM_EVENT_ID,                        ///< 35
    JPEG_LOAD_DATA_EVENT_ID1,               ///< 36
    MPV_EVENT_ID,                           ///< 37
    PP_EVENT_ID,                            ///< 38
    MCARD_MAIL_ID,                          ///< 39
    MCARD_EVENT_ID,                         ///< 40
    MCARD_SEMAPHORE_ID,                     ///< 41
    AV_DECODER_EVENT,                       ///< 42
    SENSOR_IPW_FRAME_END_EVENT_ID,          ///< 43  /*BIT0 for record, BIT1  for display, BIT2 for caputre flow, BIT3 for PCCam*/
    SCALING_FINISH_EVENT_ID,                ///< 44
    I2CM_SEMA_ID,                           ///< 45
    FILE_WRITE_SEMA_ID,                     ///< 46  /* temp solution for protecting concurrent multiple file write operations */
    NETWORK_STREAM_EVENT,                   ///< 47
    UPNP_START_EVENT,                       ///< 48

    EXCEPTION_MSG_ID,
    STREAM_READ_SEMA_ID,                    ///< 50
    STTB_SEMA_ID,                           ///< 51

#if EREADER_ENABLE
    TYPESETTING_EVENT,                      ///< 52
    TYPESETTING_EVENT2,                     ///< 53
    TYPESETTING_SEMA_ID,
#endif

    BTstack_SEMA_ID,
    BTwaveIn_SEMA_ID,
    BTwaveOut_SEMA_ID,
    BTpacket_SEMA_ID,

    BT_A2DP_EVENT,
    BT_SCO_EVENT,
    BT_OS_EVENT,
    BT_UART_EVENT,

    IDU_WAIT_BE_SEMA_ID,

	AVP_SYNC_EVENT,
#if TOUCH_CONTROLLER_ENABLE
    TOUCH_CONTROLLER_MSG_ID,
    UI_TC_MSG_ID,
    UI_TC_CAL_MSG_ID,
#endif

#if DEMO_PID
    PID_AD_MSG_DISPLAY_EVENT_ID,
    PID_PHOTO_MSG_ID,
    PID_TTF_MSG_ID,
#endif

#if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
    USBOTG_DEVICE_EXTERN_MAIL_ID,
#endif

#if WEB_CAM_DEVICE
	UVC_MAIL_ID,
	UAC_MAIL_ID,
#endif

    //////// Semaphore IDs for each drive's FAT cache buffer [begin] ////////
    /* note: make sure count of reserved semaphore IDs here must >= MAX_DRIVE (max drive index ID defined in drive.h) */
    DRIVE_FAT_CACHE_SEMA_ID_BASE,
    DRIVE_FAT_CACHE_SEMA_ID_MAX = (DRIVE_FAT_CACHE_SEMA_ID_BASE + 36),
    //////// Semaphore IDs for each drive's FAT cache buffer [end] ////////


	STREAM_CACHE_EVENT,
//#if RECORD_ENABLE
    REC_ERR_EVENT,
    REC_OP_MSG_ID,
    REC_SENDMSG_EVENT,
    AUDIO_ID,
    RECORD_STREAM_CACHE_SEMA_ID,
    RECORD_opcode_SEMA_ID,
    AUDIO_STREAM_MSG_ID,
//#endif
#if USBOTG_HOST_ISOC
    USBOTG_HOST_ISOC_EVENT,
#endif

#if SOFT_RECORD
	SOFT_RECORD_AUDIO,
#endif

    UI_KEY_MSG_ID,
#if (DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE)
	ETHERNET_RECV_EVENT,
	ETHERNET_ARPRECV_EVENT,
	CFETHERNET_MCARD_SEMA,
#endif
#if AMR_RECORD
	AMR_RECORD_AUDIO,
#endif

	VIDEO_DATA_EVENT,
	MOVIE_STATUS,

    JPEG_CDU_SEMA_ID,

#if USBOTG_HOST_ISOC // Ring Buffer
    RTP_Queue_SEMA_ID,
    DECODE_QUEUE_SEMA_ID,
    ENCODE_QUEUE_SEMA_ID,
    VIDEO_QUEUE_SEMA_ID,
#endif

#if Make_USB == REALTEK_RTL8188CU
	RTL8188_CMD_QUEUE_SEMA_ID,
	RTL8188_TERMINATE_CMDTHREAD_SEMA_ID,
#endif

    TOTAL_OBJECT_NUMBER                     // Keep this at last and should be small than OS_MAX_NUMBER_OF_OBJECT
};

// define main function ID
#define MNULL_FUNCTION      0
#define MSEL_DRIVE          1
#define MSEL_PLAYER         2
#define MIMG_PLAYER         3
#define MMOVIE_PLAYER       4
#define MAUDIO_PLAYER       5
#define MFILE_BROWSER       6
#define MUTILITY            7
#define MUSB_MASS_STORAGE   8
#define MPWR_OFF            9

// define file browser ID
#define BROWSER_PHOTO       0
#define BROWSER_MUSIC       1
#define BROWSER_VIDEO       2
#define BROWSER_FILE        3

// define main media ID
#define MEDIA_PHOTO         0
#define MEDIA_MUSIC         1
#define MEDIA_VIDEO         2


// define main flow mode status ID
#define MODE_NULL           0
#define MODE_MENULIST       1
#define MODE_PLAYER         2
#define MODE_SLIDESHOW      3
#define MODE_PRINTER        4


// define resource tag
#define RES_NULL            0x4e554c4c  //NULL
#define RES_BAKGND_IMG0     0x4a424730  //JBG0, for image browser mode
#define RES_BAKGND_IMG1     0x4a424731  //JBG1, for image static play mode
#define RES_BAKGND_IMG2     0x4a424732  //JBG2
#define RES_IMG_PLAYER      0x49494d50  //IIMP
#define RES_MOVIE_PLAYER    0x494d4f50  //IMOP
#define RES_AUDIO_PLAYER    0x49414450  //IADP
#define RES_FILE_BROWSER    0x49464252  //IFBR
#define RES_UTILITY         0x49555449  //IUTI

#define RES_HD0             0x49484430  //IHD0
#define RES_CF0             0x49434630  //ICF0
#define RES_SDMMC0          0x49534430  //ISD0
#define RES_MS0             0x494D5330  //IMS0
#define RES_SM0             0x49534D30  //ISM0
#define RES_NAND0           0x494E4130  //INA0
#define RES_XD0             0x49584430  //IXD0
#define RES_USBH0           0x49554830  //IUH0

#define RES_FONT            0x464F4E54  //FONT


/*
// Structure declarations
*/

typedef struct {
    DWORD dwArgument1;
    DWORD dwArgument2;
    DWORD dwArgument3;
    DWORD dwArgument4;
    DWORD dwArgument5;
    DWORD dwArgument6;
    DWORD dwArgument7;
    DWORD dwArgument8;
} ST_MESSAGE_CONTENT;

#endif

