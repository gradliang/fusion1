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
* Filename      : NandPhyAdv_Mp615.c
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
#if ((CHIP_VER_MSB == CHIP_VER_615) && (NAND_DRV == NAND_FTL_DRV))
#if (NAND_ENABLE || ISP_FUNC_ENABLE)

#include "mpTrace.h"
#include "Mcard.h"
#include "nand.h"
#include "Uti.h"
#include "taskid.h"


#undef TRIGGER_BY_INT
#define TRIGGER_BY_INT		0

#define DETECT_ENABLE		0x00000001
#define WRITE_ENABLE		0x00000002
#define ECC_ENABLE			0x00000004
#define TYPE_256_BYTE		0x00000000
#define TYPE_512_BYTE		0x00000008
#define CE_MASK_HIGH		0x00000010
#define ADR_LENGTH_3		0x00000000
#define ADR_LENGTH_4		0x00000020
#define ADR_LENGTH_5		0x00000040
#define CS_NAND				0x00000000
#define IC_SRB				0x00000001
#define IC_ECB				0x00000002
#define IC_SFTCE			0x00000004
#define IC_SMTO				0x00000008
#define IC_CDX				0x00000010
#define IM_SRB				0x00000100
#define IM_ECB				0x00000200
#define IM_SFTCE			0x00000400
#define IM_SMTO				0x00000800
#define IM_CDX				0x00001000
#if TRIGGER_BY_INT
#define IM_ALL				0x01010000
#else
#define IM_ALL				0
#endif
#define PL_HIGH_SRB			0x00010000
#define PL_HIGH_ECB			0x00020000
#define PL_HIGH_SFTCE		0x00040000
#define PL_HIGH_SMTO		0x00080000
#define PL_HIGH_CDX			0x00100000
#define MD_EDGE_SRB			0x01000000
#define MD_EDGE_ECB			0x02000000
#define MD_EDGE_SFTCE		0x04000000
#define MD_EDGE_SMTO		0x08000000
#define MD_EDGE_CDX			0x10000000
#define MCR_TRS				0
#define MCR_HLD				2
#define MCR_PLW				2
#define MCR_ST				1
#define MCW_TRS				0
#define MCW_HLD				2
#define MCW_PLW				2
#define MCW_ST				1

#define CE_BIT_OFFSET		7

#define ECC4SEN				BIT0 // 1 : enable ECC4S , 0 : disable ECC4S
#define ECC4SRN				BIT1 // 0 : reset ECC4S
#define EDN					BIT2 // 1 : encode , 0 : decode
//MCARD ECC4S State Register Bit Define

#define MC_ECC4S_STATE_STATE_MASK		0x0000000f
#define ENCODE_NORMAL_COMPLETION		0x00000000
#define DECODE_NO_ERROR					0x00000000
#define DECODE_CORRECTION_IMPOSSIBLE	0x00000001
#define DECODE_CORRECTION_COMPLETED		0x00000003
#define MC_ECC4S_STATE_SERR_MASK		0x000000f0
#define MC_ECC4S_STATE_ERRSTATE_MASK	0x00000f00
#define MC_ECC4S_STATE_READY_MASK		0x0000f000
#define MC_ECC4S_STATE_READY_READY1		0x00001000
#define MC_ECC4S_STATE_READY_READY2		0x00002000
#define MC_ECC4S_STATE_READY_READY3		0x00004000
#define MC_ECC4S_STATE_READY_READY4		0x00008000
#define MC_ECC4S_STATE_CTLSSTATE_MASK	0x00ff0000
#define MC_ECC4S_STATE_CTLSSTATE_IDLE	0
//MCARD ECC4S Error Define
#define NO_ERROR			0
#define CORRECT_COMPLETE	1
#define ENCODE_COMPLETION	2
#define ENCODE_ERROR		-1
#define DECODE_ERROR		-2
#define DECODE_CORRECT_IMPOSSIBLE	-3
#define DECODE_CHECK_TIMEOUT		-4

enum SPARE_AREA_IDX
{
	BLOCK_STATUS		= 0,	// bad block mark in Nand
	LOG_ADDR_1			= 1,
	LOG_ADDR_2			= 2,
	LOG_ADDR_3			= 3,
	LOG_ADDR_4			= 4,
	LOG_ADDR2_1			= 5,
	LOG_ADDR2_2			= 6,
	LOG_ADDR2_3			= 7,
	LOG_ADDR2_4			= 8,
	FTL_LENGTH
};

typedef enum ECC_TYPE
{
	MLC_8BIT = 0,
	MLC_4BIT = 0x01,
	SLC_1BIT = 0x02
}EccType;

typedef struct MAKER_MAP
{
	BYTE MakerID;
	BYTE MakerName[16];
}MakerMap;

static MCARD *pstMcard = (MCARD *)MCARD_BASE;
static NandInfo NandParameter = {0};
static DWORD pdwTempBuffer[128];	// 128*sizeof(DWORD)=512
static BYTE *pbTempBuffer = (BYTE *)pdwTempBuffer;
static BYTE MaxErrBitFound = 0;

static SWORD ECC1S_Correct(BYTE *pbBuffer, DWORD dwNandEcc, DWORD dw600Ecc, DWORD datalen)
{
	DWORD dwEccDiff;
	SWORD ret = PASS;

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

static SWORD Slc_Ecc_Correct(BYTE *pbBuffer,BYTE bField, DWORD dw600Ecc, BYTE *sparearea)
{
	DWORD dwNandEcc,dwEccValue;
	BYTE bCPValue,bCPCount,bLPCount,bBitAddr,bOldErrValue,bNewErrValue,bXorBitValue;
	BYTE bTempCount;
	WORD wLPValue,wByteAddr,wTemp;
	
	if(bField == 1)
		dwNandEcc = ( (sparearea[5] << 16) | (sparearea[4] << 8) |sparearea[3] );	
	else if(bField== 0)
		dwNandEcc = ( (sparearea[2] << 16) | (sparearea[1] << 8) |sparearea[0] );
	SWORD ret = ECC1S_Correct(pbBuffer, dwNandEcc, dw600Ecc, 256);

	if (ret)
	{
		MP_DEBUG("%d. %x, %x", bField, dwNandEcc, dw600Ecc);
	}

	return ret;
}
static SWORD ECC_Check(BYTE *dwDataAddr,BYTE* sparearea)
{
	SWORD ret = 0;
	BYTE bECCCode[3];

	bECCCode[2] = ((pstMcard->McSmEcc1 >> 16) & 0xff);
	bECCCode[1] = ((pstMcard->McSmEcc1 >> 8) & 0xff);
	bECCCode[0] = (pstMcard->McSmEcc1  & 0xff);

	if((sparearea[0] != bECCCode[0]) ||(sparearea[1] != bECCCode[1]) ||(sparearea[2] != bECCCode[2]))
	{
		if (Slc_Ecc_Correct(dwDataAddr, 0, (bECCCode[2] << 16) | (bECCCode[1] << 8) | bECCCode[0], sparearea) != PASS)
			ret = 1;
	}
	bECCCode[2] = ((pstMcard->McSmEcc2 >> 16) & 0xff);
	bECCCode[1] = ((pstMcard->McSmEcc2 >> 8) & 0xff);
	bECCCode[0] = (pstMcard->McSmEcc2  & 0xff);	

	if((sparearea[3] != bECCCode[0]) ||(sparearea[4] != bECCCode[1]) ||(sparearea[5] != bECCCode[2]))
	{
		if (Slc_Ecc_Correct((dwDataAddr+256), 1, (bECCCode[2] << 16) | (bECCCode[1] << 8) | bECCCode[0], sparearea) != PASS)
			ret |= 2;
	}
	return ret;
}
static ECC_Get(BYTE *ptr)
{
	if (NandParameter.ECC_Type == SLC_1BIT)
	{
		ptr[2] = ((pstMcard->McSmEcc1 >> 16) & 0xff);
		ptr[1] = ((pstMcard->McSmEcc1 >> 8) & 0xff);
		ptr[0] = (pstMcard->McSmEcc1  & 0xff);
		ptr[5] = ((pstMcard->McSmEcc2 >> 16) & 0xff);
		ptr[4] = ((pstMcard->McSmEcc2 >> 8) & 0xff);
		ptr[3] = (pstMcard->McSmEcc2  & 0xff);	
	}
	else if (NandParameter.ECC_Type == MLC_4BIT)
	{
		ptr[6] = (BYTE) ((pstMcard->McEcc4SCrc41>>8) & 0xff);
		ptr[7] = (BYTE) (pstMcard->McEcc4SCrc41 & 0xff);
		ptr[8] = (BYTE) ((pstMcard->McEcc4SCrc10>>24) & 0xff);
		ptr[9] = (BYTE) ((pstMcard->McEcc4SCrc10>>16) & 0xff);
		ptr[3] = (BYTE) (pstMcard->McEcc4SCrc74 & 0xff);
		ptr[4] = (BYTE) ((pstMcard->McEcc4SCrc41>>24) & 0xff);
		ptr[5] = (BYTE) ((pstMcard->McEcc4SCrc41>>16) & 0xff);
		ptr[0] = (BYTE) ((pstMcard->McEcc4SCrc74>>24) & 0xff);
		ptr[1] = (BYTE) ((pstMcard->McEcc4SCrc74>>16) & 0xff);
		ptr[2] = (BYTE) ((pstMcard->McEcc4SCrc74>>8) & 0xff);	
	}
}

static void ECC4S_Encode(void)
{
	pstMcard->McEcc4SC = 0;
	pstMcard->McEcc4SC = (EDN |ECC4SRN |ECC4SEN) ;
}
static void ECC4S_Decode(void)
{
	pstMcard->McEcc4SC = 0;
	pstMcard->McEcc4SC = ECC4SEN ;
	pstMcard->McEcc4SC |= ECC4SRN;
	pstMcard->McEcc4SC |= 1 << 11;
}
static void ECC4S_Decode_Fill(BYTE *eccBuf)
{
    pstMcard->McEcc4SSA0[1] &= 0xffff0000;
    pstMcard->McEcc4SSA0[1] = ((eccBuf[0]<<8) | eccBuf[1]);
    pstMcard->McEcc4SSA0[2] = ((eccBuf[2]<<24) | (eccBuf[3]<<16) | (eccBuf[4]<<8) | eccBuf[5]);
    pstMcard->McEcc4SSA0[3] = ((eccBuf[6]<<24) | (eccBuf[7]<<16) | (eccBuf[8]<<8) | eccBuf[9]);
}
static void ECC4S_Correct(BYTE *pdwBufferAddress)
{
	BYTE bErrCount,i,bVal;
	BYTE * pbBuffer;
	WORD wAddr;
	BYTE bOld;
	pbBuffer = pdwBufferAddress;
	bErrCount = (BYTE)((pstMcard->McEcc4SState & MC_ECC4S_STATE_SERR_MASK)>>4);
	if (bErrCount > MaxErrBitFound)
		MaxErrBitFound = bErrCount;
	MP_DEBUG1("ECC4S Correct count = %d",bErrCount);
	for(i=0;i<bErrCount;i++)
	{
		wAddr = 521 - (WORD)(pstMcard->McEcc4SErr[i] & 0x000003ff);
		if(wAddr < 512)
		{
			bOld = pbBuffer[wAddr]; 
			bVal =(BYTE) ((pstMcard->McEcc4SErr[i] & 0x03ff0000)>>16);
			pbBuffer[wAddr] ^= (BYTE)bVal;	
			MP_DEBUG2("0x%x -> 0x%x",bOld,pbBuffer[wAddr]);
		}
	}
	
}
static DWORD ECC4S_Codec_Wait()
{
    DWORD timeout = 0xffff;
    BYTE sts;

    while(timeout)
    {
        timeout--;
        sts = (pstMcard->McEcc4SState & 0xff0000) >> 16;
        if(!sts)
            break;
        else
        {
            if(sts == 0x4 && !(pstMcard->McEcc4SC & 0x400))   //auto mode disable
                break;
        }
    }
    if(!timeout)
    {
        mpDebugPrint("ECC4S timeout1: %x", pstMcard->McEcc4SState);
        return FAIL;
    }

    timeout = 0xffff;
    while(timeout)
    {
        timeout--;
        sts = pstMcard->McEcc4SState & 0xf;
        if(sts == 0 || sts == 1 || sts == 3)
            break;
    }
    if(!timeout)
    {
        mpDebugPrint("ECC4S timeout2: %x", pstMcard->McEcc4SState);
        return FAIL;
    }


    sts = pstMcard->McEcc4SState & 0xf;
    if(!sts || sts == 0x3)  //no err or correction complete
        return PASS;
    else
    {
        if((pstMcard->McEcc4SSA0[1] & 0xffff) != 0xffff || pstMcard->McEcc4SSA0[2] != 0xffffffff ||
            pstMcard->McEcc4SSA0[3] != 0xffffffff)   //when the page is not even writed
        {
            mpDebugPrint("ECC4S state error: %x", pstMcard->McEcc4SState);
            return FAIL;
        }
        else
        {
            //DPrintf("X");
        }
    }

    return PASS;
}
static SWORD ECC4S_Decode_Check(BYTE *pdwBufferAddress)
{
	SWORD dwRetValue;
	
	if (ECC4S_Codec_Wait() == PASS)
	{
		if((pstMcard->McEcc4SState & MC_ECC4S_STATE_STATE_MASK) == DECODE_NO_ERROR)
		{
			dwRetValue = CORRECT_COMPLETE;
		}
		else if((pstMcard->McEcc4SState & MC_ECC4S_STATE_STATE_MASK) == DECODE_CORRECTION_COMPLETED)
		{		
			ECC4S_Correct(pdwBufferAddress);
//			MP_DEBUG("ECC4S_Decode_Check correction Complete");
			dwRetValue = CORRECT_COMPLETE;
		}
		else if((pstMcard->McEcc4SState & MC_ECC4S_STATE_STATE_MASK) == DECODE_CORRECTION_IMPOSSIBLE)
		{
	        if((pstMcard->McEcc4SSA0[1] & 0xffff) != 0xffff || pstMcard->McEcc4SSA0[2] != 0xffffffff ||
    	        pstMcard->McEcc4SSA0[3] != 0xffffffff)   //when the page is not even writed
	        {
				MP_DEBUG("ECC4S_Decode_Check correction impossible");
				dwRetValue = DECODE_CORRECT_IMPOSSIBLE;
	        }
			else
				dwRetValue = CORRECT_COMPLETE;
		}
		else
		{
			MP_DEBUG("ECC4S_Decode_Check unknown error(%x)", pstMcard->McEcc4SState);
			dwRetValue = DECODE_ERROR;
		}
	}
	else
	{
		MP_DEBUG("ECC4 R->%x,%x,%x", pstMcard->McEcc4SSA0[1], pstMcard->McEcc4SSA0[2], pstMcard->McEcc4SSA0[3]);
		dwRetValue = DECODE_CHECK_TIMEOUT;
	}
//	MP_DEBUG("ECC4S_Decode_Check no error");
	return dwRetValue;
}

static SWORD WaitReadyBusy()
{
#if TRIGGER_BY_INT
	DWORD event = 0;

	pstMcard->McSmIc |= IM_SRB;
	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &event, 3000) != PASS)
		mpDebugPrint("WaitReadyBusy timeout - %x, %x", pstMcard->McSmIc, event);
	pstMcard->McSmIc &= ~IM_SRB;

	return (event == IC_SRB) ? PASS : FAIL;
#else
	SWORD ret = PASS;
	DWORD dwTimeCount = 2400000;

	while (pstMcard->McSmIc & IC_SRB)
	{
		if (--dwTimeCount == 0)
		{
			ret = TIMEOUT;
			mpDebugPrint("Nand R/B timeout!");
			break;
		}
	}

	return ret;
#endif
}

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

static void SetAddress(DWORD Row, DWORD Col)
{
	DWORD i;
	
	//mpDebugPrint("Addr=%d,%d", Row, Col);
	pstMcard->McSmC &= ~(3 << 5);
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

static void SetRowAddress(DWORD Row)	// for block erase
{
	DWORD i;
	
	//mpDebugPrint("Addr=%d,%d", Row, Col);
	NandSetCEx(Row);
	pstMcard->McSmC &= ~(3 << 5);
	if (NandParameter.RowAddrNr > 3)
		pstMcard->McSmC |= (NandParameter.RowAddrNr - 3) << 5;
	for (i = 0 ; i < NandParameter.RowAddrNr ; i++)
	{
		SEND_ADDRESS(Row);
		Row >>= 8;
	}
}

static void SetColAddress(DWORD col)
{
	DWORD i;
	
	//mpDebugPrint("Addr=%d", col);
	pstMcard->McSmC &= ~(3 << 5);
	if (NandParameter.ColAddrNr > 3)
		pstMcard->McSmC |= (NandParameter.ColAddrNr - 3) << 5;
	for (i = 0 ; i < NandParameter.ColAddrNr ; i++)
	{
		SEND_ADDRESS(col);
		col >>= 8;
	}
}

static BOOL CheckSatusFlag(BYTE sflag)
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

static BOOL CheckEvenParity(DWORD dat)
{
	DWORD i, even = 0;

	for (i = 0 ; i < 32 ; i++)
	{
		if (dat & 0x01)
			even++;
		dat >>= 1;
	}

	return !(even&0x01);
}

static DWORD ReadLogAddr(BYTE *redundent)
{
	DWORD logaddr = (redundent[LOG_ADDR_1] << 24) + (redundent[LOG_ADDR_2] << 16)
					+ (redundent[LOG_ADDR_3] << 8) + redundent[LOG_ADDR_4];
	DWORD logaddr2 = (redundent[LOG_ADDR2_1] << 24) + (redundent[LOG_ADDR2_2] << 16)
					+ (redundent[LOG_ADDR2_3] << 8) + redundent[LOG_ADDR2_4];
	DWORD ret = LB_UNRECOGNIZED;

	if (logaddr == logaddr2)
		ret = (logaddr != LB_NO_ADDR) ? (logaddr >> 1) : LB_NO_ADDR;
	else if ((logaddr != LB_NO_ADDR) && CheckEvenParity(logaddr))
		ret = logaddr >> 1;
	else if ((logaddr2 != LB_NO_ADDR) && CheckEvenParity(logaddr2))
		ret = logaddr2 >> 1;
	
	return ret;
}

static SWORD ReadSpareArea(DWORD PageAddr, BYTE *bRedtData)
{
	SWORD swRetValue = PASS;
	DWORD timeout = 0x0001ffff;
	DWORD i;
	
	pstMcard->McSmIc = 0;
	NandSetCEx(PageAddr);
	SEND_COMMAND(READ_PAGE1_CMD);
	SetAddress(PageAddr, 1 << NandParameter.PageSize2Exp);
	SEND_COMMAND(READ_PAGE1_CMD_2CYC);
	WaitReadyBusy();

	for (i = 0 ; i < NandParameter.SpareSize ; i++)
	{
		bRedtData[i] = READ_DATA();
	}

	return swRetValue;

}

static SWORD ReadFTLArea(DWORD PageAddr, BYTE *bRedtData)
{
	SWORD swRetValue = PASS;
	DWORD timeout = 0x0001ffff;
	DWORD i;
	
	pstMcard->McSmIc = 0;
	NandSetCEx(PageAddr);
	SEND_COMMAND(READ_PAGE1_CMD);
	SetAddress(PageAddr, 1 << NandParameter.PageSize2Exp);
	SEND_COMMAND(READ_PAGE1_CMD_2CYC);
	WaitReadyBusy();

	for (i = 0 ; i < FTL_LENGTH ; i++)
	{
		bRedtData[i] = READ_DATA();
	}

	return swRetValue;

}

DWORD GetNandLogAddr(DWORD PhyAddr)
{
	DWORD swRetValue;
	BYTE retry = 16;

	do
	{
		if (ReadFTLArea(PhyAddr, pbTempBuffer) != PASS)
			swRetValue = LB_READ_FAIL;
		else if (CheckSatusFlag(pbTempBuffer[BLOCK_STATUS]) >= 2)	// bad block: more than 2 bits zero
			swRetValue = LB_BAD_BLOCK;
		else
			swRetValue = ReadLogAddr(pbTempBuffer);
	} while ((swRetValue == LB_UNRECOGNIZED) && retry--);

	return swRetValue;
}

SWORD CheckSpareContent(DWORD PhyAddr)
{
	DWORD sz = NandParameter.SpareSize;
	SWORD Has_Content = FALSE;

	if (ReadSpareArea(PhyAddr, pbTempBuffer) != PASS)
	{
		Has_Content = TRUE;
	}
	else
	{
		while (sz--)
		{
			if (pbTempBuffer[sz] != 0xff)
			{
				Has_Content = TRUE;
				break;
			}
		}
	}
	
	#if 0
	if (Has_Content)
		MemDump(spare, NandParameter.SpareSize);
	#endif
	return Has_Content;
}

static SWORD ReadSector(DWORD dwBufferAddress, WORD wSize)
{
	CHANNEL *sChannel = (CHANNEL *) (DMA_MC_BASE);
	DWORD dwTimeOutCount;
	DWORD ICmask = IC_SFTCE;

	pstMcard->McardC &= ~MCARD_FIFO_ENABLE;
	if (dwBufferAddress & 0x3)
	{
		mpDebugPrint("-E- target buffer must align to 4 bytes boundary !");
		return FAIL;
	}
	pstMcard->McSmC &= ~ECC_ENABLE;
	if (NandParameter.ECC_Type == SLC_1BIT)
	{
		pstMcard->McSmC |= ECC_ENABLE;
		ICmask |= IC_ECB;
	}
	else if (NandParameter.ECC_Type == MLC_4BIT)
	{
		ECC4S_Decode();
	}
	else
	{
		mpDebugPrint("unsupported ECC type %d", NandParameter.ECC_Type);
	}

	sChannel->Control = 0x0;
	sChannel->StartA = dwBufferAddress;
	sChannel->EndA = dwBufferAddress + wSize - 1;
	pstMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE + (wSize >> 2);
	pstMcard->McardC = ((pstMcard->McardC & 0xffffffef) | MCARD_DMA_DIR_CM | MCARD_FIFO_ENABLE);
	sChannel->Control = MCARD_DMA_ENABLE;

#if TRIGGER_BY_INT
	DWORD event = 0;

	pstMcard->McSmIc |= ICmask << 8;
	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &event, 5000) != PASS)
	{
		mpDebugPrint("ReadSector timeout - %x, %x", pstMcard->McSmIc, pstMcard->McardC);
		return FAIL;
	}

	//if ((event & ICmask) != ICmask)
	//	return FAIL;
#else
	dwTimeOutCount = 0x7fffff;
	while((pstMcard->McSmIc & ICmask) != ICmask)      // Mcard FIFO transfer ok
	{
		if(dwTimeOutCount-- == 0)
		{
			mpDebugPrint("-E- Mcard FIFO read timeout %x\n", pstMcard->McSmIc);
			return FAIL;
		}

	}
#endif
	dwTimeOutCount = 0x0007ffff;
	while (sChannel->Control & MCARD_DMA_ENABLE)
	{
		if (dwTimeOutCount-- == 0)
		{
			mpDebugPrint("-E- DMA read end FAIL (status: %x)", sChannel->Control);
			return FAIL;
		}
	}	// DMA transfer ok
	
	return PASS;
}

// dwPhyAddr means unit of page
DWORD NandPageRead(DWORD dwPhyAddr, DWORD dwPageCount, DWORD Buffer, DWORD *progress)
{
	DWORD RetValue = 0;
	BYTE nr = 1 << (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);
	BYTE sz = (NandParameter.ECC_Type == SLC_1BIT) ? 6 : 10;
	DWORD i;
	DWORD retry = 500;

	MP_DEBUG("\t%s: phyAddr=%d,PageNrs=%d,buf=%x", __FUNCTION__, dwPhyAddr, dwPageCount, Buffer);

	for (i = 0 ; i < dwPageCount ; i++)
	{
		ReadSpareArea(dwPhyAddr, pbTempBuffer);
		SEND_COMMAND(READ_IN_PAGE_S_CMD);
		SetColAddress(0);
		SEND_COMMAND(READ_IN_PAGE_E_CMD);

		DWORD j;
		BYTE *ptr = pbTempBuffer+FTL_LENGTH;
		SWORD swRetValue = PASS;

		for (j = 0 ; j < nr ; j++)
		{

			if (NandParameter.ECC_Type == MLC_4BIT)
				ECC4S_Decode_Fill(ptr);

			if (ReadSector((DWORD) Buffer, MCARD_SECTOR_SIZE))
			{
				mpDebugPrint("-E- Nand read fail, PageAddr=%d,sector=%x",dwPhyAddr,j);
				break;
			}

			if (NandParameter.ECC_Type == SLC_1BIT)
			{
				if (ECC_Check((BYTE *)Buffer, ptr))
				{
					swRetValue = FAIL;
				}
			}
			else if (NandParameter.ECC_Type == MLC_4BIT)
			{
				if(ECC4S_Decode_Check((BYTE *)Buffer) != CORRECT_COMPLETE)
				{
					swRetValue = FAIL;
				}
			}
			ptr += sz;
			Buffer += 512;
			if (progress)
				(*progress)++;
		}
		if (swRetValue == PASS)
		{
			dwPhyAddr++;
			RetValue++;
			retry = 16;
		}
		else if (retry-- > 0)	// retry
		{
			i--;
			Buffer -= 1 << NandParameter.PageSize2Exp;
		}
		else	// give up
		{
			mpDebugPrint("ECC error, PageAddr=%d",dwPhyAddr);
			dwPhyAddr++;
			retry = 16;
		}
	}

	return RetValue;
}

static BOOL CalEvenParity(DWORD dat)
{
	DWORD i, even = 0;

	for (i = 0 ; i < 32 ; i++)
	{
		if (dat & 0x01)
			even++;
		dat >>= 1;
	}

	return (even&0x01);
}

static void WriteSpareArea(BYTE *bRedtData, DWORD LogAddr)
{
	DWORD i;
	BOOL parity = CalEvenParity(LogAddr);

	LogAddr = (LogAddr << 1) | parity;

	bRedtData[BLOCK_STATUS] = 0xff;
	bRedtData[LOG_ADDR_1] = bRedtData[LOG_ADDR2_1] = (BYTE) ((LogAddr>>24)&0xff);
	bRedtData[LOG_ADDR_2] = bRedtData[LOG_ADDR2_2] = (BYTE) ((LogAddr>>16)&0xff);
	bRedtData[LOG_ADDR_3] = bRedtData[LOG_ADDR2_3] = (BYTE) ((LogAddr>>8)&0xff);
	bRedtData[LOG_ADDR_4] = bRedtData[LOG_ADDR2_4] = (BYTE) (LogAddr&0xff);

	for (i = 0 ; i < NandParameter.SpareSize ; i++)
		SEND_DATA(bRedtData[i]);
}

static SWORD WriteSector(DWORD dwBufferAddress, DWORD wSize)
{
	CHANNEL *sChannel = (CHANNEL *)DMA_MC_BASE;
	DWORD dwTimeOutCount;
	DWORD ICmask = IC_SFTCE;
	
	pstMcard->McardC &= ~MCARD_FIFO_ENABLE;
	pstMcard->McSmC &= ~ECC_ENABLE;
	if (dwBufferAddress & 0x3)
	{
		mpDebugPrint("-E- target buffer must align to 4 bytes boundary !");
		return FAIL;
	}
	if (NandParameter.ECC_Type == MLC_4BIT)
	{
		ECC4S_Encode();
	}
	else
	{
		pstMcard->McSmC |= ECC_ENABLE;
		ICmask |= IC_ECB;
	}

	sChannel->Control = 0x0;
	sChannel->StartA = dwBufferAddress;
	sChannel->EndA = dwBufferAddress + wSize - 1;
	pstMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE + (wSize >> 2);
	pstMcard->McardC = ((pstMcard->McardC & 0xffffffef) | MCARD_DMA_DIR_MC | MCARD_FIFO_ENABLE);
	sChannel->Control = MCARD_DMA_ENABLE;

	dwTimeOutCount = 0x007fffff;
	while (((CHANNEL *)DMA_MC_BASE)->Control & MCARD_DMA_ENABLE)
	{
		if (dwTimeOutCount-- == 0)
		{
			mpDebugPrint("-E- DMA write end FAIL (status: %x)", ((CHANNEL *)DMA_MC_BASE)->Control);
			return FAIL;
		}
	}	// DMA transfer ok

#if TRIGGER_BY_INT
	DWORD event = 0;

	pstMcard->McSmIc |= ICmask << 8;
	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &event, 5000) != PASS)
	{
		mpDebugPrint("WriteSector timeout - %x", pstMcard->McSmIc);
		return FAIL;
	}

	//if ((event & ICmask) != ICmask)
	//	return FAIL;
#else
    dwTimeOutCount = 0x000fffff;
    while((pstMcard->McSmIc & ICmask) != ICmask)      // Mcard FIFO transfer ok
    {
        if(dwTimeOutCount-- == 0)
        {
            mpDebugPrint("-E- Mcard FIFO write timeout %x\n", pstMcard->McSmIc);
            return FAIL;
        }
    }
#endif

	return PASS;

}

// dwPhyAddr means unit of page
SWORD NandPageWrite(DWORD dwPhyAddr, DWORD PageCnt, DWORD Buffer, DWORD LogAddr, DWORD *progress)
{
	SWORD swRetValue = PASS;
	DWORD i;
	DWORD nr = 1 << (NandParameter.PageSize2Exp - MCARD_SECTOR_SIZE_EXP);
	BYTE sz = (NandParameter.ECC_Type == SLC_1BIT) ? 6 : 10;

	MP_DEBUG("\t%s: phyAddr=%d,logAddr=%d,buf=%x", __FUNCTION__, dwPhyAddr, LogAddr, Buffer);
	for (i = 0 ; i < PageCnt ; i++)
	{
		NandSetCEx(dwPhyAddr);
		SEND_COMMAND(SEQ_DATA_IN_CMD);
		SetAddress(dwPhyAddr, 0);

		DWORD j;
		BYTE *ptr = pbTempBuffer+FTL_LENGTH;

		for (j = 0 ; j < nr ; j++)
		{
			if ((swRetValue = WriteSector(Buffer, MCARD_SECTOR_SIZE)))
				break;

			if (NandParameter.ECC_Type == MLC_4BIT)
			{
				if (ECC4S_Codec_Wait() != PASS)
					mpDebugPrint("-E- Nand write ECC timeout, PageAddr=%d,sector=%d",dwPhyAddr,i);
			}
			ECC_Get(ptr);
			ptr += sz;
			Buffer += 512;
			if (progress)
				(*progress)++;
		}
		// one page done...write spare area and start programming...
		WriteSpareArea(pbTempBuffer, LogAddr);
		SEND_COMMAND(PAGE_PROG_CMD);
		if ((swRetValue = WaitReadyBusy()))
		{
			mpDebugPrint("NandBlockWrite : Nand write R/B timeout, PageAddr=%d,sector=%d",dwPhyAddr,i);
			break;
		}
		BYTE ret = ReadStatus();
		if (ret & 0x01)
		{
			mpDebugPrint("-E- Nand write, status failed, PageAddr=%d,sector=%d,status=%x", dwPhyAddr, i, ret);
			swRetValue = FAIL;
			break;
		}
		LogAddr++;
		dwPhyAddr++;
	}
	
	return swRetValue;
}

// dstAddr, srcAddr are unit of page
SWORD NandPageCopy(DWORD dstAddr, DWORD srcAddr, DWORD logAddr, BYTE *tmpBuf)
{
	SWORD ret = PASS;

	NandPageRead(srcAddr, 1, (DWORD)tmpBuf, NULL);
	if (logAddr == -1)	// copy log addr field directly
	{
		logAddr = ReadLogAddr(pbTempBuffer);
		if (logAddr == -1)
			GetNandLogAddr(srcAddr);
	}
	if (NandPageWrite(dstAddr, 1, (DWORD)tmpBuf, logAddr, NULL) != PASS)
	{
		mpDebugPrint("\tNandPageCopy: programming failed!");
		ret = FAIL;
	}

	return ret;
}

// dwPhyAddr is unit of page
SWORD badBlockMark(DWORD dwPhyAddr)
{
	DWORD i;
	SWORD swRetValue = PASS;
	
	NandSetCEx(dwPhyAddr);
	SEND_COMMAND(SEQ_DATA_IN_CMD);
	SetAddress(dwPhyAddr, 1<<NandParameter.PageSize2Exp);

	for (i = 0 ; i < NandParameter.SpareSize ; i++)
	{
		SEND_DATA(0);
	}
	SEND_COMMAND(PAGE_PROG_CMD);
	if ((swRetValue = WaitReadyBusy()))
	{
		mpDebugPrint("badBlockMark : Nand write R/B timeout, PageAddr=%d", dwPhyAddr);
	}
	BYTE ret = ReadStatus();
	if (ret & 0x01)
	{
		mpDebugPrint("-E- badBlockMark write, status failed, PageAddr=%d,status=%x", dwPhyAddr, ret);
		swRetValue = FAIL;
	}

	return swRetValue;
}

// dwPhyAddr is unit of page
SWORD NandBlockErase(DWORD dwPhyAddr)
{
	SWORD swRetValue = PASS;

	MP_DEBUG("\t%s: phyAddr=%d", __FUNCTION__, dwPhyAddr);
	SEND_COMMAND(BLOCK_ERASE_CMD_1CYC);
	SetRowAddress(dwPhyAddr);
	SEND_COMMAND(BLOCK_ERASE_CMD_2CYC);
	if ((swRetValue = WaitReadyBusy()))
	{
		mpDebugPrint("NandBlockErase : Nand erase R/B timeout, addr=%d", dwPhyAddr);
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

// how many bits error could be damaged before uncorrectable?
BYTE GetNandAvailEccBit()
{
	const static BYTE eccbits[] = {1, 8, 12, 16, 24};
	BYTE ret = eccbits[NandParameter.ECC_Type] - MaxErrBitFound;
	MaxErrBitFound = 0;
	return ret;
}

void GetNandGeometry(DWORD *BlkNr, DWORD *PagePerBlk, DWORD *PageSz)
{
	if (BlkNr)
		*BlkNr = NandParameter.Block_Nr;
	if (PageSz)
		*PageSz = 1 << NandParameter.PageSize2Exp;
	if (PagePerBlk)
		*PagePerBlk = NandParameter.PagePerBlock;
}

void McardNandActive()
{
	DWORD i;
	GPIO *sGpio = (GPIO *)GPIO_BASE;

	//*(DWORD *)0xa8021330 = 0;
	pstMcard->McEcc4SC = 0;
	pstMcard->McardC = 0;
	pstMcard->McardC = (MCARD_FLASH_ENABLE | MCARD_DMA_SM);
	pstMcard->McRtm = (MCR_TRS<<24) | (MCR_HLD<<16) | (MCR_PLW<<8) | MCR_ST;
	pstMcard->McWtm = (MCW_TRS<<24) | (MCW_HLD<<16) | (MCW_PLW<<8) | MCW_ST;
	pstMcard->McSmIc = IM_ALL;
	pstMcard->McSmC = CS_NAND | TYPE_512_BYTE | WRITE_ENABLE;
	McardSelect(PIN_DEFINE_FOR_NAND);

#ifdef	NAND_MULTI_CE_SUPPOUR
		//Setting multi CE GPIO
#endif
}

void McardNandInactive()
{
	DWORD i;
	GPIO *sGpio = (GPIO *)GPIO_BASE;

	McardDeselect(PIN_DEFINE_FOR_NAND);
	pstMcard->McSmC = 0;
	pstMcard->McardC = 0;
	pstMcard->McSmIc = 0;
}

static MakerMap MakerMappingTab[] =
{
	{0xEC, "Samsung"},
	{0x98, "Toshiba"},
	{0x20, "ST"},
	{0xAD, "Hynix"},
	{0x2C, "Micron"}
};

static WORD DeviceIdTable[][2] = {
	//[Id,   ChipSize(MB)]
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
	while(DeviceIdTable[i][0])
	{
		if (DeviceIdTable[i][0] == id[1])
		{
			*nMB = DeviceIdTable[i][1];
			break;
		}
		i++;
	}
}

static SWORD NandSamsungNewId(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz, DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane)
{
	DWORD i;
	DWORD redt = ((id[3] & 0x40) >> 4) | ((id[3] & 0x0C) >> 2);
	
	*ecc_bit = 1 << ((id[4] & 0x70) >> 4);
	*PageSz = 2048 << (id[3] & 0x03);	// page size
	*BlkSz = 128 * 1024 << (((id[3] & 0x80) >> 5) | ((id[3] & 0x30) >> 4));	// block size
	*plane = 1 << ((id[4] >> 2) & 0x03);
	if (redt == 1)
		*SpareSz = 128;
	else if (redt == 2)
		*SpareSz = 218;
	else
	{
		mpDebugPrint("spare area size is unknown...");
		*SpareSz = (16 * (*PageSz / 512));
	}
	i = 0;
	while(DeviceIdTable[i][0])
	{
		if (DeviceIdTable[i][0] == id[1])
		{
			*nMB = DeviceIdTable[i][1];
			break;
		}
		i++;
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
		DWORD *buf = (DWORD *)((DWORD)ext_mem_malloc(NandParameter.PagePerBlock << NandParameter.PageSize2Exp) | 0xA0000000);

		MP_DEBUG(__FUNCTION__);
		if (NandPageRead(0, NandParameter.PagePerBlock, (DWORD)buf, NULL) > 0)
		{
			for (p = 0 ; p < NandParameter.PagePerBlock ; p++)	// search page per page in 0th block
			{
				DWORD pos = p << (NandParameter.PageSize2Exp - 2);
				for (i = 0 ; i < 3 ; i++)	// 3 copies in each page
				{
					chksum = 0;
					for (j = 0 ; j < len ; j++)	// checksum verifying
						chksum += buf[pos+j];
					if ((chksum == buf[pos+len]) && !memcmp(&NandParameter, &buf[pos], len*sizeof(DWORD)))
					{
						bpc_err = PASS;
						break;
					}
				}
				if (bpc_err == PASS)
					break;
			}
		}
		if (bpc_err != PASS)
		{	// checksum failed, recover it.
			DWORD *ptr;

			mpDebugPrint("Boot page is wrong, try to recover...");
			memcpy(buf, &NandParameter, sizeof(NandInfo));
			buf[len] = 0;
			for (i = 0 ; i < len ; i++)
				buf[len] += buf[i];
			ptr = &buf[len + 1];
			for (i = 0 ; i < 3 ; i++)	// 3 copies
			{
				memcpy(ptr, buf, sizeof(NandInfo)+sizeof(DWORD));
				ptr += len + 1;
			}
			for (p = 1 ; p < NandParameter.PagePerBlock ; p++)	// copy to all pages in 0th block
			{
				DWORD pos = p << (NandParameter.PageSize2Exp - 2);
				memcpy(&buf[pos], buf, sizeof(NandInfo)+sizeof(DWORD));
			}
			NandBlockErase(0);
			NandPageWrite(0, NandParameter.PagePerBlock, (DWORD)buf, 0xffffffff, NULL);
		}
		else
		{
			mpDebugPrint("Boot page check ok!");
		}
		ext_mem_free(buf);
		checked = TRUE;
	}
}

void NandInfoDump()
{
	mpDebugPrint("Ecc type= %d", NandParameter.ECC_Type);
	mpDebugPrint("Col addr= %d", NandParameter.ColAddrNr);
	mpDebugPrint("Row addr= %d", NandParameter.RowAddrNr);
	mpDebugPrint("Page size exp= %d", NandParameter.PageSize2Exp);
	mpDebugPrint("Spare size= %d", NandParameter.SpareSize);
	mpDebugPrint("Page per blk= %d", NandParameter.PagePerBlock);
	mpDebugPrint("Block number= %d", NandParameter.Block_Nr);
	mpDebugPrint("Plane size exp= %d", NandParameter.PlaneSizeExp);
}

SWORD NandIdentify()
{
	SWORD swRetValue = PASS;
	BYTE bIDBuffer[6],i;
	DWORD PageNr, PageSz, BlkSz, SpareSz, EccBit, TotalSz, plane, tmp;
	DWORD know_nr = sizeof(MakerMappingTab) / sizeof(MakerMap);

	UartOutText("Nand Driver (MP615, ");
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
		#if (NAND_ID_TYPE == SAMSUNG_NEW)
		NandSamsungNewId(bIDBuffer, &TotalSz, &BlkSz, &PageSz, &SpareSz, &EccBit, &plane);
		#else
		NandTraditionId(bIDBuffer, &TotalSz, &BlkSz, &PageSz, &SpareSz, &EccBit, &plane);
		#endif
		if (EccBit == 1)
		{
	        mpDebugPrint("1Bit ECC)");
			NandParameter.ECC_Type = SLC_1BIT;
		}
		else if (EccBit == 4)
		{
	        mpDebugPrint("4Bit ECC)");
			NandParameter.ECC_Type = MLC_4BIT;
		}
		else if (EccBit == 8)
		{
	        mpDebugPrint("8Bit ECC)");
			NandParameter.ECC_Type = MLC_8BIT;
		}
		else
		{
			mpDebugPrint("ECC type(%d) unknown, force to use 4 bit!)", EccBit);
			NandParameter.ECC_Type = MLC_4BIT;
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
		NandParameter.Block_Nr = (TotalSz * 1024) / (BlkSz / 1024);
		PageNr = (TotalSz * 1024) / (PageSz / 1024);
		NandSetChipNr(NAND_CE_PIN_NR, PageNr);
		NandParameter.PlaneSizeExp = 19;
		tmp = TotalSz / plane;
		while (tmp)
		{
			tmp >>= 1;
			NandParameter.PlaneSizeExp++;
		}
		NandParameter.ColAddrNr = 0;
		PageSz--;
		while (PageSz)
		{
			NandParameter.ColAddrNr++;
			PageSz >>= 8;
		}
		NandParameter.RowAddrNr = 0;
		PageNr--;
		while (PageNr)
		{
			NandParameter.RowAddrNr++;
			PageNr >>= 8;
		}
		mpDebugPrint("\tPageSize=%d,SpareSize=%d,PagePerBlock=%d,BlockNum=%d", (1 << NandParameter.PageSize2Exp), NandParameter.SpareSize, NandParameter.PagePerBlock, NandParameter.Block_Nr);
		MP_DEBUG("address cycle: row=%d, col=%d", NandParameter.RowAddrNr, NandParameter.ColAddrNr);
	}

	return swRetValue;
}

#endif
#endif
