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
* Filename      : exFAT.h
* Programmers   : WeiChing Tu
* Created       :
* Descriptions  : Support exFAT file system.
*******************************************************************************
*/
#ifndef __EXFAT_H
#define __EXFAT_H


/*
*******************************************************************************
*        #include
*******************************************************************************
*/
#include "UtilTypeDef.h"
#include "file.h"


/*
*******************************************************************************
*        #define CONSTANTS
*******************************************************************************
*/

#define EXFAT_FIRST_DATA_CLUSTER      2
#define EXFAT_ENTRY_FILE_NAME_LEN    15  /* 15 UTF-16 characters in a exFAT File Name DirEntry */

/* state of cluster */
#define EXFAT_CLUSTER_FREE            0  /* free cluster */
#define EXFAT_CLUSTER_BAD    0xfffffff7  /* cluster contains bad block */
#define EXFAT_CLUSTER_END    0xffffffff  /* final cluster of file or directory */

/* exFAT file attributes == FAT32 file attributes. But exFAT has no FDB_LONG_NAME */
#define EXFAT_FDB_ATTRIB_RO        0x01  /* == FDB_READ_ONLY */
#define EXFAT_FDB_ATTRIB_HIDDEN    0x02  /* == FDB_HIDDEN */
#define EXFAT_FDB_ATTRIB_SYSTEM    0x04  /* == FDB_SYSTEM */
#define EXFAT_FDB_ATTRIB_VOLUME    0x08  /* == FDB_LABEL */
#define EXFAT_FDB_ATTRIB_DIR       0x10  /* == FDB_SUB_DIR */
#define EXFAT_FDB_ATTRIB_ARCH      0x20  /* == FDB_ARCHIVE */

/* bit mask for bit fields in exFAT EntryType */
#define EXFAT_EntryType_MASK_TypeCode         0x1F
#define EXFAT_EntryType_MASK_TypeImportance   0x20
#define EXFAT_EntryType_MASK_TypeCategory     0x40
#define EXFAT_EntryType_MASK_InUse            0x80

/* exFAT defined DirectoryEntry type values */
#define EXFAT_ENTRY_TYPE_EOD                   0x00
#define EXFAT_ENTRY_TYPE_BITMAP                0x81
#define EXFAT_ENTRY_TYPE_UPCASE_TABLE          0x82
#define EXFAT_ENTRY_TYPE_VOL_LABEL             0x83
#define EXFAT_ENTRY_TYPE_FILE                  0x85
#define EXFAT_ENTRY_TYPE_VOL_GUID              0xA0
#define EXFAT_ENTRY_TYPE_STREAM_EXT            0xC0
#define EXFAT_ENTRY_TYPE_FILE_NAME             0xC1
#define EXFAT_ENTRY_TYPE_VENDOR_EXT            0xE0
#define EXFAT_ENTRY_TYPE_VENDOR_ALLOC          0xE1
/* note: EXFAT_ENTRY_TYPE_CONTINU_INFO_MANAGE is an pre-defined instance of EXFAT_ENTRY_TYPE_VENDOR_EXT type */
/* note: EXFAT_ENTRY_TYPE_CONTINU_INFO is an pre-defined instance of EXFAT_ENTRY_TYPE_VENDOR_ALLOC type */
#define EXFAT_ENTRY_TYPE_CONTINU_INFO_MANAGE   0xE0
#define EXFAT_ENTRY_TYPE_CONTINU_INFO          0xE1

/* note: Observed DirectoryEntry type values using WinHex, not found in exFAT Spec (Ver 3.00 Draft 1.00) from SD Card Association */
#if 1
#define EXFAT_ENTRY_TYPE_FILE_DELETED          0x05
#define EXFAT_ENTRY_TYPE_STREAM_EXT_DELETED    0x40
#define EXFAT_ENTRY_TYPE_FILE_NAME_DELETED     0x41
#endif

#if 1
/* below types are used in WinCE, but their type values and structures are not defined in exFAT 1.00 spec */
#define EXFAT_ENTRY_TYPE_TEXFAT_PADDING           /* unknown, not defined in spec */
#define EXFAT_ENTRY_TYPE_WINCE_ACCESS_CTRL_TABLE  /* unknown, not defined in spec */
#define EXFAT_ENTRY_TYPE_WINCE_ACCESS_CTRL        /* unknown, not defined in spec */
#endif

/* bit mask for bit fields in VolumeFlags of exFAT Boot Sectors */
#define EXFAT_VolumeFlags_MASK_ActiveFat        0x01
#define EXFAT_VolumeFlags_MASK_VolDirty         0x02
#define EXFAT_VolumeFlags_MASK_MediaFail        0x04
#define EXFAT_VolumeFlags_MASK_ClearToZero      0x08

/* bit mask for bit fields in BitmapFlags of exFAT Allocation Bitmap DirectoryEntry */
#define EXFAT_BitmapFlags_MASK_BitmapId         0x01

/* bit mask for flag fields in GeneralPrimaryFlags and GeneralSecondaryFlags in some of exFAT DirEntries */
#define EXFAT_DirEntryFlags_MASK_AllocPossible  0x01
#define EXFAT_DirEntryFlags_MASK_NoFatChain     0x02


/*
*******************************************************************************
*        GLOBAL DATA TYPES
*******************************************************************************
*/

//Main and Backup Boot Sector
typedef struct _exfat_boot_sector
{
    BYTE   jumpBoot[3];              /* jmp instructions, should be 0xEB, 0x76, 0x90 */
    BYTE   fsName[8];                /* file system name, "EXFAT   " */
    BYTE   zeroBytes[53];            /* always 0 */
    U64    partitionOffset;          /* partition start block */
    U64    volumeLength;             /* partition blocks count */
    DWORD  fatOffset;                /* FAT start block */
    DWORD  fatLength;                /* FAT blocks count */
    DWORD  clusterHeapOffset;        /* start block of cluster heap */
    DWORD  clusterCount;             /* total clusters count */
    DWORD  rootDir_firstCluster;     /* first cluster of the root dir */
    DWORD  volSerial;                /* volume serial number */
    WORD   fsRevision;               /* file system revision number */
    WORD   volumeFlags;              /* volume flags */
    BYTE   bytesPerSectorShift;      /* bytes per sector bit shift, block size as (1 << n) */
    BYTE   sectorsPerClusterShift;   /* blocks per cluster bit shift, cluster size as (1 << n) */
    BYTE   numberOfFats;             /* number of FATs, always 1 for exFAT */
    BYTE   driveSelect;              /* drive select, always 0x80 */
    BYTE   percentInUse;             /* percentage of allocated space */
    BYTE   reserved[7];              /* reserved */
    BYTE   bootCode[390];            /* boot code */
    WORD   bootSignature;            /* boot sector signature: 0x55, 0xAA */
} TYPE_exFAT_Boot_Sector;

//Main and Backup Extended Boot Sectors
typedef struct _exfat_extended_boot_sector
{
    BYTE   extendedBootCode[508];    /* extended boot code */
    DWORD  extendedBootSignature;    /* extended boot sector signature: 0x00, 0x00, 0x55, 0xAA */
} TYPE_exFAT_Extended_Boot_Sector;


/* note: all exFAT DirEntry types below are variants of the fixed 32-byte FDB type */
/*       Do not modify/move any data member field in the following exFAT DirEntry structures */

//Generic DirEntry Template
typedef struct _exFAT_generic_DirEntry_template
{
    BYTE   entryType;
    BYTE   customDefined[19];
    DWORD  firstCluster;
    U64    dataLength;
} TYPE_exFAT_Generic_DirEntry_Template;

//Generic Primary DirEntry Template
typedef struct _exFAT_generic_primary_DirEntry_template
{
    BYTE   entryType;
    BYTE   secondaryCount;
    WORD   setChksum;
    WORD   generalPrimaryFlags;
    BYTE   customDefined[14];
    DWORD  firstCluster;
    U64    dataLength;
} TYPE_exFAT_Generic_Primary_DirEntry_Template;

//Generic Secondary DirEntry Template
typedef struct _exFAT_generic_secondary_DirEntry_template
{
    BYTE   entryType;
    BYTE   generalSecondaryFlags;
    BYTE   customDefined[18];
    DWORD  firstCluster;
    U64    dataLength;
} TYPE_exFAT_Generic_Secondary_DirEntry_Template;

//Allocation Bitmap DirEntry
typedef struct _exFAT_Alloc_Bitmap_DirEntry
{
    BYTE   entryType;
    BYTE   bitmapFlags;
    BYTE   reserved[18];
    DWORD  firstCluster;
    U64    dataLength;
} TYPE_exFAT_Alloc_Bitmap_DirEntry;

//Up-case Table DirEntry
typedef struct _exFAT_Upcase_Table_DirEntry
{
    BYTE   entryType;
    BYTE   reserved1[3];
    DWORD  tableChksum;
    BYTE   reserved2[12];
    DWORD  firstCluster;
    U64    dataLength;
} TYPE_exFAT_Upcase_Table_DirEntry;

//Volume Label DirEntry
typedef struct _exFAT_Vol_Label_DirEntry
{
    BYTE   entryType;
    BYTE   charCount;
    BYTE   volLabel[22];   /* UTF-16 volume label string */
    BYTE   reserved[8];
} TYPE_exFAT_Vol_Label_DirEntry;

//File DirEntry
typedef struct _exFAT_File_DirEntry
{
    BYTE   entryType;
    BYTE   secondaryCount;
    WORD   setChksum;
    WORD   fileAttributes;
    BYTE   reserved1[2];
    DWORD  createTimestamp;
    DWORD  lastModifyTimestamp;
    DWORD  lastAccessTimestamp;
    BYTE   create10msIncrement;
    BYTE   lastModify10msIncrement;
    BYTE   createUtcOffset;
    BYTE   lastModifyUtcOffset;
    BYTE   lastAccessUtcOffset;
    BYTE   reserved2[7];
} TYPE_exFAT_File_DirEntry;

//Volume GUID DirEntry
typedef struct _exFAT_Vol_GUID_DirEntry
{
    BYTE   entryType;
    BYTE   secondaryCount;
    WORD   setChksum;
    WORD   generalPrimaryFlags;
    BYTE   volGUID[16];
    BYTE   reserved[10];
} TYPE_exFAT_Vol_GUID_DirEntry;

//Stream Extension DirEntry
typedef struct _exFAT_Stream_Extension_DirEntry
{
    BYTE   entryType;
    BYTE   generalSecondaryFlags;
    BYTE   reserved1;
    BYTE   nameLength;
    WORD   nameHash;
    WORD   reserved2;
    U64    validDataLength;
    DWORD  reserved3;
    DWORD  firstCluster;
    U64    dataLength;
} TYPE_exFAT_Stream_Extension_DirEntry;

//File Name DirEntry
typedef struct _exFAT_File_Name_DirEntry
{
    BYTE   entryType;
    BYTE   generalSecondaryFlags;
    BYTE   fileName[30];           /* UTF-16 filename string => 15 characters */
} TYPE_exFAT_File_Name_DirEntry;

//Vendor Extension DirEntry Template
typedef struct _exFAT_Vendor_Extension_DirEntry_template
{
    BYTE   entryType;
    BYTE   generalSecondaryFlags;
    BYTE   vendorGUID[16];
    BYTE   vendorDefined[14];
} TYPE_exFAT_Vendor_Extension_DirEntry_Template;

//Vendor Allocation DirEntry Template
typedef struct _exFAT_Vendor_Allocation_DirEntry_template
{
    BYTE   entryType;
    BYTE   generalSecondaryFlags;
    BYTE   vendorGUID[16];
    BYTE   vendorDefined[2];
    DWORD  firstCluster;
    U64    dataLength;
} TYPE_exFAT_Vendor_Allocation_DirEntry_Template;

//Continuous Information Manage DirEntry
typedef struct _exFAT_Continu_Info_Manage_DirEntry
{
    BYTE   entryType;
    BYTE   generalSecondaryFlags;
    BYTE   vendorGUID[16];
    BYTE   reserved1[2];
    DWORD  fatChksum;
    BYTE   reserved2[8];
} TYPE_exFAT_Continu_Info_Manage_DirEntry;

//Continuous Information DirEntry
typedef struct _exFAT_Continu_Info_DirEntry
{
    BYTE   entryType;
    BYTE   generalSecondaryFlags;
    BYTE   vendorGUID[16];
    WORD   setChksum;
    DWORD  firstCluster;
    U64    dataLength;
} TYPE_exFAT_Continu_Info_DirEntry;


/*
*******************************************************************************
*        GLOBAL FUNCTION PROTOTYPES
*******************************************************************************
*/

int   Load_exFatAllocBitmap(DRIVE * drv);
int   Load_exFatUpcaseTable(DRIVE * drv);
WORD  Cal_exFatFilenameHash(WORD * utf16_filename, BYTE name_length);
int   ScanAllocBitmap_FreeClusters(DRIVE * drv);
void  FileGetDateTime_for_exFAT(DWORD timestamp, DATE_TIME_INFO_TYPE * date_time_info);
DWORD FileSetDateTime_for_exFAT(DATE_TIME_INFO_TYPE date_time_info);


#endif //__EXFAT_H
