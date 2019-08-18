#ifndef __BTSETTING_H
#define __BTSETTING_H

#include "corelib.h"

/* note: for making libBlueTooth.a  [begin] */
#define P_A2DP     1
#define P_FTP      1
#define P_SPP      1
#define P_HF       1
/* note: always set e_T to 1 for multi-tasking */
#define e_T        1 // multi task switch
/* note: for making libBlueTooth.a  [end] */

#define M_SERVER    1 // rick 0703
#define M_CLIENT    1 // rick 0702
#define M_WRITE     1
#define M_PROFILE   0
#define BT_CS320   0
#define BT_CS322   1
#define BTCHIP     BT_CS322

#if (e_T == 1) // XA_MULTITASKING of config.h
    #define BT_MULTI_TASK   1
#else
    #define BT_MULTI_TASK   0
#endif

#define ENABLE              1
#define DISABLE             0

#define BLUETOOTH           MAKE_BLUETOOTH

#define NONE                0
#define BT_USB              1
#define BT_UART             2

#define MPX_XA_DEBUG        ENABLE // XA_DEBUG of override.h
#define DIALOG_BOX          ENABLE

#ifndef BT_DRIVER_TYPE
    #define BT_DRIVER_TYPE    NONE
#endif
#ifndef BT_PROFILE_TYPE
    #define BT_PROFILE_TYPE   NONE
#endif

#define BT_PUSH             0x01
#define BT_FTP_SERVER       0x02
#define BT_FTP_CLIENT       0x04
#define BT_A2DP             0x08
#define BT_SPP              0x10
#define BT_HF               0x20
#define BT_HS               0x40


#define FIXED_WRITE_BUFFER     0
#define AVRCP_AUTO_RELEASE     0


#if (BLUETOOTH == ENABLE)
    #ifdef BT_DRIVER_TYPE
        #undef BT_DRIVER_TYPE
    #endif

    #define BT_DRIVER_TYPE      BT_USB

    #ifdef BT_PROFILE_TYPE
        #undef BT_PROFILE_TYPE
    #endif

    #define BT_DEBUG_ENABLE         1
    #define BT_OPUSH_AUTO_ENABLE    1

//    #define BT_PROFILE_TYPE     (BT_PUSH)
    #define BT_PROFILE_TYPE     (BT_PUSH|BT_FTP_CLIENT|BT_FTP_SERVER|BT_SPP|BT_A2DP|BT_HF)

    #if (BT_DRIVER_TYPE == BT_USB)
        #define AUTORUN_BLUETOOTH   DISABLE
    #else
        #define AUTORUN_BLUETOOTH   DISABLE
    #endif


    #if ((MAKE_BLUETOOTH == 1) && (Make_TESTCONSOLE == 0))
        #define BT_XPG_UI           ENABLE
    #else
        #define BT_XPG_UI           DISABLE         // Don't change
    #endif

    #ifdef DIALOG_BOX
        #undef DIALOG_BOX
    #endif
    #define DIALOG_BOX          ENABLE
#endif //BLUETOOTH

#endif //__BTSETTING_H


