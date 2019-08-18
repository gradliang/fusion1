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
* Filename      : MiniDV_StateMachine.c
* Programmer(s) : Allen Chen
* Created       : Allen Chen
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "peripheral.h"
#include "ui_timer.h"
#include "../../ui/include/uiGpio.h"
#include "MiniDV_StateMachine.h"
#include "Camcorder_func.h"
#include "Camera_func.h"
#include "PhotoFrame.h"
#include "record.h"
#include "xpgCamFunc.h"
#include "xpgFunc.h"
/*
// Constant declarations
*/

#if STATE_MACHINE_ENABLE

#define memcpy      MpMemCopy
#define memset      MpMemSet
#define NEXT_KEY    4
#define __FAST_FLASH_PERIOD__ 200
#define POWER_SHUTDOWN_ALARM_PERIOD 1000
/*
// Variable declarations
*/
//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
extern E_CAMCORDER_TIMING recordTiming_i;
extern AP_recording_stop, AP_recording_start;
int USB_otg_first = 0;
extern E_MINI_DV_MODE g_MinidvMode;

static void ledMode_PowerDown(void)
{
        Led_ModeLedToggle();
        Ui_TimerProcAdd(__FAST_FLASH_PERIOD__, ledMode_PowerDown);
}

static void stateMachine_StateHandler_PowerDown(DWORD newState)
{
    switch (newState)
    {
    case STATE_STANDY_BY:
        Ui_TimerProcRemove(ledMode_PowerDown);
        break;

    case STATE_POWER_DOWN_MODE:
        break;

    case STATE_POWER_DOWN_LOOP:
        Ui_TimerProcRemove(ledMode_PowerDown);
#if defined(__PWR_RN_GPIO__)&&(__PWR_RN_GPIO__ != GPIO_GPIO_NULL)
        Ui_SystemPowerHold(FALSE);
#endif
        break;

    default:
        MP_ALERT("--E-- Can not change state from STATE_POWER_DOWN_MODE to others");
        break;
    }
}

static void stateMachine_PowerShutDown(void)
{
    stateMachine_StateHandler_PowerDown(STATE_POWER_DOWN_LOOP);
}

void StateMachine_Handler(DWORD dwEvent, BYTE *infoPtr)
{

    switch (dwEvent)
    {
    ////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////

    case STATE_POWER_DOWN_MODE:
		
		if(g_MinidvMode == CAMRECORDING)
			xpgCb_CamCoderRecordStop();
		
		stateMachine_PowerShutDown();
        break;

    ////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////    
    case EVENT_GPIO_KEY:
        break;

    ////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////
    case EVENT_DISK_FULL:
        MP_ALERT("EVENT_DISK_FULL -");

        if((g_bAniFlag & ANI_CAMRECORDING)){        
            MP_ALERT("STARTING CYCLIC RECORDING - RECORDSTOP !!");
            Camcorder_RecordStop();  
            MP_ALERT("STARTING CYCLIC RECORDING - RECORDSTART !!");            
            Camcorder_RecordStart(1);       
        }
        break;

    ////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////
    case EVENT_DISK_FATAL_ERR:
        MP_ALERT("EVENT_DISK_FATAL_ERR -");
        break;

    ////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////
    case EVENT_USB0_CHG:
        MP_ALERT("EVENT_USB0_CHG -");
        if(USB_otg_first == 0){
            Camcorder_PreviewStop();
            USB_otg_first = 1;   
        }
        else{
            Camcorder_PreviewStart(recordTiming_i);
            USB_otg_first = 0;
        }
        break;
        
    ////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////        
    case EVENT_USB1_CHG:
        MP_ALERT("EVENT_USB1_CHG -");
        break;

    ////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////
    case EVENT_LOW_BATTERY:
        MP_ALERT("EVENT_LOW_BATTERY -");
        Led_StandbyLedSet(__LED_TURN_OFF__);
        Led_ModeLedSet(__LED_TURN_ON__);
        break;

    ////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////
    case EVENT_CARD_OUT:
        MP_ALERT("EVENT_CARD_OUT -");
        if(g_MinidvMode == CAMRECORDING){
            Idu_OsdErase();     
        }
        break;
            
    ////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////
    case EVENT_CARD_IN:
        MP_ALERT("EVENT_CARD_IN -");
        if(g_MinidvMode == CAMRECORDING){
            Camcorder_PreviewStart(recordTiming_i);
        }
        break;
        
    }
}

#endif

