#ifndef __LIB_CLOCK_H
#define __LIB_CLOCK_H

#include "iplaysysconfig.h"

enum {
    CLOCK_96M_PLL1CFG       = 0,
    CLOCK_72M_PLL1CFG       = 1,
    CLOCK_108M_PLL2CFG      = 2,
    CLOCK_74P25M_PLL2CFG    = 3,
    CLOCK_108M_PLL1CFG      = 4,
    CLOCK_120M_PLL1CFG      = 5,
    CLOCK_150M_PLL1CFG      = 6,
};

///
///@ingroup     CLOCK_MODULE
///@brief       define the PLL's index
typedef enum {
    CLOCK_PLL1_INDEX = 0,       ///< 0: PLL1's index number
    CLOCK_PLL2_INDEX,           ///< 1: PLL2's index number
    CLOCK_PLL3_INDEX,           ///< 2: PLL3's index number
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    CLOCK_PLLIDU_INDEX,         ///< 3: PLL-IDU's index number, only for \b MP650 serial
#endif

    CLOCK_PLL_INDEX_MAX
} E_PLL_INDEX;


//////////////////////////////////////////////////////
//
//  PLL setting table
//
//            N + 1
// f * ---------------------, f = MAIN_CRYSTAL_CLOCK
//      (M + 1) * (DIV + 1)
//
//////////////////////////////////////////////////////

#ifndef MAIN_CRYSTAL_CLOCK
    #error - Undefined MAIN_CRYSTAL_CLOCK
#endif

//////////////////////////////////////////////////////
//
//  MAIN_CRYSTAL_CLOCK is 13.5MHz
//
//////////////////////////////////////////////////////
#if (MAIN_CRYSTAL_CLOCK == 13500000)

#if (CHIP_VER_MSB == CHIP_VER_615)

//=========This Section need more check================================
#if 0 //Rebecca 070910: adjust clock
#define PLL3_CONFIG_CF_12M_DIV_0    0x00020b0c  //constant frequency 12 MFS
#define PLL3_CONFIG_FS_08K_DIV_0    0x00020b09  //FS_8kHz
#define PLL3_CONFIG_FS_11K_DIV_1    0x01022012  //FS_11kHz
#define PLL3_CONFIG_FS_12K_DIV_0    0x00024448  //FS_12kHz
#define PLL3_CONFIG_FS_16K_DIV_0    0x00021709  //FS_16kHz
#define PLL3_CONFIG_FS_22K_DIV_1    0x01022012  //FS_22kHz
#define PLL3_CONFIG_FS_24K_DIV_0    0x00024448  //FS_24kHz
#define PLL3_CONFIG_FS_32K_DIV_0    0x00022d49  //FS_32kHz
#define PLL3_CONFIG_FS_44K_DIV_1    0x01022012  //FS_44kHz
#define PLL3_CONFIG_FS_48K_DIV_0    0x00024448  //FS_48khz
#define PLL3_CONFIG_FS_64K_DIV_0    0x00025d48  //FS_64khz
#define PLL3_CONFIG_FS_88K_DIV_1    0x0102fc48  //FS_88khz
#define PLL3_CONFIG_FS_96K_DIV_0    0x00028948  //FS_96khz
#else //these set should acompany with audio_hal.c AK4387_config( )  audio_clock->Clkss_EXT1 = 0x0000000b;//for PLL3/4  and enable
#define PLL3_CONFIG_CF_12M_DIV_0    0x00020b0c  //constant frequency 12 MFS
#define PLL3_CONFIG_FS_08K_DIV_0    0x0002821a  //FS_8kHz
#define PLL3_CONFIG_FS_11K_DIV_1    0x0102f624  //FS_11kHz
#define PLL3_CONFIG_FS_12K_DIV_0    0x00028d26  //FS_12kHz
#define PLL3_CONFIG_FS_16K_DIV_0    0x0002870d  //FS_16kHz
#define PLL3_CONFIG_FS_22K_DIV_1    0x0102f624  //FS_22kHz
#define PLL3_CONFIG_FS_24K_DIV_0    0x0002ce38  //FS_24kHz
#define PLL3_CONFIG_FS_32K_DIV_0    0x00022d12  //FS_32kHz
#define PLL3_CONFIG_FS_44K_DIV_1    0x0102f624  //FS_44kHz
#define PLL3_CONFIG_FS_48K_DIV_0    0x0002ce38  //FS_48khz
#define PLL3_CONFIG_FS_64K_DIV_0    0x00025d48  //FS_64khz
#define PLL3_CONFIG_FS_88K_DIV_1    0x0102fc48  //FS_88khz
#define PLL3_CONFIG_FS_96K_DIV_0    0x00028948  //FS_96khz
#endif
//====================================================================

// For general case
#define PLL_CONFIG_6M               0x01020708
#define PLL_CONFIG_12M              0x01020F08
#define PLL_CONFIG_15M              0x01021308
#define PLL_CONFIG_20M              0x01021B08
#define PLL_CONFIG_30M              0x00003B1A
#define PLL_CONFIG_48M              0x01023F08
#define PLL_CONFIG_54300K           0x01022704
#define PLL_CONFIG_65250K           0x01021C02
#define PLL_CONFIG_72M              0x00023F0B
#define PLL_CONFIG_74P25M           0x00026211
#define PLL_CONFIG_96M              0x00023F08
#define PLL_CONFIG_108M             0x00023F07
#define PLL_CONFIG_120M             0x00124F08
#define PLL_CONFIG_150M             0x00126308
#define PLL_CONFIG_160M             0x0012EC13  // 159.975MHz
#define PLL_CONFIG_166M             0x0012D010  // 165.97MHz

#define PLL1_CONFIG_96M_DIV_0       PLL_CONFIG_96M
#define PLL1_CONFIG_96M_DIV_1       0x01027f08
#define PLL2_CONFIG_108M            PLL_CONFIG_108M

#else   // MP650/660

#warning Undefined crystal frequency 13.5MHz for MP650/660
#define __PLL_CFG_ALWAYS_CALCULATE

#endif

//////////////////////////////////////////////////////
//
//  MAIN_CRYSTAL_CLOCK is 12MHz
//
//////////////////////////////////////////////////////
#elif (MAIN_CRYSTAL_CLOCK == 12000000)

#if (CHIP_VER_MSB == CHIP_VER_615)

#warning Undefined crystal frequency 12MHz for MP615
#define __PLL_CFG_ALWAYS_CALCULATE

#else   // MP650/660

#define PLL3_CONFIG_CF_12M_DIV_0    0  //constant frequency 12 MFS
#define PLL3_CONFIG_FS_08K_DIV_0    0  //FS_8kHz
#define PLL3_CONFIG_FS_11K_DIV_1    0  //FS_11kHz
#define PLL3_CONFIG_FS_12K_DIV_0    0  //FS_12kHz
#define PLL3_CONFIG_FS_16K_DIV_0    0  //FS_16kHz
#define PLL3_CONFIG_FS_22K_DIV_1    0  //FS_22kHz
#define PLL3_CONFIG_FS_24K_DIV_0    0  //FS_24kHz
#define PLL3_CONFIG_FS_32K_DIV_0    0  //FS_32kHz
#define PLL3_CONFIG_FS_44K_DIV_1    0  //FS_44kHz
#define PLL3_CONFIG_FS_48K_DIV_0    0  //FS_48khz
#define PLL3_CONFIG_FS_64K_DIV_0    0  //FS_64khz
#define PLL3_CONFIG_FS_88K_DIV_1    0  //FS_88khz
#define PLL3_CONFIG_FS_96K_DIV_0    0  //FS_96khz

// For general case
#define PLL_CONFIG_6M               0x01770707
#define PLL_CONFIG_12M              0x0177050B
#define PLL_CONFIG_15M              0x0177050E
#define PLL_CONFIG_20M              0x01770513
#define PLL_CONFIG_30M              0x0077051D
#define PLL_CONFIG_48M              0x01770b47
#define PLL_CONFIG_54M              0x01770b6B
#define PLL_CONFIG_54300K           0x017713B4
#define PLL_CONFIG_65250K           0x01770756
#define PLL_CONFIG_72M              0x00770B47
#define PLL_CONFIG_74P25M           0x00770F62
#define PLL_CONFIG_81M              0x00770735
#define PLL_CONFIG_96M              0x0077073F
#define PLL_CONFIG_108M             0x00770747
#define PLL_CONFIG_120M             0x0077074F
#define PLL_CONFIG_150M             0x00770763
#define PLL_CONFIG_153M             0x00770765
#define PLL_CONFIG_160M             0x00770877
#define PLL_CONFIG_166M             0x00770BA5

#define PLL1_CONFIG_96M_DIV_0       PLL_CONFIG_96M
#define PLL1_CONFIG_96M_DIV_1       0x0177055F
#define PLL2_CONFIG_108M            PLL_NEW_CONFIG_108M

// For PLL2 and PLL-IDU at MP650

// divN = ((regClockPtr->PLL2Cfg >> 8) & 0x00FF) + 1;
// Step 1). >> 8 // 所以 和 最後 個位, 十位 Hex 無關
// Step 2). & 0x00FF // 所以 只和 百位, 千位 Hex 有關 
// divM = (regClockPtr->PLL2Cfg & 0x00FF) + 1;
// Step 1). & 0x00FF // 所以 只和 個位, 十位 Hex 有關 
// divide = (regClockPtr->PLL2Cfg & (BIT25 | BIT24)) >> 24;
// Step 1). >> 24 // 所以 只和 最左邊 兩個Hex 有關

// 量測電流 only, system not support this setting
#define PLL_NEW_CONFIG_48M          0x03020F00      // 48MHz
// type1 0x03......
#define PLL_NEW_CONFIG_65250K       0x03021500      // 66MHz
#define PLL_NEW_CONFIG_66M          0x03021500
#define PLL_NEW_CONFIG_72M          0x03021700
#define PLL_NEW_CONFIG_74P25M       0x03021800      // 75MHz
#define PLL_NEW_CONFIG_81M          0x03021A00
#define PLL_NEW_CONFIG_96M          0x03021F00      // 96MHz
#define PLL_NEW_CONFIG_108M         0x03022300      // 108MHz
// type2 0x01......
#define PLL_NEW_CONFIG_120M         0x01021300      // 120MHz
#define PLL_NEW_CONFIG_150M         0x01021800
#define PLL_NEW_CONFIG_153M         0x01023201      // 153MHz
#define PLL_NEW_CONFIG_160M         0x01023401      // 159MHz
#define PLL_NEW_CONFIG_165M         0x01023601
// type3 0x00......
#define PLL_NEW_CONFIG_498M         0x00025201      // 498MHz

#endif

//////////////////////////////////////////////////////
//
//  MAIN_CRYSTAL_CLOCK is other
//
//////////////////////////////////////////////////////
#else   // MAIN_CRYSTAL_CLOCK is other

#define PLL3_CONFIG_CF_12M_DIV_0    0  //constant frequency 12 MFS
#define PLL3_CONFIG_FS_08K_DIV_0    0  //FS_8kHz
#define PLL3_CONFIG_FS_11K_DIV_1    0  //FS_11kHz
#define PLL3_CONFIG_FS_12K_DIV_0    0  //FS_12kHz
#define PLL3_CONFIG_FS_16K_DIV_0    0  //FS_16kHz
#define PLL3_CONFIG_FS_22K_DIV_1    0  //FS_22kHz
#define PLL3_CONFIG_FS_24K_DIV_0    0  //FS_24kHz
#define PLL3_CONFIG_FS_32K_DIV_0    0  //FS_32kHz
#define PLL3_CONFIG_FS_44K_DIV_1    0  //FS_44kHz
#define PLL3_CONFIG_FS_48K_DIV_0    0  //FS_48khz
#define PLL3_CONFIG_FS_64K_DIV_0    0  //FS_64khz
#define PLL3_CONFIG_FS_88K_DIV_1    0  //FS_88khz
#define PLL3_CONFIG_FS_96K_DIV_0    0  //FS_96khz

#define PLL1_CONFIG_96M_DIV_0       0
#define PLL1_CONFIG_96M_DIV_1       0
#define PLL2_CONFIG_108M            0

#warning Undefined crystal frequency - always calculate by formul
#define __PLL_CFG_ALWAYS_CALCULATE

#endif

///
///@ingroup     CLOCK_MODULE
///@brief       the selection of CPU's clock source
///
typedef enum {
    CPUCKS_PLL1 = 0,                ///< CPU's clock source as PLL1
    CPUCKS_PLL1_DIV_2,              ///< CPU's clock source as PLL1 / 2
    CPUCKS_PLL1_DIV_3,              ///< CPU's clock source as PLL1 / 3
#if (CHIP_VER_MSB == CHIP_VER_650)
    CPUCKS_PLL1_DIV_1_5,            ///< CPU's clock source as PLL1 / 1.5
#elif (CHIP_VER_MSB == CHIP_VER_660)
    #if (CHIP_VER_LSB == CHIP_VER_A)
    CPUCKS_PLL1_DIV_4,              ///< CPU's clock source as PLL1 / 4
    #else
    CPUCKS_PLL1_DIV_1_5,            ///< CPU's clock source as PLL1 / 1.5
    #endif
#else
    CPUCKS_PLL1_DIV_4,              ///< CPU's clock source as PLL1 / 4
#endif
    CPUCKS_PLL1_DIV_5,              ///< CPU's clock source as PLL1 / 5
#if (CHIP_VER_MSB == CHIP_VER_615)
    CPUCKS_PLL1_DIV_6,              ///< CPU's clock source as PLL1 / 6, only for \b MP615 serial
    CPUCKS_PLL1_DIV_7,              ///< CPU's clock source as PLL1 / 7, only for \b MP615 serial
    CPUCKS_PLL1_DIV_8,              ///< CPU's clock source as PLL1 / 8, only for \b MP615 serial
#else
    CPUCKS_PLL1_DIV_128,            ///< CPU's clock source as PLL1 / 128, only for \b MP650 serial
    CPUCKS_PLL1_DIV_256,            ///< CPU's clock source as PLL1 / 256, only for \b MP650 serial
    CPUCKS_PLL1_DIV_512,            ///< CPU's clock source as PLL1 / 512, only for \b MP650 serial
#endif
    CPUCKS_PLL2,                    ///< CPU's clock source as PLL2
    CPUCKS_PLL2_DIV_2,              ///< CPU's clock source as PLL2 / 2
    CPUCKS_PLL2_DIV_3,              ///< CPU's clock source as PLL2 / 3
#if (CHIP_VER_MSB == CHIP_VER_650)
    CPUCKS_PLL2_DIV_1_5,            ///< CPU's clock source as PLL2 / 1.5
#elif (CHIP_VER_MSB == CHIP_VER_660)
    #if (CHIP_VER_LSB == CHIP_VER_A)
    CPUCKS_PLL2_DIV_4,              ///< CPU's clock source as PLL2 / 4
    #else
    CPUCKS_PLL2_DIV_1_5,            ///< CPU's clock source as PLL2 / 1.5
    #endif
#else
    CPUCKS_PLL2_DIV_4,              ///< CPU's clock source as PLL2 / 4
#endif
    CPUCKS_PLL2_DIV_5,              ///< CPU's clock source as PLL2 / 5
#if (CHIP_VER_MSB == CHIP_VER_615)
    CPUCKS_PLL2_DIV_6,              ///< CPU's clock source as PLL2 / 6, only for \b MP615 serial
    CPUCKS_PLL2_DIV_7,              ///< CPU's clock source as PLL2 / 7, only for \b MP615 serial
    CPUCKS_PLL2_DIV_8,              ///< CPU's clock source as PLL2 / 8, only for \b MP615 serial
#else
    CPUCKS_PLL2_DIV_128,            ///< CPU's clock source as PLL2 / 128, only for \b MP650 serial
    CPUCKS_PLL2_DIV_256,            ///< CPU's clock source as PLL2 / 256, only for \b MP650 serial
    CPUCKS_PLL2_DIV_512,            ///< CPU's clock source as PLL2 / 512, only for \b MP650 serial
#endif

    MAX_CPUCLK_SELECT
} E_CPU_CLOCK_SELECT;

///
///@ingroup     CLOCK_MODULE
///@brief       the selection of Memory's clock source
typedef enum {
#if (CHIP_VER_MSB == CHIP_VER_615)
    MEMCKS_PLL1 = 0,
    MEMCKS_PLL1_DIV_2,
    MEMCKS_PLL1_DIV_3,
    MEMCKS_PLL1_DIV_4,
    MEMCKS_PLL2,
    MEMCKS_PLL2_DIV_2,
    MEMCKS_PLL2_DIV_3,
    MEMCKS_PLL2_DIV_4,
#else   // MP650/660
    MEMCKS_PLL2 = 0,                ///< Memory's clock source as PLL2, \b 0 for \b MP650 serial, \b 4 for \b MP615 serial
    MEMCKS_PLL2_DIV_2,              ///< Memory's clock source as PLL2 / 2, \b 1 for \b MP650 serial, \b 5 for \b MP615 serial
    MEMCKS_PLL2_DIV_3,              ///< Memory's clock source as PLL2 / 3, \b 2 for \b MP650 serial, \b 6 for \b MP615 serial
    MEMCKS_PLL2_DIV_4,              ///< Memory's clock source as PLL2 / 4, \b 3 for \b MP650 serial, \b 7 for \b MP615 serial
    MEMCKS_PLL1,                    ///< Memory's clock source as PLL1, \b 4 for \b MP650 serial, \b 0 for \b MP615 serial
    MEMCKS_PLL1_DIV_2,              ///< Memory's clock source as PLL1 / 2, \b 5 for \b MP650 serial, \b 1 for \b MP615 serial
    MEMCKS_PLL1_DIV_3,              ///< Memory's clock source as PLL1 / 3, \b 6 for \b MP650 serial, \b 2 for \b MP615 serial
    MEMCKS_PLL1_DIV_4,              ///< Memory's clock source as PLL1 / 4, \b 7 for \b MP650 serial, \b 3 for \b MP615 serial
#endif

    MAX_MEMCLK_SELECT
} E_MEM_CLOCK_SELECT;


///
///@ingroup     CLOCK_MODULE
///@brief       the selection of Scaler(IPU)'s clock source
typedef enum {
    SCACKS_PLL1         = 0,        ///< IPU's clock source as PLL1
    SCACKS_PLL1_DIV_2   = 1,        ///< IPU's clock source as PLL1 / 2
    SCACKS_PLL1_DIV_3   = 2,        ///< IPU's clock source as PLL1 / 3
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    SCACKS_PLL1_DIV_1_5 = 3,        ///< IPU's clock source as PLL1 / 1.5
#else
    SCACKS_PLL1_DIV_4   = 3,        ///< IPU's clock source as PLL1 / 4
#endif
    SCACKS_PLL2         = 4,        ///< IPU's clock source as PLL2
    SCACKS_PLL2_DIV_2   = 5,        ///< IPU's clock source as PLL2 / 2
    SCACKS_PLL2_DIV_3   = 6,        ///< IPU's clock source as PLL2 / 3
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    SCACKS_PLL2_DIV_1_5 = 7,        ///< IPU's clock source as PLL2 / 1.5
#else
    SCACKS_PLL2_DIV_4   = 7,        ///< IPU's clock source as PLL2 / 4
#endif
} E_IPU_CLOCK_SELECT, E_SCALE_CLOCK_SELECT;

//////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////
typedef enum {
    VDACKS_TVCK     = 0,
    VDACKS_IDUCK    = 1,
} E_VDA_CLOCK_SELECT;

typedef enum {
    IDUCKS_PLL1         = 0,
    IDUCKS_PLL1_DIV_2   = 1,
    IDUCKS_PLL1_DIV_3   = 2,
    IDUCKS_PLL1_DIV_4   = 3,
    IDUCKS_PLL1_DIV_5   = 4,
    IDUCKS_PLL1_DIV_6   = 5,
    IDUCKS_PLL1_DIV_7   = 6,
    IDUCKS_PLL1_DIV_8   = 7,
    IDUCKS_PLL2         = 8,
    IDUCKS_PLL2_DIV_2   = 9,
    IDUCKS_PLL2_DIV_3   = 0xA,
    IDUCKS_PLL2_DIV_4   = 0xB,
    IDUCKS_PLL2_DIV_5   = 0xC,
    IDUCKS_PLL2_DIV_6   = 0xD,
    IDUCKS_PLL2_DIV_7   = 0xE,
    IDUCKS_PLL2_DIV_8   = 0xF,
} E_IDU_CLOCK_SELECT;

typedef enum {
    TVCKS_PLL1          = 0,
    TVCKS_PLL1_DIV_2    = 1,
    TVCKS_PLL1_DIV_3    = 2,
    TVCKS_PLL1_DIV_4    = 3,
    TVCKS_PLL2          = 4,
    TVCKS_PLL2_DIV_2    = 5,
    TVCKS_PLL2_DIV_3    = 6,
    TVCKS_PLL2_DIV_4    = 7,
} E_TVOUT_CLOCK_SELECT;

typedef enum {
    IDU2CKS_PLL1        = 0,
    IDU2CKS_PLL1_DIV_2  = 1,
    IDU2CKS_PLL1_DIV_3  = 2,
    IDU2CKS_PLL1_DIV_4  = 3,
    IDU2CKS_PLL2        = 4,
    IDU2CKS_PLL2_DIV_2  = 5,
    IDU2CKS_PLL2_DIV_3  = 6,
    IDU2CKS_PLL2_DIV_4  = 7,
} E_IDU2_CLOCK_SELECT;

typedef enum {
    SDACKS_PLL1         = 0,
    SDACKS_PLL1_DIV_2   = 1,
    SDACKS_PLL2         = 2,
    SDACKS_PLL2_DIV_2   = 3,
} E_SDA_CLOCK_SELECT;

typedef enum {
    CDUCKS_PLL1         = 0,
    CDUCKS_PLL1_DIV_2   = 1,
    CDUCKS_PLL1_DIV_3   = 2,
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    CDUCKS_PLL1_DIV_1_5 = 3,
#else
    CDUCKS_PLL1_DIV_4   = 3,
#endif
    CDUCKS_PLL2         = 4,
    CDUCKS_PLL2_DIV_2   = 5,
    CDUCKS_PLL2_DIV_3   = 6,
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    CDUCKS_PLL2_DIV_1_5 = 7,
#else
    CDUCKS_PLL2_DIV_4   = 7,
#endif
} E_CDU_CLOCK_SELECT;

typedef enum {
    MCARDCKS_PLL1           = 0,
    MCARDCKS_PLL1_DIV_2     = 1,
    MCARDCKS_PLL1_DIV_3     = 2,
    MCARDCKS_PLL1_DIV_4     = 3,
    MCARDCKS_PLL1_DIV_5     = 4,
    MCARDCKS_PLL1_DIV_6     = 5,
    MCARDCKS_PLL1_DIV_128   = 6,
    MCARDCKS_PLL1_DIV_256   = 7,
    MCARDCKS_PLL2           = 8,
    MCARDCKS_PLL2_DIV_2     = 9,
    MCARDCKS_PLL2_DIV_3     = 0xA,
    MCARDCKS_PLL2_DIV_4     = 0xB,
    MCARDCKS_PLL2_DIV_5     = 0xC,
    MCARDCKS_PLL2_DIV_6     = 0xD,
    MCARDCKS_PLL2_DIV_128   = 0xE,
    MCARDCKS_PLL2_DIV_256   = 0xF,
} E_MCARD_CLOCK_SELECT;

//Clock Source Select 2
typedef enum {
    MPACKS_PLL1         = 0,
    MPACKS_PLL1_DIV_2   = 1,
    MPACKS_PLL1_DIV_3   = 2,
    MPACKS_PLL1_DIV_4   = 3,
    MPACKS_PLL2         = 4,
    MPACKS_PLL2_DIV_2   = 5,
    MPACKS_PLL2_DIV_3   = 6,
    MPACKS_PLL2_DIV_4   = 7,
} E_MPA_CLOCK_SELECT;

typedef enum {
    MPVCKS_PLL1         = 0,
    MPVCKS_PLL1_DIV_2   = 1,
    MPVCKS_PLL1_DIV_3   = 2,
    MPVCKS_PLL1_DIV_4   = 3,
    MPVCKS_PLL2         = 4,
    MPVCKS_PLL2_DIV_2   = 5,
    MPVCKS_PLL2_DIV_3   = 6,
    MPVCKS_PLL2_DIV_4   = 7,
} E_MPV_CLOCK_SELECT;

typedef enum {
    USBHCKS_PLL1        = 0,
    USBHCKS_PLL1_DIV_2  = 1,
    USBHCKS_PLL2        = 2,
    USBHCKS_PLL2_DIV_2  = 3,
} E_USBH_CLOCK_SELECT;

typedef enum {
    USBDCKS_PHY_DISABLE = 0,
    USBDCKS_PHY_ENABLE  = 1,
} E_USBD_CLOCK_SELECT;

typedef enum {
    USBHBCKS_PLL1_DIV_2 = 0,
    USBHBCKS_PLL1_DIV_4 = 1,
    USBHBCKS_PLL1_DIV_6 = 2,
    USBHBCKS_PLL1_DIV_8 = 3,
    USBHBCKS_PLL2_DIV_2 = 4,
    USBHBCKS_PLL2_DIV_4 = 5,
    USBHBCKS_PLL2_DIV_6 = 6,
    USBHBCKS_PLL2_DIV_8 = 7,
} E_USBHB_CLOCK_SELECT;

typedef enum {
    I2CMCKS_PLL1_DIV_4      = 0,
    I2CMCKS_PLL1_DIV_8      = 1,
    I2CMCKS_PLL1_DIV_64     = 2,
    I2CMCKS_PLL1_DIV_256    = 3,
    I2CMCKS_PLL2_DIV_4      = 4,
    I2CMCKS_PLL2_DIV_8      = 5,
    I2CMCKS_PLL2_DIV_64     = 6,
    I2CMCKS_PLL2_DIV_256    = 7,
} E_I2CM_CLOCK_SELECT;

#if (CHIP_VER_MSB == CHIP_VER_615)

typedef enum {
    IRCKS_XIN           = 0,
    IRCKS_PLL1_DIV_2,
    IRCKS_PLL2_DIV_2,
    IRCKS_0,
} E_IR_CLOCK_SELECT;

#else   // MP650/660

typedef enum {
    IRCKS_RXIN          = 0,
    IRCKS_PLL1_DIV_2,
    IRCKS_PLL2_DIV_2,
    IRCKS_XIN
} E_IR_CLOCK_SELECT;

#endif



///
///@ingroup     CLOCK_MODULE
///@brief       Get PLL's freq
///
///@param       pllIndex    select PLL index, 0~3, See E_PLL_INDEX\n
///                         For MP650 - 0~3\n
///                         For MP615 - 0~2
///@retval      PLL's frequence by Hz
///
///@remark
///
DWORD Clock_PllFreqGet(E_PLL_INDEX pllIndex);

///
///@ingroup     CLOCK_MODULE
///@brief       Setting PLL by frequency
///
///@param       pllIndex    select PLL index, 0~3, See E_PLL_INDEX\n
///                         For MP650 - 0~3\n
///                         For MP615 - 0~2
///@param       pllFreq     new PLL frequency by Hz
///
///@retval      FAIL: Setting fail
///@retval      PASS: real frequency
///
///@remark
///
SDWORD Clock_PllFreqSet(E_PLL_INDEX pllIndex, DWORD pllFreq);

///
///@ingroup     CLOCK_MODULE
///@brief       Set PLL's freq
///
///@param       pllIndex    select PLL source, See E_PLL_INDEX\n
///                         For MP650 - 0~3\n
///                         For MP615 - 0~2
///@retval      PLL's frequence by Hz
///
///@remark
///
SDWORD Clock_PllFreqSet(E_PLL_INDEX pllIndex, DWORD pllFreq);

///
///@ingroup     CLOCK_MODULE
///@brief       Get PLL's confugure
///
///@param       pllIndex    select PLL index, 0~3, See E_PLL_INDEX\n
///                         For MP650 - 0~3\n
///                         For MP615 - 0~2
///
///@retval      return PLL's configure value
///
///@remark
///
DWORD Clock_PllCfgGet(E_PLL_INDEX pllIndex);

///
///@ingroup     CLOCK_MODULE
///@brief       Set PLL by constant
///
///@param       pllIndex    select PLL index, 0~3, See E_PLL_INDEX\n
///                         For MP650 - 0~3\n
///                         For MP615 - 0~2
///@param       pllCfg      new PLL setting value
///
///@retval      FAIL: Setting fail
///@retval      PASS: real frequency
///
///@remark
///
SDWORD Clock_PllCfgSet(E_PLL_INDEX pllIndex, DWORD pllCfg);

///
///@ingroup     CLOCK_MODULE
///@brief       Get CPU's freq
///
///@param       None
///@retval      CPU's frequence by Hz
///
///@remark
///
DWORD Clock_CpuFreqGet(void);

///
///@ingroup     CLOCK_MODULE
///@brief       Set CPU's clock select
///
///@param       select      select CPU's clock source, See E_CPU_CLOCK_SELECT
///
///@retval      None
///
///@remark
///
void Clock_CpuClockSelSet(E_CPU_CLOCK_SELECT select);

///
///@ingroup     CLOCK_MODULE
///@brief       Get CPU's clock select
///
///@param       None
///
///@retval      return CPU's clock select, See E_CPU_CLOCK_SELECT
///
///@remark
///
E_CPU_CLOCK_SELECT Clock_CpuClockSelGet(void);

///
///@ingroup     CLOCK_MODULE
///@brief       Set Memory's clock select
///
///@param       select      select Memory's clock source, See E_MEM_CLOCK_SELECT
///
///@retval      None
///
///@remark
///
void Clock_MemClockSelSet(E_MEM_CLOCK_SELECT select);

///
///@ingroup     CLOCK_MODULE
///@brief       Get Memory's clock select
///
///@param       None
///
///@retval      return Memory's clock select, See E_MEM_CLOCK_SELECT
///
///@remark
///
E_MEM_CLOCK_SELECT Clock_MemClockSelGet(void);

///
///@ingroup     CLOCK_MODULE
///@brief       Get Memory's freq
///
///@param       None
///@retval      Memory's frequence by Hz
///
///@remark
///
DWORD Clock_MemFreqGet(void);

///
///@ingroup     CLOCK_MODULE
///@brief       Set IPU's clock select
///
///@param       select      select IPU's clock source, See E_IPU_CLOCK_SELECT
///
///@retval      None
///
///@remark
///
void Clock_IpuClockSelSet(E_IPU_CLOCK_SELECT select);

///
///@ingroup     CLOCK_MODULE
///@brief       Get IPU's clock select
///
///@param       None
///
///@retval      return IPU's clock select, See E_IPU_CLOCK_SELECT
///
///@remark
///
E_IPU_CLOCK_SELECT Clock_IpuClockSelGet(void);

void Clock_MemClockSelSet4Pwdc(E_MEM_CLOCK_SELECT select);

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
///
///@ingroup     CLOCK_MODULE
///@brief       Setting PLL's spread spectrum.
///
///@param       pllIndex    select PLL index, 0 or 2, See E_PLL_INDEX
///                         For MP650 - 0 or 2\n
///                         For MP615 - 0~2
///@param       enable      ENABLE(1)/DISABLE(0) the function of spread spectrum.
///@param       w           spread range (equal to Fvco Division value, N), 0~15
///@param       a           Modulation frequency parameter A, 0~31 (Duty)
///@param       b           Modulation frequency parameter B, 0~31 (Period)
///
///@remark      Only for MP650 or later.
///@remark      For EMI issue, we especially provide Spread mode to reduce electromagnetic interference in PLL clock generators.\n
///             If user enable Spread mode, Fout will spread between Fout-Org and Fout-New.\n
///             Fout-Org = Fin * (N + 1) / (M + 1), Fout-New = Fin * (N + W + 1) / (M + 1)\n
///@remark
///
void Clock_PllSprdSet(E_PLL_INDEX pllIndex, BOOL enable, DWORD w, DWORD a, DWORD b);

void Clock_PllSprdSetByFreq(E_PLL_INDEX pllIndex, BOOL enable, DWORD upBoundFreq, DWORD lowBoundPeriod, DWORD fullBoundPeriod);
#endif

SBYTE Clock_MemTypeGet(void);
void Dma_FdmaFifoTimingAdjust(BYTE maxContReq);
void Dma_PriorityDefault(void);
void Dma_FirstPrioritySet(DWORD bitMapping);
void Dma_SecondaryPrioritySet(DWORD bitMapping);
void Dma_ThirdPrioritySet(DWORD bitMapping);
void Dma_FourthPrioritySet(DWORD bitMapping);
void Dma_PriorityDefaultSave(void);
void Dma_BiuRequestSuspendSetting(BOOL enable, BOOL noLessThan4Word);
void Dma_BiuPghitCheckEnable(BOOL enable);

BYTE Clock_Dma_tWTR_Get(void);
void Clock_Dma_tWTR_Set(DWORD tWTR);
BYTE Clock_Dma_tRP_Get(void);
void Clock_Dma_tRP_Set(DWORD tRP);
BYTE Clock_Dma_tRCD_Get(void);
void Clock_Dma_tRCD_Set(DWORD tRCD);

void Clock_SystemFrequencyChange(DWORD targetPllFreq, E_PLL_INDEX targetPllIndex,
                                 SDWORD tRCD, SDWORD tRP, SDWORD tWTR,
                                 SDWORD tXSRD, SDWORD tRWD, SDWORD tWRD,
                                 SDWORD sdckDls,
                                 SDWORD ckDlyManu, SDWORD ckDlyVal,
                                 SDWORD udqsDlyManu, SDWORD udqsDlyVal,
                                 SDWORD ldqsDlyManu, SDWORD ldqsDlyVal);

void Clock_DRAM_timing_Set(SDWORD tRCD, SDWORD tRP, SDWORD tWTR, SDWORD tXSRD, SDWORD tRWD, SDWORD tWRD, SDWORD sdckDls,
                           SDWORD ckDlyManu, SDWORD ckDlyVal,
                           SDWORD udqsDlyManu, SDWORD udqsDlyVal,
                           SDWORD ldqsDlyManu, SDWORD ldqsDlyVal);

//void Clock_DqsDelay_Set(SDWORD udqsDlyManu, SDWORD udqsDlyVal, SDWORD ldqsDlyManu, SDWORD ldqsDlyVal);
void Clock_DllDelay_Set(SDWORD ck90DlyManu, SDWORD ck90DlyVal, SDWORD udqsDlyManu, SDWORD udqsDlyVal, SDWORD ldqsDlyManu, SDWORD ldqsDlyVal);
void Clock_DllDelayTuning(void);

#endif      // #ifndef __LIB_CLOCK_H

