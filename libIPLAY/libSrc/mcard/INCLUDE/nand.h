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
* Filename      : nand.h
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
#ifndef __NAND_H
#define __NAND_H

/*
// constant define section
*/
#define NAND_CLOCK_KHZ			80000	// KHz

#define LOADER_CODE             0
#define RUN_ON_REAL_CHIP
#define NAND_ECC_TEST           0

/// Unsupport SmartMedia or XD type.
#define TYPE_NOT_SUPPORT		-2
/// Wait for SmartMedia or XD's ready signal fail.
#define TIMEOUT					-3
/// Wait for FIFO transaction end Fail.
#define IC_SFTCE_TIMEOUT		-4
/// Wait for DMA end Fail.
#define DMA_TIMEOUT				-5
/// 2 bit ecc error
#define UNCORRECTABLE_ERROR		-6
/// Read Only
#define READ_ONLY				-7
/// Read Status Fail
#define READ_STATUS_FAIL		-8


/****************************************************************************************
 *                              Nand flash command
 ****************************************************************************************/
#define SEQ_DATA_IN_CMD     	0x80	// Sequential Data Input
#define READ_PAGE1_CMD      	0x00	// Read fist page (0 to 255 byte)
#define READ_PAGE2_CMD      	0x01	// Read second page (256 to 511 byte)
#define READ_PAGE1_CMD_2CYC 	0x30	// Nand flash read command
#define READ_REDUNDANT_CMD  	0x50	// Read Redundant page (512 to 527 byte)
#define READ_IN_PAGE_S_CMD		0x05	// Random access within a page, column address need only
#define READ_IN_PAGE_E_CMD		0xE0
#define READ_ID_CMD         	0X90	// Read ID
#define READ_ID3_CMD			0x9A
#define RESET_CMD           	0xFF	// Reset
#define PAGE_PROG_CMD       	0x10	// Write
#define BLOCK_ERASE_CMD_1CYC	0x60	// Block Erase 1st cycle
#define BLOCK_ERASE_CMD_2CYC	0xD0	// Block Erase 2nd cycle
#define READ_STATUS         	0x70	// Read Status

/*****************************************************************************************
 *                             Nand flash type
 *****************************************************************************************/
#define TRADITIONAL             0
#define SAMSUNG_NEW             1
#define MICRON_12BITS_AAA       2
#define MICRON_12BITS_ABA       3
#define HYNIX_H_SERIES          4
#define TOSHIBA_NAND            5
#define HYNIX_HB_SERIES         6
#define HYNIX_HB_BG_SERIES      7
#define HYNIX_HC_AG_SERIES      8

/*****************************************************************************************
 *                             Define based on Platform
 *****************************************************************************************/
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define DIR_WRITE				0
#define DIR_READ				1
#endif


/*****************************************************************************************/
#if ((CHIP_VER_MSB == CHIP_VER_615) && (NAND_DRV == NAND_SIMPLE_DRV))
typedef struct NAND_INFO
{
	DWORD ECC_Type;
	DWORD ColAddrNr;	// offset within a page
	DWORD RowAddrNr;
	DWORD PageSize2Exp;
	DWORD SpareSize;
	DWORD PagePerBlock;
	DWORD SectorPerBlock;
	DWORD Block_Nr;
	DWORD PlaneSizeExp;
}NandInfo;
#endif

#if ((CHIP_VER_MSB == CHIP_VER_615) && (NAND_DRV == NAND_FTL_DRV))
typedef struct NAND_INFO
{
	DWORD ECC_Type;
	DWORD ColAddrNr;	// offset within a page
	DWORD RowAddrNr;
	DWORD PageSize2Exp;
	DWORD SpareSize;
	DWORD PagePerBlock;
	DWORD Block_Nr;
	DWORD PlaneSizeExp;
}NandInfo;
#endif

#if (((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660)) && (NAND_DRV == NAND_SIMPLE_DRV))
typedef struct NAND_INFO
{
	DWORD ECC_Type;
	DWORD ECC_Size;		// 512B or 1024B
	DWORD ColAddrNr;	// offset within a page
	DWORD RowAddrNr;
	DWORD PageSize2Exp;
	DWORD SpareSize;
	DWORD EccOffset;
	DWORD PagePerBlock;
	DWORD SectorPerBlock;
	DWORD Block_Nr;
	DWORD PlaneSizeExp;
} NandInfo;
#endif

#if (((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660)) && (NAND_DRV == NAND_FTL_DRV))
typedef struct NAND_INFO
{
	DWORD ECC_Type;
	DWORD ECC_Size;		// 512B or 1024B
	DWORD ColAddrNr;	// offset within a page
	DWORD RowAddrNr;
	DWORD PageSize2Exp;
	DWORD SpareSize;
	DWORD EccOffset;
	DWORD PagePerBlock;
	DWORD SectorPerBlock;
	DWORD Block_Nr;
	DWORD PlaneSizeExp;
} NandInfo;
#endif
/*****************************************************************************************/

enum LOG_BLK_STATUS
{
	LB_NO_ADDR            = 0xFFFFFFFF,
	LB_DEFAULT_BAD_BLOCK  = 0xFFFFFFFE,
	LB_BAD_BLOCK          = 0xFFFFFFFD,
	LB_READ_FAIL          = 0xFFFFFFFC,
	LB_UNRECOGNIZED       = 0xFFFFFFFB,
	LB_ERROR              = 0xFFFFFFFA
} LogBlkStatus;

#define LB_INVALID_ADDR_LOWBOUND    0x7ffffff0

enum STATUS_FLAG_TYPE
{
	SFT_NORMAL = 0,
	SFT_BAD_BLOCK,
	SFT_DATA_FAIL
}StatusTypeFlag;

typedef enum ECC_TYPE
{
	ECC_1BIT	= 0,
	ECC_8BIT	= 1,
	ECC_12BIT	= 2,
	ECC_16BIT	= 3,
	ECC_24BIT	= 4
} EccType;

/*****************************************************************************************
 *                             Macro
 *****************************************************************************************/
// Macro
#define SEND_COMMAND(bCommand)	{*((BYTE *)(&pstMcard->McSmCmd)) = bCommand & 0xff;}
#define SEND_ADDRESS(adr) 		{pstMcard->McSmAdr = adr & 0xff;}
#define READ_DATA()				pstMcard->McSmDat
#define SEND_DATA(dat)			{pstMcard->McSmDat = dat & 0xff;}


/*****************************************************************************************
 *                             Function prototype
 *****************************************************************************************/
#if (NAND_DRV == NAND_SIMPLE_DRV)
#if LOADER_CODE
#define NandSetChipNr(...)
#define NandSetCEx(dwPhyAddr)
#define ReadStatus()
#define SetRowAddress(Row)
#define CheckStatusFlag(x)
#define CheckEvenParity(x)
#define ReadLogBlkAddr(a, b)
#define GetNandLogBlkAddr(a, b)	LB_READ_FAIL
#define DumpSpareContent(x)
#define CheckSpareContent(x)	TRUE
#define SectorRead(a, b)		PASS
#define NandRawPageRead(a, b)	PASS
#define CalEvenParity(x)
#define BuildFTLArea(...)		PASS
#define NandBlockWrite(...)		PASS
#define NandRawPageWrite(...)	PASS
#define NandBlockErase(x)		PASS
#else
WORD GetNandLogBlkAddr(DWORD PhyAddr, DWORD MaxLogBlkAddr);
void DumpSpareContent(DWORD PhyAddr);
SWORD CheckSpareContent(DWORD PhyAddr);
SWORD NandRawPageRead(DWORD dwPhyAddr, DWORD dwBufferAddress);
SWORD NandSectorRead(DWORD dwPhyAddr, DWORD dwSectorCount, DWORD dwBufferAddress, DWORD *progress, BYTE *SectorStatus);
SWORD NandRawPageWrite(DWORD dwPhyAddr, DWORD dwBufferAddress);
SWORD NandBlockWrite(DWORD dwPhyAddr, DWORD dwBufferAddress, DWORD LogBlkAddr, DWORD *progress, BYTE bValid, BYTE *SectorStatus);
SWORD NandBlockErase(DWORD dwPhyAddr);
#endif

#elif (NAND_DRV == NAND_FTL_DRV)
DWORD GetNandLogAddr(DWORD PhyAddr);
SWORD CheckSpareContent(DWORD PhyAddr);
DWORD NandPageRead(DWORD dwPhyAddr, DWORD dwPageCount, DWORD Buffer, DWORD *progress);
SWORD NandPageWrite(DWORD dwPhyAddr, DWORD PageCnt, DWORD Buffer, DWORD LogAddr, DWORD *progress);
SWORD NandPageCopy(DWORD dstAddr, DWORD srcAddr, DWORD logAddr, BYTE *tmpBuf);
SWORD badBlockMark(DWORD dwPhyAddr);
SWORD badBlockMarkFirst(DWORD dwPhyAddr);
SWORD badBlockMarkLast(DWORD dwPhyAddr);
SWORD NandBlockErase(DWORD dwPhyAddr);
void NandDataRandom(BYTE *dst, BYTE *src, DWORD len, DWORD off);
void NandEraseAllBlocks();
void NandFTLEccCheck();
void Nand_SurfaceScan();
void Nand_Test_BlkLoop();
void McardNandActive();
void McardNandInactive();
void BootPageCheck();
SWORD NandIdentify();
void NandIdPaserFunInit(void **IdInfoGet);
#endif


#endif  // __SM_H

