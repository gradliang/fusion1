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
* Filename      : ui_timer.c
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
#include "ui_timer.h"
#include "taskid.h"
#include "ui.h"
#include "../../../libiPlay/libSrc/Os/Include/osinner.h"

/*
// Constant declarations
*/
#define MAX_UI_TIMER_PROC       32

/*
// Structure declarations
*/
struct ST_UI_TIMER_PROC_TAG
{
    volatile struct ST_UI_TIMER_PROC_TAG *sPrevTimerProc;
    volatile struct ST_UI_TIMER_PROC_TAG *sNextTimerProc;
    volatile DWORD dwCounterBase;
    volatile DWORD dwOffsetValue;
    void (*Action) (void);
};

struct ST_UI_TIMER_PROC_LIST_TAG
{
    volatile struct ST_UI_TIMER_PROC_TAG *sHeadTimerProc;
    volatile struct ST_UI_TIMER_PROC_TAG *sRearTimerProc;
    volatile DWORD dwCounter;
};

/*
// Type declarations
*/
typedef struct ST_UI_TIMER_PROC_LIST_TAG    ST_UI_TIMER_PROC_LIST;
typedef struct ST_UI_TIMER_PROC_TAG         ST_UI_TIMER_PROC;

/*
// Function declarations
*/


/*
// Variable declarations
*/
static ST_UI_TIMER_PROC         stUiTimerProc[MAX_UI_TIMER_PROC];
static ST_UI_TIMER_PROC_LIST    stUiTimerProcList;

/*
// Macro declarations
*/
#define UI_TIMER_PROC_ZERO      &stUiTimerProc[0]
#define UI_TIMER_PROC_END       &stUiTimerProc[(MAX_UI_TIMER_PROC - 1)]


//---------------------------------------------------------------------------
//             function for timer process
//---------------------------------------------------------------------------
static void currTimerProcRemove(ST_UI_TIMER_PROC *spTimerProc)
{
    ST_UI_TIMER_PROC *spPrevTimerProc, *spNextTimerProc;

    MP_DEBUG("UI currTimerProcRemove -");

    if (!spTimerProc->Action)
    {
        MP_DEBUG("current timer proc already removed!!!!");

        return;
    }

    spPrevTimerProc = (ST_UI_TIMER_PROC *) spTimerProc->sPrevTimerProc;
    spNextTimerProc = (ST_UI_TIMER_PROC *) spTimerProc->sNextTimerProc;

    if ((spPrevTimerProc == 0) && (spNextTimerProc == 0))
    {
        stUiTimerProcList.sHeadTimerProc = 0;
        stUiTimerProcList.sRearTimerProc = 0;
    }
    else if ((spPrevTimerProc == 0) && (spNextTimerProc != 0))
    {
        stUiTimerProcList.sHeadTimerProc = spNextTimerProc;
        spNextTimerProc->sPrevTimerProc = 0;
    }
    else if ((spPrevTimerProc != 0) && (spNextTimerProc == 0))
    {
        stUiTimerProcList.sRearTimerProc = spPrevTimerProc;
        spPrevTimerProc->sNextTimerProc = 0;
    }
    else //if ((spPrevTimerProc != 0) && (spNextTimerProc != 0))
    {
        spNextTimerProc->sPrevTimerProc = spPrevTimerProc;
        spPrevTimerProc->sNextTimerProc = spNextTimerProc;
    }

    memset((BYTE *) spTimerProc, 0, sizeof(ST_UI_TIMER_PROC));

    if (stUiTimerProcList.dwCounter)
        stUiTimerProcList.dwCounter--;
}



//---------------------------------------------------------------------------
static void uiTimerProcInit(void)
{
    memset((BYTE *) &stUiTimerProc, 0, sizeof(ST_UI_TIMER_PROC) * MAX_UI_TIMER_PROC);
    stUiTimerProcList.sHeadTimerProc = 0;
    stUiTimerProcList.sRearTimerProc = 0;
    stUiTimerProcList.dwCounter = 0;
}



// the function call is called from system timer(timer0)
static void ui_Detect(void)
{
#if TSPI_ENBALE
	TSPI_Receive_Check();
#endif
#if GPIO_SIMPLE_KEY_ENABE
	SimpleGpioKeyScan();
#endif
#if ((TOUCH_CONTROLLER_ENABLE == ENABLE) && (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_GT911))
	TimerToKeyRelease();
#endif
}



static void uiTimerHandler(WORD count)
{
    if (stUiTimerProcList.dwCounter > 0)
    {
        EventSet(UI_EVENT, EVENT_TIMER);
    }

    ui_Detect();
}



SWORD Ui_TimerProcRemove(register void (*Action) (void))
{
    ST_UI_TIMER_PROC *spTimerProc, *spNextTimerProc;
    BYTE found = FALSE;

    if ((stUiTimerProcList.dwCounter == 0) || !Action)
    {
        MP_DEBUG("--W-- Ui_TimerProcRemove - Proc. array is empty or null pointer !!!!\r\n");

        return -1;
    }

    IntDisable();

    //get remove process
    spTimerProc = (ST_UI_TIMER_PROC *) stUiTimerProcList.sHeadTimerProc;

    while (spTimerProc != 0)
    {
        if (spTimerProc->Action == Action)
        {
            spNextTimerProc = (ST_UI_TIMER_PROC *) spTimerProc->sNextTimerProc;
            currTimerProcRemove((ST_UI_TIMER_PROC *) spTimerProc);
            spTimerProc = spNextTimerProc;
            found = TRUE;

            MP_DEBUG("Remove Timer Proc Addr = 0x%08X, Remain Timer Count = %02d", (DWORD) Action, stUiTimerProcList.dwCounter);
            //break;
        }
        else
        {
            spTimerProc = (ST_UI_TIMER_PROC *) spTimerProc->sNextTimerProc;
        }
    }

    if (!found)
        MP_DEBUG("Ui_TimerProcRemove - Can't found time proc - 0x%08X!!!", (DWORD) Action);

    IntEnable();

    return 0;
}



//---------------------------------------------------------------------------
// The unit of dwOffsetValue is ms
SWORD Ui_TimerProcAdd(register DWORD dwOffsetValue, register void (*Action) (void))
{
    ST_UI_TIMER_PROC *spTimerProc;
    DWORD i;

    IntDisable();

    if ((stUiTimerProcList.dwCounter >= MAX_UI_TIMER_PROC) || !Action)
    {
        MP_DEBUG("--E-- Ui_TimerProcAdd - Proc. array is full or callback is null pointer !!!!\r\n");
        IntEnable();

        return FAIL;
    }

    //change period when the callback was existed
    spTimerProc = UI_TIMER_PROC_ZERO;

    for (i = 0; i < MAX_UI_TIMER_PROC; i++)
    {
        if (spTimerProc->Action == Action)
        {
            spTimerProc->dwOffsetValue = dwOffsetValue;
            spTimerProc->dwCounterBase = GetSysTime();
            MP_DEBUG("--W-- Ui_TimerProcAdd - Callback Proc. already existed, just change time period !!!\r\n");
            IntEnable();

            return PASS;
        }

        spTimerProc++;
    }

    //get free timer process list
    spTimerProc = UI_TIMER_PROC_ZERO;

    for (i = 0; i < MAX_UI_TIMER_PROC; i++)
    {
        if (spTimerProc->Action == 0)
        {
            break;
        }

        spTimerProc++;
    }

    if (i == MAX_UI_TIMER_PROC)
    {
        MP_DEBUG("--E-- Ui_TimerProcAdd - Can't found the empty proc !!!!\r\nThis case should not happened !!!\r\n");
        IntEnable();

        return FAIL;
    }

    // change link
    stUiTimerProcList.dwCounter++;

    if (stUiTimerProcList.sHeadTimerProc == 0)
    {   // First
        stUiTimerProcList.sHeadTimerProc = spTimerProc;
        spTimerProc->sPrevTimerProc = 0;
    }
    else
    {   // Add
        spTimerProc->sPrevTimerProc = stUiTimerProcList.sRearTimerProc;
        stUiTimerProcList.sRearTimerProc->sNextTimerProc = spTimerProc;
    }

    stUiTimerProcList.sRearTimerProc = spTimerProc;
    spTimerProc->sNextTimerProc = 0;
    spTimerProc->Action = Action;
    spTimerProc->dwOffsetValue = dwOffsetValue;
    spTimerProc->dwCounterBase = GetSysTime();

    MP_DEBUG("Add Timer Proc Addr = 0x%08X, New Timer Count = %02d", (DWORD) Action, stUiTimerProcList.dwCounter);

    IntEnable();

    return PASS;
}



//
// Definition of local functions
//
void Ui_TimerProcessHandle(void)
{
    ST_UI_TIMER_PROC *spTimerProc, *spNextTimerProc;
    void (*callBackFunPtr) (void);

    if (stUiTimerProcList.dwCounter == 0)
    {
        return;
    }

    IntDisable();
    spTimerProc = (ST_UI_TIMER_PROC *) stUiTimerProcList.sHeadTimerProc;

    while (spTimerProc != 0)
    {
        spNextTimerProc = (ST_UI_TIMER_PROC *) spTimerProc->sNextTimerProc;

        if (spTimerProc->dwOffsetValue < SystemGetElapsedTime(spTimerProc->dwCounterBase))
        {
            callBackFunPtr = spTimerProc->Action;
            // some application will call add proc in Action()
            //RemoveTimerProc(spTimerProc->Action);
            MP_DEBUG("Timer Proc Handle = 0x%08X, Remain Timer Count = %02d", (DWORD) spTimerProc->Action, stUiTimerProcList.dwCounter - 1);
            currTimerProcRemove((ST_UI_TIMER_PROC *) spTimerProc);

            // Maybe current callback function is removed by pre-callback function
            if (callBackFunPtr)
            {
                IntEnable();
                callBackFunPtr();
            }

            IntDisable();
        }

        spTimerProc = spNextTimerProc;
    }

    IntEnable();
}


//---------------------------------------------------------------------------
int Ui_TimerProcessCheck(void)
{
    ST_UI_TIMER_PROC *spTimerProc;

    if (stUiTimerProcList.dwCounter == 0)
    {
        return FALSE;
    }

    spTimerProc = (ST_UI_TIMER_PROC *) stUiTimerProcList.sHeadTimerProc;

    while (spTimerProc != 0)
    {
        if (spTimerProc->dwOffsetValue < SystemGetElapsedTime(spTimerProc->dwCounterBase))
        {
            return TRUE;
        }

        spTimerProc = (ST_UI_TIMER_PROC *) spTimerProc->sNextTimerProc;
    }

    return FALSE;
}

DWORD CheckTimerAction(register void (*Action) (void))
{
	ST_UI_TIMER_PROC *spTimerProc;
	DWORD dwOffset;

	if (stUiTimerProcList.dwCounter == 0)
		return 0;

	spTimerProc = stUiTimerProcList.sHeadTimerProc;

	while (spTimerProc != 0)
	{
		if (spTimerProc->Action == Action)
		{
			dwOffset=SystemGetElapsedTime(spTimerProc->dwCounterBase);
			if (spTimerProc->dwOffsetValue > dwOffset)
				return (spTimerProc->dwOffsetValue-dwOffset);
			else
				return 1;
		}
		spTimerProc = spTimerProc->sNextTimerProc;
	}
	return 0;
}

void Ui_TimerProcInit(void)
{
    mpDebugPrint("### %s:%d ###", __FUNCTION__, __LINE__);
    uiTimerProcInit();
    SysTimerProcAdd(UI_TIMER_TICK_PERIOD, uiTimerHandler, FALSE);
}



void Ui_TimerProcPause(void)
{
    SysTimerProcPause(uiTimerHandler);
}



void Ui_TimerProcResume(void)
{
    SysTimerProcResume(uiTimerHandler);
}

//------------------------------------------------------------------------------
// UI ClockTimer
static ST_EXCEPTION_MSG stMsgContent4Clock = {0};
static BOOL clockTimerCreated = FALSE;
BOOL xpgGetclockTimerCreated()
{
    return clockTimerCreated;
}

static volatile WORD dropClockMsg = FALSE;
static void HHMMSSTimerProc(void)
{
    //mpDebugPrint("ClockTimerProc()");

    SDWORD retVal;

    if (dropClockMsg == FALSE)
    {
        dropClockMsg = TRUE;

        retVal = MessageDrop((BYTE) EXCEPTION_MSG_ID, (BYTE *) &stMsgContent4Clock, sizeof(ST_EXCEPTION_MSG));

        if (retVal != OS_STATUS_OK)
        {
            dropClockMsg = FALSE;
            MP_ALERT("--E-- Clock message drop !!!");
        }
    }
}

// Page Exit need to DeInit! By xpgGotoPage()
void HHMMSSExceptionTaskDeInit()
{
    mpDebugPrint("%s", __FUNCTION__);
    ExceptionTagRelease(stMsgContent4Clock.dwTag);
    clockTimerCreated = FALSE;
    SysTimerProcRemove(HHMMSSTimerProc);
}


