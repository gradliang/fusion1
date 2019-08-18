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
* Filename      : GPIO_MP650.h
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : Supported for MP650/660
*******************************************************************************
*/

#ifndef __LIB_MP650_GPIO_H
#define __LIB_MP650_GPIO_H

#include "iplaysysconfig.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

/*
// Constant declarations
*/
///
///@ingroup     GPIO_MODULE
///@brief       define GPIO's pin name for normal purpose
typedef enum
{
    GPIO_GPIO_START = HAL_GPIO_GROUP_INDEX,
    GPIO_GPIO_0 = GPIO_GPIO_START,          ///< GPIO-0
    GPIO_GPIO_1,                            ///< GPIO-1
    GPIO_GPIO_2,                            ///< GPIO-2
    GPIO_GPIO_3,                            ///< GPIO-3
    GPIO_GPIO_4,                            ///< GPIO-4
    GPIO_GPIO_5,                            ///< GPIO-5
#if (CHIP_VER_MSB == CHIP_VER_650)
    GPIO_GPIO_6,                            ///< GPIO-6
    GPIO_GPIO_7,                            ///< GPIO-7
    GPIO_RGPIN,                             ///< RGPIN
#else   // MP663
    GPIO_GPIO_12,                           ///< GPIO-12
    GPIO_GPIO_13,                           ///< GPIO-13
#endif
    GPIO_GPIO_END,

    GPIO_FGPIO_START = HAL_FGPIO_GROUP_INDEX + (GPIO_GPIO_END & HAL_GPIO_NUMBER_MASK),
    GPIO_FGPIO_0 = GPIO_FGPIO_START,        ///< FGPIO-0
    GPIO_FGPIO_1,                           ///< FGPIO-1
    GPIO_FGPIO_2,                           ///< FGPIO-2
    GPIO_FGPIO_3,                           ///< FGPIO-3
    GPIO_FGPIO_4,                           ///< FGPIO-4
    GPIO_FGPIO_5,                           ///< FGPIO-5
    GPIO_FGPIO_6,                           ///< FGPIO-6
    GPIO_FGPIO_7,                           ///< FGPIO-7
    GPIO_FGPIO_8,                           ///< FGPIO-8
    GPIO_FGPIO_9,                           ///< FGPIO-9
    GPIO_FGPIO_10,                          ///< FGPIO-10
    GPIO_FGPIO_11,                          ///< FGPIO-11
    GPIO_FGPIO_12,                          ///< FGPIO-12
    GPIO_FGPIO_13,                          ///< FGPIO-13
    GPIO_FGPIO_14,                          ///< FGPIO-14
    GPIO_FGPIO_15,                          ///< FGPIO-15
    GPIO_FGPIO_16,                          ///< FGPIO-16
    GPIO_FGPIO_17,                          ///< FGPIO-17
    GPIO_FGPIO_18,                          ///< FGPIO-18
    GPIO_FGPIO_19,                          ///< FGPIO-19
    GPIO_FGPIO_20,                          ///< FGPIO-20
    GPIO_FGPIO_21,                          ///< FGPIO-21
    GPIO_FGPIO_22,                          ///< FGPIO-22
    GPIO_FGPIO_23,                          ///< FGPIO-23
    GPIO_FGPIO_24,                          ///< FGPIO-24
    GPIO_FGPIO_25,                          ///< FGPIO-25
    GPIO_FGPIO_26,                          ///< FGPIO-26
    GPIO_FGPIO_27,                          ///< FGPIO-27
    GPIO_FGPIO_28,                          ///< FGPIO-28
    GPIO_FGPIO_29,                          ///< FGPIO-29
    GPIO_FGPIO_30,                          ///< FGPIO-30
    GPIO_FGPIO_31,                          ///< FGPIO-31
    GPIO_FGPIO_32,                          ///< FGPIO-32
    GPIO_FGPIO_33,                          ///< FGPIO-33
    GPIO_FGPIO_34,                          ///< FGPIO-34
    GPIO_FGPIO_35,                          ///< FGPIO-35
    GPIO_FGPIO_36,                          ///< FGPIO-36
    GPIO_FGPIO_37,                          ///< FGPIO-37
    GPIO_FGPIO_38,                          ///< FGPIO-38
    GPIO_FGPIO_39,                          ///< FGPIO-39
    GPIO_FGPIO_40,                          ///< FGPIO-40
    GPIO_FGPIO_41,                          ///< FGPIO-41
    GPIO_FGPIO_42,                          ///< FGPIO-42
    GPIO_FGPIO_43,                          ///< FGPIO-43
    GPIO_FGPIO_44,                          ///< FGPIO-44
    GPIO_FGPIO_45,                          ///< FGPIO-45
    GPIO_FGPIO_END,

    // UGPIO0~3 just only for FPGA
    GPIO_UGPIO_START = HAL_UGPIO_GROUP_INDEX + (GPIO_FGPIO_END & HAL_GPIO_NUMBER_MASK),
#if (CHIP_VER == (CHIP_VER_650 | CHIP_VER_FPGA))
    GPIO_UGPIO_0 = GPIO_UGPIO_START,        ///< UGPIO-0
    GPIO_UGPIO_1,                           ///< UGPIO-1
    GPIO_UGPIO_2,                           ///< UGPIO-2
    GPIO_UGPIO_3,                           ///< UGPIO-3
    GPIO_UGPIO_4,                           ///< UGPIO-4
    GPIO_UGPIO_5,                           ///< UGPIO-5
    GPIO_UGPIO_6,                           ///< UGPIO-6
    GPIO_UGPIO_7,                           ///< UGPIO-7
#elif (CHIP_VER_MSB == CHIP_VER_650)
    GPIO_UGPIO_4 = GPIO_UGPIO_START,        ///< UGPIO-4
    GPIO_UGPIO_5,                           ///< UGPIO-5
    GPIO_UGPIO_6,                           ///< UGPIO-6
    GPIO_UGPIO_7,                           ///< UGPIO-7
#else
    GPIO_UGPIO_0 = GPIO_UGPIO_START,        ///< UGPIO-0
    GPIO_UGPIO_1,                           ///< UGPIO-1
#endif
    GPIO_UGPIO_END,

    GPIO_VGPIO_START = HAL_VGPIO_GROUP_INDEX + (GPIO_UGPIO_END & HAL_GPIO_NUMBER_MASK),
    GPIO_VGPIO_0 = GPIO_VGPIO_START,        ///< VGPIO-0
    GPIO_VGPIO_1,                           ///< VGPIO-1
    GPIO_VGPIO_2,                           ///< VGPIO-2
    GPIO_VGPIO_3,                           ///< VGPIO-3
    GPIO_VGPIO_4,                           ///< VGPIO-4
    GPIO_VGPIO_5,                           ///< VGPIO-5
    GPIO_VGPIO_6,                           ///< VGPIO-6
    GPIO_VGPIO_7,                           ///< VGPIO-7
    GPIO_VGPIO_8,                           ///< VGPIO-8
    GPIO_VGPIO_9,                           ///< VGPIO-9
    GPIO_VGPIO_10,                          ///< VGPIO-10
    GPIO_VGPIO_11,                          ///< VGPIO-11
    GPIO_VGPIO_12,                          ///< VGPIO-12
    GPIO_VGPIO_13,                          ///< VGPIO-13
    GPIO_VGPIO_14,                          ///< VGPIO-14
    GPIO_VGPIO_15,                          ///< VGPIO-15
    GPIO_VGPIO_16,                          ///< VGPIO-16
    GPIO_VGPIO_17,                          ///< VGPIO-17
    GPIO_VGPIO_18,                          ///< VGPIO-18
    GPIO_VGPIO_19,                          ///< VGPIO-19
    GPIO_VGPIO_20,                          ///< VGPIO-20
    GPIO_VGPIO_21,                          ///< VGPIO-21
    GPIO_VGPIO_22,                          ///< VGPIO-22
    GPIO_VGPIO_23,                          ///< VGPIO-23
    GPIO_VGPIO_24,                          ///< VGPIO-24
    GPIO_VGPIO_25,                          ///< VGPIO-25
    GPIO_VGPIO_26,                          ///< VGPIO-26
    GPIO_VGPIO_27,                          ///< VGPIO-27
#if (CHIP_VER_MSB == CHIP_VER_650)
    GPIO_VGPIO_28,                          ///< VGPIO-28
#endif
    GPIO_VGPIO_END,

    GPIO_AGPIO_START = HAL_AGPIO_GROUP_INDEX + (GPIO_VGPIO_END & HAL_GPIO_NUMBER_MASK),
    GPIO_AGPIO_0 = GPIO_AGPIO_START,        ///< AGPIO-0
    GPIO_AGPIO_1,                           ///< AGPIO-1
    GPIO_AGPIO_2,                           ///< AGPIO-2
    GPIO_AGPIO_3,                           ///< AGPIO-3
    GPIO_AGPIO_END,

    GPIO_OGPIO_START = HAL_OGPIO_GROUP_INDEX + (GPIO_AGPIO_END & HAL_GPIO_NUMBER_MASK),
    GPIO_OGPIO_0 = GPIO_OGPIO_START,        ///< OGPIO-0
#if (CHIP_VER_MSB == CHIP_VER_650)
    GPIO_OGPIO_1,                           ///< 0GPIO-1
#endif
    GPIO_OGPIO_END,

    GPIO_PGPIO_START = HAL_PGPIO_GROUP_INDEX + (GPIO_OGPIO_END & HAL_GPIO_NUMBER_MASK),
    GPIO_PGPIO_0 = GPIO_PGPIO_START,        ///< PGPIO-0
    GPIO_PGPIO_1,                           ///< PGPIO-1
    GPIO_PGPIO_2,                           ///< PGPIO-2
    GPIO_PGPIO_3,                           ///< PGPIO-3
    GPIO_PGPIO_END,

    GPIO_KGPIO_START = HAL_KGPIO_GROUP_INDEX + (GPIO_PGPIO_END & HAL_GPIO_NUMBER_MASK),
    GPIO_KGPIO_0 = GPIO_KGPIO_START,        ///< KGPIO-0
    GPIO_KGPIO_1,                           ///< KGPIO-1
#if (CHIP_VER_MSB == CHIP_VER_650)
    GPIO_KGPIO_2,                           ///< KGPIO-2
    GPIO_KGPIO_3,                           ///< KGPIO-3
#endif
    GPIO_KGPIO_END,

    MAX_GPIO_GROUP_INDEX
} E_GPIO_GROUP_INDEX;

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO's pin name for interrupt
typedef enum
{
    GPINT_GPIO_START = HAL_GPIO_GROUP_INDEX,
    GPINT_GPIO_0 = GPINT_GPIO_START,            ///< GPIO-0
    GPINT_GPIO_1,                               ///< GPIO-1
    GPINT_GPIO_2,                               ///< GPIO-2
    GPINT_GPIO_3,                               ///< GPIO-3
    GPINT_GPIO_4,                               ///< GPIO-4
    GPINT_GPIO_5,                               ///< GPIO-5
    GPINT_GPIO_6,                               ///< GPIO-6
    GPINT_GPIO_7,                               ///< GPIO-7
    GPINT_RGPIN,                                ///< RGPIN (RALARM)
    GPINT_GPIO_END,

    GPINT_FGPIO_START = HAL_FGPIO_GROUP_INDEX + (GPINT_GPIO_END & HAL_GPIO_NUMBER_MASK),
    GPINT_CF_CARD_DETECT = GPINT_FGPIO_START,   ///< FGPIO-26
    GPINT_XD_CARD_DETECT,                       ///< FGPIO-27
    GPINT_SD_CARD_DETECT,                       ///< FGPIO-28
    GPINT_MS_CARD_DETECT,                       ///< FGPIO-29
    GPINT_SDIO_CARD_DETECT,                     ///< FGPIO-43
    GPINT_FGPIO_END,

    MAX_GPIO_INT_GROUP_INDEX
} E_GPIO_INT_GROUP_INDEX;

#define GPIO_INT_COUNT          (GPINT_GPIO_END - GPINT_GPIO_START) //9
#define GPIO_INT_OFFSET         0

#define FGPIO_INT_COUNT         (GPINT_FGPIO_END - GPINT_FGPIO_START) //5
#define FGPIO_INT_OFFSET        (GPIO_INT_OFFSET + GPIO_INT_COUNT)

#define TOTAL_GPIO_INT_COUNT    (GPIO_INT_COUNT + FGPIO_INT_COUNT)

///
///@ingroup     GPIO_MODULE
///@brief       define int code (4) for SDIO/SD2's CD when chip version is \b MP650.
#define SDIO_CD_INT_CODE    (GPINT_SDIO_CARD_DETECT - GPINT_FGPIO_START)

///
///@ingroup     GPIO_MODULE
///@brief       define int code (2) for SD/MMC's CD when chip version is \b MP650.
#define SD_CD_INT_CODE      (GPINT_SD_CARD_DETECT - GPINT_FGPIO_START)

///
///@ingroup     GPIO_MODULE
///@brief       define int code (3) for MS's CD when chip version is \b MP650.
#define MS_CD_INT_CODE      (GPINT_MS_CARD_DETECT - GPINT_FGPIO_START)

///
///@ingroup     GPIO_MODULE
///@brief       define int code (1) for XD's CD when chip version is \b MP650.
#define XD_CD_INT_CODE      (GPINT_XD_CARD_DETECT - GPINT_FGPIO_START)

///
///@ingroup     GPIO_MODULE
///@brief       define int code (0) for CF's CD when chip version is \b MP650.
#define CF_CD_INT_CODE      (GPINT_CF_CARD_DETECT - GPINT_FGPIO_START)

#endif      // #if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

#endif      // #ifndef __LIB_MP650_GPIO_H

