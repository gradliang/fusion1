/*
*******************************************************************************
*                           Magic Pixel Inc. Inc.                             *
*                  Copyright (c) 2007 -, All Rights Reserved                  *
*                                                                             *
* TS module driver.                                                           *
*                                                                             *
* File : TsTask.h                                                             *
* By : Kevin Huang                                                            *
*                                                                             *
*                                                                             *
* Description : header file of Touch screen function                          *
*                                                                             *
* History : 2007/03/13 File created.                                          *
*                                                                             *
*******************************************************************************
*/
#ifndef _TC_TASK_H_
#define _TC_TASK_H_
/*************************************************************************/
/***                        Include Files                              ***/
/*************************************************************************/
#include "utiltypedef.h"
#include "os.h"
#include "tc_driver.h"

#define TC_POLL_TIMER_PERIOD_50MS 5
#define TC_POLL_TIMER_PERIOD_100MS 10
#define TC_POLL_TIMER_PERIOD_200MS 20
#define TC_POLL_TIMER_PERIOD_500MS 50
#define TC_POLL_TIMER_PERIOD_1000MS 100
#define TC_POLL_TIMER_PERIOD_DEFAULT TC_POLL_TIMER_PERIOD_50MS

#define TC_HWR_TIMER_PERIOD_1S 1
#define TC_HWR_TIMER_PERIOD_2S 2
#define TC_HWR_TIMER_PERIOD_3S 3
#define TC_HWR_TIMER_PERIOD_4S 4
#define TC_HWR_TIMER_PERIOD_5S 5
#define TC_HWR_TIMER_PERIOD_DEFAULT TC_HWR_TIMER_PERIOD_2S

/*************************************************************************/
/***               Public Variable Declaration                         ***/
/*************************************************************************/

/* return msg type of TS task */
#define TC_ADJUSTFAIL (SYSTEM_MESSAGE_POINT|16)
#define TC_START_ADJUST2 (SYSTEM_MESSAGE_POINT|17)
#define TC_START_ADJUST3 (SYSTEM_MESSAGE_POINT|18)
#define TC_FINISH (SYSTEM_MESSAGE_POINT|19)

/* error number*/
#define ERR_NO_TC_BASE                             0x800B0000

#define ERR_NO_TC_GENERAL_FAIL                     (1 + ERR_NO_TC_BASE)
#define ERR_NO_TC_TC_NOT_EXIST                     (2 + ERR_NO_TC_BASE)
#define ERR_NO_TC_HWR_NOT_EXIST                    (3 + ERR_NO_TC_BASE)
#define ERR_NO_TC_ADJUST_FAIL                      (4 + ERR_NO_TC_BASE)
#define ERR_NO_TC_NODATA                           (5 + ERR_NO_TC_BASE)
#define ERR_NO_TC_ADJUST_TIMEOUT                   (6 + ERR_NO_TC_BASE)


/*
typedef struct
{
    BYTE       status;
    BYTE       reserved[3];
    WORD       x1;
    WORD       y1;
    WORD       x2;
    WORD       y2;
    WORD       z;
} ST_TC_FULL_MSG;
*/

/* the msg structure between TS task and UI */
typedef struct
{
    BYTE       status;
    BYTE       reserved[3];
    WORD       x1;
    WORD       y1;
    WORD       x2;
    WORD       y2;

} ST_TC_DATA;


typedef struct
{
    SWORD x_off;
    SWORD y_off;
    WORD x_max;
    WORD y_max;
    WORD x_min;
    WORD y_min;
}ST_TC_PARA;



typedef void (*TC_GET_DATA_CALLBACK) (BYTE, WORD, WORD, WORD, WORD);

typedef struct
{
    // data members definition
    WORD      * pCross_width;
    WORD      * pCross_single_width;
    DWORD      * pPoll_timer;
    DWORD      * pHwr_timer;
    // function members definition
    SDWORD (*FuncInit) ();
    void(*FuncPosReset) (SWORD, SWORD, WORD, WORD, WORD, WORD);
    void(*FuncPosGet) (ST_TC_PARA* tcpara);
    SDWORD (*FuncGetTaskID) ();
    //SDWORD(*FuncGetTimerID) ();
    SDWORD(*FuncGetMsgId) ();
    SDWORD(*FuncGetRetMsgId) ();
    void(*FuncGetDataCallBackReg) (TC_GET_DATA_CALLBACK func);
    void(*FuncGetDataCallBackDeReg) ();
    SDWORD(*FuncClose) ();
    SDWORD(*FuncPause) ();
    SDWORD(*FuncResume) ();
    SDWORD(*FuncChgSensitivity) (DWORD, DWORD, DWORD);
} ST_TC_FUNC;


typedef void (*TC_FUNC_CONSTRUCTOR) (ST_TC_FUNC *);
typedef void (*TC_DRIVER_CONSTRUCTOR) (ST_TC_DRIVER *);

/*************************************************************************/
/***                        Functions definition                       ***/
/*************************************************************************/
void TcFuncConstruct(ST_TC_FUNC * TcFunc);

#endif
