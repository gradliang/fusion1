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
* Filename      : hal_ADC.h
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : 4-bits ADC for MP650/660
*******************************************************************************
*/
#ifndef __HAL_ADC_H
#define __HAL_ADC_H

////////////////////////////////////////////////////////////
//
// Include section
//
////////////////////////////////////////////////////////////

#include "UtilTypedef.h"
#include "bitsdefine.h"
#include "iplaysysconfig.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

////////////////////////////////////////////////////////////
//
// Constant declarations
//
////////////////////////////////////////////////////////////

///
///@ingroup     ADC_MODULE
///@brief       define the index numberr of 4-bits ADC.
typedef enum {
    E_ADC_GROUP_INDEX_0 = 0,                    ///< ADC0 index number (0)
    E_ADC_GROUP_INDEX_1,                        ///< ADC1 index number (1)
#if (CHIP_VER_MSB == CHIP_VER_650)
    E_ADC_GROUP_INDEX_2,                        ///< ADC2 index number (2)
    E_ADC_GROUP_INDEX_3                         ///< ADC3 index number (3)
#endif
} E_ADC_GROUP_INDEX;

#define ADC_CLK_SEL_PLL2        0x0             // PLL2
#define ADC_CLK_SEL_PLL3        0x8             // PLL3

#define ADC_CLK_DIV_16          0x0
#define ADC_CLK_DIV_32          0x1
#define ADC_CLK_DIV_64          0x2
#define ADC_CLK_DIV_128         0x3
#define ADC_CLK_DIV_256         0x4
#define ADC_CLK_DIV_512         0x5
#define ADC_CLK_DIV_1024        0x6
#define ADC_CLK_DIV_2048        0x7

#define ADC0_PWR_DISABLE        BIT4
#define ADC1_PWR_DISABLE        BIT12
#define ADC2_PWR_DISABLE        BIT20
#define ADC3_PWR_DISABLE        BIT28

#define MAX_ADC_CLK_FREQ        10000000

////////////////////////////////////////////////////////////
//
// Function prototype
//
////////////////////////////////////////////////////////////

///
///@ingroup     ADC_MODULE
///@brief       Enable Embedded ADC (Keypad)
///
///@param       group       ADC group index, see E_ADC_GROUP_INDEX
///@param       clkSel      ADC's clock source\n
///                         0 (ADC_CLK_SEL_PLL2 | ADC_CLK_DIV_16)   - PLL2/16\n
///                         1 (ADC_CLK_SEL_PLL2 | ADC_CLK_DIV_32)   - PLL2/32\n
///                         2 (ADC_CLK_SEL_PLL2 | ADC_CLK_DIV_64)   - PLL2/64\n
///                         3 (ADC_CLK_SEL_PLL2 | ADC_CLK_DIV_128)  - PLL2/128\n
///                         4 (ADC_CLK_SEL_PLL2 | ADC_CLK_DIV_256)  - PLL2/256\n
///                         5 (ADC_CLK_SEL_PLL2 | ADC_CLK_DIV_512)  - PLL2/512\n
///                         6 (ADC_CLK_SEL_PLL2 | ADC_CLK_DIV_1024) - PLL2/1024\n
///                         7 (ADC_CLK_SEL_PLL2 | ADC_CLK_DIV_2048) - PLL2/2048\n
///                         8 (ADC_CLK_SEL_PLL3 | ADC_CLK_DIV_16)   - PLL3/16\n
///                         9 (ADC_CLK_SEL_PLL3 | ADC_CLK_DIV_32)   - PLL3/32\n
///                         10 (ADC_CLK_SEL_PLL3 | ADC_CLK_DIV_64)  - PLL3/64\n
///                         11 (ADC_CLK_SEL_PLL3 | ADC_CLK_DIV_128) - PLL3/128\n
///                         12 (ADC_CLK_SEL_PLL3 | ADC_CLK_DIV_256) - PLL3/256\n
///                         13 (ADC_CLK_SEL_PLL3 | ADC_CLK_DIV_512) - PLL3/512\n
///                         14 (ADC_CLK_SEL_PLL3 | ADC_CLK_DIV_1024)- PLL3/1024\n
///                         15 (ADC_CLK_SEL_PLL3 | ADC_CLK_DIV_2048)- PLL3/2048\n
///
///@retval      NO_ERR      No any error
///@retval      -1          Wrong Group Index
///
///@remark
///
SDWORD EmbededAdc_Enable(E_ADC_GROUP_INDEX group, DWORD clkSel);

///
///@ingroup     ADC_MODULE
///@brief       Disable Embedded ADC (Keypad)
///
///@param       group       ADC group index, see E_ADC_GROUP_INDEX
///
///@retval      NO_ERR      No any error
///@retval      -1          Wrong Group Index
///
///@remark
///
SDWORD EmbededAdc_Disable(E_ADC_GROUP_INDEX group);

///
///@ingroup     ADC_MODULE
///@brief       Get value of Embedded ADC (Keypad)
///
///@param       group       ADC group index, see E_ADC_GROUP_INDEX
///
///@retval      >= 0        ADC value (0 ~ 15)
///@retval      -1          Wrong ADC Index
///
///@remark
///
SDWORD EmbededAdc_ValueGet(E_ADC_GROUP_INDEX group);

SDWORD EmbededAdc_LPF_Initial(E_ADC_GROUP_INDEX group, BOOL enable, BYTE defaultValue);

#endif  // #if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

#endif  // __HAL_ADC_H

