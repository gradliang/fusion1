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
* Filename      : fat.h
* Programmers   : 
* Created       : 
* Descriptions  : 
*******************************************************************************
*/
#ifndef FAT_H
#define FAT_H


/*
*******************************************************************************
*        #define CONSTANTS
*******************************************************************************
*/

/* file system type values for Flag.FsType in DRIVE structure */
///
///@ingroup     FS_CONST
///@brief       Enumeration constants of supported file system types.
typedef enum
{
	FS_TYPE_UNSUPPORT = 0,  ///< unsupported file system type
	FS_TYPE_FAT12     = 1,  ///< FAT12 file system type
	FS_TYPE_FAT16     = 2,  ///< FAT16 file system type
	FS_TYPE_FAT32     = 3,  ///< FAT32 file system type
	FS_TYPE_exFAT     = 4   ///< exFAT file system type
} FS_TYPE;


///@ingroup     FS_CONST
///@brief       Power of 2 for the size of FAT cache in number of sectors
#define FAT_CACHE_SIZE_IN_SECTORS_EXP    2  // 4 sectors in FAT cache

///@ingroup     FS_CONST
///@brief       The size of FAT cache in number of bytes
#define FAT_CACHE_BYTE_SIZE      (4096 * (1 << FAT_CACHE_SIZE_IN_SECTORS_EXP))  /* reserved enough space for iPod 4KB/sector (from MP62X/630 code) */

///@ingroup     FS_CONST
///@brief       The size of directory cache in number of FDBs, each FDB is fixed 32-bytes long
#define DIR_CACHE_IN_FDB_COUNT   (16 * 4 * 2)  /* for 1 sector, but (16 * 4 * 2) = 4KB due to reserved enough space for iPod 4KB/sector (from MP62X/630 code) */

///@ingroup     FS_CONST
///@brief       Maximum number of files opened concurrently
#define FS_MAX_STREAM       0x10

///@ingroup     FS_CONST
///@brief       The maximum level of subdirectory
#define DIR_STACK_DEEP      0x20

///@ingroup     FS_CONST
///@brief       The maximun path name length
#define MAX_PATH_LENG       1024 

///@ingroup     FS_CONST
///@brief       The maximun number of characters a long filename can contain
#define MAX_L_NAME_LENG     256 

///@ingroup     FS_CONST
///@brief       Offset value of 0x55AA signature
#define SIGNATURE_OFFSET    0x01fe            // will contain 0x55AA

///@ingroup     FS_CONST
///@brief       Offset value of partition table
#define PARTITION_OFFSET    0x01be            // will contain partition table

///@ingroup     FS_CONST
///@brief       Offset value of FSINFO structure
#define FSINFO_OFFSET       0x01e4

///@ingroup     FS_CONST
///@brief       0x55AA signature
#define COMMON_SIGNATURE    0x55AA

///@ingroup     FS_CONST
///@brief       Value of FSINFO signature
#define FSINFO_SIGNATURE    0x72724161L

///@ingroup     FS_CONST
///@brief       The default size of a sector
#define SECTOR_SIZE			512

///@ingroup     FS_CONST
///@brief       The common value for "end of chain" for ReadFatXX(), regardless of different bit numbers between FAT12, FAT16 and FAT32.
#define FAT_READ_END_OF_CHAIN   0xFFFFFFFF 

///@ingroup     FS_CONST
///@brief       FAT entry value for "end of chain" in FAT32
#define EOC_FAT32               0x0FFFFFFF 

///@ingroup     FS_CONST
///@brief       FAT entry value for "end of chain" in FAT16
#define EOC_FAT16               0xFFFF 

///@ingroup     FS_CONST
///@brief       FAT entry value for "end of chain" in FAT12
#define EOC_FAT12               0xFFF 

///@ingroup     FS_CONST
///@brief       FAT entry value for "cluster 0" in FAT32
#define CLUSTER0_FAT32          0x0FFFFFF8 

///@ingroup     FS_CONST
///@brief       FAT entry value for "cluster 0" in FAT16
#define CLUSTER0_FAT16          0xFFF8 

///@ingroup     FS_CONST
///@brief       FAT entry value for "cluster 0" in FAT12
#define CLUSTER0_FAT12          0xFF8 

///@ingroup     FS_CONST
///@brief       Default count of sectors to reserve before boot sector of 1st partition for disk partitioning function
#define Default_BPB_HiddSec     32 

///@ingroup     FS_CONST
///@brief       Count of sectors to reserve before boot sector of 1st partition on tiny-size storage device for disk partitioning function
#define Small_BPB_HiddSec       2 


/* Return code and Error code */

///@ingroup     FS_ERROR_CODE
///@brief       The function operated successfully without any problem
#define FS_SUCCEED              0L

///@ingroup     FS_ERROR_CODE
///@brief       Invalid DRIVE handle or no DRIVE handle can use
#define INVALID_DRIVE           -1L

///@ingroup     FS_ERROR_CODE
///@brief       Over FAT12/FAT16 root directory region boundary
#define OVER_ROOT_BOUNDARY      -2L

///@ingroup     FS_ERROR_CODE
///@brief       No enough free space in the drive
#define DISK_FULL               -3L

///@ingroup     FS_ERROR_CODE
///@brief       The seek destination position out of range
#define OUT_OF_RANGE            -4L

///@ingroup     FS_ERROR_CODE
///@brief       Unknown or not supported file system
#define NOT_SUPPORT_FS          -5L

///@ingroup     FS_ERROR_CODE
///@brief       FDB node is in the directory which is reached end
#define END_OF_DIR              -6L

///@ingroup     FS_ERROR_CODE
///@brief       FDB node is at the beginning position in the dirctory
#define BEGIN_OF_DIR            -7L

///@ingroup     FS_ERROR_CODE
///@brief       Specified file not found
#define FILE_NOT_FOUND          -8L

///@ingroup     FS_ERROR_CODE
///@brief       Scanning down the FDB node one by one for extracting file name should not stop yet 
#define FILENAME_SCAN_CONTINUE  -9L

///@ingroup     FS_ERROR_CODE
///@brief       Unknow error encountered
#define ABNORMAL_STATUS         -10L

///@ingroup     FS_ERROR_CODE
///@brief       Read / write drive fail
#define DRIVE_ACCESS_FAIL		-11L

///@ingroup     FS_ERROR_CODE
///@brief       Drive file system scan fail
#define FS_SCAN_FAIL			-12L

///@ingroup     FS_ERROR_CODE
///@brief       Operation failed due to disk drive is read only
#define DISK_READ_ONLY			-13L

///@ingroup     FS_ERROR_CODE
///@brief       Copy failed due to the user cancelled the copy operation
#define USER_CANCEL_COPY		-14L

///@ingroup     FS_ERROR_CODE
///@brief       Operation failed/stopped due to the target drive is NAND's partition 2 and a same name file already exists
#define FIND_SAME_FILE_NOT_COPY	-15L //rick for nand 2 partition

///@ingroup     FS_ERROR_CODE
///@brief       Depth limit of Drive's DirStack buffer has been reached
#define FS_DIR_STACK_LIMIT_REACHED    -16L


/*
*******************************************************************************
*        GLOBAL MACROS
*******************************************************************************
*/

#define MB_SIZE(byte_len)            ((byte_len) >> 20)
#define KB_SIZE(byte_len)            ((byte_len) >> 10)


/*
*******************************************************************************
*        GLOBAL DATA TYPES 
*******************************************************************************
*/

///
///@ingroup     DATA_STRUCT
///@brief       Partition entries definition.
typedef struct            // Partition Entries definition
{
	BYTE Active;  ///< Active (bootable) flag of this partition
	BYTE res1;    ///< Part of lagacy CHS address of first block in partition. Not used here.
	BYTE res2;    ///< Part of lagacy CHS address of first block in partition. Not used here.
	BYTE res3;    ///< Part of lagacy CHS address of first block in partition. Not used here.
	BYTE Type;    ///< File system type of this partition
	BYTE res4;    ///< Part of lagacy CHS address of last block in partition. Not used here.
	BYTE res5;    ///< Part of lagacy CHS address of last block in partition. Not used here.
	BYTE res6;    ///< Part of lagacy CHS address of last block in partition. Not used here.
	DWORD Start;  ///< LBA of the first sector in this partition
	DWORD Size;   ///< Length (number of blocks) of this partion, in little-endian format
} PARTITION;


///
///@ingroup     DATA_STRUCT
///@brief       Partition Boot Record definition, in little-endian format.
typedef struct          // Partition Boot Record definition
{
	BYTE JumpCode[3];           ///< Jump instruction to bootstrap code 
	BYTE OEM[8];                ///< OEM name/version 
	BYTE BytesPerSector;        ///< First byte of number of bytes per sector. Splitting to two byte fields is for alignment issue
	BYTE BytesPerSector_B2;     ///< Second byte of number of bytes per sector. Splitting to two byte fields is for alignment issue
	BYTE SectorsPerCluster;     ///< Number of sectors per cluster 
	WORD ReservedSectors;       ///< Number of reserved sectors 
	BYTE NumberOfFATs;          ///< Number of FAT copies 
	BYTE RootEntries;           ///< First byte of number of root directory entries (FDBs). Splitting to two byte fields is for alignment issue
	BYTE RootEntries_B2;        ///< Second byte of number of root directory entries (FDBs). Splitting to two byte fields is for alignment issue
	BYTE TotalSectors;          ///< First byte of old 16-bit FAT12/16 total number of sectors on the volume. Splitting to two byte fields is for alignment issue
	BYTE TotalSectors_B2;       ///< Second byte of old 16-bit FAT12/16 total number of sectors on the volume. Splitting to two byte fields is for alignment issue
	BYTE MediaDescritor;        ///< Media descriptor type 
	WORD SectorsPerFAT;         ///< Old 16-bit FAT12/16 total number of sectors per FAT table 
	WORD SectorsPerTrack;       ///< Number of sectors per track 
	WORD Heads;                 ///< Number of heads 
	DWORD HiddenSectors;        ///< Number of hidden sectors 
	DWORD BigTotalSectors;      ///< New 32-bit total number of sectors on the volume 
	DWORD BigSectorPerFAT;      ///< New 32-bit total number of sectors per FAT table for FAT32 volume 
	WORD ExtFlags;              ///< FAT Flags for FAT32 media 
	WORD FS_Version;            ///< Version for FAT32 media 
	DWORD RootDirStartCluster;  ///< Cluster number of root directory start 
	WORD FSInfoSector;          ///< LBA of FSINFO Sector for FAT32 volume 
	WORD BackUpBootSector;      ///< LBA of the backup copy of this boot sector 
} BOOT_SECTOR;


///
///@ingroup     DATA_STRUCT
///@brief       FSINFO structure information.
typedef struct
{
	DWORD Signature;        ///< The FSInfo signature
	DWORD FreeClusters;     ///< Free cluster count 
	DWORD NextFree;         ///< Next free cluster
} FSINFO;


///
///@ingroup     DATA_STRUCT
///@brief       Short name FDB (file description block) definition, in little-endian format.
typedef struct
{
	BYTE	Name[8];        ///< 8-char primary part of 8.3 format short filename
	BYTE	Extension[3];   ///< 3-char extension part of 8.3 format short filename
	BYTE	Attribute;      ///< File attribute
	WORD	Attribute2;     ///< Actually, 1-byte reserved and 1-byte millisecond stamp of file creation time
	WORD	CreateTime;     ///< File creation time
	WORD	CreateDate;     ///< File creation date
	WORD	AccessDate;     ///< Last access date
	WORD	StartHigh;      ///< High word of this entry's first cluster number
	WORD	ModifyTime;     ///< Time of last write
	WORD	ModifyDate;     ///< Date of last write
	WORD	StartLow;       ///< Low word of this entry's first cluster number
	DWORD	Size;           ///< File size
} FDB;


///
///@ingroup     DATA_STRUCT
///@brief       Long name FDB definition.
typedef struct
{
	BYTE		Number;       ///< Order number of this LFN FDB 
	BYTE		Name0[10];    ///< UTF-16 characters 1~5 of name entry
	BYTE		Attribute1;   ///< File attribute
	BYTE		Attribute2;   ///< Should be 0x00
	BYTE		CheckSum;     ///< Checksum of short file name
	BYTE		Name1[12];    ///< UTF-16 characters 6~11 of name entry
	BYTE		res[2];       ///< Must be 0x0000
	BYTE		Name2[4];     ///< UTF-16 characters 12~13 of name entry
} LONG_NAME;


///
///@ingroup     DATA_STRUCT
///@brief       Chain structure definition.
typedef struct
{
    volatile DWORD    Start;      ///< the start cluster of this chain
    volatile DWORD    Current;    ///< the current cluster the chain point located
    volatile DWORD    Point;      ///< the logical byte location counter of chain point (0, 32, 64, ....)
    volatile DWORD    Size;       ///< the chain actual size in byte

#if 1 //flags for exFAT only [begin]
    struct {
        BYTE f_NoFatChain: 1;      ///< NoFatChain flag
        BYTE f_reserved: 7;        ///< reserved bits
    } exfat_DirEntryFlags;     ///< exFAT DirEntry flags
    BYTE reservedBytes[3];     ///< reserved bytes for alignment
#endif //flags for exFAT only [end]

} CHAIN;


/* for protection/sync mechanism for multi-task concurrent access [begin] */
#define FUNC_SCOPE_MAX_NAME_LEN   128

///
///@ingroup     DATA_STRUCT
///@brief       CACHE structure definition, for directory operations.
typedef struct
{
    volatile BYTE   FatCache[FAT_CACHE_BYTE_SIZE];      ///< buffer for FAT entries content buffer for a drive
    volatile FDB    DirCache[DIR_CACHE_IN_FDB_COUNT];   ///< buffer for directory content cache buffer for a drive
    volatile CHAIN  Stack[DIR_STACK_DEEP];              ///< buffer for directories cache stack for a drive
    volatile DWORD  LongNameFdbCount;                   ///< count of long name FDB nodes of current file or directory pointed by drv->Node

#if 1 //record of exFAT DirectoryEntry info for current file/directory entry [begin]
    volatile DWORD  createTimestamp;           ///< timestamp of file creation
    volatile DWORD  lastModifyTimestamp;       ///< timestamp of file last modification
    volatile BYTE   createUtcOffset;           ///< UTC time offset of file creation
    volatile BYTE   lastModifyUtcOffset;       ///< UTC time offset of file last modification
    volatile WORD   fileAttributes;            ///< file attributes
    volatile WORD   nameHash;                  ///< hash value of filename
    volatile BYTE   nameLength;                ///< character count of filename
    volatile struct {
        BYTE f_AllocPossible: 1;               ///< AllocationPossible flag
        BYTE f_NoFatChain: 1;                  ///< NoFatChain flag
        BYTE f_reserved: 6;                    ///< reserved bits
    } exfat_DirEntryFlags;                     ///< exFAT DirEntry flags
    volatile DWORD  firstCluster;              ///< cluster number of the first cluster of the file content
    volatile U64    dataLength;                ///< 64-bit file data size
#endif //record of exFAT DirectoryEntry info for current file/directory entry [end]

    /* fields for protection/sync mechanism for multi-task concurrent access [begin] */
    volatile BYTE   f_IsUpdatingFdbNodes;               ///< flag to indicate FDBs allocation in a directory is in progress
    volatile BYTE   TaskId_of_UpdatingFdbNodes;         ///< task id of the task which is allocating FDBs in a directory
    volatile BYTE   f_IsUpdatingFatEntries;             ///< flag to indicate FAT entries update is in progress
    volatile BYTE   TaskId_of_UpdatingFatEntries;       ///< task id of the task which is updating FAT entries
    volatile BYTE   FuncScope_of_UpdatingFatEntries[FUNC_SCOPE_MAX_NAME_LEN];      ///< internal record for function scope that FAT entries modifying is performed
    volatile BYTE   FuncScope_of_UpdatingFdbNodes[FUNC_SCOPE_MAX_NAME_LEN];        ///< internal record for function scope that DirCache modifying is performed
    /* fields for protection/sync mechanism for multi-task concurrent access [end] */
} CACHE;


///
///@ingroup     DATA_STRUCT
///@brief       EXFAT_NEW_INFO structure definition, for new info fields introduced by exFAT spec.
typedef struct
{
	U64    VolumeLength;           ///< 64-bit partition offset of the exFAT drive
	DWORD  VolumeSerialNo;         ///< Volume serial number of the drive
	BYTE   FsRevision_Major;       ///< Major version number of exFAT file system revision
	BYTE   FsRevision_Minor;       ///< Minor version number of exFAT file system revision
	BYTE   DriveSelect;            ///< exFAT boot sector DriveSelect byte value
	BYTE   PercentInUse;           ///< Percentage of used clusters of the drive

	/* 16-bit VolumeFlags definition */
	struct
	{
		WORD ActiveFat  :1;        ///< ActiveFat bit in 16-bit VolumeFlags
		WORD VolumeDirty  :1;      ///< VolumeDirty bit in 16-bit VolumeFlags
		WORD MediaFailure  :1;     ///< MediaFailure bit in 16-bit VolumeFlags
		WORD ClearToZero  :1;      ///< ClearToZero bit in 16-bit VolumeFlags
		WORD reserved  :12;        ///< reserved bits in 16-bit VolumeFlags
	} VolumeFlags;                 ///< 16-bit VolumeFlags field of exFAT boot sector
	BYTE   reserved[2];            ///< reserved bytes for alignment

	/* external data buffers */
	BYTE   *AllocBitmapContent[2];   ///< pointer to the data buffer of exFAT Allocation Bitmap content of the drive
	DWORD  AllocBitmapSize[2];       ///< size (in bytes) of the exFAT Allocation Bitmap content of the drive
	BYTE   *UpCaseTableContent;      ///< pointer to the data buffer of exFAT UpCase Table content of the drive
	DWORD  UpCaseTableSize;          ///< size (in bytes) of the exFAT UpCase Table content of the drive
	DWORD  UpCaseTableChkSum;        ///< Checksum of the Up-case Table
} EXFAT_NEW_INFO;


///
///@ingroup     DATA_STRUCT
///@brief       Drive information structure definition.
typedef struct
{
	volatile BYTE DevID;                    ///< device ID of the physical storage device
	volatile BYTE DrvIndex;                 ///< drive index ID, and index of the Drive table entry
	volatile BYTE Partition;                ///< partition ID of the partition of the drive
	volatile BYTE bSectorExp;               ///< power of 2 for the number of bytes in a sector of the drive
	volatile DWORD partitionStartLba;       ///< 32-bit LBA of the first sector in this partition of the storage device

	/* function members definition */
	DWORD  (*FatRead)(void *, DWORD);        ///< FAT entry read function for the drive
	int  (*FatWrite)(void *, DWORD, DWORD);  ///< FAT entry write function for the drive

	/* status or error report */
	volatile int  StatusCode;                ///< status or error report

	/* flags definition */
	struct
	{
		WORD FsType  :3;           ///< bits to indicate the file system type of the drive
		WORD Present  :1;          ///< flag to indicate the drive is present
		WORD ReadOnly  :1;         ///< flag to indicate the drive is read-only
		WORD FatCacheChanged  :1;  ///< flag to indicate content of FAT cache buffer for the drive has been changed
		WORD DirCacheChanged  :1;  ///< flag to indicate content of directory cache buffer for the drive has been changed
		WORD LongNameHit  :1;      ///< flag to indicate the long filename string of a file has been extracted
		WORD AllocBitmapReady :1;  ///< flag to indicate Allocation Bitmap content for non-exFAT drive is ready or not
		WORD reserved_bits :7;     ///< reserved flag bits
	} Flag;   ///< drive flags definition

	volatile WORD   ClusterSize;      ///< number of sectors per allocation unit  (May exceed 255 in exFAT drive, so set to WORD type)
	volatile BYTE   ClusterExp;       ///< power of 2 for the number of sectors in a cluster of the drive
	volatile BYTE   NumberOfFat;      ///< number of FAT copies in the drive
	BYTE reservedBytes[2];            ///< reserved bytes for alignment

	volatile WORD   DirStackPoint;    ///< internal stack point record for the directories stack for the drive
	volatile WORD   DirAuxPoint;      ///< used for parsing the long name
	volatile DWORD  FsInfoAddr;       ///< LBA of the sector that FSINFO structure locates in the drive
	volatile DWORD  FatStart;         ///< LBA of the first sector of FAT table copies of the drive
	volatile DWORD  FatSize;          ///< size (number of sectors) of a FAT table copy in the drive
	volatile DWORD  RootStart;        ///< cluster number of the first cluster of root directory for FAT32 and exFAT; but for FAT12/16, RootStart is lba of first sector of root directory
	volatile DWORD  DataStart;        ///< LBA of the first sector of Data Region of the drive
	volatile CACHE  *CacheBufPoint;   ///< pointer to the internal cache buffers for the drive
	volatile DWORD  FatCachePoint;    ///< internal FAT entries cache point for the beginning sector position of cached FAT table sectors within the whole FAT table
	volatile DWORD  FreeClusters;     ///< total number of free clusters in this drive
	volatile DWORD  LastCluster;      ///< record of cluster number of last free cluster in the drive
	volatile DWORD  TotalClusters;    ///< total number of clusters in this drive
	volatile BYTE   *FatCacheBuffer;  ///< internal buffer for FAT entries content buffer for the drive

	volatile CHAIN  *DirStackBuffer;  ///< internal buffer for directories stack for the drive
	volatile FDB    *DirCacheBuffer;  ///< internal buffer for directory content cache buffer for the drive
	volatile FDB    *Node;            ///< the FDB node of a file or directory currently accessed
	volatile DWORD  DirCachePoint;    ///< internal directory content cache point for the LBA of a directory's current position
	volatile WORD   LongName[MAX_L_NAME_LENG];  ///< UTF-16 string buffer for the long filename of current FDB node
	volatile DWORD  dwAudioTotalFiles;  ///< total number of audio files for GUI file browser
	volatile DWORD  dwImageTotalFiles;  ///< total number of image files for GUI file browser
	volatile DWORD  dwMovieTotalFiles;  ///< total number of video files for GUI file browser
#if EREADER_ENABLE
	volatile DWORD	dwEbookTotalFiles;
#endif

	volatile BYTE   *AllocBitmap_for_NonExFAT;  ///< pointer to the external buffer of Allocation Bitmap content for non-exFAT drive only
	EXFAT_NEW_INFO  *exFAT_InfoFileds;          ///< new info fields introduced by exFAT spec
} DRIVE;


///
///@ingroup     DATA_STRUCT
///@brief       File (STREAM) information structure definition.
typedef struct
{
	volatile DRIVE *Drv;		///< drive pointer, if 0 mean this handle is not used
	volatile DWORD DirSector;	///< the sector lba of directory that contain this file
	volatile DWORD FdbOffsetInDirCache;	///< the offset of this file or directory FDB node from the beginning of the internal Dir cache buffer of the drive. (unit: number of FDB nodes)

	struct
	{
		volatile DWORD SizeChanged :1;       ///< flag to indicate file size changed
		volatile DWORD f_Utf16_LongName :1;  ///< flag to indicate whether if filename is UTF-16 long name or 8.3 ASCII short name
		volatile DWORD f_IsRamFile :1;       ///< flag to indicate whether if this "file" is actually a RAM file (a buffer, not classic file)
		volatile DWORD f_ExtraClustersAllocated :1;  ///< flag to indicate whether if extra FAT clusters are pre-allocated by FileWrite_withWriteBack_FileSize() case
	} Flag;                         ///< some file status flags

	volatile CHAIN Chain;           ///< the chain of data of this file
	volatile BYTE  *ramFile_buf;    ///< pointer to the allocated file content buffer of a "RamFile"
	volatile DWORD ramFile_bufSize; ///< buffer size of allocated memory for the "RamFile"
} STREAM;


typedef struct
{
    BYTE Byte[512];
} SECTOR;

typedef struct
{
    BYTE Byte0;
    BYTE Byte1;
    BYTE Byte2;
    BYTE Byte3;
} BYTE_STRC;



/////////////////////////////////
//         utilities           //
/////////////////////////////////

WORD   LoadUnalign16(void * in);
DWORD  LoadUnalign32(void * in);
WORD   LoadAlien16(void * in);
//DWORD  LoadAlien32(void * in);

#define LoadAlien32(a) ((((BYTE *)a)[3] << 24) | (((BYTE *)a)[2] << 16) | (((BYTE *)a)[1] << 8) | ((BYTE *)a)[0])

void   SaveAlien16(void * addr, WORD data);
void   SaveAlien32(void * addr, DWORD data);
WORD   *LoadAlienArray16(WORD * target, void * source, int length);



/*
*******************************************************************************
*        GLOBAL FUNCTION PROTOTYPES
*******************************************************************************
*/

/* none */


#endif //FAT_H

