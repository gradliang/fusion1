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
* Filename      : hal_adc.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : Supported for MP650/660
*
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "peripheral.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

//---------------------------------------------------------------------------
//
//  ADC0~3
//
//---------------------------------------------------------------------------
#define __ENABLE_SAR_ADC_LOW_PASS_FILTER__

#ifdef __ENABLE_SAR_ADC_LOW_PASS_FILTER__
#define __SAR_ADC_FILTER_ORDER_NUMBER       3

static BOOL adcLpfEnabl[4] = {0};
static SDWORD preAdcValue[4][__SAR_ADC_FILTER_ORDER_NUMBER]; // do bouncing process
static SDWORD preValidAdcValue[4];
#endif

static BYTE sarAdcLpfCalculater(E_ADC_GROUP_INDEX adcIndex, BYTE adcValue)
{
    MP_DEBUG("%s: adcIndex = %d, adcValue = %d", __func__, adcIndex, adcValue);
    
#ifdef __ENABLE_SAR_ADC_LOW_PASS_FILTER__
    SDWORD error, historyAdcValue = 0;
    BYTE j;

#if (CHIP_VER_MSB == CHIP_VER_650)
    if (adcIndex > E_ADC_GROUP_INDEX_3)
        return adcValue;
else
    if (adcIndex > E_ADC_GROUP_INDEX_1)
        return adcValue;
#endif

    MP_DEBUG("adcLpfEnabl[adcIndex] = %d", adcLpfEnabl[adcIndex]);
    if (adcLpfEnabl[adcIndex])
    {
        for (j = 0; j < __SAR_ADC_FILTER_ORDER_NUMBER; j++)
            historyAdcValue += preAdcValue[adcIndex][j];

        if (historyAdcValue > (adcValue * __SAR_ADC_FILTER_ORDER_NUMBER))
            error = historyAdcValue - (adcValue * __SAR_ADC_FILTER_ORDER_NUMBER);
        else
            error = (adcValue * __SAR_ADC_FILTER_ORDER_NUMBER) - historyAdcValue;

        if (error != 0)
        {
            for (j = 1; j < __SAR_ADC_FILTER_ORDER_NUMBER; j++)
            {
                preAdcValue[adcIndex][j - 1] = preAdcValue[adcIndex][j];
            }
            MP_DEBUG("pre = %d, %d, %d", preAdcValue[adcIndex][0], preAdcValue[adcIndex][1], preAdcValue[adcIndex][2]);

            preAdcValue[adcIndex][__SAR_ADC_FILTER_ORDER_NUMBER - 1] = adcValue;
            adcValue = preValidAdcValue[adcIndex];
        }
        else
        {
            preValidAdcValue[adcIndex] = adcValue;
        }
    }
#endif
    
    if(adcValue < 15)
        MP_DEBUG("-> %d,", adcValue);
    
    return adcValue;
}



SDWORD EmbededAdc_LPF_Initial(E_ADC_GROUP_INDEX group, BOOL enable, BYTE defaultValue)
{
    MP_DEBUG("%s", __func__);
    //MP_ALERT("%s: group = %d, enable = %d, defaultValue = %d", __func__, group, enable, defaultValue);
#ifdef __ENABLE_SAR_ADC_LOW_PASS_FILTER__
    BYTE j;

#if (CHIP_VER_MSB == CHIP_VER_650)
    if (group > E_ADC_GROUP_INDEX_3)
        return FALSE;
else
    if (group > E_ADC_GROUP_INDEX_1)
        return FALSE;
#endif

    adcLpfEnabl[group] = enable;

    for (j = 0; j < __SAR_ADC_FILTER_ORDER_NUMBER; j++)
    {
        preAdcValue[group][j] = defaultValue;
        MP_DEBUG("preAdcValue[%d][%d] = %d", group, j, preAdcValue[group][j]);
    }

    preValidAdcValue[group] = defaultValue;
    MP_DEBUG("preValidAdcValue[%d] = %d", group, preValidAdcValue[group]);

    return TRUE;
#else
    return FALSE;
#endif
}



SDWORD EmbededAdc_Enable(E_ADC_GROUP_INDEX group, DWORD clkSel)
{
    MP_DEBUG("%s: group = %d, clkSel = %d", __func__, group, clkSel);
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    GPIO *regGpioPtr = (GPIO *) GPIO_BASE;
    DWORD pllFreq, adcFreq;

    clkSel &= 0x0000000F;

    if ((clkSel & BIT3) == 0)
        pllFreq = Clock_PllFreqGet(CLOCK_PLL2_INDEX);
    else
        pllFreq = Clock_PllFreqGet(CLOCK_PLL3_INDEX);

    if ((pllFreq / 2048) > MAX_ADC_CLK_FREQ)
    {
        MP_ALERT("--E-- PLL's frequencky exceed to max ADC's max frequency, %dKHz!!!", MAX_ADC_CLK_FREQ);

        return -2;
    }

    adcFreq = pllFreq / (16 * (1 << (clkSel & 0x07)));
    MP_DEBUG("Embdded ADC-%d clk is %dKHz, clkSel = %d", group, adcFreq / 1000, clkSel);

    if (adcFreq > MAX_ADC_CLK_FREQ)
    {
        DWORD newClkSel = clkSel & 0x07;

        MP_DEBUG("--W-- Exceed to embdded ADC clk's max freq %dKHz", MAX_ADC_CLK_FREQ / 1000);

        do {
            newClkSel++;
            adcFreq = pllFreq / (16 * (1 << (newClkSel & 0x07)));
            MP_DEBUG("pllFreq = %dKHz, ADC clk is %dKHz, newClkSel = %d", pllFreq / 1000, adcFreq / 1000, newClkSel);
        } while ((adcFreq > MAX_ADC_CLK_FREQ) && (newClkSel <= ADC_CLK_DIV_2048));

        MP_ALERT("--W-- New embdded ADC clk is %dKHz, newClkSel = %d", adcFreq / 1000, newClkSel);
        clkSel = (clkSel & 0x8) | newClkSel;
    }

    IntDisable();

    switch (group)
    {
    case E_ADC_GROUP_INDEX_0:
        regClockPtr->Clkss_EXT0 = (regClockPtr->Clkss_EXT0 & 0xFFFFFFF0) | clkSel;
        regClockPtr->Clkss_EXT1 |= CKE_ADC0;
        regGpioPtr->AD_Level &= ~ADC0_PWR_DISABLE;      // ADC0 Power On
        Gpio_ConfiguraionSet(GPIO_KGPIO_0, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, GPIO_DATA_LOW, 1);
        break;

    case E_ADC_GROUP_INDEX_1:
        regClockPtr->Clkss_EXT0 = (regClockPtr->Clkss_EXT0 & 0xFFFFFF0F) | (clkSel << 4);
        regClockPtr->Clkss_EXT1 |= CKE_ADC1;
        regGpioPtr->AD_Level &= ~ADC1_PWR_DISABLE;      // ADC1 Power On
        Gpio_ConfiguraionSet(GPIO_KGPIO_1, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, GPIO_DATA_LOW, 1);
        break;
#if (CHIP_VER_MSB == CHIP_VER_650)
    case E_ADC_GROUP_INDEX_2:
        regClockPtr->Clkss_EXT0 = (regClockPtr->Clkss_EXT0 & 0xFFFF0FFF) | (clkSel << 12);
        regClockPtr->Clkss_EXT1 |= CKE_ADC2;
        regGpioPtr->AD_Level &= ~ADC2_PWR_DISABLE;      // ADC2 Power On
        Gpio_ConfiguraionSet(GPIO_KGPIO_2, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, GPIO_DATA_LOW, 1);
        break;

    case E_ADC_GROUP_INDEX_3:
        regClockPtr->Clkss_EXT0 = (regClockPtr->Clkss_EXT0 & 0xFF0FFFFF) | (clkSel << 20);
        regClockPtr->Clkss_EXT1 |= CKE_ADC2;            // MP65X IC bug
        regClockPtr->Clkss_EXT1 |= CKE_ADC3;
        regGpioPtr->AD_Level &= ~ADC3_PWR_DISABLE;      // ADC3 Power On
        Gpio_ConfiguraionSet(GPIO_KGPIO_3, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, GPIO_DATA_LOW, 1);
        break;
#endif

    default:
        MP_ALERT("--E-- EmbededAdc_Enable - Wrong ADC Index !!!");
        IntEnable();

        return -1;
        break;
    }

    IntEnable();

    return NO_ERR;
}



SDWORD EmbededAdc_Disable(E_ADC_GROUP_INDEX group)
{
    MP_DEBUG("%s", __func__);
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    GPIO *regGpioPtr = (GPIO *) GPIO_BASE;

    switch (group)
    {
    case E_ADC_GROUP_INDEX_0:
        Gpio_Config2GpioFunc(GPIO_KGPIO_0, GPIO_INPUT_MODE, 0, 1);
        IntDisable();
        regGpioPtr->AD_Level |= ADC0_PWR_DISABLE;      // ADC0 Power Down
        regClockPtr->Clkss_EXT1 &= ~CKE_ADC0;
        break;

    case E_ADC_GROUP_INDEX_1:
        Gpio_Config2GpioFunc(GPIO_KGPIO_1, GPIO_INPUT_MODE, 0, 1);
        IntDisable();
        regGpioPtr->AD_Level |= ADC1_PWR_DISABLE;       // ADC1 Power Down
        regClockPtr->Clkss_EXT1 &= ~CKE_ADC1;
        break;

#if (CHIP_VER_MSB == CHIP_VER_650)
    case E_ADC_GROUP_INDEX_2:
        Gpio_Config2GpioFunc(GPIO_KGPIO_2, GPIO_INPUT_MODE, 0, 1);
        IntDisable();
        regGpioPtr->AD_Level |= ADC2_PWR_DISABLE;      // ADC2 Power Down
        regClockPtr->Clkss_EXT1 &= ~CKE_ADC2;
        break;

    case E_ADC_GROUP_INDEX_3:
        Gpio_Config2GpioFunc(GPIO_KGPIO_3, GPIO_INPUT_MODE, 0, 1);
        IntDisable();
        regGpioPtr->AD_Level |= ADC3_PWR_DISABLE;      // ADC3 Power Down
        regClockPtr->Clkss_EXT1 &= ~CKE_ADC3;
        break;
#endif

    default:
        MP_ALERT("--E-- EmbededAdc_Disable - Wrong ADC Index !!!");

        return -1;
        break;
    }

    IntEnable();

    return NO_ERR;
}



#define __ADC_PD_DELAY_TIME__               200

SDWORD EmbededAdc_ValueGet(E_ADC_GROUP_INDEX group)
{
    MP_DEBUG("%s", __func__);
    GPIO *regGpioPtr = (GPIO *) GPIO_BASE;
    SDWORD data;
    volatile WORD delay = __ADC_PD_DELAY_TIME__;

    switch (group)
    {
    case E_ADC_GROUP_INDEX_0:
        regGpioPtr->AD_Level &= ~ADC0_PWR_DISABLE;      // ADC0 Power On
        while (delay--);
        data = regGpioPtr->AD_Level & 0x0F;
        regGpioPtr->AD_Level |= ADC0_PWR_DISABLE;       // ADC0 Power Down
        break;

    case E_ADC_GROUP_INDEX_1:
        regGpioPtr->AD_Level &= ~ADC1_PWR_DISABLE;      // ADC1 Power On
        while (delay--);
        data = (regGpioPtr->AD_Level >> 8) & 0x0F;
        regGpioPtr->AD_Level |= ADC1_PWR_DISABLE;       // ADC1 Power Down
        break;

#if (CHIP_VER_MSB == CHIP_VER_650)
    case E_ADC_GROUP_INDEX_2:
        regGpioPtr->AD_Level &= ~ADC2_PWR_DISABLE;      // ADC2 Power On
        while (delay--);
        data = (regGpioPtr->AD_Level >> 16) & 0x0F;
        regGpioPtr->AD_Level |= ADC2_PWR_DISABLE;       // ADC2 Power Down
        break;

    case E_ADC_GROUP_INDEX_3:
        regGpioPtr->AD_Level &= ~ADC3_PWR_DISABLE;      // ADC3 Power On
        while (delay--);
        data = (regGpioPtr->AD_Level >> 24) & 0x0F;
        regGpioPtr->AD_Level |= ADC3_PWR_DISABLE;       // ADC3 Power Down
        break;
#endif

    default:
        MP_ALERT("--E-- EmbededAdc_ValueGet - Wrong ADC Index !!!");

        return -1;
        break;
    }

    if(data < 15)
        MP_DEBUG("ADC%02d - %d", group, data);
    data = sarAdcLpfCalculater(group, (BYTE) data);

    return data;
}



#if 0
//---------------------------------------------------------------------------
//
//  KP ADC
//
//---------------------------------------------------------------------------
static void (*KpAdc_CallBack_function)(void) = 0;

void EnableKpAdc(BOOL enableInt)
{
    MP_DEBUG("%s", __func__);
    BIU *biu = (BIU *) BIU_BASE;
    CLOCK *clock = (CLOCK *) CLOCK_BASE;
    GPIO *gpio = (GPIO *) GPIO_BASE;
    DWORD tmp;

    clock->Clkss_EXT2 |= BIT9;                      // KPADC clk enable
    clock->Clkss_EXT2 &= ~BIT8;                     // default, KPADCK select PLL2/4
    //clock->Clkss_EXT2 |= BIT8;                      // KPADCK select PLL1/4

    // Reset KeyPad Logic
    biu->BiuArst &= ~ARST_KPAD;
    for (tmp = 0; tmp <= 0x100; tmp++);
    biu->BiuArst |= ARST_KPAD;

    gpio->Gpdat0 &= ~BIT16;                         // GP0 as input
    gpio->Gpdat0 |= BIT17;                          // GP1 as output
    gpio->Gpcfg0 &= ~(BIT17 | BIT16 | BIT1 | BIT0); // GP1, GP0 as default function
    gpio->Gpcfg0 |= BIT1 | BIT0;                    // GP1, GP0 as Alternative function 1

    if (enableInt)
    {
        INTERRUPT *isr = (INTERRUPT *) INT_BASE;

        isr->MiMask |= IM_KPAD;
    }
}



void DisableKpAdc(void)
{
    MP_DEBUG("%s", __func__);
    INTERRUPT *isr = (INTERRUPT *) INT_BASE;
    BIU *biu = (BIU *) BIU_BASE;
    CLOCK *clock = (CLOCK *) CLOCK_BASE;
    GPIO *gpio = (GPIO *) GPIO_BASE;
    DWORD tmp;

    isr->MiMask &= ~IM_KPAD;

    // Reset KeyPad Logic
    biu->BiuArst &= ~ARST_KPAD;
    for (tmp = 0; tmp <= 0x100; tmp++);
    biu->BiuArst |= ARST_KPAD;

    gpio->Gpdat0 &= ~(BIT17 | BIT16);               // GP1, GP0 as input
    gpio->Gpcfg0 &= ~(BIT17 | BIT16 | BIT1 | BIT0); // GP1, GP0 as default function

    clock->Clkss_EXT2 &= ~BIT9;                     // KPADC clk disable
}



DWORD GetKpAdcValue(BYTE group)
{
    GPIO *gpio = (GPIO *) GPIO_BASE;

    MP_ALERT("--E-- GetKpAdcValue - Not finished yet !!!");

    return 0;
}



void KpAdcRegisterCallBackFunc(void (*KpAdc_CallBack_func) (void))
{
    MP_DEBUG("%s", __func__);
    KpAdc_CallBack_function = KpAdc_CallBack_func;
}



void KpAdcClearCallBackFunc(void)
{
    MP_DEBUG("%s", __func__);
    KpAdc_CallBack_function = NULL;
}



void KpAdc_Isr()
{
    MP_DEBUG("%s", __func__);
    if (KpAdc_CallBack_function)
        KpAdc_CallBack_function();
}
#endif



////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(EmbededAdc_Enable);
MPX_KMODAPI_SET(EmbededAdc_Disable);
MPX_KMODAPI_SET(EmbededAdc_ValueGet);
#endif

#endif      // #if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

