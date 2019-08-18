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
* Filename      : NandPhy_Mp650.c
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
#if (((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660)) && (NAND_DRV == NAND_SIMPLE_DRV))
#if (NAND_ENABLE || ISP_FUNC_ENABLE)

#include "mpTrace.h"
#include "Mcard.h"
#include "nand.h"
#include "Uti.h"
#include "taskid.h"


#if LOADER_CODE
#define AP_TAG					0x4d504150		// MPAP
#define mpDebugPrint(...)
#define MP_DEBUG(...)
#define MP_DEBUG2(...)
#define MemDump(...)
#endif
#undef TRIGGER_BY_INT
#define TRIGGER_BY_INT			0


#define SW_MANUAL				0
#define HW_AUTO					1
// nand flash command is send by HW or SW
#if LOADER_CODE
#define NAND_CMD_MODE			SW_MANUAL
#else
//#define NAND_CMD_MODE			HW_AUTO
#define NAND_CMD_MODE			SW_MANUAL // 512 bytes per page SLC
#endif

#define ECC_FLOW_EN				0x00000001
#define ECC_FLOW_RST			0x00000002

#define FIFO_FIX				0x00000002
#define ECC_ENABLE				0x00000004
#define TYPE_256_BYTE			0x00000000
#define TYPE_512_BYTE			0x00000008
#define CS_NAND					0x00000000
#define IC_SRB					0x00000001
#define IC_ECB  				0x00000002
#define IC_SFTCE				0x00000004
#define IC_SMTO 				0x00000008
#define IC_CDX  				0x00000010
#define IM_SRB					0x00000100
#define IM_ECB					0x00000200
#define IM_SFTCE				0x00000400
#define IM_SMTO					0x00000800
#define IM_CDX					0x00001000
#if TRIGGER_BY_INT
#define IM_ALL					(0x01010000)	// edge & high trigger
#else
#define IM_ALL					0
#endif

#if 1
#define MCR_TRS					0
#define MCR_HLD					2
#define MCR_PLW					4
#define MCR_ST					0
#define MCW_TRS					0
#define MCW_HLD					2
#define MCW_PLW					4
#define MCW_ST					0
#else    // More stable setting for testing...
#define MCR_TRS					3
#define MCR_HLD					3
#define MCR_PLW					7
#define MCR_ST					3
#define MCW_TRS					3
#define MCW_HLD					3
#define MCW_PLW					7
#define MCW_ST					3
#endif

#define CE_BIT_OFFSET			8

//MCARD ECC4S Error Define
#define NO_ERROR				0
#define CORRECT_COMPLETE		1
#define ENCODE_COMPLETION		2
#define ENCODE_ERROR			-1
#define DECODE_ERROR			-2
#define DECODE_CORRECT_IMPOSSIBLE	-3
#define DECODE_CHECK_TIMEOUT	-4

#define READ_CMD_SET			((READ_PAGE1_CMD<<24) | (READ_PAGE1_CMD_2CYC<<16) | (READ_IN_PAGE_S_CMD<<8) | READ_IN_PAGE_E_CMD)
#define WRITE_CMD_SET			((SEQ_DATA_IN_CMD<<24) | (PAGE_PROG_CMD<<16))

// constant define
#define SECTOR_PER_PAGE			(1 << (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP))
#define ECC_PER_PAGE			(1 << (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP - NandParameter.ECC_Size))
#define ECC_START_ADDR			(FTL_LENGTH + 3)

#define ECC_FAIL_RETRY			16

// Macro
#define START_MCARD_FUNC(type, dir, nr, hwauto) \
{ \
	if (dir) \
		pstMcard->McardC |= dir << 4; \
	else \
		pstMcard->McardC &= ~MCARD_DIR_MASK; \
	pstMcEcc->MC_ECC_OPCT[0] = 0; \
	pstMcard->McSmEccIc = 0x0f000000 | (TRIGGER_BY_INT << 8); \
	pstMcEcc->MC_ECC_OPCT[0] = ECC_FLOW_EN | ECC_FLOW_RST \
							| ((type) << 8) \
							| (((nr)-1) << 12) \
							| (hwauto << 16) \
							| ((hwauto&dir) << 21); \
}

#define START_ECC() \
{ \
	if (NandParameter.ECC_Type == ECC_1BIT) \
	{ \
		pstMcard->McSmC &= ~ECC_ENABLE; \
		pstMcard->McSmC |= ECC_ENABLE; \
	} \
	else \
	{ \
		pstMcEcc->MC_ECCBCH_C = 0; \
		pstMcEcc->MC_ECCBCH_C = 0x3; \
	} \
}

enum SPARE_AREA_IDX
{
	BLOCK_STATUS_N		= 0,	// bad block mark in Nand
	LOG_BLK_ADDR_HI		= 1,
	LOG_BLK_ADDR_LO		= 2,
	DATA_STATUS			= 3,
	BLOCK_STATUS_X		= 4,	// bad block mark in xD
	FTL_LENGTH			= 5
};

typedef enum ECC_FLOW_TYPE
{
	ALL_IN_ONE	= 0,
	FTL_ONLY	= 1,
	ECC_ONLY	= 2,
	FTL_N_ECC	= 3,
	DATA_ONLY	= 4
} EccFlowType;

typedef struct MAKER_MAP
{
	BYTE MakerID;
	BYTE MakerName[16];
} MakerMap;

static MCARD *pstMcard = (MCARD *)MCARD_BASE;
static McardEcc *pstMcEcc = (McardEcc *)MC_ECC_BASE;
static CHANNEL *pstChannel = (CHANNEL *)DMA_MC_BASE;
static NandInfo NandParameter = {0};
static SWORD (*ECC_Correct)(BYTE *, DWORD) = NULL;

#if LOADER_CODE
DWORD ReservedBuffer[(16*512+512*2)>>2];	// 16x512(the max size of a page) + 512x2(two spare area size)

static void memcpy(BYTE *dst, BYTE *src, DWORD len)
{
	while(len--)
		*dst++ = *src++;
}
#endif
static SWORD WaitEccAction()
{
	SWORD ret = PASS;
#if TRIGGER_BY_INT
	DWORD event;
	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &event, 5000) != PASS)
		mpDebugPrint("%Ecc timeout x", pstMcard->McSmEccIc);
	if (!(event & 0x00000001))
	{
        mpDebugPrint("\t\t%s: timeout McSmEccIc = %x", __FUNCTION__, pstMcard->McSmEccIc);
		mpDebugPrint("\t\t\t , MCARD=%x, OPCT0=%x", pstMcard->McardC, pstMcEcc->MC_ECC_OPCT[0]);
		mpDebugPrint("\t\t\t , channels=%x, channele=%x", pstChannel->StartA, pstChannel->EndA);
        ret = FAIL;
	}
#else
	DWORD tmr = GetSysTime();

	ret = FAIL;
	do
	{
		if ((pstMcard->McSmEccIc & 0x00000001) == 0x00000001)
		{
			ret = PASS;
			break;
		}
	}
	while (SystemGetElapsedTime(tmr) < 1000);	// 1sec

	if(ret == FAIL)
	{
		mpDebugPrint("\t\t%s: timeout McSmEccIc = %x", __FUNCTION__, pstMcard->McSmEccIc);
		mpDebugPrint("\t\t\t , MCARD=%x, OPCT0=%x", pstMcard->McardC, pstMcEcc->MC_ECC_OPCT[0]);
		mpDebugPrint("\t\t\t , channels=%x, channele=%x", pstChannel->StartA, pstChannel->EndA);
	}
	pstMcard->McSmEccIc = 0x0f000000;	// clean interrupt cause
#endif

	if (ret == PASS)
	{
		tmr = GetSysTime();
		do
		{
			if (!(pstChannel->Control & MCARD_DMA_ENABLE) && !(pstMcard->McardC & MCARD_FIFO_ENABLE))
			{
				ret = PASS;
				break;
			}
		}
		while (SystemGetElapsedTime(tmr) < 1000);	// 1sec
		if (ret == FAIL)
		{
			mpDebugPrint("Nand: DMA/FIFO timeout, DMA_C=%x,MCARD_C=%x", pstChannel->Control, pstMcard->McardC);
			while(1);
		}
	}

	return ret;
}

static SWORD ECC_1Bit_Correct(BYTE *pbBuffer, DWORD dwNandEcc, DWORD dw600Ecc, DWORD datalen)
{
	DWORD dwEccDiff;
	SWORD ret = PASS;

	//mpDebugPrint("ECC 1bit correct...");
	// CP5,CP4,CP3,CP2,CP1,CP0, 1, 1,LP15,LP14,LP13,LP12,...,LP1,LP0
	dwEccDiff = dwNandEcc ^ dw600Ecc;
	// ->CP5,CP4,CP3,CP2,CP1,CP0,LP15,LP14,LP13,LP12,...,LP1,LP0
	dwEccDiff = ((dwEccDiff >> 2) & 0x003f0000) | (dwEccDiff & 0x0000ffff);
	if (((dwEccDiff ^ (dwEccDiff>>1)) & 0x00155555) == 0x00155555)	// correctable
	{
		DWORD CP = (dwEccDiff & 0x003f0000) >> 16;	// column parity, CP5~CP0
		DWORD LP = dwEccDiff & 0x0000ffff;	// line parity, LP15~LP0
		DWORD i, byteAddr = 0, bitAddr = 0;

		for (i = 0 ; i < 8 ; i++)	// find error byte by LP
		{	// check LP15,LP13,LP11,LP9,LP7,LP5,LP3,LP1
			byteAddr <<= 1;
			if (LP & 0x8000)
				byteAddr |= 1;
			LP <<= 2;
		}
		for (i = 0 ; i < 3 ; i++)	// find error bit by CP
		{	// check CP5,CP3,CP1
			bitAddr <<= 1;
			if (CP & 0x20)
				bitAddr |= 1;
			CP <<= 2;
		}
		if (byteAddr >= datalen)
		{
			MP_DEBUG("\tdata correction is out of range!(%d,%d)", byteAddr, bitAddr);
			//MemDump(pbBuffer, 256);
		}
		else
		{
			MP_DEBUG("\t1Bit ECC: [%d:%d]=%x", byteAddr, bitAddr, pbBuffer[byteAddr]);
			pbBuffer[byteAddr] ^= 1<<bitAddr;
			MP_DEBUG("\t\t->%x", pbBuffer[byteAddr]);
		}
	}
	else
	{
		MP_DEBUG("\t1Bit ECC uncorrectable (%x, %x)", dwEccDiff, (dwEccDiff ^ (dwEccDiff>>1)));
		ret = FAIL;
	}

	return ret;
}

static SWORD ECC_1Bit_Handler(BYTE *dwDataAddr, DWORD nr)
{
	SWORD ret = PASS;
	DWORD i;
	volatile DWORD *datptr = pstMcEcc->MC_ECC_ERR[nr].MC_ECC_ERR;
	DWORD ECCCode[2], ECCNand[2];

	ECCCode[0] = datptr[0] >> 8;
	ECCCode[1] = ((datptr[0] & 0xff) << 16) | (datptr[1] & 0xffff);

	datptr = &pstMcEcc->MC_ECC_CRC[nr<<1];
	ECCNand[0] = datptr[0] >> 8;
	ECCNand[1] = ((datptr[0] & 0xff) << 16) | (datptr[1] >> 16);

	if (ECCCode[0] != ECCNand[0])
	{
		if (ECC_1Bit_Correct(dwDataAddr, ECCNand[0], ECCCode[0], 256) != PASS)
		{
			MP_DEBUG("\t->%x,%x", ECCNand[0], ECCCode[0]);
			ret = 1;
		}
	}

	if (ECCCode[1] != ECCNand[1])
	{
		if (ECC_1Bit_Correct(dwDataAddr + 256, ECCNand[1], ECCCode[1], 256) != PASS)
		{
			MP_DEBUG("\t->%x,%x", ECCNand[1], ECCCode[1]);
			ret |= 2;
		}
	}

	return ret;
}

static SWORD ECC_Bch_Handler(BYTE *dwDataAddr, DWORD nr)
{
	SWORD ret = PASS;
	volatile WORD *info = (WORD *)pstMcEcc->MC_ECC_ERR[nr].MC_ECC_ERR;
	BYTE ErrNr = *(((BYTE *)pstMcEcc->MC_ECC_ERRCNT) + nr);
	DWORD i;

	if (pstMcEcc->MC_ECC_PASSFAIL & (0x00010000 << nr))	// uncorrectable
	{
		MP_DEBUG("\t\tuncorrectable: 0x%x (%d bit)", pstMcEcc->MC_ECC_PASSFAIL, ErrNr);
		ret = FAIL;
	}
	else
	{
		MP_DEBUG("\tfound %d error:", ErrNr);
		for (i = 0 ; i < ErrNr ; i++)
		{
			WORD ErrValue = info[i];
			WORD BitValue = 1 << (ErrValue & 0x07);	// which bit is wrong
			WORD BytePos = ErrValue >> 3;	// position where wrong data is
			if (BytePos < (512 << NandParameter.ECC_Size))
			{
				MP_DEBUG("\t\t ecc correct...addr=%x, data %x->%x", BytePos, dwDataAddr[BytePos], dwDataAddr[BytePos]^BitValue);
				dwDataAddr[BytePos] ^= BitValue;
			}
		}
	}

	return ret;
}

static SWORD ECC1S_FtlCheck(BYTE *dwDataAddr)
{
	SWORD ret = PASS;
	DWORD nand = pstMcEcc->MC_FTL_ECCDATA_RD >> 8;
	DWORD cal = pstMcEcc->MC_FTL_ECCDATA_EXE >> 8;

	if (cal != nand)
	{
		if (ECC_1Bit_Correct(dwDataAddr, nand, cal, NandParameter.EccOffset - 3) != PASS)
		{
			MP_DEBUG("ECC=0x%x, 0x%x", cal, nand);
			ret = FAIL;
		}
	}

	return ret;
}

static SWORD WaitReadyBusy()
{
#if TRIGGER_BY_INT
	DWORD event;

	pstMcard->McSmIc |= 0x00000100;	// enable ready/busy interrupt
	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &event, 5000) != PASS)
		mpDebugPrint("R/b timeout %x", pstMcard->McSmIc);

	return (event & IC_SRB) ? PASS : FAIL;
#else
	SWORD ret = TIMEOUT;
	DWORD tmr = GetSysTime();

	do
	{
		if (!(pstMcard->McSmIc & IC_SRB))
		{
			ret = PASS;
			break;
		}
	}
	while (SystemGetElapsedTime(tmr) < 1000);	// 1sec

	return ret;
#endif
}
#if !LOADER_CODE
// two chip eanble could be selected now
static DWORD CE_ADDRESS_OFFSET = 1;
static void NandSetChipNr(BYTE chips, DWORD MaxPageNr)
{
	CE_ADDRESS_OFFSET = (DWORD) MaxPageNr / chips;
	MP_DEBUG("%d die", chips);
}

static void NandSetCEx(DWORD dwPhyAddr)
{
    DWORD result = (DWORD) dwPhyAddr / CE_ADDRESS_OFFSET;

    switch(result)
    {
        case 0:
			//mpDebugPrint("PageNr=%x, CE0", rowAddr);
			pstMcard->McSmC &= ~(0x3 << CE_BIT_OFFSET);
            break;
        case 1:
			// disable CE0 and enable CE1
			//mpDebugPrint("PageNr=%x, CE1", rowAddr);
			pstMcard->McSmC |= 0x1 << CE_BIT_OFFSET;
            break;

#ifdef NAND_MULTI_CE_SUPPOUR
		case 2:
		case 3:
			//Setting multi CE GPIO
			mpDebugPrint("Out of NAND address range");
			break;
#endif

        default:
			mpDebugPrint("Out of NAND address range");
            break;
    }
}

static BYTE ReadStatus()
{
	BYTE status;

	do
	{
		SEND_COMMAND(READ_STATUS);
		status = READ_DATA();
	} while (!(status & 0x40));

	return status;
}
#endif
static void SetAddress(DWORD Row, DWORD Col)
{
	DWORD i;

	//mpDebugPrint("Addr=%d,%d", Row, Col);
	pstMcard->McSmC &= ~(7 << 5);
	pstMcard->McSmC |= (NandParameter.ColAddrNr + NandParameter.RowAddrNr - 3) << 5;
	for (i = 0 ; i < NandParameter.ColAddrNr ; i++)
	{
		SEND_ADDRESS(Col);
		Col >>= 8;
	}
	for (i = 0 ; i < NandParameter.RowAddrNr ; i++)
	{
		SEND_ADDRESS(Row);
		Row >>= 8;
	}
}
#if !LOADER_CODE
static void SetRowAddress(DWORD Row)	// for block erase
{
	DWORD i;

	//mpDebugPrint("Addr=%d", Row);
	NandSetCEx(Row);
	pstMcard->McSmC &= ~(7 << 5);
	if (NandParameter.RowAddrNr > 3)
		pstMcard->McSmC |= (NandParameter.RowAddrNr - 3) << 5;
	for (i = 0 ; i < NandParameter.RowAddrNr ; i++)
	{
		SEND_ADDRESS(Row);
		Row >>= 8;
	}
}
#endif
static void SetColAddress(DWORD col)
{
	DWORD i;

	//mpDebugPrint("Addr=%d", col);
	pstMcard->McSmC &= ~(7 << 5);
	if (NandParameter.ColAddrNr > 3)
		pstMcard->McSmC |= (NandParameter.ColAddrNr - 3) << 5;
	for (i = 0 ; i < NandParameter.ColAddrNr ; i++)
	{
		SEND_ADDRESS(col);
		col >>= 8;
	}
}
#if (NAND_CMD_MODE == HW_AUTO)
static void SetReadCmdAddr(DWORD row, DWORD col1, DWORD col2)
{
	pstMcEcc->MC_AUTO_ALE_DAT[0] = col1 | (row << 16);
	pstMcEcc->MC_AUTO_ALE_DAT[1] = row >> 16;
	pstMcEcc->MC_AUTO_ALE_DAT[3] = col2;
	pstMcEcc->MC_AUTO_CMD_DAT = READ_CMD_SET;
}

static void SetWriteCmdAddr(DWORD row, DWORD col)
{
	pstMcEcc->MC_AUTO_ALE_DAT[0] = col | (row << 16);
	pstMcEcc->MC_AUTO_ALE_DAT[1] = row >> 16;
	pstMcEcc->MC_AUTO_CMD_DAT = WRITE_CMD_SET;
}
#endif

#if !LOADER_CODE
static BOOL CheckStatusFlag(BYTE sflag)
{
	BYTE zeroCnt = 8;
	BYTE i;

	for (i = 0 ; i < 8 ; i++)
	{
		if (sflag & 0x01)
			zeroCnt--;
		sflag >>= 1;
	}

	return zeroCnt;
}

static BOOL CheckEvenParity(WORD dat)
{
	DWORD i, even = 0;

	for (i = 0 ; i < 16 ; i++)
	{
		if (dat & 0x01)
			even++;
		dat >>= 1;
	}

	return !(even&0x01);
}

// XD Tcode 280, Host Guideline Page 18 Figure 4.6
static WORD ReadLogBlkAddr(BYTE *redundent, DWORD maxAddr)
{
	#define CHECKRANGE(x)	(((x & 0xf800) == 0x1000) && (((x >> 1) & 0x03ff) < maxAddr))
	WORD blkaddr = (redundent[LOG_BLK_ADDR_HI] << 8) + redundent[LOG_BLK_ADDR_LO];
	WORD ret = -1;

	if (CheckEvenParity(blkaddr) && CHECKRANGE(blkaddr))
		ret = (blkaddr >> 1) & 0x3ff;

	return ret;
}
#endif
static SWORD ReadFtlArea(DWORD dwPhyAddr, BYTE *bRedtData)
{
	SWORD swRetValue = PASS;
	DWORD retry = ECC_FAIL_RETRY, ecc_chk = PASS;
	BYTE *spare = (BYTE *)pstMcEcc->MC_FTL_REG;
	DWORD row = dwPhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.

	do
	{
		ecc_chk = PASS;
		// spare area address
		NandSetCEx(row);
		if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
		{
			SEND_COMMAND(READ_REDUNDANT_CMD);
			SetAddress(row, 0);
		}
		else
		{
			SEND_COMMAND(READ_PAGE1_CMD);
			SetAddress(row, 1 << NandParameter.PageSize2Exp);
			SEND_COMMAND(READ_PAGE1_CMD_2CYC);
		}
		WaitReadyBusy();
		// ECC
		pstMcard->McSmC &= ~ECC_ENABLE;
		pstMcard->McSmC |= ECC_ENABLE;
		START_MCARD_FUNC(FTL_ONLY, DIR_READ, 1, SW_MANUAL);
		swRetValue = WaitEccAction();
		if (swRetValue != PASS)
		{
			mpDebugPrint("read FTL area failed(1):");
			MemDump(spare, NandParameter.EccOffset - 3);
			swRetValue = FAIL;
		}
		else
		{
			// correction
			memcpy(bRedtData, spare, NandParameter.EccOffset - 3);
			if (ECC1S_FtlCheck(bRedtData) != PASS)
			{
				if (retry)
				{
					ecc_chk = FAIL;
				}
				else
				{
					MP_DEBUG("FTL has ECC error!");
					//MemDump(bRedtData, NandParameter.EccOffset - 3);
				}
			}
			else
			{
				if (retry != ECC_FAIL_RETRY)
					MP_DEBUG("FTL ECC retry %d times", ECC_FAIL_RETRY-retry);
			}
		}
	} while (retry-- && ecc_chk);

	return swRetValue;
}

#if !LOADER_CODE
WORD GetNandLogBlkAddr(DWORD PhyAddr, DWORD MaxLogBlkAddr)
{
	WORD swRetValue;
	BYTE spare[FTL_LENGTH];
	DWORD badpos = (NandParameter.PageSize2Exp > MCARD_SECTOR_SIZE_EXP) ? BLOCK_STATUS_N : BLOCK_STATUS_X;

	if (ReadFtlArea(PhyAddr, spare) != PASS)
	{
		if (spare[badpos] == 0)
			swRetValue = LB_DEFAULT_BAD_BLOCK;
		else if (CheckStatusFlag(spare[badpos]) >= 2)	// bad block: more than 2 bits zero
			swRetValue = LB_BAD_BLOCK;
		else
			swRetValue = LB_READ_FAIL;
	}
	else if (spare[badpos] == 0)
		swRetValue = LB_DEFAULT_BAD_BLOCK;
	else if (CheckStatusFlag(spare[badpos]) >= 2)	// bad block: more than 2 bits zero
		swRetValue = LB_BAD_BLOCK;
	else
		swRetValue = ReadLogBlkAddr(spare, MaxLogBlkAddr);

	return swRetValue;
}

void DumpSpareContent(DWORD PhyAddr)
{
	DWORD i;
	BYTE spare[512];
	DWORD sz = NandParameter.SpareSize;
	DWORD row = PhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.

	NandSetCEx(row);
	if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
	{
		SEND_COMMAND(READ_REDUNDANT_CMD);
		SetAddress(row, 0);
	}
	else
	{
		SEND_COMMAND(READ_PAGE1_CMD);
		SetAddress(row, 1 << NandParameter.PageSize2Exp);
		SEND_COMMAND(READ_PAGE1_CMD_2CYC);
	}
	WaitReadyBusy();
	for (i = 0 ; i < sz ; i++)
		spare[i] = READ_DATA();
	MemDump(spare, sz);
}

void DumpPageContent(DWORD PhyAddr)
{
	DWORD i;
	DWORD sz = (1 << NandParameter.PageSize2Exp) + NandParameter.SpareSize;
	DWORD row = PhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.
	BYTE *buf = (BYTE *)ext_mem_malloc(sz);

	NandSetCEx(row);
	if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
	{
		SEND_COMMAND(READ_PAGE1_CMD);
		SetAddress(row, 0);
	}
	else
	{
		SEND_COMMAND(READ_PAGE1_CMD);
		SetAddress(row, 0);
		SEND_COMMAND(READ_PAGE1_CMD_2CYC);
	}
	WaitReadyBusy();
	for (i = 0 ; i < sz ; i++)
		buf[i] = READ_DATA();
	MemDump(buf, sz);
	ext_mem_free(buf);
}

SWORD CheckSpareContent(DWORD PhyAddr)
{
	DWORD i;
	SWORD Has_Content = FALSE;
	BYTE spare[512];
	DWORD sz = NandParameter.SpareSize;
	DWORD row = PhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.

	NandSetCEx(row);
#if 0 // caused 512 bytes per page ECC failed. Tim 2010/05/05
	if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
	{
		SEND_COMMAND(READ_REDUNDANT_CMD);
		SetAddress(row, 0);
	}
	else
#endif
	{
		SEND_COMMAND(READ_PAGE1_CMD);
		SetAddress(row, 1 << NandParameter.PageSize2Exp);
		SEND_COMMAND(READ_PAGE1_CMD_2CYC);
	}
	WaitReadyBusy();
	for (i = 0 ; i < NandParameter.SpareSize ; i++)
		spare[i] = READ_DATA();
	// correction
	while (sz--)
	{
		if (spare[sz] != 0xff)
		{
			Has_Content = TRUE;
			break;
		}
	}
	#if 0	// for debug
	if (Has_Content)
		MemDump(spare, NandParameter.SpareSize);
	#endif

	return Has_Content;
}
#endif


static SWORD NandPhyTransfer(DWORD dwBufferAddress, DWORD Size)
{
	pstMcard->McardC &= ~MCARD_FIFO_ENABLE;
	// DMA
	pstChannel->Control = 0x0;
	pstChannel->StartA = dwBufferAddress;
	pstChannel->EndA = dwBufferAddress + Size - 1;
	pstChannel->Control = MCARD_DMA_ENABLE;
	// MCard
	pstMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE + (Size >> 2);
	pstMcard->McardC |= MCARD_FIFO_ENABLE;

	return PASS;
}

static BYTE GetBCHByteNr(DWORD type)
{
	BYTE ret = 0;

	if (type == ECC_1BIT)
		ret = 6;
	else if (type == ECC_8BIT)
		ret = 14;
	else if (type == ECC_12BIT)
		ret = 21;
	else if (type == ECC_16BIT)
		ret = 28;
	else if (type == ECC_24BIT)
		ret = 42;

	return ret;
}

#if !LOADER_CODE
void NandRawPageDump(DWORD dwPhyAddr)
{
	BYTE *ptr = (BYTE *)((DWORD)ext_mem_malloc((1<<NandParameter.PageSize2Exp)+NandParameter.SpareSize) | 0xA0000000);
	DWORD row = dwPhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.
	DWORD i;
	mpDebugPrint("Dumping page %d", row);
	NandSetCEx(row);
	if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
	{
		SEND_COMMAND(READ_PAGE1_CMD);
		SetAddress(row, 0);
	}
	else
	{
		SEND_COMMAND(READ_PAGE1_CMD);
		SetAddress(row, 0);
		SEND_COMMAND(READ_PAGE1_CMD_2CYC);
	}
	if (WaitReadyBusy() != PASS)
	{
		mpDebugPrint("\t  %s: R/B timeout(1), PageAddr=%d", __FUNCTION__,row);
	}
	else
	{
		for (i = 0 ; i < (1<<NandParameter.PageSize2Exp)+NandParameter.SpareSize ; i++)
		{
			ptr[i] = READ_DATA();
		}
		MemDump(ptr, (1<<NandParameter.PageSize2Exp)+NandParameter.SpareSize);
	}
	ext_mem_free(ptr);
}

static SWORD SectorRead(DWORD PhyAddr, DWORD dwBufferAddress)
{
	SWORD swRetValue = PASS;
	DWORD retry = ECC_FAIL_RETRY, ecc_chk = PASS;
	DWORD i;
	DWORD nr = SECTOR_PER_PAGE;
	DWORD sz;
	DWORD cur = PhyAddr % nr;
	DWORD row = PhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.

	MP_DEBUG("\t%s: phyAddr=%d,buf=%x", __FUNCTION__, PhyAddr, dwBufferAddress);

	sz = GetBCHByteNr(NandParameter.ECC_Type);
	do
	{
		ecc_chk = PASS;
		// addressing, row+col
		NandSetCEx(row);
#if (NAND_CMD_MODE == SW_MANUAL)
		if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
		{
			SEND_COMMAND(READ_REDUNDANT_CMD);
			SetAddress(row, NandParameter.EccOffset);
		}
		else
		{
			SEND_COMMAND(READ_PAGE1_CMD);
			SetAddress(row, (1 << NandParameter.PageSize2Exp)+NandParameter.EccOffset+(sz*cur));
			SEND_COMMAND(READ_PAGE1_CMD_2CYC);
		}
		if ((swRetValue = WaitReadyBusy()))
		{
			mpDebugPrint("\t  %s: R/B timeout(1)", __FUNCTION__);
			return swRetValue;
		}
		// ECC
		START_MCARD_FUNC(ECC_ONLY, DIR_READ, 1, SW_MANUAL);
		swRetValue = WaitEccAction();
		if (swRetValue != PASS)
			mpDebugPrint("\t  %s: read ECC area failed!", __FUNCTION__);

		if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
		{
			SEND_COMMAND(READ_PAGE1_CMD);
			SetAddress(row, 0);
			if ((swRetValue = WaitReadyBusy()))
			{
				mpDebugPrint("\t  %s: R/B timeout(2)", __FUNCTION__);
				return swRetValue;
			}
		}
		else
		{
			// sector address, col only
			SEND_COMMAND(READ_IN_PAGE_S_CMD);
			SetColAddress(cur<<MCARD_SECTOR_SIZE_EXP);
			SEND_COMMAND(READ_IN_PAGE_E_CMD);
			// no waiting while read within page
		}
		START_ECC();
		START_MCARD_FUNC(DATA_ONLY, DIR_READ, 1, SW_MANUAL);
#else
		SetReadCmdAddr(row, (1 << NandParameter.PageSize2Exp)+NandParameter.EccOffset+(sz*cur), cur<<MCARD_SECTOR_SIZE_EXP);
		START_ECC();
		START_MCARD_FUNC(ALL_IN_ONE, DIR_READ, 1, HW_AUTO);
#endif

		NandPhyTransfer(dwBufferAddress, MCARD_SECTOR_SIZE);
		swRetValue = WaitEccAction();
		if (swRetValue != PASS)
		{
			mpDebugPrint("\t  %s: read data area failed!", __FUNCTION__);
			MemDump((BYTE *)dwBufferAddress, MCARD_SECTOR_SIZE);
		}

		if (pstMcEcc->MC_ECC_PASSFAIL & 0x01)
		{
			if (ECC_Correct((BYTE *)dwBufferAddress, 0) != PASS)
			{
				if (CheckSpareContent(PhyAddr) == TRUE)	// has content
				{
					if (retry)
					{
						retry--;
						ecc_chk = FAIL;
					}
					else
					{
						swRetValue = FAIL;
						mpDebugPrint("\t  %s: ECC failed at sector %d!", __FUNCTION__, PhyAddr);
						#if 0	// for debug
						MemDump((BYTE *)dwBufferAddress, MCARD_SECTOR_SIZE);
						DumpSpareContent(PhyAddr);
						MemDump((BYTE *)pstMcEcc->MC_ECC_CRC, 128);
						#endif
					}
				}
			}
		}
		else
		{
			if (retry != ECC_FAIL_RETRY)
				MP_DEBUG("\t\t read(1) retry %d times", ECC_FAIL_RETRY-retry);
		}
	} while(retry && ecc_chk);

	return swRetValue;
}
#endif

// read 1 sector (512B) from a page
// dwPhyAddr means unit of sector
static SWORD BootSectorRead(DWORD row, DWORD pbBuffer)
{
	SWORD swRetValue = PASS;
	DWORD i;
	DWORD ecctype = NandParameter.ECC_Type;
	void *tmp = ECC_Correct;

	MP_DEBUG("\t%s: phyAddr=%d,buf=%x", __FUNCTION__, row, pbBuffer);

	pstMcEcc->MC_ECC_CNFG = ECC_8BIT;
	NandParameter.ECC_Type = ECC_8BIT;
	ECC_Correct = ECC_Bch_Handler;
	// spare area address
	NandSetCEx(row);
	if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
	{
		SEND_COMMAND(READ_REDUNDANT_CMD);
		SetAddress(row, 0);
	}
	else
	{
		SEND_COMMAND(READ_PAGE1_CMD);
		SetAddress(row, MCARD_SECTOR_SIZE);	// stored as 512 bytes data + 16 byte spare
		SEND_COMMAND(READ_PAGE1_CMD_2CYC);
	}
	if (WaitReadyBusy() != PASS)
	{
		mpDebugPrint("\t  %s: R/B timeout(3), PageAddr=%d", __FUNCTION__, row);
		swRetValue = FAIL;
	}

	// ECC
	pstMcard->McardC |= DIR_READ << 4;
	pstMcEcc->MC_ECC_OPCT[0] = 0;
	pstMcard->McSmEccIc = 0x0f000000 | (TRIGGER_BY_INT << 8);
	pstMcEcc->MC_ECC_OPCT[0] = ECC_FLOW_EN | ECC_FLOW_RST | (ECC_ONLY << 8);
	swRetValue = WaitEccAction();
	if (swRetValue != PASS)
		mpDebugPrint("\t  %s: read ECC area failed!", __FUNCTION__);
	if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
	{
		SEND_COMMAND(READ_PAGE1_CMD);
		SetAddress(row, 0);
		if ((swRetValue = WaitReadyBusy()))
		{
			mpDebugPrint("\t  %s: R/B timeout(4)", __FUNCTION__);
			swRetValue = FAIL;
		}
	}
	else
	{
		// page aligned address, col only
		SEND_COMMAND(READ_IN_PAGE_S_CMD);
		SetColAddress(0);
		SEND_COMMAND(READ_IN_PAGE_E_CMD);
		// no waiting while read within page
	}
	START_ECC();
	pstMcard->McardC |= DIR_READ << 4;
	pstMcEcc->MC_ECC_OPCT[0] = 0;
	pstMcard->McSmEccIc = 0x0f000000 | (TRIGGER_BY_INT << 8);
	pstMcEcc->MC_ECC_OPCT[0] = ECC_FLOW_EN | ECC_FLOW_RST | (DATA_ONLY << 8);

	NandPhyTransfer(pbBuffer, MCARD_SECTOR_SIZE);
	swRetValue = WaitEccAction();
	if (swRetValue != PASS)
	{
		mpDebugPrint("\t  %s: read data area failed!", __FUNCTION__);
		MemDump((BYTE *)pbBuffer, 1 << NandParameter.PageSize2Exp);
	}

	if (pstMcEcc->MC_ECC_PASSFAIL & 0x1)
	{
		if (ECC_Correct((BYTE *)pbBuffer, 0) != PASS)
		{
			swRetValue = FAIL;
			mpDebugPrint("\t  %s: ECC failed at row %d!", __FUNCTION__, row);
		}
	}
	NandParameter.ECC_Type = ecctype;
	pstMcEcc->MC_ECC_CNFG = (NandParameter.ECC_Size << 4) | NandParameter.ECC_Type;
	ECC_Correct = tmp;

	return swRetValue;
}

// dwPhyAddr means unit of sector
SWORD NandSectorRead(DWORD dwPhyAddr, DWORD dwSectorCount, DWORD dwBufferAddress, DWORD *progress, BYTE *SectorStatus)
{
	SWORD swRetValue = PASS;
	DWORD retry = ECC_FAIL_RETRY;
	DWORD i;
	BYTE *pbBuffer = (BYTE *) dwBufferAddress;
	DWORD nr = SECTOR_PER_PAGE;
	DWORD cur = dwPhyAddr % nr, wCount = 0;
	DWORD row = dwPhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.
	DWORD EccFailMask = (1 << nr) - 1;

	MP_DEBUG("\t%s: phyAddr=%d,SectorNrs=%d,buf=%x", __FUNCTION__, dwPhyAddr, dwSectorCount, dwBufferAddress);
#if !LOADER_CODE
	if (cur != 0)	// read data, sector by sector
	{
		DWORD limit = (nr > dwSectorCount) ? dwSectorCount : nr;
		for (i = cur ; i < limit ; i++)
		{
			if (SectorRead(dwPhyAddr, dwBufferAddress) != PASS)
			{
				if (SectorStatus)
					SectorStatus[wCount] = 1;
				swRetValue = FAIL;
			}
			dwPhyAddr++;
			dwSectorCount--;
			wCount++;
			dwBufferAddress += MCARD_SECTOR_SIZE;
			if (progress)
				(*progress)++;
		}
		row++;
	}
#endif
	while (dwSectorCount >= nr)	// read data, page by page
	{
		DWORD ecc_chk = PASS;

		// spare area address
		NandSetCEx(row);
#if (NAND_CMD_MODE == SW_MANUAL)
		if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
		{
			SEND_COMMAND(READ_REDUNDANT_CMD);
			SetAddress(row, NandParameter.EccOffset);
		}
		else
		{
			SEND_COMMAND(READ_PAGE1_CMD);
			SetAddress(row, (1 << NandParameter.PageSize2Exp) + NandParameter.EccOffset);
			SEND_COMMAND(READ_PAGE1_CMD_2CYC);
		}
		if (WaitReadyBusy() != PASS)
		{
			mpDebugPrint("\t  %s: R/B timeout(1), PageAddr=%d,sector=%d", __FUNCTION__,row,wCount);
			if (SectorStatus)
				SectorStatus[wCount] = 1;
			swRetValue = FAIL;
			continue;
		}

		// ECC
		START_MCARD_FUNC(ECC_ONLY, DIR_READ, ECC_PER_PAGE, SW_MANUAL);
		swRetValue = WaitEccAction();
		if (swRetValue != PASS)
			mpDebugPrint("\t  %s: read ECC area failed!", __FUNCTION__);
		if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
		{
			SEND_COMMAND(READ_PAGE1_CMD);
			SetAddress(row, 0);
			if (WaitReadyBusy() != PASS)
			{
				mpDebugPrint("\t  %s: R/B timeout(2), PageAddr=%d,sector=%d", __FUNCTION__,row,wCount);
				if (SectorStatus)
					SectorStatus[wCount] = 1;
				swRetValue = FAIL;
				continue;
			}
		}
		else
		{
			// page aligned address, col only
			SEND_COMMAND(READ_IN_PAGE_S_CMD);
			SetColAddress(0);
			SEND_COMMAND(READ_IN_PAGE_E_CMD);
			// no waiting while read within page
		}
		START_ECC();
		START_MCARD_FUNC(DATA_ONLY, DIR_READ, ECC_PER_PAGE, SW_MANUAL);
#else
		SetReadCmdAddr(row, (1 << NandParameter.PageSize2Exp) + NandParameter.EccOffset, 0);
		START_ECC();
		START_MCARD_FUNC(ALL_IN_ONE, DIR_READ, ECC_PER_PAGE, HW_AUTO);
#endif

		NandPhyTransfer(dwBufferAddress, 1 << NandParameter.PageSize2Exp);
		swRetValue = WaitEccAction();
		if (swRetValue != PASS)
		{
			mpDebugPrint("\t  %s: read data area failed!", __FUNCTION__);
			MemDump((BYTE *)dwBufferAddress, 1 << NandParameter.PageSize2Exp);
		}

		if (pstMcEcc->MC_ECC_PASSFAIL & EccFailMask)
		{
			register DWORD FailValue = pstMcEcc->MC_ECC_PASSFAIL;
			for (i = 0 ; i < ECC_PER_PAGE ; i++)
			{
				if (FailValue & (1 << i))
				{
					if (ECC_Correct((BYTE *)(dwBufferAddress+(i<<(MCARD_SECTOR_SIZE_EXP+NandParameter.ECC_Size))), i) != PASS)
					{
#if !LOADER_CODE
						if (CheckSpareContent(dwPhyAddr+i) == TRUE)	// has content
#endif
						{
							MP_DEBUG("\t  %s: ECC error at sector %d!", __FUNCTION__, dwPhyAddr+i);
							if (retry)
							{
								retry--;
								ecc_chk = FAIL;
								break;
							}
							else
							{
								if (SectorStatus)
									SectorStatus[wCount+i] = 1;
								swRetValue = FAIL;
								mpDebugPrint("\t  %s: ECC failed at sector %d!", __FUNCTION__, dwPhyAddr+i);
//mpDebugPrint("------ SRAM ECC dump ------");
//BYTE tmp = ((GetBCHByteNr(NandParameter.ECC_Type) + 3) >> 2) << 2;
//MemDump((BYTE *)pstMcEcc->MC_ECC_CRC, tmp*nr);
//NandECCSramDump(ECC_PER_PAGE);
//mpDebugPrint("------ Data dump ------");
//								MemDump((BYTE *)dwBufferAddress, 1 << NandParameter.PageSize2Exp);
//mpDebugPrint("------ ECC dump ------");
//								NandRawPageDump(dwPhyAddr+i);
//while(1);
							}
						}
					}
					else
					{
						if (retry != ECC_FAIL_RETRY)
							MP_DEBUG("\t\t read(2) retry %d times", ECC_FAIL_RETRY-retry);
					}
				}
				//MemDump((BYTE *)pstMcEcc->MC_ECC_ERR[i].MC_ECC_ERR, 8);
			}
		}
		#if 0	// for debug
		if (swRetValue)
		{
			mpDebugPrint("**********************************************************");
			mpDebugPrint("*********************read fail****************************");
			mpDebugPrint("**********************************************************");
			BYTE tmp = ((GetBCHByteNr(NandParameter.ECC_Type) + 3) >> 2) << 2;
			MemDump((BYTE *)pstMcEcc->MC_ECC_CRC, tmp*nr);
			DumpPageContent(dwPhyAddr);
		}
		#endif
		if (ecc_chk == PASS)
		{
			retry = ECC_FAIL_RETRY;
			wCount += nr;
			dwBufferAddress += nr << MCARD_SECTOR_SIZE_EXP;
//mpDebugPrint("Read page: %x", dwPhyAddr);
			dwPhyAddr += nr;
			dwSectorCount -= nr;
			row++;
			if (progress)
				(*progress) += nr;
/*
mpDebugPrint("------ SRAM ECC dump ------");
BYTE tmp = ((GetBCHByteNr(NandParameter.ECC_Type) + 3) >> 2) << 2;
MemDump((BYTE *)pstMcEcc->MC_ECC_CRC, tmp*nr);

//mpDebugPrint("------ Data dump ------");
//								MemDump((BYTE *)dwBufferAddress, 1 << NandParameter.PageSize2Exp);
mpDebugPrint("------ ECC dump ------");
//								NandRawPageDump(dwPhyAddr+i);
NandECCSramDump(nr);
while(1);
*/
		}
	}

#if !LOADER_CODE
	for (i = 0 ; i < dwSectorCount ; i++)	// read data, sector by sector
	{
		if (SectorRead(dwPhyAddr, dwBufferAddress) != PASS)
		{
			if (SectorStatus)
				SectorStatus[wCount] = 1;
				swRetValue = FAIL;
		}
		wCount++;
		dwPhyAddr++;
		dwBufferAddress += MCARD_SECTOR_SIZE;
	}
#endif

	return swRetValue;
}

#if !LOADER_CODE
// dwPhyAddr means unit of sector
SWORD NandRawPageRead(DWORD dwPhyAddr, DWORD dwBufferAddress)
{
	SWORD swRetValue = PASS;
	DWORD retry = ECC_FAIL_RETRY;
	DWORD i;
	BYTE *pbBuffer = (BYTE *) dwBufferAddress;
	DWORD nr = SECTOR_PER_PAGE;
	DWORD row = dwPhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.
	DWORD ecc_chk = FAIL;
	DWORD EccFailMask = (1 << nr) - 1;

	MP_DEBUG("\t%s: phyAddr=%d,buf=%x", __FUNCTION__, dwPhyAddr, dwBufferAddress);

	while (ecc_chk)
	{
		ecc_chk = PASS;
		// spare area address
		NandSetCEx(row);
		if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
		{
			SEND_COMMAND(READ_REDUNDANT_CMD);
			SetAddress(row, NandParameter.EccOffset);
		}
		else
		{
			SEND_COMMAND(READ_PAGE1_CMD);
			SetAddress(row, (1 << NandParameter.PageSize2Exp)+NandParameter.EccOffset);
			SEND_COMMAND(READ_PAGE1_CMD_2CYC);
		}
		if (WaitReadyBusy() != PASS)
		{
			mpDebugPrint("\t  %s: R/B timeout(1), PageAddr=%d", __FUNCTION__, row);
			swRetValue = FAIL;
			break;
		}

		// ECC
		START_MCARD_FUNC(ECC_ONLY, DIR_READ, ECC_PER_PAGE, SW_MANUAL);
		swRetValue = WaitEccAction();
		if (swRetValue != PASS)
		{
			mpDebugPrint("\t  %s: read ECC area failed!", __FUNCTION__);
			break;
		}

		if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
		{
			SEND_COMMAND(READ_PAGE1_CMD);
			SetAddress(row, 0);
			if (WaitReadyBusy() != PASS)
			{
				mpDebugPrint("\t  %s: R/B timeout(2), PageAddr=%d", __FUNCTION__,row);
				swRetValue = FAIL;
				continue;
			}
		}
		else
		{
			// page aligned address, col only
			SEND_COMMAND(READ_IN_PAGE_S_CMD);
			SetColAddress(0);
			SEND_COMMAND(READ_IN_PAGE_E_CMD);
			// no waiting while read within page
		}

		START_ECC();
		START_MCARD_FUNC(DATA_ONLY, DIR_READ, ECC_PER_PAGE, SW_MANUAL);
		NandPhyTransfer(dwBufferAddress, 1 << NandParameter.PageSize2Exp);
		swRetValue = WaitEccAction();
		if (swRetValue != PASS)
		{
			mpDebugPrint("\t  %s: read data area failed!", __FUNCTION__);
			MemDump((BYTE *)dwBufferAddress, 1 << NandParameter.PageSize2Exp);
			break;
		}

		if (pstMcEcc->MC_ECC_PASSFAIL & EccFailMask)
		{
			for (i = 0 ; i < nr ; i++)
			{
				if (ECC_Correct((BYTE *)(dwBufferAddress+(i<<MCARD_SECTOR_SIZE_EXP)), i) != PASS)
				{
					MP_DEBUG("\t  %s: ECC error at sector %d!", __FUNCTION__, dwPhyAddr+i);
					if (retry)
					{
						retry--;
						ecc_chk = FAIL;
						break;
					}
					else
					{
						swRetValue = FAIL;
						mpDebugPrint("\t  %s: ECC failed at sector %d!", __FUNCTION__, dwPhyAddr+i);
					}
				}
				else
				{
					if (retry != ECC_FAIL_RETRY)
						MP_DEBUG("\t\t read(2) retry %d times", ECC_FAIL_RETRY-retry);
				}
			}
		}
	}
	pbBuffer += 1 << NandParameter.PageSize2Exp;
	for (i = 0 ; i < NandParameter.SpareSize ; i++)
		pbBuffer[i] = READ_DATA();

	return swRetValue;
}

static BOOL CalEvenParity(WORD dat)
{
	DWORD i, even = 0;

	for (i = 0 ; i < 16 ; i++)
	{
		if (dat & 0x01)
			even++;
		dat >>= 1;
	}

	return (even&0x01);
}

static void BuildFTLArea(volatile BYTE *bRedtData, DWORD LogBlkAddr,BYTE bValid)
{
	BYTE tmp[FTL_LENGTH];
	WORD BlkAddr = 0x1000 | (LogBlkAddr<<1);	// the block address is xD compatible
	DWORD badpos = (NandParameter.PageSize2Exp > MCARD_SECTOR_SIZE_EXP) ? BLOCK_STATUS_N : BLOCK_STATUS_X;

	BlkAddr |= CalEvenParity(BlkAddr);
	memset(tmp, 0xff, FTL_LENGTH);
	tmp[LOG_BLK_ADDR_HI] = (BYTE)(BlkAddr >> 8);
	tmp[LOG_BLK_ADDR_LO] = (BYTE)(BlkAddr & 0xff);

	if (bValid == SFT_BAD_BLOCK)
	{
		tmp[badpos] = 0xf0;	// block status flag
		//mpDebugPrint("Bad Block Mark");
	}
	else if (bValid == SFT_DATA_FAIL)
	{
		tmp[DATA_STATUS] = 0;	// data status flag
		//mpDebugPrint("Data Fail Mark");
	}
	memcpy((BYTE *)bRedtData, tmp, FTL_LENGTH);
}

// dwPhyAddr means unit of sector
static SWORD BootSectorWrite(DWORD row, DWORD pbBuffer)
{
	SWORD swRetValue = PASS;
	DWORD i, j;
	DWORD wCount = 0;
	DWORD ecctype = NandParameter.ECC_Type;

	MP_DEBUG("\t%s: phyAddr=%d,buf=%x", __FUNCTION__, row, pbBuffer);
	pstMcEcc->MC_ECC_CNFG = ECC_8BIT;
	NandParameter.ECC_Type = ECC_8BIT;
	NandSetCEx(row);
	SEND_COMMAND(SEQ_DATA_IN_CMD);
	SetAddress(row, 0);
	// enable ECC module
	START_ECC();
	pstMcard->McardC &= ~MCARD_DIR_MASK;
	pstMcEcc->MC_ECC_OPCT[0] = 0;
	pstMcard->McSmEccIc = 0x0f000000 | (TRIGGER_BY_INT << 8);
	pstMcEcc->MC_ECC_OPCT[0] = ECC_FLOW_EN | ECC_FLOW_RST | ((DATA_ONLY) << 8);
	// start sending data
	NandPhyTransfer(pbBuffer, MCARD_SECTOR_SIZE);
	// wait for ECC finished
	if ((swRetValue = WaitEccAction()) != PASS)
		mpDebugPrint("-E- Nand write timeout: PageAddr=%d(%x)", row, pstMcEcc->MC_ECC_PASSFAIL);
	pstMcard->McardC &= ~MCARD_DIR_MASK;
	pstMcEcc->MC_ECC_OPCT[0] = 0;
	pstMcard->McSmEccIc = 0x0f000000 | (TRIGGER_BY_INT << 8);
	pstMcEcc->MC_ECC_OPCT[0] = ECC_FLOW_EN | ECC_FLOW_RST | ((ECC_ONLY) << 8); \
	swRetValue = WaitEccAction();
	if (swRetValue != PASS)
	{
		mpDebugPrint("\t\t%s: FTL wrote failed!", __FUNCTION__);
		swRetValue = FAIL;
	}
	// command Nand flash to start programming actually
	SEND_COMMAND(PAGE_PROG_CMD);
	if ((swRetValue = WaitReadyBusy()))
	{
		mpDebugPrint("-E- Nand write R/B timeout, PageAddr=%d", row);
		swRetValue = FAIL;
	}
	BYTE ret = ReadStatus();
	if (ret & 0x01)
	{
		mpDebugPrint("-E- Nand write, status failed, PageAddr=%d,status=%x", row, ret);
		swRetValue = FAIL;
	}
	NandParameter.ECC_Type = ecctype;
	pstMcEcc->MC_ECC_CNFG = (NandParameter.ECC_Size << 4) | NandParameter.ECC_Type;

	return swRetValue;
}


// dwPhyAddr means unit of sector
SWORD NandBlockWrite(DWORD dwPhyAddr, DWORD dwBufferAddress, DWORD LogBlkAddr, DWORD *progress, BYTE bValid, BYTE *SectorStatus)
{
	SWORD swRetValue = PASS;
	DWORD i;
	BYTE *pbBuffer = (BYTE *) dwBufferAddress;
	DWORD wCount = 0;
	DWORD row = dwPhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.

	MP_DEBUG("\t%s: phyAddr=%d,logBlkAddr=%d,buf=%x", __FUNCTION__, dwPhyAddr, LogBlkAddr, dwBufferAddress);
	NandSetCEx(row);
	for (i = 0 ; i < NandParameter.PagePerBlock ; i++)
	{
		if (SectorStatus && SectorStatus[wCount] != 0)
			BuildFTLArea((volatile BYTE *)pstMcEcc->MC_FTL_REG, LogBlkAddr, SFT_DATA_FAIL);
		else
			BuildFTLArea((volatile BYTE *)pstMcEcc->MC_FTL_REG, LogBlkAddr, bValid);
#if (NAND_CMD_MODE == SW_MANUAL)
		if (NandParameter.PageSize2Exp == MCARD_SECTOR_SIZE_EXP)
			SEND_COMMAND(READ_PAGE1_CMD);
		SEND_COMMAND(SEQ_DATA_IN_CMD);
		SetAddress(row, 0);
		// enable ECC module
		START_ECC();
		START_MCARD_FUNC(DATA_ONLY, DIR_WRITE, ECC_PER_PAGE, SW_MANUAL);
		// start sending data
		NandPhyTransfer((DWORD)pbBuffer, 1<<NandParameter.PageSize2Exp);
		// wait for ECC finished
		if ((swRetValue = WaitEccAction()) != PASS)
			mpDebugPrint("-E- Nand write timeout: PageAddr=%d(%x)", row, pstMcEcc->MC_ECC_PASSFAIL);
		pstMcard->McSmC &= ~ECC_ENABLE;
		pstMcard->McSmC |= ECC_ENABLE;
		START_MCARD_FUNC(FTL_ONLY, DIR_WRITE, 1, SW_MANUAL);
		swRetValue = WaitEccAction();
		if (swRetValue != PASS)
		{
			mpDebugPrint("\t\t%s: FTL wrote failed!", __FUNCTION__);
			swRetValue = FAIL;
			break;
		}
		START_MCARD_FUNC(ECC_ONLY, DIR_WRITE, ECC_PER_PAGE, SW_MANUAL);
		swRetValue = WaitEccAction();
		if (swRetValue != PASS)
		{
			mpDebugPrint("\t\t%s: ECC wrote failed!", __FUNCTION__);
			swRetValue = FAIL;
			break;
		}
		// command Nand flash to start programming actually
		SEND_COMMAND(PAGE_PROG_CMD);
		if ((swRetValue = WaitReadyBusy()))
		{
			mpDebugPrint("-E- Nand write R/B timeout, PageAddr=%d", row);
			swRetValue = FAIL;
			break;
		}
#else
		// enable ECC module
		SetWriteCmdAddr(row, 0);
		START_ECC();
		START_MCARD_FUNC(ALL_IN_ONE, DIR_WRITE, ECC_PER_PAGE, HW_AUTO);
		// start sending data
		NandPhyTransfer((DWORD)pbBuffer, 1<<NandParameter.PageSize2Exp);
		// wait for ECC finished
		if ((swRetValue = WaitEccAction()) != PASS)
			mpDebugPrint("-E- Nand write timeout: PageAddr=%d(%x)",row, pstMcEcc->MC_ECC_PASSFAIL);
		//MemDump((BYTE *)pstMcEcc->MC_ECC_CRC, ECC_PER_PAGE*ALIGN_4(GetBCHByteNr(NandParameter.ECC_Type)));
#endif
		BYTE ret = ReadStatus();
		if (ret & 0x01)
		{
			mpDebugPrint("-E- Nand write, status failed, PageAddr=%d,status=%x", row, ret);
			swRetValue = FAIL;
			break;
		}
		//NandRawPageDump(row<<(NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP));
		pbBuffer += 1<<NandParameter.PageSize2Exp;
		row++;
		wCount++;
		if (progress)
			(*progress) += 1 << (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);
	}

	return swRetValue;
}

// dwPhyAddr means unit of sector
SWORD NandRawPageWrite(DWORD dwPhyAddr, DWORD dwBufferAddress)
{
	SWORD swRetValue = PASS;
	DWORD i;
	BYTE *pbBuffer = (BYTE *) dwBufferAddress;
	DWORD row = dwPhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.

	MP_DEBUG("\t%s: phyAddr=%d,buf=%x", __FUNCTION__, dwPhyAddr, dwBufferAddress);
	NandSetCEx(row);
	SEND_COMMAND(SEQ_DATA_IN_CMD);
	SetAddress(row, 0);
	// enable ECC module
	START_ECC();
	START_MCARD_FUNC(DATA_ONLY, DIR_WRITE, ECC_PER_PAGE, SW_MANUAL);
	// start sending data
	NandPhyTransfer((DWORD)pbBuffer, 1<<NandParameter.PageSize2Exp);
	// wait for ECC finished
	if ((swRetValue = WaitEccAction()) != PASS)
	{
		mpDebugPrint("-E- Nand write timeout, PageAddr=%d",row);
	}
	pbBuffer += 1<<NandParameter.PageSize2Exp;
	for (i = 0 ; i < NandParameter.SpareSize ; i++)
		SEND_DATA(pbBuffer[i]);
	// command Nand flash to start programming actually
	SEND_COMMAND(PAGE_PROG_CMD);
	if ((swRetValue = WaitReadyBusy()))
	{
		mpDebugPrint("-E- Nand write R/B timeout, PageAddr=%d,sector=%d",row,i);
		swRetValue = FAIL;
	}
	BYTE ret = ReadStatus();
	if (ret & 0x01)
	{
		mpDebugPrint("-E- Nand write, status failed, PageAddr=%d,sector=%d,status=%x", row, i, ret);
		swRetValue = FAIL;
	}

	return swRetValue;
}

// dwPhyAddr is unit of sector
SWORD NandBlockErase(DWORD dwPhyAddr)
{
	SWORD swRetValue = PASS;
	DWORD row = dwPhyAddr >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);	// which page no.

	MP_DEBUG("\t%s: phyAddr=%d", __FUNCTION__, dwPhyAddr);
	SEND_COMMAND(BLOCK_ERASE_CMD_1CYC);
	SetRowAddress(row);
	SEND_COMMAND(BLOCK_ERASE_CMD_2CYC);
	swRetValue = WaitReadyBusy();
	if (swRetValue)
	{
		mpDebugPrint("-E- Nand erase R/B timeout, addr=%d", dwPhyAddr);
	}
	else
	{
		BYTE ret = ReadStatus();
		if (ret & 0x01)
		{
			swRetValue = FAIL;
			mpDebugPrint("-E- Nand erase FAIL, addr=%d,status=%x", dwPhyAddr, ret);
		}
	}

	return swRetValue;
}
#endif

void GetNandRawGeometry(DWORD *PageSz, DWORD *PagePerBlk)
{
	*PageSz = (1 << NandParameter.PageSize2Exp) + NandParameter.SpareSize;
	*PagePerBlk = NandParameter.PagePerBlock;
}

void GetNandGeometry(DWORD *BlkNr, DWORD *SectorPerBlk, DWORD *PageSize)
{
	*BlkNr = NandParameter.Block_Nr;
	*SectorPerBlk = NandParameter.SectorPerBlock;
	*PageSize = (1<<NandParameter.PageSize2Exp);
}

void MC_ECCCRC_test()
{
	int i;
	for(i = 0; i < 112; i++)
		pstMcEcc->MC_ECC_CRC[i] = 0x87654321;

	for(i = 0; i < 112; i++)
		mpDebugPrint("ECC crc[%d]: %x", i, pstMcEcc->MC_ECC_CRC[i]);

	for(i = 0; i < 112; i++)
		mpDebugPrint("%x(%x): %x", 0x08121800 + i * 4, (DWORD*)pstMcEcc->MC_ECC_CRC + i,
		                           (DWORD)*((DWORD*)0xa8021800 + i));

	mpDebugPrint("ECC CRC testing end, system hanged...");
	while(1);
}



#if LOADER_CODE
static const BYTE NandPins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 21, 22, 23};
#endif
void McardNandActive()
{
	DWORD i;

    ((CLOCK *)CLOCK_BASE)->Clkss_EXT2 &= ~BIT6;
#if LOADER_CODE
    CLOCK *sClock = (CLOCK *)CLOCK_BASE;
#ifdef MP650_FPGA_SD_CLK_FOR_HAPS52               // using PLL3
    sClock->Clkss_EXT1 &= ~(0xf << 16);
    sClock->Clkss_EXT1 |= (1 << 16);
    sClock->Clkss_EXT1 |= BIT20;
#else
	sClock->Clkss1 &= ~(0x0f<<28);
#ifdef RUN_ON_REAL_CHIP
	sClock->Clkss1 |= (2<<28);
#else
	sClock->Clkss1 |= (1<<28);
#endif
	sClock->MdClken |= (1<<13);
#endif
#endif
	pstMcEcc->MC_ECC_OPCT[0] = 0;
	pstMcard->McardC = 0;
#if LOADER_CODE
	pstMcard->SPI_Mode = 1;
#ifdef RUN_ON_REAL_CHIP
	GPIO *sGpio = (GPIO *)GPIO_BASE;
	for (i = 0 ; i < sizeof(NandPins) ; i++)
	{	// set to Alt. Func. 1
		sGpio->Fgpcfg[NandPins[i]>>4] &= ~(0x00010001 << (NandPins[i] & 0x0f));
		sGpio->Fgpcfg[NandPins[i]>>4] |= 0x00000001 << (NandPins[i] & 0x0f);
	}
#endif
#else
	McardSelect(PIN_DEFINE_FOR_NAND);
#endif
	pstMcard->McardC = (MCARD_FLASH_ENABLE | MCARD_DMA_SM);
	pstMcard->McRtm = (MCR_TRS<<24) | (MCR_HLD<<16) | (MCR_PLW<<8) | MCR_ST;
	pstMcard->McWtm = (MCW_TRS<<24) | (MCW_HLD<<16) | (MCW_PLW<<8) | MCW_ST;
	pstMcard->McSmIc = IM_ALL;
	pstMcard->McSmC = CS_NAND | TYPE_512_BYTE | FIFO_FIX;
	pstMcEcc->MC_FTL_NUMBER = NandParameter.EccOffset - 3;
	pstMcEcc->MC_ECC_CNFG = (NandParameter.ECC_Size << 4) | NandParameter.ECC_Type;
	pstMcEcc->MC_ECCBCH_C = 0;
#if (NAND_CMD_MODE == HW_AUTO)
	pstMcEcc->MC_AUTO_ALE_CYCNT = ((NandParameter.ColAddrNr + NandParameter.RowAddrNr) << 24) | (NandParameter.ColAddrNr << 16);
#endif

#ifdef	NAND_MULTI_CE_SUPPOUR
		//Setting multi CE GPIO
#endif
}

void McardNandInactive()
{
	DWORD i;
	GPIO *sGpio = (GPIO *)GPIO_BASE;

#if LOADER_CODE
#ifdef RUN_ON_REAL_CHIP
	sGpio->Fgpcfg[0] = 0;	// gpio
	sGpio->Fgpdat[0] = 0;	// input... means floating
	sGpio->Fgpcfg[1] = 0;	// gpio
	sGpio->Fgpdat[1] = 0;	// input
#endif
#else
	McardDeselect(PIN_DEFINE_FOR_NAND);
#endif
	pstMcEcc->MC_ECC_OPCT[0] = 0;
	pstMcard->McSmC = 0;
	pstMcard->McardC = 0;
	pstMcard->McSmIc = 0;
	pstMcard->McEcc4SC = 0;
	pstMcard->MC_ECC8S_C = 0;
}

#if LOADER_CODE

SWORD BootPageCheck(DWORD rowAddr, BYTE SmallBlk)
{
	DWORD chksum = 0;
	DWORD i, j;
	DWORD len = sizeof(NandInfo) / sizeof(DWORD);
	DWORD bp_nr = MCARD_SECTOR_SIZE / ((len + 1) * sizeof(DWORD));
	BYTE *buf = (BYTE *)((DWORD)ReservedBuffer | 0xA0000000);
	SWORD ret = PASS;

	McardNandActive();
	if (SmallBlk)
	{
		NandParameter.ColAddrNr = 1;
		NandParameter.PageSize2Exp = MCARD_SECTOR_SIZE_EXP;
	}
	else
	{
		// column will always be 2 cycles
		NandParameter.ColAddrNr = 2;
		NandParameter.PageSize2Exp = MCARD_SECTOR_SIZE_EXP+2;
	}
	NandParameter.ECC_Type = ECC_8BIT;
	// read 1 sector(512 bytes)
	DWORD *ptr = (DWORD *)buf;
	if (BootSectorRead(rowAddr, (DWORD)buf) != PASS)
	{
		chksum = ptr[len];
	}
	else
	{
		for (i = 0 ; i < bp_nr ; i++)	// there are many records in a sector
		{
			chksum = 0;
			for (i = 0 ; i < len ; i++)
				chksum += ptr[i];
			if (chksum == ~ptr[len])	// find a valid ID record
				break;
			ptr += len + 1;
		}
	}
	if (chksum != ~ptr[len])
		ret = FAIL;
	else
	{
		memcpy((BYTE *)&NandParameter, (BYTE *)ptr, len*sizeof(DWORD));	// found and use it
		MemDump(buf, 512);
	}

	return ret;
}

SWORD NandIdentify()
{
	SWORD swRetValue = FAIL;
	DWORD i;
	DWORD row_cfg = 7;

	NandSetCEx(0);
	WaitReadyBusy();
	SEND_COMMAND(RESET_CMD);
	IODelay(100);
	WaitReadyBusy();

	// use gpio to know address cycle
	// 0: 1 cycles, 1: 2 cycles, 2: 3 cycles, 3: 4 cycles,..., 7: 8 cycles.
	#ifdef RUN_ON_REAL_CHIP
	row_cfg = (((GPIO *)GPIO_BASE)->Vgpdat0 >> 5) & 0x07;
	#endif
	NandParameter.RowAddrNr = (row_cfg == 7) ? 1 : row_cfg + 1;
	for (i = 0 ; i < 64 ; i++)	// search from page 0 to page 64
	{
		for (; NandParameter.RowAddrNr < 7 ; NandParameter.RowAddrNr++)	// try row address cycle
		{
			if ((swRetValue = BootPageCheck(i, 0)) == PASS)	// get nand ID that were writen by AP or write machine
				break;
			else if ((swRetValue = BootPageCheck(i, 1)) == PASS)
				break;
		}
		if (swRetValue == PASS)
			break;
	}
	if (swRetValue == PASS)
	{
		if (NandParameter.ECC_Type == ECC_1BIT)
			ECC_Correct = ECC_1Bit_Handler;
		else
			ECC_Correct = ECC_Bch_Handler;

		UartOutText("Nand - (");
		UartOutValue(1 << NandParameter.PageSize2Exp, 4);
		PutUartChar('+');
		UartOutValue(NandParameter.SpareSize, 3);
		UartOutText(")x");
		UartOutValue(NandParameter.PagePerBlock, 3);
		PutUartChar('x');
		UartOutValue(NandParameter.Block_Nr, 4);
		UartOutText(", ECC:");
		UartOutValue(NandParameter.ECC_Type, 1);
		UartOutText(",Addr:");
		UartOutValue(NandParameter.RowAddrNr, 1);
		PutUartChar('+');
		UartOutValue(NandParameter.ColAddrNr, 1);
		UartOutText("\r\n");
	}
	else
	{
		UartOutText("No Nand ID!\r\n");
	}

	return swRetValue;
}


int LoadBinFromNAND(DWORD loading_addr)
{
	DWORD state, dwBlockAddr, i;
	BYTE *spare1 = (BYTE *)((DWORD)ReservedBuffer | 0xA0000000);
	BYTE *spare2 = (BYTE *)((DWORD)spare1 + 0x200);
	BYTE *AP_Code_start_addr = (BYTE *)loading_addr;
	DWORD *tmp = (DWORD *)((DWORD)spare2 + 0x200);

	McardNandActive();
	if(NandIdentify() == PASS)
	{
		// after identified,  active mcard with correct values
		McardNandInactive();
		McardNandActive();
		DWORD chksum, len, sectCnt;
		DWORD SectPerPage = 1 << (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);
		state = 0;
		UartOutText("Load From Nand...");
		for(dwBlockAddr = 1 ; dwBlockAddr < NandParameter.Block_Nr ; dwBlockAddr++)
		{
			DWORD badpos = (NandParameter.PageSize2Exp > MCARD_SECTOR_SIZE_EXP) ? BLOCK_STATUS_N : BLOCK_STATUS_X;
			DWORD phyaddr = dwBlockAddr*NandParameter.SectorPerBlock;

			if (ReadFtlArea(phyaddr, spare1) != PASS)
			{
				mpDebugPrint("Read FTL1 failed!\r\n");
			}
			else if (ReadFtlArea(phyaddr+NandParameter.SectorPerBlock-SectPerPage, spare2) != PASS)
			{
				mpDebugPrint("Read FTLn failed!\r\n");
			}
			else if ((spare1[badpos] != 0xff) || (spare2[badpos] != 0xff))	// bad block
			{
				mpDebugPrint("Bad block!(%d)\r\n", phyaddr);
			}
			else
			{
				switch (state)
				{
					case 0:	// find and get MPAP
						NandSectorRead(phyaddr, SectPerPage, (DWORD)tmp, NULL, NULL);
						if (tmp[0] == AP_TAG)
						{
							DWORD k = (1 << NandParameter.PageSize2Exp) - 32;

							UartOutText("AP@blk");
							UartOutValue(dwBlockAddr, 3);
							UartOutText(" : L=");
							len = tmp[2];
							sectCnt = ((len + (1<<NandParameter.PageSize2Exp) - 1) >> NandParameter.PageSize2Exp) << (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);
							chksum = tmp[4];
							UartOutValue(len, 8);
							UartOutText(" C=");
							UartOutValue(chksum, 8);

							memcpy((BYTE *)AP_Code_start_addr, (BYTE *)&tmp[8], k);
							AP_Code_start_addr += k;
							NandSectorRead(phyaddr+SectPerPage, NandParameter.SectorPerBlock - SectPerPage, (DWORD)AP_Code_start_addr, NULL, NULL);
							AP_Code_start_addr += (NandParameter.SectorPerBlock - SectPerPage)<<MCARD_SECTOR_SIZE_EXP;
							if (sectCnt <= NandParameter.SectorPerBlock)
								sectCnt = 0;
							else
								sectCnt -= NandParameter.SectorPerBlock;
							state = 1;
						}
						else
						{
							MemDump(tmp, 512);
						}
						break;
					case 1:
						if (sectCnt > NandParameter.SectorPerBlock)
						{
							NandSectorRead(phyaddr, NandParameter.SectorPerBlock, (DWORD)AP_Code_start_addr, NULL, NULL);
							AP_Code_start_addr += NandParameter.SectorPerBlock<<MCARD_SECTOR_SIZE_EXP;
							sectCnt -= NandParameter.SectorPerBlock;
						}
						else
						{
							DWORD *ptr = (DWORD *)loading_addr;
							NandSectorRead(phyaddr, sectCnt, (DWORD)AP_Code_start_addr, NULL, NULL);
							len >>= 2;
							for (i = 0 ; i < len ; i++)
								chksum += ptr[i];
							if (chksum == 0)
							{
								UartOutText("\tok\r\n");
								FlushDataCache();
								UartOutText("\r\nload AP OK. Start to run AP.\r\n");
								__asm("	li		$26, 0x80000010 ");
								__asm("	jr		$26				");
								__asm("	nop						");
							}
							else
							{
								UartOutText("\tChecksum error:");
								UartOutValue(chksum, 8);
								UartOutText("\r\n");
#if 0	// dump whole AP
								for (i = 0 ; i < len ; i++)
								{
									if ((i & 0x03) == 0)
									{
										UartOutText("\r\n");
										UartOutValue((i<<2)+0x20, 8);
										UartOutText(": ");
									}
									UartOutValue(ptr[i], 8);
									PutUartChar(' ');
								}
#endif

							}
							sectCnt = 0;
							state = 0;
							dwBlockAddr = NandParameter.Block_Nr;	// end of loop
						}
						break;
					default:
						break;
				}
			}
		}
		UartOutText("\tFailed!\r\n");
	}
	McardNandInactive();

	return FAIL;
}//end of LoadBinFromNAND

#else

static MakerMap MakerMappingTab[] =
{
	{0xEC, "Samsung"},
	{0x98, "Toshiba"},
	{0x20, "ST"},
	{0xAD, "Hynix"},
	{0x2C, "Micron"},
	{0xC2, "MXIC"},
	{0x9B, "ATO"}
};

static WORD DeviceIdTable[][2] = {
//               [Id,       ChipSize(MB)]
//large block
/*64   MB*/    {0xf0,           64},
/*128  MB*/    {0xd1,          128},
/*128  MB*/    {0xf1,          128},
/*256  MB*/    {0xda,          256},
/*512  MB*/    {0xdc,          512},
/*1024 MB*/    {0xd3,         1024},
/*2048 MB*/    {0xd5,         2048},
/*4096 MB*/    {0xd7,         4096},
             {   0,            0}
};

static WORD DeviceIdTableSmall[][4] = {
//                 [Id,   TotalBlock, PagePBlock, RowAdr]
//small block
/*4    MB*/    {0xe5,      512,       16,      2},
/*8    MB*/    {0xe6,     1024,       16,      2},
/*16   MB*/    {0x73,   1*1024,       32,      2},
/*32   MB*/    {0x75,   2*1024,       32,      2},
/*64   MB*/    {0x76,   4*1024,       32,      3},
/*128  MB*/    {0x79,   8*1024,       32,      3},
/*256  MB*/    {0x71,  16*1024,       32,      3},
             {   0,        0,        0,      0}
};

static SWORD NandTraditionId(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz, DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane)
{
	DWORD i;

    if (id[2]&0x0c)   //MLC
    {
        *ecc_bit = 4;
    }
    else
    {
        *ecc_bit = 1;
    }
	*PageSz = (1<<(id[3] & 0x03)) * 1024;
	*SpareSz = (8 << ((id[3] >> 2) & 0x01)) * (*PageSz / 512);
	*BlkSz = 64 * 1024 << ((id[3] >> 4) & 0x03);	// block size
	*plane = 1 << ((id[4] >> 2) & 0x03);
	i = 0;
	*nMB = 0;
	while(DeviceIdTable[i][0])
	{
		if (DeviceIdTable[i][0] == id[1])
		{
			*nMB = DeviceIdTable[i][1];
			break;
		}
		i++;
	}
	if (*nMB == 0)
	{
		i = 0;
		while (DeviceIdTableSmall[i][0])
		{
			if (DeviceIdTableSmall[i][0] == id[1])
			{
				*ecc_bit = 1;
				*PageSz = 512;
				*SpareSz = 16;
				*BlkSz = DeviceIdTableSmall[i][2]*512;	// block size
				*nMB = DeviceIdTableSmall[i][1]*DeviceIdTableSmall[i][2]/2048;
				*plane = *nMB;
				break;
			}
			i++;
		}
	}
}

void BootPageCheck()
{
	static BOOL checked = FALSE;

	if (checked == FALSE)
	{
		DWORD chksum = 0;
		DWORD bpc_err = FAIL;
		DWORD i, j, p;
		DWORD len = sizeof(NandInfo) / sizeof(DWORD);
		DWORD bp_nr = MCARD_SECTOR_SIZE / ((len + 1) * sizeof(DWORD));
		DWORD *buf = (DWORD *)((DWORD)ext_mem_malloc(MCARD_SECTOR_SIZE) | 0xA0000000);
		DWORD *ptr = buf;

		for (p = 0 ; p < NandParameter.PagePerBlock ; p++)	// search page per page in 0th block
		{
			if (BootSectorRead(p, (DWORD)buf) == PASS)
			{
				for (i = 0 ; i < bp_nr ; i++)	// many copies in each page
				{
					chksum = 0;
					for (j = 0 ; j < len ; j++)	// checksum verifying
						chksum += ptr[j];
					if (chksum == ~ptr[len])
					{
						if (memcmp(&NandParameter, buf, len*sizeof(DWORD)))
						{
							mpDebugPrint("\tBootPage checksum's OK but content's NG");
							bpc_err = FAIL;
							break;
						}
						else
						{
							bpc_err = PASS;
							break;
						}
					}
					ptr += len + 1;
				}
			}
			if (bpc_err == PASS)
				break;
		}
		if (bpc_err != PASS)
		{	// checksum failed, recover it.
			mpDebugPrint("Boot page is wrong, try to recover...");
			memset(buf, MCARD_SECTOR_SIZE, 0);
			memcpy(buf, &NandParameter, sizeof(NandInfo));
			buf[len] = 0;
			for (i = 0 ; i < len ; i++)
				buf[len] += buf[i];
			buf[len] = ~buf[len];
			ptr = &buf[len + 1];
			for (i = 1 ; i < bp_nr ; i++)	// many copies
			{
				memcpy(ptr, buf, sizeof(NandInfo)+sizeof(DWORD));
				ptr += len + 1;
			}
			NandBlockErase(0);
			for (p = 0 ; p < NandParameter.PagePerBlock ; p++)	// copy to all pages in block
			{
				BootSectorWrite(p, (DWORD)buf);
			}
		}
		else
		{
			mpDebugPrint("Boot page check ok!");
		}
		ext_mem_free(buf);
		checked = TRUE;
	}
}

SWORD NandIdentify()
{
	static BYTE init = FALSE;
	SWORD swRetValue = PASS;
	if (init == FALSE)
	{
		BYTE bIDBuffer[6], i;
		DWORD PageNr, PageSz, BlkSz, SpareSz, EccBit, TotalSz, plane, tmp;
		DWORD know_nr = sizeof(MakerMappingTab) / sizeof(MakerMap);

		UartOutText("Nand Driver (MP650, ");
		#if 0	// for debug
		mpDebugPrint("OPTC=%x, ECC8S4S=%x, cal=%x, nand=%x", &pstMcEcc->MC_ECC_OPCT[0], &pstMcEcc->MC_ECC_ERR[0], &pstMcEcc->MC_FTL_ECCDATA_EXE, &pstMcEcc->MC_FTL_ECCDATA_RD);
		mpDebugPrint("CYCNT=%x, ALE0=%x, ALE1=%x, CMD=%x", &pstMcEcc->MC_AUTO_ALE_CYCNT, &pstMcEcc->MC_AUTO_ALE_DAT[0], &pstMcEcc->MC_AUTO_ALE_DAT[1], &pstMcEcc->MC_AUTO_CMD_DAT);
		mpDebugPrint("FTL_NUM=%x, BCHC=%x, CNFG=%x, PASSFAIL=%x", &pstMcEcc->MC_FTL_NUMBER, &pstMcEcc->MC_ECCBCH_C, &pstMcEcc->MC_ECC_CNFG, &pstMcEcc->MC_ECC_PASSFAIL);
		#endif
		// first command should be RESET command
		NandSetCEx(0);
		SEND_COMMAND(RESET_CMD);
		WaitReadyBusy();
		// read ID
		SEND_COMMAND(READ_ID_CMD);
		SEND_ADDRESS(0x00);
		for (i = 0 ; i < 6 ; i++)
			bIDBuffer[i] = READ_DATA();

		MP_DEBUG("Nand ID: %x,%x,%x,%x,%x,%x", bIDBuffer[0], bIDBuffer[1],bIDBuffer[2],bIDBuffer[3], bIDBuffer[4], bIDBuffer[5]);
		for (i = 0 ; i < know_nr ; i++)
		{
			if (MakerMappingTab[i].MakerID == bIDBuffer[0])
			{
				UartOutText(MakerMappingTab[i].MakerName);
				break;
			}
		}
		if (i >= know_nr){
			UartOutText("Unknown");
			swRetValue = FAIL;
		}
		UartOutText(", ");

		if (swRetValue == PASS)
		{
			// according to different nand type, we use different identify way
			NandTraditionId(bIDBuffer, &TotalSz, &BlkSz, &PageSz, &SpareSz, &EccBit, &plane);

			if (EccBit == 1)
			{
				mpDebugPrint("1Bit ECC)");
				ECC_Correct = ECC_1Bit_Handler;
				NandParameter.ECC_Type = ECC_1BIT;
			}
			else
			{
				mpDebugPrint("%dBit ECC)", EccBit);
				ECC_Correct = ECC_Bch_Handler;
				if (EccBit == 8)
					NandParameter.ECC_Type = ECC_8BIT;
				else if (EccBit == 12)
					NandParameter.ECC_Type = ECC_12BIT;
				else if (EccBit == 16)
					NandParameter.ECC_Type = ECC_16BIT;
				else if (EccBit == 24)
					NandParameter.ECC_Type = ECC_24BIT;
				else
					NandParameter.ECC_Type = ECC_8BIT;
			}
			NandParameter.SpareSize = SpareSz;
			NandParameter.PageSize2Exp = 0;
			tmp = PageSz >> 1;
			while (tmp)
			{
				tmp >>= 1;
				NandParameter.PageSize2Exp++;
			}
			NandParameter.PagePerBlock = BlkSz >> NandParameter.PageSize2Exp;
			NandParameter.SectorPerBlock = BlkSz >> MCARD_SECTOR_SIZE_EXP;
			NandParameter.Block_Nr = (TotalSz * 1024) / (BlkSz / 1024);
			if (PageSz == 512)
				PageNr = TotalSz * 1024 * 2;
			else
				PageNr = (TotalSz * 1024) / (PageSz / 1024);
			NandSetChipNr(NAND_CE_PIN_NR, PageNr);
			NandParameter.PlaneSizeExp = 19;
			tmp = TotalSz / plane;
			while (tmp)
			{
				tmp >>= 1;
				NandParameter.PlaneSizeExp++;
			}
			if (PageSz == MCARD_SECTOR_SIZE)
			{
				NandParameter.ColAddrNr = 1;
			}
			else
			{
				NandParameter.ColAddrNr = 0;
				PageSz--;
				while (PageSz)
				{
					NandParameter.ColAddrNr++;
					PageSz >>= 8;
				}
			}
			NandParameter.RowAddrNr = 0;
			PageNr--;
			while (PageNr)
			{
				NandParameter.RowAddrNr++;
				PageNr >>= 8;
			}
			NandParameter.EccOffset = ECC_START_ADDR;

			McardNandInactive();
			McardNandActive();	// set correct value for auto mode

			mpDebugPrint("\tPageSize=%d,SpareSize=%d,PagePerBlock=%d,BlockNum=%d,PlaneSiz=%d", (1 << NandParameter.PageSize2Exp), NandParameter.SpareSize, NandParameter.PagePerBlock, NandParameter.Block_Nr, 1<<NandParameter.PlaneSizeExp);
			MP_DEBUG("address cycle: row=%d, col=%d", NandParameter.RowAddrNr, NandParameter.ColAddrNr);
			if (GetBCHByteNr(NandParameter.ECC_Type) * ECC_PER_PAGE + FTL_LENGTH > SpareSz)
			{
				mpDebugPrint("**************************************************");
				mpDebugPrint("FATAL ERROR: spare size is not enough!(%d > %d)", GetBCHByteNr(NandParameter.ECC_Type) * ECC_PER_PAGE + FTL_LENGTH, SpareSz);
				mpDebugPrint("**************************************************");
				swRetValue = FAIL;
			}
		}
		if (swRetValue == PASS)
			init = TRUE;
	}

	return swRetValue;
}

#endif

#if NAND_ECC_TEST
void NandInfoDump()
{
	mpDebugPrint("Ecc type= %d", NandParameter.ECC_Type);
	mpDebugPrint("Ecc size= %d", NandParameter.ECC_Size);
	mpDebugPrint("Col addr= %d", NandParameter.ColAddrNr);
	mpDebugPrint("Row addr= %d", NandParameter.RowAddrNr);
	mpDebugPrint("Page size exp= %d", NandParameter.PageSize2Exp);
	mpDebugPrint("Spare size= %d", NandParameter.SpareSize);
	mpDebugPrint("Ecc offset= %d", NandParameter.EccOffset);
	mpDebugPrint("Page per blk= %d", NandParameter.PagePerBlock);
	mpDebugPrint("Sector per blk= %d", NandParameter.SectorPerBlock);
	mpDebugPrint("Block number= %d", NandParameter.Block_Nr);
	mpDebugPrint("Plane size exp= %d", NandParameter.PlaneSizeExp);
}

static NandInfo backup;
void NandConfigForTest(BYTE ecc_bit_nr)
{
	memcpy(&backup, &NandParameter, sizeof(NandInfo));
	if (ecc_bit_nr == 0)
	{
		ECC_Correct = ECC_1Bit_Handler;
	}
	else
	{
		ECC_Correct = ECC_Bch_Handler;
		if (ecc_bit_nr >= ECC_12BIT)
		{
			NandParameter.PageSize2Exp--;
			NandParameter.PlaneSizeExp--;
			NandParameter.SectorPerBlock >>= 1;
			NandParameter.SpareSize = 256;
		}
	}
	NandParameter.ECC_Type = ecc_bit_nr;
	pstMcEcc->MC_ECC_CNFG = (NandParameter.ECC_Size << 4) | NandParameter.ECC_Type;
	mpDebugPrint("ECC CONFG: 0x%.8x", pstMcEcc->MC_ECC_CNFG);
	NandInfoDump();
}

void NandConfigForNormal()
{
	memcpy(&NandParameter, &backup, sizeof(NandInfo));
	pstMcEcc->MC_ECC_CNFG = (NandParameter.ECC_Size << 4) | NandParameter.ECC_Type;
}

void FtlTest(DWORD sect)
{
	BYTE *spare = (BYTE *)pstMcEcc->MC_FTL_REG;
	DWORD bRedtData[128];
	DWORD row = sect >> (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);
	static DWORD kk = 0;

	mpDebugPrint("FTL ECC testing...");
	NandBlockErase(sect);
	// spare area address
	BuildFTLArea(pstMcEcc->MC_FTL_REG, 0x5a+kk, SFT_NORMAL);
	kk++;
	NandSetCEx(row);
	mpDebugPrint("\tWrite...");
	IODelay(2000);
	SEND_COMMAND(SEQ_DATA_IN_CMD);
	SetAddress(row, 1 << NandParameter.PageSize2Exp);
	// ECC
	pstMcard->McSmC &= ~ECC_ENABLE;
	pstMcard->McSmC |= ECC_ENABLE;
	START_MCARD_FUNC(FTL_ONLY, DIR_WRITE, 1, SW_MANUAL);
	if (WaitEccAction() != PASS)
	{
		mpDebugPrint("\twrite FTL area failed(1)");
	}
	else
	{
		SEND_COMMAND(PAGE_PROG_CMD);
		if (WaitReadyBusy())
		{
			mpDebugPrint("\t-E- Nand write R/B timeout, PageAddr=%d",row);
		}
		BYTE ret = ReadStatus();
		if (ret & 0x01)
		{
			mpDebugPrint("\t-E- Nand write, status failed, PageAddr=%d,status=%x", row, ret);
		}
		mpDebugPrint("\tread...");
		IODelay(2000);
		SEND_COMMAND(READ_PAGE1_CMD);
		SetAddress(row, 1 << NandParameter.PageSize2Exp);
		SEND_COMMAND(READ_PAGE1_CMD_2CYC);
		WaitReadyBusy();
		// ECC
		pstMcard->McSmC &= ~ECC_ENABLE;
		pstMcard->McSmC |= ECC_ENABLE;
		START_MCARD_FUNC(FTL_ONLY, DIR_READ, 1, SW_MANUAL);
		if (WaitEccAction() != PASS)
		{
			mpDebugPrint("\tread FTL area failed(1)");
			MemDump(spare, FTL_LENGTH);
		}
		else
		{
			// correction
			memcpy(bRedtData, spare, FTL_LENGTH);
			if (ECC1S_FtlCheck(bRedtData) != PASS)
			{
				mpDebugPrint("\tECC in FTL failed!");
			}
			else
			{
				BYTE *tmp = bRedtData;
				NandBlockErase(sect);
				mpDebugPrint("\tGenerate fake FTL ECC...");
				tmp[kk%FTL_LENGTH] ^= 1<<(kk&0x07);
				SEND_COMMAND(SEQ_DATA_IN_CMD);
				SetAddress(row, 1 << NandParameter.PageSize2Exp);
				tmp[FTL_LENGTH] = pstMcEcc->MC_FTL_ECCDATA_RD >> 24;
				tmp[FTL_LENGTH+1] = (pstMcEcc->MC_FTL_ECCDATA_RD >> 16) & 0xff;
				tmp[FTL_LENGTH+2] = (pstMcEcc->MC_FTL_ECCDATA_RD >> 8) & 0xff;
				DWORD i;
				for (i = 0 ; i < NandParameter.EccOffset ; i++)
					SEND_DATA(tmp[i]);
				SEND_COMMAND(PAGE_PROG_CMD);
				if (WaitReadyBusy())
				{
					mpDebugPrint("\t-E- Nand write R/B timeout, PageAddr=%d",row);
				}
				BYTE ret = ReadStatus();
				if (ret & 0x01)
				{
					mpDebugPrint("\t-E- Nand write, status failed, PageAddr=%d,status=%x", row, ret);
				}
				mpDebugPrint("\tread and correction...");
				IODelay(2000);
				SEND_COMMAND(READ_PAGE1_CMD);
				SetAddress(row, 1 << NandParameter.PageSize2Exp);
				SEND_COMMAND(READ_PAGE1_CMD_2CYC);
				WaitReadyBusy();
				// ECC
				pstMcard->McSmC &= ~ECC_ENABLE;
				pstMcard->McSmC |= ECC_ENABLE;
				START_MCARD_FUNC(FTL_ONLY, DIR_READ, 1, SW_MANUAL);
				if (WaitEccAction() != PASS)
				{
					mpDebugPrint("\tread FTL area failed(2)");
					MemDump(spare, FTL_LENGTH);
				}
				else
				{
					// correction
					memcpy(bRedtData, spare, FTL_LENGTH);
					if (ECC1S_FtlCheck(bRedtData) != PASS)
					{
						mpDebugPrint("ECC in FTL failed!");
					}
					else
					{
						mpDebugPrint("ECC in FTL success!");
					}
					MemDump(bRedtData, FTL_LENGTH);
				}
			}
			//MemDump(bRedtData, FTL_LENGTH);
		}
	}
}
MPX_KMODAPI_SET(FtlTest);
#endif

#endif
#endif

