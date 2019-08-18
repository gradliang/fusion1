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
* Filename      : hal_gpio.c
* Programmer(s) : TY Miao
* Created       : TY Miao
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
#include "hal_gpio.h"

/*
// Define declarations
*/
#define GPIO_CFG_REG_OFFSET         (0x40 >> 2)

static void gpioDefaultInt(WORD);
static void gpioIsr(void);
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
static void fgpioIsr(void);
#endif

static void (*gpioIntFunPtrArray[TOTAL_GPIO_INT_COUNT])(WORD) = {gpioDefaultInt};
static BOOL gpioIntAlreadyInitialed = FALSE;

/*
// Struct declarations
*/
typedef struct
{
    DWORD dwGroupAddr;
    BYTE bBitOffset;
    BYTE bGpDir          : 1;
    BYTE bGpDat          : 1;
    BYTE bGpCfg          : 2;
    //BYTE bGpIntMode      : 1;
    //BYTE bGpIntPolarity  : 1;
    //BYTE bGpIntEnable    : 1;
    BYTE reserved[2];
} ST_GPIO_CONFIG_INFO;


typedef struct
{
    DWORD dwGroupAddr;
    BYTE bBitOffset;
    BYTE bGpIntMode      : 1;
    BYTE bGpIntPolarity  : 1;
    BYTE bGpIntEnable    : 1;
    BYTE reserved[2];
} ST_GPIO_INT_CONFIG_INFO;

/////////////////////////////////////////////
///    DWORD dwGroupAddr;
///    BYTE bBitOffset;
///    BYTE bGpDir          : 1;
///    BYTE bGpDat          : 1;
///    BYTE bGpCfg          : 2;
static const ST_GPIO_CONFIG_INFO stGpioConfigInfo[(MAX_GPIO_GROUP_INDEX - 1) & HAL_GPIO_NUMBER_MASK] =
{
    //dwGroupAddr,                             Offset, Dir,    GpDat,  GpCfg
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     0,      0,      1,      0}, // GPIO_0
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     1,      0,      1,      0}, // GPIO_1
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     2,      0,      1,      0}, // GPIO_2
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     3,      0,      1,      0}, // GPIO_3
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     4,      0,      1,      0}, // GPIO_4
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     5,      0,      1,      0}, // GPIO_5
#if (CHIP_VER_MSB != CHIP_VER_660)
    // MP615 and MP650
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     6,      0,      1,      0}, // GPIO_6
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     7,      0,      1,      0}, // GPIO_7

    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     8,      0,      1,      0}, // RGPIN (RALARM)
#else
    // MP660
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     12,     0,      1,      0}, // GPIO_12
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     13,     0,      1,      0}, // GPIO_13
#endif
#if (CHIP_VER_MSB == CHIP_VER_615)
    // MP615
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     9,      0,      1,      0}, // GPIO_9
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     10,     0,      1,      0}, // GPIO_10
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     11,     0,      1,      0}, // GPIO_11
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     12,     0,      1,      0}, // GPIO_12
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     13,     0,      1,      0}, // GPIO_13
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     14,     0,      1,      0}, // GPIO_14
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat0),     15,     0,      1,      0}, // GPIO_15

    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     0,      0,      1,      0}, // GPIO_16
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     1,      0,      1,      0}, // GPIO_17
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     2,      0,      1,      0}, // GPIO_18
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     3,      0,      1,      0}, // GPIO_19
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     4,      0,      1,      0}, // GPIO_20
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     5,      0,      1,      0}, // GPIO_21
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     6,      0,      1,      0}, // GPIO_22
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     7,      0,      1,      0}, // GPIO_23

    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     8,      0,      1,      0}, // GPIO_24
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     9,      0,      1,      0}, // GPIO_25
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     10,     0,      1,      0}, // GPIO_26
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     11,     0,      1,      0}, // GPIO_27
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     12,     0,      1,      0}, // GPIO_28
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     13,     0,      1,      0}, // GPIO_29
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     14,     0,      1,      0}, // GPIO_30
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpdat1),     15,     0,      1,      0}, // GPIO_31
#endif
    //dwGroupAddr,                             Offset, Dir,    GpDat,  GpCfg
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  0,      0,      0,      0}, // FGPIO_0
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  1,      0,      0,      0}, // FGPIO_1
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  2,      0,      0,      0}, // FGPIO_2
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  3,      0,      0,      0}, // FGPIO_3
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  4,      0,      0,      0}, // FGPIO_4
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  5,      0,      0,      0}, // FGPIO_5
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  6,      0,      0,      0}, // FGPIO_6
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  7,      0,      0,      0}, // FGPIO_7

    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  8,      0,      0,      0}, // FGPIO_8
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  9,      0,      0,      0}, // FGPIO_9
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  10,     0,      0,      0}, // FGPIO_10
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  11,     0,      0,      0}, // FGPIO_11
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  12,     0,      0,      0}, // FGPIO_12
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  13,     0,      0,      0}, // FGPIO_13
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  14,     0,      0,      0}, // FGPIO_14
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[0]),  15,     0,      0,      0}, // FGPIO_15

    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  0,      0,      0,      0}, // FGPIO_16
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  1,      0,      0,      0}, // FGPIO_17
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  2,      0,      0,      0}, // FGPIO_18
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  3,      0,      0,      0}, // FGPIO_19
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  4,      0,      0,      0}, // FGPIO_20
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  5,      0,      0,      0}, // FGPIO_21
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  6,      0,      0,      0}, // FGPIO_22
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  7,      0,      0,      0}, // FGPIO_23

    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  8,      0,      0,      0}, // FGPIO_24
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  9,      0,      0,      0}, // FGPIO_25
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  10,     0,      0,      0}, // FGPIO_26
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  11,     0,      0,      0}, // FGPIO_27
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  12,     0,      0,      0}, // FGPIO_28
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  13,     0,      0,      0}, // FGPIO_29
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  14,     0,      0,      0}, // FGPIO_30
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[1]),  15,     0,      0,      0}, // FGPIO_31

    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  0,      0,      0,      0}, // FGPIO_32
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  1,      0,      0,      0}, // FGPIO_33
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  2,      0,      0,      0}, // FGPIO_34
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  3,      0,      0,      0}, // FGPIO_35
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  4,      0,      0,      0}, // FGPIO_36
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  5,      0,      0,      0}, // FGPIO_37
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  6,      0,      0,      0}, // FGPIO_38
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  7,      0,      0,      0}, // FGPIO_39

    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  8,      0,      0,      0}, // FGPIO_40
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  9,      0,      0,      0}, // FGPIO_41
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  10,     0,      0,      0}, // FGPIO_42
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  11,     0,      0,      0}, // FGPIO_43
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  12,     0,      0,      0}, // FGPIO_44
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  13,     0,      0,      0}, // FGPIO_45
#if (CHIP_VER_MSB == CHIP_VER_615)
    // MP615
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  14,     0,      0,      0}, // FGPIO_46
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[2]),  15,     0,      0,      0}, // FGPIO_47

    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  0,      0,      0,      0}, // FGPIO_48
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  1,      0,      0,      0}, // FGPIO_49
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  2,      0,      0,      0}, // FGPIO_50
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  3,      0,      0,      0}, // FGPIO_51
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  4,      0,      0,      0}, // FGPIO_52
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  5,      0,      0,      0}, // FGPIO_53
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  6,      0,      0,      0}, // FGPIO_54
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  7,      0,      0,      0}, // FGPIO_55

    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  8,      0,      0,      0}, // FGPIO_56
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  9,      0,      0,      0}, // FGPIO_57
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  10,     0,      0,      0}, // FGPIO_58
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  11,     0,      0,      0}, // FGPIO_59
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  12,     0,      0,      0}, // FGPIO_60
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  13,     0,      0,      0}, // FGPIO_61
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  14,     0,      0,      0}, // FGPIO_62
    {(DWORD)&(((GPIO *)GPIO_BASE)->Fgpdat[3]),  15,     0,      0,      0}, // FGPIO_63
#endif

    //dwGroupAddr,                             Offset, Dir,    GpDat,  GpCfg
#if ( (CHIP_VER_MSB == CHIP_VER_615) || (CHIP_VER == (CHIP_VER_650 | CHIP_VER_FPGA)) )
    // MP615
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     0,      0,      0,      0}, // UGPIO_0
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     1,      0,      0,      0}, // UGPIO_1
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     2,      0,      0,      0}, // UGPIO_2
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     3,      0,      0,      0}, // UGPIO_3
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     4,      0,      0,      0}, // UGPIO_4
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     5,      0,      0,      0}, // UGPIO_5
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     6,      0,      0,      0}, // UGPIO_6
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     7,      0,      0,      0}, // UGPIO_7
#elif (CHIP_VER_MSB == CHIP_VER_650)
    // MP650
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     4,      0,      0,      0}, // UGPIO_4
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     5,      0,      0,      0}, // UGPIO_5
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     6,      0,      0,      0}, // UGPIO_6
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     7,      0,      0,      0}, // UGPIO_7
#else
    // MP660
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     0,      0,      0,      0}, // UGPIO_0
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ugpdat),     1,      0,      0,      0}, // UGPIO_1
#endif

    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    0,      0,      0,      0}, // VGPIO_0
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    1,      0,      0,      0}, // VGPIO_1
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    2,      0,      0,      0}, // VGPIO_2
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    3,      0,      0,      0}, // VGPIO_3
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    4,      0,      0,      0}, // VGPIO_4
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    5,      0,      0,      0}, // VGPIO_5
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    6,      0,      0,      0}, // VGPIO_6
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    7,      0,      0,      0}, // VGPIO_7

    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    8,      0,      0,      0}, // VGPIO_8
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    9,      0,      0,      0}, // VGPIO_9
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    10,     0,      0,      0}, // VGPIO_10
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    11,     0,      0,      0}, // VGPIO_11
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    12,     0,      0,      0}, // VGPIO_12
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    13,     0,      0,      0}, // VGPIO_13
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    14,     0,      0,      0}, // VGPIO_14
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat0),    15,     0,      0,      0}, // VGPIO_15

    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    0,      0,      0,      0}, // VGPIO_16
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    1,      0,      0,      0}, // VGPIO_17
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    2,      0,      0,      0}, // VGPIO_18
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    3,      0,      0,      0}, // VGPIO_19
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    4,      0,      0,      0}, // VGPIO_20
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    5,      0,      0,      0}, // VGPIO_21
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    6,      0,      0,      0}, // VGPIO_22
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    7,      0,      0,      0}, // VGPIO_23

    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    8,      0,      0,      0}, // VGPIO_24
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    9,      0,      0,      0}, // VGPIO_25
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    10,     0,      0,      0}, // VGPIO_26
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    11,     0,      0,      0}, // VGPIO_27
#if (CHIP_VER_MSB == CHIP_VER_650)
    // MP650
    {(DWORD)&(((GPIO *)GPIO_BASE)->Vgpdat1),    12,     0,      0,      0}, // VGPIO_28
#endif

    {(DWORD)&(((GPIO *)GPIO_BASE)->Agpdat),     0,      0,      0,      0}, // AGPIO_0
    {(DWORD)&(((GPIO *)GPIO_BASE)->Agpdat),     1,      0,      0,      0}, // AGPIO_1
    {(DWORD)&(((GPIO *)GPIO_BASE)->Agpdat),     2,      0,      0,      0}, // AGPIO_2
    {(DWORD)&(((GPIO *)GPIO_BASE)->Agpdat),     3,      0,      0,      0}, // AGPIO_3

    //dwGroupAddr,                             Offset, Dir,    GpDat,  GpCfg
#if (CHIP_VER_MSB == CHIP_VER_615)
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    0,      0,      0,      0}, // MGPIO_0
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    1,      0,      0,      0}, // MGPIO_1
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    2,      0,      0,      0}, // MGPIO_2
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    3,      0,      0,      0}, // MGPIO_3
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    4,      0,      0,      0}, // MGPIO_4
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    5,      0,      0,      0}, // MGPIO_5
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    6,      0,      0,      0}, // MGPIO_6
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    7,      0,      0,      0}, // MGPIO_7

    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    8,      0,      0,      0}, // MGPIO_8
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    9,      0,      0,      0}, // MGPIO_9
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    10,     0,      0,      0}, // MGPIO_10
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    11,     0,      0,      0}, // MGPIO_11
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    12,     0,      0,      0}, // MGPIO_12
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    13,     0,      0,      0}, // MGPIO_13
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    14,     0,      0,      0}, // MGPIO_14
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpdat0),    15,     0,      0,      0}, // MGPIO_15
#else   // MP65x/66x
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ogpdat),     0,      0,      0,      1}, // OGPIO_0
#if (CHIP_VER_MSB == CHIP_VER_650)
    // MP650
    {(DWORD)&(((GPIO *)GPIO_BASE)->Ogpdat),     1,      0,      0,      1}, // OGPIO_1
#endif
    {(DWORD)&(((GPIO *)GPIO_BASE)->Pgpdat),     0,      0,      0,      1}, // PGPIO_0
    {(DWORD)&(((GPIO *)GPIO_BASE)->Pgpdat),     1,      0,      0,      1}, // PGPIO_1
    {(DWORD)&(((GPIO *)GPIO_BASE)->Pgpdat),     2,      0,      0,      1}, // PGPIO_2
    {(DWORD)&(((GPIO *)GPIO_BASE)->Pgpdat),     3,      0,      0,      1}, // PGPIO_3

    {(DWORD)&(((GPIO *)GPIO_BASE)->Kgpdat),     0,      0,      0,      0}, // KGPIO_0
    {(DWORD)&(((GPIO *)GPIO_BASE)->Kgpdat),     1,      0,      0,      0}, // KGPIO_1
#if (CHIP_VER_MSB == CHIP_VER_650)
    // MP650
    {(DWORD)&(((GPIO *)GPIO_BASE)->Kgpdat),     2,      0,      0,      0}, // KGPIO_2
    {(DWORD)&(((GPIO *)GPIO_BASE)->Kgpdat),     3,      0,      0,      0}, // KGPIO_3
#endif
#endif
};

static SDWORD checkGpioPinVaild(E_GPIO_GROUP_INDEX gpioNum)
{
    DWORD pinGroup;

    pinGroup = gpioNum & HAL_GPIO_GROUP_MASK;

    switch (pinGroup)
    {
    case HAL_GPIO_GROUP_INDEX:
        if ((gpioNum < GPIO_GPIO_START) || (gpioNum >= GPIO_GPIO_END))
        {
            MP_ALERT("--E-- checkGpioIntVaild - GPIO pin number exceed max GPIO pin - 0x%08X", gpioNum);

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;

    case HAL_FGPIO_GROUP_INDEX:
        if ((gpioNum < GPIO_FGPIO_START) || (gpioNum >= GPIO_FGPIO_END))
        {
            MP_ALERT("--E-- checkGpioIntVaild - FGPIO pin number exceed max FGPIO pin - 0x%08X", gpioNum);

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;

    case HAL_UGPIO_GROUP_INDEX:
        if ((gpioNum < GPIO_UGPIO_START) || (gpioNum >= GPIO_UGPIO_END))
        {
            MP_ALERT("--E-- checkGpioIntVaild - UGPIO pin number exceed max UGPIO pin - 0x%08X", gpioNum);

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;

    case HAL_VGPIO_GROUP_INDEX:
        if ((gpioNum < GPIO_VGPIO_START) || (gpioNum >= GPIO_VGPIO_END))
        {
            MP_ALERT("--E-- checkGpioIntVaild - VGPIO pin number exceed max VGPIO pin - 0x%08X", gpioNum);

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;

    case HAL_AGPIO_GROUP_INDEX:
        if ((gpioNum < GPIO_AGPIO_START) || (gpioNum >= GPIO_AGPIO_END))
        {
            MP_ALERT("--E-- checkGpioIntVaild - AGPIO pin number exceed max AGPIO pin - 0x%08X", gpioNum);

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    case HAL_OGPIO_GROUP_INDEX:
        if ((gpioNum < GPIO_OGPIO_START) || (gpioNum >= GPIO_OGPIO_END))
        {
            MP_ALERT("--E-- checkGpioIntVaild - OGPIO pin number exceed max OGPIO pin - 0x%08X", gpioNum);

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;

    case HAL_PGPIO_GROUP_INDEX:
        if ((gpioNum < GPIO_PGPIO_START) || (gpioNum >= GPIO_PGPIO_END))
        {
            MP_ALERT("--E-- checkGpioIntVaild - PGPIO pin number exceed max PGPIO pin - 0x%08X", gpioNum);

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;

    case HAL_KGPIO_GROUP_INDEX:
        if ((gpioNum < GPIO_KGPIO_START) || (gpioNum >= GPIO_KGPIO_END))
        {
            MP_ALERT("--E-- checkGpioIntVaild - KGPIO pin number exceed max KGPIO pin - 0x%08X", gpioNum);

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;
#endif

#if (CHIP_VER_MSB == CHIP_VER_615)
    case HAL_MGPIO_GROUP_INDEX:
        if ((gpioNum < GPIO_MGPIO_START) || (gpioNum >= GPIO_MGPIO_END))
        {
            MP_ALERT("--E-- checkGpioIntVaild - MGPIO pin number exceed max MGPIO pin - 0x%08X", gpioNum);

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;
#endif

    default:
        MP_ALERT("--E-- checkGpioIntVaild - GPIO pin group exceed max GPIO pin group - 0x%08X", gpioNum);

        return ERR_GPIO_INVALID_NUMBER;
    }

    return NO_ERR;

}


SDWORD Gpio_DefaultSettingSet(E_GPIO_GROUP_INDEX gpioNum, BYTE numPins)
{
    volatile DWORD *groupAddr;
    DWORD bitOffset;
    DWORD bitMask, defDir, defDat, defCfg;
    BYTE i;

    MP_DEBUG("Gpio_DefaultSettingSet -");

    if ( checkGpioPinVaild(gpioNum) != NO_ERR )
        return ERR_GPIO_INVALID_NUMBER;

    if (numPins == 0)
        return ERR_GPIO_ZERO_PIN_NUM;

    gpioNum &= HAL_GPIO_NUMBER_MASK;
    groupAddr = (DWORD *) stGpioConfigInfo[gpioNum].dwGroupAddr;
    bitOffset = (DWORD) stGpioConfigInfo[gpioNum].bBitOffset;

    defDir = stGpioConfigInfo[gpioNum].bGpDir;
    defDat = stGpioConfigInfo[gpioNum].bGpDat;
    defCfg = ((stGpioConfigInfo[gpioNum].bGpCfg & BIT1) << 15) |
              (stGpioConfigInfo[gpioNum].bGpCfg & BIT0);

    if (numPins > 16)
        numPins = 16;

    bitMask = 1;

    for (i = 1; i < numPins; i++)
    {
        bitMask = (bitMask << 1) | BIT0;
        defDir |= (stGpioConfigInfo[gpioNum + i].bGpDir << i);
        defDat |= (stGpioConfigInfo[gpioNum + i].bGpDat << i);
        defCfg |= ((stGpioConfigInfo[gpioNum + i].bGpCfg & BIT1) << (15 + i)) |
                  ((stGpioConfigInfo[gpioNum + i].bGpCfg & BIT0) << i);
    }

    bitMask = (bitMask << bitOffset) & 0x0000FFFF;
    defDir = ((defDir << bitOffset) & bitMask) << 16;
    defDat = (defDat << bitOffset) & bitMask;
    defCfg = (defCfg << bitOffset) & ((bitMask << 16) | bitMask);

    IntDisable();
    // Clear and Set "xGPDAT" register bit
    *groupAddr = ( (*groupAddr) & ~((bitMask << 16) | bitMask) ) | defDir | defDat;
    // Clear and Set "xGPCFG" register bit
    *(groupAddr + GPIO_CFG_REG_OFFSET) = ( (*(groupAddr + GPIO_CFG_REG_OFFSET)) & ~((bitMask << 16) | bitMask) ) | defCfg;
    IntEnable();

    return NO_ERR;
}



SDWORD Gpio_DataSet(E_GPIO_GROUP_INDEX gpioNum, WORD outData, BYTE numPins)
{
    volatile DWORD *groupAddr;
    DWORD bitOffset;
    DWORD bitMask;
    BYTE i;

    MP_DEBUG("Gpio_DataSet -");

    if ( checkGpioPinVaild(gpioNum) != NO_ERR )
        return ERR_GPIO_INVALID_NUMBER;

    if (numPins == 0)
        return ERR_GPIO_ZERO_PIN_NUM;

    gpioNum &= HAL_GPIO_NUMBER_MASK;
    bitOffset = stGpioConfigInfo[gpioNum].bBitOffset;
    groupAddr = (DWORD *) stGpioConfigInfo[gpioNum].dwGroupAddr;

    if (numPins > 16)
        numPins = 16;

    bitMask = 1;

    for (i = 1; i < numPins; i++)
    {
        bitMask = (bitMask << 1) | BIT0;
    }

    bitMask = (bitMask << bitOffset) & 0x0000FFFF;
    outData = (outData << bitOffset) & bitMask;

    IntDisable();
    *groupAddr = (*groupAddr & ~bitMask) | (bitMask << 16) | outData;
    //*groupAddr = (*groupAddr & ~bitMask) | data;

    if ((*groupAddr & bitMask) != outData)
        MP_DEBUG("S %02d 0x%04X 0x%04X", gpioNum, *groupAddr & bitMask, outData);

    IntEnable();

    return NO_ERR;
}



SDWORD Gpio_DataGet(E_GPIO_GROUP_INDEX gpioNum, WORD *inData, BYTE numPins)
{
    volatile DWORD *groupAddr;
    DWORD bitOffset;
    DWORD bitMask;
    BYTE i;

    MP_DEBUG("Gpio_DataGet -");

    if ( checkGpioPinVaild(gpioNum) != NO_ERR )
        return ERR_GPIO_INVALID_NUMBER;

    if (numPins == 0)
        return ERR_GPIO_ZERO_PIN_NUM;

    gpioNum &= HAL_GPIO_NUMBER_MASK;
    bitOffset = stGpioConfigInfo[gpioNum].bBitOffset;
    groupAddr = (DWORD *) stGpioConfigInfo[gpioNum].dwGroupAddr;

    if (numPins > 16)
        numPins = 16;

    bitMask = 1;

    for (i = 1; i < numPins; i++)
        bitMask = (bitMask << 1) | BIT0;

    bitMask = (bitMask << bitOffset) & 0x0000FFFF;
    //*groupAddr &= ~(bitMask << 16);                 // input mode

    // Set "xGPDAT" register bit
    if (inData)
    {
        *inData = (WORD) ( ((*groupAddr) & bitMask) >> bitOffset );
    }

    return NO_ERR;
}

SWORD Gpio_ValueGet(E_GPIO_GROUP_INDEX gpioNum)
{
	SWORD swRet=-1;
	WORD wData;

    if (Gpio_DataGet(gpioNum, &wData, 1) == NO_ERR)
			swRet=wData;
	return swRet;

}

SDWORD Gpio_ConfiguraionSet(E_GPIO_GROUP_INDEX gpioNum, DWORD conf, BYTE direction, WORD outData, BYTE numPins)
{
    volatile DWORD *groupAddr;
    DWORD bitOffset;
    DWORD bitMask, defCfg;
    BYTE i;

    MP_DEBUG("Gpio_ConfiguraionSet -");

    if ( checkGpioPinVaild(gpioNum) != NO_ERR )
        return ERR_GPIO_INVALID_NUMBER;

    if (numPins == 0)
        return ERR_GPIO_ZERO_PIN_NUM;

#if 1//MINIDV_YBD_FUNCION
	BYTE bGpio11=0;
	if (gpioNum==GPIO_FGPIO_11)
	{
		gpioNum=GPIO_FGPIO_10;//fgpdat11 is dependent of fgpdat10
		bGpio11=1;
	}
#endif

    gpioNum &= HAL_GPIO_NUMBER_MASK;
    bitOffset = stGpioConfigInfo[gpioNum].bBitOffset;
    groupAddr = (DWORD *) stGpioConfigInfo[gpioNum].dwGroupAddr;
    conf = ((conf & BIT1) << 15) | (conf & BIT0);
    defCfg = conf;

    if (numPins > 16)
        numPins = 16;

    bitMask = BIT0;

    for (i = 1; i < numPins; i++)
    {
        bitMask = (bitMask << 1) | BIT0;
        defCfg |= conf << i;
    }

    bitMask = (bitMask << bitOffset) & 0x0000FFFF;
    defCfg = (defCfg << bitOffset) & ((bitMask << 16) | bitMask);
    outData = (outData << bitOffset) & bitMask;

    MP_DEBUG("gpioNum = 0x%08X", gpioNum);
    MP_DEBUG("defCfg = 0x%08X", defCfg);
    MP_DEBUG("bitMask = 0x%08X", (bitMask << 16) | bitMask);
    MP_DEBUG("bitOffset = 0x%08X", bitOffset);

    IntDisable();

    MP_DEBUG("Orginal -");
    MP_DEBUG("Direction Addr = 0x%08X, Configuration Addr = 0x%08X", (DWORD) groupAddr, (DWORD) (groupAddr + GPIO_CFG_REG_OFFSET));
    MP_DEBUG("Direction Value = 0x%08X", *groupAddr);
    MP_DEBUG("Configuration Value = 0x%08X", *(groupAddr + GPIO_CFG_REG_OFFSET));

    if (direction == GPIO_INPUT_MODE)
    {   // input
        *groupAddr = (*groupAddr) & ~(bitMask << 16);
    }
    else
    {   // output
        *groupAddr |= bitMask << 16;
#if 1//MINIDV_YBD_FUNCION
		if (bGpio11)
        *groupAddr = ((*groupAddr) & ~(bitMask<<1)) | (outData<<1);
		else
#endif
        *groupAddr = ((*groupAddr) & ~bitMask) | outData;
    }

    *(groupAddr + GPIO_CFG_REG_OFFSET) = ( (*(groupAddr + GPIO_CFG_REG_OFFSET)) & ~((bitMask << 16) | bitMask) ) | defCfg;

    MP_DEBUG("New -");
    MP_DEBUG("Direction = 0x%08X", *groupAddr);
    MP_DEBUG("Configuration = 0x%08X", *(groupAddr + GPIO_CFG_REG_OFFSET));

    IntEnable();

    return NO_ERR;
}



SDWORD Gpio_Config2GpioFunc(E_GPIO_GROUP_INDEX gpioNum, BYTE direction, WORD outData, BYTE numPins)
{
    return Gpio_ConfiguraionSet(gpioNum, (DWORD) stGpioConfigInfo[gpioNum & HAL_GPIO_NUMBER_MASK].bGpCfg,
                                direction, outData, numPins);
}



SDWORD Gpio_ConfigurationGet(E_GPIO_GROUP_INDEX gpioNum, DWORD *retConf)
{
    volatile DWORD *groupAddr;
    DWORD bitOffset, value;

    MP_DEBUG("Gpio_ConfigurationGet -");

    if (!retConf)
        return NO_ERR;

    if ( checkGpioPinVaild(gpioNum) != NO_ERR )
    {
        MP_ALERT("%s - Invalid GPIO number", __FUNCTION__);

        return ERR_GPIO_INVALID_NUMBER;
    }

    gpioNum &= HAL_GPIO_NUMBER_MASK;
    bitOffset = stGpioConfigInfo[gpioNum].bBitOffset;
    groupAddr = (DWORD *) stGpioConfigInfo[gpioNum].dwGroupAddr;

    value = *(groupAddr + GPIO_CFG_REG_OFFSET);
    *retConf = (DWORD) ( ((value >> (bitOffset + 15)) & BIT1) | ((value >> bitOffset) & BIT0) );

    return NO_ERR;
}



SDWORD Gpio_DirectionGet(E_GPIO_GROUP_INDEX gpioNum, DWORD *retDirection)
{
    volatile DWORD *groupAddr;
    DWORD bitOffset, value;

    MP_DEBUG("Gpio_DirectionGet -");

    if (!retDirection)
        return NO_ERR;

    if ( checkGpioPinVaild(gpioNum) != NO_ERR )
    {
        MP_ALERT("%s - Invalid GPIO number", __FUNCTION__);

        return ERR_GPIO_INVALID_NUMBER;
    }

    gpioNum &= HAL_GPIO_NUMBER_MASK;
    bitOffset = stGpioConfigInfo[gpioNum].bBitOffset;
    groupAddr = (DWORD *) stGpioConfigInfo[gpioNum].dwGroupAddr;

    value = *groupAddr;
    *retDirection = (DWORD) ((value >> (bitOffset + 8)) & BIT0);

    return NO_ERR;
}

/////////////////////////////////////////////////////
// GPIO Interrupt Functions
/////////////////////////////////////////////////////
///    DWORD dwGroupAddr;
///    BYTE bBitOffset;
///    BYTE bGpIntMode      : 1;
///    BYTE bGpIntPolarity  : 1;
///    BYTE bGpIntEnable    : 1;
static const ST_GPIO_INT_CONFIG_INFO GpioIntConfigInfo[(MAX_GPIO_INT_GROUP_INDEX - 1) & HAL_GPIO_NUMBER_MASK] =
{
    //dwGroupAddr,                              Offset, bGpIntMode,    bGpIntPolarity,  bGpIntEnable
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[0]),   0,      0,      0,      0}, // GPIO_0
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[0]),   1,      0,      0,      0}, // GPIO_1
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[0]),   2,      0,      0,      0}, // GPIO_2
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[0]),   3,      0,      0,      0}, // GPIO_3
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[0]),   4,      0,      0,      0}, // GPIO_4
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[0]),   5,      0,      0,      0}, // GPIO_5
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[0]),   6,      0,      0,      0}, // GPIO_6
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[0]),   7,      0,      0,      0}, // GPIO_7
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[1]),   0,      0,      0,      0}, // GPIO_8 for MP615, RGPIN for MP650,660
#if (CHIP_VER_MSB == CHIP_VER_615)
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[1]),   1,      0,      0,      0}, // GPIO_9
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[1]),   2,      0,      0,      0}, // GPIO_10
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[1]),   3,      0,      0,      0}, // GPIO_11
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[1]),   4,      0,      0,      0}, // GPIO_12
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[1]),   5,      0,      0,      0}, // GPIO_13
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[1]),   6,      0,      0,      0}, // GPIO_14
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[1]),   7,      0,      0,      0}, // GPIO_15

    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[2]),   0,      0,      0,      0}, // GPIO_16
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[2]),   1,      1,      1,      0}, // GPIO_17
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[2]),   2,      1,      1,      0}, // GPIO_18
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[2]),   3,      0,      0,      0}, // GPIO_19
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[2]),   4,      0,      0,      0}, // GPIO_20
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[2]),   5,      0,      0,      0}, // GPIO_21
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[2]),   6,      0,      0,      0}, // GPIO_22
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[2]),   7,      0,      0,      0}, // GPIO_23

    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[3]),   0,      0,      0,      0}, // GPIO_24
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[3]),   1,      0,      0,      0}, // GPIO_25
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[3]),   2,      0,      0,      0}, // GPIO_26
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[3]),   3,      0,      0,      0}, // GPIO_27
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[3]),   4,      0,      0,      0}, // GPIO_28
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[3]),   5,      0,      0,      0}, // GPIO_29
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[3]),   6,      0,      0,      0}, // GPIO_30
    {(DWORD)&(((GPIO *)GPIO_BASE)->Gpint[3]),   7,      0,      0,      0}, // GPIO_31
#endif

#if (CHIP_VER_MSB == CHIP_VER_615)
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     0,      0,      0,      0}, // FGPIO_53
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     1,      0,      0,      0}, // FGPIO_54
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     2,      0,      0,      0}, // FGPIO_55
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     3,      0,      0,      0}, // FGPIO_56
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     4,      0,      0,      0}, // FGPIO_57
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     5,      0,      0,      0}, // FGPIO_58
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     6,      0,      0,      0}, // FGPIO_59
#else
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     0,      0,      0,      0}, // FGPIO_26
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     1,      0,      0,      0}, // FGPIO_27
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     2,      0,      0,      0}, // FGPIO_28
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     3,      0,      0,      0}, // FGPIO_29
    {(DWORD)&(((GPIO *)GPIO_BASE)->Mgpint),     4,      0,      0,      0}, // FGPIO_43
#endif
};

/*
MGPIO CONFIG for MP650
[7:0]           GPIO_IC
[15:8]          GPIO_IE
[23:16]         GPIO_IPOL   PIO Interrupt Polarity
                            Each bit controls the corresponding GPIO pin.
                            = 0, Active high for level mode, rising edge for edge mode
                            = 1, Active low for level mode, falling edge for edge mode
[31:24]         GPIO_IMOD   GPIO Interrupt Mode
                            Each bit controls the corresponding GPIO pin.
                            = 0, Level Mode
                            = 1, Edge Mode
[0] P_FCF_CD    CF card detect
[1] P_FXD_CD    XD card detect
[2] P_FSD_CD    SD1 card detect
[3] P_FMS_CD    MS card detect
[4] P_FSDIO     SDIO/SD2 card detect
[5]             Reserved
[6]             Reserved
[7]             Reserved

MGPIO CONFIG for MP615
[7:0]           GPIO_IC
[15:8]          GPIO_IE
[23:16]         GPIO_IPOL   PIO Interrupt Polarity
                            Each bit controls the corresponding GPIO pin.
                            = 0, Active high for level mode, rising edge for edge mode
                            = 1, Active low for level mode, falling edge for edge mode
[31:24]         GPIO_IMOD   GPIO Interrupt Mode
                            Each bit controls the corresponding GPIO pin.
                            = 0, Level Mode
                            = 1, Edge Mode
[0] P_FSMCDx    SM card detect
[1] P_FSMWPx    SM write protect
[2] P_FSDCDx    SD card detect
[3] P_FSDWPx    SD write protect
[4] P_FMSINS    MS card detect
[5]             Reserved
[6] _FXDCDx     XD card detect
*/

enum
{
    GPIO_INT0_TYPE = 0,
#if (CHIP_VER_MSB == CHIP_VER_615)
    GPIO_INT1_TYPE,
    GPIO_INT2_TYPE,
    GPIO_INT3_TYPE,
#endif
    FGPIO_INT_TYPE,
};



static SDWORD checkGpioIntVaild(DWORD gpioNum)
{
    DWORD groupId, gpioIndex;

    groupId = gpioNum & HAL_GPIO_GROUP_MASK;
    gpioNum &= HAL_GPIO_NUMBER_MASK;

    switch ( groupId )
    {
    case HAL_GPIO_GROUP_INDEX:
        gpioIndex = gpioNum - GPIO_INT_OFFSET;

        if (gpioIndex >= GPIO_INT_COUNT)
        {
            MP_ALERT("--E-- Exceed to the max GPIO int num");

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;

    case HAL_FGPIO_GROUP_INDEX:
        if (gpioNum < FGPIO_INT_OFFSET)
        {
            MP_ALERT("--E-- checkGpioIntVaildWrong FGPIO int num");

            return ERR_GPIO_INVALID_NUMBER;
        }

        gpioIndex = gpioNum - FGPIO_INT_OFFSET;

        if (gpioIndex >= FGPIO_INT_COUNT)
        {
            MP_ALERT("--E-- Exceed to the max FGPIO int num");

            return ERR_GPIO_INVALID_NUMBER;
        }
        break;
    default:
        MP_ALERT("--E-- Unsupported GPIO Interrupt Register: 0x%X", groupId | gpioNum);

        return ERR_GPIO_INVALID_NUMBER;
    }

    return NO_ERR;
}



//#define _DEBUG_HAL_GPIO_ISR

static void gpioIntService(DWORD regGpIntAddr, DWORD offset, DWORD intCounts)
{
    volatile DWORD *regGpIntPtr = (DWORD *) regGpIntAddr;
    BYTE cause, mask, mode;
    BYTE index;

    cause = (*regGpIntPtr) & 0x000000FF;            // GET_REG_BITS(*xgpInt, GPINT_CAUSE)
    mask = ((*regGpIntPtr) & 0x0000FF00) >> 8;      // GET_REG_BITS(*xgpInt, GPINT_ENABLE)
    mode = ((*regGpIntPtr) & 0xFF000000) >> 24;     // GET_REG_BITS(*xgpInt, GPINT_MODE)
    //polarity = ((*regGpIntPtr) & 0x00FF0000) >> 16; // GET_REG_BITS(*xgpInt, GPINT_POLARITY)

#ifdef _DEBUG_HAL_GPIO_ISR
    MP_ALERT("INT Cause = 0x%X", cause);
    MP_ALERT("INT Mask = 0x%X", mask);
    MP_ALERT("INT Mode = 0x%X", mode);
    //MP_ALERT("INT Polarity = 0x%X", polarity);
#endif

    cause &= mask;
    index = 0;

    while ( cause && (intCounts > index))
    {
        if (cause & BIT0)
        {
            if ( GPIO_EDGE_TRIGGER == (mode & BIT0) )
            {
#ifdef _DEBUG_HAL_GPIO_ISR
                MP_ALERT("Clean Cause Bit-%d, 0x%X", index, *regGpIntPtr & 0xFF);
#endif

                *regGpIntPtr &= ~(1 << index);

#ifdef _DEBUG_HAL_GPIO_ISR
                MP_ALERT("New Cause = 0x%X", *regGpIntPtr & 0xFF);
#endif
            }

            gpioIntFunPtrArray[offset + index](index);
        }

        cause >>= 1;
        mode >>= 1;
        index++;
    }
}



static void gpioDefaultInt(WORD index)
{
    // nothing will be done here!!
}



static DWORD gpioIntGroupAddrGet(E_GPIO_INT_GROUP_INDEX gpioNum)
{
    if (checkGpioIntVaild(gpioNum) != NO_ERR)
        return NULL;

    return GpioIntConfigInfo[gpioNum & HAL_GPIO_NUMBER_MASK].dwGroupAddr;
}



BOOL Gpio_GpioInstallStatus(void)
{
    return gpioIntAlreadyInitialed;
}



SDWORD Gpio_IntInit(void)
{
    GPIO *regGpioPtr = (GPIO *) GPIO_BASE;

    MP_DEBUG("GPIO_INT_COUNT = %d, 9", GPIO_INT_COUNT);
    MP_DEBUG("FGPIO_INT_COUNT = %d, 5", FGPIO_INT_COUNT);
    MP_DEBUG("FGPIO_INT_OFFSET = %d, 9", FGPIO_INT_OFFSET);
    MP_DEBUG("TOTAL_GPIO_INT_COUNT = %d, 14", TOTAL_GPIO_INT_COUNT);

    if (!gpioIntAlreadyInitialed)
    {
        BYTE i;

        SystemIntDis(IM_GPIO);
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
        SystemIntDis(IM_FGPIO);
#endif

        for (i = 0; i < TOTAL_GPIO_INT_COUNT; i++)
            gpioIntFunPtrArray[i] = gpioDefaultInt;

        gpioIntAlreadyInitialed = TRUE;

        SystemIntHandleRegister(IM_GPIO, gpioIsr);
        SystemIntEna(IM_GPIO);

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
        SystemIntHandleRegister(IM_FGPIO, fgpioIsr);
        SystemIntEna(IM_FGPIO);
#endif
    }

    // Initial unbound GPIO as output high
#if (CHIP_VER_LSB != CHIP_VER_FPGA)
#if (CHIP_VER_MSB == CHIP_VER_650)
    // UGPIO-0~3 output high
    regGpioPtr->Ugpdat |= 0x000F000F;
    regGpioPtr->Ugpcfg &= ~0x000F000F;
    regGpioPtr->Ugpdat |= 0x000F000F;

    // GPIO-8~31 output high
    regGpioPtr->Gpdat0 |= 0xFF00FF00;
    regGpioPtr->Gpcfg0 &= ~0xFF00FF00;
    regGpioPtr->Gpdat0 |= 0xFF00FF00;
    regGpioPtr->Gpdat1 = 0xFFFFFFFF;
    regGpioPtr->Gpcfg1 = 0;
    regGpioPtr->Gpdat1 = 0xFFFFFFFF;
#endif

#if (CHIP_VER_MSB == CHIP_VER_660)
    // UGPIO-2~7 output high
    regGpioPtr->Ugpdat |= 0x00FC00FC;
    regGpioPtr->Ugpcfg &= ~0x00FC00FC;
    regGpioPtr->Ugpdat |= 0x00FC00FC;

    // GPIO-4~31 output high
    regGpioPtr->Gpdat0 |= 0xFFF0FFF0;
    regGpioPtr->Gpcfg0 &= ~0xFFF0FFF0;
    regGpioPtr->Gpdat0 |= 0xFFF0FFF0;
    regGpioPtr->Gpdat1 = 0xFFFFFFFF;
    regGpioPtr->Gpcfg1 = 0;
    regGpioPtr->Gpdat1 = 0xFFFFFFFF;
#endif
#endif      // #if (CHIP_VER_LSB != CHIP_VER_FPGA)

    return NO_ERR;
}



SDWORD Gpio_IntConfig(E_GPIO_INT_GROUP_INDEX gpioNum, DWORD triggerPolarity, DWORD triggerMode)
{
    volatile DWORD *groupAddr;
    DWORD bitOffset;

    if (gpioNum == GPIO_GPIO_NULL)
        return ERR_GPIO_INVALID_NUMBER;

    groupAddr = (DWORD *) gpioIntGroupAddrGet(gpioNum);

    if (groupAddr == NULL)
    {
        MP_ALERT("--E-- Gpio_IntConfig - Unsupported GPIO Interrupt: 0x%X\n\r", gpioNum);

        return ERR_GPIO_INVALID_NUMBER;
    }

    gpioNum &= HAL_GPIO_NUMBER_MASK;
    bitOffset = GpioIntConfigInfo[gpioNum].bBitOffset;
    triggerPolarity &= BIT0;
    triggerMode &= BIT0;

    IntDisable();
    *groupAddr = ( *groupAddr & ~((1 << (bitOffset + 24)) | (1 << (bitOffset + 16))) ) |
                 ( (triggerMode << (bitOffset + 24)) | (triggerPolarity << (bitOffset + 16)) );
    IntEnable();

    return NO_ERR;
}



SDWORD Gpio_IntConfigGet(E_GPIO_INT_GROUP_INDEX gpioNum, BOOL *triggerPolarity, BOOL *triggerMode)
{
    volatile DWORD *groupAddr;
    DWORD bitOffset;

    if (gpioNum == GPIO_GPIO_NULL)
        return ERR_GPIO_INVALID_NUMBER;

    groupAddr = (DWORD *) gpioIntGroupAddrGet(gpioNum);

    if (groupAddr == NULL)
    {
        MP_ALERT("--E-- Gpio_IntConfigGet - Unsupported GPIO Interrupt: 0x%X", gpioNum);

        return ERR_GPIO_INVALID_NUMBER;
    }

    gpioNum &= HAL_GPIO_NUMBER_MASK;
    bitOffset = GpioIntConfigInfo[gpioNum].bBitOffset;

    if (triggerPolarity)
        *triggerPolarity = (*groupAddr >> (bitOffset + 16)) & BIT0;

    if (triggerMode)
        *triggerMode = (*groupAddr >> (bitOffset + 24)) & BIT0;

    return NO_ERR;
}



SDWORD Gpio_IntCallbackFunRegister(E_GPIO_INT_GROUP_INDEX gpioNum, void (*gpioIsrPtr)(WORD))
{
    if (checkGpioIntVaild(gpioNum) != NO_ERR)
        return ERR_GPIO_INVALID_NUMBER;

    if (!gpioIsrPtr)
        gpioIntFunPtrArray[gpioNum & HAL_GPIO_NUMBER_MASK] = gpioDefaultInt;
    else
        gpioIntFunPtrArray[gpioNum & HAL_GPIO_NUMBER_MASK] = gpioIsrPtr;

    return NO_ERR;
}



SDWORD Gpio_IntEnable(E_GPIO_INT_GROUP_INDEX gpioNum)
{
    volatile DWORD *groupAddr;

    if (gpioNum == GPIO_GPIO_NULL)
        return ERR_GPIO_INVALID_NUMBER;

    groupAddr = (DWORD *) gpioIntGroupAddrGet(gpioNum);

    if ( groupAddr == NULL )
    {
        MP_ALERT("--E-- Unsupported GPIO Interrupt Enable: 0x%X", gpioNum);

        return ERR_GPIO_INVALID_NUMBER;
    }

    MP_DEBUG("Gpio_IntEnable -\nBefore 0x%X = 0x%X", (DWORD) groupAddr, *groupAddr);

    //Gpio_DataGet(gpioNum, 0, 1);            // input
    gpioNum &= HAL_GPIO_NUMBER_MASK;
    IntDisable();
    *groupAddr |= (1 << (GpioIntConfigInfo[gpioNum].bBitOffset + 8));
    IntEnable();

    MP_DEBUG("After Gpio_IntEnable - 0x%X = 0x%X", (DWORD) groupAddr, *groupAddr);

    return NO_ERR;
}



SDWORD Gpio_IntDisable(E_GPIO_INT_GROUP_INDEX gpioNum)
{
    volatile DWORD *groupAddr;

    if (gpioNum == GPIO_GPIO_NULL)
        return ERR_GPIO_INVALID_NUMBER;

    groupAddr = (DWORD *) gpioIntGroupAddrGet(gpioNum);

    if ( groupAddr == NULL )
    {
        MP_ALERT("--E-- Unsupported GPIO Interrupt Disable: 0x%X", gpioNum);

        return ERR_GPIO_INVALID_NUMBER;
    }

    MP_DEBUG("Gpio_IntDisable -\nBefore 0x%X = 0x%X", (DWORD) groupAddr, *groupAddr);

    gpioNum &= HAL_GPIO_NUMBER_MASK;

    IntDisable();
    *groupAddr &= ~(1 << (GpioIntConfigInfo[gpioNum].bBitOffset + 8));
    IntEnable();

    MP_DEBUG("After 0x%X = 0x%X", (DWORD) groupAddr, *groupAddr);

    return NO_ERR;
}



static void gpioIsr(void)
{
    volatile DWORD regGpIntAddr = 0;
    DWORD offset, intCounts = 0;
    GPIO *regGpioPtr = (GPIO *) GPIO_BASE;

#if (CHIP_VER_MSB == CHIP_VER_615)

    DWORD gpioType = GPIO_INT0_TYPE;

    while ( gpioType <= FGPIO_INT_TYPE )
    {
        switch ( gpioType )
        {
        case GPIO_INT0_TYPE:
            regGpIntAddr = (DWORD) (&regGpioPtr->Gpint[0]);
            intCounts = 8;
            offset = 0;

            #ifdef _DEBUG_HAL_GPIO_ISR
            MP_ALERT("GPIO_INT0_TYPE-");
            #endif
            break;

        case GPIO_INT1_TYPE:
            regGpIntAddr = (DWORD) (&regGpioPtr->Gpint[1]);
            offset = 8;
            intCounts = 8;

            #ifdef _DEBUG_HAL_GPIO_ISR
            MP_ALERT("GPIO_INT1_TYPE-");
            #endif
            break;

        case GPIO_INT2_TYPE:
            regGpIntAddr = (DWORD) (&regGpioPtr->Gpint[2]);
            offset = 16;
            intCounts = 8;

            #ifdef _DEBUG_HAL_GPIO_ISR
            MP_ALERT("GPIO_INT2_TYPE-");
            #endif
            break;

        case GPIO_INT3_TYPE:
            regGpIntAddr = (DWORD) (&regGpioPtr->Gpint[3]);
            offset = 24;
            intCounts = 8;

            #ifdef _DEBUG_HAL_GPIO_ISR
            MP_ALERT("GPIO_INT3_TYPE-");
            #endif
            break;

        case FGPIO_INT_TYPE:
            regGpIntAddr = (DWORD) (&regGpioPtr->Mgpint);
            offset = FGPIO_INT_OFFSET;
            intCounts = FGPIO_INT_COUNT;

            #ifdef _DEBUG_HAL_GPIO_ISR
            MP_ALERT("FGPIO_INT_TYPE-");
            #endif
            break;
        }

        gpioType++;
        gpioIntService(regGpIntAddr, offset, intCounts);
    }

#else   // MP650/660

    regGpIntAddr = (DWORD) (&regGpioPtr->Gpint[0]);
    intCounts = 8;
    offset = 0;

    #ifdef _DEBUG_HAL_GPIO_ISR
    MP_ALERT("GPIO_INT0_TYPE-");
    #endif

    gpioIntService(regGpIntAddr, offset, intCounts);

#endif
}



#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
static void fgpioIsr(void)
{
    volatile DWORD regGpIntAddr;
    DWORD offset, intCounts = 0;
    GPIO *regGpioPtr = (GPIO *) GPIO_BASE;

    regGpIntAddr = (DWORD) (&regGpioPtr->Mgpint);
    offset = FGPIO_INT_OFFSET;
    intCounts = FGPIO_INT_COUNT;

    #ifdef _DEBUG_HAL_GPIO_ISR
    MP_ALERT("F - FGPIO_INT_TYPE-");
    #endif

    gpioIntService(regGpIntAddr, offset, intCounts);
}
#endif



void SetGPIOValue(WORD gpioNum, BYTE gpioValue)
{
    DWORD gpioNumIndex;

    switch (gpioNum & 0xFF00)
    {
    case NORMAL_GPIO:
        gpioNumIndex = GPIO_GPIO_START;
        break;

    case FGPIO:
        gpioNumIndex = GPIO_FGPIO_START;
        break;

    case UGPIO:
        gpioNumIndex = GPIO_UGPIO_START;
        break;

    case VGPIO:
        gpioNumIndex = GPIO_VGPIO_START;
        break;

    case AGPIO:
        gpioNumIndex = GPIO_AGPIO_START;
        break;

#if (CHIP_VER_MSB == CHIP_VER_615)
    case MGPIO:
        gpioNumIndex = GPIO_MGPIO_START;
        break;
#else
    case KGPIO:
        gpioNumIndex = GPIO_KGPIO_START;
        break;

    case OGPIO:
        gpioNumIndex = GPIO_OGPIO_START;
        break;

    case PGPIO:
        gpioNumIndex = GPIO_PGPIO_START;
        break;
#endif

    default:
        MP_ALERT("--E-- SetGPIOValue - Wrong GPIO number - 0x%4X!!!!", gpioNum);

        return;
        break;
    }

    gpioNumIndex += gpioNum & 0x00FF;
    MP_DEBUG("SetGPIOValue - 0x%04X 0x%08X", gpioNum, gpioNumIndex);
    Gpio_Config2GpioFunc(gpioNumIndex, GPIO_OUTPUT_MODE, gpioValue, 1);
}



BYTE GetGPIOValue(WORD gpioNum)
{
    DWORD gpioNumIndex;
    WORD data;

    switch (gpioNum & 0xFF00)
    {
    case NORMAL_GPIO:
        gpioNumIndex = GPIO_GPIO_START;
        break;

    case FGPIO:
        gpioNumIndex = GPIO_FGPIO_START;
        break;

    case UGPIO:
        gpioNumIndex = GPIO_UGPIO_START;
        break;

    case VGPIO:
        gpioNumIndex = GPIO_VGPIO_START;
        break;

    case AGPIO:
        gpioNumIndex = GPIO_AGPIO_START;
        break;

#if (CHIP_VER_MSB == CHIP_VER_615)
    case MGPIO:
        gpioNumIndex = GPIO_MGPIO_START;
        break;
#else
    case KGPIO:
        gpioNumIndex = GPIO_KGPIO_START;
        break;

    case OGPIO:
        gpioNumIndex = GPIO_OGPIO_START;
        break;

    case PGPIO:
        gpioNumIndex = GPIO_PGPIO_START;
        break;
#endif

    default:
        MP_ALERT("--E-- GetGPIOValue - Wrong GPIO number - 0x%4X!!!!", gpioNum);

        return;
        break;
    }

    gpioNumIndex += gpioNum & 0xFF;
    MP_DEBUG("GetGPIOValue - 0x%04X 0x%08X", gpioNum, gpioNumIndex);
    Gpio_Config2GpioFunc(gpioNumIndex, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);

    if (Gpio_DataGet(gpioNumIndex, &data, 1) == NO_ERR)
        return (BYTE) data;
    else
        return 0xFF;
}

BYTE QuickGetGPIOValue(WORD gpioNum)
{
    DWORD gpioNumIndex;
    WORD data;

    switch (gpioNum & 0xFF00)
    {
    case NORMAL_GPIO:
        gpioNumIndex = GPIO_GPIO_START;
        break;

    case FGPIO:
        gpioNumIndex = GPIO_FGPIO_START;
        break;

    case UGPIO:
        gpioNumIndex = GPIO_UGPIO_START;
        break;

    case VGPIO:
        gpioNumIndex = GPIO_VGPIO_START;
        break;

    case AGPIO:
        gpioNumIndex = GPIO_AGPIO_START;
        break;

#if (CHIP_VER_MSB == CHIP_VER_615)
    case MGPIO:
        gpioNumIndex = GPIO_MGPIO_START;
        break;
#else
    case KGPIO:
        gpioNumIndex = GPIO_KGPIO_START;
        break;

    case OGPIO:
        gpioNumIndex = GPIO_OGPIO_START;
        break;

    case PGPIO:
        gpioNumIndex = GPIO_PGPIO_START;
        break;
#endif

    default:
        MP_ALERT("--E-- GetGPIOValue - Wrong GPIO number - 0x%4X!!!!", gpioNum);

        return;
        break;
    }

    gpioNumIndex += gpioNum & 0xFF;
    MP_DEBUG("GetGPIOValue - 0x%04X 0x%08X", gpioNum, gpioNumIndex);
  //  Gpio_Config2GpioFunc(gpioNumIndex, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);

    if (Gpio_DataGet(gpioNumIndex, &data, 1) == NO_ERR)
        return (BYTE) data;
    else
        return 0xFF;
}

E_GPIO_GROUP_INDEX GetGPIOPinIndex(WORD gpioNum)
{
    DWORD gpioNumIndex;
    WORD data;

    switch (gpioNum & 0xFF00)
    {
    case NORMAL_GPIO:
        gpioNumIndex = GPIO_GPIO_START;
        break;

    case FGPIO:
        gpioNumIndex = GPIO_FGPIO_START;
        break;

    case UGPIO:
        gpioNumIndex = GPIO_UGPIO_START;
        break;

    case VGPIO:
        gpioNumIndex = GPIO_VGPIO_START;
        break;

    case AGPIO:
        gpioNumIndex = GPIO_AGPIO_START;
        break;

#if (CHIP_VER_MSB == CHIP_VER_615)
    case MGPIO:
        gpioNumIndex = GPIO_MGPIO_START;
        break;
#else
    case KGPIO:
        gpioNumIndex = GPIO_KGPIO_START;
        break;

    case OGPIO:
        gpioNumIndex = GPIO_OGPIO_START;
        break;

    case PGPIO:
        gpioNumIndex = GPIO_PGPIO_START;
        break;
#endif

    default:
        MP_ALERT("--E-- GetGPIOValue - Wrong GPIO number - 0x%4X!!!!", gpioNum);

        return;
        break;
    }

    gpioNumIndex += gpioNum & 0xFF;
    MP_DEBUG("GetGPIOValue - 0x%04X 0x%08X", gpioNum, gpioNumIndex);

    return (E_GPIO_GROUP_INDEX) gpioNumIndex;
}



E_GPIO_INT_GROUP_INDEX GetGpioIntIndexByGpioPinNum(E_GPIO_GROUP_INDEX gpioNum)
{
#if (CHIP_VER_MSB == CHIP_VER_650)
    if ((gpioNum >= GPIO_GPIO_START) && (gpioNum <= GPIO_RGPIN))
    {
        return (GPINT_GPIO_START + (gpioNum - GPIO_GPIO_START));
    }
    else
    {
        switch (gpioNum)
        {
        case GPIO_FGPIO_26:
            return GPINT_CF_CARD_DETECT;

        case GPIO_FGPIO_27:
            return GPINT_XD_CARD_DETECT;

        case GPIO_FGPIO_28:
            return GPINT_SD_CARD_DETECT;

        case GPIO_FGPIO_29:
            return GPINT_MS_CARD_DETECT;

        case GPIO_FGPIO_43:
            return GPINT_SDIO_CARD_DETECT;

        default:
            return 0xFFFFFFFF;
        }
    }
#elif (CHIP_VER_MSB == CHIP_VER_660)
    if ((gpioNum >= GPIO_GPIO_START) && (gpioNum <= GPIO_GPIO_3))
    {
        return (GPINT_GPIO_START + (gpioNum - GPIO_GPIO_START));
    }
    else
    {
        switch (gpioNum)
        {
        case GPIO_FGPIO_26:
            return GPINT_CF_CARD_DETECT;

        case GPIO_FGPIO_27:
            return GPINT_XD_CARD_DETECT;

        case GPIO_FGPIO_28:
            return GPINT_SD_CARD_DETECT;

        case GPIO_FGPIO_29:
            return GPINT_MS_CARD_DETECT;

        case GPIO_FGPIO_43:
            return GPINT_SDIO_CARD_DETECT;

        default:
            return 0xFFFFFFFF;
        }
    }
#else   // MP615
    if ((gpioNum >= GPIO_GPIO_START) && (gpioNum <= GPIO_GPIO_31))
    {
        return (GPINT_GPIO_START + (gpioNum - GPIO_GPIO_START));
    }
    else
    {
        switch (gpioNum)
        {
        case GPIO_FGPIO_53:
            return GPINT_SM_CARD_DETECT;

        case GPIO_FGPIO_54:
            return GPINT_SM_WRITE_PROTECT;

        case GPIO_FGPIO_55:
            return GPINT_SD_CARD_DETECT;

        case GPIO_FGPIO_56:
            return GPINT_SD_WRITE_PROTECT;

        case GPIO_FGPIO_57:
            return GPINT_MS_CARD_DETECT;

        case GPIO_FGPIO_59:
            return GPINT_XD_CARD_DETECT;

        default:
            return 0xFFFFFFFF;
        }
    }
#endif
}



////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(Gpio_Config2GpioFunc);
MPX_KMODAPI_SET(Gpio_ConfiguraionSet);
MPX_KMODAPI_SET(Gpio_ConfigurationGet);
MPX_KMODAPI_SET(Gpio_DataGet);
MPX_KMODAPI_SET(Gpio_DataSet);

MPX_KMODAPI_SET(Gpio_IntInit);
MPX_KMODAPI_SET(Gpio_IntCallbackFunRegister);
MPX_KMODAPI_SET(Gpio_IntConfig);
MPX_KMODAPI_SET(Gpio_IntConfigGet);
MPX_KMODAPI_SET(Gpio_IntEnable);
MPX_KMODAPI_SET(Gpio_IntDisable);

MPX_KMODAPI_SET(SetGPIOValue);
MPX_KMODAPI_SET(GetGPIOValue);
#endif

