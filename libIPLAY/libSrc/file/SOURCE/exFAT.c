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
* Filename      : exFAT.c
* Programmers   : WeiChing Tu
* Created       : 
* Descriptions  : Support exFAT file system.
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE   0

/*
// Include section
*/
#include "global612.h"
#include "mptrace.h"
#include "exFAT.h"
#include "fat.h"
#include "file.h"
#include "chain.h"
#include "dir.h"
#include "index.h"
#include "devio.h"
#include "taskid.h"
#include <string.h>

#if EXFAT_ENABLE

static DWORD CalUpcaseTableChksum(BYTE *table_buffer, DWORD table_len);
static int ParseUpcaseTableCharMappings(BYTE *table_buffer, DWORD table_len);


/* Load the exFAT Allocation Bitmaps content from the exFAT drive storage.
 *
 * return  FS_SUCCEED         Load the Allocation Bitmaps content successfully. \n\n
 * return  END_OF_DIR         Reach end of directory, and Allocation Bitmap not found. \n\n
 * return  ABNORMAL_STATUS    Failed due to some error. \n\n
 * retval  INVALID_DRIVE      Invalid drive.
 */
int Load_exFatAllocBitmap(DRIVE * drv)
{
    TYPE_exFAT_Generic_DirEntry_Template *exfat_node;
    TYPE_exFAT_Alloc_Bitmap_DirEntry *bitmap_node;
    BYTE bitmapId = 0;
    int numOfBitmaps = 0;
    BYTE *buffer;
    DWORD work_lba, work_sectorCount, table_len;
    U64 dataLength;
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        MP_ALERT("%s: Error! The drive is not exFAT file system type !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    FirstNode(drv);
    do
    {
        exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) (drv->Node);
        if (exfat_node->entryType == EXFAT_ENTRY_TYPE_EOD) /* 0x00: means end of directory */
        {
            return END_OF_DIR;
        }
        else if (exfat_node->entryType == EXFAT_ENTRY_TYPE_BITMAP)
        {
            numOfBitmaps++;
            bitmap_node = (TYPE_exFAT_Alloc_Bitmap_DirEntry *) exfat_node;
            bitmapId = ((BYTE) bitmap_node->bitmapFlags & EXFAT_BitmapFlags_MASK_BitmapId);

            BYTE *Hi32_of_64, *Lo32_of_64;
            Lo32_of_64 = (BYTE *) &bitmap_node->dataLength; /* due to little-endian */
            Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
            dataLength = (U64) ((U64) LoadAlien32(Hi32_of_64) << 32) + (U64) LoadAlien32(Lo32_of_64);
            if ((U64) dataLength >= (DWORD) 0xFFFFFFFF)
            {
                MP_ALERT("%s: Impossible !  exFAT Allocation Bitmap size >= 4GB => too big !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
            else
            {
                table_len = (DWORD) dataLength;
                MP_DEBUG("%s: exFAT Allocation Bitmap size = %lu bytes, BitmapId = %d", __FUNCTION__, table_len, bitmapId);

                if (table_len % (1 << drv->bSectorExp))
                    work_sectorCount = (table_len >> drv->bSectorExp) + 1;
                else
                    work_sectorCount = (table_len >> drv->bSectorExp);

                buffer = (BYTE *) ker_mem_malloc((work_sectorCount << drv->bSectorExp), TaskGetId()); //length must be multiple of sector size for DriveRead() !
                if (buffer == NULL)
                {
                    MP_ALERT("%s: Error! ker_mem_malloc() failed !", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }
                MpMemSet(buffer, 0, (work_sectorCount << drv->bSectorExp));

                work_lba = drv->DataStart + ((LoadAlien32(&bitmap_node->firstCluster) - EXFAT_FIRST_DATA_CLUSTER) << drv->ClusterExp);

                MP_DEBUG("%s: load Allocation Bitmap ..., lba = 0x%x, sectors count = %d", __FUNCTION__, work_lba, work_sectorCount);
                if (DriveRead(drv, buffer, work_lba, work_sectorCount) != FS_SUCCEED)
                {
                    MP_ALERT("%s: DriveRead() failed !", __FUNCTION__);
                    ker_mem_free(buffer);
                    return ABNORMAL_STATUS;
                }

                drv->exFAT_InfoFileds->AllocBitmapContent[bitmapId] = buffer;
                drv->exFAT_InfoFileds->AllocBitmapSize[bitmapId] = table_len;
                drv->Flag.AllocBitmapReady = 1;
                MP_ALERT("%s: exFAT Allocation Bitmap of [BitmapId = %d] loaded OK !\r\n", __FUNCTION__, bitmapId);
            }
        }
        else
        {
            ret = NextNode(drv);
            if (ret != FS_SUCCEED)
                return ret;
        }
    } while ((exfat_node->entryType != EXFAT_ENTRY_TYPE_BITMAP) && (numOfBitmaps < drv->NumberOfFat));

    return FS_SUCCEED;
}



/* Calculate the checksum of whole exFAT Up-case Table content */
/* algorithm following page 60 of exFAT Spec (Ver 3.00 Draft 1.00) from SD Card Association */
static DWORD CalUpcaseTableChksum(BYTE *table_buffer, DWORD table_len)
{
    DWORD chksum = 0x00000000;
    DWORD index;


    if ((table_buffer == NULL) || (table_len == 0))
    {
        MP_ALERT("%s: Error! NULL Upcase Table !", __FUNCTION__);
        return 0;
    }

    for (index = 0; index < table_len; index++)
    {
        chksum = ((chksum << 31) | (chksum >> 1)) + (DWORD) table_buffer[index];
    }

    return chksum;
}



/* Load the exFAT Up-case Table content from the exFAT drive storage.
 *
 * return  FS_SUCCEED         Load the Up-case Table content successfully. \n\n
 * return  END_OF_DIR         Reach end of directory, and Up-case Table not found. \n\n
 * return  ABNORMAL_STATUS    Failed due to some error. \n\n
 * retval  INVALID_DRIVE      Invalid drive.
 */
int Load_exFatUpcaseTable(DRIVE * drv)
{
    TYPE_exFAT_Generic_DirEntry_Template *exfat_node;
    TYPE_exFAT_Upcase_Table_DirEntry *upcaseTable_node;
    BYTE *buffer;
    DWORD work_lba, work_sectorCount, table_len;
    U64 dataLength;
    DWORD chksum;
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        MP_ALERT("%s: Error! The drive is not exFAT file system type !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    FirstNode(drv);
    do
    {
        exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) (drv->Node);
        if (exfat_node->entryType == EXFAT_ENTRY_TYPE_EOD) /* 0x00: means end of directory */
        {
            return END_OF_DIR;
        }
        else if (exfat_node->entryType == EXFAT_ENTRY_TYPE_UPCASE_TABLE)
        {
            upcaseTable_node = (TYPE_exFAT_Upcase_Table_DirEntry *) exfat_node;

            BYTE *Hi32_of_64, *Lo32_of_64;
            Lo32_of_64 = (BYTE *) &upcaseTable_node->dataLength; /* due to little-endian */
            Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
            dataLength = (U64) ((U64) LoadAlien32(Hi32_of_64) << 32) + (U64) LoadAlien32(Lo32_of_64);
            if ((U64) dataLength >= (DWORD) 0xFFFFFFFF)
            {
                MP_ALERT("%s: Impossible !  exFAT Up-case Table size >= 4GB => too big !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
            else
            {
                table_len = (DWORD) dataLength;
                MP_DEBUG("%s: exFAT Up-case Table size = %lu bytes", __FUNCTION__, table_len);

                if (table_len % (1 << drv->bSectorExp))
                    work_sectorCount = (table_len >> drv->bSectorExp) + 1;
                else
                    work_sectorCount = (table_len >> drv->bSectorExp);

                buffer = (BYTE *) ker_mem_malloc((work_sectorCount << drv->bSectorExp), TaskGetId()); //length must be multiple of sector size for DriveRead() !
                if (buffer == NULL)
                {
                    MP_ALERT("%s: Error! ker_mem_malloc() failed !", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }
                MpMemSet(buffer, 0, (work_sectorCount << drv->bSectorExp));

                work_lba = drv->DataStart + ((LoadAlien32(&upcaseTable_node->firstCluster) - EXFAT_FIRST_DATA_CLUSTER) << drv->ClusterExp);

                MP_DEBUG("%s: load Up-case Table ..., lba = 0x%x, sectors count = %d", __FUNCTION__, work_lba, work_sectorCount);
                if (DriveRead(drv, buffer, work_lba, work_sectorCount) != FS_SUCCEED)
                {
                    MP_ALERT("%s: DriveRead() failed !", __FUNCTION__);
                    ker_mem_free(buffer);
                    return ABNORMAL_STATUS;
                }

                if (drv->exFAT_InfoFileds->UpCaseTableContent)
                    ker_mem_free(drv->exFAT_InfoFileds->UpCaseTableContent);

                chksum = CalUpcaseTableChksum(buffer, table_len);
                if (chksum != LoadAlien32(&upcaseTable_node->tableChksum))
                {
                    MP_ALERT("%s: calculated checksum = 0x%08x, checksum field in the DirectoryEntry = 0x%08x", __FUNCTION__, chksum, LoadAlien32(&upcaseTable_node->tableChksum));
                    MP_ALERT("%s: Error! Invalid checksum of the Up-case Table => discard whole table content !", __FUNCTION__);
                    drv->exFAT_InfoFileds->UpCaseTableContent = NULL;
                    drv->exFAT_InfoFileds->UpCaseTableSize = 0;
                    drv->exFAT_InfoFileds->UpCaseTableChkSum = 0;
                    ker_mem_free(buffer);
                    return ABNORMAL_STATUS;
                }
                else
                {
                    drv->exFAT_InfoFileds->UpCaseTableContent = buffer;
                    drv->exFAT_InfoFileds->UpCaseTableSize = table_len;
                    drv->exFAT_InfoFileds->UpCaseTableChkSum = chksum;

                    ret = ParseUpcaseTableCharMappings(buffer, table_len);
                    if (ret != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Parse Unicode character mappings of Up-case Table failed !", __FUNCTION__);
                    }
                    else
                    {
                        MP_ALERT("%s: exFAT Up-case Table loaded OK !\r\n", __FUNCTION__);
                    }

                    return ret;
                }
            }
        }
        else
        {
            ret = NextNode(drv);
            if (ret != FS_SUCCEED)
                return ret;
        }
    } while (exfat_node->entryType != EXFAT_ENTRY_TYPE_UPCASE_TABLE);

    return FS_SUCCEED;
}



/* Parse the series of Unicode character mappings in the whole exFAT Up-case Table */
static int ParseUpcaseTableCharMappings(BYTE *table_buffer, DWORD table_len)
{
    DWORD i;


    if ((table_buffer == NULL) || (table_len == 0))
    {
        MP_ALERT("%s: Error! NULL Upcase Table !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

mpDebugPrint("%s: To-Do: Parse series of Unicode character mappings in the exFAT Up-case Table ...", __FUNCTION__);

#if 0
    do
    {

        /* To-Do: where to keep the parsed mapping data ? */

    } while (i < table_len);
#endif

    return FS_SUCCEED;
}



/* Calculate the hash value of exFAT up-cased file name */
WORD Cal_exFatFilenameHash(WORD * utf16_filename, BYTE name_length)
{
    WORD hash = 0x0000;
    WORD index, wCh;


    if ((utf16_filename == NULL) || (name_length == 0))
    {
        MP_ALERT("%s: Error! Invalid input parameter !", __FUNCTION__);
        return 0;
    }

#if 0 /* this NG !! */
    /* algorithm exactly following page 82 of exFAT Spec (Ver 3.00 Draft 1.00) from SD Card Association */
    BYTE *buffer = (BYTE *) utf16_filename;
    WORD numOfBytes = (WORD) name_length * 2;
    for (index = 0; index < numOfBytes; index++)
    {
        wCh = (WORD) buffer[index];
        CharUpperCase16((WORD *) &wCh); /* convert to uppercase */
        hash = ((hash << 15) | (hash >> 1)) + wCh;
    }
#else /* Workable !! */
    /* algorithm referred to the exFAT implementation library by Andrew Nayenko */
    for (index = 0; index < name_length; index++)
    {
        wCh = utf16_filename[index];
        CharUpperCase16((WORD *) &wCh); /* convert to uppercase */

        hash = ((hash << 15) | (hash >> 1)) + (wCh & 0xff);
        hash = ((hash << 15) | (hash >> 1)) + (wCh >> 8);
    }
#endif

    return hash;
}



/* Scan Allocation Bitmap for counting total number of free clusters.
 *
 * return  FS_SUCCEED         Scan Allocation Bitmap successfully. \n\n
 * return  ABNORMAL_STATUS    Failed due to some error. \n\n
 * retval  INVALID_DRIVE      Invalid drive.
 */
int ScanAllocBitmap_FreeClusters(DRIVE * drv)
{
    DWORD cluster;
    DWORD ret;
    BYTE  value;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    drv->FreeClusters = 0;
    drv->LastCluster = 0;
    for (cluster = 2; cluster < drv->TotalClusters + 2; cluster++)
    {
        value = Read_AllocBitmapEntry(drv, cluster);
        if (value == 0xFF)
        {
            MP_ALERT("%s: Read_AllocBitmapEntry() failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else if (value == 0x00) /* 0 means free entry */
        {
            drv->FreeClusters++;

            if (!drv->LastCluster)
                drv->LastCluster = cluster;
        }
    }

    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Get/extract (year, month, day, hour, minute, second) date/time info from a 32-bit exFAT little-endian timestamp field value of an exFAT File DirectoryEntry.
///
///@param   fileTimestamp       [IN] The 32-bit little-endian timestamp field value within an exFAT File DirectoryEntry. \n\n
///@param   date_time_info      [OUT] Output date/time info contained in a DATE_TIME_INFO_TYPE type structure. \n\n
///
///@return  None.
///
///@remark  The input 'fileTimestamp' value is 32-bit little-endian because exFAT file system on disk data structure
///         are all little-endian.
///
///@remark  This function is used for exFAT file system only.
///
void FileGetDateTime_for_exFAT(DWORD timestamp, DATE_TIME_INFO_TYPE * date_time_info)
{
    DWORD dwTempValue;

    if (date_time_info == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return;
    }

    dwTempValue = (DWORD) byte_swap_of_dword(timestamp);

    date_time_info->year   = (WORD) (((dwTempValue & 0xfe000000) >> 25) + 1980);
    date_time_info->month  = (BYTE)  ((dwTempValue & 0x01f00000) >> 21);
    date_time_info->day    = (BYTE)  ((dwTempValue & 0x001f0000) >> 16);
    date_time_info->hour   = (BYTE)  ((dwTempValue & 0x0000f800) >> 11);
    date_time_info->minute = (BYTE)  ((dwTempValue & 0x000007e0) >> 5);
    date_time_info->second = (BYTE)  ((dwTempValue & 0x0000001f) * 2); //unit: 2 seconds
    date_time_info->UtcOffset = 0; /* because UtcOffset info is not contained in the 32-bit exFAT timestamp */

    MP_DEBUG("%s: (timestamp = 0x%08x => 0x%08x) => %u/%u/%u %u:%u:%u", __FUNCTION__, timestamp, dwTempValue, date_time_info->year, date_time_info->month, date_time_info->day, date_time_info->hour, date_time_info->minute, date_time_info->second);
    return;
}



///
///@ingroup FILE
///@brief   Convert the (year, month, day, hour, minute, second) date/time to a 32-bit little-endian timestamp field value for exFAT File DirectoryEntry.
///
///@param   date_time_info       Input date/time info contained in a DATE_TIME_INFO_TYPE type structure.\n\n
///
///@return  The converted 32-bit little-endian timestamp field value for setting to an exFAT File DirectoryEntry.
///
///@remark  The returned timestamp value for setting to an exFAT File DirectoryEntry is 32-bit little-endian because exFAT file system on disk
///         data structure are all little-endian.
///
///@remark  This function is used for exFAT file system only.
///
DWORD FileSetDateTime_for_exFAT(DATE_TIME_INFO_TYPE date_time_info)
{
    DWORD dwTempValue;

    /* check for valid ranges */
    if ( ((date_time_info.year < 1980) || (date_time_info.year > 2107)) ||
         ((date_time_info.month < 1) || (date_time_info.month > 12)) ||
         ((date_time_info.day < 1) || (date_time_info.day > 31)) ||
         (date_time_info.hour > 23) || (date_time_info.minute > 59) || (date_time_info.second > 59) )
    {
        return 0;
    }

    dwTempValue  = ((DWORD) (date_time_info.year - 1980) << 25);
    dwTempValue |= ((DWORD) date_time_info.month << 21);
    dwTempValue |= ((DWORD) date_time_info.day << 16);
    dwTempValue |= ((DWORD) date_time_info.hour << 11);
    dwTempValue |= ((DWORD) date_time_info.minute << 5);
    dwTempValue |= ((DWORD) date_time_info.second / 2); //unit: 2 seconds

    return (DWORD) byte_swap_of_dword(dwTempValue);
}



#endif //EXFAT_ENABLE
