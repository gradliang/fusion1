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
* Filename      : drive.h
* Programmers   :
* Created       :
* Descriptions  :
*******************************************************************************
*/
#ifndef __DRIVE_H
#define __DRIVE_H

#include "utiltypedef.h"
#include "devio.h"
#include "fat.h"

/* note: this drive index ID enumeration will be included in file system module Doxygen document.  [begin] */
/// ID number of the memory card
///
///@ingroup     FS_CONST
///@brief       Enumeration constants of drive index ID for all drives of storage devices.
///@{
enum
{
    NULL_DRIVE = 0,     ///< For Null Drive
    // OTG-0
    USB_HOST_ID1,       ///< For USB_HOST_ID1
    USB_HOST_ID2,       ///< For USB_HOST_ID2
    USB_HOST_ID3,       ///< For USB_HOST_ID3
    USB_HOST_ID4,       ///< For USB_HOST_ID4
    USB_HOST_PTP,       ///< For USB_PTP
    // OTG-1
    USBOTG1_HOST_ID1,   ///< For USBOTG1_HOST_ID1
    USBOTG1_HOST_ID2,   ///< For USBOTG1_HOST_ID2
    USBOTG1_HOST_ID3,   ///< For USBOTG1_HOST_ID3
    USBOTG1_HOST_ID4,   ///< For USBOTG1_HOST_ID4
    USBOTG1_HOST_PTP,   ///< For USBOTG1_PTP
    NAND_ISP,           ///< For NAND ISP
    NAND_PART1,         ///< For NAND partition 1
    NAND_PART2,         ///< For NAND partition 2
    NAND_PART3,         ///< For NAND partition 3
    NAND_PART4,         ///< For NAND partition 4
    SM,                 ///< For SM card
    XD,                 ///< For XD card
    MS,                 ///< For MS card
    SD_MMC_PART1,       ///< For SD partition 1
    SD_MMC_PART2,       ///< For SD partition 2
    SD_MMC_PART3,       ///< For SD partition 3
    SD2,                ///< For SD card 2
    CF,                 ///< For CF card
    HD,                 ///< For HD Drive
    HD2,                ///< For HD Drive partition 2
    HD3,                ///< For HD Drive partition 3
    HD4,                ///< For HD Drive partition 4
    SPI_FLASH_PART1,    ///< For SPI Flash partition 1
    SPI_FLASH_PART2,    ///< For SPI Flash partition 2
    SPI_FLASH_ISP,      ///< For SPI Flash ISP
    SDIO,               ///< For SDIO
    USB_WIFI_DEVICE,    ///< For USB_WiFi
    USB_PPP,            ///< For USB_PPP
    CF_ETHERNET_DEVICE, ///< For CF_ETHERNET
    USB_ETHERNET_DEVICE, ///< For USB ETHERNET
    USB_WEBCAM_DEVICE,  ///< For USB Webcam
    MAX_DRIVE           ///< Maximum number of drives
};  /* used for all Drive[] table entries */

/* note: drive index 'NAND' and 'SPI_FLASH' values are defined in SystemConfig.h according to SYS_DRV_ID value now.
 *       drive index id 'NAND' is for the user drive on NAND FLASH device;
 *       drive index id 'SPI_FLASH' is for the user drive on SPI FLASH device.
 */

///
///@ingroup     FS_CONST
///@brief       Type definition for the enumeration constants of drive index ID.
typedef BYTE  E_DRIVE_INDEX_ID; /* force E_DRIVE_INDEX_ID type to be 1-byte integer */

///@ingroup     FS_CONST
///@brief       Maximum number of drives
#define MAX_DRIVE_NUM               MAX_DRIVE
///@}

/* note: this drive ID enumeration will be included in file system module Doxygen document.  [end] */



/* for System Drive related drive index ID definitions  [begin] */
#ifndef SYSTEM_DRIVE
    #define SYSTEM_DRIVE        SYSTEM_DRIVE_NONE
#endif

#if ((SYSTEM_DRIVE != SYSTEM_DRIVE_NONE) && ((SYS_DRV_SIZE == 0) || !defined(SYS_DRV_SIZE)))
    #error "Error!  Please re-config SYS_DRV_SIZE setting to non-zero value !!"
#endif

#if (SYSTEM_DRIVE == SYSTEM_DRIVE_NONE)
    #define SYS_DRV_ID          NULL_DRIVE
    #define TOOL_DRV_ID         NULL_DRIVE
    #define NAND                NAND_PART1
    #define SPI_FLASH           SPI_FLASH_PART1
    #define SD_MMC              SD_MMC_PART1
#elif (SYSTEM_DRIVE == SYSTEM_DRIVE_ON_NAND)
    #if NAND_ENABLE
        #define SYS_DRV_ID      NAND_PART1

        #if (TOOL_DRV_SIZE)
        #define TOOL_DRV_ID     NAND_PART2
        #define NAND            NAND_PART3
        #else
        #define TOOL_DRV_ID     NULL_DRIVE
        #define NAND            NAND_PART2
        #endif

        #define SPI_FLASH       SPI_FLASH_PART1
        #define SD_MMC          SD_MMC_PART1
    #else
        #define SYS_DRV_ID      NULL_DRIVE
        #define TOOL_DRV_ID     NULL_DRIVE
        #define NAND            NAND_PART1
        #define SPI_FLASH       SPI_FLASH_PART1
        #define SD_MMC          SD_MMC_PART1
    #endif
#elif (SYSTEM_DRIVE == SYSTEM_DRIVE_ON_SPI)
    #if SPI_STORAGE_ENABLE
        #define SYS_DRV_ID      SPI_FLASH_PART1
        #define TOOL_DRV_ID     NULL_DRIVE
        #define NAND            NAND_PART1
        #define SPI_FLASH       SPI_FLASH_PART2
        #define SD_MMC          SD_MMC_PART1
    #else
        #define SYS_DRV_ID      NULL_DRIVE
        #define TOOL_DRV_ID     NULL_DRIVE
        #define NAND            NAND_PART1
        #define SPI_FLASH       SPI_FLASH_PART1
        #define SD_MMC          SD_MMC_PART1
    #endif
#elif (SYSTEM_DRIVE == SYSTEM_DRIVE_ON_SD)
        #define SYS_DRV_ID      SD_MMC_PART1
        #define NAND            NAND_PART1
        #define SPI_FLASH       SPI_FLASH_PART1

        #if (TOOL_DRV_SIZE)
        #define TOOL_DRV_ID     SD_MMC_PART2
        #define SD_MMC          SD_MMC_PART3
        #else
        #define TOOL_DRV_ID     NULL_DRIVE
        #define SD_MMC          SD_MMC_PART2
        #endif
#else
    #define SYS_DRV_ID          NULL_DRIVE
    #define TOOL_DRV_ID         NULL_DRIVE
    #define NAND                NAND_PART1
    #define SPI_FLASH           SPI_FLASH_PART1
    #define SD_MMC              SD_MMC_PART1
#endif

/* for System Drive related drive index ID definitions  [end] */


/* for CreateNewDiskPartition() */
#define ALLOC_ALL_LEFT_DISK_SPACE    0xFFFFFFFF

///
///@ingroup     FS_CONST
///@brief       Enumeration constants of operation types for how to creating a new disk partition.
typedef enum {
    E_ERASE_OLD_PARTITION_TABLE_FIRST,    ///< Force erasing old partition table first before creating a new disk partition
    E_BEHIND_EXISTING_PARTITION_ENTRIES   ///< Create a new disk partition directly behind existing partitions
} DISK_PARTITIONING_OP_TYPE;


void  DriveHandleCopy(DRIVE * drv1, DRIVE * drv2);  // for xpgslidefunc use
void  FileSystemInit(void);
int   DriveAdd(E_DRIVE_INDEX_ID drv_index_ID);
void  DriveDelete(E_DRIVE_INDEX_ID drv_index_ID);
void  SingleDriveDelete(E_DRIVE_INDEX_ID drv_index_ID);
void  RenewAllDrv(void);
DRIVE *DriveChange(E_DRIVE_INDEX_ID drv_index_ID);
DRIVE *DriveGet(E_DRIVE_INDEX_ID drv_index_ID);
E_DRIVE_INDEX_ID  DriveCurIdGet(void);
BYTE  DriveCountGet(void);
BYTE  DriveCountUpdate(void);
DWORD DriveFreeClustersCountGet(DRIVE * drv);
DWORD DriveFreeSizeGet(DRIVE * drv);  /* unit : sectors */
DWORD DriveSetcorSizeGet(DRIVE * drv);
DWORD DriveTotalSizeGet(DRIVE * drv);
DWORD DriveNewClusGet(DRIVE * drv);
int   DriveRead(DRIVE * drv, BYTE * buffer, DWORD lba, DWORD len);
int   DriveWrite(DRIVE * drv, BYTE * buffer, DWORD lba, DWORD len);
int   DriveRefresh(DRIVE * drv);
int   FatCaching(register DRIVE * drv, DWORD sector);
int   QuickFormat(DRIVE * drv);
SWORD Drive_Formatting(E_DRIVE_INDEX_ID drive_index, char * label);
SWORD Fat32_Format(DRIVE * drv, char * label);
SWORD Fat16_Format(DRIVE * drv, char * label);
SWORD Fat12_Format(DRIVE * drv, char * label);
SWORD DiskPartitioning_WithMaxTwoPartitions(E_DRIVE_INDEX_ID drv_index_ID, DWORD partition2_size);
SWORD CreateNewDiskPartition(E_DEVICE_ID phyDevID, DWORD new_partition_size, DISK_PARTITIONING_OP_TYPE op_type, DWORD * left_unused_space_size);
SWORD GetDrvPartitionInfoFromMBR(E_DRIVE_INDEX_ID drv_index_ID, BYTE * partition_idx, BYTE * partition_type, DWORD * partition_start_lba,
                                 DWORD * partition_SectorNr, DWORD * partition_blockSize);
SWORD DumpDevPartitionTableFromMBR(E_DEVICE_ID phyDevID);
void  SetDrvPresentFlag(E_DRIVE_INDEX_ID drv_index_ID, BOOL flag);
BYTE  Read_AllocBitmapEntry(DRIVE * drv, DWORD cluster);
int   Set_AllocBitmapEntry(DRIVE * drv, DWORD cluster, BYTE bit_value);


#endif //__DRIVE_H

