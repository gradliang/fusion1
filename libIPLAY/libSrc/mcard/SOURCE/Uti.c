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
* Filename      : uti.c
* Programmer(s) :
* Created       :
* Descriptions  :
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
#include "uti.h"

/*
// Constant declarations
*/

/*
// Structure declarations
*/
/*
// Type declarations
*/
/*
// Variable declarations
*/

static DWORD PLLClock[16] = {0};
static BYTE  isWPenable = 1;

/*
// Static function prototype
*/

/*
// Definition of internal functions
*/

// for debug
void MemDump(BYTE *ptr, DWORD num)
{
	int e;
	//mpDebugPrint("Dumping address = 0x%x", ptr);
	for (e = 0 ; e < num ; e++)
	{
		if ((e&0xf) == 0)
		{
			if (e)
				UartOutText("\r\n");
			UartOutValue(e, 8);
			UartOutText(" : ");
		}
		UartOutValue(ptr[e], 2);
		PutUartChar(' ');
	}
	UartOutText("\r\n");
}

void Mcard_TaskYield(DWORD Tms)
{
	DWORD cur;
	DWORD start = GetSysTime();

	do
	{
		TaskYield();
		cur = GetSysTime();
	} while (TIME_PERIOD(cur, start) < Tms);
}

DWORD CalValue2Exp(DWORD value)
{
    DWORD exp;

    exp = 0;
    value = value >> 1;
    while(value)
    {
        value = value >> 1;
        exp++;
    }
    return exp;
}

void GeneratePLLClockTab(DWORD pll1, DWORD pll2)
{
	DWORD i;

	PLLClock[0] = pll1/1000;
	PLLClock[8] = pll2/1000;
	MP_DEBUG("Mcard clk source 1:%dKHz, 2:%dKHz", PLLClock[0], PLLClock[8]);

	for (i = 1 ; i < 6 ; i++)
		PLLClock[i] = PLLClock[0] / (i + 1);
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
	PLLClock[6] = PLLClock[0] >> 8;
	PLLClock[7] = PLLClock[0] >> 9;
#else
	PLLClock[6] = PLLClock[0] >> 7;
	PLLClock[7] = PLLClock[0] >> 8;
#endif
	for (i = 9 ; i < 14 ; i++)
		PLLClock[i] = PLLClock[8] / (i - 7);
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
	PLLClock[14] = PLLClock[8] >> 8;
	PLLClock[15] = PLLClock[8] >> 9;
#else
	PLLClock[14] = PLLClock[8] >> 7;
	PLLClock[15] = PLLClock[8] >> 8;
#endif

    //for (i=0 ; i<16 ; i++)
    //{
	//   mpDebugPrint("Clock PLL[%d]=%ld", i, PLLClock[i]);
    //}
}

/*

 // According to MaxSpeed to find an appropriate clock rate from PLL1 and PLL2.

*/
static BYTE Freq_To_McardCks(DWORD MaxSpeedKHz)
{
	BYTE i;
	BYTE retCode = 7;

	for (i = 0 ; i < 16 ; i++)
	{
		if ((PLLClock[i] <= MaxSpeedKHz) && (PLLClock[i] > PLLClock[retCode]))
		{
			retCode = i;
		}
	}
#if (STD_BOARD_VER == MP650_FPGA)
	if (retCode == 0)
		retCode = 1;
#endif

	MP_DEBUG("Desired clock: %dKHz--> Real clock: %d(%d)", MaxSpeedKHz, PLLClock[retCode], retCode);

	return retCode;
}

DWORD GetMcardClock()
{
	DWORD div, ret;

	#if (STD_BOARD_VER != MP650_FPGA)
	div = mGetMcardCks();
	#else
	div = (((CLOCK *)CLOCK_BASE)->Clkss_EXT1 & (0x0f << 16)) >> 16;
	#endif

	if (div < 8)
		ret = PLLClock[0] / (div + 1);
	else
		ret = PLLClock[8] / (div - 8 + 1);

	return ret;
}

void SetMcardClock(DWORD freq)
{
	CLOCK *sClock = (CLOCK *)CLOCK_BASE;
	BYTE bClock = Freq_To_McardCks(freq);

	MP_DEBUG("Clock %dKHz and div %d", freq, bClock);
#if (STD_BOARD_VER != MP650_FPGA)
	mClrMcardCks();
	mSetMcardCks(bClock);
	sClock->MdClken |= 0x00002000;
#else
	sClock->Clkss_EXT1 &= ~(0x0f << 16);
	if (freq <= 400)
		sClock->Clkss_EXT1 |= 6 << 16;
	else
		sClock->Clkss_EXT1 |= 1 << 16;
	sClock->Clkss_EXT1 |= 1 << 20;
#endif

    MP_DEBUG("sClock->Clkss_EXT1: %08lX, sClock->Clkss1: %08lX", sClock->Clkss_EXT1, sClock->Clkss1);
}

void SetSD2Clock(DWORD freq)
{
	CLOCK *sClock = (CLOCK *)CLOCK_BASE;
	BYTE bClock = Freq_To_McardCks(freq);

	MP_DEBUG("SD2 Clock %dKHz and div %d", freq, bClock);
	sClock->Clkss2 &= ~(0x0f << 28);
	sClock->Clkss2 |= (bClock << 28);
	sClock->MdClken |= (1<<17);
}

// Mcard PIN define
// Mcard PIN define, the number is order of FGPIO
// please modify by chip datasheet or circuit schematic
//
// please put WP at last for being compatible with disable write protection methodology!!!!
#if (CHIP_VER_MSB == CHIP_VER_615)
#define SD_WP		56
#define SD_DATA0	8
static BYTE SDPins[] = {8, 9, 10, 11, 12, 13, 14, 15, 18, 46, 56};
static BYTE MMCPins[] = {8, 9, 10, 11, 12, 13, 14, 15, 18, 46, 56};
static BYTE MSPins[] = {20, 47};
static BYTE NandPins[] = {4, 5, 6, 7, 22, 24, 33, 39, 40, 41, 42, 43, 44, 45, 54};
static BYTE XDPins[] = {0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 21, 31, 32, 51, 58};
static BYTE CFMPins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
							, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 31, 32, 35, 36, 37, 38};
static const BYTE SPIPins[] = {};

#elif (CHIP_VER_MSB == CHIP_VER_650)
#define SD_WP		30
#define SD_DATA0	2
#define SD2_WP		44
static BYTE SDPins[] = {0, 1, 2, 3, 4, 5, 30};
static BYTE SD2Pins[] = {36, 37, 38, 39, 40, 41, 44};
static BYTE MMCPins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 30};
static BYTE MSPins[] = {0, 6, 7, 8, 9, 12, 16, 17, 18, 19};
static BYTE NandPins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 21, 22,
#if MCARD_POWER_CTRL
	23,
#endif
	31};
static BYTE XDPins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 24, 31, 45};
static BYTE SPIPins[] = {0, 6, 7, 8, 9, 22, 30};
static BYTE CFMPins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 20, 25, 32, 33, 42};

#elif (CHIP_VER_MSB == CHIP_VER_660)
#define SD_WP		30
#define SD_DATA0	2
static BYTE SDPins[] = {0, 1, 2, 3, 4, 5, 30};
static BYTE MMCPins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 30};
static BYTE MSPins[] = {0, 6, 7, 8, 9, 12, 16, 17, 18, 19};
static BYTE NandPins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 21, 22,
#if MCARD_POWER_CTRL
	23,
#endif
	31};
static BYTE XDPins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 24, 31, 45};
static BYTE SPIPins[] = {0, 6, 7, 8, 9, 22, 30};
static BYTE CFMPins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 20, 25, 32, 33, 42};
#endif

BYTE SDPin_WP_Get()
{
	return SD_WP;
}

BYTE SdHost_GetCardWP(void)
{
	if(!isWPenable)
		return 0;
	else
		return !!(((GPIO *)GPIO_BASE)->Fgpdat[SDPin_WP_Get() >> 4] & (1 << (SDPin_WP_Get() & 0x0f)));
}

#if (CHIP_VER_MSB == CHIP_VER_650)
BYTE SD2Pin_WP_Get()
{
	return SD2_WP;
}

BYTE SdHost2_GetCardWP(void)
{
	if(!isWPenable)
		return 0;
	else
		return !!(((GPIO *)GPIO_BASE)->Fgpdat[SD2Pin_WP_Get() >> 4] & (1 << (SD2Pin_WP_Get() & 0x0f)));
}

#endif



BYTE SDPin_Data_Get()
{
	return SD_DATA0;
}

static BYTE *McardPinGet(DWORD type, BYTE *nr)
{
	BYTE *ret;

	switch (type)
	{
		case PIN_DEFINE_FOR_SD:
			ret = SDPins;
			*nr = sizeof(SDPins);
			break;
#if (CHIP_VER_MSB == CHIP_VER_650)
		case PIN_DEFINE_FOR_SD2:
			ret = SD2Pins;
			*nr = sizeof(SD2Pins);
			break;
#endif
		case PIN_DEFINE_FOR_MMC:
			ret = MMCPins;
			*nr = sizeof(MMCPins);
			break;
		case PIN_DEFINE_FOR_MS:
			ret = MSPins;
			*nr = sizeof(MSPins);
			break;
		case PIN_DEFINE_FOR_NAND:
			ret = NandPins;
			*nr = sizeof(NandPins);
			break;
		case PIN_DEFINE_FOR_XD:
			ret = XDPins;
			*nr = sizeof(XDPins);
			break;
		case PIN_DEFINE_FOR_CFM:
			ret = CFMPins;
			*nr = sizeof(CFMPins);
			break;
		case PIN_DEFINE_FOR_SPI:
			ret = SPIPins;
			*nr = sizeof(SPIPins);
			break;
		default:
			ret = NULL;
			*nr = 0;
			break;
	}

	return ret;
}

void McardSelect(DWORD type)
{
	GPIO *sGpio = (GPIO *)GPIO_BASE;
	DWORD i;
	BYTE nr;
	BYTE *McardPinDefine = McardPinGet(type, &nr);
	//mpDebugPrint("  card[%d]Active...", type);

	if(!isWPenable && (type == PIN_DEFINE_FOR_SD || type == PIN_DEFINE_FOR_SD2))    // skip wp pin
		nr--;

	if (McardPinDefine != NULL)
	{
		for (i = 0 ; i < nr ; i++)
		{	// set to Alt. Func. 1
			sGpio->Fgpcfg[McardPinDefine[i]>>4] &= ~(0x00010001 << (McardPinDefine[i] & 0x0f));
			sGpio->Fgpcfg[McardPinDefine[i]>>4] |= 0x00000001 << (McardPinDefine[i] & 0x0f);
		}
	}
}

void McardDeselect(DWORD type)
{
	GPIO *sGpio = (GPIO *)GPIO_BASE;
	DWORD i;
	BYTE nr;
	BYTE *McardPinDefine = McardPinGet(type, &nr);
	//mpDebugPrint("  card[%d]Inactive...", type);

	if(!isWPenable && (type == PIN_DEFINE_FOR_SD || type == PIN_DEFINE_FOR_SD2))    // skip wp pin
		nr--;

	if (McardPinDefine != NULL)
	{
		for (i = 0 ; i <= nr ; i++)
		{
			// set to Alt. Func. 1
			sGpio->Fgpcfg[McardPinDefine[i]>>4] &= ~(0x00010001 << (McardPinDefine[i] & 0x0f));	// gpio
			sGpio->Fgpdat[McardPinDefine[i]>>4] &= ~(0x00010000 << (McardPinDefine[i] & 0x0f));	// input
		}
	}
}

static DWORD FgpioPowerPin[MAX_DEVICE_DRV];
static BOOL PowerCtrlEnable[MAX_DEVICE_DRV] = {0};
static BOOL PwPinActiveHi[MAX_DEVICE_DRV];

void McardPowerPinConfig(DWORD DevId, BOOL PowerCtrl, DWORD FgpioNo, BOOL ActiveHigh)
{
	if (PowerCtrl == TRUE)
	{
		FgpioPowerPin[DevId] = FgpioNo;
		PwPinActiveHi[DevId] = ActiveHigh;
	}
	PowerCtrlEnable[DevId] = PowerCtrl;
}

void McardPowerOn(DWORD DevId)
{
	if (PowerCtrlEnable[DevId] == TRUE)
	{
		mpDebugPrint("Mcard Power On");
		GPIO *gpio = (GPIO *)GPIO_BASE;
		DWORD nr = FgpioPowerPin[DevId] >> 4;
		DWORD bitshift = FgpioPowerPin[DevId] & 0xf;
		gpio->Fgpcfg[nr] &= ~(0x00010001 << bitshift);
		gpio->Fgpdat[nr] |= (0x00010000 << bitshift);	// output
		if (PwPinActiveHi[DevId] == TRUE)
			gpio->Fgpdat[nr] |= (0x00000001 << bitshift);	// high
		else
			gpio->Fgpdat[nr] &= ~(0x00000001 << bitshift);	// low

		McardIODelay(0x500);      // for power rasing time
	}
}

void McardPowerOff(DWORD DevId)
{
	if (PowerCtrlEnable[DevId] == TRUE)
	{
		if( !Mcard_GetAllFlagPresent())
		{
			mpDebugPrint("Mcard Power Off");
			GPIO *gpio = (GPIO *)GPIO_BASE;
			DWORD nr = FgpioPowerPin[DevId] >> 4;
			DWORD bitshift = FgpioPowerPin[DevId] & 0xf;
			gpio->Fgpcfg[nr] &= ~(0x00010001 << bitshift);
			gpio->Fgpdat[nr] |= (0x00010000 << bitshift);	// output
			if (PwPinActiveHi[DevId] == TRUE)
				gpio->Fgpdat[nr] &= ~(0x00000001 << bitshift);	// low
			else
				gpio->Fgpdat[nr] |= (0x00000001 << bitshift);	// high

			McardIODelay(0x500);     // for power falling time
		}
		else
		{
			mpDebugPrint("some card still in the socket  and not power off");
		}
	}
}

void McardPduEnable()
{
#if (CHIP_VER_MSB == CHIP_VER_650 || CHIP_VER_MSB == CHIP_VER_660)
	CLOCK *clk = (CLOCK *)CLOCK_BASE;

	#if SD_MMC_ENABLE
	clk->PduEn0 |= (BIT0|BIT1|BIT28|BIT30);
	#else
	clk->PduEn0 &= ~(BIT0|BIT1|BIT28|BIT30);
	#endif
	#if SD2_ENABLE
	clk->PduEn1 |= (BIT4|BIT5|BIT6|BIT7|BIT8|BIT9|BIT11|BIT12);
	#else
	clk->PduEn1 &= ~(BIT4|BIT5|BIT6|BIT7|BIT8|BIT9|BIT11|BIT12);
	#endif
	#if NAND_ENABLE
	clk->PduEn0 |= (BIT21|BIT22|BIT23|BIT31);
	#else
	clk->PduEn0 &= ~(BIT21|BIT22|BIT23|BIT31);
	#endif
	#if MS_ENABLE
	clk->PduEn0 |= (BIT16|BIT17|BIT18|BIT19|BIT29);
	#else
	clk->PduEn0 &= ~(BIT16|BIT17|BIT18|BIT19|BIT29);
	#endif
	#if XD_ENABLE
	clk->PduEn0 |= (BIT24|BIT27);
	clk->PduEn1 |= BIT13;
	#else
	clk->PduEn0 &= ~(BIT24|BIT27);
	clk->PduEn1 &= ~BIT13;
	#endif

	/*disable BIT6|BIT7|BIT8|BIT9*/
	clk->PduEn0 |= (BIT2|BIT3|BIT4|BIT5|BIT10|BIT11|BIT12|BIT13|BIT14|BIT15
					|BIT20|BIT25|BIT26);
	clk->PduEn1 |= (BIT0|BIT1|BIT2|BIT3|BIT10);
#endif
	//mpDebugPrint("2. clk->PduEn0: %x, clk->PduEn1: %x", clk->PduEn0, clk->PduEn1);
}

void McardPduDisable()
{
#if (CHIP_VER_MSB == CHIP_VER_650)
	CLOCK *clk = (CLOCK *)CLOCK_BASE;

	clk->PduEn0 = 0;
	clk->PduEn1 &= ~0x3fff;
#endif
}

void McardHwConfig(BYTE cardCtl)
{
	McardPduEnable();
	if(cardCtl & DIS_WRITE_PROTECTION){
		MP_DEBUG("[W] Disable write protect...");
		isWPenable = 0;
	}

	// change DMA priority within chip
	#if (0)
	DMA *dma = (DMA *)DMA_BASE;
	dma->FDMACTL_EXT2 = BIT18;
	dma->FDMACTL_EXT3 = ~BIT18;
	dma->FDMACTL_EXT1 |= BIT0;
	#endif
}

