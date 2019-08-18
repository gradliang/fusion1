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
* Filename      : drive.c
* Programmers   :
* Created       :
* Descriptions  :
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
#include "fat.h"
#include "exfat.h"
#include "file.h"
#include "chain.h"
#include "dir.h"
#include "index.h"
#include "devio.h"
#include "os.h"
#include "taskid.h"
//#include "usbotg.h"
#include <string.h>


/* note: Pls refer to drive.h for reserved drive index ID for each partition of storage device */
#define MAX_PARTITION_NUM    4  /* supported max partitions on a Non-removable storage device (ex: NAND, SPI, IDE-HDD ... etc) */

#pragma alignvar(4)
#if (MEM_2M)
static DRIVE Drive[MAX_DRIVE_NUM];
#else
static DRIVE Drive[MAX_DRIVE_NUM + 1]; //add one extra DRIVE entry for xpgslidefunc use
static CACHE CacheBuffer[1];
#endif

#if FS_REENTRANT_API
TASK_WORKING_DRIVE_TYPE *Tasks_WorkingDrv_List = NULL, *Tasks_WorkingDrv_List_Tail = NULL;
#endif

static BYTE DriveCounter = 0;
static E_DRIVE_INDEX_ID CurrDrive = NULL_DRIVE;


static DWORD ReadFat12(void * drv, DWORD cluster);
static DWORD ReadFat16(void * drv, DWORD cluster);
static DWORD ReadFat32(void * drv, DWORD cluster);
static int WriteFat12(void * drv, DWORD cluster, DWORD content);
static int WriteFat16(void * drv, DWORD cluster, DWORD content);
static int WriteFat32(void *, DWORD, DWORD);
#if EXFAT_ENABLE
static DWORD Read_exFAT(void * drv, DWORD cluster);
static int Write_exFAT(void * drv, DWORD cluster, DWORD content);
#endif

static int ScanFat(DRIVE * drv);
static int BootRecordRead(DRIVE * drv, DWORD start);
static int FloppyCheck(DRIVE * drv);
static int PartitionScan(DRIVE * drv);


/* dummy FAT entry read function */
static DWORD NullFatRead(void *drv, DWORD cluster)
{
    return FAT_READ_END_OF_CHAIN;
}



/* dummy FAT entry write function */
static int NullFatWrite(void * drv, DWORD cluster, DWORD content)
{
    return DRIVE_ACCESS_FAIL;
}



#if FS_REENTRANT_API

void Set_UpdatingFAT_Status(DRIVE * drv, BYTE * func_scope)
{
    BYTE curr_task_id = TaskGetId();

    if ((drv == NULL) || (drv->CacheBufPoint == NULL))
    {
        MP_ALERT("%s: Error! NULL handle !", __FUNCTION__);
        return;
    }

    /* check whether if previous FAT entries or DirCache updating process by another task is still not finished */
    while ( ((drv->CacheBufPoint->f_IsUpdatingFatEntries) && (drv->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id)) ||
            ((drv->CacheBufPoint->f_IsUpdatingFdbNodes) && (drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes != curr_task_id)) )
    {
        if ((drv->CacheBufPoint->f_IsUpdatingFatEntries) && (drv->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id))
            mpDebugPrint("%s: task ID %d is waiting FAT update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, drv->CacheBufPoint->TaskId_of_UpdatingFatEntries);
        else if ((drv->CacheBufPoint->f_IsUpdatingFdbNodes) && (drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes != curr_task_id))
            mpDebugPrint("%s: task ID %d is waiting DirCache update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes);

        TaskSleep(10); /* force task context switch to let another task to finish its process first */
    }

    IntDisable();

    drv->CacheBufPoint->f_IsUpdatingFatEntries = 1;
    drv->CacheBufPoint->TaskId_of_UpdatingFatEntries = curr_task_id;
    if (StringCompare08((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFatEntries, "") == E_COMPARE_EQUAL)
    {
        StringCopy08((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFatEntries, func_scope);
    }

    IntEnable();
}


void Reset_UpdatingFAT_Status(DRIVE * drv, BYTE * func_scope)
{
    BYTE curr_task_id = TaskGetId();

    if ((drv == NULL) || (drv->CacheBufPoint == NULL))
    {
        MP_ALERT("%s: Error! NULL handle !", __FUNCTION__);
        return;
    }

    /* check whether if previous FAT entries or DirCache updating process by another task is still not finished */
    while ( ((drv->CacheBufPoint->f_IsUpdatingFatEntries) && (drv->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id)) ||
            ((drv->CacheBufPoint->f_IsUpdatingFdbNodes) && (drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes != curr_task_id)) )
    {
        if ((drv->CacheBufPoint->f_IsUpdatingFatEntries) && (drv->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id))
            mpDebugPrint("%s: task ID %d is waiting FAT update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, drv->CacheBufPoint->TaskId_of_UpdatingFatEntries);
        else if ((drv->CacheBufPoint->f_IsUpdatingFdbNodes) && (drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes != curr_task_id))
            mpDebugPrint("%s: task ID %d is waiting DirCache update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes);

        TaskSleep(10); /* force task context switch to let another task to finish its process first */
    }

    IntDisable();

    if ( (StringCompare08((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFatEntries, func_scope) == E_COMPARE_EQUAL) ||
         (StringCompare08((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFatEntries, "") == E_COMPARE_EQUAL) )
    {
        /* note: add pointer check for safety ! Because MpMemSet() may cause "break 100" when memory buffer was already released first by Card Out, for example. */
        if ((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFatEntries != NULL)
            MpMemSet((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFatEntries, 0x00, FUNC_SCOPE_MAX_NAME_LEN);

        drv->CacheBufPoint->f_IsUpdatingFatEntries = 0;
        drv->CacheBufPoint->TaskId_of_UpdatingFatEntries = 0; /* task id 0 is unused */
    }

    IntEnable();
}


void Set_UpdatingFdbNodes_Status(DRIVE * drv, BYTE * func_scope)
{
    BYTE curr_task_id = TaskGetId();

    if ((drv == NULL) || (drv->CacheBufPoint == NULL))
    {
        MP_ALERT("%s: Error! NULL handle !", __FUNCTION__);
        return;
    }

    /* check whether if previous FAT entries or DirCache updating process by another task is still not finished */
    while ( ((drv->CacheBufPoint->f_IsUpdatingFatEntries) && (drv->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id)) ||
            ((drv->CacheBufPoint->f_IsUpdatingFdbNodes) && (drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes != curr_task_id)) )
    {
        if ((drv->CacheBufPoint->f_IsUpdatingFatEntries) && (drv->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id))
            mpDebugPrint("%s: task ID %d is waiting FAT update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, drv->CacheBufPoint->TaskId_of_UpdatingFatEntries);
        else if ((drv->CacheBufPoint->f_IsUpdatingFdbNodes) && (drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes != curr_task_id))
            mpDebugPrint("%s: task ID %d is waiting DirCache update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes);

        TaskSleep(10); /* force task context switch to let another task to finish its process first */
    }

    IntDisable();

    drv->CacheBufPoint->f_IsUpdatingFdbNodes = 1;
    drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes = curr_task_id;
    if (StringCompare08((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFdbNodes, "") == E_COMPARE_EQUAL)
    {
        StringCopy08((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFdbNodes, func_scope);
    }

    IntEnable();
}


void Reset_UpdatingFdbNodes_Status(DRIVE * drv, BYTE * func_scope)
{
    BYTE curr_task_id = TaskGetId();

    if ((drv == NULL) || (drv->CacheBufPoint == NULL))
    {
        MP_ALERT("%s: Error! NULL handle !", __FUNCTION__);
        return;
    }

    /* check whether if previous FAT entries or DirCache updating process by another task is still not finished */
    while ( ((drv->CacheBufPoint->f_IsUpdatingFatEntries) && (drv->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id)) ||
            ((drv->CacheBufPoint->f_IsUpdatingFdbNodes) && (drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes != curr_task_id)) )
    {
        if ((drv->CacheBufPoint->f_IsUpdatingFatEntries) && (drv->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id))
            mpDebugPrint("%s: task ID %d is waiting FAT update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, drv->CacheBufPoint->TaskId_of_UpdatingFatEntries);
        else if ((drv->CacheBufPoint->f_IsUpdatingFdbNodes) && (drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes != curr_task_id))
            mpDebugPrint("%s: task ID %d is waiting DirCache update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes);

        TaskSleep(10); /* force task context switch to let another task to finish its process first */
    }

    IntDisable();

    if ( (StringCompare08((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFdbNodes, func_scope) == E_COMPARE_EQUAL) ||
         (StringCompare08((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFdbNodes, "") == E_COMPARE_EQUAL) )
    {
        /* note: add pointer check for safety ! Because MpMemSet() may cause "break 100" when memory buffer was already released first by Card Out, for example. */
        if ((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFdbNodes != NULL)
            MpMemSet((BYTE *) drv->CacheBufPoint->FuncScope_of_UpdatingFdbNodes, 0x00, FUNC_SCOPE_MAX_NAME_LEN);

        drv->CacheBufPoint->f_IsUpdatingFdbNodes = 0;
        drv->CacheBufPoint->TaskId_of_UpdatingFdbNodes = 0; /* task id 0 is unused */
    }

    IntEnable();
}

#endif //FS_REENTRANT_API



/* Initialize the internal DRIVE table and assign internal cache */
static void DriveInit(void)
{
    DRIVE *drv;
    CACHE *cache;
    E_DRIVE_INDEX_ID i;

#if (MEM_2M)
    /* note: add pointer check for safety ! Because MpMemSet() may cause "break 100" when memory buffer was already released first by Card Out, for example. */
    if ((BYTE *) Drive != NULL)
        MpMemSet(Drive, 0, sizeof(DRIVE) * MAX_DRIVE_NUM);
#else
    /* note: add pointer check for safety ! Because MpMemSet() may cause "break 100" when memory buffer was already released first by Card Out, for example. */
    if ((BYTE *) Drive != NULL)
        MpMemSet(Drive, 0, sizeof(DRIVE) * (MAX_DRIVE_NUM + 1));

    if ((BYTE *) CacheBuffer != NULL)
        MpMemSet(CacheBuffer, 0, sizeof(CACHE) * 1);
#endif

    drv = Drive;

#if (MEM_2M)
    for (i = 0; i < MAX_DRIVE_NUM; i++)
    {
#else
    for (i = 0; i < (MAX_DRIVE_NUM + 1); i++)
    {
        if (i == MAX_DRIVE_NUM)
        {
            cache = (CACHE *) ((DWORD) (&CacheBuffer[0]) | 0xa0000000);
            MpMemSet(cache, 0, sizeof(CACHE));

            drv->CacheBufPoint = cache;
            drv->FatCacheBuffer = cache->FatCache;
            drv->DirStackBuffer = cache->Stack;
            drv->DirCacheBuffer = cache->DirCache;
        }
        else
#endif
        {
            drv->CacheBufPoint = NULL;
            drv->FatCacheBuffer = NULL;
            drv->DirStackBuffer = NULL;
            drv->DirCacheBuffer = NULL;
        }

        drv->FatCachePoint = 0xffffffff;
        drv->DirCachePoint = 0xffffffff;
        drv->DevID = DriveIndex2PhyDevID(i);
        drv->DrvIndex = i;
        drv->bSectorExp = 9;	// initial value for 512 bytes
        drv->FatRead = NullFatRead;
        drv->FatWrite = NullFatWrite;
        drv->StatusCode = NOT_SUPPORT_FS;
        /* For non-storage or non-FAT devices, always set drv->Flag.Present as 1 for passing card present check */
        if ((drv->DevID == DEV_USB_HOST_PTP)       ||\
            (drv->DevID == DEV_USBOTG1_HOST_PTP)   ||\
            (drv->DevID == DEV_USB_WIFI_DEVICE)    ||\
            (drv->DevID == DEV_CF_ETHERNET_DEVICE) ||\
            (drv->DevID == DEV_USB_ETHERNET_DEVICE))
        {
            drv->Flag.Present = 1;
        }
        else
            drv->Flag.Present = 0;

        SemaphoreCreate(DRIVE_FAT_CACHE_SEMA_ID_BASE + i, OS_ATTR_PRIORITY, 1); /* semaphore for each drive's FAT cache buffer */

        /* next drive entry */
        drv++;
    }
}



///
///@ingroup DRIVE
///@brief   Copy whole data structure content of a DRIVE handle (including DRIVE structure and its internal CACHE buffer) to another DRIVE handle.
///
///@param   drv1         The target DRIVE handle. \n\n
///@param   drv2         The source DRIVE handle.
///
///@return  None.
///
///@remark  This function only performs copying of a DRIVE data structure and its internal CACHE buffer content, nothing else.
///
void DriveHandleCopy(DRIVE * drv1, DRIVE * drv2)  // for xpgslidefunc use
{
#if (MEM_2M == 0)
    CACHE *cache;

    if ((drv1 == NULL) || (drv2 == NULL))
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return;
    }

    cache = (CACHE *) ((DWORD) (&CacheBuffer[0]) | 0xa0000000);

    MpMemCopy((BYTE *) drv1, (BYTE *) drv2, sizeof(DRIVE));
    MpMemCopy((BYTE *) &CacheBuffer[0], (BYTE *) drv2->CacheBufPoint, sizeof(CACHE));
    drv1->FatCacheBuffer = cache->FatCache;
    drv1->DirStackBuffer = cache->Stack;
    drv1->DirCacheBuffer = cache->DirCache;
#endif
}



#if (((CHIP_VER_MSB != CHIP_VER_650) && (CHIP_VER_MSB != CHIP_VER_660)) && CF_ENABLE)
extern void Ui_CfDetect(void);
#endif

///
///@ingroup DRIVE
///@brief   Initial all the data structure and operation variables for file system.
///
///@param   None.
///
///@return  None.
///
///@remark  This function is called only when file system initialization.
///
void FileSystemInit(void)
{
    DriveInit();
    StreamInit();
    SemaphoreCreate(FILE_READ_SEMA_ID, OS_ATTR_PRIORITY, 1);   /* temp solution for protecting concurrent multiple file read operations */
    SemaphoreCreate(FILE_WRITE_SEMA_ID, OS_ATTR_PRIORITY, 1);  /* temp solution for protecting concurrent multiple file write operations */
    DriveCounter = 0;
    CurrDrive = NULL_DRIVE;

#ifdef CARD_DETECT_FUNC_ENABLE
    SystemCardDetectCtrlMode(CARD_DETECT_FUNC_ENABLE);
#endif

#ifdef CARD_PROTECT_FUNC_ENABLE
    SystemCardProtectCtrlMode(CARD_PROTECT_FUNC_ENABLE);
#endif

    Mcard_DeviceClkConfig();
    RegisterCardDetectCB(SystemCardEventSet);
    RegisterCardFatalErrorCB(SystemCardFatalErrorEventSet);
    Mcard_Init(SystemCardCtrlModeGet());
#if SC_USBDEVICE
    SystemDeviceLunInfoUpdate();        // Update LUN info to MCard Task
#endif

#if ((CHIP_VER_MSB == CHIP_VER_615) && CF_ENABLE)
    SysTimerProcAdd(100, Ui_CfDetect, FALSE);
#endif

#if ((ISP_FUNC_ENABLE == ENABLE) && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
    ISP_SetNandReservedSize(NAND_RESERVED_SIZE);
#endif

    SysTimerProcAdd(2000, SystemCardStatusPollingEnable, TRUE);
}



///
///@ingroup DRIVE
///@brief   Scan partitions and the file system of each scanned partition on the specified device, and add scanned drives to
///         the internal Drive table.
///
///@param   drv_index_ID      The drive index ID of a storage drive to scan all drives on the device.
///
///@return  Number of drives on the device scanned and well added.
///
///@remark  This function call only adds well formatted device drives to the internal Drive table. If a device drive is not
///         formatted, it will not be added to the internal Drive table and not be counted for the return value. \n
///         Note that only HDD (hard disk) supports multiple partitions currently.
///
int DriveAdd(E_DRIVE_INDEX_ID drv_index_ID)
{
    SBYTE found_partitions, count, well_added_count = 0;
    DRIVE *drv;
    CACHE *cache;
    DRIVE_PHY_DEV_ID phyDevID = DriveIndex2PhyDevID(drv_index_ID);
    DWORD  size_of_AllocBitmap;


    if (drv_index_ID >= MAX_DRIVE_NUM)
    {
        MP_ALERT("%s: Invalid drive index ID (%d) !", __FUNCTION__, drv_index_ID);
        return 0;
    }

    MP_DEBUG("\r\n ### enter %s(drv_index_ID = %u):  phyDevID = %u, Ori DriveCounter = %u", __FUNCTION__, drv_index_ID, phyDevID, DriveCounter);
    drv = &Drive[PhyDevID2DriveIndex(phyDevID)]; /* note: point to DRIVE table entry of the 1st partition on the device */
    drv->DevID = phyDevID;
    drv->DrvIndex = PhyDevID2DriveIndex(phyDevID); /* note: the 1st partition of the device, set drive index value carefully */

    if ( (drv->CacheBufPoint == NULL) && \
         (drv->DevID != DEV_USB_WIFI_DEVICE) && \
         ((drv->DevID != DEV_USB_HOST_PTP) && \
          (drv->DevID != DEV_USBOTG1_HOST_PTP) && \
          (drv->DevID != DEV_CF_ETHERNET_DEVICE)) && \
          (drv->DevID != DEV_USB_WEBCAM) && \
          (drv->DevID != DEV_USB_ETHERNET_DEVICE)
       )
    {
        cache = (CACHE *) ker_mem_malloc(sizeof(CACHE), TaskGetId());
        if (cache == NULL)
        {
            MP_ALERT("%s: Error: ker_mem_malloc() failed for size %d bytes !", __FUNCTION__, sizeof(CACHE));
            return 0;
        }

        cache = (CACHE *) ((DWORD) cache | BIT29);
        MpMemSet(cache, 0, sizeof(CACHE));

        drv->CacheBufPoint = cache;
        drv->FatCacheBuffer = cache->FatCache;
        drv->DirStackBuffer = cache->Stack;
        drv->DirCacheBuffer = cache->DirCache;
    #if FS_REENTRANT_API
        Reset_UpdatingFdbNodes_Status(drv, ""); //initialize
        Reset_UpdatingFAT_Status(drv, ""); //initialize
    #endif
    }

    /* For non-storage or non-FAT devices, always set drv->Flag.Present as 1 for passing card present check */
    if ( (drv->DevID == DEV_USB_WIFI_DEVICE) || \
         (drv->DevID == DEV_CF_ETHERNET_DEVICE) || \
         ((drv->DevID == DEV_USB_HOST_PTP) || \
         (drv->DevID == DEV_USBOTG1_HOST_PTP)) ||\
         (drv->DevID == DEV_USB_WEBCAM) ||\
         (drv->DevID == DEV_USB_ETHERNET_DEVICE))
    {
        drv->FatRead        = NULL; /* note: here, must not use NullFatRead to meet DriveAdd() checking code for already well-added */
        drv->FatWrite       = NULL; /* note: here, must not use NullFatWrite to meet DriveAdd() checking code for already well-added */
        drv->Partition      = 0;
        drv->Flag.FsType    = FS_TYPE_UNSUPPORT;
        drv->Flag.Present   = 1; /* For non-storage or non-FAT devices, always set drv->Flag.Present as 1 for passing card present check */
        drv->StatusCode     = FS_SUCCEED;
        well_added_count = found_partitions = 1;
        DriveCountUpdate();
    }
    else
    {
        // scan the partition table of current drive
        found_partitions = PartitionScan(drv);
        if (found_partitions == 0)
        {
            if (drv->StatusCode != FS_SUCCEED)
            {
                // if Partition Scan failed, then need to do FloppyCheck
                if (FloppyCheck(drv) != FS_SUCCEED)
                {
                    drv->StatusCode = FS_SCAN_FAIL;
                    drv->Flag.Present = 0;

#if (SC_USBDEVICE)
                    SystemDriveLunInfoChange(drv->DrvIndex);
#endif

                    MP_ALERT("%s() failed due to FloppyCheck() failed !", __FUNCTION__);
                    return 0;
                }
                else
                {
                    drv->StatusCode = FS_SUCCEED;
                    drv->Flag.Present = 1;
                    well_added_count = found_partitions = 1;
                    DriveCountUpdate();
                }
            }
        }

        count = found_partitions; // number of scanned partitions of the device
        if (count <= 1)
        {
            if ((count == 1) && (drv->StatusCode == FS_SUCCEED))
            {
                DirReset(drv); // reset/initialize working directory status to Root Dir of the drive
#if (SC_USBDEVICE)
                SystemDriveLunInfoChange(drv->DrvIndex);
#endif

#if EXFAT_ENABLE
                if (drv->Flag.FsType == FS_TYPE_exFAT)
                {
                    if (Load_exFatAllocBitmap(drv) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Error! Load_exFatAllocBitmap() failed for the exFAT drive !", __FUNCTION__);
                        drv->StatusCode = NOT_SUPPORT_FS;
                        DriveCountUpdate();
                        MP_DEBUG(" exit %s(): found_partitions = %u, well_added_count = %u, DriveCounter = %u", __FUNCTION__, found_partitions, well_added_count, DriveCounter);
                        return well_added_count;
                    }
                    else
                    {
                        if (ScanAllocBitmap_FreeClusters(drv) != FS_SUCCEED)
                        {
                            MP_ALERT("%s: Error! ScanAllocBitmap_FreeClusters() failed for the exFAT drive !", __FUNCTION__);
                            drv->StatusCode = NOT_SUPPORT_FS;
                            DriveCountUpdate();
                            MP_DEBUG(" exit %s(): found_partitions = %u, well_added_count = %u, DriveCounter = %u", __FUNCTION__, found_partitions, well_added_count, DriveCounter);
                            return well_added_count;
                        }
                        else
                        {
                            MP_DEBUG("%s: Current count of free clusters = %lu \r\n", __FUNCTION__, drv->FreeClusters);
                        }
                    }

                    if (Load_exFatUpcaseTable(drv) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Error! Load_exFatUpcaseTable() failed for the exFAT drive !", __FUNCTION__);
                        drv->StatusCode = NOT_SUPPORT_FS;
                        DriveCountUpdate();
                        MP_DEBUG(" exit %s(): found_partitions = %u, well_added_count = %u, DriveCounter = %u", __FUNCTION__, found_partitions, well_added_count, DriveCounter);
                        return well_added_count;
                    }
                }
                else
#endif
                {
                    /* reset free cluster counter values first */
                    drv->FreeClusters = 0;
                    drv->LastCluster = 0;

            /* Note: To reduce total boot time, move ScanFat() of FAT12/16/32 drives out of DriveAdd() procedure.
             * Do ScanFat() later for FAT12/16/32 drives when first time accessing the drv->FreeClusters value (ex: file write, file create or check free disk space, ... etc)
             */
            #if 0
                    if (ScanFat(drv) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Error! ScanFat() failed !", __FUNCTION__);
                        drv->StatusCode = NOT_SUPPORT_FS;
                        DriveCountUpdate();
                        MP_DEBUG(" exit %s(): found_partitions = %u, well_added_count = %u, DriveCounter = %u", __FUNCTION__, found_partitions, well_added_count, DriveCounter);
                        return well_added_count;
                    }
                    else
                    {
                        MP_DEBUG("%s: Current count of free clusters = %lu \r\n", __FUNCTION__, drv->FreeClusters);
                    }
            #endif
                }
            }

            if (drv->StatusCode == FS_SUCCEED)
                well_added_count = 1;
            else
                well_added_count = 0;

            DriveCountUpdate();
            MP_DEBUG(" exit %s(): found_partitions = %u, well_added_count = %u, DriveCounter = %u", __FUNCTION__, found_partitions, well_added_count, DriveCounter);
            return well_added_count;
        }
        else
        {
            MP_ALERT("%s: multiple partitions found on the storage device !", __FUNCTION__);
        }

        MP_DEBUG("%s: scanned partitions count (not final well-added count) = %u", __FUNCTION__, count);
        while (count > 0)
        {
            drv->Partition++; // if this device contains more than one partition, then let these partitions number starting from 1, not from 0

            if (drv->CacheBufPoint == NULL)
            {
                cache = (CACHE *) ker_mem_malloc(sizeof(CACHE), TaskGetId());
                if (cache == NULL)
                {
                    MP_ALERT("%s: Error: ker_mem_malloc() failed for size %d bytes !", __FUNCTION__, sizeof(CACHE));
                    DriveCountUpdate();
                    MP_DEBUG(" exit %s(): found_partitions = %u, well_added_count = %u, DriveCounter = %u", __FUNCTION__, found_partitions, well_added_count, DriveCounter);
                    return well_added_count;
                }
                cache = (CACHE *) ((DWORD) cache | BIT29);
                MpMemSet(cache, 0, sizeof(CACHE));

                drv->CacheBufPoint = cache;
                drv->FatCacheBuffer = cache->FatCache;
                drv->DirStackBuffer = cache->Stack;
                drv->DirCacheBuffer = cache->DirCache;
            }

            if (drv->StatusCode == FS_SUCCEED)
            {
                DirReset(drv); // reset/initialize working directory status to Root Dir of the drive
#if (SC_USBDEVICE)
                SystemDriveLunInfoChange(drv->DrvIndex); /* use drv->DrvIndex here because drv->DrvIndex has been correctly set during PartitionScan() ! */
#endif

#if EXFAT_ENABLE
                if (drv->Flag.FsType == FS_TYPE_exFAT)
                {
                    if (Load_exFatAllocBitmap(drv) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Error! Load_exFatAllocBitmap() failed for the exFAT drive !", __FUNCTION__);
                        drv->StatusCode = NOT_SUPPORT_FS;
                    }
                    else
                    {
                        if (ScanAllocBitmap_FreeClusters(drv) != FS_SUCCEED)
                        {
                            MP_ALERT("%s: Error! ScanAllocBitmap_FreeClusters() failed for the exFAT drive !", __FUNCTION__);
                            drv->StatusCode = NOT_SUPPORT_FS;
                        }
                        else
                        {
                            MP_DEBUG("%s: Current count of free clusters = %lu \r\n", __FUNCTION__, drv->FreeClusters);
                        }
                    }

                    if (Load_exFatUpcaseTable(drv) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Error! Load_exFatUpcaseTable() failed for the exFAT drive !", __FUNCTION__);
                        drv->StatusCode = NOT_SUPPORT_FS;
                    }
                }
                else
#endif
                {
                    /* reset free cluster counter values first */
                    drv->FreeClusters = 0;
                    drv->LastCluster = 0;

            /* Note: To reduce total boot time, move ScanFat() of FAT12/16/32 drives out of DriveAdd() procedure.
             * Do ScanFat() later for FAT12/16/32 drives when first time accessing the drv->FreeClusters value (ex: file write, file create or check free disk space, ... etc)
             */
            #if 0
                    if (ScanFat(drv) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Error! ScanFat() failed !", __FUNCTION__);
                        drv->StatusCode = NOT_SUPPORT_FS;
                        DriveCountUpdate();
                        MP_DEBUG(" exit %s(): found_partitions = %u, well_added_count = %u, DriveCounter = %u", __FUNCTION__, found_partitions, well_added_count, DriveCounter);
                        return well_added_count;
                    }
                    else
                    {
                        MP_DEBUG("%s: Current count of free clusters = %lu \r\n", __FUNCTION__, drv->FreeClusters);
                    }
            #endif
                }

                well_added_count++; //count of well added partition drives of the device
            }

            drv++; // go to next entry in the Drive table for next partition found
            count--;
        }

        DriveCountUpdate();
        MP_DEBUG(" exit %s(): found_partitions = %u, well_added_count = %u, DriveCounter = %u", __FUNCTION__, found_partitions, well_added_count, DriveCounter);
        return well_added_count;
    }

    DriveCountUpdate();
    DirReset(drv); // reset/initialize working directory status to Root Dir of the drive

    MP_DEBUG(" exit %s(): found_partitions = %u, well_added_count = %u, DriveCounter = %u", __FUNCTION__, found_partitions, well_added_count, DriveCounter);
    return well_added_count;
}



///
///@ingroup DRIVE
///@brief   Delete all the drives of a device corresponding to the specified drive index ID from the internal Drive table and release their allocated cache buffers.
///
///@param   drv_index_ID      The drive index ID of a storage drive to delete all drives on the device.
///
///@return  None.
///
///@remark  Note that the only difference between SingleDriveDelete() function and DriveDelete() is how many drives to be deleted from Drive[] table.
///
void DriveDelete(E_DRIVE_INDEX_ID drv_index_ID)
{
    DRIVE *drv;
    E_DRIVE_INDEX_ID idx;
    DRIVE_PHY_DEV_ID phyDevID = DriveIndex2PhyDevID(drv_index_ID);

    if (drv_index_ID >= MAX_DRIVE_NUM)
    {
        MP_ALERT("%s: Invalid drive index ID (%d) !", __FUNCTION__, drv_index_ID);
        return;
    }

    /* note: delete drives info starting from the DRIVE table entry for the 1st partition of the device */
    for (idx = PhyDevID2DriveIndex(phyDevID); (idx < MAX_DRIVE_NUM) && (Drive[idx].DevID == phyDevID); idx++)
    {
        /* make sure the accesssing for this drive's FAT cache by other task is finished */
        SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + idx);

    /* note: Disable this! this caused side-effect in MP622D ! Because the Drive[] entry was not reset, and then in some cases (ex: ScanFat(), ... etc) it may fail due to drv->StatusCode checking ! */
    #if 0
        if ((Drive[idx].FatRead == NullFatRead) && (Drive[idx].FatWrite == NullFatWrite)) //not used entry
        {
            SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + idx); /* release semaphore */
            continue;
        }
    #endif

        MP_DEBUG("%s: clear info of Drive[%d] table entry...", __FUNCTION__, idx);
        drv = &Drive[idx];

        /* release allocated cache buffer */
        if (drv->CacheBufPoint)
            ker_mem_free(drv->CacheBufPoint);

#if EXFAT_ENABLE
        if (drv->exFAT_InfoFileds)
        {
            if (drv->exFAT_InfoFileds->AllocBitmapContent[0])
                ker_mem_free(drv->exFAT_InfoFileds->AllocBitmapContent[0]);
            if (drv->exFAT_InfoFileds->AllocBitmapContent[1])
                ker_mem_free(drv->exFAT_InfoFileds->AllocBitmapContent[1]);

            if (drv->exFAT_InfoFileds->UpCaseTableContent)
                ker_mem_free(drv->exFAT_InfoFileds->UpCaseTableContent);

            ker_mem_free(drv->exFAT_InfoFileds);
        }
#endif

#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
        if (drv->AllocBitmap_for_NonExFAT)
            ext_mem_free(drv->AllocBitmap_for_NonExFAT); /* note: kernel memory is not very enough, use ext_mem_malloc/ext_mem_free for AllocBitmap of non-exFAT drives */
#endif

        if ((drv->StatusCode != NOT_SUPPORT_FS) && !((drv->FatRead == NullFatRead) && (drv->FatWrite == NullFatWrite)))  /* a well-added Drive[] entry */
            DriveCounter--;

        MpMemSet(drv, 0, sizeof(DRIVE));
        drv->CacheBufPoint = NULL;
        drv->FatCacheBuffer = NULL;
        drv->DirStackBuffer = NULL;
        drv->DirCacheBuffer = NULL;
        drv->FatCachePoint = 0xffffffff;
        drv->DirCachePoint = 0xffffffff;
        drv->bSectorExp = 9;	// initial value for 512 bytes
        drv->FatRead = NullFatRead;
        drv->FatWrite = NullFatWrite;
        drv->DevID = phyDevID; /* device ID */
        drv->DrvIndex = idx; /* drive index ID */
        drv->StatusCode = NOT_SUPPORT_FS;

        /* For non-storage or non-FAT devices, always set drv->Flag.Present as 1 for passing card present check */
        if ( ((drv->DevID == DEV_USB_HOST_PTP) || \
              (drv->DevID == DEV_USBOTG1_HOST_PTP)) || \
              (drv->DevID == DEV_USB_WIFI_DEVICE) || \
              (drv->DevID == DEV_CF_ETHERNET_DEVICE) ||\
               (drv->DevID == DEV_USB_WEBCAM) ||\
               (drv->DevID == DEV_USB_ETHERNET_DEVICE) )
        {
            drv->Flag.Present = 1;
        }
        else
            drv->Flag.Present = 0;

#if (SC_USBDEVICE)
        SystemDriveLunInfoChange(drv->DrvIndex);
#endif

        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + idx); /* release semaphore */
    }
    MP_DEBUG("### exit %s(): DriveCounter = %u \r\n", __FUNCTION__, DriveCounter);
}



///
///@ingroup DRIVE
///@brief   Delete the single drive by the specified drive index ID from the internal Drive table and release its allocated cache buffers.
///
///@param   drv_index_ID      The drive index ID of a storage drive to delete.
///
///@return  None.
///
///@remark  Note that the only difference between SingleDriveDelete() function and DriveDelete() is how many drives to be deleted from Drive[] table.
///
void SingleDriveDelete(E_DRIVE_INDEX_ID drv_index_ID)
{
    DRIVE *drv;
    DRIVE_PHY_DEV_ID phyDevID = DriveIndex2PhyDevID(drv_index_ID);

    if (drv_index_ID >= MAX_DRIVE_NUM)
    {
        MP_ALERT("%s: Invalid drive index ID (%d) !", __FUNCTION__, drv_index_ID);
        return;
    }

    /* make sure the accesssing for this drive's FAT cache by other task is finished */
    SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv_index_ID);

    /* note: reset the Drive[] entry */
    MP_DEBUG("%s: clear info of Drive[%d] table entry...", __FUNCTION__, drv_index_ID);
    drv = &Drive[drv_index_ID];

    /* release allocated cache buffer */
    if (drv->CacheBufPoint)
        ker_mem_free(drv->CacheBufPoint);

#if EXFAT_ENABLE
    if (drv->exFAT_InfoFileds)
    {
        if (drv->exFAT_InfoFileds->AllocBitmapContent[0])
            ker_mem_free(drv->exFAT_InfoFileds->AllocBitmapContent[0]);
        if (drv->exFAT_InfoFileds->AllocBitmapContent[1])
            ker_mem_free(drv->exFAT_InfoFileds->AllocBitmapContent[1]);

        if (drv->exFAT_InfoFileds->UpCaseTableContent)
            ker_mem_free(drv->exFAT_InfoFileds->UpCaseTableContent);

        ker_mem_free(drv->exFAT_InfoFileds);
    }
#endif

#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
    if (drv->AllocBitmap_for_NonExFAT)
        ext_mem_free(drv->AllocBitmap_for_NonExFAT); /* note: kernel memory is not very enough, use ext_mem_malloc/ext_mem_free for AllocBitmap of non-exFAT drives */
#endif

    if ((drv->StatusCode != NOT_SUPPORT_FS) && !((drv->FatRead == NullFatRead) && (drv->FatWrite == NullFatWrite)))  /* a well-added Drive[] entry */
        DriveCounter--;

    MpMemSet(drv, 0, sizeof(DRIVE));
    drv->CacheBufPoint = NULL;
    drv->FatCacheBuffer = NULL;
    drv->DirStackBuffer = NULL;
    drv->DirCacheBuffer = NULL;
    drv->FatCachePoint = 0xffffffff;
    drv->DirCachePoint = 0xffffffff;
    drv->bSectorExp = 9;	// initial value for 512 bytes
    drv->FatRead = NullFatRead;
    drv->FatWrite = NullFatWrite;
    drv->DevID = phyDevID; /* device ID */
    drv->DrvIndex = drv_index_ID; /* drive index ID */
    drv->StatusCode = NOT_SUPPORT_FS;

    /* For non-storage or non-FAT devices, always set drv->Flag.Present as 1 for passing card present check */
    if ( ((drv->DevID == DEV_USB_HOST_PTP) ||\
          (drv->DevID == DEV_USBOTG1_HOST_PTP)) ||\
          (drv->DevID == DEV_USB_WIFI_DEVICE) ||\
          (drv->DevID == DEV_CF_ETHERNET_DEVICE) ||\
          (drv->DevID == DEV_USB_WEBCAM) ||\
          (drv->DevID == DEV_USB_ETHERNET_DEVICE))
    {
        drv->Flag.Present = 1;
    }
    else
        drv->Flag.Present = 0;

#if (SC_USBDEVICE)
    SystemDriveLunInfoChange(drv_index_ID);
#endif

    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv_index_ID); /* release semaphore */

    MP_DEBUG("### exit %s(): DriveCounter = %u \r\n", __FUNCTION__, DriveCounter);
}



///
///@ingroup DRIVE
///@brief   Change current drive to the specific drive index ID.
///
///@param   drv_index_ID      The drive index ID.
///
///@return  The DRIVE handle of current drive.
///
DRIVE *DriveChange(E_DRIVE_INDEX_ID drv_index_ID)
{
    if (drv_index_ID > MAX_DRIVE_NUM) /* note: Drive[MAX_DRIVE_NUM] is the extra DRIVE entry for xpgslidefunc use */
    {
        MP_ALERT("%s: Error! drive index (%s) is out of Drive Table range !", __FUNCTION__, drv_index_ID);
        return NULL;
    }
    else if (drv_index_ID == NULL_DRIVE)
    {
        MP_ALERT("%s: Warning! drive index == NULL_DRIVE !", __FUNCTION__);
    }

    CurrDrive = drv_index_ID;
	if (Drive[CurrDrive].FreeClusters == 0)  /* maybe FAT table of this drive has not been scanned yet */
		DriveFreeClustersCountGet(&Drive[CurrDrive]); /* force to do ScanFat() for FAT12/16/32 drive */
    return &Drive[CurrDrive];
}



///
///@ingroup DRIVE
///@brief   Get the DRIVE handle of specific drive index ID.
///
///@param   drv_index_ID      The drive index ID.
///
///@return  The DRIVE handle of the specific drive index ID.
///
DRIVE *DriveGet(E_DRIVE_INDEX_ID drv_index_ID)
{
    if (drv_index_ID > MAX_DRIVE_NUM) /* note: Drive[MAX_DRIVE_NUM] is the extra DRIVE entry for xpgslidefunc use */
    {
        MP_ALERT("%s: Error! drive index (%s) is out of Drive Table range !", __FUNCTION__, drv_index_ID);
        return NULL;
    }
    else if (drv_index_ID == NULL_DRIVE)
    {
        MP_ALERT("%s: Warning! drive index == NULL_DRIVE !", __FUNCTION__);
    }

    return &Drive[drv_index_ID];
}



///
///@ingroup DRIVE
///@brief   Get drive index ID of current drive.
///
///@param   None.
///
///@return  Drive index ID of current drive.
///
BYTE DriveCurIdGet(void)
{
    return CurrDrive;
}



///
///@ingroup DRIVE
///@brief   Get the total number of well-added drives in the internal Drive table.
///
///@param   None.
///
///@return  Total number of drives.
///
///@remark  DriveCountUpdate() will update 'DriveCounter' value by checking Drive table, and then return 'DriveCounter' value. \n
///         DriveCountGet() will not update 'DriveCounter' value, and return its value directly instead.
///
BYTE DriveCountGet(void)
{
    return DriveCounter;
}



///
///@ingroup DRIVE
///@brief   Update the total number of well-added drives by scanning the internal Drive table.
///
///@param   None.
///
///@return  Total number of drives.
///
///@remark  DriveCountUpdate() will update 'DriveCounter' value by checking Drive table, and then return 'DriveCounter' value. \n
///         DriveCountGet() will not update 'DriveCounter' value, and return its value directly instead.
///
BYTE DriveCountUpdate(void)
{
    DRIVE *drv;
    BYTE i, well_added_count = 0;

    for (i = 0; i < MAX_DRIVE_NUM; i++)
    {
        drv = (DRIVE *) &Drive[i];

        if ((drv->StatusCode != NOT_SUPPORT_FS) && !((drv->FatRead == NullFatRead) && (drv->FatWrite == NullFatWrite)))  /* a well-added Drive[] entry */
            well_added_count++;
    }

    DriveCounter = well_added_count;

    return DriveCounter;
}



///
///@ingroup DRIVE
///@brief   Get current count of free clusters of the specified drive.
///
///@param   drv      The drive to access.
///
///@return  Current count of free clusters of the specified drive.
///
DWORD DriveFreeClustersCountGet(DRIVE * drv)
{
    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return 0;
    }

    if (! SystemCardPresentCheck(drv->DrvIndex))
    {
        MP_ALERT("%s: Card not present !", __FUNCTION__);
        return 0;
    }

    if (drv->StatusCode == NOT_SUPPORT_FS)
    {
        MP_ALERT("%s: file system on the drive has problem ! Please check it ...", __FUNCTION__);
        return 0;
    }

    if (drv->FreeClusters == 0)  /* maybe FAT table of this drive has not been scanned yet */
    {
#if EXFAT_ENABLE
        if (drv->Flag.FsType == FS_TYPE_exFAT)
        {
            /* Because Allocation Bitmap is mandatory for exFAT, the loading and scanning of exFAT Alocation Bitmap have always been done during DriveAdd().
             * So, no need to do it again here.
             */
        }
        else
#endif
        {
            /* scan FAT for counting free clusters */
            MP_DEBUG("%s: => ScanFat(drv) ...", __FUNCTION__);
            if (ScanFat(drv) != FS_SUCCEED)
                drv->FreeClusters = 0;
            else
                MP_DEBUG("%s: current count of free clusters = %lu", __FUNCTION__, drv->FreeClusters);
        }
    }

    //MP_DEBUG("%s: current count of free clusters = %lu", __FUNCTION__, drv->FreeClusters);
    return drv->FreeClusters;
}



///
///@ingroup DRIVE
///@brief   Get free size of the specified drive (unit : sectors).
///
///@param   drv      The drive to access.
///
///@return  Free size of the specified drive (unit : sectors).
///
DWORD DriveFreeSizeGet(DRIVE * drv)
{
    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return 0;
    }

    return (DriveFreeClustersCountGet(drv) << drv->ClusterExp);
}



///
///@ingroup DRIVE
///@brief   Get sector size of the specified drive (unit : bytes).
///
///@param   drv      The drive to access.
///
///@return  Sector size of the specified drive (unit : bytes).
///
DWORD DriveSetcorSizeGet(DRIVE * drv)
{
    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return 0;
    }

    if (! SystemCardPresentCheck(drv->DrvIndex))
    {
        MP_ALERT("%s: Card not present !", __FUNCTION__);
        return 0;
    }

    return (1 << drv->bSectorExp);
}



///
///@ingroup DRIVE
///@brief   Get total size of the specified drive (unit : sectors).
///
///@param   drv      The drive to access.
///
///@return  Total size of the specified drive (unit : sectors).
///
DWORD DriveTotalSizeGet(DRIVE * drv)
{
    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return 0;
    }

    if (! SystemCardPresentCheck(drv->DrvIndex))
    {
        MP_ALERT("%s: Card not present !", __FUNCTION__);
        return 0;
    }

    return (drv->TotalClusters << drv->ClusterExp);
}



#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup DRIVE
///@brief   Get/allocate a free cluster on the drive.
///
///@param   drv      The drive to access.
///
///@return  It will return cluster number of the allocated cluster. If no free cluster available, it will return 0xffffffff.
///
#endif
DWORD DriveNewClusGet(DRIVE * drv)
{
    DWORD tmpLastCluster;
    BYTE  bitmap_bit_value;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return 0xffffffff;
    }

    if (! SystemCardPresentCheck(drv->DrvIndex))
    {
        MP_ALERT("%s: Card not present !", __FUNCTION__);
        return 0xffffffff;
    }

#if FS_REENTRANT_API
    /* check whether if this drive handle is an external working drive copy */
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);
    BYTE curr_task_id = TaskGetId();

    if (drv != ori_drv_in_drive_table) /* the input 'drv' is an external working drive copy */
    {
        /* check whether if previous FAT entries updating process by another task is still not finished */
        while ( (ori_drv_in_drive_table->CacheBufPoint->f_IsUpdatingFatEntries) &&
                (ori_drv_in_drive_table->CacheBufPoint->TaskId_of_UpdatingFatEntries != curr_task_id) )
        {
            MP_DEBUG("%s: task ID %d is waiting FAT update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, ori_drv_in_drive_table->CacheBufPoint->TaskId_of_UpdatingFatEntries);
            TaskSleep(10); /* force task context switch to let another task to finish its process first */
        }

//        /* check whether if FSInfo free cluster info in the original drive table entry has been changed */
//        if ((drv->LastCluster != ori_drv_in_drive_table->LastCluster) || (drv->FreeClusters != ori_drv_in_drive_table->FreeClusters))
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
#endif

#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
    SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv->DrvIndex);
#endif

    if (DriveFreeClustersCountGet(drv) == 0)
    {
        MP_ALERT("%s: Warning! No free cluster available on the drive !", __FUNCTION__);
    #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv->DrvIndex);
    #endif
        return 0xffffffff;
    }

    tmpLastCluster = drv->LastCluster;

    while (1)
    {
        if (! SystemCardPresentCheck(drv->DrvIndex))
        {
            MP_ALERT("%s: Card not present !", __FUNCTION__);
        #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
            SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv->DrvIndex);
        #endif
            return 0xffffffff;
        }

        drv->LastCluster++;

        if (drv->LastCluster >= drv->TotalClusters + 2)	//if cluster number overflow, rewind to head. It's a circular counter
            drv->LastCluster = 2;

        if (drv->Flag.FsType != FS_TYPE_exFAT)
        {
    #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
            if (drv->Flag.AllocBitmapReady)
            {
                bitmap_bit_value = Read_AllocBitmapEntry(drv, drv->LastCluster);
                if (bitmap_bit_value == 0x00) // free cluster in Allocation Bitmap entry found
                    break;
                else
                {
                    if (bitmap_bit_value != 0x01) // => means Read_AllocBitmapEntry() failed
                    {
                        MP_ALERT("%s: Error! Read_AllocBitmapEntry() failed => Please check the drive !", __FUNCTION__);
                        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv->DrvIndex);
                        return 0xffffffff;
                    }
                }
            }
            else
            {
                MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
                SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv->DrvIndex);
                return 0xffffffff;
            }
    #else
            if (!drv->FatRead(drv, drv->LastCluster)) // free cluster (=0x00) in FAT entry found
                break;
            else
            {
                if (drv->StatusCode == FS_SCAN_FAIL)
                {
                    MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                    return 0xffffffff;
                }
            }
    #endif
        }
#if EXFAT_ENABLE
        else
        {
            bitmap_bit_value = Read_AllocBitmapEntry(drv, drv->LastCluster);
            if (bitmap_bit_value == 0x00) // free cluster in Allocation Bitmap entry found
                break;
            else
            {
                if (bitmap_bit_value != 0x01) // => means Read_AllocBitmapEntry() failed
                {
                    MP_ALERT("%s: Error! Read_AllocBitmapEntry() failed => Please check the drive !", __FUNCTION__);
    #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
                    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv->DrvIndex);
    #endif
                    return 0xffffffff;
                }
            }
        }
#endif

        if (tmpLastCluster == drv->LastCluster)	// all FAT entries was scanned, and no free cluster => set FreeClusters to zero
        {
            drv->FreeClusters = 0;
            MP_ALERT("%s: Warning! After checking whole FAT table, no free cluster available on the drive !", __FUNCTION__);
    #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
            SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv->DrvIndex);
    #endif
            return 0xffffffff;
        }
    }

#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
    /* allocate the cluster in Allocation Bitmap first */
    if (Set_AllocBitmapEntry(drv, drv->LastCluster, 1) != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! Set_AllocBitmapEntry() failed => Please check the drive !", __FUNCTION__);
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv->DrvIndex);
        return 0xffffffff;
    }

  #if EXFAT_ENABLE
    if (drv->Flag.FsType == FS_TYPE_exFAT)
        drv->exFAT_InfoFileds->VolumeFlags.VolumeDirty = 1; //set VolumeDirty
  #endif
#endif

    drv->FreeClusters--;

#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + drv->DrvIndex);
#endif

    return drv->LastCluster;
}



///
///@ingroup DRIVE
///@brief   Quick format a drive.
///
///@param   drv      The drive to format.
///
///@retval  FS_SUCCEED           Format successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL    Format unsuccessfully. \n\n
///@retval  INVALID_DRIVE        Invalid drive.
///
///@remark  The function call only quick format the drive, meaning that it only clear FAT table and root directory (file info if FAT32).\n
///         If the drive is not formatted yet, quick format will not work.
///
int QuickFormat(DRIVE * drv)
{
    DWORD temp;
    BYTE len;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (! SystemCardPresentCheck(drv->DrvIndex))
    {
        MP_ALERT("%s: Card not present !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

    /* note: add pointer check for safety ! Because MpMemSet() may cause "break 100" when memory buffer was already released first by Card Out, for example. */
    if ((BYTE *) drv->FatCacheBuffer != NULL)
        MpMemSet((BYTE *) drv->FatCacheBuffer, 0, FAT_CACHE_BYTE_SIZE); // clear the FAT cache of this drive
    if ((BYTE *) drv->DirCacheBuffer != NULL)
        MpMemSet((BYTE *) drv->DirCacheBuffer, 0, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT); // clear the Dir cache of this drive

    drv->Flag.FatCacheChanged = 0;
    drv->Flag.DirCacheChanged = 0;
    drv->FatCachePoint = 0xffffffff;
    drv->DirCachePoint = 0xffffffff;

    // clear all the sectors from FAT start to Data start
    if (SectorClear(drv, drv->FatStart, drv->DataStart - drv->FatStart) != FS_SUCCEED)
    {
        MP_ALERT("%s: SectorClear() failed !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

    if (drv->Flag.FsType == FS_TYPE_FAT32) // clear root directory when FAT32
    {
        //note: for FAT32, drv->RootStart is cluster number of the first cluster of root directory
        temp = drv->RootStart; // get root cluster
        drv->FatWrite(drv, temp, 0xffffffff);
        drv->FreeClusters = drv->TotalClusters - 1;
        drv->LastCluster = temp;
        temp = drv->DataStart + ((temp - 2) << drv->ClusterExp); // convert to LBA
        if (SectorClear(drv, temp, drv->ClusterSize) != FS_SUCCEED) // clear root directory sectors (a cluster range)
        {
            MP_ALERT("%s: SectorClear() failed !", __FUNCTION__);
            return DRIVE_ACCESS_FAIL;
        }
    }
    else if ((drv->Flag.FsType == FS_TYPE_FAT12) || (drv->Flag.FsType == FS_TYPE_FAT16))
    {
        drv->FreeClusters = drv->TotalClusters;
    }
#if EXFAT_ENABLE
    else if (drv->Flag.FsType == FS_TYPE_exFAT)
    {
    #if EXFAT_WRITE_ENABLE
        MP_ALERT("%s: To-Do: how to process exFAT Write operations ??", __FUNCTION__);
        return DRIVE_ACCESS_FAIL; /* return error temporarily */
    #else
        MP_ALERT("%s: -I- exFAT Write operations are not supported !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    #endif
    }
#endif

    // re-build the FAT
    drv->FatWrite(drv, 0, CLUSTER0_FAT32);
    drv->FatWrite(drv, 1, EOC_FAT32);
    if (DriveWrite(drv, (BYTE *) drv->FatCacheBuffer, drv->FatCachePoint + drv->FatStart, 1) != FS_SUCCEED)
    {
        MP_ALERT("%s: DriveWrite() failed !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }
    if (drv->NumberOfFat > 1)
    {
        if (DriveWrite(drv, (BYTE *) drv->FatCacheBuffer, drv->FatCachePoint + drv->FatStart + drv->FatSize, 1) != FS_SUCCEED)
        {
            MP_ALERT("%s: DriveWrite() failed !", __FUNCTION__);
            return DRIVE_ACCESS_FAIL;
        }
    }

    drv->Flag.FatCacheChanged = 0;

    if (DriveRefresh(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: DriveRefresh() failed !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }
    else
        return FS_SUCCEED;
}



///
///@ingroup DRIVE
///@brief   Write back the FAT cache data, directory and FSINFO file info sector to the drive storage.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED          Write back successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL   Write back unsuccessfully. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
///@remark  The function call writes back the FAT cache data and directory data based on the drv->Flag.FatCacheChanged and
///         drv->Flag.DirCacheChanged flags.\n
///         If the drive is FAT32, it will also write back FSINFO file info sector.
///
int DriveRefresh(DRIVE * drv)
{
    DWORD *point;
    BYTE  *buffer;
    BYTE  len;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (! SystemCardPresentCheck(drv->DrvIndex))
    {
        MP_ALERT("%s: Card not present !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

#if FS_REENTRANT_API
    BOOL f_Is_ext_working_drv;
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);
    TASK_WORKING_DRIVE_TYPE *list_ptr;
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

        CHAIN *dir_of_working_drv, *dir_of_ori_drv;

        dir_of_working_drv = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
        dir_of_ori_drv = (CHAIN *) ((BYTE *) ori_drv_in_drive_table->DirStackBuffer + ori_drv_in_drive_table->DirStackPoint);

        /* check whether if previous FDB nodes allocation process by another task is still not finished */
        while ( (ori_drv_in_drive_table->CacheBufPoint->f_IsUpdatingFdbNodes) &&
                (ori_drv_in_drive_table->CacheBufPoint->TaskId_of_UpdatingFdbNodes != curr_task_id) )
        {
            MP_DEBUG("%s: task ID %d is waiting DirCache update (by other task ID = %d) to finish ...", __FUNCTION__, curr_task_id, ori_drv_in_drive_table->CacheBufPoint->TaskId_of_UpdatingFdbNodes);
            TaskSleep(10); /* force task context switch to let another task to finish its process first */
        }
    }
#endif

    if (drv->Flag.FatCacheChanged)
    {
/* note: must sync FAT cache buffer content before DriveWrite() I/O because DriveWrite() will cause task context switch !! */
#if FS_REENTRANT_API

        Set_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        {
            /* sync current FSInfo free cluster info into original drive table entry */
            ori_drv_in_drive_table->LastCluster = drv->LastCluster;
            ori_drv_in_drive_table->FreeClusters = drv->FreeClusters;

            /* sync FAT cache buffer content */
            /* sync: copy from working drive to ori drive */
            MpMemCopy((BYTE *) ori_drv_in_drive_table->FatCacheBuffer, (BYTE *) drv->FatCacheBuffer, FAT_CACHE_BYTE_SIZE);
            ori_drv_in_drive_table->FatCachePoint = drv->FatCachePoint;

            /* sync: copy from working drive to all other tasks' working drives w.r.t same storage drive */
            list_ptr = Tasks_WorkingDrv_List;
            while (list_ptr != NULL)
            {
                if ((list_ptr->TaskId != curr_task_id) && (list_ptr->Work_Drv) && (list_ptr->Work_Drv->DrvIndex == drv->DrvIndex))
                {
                    /* sync FAT cache content of working drives only when their cache points of FAT sectors are the same */
                    if (list_ptr->Work_Drv->FatCachePoint == drv->FatCachePoint)
                    {
                        MpMemCopy((BYTE *) list_ptr->Work_Drv->FatCacheBuffer, (BYTE *) drv->FatCacheBuffer, FAT_CACHE_BYTE_SIZE);
                    }
                }

                list_ptr = list_ptr->next;
            }
        }
#endif

        if ((drv->FatSize - drv->FatCachePoint) > (1 << FAT_CACHE_SIZE_IN_SECTORS_EXP))
            len = (1 << FAT_CACHE_SIZE_IN_SECTORS_EXP);
        else
            len = drv->FatSize - drv->FatCachePoint;

        if (DriveWrite(drv, (BYTE *) drv->FatCacheBuffer, drv->FatCachePoint + drv->FatStart, len) != FS_SUCCEED)
        {
            MP_ALERT("DriveRefresh(): DriveWrite() failed !!!");

#if FS_REENTRANT_API
            Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

            return DRIVE_ACCESS_FAIL;
        }

        if (drv->NumberOfFat > 1)
        {
            if (DriveWrite(drv, (BYTE *) drv->FatCacheBuffer, drv->FatCachePoint + drv->FatStart + drv->FatSize, len) != FS_SUCCEED)
            {
                MP_ALERT("DriveRefresh(): DriveWrite() failed !!!");

#if FS_REENTRANT_API
            Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

                return DRIVE_ACCESS_FAIL;
            }
        }

        drv->Flag.FatCacheChanged = 0;

#if FS_REENTRANT_API
            Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif
    }

    if (drv->Flag.DirCacheChanged)
    {
/* note: must sync Dir cache buffer content before DriveWrite() I/O because DriveWrite() will cause task context switch !! */
#if FS_REENTRANT_API
        Set_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Set_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        {
            /* sync Dir cache buffer content */
            /* sync: copy from working drive to ori drive */
            MpMemCopy((BYTE *) ori_drv_in_drive_table->DirCacheBuffer, (BYTE *) drv->DirCacheBuffer, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT);
            ori_drv_in_drive_table->DirCachePoint = drv->DirCachePoint;

            /* sync: copy from working drive to all other tasks' working drives w.r.t same storage drive if same directory is cached */
            list_ptr = Tasks_WorkingDrv_List;
            while (list_ptr != NULL)
            {
                if ((list_ptr->TaskId != curr_task_id) && (list_ptr->Work_Drv) && (list_ptr->Work_Drv->DrvIndex == drv->DrvIndex))
                {
                    /* sync directory cache content of working drives only when their cache points of directory sectors are the same */
                    if (list_ptr->Work_Drv->DirCachePoint == drv->DirCachePoint)
                    {
                        MpMemCopy((BYTE *) list_ptr->Work_Drv->DirCacheBuffer, (BYTE *) drv->DirCacheBuffer, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT);
                    }
                }

                list_ptr = list_ptr->next;
            }
        }
#endif

        if (DriveWrite(drv, (BYTE *) drv->DirCacheBuffer, drv->DirCachePoint, 1) != FS_SUCCEED)
        {
            MP_ALERT("DriveRefresh(): DriveWrite() failed !!!");

#if FS_REENTRANT_API
            Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

            return DRIVE_ACCESS_FAIL;
        }

        drv->Flag.DirCacheChanged = 0;

#if FS_REENTRANT_API
        Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif
    }

    // write the FS INFO sector of FAT32
    if ((drv->Flag.FsType == FS_TYPE_FAT32) && (drv->FsInfoAddr != 0xffffffff))
    {
        buffer = (BYTE *) ker_mem_malloc(1 << drv->bSectorExp, TaskGetId());
        if (buffer == NULL)
        {
            MP_ALERT("%s: Error: ker_mem_malloc() failed for size %d bytes !", __FUNCTION__, (1 << drv->bSectorExp));
            return DRIVE_ACCESS_FAIL;
        }

        point = (DWORD *) buffer;
        MpMemSet(buffer, 0, 1 << drv->bSectorExp);

#if FS_REENTRANT_API
        Set_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        *point = 0x52526141;
        point = point + FSINFO_OFFSET / 4;
        *point = FSINFO_SIGNATURE;
        point++;
        SaveAlien32(point, drv->FreeClusters);
        point++;
        SaveAlien32(point, drv->LastCluster);
        point += 4;
        *point = 0x000055AA;

        if (DriveWrite(drv, buffer, drv->FsInfoAddr, 1) != FS_SUCCEED)
        {
            ker_mem_free(buffer);
            MP_ALERT("DriveRefresh(): DriveWrite() failed !!!");

#if FS_REENTRANT_API
            Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

            return DRIVE_ACCESS_FAIL;
        }

        ker_mem_free(buffer);
    }

#if FS_REENTRANT_API
    Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

#if EXFAT_ENABLE
    if (drv->Flag.FsType == FS_TYPE_exFAT)
    {
        if (drv->exFAT_InfoFileds->VolumeFlags.VolumeDirty)
        {
    #if EXFAT_WRITE_ENABLE
            //To-Do: when and how to write back Allocation Bitmap to disk ?? If so, this writing back may occurred too frequently ??
            MP_ALERT("%s: To-Do: when and how to write back Allocation Bitmap to disk ??");

            MP_ALERT("%s: To-Do: how to process exFAT Write operations ??", __FUNCTION__);
            return DRIVE_ACCESS_FAIL; /* return error temporarily */

            drv->exFAT_InfoFileds->VolumeFlags.VolumeDirty = 0; //clear VolumeDirty
    #else
            MP_ALERT("%s: -I- exFAT Write operations are not supported !", __FUNCTION__);
            return DRIVE_ACCESS_FAIL;
    #endif
        }
    }
#endif

    return FS_SUCCEED;
}



#define TMPBUF_SECTORS_CNT  32   /* 32 sectors length buffer for DMA transfer to reduce time */

///
///@ingroup DRIVE
///@brief   Write the buffer data to the specified sector(s) on the drive.
///
///@param   drv      The drive to access. \n\n
///@param   buffer   Buffer of data to be written. \n\n
///@param   lba      The lba (logical block address) of sector to write. \n\n
///@param   len      The length of data to write (unit : sectors).
///
///@retval  FS_SUCCEED        Write successfully. \n\n
///@retval  ABNORMAL_STATUS   Write unsuccessfully due to some error. \n\n
///@retval  DISK_READ_ONLY    Write unsuccessfully due to the drive is read-only.
///@retval  INVALID_DRIVE     Invalid drive.
///
/* rename original DriveWrite() to __DriveWrite(), and add DriveWrite() wrapper to consider not in DMA 32-bit address boundary case */
int DriveWrite(DRIVE * drv, BYTE * buffer, DWORD lba, DWORD len)
{
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (buffer == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (! SystemCardPresentCheck(drv->DrvIndex))
    {
        MP_ALERT("%s: Card not present !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* check Read-Only flag by underlying Mcard layer info => physical lock latch or Mcard layer lock */
    if (SystemGetFlagReadOnly(drv->DrvIndex))
    {
        MP_ALERT("%s: -E- The drive is Read-Only (controlled in Mcard layer) !", __FUNCTION__);
        return DISK_READ_ONLY;
    }

    /* check Read-Only flag by file system layer => logical or controlled by S/W */
    if (drv->Flag.ReadOnly)
    {
        MP_ALERT("%s: -E- The drive is Read-Only (controlled in file system layer) !", __FUNCTION__);
    #if EXFAT_ENABLE
        if (drv->Flag.FsType == FS_TYPE_exFAT)
        {
            MP_DEBUG("%s: The drive is exFAT file system.", __FUNCTION__);
        }
    #endif
        return DISK_READ_ONLY;
    }

    if ((DWORD) buffer & 0x0003) //not 32-bit address boundary
    {
        DWORD sectors_left = len, buf_pos = 0, sectors_to_transfer, lba_new = lba;
        BYTE *tmp_buff = (BYTE *) ker_mem_malloc((TMPBUF_SECTORS_CNT << drv->bSectorExp), TaskGetId());
        if (tmp_buff == NULL)
        {
            MP_ALERT("%s: Error: ker_mem_malloc() failed for size %d bytes !", __FUNCTION__, (TMPBUF_SECTORS_CNT << drv->bSectorExp));
            return ABNORMAL_STATUS;
        }

//		tmp_buff = (DWORD)tmp_buff | 0xA0000000;

        while (sectors_left)
        {
            sectors_to_transfer = (sectors_left >= TMPBUF_SECTORS_CNT)? TMPBUF_SECTORS_CNT:sectors_left;
            MpMemSet(tmp_buff, 0, (TMPBUF_SECTORS_CNT << drv->bSectorExp));
            MpMemCopy(tmp_buff, (BYTE *) (buffer + buf_pos), (sectors_to_transfer << drv->bSectorExp));
            buf_pos += (sectors_to_transfer << drv->bSectorExp);
            sectors_left -= sectors_to_transfer;

            if ((ret = Mcard_DeviceWrite(drv, tmp_buff, lba_new + (DWORD) drv->partitionStartLba, sectors_to_transfer)) != FS_SUCCEED)
            {
                MP_ALERT("Error: Mcard_DeviceWrite() failed! ret = 0x%x", ret);
                break;
            }

            lba_new += sectors_to_transfer;
        }

        ker_mem_free((BYTE *)tmp_buff);
    }
    else
    {
        ret = Mcard_DeviceWrite(drv, buffer, lba + (DWORD) drv->partitionStartLba, len);
        if (ret != FS_SUCCEED)
        {
            MP_ALERT("Error: Mcard_DeviceWrite() failed! ret = 0x%x", ret);
        }
    }

    return ret;
}



///
///@ingroup DRIVE
///@brief   Read/load data from the specified sector(s) on the drive to the buffer.
///
///@param   drv      The drive to access. \n\n
///@param   buffer   Buffer to store read data. \n\n
///@param   lba      The lba (logical block address) of sector to read. \n\n
///@param   len      The length of data to read (unit : sectors).
///
///@retval  FS_SUCCEED        Read successfully. \n\n
///@retval  ABNORMAL_STATUS   Read unsuccessfully. \n\n
///@retval  INVALID_DRIVE     Invalid drive.
///
/* rename original DriveRead() to __DriveRead(), and add DriveRead() wrapper to consider not in DMA 32-bit address boundary case */
int DriveRead(DRIVE * drv, BYTE * buffer, DWORD lba, DWORD len)
{
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (buffer == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (! SystemCardPresentCheck(drv->DrvIndex))
    {
        MP_ALERT("%s: Card not present !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if ((DWORD) buffer & 0x0003) //not 32-bit address boundary
    {
        DWORD sectors_left = len, buf_pos = 0, sectors_to_transfer, lba_new = lba;
        BYTE *tmp_buff = (BYTE *) ker_mem_malloc((TMPBUF_SECTORS_CNT << drv->bSectorExp), TaskGetId());
        if (tmp_buff == NULL)
        {
            MP_ALERT("%s: Error: ker_mem_malloc() failed for size %d bytes !", __FUNCTION__, (TMPBUF_SECTORS_CNT << drv->bSectorExp));
            return ABNORMAL_STATUS;
        }

        tmp_buff = (BYTE *) ((DWORD)tmp_buff | 0xA0000000);

        while (sectors_left)
        {
            sectors_to_transfer = (sectors_left >= TMPBUF_SECTORS_CNT)? TMPBUF_SECTORS_CNT:sectors_left;
            MpMemSet(tmp_buff, 0, (TMPBUF_SECTORS_CNT << drv->bSectorExp));
            if ((ret = Mcard_DeviceRead(drv, tmp_buff, lba_new + (DWORD) drv->partitionStartLba, sectors_to_transfer)) != FS_SUCCEED)
            {
                MP_ALERT("Error: Mcard_DeviceRead() failed! ret = 0x%x", ret);
                break;
            }

            MpMemCopy((BYTE *) (buffer + buf_pos), tmp_buff, (sectors_to_transfer << drv->bSectorExp));
            buf_pos += (sectors_to_transfer << drv->bSectorExp);
            sectors_left -= sectors_to_transfer;

            lba_new += sectors_to_transfer;
        }

        ker_mem_free((BYTE *)tmp_buff);
    }
    else
    {
        ret = Mcard_DeviceRead(drv, buffer, lba + (DWORD) drv->partitionStartLba, len);
        if (ret != FS_SUCCEED)
        MP_ALERT("Error: Mcard_DeviceRead() failed! ret = 0x%x", ret);
    }

    return ret;
}



/*
*******************************************************************************
*        LOCAL FUNCTIONS
*******************************************************************************
*/

#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///@ingroup DRIVE
///@brief   Refresh content of the FAT cache buffer of the drive to cover the specified lba of a FAT table sector. \n
///         If the FAT cache buffer content has been changed, write the cached data back to the disk drive first.
///
///@param   drv      The drive to refresh its FAT cache buffer.
///@param   sector   The position offset of a sector which is part of the whole FAT table to the beginning of first FAT table (unit: number of sectors).
///
///@retval  FS_SUCCEED           Refresh successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL    Refresh unsuccessfully. \n\n
///@retval  INVALID_DRIVE        Invalid drive.
///
#endif
int FatCaching(DRIVE * drv, DWORD sector)
{
    DWORD block;
    BYTE len;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (! SystemCardPresentCheck(drv->DrvIndex))
    {
        MP_ALERT("%s: Card not present !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

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

#if 0 //This block causes side-effect !!??  => to be checked...
//        /* check whether if FSInfo free cluster info in the original drive table entry has been changed */
//        if ((drv->LastCluster != ori_drv_in_drive_table->LastCluster) || (drv->FreeClusters != ori_drv_in_drive_table->FreeClusters))
        {
            /* sync current FSInfo free cluster info into the external working drive copy */
            drv->LastCluster = ori_drv_in_drive_table->LastCluster;
            drv->FreeClusters = ori_drv_in_drive_table->FreeClusters;

            /* sync FAT cache buffer content */
            /* sync: copy from ori drive to working drive */
            MpMemCopy((BYTE *) drv->FatCacheBuffer, (BYTE *) ori_drv_in_drive_table->FatCacheBuffer, FAT_CACHE_BYTE_SIZE);
            drv->FatCachePoint = ori_drv_in_drive_table->FatCachePoint;
        }
#endif

    }

    Set_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    block = sector & ~((1 << FAT_CACHE_SIZE_IN_SECTORS_EXP) - 1);

    if (drv->FatCachePoint != block)
    {
        if (drv->Flag.FatCacheChanged)
        {
/* note: must sync FAT cache buffer content before DriveWrite() I/O because DriveWrite() will cause task context switch !! */
#if FS_REENTRANT_API
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            {
                /* sync FAT cache buffer content */
                /* sync: copy from working drive to ori drive */
                MpMemCopy((BYTE *) ori_drv_in_drive_table->FatCacheBuffer, (BYTE *) drv->FatCacheBuffer, FAT_CACHE_BYTE_SIZE);
                ori_drv_in_drive_table->FatCachePoint = drv->FatCachePoint;

                /* sync: copy from working drive to all other tasks' working drives w.r.t same storage drive */
                TASK_WORKING_DRIVE_TYPE *list_ptr = Tasks_WorkingDrv_List;
                while (list_ptr != NULL)
                {
                    if ((list_ptr->TaskId != curr_task_id) && (list_ptr->Work_Drv) && (list_ptr->Work_Drv->DrvIndex == drv->DrvIndex))
                    {
                        /* sync FAT cache content of working drives only when their cache points of FAT sectors are different */
                        if (list_ptr->Work_Drv->FatCachePoint == drv->FatCachePoint)
                        {
                            MpMemCopy((BYTE *) list_ptr->Work_Drv->FatCacheBuffer, (BYTE *) drv->FatCacheBuffer, FAT_CACHE_BYTE_SIZE);
                        }
                    }

                    list_ptr = list_ptr->next;
                }
            }
#endif

            if ((drv->FatSize - drv->FatCachePoint) > (1 << FAT_CACHE_SIZE_IN_SECTORS_EXP))
                len = (1 << FAT_CACHE_SIZE_IN_SECTORS_EXP);
            else
                len = drv->FatSize - drv->FatCachePoint;

            if (DriveWrite(drv, (BYTE *) drv->FatCacheBuffer, drv->FatCachePoint + drv->FatStart, len) != FS_SUCCEED)
            {
                MP_ALERT("%s: DriveWrite() failed !", __FUNCTION__);

#if FS_REENTRANT_API
                Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
                if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                    Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

                return DRIVE_ACCESS_FAIL;
            }

            if (drv->NumberOfFat > 1)
            {
                if (DriveWrite(drv, (BYTE *) drv->FatCacheBuffer, drv->FatCachePoint + drv->FatStart + drv->FatSize, len) != FS_SUCCEED)
                {
                    MP_ALERT("%s: DriveWrite() failed !", __FUNCTION__);

#if FS_REENTRANT_API
                    Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
                    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                        Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

                    return DRIVE_ACCESS_FAIL;
                }
            }
        }

        drv->FatCachePoint = block;
        if (DriveRead(drv, (BYTE *) drv->FatCacheBuffer, block + drv->FatStart, (1 << FAT_CACHE_SIZE_IN_SECTORS_EXP)) != FS_SUCCEED)
        {
            MP_ALERT("%s: DriveRead() failed !", __FUNCTION__);

#if FS_REENTRANT_API
            Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

            return DRIVE_ACCESS_FAIL;
        }

        drv->Flag.FatCacheChanged = 0;
    }

#if FS_REENTRANT_API
    Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    return FS_SUCCEED;
}



static DWORD ReadFat16(void * drv, DWORD cluster)
{
    register DWORD content;
    register BYTE *addr;
    register DRIVE *pDrv = (DRIVE *) drv;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        __asm("break 100"); //this should be impossible! Return type DWORD => no suitable error code to return !
    }

    SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);

    if (FatCaching(pDrv, cluster >> (pDrv->bSectorExp - 1)) != FS_SUCCEED)
    {
        pDrv->StatusCode = FS_SCAN_FAIL;
        MP_ALERT("ReadFat16(drv, cluster = 0x%x): FatCaching(drv, %d) failed !!", cluster, cluster >> (pDrv->bSectorExp - 1));
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return FAT_READ_END_OF_CHAIN;
    }

    addr = (BYTE *) (pDrv->FatCacheBuffer + ((cluster << 1) & ((1 << (pDrv->bSectorExp + FAT_CACHE_SIZE_IN_SECTORS_EXP)) - 1)));
    content = (DWORD) LoadAlien16(addr);

    if (content >= 0xfff8)
        content = FAT_READ_END_OF_CHAIN;

    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
    return content;
}



static DWORD ReadFat12(void * drv, DWORD cluster)
{
    register WORD byte_offset;
    register DWORD sector, content;
    register BYTE *point;
    register DRIVE *pDrv = (DRIVE *) drv;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        __asm("break 100"); //this should be impossible! Return type DWORD => no suitable error code to return !
    }

    SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);

    byte_offset = cluster + (cluster >> 1);	// byte_offset = cluster * 1.5
    sector = byte_offset >> pDrv->bSectorExp;
    byte_offset = byte_offset & ((1 << (pDrv->bSectorExp + FAT_CACHE_SIZE_IN_SECTORS_EXP)) - 1);

    if (FatCaching(pDrv, sector) != FS_SUCCEED)
    {
        pDrv->StatusCode = FS_SCAN_FAIL;
        MP_ALERT("ReadFat12(drv, cluster = 0x%x): FatCaching(drv, %d) failed !!", cluster, sector);
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return FAT_READ_END_OF_CHAIN;
    }

    point = (BYTE *) (pDrv->FatCacheBuffer + byte_offset);
    content = (DWORD) *point;

    byte_offset++;
    point++;

    if (byte_offset == (1 << (pDrv->bSectorExp + FAT_CACHE_SIZE_IN_SECTORS_EXP)))
    {
        sector++;
        if (FatCaching(pDrv, sector) != FS_SUCCEED)
        {
            pDrv->StatusCode = FS_SCAN_FAIL;
            SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
            return FAT_READ_END_OF_CHAIN;
        }

        //FatCaching((DRIVE *)drv, sector);
        point = (BYTE *) (pDrv->FatCacheBuffer);
    }

    content += *point << 8;

    // if cluster is odd then right shift a nibble
    // else then clear the fourth nibble
    if (cluster & 1)
        content >>= 4;
    else
        content &= 0x0fff;

    if (content >= 0x0ff8)
        content = FAT_READ_END_OF_CHAIN;

    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
    return content;
}



static DWORD ReadFat32(void * drv, DWORD cluster)
{
    register DWORD content;
    register BYTE *addr;
    register DRIVE *pDrv = (DRIVE *) drv;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        __asm("break 100"); //this should be impossible! Return type DWORD => no suitable error code to return !
    }

    SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);

    if (FatCaching(pDrv, cluster >> (pDrv->bSectorExp - 2)) != FS_SUCCEED)
    {
        pDrv->StatusCode = FS_SCAN_FAIL;
        MP_ALERT("ReadFat32(drv, cluster = 0x%x): FatCaching(drv, %d) failed !!", cluster, cluster >> (pDrv->bSectorExp - 2));
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return FAT_READ_END_OF_CHAIN;
    }

    addr = (BYTE *) (pDrv->FatCacheBuffer + ((cluster << 2) & ((1 << (pDrv->bSectorExp + FAT_CACHE_SIZE_IN_SECTORS_EXP)) - 1)));
    content = (DWORD) LoadAlien32(addr);

    if (content >= 0x0ffffff8)
        content = FAT_READ_END_OF_CHAIN;

    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
    return content;
}



static int WriteFat12(void * drv, DWORD cluster, DWORD content)
{
    WORD byte_offset, mask;
    DWORD sector;
    BYTE *point;
    register DRIVE *pDrv = (DRIVE *) drv;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);

    byte_offset = cluster + (cluster >> 1);	// byte_offset = cluster * 1.5
    sector = byte_offset >> pDrv->bSectorExp;
    byte_offset = byte_offset & ((1 << (pDrv->bSectorExp + FAT_CACHE_SIZE_IN_SECTORS_EXP)) - 1);
    content = content & 0xfff; //content (FAT entry value): 0 ~ 0xFFF in FAT12

    if (FatCaching(pDrv, sector) != FS_SUCCEED)
    {
        MP_ALERT("WriteFat12(drv, cluster = 0x%x, content = 0x%x): FatCaching(drv, %d) failed !!", cluster, content, sector);
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return DRIVE_ACCESS_FAIL;
    }

    point = (BYTE *) (pDrv->FatCacheBuffer + byte_offset);

    // if cluster is odd then left shift a nibble
    mask = 0xf000;

    if (cluster & 1)
    {
        content <<= 4;
        mask = 0x000f;
    }

    *point = (*point & mask) | content;
    pDrv->Flag.FatCacheChanged = 1;	// toggle for cache has been updated

    point++;
    byte_offset++;
    mask >>= 8;
    content >>= 8;

    if (byte_offset == (1 << (pDrv->bSectorExp + FAT_CACHE_SIZE_IN_SECTORS_EXP)))
    {
        sector++;
        FatCaching(pDrv, sector);
        point = (BYTE *) pDrv->FatCacheBuffer;
    }

    *point = (*point & mask) | content;
    pDrv->Flag.FatCacheChanged = 1;	// toggle for cache has been updated

#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
    if (pDrv->Flag.AllocBitmapReady)
    {
        /* Sync Allocation Bitmap entry with FAT entry */
        if (content == 0)
            Set_AllocBitmapEntry(pDrv, cluster, 0);
        else
            Set_AllocBitmapEntry(pDrv, cluster, 1);
    }
    else
    {
        MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return DRIVE_ACCESS_FAIL;
    }
#endif

    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
    return FS_SUCCEED;
}



static int WriteFat16(void * drv, DWORD cluster, DWORD content)
{
    BYTE *addr;
    register DRIVE *pDrv = (DRIVE *) drv;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);

    if (FatCaching(pDrv, cluster >> (pDrv->bSectorExp - 1)) != FS_SUCCEED)
    {
        MP_ALERT("WriteFat16(drv, cluster = 0x%x, content = 0x%x): FatCaching(drv, %d) failed !!", cluster, content, cluster >> (pDrv->bSectorExp - 1));
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return DRIVE_ACCESS_FAIL;
    }

    addr = (BYTE *) (pDrv->FatCacheBuffer + ((cluster << 1) & ((1 << (pDrv->bSectorExp + FAT_CACHE_SIZE_IN_SECTORS_EXP)) - 1)));
    SaveAlien16(addr, (WORD) content);
    pDrv->Flag.FatCacheChanged = 1;	// toggle for cache has been updated

#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
    if (pDrv->Flag.AllocBitmapReady)
    {
        /* Sync Allocation Bitmap entry with FAT entry */
        if (content == 0)
            Set_AllocBitmapEntry(pDrv, cluster, 0);
        else
            Set_AllocBitmapEntry(pDrv, cluster, 1);
    }
    else
    {
        MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return DRIVE_ACCESS_FAIL;
    }
#endif

    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
    return FS_SUCCEED;
}



static int WriteFat32(void * drv, DWORD cluster, DWORD content)
{
    BYTE *addr;
    register DRIVE *pDrv = (DRIVE *) drv;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);

    content &= 0x0fffffff;

    if (FatCaching(pDrv, cluster >> (pDrv->bSectorExp - 2)) != FS_SUCCEED)
    {
        MP_ALERT("WriteFat32(drv, cluster = 0x%x, content = 0x%x): FatCaching(drv, %d) failed !!", cluster, content, cluster >> (pDrv->bSectorExp - 2));
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return DRIVE_ACCESS_FAIL;
    }

    addr = (BYTE *) (pDrv->FatCacheBuffer + ((cluster << 2) & ((1 << (pDrv->bSectorExp + FAT_CACHE_SIZE_IN_SECTORS_EXP)) - 1)));
    SaveAlien32(addr, content);
    pDrv->Flag.FatCacheChanged = 1;	// toggle for cache has been updated

#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
    if (pDrv->Flag.AllocBitmapReady)
    {
        /* Sync Allocation Bitmap entry with FAT entry */
        if (content == 0)
            Set_AllocBitmapEntry(pDrv, cluster, 0);
        else
            Set_AllocBitmapEntry(pDrv, cluster, 1);
    }
    else
    {
        MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return DRIVE_ACCESS_FAIL;
    }
#endif

    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
    return FS_SUCCEED;
}



#if EXFAT_ENABLE
static DWORD Read_exFAT(void * drv, DWORD cluster)
{
    register DWORD content;
    register BYTE *addr;
    register DRIVE *pDrv = (DRIVE *) drv;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        __asm("break 100"); //this should be impossible! Return type DWORD => no suitable error code to return !
    }

    if (pDrv->Flag.FsType != FS_TYPE_exFAT)
    {
        MP_ALERT("%s: Error! The drive is not exFAT file system !", __FUNCTION__);
        __asm("break 100"); //this should be impossible! Return type DWORD => no suitable error code to return !
    }

    SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);

    if (FatCaching(pDrv, cluster >> (pDrv->bSectorExp - 2)) != FS_SUCCEED)
    {
        pDrv->StatusCode = FS_SCAN_FAIL;
        MP_ALERT("ReadFat32(drv, cluster = 0x%x): FatCaching(drv, %d) failed !!", cluster, cluster >> (pDrv->bSectorExp - 2));
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return FAT_READ_END_OF_CHAIN;
    }

    addr = (BYTE *) (pDrv->FatCacheBuffer + ((cluster << 2) & ((1 << (pDrv->bSectorExp + FAT_CACHE_SIZE_IN_SECTORS_EXP)) - 1)));
    content = (DWORD) LoadAlien32(addr);

    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
    return content;
}



static int Write_exFAT(void * drv, DWORD cluster, DWORD content)
{
    BYTE *addr;
    register DRIVE *pDrv = (DRIVE *) drv;
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (pDrv->Flag.FsType != FS_TYPE_exFAT)
    {
        MP_ALERT("%s: Error! The drive is not exFAT file system !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    SemaphoreWait(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);

    content &= EXFAT_CLUSTER_END;

    if (FatCaching(pDrv, cluster >> (pDrv->bSectorExp - 2)) != FS_SUCCEED)
    {
        MP_ALERT("WriteFat32(drv, cluster = 0x%x, content = 0x%x): FatCaching(drv, %d) failed !!", cluster, content, cluster >> (pDrv->bSectorExp - 2));
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return DRIVE_ACCESS_FAIL;
    }

    addr = (BYTE *) (pDrv->FatCacheBuffer + ((cluster << 2) & ((1 << (pDrv->bSectorExp + FAT_CACHE_SIZE_IN_SECTORS_EXP)) - 1)));
    SaveAlien32(addr, content);
    pDrv->Flag.FatCacheChanged = 1;	// toggle for cache has been updated

    if (pDrv->Flag.AllocBitmapReady)
    {
        /* Sync Allocation Bitmap entry with FAT entry */
        if (content == EXFAT_CLUSTER_FREE)
            Set_AllocBitmapEntry(pDrv, cluster, 0);
        else
            Set_AllocBitmapEntry(pDrv, cluster, 1);

        pDrv->exFAT_InfoFileds->VolumeFlags.VolumeDirty = 1; //set VolumeDirty
    }
    else
    {
        MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
        SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
        return DRIVE_ACCESS_FAIL;
    }

    SemaphoreRelease(DRIVE_FAT_CACHE_SEMA_ID_BASE + pDrv->DrvIndex);
    return ret;
}
#endif



/* Scan FAT table for counting total number of free clusters.
 * Note: This function is only suitable for FAT12/16/32 file systems to count free clusters. For exFAT file system, Allocation Bitmap
 *       mechanism is introduced for such purpose. Use another function ScanAllocBitmap_FreeClusters() for counting total number of
 *       free clusters on an exFAT drive.
 */
static int ScanFat(DRIVE * drv)
{
    DWORD cluster;
    DWORD ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (! SystemCardPresentCheck(drv->DrvIndex))
    {
        MP_ALERT("%s: Card not present !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

    if (drv->Flag.FsType == FS_TYPE_exFAT)
    {
        MP_ALERT("%s: Error! This function cannot be used for exFAT drive !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    MP_DEBUG("Enter %s(drv)..., drv->DevID = %d, drv->partitionStartLba = %d, drv->TotalClusters = %d", __FUNCTION__, drv->DevID, drv->partitionStartLba, drv->TotalClusters);

    /* initialize counter values */
    drv->FreeClusters = 0;
    drv->LastCluster = 0;
    drv->Flag.AllocBitmapReady = 0;

/* mimic exFAT's Allocation Bitmap for non-exFAT drives */
#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
    DWORD  size_of_AllocBitmap;
    BYTE   *alloc_bitmap_buff = NULL;

    size_of_AllocBitmap = (drv->TotalClusters / 8);
    if (drv->TotalClusters % 8)
        size_of_AllocBitmap++;

    /* Note 1: check drive present status because DriveDelete() will be called when Card Out occurrs, and then causes
     *         memory of drv->AllocBitmap_for_NonExFAT be released.
     * Note 2: MpMemSet() a memory buffer which was already freed may lead "break 100" in MpMemSet() code !
     */

    if ((drv->AllocBitmap_for_NonExFAT != NULL) && SystemCardPresentCheck(drv->DrvIndex))
    {
        /* release old AllocBitmap content */
        ext_mem_free(drv->AllocBitmap_for_NonExFAT);
        drv->AllocBitmap_for_NonExFAT = NULL;
    }

    if ((drv->AllocBitmap_for_NonExFAT == NULL) && SystemCardPresentCheck(drv->DrvIndex))
    {
        /* note: kernel memory is not very enough, use ext_mem_malloc/ext_mem_free for AllocBitmap of non-exFAT drives */
        alloc_bitmap_buff = (BYTE *) ext_mem_malloc(size_of_AllocBitmap);
        if (alloc_bitmap_buff == NULL)
        {
            MP_ALERT("%s: Error: memory allocation failed for size %lu bytes !", __FUNCTION__, size_of_AllocBitmap);
            drv->StatusCode = NOT_SUPPORT_FS;
            return ABNORMAL_STATUS;
        }
        MpMemSet(alloc_bitmap_buff, 0, size_of_AllocBitmap);  /* clear AllocBitmap first */
        MP_DEBUG("%s: AllocBitmap memory allocated (size %lu bytes) ", __FUNCTION__, size_of_AllocBitmap);
        drv->AllocBitmap_for_NonExFAT = alloc_bitmap_buff;
    }
#endif

/* use bigger buffer for FAT sectors content to speed up ScanFat() */
#define TEMP_FAT_BUFFER_SIZE  (150 * 1024)  /* 150 KB, to consider FAT12 whose each FAT entry is 1.5 bytes width */
    BYTE   *temp_FAT_buffer;
    DWORD  covered_FAT_sectors = 0;
    WORD   buff_sectors_count = (TEMP_FAT_BUFFER_SIZE >> drv->bSectorExp);
    DWORD  beginning_cluster, ending_cluster;
    DWORD  content;
    BYTE   *addr;

    temp_FAT_buffer = (BYTE *) ext_mem_malloc(TEMP_FAT_BUFFER_SIZE);
    if (temp_FAT_buffer == NULL)
    {
        MP_ALERT("%s: mem alloc failed for %u bytes !", __FUNCTION__, TEMP_FAT_BUFFER_SIZE);
        return ABNORMAL_STATUS;
    }

    covered_FAT_sectors = 0;
    while (covered_FAT_sectors < drv->FatSize)
    {
        if (! SystemCardPresentCheck(drv->DrvIndex))
        {
            MP_ALERT("%s: Card not present !", __FUNCTION__);
            ext_mem_free(temp_FAT_buffer);
            return DRIVE_ACCESS_FAIL;
        }

        MpMemSet(temp_FAT_buffer, 0, TEMP_FAT_BUFFER_SIZE);
        if (DriveRead(drv, (BYTE *) temp_FAT_buffer, drv->FatStart + covered_FAT_sectors, buff_sectors_count) != FS_SUCCEED)
        {
            MP_ALERT("%s: -E- DriveRead() failed !", __FUNCTION__);
            ext_mem_free(temp_FAT_buffer);
            return DRIVE_ACCESS_FAIL;
        }

        if (drv->Flag.FsType == FS_TYPE_FAT32)
        {
            beginning_cluster = ((covered_FAT_sectors / buff_sectors_count) * TEMP_FAT_BUFFER_SIZE) >> 2;
            ending_cluster = (((covered_FAT_sectors / buff_sectors_count) + 1) * TEMP_FAT_BUFFER_SIZE) >> 2;
        }
        else if (drv->Flag.FsType == FS_TYPE_FAT16)
        {
            beginning_cluster = ((covered_FAT_sectors / buff_sectors_count) * TEMP_FAT_BUFFER_SIZE) >> 1;
            ending_cluster = (((covered_FAT_sectors / buff_sectors_count) + 1) * TEMP_FAT_BUFFER_SIZE) >> 1;
        }
        else if (drv->Flag.FsType == FS_TYPE_FAT12)
        {
            beginning_cluster = ((covered_FAT_sectors / buff_sectors_count) * TEMP_FAT_BUFFER_SIZE) * 2 / 3;
            ending_cluster = (((covered_FAT_sectors / buff_sectors_count) + 1) * TEMP_FAT_BUFFER_SIZE) * 2 / 3;
        }

        /* read each FAT entry value */
        for (cluster = beginning_cluster; (cluster < ending_cluster) && (cluster < drv->TotalClusters + 2); cluster++)
        {
            if (cluster < 2)
                continue;

            if (drv->Flag.FsType == FS_TYPE_FAT32)
            {
                addr = (BYTE *) (temp_FAT_buffer + ((cluster - beginning_cluster) << 2));
                content = (DWORD) LoadAlien32(addr);
                if (content >= 0x0ffffff8)
                    content = FAT_READ_END_OF_CHAIN;
            }
            else if (drv->Flag.FsType == FS_TYPE_FAT16)
            {
                addr = (BYTE *) (temp_FAT_buffer + ((cluster - beginning_cluster) << 1));
                content = (DWORD) LoadAlien16(addr);
                if (content >= 0xfff8)
                    content = FAT_READ_END_OF_CHAIN;
            }
            else if (drv->Flag.FsType == FS_TYPE_FAT12)
            {
                WORD byte_offset;

                byte_offset = (cluster - beginning_cluster) + ((cluster - beginning_cluster) >> 1);	// byte_offset = cluster * 1.5

                addr = (BYTE *) (temp_FAT_buffer + byte_offset);
                content = (DWORD) *addr;

                byte_offset++;
                addr++;

                content += *addr << 8;

                // if cluster is odd then right shift a nibble
                // else then clear the fourth nibble
                if (cluster & 1)
                    content >>= 4;
                else
                    content &= 0x0fff;

                if (content >= 0x0ff8)
                    content = FAT_READ_END_OF_CHAIN;
            }

            if (content == 0)
            {
                drv->FreeClusters++;
                if (!drv->LastCluster)
                    drv->LastCluster = cluster;

        #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
              /* enhance performance: No need to set 0 into AllocBitmap entries here, because AllocBitmap was cleared first */
              #if 0
                if (Set_AllocBitmapEntry(drv, cluster, 0) != FS_SUCCEED)
                {
                    MP_ALERT("%s: -E- Set_AllocBitmapEntry() failed !", __FUNCTION__);
                    ext_mem_free(temp_FAT_buffer);
                    return ABNORMAL_STATUS;
                }
              #endif
            }
            else
            {
                if (Set_AllocBitmapEntry(drv, cluster, 1) != FS_SUCCEED)
                {
                    MP_ALERT("%s: -E- Set_AllocBitmapEntry() failed !", __FUNCTION__);
                    ext_mem_free(temp_FAT_buffer);
                    return ABNORMAL_STATUS;
                }
        #endif
            }
        }
        covered_FAT_sectors += buff_sectors_count;
    }

    ext_mem_free(temp_FAT_buffer);

#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
    drv->Flag.AllocBitmapReady = 1;
#endif

    MP_DEBUG("%s: drive index = %u, current count of free clusters = %lu \r\n", __FUNCTION__, drv->DrvIndex, drv->FreeClusters);
    return FS_SUCCEED;
}



/* Read boot sector info and get/calculate all FAT file system info of the drive */
static int BootRecordRead(DRIVE * drv, DWORD start)
{
    BOOT_SECTOR *boot_sector;
    FSINFO *fsinfo;
    DWORD temp;
    BYTE *buffer;
    BYTE bFactor = 0;
    WORD wBytesPerSector = 0;
    BYTE bExp = 0;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    MP_DEBUG("Enter %s(drv, start = %d)..., drv->DevID = %d, drv->partitionStartLba = %d", __FUNCTION__, start, drv->DevID, drv->partitionStartLba);

    // read the sector that contain the partition table
    buffer = (BYTE *) ker_mem_malloc(4096, TaskGetId()); // For iPod 4KB/Sector
    if (buffer == NULL)
    {
        MP_ALERT("%s: Error: ker_mem_malloc() failed for size 4096 bytes !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

    if (DriveRead(drv, buffer, start, 1) != FS_SUCCEED)
    {
        ker_mem_free(buffer);
        MP_ALERT("%s: DriveRead() failed !!!", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

    if (*(WORD *) (buffer + SIGNATURE_OFFSET) != COMMON_SIGNATURE)
    {
        ker_mem_free(buffer);
        drv->Flag.FsType = FS_TYPE_UNSUPPORT;

        MP_ALERT("%s: -E- (Boot Sector) Signature 0x55AA not found => NG !!", __FUNCTION__);
#if 0 //dump the last 64 bytes of the buffer for check
        BYTE x, y;
        for (x=0; x < 4; x++)
        {
            for (y=0; y < 16; y++)
                mpDebugPrintN("0x%02x ", buffer[512 - 64 + y + (16 * x)]);

            mpDebugPrint("");
        }
#endif

        return DRIVE_ACCESS_FAIL;
    }

#if EXFAT_ENABLE
    /* check 3-byte BS_jmpBoot in boot sector to see whether if this partition is exFAT type */
    if ((buffer[0] == 0xEB) && (buffer[1] == 0x76) && (buffer[2] == 0x90))  /* check first 3 bytes for BS_jmpBoot */
    {
        TYPE_exFAT_Boot_Sector *exfat_boot_sector;
        BYTE *Hi32_of_64, *Lo32_of_64;
        EXFAT_NEW_INFO *exfat_info_buffer;

        MP_DEBUG("%s: Volume is exFAT type", __FUNCTION__);
        drv->FatRead = Read_exFAT;
        drv->FatWrite = Write_exFAT;
        drv->Flag.FsType = FS_TYPE_exFAT;

    #if EXFAT_WRITE_ENABLE
        MP_ALERT("%s: To-Do: remember to set Read-Only flag to 0 here after exFAT Write operations implementation are reay !!!", __FUNCTION__);
        drv->Flag.ReadOnly = 1; //not supported yet => set Read-Only flag to 1 temporarily to avoid writing to the exFAT drive
    #else
        MP_DEBUG("%s: exFAT Write operations are not supported => set Read-Only flag to 1 !", __FUNCTION__);
        drv->Flag.ReadOnly = 1;
    #endif

        if (drv->exFAT_InfoFileds == NULL)
        {
            exfat_info_buffer = (EXFAT_NEW_INFO *) ker_mem_malloc(sizeof(EXFAT_NEW_INFO), TaskGetId());
            if (exfat_info_buffer == NULL)
            {
                MP_ALERT("%s: Error! ker_mem_malloc() failed for !", __FUNCTION__);
                ker_mem_free(buffer);
                return ABNORMAL_STATUS;
            }
            MpMemSet((BYTE *) exfat_info_buffer, 0, sizeof(EXFAT_NEW_INFO));
            drv->exFAT_InfoFileds = (EXFAT_NEW_INFO *) exfat_info_buffer;
        }

        exfat_boot_sector = (TYPE_exFAT_Boot_Sector *) buffer;

        /* PartitionOffset:  although 64-bit in exFAT spec, its real value will not exceed 32-bit ! */
        Lo32_of_64 = (BYTE *) &exfat_boot_sector->partitionOffset; /* due to little-endian */
        Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
        if ((DWORD) LoadAlien32(Hi32_of_64) > 0)
        {
            MP_ALERT("%s: Warning!! PartitionOffset value exceeds 32-bit => must extend the field to 64-bit !!", __FUNCTION__);
        }
        else if ((DWORD) LoadAlien32(Lo32_of_64) != drv->partitionStartLba)
        {
            MP_ALERT("%s: Warning!! PartitionOffset value is not equal to the record in MBR partition table !", __FUNCTION__);
            /* we believe the value of MBR partition table */
        }
        else
            drv->partitionStartLba = (DWORD) LoadAlien32(Lo32_of_64);

        /* VolumeLength: 64-bit */
        Lo32_of_64 = (BYTE *) &exfat_boot_sector->volumeLength; /* due to little-endian */
        Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
        drv->exFAT_InfoFileds->VolumeLength = (U64) ((U64) LoadAlien32(Hi32_of_64) << 32) + (U64) LoadAlien32(Lo32_of_64);

        /* FatOffset */
        drv->FatStart = LoadAlien32(&exfat_boot_sector->fatOffset);

        /* FatLength */
        drv->FatSize = LoadAlien32(&exfat_boot_sector->fatLength);

        /* ClusterHeapOffset */
        drv->DataStart = LoadAlien32(&exfat_boot_sector->clusterHeapOffset);

        /* ClusterCount */
        drv->TotalClusters = LoadAlien32(&exfat_boot_sector->clusterCount);

        /* FirstClusterOfRootDirectory */
        drv->RootStart = LoadAlien32(&exfat_boot_sector->rootDir_firstCluster);

        /* VolumeSerialNumber */
        drv->exFAT_InfoFileds->VolumeSerialNo = LoadAlien32(&exfat_boot_sector->volSerial);

        /* FileSystemRevision */
        drv->exFAT_InfoFileds->FsRevision_Major = (BYTE) ((LoadAlien16(&exfat_boot_sector->fsRevision) & 0xFF00) >> 8);
        drv->exFAT_InfoFileds->FsRevision_Minor = (BYTE) (LoadAlien16(&exfat_boot_sector->fsRevision) & 0x00FF);

        /* VolumeFlags */
        drv->exFAT_InfoFileds->VolumeFlags.ActiveFat = (BYTE) (exfat_boot_sector->volumeFlags & 0x0001);
        drv->exFAT_InfoFileds->VolumeFlags.VolumeDirty = (BYTE) ((exfat_boot_sector->volumeFlags & 0x0002) >> 1);
        drv->exFAT_InfoFileds->VolumeFlags.MediaFailure = (BYTE) ((exfat_boot_sector->volumeFlags & 0x0004) >> 2);
        drv->exFAT_InfoFileds->VolumeFlags.ClearToZero = (BYTE) ((exfat_boot_sector->volumeFlags & 0x0008) >> 3);

        /* BytesPerSectorShift */
        drv->bSectorExp = (BYTE) exfat_boot_sector->bytesPerSectorShift;

        /* SectorsPerClusterShift */
        drv->ClusterExp = (BYTE) exfat_boot_sector->sectorsPerClusterShift;
        drv->ClusterSize = (1 << drv->ClusterExp);

        /* NumberOfFats */
        drv->NumberOfFat = (BYTE) exfat_boot_sector->numberOfFats;

        /* DriveSelect */
        drv->exFAT_InfoFileds->DriveSelect = (BYTE) exfat_boot_sector->driveSelect;

        /* PercentInUse */
        drv->exFAT_InfoFileds->PercentInUse = (BYTE) exfat_boot_sector->percentInUse;

    #if 1 // for debugging exFAT boot sector
        /* FileSystemName */
        BYTE fs_name[9];
        MpMemCopy(fs_name, exfat_boot_sector->fsName, 8);
        fs_name[8] = 0x00;
        MP_DEBUG("  FileSystemName = [%s]", fs_name);

        /* PartitionOffset:  although 64-bit in exFAT spec, its real value will not exceed 32-bit ! */
        MP_DEBUG("  PartitionOffset = 0x%08x \r\n", (DWORD) drv->partitionStartLba);

        /* VolumeLength: 64-bit */
        MP_DEBUG("  VolumeLength = 0x%08x%08x \r\n",
                 (DWORD) (((U64) drv->exFAT_InfoFileds->VolumeLength & 0xFFFFFFFF00000000) >> 32),
                 (DWORD) (drv->exFAT_InfoFileds->VolumeLength & 0x00000000FFFFFFFF));

        MP_DEBUG("  FatOffset = %lu = 0x%x", drv->FatStart, drv->FatStart);
        MP_DEBUG("  FatLength = %lu = 0x%x", drv->FatSize, drv->FatSize);
        MP_DEBUG("  ClusterHeapOffset = %lu = 0x%x", drv->DataStart, drv->DataStart);
        MP_DEBUG("  ClusterCount = %lu = 0x%x", drv->TotalClusters, drv->TotalClusters);
        MP_DEBUG("  FirstClusterOfRootDirectory = %lu = 0x%x", drv->RootStart, drv->RootStart);
        MP_DEBUG("  VolumeSerialNumber = %lu", drv->exFAT_InfoFileds->VolumeSerialNo);
        MP_DEBUG("  FileSystemRevision = (%d.%d)", drv->exFAT_InfoFileds->FsRevision_Major, drv->exFAT_InfoFileds->FsRevision_Minor);

        /* VolumeFlags */
        MP_DEBUG("  VolumeFlags = 0x%04x", (WORD) exfat_boot_sector->volumeFlags);
        MP_DEBUG("  VolumeFlags (ActiveFat, VolumeDirty, MediaFailure, ClearToZero) = (%d, %d, %d, %d)",
                 (BYTE) drv->exFAT_InfoFileds->VolumeFlags.ActiveFat,
                 (BYTE) drv->exFAT_InfoFileds->VolumeFlags.VolumeDirty,
                 (BYTE) drv->exFAT_InfoFileds->VolumeFlags.MediaFailure,
                 (BYTE) drv->exFAT_InfoFileds->VolumeFlags.ClearToZero);

        MP_DEBUG("  BytesPerSectorShift = %d", drv->bSectorExp);
        MP_DEBUG("  SectorsPerClusterShift = %d", drv->ClusterExp);
        MP_DEBUG("  NumberOfFats = %d", drv->NumberOfFat);
        MP_DEBUG("  DriveSelect = 0x%02x", drv->exFAT_InfoFileds->DriveSelect);
        MP_DEBUG("  PercentInUse = %d (%%)", drv->exFAT_InfoFileds->PercentInUse);
    #endif
    }
    else /* not exFAT */
#endif //EXFAT_ENABLE
    {
        boot_sector = (BOOT_SECTOR *) buffer;
        drv->ClusterSize = boot_sector->SectorsPerCluster;
        MP_DEBUG("%s: Sectors per cluster = %d", __FUNCTION__, drv->ClusterSize);

        temp = LoadAlien16(&boot_sector->BytesPerSector);
        MP_DEBUG("%s: Bytes per sector = %d", __FUNCTION__, temp);

        if (temp == 0)
        {
            MP_ALERT("%s: parsed 'Bytes per sector' = 0 => invalid !!", __FUNCTION__);
            ker_mem_free(buffer);
            drv->Flag.FsType = FS_TYPE_UNSUPPORT;  // toggle this drive is not formated
            return DRIVE_ACCESS_FAIL;   // not formated sector, return fail
        }
        else if (temp % 512) /* sector size should be multiples of 512-bytes */
        {
            MP_ALERT("%s: parsed 'Bytes per sector' (%u) not multiples of 512 => invalid !!", __FUNCTION__, temp);
            ker_mem_free(buffer);
            drv->Flag.FsType = FS_TYPE_UNSUPPORT;  // toggle this drive is not formated
            return DRIVE_ACCESS_FAIL;   // not formated sector, return fail
        }

        // calculate the exponent of 2 for bytes-per-sector
        drv->bSectorExp = 0;
        while (temp != 1)
        {
            temp >>= 1;
            drv->bSectorExp++;
        }

        // calculate the exponent of 2 for sectors-per-cluster
        temp = drv->ClusterSize >> 1;
        drv->ClusterExp = 0;
        while (temp)
        {
            temp >>= 1;
            drv->ClusterExp++;
        }

        MP_DEBUG("%s: SectorExp = %d, ClusterExp = %d", __FUNCTION__, drv->bSectorExp, drv->ClusterExp);

        drv->NumberOfFat = boot_sector->NumberOfFATs;
        drv->FatStart = start + LoadAlien16(&boot_sector->ReservedSectors);

        drv->FatSize = LoadAlien16(&boot_sector->SectorsPerFAT);

        if (!drv->FatSize)
            drv->FatSize = LoadAlien32(&boot_sector->BigSectorPerFAT);

        drv->TotalClusters = LoadAlien16(&boot_sector->TotalSectors);

        if (!drv->TotalClusters)
            drv->TotalClusters = LoadAlien32(&boot_sector->BigTotalSectors);

        // FAT Type Determination
        //
        // Microsoft FAT32 File System Specification		ver1.03 December 6, 2000
        //
        // Please referen page.14 - page.15
        //											C.W 080828
        DWORD FATSz = 0;
        DWORD TotSec = 0;
        DWORD DataSec = 0;
        DWORD CountofClusters = 0;
        DWORD RootDirSecs = 0;

        DWORD BPB_BytsPerSec   = LoadAlien16(&boot_sector->BytesPerSector);    // offset 11 - 12
        DWORD BPB_SecPerClus   = boot_sector->SectorsPerCluster;               // offset 13
        DWORD BPB_ResvdSecCnt  = LoadAlien16(&boot_sector->ReservedSectors);   // offset 14 - 15
        BYTE  BPB_NumFats      = boot_sector->NumberOfFATs;                    // offset 16
        DWORD BPB_RootEntCnt   = LoadAlien16(&boot_sector->RootEntries);       // offset 17

        FATSz  = drv->FatSize;
        TotSec = drv->TotalClusters;
        RootDirSecs = ( (BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1) ) / BPB_BytsPerSec;
        DataSec = TotSec - (BPB_ResvdSecCnt + (BPB_NumFats * FATSz) + RootDirSecs);
        CountofClusters = DataSec / BPB_SecPerClus;
        MP_DEBUG("%s: Count of clusters = %d", __FUNCTION__, CountofClusters);

        if (CountofClusters < 4085)  //Volume is FAT12
        {
            MP_DEBUG("%s: Volume is FAT12 type", __FUNCTION__);
            drv->FatRead = ReadFat12;
            drv->FatWrite = WriteFat12;
            drv->Flag.FsType = FS_TYPE_FAT12;
        }
        else if (CountofClusters < 65525)  //Volume is FAT16
        {
            MP_DEBUG("%s: Volume is FAT16 type", __FUNCTION__);
            drv->FatRead = ReadFat16;
            drv->FatWrite = WriteFat16;
            drv->Flag.FsType = FS_TYPE_FAT16;
        }
        else  //Volume is FAT32
        {
            MP_DEBUG("%s: Volume is FAT32 type", __FUNCTION__);
            drv->FatRead = ReadFat32;
            drv->FatWrite = WriteFat32;
            drv->Flag.FsType = FS_TYPE_FAT32;
        }

        if (drv->Flag.FsType == FS_TYPE_FAT32)
        {
            //note: for FAT32, drv->RootStart is cluster number of the first cluster of root directory
            drv->RootStart = LoadAlien32(&boot_sector->RootDirStartCluster);
            drv->FsInfoAddr = LoadAlien16(&boot_sector->FSInfoSector);
            if (drv->FsInfoAddr != 0xffffffff)
                drv->FsInfoAddr += start;
            drv->DataStart = drv->FatStart + drv->FatSize * drv->NumberOfFat;
        }
        else if ((drv->Flag.FsType == FS_TYPE_FAT12) || (drv->Flag.FsType == FS_TYPE_FAT16))
        {
            //note: for FAT12/16, drv->RootStart is lba of first sector of root directory
            drv->RootStart = drv->FatStart + drv->FatSize * drv->NumberOfFat;
            drv->DataStart = drv->RootStart + (LoadAlien16(&boot_sector->RootEntries) >> (drv->bSectorExp - 5));
            drv->FsInfoAddr = 0xffffffff;
        }

        drv->TotalClusters = (drv->TotalClusters - (drv->DataStart - start)) >> drv->ClusterExp;
        drv->FreeClusters = 0xffffffff;

        if (drv->FsInfoAddr != 0xffffffff)
        {
            if (DriveRead(drv, buffer, drv->FsInfoAddr, 1) != FS_SUCCEED)
            {
                ker_mem_free(buffer);
                MP_ALERT("%s: DriveRead() failed !!!", __FUNCTION__);
                return DRIVE_ACCESS_FAIL;
            }

            fsinfo = (FSINFO *) (buffer + FSINFO_OFFSET);
            if (fsinfo->Signature == FSINFO_SIGNATURE)
            {
                drv->FreeClusters = LoadAlien32(&fsinfo->FreeClusters);
                drv->LastCluster = LoadAlien32(&fsinfo->NextFree);
            }
        }
    }

    ker_mem_free(buffer);
    return FS_SUCCEED;
}



/* This function is used when there is no partition table found in the first sector of the device.
   Check whether if the first sector of the device is a boot record or not. If yes, read the boot record info.
   If not, then the drive is not formatted.
*/
static int FloppyCheck(DRIVE * drv)
{
    BYTE *buffer;


    MP_DEBUG("Enter %s()...", __FUNCTION__);

    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    buffer = (BYTE *) ker_mem_malloc(4096, TaskGetId()); // For iPod 4KB/Sector
    if (buffer == NULL)
    {
        MP_ALERT("%s: Error: ker_mem_malloc() failed for size 4096 bytes !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

    // read the first sector
    drv->partitionStartLba = 0; //reset to 0, thus can scan from LBA 0 sector
    if (DriveRead(drv, buffer, 0, 1) != FS_SUCCEED)
    {
        ker_mem_free(buffer);
        MP_ALERT("%s(): DriveRead() failed !!!", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

    if (*(WORD *) (buffer + SIGNATURE_OFFSET) != COMMON_SIGNATURE)
    {
        MP_ALERT("%s: -E- (Boot Sector) Signature 0x55AA not found => NG !!", __FUNCTION__);
        ker_mem_free(buffer);
        return FS_SCAN_FAIL;	// not formated sector, return fail
    }

    /* compare BS_FileSysType field string in boot sector, this is informational only, not mandatory */
    if (!memcmp(buffer + 0x36, "FAT12", 5))
    {
        drv->FatRead = ReadFat12;
        drv->FatWrite = WriteFat12;
    }
    else if (!memcmp(buffer + 0x36, "FAT16", 5))
    {
        drv->FatRead = ReadFat16;
        drv->FatWrite = WriteFat16;
    }
    else if (!memcmp(buffer + 0x52, "FAT32", 5))
    {
        drv->FatRead = ReadFat32;
        drv->FatWrite = WriteFat32;
    }
#if EXFAT_ENABLE
    else if (((buffer[0] == 0xEB) && (buffer[1] == 0x76) && (buffer[2] == 0x90)) && (!memcmp(buffer + 0x03, "EXFAT", 5)))
    {
        drv->FatRead = Read_exFAT;
        drv->FatWrite = Write_exFAT;
    }
#endif
    else
    {
  /* mark this, because BS_FileSysType field string in boot sector is informational only, not mandatory */
  #if 0
        MP_ALERT("%s: Not supported file system !!!", __FUNCTION__);
        ker_mem_free(buffer);
        return FS_SCAN_FAIL;
  #endif
    }

    ker_mem_free(buffer);

    // read boot sector info and get FAT file system info of the drive
    if (BootRecordRead(drv, 0) != FS_SUCCEED)
    {
        MP_ALERT("%s: BootRecordRead() failed !!!", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

    drv->Partition = 0;

    return FS_SUCCEED;
}



/* Scan the partition table of the device, and return the number of partitions counted. */
static int PartitionScan(DRIVE * drv)
{
    PARTITION *par, *Expar;
    BYTE *buffer;
    DRIVE_PHY_DEV_ID phyDevID;
    E_DRIVE_INDEX_ID first_drv_index_ID;
    DWORD work_lba, base_lba, temp_lba;
    SDWORD flag;
    BYTE parCounter; /* for counting partitions in a device */
    BYTE par_idx = 0;
    DRIVE *part_drv;
    DWORD total_SectorNr;


    MP_DEBUG("Enter %s()...", __FUNCTION__);
    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return 0;
    }

    work_lba = 0;
    base_lba = 0;
    parCounter = 0;
    phyDevID = DriveIndex2PhyDevID(drv->DrvIndex);

    total_SectorNr = Mcard_GetCapacity(phyDevID);
    if (total_SectorNr == 0)
    {
        MP_ALERT("%s: Error! Count of total sectors of the device = 0 !", __FUNCTION__);
        return 0;
    }
    MP_DEBUG("%s: total sectors = %lu", __FUNCTION__, total_SectorNr);

    first_drv_index_ID = PhyDevID2DriveIndex(phyDevID);
    part_drv = DriveGet(first_drv_index_ID);
    part_drv->partitionStartLba = 0; //reset to 0, thus can scan from LBA 0 sector

    buffer = (BYTE *) ker_mem_malloc(4096, TaskGetId()); // For iPod 4KB/Sector
    if (buffer == NULL)
    {
        MP_ALERT("%s: Error: ker_mem_malloc() failed for size 4096 bytes !", __FUNCTION__);
        return 0;
    }

    work_lba = 0;
    MP_DEBUG("%s: DriveRead LBA addr = %d", __FUNCTION__, work_lba);

    if (DriveRead(part_drv, buffer, work_lba, 1) != FS_SUCCEED)
    {
        ker_mem_free(buffer);
        MP_ALERT("%s: DriveRead() failed !!!", __FUNCTION__);
        return 0;
    }

    if (*(WORD *) (buffer + SIGNATURE_OFFSET) != COMMON_SIGNATURE)  /* check MBR sector signature (2 bytes: offset 510, 511) */
    {
        MP_ALERT("%s: -I- (MBR) Signature 0x55AA not found.", __FUNCTION__);
        ker_mem_free(buffer);
        return 0;
    }

    BYTE *sector_buffer = (BYTE *) ker_mem_malloc(4096, TaskGetId()); // For iPod 4KB/Sector
    if (sector_buffer == NULL)
    {
        MP_ALERT("%s: Error: ker_mem_malloc() failed !", __FUNCTION__);
        ker_mem_free(buffer);
        return 0;
    }

    BYTE *pTmpByte;
    par = (PARTITION *) (buffer + PARTITION_OFFSET);
    while (1)
    {
        DWORD partitionSize = 0;

        flag = 1;  // preset to supported flag type
        part_drv->DevID = phyDevID;
        //part_drv->DrvIndex = first_drv_index_ID + parCounter;
        part_drv->partitionStartLba = LoadAlien32(&par->Start);
        partitionSize = LoadAlien32(&par->Size);
        MP_DEBUG("\r\n %s: Partition[%d] start sector = %lu, type = %u, count of sectors = %lu", __FUNCTION__, par_idx, part_drv->partitionStartLba, par->Type, partitionSize);

        pTmpByte = (BYTE *) par;
        if (((BYTE) *pTmpByte != 0x80) && ((BYTE) *pTmpByte != 0x00))
        {
            MP_DEBUG("%s: Invalid Active (Bootable) flag of this partition table entry.", __FUNCTION__);
            goto L_next_partition; /* scan next partition */
        }

        /* note: loose check of partition sector range to avoid DriveAdd() fail (ex: some USB Pen Drives, some MS cards, ... etc),
         * because 'partition size' field of each partition table entry may be not exactly right and precise.
         */
        if (part_drv->partitionStartLba >= total_SectorNr)
        //if ( (part_drv->partitionStartLba >= total_SectorNr) || ((part_drv->partitionStartLba + partitionSize - 1) > total_SectorNr) )
        {
            MP_DEBUG("%s: partition entry setting exceeds device capacity (total_SectorNr = %lu) => Not a valid partition table entry.", __FUNCTION__, total_SectorNr);
            goto L_next_partition; /* scan next partition */
        }

        switch (par->Type)
        {
            case 1:     // FAT12 type
            case 4:     // FAT16 type
            case 6:     // FAT16 type
            case 14:    // FAT16 type
            case 11:    // FAT32 type
            case 12:    // FAT32 type
            case 5:     // DOS extended type
            case 15:    // DOS extended type
            case 7:     // NTFS, OS/2 HPFS type or exFAT type
                MP_DEBUG("%s: known partition type (%u) => check boot sector ...", __FUNCTION__, par->Type);
                break;

            default:    // not support partition type
                part_drv->partitionStartLba = 0; // maybe bad MBR or bad partition table => reset part_drv->partitionStartLba to 0 for later Boot Sector check !
                goto L_next_partition; /* scan next partition */
        }

        /* check whether if boot sector exist at the specified sector by the partition table entry */
        if (DriveRead(part_drv, sector_buffer, 0, 1) != FS_SUCCEED)
        {
            ker_mem_free(buffer);
            ker_mem_free(sector_buffer);
            MP_ALERT("%s: DriveRead() failed !!!", __FUNCTION__);
            return parCounter;
        }

        if (*((WORD *) (sector_buffer + SIGNATURE_OFFSET)) != COMMON_SIGNATURE)
        {
            MP_DEBUG("%s: (Boot Sector) Signature 0x55AA not found for this partition.", __FUNCTION__);

            if ((phyDevID == DriveIndex2PhyDevID(SYS_DRV_ID)) && (phyDevID == (((DRIVE *) part_drv) + 1)->DevID))
            {
                part_drv++;  /* increase to next drive and prepare for next partition */
                parCounter++;
            }

            goto L_next_partition; /* scan next partition */
        }

        /* check first 3 bytes for boot sector BS_jmpBoot pattern */
        if (!( ((sector_buffer[0] == 0xEB) && (sector_buffer[2] == 0x90)) || (sector_buffer[0] == 0xE9) ))
        {
            MP_ALERT("%s: Warning: (Boot Sector) BS_jmpBoot pattern not meet FATxx spec for this partition.", __FUNCTION__);
            MP_DEBUG("3-byte BS_jmpBoot = 0x%02x 0x%02x 0x%02x", sector_buffer[0], sector_buffer[1], sector_buffer[2]);

            if ((phyDevID == DriveIndex2PhyDevID(SYS_DRV_ID)) && (phyDevID == (((DRIVE *) part_drv) + 1)->DevID))
            {
                part_drv++;  /* increase to next drive and prepare for next partition */
                parCounter++;
            }

            goto L_next_partition; /* scan next partition */
        }

        switch (par->Type)
        {
        case 1:     // FAT12 type
            MP_DEBUG("%s: Partition type: %d => FAT12 type partition !", __FUNCTION__, par->Type);
            break;

        case 4:
        case 6:
        case 14:    // FAT16 type
            MP_DEBUG("%s: Partition type: %d => FAT16 type partition !", __FUNCTION__, par->Type);
            break;

        case 11:
        case 12:    // FAT32 type
            MP_DEBUG("%s: Partition type: %d => FAT32 type partition !", __FUNCTION__, par->Type);
            break;

        case 5:
        case 15:    // DOS extended type
            /* To-Do: this processing for "DOS extended partition type" can work ??  need to be checked ... */

            MP_DEBUG("%s: Partition type: %d => DOS extended type partition !", __FUNCTION__, par->Type);
            //// IF the extended partition need to be last ?
            work_lba = base_lba;

            // if base_lba zero mean this extended partition is the first
            // extended partition, then it would be the base partition of
            // the following extended partitions
            if (!base_lba)
            {
                (PARTITION *)Expar = (PARTITION *)par;
                (PARTITION *)Expar++;
                base_lba = work_lba;
            }

            if (DriveRead(part_drv, buffer, work_lba, 1) != FS_SUCCEED)
            {
                ker_mem_free(buffer);
                ker_mem_free(sector_buffer);
                MP_ALERT("%s: DriveRead() failed !!!", __FUNCTION__);
                return parCounter;
            }

            // if not formated sector, then ending the scanning
            if (*((WORD *) (buffer + SIGNATURE_OFFSET)) != COMMON_SIGNATURE)
            {
                MP_ALERT("%s: -I- (Boot Sector) Signature 0x55AA not found for this partition.", __FUNCTION__);
                ker_mem_free(buffer);
                ker_mem_free(sector_buffer);
                return parCounter;
            }

            // reset the partition table point
            par = (PARTITION *) (buffer + PARTITION_OFFSET - 0x10);
            flag = 0;
            break;

        case 0x07:  // NTFS, OS/2 HPFS type or exFAT type
        {
            MP_ALERT("%s: Partition type: %d, partition start Lba = %lu, check to see if exFAT type ...", __FUNCTION__, par->Type, part_drv->partitionStartLba);

#if EXFAT_ENABLE
            /* check 3-byte BS_jmpBoot in boot sector to see whether if this partition is exFAT type */
            if ((sector_buffer[0] == 0xEB) && (sector_buffer[1] == 0x76) && (sector_buffer[2] == 0x90))  /* check first 3 bytes for BS_jmpBoot */
            {
                MP_ALERT("%s: Partition type: %d, and this partition is exFAT type.", __FUNCTION__, par->Type);
                part_drv->Flag.FsType = FS_TYPE_exFAT;
            }
            else
#endif
            {
    #if EXFAT_ENABLE
                MP_ALERT("%s: Partition type: %d, but this partition is not exFAT type !", __FUNCTION__, par->Type);
    #endif
                MP_ALERT("Maybe NTFS format, MPX do not support it !");
                part_drv->Flag.FsType = FS_TYPE_UNSUPPORT;
                flag = 0;
            }
        }
        break;

        default:    // not support partition type
            flag = 0;
            part_drv->partitionStartLba = 0; // maybe bad MBR or bad partition table => reset part_drv->partitionStartLba to 0 for later Boot Sector check !
        }

        if (flag)
        {
            CACHE *cache;

            if (part_drv->CacheBufPoint == NULL)
            {
                cache = (CACHE *) ker_mem_malloc(sizeof(CACHE), TaskGetId());
                if (cache == NULL)
                {
                    MP_ALERT("%s: Error: ker_mem_malloc() failed for size %d bytes !", __FUNCTION__, sizeof(CACHE));
                    ker_mem_free(buffer);
                    ker_mem_free(sector_buffer);
                    return parCounter;
                }

                cache = (CACHE *) ((DWORD) cache | BIT29);
                MpMemSet(cache, 0, sizeof(CACHE));

                part_drv->CacheBufPoint = cache;
                part_drv->FatCacheBuffer = cache->FatCache;
                part_drv->DirStackBuffer = cache->Stack;
                part_drv->DirCacheBuffer = cache->DirCache;
            }

            // parsing the partition boot sector
            temp_lba = work_lba;

            if (BootRecordRead(part_drv, temp_lba) != FS_SUCCEED)
            {
                ker_mem_free(buffer);
                ker_mem_free(sector_buffer);
                part_drv->StatusCode = FS_SCAN_FAIL;
                part_drv->Flag.Present = 0;
                return parCounter;
            }

            // set and increase the partition counter
            part_drv->Partition = parCounter;
            part_drv->StatusCode = FS_SUCCEED;
            part_drv->Flag.Present = 1;
            parCounter++;

            if (parCounter >= MAX_PARTITION_NUM)
            {
                ker_mem_free(buffer);
                ker_mem_free(sector_buffer);
                return parCounter;
            }

            /* we only support multiple partitions for these kinds of storage devices */
            if ((phyDevID == DEV_NAND) || (phyDevID == DEV_SPI_FLASH) || (phyDevID == DEV_HD) || (phyDevID == DEV_SD_MMC))
            {
                /* increase to next drive and prepare for next partition */
                part_drv++;
                if (part_drv->DevID == phyDevID)  /* same storage device */
                {
                    //part_drv->DrvIndex = first_drv_index_ID + parCounter;
                    ;  /* do nothing */
                }
                else  /* different storage device => finish partition scanning */
                {
                    ker_mem_free(buffer);
                    ker_mem_free(sector_buffer);
                    return parCounter;
                }
            }
        }

L_next_partition:
        /* we only support multiple partitions for these kinds of storage devices */
        if ((phyDevID != DEV_NAND) && (phyDevID != DEV_SPI_FLASH) && (phyDevID != DEV_HD) && (phyDevID != DEV_SD_MMC))
        {
            /* note: consider "Apple iPod 30GB" special case: it has 2 partitions, the 1st partition is unknown partition type and the 2nd one is FAT32 */
            if (parCounter > 0)
            {
                MP_DEBUG("%s: -I-  We do not support multiple partitions for removable storage devices.", __FUNCTION__);
                ker_mem_free(buffer);
                ker_mem_free(sector_buffer);
                return parCounter;
            }
        }
        // increase to next partition entry
        // and if exceed the partition table then ending the scanning
        par++;

        if (par >= (PARTITION *) (buffer + SIGNATURE_OFFSET))
        {
            ker_mem_free(buffer);
            ker_mem_free(sector_buffer);

            if (parCounter == 0)
            {
                part_drv->StatusCode = NOT_SUPPORT_FS; // unknown or not supported file system
            }

            if (base_lba == 0)
                return parCounter;
            else
            {
                /* To-Do: this processing for "DOS extended partition type" can work ??  need to be checked ... */

                buffer = (BYTE *) ker_mem_malloc(4096, TaskGetId()); // For iPod 4KB/Sector
                if (buffer == NULL)
                {
                    MP_ALERT("%s: Error! ker_mem_malloc() failed for size 4096 bytes !", __FUNCTION__);
                    return 0;
                }

                work_lba = 0;
                par = Expar;
                base_lba = 0;

                if (DriveRead(part_drv, buffer, work_lba, 1) != FS_SUCCEED)
                {
                    ker_mem_free(buffer);
                    MP_ALERT("%s: DriveRead() failed !!!", __FUNCTION__);
                    return 0;
                }

                if (*(WORD *) (buffer + SIGNATURE_OFFSET) != COMMON_SIGNATURE)
                {
                    MP_ALERT("%s: -I- (Boot Sector) Signature 0x55AA not found for this partition.", __FUNCTION__);
                    ker_mem_free(buffer);
                    return 0;
                }
            }
        }

        par_idx++;
    }
}



///
///@ingroup DRIVE
///@brief   DriveDelete all well-added drives and then DriveAdd them again to renew info of all drives.
///
///@param   None.
///
///@return  None.
///
void RenewAllDrv(void)
{
    E_DRIVE_INDEX_ID i;
    DRIVE *drv;

    for (i=1; i < MAX_DRIVE_NUM; i++) /* skip drive index id 0 (NULL_DRIVE) */
    {
        drv = DriveGet(i);
        if (SystemCardPresentCheck(i) && (drv->StatusCode == FS_SUCCEED))
        {
            DriveDelete(i);
            DriveAdd(i);
        }
    }

    return;
}



///
///@ingroup DRIVE
///@brief   Read MBR partition table on the storage device to get partition info of a partition for the specified partition drive.
///
///@param   drv_index_ID            [IN] The drive index ID of the partition to get its partition info. \n\n
///@param   partition_idx           [OUT] Partition index value for the drive, valid values are 0 ~ 3. \n\n
///@param   partition_type          [OUT] Partition type for the drive. \n\n
///@param   partition_start_lba     [OUT] LBA of the start sector of the partition. \n\n
///@param   partition_SectorNr      [OUT] Partition size (number of sectors) for the drive. \n\n
///@param   partition_blockSize     [OUT] Block size (bytes per sector) for the partition drive.
///
///@retval  PASS            Get partition info successfully. \n\n
///@retval  FAIL            Failed due to some error. \n\n
///@retval  FS_SCAN_FAIL    Failed due to invaild MBR (and thus invalid partition table).
///
SWORD GetDrvPartitionInfoFromMBR(E_DRIVE_INDEX_ID drv_index_ID, BYTE * partition_idx, BYTE * partition_type, DWORD * partition_start_lba,
                                 DWORD * partition_SectorNr, DWORD * partition_blockSize)
{
    BYTE *sector_buf;
    PARTITION *partition;
    DWORD SectorSize, total_SectorNr;
    DRIVE_PHY_DEV_ID phyDevID = DriveIndex2PhyDevID(drv_index_ID);
    E_DRIVE_INDEX_ID drv_p0_index = PhyDevID2DriveIndex(phyDevID);
    DRIVE *drv = DriveGet(drv_index_ID);
    BYTE part_index;
    BOOL f_have_MBR = FALSE;
    DWORD start_lba, partitionSize;
    BYTE i;


    if (phyDevID == DEV_NULL)
    {
        MP_ALERT("%s: Error! Invalid device ID ! drv_index_ID = %d", __FUNCTION__, drv_index_ID);
        return FAIL;
    }
    else if ((phyDevID == DEV_USB_WIFI_DEVICE) ||\ 
             (phyDevID == DEV_CF_ETHERNET_DEVICE) ||\
             (phyDevID == DEV_USB_WEBCAM)||\
             (phyDevID== DEV_USB_ETHERNET_DEVICE)
)
    {
        MP_ALERT("%s: Error! The device is not a storage device !", __FUNCTION__);
        return FAIL;
    }

    /* check card whether if present */
    if (! SystemCardPresentCheck(drv_index_ID))
    {
        MP_ALERT("%s: The storage device (%s) is not present !", __FUNCTION__, DriveIndex2DrvName(drv_index_ID));
        return FAIL;
    }

    if (drv_index_ID == drv_p0_index)
        part_index = 0; /* 1st partition */
    else if (drv_index_ID == (drv_p0_index + 1))
        part_index = 1; /* 2nd partition */
    else if (drv_index_ID == (drv_p0_index + 2))
        part_index = 2; /* 3rd partition */
    else if (drv_index_ID == (drv_p0_index + 3))
        part_index = 3; /* 4th partition */
    else
    {
        MP_ALERT("%s: Error! Unsupported partition number !", __FUNCTION__);
        return FAIL;
    }

    if (partition_idx)
        *partition_idx = part_index;

    SectorSize = Mcard_GetSectorSize(phyDevID);
    if (SectorSize == 0)
    {
        MP_ALERT("%s: Error! Sector size = 0 ! phyDevID = %d", __FUNCTION__, phyDevID);
        return FAIL;
    }

    total_SectorNr = Mcard_GetCapacity(phyDevID);
    if (total_SectorNr == 0)
    {
        MP_ALERT("%s: Error! Count of total sectors of the device = 0 !", __FUNCTION__);
        return FAIL;
    }

    MP_DEBUG("%s: total sectors = %lu, sector size = %u", __FUNCTION__, total_SectorNr, SectorSize);

    if (partition_blockSize)
        *partition_blockSize = SectorSize;

    /* make sure minimum necessary fields for Mcard Read/Write functions: drv->DevID and drv->bSectorExp */
    if (drv->DevID == 0)
        drv->DevID = phyDevID;

    if (drv->bSectorExp == 0)
    {
        // calculate the exponent of 2 for bytes-per-sector
        DWORD temp = SectorSize;
        drv->bSectorExp = 0;
        while (temp != 1)
        {
            temp >>= 1;
            drv->bSectorExp++;
        }
    }

    sector_buf = (BYTE *) ext_mem_malloc(SectorSize);
    if (sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        return FAIL;
    }
    sector_buf = (BYTE *) ((DWORD) sector_buf | BIT29);

    BYTE *tmp_sector_buf = (BYTE *) ext_mem_malloc(SectorSize);
    if (tmp_sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        ext_mem_free(sector_buf);
        return FAIL;
    }
    tmp_sector_buf = (BYTE *) ((DWORD) tmp_sector_buf | BIT29);

    /* read MBR */
    Mcard_DeviceRead(drv, sector_buf, 0, 1);

    if (*(WORD *) (sector_buf + SIGNATURE_OFFSET) != COMMON_SIGNATURE)  /* check MBR sector signature (2 bytes: offset 510, 511) */
    {
        MP_ALERT("%s: -I- (MBR) Signature 0x55AA not found.", __FUNCTION__);

        /* no MBR => treat whole device as a single partition, set returned info values */
        if (partition_start_lba)
            *partition_start_lba = 0;

        if (partition_SectorNr)
            *partition_SectorNr = Mcard_GetCapacity(phyDevID);

        if (partition_type)
            *partition_type = 0x00; /* unknown partition type */

        ext_mem_free(sector_buf);
        ext_mem_free(tmp_sector_buf);
        return FS_SCAN_FAIL; /* here, this means failed to get vaild MBR */
    }

    BYTE *pTmpByte;
    partition = (PARTITION *) (sector_buf + PARTITION_OFFSET);
    for (i = 0; i < 4; i++)
    {
        pTmpByte = (BYTE *) &partition[i];
        if (((BYTE) *pTmpByte != 0x80) && ((BYTE) *pTmpByte != 0x00))
        {
            MP_DEBUG("%s: Invalid Active (Bootable) flag of this partition table entry.", __FUNCTION__);
            continue; /* check next partition */
        }

        switch (partition[i].Type)
        {
            case 1:     // FAT12 type
            case 4:     // FAT16 type
            case 6:     // FAT16 type
            case 14:    // FAT16 type
            case 11:    // FAT32 type
            case 12:    // FAT32 type
            case 5:     // DOS extended type
            case 15:    // DOS extended type
            case 7:     // NTFS OS/2 HPFS type or exFAT type
                MP_DEBUG("%s: known partition type (%u) ...", __FUNCTION__, partition[i].Type);
                start_lba = LoadAlien32(&partition[i].Start);  //LBA of the first sector in this partition
                partitionSize = LoadAlien32(&partition[i].Size);  //length (number of blocks) of this partion

//#if 0  /* Not check reasonability of values in partition table entry w.r.t total_SectorNr */
#if 1  /* Simple check reasonability of values in partition table entry w.r.t total_SectorNr */
                if ( (start_lba >= total_SectorNr) || ((start_lba + partitionSize - 1) > total_SectorNr) )
                {
                    /* note: Do not strictly check for USB Pen drive devices, because their MBR partition table setting may be not exactly right and precise */
                    /*       Check the partition sector range for all non-USB devices */
                    if ( (phyDevID != DEV_USB_HOST_ID1) && (phyDevID != DEV_USB_HOST_ID2) && (phyDevID != DEV_USB_HOST_ID3) &&
                         (phyDevID != DEV_USB_HOST_ID4) && (phyDevID != DEV_USB_HOST_PTP) &&
                         (phyDevID != DEV_USBOTG1_HOST_ID1) && (phyDevID != DEV_USBOTG1_HOST_ID2) && (phyDevID != DEV_USBOTG1_HOST_ID3) &&
                         (phyDevID != DEV_USBOTG1_HOST_ID4) && (phyDevID != DEV_USBOTG1_HOST_PTP) )
                    {
                        MP_DEBUG("%s: Value setting exceeds device capacity (total_SectorNr = %lu) => Not a valid partition table entry.", __FUNCTION__, total_SectorNr);
                        continue; /* check next partition */
                    }
                }
                else
#endif
                {
                    f_have_MBR = TRUE;
                    MP_DEBUG("%s: => check partition boot sector ...", __FUNCTION__);
                }
                break;
            default:    // not support partition type
                continue; /* check next partition */
        }

        /* check whether if boot sector exist at the sector specified by the partition table entry */
        if (Mcard_DeviceRead(drv, tmp_sector_buf, start_lba, 1) != FS_SUCCEED)
        {
            MP_ALERT("%s: Mcard_DeviceRead() failed !", __FUNCTION__);
            continue; /* check next partition */
        }

        if (*((WORD *) (tmp_sector_buf + SIGNATURE_OFFSET)) != COMMON_SIGNATURE)
        {
            MP_DEBUG("%s: Warning: (Boot Sector) Signature 0x55AA not found for this partition.", __FUNCTION__);
            continue; /* check next partition */
        }
    }

    if (!f_have_MBR)
    {
        MP_ALERT("%s: -I- sector 0 is actually a Boot Sector, not the MBR sector.", __FUNCTION__);

        /* no MBR => treat whole device as a single partition, set returned info values */
        if (partition_start_lba)
            *partition_start_lba = 0;

        if (partition_SectorNr)
            *partition_SectorNr = total_SectorNr;

        if (partition_type)
        {
            /* work around for partition type value decision */
            if (drv->FatRead == ReadFat32)
                *partition_type = 0x0b;
            else if (drv->FatRead == ReadFat16)
            {
                if (((total_SectorNr * 512) >> 20) > 32) //FAT16 with size over 32MB
                    *partition_type = 0x06;
                else //FAT16 with size less than 32MB
                    *partition_type = 0x04;
            }
            else if (drv->FatRead == ReadFat12)
                *partition_type = 0x01;
            else
                *partition_type = 0x00; /* unknown partition type */
        }

        ext_mem_free(sector_buf);
        ext_mem_free(tmp_sector_buf);
        return FS_SCAN_FAIL; /* here, this means failed to get vaild MBR */
    }

    if (partition_type)
        *partition_type = partition[part_index].Type;

    if (partition_start_lba)
        *partition_start_lba = LoadAlien32(&partition[part_index].Start);  //LBA of the first sector in this partition

    if (partition_SectorNr)
        *partition_SectorNr = LoadAlien32(&partition[part_index].Size);  //length (number of blocks) of this partion

    MP_DEBUG("%s: partition index = %d, type = 0x%02x, start LBA = %d, SectorNr = %d, BlockSize = %d", __FUNCTION__,
             part_index, partition[part_index].Type, LoadAlien32(&partition[part_index].Start), LoadAlien32(&partition[part_index].Size), SectorSize);

    ext_mem_free(sector_buf);
    ext_mem_free(tmp_sector_buf);
    return PASS;
}



///
///@ingroup DRIVE
///@brief   Read physical sector 0 (usually, the MBR sector) partition table on the storage device and dump on console for debugging purpose.
///
///@param   phyDevID        The physical device ID to dump its partition table info. \n\n
///
///@retval  PASS            Get partition table successfully. \n\n
///@retval  FAIL            Failed due to some error. \n\n
///@retval  FS_SCAN_FAIL    Failed due to invaild MBR (and thus invalid partition table).
///
SWORD DumpDevPartitionTableFromMBR(E_DEVICE_ID phyDevID)
{
    BYTE *sector_buf;
    PARTITION *partition;
    DWORD SectorSize;
    E_DRIVE_INDEX_ID drv_p0_index = PhyDevID2DriveIndex(phyDevID);
    DRIVE *drv = DriveGet(drv_p0_index);
    BYTE part_index;
    BOOL f_have_MBR = FALSE;
    DWORD start_lba;
    BYTE i;


    if (phyDevID == DEV_NULL)
    {
        MP_ALERT("%s: Error! Invalid NULL device ID !", __FUNCTION__);
        return FAIL;
    }
    else if ((phyDevID == DEV_USB_WIFI_DEVICE) ||\
             (phyDevID == DEV_CF_ETHERNET_DEVICE) ||\
             (phyDevID == DEV_USB_WEBCAM) ||\
             (phyDevID == DEV_USB_ETHERNET_DEVICE))
    {
        MP_ALERT("%s: Error! The device is not a storage device !", __FUNCTION__);
        return FAIL;
    }

    /* check card whether if present */
    if (! SystemCardPresentCheck(drv_p0_index))
    {
        MP_ALERT("%s: The storage device (drvID = %d) is not present !", __FUNCTION__, drv_p0_index);
        return FAIL;
    }

    MP_DEBUG("%s: total sectors count of the device (dev ID = %u) = %u", __FUNCTION__, phyDevID, Mcard_GetCapacity(phyDevID));

    SectorSize = Mcard_GetSectorSize(phyDevID);
    if (SectorSize == 0)
    {
        MP_ALERT("%s: Error! Sector size = 0 ! phyDevID = %d", __FUNCTION__, phyDevID);
        return FAIL;
    }

    /* make sure minimum necessary fields for Mcard Read/Write functions: drv->DevID and drv->bSectorExp */
    if (drv->DevID == 0)
        drv->DevID = phyDevID;

    if (drv->bSectorExp == 0)
    {
        // calculate the exponent of 2 for bytes-per-sector
        DWORD temp = SectorSize;
        drv->bSectorExp = 0;
        while (temp != 1)
        {
            temp >>= 1;
            drv->bSectorExp++;
        }
    }

    sector_buf = (BYTE *) ext_mem_malloc(SectorSize);
    if (sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        return FAIL;
    }
    sector_buf = (BYTE *) ((DWORD) sector_buf | BIT29);

    /* read MBR */
    Mcard_DeviceRead(drv, sector_buf, 0, 1);

    if (*(WORD *) (sector_buf + SIGNATURE_OFFSET) != COMMON_SIGNATURE)  /* check MBR sector signature (2 bytes: offset 510, 511) */
    {
        MP_ALERT("%s: -I- (MBR) Signature 0x55AA not found.", __FUNCTION__);
        ext_mem_free(sector_buf);
        return FS_SCAN_FAIL; /* here, this means failed to get vaild MBR */
    }

#if 0  /* for debug only => dump whole MBR sector content */
    mpDebugPrint("---- dumping MBR content ---- [begin] ----\r\n");
    BYTE x, y;
    for (x=0; x < 32; x++)
    {
        for (y=0; y < 16; y++)
            mpDebugPrintN("0x%02x ", sector_buf[y + (16 * x)]);

        mpDebugPrint("");
    }
    mpDebugPrint("\r\n---- dumping MBR content ---- [end] ----");
#endif

    BYTE *tmp_sector_buf = (BYTE *) ext_mem_malloc(SectorSize);
    if (tmp_sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        ext_mem_free(sector_buf);
        return FAIL;
    }
    tmp_sector_buf = (BYTE *) ((DWORD) tmp_sector_buf | BIT29);

    BYTE *pTmpByte;
    partition = (PARTITION *) (sector_buf + PARTITION_OFFSET);
    for (i = 0; i < 4; i++)
    {
        pTmpByte = (BYTE *) &partition[i];
        if (((BYTE) *pTmpByte != 0x80) && ((BYTE) *pTmpByte != 0x00))
        {
            MP_DEBUG("%s: Invalid Active (Bootable) flag of this partition table entry.", __FUNCTION__);
            continue; /* check next partition */
        }

        switch (partition[i].Type)
        {
            case 1:     // FAT12 type
            case 4:     // FAT16 type
            case 6:     // FAT16 type
            case 14:    // FAT16 type
            case 11:    // FAT32 type
            case 12:    // FAT32 type
            case 5:     // DOS extended type
            case 15:    // DOS extended type
            case 7:     // NTFS OS/2 HPFS type or exFAT type
                f_have_MBR = TRUE;
                MP_DEBUG("%s: known partition type (%d) => check boot sector ...", __FUNCTION__, partition[i].Type);
                start_lba = LoadAlien32(&partition[i].Start);  //LBA of the first sector in this partition
                break;
            default:    // not support partition type
                continue; /* check next partition */
        }

        /* check whether if boot sector exist at the sector specified by the partition table entry */
        if (Mcard_DeviceRead(drv, tmp_sector_buf, start_lba, 1) != FS_SUCCEED)
        {
            MP_ALERT("%s: Mcard_DeviceRead() failed !", __FUNCTION__);
            continue; /* check next partition */
        }

        if (*((WORD *) (tmp_sector_buf + SIGNATURE_OFFSET)) != COMMON_SIGNATURE)
        {
            MP_DEBUG("%s: Warning: (Boot Sector) Signature 0x55AA not found for this partition.", __FUNCTION__);
            continue; /* check next partition */
        }
    }

    ext_mem_free(sector_buf);
    ext_mem_free(tmp_sector_buf);

    if (!f_have_MBR)
    {
        MP_ALERT("%s: -I- sector 0 is actually a Boot Sector, not the MBR sector.", __FUNCTION__);
        return FS_SCAN_FAIL;
    }
    else
    {
        partition = (PARTITION *) (sector_buf + PARTITION_OFFSET);
        for (part_index = 0; part_index < 4; part_index++)
        {
            MP_ALERT("--- partition index = %d, type = 0x%02x, start LBA = %d, SectorNr = %d, BlockSize = %d",
                     part_index, partition[part_index].Type, LoadAlien32(&partition[part_index].Start), LoadAlien32(&partition[part_index].Size), SectorSize);
        }
        return PASS;
    }
}



void SetDrvPresentFlag(E_DRIVE_INDEX_ID drv_index_ID, BOOL flag)
{
    DRIVE *drv;

    if ((drv_index_ID == NULL_DRIVE) || (drv_index_ID >= MAX_DRIVE_NUM))
    {
        MP_ALERT("%s: Error! Invalid drive index ID ! drv_index_ID = %d", __FUNCTION__, drv_index_ID);
        return;
    }

    drv = &Drive[drv_index_ID];
    /* For non-storage or non-FAT devices, always set drv->Flag.Present as 1 for passing card present check */
    if ( (drv->DevID == DEV_USB_WIFI_DEVICE) || \
         (drv->DevID == DEV_CF_ETHERNET_DEVICE) || \
         ((drv->DevID == DEV_USB_HOST_PTP) || \
          (drv->DevID == DEV_USBOTG1_HOST_PTP)) ||\
          (drv->DevID == DEV_USB_WEBCAM)||\
          (drv->DevID == DEV_USB_ETHERNET_DEVICE))
    {
        return;
    }

    IntDisable();
    Drive[drv_index_ID].Flag.Present = flag;

#if (!MEM_2M)
    if (Drive[MAX_DRIVE_NUM].DevID == Drive[drv_index_ID].DevID)
    {
        Drive[MAX_DRIVE_NUM].Flag.Present = flag;
        if (flag == 0) /* unplugged */
        {
            Drive[MAX_DRIVE_NUM].DevID = MAX_DEVICE_DRV;
            Drive[MAX_DRIVE_NUM].DrvIndex = MAX_DRIVE_NUM;
            Drive[MAX_DRIVE_NUM].StatusCode = NOT_SUPPORT_FS;
        }
    }
#endif

    IntEnable();

    return;
}



/* Read the bit entry value (0 or 1) from the Allocation Bitmap with respect to the cluster number.
 * Note: Here, we use return value 0xFF to present error condition occurred.
 */
BYTE Read_AllocBitmapEntry(DRIVE * drv, DWORD cluster)
{
    BYTE content_byte;
    BYTE ret_bit;
    DWORD byte_offset;
    BYTE  bit_offset;
    BYTE  *AllocBitmap_table;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return 0xFF;
    }

    if (drv->Flag.FsType == FS_TYPE_exFAT)
    {
        if (drv->exFAT_InfoFileds == NULL)
        {
            MP_ALERT("%s: Error! drv->exFAT_InfoFileds == NULL !", __FUNCTION__);
            return 0xFF;
        }

        if ((drv->exFAT_InfoFileds->AllocBitmapContent[drv->exFAT_InfoFileds->VolumeFlags.ActiveFat] == NULL) || (drv->exFAT_InfoFileds->AllocBitmapSize[drv->exFAT_InfoFileds->VolumeFlags.ActiveFat] == 0))
        {
            MP_ALERT("%s: Error! The exFAT drive has no Allocation Bitmap !", __FUNCTION__);
            return 0xFF;
        }

        AllocBitmap_table = (BYTE *) drv->exFAT_InfoFileds->AllocBitmapContent[drv->exFAT_InfoFileds->VolumeFlags.ActiveFat];
    }
    else
    {
#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
        /* mimic exFAT's Allocation Bitmap for non-exFAT drives */
        if (drv->AllocBitmap_for_NonExFAT == NULL)
        {
            MP_ALERT("%s: Error! drv->AllocBitmap_for_NonExFAT == NULL !", __FUNCTION__);
            return 0xFF;
        }

        AllocBitmap_table = (BYTE *) drv->AllocBitmap_for_NonExFAT;
#endif
    }

    IntDisable();

    byte_offset = ((cluster - 2) / 8);
    bit_offset = ((cluster - 2) % 8);
    content_byte = (BYTE) *((BYTE *) AllocBitmap_table + byte_offset);
    ret_bit = (BYTE) (content_byte & (0x01 << bit_offset)); /* note: bit order for clusters: from LSB bit to MSB bit */

    IntEnable();

    return (ret_bit > 0)? 0x01:0x00;
}



/* Set the bit value to the cluster entry in the Allocation Bitmap.
 *
 * return  FS_SUCCEED         Set value successfully. \n\n
 * return  ABNORMAL_STATUS    Failed due to some error. \n\n
 * retval  INVALID_DRIVE      Invalid drive.
 */
int Set_AllocBitmapEntry(DRIVE * drv, DWORD cluster, BYTE bit_value)
{
    BYTE content_byte;
    BYTE *addr;
    DWORD byte_offset;
    BYTE  bit_offset;
    BYTE  *AllocBitmap_table;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (drv->Flag.FsType == FS_TYPE_exFAT)
    {
        if (drv->exFAT_InfoFileds == NULL)
        {
            MP_ALERT("%s: Error! drv->exFAT_InfoFileds == NULL !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        if ((drv->exFAT_InfoFileds->AllocBitmapContent[drv->exFAT_InfoFileds->VolumeFlags.ActiveFat] == NULL) || (drv->exFAT_InfoFileds->AllocBitmapSize[drv->exFAT_InfoFileds->VolumeFlags.ActiveFat] == 0))
        {
            MP_ALERT("%s: Error! The exFAT drive has no Allocation Bitmap !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        AllocBitmap_table = (BYTE *) drv->exFAT_InfoFileds->AllocBitmapContent[drv->exFAT_InfoFileds->VolumeFlags.ActiveFat];
    }
    else
    {
#if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
        /* mimic exFAT's Allocation Bitmap for non-exFAT drives */
        if (drv->AllocBitmap_for_NonExFAT == NULL)
        {
            MP_ALERT("%s: Error! drv->AllocBitmap_for_NonExFAT == NULL !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        AllocBitmap_table = (BYTE *) drv->AllocBitmap_for_NonExFAT;
#endif
    }

    if (bit_value > 1)
    {
        MP_ALERT("%s: Error! Invalid input value !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    IntDisable();

    byte_offset = ((cluster - 2) / 8);
    bit_offset = ((cluster - 2) % 8);

    addr = (BYTE *) ((BYTE *) AllocBitmap_table + byte_offset);
    content_byte = (BYTE) *addr;

    /* note: bit order for clusters: from LSB bit to MSB bit */
    if (bit_value & 0x01)
        (BYTE) *addr = (BYTE) (content_byte | (0x01 << bit_offset));
    else
        (BYTE) *addr = (BYTE) (content_byte & (~(0x01 << bit_offset)));

    IntEnable();

    return FS_SUCCEED;
}




MPX_KMODAPI_SET(DriveGet);
MPX_KMODAPI_SET(DriveAdd);
MPX_KMODAPI_SET(DriveChange);
MPX_KMODAPI_SET(DriveRefresh);
MPX_KMODAPI_SET(GetDrvPartitionInfoFromMBR);

