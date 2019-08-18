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
* Filename      : ffile.c
* Programmers   : 
* Created       : 
* Descriptions  : 
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0


//Don't fix this. And don't define DOXYGEN_SHOW_INTERNAL_USAGE_API manually in any C source file !!
//For Doxygen preprocessor processing for including our internal usage APIs in this module
#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
  #define ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING   1 
#endif

/*
// Include section 
*/

#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"


#if ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING 
/*
// Variable declarations
*/
static volatile DWORD *g_FileChainClusterCache = NULL, *g_FileChainClusterCache_Ori = NULL;
static volatile DWORD g_FileChainClusterCache_tail_idx;
static BOOL g_FileChainClusterCaching_ok = TRUE;


/*
// Static function prototype
*/
static SWORD LoadFileFatClusters(STREAM * handle);
static SWORD FChainFragmentRead(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD size);
static SWORD FChainSeekForward(DRIVE * drv, CHAIN * chain, DWORD distance);
static SWORD FChainRead(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD sector);
static DWORD FChainGetContinuity(DRIVE * drv, CHAIN * chain, DWORD * request);
static SWORD FChainSeek(DRIVE * drv, CHAIN * chain, DWORD position);



///
///@ingroup For_VideoPlayback
///@brief   Read data from the specified (video) file for playback.
///
///@param   handle      The handle of file to access. \n\n
///@param   buffer      The pointer of buffer for data read. \n\n
///@param   size        Byte size of data.
///
///@return  The number of bytes actually read. If an error occurred or reach the end of the file,\n
///         it may return 0 or the actual count value which is less than the input 'size'.
///
///@remark  The function call will read data from the byte offset saved in chain->Point.\n
///         Note that this function is only for video file playback usage!
///
DWORD Fstream_FileRead(STREAM * handle, BYTE * buffer, DWORD size)
{
    DWORD head, body, tail;
    DWORD dwByteCount = 0;
    DRIVE *drv;
    BYTE *tmpbuffer;


    if (handle == NULL)
        return 0;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        MP_DEBUG("%s: this operation is not supported for \"RamFile\" type files !", __FUNCTION__);
        return 0;
    }
#endif

    /* special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (! handle->Flag.f_IsRamFile)
    {
        if (handle->Drv == NULL)
            return 0; // this file has been closed
    }

    if (!handle->Drv->Flag.Present)
        return 0;

//note: this semaphore seems may block the audio or video task a period of time while video file playback !
    //SemaphoreWait(FILE_READ_SEMA_ID);

#if USBOTG_HOST_PTP
    if ((handle->Drv->DevID == DEV_USB_HOST_PTP) || (handle->Drv->DevID == DEV_USBOTG1_HOST_PTP)) //(handle->Drv->DevID == DEV_USB_HOST_PTP)
    {
        if ((handle->Chain.Point + size) > handle->Chain.Size)
            size = handle->Chain.Size - handle->Chain.Point;

        handle->Drv->Node = (FDB*)handle->DirSector;
        drv = (DRIVE *)handle->Drv;
        if (DriveRead(drv, buffer, handle->Chain.Point, size) != FS_SUCCEED)
        {
            //SemaphoreRelease(FILE_READ_SEMA_ID);
            return 0;
        }

        handle->Chain.Point += size;
        //SemaphoreRelease(FILE_READ_SEMA_ID);
        return size;
    }
#endif

    if (handle->Chain.Point >= handle->Chain.Size) //reach EOF
    {
        MP_DEBUG("%s: reach EOF !", __FUNCTION__);
        goto FFileReadError;
    }

    tmpbuffer = buffer;
    dwByteCount = 0;
    if ((handle->Chain.Point + size) > handle->Chain.Size)
        size = handle->Chain.Size - handle->Chain.Point;

    /* devide the read range into 3 segments */
    Segmentize(handle, &head, &body, &tail, size);

    drv = (DRIVE *) handle->Drv;

    if (head)
    {
#if ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING 
        if (g_FileChainClusterCaching_ok)
        {
            if (FChainFragmentRead(drv, (CHAIN *) (&handle->Chain), tmpbuffer, head) != FS_SUCCEED)
                goto FFileReadError;
        }
        else
        {
            if (ChainFragmentRead(drv, (CHAIN *) (&handle->Chain), tmpbuffer, head) != FS_SUCCEED)
                goto FFileReadError;
        }
#else
        if (ChainFragmentRead(drv, (CHAIN *) (&handle->Chain), tmpbuffer, head) != FS_SUCCEED)
            goto FFileReadError;
#endif

        tmpbuffer += head;
        dwByteCount += head;
    }

    if (body)
    {
        //note: here, non-aligned buffer address issue is processed in DriveRead()/DriveWrite()
#if ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING 
        if (g_FileChainClusterCaching_ok)
        {
            if (FChainRead(drv, (CHAIN *) (&handle->Chain), tmpbuffer, body >> drv->bSectorExp) != FS_SUCCEED)
                goto FFileReadError;
        }
        else
        {
            if (ChainRead(drv, (CHAIN *) (&handle->Chain), tmpbuffer, body >> drv->bSectorExp) != FS_SUCCEED)
                goto FFileReadError;
        }
#else
        if (ChainRead(drv, (CHAIN *) (&handle->Chain), tmpbuffer, body >> drv->bSectorExp) != FS_SUCCEED)
            goto FFileReadError;
#endif

        tmpbuffer += body;
        dwByteCount += body;
    }

    if (tail)
    {
#if ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING 
        if (g_FileChainClusterCaching_ok)
        {
            if (FChainFragmentRead(drv, (CHAIN *) (&handle->Chain), tmpbuffer, tail) != FS_SUCCEED)
                goto FFileReadError;
        }
        else
        {
            if (ChainFragmentRead(drv, (CHAIN *) (&handle->Chain), tmpbuffer, tail) != FS_SUCCEED)
                goto FFileReadError;
        }
#else
        if (ChainFragmentRead(drv, (CHAIN *) (&handle->Chain), tmpbuffer, tail) != FS_SUCCEED)
            goto FFileReadError;
#endif

        dwByteCount += tail;
    }

FFileReadError:	
    //SemaphoreRelease(FILE_READ_SEMA_ID);
    return dwByteCount;
}



///
///@ingroup For_VideoPlayback
///@brief   Move the file byte offset to spcific position.
///
///@param   handle      The handle of file to seek. \n\n
///@param   position    The byte offset to move.
///
///@retval  FS_SUCCEED          Seek successfully. \n\n
///@retval  OUT_OF_RANGE        The position to seek exceeds file size. \n\n
///@retval  FILE_NOT_FOUND      File not found.
///
///@remark  Note that this function is only for video file playback usage!
///
SWORD Fstream_Seek(STREAM * handle, DWORD position)
{
    if (handle == NULL)
        return FILE_NOT_FOUND;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        MP_DEBUG("%s: this operation is not supported for \"RamFile\" type files !", __FUNCTION__);
        return FILE_NOT_FOUND;
    }
#endif

    /* special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (! handle->Flag.f_IsRamFile)
    {
        if (handle->Drv == NULL)
            return FILE_NOT_FOUND; // this file has been closed
    }

    /* Here, seek function allows (position == FileSizeGet(handle)).
     * In such case, Fstream_FileRead() function will return 0 because EOF reached. Valid data range is 0 ~ (FileSizeGet(handle) - 1)
     */
    if (position > FileSizeGet(handle))
    {
        MP_ALERT("%s: Error! file position (= %lu) to seek exceeds file content range ! (file size = %lu)", __FUNCTION__, position, FileSizeGet(handle));
        return OUT_OF_RANGE;
    }

#if USBOTG_HOST_PTP
    if ((handle->Drv->DevID == DEV_USB_HOST_PTP) || (handle->Drv->DevID == DEV_USBOTG1_HOST_PTP)) //(handle->Drv->DevID == DEV_USB_HOST_PTP)
    {
        handle->Chain.Point = position;
        return FS_SUCCEED;
    }
#endif

#if ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING 
    if (g_FileChainClusterCaching_ok)
        return FChainSeek((DRIVE *) (handle->Drv), (CHAIN *) (&handle->Chain), position);
    else
        return ChainSeek((DRIVE *) (handle->Drv), (CHAIN *) (&handle->Chain), position);
#else
    return ChainSeek((DRIVE *) (handle->Drv), (CHAIN *) (&handle->Chain), position);
#endif
}



///
///@ingroup For_VideoPlayback
///@brief   Initialize/reset content of the preload cache table of video file chain clusters.
///
///@param   handle      The handle of video file to playback.
///
///@retval  PASS      Caching video file chain clusters successfully. \n\n
///@retval  FAIL      Caching video file chain clusters unsuccessfully.
///
///@remark  Note that this function is only for video file playback usage!
///
SWORD InitPreload_FileChainClustersCache(STREAM * handle)
{
    DWORD file_size;
    DWORD file_clusters_count;


    g_FileChainClusterCaching_ok = FALSE;

    if (handle == NULL)
        return FAIL;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        MP_DEBUG("%s: this operation is not supported for \"RamFile\" type files !", __FUNCTION__);
        return FAIL;
    }
#endif

    /* special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (! handle->Flag.f_IsRamFile)
    {
        if (handle->Drv == NULL)
            return FAIL; // this file has been closed
    }

    if (handle->Drv->Flag.FsType != FS_TYPE_exFAT)
    {
        file_size = FileSizeGet(handle);

        if (file_size % (1 << (handle->Drv->ClusterExp + handle->Drv->bSectorExp)))
            file_clusters_count = (file_size >> (handle->Drv->ClusterExp + handle->Drv->bSectorExp)) + 1;
        else
            file_clusters_count = (file_size >> (handle->Drv->ClusterExp + handle->Drv->bSectorExp));
    }
#if EXFAT_ENABLE
    else
    {
//To-Do: 64-bit file size ??
        file_size = FileSizeGet(handle);

        if (file_size % (1 << (handle->Drv->ClusterExp + handle->Drv->bSectorExp)))
            file_clusters_count = (file_size >> (handle->Drv->ClusterExp + handle->Drv->bSectorExp)) + 1;
        else
            file_clusters_count = (file_size >> (handle->Drv->ClusterExp + handle->Drv->bSectorExp));
    }
#endif

#if 0
    /* if needed memory size for video chain clusters caching > 1 MB, then we don't use caching mechanism and don't allocate memory at all */
    if ((sizeof(DWORD) * (file_clusters_count + 1)) > (1 << 20))
    {
        MP_ALERT("%s: Needed memory size for chain clusters caching > 1 MB => we will not use caching mechanism ...\r\n", __FUNCTION__);
        return FAIL;
    }
#endif

    MP_DEBUG("%s: need to malloc (%lu) bytes memory for video file clusters caching...", __FUNCTION__, sizeof(DWORD) * (file_clusters_count + 1));
    g_FileChainClusterCache_Ori = (DWORD *) ext_mem_malloc(sizeof(DWORD) * (file_clusters_count + 1)); //one more entry for tail ending
    if (g_FileChainClusterCache_Ori == NULL)
    {
        MP_ALERT("%s: malloc fail !! No enough memory for clusters caching !!\r\n", __FUNCTION__);
        return FAIL;
    }
    g_FileChainClusterCache = (DWORD *) ((DWORD)g_FileChainClusterCache_Ori | 0xA0000000); //make to be non-cacheable
    MpMemSet((BYTE *) g_FileChainClusterCache, 0, sizeof(DWORD) * (file_clusters_count + 1));

    g_FileChainClusterCache[0] = handle->Chain.Start;
    MP_DEBUG("%s: initialize g_FileChainClusterCache[0] to 0x%x", __FUNCTION__, g_FileChainClusterCache[0]);
    g_FileChainClusterCache_tail_idx = 1;
    if (LoadFileFatClusters(handle) == PASS)
    {
        g_FileChainClusterCaching_ok = TRUE;
        MP_DEBUG("%s: caching OK.  ext_mem_get_free_space() = %lu", __FUNCTION__, ext_mem_get_free_space());
        return PASS;
    }
    else
    {
        g_FileChainClusterCaching_ok = FALSE;
        Clear_FileChainClustersCache();
        MP_ALERT("%s: caching FAIL.  ext_mem_get_free_space() = %lu", __FUNCTION__, ext_mem_get_free_space());
        return FAIL;
    }
}



///
///@ingroup For_VideoPlayback
///@brief   Clear the whole cache of video file chain clusters, and release its allocated memory.
///
///@param   None.
///
///@return  None.
///
void Clear_FileChainClustersCache(void)
{
    if (g_FileChainClusterCache_Ori != NULL)
    {
        MP_DEBUG("%s: release memory of video file clusters caching...", __FUNCTION__);
        ext_mem_free(g_FileChainClusterCache_Ori);
    }

    g_FileChainClusterCache = g_FileChainClusterCache_Ori = NULL;
    g_FileChainClusterCache_tail_idx = 0;
    g_FileChainClusterCaching_ok = FALSE;
    return;
}



static SWORD LoadFileFatClusters(STREAM * handle)
{
    if (handle == NULL)
        return FAIL;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        MP_DEBUG("%s: this operation is not supported for \"RamFile\" type files !", __FUNCTION__);
        return FAIL;
    }
#endif

    /* special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (! handle->Flag.f_IsRamFile)
    {
        if (handle->Drv == NULL)
            return FAIL; // this file has been closed
    }

    DWORD clus_size = (1 << (handle->Drv->ClusterExp + handle->Drv->bSectorExp));
    DWORD file_size = FileSizeGet(handle);

    handle->Chain.Current = handle->Chain.Start;
    if (handle->Drv->Flag.FsType != FS_TYPE_exFAT)
    {
        DWORD temp_size = 0;
        while ((handle->Chain.Current != FAT_READ_END_OF_CHAIN) && (handle->Chain.Current >= 2) && (temp_size < file_size))
        {
            if (! SystemCardPresentCheck(handle->Drv->DrvIndex))
            {
                MP_ALERT("%s: Card not present !", __FUNCTION__);
                return FAIL;
            }

            handle->Chain.Current = handle->Drv->FatRead((DRIVE *) handle->Drv, handle->Chain.Current);

            if (handle->Drv->StatusCode == FS_SCAN_FAIL)
            {
                MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                return FAIL;
            }

            g_FileChainClusterCache[g_FileChainClusterCache_tail_idx++] = handle->Chain.Current;
            temp_size += clus_size;

            //just for debugging info
            if ((temp_size >= file_size) && (handle->Chain.Current != FAT_READ_END_OF_CHAIN))
            {
                MP_ALERT("%s: -I- FAT cluster chain not ended, but already reach whole file size (= %lu), stop loop.", __FUNCTION__, file_size);
            }
        }
    }
#if EXFAT_ENABLE
    else
    {
        BOOL f_chain_fragmented;
        DWORD temp_size = 0; //To-Do: 64-bit for exFAT ??

        if (handle->Chain.exfat_DirEntryFlags.f_NoFatChain) /* chain is not fragmented */
            f_chain_fragmented = FALSE;
        else
        {
            if (! SystemCardPresentCheck(handle->Drv->DrvIndex))
            {
                MP_ALERT("%s: Card not present !", __FUNCTION__);
                return FAIL;
            }

            if (handle->Drv->FatRead((DRIVE *) handle->Drv, handle->Chain.Start) == 0) /* chain is not fragmented */
                f_chain_fragmented = FALSE;
            else
            {
                if (handle->Drv->StatusCode == FS_SCAN_FAIL)
                {
                    MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                    return FAIL;
                }

                f_chain_fragmented = TRUE;
            }
        }

        if (!f_chain_fragmented) /* chain is not fragmented */
        {
            DWORD clus_offset = 1; /* next cluster w.r.t handle->Chain.Start */
            while (temp_size < file_size)
            {
                g_FileChainClusterCache[g_FileChainClusterCache_tail_idx++] = handle->Chain.Start + clus_offset;
                handle->Chain.Current = handle->Chain.Start + clus_offset;
                temp_size += clus_size;
                clus_offset++;
            }
        }
        else
        {
            while ((handle->Chain.Current != FAT_READ_END_OF_CHAIN) && (handle->Chain.Current >= 2) && (temp_size < file_size))
            {
                if (! SystemCardPresentCheck(handle->Drv->DrvIndex))
                {
                    MP_ALERT("%s: Card not present !", __FUNCTION__);
                    return FAIL;
                }

                handle->Chain.Current = handle->Drv->FatRead((DRIVE *) handle->Drv, handle->Chain.Current);

                if (handle->Drv->StatusCode == FS_SCAN_FAIL)
                {
                    MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                    return FAIL;
                }

                g_FileChainClusterCache[g_FileChainClusterCache_tail_idx++] = handle->Chain.Current;
                temp_size += clus_size;

                //just for debugging info
                if ((temp_size >= file_size) && (handle->Chain.Current != FAT_READ_END_OF_CHAIN))
                {
                    MP_ALERT("%s: -I- FAT cluster chain not ended, but already reach whole file size (= %lu), stop loop.", __FUNCTION__, file_size);
                }
            }
        }
    }
#endif

    return PASS;
}



//note: Actually, 'static' function will not be included in the generated Doxygen document.
#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup For_VideoPlayback
///@brief   Read data fragment which is less than size of a sector from chain. \n
///         This function is variant of the ChainFragmentRead() in CHAIN module and only for video playback!
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
///@remark   Value of 'size' parameter must satisfy the condition that \n
///          (size + (chain->Point & ((1 << drv->bSectorExp) - 1)) <= (1 << drv->bSectorExp).\n
///          Note that this function is only for video file playback usage!
///
#endif
static SWORD FChainFragmentRead(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD size)
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
        MP_DEBUG("-E FChainFragmentRead malloc fail");
        return DRIVE_ACCESS_FAIL;
    }	

    lba = ChainGetLba(drv, chain);
    if (lba == 0xFFFFFFFF)
    {
        MP_ALERT("%s: -I-  EOC reached and at cluster boundary => out of chain range.", __FUNCTION__);
        ker_mem_free(fragment);
        return ABNORMAL_STATUS;
    }

    if (DriveRead(drv, fragment, lba, 1) != FS_SUCCEED)
    {
        MP_ALERT("FChainFragmentRead(): DriveRead() failed, return DRIVE_ACCESS_FAIL;");
        ker_mem_free(fragment);
        return DRIVE_ACCESS_FAIL;
    }
    MpMemCopy(buffer, fragment + (chain->Point & ((1 << drv->bSectorExp)-1)), size);

    ker_mem_free(fragment);

    FChainSeekForward(drv, chain, size);

    return FS_SUCCEED;
}



//note: Actually, 'static' function will not be included in the generated Doxygen document.
#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup For_VideoPlayback
///@brief   Seek forward from current chain point to the specified point in the chain. \n
///         This function is variant of the ChainSeekForward() in CHAIN module and only for video playback!
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
///@remark  Note that this function is only for video file playback usage!
///
#endif
static SWORD FChainSeekForward(DRIVE * drv, CHAIN * chain, DWORD distance)
{
    DWORD john, mary;
    SWORD retVal;
    BOOL  f_chain_fragmented;
    DWORD ori_point;


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

    retVal = FS_SUCCEED;
    ori_point = chain->Point;

    john = chain->Point >> (drv->ClusterExp + drv->bSectorExp);
    chain->Point += distance;

    if (chain->Point > chain->Size) /* note: here NOT (chain->Point >= chain->Size) */
    {
    #if 0 //original code
        retVal = OUT_OF_RANGE;
        chain->Point = chain->Size - 1;
    #else //return fail directly
        chain->Point = ori_point; //recover chain->Point value
        MP_ALERT("%s: destination out of range of chain size ! (chain->Size = %lu, ori point = %lu, distance = %lu, dest Point = %lu)", __FUNCTION__, chain->Size, ori_point, distance, chain->Point);
        return OUT_OF_RANGE;
    #endif
    }

    mary = chain->Point >> (drv->ClusterExp + drv->bSectorExp);

    // if at root of FAT12/FAT16
    if (!chain->Start)
        return retVal;

    if (drv->Flag.FsType != FS_TYPE_exFAT)
        f_chain_fragmented = TRUE;
#if EXFAT_ENABLE
    else
    {
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
    }
#endif

    if (!f_chain_fragmented) /* chain is not fragmented */
        chain->Current = g_FileChainClusterCache[mary];
    else
    {
        MP_DEBUG("%s: while loop: john = %lu, mary = %lu", __FUNCTION__, john, mary);
        while (john < mary)  // move to the specified cluster position
        {
            if (g_FileChainClusterCache != NULL)
            {
                if (g_FileChainClusterCache[mary] != 0) //the destination cluster already known
                {
                    chain->Current = g_FileChainClusterCache[mary];
                    break;
                }

                int i;
                for (i = john; (i < g_FileChainClusterCache_tail_idx) && (g_FileChainClusterCache[i] != FAT_READ_END_OF_CHAIN); i++)
                {
                    if (chain->Current == g_FileChainClusterCache[i]) //found matched cluster entry in the cache
                    {
                        if (g_FileChainClusterCache[i+1] != 0) //next cluster already known
                            chain->Current = g_FileChainClusterCache[i+1];
                        else //next cluster still unknown
                        {
                            if (! SystemCardPresentCheck(drv->DrvIndex))
                            {
                                MP_ALERT("%s: Card not present !", __FUNCTION__);
                                return ABNORMAL_STATUS;
                            }

                            chain->Current = drv->FatRead(drv, chain->Current);

                            if (drv->StatusCode == FS_SCAN_FAIL)
                            {
                                MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                                return ABNORMAL_STATUS;
                            }

                            g_FileChainClusterCache[i+1] = chain->Current;
                            g_FileChainClusterCache_tail_idx++;
                        }
                        break;
                    }
                }

                if (g_FileChainClusterCache[i] == FAT_READ_END_OF_CHAIN)
                {
                    retVal = OUT_OF_RANGE; //ABNORMAL_STATUS
                    break;
                }
                else if (i >= g_FileChainClusterCache_tail_idx) //chain->Current cluster not found in Chain Cluster Cache => should be impossible
                {
                    MP_DEBUG("%s: (i >= g_FileChainClusterCache_tail_idx=%lu), should be impossible !!", __FUNCTION__, g_FileChainClusterCache_tail_idx);
                    retVal = ABNORMAL_STATUS;
                    break;
                }
            }
            else
            {
                if (! SystemCardPresentCheck(drv->DrvIndex))
                {
                    MP_ALERT("%s: Card not present !", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }

                chain->Current = drv->FatRead(drv, chain->Current);

                if (drv->StatusCode == FS_SCAN_FAIL)
                {
                    MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }
            }

            john++;
        }
    }

    return retVal;
}



//note: Actually, 'static' function will not be included in the generated Doxygen document.
#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup For_VideoPlayback
///@brief   Read data from chain. \n
///         This function is variant of the ChainRead() in CHAIN module and only for video playback!
///
///@param   drv         The drive to access. \n\n
///@param   chain       The chain to read data. \n\n
///@param   buffer      The pointer of buffer for data read. \n\n
///@param   sector      Length of data to read (number of sectors for Mcards).
///
///@retval  FS_SUCCEED          Read successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL   Read unsuccessfully. \n\n
///@retval  ABNORMAL_STATUS     Read unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
///@remark  Note that this function is only for video file playback usage!
///
#endif
static SWORD FChainRead(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD sector)
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
    while (sectorCnt)
    {
        tmp_cnt = sectorCnt;

        lba = ChainGetLba(drv, chain);
        if (lba == 0xFFFFFFFF)
        {
            MP_ALERT("%s: -I-  EOC reached and at cluster boundary => out of chain range.", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        count = FChainGetContinuity(drv, chain, &sectorCnt);
        if (!count)
        {
            MP_ALERT("%s: FChainGetContinuity() failed, return DRIVE_ACCESS_FAIL;", __FUNCTION__);
            return DRIVE_ACCESS_FAIL;
        }

        if (DriveRead(drv, buffer, lba, count) != FS_SUCCEED)
        {
            MP_ALERT("%s: DriveRead() failed, return DRIVE_ACCESS_FAIL;", __FUNCTION__);
            return DRIVE_ACCESS_FAIL;
        }

        buffer += count << drv->bSectorExp; // increase the destination buffer point
        sectorCnt = tmp_cnt - count;

        TaskYield();
    }

    return FS_SUCCEED;
}



//note: Actually, 'static' function will not be included in the generated Doxygen document.
#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup For_VideoPlayback
///@brief   Get continuous sector blocks from current chain point with size not greater than the request sector count. \n
///         This function is variant of the ChainGetContinuity() in CHAIN module and only for video playback!
///
///@param   drv        The drive to access. \n\n
///@param   chain      The chain to access. \n\n
///@param   request    [IN/OUT] The requested sector count. And then it will be updated to (request - result).
///
///@return   The number of continuous sectors.
///
///@remark  After returning, the chain point will move to the point right after this continuous blocks area.\n
///         And this function will update the value of 'request' parameter to (request - result). \n
///         Note that this function is only for video file playback usage!
///
#endif
static DWORD FChainGetContinuity(DRIVE * drv, CHAIN * chain, DWORD * request)
{
    DWORD addend, result, previous, current;
    BOOL f_chain_fragmented;


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

    result = 0;

    if (drv->Flag.FsType != FS_TYPE_exFAT)
        f_chain_fragmented = TRUE;
#if EXFAT_ENABLE
    else
    {
        if (chain->exfat_DirEntryFlags.f_NoFatChain) /* chain is not fragmented */
            f_chain_fragmented = FALSE;
        else
        {
            if (! SystemCardPresentCheck(drv->DrvIndex))
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
    }
#endif

    // get the sector offset of the current point within the current cluster
    addend = (chain->Point >> drv->bSectorExp);
    addend = (addend & ((1 << drv->ClusterExp) - 1));
    addend = drv->ClusterSize - addend;

    int search_start_idx;
    search_start_idx = chain->Point >> (drv->ClusterExp + drv->bSectorExp);

    if (!f_chain_fragmented) /* chain is not fragmented */
    {
        do
        {
            previous = chain->Current;
            if (g_FileChainClusterCache != NULL)
            {
                int i;
                for (i = search_start_idx; (i < g_FileChainClusterCache_tail_idx) && (g_FileChainClusterCache[i] != FAT_READ_END_OF_CHAIN); i++)
                {
                    if (chain->Current == g_FileChainClusterCache[i]) //found matched cluster entry in the cache
                    {
                        current = g_FileChainClusterCache[i+1];	
                        search_start_idx = (i + 1);
                        break;
                    }
                }
            }
            else
            {
                MP_ALERT("%s: (g_FileChainClusterCache == NULL),  This should be impossible !!", __FUNCTION__);

                //how to handle such case if really reach here ??? 
                break;
            }

            if (*request >= addend)
            {
                result += addend;
                chain->Point += addend << drv->bSectorExp;
                *request -= addend;
            }
            else
            {
                result += *request;
                chain->Point += *request << drv->bSectorExp;
                *request = 0;
                break;
            }

            addend = drv->ClusterSize;
            chain->Current = current;
        } while (chain->Current == previous + 1);
    }
    else /* chain is fragmented */
    {
        do
        {
            previous = chain->Current;
            if (g_FileChainClusterCache != NULL)
            {
                int i;
                for (i = search_start_idx; (i < g_FileChainClusterCache_tail_idx) && (g_FileChainClusterCache[i] != FAT_READ_END_OF_CHAIN); i++)
                {
                    if (chain->Current == g_FileChainClusterCache[i]) //found matched cluster entry in the cache
                    {
                        if (g_FileChainClusterCache[i+1] != 0) //next cluster already known
                            current = g_FileChainClusterCache[i+1];
                        else //next cluster still unknown
                        {
                            if (! SystemCardPresentCheck(drv->DrvIndex))
                            {
                                MP_ALERT("%s: Card not present !", __FUNCTION__);
                                return 0;
                            }

                            current = drv->FatRead(drv, chain->Current);

                            if (drv->StatusCode == FS_SCAN_FAIL)
                            {
                                MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                                return 0;
                            }

                            g_FileChainClusterCache[i+1] = current;
                            g_FileChainClusterCache_tail_idx++;					
                        }

                        search_start_idx = (i + 1);
                        break;
                    }
                }

                if (g_FileChainClusterCache[i] == FAT_READ_END_OF_CHAIN) //already reach end of chain
                {
                    current = FAT_READ_END_OF_CHAIN;
                    break;
                }
                else if (i >= g_FileChainClusterCache_tail_idx) //chain->Current cluster not found in Chain Cluster Cache => should be impossible
                {
                    MP_DEBUG("%s: (i >= g_FileChainClusterCache_tail_idx=%lu), should be impossible !!", __FUNCTION__, g_FileChainClusterCache_tail_idx);
                    //normally, should be impossible to reach here

                    //how to handle such case if really reach here ??? 
                    break;
                }
            }
            else
            {
                if (! SystemCardPresentCheck(drv->DrvIndex))
                {
                    MP_ALERT("%s: Card not present !", __FUNCTION__);
                    return 0;
                }

                current = drv->FatRead(drv, chain->Current);

                if (drv->StatusCode == FS_SCAN_FAIL)
                {
                    MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                    return 0;
                }
            }

            if (current == FAT_READ_END_OF_CHAIN)
            {
                if (addend >= *request)
                {
                    result += *request;
                    chain->Point += *request << drv->bSectorExp;
                }
                else
                {
                    result += addend;
                    chain->Point += addend << drv->bSectorExp;
                }

                *request = 0;
                break;
            }

            if (*request >= addend)
            {
                result += addend;
                chain->Point += addend << drv->bSectorExp;
                *request -= addend;
            }
            else
            {
                result += *request;
                chain->Point += *request << drv->bSectorExp;
                *request = 0;
                break;
            }

            addend = drv->ClusterSize;
            chain->Current = current;
        } while (chain->Current == previous + 1);
    }

    return result;
}



//note: Actually, 'static' function will not be included in the generated Doxygen document.
#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup For_VideoPlayback
///@brief   Seek from current chain point to the specified position in the chain. \n
///         This function is variant of the ChainSeek() in CHAIN module and only for video playback!
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
///@remark  Note that this function is only for video file playback usage!
///
#endif
static SWORD FChainSeek(DRIVE * drv, CHAIN * chain, DWORD position)
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

    if (position < chain->Point)
    {
        if( (position >> (drv->ClusterExp + drv->bSectorExp))
             == (chain->Point >> (drv->ClusterExp + drv->bSectorExp)) )  //position within same (current) cluster
        {
            chain->Point = position;
            return FS_SUCCEED;
        }
        else
            ChainSeekSet(chain);
    }

    return FChainSeekForward(drv, chain, position - chain->Point);
}


#endif //ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING

