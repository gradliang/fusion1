
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
* Filename      : chain.c
* Programmers   :
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
#include "fat.h"
#include "exfat.h"
#include "file.h"
#include "drive.h"
#include "chain.h"
//#include "os.h"
#include "mpTrace.h"

#include <string.h>


extern BOOL (*fileUiUpdateIconAniPtr)(void);


/* note: Do not update these static variables directly */
static BOOL f_ChainCopy_Inprogress = FALSE;
static DRIVE  *ChainCopy_SrcDrv = NULL, *ChainCopy_TrgDrv = NULL;

/* note: only do Set_Copy_SrcTrgDrives_Status() and Reset_Copy_SrcTrgDrives_Status() in ChainCopy() in chain.c !! */
static void Reset_Copy_SrcTrgDrives_Status(void);
static void Set_Copy_SrcTrgDrives_Status(DRIVE * src_drv, DRIVE * trg_drv);
static BOOL Check_Copy_SrcTrgDrives_Both_Present(void);



/*
*******************************************************************************
*        GLOBAL FUNCTIONS
*******************************************************************************
*/

///
///@ingroup CHAIN
///@brief   Clear a number of sectors (fully fill with 0x00 to whole sectors).
///
///@param   drv        The drive to access. \n\n
///@param   start      The lba of the starting sector to be cleared. \n\n
///@param   count      Number of sectors to be cleared.
///
///@retval  FS_SUCCEED          Clear sectors successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL   Clear sectors unsuccessfully. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
int SectorClear(DRIVE * drv, DWORD start, SDWORD count)
{
	BYTE *addr;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	addr = (BYTE *) ker_mem_malloc(1 << drv->bSectorExp, TaskGetId());
	if (addr == NULL)
	{
		MP_ALERT("-E SectorClear malloc fail");
		return DRIVE_ACCESS_FAIL;
	}

	MpMemSet(addr, 0, 1 << drv->bSectorExp);

	while (count)
	{
		if (DriveWrite(drv, addr, start, 1) != FS_SUCCEED)
		{
			ker_mem_free(addr);
			return DRIVE_ACCESS_FAIL;
		}
		start++;
		count--;
	}

	ker_mem_free(addr);
	return FS_SUCCEED;
}



///
///@ingroup CHAIN
///@brief   Initialize a chain structure.
///
///@param   chain      The chain to be initialized. \n\n
///@param   start      The start cluster of the chain. \n\n
///@param   size       The total size of the chain.
///
///@return   None.
///
///@remark   If ('start' == 0), means in the root directory area of FAT12/FAT16.
///
void ChainInit(CHAIN * chain, DWORD start, DWORD size)
{
	if (chain == NULL)
	{
		MP_ALERT("%s: Error! NULL chain pointer !", __FUNCTION__);
		return;
	}

	chain->Start = start;
	chain->Current = start;
	chain->Point = 0;
	chain->Size = size;
}



/////////////////////////////////
//         utilities           //
/////////////////////////////////

//Dangerous!!	Checked by C.W 080613
WORD LoadUnalign16(void * in)
{
	BYTE_STRC out;


	if (in == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return 0; //just return! what value is suitable ?
	}

	out.Byte0 = ((BYTE *) in)[0];
	out.Byte1 = ((BYTE *) in)[1];

	return *((WORD *) (&out));
}



//Dangerous!!	Checked by C.W 080613
DWORD LoadUnalign32(void * in)
{
	BYTE_STRC out;


	if (in == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return 0; //just return! what value is suitable ?
	}

	out.Byte0 = ((BYTE *) in)[0];
	out.Byte1 = ((BYTE *) in)[1];
	out.Byte2 = ((BYTE *) in)[2];
	out.Byte3 = ((BYTE *) in)[3];

	return *((DWORD *) (&out));
}



WORD LoadAlien16(void * in)		// the alien mean big-endian and maybe un-alignned
{
	register WORD out;


	if (in == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return 0; //just return! what value is suitable ?
	}

	out = ((BYTE_STRC *) in)->Byte1;
	out = out << 8;
	out += ((BYTE_STRC *) in)->Byte0;

	return out;
}



#if 0
DWORD LoadAlien32(void * in)		// the alien mean big-endian and maybe un-alignned
{
	register DWORD out;


	if (in == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return 0; //just return! what value is suitable ?
	}

	out = ((BYTE_STRC *) in)->Byte3;
	out = out << 8;
	out += ((BYTE_STRC *) in)->Byte2;
	out = out << 8;
	out += ((BYTE_STRC *) in)->Byte1;
	out = out << 8;
	out += ((BYTE_STRC *) in)->Byte0;

	return out;
}
#endif



void SaveAlien16(void * addr, WORD data)
{
	if (addr == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return;
	}

	((BYTE_STRC *) addr)->Byte0 = data;
	data = data >> 8;
	((BYTE_STRC *) addr)->Byte1 = data;
}



void SaveAlien32(void * addr, DWORD data)
{
	if (addr == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return;
	}

	((BYTE_STRC *) addr)->Byte0 = data;
	data = data >> 8;
	((BYTE_STRC *) addr)->Byte1 = data;
	data = data >> 8;
	((BYTE_STRC *) addr)->Byte2 = data;
	data = data >> 8;
	((BYTE_STRC *) addr)->Byte3 = data;
}



// input the address of the little-endian 16-bits value array for source
// input the address of the WORD array address for target and a length in bytes
// return the rear address the target address after copying
WORD *LoadAlienArray16(WORD * target, void * source, int length)
{
	register BYTE *point;
	register WORD unicode;


	if ((target == NULL) || (source == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return NULL;
	}

	point = (BYTE *) source;
	point++;					// point to the MSB of the first value of source

	while (length)
	{
		unicode = *point;		// load the MSB
		point--;				// move the LSB
		unicode <<= 8;			// shift target temp to high byte
		unicode += *point;		// load the LSB to low byte of target temp
		point += 3;				// move the the MSB of next half word
		*target = unicode;		// save the target
		target++;
		length -= 2;
	}

	return target;				// return the rear address of array
}

/////////////////utility/////////////////////////////////



/*
*******************************************************************************
*        LOCAL FUNCTIONS
*******************************************************************************
*/

///
///@ingroup CHAIN
///@brief   Check whether if current chain point reaches EOC (end of chain) and at the cluster boundary behind the \n
///         final cluster of the specified chain.
///
///@param   drv        The drive to access. \n\n
///@param   chain      The chain to check its current chain point position.
///
///@return  Return true if current chain point reaches EOC (end of chain) and at the cluster boundary, otherwise, return false.
///
BOOL Is_ChainPoint_EOC_and_At_Boundary(DRIVE * drv, CHAIN * chain)
{
	if ((drv == NULL) || (chain == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return FALSE;
	}

	if ( (chain->Point >> (drv->ClusterExp + drv->bSectorExp)) &&
	     ((chain->Point << (32 - (drv->ClusterExp + drv->bSectorExp))) == 0) &&
	     (chain->Point >= chain->Size) )  //position at cluster boundary
		return TRUE;
	else
		return FALSE;
}



///
///@ingroup CHAIN
///@brief   Move/reset the chain point to the start of the chain.
///
///@param   chain      The chain to access.
///
///@return   None.
///
void ChainSeekSet(CHAIN * chain)
{
	if (chain == NULL)
	{
		MP_ALERT("%s: Error! NULL chain pointer !", __FUNCTION__);
		return;
	}

	chain->Current = chain->Start;
	chain->Point = 0;
}



///
///@ingroup CHAIN
///@brief   Move the chain point to the end of the chain.
///
///@param   drv        The drive to access. \n\n
///@param   chain      The chain to access.
///
///@return   None.
///
void ChainSeekEnd(DRIVE * drv, CHAIN * chain)
{
	DWORD john;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return;
	}

	if (chain == NULL)
	{
		MP_ALERT("%s: Error! NULL chain pointer !", __FUNCTION__);
		return;
	}

	chain->Point = chain->Size;

	// if not in the root of FAT12/FAT16
	if (chain->Current < 2)
		return;

	john = chain->Current;
	if (drv->Flag.FsType != FS_TYPE_exFAT)
	{
		while ((john != FAT_READ_END_OF_CHAIN) && (john >= 2))
		{
			if (! SystemCardPresentCheck(drv->DrvIndex))
			{
				MP_ALERT("%s: Card not present !", __FUNCTION__);
				return;
			}

			chain->Current = john;
			john = drv->FatRead(drv, john);

			if (drv->StatusCode == FS_SCAN_FAIL)
			{
				MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
				return;
			}
		}
	}
#if EXFAT_ENABLE
	else
	{
		BOOL f_chain_fragmented;

		if (drv->exFAT_InfoFileds->AllocBitmapSize[drv->exFAT_InfoFileds->VolumeFlags.ActiveFat])
		{
			BYTE bitEntry_byte = Read_AllocBitmapEntry(drv, chain->Start);
			if (bitEntry_byte == 0xFF)
			{
				MP_ALERT("%s: Error! Read_AllocBitmapEntry() failed !", __FUNCTION__);
				return;
			}
			else if (bitEntry_byte == 0)
			{
				MP_ALERT("%s: Error! The chain cluster is actually not allocated !", __FUNCTION__);
				return;
			}
		}

		if (chain->exfat_DirEntryFlags.f_NoFatChain) /* chain is not fragmented */
			f_chain_fragmented = FALSE;
		else
		{
			if (! SystemCardPresentCheck(drv->DrvIndex))
			{
				MP_ALERT("%s: Card not present !", __FUNCTION__);
				return;
			}

			if (drv->FatRead(drv, chain->Start) == 0) /* chain is not fragmented */
				f_chain_fragmented = FALSE;
			else
			{
				if (drv->StatusCode == FS_SCAN_FAIL)
				{
					MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
					return;
				}

				f_chain_fragmented = TRUE;
			}
		}
	
		if (!f_chain_fragmented) /* chain is not fragmented */
		{
			DWORD mary = chain->Size >> (drv->ClusterExp + drv->bSectorExp);

			if ((chain->Size & ((1 << (drv->ClusterExp + drv->bSectorExp)) - 1)) || (chain->Point != chain->Size))
				chain->Current = chain->Start + mary;
			else  /* position is EOC and at the cluster boundary */
				chain->Current = chain->Start + mary - 1;

			return;
		}
		else
		{
			while ((john != FAT_READ_END_OF_CHAIN) && (john >= 2))
			{
				if (! SystemCardPresentCheck(drv->DrvIndex))
				{
					MP_ALERT("%s: Card not present !", __FUNCTION__);
					return;
				}

				chain->Current = john;
				john = drv->FatRead(drv, john);

				if (drv->StatusCode == FS_SCAN_FAIL)
				{
					MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
					return;
				}
			}
		}
	}
#endif
}



///
///@ingroup CHAIN
///@brief   Seek forward from current chain point to the specified point in the chain.
///
///@param   drv           The drive to access. \n\n
///@param   chain         The chain to access. \n\n
///@param   distance      The distance of bytes between the destination point and current point of chain.
///
///@retval  FS_SUCCEED         Seek successfully. \n\n
///@retval  OUT_OF_RANGE       Seek unsuccessfully due to out of range of chain size. \n\n
///@retval  ABNORMAL_STATUS    Seek unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE      Seek unsuccessfully due to invalid drive.
///
int ChainSeekForward(DRIVE * drv, CHAIN * chain, DWORD distance)
{
	DWORD john, mary;
	int retVal;
	DWORD ori_point;
	BOOL  f_EOF_and_AtClusBoundary = FALSE;

	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if (chain == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	if (distance == 0)
		return FS_SUCCEED;

	retVal = FS_SUCCEED;
	ori_point = chain->Point;

	john = chain->Point >> (drv->ClusterExp + drv->bSectorExp);
	chain->Point += distance; /* update chain->Point to destination point first */

	mary = chain->Point >> (drv->ClusterExp + drv->bSectorExp);

	// if at root directory of FAT12/FAT16
	if (!chain->Start)
	{
		if ((drv->Flag.FsType != FS_TYPE_FAT12) && (drv->Flag.FsType != FS_TYPE_FAT16))
		{
			MP_ALERT("%s: -E-  (chain->Start==0) but drive is not FAT12/16 ! => Impossible, Pls check file system ...", __FUNCTION__);
			chain->Point = ori_point;
			return ABNORMAL_STATUS;
		}

		return FS_SUCCEED;
	}

	/* note: in current iPlay code, 'chain->Size' may be value of "real file data length" or "allocated clusters chain size" in different cases.
	 * And we have some file-size and/or chain-size related APIs need to take care: ChainChangeSize(), FileWrite_withWriteBack_FileSize() and
	 * FileChain_AdjustSize_TrimExtraClusters().
	 *
	 * Note that the "file size" (i.e. handle->Chain.Size) of a just created file by our file system APIs is set to zero, although we have already
	 * allocated a cluster for it.
	 *
	 * So, we check first whether if destination point and current point are in the same cluster to avoid "out of range of chain size" error.
	 */
	if (mary == john) /* both positions are in the same cluster */
	{
		if (chain->Point > chain->Size)
		{
			MP_ALERT("%s: Warning: destination (%lu) > chain->Size (%lu), although both positions are in the same cluster.", __FUNCTION__, chain->Point, chain->Size);
		}

		return FS_SUCCEED;
	}

	DWORD clus_count, range_max;
	if (chain->Size == 0)
		clus_count = 1;
	else
	{
		clus_count = chain->Size >> (drv->ClusterExp + drv->bSectorExp);
		if (chain->Size % (1 << (drv->ClusterExp + drv->bSectorExp)))
			clus_count += 1;
	}
	range_max = clus_count << (drv->ClusterExp + drv->bSectorExp); 

	if (chain->Point > range_max) /* note: here NOT (chain->Point >= range_max), nor (chain->Point >= chain->Size) */
	{
		chain->Point = ori_point; //recover chain->Point value
		MP_ALERT("%s: -E- destination out of range of chain ! (chain->Size = %lu, range_max = %lu, ori point = %lu, distance = %lu, dest Point = %lu)",
		         __FUNCTION__, chain->Size, range_max, ori_point, distance, chain->Point);
		return OUT_OF_RANGE;
	}

	f_EOF_and_AtClusBoundary = Is_ChainPoint_EOC_and_At_Boundary(drv, chain);

	DWORD temp_clus = chain->Current;
	if (drv->Flag.FsType != FS_TYPE_exFAT)
	{
		while (john < mary) /* move to the specified cluster position */
		{
			if (! SystemCardPresentCheck(drv->DrvIndex))
			{
				MP_ALERT("%s: Card not present !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}

			temp_clus = drv->FatRead(drv, chain->Current);
			if (temp_clus != FAT_READ_END_OF_CHAIN)
				chain->Current = temp_clus;
			else
			{
				if (drv->StatusCode == FS_SCAN_FAIL)
				{
					MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
					return ABNORMAL_STATUS;
				}

				if (f_EOF_and_AtClusBoundary)
				{
					MP_ALERT("%s: -I- position is EOF and at cluster boundary => chain->Current is the final cluster.", __FUNCTION__);
				}

				MP_DEBUG("%s: next cluster == 0xffffffff, break loop !", __FUNCTION__);
				break;
			}

			john++;
		}
	}
#if EXFAT_ENABLE
	else
	{
		BOOL f_chain_fragmented;

		if (drv->exFAT_InfoFileds->AllocBitmapSize[drv->exFAT_InfoFileds->VolumeFlags.ActiveFat])
		{
			BYTE bitEntry_byte = Read_AllocBitmapEntry(drv, chain->Start);
			if (bitEntry_byte == 0xFF)
			{
				MP_ALERT("%s: Error! Read_AllocBitmapEntry() failed !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}
			else if (bitEntry_byte == 0)
			{
				MP_ALERT("%s: Error! The chain cluster is actually not allocated !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}
		}

		if (chain->exfat_DirEntryFlags.f_NoFatChain) /* chain is not fragmented */
			f_chain_fragmented = FALSE;
		else
		{
			if (! SystemCardPresentCheck(drv->DrvIndex))
			{
				MP_ALERT("%s: Card not present !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}

			if (drv->FatRead(drv, chain->Start) == 0) /* chain is not fragmented */
				f_chain_fragmented = FALSE;
			else
			{
				if (drv->StatusCode == FS_SCAN_FAIL)
				{
					MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
					return ABNORMAL_STATUS;
				}

				f_chain_fragmented = TRUE;
			}
		}
	
		if (!f_chain_fragmented) /* chain is not fragmented */
		{
			if (!f_EOF_and_AtClusBoundary)
				chain->Current = chain->Start + mary;
			else  /* position is EOC and at the cluster boundary */
				chain->Current = chain->Start + mary - 1;
		}
		else
		{
			while (john < mary) /* move to the specified cluster position */
			{
				if (! SystemCardPresentCheck(drv->DrvIndex))
				{
					MP_ALERT("%s: Card not present !", __FUNCTION__);
					return ABNORMAL_STATUS;
				}

				temp_clus = drv->FatRead(drv, chain->Current);
				if (temp_clus != FAT_READ_END_OF_CHAIN)
				{
					chain->Current = temp_clus;
					john++;
				}
				else
				{
					if (drv->StatusCode == FS_SCAN_FAIL)
					{
						MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
						return ABNORMAL_STATUS;
					}

					if (f_EOF_and_AtClusBoundary)
					{
						MP_ALERT("%s: -I- position is EOF and at cluster boundary => chain->Current is the final cluster.", __FUNCTION__);
					}

					MP_DEBUG("%s: next cluster == 0xffffffff, break loop !", __FUNCTION__);
					break;
				}
			}
		}
	}
#endif

	return retVal;
}



///
///@ingroup CHAIN
///@brief   Seek from current chain point to the specified position in the chain.
///
///@param   drv           The drive to access. \n\n
///@param   chain         The chain to access. \n\n
///@param   position      The position (offset from the starting point of chain) to seek.
///
///@retval  FS_SUCCEED          Seek successfully. \n\n
///@retval  OUT_OF_RANGE        Seek unsuccessfully due to out of range of chain size. \n\n
///@retval  ABNORMAL_STATUS     Seek unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
int ChainSeek(DRIVE * drv, CHAIN * chain, DWORD position)
{
	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if (chain == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	if (position == chain->Point)
		return FS_SUCCEED;
	else if (position < chain->Point)
	{
		if( (position >> (drv->ClusterExp + drv->bSectorExp))
		     == (chain->Point >> (drv->ClusterExp + drv->bSectorExp)) )  //position within same (current) cluster
		{
			chain->Point = position;
			return FS_SUCCEED;
		}
		else
		{
			ChainSeekSet(chain);
			return ChainSeekForward(drv, chain, position);
		}
	}
	else
		return ChainSeekForward(drv, chain, position - chain->Point);
}



///
///@ingroup CHAIN
///@brief   Change the total size of chain to the specified chain size.
///
///@param   drv        The drive to access. \n\n
///@param   chain      The chain to change its size. \n\n
///@param   size       The specified chain size to change to.
///
///@retval  FS_SUCCEED            Change size successfully. \n\n
///@retval  OVER_ROOT_BOUNDARY    Change size unsuccessfully due to specified size greater than FAT12/16 root directory size. \n\n
///@retval  DISK_FULL             Change size unsuccessfully due to no free cluster available on the drive. \n\n
///@retval  ABNORMAL_STATUS       Change size unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE         Invalid drive.
///
///@remark  After this function call, the chain point will move to the end of the chain.
///
int ChainChangeSize(DRIVE * drv, CHAIN * chain, DWORD size)
{
	DWORD john, mary;


	MP_DEBUG("#### enter %s(): just entered, (chain->Size, chain->Point) = (%lu, %lu)", __FUNCTION__, chain->Size, chain->Point);
	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if (chain == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	if (drv->FreeClusters == 0)  /* maybe FAT table of this drive has not been scanned yet */
		DriveFreeClustersCountGet(drv); /* force to do ScanFat() for FAT12/16/32 drive */

  #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
	if (! drv->Flag.AllocBitmapReady)
	{
		MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}
  #endif

	if ( f_ChainCopy_Inprogress && !Check_Copy_SrcTrgDrives_Both_Present() )
	{
		MP_ALERT("Copy failed -- Card not present !");
		return ABNORMAL_STATUS;
	}
	else if (! SystemCardPresentCheck(drv->DrvIndex))
	{
		MP_ALERT("%s: Card not present !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	/* if in the root directory of FAT12/FAT16 */
	if (!chain->Start)
	{
		/* note: for FAT12/16, drv->RootStart is lba of first sector of root directory */
		if (size > (drv->DataStart - drv->RootStart) << drv->bSectorExp)
		{
			MP_ALERT("%s: Error! Over FAT12/16 root directory boundary !", __FUNCTION__);
			return OVER_ROOT_BOUNDARY;
		}

		chain->Size = size;
		chain->Point = size;
		return FS_SUCCEED;
	}

#if FS_REENTRANT_API
	BOOL f_Is_ext_working_drv;
	DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);
	BYTE curr_task_id = TaskGetId();

	/* check whether if this drive handle is an external working drive copy */
	f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;

	if (f_Is_ext_working_drv) /* the input 'drv' is an external working drive copy */
	{
		/* check whether if previous FAT entries updating process by another task is still not finished */
		while ( (ori_drv_in_drive_table->CacheBufPoint->f_IsUpdatingFatEntries) && 
		        (ori_drv_in_drive_table->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id) )
		{
			MP_DEBUG("%s: task ID %d is waiting FAT update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, ori_drv_in_drive_table->CacheBufPoint->TaskId_of_UpdatingFatEntries);
			TaskSleep(10); /* force task context switch to let another task to finish its process first */
		}

		/* check whether if FSInfo free cluster info in the original drive table entry has been changed */
		if ((drv->LastCluster != ori_drv_in_drive_table->LastCluster) || (DriveFreeClustersCountGet(drv) != ori_drv_in_drive_table->FreeClusters))
		{
			/* sync current FSInfo free cluster info into the external working drive copy */
			drv->LastCluster = ori_drv_in_drive_table->LastCluster;
			drv->FreeClusters = ori_drv_in_drive_table->FreeClusters;

			/* sync FAT cache buffer content */
			/* sync: copy from ori drive to working drive */
			MpMemCopy((BYTE *) drv->FatCacheBuffer, (BYTE *) ori_drv_in_drive_table->FatCacheBuffer, FAT_CACHE_BYTE_SIZE);
			drv->FatCachePoint = ori_drv_in_drive_table->FatCachePoint;
		}
	}

	Set_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
	if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
		Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

	/* convert the byte count to cluster count */
	john = chain->Size >> (drv->bSectorExp + drv->ClusterExp);
	if (chain->Size << (32 - drv->bSectorExp - drv->ClusterExp))
		john++;

	if (!chain->Size)
	{
		john++;  /* even the zero size have at least one cluster */
		if (drv->FatWrite(drv, chain->Current, FAT_READ_END_OF_CHAIN) != FS_SUCCEED)  /* set terminator of this chain */
		{
			MP_ALERT("%s: drv->FatWrite() failed !", __FUNCTION__);
			return ABNORMAL_STATUS;
		}
		chain->Size += (1 << (drv->bSectorExp + drv->ClusterExp));  /* update chain->Size for possible card out event */
	}

	mary = size >> (drv->bSectorExp + drv->ClusterExp);
	if (size << (32 - drv->bSectorExp - drv->ClusterExp))
		mary++;

	if (mary > john)  /* test if requested chain size greater than original chain size */
	{
		john = mary - john;  /* calculate needed new cluster count */
		if (john > DriveFreeClustersCountGet(drv))
		{
	#if FS_REENTRANT_API
			Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
			if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
				Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
	#endif

			MP_ALERT("%s: target drive disk full !", __FUNCTION__);
			return DISK_FULL;
		}

		ChainSeekEnd(drv, chain);  /* move to tail of the chain */

		while (john)
		{
			if ( f_ChainCopy_Inprogress && !Check_Copy_SrcTrgDrives_Both_Present() )
			{
				MP_ALERT("Copy failed -- Card not present !");
				return ABNORMAL_STATUS;
			}
			else if (! SystemCardPresentCheck(drv->DrvIndex))
			{
				MP_ALERT("%s: Card not present !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}

			mary = DriveNewClusGet(drv);
			if (mary == 0xffffffff)
			{
		#if FS_REENTRANT_API
				Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
				if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
					Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
		#endif

				MP_ALERT("%s: target drive disk full !", __FUNCTION__);
				return DISK_FULL;
			}

			if (drv->FatWrite(drv, chain->Current, mary) != FS_SUCCEED)  /* extend this chain on cluster */
			{
				MP_ALERT("%s: drv->FatWrite() failed !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}
			chain->Size += (1 << (drv->bSectorExp + drv->ClusterExp));  /* update chain->Size for possible card out event */

			chain->Current = mary;
			john--;
		}

		if (drv->FatWrite(drv, chain->Current, FAT_READ_END_OF_CHAIN) != FS_SUCCEED)  /* set terminator of this chain */
		{
			MP_ALERT("%s: drv->FatWrite() failed !", __FUNCTION__);
			return ABNORMAL_STATUS;
		}
	}
	else if (john > mary)  /* test if the original chain size greater than the requested chain size */
	{
		if ( f_ChainCopy_Inprogress && !Check_Copy_SrcTrgDrives_Both_Present() )
		{
			MP_ALERT("Copy failed -- Card not present !");
			return ABNORMAL_STATUS;
		}
		else if (! SystemCardPresentCheck(drv->DrvIndex))
		{
			MP_ALERT("%s: Card not present !", __FUNCTION__);
			return ABNORMAL_STATUS;
		}

		BOOL  f_Destination_AtClusBoundary;

		/* Note: we need special processing for final 'size' value at cluster boundary case here.
		/* If target position ('size') is at cluster boundary, then seek to position 'size -1' first to make chain->Current value correct.
		 * Then set FAT cluster terminator of this chain. Finally, add chain->Point with one more byte to be the 'size' value.
		 */
		if ( (size >> (drv->ClusterExp + drv->bSectorExp)) &&
		     ((size << (32 - (drv->ClusterExp + drv->bSectorExp))) == 0) ) //position at cluster boundary
		{
			f_Destination_AtClusBoundary = TRUE;
			ChainSeek(drv, chain, size - 1); /* seek to 'size - 1' position to make chain->Current value correct */
		}
		else
		{
			f_Destination_AtClusBoundary = FALSE;
			ChainSeek(drv, chain, size);
		}

		john = drv->FatRead(drv, chain->Current);

		if (drv->StatusCode == FS_SCAN_FAIL)
		{
			MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
			return ABNORMAL_STATUS;
		}

		if (drv->FatWrite(drv, chain->Current, FAT_READ_END_OF_CHAIN) != FS_SUCCEED)  /* set terminator of this chain */
		{
			MP_ALERT("%s: drv->FatWrite() failed !", __FUNCTION__);
			return ABNORMAL_STATUS;
		}

		/* Note: we need special processing for final 'size' value at cluster boundary case here.
		 * After setting FAT cluster terminator of this chain, simply add chain->Point with one more byte to be the 'size' value.
		 */
		if (f_Destination_AtClusBoundary)
			chain->Point += 1;

		while (john != FAT_READ_END_OF_CHAIN)  /* free the following clusters */
		{
			if ( f_ChainCopy_Inprogress && !Check_Copy_SrcTrgDrives_Both_Present() )
			{
				MP_ALERT("Copy failed -- Card not present !");
				return ABNORMAL_STATUS;
			}
			else if (! SystemCardPresentCheck(drv->DrvIndex))
			{
				MP_ALERT("%s: Card not present !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}

			mary = drv->FatRead(drv, john);

			if (drv->StatusCode == FS_SCAN_FAIL)
			{
				MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}

			if (drv->FatWrite(drv, john, 0) != FS_SUCCEED)
			{
				MP_ALERT("%s: drv->FatWrite() failed !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}
			chain->Size -= (1 << (drv->bSectorExp + drv->ClusterExp));  /* update chain->Size for possible card out event */

			drv->FreeClusters++;  /* update free clusters count */
			john = mary;
		}
	}

#if FS_REENTRANT_API
	/* FAT cluster allocation is changed => force to write FAT cache and FSInfo to device right away !
	 * note: this is important to sync FAT and free cluster info for concurrent multiple File I/O API invoking case.
	 */
	if (DriveRefresh(drv) != FS_SUCCEED)
	{
		Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
		if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
			Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

		MP_ALERT("%s: DriveRefresh() failed !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
	if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
		Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

	chain->Size = size;
	chain->Point = size;

	MP_DEBUG("### %s: final update (chain->Size, chain->Point) = (%lu, %lu)", __FUNCTION__, chain->Size, chain->Point);
	return FS_SUCCEED;
}



#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup CHAIN
///@brief   Extend the specified chain with one free cluster and clear the extended part.
///
///@param   drv        The drive to access. \n\n
///@param   chain      The chain to be extended.
///
///@retval  FS_SUCCEED            Extend chain size successfully. \n\n
///@retval  OVER_ROOT_BOUNDARY    Extend chain size unsuccessfully due to at root directory of FAT12/16. \n\n
///@retval  DISK_FULL             Extend chain size unsuccessfully due to no free cluster available on the drive. \n\n
///@retval  ABNORMAL_STATUS       Extend chain size unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE         Invalid drive.
///
#endif
int ChainExtending(DRIVE * drv, CHAIN * chain)
{
	DWORD john, mary;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if (chain == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	if (drv->FreeClusters == 0)  /* maybe FAT table of this drive has not been scanned yet */
		DriveFreeClustersCountGet(drv); /* force to do ScanFat() for FAT12/16/32 drive */

  #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
	if (! drv->Flag.AllocBitmapReady)
	{
		MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}
  #endif

	// if at root of FAT12/FAT16
	if (!chain->Start)
	{
		MP_ALERT("%s: Error! Cannot extend the Root Dir region of FAT12/FAT16 drive !", __FUNCTION__);
		return OVER_ROOT_BOUNDARY;
	}

	john = chain->Point;		// save the chain->Point
	ChainSeekEnd(drv, chain);	// move to tail of the chain

#if FS_REENTRANT_API
	BOOL f_Is_ext_working_drv;
	DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);

	/* check whether if this drive handle is an external working drive copy */
	f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;

	Set_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
	if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
		Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

	mary = DriveNewClusGet(drv);
	if (mary == 0xffffffff)
	{
		MP_ALERT("ChainExtending(): DriveNewClusGet(drv) failed => DISK_FULL !!");

#if FS_REENTRANT_API
		Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
		if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
			Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

		return DISK_FULL;
	}

	if (drv->FatWrite(drv, chain->Current, mary) != FS_SUCCEED) // extend this chain one cluster
	{
		MP_ALERT("ChainExtending(): drv->FatWrite() failed !!");

#if FS_REENTRANT_API
		Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
		if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
			Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

		return ABNORMAL_STATUS;
	}

	if (drv->FatWrite(drv, mary, FAT_READ_END_OF_CHAIN) != FS_SUCCEED) // set terminator of this chain
	{
		MP_ALERT("ChainExtending(): drv->FatWrite() failed !!");

#if FS_REENTRANT_API
		Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
		if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
			Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

		return ABNORMAL_STATUS;
	}

#if FS_REENTRANT_API
	/* FAT cluster allocation is changed => force to write FAT cache and FSInfo to device right away !
	 * note: this is important to sync FAT and free cluster info for concurrent multiple File I/O API invoking case.
	 */
	if (DriveRefresh(drv) != FS_SUCCEED)
	{
		Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
		if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
			Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

		return DRIVE_ACCESS_FAIL;
	}
#endif

	chain->Point = john; // recover the chain->Point
	chain->Size += (drv->ClusterSize << drv->bSectorExp);	// update the chain size

	// clear the extended part
	if (SectorClear(drv, ((mary - 2) << drv->ClusterExp) + drv->DataStart, drv->ClusterSize) != FS_SUCCEED)
	{
		MP_ALERT("ChainExtending(): SectorClear() failed !!");

#if FS_REENTRANT_API
		Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
		if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
			Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

		return ABNORMAL_STATUS;
	}

#if FS_REENTRANT_API
	Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
	if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
		Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

	return FS_SUCCEED;
}



#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup CHAIN
///@brief   Get continuous sector blocks from current chain point with size not greater than the request sector count.
///
///@param   drv        The drive to access. \n\n
///@param   chain      The chain to access. \n\n
///@param   request    [IN/OUT] The requested sector count. And then it will be updated to (request - result).
///
///@return   The number of continuous sectors.
///
///@remark  After returning, the chain point will move to the point right after this continuous blocks area.
///         And this function will update the value of 'request' parameter to (request - result).
///
#endif
DWORD ChainGetContinuity(DRIVE * drv, CHAIN * chain, DWORD * request)
{
	DWORD addend, result;
	DWORD previous, current;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return 0;
	}

	if ((chain == NULL) || (request == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return 0;
	}

	result = 0;

	// get the sector offset of the current point within the current cluster
	addend = (chain->Point >> drv->bSectorExp);
	addend = (addend & ((1 << drv->ClusterExp) - 1));
	addend = drv->ClusterSize - addend;

	if (drv->Flag.FsType != FS_TYPE_exFAT)
	{
		do
		{
			if ( f_ChainCopy_Inprogress && !Check_Copy_SrcTrgDrives_Both_Present() )
			{
				MP_ALERT("Copy failed -- Card not present !");
				return 0;
			}
			else if (! SystemCardPresentCheck(drv->DrvIndex))
			{
				MP_ALERT("%s: Card not present !", __FUNCTION__);
				return 0;
			}

			previous = chain->Current;
			current = drv->FatRead(drv, chain->Current);

			if (drv->StatusCode == FS_SCAN_FAIL)
			{
				MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
				return 0;
			}

			if (current == FAT_READ_END_OF_CHAIN)
			{
				if (addend >= *request)
				{
					result += *request;
					chain->Point += (*request << drv->bSectorExp);
				}
				else
				{
					result += addend;
					chain->Point += (addend << drv->bSectorExp);
				}
	
				*request = 0;
				break;
			}
	
			if (*request >= addend)
			{
				result += addend;
				chain->Point += (addend << drv->bSectorExp);
				*request -= addend;
			}
			else
			{
				result += *request;
				chain->Point += (*request << drv->bSectorExp);
				*request = 0;
				break;
			}
	
			addend = drv->ClusterSize;
			chain->Current = current;
		} while (chain->Current == previous + 1);
	}
#if EXFAT_ENABLE
	else
	{
		DWORD temp_clus;
		DWORD mary;
		BOOL  f_chain_fragmented;

		if (drv->exFAT_InfoFileds->AllocBitmapSize[drv->exFAT_InfoFileds->VolumeFlags.ActiveFat])
		{
			BYTE bitEntry_byte = Read_AllocBitmapEntry(drv, chain->Start);
			if (bitEntry_byte == 0xFF)
			{
				MP_ALERT("%s: Error! Read_AllocBitmapEntry() failed !", __FUNCTION__);
				return 0; /* return values: (result = 0) and (*request not changed) */
			}
			else if (bitEntry_byte == 0)
			{
				MP_ALERT("%s: Error! The chain cluster is actually not allocated !", __FUNCTION__);
				return 0; /* return values: (result = 0) and (*request not changed) */
			}
		}

		if (chain->exfat_DirEntryFlags.f_NoFatChain) /* chain is not fragmented */
			f_chain_fragmented = FALSE;
		else
		{
			if ( f_ChainCopy_Inprogress && !Check_Copy_SrcTrgDrives_Both_Present() )
			{
				MP_ALERT("Copy failed -- Card not present !");
				return 0;
			}
			else if (! SystemCardPresentCheck(drv->DrvIndex))
			{
				MP_ALERT("%s: Card not present !", __FUNCTION__);
				return 0;
			}

			if (drv->FatRead(drv, chain->Start) == 0) /* chain is not fragmented */
				f_chain_fragmented = FALSE;
			else
			{
				if (drv->StatusCode == FS_SCAN_FAIL)
				{
					MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
					return 0;
				}

				f_chain_fragmented = TRUE;
			}
		}

		if (!f_chain_fragmented) /* chain is not fragmented */
		{
			if ((chain->Point + (*request << drv->bSectorExp)) <= chain->Size)
			{
				result = *request;
				*request = 0;

				chain->Point += (result << drv->bSectorExp);
				mary = chain->Point >> (drv->ClusterExp + drv->bSectorExp);

				if ((chain->Size & ((1 << (drv->ClusterExp + drv->bSectorExp)) - 1)) || (chain->Point != chain->Size))
					chain->Current = chain->Start + mary;
				else  /* position is EOC and at the cluster boundary */
					chain->Current = chain->Start + mary - 1;
			}
			else
			{
				if (chain->Size & ((1 << drv->bSectorExp) - 1))
					result = (chain->Size >> drv->bSectorExp) - (chain->Point >> drv->bSectorExp) + 1;
				else  /* EOF position is at the sector boundary */
					result = (chain->Size >> drv->bSectorExp) - (chain->Point >> drv->bSectorExp);
				*request -= result;

				chain->Point = chain->Size;
				mary = chain->Point >> (drv->ClusterExp + drv->bSectorExp);

				if (chain->Size & ((1 << (drv->ClusterExp + drv->bSectorExp)) - 1))
					chain->Current = chain->Start + mary;
				else  /* EOF position is at the cluster boundary */
					chain->Current = chain->Start + mary - 1;
			}
		}
		else
		{
			do
			{
				if ( f_ChainCopy_Inprogress && !Check_Copy_SrcTrgDrives_Both_Present() )
				{
					MP_ALERT("Copy failed -- Card not present !");
					return 0;
				}
				else if (! SystemCardPresentCheck(drv->DrvIndex))
				{
					MP_ALERT("%s: Card not present !", __FUNCTION__);
					return 0;
				}

				previous = chain->Current;
				current = drv->FatRead(drv, chain->Current);

				if (drv->StatusCode == FS_SCAN_FAIL)
				{
					MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
					return 0;
				}

				if (current == FAT_READ_END_OF_CHAIN)
				{
					if (addend >= *request)
					{
						result += *request;
						chain->Point += (*request << drv->bSectorExp);
					}
					else
					{
						result += addend;
						chain->Point += (addend << drv->bSectorExp);
					}
		
					*request = 0;
					break;
				}
		
				if (*request >= addend)
				{
					result += addend;
					chain->Point += (addend << drv->bSectorExp);
					*request -= addend;
				}
				else
				{
					result += *request;
					chain->Point += (*request << drv->bSectorExp);
					*request = 0;
					break;
				}
		
				addend = drv->ClusterSize;
				chain->Current = current;
			} while (chain->Current == previous + 1);
		}
	}
#endif

	return result;
}



///
///@ingroup CHAIN
///@brief   Get the lba of sector that current chain point located.
///
///@param   drv        The drive to access. \n\n
///@param   chain      The chain to calculate its current point's lba value.
///
///@return   The lba value of the sector that current chain point located. \n
///          Return value 0xFFFFFFFF if the current chain point already reaches EOC (end of chain) and is over and at the cluster boundary.
///
DWORD ChainGetLba(DRIVE * drv, CHAIN * chain)
{
	DWORD lba;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return 0;
	}

	if (chain == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return 0;
	}

	if (!chain->Start) /* if at the root of FAT12/FAT16 */
	{
		/* note: for FAT12/16, drv->RootStart is lba of first sector of root directory */
		lba = drv->RootStart + (chain->Point >> drv->bSectorExp);
	}
	else
	{
		if (! Is_ChainPoint_EOC_and_At_Boundary(drv, chain))
		{
			lba = drv->DataStart + ((chain->Point >> drv->bSectorExp) & (drv->ClusterSize - 1))
			      + ((chain->Current - 2) << drv->ClusterExp);
		}
		else /* position is EOC and at the cluster boundary */
		{
			MP_ALERT("%s: -I-  position is EOC and at the cluster boundary.", __FUNCTION__);
			return 0xFFFFFFFF; /* here, use '0xFFFFFFFF' to indicate EOC and at the cluster boundary */
		}
	}

	return lba;
}



///
///@ingroup CHAIN
///@brief   Read data from chain.
///
///@param   drv         The drive to access. \n\n
///@param   chain       The chain to read data. \n\n
///@param   buffer      The pointer of buffer for data read. \n\n
///@param   sector      Length of data to read (number of sectors for Mcards, but number of bytes for USB_PTP and WiFi cases).
///
///@retval  FS_SUCCEED          Read successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL   Read unsuccessfully. \n\n
///@retval  ABNORMAL_STATUS     Read unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
///@remark  For Mcards (SD, CF, xD ... etc) cases, the 'sector' parameter is number of sectors to read.
///         But for USB_PTP and WiFi cases, 'sector' parameter is length of bytes to read.
///
int ChainRead(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD sector)
{
	DWORD count, lba, sectorCnt, tmp_cnt;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if ((chain == NULL) || (buffer == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	sectorCnt = sector;

#if USBOTG_HOST_PTP
	if ((drv->DevID == DEV_USB_HOST_PTP) || (drv->DevID == DEV_USBOTG1_HOST_PTP))
	{
  #if (SC_USBHOST)
		if ((chain->Point + sector) > chain->Size)
		{
			sector = chain->Size - chain->Point;
		}

		if (DriveRead(drv, buffer, chain->Point, sector) != FS_SUCCEED)
			return DRIVE_ACCESS_FAIL;
		chain->Point += sector;
		MP_DEBUG("read pass");
  #else
		return ABNORMAL_STATUS;
  #endif // (SC_USBHOST)
	}
	else
#endif
	{
		while (sectorCnt)
		{
			if ( f_ChainCopy_Inprogress && !Check_Copy_SrcTrgDrives_Both_Present() )
			{
				MP_ALERT("%s: Copy failed -- DRIVE_ACCESS_FAIL", __FUNCTION__);
				return DRIVE_ACCESS_FAIL;
			}

			tmp_cnt = sectorCnt;

//#if NETWARE_ENABLE
#if (NETWARE_ENABLE && MAKE_XPG_PLAYER)
			if ((drv->DevID == DEV_USB_WIFI_DEVICE)    ||\
                            (drv->DevID == DEV_CF_ETHERNET_DEVICE) ||\
                            (drv->DevID == DEV_USB_ETHERNET_DEVICE))
			{
				if ((chain->Point + sectorCnt) > chain->Size)
					sectorCnt = chain->Size - chain->Point;

				sectorCnt = NetFileRead(NetGetFileIndex(), buffer, chain->Point, (chain->Point + sectorCnt - 1));

				chain->Point += sectorCnt;

				//sectorCnt = 0; //to break loop
				if( (NetConnected() == FALSE) && (NetDevicePresent()== TRUE) )
				{
					MP_ALERT("%s: (NETWORK) DRIVE_ACCESS_FAIL", __FUNCTION__);
					return DRIVE_ACCESS_FAIL;
				}
				break;
			}
			else
#endif
			{
				lba = ChainGetLba(drv, chain);
				if (lba == 0xFFFFFFFF)
				{
					MP_ALERT("%s: -I-  EOC reached and at cluster boundary => out of chain range.", __FUNCTION__);
					return ABNORMAL_STATUS;
				}

				count = ChainGetContinuity(drv, chain, &sectorCnt);

				if (!count)
				{
					MP_ALERT("%s: -E- ChainGetContinuity() failed !", __FUNCTION__);
					return DRIVE_ACCESS_FAIL;
				}

				if (DriveRead(drv, buffer, lba, count) != FS_SUCCEED)
				{
					MP_ALERT("%s: -E- DriveRead() failed !", __FUNCTION__);
					return DRIVE_ACCESS_FAIL;
				}

				buffer += (count << drv->bSectorExp);	// increase the destination buffer point
				sectorCnt = tmp_cnt - count;
			}
		}
	}
	return FS_SUCCEED;
}



///
///@ingroup CHAIN
///@brief   Write data to chain.
///
///@param   drv         The drive to access. \n\n
///@param   chain       The chain to write data. \n\n
///@param   buffer      The pointer of data buffer. \n\n
///@param   sector      Length of data to write (unit: number of sectors for Mcards).
///
///@retval  FS_SUCCEED          Write successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL   Write unsuccessfully. \n\n
///@retval  ABNORMAL_STATUS     Write unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
int ChainWrite(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD sector)
{
	DWORD count, lba, sectorCnt, tmp_cnt;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if ((chain == NULL) || (buffer == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	if (drv->FreeClusters == 0)  /* maybe FAT table of this drive has not been scanned yet */
		DriveFreeClustersCountGet(drv); /* force to do ScanFat() for FAT12/16/32 drive */

  #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
	if (! drv->Flag.AllocBitmapReady)
	{
		MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}
  #endif

	sectorCnt = sector;
	while (sectorCnt)
	{
		if ( f_ChainCopy_Inprogress && !Check_Copy_SrcTrgDrives_Both_Present() )
		{
			MP_ALERT("%s: Copy failed -- DRIVE_ACCESS_FAIL", __FUNCTION__);
			return DRIVE_ACCESS_FAIL;
		}

		tmp_cnt = sectorCnt;

		lba = ChainGetLba(drv, chain);
		if (lba == 0xFFFFFFFF)
		{
			MP_ALERT("%s: -I-  EOC reached and at cluster boundary => out of chain range.", __FUNCTION__);
			return ABNORMAL_STATUS;
		}

		count = ChainGetContinuity(drv, chain, &sectorCnt);
		if (!count)
		{
			MP_ALERT("%s: -E- ChainGetContinuity() failed !", __FUNCTION__);
			return DRIVE_ACCESS_FAIL;
		}

		if (DriveWrite(drv, buffer, lba, count) != FS_SUCCEED)
		{
			MP_ALERT("%s: -E- DriveWrite() failed !", __FUNCTION__);
			return DRIVE_ACCESS_FAIL;
		}

		buffer = buffer + (count << drv->bSectorExp);	// increase the destination buffer point
		sectorCnt = tmp_cnt - count;
	}

	return FS_SUCCEED;
}



///
///@ingroup CHAIN
///@brief   Read data fragment which is less than size of a sector from chain.
///
///@param   drv         The drive to access. \n\n
///@param   chain       The chain to read data. \n\n
///@param   buffer      The pointer of buffer for data read. \n\n
///@param   size        Byte length of data to read, which must be less than the size of a sector.
///
///@retval  FS_SUCCEED          Read successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL   Read unsuccessfully. \n\n
///@retval  ABNORMAL_STATUS     Read unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
///@remark   Value of 'size' parameter must satisfy the condition that
///          (size + (chain->Point & ((1 << drv->bSectorExp) - 1)) <= (1 << drv->bSectorExp).
///
int ChainFragmentRead(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD size)
{
	BYTE *fragment;
	DWORD lba;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if ((chain == NULL) || (buffer == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	fragment = (BYTE *) ker_mem_malloc(1 << drv->bSectorExp, TaskGetId());
	if (fragment == NULL)
	{
		MP_ALERT("-E ChainFragmentRead malloc fail");
		return DRIVE_ACCESS_FAIL;
	}

	lba = ChainGetLba(drv, chain);
	if (lba == 0xFFFFFFFF)
	{
		MP_ALERT("%s: -I-  EOC reached and at cluster boundary => out of chain range.", __FUNCTION__);
		ker_mem_free(fragment);
		return ABNORMAL_STATUS;
	}

	if (!drv->Flag.Present)
	{
		ker_mem_free(fragment);
		return DRIVE_ACCESS_FAIL;
	}
	if (DriveRead(drv, fragment, lba, 1) != FS_SUCCEED)
	{
		MP_ALERT("ChainFragmentRead(): DriveRead() failed, return DRIVE_ACCESS_FAIL;");
		ker_mem_free(fragment);
		return DRIVE_ACCESS_FAIL;
	}
	MpMemCopy(buffer, (BYTE *) (fragment + (chain->Point & ((1 << drv->bSectorExp) - 1))), size);

	ker_mem_free(fragment);
	if (!drv->Flag.Present)
		return DRIVE_ACCESS_FAIL;
	ChainSeekForward(drv, chain, size);

	return FS_SUCCEED;
}



///
///@ingroup CHAIN
///@brief   Write data fragment which is less than size of a sector to chain.
///
///@param   drv         The drive to access. \n\n
///@param   chain       The chain to write data. \n\n
///@param   buffer      The pointer of data buffer. \n\n
///@param   size        Byte length of data to write, which must be less than the size of a sector.
///
///@retval  FS_SUCCEED          Write successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL   Write unsuccessfully. \n\n
///@retval  ABNORMAL_STATUS     Write unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
///@remark   Value of 'size' parameter must satisfy the condition that
///          (size + (chain->Point & ((1 << drv->bSectorExp) - 1)) <= (1 << drv->bSectorExp).
///
int ChainFragmentWrite(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD size)
{
	BYTE *fragment;
	DWORD lba;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if ((chain == NULL) || (buffer == NULL))
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	if (drv->FreeClusters == 0)  /* maybe FAT table of this drive has not been scanned yet */
		DriveFreeClustersCountGet(drv); /* force to do ScanFat() for FAT12/16/32 drive */

  #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
	if (! drv->Flag.AllocBitmapReady)
	{
		MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}
  #endif

	fragment = (BYTE *) ker_mem_malloc(1 << drv->bSectorExp, TaskGetId());
	if (fragment == NULL)
	{
		MP_ALERT("-E ChainFragmentWrite malloc fail");
		return DRIVE_ACCESS_FAIL;
	}

	lba = ChainGetLba(drv, chain);
	if (lba == 0xFFFFFFFF)
	{
		MP_ALERT("%s: -I-  EOC reached and at cluster boundary => out of chain range.", __FUNCTION__);
		ker_mem_free(fragment);
		return ABNORMAL_STATUS;
	}

	if (DriveRead(drv, fragment, lba, 1) != FS_SUCCEED)	// read a whole sector first
	{
		MP_ALERT("ChainFragmentWrite(): DriveRead() failed, return DRIVE_ACCESS_FAIL;");
		ker_mem_free(fragment);
		return DRIVE_ACCESS_FAIL;
	}
	MpMemCopy((BYTE *) (fragment + (chain->Point & ((1 << drv->bSectorExp) - 1))), buffer, size);	// modity the specified portion

	if (DriveWrite(drv, fragment, lba, 1) != FS_SUCCEED)	// write sector back
	{
		MP_ALERT("ChainFragmentWrite(): DriveWrite() failed, return DRIVE_ACCESS_FAIL;");
		ker_mem_free(fragment);
		return DRIVE_ACCESS_FAIL;
	}

	ker_mem_free(fragment);
	ChainSeekForward(drv, chain, size);

	return FS_SUCCEED;
}



///
///@ingroup CHAIN
///@brief   Free all the allocated clusters of a chain.
///
///@param   drv        The drive to access. \n\n
///
///@param   chain      The chain to be freed/released. \n\n
///
///@retval  FS_SUCCEED          Free/release chain clusters successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL   Free/release chain clusters unsuccessfully. \n\n
///@retval  ABNORMAL_STATUS     Free/release chain clusters unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
int ChainFree(DRIVE * drv, CHAIN * chain)
{
	DWORD size, clusterSize, current, next;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if (chain == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	if (drv->FreeClusters == 0)  /* maybe FAT table of this drive has not been scanned yet */
		DriveFreeClustersCountGet(drv); /* force to do ScanFat() for FAT12/16/32 drive */

  #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
	if (! drv->Flag.AllocBitmapReady)
	{
		MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}
  #endif

#if FS_REENTRANT_API
	BOOL f_Is_ext_working_drv;
	DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);
	BYTE curr_task_id = TaskGetId();

	/* check whether if this drive handle is an external working drive copy */
	f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;

	if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
	{
		/* check whether if previous FAT entries updating process by another task is still not finished */
		while ( (ori_drv_in_drive_table->CacheBufPoint->f_IsUpdatingFatEntries) && 
		        (ori_drv_in_drive_table->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id) )
		{
			MP_DEBUG("%s: task ID %d is waiting FAT update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, ori_drv_in_drive_table->CacheBufPoint->TaskId_of_UpdatingFatEntries);
			TaskSleep(10); /* force task context switch to let another task to finish its process first */
		}
	}

	Set_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
	if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
		Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

	size = chain->Size;
	clusterSize = 1 << (drv->ClusterExp + drv->bSectorExp);
	current = chain->Start;

	while ((current >= 2) && (current < (drv->TotalClusters + 2)))
	{
		if (! SystemCardPresentCheck(drv->DrvIndex))
		{
			MP_ALERT("%s: Card not present !", __FUNCTION__);
			return DRIVE_ACCESS_FAIL;
		}

		next = drv->FatRead(drv, current);

		if (drv->StatusCode == FS_SCAN_FAIL)
		{
			MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
			return DRIVE_ACCESS_FAIL;
		}

		drv->FatWrite(drv, current, 0);
		current = next;
		drv->FreeClusters++;
		if (size < clusterSize) /* this should be impossible, except that FAT table is corrupt */
		{
			chain->Size = 0;
			break;
		}
		else
			size = size - clusterSize;

		if (current == FAT_READ_END_OF_CHAIN) /* reach EOC (end of chain) */
			break;
	}

#if FS_REENTRANT_API
	/* FAT cluster allocation is changed => force to write FAT cache and FSInfo to device right away !
	 * note: this is important to sync FAT and free cluster info for concurrent multiple File I/O API invoking case.
	 */
	if (DriveRefresh(drv) != FS_SUCCEED)
	{
		MP_ALERT("%s: Error! DriveRefresh() failed !", __FUNCTION__);

		Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
		if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
			Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

		return DRIVE_ACCESS_FAIL;
	}

	Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
	if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
		Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

	return FS_SUCCEED;
}



///
///@ingroup CHAIN
///@brief   Free all the allocated clusters of a chain, but reserve its start cluster. After calling this function,
///         the chain size value becomes size of a cluster, because a cluster (the start cluster) is still hold by this chain.
///
///@param   drv        The drive to access. \n\n
///
///@param   chain      The chain to be freed/released. \n\n
///
///@retval  FS_SUCCEED          Free/release chain clusters successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL   Free/release chain clusters unsuccessfully. \n\n
///@retval  ABNORMAL_STATUS     Free/release chain clusters unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
int ChainFreeToReserveStartCluster(DRIVE * drv, CHAIN * chain)
{
    int ret;
    DWORD start_clus, clusterSize;


    if ((drv == NULL) || (chain == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    start_clus = chain->Start;
    clusterSize = 1 << (drv->ClusterExp + drv->bSectorExp);

    ret = ChainFree(drv, chain);
    if (ret != FS_SUCCEED)
    {
        MP_ALERT("%s: ChainFree() failed !", __FUNCTION__);
        return ret;
    }
    else
    {
        ret = drv->FatWrite(drv, start_clus, FAT_READ_END_OF_CHAIN);  /* allocate the cluster, and set terminator of this chain */
        if (ret != FS_SUCCEED)
        {
            MP_ALERT("%s: drv->FatWrite() failed !", __FUNCTION__);
            return ret;
        }

        ChainInit(chain, start_clus, clusterSize);
        return FS_SUCCEED;
    }
}



///
///@ingroup CHAIN
///@brief   Copy the whole data of a source chain on the source drive to the new target chain on the target dirve.
///
///@param   target      DRIVE handle of the target drive. \n\n
///@param   trgChain    The target chain on the target drive. \n\n
///@param   source      DRIVE handle of the source drive. \n\n
///@param   srcChain    The source chain on the source drive. \n\n
///@param   bufaddress    The pointer/address of a prepared temporary buffer to be used during the copy process. \n\n
///@param   size          Size of the prepared temporary buffer.
///
///@retval  FS_SUCCEED             Copy successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL      Copy unsuccessfully. \n\n
///@retval  DISK_FULL              Copy unsuccessfully due to no free cluster available on the target drive. \n\n
///@retval  ABNORMAL_STATUS        Copy unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE          Invalid drive.
///
int ChainCopy(DRIVE * target, CHAIN * trgChain, DRIVE * source, CHAIN * srcChain, DWORD bufaddress, DWORD size)
{
	DWORD residueS, residueT, contS, contT, batchS, batchT;
	BYTE *buffer;
	DWORD cnt = 0;


	if ((target == NULL) || (source == NULL))
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if ((trgChain == NULL) || (srcChain == NULL))
	{
		MP_ALERT("%s: Error! NULL chain pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	Set_Copy_SrcTrgDrives_Status(source, target);

	if (target->FreeClusters == 0)  /* maybe FAT table of this drive has not been scanned yet */
		DriveFreeClustersCountGet(target); /* force to do ScanFat() for FAT12/16/32 drive */

  #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
	if (! target->Flag.AllocBitmapReady)
	{
		MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
		Reset_Copy_SrcTrgDrives_Status();
		return ABNORMAL_STATUS;
	}
  #endif

	/* check as soon as possible for possible source or target card out */
	if ( !Check_Copy_SrcTrgDrives_Both_Present() )
	{
		MP_ALERT("%s: Card not present !", __FUNCTION__);
		Reset_Copy_SrcTrgDrives_Status();
		return DRIVE_ACCESS_FAIL;
	}

#if FS_REENTRANT_API
	BOOL f_Is_ext_working_drv;
	DRIVE *ori_drv_in_drive_table = DriveGet(target->DrvIndex);

	/* check whether if this drive handle is an external working drive copy */
	f_Is_ext_working_drv = (target != ori_drv_in_drive_table)? TRUE:FALSE;

	Set_UpdatingFAT_Status(target, (BYTE *) __FUNCTION__);
	if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
		Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

	contT = DriveNewClusGet(target);	// get a new cluster for this new chain
	if (contT == 0xffffffff)
	{
	#if FS_REENTRANT_API
		Reset_UpdatingFAT_Status(target, (BYTE *) __FUNCTION__);
		if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
			Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
	#endif

		Reset_Copy_SrcTrgDrives_Status();
		return DISK_FULL;
	}

	if (target->FatWrite(target, contT, FAT_READ_END_OF_CHAIN) != FS_SUCCEED)	// set chain terminator
	{
		MP_ALERT("%s: target->FatWrite() failed !", __FUNCTION__);
		Reset_Copy_SrcTrgDrives_Status();
		return DRIVE_ACCESS_FAIL;
	}

#if FS_REENTRANT_API
	/* FAT cluster allocation is changed => force to write FAT cache and FSInfo to device right away !
	 * note: this is important to sync FAT and free cluster info for concurrent multiple File I/O API invoking case.
	 */
	if (DriveRefresh(target) != FS_SUCCEED)
	{
		Reset_UpdatingFAT_Status(target, (BYTE *) __FUNCTION__);
		if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
			Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

		Reset_Copy_SrcTrgDrives_Status();
		return DRIVE_ACCESS_FAIL;
	}
#endif

	/* check as soon as possible for possible source or target card out */
	if ( !Check_Copy_SrcTrgDrives_Both_Present() )
	{
		MP_ALERT("%s: Card not present !", __FUNCTION__);
		Reset_Copy_SrcTrgDrives_Status();
		return DRIVE_ACCESS_FAIL;
	}

	ChainInit(trgChain, contT, 0);	// initialize the new chain

	MP_ALERT("%s: => ChainChangeSize() to (%lu) bytes for target chain ...", __FUNCTION__, srcChain->Size);
	MP_DEBUG("%s: start cluster of target chain in ChainChangeSize() = %lu", __FUNCTION__, trgChain->Start);
	/* extend the new chain to expected size on target drive */
	if (ChainChangeSize(target, trgChain, srcChain->Size) != FS_SUCCEED)
	{
	#if FS_REENTRANT_API
		Reset_UpdatingFAT_Status(target, (BYTE *) __FUNCTION__);
		if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
			Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
	#endif

		Reset_Copy_SrcTrgDrives_Status();
		return DISK_FULL;
	}

	// reset each chains point
	ChainSeekSet(srcChain);
	ChainSeekSet(trgChain);

	residueS = (srcChain->Size >> source->bSectorExp);
	if (srcChain->Size & ((1 << source->bSectorExp) - 1))
		residueS++;
	residueT = (srcChain->Size >> target->bSectorExp); // use source chain size to caculate the residueT
	if (srcChain->Size & ((1 << target->bSectorExp) - 1))
		residueT++;

#if USBOTG_HOST_PTP
    if ((source->DevID == DEV_USB_HOST_PTP) || (source->DevID == DEV_USBOTG1_HOST_PTP))
		residueS = srcChain->Size;
#endif //  USBHOST_PTP

	batchS = batchT = size;
#if 0   //Lighter 3/13: speed up copy time
	if (batchS > 0x10000)
		batchS = batchT = 0x10000;    			// if buffer size greater than 64KB than limit it to 2M
#endif

	buffer = (BYTE *) bufaddress;
	batchS >>= source->bSectorExp;
	batchT >>= target->bSectorExp;

#if USBOTG_HOST_PTP
    if ((source->DevID == DEV_USB_HOST_PTP) || (source->DevID == DEV_USBOTG1_HOST_PTP))
		batchS = size;
#endif //  USBHOST_PTP

	while (residueS)
	{
		if (batchS > residueS)
			contS = residueS;
		else
			contS = batchS;

		if (batchT > residueT)
			contT = residueT;
		else
			contT = batchT;

		/* check as soon as possible for possible source or target card out */
		if ( !Check_Copy_SrcTrgDrives_Both_Present() )
		{
			MP_ALERT("%s: Card not present !", __FUNCTION__);
			Reset_Copy_SrcTrgDrives_Status();
			return DRIVE_ACCESS_FAIL;
		}

		if (ChainRead(source, srcChain, buffer, contS) != FS_SUCCEED)
			break;

		/* check as soon as possible for possible source or target card out */
		if ( !Check_Copy_SrcTrgDrives_Both_Present() )
		{
			MP_ALERT("%s: Card not present !", __FUNCTION__);
			Reset_Copy_SrcTrgDrives_Status();
			return DRIVE_ACCESS_FAIL;
		}

		if (ChainWrite(target, trgChain, buffer, contT) != FS_SUCCEED)
			break;

		residueS -= contS;
		residueT -= contT;

		cnt += contS;
		if (cnt >= 128)
		{
			cnt = 0;

			if (fileUiUpdateIconAniPtr)
				fileUiUpdateIconAniPtr();
		}
	}

#if FS_REENTRANT_API
	Reset_UpdatingFAT_Status(target, (BYTE *) __FUNCTION__);
	if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
		Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

	Reset_Copy_SrcTrgDrives_Status();

	if (residueS)
		return DRIVE_ACCESS_FAIL;
	else
		return FS_SUCCEED;
}



///
///@ingroup CHAIN
///@brief   Traverse the FAT table entries of a cluster chain to calculate the total size of the chain.
///
///@param   drv            The drive to access the chain. \n\n
///
///@param   start_clus     The start cluster of the chain to calculate its total length. \n\n
///
///@return   The total size (in bytes) of the chain. If error occurred, return 0.
///
///@remark   This function always works well for FAT12/16/32 drives, because FAT12/16/32 file systems track all the clusters of  \n
///          a file or a directory in the FAT table. \n
///          For exFAT file system, all the clusters of a file or a directory may be continuous and therefore exFAT may not track the  \n
///          clusters in FAT table at all. For such kind exFAT cases, only the 'start_clus' parameter info is not enough to judge whether if  \n
///          the file or directory is fragmented or not. \n\n
///
///@remark   For exFAT drives, you must check the exFAT 'NoFatChain' flag of the file/directory and make sure the file/directory's clusters are  \n
///          tracked in FAT table (i.e. 'NoFatChain' flag == 0). Then you can calling this function to calculate total size of the chain.
///
DWORD GetChainTotalSizeByTraverseFatTable(DRIVE * drv, DWORD start_clus)
{
	DWORD total_size = 0;
	DWORD clusterSize, current, next;


	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return 0;
	}

#if EXFAT_ENABLE
	if ((drv->Flag.FsType == FS_TYPE_FAT32) || (drv->Flag.FsType == FS_TYPE_exFAT)) /* FAT32 or exFAT */
#else
	if (drv->Flag.FsType == FS_TYPE_FAT32) /* FAT32 */
#endif
	{
		if ((start_clus < 2) || (start_clus >= (drv->TotalClusters + 2)))
		{
			MP_ALERT("%s: Error! Invalid cluster number !", __FUNCTION__);
			return 0;
		}
	}
	else if ((drv->Flag.FsType == FS_TYPE_FAT12) || (drv->Flag.FsType == FS_TYPE_FAT16)) /* FAT12 or FAT16 */
	{
		if (drv->DirStackPoint != 0) /* not in RootDir */
		{
			if ((start_clus < 2) || (start_clus >= (drv->TotalClusters + 2)))
			{
				MP_ALERT("%s: Error! Invalid cluster number !", __FUNCTION__);
				return 0;
			}
		}
		else /* in RootDir */
		{
			/* note: for FAT12/16, drv->RootStart is lba of first sector of root directory */
			total_size = (drv->DataStart - drv->RootStart) << drv->bSectorExp;
			return total_size;
		}
	}

	clusterSize = 1 << (drv->ClusterExp + drv->bSectorExp);
	current = start_clus;

	if (drv->Flag.FsType != FS_TYPE_exFAT)
	{
		while ((current >= 2) && (current < (drv->TotalClusters + 2)))
		{
			if (! SystemCardPresentCheck(drv->DrvIndex))
			{
				MP_ALERT("%s: Card not present !", __FUNCTION__);
				return 0;
			}

			total_size += clusterSize;
			next = drv->FatRead(drv, current);

			/* avoid potential endless loop, which may be caused by unplugging the card */
			if (drv->StatusCode == FS_SCAN_FAIL)
			{
				MP_ALERT("%s: failed to read FAT table entry ! => return 0;", __FUNCTION__);
				return 0;
			}

			if (next == FAT_READ_END_OF_CHAIN) /* reach EOC (end of chain) */
				break;
	
			current = next;
		}
	}
#if EXFAT_ENABLE
	else
	{
		if (drv->exFAT_InfoFileds->AllocBitmapSize[drv->exFAT_InfoFileds->VolumeFlags.ActiveFat])
		{
			BYTE bitEntry_byte = Read_AllocBitmapEntry(drv, start_clus);
			if (bitEntry_byte == 0xFF)
			{
				MP_ALERT("%s: Error! Read_AllocBitmapEntry() failed !", __FUNCTION__);
				return 0;
			}
			else if (bitEntry_byte == 0)
			{
				MP_ALERT("%s: Error! The chain cluster is actually not allocated !", __FUNCTION__);
				return 0;
			}
		}

		if (start_clus == drv->RootStart) /* chain is the RootDir */
		{
			while ((current >= 2) && (current < (drv->TotalClusters + 2)))
			{
				if (! SystemCardPresentCheck(drv->DrvIndex))
				{
					MP_ALERT("%s: Card not present !", __FUNCTION__);
					return 0;
				}

				total_size += clusterSize;
				next = drv->FatRead(drv, current);

				/* avoid potential endless loop, which may be caused by unplugging the card */
				if (drv->StatusCode == FS_SCAN_FAIL)
				{
					MP_ALERT("%s: failed to read FAT table entry ! => return 0;", __FUNCTION__);
					return 0;
				}

				if (next == FAT_READ_END_OF_CHAIN) /* reach EOC (end of chain) */
					break;

				current = next;
			}
		}
		else /* not in RootDir */
		{
			/* this function only has a 'start_clus' parameter and cannot judge whether if the file or directory is fragmented or not.
			 * how to check the exFAT NoFatChain flag for this directory ??
			 */

			if (! SystemCardPresentCheck(drv->DrvIndex))
			{
				MP_ALERT("%s: Card not present !", __FUNCTION__);
				return 0;
			}

			if (drv->FatRead(drv, start_clus) == 0) /* chain is not fragmented */
			{
				MP_DEBUG("%s: To-Do: how to get chain size if the chain is not fragmented and only start_clus is known  ??", __FUNCTION__);
				MP_DEBUG("%s:  => The chain size can be gotten only via its Stream Extension DirectoryEntry !", __FUNCTION__);
				return 0;
			}
			else
			{
				if (drv->StatusCode == FS_SCAN_FAIL)
				{
					MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
					return 0;
				}

				while ((current >= 2) && (current < (drv->TotalClusters + 2)))
				{
					if (! SystemCardPresentCheck(drv->DrvIndex))
					{
						MP_ALERT("%s: Card not present !", __FUNCTION__);
						return 0;
					}

					total_size += clusterSize;
					next = drv->FatRead(drv, current);

					/* avoid potential endless loop, which may be caused by unplugging the card */
					if (drv->StatusCode == FS_SCAN_FAIL)
					{
						MP_ALERT("%s: failed to read FAT table entry ! => return 0;", __FUNCTION__);
						return 0;
					}

					if (next == FAT_READ_END_OF_CHAIN) /* reach EOC (end of chain) */
						break;
			
					current = next;
				}
			}
		}
	}
#endif

	return total_size;
}



static BOOL Check_Copy_SrcTrgDrives_Both_Present(void)
{
	if (f_ChainCopy_Inprogress && (ChainCopy_SrcDrv != NULL) && (ChainCopy_TrgDrv != NULL))
	{
		if ( !SystemCardPresentCheck(ChainCopy_SrcDrv->DrvIndex) || !SystemCardPresentCheck(ChainCopy_TrgDrv->DrvIndex) )
			return FALSE;
		else
			return TRUE;
	}
	else
		return FALSE;
}



static void Set_Copy_SrcTrgDrives_Status(DRIVE * src_drv, DRIVE * trg_drv)
{
	if ((src_drv != NULL) && (trg_drv != NULL))
	{
		f_ChainCopy_Inprogress = TRUE;
		ChainCopy_SrcDrv = src_drv;
		ChainCopy_TrgDrv = trg_drv;
	}
}



static void Reset_Copy_SrcTrgDrives_Status(void)
{
	f_ChainCopy_Inprogress = FALSE;
	ChainCopy_SrcDrv = ChainCopy_TrgDrv = NULL;
}




