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
* Filename      : file.h
* Programmers   :
* Created       :
* Descriptions  :
*******************************************************************************
*/
#ifndef __FILE_H
#define __FILE_H
/*
*******************************************************************************
*        #include
*******************************************************************************
*/

#include "drive.h"
#include "chain.h"


#ifndef EXFAT_ENABLE
  #define EXFAT_ENABLE   1
#endif

#if EXFAT_ENABLE
  /* Note!!! exFAT Write Operations support is still under implementing ... >>
   * Do not enable this !!
   */
  #define EXFAT_WRITE_ENABLE   0
#endif


/* enable this if want to use clusters Allocation Bitmap mechanism for FAT12/16/32 file systems to speed up free clusters searching */
#ifndef ENABLE_ALLOC_BITMAP_FOR_FAT121632
  #define ENABLE_ALLOC_BITMAP_FOR_FAT121632    0
#endif


/* "RamFile" is actually a memory buffer allocated by RamFileAlloc(), for using FileRead/FileWrite/Seek/Fseek/SeekSet/FileClose APIs
 * to operate its file content.
 *
 * The file content of a "RamFile" should be prepared first by using FileWrite() to feed data into the internal buffer of the "RamFile"
 * by the caller task. Remember to rewind/reset the file position to 0 by using SeekSet(handle) or Seek(handle, 0) or Fseek(handle, 0, SEEK_SET)
 * before starting to read data from the "RamFile" by using FileRead().
 *
 * The FileClose() and DeleteFile() APIs are the same operation for a "RamFile", they both release allocated memory buffer within a "RamFile" and
 * release the file handle to system.
 *
 * Note that "RamFile" files have no association to any storage drive, and therefore any file operations with DRIVE and/or CHAIN structures
 * accessing is not allowed to apply to "RamFile" files !
 */
#define RAM_FILE_ENABLE  1  /* if set to 1, "RamFile" is supported */


/* SONY DCF (Digital Camera File System) rules support. When enabled, use customized behaviors. So, disable it by default. And only enabled by FAE in Platform_XXXX.h */
#ifndef SONY_DCF_ENABLE
  #define SONY_DCF_ENABLE   0
#endif



/* Note!!  Current Path-related APIs are still not re-entrant or thread-safe !!
 *   They work well only when single task access the drive for file I/O operation at a time.
 *   Multiple tasks accessing same drive concurrently may cause some kind of problems !! ==> wait for FS_REENTRANT_API solution !! >>
 */

/* Note!!!
 * << Under debugging...  Very dangerous to enable this !! >>
 *
 * Do not enable this FS_REENTRANT_API to use re-entrant path-related APIs because they have some constraints need to be specially cared !!
 * If not obeying their working rule, it will still cause problems if called by multiple tasks concurrently.
 *
 * These API code is temporarily added into SVN for backup. We are developping thread-safe APIs to make it more transparent to App...
 */
#define  FS_REENTRANT_API  0


/*
*******************************************************************************
*        #define CONSTANTS
*******************************************************************************
*/

///
///@ingroup     FS_CONST
///@{
///@brief       Bit mask for FDB attributes: Read-only attribute
#define FDB_READ_ONLY     0x01

///@brief       Bit mask for FDB attributes: Hidden attribute
#define FDB_HIDDEN        0x02

///@brief       Bit mask for FDB attributes: System attribute
#define FDB_SYSTEM        0x04

///@brief       Bit mask for FDB attributes: Volume label attribute
#define FDB_LABEL         0x08

///@brief       Bit mask for FDB attributes: Directory attribute
#define FDB_SUB_DIR       0x10

///@brief       Bit mask for FDB attributes: Archive attribute
#define FDB_ARCHIVE       0x20

///@brief       Bit mask for FDB attributes: Long name attribute == (FDB_READ_ONLY | FDB_HIDDEN | FDB_SYSTEM | FDB_LABEL)
#define FDB_LONG_NAME     0x0f
///@}

/*
 define the FDB attribute2 mask
*/
#define NAME_LOWER_CASE	      0x08
#define EXTNAME_LOWER_CASE    0x10

/*
 define the FAT entry constant
*/
#define FAT32_EOC        0x0fffffff
#define FAT16_EOC        0xffff
#define FAT12_EOC        0xfff

#define GLOBAL_SEARCH                   0x1
#define LOCAL_SEARCH                    0x2
#define LOCAL_RECURSIVE_SEARCH          0x4
#define LOCAL_SEARCH_INCLUDE_FOLDER     0x8

#define SEARCH_INFO_FOLDER              0x1
#define SEARCH_INFO_FILE                0x2
#define SEARCH_INFO_CHANGE_PATH         0x4
#define SEARCH_INFO_TYPE_MASK           0x7  	// 06.26.2006 for use bParameter record invalid file
#define SEARCH_INFO_INVALID_MEDIA       0x80  	// 06.26.2006 for use bParameter record invalid file

///
///@ingroup     FS_CONST
///@brief       Enumeration constants of directory entry types used in directory entry searching.
typedef enum {
    E_FILE_TYPE,   ///< The directory entry to search is a file
    E_DIR_TYPE,    ///< The directory entry to search is a directory
    E_BOTH_FILE_AND_DIR_TYPE  ///< Used for file and directory creation, which not allow same name file and directory coexist in a same folder
} ENTRY_TYPE;

///
///@ingroup     FS_CONST
///@brief       Enumeration constants of filename types of a directory entry.
///@{
enum {
    E_FILENAME_UNKNOWN = 0x00,   ///< The filename type of the directory entry is still unknown
    E_FILENAME_8_3_SHORT_NAME = 0x01,   ///< The filename of the directory entry is a 8.3 short filename
    E_FILENAME_UTF16_LONG_NAME = 0x02   ///< The filename of the directory entry is a UTF-16 long filename
}; //values for ENTRY_FILENAME_TYPE type below

///
///@ingroup     FS_CONST
///@brief       Type definition for the enumeration constant values of filename types.
typedef BYTE  ENTRY_FILENAME_TYPE; /* note: single byte, avoid using integer implicitly by above enum */
///@}



///
///@ingroup     FS_CONST
///@brief       Enumeration constants of directory entry types of the final target entry in the path string.
typedef enum {
    E_PATH_TARGET_IS_FILE,   ///< The target directory entry is a file
    E_PATH_TARGET_IS_DIR,    ///< The target directory entry is a directory
    E_PATH_TARGET_DONT_CARE_FILE_OR_DIR  ///< used in PathAPI__Locate_Path_XXX APIs, don't care the target is a file or dir
} PATH_TARGET_TYPE;

///
///@ingroup     FS_CONST
///@brief       Enumeration constants of operation types for creating midway directories of the specified path string or not.
typedef enum {
    E_NOT_CREATE_MIDWAY_DIR,   ///< Do not create midway directories when they do not exist in the path
    E_AUTO_CREATE_MIDWAY_DIR   ///< Automatically create midway directories if they do not exist in the path
} MIDWAY_DIR_OP_TYPE;


#if FS_REENTRANT_API
typedef struct _Task_Working_Drive_s
{
    DRIVE                         *Work_Drv;    ///< pointer to the working drive instance (DRIVE structure with caches)
    struct _Task_Working_Drive_s  *next;        ///< pointer to next entry in working drive instance list
    BYTE                          TaskId;       ///< task id of the task which uses this working drive instance
    BYTE                          reserved[3];  ///< reserved bytes for alignment
} TASK_WORKING_DRIVE_TYPE;

extern TASK_WORKING_DRIVE_TYPE *Tasks_WorkingDrv_List, *Tasks_WorkingDrv_List_Tail;
#endif //FS_REENTRANT_API


#ifndef SEEK_SET
//SeekFile
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2
#endif


/*
*******************************************************************************
*        GLOBAL MACROS
*******************************************************************************
*/

/* useful macros copied from libIPLAY\libSrc\Usbh\include\UsbHost.h */
#ifndef __USBHOST_H__
  #define hi_byte_of_word(x)      (unsigned char) (((unsigned short)(x)) >> 8)
  #define lo_byte_of_word(x)      (unsigned char) ((unsigned short)(x))
  #define byte_swap_of_word(x)    (unsigned short)(((unsigned short)(x) >> 8) | ((unsigned short)(x) << 8))
  #define hi_byte_of_dword(x)     (unsigned char) ((unsigned long)(x) >> 24)
  #define midhi_byte_of_dword(x)  (unsigned char) ((unsigned long)(x) >> 16)
  #define midlo_byte_of_dword(x)  (unsigned char) ((unsigned long)(x) >> 8)
  #define lo_byte_of_dword(x)     (unsigned char) ((unsigned long)(x))
  #define byte_swap_of_dword(x)   (((unsigned long)(x) << 24) | (((unsigned long)(x) & 0x0000ff00) << 8) | \
                                  (((unsigned long)(x) & 0x00ff0000) >> 8) | ((unsigned long)(x) >> 24))
#endif //__USBHOST_H__

#ifndef MIN
  #define MIN(a,b)              ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
  #define MAX(a,b)              ((a) > (b) ? (a) : (b))
#endif

/*
*******************************************************************************
*        GLOBAL DATA TYPES
*******************************************************************************
*/

///
///@ingroup     DATA_STRUCT
///@brief       Data Structure for date (year/month/day) and time (hour/minute/second) info.
typedef struct
{
	WORD  year;       ///< year
	BYTE  month;      ///< month
	BYTE  day;        ///< day
	BYTE  hour;       ///< hour
	BYTE  minute;     ///< minute
	BYTE  second;     ///< second
	BYTE  UtcOffset;  ///< UTC time offset
} DATE_TIME_INFO_TYPE;

///
///@ingroup     DATA_STRUCT
///@brief       Data Structure containing file attributes, file date/time, file size and start cluster info of a "directory entry".
typedef struct
{
	struct {
		BYTE f_ReadOnly: 1;        ///< Read-only attribute
		BYTE f_Hidden: 1;          ///< Hidden attribute
		BYTE f_System: 1;          ///< System attribute
		BYTE f_SubDir: 1;          ///< Directory attribute
		BYTE f_Archive: 1;         ///< Archive attribute
		BYTE reserved  :3;         ///< reserved bits
	} fileAttribute;           ///< file attribute

#if 1 //flags for exFAT only [begin]
	struct {
		BYTE f_AllocPossible: 1;   ///< AllocationPossible flag
		BYTE f_NoFatChain: 1;      ///< NoFatChain flag
		BYTE f_reserved: 6;        ///< reserved bits
	} exfat_DirEntryFlags;     ///< exFAT DirEntry flags
#endif //flags for exFAT only [end]

	BYTE reservedBytes[2];     ///< reserved bytes for alignment

	DATE_TIME_INFO_TYPE	 create_date_time;  ///< file creation date and time
	DATE_TIME_INFO_TYPE	 modify_date_time;  ///< date and time of last write
	DATE_TIME_INFO_TYPE	 access_date_time;  ///< fast access date and time

	DWORD  fileSize;           ///< file size
	DWORD  startCluster;       ///< start cluster
} DIR_ENTRY_INFO_TYPE;

///
///@ingroup     FILE_BROWSER
///@brief       Data Structure for a file or directory in the file lists of GUI, in little-endian format.
typedef struct
{
	DWORD dwDirStart;              ///< The start cluster of the directory that contains this file or directory FDB node
	DWORD dwDirPoint;              ///< The lba of a sector of the directory that contains this file or directory FDB node
	DWORD dwFdbOffset;             ///< The offset of this FDB node from the beginning of the directory that contains this file or directory FDB node (unit: number of FDB nodes)
	DWORD dwLongFdbCount;          ///< Count of long name FDB nodes of this file or directory
	BYTE  bName[8];                ///< 8-char primary part of 8.3 format short filename, without string null terminator!  Note: exFAT does not support 8.3 short name, so this field is not the filename in exFAT case.
	BYTE  bExt[4];                 ///< 3-char extension part of 8.3 format short filename, declaring [4] here is for alignment.  Note: this field may be used for file type identification, so we also maintain this field in exFAT case.
	DATE_TIME_INFO_TYPE DateTime;  ///< Last modify date and time of the file
	BYTE  bParameter;              ///< Parameter for file type searching for GUI File Browser
	BYTE  bFileType;               ///< File type, 1-.JPG, 2-.MP3, 3-.MOV, 5-FOLDER
	BYTE  FsType;                  ///< File system type
	BYTE  DrvIndex;                ///< Drive index ID
	DWORD dwMediaInfo;             ///< Extra media info: for music is time, for image is w << 16 | h
	DWORD dwPtpObjectHandle;       ///< Object handle for USB HOST PTP
	DWORD dwFileSize;              ///< 32-bit file size

#if EXFAT_ENABLE //extra fields for exFAT [begin]
	DWORD dwParentDirSize;         ///< [exFAT only] The chain size (in bytes) of the directory that contains this file/directory
	U64   u64FileSize;             ///< [exFAT only] 64-bit file size for exFAT
	WORD *utf16_name;              ///< [exFAT only] pointer to external temporary UTF-16 filename buffer, only used for file sorting by filename
	WORD  exfat_NameHash;          ///< [exFAT only] 16-bit filename Hash value for exFAT
	struct {
		BYTE f_AllocPossible: 1;   ///< AllocationPossible flag
		BYTE f_NoFatChain: 1;      ///< NoFatChain flag
		BYTE f_reserved: 6;        ///< reserved bits
	} exfat_DirEntryFlags;         ///< exFAT DirEntry flags
	BYTE  reserved;                ///< reserved bytes for alignment
#endif //extra fields for exFAT [end]
} ST_SEARCH_INFO;

///
///@ingroup     FILE_BROWSER
///@brief       Data Structure for keeping some cared info in ST_SEARCH_INFO used for GUI Slide Show.
typedef struct
{
	DWORD dwDirStart;         ///< The start cluster of the directory that contains this file or directory FDB node
	DWORD dwDirPoint;         ///< The lba of a sector of the directory that contains this file or directory FDB node
	DWORD dwFdbOffset;        ///< The offset of this FDB node from the beginning of the directory that contains this file or directory FDB node (unit: number of FDB nodes)
	BYTE bExt[4];             ///< 3-char extension part of 8.3 format short filename, declaring [4] here is for alignment.
} ST_SEARCH_INFO_Slideshow;

///
///@ingroup     DIRECTORY
///@brief       Data Structure for keeping the FDB node position info of a file or directory.
typedef struct
{
	DWORD dwDirStart;         ///< The start cluster of the directory that contains this file or directory FDB node
	DWORD dwDirPoint;         ///< The lba of a sector of the directory that contains this file or directory FDB node
	DWORD dwFdbOffset;        ///< The offset of this FDB node from the beginning of the directory that contains this file or directory FDB node (unit: number of FDB nodes)

#if 1 //flags for exFAT only [begin]
	struct {
		BYTE f_NoFatChain: 1;     ///< NoFatChain flag
		BYTE f_reserved: 7;       ///< reserved bits
	} exfat_DirEntryFlags;     ///< exFAT DirEntry flags
	BYTE reservedBytes[3];     ///< reserved bytes for alignment
#endif //flags for exFAT only [end]

} ST_SAVE_FDB_POS;


#if (SONY_DCF_ENABLE)
#define TEMP_FILE_LIST_MAX_ENTRIES_COUNT      (FILE_LIST_SIZE * 2)  /* count of temp file list entries for DCF objects processing */
#define FILE_LIST_SONY_DCF_ENTRIES_THRESHOLD  9999  /* SONY spec ask at least 9,999 DCF objects (image files) */
#if (FILE_LIST_SIZE < FILE_LIST_SONY_DCF_ENTRIES_THRESHOLD)
  #error "FILE_LIST_SIZE constant too small to hold 9999 DCF objects !!  Pls enlarge FILE_LIST_SIZE value !!" 
#endif

typedef enum
{
    E_DCF_ROOT_DIR_NAMING = 1,
    E_DCF_OBJ_NAMING = 2
} DCF_ENTRY_FILENAMING_TYPE;

/* struct for SONY DCF Obj processing */
typedef struct
{
	struct {
		BYTE f_maybe_DCF_Obj: 1;                       //flag for whether if it may be a SONY DCF essential object
		BYTE f_final_confirmed: 1;                     //flag for finally confirmed whether if it is a SONY DCF essential object
		BYTE f_removed_lower_prior_DCF_obj: 1;         //flag to indicate this is a lower-priority DCF essential object to remove from the file list
		BYTE f_have_EXIF_time_info: 1;                 //flag to indicate this image file has EXIF date time info
		BYTE f_reserved: 4;                            //reserved bits
	} Flags;  //status flags w.r.t DCF object rules
	BYTE  reserved;                        //reserved bytes for alignment
	SWORD RootDCIM_000AAAAA_Dir_DigitNum;  //digit number of the /DCIM/000AAAAA folder
	DWORD merged_display_integer;          //32-bit integer of the 'merged' interger format for the DCF object display name. (ex: 100-0001.JPG => integer 1000001)
	DATE_TIME_INFO_TYPE exif_date_time;    //EXIF date time info contained in the image file
} DCF_INFO_TYPE;

#else  /* struct for SONY_DCF_ENABLE disabled cases */
#define TEMP_FILE_LIST_MAX_ENTRIES_COUNT    (FILE_LIST_SIZE)  /* count of temp companion file list entries for processing extra info not included in ST_SEARCH_INFO structure (ex: EXIF data time, ... etc). Usually one-to-one mapping */

/* note: this structure is for extra interested info for the file list entries. Its data fields can be expanded */
typedef struct
{
	struct {
		BYTE f_final_confirmed: 1;                     //flag for finally confirmed the extra info for the specified/corresponding file list entry
		BYTE f_have_EXIF_time_info: 1;                 //flag to indicate this image file has EXIF date time info
		BYTE f_reserved: 6;                            //reserved bits
	} Flags;  //status flags w.r.t extra info fields
	BYTE  reserved[3];                     //reserved bytes for alignment
	DATE_TIME_INFO_TYPE exif_date_time;    //EXIF date time info contained in the image file
} EXTRA_FILE_LIST_INFO_TYPE;
#endif


///
///@ingroup     FILE_BROWSER
///@brief       Enumeration constants of file sorting basis type for GUI File Browser.
typedef enum
{
	FILE_SORTING_BY_83SHORT_NAME = 0,                ///< For file sorting by 8.3 short name in ascending order
	FILE_SORTING_BY_83SHORT_NAME_REVERSE,            ///< For file sorting by 8.3 short name in descending order
	FILE_SORTING_BY_DATE_TIME,                       ///< For file sorting by date/time in ascending order
	FILE_SORTING_BY_DATE_TIME_REVERSE,               ///< For file sorting by date/time in descending order
	FILE_SORTING_BY_SIZE,                            ///< For file sorting by file size in ascending order
	FILE_SORTING_BY_SIZE_REVERSE,                    ///< For file sorting by file size in descending order
	FILE_SORTING_BY_EXIF_DATE_TIME,                  ///< For file sorting by photo file EXIF date/time in ascending order
	FILE_SORTING_BY_EXIF_DATE_TIME_REVERSE,          ///< For file sorting by photo file EXIF date/time in descending order
#if (SONY_DCF_ENABLE)
	FILE_SORTING_BY_SONY_DCF_NAMING,                 ///< For file sorting by SONY DCF file naming in ascending order
	FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE,         ///< For file sorting by SONY DCF file naming in descending order
#endif
	FILE_SORTING_MAX,                                ///< Max value of the file sorting basis, this one is excluded (invalid) sorting method
} FILE_SORTING_BASIS_TYPE;


/*
*******************************************************************************
*        GLOBAL VARIABLES
*******************************************************************************
*/

/* none */

/*
*******************************************************************************
*        GLOBAL FUNCTION PROTOTYPES
*******************************************************************************
*/

DWORD   FreeFileHandleCountGet(void);
int     PathAPI__Locate_Path(const BYTE * utf8_path_str, E_DRIVE_INDEX_ID * drv_ID, PATH_TARGET_TYPE * found_entry_type, DWORD * final_token_offset, DWORD * final_token_strlen,
                             PATH_TARGET_TYPE path_target_type, MIDWAY_DIR_OP_TYPE midway_dir_op_type);
int     PathAPI__Locate_Path_UTF16(const WORD * utf16_path_str, E_DRIVE_INDEX_ID * drv_ID, PATH_TARGET_TYPE * found_entry_type, DWORD * final_token_offset, DWORD * final_token_strlen,
                                   PATH_TARGET_TYPE path_target_type, MIDWAY_DIR_OP_TYPE midway_dir_op_type);
STREAM  *PathAPI__Fopen(const BYTE * utf8_path_str, const char * mode);
STREAM  *PathAPI__Fopen_UTF16(const WORD * utf16_path_str, const char * mode);
STREAM  *PathAPI__CreateFile(const BYTE * utf8_path_str);
STREAM  *PathAPI__CreateFile_UTF16(const WORD * utf16_path_str);
int     PathAPI__MakeDir(const BYTE * utf8_path_str);
int     PathAPI__MakeDir_UTF16(const WORD * utf16_path_str);
int     PathAPI__Cd_Path(const BYTE * utf8_path_str);
int     PathAPI__Cd_Path_UTF16(const WORD * utf16_path_str);
int     PathAPI__RemoveFile(const BYTE * utf8_path_str);
int     PathAPI__RemoveFile_UTF16(const WORD * utf16_path_str);
int     PathAPI__RemoveDir(const BYTE * utf8_path_str);
int     PathAPI__RemoveDir_UTF16(const WORD * utf16_path_str);
int     PathAPI__GetEntriesCountInDir(const BYTE * utf8_path_str, DWORD * entries_count);
int     PathAPI__GetEntriesCountInDir_UTF16(const WORD * utf16_path_str, DWORD * entries_count);
int     PathAPI__GetFileAttributeAndTimeInfo(const BYTE * utf8_path_str, DIR_ENTRY_INFO_TYPE * dir_entry_info);
int     PathAPI__GetFileAttributeAndTimeInfo_UTF16(const WORD * utf16_path_str, DIR_ENTRY_INFO_TYPE * dir_entry_info);

#if FS_REENTRANT_API
DRIVE   *AllocateWorkingDrv(void);
void    ReleaseWorkingDrv(DRIVE * drv);
int     PathAPI__Locate_Path_UTF16_r(const WORD * utf16_path_str, E_DRIVE_INDEX_ID * drv_ID, PATH_TARGET_TYPE * found_entry_type, DWORD * final_token_offset, DWORD * final_token_strlen,
                                     DRIVE * ext_working_drv, PATH_TARGET_TYPE path_target_type, MIDWAY_DIR_OP_TYPE midway_dir_op_type);
STREAM  *PathAPI__Fopen_UTF16_r(const WORD * utf16_path_str, const char * mode, DRIVE * ext_working_drv);
STREAM  *PathAPI__CreateFile_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv);
int     PathAPI__MakeDir_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv);
int     PathAPI__Cd_Path_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv);
int     PathAPI__RemoveFile_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv);
int     PathAPI__RemoveDir_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv);
int     PathAPI__GetEntriesCountInDir_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv, DWORD * entries_count);
int     PathAPI__GetFileAttributeAndTimeInfo_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv, DIR_ENTRY_INFO_TYPE * dir_entry_info);

/* internal functions for FS_REENTRANT_API protection / synchronization mechanism [begin] */
void    Set_UpdatingFAT_Status(DRIVE * drv, BYTE * func_scope);
void    Reset_UpdatingFAT_Status(DRIVE * drv, BYTE * func_scope);
void    Set_UpdatingFdbNodes_Status(DRIVE * drv, BYTE * func_scope);
void    Reset_UpdatingFdbNodes_Status(DRIVE * drv, BYTE * func_scope);
/* internal functions for FS_REENTRANT_API protection / synchronization mechanism [end] */
#endif //FS_REENTRANT_API


void    StreamInit(void);
STREAM  *GetFreeFileHandle(DRIVE * drv);
int     Fseek(STREAM * handle, DWORD offset, DWORD whence);
#if (RAM_FILE_ENABLE)
STREAM  *RamFileAlloc(DWORD buf_size);
#endif
STREAM  *FileOpen(DRIVE * drv);
int     FileClose(STREAM * handle);
int     FileClose_NoWriteBack(STREAM * handle);
int     FileForceSync(STREAM * handle);
int     File_ReleaseSpace_For_OverwriteContent(STREAM * handle);
int     CreateFile(DRIVE * drv, BYTE * name, BYTE * extension);
int     CreateFile_UTF16(DRIVE * drv, const WORD * name);
int     DeleteFile(STREAM * handle);
int     Seek(STREAM * handle, DWORD position);
void    SeekSet(STREAM * handle);
void    EndOfFile(STREAM * handle);
DWORD   FilePosGet(STREAM * handle);
DWORD   FileSizeGet(STREAM * handle);
DWORD   FileRead(STREAM * handle, BYTE * buffer, DWORD size);
DWORD   FileWrite(STREAM * handle, BYTE * buffer, DWORD size);
DWORD   FileWrite_withWriteBack_FileSize(STREAM * handle, BYTE * buffer, DWORD size, DWORD num_MB_to_extend_chain);
int     FileChain_AdjustSize_TrimExtraClusters(STREAM * handle);
int     FileSearch(DRIVE * drv, BYTE * name, BYTE * extension, ENTRY_TYPE entry_type);
int     FileSearchLN(DRIVE * drv, WORD * name, DWORD dwNameLength, ENTRY_TYPE entry_type);
SWORD   CheckSpaceIfEnough(DRIVE * stDrv, DWORD dwSize);
SWORD   FileGetLongName(DRIVE * sDrv, ST_SEARCH_INFO * sSearchInfo, BYTE * pbBuf, DWORD * pdwBufferCount, DWORD dwMaxBufferSize);
SWORD   FileExtSearch(DRIVE * sDrv, DWORD * pdwExtArray, ST_SEARCH_INFO * sSearchInfo, DWORD dwMaxElement, DWORD * pdwRecordElement, BYTE bSearchType, BYTE bFileType, BYTE bSortFlag);
STREAM  *FileExtSearchOpen(BYTE bMcardId, BYTE *bExt, DWORD dwSearchCount);
int     FileExtRename(STREAM * handle, BYTE * new_extname);
STREAM  *FileListOpen(DRIVE * sDrv, ST_SEARCH_INFO * pSearchInfo);
SWORD   Locate_DirEntryNode_of_SearchInfo(DRIVE * drv, ST_SEARCH_INFO * pSearchInfo);
SWORD   FileCopy(DRIVE * stTargetDrive, STREAM * stSourceHandle, DWORD dwBufAddress, DWORD dwTempBufSize);
SWORD   QuickSort(void * pBase, DWORD dwElementCount, DWORD dwElementSize,
                  WORD (* Compare)(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order), BOOL reverse_order);
SWORD   FileSort(ST_SEARCH_INFO * sSearchInfo, DWORD dwElementCount, FILE_SORTING_BASIS_TYPE sorting_basis);
FILE_SORTING_BASIS_TYPE GetCurFileSortingBasis(void);
void    SetCurFileSortingBasis(FILE_SORTING_BASIS_TYPE sort_basis);
void    FileGetDate_for_FATxx(WORD wDate, WORD * wYear, BYTE * bMonth, BYTE * bDay);
void    FileGetTime_for_FATxx(WORD wTime, BYTE * bHour, BYTE * bMinute, BYTE * bSecond);
WORD    FileSetDate_for_FATxx(WORD wYear, BYTE bMonth, BYTE bDay);
WORD    FileSetTime_for_FATxx(BYTE bHour, BYTE bMinute, BYTE bSecond);
DWORD   FillUniNumber(WORD * pwBuffer, DWORD dwNumber);
SWORD   DirectoryCopy(DRIVE * stTargetDrive, DRIVE * stSourceDrive, DWORD dwBufAddress, DWORD dwTempBufSize);
SWORD   DriveCopyAllFiles(DRIVE * stTargetDrive, DRIVE * stSourceDrive);
SWORD   GetFilenameOfCurrentNode(DRIVE * drv, ENTRY_FILENAME_TYPE * pbFilenameType, BYTE * pbBuf, DWORD * pdwBufferCount, DWORD dwMaxBufferSize);
SWORD   GetAsciiShortFilenameOfCurrentNode(DRIVE * drv, BYTE * pbBuf, DWORD * pdwBufferCount, DWORD dwMaxBufferSize);
SWORD   GetFileAttributeAndTimeInfoFromFDB(DRIVE * drv, FDB * fdb_ptr, DIR_ENTRY_INFO_TYPE * dir_entry_info);
SWORD   DeleteFileOrFolderOfCurrentFDB(DRIVE * drv);
SWORD   SaveCurrentDirNodePosition(DRIVE * drv, ST_SAVE_FDB_POS * fdb_pos_info);
SWORD   RestoreDirNodePosition(DRIVE * drv, ST_SAVE_FDB_POS * fdb_pos_info);

void    FileUiUpdateIconAniFunPtrRegister(BOOL (*funPtr)(void));

/* functions for SONY DCF processing */
#if (SONY_DCF_ENABLE)
/* note: must re-work these functions in exFAT case, because exFAT not support 8.3 short name !! */
BOOL    Valid_DCF_PrimaryFilename(const BYTE * primary_name, DCF_ENTRY_FILENAMING_TYPE entry_naming_type);
SWORD   Extract_DigitNum_of_RootDCIM_000AAAAA_Dir(const BYTE * primary_name);
int     Extract_DigitNum_of_DCF_Obj(const BYTE * primary_name);
SWORD   Makeup_DCF_Obj_DisplayName(BYTE * DCF_obj_pri_name, DWORD file_list_index, BYTE * output_name_buffer, BYTE output_buf_len, DWORD * output_display_integer);
DWORD   Get_DCF_Obj_TotalCnt(void);
BOOL    Check_If_FileListEntry_In_DCF_Objs_Set(ST_SEARCH_INFO * pSearchInfo, DWORD * return_list_index);
#endif


/*
*******************************************************************************
*        #ERROR SECTION
*******************************************************************************
*/

/* none */

#endif //__FILE_H

