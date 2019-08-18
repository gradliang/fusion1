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
* Filename      : SimpleNandLog.c
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
#if ((NAND_ENABLE  && (NAND_DRV == NAND_SIMPLE_DRV)) || (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND)))
#include "mpTrace.h"

#include "Mcard.h"
#include "nand.h"
#include "Uti.h"

/*
// Constant declarations
*/
#define NAND_WRITE_VERIFY	0
#define LOG_BLK_PER_ZONE	1000
#define BLOCK_PER_ZONE 		1024
#define MAX_BADBLK_PER_ZONE	(NandLog.BlockPerZone - NandLog.LogBlkPerZone)

typedef struct ST_NANDLOG
{
	DWORD Inited;
	DWORD ZoneNr;
	DWORD LogBlkPerZone;
	DWORD BlockPerZone;
	DWORD SectorPerBlock;
} ST_NandLog;

/*
// Variable declarations
*/
static ST_NandLog NandLog = {0};
static DWORD ReservedsSectorNr = 0;
#if (NAND_ENABLE  && (NAND_DRV == NAND_SIMPLE_DRV))
static WORD *NandLog2PhyTable = NULL;	//[NandLog.BlockPerZone * NAND_MAX_ZONE];
static WORD *BadBlockPtr = NULL;	//[MAX_ZONE];
static WORD *FreeBlockPtr = NULL;	//[MAX_ZONE];
static BYTE bDescriptor[] = "NAND";
static DWORD MaxBadBlkNr = 0, TotalBadBlkNr = 0;
#endif

static DWORD NandLogicalInit()
{
	DWORD BlkNr, SkipBlk;
	DWORD capacity;

	GetNandGeometry(&BlkNr, &NandLog.SectorPerBlock);
	NandLog.ZoneNr = BlkNr / BLOCK_PER_ZONE;
	if (ISP_GetNandReservedSize())
	{
		DWORD Sectors = (ISP_GetNandReservedSize() + MCARD_SECTOR_SIZE - 1) >> MCARD_SECTOR_SIZE_EXP;
		DWORD Blocks = (Sectors + NandLog.SectorPerBlock - 1) / NandLog.SectorPerBlock;
		SkipBlk = ((Blocks + NandLog.ZoneNr - 1) / NandLog.ZoneNr) * NandLog.ZoneNr;
		mpDebugPrint("skip %d blocks", SkipBlk);
	}
	else
	{
		SkipBlk = 1;
	}
	do
	{
		NandLog.BlockPerZone = (BlkNr - SkipBlk) / NandLog.ZoneNr;
		if ((NandLog.ZoneNr > 1) && (NandLog.BlockPerZone <= (BLOCK_PER_ZONE >> 1)))
			NandLog.ZoneNr >>= 1;
		else
		{
			NandLog.LogBlkPerZone = NandLog.BlockPerZone * LOG_BLK_PER_ZONE / BLOCK_PER_ZONE;
			break;
		}
	} while (NandLog.ZoneNr > 1);
	capacity = NandLog.ZoneNr * NandLog.LogBlkPerZone * NandLog.SectorPerBlock;
	ReservedsSectorNr = SkipBlk * NandLog.SectorPerBlock;
	MP_DEBUG("Block:%d, SectorPerBlk:%d", BlkNr, NandLog.SectorPerBlock);
	MP_DEBUG("Zone:%d, Physical Block per zone:%d, Logical Block per zone:%d", NandLog.ZoneNr, NandLog.BlockPerZone, NandLog.LogBlkPerZone);

	return capacity;
}

#if (NAND_ENABLE  && (NAND_DRV == NAND_SIMPLE_DRV))
static SWORD Log2PhyTabInit(BOOL GarbageCollection)
{
	DWORD PhyAddr;	// physical address of nandflash, in sector unit
	SWORD swRetValue = PASS;
	DWORD i, z, b;
	WORD LogBlockAddr1, LogBlockAddrN;
	WORD *Log2PhyTab;
	BYTE bRedtData[256];
	WORD EraseBlk;
	WORD emptylist[BLOCK_PER_ZONE], emptr;

	MP_DEBUG(__FUNCTION__);
	MaxBadBlkNr = 0;
	TotalBadBlkNr = 0;
	PhyAddr = ReservedsSectorNr;
	Log2PhyTab = NandLog2PhyTable;
	for (z = 0; z < NandLog.ZoneNr ; z++, Log2PhyTab+=NandLog.BlockPerZone)
	{
		emptr = 0;
		BadBlockPtr[z] = NandLog.BlockPerZone;
		FreeBlockPtr[z] = NandLog.LogBlkPerZone-1;
		for (i = 0; i < NandLog.BlockPerZone; i++)
		{
			Log2PhyTab[i] = 0xFFFF;
			emptylist[i] = 0xFFFF;
		}

		for (b = 0 ; b < NandLog.BlockPerZone ; b++, PhyAddr+=NandLog.SectorPerBlock)
		{
			EraseBlk = 0xFFFF;
			LogBlockAddr1 = GetNandLogBlkAddr(PhyAddr, NandLog.LogBlkPerZone);
			LogBlockAddrN = GetNandLogBlkAddr(PhyAddr+NandLog.SectorPerBlock-1, NandLog.LogBlkPerZone);
			if ((LogBlockAddr1 == LB_READ_FAIL) || (LogBlockAddrN == LB_READ_FAIL))
			{
				MP_DEBUG1("-E- ReadRedtData FAIL (swRetValue: %d)", swRetValue);
				return FAIL;
			}
			else if ((LogBlockAddr1 > LB_ERROR && LogBlockAddr1 < LB_NO_ADDR)
					|| (LogBlockAddrN > LB_ERROR && LogBlockAddrN < LB_NO_ADDR))
			{	// found a bad block, add to bad list
				TotalBadBlkNr++;
				if ((BadBlockPtr[z] - 1) >= NandLog.LogBlkPerZone)
				{
					if ((BadBlockPtr[z] - 1) <= FreeBlockPtr[z])
					{	// no space for a BAD, remove a FREE
						emptylist[emptr++] = Log2PhyTab[FreeBlockPtr[z]];
						FreeBlockPtr[z]--;
					}
					BadBlockPtr[z]--;
					Log2PhyTab[BadBlockPtr[z]] = b;
					WORD BadNr = NandLog.BlockPerZone - BadBlockPtr[z];
					if (BadNr > MaxBadBlkNr)
						MaxBadBlkNr = BadNr;
					mpDebugPrint("BadBlock(%d,(%d->%d)~phy=%d",z,b,BadBlockPtr[z], PhyAddr);
					//DumpSpareContent(PhyAddr);
				}
			}
			else if (LogBlockAddr1 >= NandLog.LogBlkPerZone)	// wrong number
			{
				if (CheckSpareContent(PhyAddr) == TRUE)	// has content?
				{
					//mpDebugPrint("Invalid lba, %d", LogBlockAddr1);
					EraseBlk = b;
				}
				else if ((FreeBlockPtr[z]+1) < BadBlockPtr[z])
				{
					FreeBlockPtr[z]++;
					Log2PhyTab[FreeBlockPtr[z]] = b;
				}
				else
				{
					emptylist[emptr++] = b;
				}
			}
			else
			{	// XD Tcode 260
				if (Log2PhyTab[LogBlockAddr1] == 0xffff)	// this logical block address does not been added.
				{
					Log2PhyTab[LogBlockAddr1] = b;
				}
				else	// already be occupied.
				{	// check last page of occupyier block
					DWORD LogBlkAddr;
					DWORD occupierSect = Log2PhyTab[LogBlockAddr1] * NandLog.SectorPerBlock;
					mpDebugPrint("-E- LogBlk %d found at %d,%d", LogBlockAddr1, Log2PhyTab[LogBlockAddr1], b);
					LogBlkAddr = GetNandLogBlkAddr(occupierSect+NandLog.SectorPerBlock-1, NandLog.LogBlkPerZone);
					if (LogBlkAddr == LB_READ_FAIL)
					{
						MP_DEBUG1("-E- ReadRedtData FAIL (swRetValue: %d)", swRetValue);
						return FAIL;
					}
					else if (LogBlkAddr == LogBlockAddr1)
					{	// last page's block address is also correct. Use it and erase b.
						mpDebugPrint("\t->select %d", Log2PhyTab[LogBlockAddr1]);
						EraseBlk = b;
					}
					else
					{	// occupier is not good. Erase it
						EraseBlk = Log2PhyTab[LogBlockAddr1];
						// check last page of current block
						if (LogBlockAddrN == LogBlockAddr1)
						{	// it's good. use it
							mpDebugPrint("\t->select %d", b);
							Log2PhyTab[LogBlockAddr1] = b;
						}
						else
						{	// not good, erase it.
							mpDebugPrint("\t->both failed!");
							EraseBlk = b;
						}
					}
				}
			}
			if (GarbageCollection && (EraseBlk < NandLog.BlockPerZone))
			{
				mpDebugPrint("\tErase %d", EraseBlk);
				if (NandBlockErase(EraseBlk * NandLog.SectorPerBlock) != PASS)	// try to erase recycle block
				{
					TotalBadBlkNr++;
					if ((BadBlockPtr[z] - 1) >= NandLog.LogBlkPerZone)
					{
						if ((BadBlockPtr[z] - 1) <= FreeBlockPtr[z])
						{	// no space for a BAD, remove a FREE
							emptylist[emptr++] = Log2PhyTab[FreeBlockPtr[z]];
							FreeBlockPtr[z]--;
						}
						BadBlockPtr[z]--;
						Log2PhyTab[BadBlockPtr[z]] = EraseBlk;
						WORD BadNr = NandLog.BlockPerZone - BadBlockPtr[z];
						if (BadNr > MaxBadBlkNr)
							MaxBadBlkNr = BadNr;
						mpDebugPrint("BadBlock(%d,(%d->%d)",z,EraseBlk,BadBlockPtr[z]);
					}
				}
				else if ((FreeBlockPtr[z]+1) < BadBlockPtr[z])
				{
					FreeBlockPtr[z]++;
					Log2PhyTab[FreeBlockPtr[z]] = EraseBlk;
				}
			}
		}
		// check whether whole logical block are mapped or not.
		b = 0;
		while (emptr && (b < NandLog.LogBlkPerZone))
		{
			if (Log2PhyTab[b] == 0xffff)
			{
				Log2PhyTab[b] = emptylist[--emptr];
			}
			b++;
		}
		emptr = 0;
		#if LOCAL_DEBUG_ENABLE
		BYTE info[64];
		mpDebugPrint("Zone %d, badptr=%d,freeptr=%d:", z, BadBlockPtr[z], FreeBlockPtr[z]);
		for ( b = 0 ; b < NandLog.BlockPerZone ; b++)
		{
			sprintf(info, "Log2Phy[%d]= %d, ", b, Log2PhyTab[b]);
			UartOutText(info);
			if ((b & 0x3) == 0x03)
				UartOutText("\r\n");
		}
		#endif
	}

	return PASS;
}

static SWORD LogicalRead(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress, DWORD *progress, BYTE *DataStatus)
{
	SWORD ret = PASS;
	DWORD blk = dwLogAddr / NandLog.SectorPerBlock;
	DWORD logblk = blk % NandLog.LogBlkPerZone;
	DWORD zoneaddr = (blk / NandLog.LogBlkPerZone) * NandLog.BlockPerZone;	// which zone does dwLogAddr locate in
	DWORD phyaddr = ReservedsSectorNr + (zoneaddr + NandLog2PhyTable[zoneaddr+logblk]) * NandLog.SectorPerBlock + (dwLogAddr % NandLog.SectorPerBlock);
	DWORD cnt;

	cnt = (blk + 1) * NandLog.SectorPerBlock - dwLogAddr;
	cnt = (cnt > dwSectorCount) ? dwSectorCount : cnt;
	while (dwSectorCount)
	{
		MP_DEBUG("   read blk %d ->%d", logblk, NandLog2PhyTable[zoneaddr+logblk]);
		if (NandLog2PhyTable[zoneaddr+logblk] == 0xffff)	// not using
		{
			memset((BYTE *)dwBufferAddress, 0xff, cnt << MCARD_SECTOR_SIZE_EXP);
			if (DataStatus)
				memset(DataStatus, SFT_NORMAL, cnt);
			if (progress)
				(*progress) += cnt;
			ret = PASS;
		}
		else
		{
			ret = NandSectorRead(phyaddr, cnt, dwBufferAddress, progress, DataStatus);
		}
		if (ret != PASS)
			break;
		dwSectorCount -= cnt;
		if (dwSectorCount)
		{
			dwBufferAddress += cnt << MCARD_SECTOR_SIZE_EXP;
			if (DataStatus)
				DataStatus += cnt;
			if (++logblk >= NandLog.LogBlkPerZone)
			{
				zoneaddr += NandLog.BlockPerZone;
				logblk = 0;
			}
			phyaddr = ReservedsSectorNr + (zoneaddr + NandLog2PhyTable[zoneaddr+logblk]) * NandLog.SectorPerBlock;
			cnt = (dwSectorCount > NandLog.SectorPerBlock) ? NandLog.SectorPerBlock : dwSectorCount;
		}
	}

	return ret;
}

static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress)
{
	BYTE bRetry = MCARD_RETRY_TIME;
	SWORD swRetValue = PASS;

	MP_DEBUG("Nand FlatRead  ,dwLogAddr %x,dwSectorCount %d",dwLogAddr,dwSectorCount);
	while (bRetry--)
	{
		if (progress)
			*progress = 0;
		swRetValue = LogicalRead(dwLogAddr, dwSectorCount, dwBufferAddress, progress, NULL);
		if (swRetValue == PASS)
			break;
		mpDebugPrint("   %s: Addr=%d, remain retry times %d", __FUNCTION__, dwLogAddr, bRetry);
	}

	return swRetValue;
}

static SWORD MarkBadBlock(DWORD ZoneAddr, WORD PhyBlkAddr)
{
	SWORD ret = PASS;
	BYTE *pbBuffer = (BYTE *)((DWORD)ext_mem_malloc(NandLog.SectorPerBlock << MCARD_SECTOR_SIZE_EXP) | 0xA0000000);
	DWORD phyaddr = ReservedsSectorNr + (ZoneAddr * NandLog.BlockPerZone + PhyBlkAddr) * NandLog.SectorPerBlock;
	WORD *Log2PhyTab = &NandLog2PhyTable[ZoneAddr*NandLog.BlockPerZone];

	memset(pbBuffer,0x5a,NandLog.SectorPerBlock << MCARD_SECTOR_SIZE_EXP);
	TotalBadBlkNr++;
	NandBlockErase(phyaddr);
	NandBlockWrite(phyaddr, (DWORD) pbBuffer, 0xFFFF, NULL, SFT_BAD_BLOCK, NULL);	// mark as bad
	if ((BadBlockPtr[ZoneAddr] - 1) >= NandLog.LogBlkPerZone)
	{
		if ((BadBlockPtr[ZoneAddr] - 1) <= FreeBlockPtr[ZoneAddr])	// no space for a BAD, reduce FREE
			FreeBlockPtr[ZoneAddr] = BadBlockPtr[ZoneAddr] - 1;
		BadBlockPtr[ZoneAddr]--;
		Log2PhyTab[BadBlockPtr[ZoneAddr]] = PhyBlkAddr;
		WORD BadNr = NandLog.BlockPerZone - BadBlockPtr[ZoneAddr];
		if (BadNr > MaxBadBlkNr)
			MaxBadBlkNr = BadNr;
		mpDebugPrint("BadBlock(%d,(%d->%d)",ZoneAddr,PhyBlkAddr,BadBlockPtr[ZoneAddr]);
	}
	ext_mem_free(pbBuffer);

	return ret;
}

static SWORD LogicalBlockWrite(DWORD LogBlkAddr, DWORD buffer, DWORD *progress, BYTE *SectorStatus)
{
	SWORD ret = FAIL;
	DWORD zone = LogBlkAddr / NandLog.LogBlkPerZone;	// which zone does dwLogAddr locate in
	DWORD logblk = LogBlkAddr % NandLog.LogBlkPerZone;
	DWORD zoneaddr = zone * NandLog.BlockPerZone;

	do
	{
		if ((FreeBlockPtr[zone] + 1) < BadBlockPtr[zone])	// leakage found, re-build table
			Log2PhyTabInit(TRUE);
		if (FreeBlockPtr[zone] >= NandLog.LogBlkPerZone)
		{
			WORD CurBlk = NandLog2PhyTable[zoneaddr+logblk];
			WORD FreeBlk = NandLog2PhyTable[zoneaddr+FreeBlockPtr[zone]];
			DWORD phyaddr = ReservedsSectorNr + (zoneaddr + CurBlk) * NandLog.SectorPerBlock;
			DWORD newphyaddr = ReservedsSectorNr + (zoneaddr + FreeBlk) * NandLog.SectorPerBlock;
			MP_DEBUG("   write old blk %d -> new blk %d", CurBlk, FreeBlk);
			if ((ret = NandBlockWrite(newphyaddr, buffer, logblk, progress, SFT_NORMAL, SectorStatus)) != PASS)
			{
				MarkBadBlock(zone, FreeBlk);
				mpDebugPrint("Nand block %d write failed! ", newphyaddr);
				NandLog2PhyTable[zoneaddr+FreeBlockPtr[zone]] = 0xFFFF;	// new block is bad, remove from free list
				FreeBlockPtr[zone]--;
			}
			else if (CurBlk == 0xffff)
			{
				NandLog2PhyTable[zoneaddr+logblk] = FreeBlk;	// new block picked up and is using now
				FreeBlockPtr[zone]--;
				ret = PASS;
			}
			else if ((CheckSpareContent(phyaddr) == TRUE) && (NandBlockErase(phyaddr) != PASS))
			{	// erase failed... add CurBlk to bad block list...
				NandLog2PhyTable[zoneaddr+logblk] = FreeBlk;	// new block is picked up and is using now
				MarkBadBlock(zone, CurBlk);	// old block is bad, mark it
				NandLog2PhyTable[zoneaddr+FreeBlockPtr[zone]] = 0xFFFF;	// old block is bad, don't add to free list
				FreeBlockPtr[zone]--;
				if (MaxBadBlkNr >= MAX_BADBLK_PER_ZONE)
				{
					mpDebugPrint("Nand: no enough block to write");
					break;
				}
			}
			else
			{
				NandLog2PhyTable[zoneaddr+logblk] = FreeBlk;	// new block picked up and is using now
				NandLog2PhyTable[zoneaddr+FreeBlockPtr[zone]] = CurBlk;	// free old block, add to free list
				ret = PASS;
			}
		}
	} while ((ret != PASS)		// write failed
		&& ((BadBlockPtr[zone] - 1) >= NandLog.LogBlkPerZone));	//  still some free block available...try again

	if (ret != PASS)
	{
		mpDebugPrint("Zone %d : bad=%d, free=%d", zone, BadBlockPtr[zone], FreeBlockPtr[zone]);
	}

	return ret;
}

static SWORD LogicalWrite(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress, DWORD *progress)
{
	SWORD ret = PASS;
	DWORD blk = dwLogAddr / NandLog.SectorPerBlock;
	DWORD cnt = NandLog.SectorPerBlock - (dwLogAddr % NandLog.SectorPerBlock);	// sectors need to write
	BYTE *tmpBuf;	// temp. buffer
	BYTE *statusFlag;	// status type flag array

	MP_DEBUG("%s: %x,%d,%x", __FUNCTION__, dwLogAddr, dwSectorCount, dwBufferAddress);
	tmpBuf = (BYTE *)((DWORD)ext_mem_malloc(NandLog.SectorPerBlock << MCARD_SECTOR_SIZE_EXP) | 0xA0000000);
	statusFlag = (BYTE *)((DWORD)ext_mem_malloc(NandLog.SectorPerBlock) | 0xA0000000);
	if (tmpBuf && statusFlag)
	{
		if (cnt != NandLog.SectorPerBlock)
		{
			DWORD readSector = NandLog.SectorPerBlock - cnt;	// sectors need to read
			memset(statusFlag, SFT_NORMAL, NandLog.SectorPerBlock);
			LogicalRead(dwLogAddr - readSector, readSector, (DWORD)tmpBuf, NULL, statusFlag);
			if (dwSectorCount < cnt)
			{
				memcpy(&tmpBuf[readSector << MCARD_SECTOR_SIZE_EXP]
						, (BYTE *)dwBufferAddress
						, dwSectorCount << MCARD_SECTOR_SIZE_EXP);
				LogicalRead(dwLogAddr + dwSectorCount
							, cnt - dwSectorCount
							, (DWORD)&tmpBuf[(readSector + dwSectorCount) << MCARD_SECTOR_SIZE_EXP]
							, NULL, &statusFlag[readSector + dwSectorCount]);
				dwSectorCount = 0;
			}
			else
			{
				memcpy(&tmpBuf[readSector << MCARD_SECTOR_SIZE_EXP]
						, (BYTE *)dwBufferAddress
						, cnt << MCARD_SECTOR_SIZE_EXP);
				dwSectorCount -= cnt;
			}
			ret = LogicalBlockWrite(blk, (DWORD)tmpBuf, progress, statusFlag);
			blk++;
			dwLogAddr += cnt;
			dwBufferAddress += cnt << MCARD_SECTOR_SIZE_EXP;
		}
		while ((ret == PASS) && (dwSectorCount >= NandLog.SectorPerBlock))
		{
			ret = LogicalBlockWrite(blk, dwBufferAddress, progress, NULL);
			blk++;
			dwLogAddr += NandLog.SectorPerBlock;
			dwSectorCount -= NandLog.SectorPerBlock;
			dwBufferAddress += NandLog.SectorPerBlock << MCARD_SECTOR_SIZE_EXP;
		}
		if ((ret == PASS) && dwSectorCount)
		{
			memset(statusFlag, SFT_NORMAL, NandLog.SectorPerBlock);
			memcpy(tmpBuf, (BYTE*)dwBufferAddress, dwSectorCount << MCARD_SECTOR_SIZE_EXP);
			LogicalRead(dwLogAddr + dwSectorCount
						, NandLog.SectorPerBlock - dwSectorCount
						, (DWORD)&tmpBuf[dwSectorCount << MCARD_SECTOR_SIZE_EXP]
						, NULL, &statusFlag[dwSectorCount]);
			ret = LogicalBlockWrite(blk, (DWORD)tmpBuf, progress, statusFlag);
		}
	}
	else
	{
		mpDebugPrint("Nand.c %s: out of memory!", __FUNCTION__);
		ret = FAIL;
	}
	if (tmpBuf)
		ext_mem_free(tmpBuf);
	if (statusFlag)
		ext_mem_free(statusFlag);

	return ret;
}

static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress)
{
	SWORD swRetValue = PASS;
	BYTE bRetry = MCARD_RETRY_TIME;

	MP_DEBUG("Nand FlatWrite  ,dwLogAddr %d,dwSectorCount %d(%x)",dwLogAddr,dwSectorCount, dwBufferAddress);
	while (bRetry--)
	{
		if (progress)
			*progress = 0;
		swRetValue = LogicalWrite(dwLogAddr, dwSectorCount, dwBufferAddress, progress);
		if (swRetValue == PASS)
			break;
		mpDebugPrint("   %s: Addr=%d, remain retry times %d", __FUNCTION__, dwLogAddr, bRetry);
	}
	#if NAND_WRITE_VERIFY
	if (swRetValue == PASS)
	{
		DWORD size = dwSectorCount << MCARD_SECTOR_SIZE_EXP;
		DWORD *buf = (DWORD *)((DWORD)ext_mem_malloc(size) | 0xA0000000);
		DWORD *target = (DWORD *)dwBufferAddress;

		if (buf)
		{
			DWORD i;
			FlatRead((DWORD)buf, dwSectorCount, dwLogAddr, NULL);
			for (i = 0 ; i < (size >> 2) ; i++)
			{
				if (buf[i] != target[i])
				{
					mpDebugPrint("\r\nSector %d write verify failed!!!\r\nSource(%x):", dwLogAddr, target);
					MemDump(target, size);
					mpDebugPrint("\r\nDestination(%x):", buf);
					MemDump(buf, size);
					break;
				}
			}
			ext_mem_free(buf);
		}
	}
	#endif

	return swRetValue;
}

static SWORD LowLevelFormat(BYTE deepVerify)
{
	DWORD i, j;
	DWORD Blks, SectPerBlk;
	DWORD ret = PASS;

	McardNandActive();
	GetNandGeometry(&Blks, &SectPerBlk);
	BYTE *ptr = (BYTE *)((DWORD)ext_mem_malloc(SectPerBlk * MCARD_SECTOR_SIZE) | 0xA0000000);
	mpDebugPrint("Erase nand flash (%d blk)", Blks);
	for (i = ReservedsSectorNr / SectPerBlk ; i < Blks ; i++)
	{
		if (GetNandLogBlkAddr(i*SectPerBlk, LOG_BLK_PER_ZONE) == LB_DEFAULT_BAD_BLOCK)	// bad block
		{
			mpDebugPrint("Original Bad Block %d", i);
		}
		else
		{
			if (!deepVerify)
			{
				if (NandBlockErase(i*SectPerBlk) == FAIL)
				{
					mpDebugPrint("New Bad Block %d", i);
					NandBlockWrite(i*SectPerBlk, NULL, 0xFFFF,NULL, SFT_BAD_BLOCK, NULL);	// mark as bad
				}
			}
			else
			{
				memset(ptr, 0, SectPerBlk * MCARD_SECTOR_SIZE);
				// 1. erase first
				if (NandBlockErase(i*SectPerBlk) != PASS)
				{
					mpDebugPrint("Erase Failed! New Bad Block %d", i);
					NandBlockWrite(i*SectPerBlk, NULL, 0xFFFF, NULL, SFT_BAD_BLOCK, NULL);	// mark as bad
				}
				// 2. write data pattern
				if (NandBlockWrite(i*SectPerBlk, (DWORD)ptr, 0xFFFF, NULL, SFT_NORMAL, NULL) != PASS)
				{
					mpDebugPrint("Program Failed! New Bad Block %d", i);
					NandBlockErase(i*SectPerBlk);
					NandBlockWrite(i*SectPerBlk, NULL, 0xFFFF, NULL, SFT_BAD_BLOCK, NULL);	// mark as bad
				}
				else
				{
					memset(ptr, 0x5a, SectPerBlk * MCARD_SECTOR_SIZE);
					// 3. read and verify
					if (NandSectorRead(i*SectPerBlk, SectPerBlk, (DWORD)ptr, NULL, NULL) == PASS)
					{
						DWORD *tmp = (DWORD *)ptr;
						SWORD err = 0;

						for (j = 0 ; j < (SectPerBlk * MCARD_SECTOR_SIZE>>2) ; j++)
						{
							if (tmp[j] != 0)
							{
								mpDebugPrint("Pattern test Failed! New Bad Block %d", j);
								NandBlockErase(i*SectPerBlk);
								NandBlockWrite(i*SectPerBlk, NULL, 0xFFFF, NULL, SFT_BAD_BLOCK, NULL);	// mark as bad
								err = 1;
								break;
							}
						}

						if (err == 0)
						{
							PutUartChar('[');
							UartOutValue(i, 4);
							PutUartChar(']');
							NandBlockErase(i*SectPerBlk);
						}
					}
					else
					{
						mpDebugPrint("Read Failed! New Bad Block %d", j);
						NandBlockErase(i*SectPerBlk);
						NandBlockWrite(i*SectPerBlk, NULL, 0xFFFF, NULL, SFT_BAD_BLOCK, NULL);	// mark as bad
					}
				}

			}
		}
	}
	mpDebugPrint("Erase done");
	ext_mem_free(ptr);
	if (NandLog.Inited == TRUE)
	{
		ret = Log2PhyTabInit(FALSE);
	}

	return ret;
}

static void NandFreeTabMem()
{
	if (NandLog2PhyTable)
		ker_mem_free(NandLog2PhyTable);
	if (BadBlockPtr)
		ker_mem_free(BadBlockPtr);
	if (FreeBlockPtr)
		ker_mem_free(FreeBlockPtr);
	NandLog2PhyTable = NULL;
	BadBlockPtr = NULL;
	FreeBlockPtr = NULL;
}

static SWORD NandAllocTabMem()
{
	SWORD ret = PASS;

	if (NandLog2PhyTable)
		ker_mem_free(NandLog2PhyTable);
	if (BadBlockPtr)
		ker_mem_free(BadBlockPtr);
	if (FreeBlockPtr)
		ker_mem_free(FreeBlockPtr);
	NandLog2PhyTable = (WORD *)ker_mem_malloc(NandLog.ZoneNr * NandLog.BlockPerZone * sizeof(WORD), TaskGetId());
	BadBlockPtr = (WORD *)ker_mem_malloc(NandLog.ZoneNr * sizeof(WORD), TaskGetId());
	FreeBlockPtr = (WORD *)ker_mem_malloc(NandLog.ZoneNr * sizeof(WORD), TaskGetId());
	MP_DEBUG("%s:%x,%x,%x", __FUNCTION__, NandLog2PhyTable, BadBlockPtr, FreeBlockPtr);
	if (!NandLog2PhyTable || !BadBlockPtr || !FreeBlockPtr)
	{
		mpDebugPrint("%s: out of memory", __FUNCTION__);
		NandFreeTabMem();
		ret = FAIL;
	}

	return ret;
}

static void CommandProcess(void *pMcardDev)
{
	ST_MCARD_DEV *pDev = (ST_MCARD_DEV *) pMcardDev;
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;
	DWORD tmp;

	mail->swStatus = PASS;
#if SD_MMC_ENABLE
    SdProgChk();
#endif
	SetMcardClock(NAND_CLOCK_KHZ);
	//mpDebugPrint("Nand(%x) Command: %x(%x,%d)", pDev->wMcardType, mail->wCmd, mail->dwBlockAddr, mail->dwBlockCount);
	if (pDev->wMcardType != DEV_NAND)
	{
		mpDebugPrint("CommandProcess, not NAND!");
		mail->swStatus = FAIL;
		return;
	}
	McardNandActive();
	switch (mail->wCmd)
	{
		case INIT_CARD_CMD:
			// pre-set all necessary variables
			pDev->Flag.Detected = 1;
			if (NandLog.Inited != TRUE)
			{
				pDev->Flag.Present = 0;
				pDev->dwCapacity = 0;
				pDev->Flag.ReadOnly = TRUE;
				if ((mail->swStatus = NandIdentify()) == PASS)
				{
					pDev->dwCapacity = NandLogicalInit();
					pDev->wSectorSize = MCARD_SECTOR_SIZE;
					pDev->wSectorSizeExp = MCARD_SECTOR_SIZE_EXP;
					//Nand_RW_Test(0, 32);
					//LowLevelFormat(0);
					//MXIC_Nand_RW_Test(0, 32);
					//while(1);
					if (NandAllocTabMem() != PASS)
					{
					}
					else if (mail->swStatus = Log2PhyTabInit(FALSE))
					{
						mpDebugPrint("Log2PhyTabInit fail");
						NandFreeTabMem();
					}
					else
					{
						MP_DEBUG("Log2PhyTabInit pass");
						pDev->wRenewCounter++;
						pDev->Flag.ReadOnly = (MaxBadBlkNr >= MAX_BADBLK_PER_ZONE) ? TRUE : FALSE;
						pDev->Flag.Present = 1;
						if (pDev->Flag.ReadOnly)
							mpDebugPrint("Read Only!");
						NandLog.Inited = TRUE;
					}
				}
			}
			break;
		case REMOVE_CARD_CMD:		//Athena 03.11.2006 seperate card in & out
			//card out
			pDev->Flag.Present = 0;
			pDev->Flag.ReadOnly = TRUE;
			pDev->Flag.PipeEnable = 0;
			mail->swStatus = 0;
			pDev->dwCapacity = 0;
			pDev->wSectorSize = 0;
			pDev->wSectorSizeExp = 0;
			NandFreeTabMem();
			NandLog.Inited = FALSE;
			break;
		case SECTOR_READ_CMD:
			if (mail->dwBlockAddr < pDev->dwCapacity)
				mail->swStatus = FlatRead(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr, &mail->dwProgress);
			else
				mail->swStatus = FAIL;
			break;

		case SECTOR_WRITE_CMD:
			if (mail->dwBlockAddr < pDev->dwCapacity)
			{
				if (pDev->Flag.ReadOnly != TRUE)
				{
					mail->swStatus = FlatWrite(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr, &mail->dwProgress);
					if (MaxBadBlkNr >= MAX_BADBLK_PER_ZONE)
					{
						pDev->Flag.ReadOnly = TRUE;
						mpDebugPrint("Read Only!");
					}
				}
			}
			else
				mail->swStatus = FAIL;
			break;

		case RAW_FORMAT_CMD:
			mail->swStatus = LowLevelFormat(mail->dwBlockAddr);
			if (MaxBadBlkNr >= MAX_BADBLK_PER_ZONE)
			{
				pDev->Flag.ReadOnly = TRUE;
				mpDebugPrint("Read Only!");
			}
			break;

		default:
			MP_DEBUG("-E- INVALID CMD");
			break;
	}
	McardNandInactive();
}

void NandInit(ST_MCARD_DEV *sDev)
{
	MP_DEBUG(__FUNCTION__);
	sDev->pbDescriptor = bDescriptor;
	sDev->wMcardType = DEV_NAND;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
}

#if NAND_ECC_TEST
// EccBit : 0(1 bit ECC), 1(8 bit ECC), 2(12 bit ECC), 3(16 bit ECC), 4(24 bit ECC)
// TestCnt: Number of block for testing
void Nand_RW_Test(BYTE EccBit, DWORD TestCnt)
{
	BYTE ecc_nr[] = {1, 8, 12, 16, 24};
	DWORD i, s;
	DWORD *buf = (DWORD *)((DWORD)ext_mem_malloc(NandLog.SectorPerBlock * MCARD_SECTOR_SIZE) | 0xa0000000);
	DWORD blk, SectorPerBlk;

	mpDebugPrint("Start Nand reading/writing testing:");

	NandConfigForTest(EccBit);
	McardNandInactive();
	McardNandActive();
	GetNandGeometry(&blk, &SectorPerBlk);
	mpDebugPrint("Nandlog SectorPerBlock: %d, SectorPerBlk: %d", NandLog.SectorPerBlock, SectorPerBlk);
	//MC_ECCCRC_test();
	for (s = ReservedsSectorNr ; s < ReservedsSectorNr + TestCnt * SectorPerBlk ; s += SectorPerBlk)
	{
		UartOutValue(s/SectorPerBlk, 8);  // Print current block number
		UartOutText("...");
#if 1
		for (i = 0 ; i < SectorPerBlk * (MCARD_SECTOR_SIZE >> 2) ; i++)
			buf[i] = i + s + 0xa5a5a5a5;
		//mpDebugPrint("test pattern:");
		//MemDump(buf, NandLog.SectorPerBlock * 512);
		if (NandBlockErase(s) != PASS)
		{
			mpDebugPrint("%d is bad block", s);
			continue;
		}
		NandBlockWrite(s, (DWORD)buf, s/SectorPerBlk, NULL, SFT_NORMAL, NULL);
		//NandRawPageDump(s);
#else
		UartOutText("\r\n");
#endif
		memset(buf, 0x5A, SectorPerBlk * MCARD_SECTOR_SIZE);
#if 0
		NandSectorRead(s, 1, (DWORD)buf, NULL, NULL);
		NandSectorRead(s+1, SectorPerBlk-1, ((DWORD)buf)+512, NULL, NULL);
#else
		NandSectorRead(s, SectorPerBlk, (DWORD)buf, NULL, NULL);
#endif
		for (i = 0 ; i < SectorPerBlk * (MCARD_SECTOR_SIZE >> 2) ; i++)
		{
			if (buf[i] != (i + s + 0xa5a5a5a5))
			{
				mpDebugPrint("NG([%x] : 0x%x,0x%x)", i<<2, buf[i], i+s);
				MemDump(&buf[i&(~0x7f)], 512);
				while(1);
				//break;
			}
		}
		#if 0
		if (i >= (SectorPerBlk * (MCARD_SECTOR_SIZE >> 2)))
		{
			DWORD v = s / SectorPerBlk;
			mpDebugPrint("Good");
			NandRawPageRead(s, buf);
			NandBlockErase(s);
			#if 0
			buf[v&0x3f] ^= 0x01 << (v & 0x07);
			#else /*
			       *  Create (EccBit-1) bits error inside data section.   Designed by Logan
                   *  Using for ECC correction testing
			       */
			BYTE *ptr = (BYTE*)buf;
			DWORD tt = GetSysTime();
			for (i = 0 ; i < ecc_nr[EccBit] ; i++)
			{
				tt += GetSysTime() + v;
				mpDebugPrint("	%d - %d", i, tt);	// Without this line, tt value looks so close.
				ptr[(tt&0xff)] ^= 1<<(GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 1 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 2 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 3 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
			}
			#endif
			NandRawPageWrite(s, buf);
			memset(buf, 0x5a, 4 * MCARD_SECTOR_SIZE);
			NandSectorRead(s, 4, (DWORD)buf, NULL, NULL);
			for (i = 0 ; i < ((4 * MCARD_SECTOR_SIZE) >> 2) ; i++)
			{
				if (buf[i] != (i + s + 0xa5a5a5a5))
				{
					mpDebugPrint("->NG([%x] : 0x%x,0x%x)", i<<2, buf[i], i+s);
					//MemDump(buf, SectorPerBlk * 512);
					//break;
				}
			}
		}
		#else
		if (i >= (SectorPerBlk*(MCARD_SECTOR_SIZE>>2)))
			mpDebugPrint("Good");

		#endif
	}

	ext_mem_free(buf);
	NandConfigForNormal();
}

// EccBit : 0(1 bit ECC), 1(8 bit ECC), 2(12 bit ECC), 3(16 bit ECC), 4(24 bit ECC)
// TestCnt: Number of block for testing
void MXIC_Nand_RW_Test(BYTE EccBit, DWORD TestCnt)
{
	BYTE ecc_nr[] = {1, 8, 12, 16, 24};
	DWORD i, j, s, loop_cnt;
	DWORD *buf = (DWORD *)((DWORD)ext_mem_malloc(NandLog.SectorPerBlock * MCARD_SECTOR_SIZE) | 0xa0000000);
	DWORD blk, SectorPerBlk;

	mpDebugPrint("Start MXIC Nand reading/writing testing:");

	NandConfigForTest(EccBit);
	McardNandInactive();
	McardNandActive();
	GetNandGeometry(&blk, &SectorPerBlk);
	mpDebugPrint("Nandlog SectorPerBlock: %d, SectorPerBlk: %d", NandLog.SectorPerBlock, SectorPerBlk);
	//MC_ECCCRC_test();
	s = 0;
	for (j = 0 ; j < 0xffffffff ; j++)
	{
		//UartOutValue(s/SectorPerBlk, 8);  // Print current block number
		UartOutText("...");
#if 1
		for (i = 0 ; i < SectorPerBlk * (MCARD_SECTOR_SIZE >> 2) ; i++)
			buf[i] = 0xa5a5a5a5;
		//mpDebugPrint("test pattern:");
		//MemDump(buf, NandLog.SectorPerBlock * 512);
		if (NandBlockErase(s) != PASS)
		{
			mpDebugPrint("Block 0 erase %d times become bad block", j);
			break;
		}
		NandBlockWrite(s, (DWORD)buf, s/SectorPerBlk, NULL, SFT_NORMAL, NULL);
		//NandRawPageDump(s);
#else
		UartOutText("\r\n");
#endif
		memset(buf, 0x5A, SectorPerBlk * MCARD_SECTOR_SIZE);
#if 0
		NandSectorRead(s, 1, (DWORD)buf, NULL, NULL);
		NandSectorRead(s+1, SectorPerBlk-1, ((DWORD)buf)+512, NULL, NULL);
#else
		NandSectorRead(s, SectorPerBlk, (DWORD)buf, NULL, NULL);
#endif
		for (i = 0 ; i < SectorPerBlk * (MCARD_SECTOR_SIZE >> 2) ; i++)
		{
			if (buf[i] != (0xa5a5a5a5))
			{
				mpDebugPrint("NG([%x] : 0x%x,0x%x)", i<<2, buf[i], i+s);
				mpDebugPrint("MXIC NAND Block 0 test %d times fail)", j);
				MemDump(&buf[i&(~0x7f)], 512);
				//while(1);
				break;
			}
		}
		#if 0
		if (i >= (SectorPerBlk * (MCARD_SECTOR_SIZE >> 2)))
		{
			DWORD v = s / SectorPerBlk;
			mpDebugPrint("Good");
			NandRawPageRead(s, buf);
			NandBlockErase(s);
			#if 0
			buf[v&0x3f] ^= 0x01 << (v & 0x07);
			#else /*
			       *  Create (EccBit-1) bits error inside data section.   Designed by Logan
                   *  Using for ECC correction testing
			       */
			BYTE *ptr = (BYTE*)buf;
			DWORD tt = GetSysTime();
			for (i = 0 ; i < ecc_nr[EccBit] ; i++)
			{
				tt += GetSysTime() + v;
				mpDebugPrint("	%d - %d", i, tt);	// Without this line, tt value looks so close.
				ptr[(tt&0xff)] ^= 1<<(GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 1 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 2 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 3 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
			}
			#endif
			NandRawPageWrite(s, buf);
			memset(buf, 0x5a, 4 * MCARD_SECTOR_SIZE);
			NandSectorRead(s, 4, (DWORD)buf, NULL, NULL);
			for (i = 0 ; i < ((4 * MCARD_SECTOR_SIZE) >> 2) ; i++)
			{
				if (buf[i] != (i + s + 0xa5a5a5a5))
				{
					mpDebugPrint("->NG([%x] : 0x%x,0x%x)", i<<2, buf[i], i+s);
					//MemDump(buf, SectorPerBlk * 512);
					//break;
				}
			}
		}
		#else
		if (i >= (SectorPerBlk*(MCARD_SECTOR_SIZE>>2)))
			mpDebugPrint("Good");

		#endif
	}
	s = 512;
	for (j = 0 ; j < 0xffffffff ; j++)
	{
		//UartOutValue(s/SectorPerBlk, 8);  // Print current block number
		UartOutText("...");
#if 1
		for (i = 0 ; i < SectorPerBlk * (MCARD_SECTOR_SIZE >> 2) ; i++)
			buf[i] = 0xa5a5a5a5;
		//mpDebugPrint("test pattern:");
		//MemDump(buf, NandLog.SectorPerBlock * 512);
		if (NandBlockErase(s) != PASS)
		{
			mpDebugPrint("Block 512 erase %d times become bad block", j);
			break;
		}
		NandBlockWrite(s, (DWORD)buf, s/SectorPerBlk, NULL, SFT_NORMAL, NULL);
		//NandRawPageDump(s);
#else
		UartOutText("\r\n");
#endif
		memset(buf, 0x5A, SectorPerBlk * MCARD_SECTOR_SIZE);
#if 0
		NandSectorRead(s, 1, (DWORD)buf, NULL, NULL);
		NandSectorRead(s+1, SectorPerBlk-1, ((DWORD)buf)+512, NULL, NULL);
#else
		NandSectorRead(s, SectorPerBlk, (DWORD)buf, NULL, NULL);
#endif
		for (i = 0 ; i < SectorPerBlk * (MCARD_SECTOR_SIZE >> 2) ; i++)
		{
			if (buf[i] != (0xa5a5a5a5))
			{
				mpDebugPrint("NG([%x] : 0x%x,0x%x)", i<<2, buf[i], i+s);
				mpDebugPrint("MXIC NAND Block 512 test %d times fail)", j);
				MemDump(&buf[i&(~0x7f)], 512);
				//while(1);
				break;
			}
		}
		#if 0
		if (i >= (SectorPerBlk * (MCARD_SECTOR_SIZE >> 2)))
		{
			DWORD v = s / SectorPerBlk;
			mpDebugPrint("Good");
			NandRawPageRead(s, buf);
			NandBlockErase(s);
			#if 0
			buf[v&0x3f] ^= 0x01 << (v & 0x07);
			#else /*
			       *  Create (EccBit-1) bits error inside data section.   Designed by Logan
                   *  Using for ECC correction testing
			       */
			BYTE *ptr = (BYTE*)buf;
			DWORD tt = GetSysTime();
			for (i = 0 ; i < ecc_nr[EccBit] ; i++)
			{
				tt += GetSysTime() + v;
				mpDebugPrint("	%d - %d", i, tt);	// Without this line, tt value looks so close.
				ptr[(tt&0xff)] ^= 1<<(GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 1 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 2 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 3 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
			}
			#endif
			NandRawPageWrite(s, buf);
			memset(buf, 0x5a, 4 * MCARD_SECTOR_SIZE);
			NandSectorRead(s, 4, (DWORD)buf, NULL, NULL);
			for (i = 0 ; i < ((4 * MCARD_SECTOR_SIZE) >> 2) ; i++)
			{
				if (buf[i] != (i + s + 0xa5a5a5a5))
				{
					mpDebugPrint("->NG([%x] : 0x%x,0x%x)", i<<2, buf[i], i+s);
					//MemDump(buf, SectorPerBlk * 512);
					//break;
				}
			}
		}
		#else
		if (i >= (SectorPerBlk*(MCARD_SECTOR_SIZE>>2)))
			mpDebugPrint("Good");

		#endif
	}
	s = 1024;
	for (j = 0 ; j < 0xffffffff ; j++)
	{
		//UartOutValue(s/SectorPerBlk, 8);  // Print current block number
		UartOutText("...");
#if 1
		for (i = 0 ; i < SectorPerBlk * (MCARD_SECTOR_SIZE >> 2) ; i++)
			buf[i] = 0xa5a5a5a5;
		//mpDebugPrint("test pattern:");
		//MemDump(buf, NandLog.SectorPerBlock * 512);
		if (NandBlockErase(s) != PASS)
		{
			mpDebugPrint("Block 1024 erase %d times become bad block", j);
			break;
		}
		NandBlockWrite(s, (DWORD)buf, s/SectorPerBlk, NULL, SFT_NORMAL, NULL);
		//NandRawPageDump(s);
#else
		UartOutText("\r\n");
#endif
		memset(buf, 0x5A, SectorPerBlk * MCARD_SECTOR_SIZE);
#if 0
		NandSectorRead(s, 1, (DWORD)buf, NULL, NULL);
		NandSectorRead(s+1, SectorPerBlk-1, ((DWORD)buf)+512, NULL, NULL);
#else
		NandSectorRead(s, SectorPerBlk, (DWORD)buf, NULL, NULL);
#endif
		for (i = 0 ; i < SectorPerBlk * (MCARD_SECTOR_SIZE >> 2) ; i++)
		{
			if (buf[i] != (0xa5a5a5a5))
			{
				mpDebugPrint("NG([%x] : 0x%x,0x%x)", i<<2, buf[i], i+s);
				mpDebugPrint("MXIC NAND Block 1024 test %d times fail)", j);
				MemDump(&buf[i&(~0x7f)], 512);
				//while(1);
				break;
			}
		}
		#if 0
		if (i >= (SectorPerBlk * (MCARD_SECTOR_SIZE >> 2)))
		{
			DWORD v = s / SectorPerBlk;
			mpDebugPrint("Good");
			NandRawPageRead(s, buf);
			NandBlockErase(s);
			#if 0
			buf[v&0x3f] ^= 0x01 << (v & 0x07);
			#else /*
			       *  Create (EccBit-1) bits error inside data section.   Designed by Logan
                   *  Using for ECC correction testing
			       */
			BYTE *ptr = (BYTE*)buf;
			DWORD tt = GetSysTime();
			for (i = 0 ; i < ecc_nr[EccBit] ; i++)
			{
				tt += GetSysTime() + v;
				mpDebugPrint("	%d - %d", i, tt);	// Without this line, tt value looks so close.
				ptr[(tt&0xff)] ^= 1<<(GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 1 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 2 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
				tt = (tt+1)<<1;
				ptr[(tt&0xff) + 3 * MCARD_SECTOR_SIZE] ^= 1 << (GetSysTime() & 0x07);
			}
			#endif
			NandRawPageWrite(s, buf);
			memset(buf, 0x5a, 4 * MCARD_SECTOR_SIZE);
			NandSectorRead(s, 4, (DWORD)buf, NULL, NULL);
			for (i = 0 ; i < ((4 * MCARD_SECTOR_SIZE) >> 2) ; i++)
			{
				if (buf[i] != (i + s + 0xa5a5a5a5))
				{
					mpDebugPrint("->NG([%x] : 0x%x,0x%x)", i<<2, buf[i], i+s);
					//MemDump(buf, SectorPerBlk * 512);
					//break;
				}
			}
		}
		#else
		if (i >= (SectorPerBlk*(MCARD_SECTOR_SIZE>>2)))
			mpDebugPrint("Good");

		#endif
	}
	ext_mem_free(buf);
	NandConfigForNormal();
}

MPX_KMODAPI_SET(Nand_RW_Test);
#endif

#endif

#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND) && (NAND_DRV == NAND_SIMPLE_DRV))
static void IspCommandProcess(void *pMcardDev)
{
	ST_MCARD_DEV *pDev = (ST_MCARD_DEV *) pMcardDev;
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;
	DWORD tmp;

	mail->swStatus = PASS;
	SetMcardClock(NAND_CLOCK_KHZ);
	//mpDebugPrint("NandIsp(%x) Command: %x(%x,%d)", pDev->wMcardType, mail->wCmd, mail->dwBlockAddr, mail->dwBlockCount);
	McardNandActive();
	switch (mail->wCmd)
	{
		case CODE_IDENTIFY_CMD:
			{
				static BYTE identified = FALSE;
				DWORD blknr, sectperblk, pagesize;
				if (identified == FALSE)
				{
					NandIdentify();
					BootPageCheck();
					// unknown problem -> 用SLC時會發生malloc fail
					tmp = ext_mem_get_free_space();
					mpDebugPrint("%s - mem free space = %d", __FUNCTION__, tmp);
					NandLogicalInit();
					identified = TRUE;
				}
				GetNandGeometry(&blknr, &sectperblk, &pagesize);
				((DWORD *)mail->dwBuffer)[0] = sectperblk;		// Sector per block
				((DWORD *)mail->dwBuffer)[1] = blknr;			// total block number
				((DWORD *)mail->dwBuffer)[2] = pagesize;		// Page size
			}
			break;

		case ISP_READ_CMD:
			mail->swStatus = NandSectorRead(mail->dwBlockAddr, mail->dwBlockCount, mail->dwBuffer, NULL, NULL);
			break;

		case ISP_WRITE_CMD:
			{
				DWORD i;
				DWORD addr = mail->dwBlockAddr, buf = mail->dwBuffer;
				for (i = 0 ; i < mail->dwBlockCount ; i++)
				{
					mail->swStatus = NandBlockWrite(addr, buf, 0xffff, NULL, SFT_NORMAL, NULL);
					if (mail->swStatus)
						break;
					addr += NandLog.SectorPerBlock;
					buf += NandLog.SectorPerBlock << MCARD_SECTOR_SIZE_EXP;
				}
			}
			break;

		case CODE_BLK_INVALID_CMD:
			{
				DWORD logblk = GetNandLogBlkAddr(mail->dwBlockAddr, LOG_BLK_PER_ZONE);
				if ((logblk == LB_DEFAULT_BAD_BLOCK) || (logblk == LB_BAD_BLOCK))
					((DWORD *)mail->dwBuffer)[0] = TRUE;
				else
					((DWORD *)mail->dwBuffer)[0] = FALSE;
			}
			break;

		case CODE_BLK_ERASE_CMD:
			tmp = NandBlockErase(mail->dwBlockAddr);
			if (tmp != PASS)
				NandBlockWrite(mail->dwBlockAddr, NULL, 0xFFFF, NULL, SFT_BAD_BLOCK, NULL);	// mark as bad
			((DWORD *)mail->dwBuffer)[0] = tmp;
			break;

		default:
			MP_DEBUG("-E- INVALID CMD");
			break;
	}
	McardNandInactive();
}

void NandIspInit(ST_MCARD_DEV * sDev)
{
	MP_DEBUG(__FUNCTION__);
	sDev->pbDescriptor = "Nand Isp";
	sDev->wMcardType = DEV_NAND_ISP;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = IspCommandProcess;
}
#endif

#endif
