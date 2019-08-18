
#ifndef __UI_H
#define __UI_H

#include "utiltypedef.h"

/*
// Constant declarations
*/
#define EVENT_CARD_FATAL_ERROR  BIT0
#define EVENT_CARD_IN           BIT1
#define EVENT_CARD_OUT          BIT2
#define EVENT_ERROR             BIT3
#define EVENT_LOW_BATTERY       BIT4
//#define EVENT_ON_CHARGING       BIT5
#define EVENT_IR                BIT6
#define EVENT_GPIO_KEY          BIT7
#define EVENT_LONG_PRESS_KEY    BIT8 // for what use ? //EVENT_MPXTOOL_RELOAD
#define EVENT_TIMER             BIT9 // for what use ?
#define EVENT_USB0_CHG          BIT10
#define EVENT_USB1_CHG          BIT11
#define EVENT_PRINT_INIT        BIT12
#define EVENT_PRINT_PRINTING    BIT13
#define EVENT_PRINT_FINISH      BIT14
#define EVENT_PRINT_ERROR       BIT15
#define EVENT_AUDIO_END         BIT16
#define EVENT_VIDEO_END         BIT17
#define EVENT_YOUTUBE_END       BIT18 // If without support YouTubem then disable
#define EVENT_MOTION_DETECT_OK       BIT18 // If without motion then disable

#define EVENT_DISK_FULL         BIT19
#define EVENT_DISK_FATAL_ERR    BIT20
#define EVENT_AUTO_DEMO         BIT21

#if USBOTG_HOST_HID
#define EVENT_HID_KEY           BIT22
#define EVENT_HID_MOUSE         BIT23   // Attention! Conflict with BLUETOOTH
#elif TSPI_ENBALE
#define EVENT_TSPI_START       				BIT22
#endif
#if (PRODUCT_UI==UI_WELDING)
#define EVENT_PROC_DATA       					BIT23
#define EVENT_DISP_DATA       					BIT24
#endif

#if (BLUETOOTH == ENABLE )
#if (BT_DRIVER_TYPE == BT_USB)
#define EVENT_BT_DONGLE_IN      BIT23
#define EVENT_BT_DONGLE_OUT     BIT24
#endif
#define EVENT_BLUETOOTH         BIT25
#endif

#if (USBOTG_WEB_CAM || USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
#define EVENT_WEB_CAM_IN        BIT26
#define EVENT_WEB_CAM_OUT       BIT27
#endif

#define EVENT_SERVER_IN         BIT29

#if (TOUCH_CONTROLLER_ENABLE == ENABLE)
#define EVENT_TOUCH_COLTROLLER  BIT30
#else
#define EVENT_TIMER_SECOND        BIT30
#endif

//#define EVENT_TIMER_SECOND      BIT31

#define EVENT_IR_KEY_DOWN       EVENT_IR
#define EVENT_KEYS              (EVENT_IR | EVENT_IR_KEY_DOWN)

#ifndef BT_XPG_UI
#define BT_XPG_UI               DISABLE
#endif

/*
// Structure declarations
*/



/*
// Function prototype
*/

#endif  //__UI_H

