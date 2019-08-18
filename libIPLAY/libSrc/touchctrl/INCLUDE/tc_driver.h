/*
*******************************************************************************
*                           Magic Pixel Inc. Inc.                             *
*                  Copyright (c) 2007 -, All Rights Reserved                  *
*                                                                             *
* TC module driver.                                                           *
*                                                                             *
* File : tc_driver.h                                                          *
*                                                                             *
*                                                                             *
*                                                                             *
*******************************************************************************
*/
#ifndef _TC_DRIVER_H_
#define _TC_DRIVER_H_
/*************************************************************************/
/***                        Include Files                              ***/
/*************************************************************************/
#include "utiltypedef.h"
//#include "touchCtrller.h"


/*************************************************************************/
/***               Public Variable Declaration                         ***/
/*************************************************************************/
/* the msg type of TS task */
#define SYSTEM_MESSAGE_POINT        0x40        //???
#define TC_DOWN (SYSTEM_MESSAGE_POINT|0)
#define TC_POLLING (SYSTEM_MESSAGE_POINT|1)
#define TC_UP (SYSTEM_MESSAGE_POINT|2)
#define TC_ADJUST (SYSTEM_MESSAGE_POINT|3)
#define TC_STROKE_STOP (SYSTEM_MESSAGE_POINT|4)
#define TC_TEST (SYSTEM_MESSAGE_POINT|5)
#define TC_MOVEUP (SYSTEM_MESSAGE_POINT|6)
#define TC_MOVEDOWN (SYSTEM_MESSAGE_POINT|7)
#define TC_INT (SYSTEM_MESSAGE_POINT|8)

typedef struct
{
    WORD       x1;
    WORD       y1;
    WORD       x2;
    WORD       y2;
    WORD       z;
    WORD       reserved;
} Tc_point;

typedef struct
{
    // data members definition
    BYTE      *u08Name;
    // function members definition
    SDWORD(*TcInit) ();
    SDWORD(*TcSleep) (BYTE);
    SDWORD(*TcGetData) (Tc_point * data);
    SDWORD(*TcCheckInterrupt) ();
    SDWORD(*TcChangeSensitivity) (DWORD, DWORD, DWORD);
} ST_TC_DRIVER;

/* the msg structure between TS task and driver */
typedef struct
{
    BYTE       status;
} ST_TC_MSG;


/*************************************************************************/
/***                       Functions definition                        ***/
/*************************************************************************/
SDWORD       TcGetMsgId(void);
SDWORD TcDriverInit( ST_TC_DRIVER *stTcDriver );
void TouchIntEnable();

#endif

