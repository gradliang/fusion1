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
* Filename      : hal_IICS.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : Slave I2C module
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
#include "os.h"
#include "peripheral.h"

#if (HW_IIC_SLAVE_ENABLE == 1)

static void dummyFunc(void);
static void i2cs_ReadIsr(void);
static void i2cs_WriteIsr(void);

/*
// Variable declarations
*/
static void (*i2csWriteCallBackFunPtr)(void) = dummyFunc;
static void (*i2csReadCallBackFunPtr)(void) = dummyFunc;

static void dummyFunc(void)
{
    mpDebugPrint("I2C Slave - dummy function !!!!");
}



static void i2cs_GpioInit(BYTE enable)
{
    if (enable == ENABLE)
    {
#if (CHIP_VER_MSB == CHIP_VER_615)
        Gpio_ConfiguraionSet(GPIO_GPIO_22, GPIO_ALT_FUNC_3, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
#else
        #if (CHIP_VER == (CHIP_VER_650 | CHIP_VER_FPGA))
        Gpio_ConfiguraionSet(GPIO_GPIO_3, GPIO_ALT_FUNC_3, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
        #else
        Gpio_ConfiguraionSet(GPIO_PGPIO_2, GPIO_ALT_FUNC_2, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
        #endif
#endif
    }
    else
    {
#if (CHIP_VER_MSB == CHIP_VER_615)
        Gpio_Config2GpioFunc(GPIO_GPIO_22, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
#else
        #if (CHIP_VER == (CHIP_VER_650 | CHIP_VER_FPGA))
        Gpio_Config2GpioFunc(GPIO_GPIO_3, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
        #else
        Gpio_Config2GpioFunc(GPIO_PGPIO_2, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
        #endif
#endif
    }
}



void I2CS_Init(BYTE chipID)
{
    BIU *regBiuPtr = (BIU *) BIU_BASE;
    S_I2C_REG *regI2cPtr = (S_I2C_REG *) S_I2C_BASE;
    CLOCK *regClkPtr = (CLOCK *) CLOCK_BASE;
    USB_OTG *regOtgPtr = (USB_OTG *) USBOTG0_BASE;
    WORD tmp;

#if (CHIP_VER_MSB != CHIP_VER_615)
    //regOtgPtr->RgEco |= BIT31;                      // Enable I2CS SCL and SDA clock latch by CPU clock
    regOtgPtr->RgEco |= BIT31 | BIT30;              // Enable I2CS SCL and SDA clock latch by I2C-M clock
#endif
    I2C_ClkSel();                                   // using IIC-M's clk sel

    //reset IIC Slave module
    regBiuPtr->BiuArst &= ~ARST_I2CS;
    regClkPtr->MdClken &= ~CKE_I2CS;                // disable IICS clock
    for (tmp = 0; tmp < 0x500; tmp++);
    regClkPtr->MdClken |= CKE_I2CS;                 // enable IICS clock
    for (tmp = 0; tmp < 0x500; tmp++);
    regBiuPtr->BiuArst |= ARST_I2CS;
    for (tmp = 0; tmp < 0x500; tmp++);

    //Initial I2C read region
    regI2cPtr->I2C_SWHandShake = 0x07000001;        // TY - BurstMode, Enable Int, Clear Int, wNotify
    //regI2cPtr->I2C_SWHandShake = 0x07010000;        // Org - BurstMode, Enable Int, Clear Int, wNotify

    // default value
    regI2cPtr->I2C_SRReg1 = 0xabcdef12;
    regI2cPtr->I2C_SRReg2 = 0x21fedcba;
    regI2cPtr->I2C_SRReg3 = 0x34567890;
    regI2cPtr->I2C_SRReg4 = 0x09876543;
    regI2cPtr->I2C_SIDReg = (chipID >> 1);

    MP_DEBUG("Slave I2C Chip ID is 0x%02X, 0x%08X, 0x%X", chipID, &regI2cPtr->I2C_SIDReg, regI2cPtr->I2C_SIDReg);

    i2cs_GpioInit(ENABLE);

    //regI2cPtr->I2C_SWHandShake = 0x06010000;        // Org - BurstMode, Enable Int, wNotify
    regI2cPtr->I2C_SWHandShake = 0x06000001;        // TY -BurstMode, Enable Int, wNotify
    //regI2cPtr->I2C_SRHandShake = 0x02010000;        // Org - Enable Int, rNotify
    //regI2cPtr->I2C_SRHandShake = 0x02000001;        // TY - Enable Int, rNotify

    i2csWriteCallBackFunPtr = dummyFunc;
    i2csReadCallBackFunPtr = dummyFunc;

    // Enable IICS Interrupt
    SystemIntHandleRegister(IM_I2CSR, i2cs_ReadIsr);
    SystemIntEna(IM_I2CSR);
    SystemIntHandleRegister(IM_I2CSW, i2cs_WriteIsr);
    SystemIntEna(IM_I2CSW);
}                               //end of I2CS_Init



void I2CS_DeInit(void)
{
    BIU *regBiuPtr = (BIU *) BIU_BASE;
    CLOCK *regClkPtr = (CLOCK *) CLOCK_BASE;
    WORD tmp;

    i2cs_GpioInit(DISABLE);

    //reset IIC Slave module
    regBiuPtr->BiuArst &= ~ARST_I2CS;
    for (tmp = 0; tmp < 0x80; tmp++);
    regClkPtr->MdClken &= ~CKE_I2CS;        // disable IICS clock

    // Disable IICS Interrupt
    SystemIntDis(IM_I2CSR);
    SystemIntHandleRelease(IM_I2CSR);
    SystemIntDis(IM_I2CSW);
    SystemIntHandleRelease(IM_I2CSW);
}



static void i2cs_WriteIsr(void)
{
    S_I2C_REG *regI2cPtr = (S_I2C_REG *) S_I2C_BASE;

    //clear write INT signal
    regI2cPtr->I2C_SWHandShake |= BIT24;
    regI2cPtr->I2C_SWHandShake &= ~BIT24;

    i2csWriteCallBackFunPtr();
}



static void i2cs_ReadIsr(void)
{
    S_I2C_REG *regI2cPtr = (S_I2C_REG *) S_I2C_BASE;

    //clear read INT signal
    regI2cPtr->I2C_SRHandShake |= BIT24;
    regI2cPtr->I2C_SRHandShake &= ~BIT24;

    i2csReadCallBackFunPtr();
}



void I2CS_WriteRegisterCallBackFunc(void (*I2CS_CallBack_func)(void))
{
    if (I2CS_CallBack_func)
        i2csWriteCallBackFunPtr = I2CS_CallBack_func;
    else
        i2csWriteCallBackFunPtr = dummyFunc;
}



void I2CS_WriteClearCallBackFunc(void)
{
    i2csWriteCallBackFunPtr = dummyFunc;
}



void I2CS_ReadRegisterCallBackFunc(void (*I2CS_CallBack_func)(void))
{
    if (I2CS_CallBack_func)
        i2csReadCallBackFunPtr = I2CS_CallBack_func;
    else
        i2csReadCallBackFunPtr = dummyFunc;
}



void I2CS_ReadClearCallBackFunc(void)
{
    i2csReadCallBackFunPtr = dummyFunc;
}



////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(I2CS_Init);
MPX_KMODAPI_SET(I2CS_DeInit);
MPX_KMODAPI_SET(I2CS_ReadRegisterCallBackFunc);
MPX_KMODAPI_SET(I2CS_ReadClearCallBackFunc);
MPX_KMODAPI_SET(I2CS_WriteRegisterCallBackFunc);
MPX_KMODAPI_SET(I2CS_WriteClearCallBackFunc);
MPX_KMODAPI_SET(I2CM_WtReg16Data8);
#endif

#endif

