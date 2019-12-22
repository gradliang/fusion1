#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "os.h"
#include "taskid.h"
#include "mpTrace.h"
#include "touchCtrller.h"
#include "tcapi.h"
#include "ui.h"
//#include "stdio.h"
//#include "stdlib.h"
//#include "string.h"
//#define TC_DEBUG


#if (TOUCH_CONTROLLER_ENABLE == ENABLE)


#define TC_POLLING_INTERVAL             50
#define DEFAULT_CROSS_WIDTH             13
#define DEFAULT_CROSS_SINGLE_WIDTH      5


typedef enum
{
    E_TC_NO_ADJUST = 0,
    E_TC_ADJUST1,
    E_TC_ADJUST2,
    E_TC_ADJUST3,
    E_TC_TEST,
} E_TC_STATE;


static ST_TC_DRIVER stTcDriver;
/*
static Tc_point  old_data;
static WORD touch_width, touch_height;
static WORD x_pen_pos, y_pen_pos;
static WORD x_Max, y_Max;
static WORD x_Min, y_Min;
static WORD x_max, x_min;
static WORD y_max, y_min;
static SWORD x_offset, y_offset;
static float x_Sub, x_SubI;
static float y_Sub, y_SubI;
*/
static WORD cross_width;
static WORD cross_single_width;

static TC_GET_DATA_CALLBACK TcGetDataCallBack = NULL;
static SDWORD s32TcMsgId = OS_STATUS_INVALID_ID;
static SDWORD s32TcRetMsgId = OS_STATUS_INVALID_ID;
static DWORD u32PollTimerPeriod;
static DWORD u32HwrTimer;
static BOOL boolMsgRetStop = FALSE;

#define TC_POLLING_ENABLE 0


#if TC_POLLING_ENABLE
static void TcTimerIsr(void);
#endif
static void TcStrokeTimerIsr(void);




/****************************************************************************
 **
 ** NAME:           TcGetDataCallBackReg
 **
 ** PARAMETERS:     Registered callback function
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Register callback.
 **
 ****************************************************************************/
static void TcGetDataCallBackReg(TC_GET_DATA_CALLBACK func)
{
    TcGetDataCallBack = func;
}



/****************************************************************************
 **
 ** NAME:           TcGetDataCallBackDeReg
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    DeRegister callback.
 **
 ****************************************************************************/
static void TcGetDataCallBackDeReg(void)
{
    TcGetDataCallBack = NULL;
}



/****************************************************************************
 **
 ** NAME:           swap
 **
 ** PARAMETERS:     a, b
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Swap two variable.
 **
 ****************************************************************************/
static void swap(volatile WORD * a, volatile WORD * b)
{
    volatile WORD temp;

    temp = *a;
    *a = *b;
    *b = temp;
}




/****************************************************************************
 **
 ** NAME:           TcGetMsgId
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Msg ID
 **
 ** DESCRIPTION:    The Msg is between TS task and TS driver. Driver need this
 **                 function to get Msg ID.
 **
 ****************************************************************************/
SDWORD TcGetMsgId(void)
{
    return s32TcMsgId;
}




/****************************************************************************
 **
 ** NAME:           TcGetRetMsgId
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  return msg ID
 **
 ** DESCRIPTION:    get return msg ID.
 **
 ****************************************************************************/
static SDWORD TcGetRetMsgId(void)
{
    return s32TcRetMsgId;
}



/****************************************************************************
 **
 ** NAME:           TouchCtrllerTask
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Tc task main function.
 **                 Four type of msg:
 **                 TC_DOWN : pen down msg from driver
 **                 TC_POLLING : pen down msg from TS timer
 **                 TC_UP : pen up msg from driver
 **                 TC_ADJUST : pen adjustment msg from upper layer(UI layer)
 **
 ****************************************************************************/
#if 0
void TouchCtrllerTask(void)
{
	ST_TC_MSG msg;
	Tc_point  data;
	ST_TC_DATA pos_data;


	ST_EXCEPTION_MSG stTouchCtrllerMsg;
	volatile void (*callbackFunPtr)(DWORD);
	SDWORD status;

	IntEnable();

	memset(&data, 0, sizeof(Tc_point));
	memset(&old_data, 0, sizeof(Tc_point));

	old_data.reserved = TC_UP;//init last status

	while(1)
	{
		MessageReceive(s32TcMsgId, (BYTE *) &msg);
		if(msg.status == TC_INT)
		{
			if ( stTcDriver.TcGetData(&data) == NO_ERR )
			{
				pos_data.x1 = data.x1;
				pos_data.y1 = data.y1;
				pos_data.x2 = data.x2;
				pos_data.y2 = data.y2;
				pos_data.status=data.reserved;

					
				
				EventSet(UI_EVENT, EVENT_TOUCH_COLTROLLER);
				MessageDrop(UI_TC_MSG_ID, (BYTE*)&pos_data, sizeof(pos_data));

			}
		}
	}
}
#else
void TouchCtrllerTask(void)
{
    ST_TC_MSG msg;
    Tc_point  data;
    ST_TC_DATA pos_data;
    //ST_LCD_CONTROL *lcd_ctrl;
    //WORD width, height;

    ST_EXCEPTION_MSG stTouchCtrllerMsg;
    volatile void (*callbackFunPtr)(DWORD);
    SDWORD status;

    IntEnable();

    memset(&data, 0, sizeof(Tc_point));
  //  memset(&old_data, 0, sizeof(Tc_point));

  //  old_data.reserved = TC_UP;//init last status

    while(1)
    {
        MessageReceive(s32TcMsgId, (BYTE *) &msg);
        if(msg.status == TC_DOWN)
        {
            if ( stTcDriver.TcGetData(&data) == NO_ERR )
            {
                MP_DEBUG("Tc Task : down %d %d %d %d %d", data.x1, data.y1, data.x2, data.y2, data.z);

				pos_data.x1 = data.x1;
				pos_data.y1 = data.y1;
				pos_data.status=TC_DOWN;	
				stTcDriver.TcSleep(1);
				MessageDrop(UI_TC_MSG_ID, (BYTE*)&pos_data, sizeof(pos_data));
				EventSet(UI_EVENT, EVENT_TOUCH_COLTROLLER);
                
            }
        }
        else if(msg.status == TC_INT)
        {
				stTcDriver.TcSleep(0);
        }
#if 0
        else if(msg.status == TC_POLLING)
        {
            /* Start TS timer to do next polling */
#if TC_POLLING_ENABLE
            MP_DEBUG("Start TS timer to do next polling");
            if(SysTimerProcAdd(TC_POLLING_INTERVAL, TcTimerIsr, TRUE) < 0)
            {
                MP_DEBUG("Tc Timer Start Failure ...");
            }
#endif
            if(!stTcDriver.TcCheckInterrupt())  //double check the int pin
            {
                if ( stTcDriver.TcGetData(&data) == NO_ERR )
                {
                    /* back up last data */
#ifdef TC_DEBUG
                    MP_DEBUG("Tc Task : move %d %d %d %d %d", data.x1, data.y1, data.x2, data.y2, data.z);
                    MP_DEBUG("move pos : %d %d", x_pen_pos, y_pen_pos);
#endif
                    
                }
            }
        }
        else if(msg.status == TC_UP)
        {
#if TC_POLLING_ENABLE
            MP_DEBUG("Timer remove TCTimeIsr");
            SysTimerProcRemove(TcTimerIsr);       // stop polling
#endif
            //MessageReset(s32TcMsgId);

//mpDebugPrint("-");
#ifdef TC_DEBUG
            MP_ALERT("Tc Task : up %d %d %d %d %d", old_data.x1, old_data.y1, old_data.x2, old_data.y2, old_data.z);
#endif
            /* 3 points adjustment */
            
        }
#endif
    }
}

#endif

/****************************************************************************
 **
 ** NAME:           TcTaskInit
 **
 ** PARAMETERS:     msgId from UI layer
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    initial.
 **
 ****************************************************************************/
static SDWORD TouchCtrllerTaskInit(void)
{
    MP_DEBUG("__%s__", __FUNCTION__);
    TcDriverInit(&stTcDriver);

    if(stTcDriver.TcInit())
    {
        MP_ALERT("Touch controller init fail");
        return -1;
    }

    MessageCreate(s32TcMsgId, OS_ATTR_FIFO, sizeof(ST_TC_MSG) * 128);
    MessageCreate(s32TcRetMsgId, OS_ATTR_FIFO, sizeof(ST_TC_MSG) * 128);

    if (TaskCreate(TOUCH_CONTROLLRT_TASK, TouchCtrllerTask, DRIVER_PRIORITY, 0x2000) == OS_STATUS_OK)
        TaskStartup(TOUCH_CONTROLLRT_TASK);
    else
    {
        MP_ALERT("--E-- TOUCH_CONTROLLRT_TASK create fail !!!\n\r");
        //__asm("break 100");
    }

}




/****************************************************************************
 **
 ** NAME:           TcTimerIsr
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Isr of Tc timer.
 **                 It send a msg to wake TS task up and polling the TS driver.
 **
 ****************************************************************************/
#if TC_POLLING_ENABLE
static void TcTimerIsr(void)
{
    ST_TC_MSG tcMessage;
    tcMessage.status = TC_POLLING;
    MessageDrop(s32TcMsgId, (BYTE *) & tcMessage, 1);
}
#endif


/****************************************************************************
 **
 ** NAME:           TcStrokeTimerIsr
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Isr of Tc Stroke timer.
 **                 Timeout if user do not generate stroke any more.
 **
 ****************************************************************************/
static void TcStrokeTimerIsr(void)
{
    ST_TC_DATA pos_data;

    pos_data.status = TC_STROKE_STOP;
    pos_data.x1 = 0;
    pos_data.y1 = 0;
    pos_data.x2 = 0;
    pos_data.y2 = 0;

    if(TcGetDataCallBack)
        TcGetDataCallBack(pos_data.status, pos_data.x1, pos_data.y1, pos_data.x2, pos_data.y2);
    else if ( !boolMsgRetStop )
	{
        EventSet(UI_EVENT, EVENT_TOUCH_COLTROLLER);
        MessageDrop(UI_TC_MSG_ID, (BYTE *) & pos_data, sizeof(pos_data));
	}
}




/****************************************************************************
 **
 ** NAME:           TcResume
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    resume.
 **
 ****************************************************************************/
static SDWORD TcResume(void)
{
    return stTcDriver.TcSleep(FALSE);
}



/****************************************************************************
 **
 ** NAME:           TcPosReset
 **
 ** PARAMETERS:     width and height of panel
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Reset all variable about pen adjustment.
 **
 ****************************************************************************/
static void TcPosReset(S16 x_off, S16 y_off, U16 x_max, U16 y_max, U16 x_min, U16 y_min)
{
}



/****************************************************************************
 **
 ** NAME:           TcPosGet
 **
 ** PARAMETERS:
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    .
 **
 ****************************************************************************/
static void TcPosGet(ST_TC_PARA* tcpara)
{
}



/****************************************************************************
 **
 ** NAME:           TcPause
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    pause.
 **
 ****************************************************************************/
static SDWORD TcPause(void)
{
    return stTcDriver.TcSleep(TRUE);
}




/****************************************************************************
 **
 ** NAME:           TcTaskClose
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    close.
 **
 ****************************************************************************/
static SDWORD TcTaskClose(void)
{
    stTcDriver.TcSleep(TRUE);

    return 0;
}



/****************************************************************************
 **
 ** NAME:           TcChgSensitivity
 **
 ** PARAMETERS:     x_sen : Sensitivity of X-axix .
 **                 y_sen : Sensitivity of Y-axix .
 **                 z_sen : Sensitivity of Z-axix .
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Change sensitivity of touch panel
 **
 ****************************************************************************/
static SDWORD TcChgSensitivity(DWORD x_sen, DWORD y_sen, DWORD z_sen)
{
    return stTcDriver.TcChangeSensitivity(x_sen, y_sen, z_sen);
}


/****************************************************************************
 **
 ** NAME:           TcFuncConstruct
 **
 ** PARAMETERS:     TcFunc
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    constructor of TS task.
 **
 ****************************************************************************/
void TcFuncConstruct(ST_TC_FUNC * TcFunc)
{
    s32TcMsgId = TOUCH_CONTROLLER_MSG_ID;
    s32TcRetMsgId = UI_TC_CAL_MSG_ID;

    cross_width = DEFAULT_CROSS_WIDTH;
    TcFunc->pCross_width = &cross_width;
    cross_single_width = DEFAULT_CROSS_SINGLE_WIDTH;
    TcFunc->pCross_single_width = &cross_single_width;
    u32PollTimerPeriod = 1;
    TcFunc->pPoll_timer = &u32PollTimerPeriod;
    u32HwrTimer = TC_HWR_TIMER_PERIOD_DEFAULT;
    TcFunc->pHwr_timer = &u32HwrTimer;
    TcFunc->FuncInit = TouchCtrllerTaskInit;
    TcFunc->FuncPosReset = TcPosReset;
    TcFunc->FuncPosGet = TcPosGet;
    TcFunc->FuncGetTaskID = TouchCtrllerTask;
    //TcFunc->FuncGetTimerID = NULL;//TcGetTimerID;
    TcFunc->FuncGetMsgId = TcGetMsgId;
    TcFunc->FuncGetRetMsgId = TcGetRetMsgId;
    TcFunc->FuncGetDataCallBackReg = TcGetDataCallBackReg;
    TcFunc->FuncGetDataCallBackDeReg = TcGetDataCallBackDeReg;
    TcFunc->FuncClose = TcTaskClose;
    TcFunc->FuncPause = TcPause;
    TcFunc->FuncResume = TcResume;
    TcFunc->FuncChgSensitivity = TcChgSensitivity;
    TcGetDataCallBack = NULL;
}


#endif  //#if (TOUCH_CONTROLLER_ENABLE == ENABLE)


