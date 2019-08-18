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
* Filename      : IICM.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : Master I2C module
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE      0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "os.h"
#include "peripheral.h"

#if (HW_IIC_MASTER_ENABLE == 1)

#define IIC_CLOCK_SOURCE_USING_PLL2

#define I2C_DELAY_LENGTH            20
#define I2C_WAIT_DONE_TIMEOUT       20          // ms
#define I2C_WAIT_NEXT_TIME          1000          // ms
#define I2C_WAIT_BUSY_TIME          100         // ms

// For I2C_MReg62 (I2C_MReg3)
#define I2C_WAIT_NEXT               BIT0
#define I2C_DONE                    BIT1
#define I2C_SUCCESS                 BIT2
#define I2C_16BIT_MODE              BIT3
#define I2C_NON_ACK                 BIT4
#define I2C_NO_RSTART               BIT5
#define I2C_LOOP_MODE               BIT6        // Bust mode
#define I2C_WAIT_NEXT_EN            BIT16
#define I2C_DONE_INT_EN             BIT17
#define I2C_SCL_FROM_EXT            BIT31

static BOOL i2cSemaphoreCreated = FALSE;
static DWORD i2cDefaultTiming = 0x32323203;
static volatile BOOL i2cmUsed = FALSE;
static ST_IICM_TIMING_TABLE i2cmTimingTable[MAX_IICM_DEV] = {0};

////////////////////////////////////////////////////////
//
// Local function
//
////////////////////////////////////////////////////////
static i2cmDelayWait(void)
{
    volatile DWORD i;

    for (i = 0; i < 800; i++);
}



static SDWORD i2cm_TimingTableSearch(BYTE devId)
{
    DWORD tmp;

    if (devId == 0)
        return i2cDefaultTiming;

    for (tmp = 0; tmp < MAX_IICM_DEV; tmp++)
    {
        if (devId == i2cmTimingTable[tmp].devId)
        {
            MP_DEBUG("0x%08X", i2cmTimingTable[tmp].timing);

            return i2cmTimingTable[tmp].timing;
        }
    }

    MP_DEBUG("--W-- Timing Table was not found, using default I2C timing !!!! - 0x%08X", i2cDefaultTiming);

    return i2cDefaultTiming;
}



static SDWORD i2cm_TimingTableAdd(BYTE devId, DWORD newTiming)
{
    DWORD tmp;

    if (devId == 0)
    {
        i2cDefaultTiming = (newTiming & 0xFFFFFF00) | 0x03;
        MP_DEBUG("Change I2C's default timing to 0x%08X", newTiming);

        return PASS;
    }

    for (tmp = 0; tmp < MAX_IICM_DEV; tmp++)
    {
        if (devId == (BYTE) i2cmTimingTable[tmp].devId)
        {
            i2cmTimingTable[tmp].timing = newTiming;
            MP_ALERT("Change I2C-0x%2X's timing to 0x%08X", devId, newTiming);

            return PASS;
        }
    }

    for (tmp = 0; tmp < MAX_IICM_DEV; tmp++)
    {
        if (i2cmTimingTable[tmp].devId == -1)
        {
            i2cmTimingTable[tmp].devId = devId;
            i2cmTimingTable[tmp].timing = newTiming;
            MP_ALERT("Add I2C-0x%2X's timing to 0x%08X, index = %2d", devId, newTiming, tmp);

            return PASS;
        }
    }

    MP_ALERT("--E-- I2CM Timing Table was full !!!!");

    return FAIL;
}



static SWORD i2cm_resource_lock(BOOL *semaFlag)
{
    IntDisable();

    if (ContextStatusGet())
    {   // in Task
        SemaphoreWait(I2CM_SEMA_ID);
        IntDisable();
        *semaFlag = TRUE;
        i2cmUsed = TRUE;
        IntEnable();
    }
    else
    {   // in ISR
        if (i2cmUsed)
            return FAIL;

        *semaFlag = FALSE;
        i2cmUsed = TRUE;
    }

    return PASS;
}



static void i2cm_resource_unlock(BOOL semaLocked)
{
    IntDisable();
    i2cmUsed = FALSE;

    if (semaLocked)
        SemaphoreRelease(I2CM_SEMA_ID);
    else
        IntEnable();
}



static SDWORD i2cm_WaitAck(void)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    DWORD tick, tpc, tmv;
    DWORD timeOut;

    tick = GetSysTime();
    timeOut = 0;

    while (((regI2cPtr->I2C_MReg62 & I2C_DONE) != I2C_DONE) && (timeOut < I2C_WAIT_DONE_TIMEOUT))
    {
        timeOut = SystemGetElapsedTime(tick);
    }

    if ((regI2cPtr->I2C_MReg62 & I2C_SUCCESS) == I2C_SUCCESS)
        return PASS;

    if (timeOut >= I2C_WAIT_DONE_TIMEOUT)
        MP_ALERT("--E-- i2cm_WaitAck timeout !!!");
    else
        MP_ALERT("--E-- i2cm_WaitAck no-ack !!!");

    return FAIL;
}



static SDWORD i2cm_WaitNext(void)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    DWORD tick, tpc, tmv;
    DWORD timeOut;

    tick = GetSysTime();
    timeOut = 0;

    while ( ((regI2cPtr->I2C_MReg62 & (I2C_DONE | I2C_WAIT_NEXT)) == 0) &&
            (timeOut < I2C_WAIT_NEXT_TIME) )
    {
        timeOut = SystemGetElapsedTime(tick);
    }

    if ((regI2cPtr->I2C_MReg62 & (I2C_DONE | I2C_WAIT_NEXT)) == I2C_WAIT_NEXT)
        return PASS;

    // Error
    if ((regI2cPtr->I2C_MReg62 & (I2C_DONE | I2C_WAIT_NEXT)) == I2C_DONE)
         MP_ALERT("--E-- i2cm_WaitNext No-Ack !!!");
    else if (timeOut >= I2C_WAIT_NEXT_TIME)
        MP_ALERT("--E-- i2cm_WaitNext timeout !!!");
    else
        MP_ALERT("--E-- i2cm_WaitNext Unknow case !!!");

    return FAIL;
}



static void i2cm_GpioCfg(BOOL i2cEnable)
{
    GPIO *regGpioPtr = (GPIO *) GPIO_BASE;

    if (i2cEnable == ENABLE)
    {
        // Configuration to I2C function
#if (CHIP_VER_MSB == CHIP_VER_615)
        regGpioPtr->Gpdat0 &= ~0x00300000;      // Input mode
        regGpioPtr->Gpcfg1 |= 0x00300030;
        //Gpio_ConfiguraionSet(GPIO_GPIO_20, GPIO_ALT_FUNC_3, GPIO_OUTPUT_MODE, 0x03, 2);
#else   // MP65x/MP66x
        #if (IICM_PIN_USING_PGPIO == ENABLE)
        regGpioPtr->Pgpdat &= ~0x00030000;      // Input mode, PGPIO[0:1]
        regGpioPtr->Pgpcfg = (regGpioPtr->Pgpcfg & ~0x00030003) | 0x00030000;
        //Gpio_ConfiguraionSet(GPIO_PGPIO_0, GPIO_ALT_FUNC_2, GPIO_OUTPUT_MODE, 0x03, 2);
        #elif (IICM_PIN_USING_VGPIO == ENABLE)
        regGpioPtr->Vgpdat1 &= ~0x01800000;      // Input mode, VGPIO[23:24]
        regGpioPtr->Vgpcfg1 |= 0x01800180;
        //Gpio_ConfiguraionSet(GPIO_VGPIO_23, GPIO_ALT_FUNC_3, GPIO_OUTPUT_MODE, 0x03, 2);
        #else
        regGpioPtr->Gpdat0 &= ~0x00030000;      // Input mode, GPIO[0:1]
        regGpioPtr->Gpcfg0 |= 0x00030003;
        //Gpio_ConfiguraionSet(GPIO_GPIO_0, GPIO_ALT_FUNC_3, GPIO_OUTPUT_MODE, 0x03, 2);
        #endif
#endif
    }
    else
    {
        // Configuration to GPIO input mode
#if (CHIP_VER_MSB == CHIP_VER_615)
        regGpioPtr->Gpdat1 &= ~0x00300000;      // Input mode
        regGpioPtr->Gpcfg1 &= ~0x00300030;
        //Gpio_Config2GpioFunc(GPIO_GPIO_20, GPIO_INPUT_MODE, 0x03, 2);
#else   // MP65x/MP66x
        #if (IICM_PIN_USING_PGPIO == ENABLE)
        regGpioPtr->Pgpdat &= ~0x00030000;      // Input mode, PGPIO[0:1]
        regGpioPtr->Pgpcfg = (regGpioPtr->Pgpcfg & ~0x00030003) | 0x00000003;
        //Gpio_Config2GpioFunc(GPIO_PGPIO_0, GPIO_INPUT_MODE, 0x03, 2);
        #elif (IICM_PIN_USING_VGPIO == ENABLE)
        regGpioPtr->Vgpdat1 &= ~0x01800000;      // Input mode, VGPIO[23:24]
        regGpioPtr->Vgpcfg1 &= ~0x01800180;
        //Gpio_Config2GpioFunc(GPIO_VGPIO_23, GPIO_INPUT_MODE, 0x03, 2);
        #else
        regGpioPtr->Gpdat0 &= ~0x00030000;      // Input mode, GPIO[0:1]
        regGpioPtr->Gpcfg0 &= ~0x00030003;
        //Gpio_Config2GpioFunc(GPIO_GPIO_0, GPIO_INPUT_MODE, 0x03, 2);
        #endif
#endif
    }
}



static BOOL i2cm_BusyWiat(void)
{
    WORD inData = GPIO_DATA_LOW;
    DWORD startTime = GetSysTime();

	IODelay(20);
    return;

    while ((SystemGetElapsedTime(startTime) < I2C_WAIT_BUSY_TIME) && (inData == GPIO_DATA_LOW))
    {
#if (CHIP_VER_MSB == CHIP_VER_615)
        Gpio_DataGet(GPIO_GPIO_20, &inData, 1);
#else   // MP65x/MP66x
        #if (IICM_PIN_USING_PGPIO == ENABLE)
        Gpio_DataGet(GPIO_PGPIO_0, &inData, 1);
        #elif (IICM_PIN_USING_VGPIO == ENABLE)
        Gpio_DataGet(GPIO_VGPIO_23, &inData, 1);
        #else
        Gpio_DataGet(GPIO_GPIO_0, &inData, 1);
        #endif
#endif
    }

    if (inData == GPIO_DATA_LOW)
    {
        mpDebugPrintChar('B');

        return FAIL;
    }

    return PASS;
}


static void i2cm_PowerCtrl(BOOL i2cEnable)
{
    BIU *regBiuPtr = (BIU *) BIU_BASE;
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;

    IntDisable();

    if (i2cEnable == ENABLE)
    {
        //reset IIC Master module
        regClockPtr->MdClken |= CKE_I2CM;
        i2cmDelayWait();
        regBiuPtr->BiuArst |= ARST_I2CM;
        i2cmDelayWait();
        i2cm_GpioCfg(ENABLE);
    }
    else
    {
        if ((regClockPtr->MdClken & CKE_I2CS) == 0)
        {   // IICS is not actived
            i2cm_GpioCfg(DISABLE);
            //reset IIC Master module
            regBiuPtr->BiuArst &= ~ARST_I2CM;
            i2cmDelayWait();
            regClockPtr->MdClken &= ~CKE_I2CM;
            i2cmDelayWait();
        }
    }

    IntEnable();
}                               //end of I2CM_Init



////////////////////////////////////////////////////////
//
// Global function
//
////////////////////////////////////////////////////////
void I2CM_Init(void)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DWORD tmp;

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    #if (IICM_PIN_USING_PGPIO == ENABLE)
    regClockPtr->PinMuxCfg = (regClockPtr->PinMuxCfg & ~(BIT3 | BIT4)) | BIT3;
    #elif (IICM_PIN_USING_VGPIO == ENABLE)
    regClockPtr->PduEn2 &= ~(BIT26 | BIT27);        // It's pull-low, disable it
    regClockPtr->PinMuxCfg = (regClockPtr->PinMuxCfg & ~(BIT3 | BIT4)) | BIT4;
    #else
    //regClockPtr->PduEn3 |= BIT0 | BIT1;
    regClockPtr->PinMuxCfg &= ~(BIT3 | BIT4);
    #endif
#endif

    I2C_ClkSel();
    i2cm_PowerCtrl(DISABLE);

    if (!i2cSemaphoreCreated)
    {
        for (tmp = 0; tmp < MAX_IICM_DEV; tmp++)
            i2cmTimingTable[tmp].devId = -1;

        SemaphoreCreate(I2CM_SEMA_ID, OS_ATTR_FIFO, 1);
        i2cSemaphoreCreated = TRUE;
        I2CM_FreqChg(0, 300000);
    }
}                               //end of I2CM_Init



void I2CM_DeInit(void)
{
    DWORD tmp;

    i2cm_PowerCtrl(DISABLE);
    IntDisable();

    if (i2cSemaphoreCreated == TRUE)
    {
        for (tmp = 0; tmp < MAX_IICM_DEV; tmp++)
            i2cmTimingTable[tmp].devId = -1;

        i2cSemaphoreCreated = FALSE;
        SemaphoreDestroy(I2CM_SEMA_ID);
    }

    IntEnable();
}                               //end of I2CM_Init



void I2CM_TimingCfg(BYTE devId, BYTE startDly, BYTE highDly, BYTE lowDly, BYTE dataChDly)
{
    DWORD value;

    value = ((DWORD) startDly) << 24;
    value |= ((DWORD) highDly) << 16;
    value |= ((DWORD) lowDly) << 8;
    value |= dataChDly & 0x03;

    IntDisable();
    i2cm_TimingTableAdd(devId, value);
    IntEnable();
}



void I2CM_FreqChg(BYTE devId, DWORD i2cFreq)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DWORD value, pllFreq, i2cBaseClk, halfClkDuty, dataChDly;
    DWORD highClkDuty, lowClkDuty;

    // Clock source select
    // 0: PLL1/4, 1: PLL1/8, 2: PLL1/64, 3: PLL1/256
    // 4: PLL2/4, 5: PLL2/8, 6: PLL2/64, 7: PLL2/256
    i2cBaseClk = (regClockPtr->Clkss2 & (BIT21 | BIT20 | BIT19)) >> 19;

    if (i2cBaseClk < 4)
    {   // PLL1
        pllFreq = Clock_PllFreqGet(CLOCK_PLL1_INDEX);
        MP_DEBUG("I2CM clock source is PLL1 %dMHz", pllFreq / 1000000);
    }
    else
    {   // PLL2
        pllFreq = Clock_PllFreqGet(CLOCK_PLL2_INDEX);
        MP_DEBUG("I2CM clock source is PLL2 %dMHz", pllFreq / 1000000);
    }

    // divid index of Base clock
    switch (i2cBaseClk % 4)
    {
    case 0:
        i2cBaseClk = pllFreq >> 2;          // / 4
        MP_DEBUG("I2CM clock select is PLL / 4 = %dKHz", pllFreq / 4000);
        break;

    case 1:
        i2cBaseClk = pllFreq >> 3;          // / 8
        MP_DEBUG("I2CM clock select is PLL / 8 = %dKHz", pllFreq / 8000);
        break;

    case 2:
        i2cBaseClk = pllFreq >> 6;          // / 64
        MP_DEBUG("I2CM clock select is PLL / 64 = %dKHz", pllFreq / 64000);
        break;

    case 3:
        i2cBaseClk = pllFreq >> 8;          // / 256
        MP_DEBUG("I2CM clock select is PLL / 256 = %dKHz", pllFreq / 256000);
        break;
    }

    ////////////////////////////////////////////////////////////////
    // i2cFreq = i2cBaseClk / ((High Duty * 4 + 1) + (Low Duty * 4 + 1))
    // theory of max I2C freq = I2C base clock / 2, when High Duty = Low Duty = 0
    // theory of min I2C freq = I2C base clock / 2042, when High Duty = Low Duty = 255
    if (i2cFreq > (i2cBaseClk >> 1))
    {
        i2cFreq = i2cBaseClk >> 1;
        MP_ALERT("Wish frequency exceed to max frequency %dKHz", i2cBaseClk / 2000);
        MP_ALERT("I2C frequency will set to max frequency - %dKHz!!!", i2cBaseClk >> 1);
    }
    else if (i2cFreq < (i2cBaseClk / 2042))
    {
        i2cFreq = i2cBaseClk / 2042;
        MP_ALERT("Wish frequency under min frequency %dHz", i2cBaseClk / 2042);
        MP_ALERT("I2C frequency will set to min frequency - %dHz!!!", i2cBaseClk / 2042);
    }

    ////////////////////////////////////////////////////////////////
    // halfClkDuty = (High Duty + Low Duty) * 8
    halfClkDuty = (i2cBaseClk / i2cFreq) - 2;
    lowClkDuty = halfClkDuty >> 3;
    highClkDuty = lowClkDuty;

    if (highClkDuty > 0xFF)
        highClkDuty = 0xFF;

    if (lowClkDuty > 0xFF)
        lowClkDuty = 0xFF;

    MP_DEBUG("Widh I2C clock frequency is %dHz", i2cFreq);
    MP_DEBUG("I2C clock frequency is %dHz", i2cBaseClk / ((highClkDuty + lowClkDuty) * 4 + 2));
    MP_DEBUG("halfClkDuty is %d", highClkDuty);
    MP_DEBUG("lowClkDuty is %d", lowClkDuty);

    if (lowClkDuty < 2)
        value = 2 << 24;
    else if (lowClkDuty <= 0x7F)
        value = lowClkDuty << 25;
        //value = lowClkDuty << 24;
    else
        value = 0xFF000000;

    value |= (highClkDuty << 16) & 0x00FF0000;
    value |= (lowClkDuty << 8) & 0x0000FF00;

    ////////////////////////////////////////////////////////////////
    dataChDly = i2cm_TimingTableSearch(devId) & 0x03;

    if (dataChDly > lowClkDuty)
        dataChDly = lowClkDuty >> 1;

    value |= dataChDly;

    ////////////////////////////////////////////////////////////////
    IntDisable();
    i2cm_TimingTableAdd(devId, value);
    IntEnable();
}



SDWORD I2CM_RdReg8Data8(BYTE DevID, BYTE reg)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD data = -1;
    BOOL semaLocked = FALSE;

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s - DevID = 0x%X reg = 0x%X", __FUNCTION__, DevID, reg);

    i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    regI2cPtr->I2C_MReg60 = BIT24;                          // read operation
    regI2cPtr->I2C_MReg60 |= ((DWORD) DevID) << 24;
    regI2cPtr->I2C_MReg60 |= ((DWORD) reg) << 16;           // register address
    //IICM waveform construction configuration
    regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
    regI2cPtr->I2C_MReg62 &= ~(I2C_16BIT_MODE | I2C_NO_RSTART); // 8bits, restart

    //IICM start
    i2cm_BusyWiat();
    regI2cPtr->I2C_MReg64 |= BIT0;

    if (i2cm_WaitAck() == PASS)
    {
        data = (regI2cPtr->I2C_MReg63 & 0xFF00) >> 8;
    }
    else
    {
        MP_ALERT("--E-- DevID=0x%2X, Reg=0x%02X read8-8 timeout !!!", (WORD) DevID, (WORD) reg);
    }

    i2cmDelayWait();
    i2cm_PowerCtrl(DISABLE);
    i2cm_resource_unlock(semaLocked);

    return data;
}                               //end of I2CM_RdReg8Data8



SDWORD I2CM_RdReg8Data16(BYTE DevID, BYTE reg)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD data = -1;
    BOOL semaLocked = FALSE;

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s - DevID = 0x%X reg = 0x%X", __FUNCTION__, DevID, reg);

    i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    regI2cPtr->I2C_MReg60 = BIT24;                          // read operation
    regI2cPtr->I2C_MReg60 |= ((DWORD) DevID) << 24;
    regI2cPtr->I2C_MReg60 |= ((DWORD) reg) << 16;           // register address
    //IICM waveform construction configuration
    regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
    regI2cPtr->I2C_MReg62 |= I2C_16BIT_MODE;
    regI2cPtr->I2C_MReg62 &= ~I2C_NO_RSTART;

    //IICM start
    i2cm_BusyWiat();
    regI2cPtr->I2C_MReg64 |= BIT0;

    if (i2cm_WaitAck() == PASS)
    {
        data = regI2cPtr->I2C_MReg63 & 0xFFFF;
    }
    else
    {
        MP_ALERT("--E-- DevID=0x%2X, Reg=0x%02X, read8-16 timeout !!!", (WORD) DevID, (WORD) reg);
    }

    i2cmDelayWait();
    i2cm_PowerCtrl(DISABLE);
    i2cm_resource_unlock(semaLocked);

    return data;
}                               //end of I2CM_RdReg8Data16




SDWORD I2CM_RdReg8Data8Nonrestart(BYTE DevID, BYTE reg)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD data = -1;
    BOOL semaLocked = FALSE;
    SDWORD status;

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s - DevID = 0x%X reg = 0x%X", __FUNCTION__, DevID, reg);

    i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    regI2cPtr->I2C_MReg60 = BIT24 | (((DWORD) DevID) << 24) | (((DWORD) reg) << 16);
    //IICM waveform construction configuration
    regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
    regI2cPtr->I2C_MReg62 &= ~I2C_16BIT_MODE;
    regI2cPtr->I2C_MReg62 |= I2C_NO_RSTART;

    //IICM start
    i2cm_BusyWiat();
    regI2cPtr->I2C_MReg64 |= BIT0;
    status = i2cm_WaitAck();

    if (status == PASS)
    {
        data = (regI2cPtr->I2C_MReg63 & 0xFF00) >> 8;
    }

    i2cm_PowerCtrl(DISABLE);
    i2cmDelayWait();
    i2cm_resource_unlock(semaLocked);

    if (status != PASS)
    {
        MP_ALERT("--E-- DevID=0x%2X, Reg=0x%02X read8-8 fail !!!", (WORD) DevID, (WORD) reg);
        //__asm("break 100");
    }

    return data;
}                               //end of I2CM_RdReg8Data8Nonrestart



SDWORD I2CM_RdReg8Data16Nonrestart(BYTE DevID, BYTE reg)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD data = -1;
    BOOL semaLocked = FALSE;
    SDWORD status;

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s - DevID = 0x%X reg = 0x%X", __FUNCTION__, DevID, reg);

    i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    regI2cPtr->I2C_MReg60 = BIT24 | (((DWORD) DevID) << 24) | (((DWORD) reg) << 16);
    //IICM waveform construction configuration
    regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
    regI2cPtr->I2C_MReg62 |= I2C_16BIT_MODE | I2C_NO_RSTART;

    //IICM start
    i2cm_BusyWiat();
    regI2cPtr->I2C_MReg64 |= BIT0;
    status = i2cm_WaitAck();

    if (status == PASS)
    {
        data = regI2cPtr->I2C_MReg63 & 0xFFFF;
    }

    i2cm_PowerCtrl(DISABLE);
    i2cmDelayWait();
    i2cm_resource_unlock(semaLocked);

    if (status != PASS)
    {
        MP_ALERT("--E-- DevID=0x%2X, Reg=0x%02X, read8-16 fail !!!", (WORD) DevID, (WORD) reg);
        //__asm("break 100");
    }

    return data;
}                               //end of I2CM_RdReg8Data16Nonrestart




SDWORD I2CM_WtReg8Data8(BYTE DevID, BYTE bReg, BYTE bData)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD status;
    BOOL semaLocked = FALSE;

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s - DevID = 0x%X bReg = 0x%X bData = 0x%X", __FUNCTION__, DevID, bReg, bData);

    i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    regI2cPtr->I2C_MReg60 = (((DWORD) DevID) << 24) | (((DWORD) bReg) << 16);
    //IICM waveform construction configuration
    regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
    regI2cPtr->I2C_MReg62 = 0;
    regI2cPtr->I2C_MReg63 = ((DWORD) bData) << 24;

    //IICM start
    i2cm_BusyWiat();
    regI2cPtr->I2C_MReg64 |= BIT0;
    status = i2cm_WaitAck();
    i2cmDelayWait();
    i2cm_PowerCtrl(DISABLE);
    i2cm_resource_unlock(semaLocked);

    if (status == FAIL)
    {
       	MP_ALERT("--E-- DevID=0x%02X, Reg=0x%02X, write8-8 fail !!!", (WORD) DevID, (WORD) bReg);
        //__asm("break 100");
    }
	else
	{
//        MP_ALERT("--E-- DevID=0x%02X, Reg=0x%02X, write8-8 OK !!!", (WORD) DevID, (WORD) bReg);

	}

    return status;
}                               //end of I2CM_Wt



SDWORD I2CM_WtReg16Data8(BYTE DevID, WORD wReg, BYTE bData)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD status;
    BOOL semaLocked = FALSE;

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s - DevID = 0x%X wReg = 0x%X bData = 0x%X", __FUNCTION__, DevID, wReg, bData);

    i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    regI2cPtr->I2C_MReg60 = (((DWORD) DevID) << 24) | ((((DWORD) wReg) >> 8) << 16);
    //IICM waveform construction configuration
    regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
    regI2cPtr->I2C_MReg62 = I2C_16BIT_MODE;
    regI2cPtr->I2C_MReg63 = (((DWORD) wReg & 0xFF) << 24) | (((DWORD) bData) << 16);
    regI2cPtr->I2C_MReg63 |= ((DWORD) bData) << 16;

    //IICM start
    i2cm_BusyWiat();
    regI2cPtr->I2C_MReg64 |= BIT0;
    status = i2cm_WaitAck();
    i2cmDelayWait();
    i2cm_PowerCtrl(DISABLE);
    i2cm_resource_unlock(semaLocked);

    if (status == FAIL)
    {
        MP_ALERT("--E-- DevID=0x%2X, Reg=0x%02X, write16-8 fail !!!", (WORD) DevID, (WORD) wReg);
        //__asm("break 100");
    }

    return status;
}                               //end of I2CM_WtReg16



SDWORD I2CM_WtReg8Data16(BYTE DevID, BYTE bReg, WORD wData)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD status;
    BOOL semaLocked = FALSE;

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s - DevID = 0x%X bReg = 0x%X wData = 0x%X", __FUNCTION__, DevID, bReg, wData);

    i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    regI2cPtr->I2C_MReg60 = (((DWORD) DevID) << 24) | (((DWORD) bReg) << 16);
    //IICM waveform construction configuration
    regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
    regI2cPtr->I2C_MReg62 = I2C_16BIT_MODE;
    regI2cPtr->I2C_MReg63 = ((DWORD) wData) << 16;

    //IICM start
    i2cm_BusyWiat();
    regI2cPtr->I2C_MReg64 |= BIT0;
    status = i2cm_WaitAck();
    i2cmDelayWait();
    i2cm_PowerCtrl(DISABLE);
    i2cm_resource_unlock(semaLocked);

    if (status == FAIL)
    {
        MP_ALERT("--E-- DevID=0x%2X, Reg=0x%02X, write8-16 fail !!!", (WORD) DevID, (WORD) bReg);
        //__asm("break 100");
    }

    return status;
}                               //end of I2CM_WtData16




SDWORD I2CM_Write_BustMode(BYTE DevID, BYTE *outDataBuf, WORD dataLength)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD status = PASS;
    DWORD i;
    BOOL semaLocked = FALSE;

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s to DevID-0x%02X, total %dbytes", __FUNCTION__, DevID, dataLength);

    i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    regI2cPtr->I2C_MReg60 = ((DWORD) DevID) << 24;
    regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
    regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART | I2C_16BIT_MODE;

    for (i = 0; i < dataLength; i++)
    {
        regI2cPtr->I2C_MReg63 = ((DWORD) outDataBuf[i]) << 24;
        regI2cPtr->I2C_MReg62 |= I2C_16BIT_MODE;
        i2cm_BusyWiat();
        regI2cPtr->I2C_MReg64 |= BIT0;
	IODelay(20);
        if (i2cm_WaitNext() == FAIL)
        {
            status = FAIL;
            mpDebugPrint("--E-- %s:A DevID=0x%02X fail !!!", __FUNCTION__, (WORD) DevID);

            for (i = 0; i < dataLength; i++)
                mpDebugPrintN("0x%02X ", outDataBuf[i]);

            break;
        }
    }

    if (status != FAIL)
    {
        regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART;
        regI2cPtr->I2C_MReg62 &= ~I2C_16BIT_MODE;
        i2cm_BusyWiat();
        regI2cPtr->I2C_MReg64 |= BIT0;

        status = i2cm_WaitAck();

        if (status == FAIL)
        {
            MP_ALERT("--E-- %s:B DevID=0x%02X fail !!!", __FUNCTION__, (WORD) DevID);
        }
    }

    i2cmDelayWait();
    i2cm_PowerCtrl(DISABLE);
    i2cm_resource_unlock(semaLocked);

    return status;
}
SDWORD I2CM_Read_Length_BustMode(BYTE DevID, BYTE *outDataBuf, BYTE dataLength, BYTE *inDataBuf, WORD readLength)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD status = PASS;
    DWORD i;
    BOOL semaLocked = FALSE;
    BYTE tmpRegAddr[2];

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s to DevID, total %dbytes", __FUNCTION__, DevID, readLength);


    i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    regI2cPtr->I2C_MReg60 = ((DWORD) DevID) << 24;
    regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
    regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART | I2C_16BIT_MODE;

    for (i = 0; i < dataLength; i++)
    {
        regI2cPtr->I2C_MReg63 = ((DWORD) outDataBuf[i]) << 24;
        i2cm_BusyWiat();
        regI2cPtr->I2C_MReg64 |= BIT0;

        if (i2cm_WaitNext() == FAIL)
        {
            status = FAIL;
            MP_ALERT("--E-- %s:A DevID=0x%02X timeout !!!", __FUNCTION__, (WORD) DevID);

            break;
        }
    }

    regI2cPtr->I2C_MReg62 = I2C_LOOP_MODE | I2C_NO_RSTART;
    i2cm_BusyWiat();
    regI2cPtr->I2C_MReg64 |= BIT0;

    if (status == FAIL)
        i2cm_WaitAck();
    else
        status = i2cm_WaitAck();

    if (status == PASS)
    {
        regI2cPtr->I2C_MReg60 = BIT24 | (((DWORD) DevID) << 24);        // read operation
        regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
        regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART | I2C_16BIT_MODE;

        for (i = 0; i < readLength; i++)
        {
            i2cm_BusyWiat();
            regI2cPtr->I2C_MReg64 |= BIT0;

            if (i2cm_WaitNext() == FAIL)
            {
                status = FAIL;
                MP_ALERT("--E-- %s: DevID=0x%02X timeout !!!", __FUNCTION__, (WORD) DevID);
                break;
            }

            inDataBuf[i] = (regI2cPtr->I2C_MReg63 & 0xFF00) >> 8;
        }

        regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART;
        i2cm_BusyWiat();
        regI2cPtr->I2C_MReg64 |= BIT0;
        i2cm_WaitAck();
    }

    i2cm_PowerCtrl(DISABLE);
    i2cmDelayWait();
    i2cm_resource_unlock(semaLocked);

    return status;
}


SDWORD I2CM_Read_BustMode(BYTE DevID, WORD regAddr, BYTE regAddr16, BYTE *inDataBuf, WORD readLength)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD status = PASS;
    DWORD i, dataLength;
    BOOL semaLocked = FALSE;
    BYTE tmpRegAddr[2];

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s to DevID, total %dbytes", __FUNCTION__, DevID, readLength);

    if (regAddr16)
    {
        dataLength = 2;
        tmpRegAddr[0] = regAddr >> 8;
        tmpRegAddr[1] = regAddr & 0xFF;
    }
    else
    {
        dataLength = 1;
        tmpRegAddr[0] = regAddr & 0xFF;
    }

    i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    regI2cPtr->I2C_MReg60 = ((DWORD) DevID) << 24;
    regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
    regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART | I2C_16BIT_MODE;

    for (i = 0; i < dataLength; i++)
    {
        regI2cPtr->I2C_MReg63 = ((DWORD) tmpRegAddr[i]) << 24;
        i2cm_BusyWiat();
        regI2cPtr->I2C_MReg64 |= BIT0;

        if (i2cm_WaitNext() == FAIL)
        {
            status = FAIL;
            MP_ALERT("--E-- %s:A DevID=0x%02X timeout !!!", __FUNCTION__, (WORD) DevID);

            break;
        }
    }

    regI2cPtr->I2C_MReg62 = I2C_LOOP_MODE | I2C_NO_RSTART;
    i2cm_BusyWiat();
    regI2cPtr->I2C_MReg64 |= BIT0;

    if (status == FAIL)
        i2cm_WaitAck();
    else
        status = i2cm_WaitAck();

    if (status == PASS)
    {
        regI2cPtr->I2C_MReg60 = BIT24 | (((DWORD) DevID) << 24);        // read operation
        regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
        regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART | I2C_16BIT_MODE;

        for (i = 0; i < readLength; i++)
        {
            i2cm_BusyWiat();
            regI2cPtr->I2C_MReg64 |= BIT0;

            if (i2cm_WaitNext() == FAIL)
            {
                status = FAIL;
                MP_ALERT("--E-- %s: DevID=0x%02X timeout !!!", __FUNCTION__, (WORD) DevID);
                break;
            }

            inDataBuf[i] = (regI2cPtr->I2C_MReg63 & 0xFF00) >> 8;
        }

        regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART;
        i2cm_BusyWiat();
        regI2cPtr->I2C_MReg64 |= BIT0;
        i2cm_WaitAck();
    }

    i2cm_PowerCtrl(DISABLE);
    i2cmDelayWait();
    i2cm_resource_unlock(semaLocked);

    return status;
}

SDWORD I2CM_ReadData_BustMode(BYTE DevID, WORD *inDataBuf, WORD readLength)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD status = PASS;
    DWORD i, dataLength;
    BOOL semaLocked = FALSE;
    BYTE tmpRegAddr[2];

    i2cm_resource_lock(&semaLocked);
    MP_DEBUG("%s to DevID, total %dbytes", __FUNCTION__, DevID, readLength);

    if (status == PASS)
    {
    	 i2cm_PowerCtrl(ENABLE);
        DevID &= 0xFF;
        regI2cPtr->I2C_MReg60 = BIT24 | (((DWORD) DevID) << 24);        // read operation
        regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
        regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART | I2C_16BIT_MODE;

        for (i = 0; i < readLength; i++)
        {
            i2cm_BusyWiat();
            regI2cPtr->I2C_MReg64 |= BIT0;

            if (i2cm_WaitNext() == FAIL)
            {
                status = FAIL;
                MP_ALERT("--E-- %s: DevID=0x%02X timeout !!!", __FUNCTION__, (WORD) DevID);
                break;
            }

            inDataBuf[i] = (regI2cPtr->I2C_MReg63 & 0xFF00) >> 8;
        }

        regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART;
        i2cm_BusyWiat();
        regI2cPtr->I2C_MReg64 |= BIT0;
        i2cm_WaitAck();
    }

    i2cm_PowerCtrl(DISABLE);
    i2cmDelayWait();
    i2cm_resource_unlock(semaLocked);

    return status;
}
#endif


#if ((HW_IIC_MASTER_ENABLE == 1) || (HW_IIC_SLAVE_ENABLE == 1))
void I2C_ClkSel(void)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    //IICM clock configuration
    // 0: PLL1/4, 1: PLL1/8, 2: PLL1/64, 3: PLL1/256
    // 4: PLL2/4, 5: PLL2/8, 6: PLL2/64, 7: PLL2/256
    regClockPtr->Clkss2 &= ~(BIT21 | BIT20 | BIT19);    //Clean all bits
#ifdef IIC_CLOCK_SOURCE_USING_PLL2
    regClockPtr->Clkss2 |= (BIT21 | BIT19);             // 5, IICM clock = PLL2 / 8, 153/8 = 19MHz
    //regClockPtr->Clkss2 |= (BIT21 | BIT20);             // 6, IICM clock = PLL2 / 64 = 2.59MHz
#else
    regClockPtr->Clkss2 |= BIT19;                       // 1, IICM clock = PLL1 / 8 = 12MHz
#endif
}
#endif



////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(I2CM_Init);
MPX_KMODAPI_SET(I2CM_DeInit);
MPX_KMODAPI_SET(I2CM_TimingCfg);
MPX_KMODAPI_SET(I2CM_FreqChg);
MPX_KMODAPI_SET(I2CM_RdReg8Data16);
MPX_KMODAPI_SET(I2CM_RdReg8Data8);
MPX_KMODAPI_SET(I2CM_WtReg16Data8);
#endif

