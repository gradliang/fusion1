#ifndef __NAND_CONFIG_H__
#define __NAND_CONFIG_H__

#define NAND_CLOCK_KHZ    80000  // KHz
#undef  DDP_NAND
//#define DDP_NAND
#define NAND_PAGE_RANDOM  0

#if     (NAND_ID == HY27UG088G5M) || (NAND_ID == HY27UH08AG5M)

    #define NAND_CE_PIN_NR              2
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                TRADITIONAL

#elif   (NAND_ID == HY27US08121B) || (NAND_ID == HY27US08121A) || (NAND_ID == HY27US08561A) || (NAND_ID == HY27US08281A) || \
        (NAND_ID == NAND128W3A2BN6) || (NAND_ID == K9F1208U0M) || (NAND_ID == K9F5608U0D) || \
        (NAND_ID == MX30LF1G8AM)

    #define NAND_CE_PIN_NR              1
    #define NAND_DRV                    NAND_SIMPLE_DRV

#elif   (NAND_ID == K9LBG08U0D) || (NAND_ID == K9G8G08U0C) || (NAND_ID == K9GAG08U0D) || (NAND_ID == K9GAG08U0E) || \
        (NAND_ID == K9GAG08U0F)

    #define NAND_CE_PIN_NR              1
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                SAMSUNG_NEW

#elif   (NAND_ID == K9GBG08U0A) || (NAND_ID == K9LCG08U0A)
	// Those nand flashes should run at low frequency. And we still can not read correct ID. Must use hardcore table.
    #define NAND_CE_PIN_NR              1
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                NULL
	#undef  NAND_CLOCK_KHZ
	#define NAND_CLOCK_KHZ              40000
	#error - You must use hardcore table!!

#elif   (NAND_ID == K9HCG08U1D)

    #define NAND_CE_PIN_NR              2
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                SAMSUNG_NEW

#elif   (NAND_ID == MICRON_29F32G080CBAAA)

    #define NAND_CE_PIN_NR              1
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                MICRON_12BITS_AAA

#elif   (NAND_ID == MICRON_29F64G08CFABA) || (NAND_ID == MICRON_29F128G08CJABA)

    #define NAND_CE_PIN_NR              2
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                MICRON_12BITS_ABA

#elif   (NAND_ID == H27UAG8T2ATR)

    #define NAND_CE_PIN_NR              1
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                HYNIX_H_SERIES

#elif   (NAND_ID == H27UAG8T2BTR)

    #define NAND_CE_PIN_NR              1
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                HYNIX_HB_SERIES

#elif   (NAND_ID == H27UAG8T2CTR)

    #define NAND_CE_PIN_NR              1
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                HYNIX_HC_AG_SERIES
    #undef 	NAND_PAGE_RANDOM
	#define NAND_PAGE_RANDOM            1
	#define NAND_RANDOM_TBL_IDX         8192

#elif   (NAND_ID == H27UBG8T2BTR)

    #define NAND_CE_PIN_NR              1
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                HYNIX_HB_BG_SERIES
    #undef 	NAND_PAGE_RANDOM
	#define NAND_PAGE_RANDOM            1
	#define NAND_RANDOM_TBL_IDX         32

#elif	(NAND_ID == TC58NVG4D2ETA00)

    #define NAND_CE_PIN_NR              1
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                TOSHIBA_NAND

#elif   (NAND_ID == COMMON_NAND) || (NAND_ID == HY27UT088G2M) || (NAND_ID == HY27UF081G2A) || \
        (NAND_ID == HY27UT084G2M)|| (NAND_ID == HY27UT084G2A) || (NAND_ID == H27UAG8T2MTR) || \
        (NAND_ID == HY27UU08AG5M) || (NAND_ID == HY27UF084G2B)|| (NAND_ID == MICRON_29F8G08MAD) || \
        (NAND_ID == TC58NVG0S3ETA00) || (NAND_ID == HY27UF082G2A) || (NAND_ID == K9G4G08U0A) || \
        (NAND_ID == K9F2G08U0A) ||(NAND_ID == K9GAG08U0M) ||(NAND_ID == K9K8G08U0A) || \
        (NAND_ID == K9F1G08U0B) ||(NAND_ID == K9F1G08U0A) || (NAND_ID == K9G4G08U0B) || \
        (NAND_ID == MICRON_29F16G1608MAA) || (NAND_ID == MX30LF1208AA) || (NAND_ID == MX30LF1G08AA)

    #define NAND_CE_PIN_NR             1
    //#define                           NAND_MULTI_CE_SUPPOUR
	//#define NAND_DRV                    NAND_SIMPLE_DRV
    #define NAND_DRV                    NAND_FTL_DRV
    #define NAND_ID_TYPE                TRADITIONAL
#else
    #error "NAND TYPE DEFINE ERROR"
#endif

#endif //__NAND_CONFIG_H__
