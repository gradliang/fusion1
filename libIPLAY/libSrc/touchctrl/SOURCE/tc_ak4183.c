/*
*******************************************************************************
*                           Magic Pixel Inc.                                  *
*                  Copyright (c) 2007 -, All Rights Reserved                  *
*                                                                             *
* TS module driver.                                                           *
*                                                                             *
* File : Tc_Ak4183.c                                                          *
* By :                                                                        *
*                                                                             *
*                                                                             *
* Description : AKM AK4183 touch screen controller driver                     *
*                                                                             *
* History :                                                                   *
*                                                                             *
*******************************************************************************
*/

/*************************************************************************/
/***                        Include Files                              ***/
/*************************************************************************/

#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"

#if ((TOUCH_CONTROLLER_ENABLE == ENABLE) && (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_AK4183))
#include "mpTrace.h"
#include "os.h"
#include "taskid.h"
#include "peripheral.h"
#include "tc_driver.h"
/*************************************************************************/
/***               Public Variable Declaration                         ***/
/*************************************************************************/

/*************************************************************************/
/***               Internally Visible Constants and Static Data        ***/
/*************************************************************************/
/* ak4183 command define */
#define AK4183_I2C_ADDRESS      0x90//0x48
#define AK4183_I2C_CMD_INIT     0xC0
#define AK4183_I2C_CMD_SLEEP    0x72
#define AK4183_I2C_CMD_X        0x80
#define AK4183_I2C_CMD_Y        0x90
#define AK4183_I2C_CMD_Z1       0xa0
#define AK4183_I2C_CMD_Z2       0xb0

#define AK4183_I2C_BIT_A2       0x40
#define AK4183_I2C_BIT_PD0      0x04

#define INT_LOW_LEVEL           0
#define INT_HIGH_LEVEL          1

//#define TC_INT_USING_EDGE_TRIGGER
#ifdef TC_INT_USING_EDGE_TRIGGER
#define TC_TRIGGER_MODE     GPIO_EDGE_TRIGGER
#else
#define TC_TRIGGER_MODE     GPIO_LEVEL_TRIGGER
#endif


static BYTE u08IntModeFlag;

#define SAMPLE_ARRAY_SIZE 5
typedef struct
{
    WORD x;
    WORD y;
} _SampleAry;

typedef struct
{
    _SampleAry SampleAry[SAMPLE_ARRAY_SIZE];
    BYTE index;
    BYTE num;
} _SampleBucket;

static _SampleBucket SampleBucket;

static DWORD gpioNum, gpioIntNum;

static void TcIsr(void);
static SDWORD Ak4183_change_sensitivity(DWORD, DWORD, DWORD);



#ifndef FOR_TOUCH_CONTROLLRT_TASK
void read_data_test(void)
{
    Tc_point data;

    Ak4183_get_point(&data);

    mpDebugPrintN("x = %d, y = %d\r\n", data.x1, data.y1);
}
#endif



/****************************************************************************
 **
 ** NAME:           TcDebounceTimerIsr
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  return msg ID
 **
 ** DESCRIPTION:    Isr of Tc Debounce timer.
 **
 ****************************************************************************/
static void TcDebounceTimerIsr(WORD noUse)
{
    ST_TC_MSG tcMessage;

    if(u08IntModeFlag == INT_LOW_LEVEL)
    {
        Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_HIGH, TC_TRIGGER_MODE);
        tcMessage.status = TC_DOWN;
        u08IntModeFlag = INT_HIGH_LEVEL;
        SampleBucket.num = 0;//Clear number to zero
        SampleBucket.index = 0;
    }
    else
    {
        Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
        tcMessage.status = TC_UP;
        u08IntModeFlag = INT_LOW_LEVEL;
    }

    #define FOR_TOUCH_CONTROLLRT_TASK
    #ifdef FOR_TOUCH_CONTROLLRT_TASK
        Gpio_IntEnable(gpioIntNum);
        MessageDrop(TcGetMsgId(), (BYTE *) & tcMessage, 1);
    #else
        read_data_test();
        Gpio_IntEnable(gpioIntNum);
    #endif
}



/****************************************************************************
 **
 ** NAME:           Ak4183_get_value
 **
 ** PARAMETERS:     val[]:the value from chip, cnt:value count
 **
 ** RETURN VALUES:  The result of sample
 **
 ** DESCRIPTION:    get cnt samples average value, and do not consider max and min
 **
 ****************************************************************************/
static WORD Ak4183_get_value(WORD val[], WORD cnt)
{
    DWORD i, ret = 0, max = 0, min = 0xffffffff;

    if ( cnt <= 3 )
    {
        for(i = 0; i < cnt; i++)
            ret += val[i];

        ret /= cnt;
    }
    else
    {
        for(i = 0; i < cnt; i++)
        {
            ret += val[i];
            if(val[i] > max)
                max = val[i];
            if(val[i] < min)
                min = val[i];
        }
        ret = (ret - (max + min)) / (cnt - 2);
    }

    return ret;
}

/****************************************************************************
 **
 ** NAME:           TcIsr
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    The interrupt service routine of Touch screen controller.
 **                 Set low level trigger when pen not be touched.
 **                 Set high level trigger when pen be touched.
 **
 ****************************************************************************/
static void TcIsr(void)
{
    Gpio_IntDisable(gpioIntNum);
    SysTimerProcAdd(4, TcDebounceTimerIsr, TRUE);
}

/****************************************************************************
 **
 ** NAME:           Ak4183_Sleep
 **
 ** PARAMETERS:     sleep: TRUE or FALSE
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Enter power saving mode of AK4183
 **
 ****************************************************************************/
static SDWORD Ak4183_Sleep(BYTE sleep)
{
    BYTE cmd;
    if(sleep)
    {
        cmd = AK4183_I2C_CMD_SLEEP;
        I2CM_Write_BustMode(AK4183_I2C_ADDRESS, &cmd, 1);
        Gpio_IntDisable(gpioIntNum);
    }
    else
    {
        cmd = AK4183_I2C_CMD_INIT;
        I2CM_Write_BustMode(AK4183_I2C_ADDRESS, &cmd, 1);
        Gpio_IntEnable(gpioIntNum);
    }

    return 0;
}

/****************************************************************************
 **
 ** NAME:           Ak4183_Check_Intterupt
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Check interrupt pin of Ak4183
 **
 ****************************************************************************/
static SDWORD Ak4183_Check_Interrupt(void)
{
    WORD       data;

    Gpio_DataGet(gpioNum, &data, 1);
    if(data)
    {
        return 1;       // no touch
    }
    else
    {
        return 0;       // touch
    }
}

/****************************************************************************
 **
 ** NAME:           Ak4183_get_point
 **
 ** PARAMETERS:     data: the sapce of data to return
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    The procedure of getting data from Ak4183.
 **
 ****************************************************************************/
static SDWORD Ak4183_get_point(Tc_point * data)
{
    BYTE i;
    WORD val[SAMPLE_ARRAY_SIZE];
    WORD x, y, z1, z2;

    if ( SampleBucket.num < SAMPLE_ARRAY_SIZE )
    {
        if (!Ak4183_Check_Interrupt())
        {
            Gpio_IntDisable(gpioIntNum);        //avoid interrupt when getting data
            do
            {
                x = (WORD)I2CM_RdReg8Data16(AK4183_I2C_ADDRESS, AK4183_I2C_CMD_X | AK4183_I2C_BIT_A2) >> 4;
                y = (WORD)I2CM_RdReg8Data16(AK4183_I2C_ADDRESS, AK4183_I2C_CMD_Y | AK4183_I2C_BIT_A2) >> 4;
                SampleBucket.SampleAry[SampleBucket.index].x = x;
                SampleBucket.SampleAry[SampleBucket.index].y = y;
                SampleBucket.index++;
                if (SampleBucket.index >= SAMPLE_ARRAY_SIZE)
                    SampleBucket.index = 0;
            } while(++SampleBucket.num < SAMPLE_ARRAY_SIZE);

            if (Ak4183_Check_Interrupt())
            {
                Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
                u08IntModeFlag = INT_LOW_LEVEL;
                SampleBucket.num = 0;//Clear number to zero
                SampleBucket.index = 0;
                Gpio_IntEnable(gpioIntNum);

                return 1;
            }
        }
        else
            return 1;
    }
    else
    {
        Gpio_IntDisable(gpioIntNum);        //avoid interrupt when getting data
        x = (WORD)I2CM_RdReg8Data16(AK4183_I2C_ADDRESS, AK4183_I2C_CMD_X | AK4183_I2C_BIT_A2) >> 4;
        y = (WORD)I2CM_RdReg8Data16(AK4183_I2C_ADDRESS, AK4183_I2C_CMD_Y | AK4183_I2C_BIT_A2) >> 4;
        SampleBucket.SampleAry[SampleBucket.index].x = x;
        SampleBucket.SampleAry[SampleBucket.index].y = y;
        SampleBucket.index++;
        if ( SampleBucket.index >= SAMPLE_ARRAY_SIZE )
            SampleBucket.index = 0;
    }
    for(i = 0; i < SampleBucket.num; i++)
        val[i] = SampleBucket.SampleAry[i].x;
    data->x1 = Ak4183_get_value(val, SampleBucket.num);

    for(i = 0; i < SampleBucket.num; i++)
        val[i] = SampleBucket.SampleAry[i].y;
    data->y1 = Ak4183_get_value(val, SampleBucket.num);
    Gpio_IntEnable(gpioIntNum);

    return 0;
}



/****************************************************************************
 **
 ** NAME:           Ak4183_change_sensitivity
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
static SDWORD Ak4183_change_sensitivity(DWORD x_sen, DWORD y_sen, DWORD z_sen)
{
    return 0;
}



/****************************************************************************
 **
 ** NAME:           Ak4183_Init
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Init Ak4183. and Setting the mode of interrupt pin.
 **
 ****************************************************************************/
static SDWORD Ak4183_Init(void)
{
    MP_DEBUG("__%s__", __FUNCTION__);

    SDWORD       status;

    u08IntModeFlag = INT_LOW_LEVEL;

    gpioNum = GetGPIOPinIndex(TOUCH_CONTROLLER_INT_PIN);


    BYTE cmd = AK4183_I2C_CMD_INIT;
    I2CM_Write_BustMode(AK4183_I2C_ADDRESS, &cmd, 1);

    Gpio_Config2GpioFunc(gpioNum, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);
    gpioIntNum = GetGpioIntIndexByGpioPinNum(gpioNum);
    Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
    Gpio_IntCallbackFunRegister(gpioIntNum, TcDebounceTimerIsr);

    Gpio_IntEnable(gpioIntNum);
//    SystemIntEna(IM_GPIO);

    return 0;
}



/****************************************************************************
 **
 ** NAME:           TcDriverInit
 **
 ** PARAMETERS:     stTcDriver
 **
 ** RETURN VALUES:  result
 **
 ** DESCRIPTION:    constructor of TS driver.
 **
 ****************************************************************************/
SDWORD TcDriverInit(ST_TC_DRIVER * stTcDriver)
{
    stTcDriver->u08Name = "AKM AK4183 Touch screen device driver";
    stTcDriver->TcInit = Ak4183_Init;
    stTcDriver->TcSleep = Ak4183_Sleep;
    stTcDriver->TcGetData = Ak4183_get_point;
    stTcDriver->TcCheckInterrupt = Ak4183_Check_Interrupt;
    stTcDriver->TcChangeSensitivity = Ak4183_change_sensitivity;


    return 0;
}


#endif //#if (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_AK4183)


