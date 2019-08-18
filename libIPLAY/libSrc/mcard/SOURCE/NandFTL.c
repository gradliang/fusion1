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
* Filename      : NandFTL.c
* Programmer(s) :
* Created       :
* Descriptions  : FTL driver which implement a flash memory tanslation layer
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
#if (NAND_ENABLE  && (NAND_DRV == NAND_FTL_DRV))
#include "mpTrace.h"

#include "Mcard.h"
#include "McardApi.h"
#include "uti.h"
#include "NandFTL.h"
#include "nand.h"

/*
// Constant declarations
*/
#define BLK_TAB_DUMP			0
#define BLK_ERS_TAB_DUMP		0
//#define NAND_FTL_DEBUG
//#define D3
#define CACHEPAGE_MECHANISM     1    // For enhancing non-page aligned tail data.   from Oliver            !! Looks ok(can be used) !!
#define SEQUENTIAL_MECHANISM    0    // Asure pages inside data block are keeping in sequential order.     !! UNDER CONSTRUCTION !!

#define NAND_BLOCK_PER_ZONE_EXP	 14
#define NAND_BLOCK_PER_ZONE		 (1 << NAND_BLOCK_PER_ZONE_EXP)
#define FREE_BLOCK_EXP      	 5
#define MAX_BLOCK_NUM        	 ((1 << NAND_BLOCK_PER_ZONE_EXP) - 1)
#define MAX_PAGE_NUM        	 ((1 << 28) - 1)
#define FTL_CACHE_SET_EXP   	 2
#define MAX_ZONE    			 8
#define EXP_EXPECT_BADBLOCK   	 6
#define NAND_BUF_SIZE 			 (128 * 1024)
#define NAND_ERASE_CNT_VER		 20100305    // To change erase count stored structure, this version has to be changed.
#define NAND_WL_READ_THRESHOLD	 1           // How many bits will the page reach the max correctable bits?
									         // If meet threshold, do wear-leveling.
#define NAND_WL_READ_FUNC		 1
#define NAND_WL_WRITE_FUNC		 1

/*
// Structure declarations
*/
typedef struct RING_BUFFER
{
	DWORD *Buffer;	// please allocate memory for it.
	DWORD head;
	DWORD tail;
} ST_RINGBUFFER;

typedef struct FTL_PT_CACHE_LINE
{
    DWORD u32LogicBlock;
    DWORD u32PhyBlock;        // cache tag
    DWORD u32LRU;             // LRU replacement policy
    DWORD u32ReplaceBlock;    // replace block Link for temp page store
    DWORD u32FreePageNum;     // free page count
    DWORD *pu32PageTable;     // mapping table
} ST_FTL_PT_CACHE_LINE;

typedef struct FTL_PAGE_TANSLATION_CACHE
{
    WORD u16SetNum;
    BYTE u08LinePSet;
    BYTE u08LinPSetExp;

    ST_FTL_PT_CACHE_LINE *CacheBuf;
} ST_FTL_PAGE_TANSLATION_CACHE;

typedef struct NAND_FTL_STRUCT
{
    WORD u16BlockPerZone;
    WORD u16DataBlockPerZone;

    BYTE u08ZoneNum;
    BYTE u08BlockPerZoneExp;
    BYTE u08PagePerBlockExp;
	BYTE u08SectorPerBlockExp;
#if CACHEPAGE_MECHANISM
	WORD u16SectorPerBlock;
//	BYTE rev[2];
#endif
    BYTE u08SectorPerPageExp;
    BYTE u08SectorPerPage;

    WORD u16PagePerBlock;
    WORD u16PageSize;

    WORD **pBlockTranslation;
    WORD **pReplaceBlock;
    WORD u16FreeBlockCount[MAX_ZONE];
    ST_RINGBUFFER pFreeBlockTable[MAX_ZONE];
#if CACHEPAGE_MECHANISM
    DWORD u32CacheLogAddress;
    DWORD u32CacheBaseAddress;
    WORD  u16CacheBlock;
    WORD  u16FreePageIndex;
    WORD  u16CacheDataMissTimes;
    BYTE  u08CacheBufferUsing;
    BYTE  u08CacheDataValid;
#endif
    ST_FTL_PAGE_TANSLATION_CACHE FTLCache;
	DWORD ReadOnly;

	// For wear-leavling
	DWORD u32PowerUpCnt;
	DWORD bEraseCntUpdate;
	DWORD *pEraseCnt;
	DWORD dwMinErsCntBlk;
} ST_NAND_FTL_STRUCT;


/*
// Type declarations
*/

/*
// Variable declarations
*/
static BYTE identified            = FALSE;
static BYTE bDescriptor[]         = "NandFTL";
static DWORD badblock_nr          = 0;
static DWORD skipBlockNum         = 0;
static ST_NAND_FTL_STRUCT nandFTL = {0};
static BYTE *IspBadBlkTable       = NULL;   // speed up purpose for ISP area
static BYTE *CachePtr             = NULL;
static BOOL CacheValid            = FALSE;
static DWORD CachePageAddr        = -1;
static DWORD *pEraseCntTab        = NULL;
static DWORD EraseCntThreshold    = 1024;   // threshold to perform a wear-leveling

/*
// Macro declarations
*/
// Wear-leveling macro define
#define ERASE_CNT_INC(blk) \
{ \
	nandFTL.pEraseCnt[blk]++; \
	if (nandFTL.dwMinErsCntBlk == blk) \
		ftlWLStatistics(); \
	nandFTL.bEraseCntUpdate = 1; \
}
#define ERASE_CNT_INVALID(blk) \
{ \
	nandFTL.pEraseCnt[blk] = 0xffffffff; \
	nandFTL.bEraseCntUpdate = 1; \
}


/*
// Static function prototype
*/
static void CommandProcess(void *pMcardDev);
static SWORD LowLevelFormat(BYTE deepVerify);
static DWORD FTLWrite(DWORD buffer, DWORD PageCnt, DWORD dwLogAddr, DWORD *progress);

static DWORD CAL_SKIP_BLKS(DWORD blksz, DWORD revSize)
{
	return (1 + ((revSize) / (blksz)));
}


void DumpFTLTables()
{
#if 0	// For debugging
	DWORD i, j;
	WORD **BlockMap = nandFTL.pBlockTranslation;
	WORD **ReplaceMap = nandFTL.pReplaceBlock;

	for(i = 0; i < nandFTL.u08ZoneNum; i++)
	{
		for(j = 0; j < nandFTL.u16DataBlockPerZone; j++)	// logical block j
		{
			if (BlockMap[i][j] < nandFTL.u16BlockPerZone)
				mpDebugPrint("Block log %d -> phy %d", (i<<nandFTL.u08BlockPerZoneExp)+j, BlockMap[i][j]);
			if (ReplaceMap[i][j] < nandFTL.u16BlockPerZone)
				mpDebugPrint("\tReplace log %d -> phy %d", (i<<nandFTL.u08BlockPerZoneExp)+j, ReplaceMap[i][j]);
		}
	}
#else
	return;
#endif
}


/*
// Definition of local functions
*/
#if 0
static void ftlWLStatistics()
{
	DWORD i, max = 0, min = -1, max_blk = -1;

	McardNandActive();
	for (i = skipBlockNum ; i < nandFTL.u08ZoneNum * nandFTL.u16BlockPerZone ; i++)
	{
        DWORD rowAddr = i << nandFTL.u08PagePerBlockExp;
		DWORD logblk1 = GetNandLogAddr(rowAddr);
		DWORD logblk2 = GetNandLogAddr(rowAddr+nandFTL.u16PagePerBlock-1);

        //check bad block
        if ((logblk1 > LB_ERROR && logblk1 < LB_NO_ADDR)
			|| (logblk2 > LB_ERROR && logblk2 < LB_NO_ADDR))
        {
			ERASE_CNT_INVALID(i);
        }
		else if (nandFTL.pEraseCnt[i]!= 0xffffffff && nandFTL.pEraseCnt[i] > max)
		{
			max = nandFTL.pEraseCnt[i];
			max_blk = i;
		}
		else if (nandFTL.pEraseCnt[i] < min)
		{
			min = nandFTL.pEraseCnt[i];
			nandFTL.dwMinErsCntBlk = i;
		}
		else if (nandFTL.pEraseCnt[i] == 0xffffffff)
		{
			nandFTL.pEraseCnt[i] = 0;
		}
	}
	McardNandInactive();
	mpDebugPrint("\tNand erase count: max:%d(%d), min:%d(%d)", max_blk, max, nandFTL.dwMinErsCntBlk, min);
}
#else
static void ftlWLStatistics()
{
	DWORD i, min = -1;

	for (i = skipBlockNum ; i < nandFTL.u08ZoneNum * nandFTL.u16BlockPerZone ; i++)
	{
		if (nandFTL.pEraseCnt[i] < min)
		{
			min = nandFTL.pEraseCnt[i];
			nandFTL.dwMinErsCntBlk = i;
		}
	}
	MP_DEBUG("\tNand erase count: min:%x(%d)", nandFTL.dwMinErsCntBlk, min);
}
#endif

#if NAND_WL_WRITE_FUNC
static void ftlWLBlockCopy(DWORD dst, DWORD src)
{
	BYTE *ptr = (BYTE *)((DWORD)ker_mem_malloc(nandFTL.u16PageSize, TaskGetId())|BIT29);
	DWORD k;

	for (k = 0 ; k < nandFTL.u16PagePerBlock ; k++)
	{
		DWORD logicPage = GetNandLogAddr(src+k);
		if(logicPage == LB_NO_ADDR)	     // no address information. means free page
		{
			break;
		}
		else if (logicPage > LB_ERROR)   // or > nandFTL.u16DataBlockPerZone
		{
			mpDebugPrint("\tLogical address unavailable!(%x,%x)", src+k, logicPage);
		}
		else	// Get a logical address
		{
			NandPageCopy(dst+k, src+k, logicPage, ptr);
		}
	}

	ker_mem_free(ptr);
}

static void ftlWearLeveling(BYTE zone)
{
	ST_RINGBUFFER *free_tab = &nandFTL.pFreeBlockTable[zone];
	DWORD ring_mask = (1 << nandFTL.u08BlockPerZoneExp) - 1;
	DWORD n, max = 0, target_idx = 0;

	if (free_tab->Buffer[free_tab->head] == nandFTL.dwMinErsCntBlk)
	{	// next free block is min used one.
		return;
	}

	// free block list traversal, found a mass used block
	for(n = free_tab->head ; n != free_tab->tail ; n = (n+1)&ring_mask)
	{
		if (nandFTL.pEraseCnt[free_tab->Buffer[n]] > max)
		{
			max = nandFTL.pEraseCnt[free_tab->Buffer[n]];
			target_idx = n;
		}
	}

	DWORD Block = free_tab->Buffer[target_idx];
	if ((nandFTL.pEraseCnt[Block] > nandFTL.pEraseCnt[nandFTL.dwMinErsCntBlk])
		&& ((nandFTL.pEraseCnt[Block] - nandFTL.pEraseCnt[nandFTL.dwMinErsCntBlk]) > EraseCntThreshold))
	{	// swap them
		DWORD tmr = GetSysTime();
		DWORD i, j;
		BOOL found = FALSE;
		MP_DEBUG("WL:free block %x used %d times(>%d of block %x)", Block, nandFTL.pEraseCnt[Block], nandFTL.pEraseCnt[nandFTL.dwMinErsCntBlk], nandFTL.dwMinErsCntBlk);
		// 1. search min-erase-count block in free block list
		for (i = free_tab->head; i != free_tab->tail; i = (i+1) & ring_mask)
		{
			if (free_tab->Buffer[i] == nandFTL.dwMinErsCntBlk) // find the block of min erase count in free list
			{
				MP_DEBUG("\tWear-Leveling from FREE block %x(%d ms)", free_tab->Buffer[i], GetSysTime()-tmr);
				WORD tmp = free_tab->Buffer[i];
				// move the min to head of ring buffer
				free_tab->Buffer[i] = free_tab->Buffer[free_tab->head];
				free_tab->Buffer[free_tab->head] = tmp;
				found = TRUE;
				break;
			}
		}
		// 2. search min-erase-count block in replace block list
		// 3. search min-erase-count block is data block list
		DWORD dst = -1, src = -1;
		if (found != TRUE)
		{
			WORD **replace = nandFTL.pReplaceBlock;
			WORD **data = nandFTL.pBlockTranslation;
			found = FALSE;
			for (j = 0 ; j < nandFTL.u08ZoneNum ; j++)
			{
				for (i = 0 ; i < nandFTL.u16DataBlockPerZone ; i++)
				{
					if (replace[j][i] == nandFTL.dwMinErsCntBlk) // find the block of min erase count in replace list
					{
						dst = free_tab->Buffer[target_idx];
						src = replace[j][i];

						ftlWLBlockCopy(dst << nandFTL.u08PagePerBlockExp, src << nandFTL.u08PagePerBlockExp);
						// swap phyical block address
						replace[j][i] = dst;
						free_tab->Buffer[target_idx] = src;
						// erase to free it
						NandBlockErase(src << nandFTL.u08PagePerBlockExp);
						ERASE_CNT_INC(src);
						found = TRUE;
						MP_DEBUG("\tWear-Leveling from REPLACE block [%d][%d]%x(%d ms)", j, i, free_tab->Buffer[target_idx], GetSysTime()-tmr);
						break;
					}
					else if (data[j][i] == nandFTL.dwMinErsCntBlk)	// find the block of min erase count in data list
					{
						dst = free_tab->Buffer[target_idx];
						src = data[j][i];

						ftlWLBlockCopy(dst << nandFTL.u08PagePerBlockExp, src << nandFTL.u08PagePerBlockExp);
						// swap phyical block address
						data[j][i] = dst;
						free_tab->Buffer[target_idx] = src;
						// erase to free it
						NandBlockErase(src << nandFTL.u08PagePerBlockExp);
						ERASE_CNT_INC(src);
						found = TRUE;
						MP_DEBUG("\tWear-Leveling from DATA block [%d][%d]%x(%d ms)", j, i, free_tab->Buffer[target_idx], GetSysTime()-tmr);
						break;
					}
				}
			}
			// 4. update cache
			if (found == TRUE)
			{
				ST_FTL_PT_CACHE_LINE *tmpCacheLine = nandFTL.FTLCache.CacheBuf;
				for(i = 0 ; i < (nandFTL.FTLCache.u16SetNum << FTL_CACHE_SET_EXP) ; i++)
				{
					if ((tmpCacheLine[i].u32PhyBlock == src) || (tmpCacheLine[i].u32ReplaceBlock == src))
					{
						MP_DEBUG("\tRemove from cache (phy%x)", src);
						tmpCacheLine[i].u32LogicBlock = 0xffffffff;
						tmpCacheLine[i].u32PhyBlock = 0xffffffff;
						tmpCacheLine[i].u32ReplaceBlock = 0xffffffff;
						tmpCacheLine[i].u32LRU = 0;
						tmpCacheLine[i].u32FreePageNum = 0;
					}
				}
				CacheValid = FALSE;
				CachePageAddr = -1;
			}
			else
			{
				DumpFTLTables();
			}
		}
	}
}
#endif

static DWORD ftlPhysicBlockGet(DWORD logicBlock)
{
	if (logicBlock < nandFTL.u16DataBlockPerZone)
	{
		return nandFTL.pBlockTranslation[0][logicBlock];
	}
	else
	{
	    DWORD zone = logicBlock / nandFTL.u16DataBlockPerZone;
	    return (zone * nandFTL.u16DataBlockPerZone) + nandFTL.pBlockTranslation[zone][logicBlock % nandFTL.u16DataBlockPerZone];
	}
}

static DWORD ftlPhysicReplaceBlockGet(DWORD logicBlock)
{
	if (logicBlock < nandFTL.u16DataBlockPerZone)
	{
		return nandFTL.pReplaceBlock[0][logicBlock];
	}
	else
	{
	    DWORD zone = logicBlock / nandFTL.u16DataBlockPerZone;
	    return (zone * nandFTL.u16DataBlockPerZone) + nandFTL.pReplaceBlock[zone][logicBlock % nandFTL.u16DataBlockPerZone];
	}
}

static DWORD ftlFreeBlockGet(BYTE zone)
{
	DWORD Block = 0xffff;
	ST_RINGBUFFER *free_tab = &nandFTL.pFreeBlockTable[zone];
	DWORD ring_mask = (1 << nandFTL.u08BlockPerZoneExp) - 1;

	if (free_tab->head != free_tab->tail)
	{
		Block = free_tab->Buffer[free_tab->head];
		free_tab->head = (free_tab->head + 1) & ring_mask;
		nandFTL.u16FreeBlockCount[zone]--;
#ifdef NAND_FTL_DEBUG
		mpDebugPrint("ftlFreeBlockGet %d(%d)", Block, nandFTL.u16FreeBlockCount[zone]);
#endif
	}

	return Block;
}

static void ftlPhysicBlockSet(DWORD logicBlock, DWORD physicBlock)
{
	if (logicBlock < nandFTL.u16DataBlockPerZone)
	{
		nandFTL.pBlockTranslation[0][logicBlock] = physicBlock;
	}
	else
	{
		DWORD zone = physicBlock >> nandFTL.u08BlockPerZoneExp;
		DWORD zoneMask = (1 << nandFTL.u08BlockPerZoneExp) - 1;

		nandFTL.pBlockTranslation[zone][logicBlock / nandFTL.u16DataBlockPerZone] = (WORD)(physicBlock & zoneMask);
	}
}

static void ftlReplacedBlockSet(DWORD logicBlock, DWORD physicBlock)
{
	if (logicBlock < nandFTL.u16DataBlockPerZone)
	{
		nandFTL.pReplaceBlock[0][logicBlock] = physicBlock;
	}
	else
	{
		DWORD zone = physicBlock >> nandFTL.u08BlockPerZoneExp;
		DWORD zoneMask = (1 << nandFTL.u08BlockPerZoneExp) - 1;

		nandFTL.pReplaceBlock[zone][logicBlock / nandFTL.u16DataBlockPerZone] = (physicBlock == 0xFFFF) ? 0xFFFF : (physicBlock & zoneMask);
	}
}

//Add "Block" into free block table
static SWORD ftlFreeBlockSet(DWORD Block)
{
	DWORD zone = Block >> nandFTL.u08BlockPerZoneExp;
	ST_RINGBUFFER *free_tab = &nandFTL.pFreeBlockTable[zone];
	DWORD ring_mask = (1 << nandFTL.u08BlockPerZoneExp) - 1;
	SWORD ret = PASS;

#ifdef NAND_FTL_DEBUG
	mpDebugPrint("ftlFreeBlockSet %d(%d)", Block, nandFTL.u16FreeBlockCount[zone]);
#endif

	if (((free_tab->tail + 1) & ring_mask) == free_tab->head)	// buffer full
	{
		mpDebugPrint("free block is overflow, %d", Block);
		ret = FAIL;
	}
	else
	{
		free_tab->Buffer[free_tab->tail] = Block;
		free_tab->tail = (free_tab->tail + 1) & ring_mask;
		nandFTL.u16FreeBlockCount[zone]++;
	}

	return ret;
}

static DWORD *ftlPtCacheFill(DWORD logicBlock, DWORD physicBlock, ST_FTL_PT_CACHE_LINE *LRUCache)
{
	DWORD *ptBuf = NULL;
	DWORD rowAddr;
	DWORD logicPage;
	DWORD replaceBlock = 0;
	BOOL ReplaceBlkFound = FALSE;

#ifdef D3
	mpDebugPrint("ftlPtCacheFill, phyBlk=%d, %x", physicBlock, LRUCache);
#endif
	if (physicBlock == 0xFFFF)	// physical block is a new free block.
	{
		DWORD i;

		physicBlock = ftlFreeBlockGet(logicBlock / nandFTL.u16DataBlockPerZone);
		if (physicBlock >= nandFTL.u16BlockPerZone)
		{
			mpDebugPrint("NandFTL: FATAL error! no free physical block for unmap logical block!");
		}
		else
		{
			ptBuf = LRUCache->pu32PageTable;
			for(i = 0 ; i < nandFTL.u16PagePerBlock ; i++)
				ptBuf[i] = 0xffffffff;
			LRUCache->u32PhyBlock = physicBlock;
			LRUCache->u32ReplaceBlock = 0xffffffff;
			LRUCache->u32LRU = 0;
			LRUCache->u32FreePageNum = nandFTL.u16PagePerBlock;
			ftlPhysicBlockSet(logicBlock, physicBlock);
		}
	}
	else  // physical block is not a new one, build page table!
	{
		DWORD zone = physicBlock >> nandFTL.u08BlockPerZoneExp;
		DWORD pagePerBlockMask = nandFTL.u16PagePerBlock - 1;

		rowAddr = physicBlock << nandFTL.u08PagePerBlockExp;
		ptBuf = LRUCache->pu32PageTable;

		//memset(ptBuf, 0xff, nandFTL.u16PagePerBlock << 2);   //init page translation cache
		//clean page translation cache
		{
			DWORD i;
			for(i = 0 ; i < nandFTL.u16PagePerBlock ; i++)
				ptBuf[i] = 0xffffffff;
		}

		// parse all page in the Block
		LRUCache->u32PhyBlock = physicBlock;
		LRUCache->u32ReplaceBlock = 0xffffffff;
		LRUCache->u32LRU = 0;
		LRUCache->u32FreePageNum = 0;
		while(1)
		{
			logicPage = GetNandLogAddr(rowAddr);
			//mpDebugPrint("[%d.%d]: %x", physicBlock, rowAddr, logicPage);
			if (logicPage == LB_NO_ADDR)	// no address information. means free page
			{
				LRUCache->u32FreePageNum = nandFTL.u16PagePerBlock - (rowAddr & pagePerBlockMask);
				break;
			}

			/*
			 *  If logical page > LB_INVALID_ADDR_LOWBOUND, this page is abandoned!!
			 *
			 *  By our nand design rule, if we use 0xfffffff0 as logical address
			 *  when calling function -- "NandPageWrite()".
			 *  We will get 0x7ffffff0(LB_INVALID_ADDR_LOWBOUND) here.
			 */
			//else if (logicPage > LB_ERROR)	// or > nandFTL.u16DataBlockPerZone
			else if (logicPage >= LB_INVALID_ADDR_LOWBOUND)
			{
				if(logicPage != LB_INVALID_ADDR_LOWBOUND)
					mpDebugPrint("\tLogical address unavailable!(%x,%x)", rowAddr, logicPage);
			}
			else	// got a logical address
			{
				ptBuf[logicPage & pagePerBlockMask] = rowAddr;
			}
			rowAddr++;
			if((rowAddr & pagePerBlockMask) == 0)  //the last page of Block
			{
				if(ReplaceBlkFound == TRUE)
					break;

				// Get replace block from replace block table, we also need to parse it for building page table.
				replaceBlock = ftlPhysicReplaceBlockGet(logicBlock);
				if(replaceBlock < nandFTL.u16BlockPerZone)
				{
					replaceBlock += zone << nandFTL.u08BlockPerZoneExp;
					MP_DEBUG("cache fill-logBlk %d , phy %d ,rep %d ", logicBlock, physicBlock, replaceBlock);
					LRUCache->u32ReplaceBlock = replaceBlock;
					rowAddr = replaceBlock << nandFTL.u08PagePerBlockExp;
					ReplaceBlkFound = TRUE;
				}
				else
				{
					break;
				}
			}
		}
		//mpDebugPrint("Current block(%d, %d): %d....", logicBlock, physicBlock, LRUCache->u32FreePageNum);
	}

#if CACHEPAGE_MECHANISM
	if( nandFTL.u32CacheLogAddress != 0xFFFFFFFF ) {
		if( ( nandFTL.u32CacheLogAddress/nandFTL.u16SectorPerBlock ) == logicBlock ) {

			rowAddr = nandFTL.u16CacheBlock << nandFTL.u08PagePerBlockExp;
			rowAddr = rowAddr + nandFTL.u16FreePageIndex - 1;
			logicPage = nandFTL.u32CacheLogAddress >> nandFTL.u08SectorPerPageExp;
			logicPage = logicPage & ( nandFTL.u16PagePerBlock - 1);
			ptBuf[ logicPage ] = rowAddr;
		}
	}
#endif

	return ptBuf;
}


// Get logical block information.
// If cache miss happens, call ftlPtCacheFill() to write logical block information into cahe line.
static ST_FTL_PT_CACHE_LINE *ftlPtCacheGet(DWORD logicBlock, DWORD physicBlock)
{
	DWORD i;
	DWORD cacheSet;
	DWORD maxLRU = 0;
	ST_FTL_PT_CACHE_LINE *tmpCacheLine, *ptCacheLine = 0;
	DWORD *ptBuf = NULL;

	cacheSet = logicBlock % nandFTL.FTLCache.u16SetNum;
	tmpCacheLine = &nandFTL.FTLCache.CacheBuf[cacheSet << nandFTL.FTLCache.u08LinPSetExp];

	for(i=0; i<nandFTL.FTLCache.u08LinePSet; i++)
	{
		if(tmpCacheLine->u32PhyBlock == physicBlock) // cache hit!
		{
			ptBuf = tmpCacheLine->pu32PageTable;
			ptCacheLine = tmpCacheLine;
			tmpCacheLine->u32LRU = 0;
		}
		else
		{
			if ((tmpCacheLine->u32LRU + 1) != 0)
				tmpCacheLine->u32LRU++;	      // aging
			if(!ptBuf && maxLRU < tmpCacheLine->u32LRU)
			{
				ptCacheLine = tmpCacheLine;	  // pick up LRU one
				maxLRU = tmpCacheLine->u32LRU;
			}
		}
		tmpCacheLine++;
	}

	MP_DEBUG("%s: cache %d, %x, %x(%d)", __FUNCTION__, cacheSet, &nandFTL.FTLCache.CacheBuf[cacheSet << nandFTL.FTLCache.u08LinPSetExp], ptCacheLine, i);
	if(!ptBuf)  //cache miss
	{
#ifdef D3
		mpDebugPrint("cache miss");
#endif
		ptBuf = ftlPtCacheFill(logicBlock, physicBlock, ptCacheLine);	// cache out the LRU one
	}

	ptCacheLine->u32LogicBlock = logicBlock;

	return ptCacheLine;
}

static DWORD ftlContValidPageGet(DWORD *lba, DWORD maxPage)
{
	DWORD logicBlock, physicBlock;
	DWORD *ptBuf;
	DWORD offset;
	DWORD prevPage, currPage;
	DWORD count = 0;

	BYTE SecPBlockExp = nandFTL.u08SectorPerBlockExp;
	BYTE SecPPageExp = nandFTL.u08SectorPerPageExp;

#ifdef D3
	mpDebugPrint("	ftlContValidPageGet");
	mpDebugPrint("	R: log lba:%d, maxPage:%d, Block:%5d",*lba, maxPage, *lba >> nandFTL.u08SectorPerBlockExp);
#endif
	logicBlock = *lba >> SecPBlockExp;
	physicBlock = ftlPhysicBlockGet(logicBlock);
	if (physicBlock != 0xffff)	// not free block
	{
#ifdef D3
		mpDebugPrint("	phBlk=%d", physicBlock);
#endif
		ptBuf = ftlPtCacheGet(logicBlock, physicBlock)->pu32PageTable;
		// get page offset within the Block
		offset = (*lba >> SecPPageExp) & (nandFTL.u16PagePerBlock - 1);

#ifdef D3
		mpDebugPrint("	offset = %d, prevPage=%x", offset, ptBuf[offset]);
#endif
		prevPage = ptBuf[offset++];
		if(prevPage != 0xffffffff)
		{
			count++;
			*lba = prevPage << nandFTL.u08SectorPerPageExp; // Save physical sector address
			while((offset < nandFTL.u16PagePerBlock) && (count < maxPage))
			{
				currPage = ptBuf[offset++];
				if(prevPage == (currPage - 1))
				{
					count++;
					prevPage = currPage;
				}
				else
				{
					break;
				}
			}
		}
		//else
		//all pages are free
#ifdef D3
		mpDebugPrint("	R: phy lba=%d, count=%d, Block=%5d",*lba, count, *lba >> nandFTL.u08SectorPerBlockExp);
#endif
	}

	return count;
}

static DWORD ftlContReadSectorGet(DWORD *lba, DWORD totalSec)
{
	DWORD phyLba;
	DWORD contPage;
	DWORD contSec;
	DWORD secOffset;
#ifdef D3
	mpDebugPrint("	ftlContReadSectorGet, lba=%d, totalSec=%d", *lba, totalSec);
#endif

	BYTE SecPPage = nandFTL.u08SectorPerPage;
	BYTE SecPPageExp = nandFTL.u08SectorPerPageExp;

	phyLba= *lba;
	contPage = ftlContValidPageGet(&phyLba, (totalSec>>SecPPageExp)+1);
	if(contPage)
	{
		secOffset = *lba & (SecPPage-1);
		*lba = phyLba + secOffset;

		contSec = (contPage << SecPPageExp) - secOffset;
		if(contSec > totalSec)
			contSec = totalSec;
	}
	else
	{
		contSec = 0;
	}
#ifdef D3
	mpDebugPrint("	lba=%d, contSec=%d", *lba, contSec);
#endif

	return contSec;
}

static SWORD ftlPartialMerge(ST_FTL_PT_CACHE_LINE *ptCacheLine)
{
	DWORD i, validPageNum;
	DWORD currDataBlock;
	DWORD *ptBuf;
	BYTE pagePerBlockExp;

	// Partial merging is not good for performance, may take off this section soon!
	return FAIL;

	if ((ptCacheLine->u32ReplaceBlock == 0xffffffff) || (ptCacheLine->u32FreePageNum == 0))
		return FAIL;

	currDataBlock = ptCacheLine->u32PhyBlock;
	pagePerBlockExp = nandFTL.u08PagePerBlockExp;
	ptBuf = ptCacheLine->pu32PageTable;

	//1. Check valid page number in the Data Block
	validPageNum = 0;
	for(i = 0; i < nandFTL.u16PagePerBlock; i++)
	{
		if((ptBuf[i]>>pagePerBlockExp) == currDataBlock)    // found valid page in data block
		{
			validPageNum++;
			if(validPageNum > ptCacheLine->u32FreePageNum)
				return FAIL;
		}
	}

	DWORD PageNrMask = nandFTL.u16PagePerBlock - 1;
	BYTE *dataBuf = (BYTE *)((DWORD)ext_mem_malloc(nandFTL.u16PageSize) | 0xA0000000);
	DWORD LogAddr = ptCacheLine->u32LogicBlock << pagePerBlockExp;
	DWORD trgPage = ptCacheLine->u32ReplaceBlock << pagePerBlockExp;
	trgPage += (nandFTL.u16PagePerBlock - ptCacheLine->u32FreePageNum);

	//2. Copy valid page in data block to replaced block
	for(i = 0 ; i < nandFTL.u16PagePerBlock ; i++)
	{
		if((ptBuf[i]>>pagePerBlockExp) == currDataBlock)   // valid page in data block
		{
			DWORD addr;
			if ((ptBuf[i] & PageNrMask) == PageNrMask)     // Last page using special logical address tag
				addr = LogAddr + i;
			else
				addr = -1;                                 // copy log addr field directly, please reference NandPageCopy()
			if (NandPageCopy(trgPage, ptBuf[i], addr, dataBuf) != PASS)
			{
				mpDebugPrint("Bad block occur when partial merge!");
				break;
			}
			ptCacheLine->u32FreePageNum--;
			ptBuf[i] = trgPage; //update page translation table
			trgPage++;
		}
	}
	ext_mem_free(dataBuf);
#ifdef NAND_FTL_DEBUG
	mpDebugPrint("PM(%x):%x=>%x", ptCacheLine->u32LogicBlock, currDataBlock, ptCacheLine->u32ReplaceBlock);
#endif

	//3. free the orignal Data Block
	if (NandBlockErase(currDataBlock << pagePerBlockExp) == FAIL)
	{
		ERASE_CNT_INVALID(currDataBlock);
		badBlockMark(currDataBlock << pagePerBlockExp);
	}
	else
	{
		ERASE_CNT_INC(currDataBlock);
		ftlFreeBlockSet(currDataBlock);
	}

	//4. let the orignal replaced Block to become to Data Block
	ptCacheLine->u32PhyBlock = ptCacheLine->u32ReplaceBlock;
	ftlPhysicBlockSet(ptCacheLine->u32LogicBlock, ptCacheLine->u32PhyBlock);
	ftlReplacedBlockSet( ptCacheLine->u32LogicBlock, 0xFFFF);
	ptCacheLine->u32ReplaceBlock = 0xffffffff;

#ifdef NAND_FTL_DEBUG
	mpDebugPrint("PMEND:  Phy:%5d  Rep:%5d     Log:%5d   FPG:%5d", ptCacheLine->u32PhyBlock, ptCacheLine->u32ReplaceBlock, ptCacheLine->u32LogicBlock, ptCacheLine->u32FreePageNum);
#endif

	return PASS;
}

static SWORD ftlFullMerge(ST_FTL_PT_CACHE_LINE *ptCacheLine, DWORD newBlock)
{
	DWORD i;
	SWORD ret = PASS;
	DWORD currDataBlock, replacedBlock;
	DWORD *ptBuf;
	DWORD TargetPage;
	BYTE *dataBuf = (BYTE *)((DWORD)ext_mem_malloc(nandFTL.u16PageSize) | 0xA0000000);

	currDataBlock = ptCacheLine->u32PhyBlock;
	replacedBlock = ptCacheLine->u32ReplaceBlock;
	TargetPage = newBlock << nandFTL.u08PagePerBlockExp;
	ptCacheLine->u32FreePageNum = nandFTL.u16PagePerBlock;

	//0. set the logic block address to the new block's spare area

	//1. do merge procedure that copy valid page in the Data Block and Replaced Block to new allocated Block
	ptBuf = ptCacheLine->pu32PageTable;
	DWORD LogAddr = ptCacheLine->u32LogicBlock<<nandFTL.u08PagePerBlockExp;
	DWORD PageNrMask = nandFTL.u16PagePerBlock - 1;
	for(i = 0 ; i < nandFTL.u16PagePerBlock ; i++)
	{
		if(ptBuf[i] != 0xffffffff)
		{
			DWORD addr;
			if ((ptBuf[i] & PageNrMask) == PageNrMask)
				addr = LogAddr + i;
			else
				addr = -1;
			if (NandPageCopy(TargetPage, ptBuf[i], addr, dataBuf) != PASS)
			{
				ret = FAIL;
				break;
			}
			TargetPage++;
			ptCacheLine->u32FreePageNum--;
		}
	}

	ext_mem_free(dataBuf);
	if (ret == FAIL)
	{
#if (NAND_ID_TYPE == HYNIX_HB_SERIES)
		NandBlockErase(newBlock << nandFTL.u08PagePerBlockExp);
#endif
		ERASE_CNT_INVALID(newBlock);
		badBlockMark(newBlock << nandFTL.u08PagePerBlockExp);
	}
	else
	{
		TargetPage = newBlock << nandFTL.u08PagePerBlockExp;
		for(i = 0; i < nandFTL.u16PagePerBlock ; i++)
		{
			if(ptBuf[i] != 0xffffffff)
				ptBuf[i] = TargetPage++; //update page translation table
		}
#ifdef NAND_FTL_DEBUG
		mpDebugPrint("FM(%x):%x,%x=>%x(%d)", ptCacheLine->u32LogicBlock, currDataBlock, ptCacheLine->u32ReplaceBlock, newBlock, ptCacheLine->u32FreePageNum);
#endif
		//2. free the orignal Data Block and Replaced Block
		if (NandBlockErase(currDataBlock << nandFTL.u08PagePerBlockExp) == FAIL)
		{
			ERASE_CNT_INVALID(currDataBlock);
			badBlockMark(currDataBlock << nandFTL.u08PagePerBlockExp);
		}
		else
		{
			ERASE_CNT_INC(currDataBlock);
			ftlFreeBlockSet(currDataBlock);
		}

		if(replacedBlock != 0xffffffff)
		{
			if (NandBlockErase(replacedBlock << nandFTL.u08PagePerBlockExp) == FAIL)
			{
				ERASE_CNT_INVALID(replacedBlock);
				badBlockMark(replacedBlock << nandFTL.u08PagePerBlockExp);
			}
			else
			{
				ERASE_CNT_INC(replacedBlock);
				ftlFreeBlockSet(replacedBlock);
			}
		}

		//3. assigned the new Block as a Data Block
		ptCacheLine->u32PhyBlock = newBlock;
		ptCacheLine->u32ReplaceBlock = 0xffffffff;
		ftlPhysicBlockSet(ptCacheLine->u32LogicBlock, ptCacheLine->u32PhyBlock);
		ftlReplacedBlockSet(ptCacheLine->u32LogicBlock, 0xFFFF);      //clean replace table
#ifdef NAND_FTL_DEBUG
		mpDebugPrint("FM END: Phy:%5d  Rep:%5d  Log:%5d   FPG: %5d", ptCacheLine->u32PhyBlock, ptCacheLine->u32ReplaceBlock,ptCacheLine->u32LogicBlock,ptCacheLine->u32FreePageNum);
#endif
	}

	return ret;
}

#if CACHEPAGE_MECHANISM
static void ftlGarbageCollection(BYTE zone, DWORD minFreeCount, DWORD logicBlock)
{
	static DWORD circular = 0;
	static DWORD total_tm = 200, total_cnt = 1;
	DWORD tmr = GetSysTime();
	DWORD i, idx;
	DWORD newBlock;
	DWORD totalCacheLine;
	DWORD zoneBase, zoneMask;
	ST_FTL_PT_CACHE_LINE *ptCacheLine;

	if(nandFTL.u16FreeBlockCount[zone] > minFreeCount)
	{
		mpDebugPrint("G.F");
		return;
	}

	zoneBase = zone << nandFTL.u08BlockPerZoneExp;
	zoneMask = ~(nandFTL.u16BlockPerZone - 1);

	//search FTL cache to find the Block that has replaced Block
	ptCacheLine = nandFTL.FTLCache.CacheBuf;
	totalCacheLine = nandFTL.FTLCache.u16SetNum << nandFTL.FTLCache.u08LinPSetExp;
	idx = logicBlock % nandFTL.FTLCache.u16SetNum ;
	if( idx ) {
		idx--;
	}
	idx *= nandFTL.FTLCache.u08LinePSet;

	for(i=0; i < totalCacheLine; i++)
	{
		if( (ptCacheLine[idx].u32ReplaceBlock != 0xffffffff) &&
			(ptCacheLine[idx].u32LogicBlock != logicBlock) &&
			((ptCacheLine[idx].u32PhyBlock & zoneMask) == zoneBase) )
		{
			//UartOutText("GC1 ");
			if( ftlPartialMerge(&ptCacheLine[idx]) != PASS)
			{
				SWORD ret;

				do
				{
					if((newBlock = ftlFreeBlockGet(zone)) >= nandFTL.u16BlockPerZone)
					{
						mpDebugPrint("No any free Block, garbage collection fail");
						return;
					}
					if ((ret = ftlFullMerge(&ptCacheLine[idx], newBlock)) != PASS)
					{
						ERASE_CNT_INVALID(newBlock);
						badBlockMark(newBlock<<nandFTL.u08PagePerBlockExp);
					}
				} while (ret != PASS);
			}
			DWORD tm = GetSysTime() - tmr;
			if((nandFTL.u16FreeBlockCount[zone] >= minFreeCount) && ((tm << 1) >= (total_tm/total_cnt)))
			{
				circular = idx;	// remember where we collected
//				mpDebugPrint("GC(1):%d ms", tm);
				if (total_tm + tm < total_tm)
				{
					total_tm /= total_cnt;
					total_cnt = 1;
				}
				else
				{
					total_tm += tm;
					total_cnt++;
				}
				return;
			}
		}
//		if (++idx == totalCacheLine)
//			idx = 0;
		if (idx) {
			idx--;
		}
		else {
			idx = totalCacheLine-1;
		}

	}
	MP_DEBUG("Outside cache");

	//parser all Block
	DWORD totalBlockPerZone;
	DWORD pageAddr1, pageAddr2;
	DWORD replacedBlock;

	WORD **BlockMap = nandFTL.pBlockTranslation;
	WORD **ReplaceMap = nandFTL.pReplaceBlock;
    for(i = 10 ; i < nandFTL.u16DataBlockPerZone ; i++)	// logical block i
    {
		if( (ReplaceMap[zone][i] < nandFTL.u16BlockPerZone)	// valid physical block number
		       && ( BlockMap[zone][i] != logicBlock ) )
		{	// yes, there is a replace block for logical blk i
			//UartOutText("GC2 ");
			if (BlockMap[zone][i] == 0xFFFF)
				mpDebugPrint("FATAL ERROR: blk %d replace to a free block!", ReplaceMap[zone][i]);
			ptCacheLine = ftlPtCacheGet(zoneBase+i, BlockMap[zone][i]);
			if(ftlPartialMerge(ptCacheLine) != PASS)
			{
				SWORD ret;
				do
				{
					if((newBlock = ftlFreeBlockGet(zone)) >= nandFTL.u16BlockPerZone)
					{
						mpDebugPrint("No any free Block, garbage collection fail");
						return;
					}
					if ((ret = ftlFullMerge(ptCacheLine, newBlock)) != PASS)
					{
						ERASE_CNT_INVALID(newBlock);
						badBlockMark(newBlock<<nandFTL.u08PagePerBlockExp);
					}
				} while (ret != PASS);
			}
			DWORD tm = GetSysTime()-tmr;
			if((nandFTL.u16FreeBlockCount[zone] >= minFreeCount) && ((tm << 1) >= (total_tm/total_cnt)))
			{
//				mpDebugPrint("GC(2):%d ms", GetSysTime()-tmr);
				if (total_tm + tm < total_tm)
				{
					total_tm /= total_cnt;
					total_cnt = 1;
				}
				else
				{
					total_tm += tm;
					total_cnt++;
				}
				return;
			}
		}
    }
	mpDebugPrint("not found any Block include replaced Block");
	DumpFTLTables();
	nandFTL.ReadOnly = TRUE;

	return ;
}

#else
static void ftlGarbageCollection(BYTE zone, DWORD minFreeCount, DWORD logicBlock)
{
	static DWORD circular = 0;
	static DWORD total_tm = 200, total_cnt = 1;
	DWORD tmr = GetSysTime();
	DWORD i, idx;
	DWORD newBlock;
	DWORD totalCacheLine;
	DWORD zoneBase, zoneMask;
	ST_FTL_PT_CACHE_LINE *ptCacheLine;

	if(nandFTL.u16FreeBlockCount[zone] > minFreeCount)
	{
		mpDebugPrint("G.F");
		return;
	}

	MP_DEBUG("G");
	zoneBase = zone << nandFTL.u08BlockPerZoneExp;
	zoneMask = ~(nandFTL.u16BlockPerZone - 1);

	//search FTL cache to find the Block that has replaced Block first
	ptCacheLine = nandFTL.FTLCache.CacheBuf;
	totalCacheLine = nandFTL.FTLCache.u16SetNum << nandFTL.FTLCache.u08LinPSetExp;
	idx = circular;
	for(i=0; i < totalCacheLine; i++)
	{
		if( (ptCacheLine[idx].u32ReplaceBlock != 0xffffffff) &&
			((ptCacheLine[idx].u32PhyBlock & zoneMask) == zoneBase) )
		{
			//UartOutText("GC1 ");
			if( ftlPartialMerge(&ptCacheLine[idx]) != PASS)
			{
				SWORD ret;

				do
				{
					if((newBlock = ftlFreeBlockGet(zone)) >= nandFTL.u16BlockPerZone)
					{
						mpDebugPrint("No any free Block, garbage collection fail");
						return;
					}
					if ((ret = ftlFullMerge(&ptCacheLine[idx], newBlock)) != PASS)
					{
						ERASE_CNT_INVALID(newBlock);
						badBlockMark(newBlock<<nandFTL.u08PagePerBlockExp);
					}
				} while (ret != PASS);
			}
			DWORD tm = GetSysTime() - tmr;
			if((nandFTL.u16FreeBlockCount[zone] >= minFreeCount) && ((tm << 1) >= (total_tm/total_cnt)))
			{
				circular = idx;	// remember where we collected
//				mpDebugPrint("GC(1):%d ms", tm);
				if (total_tm + tm < total_tm)
				{
					total_tm /= total_cnt;
					total_cnt = 1;
				}
				else
				{
					total_tm += tm;
					total_cnt++;
				}
				return;
			}
		}
		if (++idx == totalCacheLine)
			idx = 0;
	}
	MP_DEBUG("Outside cache");

	//parser all Block
	DWORD totalBlockPerZone;
	DWORD pageAddr1, pageAddr2;
	DWORD replacedBlock;
	WORD **BlockMap = nandFTL.pBlockTranslation;
	WORD **ReplaceMap = nandFTL.pReplaceBlock;

    for(i = 0 ; i < nandFTL.u16DataBlockPerZone ; i++)	// logical block i
    {
		if (ReplaceMap[zone][i] < nandFTL.u16BlockPerZone)	// valid physical block number
		{	// yes, there is a replace block for logical blk i
			//UartOutText("GC2 ");
			if (BlockMap[zone][i] == 0xFFFF)
				mpDebugPrint("FATAL ERROR: blk %d replace to a free block!", ReplaceMap[zone][i]);
			ptCacheLine = ftlPtCacheGet(zoneBase+i, BlockMap[zone][i]);
			if(ftlPartialMerge(ptCacheLine) != PASS)
			{
				SWORD ret;
				do
				{
					if((newBlock = ftlFreeBlockGet(zone)) >= nandFTL.u16BlockPerZone)
					{
						mpDebugPrint("No any free Block, garbage collection fail");
						return;
					}
					if ((ret = ftlFullMerge(ptCacheLine, newBlock)) != PASS)
					{
						ERASE_CNT_INVALID(newBlock);
						badBlockMark(newBlock<<nandFTL.u08PagePerBlockExp);
					}
				} while (ret != PASS);
			}
			DWORD tm = GetSysTime()-tmr;
			if((nandFTL.u16FreeBlockCount[zone] >= minFreeCount) && ((tm << 1) >= (total_tm/total_cnt)))
			{
//				mpDebugPrint("GC(2):%d ms", GetSysTime()-tmr);
				if (total_tm + tm < total_tm)
				{
					total_tm /= total_cnt;
					total_cnt = 1;
				}
				else
				{
					total_tm += tm;
					total_cnt++;
				}
				return;
			}
		}
    }
	mpDebugPrint("not found any Block include replaced Block");
	DumpFTLTables();
	nandFTL.ReadOnly = TRUE;

	return ;
}
#endif

static DWORD ftlContFreePageGet(DWORD *lba, DWORD maxPage, BYTE *replace)
{
	DWORD logicBlock, physicBlock, newBlock;
	ST_FTL_PT_CACHE_LINE *ptCacheLine;
	DWORD offset;
	DWORD count;
	WORD pagePerBlock;
	BYTE zone;

	BYTE SecPBlockExp = nandFTL.u08SectorPerBlockExp;
	BYTE SecPPageExp = nandFTL.u08SectorPerPageExp;

#ifdef D3
	mpDebugPrint("ftlContFreePageGet");
#endif
#ifdef NAND_FTL_DEBUG
	MP_DEBUG("W: log lba:%d, cPage:%d, Block:%5d",*lba, maxPage, *lba >> nandFTL.u08SectorPerBlockExp);
#endif
	logicBlock = *lba >> SecPBlockExp;
	if (logicBlock < nandFTL.u16DataBlockPerZone)
		zone = 0;
	else
		zone = logicBlock / nandFTL.u16DataBlockPerZone;

	if(zone >= nandFTL.u08ZoneNum){  // Avoid address which is out of nand flash range
		MP_DEBUG("Zone value is out of size");
		return 0;
	}

	pagePerBlock = nandFTL.u16PagePerBlock;
	*replace = 0;    // initial replace data

	if(nandFTL.u16FreeBlockCount[zone] < 3) {
		ftlGarbageCollection(zone, 3, logicBlock);
	}

	physicBlock = ftlPhysicBlockGet(logicBlock);
#ifdef D3
	mpDebugPrint("	physicBlock=%d", physicBlock);
#endif
	offset = (*lba >> SecPPageExp) & (pagePerBlock - 1);    //the page offset in Block
	if(maxPage > (pagePerBlock - offset))
		maxPage = pagePerBlock - offset;

	ptCacheLine = ftlPtCacheGet(logicBlock, physicBlock);

#if CACHEPAGE_MECHANISM
	if( nandFTL.u08CacheBufferUsing ) {
		*lba = (nandFTL.u16CacheBlock << nandFTL.u08PagePerBlockExp) + nandFTL.u16FreePageIndex;	// return page address
		count = 0x01;

		return count;
	}
#endif

	if ((ptCacheLine->u32FreePageNum < maxPage)             // No free enough page
		&& (ptCacheLine->u32ReplaceBlock != 0xffffffff))    // and already has a replace block
	{	// do full merge procedure
		SWORD ret;
		do
		{
			if((newBlock = ftlFreeBlockGet(zone)) >= nandFTL.u16BlockPerZone)
			{
				mpDebugPrint("No any free Block, get free page failed");
				nandFTL.ReadOnly = TRUE;
				return 0;
			}
			if ((ret = ftlFullMerge(ptCacheLine, newBlock)) != PASS)
			{
				ERASE_CNT_INVALID(newBlock);
				badBlockMark(newBlock<<nandFTL.u08PagePerBlockExp);
			}
		} while (ret != PASS);
	}

	if(!ptCacheLine->u32FreePageNum)    //No free page, assigned a replaced Block
	{
		if(ptCacheLine->u32ReplaceBlock != 0xffffffff)	// full merge failed
		{
			mpDebugPrint("-E- FATAL ERROR: A replace block existed after full merge!");
		}
		else
		{
			if((newBlock = ftlFreeBlockGet(zone)) >= nandFTL.u16BlockPerZone)
			{
				mpDebugPrint("No any free Block, get free page failed");
				nandFTL.ReadOnly = TRUE;
				return 0;
			}
			ptCacheLine->u32ReplaceBlock = newBlock;
			ptCacheLine->u32FreePageNum = pagePerBlock;
			ftlReplacedBlockSet(logicBlock, newBlock);
			MP_DEBUG("LB %x use p %x r %x", logicBlock, ptCacheLine->u32PhyBlock, newBlock);
		}
	}

	count = ptCacheLine->u32FreePageNum;
	if(ptCacheLine->u32ReplaceBlock != 0xffffffff)   // If replaced Block has exsit, the free page is in replaced Block
	{
		physicBlock = ptCacheLine->u32ReplaceBlock;
		*replace = 1;
	}
	else
	{
		physicBlock = ptCacheLine->u32PhyBlock;
	}

	offset = nandFTL.u16PagePerBlock - count;
	*lba = (physicBlock << nandFTL.u08PagePerBlockExp) + offset;	// return page address
	if(count > maxPage)
		count = maxPage;
#ifdef D3
	mpDebugPrint("	u32FreePageNum=%d, offset=%d", ptCacheLine->u32FreePageNum, offset);
	mpDebugPrint("	W: phy lba:%d, cPage:%d, Block:%5d",*lba, count, *lba >> nandFTL.u08PagePerBlockExp);
#endif

	if(!count)
	{
		mpDebugPrint("FTL fatal error 2");
	}

	return count;
}

static SWORD ftlSwitchMerge(ST_FTL_PT_CACHE_LINE *ptCacheLine)
{
	SWORD ret = FAIL;
	DWORD i;
	DWORD currDataBlock;
	BYTE pagePerBlockExp;
	DWORD *ptBuf;
	BOOL merge = TRUE;
	DWORD PageNrMask = nandFTL.u16PagePerBlock - 1;

	currDataBlock = ptCacheLine->u32PhyBlock;
	pagePerBlockExp = nandFTL.u08PagePerBlockExp;

	ptBuf = ptCacheLine->pu32PageTable;

	for(i=0; i<nandFTL.u16PagePerBlock; i++)
	{
		if((ptBuf[i]>>pagePerBlockExp) == currDataBlock)    //any valid page in data block?
		{
			merge = FALSE;
			break;
		}
		else {
			if (ptBuf[i] != 0xffffffff)	// in replace block (and is valid)
			{
				if ((ptBuf[i] & PageNrMask) != i)	// last page used in replace blk
				{
					merge = FALSE;
					break;
				}
			}
			else {
				merge = FALSE;
				break;
			}
		}
	}

	if(merge == TRUE)   //no valid page in current Data Block, free it
	{
		MP_DEBUG(" Using Switch Mode ");
#ifdef NAND_FTL_DEBUG
		mpDebugPrint("ftlSwitchMerge: %5d -> %5d => %5d", currDataBlock, ptCacheLine->u32ReplaceBlock, ptCacheLine->u32ReplaceBlock);
#endif
		//MP_DEBUG("S");
		//1. free the orignal Data Block
		if (NandBlockErase(currDataBlock << pagePerBlockExp) == FAIL)
		{
			ERASE_CNT_INVALID(currDataBlock);
			badBlockMark(currDataBlock << pagePerBlockExp);
		}
		else
		{
			ERASE_CNT_INC(currDataBlock);
			ftlFreeBlockSet(currDataBlock);
		}

#if CACHEPAGE_MECHANISM
		if( ( nandFTL.u32CacheBaseAddress/nandFTL.u16SectorPerBlock ) == ptCacheLine->u32LogicBlock ) {
			nandFTL.u32CacheBaseAddress = 0xFFFFFFFF;
		}
#endif

		//2. let the orignal replaced Block to become to Data Block
		ptCacheLine->u32PhyBlock = ptCacheLine->u32ReplaceBlock;
		ftlPhysicBlockSet(ptCacheLine->u32LogicBlock, ptCacheLine->u32PhyBlock);
		ptCacheLine->u32ReplaceBlock = 0xffffffff;
		ftlReplacedBlockSet( ptCacheLine->u32LogicBlock, 0xFFFF);
		ret = PASS;
#ifdef NAND_FTL_DEBUG
		mpDebugPrint("SMEND: Phy:%5d   Rep%5d    Log: %5d    FPG:%5d", ptCacheLine->u32PhyBlock, ptCacheLine->u32ReplaceBlock,ptCacheLine->u32LogicBlock,ptCacheLine->u32FreePageNum);
#endif
	}

	return ret;
}

static BYTE ftlPageTableUpdate(DWORD logicPage, DWORD newPhyPage, DWORD pageCount)
{
	ST_FTL_PT_CACHE_LINE *ptCacheLine;
	DWORD *ptBuf;
	DWORD logicBlock, physicBlock, newBlock;
#ifdef D3
	mpDebugPrint("	ftlPageTableUpdate, logicPage=%d, newPhyPage=%d, pageCount=%d", logicPage, newPhyPage, pageCount);
#endif
	logicBlock = logicPage >> nandFTL.u08PagePerBlockExp;

	physicBlock = ftlPhysicBlockGet(logicBlock);

	ptCacheLine = ftlPtCacheGet(logicBlock, physicBlock);
	ptBuf = ptCacheLine->pu32PageTable;

#if CACHEPAGE_MECHANISM
    if( nandFTL.u08CacheBufferUsing == 0x00 ){
		if(ptCacheLine->u32FreePageNum < pageCount)
		{
			mpDebugPrint("FTL fatal error 1(free page:%d/%d)", pageCount, ptCacheLine->u32FreePageNum);
			return FAIL;
		}

		ptCacheLine->u32FreePageNum -= pageCount;
    }
	else {
		nandFTL.u16FreePageIndex += 1;
	}
#else
	if(ptCacheLine->u32FreePageNum < pageCount)
	{
		mpDebugPrint("FTL fatal error 1(free page:%d/%d)", pageCount, ptCacheLine->u32FreePageNum);
		return FAIL;
	}

	ptCacheLine->u32FreePageNum -= pageCount;
#endif
	ptBuf = &ptBuf[logicPage & (nandFTL.u16PagePerBlock - 1)];

#ifdef D3
	DWORD offset = (logicPage>>1)&(nandFTL.u16PagePerBlock - 1);
	mpDebugPrint("	freePageNum=%d, prev ptBuf[%d]=%d", ptCacheLine->u32FreePageNum, offset, *ptBuf);
#endif
	while(pageCount--)
		*ptBuf++ = newPhyPage++;
#ifdef D3
	mpDebugPrint("	cur ptBuf[%d]=%d", offset, ptCacheLine->pu32PageTable[offset]);
#endif

	if( ptCacheLine->u32FreePageNum == 0x00 ) {
		ftlSwitchMerge(ptCacheLine);
		if( ( ptCacheLine->u32PhyBlock != 0xFFFFFFFF )
			  && ( ptCacheLine->u32ReplaceBlock != 0xFFFFFFFF ) ) {
			SWORD ret;
			do
			{
				if((newBlock = ftlFreeBlockGet( ptCacheLine->u32LogicBlock / nandFTL.u16DataBlockPerZone))
					>= nandFTL.u16BlockPerZone)
				{
					mpDebugPrint("No any free Block, garbage collection fail");
					return;
				}
				if ((ret = ftlFullMerge(ptCacheLine, newBlock)) != PASS)
				{
					ERASE_CNT_INVALID(newBlock);
					badBlockMark(newBlock<<nandFTL.u08PagePerBlockExp);
				}
			} while (ret != PASS);
		}
	}
	return PASS;
}

DWORD ftlGenLogicalAddr(DWORD lba, DWORD phyaddr, DWORD replace)	// Generate a logical address
{
	DWORD logAddr = lba >> nandFTL.u08SectorPerPageExp;

#if CACHEPAGE_MECHANISM
	replace = 0;
	if  ( nandFTL.u08CacheBufferUsing == 0x01 )
	{
		logAddr =  logAddr | 0xF0000000;
	}
#else
	if (replace)
	{
		DWORD PageNrMask = nandFTL.u16PagePerBlock - 1;
		if ((phyaddr & PageNrMask) == PageNrMask)    // If phyaddr is the last page of replace block, generate special logAddr tag
		{
			logAddr = logAddr & PageNrMask;
			logAddr = (0xffff << nandFTL.u08PagePerBlockExp) | logAddr;
			//mpDebugPrint("\tLogical Address : %x, %d => %x->%x->%x", phyaddr, replace, lba, lba >> nandFTL.u08SectorPerPageExp, logAddr);
		}
	}
#endif

	return logAddr;
}

static void nandBadBlockProc(DWORD logicLba, DWORD badPhysicLba)
{
	DWORD srcPage, trgPage;
	DWORD badBlock, backupBlock;
	BYTE zone;

	ST_FTL_PT_CACHE_LINE *ptCacheLine;
	DWORD i, totalCacheLine;

	mpDebugPrint("nandBadBlockProc logic LBA=%d, physic LBA=%d",logicLba, badPhysicLba);
	badBlock = badPhysicLba >> nandFTL.u08SectorPerBlockExp;
	srcPage = badBlock << nandFTL.u08PagePerBlockExp;

	//step1.get free block
	zone = badBlock >> nandFTL.u08BlockPerZoneExp;
	backupBlock = ftlFreeBlockGet(zone);

	//step2.backup bad block data
	//utilize the full merge function to backup the data of bad block
	if(backupBlock < MAX_BLOCK_NUM)
	{
		totalCacheLine = nandFTL.FTLCache.u16SetNum << nandFTL.FTLCache.u08LinPSetExp;
		ptCacheLine = nandFTL.FTLCache.CacheBuf;
		//the working block must be in the cache
		for(i=0; i < totalCacheLine; i++)
		{
			if(ptCacheLine[i].u32PhyBlock == badBlock || ptCacheLine[i].u32ReplaceBlock == badBlock)
			{
				ftlFullMerge(&ptCacheLine[i], backupBlock);
				break;
			}
		}
	}

	//step3.mark a bad block
	//the bad block should be erased and marked the bad block information
	//in the full merge funciton and partial merge function

	//But, the erase operation for the bad block may be success in the full
	//merge and partial merge function, such that the bad block will be put
	//to the free block pool and not be marked as a bad block.
	//So, force to mark the bad block again and retrieve the bad block from the free block pool
	ERASE_CNT_INVALID(badBlock);
	badBlockMark(srcPage);
	for (i = 0 ; i < nandFTL.u16BlockPerZone ; i++)
	{
		DWORD tmpBlock;

		tmpBlock = ftlFreeBlockGet(zone);
		if(tmpBlock == badBlock)
		{
			MP_DEBUG("found bad block in free block pool");
			break;
		}
		ftlFreeBlockSet(tmpBlock);
	}

	return ;
}

static DWORD NandFtlFree()
{
	if (nandFTL.pBlockTranslation)
		ker_mem_free(nandFTL.pBlockTranslation);
	if (nandFTL.pReplaceBlock)
		ker_mem_free(nandFTL.pReplaceBlock);
	if (nandFTL.pFreeBlockTable[0].Buffer)
		ker_mem_free(nandFTL.pFreeBlockTable[0].Buffer);
	if (nandFTL.FTLCache.CacheBuf[0].pu32PageTable)
		ker_mem_free(nandFTL.FTLCache.CacheBuf[0].pu32PageTable);
	if (nandFTL.FTLCache.CacheBuf)
		ker_mem_free(nandFTL.FTLCache.CacheBuf);
}

static SWORD NandFtlMalloc()
{
	SWORD ret = PASS;
	DWORD cacheLineNum;
	DWORD ptSizePerBlock = nandFTL.u16PagePerBlock * sizeof(DWORD);
	SDWORD MaxAllowBuf;
	DWORD blk_tab_sz = nandFTL.u08ZoneNum * sizeof(WORD *)
						+ (nandFTL.u08ZoneNum * nandFTL.u16DataBlockPerZone) * sizeof(WORD);
	DWORD free_tab_sz = nandFTL.u16BlockPerZone	* nandFTL.u08ZoneNum * sizeof(DWORD);
	SDWORD ftlCacheSize = nandFTL.u08ZoneNum * nandFTL.u16BlockPerZone * nandFTL.u16PagePerBlock * sizeof(DWORD);

	// cache calculate according to memory pool and nandflash size
    cacheLineNum = ((ftlCacheSize / ptSizePerBlock) >> FTL_CACHE_SET_EXP) << FTL_CACHE_SET_EXP;
	do
	{
		MaxAllowBuf = NAND_BUF_SIZE - blk_tab_sz * 2 - free_tab_sz - sizeof(ST_FTL_PT_CACHE_LINE) * cacheLineNum;
    	if(MaxAllowBuf < ftlCacheSize)
    	{
			if (ftlCacheSize > ptSizePerBlock)
				ftlCacheSize -= ptSizePerBlock;
			else
				break;
		    cacheLineNum = ftlCacheSize / ptSizePerBlock;
    	}
	} while (ftlCacheSize > MaxAllowBuf);
    cacheLineNum = (cacheLineNum >> FTL_CACHE_SET_EXP) << FTL_CACHE_SET_EXP;
    MP_DEBUG("FTL PT cache: %d line(= %d bytes)", cacheLineNum, ftlCacheSize);

    //Data Block translation buf
	nandFTL.pBlockTranslation = (WORD **)ker_mem_malloc(blk_tab_sz, TaskGetId());
	nandFTL.pReplaceBlock =(WORD **)ker_mem_malloc(blk_tab_sz, TaskGetId());
    //Allocate free Block buf
    nandFTL.pFreeBlockTable[0].Buffer = (DWORD *)ker_mem_malloc(free_tab_sz, TaskGetId());
    //FTL cache line struct buf
	nandFTL.FTLCache.CacheBuf = (ST_FTL_PT_CACHE_LINE *)ker_mem_malloc(sizeof(ST_FTL_PT_CACHE_LINE) * cacheLineNum, TaskGetId());
    //FTL page translation cache buf
    nandFTL.FTLCache.CacheBuf[0].pu32PageTable = (DWORD *)((DWORD)ker_mem_malloc(ftlCacheSize, TaskGetId()) & (~0x20000000));
	if (nandFTL.pBlockTranslation && nandFTL.pReplaceBlock
		&& nandFTL.pFreeBlockTable[0].Buffer && nandFTL.FTLCache.CacheBuf[0].pu32PageTable)
	{
		DWORD i;
    	DWORD *ptr = nandFTL.FTLCache.CacheBuf[0].pu32PageTable;

		// use with cache enable
		nandFTL.pBlockTranslation = (WORD **)((DWORD)nandFTL.pBlockTranslation & (~0x20000000));
		nandFTL.pBlockTranslation[0] = (WORD *)((DWORD)nandFTL.pBlockTranslation + nandFTL.u08ZoneNum * sizeof(WORD *));
		for (i = 0 ; i < nandFTL.u08ZoneNum ; i++)
			nandFTL.pBlockTranslation[i] = (WORD *)((DWORD)nandFTL.pBlockTranslation[0] + i * nandFTL.u16DataBlockPerZone * sizeof(WORD));

		nandFTL.pReplaceBlock = (WORD **)((DWORD)nandFTL.pReplaceBlock & (~0x20000000));
		nandFTL.pReplaceBlock[0] = (WORD *)((DWORD)nandFTL.pReplaceBlock + nandFTL.u08ZoneNum * sizeof(WORD *));
		for (i = 0 ; i < nandFTL.u08ZoneNum ; i++)
			nandFTL.pReplaceBlock[i] = (WORD *)((DWORD)nandFTL.pReplaceBlock[0] + i * nandFTL.u16DataBlockPerZone * sizeof(WORD));

	    nandFTL.FTLCache.CacheBuf = (ST_FTL_PT_CACHE_LINE *)((DWORD)nandFTL.FTLCache.CacheBuf & (~0x20000000));
		for (i = 0 ; i < nandFTL.u08ZoneNum ; i++)
		{
			nandFTL.pFreeBlockTable[i].Buffer = nandFTL.pFreeBlockTable[0].Buffer + i * sizeof(DWORD) * nandFTL.u16BlockPerZone;
			nandFTL.pFreeBlockTable[i].head = nandFTL.pFreeBlockTable[i].tail = 0;
		}

		// initail buffers
		for (i = 0 ; i < nandFTL.u08ZoneNum ; i++)
			memset(nandFTL.pBlockTranslation[i], 0xff, nandFTL.u16DataBlockPerZone*sizeof(WORD));

		for (i = 0 ; i < nandFTL.u08ZoneNum ; i++)
			memset(nandFTL.pReplaceBlock[i], 0xff, nandFTL.u16DataBlockPerZone*sizeof(WORD));

	    nandFTL.FTLCache.u16SetNum = cacheLineNum >> FTL_CACHE_SET_EXP;
	    nandFTL.FTLCache.u08LinePSet = 1 << FTL_CACHE_SET_EXP;
	    nandFTL.FTLCache.u08LinPSetExp = FTL_CACHE_SET_EXP;

		for(i = 0; i<cacheLineNum; i++)
		{
			nandFTL.FTLCache.CacheBuf[i].pu32PageTable = ptr;
			nandFTL.FTLCache.CacheBuf[i].u32FreePageNum = 0;
			nandFTL.FTLCache.CacheBuf[i].u32LRU = 0;
			nandFTL.FTLCache.CacheBuf[i].u32LogicBlock = 0xffffffff;
			nandFTL.FTLCache.CacheBuf[i].u32ReplaceBlock = 0xffffffff;
			nandFTL.FTLCache.CacheBuf[i].u32PhyBlock = 0xffffffff;
			ptr = (DWORD *)((DWORD)ptr + ptSizePerBlock);
		}

		#if CACHEPAGE_MECHANISM
		nandFTL.u32CacheLogAddress  = 0xFFFFFFFF;
		nandFTL.u32CacheBaseAddress = 0xFFFFFFFF;
		nandFTL.u16CacheBlock       = 0xFFFF;
		nandFTL.u16FreePageIndex    = nandFTL.u16PagePerBlock;
		nandFTL.u08CacheBufferUsing = 0x00;
		nandFTL.u08CacheDataValid   = 0x00;
		nandFTL.u16CacheDataMissTimes = 0x00;
		#endif
	}
	else
	{
		mpDebugPrint("Memory allocation failed: %x,%x,%x,%x", nandFTL.pBlockTranslation, nandFTL.pReplaceBlock,
						nandFTL.pFreeBlockTable[0].Buffer, nandFTL.FTLCache.CacheBuf[0].pu32PageTable);
		NandFtlFree();
		ret = FAIL;
	}

	return ret;
}

void NandFTLInfoInit()
{
	DWORD totalBlockNum, PagePerBlk, PageSize;

	GetNandGeometry(&totalBlockNum, &PagePerBlk, &PageSize);
	nandFTL.u16PagePerBlock      = PagePerBlk;
	nandFTL.u16PageSize          = PageSize;
	nandFTL.u08SectorPerPage     = nandFTL.u16PageSize / MCARD_SECTOR_SIZE;
	nandFTL.u08SectorPerPageExp  = CalValue2Exp(nandFTL.u08SectorPerPage);
	nandFTL.u08PagePerBlockExp   = CalValue2Exp(nandFTL.u16PagePerBlock);
	nandFTL.u08SectorPerBlockExp = nandFTL.u08PagePerBlockExp + nandFTL.u08SectorPerPageExp;
#if CACHEPAGE_MECHANISM
	nandFTL.u16SectorPerBlock = 1 << nandFTL.u08SectorPerBlockExp;
#endif
	nandFTL.ReadOnly             = FALSE;

    //set FTL info.
	nandFTL.u08ZoneNum = (totalBlockNum + (NAND_BLOCK_PER_ZONE - 1)) >> NAND_BLOCK_PER_ZONE_EXP;
	DWORD free_blk_exp = FREE_BLOCK_EXP;
#if (BOOTUP_TYPE == BOOTUP_TYPE_NAND)
	// set reversed some bad block range if free block count < total bad block
	DWORD expect_badblock_size = (totalBlockNum >> EXP_EXPECT_BADBLOCK) * nandFTL.u16PagePerBlock * nandFTL.u16PageSize;
	uint64_t totalsize = (uint64_t)nandFTL.u16PageSize * nandFTL.u16PagePerBlock * totalBlockNum;
	while ((totalsize >> free_blk_exp) <= (ISP_GetNandReservedSize() + expect_badblock_size))
		free_blk_exp--;
	MP_DEBUG("Nandflash reserved %d blocks", totalBlockNum >> free_blk_exp);
#endif
	if(totalBlockNum <= NAND_BLOCK_PER_ZONE)
	{
		nandFTL.u16BlockPerZone = totalBlockNum;
		nandFTL.u16DataBlockPerZone = totalBlockNum - (totalBlockNum >> free_blk_exp);
		nandFTL.u08BlockPerZoneExp = CalValue2Exp(nandFTL.u16BlockPerZone);
	}
	else
	{
		nandFTL.u16BlockPerZone = NAND_BLOCK_PER_ZONE;
		nandFTL.u16DataBlockPerZone = NAND_BLOCK_PER_ZONE - (NAND_BLOCK_PER_ZONE >> free_blk_exp);
		nandFTL.u08BlockPerZoneExp = NAND_BLOCK_PER_ZONE_EXP;
	}

	nandFTL.u32PowerUpCnt = 0;
	nandFTL.bEraseCntUpdate = 0;
	// assume the endurance of erase count is less if density is more.
	// then, the erase count threshold for wear-leveing will be:
	// +----------+----------+
	// |page size | threshold|
	// +----------+----------+
	// |      512 |      2048|
	// |     1024 |      1024|
	// |     2048 |       512|
	// |     4096 |       256|
	// |     8192 |       128|
	// +----------+----------+
	EraseCntThreshold = 2048 / (PageSize / 512);
    //Data Block erase count buffer
    if (pEraseCntTab == NULL)
    {
		DWORD CntSz = nandFTL.u08ZoneNum*nandFTL.u16BlockPerZone*sizeof(DWORD);
		nandFTL.pEraseCnt = pEraseCntTab = (DWORD *)ker_mem_malloc(CntSz, TaskGetId());
		memset(nandFTL.pEraseCnt, 0x00, CntSz);
    }
	else
	{
		nandFTL.pEraseCnt = pEraseCntTab;
	}
}

#if CACHEPAGE_MECHANISM
static void CheckCacheBlockValid( void )
{
	DWORD rowAddr;
	DWORD logblk, logblk1, logAddress, i;
	DWORD logicBlock, checkBlock, checkPage;

	MP_DEBUG(" Cache block : %x  ", nandFTL.u16CacheBlock);
	nandFTL.u16FreePageIndex = 0x00;
	rowAddr = nandFTL.u16CacheBlock << nandFTL.u08PagePerBlockExp;
    for( i = 0; i<nandFTL.u16PagePerBlock; i++ ) {
		logblk = GetNandLogAddr(rowAddr);
		if( logblk == LB_NO_ADDR ) {
			nandFTL.u16FreePageIndex = i;
			break;
		}
		else {
			nandFTL.u32CacheLogAddress = ( logblk & 0x0FFFFFFF ) << nandFTL.u08SectorPerPageExp;
		}
		rowAddr++;
    }

	MP_DEBUG(" FreePageIndex  : %x %x ", nandFTL.u32CacheLogAddress, nandFTL.u16FreePageIndex);

	// New method from CW
	BYTE CachePageValid = 0;

	// FreePageIndex located at odd position means cachePage is valid.
	CachePageValid = (nandFTL.u16FreePageIndex % 2);

	if(CachePageValid && nandFTL.u16FreePageIndex != 0){
		//mpDebugPrint(" Cache block data valid~~~~~~~~");
        nandFTL.u08CacheBufferUsing   = 0x01;
    	nandFTL.u16CacheDataMissTimes = 0x00;
	    nandFTL.u08CacheDataValid     = 0x01;
		nandFTL.u32CacheBaseAddress   = nandFTL.u32CacheLogAddress & ( ~( nandFTL.u16SectorPerBlock - 1 ));

        rowAddr = nandFTL.u16CacheBlock << nandFTL.u08PagePerBlockExp;
		nandFTL.u16FreePageIndex--;
        rowAddr += nandFTL.u16FreePageIndex;
		ftlPageTableUpdate(nandFTL.u32CacheLogAddress>> nandFTL.u08SectorPerPageExp, rowAddr, 1);
        nandFTL.u08CacheBufferUsing = 0x00;
	}
	else {
		//mpDebugPrint(" Cache block data invalid~~~~~~~~");
		rowAddr = nandFTL.u16CacheBlock << nandFTL.u08PagePerBlockExp;
		if (NandBlockErase(rowAddr) == FAIL){
			ERASE_CNT_INVALID(nandFTL.u16CacheBlock);
			badBlockMark(rowAddr);
		}
		else{
			ERASE_CNT_INC(nandFTL.u16CacheBlock);
			ftlFreeBlockSet(nandFTL.u16CacheBlock);
		}

		nandFTL.u32CacheLogAddress  = 0xFFFFFFFF;
		nandFTL.u32CacheBaseAddress = 0xFFFFFFFF;
		nandFTL.u16CacheBlock       = 0xFFFF;
		nandFTL.u16FreePageIndex    = nandFTL.u16PagePerBlock;
		nandFTL.u08CacheBufferUsing = 0x00;
		nandFTL.u08CacheDataValid   = 0x00;
		nandFTL.u16CacheDataMissTimes = 0x00;
	}

	return;
}
#endif

SWORD NandFtlInit()
{
	SWORD ret = PASS;
	DWORD i,j,k;
	DWORD rowAddr;
	DWORD dataBlockCount     = 0;
	DWORD replacedBlockCount = 0;
	DWORD freeBlockCount     = 0;
	WORD *tmpFreeBlockBuf    = 0;

	//FTL buf record
	ST_FTL_PT_CACHE_LINE *cacheBuf = 0;

	MP_DEBUG("======================================");

	NandFtlFree();
	memset(&nandFTL, 0x00, sizeof(ST_NAND_FTL_STRUCT));

	if(identified == FALSE)
	{
		if ((ret = NandIdentify()) == PASS)
			identified = TRUE;
	}

	NandFTLInfoInit();

	//Allocate FTL cache
	if ((ret == FAIL) || (NandFtlMalloc() == FAIL))
	{
		ret = FAIL;
	}
	else if ((tmpFreeBlockBuf = (WORD *)ext_mem_malloc(nandFTL.u16BlockPerZone << 1)) == NULL)
	{
		ret = FAIL;
	}
	else
	{
	    // skip boot area
	    #if (BOOTUP_TYPE == BOOTUP_TYPE_NAND)
	    DWORD blockSize = nandFTL.u16PagePerBlock * nandFTL.u16PageSize;
	    skipBlockNum = CAL_SKIP_BLKS(blockSize, ISP_GetNandReservedSize());
		MP_DEBUG("Skip %d blocks", skipBlockNum);
		#else
	    skipBlockNum = 0;
		#endif
		badblock_nr = 0;
		WORD **BlockMap = nandFTL.pBlockTranslation;
		WORD **ReplaceMap = nandFTL.pReplaceBlock;

		//Nand_SurfaceScan();     // debug surface scan

	    //parser all physic Block
	    for(i=0; i < nandFTL.u08ZoneNum; i++)
	    {
			j = (i==0) ? skipBlockNum : 0;	//skip boot area
	        for(; j < nandFTL.u16BlockPerZone; j++)
	        {
				DWORD logblk1, logblk2, logblk3;
				#if BLK_TAB_DUMP
				BYTE info[50];
				sprintf(info, "Phy[%d,%d]:", i, j);
				#endif
				rowAddr = ((i<<nandFTL.u08BlockPerZoneExp) + j) << nandFTL.u08PagePerBlockExp;

				logblk1 = GetNandLogAddr(rowAddr);
				logblk2 = GetNandLogAddr(rowAddr+nandFTL.u16PagePerBlock-1);
				logblk3 = logblk1;
//if(NandBlockErase(rowAddr) == PASS);

				#if BLK_TAB_DUMP
				sprintf(info, "%s Log(%d,%d) is", info, logblk1, logblk2);
				#endif
	            //check bad block
	            if ((logblk1 > LB_ERROR && logblk1 < LB_NO_ADDR)
					|| (logblk2 > LB_ERROR && logblk2 < LB_NO_ADDR))
	            {
					MP_DEBUG("\t -NAND Bad block: %d, PageAddr: 0x%X", j, rowAddr);
					ERASE_CNT_INVALID(rowAddr>>nandFTL.u08PagePerBlockExp);
	                continue;
	            }

	            //get logic block field
	            if ((logblk1 == LB_NO_ADDR) && (logblk2 == LB_NO_ADDR)
					&& (CheckSpareContent(rowAddr) == FALSE)
					&& (CheckSpareContent(rowAddr+nandFTL.u16PagePerBlock-1) == FALSE))
	            {
					#if BLK_TAB_DUMP
					//mpDebugPrint("%s free block:%d", info, j);
					#endif
					// free block
	                tmpFreeBlockBuf[freeBlockCount++] = j;
	            }
				else //a Data Block or Replaced Block
				{
					logblk1 >>= nandFTL.u08PagePerBlockExp;	// become logical block address
					logblk2 >>= nandFTL.u08PagePerBlockExp;
					if (logblk1 >= nandFTL.u16BlockPerZone)
					{
#if CACHEPAGE_MECHANISM
						if( ( logblk3 & 0x0FFFFFFF ) >> nandFTL.u08PagePerBlockExp < nandFTL.u16BlockPerZone ) {
							nandFTL.u16CacheBlock = j;
						}
						else {
//							mpDebugPrint("Unknown logical address:%x,%x, %x", logblk1, logblk2, logblk3 );
							MP_DEBUG("Unknown logical address:%x,%x, %x", logblk1, logblk2, logblk3 );
						}
#else
						mpDebugPrint("Unknown logical address:%x,%x", logblk1, logblk2);
#endif
					}
					else if(BlockMap[i][logblk1] == 0xffff)
					{
#if BLK_TAB_DUMP
						mpDebugPrint("%s data block:%d", info, j);
#endif
						dataBlockCount++;
						BlockMap[i][logblk1] = j;
					}
					else	// it means the Logic Block have two block in it, in other word  it have a data block and replace block
					{
						if (ReplaceMap[i][logblk1] != 0xffff)
							mpDebugPrint(" -R(%d)- ", ReplaceMap[i][logblk1]);
						if(logblk2 < nandFTL.u16DataBlockPerZone) //the current Block is a Data Block
						{
							DWORD orgDataBlock = BlockMap[i][logblk1];
							#if BLK_TAB_DUMP
							mpDebugPrint("%s data block:%d, replace block:%d", info, j, orgDataBlock);
							#endif
							BlockMap[i][logblk1] = j;
							ReplaceMap[i][logblk1] = orgDataBlock;
							replacedBlockCount++;
						}
						else	//the current Block may be a Replaced Block
						{
							#if BLK_TAB_DUMP
							mpDebugPrint("%s replace block:%d", info, j);
							#endif
							ReplaceMap[i][logblk1] = j;
							replacedBlockCount++;
						}
	                }
	            }
	        }

	        MP_DEBUG("Z%1d: F-Block:%d, D-Block:%6d/%6d, R-Block:%6d badblock %d", i, freeBlockCount, dataBlockCount,
	            nandFTL.u16DataBlockPerZone, replacedBlockCount, badblock_nr);

	        //allocate free sBlock to data sBlock
	        #if 0
	        if(dataBlockCount < nandFTL.u16DataBlockPerZone)
	        {
	            if((nandFTL.u16DataBlockPerZone - dataBlockCount) >= freeBlockCount)
	            {
	                mpDebugPrint("not enougth free Block, set read only mode");
					ret = -2;
	            }

	            for(k=0; k < nandFTL.u16DataBlockPerZone; k++)
	            {
	                if(BlockMap[i][k] == 0xffff)
	                {
	                    freeBlockCount--;
	                    BlockMap[i][k] = tmpFreeBlockBuf[freeBlockCount];
	                }

					if (freeBlockCount == 0)
						break;
	            }

	        }
			#endif

	        while(freeBlockCount--)
	        {
	            ftlFreeBlockSet((i << nandFTL.u08BlockPerZoneExp) + tmpFreeBlockBuf[freeBlockCount]);
#ifdef NAND_FTL_DEBUG
				mpDebugPrint("tmpFreeBlockBuf[%2d] = %4d", freeBlockCount, tmpFreeBlockBuf[freeBlockCount]);
#endif
	        }
			MP_DEBUG("zone: %d  Free block count = %d", i, nandFTL.u16FreeBlockCount[i]);
	    }
	}
	//DumpFTLTables();

    if(tmpFreeBlockBuf)
        ext_mem_free(tmpFreeBlockBuf);

#if CACHEPAGE_MECHANISM
	mpDebugPrint("check cache block status stage...");
	if( nandFTL.u16CacheBlock != 0xFFFF ) {
		CheckCacheBlockValid();
	}
	mpDebugPrint("....Cache page mechanism open!");
#else
	//mpDebugPrint("....Cache page mechanism turn off!");
#endif

    MP_DEBUG("======================================");

	if (ret == FAIL)
		NandFtlFree();

	return ret;
}

static void FTLReadCacheChk(DWORD PhyAddr, DWORD PageCnt, BYTE *buffer)
{
	if (CachePtr && (CachePageAddr >= PhyAddr) && (CachePageAddr < (PhyAddr + PageCnt)))
	{
		//mpDebugPrint("PCU!(%d,%d)", PhyAddr, PageCnt);
		mmcp_memcpy_polling(CachePtr, &buffer[(CachePageAddr-PhyAddr) * nandFTL.u16PageSize], nandFTL.u16PageSize);
		CacheValid = TRUE;
	}
}

static SWORD FTLISPRead(DWORD logAddr, DWORD lba, DWORD SectorCnt, DWORD buffer, DWORD *progress)
{
	if (CachePtr == NULL)
	{
		CacheValid = FALSE;
		CachePtr = (BYTE *)((DWORD)ker_mem_malloc(nandFTL.u16PageSize, TaskGetId()) | 0xA0000000);
		if (CachePtr == NULL)
		{
			mpDebugPrint("Fatal error, nand driver is not available to run!");
		}
	}
	GetNandAvailEccBit();	// is to clean the error bit record
	if (CachePtr)
	{
		DWORD PageAddr = lba >> nandFTL.u08SectorPerPageExp;
		DWORD offset = lba - (PageAddr << nandFTL.u08SectorPerPageExp);
		if (offset)
		{
			DWORD nr = nandFTL.u08SectorPerPage - offset;

			//mpDebugPrint("%s:1. PageAddr=%d(%x)", __FUNCTION__, PageAddr, buffer);
			if (nr > SectorCnt)
				nr = SectorCnt;
			if (PageAddr != CachePageAddr || CacheValid != TRUE)	// cache missed, read it from physical
			{
				//mpDebugPrint("\tcaching...");
				NandPageRead(PageAddr, 1, (DWORD)CachePtr, NULL);
#if NAND_PAGE_RANDOM
				NandDataRandom(CachePtr,
								CachePtr,
								nandFTL.u16PageSize,
								(PageAddr&0xFF)*NAND_RANDOM_TBL_IDX);
#endif
				#if NAND_WL_READ_FUNC
				if (GetNandAvailEccBit() <= NAND_WL_READ_THRESHOLD)
				{
					//if (logAddr == -1)	// always stand
					{
						mpDebugPrint("ToDo:WearLeveling for Read(1) is acted on page%d in ISP area", PageAddr);
					}
					//else
					//{
					//	FTLWrite((DWORD)CachePtr, 1, logAddr-offset, NULL);
					//	MP_DEBUG("\tWearLeveling for Read(1) is acted on page%d!", PageAddr);
					//}
				}
				#endif
				CacheValid = TRUE;
				CachePageAddr = PageAddr;
			}
			else
			{
				//UartOutText("(Cache Hit 1)");
			}
			mmcp_memcpy_polling((BYTE *)buffer, &CachePtr[offset<<MCARD_SECTOR_SIZE_EXP], nr << MCARD_SECTOR_SIZE_EXP);
			SectorCnt -= nr;
			buffer += nr << MCARD_SECTOR_SIZE_EXP;
			//if (logAddr != -1)	// always not stand
			//	logAddr += nr;
			//if (progress)			// always not stand
			//	*progress += nr;
			PageAddr++;
		}
		DWORD PageCnt = SectorCnt >> nandFTL.u08SectorPerPageExp;
		SectorCnt = SectorCnt - (PageCnt << nandFTL.u08SectorPerPageExp);
		if (PageCnt > 0)
		{
			//mpDebugPrint("%s:2. PageAddr=%d(%d)(%x)", __FUNCTION__, PageAddr, PageCnt, buffer);
			NandPageRead(PageAddr, PageCnt, buffer, progress);
#if NAND_PAGE_RANDOM
{
			int i;
			for (i=0 ; i<PageCnt ; i++)
			{
				NandDataRandom((BYTE *)(buffer+i*nandFTL.u16PageSize),
								(BYTE *)(buffer+i*nandFTL.u16PageSize),
								nandFTL.u16PageSize,
								((PageAddr+i)&0xFF)*NAND_RANDOM_TBL_IDX);
			}
}
#endif
			#if NAND_WL_READ_FUNC
			if (GetNandAvailEccBit() <= NAND_WL_READ_THRESHOLD)
			{
				//if (logAddr == -1)	// always stand
				{
					mpDebugPrint("ToDo:WearLeveling for Read(2) is acted on page%d in ISP area", PageAddr);
				}
				//else
				//{
				//	FTLWrite(buffer, PageCnt, logAddr, NULL);
				//	MP_DEBUG("\tWearLeveling for Read(2) is acted on page%d!", PageAddr);
				//}
			}
			#endif
			//if (logAddr != -1)	// always not stand
			//	logAddr += PageCnt << nandFTL.u08SectorPerPageExp;
		}
		if (SectorCnt)
		{
			PageAddr += PageCnt;
			buffer += PageCnt << (nandFTL.u08SectorPerPageExp + MCARD_SECTOR_SIZE_EXP);
			//mpDebugPrint("%s:3, PageAddr=%d(%x)", __FUNCTION__, PageAddr, buffer);
			if (PageAddr != CachePageAddr || CacheValid != TRUE)	// cache missed, read it from physical
			{
				//mpDebugPrint("\tcaching...");
				NandPageRead(PageAddr, 1, (DWORD)CachePtr, NULL);
#if NAND_PAGE_RANDOM
				NandDataRandom(CachePtr,
								CachePtr,
								nandFTL.u16PageSize,
								(PageAddr&0xFF)*NAND_RANDOM_TBL_IDX);
#endif
				#if NAND_WL_READ_FUNC
				if (GetNandAvailEccBit() <= NAND_WL_READ_THRESHOLD)
				{
					//if (logAddr == -1)	// always stand
					//{
						mpDebugPrint("ToDo:WearLeveling for Read(3) is acted on page%d in ISP area", PageAddr);
					//}
					//else
					//{
					//	FTLWrite((DWORD)CachePtr, 1, logAddr, NULL);
					//	MP_DEBUG("\tWearLeveling for Read(3) is acted on page%d!", PageAddr);
					//}
				}
				#endif
				CacheValid = TRUE;
				CachePageAddr = PageAddr;
			}
			else
			{
				//UartOutText("(Cache Hit 2)");
			}
			mmcp_memcpy_polling((BYTE *)buffer, CachePtr, SectorCnt << MCARD_SECTOR_SIZE_EXP);
		}
	}

	return PASS;
}

static SWORD FTLRead(DWORD logAddr, DWORD lba, DWORD SectorCnt, DWORD buffer, DWORD *progress)
{
	if (CachePtr == NULL)
	{
		CacheValid = FALSE;
		CachePtr = (BYTE *)((DWORD)ker_mem_malloc(nandFTL.u16PageSize, TaskGetId()) | 0xA0000000);
		if (CachePtr == NULL)
		{
			mpDebugPrint("Fatal error, nand driver is not available to run!");
		}
	}
	GetNandAvailEccBit();	// is to clean the error bit record
	if (CachePtr)
	{
		DWORD PageAddr = lba >> nandFTL.u08SectorPerPageExp;
		DWORD offset = lba - (PageAddr << nandFTL.u08SectorPerPageExp);
		if (offset)
		{
			DWORD nr = nandFTL.u08SectorPerPage - offset;

			//mpDebugPrint("%s:1. PageAddr=%d(%x)", __FUNCTION__, PageAddr, buffer);
			if (nr > SectorCnt)
				nr = SectorCnt;
			if (PageAddr != CachePageAddr || CacheValid != TRUE)	// cache missed, read it from physical
			{
				//mpDebugPrint("\tcaching...");
				NandPageRead(PageAddr, 1, (DWORD)CachePtr, NULL);
				#if NAND_WL_READ_FUNC
				if (GetNandAvailEccBit() <= NAND_WL_READ_THRESHOLD)
				{
					if (logAddr == -1)
					{
						mpDebugPrint("ToDo:WearLeveling for Read(1) is acted on page%d in ISP area", PageAddr);
					}
					else
					{
						FTLWrite((DWORD)CachePtr, 1, logAddr-offset, NULL);
						MP_DEBUG("\tWearLeveling for Read(1) is acted on page%d!", PageAddr);
					}
				}
				#endif
				CacheValid = TRUE;
				CachePageAddr = PageAddr;
			}
			else
			{
				//UartOutText("(Cache Hit 1)");
			}
			mmcp_memcpy_polling((BYTE *)buffer, &CachePtr[offset<<MCARD_SECTOR_SIZE_EXP], nr << MCARD_SECTOR_SIZE_EXP);
			SectorCnt -= nr;
			buffer += nr << MCARD_SECTOR_SIZE_EXP;
			if (logAddr != -1)
				logAddr += nr;
			if (progress)
				*progress += nr;
			PageAddr++;
		}
		DWORD PageCnt = SectorCnt >> nandFTL.u08SectorPerPageExp;
		SectorCnt = SectorCnt - (PageCnt << nandFTL.u08SectorPerPageExp);
		if (PageCnt > 0)
		{
			//mpDebugPrint("%s:2. PageAddr=%d(%d)(%x)", __FUNCTION__, PageAddr, PageCnt, buffer);
			NandPageRead(PageAddr, PageCnt, buffer, progress);
			#if NAND_WL_READ_FUNC
			if (GetNandAvailEccBit() <= NAND_WL_READ_THRESHOLD)
			{
				if (logAddr == -1)
				{
					mpDebugPrint("ToDo:WearLeveling for Read(2) is acted on page%d in ISP area", PageAddr);
				}
				else
				{
					FTLWrite(buffer, PageCnt, logAddr, NULL);
					MP_DEBUG("\tWearLeveling for Read(2) is acted on page%d!", PageAddr);
				}
			}
			#endif
			if (logAddr != -1)
				logAddr += PageCnt << nandFTL.u08SectorPerPageExp;
		}
		if (SectorCnt)
		{
			PageAddr += PageCnt;
			buffer += PageCnt << (nandFTL.u08SectorPerPageExp + MCARD_SECTOR_SIZE_EXP);
			//mpDebugPrint("%s:3, PageAddr=%d(%x)", __FUNCTION__, PageAddr, buffer);
			if (PageAddr != CachePageAddr || CacheValid != TRUE)	// cache missed, read it from physical
			{
				//mpDebugPrint("\tcaching...");
				NandPageRead(PageAddr, 1, (DWORD)CachePtr, NULL);
				#if NAND_WL_READ_FUNC
				if (GetNandAvailEccBit() <= NAND_WL_READ_THRESHOLD)
				{
					if (logAddr == -1)
					{
						mpDebugPrint("ToDo:WearLeveling for Read(3) is acted on page%d in ISP area", PageAddr);
					}
					else
					{
						FTLWrite((DWORD)CachePtr, 1, logAddr, NULL);
						MP_DEBUG("\tWearLeveling for Read(3) is acted on page%d!", PageAddr);
					}
				}
				#endif
				CacheValid = TRUE;
				CachePageAddr = PageAddr;
			}
			else
			{
				//UartOutText("(Cache Hit 2)");
			}
			mmcp_memcpy_polling((BYTE *)buffer, CachePtr, SectorCnt << MCARD_SECTOR_SIZE_EXP);
		}
	}

	return PASS;
}

static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress)
{
	DWORD phyLba;
	DWORD contSec;

#ifdef D3
	mpDebugPrint("NandFtlRead logLba=0x%x, totalSec=%d (0x%x)", dwLogAddr, dwSectorCount, dwBufferAddress);
#endif
	while(dwSectorCount)
	{
		phyLba = dwLogAddr;
		contSec = ftlContReadSectorGet(&phyLba, dwSectorCount);
		if(!contSec)    //all read page are free
		{
#ifdef D3
			mpDebugPrint("	all read page are free, set 0xff");
#endif
			contSec = (nandFTL.u08SectorPerPage > dwSectorCount) ? dwSectorCount : nandFTL.u08SectorPerPage;
			memset((BYTE *)dwBufferAddress, 0xff, contSec<<9);
		}
		else if(FAIL == FTLRead(dwLogAddr, phyLba, contSec, dwBufferAddress, progress))
		{
			// don't care bad block issue when read, leave it for FAT management.
			//nandBadBlockProc(dwLogAddr, phyLba, nandFTL.u16PagePerBlock);
			mpDebugPrint("nand read failed..lba:%d", dwLogAddr);
		}
		dwSectorCount -= contSec;
		dwLogAddr += contSec;
		dwBufferAddress += (contSec<<9);
	}

	return PASS;
}

#if CACHEPAGE_MECHANISM
static SWORD CachePageWrite(DWORD buffer, DWORD dwLogAddr)
{
	SWORD retCode;
	DWORD progress;
	BYTE  replace;
	DWORD phyLba = dwLogAddr;

	nandFTL.u08CacheBufferUsing = 1;
	ftlContFreePageGet(&phyLba, 1, &replace);

//mpDebugPrint("Cache write> phsical addr: %x, logical addr: %x",
//	phyLba, ftlGenLogicalAddr(dwLogAddr, phyLba, replace));

//mpDebugPrint("    1.freepageIndex...%d", nandFTL.u16FreePageIndex);

if(nandFTL.u16FreePageIndex % 2 == 0)
	mpDebugPrint("Terrible error!!!!!!!!!!!!!!!");


	retCode = NandPageWrite(phyLba, 1, buffer, ftlGenLogicalAddr(dwLogAddr, phyLba, replace), &progress);
	if (retCode)
	{
		mpDebugPrint("\tNand cache block Write failed!(lba:%d, cnt:%d)",
			dwLogAddr, 1<<nandFTL.u08SectorPerPageExp);
		nandBadBlockProc(dwLogAddr, phyLba);
	}
	else{
		nandFTL.u16FreePageIndex++;
//mpDebugPrint("    2.freepageIndex...%d", nandFTL.u16FreePageIndex);
		if(nandFTL.u16FreePageIndex >= nandFTL.u16PagePerBlock)
			mpDebugPrint("shit!!!! what's a hell!!!!!");
	}

	nandFTL.u08CacheBufferUsing = 0;
	return retCode;
}
#endif

static DWORD FTLWrite(DWORD buffer, DWORD PageCnt, DWORD dwLogAddr, DWORD *progress)
{
	static BYTE zone = 0;
	DWORD phyLba, contPage = 0;
	BYTE replace;
	SWORD retCode;
	DWORD PageNrMask = nandFTL.u16PagePerBlock - 1;

#if NAND_WL_WRITE_FUNC
	ftlWearLeveling(zone);
	if (nandFTL.u08ZoneNum > 1)
	{
		zone++;
		if (zone >= nandFTL.u08ZoneNum)
			zone = 0;
	}
#endif

	phyLba = dwLogAddr;
	contPage = ftlContFreePageGet(&phyLba, PageCnt, &replace);
	if(!contPage)
	{
		mpDebugPrint("should become read only");
		nandFTL.ReadOnly = TRUE;
	}
	retCode = NandPageWrite(phyLba, contPage, buffer, ftlGenLogicalAddr(dwLogAddr, phyLba, replace), progress);
	if (retCode)
	{
		mpDebugPrint("\tNand FlatWrite failed!(lba:%d, cnt:%d)", dwLogAddr, contPage<<nandFTL.u08SectorPerPageExp);
		nandBadBlockProc(dwLogAddr, phyLba);
	}
	if (contPage)
	{
		ftlPageTableUpdate(dwLogAddr >> nandFTL.u08SectorPerPageExp, phyLba, contPage);
		FTLReadCacheChk(phyLba, contPage, (BYTE *)buffer);  // Update cached page data
	}

	return contPage;
}

static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress)
{
	SWORD ret = PASS;
	DWORD phyLba, retCode, SectorCount;
	DWORD contPage, totalPage;
	BYTE *pageBuf = NULL;
	BYTE SectorPerPage;
	BYTE SectorPerPageExp;
	BYTE offset;

#ifdef D3
	mpDebugPrint("write nand lba=%d, %d (0x%x)", dwLogAddr, dwSectorCount, dwBufferAddress);
#endif
	SectorCount = dwSectorCount;
	SectorPerPage = nandFTL.u08SectorPerPage;
	SectorPerPageExp = nandFTL.u08SectorPerPageExp;

#if CACHEPAGE_MECHANISM
	BYTE DrawBackPage = FALSE;

	if( nandFTL.u08CacheDataValid ) {
		if( !pageBuf ) {
			pageBuf = (BYTE *)((DWORD)ext_mem_malloc(nandFTL.u16PageSize) | 0xA0000000);
		}

		if( nandFTL.u32CacheLogAddress == ( dwLogAddr - ( dwLogAddr & (nandFTL.u08SectorPerPage - 1) ) ) ) {
			/*
			 *    At this case, set cached page invalid.
			 *    Cached page will be combined with new wrote data at head section by FlatRead().
			 *    Because cached page address still dock in cache line.
			 */

//UartOutText("S->");
//mpDebugPrint(" %x, %x, %x", nandFTL.u32CacheLogAddress, ( dwLogAddr - ( dwLogAddr & (nandFTL.u08SectorPerPage-1) ) ), dwLogAddr);
			FlatRead((DWORD)pageBuf, SectorPerPage, nandFTL.u32CacheLogAddress, NULL);
			DrawBackPage = TRUE;
			// Write to cache block again -> Even page means cache invalid
			CachePageWrite((DWORD)pageBuf, nandFTL.u32CacheLogAddress);

			nandFTL.u32CacheBaseAddress = 0xFFFFFFFF;
			nandFTL.u32CacheLogAddress = 0xFFFFFFFF;
			nandFTL.u08CacheDataValid = 0x00;
			nandFTL.u16CacheDataMissTimes = 0x00;
		}
		else {
//mpDebugPrint("SectorCout: %d", SectorCount);
			if( SectorCount >= 32 ) {
//UartOutText("m->");
//mpDebugPrint(" %x, %x, %x", nandFTL.u32CacheLogAddress, ( dwLogAddr - ( dwLogAddr & (nandFTL.u08SectorPerPage-1) ) ), dwLogAddr);

				nandFTL.u16CacheDataMissTimes++;
			}
			if( ( nandFTL.u16CacheDataMissTimes > 0x0002 ) ||
				( ( nandFTL.u32CacheLogAddress >> nandFTL.u08SectorPerBlockExp )
				             == ( dwLogAddr >> nandFTL.u08SectorPerBlockExp ) ) ||
				( ( nandFTL.u32CacheLogAddress >> nandFTL.u08SectorPerBlockExp )
				             == ( (dwLogAddr+dwSectorCount) >> nandFTL.u08SectorPerBlockExp ) ) )
			{
//UartOutText("f");
				FlatRead((DWORD)pageBuf, SectorPerPage, nandFTL.u32CacheLogAddress, NULL);

				// Write to cache block again -> Even page means cache invalid
				CachePageWrite((DWORD)pageBuf, nandFTL.u32CacheLogAddress);
				nandFTL.u08CacheBufferUsing = 0x00;
				if(FTLWrite((DWORD)pageBuf, 1, nandFTL.u32CacheLogAddress, progress) == 0){
					mpDebugPrint("Fatal FTL error, must check!!!");
					goto FTL_WRITE_RET_FAIL;
				}
/*    		    nandFTL.u08CacheBufferUsing = 0x01;
				FTLWrite((DWORD)pageBuf, 1, nandFTL.u32CacheLogAddress, progress);						 */
				nandFTL.u32CacheLogAddress = 0xFFFFFFFF;
				nandFTL.u32CacheBaseAddress = 0xFFFFFFFF;
				nandFTL.u08CacheDataValid = 0x00;
				nandFTL.u16CacheDataMissTimes = 0x00;
			}
		}
	}
#endif

#ifdef D3
	mpDebugPrint("=============head==============");
#endif
	//head (not align to page)--------------------------------------------------------------------------
	offset = dwLogAddr & (SectorPerPage - 1);
	if(offset)
	{
		DWORD alignLba = dwLogAddr - offset;
		BYTE SectorNr = SectorPerPage - offset;

		if(SectorNr > dwSectorCount)
			SectorNr = dwSectorCount;
#ifdef D3
		mpDebugPrint("head offset=%d", offset);
#endif

		if( !pageBuf ) {
			pageBuf = (BYTE *)((DWORD)ext_mem_malloc(nandFTL.u16PageSize) | 0xA0000000);
		}

#if CACHEPAGE_MECHANISM
		if(DrawBackPage == FALSE)
			FlatRead((DWORD)pageBuf, SectorPerPage, alignLba, NULL);
#else
		FlatRead((DWORD)pageBuf, SectorPerPage, alignLba, NULL);
#endif
		mmcp_memcpy_polling(pageBuf + (offset << MCARD_SECTOR_SIZE_EXP), (BYTE *)dwBufferAddress, SectorNr << MCARD_SECTOR_SIZE_EXP);
		if (FTLWrite((DWORD)pageBuf, 1, alignLba, progress) == 0)
			goto FTL_WRITE_RET_FAIL;
		dwLogAddr += SectorNr;
		dwSectorCount -= SectorNr;
		dwBufferAddress += SectorNr << MCARD_SECTOR_SIZE_EXP;
	}
	//------------------------------------------------------------------------------------------

#ifdef D3
	mpDebugPrint("		body		");
#endif
	//page aligment body------------------------------------------------------------------------
	totalPage = dwSectorCount >> SectorPerPageExp;
	while(totalPage)
	{
		if ((contPage = FTLWrite(dwBufferAddress, totalPage, dwLogAddr, progress)) == 0)
			goto FTL_WRITE_RET_FAIL;
		totalPage -= contPage;
		dwLogAddr += contPage << SectorPerPageExp;
		dwBufferAddress += contPage << (SectorPerPageExp + MCARD_SECTOR_SIZE_EXP);
	}
	//------------------------------------------------------------------------------------------

#ifdef D3
	mpDebugPrint("___________tail___________");
#endif
	//tail fragment-----------------------------------------------------------------------------
	dwSectorCount = dwSectorCount & (SectorPerPage - 1);	// rest of sectors
	if(dwSectorCount)
	{
		if(!pageBuf)
			pageBuf = (BYTE *)((DWORD)ext_mem_malloc(nandFTL.u16PageSize) | 0xA0000000);

#ifdef D3
		mpDebugPrint("tail sector count=%d", dwSectorCount);
#endif
		mmcp_memcpy_polling(pageBuf, (BYTE *)dwBufferAddress, dwSectorCount << MCARD_SECTOR_SIZE_EXP);
		FlatRead((DWORD)(pageBuf + (dwSectorCount << MCARD_SECTOR_SIZE_EXP)), (SectorPerPage - dwSectorCount), dwLogAddr + dwSectorCount, NULL);

#if CACHEPAGE_MECHANISM
		if(SectorCount >= 32)
		{
	        if( nandFTL.u08CacheDataValid == 0x00 )
			{
				//mpDebugPrint("Base: %x, target: %x, log/Sec: %x/%x",
				//	nandFTL.u32CacheBaseAddress, ( dwLogAddr & ( ~( nandFTL.u16SectorPerBlock - 1 ) ) ),
				//	dwLogAddr, ( ~( nandFTL.u16SectorPerBlock - 1 ) ));

//				if( ( nandFTL.u32CacheBaseAddress == 0xFFFFFFFF )
//					   || ( nandFTL.u32CacheBaseAddress == ( dwLogAddr & ( ~( nandFTL.u16SectorPerBlock - 1 ) ) ) ) )
//				{
					//UartOutText("->H");
					// No cache block, alloc one from freeblock list
					if( nandFTL.u16CacheBlock == 0xFFFF ) {
						if((nandFTL.u16CacheBlock = ftlFreeBlockGet(0)) < nandFTL.u16BlockPerZone){
							nandFTL.u16FreePageIndex = 0x00;
						}
					}
					// A cache block already exists
					else {
						// Realloc a new cache block and erase the old one.
//						if( ( nandFTL.u32CacheBaseAddress != ( dwLogAddr & ( ~( nandFTL.u16SectorPerBlock - 1 ) ) ) ) ||
//							 ( nandFTL.u16FreePageIndex > (nandFTL.u16PagePerBlock-3) ) )
						if( nandFTL.u16FreePageIndex > (nandFTL.u16PagePerBlock-3))
						{
#if 0
if(nandFTL.u32CacheBaseAddress != ( dwLogAddr & ( ~( nandFTL.u16SectorPerBlock - 1 ))) ){
	mpDebugPrint("CacheBaseAddr: %x, dwLogAddr: %x, !!(%x)(%x/%x)",
		nandFTL.u32CacheBaseAddress, dwLogAddr, ( dwLogAddr & ( ~( nandFTL.u16SectorPerBlock - 1 ))),
		~(nandFTL.u16SectorPerBlock - 1 ), (nandFTL.u16SectorPerBlock - 1 ));

	mpDebugPrint("....Do cache block erase....");
}
#endif
							if (NandBlockErase(nandFTL.u16CacheBlock << nandFTL.u08PagePerBlockExp) == FAIL){
								ERASE_CNT_INVALID(nandFTL.u16CacheBlock);
								badBlockMark(nandFTL.u16CacheBlock << nandFTL.u08PagePerBlockExp);
							}
							else{
								ERASE_CNT_INC(nandFTL.u16CacheBlock);
								ftlFreeBlockSet(nandFTL.u16CacheBlock);
							}

							nandFTL.u16CacheBlock = 0xFFFF;
							if((nandFTL.u16CacheBlock = ftlFreeBlockGet(0)) < nandFTL.u16BlockPerZone){
								nandFTL.u16FreePageIndex = 0x00;
							}
						}
					}
					if( ( nandFTL.u16CacheBlock != 0xFFFF )
						     && ( nandFTL.u16FreePageIndex < nandFTL.u16PagePerBlock ) ) {
//UartOutText("R");
						nandFTL.u32CacheLogAddress = dwLogAddr;
						nandFTL.u32CacheBaseAddress	= dwLogAddr & ( ~( nandFTL.u16SectorPerBlock - 1 ) );
		    	    	nandFTL.u08CacheBufferUsing = 0x01;
						nandFTL.u08CacheDataValid = 0x01;
					    nandFTL.u16CacheDataMissTimes = 0x00;

//mpDebugPrint("    a.freepageIndex...%d", nandFTL.u16FreePageIndex);
//mpDebugPrint(" cahe page> write address : %x , %x, <%x, %x> ", dwLogAddr, dwSectorCount, nandFTL.u16CacheBlock, nandFTL.u16FreePageIndex);
					}
//				}
    	    }
		}
#endif
		if(FTLWrite((DWORD)pageBuf, 1, dwLogAddr, progress) == 0)
			goto FTL_WRITE_RET_FAIL;

#if CACHEPAGE_MECHANISM
		if( nandFTL.u08CacheBufferUsing ) {
//mpDebugPrint("    b.freepageIndex...%d", nandFTL.u16FreePageIndex);
		}
		nandFTL.u08CacheBufferUsing = 0x00;
#endif
	}
	//------------------------------------------------------------------------------------------
#ifdef D3
	mpDebugPrint("\n\n");
#endif

FTL_WRITE_RET_FAIL:
	if(pageBuf)
		ext_mem_free(pageBuf);

	return ret;
}


#if NAND_DUMPAP
static DWORD  CURBLOCK;
static WORD   PAGECNT;
static DWORD  datablockTab[4096];
static WORD   tabIdx;
SWORD NandDumpInit(DWORD *rawPageSize)
{
	DWORD Blks = 0, PagePerBlk = 0, PageSz = 0;
	DWORD i, j = 1;
	DWORD logAddr;

	McardNandActive();
	GetNandGeometry(&Blks, &PagePerBlk, &PageSz);
	memset(datablockTab, 0xff, 4096 << 2);
	CURBLOCK = 0;
	PAGECNT = 0;
	tabIdx  = 0;

	for(i = 0; i < skipBlockNum; i++)
		datablockTab[i] = i;	// Always record Block 0
	mpDebugPrint("skip block num: %d", skipBlockNum);

	j = skipBlockNum;
	for (i = skipBlockNum; i < Blks ; i++)
	{
		logAddr = GetNandLogAddr(i * PagePerBlk);
		if(logAddr < LB_ERROR)	// bad block
		{
			mpDebugPrint("Block %d ok", i);
			datablockTab[j] = i;
			j++;
		}
	}

	for (i = 0; i < Blks ; i++)
		if(datablockTab[i] != 0xffffffff)
			mpDebugPrint("Block %d is recorded..", datablockTab[i]);

	CURBLOCK = datablockTab[0];
	mpDebugPrint("CurBlock..:%d", CURBLOCK);

	*rawPageSize = GetNandRawPageSize();
	//DumpFTLTables();
	return PASS;
}

SWORD NandWriteNextPage(BYTE *tmpbuf)
{
	DWORD Blks = 0, PagePerBlk = 0, PageSz = 0;
	DWORD ret = FAIL;
	DWORD i, logAddr, isWrite = 1;
	static DWORD pageCnt = 0;
	static DWORD BlockCnt = 0;


	GetNandGeometry(&Blks, &PagePerBlk, &PageSz);
	if(BlockCnt >= Blks){
		mpDebugPrint("No space and hanged on...");
		while(1);
	}

	DWORD *check = (tmpbuf + PageSz);

//	if(*check == 0xffffffff && *(check+1) == 0xffffffff && *(check+2) == 0xffffffff && BlockCnt >= skipBlockNum){
//		mpDebugPrint("Stop write: %d, %d", BlockCnt, pageCnt);
//		isWrite = 0;
//	}


	McardNandActive();
	if(pageCnt < PagePerBlk){
//		logAddr = GetNandLogAddr(CURBLOCK * PagePerBlk + PAGECNT);
//		mpDebugPrint("<%d.%d>: Address value %x/%x", CURBLOCK, PAGECNT, logAddr, Blks * PagePerBlk);
		if(isWrite)
			ret = NandRawPageWrite2(BlockCnt * PagePerBlk + pageCnt, tmpbuf);
		else
			ret = PASS;
		if(ret != PASS){
			mpDebugPrint("Shit....crash.....");
			while(1);
		}
		else{
			pageCnt++;
			//mpDebugPrint("	[W]Cur page: %d", pageCnt);
		}
	}
	else{
		pageCnt = 0;
		BlockCnt++;
		logAddr = GetNandLogAddr(BlockCnt * PagePerBlk);

		while( logAddr != LB_NO_ADDR){	// Find a useable block
			BlockCnt++;
			logAddr = GetNandLogAddr(BlockCnt * PagePerBlk);
		}

//			logAddr = GetNandLogAddr(CURBLOCK * PagePerBlk + PAGECNT);
//			mpDebugPrint("<%d.%d>: Address value %x/%x", CURBLOCK, PAGECNT, logAddr, Blks * PagePerBlk);
		if(isWrite){
			while(NandRawPageWrite2(BlockCnt * PagePerBlk, tmpbuf) != PASS){
				badBlockMark(BlockCnt * PagePerBlk);
				BlockCnt++;
				if(BlockCnt >= Blks){
					mpDebugPrint("Ran out all storage space and hanged on...");
					while(1);
				}
			}
		}
		else
			ret = PASS;
		mpDebugPrint("------ Start to write block: %d ------", BlockCnt);

		pageCnt++;
		ret = PASS;
	}

	//mpDebugPrint("Cur ret: %x", ret);
	return ret;
}

SWORD NandGetNextValidPage(BYTE *tmpbuf)
{
	DWORD Blks = 0, PagePerBlk = 0, PageSz = 0;
	DWORD ret = FAIL;
	DWORD logAddr = 0;

	GetNandGeometry(&Blks, &PagePerBlk, &PageSz);

	McardNandActive();
	if(PAGECNT < PagePerBlk){
		logAddr = GetNandLogAddr(CURBLOCK * PagePerBlk + PAGECNT);
//		mpDebugPrint("<%d.%d>: Address value %x/%x", CURBLOCK, PAGECNT, logAddr, Blks * PagePerBlk);
		if(logAddr == LB_NO_ADDR){
			//Mark this page is abandoned first and read again
			DWORD progress;
			memset(tmpbuf, 0xff, PageSz);
			NandPageWrite(CURBLOCK * PagePerBlk + PAGECNT, 1, tmpbuf, 0xfffffff0, &progress);
			NandRawPageRead2(CURBLOCK * PagePerBlk + PAGECNT, tmpbuf);
		}
		else{
			NandRawPageRead2(CURBLOCK * PagePerBlk + PAGECNT, tmpbuf);
		}
		PAGECNT++;
		ret = PASS;
	}
	else{
		tabIdx++;
		PAGECNT = 0;
		CURBLOCK = datablockTab[tabIdx];
		if(CURBLOCK != 0xffffffff){
			logAddr = GetNandLogAddr(CURBLOCK * PagePerBlk + PAGECNT);
			mpDebugPrint("<%d.%d>: Address value %x/%x", CURBLOCK, PAGECNT, logAddr, Blks * PagePerBlk);
			NandRawPageRead2(CURBLOCK * PagePerBlk, tmpbuf);
			PAGECNT++;
			ret = PASS;
		}
	}

	return ret;
}
#endif

static SWORD LowLevelFormat(BYTE deepVerify)
{
	DWORD i, j;
	DWORD Blks = 0, PagePerBlk = 0, PageSz = 0;
	DWORD ret = PASS;

	McardNandActive();
	GetNandGeometry(&Blks, &PagePerBlk, &PageSz);
	BYTE *ptr = (BYTE *)((DWORD)ext_mem_malloc(PagePerBlk * PageSz) | 0xA0000000);
	mpDebugPrint("Erase nand flash (%d blk)", Blks);

//	for (i = 0; i < Blks ; i++)
	for (i = skipBlockNum; i < Blks ; i++)
	{
		if (GetNandLogAddr(i*PagePerBlk) == LB_DEFAULT_BAD_BLOCK)	// bad block
		{
			mpDebugPrint("Original Bad Block %d", i);
		}
#if (NAND_ID_TYPE == HYNIX_HB_SERIES)
		else if (GetNandLogAddr((i*PagePerBlk)+PagePerBlk-1) == LB_DEFAULT_BAD_BLOCK)	// bad block
		{
			mpDebugPrint("Original Bad Block %d", i);
		}
#endif
		else
		{
			if (!deepVerify)
			{
				if (NandBlockErase(i*PagePerBlk) == FAIL)
				{
					ERASE_CNT_INVALID(i);
					mpDebugPrint("New Bad Block %d", i);
					badBlockMark(i*PagePerBlk);	// mark as bad
				}
				else
				{
					ERASE_CNT_INC(i);
				}
			}
			else
			{
				memset(ptr, 0, PagePerBlk * PageSz);
				// 1. erase first
				if (NandBlockErase(i*PagePerBlk) != PASS)
				{
					ERASE_CNT_INVALID(i);
					mpDebugPrint("Erase Failed! New Bad Block %d", i);
					badBlockMark(i*PagePerBlk);	// mark as bad
				}
				else
				{
					ERASE_CNT_INC(i);
				}
				// 2. write data pattern
				if (NandPageWrite(i*PagePerBlk, PagePerBlk, (DWORD)ptr, 0xFFFFFFFF, NULL) != PASS)
				{
					ERASE_CNT_INVALID(i);
					mpDebugPrint("Program Failed! New Bad Block %d", i);
					NandBlockErase(i*PagePerBlk);
					badBlockMark(i*PagePerBlk);	// mark as bad
				}
				else
				{
					memset(ptr, 0x5a, PagePerBlk * PageSz);
					// 3. read and verify
					if (NandPageRead(i*PagePerBlk, PagePerBlk, (DWORD)ptr, NULL) == PagePerBlk)
					{
						DWORD *tmp = (DWORD *)ptr;
						SWORD err = 0;

						for (j = 0 ; j < (PagePerBlk*PageSz>>2) ; j++)
						{
							if (tmp[j] != 0)
							{
								ERASE_CNT_INVALID(i);
								mpDebugPrint("Pattern test Failed! New Bad Block %d", j);
								NandBlockErase(i*PagePerBlk);
								badBlockMark(i*PagePerBlk);	// mark as bad
								err = 1;
								break;
							}
						}

						if (err == 0)
						{
							PutUartChar('[');
							UartOutValue(i, 4);
							PutUartChar(']');
							NandBlockErase(i*PagePerBlk);
							ERASE_CNT_INC(i);
						}
					}
					else
					{
						ERASE_CNT_INVALID(i);
						mpDebugPrint("Read Failed! New Bad Block %d", j);
						NandBlockErase(i*PagePerBlk);
						badBlockMark(i*PagePerBlk);	// mark as bad
					}
				}

			}
		}
	}
	mpDebugPrint("Erase done");
	ext_mem_free(ptr);

	ret = NandFtlInit();

	return ret;
}



static void CommandProcess(void *pMcardDev)
{
	register ST_MCARD_DEV *pDev = ((ST_MCARD_DEV *) pMcardDev);
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;

	//mpDebugPrint("FTL(%x) Command: %x(%x,%d)", pDev->wMcardType, mail->wCmd, mail->dwBlockAddr, mail->dwBlockCount);
#if SD_MMC_ENABLE
    SdProgChk();
#endif
	SetMcardClock(NAND_CLOCK_KHZ);
	McardNandActive();
	switch (mail->wCmd)
	{
		case INIT_CARD_CMD:
			pDev->Flag.Detected = 1;
			if ((mail->swStatus = NandFtlInit()) != FAIL)
			{
				pDev->Flag.Present = 1;
				pDev->dwCapacity = (nandFTL.u16DataBlockPerZone * nandFTL.u08ZoneNum) << (nandFTL.u08PagePerBlockExp + nandFTL.u08SectorPerPageExp);
				pDev->wSectorSize = MCARD_SECTOR_SIZE;
				pDev->wSectorSizeExp = MCARD_SECTOR_SIZE_EXP;
				pDev->wRenewCounter++;
				mail->swStatus = PASS;
				NandInfoDump();
			}
			else
			{
				mpDebugPrint("FTL initial failed!");
			}
			break;

		case REMOVE_CARD_CMD:
			//DumpFTLTables();
			pDev->Flag.Present = 0;
			pDev->Flag.ReadOnly = TRUE;
			pDev->Flag.PipeEnable = 0;
			mail->swStatus = 0;
			pDev->dwCapacity = 0;
			pDev->wSectorSize = 0;
			pDev->wSectorSizeExp = 0;
			break;

		case SECTOR_READ_CMD:
			if (mail->dwBlockAddr < pDev->dwCapacity)
			{
				mail->swStatus = FlatRead(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr, &mail->dwProgress);
			}
			else
			{
				mpDebugPrint("NAND : read() invalid blk %x, max=%x", mail->dwBlockAddr, pDev->dwCapacity);
				mail->swStatus = FAIL;
			}
			break;

		case SECTOR_WRITE_CMD:
			if (mail->dwBlockAddr < pDev->dwCapacity)
			{
				if (pDev->Flag.ReadOnly != TRUE)
					mail->swStatus = FlatWrite(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr, &mail->dwProgress);
				else
					mpDebugPrint("[WARN] NAND is at Read Only status....");
			}
			else
			{
				mpDebugPrint("NAND : read() invalid blk %x, max=%x", mail->dwBlockAddr, pDev->dwCapacity);
				mail->swStatus = FAIL;
			}
			break;

		case RAW_FORMAT_CMD:
			mail->swStatus = LowLevelFormat(mail->dwBlockAddr);
			break;

#if NAND_DUMPAP
		case RAWPAGE_READ_CMD:
			mail->swStatus = NandGetNextValidPage(mail->dwBuffer);
			break;

		case RAWPAGE_WRITE_CMD:
			mail->swStatus = NandWriteNextPage(mail->dwBuffer);
			break;
#endif
		default:
			mail->swStatus = FAIL;
			break;
	}
	pDev->Flag.ReadOnly = nandFTL.ReadOnly;
	McardNandInactive();
	//mpDebugPrint("\t->Done!");
}

void NandInit(ST_MCARD_DEV *sDev)
{
	MP_DEBUG(__FUNCTION__);
	MP_ASSERT(sDev != NULL);

	sDev->pbDescriptor = bDescriptor;
	sDev->wMcardType = DEV_NAND;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
}

enum
{
	WL_POS_VER	= 0,
	WL_POS_PW_ON,
	WL_POS_ERASE_CNT
};

void NandSetWLInfo(DWORD *buf)
{
	static BOOL init = 0;

	if (buf[WL_POS_VER] == NAND_ERASE_CNT_VER)	// if erase count stored with different structure, ignore the stored informations.
	{
		mmcp_memcpy_polling((BYTE *)nandFTL.pEraseCnt, (BYTE *)&buf[WL_POS_ERASE_CNT], nandFTL.u08ZoneNum * nandFTL.u16BlockPerZone * sizeof(DWORD));
		#if BLK_ERS_TAB_DUMP
		DWORD i, max=0, min=-1, idx_max=-1, idx_min=-1;
		BYTE info[100];
		for (i = 0 ; i < nandFTL.u08ZoneNum * nandFTL.u16BlockPerZone ; i++)
		{
			if (i >= skipBlockNum)
			{
				if (nandFTL.pEraseCnt[i] != -1 && nandFTL.pEraseCnt[i] > max)
				{
					idx_max = i;
					max = nandFTL.pEraseCnt[i];
				}
				if (nandFTL.pEraseCnt[i] < min)
				{
					idx_min = i;
					min = nandFTL.pEraseCnt[i];
				}
			}
			sprintf(info, "%s [%03x]:%04x", info, i, (WORD)nandFTL.pEraseCnt[i]);
			if (i!=0&&((i & 0x03) == 0))
			{
				mpDebugPrint(info);
				info[0] = '\0';
			}
		}
		mpDebugPrint("\tErase count threshold=%dtimes", EraseCntThreshold);
		mpDebugPrint("\tErase Count max=blk%d(%dtimes), min=blk%d(%dtimes)", idx_max, max, idx_min, min);
		#endif
	}
	else
	{
		mpDebugPrint("Erase count re-build...");
	}
	if (init == 0)
	{
		init = 1;
		nandFTL.u32PowerUpCnt = buf[WL_POS_PW_ON] + 1;
		nandFTL.bEraseCntUpdate = 1;
		mpDebugPrint("PwrOn[%d]", nandFTL.u32PowerUpCnt);
		ftlWLStatistics();
	}
}

DWORD NandGetWLInfoSize()
{
	return (sizeof(DWORD) * WL_POS_ERASE_CNT) + nandFTL.u08ZoneNum * nandFTL.u16BlockPerZone * sizeof(DWORD);
}

BOOL NandWLInfoUpdate()
{
	if (nandFTL.u32PowerUpCnt == 0)	// first time case
		nandFTL.bEraseCntUpdate = 1;
	return nandFTL.bEraseCntUpdate;
}

void NandGetWLInfo(DWORD *buf)
{
	if (nandFTL.bEraseCntUpdate)
	{
		buf[WL_POS_VER] = NAND_ERASE_CNT_VER;
		buf[WL_POS_PW_ON] = nandFTL.u32PowerUpCnt;
		mmcp_memcpy_polling((BYTE *)&buf[WL_POS_ERASE_CNT], (BYTE *)nandFTL.pEraseCnt, nandFTL.u08ZoneNum * nandFTL.u16BlockPerZone * sizeof(DWORD));
		nandFTL.bEraseCntUpdate = 0;
	}
}

#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
static void IspCommandProcess(void *pMcardDev)
{
	ST_MCARD_DEV *pDev = (ST_MCARD_DEV *) pMcardDev;
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;
	DWORD tmp;
	static DWORD BlkNr = 0, SectorPerBlk = 0, SectorPerPage2Exp = 0, pageperblk = 0;

	mail->swStatus = PASS;
	SetMcardClock(NAND_CLOCK_KHZ);
	//mpDebugPrint("NandIsp(%x) Command: %x(%x,%d)", pDev->wMcardType, mail->wCmd, mail->dwBlockAddr, mail->dwBlockCount);
	McardNandActive();
	switch (mail->wCmd)
	{
		case CODE_IDENTIFY_CMD:
			{
				static BOOL inited = FALSE;
				DWORD pagesize;

				if (inited == FALSE)
				{
					if(identified == FALSE)
					{
						if (NandIdentify() == PASS)
						{
							identified = TRUE;
							NandFTLInfoInit();
						}
					}
					BootPageCheck();
					inited = TRUE;
					GetNandGeometry(&BlkNr, &pageperblk, &pagesize);
					SectorPerBlk = pageperblk * pagesize >> MCARD_SECTOR_SIZE_EXP;
					SectorPerPage2Exp = CalValue2Exp(pagesize) - MCARD_SECTOR_SIZE_EXP;
					if (IspBadBlkTable == NULL)
					{
						DWORD i;

						skipBlockNum = CAL_SKIP_BLKS(pageperblk*pagesize, ISP_GetNandReservedSize());
						IspBadBlkTable = (BYTE *)ker_mem_malloc(skipBlockNum, TaskGetId());
						for (i = 0 ; i < skipBlockNum ; i++)
						{
							DWORD logblk = GetNandLogAddr(i*pageperblk);
							if ((logblk == LB_BAD_BLOCK) || (logblk == LB_DEFAULT_BAD_BLOCK))
							{
								IspBadBlkTable[i] = TRUE;
								MP_DEBUG("    ISP bad block %X  --CODE_IDENTIFY_CMD", i*pageperblk);
							}
							else
							{
#if (NAND_ID_TYPE == HYNIX_HB_SERIES)
								logblk = GetNandLogAddr((i*pageperblk)+pageperblk-1);
								if ((logblk == LB_BAD_BLOCK) || (logblk == LB_DEFAULT_BAD_BLOCK))
								{
									IspBadBlkTable[i] = TRUE;
									MP_DEBUG("    ISP bad block %X  --CODE_IDENTIFY_CMD", (i*pageperblk)+pageperblk-1);
								}
								else
								{
									IspBadBlkTable[i] = FALSE;
								}
#else
								IspBadBlkTable[i] = FALSE;
#endif
							}
						}
					}
				}
				else{
					GetNandGeometry(&BlkNr, &pageperblk, &pagesize);
					SectorPerBlk = pageperblk * pagesize >> MCARD_SECTOR_SIZE_EXP;
				}

				((DWORD *)mail->dwBuffer)[0] = SectorPerBlk;	// Sector per block
				((DWORD *)mail->dwBuffer)[1] = BlkNr;			// total block number
				((DWORD *)mail->dwBuffer)[2] = pagesize;		// Bytes per page
			}
			break;

		case ISP_READ_CMD:
			// Another FTLRead function include De-randomization after NandPageRead
			mail->swStatus = FTLISPRead(-1, mail->dwBlockAddr, mail->dwBlockCount, mail->dwBuffer, NULL);
			break;

		case ISP_WRITE_CMD:
#if NAND_PAGE_RANDOM
{
			// De-randomization for ISP write original data
			DWORD pagesize, i;
			DWORD PageAdd = mail->dwBlockAddr >> SectorPerPage2Exp;
			DWORD BufAdd = mail->dwBuffer;
			GetNandGeometry(&BlkNr, &pageperblk, &pagesize);

			for (i=0 ; i<(mail->dwBlockCount)*pageperblk ; i++)
			{
				NandDataRandom((BYTE *)BufAdd,
								(BYTE *)BufAdd,
								nandFTL.u16PageSize,
								((PageAdd&0xFF)*NAND_RANDOM_TBL_IDX));

				mail->swStatus = NandPageWrite(PageAdd, 1, BufAdd, 0xffffffff, NULL);

				// De-randomization for buffer keep original data
				NandDataRandom((BYTE *)BufAdd,
								(BYTE *)BufAdd,
								nandFTL.u16PageSize,
								((PageAdd&0xFF)*NAND_RANDOM_TBL_IDX));

				BufAdd += pagesize;
				PageAdd++;
			}
}
#else
			mail->swStatus = NandPageWrite(mail->dwBlockAddr>>SectorPerPage2Exp, mail->dwBlockCount*pageperblk, mail->dwBuffer, 0xffffffff, NULL);
#endif
			break;

		case CODE_BLK_INVALID_CMD:
			if (IspBadBlkTable == NULL)
			{
				DWORD logblk = GetNandLogAddr(mail->dwBlockAddr>>SectorPerPage2Exp);
				if ((logblk == LB_BAD_BLOCK) || (logblk == LB_DEFAULT_BAD_BLOCK))
				{
					((DWORD *)mail->dwBuffer)[0] = TRUE;
					MP_DEBUG("    ISP bad block %X  --CODE_BLK_INVALID_CMD", logblk);
				}
				else
				{
#if (NAND_ID_TYPE == HYNIX_HB_SERIES)
					DWORD pagesize;

					GetNandGeometry(&BlkNr, &pageperblk, &pagesize);
					logblk = GetNandLogAddr((mail->dwBlockAddr>>SectorPerPage2Exp)+pageperblk-1);
					if ((logblk == LB_BAD_BLOCK) || (logblk == LB_DEFAULT_BAD_BLOCK))
					{
						((DWORD *)mail->dwBuffer)[0] = TRUE;
						MP_DEBUG("    ISP bad block %X  --CODE_BLK_INVALID_CMD", logblk);
					}
					else
					{
						((DWORD *)mail->dwBuffer)[0] = FALSE;
					}
#else
					((DWORD *)mail->dwBuffer)[0] = FALSE;
#endif
				}
			}
			else
			{
				((DWORD *)mail->dwBuffer)[0] = IspBadBlkTable[mail->dwBlockAddr/SectorPerBlk];
			}
			break;

		case CODE_BLK_ERASE_CMD:
			tmp = NandBlockErase(mail->dwBlockAddr>>SectorPerPage2Exp);
			if (tmp != PASS)
			{
				ERASE_CNT_INVALID(mail->dwBlockAddr/SectorPerBlk);
				badBlockMark(mail->dwBlockAddr>>SectorPerPage2Exp);	// mark as bad
				if (IspBadBlkTable == NULL)
					IspBadBlkTable[mail->dwBlockAddr/SectorPerBlk] = TRUE;
				else
					IspBadBlkTable[mail->dwBlockAddr/SectorPerBlk] = FALSE;
			}
			else
			{
				ERASE_CNT_INC(mail->dwBlockAddr/SectorPerBlk);
			}
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
