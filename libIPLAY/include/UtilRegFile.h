/*
*****************************************************************************************
 UtilRegFile.h
 Created: 2004/11/01

 Description: define register addresses
*****************************************************************************************
*/
#ifndef UTIL_REG_FILE_H
#define UTIL_REG_FILE_H

#include "iplaysysconfig.h"
#include "BitsDefine.h"
#include "UtilTypeDef.h"

#ifdef PLATFORM800TC
#define REGISTER_BASE       BASE_CPU_REGISTER
#else
#define REGISTER_BASE       0xa8000000
#endif

/*
*****************************************************************************************
                                        AIU Registers
*****************************************************************************************
*/

#define AIU_BASE (REGISTER_BASE + 0x00014000)
typedef struct{
    volatile DWORD AiuCtl;
    volatile DWORD AiuCtl1;
    volatile DWORD AiuSta;
    volatile DWORD AcrgWr;
    volatile DWORD AcrgSta;
    volatile DWORD AcslWr;
    volatile DWORD AcslRden;
    volatile DWORD AcslSta;
    volatile DWORD res[8];			//There are no below flag in Mp615, Any problems?????		C.W 080828
    volatile DWORD AiuDacCtl;
    volatile DWORD AiuTopanaCtl;
    volatile DWORD AiuVolCtl;
    volatile DWORD AiuGainCtl;
    volatile DWORD AiuDePop;
} AIU;


/*
*****************************************************************************************
                                        BIU Registers
*****************************************************************************************
*/

#define BIU_BASE (REGISTER_BASE + 0x00008000)
typedef struct{
    volatile DWORD BiuArst;                     // 0x00
    volatile DWORD BiuSrst;                     // 0x04
    volatile DWORD BiuChid;                     // 0x08
    volatile DWORD BIU_STRAP_CFG;               // 0x0c
} BIU;


#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

#define ARST_DMA            BIT0
#define ARST_USBOTG1        BIT1
#define ARST_IPU            BIT2
#define ARST_IDU            BIT3
#define ARST_CDU            BIT4
#define ARST_MCARD          BIT5
#define ARST_UARTA          BIT6
#define ARST_USBOTG0        BIT7
#define ARST_IR             BIT8
#define ARST_AUD            BIT9
#define ARST_GPIO           BIT10
#define ARST_TIMER          BIT11
#define ARST_I2CM           BIT12
#define ARST_I2CS           BIT13
#define ARST_UARTB          BIT14
#define ARST_MPV            BIT15
#define ARST_MMCP           BIT16
#define ARST_SPI            BIT18
#define ARST_KPAD           BIT19

#else

#define ARST_I2CS           BIT13
#define ARST_I2CM           BIT12
#define ARST_IR             BIT8

#endif

/*
*****************************************************************************************
                                        CDU Registers
*****************************************************************************************
*/


#define CDU_BASE (REGISTER_BASE + 0x00022000)
typedef struct{
    volatile DWORD CduC;
    volatile DWORD CduDri;
    volatile DWORD CduSize;
    volatile DWORD CduNmcu;
    volatile DWORD CduCmps;
    volatile DWORD CduIc;
    volatile DWORD CduOp;
    volatile DWORD CduTest;
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    volatile DWORD CduCmp0;
    volatile DWORD CduCmp1;
    volatile DWORD CduCmp2;
    volatile DWORD CduCmp3;
    volatile DWORD res_0;
    volatile DWORD CduCrop0;
    volatile DWORD CduCrop1;
    volatile DWORD res_1[1];
#else
    volatile DWORD res0[8];
#endif
    volatile DWORD CduHdmt[16];
    volatile DWORD res1[32];
    volatile DWORD CduHdbt[64];
    volatile DWORD CduQt[128];
    volatile DWORD res2[256];
    volatile DWORD CduHet[512];
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    volatile DWORD CduCdr[64];
    volatile DWORD CduLhr[16];
    volatile DWORD JMCU_DMASA;
    volatile DWORD JMCU_FBWID;
    volatile DWORD res_2[2];
    volatile DWORD JVLC_DMASA;
    volatile DWORD JVLC_BUFSZ;
#endif
} CDU;

/*
*****************************************************************************************
                                        CKG Registers
*****************************************************************************************
*/
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define CLOCK_BASE (REGISTER_BASE + 0x0000a000)
typedef struct{
    volatile DWORD ClkCtrl;
    volatile DWORD Clkss1;
    volatile DWORD Clkss2;
    volatile DWORD AudClkC;
    volatile DWORD Aux0ClkC;
    volatile DWORD Aux1ClkC;
    volatile DWORD SioClkC;             // Reserved
    volatile DWORD TmClkC;
    volatile DWORD PLL1Cfg;
    volatile DWORD PLL2Cfg;
    volatile DWORD PwrPdc;
    volatile DWORD res0[3];
    volatile DWORD MdClken;
    volatile DWORD Pwrdn;
    volatile DWORD UartH_C;             // UARTHCK
    volatile DWORD PLL3Cfg;
    volatile DWORD Clkss_EXT0;
    volatile DWORD Clkss_EXT1;
    volatile DWORD Clkss_EXT2;
    volatile DWORD Clkss_EXT3;
    volatile DWORD Clkss_EXT4;
    volatile DWORD Clkss_EXT5;
    volatile DWORD PLLIduCfg;
    volatile DWORD PLL1SsCfg;
    volatile DWORD PLL2SsCfg;
    volatile DWORD PLL3SsCfg;
    volatile DWORD PLLIduCfg_EXT;
    volatile DWORD Clkss_EXT6;
    volatile DWORD Clkss_EXT7;
    volatile DWORD Clkss_EXT8;
    volatile DWORD PduEn0;              // FGPIO[0] ~ FGPIO[31]
    volatile DWORD PduEn1;              // FGPIO[32] ~ FGPIO[45], PGP1, PGP3, UGPIO[4] ~ UGPIO[7]
    volatile DWORD PduEn2;              // VGPIO[0] ~ VGPIO[27], KGPIO[0] ~ KGPIO[3]
    volatile DWORD PduEn3;              // GPIO[0] ~ GPIO[7]
    volatile DWORD PduEn4;              // AGPIO[0] ~ AGPIO[3]
    volatile DWORD PinMuxCfg;
} CLOCK;

#define CKE_UARTA           BIT30
#define CKE_IR              BIT29

#define CKE_IDU2CK          BIT26

#define CKE_TM5             BIT23
#define CKE_TM4             BIT22
#define CKE_TM3             BIT21
#define CKE_TM2             BIT20
#define CKE_TM1             BIT19
#define CKE_TM0             BIT18
#define CKE_SDIO            BIT17
#define CKE_AUDB            BIT16
#define CKE_AUDM            BIT15
#define CKE_UARTB           BIT14
#define CKE_MCARD           BIT13
#define CKE_CDU             BIT12
#define CKE_SENIN           BIT11
#define CKE_SENTO           BIT10
#define CKE_LVDS            BIT9
#define CKE_IDUCK           BIT8
#define CKE_IPUYUVP         BIT7
#define CKE_IPUSCL          BIT6
#define CKE_IPURGBP         BIT5

#define CKE_I2CS            BIT3
#define CKE_I2CM            BIT2
#define CKE_VIDEO           BIT1

#define CKE_ADC0            BIT4
#define CKE_ADC1            BIT5
#define CKE_ADC2            BIT6
#define CKE_ADC3            BIT7

#else   //Not MP650

#define CLOCK_BASE (REGISTER_BASE + 0x0000a000)
typedef struct{
    volatile DWORD ClkCtrl;
    volatile DWORD Clkss1;
    volatile DWORD Clkss2;
    volatile DWORD AudClkC;
    volatile DWORD Aux0ClkC;
    volatile DWORD Aux1ClkC;
    volatile DWORD SioClkC;
    volatile DWORD TmClkC;
    volatile DWORD PLL1Cfg;
    volatile DWORD PLL2Cfg;
    volatile DWORD res0[4];
    volatile DWORD MdClken;
    volatile DWORD Pwrdn;
    // new registers of MP615
    volatile DWORD UartH_C;
    volatile DWORD PLL3Cfg;
    volatile DWORD Clkss_EXT0;
    volatile DWORD Clkss_EXT1;
} CLOCK;

#define CKE_IR              BIT29
#define CKE_TM5             BIT23
#define CKE_TM4             BIT22
#define CKE_TM3             BIT21
#define CKE_TM2             BIT20
#define CKE_TM1             BIT19
#define CKE_TM0             BIT18
#define CKE_I2CS            BIT3
#define CKE_I2CM            BIT2

#endif

/*
*****************************************************************************************
                                        DMA Registers
*****************************************************************************************
*/

#define DMA_BASE (REGISTER_BASE + 0x0000c000)
typedef struct{
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    volatile DWORD res;                         // 0x00
    volatile DWORD SdRefCtl;                    // 0x04
    volatile DWORD SdramCtl;                    // 0x08
    volatile DWORD res0;                        // 0x0C
    volatile DWORD DllCtl;                      // 0x10
    volatile DWORD res1;                        // 0x14
    volatile DWORD Qdiv4;                       // 0x18
    volatile DWORD FDMACTL_EXT0;                // 0x1C
    volatile DWORD res3;                        // 0x20
    volatile DWORD FDMACTL_EXT1;                // 0x24
    //volatile DWORD ChaseTrg;                    // 0x28
    volatile DWORD JPG2SCLCtl0;                 // 0x28
    //volatile DWORD ChaseSize;                   // 0x2C
    volatile DWORD JPG2SCLCtl1;                 // 0x2C
    volatile DWORD FDMACTL_EXT2;                // 0x30
    volatile DWORD FDMACTL_EXT3;                // 0x34
    volatile DWORD FDMACTL_EXT4;                // 0x38
    volatile DWORD res4;                        // 0x3C
#else   // not MP650
    volatile DWORD DmaCtl;
    volatile DWORD SdRefCtl;
    volatile DWORD SdramCtl;
    volatile DWORD BtBtc;
    volatile DWORD DllCtl;
    volatile DWORD PbBtc;
    volatile DWORD res0;
    volatile DWORD PdmaCtl;
    volatile DWORD res1;
    volatile DWORD PbIc;

#endif
} DMA;

#define DMA_IDU_BASE      (REGISTER_BASE + 0x0000c040)
#define DMA_OSD_BASE      (REGISTER_BASE + 0x0000c060)
#define DMA_SCW_BASE      (REGISTER_BASE + 0x0000c080)
#define DMA_SCR_BASE      (REGISTER_BASE + 0x0000C0a0)
#define DMA_THMW_BASE     (REGISTER_BASE + 0x0000c0c0)
#define DMA_MPVR0_BASE    (REGISTER_BASE + 0x0000c0e0)
#define DMA_MPVR1_BASE    (REGISTER_BASE + 0x0000c100)
#define DMA_MPVW0_BASE    (REGISTER_BASE + 0x0000c120)
#define DMA_MPVW1_BASE    (REGISTER_BASE + 0x0000c140)
#define DMA_USBDEV_BASE   (REGISTER_BASE + 0x0000c160)
#define DMA_USBHST_BASE   (REGISTER_BASE + 0x0000c180)
#define DMA_MPAR_BASE     (REGISTER_BASE + 0x0000c1a0)
#define DMA_MPAW_BASE     (REGISTER_BASE + 0x0000c1c0)
#define DMA_INTSRAM0_BASE (REGISTER_BASE + 0x0000c1e0)
#define DMA_JMCU_BASE     (REGISTER_BASE + 0x0000c200)
#define DMA_JVLC_BASE     (REGISTER_BASE + 0x0000c220)
#define DMA_AIU_BASE      (REGISTER_BASE + 0x0000c240)
#define DMA_INTSRAM1_BASE (REGISTER_BASE + 0x0000c260)
#define DMA_MC_BASE       (REGISTER_BASE + 0x0000c280)
#define DMA_PER_BASE      (REGISTER_BASE + 0x0000c2a0)

typedef struct{
    volatile DWORD Control;                     // 0x00
    volatile DWORD Current;                     // 0x04
    volatile DWORD StartA;                      // 0x08
    volatile DWORD EndA;                        // 0x0C
    volatile DWORD StartB;                      // 0x10
    volatile DWORD EndB;                        // 0x14
    volatile DWORD LineCount;                   // 0x18
#if (CHIP_VER_MSB == CHIP_VER_615)
    volatile DWORD JvlBufOff;                   // 0x1C
#endif
} CHANNEL;

#define DMA_PRIORITY_MASK_IDU_MAIN              BIT0
#define DMA_PRIORITY_MASK_OSD                   BIT1
#define DMA_PRIORITY_MASK_IPW                   BIT2
#define DMA_PRIORITY_MASK_IPR                   BIT3
#define DMA_PRIORITY_MASK_IPR_EXT               BIT4
#define DMA_PRIORITY_MASK_VIDEO_I               BIT5
#define DMA_PRIORITY_MASK_VIDEO_II              BIT6
#define DMA_PRIORITY_MASK_MMCP                  BIT7
#define DMA_PRIORITY_MASK_USB1                  BIT9
#define DMA_PRIORITY_MASK_USB2                  BIT10
#define DMA_PRIORITY_MASK_UART                  BIT13
#define DMA_PRIORITY_MASK_JPG_MCU               BIT14
#define DMA_PRIORITY_MASK_JPG_VLC               BIT15
#define DMA_PRIORITY_MASK_AIU                   BIT16
#define DMA_PRIORITY_MASK_SDIO                  BIT17
#define DMA_PRIORITY_MASK_MCARD                 BIT18
#define DMA_PRIORITY_MASK_CPU                   BIT20

/*
*****************************************************************************************
                                        GPIO Registers
*****************************************************************************************
*/

#define GPIO_BASE (REGISTER_BASE + 0x0001c000)

#if (CHIP_VER_MSB == CHIP_VER_615)

typedef struct{
    volatile DWORD Gpdat0;
    volatile DWORD Gpdat1;
    volatile DWORD Mgpdat0;
    volatile DWORD Fgpdat[4];
    volatile DWORD Ugpdat;
    volatile DWORD Vgpdat0;
    volatile DWORD Vgpdat1;
    volatile DWORD res0[2];
    volatile DWORD Agpdat;
    volatile DWORD res1[3];
    volatile DWORD Gpcfg0;
    volatile DWORD Gpcfg1;
    volatile DWORD Mgpcfg0;
    volatile DWORD Fgpcfg[4];
    volatile DWORD Ugpcfg;
    volatile DWORD Vgpcfg0;
    volatile DWORD Vgpcfg1;
    volatile DWORD res2[2];
    volatile DWORD Agpcfg;
    volatile DWORD res3[3];
    volatile DWORD Gpint[4];
    volatile DWORD Mgpint;
} GPIO;

#else   // MP650/660

typedef struct{
    volatile DWORD Gpdat0;
    volatile DWORD Gpdat1;          // Reserved
    volatile DWORD res0[1];         // Reserved
    volatile DWORD Fgpdat[4];       // Fgpdata[3], Reserved
    volatile DWORD Ugpdat;
    volatile DWORD Vgpdat0;
    volatile DWORD Vgpdat1;
    volatile DWORD res1[2];
    volatile DWORD Agpdat;
    volatile DWORD Ogpdat;
    volatile DWORD Pgpdat;
    volatile DWORD Kgpdat;

    volatile DWORD Gpcfg0;
    volatile DWORD Gpcfg1;          // reserved
    volatile DWORD Mgpcfg0;         // reserved
    volatile DWORD Fgpcfg[4];       // Fgpcfg[3], reserved
    volatile DWORD Ugpcfg;
    volatile DWORD Vgpcfg0;
    volatile DWORD Vgpcfg1;
    volatile DWORD res2[2];
    volatile DWORD Agpcfg;
    volatile DWORD Ogpcfg;
    volatile DWORD Pgpcfg;
    volatile DWORD Kgpcfg;

    volatile DWORD Gpint[4];        // Gpint0, reserved[3]
    volatile DWORD Mgpint;          // Rename to Fgpint
    volatile DWORD res4[11];
    volatile DWORD AD_Level;
} GPIO;

#endif

/*
*****************************************************************************************
                                        IDU Registers
*****************************************************************************************
*/

#define IDU_BASE (REGISTER_BASE + 0x0001e000)
typedef struct {

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    volatile DWORD IduCtrl0;
    volatile DWORD IduSts;
    volatile DWORD HScaleCtrl0;
    volatile DWORD VScaleCtrl0;
    volatile DWORD TvHCtrl0;
    volatile DWORD TvHCtrl1;
    volatile DWORD TvVCtrl0;
    volatile DWORD TvVCtrl1;
    volatile DWORD IduCtrl1;
    volatile DWORD VScaleCtrl1;
    volatile DWORD HScaleCtrl1;
    volatile DWORD HScaleCtrl2; //reserved
    volatile DWORD CGainCtrl;
    volatile DWORD YGammaR[0x8];
    volatile DWORD TvCGainCtrl; //reserved
   // volatile DWORD TvYGammaR[0x4];
    volatile DWORD TvCtrl0;
    volatile DWORD TvCtrl1;  //reserved
    volatile DWORD CasioTG[0xC]; //reserved
    volatile DWORD Palette[0x80];
    volatile DWORD OsdHStr;
    volatile DWORD OsdHEnd;
    volatile DWORD OsdVStr;
    volatile DWORD OsdVEnd;
    volatile DWORD Iducsc0;
    volatile DWORD Iducsc1;
    volatile DWORD Iducsc2;
    volatile DWORD Iducsc3;
    volatile DWORD Dhr;
    volatile DWORD Gpov0;
    volatile DWORD Gpoh0;
    volatile DWORD Gpov1;
    volatile DWORD Gpoh1;
    volatile DWORD Gpov2;
    volatile DWORD Gpoh2;
    volatile DWORD Gpov3;
    volatile DWORD Gpoh3;
    volatile DWORD Gpov4;
    volatile DWORD Gpoh4;
    volatile DWORD Gpov5;
    volatile DWORD Gpoh5;
    volatile DWORD Gpov6;
    volatile DWORD Gpoh6;
    volatile DWORD Gpov7;
    volatile DWORD Gpoh7;
    volatile DWORD Gpov8;
    volatile DWORD Gpoh8;
    volatile DWORD Gpov9;
    volatile DWORD Gpoh9;
    volatile DWORD Gpoctrl0;
    volatile DWORD Gpoctrl1;
    //volatile DWORD Color_Ctrl0;
    volatile DWORD res2[16];
    //volatile DWORD ATCON_Ctrl0;
    //volatile DWORD res3[9];
    //volatile DWORD ATCON_com_rgb_inv;
    volatile DWORD ATCON_DAC_ctrl;
    volatile DWORD ATCON_srgb_ctrl;
    volatile DWORD RST_Ctrl_Bypass;
    volatile DWORD BG_Ctrl;
    volatile DWORD	GammaG[0x8];
    volatile DWORD	GammaB[0x8];
    volatile DWORD	Gamma_Max_Ctrl;
    volatile DWORD  res3;
    volatile DWORD  res4;
    volatile DWORD	Gamma_Min_Ctrl;
    volatile DWORD	Win_Hctrl;
    volatile DWORD	Win_Vctrl;

#else
    volatile DWORD IduCtrl0;
    volatile DWORD IduSts;
    volatile DWORD HScaleCtrl0;
    volatile DWORD VScaleCtrl0;
    volatile DWORD TvHCtrl0;
    volatile DWORD TvHCtrl1;
    volatile DWORD TvVCtrl0;
    volatile DWORD TvVCtrl1;
    volatile DWORD IduCtrl1;
    volatile DWORD VScaleCtrl1;
    volatile DWORD HScaleCtrl1;
    volatile DWORD HScaleCtrl2;
    volatile DWORD CGainCtrl;
    volatile DWORD YGammaR[0x4];
    volatile DWORD TvCGainCtrl;
    volatile DWORD TvYGammaR[0x4];
    volatile DWORD TvCtrl0;
    volatile DWORD TvCtrl1;
    volatile DWORD CasioTG[0xC];
    volatile DWORD Palette[0x80];
    volatile DWORD OsdHStr;
    volatile DWORD OsdHEnd;
    volatile DWORD OsdVStr;
    volatile DWORD OsdVEnd;

    volatile DWORD Iducsc0;
    volatile DWORD Iducsc1;
    volatile DWORD Iducsc2;
    volatile DWORD Iducsc3;
    volatile DWORD Dhr;
    volatile DWORD Gpov0;
    volatile DWORD Gpoh0;
    volatile DWORD Gpov1;
    volatile DWORD Gpoh1;
    volatile DWORD Gpov2;
    volatile DWORD Gpoh2;
    volatile DWORD Gpov3;
    volatile DWORD Gpoh3;
    volatile DWORD Gpov4;
    volatile DWORD Gpoh4;
    volatile DWORD Gpov5;
    volatile DWORD Gpoh5;
    volatile DWORD Gpov6;
    volatile DWORD Gpoh6;
    volatile DWORD Gpov7;
    volatile DWORD Gpoh7;
    volatile DWORD Gpov8;
    volatile DWORD Gpoh8;
    volatile DWORD Gpov9;
    volatile DWORD Gpoh9;
    volatile DWORD Gpoctrl0;
    volatile DWORD Gpoctrl1;


#endif
} IDU;


/*
*****************************************************************************************
                                        INT Registers
*****************************************************************************************
*/

#define INT_BASE (REGISTER_BASE + 0x0000b000)
typedef struct{
    volatile DWORD Micause;
    volatile DWORD IcauseDma;
    volatile DWORD MiMask;
    volatile DWORD ImaskDma;
} INTERRUPT;


/*
*****************************************************************************************
                                        IPU Registers
*****************************************************************************************
*/

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define IPU_BASE (REGISTER_BASE + 0x00000040)
typedef struct{
    volatile DWORD Ipu_reg_10;
    volatile DWORD Ipu_reg_11;
    volatile DWORD Ipu_reg_12;
    volatile DWORD Ipu_res0;
    volatile DWORD Ipu_reg_14;
    volatile DWORD Ipu_reg_15;
    volatile DWORD Ipu_reg_16;
    volatile DWORD Ipu_reg_17;
    volatile DWORD Ipu_reg_18;
    volatile DWORD Ipu_reg_19;
    volatile DWORD Ipu_reg_1A;
    volatile DWORD Ipu_reg_1B;
    volatile DWORD Ipu_reg_1C;
    volatile DWORD Ipu_reg_1D;
    volatile DWORD Ipu_reg_1E;
    volatile DWORD Ipu_reg_1F;
    volatile DWORD Ipu_reg_20;
    volatile DWORD Ipu_reg_21;
    volatile DWORD Ipu_reg_22;
    volatile DWORD Ipu_reg_23;
    volatile DWORD Ipu_reg_24;
    volatile DWORD Ipu_reg_25;
    volatile DWORD Ipu_reg_26;
    volatile DWORD Ipu_reg_27;
    volatile DWORD Ipu_reg_28;
    volatile DWORD Ipu_reg_29;
    volatile DWORD Ipu_reg_2A;
    volatile DWORD Ipu_reg_2B;
    volatile DWORD Ipu_res1[4];
    volatile DWORD Ipu_reg_30;
    volatile DWORD Ipu_reg_31;
    volatile DWORD Ipu_reg_32;
    volatile DWORD Ipu_reg_33;
    volatile DWORD Ipu_reg_34;
    volatile DWORD Ipu_reg_35;
    volatile DWORD Ipu_reg_36;
    volatile DWORD Ipu_reg_37;
    volatile DWORD Ipu_reg_38;
    volatile DWORD Ipu_reg_39;
    volatile DWORD Ipu_reg_3A;
    volatile DWORD Ipu_reg_3B;
    volatile DWORD Ipu_reg_3C;
    volatile DWORD Ipu_reg_3D;
    volatile DWORD Ipu_reg_3E;
    volatile DWORD Ipu_res2;
    volatile DWORD Ipu_reg_40;
    volatile DWORD Ipu_reg_41;
    volatile DWORD Ipu_reg_42;
    volatile DWORD Ipu_reg_43;
    volatile DWORD Ipu_reg_44;
    volatile DWORD Ipu_reg_45;
    volatile DWORD Ipu_reg_46;
    volatile DWORD Ipu_reg_47;
    volatile DWORD Ipu_reg_48;
    volatile DWORD Ipu_reg_49;
    volatile DWORD Ipu_reg_4A;
    volatile DWORD Ipu_reg_4B;
    volatile DWORD Ipu_reg_4C;
    volatile DWORD Ipu_reg_4D;
    volatile DWORD Ipu_res3[2];
    volatile DWORD Ipu_reg_50;
    volatile DWORD Ipu_reg_51;
    volatile DWORD Ipu_reg_52;
    volatile DWORD Ipu_reg_53;
    volatile DWORD Ipu_reg_54;
    volatile DWORD Ipu_reg_55;
    volatile DWORD Ipu_reg_56;
    volatile DWORD Ipu_reg_57;
    volatile DWORD Ipu_reg_58;
    volatile DWORD Ipu_res4[7];
    volatile DWORD Ipu_reg_60;
    volatile DWORD Ipu_reg_61;
    volatile DWORD Ipu_reg_62;
    volatile DWORD Ipu_reg_63;
    volatile DWORD Ipu_reg_64;
    volatile DWORD Ipu_reg_65;
    volatile DWORD Ipu_reg_66;
    volatile DWORD Ipu_reg_67;
    volatile DWORD Ipu_reg_68;
    volatile DWORD Ipu_reg_69;
    volatile DWORD Ipu_reg_6A;
    volatile DWORD Ipu_reg_6B;
    volatile DWORD Ipu_reg_6C;
    volatile DWORD Ipu_reg_6D;
    volatile DWORD Ipu_reg_6E;
    volatile DWORD Ipu_reg_6F;
    volatile DWORD Ipu_reg_70;
    volatile DWORD Ipu_reg_71;
    volatile DWORD Ipu_res5[14];
    volatile DWORD Ipu_reg_80;
    volatile DWORD Ipu_reg_81;
    volatile DWORD Ipu_reg_82;
    volatile DWORD Ipu_res6[13];
    volatile DWORD Ipu_reg_90;
    volatile DWORD Ipu_reg_91;
    volatile DWORD Ipu_reg_92;
    volatile DWORD Ipu_reg_93;
    volatile DWORD Ipu_reg_94;
    volatile DWORD Ipu_res7[11];
    volatile DWORD Ipu_reg_A0;
    volatile DWORD Ipu_reg_A1;
    volatile DWORD Ipu_reg_A2;
    volatile DWORD Ipu_reg_A3;
    volatile DWORD Ipu_reg_A4;
    volatile DWORD Ipu_res8[11];
    volatile DWORD Ipu_reg_B0;
    volatile DWORD Ipu_reg_B1;
    volatile DWORD Ipu_reg_B2;
    volatile DWORD Ipu_reg_B3;
    volatile DWORD Ipu_reg_B4;
    volatile DWORD Ipu_reg_B5;
    volatile DWORD Ipu_reg_B6;
    volatile DWORD Ipu_reg_B7;
    volatile DWORD Ipu_reg_B8;
    volatile DWORD Ipu_reg_B9;
    volatile DWORD Ipu_reg_BA;
    volatile DWORD Ipu_res9[53];
    volatile DWORD Ipu_reg_F0;
    volatile DWORD Ipu_reg_F1;
    volatile DWORD Ipu_reg_F2;
    volatile DWORD Ipu_reg_F3;
    volatile DWORD Ipu_reg_F4;
    volatile DWORD Ipu_reg_F5;
    volatile DWORD Ipu_reg_F6;
    volatile DWORD Ipu_reg_F7;
    volatile DWORD Ipu_reg_F8;
    volatile DWORD Ipu_res10[7];
    volatile DWORD Ipu_reg_100;
    volatile DWORD Ipu_reg_101;
    volatile DWORD Ipu_reg_102;
    volatile DWORD Ipu_reg_103;
    volatile DWORD Ipu_reg_104;
    volatile DWORD Ipu_reg_105;
    volatile DWORD Ipu_reg_106;
    volatile DWORD Ipu_reg_107;
    volatile DWORD Ipu_reg_108;
    volatile DWORD Ipu_reg_109;
    volatile DWORD Ipu_reg_10A;
    volatile DWORD Ipu_reg_10B;
} IPU;
#else
#define IPU_BASE (REGISTER_BASE + 0x00000000)
typedef struct{
    volatile DWORD IpIc;
    volatile DWORD IpIm;
    volatile DWORD IpIpr0;
    volatile DWORD IpIpr1;
    volatile DWORD IpIpr2;
    volatile DWORD IpIpw10;
    volatile DWORD IpIpw11;
    volatile DWORD res;
    volatile DWORD IpIpw13;
    volatile DWORD IpIpw14;
    volatile DWORD IpMnsc0;
    volatile DWORD IpMnsc1;
    volatile DWORD IpMnsc2;
    volatile DWORD IpMnsc3;
    volatile DWORD IpMnsc4;
    #if ((CHIP_VER & 0xffff0000) == CHIP_VER_600)
    volatile DWORD IpOut;
    volatile DWORD IpMul0Coe;
    volatile DWORD IpMul1Coe;
    volatile DWORD IpMul2Coe;
    volatile DWORD IpOffsetCoef;
    #endif
} IPU;
#endif

/*
*****************************************************************************************
                                        MCARD Registers
*****************************************************************************************
*/

#define MC_CFCOM_BASE (REGISTER_BASE + 0x00020000)
#define MC_CFATR_BASE (REGISTER_BASE + 0x00020800)
#define MCARD_BASE    (REGISTER_BASE + 0x00021000)
#define MCSDIO_BASE   (REGISTER_BASE + 0x00021200)
typedef struct{
    volatile DWORD McardC;      //2_1000
    volatile DWORD McDmarl;     //2_1004
    volatile DWORD McRtm;       //2_1008
    volatile DWORD McWtm;       //2_100c
    volatile DWORD McWdt;       //2_1010
    volatile DWORD res0[3];     //2_1014, 2_1018, 2_101c
    volatile DWORD McCfC;       //2_1020
    volatile DWORD McCfIc;      //2_1024
    volatile DWORD McCfSc;      //2_1028
    volatile DWORD res1[5];     //2_102c, 2_1030, 2_1034, 2_1038, 2_103c
    volatile DWORD McSmC;       //2_1040
    volatile DWORD McSmIc;      //2_1044
    volatile DWORD McSmCmd;     //2_1048
    volatile BYTE  McSmAdr;     //2_104c
    volatile BYTE  res2[3];
    volatile BYTE  McSmDat;     //2_1050
    volatile BYTE  res3[3];
    volatile DWORD McSmEcc1;    //2_1054
    volatile DWORD McSmEcc2;    //2_1058
    volatile DWORD SPI_Mode;    //2_105c	// For MP630
    volatile DWORD McSmEccIc;	//2_1060	// for MP650/MP660
    volatile DWORD res4[3];     //2_1064, 2_1068, 2_106c
    volatile DWORD McSdOp;      //2_1070
    volatile DWORD McSdIc;      //2_1074
    volatile DWORD McSdC;       //2_1078
    volatile DWORD McSdArg;     //2_107c
    volatile DWORD McSdRspA;    //2_1080
    volatile DWORD McSdRspB;    //2_1084
    volatile DWORD McSdRspC;    //2_1088
    volatile DWORD McSdRspD;    //2_108c
    volatile DWORD McSdSc;      //2_1090
    volatile DWORD McSdBl;      //2_1094
    volatile DWORD res5[2];     //2_1098, 2_109c
    volatile DWORD McMsCmd;     //2_10a0
    volatile DWORD McMsFifo;    //2_10a4
    volatile DWORD McMsCrc;     //2_10a8
    volatile DWORD McMsT0;      //2_10ac
    volatile DWORD McMsT1;      //2_10b0
    volatile DWORD McMsStatus;  //2_10b4
    volatile DWORD McMsIc;      //2_10b8
    volatile DWORD res6[1];     // 10BC
    volatile DWORD res7[4];     // 10CC
    volatile DWORD res8[4];     // 10DC
    volatile DWORD res9[4];     // 10EC
    volatile DWORD res10[4];    // 10FC
    volatile DWORD McEcc4SC;    // 1100
    volatile DWORD McEcc4SState; // 1104
    volatile DWORD McEcc4SCrc74; // 1108
    volatile DWORD McEcc4SCrc41; // 110C
    volatile DWORD McEcc4SCrc10; // 1110
    volatile DWORD McEcc4SErr[4]; // 1114 ~1120
    volatile DWORD McEcc4SSA0[4]; // 1124 ~ 1130
    volatile DWORD MC_ECC8S_C;  // 1134
    volatile DWORD res11[2];    // 1138, 113c
    volatile DWORD MC_ECC8S_STATE;  // 1140
    volatile DWORD MC_ECC8S_CRC[6]; // 1144, 1148, 114c, 1150, 1154, 1158
    volatile DWORD MC_ECC8S_ERR[8]; // 115c, 1160, 1164, 1168, 116c, 1170, 1174, 1178
} MCARD;

/*
*****************************************************************************************
                                        ECC extension Registers
*****************************************************************************************
*/
#define MC_ECC_BASE 			(REGISTER_BASE + 0x00020000 + 0x1330)

typedef struct MC_ECC_RESULT
{
	volatile DWORD MC_ECC_ERR_NUM;
	volatile DWORD MC_ECC_ERR[12];
	volatile DWORD reserved[(0x40-0x34)>>2];
} McEccResult;

typedef struct MC_ECC
{
	volatile DWORD MC_ECC_OPCT[3];		// 0x1330
	volatile DWORD MC_ECCBCH_C;			// 0x133c
	volatile DWORD MC_ECC_CNFG;			// 0x1340
	volatile DWORD MC_FTL_NUMBER;		// 0x1344
	volatile DWORD MC_FTL_REG[8];		// 0x1348
	volatile DWORD MC_FTL_ECCDATA_RD;	// 0x1368
	volatile DWORD MC_FTL_ECCDATA_EXE;	// 0x136c
	volatile DWORD reserved1[(0x137c-0x1370)>>2];
	volatile DWORD MC_ECC_PASSFAIL;		// 0x137c
	volatile DWORD MC_ECC_ERRCNT[4];	// 0x1380~0x138c
	volatile DWORD MC_AUTO_ALE_CYCNT;	// 0x1390
	volatile DWORD MC_AUTO_ALE_DAT[4];	// 0x1394
	volatile DWORD MC_AUTO_CMD_DAT;		// 0x13a4
	volatile DWORD MC_AUTO_PAR[2];		// 0x13a8,0x13ac
	volatile DWORD reserved3[(0x1800-0x13b0)>>2];
	volatile DWORD MC_ECC_CRC[112];		// 0x1800~0x19bc
	volatile DWORD reserved4[(0x1a00-0x19c0)>>2];
	volatile McEccResult MC_ECC_ERR[16];	// 0x1a00
}McardEcc;


#define ATA_BASE (REGISTER_BASE + 0x00021f38)
typedef struct{
    volatile DWORD AtaAstsDctl;     //6
    volatile DWORD res0;
    volatile DWORD AtaData;         //0
    volatile DWORD AtaErrorFeature; //1
    volatile DWORD AtaSectorCnt;    //2
    volatile DWORD AtaLbaL;         //3
    volatile DWORD AtaLbaM;         //4
    volatile DWORD AtaLbaH;         //5
    volatile DWORD AtaDevice;       //6
    volatile DWORD AtaStatusCommand;//7
    volatile DWORD res1[8];
    volatile DWORD AtaDmaCtl;
    volatile DWORD AtaPioTim;
    volatile DWORD AtaMdmaTim;
    volatile DWORD AtaUdmaTim;
    volatile DWORD AtaSta;
    volatile DWORD AtaIc;
    volatile DWORD AtaCrc;
} ATA;


typedef struct
{
    volatile BYTE Data;
    volatile BYTE ErrorFeature;
    volatile BYTE SectorCount;
    volatile BYTE Lba0;                     // alias to sector number register
    volatile BYTE Lba1;                     // alias to cylinder low register
    volatile BYTE Lba2;                     // alias to cylinder high register
    volatile BYTE Lba3;                     // alias to drive and head register
    volatile BYTE StatusCommand;
    BYTE Res0[6];
    volatile BYTE AltStatus;
    BYTE Res1;
} CFATA;

/*
*****************************************************************************************
                                        MPA Registers
*****************************************************************************************
*/

#define MPA_BASE (REGISTER_BASE + 0x00034000)
typedef struct{
    volatile DWORD MpaCtrl;
    volatile DWORD MpaStatus;
    volatile DWORD MpaIcause;
    volatile DWORD MpaImask;
    volatile DWORD MsMode;
    volatile DWORD Ismsmixed;
    volatile DWORD Mp3Id;
    volatile DWORD IsBound;
    volatile DWORD Version;
    volatile DWORD GlobalGain;
    volatile DWORD ScalefacScale;
    volatile DWORD Preflag;
    volatile DWORD BlkType;
    volatile DWORD MixedBlkFlag;
    volatile DWORD SubGain0;
    volatile DWORD SubGain1;
    volatile DWORD Voffset;
    volatile DWORD Mono;
    volatile DWORD Bypass;
    volatile DWORD Op_Mode;
    volatile DWORD FreqSmpl0;
    volatile DWORD FreqSmpl1;

    volatile DWORD res0[10];
    volatile DWORD WinGrpLenL;
    volatile DWORD MsMaskPresentL;
    volatile DWORD NumWindowGroupsL;
    volatile DWORD NumSwbL;
    volatile DWORD MaxSfbL;
    volatile DWORD PnsUsedL;
    volatile DWORD TnsUsedL;
    volatile DWORD TnsL;
    volatile DWORD TnsL_N[8][3];
    volatile DWORD res1[8];
    volatile DWORD WinGrpLenR;
    volatile DWORD MsMaskPresentR;
    volatile DWORD NumWindowGroupsR;
    volatile DWORD NumSwbR;
    volatile DWORD MaxSfbR;
    volatile DWORD PnsUsedR;
    volatile DWORD TnsUsedR;
    volatile DWORD TnsR;
    volatile DWORD TnsR_N[8][3];
    volatile DWORD res2[24];
    volatile DWORD ScalefacMb;
    volatile DWORD SectsfbMb;
    volatile DWORD SfbcbMb;
    volatile DWORD SideinfoMb;
    volatile DWORD DiSwinMb;
    volatile DWORD MbRst;
    // new registers of MP615
    volatile DWORD EqSet;
    volatile DWORD EqG0;
    volatile DWORD EqG1;
    volatile DWORD EqG2;
    volatile DWORD EqG3;
    volatile DWORD EqG4;
    volatile DWORD EqG5;
    volatile DWORD EqG6;
    volatile DWORD EqG7;
} MPA;


/*
*****************************************************************************************
                                        MPV Registers
*****************************************************************************************
*/
#define MPV_BASE (REGISTER_BASE + 0x00030000)
typedef struct{
    volatile DWORD CtrlReg;
    volatile DWORD ParaReg;
    volatile DWORD StatusReg;
    volatile DWORD IcReg;
    volatile DWORD CoeFifoReg;
    volatile DWORD MvFifoReg;
    // new registers of MP615
    volatile DWORD VopinfReg;
    volatile DWORD DbkinfReg;
    volatile DWORD res1[8];
    volatile DWORD VldCtrlReg;
    volatile DWORD VldStartReg;
    volatile DWORD VldParaReg;
    volatile DWORD VldMbInfReg;
    volatile DWORD VldMbRangeReg;
    volatile DWORD VldMbStaReg;
    volatile DWORD VldSysMReg;
    volatile DWORD VldFlushReg;
    volatile DWORD VldRdFlushFlcReg;
    volatile DWORD VldRdFlushVlcReg;
    volatile DWORD VldSysRFlcReg;
    volatile DWORD VldSysRVlcReg;
    volatile DWORD VldPassWordCountReg;
    volatile DWORD VldDivxTabReg;
} MPV;


/*
*****************************************************************************************
                                        RTC Registers
*****************************************************************************************
*/

#if (CHIP_VER_MSB == CHIP_VER_615)

#define RTC_BASE (REGISTER_BASE + 0x0001a000)
typedef struct{
    volatile DWORD RtcModeEna;
    volatile DWORD RtcCtl;
    volatile DWORD RtcCnt;
    volatile DWORD RtcAlarm;
    //volatile DWORD RtcRam[64];
} RTC;

#else

#define RTC_BASE (REGISTER_BASE + 0x0001BFC0)
typedef struct{
    volatile DWORD RtcModeEna;
    volatile DWORD RtcCtl;
    volatile DWORD RtcCnt;
    volatile DWORD RtcAlarm;
    volatile DWORD RtcExt0;
    volatile DWORD RtcExt1;
} RTC;

#endif



/*
*****************************************************************************************
                                        SIO Registers
*****************************************************************************************
*/

#define SIO_BASE (REGISTER_BASE + 0x00010000)
typedef struct{
    volatile DWORD Sio1C;
    volatile DWORD Sio1Ic;
    volatile DWORD SiodTr1;
    volatile DWORD SiodRr1;
    volatile DWORD Sio2C;
    volatile DWORD Sio2Ic;
    volatile DWORD SiodTr2;
    volatile DWORD SiodRr2;
} SIO;


/*
*****************************************************************************************
                                        TIMER Registers
*****************************************************************************************
*/

#define TIMER0_BASE   (REGISTER_BASE + 0x0000e000)
#define TIMER1_BASE   (REGISTER_BASE + 0x0000e020)
#define TIMER2_BASE   (REGISTER_BASE + 0x0000e040)
#define TIMER3_BASE   (REGISTER_BASE + 0x0000e060)
#define TIMER4_BASE   (REGISTER_BASE + 0x0000e080)
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define TIMER5_BASE   (REGISTER_BASE + 0x0000e0a0)
#endif

typedef struct{
    volatile DWORD TmC;
    volatile DWORD TmV;
    volatile DWORD TcA;
    volatile DWORD TcB;
    volatile DWORD Tpc;
    volatile DWORD Tmoff;
} TIMER;



/*
*****************************************************************************************
                                        Watch Dog Registers
*****************************************************************************************
*/
#if ((CHIP_VER_MSB == CHIP_VER_615))
#define WATCHDOG_BASE (REGISTER_BASE + 0x0000e0a0)
typedef struct{
    volatile DWORD WdTc;
    volatile DWORD WdTv;
    volatile DWORD WdTt;
} WATCHDOG;
#endif

/*
*****************************************************************************************
                                        UART Registers
*****************************************************************************************
*/

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define HUART1_BASE (REGISTER_BASE + 0x00012000)
#define HUART2_BASE (REGISTER_BASE + 0x00012100)

typedef struct{
    volatile DWORD HUartC;
    volatile DWORD HUartWrBuf;
    volatile DWORD HUartRdBuf;
    volatile DWORD HUartBufCtl;
    volatile DWORD HUartDmaLen;
    volatile DWORD HUartDmaReLen;
    volatile DWORD HUartIntCtl;
    volatile DWORD HUartRBufCnt;
    volatile DWORD HUartRxDmaSA;
    volatile DWORD HUartRxDmaEA;
    volatile DWORD HUartRxDmaCurA;
    volatile DWORD HUartTxDmaSA;
    volatile DWORD HUartTxDmaEA;
    volatile DWORD HUartTxDmaCurA;
} HUART;


#else
#define UART_BASE (REGISTER_BASE + 0x00012000)

typedef struct{
    volatile DWORD UartC;
    volatile DWORD UartInt;
    volatile DWORD UartData;
    volatile DWORD UartFifoCnt;
    volatile DWORD res0[4];
    volatile DWORD HUartC;
    volatile DWORD HUartWrBuf;
    volatile DWORD HUartRdBuf;
    volatile DWORD HUartBufCtl;
    volatile DWORD HUartDmaLen;
    volatile DWORD HUartDmaReLen;
    volatile DWORD HUartIntCtl;
    volatile DWORD HUartRBufPtr;
    volatile DWORD HUartDmaSA;
    volatile DWORD HUartDmaEA;
    volatile DWORD reserved0[4];
    volatile DWORD HUartFctlRxTimeOut;
} UART;
#endif

/*
*****************************************************************************************
                                        USB OTG Registers
*****************************************************************************************
*/
//#define USB_OTG_BASE (REGISTER_BASE + REGISTER_USB_BASE)
#define USBOTG0_BASE (REGISTER_BASE + 0x00018000)
#define USBOTG1_BASE (REGISTER_BASE + 0x00019000)
typedef struct {
    volatile DWORD HcCapability;                         // 0x000
    volatile DWORD HcStructuralParameters;               // 0x004
    volatile DWORD HcCapabilityParameters;               // 0x008
    volatile DWORD reserved1;                            // 0x00C

    volatile DWORD HcUsbCommand;                         // 0x010
    volatile DWORD HcUsbStatus;                          // 0x014
    volatile DWORD HcUsbInterruptEnable;                 // 0x018
    volatile DWORD HcFrameIndex;                         // 0x01C

    volatile DWORD reserved2;                            // 0x020
    volatile DWORD HcPeriodicFrameListBaseAddress;       // 0x024
    volatile DWORD HcCurrentAsynListAddress;             // 0x028
    volatile DWORD reserved3;                            // 0x02C

    volatile DWORD HcPortStatusAndControl;               // 0x030
    volatile DWORD reserved4;                            // 0x034
    volatile DWORD reserved5;                            // 0x038
    volatile DWORD reserved6;                            // 0x03C

    volatile DWORD HcMisc;                               // 0x040
    volatile DWORD reserved7[15];                        // 0x044 ~ 0x07F

    volatile DWORD OtgControlStatus;                     // 0x080
    volatile DWORD OtgInterruptStatus;                   // 0x084
    volatile DWORD OtgInterruptEnable;                   // 0x088
    volatile DWORD reserved8[13];                        // 0x08C ~ 0x0BF

    volatile DWORD GlobalHcOtgDevInterruptStatus;        // 0x0C0
    volatile DWORD GlobalMaskofHcOtgDevInterrupt;        // 0x0C4
    volatile DWORD reserved9[14];                        // 0x0C8 ~ 0x0FF

    volatile DWORD DeviceMainControl;                    // 0x100
    volatile DWORD DeviceAddress;                        // 0x104
    volatile DWORD DeviceTest;                           // 0x108
    volatile DWORD DeviceSofFrameNumber;                 // 0x10C

    volatile DWORD DeviceSofMaskTimer;                   // 0x110
    volatile DWORD PhyTestModeSelector;                  // 0x114
    volatile DWORD DeviceVendorSpecificIoControl;        // 0x118
    volatile DWORD DeviceCxCfgAndStatus;                 // 0x11C

    volatile DWORD DeviceCxCfgAndFifoEmptyStatus;        // 0x120
    volatile DWORD DeviceIdleCounter;                    // 0x124
    volatile DWORD reserved10;                           // 0x128
    volatile DWORD reserved11;                           // 0x12C

    volatile DWORD DeviceMaskofInterruptGroup;           // 0x130
    volatile DWORD DeviceMaskofInterruptSrcGroup0;       // 0x134
    volatile DWORD DeviceMaskofInterruptSrcGroup1;       // 0x138
    volatile DWORD DeviceMaskofInterruptSrcGroup2;       // 0x13C

    volatile DWORD DeviceInterruptGroup;                 // 0x140
    volatile DWORD DeviceInterruptSourceGroup0;          // 0x144
    volatile DWORD DeviceInterruptSourceGroup1;          // 0x148
    volatile DWORD DeviceInterruptSourceGroup2;          // 0x14C

    volatile DWORD DeviceRxZeroLengthDataPacket;         // 0x150
    volatile DWORD DeviceTxZeroLengthDataPacket;         // 0x154
    volatile DWORD DeviceIsocSequentialErrorAbort;       // 0x158
    volatile DWORD reserved12;                           // 0x15C

    volatile DWORD DeviceInEndpoint1MaxPacketSize;       // 0x160
    volatile DWORD DeviceInEndpoint2MaxPacketSize;       // 0x164
    volatile DWORD DeviceInEndpoint3MaxPacketSize;       // 0x168
    volatile DWORD DeviceInEndpoint4MaxPacketSize;       // 0x16C

    volatile DWORD DeviceInEndpoint5MaxPacketSize;       // 0x170
    volatile DWORD DeviceInEndpoint6MaxPacketSize;       // 0x174
    volatile DWORD DeviceInEndpoint7MaxPacketSize;       // 0x178
    volatile DWORD DeviceInEndpoint8MaxPacketSize;       // 0x17C

    volatile DWORD DeviceOutEndpoint1MaxPacketSize;      // 0x180
    volatile DWORD DeviceOutEndpoint2MaxPacketSize;      // 0x184
    volatile DWORD DeviceOutEndpoint3MaxPacketSize;      // 0x188
    volatile DWORD DeviceOutEndpoint4MaxPacketSize;      // 0x18C

    volatile DWORD DeviceOutEndpoint5MaxPacketSize;      // 0x190
    volatile DWORD DeviceOutEndpoint6MaxPacketSize;      // 0x194
    volatile DWORD DeviceOutEndpoint7MaxPacketSize;      // 0x198
    volatile DWORD DeviceOutEndpoint8MaxPacketSize;      // 0x19C

    volatile DWORD DeviceEndpoint1to4Map;                // 0x1A0
    volatile DWORD DeviceEndpoint5to8Map;                // 0x1A4
    volatile DWORD DeviceFifoMap;                        // 0x1A8
    volatile DWORD DeviceFifoConfiguration;              // 0x1AC

    volatile DWORD DeviceFifo0InstructionAndByteCount;    // 0x1B0
    volatile DWORD DeviceFifo1InstructionAndByteCount;    // 0x1B4
    volatile DWORD DeviceFifo2InstructionAndByteCount;    // 0x1B8
    volatile DWORD DeviceFifo3InstructionAndByteCount;    // 0x1BC

    volatile DWORD DeviceDmaTargetFifNumber;             // 0x1C0
    volatile DWORD reserved13;                           // 0x1C4
    volatile DWORD DeviceDmaControllerParameterSetting1; // 0x1C8
    volatile DWORD DeviceDmaControllerParameterSetting2; // 0x1CC

    volatile DWORD DeviceDmaControllerParameterSetting3; // 0x1D0
    volatile DWORD DmaControllerStatus;                  // 0x1D4
    //volatile DWORD DeviceDmaControllerParameterSetting4; // 0x1D8
    //volatile DWORD DeviceDmaControllerParameterSetting5; // 0x1DC

    //volatile DWORD reserved14[4];                      // 0x1E0 ~ 0x1EC
    volatile DWORD reserved14[2];                        // 0x1D8 ~ 0x1DC
    volatile DWORD SwapBufferStart1;                       // 0x1E0
    volatile DWORD SwapBufferEnd1;                         // 0x1E4
    volatile DWORD SwapBufferStart2;                       // 0x1E8
    volatile DWORD SwapBufferEnd2;                         // 0x1EC
    volatile DWORD WrapperCtrl;                          // 0x1F0 // bit 0:wakeup; bit25:Enable 2; bit24:Enable 1
    volatile DWORD PhyUtmiSwCtrl;                        // 0x1F4 // USBOTG PHY Control for ChipIdea
    volatile DWORD RgEco;                               // 0x1F8
    volatile DWORD reserved15;                          // 0x1FF
} USB_OTG, *PUSB_OTG;

/*
*****************************************************************************************
                                        IR Registers
*****************************************************************************************
*/

#define IR_BASE (REGISTER_BASE + 0x00010040)

typedef struct{
    volatile DWORD IrCtrl;
    volatile DWORD IrStatus;
    volatile DWORD IrData;
    volatile DWORD res0;
    volatile DWORD IrTimeBase;
    volatile DWORD IrLStart;
    volatile DWORD IrLRepeat;
    volatile DWORD IrLBit1;
    volatile DWORD IrLBit0;
    volatile DWORD IrTSetting;
    volatile DWORD IrExpected;
    volatile DWORD IrOthers;
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    volatile DWORD IrExpectedData;
    volatile DWORD IrCtrlPin;
#endif
} IR;


/*
*****************************************************************************************
                                        SRAMGP Registers
*****************************************************************************************
*/
//#define SRMGP_BASE 0xafc08004;
#define SRMGP_BASE 0xafc08000;
typedef struct{
    volatile DWORD SRMFill;
    volatile DWORD SRMFillVal;
    volatile DWORD StartAddr0;
    volatile DWORD EndAddr0;
    volatile DWORD StartAddr1;
    volatile DWORD EndAddr1;
    volatile DWORD ToVal;
    volatile DWORD DMACfg;
    volatile DWORD DMAStatus;
} SRMGP;


/*
*****************************************************************************************
                                  I2C Master Registers
*****************************************************************************************
*/
#define I2C_MR_BASE (REGISTER_BASE + 0x00010080)
typedef struct{
    volatile DWORD I2C_MReg60;          // I2C_MReg1
    volatile DWORD I2C_MReg61;          // I2C_MReg2
    volatile DWORD I2C_MReg62;          // I2C_MReg3
    volatile DWORD I2C_MReg63;          // I2C_MReg4
    volatile DWORD I2C_MReg64;          // I2C_MReg5
} M_I2C_REG;



/*
*****************************************************************************************
                                  I2C Slave Registers
*****************************************************************************************
*/
#define S_I2C_BASE (REGISTER_BASE + 0x00010100)
typedef struct{
    volatile DWORD I2C_SWReg1;
    volatile DWORD I2C_SWReg2;
    volatile DWORD I2C_SWReg3;
    volatile DWORD I2C_SWReg4;
    volatile DWORD I2C_SWHandShake;
    volatile DWORD I2C_SIDReg;
    volatile DWORD I2C_res1;
    volatile DWORD I2C_res2;
    volatile DWORD I2C_SRReg1;
    volatile DWORD I2C_SRReg2;
    volatile DWORD I2C_SRReg3;
    volatile DWORD I2C_SRReg4;
    volatile DWORD I2C_SRHandShake;
} S_I2C_REG;



/*
*****************************************************************************************
                                  SPI Registers
*****************************************************************************************
*/
#define SPI_BASE (REGISTER_BASE + 0x00024000)
typedef struct{
    volatile DWORD SPI_CFG;                         // 0x00
    volatile DWORD SPI_RCMD;                        // 0x04
    volatile DWORD SPI_WCMD;                        // 0x08
    volatile DWORD SPI_TX0;                         // 0x0c
    volatile DWORD SPI_TX1;                         // 0x10
    volatile DWORD SPI_RX0;                         // 0x14
    volatile DWORD SPI_RX1;                         // 0x18
    volatile DWORD SPI_TIME;                        // 0x1c
} SPI;



/*
*****************************************************************************************
                                  MMCP Registers
*****************************************************************************************
*/
#define MMCP_BASE (REGISTER_BASE + 0x00034000)
typedef struct{
    volatile DWORD MMCP_CFG;                        // 0x00
    volatile DWORD SRC_ADDR;                        // 0x04
    volatile DWORD DST_ADDR;                        // 0x08
    volatile DWORD DATA_LEN;                        // 0x0c
    volatile DWORD SRC_INCR;                        // 0x10
    volatile DWORD DST_INCR;                        // 0x14
    volatile DWORD LOOP_CNT;                        // 0x18
    volatile DWORD WOM_DATA;                        // 0x1c
} MMCP;



// define master interrupt cause
#define IC_DMA          BIT0
#define IC_SCA          BIT1
#define IC_IDU          BIT2
#define IC_USBDEV       BIT3
#define IC_USBHST       BIT4
#define IC_USB0         BIT3
#define IC_USB1         BIT4

#define IC_UART         BIT5
#define IC_JPEG         BIT6
#define IC_MPV          BIT7
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define IC_PWDC         BIT8
#else
#define IC_MPA          BIT8
#endif
#define IC_GPIO         BIT9
#define IC_MCARD        BIT10
#define IC_AIU          BIT11
#define IC_TM0          BIT12
#define IC_TM1          BIT13
#define IC_TM2          BIT14
#define IC_TM3          BIT15
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define IC_SDIO_EXT     BIT16
#define IC_UART2        BIT17
#define IC_KPAD         BIT18
#endif
#define IC_RTC          BIT19
#define IC_I2CSR        BIT20
#define IC_I2CSW        BIT21
#define IC_IR           BIT22
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define IC_FGPIO        BIT23
#define IC_TM4          BIT24
#define IC_TM5          BIT25
#define IC_MMCP         BIT26
#endif
#if (CHIP_VER_MSB == CHIP_VER_660)
#define IC_I2CM         BIT27
#endif

// define DMA interrupt cause
#define IC_IDUDM        BIT0
#define IC_OSDDM        BIT1
#define IC_SCWDM        BIT2
#define IC_SCRDM        BIT3
#define IC_THMWDM       BIT4
#define IC_MPVR0DM      BIT5
#define IC_MPVR1DM      BIT6
#define IC_MPVW0DM      BIT7
#define IC_MPVW1DM      BIT8
#define IC_UDEVDM       BIT9
#define IC_UHSTDM       BIT10
#define IC_MPARDM       BIT11
#define IC_MPAWDM       BIT12
#define IC_SRAM0DM      BIT13
#define IC_JMCUDM       BIT14
#define IC_JVLCDM       BIT15
#define IC_AIUDM        BIT16
#define IC_SRAM1DM      BIT17
#define IC_MCRDDM       BIT18
#define IC_PERIDM       BIT19

// define master interrupt mask
#define IM_DMA          BIT0
#define IM_IPU          BIT1
#define IM_IDU          BIT2
#define IM_USBD         BIT3
#define IM_USBH         BIT4
#define IM_USB0         BIT3
#define IM_USB1         BIT4
#define IM_UART         BIT5
#define IM_JPEG         BIT6
#define IM_MPV          BIT7
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define IM_PWDC         BIT8
#else
#define IM_MPA          BIT8
#endif
#define IM_GPIO         BIT9
#define IM_MCARD        BIT10
#define IM_AIU          BIT11
#define IM_TM0          BIT12
#define IM_TM1          BIT13
#define IM_TM2          BIT14
#define IM_TM3          BIT15
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define IM_SDIO_EXT     BIT16
#define IM_UART2        BIT17
#define IM_KPAD         BIT18
#endif
#define IM_RTC          BIT19
#define IM_I2CSR        BIT20
#define IM_I2CSW        BIT21
#define IM_IR           BIT22
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define IM_FGPIO        BIT23
#define IM_TM4          BIT24
#define IM_TM5          BIT25
#define IM_MMCP         BIT26
#endif
#if (CHIP_VER_MSB == CHIP_VER_660)
#define IM_I2CM         BIT27
#endif

// define DMA interrupt mask
#define IM_IDUDM        BIT0
#define IM_OSDDM        BIT1
#define IM_SCWDM        BIT2
#define IM_SCRDM        BIT3
#define IM_THMWDM       BIT4
#define IM_MPVR0DM      BIT5
#define IM_MPVR1DM      BIT6
#define IM_MPVW0DM      BIT7
#define IM_MPVW1DM      BIT8
#define IM_UDEVDM       BIT9
#define IM_UHSTDM       BIT10
#define IM_MPARDM       BIT11
#define IM_MPAWDM       BIT12
#define IM_SRAM0DM      BIT13
#define IM_JMCUDM       BIT14
#define IM_JVLCDM       BIT15
#define IM_AIUDM        BIT16
#define IM_SRAM1DM      BIT17
#define IM_MCRDDM       BIT18
#define IM_PERIDM       BIT19

#endif

