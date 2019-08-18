/*
*******************************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2006, Magic Pixel Inc., HsinChu, Taiwan. All rights reserved.
*
*------------------------------------------------------------------------------
* Description: glocal type definition for 321/322 project
*
*------------------------------------------------------------------------------
* Change Log (most recent first):
*   08/10/2006  Wesley  Create for Omega2
*******************************************************************************
*/

#ifndef __TYPEDEF_H_
#define __TYPEDEF_H_

#if 0
#define MP600
#define MP620
#define PLATFORM_MPIXEL 1
#define NEW_NETSOCKET   1
#else
#include "platform_config.h"
#endif

#include "UtilTypeDef.h"
#include "param.h"

#ifndef IMAGIC_EMBEDDED_SDRAM
    #define IMAGIC_EMBEDDED_SDRAM   0
#endif

#ifdef IMAGIC_CHIP_TYPE
    #if (IMAGIC_CHIP_TYPE == 0)
        // MP321D Chip Type
        #define CHIP_TYPE_MP321D
    #elif (IMAGIC_CHIP_TYPE == 1)
        // MP322A Chip Type
        #define CHIP_TYPE_MP322
        #define CHIP_TYPE_MP322_A
    #elif (IMAGIC_CHIP_TYPE == 2)
        // MP322B Chip Type
        #define CHIP_TYPE_MP322
        #define CHIP_TYPE_MP322_B
    #elif (IMAGIC_CHIP_TYPE == 3)
        // MP322C Chip Type
        #define CHIP_TYPE_MP322
        #define CHIP_TYPE_MP322_C
    #elif (IMAGIC_CHIP_TYPE == 4)
        // MP322C Chip Type
        #define CHIP_TYPE_MP322
        #define CHIP_TYPE_MP310_B
    #else   // default, last version
        // MP322C Chip Type
        #define CHIP_TYPE_MP322
        #define CHIP_TYPE_MP322_C
    #endif
#else
    // MP321D Chip Type
    #define CHIP_TYPE_MP321D
#endif

#if defined(CHIP_TYPE_MP322)
    // This option is for DRAI clock, and will limited DRAI clock always small than PLL2/2
    //#define BOARD_MP322_EVB             // else BOARD_MP522_DTB_VA

    //#define PLL_BASE_ON_13MHZ
    #define PLL_BASE_ON_13_5MHZ

    #ifdef IMAGIC_EMBEDDED_SDRAM
        #if (IMAGIC_EMBEDDED_SDRAM == 0)        // MP5xx serial
            #define NEED_INCREASE_DRAI_DRV
        #endif
    #else
        #define NEED_INCREASE_DRAI_DRV          // default
    #endif
#elif defined(CHIP_TYPE_MP321D)
    #define PLL_BASE_ON_13MHZ
    //#define PLL_BASE_ON_13_5MHZ
#endif

#undef TRUE
#undef FALSE
#undef NULL

#define TRUE                1
#define FALSE               0
#define NULL                0

#ifdef MP600
typedef U08 BOOLEAN;
typedef BOOLEAN *PBOOLEAN;
#endif

#ifndef __KERNEL__
typedef U08 u8;
typedef U16 u16;
typedef U32 u32;
typedef S08 s8;
typedef U64 u64;
typedef S64 s64;
typedef S16 s16;
typedef S32 s32;

#ifndef _ASM_GENERIC_INT_LL64_H
typedef unsigned char __u8;
typedef U16 __u16;
typedef U32 __u32;
typedef S08 __s8;
typedef U64 __u64;
typedef S16 __s16;
typedef S32 __s32;
//typedef unsigned int uint;
typedef unsigned long ulong;

typedef __u16 __le16;
typedef __u16 __be16;
typedef __u32 __le32;
typedef __u32 __be32;
typedef __u64 __le64;
typedef __u64 __be64;
typedef __u16 __sum16;
typedef __u32 __wsum;
#endif
#endif

#include "os_mp52x.h"

#ifndef __XATYPES_H         //because iAnywhere bluetooth IP are using the same typedef name
//#ifndef _BOOL
//#define _BOOL
//typedef unsigned int        BOOL;
//#endif
#endif

#if 0
#include "global612.h"
#else
/* global612.h includes too many non-global header files now (e.g., usbotg.h) */
#include "iplaysysconfig.h"
#include "UtilTypeDef.h"
#include "os.h"
#endif
#include "mpTrace.h"
//#include "lwip_incl.h"

typedef union
{
    U32 u32;
    U16 u16[2];
    U08 u08[4];
} U32_UNION;

typedef union
{
    U16 u16;
    U08 u08[2];
} U16_UNION;

#define scnprintf   snprintf

#ifndef MIN
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)		((a) > (b) ? (a) : (b))
#endif

#endif    // __TYPEDEF_H_

