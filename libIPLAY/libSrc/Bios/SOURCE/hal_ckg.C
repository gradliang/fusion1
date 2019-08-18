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
* Filename      : hal_Ckg.c
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
#include "hal_ckg.h"

#define PLL_ADJUST_STABLE_TIME              100000
#define CPU_CLK_ADJUST_STABLE_TIME          100000
#define MEM_CLK_ADJUST_STABLE_TIME          100000

#if (CHIP_VER_LSB == CHIP_VER_FPGA)
    #define CKG_CHIP_IS_FPGA
#endif

#define PLL1_EN                             BIT0
#define PLL2_EN                             BIT4
#define PLL3_EN                             BIT24
#define PLLIDU_EN                           BIT27

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define PLL1_OE                             BIT17
#define PLL2_OE                             BIT18
#define PLL3_OE                             BIT19
#define PLLIDU_OE                           BIT20

#define PLL1_RESET                          BIT7
#define PLL2_RESET                          BIT8
#define PLL3_RESET                          BIT30
#define PLLIDU_RESET                        BIT31
#endif

static void adjustPllCfg(register DWORD *, register DWORD); __attribute__((section(".adjustpll")))

static void adjustPllCfg(register DWORD *pllCfgRegAddr, register DWORD pllCfg)
{
    register DWORD counter;

    *(volatile DWORD *) pllCfgRegAddr = pllCfg;
    for (counter = 0; counter < PLL_ADJUST_STABLE_TIME; counter++);
}



static void adjustMemCs(register DWORD *, register DWORD); __attribute__((section(".adjustmemcs")))

static void adjustMemCs(register DWORD *memCsRegAddr, register DWORD memSel)
{
    register DWORD counter;

    *(volatile DWORD *) memCsRegAddr = memSel;
    for (counter = 0; counter < MEM_CLK_ADJUST_STABLE_TIME; counter++);
}



DWORD calculatePllParam(BOOL usingNewPll, DWORD oscFreq, DWORD dwPllFreq, DWORD *pdwPllCfg)
{
    DWORD finalDivN, finalDivM, finalDivider;
    DWORD finalVcoGain = 7, finalChargePump = 7;
    DWORD tmpDivN, tmpDivM, tmpDivider;
    DWORD tmpFreq, tolerance;
    DWORD finialTolerance = (DWORD) -1, finialPllFreq;

    if (usingNewPll == FALSE)
    {   // OLD PLL
        #define PLL_DIVID_THRESHOLD                 66000000
        #define PLL_VCO_GAIN_TH                     110000000
        #define PLL_MIN_DIV_M                       8

        MP_DEBUG("\r\ncalculatePllParam to %dKHz for old PLL", dwPllFreq / 1000);

        if (dwPllFreq < PLL_DIVID_THRESHOLD)
            tmpDivider = 2;
        else
            tmpDivider = 1;

        for (tmpDivN = 1; tmpDivN <= 256; tmpDivN++)
        {
            for (tmpDivM = PLL_MIN_DIV_M; tmpDivM <= 256; tmpDivM++)
            {
                if (oscFreq > 16000000)
                    tmpFreq = oscFreq / 10 * tmpDivN / tmpDivider / tmpDivM * 10;
                else
                    tmpFreq = oscFreq * tmpDivN / (tmpDivider * tmpDivM);   // oscFreq small than 16MHz, it should not overflow

                if (tmpFreq > dwPllFreq)
                    tolerance = tmpFreq - dwPllFreq;
                else
                    tolerance = dwPllFreq - tmpFreq;

                if (tolerance < finialTolerance)
                {
                    finalDivN = tmpDivN - 1;
                    finalDivM = tmpDivM - 1;
                    finalDivider = tmpDivider - 1;
                    finialTolerance = tolerance;
                    finialPllFreq = tmpFreq;

                    //MP_ALERT("%dKHz, N=%03d, M=%03d P=%02d - %d", finialPllFreq / 1000, finalDivN, finalDivM, finalDivider, finialTolerance);
                }

                if (finialTolerance == 0)
                {   // Got best setting
                    tmpDivN = 257;
                    tmpDivM = 257;

                    MP_DEBUG("%dKHz, N=%03d, M=%03d P=%02d - %d", finialPllFreq / 1000, finalDivN, finalDivM, finalDivider, finialTolerance);
                }
            }
        }

#if (CHIP_VER_MSB == CHIP_VER_615)
        if (finialPllFreq < PLL_VCO_GAIN_TH)
            finalVcoGain = 0;
        else
            finalVcoGain = 1;

        finalChargePump = 2;
#else   // MP650/660, using default value of this function
        //finalVcoGain = 7;
        //finalChargePump = 7;
#endif
    }
    else
    {   // NEW PLL
        #define MAX_FVCO_VID            100
        #define MAX_FIN_DIV             32
        #define MAX_OUTPUT_DIVISION     4
        #define MIN_PLL_OUTPUT_FREQ     225000000
        #define MAX_PLL_OUTPUT_FREQ     500000000
        #define MIN_PLL_FB_FREQ         5000000
        #define MAX_PLL_FB_FREQ         50000000

        DWORD maxDividM;

        MP_DEBUG("\r\ncalculatePllParam to %dKHz for new PLL", dwPllFreq / 1000);

        if (dwPllFreq < (MIN_PLL_OUTPUT_FREQ / MAX_OUTPUT_DIVISION))
        {
            MP_ALERT("-E- Out of range for new PLL - %dKHz", dwPllFreq / 1000);

            return 0;
        }
        else if (dwPllFreq > MAX_PLL_OUTPUT_FREQ)
        {
            MP_ALERT("-E- Exceed to the max freq of new PLL - %dKHz", dwPllFreq / 1000);

            return 0;
        }

        for (maxDividM = 1; maxDividM <= MAX_FIN_DIV; maxDividM++)
        {
            if (((oscFreq / maxDividM) < MIN_PLL_FB_FREQ) ||
                ((oscFreq / maxDividM) > MAX_PLL_FB_FREQ))
            {
                maxDividM--;
                break;
            }
        }

        MP_DEBUG("Max. Div-M is %d", maxDividM);

        for (tmpDivider = 1; tmpDivider <= MAX_OUTPUT_DIVISION; tmpDivider++)
        {
            if (tmpDivider == 3)
                tmpDivider++;

            for (tmpDivN = 2; tmpDivN <= MAX_FVCO_VID; tmpDivN++)
            {
                for (tmpDivM = 1; tmpDivM <= maxDividM; tmpDivM++)
                {
                    // oscFreq small than 42.9MHz, it should not overflow
                    tmpFreq = oscFreq * tmpDivN / tmpDivM;

                    if ((tmpFreq > MAX_PLL_OUTPUT_FREQ) || (tmpFreq < MIN_PLL_OUTPUT_FREQ))
                    {
                        //MP_DEBUG("N=%03d, M=%03d - Not match PLL limitation-2", tmpDivN, tmpDivM);

                        continue;
                    }

                    tmpFreq /= tmpDivider;

                    if (tmpFreq > dwPllFreq)
                        tolerance = tmpFreq - dwPllFreq;
                    else
                        tolerance = dwPllFreq - tmpFreq;

                    if (tolerance < finialTolerance)
                    {
                        finalDivN = tmpDivN - 1;
                        finalDivM = tmpDivM - 1;
                        finalDivider = tmpDivider - 1;
                        finialTolerance = tolerance;
                        finialPllFreq = tmpFreq;

                        //MP_ALERT("%dKHz, N=%03d, M=%03d P=%02d - %d", finialPllFreq / 1000, finalDivN, finalDivM, finalDivider, finialTolerance);
                    }

                    if (finialTolerance == 0)
                    {   // Got best setting, exit loop
                        tmpDivN = MAX_FVCO_VID + 1;
                        tmpDivM = MAX_FIN_DIV + 1;
                        tmpDivider = MAX_OUTPUT_DIVISION + 1;

                        MP_DEBUG("%dKHz, N=%03d, M=%03d P=%02d - %d", finialPllFreq / 1000, finalDivN, finalDivM, finalDivider, finialTolerance);
                    }
                }
            }
        }

        finalVcoGain = 0;
        finalChargePump = 2;
    }

    if (finialTolerance == (DWORD) -1)
    {
        MP_ALERT("-E- calculatePllParam error !!!\r\n-E- Could not find out the parameter for %dKHz", dwPllFreq);

        return 0;
    }

    MP_DEBUG("Wish PLL frequency is %dKHz", dwPllFreq / 1000);
    MP_DEBUG("Output PLL frequency is %dKHz", finialPllFreq / 1000);
    MP_DEBUG("tolerance is %d percent", finialTolerance * 100 / dwPllFreq);
    MP_DEBUG("PLL parameter is N = %d, M = %d, Divider = %d", finalDivN, finalDivM, finalDivider);

    if (usingNewPll == TRUE)
    {   // new PLL
        MP_DEBUG("PLL parameter is 0x%08X\r\n", (finalDivider << 24) | (finalVcoGain << 20) | (finalChargePump << 16) |
                                                (finalDivN << 8) | finalDivM);
    }
    else
    {   // old PLL
        #if (CHIP_VER_MSB == CHIP_VER_615)
        MP_DEBUG("PLL parameter is 0x%08X\r\n", (finalDivider << 24) | (finalVcoGain << 20) | (finalChargePump << 16) |
                                                (finalDivN << 8) | finalDivM);
        #else   // MP650/660
        MP_DEBUG("PLL parameter is 0x%08X\r\n", (finalDivider << 24) | (finalVcoGain << 20) | (finalChargePump << 16) |
                                                (finalDivM << 8) | finalDivN);
        #endif
    }

    MP_DEBUG("\r\n");

    if (pdwPllCfg)
    {
#if (CHIP_VER_MSB == CHIP_VER_615)
        *pdwPllCfg = (finalDivider << 24) | (finalVcoGain << 20) | (finalChargePump << 16) |
                     (finalDivN << 8) | finalDivM;
#else   // MP650/660
        if (usingNewPll)
        {   // new PLL
            *pdwPllCfg = (finalDivider << 24) | (finalVcoGain << 20) | (finalChargePump << 16) |
                         (finalDivN << 8) | finalDivM;
        }
        else
        {
            *pdwPllCfg = (finalDivider << 24) | (finalVcoGain << 20) | (finalChargePump << 16) |
                         (finalDivM << 8) | finalDivN;
        }
#endif
    }

    return finialPllFreq;
}



DWORD Clock_PllFreqGet(E_PLL_INDEX pllIndex)
{
    DWORD pllFreq;

#ifdef CKG_CHIP_IS_FPGA
    pllFreq = MAIN_CRYSTAL_CLOCK;
#else
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DWORD divM, divN, divide;
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    DWORD pllOeBit;
#endif

    //                                   Div-N + 1
    // PLL Clock (MHz) = Fin(MHz) * ----------------------
    //                               (Div-M + 1) * divide
    switch (pllIndex)
    {
    case CLOCK_PLL1_INDEX:
#if (CHIP_VER_MSB == CHIP_VER_615)
        divN = ((regClockPtr->PLL1Cfg >> 8) & 0x00FF) + 1;
        divM = (regClockPtr->PLL1Cfg & 0x00FF) + 1;
#else
        pllOeBit = PLL1_OE;
        divM = ((regClockPtr->PLL1Cfg >> 8) & 0x00FF) + 1;
        divN = (regClockPtr->PLL1Cfg & 0x00FF) + 1;
#endif
        divide = (regClockPtr->PLL1Cfg & (BIT25 | BIT24)) >> 24;
        break;

    case CLOCK_PLL2_INDEX:
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
        pllOeBit = PLL2_OE;
#endif
        divN = ((regClockPtr->PLL2Cfg >> 8) & 0x00FF) + 1;
        divM = (regClockPtr->PLL2Cfg & 0x00FF) + 1;
        divide = (regClockPtr->PLL2Cfg & (BIT25 | BIT24)) >> 24;
        break;

    case CLOCK_PLL3_INDEX:
#if (CHIP_VER_MSB == CHIP_VER_615)
        divN = ((regClockPtr->PLL3Cfg >> 8) & 0x00FF) + 1;
        divM = (regClockPtr->PLL3Cfg & 0x00FF) + 1;
#else
        pllOeBit = PLL3_OE;
        divM = ((regClockPtr->PLL3Cfg >> 8) & 0x00FF) + 1;
        divN = (regClockPtr->PLL3Cfg & 0x00FF) + 1;
#endif
        divide = (regClockPtr->PLL3Cfg & (BIT25 | BIT24)) >> 24;
        break;

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    case CLOCK_PLLIDU_INDEX:
        pllOeBit = PLLIDU_OE;
        divN = ((regClockPtr->PLLIduCfg >> 8) & 0x00FF) + 1;
        divM = (regClockPtr->PLLIduCfg & 0x00FF) + 1;
        divide = (regClockPtr->PLLIduCfg & (BIT25 | BIT24)) >> 24;
        break;
#endif

    default:
        MP_ALERT("-E- Clock_PllFreqGet : Wrong PLL index - %1d", (WORD) pllIndex);

        return 0;
        break;
    }

    pllFreq = MAIN_CRYSTAL_CLOCK / 10 * divN / divM * 10;
    pllFreq /= divide + 1;

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    if ((regClockPtr->ClkCtrl & pllOeBit) == 0)
    {
        MP_ALERT("--E-- PLL-%1d not enabled yet, return 0 !!!",(WORD) pllIndex);
        //__asm("break 100");
        return 0;
    }
#endif

#endif
    //MP_DEBUG("PLL%01d Freq is %dKHz", (WORD) index + 1, pllFreq / 1000);

    return pllFreq;
}



DWORD Clock_CpuFreqGet(void)
{
    DWORD pllFreq;
    DWORD cpuFreq;
    BYTE cpuSel = Clock_CpuClockSelGet();

#if (CHIP_VER_MSB == CHIP_VER_615)
    pllFreq = Clock_PllFreqGet(cpuSel >> 3);

    switch (cpuSel & 0x07)
    {
    case CPUCKS_PLL1:
        cpuFreq = pllFreq;
        break;
    case CPUCKS_PLL1_DIV_2:
        cpuFreq = (pllFreq >> 1);
        break;
    case CPUCKS_PLL1_DIV_3:
        cpuFreq = (pllFreq / 3);
        break;
    case CPUCKS_PLL1_DIV_4:
        cpuFreq = (pllFreq >> 2);
        break;
    case CPUCKS_PLL1_DIV_5:
        cpuFreq = (pllFreq / 5);
        break;
    case CPUCKS_PLL1_DIV_6:
        cpuFreq = (pllFreq / 6);
        break;
    case CPUCKS_PLL1_DIV_7:
        cpuFreq = (pllFreq / 7);
        break;
    case CPUCKS_PLL1_DIV_8:
        cpuFreq = (pllFreq >> 3);
        break;
    }

#else   // MP65x/MP66x

    #ifdef CKG_CHIP_IS_FPGA
    cpuFreq = MAIN_CRYSTAL_CLOCK;
    #else   // real chip
    pllFreq = Clock_PllFreqGet(cpuSel >> 3);

    switch (cpuSel & 0x07)
    {
    case CPUCKS_PLL1:
    case CPUCKS_PLL2:
        cpuFreq = pllFreq;
        break;

    case CPUCKS_PLL1_DIV_2:
    case CPUCKS_PLL2_DIV_2:
        cpuFreq = (pllFreq >> 1);
        break;

    case CPUCKS_PLL1_DIV_3:
    case CPUCKS_PLL2_DIV_3:
        cpuFreq = (pllFreq / 3);
        break;

#if ((CHIP_VER_LSB == CHIP_VER_A) || (CHIP_VER_LSB == CHIP_VER_FPGA))
  #if (CHIP_VER_MSB != CHIP_VER_650)
    case CPUCKS_PLL1_DIV_4:
    case CPUCKS_PLL2_DIV_4:
        cpuFreq = (pllFreq >> 2);
  #else
    //note: accroding to hal_ckg.h, MP650 REV_A has no CPUCKS_PLL1_DIV_4 & CPUCKS_PLL2_DIV_4 definitions.
  #endif

#else
    case CPUCKS_PLL1_DIV_1_5:
    case CPUCKS_PLL2_DIV_1_5:
        cpuFreq = (pllFreq * 3) / 2;
#endif
        break;

    case CPUCKS_PLL1_DIV_5:
    case CPUCKS_PLL2_DIV_5:
        cpuFreq = (pllFreq / 5);
        break;

    case CPUCKS_PLL1_DIV_128:
    case CPUCKS_PLL2_DIV_128:
        cpuFreq = (pllFreq >> 7);
        break;

    case CPUCKS_PLL1_DIV_256:
    case CPUCKS_PLL2_DIV_256:
        cpuFreq = (pllFreq >> 8);
        break;

    case CPUCKS_PLL1_DIV_512:
    case CPUCKS_PLL2_DIV_512:
        cpuFreq = (pllFreq >> 9);
        break;
    }
    #endif

#endif

    //MP_DEBUG("CPU frequence is %dKHz", cpuFreq / 1000);

    return cpuFreq;
}



void Clock_CpuClockSelSet(E_CPU_CLOCK_SELECT select)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    if (((regClockPtr->Clkss1 >> 16) & 0x0F) != select)
    {
        DWORD cpuSel;

        cpuSel = ((DWORD) select & 0x0F) << 16;
        IntDisable();
        regClockPtr->Clkss1 = (regClockPtr->Clkss1 & 0xFFF0FFFF) | cpuSel;
        IntEnable();
    }
}



E_CPU_CLOCK_SELECT Clock_CpuClockSelGet(void)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    return (E_CPU_CLOCK_SELECT) ((regClockPtr->Clkss1 >> 16) & 0x0F);
}



void Clock_Dma_tWTR_Set(DWORD tWTR)
{
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    DWORD regValue;

    tWTR &= 0x07;
    IntDisable();
    regValue = (regDmaPtr->FDMACTL_EXT0 & 0xFFF8FFFF) | (tWTR << 16);
    adjustMemCs((DWORD *) &(regDmaPtr->FDMACTL_EXT0), regValue);
    IntEnable();
}



BYTE Clock_Dma_tWTR_Get(void)
{
    DMA *regDmaPtr = (DMA *) DMA_BASE;

    return ((regDmaPtr->FDMACTL_EXT0 >> 16) & 0x07);
}



void Clock_Dma_tRP_Set(DWORD tRP)
{
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    DWORD regValue;

    tRP &= 0x03;
    IntDisable();
    regValue = (regDmaPtr->SdramCtl & 0xFF3FFFFF) | (tRP << 22);
    adjustMemCs((DWORD *) &(regDmaPtr->SdramCtl), regValue);
    IntEnable();
}



BYTE Clock_Dma_tRP_Get(void)
{
    DMA *regDmaPtr = (DMA *) DMA_BASE;

    return ((regDmaPtr->SdramCtl >> 22) & 0x03);
}



void Clock_Dma_tRCD_Set(DWORD tRCD)
{
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    DWORD regValue;

    tRCD &= 0x03;
    IntDisable();
    regValue = (regDmaPtr->SdramCtl & 0xFFCFFFFF) | (tRCD << 20);
    adjustMemCs((DWORD *) &(regDmaPtr->SdramCtl), regValue);
    IntEnable();
}



BYTE Clock_Dma_tRCD_Get(void)
{
    DMA *regDmaPtr = (DMA *) DMA_BASE;

    return ((regDmaPtr->SdramCtl >> 20) & 0x03);
}



void Clock_MemClockSelSet(E_MEM_CLOCK_SELECT select)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    if ( ((regClockPtr->Clkss1 >> 20) & 0x07) != select )
    {
        DWORD memSel;

        memSel = (regClockPtr->Clkss1 & 0xFF8FFFFF) | (((DWORD) select & 0x07) << 20);
        IntDisable();
        adjustMemCs((DWORD *) &(regClockPtr->Clkss1), memSel);
        IntEnable();
    }
}



void Clock_MemClockSelSet4Pwdc(E_MEM_CLOCK_SELECT select)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    if ( ((regClockPtr->Clkss1 >> 20) & 0x07) != select )
    {
        DWORD memSel;

        memSel = (regClockPtr->Clkss1 & 0xFF8FFFFF) | (((DWORD) select & 0x07) << 20);
        adjustMemCs((DWORD *) &(regClockPtr->Clkss1), memSel);
    }
}



E_MEM_CLOCK_SELECT Clock_MemClockSelGet(void)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    return ((regClockPtr->Clkss1 >> 20) & 0x07);
}



DWORD Clock_MemFreqGet(void)
{
    DWORD pllFreq;
    DWORD memFreq;
    BYTE memSel = Clock_MemClockSelGet();

#ifdef CKG_CHIP_IS_FPGA
    memFreq = MAIN_CRYSTAL_CLOCK;
#elif(CHIP_VER_MSB == CHIP_VER_615)
    pllFreq = Clock_PllFreqGet(memSel >> 2);

    switch (memSel & 0x03)
    {
    case MEMCKS_PLL1:
        memFreq = pllFreq;
        break;
    case MEMCKS_PLL1_DIV_2:
        memFreq = (pllFreq >> 1);
        break;
    case MEMCKS_PLL1_DIV_3:
        memFreq = (pllFreq / 3);
        break;
    case MEMCKS_PLL1_DIV_4:
        memFreq = (pllFreq >> 2);
        break;
    }
#else   //MP65X, MP66X
    pllFreq = Clock_PllFreqGet(((memSel >> 2) == 0 ? 1 : 0));       // 0 : PLL2, 1 : PLL1

    switch (memSel & 0x03)
    {
    case MEMCKS_PLL1:
    case MEMCKS_PLL2:
        memFreq = pllFreq;
        break;
    case MEMCKS_PLL1_DIV_2:
    case MEMCKS_PLL2_DIV_2:
        memFreq = (pllFreq >> 1);
        break;
    case MEMCKS_PLL1_DIV_3:
    case MEMCKS_PLL2_DIV_3:
        memFreq = (pllFreq / 3);
        break;
    case MEMCKS_PLL1_DIV_4:
    case MEMCKS_PLL2_DIV_4:
        memFreq = (pllFreq >> 2);
        break;
    }
#endif


    //MP_DEBUG("Memory frequence is %dKHz", memFreq / 1000);

    return memFreq;
}



SBYTE Clock_MemTypeGet(void)
{
    BIU *regBiuPtr = (BIU *) BIU_BASE;
    DMA *regDmaPtr = (DMA *) DMA_BASE;

    if ((regBiuPtr->BiuChid >> 16) == 0x0660)
    {
        return 0;           // SDRAM
    }
    else if ((regBiuPtr->BiuChid >> 16) == 0x0650)
    {
        if ((regDmaPtr->SdramCtl & BIT15) == 0)
            return 0;       // SDRAM
        else
            return 1;       // DDR
    }
    else if ((regBiuPtr->BiuChid >> 16) == 0x0615)
    {
        return 1;
    }

    return -1;
}



void Clock_IpuClockSelSet(E_IPU_CLOCK_SELECT select)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    if ( (regClockPtr->Clkss1 & 0x07) != select )
    {
        DWORD ipuSel;

        ipuSel = select & 0x07;
        IntDisable();
        regClockPtr->Clkss1 = (regClockPtr->Clkss1 & 0xFFFFFFF8) | ipuSel;
        IntEnable();
    }
}



E_IPU_CLOCK_SELECT Clock_IpuClockSelGet(void)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    return (E_SCALE_CLOCK_SELECT) (regClockPtr->Clkss1 & 0x07);
}



SDWORD Clock_PllCfgSet(E_PLL_INDEX pllIndex, DWORD pllCfg)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DWORD *pllCfgRegAddr;
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    //DWORD pllResetBit;
    DWORD pllOeBit;
#endif
    BYTE resetShiftBits;

    switch (pllIndex)
    {
    case CLOCK_PLL1_INDEX:
        regClockPtr->ClkCtrl |= PLL1_EN;
        pllCfgRegAddr = (DWORD *) &(regClockPtr->PLL1Cfg);
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
        //pllResetBit = PLL1_RESET;
        pllOeBit = PLL1_OE;
#endif
        break;

    case CLOCK_PLL2_INDEX:
        regClockPtr->ClkCtrl |= PLL2_EN;
        pllCfgRegAddr = (DWORD *) &(regClockPtr->PLL2Cfg);
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
        //pllResetBit = PLL2_RESET;
        pllOeBit = PLL2_OE;
#endif
        break;

    case CLOCK_PLL3_INDEX:
        regClockPtr->ClkCtrl |= PLL3_EN;
        pllCfgRegAddr = (DWORD *) &(regClockPtr->PLL3Cfg);
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
        //pllResetBit = PLL3_RESET;
        pllOeBit = PLL3_OE;
#endif
        break;

#if (CHIP_VER_MSB != CHIP_VER_615)
    case CLOCK_PLLIDU_INDEX:
        regClockPtr->ClkCtrl |= PLLIDU_EN;
        pllCfgRegAddr = (DWORD *) &(regClockPtr->PLLIduCfg);
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
        //pllResetBit = PLLIDU_RESET;
        pllOeBit = PLLIDU_OE;
#endif
        break;
#endif

    default:
        MP_ALERT("--E-- Clock_PllCfgSet wrong PLL index %01d", pllIndex);

        return FAIL;
        break;
    }

    MP_DEBUG("Set PLL%01d 0x%X to 0x%08X", pllIndex + 1, (DWORD) pllCfgRegAddr, pllCfg);

    if (*pllCfgRegAddr != pllCfg)
    {
        DWORD i;

        IntDisable();
        adjustPllCfg(pllCfgRegAddr, pllCfg);
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
        regClockPtr->ClkCtrl |= pllOeBit;
#endif
        IntEnable();

        MP_DEBUG("Real PLL%01d is 0x%08X", pllIndex + 1, *pllCfgRegAddr);
    }

    return PASS;
}



DWORD Clock_PllCfgGet(E_PLL_INDEX pllIndex)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    switch (pllIndex)
    {
    case CLOCK_PLL1_INDEX:
        return regClockPtr->PLL1Cfg;
        break;

    case CLOCK_PLL2_INDEX:
        return regClockPtr->PLL2Cfg;
        break;

    case CLOCK_PLL3_INDEX:
        return regClockPtr->PLL3Cfg;
        break;

#if (CHIP_VER_MSB != CHIP_VER_615)
    case CLOCK_PLLIDU_INDEX:
        return regClockPtr->PLLIduCfg;
        break;
#endif

    default:
        MP_ALERT("--E-- Clock_PllCfgGet wrong PLL index %01d", pllIndex);

        return 0;
        break;
    }
}



SDWORD Clock_PllFreqSet(E_PLL_INDEX pllIndex, DWORD pllFreq)
{
    DWORD pllCfg;
    BYTE bDivN, bDivM, bDivider;
    BOOL usingNewPll = FALSE;

#if (CHIP_VER_MSB != CHIP_VER_615)
    if ((pllIndex == CLOCK_PLL2_INDEX) || (pllIndex == CLOCK_PLLIDU_INDEX))
        usingNewPll = TRUE;
#endif

#ifdef __PLL_CFG_ALWAYS_CALCULATE

    if (calculatePllParam(usingNewPll, MAIN_CRYSTAL_CLOCK, pllFreq, &pllCfg) == 0)
        return FAIL;

#else

    if (usingNewPll == FALSE)
    {   // OLD PLL
        switch (pllFreq)
        {
        case 6000000:
            pllCfg = PLL_CONFIG_6M;
            break;

        case 12000000:
            pllCfg = PLL_CONFIG_12M;
            break;

        case 15000000:
            pllCfg = PLL_CONFIG_15M;
            break;

        case 20000000:
            pllCfg = PLL_CONFIG_20M;
            break;

        case 30000000:
            pllCfg = PLL_CONFIG_30M;
            break;

        case 48000000:
            pllCfg = PLL_CONFIG_48M;
            break;

        case 54300000:
            pllCfg = PLL_CONFIG_54300K;
            break;

        case 65250000:
            pllCfg = PLL_CONFIG_65250K;
            break;

        case 72000000:
            pllCfg = PLL_CONFIG_72M;
            break;

        case 74250000:
            pllCfg = PLL_CONFIG_74P25M;
            break;

        case 96000000:
            pllCfg = PLL_CONFIG_96M;
            break;

        case 108000000:
            pllCfg = PLL_CONFIG_108M;
            break;

        case 120000000:
            pllCfg = PLL_CONFIG_120M;
            break;

        case 150000000:
            pllCfg = PLL_CONFIG_150M;
            break;

        case 160000000:
            pllCfg = PLL_CONFIG_160M;
            break;

        case 166000000:
            pllCfg = PLL_CONFIG_166M;
            break;

        default:
            if (calculatePllParam(FALSE, MAIN_CRYSTAL_CLOCK, pllFreq, &pllCfg) == 0)
                return FAIL;
            break;
        }
    }
    #if (CHIP_VER_MSB != CHIP_VER_615)
    else
    {   // New PLL
        switch (pllFreq)
        {
#if 1 // Alert! System not support this setting, it makes system hang, it is for electric measurement only       
        case 48000000:
            pllCfg = PLL_NEW_CONFIG_48M;
            break;
#endif            
        case 66000000:
        case 65250000:
            pllCfg = PLL_NEW_CONFIG_66M;
            break;

        case 72000000:
            pllCfg = PLL_NEW_CONFIG_72M;
            break;

        case 74250000:
            pllCfg = PLL_NEW_CONFIG_74P25M;
            break;

        case 96000000:
            pllCfg = PLL_NEW_CONFIG_96M;
            break;

        case 108000000:
            pllCfg = PLL_NEW_CONFIG_108M;
            break;

        case 120000000:
            pllCfg = PLL_NEW_CONFIG_120M;
            break;

        case 150000000:
            pllCfg = PLL_NEW_CONFIG_150M;
            break;

        case 160000000:
            pllCfg = PLL_NEW_CONFIG_160M;
            break;

        case 165000000:
        case 166000000:
            pllCfg = PLL_NEW_CONFIG_165M;
            break;

        default:
            if (calculatePllParam(TRUE, MAIN_CRYSTAL_CLOCK, pllFreq, &pllCfg) == 0)
                return FAIL;
            break;
        }
    }
    #endif

#endif      // #ifdef __PLL_CFG_ALWAYS_CALCULATE

    return Clock_PllCfgSet(pllIndex, pllCfg);
}



#if (CHIP_VER_MSB != CHIP_VER_615)
// Spread Spectrum
void Clock_PllSprdSet(E_PLL_INDEX pllIndex, BOOL enable, DWORD w, DWORD a, DWORD b)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DWORD newFreq, orgFreq, pllCfg;
    DWORD div2, divN, divM, periodA, periodB;

    MP_ALERT("%s -", __FUNCTION__);

    switch (pllIndex)
    {
    case CLOCK_PLL1_INDEX:
    case CLOCK_PLL3_INDEX:
        break;
    default:
        MP_ALERT("-E- Clock_PllSprd : Wrong PLL index - %2d", (WORD) pllIndex);

        return;
        break;
    }

    enable &= BIT0;

    if (enable == FALSE)
    {   // Disable spread spectrum
        regClockPtr->PLL3SsCfg &= BIT24;
        return;
    }

    w &= 0x0F;
    a &= 0x1F;
    b &= 0x1F;

    // Check the releationship between b and a
    if (b <= a)
    {
        if (a == 0x1F)
            a = 0x1E;

        b = a + 1;
    }

    if (pllIndex == CLOCK_PLL1_INDEX)
        regClockPtr->PLL1SsCfg = (enable << 24) | (w << 16) | (a << 8) | b;
    else
        regClockPtr->PLL3SsCfg = (enable << 24) | (w << 16) | (a << 8) | b;

    pllCfg = Clock_PllCfgGet(pllIndex);
    div2 = (pllCfg >> 24) & BIT0;
    divM = (pllCfg >> 8) & 0xFF;
    divN = pllCfg & 0xFF;
    newFreq = (MAIN_CRYSTAL_CLOCK * (divN + w + 1)) / (div2 + 1) / (divM + 1);
    periodA = (divN + 1) * a * 1000 / (orgFreq / 1000);
    periodB = (divN + 1) * (b - a) * 1000 / (newFreq / 1000);

    MP_ALERT("New PLL%1d frequency output between %dKHz to %dKHz", pllIndex + 1, orgFreq / 1000, newFreq / 1000);
    MP_ALERT("Period A is %dus, Period B is %dus", periodA, periodB);
}



void Clock_PllSprdSetByFreq(E_PLL_INDEX pllIndex, BOOL enable, DWORD upBoundFreq, DWORD lowBoundPeriod, DWORD fullBoundPeriod)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DWORD newFreq, orgFreq, pllCfg, divW;
    DWORD div2, divN, divM, periodA, periodB, paramA, paramB;

    MP_ALERT("%s -", __FUNCTION__);

    switch (pllIndex)
    {
    case CLOCK_PLL1_INDEX:
    case CLOCK_PLL3_INDEX:
        break;

    default:
        MP_ALERT("-E- Clock_PllSprd : Wrong PLL index - %2d", (WORD) pllIndex);

        return;
        break;
    }

    enable &= BIT0;

    if (enable == FALSE)
    {   // Disable spread spectrum
        regClockPtr->PLL3SsCfg &= BIT24;
        MP_ALERT("Spread specturm disabled !!!");

        return;
    }

    orgFreq = Clock_PllFreqGet(pllIndex);

    if (upBoundFreq <= orgFreq)
    {
        MP_ALERT("--E-- Up bound frequency small or same as orginal PLL frequency !!!");
        return;
    }

    if (lowBoundPeriod >= fullBoundPeriod)
    {
        MP_ALERT("--E-- Wrong period parameter !!!");
        return;
    }

    pllCfg = Clock_PllCfgGet(pllIndex);
    div2 = (pllCfg >> 24) & BIT0;
    divM = (pllCfg >> 8) & 0xFF;
    divN = pllCfg & 0xFF;

    newFreq = orgFreq;

    for (divW = 1; divW < 0x10; divW++)
    {
        newFreq = (MAIN_CRYSTAL_CLOCK * (divN + divW + 1)) / (div2 + 1) / (divM + 1);

        if (newFreq >= upBoundFreq)
            break;
    }

    if (newFreq > upBoundFreq)
    {
        if (divW > 1)
            divW--;
        else
        {
            MP_ALERT("--E-- Up bound frequency too small !!!");
            return;
        }
    }

    newFreq = (MAIN_CRYSTAL_CLOCK * (divN + divW + 1)) / (div2 + 1) / (divM + 1);
    MP_ALERT("PLL-W = %d", divW);
    MP_ALERT("Wish Up bound frequency is %dKHz", upBoundFreq / 1000);
    MP_ALERT("New Up bound frequency is %dKHz", newFreq / 1000);

    paramA = lowBoundPeriod * (orgFreq / 1000) / (divN + 1) / 1000;

    if (paramA > 0x1E)
    {
        MP_ALERT("--E-- Low bound period to long, change it to max value!!!");
        paramA = 0x1E;
        paramB = 0x1F;
    }
    else
    {
        paramB = (fullBoundPeriod - lowBoundPeriod) * (newFreq / 1000000) / (divN + 1);

        if (paramB > 0x1F)
        {
            MP_ALERT("--E-- Full bound period to long, change it to max value!!!");
            paramB = 0x1F;
        }
    }

    if (pllIndex == CLOCK_PLL1_INDEX)
        regClockPtr->PLL1SsCfg = (enable << 24) | (divW << 16) | (paramA << 8) | paramB;
    else
        regClockPtr->PLL3SsCfg = (enable << 24) | (divW << 16) | (paramA << 8) | paramB;

    periodA = (divN + 1) * paramA * 1000 / (orgFreq / 1000);
    periodB = (divN + 1) * (paramB - paramA) * 1000 / (newFreq / 1000);

    MP_ALERT("New PLL%1d frequency output between %dKHz to %dKHz", pllIndex + 1, orgFreq / 1000, newFreq / 1000);
    MP_ALERT("Low bound period is %dus, Full period is %dus", periodA, periodB);
}
#endif




void Dma_FdmaFifoTimingAdjust(BYTE maxContReq)
{
#if (CHIP_VER_MSB != CHIP_VER_615)
    DMA *regDmaPtr = (DMA *) DMA_BASE;

    IntDisable();

    regDmaPtr->FDMACTL_EXT0 = (regDmaPtr->FDMACTL_EXT0 & (~0x0000003F)) | maxContReq;

    regDmaPtr->FDMACTL_EXT1 |= BIT0;

    IntEnable();
#endif
}


#if (CHIP_VER_MSB != CHIP_VER_615)
static DWORD regDmaEXT0 = 0x00000003;
static DWORD regDmaEXT1 = 0x01003000;
static DWORD regDmaEXT2 = 0x00100602;
static DWORD regDmaEXT3 = 0xFFEFF9FC;
static DWORD regDmaEXT4 = 0x00000000;
#endif

void Dma_PriorityDefault(void)
{
#if (CHIP_VER_MSB != CHIP_VER_615)
    DMA *regDmaPtr = (DMA *) DMA_BASE;

    IntDisable();

    regDmaPtr->FDMACTL_EXT1 = regDmaEXT1;
    IODelay(100);
    regDmaPtr->FDMACTL_EXT0 &= ~0x0000003F;
    regDmaPtr->FDMACTL_EXT0 |= regDmaEXT0;
    regDmaPtr->FDMACTL_EXT0 &= ~0x00000100;

    regDmaPtr->FDMACTL_EXT2 = regDmaEXT2;
    regDmaPtr->FDMACTL_EXT3 = regDmaEXT3;
    regDmaPtr->FDMACTL_EXT4 = regDmaEXT4;

    regDmaPtr->FDMACTL_EXT1 |= BIT0;

    IntEnable();
#endif
}



void Dma_PriorityDefaultSave(void)
{
#if (CHIP_VER_MSB != CHIP_VER_615)
    DMA *regDmaPtr = (DMA *) DMA_BASE;

    IntDisable();

    regDmaEXT0 = regDmaPtr->FDMACTL_EXT0 & 0x0000003F;
    regDmaEXT1 = regDmaPtr->FDMACTL_EXT1;
    regDmaEXT2 = regDmaPtr->FDMACTL_EXT2;
    regDmaEXT3 = regDmaPtr->FDMACTL_EXT3;
    regDmaEXT4 = regDmaPtr->FDMACTL_EXT4;

    IntEnable();
#endif
}



void Dma_FirstPrioritySet(DWORD bitMapping)
{
#if (CHIP_VER_MSB != CHIP_VER_615)
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    DWORD orginalFirstPriority, newFirstPriority;

    if (bitMapping == 0)
        return;

    IntDisable();
    newFirstPriority = 0;

    while ((bitMapping & BIT0) == 0)
    {
        newFirstPriority++;
        bitMapping >>= 1;
    }

    MP_ALERT("newFirstPriority = %d", newFirstPriority);

    orginalFirstPriority = 1 << ((regDmaPtr->FDMACTL_EXT1 & 0xFFE0FFFF) >> 16);
    regDmaPtr->FDMACTL_EXT2 &= ~bitMapping;
    regDmaPtr->FDMACTL_EXT3 &= ~bitMapping;
    regDmaPtr->FDMACTL_EXT4 &= ~bitMapping;
    regDmaPtr->FDMACTL_EXT1 = (regDmaPtr->FDMACTL_EXT1 & 0xFFE0FFFF) | (newFirstPriority << 16);
    regDmaPtr->FDMACTL_EXT2 |= orginalFirstPriority;
    regDmaPtr->FDMACTL_EXT1 |= BIT0;
    IntEnable();
#endif
}



void Dma_SecondaryPrioritySet(DWORD bitMapping)
{
#if (CHIP_VER_MSB != CHIP_VER_615)
    DMA *regDmaPtr = (DMA *) DMA_BASE;

    IntDisable();
    regDmaPtr->FDMACTL_EXT2 |= bitMapping;
    regDmaPtr->FDMACTL_EXT3 &= ~bitMapping;
    regDmaPtr->FDMACTL_EXT4 &= ~bitMapping;
    regDmaPtr->FDMACTL_EXT1 |= BIT0;
    IntEnable();
#endif
}



void Dma_ThirdPrioritySet(DWORD bitMapping)
{
#if (CHIP_VER_MSB != CHIP_VER_615)
    DMA *regDmaPtr = (DMA *) DMA_BASE;

    IntDisable();
    regDmaPtr->FDMACTL_EXT2 &= ~bitMapping;
    regDmaPtr->FDMACTL_EXT3 |= bitMapping;
    regDmaPtr->FDMACTL_EXT4 &= ~bitMapping;
    regDmaPtr->FDMACTL_EXT1 |= BIT0;
    IntEnable();
#endif
}




void Dma_FourthPrioritySet(DWORD bitMapping)
{
#if (CHIP_VER_MSB != CHIP_VER_615)
    DMA *regDmaPtr = (DMA *) DMA_BASE;

    IntDisable();
    regDmaPtr->FDMACTL_EXT2 &= ~bitMapping;
    regDmaPtr->FDMACTL_EXT3 &= ~bitMapping;
    regDmaPtr->FDMACTL_EXT4 |= bitMapping;
    regDmaPtr->FDMACTL_EXT1 |= BIT0;
    IntEnable();
#endif
}



void Dma_BiuRequestSuspendSetting(BOOL enable, BOOL noLessThan4Word)
{
#if ((CHIP_VER_MSB == CHIP_VER_650) && (CHIP_VER_LSB != CHIP_VER_A) && (CHIP_VER_LSB != CHIP_VER_FPGA))
    BIU *regBiuPtr = (BIU *) BIU_BASE;

    IntDisable();

    if (enable)
    {
        // suspend single non-burst DRAM write request
        regBiuPtr->BIU_STRAP_CFG &= ~BIT31;

        if (noLessThan4Word)
            // if BIU write FIFO depth is no less than 4-word
            regBiuPtr->BIU_STRAP_CFG &= ~BIT29;
        else
            // if BIU write FIFO depth is no less than 2-word
            regBiuPtr->BIU_STRAP_CFG |= BIT29;
    }
    else
        regBiuPtr->BIU_STRAP_CFG |= BIT31;

    IntEnable();
#endif
}



void Dma_BiuPghitCheckEnable(BOOL enable)
{
#if ((CHIP_VER_MSB == CHIP_VER_660) && (CHIP_VER_LSB != CHIP_VER_A) && (CHIP_VER_LSB != CHIP_VER_FPGA))
    BIU *regBiuPtr = (BIU *) BIU_BASE;

    IntDisable();

    if (enable)
        regBiuPtr->BIU_STRAP_CFG |= BIT31;
    else
        regBiuPtr->BIU_STRAP_CFG &= ~BIT31;

    IntEnable();
#endif
}



#define SCRATCH_PAD_DATA_RAM_ADDR           0x98000000

#define REPEAT_COUNTER                      SCRATCH_PAD_DATA_RAM_ADDR
#define DELAY_LOOP_COUNTER                  (SCRATCH_PAD_DATA_RAM_ADDR + 4)
#define DUMMY_READ                          (SCRATCH_PAD_DATA_RAM_ADDR + 8)
#define DATA_TMP_ADDR                       (SCRATCH_PAD_DATA_RAM_ADDR + 12)

#define REG_PLL_CFG_ADDR                    (SCRATCH_PAD_DATA_RAM_ADDR + 16)
#define REG_PLL_CFG_VALUE                   (SCRATCH_PAD_DATA_RAM_ADDR + 20)
#define NEW_REG_PLL_CFG_ADDR                (SCRATCH_PAD_DATA_RAM_ADDR + 24)
#define REG_SDRAMCTRL_ADDR                  (SCRATCH_PAD_DATA_RAM_ADDR + 28)
#define REG_SDRAMCTL_VALUE                  (SCRATCH_PAD_DATA_RAM_ADDR + 32)
#define NEW_REG_SDRAMCTRL_ADDR              (SCRATCH_PAD_DATA_RAM_ADDR + 36)
#define REG_CLKCTRL_ADDR                    (SCRATCH_PAD_DATA_RAM_ADDR + 40)
#define REG_CLKCTRL_VALUE                   (SCRATCH_PAD_DATA_RAM_ADDR + 44)
#define NEW_REG_CLKCTRL_ADDR                (SCRATCH_PAD_DATA_RAM_ADDR + 48)
#define REG_FDMACTL_EXT0_ADDR               (SCRATCH_PAD_DATA_RAM_ADDR + 52)
#define REG_FDMACTL_EXT0_VALUE              (SCRATCH_PAD_DATA_RAM_ADDR + 56)
#define NEW_REG_FDMACTL_EXT0_ADDR           (SCRATCH_PAD_DATA_RAM_ADDR + 60)
#define REG_DLLCTRL_ADDR                    (SCRATCH_PAD_DATA_RAM_ADDR + 64)
#define REG_DLLCTRL_VALUE                   (SCRATCH_PAD_DATA_RAM_ADDR + 68)
#define NEW_REG_DLLCTRL_ADDR                (SCRATCH_PAD_DATA_RAM_ADDR + 72)
#define REG_CLKSS1_ADDR                     (SCRATCH_PAD_DATA_RAM_ADDR + 76)
#define NEW_REG_CLKSS1_ADDR                 (SCRATCH_PAD_DATA_RAM_ADDR + 80)
#define CPU_CLK_SEL                         (SCRATCH_PAD_DATA_RAM_ADDR + 84)
#define ORG_CPU_CLK_SEL                     (SCRATCH_PAD_DATA_RAM_ADDR + 88)
#define REG_SDREFCTL_ADDR                   (SCRATCH_PAD_DATA_RAM_ADDR + 92)
#define NEW_REG_SDREFCTL_ADDR               (SCRATCH_PAD_DATA_RAM_ADDR + 96)
#define REG_UART_B_TX_FIFO                  (SCRATCH_PAD_DATA_RAM_ADDR + 100)
#define REG_SDRAMCTRL_SDPWRDN               (SCRATCH_PAD_DATA_RAM_ADDR + 104)
#define REG_REAL_DLLCTRL_ADDR               (SCRATCH_PAD_DATA_RAM_ADDR + 108)
#define NEW_REG_REAL_DLLCTRL_ADDR           (SCRATCH_PAD_DATA_RAM_ADDR + 112)

#define DELAY_COUNT_FOR_CPU_SEL             (SCRATCH_PAD_DATA_RAM_ADDR + 120)
#define DELAY_COUNT_FOR_SELFREF             (SCRATCH_PAD_DATA_RAM_ADDR + 124)
#define DELAY_COUNT_FOR_SHORT               (SCRATCH_PAD_DATA_RAM_ADDR + 128)
#define DELAY_COUNT_FOR_PLL_STABLE          (SCRATCH_PAD_DATA_RAM_ADDR + 132)
#define DELAY_COUNT_FOR_VERY_SHORT          (SCRATCH_PAD_DATA_RAM_ADDR + 136)
#define DELAY_COUNT_FOR_DLL_STABLE          (SCRATCH_PAD_DATA_RAM_ADDR + 140)


#define __ENTER_SELF_REFRESH__

void Clock_SystemFrequencyChange(DWORD targetPllFreq, E_PLL_INDEX targetPllIndex,
                                 SDWORD tRCD, SDWORD tRP, SDWORD tWTR,
                                 SDWORD tXSRD, SDWORD tRWD, SDWORD tWRD,
                                 SDWORD sdckDls,
                                 SDWORD ckDlyManu, SDWORD ckDlyVal,
                                 SDWORD udqsDlyManu, SDWORD udqsDlyVal,
                                 SDWORD ldqsDlyManu, SDWORD ldqsDlyVal); __attribute__((section(".memfreqchg")))

void Clock_SystemFrequencyChange(DWORD targetPllFreq, E_PLL_INDEX targetPllIndex,
                                 SDWORD tRCD, SDWORD tRP, SDWORD tWTR,
                                 SDWORD tXSRD, SDWORD tRWD, SDWORD tWRD,
                                 SDWORD sdckDls,
                                 SDWORD ckDlyManu, SDWORD ckDlyVal,
                                 SDWORD udqsDlyManu, SDWORD udqsDlyVal,
                                 SDWORD ldqsDlyManu, SDWORD ldqsDlyVal)
{
    MP_ALERT("%s: targetPllFreq = %d, ", __func__, targetPllFreq);
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    BOOL usingNewPll = FALSE;
    DWORD pllCfg;
    E_CPU_CLOCK_SELECT tempCpuClkIndex;
    BOOL isDDR;

    IntDisable();

    /////////////////////////////////////////////////////////////////////////
    //
    //
    //
    /////////////////////////////////////////////////////////////////////////
    switch (targetPllIndex)
    {
    case CLOCK_PLL1_INDEX:
        *(volatile DWORD *) NEW_REG_PLL_CFG_ADDR = (DWORD) &(regClockPtr->PLL1Cfg);
        *(volatile DWORD *) CPU_CLK_SEL = CPUCKS_PLL2;
        break;

    case CLOCK_PLL2_INDEX:
        *(volatile DWORD *) NEW_REG_PLL_CFG_ADDR = (DWORD) &(regClockPtr->PLL2Cfg);
        *(volatile DWORD *) CPU_CLK_SEL = CPUCKS_PLL1;
        break;

    default:
        MP_ALERT("--E-- Clock_PllCfgSet wrong PLL index %01d", targetPllIndex);

        return;
        break;
    }

    if (targetPllIndex == CLOCK_PLL2_INDEX)
        usingNewPll = TRUE;

    if (calculatePllParam(usingNewPll, MAIN_CRYSTAL_CLOCK, targetPllFreq, &pllCfg) == 0)
    {
        MP_ALERT("--E-- calculatePllParam fail !!!");
        IntEnable();

        return;
    }

    *(volatile DWORD *) ORG_CPU_CLK_SEL = regClockPtr->Clkss1 & 0x000F0000;
    *(volatile DWORD *) REG_PLL_CFG_VALUE = pllCfg;
    *(volatile DWORD *) REG_PLL_CFG_ADDR = DATA_TMP_ADDR;
    *(volatile DWORD *) REG_CLKSS1_ADDR = DATA_TMP_ADDR;

    if (pllCfg == Clock_PllCfgGet(targetPllIndex))
        *(volatile DWORD *) NEW_REG_CLKSS1_ADDR = DATA_TMP_ADDR;
    else
        *(volatile DWORD *) NEW_REG_CLKSS1_ADDR = (DWORD) &(regClockPtr->Clkss1);

    isDDR = (regDmaPtr->SdramCtl & BIT15) >> 15;

    if (isDDR == 0)
    {   // single data rate SDRAM
        MP_ALERT("--W-- %s: Not DDR memory, DLL timing will not change !!!", __FUNCTION__);

        tRCD = -1;
        tRP = -1;
        tWTR = -1;
        tXSRD = -1;
        tRWD = -1;
        tWRD = -1;
        sdckDls = -1;
        ckDlyManu = -1;
        ckDlyVal = -1;
        udqsDlyManu = -1;
        udqsDlyVal = -1;
        ldqsDlyManu = -1;
        ldqsDlyVal = -1;

        *(volatile DWORD *) NEW_REG_REAL_DLLCTRL_ADDR = DATA_TMP_ADDR;
    }
    else
        *(volatile DWORD *) NEW_REG_REAL_DLLCTRL_ADDR = (DWORD) &(regDmaPtr->DllCtl);

    /////////////////////////////////////////////////////////////////////////
    //
    // Prepare setting for first run
    //
    /////////////////////////////////////////////////////////////////////////
    MP_ALERT("    Old setting - FDMACTL_EXT0 = 0x%08X, DLLCTRL = 0x%08X", regDmaPtr->FDMACTL_EXT0, regClockPtr->ClkCtrl);
    MP_DEBUG("\r\n\r\nOrginal setting -");
    MP_DEBUG("SdramCtl = 0x%08X", regDmaPtr->SdramCtl);
    MP_DEBUG("FDMACTL_EXT0 = 0x%08X", regDmaPtr->FDMACTL_EXT0);
    MP_DEBUG("ClkCtrl = 0x%08X", regClockPtr->ClkCtrl);
    MP_DEBUG("DllCtl = 0x%08X", regDmaPtr->DllCtl);

    /////////////////////////////////////////////////////////////////////////
    if (tRCD < 0)
        tRCD = regDmaPtr->SdramCtl & 0x00300000;
    else
        tRCD = (tRCD & 0x03) << 20;

    if (tRP < 0)
        tRP = regDmaPtr->SdramCtl & 0x00C00000;
    else
        tRP = (tRP & 0x03) << 22;

    if (tWTR < 0)
        tWTR = regDmaPtr->SdramCtl & 0x00070000;
    else
        tWTR = (tWTR & 0x07) << 16;

    *(volatile DWORD *) REG_SDRAMCTL_VALUE = (regDmaPtr->SdramCtl & 0xFF08FFFF) | tRCD | tRP | tWTR;

    if (*(volatile DWORD *) REG_SDRAMCTL_VALUE == regDmaPtr->SdramCtl)
        *(volatile DWORD *) NEW_REG_SDRAMCTRL_ADDR = DATA_TMP_ADDR;
    else
        *(volatile DWORD *) NEW_REG_SDRAMCTRL_ADDR = (DWORD) &(regDmaPtr->SdramCtl);

    /////////////////////////////////////////////////////////////////////////
    if (sdckDls < 0)
        sdckDls = regClockPtr->ClkCtrl & 0x00007000;
    else
        sdckDls = (sdckDls & 0x07) << 12;

    *(volatile DWORD *) REG_CLKCTRL_VALUE = (regClockPtr->ClkCtrl & 0xFFFF8FFF) | sdckDls;
    *(volatile DWORD *) REG_CLKCTRL_ADDR = DATA_TMP_ADDR;

    if (*(volatile DWORD *) REG_CLKCTRL_VALUE == regClockPtr->ClkCtrl)
        *(volatile DWORD *) NEW_REG_CLKCTRL_ADDR = DATA_TMP_ADDR;
    else
        *(volatile DWORD *) NEW_REG_CLKCTRL_ADDR = (DWORD) &(regClockPtr->ClkCtrl);

    /////////////////////////////////////////////////////////////////////////
    if (tXSRD < 0)
        tXSRD = regDmaPtr->FDMACTL_EXT0 & 0xFF000000;
    else
        tXSRD = (tXSRD & 0xFF) << 24;

    if (tRWD < 0)
        tRWD = regDmaPtr->FDMACTL_EXT0 & 0x00700000;
    else
        tRWD = (tRWD & 0x07) << 20;

    if (tWRD < 0)
        tWRD = regDmaPtr->FDMACTL_EXT0 & 0x00070000;
    else
        tWRD = (tWRD & 0x07) << 16;

    *(volatile DWORD *) REG_FDMACTL_EXT0_VALUE = (regDmaPtr->FDMACTL_EXT0 & 0x0088FFFF) | tXSRD | tRWD | tWRD;
    *(volatile DWORD *) REG_FDMACTL_EXT0_ADDR = DATA_TMP_ADDR;

    if (*(volatile DWORD *) REG_FDMACTL_EXT0_VALUE == regDmaPtr->FDMACTL_EXT0)
        *(volatile DWORD *) NEW_REG_FDMACTL_EXT0_ADDR = DATA_TMP_ADDR;
    else
        *(volatile DWORD *) NEW_REG_FDMACTL_EXT0_ADDR = (DWORD) &(regDmaPtr->FDMACTL_EXT0);

    /////////////////////////////////////////////////////////////////////////
    if (ckDlyManu < 0)
        ckDlyManu = regDmaPtr->DllCtl & BIT21;
    else
        ckDlyManu = (ckDlyManu & 0x01) << 21;

    if (ckDlyVal < 0)
        ckDlyVal = regDmaPtr->DllCtl & 0x001F0000;
    else
        ckDlyVal = (ckDlyVal & 0x1F) << 16;

    if (udqsDlyManu < 0)
        udqsDlyManu = regDmaPtr->DllCtl & BIT13;
    else
        udqsDlyManu = (udqsDlyManu & 0x01) << 13;

    if (udqsDlyVal < 0)
        udqsDlyVal = regDmaPtr->DllCtl & 0x00001F00;
    else
        udqsDlyVal = (udqsDlyVal & 0x1F) << 8;

    if (ldqsDlyManu < 0)
        ldqsDlyManu = regDmaPtr->DllCtl & BIT5;
    else
        ldqsDlyManu = (ldqsDlyManu & 0x01) << 5;

    if (ldqsDlyVal < 0)
        ldqsDlyVal = regDmaPtr->DllCtl & 0x0000001F;
    else
        ldqsDlyVal = ldqsDlyVal & 0x1F;

    *(volatile DWORD *) REG_DLLCTRL_VALUE = (regDmaPtr->DllCtl & 0xFFC0C0C0) | \
                                            ckDlyManu | ckDlyVal | \
                                            udqsDlyManu | udqsDlyVal | \
                                            ldqsDlyManu | ldqsDlyVal;

    *(volatile DWORD *) REG_DLLCTRL_ADDR = DATA_TMP_ADDR;

    if (*(volatile DWORD *) REG_DLLCTRL_VALUE == regDmaPtr->DllCtl)
        *(volatile DWORD *) NEW_REG_DLLCTRL_ADDR = DATA_TMP_ADDR;
    else
        *(volatile DWORD *) NEW_REG_DLLCTRL_ADDR = (DWORD) &(regDmaPtr->DllCtl);

    MP_ALERT("    New setting - FDMACTL_EXT0 = 0x%08X, DLLCTRL = 0x%08X", *(volatile DWORD *) REG_FDMACTL_EXT0_VALUE, *(volatile DWORD *) REG_DLLCTRL_VALUE);
    mpDebugPrint("");
    MP_DEBUG("\r\n\r\nNew setting -");
    MP_DEBUG("REG_SDRAMCTL_VALUE = 0x%08X", *(volatile DWORD *) REG_SDRAMCTL_VALUE);
    MP_DEBUG("REG_FDMACTL_EXT0_VALUE = 0x%08X", *(volatile DWORD *) REG_FDMACTL_EXT0_VALUE);
    MP_DEBUG("REG_CLKCTRL_VALUE = 0x%08X", *(volatile DWORD *) REG_CLKCTRL_VALUE);
    MP_DEBUG("REG_DLLCTRL_VALUE = 0x%08X", *(volatile DWORD *) REG_DLLCTRL_VALUE);
    MP_DEBUG("REF_IVL = %d", regDmaPtr->SdRefCtl & 0x00007FFF);

    /////////////////////////////////////////////////////////////////////////
    //
    // Prepare setting for first run
    //
    /////////////////////////////////////////////////////////////////////////
    *(volatile DWORD *) REPEAT_COUNTER = 2;
    *(volatile DWORD *) DELAY_COUNT_FOR_CPU_SEL = 1;
    *(volatile DWORD *) DELAY_COUNT_FOR_SHORT = 1;
    *(volatile DWORD *) DELAY_COUNT_FOR_PLL_STABLE = 1;
    *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT = 1;
    *(volatile DWORD *) DELAY_COUNT_FOR_DLL_STABLE = 1;

    *(volatile DWORD *) REG_SDRAMCTRL_ADDR = DATA_TMP_ADDR;
    *(volatile DWORD *) REG_REAL_DLLCTRL_ADDR = DATA_TMP_ADDR;
    *(volatile DWORD *) REG_SDREFCTL_ADDR = DATA_TMP_ADDR;
    *(volatile DWORD *) NEW_REG_SDREFCTL_ADDR = (DWORD) &(regDmaPtr->SdRefCtl);
    *(volatile DWORD *) REG_SDRAMCTRL_SDPWRDN = 0;

    //*(volatile DWORD *) 0xA8012100 |= HUART_TX_ENABLE;
    //*(volatile DWORD *) 0xA8012104 = 'S';

    while (*(volatile DWORD *) REPEAT_COUNTER)
    {
        // Change CPU clock select
        *(volatile DWORD *) (*(volatile DWORD *) REG_CLKSS1_ADDR) = ((*(volatile DWORD *) (*(volatile DWORD *) REG_CLKSS1_ADDR)) & 0xFFF0FFFF) | *(volatile DWORD *) CPU_CLK_SEL;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_CPU_SEL;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

#ifdef __ENTER_SELF_REFRESH__
        // Disable Auto-refresh, min is 72ns
        *(volatile DWORD *) (*(volatile DWORD *) REG_SDREFCTL_ADDR) &= ~BIT15;

        // SDRAM enter selfrefresh mode
        *(volatile DWORD *) (*(volatile DWORD *) REG_SDRAMCTRL_ADDR) |= *(volatile DWORD *) REG_SDRAMCTRL_SDPWRDN;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );
#endif

        // Change Target PLL cfg
        *(volatile DWORD *) (*(volatile DWORD *) REG_PLL_CFG_ADDR) = *(volatile DWORD *) REG_PLL_CFG_VALUE;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_PLL_STABLE;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // Change CPU clock select to target clk src
        *(volatile DWORD *) (*(volatile DWORD *) REG_CLKSS1_ADDR) = ((*(volatile DWORD *) (*(volatile DWORD *) REG_CLKSS1_ADDR)) & 0xFFF0FFFF) | *(volatile DWORD *) ORG_CPU_CLK_SEL;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_CPU_SEL;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // DLL lock and disable auto adjust, Keep at least 400 MCLK
        *(volatile DWORD *) (*(volatile DWORD *) REG_REAL_DLLCTRL_ADDR) &= ~(BIT30 | BIT25);
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_DLL_STABLE;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        *(volatile DWORD *) (*(volatile DWORD *) REG_REAL_DLLCTRL_ADDR) |= BIT30 | BIT25;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // New DDR timing
        // tRP, tCD, tWTR
        *(volatile DWORD *) (*(volatile DWORD *) REG_SDRAMCTRL_ADDR) = (*(volatile DWORD *) REG_SDRAMCTL_VALUE) | (*(volatile DWORD *) REG_SDRAMCTRL_SDPWRDN);
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // tXSRD, tRWD, tWRD
        *(volatile DWORD *) (*(volatile DWORD *) REG_FDMACTL_EXT0_ADDR) = *(volatile DWORD *) REG_FDMACTL_EXT0_VALUE;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // sdckDls
        *(volatile DWORD *) (*(volatile DWORD *) REG_CLKCTRL_ADDR) = *(volatile DWORD *) REG_CLKCTRL_VALUE;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // CkDlyManu, CkDlyVal, udqsDlyManu, udqsDlyVal, ldqsDlyManu, ldqsDlyVal
        *(volatile DWORD *) (*(volatile DWORD *) REG_DLLCTRL_ADDR) |= BIT28;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        *(volatile DWORD *) (*(volatile DWORD *) REG_DLLCTRL_ADDR) = BIT28 | *(volatile DWORD *) REG_DLLCTRL_VALUE;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        *(volatile DWORD *) (*(volatile DWORD *) REG_DLLCTRL_ADDR) &= ~BIT28;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // DLL lock
        *(volatile DWORD *) (*(volatile DWORD *) REG_REAL_DLLCTRL_ADDR) |= BIT28;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

#ifdef __ENTER_SELF_REFRESH__
        // Dummy read, need 200 MCLK
        *(volatile DWORD *) DUMMY_READ = *(volatile DWORD *) 0xA07FFFFC;        // 8M
        // Disbale Self-refresh
        *(volatile DWORD *) (*(volatile DWORD *) REG_SDRAMCTRL_ADDR) &= ~BIT0;
        // Enable Auto-refresh
        *(volatile DWORD *) (*(volatile DWORD *) REG_SDREFCTL_ADDR) |= BIT15;
#endif

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // Real setting for second run
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        (*(volatile DWORD *) REPEAT_COUNTER)--;
        *(volatile DWORD *) REG_CLKSS1_ADDR = *(volatile DWORD *) NEW_REG_CLKSS1_ADDR;
        *(volatile DWORD *) REG_PLL_CFG_ADDR = *(volatile DWORD *) NEW_REG_PLL_CFG_ADDR;

        *(volatile DWORD *) REG_SDREFCTL_ADDR = *(volatile DWORD *) NEW_REG_SDREFCTL_ADDR;

        *(volatile DWORD *) REG_SDRAMCTRL_ADDR = *(volatile DWORD *) NEW_REG_SDRAMCTRL_ADDR;
        *(volatile DWORD *) REG_DLLCTRL_ADDR = *(volatile DWORD *) NEW_REG_DLLCTRL_ADDR;
        *(volatile DWORD *) REG_REAL_DLLCTRL_ADDR = *(volatile DWORD *) NEW_REG_REAL_DLLCTRL_ADDR;
        *(volatile DWORD *) REG_FDMACTL_EXT0_ADDR = *(volatile DWORD *) NEW_REG_FDMACTL_EXT0_ADDR;
        *(volatile DWORD *) REG_CLKCTRL_ADDR = *(volatile DWORD *) NEW_REG_CLKCTRL_ADDR;

#ifdef __ENTER_SELF_REFRESH__
        *(volatile DWORD *) REG_SDRAMCTRL_SDPWRDN = BIT0;
#endif

        *(volatile DWORD *) DELAY_COUNT_FOR_CPU_SEL = 10;
        *(volatile DWORD *) DELAY_COUNT_FOR_SHORT = 100;
        *(volatile DWORD *) DELAY_COUNT_FOR_PLL_STABLE = 50000;
        *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT = 10;
        *(volatile DWORD *) DELAY_COUNT_FOR_DLL_STABLE = 40000;
    }

    IntEnable();
#endif
}



void Clock_DRAM_timing_Set(SDWORD tRCD, SDWORD tRP, SDWORD tWTR, SDWORD tXSRD, SDWORD tRWD, SDWORD tWRD, SDWORD sdckDls,
                           SDWORD ckDlyManu, SDWORD ckDlyVal,
                           SDWORD udqsDlyManu, SDWORD udqsDlyVal,
                           SDWORD ldqsDlyManu, SDWORD ldqsDlyVal); __attribute__((section(".setdramtiming")))

void Clock_DRAM_timing_Set(SDWORD tRCD, SDWORD tRP, SDWORD tWTR, SDWORD tXSRD, SDWORD tRWD, SDWORD tWRD, SDWORD sdckDls,
                           SDWORD ckDlyManu, SDWORD ckDlyVal,
                           SDWORD udqsDlyManu, SDWORD udqsDlyVal,
                           SDWORD ldqsDlyManu, SDWORD ldqsDlyVal)
{
#if (CHIP_VER_MSB == CHIP_VER_650)
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    IntDisable();

    if ((regDmaPtr->SdramCtl & BIT15) == 0)
    {
        MP_ALERT("--E-- %s: Not DDR memory !!!", __FUNCTION__);
        IntEnable();

        return;
    }

    /////////////////////////////////////////////////////////////////////////
    //
    // Prepare setting for first run
    //
    /////////////////////////////////////////////////////////////////////////
    MP_ALERT("\r\n\r\nOrginal setting -");
    MP_ALERT("SdramCtl = 0x%08X", regDmaPtr->SdramCtl);
    MP_ALERT("FDMACTL_EXT0 = 0x%08X", regDmaPtr->FDMACTL_EXT0);
    MP_ALERT("ClkCtrl = 0x%08X", regClockPtr->ClkCtrl);
    MP_ALERT("DllCtl = 0x%08X", regDmaPtr->DllCtl);

    /////////////////////////////////////////////////////////////////////////
    if (tRCD < 0)
        tRCD = regDmaPtr->SdramCtl & 0x00300000;
    else
        tRCD = (tRCD & 0x03) << 20;

    if (tRP < 0)
        tRP = regDmaPtr->SdramCtl & 0x00C00000;
    else
        tRP = (tRP & 0x03) << 22;

    if (tWTR < 0)
        tWTR = regDmaPtr->SdramCtl & 0x00070000;
    else
        tWTR = (tWTR & 0x07) << 16;

    *(volatile DWORD *) REG_SDRAMCTL_VALUE = (regDmaPtr->SdramCtl & 0xFF08FFFF) | tRCD | tRP | tWTR;

    if (*(volatile DWORD *) REG_SDRAMCTL_VALUE == regDmaPtr->SdramCtl)
        *(volatile DWORD *) NEW_REG_SDRAMCTRL_ADDR = DATA_TMP_ADDR;
    else
        *(volatile DWORD *) NEW_REG_SDRAMCTRL_ADDR = (DWORD) &(regDmaPtr->SdramCtl);

    /////////////////////////////////////////////////////////////////////////
    if (sdckDls < 0)
        sdckDls = regClockPtr->ClkCtrl & 0x00007000;
    else
        sdckDls = (sdckDls & 0x07) << 12;

    *(volatile DWORD *) REG_CLKCTRL_VALUE = (regClockPtr->ClkCtrl & 0xFFFF8FFF) | sdckDls;
    *(volatile DWORD *) REG_CLKCTRL_ADDR = DATA_TMP_ADDR;

    if (*(volatile DWORD *) REG_CLKCTRL_VALUE == regClockPtr->ClkCtrl)
        *(volatile DWORD *) NEW_REG_CLKCTRL_ADDR = DATA_TMP_ADDR;
    else
        *(volatile DWORD *) NEW_REG_CLKCTRL_ADDR = (DWORD) &(regClockPtr->ClkCtrl);

    /////////////////////////////////////////////////////////////////////////
    if (tXSRD < 0)
        tXSRD = regDmaPtr->FDMACTL_EXT0 & 0xFF000000;
    else
        tXSRD = (tXSRD & 0xFF) << 24;

    if (tRWD < 0)
        tRWD = regDmaPtr->FDMACTL_EXT0 & 0x00700000;
    else
        tRWD = (tRWD & 0x07) << 20;

    if (tWRD < 0)
        tWRD = regDmaPtr->FDMACTL_EXT0 & 0x00070000;
    else
        tWRD = (tWRD & 0x07) << 16;

    *(volatile DWORD *) REG_FDMACTL_EXT0_VALUE = (regDmaPtr->FDMACTL_EXT0 & 0x0088FFFF) | tXSRD | tRWD | tWRD;
    *(volatile DWORD *) REG_FDMACTL_EXT0_ADDR = DATA_TMP_ADDR;

    if (*(volatile DWORD *) REG_FDMACTL_EXT0_VALUE == regDmaPtr->FDMACTL_EXT0)
        *(volatile DWORD *) NEW_REG_FDMACTL_EXT0_ADDR = DATA_TMP_ADDR;
    else
        *(volatile DWORD *) NEW_REG_FDMACTL_EXT0_ADDR = (DWORD) &(regDmaPtr->FDMACTL_EXT0);

    /////////////////////////////////////////////////////////////////////////
    if (ckDlyManu < 0)
        ckDlyManu = regDmaPtr->DllCtl & BIT21;
    else
        ckDlyManu = (ckDlyManu & 0x01) << 21;

    if (ckDlyVal < 0)
        ckDlyVal = regDmaPtr->DllCtl & 0x001F0000;
    else
        ckDlyVal = (ckDlyVal & 0x1F) << 16;

    if (udqsDlyManu < 0)
        udqsDlyManu = regDmaPtr->DllCtl & BIT13;
    else
        udqsDlyManu = (udqsDlyManu & 0x01) << 13;

    if (udqsDlyVal < 0)
        udqsDlyVal = regDmaPtr->DllCtl & 0x00001F00;
    else
        udqsDlyVal = (udqsDlyVal & 0x1F) << 8;

    if (ldqsDlyManu < 0)
        ldqsDlyManu = regDmaPtr->DllCtl & BIT5;
    else
        ldqsDlyManu = (ldqsDlyManu & 0x01) << 5;

    if (ldqsDlyVal < 0)
        ldqsDlyVal = regDmaPtr->DllCtl & 0x0000001F;
    else
        ldqsDlyVal = ldqsDlyVal & 0x1F;

    *(volatile DWORD *) REG_DLLCTRL_VALUE = (regDmaPtr->DllCtl & 0xFFC0C0C0) | \
                                            ckDlyManu | ckDlyVal | \
                                            udqsDlyManu | udqsDlyVal | \
                                            ldqsDlyManu | ldqsDlyVal;

    *(volatile DWORD *) REG_DLLCTRL_ADDR = DATA_TMP_ADDR;

    if (*(volatile DWORD *) REG_DLLCTRL_VALUE == regDmaPtr->DllCtl)
        *(volatile DWORD *) NEW_REG_DLLCTRL_ADDR = DATA_TMP_ADDR;
    else
        *(volatile DWORD *) NEW_REG_DLLCTRL_ADDR = (DWORD) &(regDmaPtr->DllCtl);

    MP_ALERT("\r\n\r\nNew setting -");
    MP_ALERT("REG_SDRAMCTL_VALUE = 0x%08X", *(volatile DWORD *) REG_SDRAMCTL_VALUE);
    MP_ALERT("REG_FDMACTL_EXT0_VALUE = 0x%08X", *(volatile DWORD *) REG_FDMACTL_EXT0_VALUE);
    MP_ALERT("REG_CLKCTRL_VALUE = 0x%08X", *(volatile DWORD *) REG_CLKCTRL_VALUE);
    MP_ALERT("REG_DLLCTRL_VALUE = 0x%08X", *(volatile DWORD *) REG_DLLCTRL_VALUE);

    *(volatile DWORD *) DELAY_COUNT_FOR_SHORT = 1;
    *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT = 1;
    *(volatile DWORD *) DELAY_COUNT_FOR_DLL_STABLE = 1;
    *(volatile DWORD *) REPEAT_COUNTER = 2;

    *(volatile DWORD *) REG_REAL_DLLCTRL_ADDR = DATA_TMP_ADDR;
    *(volatile DWORD *) REG_SDRAMCTRL_ADDR = DATA_TMP_ADDR;

    /////////////////////////////////////////////////////////////////////////
    while (*(volatile DWORD *) REPEAT_COUNTER)
    {
        // DLL lock and disable auto adjust, Keep at least 400 MCLK
        *(volatile DWORD *) (*(volatile DWORD *) REG_REAL_DLLCTRL_ADDR) &= ~(BIT30 | BIT25);
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_DLL_STABLE;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        *(volatile DWORD *) (*(volatile DWORD *) REG_REAL_DLLCTRL_ADDR) |= BIT30 | BIT25;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // New DDR timing
        // tRP, tCD, tWTR
        *(volatile DWORD *) (*(volatile DWORD *) REG_SDRAMCTRL_ADDR) = (*(volatile DWORD *) REG_SDRAMCTL_VALUE) | (*(volatile DWORD *) REG_SDRAMCTRL_SDPWRDN);
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // tXSRD, tRWD, tWRD
        *(volatile DWORD *) (*(volatile DWORD *) REG_FDMACTL_EXT0_ADDR) = *(volatile DWORD *) REG_FDMACTL_EXT0_VALUE;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // sdckDls
        *(volatile DWORD *) (*(volatile DWORD *) REG_CLKCTRL_ADDR) = *(volatile DWORD *) REG_CLKCTRL_VALUE;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // CkDlyManu, CkDlyVal, udqsDlyManu, udqsDlyVal, ldqsDlyManu, ldqsDlyVal
        *(volatile DWORD *) (*(volatile DWORD *) REG_DLLCTRL_ADDR) |= BIT28;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        *(volatile DWORD *) (*(volatile DWORD *) REG_DLLCTRL_ADDR) = BIT28 | *(volatile DWORD *) REG_DLLCTRL_VALUE;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        *(volatile DWORD *) (*(volatile DWORD *) REG_DLLCTRL_ADDR) &= ~BIT28;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        // DLL lock
        *(volatile DWORD *) (*(volatile DWORD *) REG_REAL_DLLCTRL_ADDR) |= BIT28;
        *(volatile DWORD *) DELAY_LOOP_COUNTER = *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT;
        while ( (*(volatile DWORD *) DELAY_LOOP_COUNTER) -- );

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        (*(volatile DWORD *) REPEAT_COUNTER)--;
        *(volatile DWORD *) REG_SDRAMCTRL_ADDR = *(volatile DWORD *) NEW_REG_SDRAMCTRL_ADDR;
        *(volatile DWORD *) REG_DLLCTRL_ADDR = *(volatile DWORD *) NEW_REG_DLLCTRL_ADDR;
        *(volatile DWORD *) REG_REAL_DLLCTRL_ADDR = *(volatile DWORD *) NEW_REG_REAL_DLLCTRL_ADDR;
        *(volatile DWORD *) REG_FDMACTL_EXT0_ADDR = *(volatile DWORD *) NEW_REG_FDMACTL_EXT0_ADDR;
        *(volatile DWORD *) REG_CLKCTRL_ADDR = *(volatile DWORD *) NEW_REG_CLKCTRL_ADDR;

        *(volatile DWORD *) DELAY_COUNT_FOR_SHORT = 500;
        *(volatile DWORD *) DELAY_COUNT_FOR_VERY_SHORT = 100;
        *(volatile DWORD *) DELAY_COUNT_FOR_DLL_STABLE = 10000;
    }

    IntEnable();
#endif
}



static void adjustDllDelay(register DWORD regAddr, register DWORD); __attribute__((section(".dlldelay")))

static void adjustDllDelay(register DWORD regAddr, register DWORD regValue)
{
    register DWORD reg1, reg2, reg3;

    reg1 = *(volatile DWORD *) regAddr | BIT28;
    reg2 = regValue & ~BIT28;
    asm("nop");
    asm("nop");
    *(volatile DWORD *) regAddr = reg1;
    asm("nop");
    asm("nop");
    *(volatile DWORD *) regAddr = regValue;
    asm("nop");
    asm("nop");
    *(volatile DWORD *) regAddr = reg2;
}



/*
void Clock_DqsDelay_Set(SDWORD udqsDlyManu, SDWORD udqsDlyVal, SDWORD ldqsDlyManu, SDWORD ldqsDlyVal); __attribute__((section(".setdramtiming")))

void Clock_DqsDelay_Set(SDWORD udqsDlyManu, SDWORD udqsDlyVal, SDWORD ldqsDlyManu, SDWORD ldqsDlyVal)
{
#if (CHIP_VER_MSB == CHIP_VER_650)
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    DWORD newSetting;

    if ((regDmaPtr->SdramCtl & BIT15) == 0)
    {
        MP_ALERT("--E-- %s: Not DDR memory !!!", __FUNCTION__);

        return;
    }

    IntDisable();

    /////////////////////////////////////////////////////////////////////////
    //
    // Prepare setting for first run
    //
    /////////////////////////////////////////////////////////////////////////
    MP_DEBUG("\r\n\r\nOrginal DllCtl = 0x%08X", regDmaPtr->DllCtl);

    if (udqsDlyManu < 0)
        udqsDlyManu = regDmaPtr->DllCtl & BIT13;
    else
        udqsDlyManu = (udqsDlyManu & 0x01) << 13;

    if (udqsDlyVal < 0)
        udqsDlyVal = regDmaPtr->DllCtl & 0x00001F00;
    else
        udqsDlyVal = (udqsDlyVal & 0x1F) << 8;

    if (ldqsDlyManu < 0)
        ldqsDlyManu = regDmaPtr->DllCtl & BIT5;
    else
        ldqsDlyManu = (ldqsDlyManu & 0x01) << 5;

    if (ldqsDlyVal < 0)
        ldqsDlyVal = regDmaPtr->DllCtl & 0x0000001F;
    else
        ldqsDlyVal = ldqsDlyVal & 0x1F;

    newSetting = (regDmaPtr->DllCtl & 0xFFFFC0C0) | udqsDlyManu | udqsDlyVal | ldqsDlyManu | ldqsDlyVal;

    if (regDmaPtr->DllCtl != newSetting)
        adjustDllDelay((DWORD) &(regDmaPtr->DllCtl), BIT28 | newSetting);

    IntEnable();
#endif
}
*/



void Clock_DllDelay_Set(SDWORD ck90DlyManu, SDWORD ck90DlyVal, SDWORD udqsDlyManu, SDWORD udqsDlyVal, SDWORD ldqsDlyManu, SDWORD ldqsDlyVal); __attribute__((section(".setdramtiming")))

void Clock_DllDelay_Set(SDWORD ck90DlyManu, SDWORD ck90DlyVal, SDWORD udqsDlyManu, SDWORD udqsDlyVal, SDWORD ldqsDlyManu, SDWORD ldqsDlyVal)
{
#if (CHIP_VER_MSB == CHIP_VER_650)
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    DWORD newSetting;

    if ((regDmaPtr->SdramCtl & BIT15) == 0)
    {
        MP_ALERT("--E-- %s: Not DDR memory !!!", __FUNCTION__);

        return;
    }

    IntDisable();

    /////////////////////////////////////////////////////////////////////////
    //
    // Prepare setting for first run
    //
    /////////////////////////////////////////////////////////////////////////
    MP_DEBUG("\r\n\r\nOrginal DllCtl = 0x%08X", regDmaPtr->DllCtl);

    if (ck90DlyManu < 0)
        ck90DlyManu = regDmaPtr->DllCtl & BIT21;
    else
        ck90DlyManu = (ck90DlyManu & 0x01) << 21;

    if (ck90DlyVal < 0)
        ck90DlyVal = regDmaPtr->DllCtl & 0x001F0000;
    else
        ck90DlyVal = (ck90DlyVal & 0x1F) << 16;

    if (udqsDlyManu < 0)
        udqsDlyManu = regDmaPtr->DllCtl & BIT13;
    else
        udqsDlyManu = (udqsDlyManu & 0x01) << 13;

    if (udqsDlyVal < 0)
        udqsDlyVal = regDmaPtr->DllCtl & 0x00001F00;
    else
        udqsDlyVal = (udqsDlyVal & 0x1F) << 8;

    if (ldqsDlyManu < 0)
        ldqsDlyManu = regDmaPtr->DllCtl & BIT5;
    else
        ldqsDlyManu = (ldqsDlyManu & 0x01) << 5;

    if (ldqsDlyVal < 0)
        ldqsDlyVal = regDmaPtr->DllCtl & 0x0000001F;
    else
        ldqsDlyVal = ldqsDlyVal & 0x1F;

    newSetting = (regDmaPtr->DllCtl & 0xFFC0C0C0) | ck90DlyManu | ck90DlyVal  | udqsDlyManu | udqsDlyVal | ldqsDlyManu | ldqsDlyVal;

    if (regDmaPtr->DllCtl != newSetting)
        adjustDllDelay((DWORD) &(regDmaPtr->DllCtl), BIT28 | newSetting);

    IntEnable();
#endif
}


#define __MEM_FREQ_INDEX__                      0
#define __COLD_TMP_QDIV4_INDEX__                1
#define __WARM_TMP_DELAY__                      2
#define __COLD_TMP_DELAY__                      3
#define __DQS_LOW_BOUND_DELAY__                 4
#define __DQS_UP_BOUND_DELAY__                  5
#define __MEMBER_NUM_OF_DLL_DELAY_ARRAY__       6

const SWORD dllDelayArray[][__MEMBER_NUM_OF_DLL_DELAY_ARRAY__] = {
    {138,   9,  -1, 0, 0x05,   0x0B},           // 138MHz
    {153,   9,  -1, 0, 0x05,   0x0B,},          // 153Mhz
};

void Clock_DllDelayTuning(void);

void Clock_DllDelayTuning(void)
{
#if (CHIP_VER_MSB == CHIP_VER_650)
    static DWORD qdiv4Value[2] = {0};
    SDWORD newDqsDelayValue;
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    DWORD tmpData;
    SWORD qDiv4BitField = 0;
    WORD tabIndex, memFreq;

#if FUJI_DQS_AUTO_ADJUST_TEST
    return;
#endif

    if ((regDmaPtr->SdramCtl & BIT15) == 0)
    {
        MP_ALERT("--E-- %s: Not DDR memory !!!", __FUNCTION__);

        return;
    }

    IntDisable();
    qdiv4Value[1] = qdiv4Value[0];
    qdiv4Value[0] = regDmaPtr->Qdiv4 & 0x00FFFFFF;

    memFreq = Clock_MemFreqGet() / 1000000;

    MP_DEBUG("qdiv4Value[0] = 0x%X, qdiv4Value[1] = 0x%X", qdiv4Value[0], qdiv4Value[1]);

    for (tabIndex = 0; tabIndex < ((sizeof(dllDelayArray) / sizeof(SWORD)) / __MEMBER_NUM_OF_DLL_DELAY_ARRAY__); tabIndex++)
    {
        if (memFreq == dllDelayArray[tabIndex][__MEM_FREQ_INDEX__])
            break;
    }

    if (tabIndex >= ((sizeof(dllDelayArray) / sizeof(SWORD)) / __MEMBER_NUM_OF_DLL_DELAY_ARRAY__))
    {
        //MP_ALERT("--E-- T");
        IntEnable();
        return;
    }

    tmpData = qdiv4Value[0];

    if (qdiv4Value[0] != 0)
    {
        for (qDiv4BitField = 0; qDiv4BitField < 31; qDiv4BitField++)
        {
            if (tmpData & BIT0)
                break;

            tmpData >>= 1;
        }
    }

    (qDiv4BitField >= dllDelayArray[tabIndex][__COLD_TMP_QDIV4_INDEX__]) ? \
            (newDqsDelayValue = qDiv4BitField + dllDelayArray[tabIndex][__COLD_TMP_DELAY__]) : \
            (newDqsDelayValue = qDiv4BitField + dllDelayArray[tabIndex][__WARM_TMP_DELAY__]);

    if (newDqsDelayValue < dllDelayArray[tabIndex][__DQS_LOW_BOUND_DELAY__])
        newDqsDelayValue = dllDelayArray[tabIndex][__DQS_LOW_BOUND_DELAY__];
    else if (newDqsDelayValue > dllDelayArray[tabIndex][__DQS_UP_BOUND_DELAY__])
        newDqsDelayValue = dllDelayArray[tabIndex][__DQS_UP_BOUND_DELAY__];

    MP_DEBUG("Qdiv4 = 0x%08X, BIT%d", qdiv4Value[0], qDiv4BitField);

    if ((regDmaPtr->DllCtl & (BIT5 | BIT13)) != (BIT5 | BIT13))
    {   // First time, change to manual mode
        Clock_DllDelay_Set(1, qDiv4BitField, 1, newDqsDelayValue, 1, newDqsDelayValue);
    }
    else if (qdiv4Value[1] == qdiv4Value[0])
    {
#if (FUJI_DQS_MANUAL_ADJUST_TEST == 0)
        SDWORD currDqsDelayValue;
        SDWORD currCk90DelayValue;

        currDqsDelayValue = regDmaPtr->DllCtl & 0x0000001F;
        currCk90DelayValue = (regDmaPtr->DllCtl & 0x001F0000) >> 16;

        if ((currDqsDelayValue != newDqsDelayValue) || (currCk90DelayValue != qDiv4BitField))
        {
            // step by step apporch
            (currDqsDelayValue > newDqsDelayValue) ? \
                (newDqsDelayValue = currDqsDelayValue - 1) : (newDqsDelayValue = currDqsDelayValue + 1);

//            (currCk90DelayValue > qDiv4BitField) ? \
//                (qDiv4BitField = currCk90DelayValue - 1) : (qDiv4BitField = currCk90DelayValue + 1);

#if SYSTEM_MEMORY_RW_TEST
            Clock_DllDelay_Set(-1, -1, 1, newDqsDelayValue, 1, newDqsDelayValue);
#else
            Clock_DllDelay_Set(1, qDiv4BitField, 1, newDqsDelayValue, 1, newDqsDelayValue);
#endif
        }
#endif
    }

    IntEnable();
#endif
}

