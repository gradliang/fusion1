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
* Filename      : IR.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "peripheral.h"
#include "taskid.h"
#include "ui.h"

#if (IR_ENABLE == 1)

/*
// Structure declarations
*/


/*
// Constant declarations
*/
#define REPEAT_CODE_AS_KEY_DOWN

////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////
#if (REMOTE_CONTRLLER == REMOTE_MPX_STANDARD_28KEYS)
//user-defined IR event
#define IR_UP_CODE              0x61d622dd
#define IR_DOWN_CODE            0x61d612ed
#define IR_LEFT_CODE            0x61d632cd
#define IR_RIGHT_CODE           0x61d602fd
#define IR_MENU_CODE            0x61d6c03f
#define IR_SOURCE_CODE          0x61d6e01f
#define IR_EXIT_CODE            0x61d6f00f
#define IR_PP_CODE              0x61d6b847
#define IR_BACKWARD_CODE        0x61d638c7
#define IR_FORWARD_CODE         0x61d67887
#define IR_STOP_CODE            0x61d6f807
#define IR_VOLUP_CODE           0x61d640bf
#define IR_VOLDOWN_CODE         0x61d6708f
#define IR_ENTER_CODE           0x61d6609f
#define IR_SIZE_CODE            0x61d63ac5
#define IR_POSITION_CODE        0x61d61ae5
#define IR_POP_CODE             0x61d62ad5
#define IR_SWAP_CODE            0x61d60af5
#define IR_INPUT_CODE           0x61d68a75
#define IR_BLENDING_CODE        0x61d6ba45
#define IR_FTB_CODE             0x61d6da25
#define IR_TELETEXT_CODE        0x61d6ea15
#define IR_VERSION_CODE         0x61d610ef
#define IR_BRIGHTUP_CODE        0x61d69a65
#define IR_BRIGHTDOWN_CODE      0x61d6ba45
#define IR_SETUP_CODE           0x61d6926d
#define IR_CANAEL_CODE          0x61d67a85
#define IR_SAVE_CODE            0x61d6b24d
#define IR_NULL_CODE            0x0

#define IR_POWER_CODE           0x61d600FF
#define IR_MUTE_CODE            0x61D630CF
#define IR_PREVIOUS_CODE        0x2
#define IR_NEXT_CODE            0x3
#define IR_PAL_NTSC_CODE        0x4
#define IR_ROTATE_CODE          0x5
#define IR_ZOOM_CODE            0x6
#define IR_PIC_MP3_CODE         0x7
//#define IR_SETUP_CODE           0x8
#define IR_INFO_CODE            0x9
//#define IR_POWER_CODE           0xa
#define IR_REPEAT_CODE          0xb
#define IR_BRIGHTNESS_CODE      0xc
#define IR_PLAY_PAUSE_CODE      0xd

static const ST_IR_KEY sIRMapTable[] = {
    // IR Code          Key Code            Enable Repeat Code
    {IR_UP_CODE,        KEY_UP,             ENABLE},
    {IR_DOWN_CODE,      KEY_DOWN,           ENABLE},
    {IR_LEFT_CODE,      KEY_LEFT,           ENABLE},
    {IR_RIGHT_CODE,     KEY_RIGHT,          ENABLE},
    {IR_ENTER_CODE,     KEY_ENTER,          DISABLE},
    {IR_MENU_CODE,      KEY_MENU,           DISABLE},
    {IR_SOURCE_CODE,    KEY_SOURCE,         DISABLE},
    {IR_EXIT_CODE,      KEY_EXIT,           DISABLE},
    {IR_PP_CODE,        KEY_PP,             DISABLE},
    {IR_BACKWARD_CODE,  KEY_BACKWARD,       DISABLE},
    {IR_FORWARD_CODE,   KEY_FORWARD,        DISABLE},
    {IR_STOP_CODE,      KEY_STOP,           DISABLE},
    {IR_POSITION_CODE,  KEY_ROTATE,         DISABLE},
    {IR_POP_CODE,       KEY_EFFECTTOGGLE,   DISABLE},
    {IR_SWAP_CODE,      KEY_VIDEOSWAP,      DISABLE},
    {IR_FTB_CODE,       KEY_IRPRINT,        DISABLE},
    {IR_TELETEXT_CODE,  KEY_PIC_MP3,        DISABLE},
    {IR_VERSION_CODE,   KEY_VERSION,        DISABLE},
    {IR_BRIGHTUP_CODE,  KEY_BRIGHTUP,       DISABLE},
    {IR_BRIGHTDOWN_CODE,KEY_BRIGHTDOWN,     DISABLE},
    {IR_SETUP_CODE,     KEY_SETUP,          DISABLE},
    {IR_CANAEL_CODE,    KEY_CANCEL,         DISABLE},
    {IR_SAVE_CODE,      KEY_COPY,           DISABLE},
    {IR_SIZE_CODE,      KEY_ZOOM,           ENABLE},
    {IR_VOLUP_CODE,     KEY_VOLUP,          ENABLE},
    {IR_VOLDOWN_CODE,   KEY_VOLDOWN,        ENABLE},
    {IR_NULL_CODE,      KEY_NULL,           DISABLE},
};

////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////
#elif (REMOTE_CONTRLLER == REMOTE_MPX_STANDARD_24KEYS)

#define IR_UP_CODE              0x61d622dd  // 1
#define IR_DOWN_CODE            0x61d612ed  // 2
#define IR_LEFT_CODE            0x61d632cd  // 3
#define IR_RIGHT_CODE           0x61d602fd  // 4
#define IR_EXIT_CODE            0x61d6f00f  // 5
#define IR_VOLUP_CODE           0x61d640bf  // 6
#define IR_VOLDOWN_CODE         0x61d6708f  // 7
#define IR_ENTER_CODE           0x61d6609f  // 8
#define IR_ZOOM_CODE            0x61d63ac5  // 9
#define IR_ROTATE_CODE          0x61d61ae5  // 10
#define IR_SOURCE_CODE          0x61d62ad5  // 11
#define IR_PIC_MP3_CODE         0x61d6ea15  // 12
#define IR_SETUP_CODE           0x61d6926d  // 13
#define IR_SELECT_CODE          0x61d60af5  // 14
#define IR_PHOTOMODE_CODE       0x61d6aa55  // 15
#define IR_MUSICMODE_CODE       0x61d68a75  // 16
#define IR_VIDEOMODE_CODE       0x61d6f20d  // 17
//#define IR_CHANGEUI_CODE        0x61d6e01f // Unknown, Jonny off 20090511
#define IR_CALENDAR_CODE        0x61d67a85  // 18
#define IR_CLOCK_CODE           0x61d6f807  // 19

#define IR_MUTE_CODE            0x61d630CF  // 20
#define IR_PAUSE_RESUME_CODE    0x61d6b847  // 21
#define IR_TV_O_P_CODE          0x61d6b04f  // 22
#define IR_POWER_CODE           0x61d600FF  // 23
#define IR_ALARM_CODE           0x61D64AB5  // 24

#define IR_NULL_CODE            0x0

static const ST_IR_KEY sIRMapTable[] = {
    // IR Code          Key Code            Enable Repeat Code
    {IR_UP_CODE,        KEY_UP,             ENABLE},    // 1
    {IR_DOWN_CODE,      KEY_DOWN,           ENABLE},    // 2
    {IR_LEFT_CODE,      KEY_LEFT,           ENABLE},    // 3
    {IR_RIGHT_CODE,     KEY_RIGHT,          ENABLE},    // 4
    {IR_ENTER_CODE,     KEY_ENTER,          DISABLE},   // 5
    {IR_EXIT_CODE,      KEY_EXIT,           DISABLE},   // 6
    {IR_ROTATE_CODE,    KEY_ROTATE,         ENABLE},    // 7
    {IR_PIC_MP3_CODE,   KEY_PIC_MP3,        DISABLE},   // 8
    {IR_SETUP_CODE,     KEY_SETUP,          DISABLE},   // 9
    {IR_ZOOM_CODE,      KEY_ZOOM,           ENABLE},    // 10
    {IR_VOLUP_CODE,     KEY_VOLUP,          ENABLE},    // 11
    {IR_VOLDOWN_CODE,   KEY_VOLDOWN,        ENABLE},    // 12
    {IR_SELECT_CODE,    KEY_SELECT,         DISABLE},   // 13
    {IR_SOURCE_CODE,    KEY_SOURCE,         DISABLE},   // 14
    {IR_PHOTOMODE_CODE, KEY_PHOTOMODE,      DISABLE},   // 15
    {IR_MUSICMODE_CODE, KEY_MUSICMODE,      DISABLE},   // 16
    {IR_VIDEOMODE_CODE, KEY_VIDEOMODE,      DISABLE},   // 17
    //{IR_CHANGEUI_CODE,  KEY_CHANGEUI,       DISABLE},   // Unknown, Jonny off 20090511
//#if RTC_ENABLE
    {IR_CALENDAR_CODE,  KEY_CALENDAR,       DISABLE},   // 18
    {IR_CLOCK_CODE,     KEY_CLOCK,          DISABLE},   // 19
//#endif
    {IR_MUTE_CODE,      KEY_MUTE,           DISABLE},   // 20
    {IR_PAUSE_RESUME_CODE,KEY_PAUSE_RESUME, DISABLE},   // 21
    {IR_TV_O_P_CODE,    KEY_TV_O_P,         DISABLE},   // 22
    {IR_POWER_CODE,     KEY_POWER,          DISABLE},   // 23
    {IR_ALARM_CODE,     KEY_ALARM,          DISABLE},   // 24

    {IR_NULL_CODE,      KEY_NULL,           DISABLE},   //This one must be last
};

////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////
#elif (REMOTE_CONTRLLER == REMOTE_WA_15KEYS)

#define IR_EXIT_CODE            0x41bef00f
#define IR_SETUP_CODE           0x41be30cf
#define IR_UP_CODE              0x41bed02f
#define IR_VOLUP_CODE           0x41be10ef
#define IR_LEFT_CODE            0x41be40bf
#define IR_ENTER_CODE           0x41bec03f
#define IR_RIGHT_CODE           0x41beb04f
#define IR_VOLDOWN_CODE         0x41be708f
#define IR_PLAY_PAUSE_CODE      0x41bee01f
#define IR_DOWN_CODE            0x41be609f
#define IR_STOP_CODE            0x41bea05f
#define IR_ROTATE_CODE          0x41be906f
#define IR_ZOOM_CODE            0x41be50af
#define IR_PIC_MP3_CODE         0x41be20df
#define IR_MUTE_CODE            0x41bec837
#define IR_NULL_CODE            0x0

#if 0
static const ST_IR_KEY sIRMapTable[] = {
    {IR_EXIT_CODE, KEY_EXIT},
    {IR_SETUP_CODE, KEY_SETUP},
    {IR_UP_CODE, KEY_UP},
    {IR_VOLUP_CODE, KEY_VOLUP},
    {IR_LEFT_CODE, KEY_LEFT},
    {IR_ENTER_CODE, KEY_ENTER},
    {IR_RIGHT_CODE, KEY_RIGHT},
    {IR_VOLDOWN_CODE, KEY_VOLDOWN},
    {IR_PLAY_PAUSE_CODE, KEY_PP},
    {IR_DOWN_CODE, KEY_DOWN},
    {IR_STOP_CODE, KEY_STOP},
    {IR_ROTATE_CODE, KEY_ROTATE},
    {IR_ZOOM_CODE, KEY_ZOOM},
    {IR_PIC_MP3_CODE, KEY_PIC_MP3},
    {IR_MUTE_CODE, KEY_MUTE},
    {IR_NULL_CODE, KEY_NULL},
};
#else
static const ST_IR_KEY sIRMapTable[] = {
    // IR Code          Key Code        Enable Repeat Code
    {IR_UP_CODE,            KEY_UP,         ENABLE},
    {IR_DOWN_CODE,          KEY_DOWN,       ENABLE},
    {IR_LEFT_CODE,          KEY_LEFT,       ENABLE},
    {IR_RIGHT_CODE,         KEY_RIGHT,      ENABLE},
    {IR_ENTER_CODE,         KEY_ENTER,      DISABLE},
    {IR_EXIT_CODE,          KEY_EXIT,       DISABLE},
    {IR_ROTATE_CODE,        KEY_ROTATE,     ENABLE},
    {IR_PIC_MP3_CODE,       KEY_PIC_MP3     DISABLE},
    {IR_SETUP_CODE,         KEY_SETUP,      DISABLE},
    {IR_ZOOM_CODE,          KEY_ZOOM,       DISABLE},
    //{IR_VOLUP_CODE,         KEY_VOLUP,      ENABLE},
    //{IR_VOLDOWN_CODE,       KEY_VOLDOWN,    ENABLE},
    {IR_VOLUP_CODE,         KEY_SELECT,     DISABLE},
    {IR_VOLDOWN_CODE,       KEY_SOURCE,     DISABLE},
    {IR_MUTE_CODE,          KEY_PHOTOMODE,  DISABLE},
    {IR_PLAY_PAUSE_CODE,    KEY_MUSICMODE,  DISABLE},
    {IR_STOP_CODE,          KEY_VIDEOMODE,  DISABLE},
    {IR_NULL_CODE,          KEY_NULL,       DISABLE},
};
#endif

////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////
#elif (REMOTE_CONTRLLER == REMOTE_CS_16KEY)
//chisam new ir
#define IR_EXIT_CODE            0x41be18e7
#define IR_SETUP_CODE           0x41bef00f
#define IR_MUTE_CODE            0x41bec837
#define IR_UP_CODE              0x41be30cf
#define IR_VOLUP_CODE           0x41bea05f
#define IR_LEFT_CODE            0x41be906f
#define IR_ENTER_CODE           0x41be50af
#define IR_RIGHT_CODE           0x41be10ef
#define IR_VOLDOWN_CODE         0x41be20df
#define IR_REPEAT_CODE          0x41be807f  ////fengrs 6/13 use to do file copy(PHOTO/MOVIE)
#define IR_DOWN_CODE            0x41bed02f
//#define IR_STOP_CODE            0x10a7609f
#define IR_INFO_CODE            0x41be708f  //fengrs 6/13 use to do power off(THUMB)
#define IR_ROTATE_CODE          0x41bee01f
#define IR_ZOOM_CODE            0x41bec03f
#define IR_PIC_MP3_CODE         0x41beb04f
#define IR_CANAEL_CODE          0x41be609f  //fengrs 6/13 use to do file-delete
//#define IR_SAVE_CODE            0x41be807f//fengrs 6/13 use to do file copy(PHOTO/MOVIE)

#define IR_NULL_CODE            0x0

static const ST_IR_KEY sIRMapTable[] = {
    // IR Code          Key Code        Enable Repeat Code
    {IR_EXIT_CODE,      KEY_EXIT,       DISABLE},
    {IR_SETUP_CODE,     KEY_SETUP,      DISABLE},
    {IR_MUTE_CODE,      KEY_MUTE,       DISABLE},
    {IR_UP_CODE,        KEY_UP,         ENABLE},
    {IR_VOLUP_CODE,     KEY_VOLUP,      ENABLE},
    {IR_LEFT_CODE,      KEY_LEFT,       ENABLE},
    {IR_ENTER_CODE,     KEY_ENTER,      DISABLE},
    {IR_RIGHT_CODE,     KEY_RIGHT,      ENABLE},
    {IR_VOLDOWN_CODE,   KEY_VOLDOWN,    ENABLE},
    {IR_DOWN_CODE,      KEY_DOWN,       ENABLE},
    //{IR_STOP_CODE,      KEY_STOP,       DISABLE},
    {IR_REPEAT_CODE,    KEY_REPEAT,     DISABLE},//PHOTO/MOVIE
    {IR_INFO_CODE,      KEY_INFO,       DISABLE},
    {IR_ROTATE_CODE,    KEY_ROTATE,     ENABLE},
    {IR_ZOOM_CODE,      KEY_ZOOM,       ENABLE},
    {IR_PIC_MP3_CODE,   KEY_PIC_MP3,    DISABLE},
    {IR_CANAEL_CODE,    KEY_CANCEL,     DISABLE},
    //{IR_SAVE_CODE,      KEY_COPY,       DISABLE},
    {IR_NULL_CODE,      KEY_NULL,       DISABLE},
};

////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////
#elif (REMOTE_CONTRLLER == REMOTE_MPX_TV)

#define IR_UP_CODE              0x106722dd  // 1
#define IR_DOWN_CODE            0x106712ed  // 2
#define IR_LEFT_CODE            0x106732cd  // 3
#define IR_RIGHT_CODE           0x106702fd  // 4
#define IR_EXIT_CODE            0x1067f00f  // 5
#define IR_VOLUP_CODE           0x106740bf  // 6
#define IR_VOLDOWN_CODE         0x1067708f  // 7
#define IR_ENTER_CODE           0x1067609f  // 8
#define IR_ZOOM_CODE            0x10673ac5  // 9
#define IR_ROTATE_CODE          0x10671ae5  // 10
#define IR_SOURCE_CODE          0x10672ad5  // 11
#define IR_PIC_MP3_CODE         0x1067ea15  // 12
#define IR_SETUP_CODE           0x1067926d  // 13
#define IR_SELECT_CODE          0x10670af5  // 14
#define IR_PHOTOMODE_CODE       0x1067aa55  // 15
#define IR_MUSICMODE_CODE       0x10678a75  // 16
#define IR_VIDEOMODE_CODE       0x1067f20d  // 17
//#define IR_CHANGEUI_CODE        0x1067e01f // Unknown, Jonny off 20090511
#define IR_CALENDAR_CODE        0x10677a85  // 18
#define IR_CLOCK_CODE           0x1067f807  // 19

#define IR_MUTE_CODE            0x106730CF  // 20
#define IR_PAUSE_RESUME_CODE    0x1067b847  // 21
#define IR_TV_O_P_CODE          0x1067b04f  // 22
#define IR_POWER_CODE           0x106700FF  // 23
#define IR_ALARM_CODE           0x61D64AB5  // 24

#define IR_NULL_CODE            0x0

static const ST_IR_KEY sIRMapTable[] = {
    // IR Code          Key Code            Enable Repeat Code
    {IR_UP_CODE,        KEY_UP,             ENABLE},    // 1
    {IR_DOWN_CODE,      KEY_DOWN,           ENABLE},    // 2
    {IR_LEFT_CODE,      KEY_LEFT,           ENABLE},    // 3
    {IR_RIGHT_CODE,     KEY_RIGHT,          ENABLE},    // 4
    {IR_ENTER_CODE,     KEY_ENTER,          DISABLE},   // 5
    {IR_EXIT_CODE,      KEY_EXIT,           DISABLE},   // 6
    {IR_ROTATE_CODE,    KEY_ROTATE,         ENABLE},    // 7
    {IR_PIC_MP3_CODE,   KEY_PIC_MP3,        DISABLE},   // 8
    {IR_SETUP_CODE,     KEY_SETUP,          DISABLE},   // 9
    {IR_ZOOM_CODE,      KEY_ZOOM,           ENABLE},    // 10
    {IR_VOLUP_CODE,     KEY_VOLUP,          ENABLE},    // 11
    {IR_VOLDOWN_CODE,   KEY_VOLDOWN,        ENABLE},    // 12
    {IR_SELECT_CODE,    KEY_SELECT,         DISABLE},   // 13
    {IR_SOURCE_CODE,    KEY_SOURCE,         DISABLE},   // 14
    {IR_PHOTOMODE_CODE, KEY_PHOTOMODE,      DISABLE},   // 15
    {IR_MUSICMODE_CODE, KEY_MUSICMODE,      DISABLE},   // 16
    {IR_VIDEOMODE_CODE, KEY_VIDEOMODE,      DISABLE},   // 17
    //{IR_CHANGEUI_CODE,  KEY_CHANGEUI,       DISABLE},   // Unknown, Jonny off 20090511
//#if RTC_ENABLE
    {IR_CALENDAR_CODE,  KEY_CALENDAR,       DISABLE},   // 18
    {IR_CLOCK_CODE,     KEY_CLOCK,          DISABLE},   // 19
//#endif
    {IR_MUTE_CODE,      KEY_MUTE,           DISABLE},   // 20
    {IR_PAUSE_RESUME_CODE,KEY_PAUSE_RESUME, DISABLE},   // 21
    {IR_TV_O_P_CODE,    KEY_TV_O_P,         DISABLE},   // 22
    {IR_POWER_CODE,     KEY_POWER,          DISABLE},   // 23
    {IR_ALARM_CODE,     KEY_ALARM,          DISABLE},   // 24

    {IR_NULL_CODE,      KEY_NULL,           DISABLE},   //This one must be last
};

////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////
#elif (REMOTE_CONTRLLER == REMOTE_SONY_RMT_DPF)

#define IR_UP_CODE              0x0004EB3F
#define IR_DOWN_CODE            0x000CEB3F
#define IR_LEFT_CODE            0x000AEB3F
#define IR_RIGHT_CODE           0x0002EB3F
#define IR_EXIT_CODE            0x000C6B3F
#define IR_VOLUP_CODE           0xFFFFFFFF
#define IR_VOLDOWN_CODE         0xFFFFFFFF
#define IR_ENTER_CODE           0x0006EB3F
#define IR_ZOOM_CODE            0x000EEB3F
#define IR_ROTATE_CODE          0x0007EB3F
#define IR_SOURCE_CODE          0xFFFFFFFF
#define IR_PIC_MP3_CODE         0x00008B3F
#define IR_SETUP_CODE           0x000B2B3F
#define IR_SELECT_CODE          0xFFFFFFFF
#define IR_PHOTOMODE_CODE       0x00004B3F
#define IR_MUSICMODE_CODE       0xFFFFFFFF
#define IR_VIDEOMODE_CODE       0x00026B3F
#define IR_CALENDAR_CODE        0xFFFFFFFF
#define IR_CLOCK_CODE           0x000BCB3F

#define IR_MUTE_CODE            0xFFFFFFFF
#define IR_PAUSE_RESUME_CODE    0xFFFFFFFF
#define IR_TV_O_P_CODE          0xFFFFFFFF
#define IR_POWER_CODE           0x000A8B3F
#define IR_ALARM_CODE           0xFFFFFFFF

#define IR_NULL_CODE            0xFFFFFFFF

static const ST_IR_KEY sIRMapTable[] = {
    // IR Code          Key Code            Enable Repeat Code
    {IR_UP_CODE,        KEY_UP,             DISABLE},    // 1
    {IR_DOWN_CODE,      KEY_DOWN,           DISABLE},    // 2
    {IR_LEFT_CODE,      KEY_LEFT,           DISABLE},    // 3
    {IR_RIGHT_CODE,     KEY_RIGHT,          DISABLE},    // 4
    {IR_ENTER_CODE,     KEY_ENTER,          DISABLE},   // 5
    {IR_EXIT_CODE,      KEY_EXIT,           DISABLE},   // 6
    {IR_ROTATE_CODE,    KEY_ROTATE,         DISABLE},    // 7
    {IR_PIC_MP3_CODE,   KEY_PIC_MP3,        DISABLE},   // 8
    {IR_SETUP_CODE,     KEY_SETUP,          DISABLE},   // 9
    {IR_ZOOM_CODE,      KEY_ZOOM,           DISABLE},    // 10
    {IR_VOLUP_CODE,     KEY_VOLUP,          DISABLE},    // 11
    {IR_VOLDOWN_CODE,   KEY_VOLDOWN,        DISABLE},    // 12
    {IR_SELECT_CODE,    KEY_SELECT,         DISABLE},   // 13
    {IR_SOURCE_CODE,    KEY_SOURCE,         DISABLE},   // 14
    {IR_PHOTOMODE_CODE, KEY_PHOTOMODE,      DISABLE},   // 15
    {IR_MUSICMODE_CODE, KEY_MUSICMODE,      DISABLE},   // 16
    {IR_VIDEOMODE_CODE, KEY_VIDEOMODE,      DISABLE},   // 17
    {IR_CALENDAR_CODE,  KEY_CALENDAR,       DISABLE},   // 18
    {IR_CLOCK_CODE,     KEY_CLOCK,          DISABLE},   // 19
    {IR_MUTE_CODE,      KEY_MUTE,           DISABLE},   // 20
    {IR_PAUSE_RESUME_CODE,KEY_PAUSE_RESUME, DISABLE},   // 21
    {IR_TV_O_P_CODE,    KEY_TV_O_P,         DISABLE},   // 22
    {IR_POWER_CODE,     KEY_POWER,          DISABLE},   // 23
    {IR_ALARM_CODE,     KEY_ALARM,          DISABLE},   // 24

    {IR_NULL_CODE,      KEY_NULL,           DISABLE},   //This one must be last
};

#elif (REMOTE_CONTRLLER == REMOTE_MP662W_MINIDV)

#define IR_UP_CODE              0x00FFD827
#define IR_DOWN_CODE            0x00FF58A7
#define IR_LEFT_CODE            0x00FFAA55
#define IR_RIGHT_CODE           0x00FFA857
#define IR_ENTER_CODE           0x00FFE817
#define IR_EXIT_CODE            0x00FF9A65
#define IR_ROTATE_CODE          0x00FF18E7
#define IR_ZOOM_CODE            0x00FFBA45
#define IR_DELETE_CODE          0x00FFB847
#define IR_LOCK_CODE            0x00FF2AD5
#define IR_MENU_CODE            0x00FF8877
#define IR_VOLUP_CODE           0x00FF38C7
#define IR_VOLDOWN_CODE         0x00FF9867
#define IR_MODETECT_CODE        0x00FF1AE5
#define IR_AV1AV2_CODE          0x00FF6897
#define IR_MP4_CODE             0x00FFF807
#define IR_MP3_CODE             0x00FF7887
#define IR_POWER_CODE           0x00FF3AC5
#define IR_NULL_CODE            0x0

static const ST_IR_KEY sIRMapTable[] = {
    // IR Code          Key Code            Enable Repeat Code
    {IR_UP_CODE,        KEY_UP,         ENABLE},
    {IR_DOWN_CODE,      KEY_DOWN,       ENABLE},
    {IR_ENTER_CODE,     KEY_ENTER,      DISABLE},
    {IR_EXIT_CODE,      KEY_EXIT,       DISABLE},
    {IR_MENU_CODE,      KEY_MENU,       DISABLE},
    {IR_POWER_CODE,     KEY_POWER,      DISABLE},
    {IR_NULL_CODE,      KEY_NULL,       DISABLE},   //This one must be last
};


#elif (REMOTE_CONTRLLER==PID_IR_CR2025)
#define IR_POWER_CODE            0x01FE48B7//IR_MUTE_CODE
#define IR_REPEAT_CODE          0x01FE58A7
#define IR_MUTE_CODE           0x01FE7887//IR_POWER_CODE
#define IR_PHOTOMODE_CODE       0x01FE807F
#define IR_MUSICMODE_CODE       0x01FE40BF
#define IR_VIDEOMODE_CODE       0x01FEC03F  
#define IR_PAUSE_RESUME_CODE    0x01FE20DF
#define IR_UP_CODE              0x01FEA05F
#define IR_DOWN_CODE            0x01FED827
#define IR_LEFT_CODE            0x01FEE01F
#define IR_RIGHT_CODE           0x01FE906F
#define IR_ENTER_CODE           0x01FE10EF
#define IR_MENU_CODE           0x01FE609F
#define IR_SETUP_CODE           0x01FE50AF
#define IR_BACK_MUSIC_CODE        0x01FEF807
#define IR_VOLUP_CODE           0x01FE30CF
#define IR_FB_CODE           0x01FEB04F 
#define IR_PREVIOUS_CODE        0x01FE708F
#define IR_VOLDOWN_CODE         0x01FE00FF
#define IR_FF_CODE           0x01FEF00F 
#define IR_NEXT_CODE            0x01FE9867
#define IR_NULL_CODE            0x0

static const ST_IR_KEY sIRMapTable[] = {
	{IR_REPEAT_CODE, KEY_REPEAT,ENABLE},                                 //KEY_UP
	{IR_UP_CODE, KEY_UP,ENABLE},                                 //KEY_UP
	{IR_DOWN_CODE, KEY_DOWN,ENABLE},                    //KEY_DOWN
	{IR_LEFT_CODE, KEY_LEFT,ENABLE},                       //KEY_LEFT
	{IR_RIGHT_CODE, KEY_RIGHT,ENABLE},                      //KEY_RIGHT
	{IR_ENTER_CODE, KEY_ENTER,ENABLE},
	{IR_BACK_MUSIC_CODE, KEY_EXIT,DISABLE},
//	{IR_MENU_CODE, KEY_MENU,DISABLE},
	{IR_SETUP_CODE, KEY_SETUP,DISABLE},
	{IR_POWER_CODE,KEY_POWER,DISABLE},
	{IR_PHOTOMODE_CODE, KEY_PHOTOMODE,DISABLE},
	{IR_MUSICMODE_CODE, KEY_MUTE,ENABLE},
	{IR_VIDEOMODE_CODE, KEY_VIDEOMODE,DISABLE},
	{IR_VOLUP_CODE, KEY_VOLUP,ENABLE},
	{IR_VOLDOWN_CODE, KEY_VOLDOWN,ENABLE},
#if PID7DW_ZC6252V2_15DW
	{IR_MUTE_CODE, KEY_MUTE,DISABLE},
#else
	{IR_MUTE_CODE, KEY_SETUP,DISABLE},//KEY_MUTE
#endif

	{IR_PREVIOUS_CODE, KEY_VOLDOWN,DISABLE},
	
	{IR_NULL_CODE, KEY_NULL,DISABLE},
};

#elif (REMOTE_CONTRLLER==REMOTE_YBD_4WIN)
#define IR_POWER_CODE            0x01FE48B7//no use
#define IR_NULL_CODE            0x0
static const ST_IR_KEY sIRMapTable[] = {
	{0x00FF708F, KEY_WINDOWMODE,DISABLE},
	{0x00FF807F, KEY_UP,DISABLE},
	{0x00FF00FF, KEY_DOWN,DISABLE},
	{0x00FF50AF, KEY_DEL_WIN_1,DISABLE},
	{0x00FFA857, KEY_DEL_WIN_2,DISABLE},
	{0x00FFB04F, KEY_DEL_WIN_3,DISABLE},
	{0x00FF6897, KEY_DEL_WIN_4,DISABLE},
	{0x00FF30CF, KEY_DEL_PAGE,DISABLE},
	{0x00FF906F, KEY_DEL_ALL,DISABLE},
	{0x00FF28D7, KEY_ENTER,DISABLE},

	{IR_NULL_CODE, KEY_NULL,DISABLE},
};

#else

#define IR_NULL_CODE            0x0

static const ST_IR_KEY sIRMapTable[] = {
    // IR Code          Key Code            Enable Repeat Code
    {IR_NULL_CODE,      KEY_NULL,           DISABLE},   //This one must be last
};

#endif



/*
// Variable declarations
*/
static DWORD g_dwIrKey = 0;


/*
// Definition of external functions
*/
///
///@ingroup IR
///
///@brief   Get IR key
///
///@param
///
///@retval
///
DWORD Ui_GetIrKey(void)
{
    return g_dwIrKey;
}



///
///@ingroup IR
///
///@brief   Set IR key
///
///@param
///
///@retval
///
void Ui_SetIrKey(DWORD key)
{
    g_dwIrKey = key;
}



///
///@ingroup IR
///
///@brief   Isr Call back function
///
///@param   keyCode - input keyCode
///
///@param   state - IR_NECDATA or IR_NECREPEAT
///@retval
///
///Remark   add key repeat and hold
static void uiIrIsrCallback(DWORD keyCode, BYTE state)
{
    g_dwIrKey = keyCode;

#ifdef REPEAT_CODE_AS_KEY_DOWN
    EventSet(UI_EVENT, EVENT_IR_KEY_DOWN);
#else
    if (state == IR_NECDATA)
    {
        EventSet(UI_EVENT, EVENT_IR_KEY_DOWN);
    }
    else if (state == IR_NECREPEAT)
    {   // NEC repeat code
        EventSet(UI_EVENT, EVENT_IR);
    }
#endif
}



///
///@ingroup IR
///
///@brief   Init IR
///
///@param
///
///@retval
///
void Ui_IR_Init(void)
{
#if (REMOTE_CONTRLLER == REMOTE_SONY_RMT_DPF)
    //IR_TypeConfig(IR_SONY_SIRC_DECODER_BIT_TYPE, 20);
    IR_TypeConfig(IR_SONY_SIRC_DECODER_PACKET_TYPE, 20);
#else
    IR_TypeConfig(IR_NEC_DECODER, 32);
#endif
    IR_Init();
    IR_RegisterCallBackFunc(uiIrIsrCallback);
    IR_KeyTabSet((ST_IR_KEY *) &sIRMapTable, sizeof(sIRMapTable) / sizeof(ST_IR_KEY));
}



///
///@ingroup IR
///
///@brief
///
///@param
///
///@retval
///
DWORD Ui_IrPowerKeyCodeGet(void)
{
    return IR_POWER_CODE;
}



////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

MPX_KMODAPI_SET(Ui_SetIrKey);
MPX_KMODAPI_SET(Ui_GetIrKey);
MPX_KMODAPI_SET(Ui_IR_Init);

#endif      // #if (IR_ENABLE == 1)

