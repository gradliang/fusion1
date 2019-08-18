/*
*******************************************************************************
*                           Magic Pixel Inc.                                  *
*                  Copyright (c) 2007 -, All Rights Reserved                  *
*                                                                             *
* TS module driver.                                                           *
*                                                                             *
* File : Tc_a043vl01.c                                                        *
* By :                                                                        *
*                                                                             *
*                                                                             *
* Description : AUO A043VL01 touch screen controller driver                   *
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

#if ((TOUCH_CONTROLLER_ENABLE == ENABLE) && (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_A043VL01))
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
/* A043VL01 command define */
#define A043VL01_I2C_ADDRESS        (0x5C << 1)
#define A043VL01_I2C_CMD_POWER_MODE 0x70
#define A043VL01_I2C_CMD_X1_LSB     0x40
#define A043VL01_I2C_CMD_X1_MSB     0x44
#define A043VL01_I2C_CMD_X2_LSB     0x42
#define A043VL01_I2C_CMD_X2_MSB     0x46
#define A043VL01_I2C_CMD_Y1_LSB     0x41
#define A043VL01_I2C_CMD_Y1_MSB     0x45
#define A043VL01_I2C_CMD_Y2_LSB     0x43
#define A043VL01_I2C_CMD_Y2_MSB     0x47

#define A043VL01_I2C_CMD_X_SEN      0x67
#define A043VL01_I2C_CMD_Y_SEN      0x68

#define A043VL01_I2C_CMD_INT_SETTING        0x6E
#define A043VL01_I2C_CMD_INT_WIDTH          0x6F

#define A043VL01_INT_MODE_ASSERT_PERI       0           // INT assert periodically
#define A043VL01_INT_MODE_ASSERT_POS_CHG    0x01        // INT assert only when coordinate difference
#define A043VL01_INT_MODE_TOUCH_INDICATE    0x02        // Touch indicate
#define A043VL01_INT_POL_LOW_ACTIVE         0
#define A043VL01_INT_POL_HIGH_ACTIVE        1
#define A043VL01_EN_INT                     BIT3        // 0 : Disable interrupt mechanism.  1 : Enable interrupt mechanism

#define INT_LOW_LEVEL               0
#define INT_HIGH_LEVEL              1

//#define TC_INT_USING_EDGE_TRIGGER
#ifdef TC_INT_USING_EDGE_TRIGGER
#define TC_TRIGGER_MODE     GPIO_EDGE_TRIGGER
#else
#define TC_TRIGGER_MODE     GPIO_LEVEL_TRIGGER
#endif

static BYTE u08IntModeFlag;
static DWORD gpioNum, gpioIntNum;


#define SAMPLE_ARRAY_SIZE 5
typedef struct
{
    WORD x1;
    WORD y1;
    WORD x2;
    WORD y2;
} _SampleAry;

typedef struct
{
    _SampleAry SampleAry[SAMPLE_ARRAY_SIZE];
    BYTE index;
    BYTE num;
} _SampleBucket;

static _SampleBucket SampleBucket;

static void TcIsr(void);
static SDWORD A043VL01_get_point(Tc_point*);
static SDWORD A043VL01_change_sensitivity(DWORD, DWORD, DWORD);



#ifndef FOR_TOUCH_CONTROLLRT_TASK
void read_data_test(void)
{
    Tc_point data;

    A043VL01_get_point(&data);

    mpDebugPrintN("x1 = %d, y1 = %d, x2 = %d, y2 = %d\r\n", data.x1, data.y1, data.x2, data.y2);
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
 ** NAME:           A043VL01_get_value
 **
 ** PARAMETERS:     val[]:the value from chip, cnt:value count
 **
 ** RETURN VALUES:  The result of sample
 **
 ** DESCRIPTION:    get cnt samples average value, and do not consider max and min
 **
 ****************************************************************************/
static WORD A043VL01_get_value(WORD val[], WORD cnt)
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
 ** NAME:           A043VL01_Sleep
 **
 ** PARAMETERS:     sleep: TRUE or FALSE
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Enter power saving mode of A043VL01
 **
 ****************************************************************************/
static SDWORD A043VL01_Sleep(BYTE sleep)
{
    if(sleep)
    {
        //I2CM_WtReg8Data8(A043VL01_I2C_ADDRESS, A043VL01_I2C_CMD_POWER_MODE, UN_KNOWN);
        Gpio_IntDisable(gpioIntNum);
    }
    else
    {
        //I2CM_WtReg8Data8(A043VL01_I2C_ADDRESS, A043VL01_I2C_CMD_POWER_MODE, UN_KNOWN);
        Gpio_IntEnable(gpioIntNum);
    }

    return 0;
}



/****************************************************************************
 **
 ** NAME:           A043VL01_Check_Intterupt
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Check interrupt pin of A043VL01
 **
 ****************************************************************************/
static SDWORD A043VL01_Check_Interrupt(void)
{
    WORD data;

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
 ** NAME:           A043VL01_get_point
 **
 ** PARAMETERS:     data: the sapce of data to return
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    The procedure of getting data from A043VL01.
 **
 ****************************************************************************/
static SDWORD A043VL01_get_point(Tc_point * data)
{
    BYTE i;
    WORD val[SAMPLE_ARRAY_SIZE];
    BYTE dataBuf[8];
    //WORD x1, x2, y1, y2;

    if ( SampleBucket.num < SAMPLE_ARRAY_SIZE )
    {
        if (!A043VL01_Check_Interrupt())
        {
            Gpio_IntDisable(gpioIntNum);        //avoid interrupt when getting data
            do
            {
                if(I2CM_Read_BustMode(A043VL01_I2C_ADDRESS, A043VL01_I2C_CMD_X1_LSB, 0, dataBuf, 8) != PASS)
                {
                    MP_ALERT("I2CM_Read_BustMode error!!!");
                    continue;
                }

                SampleBucket.SampleAry[SampleBucket.index].x1 = ((WORD)dataBuf[4] << 8) | dataBuf[0];
                SampleBucket.SampleAry[SampleBucket.index].y1 = ((WORD)dataBuf[5] << 8) | dataBuf[1];
                SampleBucket.SampleAry[SampleBucket.index].x2 = ((WORD)dataBuf[6] << 8) | dataBuf[2];
                SampleBucket.SampleAry[SampleBucket.index].y2 = ((WORD)dataBuf[7] << 8) | dataBuf[3];
                SampleBucket.index++;
                if (SampleBucket.index >= SAMPLE_ARRAY_SIZE)
                    SampleBucket.index = 0;
            } while(++SampleBucket.num < SAMPLE_ARRAY_SIZE);

            if (A043VL01_Check_Interrupt())
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
        if(I2CM_Read_BustMode(A043VL01_I2C_ADDRESS, A043VL01_I2C_CMD_X1_LSB, 0, dataBuf, 8) != PASS)
        {
            MP_ALERT("I2CM_Read_BustMode error!!!");
            return FAIL;
        }

        SampleBucket.SampleAry[SampleBucket.index].x1 = ((WORD)dataBuf[4] << 8) | dataBuf[0];
        SampleBucket.SampleAry[SampleBucket.index].y1 = ((WORD)dataBuf[5] << 8) | dataBuf[1];
        SampleBucket.SampleAry[SampleBucket.index].x2 = ((WORD)dataBuf[6] << 8) | dataBuf[2];
        SampleBucket.SampleAry[SampleBucket.index].y2 = ((WORD)dataBuf[7] << 8) | dataBuf[3];
        SampleBucket.index++;
        if ( SampleBucket.index >= SAMPLE_ARRAY_SIZE )
            SampleBucket.index = 0;
    }
    for(i = 0; i < SampleBucket.num; i++)
        val[i] = SampleBucket.SampleAry[i].x1;
    data->x1 = A043VL01_get_value(val, SampleBucket.num);

    for(i = 0; i < SampleBucket.num; i++)
        val[i] = SampleBucket.SampleAry[i].y1;
    data->y1 = A043VL01_get_value(val, SampleBucket.num);

    for(i = 0; i < SampleBucket.num; i++)
        val[i] = SampleBucket.SampleAry[i].x2;
    data->x2 = A043VL01_get_value(val, SampleBucket.num);

    for(i = 0; i < SampleBucket.num; i++)
        val[i] = SampleBucket.SampleAry[i].y2;
    data->y2 = A043VL01_get_value(val, SampleBucket.num);

#if (LOCAL_DEBUG_ENABLE == ENABLE)
    MP_ALERT("%d,%d,%d,%d", data->x1, data->y1, data->x2, data->y2);
#endif
    Gpio_IntEnable(gpioIntNum);

    return 0;
}



/****************************************************************************
 **
 ** NAME:           A043VL01_Init
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Init A043VL01. and Setting the mode of interrupt pin.
 **
 ****************************************************************************/
static SDWORD A043VL01_Init(void)
{
    MP_DEBUG("__%s__", __FUNCTION__);

    SDWORD status;

    u08IntModeFlag = INT_LOW_LEVEL;

    gpioNum = GetGPIOPinIndex(TOUCH_CONTROLLER_INT_PIN);

    I2CM_FreqChg(A043VL01_I2C_ADDRESS, 300000);         // measured clock is 270KHz

    A043VL01_change_sensitivity(255, 255, 0);      // most sensitive.
    I2CM_WtReg8Data8(A043VL01_I2C_ADDRESS, A043VL01_I2C_CMD_INT_SETTING,
                     (A043VL01_EN_INT | A043VL01_INT_POL_LOW_ACTIVE | A043VL01_INT_MODE_TOUCH_INDICATE));

    I2CM_WtReg8Data8(A043VL01_I2C_ADDRESS, A043VL01_I2C_CMD_INT_WIDTH, 0x01);

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
 ** NAME:           A043VL01_change_sensitivity
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
static SDWORD A043VL01_change_sensitivity(DWORD x_sen, DWORD y_sen, DWORD z_sen)
{
    WORD buf;
    x_sen = (x_sen > 255) ? 255 : x_sen;
    y_sen = (y_sen > 255) ? 255 : y_sen;
    buf = (WORD)x_sen << 8 | y_sen;

    if(I2CM_WtReg8Data16(A043VL01_I2C_ADDRESS, A043VL01_I2C_CMD_X_SEN, buf) != PASS)
        return FAIL;

    buf = (WORD)I2CM_RdReg8Data16(A043VL01_I2C_ADDRESS, A043VL01_I2C_CMD_X_SEN);

    MP_ALERT("buf = 0x%x", buf);
    return PASS;

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
    stTcDriver->u08Name = "AUO A043VL01 Touch screen device driver";
    stTcDriver->TcInit = A043VL01_Init;
    stTcDriver->TcSleep = A043VL01_Sleep;
    stTcDriver->TcGetData = A043VL01_get_point;
    stTcDriver->TcCheckInterrupt = A043VL01_Check_Interrupt;
    stTcDriver->TcChangeSensitivity = A043VL01_change_sensitivity;

    return 0;
}


#endif //#if (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_A043VL01)


