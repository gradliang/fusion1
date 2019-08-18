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
* Filename      : sm.c
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
#if XD_ENABLE
#include "mpTrace.h"
#include "Mcard.h"
#include "uti.h"
#include "xD.h"

#if (DM9KS_ETHERNET_ENABLE == 1 )
#include "taskid.h"
#include "Peripheral.h"
#endif

/*
// Constant declarations
*/

#define SM_CLOCK_KHZ		48000
#define XD_CERTIFICATION
//#define SW_ECC 
#define DUMP_ERROR_SECTOR	0
#define WRITE_VERIFY		0

#define ECC4CHECK				ENABLE
#define ECC4S_ENABLE			ENABLE
#define DETECT_ENABLE           0x00000001
#define WRITE_ENABLE            0x00000002
#define ECC_ENABLE              0x00000004
#define TYPE_256_BYTE           0x00000000
#define TYPE_512_BYTE			0x00000008
#define CE_MASK_HIGH 			0x00000010
#define ADR_LENGTH_3 			0x00000000
#define ADR_LENGTH_4 			0x00000020
#define ADR_LENGTH_5 			0x00000040
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define ADR_LENGTH_6 			0x00000080
#define CS_SM        			0x00000100
#define CS_XD        			0x00000200
#define SM_WP					(1<<11)
#else
#define CS_SM        			0x00000080
#define CS_XD        			0x00000100
#define SM_WP					(1<<10)
#endif
#define SETTING_SM				(CS_SM | TYPE_512_BYTE | WRITE_ENABLE)
#define SETTING_XD				(CS_XD | TYPE_512_BYTE | WRITE_ENABLE)
#define IC_SRB                  0x00000001
#define IC_ECB  				0x00000002
#define IC_SFTCE				0x00000004
#define IC_SMTO 				0x00000008
#define IC_CDX  				0x00000010
#define IM_SRB  				0x00000100
#define IM_ECB  				0x00000200
#define IM_SFTCE				0x00000400
#define IM_SMTO 				0x00000800
#define IM_CDX  				0x00001000
#define IM_ALL 0				/*(IM_SRB | IM_ECB | IM_SFTCE | IM_SMTO | IM_CDX) */
#define PL_HIGH_SRB  			0x00010000
#define PL_HIGH_ECB  			0x00020000
#define PL_HIGH_SFTCE			0x00040000
#define PL_HIGH_SMTO 			0x00080000
#define PL_HIGH_CDX  			0x00100000
#define MD_EDGE_SRB  			0x01000000
#define MD_EDGE_ECB  			0x02000000
#define MD_EDGE_SFTCE			0x04000000
#define MD_EDGE_SMTO 			0x08000000
#define MD_EDGE_CDX  			0x10000000

#define SEQ_DATA_IN_CMD     	0x80	//Sequential Data Input
#define READ_PAGE1_CMD      	0x00	//Read fist page (0 to 255 byte)
#define READ_PAGE2_CMD      	0x01	//Read second page (256 to 511 byte)
#define READ_PAGE1_CMD_2CYC 	0x30	// nand flash read command
#define READ_REDUNDANT_CMD  	0x50	//Read Redundant page (512 to 527 byte)
#define READ_ID_CMD         	0X90	//Read ID
#define READ_ID3_CMD			0x9A
#define RESET_CMD           	0xFF	//Reset
#define PAGE_PROG_CMD       	0x10	//Write
#define BLOCK_ERASE_CMD_1CYC	0x60	//Block Erase 1st cycle
#define BLOCK_ERASE_CMD_2CYC	0xD0	//Block Erase 2nd cycle
#define READ_STATUS         	0x70	//Read Status

#define READ_TIMING_SM      0x00020201	
#define WRITE_TIMING_SM     0x00020201	

#define READ_TIMING_XD      0x00020201
#define WRITE_TIMING_XD     0x00020201

enum SM_TYPE{
	INITIAL_XD = 0,
	INITIAL_SM,
	INITIAL_NR
};

#define MAX_LOGBLOCK		0x3e8
#define MAX_BLOCK   		0x400
#define XD_MAX_ZONE    		0x80
#define XD_STA_ZONE    		0x02
#define XD_DNY_ZONE    		0x02
//#define XD_STA_ZONE    		0x80
//#define XD_DNY_ZONE    		0x00

#define MAX_ZONE    		0x80

#define TIMEOUT_COUNT		1000000
#define UNUSABLE_BLOCK		1
#define USABLE_BLOCK  		0

#define STATUS_FAIL        	0x01
#define STATUS_READY       	0x40

#define SM_CLOCK    0x1

#define ECC4SEN		BIT0 // 1 : enable ECC4S , 0 : disable ECC4S
#define ECC4SRN		BIT1 // 0 : reset ECC4S
#define EDN			BIT2 // 1 : encode , 0 : decode
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
#define NO_ERROR                        0
#define CORRECT_COMPLETE                1
#define ENCODE_COMPLETION               2
#define ENCODE_ERROR                   -1
#define DECODE_ERROR                   -2
#define DECODE_CORRECT_IMPOSSIBLE      -3
#define DECODE_CHECK_TIMEOUT           -4
//Multi Die Nand Define
#define MultiDie 1

enum STATUS_FLAG_TYPE
{
	SFT_NORMAL = 0,
	SFT_BAD_BLOCK,
	SFT_DATA_FAIL
};

///
///@defgroup SM SmartMedia and XD
///@ingroup CONSTANT
///@{


/// Unsupport SmartMedia or XD type.
#define TYPE_NOT_SUPPORT            -2
/// Wait for SmartMedia or XD's ready signal fail.
#define TIMEOUT                     -3
/// Wait for FIFO transaction end Fail.
#define IC_SFTCE_TIMEOUT            -4
/// Wait for DMA end Fail.
#define DMA_TIMEOUT                 -5
/// 2 bit ecc error
#define UNCORRECTABLE_ERROR         -6
/// Read Only
#define READ_ONLY                   -7
/// Read Status Fail
#define READ_STATUS_FAIL            -8
///@}
/*
// Structure declarations
*/
struct ST_MEDIA_MEAT_TAG
{
	BYTE bMaxZone;				// Max Zone Size        
	BYTE bMaxSectorExp;
	WORD wMaxSector;			// Sectors per block
	WORD wMaxBlock;				// Blocks per zone
	WORD wMaxLogBlock;			// LogBlocks perb zone
};
struct ST_MEDIA_MEAT_TABLE_TAG
{
	BYTE bID;
	struct ST_MEDIA_MEAT_TAG sMediaMeat;
};
struct ST_INFO_TAG
{
	BYTE bCurMode;
	BYTE bCurID[2];
	BYTE bReserve[1];
	DWORD dwXdCurZone;
	DWORD dwCurModeSetting;
	DWORD dwReadTiming;
	DWORD dwWriteTiming;
	struct ST_MEDIA_MEAT_TAG *sMediaMeat[2];
	WORD *wXdSLog2PhyTable; //[1][MAX_BLOCK * XD_STA_ZONE];
	WORD *wXdDLog2PhyTable; //[1][MAX_BLOCK * XD_DNY_ZONE];
	BYTE bBadBlockTable[INITIAL_NR][MAX_ZONE];
	BYTE bReAssignTable[INITIAL_NR][MAX_ZONE];
	BYTE bAddrInfo[INITIAL_NR][3];		//[n][0] command times, [n][1] read/write command, [n][2] earse command
};


/*
// Type declarations
*/
typedef struct ST_MEDIA_MEAT_TAG ST_MEDIA_MEAT;
typedef struct ST_MEDIA_MEAT_TABLE_TAG ST_MEDIA_MEAT_TABLE;
typedef struct ST_INFO_TAG ST_INFO;

/*
// Variable declarations
*/
static ST_INFO sInfo;
static const ST_MEDIA_MEAT_TABLE sMediaMeatTable_S[] = {
	//Id, MaxZone, MaxSectorExp, MaxSector, MaxBlock, MaxLogBlock 
	{0xe5, 0x01, 0x4, 0x10, 0x200, 0x1f4},	//  4 MB
	{0xe6, 0x01, 0x4, 0x10, 0x400, 0x3e8},	//  8 MB    
	{0x73, 0x01, 0x5, 0x20, 0x400, 0x3e8},	//  16 MB   
	{0x75, 0x02, 0x5, 0x20, 0x400, 0x3e8},	//  32 MB       
	{0x76, 0x04, 0x5, 0x20, 0x400, 0x3e8},	//  64 MB       
	{0x79, 0x08, 0x5, 0x20, 0x400, 0x3e8},	//  128 small MB    
	{0x71, 0x10, 0x5, 0x20, 0x400, 0x3e8},	//  256 small MB            
	{0xdc, 0x20, 0x5, 0x20, 0x400, 0x3e8},	//  512 small MB            
	{0xd3, 0x40, 0x5, 0x20, 0x400, 0x3e8},	//    1 small GB        
	{0xd5, 0x80, 0x5, 0x20, 0x400, 0x3e8},	//    2 small GB            
	{0x00, 0x00, 0x0, 0x00, 0x000, 0x000}
};

static const ST_MEDIA_MEAT_TABLE sMediaMeatTable_B[] = {
	//Id, MaxZone, MaxSectorExp, MaxSector, MaxBlock, MaxLogBlock 
	{0xf1, 0x01, 0x8, 0x100, 0x400, 0x3e8},	//  128 big MB        
	{0xda, 0x02, 0x8, 0x100, 0x400, 0x3e8},	//  256 big MB        
	{0xdc, 0x04, 0x8, 0x100, 0x400, 0x3e8},	//  512 big MB            
//	{0xdc, 0x02, 0x9, 0x200, 0x400, 0x3e8},	//  512 big MB            
	{0xd3, 0x08, 0x8, 0x100, 0x400, 0x3e8},	//    1 big GB            
	{0xd5, 0x10, 0x8, 0x100, 0x400, 0x3e8},	//    2 big GB        
	{0x00, 0x00, 0x0, 0x000, 0x000, 0x000}
};
static const ST_MEDIA_MEAT_TABLE sMediaMeatTable_MLC[] = {
	//Id, MaxZone, MaxSectorExp, MaxSector, MaxBlock, MaxLogBlock 
	{0xda, 0x01, 0x9, 0x200, 0x400, 0x3e8},	//  256 big MB       
	{0xdc, 0x02, 0x9, 0x200, 0x400, 0x3e8},	//  512 big MB            
	{0xd3, 0x04, 0x9, 0x200, 0x400, 0x3e8},	//  1 GB          	
	{0xd5, 0x08, 0x9, 0x200, 0x400, 0x3e8},	//  2 GB          	
	{0x00, 0x00, 0x0, 0x000, 0x000, 0x000}
};
static DWORD MaxPhyPage ;
static DWORD MaxLogPage ;
static WORD wLogAddr1;
static WORD wLogAddr2;
static BYTE MLC[INITIAL_NR] = {FALSE, FALSE};
static BYTE PreSMType;
static BYTE bECCCode[3];
#pragma alignvar(4)
static BYTE bRedtData[16];
static volatile BOOL blTimeOutFlag;
static WORD cardType=0;
static BYTE bDescriptor[INITIAL_NR][3] = {"XD", "SM"}; 
static DWORD BadBlockNr[INITIAL_NR] = {0};
static DWORD CIS_BlkAddr = 0;
#pragma alignvar(4)
static const BYTE bCISField[] = {
	0x01, 0x03, 0xd9, 0x01, 0xff, 0x18, 0x02, 0xdf, 0x01, 0x20, 0x04, 0x00, 0x00, 0x00, 0x00, 0x21,
	0x02, 0x04, 0x01, 0x22, 0x02, 0x01, 0x01, 0x22, 0x03, 0x02, 0x04, 0x07, 0x1a, 0x05, 0x01, 0x03,
	0x00, 0x02, 0x0f, 0x1b, 0x08, 0xc0, 0xc0, 0xa1, 0x01, 0x55, 0x08, 0x00, 0x20, 0x1b, 0x0a, 0xc1,
	0x41, 0x99, 0x01, 0x55, 0x64, 0xf0, 0xff, 0xff, 0x20, 0x1b, 0x0c, 0x82, 0x41, 0x18, 0xea, 0x61,
	0xf0, 0x01, 0x07, 0xf6, 0x03, 0x01, 0xee, 0x1b, 0x0c, 0x83, 0x41, 0x18, 0xea, 0x61, 0x70, 0x01,
	0x07, 0x76, 0x03, 0x01, 0xee, 0x15, 0x14, 0x05, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x00, 0x20, 0x20, 0x20, 0x20, 0x00, 0x30, 0x2e, 0x30, 0x00, 0xff, 0x14, 0x00, 0xff, 0x00, 0x00
};

/*
// Macro declarations
*/
//#define SM_TIMEOUT_HANDLE

//#define SM_TASK_YIELD
#ifdef SM_TASK_YIELD
#define TASK_YIELD() TaskYield()
#else
#define TASK_YIELD()
#endif

/*
// Static function prototype
*/
static void En_WP(void);
static void Dis_WP(void);
static void Power_On(void);
static void Power_Off(void);
static void ChipEnable();
static void ChipDisable();
static void CommandProcess(void *pMcardDev);
//static SWORD Log2PhyTabInit(void);
static SWORD Log2PhyTabInit(BYTE bStartZone,BYTE bEndZone);
static BOOL CheckBadBlockTable(WORD wCurPhyBlock, WORD wCurZone);
static BOOL CheckBlockStatus(void);
static WORD GetSMEmptyBlock(DWORD dwCurZone,WORD wZoneNum,WORD *pwLog2PhyTable);
static SWORD ReadRedtData(DWORD dwSectorAddr);
static DWORD GetSameBlock(DWORD dwLogAddr, DWORD dwSectorCount);
static void Select(BYTE bMode);
static void DeSelect(void);
static inline BOOL Polling_SM_Status(void);
static void SetCommand(BYTE bCommand);
static SWORD Identify(DWORD * pdwTotalSector);
static void SetAddrInfo(BYTE bType);
static void SetControl(void);
static SWORD WaitReady(void);
static void SetAddress(DWORD dwPhyAddr, BYTE bMode);
static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr);
static SWORD LogicalRead(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress);
static SWORD PhysicalRead(DWORD dwPhyAddr, DWORD dwSectorCount, DWORD dwBufferAddress, BYTE *SectorStatus);
static SWORD ReadSector(DWORD dwBufferAddress, WORD wSize);
static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr);
static SWORD LogicalWrite(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress);
static SWORD RecursivePhysicalWrite(DWORD* pdwPhyBlock, DWORD dwSectorCount, DWORD dwBufferAddress,
						   DWORD dwLogAddr,DWORD dwSZone,DWORD dwZoneNum,WORD *pwLog2PhyTable,BYTE bXd, BYTE *SectorStatus);

static SWORD PhysicalWrite(DWORD dwPhyAddr, DWORD dwSectorCount, DWORD dwBufferAddress,
						   DWORD dwLogAddr,BYTE bValid, BYTE *SectorStatus);
static SWORD WriteSector(DWORD dwBufferAddress, WORD wSize);
static SWORD PhysicalErase(DWORD dwPhyAddr);
static SWORD BadBlockMangment(DWORD* pdwCurPhyAddr,DWORD dwLogBlock,DWORD dwZoneNum);

static SWORD RecursivePhysicalErase(DWORD* pdwCurPhyAddr,DWORD dwLogBlock,DWORD dwSZone,DWORD dwZoneNum,WORD *pwLog2PhyTable);
static SWORD ReadStatus(void);
static void BuildRedtData(DWORD dwLogAddr,BYTE bValid);
static SWORD LowLevelFormat();
static int CheckWordParity(WORD wBlockAddr);
static int BitXor(DWORD dwVector, DWORD dwSize);
static DWORD RowBlockXor(DWORD dwStartAddr, int RowSize);
static int ECCBitCode(DWORD dwStartAddr, int FristStart, int LastStart, int RowSize);
static DWORD OneRowXor(DWORD dwStartAddr, int FristStart, int LastStart);
static int ColXor(DWORD dwVector, int ColSize);
static void CalECC(DWORD DataAddr);
static void ECC4S_Init(void);
static void ECC4S_Encode(void);
static void ECC4S_Decode(void);
static SWORD ECC4S_Decode_Check(BYTE *pdwBufferAddress);
static SWORD ECC4S_Encode_Check(void);
static void ECC4S_Correct(BYTE *pdwBufferAddress);
static SWORD Slc_Ecc_Correct(BYTE *pbBuffer,BYTE bField);
static void EraseWhole();
static void xD_RW_Test();

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
static WORD ReadLogicalBlockAddr(BYTE *redundent, DWORD maxAddr)
{
	#define CHECKRANGE(x)	(((x & 0xf800) == 0x1000) && (((x >> 1) & 0x03ff) < maxAddr))
	WORD blkaddr1, blkaddr2;
	WORD ret = -1;

	blkaddr1 = (redundent[6] << 8) + redundent[7];
	blkaddr2 = (redundent[11] << 8) + redundent[12];
	if (blkaddr1 == blkaddr2)
	{
		if ((CheckEvenParity(blkaddr1) && CHECKRANGE(blkaddr1))
			|| (blkaddr1 == 0))	// CIS block
			ret = (blkaddr1 >> 1) & 0x3ff;
	}
	else	// different, one of them are correct address.
	{
		if (CheckEvenParity(blkaddr2) && CHECKRANGE(blkaddr2))	// addr2 is good
		{
			if (!CheckEvenParity(blkaddr1) || !CHECKRANGE(blkaddr1))	// addr1 is bad
				ret = (blkaddr2 >> 1) & 0x3ff;
		}
		else	// addr2 is bad
		{
			if (CheckEvenParity(blkaddr1) && CHECKRANGE(blkaddr1))	// addr1 is good
				ret = (blkaddr1 >> 1) & 0x3ff;
		}
	}
	
	return ret;	
}

/*
// Definition of internal functions
*/
static SWORD Slc_Ecc_Correct(BYTE *pbBuffer,BYTE bField)
{
	DWORD dwNandEcc,dw600Ecc,dwEccValue;
	BYTE bCPValue,bCPCount,bLPCount,bBitAddr,bOldErrValue,bNewErrValue,bXorBitValue;
	BYTE bTempCount;
	WORD wLPValue,wByteAddr,wTemp;
	if(bField == 1)
		dwNandEcc = ( (bRedtData[10] << 16) | (bRedtData[9] << 8) |bRedtData[8] );	
	else if(bField== 0)
		dwNandEcc = ( (bRedtData[15] << 16) | (bRedtData[14] << 8) |bRedtData[13] );

	dw600Ecc = ( (bECCCode[2] << 16) |(bECCCode[1] << 8) |bECCCode[0] );
	dwEccValue = dwNandEcc ^ dw600Ecc;
	bCPValue = (dwEccValue & 0xfc0000) >> 18;
	wLPValue = dwEccValue & 0xffff;
// ECC Pattern error check 
	bTempCount = 16;
	wTemp = wLPValue;
	while(bTempCount)
	{
		if( ((wTemp & 0x03) == 0x11) || ((wTemp & 0x03) == 0x00) )
		{
			MP_DEBUG("Un-correctable data error or ECC error happen ....");
			return UNCORRECTABLE_ERROR;
		}
		wTemp >>= 2;
		bTempCount -=2;	
	}
		

// 1 bit correct	
	bCPCount = 6;
	bLPCount= 16;
	bBitAddr=0;
	wByteAddr= 0;
	while(bCPCount)
	{
		bBitAddr <<=1;	
		if(bCPValue & 0x20)
			bBitAddr |= 0x01;
		bCPValue <<= 2;
		bCPCount -= 2;
	};
	
	while(bLPCount)
	{
		wByteAddr <<=1;
		if(wLPValue & 0x8000)
			wByteAddr |= 0x01;
		wLPValue <<= 2;

		bLPCount -= 2;
	};
	bXorBitValue = 1 << bBitAddr;
	MP_DEBUG2("Byte Addr 0x%x, Bit Addr %d",wByteAddr,bBitAddr);
	bOldErrValue = pbBuffer[wByteAddr];
	bNewErrValue = bOldErrValue;

	bNewErrValue ^= bXorBitValue;
	pbBuffer[wByteAddr] = bNewErrValue;

	MP_DEBUG2("Correct ok 0x%x -> 0x%x",bOldErrValue,bNewErrValue);

	return PASS;

}
#if ECC4S_ENABLE
static void ECC4S_Init(void)
{
	MCARD * sMcard =(MCARD *)MCARD_BASE;
	sMcard->McEcc4SC = (EDN | ECC4SRN | ECC4SEN);
}
static void ECC4S_Encode(void)
{
	MCARD * sMcard =(MCARD *)MCARD_BASE;
	sMcard->McEcc4SC = 0;
	sMcard->McEcc4SC = (EDN |ECC4SRN |ECC4SEN) ;
}
static void ECC4S_Decode(void)
{
	MCARD * sMcard =(MCARD *)MCARD_BASE;
	sMcard->McEcc4SC = 0;
	sMcard->McEcc4SC = ECC4SEN ;
	sMcard->McEcc4SC |= ECC4SRN;
	sMcard->McEcc4SC |= 1 << 11;
}
static void ECC4S_Decode_Fill(BYTE *eccBuf)
{
	MCARD * sMcard =(MCARD *)MCARD_BASE;
    sMcard->McEcc4SSA0[1] &= 0xffff0000;
    sMcard->McEcc4SSA0[1] = ((eccBuf[0]<<8) | eccBuf[1]);
    sMcard->McEcc4SSA0[2] = ((eccBuf[2]<<24) | (eccBuf[3]<<16) | (eccBuf[4]<<8) | eccBuf[5]);
    sMcard->McEcc4SSA0[3] = ((eccBuf[6]<<24) | (eccBuf[7]<<16) | (eccBuf[8]<<8) | eccBuf[9]);
}
static void ECC4S_Correct(BYTE *pdwBufferAddress)
{
	register MCARD * sMcard = (MCARD *) MCARD_BASE;
	BYTE bErrCount,i,bVal;
	BYTE * pbBuffer;
	WORD wAddr;
	BYTE bOld;
	pbBuffer = pdwBufferAddress;
	bErrCount = (BYTE)((sMcard->McEcc4SState & MC_ECC4S_STATE_SERR_MASK)>>4);
	MP_DEBUG1("ECC4S Correct count = %d",bErrCount);
	for(i=0;i<bErrCount;i++)
	{
		wAddr = (WORD)(sMcard->McEcc4SErr[i] & 0x000003ff);
		if(wAddr <=7)
			continue;
		bOld = pbBuffer[521-wAddr]; 
		bVal =(BYTE) ((sMcard->McEcc4SErr[i] & 0x03ff0000)>>16);
		pbBuffer[521-wAddr] ^= (BYTE)bVal;	
		MP_DEBUG2("0x%x -> 0x%x",bOld,pbBuffer[521-wAddr]);
	}
	
}
static DWORD ECC4S_Decode_Wait()
{
	register MCARD * sMcard = (MCARD *)MCARD_BASE;
    DWORD timeout = 0xffff;
    BYTE sts;

    while(timeout)
    {
        timeout--;
        sts = (sMcard->McEcc4SState & 0xff0000) >> 16;
        if(!sts)
            break;
        else
        {
            if(sts == 0x4 && !(sMcard->McEcc4SC & 0x400))   //auto mode disable
                break;
        }
    }
    if(!timeout)
    {
        mpDebugPrint("ECC4S timeout1: %x", sMcard->McEcc4SState);
        return FAIL;
    }

    timeout = 0xffff;
    while(timeout)
    {
        timeout--;
        sts = sMcard->McEcc4SState & 0xf;
        if(sts == 0 || sts == 1 || sts == 3)
            break;
    }
    if(!timeout)
    {
        mpDebugPrint("ECC4S timeout2: %x", sMcard->McEcc4SState);
        return FAIL;
    }


    sts = sMcard->McEcc4SState & 0xf;
    if(!sts || sts == 0x3)  //no err or correction complete
        return PASS;
    else
    {
        if((sMcard->McEcc4SSA0[1] & 0xffff) != 0xffff || sMcard->McEcc4SSA0[2] != 0xffffffff ||
            sMcard->McEcc4SSA0[3] != 0xffffffff)   //when the page is not even writed
        {
            mpDebugPrint("ECC4S state error: %x", sMcard->McEcc4SState);
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
	register MCARD * sMcard = (MCARD *)MCARD_BASE;
	SWORD dwRetValue;
	
	if (ECC4S_Decode_Wait() == PASS)
	{
		if((sMcard->McEcc4SState & MC_ECC4S_STATE_STATE_MASK) == DECODE_NO_ERROR)
		{
			dwRetValue = CORRECT_COMPLETE;
		}
		else if((sMcard->McEcc4SState & MC_ECC4S_STATE_STATE_MASK) == DECODE_CORRECTION_COMPLETED)
		{		
			ECC4S_Correct(pdwBufferAddress);
//			MP_DEBUG("ECC4S_Decode_Check correction Complete");
			dwRetValue = CORRECT_COMPLETE;
		}
		else if((sMcard->McEcc4SState & MC_ECC4S_STATE_STATE_MASK) == DECODE_CORRECTION_IMPOSSIBLE)
		{
	        if((sMcard->McEcc4SSA0[1] & 0xffff) != 0xffff || sMcard->McEcc4SSA0[2] != 0xffffffff ||
    	        sMcard->McEcc4SSA0[3] != 0xffffffff)   //when the page is not even writed
	        {
				MP_DEBUG("ECC4S_Decode_Check correction impossible");
				dwRetValue = DECODE_CORRECT_IMPOSSIBLE;
	        }
			else
				dwRetValue = CORRECT_COMPLETE;
		}
		else
		{
			MP_DEBUG("ECC4S_Decode_Check unknown error(%x)", sMcard->McEcc4SState);
			dwRetValue = DECODE_ERROR;
		}
	}
	else
	{
		MP_DEBUG("ECC4 R->%x,%x,%x", sMcard->McEcc4SSA0[1], sMcard->McEcc4SSA0[2], sMcard->McEcc4SSA0[3]);
		dwRetValue = DECODE_CHECK_TIMEOUT;
	}
//				MP_DEBUG("ECC4S_Decode_Check no error");
	return dwRetValue;
}
static SWORD ECC4S_Encode_Check(void)
{
	register MCARD * sMcard = (MCARD *)MCARD_BASE;
	SWORD dwRetValue;
	DWORD dwCounter = 0x1000;
	while((sMcard->McEcc4SState & MC_ECC4S_STATE_STATE_MASK) != ENCODE_NORMAL_COMPLETION)
	{
		if(dwCounter == 0)
		{
			MP_DEBUG("ECC4S_Encode_Check time out");		
			return -1;
		}
		dwCounter--;
	}
	return ENCODE_COMPLETION;

}
#endif
static SWORD ECC_Check(DWORD dwDataAddr,BYTE bFlag)
{
	SWORD ret = 0;
#ifdef SW_ECC

	register MCARD * sMcard = (MCARD *)MCARD_BASE;
	CalECC( dwDataAddr);
	bECCCode[2] = ((sMcard->McSmEcc1 >> 16) & 0xff);
	bECCCode[1] = ((sMcard->McSmEcc1 >> 8) & 0xff);
	bECCCode[0] = (sMcard->McSmEcc1  & 0xff);
	if((bRedtData[13] != bECCCode[0]) ||(bRedtData[14] != bECCCode[1]) ||(bRedtData[15] != bECCCode[2]))
	{
		if (Slc_Ecc_Correct(dwDataAddr,0) != PASS)
			ret = 1;
	}
	CalECC( dwDataAddr+256);

	if((bRedtData[8] != bECCCode[0]) ||(bRedtData[9] != bECCCode[1]) ||(bRedtData[10] != bECCCode[2]))
	{
		if (Slc_Ecc_Correct(dwDataAddr+256,1) != PASS)
			ret |= 2;
	}
	return PASS;


#else
	register MCARD * sMcard = (MCARD *)MCARD_BASE;

	bECCCode[2] = ((sMcard->McSmEcc1 >> 16) & 0xff);
	bECCCode[1] = ((sMcard->McSmEcc1 >> 8) & 0xff);
	bECCCode[0] = (sMcard->McSmEcc1  & 0xff);



	if((bRedtData[13] != bECCCode[0]) ||(bRedtData[14] != bECCCode[1]) ||(bRedtData[15] != bECCCode[2]))
	{
		if (Slc_Ecc_Correct((BYTE *)dwDataAddr,0) != PASS)
			ret = 1;

	}
	bECCCode[2] = ((sMcard->McSmEcc2 >> 16) & 0xff);
	bECCCode[1] = ((sMcard->McSmEcc2 >> 8) & 0xff);
	bECCCode[0] = (sMcard->McSmEcc2  & 0xff);	


	if((bRedtData[8] != bECCCode[0]) ||(bRedtData[9] != bECCCode[1]) ||(bRedtData[10] != bECCCode[2]))
	{
		if (Slc_Ecc_Correct((BYTE *)(dwDataAddr+256),1) != PASS)
			ret |= 2;

	}
	return ret;

#endif
}

void xDInit(ST_MCARD_DEV * sDev)
{
	sDev->pbDescriptor = bDescriptor[0];
	sInfo.wXdDLog2PhyTable = NULL;
	sInfo.wXdSLog2PhyTable = NULL;
	sDev->wMcardType = DEV_XD;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
}

static void CommandProcess(void *pMcardDev)
{
	register ST_MCARD_DEV *pDev = ((ST_MCARD_DEV *) pMcardDev);
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;
	ST_MEDIA_MEAT *sMediaMeat;
	BYTE bStartZone,bEndZone;

#if DM9KS_ETHERNET_ENABLE
    SemaphoreWait(CFETHERNET_MCARD_SEMA);
    Gpio_IntDisable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
#endif

	SetMcardClock(SM_CLOCK_KHZ);
	//mpDebugPrint("xD(%x) Command: %x(%x,%d)", pDev->wMcardType, mail->wCmd, mail->dwBlockAddr, mail->dwBlockCount);
	if (pDev->wMcardType == DEV_XD)
	{
#ifdef XD_CERTIFICATION
		if( mail->wCmd == INIT_CARD_CMD )
		{
            Power_On();
		}
#endif
		Select(INITIAL_XD);
	}
#if SM_ENABLE
	else if (pDev->wMcardType == SM)
	{
		Select(INITIAL_SM);
	}
#endif
	else
	{
#if (DM9KS_ETHERNET_ENABLE == 1 )
    Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
    SemaphoreRelease(CFETHERNET_MCARD_SEMA);
#endif
		return;
	}
	switch (mail->wCmd)
	{
	case INIT_CARD_CMD:
		MP_DEBUG1("pDev->wMcardType %d", pDev->wMcardType);
		En_WP();
	
		MP_DEBUG1("pDev->Flag.Detected %d", pDev->Flag.Detected);
		
		if (pDev->Flag.Detected)
		{
			if (((mail->swStatus = Identify(&pDev->dwCapacity))))
			{
				pDev->Flag.Present = 0;
				pDev->dwCapacity = 0;
				pDev->wSectorSize = 0;
				pDev->wSectorSizeExp = 0;
			}
			else
			{
				//LowLevelFormat();
				sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];			
				pDev->wSectorSize = MCARD_SECTOR_SIZE;
				pDev->wSectorSizeExp = MCARD_SECTOR_SIZE_EXP;
				bStartZone = 0;
				bEndZone = (sMediaMeat->bMaxZone > XD_STA_ZONE) ? XD_STA_ZONE : sMediaMeat->bMaxZone;
				sInfo.dwXdCurZone = (DWORD)bStartZone;
				if(sInfo.wXdSLog2PhyTable == NULL)
				{
					sInfo.wXdSLog2PhyTable = (WORD *) ker_mem_malloc(MAX_BLOCK * XD_STA_ZONE*2, TaskGetId());
					if(sInfo.wXdSLog2PhyTable == NULL)
						goto Log2PhyTabFail;
				}

				if(sInfo.wXdDLog2PhyTable == NULL)
				{
					sInfo.wXdDLog2PhyTable = (WORD *) ker_mem_malloc(MAX_BLOCK * XD_DNY_ZONE*2, TaskGetId());
					if(sInfo.wXdDLog2PhyTable == NULL)
					{
						ker_mem_free(sInfo.wXdSLog2PhyTable);
						sInfo.wXdSLog2PhyTable = NULL;
						goto Log2PhyTabFail;
					}
				}

 				if (mail->swStatus = Log2PhyTabInit(bStartZone,bEndZone))
				{
Log2PhyTabFail:
					MP_DEBUG("Log2PhyTabInit fail");
					pDev->Flag.Present = 0;
					pDev->dwCapacity = 0;
					pDev->wSectorSize = 0;
					pDev->wSectorSizeExp = 0;
				}
				else
				{
					MP_DEBUG("Log2PhyTabInit pass");
					pDev->wRenewCounter++;
					if (BadBlockNr[sInfo.bCurMode] >= (MAX_BLOCK - MAX_LOGBLOCK))
						pDev->Flag.ReadOnly = 1;
					else
						pDev->Flag.ReadOnly = 0;
					pDev->Flag.Present = 1;
				}
			}
		}
		else
		{
			//card out

            En_WP();
			ChipDisable();
            Power_Off();
			if(sInfo.wXdSLog2PhyTable != NULL)
			{
				ker_mem_free(sInfo.wXdSLog2PhyTable);
				sInfo.wXdSLog2PhyTable = NULL;
			}
			if(sInfo.wXdDLog2PhyTable != NULL)
			{
				ker_mem_free(sInfo.wXdDLog2PhyTable);
				sInfo.wXdDLog2PhyTable = NULL;
			}
				
			pDev->Flag.Present = 0;
			pDev->Flag.ReadOnly = 0;
			pDev->Flag.PipeEnable = 0;
			mail->swStatus = 0;
			pDev->dwCapacity = 0;
			pDev->wSectorSize = 0;
			pDev->wSectorSizeExp = 0;
		}
		break;
	case REMOVE_CARD_CMD:		//Athena 03.11.2006 seperate card in & out
		//card out
		if(sInfo.wXdSLog2PhyTable != NULL)
		{
			ker_mem_free(sInfo.wXdSLog2PhyTable);
			sInfo.wXdSLog2PhyTable = NULL;
		}
		if(sInfo.wXdDLog2PhyTable != NULL)
		{
			ker_mem_free(sInfo.wXdDLog2PhyTable);
			sInfo.wXdDLog2PhyTable = NULL;
		}
		pDev->Flag.Present = 0;
		pDev->Flag.ReadOnly = 0;
		pDev->Flag.PipeEnable = 0;
		mail->swStatus = 0;
		pDev->dwCapacity = 0;
		pDev->wSectorSize = 0;
		pDev->wSectorSizeExp = 0;

        if (pDev->wMcardType == DEV_XD)		
        {

//#ifdef XD_CERTIFICATION
#if MCARD_POWER_CTRL
			((MCARD *)MCARD_BASE)->McardC = 0;
            En_WP();
			ChipDisable();	            
			Power_Off();
			McardIODelay(0x500);
#endif
        }
		break;
	case SECTOR_READ_CMD:
		if (mail->dwBlockAddr >= pDev->dwCapacity)
		{
			mail->swStatus = FAIL;
			mpDebugPrint("xD : read() invalid blk %x", mail->dwBlockAddr);
		}
		else
		{
			mail->swStatus = FlatRead(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr);
		}
		break;

	case SECTOR_WRITE_CMD:
		if (pDev->Flag.ReadOnly == 1)
		{
			mail->swStatus = FAIL;
			mpDebugPrint("xD : Read only!");
		}
		else if (mail->dwBlockAddr >= pDev->dwCapacity)
		{
			mail->swStatus = FAIL;
			mpDebugPrint("xD : write() invalid blk %x", mail->dwBlockAddr);
		}
		else
		{
			Dis_WP();
			mail->swStatus = FlatWrite(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr);
			En_WP();
			if (BadBlockNr[sInfo.bCurMode] >= (MAX_BLOCK - MAX_LOGBLOCK))
				pDev->Flag.ReadOnly = 1;
		}
		break;

	case RAW_FORMAT_CMD:
		if (pDev->Flag.ReadOnly != 1)
		{
			Dis_WP();
			mail->swStatus = LowLevelFormat();
			En_WP();
			if (BadBlockNr[sInfo.bCurMode] >= (MAX_BLOCK - MAX_LOGBLOCK))
				pDev->Flag.ReadOnly = 1;
		}
		break;
		
	default:
		MP_DEBUG("-E- INVALID CMD");
		break;
	}
	DeSelect();
    En_WP();

#if (DM9KS_ETHERNET_ENABLE == 1 )
    Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
    SemaphoreRelease(CFETHERNET_MCARD_SEMA);
#endif
}


static SWORD Log2PhyTabInit(BYTE bStartZone,BYTE bEndZone)
{
	ST_MEDIA_MEAT *sMediaMeat;
	DWORD dwCurAddr, dwCurAddrBase;
	SWORD swRetValue;
	WORD wLogBlockAddr, wZoneNum, wReassignState, i, wCurPhyBlock, wLogBlock, wPhyBlock, wTemp;
	WORD *pwLog2PhyTable;
	BYTE *pReAssignTable;
	BYTE *pBadBlockTable;	
	BYTE bBadBlock;
	DWORD dwBase;
	BYTE *bTempBuffer;
	
	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];
	if(bStartZone>= XD_STA_ZONE)
		pwLog2PhyTable = (WORD *) sInfo.wXdDLog2PhyTable;
	else if(bStartZone < XD_STA_ZONE)
		pwLog2PhyTable = (WORD *) sInfo.wXdSLog2PhyTable;
	pReAssignTable = (BYTE *) sInfo.bReAssignTable[sInfo.bCurMode];
	pBadBlockTable = (BYTE *) sInfo.bBadBlockTable[sInfo.bCurMode];	
	bTempBuffer = (BYTE *)ext_mem_malloc(0x400 * (bEndZone-bStartZone) * 2); // rick : can shrink dowm
	
	if(bTempBuffer == NULL)
		return FAIL;
	
	for (wZoneNum = bStartZone; wZoneNum < bEndZone; wZoneNum++)
	{
		pBadBlockTable[wZoneNum]=0;	
		dwBase = (wZoneNum-bStartZone)*MAX_BLOCK;
		wReassignState = 0;
		for (i = 0; i < MAX_BLOCK; i++)
		{
			pwLog2PhyTable[dwBase + i] = 0xffff;
			bTempBuffer[(i << 1)] = 0xff;
			bTempBuffer[(i << 1) + 1] = 0xff;
		}
		if ((sInfo.bCurMode == INITIAL_XD) && (wZoneNum == 0))	// skip CIS
		{
			dwCurAddrBase = 0;
			dwCurAddr = (CIS_BlkAddr + 1) * sMediaMeat->wMaxSector;
			wCurPhyBlock = CIS_BlkAddr + 1;
		}
		else
		{
			dwCurAddrBase = dwCurAddr = wZoneNum * sMediaMeat->wMaxBlock * sMediaMeat->wMaxSector;
			wCurPhyBlock = 0;
		}
		for (; wCurPhyBlock < sMediaMeat->wMaxBlock; wCurPhyBlock++)
		{
			if (Polling_SM_Status())
			{
				ext_mem_free(bTempBuffer);
				return FAIL;
			}
			if ((swRetValue = ReadRedtData(dwCurAddr)))
			{
				ext_mem_free(bTempBuffer);
				MP_DEBUG1("-E- ReadRedtData FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			dwCurAddr += sMediaMeat->wMaxSector;	

			if (CheckSatusFlag(bRedtData[5]) >= 2)	// bad block: more than 2 bits zero
			{
				pBadBlockTable[wZoneNum]++;
				if (pBadBlockTable[wZoneNum] > BadBlockNr[sInfo.bCurMode])
					BadBlockNr[sInfo.bCurMode] = pBadBlockTable[wZoneNum];
				pwLog2PhyTable[dwBase+ MAX_BLOCK - pBadBlockTable[wZoneNum]] = wCurPhyBlock;
				MP_DEBUG("%d zone %d Block is bad block",wZoneNum,wCurPhyBlock);
				continue;
			}

			wLogBlockAddr = ReadLogicalBlockAddr(bRedtData, sMediaMeat->wMaxLogBlock);

			if (wLogBlockAddr >= sMediaMeat->wMaxLogBlock)	// add to free list
			{
				bTempBuffer[wReassignState * 2] = (BYTE) (wCurPhyBlock >> 8);
				bTempBuffer[wReassignState * 2 + 1] = (BYTE) wCurPhyBlock;
				wReassignState++;
			}
			else
			{	// XD Tcode 260
				if (pwLog2PhyTable[dwBase + wLogBlockAddr] == 0xffff)
				{
					pwLog2PhyTable[dwBase + wLogBlockAddr] = wCurPhyBlock;
				}
				else	// already be occupied.
				{	// check last page of occupyier block
					DWORD occupierSect = dwCurAddrBase+pwLog2PhyTable[dwBase+wLogBlockAddr]*sMediaMeat->wMaxSector;
					mpDebugPrint("-E- Logical address is duplication(%d,%d)", pwLog2PhyTable[dwBase+wLogBlockAddr], wCurPhyBlock);
					if ((swRetValue = ReadRedtData(occupierSect+sMediaMeat->wMaxSector-1)))
					{
						ext_mem_free(bTempBuffer);
				        MP_DEBUG1("-E- ReadRedtData FAIL (swRetValue: %d)", swRetValue);
						return swRetValue;
					}
					if (ReadLogicalBlockAddr(bRedtData, sMediaMeat->wMaxLogBlock) == wLogBlockAddr)
					{	// last page's block address is also correct. Use it and add wCurPhyBlock to free list.
						mpDebugPrint("\t->select %d", pwLog2PhyTable[dwBase+wLogBlockAddr]);
						bTempBuffer[wReassignState * 2] = (BYTE) (wCurPhyBlock >> 8);
						bTempBuffer[wReassignState * 2 + 1] = (BYTE) wCurPhyBlock;
						wReassignState++;
					}
					else
					{	// occupier is not good.
						wPhyBlock = pwLog2PhyTable[dwBase + wLogBlockAddr];	// add occupier to free list
						bTempBuffer[wReassignState * 2] = (BYTE) (wPhyBlock >> 8);
						bTempBuffer[wReassignState * 2 + 1] = (BYTE) wPhyBlock;
						wReassignState++;
						// check last page of current block
						if ((swRetValue = ReadRedtData(dwCurAddr - 1)))	// check last page of this block
						{
							ext_mem_free(bTempBuffer);
					        MP_DEBUG1("-E- ReadRedtData FAIL (swRetValue: %d)", swRetValue);
							return swRetValue;
						}
						if (ReadLogicalBlockAddr(bRedtData, sMediaMeat->wMaxLogBlock) == wLogBlockAddr)
						{	// it's good. add occupier to free list.
							mpDebugPrint("\t->select %d", wCurPhyBlock);
							pwLog2PhyTable[dwBase + wLogBlockAddr] = wCurPhyBlock;
						}
						else
						{
							mpDebugPrint("\t->both failed!");
							bTempBuffer[wReassignState * 2] = (BYTE) (wCurPhyBlock >> 8);
							bTempBuffer[wReassignState * 2 + 1] = (BYTE) wCurPhyBlock;
							wReassignState++;
						}
					}
				}
			}
		}

		for (i = 0; i < sMediaMeat->wMaxLogBlock; i++)
		{
			if ((pwLog2PhyTable[dwBase + i] == 0xffff) && (wReassignState != 0))
			{
				wTemp = bTempBuffer[wReassignState * 2 - 2];
				wTemp = (wTemp << 8) + bTempBuffer[wReassignState * 2 - 1];
				pwLog2PhyTable[dwBase + i] = wTemp;
				wReassignState--;
			}
		}


		for (i = 0; i < (MAX_BLOCK-MAX_LOGBLOCK-pBadBlockTable[wZoneNum]); i++)
		{
			if (wReassignState != 0)
			{
				wTemp = bTempBuffer[wReassignState * 2 - 2];
				wTemp = (wTemp << 8) + bTempBuffer[wReassignState * 2 - 1];
				pwLog2PhyTable[dwBase + MAX_LOGBLOCK + i] = wTemp;
				wReassignState--;
			}
		}

		pReAssignTable[wZoneNum] = 0;
		MP_DEBUG2("%d zone have %d bad block",wZoneNum,pBadBlockTable[wZoneNum]);


		if (GetSMEmptyBlock((wZoneNum-sInfo.dwXdCurZone),wZoneNum,pwLog2PhyTable) == 0xffff)
		{
			MP_DEBUG("-E- Log2PhyTabInit Get Empty Block Address FAIL");
			BadBlockNr[sInfo.bCurMode] = MAX_BLOCK - MAX_LOGBLOCK;
		}
	}
    ext_mem_free(bTempBuffer);
	return PASS;
}

static WORD GetSMEmptyBlock(DWORD dwCurZone,WORD wZoneNum,WORD *pwLog2PhyTable)
{
	WORD wBlankAddr;
	BYTE *pReAssignTable;
	BYTE *pBadBlockTable;
	pReAssignTable = (BYTE *) sInfo.bReAssignTable[sInfo.bCurMode];
	pBadBlockTable = (BYTE *) sInfo.bBadBlockTable[sInfo.bCurMode];	
	if(pBadBlockTable[wZoneNum]>=(MAX_BLOCK-MAX_LOGBLOCK))
	{
		MP_DEBUG1("%d Zone GetSMEmptyBlock fail",wZoneNum);
		return 0xffff;
	}
	wBlankAddr = pwLog2PhyTable[dwCurZone * MAX_BLOCK + pReAssignTable[wZoneNum] + MAX_LOGBLOCK];

	return (wBlankAddr);
}

static SWORD ReadRedtData(DWORD dwSectorAddr)
{
	MCARD *sMcard = (MCARD *)MCARD_BASE;
	SWORD swRetValue;
	WORD wNum;
	BYTE bTempMemID, *pbBuffer;
	
	SetControl();
	ChipEnable();
	WaitReady();


	SetCommand(READ_REDUNDANT_CMD);
	SetAddress(dwSectorAddr, 0);	
	if ((swRetValue = WaitReady()))
	{
		return swRetValue;
	}

	for (wNum = 0; wNum < 16; wNum++)
	{
		bRedtData[wNum] = sMcard->McSmDat;
	}
	sMcard->McSmC = 0x0;
	ChipDisable();

	return PASS;
}

static DWORD GetSameBlock(DWORD dwLogAddr, DWORD dwSectorCount)
{
	DWORD dwPage, dwCount;
	ST_MEDIA_MEAT *sMediaMeat;

	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];

	dwPage = dwLogAddr % sMediaMeat->wMaxSector;
	if (dwSectorCount > (sMediaMeat->wMaxSector - dwPage))
	{
		return (sMediaMeat->wMaxSector - dwPage);
	}
	return dwSectorCount;
}

static void Select(BYTE bMode)
{
	register GPIO *sGpio = (GPIO *) (GPIO_BASE);

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    ((CLOCK *)CLOCK_BASE)->Clkss_EXT2 &= ~BIT6;	// disable SPI flash function
#endif
#if SD_MMC_ENABLE
    SdProgChk();
#endif
	sInfo.bCurMode = bMode;
       // the register MC_ECC_CNFG must be clean to 0 when select xD card( or 1 bit ecc nand flash using old flow.).
	((McardEcc *)MC_ECC_BASE)->MC_ECC_CNFG = 0x00;
	switch (bMode)
	{
#if SM_ENABLE
	case INITIAL_SM:
		McardSelect(PIN_DEFINE_FOR_XD);
		sInfo.dwCurModeSetting = SETTING_SM;
		sInfo.dwReadTiming = READ_TIMING_SM;
		sInfo.dwWriteTiming = WRITE_TIMING_SM;
//		MP_DEBUG("-I- initial SM");
		break;
#endif

	case INITIAL_XD:
		McardSelect(PIN_DEFINE_FOR_XD);
		ChipEnable();
		En_WP();
		sInfo.dwCurModeSetting = SETTING_XD;
		sInfo.dwReadTiming = READ_TIMING_XD;
		sInfo.dwWriteTiming = WRITE_TIMING_XD;
		break;
	}
}

static void DeSelect(void)
{
	register GPIO *sGpio;
	register MCARD *sMcard;

	sMcard = (MCARD *) (MCARD_BASE);
	sGpio = (GPIO *) (GPIO_BASE);
	switch (sInfo.bCurMode)
	{
#if SM_ENABLE
	case INITIAL_SM:
		McardDeselect(PIN_DEFINE_FOR_XD);
		break;
#endif

	case INITIAL_XD:
		McardDeselect(PIN_DEFINE_FOR_XD);
		break;

	}
	sMcard->McardC = 0;
	sMcard->McSmC = 0;

}

static void Power_Off(void)
{
	McardPowerOff(DEV_XD);
}

static void Power_On(void)
{
	McardPowerOn(DEV_XD);
}

static void En_WP(void)
{
	((MCARD *)MCARD_BASE)->McSmC &= ~SM_WP;
}
static void Dis_WP(void)
{
	((MCARD *)MCARD_BASE)->McSmC |= SM_WP;
}

static void ChipEnable()
{
	((MCARD *)MCARD_BASE)->McSmC &= ~CE_MASK_HIGH;
}

static void ChipDisable()
{
	((MCARD *)MCARD_BASE)->McSmC |= CE_MASK_HIGH;
}

static void SetCommand(BYTE bCommand)
{
	register MCARD *sMcard;

	sMcard = (MCARD *) (MCARD_BASE);
	*((BYTE *) (&sMcard->McSmCmd)) = bCommand;
}

// XD Host Guildline V1.20 (Tcode 120), Page 23 Figure 4-9
DWORD CheckCISField(BYTE SectorPerBlk, BYTE AutoFix)
{
	DWORD i, nr = (MAX_BLOCK - MAX_LOGBLOCK) * SectorPerBlk;
	DWORD ret = FAIL;
	BYTE buffer[512];
	BYTE *buf = (BYTE *)((DWORD)buffer | 0xa0000000);

	MP_DEBUG("CIS checking...");
	memset(buf, 0, 512);
	for (i = 0 ; i < nr ; i+=SectorPerBlk)
	{
		if (ReadRedtData(i) == PASS)
		{
			if (CheckSatusFlag(bRedtData[5]) < 2)	// normal block
				break;
		}
	}
	if (i < nr)
	{
		DWORD x = i + 8;
		DWORD MaxLogicalAddr = sInfo.sMediaMeat[sInfo.bCurMode]->wMaxLogBlock;
		MP_DEBUG("\t1st vaild phy block found at %d", i/SectorPerBlk);
		for (; i < x ; i++)
		{
			if (ReadRedtData(i) != PASS)
				mpDebugPrint("Redundent data read failed!");
			else if (CheckSatusFlag(bRedtData[4]) >= 4)
				mpDebugPrint("Data fiailed! status flag=%x", bRedtData[4]);
			else if ((ReadLogicalBlockAddr(bRedtData, MaxLogicalAddr) != 0) && !AutoFix)
				mpDebugPrint("Block addr error: 0x%02x%02x,0x%02x%02x", bRedtData[6], bRedtData[7], bRedtData[11], bRedtData[12]);
			else
			{	// valid sector 0
				SWORD r;
				MP_DEBUG("\t1st vaild sector found at %d", i%SectorPerBlk);
				if ((r = PhysicalRead(i, 1, (DWORD)buf, NULL)) < 3)	// 0: sucess, 1: 0-256byte bad, 2: 256-512byte bad
				{
					BYTE *ptr = (r <= 1)?buf:&buf[256];
					MP_DEBUG("\t%s", (r==0)?"ECC ok!":((r==1)?"ECC2 failed!":"ECC1 failed!"));
					if (memcmp(ptr, bCISField, 10) != 0)
					{
						if (r == 0)
						{
							ptr = &buf[256];
							if (memcmp(ptr, bCISField, 10) == 0)
								ret = PASS;
						}
					}
					else
					{
						ret = PASS;
					}
					if (ret == PASS)
					{
						CIS_BlkAddr = i / SectorPerBlk;
						mpDebugPrint("XD CIS found");
						mpDebugPrint("\tManufacture: %s", &ptr[0x59]);
						mpDebugPrint("\tProduct: %s V%s", &ptr[0x61], &ptr[0x66]);
					}
					else
					{
						DWORD k;
						if (AutoFix == 1)
						{
							BYTE *dat_buf = (BYTE *)ext_mem_malloc(SectorPerBlk*512);
							BYTE *dat_status = (BYTE *)ext_mem_malloc(SectorPerBlk);
							mpDebugPrint("Try to fix CIS:");
							memcpy(ptr, bCISField, 10);
							if (PhysicalRead(i, SectorPerBlk, (DWORD)dat_buf, dat_status) != PASS)
								mpDebugPrint("CIS block Read failed!");
							memcpy(&dat_buf[(i%SectorPerBlk)*512], buffer, 512);
							PhysicalErase(i);
							PhysicalWrite(i, SectorPerBlk, (DWORD)dat_buf, 0, SFT_NORMAL, dat_status);
						}
						else
						{
							mpDebugPrint("CIS wrong:");
							UartOutText("000: ");
							for (k = 0 ; k < 256 ; k++)
							{
								UartOutValue(ptr[k], 2);
								PutUartChar(' ');
								if ((k & 0x0f) == 0xf)
								{
									UartOutText("\r\n");
									UartOutValue(k, 3);
									UartOutText(": ");
								}
							}
						}
					}
					break;
				}
			}
		}
	}
	if (ret && AutoFix)
	{
		BYTE *dat_buf = (BYTE *)ext_mem_malloc(SectorPerBlk*512);
		BYTE *dat_status = (BYTE *)ext_mem_malloc(SectorPerBlk);
		mpDebugPrint("Try to fix CIS:");
		if (PhysicalRead(0, SectorPerBlk, (DWORD)dat_buf, dat_status) != PASS)
			mpDebugPrint("CIS block Read failed!");
		memcpy(dat_buf, bCISField, 512);
		PhysicalErase(0);
		PhysicalWrite(0, SectorPerBlk, (DWORD)dat_buf, 0, SFT_NORMAL, NULL);
	}

	return ret;
}

static SWORD Identify(DWORD * pdwTotalSector)
{
	MCARD *sMcard = (MCARD *)MCARD_BASE;
	ST_MEDIA_MEAT_TABLE *sMediaMeatTable;
	SWORD swRetValue;
	BYTE bIDBuffer[4],DieNum,i;

	SetControl();
	WaitReady();
	CIS_BlkAddr = 0;
	BadBlockNr[sInfo.bCurMode] = 0;
	// XD Tcode 110
	if (sInfo.bCurMode == INITIAL_XD)
	{
		ChipEnable();
		McardIODelay(0x480); // 153ms
		SetCommand(RESET_CMD);
		McardIODelay(0x300);  //  68ms
		WaitReady();
		MLC[sInfo.bCurMode] = FALSE;
		ChipEnable();
		SetCommand(READ_ID3_CMD);
		sMcard->McSmAdr = 0x00;
		WaitReady();
		bIDBuffer[0] = sMcard->McSmDat;
		bIDBuffer[1] = sMcard->McSmDat;
		bIDBuffer[2] = sMcard->McSmDat;
		bIDBuffer[3] = sMcard->McSmDat;
		ChipDisable();
		if (bIDBuffer[2] != 0xB5)
		{
			mpDebugPrint("XD cmd-9A error:%x", bIDBuffer[2]);
			return FAIL;
		}
	}
	else
	{
		McardIODelay(0x500);
		ChipEnable();
		WaitReady();
		SetCommand(RESET_CMD);
		WaitReady();
	}
	ChipEnable();
	SetCommand(READ_ID_CMD);
	sMcard->McSmAdr = 0x00;
	WaitReady();
	bIDBuffer[0] = sMcard->McSmDat;
	bIDBuffer[1] = sMcard->McSmDat;
	bIDBuffer[2] = sMcard->McSmDat;
	bIDBuffer[3] = sMcard->McSmDat;
	sMcard->McSmC = 0x0;
	ChipDisable();
	MP_DEBUG4("-I- identify SM card Maker %x and Device code %x,3RD code %x , 4TH code %x", bIDBuffer[0], bIDBuffer[1],bIDBuffer[2],bIDBuffer[3]);

	swRetValue = TYPE_NOT_SUPPORT;
	// XD Host Guideline V1.20, Capacities table in Page 13...only 16MB~2GB should be recognized.
	sMediaMeatTable = (sInfo.bCurMode != INITIAL_XD) ? (ST_MEDIA_MEAT_TABLE *)sMediaMeatTable_S : (ST_MEDIA_MEAT_TABLE *)&sMediaMeatTable_S[2];
	for ( ; sMediaMeatTable->bID != 0 ; sMediaMeatTable++)
	{
		if (sMediaMeatTable->bID == bIDBuffer[1])
		{
			sInfo.sMediaMeat[sInfo.bCurMode] = &sMediaMeatTable->sMediaMeat;
			SetAddrInfo(0);
			swRetValue = PASS;
			break;
		}
	}

	if (sInfo.bCurMode != INITIAL_XD)
	{
		// swRetValue = TYPE_NOT_SUPPORT;  // for NAND      
		for (sMediaMeatTable = (ST_MEDIA_MEAT_TABLE *) sMediaMeatTable_B; sMediaMeatTable->bID != 0;
			 sMediaMeatTable++)
		{
			if (sMediaMeatTable->bID == bIDBuffer[1])
			{
				sInfo.sMediaMeat[sInfo.bCurMode] = &sMediaMeatTable->sMediaMeat;
				SetAddrInfo(1);
				swRetValue = PASS;
				break;
			}
		}
	}
	
	if(swRetValue != PASS){
		MP_DEBUG("TYPE - NOT - SUPPORT");
		return swRetValue;
	}

		
	*pdwTotalSector =
		(sInfo.sMediaMeat[sInfo.bCurMode]->bMaxZone *
		 sInfo.sMediaMeat[sInfo.bCurMode]->wMaxLogBlock *
		 sInfo.sMediaMeat[sInfo.bCurMode]->wMaxSector);

	if ((sInfo.bCurMode == INITIAL_XD) && (swRetValue == PASS))
		swRetValue = CheckCISField(sInfo.sMediaMeat[sInfo.bCurMode]->wMaxSector, 0);
	MP_DEBUG3("-I- Total %d(%x) sectors, %d KB", *pdwTotalSector, *pdwTotalSector,
			  (*pdwTotalSector >> 1));	
	return swRetValue;
}

static void SetAddrInfo(BYTE bType)
{
	DWORD dwSize;
	BYTE bTimes;

	if (bType == 0)
	{
		dwSize =
			(sInfo.sMediaMeat[sInfo.bCurMode]->bMaxZone *
			 sInfo.sMediaMeat[sInfo.bCurMode]->wMaxBlock *
			 sInfo.sMediaMeat[sInfo.bCurMode]->wMaxSector) - 1;
	}
	else
	{
		dwSize =
			((sInfo.sMediaMeat[sInfo.bCurMode]->bMaxZone *
			  sInfo.sMediaMeat[sInfo.bCurMode]->wMaxBlock *
			  sInfo.sMediaMeat[sInfo.bCurMode]->wMaxSector) >> 2) - 1;
	}

	bTimes = 0;
	while (dwSize)
	{
		dwSize = dwSize >> 8;
		bTimes += 1;
	}

	sInfo.bAddrInfo[sInfo.bCurMode][0] = bTimes;

	sInfo.bAddrInfo[sInfo.bCurMode][2] = ADR_LENGTH_3;
	if (bTimes == 4)
	{
		sInfo.bAddrInfo[sInfo.bCurMode][2] = ADR_LENGTH_4;
	}
	else if (bTimes >= 5)
	{
		sInfo.bAddrInfo[sInfo.bCurMode][2] = ADR_LENGTH_5;
	}

	if (bType == 0)
	{
		bTimes += 1;
	}
	else
	{
		bTimes += 2;
	}
	sInfo.bAddrInfo[sInfo.bCurMode][1] = ADR_LENGTH_3;
	if (bTimes == 4)
	{
		sInfo.bAddrInfo[sInfo.bCurMode][1] = ADR_LENGTH_4;
	}
	else if (bTimes >= 5)
	{
		sInfo.bAddrInfo[sInfo.bCurMode][1] = ADR_LENGTH_5;
	}
}

static void SetControl(void)
{
	register MCARD *sMcard = (MCARD *)MCARD_BASE;

	sMcard->McardC = 0;
	sMcard->McardC = (MCARD_FLASH_ENABLE | MCARD_DMA_SM);
	sMcard->McRtm = sInfo.dwReadTiming;
	sMcard->McWtm = sInfo.dwWriteTiming;
	sMcard->McSmIc = IM_ALL;
	sMcard->McSmC = sInfo.dwCurModeSetting;
	sMcard->McEcc4SC = 0;
}


//==========================================================================
static inline BOOL Polling_SM_Status(void)
{
	if (sInfo.dwCurModeSetting == SETTING_SM)
		return !Mcard_GetDetected(DEV_SM);
	else if (sInfo.dwCurModeSetting == SETTING_XD)
		return !Mcard_GetDetected(DEV_XD);
	else
		return 0;
}

static SWORD WaitReady(void)
{
	register MCARD *sMcard;
	DWORD dwTimeCount;

	sMcard = (MCARD *) (MCARD_BASE);
	dwTimeCount = (g_bAniFlag & ANI_VIDEO)? (TIMEOUT_COUNT>>3):TIMEOUT_COUNT;

	while (dwTimeCount)
	{
		if (Polling_SM_Status())
			return FAIL;

		if (!(sMcard->McSmIc & IC_SRB))
		{
			return PASS;
		}
		dwTimeCount--;
//		McardIODelay(5);
		TASK_YIELD();
	}
	mpDebugPrint("WaitReady timeout!");
	return TIMEOUT;
}

static void SetAddress(DWORD PhyAddr, BYTE bMode)
{
	register MCARD *sMcard;
	DWORD dwTemp, i, dwPhyAddr;
	
	dwPhyAddr = PhyAddr;
	
	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McSmC &= ~(0x07 << 5);
	switch (bMode)
	{
		case 0:
		default:
			sMcard->McSmC |= sInfo.bAddrInfo[sInfo.bCurMode][1];
			dwTemp = dwPhyAddr;	
			sMcard->McSmAdr = 0x00;	// column address
			break;
		case 1:
			dwTemp = dwPhyAddr;
			sMcard->McSmC |= sInfo.bAddrInfo[sInfo.bCurMode][2];
			break;
		case 2:
			sMcard->McSmC |= sInfo.bAddrInfo[sInfo.bCurMode][1];
			break;	
	}

	
	for (i = 0; i < sInfo.bAddrInfo[sInfo.bCurMode][0]; i++)
	{	
		sMcard->McSmAdr = (BYTE)(dwTemp & 0xff);	
		dwTemp = dwTemp >> 8;
	}
}

static BOOL CheckBadBlockTable(WORD wCurPhyBlock, WORD wZoneNum)
{
	BYTE bBBNo;	
	BYTE *pBadBlockTable;
	WORD *pwLog2PhyTable;
	DWORD dwPhyAddr,dwZone,dwEZone,dwLogBlock;
	ST_MEDIA_MEAT *sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];
	pBadBlockTable = (BYTE *) sInfo.bBadBlockTable[sInfo.bCurMode];	
	bBBNo = pBadBlockTable[wZoneNum];
	
	if(wZoneNum >= XD_STA_ZONE)
	{		
		if((sInfo.dwXdCurZone > wZoneNum) ||((sInfo.dwXdCurZone+XD_DNY_ZONE-1) < wZoneNum) \
			||(sInfo.dwXdCurZone ==0))
		{
			sInfo.dwXdCurZone = (BYTE)wZoneNum;
			dwEZone = wZoneNum+XD_DNY_ZONE;
			if(dwEZone > sMediaMeat->bMaxZone)
				dwEZone = sMediaMeat->bMaxZone; 
			MP_DEBUG2("Read Dny table from zone %d to zone %d ",wZoneNum,dwEZone);
			Log2PhyTabInit((BYTE)wZoneNum, (BYTE)dwEZone);
		}
		pwLog2PhyTable = (WORD *) sInfo.wXdDLog2PhyTable;
	}
	else
	{
		pwLog2PhyTable = (WORD *) sInfo.wXdSLog2PhyTable;
	}

	if(wZoneNum >=XD_STA_ZONE)
		wZoneNum = (wZoneNum-sInfo.dwXdCurZone);
	while(bBBNo)
	{
		if(wCurPhyBlock == pwLog2PhyTable[(wZoneNum+1) * MAX_BLOCK - bBBNo])
			return UNUSABLE_BLOCK;
		
		bBBNo--;
	};
	return USABLE_BLOCK;
}


static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	DWORD dwCount;
	SWORD swRetValue;
	BYTE bRetry;
    DWORD dwDataFlag;
	
	MP_DEBUG2("-I- xD FlatRead  ,dwLogAddr %d,dwSectorCount %d",dwLogAddr,dwSectorCount);	
	while (dwSectorCount)
	{
		bRetry = MCARD_RETRY_TIME;
		dwCount = GetSameBlock(dwLogAddr, dwSectorCount);
		while (bRetry)
		{
			if (Polling_SM_Status())
				return FAIL;
			if (!(swRetValue = LogicalRead(dwLogAddr, dwCount, dwBufferAddress)))
			{
				break;
			}
			bRetry--;
			MP_DEBUG1("-I- remain retry times %d", bRetry);
		}
		if (swRetValue)
		{
			return swRetValue;
		}
		dwSectorCount -= dwCount;
		dwLogAddr += dwCount;
		dwBufferAddress += (dwCount << MCARD_SECTOR_SIZE_EXP);
	}
	//DeSelect();
	return PASS;
}

static SWORD LogicalRead(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress)
{
	ST_MEDIA_MEAT *sMediaMeat;
	DWORD dwPhyBlock, dwLogBlock, dwZone, dwPhyAddr, dwPhyPage;
	SWORD swRetValue;
	WORD *pwLog2PhyTable;
	DWORD dwEZone;

	if (!dwSectorCount)
	{
		MP_DEBUG1("-E- dwSectorCount %d", dwSectorCount);
		return FAIL;
	}
	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];


	dwPhyPage = dwLogAddr % sMediaMeat->wMaxSector;
	dwPhyBlock = dwLogAddr / sMediaMeat->wMaxSector;
	dwZone = dwPhyBlock / sMediaMeat->wMaxLogBlock;
	dwLogBlock = dwPhyBlock % sMediaMeat->wMaxLogBlock;
	if(dwZone >= XD_STA_ZONE)
	{		
		if((sInfo.dwXdCurZone > dwZone) ||((sInfo.dwXdCurZone+XD_DNY_ZONE-1) < dwZone) \
			||(sInfo.dwXdCurZone ==0))
		{
			sInfo.dwXdCurZone = (BYTE)dwZone;
			dwEZone = dwZone+XD_DNY_ZONE;
			if(dwEZone > sMediaMeat->bMaxZone)
				dwEZone = sMediaMeat->bMaxZone; 
			MP_DEBUG2("Read Dny table from zone %d to zone %d ",dwZone,dwEZone);
			Log2PhyTabInit(dwZone, dwEZone);
		}
		pwLog2PhyTable = (WORD *) sInfo.wXdDLog2PhyTable;
		dwPhyAddr = (((dwZone-XD_STA_ZONE) / XD_DNY_ZONE)*XD_DNY_ZONE*MAX_BLOCK);
		dwPhyAddr += (((dwZone-XD_STA_ZONE) % XD_DNY_ZONE)*MAX_BLOCK);
		dwPhyAddr += XD_STA_ZONE*MAX_BLOCK;
//			dwPhyAddr = dwZone*MAX_BLOCK;
		dwPhyAddr+=pwLog2PhyTable[((dwZone-sInfo.dwXdCurZone)%XD_DNY_ZONE) * MAX_BLOCK + dwLogBlock];
	}
	else
	{
		pwLog2PhyTable = (WORD *) sInfo.wXdSLog2PhyTable;
		dwPhyAddr = (dwZone *MAX_BLOCK);
		dwPhyAddr+=pwLog2PhyTable[dwZone * MAX_BLOCK + dwLogBlock];	
	}
	dwPhyAddr = (dwPhyAddr * sMediaMeat->wMaxSector) + dwPhyPage;	// get physical sector address;
	if ((swRetValue = PhysicalRead(dwPhyAddr, dwSectorCount, dwBufferAddress, NULL)))
	{
		MP_DEBUG1("-E- SmPhysicalRead FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	return PASS;
}

static SWORD PhysicalRead(DWORD dwPhyAddr, DWORD dwSectorCount, DWORD dwBufferAddress, BYTE *SectorStatus)
{
	MCARD *sMcard = (MCARD *)MCARD_BASE;
	SWORD swRetValue, ret = PASS;
	DWORD i, dwTempAddr;
	BYTE *pbBuffer;
	BYTE bp=0;
	WORD wCount =0,wCount1;
	BYTE bFlag =0;
	WORD wLogBlockAddr1,wLogBlockAddr2;
	pbBuffer = (BYTE *) dwBufferAddress;
	
	SetControl();
	ChipEnable();
	while (dwSectorCount)
	{
		WaitReady();
		SetCommand(READ_PAGE1_CMD);
		SetAddress(dwPhyAddr, 0);
//			if(sInfo.bCurMode == INITIAL_XD)
//				McardIODelay(20);		// abel 20070706 for UNI-V KinSton CF + XD issue		
//mark the delay will increase read speed about 800KB in XD 2G H Type
		if ((swRetValue = WaitReady()))
		{
			MP_DEBUG1("-E- Read Command FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if ((swRetValue = ReadSector((DWORD) pbBuffer, MCARD_SECTOR_SIZE)))
		{
			MP_DEBUG1("-E- DMA Read FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if (dwSectorCount != 0)
		{
			for (i = 0; i < 0x10; i++)
			{
				bRedtData[i] = sMcard->McSmDat;
			}
		}
		if (CheckSatusFlag(bRedtData[5]) >= 2)
		{
			mpDebugPrint("-E- Block Status error!");
			if (!ret)
				ret = FAIL;
		}
		if (CheckSatusFlag(bRedtData[4]) >= 4)	// Tcode 300
		{
			if (SectorStatus)
				SectorStatus[wCount] = 1;
			mpDebugPrint("-E- Data Status error!");
			if (!ret)
				ret = FAIL;
		}
		DWORD ecc_errno;
		if ((ecc_errno = ECC_Check((DWORD)pbBuffer,bFlag)) != PASS)
		{
			if (SectorStatus)
				SectorStatus[wCount] = 1;
			mpDebugPrint("-E- ECC uncorrectable!");
			ret = ecc_errno;
		}
		if (ret)
		{
			MemDump(pbBuffer, 512);
			MemDump(bRedtData, 16);
		}

		pbBuffer += MCARD_SECTOR_SIZE;
		dwSectorCount--;
		wCount++;
		dwPhyAddr++;
	}
	ChipDisable();
	return ret;
}

static SWORD BadBlockMangment(DWORD* pdwCurPhyBlock,DWORD dwLogBlock,DWORD dwZoneNum)
{
	ST_MEDIA_MEAT *sMediaMeat;
	WORD *pLog2PhyTable;
	BYTE *pReAssignTable;
	BYTE *pBadBlockTable;
	BYTE bReUseIndex;
	BYTE *pbBuffer;
	BYTE bTempMemID;
	DWORD dwEZone;
	BYTE bCount;
	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];

	pReAssignTable = (BYTE *) sInfo.bReAssignTable[sInfo.bCurMode];
	pBadBlockTable = (BYTE *) sInfo.bBadBlockTable[sInfo.bCurMode];

	if(dwZoneNum >= XD_STA_ZONE)
	{		
		if((sInfo.dwXdCurZone > dwZoneNum) ||((sInfo.dwXdCurZone+XD_DNY_ZONE-1) < dwZoneNum) \
			||(sInfo.dwXdCurZone ==0))
		{
			sInfo.dwXdCurZone = (BYTE)dwZoneNum;
			dwEZone = dwZoneNum+XD_DNY_ZONE;
			if(dwEZone > sMediaMeat->bMaxZone)
				dwEZone = sMediaMeat->bMaxZone; 
			MP_DEBUG2("Read Dny table from zone %d to zone %d ",dwZoneNum,dwEZone);
			Log2PhyTabInit((BYTE)dwZoneNum, (BYTE)dwEZone);
		}
		pLog2PhyTable = (WORD *) sInfo.wXdDLog2PhyTable;
	}
	else
	{
		pLog2PhyTable = (WORD *) sInfo.wXdSLog2PhyTable;
	}		
	
	pbBuffer = (BYTE *) ext_mem_malloc(sMediaMeat->wMaxSector << 9);
	memset(pbBuffer,0x5a,sMediaMeat->wMaxSector << 9);

	PhysicalErase((dwZoneNum*MAX_BLOCK+(*pdwCurPhyBlock)) * sMediaMeat->wMaxSector);
	PhysicalWrite(((dwZoneNum*MAX_BLOCK+(*pdwCurPhyBlock)) * sMediaMeat->wMaxSector), sMediaMeat->wMaxSector,(DWORD) pbBuffer, dwLogBlock,SFT_BAD_BLOCK, NULL);
//	memset(pbBuffer,0x00,0x40000);
//	PhysicalRead(((dwZoneNum*MAX_BLOCK+(*pdwCurPhyBlock)) * sMediaMeat->wMaxSector), sMediaMeat->wMaxSector, (DWORD) pbBuffer);	


	bReUseIndex= pReAssignTable[dwZoneNum];	
	bCount =(MAX_BLOCK-MAX_LOGBLOCK)-bReUseIndex-pBadBlockTable[dwZoneNum]-1; 
	pBadBlockTable[dwZoneNum]++;
	if (pBadBlockTable[dwZoneNum] > BadBlockNr[sInfo.bCurMode])
		BadBlockNr[sInfo.bCurMode] = pBadBlockTable[dwZoneNum];
	if(pBadBlockTable[dwZoneNum]>=(MAX_BLOCK-MAX_LOGBLOCK))
	{	
	    ext_mem_free(pbBuffer);
		MP_DEBUG1("%d Zone recursive Physical Erase fail",dwZoneNum);
		return FAIL;
	}
	if(dwZoneNum <XD_STA_ZONE)
		dwEZone = dwZoneNum;
	else
		dwEZone = (dwZoneNum-sInfo.dwXdCurZone);
	while(bCount)
	{
		pLog2PhyTable[dwEZone * MAX_BLOCK + bReUseIndex+ MAX_LOGBLOCK] =
			pLog2PhyTable[dwEZone * MAX_BLOCK + (bReUseIndex+1)+ MAX_LOGBLOCK];
		bReUseIndex++;
		bCount --;
	};
	pLog2PhyTable[(dwEZone+1) * MAX_BLOCK - pBadBlockTable[dwZoneNum]] = *pdwCurPhyBlock;			

	ext_mem_free(pbBuffer);
	return PASS;
}
static SWORD ReadSector(DWORD dwBufferAddress, WORD wSize)
{
	CHANNEL *sChannel = (CHANNEL *) (DMA_MC_BASE);
	MCARD *pstMcard = (MCARD *)MCARD_BASE;
	DWORD dwTimeOutCount;
	DWORD ICmask = IC_SFTCE | IC_ECB;

	if (dwBufferAddress & 0x3)
	{
		mpDebugPrint("-E- target buffer must align to 4 bytes boundary !");
		return FAIL;
	}
	pstMcard->McSmC &=~ ECC_ENABLE;
	pstMcard->McSmC |= ECC_ENABLE;

	sChannel->Control = 0x0;
	sChannel->StartA = dwBufferAddress;
	sChannel->EndA = dwBufferAddress + wSize - 1;
	pstMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE + (wSize >> 2);
	pstMcard->McardC = ((pstMcard->McardC & 0xffffffef) | MCARD_DMA_DIR_CM | MCARD_FIFO_ENABLE);
	sChannel->Control = MCARD_DMA_ENABLE;

	dwTimeOutCount = 0x7fffff;
	while((pstMcard->McSmIc & ICmask) != ICmask)      // Mcard FIFO transfer ok
	{
		if(dwTimeOutCount-- == 0)
		{
			mpDebugPrint("-E- Mcard FIFO read timeout %x\n", pstMcard->McSmIc);
			return FAIL;
		}

	}

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

#define  Erase_Free_Block	0
#define Erase_Used_Block	1
static SWORD RecursivePhysicalErase(DWORD* pdwCurPhyBlock,DWORD dwLogBlock,DWORD dwSZone,DWORD dwZoneNum,WORD *pwLog2PhyTable)
{
	ST_MEDIA_MEAT *sMediaMeat;
	WORD *pLog2PhyTable;
	BYTE *pReAssignTable;
	BYTE *pBadBlockTable;
	BYTE bReUseIndex;
	BOOL bTest;
	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];
	pLog2PhyTable = (WORD *)pwLog2PhyTable;
	pReAssignTable = (BYTE *) sInfo.bReAssignTable[sInfo.bCurMode];
	pBadBlockTable = (BYTE *) sInfo.bBadBlockTable[sInfo.bCurMode];
	while(PhysicalErase(((dwZoneNum*MAX_BLOCK+(*pdwCurPhyBlock)) * sMediaMeat->wMaxSector)) != PASS)
	{
		mpDebugPrint("RecursivePhysicalErase PhysicalErase fail");

		if (Polling_SM_Status())
			return FAIL;

		if(BadBlockMangment((DWORD *)pdwCurPhyBlock, dwLogBlock, dwZoneNum) != PASS)
		{
			mpDebugPrint("BadBlockMangment fail");			
			return FAIL;
		}
//		pReAssignTable[dwZoneNum] ++;
		if(pReAssignTable[dwZoneNum]>=(MAX_BLOCK-MAX_LOGBLOCK-pBadBlockTable[dwZoneNum]))
			pReAssignTable[dwZoneNum] = 0;			
		*pdwCurPhyBlock = GetSMEmptyBlock(dwSZone,dwZoneNum,pwLog2PhyTable);	
	};	
	return PASS;
}
static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	DWORD dwCount;
	DWORD i,dwVeryCount,dwVeryLogAddr;
	SWORD swRetValue;
	BYTE bRetry;
	DWORD dwVBufferAddress1,dwVCount,dwVSectorCount,dwVLogAddr,dwIndex,dwWrongCount;
	BYTE * pbVTempBuffer;
	BYTE *pbSTempBuffer;
	BYTE *pbSTempBuffer1;
	WORD wLogBlockAddr;
	dwVLogAddr = dwLogAddr;
	ST_MEDIA_MEAT *sMediaMeat;
	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];	
	MP_DEBUG2("-I- xD FlatWrite  ,dwLogAddr %d,dwSectorCount %d",dwLogAddr,dwSectorCount);
	while (dwSectorCount)
	{
		bRetry = MCARD_RETRY_TIME;
		dwCount = GetSameBlock(dwLogAddr, dwSectorCount);
		while (bRetry)
		{
			if (Polling_SM_Status())
				return FAIL;
			if (!(swRetValue = LogicalWrite(dwLogAddr, dwCount, dwBufferAddress)))
			{
				break;
			}
			bRetry--;
			MP_DEBUG1("-I- remain retry times %d", bRetry);
		}
		#if WRITE_VERIFY
		{
			DWORD size = dwSectorCount << MCARD_SECTOR_SIZE_EXP;
			DWORD *buf = (DWORD *)((DWORD)ext_mem_malloc(size) | 0xA0000000);
			DWORD *target = (DWORD *)dwBufferAddress;

			if (buf)
			{
				DWORD i;
				FlatRead((DWORD)buf, dwSectorCount, dwLogAddr);
				for (i = 0 ; i < (size >> 2) ; i++)
				{
					if (buf[i] != target[i])
					{
						mpDebugPrint("\r\nSector %d write verify failed!!!\r\nSource(%x):", dwLogAddr, target);
						//MemDump(target, size);
						mpDebugPrint("\r\nDestination(%x):", buf);
						//MemDump(buf, size);
						break;
					}
				}
				ext_mem_free(buf);
			}
		}
		#endif				
		if (swRetValue)
		{
			return swRetValue;
		}
		dwSectorCount -= dwCount;
		dwLogAddr += dwCount;
		dwBufferAddress += (dwCount << MCARD_SECTOR_SIZE_EXP);
	}
	return PASS;
}

static SWORD LogicalWrite(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress)
{
	ST_MEDIA_MEAT *sMediaMeat;
	DWORD dwNewPhyAddr, dwBlock, dwLogBlock, dwZone, dwPhyBlockAddr,dwOldPhyBlockAddr, dwPhyAddr, dwPage, dwTail, i;
	SWORD swRetValue;
	WORD *pwLog2PhyTable;
	BYTE *pbTempBuffer, *pbBuffer, *pReAssignTable,*pbTempBuffer1;
	DWORD dwEZone,dwSZone;
	DWORD dwEmptyBlockOffset;
	BYTE *pBadBlockTable;
	BYTE *SectStatus;
	pbTempBuffer1 = (BYTE *)0xffffffff;
	BYTE bXd =0;
	if (!dwSectorCount)
	{
		MP_DEBUG1("-E- dwSectorCount %d", dwSectorCount);
		return FAIL;
	}
	
	pbBuffer = (BYTE *) dwBufferAddress;
	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];
	pReAssignTable = (BYTE *) sInfo.bReAssignTable[sInfo.bCurMode];
	pBadBlockTable = (BYTE *) sInfo.bBadBlockTable[sInfo.bCurMode];
	
	dwPage = dwLogAddr % sMediaMeat->wMaxSector;
	dwBlock = dwLogAddr / sMediaMeat->wMaxSector;
	dwZone = dwBlock / sMediaMeat->wMaxLogBlock;
	dwLogBlock = dwBlock % sMediaMeat->wMaxLogBlock;

	SectStatus = (BYTE *)ext_mem_malloc(sMediaMeat->wMaxSector);
	memset(SectStatus, 0, sMediaMeat->wMaxSector);

	if(dwZone >= XD_STA_ZONE)
	{		
		if((sInfo.dwXdCurZone > dwZone) ||((sInfo.dwXdCurZone+XD_DNY_ZONE-1) < dwZone) \
			||(sInfo.dwXdCurZone ==0))
		{
			sInfo.dwXdCurZone = (BYTE)dwZone;
			dwEZone = dwZone+XD_DNY_ZONE;
			if(dwEZone > sMediaMeat->bMaxZone)
				dwEZone = sMediaMeat->bMaxZone; 
			MP_DEBUG2("Write Dny table from zone %d to zone %d ",dwZone,dwEZone);			
			Log2PhyTabInit(dwZone, dwEZone);
			bXd = 1;
		}
		pwLog2PhyTable = (WORD *) sInfo.wXdDLog2PhyTable;
		dwPhyBlockAddr = dwZone*MAX_BLOCK;
		dwSZone = dwZone-sInfo.dwXdCurZone;
		dwOldPhyBlockAddr=pwLog2PhyTable[(dwSZone%XD_DNY_ZONE) * MAX_BLOCK + dwLogBlock];
		dwPhyBlockAddr += dwOldPhyBlockAddr;
	}
	else
	{
		pwLog2PhyTable = (WORD *) sInfo.wXdSLog2PhyTable;
		dwPhyBlockAddr = (dwZone *MAX_BLOCK);
		dwSZone = dwZone;
		dwOldPhyBlockAddr=pwLog2PhyTable[dwZone * MAX_BLOCK + dwLogBlock];	
		dwPhyBlockAddr += dwOldPhyBlockAddr;
	}
	dwPhyAddr = (dwPhyBlockAddr * sMediaMeat->wMaxSector) + dwPage;	// get physical sector address;
	dwTail = sMediaMeat->wMaxSector - dwPage - dwSectorCount;
	if ((dwPage == 0) && (dwTail == 0))
	{
		pbTempBuffer = (BYTE *) dwBufferAddress;
	}
	else
	{
		pbTempBuffer1 = (BYTE *)((DWORD)ext_mem_malloc(sMediaMeat->wMaxSector << 9) | 0xa0000000);	
		pbTempBuffer = pbTempBuffer1;
		
		if (dwPage)
		{
			PhysicalRead((dwPhyAddr - dwPage), dwPage, (DWORD) pbTempBuffer, SectStatus);	// read 0~dwPage
			pbTempBuffer += (dwPage << MCARD_SECTOR_SIZE_EXP);
		}
		for (i = 0; i < (dwSectorCount << MCARD_SECTOR_SIZE_EXP); i++)
		{
			*pbTempBuffer++ = *pbBuffer++;
		}
		if (dwTail)
		{
			PhysicalRead((dwPhyAddr + dwSectorCount), dwTail, (DWORD) pbTempBuffer, &SectStatus[sMediaMeat->wMaxSector-dwTail]); // read (dwPage+dwSectorCount)~sMediaMeat->wMaxSector
		}
		pbTempBuffer = pbTempBuffer1;
	}

	dwNewPhyAddr = GetSMEmptyBlock(dwSZone,dwZone,pwLog2PhyTable);
	if (dwNewPhyAddr == 0xffff)
	{
		pReAssignTable[dwZone] = 0;
		dwNewPhyAddr = GetSMEmptyBlock(dwSZone,dwZone,pwLog2PhyTable);

		if (dwNewPhyAddr == 0xffff)
		{
			if((DWORD)pbTempBuffer1 != 0xffffffff)
				ext_mem_free(pbTempBuffer1);
			if (SectStatus)
				ext_mem_free(SectStatus);
				
			MP_DEBUG("-E- Logical Write no free block");
			return FAIL;
		}
	}
	if(swRetValue = RecursivePhysicalErase((DWORD *)&dwNewPhyAddr,dwLogBlock,dwSZone,dwZone,(WORD *)pwLog2PhyTable))
	{
		if((DWORD)pbTempBuffer1 != 0xffffffff)
			ext_mem_free(pbTempBuffer1);
		if (SectStatus)
			ext_mem_free(SectStatus);
			
		return swRetValue;
	}
	if (swRetValue =RecursivePhysicalWrite((DWORD *)&dwNewPhyAddr , sMediaMeat->wMaxSector,(DWORD) pbTempBuffer, dwBlock,dwSZone,dwZone,(WORD *)pwLog2PhyTable,bXd, SectStatus))
	{
		if((DWORD)pbTempBuffer1 != 0xffffffff)
			ext_mem_free(pbTempBuffer1);
		if (SectStatus)
			ext_mem_free(SectStatus);
			
		return swRetValue;
	}	
	bXd =0;
	if(swRetValue = RecursivePhysicalErase((DWORD *)&dwOldPhyBlockAddr,dwLogBlock,dwSZone,dwZone,(WORD *)pwLog2PhyTable))
	{
		if((DWORD)pbTempBuffer1 != 0xffffffff)	
			ext_mem_free(pbTempBuffer1);
		if (SectStatus)
			ext_mem_free(SectStatus);
			
		return swRetValue;
	}
	
	pwLog2PhyTable[dwSZone * MAX_BLOCK + dwLogBlock] = \
		(WORD) (dwNewPhyAddr % MAX_BLOCK);
	pwLog2PhyTable[dwSZone * MAX_BLOCK + pReAssignTable[dwZone] + MAX_LOGBLOCK] =\
		(WORD) (dwPhyBlockAddr % MAX_BLOCK);

	pReAssignTable[dwZone] ++;
	if(pReAssignTable[dwZone]>=(MAX_BLOCK-MAX_LOGBLOCK-pBadBlockTable[dwZone]))
		pReAssignTable[dwZone] = 0;	

	if((DWORD)pbTempBuffer1 != 0xffffffff)
	{	
		ext_mem_free(pbTempBuffer1);
		
	}
	if (SectStatus)
		ext_mem_free(SectStatus);
	return PASS;
}
static SWORD RecursivePhysicalWrite(DWORD *pdwPhyBlock, DWORD dwSectorCount, DWORD dwBufferAddress,
						   DWORD dwLogAddr,DWORD dwSZone,DWORD dwZoneNum,WORD *pwLog2PhyTable,BYTE bXd, BYTE *SectorStatus)
{
	ST_MEDIA_MEAT *sMediaMeat;
	WORD *pLog2PhyTable;
	BYTE *pReAssignTable;
	BYTE *pBadBlockTable;
	BYTE bReUseIndex;
	BYTE *pbTempBuffer,*pbBuffer;

	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];
	pLog2PhyTable = (WORD *) pwLog2PhyTable;

	pReAssignTable = (BYTE *) sInfo.bReAssignTable[sInfo.bCurMode];
	pBadBlockTable = (BYTE *) sInfo.bBadBlockTable[sInfo.bCurMode];
	pbTempBuffer = (BYTE *)dwBufferAddress;
	if(bXd ==1)
	{
	    
		pbBuffer = (BYTE *)((DWORD)ext_mem_malloc((sMediaMeat->wMaxSector) << 9) | 0xa0000000);	
		PhysicalRead(((dwZoneNum*MAX_BLOCK+*pdwPhyBlock) * sMediaMeat->wMaxSector), sMediaMeat->wMaxSector,(DWORD) pbBuffer, NULL);
		ext_mem_free(pbBuffer);
		
	}
	while(PhysicalWrite(((dwZoneNum*MAX_BLOCK+(*pdwPhyBlock)) * sMediaMeat->wMaxSector), sMediaMeat->wMaxSector,(DWORD) pbTempBuffer, dwLogAddr,SFT_NORMAL, SectorStatus) != PASS)
	{
		mpDebugPrint("%dZone,%d Block RecursivePhysicalWrite PhysicalWrite fail",dwZoneNum,*pdwPhyBlock);	
		if (Polling_SM_Status())
			return FAIL;
		if(BadBlockMangment(pdwPhyBlock, dwLogAddr, dwZoneNum) != PASS)
		{
			mpDebugPrint("BadBlockMangment fail");	
			return FAIL;
		}
//		pReAssignTable[dwZoneNum] ++;
		if(pReAssignTable[dwZoneNum]>=(MAX_BLOCK-MAX_LOGBLOCK-pBadBlockTable[dwZoneNum]))
			pReAssignTable[dwZoneNum] = 0;				
		*pdwPhyBlock = GetSMEmptyBlock(dwSZone,dwZoneNum,pwLog2PhyTable);		
	};	
	return PASS;
}
static SWORD PhysicalWrite(DWORD dwPhyAddr, DWORD dwSectorCount, DWORD dwBufferAddress,
						   DWORD dwLogAddr,BYTE bValid, BYTE *SectorStatus)
{
	MCARD *sMcard = (MCARD *)MCARD_BASE;
	SWORD swRetValue;
	BYTE i;
	BYTE *pbBuffer;
	BYTE bCount = 0;

	ST_MEDIA_MEAT *sMediaMeat;	
	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];

	if(dwPhyAddr >= (sMediaMeat->bMaxZone*sMediaMeat->wMaxSector*sMediaMeat->wMaxBlock))		// abel add 20080221
		return OUT_OF_RANGE;

//	MP_DEBUG2("-I- SM PhysicalWrite  ,dwPhyAddr %d,dwSectorCount %d",dwPhyAddr,dwSectorCount);
	pbBuffer = (BYTE *) dwBufferAddress;
	while (dwSectorCount)
	{
		if (Polling_SM_Status())
			return FAIL;

		WaitReady();
		SetControl();
	    Dis_WP();
		ChipEnable();
		SetCommand(SEQ_DATA_IN_CMD);
		SetAddress(dwPhyAddr, 0);
		if ((swRetValue = WriteSector((DWORD) pbBuffer, MCARD_SECTOR_SIZE)))
		{
			MP_DEBUG1("-E- SmWriteSector FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if (SectorStatus && SectorStatus[bCount] != 0)
			BuildRedtData(dwLogAddr, SFT_DATA_FAIL);
		else
			BuildRedtData(dwLogAddr, bValid);
		
		for (i = 0; i < 16; i++)
			sMcard->McSmDat = bRedtData[i];

		pbBuffer += MCARD_SECTOR_SIZE;
		dwSectorCount--;
		dwPhyAddr++;
		bCount++;
		
		SetCommand(PAGE_PROG_CMD);
		if ((swRetValue = WaitReady()))
		{
			MP_DEBUG1("-E- WaitReady FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
	    En_WP();
		if ((swRetValue = ReadStatus()))
		{
			MP_DEBUG1("-E- write SmReadStatus FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		ChipDisable();	
	}
	
	return PASS;
}

static SWORD WriteSector(DWORD dwBufferAddress, WORD wSize)
{
	CHANNEL *sChannel = (CHANNEL *)DMA_MC_BASE;
	MCARD *pstMcard = (MCARD *)MCARD_BASE;
	DWORD dwTimeOutCount;
	DWORD ICmask = IC_SFTCE | IC_ECB;
	

	if ((DWORD)dwBufferAddress & 0x3)
	{
		MP_DEBUG("-E- target buffer must align to 4 bytes boundary !");
		return FAIL;
	}
	pstMcard->McSmC &=~ ECC_ENABLE;
	pstMcard->McSmC |= ECC_ENABLE;

	sChannel->Control = 0x0;
	sChannel->StartA = (DWORD)dwBufferAddress;
	sChannel->EndA = (DWORD)dwBufferAddress + wSize - 1;
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

    dwTimeOutCount = 0x000fffff;
    while((pstMcard->McSmIc & ICmask) != ICmask)      // Mcard FIFO transfer ok
    {
        if(dwTimeOutCount-- == 0)
        {
            mpDebugPrint("-E- Mcard FIFO write timeout %x\n", pstMcard->McSmIc);
            return FAIL;
        }
    }

	return PASS;
}

static SWORD PhysicalErase(DWORD dwPhyAddr)
{
	SWORD swRetValue = PASS;
	ST_MEDIA_MEAT *sMediaMeat;	
	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];
	
	if(dwPhyAddr >= (sMediaMeat->bMaxZone*sMediaMeat->wMaxSector*sMediaMeat->wMaxBlock))		// abel add 20080221
		return OUT_OF_RANGE;	
	
	//MP_DEBUG1("-I- SM PhysicalErase  ,dwPhyAddr %d",dwPhyAddr);
	SetControl();
	Dis_WP();
	ChipEnable();
	WaitReady();
	SetCommand(BLOCK_ERASE_CMD_1CYC);
	SetAddress(dwPhyAddr, 1);
	SetCommand(BLOCK_ERASE_CMD_2CYC);
	if ((swRetValue = WaitReady()))
	{
		MP_DEBUG1("-E- WaitReady FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
    En_WP();
	if ((swRetValue = ReadStatus()))
		MP_DEBUG1("-E- %d block erase SmReadStatus FAIL %d",(dwPhyAddr/sMediaMeat->wMaxSector));
	ChipDisable();
		
	return swRetValue;
}

static SWORD ReadStatus(void)
{
	register MCARD *sMcard = (MCARD *)MCARD_BASE;
	DWORD dwTimeOutCount;
	BYTE bStatus;

	dwTimeOutCount = (g_bAniFlag & ANI_VIDEO)? (TIMEOUT_COUNT>>3):TIMEOUT_COUNT;
	do
	{
		if (Polling_SM_Status())
			return FAIL;
        
		if (!dwTimeOutCount)
		{
			MP_DEBUG1("-E- Read Status TimeOut (status: %x)", bStatus);
			return TIMEOUT;
		}
		SetCommand(READ_STATUS);
		dwTimeOutCount--;
		bStatus = sMcard->McSmDat;
		TASK_YIELD();
	} while (!(bStatus & STATUS_READY));

	if (bStatus & STATUS_FAIL)
	{
		MP_DEBUG1("-E- Read Status FAIL (status: %x)", bStatus);
		return READ_STATUS_FAIL;
	}
	return PASS;
}

static void BuildRedtData(DWORD dwLogAddr,BYTE bValid)
{
	register MCARD *sMcard;
	DWORD dwTempAddr, dwParity;
	ST_MEDIA_MEAT *sMediaMeat;


	sMcard = (MCARD *) (MCARD_BASE);
	sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];
	dwTempAddr = dwLogAddr % sMediaMeat->wMaxLogBlock;
	dwParity = CheckWordParity((WORD) (dwTempAddr) + 0x1000);
	dwTempAddr = ((dwTempAddr << 1) | 0x1000) + dwParity;
	bRedtData[0] = 0xff;
	bRedtData[1] = 0xff;
	bRedtData[2] = 0xff;
	bRedtData[3] = 0xff;
	bRedtData[4] = 0xff;
	bRedtData[5] = 0xff;
	bRedtData[6] = (BYTE) (dwTempAddr >> 8);
	bRedtData[7] = (BYTE) dwTempAddr;
	bRedtData[8] = (BYTE) (sMcard->McSmEcc2 & 0xff);
	bRedtData[9] = (BYTE) ((sMcard->McSmEcc2 >> 8) & 0xff);
	bRedtData[10] = (BYTE) ((sMcard->McSmEcc2 >> 16) & 0xff);
	bRedtData[11] = bRedtData[6];
	bRedtData[12] = bRedtData[7];
	bRedtData[13] = (BYTE) (sMcard->McSmEcc1 & 0xff);
	bRedtData[14] = (BYTE) ((sMcard->McSmEcc1 >> 8) & 0xff);
	bRedtData[15] = (BYTE) ((sMcard->McSmEcc1 >> 16) & 0xff);	

	if (bValid == SFT_BAD_BLOCK)
	{
		bRedtData[5] = 0xf0;	// block status flag
		MP_DEBUG("Bad Block Mark");
	}
	else if (bValid == SFT_DATA_FAIL)
	{
		bRedtData[4] = 0;	// data status flag
		MP_DEBUG("Data Fail Mark");
	}
}

static int CheckWordParity(WORD wBlockAddr)
{
	wBlockAddr ^= (wBlockAddr >> 8);
	wBlockAddr ^= (wBlockAddr >> 4);
	wBlockAddr ^= (wBlockAddr >> 2);
	wBlockAddr ^= (wBlockAddr >> 1);
	wBlockAddr = (int) (wBlockAddr & 0x0001);
	return (wBlockAddr);
}

static int BitXor(DWORD dwVector, DWORD dwSize)
{
	switch (dwSize)
	{
	case 32:
		dwVector ^= dwVector >> 16;
		dwVector ^= dwVector >> 8;
		dwVector ^= dwVector >> 4;
		dwVector ^= dwVector >> 2;
		dwVector ^= dwVector >> 1;
		break;
	case 16:
		dwVector ^= dwVector >> 8;
		dwVector ^= dwVector >> 4;
		dwVector ^= dwVector >> 2;
		dwVector ^= dwVector >> 1;
		break;
	case 8:
		dwVector ^= dwVector >> 4;
		dwVector ^= dwVector >> 2;
		dwVector ^= dwVector >> 1;
		break;
	case 4:
		dwVector ^= dwVector >> 2;
		dwVector ^= dwVector >> 1;
		break;
	case 2:
		dwVector ^= dwVector >> 1;
		break;
	}
	dwVector = (int) (dwVector & 0x00000001);
	return (dwVector);
}

static DWORD RowBlockXor(DWORD dwStartAddr, int RowSize)
{
	DWORD dwTemp, dwTempNow, dwRowCount;

	dwTemp = *((DWORD *) (dwStartAddr));
	for (dwRowCount = 1; dwRowCount < RowSize; dwRowCount++)
	{
		dwTempNow = *((DWORD *) (dwStartAddr) + dwRowCount);
		dwTemp ^= dwTempNow;
	}
	return (dwTemp);
}

static int ECCBitCode(DWORD dwStartAddr, int FristStart, int LastStart, int RowSize)
{
	DWORD dwTemp, dwBlockAd;

	if (RowSize >= 32)
	{
		dwTemp = RowBlockXor(dwStartAddr, RowSize);
		dwBlockAd = BitXor(dwTemp, 32);
		return (dwBlockAd);
	}
	else
	{
		dwTemp = RowBlockXor(dwStartAddr, RowSize);
		for (dwBlockAd = FristStart; dwBlockAd < LastStart; dwBlockAd += (RowSize << 3))
		{
			dwTemp ^= RowBlockXor(dwStartAddr + dwBlockAd, RowSize);
		}
		dwBlockAd = BitXor(dwTemp, 32);
		return (dwBlockAd);
	}
}

static DWORD OneRowXor(DWORD dwStartAddr, int FristStart, int LastStart)
{
	DWORD dwTemp, swTempNow, dwCount;

	dwTemp = *((DWORD *) (dwStartAddr));
	for (dwCount = FristStart; dwCount < (LastStart + 1); dwCount += 2)
	{
		swTempNow = *((DWORD *) (dwStartAddr) + dwCount);
		dwTemp ^= swTempNow;
	}
	return (dwTemp);
}

static int ColXor(DWORD dwVector, int ColSize)
{
	int EccCode;

	switch (ColSize)
	{
	case 0x1001:
		EccCode = BitXor((dwVector), 16);
		break;
	case 0x1002:
		EccCode = BitXor((dwVector >> 16), 16);
		break;
	case 0x0801:
		EccCode = BitXor((dwVector >> 16), 8);
		EccCode ^= BitXor((dwVector), 8);
		break;
	case 0x0802:
		EccCode = BitXor((dwVector >> 24), 8);
		EccCode ^= BitXor((dwVector >> 8), 8);
		break;
	case 0x0401:
		EccCode = BitXor((dwVector >> 28), 4);
		EccCode ^= BitXor((dwVector >> 20), 4);
		EccCode ^= BitXor((dwVector >> 12), 4);
		EccCode ^= BitXor((dwVector >> 4), 4);
		break;
	case 0x0402:
		EccCode = BitXor((dwVector >> 24), 4);
		EccCode ^= BitXor((dwVector >> 16), 4);
		EccCode ^= BitXor((dwVector >> 8), 4);
		EccCode ^= BitXor((dwVector), 4);
		break;
	case 0x0201:
		EccCode = BitXor((dwVector >> 30), 2);
		EccCode ^= BitXor((dwVector >> 26), 2);
		EccCode ^= BitXor((dwVector >> 22), 2);
		EccCode ^= BitXor((dwVector >> 18), 2);
		EccCode ^= BitXor((dwVector >> 14), 2);
		EccCode ^= BitXor((dwVector >> 10), 2);
		EccCode ^= BitXor((dwVector >> 6), 2);
		EccCode ^= BitXor((dwVector >> 2), 2);
		break;
	case 0x0202:
		EccCode = BitXor((dwVector >> 28), 2);
		EccCode ^= BitXor((dwVector >> 24), 2);
		EccCode ^= BitXor((dwVector >> 20), 2);
		EccCode ^= BitXor((dwVector >> 16), 2);
		EccCode ^= BitXor((dwVector >> 12), 2);
		EccCode ^= BitXor((dwVector >> 8), 2);
		EccCode ^= BitXor((dwVector >> 4), 2);
		EccCode ^= BitXor((dwVector), 2);
		break;
	case 0x0101:
		EccCode = (dwVector >> 31);
		EccCode ^= (dwVector >> 29);
		EccCode ^= (dwVector >> 27);
		EccCode ^= (dwVector >> 25);
		EccCode ^= (dwVector >> 23);
		EccCode ^= (dwVector >> 21);
		EccCode ^= (dwVector >> 19);
		EccCode ^= (dwVector >> 17);
		EccCode ^= (dwVector >> 15);
		EccCode ^= (dwVector >> 13);
		EccCode ^= (dwVector >> 11);
		EccCode ^= (dwVector >> 9);
		EccCode ^= (dwVector >> 7);
		EccCode ^= (dwVector >> 5);
		EccCode ^= (dwVector >> 3);
		EccCode ^= (dwVector >> 1);
		EccCode = (int) (EccCode & 0x00000001);
		break;
	case 0x0102:
		EccCode = (dwVector >> 30);
		EccCode ^= (dwVector >> 28);
		EccCode ^= (dwVector >> 26);
		EccCode ^= (dwVector >> 24);
		EccCode ^= (dwVector >> 22);
		EccCode ^= (dwVector >> 20);
		EccCode ^= (dwVector >> 18);
		EccCode ^= (dwVector >> 16);
		EccCode ^= (dwVector >> 14);
		EccCode ^= (dwVector >> 12);
		EccCode ^= (dwVector >> 10);
		EccCode ^= (dwVector >> 8);
		EccCode ^= (dwVector >> 6);
		EccCode ^= (dwVector >> 4);
		EccCode ^= (dwVector >> 2);
		EccCode ^= (dwVector);
		EccCode = (int) (EccCode & 0x00000001);
		break;
	}
	return (EccCode);
}

static void CalECC(DWORD DataAddr)
{
	DWORD dwEccData, dwEcc32Vector;

	bECCCode[0] = bECCCode[1] = bECCCode[2] = 0x00;
	dwEccData = OneRowXor(DataAddr, 2, 62);
	bECCCode[0] = bECCCode[0] + (BYTE) (BitXor(dwEccData, 32) << 4);	//32
	dwEcc32Vector = dwEccData;
	dwEccData = OneRowXor((DataAddr + 4), 2, 62);
	bECCCode[0] = bECCCode[0] + (BYTE) (BitXor(dwEccData, 32) << 5);	//32'
	dwEcc32Vector ^= dwEccData;

	bECCCode[2] = bECCCode[2] + (ColXor(dwEcc32Vector, 0x0102) << 2);	//01'
	bECCCode[2] = bECCCode[2] + (ColXor(dwEcc32Vector, 0x0101) << 3);
	bECCCode[2] = bECCCode[2] + (ColXor(dwEcc32Vector, 0x0202) << 4);	//02'
	bECCCode[2] = bECCCode[2] + (ColXor(dwEcc32Vector, 0x0201) << 5);
	bECCCode[2] = bECCCode[2] + (ColXor(dwEcc32Vector, 0x0402) << 6);	//04'
	bECCCode[2] = bECCCode[2] + (ColXor(dwEcc32Vector, 0x0401) << 7);
	bECCCode[0] = bECCCode[0] + (ColXor(dwEcc32Vector, 0x0802) << 0);	//08'
	bECCCode[0] = bECCCode[0] + (ColXor(dwEcc32Vector, 0x0801) << 1);
	bECCCode[0] = bECCCode[0] + (ColXor(dwEcc32Vector, 0x1002) << 2);	//16
	bECCCode[0] = bECCCode[0] + (ColXor(dwEcc32Vector, 0x1001) << 3);
	bECCCode[0] = bECCCode[0] + (ECCBitCode(DataAddr, 16, 256, 2) << 6);	//64'
	bECCCode[0] = bECCCode[0] + (ECCBitCode((DataAddr + 8), 16, 256, 2) << 7);
	bECCCode[1] = bECCCode[1] + (ECCBitCode((DataAddr + 16), 32, 256, 4) << 1);	//128
	bECCCode[1] = bECCCode[1] + (ECCBitCode(DataAddr, 32, 256, 4));	//128'
	bECCCode[1] = bECCCode[1] + (ECCBitCode((DataAddr + 32), 64, 256, 8) << 3);	//256
	bECCCode[1] = bECCCode[1] + (ECCBitCode(DataAddr, 64, 256, 8) << 2);	//256'
	bECCCode[1] = bECCCode[1] + (ECCBitCode((DataAddr + 64), 128, 256, 16) << 5);	//512
	bECCCode[1] = bECCCode[1] + (ECCBitCode(DataAddr, 128, 256, 16) << 4);	//512'
	bECCCode[1] = bECCCode[1] + (ECCBitCode((DataAddr + 128), 256, 256, 32) << 7);	//1024
	bECCCode[1] = bECCCode[1] + (ECCBitCode(DataAddr, 256, 256, 32) << 6);	//1024'
	bECCCode[0] = ~bECCCode[0];
	bECCCode[1] = ~bECCCode[1];
	bECCCode[2] = ~bECCCode[2];
}

static void xD_RW_Test()
{
	ST_MEDIA_MEAT *mm = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];
	DWORD i;
	DWORD blk = 1024 * mm->wMaxSector;
	DWORD *buf = (DWORD *)((DWORD)ext_mem_malloc(mm->wMaxSector * 512) | 0xa0000000);

	for (i = 0 ; i < mm->wMaxSector * (512 >> 2) ; i++)
		buf[i] = i + (i >> 9);
	mpDebugPrint("test pattern:");
	MemDump((BYTE *)buf, mm->wMaxSector * 512);
	PhysicalErase(blk);
	PhysicalWrite(blk, mm->wMaxSector, (DWORD)buf, 1024, SFT_NORMAL, NULL);
	memset(buf, 0xA5, mm->wMaxSector * 512);
	PhysicalRead(blk, mm->wMaxSector, (DWORD)buf, NULL);
	for (i = 0 ; i < mm->wMaxSector * (512 >> 2) ; i++)
	{
		if (buf[i] != (i + (i >> 9)))
		{
			MemDump((BYTE *)buf, mm->wMaxSector * 512);
			break;
		}
	}
	ext_mem_free(buf);
}

static SWORD LowLevelFormat()
{
	DWORD i;
	DWORD blk;
	ST_MEDIA_MEAT *sMediaMeat = (ST_MEDIA_MEAT *) sInfo.sMediaMeat[sInfo.bCurMode];

	blk = sMediaMeat->bMaxZone * sMediaMeat->wMaxBlock;
	mpDebugPrint("Erase xD (%d blocks)", blk);
	for (i = (CIS_BlkAddr + 1) ; i < blk ; i++)
	{
		if (PhysicalErase(i*sMediaMeat->wMaxSector) != PASS)
		{
			mpDebugPrint("Bad Block %d", i);
			PhysicalWrite(i*sMediaMeat->wMaxSector, sMediaMeat->wMaxSector, 0xA0000000, 0xffff, SFT_BAD_BLOCK, NULL);	// mark as bad
		}
	}
	mpDebugPrint("Erase done");
}

#endif
