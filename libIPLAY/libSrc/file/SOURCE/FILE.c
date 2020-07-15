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
* Filename      : file.c
* Programmers   :
* Created       :
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

#define EXCLUDE_HIDDEN_FILES_FOLDERS_FOR_FILE_LIST   0  /* if set to 1, skip all hidden files and folders when doing media files scanning */

#define MEASURE_FILE_SCANNING_SORTING_TIME  0   /* for measuring time spent on media files scanning and sorting */

/* this option controls FileClose() whether if release needless extra clusters of a file allocated by FileWrite_withWriteBack_FileSize() */
#define RELEASE_NEEDLESS_EXTRA_CLUSTERS_WHEN_FILE_CLOSE  1 

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "dir.h"
#include "fat.h"
#include "chain.h"
#include "drive.h"
#include "os.h"
#include "file.h"
#include <string.h>
#include "taskid.h"

#if NETWARE_ENABLE
#include "netware.h"
#include "..\..\xml\include\netfs.h"
#if HAVE_LWIP
#include "..\..\lwip\include\net_sys.h"
#endif
#if NET_UPNP
#include "..\..\Xml\INCLUDE\Netfs_pri.h"
#endif
extern Net_App_State App_State;
#endif


/*
*******************************************************************************
*        LOCAL DATA TYPES
*******************************************************************************
*/
typedef struct
{
    SWORD swLow;
    SWORD swHigh;
} ST_STACK_INFO;

//extern BYTE g_bXpgStatus;

/*
*******************************************************************************
*        LOCAL VARIABLE
*******************************************************************************
*/
#define MAX_ELEMENT_SIZE   128  /* length (in bytes) of an element data for sorting or swap */

static STREAM StreamTable[FS_MAX_STREAM];
static DWORD HandleCounter;		// counter of opened files

static const SBYTE badFatPat[] = "/\\<>|\":*?";	/* not allowed in FAT pattern , expect symbol "." */
const SBYTE ReplaceGlyph[] = "+,;=[]."; //add 0x2E ('.') here. Note: WinHex reports as error of directory entry if 0x2E is in 8.3 filename fileds.

static FILE_SORTING_BASIS_TYPE  g_CurFileSortingBasis = FILE_SORTING_BY_83SHORT_NAME_REVERSE;//FILE_SORTING_BY_83SHORT_NAME_REVERSE;//FILE_SORTING_BY_83SHORT_NAME;

#if (SONY_DCF_ENABLE)
/* pre-defined DCF display order for DCF objects:
 * Display order for DCF image:  JPG > TIF > BMP > RAW > (THM) > JPE > JFI  (how about THM, SRF, SR2 ??)
 * Display order for thumbnail:  THM > JPG > TIF > BMP > RAW > JPE > JFI    (when to adopt this order ??)
 */
#define DCF_LATEST_DISPLAY_ORDER   0xFF  /* the latest order for displaying DCF obj */

typedef enum
{
    IMAGE_DISPLAY_ORDER_JPG = 1,
    IMAGE_DISPLAY_ORDER_TIF = 2,
    IMAGE_DISPLAY_ORDER_BMP = 3,
    IMAGE_DISPLAY_ORDER_RAW = 4,
    IMAGE_DISPLAY_ORDER_THM = 5,
    IMAGE_DISPLAY_ORDER_JPE = 6,
    IMAGE_DISPLAY_ORDER_JFI = 7,

    IMAGE_DISPLAY_ORDER_LATEST = DCF_LATEST_DISPLAY_ORDER,  /* the latest order */
} DCF_IMAGE_DISPLAY_ORDER_TYPE;

typedef enum
{
    THUMB_DISPLAY_ORDER_THM = 1,
    THUMB_DISPLAY_ORDER_JPG = 2,
    THUMB_DISPLAY_ORDER_TIF = 3,
    THUMB_DISPLAY_ORDER_BMP = 4,
    THUMB_DISPLAY_ORDER_RAW = 5,
    THUMB_DISPLAY_ORDER_JPE = 6,
    THUMB_DISPLAY_ORDER_JFI = 7,

    THUMB_DISPLAY_ORDER_LATEST = DCF_LATEST_DISPLAY_ORDER,  /* the latest order */
} DCF_THUMB_DISPLAY_ORDER_TYPE;


#define DCIM_000AAAAA_DIR_DIGITS_BITMAP_LEN   126  /* 126 * 8 = 1008 bits, to cover Dir digit number 100 ~ 999 */
static BYTE  g_DCIM_000AAAAA_Dir_Digits_Bitmap[DCIM_000AAAAA_DIR_DIGITS_BITMAP_LEN];

static ST_SEARCH_INFO *g_WorkingFileList_BasePtr = NULL;
static DWORD g_WorkingFileList_TotalCount = 0;

/* dynamically allocated list of SONY DCF info corresponding to each entry of file list */
static DCF_INFO_TYPE *g_FileList_DCF_Info_List = NULL;
static DWORD g_DCF_Obj_TotalCnt = 0;

static BYTE Read_DCIM_000AAAAA_Dir_BitmapEntry(BYTE * bitmap_table, WORD dir_digits_num);
static int Set_DCIM_000AAAAA_Dir_BitmapEntry(BYTE * bitmap_table, WORD dir_digits_num, BYTE bit_value);
static SWORD FileSort_for_Sony_DCF_Name_Order(ST_SEARCH_INFO * sSearchInfo, DWORD dwElementCount, FILE_SORTING_BASIS_TYPE sorting_basis);
static BYTE DCF_Img_DisplayOrderValue(BYTE * ext_name);
static BYTE DCF_Thum_DisplayOrderValue(BYTE * ext_name);

#else  /* struct for SONY_DCF_ENABLE disabled cases */
/* dynamically allocated list of extra info corresponding to each entry of file list */
//static EXTRA_FILE_LIST_INFO_TYPE *g_FileList_Extra_Info_List = NULL;
#endif //#if (SONY_DCF_ENABLE)

static DWORD g_Sorting_Start_Index_in_Companion_Info_List = 0; /* base index of the starting entry of the file list being sorted */


/*
*******************************************************************************
*        LOCAL FUNCTION PROTOTYPES
*******************************************************************************
*/

void Segmentize(STREAM * handle, DWORD * head, DWORD * body, DWORD * tail, DWORD size);
static WORD CompareFileName(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order);
static WORD CompareFileSize(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order);
static WORD CompareFileDateTime(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order);
static WORD CompareImgEXIFDateTime(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order);
#if (SONY_DCF_ENABLE)
static WORD CompareDCFFileDisplayName(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order);
#endif
static DWORD FillNumber(BYTE * pwBuffer, DWORD dwNumber);
static SWORD SweepCurrent(SWORD * pswRet, DRIVE * stTargetDrive, DRIVE * stSourceDrive);
static SWORD SweepForward(DRIVE * stTargetDrive, DRIVE * stSourceDrive, WORD wStartStackPoint);
static SWORD SweepFiling(DRIVE * stTargetDrive, DRIVE * stSourceDrive, WORD wStartStackPoint);
static void SwapData(DWORD DestAddr, DWORD SourceAddr, DWORD dwSize);


/*
*******************************************************************************
*        GLOBAL FUNCTIONS
*******************************************************************************
*/
BOOL (*fileUiUpdateIconAniPtr)(void) = 0;

void FileUiUpdateIconAniFunPtrRegister(BOOL (*funPtr)(void))
{
    fileUiUpdateIconAniPtr = funPtr;
}



///
///@ingroup FILE
///@brief   Initialize/reset the STREAM table (i.e. internal File table) content.
///
///@param   None.
///
///@return  None.
///
///@remark  This function is called only when file system initialization.
///
void StreamInit(void)
{
    DWORD size, i;

    //initialize the STREAM table
    size = sizeof(STREAM) * FS_MAX_STREAM;

    /* for safety, check whether if any allocated memory buffer */
    for (i = 0; i < FS_MAX_STREAM; i++)
    {
        if (StreamTable[i].ramFile_buf != NULL)
            ext_mem_free(StreamTable[i].ramFile_buf); /* "RamFile" buffer is allocated by ext_mem_malloc() */
    }

    MpMemSet(StreamTable, 0, size);
}



///
///@ingroup FILE
///@brief   Get the number of free file handlers in internal STREAM table.
///
///@param   None.
///
///@return  The number of free file handlers in internal STREAM table.
///
DWORD FreeFileHandleCountGet(void)
{
    return (FS_MAX_STREAM - HandleCounter);
}



///
///@ingroup FS_Wrapper
///@brief   Walk through the entire path (UTF-8 string) to locate the file.
///
///@param   utf8_path_str       [IN]  Pointer to the input UTF-8 or ASCII path string. \n\n
///
///@param   drv_ID              [OUT] Returned drive index ID of the drive on which the path/file located, return NULL_DRIVE if any fatal error. \n\n
///
///@param   found_entry_type    [OUT] Returned type of the finally found directory entry specified by the path string. \n
///                                   Valid values of dir_entry_type are: E_PATH_TARGET_IS_FILE, E_PATH_TARGET_IS_DIR, E_PATH_TARGET_DONT_CARE_FILE_OR_DIR. \n
///                                   When error encountered or target entry not found, return this value as E_PATH_TARGET_DONT_CARE_FILE_OR_DIR. \n\n
///
///@param   final_token_offset  [OUT] Returned 'characters count' offset value to the final filename token string from the beginning of input path string. \n
///                                   This parameter is optional for getting final token string purpose if desired; \n
///                                   If you don't care the final token string, just set 'NULL' for this parameter. \n\n
///
///@param   final_token_strlen  [OUT] Returned 'characters count' string length of the final filename token string. \n
///                                   This parameter is optional for getting final token string purpose if desired; \n
///                                   If you don't care the final token string, just set 'NULL' for this parameter. \n\n
///
///@param   path_target_type    [IN]  Specify the type of file or directory that the path string target is. \n
///                                   Valid values of path_target_type are: E_PATH_TARGET_IS_FILE, E_PATH_TARGET_IS_DIR, E_PATH_TARGET_DONT_CARE_FILE_OR_DIR. \n\n
///
///@param   midway_dir_op_type  [IN]  Specify the operation type for the midway directories within the path string. \n
///                                   Valid values of midway_dir_op_type are: E_NOT_CREATE_MIDWAY_DIR, E_AUTO_CREATE_MIDWAY_DIR. \n
///                                   E_NOT_CREATE_MIDWAY_DIR means do not create the midway directories if they are not found. \n
///                                   E_AUTO_CREATE_MIDWAY_DIR means create the midway directories automatically if they are not found. \n\n
///
///@retval  FS_SUCCEED           Locate path successfully and file found. \n\n
///@retval  FILE_NOT_FOUND       Locate path successfully but file not found. \n\n
///@retval  INVALID_DRIVE        Locate path unsuccessfully due to invalid drive name specified. \n\n
///@retval  DRIVE_ACCESS_FAIL    Locate path unsuccessfully due to some drive access error. \n\n
///@retval  ABNORMAL_STATUS      Locate path or search file unsuccessfully due to some error. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n\n
///
///@remark   1. This function call will modify the content of the input UTF-8 path string while processing tokens. \n\n
///
///@remark   2. If (midway_dir_op_type == E_AUTO_CREATE_MIDWAY_DIR) and (path_target_type == E_PATH_TARGET_IS_DIR), then it will also create
///             the final target directory in the path if the directory is not found. \n\n
///
///@remark   3. After operation of this function, the working node is within the final directory of the path string. \n\n
///
int PathAPI__Locate_Path(const BYTE * utf8_path_str, E_DRIVE_INDEX_ID * drv_ID, PATH_TARGET_TYPE * found_entry_type, DWORD * final_token_offset, DWORD * final_token_strlen,
                         PATH_TARGET_TYPE path_target_type, MIDWAY_DIR_OP_TYPE midway_dir_op_type)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    BYTE   *bToken, *final_meaningful_bToken;
    BYTE   *u8_path, *ori_u8_path_pointer;
    WORD   *wUtf16_TempBuf;
    BOOL   from_root_dir, final_token_reached = FALSE;
    int    ret;
    ENTRY_TYPE  entry_type;
    PATH_TARGET_TYPE  final_entry_type;
    WORD fileAttribute = 0x0000;


    /* initialize for return values */
    *found_entry_type = final_entry_type = E_PATH_TARGET_DONT_CARE_FILE_OR_DIR;
    *drv_ID = NULL_DRIVE;
    if (final_token_offset)
        *final_token_offset = 0xFFFFFFFF; //initialize to an impossibly big value
    if (final_token_strlen)
        *final_token_strlen = 0;

    if ((utf8_path_str == NULL) || (strcmp(utf8_path_str, "") == 0))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return INVALID_DRIVE;
    }

    if ((path_target_type != E_PATH_TARGET_IS_FILE) && (path_target_type != E_PATH_TARGET_IS_DIR) && (path_target_type != E_PATH_TARGET_DONT_CARE_FILE_OR_DIR))
    {
        MP_ALERT("%s: Invalid path target type !", __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return INVALID_DRIVE;
    }

    u8_path = (BYTE *) ext_mem_malloc(MAX_PATH_LENG);
    if (u8_path == NULL)
    {
        MP_ALERT("-E %s() malloc fail", __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return ABNORMAL_STATUS;
    }
    MpMemSet(u8_path, 0, MAX_PATH_LENG);
    StringCopy08(u8_path, (BYTE *) utf8_path_str); /* avoid to destroy original input string */
    ori_u8_path_pointer = u8_path;

    /* extract the drive name and convert to drive ID */
    bToken = String08_strstr(u8_path, ":");
    if (bToken == NULL) /* no drive name specified in the beginning of path string */
    {
        new_drv_id = DriveCurIdGet();
        drv = DriveGet(new_drv_id);
    }
    else /* the first token is a drive name */
    {
        *bToken = '\0'; /* overwrite found ':' */
        bToken = u8_path; /* first token */
        MP_DEBUG("%s: first token = [%s]", __FUNCTION__, bToken);

        if ((new_drv_id = DriveName2DrvIndexID(bToken)) == NULL_DRIVE)
        {
            MP_ALERT("%s: Error! Invalid drive name [%s] in the path string !", __FUNCTION__, bToken);
            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
            ext_mem_free(ori_u8_path_pointer);
            return INVALID_DRIVE;
        }

        drv = DriveGet(new_drv_id);
        if ((drv->StatusCode != FS_SUCCEED) || !SystemCardPresentCheck(new_drv_id))
        {
            if (DriveAdd(new_drv_id) == 0)
            {
                MP_ALERT("%s: Error! drive (ID = %d) not ready !", __FUNCTION__, new_drv_id);
                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                ext_mem_free(ori_u8_path_pointer);
                return DRIVE_ACCESS_FAIL;
            }
        }

        u8_path = (BYTE *) ((BYTE *) u8_path + StringLength08(bToken) + 1); /* skip the beginning drive name and ':' */
    }

    *drv_ID = new_drv_id; /* for returning drive ID */

    /* check whether if starting from root directory or current directory */
    if (*u8_path == '/')
    {
        MP_DEBUG("%s: beginning '/' found => search beginning from root directory.", __FUNCTION__);
        from_root_dir = TRUE;
        u8_path = (BYTE *) ((BYTE *) u8_path + 1); /* skip first '/' */
    }
    else
    {
        MP_DEBUG("%s: beginning '/' not found => search beginning from current directory.", __FUNCTION__);
        from_root_dir = FALSE;
    }

    if (from_root_dir == TRUE)
    {
        /* back to beginning of root directory */
        if (DirReset(drv) != FS_SUCCEED)
        {
            MP_ALERT("%s: -E- DirReset() back to beginning of root directory failed !", __FUNCTION__);
            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
            ext_mem_free(ori_u8_path_pointer);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: DirReset() back to beginning of root directory OK !", __FUNCTION__);
        }
    }
    else
    {
        /* back to beginning of current directory */
        ret = DirFirst(drv);
        if ((ret != FS_SUCCEED) && (ret != END_OF_DIR))
        {
            MP_ALERT("%s: Error! DirFirst() back to beginning of current directory failed !", __FUNCTION__);
            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
            ext_mem_free(ori_u8_path_pointer);
            return ABNORMAL_STATUS;
        }
        else if (ret == END_OF_DIR)
        {
            MP_ALERT("%s: -I- current directory is an empty directory.", __FUNCTION__);

            /* process the mandatory return values carefully for this case */
            *drv_ID = new_drv_id; /* return drive ID */
            if (final_token_offset)
                *final_token_offset = ((BYTE *) u8_path - (BYTE *) ori_u8_path_pointer); /* offset of the final filename token to return */
            if (final_token_strlen)
                *final_token_strlen = StringLength08(u8_path);

            ext_mem_free(ori_u8_path_pointer);
            return FILE_NOT_FOUND; /* file or directory not found following the path on the drive */
        }
        else
        {
            MP_DEBUG("%s: DirFirst() back to beginning of current directory OK !", __FUNCTION__);
        }
    }

    wUtf16_TempBuf = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
    if (wUtf16_TempBuf == NULL)
    {
        MP_ALERT("-E %s() malloc fail", __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        ext_mem_free(ori_u8_path_pointer);
        return ABNORMAL_STATUS;
    }

    /* walk through the rest of path to locate the file */
    bToken = String08_strstr(u8_path, "/");

    do
    {
        MpMemSet((BYTE *) wUtf16_TempBuf, 0, MAX_L_NAME_LENG * 2);

        if (bToken == NULL) /* '/' not found => this token is the final token */
        {
            MP_DEBUG("%s: '/' not found => this token is the final token.", __FUNCTION__);
            final_token_reached = TRUE;
            bToken = u8_path;
            TrimOffString08TailSpaces(bToken);
        }
        else
        {
            *bToken = '\0'; /* overwrite found '/' */
            final_token_reached = FALSE;
            bToken = u8_path;
        }

        if (strcmp(bToken, ".") == 0) /* "." means same level hierarchy */
        {
            MP_DEBUG("%s: '.' => same directory.", __FUNCTION__);
            if ((final_token_reached == TRUE) && ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)))
            {
                ret = FS_SUCCEED; /* path target found on the drive */
                final_entry_type = E_PATH_TARGET_IS_DIR;
                break; /* end of processing */
            }
            else
                goto L_u08_next_token;
        }
        else if (strcmp(bToken, "..") == 0) /* ".." means parent directory */
        {
            MP_DEBUG("%s: '..' => go to parent directory ...", __FUNCTION__);
            if (CdParent(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! Cannot change to parent directory !", __FUNCTION__);
                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                ext_mem_free(wUtf16_TempBuf);
                ext_mem_free(ori_u8_path_pointer);
                return ABNORMAL_STATUS;
            }
            else
            {
                MP_DEBUG("%s: changed to parent directory OK.", __FUNCTION__);
                if ((final_token_reached == TRUE) && ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)))
                {
                    ret = FS_SUCCEED; /* path target found on the drive */
                    final_entry_type = E_PATH_TARGET_IS_DIR;
                    break; /* end of processing */
                }
                else
                    goto L_u08_next_token;
            }
        }
        else
        {
            MP_DEBUG("%s: token = [%s]", __FUNCTION__, bToken);

            if (strcmp(bToken, "") == 0)
            {
                if (path_target_type == E_PATH_TARGET_IS_FILE)
                {
                    MP_ALERT("%s: Error! token is a NULL string, cannot be a file !", __FUNCTION__);
                    *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    ext_mem_free(wUtf16_TempBuf);
                    ext_mem_free(ori_u8_path_pointer);
                    return ABNORMAL_STATUS;
                }
                else if ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR))
                {
                    MP_DEBUG("%s: token is a NULL string, it's OK for path of directory case.", __FUNCTION__);
                    bToken = final_meaningful_bToken; /* return last meaningful token */
                    ret = FS_SUCCEED; /* directory found on the drive */
                    final_entry_type = E_PATH_TARGET_IS_DIR;
                    break;
                }
            }
            else if ((*bToken == ' ') || (*bToken == '\t')) /* first character of token cannot be a space (ASCII 0x20) or Tab (ASCII 0x09) */
            {
                if (final_token_reached == FALSE)
                {
                    MP_ALERT("%s: Error! token string with invalid leading character code !", __FUNCTION__);
                    *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    ext_mem_free(wUtf16_TempBuf);
                    ext_mem_free(ori_u8_path_pointer);
                    return ABNORMAL_STATUS;
                }
                else
                {
                    if (path_target_type == E_PATH_TARGET_IS_FILE)
                    {
                        MP_ALERT("%s: Error! token string with invalid leading character code !", __FUNCTION__);
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        ext_mem_free(wUtf16_TempBuf);
                        ext_mem_free(ori_u8_path_pointer);
                        return ABNORMAL_STATUS;
                    }
                    else if ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR))
                    {
                        BYTE *tmp_ch = bToken;

                        while ((*tmp_ch == ' ') || (*tmp_ch == '\t'))
                        {
                           ((BYTE *) tmp_ch)++;
                        }

                        if (*tmp_ch != '\0')
                        {
                            MP_ALERT("%s: Error! token string with invalid leading character code !", __FUNCTION__);
                            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                            ext_mem_free(wUtf16_TempBuf);
                            ext_mem_free(ori_u8_path_pointer);
                            return ABNORMAL_STATUS;
                        }
                        else
                        {
                            MP_DEBUG("%s: it's OK for path of directory case to have final token only have space or tab characters.", __FUNCTION__);
                            bToken = final_meaningful_bToken; /* return last meaningful token */
                            ret = FS_SUCCEED; /* directory found on the drive */
                            final_entry_type = E_PATH_TARGET_IS_DIR;
                            break;
                        }
                    }
                }
            }
            else /* normal token cases, record the token */
            {
                final_meaningful_bToken = bToken;
            }

            /* convert to UTF-16 string and then file search */
            mpx_UtilUtf8ToUnicodeU16(wUtf16_TempBuf, bToken);

            /* For file create/open/delete/... cases, final token in the path is "File" type and E_PATH_TARGET_IS_FILE should be specified.
             * For directory create/change/delete/... cases, final token in the path is "Dir" type and E_PATH_TARGET_IS_DIR should be specified.
             * All previous tokens should be "Dir".
             */
            if (final_token_reached == TRUE)
            {
                if (path_target_type == E_PATH_TARGET_IS_FILE)
                    entry_type = E_FILE_TYPE;
                else if (path_target_type == E_PATH_TARGET_IS_DIR)
                    entry_type = E_DIR_TYPE;
                else if (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)
                    entry_type = E_BOTH_FILE_AND_DIR_TYPE;
            }
            else
                entry_type = E_DIR_TYPE;

            if (FileSearchLN(drv, wUtf16_TempBuf, StringLength16(wUtf16_TempBuf), entry_type) == FS_SUCCEED)
            {
                MP_DEBUG("%s: FileSearchLN() token [%s] found !", __FUNCTION__, bToken);

                /* note: the file attributes reside in different DirEntries in FAT12/16/32 and exFAT */
                if (drv->Flag.FsType != FS_TYPE_exFAT)
                    fileAttribute = (WORD) drv->Node->Attribute;
            #if EXFAT_ENABLE
                else
                    fileAttribute = (WORD) drv->CacheBufPoint->fileAttributes;
            #endif

                if (fileAttribute & FDB_SUB_DIR)
                {
                    MP_DEBUG("%s: token [%s] is a folder => CdSub() ...", __FUNCTION__, bToken);
                    if (CdSub(drv) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Error! failed changing to sub-directory [%s] !", __FUNCTION__, bToken);
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        ext_mem_free(wUtf16_TempBuf);
                        ext_mem_free(ori_u8_path_pointer);
                        return ABNORMAL_STATUS;
                    }
                    else
                    {
                        MP_DEBUG("%s: changed to sub-directory [%s] OK.", __FUNCTION__, bToken);
                        if (final_token_reached == TRUE)
                        {
                            ret = FS_SUCCEED; /* path target found on the drive */
                            final_entry_type = E_PATH_TARGET_IS_DIR;
                            break; /* end of processing */
                        }
                        else
                            goto L_u08_next_token;
                    }
                }
                else
                {
                    MP_DEBUG("%s: token [%s] is a file, and file found !", __FUNCTION__, bToken);
                    ret = FS_SUCCEED; /* file found on the drive */
                    final_entry_type = E_PATH_TARGET_IS_FILE;
                    break;
                }
            }
            else /* not found */
            {
                if (final_token_reached == TRUE)
                {
                    if (path_target_type == E_PATH_TARGET_IS_FILE)
                    {
                        MP_ALERT("%s: -I- file [%s] not found in the path.", __FUNCTION__, bToken);
                    }
                    else if (path_target_type == E_PATH_TARGET_IS_DIR)
                    {
                        MP_ALERT("%s: -I- directory [%s] not found in the path.", __FUNCTION__, bToken);
                    }
                    else if (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)
                    {
                        MP_ALERT("%s: -I- entry [%s] not found in the path.", __FUNCTION__, bToken);
                    }

                    if ((midway_dir_op_type == E_AUTO_CREATE_MIDWAY_DIR) && (path_target_type == E_PATH_TARGET_IS_DIR))
                    {
                        MP_ALERT("%s: create final directory [%s] ... ", __FUNCTION__, bToken);
                        if (MakeDir_UTF16(drv, wUtf16_TempBuf) == FS_SUCCEED)
                        {
                            MP_DEBUG("%s: final directory [%s] created => CdSub() ...", __FUNCTION__, bToken);
                            if (CdSub(drv) != FS_SUCCEED)
                            {
                                MP_ALERT("%s: Error! failed changing to sub-directory [%s] !", __FUNCTION__, bToken);
                                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                                ext_mem_free(wUtf16_TempBuf);
                                ext_mem_free(ori_u8_path_pointer);
                                return ABNORMAL_STATUS;
                            }

                            MP_DEBUG("%s: changed to sub-directory [%s] OK.", __FUNCTION__, bToken);
                            ret = FS_SUCCEED; /* path target found on the drive */
                            final_entry_type = E_PATH_TARGET_IS_DIR;
                            break; /* end of processing */
                        }
                        else
                        {
                            MP_ALERT("%s: create final directory [%s] failed !", __FUNCTION__, bToken);
                            ret = ABNORMAL_STATUS;
                            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        }
                    }
                    else
                    {
                        ret = FILE_NOT_FOUND; /* file or directory not found following the path on the drive */
                        *drv_ID = new_drv_id; /* return drive ID */
                        if (final_token_offset)
                            *final_token_offset = ((BYTE *) bToken - (BYTE *) ori_u8_path_pointer); /* offset of the final filename token to return */
                        if (final_token_strlen)
                            *final_token_strlen = StringLength08(bToken);
                    }
                }
                else /* midway directory */
                {
                    if (midway_dir_op_type == E_NOT_CREATE_MIDWAY_DIR)
                    {
                        MP_ALERT("%s: -I- directory [%s] not found in the path.", __FUNCTION__, bToken);
                        ret = ABNORMAL_STATUS;
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    }
                    else if (midway_dir_op_type == E_AUTO_CREATE_MIDWAY_DIR)
                    {
                        MP_ALERT("%s: create midway directory [%s] ... ", __FUNCTION__, bToken);
                        if (MakeDir_UTF16(drv, wUtf16_TempBuf) == FS_SUCCEED)
                        {
                            MP_DEBUG("%s: midway directory [%s] created => CdSub() ...", __FUNCTION__, bToken);
                            if (CdSub(drv) != FS_SUCCEED)
                            {
                                MP_ALERT("%s: Error! failed changing to sub-directory [%s] !", __FUNCTION__, bToken);
                                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                                ext_mem_free(wUtf16_TempBuf);
                                ext_mem_free(ori_u8_path_pointer);
                                return ABNORMAL_STATUS;
                            }

                            MP_DEBUG("%s: changed to sub-directory [%s] OK.", __FUNCTION__, bToken);
                            goto L_u08_next_token;
                        }
                        else
                        {
                            MP_ALERT("%s: create midway directory [%s] failed !", __FUNCTION__, bToken);
                            ret = ABNORMAL_STATUS;
                            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        }
                    }
                    else
                    {
                        MP_ALERT("%s: Error! Invalid midway_dir_op_type parameter (%s) !", __FUNCTION__, midway_dir_op_type);
                        ret = ABNORMAL_STATUS;
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    }
                }

                ext_mem_free(wUtf16_TempBuf);
                ext_mem_free(ori_u8_path_pointer);
                return ret;
            }
        }

L_u08_next_token:
        u8_path = (BYTE *) ((BYTE *) u8_path + StringLength08(bToken) + 1); /* skip the token and '/' */
        MP_DEBUG("The rest of path = [%s] ==> continue for next token ...", u8_path);
        bToken = String08_strstr(u8_path, "/");
    } while (final_token_reached == FALSE);

    /* prepare return and output values */
    *drv_ID = new_drv_id; /* return drive ID */
    *found_entry_type = final_entry_type; /* return found entry type */
    if (final_token_offset)
        *final_token_offset = ((BYTE *) bToken - (BYTE *) ori_u8_path_pointer); /* offset of the final filename token to return */
    if (final_token_strlen)
        *final_token_strlen = StringLength08(bToken);

    ext_mem_free(wUtf16_TempBuf);
    ext_mem_free(ori_u8_path_pointer);
    return ret;
}



///
///@ingroup FS_Wrapper
///@brief   Walk through the entire path (UTF-16 string) to locate the file. \n\n
///         This function is the UTF-16 Unicode version of PathAPI__Locate_Path() function.
///
///@param   utf16_path_str      [IN]  Pointer to the input UTF-16 path string. \n\n
///
///@param   drv_ID              [OUT] Returned drive index ID of the drive on which the path/file located, return NULL_DRIVE if any fatal error. \n\n
///
///@param   found_entry_type    [OUT] Returned type of the finally found directory entry specified by the path string. \n
///                                   Valid values of dir_entry_type are: E_PATH_TARGET_IS_FILE, E_PATH_TARGET_IS_DIR, E_PATH_TARGET_DONT_CARE_FILE_OR_DIR. \n
///                                   When error encountered or target entry not found, return this value as E_PATH_TARGET_DONT_CARE_FILE_OR_DIR. \n\n
///
///@param   final_token_offset  [OUT] Returned 'characters count' offset value to the final filename token string from the beginning of input path string. \n
///                                   This parameter is optional for getting final token string purpose if desired; \n
///                                   If you don't care the final token string, just set 'NULL' for this parameter. \n\n
///
///@param   final_token_strlen  [OUT] Returned 'characters count' string length of the final filename token string. \n
///                                   This parameter is optional for getting final token string purpose if desired; \n
///                                   If you don't care the final token string, just set 'NULL' for this parameter. \n\n
///
///@param   path_target_type    [IN]  Specify the type of file or directory that the path string target is. \n
///                                   Valid values of path_target_type are: E_PATH_TARGET_IS_FILE, E_PATH_TARGET_IS_DIR, E_PATH_TARGET_DONT_CARE_FILE_OR_DIR. \n\n
///
///@param   midway_dir_op_type  [IN]  Specify the operation type for the midway directories within the path string. \n
///                                   Valid values of midway_dir_op_type are: E_NOT_CREATE_MIDWAY_DIR, E_AUTO_CREATE_MIDWAY_DIR. \n
///                                   E_NOT_CREATE_MIDWAY_DIR means do not create the midway directories if they are not found. \n
///                                   E_AUTO_CREATE_MIDWAY_DIR means create the midway directories automatically if they are not found. \n\n
///
///@retval  FS_SUCCEED           Locate path successfully and file found. \n\n
///@retval  FILE_NOT_FOUND       Locate path successfully but file not found. \n\n
///@retval  INVALID_DRIVE        Locate path unsuccessfully due to invalid drive name specified. \n\n
///@retval  DRIVE_ACCESS_FAIL    Locate path unsuccessfully due to some drive access error. \n\n
///@retval  ABNORMAL_STATUS      Locate path or search file unsuccessfully due to some error. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n\n
///
///@remark   1. This function call will modify the content of the input UTF-16 path string while processing tokens. \n\n
///
///@remark   2. If (midway_dir_op_type == E_AUTO_CREATE_MIDWAY_DIR) and (path_target_type == E_PATH_TARGET_IS_DIR), then it will also create
///             the final target directory in the path if the directory is not found. \n\n
///
///@remark   3. After operation of this function, the working node is within the final directory of the path string. \n\n
///
int PathAPI__Locate_Path_UTF16(const WORD * utf16_path_str, E_DRIVE_INDEX_ID * drv_ID, PATH_TARGET_TYPE * found_entry_type, DWORD * final_token_offset, DWORD * final_token_strlen,
                               PATH_TARGET_TYPE path_target_type, MIDWAY_DIR_OP_TYPE midway_dir_op_type)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE *drv;
    WORD  *wToken, tmpU16str[4], *final_meaningful_wToken;
    WORD  *u16_path, *ori_u16_path_pointer;
    BYTE  bToken[MAX_L_NAME_LENG];
    BOOL  from_root_dir, final_token_reached = FALSE;
    int   ret;
    ENTRY_TYPE  entry_type;
    PATH_TARGET_TYPE  final_entry_type;
    WORD fileAttribute = 0x0000;


    /* initialize for return values */
    *found_entry_type = final_entry_type = E_PATH_TARGET_DONT_CARE_FILE_OR_DIR;
    *drv_ID = NULL_DRIVE;
    if (final_token_offset)
        *final_token_offset = 0xFFFFFFFF; //initialize to an impossibly big value
    if (final_token_strlen)
        *final_token_strlen = 0;

    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return INVALID_DRIVE;
    }

    if ((path_target_type != E_PATH_TARGET_IS_FILE) && (path_target_type != E_PATH_TARGET_IS_DIR) && (path_target_type != E_PATH_TARGET_DONT_CARE_FILE_OR_DIR))
    {
        MP_ALERT("%s: Invalid path target type !", __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return INVALID_DRIVE;
    }

    u16_path = (WORD *) ext_mem_malloc(MAX_PATH_LENG * 2);
    if (u16_path == NULL)
    {
        MP_ALERT("-E %s() malloc fail", __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return ABNORMAL_STATUS;
    }
    MpMemSet((BYTE *) u16_path, 0, MAX_PATH_LENG * 2);
    StringCopy16(u16_path, (WORD *) utf16_path_str); /* avoid to destroy original input string */
    ori_u16_path_pointer = u16_path;

    /* extract the drive name and convert to drive ID */
    MpMemSet((WORD *) tmpU16str, 0, sizeof(WORD) * 4);
    tmpU16str[0] = 0x003A; /* 0x3A = ':' => tmpU16str = UTF-16 string of ":" */
    wToken = String16_strstr(u16_path, (WORD *) tmpU16str);
    if (wToken == NULL) /* no drive name specified in the beginning of path string */
    {
        new_drv_id = DriveCurIdGet();
        drv = DriveGet(new_drv_id);
    }
    else /* the first token is a drive name */
    {
        *wToken = 0x0000; /* overwrite found ':' */
        wToken = u16_path; /* first token */
        MpMemSet(bToken, 0, MAX_L_NAME_LENG);
        mpx_UtilU16ToU08(bToken, wToken);
        MP_DEBUG("%s: first token = [%s]", __FUNCTION__, bToken);

        if ((new_drv_id = DriveName2DrvIndexID(bToken)) == NULL_DRIVE)
        {
            MP_ALERT("%s: Error! Invalid drive name [%s] in the path string !", __FUNCTION__, bToken);
            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
            ext_mem_free(ori_u16_path_pointer);
            return INVALID_DRIVE;
        }

        drv = DriveGet(new_drv_id);
        if ((drv->StatusCode != FS_SUCCEED) || !SystemCardPresentCheck(new_drv_id))
        {
            if (DriveAdd(new_drv_id) == 0)
            {
                MP_ALERT("%s: Error! drive (ID = %d) not ready !", __FUNCTION__, new_drv_id);
                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                ext_mem_free(ori_u16_path_pointer);
                return DRIVE_ACCESS_FAIL;
            }
        }

        u16_path = (WORD *) ((WORD *) u16_path + StringLength16(wToken) + 1); /* skip the beginning drive name and ':' */
    }

    *drv_ID = new_drv_id; /* for returning drive ID */

    /* check whether if starting from root directory or current directory */
    if (*u16_path == 0x002F) /* 0x00, '/' (0x2F) */
    {
        MP_DEBUG("%s: beginning '/' found => search beginning from root directory.", __FUNCTION__);
        from_root_dir = TRUE;
        u16_path = (WORD *) ((WORD *) u16_path + 1); /* skip first '/' */
    }
    else
    {
        MP_DEBUG("%s: beginning '/' not found => search beginning from current directory.", __FUNCTION__);
        from_root_dir = FALSE;
    }

    if (from_root_dir == TRUE)
    {
        /* back to beginning of root directory */
        if (DirReset(drv) != FS_SUCCEED)
        {
            MP_ALERT("%s: -E- DirReset() back to beginning of root directory failed !", __FUNCTION__);
            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
            ext_mem_free(ori_u16_path_pointer);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: DirReset() back to beginning of root directory OK !", __FUNCTION__);
        }
    }
    else
    {
        /* back to beginning of current directory */
        ret = DirFirst(drv);
        if ((ret != FS_SUCCEED) && (ret != END_OF_DIR))
        {
            MP_ALERT("%s: Error! DirFirst() back to beginning of current directory failed !", __FUNCTION__);
            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
            ext_mem_free(ori_u16_path_pointer);
            return ABNORMAL_STATUS;
        }
        else if (ret == END_OF_DIR)
        {
            MP_ALERT("%s: -I- current directory is an empty directory.", __FUNCTION__);

            /* process the mandatory return values carefully for this case */
            *drv_ID = new_drv_id; /* return drive ID */
            if (final_token_offset)
                *final_token_offset = ((WORD *) u16_path - (WORD *) ori_u16_path_pointer); /* offset of the final filename token to return */
            if (final_token_strlen)
                *final_token_strlen = StringLength16(u16_path);

            ext_mem_free(ori_u16_path_pointer);
            return FILE_NOT_FOUND; /* file or directory not found following the path on the drive */
        }
        else
        {
            MP_DEBUG("%s: DirFirst() back to beginning of current directory OK !", __FUNCTION__);
        }
    }

    /* walk through the rest of path to locate the file */
    MpMemSet((WORD *) tmpU16str, 0, sizeof(WORD) * 4);
    tmpU16str[0] = 0x002F; /* 0x2F = '/' => tmpU16str = UTF-16 string of "/" */
    wToken = String16_strstr(u16_path, (WORD *) tmpU16str);

    do
    {
        if (wToken == NULL) /* '/' not found => this token is the final token */
        {
            MP_DEBUG("%s: '/' not found => this token is the final token.", __FUNCTION__);
            final_token_reached = TRUE;
            wToken = u16_path;
            TrimOffString16TailSpaces(wToken);
        }
        else
        {
            *wToken = 0x0000; /* overwrite found '/' */
            final_token_reached = FALSE;
            wToken = u16_path;
        }

        /* check directory "." for same level hierarchy */
        MpMemSet((WORD *) tmpU16str, 0, sizeof(WORD) * 4);
        tmpU16str[0] = 0x002E; /* 0x2E = '.' => tmpU16str = UTF-16 string of "." */
        if (StringCompare16(wToken, tmpU16str) == E_COMPARE_EQUAL) /* "." means same level hierarchy */
        {
            MP_DEBUG("%s: '.' => same directory.", __FUNCTION__);
            if ((final_token_reached == TRUE) && ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)))
            {
                ret = FS_SUCCEED; /* path target found on the drive */
                final_entry_type = E_PATH_TARGET_IS_DIR;
                break; /* end of processing */
            }
            else
                goto L_u16_next_token;
        }

        /* check directory ".." for parent directory */
        MpMemSet((WORD *) tmpU16str, 0, sizeof(WORD) * 4);
        tmpU16str[0] = 0x002E;
        tmpU16str[1] = 0x002E; /* 0x2E = '.' => tmpU16str = UTF-16 string of ".." */
        if (StringCompare16(wToken, tmpU16str) == E_COMPARE_EQUAL) /* ".." means parent directory */
        {
            MP_DEBUG("%s: '..' => go to parent directory ...", __FUNCTION__);
            if (CdParent(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! Cannot change to parent directory !", __FUNCTION__);
                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                ext_mem_free(ori_u16_path_pointer);
                return ABNORMAL_STATUS;
            }
            else
            {
                MP_DEBUG("%s: changed to parent directory OK.", __FUNCTION__);
                if ((final_token_reached == TRUE) && ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)))
                {
                    ret = FS_SUCCEED; /* path target found on the drive */
                    final_entry_type = E_PATH_TARGET_IS_DIR;
                    break; /* end of processing */
                }
                else
                    goto L_u16_next_token;
            }
        }
        else
        {
            MpMemSet(bToken, 0, MAX_L_NAME_LENG);
            mpx_UtilU16ToU08(bToken, wToken);
            MP_DEBUG("%s: token = [%s]", __FUNCTION__, bToken);

            if (*wToken == 0x0000)
            {
                if (path_target_type == E_PATH_TARGET_IS_FILE)
                {
                    MP_ALERT("%s: Error! token is a NULL string, cannot be a file !", __FUNCTION__);
                    *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    ext_mem_free(ori_u16_path_pointer);
                    return ABNORMAL_STATUS;
                }
                else if ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR))
                {
                    MP_DEBUG("%s: token is a NULL string, it's OK for path of directory case.", __FUNCTION__);
                    wToken = final_meaningful_wToken; /* return last meaningful token */
                    ret = FS_SUCCEED; /* directory found on the drive */
                    final_entry_type = E_PATH_TARGET_IS_DIR;
                    break;
                }
            }
            else if ((*wToken == 0x0020) || (*wToken == 0x0009)) /* first character of token cannot be a space (ASCII 0x20) or Tab (ASCII 0x09) */
            {
                if (final_token_reached == FALSE)
                {
                    MP_ALERT("%s: Error! token string with invalid leading character code !", __FUNCTION__);
                    *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    ext_mem_free(ori_u16_path_pointer);
                    return ABNORMAL_STATUS;
                }
                else
                {
                    if (path_target_type == E_PATH_TARGET_IS_FILE)
                    {
                        MP_ALERT("%s: Error! token string with invalid leading character code !", __FUNCTION__);
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        ext_mem_free(ori_u16_path_pointer);
                        return ABNORMAL_STATUS;
                    }
                    else if ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR))
                    {
                        WORD *tmp_word = wToken;

                        while ((*tmp_word == 0x0020) || (*tmp_word == 0x0009))
                        {
                           ((WORD *) tmp_word)++;
                        }

                        if (*tmp_word != 0x0000)
                        {
                            MP_ALERT("%s: Error! token string with invalid leading character code !", __FUNCTION__);
                            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                            ext_mem_free(ori_u16_path_pointer);
                            return ABNORMAL_STATUS;
                        }
                        else
                        {
                            MP_DEBUG("%s: it's OK for path of directory case to have final token only have space or tab characters.", __FUNCTION__);
                            wToken = final_meaningful_wToken; /* return last meaningful token */
                            ret = FS_SUCCEED; /* directory found on the drive */
                            final_entry_type = E_PATH_TARGET_IS_DIR;
                            break;
                        }
                    }
                }
            }
            else /* normal token cases, record the token */
            {
                final_meaningful_wToken = wToken;
            }

            /* For file create/open/delete/... cases, final token in the path is "File" type and E_PATH_TARGET_IS_FILE should be specified.
             * For directory create/change/delete/... cases, final token in the path is "Dir" type and E_PATH_TARGET_IS_DIR should be specified.
             * All previous tokens should be "Dir".
             */
            if (final_token_reached == TRUE)
            {
                if (path_target_type == E_PATH_TARGET_IS_FILE)
                    entry_type = E_FILE_TYPE;
                else if (path_target_type == E_PATH_TARGET_IS_DIR)
                    entry_type = E_DIR_TYPE;
                else if (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)
                    entry_type = E_BOTH_FILE_AND_DIR_TYPE;
            }
            else
                entry_type = E_DIR_TYPE;

            if (FileSearchLN(drv, wToken, StringLength16(wToken), entry_type) == FS_SUCCEED)
            {
                MP_DEBUG("%s: FileSearchLN() token [%s] found !", __FUNCTION__, bToken);

                /* note: the file attributes reside in different DirEntries in FAT12/16/32 and exFAT */
                if (drv->Flag.FsType != FS_TYPE_exFAT)
                    fileAttribute = (WORD) drv->Node->Attribute;
            #if EXFAT_ENABLE
                else
                    fileAttribute = (WORD) drv->CacheBufPoint->fileAttributes;
            #endif

                if (fileAttribute & FDB_SUB_DIR)
                {
                    MP_DEBUG("%s: token [%s] is a folder => CdSub() ...", __FUNCTION__, bToken);
                    if (CdSub(drv) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Error! failed changing to sub-directory [%s] !", __FUNCTION__, bToken);
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        ext_mem_free(ori_u16_path_pointer);
                        return ABNORMAL_STATUS;
                    }
                    else
                    {
                        MP_DEBUG("%s: changed to sub-directory [%s] OK.", __FUNCTION__, bToken);
                        if (final_token_reached == TRUE)
                        {
                            ret = FS_SUCCEED; /* path target found on the drive */
                            final_entry_type = E_PATH_TARGET_IS_DIR;
                            break; /* end of processing */
                        }
                        else
                            goto L_u16_next_token;
                    }
                }
                else
                {
                    MP_DEBUG("%s: token [%s] is a file, and file found !", __FUNCTION__, bToken);
                    ret = FS_SUCCEED; /* path target found on the drive */
                    final_entry_type = E_PATH_TARGET_IS_FILE;
                    break; /* end of processing */
                }
            }
            else /* not found */
            {
                if (final_token_reached == TRUE)
                {
                    if (path_target_type == E_PATH_TARGET_IS_FILE)
                    {
                        MP_ALERT("%s: -I- file [%s] not found in the path.", __FUNCTION__, bToken);
                    }
                    else if (path_target_type == E_PATH_TARGET_IS_DIR)
                    {
                        MP_ALERT("%s: -I- directory [%s] not found in the path.", __FUNCTION__, bToken);
                    }
                    else if (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)
                    {
                        MP_ALERT("%s: -I- entry [%s] not found in the path.", __FUNCTION__, bToken);
                    }

                    if ((midway_dir_op_type == E_AUTO_CREATE_MIDWAY_DIR) && (path_target_type == E_PATH_TARGET_IS_DIR))
                    {
                        MP_ALERT("%s: create final directory [%s] ... ", __FUNCTION__, bToken);
                        if (MakeDir_UTF16(drv, wToken) == FS_SUCCEED)
                        {
                            MP_DEBUG("%s: final directory [%s] created => CdSub() ...", __FUNCTION__, bToken);
                            if (CdSub(drv) != FS_SUCCEED)
                            {
                                MP_ALERT("%s: Error! failed changing to sub-directory [%s] !", __FUNCTION__, bToken);
                                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                                ext_mem_free(ori_u16_path_pointer);
                                return ABNORMAL_STATUS;
                            }

                            MP_DEBUG("%s: changed to sub-directory [%s] OK.", __FUNCTION__, bToken);
                            ret = FS_SUCCEED; /* path target found on the drive */
                            final_entry_type = E_PATH_TARGET_IS_DIR;
                            break; /* end of processing */
                        }
                        else
                        {
                            MP_ALERT("%s: create final directory [%s] failed !", __FUNCTION__, bToken);
                            ret = ABNORMAL_STATUS;
                            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        }
                    }
                    else
                    {
                        ret = FILE_NOT_FOUND; /* file or directory not found following the path on the drive */
                        *drv_ID = new_drv_id; /* return drive ID */
                        if (final_token_offset)
                            *final_token_offset = ((WORD *) wToken - (WORD *) ori_u16_path_pointer); /* offset of the final filename token to return */
                        if (final_token_strlen)
                            *final_token_strlen = StringLength16(wToken);
                    }
                }
                else /* midway directory */
                {
                    if (midway_dir_op_type == E_NOT_CREATE_MIDWAY_DIR)
                    {
                        MP_ALERT("%s: -I- directory [%s] not found in the path.", __FUNCTION__, bToken);
                        ret = ABNORMAL_STATUS;
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    }
                    else if (midway_dir_op_type == E_AUTO_CREATE_MIDWAY_DIR)
                    {
                        MP_ALERT("%s: create midway directory [%s] ... ", __FUNCTION__, bToken);
                        if (MakeDir_UTF16(drv, wToken) == FS_SUCCEED)
                        {
                            MP_DEBUG("%s: midway directory [%s] created => CdSub() ...", __FUNCTION__, bToken);
                            if (CdSub(drv) != FS_SUCCEED)
                            {
                                MP_ALERT("%s: Error! failed changing to sub-directory [%s] !", __FUNCTION__, bToken);
                                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                                ext_mem_free(ori_u16_path_pointer);
                                return ABNORMAL_STATUS;
                            }

                            MP_DEBUG("%s: changed to sub-directory [%s] OK.", __FUNCTION__, bToken);
                            goto L_u16_next_token;
                        }
                        else
                        {
                            MP_ALERT("%s: create midway directory [%s] failed !", __FUNCTION__, bToken);
                            ret = ABNORMAL_STATUS;
                            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        }
                    }
                    else
                    {
                        MP_ALERT("%s: Error! Invalid midway_dir_op_type parameter (%s) !", __FUNCTION__, midway_dir_op_type);
                        ret = ABNORMAL_STATUS;
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    }
                }

                ext_mem_free(ori_u16_path_pointer);
                return ret;
            }
        }

L_u16_next_token:
        u16_path = (WORD *) ((WORD *) u16_path + StringLength16(wToken) + 1); /* skip the token and '/' */
        MpMemSet((WORD *) tmpU16str, 0, sizeof(WORD) * 4);
        tmpU16str[0] = 0x002F; /* 0x2F = '/' => tmpU16str = UTF-16 string of "/" */
        wToken = String16_strstr(u16_path, (WORD *) tmpU16str);
    } while (final_token_reached == FALSE);

    /* prepare return and output values */
    *drv_ID = new_drv_id; /* return drive ID */
    *found_entry_type = final_entry_type; /* return found entry type */
    if (final_token_offset)
        *final_token_offset = ((WORD *) wToken - (WORD *) ori_u16_path_pointer); /* offset of the final filename token to return */
    if (final_token_strlen)
        *final_token_strlen = StringLength16(wToken);

    ext_mem_free(ori_u16_path_pointer);
    return ret;
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement as same as the C standard library "fopen" function.\n
///         It will return a file handle if opening the specified file is successful.
///
///@param   utf8_path_str    The UTF-8 or ASCII path string of a file to be opened or created. \n\n
///
///@param   mode             File access mode permitted. Currently only two modes are supported: "rb" and "wb".\n
///                          But note file access right and protection mechanism is not supported yet. \n\n
///
///@return   The function call will return the file handle if locate/open file successfully, otherwise return NULL if any error. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
///@remark    1. If any midway directory in the path does not exist, this function treats it as error and return NULL. \n\n
///
///@remark    2. File access right and protection mechanism is not supported yet. \n\n
///
STREAM *PathAPI__Fopen(const BYTE * utf8_path_str, const char * mode)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    STREAM *fHandle = NULL;
    BOOL   file_found = FALSE;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    BYTE   *bUtf8_TempBuf;


    if ((utf8_path_str == NULL) || (strcmp(utf8_path_str, "") == 0))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return NULL;
    }

    if (mode == NULL)
    {
        MP_ALERT("%s: NULL mode pointer !", __FUNCTION__);
        return NULL;
    }
    else if ((strcmp(mode, "rb") != 0) && (strcmp(mode, "wb") != 0))
    {
        MP_ALERT("%s: Error! unsupported file access mode [%s] => return.", __FUNCTION__, mode);
        return NULL;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path(utf8_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_FILE, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/file cannot be located due to some error !", __FUNCTION__);
            return NULL;
        }
        else
        {
            MP_DEBUG("%s: locate path OK, but file not found in the path !", __FUNCTION__);
            file_found = FALSE;
        }
    }
    else
    {
        MP_DEBUG("%s: locate path OK, and file found in the path !", __FUNCTION__);
        file_found = TRUE;
    }

    drv = DriveGet(new_drv_id);

    if (file_found) /* file found => open it */
    {
        MP_DEBUG("%s: file found => open it ...", __FUNCTION__);
        fHandle = FileOpen(drv);
    }
    else /* file not found */
    {
        if (strcmp(mode, "rb") == 0) /* do not force file creation if file not exist */
        {
            MP_ALERT("%s: Error! file not found. file access mode is [%s] => return.", __FUNCTION__, mode);
            return NULL;
        }
        else if (strcmp(mode, "wb") == 0) /* force file creation if file not exist */
        {
            char *name, *ext;
            int  ori_filenameL, i;

            /* prevent input path/filename string from being modified by strtok_r() or explicit '\0' assignment */
            bUtf8_TempBuf = (BYTE *) ext_mem_malloc(MAX_L_NAME_LENG);

            if (bUtf8_TempBuf == NULL)
            {
                MP_ALERT("-E %s() malloc fail", __FUNCTION__);
                return NULL;
            }

            MpMemSet(bUtf8_TempBuf, 0, MAX_L_NAME_LENG);

            /* copy exact length of token to trim off tailing spaces if any */
            MpMemCopy(bUtf8_TempBuf, ((BYTE *) utf8_path_str + final_token_offset), final_token_strlen);

            /* scan the rightest '.' in the filename for extension part */
            name = (BYTE *) bUtf8_TempBuf;
            ext = NULL;

            ori_filenameL = StringLength08(bUtf8_TempBuf);
            for (i = ori_filenameL - 1; i >= 0; i--)
            {
                if (bUtf8_TempBuf[i] == 0x2E)  // '.'
                {
                    bUtf8_TempBuf[i] = 0x00; //set null terminator
                    if (i != (ori_filenameL - 1))
                        ext = (BYTE *) &bUtf8_TempBuf[i+1];
                    else
                        ext = NULL;
                    break;
                }
            }

            MP_ALERT("%s: file not found, file access mode is [%s] => create file ...", __FUNCTION__, mode);
            if (CreateFile(drv, name, ext) != FS_SUCCEED)
            {
                ext_mem_free(bUtf8_TempBuf);
                MP_ALERT("%s: Error! file creation failed !", __FUNCTION__);
                return NULL;
            }
            else
            {
                MP_DEBUG("%s: file creation OK !", __FUNCTION__);
            }

            ext_mem_free(bUtf8_TempBuf);
            fHandle = FileOpen(drv);
        }
    }

    return fHandle; /* return file handle */
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement as same as the C standard library "fopen" function.\n
///         It will return a file handle if opening the specified file is successful. \n\n
///         This function is the UTF-16 Unicode version of PathAPI__Fopen() function.
///
///@param   utf16_path_str   The UTF-16 path string of a file to be opened or created. \n\n
///
///@param   mode             File access mode permitted. Currently only two modes are supported: "rb" and "wb".\n
///                          But note file access right and protection mechanism is not supported yet. \n\n
///
///@return   The function call will return the file handle if locate/open file successfully, otherwise return NULL if any error. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
///@remark   1. The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   2. If any midway directory in the path does not exist, this function treats it as error and return NULL. \n\n
///
///@remark   3. File access right and protection mechanism is not supported yet. \n\n
///
STREAM *PathAPI__Fopen_UTF16(const WORD * utf16_path_str, const char * mode)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    STREAM *fHandle = NULL;
    BOOL   file_found = FALSE;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    WORD   *wUtf16_TempBuf;
    int    ret;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return NULL;
    }

    if (mode == NULL)
    {
        MP_ALERT("%s: NULL mode pointer !", __FUNCTION__);
        return NULL;
    }
    else if ((strcmp(mode, "rb") != 0) && (strcmp(mode, "wb") != 0))
    {
        MP_ALERT("%s: Error! unsupported file access mode [%s] => return.", __FUNCTION__, mode);
        return NULL;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_FILE, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/file cannot be located due to some error !", __FUNCTION__);
            return NULL;
        }
        else
        {
            MP_DEBUG("%s: locate path OK, but file not found in the path !", __FUNCTION__);
            file_found = FALSE;
        }
    }
    else
    {
        MP_DEBUG("%s: locate path OK, and file found in the path !", __FUNCTION__);
        file_found = TRUE;
    }

    drv = DriveGet(new_drv_id);

    if (file_found) /* file found => open it */
    {
        MP_DEBUG("%s: file found => open it ...", __FUNCTION__);
        fHandle = FileOpen(drv);
    }
    else /* file not found */
    {
        if (strcmp(mode, "rb") == 0) /* do not force file creation if file not exist */
        {
            MP_ALERT("%s: Error! file not found. file access mode is [%s] => return.", __FUNCTION__, mode);
            return NULL;
        }
        else if (strcmp(mode, "wb") == 0) /* force file creation if file not exist */
        {
            MP_ALERT("%s: file not found, file access mode is [%s] => create file ...", __FUNCTION__, mode);

            if ( (WORD) *((WORD *) utf16_path_str + (final_token_offset + final_token_strlen)) == 0x0000 )
                ret = CreateFile_UTF16(drv, (WORD *) utf16_path_str + final_token_offset);
            else
            {
                /* trim off tailing spaces */
                wUtf16_TempBuf = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
                if (wUtf16_TempBuf == NULL)
                {
                    MP_ALERT("-E %s() malloc fail", __FUNCTION__);
                    return NULL;
                }
                MpMemSet((BYTE *) wUtf16_TempBuf, 0, MAX_L_NAME_LENG * 2);
                MpMemCopy((BYTE *) wUtf16_TempBuf, (BYTE *) ((WORD *) utf16_path_str + final_token_offset), final_token_strlen * 2);

                ret = CreateFile_UTF16(drv, (WORD *) wUtf16_TempBuf);
                ext_mem_free(wUtf16_TempBuf);
            }

            if (ret != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! file creation failed !", __FUNCTION__);
                return NULL;
            }
            else
            {
                MP_DEBUG("%s: file creation OK !", __FUNCTION__);
            }

            fHandle = FileOpen(drv);
        }
    }

    return fHandle; /* return file handle */
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement file creation function with path of the file supported.\n
///         It will return a file handle for the created file if creation is successful.
///
///@param   utf8_path_str    The UTF-8 or ASCII path string of a file to be created. \n\n
///
///@return   The function call will return the file handle if create file successfully, otherwise return NULL if any error. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
///@remark   1. This function acts like CreateFile_UTF16() when a file with same filename already exists in the path because this function
///             actually invokes CreateFile_UTF16() to do the file creation. \n\n
///
///@remark   2. If midway directories in the path do not exist, this function will create them automatically. \n
///             This is different from the behavior of PathAPI__Fopen_XXXX() functions with "wb" (write) mode. \n\n
///
STREAM *PathAPI__CreateFile(const BYTE * utf8_path_str)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    STREAM *fHandle = NULL;
    BOOL   file_found = FALSE;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    WORD   *utf16_filename;
    BYTE   *bUtf8_TempBuf;
    int    ret;


    if ((utf8_path_str == NULL) || (strcmp(utf8_path_str, "") == 0))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return NULL;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path(utf8_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_FILE, E_AUTO_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/file cannot be located due to some error !", __FUNCTION__);
            return NULL;
        }
        else
        {
            MP_DEBUG("%s: locate path OK, and file [%s] not found in the path.", __FUNCTION__, (BYTE *) utf8_path_str + final_token_offset);
            file_found = FALSE;
        }
    }
    else
    {
        MP_ALERT("%s: locate path OK, and a file with same filename exists in the path.", __FUNCTION__);
        file_found = TRUE;
    }

    drv = DriveGet(new_drv_id);

    utf16_filename = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * sizeof(WORD));
    if (utf16_filename == NULL)
    {
        MP_ALERT("-E %s() malloc fail", __FUNCTION__);
        return NULL;
    }
    MpMemSet((BYTE *) utf16_filename, 0, MAX_L_NAME_LENG * sizeof(WORD));

    /* no matter file found or not in the final directory, let CreateFile_UTF16() to do the job */

    if ( (BYTE) *((BYTE *) utf8_path_str + (final_token_offset + final_token_strlen)) == 0x00 )
        mpx_UtilUtf8ToUnicodeU16(utf16_filename, (BYTE *) utf8_path_str + final_token_offset); /* convert to UTF-16 string */
    else
    {
        /* trim off tailing spaces */
        bUtf8_TempBuf = (BYTE *) ext_mem_malloc(MAX_L_NAME_LENG);
        if (bUtf8_TempBuf == NULL)
        {
            MP_ALERT("-E %s() malloc fail", __FUNCTION__);
            ext_mem_free(utf16_filename);
            return NULL;
        }
        MpMemSet(bUtf8_TempBuf, 0, MAX_L_NAME_LENG);
        MpMemCopy(bUtf8_TempBuf, ((BYTE *) utf8_path_str + final_token_offset), final_token_strlen);

        mpx_UtilUtf8ToUnicodeU16(utf16_filename, bUtf8_TempBuf); /* convert to UTF-16 string */
        ext_mem_free(bUtf8_TempBuf);
    }

    ret = CreateFile_UTF16(drv, utf16_filename);
    ext_mem_free(utf16_filename);

    if (ret != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! file creation failed !", __FUNCTION__);
        return NULL;
    }
    else
    {
        MP_ALERT("%s: file creation OK !", __FUNCTION__);
        fHandle = FileOpen(drv);
        return fHandle; /* return file handle */
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement file creation function with path of the file supported.\n
///         It will return a file handle for the created file if creation is successful. \n\n
///         This function is the UTF-16 Unicode version of PathAPI__CreateFile() function.
///
///@param   utf16_path_str   The UTF-16 path string of a file to be created. \n\n
///
///@return   The function call will return the file handle if create file successfully, otherwise return NULL if any error. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
///@remark   1. The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   2. This function acts like CreateFile_UTF16() when a file with same filename already exists in the path because
///             this function actually invokes CreateFile_UTF16() to do the file creation. \n\n
///
///@remark   3. If midway directories in the path do not exist, this function will create them automatically. \n
///             This is different from the behavior of PathAPI__Fopen_XXXX() functions with "wb" (write) mode. \n\n
///
STREAM *PathAPI__CreateFile_UTF16(const WORD * utf16_path_str)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    STREAM *fHandle = NULL;
    BOOL   file_found = FALSE;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    WORD   *wUtf16_TempBuf;
    int    ret;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return NULL;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_FILE, E_AUTO_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/file cannot be located due to some error !", __FUNCTION__);
            return NULL;
        }
        else
        {
            MP_DEBUG("%s: locate path OK, and file not found in the path.", __FUNCTION__);
            file_found = FALSE;
        }
    }
    else
    {
        MP_ALERT("%s: locate path OK, and a file with same filename exists in the path.", __FUNCTION__);
        file_found = TRUE;
    }

    drv = DriveGet(new_drv_id);

    /* no matter file found or not in the final directory, let CreateFile() to do the job */

    if ( (WORD) *((WORD *) utf16_path_str + (final_token_offset + final_token_strlen)) == 0x0000 )
        ret = CreateFile_UTF16(drv, (WORD *) utf16_path_str + final_token_offset);
    else
    {
        /* trim off tailing spaces */
        wUtf16_TempBuf = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
        if (wUtf16_TempBuf == NULL)
        {
            MP_ALERT("-E %s() malloc fail", __FUNCTION__);
            return NULL;
        }
        MpMemSet((BYTE *) wUtf16_TempBuf, 0, MAX_L_NAME_LENG * 2);
        MpMemCopy((BYTE *) wUtf16_TempBuf, (BYTE *) ((WORD *) utf16_path_str + final_token_offset), final_token_strlen * 2);

        ret = CreateFile_UTF16(drv, (WORD *) wUtf16_TempBuf);
        ext_mem_free(wUtf16_TempBuf);
    }

    if (ret != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! file creation failed !", __FUNCTION__);
        return NULL;
    }
    else
    {
        MP_ALERT("%s: file creation OK !", __FUNCTION__);
        fHandle = FileOpen(drv);
        return fHandle; /* return file handle */
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement as same as the C standard library "remove" function.\n
///         It will delete the file specified by the path/filename string.
///
///@param   utf8_path_str       The UTF-8 or ASCII path string of a file to be deleted. \n\n
///
///@retval  FS_SUCCEED          Delete file successfully. \n\n
///@retval  ABNORMAL_STATUS     Delete file unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no need to delete it. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
int PathAPI__RemoveFile(const BYTE * utf8_path_str)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    STREAM *fHandle = NULL;
    int    ret;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;


    if ((utf8_path_str == NULL) || (strcmp(utf8_path_str, "") == 0))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path(utf8_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_FILE, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/file cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: locate path OK, but file not found in the path !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        if ((found_entry_type == E_PATH_TARGET_IS_FILE) && (final_token_strlen > 0))
        {
            MP_DEBUG("%s: locate path OK, and file found in the path !", __FUNCTION__);
            drv = DriveGet(new_drv_id);
            MP_ALERT("%s: file found => delete it ...", __FUNCTION__);
            fHandle = FileOpen(drv);
            ret = DeleteFile(fHandle);
            return ret;
        }
        else
        {
            MP_DEBUG("%s: -I- locate path OK, but the target is not a file => Check FileSystem source code !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement as same as the C standard library "remove" function.\n
///         It will delete the file specified by the path/filename string. \n\n
///         This function is the UTF-16 Unicode version of PathAPI__RemoveFile() function.
///
///@param   utf16_path_str   The UTF-16 path string of a file to be deleted. \n\n
///
///@retval  FS_SUCCEED          Delete file successfully. \n\n
///@retval  ABNORMAL_STATUS     Delete file unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no need to delete it. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
///@remark   The path string must be converted to UTF-16 before calling this function. \n\n
///
int PathAPI__RemoveFile_UTF16(const WORD * utf16_path_str)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    STREAM *fHandle = NULL;
    int    ret;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_FILE, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/file cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: locate path OK, but file not found in the path !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        if ((found_entry_type == E_PATH_TARGET_IS_FILE) && (final_token_strlen > 0))
        {
            MP_DEBUG("%s: locate path OK, and file found in the path !", __FUNCTION__);
            drv = DriveGet(new_drv_id);
            MP_ALERT("%s: file found => delete it ...", __FUNCTION__);
            fHandle = FileOpen(drv);
            ret = DeleteFile(fHandle);
            return ret;
        }
        else
        {
            MP_DEBUG("%s: -I- locate path OK, but the target is not a file => Check FileSystem source code !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to get file attribute and date/time info of a file/directory. \n\n
///
///@param   utf8_path_str       [IN] The UTF-8 or ASCII path string of a file/directory to get its info. \n\n
///
///@param   dir_entry_info      [OUT] Returned structure containing file attribute and date/time info. \n\n
///
///@retval  FS_SUCCEED          Get file attribute and date/time info successfully. \n\n
///@retval  ABNORMAL_STATUS     Get file attribute and date/time info unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no file attribute and date/time info returned. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
int PathAPI__GetFileAttributeAndTimeInfo(const BYTE * utf8_path_str, DIR_ENTRY_INFO_TYPE * dir_entry_info)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    int    ret;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    WORD   *utf16_filename;
    BYTE   *bUtf8_TempBuf;


    if ((utf8_path_str == NULL) || (strcmp(utf8_path_str, "") == 0))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (dir_entry_info == NULL)
    {
        MP_ALERT("%s: Error ! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path(utf8_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_DONT_CARE_FILE_OR_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/file cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: locate path OK, but file not found in the path !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        MP_DEBUG("%s: locate path OK, and directory entry found in the path !", __FUNCTION__);
        drv = DriveGet(new_drv_id);

        if (found_entry_type == E_PATH_TARGET_IS_DIR)
        {
            CdParent(drv);

            utf16_filename = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * sizeof(WORD));
            if (utf16_filename == NULL)
            {
                MP_ALERT("-E %s() malloc fail", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
            MpMemSet((BYTE *) utf16_filename, 0, MAX_L_NAME_LENG * sizeof(WORD));

            if ( (BYTE) *((BYTE *) utf8_path_str + (final_token_offset + final_token_strlen)) == 0x00 )
                mpx_UtilUtf8ToUnicodeU16(utf16_filename, (BYTE *) utf8_path_str + final_token_offset); /* convert to UTF-16 string */
            else
            {
                /* trim off tailing spaces */
                bUtf8_TempBuf = (BYTE *) ext_mem_malloc(MAX_L_NAME_LENG);
                if (bUtf8_TempBuf == NULL)
                {
                    MP_ALERT("-E %s() malloc fail", __FUNCTION__);
                    ext_mem_free(utf16_filename);
                    return ABNORMAL_STATUS;
                }
                MpMemSet(bUtf8_TempBuf, 0, MAX_L_NAME_LENG);
                MpMemCopy(bUtf8_TempBuf, ((BYTE *) utf8_path_str + final_token_offset), final_token_strlen);

                mpx_UtilUtf8ToUnicodeU16(utf16_filename, bUtf8_TempBuf); /* convert to UTF-16 string */
                ext_mem_free(bUtf8_TempBuf);
            }

            ret = FileSearchLN(drv, utf16_filename, StringLength16(utf16_filename), E_DIR_TYPE);
            ext_mem_free(utf16_filename);

            if (ret != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! FileSearchLN() failed, cannot move to the directory entry position !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
        }

        if (GetFileAttributeAndTimeInfoFromFDB(drv, (FDB *) drv->Node, dir_entry_info) == PASS)
            ret = FS_SUCCEED;
        else
            ret = ABNORMAL_STATUS;

        return ret;
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to get file attribute and date/time info of a file/directory. \n
///         This function is the UTF-16 Unicode version of PathAPI__GetFileAttributeAndTimeInfo() function. \n\n
///
///@param   utf16_path_str      [IN] The UTF-16 path string of a file/directory to get its info. \n\n
///
///@param   dir_entry_info      [OUT] Returned structure containing file attribute and date/time info. \n\n
///
///@retval  FS_SUCCEED          Get file attribute and date/time info successfully. \n\n
///@retval  ABNORMAL_STATUS     Get file attribute and date/time info unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no file attribute and date/time info returned. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
///@remark   The path string must be converted to UTF-16 before calling this function. \n\n
///
int PathAPI__GetFileAttributeAndTimeInfo_UTF16(const WORD * utf16_path_str, DIR_ENTRY_INFO_TYPE * dir_entry_info)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    int    ret;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    WORD   *wUtf16_TempBuf;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (dir_entry_info == NULL)
    {
        MP_ALERT("%s: Error ! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_DONT_CARE_FILE_OR_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/file cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: locate path OK, but file not found in the path !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        MP_DEBUG("%s: locate path OK, and directory entry found in the path !", __FUNCTION__);
        drv = DriveGet(new_drv_id);

        if (found_entry_type == E_PATH_TARGET_IS_DIR)
        {
            CdParent(drv);

            if ( (WORD) *((WORD *) utf16_path_str + (final_token_offset + final_token_strlen)) == 0x0000 )
                ret = FileSearchLN(drv, (WORD *) utf16_path_str + final_token_offset, final_token_strlen, E_DIR_TYPE);
            else
            {
                /* trim off tailing spaces */
                wUtf16_TempBuf = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
                if (wUtf16_TempBuf == NULL)
                {
                    MP_ALERT("-E %s() malloc fail", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }
                MpMemSet((BYTE *) wUtf16_TempBuf, 0, MAX_L_NAME_LENG * 2);
                MpMemCopy((BYTE *) wUtf16_TempBuf, (BYTE *) ((WORD *) utf16_path_str + final_token_offset), final_token_strlen * 2);

                ret = FileSearchLN(drv, (WORD *) wUtf16_TempBuf, final_token_strlen, E_DIR_TYPE);
                ext_mem_free(wUtf16_TempBuf);
            }

            if (ret != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! FileSearchLN() failed, cannot move to the directory entry position !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
        }

        if (GetFileAttributeAndTimeInfoFromFDB(drv, (FDB *) drv->Node, dir_entry_info) == PASS)
            ret = FS_SUCCEED;
        else
            ret = ABNORMAL_STATUS;

        return ret;
    }
}


#if FS_REENTRANT_API

DRIVE *AllocateWorkingDrv(void)
{
    DRIVE *drv;
    CACHE *cache;
    TASK_WORKING_DRIVE_TYPE *list_ptr = Tasks_WorkingDrv_List;
    BYTE curr_task_id = TaskGetId();


    while (list_ptr != NULL)
    {
        if (list_ptr->TaskId == curr_task_id) //entry of this task found
        {
            if (list_ptr->Work_Drv) //this task had already allocated
            {
                return list_ptr->Work_Drv;
            }
            else
            {
                break;
            }
        }

        list_ptr = list_ptr->next;
    }

    drv = (DRIVE *) ext_mem_malloc(sizeof(DRIVE));
    if (drv == NULL)
    {
        MP_ALERT("%s: malloc failed !", __FUNCTION__);
        return NULL;
    }
    drv = (DRIVE *) ((DWORD) drv | BIT29);
    MpMemSet(drv, 0, sizeof(DRIVE));

    cache = (CACHE *) ext_mem_malloc(sizeof(CACHE));
    if (cache == NULL)
    {
        MP_ALERT("%s: malloc failed !", __FUNCTION__);
        ext_mem_free(drv);
        return NULL;
    }
    cache = (CACHE *) ((DWORD) cache | BIT29);
    MpMemSet(cache, 0, sizeof(CACHE));

    drv->CacheBufPoint = cache;
    drv->FatCacheBuffer = cache->FatCache;
    drv->DirStackBuffer = cache->Stack;
    drv->DirCacheBuffer = cache->DirCache;

    /* insert to the working drive list */
    if (list_ptr)
    {
        list_ptr->Work_Drv = drv;
    }
    else
    {
        TASK_WORKING_DRIVE_TYPE *new_entry = (TASK_WORKING_DRIVE_TYPE *) ext_mem_malloc(sizeof(TASK_WORKING_DRIVE_TYPE));
        if (new_entry == NULL)
        {
            MP_ALERT("%s: malloc failed !", __FUNCTION__);
            ext_mem_free(drv);
            ext_mem_free(cache);
            return NULL;
        }

        new_entry->TaskId = curr_task_id;
        new_entry->Work_Drv = drv;
        new_entry->next = NULL;

        if (Tasks_WorkingDrv_List_Tail)
        {
            Tasks_WorkingDrv_List_Tail->next = new_entry;
            Tasks_WorkingDrv_List_Tail = new_entry;
        }
        else //empty list
        {
            Tasks_WorkingDrv_List = Tasks_WorkingDrv_List_Tail = new_entry;
        }
    }

    return drv;
}



void ReleaseWorkingDrv(DRIVE *drv)
{
    TASK_WORKING_DRIVE_TYPE *list_ptr = Tasks_WorkingDrv_List;


    if (drv == NULL)
        return;

    while (list_ptr != NULL)
    {
        if (list_ptr->Work_Drv == drv) //found
        {
            break;
        }
    }

    if (drv->CacheBufPoint)
        ext_mem_free(drv->CacheBufPoint);

    ext_mem_free(drv);
    drv = NULL;

    if (list_ptr)
    {
        list_ptr->Work_Drv = NULL;
    }
}



///
///@ingroup FS_Wrapper
///@brief   Walk through the entire path (UTF-16 string) to locate the file. \n\n
///         This function is the "re-entrant / thread-safe" version of PathAPI__Locate_Path_UTF16() !
///
///@param   utf16_path_str      [IN]  Pointer to the input UTF-16 path string. \n\n
///
///@param   drv_ID              [OUT] Returned drive index ID of the drive on which the path/file located, return NULL_DRIVE if any fatal error. \n\n
///
///@param   found_entry_type    [OUT] Returned type of the finally found directory entry specified by the path string. \n
///                                   Valid values of dir_entry_type are: E_PATH_TARGET_IS_FILE, E_PATH_TARGET_IS_DIR, E_PATH_TARGET_DONT_CARE_FILE_OR_DIR. \n
///                                   When error encountered or target entry not found, return this value as E_PATH_TARGET_DONT_CARE_FILE_OR_DIR. \n\n
///
///@param   final_token_offset  [OUT] Returned 'characters count' offset value to the final filename token string from the beginning of input path string. \n
///                                   This parameter is optional for getting final token string purpose if desired; \n
///                                   If you don't care the final token string, just set 'NULL' for this parameter. \n\n
///
///@param   final_token_strlen  [OUT] Returned 'characters count' string length of the final filename token string. \n
///                                   This parameter is optional for getting final token string purpose if desired; \n
///                                   If you don't care the final token string, just set 'NULL' for this parameter. \n\n
///
///@param   ext_working_drv     [IN]  Drive handle of the external 'working DRIVE' for the calling task to work for "re-entrant / thread-safe". \n\n
///
///@param   path_target_type    [IN]  Specify the type of file or directory that the path string target is. \n
///                                   Valid values of path_target_type are: E_PATH_TARGET_IS_FILE, E_PATH_TARGET_IS_DIR, E_PATH_TARGET_DONT_CARE_FILE_OR_DIR. \n\n
///
///@param   midway_dir_op_type  [IN]  Specify the operation type for the midway directories within the path string. \n
///                                   Valid values of midway_dir_op_type are: E_NOT_CREATE_MIDWAY_DIR, E_AUTO_CREATE_MIDWAY_DIR. \n
///                                   E_NOT_CREATE_MIDWAY_DIR means do not create the midway directories if they are not found. \n
///                                   E_AUTO_CREATE_MIDWAY_DIR means create the midway directories automatically if they are not found. \n\n
///
///@retval  FS_SUCCEED           Locate path successfully and file found. \n\n
///@retval  FILE_NOT_FOUND       Locate path successfully but file not found. \n\n
///@retval  INVALID_DRIVE        Locate path unsuccessfully due to invalid drive name specified. \n\n
///@retval  DRIVE_ACCESS_FAIL    Locate path unsuccessfully due to some drive access error. \n\n
///@retval  ABNORMAL_STATUS      Locate path or search file unsuccessfully due to some error. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n\n
///
///@remark   1. This function call will modify the content of the input UTF-16 path string while processing tokens. \n\n
///
///@remark   2. If (midway_dir_op_type == E_AUTO_CREATE_MIDWAY_DIR) and (path_target_type == E_PATH_TARGET_IS_DIR), then it will also create
///             the final target directory in the path if the directory is not found. \n\n
///
///@remark   3. After operation of this function, the working node is within the final directory of the path string. \n\n
///
///@remark   4. For "re-entrant / thread-safe" to be called by multiple tasks concurrently, a 'working DRIVE' handle parameter must be feed by each task to work ! \n\n
///
int PathAPI__Locate_Path_UTF16_r(const WORD * utf16_path_str, E_DRIVE_INDEX_ID * drv_ID, PATH_TARGET_TYPE * found_entry_type, DWORD * final_token_offset, DWORD * final_token_strlen,
                                 DRIVE * ext_working_drv, PATH_TARGET_TYPE path_target_type, MIDWAY_DIR_OP_TYPE midway_dir_op_type)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE *drv;
    WORD  *wToken, tmpU16str[4], *final_meaningful_wToken;
    WORD  *u16_path, *ori_u16_path_pointer;
    BYTE  bToken[MAX_L_NAME_LENG];
    BOOL  from_root_dir, final_token_reached = FALSE;
    int   ret;
    ENTRY_TYPE  entry_type;
    BYTE  task_id = TaskGetId();
    PATH_TARGET_TYPE  final_entry_type;
    WORD fileAttribute = 0x0000;


    /* initialize for return values */
    *found_entry_type = final_entry_type = E_PATH_TARGET_DONT_CARE_FILE_OR_DIR;
    *drv_ID = NULL_DRIVE;
    if (final_token_offset)
        *final_token_offset = 0xFFFFFFFF; //initialize to an impossibly big value
    if (final_token_strlen)
        *final_token_strlen = 0;

    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("task ID %d: %s: Invalid input path string !", task_id, __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return INVALID_DRIVE;
    }

    if (ext_working_drv == NULL)
    {
        MP_ALERT("task ID %d: %s: Error! NULL drive handle !", task_id, __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return INVALID_DRIVE;
    }
    else if (ext_working_drv->CacheBufPoint == NULL)
    {
        MP_ALERT("task ID %d: %s: Error! NULL CACHE pointer in the working drive handle !", task_id, __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return INVALID_DRIVE;
    }

    if ((path_target_type != E_PATH_TARGET_IS_FILE) && (path_target_type != E_PATH_TARGET_IS_DIR) && (path_target_type != E_PATH_TARGET_DONT_CARE_FILE_OR_DIR))
    {
        MP_ALERT("task ID %d: %s: Invalid path target type !", task_id, __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return INVALID_DRIVE;
    }

    u16_path = (WORD *) ext_mem_malloc(MAX_PATH_LENG * 2);
    if (u16_path == NULL)
    {
        MP_ALERT("-E %s() malloc fail", __FUNCTION__);
        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
        return ABNORMAL_STATUS;
    }
    MpMemSet((BYTE *) u16_path, 0, MAX_PATH_LENG * 2);
    StringCopy16(u16_path, utf16_path_str); /* avoid to destroy original input string */
    ori_u16_path_pointer = u16_path;

    /* extract the drive name and convert to drive ID */
    MpMemSet((WORD *) tmpU16str, 0, sizeof(WORD) * 4);
    tmpU16str[0] = 0x003A; /* 0x3A = ':' => tmpU16str = UTF-16 string of ":" */
    wToken = String16_strstr(u16_path, (WORD *) tmpU16str);
    if (wToken == NULL) /* no drive name specified in the beginning of path string */
    {
        new_drv_id = DriveCurIdGet();
        drv = DriveGet(new_drv_id);
    }
    else /* the first token is a drive name */
    {
        *wToken = 0x0000; /* overwrite found ':' */
        wToken = u16_path; /* first token */
        MpMemSet(bToken, 0, MAX_L_NAME_LENG);
        mpx_UtilU16ToU08(bToken, wToken);
        MP_DEBUG("task ID %d: %s: first token = [%s]", task_id, __FUNCTION__, bToken);

        if ((new_drv_id = DriveName2DrvIndexID(bToken)) == NULL_DRIVE)
        {
            MP_ALERT("task ID %d: %s: Error! Invalid drive name [%s] in the path string !", task_id, __FUNCTION__, bToken);
            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
            ext_mem_free(ori_u16_path_pointer);
            return INVALID_DRIVE;
        }

        drv = DriveGet(new_drv_id); /* not use DriveChange() here, we don't want to touch CurrDrive */

        if ((drv->StatusCode != FS_SUCCEED) || !SystemCardPresentCheck(new_drv_id))
        {
            if (DriveAdd(new_drv_id) == 0)
            {
                MP_ALERT("task ID %d: %s: Error! drive (ID = %d) not ready !", task_id, __FUNCTION__, new_drv_id);
                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                ext_mem_free(ori_u16_path_pointer);
                return DRIVE_ACCESS_FAIL;
            }
        }

        u16_path = (WORD *) ((WORD *) u16_path + StringLength16(wToken) + 1); /* skip the beginning drive name and ':' */
    }

    *drv_ID = new_drv_id; /* for returning drive ID */

    /* check whether if starting from root directory or current directory */
    if (*u16_path == 0x002F) /* 0x00, '/' (0x2F) */
    {
        MP_DEBUG("task ID %d: %s: beginning '/' found => search beginning from root directory.", task_id, __FUNCTION__);
        from_root_dir = TRUE;
        u16_path = (WORD *) ((WORD *) u16_path + 1); /* skip first '/' */
    }
    else
    {
        MP_DEBUG("task ID %d: %s: beginning '/' not found => search beginning from current directory.", task_id, __FUNCTION__);
        from_root_dir = FALSE;
    }

    /* copy whole DRIVE structure including CACHE content to prepare working space for the calling task */
    CACHE *ori_cache_ptr = (CACHE *) ext_working_drv->CacheBufPoint;
    MpMemCopy(ext_working_drv, drv, sizeof(DRIVE));
    ext_working_drv->CacheBufPoint = ori_cache_ptr;
    ext_working_drv->FatCacheBuffer = ori_cache_ptr->FatCache;
    ext_working_drv->DirStackBuffer = ori_cache_ptr->Stack;
    ext_working_drv->DirCacheBuffer = ori_cache_ptr->DirCache;
    if (drv->CacheBufPoint != NULL)
        MpMemCopy((BYTE *) ext_working_drv->CacheBufPoint, (BYTE *) drv->CacheBufPoint, sizeof(CACHE));
    else
    {
        /* note: add pointer check for safety ! Because MpMemSet() may cause "break 100" when memory buffer was already released first by Card Out, for example. */
        if ((BYTE *) ext_working_drv->CacheBufPoint != NULL)
            MpMemSet((BYTE *) ext_working_drv->CacheBufPoint, 0, sizeof(CACHE));

        ext_working_drv->FatCachePoint = 0xffffffff;
        ext_working_drv->DirCachePoint = 0xffffffff;
        ext_working_drv->DirStackPoint = 0;
        ext_working_drv->Node = NULL;
        MpMemSet((BYTE *) ext_working_drv->LongName, 0, sizeof(WORD) * MAX_L_NAME_LENG);
    }

    if (from_root_dir == TRUE)
    {
        /* back to beginning of root directory */
        if (DirReset(ext_working_drv) != FS_SUCCEED)
        {
            MP_ALERT("task ID %d: %s: -E- DirReset() back to beginning of root directory failed !", task_id, __FUNCTION__);
            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
            ext_mem_free(ori_u16_path_pointer);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("task ID %d: %s: DirReset() back to beginning of root directory OK !", task_id, __FUNCTION__);
        }
    }
    else
    {
        /* back to beginning of current directory */
        ret = DirFirst(ext_working_drv);
        if ((ret != FS_SUCCEED) && (ret != END_OF_DIR))
        {
            MP_ALERT("task ID %d: %s: Error! DirFirst() back to beginning of current directory failed !", task_id, __FUNCTION__);
            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
            ext_mem_free(ori_u16_path_pointer);
            return ABNORMAL_STATUS;
        }
        else if (ret == END_OF_DIR)
        {
            MP_ALERT("%s: -I- current directory is an empty directory.", __FUNCTION__);

            /* process the mandatory return values carefully for this case */
            *drv_ID = new_drv_id; /* return drive ID */
            if (final_token_offset)
                *final_token_offset = ((WORD *) u16_path - (WORD *) ori_u16_path_pointer); /* offset of the final filename token to return */
            if (final_token_strlen)
                *final_token_strlen = StringLength16(u16_path);

            ext_mem_free(ori_u16_path_pointer);
            return FILE_NOT_FOUND; /* file or directory not found following the path on the drive */
        }
        else
        {
            MP_DEBUG("task ID %d: %s: DirFirst() back to beginning of current directory OK !", task_id, __FUNCTION__);
        }
    }

    /* walk through the rest of path to locate the file */
    MpMemSet((WORD *) tmpU16str, 0, sizeof(WORD) * 4);
    tmpU16str[0] = 0x002F; /* 0x2F = '/' => tmpU16str = UTF-16 string of "/" */
    wToken = String16_strstr(u16_path, (WORD *) tmpU16str);

    do
    {
        if (wToken == NULL) /* '/' not found => this token is the final token */
        {
            MP_DEBUG("task ID %d: %s: '/' not found => this token is the final token.", task_id, __FUNCTION__);
            final_token_reached = TRUE;
            wToken = u16_path;
            TrimOffString16TailSpaces(wToken);
        }
        else
        {
            *wToken = 0x0000; /* overwrite found '/' */
            final_token_reached = FALSE;
            wToken = u16_path;
        }

        /* check directory "." for same level hierarchy */
        MpMemSet((WORD *) tmpU16str, 0, sizeof(WORD) * 4);
        tmpU16str[0] = 0x002E; /* 0x2E = '.' => tmpU16str = UTF-16 string of "." */
        if (StringCompare16(wToken, tmpU16str) == E_COMPARE_EQUAL) /* "." means same level hierarchy */
        {
            MP_DEBUG("task ID %d: %s: '.' => same directory.", task_id, __FUNCTION__);
            if ((final_token_reached == TRUE) && ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)))
            {
                ret = FS_SUCCEED; /* path target found on the drive */
                final_entry_type = E_PATH_TARGET_IS_DIR;
                break; /* end of processing */
            }
            else
                goto L_u16_next_token_r;
        }

        /* check directory ".." for parent directory */
        MpMemSet((WORD *) tmpU16str, 0, sizeof(WORD) * 4);
        tmpU16str[0] = 0x002E;
        tmpU16str[1] = 0x002E; /* 0x2E = '.' => tmpU16str = UTF-16 string of ".." */
        if (StringCompare16(wToken, tmpU16str) == E_COMPARE_EQUAL) /* ".." means parent directory */
        {
            MP_DEBUG("task ID %d: %s: '..' => go to parent directory ...", task_id, __FUNCTION__);
            if (CdParent(ext_working_drv) != FS_SUCCEED)
            {
                MP_ALERT("task ID %d: %s: Error! Cannot change to parent directory !", task_id, __FUNCTION__);
                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                ext_mem_free(ori_u16_path_pointer);
                return ABNORMAL_STATUS;
            }
            else
            {
                MP_DEBUG("task ID %d: %s: changed to parent directory OK.", task_id, __FUNCTION__);
                if ((final_token_reached == TRUE) && ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)))
                {
                    ret = FS_SUCCEED; /* path target found on the drive */
                    final_entry_type = E_PATH_TARGET_IS_DIR;
                    break; /* end of processing */
                }
                else
                    goto L_u16_next_token_r;
            }
        }
        else
        {
            MpMemSet(bToken, 0, MAX_L_NAME_LENG);
            mpx_UtilU16ToU08(bToken, wToken);
            MP_DEBUG("task ID %d: %s: token = [%s]", task_id, __FUNCTION__, bToken);

            if (*wToken == 0x0000)
            {
                if (path_target_type == E_PATH_TARGET_IS_FILE)
                {
                    MP_ALERT("task ID %d: %s: Error! token is a NULL string, cannot be a file !", task_id, __FUNCTION__);
                    *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    ext_mem_free(ori_u16_path_pointer);
                    return ABNORMAL_STATUS;
                }
                else if ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR))
                {
                    MP_DEBUG("task ID %d: %s: token is a NULL string, it's OK for path of directory case.", task_id, __FUNCTION__);
                    wToken = final_meaningful_wToken; /* return last meaningful token */
                    ret = FS_SUCCEED; /* directory found on the drive */
                    final_entry_type = E_PATH_TARGET_IS_DIR;
                    break;
                }
            }
            else if ((*wToken == 0x0020) || (*wToken == 0x0009)) /* first character of token cannot be a space (ASCII 0x20) or Tab (ASCII 0x09) */
            {
                if (final_token_reached == FALSE)
                {
                    MP_ALERT("task ID %d: %s: Error! token string with invalid leading character code !", task_id, __FUNCTION__);
                    *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    ext_mem_free(ori_u16_path_pointer);
                    return ABNORMAL_STATUS;
                }
                else
                {
                    if (path_target_type == E_PATH_TARGET_IS_FILE)
                    {
                        MP_ALERT("task ID %d: %s: Error! token string with invalid leading character code !", task_id, __FUNCTION__);
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        ext_mem_free(ori_u16_path_pointer);
                        return ABNORMAL_STATUS;
                    }
                    else if ((path_target_type == E_PATH_TARGET_IS_DIR) || (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR))
                    {
                        WORD *tmp_word = wToken;

                        while ((*tmp_word == 0x0020) || (*tmp_word == 0x0009))
                        {
                           ((WORD *) tmp_word)++;
                        }

                        if (*tmp_word != 0x0000)
                        {
                            MP_ALERT("task ID %d: %s: Error! token string with invalid leading character code !", task_id, __FUNCTION__);
                            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                            ext_mem_free(ori_u16_path_pointer);
                            return ABNORMAL_STATUS;
                        }
                        else
                        {
                            MP_DEBUG("task ID %d: %s: it's OK for path of directory case to have final token only have space or tab characters.", task_id, __FUNCTION__);
                            wToken = final_meaningful_wToken; /* return last meaningful token */
                            ret = FS_SUCCEED; /* directory found on the drive */
                            final_entry_type = E_PATH_TARGET_IS_DIR;
                            break;
                        }
                    }
                }
            }
            else /* normal token cases, record the token */
            {
                final_meaningful_wToken = wToken;
            }

            /* For file create/open/delete/... cases, final token in the path is "File" type and E_PATH_TARGET_IS_FILE should be specified.
             * For directory create/change/delete/... cases, final token in the path is "Dir" type and E_PATH_TARGET_IS_DIR should be specified.
             * All previous tokens should be "Dir".
             */
            if (final_token_reached == TRUE)
            {
                if (path_target_type == E_PATH_TARGET_IS_FILE)
                    entry_type = E_FILE_TYPE;
                else if (path_target_type == E_PATH_TARGET_IS_DIR)
                    entry_type = E_DIR_TYPE;
                else if (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)
                    entry_type = E_BOTH_FILE_AND_DIR_TYPE;
            }
            else
                entry_type = E_DIR_TYPE;

            if (FileSearchLN(ext_working_drv, wToken, StringLength16(wToken), entry_type) == FS_SUCCEED)
            {
                MP_DEBUG("task ID %d: %s: FileSearchLN() token [%s] found !", task_id, __FUNCTION__, bToken);

                /* note: the file attributes reside in different DirEntries in FAT12/16/32 and exFAT */
                if (drv->Flag.FsType != FS_TYPE_exFAT)
                    fileAttribute = (WORD) ext_working_drv->Node->Attribute;
            #if EXFAT_ENABLE
                else
                    fileAttribute = (WORD) ext_working_drv->CacheBufPoint->fileAttributes;
            #endif

                if (fileAttribute & FDB_SUB_DIR)
                {
                    MP_DEBUG("task ID %d: %s: token [%s] is a folder => CdSub() ...", task_id, __FUNCTION__, bToken);
                    if (CdSub(ext_working_drv) != FS_SUCCEED)
                    {
                        MP_ALERT("task ID %d: %s: Error! failed changing to sub-directory [%s] !", task_id, __FUNCTION__, bToken);
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        ext_mem_free(ori_u16_path_pointer);
                        return ABNORMAL_STATUS;
                    }
                    else
                    {
                        MP_DEBUG("task ID %d: %s: changed to sub-directory [%s] OK.", task_id, __FUNCTION__, bToken);
                        if (final_token_reached == TRUE)
                        {
                            ret = FS_SUCCEED; /* path target found on the drive */
                            final_entry_type = E_PATH_TARGET_IS_DIR;
                            break; /* end of processing */
                        }
                        else
                            goto L_u16_next_token_r;
                    }
                }
                else
                {
                    MP_DEBUG("task ID %d: %s: token [%s] is a file, and file found !", task_id, __FUNCTION__, bToken);
                    ret = FS_SUCCEED; /* path target found on the drive */
                    final_entry_type = E_PATH_TARGET_IS_FILE;
                    break; /* end of processing */
                }
            }
            else /* not found */
            {
                if (final_token_reached == TRUE)
                {
                    if (path_target_type == E_PATH_TARGET_IS_FILE)
                    {
                        MP_ALERT("task ID %d: %s: -I- file [%s] not found in the path.", task_id, __FUNCTION__, bToken);
                    }
                    else if (path_target_type == E_PATH_TARGET_IS_DIR)
                    {
                        MP_ALERT("task ID %d: %s: -I- directory [%s] not found in the path.", task_id, __FUNCTION__, bToken);
                    }
                    else if (path_target_type == E_PATH_TARGET_DONT_CARE_FILE_OR_DIR)
                    {
                        MP_ALERT("task ID %d: %s: -I- entry [%s] not found in the path.", task_id, __FUNCTION__, bToken);
                    }

                    if ((midway_dir_op_type == E_AUTO_CREATE_MIDWAY_DIR) && (path_target_type == E_PATH_TARGET_IS_DIR))
                    {
                        MP_ALERT("task ID %d: %s: create final directory [%s] ... ", task_id, __FUNCTION__, bToken);
                        if (MakeDir_UTF16(ext_working_drv, wToken) == FS_SUCCEED)
                        {
                            MP_DEBUG("task ID %d: %s: final directory [%s] created => CdSub() ...", task_id, __FUNCTION__, bToken);
                            if (CdSub(ext_working_drv) != FS_SUCCEED)
                            {
                                MP_ALERT("task ID %d: %s: Error! failed changing to sub-directory [%s] !", task_id, __FUNCTION__, bToken);
                                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                                ext_mem_free(ori_u16_path_pointer);
                                return ABNORMAL_STATUS;
                            }

                            MP_DEBUG("task ID %d: %s: changed to sub-directory [%s] OK.", task_id, __FUNCTION__, bToken);
                            ret = FS_SUCCEED; /* path target found on the drive */
                            final_entry_type = E_PATH_TARGET_IS_DIR;
                            break; /* end of processing */
                        }
                        else
                        {
                            MP_ALERT("task ID %d: %s: create final directory [%s] failed !", task_id, __FUNCTION__, bToken);
                            ret = ABNORMAL_STATUS;
                            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        }
                    }
                    else
                    {
                        ret = FILE_NOT_FOUND; /* file or directory not found following the path on the drive */
                        *drv_ID = new_drv_id; /* return drive ID */
                        if (final_token_offset)
                            *final_token_offset = ((WORD *) wToken - (WORD *) ori_u16_path_pointer); /* offset of the final filename token to return */
                        if (final_token_strlen)
                            *final_token_strlen = StringLength16(wToken);
                    }
                }
                else /* midway directory */
                {
                    if (midway_dir_op_type == E_NOT_CREATE_MIDWAY_DIR)
                    {
                        MP_ALERT("task ID %d: %s: -I- directory [%s] not found in the path.", task_id, __FUNCTION__, bToken);
                        ret = ABNORMAL_STATUS;
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    }
                    else if (midway_dir_op_type == E_AUTO_CREATE_MIDWAY_DIR)
                    {
                        MP_ALERT("task ID %d: %s: create midway directory [%s] ... ", task_id, __FUNCTION__, bToken);
                        if (MakeDir_UTF16(ext_working_drv, wToken) == FS_SUCCEED)
                        {
                            MP_DEBUG("task ID %d: %s: midway directory [%s] created => CdSub() ...", task_id, __FUNCTION__, bToken);
                            if (CdSub(ext_working_drv) != FS_SUCCEED)
                            {
                                MP_ALERT("task ID %d: %s: Error! failed changing to sub-directory [%s] !", task_id, __FUNCTION__, bToken);
                                *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                                ext_mem_free(ori_u16_path_pointer);
                                return ABNORMAL_STATUS;
                            }

                            MP_DEBUG("task ID %d: %s: changed to sub-directory [%s] OK.", task_id, __FUNCTION__, bToken);
                            goto L_u16_next_token_r;
                        }
                        else
                        {
                            MP_ALERT("task ID %d: %s: create midway directory [%s] failed !", task_id, __FUNCTION__, bToken);
                            ret = ABNORMAL_STATUS;
                            *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                        }
                    }
                    else
                    {
                        MP_ALERT("task ID %d: %s: Error! Invalid midway_dir_op_type parameter (%s) !", task_id, __FUNCTION__, midway_dir_op_type);
                        ret = ABNORMAL_STATUS;
                        *drv_ID = NULL_DRIVE; /* here, use NULL_DRIVE represents fatal error */
                    }
                }

                ext_mem_free(ori_u16_path_pointer);
                return ret;
            }
        }

L_u16_next_token_r:
        u16_path = (WORD *) ((WORD *) u16_path + StringLength16(wToken) + 1); /* skip the token and '/' */
        MpMemSet((WORD *) tmpU16str, 0, sizeof(WORD) * 4);
        tmpU16str[0] = 0x002F; /* 0x2F = '/' => tmpU16str = UTF-16 string of "/" */
        wToken = String16_strstr(u16_path, (WORD *) tmpU16str);
    } while (final_token_reached == FALSE);

    /* prepare return and output values */
    *drv_ID = new_drv_id; /* return drive ID */
    *found_entry_type = final_entry_type; /* return found entry type */
    if (final_token_offset)
        *final_token_offset = ((WORD *) wToken - (WORD *) ori_u16_path_pointer); /* offset of the final filename token to return */
    if (final_token_strlen)
        *final_token_strlen = StringLength16(wToken);

    ext_mem_free(ori_u16_path_pointer);
    return ret;
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement as same as the C standard library "fopen" function.\n
///         It will return a file handle if opening the specified file is successful. \n\n
///         This function is the "re-entrant / thread-safe" version of PathAPI__Fopen_UTF16() !
///
///@param   utf16_path_str   The UTF-16 path string of a file to be opened or created. \n\n
///
///@param   mode             File access mode permitted. Currently only two modes are supported: "rb" and "wb".\n
///                          But note file access right and protection mechanism is not supported yet. \n\n
///
///@param   ext_working_drv  Drive handle of the external 'working DRIVE' for the calling task to work for "re-entrant / thread-safe". \n\n
///
///@return   The function call will return the file handle if locate/open file successfully, otherwise return NULL if any error. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
///@remark   1. The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   2. If any midway directory in the path does not exist, this function treats it as error and return NULL. \n\n
///
///@remark   3. For "re-entrant / thread-safe" to be called by multiple tasks concurrently, a 'working DRIVE' handle parameter must be feed by each task to work ! \n\n
///
STREAM *PathAPI__Fopen_UTF16_r(const WORD * utf16_path_str, const char * mode, DRIVE * ext_working_drv)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    STREAM *fHandle = NULL;
    BOOL   file_found = FALSE;
    BYTE   task_id = TaskGetId();
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    WORD   *wUtf16_TempBuf;
    int    ret;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("task ID %d: %s: Invalid input path string !", task_id, __FUNCTION__);
        return NULL;
    }

    if (ext_working_drv == NULL)
    {
        MP_ALERT("task ID %d: %s: Error! NULL working drive handle !", task_id, __FUNCTION__);
        return NULL;
    }

    if (mode == NULL)
    {
        MP_ALERT("%s: NULL mode pointer !", __FUNCTION__);
        return NULL;
    }
    else if ((strcmp(mode, "rb") != 0) && (strcmp(mode, "wb") != 0))
    {
        MP_ALERT("task ID %d: %s: Error! unsupported file access mode [%s] => return.", task_id, __FUNCTION__, mode);
        return NULL;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16_r(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, ext_working_drv, E_PATH_TARGET_IS_FILE, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("task ID %d: %s: Error! path/file cannot be located due to some error!", task_id, __FUNCTION__);
            return NULL;
        }
        else
        {
            MP_DEBUG("task ID %d: %s: locate path OK, but file not found in the path!", task_id, __FUNCTION__);
            file_found = FALSE;
        }
    }
    else
    {
        MP_DEBUG("task ID %d: %s: locate path OK, and file found in the path!", task_id, __FUNCTION__);
        file_found = TRUE;
    }

    if (file_found) /* file found => open it */
    {
        MP_ALERT("task ID %d: %s: file found => open it ...", task_id, __FUNCTION__);
        fHandle = FileOpen(ext_working_drv);
    }
    else /* file not found */
    {
        if (strcmp(mode, "rb") == 0) /* do not force file creation if file not exist */
        {
            MP_ALERT("task ID %d: %s: Error! file not found. file access mode is [%s] => return.", task_id, __FUNCTION__, mode);
            return NULL;
        }
        else if (strcmp(mode, "wb") == 0) /* force file creation if file not exist */
        {
            MP_ALERT("task ID %d: %s: file not found, file access mode is [%s] => create file ...", task_id, __FUNCTION__, mode);

            if ( (WORD) *((WORD *) utf16_path_str + (final_token_offset + final_token_strlen)) == 0x0000 )
                ret = CreateFile_UTF16(ext_working_drv, (WORD *) utf16_path_str + final_token_offset);
            else
            {
                /* trim off tailing spaces */
                wUtf16_TempBuf = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
                if (wUtf16_TempBuf == NULL)
                {
                    MP_ALERT("-E %s() malloc fail", __FUNCTION__);
                    return NULL;
                }
                MpMemSet((BYTE *) wUtf16_TempBuf, 0, MAX_L_NAME_LENG * 2);
                MpMemCopy((BYTE *) wUtf16_TempBuf, (BYTE *) ((WORD *) utf16_path_str + final_token_offset), final_token_strlen * 2);

                ret = CreateFile_UTF16(ext_working_drv, (WORD *) wUtf16_TempBuf);
                ext_mem_free(wUtf16_TempBuf);
            }

            if (ret != FS_SUCCEED)
            {
                MP_ALERT("task ID %d: %s: Error! file creation failed !", task_id, __FUNCTION__);
                return NULL;
            }
            else
            {
                MP_ALERT("task ID %d: %s: file creation OK !", task_id, __FUNCTION__);
            }

            fHandle = FileOpen(ext_working_drv);
        }
    }

    return fHandle; /* return file handle */
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement file creation function with path of the file supported.\n
///         It will return a file handle for the created file if creation is successful. \n\n
///         This function is the "re-entrant / thread-safe" version of PathAPI__CreateFile_UTF16() !
///
///@param   utf16_path_str   The UTF-16 path string of a file to be created. \n\n
///
///@param   ext_working_drv  Drive handle of the external 'working DRIVE' for the calling task to work for "re-entrant / thread-safe". \n\n
///
///@return   The function call will return the file handle if create file successfully, otherwise return NULL if any error. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
///@remark   1. The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   2. This function acts like CreateFile_UTF16() when a file with same filename already exists in the path because
///             this function actually invokes CreateFile_UTF16() to do the file creation. \n\n
///
///@remark   3. If midway directories in the path do not exist, this function will create them automatically. \n
///             This is different from the behavior of PathAPI__Fopen_XXXX() functions with "wb" (write) mode. \n\n
///
///@remark   4. For "re-entrant / thread-safe" to be called by multiple tasks concurrently, a 'working DRIVE' handle parameter must be feed by each task to work ! \n\n
///
STREAM *PathAPI__CreateFile_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    STREAM *fHandle = NULL;
    BOOL   file_found = FALSE;
    BYTE   task_id = TaskGetId();
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    WORD   *wUtf16_TempBuf;
    int    ret;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("task ID %d: %s: Invalid input path string !", task_id, __FUNCTION__);
        return NULL;
    }

    if (ext_working_drv == NULL)
    {
        MP_ALERT("task ID %d: %s: Error! NULL working drive handle !", task_id, __FUNCTION__);
        return NULL;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16_r(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, ext_working_drv, E_PATH_TARGET_IS_FILE, E_AUTO_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("task ID %d: %s: Error! path/file cannot be located due to some error!", task_id, __FUNCTION__);
            return NULL;
        }
        else
        {
            MP_DEBUG("task ID %d: %s: locate path OK, and file not found in the path.", task_id, __FUNCTION__);
            file_found = FALSE;
        }
    }
    else
    {
        MP_ALERT("task ID %d: %s: locate path OK, and a file with same filename exists in the path.", task_id, __FUNCTION__);
        file_found = TRUE;
    }

    /* no matter file found or not in the final directory, let CreateFile() to do the job */

    if ( (WORD) *((WORD *) utf16_path_str + (final_token_offset + final_token_strlen)) == 0x0000 )
        ret = CreateFile_UTF16(ext_working_drv, (WORD *) utf16_path_str + final_token_offset);
    else
    {
        /* trim off tailing spaces */
        wUtf16_TempBuf = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
        if (wUtf16_TempBuf == NULL)
        {
            MP_ALERT("-E %s() malloc fail", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        MpMemSet((BYTE *) wUtf16_TempBuf, 0, MAX_L_NAME_LENG * 2);
        MpMemCopy((BYTE *) wUtf16_TempBuf, (BYTE *) ((WORD *) utf16_path_str + final_token_offset), final_token_strlen * 2);

        ret = CreateFile_UTF16(ext_working_drv, (WORD *) wUtf16_TempBuf);
        ext_mem_free(wUtf16_TempBuf);
    }

    if (ret != FS_SUCCEED)
    {
        MP_ALERT("task ID %d: %s: Error! file creation failed !", task_id, __FUNCTION__);
        return NULL;
    }
    else
    {
        MP_ALERT("task ID %d: %s: file creation OK !", task_id, __FUNCTION__);
        fHandle = FileOpen(ext_working_drv);
        return fHandle; /* return file handle */
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement as same as the C standard library "remove" function.\n
///         It will delete the file specified by the path/filename string. \n\n
///         This function is the "re-entrant / thread-safe" version of PathAPI__RemoveFile_UTF16() !
///
///@param   utf16_path_str   The UTF-16 path string of a file to be deleted. \n\n
///
///@param   ext_working_drv  Drive handle of the external 'working DRIVE' for the calling task to work for "re-entrant / thread-safe". \n\n
///
///@retval  FS_SUCCEED          Delete file successfully. \n\n
///@retval  ABNORMAL_STATUS     Delete file unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no need to delete it. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
///@remark   1. The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   2. For "re-entrant / thread-safe" to be called by multiple tasks concurrently, a 'working DRIVE' handle parameter must be feed by each task to work ! \n\n
///
int PathAPI__RemoveFile_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    STREAM *fHandle = NULL;
    int    ret;
    BYTE   task_id = TaskGetId();
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("task ID %d: %s: Invalid input path string !", task_id, __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (ext_working_drv == NULL)
    {
        MP_ALERT("task ID %d: %s: Error! NULL working drive handle !", task_id, __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16_r(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, ext_working_drv, E_PATH_TARGET_IS_FILE, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("task ID %d: %s: Error! path/file cannot be located due to some error !", task_id, __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("task ID %d: %s: locate path OK, but file not found in the path !", task_id, __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        if ((found_entry_type == E_PATH_TARGET_IS_FILE) && (final_token_strlen > 0))
        {
            MP_DEBUG("task ID %d: %s: locate path OK, and file found in the path !", task_id, __FUNCTION__);
            MP_ALERT("task ID %d: %s: file found => delete it ...", task_id, __FUNCTION__);
            fHandle = FileOpen(ext_working_drv);
            ret = DeleteFile(fHandle);
            return ret;
        }
        else
        {
            MP_DEBUG("task ID %d: %s: -I- locate path OK, but the target is not a file => Check FileSystem source code !", task_id, __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to get file attribute and date/time info of a file/directory. \n
///         This function is the "re-entrant / thread-safe" version of PathAPI__GetFileAttributeAndTimeInfo_UTF16() ! \n\n
///
///@param   utf16_path_str      [IN] The UTF-16 path string of a file/directory to get its info. \n\n
///
///@param   ext_working_drv     [IN] Drive handle of the external 'working DRIVE' for the calling task to work for "re-entrant / thread-safe". \n\n
///
///@param   dir_entry_info      [OUT] Returned structure containing file attribute and date/time info. \n\n
///
///@retval  FS_SUCCEED          Get file attribute and date/time info successfully. \n\n
///@retval  ABNORMAL_STATUS     Get file attribute and date/time info unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no file attribute and date/time info returned. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/test_pic.jpg  (search beginning from CF root directory) \n
///          [b]   CF:test_folder_1/sub2/longname_folder_level3/test_pic.jpg   (search beginning from CF current directory) \n
///          [c]   /test_folder_1/sub2/longname_folder_level3/test_pic.jpg     (search beginning from current drive root directory) \n
///          [d]   test_folder_1/sub2/longname_folder_level3/test_pic.jpg      (search beginning from current drive current directory) \n\n
///
///@remark   The path string must be converted to UTF-16 before calling this function. \n\n
///
int PathAPI__GetFileAttributeAndTimeInfo_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv, DIR_ENTRY_INFO_TYPE * dir_entry_info)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    int    ret;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    WORD   *wUtf16_TempBuf;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (dir_entry_info == NULL)
    {
        MP_ALERT("%s: Error ! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16_r(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, ext_working_drv, E_PATH_TARGET_DONT_CARE_FILE_OR_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/file cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: locate path OK, but file not found in the path !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        MP_DEBUG("%s: locate path OK, and directory entry found in the path !", __FUNCTION__);

        if (found_entry_type == E_PATH_TARGET_IS_DIR)
        {
            CdParent(ext_working_drv);

            if ( (WORD) *((WORD *) utf16_path_str + (final_token_offset + final_token_strlen)) == 0x0000 )
                ret = FileSearchLN(ext_working_drv, (WORD *) utf16_path_str + final_token_offset, final_token_strlen, E_DIR_TYPE);
            else
            {
                /* trim off tailing spaces */
                wUtf16_TempBuf = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
                if (wUtf16_TempBuf == NULL)
                {
                    MP_ALERT("-E %s() malloc fail", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }
                MpMemSet((BYTE *) wUtf16_TempBuf, 0, MAX_L_NAME_LENG * 2);
                MpMemCopy((BYTE *) wUtf16_TempBuf, (BYTE *) ((WORD *) utf16_path_str + final_token_offset), final_token_strlen * 2);

                ret = FileSearchLN(ext_working_drv, (WORD *) wUtf16_TempBuf, final_token_strlen, E_DIR_TYPE);
                ext_mem_free(wUtf16_TempBuf);
            }

            if (ret != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! FileSearchLN() failed, cannot move to the directory entry position !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
        }

        if (GetFileAttributeAndTimeInfoFromFDB(ext_working_drv, (FDB *) ext_working_drv->Node, dir_entry_info) == PASS)
            ret = FS_SUCCEED;
        else
            ret = ABNORMAL_STATUS;

        return ret;
    }
}
#endif //FS_REENTRANT_API



///
///@ingroup FILE
///@brief   Open a file on the specified drive.
///
///@param   drv      The handle of the file to open.
///
///@return  The function call will return NULL if no file handle can use, else return the file handle.
///
///@remark  The file to open is implicitly pointed to by current FDB node (drv->Node) before calling this function. \n
///
///@remark  Note for exFAT drive, our DirNext(), DirPrevious(), DirFirst() and DirLast() functions will move the current FDB node (drv->Node) \n
///         to the "Stream Extension DirectoryEntry" position of the file or directory if a file or directory is available.
///
STREAM *FileOpen(DRIVE * drv)
{
    STREAM *handle;
    FDB *node;

    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return NULL;
    }

    handle = GetFreeFileHandle(drv);
    if (handle == NULL)
    {
        MP_ALERT("%s: No file handle available !", __FUNCTION__);
        return NULL;
    }

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        ENTRY_FILENAME_TYPE  bFilenameType = E_FILENAME_UNKNOWN;
        BYTE  pbBuf[512];
        DWORD pdwBufferCount = 0;

        if (GetFilenameOfCurrentNode(drv, &bFilenameType, pbBuf, &pdwBufferCount, 512) != FS_SUCCEED)
        {
            MP_ALERT("%s: Error! GetFilenameOfCurrentNode() failed !", __FUNCTION__);
            IntDisable();
            handle->Drv = 0; // clear drive field mean clear handle
            HandleCounter--;
            IntEnable();
            return NULL;
        }

        if (bFilenameType == E_FILENAME_8_3_SHORT_NAME)
            handle->Flag.f_Utf16_LongName = 0;
        else if (bFilenameType == E_FILENAME_UTF16_LONG_NAME)
            handle->Flag.f_Utf16_LongName = 1;

        node = (FDB *) (drv->Node); /* GetFilenameOfCurrentNode() will go back to its original FDB position */

        handle->DirSector = drv->DirCachePoint; // the lba of DIR entry of current file
        handle->FdbOffsetInDirCache = ((WORD) ((BYTE *) node - (BYTE *) drv->DirCacheBuffer) >> 5); //unit: number of FDB nodes
        handle->Flag.SizeChanged = 0;

        ChainInit((CHAIN *) (&handle->Chain),
                  (LoadAlien16(&node->StartHigh) << 16) + LoadAlien16(&node->StartLow), LoadAlien32(&node->Size));
    }
#if EXFAT_ENABLE
    else
    {
        TYPE_exFAT_Stream_Extension_DirEntry *exfat_node;
        BYTE *Hi32_of_64, *Lo32_of_64;
        DWORD dwFileSize = 0;
        U64   u64FileSize;

        /* current drv->Node should be a "Stream Extension DirectoryEntry" of exFAT */
        exfat_node = (TYPE_exFAT_Stream_Extension_DirEntry *) (drv->Node);
        if (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT)
        {
            MP_ALERT("%s: Error! Current node should be a (Stream Extension DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
            IntDisable();
            handle->Drv = 0; // clear drive field mean clear handle
            HandleCounter--;
            IntEnable();
            return NULL;
        }

        handle->DirSector = drv->DirCachePoint; // the lba of DIR entry of current file
        handle->FdbOffsetInDirCache = ((WORD) ((BYTE *) node - (BYTE *) drv->DirCacheBuffer) >> 5); //unit: number of FDB nodes
        handle->Flag.SizeChanged = 0;
        handle->Flag.f_Utf16_LongName = 1;

        if (((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->generalSecondaryFlags & EXFAT_DirEntryFlags_MASK_NoFatChain)
            handle->Chain.exfat_DirEntryFlags.f_NoFatChain = 1;
        else
            handle->Chain.exfat_DirEntryFlags.f_NoFatChain = 0;

        Lo32_of_64 = (BYTE *) &exfat_node->dataLength; /* due to little-endian */
        Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
        u64FileSize = (U64) ((U64) LoadAlien32(Hi32_of_64) << 32) + (U64) LoadAlien32(Lo32_of_64);
        if ((U64) u64FileSize <= (U64) 0x00000000FFFFFFFF)
        {
            dwFileSize = (DWORD) u64FileSize;

mpDebugPrint("%s: within 32-bit range, => ChainInit(chain, firstCluster = 0x%08x, dwFileSize = %lu)...", __FUNCTION__,
             LoadAlien32(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->firstCluster), dwFileSize);

            ChainInit((CHAIN *) (&handle->Chain),
                      LoadAlien32(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->firstCluster), dwFileSize);
        }
        else
        {
mpDebugPrint("%s: To-Do: 64-bit size parameter may cause problem during push/pop to/from the stack !?", __FUNCTION__);
mpDebugPrint("%s: To-Do: Modify ChainInit() to accept 64-bit size parameter ...", __FUNCTION__);
            ChainInit((CHAIN *) (&handle->Chain),
                      LoadAlien32(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->firstCluster), (U64) u64FileSize);
        }
    }
#endif

    return handle;
}



#if (RAM_FILE_ENABLE)
///
///@ingroup FILE
///@brief   Allocate/create a "RamFile" with memory allocated for the specified size.
///
///@param   buf_size      The size of buffer memory to allocate for the file content of the "RamFile".
///
///@return  The function call will return NULL if no file handle can use, else return the file handle.
///
///@remark  The caller task should not access the handle->Drv (DRIVE pointer) for any field within the DRIVE structure,
///         because a "RamFile" has no association to any storage drive. The 'handle->Drv' pointer value of a "RamFile"
///         is always NULL.
///
STREAM  *RamFileAlloc(DWORD buf_size)
{
    STREAM *handle;
    BYTE   *buf;


    if (buf_size == 0)
    {
        MP_ALERT("%s: Error! buf_size cannot be zero !", __FUNCTION__);
        return NULL;
    }

    handle = GetFreeFileHandle(NULL);
    if (handle == NULL)
    {
        MP_ALERT("%s: No file handle available !", __FUNCTION__);
        return NULL;
    }

    memset((BYTE *) handle, 0, sizeof(STREAM)); /* clear all fields for the "RamFile" first */
    handle->Flag.f_IsRamFile = 1;

    buf = (BYTE *) ext_mem_malloc(buf_size);
    if (buf == NULL)
    {
        MP_ALERT("%s: mem alloc failed for (%lu) bytes !", __FUNCTION__, buf_size);
        FileClose(handle);
        return NULL;
    }
    MpMemSet(buf, 0, buf_size);

    handle->ramFile_buf = buf;
    handle->ramFile_bufSize = buf_size;

    return handle;
}
#endif



///
///@ingroup FILE
///@brief   Close an opened file.
///
///@param   handle      The handle of the file to close.
///
///@retval  FS_SUCCEED          Close file successfully. \n\n
///@retval  ABNORMAL_STATUS     Close file unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no need to close it.
///
int FileClose(STREAM * handle)
{
    DRIVE *drv;
    FDB *node;
    CHAIN *chain;


    if (handle == NULL)
        return FILE_NOT_FOUND;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        if ((handle->ramFile_buf != NULL) && (handle->ramFile_bufSize > 0))
            ext_mem_free(handle->ramFile_buf);

        MpMemSet((BYTE *) handle, 0, sizeof(STREAM)); /* clear all fields for clearing "RamFile" flag and others */

        IntDisable();
        HandleCounter--;
        IntEnable();
        return FS_SUCCEED;
    }
#endif

    drv = (DRIVE *) (handle->Drv);
    if (drv == NULL)
        return FILE_NOT_FOUND; // this file has been closed, except "RamFile"

    if (handle->Flag.SizeChanged)
    {
        /* make sure drive's Dir cache buffer covers the file FDB */
        if (FileDirCaching(handle) != FS_SUCCEED)
        {
            MP_ALERT("%s: -E- FileDirCaching() failed !", __FUNCTION__);
            IntDisable();
            handle->Drv = 0; // clear drive field mean clear handle
            HandleCounter--;
            IntEnable();
            return ABNORMAL_STATUS;
        }

        node = (FDB *) drv->DirCacheBuffer + handle->FdbOffsetInDirCache;
        chain = (CHAIN *) & handle->Chain;

        if (drv->Flag.FsType != FS_TYPE_exFAT)
        {
            if (! handle->Flag.f_ExtraClustersAllocated)  //normal FileWrite() case
                SaveAlien32(&node->Size, chain->Size);
            else //FileWrite_withWriteBack_FileSize() case
            {
                if (chain->Point == chain->Size)
                    SaveAlien32(&node->Size, chain->Size);
                else
                {
#if RELEASE_NEEDLESS_EXTRA_CLUSTERS_WHEN_FILE_CLOSE
                    MP_ALERT("%s: adjust file's chain size to (%lu) bytes ...", __FUNCTION__, chain->Point);

                    /* reduce chain size to release needless FAT clusters behind actual file content size */
                    if (ChainChangeSize(drv, chain, chain->Point) != FS_SUCCEED)
                    {
                        IntDisable();
                        handle->Drv = 0; // clear drive field mean clear handle
                        HandleCounter--;
                        IntEnable();
                        return ABNORMAL_STATUS;
                    }

                    SaveAlien32(&node->Size, chain->Point);
#else
                    SaveAlien32(&node->Size, chain->Size);
#endif
                    handle->Flag.f_ExtraClustersAllocated = 0;
                }
            }
        }
#if EXFAT_ENABLE
        else
        {
            TYPE_exFAT_Stream_Extension_DirEntry *exfat_node;
            BYTE *Hi32_of_64, *Lo32_of_64;
            DWORD dwSize = 0;

            /* current drv->Node should be a "Stream Extension DirectoryEntry" of exFAT */
            exfat_node = (TYPE_exFAT_Stream_Extension_DirEntry *) (drv->Node);
            if (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT)
            {
                MP_ALERT("%s: Error! Current node should be a (Stream Extension DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
                return ABNORMAL_STATUS;
            }

            /* note: Currently, we do not support WRITE operation in exFAT, so we don't need to process 'f_ExtraClustersAllocated' flag here */

            Lo32_of_64 = (BYTE *) &exfat_node->dataLength; /* due to little-endian */
            Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
            if ((U64) chain->Size <= (U64) 0x00000000FFFFFFFF)
            {
                dwSize = (DWORD) chain->Size;
                SaveAlien32(Hi32_of_64, 0);
                SaveAlien32(Lo32_of_64, dwSize);
            }
            else
            {
                dwSize = (DWORD) (((U64) chain->Size & 0xFFFFFFFF00000000) >> 32);
                SaveAlien32(Hi32_of_64, dwSize);

                dwSize = (DWORD) ((U64) chain->Size & 0x00000000FFFFFFFF);
                SaveAlien32(Lo32_of_64, dwSize);
            }
        }
#endif
        drv->Flag.DirCacheChanged = 1;

        // here, write back changed DirCache and FatCache and free cluster info together
        if (DriveRefresh(drv) != FS_SUCCEED)
        {
            IntDisable();
            handle->Drv = 0; // clear drive field mean clear handle
            HandleCounter--;
            IntEnable();
            return ABNORMAL_STATUS;
        }

        handle->Flag.SizeChanged = 0;
    }

    IntDisable();
    handle->Drv = 0; // clear drive field mean clear handle
    HandleCounter--;
    IntEnable();

    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Close an opened file without writing back any cached/changed file data to storage.
///         That is, this function is just to recycle the file handle entry it allocated.
///
///@param   handle      The handle of the file to close (to recycle the file handle entry it allocated).
///
///@retval  FS_SUCCEED          Close file successfully. \n\n
///@retval  ABNORMAL_STATUS     Close file unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no need to close it.
///
int FileClose_NoWriteBack(STREAM * handle)
{
    DRIVE *drv;


    if (handle == NULL)
        return FILE_NOT_FOUND;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        if ((handle->ramFile_buf != NULL) && (handle->ramFile_bufSize > 0))
            ext_mem_free(handle->ramFile_buf);

        MpMemSet((BYTE *) handle, 0, sizeof(STREAM)); /* clear all fields for clearing "RamFile" flag and others */

        IntDisable();
        HandleCounter--;
        IntEnable();
        return FS_SUCCEED;
    }
#endif

    drv = (DRIVE *) (handle->Drv);
    if (drv == NULL)
        return FILE_NOT_FOUND; // this file has been closed, except "RamFile"

    MpMemSet((BYTE *) handle, 0, sizeof(STREAM)); /* clear all fields */

    IntDisable();
    handle->Drv = 0; // clear drive field mean clear handle
    HandleCounter--;
    IntEnable();

    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Force to sync (write back) changed file info (ex: changed file size) to storage drive.
///           
///
///@param   handle      The handle of the file to sync (write back) changed file info to storage drive.
///
///@retval  FS_SUCCEED          Sync file info successfully. \n\n
///@retval  ABNORMAL_STATUS     Sync file info unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no need to sync it.
///
///@remark    The internal syncing (or writing back) behavior of this function is similar with those in
///           FileClose(). But FileClose() is to "close" the file, and FileForceSync() always leaves the file
//            retaining "open".
///
int FileForceSync(STREAM * handle)
{
    DRIVE *drv;
    FDB *node;
    CHAIN *chain;


    if (handle == NULL)
        return FILE_NOT_FOUND;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
        return FS_SUCCEED;  // no need to do anything for "RamFile" type files
#endif

    drv = (DRIVE *) (handle->Drv);
    if (drv == NULL)
        return FILE_NOT_FOUND; // this file has been closed, except "RamFile"

    if (handle->Flag.SizeChanged)
    {
        /* make sure drive's Dir cache buffer covers the file FDB */
        if (FileDirCaching(handle) != FS_SUCCEED)
        {
            MP_ALERT("%s: -E- FileDirCaching() failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        node = (FDB *) drv->DirCacheBuffer + handle->FdbOffsetInDirCache;
        chain = (CHAIN *) & handle->Chain;

        if (drv->Flag.FsType != FS_TYPE_exFAT)
        {
            SaveAlien32(&node->Size, chain->Size);
        }
#if EXFAT_ENABLE
        else
        {
            TYPE_exFAT_Stream_Extension_DirEntry *exfat_node;
            BYTE *Hi32_of_64, *Lo32_of_64;
            DWORD dwSize = 0;

            /* current drv->Node should be a "Stream Extension DirectoryEntry" of exFAT */
            exfat_node = (TYPE_exFAT_Stream_Extension_DirEntry *) (drv->Node);
            if (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT)
            {
                MP_ALERT("%s: Error! Current node should be a (Stream Extension DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
                return ABNORMAL_STATUS;
            }

            Lo32_of_64 = (BYTE *) &exfat_node->dataLength; /* due to little-endian */
            Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
            if ((U64) chain->Size <= (U64) 0x00000000FFFFFFFF)
            {
                dwSize = (DWORD) chain->Size;
                SaveAlien32(Hi32_of_64, 0);
                SaveAlien32(Lo32_of_64, dwSize);
            }
            else
            {
                dwSize = (DWORD) (((U64) chain->Size & 0xFFFFFFFF00000000) >> 32);
                SaveAlien32(Hi32_of_64, dwSize);

                dwSize = (DWORD) ((U64) chain->Size & 0x00000000FFFFFFFF);
                SaveAlien32(Lo32_of_64, dwSize);
            }
        }
#endif
        drv->Flag.DirCacheChanged = 1;

        // here, write back changed DirCache and FatCache and free cluster info together
        if (DriveRefresh(drv) != FS_SUCCEED)
        {
            MP_ALERT("%s: DriveRefresh() failed.", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        handle->Flag.SizeChanged = 0;
    }

    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Free all the allocated clusters of a file, but reserve its start cluster. And set its file size in its
///         FDB (Directory Entry) node to zero. This function is called for file content overwrite purpose.
///
///@param   handle      The handle of the file to release its allocated clusters for overwrite new content.
///
///@retval  FS_SUCCEED          Process successfully. \n\n
///@retval  ABNORMAL_STATUS     Process unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no need to process it.
///
int File_ReleaseSpace_For_OverwriteContent(STREAM * handle)
{
    DRIVE *drv;
    FDB *node;
    CHAIN *chain;


    if (handle == NULL)
        return FILE_NOT_FOUND;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        MP_ALERT("%s: this operation is not supported for \"RamFile\" type files !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
#endif

    drv = (DRIVE *) (handle->Drv);
    if (drv == NULL)
        return FILE_NOT_FOUND; // this file has been closed

    chain = (CHAIN *) & handle->Chain;
    if (ChainFreeToReserveStartCluster(drv, chain) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    /* set chain size to 0 for file content overwrite purpose */
    ChainInit(chain, chain->Start, 0);

    /* make sure drive's Dir cache buffer covers the file FDB */
    if (FileDirCaching(handle) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E- FileDirCaching() failed !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    node = (FDB *) drv->DirCacheBuffer + handle->FdbOffsetInDirCache;

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        SaveAlien32(&node->Size, 0);  /* force to reset file size in FDB node to 0 */
    }
#if EXFAT_ENABLE
    else
    {
        TYPE_exFAT_Stream_Extension_DirEntry *exfat_node;
        BYTE *Hi32_of_64, *Lo32_of_64;

        /* current drv->Node should be a "Stream Extension DirectoryEntry" of exFAT */
        exfat_node = (TYPE_exFAT_Stream_Extension_DirEntry *) (drv->Node);
        if (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT)
        {
            MP_ALERT("%s: Error! Current node should be a (Stream Extension DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
            return ABNORMAL_STATUS;
        }

        Lo32_of_64 = (BYTE *) &exfat_node->dataLength; /* due to little-endian */
        Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
        SaveAlien32(Hi32_of_64, 0);  /* force to reset file size in FDB node to 0 */
        SaveAlien32(Lo32_of_64, 0);  /* force to reset file size in FDB node to 0 */
    }
#endif
    drv->Flag.DirCacheChanged = 1;

    /* here, write back changed DirCache and FatCache and free cluster info together */
    if (DriveRefresh(drv) != FS_SUCCEED)
    {
        return ABNORMAL_STATUS;
    }

    return FS_SUCCEED;
}



/* Note that the output filename in bLongNameBuf[] buffer is actually an ASCII string of the main part of whole final filename,
 * not the whole filename !!
 * For whole filename, you can simply merge the output bLongNameBuf[] string with a '.' and the extension name string manually.
 * If no extension name, then only the output bLongNameBuf[] string (without adding a '.') is the whole final filename.
 * To get UTF-16 filename string, please call mpx_UtilUtf8ToUnicodeU16() to convert it to UTF-16 string manually.
 *
 * Also note the return value is the characters count in the final filename excluding the '.' between main part
 * and extension part of whole filename. Such return value is for simple check for 8.3 short name (max 11 characters
 * excluding the '.')
 */
DWORD BuildNewAsciiFilename(DRIVE * drv, BYTE * name, BYTE * extension, BYTE *bLongNameBuf, BOOL blLossyFlag)
{
    DWORD dwNumericTail;
    WORD *wTmpName;
    DWORD dwNameLength, i, j;
    BYTE NameL, ExtenL;
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return 0;
    }

    if (name == NULL)
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return 0;
    }

    NameL = StringLength08(name);
    ExtenL = StringLength08(extension);

    wTmpName = (WORD *) ker_mem_malloc(MAX_L_NAME_LENG * 2, TaskGetId());
    if (wTmpName == NULL)
    {
        MP_ALERT("%s: -E malloc fail", __FUNCTION__);
        return 0;
    }

    MpMemSet(wTmpName, 0, MAX_L_NAME_LENG * 2);
    dwNameLength = NameL + ExtenL;

    if ((dwNameLength > 11) || (blLossyFlag == 1) || (ExtenL > 3) || (NameL > 8)) //long file name => change to unicode
    {
        for (i=0; i < NameL; i++)
            wTmpName[i] = name[i];

        if (ExtenL)
        {
            wTmpName[i++] = 0x002E; //"."
            for ( ; i < (dwNameLength + 1); i++) //add 1 for considering '.'
                wTmpName[i] = extension[i - 1 - NameL];

            ret = FileSearchLN(drv, wTmpName, dwNameLength + 1, E_BOTH_FILE_AND_DIR_TYPE); //add 1 for considering '.'
        }
        else
            ret = FileSearchLN(drv, wTmpName, dwNameLength, E_BOTH_FILE_AND_DIR_TYPE);

        if (ret == END_OF_DIR) //no same file name found
        {
            for (j=0; j < i; j++)
                bLongNameBuf[j] = name[j];

            ker_mem_free(wTmpName);
            return dwNameLength;
        }

        // same file name found, so add _xxx to file name
        // xxx is between 1 and 999999

        for (dwNumericTail = 1; dwNumericTail < 999999; dwNumericTail++)
        {
            i = NameL;
            wTmpName[i] = 0x005F; //"_"
            j = FillUniNumber(&wTmpName[i+1], dwNumericTail) + 1;
            i += j;

            if (ExtenL)
            {
                wTmpName[i++] = 0x002E; //"."
                for ( ; i < (dwNameLength + j + 1); i++) //add 1 for considering '.'
                    wTmpName[i] = extension[i - j - NameL - 1];

                ret = FileSearchLN(drv, wTmpName, i, E_BOTH_FILE_AND_DIR_TYPE);
                if (ret == END_OF_DIR) //no same file name found
                {
                    for (j=0; j < (i - ExtenL - 1); j++)
                        bLongNameBuf[j] = wTmpName[j];

                    ker_mem_free(wTmpName);
                    return (i - 1);
                }
            }
            else
            {
                ret = FileSearchLN(drv, wTmpName, i, E_BOTH_FILE_AND_DIR_TYPE);
                if (ret == END_OF_DIR) //no same file name found
                {
                    for (j=0; j < i; j++)
                        bLongNameBuf[j] = wTmpName[j];

                    ker_mem_free(wTmpName);
                    return (i);
                }
            }
        }
    }
    else //short file name
    {
        ret = FileSearch(drv, name, extension, E_BOTH_FILE_AND_DIR_TYPE);
        if (ret == END_OF_DIR) //no same file name found
        {
            StringCopy08(bLongNameBuf, name); /* only main part of whole filename */

            ker_mem_free(wTmpName);
            return dwNameLength;
        }

        /* special process for original 8.3 short ASCII filename adding "_xxx" and becomes long filename case */
        BYTE *bName = (BYTE *) ker_mem_malloc(MAX_L_NAME_LENG, TaskGetId());
        if (bName == NULL)
        {
            MP_ALERT("%s: -E malloc fail", __FUNCTION__);
            ker_mem_free(wTmpName);
            return 0;
        }

        // same file name found, so add _xxx to file name
        // xxx is between 1 and 999999

        for (dwNumericTail = 1; dwNumericTail < 999999; dwNumericTail++)
        {
            WORD bName_len = 0;

            MpMemSet(bName, 0, MAX_L_NAME_LENG);
            for (i=0; i < NameL; i++)
                bName[i] = name[i];

            bName[i] = 0x5F; //"_"
            i += (FillNumber(&bName[i+1], dwNumericTail) + 1);

            bName_len = i;

            if (bName_len <= 8) //short file name
            {
                bName[bName_len] = '\0';

                ret = FileSearch(drv, bName, extension, E_BOTH_FILE_AND_DIR_TYPE);
                if (ret == END_OF_DIR) //no same file name found
                {
                    StringCopy08(bLongNameBuf, bName); /* only main part of whole filename */

                    ker_mem_free(wTmpName);
                    ker_mem_free(bName);
                    return (bName_len + ExtenL);
                }
            }
            else  /* become long file name => change to unicode */
            {
                for (i=0; i < bName_len; i++)
                    wTmpName[i] = bName[i];

                if (ExtenL)
                {
                    wTmpName[bName_len] = 0x002E; //"."
                    for (i=0; i < ExtenL; i++)
                        wTmpName[bName_len + 1 + i] = extension[i]; //add 1 for considering '.'

                    dwNameLength = bName_len + ExtenL;
                    ret = FileSearchLN(drv, wTmpName, dwNameLength + 1, E_BOTH_FILE_AND_DIR_TYPE); //add 1 for considering '.'
                }
                else
                {
                    dwNameLength = bName_len;
                    ret = FileSearchLN(drv, wTmpName, dwNameLength, E_BOTH_FILE_AND_DIR_TYPE);
                }

                if (ret == END_OF_DIR) // no same file name found
                {
                    for (j=0; j < bName_len; j++)
                        bLongNameBuf[j] = wTmpName[j];

                    ker_mem_free(wTmpName);
                    ker_mem_free(bName);
                    return dwNameLength;
                }
            }
        }
        ker_mem_free(bName);
    }

    ker_mem_free(wTmpName); //should not reach here. just for safety.
}



///
///@ingroup FILE
///@brief   Create a file with specified filename on the specified drive.
///
///@param   drv           The drive to create new file. \n\n
///@param   name          The pointer of new file name primary part, which is an ASCII string. \n\n
///@param   extension     The pointer of new file name extension part, which is an ASCII string.
///
///@retval  FS_SUCCEED            Create file successfully. \n\n
///@retval  ABNORMAL_STATUS       Create file unsuccessfully. \n\n
///@retval  DISK_FULL             Create file unsuccessfully due to no enough free space on the target drive. \n\n
///@retval  DISK_READ_ONLY        Create file unsuccessfully due to the drive is Read-Only. \n\n
///@retval  OVER_ROOT_BOUNDARY    Create file unsuccessfully due to directory chain extending failed at root directory of FAT12/16. \n\n
///@retval  INVALID_DRIVE         Invalid drive.
///
///@remark  The function call only creates a new file on the specified drive.\n
///         To access the created file, must open it by calling FileOpen() first.
///
int CreateFile(DRIVE * drv, BYTE * name, BYTE * extension)
{
    FDB *sNode;
    DWORD dwAddr, i, j;
    ST_SYSTEM_TIME  sys_time;
    WORD file_date, file_time;
    LONG_NAME *sLongName;
    DWORD dwNameLength, dwFdbCount, dwRemainFdbCount, dwNumericTail, dwNumericTailLength;
    BYTE bTempBuf1[32]; //enough buffer for 13 UTF-16 words in a long name FDB. One more word in buffer for easy string processing
    BYTE bChkSum;
    BYTE bShortNameBuf[12], bExt[4]; //for 8.3 format short name. One more byte in each buffer for easy string processing
    BYTE *bLongNameBuf, *bTempBuf;
    BYTE *pbTempBufPtr;
    BOOL blLossyFlag;
    BYTE NameL, ExtenL;
    CHAIN *dir;
    DWORD ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if ((name == NULL) || (strcmp(name, "") == 0))
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* check Read-Only flag by underlying Mcard layer info => physical lock latch or Mcard layer lock */
    if (SystemGetFlagReadOnly(drv->DrvIndex))
    {
        MP_ALERT("%s: -E- The drive is Read-Only (in Mcard layer) !", __FUNCTION__);
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

    /* Use semaphore to prevent free clusters from being cross-linked by multiple tasks, which calls FileWrite() or CreateFile() or MakeDir() or ... other write APIs */
    SemaphoreWait(FILE_WRITE_SEMA_ID);

#if FS_REENTRANT_API
    BOOL f_Is_ext_working_drv;
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);

    /* check whether if this drive handle is an external working drive copy */
    f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;
#endif

    NameL = StringLength08(name);
    ExtenL = StringLength08(extension);

    bLongNameBuf = (BYTE *) ker_mem_malloc(MAX_L_NAME_LENG, TaskGetId());
    if (bLongNameBuf == NULL)
    {
        MP_ALERT("%s: -E malloc fail", __FUNCTION__);
        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }
    bTempBuf = (BYTE *) ker_mem_malloc(MAX_L_NAME_LENG, TaskGetId());
    if (bTempBuf == NULL)
    {
        MP_ALERT("%s: -E malloc fail", __FUNCTION__);
        ker_mem_free(bLongNameBuf);
        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

 //Do not go back to Root Dir, and then files can be created in folders
 #if 0
    //note 1: DirReset() will reset to Root directory of the drive !
    //note 2: No need to do DirFirst() or FirstNode() here because they will be invoked within FileSearch()/FileSearchLN() later.
    if (DirReset(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E- DirReset() back to beginning of root directory failed !", __FUNCTION__);
        ker_mem_free(bLongNameBuf);
        ker_mem_free(bTempBuf);
        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }
 #endif

#if FS_REENTRANT_API
    Set_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    dwAddr = DriveNewClusGet(drv);
    if (dwAddr == 0xffffffff)
    {
        MP_ALERT("%s: -E DriveNewClusGet() failed to get free cluster !", __FUNCTION__);
        ker_mem_free(bLongNameBuf);
        ker_mem_free(bTempBuf);

#if FS_REENTRANT_API
        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return DISK_FULL;
    }

    if (drv->FatWrite(drv, dwAddr, 0xffffffff) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E drv->FatWrite() failed to write FAT entry value !", __FUNCTION__);
        ker_mem_free(bLongNameBuf);
        ker_mem_free(bTempBuf);

#if FS_REENTRANT_API
        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

#if FS_REENTRANT_API
    /* FAT cluster allocation is changed => force to write FAT cache and FSInfo to device right away !
     * note: this is important to sync FAT and free cluster info for concurrent multiple File I/O API invoking case.
     */
    if (DriveRefresh(drv) != FS_SUCCEED)
    {
        ker_mem_free(bLongNameBuf);
        ker_mem_free(bTempBuf);

        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

    Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    blLossyFlag = 0;
    for (i = 0; i < 8 && name[i] != 0; i++)
    {
        if ((strchr(ReplaceGlyph, name[i])) || (name[i] == 0x20))
        {
            blLossyFlag = 1;
            break;
        }
    }

    if ((ExtenL > 3) || (NameL > 8))
        blLossyFlag = 1;
    else if ((ExtenL > 0) && (ExtenL <= 3))
    {
        for (i = 0; i < 3 && blLossyFlag == 0 && extension[i] != 0; i++)
        {
            if ((strchr(ReplaceGlyph, extension[i])) || (extension[i] == 0x20))
            {
                blLossyFlag = 1;
                break;
            }
        }
    }

    MpMemSet(bLongNameBuf, 0, MAX_L_NAME_LENG);
    dwNameLength = BuildNewAsciiFilename(drv, name, extension, (BYTE *)bLongNameBuf, blLossyFlag);

    if ((dwNameLength > 11) || (blLossyFlag == 1) || ((ExtenL == 0) && (dwNameLength > 8)))
    {
        if (ExtenL > 0)
            dwNameLength += 1;

        dwFdbCount = (dwNameLength / 13);
        if (dwNameLength % 13)
            dwFdbCount += 1;

        //Check whether if it still has enough space in root directory. Only FAT12/16 have Root directory region.
        if ((drv->DirStackPoint == 0) && ((drv->Flag.FsType == FS_TYPE_FAT12) || (drv->Flag.FsType == FS_TYPE_FAT16)))
        {
            dwRemainFdbCount = 256; /* here, 256 == 8KB/sizeof(FDB) */
            //note: for FAT12/16, drv->RootStart is lba of first sector of root directory
            dwRemainFdbCount -= ((1 << (drv->bSectorExp - 5)) * ((DWORD) drv->DirCachePoint - drv->RootStart));
            DirLast(drv);
            dwRemainFdbCount -= (((((DWORD) drv->Node) - ((DWORD) drv->DirCacheBuffer)) / 32) + 1);
            if (dwRemainFdbCount < (dwFdbCount + 1))
            {
                MP_ALERT("%s: -E No enough space in root directory of this FAT12/FAT16 drive !", __FUNCTION__);
                MP_ALERT("%s: dwRemainFdbCount (=%d) < (dwFdbCount(=%d) + 1)", __FUNCTION__, dwRemainFdbCount, dwFdbCount);
                ker_mem_free(bLongNameBuf);
                ker_mem_free(bTempBuf);

#if FS_REENTRANT_API
                Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
                if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                    Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

                SemaphoreRelease(FILE_WRITE_SEMA_ID);
                return ABNORMAL_STATUS;
            }
        }

        // generate short name
        MpMemSet(bShortNameBuf, 0, sizeof(bShortNameBuf)); /* note: Don't use 0x20 here, to avoid string terminator issue */
        bShortNameBuf[11] = 0x00; /* need a null terminator, otherwise UpperCase08() will not stop and overwrite other memory block */

        for (i=0, j=0; (j < 6) && (((BYTE *) bLongNameBuf)[i] != 0); i++)
        {
            if (strchr(ReplaceGlyph, ((BYTE *) bLongNameBuf)[i]))
                bShortNameBuf[j] = '_';
            else
            {
                bShortNameBuf[j] = ((BYTE *) bLongNameBuf)[i];
                if (((BYTE *) bLongNameBuf)[i] == 0x20)
                    j--;
            }
            j++;
        }
        bShortNameBuf[j] = 0x00;
        UpperCase08(bShortNameBuf);

        MpMemSet(bExt, 0, 4);
        if (ExtenL > 0)
        {
            if (ExtenL >= 3)
                MpMemCopy(bExt, extension, 3);
            else
                MpMemCopy(bExt, extension, ExtenL);

            UpperCase08(bExt);
        }

        dwNumericTail = 1;
        dwNumericTailLength = 0;
        j = StringLength08(bShortNameBuf);	// current length
        while (dwNumericTailLength <= 6)	//numeric-tail range "~1" to "~999999"
        {
            dwNumericTailLength = mp_sprintf(bTempBuf, "%d", dwNumericTail);	// tail length

            if ((8 - j) > dwNumericTailLength)
            {
                bShortNameBuf[j] = '~';
                StringNCopy08(&bShortNameBuf[j + 1], bTempBuf, dwNumericTailLength);
            }
            else
            {
                bShortNameBuf[(8 - (dwNumericTailLength + 1))] = '~';
                StringNCopy08(&bShortNameBuf[(8 - dwNumericTailLength)], bTempBuf, dwNumericTailLength);
            }

            if (FileSearch(drv, bShortNameBuf, bExt, E_BOTH_FILE_AND_DIR_TYPE) != FS_SUCCEED)
            {
                if (StringLength08(bExt) > 0)
                    StringNCopy08(&bShortNameBuf[8], bExt, 3);
                else
                    MpMemSet(&bShortNameBuf[8], 0x20, 3);

                bShortNameBuf[11] = 0x00;
                break;
            }
            dwNumericTail++;
        }

        // generate checksum
        bChkSum = 0;
        for (i = 0; i < 11; i++)
        {
            if (bShortNameBuf[i] == 0x00)
                bShortNameBuf[i] = 0x20; /* 8.3 short name field in the FDB should not contain null terminators, it uses spaces for padding */

            //NOTE: The operation is an unsigned char rotate right
            bChkSum = ((bChkSum & 1) ? 0x80 : 0) + (bChkSum >> 1) + bShortNameBuf[i];
        }

        // prepare filename to be written to long name FDB nodes
        pbTempBufPtr = StringCopy08(bTempBuf, (BYTE *) bLongNameBuf);
        if (ExtenL > 0)
        {
            *pbTempBufPtr = '.';
            pbTempBufPtr = StringCopy08((pbTempBufPtr + 1), extension);
        }
        // now, buffer 'bTempBuf' contains the final long filename !

        CheckDeletedCount(dwFdbCount+1);

        dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
        if (dir->Size - dir->Point  < (dwFdbCount + 1) * sizeof(FDB))
        {
            DWORD curr_cluster, curr_sector;
            FDB *fdb_ptr;

            MP_DEBUG("%s: space in the directory not enough => ChainExtending()...", __FUNCTION__);
            ret = ChainExtending(drv, dir);
            if (ret != FS_SUCCEED)
            {
                MP_ALERT("%s: -E ChainExtending() failed! => return ret=0x%x;", __FUNCTION__, ret);
                ker_mem_free(bLongNameBuf);
                ker_mem_free(bTempBuf);

#if FS_REENTRANT_API
                Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
                if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                    Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

                SemaphoreRelease(FILE_WRITE_SEMA_ID);
                return ret;
            }

            //note: For a long-name file or directory, its FDB nodes must be contiguous. Here, "contiguous" can be spanning cluster boundary !
            //      So, we don't need to check whether if contiguous FDBs span cluster boundary.
        }

    #if FS_REENTRANT_API
        Set_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Set_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
    #endif

        // get long-name FDB nodes first
        for (i = dwFdbCount; i != 0; i--)
        {
            // get a new node location and move on it
            if (GetNewNode(drv) != FS_SUCCEED)
            {
                MP_DEBUG("%s: -I- GetNewNode() failed to get free FDB node => GetDeletedNode()...", __FUNCTION__);
                if (GetDeletedNode(drv) != FS_SUCCEED)
                {
                    MP_ALERT("%s: -E failed to get free FDB node !", __FUNCTION__);
                    ker_mem_free(bLongNameBuf);
                    ker_mem_free(bTempBuf);

    #if FS_REENTRANT_API
                    Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
                    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                       Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
    #endif

                    SemaphoreRelease(FILE_WRITE_SEMA_ID);
                    return ABNORMAL_STATUS;
                }
            }
            sLongName = (LONG_NAME *) drv->Node;

            // generate long name FDB
            sLongName->Attribute1 = FDB_LONG_NAME;
            sLongName->Attribute2 = 0x00;
            sLongName->CheckSum = bChkSum;
            sLongName->res[0] = sLongName->res[1] = 0x00;

            if (i != dwFdbCount)
            {
                sLongName->Number = i;
                (BYTE *) pbTempBufPtr -= 13;
                StringNCopy0816((WORD *) bTempBuf1, pbTempBufPtr, 13);
            }
            else
            {
                sLongName->Number = (0x40 | i);
                MpMemSet(bTempBuf1, 0xff, sizeof(bTempBuf1));
                if (i == 1) // totally only one single long name FDB
                {
                    (BYTE *) pbTempBufPtr -= StringLength08(bTempBuf);
                }
                else // this is the last long name FDB
                {
                    pbTempBufPtr = ((BYTE *) bTempBuf + ((dwFdbCount - 1) * 13));
                }
                StringCopy0816((WORD *) bTempBuf1, pbTempBufPtr);
            }

            LoadAlienArray16((WORD *) bTempBuf1, bTempBuf1, 26);

            MpMemCopy(sLongName->Name0, bTempBuf1, 10);
            MpMemCopy(sLongName->Name1, bTempBuf1 + 10, 12);
            MpMemCopy(sLongName->Name2, bTempBuf1 + 22, 4);

/* note: This block is necessary ! For long filename, contiguous FDB nodes may cross cluster boundary !
 *       We compare DirCachePoint value to check covered Dir content (FDBs) range before GetNewNode() allocating free FDBs.
 */
#if 1
            drv->Flag.DirCacheChanged = 1;

    #if FS_REENTRANT_API
            /* FDB nodes are allocated now => force to write Dir cache to device right away !
             * note: this is important to sync Dir cache and FDBs info for concurrent multiple file/folder creation API invoking case.
             */
            if (DirCaching(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: -E DirCaching() failed !! => return ABNORMAL_STATUS;", __FUNCTION__);
                ker_mem_free(bLongNameBuf);
                ker_mem_free(bTempBuf);

                Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
                if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                   Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

                SemaphoreRelease(FILE_WRITE_SEMA_ID);
                return ABNORMAL_STATUS;
            }
    #endif
#endif
        }
    }
    else //short name
    {
        MpMemSet(bShortNameBuf, 0x20, sizeof(bShortNameBuf));
        bShortNameBuf[11] = 0x00; /* need a null terminator, otherwise UpperCase08() will not stop and overwrite other memory block */

        for (i = 0; i < 8 && ((BYTE *) bLongNameBuf)[i] != 0; i++)
            bShortNameBuf[i] = ((BYTE *) bLongNameBuf)[i];

        for (i = 0; i < 3 && extension[i] != 0; i++)
            bShortNameBuf[i + 8] = extension[i];

        UpperCase08(bShortNameBuf);
    }

    // get a new node location and move on it
    if (GetNewNode(drv) != FS_SUCCEED)
    {
        MP_DEBUG("%s: -I- GetNewNode() failed to get free FDB node => GetDeletedNode()...", __FUNCTION__);
        if (GetDeletedNode(drv) != FS_SUCCEED)
        {
            MP_ALERT("%s: -E failed to get free FDB node !", __FUNCTION__);
            ker_mem_free(bLongNameBuf);
            ker_mem_free(bTempBuf);

    #if FS_REENTRANT_API
            Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
               Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
    #endif

            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return ABNORMAL_STATUS;
        }
    }
    sNode = (FDB *) (drv->Node);
    /* note: add pointer check for safety ! Because MpMemSet() may cause "break 100" when memory buffer was already released first by Card Out, for example. */
    if ((BYTE *) sNode != NULL)
        MpMemSet(sNode, 0, sizeof(FDB));

    for (i = 0; i < 11; i++)
        sNode->Name[i] = bShortNameBuf[i];

    if (sNode->Name[0] == 0xE5)
        sNode->Name[0] = 0x05;

    sNode->Attribute = FDB_ARCHIVE;

    SaveAlien16(&sNode->StartLow, dwAddr);
    dwAddr >>= 16;
    SaveAlien16(&sNode->StartHigh, dwAddr);
    SaveAlien32(&sNode->Size, 0);

    /* fill in the create date/time and modified date */
    SystemTimeGet(&sys_time);
    file_date = FileSetDate_for_FATxx(sys_time.u16Year, sys_time.u08Month, sys_time.u08Day);
    file_time = FileSetTime_for_FATxx(sys_time.u08Hour, sys_time.u08Minute, sys_time.u08Second);
    sNode->CreateTime = file_time;
    sNode->CreateDate = file_date;
    sNode->AccessDate = file_date;
    sNode->ModifyTime = file_time;
    sNode->ModifyDate = file_date;

    drv->Flag.DirCacheChanged = 1;

    /* FDB nodes are allocated now => force to write Dir cache to device right away !
     * note: this is important to sync Dir cache and FDBs info for concurrent multiple file/folder creation API invoking case.
     */
    if (DirCaching(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E DirCaching() failed !! => return ABNORMAL_STATUS;", __FUNCTION__);
        ker_mem_free(bLongNameBuf);
        ker_mem_free(bTempBuf);

    #if FS_REENTRANT_API
        Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
    #endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

#if FS_REENTRANT_API
    Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    ker_mem_free(bLongNameBuf);
    ker_mem_free(bTempBuf);
    SemaphoreRelease(FILE_WRITE_SEMA_ID);
    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Create a file with specified UTF-16 Unicode filename on the specified drive. \n
///         This function is the UTF-16 Unicode version of the CreateFile() function.
///
///@param   drv           The drive to create new file. \n\n
///@param   name          The pointer of new file name, which is an UTF-16 Unicode string with 0x0000 null terminator. \n\n
///
///@retval  FS_SUCCEED            Create file successfully. \n\n
///@retval  ABNORMAL_STATUS       Create file unsuccessfully. \n\n
///@retval  DISK_FULL             Create file unsuccessfully due to no enough free space on the target drive. \n\n
///@retval  DISK_READ_ONLY        Create file unsuccessfully due to the drive is Read-Only. \n\n
///@retval  OVER_ROOT_BOUNDARY    Create file unsuccessfully due to directory chain extending failed at root directory of FAT12/16. \n\n
///@retval  INVALID_DRIVE         Invalid drive.
///
///@remark  The function call only creates a new file on the specified drive.\n
///         To access the created file, must open it by calling FileOpen() first.
///
int CreateFile_UTF16(DRIVE * drv, const WORD * name)
{
    FDB *sNode;
    DWORD dwAddr, i, j;
    CHAIN *dir;
    DWORD ret;
    ST_SYSTEM_TIME  sys_time;
    WORD file_date, file_time;
    LONG_NAME *sLongName;
    DWORD dwFdbCount, dwRemainFdbCount, dwNumericTail, dwNumericTailLength;
    BYTE bTempBuf1[32]; //enough buffer for 13 UTF-16 words in a long name FDB. One more word in buffer for easy string processing
    BYTE bChkSum;
    BYTE bShortNameBuf[12]; //for 8.3 format short name. One more byte in each buffer for easy string processing
    BYTE bNumericTailBuf[10]; //enough for numeric-tail string range "~1" to "~999999"
    WORD *wLongNameBuf, *wTempNameBuf; //buffers for UTF-16 Unicode long name processing
    WORD *pwTempBufPtr, *ori_ext_name;
    BYTE ori_filenameL = 0, ori_primary_nameL = 0, ori_ext_nameL = 0, final_nameL = 0;
 #define EXT_NAME_LEN   4
    BYTE bExt[EXT_NAME_LEN], bTempExt[EXT_NAME_LEN], ExtenL; //file extension name with ASCII coding for 8.3 short name usage
    WORD ext_word[EXT_NAME_LEN]; //file extension name with Unicode (UTF-16) coding
    WORD *tmp_word = NULL;
    BYTE *tmp_byte;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if ((name == NULL) || ((WORD) *name == 0x0000))
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* check Read-Only flag by underlying Mcard layer info => physical lock latch or Mcard layer lock */
    if (SystemGetFlagReadOnly(drv->DrvIndex))
    {
        MP_ALERT("%s: -E- The drive is Read-Only (in Mcard layer) !", __FUNCTION__);
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

    /* Use semaphore to prevent free clusters from being cross-linked by multiple tasks, which calls FileWrite() or CreateFile() or MakeDir() or ... other write APIs */
    SemaphoreWait(FILE_WRITE_SEMA_ID);

#if FS_REENTRANT_API
    BOOL f_Is_ext_working_drv;
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);

    /* check whether if this drive handle is an external working drive copy */
    f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;
#endif

    wLongNameBuf = (WORD *) ker_mem_malloc((MAX_L_NAME_LENG * 2), TaskGetId());
    if (wLongNameBuf == NULL)
    {
        MP_ALERT("%s: -E malloc fail", __FUNCTION__);
        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }
    wTempNameBuf = (WORD *) ker_mem_malloc((MAX_L_NAME_LENG * 2), TaskGetId());
    if (wTempNameBuf == NULL)
    {
        MP_ALERT("%s: -E malloc fail", __FUNCTION__);
        ker_mem_free(wLongNameBuf);
        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

    ori_filenameL = StringLength16((WORD *) name);

    MpMemSet(ext_word, 0, EXT_NAME_LEN * 2);
    MpMemSet(bExt, 0, EXT_NAME_LEN);
    MpMemSet(bTempExt, 0, EXT_NAME_LEN);

    MpMemSet(wTempNameBuf, 0, MAX_L_NAME_LENG * 2);
    MpMemCopy(wTempNameBuf, name, (ori_filenameL + 1) * 2); //including tailing 0x0000 terminator

    // scan the rightest '.' in input filename for extension part of the UTF-16 filename
    tmp_byte = (BYTE *) wTempNameBuf;
    for (i=0; i < ori_filenameL; i++)
    {
        if ((*tmp_byte == 0x00) && (*(tmp_byte + 1) == 0x2E))  // '.'
            tmp_word = (WORD *) tmp_byte;

        tmp_byte += 2; //next word
    }

    if (tmp_word != NULL)
    {
        *tmp_word = 0x0000; //null terminator for the primary name of whole filename
        tmp_word++; //skip the '.' word
        ori_ext_nameL = StringLength16(tmp_word);

        for (i=0; ((i < 3) && (i < ori_ext_nameL)); i++)
            ext_word[i] = ((WORD *) tmp_word)[i];
    }

    ori_primary_nameL = StringLength16(wTempNameBuf);
    ori_ext_name = (ori_ext_nameL > 0)? ((WORD *) ((WORD *) name + ori_primary_nameL + 1)) : NULL;

    // convert UTF-16 extension name to ASCII for 8.3 format short name to retain file extension type (ex: "JPG") in short name FDB
    mpx_UtilU16ToU08(bTempExt, ext_word);
    ExtenL = StringLength08(bTempExt);

 //Do not go back to Root Dir, and then files can be created in folders
 #if 0
    //note 1: DirReset() will reset to Root directory of the drive !
    //note 2: No need to do DirFirst() or FirstNode() here because they will be invoked within FileSearch()/FileSearchLN() later.
    if (DirReset(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E- DirReset() back to beginning of root directory failed !", __FUNCTION__);
        ker_mem_free(wLongNameBuf);
        ker_mem_free(wTempNameBuf);
        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }
 #endif

#if FS_REENTRANT_API
    Set_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    dwAddr = DriveNewClusGet(drv);
    if (dwAddr == 0xffffffff)
    {
        MP_ALERT("%s: -E DriveNewClusGet() failed to get free cluster !", __FUNCTION__);
        ker_mem_free(wLongNameBuf);
        ker_mem_free(wTempNameBuf);

#if FS_REENTRANT_API
        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return DISK_FULL;
    }

    if (drv->FatWrite(drv, dwAddr, 0xffffffff) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E drv->FatWrite() failed to write FAT entry value !", __FUNCTION__);
        ker_mem_free(wLongNameBuf);
        ker_mem_free(wTempNameBuf);

#if FS_REENTRANT_API
        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

#if FS_REENTRANT_API
    /* FAT cluster allocation is changed => force to write FAT cache and FSInfo to device right away !
     * note: this is important to sync FAT and free cluster info for concurrent multiple File I/O API invoking case.
     */
    if (DriveRefresh(drv) != FS_SUCCEED)
    {
        ker_mem_free(wLongNameBuf);
        ker_mem_free(wTempNameBuf);

        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

    Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    final_nameL = ori_filenameL;
    ret = FileSearchLN(drv, (WORD *) name, final_nameL, E_BOTH_FILE_AND_DIR_TYPE);
    if (ret != END_OF_DIR)
    {
        // same file name found, so add _xxx to file name
        // xxx is between 1 and 999999

        for (dwNumericTail = 1; dwNumericTail < 999999; dwNumericTail++)
        {
            MpMemSet(wLongNameBuf, 0, (MAX_L_NAME_LENG * 2));
            MpMemCopy((BYTE *) wLongNameBuf, (BYTE *) wTempNameBuf, (ori_primary_nameL + 1) * 2); //including tailing 0x0000 terminator

            i = ori_primary_nameL;
            wLongNameBuf[i] = 0x005F; //"_"
            j = FillUniNumber(&wLongNameBuf[i+1], dwNumericTail) + 1;
            i += j;

            if (ori_ext_nameL)
            {
                wLongNameBuf[i++] = 0x002E; //"."

                for ( ; i < (ori_filenameL + j); i++) // the original extension name part after the rightest '.'
                    wLongNameBuf[i] = ((WORD *) ori_ext_name)[i - j - ori_primary_nameL - 1];

                wLongNameBuf[i] = 0x0000; // null terminator
            }

            final_nameL = ori_filenameL + j;
            ret = FileSearchLN(drv, wLongNameBuf, final_nameL, E_BOTH_FILE_AND_DIR_TYPE);
            if (ret != END_OF_DIR) // same file name found
                continue;
            else // no same file name found
                break;
        }
    }
    else // no same file name found
    {
        MpMemSet(wLongNameBuf, 0, (MAX_L_NAME_LENG * 2));
        MpMemCopy((BYTE *) wLongNameBuf, (BYTE *) name, (final_nameL + 1) * 2); //including tailing 0x0000 terminator
    }

    // now, we got the final UTF-16 Unicode filename to be applied
    dwFdbCount = (final_nameL / 13);
    if (final_nameL % 13)
        dwFdbCount += 1;

    //Check whether if it still has enough space in root directory. Only FAT12/16 have Root directory region.
    if ((drv->DirStackPoint == 0) && ((drv->Flag.FsType == FS_TYPE_FAT12) || (drv->Flag.FsType == FS_TYPE_FAT16)))
    {
        dwRemainFdbCount = 256; /* here, 256 == 8KB/sizeof(FDB) */
        //note: for FAT12/16, drv->RootStart is lba of first sector of root directory
        dwRemainFdbCount -= ((1 << (drv->bSectorExp - 5)) * ((DWORD) drv->DirCachePoint - drv->RootStart));
        DirLast(drv);
        dwRemainFdbCount -= (((((DWORD) drv->Node) - ((DWORD) drv->DirCacheBuffer)) / 32) + 1);
        if (dwRemainFdbCount < (dwFdbCount + 1))
        {
            MP_ALERT("%s: -E No enough space in root directory of this FAT12/FAT16 drive !", __FUNCTION__);
            MP_ALERT("%s: dwRemainFdbCount (=%d) < (dwFdbCount(=%d) + 1)", __FUNCTION__, dwRemainFdbCount, dwFdbCount);
            ker_mem_free(wLongNameBuf);
            ker_mem_free(wTempNameBuf);

#if FS_REENTRANT_API
            Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return ABNORMAL_STATUS;
        }
    }

    // generate short name
    MpMemSet(bShortNameBuf, 0, sizeof(bShortNameBuf)); /* note: Don't use 0x20 here, to avoid string terminator issue */
    bShortNameBuf[11] = 0x00; /* need a null terminator, otherwise UpperCase08() will not stop and overwrite other memory block */

    MpMemSet(bExt, 0, 4);
    if (ExtenL > 0)
    {
        if (ExtenL >= 3)
            MpMemCopy(bExt, bTempExt, 3);
        else
            MpMemCopy(bExt, bTempExt, ExtenL);

        UpperCase08(bExt);
    }

#if 1 //note: this valid_char_count_in_priname counting seems not work as what I expect.
      //      but still no harm for my later code flow.
      //Just a remark here for future fine tune.
    BYTE  priname_len_limit, valid_char_count_in_priname = 0;
    BYTE  *bTmp = (BYTE *) wLongNameBuf;
    WORD  *wTmp = (WORD *) wLongNameBuf;
    for (i=0; i < (MIN(final_nameL, ori_primary_nameL) * 2); i++)
    {
        if (((WORD) *wTmp == 0x0000) || ((WORD) *wTmp == 0x0020))
            break;

        if (i % 2)
            ((WORD *) wTmp)++;

        if ((*bTmp != 0x00) && (*bTmp != 0x20))
            valid_char_count_in_priname++;

        if (valid_char_count_in_priname > 8) /* exceed 8.3 short name limit */
            break;
    }
    //mpDebugPrint("%s: valid_char_count_in_priname = %d", __FUNCTION__, valid_char_count_in_priname);
#endif

    if (valid_char_count_in_priname <= 8) /* no need to reserve 2-byte space for "~1" when doing first time FileSearch() */
    {
        priname_len_limit = 8;

        wTmp = (WORD *) wLongNameBuf;
        for (i=0, j=0; j < MIN(priname_len_limit, ori_primary_nameL); i++)
        {
            if ((WORD) *wTmp == 0x0000)
                break;

            if (i % 2)
                ((WORD *) wTmp)++;

            if (((BYTE *) wLongNameBuf)[i] == 0)
                continue; //skip this byte, not feed into 8.3 short name buffer

            bShortNameBuf[j] = ((BYTE *) wLongNameBuf)[i];
            if (((BYTE *) wLongNameBuf)[i] == 0x20)
            {
                j--;
            }

            j++;
        }
        bShortNameBuf[j] = 0x00;
        UpperCase08(bShortNameBuf);

        if (FileSearch(drv, bShortNameBuf, bExt, E_BOTH_FILE_AND_DIR_TYPE) != FS_SUCCEED)
        {
            if (StringLength08(bExt) > 0)
                StringNCopy08(&bShortNameBuf[8], bExt, 3);
            else
                MpMemSet(&bShortNameBuf[8], 0x20, 3);

            bShortNameBuf[11] = 0x00;
            goto L_do_file_checksum;
        }
    }
    else
        priname_len_limit = 6; /* reserve 2-byte space for "~1" */

    wTmp = (WORD *) wLongNameBuf;
    for (i=0, j=0; (j < priname_len_limit); i++)
    {
        if ((WORD) *wTmp == 0x0000)
            break;

        if (i % 2)
            ((WORD *) wTmp)++;

        if (((BYTE *) wLongNameBuf)[i] == 0)
            continue; //skip this byte, not feed into 8.3 short name buffer

        bShortNameBuf[j] = ((BYTE *) wLongNameBuf)[i];
        if (((BYTE *) wLongNameBuf)[i] == 0x20)
        {
            j--;
        }

        j++;
    }
    bShortNameBuf[j] = 0x00;
    UpperCase08(bShortNameBuf);

    /* add numeric-tail into 8.3 short file name */
    dwNumericTail = 1;
    dwNumericTailLength = 0;
    j = StringLength08(bShortNameBuf); // current length
    while (dwNumericTailLength <= 6) //numeric-tail range "~1" to "~999999"
    {
        dwNumericTailLength = mp_sprintf(bNumericTailBuf, "%d", dwNumericTail); // tail length

        if ((8 - j) > dwNumericTailLength)
        {
            bShortNameBuf[j] = '~';
            StringNCopy08(&bShortNameBuf[j + 1], bNumericTailBuf, dwNumericTailLength);
        }
        else
        {
            bShortNameBuf[(8 - (dwNumericTailLength + 1))] = '~';
            StringNCopy08(&bShortNameBuf[(8 - dwNumericTailLength)], bNumericTailBuf, dwNumericTailLength);
        }

        if (FileSearch(drv, bShortNameBuf, bExt, E_BOTH_FILE_AND_DIR_TYPE) != FS_SUCCEED)
        {
            if (StringLength08(bExt) > 0)
                StringNCopy08(&bShortNameBuf[8], bExt, 3);
            else
                MpMemSet(&bShortNameBuf[8], 0x20, 3);

            bShortNameBuf[11] = 0x00;
            break;
        }
        dwNumericTail++;
    }

L_do_file_checksum:
    // generate checksum
    bChkSum = 0;
    for (i = 0; i < 11; i++)
    {
        if (bShortNameBuf[i] == 0x00)
            bShortNameBuf[i] = 0x20; /* 8.3 short name field in the FDB should not contain null terminators, it uses spaces for padding */

        //NOTE: The operation is an unsigned char rotate right
        bChkSum = ((bChkSum & 1) ? 0x80 : 0) + (bChkSum >> 1) + bShortNameBuf[i];
    }

    // prepare filename to be written to long name FDB nodes
    pwTempBufPtr = (WORD *) ((WORD *) wLongNameBuf + final_nameL);

    CheckDeletedCount(dwFdbCount+1);

    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
    if (dir->Size - dir->Point  < (dwFdbCount + 1) * sizeof(FDB))
    {
        DWORD curr_cluster, curr_sector;
        FDB *fdb_ptr;

        MP_DEBUG("%s: space in the directory not enough => ChainExtending()...", __FUNCTION__);
        ret = ChainExtending(drv, dir);
        if (ret != FS_SUCCEED)
        {
            MP_ALERT("%s: -E ChainExtending() failed! => return ret=0x%x;", __FUNCTION__, ret);
            ker_mem_free(wLongNameBuf);
            ker_mem_free(wTempNameBuf);

#if FS_REENTRANT_API
            Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return ret;
        }

        //note: For a long-name file or directory, its FDB nodes must be contiguous. Here, "contiguous" can be spanning cluster boundary !
        //      So, we don't need to check whether if contiguous FDBs span cluster boundary.
    }

#if FS_REENTRANT_API
    Set_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Set_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    // get long-name FDB nodes first
    for (i = dwFdbCount; i != 0; i--)
    {
        // get a new node location and move on it
        if (GetNewNode(drv) != FS_SUCCEED)
        {
            MP_DEBUG("%s: -I- GetNewNode() failed to get free FDB node => GetDeletedNode()...", __FUNCTION__);
            if (GetDeletedNode(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: -E failed to get free FDB node !", __FUNCTION__);
                ker_mem_free(wLongNameBuf);
                ker_mem_free(wTempNameBuf);

    #if FS_REENTRANT_API
                Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
                if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                   Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
    #endif

                SemaphoreRelease(FILE_WRITE_SEMA_ID);
                return ABNORMAL_STATUS;
            }
        }
        sLongName = (LONG_NAME *) drv->Node;

        // generate long name FDB
        sLongName->Attribute1 = FDB_LONG_NAME;
        sLongName->Attribute2 = 0x00;
        sLongName->CheckSum = bChkSum;
        sLongName->res[0] = sLongName->res[1] = 0x00;

        if (i != dwFdbCount)
        {
            sLongName->Number = i;
            (WORD *) pwTempBufPtr -= 13;
            StringNCopy16((WORD *) bTempBuf1, (WORD *) pwTempBufPtr, 13);
        }
        else
        {
            sLongName->Number = (0x40 | i);
            MpMemSet(bTempBuf1, 0xff, sizeof(bTempBuf1));
            if (i == 1) // totally only one single long name FDB
            {
                (WORD *) pwTempBufPtr -= final_nameL;
            }
            else // this is the last long name FDB
            {
                pwTempBufPtr = (WORD *) ((WORD *) wLongNameBuf + ((dwFdbCount - 1) * 13));
            }
            StringCopy16((WORD *) bTempBuf1, (WORD *) pwTempBufPtr);
        }

        LoadAlienArray16((WORD *) bTempBuf1, bTempBuf1, 26);

        MpMemCopy(sLongName->Name0, bTempBuf1, 10);
        MpMemCopy(sLongName->Name1, bTempBuf1 + 10, 12);
        MpMemCopy(sLongName->Name2, bTempBuf1 + 22, 4);

/* note: This block is necessary ! For long filename, contiguous FDB nodes may cross cluster boundary !
 *       We compare DirCachePoint value to check covered Dir content (FDBs) range before GetNewNode() allocating free FDBs.
 */
#if 1
        drv->Flag.DirCacheChanged = 1;

    #if FS_REENTRANT_API
        /* FDB nodes are allocated now => force to write Dir cache to device right away !
         * note: this is important to sync Dir cache and FDBs info for concurrent multiple file/folder creation API invoking case.
         */
        if (DirCaching(drv) != FS_SUCCEED)
        {
            MP_ALERT("%s: -E DirCaching() failed !! => return ABNORMAL_STATUS;", __FUNCTION__);
            ker_mem_free(wLongNameBuf);
            ker_mem_free(wTempNameBuf);

            Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
               Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return ABNORMAL_STATUS;
        }
    #endif
#endif
    }

    // get a new node location and move on it
    if (GetNewNode(drv) != FS_SUCCEED)
    {
        MP_DEBUG("%s: -I- GetNewNode() failed to get free FDB node => GetDeletedNode()...", __FUNCTION__);
        if (GetDeletedNode(drv) != FS_SUCCEED)
        {
            MP_ALERT("%s: -E failed to get free FDB node !", __FUNCTION__);
            ker_mem_free(wLongNameBuf);
            ker_mem_free(wTempNameBuf);

    #if FS_REENTRANT_API
            Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
               Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
    #endif

            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return ABNORMAL_STATUS;
        }
    }
    sNode = (FDB *) (drv->Node);
    /* note: add pointer check for safety ! Because MpMemSet() may cause "break 100" when memory buffer was already released first by Card Out, for example. */
    if ((BYTE *) sNode != NULL)
        MpMemSet(sNode, 0, sizeof(FDB));

    for (i = 0; i < 11; i++)
        sNode->Name[i] = bShortNameBuf[i];

    if (sNode->Name[0] == 0xE5)
        sNode->Name[0] = 0x05;

    sNode->Attribute = FDB_ARCHIVE;

    SaveAlien16(&sNode->StartLow, dwAddr);
    dwAddr >>= 16;
    SaveAlien16(&sNode->StartHigh, dwAddr);
    SaveAlien32(&sNode->Size, 0);

    /* fill in the create date/time and modified date */
    SystemTimeGet(&sys_time);
    file_date = FileSetDate_for_FATxx(sys_time.u16Year, sys_time.u08Month, sys_time.u08Day);
    file_time = FileSetTime_for_FATxx(sys_time.u08Hour, sys_time.u08Minute, sys_time.u08Second);
    sNode->CreateTime = file_time;
    sNode->CreateDate = file_date;
    sNode->AccessDate = file_date;
    sNode->ModifyTime = file_time;
    sNode->ModifyDate = file_date;

    drv->Flag.DirCacheChanged = 1;

    /* FDB nodes are allocated now => force to write Dir cache to device right away !
     * note: this is important to sync Dir cache and FDBs info for concurrent multiple file/folder creation API invoking case.
     */
    if (DirCaching(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E DirCaching() failed !! => return ABNORMAL_STATUS;", __FUNCTION__);
        ker_mem_free(wLongNameBuf);
        ker_mem_free(wTempNameBuf);

    #if FS_REENTRANT_API
        Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
    #endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

#if FS_REENTRANT_API
    Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    ker_mem_free(wLongNameBuf);
    ker_mem_free(wTempNameBuf);
    SemaphoreRelease(FILE_WRITE_SEMA_ID);
    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Delete an opened file.
///
///@param   handle      The handle of the file to delete.
///
///@retval  FS_SUCCEED          Delete file successfully. \n\n
///@retval  ABNORMAL_STATUS     Delete file unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no need to delete it.
///
int DeleteFile(STREAM * handle)
{
    DRIVE *drv;


    if (handle == NULL)
        return FILE_NOT_FOUND;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
        return FileClose(handle);
#endif

    drv = (DRIVE *) (handle->Drv);

    /* make sure drive's Dir cache buffer covers the file FDB */
    if (FileDirCaching(handle) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E- FileDirCaching() failed !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (FileClose(handle) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    if (Unlink(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    if (DriveRefresh(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Move the file byte offset to spcific position.
///
///@param   handle      The handle of the file to seek. \n\n
///@param   position    The byte offset to move.
///
///@retval  FS_SUCCEED          Seek successfully. \n\n
///@retval  OUT_OF_RANGE        The position to seek exceeds file size. \n\n
///@retval  FILE_NOT_FOUND      File not found.
///
int Seek(STREAM * handle, DWORD position)
{
    if (handle == NULL)
        return FILE_NOT_FOUND;

    /* special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (! handle->Flag.f_IsRamFile)
    {
        if (handle->Drv == NULL)
            return FILE_NOT_FOUND; // this file has been closed
    }

    /* note: move "if (position > FileSizeGet(handle))" out of range checks for "RamFile" type files, USBOTG_HOST_PTP,
     *       DEV_USB_WIFI_DEVICE and DEV_CF_ETHERNET_DEVICE cases.
     *       For normal files in FATxx file system storage, we do the checks within ChainSeekForward().
     */

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        /* Here, seek function allows (position == FileSizeGet(handle)).
         * In such case, FileRead() function will return 0 because EOF reached. Valid data range is 0 ~ (FileSizeGet(handle) - 1)
         */
        if (position > FileSizeGet(handle))
        {
            MP_ALERT("%s: Error! file position (= %lu) to seek exceeds file content range ! (file size = %lu)", __FUNCTION__, position, FileSizeGet(handle));
            return OUT_OF_RANGE;
        }

        handle->Chain.Point = position;
        return FS_SUCCEED;
    }
#endif

#if USBOTG_HOST_PTP
    if ((handle->Drv->DevID == DEV_USB_HOST_PTP) || (handle->Drv->DevID == DEV_USBOTG1_HOST_PTP)) //(handle->Drv->DevID == DEV_USB_HOST_PTP)
    {
        /* Here, seek function allows (position == FileSizeGet(handle)).
         * In such case, FileRead() function will return 0 because EOF reached. Valid data range is 0 ~ (FileSizeGet(handle) - 1)
         */
        if (position > FileSizeGet(handle))
        {
            MP_ALERT("%s: Error! file position (= %lu) to seek exceeds file content range ! (file size = %lu)", __FUNCTION__, position, FileSizeGet(handle));
            return OUT_OF_RANGE;
        }

        handle->Chain.Point = position;
        return FS_SUCCEED;
    }
#endif

#if ( NETWARE_ENABLE || USBOTG_WEB_CAM )
    if ((handle->Drv->DevID == DEV_USB_WIFI_DEVICE) ||\
        (handle->Drv->DevID == DEV_CF_ETHERNET_DEVICE) ||\
        (handle->Drv->DevID == DEV_USB_ETHERNET_DEVICE) ||\
        (handle->Drv->DevID == DEV_USB_WEBCAM))
    {
        /* Here, seek function allows (position == FileSizeGet(handle)).
         * In such case, FileRead() function will return 0 because EOF reached. Valid data range is 0 ~ (FileSizeGet(handle) - 1)
         */
        if (position > FileSizeGet(handle))
        {
            MP_ALERT("%s: Error! file position (= %lu) to seek exceeds file content range ! (file size = %lu)", __FUNCTION__, position, FileSizeGet(handle));
            return OUT_OF_RANGE;
        }

        handle->Chain.Point = position;
        return FS_SUCCEED;
    }
#endif

    /* note: in current iPlay code, 'chain->Size' may be value of "real file data length" or "allocated clusters chain size" in different cases.
     * And we have some file-size and/or chain-size related APIs need to take care: ChainChangeSize(), FileWrite_withWriteBack_FileSize() and
     * FileChain_AdjustSize_TrimExtraClusters().
     *
     * Note that the "file size" (i.e. handle->Chain.Size) of a just created file by our file system APIs is set to zero, although we have already
     * allocated a cluster for it.
     *
     * So, we do some checks here first to avoid "out of range of chain size" error.
     */
    return ChainSeek((DRIVE *) (handle->Drv), (CHAIN *) (&handle->Chain), position);
}



///
///@ingroup FILE
///@brief   Move to the start of the specified file.
///
///@param   handle      The handle of the file to seek.
///
///@return  None.
///
void SeekSet(STREAM * handle)
{
    if (handle == NULL)
        return;

    /* special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (! handle->Flag.f_IsRamFile)
    {
        if (handle->Drv == NULL)
            return;
    }

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        handle->Chain.Point = 0;
        return;
    }
#endif

    ChainSeekSet((CHAIN *) (&handle->Chain));
}



///
///@ingroup FILE
///@brief   Move to the end of the specified file.
///
///@param   handle      The handle of the file to seek.
///
///@return  None.
///
void EndOfFile(STREAM * handle)
{
    if (handle == NULL)
        return;

    /* special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (! handle->Flag.f_IsRamFile)
    {
        if (handle->Drv == NULL)
            return;
    }

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        handle->Chain.Point = handle->Chain.Size;
        return;
    }
#endif

    ChainSeekEnd((DRIVE *) (handle->Drv), (CHAIN *) (&handle->Chain));
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to be same as the C standard library "fseek" function.\n
///         It sets the file position of the file to the given offset.
///
///@param   handle      The handle of the file to seek. \n\n
///@param   offset      The number of bytes to seek from the given 'whence' position. \n\n
///@param   whence      Control where to seek from. Its valid values are as below:\n
///					      SEEK_SET   seek from the beginning of the file.\n
///					      SEEK_CUR   seek from the current position.\n
///					      SEEK_END   seek from the end of the file.
///
///@retval  FS_SUCCEED          Seek successfully. \n\n
///@retval  OUT_OF_RANGE        The position to seek exceeds file size. \n\n
///@retval  FAIL                Invalid 'whence' parameter. \n\n
///@retval  FILE_NOT_FOUND      File not found.
///
int Fseek(STREAM * handle, DWORD offset, DWORD whence)
{
    DWORD curPos;


    if (handle == NULL)
        return FILE_NOT_FOUND;

    /* special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (! handle->Flag.f_IsRamFile)
    {
        if (handle->Drv == NULL)
            return FILE_NOT_FOUND; // this file has been closed
    }

    switch (whence)
    {
        case SEEK_SET:
            SeekSet(handle);
            return Seek(handle, offset);
            break;
        case SEEK_CUR:
            curPos = FilePosGet(handle);
            return Seek(handle, curPos + offset);
            break;
        case SEEK_END:
            EndOfFile(handle);
            curPos = FilePosGet(handle);
            return Seek(handle, curPos - offset);
            break;
    }

    return FAIL;
}



///
///@ingroup FILE
///@brief   Get the current position of the specified file.
///
///@param   handle      The handle of the file to access.
///
///@return  The current byte offset within the file.
///
DWORD FilePosGet(STREAM * handle)
{
    return handle->Chain.Point;
}



///
///@ingroup FILE
///@brief   Get the size of the specified file.
///
///@param   handle      The handle of the file to access.
///
///@return  The byte size of the file.
///
DWORD FileSizeGet(STREAM * handle)
{
    return handle->Chain.Size;
}



///
///@ingroup FILE
///@brief   Write data to the specified file.
///
///@param   handle      The handle of the file to access. \n\n
///@param   buffer      The pointer of buffer contain data to write. \n\n
///@param   size        Byte size of data.
///
///@return  The number of bytes actually written. If an error occurred or disk full,\n
///         it may return 0 or the actual count value which is less than the input 'size'.
///
///@remark  The function call will write data from the byte offset saved in chain->Point.
///
DWORD FileWrite(STREAM * handle, BYTE * buffer, DWORD size)
{
    DWORD head, body, tail, dwByteCount;
    DWORD originLength, oldPosition, oldCurrCluster, oldFatCache_lba;
    CHAIN *chain;
    DRIVE *drv;
    BYTE *tmpbuffer;
    BOOL  f_EOF_and_AtClusBoundary = FALSE;


    if (handle == NULL)
        return 0;

    if (buffer == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return 0;
    }

    if (size == 0)
        return 0;

    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        if ((handle->ramFile_buf == NULL) || (handle->ramFile_bufSize == 0))
        {
            MP_ALERT("%s: Error ! The \"RamFile\" has no allocated memory yet !", __FUNCTION__);
            return 0;
        }

        MP_DEBUG("%s: current file size = %lu, current file pos = %lu, request write size = %lu", __FUNCTION__, handle->Chain.Size, handle->Chain.Point, size);

        SemaphoreWait(FILE_WRITE_SEMA_ID);

        originLength = handle->Chain.Size;
        dwByteCount = 0;

        /* test if the write amount exceed the end of the "RamFile" buffer */
        if (handle->Chain.Point + size > handle->ramFile_bufSize)
        {
            dwByteCount = handle->ramFile_bufSize - handle->Chain.Point;
            MP_ALERT("%s: warning: request write size will exceed buffer => adjust write size to (%lu)", __FUNCTION__, dwByteCount);
        }
        else
            dwByteCount = size;

        MpMemCopy((BYTE *) &handle->ramFile_buf[handle->Chain.Point], buffer, dwByteCount);
        handle->Chain.Point += dwByteCount;
        handle->Chain.Size += dwByteCount;

        if (originLength != handle->Chain.Size)
            handle->Flag.SizeChanged = 1;

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return dwByteCount;
    }

    if (handle->Drv == NULL)
        return 0; // this file has been closed

    if (!handle->Drv->Flag.Present)
        return 0;

    /* check Read-Only flag by underlying Mcard layer info => physical lock latch or Mcard layer lock */
    if (SystemGetFlagReadOnly(handle->Drv->DrvIndex))
    {
        MP_ALERT("%s: -E- The drive is Read-Only (controlled in Mcard layer) !", __FUNCTION__);
        return 0;
    }

    /* check Read-Only flag by file system layer => logical or controlled by S/W */
    if (handle->Drv->Flag.ReadOnly)
    {
        MP_ALERT("%s: -E- The drive is Read-Only (controlled in file system layer) !", __FUNCTION__);
    #if EXFAT_ENABLE
        if (handle->Drv->Flag.FsType == FS_TYPE_exFAT)
        {
            MP_DEBUG("%s: The drive is exFAT file system.", __FUNCTION__);
        }
    #endif
        return 0;
    }

    MP_DEBUG("%s: current file size = %lu, current file pos = %lu, request write size = %lu", __FUNCTION__, handle->Chain.Size, handle->Chain.Point, size);

    SemaphoreWait(FILE_WRITE_SEMA_ID);

    drv = (DRIVE *) handle->Drv;
    chain = (CHAIN *) & handle->Chain;
    originLength = chain->Size;
    tmpbuffer = buffer;
    dwByteCount = 0;

    /* save the position info for current chain point */
    oldPosition = chain->Point;
    oldCurrCluster = chain->Current;
    oldFatCache_lba = drv->FatCachePoint;

    f_EOF_and_AtClusBoundary = Is_ChainPoint_EOC_and_At_Boundary(drv, chain);

    // devide the read range into 3 segments
    Segmentize(handle, &head, &body, &tail, size);

    // test if the write amount exceed the end of the chain
    if (chain->Point + size > chain->Size) /* note: here NOT (chain->Point + size >= chain->Size) */
    {
        MP_DEBUG("%s: change and extend file's chain size to total (%lu) bytes ...", __FUNCTION__, chain->Point + size);

        /* change the chain size to have enough space */
        if (ChainChangeSize(drv, chain, chain->Point + size) != FS_SUCCEED)
        {
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }

  #if 0 //ori code: OK, but seek slow.
        if (ChainSeek(drv, chain, oldPosition) != FS_SUCCEED)
        {
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }
  #else //reduce time spent by seeking from the start cluster
        chain->Point = oldPosition;
        if (f_EOF_and_AtClusBoundary)
        {
            if (! SystemCardPresentCheck(drv->DrvIndex))
            {
                MP_ALERT("%s: Card not present !", __FUNCTION__);
                SemaphoreRelease(FILE_WRITE_SEMA_ID);
                return dwByteCount;
            }

            DWORD next_clus_for_write = drv->FatRead(drv, oldCurrCluster);

            if (drv->StatusCode == FS_SCAN_FAIL)
            {
                MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                SemaphoreRelease(FILE_WRITE_SEMA_ID);
                return dwByteCount;
            }

            if (next_clus_for_write == 0xffffffff)
            {
                MP_ALERT("%s: should be impossible to reach here! No clusters gained by ChainChangeSize() !?", __FUNCTION__);
                SemaphoreRelease(FILE_WRITE_SEMA_ID);
                return dwByteCount;
            }
            chain->Current = next_clus_for_write;
        }
        else
        {
            chain->Current = oldCurrCluster;
            FatCaching(drv, oldFatCache_lba); //update FAT cache buffer for covering current point of the chain
        }
  #endif
    }

    if (head)
    {
        if (ChainFragmentWrite((DRIVE *) (handle->Drv), chain, tmpbuffer, head) != FS_SUCCEED)
        {
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }
        tmpbuffer += head;
        dwByteCount += head;
    }

    if (body)
    {
        if (ChainWrite((DRIVE *) (handle->Drv), chain, tmpbuffer, body >> drv->bSectorExp) != FS_SUCCEED)
        {
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }
        tmpbuffer += body;
        dwByteCount += body;
    }

    if (tail)
    {
        if (ChainFragmentWrite((DRIVE *) (handle->Drv), chain, tmpbuffer, tail) != FS_SUCCEED)
        {
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }
        dwByteCount += tail;
    }

    if (originLength != chain->Size)
        handle->Flag.SizeChanged = 1;  //indicator for FileClose()/FileForceSync() to write back real file size

    SemaphoreRelease(FILE_WRITE_SEMA_ID);
    return dwByteCount;
}



///
///@ingroup FILE
///@brief   This function is a special variant of normal FileWrite() function. This function is intended to  \n
///         extend the file's clusters chain with some more extra and fixed size (ex: 0 ~ some MBs) pre-allocated clusters \n
///         when needing to extend file's clusters chain each time than the original FileWrite() function, and it will \n
///         write back the file size to disk immediately to match allocated FAT chain. \n
///         Such method is to solve the mismatch problem between file size and file's FAT chain on the disk occurred when \n
///         unplugging the disk during file writing progress (ex: MP663 miniDV video recording). \n
///         So, this function is suitable for applications like video recording to use. \n\n
///
///@param   handle      The handle of the file to access. \n\n
///@param   buffer      The pointer of buffer contain data to write. \n\n
///@param   size        Byte size of data. \n\n
///@param   num_MB_to_extend_chain   Number of MB in size of extra clusters more than original FileWrite() API to extend when needed. \n
///                                  If value of this parameter is zero, it extends same amount of clusters as FileWrite(). \n\n
///
///@return  The number of bytes actually written. If an error occurred or disk full, \n
///         it may return 0 or the actual count value which is less than the input 'size'. \n\n
///
///@remark  The function call will write data from the byte offset saved in chain->Point which is not influenced by \n
///         pre-allocated FAT clusters. \n\n
///
///@remark  Note that we have a new flag 'f_ExtraClustersAllocated' in STREAM structure type to distinguish FileWrite() \n
///         and FileWrite_withWriteBack_FileSize() for FileClose() to check whether if need to do file size correction  \n
///         and release needless FAT clusters behind actual file content size for FileWrite_withWriteBack_FileSize() case. \n\n
///
DWORD FileWrite_withWriteBack_FileSize(STREAM * handle, BYTE * buffer, DWORD size, DWORD num_MB_to_extend_chain)
{
    DWORD head, body, tail, dwByteCount;
    DWORD originLength, oldPosition, oldCurrCluster, oldFatCache_lba;
    CHAIN *chain;
    DRIVE *drv;
    BYTE  *tmpbuffer;
    DWORD allocated_clusters_count, allocated_total_clusters_size;
    BOOL  f_EOF_and_AtClusBoundary = FALSE;


    if (handle == NULL)
        return 0;

    if (buffer == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return 0;
    }

    if (size == 0)
        return 0;

    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        if ((handle->ramFile_buf == NULL) || (handle->ramFile_bufSize == 0))
        {
            MP_ALERT("%s: Error ! The \"RamFile\" has no allocated memory yet !", __FUNCTION__);
            return 0;
        }

        MP_DEBUG("%s: current file size = %lu, current file pos = %lu, request write size = %lu", __FUNCTION__, handle->Chain.Size, handle->Chain.Point, size);

        SemaphoreWait(FILE_WRITE_SEMA_ID);

        originLength = handle->Chain.Size;
        dwByteCount = 0;

        /* test if the write amount exceed the end of the "RamFile" buffer */
        if (handle->Chain.Point + size > handle->ramFile_bufSize)
        {
            dwByteCount = handle->ramFile_bufSize - handle->Chain.Point;
            MP_ALERT("%s: warning: request write size will exceed buffer => adjust write size to (%lu)", __FUNCTION__, dwByteCount);
        }
        else
            dwByteCount = size;

        MpMemCopy((BYTE *) &handle->ramFile_buf[handle->Chain.Point], buffer, dwByteCount);
        handle->Chain.Point += dwByteCount;
        handle->Chain.Size += dwByteCount;

        if (originLength != handle->Chain.Size)
            handle->Flag.SizeChanged = 1;

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return dwByteCount;
    }

    if (handle->Drv == NULL)
        return 0; // this file has been closed

    if (!handle->Drv->Flag.Present)
        return 0;

    /* check Read-Only flag by underlying Mcard layer info => physical lock latch or Mcard layer lock */
    if (SystemGetFlagReadOnly(handle->Drv->DrvIndex))
    {
        MP_ALERT("%s: -E- The drive is Read-Only (controlled in Mcard layer) !", __FUNCTION__);
        return 0;
    }

    /* check Read-Only flag by file system layer => logical or controlled by S/W */
    if (handle->Drv->Flag.ReadOnly)
    {
        MP_ALERT("%s: -E- The drive is Read-Only (controlled in file system layer) !", __FUNCTION__);
    #if EXFAT_ENABLE
        if (handle->Drv->Flag.FsType == FS_TYPE_exFAT)
        {
            MP_DEBUG("%s: The drive is exFAT file system.", __FUNCTION__);
        }
    #endif
        return 0;
    }

    MP_DEBUG("%s: current file size = %lu, current file pos = %lu, request write size = %lu", __FUNCTION__, handle->Chain.Size, handle->Chain.Point, size);

    SemaphoreWait(FILE_WRITE_SEMA_ID);

    drv = (DRIVE *) handle->Drv;
    chain = (CHAIN *) & handle->Chain;
    originLength = chain->Size;
    tmpbuffer = buffer;
    dwByteCount = 0;

    /* save the position info for current chain point */
    oldPosition = chain->Point;
    oldCurrCluster = chain->Current;
    oldFatCache_lba = drv->FatCachePoint;

    f_EOF_and_AtClusBoundary = Is_ChainPoint_EOC_and_At_Boundary(drv, chain);

    // devide the read range into 3 segments
    Segmentize(handle, &head, &body, &tail, size);

    if (chain->Size == 0) /* must be a just created file, and it has allocated a cluster already */
    {
        allocated_clusters_count = 1;
        allocated_total_clusters_size = allocated_clusters_count << (drv->ClusterExp + drv->bSectorExp);

        /* update chain->Size value to avoid later ChainSeekXXXX() failure due to "out of range of chain size" */
        chain->Size = allocated_total_clusters_size;
    }
    else
    {
        allocated_clusters_count = chain->Size >> (drv->ClusterExp + drv->bSectorExp);
        if (chain->Size % (1 << (drv->ClusterExp + drv->bSectorExp)))
            allocated_clusters_count++;
        allocated_total_clusters_size = allocated_clusters_count << (drv->ClusterExp + drv->bSectorExp);
    }

    // test if the write amount exceed the end of the chain
    if (chain->Point + size > allocated_total_clusters_size) /* note: here NOT (chain->Point + size >= allocated_total_clusters_size) */
    {
        /* calculate new enough 'allocated_total_clusters_size' value for the writing */
        allocated_clusters_count = (chain->Point + size) >> (drv->ClusterExp + drv->bSectorExp);
        if ((chain->Point + size) % (1 << (drv->ClusterExp + drv->bSectorExp)))
            allocated_clusters_count++;
        allocated_total_clusters_size = allocated_clusters_count << (drv->ClusterExp + drv->bSectorExp);

        MP_DEBUG("%s: change and extend file's chain size to total (%lu) bytes ...", __FUNCTION__, allocated_total_clusters_size + (num_MB_to_extend_chain << 20));

        /* change the chain size to have enough space */
        if (ChainChangeSize(drv, chain, allocated_total_clusters_size + (num_MB_to_extend_chain << 20)) != FS_SUCCEED)
        {
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }

        handle->Flag.f_ExtraClustersAllocated = 1;  //for FileWrite_withWriteBack_FileSize() only

        if (originLength != chain->Size)
            handle->Flag.SizeChanged = 1;  //indicator for FileClose()/FileForceSync() to write back real file size

        /* force sync to write back "total size of allocated file clusters chain" to disk for card un-plugging or power-off issue */
        if (FileForceSync(handle) != FS_SUCCEED)
        {
            MP_ALERT("%s: -E- Failed writing back file size to disk for sync !", __FUNCTION__);
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }

  #if 0 //ori code: OK, but seek slow.
        if (ChainSeek(drv, chain, oldPosition) != FS_SUCCEED)
        {
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }
  #else //reduce time spent by seeking from the start cluster
        chain->Point = oldPosition;
        if (f_EOF_and_AtClusBoundary)
        {
            if (! SystemCardPresentCheck(drv->DrvIndex))
            {
                MP_ALERT("%s: Card not present !", __FUNCTION__);
                SemaphoreRelease(FILE_WRITE_SEMA_ID);
                return dwByteCount;
            }

            DWORD next_clus_for_write = drv->FatRead(drv, oldCurrCluster);

            if (drv->StatusCode == FS_SCAN_FAIL)
            {
                MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                SemaphoreRelease(FILE_WRITE_SEMA_ID);
                return dwByteCount;
            }

            if (next_clus_for_write == 0xffffffff)
            {
                MP_ALERT("%s: should be impossible to reach here! No clusters gained by ChainChangeSize() !?", __FUNCTION__);
                SemaphoreRelease(FILE_WRITE_SEMA_ID);
                return dwByteCount;
            }
            chain->Current = next_clus_for_write;
        }
        else
        {
            chain->Current = oldCurrCluster;
            FatCaching(drv, oldFatCache_lba); //update FAT cache buffer for covering current point of the chain
        }
  #endif
    }
    else  //within the range of allocated file's clusters chain => no need to extend clusters chain
    {
        if (chain->Point + size > chain->Size)
            chain->Size = chain->Point + size;

        /* note: No need to do FileForceSync(handle) for such case, otherwise, performance will be bad.
         *       We still do the write back of "real file size" to file's FDB on disk when FileClose().
         */
    }

    if (head)
    {
        if (ChainFragmentWrite((DRIVE *) (handle->Drv), chain, tmpbuffer, head) != FS_SUCCEED)
        {
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }
        tmpbuffer += head;
        dwByteCount += head;
    }

    if (body)
    {
        if (ChainWrite((DRIVE *) (handle->Drv), chain, tmpbuffer, body >> drv->bSectorExp) != FS_SUCCEED)
        {
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }
        tmpbuffer += body;
        dwByteCount += body;
    }

    if (tail)
    {
        if (ChainFragmentWrite((DRIVE *) (handle->Drv), chain, tmpbuffer, tail) != FS_SUCCEED)
        {
            SemaphoreRelease(FILE_WRITE_SEMA_ID);
            return dwByteCount;
        }
        dwByteCount += tail;
    }

    if (originLength != chain->Size)
        handle->Flag.SizeChanged = 1;  //indicator for FileClose()/FileForceSync() to write back real file size

    SemaphoreRelease(FILE_WRITE_SEMA_ID);
    return dwByteCount;
}



///
///@ingroup FILE
///@brief   Adjust file chain size of a file to trim extra clusters allocated by FileWrite_withWriteBack_FileSize() if any. \n
///         This function is special for applications (ex: mini-DV) that use FileWrite_withWriteBack_FileSize() allocating extra clusters. \n
///         This function trim extra allocated clusters and adjust the file size to the final chain point

///         FileClose() will do similar thing with this function to trim the extra file clusters if any, but this function will \n
///         not close/release the file handle and let it remain open.
///
///@param   handle      The handle of the file to adjust its file chain size to trim extra clusters.
///
///@retval  FS_SUCCEED          Adjust file chain successfully. \n\n
///@retval  ABNORMAL_STATUS     Adjust file chain unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found or other cases that no need to do the file chain adjustment.
///
int FileChain_AdjustSize_TrimExtraClusters(STREAM * handle)
{
    DRIVE *drv;
    FDB *node;
    CHAIN *chain;


    if (handle == NULL)
        return FILE_NOT_FOUND;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
        return FILE_NOT_FOUND;
#endif

    drv = (DRIVE *) (handle->Drv);
    if (drv == NULL)
        return FILE_NOT_FOUND; // this file has been closed, except "RamFile"

    if (! handle->Flag.f_ExtraClustersAllocated) //no need to adjust file clusters
        return FS_SUCCEED;

    if (handle->Flag.SizeChanged)
    {
        /* make sure drive's Dir cache buffer covers the file FDB */
        if (FileDirCaching(handle) != FS_SUCCEED)
        {
            MP_ALERT("%s: -E- FileDirCaching() failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        node = (FDB *) drv->DirCacheBuffer + handle->FdbOffsetInDirCache;
        chain = (CHAIN *) & handle->Chain;

        if (drv->Flag.FsType != FS_TYPE_exFAT)
        {
            if (chain->Point == chain->Size)
                SaveAlien32(&node->Size, chain->Size);
            else
            {
#if RELEASE_NEEDLESS_EXTRA_CLUSTERS_WHEN_FILE_CLOSE
                MP_ALERT("%s: adjust file's chain size to (%lu) bytes ...", __FUNCTION__, chain->Point);

                /* reduce chain size to release needless FAT clusters behind actual file content size */
                if (ChainChangeSize(drv, chain, chain->Point) != FS_SUCCEED)
                {
                    MP_ALERT("%s: failed to adjust file's chain size !", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }

                SaveAlien32(&node->Size, chain->Point);
#else
                SaveAlien32(&node->Size, chain->Size);
#endif
                handle->Flag.f_ExtraClustersAllocated = 0;
            }
        }
#if EXFAT_ENABLE
        else
        {
            TYPE_exFAT_Stream_Extension_DirEntry *exfat_node;
            BYTE *Hi32_of_64, *Lo32_of_64;
            DWORD dwSize = 0;

            /* current drv->Node should be a "Stream Extension DirectoryEntry" of exFAT */
            exfat_node = (TYPE_exFAT_Stream_Extension_DirEntry *) (drv->Node);
            if (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT)
            {
                MP_ALERT("%s: Error! Current node should be a (Stream Extension DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
                return ABNORMAL_STATUS;
            }

            /* note: Currently, we do not support WRITE operation in exFAT, so we don't need to process 'f_ExtraClustersAllocated' flag here */

            Lo32_of_64 = (BYTE *) &exfat_node->dataLength; /* due to little-endian */
            Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
            if ((U64) chain->Size <= (U64) 0x00000000FFFFFFFF)
            {
                dwSize = (DWORD) chain->Size;
                SaveAlien32(Hi32_of_64, 0);
                SaveAlien32(Lo32_of_64, dwSize);
            }
            else
            {
                dwSize = (DWORD) (((U64) chain->Size & 0xFFFFFFFF00000000) >> 32);
                SaveAlien32(Hi32_of_64, dwSize);

                dwSize = (DWORD) ((U64) chain->Size & 0x00000000FFFFFFFF);
                SaveAlien32(Lo32_of_64, dwSize);
            }
        }
#endif
        drv->Flag.DirCacheChanged = 1;

        // here, write back changed DirCache and FatCache and free cluster info together
        if (DriveRefresh(drv) != FS_SUCCEED)
            return ABNORMAL_STATUS;

        handle->Flag.SizeChanged = 0;
    }

    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Read data from the specified file.
///
///@param   handle      The handle of the file to access. \n\n
///@param   buffer      The pointer of buffer for data read. \n\n
///@param   size        Byte size of data.
///
///@return  The number of bytes actually read. If an error occurred or reach the end of the file,\n
///         it may return 0 or the actual count value which is less than the input 'size'.
///
///@remark  The function call will read data from the byte offset saved in chain->Point.
///
DWORD FileRead(STREAM * handle, BYTE * buffer, DWORD size)
{
    DWORD head, body, tail;
    DWORD dwByteCount = 0;
    DRIVE *drv;
    BYTE *tmpbuffer;


    if (handle == NULL)
        return 0;

    if (buffer == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return 0;
    }

    if (size == 0)
        return 0;

    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        if ((handle->ramFile_buf == NULL) || (handle->ramFile_bufSize == 0))
        {
            MP_ALERT("%s: Error ! The \"RamFile\" has no allocated memory yet !", __FUNCTION__);
            return 0;
        }

        MP_DEBUG("%s: file size = %lu, current file pos = %lu, request read size = %lu", __FUNCTION__, handle->Chain.Size, handle->Chain.Point, size);

        SemaphoreWait(FILE_READ_SEMA_ID);
        dwByteCount = 0;

        if (handle->Chain.Point >= handle->Chain.Size) //reach EOF
        {
            MP_DEBUG("%s: reach EOF !", __FUNCTION__);
            goto FILE_READ_ERROR;
        }

        if ((handle->Chain.Point + size) > handle->Chain.Size)
        {
            dwByteCount = handle->Chain.Size - handle->Chain.Point;
            MP_DEBUG("%s: warning: request read size will exceed EOF => adjust read size to (%lu)", __FUNCTION__, dwByteCount);
        }
        else
            dwByteCount = size;

        /* read data from the "RamFile" */
        MpMemCopy(buffer, (BYTE *) &handle->ramFile_buf[handle->Chain.Point], dwByteCount);
        handle->Chain.Point += dwByteCount;

        SemaphoreRelease(FILE_READ_SEMA_ID);
        MP_DEBUG("%s: return read length = %lu, current file pos = %lu \r\n", __FUNCTION__, dwByteCount, handle->Chain.Point);
        return dwByteCount;
    }

    if (handle->Drv == NULL)
    {
        MP_ALERT("%s: Error! This file has been closed !", __FUNCTION__);
        return 0; // this file has been closed
    }

    if (!handle->Drv->Flag.Present)
    {
        MP_ALERT("%s: Error! Card not present !", __FUNCTION__);
        return 0;
    }

    MP_DEBUG("%s: file size = %lu, current file pos = %lu, request read size = %lu", __FUNCTION__, handle->Chain.Size, handle->Chain.Point, size);

    SemaphoreWait(FILE_READ_SEMA_ID);

#if USBOTG_HOST_PTP
    if ((handle->Drv->DevID == DEV_USB_HOST_PTP) || (handle->Drv->DevID == DEV_USBOTG1_HOST_PTP)) //(handle->Drv->DevID == DEV_USB_HOST_PTP)
    {
        if ((handle->Chain.Point + size) > handle->Chain.Size)
        {
            size = handle->Chain.Size - handle->Chain.Point;
        }

        if (size != 0)
        {
            handle->Drv->Node = (FDB*)handle->DirSector;
            drv = (DRIVE *)handle->Drv;
            if (DriveRead(drv, buffer, handle->Chain.Point, size) != FS_SUCCEED)
            {
                SemaphoreRelease(FILE_READ_SEMA_ID);
                return 0;
            }
            handle->Chain.Point += size;
        }
        SemaphoreRelease(FILE_READ_SEMA_ID);
        return size;
    }
#endif

#if ( NETWARE_ENABLE || USBOTG_WEB_CAM )
    if ((handle->Drv->DevID == DEV_USB_WIFI_DEVICE) ||\
        (handle->Drv->DevID == DEV_CF_ETHERNET_DEVICE) ||\ 
        (handle->Drv->DevID == DEV_USB_ETHERNET_DEVICE) ||\
        (handle->Drv->DevID == DEV_USB_WEBCAM))
    {
        if (size == 0)		// abel 20071002
        {
            SemaphoreRelease(FILE_READ_SEMA_ID);
            return 0;
        }

        if ((handle->Chain.Point + size) > handle->Chain.Size)
        {
            size = handle->Chain.Size - handle->Chain.Point;
        }

//#if (USBOTG_WEB_CAM && NETWARE_ENABLE)
#if (USBOTG_WEB_CAM && (NETWARE_ENABLE && MAKE_XPG_PLAYER))
        if ((handle->Drv->DevID == DEV_USB_ETHERNET_DEVICE) ||\
            (handle->Drv->DevID == DEV_USB_WEBCAM))
        {
            size = Fill_PlayoutBuffer(buffer,handle->Chain.Point, (handle->Chain.Point + size - 1));
        }
        else
        {
            size = NetFileRead(handle->DirSector, buffer, handle->Chain.Point, (handle->Chain.Point + size - 1));
        }
#elif ((NETWARE_ENABLE && MAKE_XPG_PLAYER) && !USBOTG_WEB_CAM)
        {
            size = NetFileRead(handle->DirSector, buffer, handle->Chain.Point, (handle->Chain.Point + size - 1));
        }
#endif
        
        handle->Chain.Point += size;
        SemaphoreRelease(FILE_READ_SEMA_ID);
        return size;
    }
#endif

    if (handle->Chain.Point >= handle->Chain.Size) //reach EOF
    {
        MP_ALERT("%s: -I-  reach EOF.", __FUNCTION__);
        goto FILE_READ_ERROR;
    }

    tmpbuffer = buffer;
    dwByteCount = 0;
    if ((handle->Chain.Point + size) > handle->Chain.Size)
    {
        size = handle->Chain.Size - handle->Chain.Point;
        MP_DEBUG("%s: warning: request read size will exceed EOF => adjust read size to (%lu)", __FUNCTION__, size);
    }
    // devide the read range into 3 segments
    Segmentize(handle, &head, &body, &tail, size);

    drv = (DRIVE *) (handle->Drv);
    if (!drv->Flag.Present)
    {
        MP_ALERT("%s: Error! Card not present !", __FUNCTION__);
        SemaphoreRelease(FILE_READ_SEMA_ID);
        return 0;
    }

    if (head)
    {
        if (ChainFragmentRead(drv, (CHAIN *) & handle->Chain, tmpbuffer, head) != FS_SUCCEED)
            goto FILE_READ_ERROR;
        tmpbuffer += head;
        dwByteCount += head;
    }

    if (body)
    {
        if (!drv->Flag.Present)
            goto FILE_READ_ERROR;

        //note: here, non-aligned buffer address issue is processed in DriveRead()/DriveWrite()
        if (ChainRead((DRIVE *) drv, (CHAIN *) (&handle->Chain), tmpbuffer, body >> drv->bSectorExp) != FS_SUCCEED)
            goto FILE_READ_ERROR;

        tmpbuffer += body;
        dwByteCount += body;
    }

    if (tail)
    {
        if (ChainFragmentRead((DRIVE *) drv, (CHAIN *) (&handle->Chain), tmpbuffer, tail) != FS_SUCCEED)
            goto FILE_READ_ERROR;

        dwByteCount += tail;
    }

FILE_READ_ERROR:
    SemaphoreRelease(FILE_READ_SEMA_ID);
    MP_DEBUG("%s: return read length = %lu, current file pos = %lu \r\n", __FUNCTION__, dwByteCount, handle->Chain.Point);
    return dwByteCount;
}



static struct
{
    BYTE *bExt;
    WORD wSize;
    WORD wType;
} file_ext_table[] =
{

//photo types
    {"JPG ", 3, FILE_OP_TYPE_IMAGE},
    {"BMP ", 3, FILE_OP_TYPE_IMAGE},
    {"GIF ", 3, FILE_OP_TYPE_IMAGE},
    {"PNG ", 3, FILE_OP_TYPE_IMAGE},
    {"TIF ", 3, FILE_OP_TYPE_IMAGE},

//other photo types specified by SONY DCF spec
#if (SONY_DCF_ENABLE)
    {"JPE ", 3, FILE_OP_TYPE_IMAGE},
    {"ARW ", 3, FILE_OP_TYPE_IMAGE},
    {"SRF ", 3, FILE_OP_TYPE_IMAGE},
    {"SR2 ", 3, FILE_OP_TYPE_IMAGE},
    {"JFI ", 3, FILE_OP_TYPE_IMAGE},
    {"THM ", 3, FILE_OP_TYPE_IMAGE}, /* .THM, thumbnail for SONY DCF */
#endif

//audio types
    {"MP3 ", 3, FILE_OP_TYPE_AUDIO},
    {"WMA ", 3, FILE_OP_TYPE_AUDIO},
    {"OGG ", 3, FILE_OP_TYPE_AUDIO},
    {"AAC ", 3, FILE_OP_TYPE_AUDIO},
    {"M4A ", 3, FILE_OP_TYPE_AUDIO},
    {"AC3 ", 3, FILE_OP_TYPE_AUDIO},
    {"WAV ", 3, FILE_OP_TYPE_AUDIO},
    {"RA  ", 2, FILE_OP_TYPE_AUDIO},
    {"RM  ", 2, FILE_OP_TYPE_AUDIO}, //RM file can be audio and video!?
    {"RAM ", 3, FILE_OP_TYPE_AUDIO},
    {"AMR ", 3, FILE_OP_TYPE_AUDIO},

//video types
    {"MPG ", 3, FILE_OP_TYPE_VIDEO},
    {"AVI ", 3, FILE_OP_TYPE_VIDEO},
    {"MOV ", 3, FILE_OP_TYPE_VIDEO},
    {"QT  ", 2, FILE_OP_TYPE_VIDEO},
    {"WMV ", 3, FILE_OP_TYPE_VIDEO},
    {"ASF ", 3, FILE_OP_TYPE_VIDEO},
    {"MP4 ", 3, FILE_OP_TYPE_VIDEO},
    {"3GP ", 3, FILE_OP_TYPE_VIDEO},
    {"MPE ", 3, FILE_OP_TYPE_VIDEO},
    {"FLV ", 3, FILE_OP_TYPE_VIDEO},
    {"TS  ", 2, FILE_OP_TYPE_VIDEO},
	{"MTS ", 3, FILE_OP_TYPE_VIDEO},
	{"M2T ", 3, FILE_OP_TYPE_VIDEO},//should be M2TS
    {"MKV ", 3, FILE_OP_TYPE_VIDEO},
    {"ASX ", 3, FILE_OP_TYPE_VIDEO},
    {"VOB ", 3, FILE_OP_TYPE_VIDEO},
    {"M2V ", 3, FILE_OP_TYPE_VIDEO},
	{"DAT ", 3, FILE_OP_TYPE_VIDEO},
#if EREADER_ENABLE
	{"DOC", 3, FILE_OP_TYPE_EBOOK},
	{"PDF", 3, FILE_OP_TYPE_EBOOK},
	{"EPUB", 3, FILE_OP_TYPE_EBOOK},
	{"PPM", 3, FILE_OP_TYPE_EBOOK},
	{"PGM", 3, FILE_OP_TYPE_EBOOK},
	{"TXT", 3, FILE_OP_TYPE_EBOOK},
	{"XML", 3, FILE_OP_TYPE_EBOOK},
  #if Make_UNRTF
    	{"RTF", 3, FILE_OP_TYPE_EBOOK},
  #endif
  #if Make_LRF
    	{"LRF", 3, FILE_OP_TYPE_EBOOK},
  #endif
#endif


//folder types
    {"??? ", 3, FILE_OP_TYPE_FOLDER},
    {"    ", 3, FILE_OP_TYPE_FOLDER},

//unknown, for end of this table
    {"----", 0, FILE_OP_TYPE_UNKNOWN}
};



///
///@ingroup FILE_BROWSER
///@brief   Get the corresponding media type value for a file extension name.
///
///@param   bExt   The pointer of ASCII string of the file extension name.
///
///@return  The media type value for the supported file extension name.\n
///         If not supported file extension name, it will return FILE_OP_TYPE_UNKNOWN.
///
WORD FileGetMediaType(const char * bExt)
{
    DWORD i, j;


    if ((bExt == NULL) || (strcmp(bExt, "") == 0))
        return FILE_OP_TYPE_UNKNOWN;

    for (i = 0; ; i++)
    {
        DWORD dwSize = file_ext_table[i].wSize;
        if (dwSize == 0)
            break;

        for (j = 0; j < dwSize; j++)
        {
            if (toupper(bExt[j]) != file_ext_table[i].bExt[j])
                break;
        }

        if (j >= dwSize)
        {
            return file_ext_table[i].wType;
        }
    }
    return FILE_OP_TYPE_UNKNOWN;
}



///
///@ingroup FILE
///@brief   Search a long-name file or directory in current directory by specified long file name.
///
///@param   drv            The drive to search file. \n\n
///@param   name           The pointer of long filename which is UTF-16 Unicode string. \n\n
///@param   dwNameLength   The length of the long filename, which is the count of UTF-16 characters. \n\n
///@param   entry_type     Specify the type (E_FILE_TYPE or E_DIR_TYPE or E_BOTH_FILE_AND_DIR_TYPE) of entry to search.
///
///@retval  FS_SUCCEED        File found. \n\n
///@retval  END_OF_DIR        File not found. \n\n
///@retval  ABNORMAL_STATUS   File search unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE     Invalid drive.
///
int FileSearchLN(DRIVE * drv, WORD * name, DWORD dwNameLength, ENTRY_TYPE entry_type)
{
    int i, LookUpFlag;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if ((name == NULL) || ((WORD) *name == 0x0000))
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if ((entry_type != E_FILE_TYPE) && (entry_type != E_DIR_TYPE) && (entry_type != E_BOTH_FILE_AND_DIR_TYPE))
    {
        MP_ALERT("%s: invalid entry_type (%d) specified !\n", __FUNCTION__, entry_type);
        return END_OF_DIR;
    }

    if (DirFirst(drv) != FS_SUCCEED)
    {
        MP_DEBUG("%s: file not found.\n", __FUNCTION__);
        return END_OF_DIR;
    }

    while (1)
    {
        LookUpFlag = 1;
        for (i = 0; i < dwNameLength; i++)
        {
            /* ignore char case difference, and not to modify original string */
            WORD ch1 = name[i], ch2 = drv->LongName[i];
            CharUpperCase16(&ch1); CharUpperCase16(&ch2);
            if (ch1 != ch2)
            {
                LookUpFlag = 0;
                break;
            }
        }

        if (LookUpFlag)
        {
            MP_DEBUG("%s: matched for beginning %d words => check an extra word: (name[%d] = 0x%x, drv->LongName[%d] = 0x%x)",
                      __FUNCTION__, dwNameLength, dwNameLength, name[dwNameLength], dwNameLength, drv->LongName[dwNameLength]);

            if ((WORD) drv->LongName[dwNameLength] != 0x0000) //long filename mismtach due to extra tailing word(s)
            {
                MP_DEBUG("%s: name mismatch after the %d-th (extra) words => continue searching...", __FUNCTION__, dwNameLength);
                goto L_next_node_LN;
            }

            /* check whether if File/Dir type mismatch with the FDB node */
            if (drv->Flag.FsType != FS_TYPE_exFAT)
            {
                if ( ((entry_type == E_FILE_TYPE) && (drv->Node->Attribute & FDB_SUB_DIR)) ||
                     ((entry_type == E_DIR_TYPE) && (!(drv->Node->Attribute & FDB_SUB_DIR))) )
                {
                    MP_ALERT("%s: name matched, but FILE/DIR type mismatch => continue searching...", __FUNCTION__);
                    goto L_next_node_LN;
                }
            }
#if EXFAT_ENABLE
            else
            {
                if ( ((entry_type == E_FILE_TYPE) && (drv->CacheBufPoint->fileAttributes & EXFAT_FDB_ATTRIB_DIR)) ||
                     ((entry_type == E_DIR_TYPE) && (!(drv->CacheBufPoint->fileAttributes & EXFAT_FDB_ATTRIB_DIR))) )
                {
                    MP_ALERT("%s: name matched, but FILE/DIR type mismatch => continue searching...", __FUNCTION__);
                    goto L_next_node_LN;
                }
            }
#endif

            MP_DEBUG("%s: file match => return FS_SUCCEED;\n", __FUNCTION__);
            return FS_SUCCEED;
        }

L_next_node_LN:
        if (DirNext(drv) != FS_SUCCEED)
        {
            MP_DEBUG("%s: file not found.\n", __FUNCTION__);
            return END_OF_DIR;
        }
    }

    MP_DEBUG("%s: file not found.\n", __FUNCTION__);
}



///
///@ingroup FILE
///@brief   Search a file or directory in current directory by specified primary name ASCII string and extension name ASCII string.
///
///@param   drv           The drive to search file. \n\n
///@param   name          The pointer of file name primary part, which is an ASCII string. \n\n
///@param   extension     The pointer of file name extension part, which is an ASCII string. \n\n
///@param   entry_type    Specify the type (E_FILE_TYPE or E_DIR_TYPE or E_BOTH_FILE_AND_DIR_TYPE) of entry to search.
///
///@retval  FS_SUCCEED        File found. \n\n
///@retval  END_OF_DIR        File not found. \n\n
///@retval  ABNORMAL_STATUS   File search unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE     Invalid drive.
///
///@remark     If the input primary name and/or extension name ASCII strings do not meet the 8.3 short filename length limit,
///            we internally convert the filename to UTF-16 string and invoke UTF-16 file search function FileSearchLN() to work.
///
int FileSearch(DRIVE * drv, BYTE * name, BYTE * extension, ENTRY_TYPE entry_type)
{
    int i, LookUpFlag;
    BYTE NameL, ExtenL;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if ((name == NULL) || (strcmp(name, "") == 0))
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if ((entry_type != E_FILE_TYPE) && (entry_type != E_DIR_TYPE) && (entry_type != E_BOTH_FILE_AND_DIR_TYPE))
    {
        MP_ALERT("%s: invalid entry_type (%d) specified !\n", __FUNCTION__, entry_type);
        return END_OF_DIR;
    }

    NameL = StringLength08(name);
    ExtenL = StringLength08(extension);

    if ( ((drv->Flag.FsType != FS_TYPE_exFAT) && ((NameL > 8) || (ExtenL > 3)))
#if EXFAT_ENABLE
         || (drv->Flag.FsType == FS_TYPE_exFAT)
#endif
       )
    {
        BYTE  ascii_full_name[MAX_L_NAME_LENG];
        WORD  utf16_full_name[MAX_L_NAME_LENG];
        WORD  fullname_len;
        int   ret;

        MpMemSet(ascii_full_name, 0, MAX_L_NAME_LENG);
        MpMemCopy(ascii_full_name, name, NameL);
        if (ExtenL)
        {
            ascii_full_name[NameL] = '.';
            MpMemCopy((BYTE *) ascii_full_name + (NameL + 1), extension, ExtenL);
        }

        fullname_len = (ExtenL)? (NameL + ExtenL + 1) : NameL;
        UpperCase08(ascii_full_name);

        MpMemSet((BYTE *) utf16_full_name, 0, MAX_L_NAME_LENG * sizeof(WORD));
        mpx_UtilUtf8ToUnicodeU16(utf16_full_name, ascii_full_name); /* convert to UTF-16 string */

        MP_DEBUG("%s: => FileSearchLN()...", __FUNCTION__);
        ret = FileSearchLN(drv, utf16_full_name, fullname_len, entry_type);
        if (ret != FS_SUCCEED)
        {
            MP_DEBUG("%s: file not found.\n", __FUNCTION__);
        }

        return ret;
    }
    else
    {
        if (DirFirst(drv) != FS_SUCCEED)
        {
            MP_DEBUG("%s: file not found.\n", __FUNCTION__);
            return END_OF_DIR;
        }

        while (1)
        {
            LookUpFlag = 1;

            for (i = 0; i < 8; i++)
            {
                if (drv->Node->Name[i] == 0x20) //0x20: '<Space>'
                {
                    if ((name[i] != 0x20) && (name[i] != 0x00)) //0x20: '<Space>';  0x00: 'Null char'
                    {
                        LookUpFlag = 0;
                    }
                    break;
                }

                if (toupper(name[i]) != toupper(drv->Node->Name[i])) /* ignore char case difference */
                {
                    LookUpFlag = 0;
                    break;
                }
            }

            if ((ExtenL == 0) && ((drv->Node->Extension[0] != 0x20) && (drv->Node->Extension[0] != 0x00)))  /* file extension name mismatch */
            {
                LookUpFlag = 0;
                goto L_next_node;
            }

            for (i = 0; (i < 3) && (LookUpFlag == 1); i++)
            {
                if (drv->Node->Extension[i] == 0x20) //0x20: '<Space>'
                {
                    if ((extension[i] != 0x20) && (extension[i] != 0x00)) //0x20: '<Space>';  0x00: 'Null char'
                    {
                        LookUpFlag = 0;
                    }
                    break;
                }

                if (toupper(extension[i]) != toupper(drv->Node->Extension[i])) /* ignore char case difference */
                {
                    LookUpFlag = 0;
                    break;
                }
            }

            if (LookUpFlag)
            {
                /* check whether if File/Dir type mismatch with the FDB node */
                if ( ((entry_type == E_FILE_TYPE) && (drv->Node->Attribute & FDB_SUB_DIR)) ||
                     ((entry_type == E_DIR_TYPE) && (!(drv->Node->Attribute & FDB_SUB_DIR))) )
                {
                    MP_ALERT("%s: name matched, but FILE/DIR type mismatch => continue searching...", __FUNCTION__);
                    goto L_next_node;
                }

                MP_DEBUG("%s: file match => return FS_SUCCEED;", __FUNCTION__);
                return FS_SUCCEED;
            }

    L_next_node:
            if (DirNext(drv) != FS_SUCCEED)
            {
                MP_DEBUG("%s: file not found.\n", __FUNCTION__);
                return END_OF_DIR;
            }
        }

        MP_DEBUG("%s: file not found.\n", __FUNCTION__);
    }
}



///
///@ingroup FILE
///@brief   Search/scan files on the drive for the file extension names specified in the array of file extension names.
///
///@param   sDrv               [IN] The drive to search. \n\n
///@param   pdwExtArray        [IN] The pointer of array of file extension names for searching.\n
///                                 The element format must be "XXX.". And the last element format must be "####". \n\n
///@param   sSearchInfo        [IN] The pointer of ST_SEARCH_INFO list array for searched information. \n\n
///@param   dwMaxElement       [IN] The max element of the searching result in the ST_SEARCH_INFO list. \n\n
///@param   pdwRecordElement   [OUT] Output value for the number of recorded elements in the searched file list. \n\n
///@param   bSearchType        [IN] There are four searching types (GLOBAL_SEARCH, LOCAL_RECURSIVE_SEARCH, LOCAL_SEARCH, LOCAL_SEARCH_INCLUDE_FOLDER) to
///                                 setup this parameter:\n
///                                   GLOBAL_SEARCH: searching all the files on the disk drive.\n
///                                   LOCAL_RECURSIVE_SEARCH: searching files under current directory, including all the
///                                                           the subdirectories under this directory.\n
///                                   LOCAL_SEARCH: searching the files under current directory.\n
///                                   LOCAL_SEARCH_INCLUDE_FOLDER: searching the files and folders under current directory. \n\n
///
///@param   bFileType          [IN] File type value. \n
///                                 If ('bFileType' > 0) => force to set file type of matched files as this 'bFileType' value. \n
///                                 If ('bFileType' == 0) => set file type of a matched file to the media type value for its file extension name. \n\n
///
///@param   bSortFlag          [IN] The flag value to enable/disable file sorting in the matched file list during the scanning process.\n
///                                 If ('bSortFlag' > 0) => enable file sorting. \n
///                                 If ('bSortFlag' == 0) => disable file sorting.
///
///@retval  FS_SUCCEED         File scan successfully. \n\n
///@retval  FS_SCAN_FAIL       File scan unsuccessfully. \n\n
///@retval  ABNORMAL_STATUS    File scan unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE      Invalid drive.
///
SWORD FileExtSearch(DRIVE * sDrv, DWORD * pdwExtArray, ST_SEARCH_INFO * sSearchInfo,
					DWORD dwMaxElement, DWORD * pdwRecordElement, BYTE bSearchType, BYTE bFileType, BYTE bSortFlag)
{
    CHAIN *sChain;
    SWORD swRetValue;
    DWORD dwBaseDeep, dwCount, i;
    DWORD dwExtension, *pdwNameSource, *pdwNameDest;
    DWORD maxFilesToScan;
    BOOL  f_Halt_for_CardOut = FALSE;
    ST_SEARCH_INFO *psSearchInfo;
    ST_SEARCH_INFO *working_FileList;
    BYTE bTemp, j, k;
#if EXFAT_ENABLE
    DWORD RootDir_size;
#endif
    FILE_SORTING_BASIS_TYPE  sorting_method;

#if (SONY_DCF_ENABLE)
    const BYTE  *DCF_RootDCIM_str = "/DCIM";
    BOOL  f_cannot_process_DCF = FALSE;
    BOOL  f_RoorDCIM_found = FALSE;
    DWORD DCF_RootDCIM_StartCluster = 0xFFFFFFFF;
    BOOL  f_under_RoorDCIM_Dir = FALSE;  /* flag for under the /DCIM directory */
    BOOL  f_under_RootDCIM_000AAAAA_Dir = FALSE;  /* flag for under a "/DCIM/000AAAAA"-format directory */
    SWORD RootDCIM_000AAAAA_Dir_DigitNum = -1;  /* for 3 digit chars range from "100" to "999" */
    BOOL  f_DCIM_sameDigit_Dir_conflict = FALSE;  /* flag for conflict of some other folder within /DCIM/ with same digit number */
    ST_SEARCH_INFO *tmp_FileList = NULL; /* temp file list for DCF files scanning and processing */
#endif

#if (MEASURE_FILE_SCANNING_SORTING_TIME)
    DWORD start_time, elapsed_time;

    MP_ALERT("***** [Stage 1] Begin FileExtSearch() searching media files ...");
    start_time = SystemGetTimeStamp();
#endif

    MP_DEBUG("Enter %s() ...", __FUNCTION__);
    if (sDrv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if ((pdwExtArray == NULL) || (sSearchInfo == NULL) || (pdwRecordElement == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (sDrv->StatusCode)
    {
        MP_ALERT("%s: Error! file system status of the drive is abnormal !", __FUNCTION__);
        return FS_SCAN_FAIL;
    }

    if (!sDrv->Flag.Present)
    {
        MP_ALERT("%s: Error! Card not present !", __FUNCTION__);
        return FS_SCAN_FAIL;
    }

    working_FileList = (ST_SEARCH_INFO *) sSearchInfo;  /* by default, work directly on the input SearchInfo file list */

#if (SONY_DCF_ENABLE)
    g_DCF_Obj_TotalCnt = 0;

    /* SONY DCF rules only apply on Photo/Image files */
    if (g_psSystemConfig->dwCurrentOpMode != OP_IMAGE_MODE)
    {
        MP_DEBUG("%s: not in OP_IMAGE_MODE mode, disable DCF processing.", __FUNCTION__);
        f_cannot_process_DCF = TRUE;
        goto L_start_scanning;
    }

    if (! f_cannot_process_DCF)
    {
        tmp_FileList = (ST_SEARCH_INFO *) ext_mem_malloc(TEMP_FILE_LIST_MAX_ENTRIES_COUNT * sizeof(ST_SEARCH_INFO));
        if (tmp_FileList == NULL)
        {
            MP_ALERT("%s: -E- mem alloc failed for temp file list => disable DCF processing !!", __FUNCTION__);
            f_cannot_process_DCF = TRUE;
            goto L_start_scanning;
        }
        else
            MpMemSet((BYTE *) tmp_FileList, 0, (TEMP_FILE_LIST_MAX_ENTRIES_COUNT * sizeof(ST_SEARCH_INFO)));

        /* prepare the dynamically allocated list of SONY DCF info corresponding to each entry of file list */
        if (g_FileList_DCF_Info_List == NULL)
        {
            g_FileList_DCF_Info_List = (DCF_INFO_TYPE *) ext_mem_malloc(TEMP_FILE_LIST_MAX_ENTRIES_COUNT * sizeof(DCF_INFO_TYPE));
            if (g_FileList_DCF_Info_List == NULL)
            {
                MP_ALERT("%s: -E- mem alloc failed for DCF Info list => disable DCF processing !!", __FUNCTION__);
                f_cannot_process_DCF = TRUE;
                goto L_start_scanning;
            }
            else
                MpMemSet((BYTE *) g_FileList_DCF_Info_List, 0, (TEMP_FILE_LIST_MAX_ENTRIES_COUNT * sizeof(DCF_INFO_TYPE)));
        }
        else //clear old DCF info list first.
        {
            //note: how about FOLDER enabled (SEARCH_TYPE not GLOBAL_SEARCH) case ??
            MpMemSet((BYTE *) g_FileList_DCF_Info_List, 0, (TEMP_FILE_LIST_MAX_ENTRIES_COUNT * sizeof(DCF_INFO_TYPE)));
        }

        MpMemSet((BYTE *) g_DCIM_000AAAAA_Dir_Digits_Bitmap, 0, DCIM_000AAAAA_DIR_DIGITS_BITMAP_LEN);

        /* find Root directory "/DCIM" of SONY DCF objects */
        MP_ALERT("\r\n%s: check whether if [%s] exist, for SONY DCF objects ...", __FUNCTION__, DCF_RootDCIM_str);

        E_DRIVE_INDEX_ID  new_drv_id;
        PATH_TARGET_TYPE  found_entry_type;
        DWORD  final_token_offset, final_token_strlen;
        if (PathAPI__Locate_Path(DCF_RootDCIM_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
        {
            MP_ALERT("%s: [%s] not found, no need DCF processing. \r\n", __FUNCTION__, DCF_RootDCIM_str);
            f_RoorDCIM_found = FALSE;
            f_cannot_process_DCF = TRUE;
        }
        else
        {
            MP_ALERT("%s: [%s] found, start scanning DCF objects ... \r\n", __FUNCTION__, DCF_RootDCIM_str);
            f_RoorDCIM_found = TRUE;
            sChain = (CHAIN *) ((DWORD) sDrv->DirStackBuffer + sDrv->DirStackPoint);
            DCF_RootDCIM_StartCluster = sChain->Start;
            working_FileList = (ST_SEARCH_INFO *) tmp_FileList;  /* work on the temp file list */
        }
    }

    g_WorkingFileList_BasePtr = working_FileList;
    g_WorkingFileList_TotalCount = 0;
#endif


L_start_scanning:
    maxFilesToScan = dwMaxElement;
    g_Sorting_Start_Index_in_Companion_Info_List = 0;

#if (SONY_DCF_ENABLE)
    if (f_cannot_process_DCF)
    {
        maxFilesToScan = MIN(FILE_LIST_SIZE, dwMaxElement);

        /* no need DCF processing => release allocated memory */
        if (tmp_FileList)
        {
            ext_mem_free(tmp_FileList);
            tmp_FileList = NULL;
        }
        if (g_FileList_DCF_Info_List)
        {
            ext_mem_free(g_FileList_DCF_Info_List);
            g_FileList_DCF_Info_List = NULL;
        }
    }
    else
        maxFilesToScan = TEMP_FILE_LIST_MAX_ENTRIES_COUNT;
#endif


#if USBOTG_HOST_PTP
    if ((sDrv->DevID == DEV_USB_HOST_PTP) || (sDrv->DevID == DEV_USBOTG1_HOST_PTP)) //(sDrv->DevID == DEV_USB_HOST_PTP)
    {
        MP_DEBUG("%s: USB PTP => UsbhStorage_PtpScanObject() ...", __FUNCTION__);
        UsbhStorage_PtpScanObject ( (DWORD)pdwExtArray,
                                    (DWORD)pdwRecordElement,
                                    working_FileList,
                                    maxFilesToScan,
                                    GetWhichUsbOtgByCardId(sDrv->DevID));
        swRetValue = FS_SUCCEED;
        goto L_File_Sorting;
    }
#endif

#if EXFAT_ENABLE
    RootDir_size = GetRootDirSize(sDrv);
#endif

    if (bSearchType & GLOBAL_SEARCH)
    {
        //reset all information of the "sDrv"
        if (DirReset(sDrv))
        {
            MP_ALERT("%s: -E- DirReset() back to beginning of root directory failed !", __FUNCTION__);
            return FS_SCAN_FAIL;
        }
    }
    else if ((bSearchType & LOCAL_RECURSIVE_SEARCH) || (bSearchType & LOCAL_SEARCH) || (bSearchType & LOCAL_SEARCH_INCLUDE_FOLDER))
    {
        if (!sDrv->DirStackPoint) /* in RootDir */
        {
            if (DirReset(sDrv))
            {
                MP_ALERT("%s: -E- DirReset() back to beginning of root directory failed !", __FUNCTION__);
                return FS_SCAN_FAIL;
            }
        }
    }
    else /* unknown search type */
    {
        MP_ALERT("%s: Error! Unknown search type ! bSearchType = 0x%02x", __FUNCTION__, (BYTE) bSearchType);
        return FS_SCAN_FAIL;
    }

    dwBaseDeep = sDrv->DirStackPoint;
    dwCount = 0;

    if (bSearchType & LOCAL_SEARCH_INCLUDE_FOLDER)
    {
        if (sDrv->DirStackPoint)
        {
            psSearchInfo = &working_FileList[dwCount];
            MpMemSet(psSearchInfo, 0x0, sizeof(ST_SEARCH_INFO));

            /* pseudo entry for Upper Folder usage, FAT12/16/32 and exFAT all need this */
            MpMemSet(psSearchInfo->bName, 0x20, 8);
            psSearchInfo->bName[0] = '.';
            psSearchInfo->bName[1] = '.';
            MpMemSet(psSearchInfo->bExt, '?', 3);

            psSearchInfo->bParameter = SEARCH_INFO_CHANGE_PATH;
            psSearchInfo->bFileType = FILE_OP_TYPE_FOLDER;
            psSearchInfo->dwMediaInfo = 0;
            psSearchInfo->FsType = sDrv->Flag.FsType;
            psSearchInfo->DrvIndex = sDrv->DrvIndex;

        #if EXFAT_ENABLE
            if (psSearchInfo->FsType == FS_TYPE_exFAT)
            {
                psSearchInfo->exfat_NameHash = 0; /* no real hash value for pseudo entry */
                psSearchInfo->exfat_DirEntryFlags.f_AllocPossible = 1; //not important here ??
                psSearchInfo->exfat_DirEntryFlags.f_NoFatChain = 1; //not important here ??
            }
        #endif

        #if (SONY_DCF_ENABLE)
            if ((! f_cannot_process_DCF) && (g_FileList_DCF_Info_List != NULL))
            {
                /* this entry is a folder => not DCF obj */
                g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
            }
        #endif

            /* next entry */
            dwCount++;
        }
    }

    swRetValue = DirFirst(sDrv);
    if (swRetValue)
    {
        *pdwRecordElement = dwCount;
        return FS_SUCCEED;
    }

    while (swRetValue == FS_SUCCEED)
    {
        if (!sDrv->Flag.Present)
        {
            MP_ALERT("%s: Error! Card not present !", __FUNCTION__);
            return FS_SCAN_FAIL;
        }
        if (sDrv->StatusCode)
        {
            MP_ALERT("%s: Error! file system status of the drive is abnormal !", __FUNCTION__);
            return FS_SCAN_FAIL;
        }

        if (sDrv->Flag.FsType != FS_TYPE_exFAT)
        {
            /* note: Ignore all hidden files and folders for Fuji project */
        #if (EXCLUDE_HIDDEN_FILES_FOLDERS_FOR_FILE_LIST)
            if (sDrv->Node->Attribute & FDB_HIDDEN)
            {
                goto L_Scan_Next_DirEntry;
            }
        #endif

            if (sDrv->Node->Attribute & FDB_SUB_DIR)
            {
                if (((bSearchType & LOCAL_SEARCH) == 0) && ((bSearchType & LOCAL_SEARCH_INCLUDE_FOLDER) == 0))
                {
        #if (SONY_DCF_ENABLE)
                    if ((! f_cannot_process_DCF) && f_RoorDCIM_found && f_under_RoorDCIM_Dir && (sDrv->DirStackPoint == sizeof(CHAIN)))
                    {
                        /* currently in /DCIM, check if sub-directory name is 000AAAAA format. If so, extract its digit number */
                        RootDCIM_000AAAAA_Dir_DigitNum = Extract_DigitNum_of_RootDCIM_000AAAAA_Dir((BYTE *) sDrv->Node->Name);
                        if ((RootDCIM_000AAAAA_Dir_DigitNum >= 100) && (RootDCIM_000AAAAA_Dir_DigitNum <= 999))
                        {
                            f_under_RootDCIM_000AAAAA_Dir = TRUE;
                            f_DCIM_sameDigit_Dir_conflict = FALSE; /* initial value before checking if any conflict same digit number */
                            MP_DEBUG("%s: before entering a /DCIM/000AAAAA format sub-directory ...", __FUNCTION__);

                            /* check whether if /DCIM/000AAAAA directories with same digit number */
                            if ((! f_cannot_process_DCF) && (g_FileList_DCF_Info_List != NULL))
                            {
                                BYTE alloc_bit;

                                alloc_bit = Read_DCIM_000AAAAA_Dir_BitmapEntry(g_DCIM_000AAAAA_Dir_Digits_Bitmap, RootDCIM_000AAAAA_Dir_DigitNum);
                                if (alloc_bit == 0xFF)
                                {
                                    MP_ALERT("%s: Read_DCIM_000AAAAA_Dir_BitmapEntry() failed => check source code for debugging !!\r\n");
                                }
                                else if (alloc_bit == 0x01) /* same directory digit number recorded in the bitmap table */
                                {
                                    f_DCIM_sameDigit_Dir_conflict = TRUE;
                                    MP_DEBUG("%s: current /DCIM/000AAAAA directory has same digit number conflicted with other directory !", __FUNCTION__);

                                    /* update current DCF info list entries with conflict directory digit number */
                                    MP_DEBUG("%s: update DCF info list entries with conflict directory digit number ...", __FUNCTION__);
                                    for (i=0; (i < dwCount) && (i <= TEMP_FILE_LIST_MAX_ENTRIES_COUNT - 1); i++)
                                    {
                                        if ( (g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj) && (! g_FileList_DCF_Info_List[i].Flags.f_final_confirmed) &&
                                             (g_FileList_DCF_Info_List[i].RootDCIM_000AAAAA_Dir_DigitNum == RootDCIM_000AAAAA_Dir_DigitNum) )
                                        {
                                            g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                            g_FileList_DCF_Info_List[i].Flags.f_final_confirmed = 1;
                                        }
                                    }
                                }
                                else if (alloc_bit == 0x00)
                                {
                                    /* record this directory digit number in the bitmap table */
                                    if (Set_DCIM_000AAAAA_Dir_BitmapEntry(g_DCIM_000AAAAA_Dir_Digits_Bitmap, RootDCIM_000AAAAA_Dir_DigitNum, 1) != FS_SUCCEED)
                                    {
                                        MP_ALERT("%s: Set_DCIM_000AAAAA_Dir_BitmapEntry() failed => check source code for debugging !!\r\n");
                                    }
                                }
                            }
                        }
                    }
        #endif

                    BYTE pri_name[9], ext_name[4];
                    MpMemCopy(pri_name, (BYTE *) sDrv->Node->Name, 8);
                    pri_name[8] = 0;
                    if (sDrv->Node->Extension[0] != ' ')
                    {
                        MpMemCopy(ext_name, (BYTE *) sDrv->Node->Extension, 3);
                        ext_name[3] = 0;
                        MP_DEBUG("%s: enter the sub-directory (short name = [%s.%s]) ...", __FUNCTION__, pri_name, ext_name);
                    }
                    else
                    {
                        MP_DEBUG("%s: enter the sub-directory (short name = [%s]) ...", __FUNCTION__, pri_name);
                    }

                    /* enter the sub-directory */
                    if ((swRetValue = CdSub(sDrv)))
                    {
                        if (swRetValue != FS_DIR_STACK_LIMIT_REACHED)
                        {
                            MP_ALERT("%s: Error! CdSub() failed !", __FUNCTION__);
                            return swRetValue;
                        }
                        else
                            goto L_Scan_Next_DirEntry;
                    }

        #if (SONY_DCF_ENABLE)
                    if (! f_cannot_process_DCF)
                    {
                        sChain = (CHAIN *) ((DWORD) sDrv->DirStackBuffer + sizeof(CHAIN));
                        if (f_RoorDCIM_found && (sDrv->DirStackPoint == sizeof(CHAIN)) && (sChain->Start == DCF_RootDCIM_StartCluster))
                        {
                            f_under_RoorDCIM_Dir = TRUE;
                            MP_DEBUG("%s: Just entered the %s folder.", __FUNCTION__, DCF_RootDCIM_str);
                        }
                    }
        #endif
                }

                if (bSearchType & LOCAL_SEARCH_INCLUDE_FOLDER)
                {
                    psSearchInfo = &working_FileList[dwCount];

                    //copy cluster and cluster offset
                    sChain = (CHAIN *) ((DWORD) sDrv->DirStackBuffer + sDrv->DirStackPoint);
                    psSearchInfo->dwDirStart = sChain->Start;	// abel add for file copy and delete
                    psSearchInfo->dwDirPoint = sDrv->DirCachePoint;
                    psSearchInfo->dwFdbOffset = (sChain->Point >> 5);	//change byte to fdb offset (32 Bytes)
                    psSearchInfo->dwLongFdbCount = sDrv->CacheBufPoint->LongNameFdbCount;
                    psSearchInfo->bParameter = SEARCH_INFO_FOLDER;
                    psSearchInfo->bFileType = FILE_OP_TYPE_FOLDER;
                    psSearchInfo->dwMediaInfo = 0;
                    psSearchInfo->FsType = sDrv->Flag.FsType;
                    psSearchInfo->DrvIndex = sDrv->DrvIndex;

                #if EXFAT_ENABLE
                    if (!sDrv->DirStackPoint) /* in RootDir */
                        psSearchInfo->dwParentDirSize = RootDir_size;
                    else
                        psSearchInfo->dwParentDirSize = sChain->Size;
                #endif

                    //copy file name
                    for (j = 0; j < 8; j++)
                    {
                        bTemp = sDrv->Node->Name[j];
                        if (((sDrv->Node->Attribute2) >> 8) & NAME_LOWER_CASE)
                        {
                            if ((bTemp >= 0x41) && (bTemp <= 0x5a))	// change upper case to lower case
                                bTemp += 0x20;
                        }

                        psSearchInfo->bName[j] = bTemp;
                    }

                    //copy file ext
                    for (j = 0; j < 3; j++)
                    {
                        bTemp = sDrv->Node->Extension[j];
                        if (((sDrv->Node->Attribute2) >> 8) & EXTNAME_LOWER_CASE)
                        {
                            if ((bTemp >= 0x41) && (bTemp <= 0x5a))	// change upper case to lower case
                                bTemp += 0x20;
                        }

                        psSearchInfo->bExt[j] = bTemp;
                    }

                    psSearchInfo->dwFileSize = 0; //directory

                    /* get file modify date/time */
                    FileGetDate_for_FATxx(sDrv->Node->ModifyDate, &psSearchInfo->DateTime.year, &psSearchInfo->DateTime.month, &psSearchInfo->DateTime.day);
                    FileGetTime_for_FATxx(sDrv->Node->ModifyTime, &psSearchInfo->DateTime.hour, &psSearchInfo->DateTime.minute, &psSearchInfo->DateTime.second);
                    psSearchInfo->DateTime.UtcOffset = 0;

            #if (SONY_DCF_ENABLE)
                    if ((! f_cannot_process_DCF) && (g_FileList_DCF_Info_List != NULL))
                    {
                        /* this entry is a folder => not DCF obj */
                        g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                        g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                    }
            #endif

                    /* next entry */
                    dwCount++;

                    if (dwCount >= maxFilesToScan)
                    {
                        *pdwRecordElement = dwCount;
                        swRetValue = FS_SUCCEED;
                        goto L_Post_Process_Before_File_Sorting;
                    }
                }
            }
            else
            {
                dwExtension = (((*(DWORD *) & sDrv->Node->Extension[0]) & 0xffffff00) | 0x2e);
                i = 0;
                while (pdwExtArray[i] != EXT_END)
                {
                    if (!sDrv->Flag.Present)
                    {
                        MP_ALERT("%s: Error! Card not present !", __FUNCTION__);
                        return FS_SCAN_FAIL;
                    }
                    if (sDrv->StatusCode)
                    {
                        MP_ALERT("%s: Error! file system status of the drive is abnormal !", __FUNCTION__);
                        return FS_SCAN_FAIL;
                    }

                    if (((dwExtension == pdwExtArray[i]) || (pdwExtArray[i] == EXT_ALL))&& Weld_CheckTimeByFileName(&sDrv->Node->Name[0]))
                    {
                        psSearchInfo = &working_FileList[dwCount];
                        //copy cluster and cluster offset
                        sChain = (CHAIN *) ((DWORD) sDrv->DirStackBuffer + sDrv->DirStackPoint);
                        psSearchInfo->dwDirStart = sChain->Start;	// abel add for file copy and delete
                        psSearchInfo->dwDirPoint = sDrv->DirCachePoint;
                        psSearchInfo->dwFdbOffset = (sChain->Point >> 5);	//change byte to fdb offset (32 Bytes)
                        psSearchInfo->dwLongFdbCount = sDrv->CacheBufPoint->LongNameFdbCount;
                        psSearchInfo->bParameter = SEARCH_INFO_FILE;
                        psSearchInfo->dwMediaInfo = 0;
                        psSearchInfo->FsType = sDrv->Flag.FsType;
                        psSearchInfo->DrvIndex = sDrv->DrvIndex;

                    #if EXFAT_ENABLE
                        if (!sDrv->DirStackPoint) /* in RootDir */
                            psSearchInfo->dwParentDirSize = RootDir_size;
                        else
                            psSearchInfo->dwParentDirSize = sChain->Size;
                    #endif

                        //copy file name
                        for (j = 0; j < 8; j++)
                        {
                            bTemp = sDrv->Node->Name[j];
                            if (((sDrv->Node->Attribute2) >> 8) & NAME_LOWER_CASE)
                            {
                                if ((bTemp >= 0x41) && (bTemp <= 0x5a))	// change upper case to lower case
                                    bTemp += 0x20;
                            }

                            psSearchInfo->bName[j] = bTemp;
                        }

                        if (psSearchInfo->bName[0] == 0x05)		// abel 20090304 add
                            psSearchInfo->bName[0] = 0xE5;

                        for (j = 0; j < 3; j++)
                        {
                            bTemp = sDrv->Node->Extension[j];
                            if (((sDrv->Node->Attribute2) >> 8) & EXTNAME_LOWER_CASE)
                            {
                                if ((bTemp >= 0x41) && (bTemp <= 0x5a))	// change upper case to lower case
                                    bTemp += 0x20;
                            }

                            psSearchInfo->bExt[j] = bTemp;
                        }

                        if (bFileType != 0)
                            psSearchInfo->bFileType = bFileType;
                        else
                            psSearchInfo->bFileType = FileGetMediaType(psSearchInfo->bExt);

                        psSearchInfo->dwFileSize = (DWORD) LoadAlien32(&sDrv->Node->Size); //file size

                        /* get file modify date/time */
                        FileGetDate_for_FATxx(sDrv->Node->ModifyDate, &psSearchInfo->DateTime.year, &psSearchInfo->DateTime.month, &psSearchInfo->DateTime.day);
                        FileGetTime_for_FATxx(sDrv->Node->ModifyTime, &psSearchInfo->DateTime.hour, &psSearchInfo->DateTime.minute, &psSearchInfo->DateTime.second);
                        psSearchInfo->DateTime.UtcOffset = 0;

                #if (SONY_DCF_ENABLE)
                        BYTE pri_name[9];
                        MpMemCopy(pri_name, (BYTE *) psSearchInfo->bName, 8);
                        pri_name[8] = 0;

                        if ((f_cannot_process_DCF) || (g_FileList_DCF_Info_List == NULL))
                            ; /* no memory for DCF Info list => do nothing w.r.t DCF info */
                        else if ((! f_RoorDCIM_found) || (! f_under_RoorDCIM_Dir) || (! f_under_RootDCIM_000AAAAA_Dir))
                        {
                            MP_DEBUG("index %u: (%s.%s) => not under a /DCIM/000AAAAA format directory. \r\n", dwCount, pri_name, psSearchInfo->bExt);
                            g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                            g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                        }
                        else
                        {
                            if (sDrv->DirStackPoint != (2 * sizeof(CHAIN)))
                            {
                                /* file not in exact "/DCIM/000AAAAA" directory level => not a DCF obj */
                                MP_DEBUG("index %u: (%s.%s) => not in an exact /DCIM/000AAAAA format directory. \r\n", dwCount, pri_name, psSearchInfo->bExt);
                                g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                                g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                            }
                            else
                            {
                                /* file in exact a "/DCIM/000AAAAA" format directory */
                                MP_DEBUG("index %u: (%s.%s) => in an exact /DCIM/000AAAAA format directory.", dwCount, pri_name, psSearchInfo->bExt);

                                /* check to make sure */
                                sChain = (CHAIN *) ((DWORD) sDrv->DirStackBuffer + sizeof(CHAIN));
                                if (sChain->Start != DCF_RootDCIM_StartCluster)
                                {
                                    MP_ALERT("%s: Error ! Something wrong/mismatch in the drive's DirStack !  check file system code ! \r\n", __FUNCTION__);

                                    /* unreasonable case, treat it as non-DCF cases */
                                    g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                                    g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                }
                                else
                                {
                                    if (f_DCIM_sameDigit_Dir_conflict)
                                    {
                                        /* /DCIM/000AAAAA same digit number conflict with other directory => not a DCF obj */
                                        MP_DEBUG(" => index %u: /DCIM/000AAAAA same digit number conflict with other directory => not a DCF obj. \r\n", dwCount);
                                        g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                                        g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                    }
                                    else
                                    {
                                        int digitNum = Extract_DigitNum_of_DCF_Obj((BYTE *) psSearchInfo->bName);
                                        if ((digitNum >= 0) && (digitNum <= 99999))
                                        {
                                            g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 0; /* to be finally confirmed in next stage */
                                            g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 1;

                                            /* record the digit number of this file's parent directory (/DCIM/000AAAAA) */
                                            g_FileList_DCF_Info_List[dwCount].RootDCIM_000AAAAA_Dir_DigitNum = RootDCIM_000AAAAA_Dir_DigitNum;
                                            MP_DEBUG(" => digit number of parent directory = %d", g_FileList_DCF_Info_List[dwCount].RootDCIM_000AAAAA_Dir_DigitNum);

                                            BYTE  output_name_buffer[9];
                                            DWORD output_display_integer = 0;
                                            if (Makeup_DCF_Obj_DisplayName(psSearchInfo->bName, dwCount, output_name_buffer, 9, &output_display_integer) == PASS)
                                            {
                                                MP_DEBUG(" => a possible DCF obj, displayed name = %s.%s (merged integer = %07lu) \r\n", output_name_buffer, psSearchInfo->bExt, output_display_integer);
                                                g_FileList_DCF_Info_List[dwCount].merged_display_integer = output_display_integer;

                                                /* check whether if DCF objects with same displayed number */
                                                for (i=0; (i < dwCount) && (i <= TEMP_FILE_LIST_MAX_ENTRIES_COUNT - 1); i++)
                                                {
                                                    if ( (g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj) && (! g_FileList_DCF_Info_List[i].Flags.f_final_confirmed) &&
                                                         (g_FileList_DCF_Info_List[i].merged_display_integer == output_display_integer) )
                                                    {
                                                        if (StringNCompare08_CaseInsensitive(psSearchInfo->bExt, working_FileList[i].bExt, 3) == E_COMPARE_EQUAL)
                                                        {
                                                            MP_DEBUG(" => AAA00000 file has same displayed number and same extension filename conflicted with other file (index %u) !", i);

                                                        //To-confirm: which rule should be applied ??
                                                        #if 1
                                                            /* only .JPG and .THM files should apply this DCF rule */
                                                            if ( (StringNCompare08_CaseInsensitive(psSearchInfo->bExt, "JPG", 3) == E_COMPARE_EQUAL) || (StringNCompare08_CaseInsensitive(psSearchInfo->bExt, "THM", 3) == E_COMPARE_EQUAL) )
                                                            {
                                                                MP_DEBUG(" => they are both .JPG or both .THM files => both are not DCF objects. \r\n");
                                                                g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                                g_FileList_DCF_Info_List[i].Flags.f_final_confirmed = 1;

                                                                g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                                g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                                            }
                                                        #else
                                                            /* all DCF essential object types apply this DCF rule */
                                                            g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                            g_FileList_DCF_Info_List[i].Flags.f_final_confirmed = 1;

                                                            g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                            g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                                        #endif
                                                        }
                                                        else
                                                        {
                                                            MP_DEBUG(" => AAA00000 file has same displayed number but different extension filename with other file (index %u).", i);

                                                            /* check whether if .JPG and .THM have same file names */
                                                            if ( (StringNCompare08_CaseInsensitive(psSearchInfo->bName, working_FileList[i].bName, 8) == E_COMPARE_EQUAL) &&
                                            	                 (((StringNCompare08_CaseInsensitive(psSearchInfo->bExt, "JPG", 3) == E_COMPARE_EQUAL) && (StringNCompare08_CaseInsensitive(working_FileList[i].bExt, "THM", 3) == E_COMPARE_EQUAL)) ||
                                                                  ((StringNCompare08_CaseInsensitive(psSearchInfo->bExt, "THM", 3) == E_COMPARE_EQUAL) && (StringNCompare08_CaseInsensitive(working_FileList[i].bExt, "JPG", 3) == E_COMPARE_EQUAL))) )
                                                            {
                                                                MP_DEBUG(" => .JPG and .THM files (index %u and %u) have same file names => both are not DCF objects. \r\n", i, dwCount);
                                                                g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                                g_FileList_DCF_Info_List[i].Flags.f_final_confirmed = 1;

                                                                g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                                g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                MP_ALERT("Makeup_DCF_Obj_DisplayName() failed => check source code for debugging !!\r\n");
                                            }
                                        }
                                        else
                                        {
                                            /* filename not AAA00000 format => not a DCF obj */
                                            MP_DEBUG(" => index %u: filename not AAA00000 format => not a DCF obj. \r\n", dwCount);
                                            g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                                            g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                        }
                                    }
                                }
                            }
                        }
                #endif

                        /* next entry */
                        dwCount++;

                        if (dwCount >= maxFilesToScan)
                        {
                            *pdwRecordElement = dwCount;
                            swRetValue = FS_SUCCEED;
                            goto L_Post_Process_Before_File_Sorting;
                        }

                        break;  /* EXT_ALL for scanning extension filename was reached */
                    }
                    i++;
                }
            }
        }
#if EXFAT_ENABLE
        else
        {
            /* note: Ignore all hidden files and folders for Fuji project */
        #if (EXCLUDE_HIDDEN_FILES_FOLDERS_FOR_FILE_LIST)
            if (sDrv->CacheBufPoint->fileAttributes & EXFAT_FDB_ATTRIB_HIDDEN)
            {
                goto L_Scan_Next_DirEntry;
            }
        #endif

            if (sDrv->CacheBufPoint->fileAttributes & EXFAT_FDB_ATTRIB_DIR)
            {
                if (((bSearchType & LOCAL_SEARCH) == 0) && ((bSearchType & LOCAL_SEARCH_INCLUDE_FOLDER) == 0))
                {
        #if (SONY_DCF_ENABLE)
                    if ((! f_cannot_process_DCF) && f_RoorDCIM_found && f_under_RoorDCIM_Dir && (sDrv->DirStackPoint == sizeof(CHAIN)))
                    {
                        /* currently in /DCIM, check if sub-directory name is 000AAAAA format. If so, extract its digit number */

                        /* To-Do: must re-work this code in exFAT case, because exFAT DirEntry definitions changed */
                        //RootDCIM_000AAAAA_Dir_DigitNum = Extract_DigitNum_of_RootDCIM_000AAAAA_Dir((BYTE *) sDrv->Node->Name);
                        RootDCIM_000AAAAA_Dir_DigitNum = -1; //temp make it failed for DCF judgement
                        if ((RootDCIM_000AAAAA_Dir_DigitNum >= 100) && (RootDCIM_000AAAAA_Dir_DigitNum <= 999))
                        {
                            f_under_RootDCIM_000AAAAA_Dir = TRUE;
                            f_DCIM_sameDigit_Dir_conflict = FALSE; /* initial value before checking if any conflict same digit number */
                            MP_DEBUG("%s: before entering a /DCIM/000AAAAA format sub-directory ...", __FUNCTION__);

                            /* check whether if /DCIM/000AAAAA directories with same digit number */
                            if ((! f_cannot_process_DCF) && (g_FileList_DCF_Info_List != NULL))
                            {
                                BYTE alloc_bit;

                                alloc_bit = Read_DCIM_000AAAAA_Dir_BitmapEntry(g_DCIM_000AAAAA_Dir_Digits_Bitmap, RootDCIM_000AAAAA_Dir_DigitNum);
                                if (alloc_bit == 0xFF)
                                {
                                    MP_ALERT("%s: Read_DCIM_000AAAAA_Dir_BitmapEntry() failed => check source code for debugging !!\r\n");
                                }
                                else if (alloc_bit == 0x01) /* same directory digit number recorded in the bitmap table */
                                {
                                    f_DCIM_sameDigit_Dir_conflict = TRUE;
                                    MP_DEBUG("%s: current /DCIM/000AAAAA directory has same digit number conflicted with other directory !", __FUNCTION__);

                                    /* update current DCF info list entries with conflict directory digit number */
                                    MP_DEBUG("%s: update DCF info list entries with conflict directory digit number ...", __FUNCTION__);
                                    for (i=0; (i < dwCount) && (i <= TEMP_FILE_LIST_MAX_ENTRIES_COUNT - 1); i++)
                                    {
                                        if ( (g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj) && (! g_FileList_DCF_Info_List[i].Flags.f_final_confirmed) &&
                                             (g_FileList_DCF_Info_List[i].RootDCIM_000AAAAA_Dir_DigitNum == RootDCIM_000AAAAA_Dir_DigitNum) )
                                        {
                                            g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                            g_FileList_DCF_Info_List[i].Flags.f_final_confirmed = 1;
                                        }
                                    }
                                }
                                else if (alloc_bit == 0x00)
                                {
                                    /* record this directory digit number in the bitmap table */
                                    if (Set_DCIM_000AAAAA_Dir_BitmapEntry(g_DCIM_000AAAAA_Dir_Digits_Bitmap, RootDCIM_000AAAAA_Dir_DigitNum, 1) != FS_SUCCEED)
                                    {
                                        MP_ALERT("%s: Set_DCIM_000AAAAA_Dir_BitmapEntry() failed => check source code for debugging !!\r\n");
                                    }
                                }
                            }
                        }
                    }
        #endif

                    MP_DEBUG("%s: enter the sub-directory ...", __FUNCTION__);
                    /* enter the sub-directory */
                    if ((swRetValue = CdSub(sDrv)))
                    {
                        if (swRetValue != FS_DIR_STACK_LIMIT_REACHED)
                        {
                            MP_ALERT("%s: Error! CdSub() failed !", __FUNCTION__);
                            return swRetValue;
                        }
                        else
                            goto L_Scan_Next_DirEntry;
                    }

        #if (SONY_DCF_ENABLE)
                    if (! f_cannot_process_DCF)
                    {
                        sChain = (CHAIN *) ((DWORD) sDrv->DirStackBuffer + sizeof(CHAIN));
                        if (f_RoorDCIM_found && (sDrv->DirStackPoint == sizeof(CHAIN)) && (sChain->Start == DCF_RootDCIM_StartCluster))
                        {
                            f_under_RoorDCIM_Dir = TRUE;
                            MP_DEBUG("%s: Just entered the %s folder.", __FUNCTION__, DCF_RootDCIM_str);
                        }
                    }
        #endif
                }

                if (bSearchType & LOCAL_SEARCH_INCLUDE_FOLDER)
                {
                    psSearchInfo = &working_FileList[dwCount];

                    //copy cluster and cluster offset
                    sChain = (CHAIN *) ((DWORD) sDrv->DirStackBuffer + sDrv->DirStackPoint);
                    psSearchInfo->dwDirStart = sChain->Start;	// abel add for file copy and delete
                    psSearchInfo->dwDirPoint = sDrv->DirCachePoint;
                    psSearchInfo->dwFdbOffset = (sChain->Point >> 5);	//change byte to fdb offset (32 Bytes)
                    psSearchInfo->dwLongFdbCount = sDrv->CacheBufPoint->LongNameFdbCount;
                    psSearchInfo->dwMediaInfo = 0;
                    psSearchInfo->FsType = sDrv->Flag.FsType;
                    psSearchInfo->DrvIndex = sDrv->DrvIndex;
                    psSearchInfo->exfat_DirEntryFlags.f_AllocPossible = sDrv->CacheBufPoint->exfat_DirEntryFlags.f_AllocPossible;
                    psSearchInfo->exfat_DirEntryFlags.f_NoFatChain = sDrv->CacheBufPoint->exfat_DirEntryFlags.f_NoFatChain;

                    if (psSearchInfo->FsType == FS_TYPE_exFAT)
                        psSearchInfo->exfat_NameHash = Cal_exFatFilenameHash((WORD *) sDrv->LongName, StringLength16((WORD *) sDrv->LongName));

                    /* entry is a directory */
                    psSearchInfo->bFileType = FILE_OP_TYPE_FOLDER;
                    psSearchInfo->bParameter = SEARCH_INFO_FOLDER;
                    (U64) psSearchInfo->u64FileSize = (U64) sDrv->CacheBufPoint->dataLength;
                    if ((U64) psSearchInfo->u64FileSize <= (U64) 0x00000000FFFFFFFF)
                        (DWORD) psSearchInfo->dwFileSize = (DWORD) psSearchInfo->u64FileSize;
                    else
                        psSearchInfo->dwFileSize = 0; /* not use 32-bit file size */

                    /* necessary for exFAT drives to keep chain size of the parent directory */
                    if (!sDrv->DirStackPoint) /* in RootDir */
                        psSearchInfo->dwParentDirSize = RootDir_size;
                    else
                        psSearchInfo->dwParentDirSize = sChain->Size;

                    /* exFAT has no 8.3 short name */
                    MpMemSet(psSearchInfo->bName, 0x20, 8);
                    MpMemSet(psSearchInfo->bExt, 0x20, 3);

                    /* get file modify date/time */
                    FileGetDateTime_for_exFAT(sDrv->CacheBufPoint->lastModifyTimestamp, &psSearchInfo->DateTime);
                    psSearchInfo->DateTime.UtcOffset = sDrv->CacheBufPoint->lastModifyUtcOffset;

            #if (SONY_DCF_ENABLE)
                    if ((! f_cannot_process_DCF) && (g_FileList_DCF_Info_List != NULL))
                    {
                        /* this entry is a folder => not DCF obj */
                        g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                        g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                    }
            #endif

                    /* next entry */
                    dwCount++;

                    if (dwCount >= maxFilesToScan)
                    {
                        *pdwRecordElement = dwCount;
                        swRetValue = FS_SUCCEED;
                        goto L_Post_Process_Before_File_Sorting;
                    }
                }
            }
            else
            {
                WORD utf16_Extname[5] = {0x002E, 0x0000, 0x0000, 0x0000, 0x0000}; //for UTF-16 of ".???" string
                BYTE ascii_Extname[4] = {0x00};
                WORD *wToken;
                BYTE *ch_ptr;
                WORD utf16_full_name[MAX_L_NAME_LENG];

                MpMemSet((BYTE *) utf16_full_name, 0, MAX_L_NAME_LENG * 2);
                StringCopy16((WORD *) utf16_full_name, (WORD *) sDrv->LongName);
                UpperCase16((WORD *) utf16_full_name); /* convert to upper case for easy string comparing */

                psSearchInfo = &working_FileList[dwCount];
                MpMemSet((BYTE *) psSearchInfo, 0, sizeof(ST_SEARCH_INFO));

                i = 0;
                while (pdwExtArray[i] != EXT_END)
                {
                    if (!sDrv->Flag.Present)
                    {
                        MP_ALERT("%s: Error! Card not present !", __FUNCTION__);
                        return FS_SCAN_FAIL;
                    }
                    if (sDrv->StatusCode)
                    {
                        MP_ALERT("%s: Error! file system status of the drive is abnormal !", __FUNCTION__);
                        return FS_SCAN_FAIL;
                    }

                    MpMemSet((BYTE *) utf16_Extname, 0, 5 * 2);
                    utf16_Extname[0] = 0x002E; //for UTF-16 ".???" string
                    MpMemSet(ascii_Extname, 0, 4);
                    ch_ptr = (BYTE *) ((DWORD *) &pdwExtArray[i]);

                    j = 0;
                    while (((BYTE) *(ch_ptr + j) != 0x2E) && (j < 3))
                    {
                        ascii_Extname[j] = (BYTE) *(ch_ptr + j);
                        j++;
                    }
                    //MP_DEBUG("%s: pdwExtArray[%d] = 0x%08x => ascii_Extname = [%s]", __FUNCTION__, i, (DWORD) pdwExtArray[i], ascii_Extname);

                    mpx_UtilUtf8ToUnicodeU16((WORD *) &utf16_Extname[1], ascii_Extname); /* convert to UTF-16 string */
                    UpperCase16((WORD *) utf16_Extname); /* convert to upper case for easy string comparing */

                    /* compare UTF-16 file extension name */
                    wToken = String16_strstr((WORD *) utf16_full_name, (WORD *) utf16_Extname);
                    if (wToken == NULL) /* not matched */
                    {
                        i++;
                        continue;
                    }
                    else
                    {
                        if (StringCompare16((WORD *) wToken, (WORD *) utf16_Extname) != E_COMPARE_EQUAL)
                        {
                            i++;
                            continue;
                        }
                    }

                    /* extension filename matched */
                    /* compare the extension filename of each media type */
                    dwExtension = (((*(DWORD *) & ascii_Extname[0]) & 0xffffff00) | 0x2e);
                    if ((dwExtension == pdwExtArray[i]) || (pdwExtArray[i] == EXT_ALL))
                    {
                        MP_DEBUG("%s: entry of [.%s] file type found:", __FUNCTION__, ascii_Extname);

                        psSearchInfo = &working_FileList[dwCount];

                        /* set ST_SEARCH_INFO fields */
                        sChain = (CHAIN *) ((DWORD) sDrv->DirStackBuffer + sDrv->DirStackPoint);
                        psSearchInfo->dwDirStart = sChain->Start;  // abel add for file copy and delete
                        psSearchInfo->dwDirPoint = sDrv->DirCachePoint;
                        psSearchInfo->dwFdbOffset = (sChain->Point >> 5);  //change byte to fdb offset (32 Bytes)
                        psSearchInfo->dwLongFdbCount = sDrv->CacheBufPoint->LongNameFdbCount;
                        psSearchInfo->dwMediaInfo = 0;
                        psSearchInfo->FsType = sDrv->Flag.FsType;
                        psSearchInfo->DrvIndex = sDrv->DrvIndex;
                        psSearchInfo->exfat_DirEntryFlags.f_AllocPossible = sDrv->CacheBufPoint->exfat_DirEntryFlags.f_AllocPossible;
                        psSearchInfo->exfat_DirEntryFlags.f_NoFatChain = sDrv->CacheBufPoint->exfat_DirEntryFlags.f_NoFatChain;

                        if (psSearchInfo->FsType == FS_TYPE_exFAT)
                            psSearchInfo->exfat_NameHash = Cal_exFatFilenameHash((WORD *) sDrv->LongName, StringLength16((WORD *) sDrv->LongName));

                        /* entry is a file */
                        psSearchInfo->bFileType = FileGetMediaType(ascii_Extname);
                        psSearchInfo->bParameter = SEARCH_INFO_FILE;
                        (U64) psSearchInfo->u64FileSize = (U64) sDrv->CacheBufPoint->dataLength;
                        if ((U64) psSearchInfo->u64FileSize <= (U64) 0x00000000FFFFFFFF)
                            (DWORD) psSearchInfo->dwFileSize = (DWORD) psSearchInfo->u64FileSize;
                        else
                            psSearchInfo->dwFileSize = 0; /* not use 32-bit file size */

                        /* necessary for exFAT drives to keep chain size of the parent directory */
                        if (!sDrv->DirStackPoint) /* in RootDir */
                            psSearchInfo->dwParentDirSize = RootDir_size;
                        else
                            psSearchInfo->dwParentDirSize = sChain->Size;

                        /* Note: exFAT does not support 8.3 short name any more.
                         * But our some modules (ex: Audio, Video, Demux...etc) use 3-byte ASCII extension name to check media type.
                         * For example, get_type_by_extension() in filter_graph.c does so.
                         * Therefore, we still set psSearchInfo->bExt field in ST_SEARCH_INFO here!
                         * But psSearchInfo->bName field is useless and ignored for exFAT drive!
                         */
                        StringCopy08(psSearchInfo->bExt, ascii_Extname);
                        MpMemSet(psSearchInfo->bName, 0x20, 8);

                        /* get file modify date/time */
                        FileGetDateTime_for_exFAT(sDrv->CacheBufPoint->lastModifyTimestamp, &psSearchInfo->DateTime);
                        psSearchInfo->DateTime.UtcOffset = sDrv->CacheBufPoint->lastModifyUtcOffset;

                #if (SONY_DCF_ENABLE)
                        if ((f_cannot_process_DCF) || (g_FileList_DCF_Info_List == NULL))
                            ; /* no memory for DCF Info list => do nothing w.r.t DCF info */
                        else if ((! f_RoorDCIM_found) || (! f_under_RoorDCIM_Dir) || (! f_under_RootDCIM_000AAAAA_Dir))
                        {
                            MP_DEBUG("index %u: => not under a /DCIM/000AAAAA format directory. \r\n", dwCount);
                            g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                            g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                        }
                        else
                        {
                            if (sDrv->DirStackPoint != (2 * sizeof(CHAIN)))
                            {
                                /* file not in exact "/DCIM/000AAAAA" directory level => not a DCF obj */
                                MP_DEBUG("index %u: => not in an exact /DCIM/000AAAAA format directory. \r\n", dwCount);
                                g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                                g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                            }
                            else
                            {
                                /* file in exact a "/DCIM/000AAAAA" format directory */
                                MP_DEBUG("index %u: => in an exact /DCIM/000AAAAA format directory.", dwCount);

                                /* check to make sure */
                                sChain = (CHAIN *) ((DWORD) sDrv->DirStackBuffer + sizeof(CHAIN));
                                if (sChain->Start != DCF_RootDCIM_StartCluster)
                                {
                                    MP_ALERT("%s: Error ! Something wrong/mismatch in the drive's DirStack !  check file system code ! \r\n", __FUNCTION__);

                                    /* unreasonable case, treat it as non-DCF cases */
                                    g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                                    g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                }
                                else
                                {
                                    if (f_DCIM_sameDigit_Dir_conflict)
                                    {
                                        /* /DCIM/000AAAAA same digit number conflict with other directory => not a DCF obj */
                                        MP_DEBUG(" => index %u: /DCIM/000AAAAA same digit number conflict with other directory => not a DCF obj. \r\n", dwCount);
                                        g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                                        g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                    }
                                    else
                                    {
                                        /* To-Do: must re-work this code in exFAT case, because exFAT DirEntry definitions changed */
                                        //int digitNum = Extract_DigitNum_of_DCF_Obj((BYTE *) psSearchInfo->bName);
                                        int digitNum = -1; //temp make it failed for DCF judgement
                                        if ((digitNum >= 0) && (digitNum <= 99999))
                                        {
                                            g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 0; /* to be finally confirmed in next stage */
                                            g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 1;

                                            /* record the digit number of this file's parent directory (/DCIM/000AAAAA) */
                                            g_FileList_DCF_Info_List[dwCount].RootDCIM_000AAAAA_Dir_DigitNum = RootDCIM_000AAAAA_Dir_DigitNum;
                                            MP_DEBUG(" => digit number of parent directory = %d", g_FileList_DCF_Info_List[dwCount].RootDCIM_000AAAAA_Dir_DigitNum);

                                        #if 0  /* To-Do: must re-work this code in exFAT case, because exFAT not support 8.3 short name !! */
                                            BYTE  output_name_buffer[9];
                                            DWORD output_display_integer = 0;
                                            if (Makeup_DCF_Obj_DisplayName(psSearchInfo->bName, dwCount, output_name_buffer, 9, &output_display_integer) == PASS)
                                            {
                                                MP_DEBUG(" => a possible DCF obj, displayed name = %s.%s (merged integer = %07lu) \r\n", output_name_buffer, psSearchInfo->bExt, output_display_integer);
                                                g_FileList_DCF_Info_List[dwCount].merged_display_integer = output_display_integer;

                                                /* check whether if DCF objects with same displayed number */
                                                for (i=0; (i < dwCount) && (i <= TEMP_FILE_LIST_MAX_ENTRIES_COUNT - 1); i++)
                                                {
                                                    if ( (g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj) && (! g_FileList_DCF_Info_List[i].Flags.f_final_confirmed) &&
                                                         (g_FileList_DCF_Info_List[i].merged_display_integer == output_display_integer) )
                                                    {
                                                        if (StringNCompare08_CaseInsensitive(psSearchInfo->bExt, working_FileList[i].bExt, 3) == E_COMPARE_EQUAL)
                                                        {
                                                            MP_DEBUG(" => AAA00000 file has same displayed number and same extension filename conflicted with other file (index %u) !", i);

                                                        //To-confirm: which rule should be applied ??
                                                        #if 1
                                                            /* only .JPG and .THM files should apply this DCF rule */
                                                            if ( (StringNCompare08_CaseInsensitive(psSearchInfo->bExt, "JPG", 3) == E_COMPARE_EQUAL) || (StringNCompare08_CaseInsensitive(psSearchInfo->bExt, "THM", 3) == E_COMPARE_EQUAL) )
                                                            {
                                                                MP_DEBUG(" => they are both .JPG or both .THM files => not a DCF obj. \r\n");
                                                                g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                                g_FileList_DCF_Info_List[i].Flags.f_final_confirmed = 1;

                                                                g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                                g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                                            }
                                                        #else
                                                            /* all DCF essential object types apply this DCF rule */
                                                            g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                            g_FileList_DCF_Info_List[i].Flags.f_final_confirmed = 1;

                                                            g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                            g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                                        #endif
                                                        }
                                                        else
                                                        {
                                                            MP_DEBUG(" => AAA00000 file has same displayed number but different extension filename with other file (index %u).", i);

                                                        #if 0  /* To-Do: must re-work this code in exFAT case, because exFAT not support 8.3 short name !! */
                                                            /* check whether if .JPG and .THM have same file names */
                                                            if ( (StringNCompare08_CaseInsensitive(psSearchInfo->bName, working_FileList[i].bName, 8) == E_COMPARE_EQUAL) &&
                                            	                 (((StringNCompare08_CaseInsensitive(psSearchInfo->bExt, "JPG", 3) == E_COMPARE_EQUAL) && (StringNCompare08_CaseInsensitive(working_FileList[i].bExt, "THM", 3) == E_COMPARE_EQUAL)) ||
                                            	                  ((StringNCompare08_CaseInsensitive(psSearchInfo->bExt, "THM", 3) == E_COMPARE_EQUAL) && (StringNCompare08_CaseInsensitive(working_FileList[i].bExt, "JPG", 3) == E_COMPARE_EQUAL))) )
                                                            {
                                                                MP_DEBUG(" => .JPG and .THM files (index %u and %u) have same file names => both are not DCF objects. \r\n", i, dwCount);
                                                                g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                                g_FileList_DCF_Info_List[i].Flags.f_final_confirmed = 1;

                                                                g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0; /* conflict => not DCF obj */
                                                                g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                                            }
                                                        #endif
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                MP_ALERT("%s: Makeup_DCF_Obj_DisplayName() failed => check source code for debugging !!\r\n");
                                            }
                                        #endif
                                        }
                                        else
                                        {
                                            /* filename not AAA00000 format => not a DCF obj */
                                            MP_DEBUG(" => index %u: filename not AAA00000 format => not a DCF obj. \r\n", dwCount);
                                            g_FileList_DCF_Info_List[dwCount].Flags.f_maybe_DCF_Obj = 0;
                                            g_FileList_DCF_Info_List[dwCount].Flags.f_final_confirmed = 1;
                                        }
                                    }
                                }
                            }
                        }
                #endif

                        /* next entry */
                        dwCount++;

                        if (dwCount >= maxFilesToScan)
                        {
                            *pdwRecordElement = dwCount;
                            swRetValue = FS_SUCCEED;
                            goto L_Post_Process_Before_File_Sorting;
                        }

                        break;  /* EXT_ALL for scanning extension filename was reached */
                    }
                    i++;
                }
            }
        }
#endif

L_Scan_Next_DirEntry:
        swRetValue = DirNext(sDrv);
        if ((swRetValue != FS_SUCCEED) && (swRetValue != END_OF_DIR))
        {
            MP_ALERT("%s: DirNext() failed !", __FUNCTION__);
            if (! SystemCardPresentCheck(sDrv->DrvIndex) || (! sDrv->Flag.Present))
            {
                MP_ALERT("%s: Card is not present (un-plugged) => release scanned file list !", __FUNCTION__);
                f_Halt_for_CardOut = TRUE;
                MpMemSet((BYTE *) sSearchInfo, 0, (dwMaxElement * sizeof(ST_SEARCH_INFO)));
                *pdwRecordElement = 0;

        #if (SONY_DCF_ENABLE)
                if (tmp_FileList)
                {
                    ext_mem_free(tmp_FileList);
                    tmp_FileList = NULL;
                }
                if (g_FileList_DCF_Info_List)
                {
                    ext_mem_free(g_FileList_DCF_Info_List);
                    g_FileList_DCF_Info_List = NULL;
                }
                g_DCF_Obj_TotalCnt = 0;
        #endif
            }
            break;
        }

        while ((sDrv->DirStackPoint != dwBaseDeep) && (swRetValue == END_OF_DIR))
        {
            if (!sDrv->Flag.Present)
            {
                MP_ALERT("%s: Error! Card not present (un-plugged) !", __FUNCTION__);
                return FS_SCAN_FAIL;
            }
            if (sDrv->StatusCode)
            {
                MP_ALERT("%s: Error! file system status of the drive is abnormal !", __FUNCTION__);
                return FS_SCAN_FAIL;
            }

            MP_DEBUG("%s: back to an upper directory level ...", __FUNCTION__);
            /* back to an upper directory level */
            if ((swRetValue = CdParent(sDrv)))
            {
                MP_ALERT("%s: Error! CdParent() failed !", __FUNCTION__);
                return swRetValue;
            }

        #if (SONY_DCF_ENABLE)
            if ((! f_cannot_process_DCF) && f_under_RoorDCIM_Dir)
            {
                if (! sDrv->DirStackPoint)
                {
                    f_under_RoorDCIM_Dir = FALSE;
                    MP_DEBUG("%s: Just escape the %s folder, currently in / root directory.", __FUNCTION__, DCF_RootDCIM_str);
                }
                else if (f_under_RootDCIM_000AAAAA_Dir && (sDrv->DirStackPoint == sizeof(CHAIN)))
                {
                    f_under_RootDCIM_000AAAAA_Dir = FALSE;
                    f_DCIM_sameDigit_Dir_conflict = FALSE;
                    MP_DEBUG("%s: Just escape a /DCIM/000AAAAA format sub-directory, currently in /DCIM directory.", __FUNCTION__);
                }
            }
        #endif

            swRetValue = DirNext(sDrv);

            if ((swRetValue != FS_SUCCEED) && (swRetValue != END_OF_DIR))
            {
                MP_ALERT("%s: DirNext() failed !", __FUNCTION__);
                if (! SystemCardPresentCheck(sDrv->DrvIndex) || (! sDrv->Flag.Present))
                {
                    MP_ALERT("%s: Card is not present (un-plugged) => release scanned file list !", __FUNCTION__);
                    f_Halt_for_CardOut = TRUE;
                    MpMemSet((BYTE *) sSearchInfo, 0, (dwMaxElement * sizeof(ST_SEARCH_INFO)));
                    *pdwRecordElement = 0;

            #if (SONY_DCF_ENABLE)
                    if (tmp_FileList)
                    {
                        ext_mem_free(tmp_FileList);
                        tmp_FileList = NULL;
                    }
                    if (g_FileList_DCF_Info_List)
                    {
                        ext_mem_free(g_FileList_DCF_Info_List);
                        g_FileList_DCF_Info_List = NULL;
                    }
                    g_DCF_Obj_TotalCnt = 0;
            #endif
                }
                break;
            }
        }

        TaskYield();
    } //while loop

L_Post_Process_Before_File_Sorting:
    if (! f_Halt_for_CardOut)
    {
        *pdwRecordElement = dwCount;
#if (SONY_DCF_ENABLE)
        g_WorkingFileList_TotalCount = dwCount;
#endif
        MP_ALERT("%s: count of recorded entries for found media files = %u", __FUNCTION__, dwCount);
    }

#if (SONY_DCF_ENABLE)
    /* post processing to confirm each possible DCF object if it meet all DCF rules */
    if ((! f_Halt_for_CardOut) && (! f_cannot_process_DCF) && (g_FileList_DCF_Info_List != NULL))
    {
        for (i=0; (i < dwCount) && (i <= TEMP_FILE_LIST_MAX_ENTRIES_COUNT - 1); i++)
        {
            /* for each .THM possible DCF object, check whether if any non-JPG DCF essential object exist with same file name as it */
            if ( (g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj) && (! g_FileList_DCF_Info_List[i].Flags.f_final_confirmed) &&
                 (StringNCompare08_CaseInsensitive(working_FileList[i].bExt, "THM", 3) == E_COMPARE_EQUAL) )
            {
                BOOL  f_non_JPG_same_name_found = FALSE;
                DWORD x;

                for (x=0; (x < dwCount) && (x <= TEMP_FILE_LIST_MAX_ENTRIES_COUNT - 1); x++)
                {
                    if ( (g_FileList_DCF_Info_List[x].Flags.f_maybe_DCF_Obj) && (x != i) &&
                         (g_FileList_DCF_Info_List[x].RootDCIM_000AAAAA_Dir_DigitNum == g_FileList_DCF_Info_List[i].RootDCIM_000AAAAA_Dir_DigitNum) &&
                         (StringNCompare08_CaseInsensitive(working_FileList[x].bName, working_FileList[i].bName, 8) == E_COMPARE_EQUAL) &&
                         (StringNCompare08_CaseInsensitive(working_FileList[x].bExt, "JPG", 3) == E_COMPARE_DIFFERENT) )
                    {
                        f_non_JPG_same_name_found = TRUE;
                        break;
                    }
                }

                if (! f_non_JPG_same_name_found)
                {
                    MP_DEBUG("%s: The .THM file (index %u) has no non-JPG DCF essential object with same name as it => not a DCF object. \r\n", __FUNCTION__, i);
                    g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj = 0;
                    g_FileList_DCF_Info_List[i].Flags.f_final_confirmed = 1;
                }
            }

            /* To-Do: if there is any extra DCF rule not being checked, process it here */

        }
    }
#endif

#if (MEASURE_FILE_SCANNING_SORTING_TIME)
    elapsed_time = SystemGetElapsedTime(start_time);
    MP_ALERT("***** [Stage 1] Total time spent on scanning media files = (%u) ms", elapsed_time);
#endif

L_File_Sorting:
    if ((! f_Halt_for_CardOut) && ((swRetValue == END_OF_DIR) || (swRetValue == FS_SUCCEED)))
    {
        if (bSortFlag != 0)
        {
    #if (MEASURE_FILE_SCANNING_SORTING_TIME)
            MP_ALERT("***** [Stage 2] begin sorting [%u] file entries ...", dwCount);
            start_time = SystemGetTimeStamp();
    #endif

#if (SONY_DCF_ENABLE)
            if (! f_cannot_process_DCF)
            {
                if (g_FileList_DCF_Info_List == NULL)
                {
                    g_Sorting_Start_Index_in_Companion_Info_List = 0;
                    swRetValue = FileSort(working_FileList, (*pdwRecordElement), g_CurFileSortingBasis);
                }
                else
                {
                    DWORD start_index_to_copy, entries_count_to_copy;
                    DWORD available_entries_count = MIN(dwMaxElement, FILE_LIST_SIZE);
                    DWORD i, j;

                    g_DCF_Obj_TotalCnt = 0;
                    for (i=0; i < dwCount; i++)
                    {
                        if (g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj)
                            g_DCF_Obj_TotalCnt++;
                    }
                    MP_ALERT("%s: count of SONY DCF essential objects = %u", __FUNCTION__, g_DCF_Obj_TotalCnt);

                    if (g_DCF_Obj_TotalCnt > 0)
                    {
                        if (g_CurFileSortingBasis == FILE_SORTING_BY_83SHORT_NAME_REVERSE)  /* reverse order => descending order of SONY file name order */
                            sorting_method = FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE;
                        else if (g_CurFileSortingBasis == FILE_SORTING_BY_83SHORT_NAME)  /* => ascending order of SONY file name order */
                            sorting_method = FILE_SORTING_BY_SONY_DCF_NAMING;
                        else
                            sorting_method = g_CurFileSortingBasis;
                    }
                    else
                        sorting_method = g_CurFileSortingBasis;

                    /* For sorting by EXIF date time, get and keep EXIF date time info of each file in the file list first */
                    if ((sorting_method == FILE_SORTING_BY_EXIF_DATE_TIME) || (sorting_method == FILE_SORTING_BY_EXIF_DATE_TIME_REVERSE))
                    {
                        /* currently, we keep EXIF date time info of each file in the companion DCF info list */
                        if (g_FileList_DCF_Info_List != NULL)
                        {
                            DATE_TIME_INFO_TYPE  exif_date_time;

                            for (i=0; i < dwCount; i++)
                            {
                                if (FileBrowserGetImgEXIF_DateTime((ST_SEARCH_INFO *) &working_FileList[i], &exif_date_time) == PASS)
                                {
                                    g_FileList_DCF_Info_List[i].Flags.f_have_EXIF_time_info = 1;
                                    MpMemCopy((BYTE *) &(g_FileList_DCF_Info_List[i].exif_date_time), &exif_date_time, sizeof(DATE_TIME_INFO_TYPE));
                                }
                                else
                                    g_FileList_DCF_Info_List[i].Flags.f_have_EXIF_time_info = 0;
                            }
                        }
                    }

                    if ((sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING) || (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE))
                    {
                        /* Re-arrange the file list to two sets: DCF objects set, and non-DCF files set. And then sort the file list
                         * in SONY DCF file name ascending or descending order.
                         */
                        swRetValue = FileSort_for_Sony_DCF_Name_Order(working_FileList, dwCount, sorting_method);

                        /* Remove lower-priority (according to display order) DCF objects with same primary filename, keep the highest-priority one in the file list.
                         * Display order for DCF image:  JPG > TIF > BMP > RAW > (THM) > JPE > JFI  (how about THM, SRF, SR2 ??)
                         * Display order for thumbnail:  THM > JPG > TIF > BMP > RAW > JPE > JFI    (when to adopt this order ??)
                         */
                        DWORD removed_lower_prior_DCF_obj_count = 0;
                        DWORD start_DCF_index;

                        /* only need to compare DCF objects part in the well-arranged and sorted file list */
                        start_DCF_index = (g_FileList_DCF_Info_List[0].Flags.f_maybe_DCF_Obj)? 0 : (dwCount - g_DCF_Obj_TotalCnt);
                        for (i = start_DCF_index; i < (start_DCF_index + g_DCF_Obj_TotalCnt); i++)
                        {
                            if (g_FileList_DCF_Info_List[i].Flags.f_removed_lower_prior_DCF_obj) /* skip those DCF object entries marked as 'removed' */
                                continue;

                            for (j = (i + 1); j < (start_DCF_index + g_DCF_Obj_TotalCnt); j++)
                            {
                                if (g_FileList_DCF_Info_List[i].merged_display_integer != g_FileList_DCF_Info_List[j].merged_display_integer)
                                    break;
                                else
                                {
                                    MP_DEBUG("Same-name DCF essential objects found:  index [%u] and [%u]", i, j);

                                    /* same name DCF essential objects found => compare their priority of DCF display order */
                                    BYTE  display_order_i, display_order_j;
                                    display_order_i = DCF_Img_DisplayOrderValue((BYTE *) working_FileList[i].bExt);
                                    display_order_j = DCF_Img_DisplayOrderValue((BYTE *) working_FileList[j].bExt);

                                    removed_lower_prior_DCF_obj_count++;

                                    /* set the 'remove' flag to remove the lower-priority entry from the list later */
                                    if (display_order_i < display_order_j)
                                    {
                                        MP_DEBUG(" => mark to remove lower-display-priority entry [%u] later ...", j);
                                        g_FileList_DCF_Info_List[j].Flags.f_removed_lower_prior_DCF_obj = 1;
                                        continue;
                                    }
                                    else
                                    {
                                        MP_DEBUG(" => mark to remove lower-display-priority entry [%u] later ...", i);
                                        g_FileList_DCF_Info_List[i].Flags.f_removed_lower_prior_DCF_obj = 1;
                                        break;  /* current list entry [i] is marked as 'removed' */
                                    }
                                }
                            }
                        }

                        /* just make sure before performing file list entries copy */
                        if (working_FileList == sSearchInfo)
                        {
                            MP_ALERT("%s: (working_FileList == sSearchInfo), This should be impossible in this case !!  Check source code for debugging...", __FUNCTION__);
                            __asm("break 100");
                        }

                        DWORD final_kept_list_entries_count, final_kept_DCF_entries_count;
                        BOOL  f_DCF_entries_in_Rear_part_of_list = FALSE;

                        if (! g_DCF_Obj_TotalCnt) /* no any DCF object */
                        {
                            entries_count_to_copy = MIN(available_entries_count, dwCount);
                            start_index_to_copy = 0;

                            MpMemCopy((BYTE *) sSearchInfo, (BYTE *) &working_FileList[start_index_to_copy], entries_count_to_copy * sizeof(ST_SEARCH_INFO));
                            *pdwRecordElement = entries_count_to_copy;
                        }
                        else
                        {
                            /* re-arrange the file list: remove entries marked as 'removed', and then compact the list */
                            if (removed_lower_prior_DCF_obj_count)
                            {
                                if ((g_FileList_DCF_Info_List[0].Flags.f_maybe_DCF_Obj) || (g_FileList_DCF_Info_List[0].Flags.f_removed_lower_prior_DCF_obj))
                                    f_DCF_entries_in_Rear_part_of_list = FALSE;
                                else
                                    f_DCF_entries_in_Rear_part_of_list = TRUE;

                                /* whole DCF objects part is more important to be copied to system file list than copy for non-DCF part.
                                 * if DCF objects are in rear part of the temp file list, need careful processing to move entries to system file list.
                                 */
                                final_kept_list_entries_count = dwCount - removed_lower_prior_DCF_obj_count;
                                if ((available_entries_count < final_kept_list_entries_count) && (f_DCF_entries_in_Rear_part_of_list))
                                {
                                    DWORD  start_entry_index_to_compact_and_copy_list;
                                    DWORD  non_DCF_count_to_copy;

                                    final_kept_DCF_entries_count = g_DCF_Obj_TotalCnt - removed_lower_prior_DCF_obj_count;
                                    if (available_entries_count < final_kept_DCF_entries_count)
                                    {
                                        /* no any non-DCF entry will be copied to system file list */
                                        non_DCF_count_to_copy = 0;
                                        start_entry_index_to_compact_and_copy_list = start_DCF_index;
                                    }
                                    else
                                    {
                                        /* some non-DCF entries and all DCF entries (not marked as 'removed') will be copied to system file list */
                                        non_DCF_count_to_copy = available_entries_count - final_kept_DCF_entries_count;
                                        if (start_DCF_index < non_DCF_count_to_copy)
                                        {
                                            MP_ALERT("%s: (start_DCF_index < non_DCF_count_to_copy), This should be impossible in this case !!  Check source code for debugging...", __FUNCTION__);
                                            __asm("break 100");
                                        }
                                        start_entry_index_to_compact_and_copy_list = start_DCF_index - non_DCF_count_to_copy;
                                    }

                                    /* start to compact the temp file list (with some 'removed' entries) and copy to system file list */
                                    final_kept_list_entries_count = final_kept_DCF_entries_count = 0;
                                    for (i = start_entry_index_to_compact_and_copy_list; i < (g_DCF_Obj_TotalCnt + non_DCF_count_to_copy); i++)
                                    {
                                        if (! g_FileList_DCF_Info_List[i].Flags.f_removed_lower_prior_DCF_obj)
                                        {
                                            MpMemCopy((BYTE *) &sSearchInfo[final_kept_list_entries_count], (BYTE *) &working_FileList[i], sizeof(ST_SEARCH_INFO));

                                            if (final_kept_list_entries_count == 0)  /* first copied DCF object entry */
                                                start_index_to_copy = i;  /* record this entry index used for later rearrangement of DCF Info List */

                                            if (g_FileList_DCF_Info_List[start_entry_index_to_compact_and_copy_list + final_kept_list_entries_count].Flags.f_removed_lower_prior_DCF_obj)
                                            {
                                                /* exchange the 'removed' entry and a normal entry */
                                                /* To increase performance (reduce memcpy times), we don't use SwapData() here for exchange two g_FileList_DCF_Info_List[] entries */
                                                MpMemCopy((BYTE *) &g_FileList_DCF_Info_List[start_entry_index_to_compact_and_copy_list + final_kept_list_entries_count], (BYTE *) &g_FileList_DCF_Info_List[i], sizeof(DCF_INFO_TYPE));
                                                g_FileList_DCF_Info_List[i].Flags.f_removed_lower_prior_DCF_obj = 1;  /* 'removed' entry was changed to this entry */
                                            }
                                            final_kept_list_entries_count++;

                                            if (g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj)
                                                final_kept_DCF_entries_count++;

                                            /* check to avoid exceeding the available entries count of system file list */
                                            if (final_kept_list_entries_count >= available_entries_count)
                                                break;
                                        }
                                    }

                                    if (start_index_to_copy != 0)
                                    {
                                        entries_count_to_copy = (final_kept_DCF_entries_count + non_DCF_count_to_copy);

                                        /* re-arrange the DCF Info List for exactly one-to-one mapping with the final system file list */
                                        /* note: must carefully move rear part entries from the DCF Info List to the front part of itself ! */
                                        MpMemCopy((BYTE *) &g_FileList_DCF_Info_List[0], (BYTE *) &g_FileList_DCF_Info_List[start_index_to_copy], entries_count_to_copy * sizeof(DCF_INFO_TYPE));

                                        /* clear the rest rear entries in DCF Info List because those DCF Info entries already became not corresponding to valid range of the system file list */
                                        MpMemSet((BYTE *) &g_FileList_DCF_Info_List[entries_count_to_copy], 0, (dwCount - entries_count_to_copy ) * sizeof(DCF_INFO_TYPE));
                                    }
                                }
                                else
                                {
                                    final_kept_list_entries_count = final_kept_DCF_entries_count = 0;
                                    for (i=0; i < dwCount; i++)
                                    {
                                        if (! g_FileList_DCF_Info_List[i].Flags.f_removed_lower_prior_DCF_obj)
                                        {
                                            MpMemCopy((BYTE *) &sSearchInfo[final_kept_list_entries_count], (BYTE *) &working_FileList[i], sizeof(ST_SEARCH_INFO));

                                            if (g_FileList_DCF_Info_List[final_kept_list_entries_count].Flags.f_removed_lower_prior_DCF_obj)
                                            {
                                                /* exchange the 'removed' entry and a normal entry */
                                                /* To increase performance (reduce memcpy times), we don't use SwapData() here for exchange two g_FileList_DCF_Info_List[] entries */
                                                MpMemCopy((BYTE *) &g_FileList_DCF_Info_List[final_kept_list_entries_count], (BYTE *) &g_FileList_DCF_Info_List[i], sizeof(DCF_INFO_TYPE));
                                                g_FileList_DCF_Info_List[i].Flags.f_removed_lower_prior_DCF_obj = 1;  /* 'removed' entry was changed to this entry */
                                            }
                                            final_kept_list_entries_count++;

                                            if (g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj)
                                                final_kept_DCF_entries_count++;

                                            /* check to avoid exceeding the available entries count of system file list */
                                            if (final_kept_list_entries_count >= MIN(dwCount, available_entries_count))
                                                break;
                                        }
                                    }
                                }

                                *pdwRecordElement = final_kept_list_entries_count;
                                g_DCF_Obj_TotalCnt = final_kept_DCF_entries_count;
                            }
                            else
                            {
                                final_kept_list_entries_count = dwCount;
                                entries_count_to_copy = MIN(available_entries_count, final_kept_list_entries_count);
                                start_index_to_copy = 0;

                                /* whole DCF objects part is more important to be copied to system file list than copy for non-DCF part */
                                if (g_FileList_DCF_Info_List[0].Flags.f_maybe_DCF_Obj)
                                    f_DCF_entries_in_Rear_part_of_list = FALSE;
                                else if (g_FileList_DCF_Info_List[final_kept_list_entries_count - 1].Flags.f_maybe_DCF_Obj)
                                    f_DCF_entries_in_Rear_part_of_list = TRUE;

                                if ((available_entries_count < final_kept_list_entries_count) && (f_DCF_entries_in_Rear_part_of_list))
                                {
                                    start_index_to_copy = final_kept_list_entries_count - available_entries_count;

                                    /* re-arrange the DCF Info List for exactly one-to-one mapping with the final system file list */
                                    /* note: must carefully move rear part entries from the DCF Info List to the front part of itself ! */
                                    MpMemCopy((BYTE *) &g_FileList_DCF_Info_List[0], (BYTE *) &g_FileList_DCF_Info_List[start_index_to_copy], entries_count_to_copy * sizeof(DCF_INFO_TYPE));
                                }

                                MpMemCopy((BYTE *) sSearchInfo, (BYTE *) &working_FileList[start_index_to_copy], entries_count_to_copy * sizeof(ST_SEARCH_INFO));
                                *pdwRecordElement = entries_count_to_copy;
                                g_DCF_Obj_TotalCnt = MIN(g_DCF_Obj_TotalCnt, entries_count_to_copy);
                            }
                        }
                    }
                    else
                    {
                        /* no need to re-arrange the file list to two sets */
                        g_Sorting_Start_Index_in_Companion_Info_List = 0;
                        swRetValue = FileSort(working_FileList, (*pdwRecordElement), g_CurFileSortingBasis);

                        entries_count_to_copy = MIN(available_entries_count, dwCount);
                        start_index_to_copy = 0;

                        MpMemCopy((BYTE *) sSearchInfo, (BYTE *) &working_FileList[start_index_to_copy], entries_count_to_copy * sizeof(ST_SEARCH_INFO));
                        *pdwRecordElement = entries_count_to_copy;
                        g_DCF_Obj_TotalCnt = MIN(g_DCF_Obj_TotalCnt, entries_count_to_copy);
                    }

                    ext_mem_free(tmp_FileList); /* release the temp file list */
                    g_WorkingFileList_BasePtr = sSearchInfo;
                    g_WorkingFileList_TotalCount = *pdwRecordElement;
                    MP_ALERT("\r\n Finally, total entries count in system file list = %u, DCF objects count in the list = %u \r\n", *pdwRecordElement, g_DCF_Obj_TotalCnt);
                }
            }
            else
            {
                g_Sorting_Start_Index_in_Companion_Info_List = 0;
                swRetValue = FileSort(working_FileList, *pdwRecordElement, g_CurFileSortingBasis);
            }
#else //for SONY_DCF_ENABLE disabled cases
#if INFO_LIST_ENBALE
            if (g_FileList_Extra_Info_List)
            {
                ext_mem_free(g_FileList_Extra_Info_List);
                g_FileList_Extra_Info_List = NULL;
            }

            sorting_method = g_CurFileSortingBasis;

            /* For sorting by EXIF date time, get and keep EXIF date time info of each file in the file list first */
            if ((sorting_method == FILE_SORTING_BY_EXIF_DATE_TIME) || (sorting_method == FILE_SORTING_BY_EXIF_DATE_TIME_REVERSE))
            {
                if (g_FileList_Extra_Info_List == NULL)
                {
                    DWORD total_count = MIN(dwCount, TEMP_FILE_LIST_MAX_ENTRIES_COUNT);

                    g_FileList_Extra_Info_List = (EXTRA_FILE_LIST_INFO_TYPE *) ext_mem_malloc(total_count * sizeof(EXTRA_FILE_LIST_INFO_TYPE));
                    if (g_FileList_Extra_Info_List == NULL)
                    {
                        MP_ALERT("%s: -E- mem alloc failed for Extra Info list => disable EXIF date time processing !!", __FUNCTION__);
                        goto L_sorting_directly;
                    }
                    MpMemSet((BYTE *) g_FileList_Extra_Info_List, 0, (total_count * sizeof(EXTRA_FILE_LIST_INFO_TYPE)));
                }

                /* currently, we keep EXIF date time info of each file in the companion info list */
                DATE_TIME_INFO_TYPE  exif_date_time;
                for (i=0; i < dwCount; i++)
                {
                    /* note: 'folder' entries have no EXIF data */
                    if (working_FileList[i].bFileType == FILE_OP_TYPE_FOLDER)
                    {
                        g_FileList_Extra_Info_List[i].Flags.f_have_EXIF_time_info = 0;
                        g_FileList_Extra_Info_List[i].Flags.f_final_confirmed = 1;
                        continue;
                    }

                    if (FileBrowserGetImgEXIF_DateTime((ST_SEARCH_INFO *) &working_FileList[i], &exif_date_time) == PASS)
                    {
                        g_FileList_Extra_Info_List[i].Flags.f_have_EXIF_time_info = 1;
                        MpMemCopy((BYTE *) &(g_FileList_Extra_Info_List[i].exif_date_time), &exif_date_time, sizeof(DATE_TIME_INFO_TYPE));
                    }
                    else
                        g_FileList_Extra_Info_List[i].Flags.f_have_EXIF_time_info = 0;

                    g_FileList_Extra_Info_List[i].Flags.f_final_confirmed = 1;
                }
            }
#endif
L_sorting_directly:
            swRetValue = FileSort(working_FileList, *pdwRecordElement, g_CurFileSortingBasis);
#endif

    #if (MEASURE_FILE_SCANNING_SORTING_TIME)
            elapsed_time = SystemGetElapsedTime(start_time);
            MP_ALERT("***** [Stage 2] Total time spent on file sorting = (%u) ms", elapsed_time);
    #endif

            return swRetValue;
        }
        else
            return FS_SUCCEED;
    }

    return swRetValue;
}



///
///@ingroup FILE
///@brief   Search files on the specified drive for the specified file extension name, and return the file handle of the first
///         matched file found. This searching only checks the first FDB nodes on the drive up to a specified max amount of FDB nodes.
///
///@param   bMcardId        The drive ID of the drive to search. \n\n
///@param   bExt            The specified file extension name to search matched file. \n\n
///@param   dwSearchCount   The specified max number of FDB nodes to check for matched file extension name on the drive.
///
///@return   This function call will return the file handle of a matched file for the specified file extension name,
///          else return NULL if any error or file not found.
///
///@remark   This function will force drive change. And it is intended for usage only for ISP and loading XPG from SD card. \n
///          Be very careful to use this function !!
///
STREAM *FileExtSearchOpen(BYTE bMcardId, BYTE *bExt, DWORD dwSearchCount)
{
    DRIVE *drv;
    int   i;
    BYTE  bInput_ext_name[4];
#if ISP_FUNC_ENABLE
    BYTE  bInput_ISP_name[9], bNode_filename[9]; /* we limit main part of ISP filename to max 8 characters (for compatible with 8.3 filename) */
#endif

#if EXFAT_ENABLE  /* used as UTF-16 string buffers for exFAT drive */
    WORD  wInput_ext_name[4];
  #if ISP_FUNC_ENABLE
    WORD  wInput_ISP_name[9]; /* we limit main part of ISP filename to max 8 characters (for compatible with 8.3 filename) */
  #endif
#endif


    drv = DriveGet(bMcardId);
    if (DirReset(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E- DirReset() back to beginning of root directory failed !", __FUNCTION__);
        return NULL;
    }
    if (DirFirst(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: DirFirst() failed !", __FUNCTION__);
        return NULL;
    }

    if (StringLength08(bExt) > 3)
    {
        MP_ALERT("%s: Error! Input file extension name [%s] exceeds 3 characters !", __FUNCTION__, bExt);
        return NULL;
    }

    MpMemSet(bInput_ext_name, 0x20, 3); /* 3 spaces for FATxx 8.3 short filename */
    bInput_ext_name[3] = 0x00; /* must have null terminator */
    MpMemCopy((BYTE *) bInput_ext_name, bExt, StringLength08(bExt)); /* to reserve tailing spaces if exist */
    UpperCase08(bInput_ext_name);

#if ISP_FUNC_ENABLE
    MpMemSet(bInput_ISP_name, 0x20, 8); /* 8 spaces for FATxx 8.3 short filename */
    bInput_ISP_name[8] = 0x00; /* must have null terminator */
  #ifdef ISP_FILENAME
    MpMemCopy((BYTE *) bInput_ISP_name, ISP_FILENAME, StringLength08(ISP_FILENAME)); /* to reserve tailing spaces if exist */
  #else
    MpMemCopy((BYTE *) bInput_ISP_name, "MP650", StringLength08("MP650")); /* to reserve tailing spaces if exist */
  #endif
    UpperCase08(bInput_ISP_name);
#endif

#if EXFAT_ENABLE
    if (drv->Flag.FsType == FS_TYPE_exFAT)
    {
        /* exFAT does not support 8.3 short filename. So, tailing spaces in the extension name must be cut off */
        for (i = 0; i < 3; i++)
        {
            if (bInput_ext_name[i] == 0x20) //space (' ')
                bInput_ext_name[i] = 0x00;
        }

        MpMemSet((BYTE *) wInput_ext_name, 0, 4 * sizeof(WORD));
        mpx_UtilUtf8ToUnicodeU16(wInput_ext_name, bInput_ext_name); /* convert to UTF-16 string */

  #if ISP_FUNC_ENABLE
        /* exFAT does not support 8.3 short filename. So, tailing spaces in the ISP filename must be cut off */
        for (i = 0; i < 8; i++)
        {
            if (bInput_ISP_name[i] == 0x20) //space (' ')
                bInput_ISP_name[i] = 0x00;
        }

        MpMemSet((BYTE *) wInput_ISP_name, 0, 9 * sizeof(WORD));
        mpx_UtilUtf8ToUnicodeU16(wInput_ISP_name, bInput_ISP_name); /* convert to UTF-16 string */
  #endif
    }
#endif

    for (i = 0; i < dwSearchCount; i++)
    {
        STREAM *handle;

        if (drv->Flag.FsType != FS_TYPE_exFAT)
        {
            if (drv->Node->Attribute & FDB_SUB_DIR) /* FDB node is a directory */
                goto L_next_fdb;
            else if (drv->Node->Attribute & FDB_LABEL) /* FDB node is the volume label of the drive */
                goto L_next_fdb;
            else if (drv->Node->Attribute == FDB_LONG_NAME) /* FDB node is a long name FDB node */
                goto L_next_fdb;
            else
            {
                if (!(drv->Node->Attribute & FDB_ARCHIVE)) /* FDB node's attribute is abnormal */
                {
                    MP_DEBUG("%s: Attribute field (=0x%02x) of current FDB node is abnormal !", __FUNCTION__, drv->Node->Attribute);
                    goto L_next_fdb;
                }
            }

            handle = FileOpen(drv);
            if (!handle)
            {
                MP_ALERT("%s: FileOpen() failed !", __FUNCTION__);
                return NULL;
            }

            if ((drv->Node->Extension[0] != bInput_ext_name[0]) || (drv->Node->Extension[1] != bInput_ext_name[1]) || (drv->Node->Extension[2] != bInput_ext_name[2]))
            {
                FileClose(handle);
                goto L_next_fdb;
            }
            else
            {
#if ISP_FUNC_ENABLE
                if ((bInput_ext_name[0] == 'I') && (bInput_ext_name[1] == 'S') && (bInput_ext_name[2] == 'P'))
                {
                    // special check file name when search isp file
            #if 0 //original hard-coded
                    if ((drv->Node->Name[0] == 'M') && (drv->Node->Name[1] == 'P') && (drv->Node->Name[2] == '6') &&
                        (drv->Node->Name[3] == '5') && (drv->Node->Name[4] == '0') && (drv->Node->Name[5] == 0x20) &&
                        (drv->Node->Name[6] == 0x20) && (drv->Node->Name[7] == 0x20))
            #else //new method
                    MpMemCopy(bNode_filename, (BYTE *) drv->Node->Name, 8);
                    bNode_filename[8] = 0x00; //must have null terminator
    					UpperCase08(bNode_filename); // Add by Tech 20150810
                    if (StringNCompare08(bInput_ISP_name, bNode_filename, 8) == E_COMPARE_EQUAL)
            #endif
                    {
                        return handle; /* ISP file found */
                    }
                    else
                    {
                        MP_ALERT("%s: ISP file found, but wrong ISP file name (%s) !", __FUNCTION__, bNode_filename);
                        FileClose(handle);
                        goto L_next_fdb;
                    }
                }
                else
#endif
                {
                    return handle; /* file found */
                }
            }

        }
#if EXFAT_ENABLE
        else
        {
            int  j;
            WORD filename_len, rightmost_dot_offset;
            WORD wExt_name[4];
            BOOL rightmost_dot_found;
            TYPE_exFAT_Stream_Extension_DirEntry *exfat_node;

            /* current drv->Node should be a "Stream Extension DirectoryEntry" of exFAT */
            exfat_node = (TYPE_exFAT_Stream_Extension_DirEntry *) (drv->Node);
            if (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT)
            {
                MP_ALERT("%s: Error! Current node should be a (Stream Extension DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
                return NULL;
            }

            if (drv->CacheBufPoint->fileAttributes & EXFAT_FDB_ATTRIB_DIR) /* DirectoryEntry is a directory */
                goto L_next_fdb;
            else if (drv->CacheBufPoint->fileAttributes & EXFAT_FDB_ATTRIB_VOLUME) /* DirectoryEntry is the volume label of the drive */
                goto L_next_fdb;
            else
            {
                if (!(drv->CacheBufPoint->fileAttributes & EXFAT_FDB_ATTRIB_ARCH)) /* DirectoryEntry's attribute is abnormal */
                {
                    MP_DEBUG("%s: Attribute field (=0x%02x) of current DirectoryEntry is abnormal !", __FUNCTION__, drv->CacheBufPoint->fileAttributes);
                    goto L_next_fdb;
                }
            }

            filename_len = StringLength16((WORD *) drv->LongName);
            MpMemSet((BYTE *) wExt_name, 0, 4 * sizeof(WORD));
            rightmost_dot_found = FALSE;
            rightmost_dot_offset = 0;
            /* check extension part of the filename. We limit it not to exceed 4 characters (".???") at most */
            for (j = filename_len - 1; (j >= 0) && (j >= filename_len - 4); j--)
            {
                if ((WORD) drv->LongName[j] == 0x002E) //'.'
                {
                    rightmost_dot_found = TRUE;
                    rightmost_dot_offset = j;
                    break;
                }
            }

            if (rightmost_dot_found)
            {
                StringCopy16(wExt_name, (WORD *) &(drv->LongName[j + 1]));
                UpperCase16(wExt_name);

                if (StringCompare16(wInput_ext_name, wExt_name) == E_COMPARE_EQUAL)
                {
#if ISP_FUNC_ENABLE
                    if ((bInput_ext_name[0] == 'I') && (bInput_ext_name[1] == 'S') && (bInput_ext_name[2] == 'P'))
                    {
                        /* note: rightmost_dot_offset is the length of main part of the filename */
                        drv->LongName[rightmost_dot_offset] = 0x0000;

                        if (StringLength16(wInput_ISP_name) == rightmost_dot_offset)
                        {
                            if (StringCompare16(wInput_ISP_name, (WORD *) drv->LongName) != E_COMPARE_EQUAL) /* main part of the ISP filename mismatched */
                            {
                                MP_ALERT("%s: ISP file found, but wrong ISP file name !", __FUNCTION__);
                                goto L_next_fdb;
                            }
                        }
                        else /* length of main part of the ISP filename mismatched */
                        {
                            MP_ALERT("%s: ISP file found, but wrong ISP file name !", __FUNCTION__);
                            goto L_next_fdb;
                        }
                    }
#endif

                    handle = FileOpen(drv);
                    if (!handle)
                    {
                        MP_ALERT("%s: FileOpen() failed !", __FUNCTION__);
                        return NULL;
                    }
                    else
                        return handle; /* file found */
                }
            }
        }
#endif

L_next_fdb:
        if (DirNext(drv) != FS_SUCCEED)
        {
            MP_DEBUG("%s: Nothing matched !", __FUNCTION__);
            return NULL;
        }
    }

    MP_DEBUG("%s: Nothing matched !", __FUNCTION__);
    return NULL;
}



///
///@ingroup FILE
///@brief   Rename the file extension name of the specified file to at most 3 ASCII character bytes. \n\n
///
///@param   handle          The file to rename its file extension name. \n\n
///@param   new_extname     ASCII string of the new file extension name for the file to rename. \n\n
///
///@retval  FS_SUCCEED          Rename the file extension name successfully. \n\n
///@retval  ABNORMAL_STATUS     Rename the file extension name unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      File not found, no need to rename it. \n\n
///
///@remark   This function only supports at most 3 ASCII character bytes in the file extension name ! \n\n
///
int FileExtRename(STREAM * handle, BYTE * new_extname)
{
    DRIVE *drv;
    FDB *node;
    CHAIN *chain;
    BYTE i;
    int ret;


    if (handle == NULL)
    {
        MP_ALERT("%s: Error ! NULL file handle !", __FUNCTION__);
        return FILE_NOT_FOUND;
    }

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (handle->Flag.f_IsRamFile)
    {
        MP_ALERT("%s: this operation is not supported for \"RamFile\" type files !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
#endif

    if (StringLength08(new_extname) > 3)
    {
        MP_ALERT("%s: only extension filename < 4 character bytes is supported !!", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    drv = (DRIVE *) (handle->Drv);
    if (drv == NULL)
        return FILE_NOT_FOUND; // this file has been closed

    /* make sure drive's Dir cache buffer covers the file FDB */
    if (FileDirCaching(handle) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E- FileDirCaching() failed !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    node = (FDB *) drv->DirCacheBuffer + handle->FdbOffsetInDirCache;

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        BYTE ori_bChkSum, new_bChkSum;
        ENTRY_FILENAME_TYPE  pbFilenameType;
        BYTE  *tmp_FilenameBuf;
        DWORD pdwBufferCount;

        tmp_FilenameBuf = (BYTE *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
        if (tmp_FilenameBuf == NULL)
        {
            MP_ALERT("%s: -E- mem alloc failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        /* check to see if long filename */
        MpMemSet(tmp_FilenameBuf, 0, MAX_L_NAME_LENG * 2);
        if (GetFilenameOfCurrentNode(drv, &pbFilenameType, tmp_FilenameBuf, &pdwBufferCount, MAX_L_NAME_LENG * 2) != FS_SUCCEED)
        {
            MP_ALERT("%s: GetFilenameOfCurrentNode() failed !", __FUNCTION__);
            ext_mem_free(tmp_FilenameBuf);
            return ABNORMAL_STATUS;
        }

        /* generate checksum */
        ori_bChkSum = 0;
        for (i = 0; i < 11; i++)
        {
            /* note: The operation is an unsigned char rotate right */
            ori_bChkSum = ((ori_bChkSum & 1) ? 0x80 : 0) + (ori_bChkSum >> 1) + node->Name[i];
        }

        memset((BYTE *) node->Extension, 0x20, 3);
        for (i=0; i < StringLength08(new_extname); i++)
            node->Extension[i] = toupper(new_extname[i]);

        new_bChkSum = 0;
        for (i = 0; i < 11; i++)
        {
            if (node->Name[i] == 0x00)
                node->Name[i] = 0x20; /* 8.3 short name field in the FDB should not contain null terminators, it uses spaces for padding */

            /* note: The operation is an unsigned char rotate right */
            new_bChkSum = ((new_bChkSum & 1) ? 0x80 : 0) + (new_bChkSum >> 1) + node->Name[i];
        }

        if (pbFilenameType == E_FILENAME_UTF16_LONG_NAME)
        {
            LONG_NAME *lnode;
            BYTE first, attr, lnode_total_count, lnode_tmp_count, rightmost_dot_in_lnode_count, bWord_offset;
            WORD *wStr, idx, rightmost_dot_index = 0;
            BYTE *pbExt_char;
            BOOL f_rightmost_dot_found = FALSE, f_rightmost_dot_well_processed = FALSE;

            lnode_total_count = (pdwBufferCount / 26);
            if (pdwBufferCount % 26)
                lnode_total_count++;

            wStr = (WORD *) tmp_FilenameBuf;
            for (idx = ((pdwBufferCount / 2) - 1); idx >= 1; idx--)
            {
                if ((WORD) wStr[idx] == 0x002e)  //'.'
                {
                    f_rightmost_dot_found = TRUE;
                    rightmost_dot_index = idx;
                    break;
                }
            }

            ext_mem_free(tmp_FilenameBuf);

            if (! f_rightmost_dot_found)
            {
                if (pdwBufferCount <= 252)
                    rightmost_dot_index = 252;
                else
                {
                    MP_ALERT("%s: file name length is too long to add the file extension name !", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }
            }

            lnode_tmp_count = 0;
            rightmost_dot_in_lnode_count = ((rightmost_dot_index + 1) / 13);
            if ((rightmost_dot_index + 1) % 13)
                rightmost_dot_in_lnode_count++;

            pbExt_char = new_extname;

            do
            {
L_next_longname_FDB_node:
                /* if no next node can read then return fail */
                if (PreviousNode(drv) != FS_SUCCEED)
                {
                    MP_ALERT("%s: PreviousNode() failed !", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }

                lnode_tmp_count++;
                first = drv->Node->Name[0];
                attr = drv->Node->Attribute;
                lnode = (LONG_NAME *) drv->Node;

                if ((first == 0xe5) || (first == 0x2e) || (attr != FDB_LONG_NAME))
                {
                    return ABNORMAL_STATUS;
                }
                else
                {
                    if (lnode->CheckSum != ori_bChkSum)
                    {
                        MP_ALERT("%s: checksum value in the long-name FDB node mismatched => Something wrong on the storage file system !", __FUNCTION__);
                        return ABNORMAL_STATUS;
                    }
                    else
                    {
                        BYTE *pbNameSegmentBuff;

                        /* update the checksum field in each long-name FDB node of this file */
                        lnode->CheckSum = new_bChkSum;

                        if (rightmost_dot_in_lnode_count == lnode_tmp_count)
                        {
                            bWord_offset = rightmost_dot_index - ((lnode_tmp_count - 1) * 13);

                            if (bWord_offset <= 4)
                                pbNameSegmentBuff = lnode->Name0;
                            else if ((5 <= bWord_offset) && (bWord_offset <= 10))
                            {
                                bWord_offset -= 5;
                                pbNameSegmentBuff = lnode->Name1;
                            }
                            else if ((11 <= bWord_offset) && (bWord_offset <= 12))
                            {
                                bWord_offset -= 11;
                                pbNameSegmentBuff = lnode->Name2;
                            }

                            if (! f_rightmost_dot_found)
                            {
                                pbNameSegmentBuff[bWord_offset * 2] = 0x2e; //'.'
                                pbNameSegmentBuff[(bWord_offset * 2) + 1] = 0x00;

                                f_rightmost_dot_found = TRUE;
                            }
                            f_rightmost_dot_well_processed = TRUE;

                            bWord_offset++;
                            if (12 < bWord_offset) /* position not in current long-name FDB node, in next one */
                            {
                                bWord_offset -= 13;
                                goto L_next_longname_FDB_node; //continue;
                            }
                        }

                        if (f_rightmost_dot_well_processed)
                        {
                            while ((BYTE) *pbExt_char != 0x00)
                            {
                                if (bWord_offset <= 4)
                                    pbNameSegmentBuff = lnode->Name0;
                                else if ((5 <= bWord_offset) && (bWord_offset <= 10))
                                {
                                    bWord_offset -= 5;
                                    pbNameSegmentBuff = lnode->Name1;
                                }
                                else if ((11 <= bWord_offset) && (bWord_offset <= 12))
                                {
                                    bWord_offset -= 11;
                                    pbNameSegmentBuff = lnode->Name2;
                                }

                                pbNameSegmentBuff[bWord_offset * 2] = (BYTE) *pbExt_char;
                                pbNameSegmentBuff[(bWord_offset * 2) + 1] = 0x00;

                                pbExt_char++;
                                bWord_offset++;

                                if (12 < bWord_offset) /* position not in current long-name FDB node, in next one */
                                {
                                    bWord_offset -= 13;
                                    goto L_next_longname_FDB_node; //continue;
                                }
                            }
                        }

                        /* if first = 0x4n mean this node is the last node of this long file name, n is its order */
                        if (first & 0x40)
                        {
                            if (! f_rightmost_dot_well_processed)
                            {
                                MP_ALERT("%s: last long-name FDB node reached, but extension name not processed yet => Something wrong on the storage file system !", __FUNCTION__);
                                return ABNORMAL_STATUS;
                            }

                            break;
                        }
                    }
                }
            } while ((first != 0xe5) && (first != 0x2e) && (attr == FDB_LONG_NAME));
        }
        else  /* not long filename */
            ext_mem_free(tmp_FilenameBuf);
    }
#if EXFAT_ENABLE
    else
    {
        TYPE_exFAT_Stream_Extension_DirEntry *exfat_node;

        /* current drv->Node should be a "Stream Extension DirectoryEntry" of exFAT */
        exfat_node = (TYPE_exFAT_Stream_Extension_DirEntry *) (drv->Node);
        if (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT)
        {
            MP_ALERT("%s: Error! Current node should be a (Stream Extension DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
            return ABNORMAL_STATUS;
        }

        /* exFAT does not support 8.3 ASCII short name, need to search the Unicode long filename of the file */
        mpDebugPrint("++++ %s: To-Do: how to process renaming for file extension name on exFAT drive ???", __FUNCTION__);

    }
#endif

    drv->Flag.DirCacheChanged = 1;

    // here, write back changed DirCache
    if (DriveRefresh(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E- DriveRefresh() failed to write back changed Dir Cache content !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    return FS_SUCCEED;
}



///
///@ingroup FILE_BROWSER
///@brief   Sort the order of files in a file list according to the specified sorting basis.
///
///@param   sSearchInfo         The pointer of ST_SEARCH_INFO list array to be sorted. \n\n
///@param   dwElementCount      The total count of entries in the list. \n\n
///@param   sorting_basis       The sorting basis to according to.
///
///@retval  FS_SUCCEED          Sorting successfully. \n\n
///@retval  ABNORMAL_STATUS     Sorting unsuccessfully due to some error.
///
SWORD FileSort(ST_SEARCH_INFO * sSearchInfo, DWORD dwElementCount, FILE_SORTING_BASIS_TYPE sorting_basis)
{
    SWORD ret;

    if (dwElementCount < 2)
    {
        MP_DEBUG("%s: dwElementCount (=%d) < 2 => return.", __FUNCTION__, dwElementCount);
        return FS_SUCCEED;
    }

    if (sSearchInfo == NULL)
    {
        MP_ALERT("%s: Error! NULL ST_SEARCH_INFO pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

#if EXFAT_ENABLE
    DRIVE *drv = DriveGet(sSearchInfo->DrvIndex);
    ST_SEARCH_INFO *file_list_entry;
    WORD *utf16_name_buf_ptr;
    DWORD name_len;
    int i;

    if (((sorting_basis == FILE_SORTING_BY_83SHORT_NAME) || (sorting_basis == FILE_SORTING_BY_83SHORT_NAME_REVERSE)) && (sSearchInfo->FsType == FS_TYPE_exFAT))
    {
        MP_ALERT("%s: getting UTF-16 filename for each file list entry from the exFAT drive ...", __FUNCTION__);
        for (i = 0; i < dwElementCount; i++)
        {
            file_list_entry = (ST_SEARCH_INFO *) &sSearchInfo[i];

            /* special processing for ".." folder node */
            if ( (file_list_entry->bParameter == SEARCH_INFO_CHANGE_PATH) && (file_list_entry->bFileType == FILE_OP_TYPE_FOLDER) )
            {
                file_list_entry->utf16_name = NULL;
                continue;
            }

            utf16_name_buf_ptr = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
            if (utf16_name_buf_ptr == NULL)
            {
                MP_ALERT("%s: -E malloc fail for filename buffer ! => Not do sorting by name !", __FUNCTION__);
                ret = ABNORMAL_STATUS;
                goto L_release_name_buffers;
            }
            //MpMemSet((BYTE *) utf16_name_buf_ptr, 0, MAX_L_NAME_LENG * 2);

            if (FileGetLongName(drv, file_list_entry, (BYTE *) utf16_name_buf_ptr, &name_len, MAX_L_NAME_LENG * 2) != FS_SUCCEED)
            {
                MP_ALERT("%s: FileGetLongName() failed ! => Not do sorting by name !", __FUNCTION__);
                ret = ABNORMAL_STATUS;
                goto L_release_name_buffers;
            }
            file_list_entry->utf16_name = utf16_name_buf_ptr;
        }
        MP_ALERT("%s: getting UTF-16 filename for each file list entry from the exFAT drive is finished.", __FUNCTION__);
    }
#endif

    if (sorting_basis == FILE_SORTING_BY_83SHORT_NAME)
        ret = QuickSort(sSearchInfo, dwElementCount, sizeof(ST_SEARCH_INFO), CompareFileName, 0);
    else if (sorting_basis == FILE_SORTING_BY_83SHORT_NAME_REVERSE)
        ret = QuickSort(sSearchInfo, dwElementCount, sizeof(ST_SEARCH_INFO), CompareFileName, 1);
    else if (sorting_basis == FILE_SORTING_BY_DATE_TIME)
        ret = QuickSort(sSearchInfo, dwElementCount, sizeof(ST_SEARCH_INFO), CompareFileDateTime, 0);
    else if (sorting_basis == FILE_SORTING_BY_DATE_TIME_REVERSE)
        ret = QuickSort(sSearchInfo, dwElementCount, sizeof(ST_SEARCH_INFO), CompareFileDateTime, 1);
    else if (sorting_basis == FILE_SORTING_BY_SIZE)
        ret = QuickSort(sSearchInfo, dwElementCount, sizeof(ST_SEARCH_INFO), CompareFileSize, 0);
    else if (sorting_basis == FILE_SORTING_BY_SIZE_REVERSE)
        ret = QuickSort(sSearchInfo, dwElementCount, sizeof(ST_SEARCH_INFO), CompareFileSize, 1);
    else if (sorting_basis == FILE_SORTING_BY_EXIF_DATE_TIME)
        ret = QuickSort(sSearchInfo, dwElementCount, sizeof(ST_SEARCH_INFO), CompareImgEXIFDateTime, 0);
    else if (sorting_basis == FILE_SORTING_BY_EXIF_DATE_TIME_REVERSE)
        ret = QuickSort(sSearchInfo, dwElementCount, sizeof(ST_SEARCH_INFO), CompareImgEXIFDateTime, 1);
#if (SONY_DCF_ENABLE)
    else if (sorting_basis == FILE_SORTING_BY_SONY_DCF_NAMING)
        ret = QuickSort(sSearchInfo, dwElementCount, sizeof(ST_SEARCH_INFO), CompareDCFFileDisplayName, 0);
    else if (sorting_basis == FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE)
        ret = QuickSort(sSearchInfo, dwElementCount, sizeof(ST_SEARCH_INFO), CompareDCFFileDisplayName, 1);
#endif

#if EXFAT_ENABLE
L_release_name_buffers:
    if (((sorting_basis == FILE_SORTING_BY_83SHORT_NAME) || (sorting_basis == FILE_SORTING_BY_83SHORT_NAME_REVERSE)) && (sSearchInfo->FsType == FS_TYPE_exFAT))
    {
        for (i = 0; i < dwElementCount; i++)
        {
            file_list_entry = (ST_SEARCH_INFO *) &sSearchInfo[i];
            if (file_list_entry->utf16_name)
            {
                ext_mem_free(file_list_entry->utf16_name);
                file_list_entry->utf16_name = NULL;
            }
        }
    }
#endif

    return ret;
}



///
///@ingroup FILE_BROWSER
///@brief   Get the long filename UTF-16 string of the specified file entry.
///
///@param   sDrv                [IN] The drive on which the specified file resides. \n\n
///@param   sSearchInfo         [IN] The ST_SEARCH_INFO pointer to a file entry to get its long filename. \n\n
///@param   pbBuf               [OUT] The pointer of an external buffer for storing the UTF-16 long filename. \n\n
///@param   pdwBufferCount      [OUT] The count of returned data bytes in the long filename buffer, not include string null terminator. \n\n
///@param   dwMaxBufferSize     [IN] The maximun number of bytes the external 'pbBuf' data buffer can contains. 512 (= 256 * 2) bytes is recommended for UTF-16 long name. \n\n
///
///@retval  FS_SUCCEED          Get filename successfully. \n\n
///@retval  ABNORMAL_STATUS     Get filename unsuccessfully. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
///@remark  If the specified file has no any long name FDB nodes, then it is actually a short-name file.
///         For such case, this function still returns FS_SUCCEED.
///
SWORD FileGetLongName(DRIVE * sDrv, ST_SEARCH_INFO * sSearchInfo, BYTE * pbBuf, DWORD * pdwBufferCount, DWORD dwMaxBufferSize)
{
    int ret;

    if (sDrv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if ((sSearchInfo == NULL) || (pbBuf == NULL) || (pdwBufferCount == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (dwMaxBufferSize == 0)
    {
        MP_DEBUG("%s: invalid input parameter !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* initialize return values */
    MpMemSet((BYTE *) pbBuf, 0, dwMaxBufferSize);
    *pdwBufferCount = 0;

    if (sSearchInfo->dwLongFdbCount == 0)
    {
        if (sDrv->Flag.FsType != FS_TYPE_exFAT)
        {
            MP_ALERT("%s: -I- this entry has no UTF-16 long filename.", __FUNCTION__);
            return FS_SUCCEED;
        }
#if EXFAT_ENABLE
        else
        {
            MP_ALERT("%s: Error! sSearchInfo->dwLongFdbCount must be non-zero for exFAT file/directory !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
#endif
    }

    ret = Locate_DirEntryNode_of_SearchInfo(sDrv, sSearchInfo);
    if ((ret != FS_SUCCEED) && (ret != FILE_NOT_FOUND))
    {
        MP_ALERT("%s: Error! Locate_DirEntryNode_of_SearchInfo() failed !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
    else if (ret == FILE_NOT_FOUND) /* here, use FILE_NOT_FOUND for [Upper Folder] case */
    {
        MP_ALERT("%s: This is an [Upper Folder] entry !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    DWORD bufByteCount = 0;
    ENTRY_FILENAME_TYPE  bFilenameType = E_FILENAME_UNKNOWN;
    ret = GetFilenameOfCurrentNode(sDrv, &bFilenameType, pbBuf, &bufByteCount, dwMaxBufferSize);
    if (ret != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! GetFilenameOfCurrentNode() failed !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
    *pdwBufferCount = StringLength16((WORD *) pbBuf);

  #if 0 //just for debug purpose
    {
        BYTE tmpBuf[512];
        MpMemSet(tmpBuf, 0, 512);
        mpx_UtilU16ToU08(tmpBuf, (WORD *) pbBuf); /* convert UTF-16 to UTF-8 string */
        MP_DEBUG("Filename of entry = %s  (len = %d bytes)\r\n", tmpBuf, bufByteCount);
    }
  #endif

    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Get filename of current directory entry implicitly pointed to by current FDB node (drv->Node) on the drive.
///
///@param   drv                 [IN] The drive on which the specified directory entry resides. \n\n
///
///@param   pbFilenameType      [OUT] Returned long or short name type of the filename. \n
///                                   Valid values are: \n
///                                     E_FILENAME_8_3_SHORT_NAME is for short name ASCII string; \n
///                                     E_FILENAME_UTF16_LONG_NAME is for long name UTF-16 string; \n
///                                     E_FILENAME_UNKNOWN if this function failed. \n\n
///
///@param   pbBuf               [OUT] The pointer of an external buffer for storing the output filename string. \n\n
///
///@param   pdwBufferCount      [OUT] The count of returned filename data bytes (not include string null terminator) in the 'pbBuf' data buffer. \n\n
///
///@param   dwMaxBufferSize     [IN] The maximun number of bytes the external 'pbBuf' data buffer can contains. 512 (= 256 * 2) bytes is recommended for UTF-16 long name. \n\n
///
///@retval  FS_SUCCEED          Get filename successfully. \n\n
///@retval  ABNORMAL_STATUS     Get filename unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE       Invalid drive. \n\n
///
///@remark  The directory entry to get its filename is implicitly pointed to by current FDB node (drv->Node) before calling this function.
///
SWORD GetFilenameOfCurrentNode(DRIVE * drv, ENTRY_FILENAME_TYPE * pbFilenameType, BYTE * pbBuf, DWORD * pdwBufferCount, DWORD dwMaxBufferSize)
{
    DWORD dwLongFdbCount, dwBufBytesCount, i;
    LONG_NAME *sLongName;
    FDB   *fdb_node;
    BOOL  f_IsLongFilename;
    ST_SAVE_FDB_POS  fdb_pos_info;
    BOOL  f_needRecoverPosition;
    int   ret;


    /* initialize for return values */
    *pbFilenameType = E_FILENAME_UNKNOWN;
    *pdwBufferCount = 0;

    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        f_needRecoverPosition = FALSE;
        ret = INVALID_DRIVE;
        goto L_ending_process;
    }

    if ((pbBuf == NULL) || (pdwBufferCount == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        f_needRecoverPosition = FALSE;
        ret = ABNORMAL_STATUS;
        goto L_ending_process;
    }

    if (dwMaxBufferSize == 0)
    {
        MP_DEBUG("%s: invalid input parameter !", __FUNCTION__);
        f_needRecoverPosition = FALSE;
        ret = ABNORMAL_STATUS;
        goto L_ending_process;
    }

    f_needRecoverPosition = TRUE;

    /* save current FDB node position and DirCache cache point info */
    SaveCurrentDirNodePosition(drv, &fdb_pos_info);

    MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2);
    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        fdb_node = (FDB *) drv->Node;
        if (fdb_node->Attribute == FDB_LONG_NAME) /* this FDB node is a long name FDB node */
        {
            f_IsLongFilename = TRUE;

            /* calculate count of long name FDBs of this directory entry */
            do
            {
                sLongName = (LONG_NAME *) (drv->Node);
                if (((sLongName->Attribute1 == FDB_LONG_NAME) && (sLongName->Attribute2 == 0)) && (sLongName->Number & 0x40))
                {
                    /* if first = 0x4n mean this node is the last node of this long file name, n is its order */
                    dwLongFdbCount = sLongName->Number - 0x40;

                    /* note: Do not simply use DirNext() here! Because DirNext() calls NextNode() first, and later check
                     * whether if (ScanFileName(drv) == FILENAME_SCAN_CONTINUE) for long filename. But the first NextNode() call
                     * in DirNext() will move the drv-Node from current '0x4n' long-name node to its next node, and then cause
                     * the ScanFileName() missed the '0x4n' info !!
                     */

                    /* go to the short-name FDB node position of this directory entry */
                    for (i = 0; i < dwLongFdbCount; i++)
                    {
                        if (NextNode(drv) != FS_SUCCEED)
                        {
                            MP_ALERT("%s: Error! NextNode() failed on %s drive !", __FUNCTION__, DriveIndex2DrvName(drv->DrvIndex));
                            ret = ABNORMAL_STATUS;
                            goto L_ending_process;
                        }
                    }

                    goto L_return_filename_process; /* process filename for return value */
                }

                ret = PreviousNode(drv);
                if (ret == BEGIN_OF_DIR) /* already at the first node of the directory */
                {
                    MP_ALERT("%s: Abnormal ! A long name directory entry is corrupt !  Please check the file system of the drive.", __FUNCTION__);
                    f_IsLongFilename = FALSE; /* corrupt => treat it as short name */
                    dwLongFdbCount = 0;

                    /* go to the short name FDB node of this long name directory entry */
                    if (DirNext(drv) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Error ! DirNext() failed !", __FUNCTION__);
                        ret = ABNORMAL_STATUS;
                        goto L_ending_process;
                    }
                    else
                        break;
                }
                else if (ret != FS_SUCCEED)
                {
                    MP_ALERT("%s: Error! PreviousNode() failed !", __FUNCTION__);
                    ret = ABNORMAL_STATUS;
                    goto L_ending_process;
                }

                fdb_node = (FDB *) drv->Node;
            } while (fdb_node->Attribute == FDB_LONG_NAME);
        }
        else /* this FDB node is a 8.3 short name FDB node => check its previous nodes for possible a long filename directory entry */
        {
            /* check '.' and '..' node cases */
            if (fdb_node->Name[0] == '.')
            {
                MpMemSet(pbBuf, 0, dwMaxBufferSize);
                if ((fdb_node->Name[1] == ' ') || (fdb_node->Name[1] == 0x00)) /* '.' node */
                {
                    pbBuf[0] = '.';
                    *pdwBufferCount = 1; //only 1 byte
                }
                else if ((fdb_node->Name[1] == '.') && ((fdb_node->Name[2] == ' ') || (fdb_node->Name[2] == 0x00))) /* '..' node */
                {
                    pbBuf[0] = '.';
                    pbBuf[1] = '.';
                    *pdwBufferCount = 2; //only 2 bytes
                }

                *pbFilenameType = E_FILENAME_8_3_SHORT_NAME;
                ret = FS_SUCCEED;
                f_needRecoverPosition = FALSE; /* this is a single short name FDB node */
                goto L_ending_process;
            }

            /* check Volume Label node case */
            if (fdb_node->Attribute & FDB_LABEL)
            {
                BYTE *ch_ptr = (BYTE *) &drv->Node->Name[0];

                /* process filename for return value */
                dwBufBytesCount = 0;
                MpMemSet(pbBuf, 0, dwMaxBufferSize);
                for (i=0; i < 11; i++)
                {
                    if ((ch_ptr[i] != ' ') && (ch_ptr[i] != 0x00))
                        pbBuf[dwBufBytesCount++] = ch_ptr[i];
                    else
                        break;
                }
                pbBuf[dwBufBytesCount] = 0x00; //null terminator of string

                /* return values */
                *pbFilenameType = E_FILENAME_8_3_SHORT_NAME;
                *pdwBufferCount = dwBufBytesCount;
                ret = FS_SUCCEED;
                f_needRecoverPosition = FALSE; /* this is a single short name FDB node */
                goto L_ending_process;
            }

            BYTE bChkSum;
            BYTE bShortNameBuf[11];
            MpMemCopy(bShortNameBuf, fdb_node->Name, 8);
            MpMemCopy(bShortNameBuf + 8, fdb_node->Extension, 3);

            /* generate checksum */
            bChkSum = 0;
            for (i = 0; i < 11; i++)
            {
                //NOTE: The operation is an unsigned char rotate right
                bChkSum = ((bChkSum & 1) ? 0x80 : 0) + (bChkSum >> 1) + bShortNameBuf[i];
            }

            ret = PreviousNode(drv);
            if (ret == BEGIN_OF_DIR) /* already at the first node of the directory */
            {
                f_IsLongFilename = FALSE;
                dwLongFdbCount = 0;
                f_needRecoverPosition = FALSE; /* this is a single short name FDB node */
            }
            else if (ret == FS_SUCCEED)
            {
                fdb_node = (FDB *) drv->Node;
                if (fdb_node->Attribute == FDB_LONG_NAME) /* previous FDB node is a long name FDB node */
                {
                    sLongName = (LONG_NAME *) (drv->Node);
                    if (sLongName->CheckSum == bChkSum) /* the directory entry is actually a long filename entry */
                    {
                        f_IsLongFilename = TRUE;

                        /* calculate count of long name FDBs of this directory entry */
                        do
                        {
                            sLongName = (LONG_NAME *) (drv->Node);
                            if (((sLongName->Attribute1 == FDB_LONG_NAME) && (sLongName->Attribute2 == 0)) && (sLongName->Number & 0x40))
                            {
                                /* if first = 0x4n mean this node is the last node of this long file name, n is its order */
                                dwLongFdbCount = sLongName->Number - 0x40;

                                /* note: Do not simply use DirNext() here! Because DirNext() calls NextNode() first, and later check
                                 * whether if (ScanFileName(drv) == FILENAME_SCAN_CONTINUE) for long filename. But the first NextNode() call
                                 * in DirNext() will move the drv-Node from current '0x4n' long-name node to its next node, and then cause
                                 * the ScanFileName() missed the '0x4n' info !!
                                 */

                                /* go to the short-name FDB node position of this directory entry */
                                for (i = 0; i < dwLongFdbCount; i++)
                                {
                                    if (NextNode(drv) != FS_SUCCEED)
                                    {
                                        MP_ALERT("%s: Error! NextNode() failed on %s drive !", __FUNCTION__, DriveIndex2DrvName(drv->DrvIndex));
                                        ret = ABNORMAL_STATUS;
                                        goto L_ending_process;
                                    }
                                }

                                goto L_return_filename_process; /* process filename for return value */
                            }

                            ret = PreviousNode(drv);
                            if (ret == BEGIN_OF_DIR) /* already at the first node of the directory */
                            {
                                MP_ALERT("%s: Abnormal ! A long name directory entry is corrupt !  Please check the file system of the drive.", __FUNCTION__);
                                f_IsLongFilename = FALSE; /* corrupt => treat it as short name */
                                dwLongFdbCount = 0;

                                /* go to the short name FDB node of this long name directory entry */
                                if (DirNext(drv) != FS_SUCCEED)
                                {
                                    MP_ALERT("%s: Error ! DirNext() failed !", __FUNCTION__);
                                    ret = ABNORMAL_STATUS;
                                    goto L_ending_process;
                                }
                                else
                                    break;
                            }
                            else if (ret != FS_SUCCEED)
                            {
                                MP_ALERT("%s: Error! PreviousNode() failed !", __FUNCTION__);
                                ret = ABNORMAL_STATUS;
                                goto L_ending_process;
                            }

                            fdb_node = (FDB *) drv->Node;
                        } while (fdb_node->Attribute == FDB_LONG_NAME);
                    }
                    else
                    {
                        MP_ALERT("%s: Abnormal ! A long name directory entry is corrupt !  Please check the file system of the drive.", __FUNCTION__);
                        f_IsLongFilename = FALSE; /* corrupt => treat it as short name */
                        dwLongFdbCount = 0;

                        /* go to the short name FDB node of this 'corrupt' long name directory entry */
                        if (NextNode(drv) != FS_SUCCEED)
                        {
                            MP_ALERT("%s: Error! NextNode() failed on %s drive !", __FUNCTION__, DriveIndex2DrvName(drv->DrvIndex));
                            ret = ABNORMAL_STATUS;
                            goto L_ending_process;
                        }

                        f_needRecoverPosition = FALSE; /* now, already go back to original node */
                    }
                }
                else
                {
                    f_IsLongFilename = FALSE;
                    /* go to the original FDB node of the short name directory entry */
                    if (NextNode(drv) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Error! NextNode() failed on %s drive !", __FUNCTION__, DriveIndex2DrvName(drv->DrvIndex));
                        ret = ABNORMAL_STATUS;
                        goto L_ending_process;
                    }

                    f_needRecoverPosition = FALSE; /* this is a single short name FDB node */
                }
            }
            else
            {
                MP_ALERT("%s: Error! PreviousNode() failed !", __FUNCTION__);
                ret = ABNORMAL_STATUS;
                f_needRecoverPosition = FALSE; /* didn't move due to PreviousNode() failed */
                goto L_ending_process;
            }
        }

L_return_filename_process:
        /* process filename for return value */
        dwBufBytesCount = 0;
        MpMemSet(pbBuf, 0, dwMaxBufferSize);
        if ((f_IsLongFilename == FALSE) || (dwLongFdbCount == 0))
        {
            for (i=0; i < 8; i++)
            {
                if ((drv->Node->Name[i] != ' ') && (drv->Node->Name[i] != 0x00))
                    pbBuf[dwBufBytesCount++] = drv->Node->Name[i];
                else
                    break;
            }
            if ((drv->Node->Extension[0] != ' ') && (drv->Node->Extension[0] != 0x00))
            {
                pbBuf[dwBufBytesCount++] = '.';
                for (i=0; i < 3; i++)
                {
                    if ((drv->Node->Extension[i] != ' ') && (drv->Node->Extension[i] != 0x00))
                        pbBuf[dwBufBytesCount++] = drv->Node->Extension[i];
                    else
                        break;
                }
            }
            pbBuf[dwBufBytesCount] = 0x00; //null terminator of string

            /* return values */
            *pbFilenameType = E_FILENAME_8_3_SHORT_NAME;
            *pdwBufferCount = dwBufBytesCount;
            ret = FS_SUCCEED;
            goto L_ending_process;
        }
        else /* process UTF-16 long filename */
        {
            /* return values */
            *pbFilenameType = E_FILENAME_UTF16_LONG_NAME;

            while (dwLongFdbCount)
            {
                ret = PreviousNode(drv);
                if (ret != FS_SUCCEED)	// begin of FDB or access fail
                {
                    MP_ALERT("%s: Error! PreviousNode() failed !", __FUNCTION__);
                    ret = ABNORMAL_STATUS;
                    goto L_ending_process;
                }

                sLongName = (LONG_NAME *) (drv->Node);
                for (i = 0; i < 10; i += 2)
                {
                    if (((sLongName->Name0[i] == 0x00) && (sLongName->Name0[i + 1] == 0x00))
                        || (dwBufBytesCount > (dwMaxBufferSize - 2)))
                    {
                        pbBuf[dwBufBytesCount] = 0x00;
                        pbBuf[dwBufBytesCount + 1] = 0x00;
                        *pdwBufferCount = dwBufBytesCount;
                        ret = FS_SUCCEED;

                        MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2);
                        MpMemCopy((BYTE *) drv->LongName, pbBuf, dwBufBytesCount + 1);
                        goto L_ending_process;
                    }

                    pbBuf[dwBufBytesCount + 1] = sLongName->Name0[i];
                    pbBuf[dwBufBytesCount] = sLongName->Name0[i + 1];
                    dwBufBytesCount += 2;
                }

                for (i = 0; i < 12; i += 2)
                {
                    if (((sLongName->Name1[i] == 0x00) && (sLongName->Name1[i + 1] == 0x00))
                        || (dwBufBytesCount > (dwMaxBufferSize - 2)))
                    {
                        pbBuf[dwBufBytesCount] = 0x00;
                        pbBuf[dwBufBytesCount + 1] = 0x00;
                        *pdwBufferCount = dwBufBytesCount;
                        ret = FS_SUCCEED;

                        MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2);
                        MpMemCopy((BYTE *) drv->LongName, pbBuf, dwBufBytesCount + 1);
                        goto L_ending_process;
                    }

                    pbBuf[dwBufBytesCount + 1] = sLongName->Name1[i];
                    pbBuf[dwBufBytesCount] = sLongName->Name1[i + 1];
                    dwBufBytesCount += 2;
                }

                for (i = 0; i < 4; i += 2)
                {
                    if (((sLongName->Name2[i] == 0x00) && (sLongName->Name2[i + 1] == 0x00))
                        || (dwBufBytesCount > (dwMaxBufferSize - 2)))
                    {
                        pbBuf[dwBufBytesCount] = 0x00;
                        pbBuf[dwBufBytesCount + 1] = 0x00;
                        *pdwBufferCount = dwBufBytesCount;
                        ret = FS_SUCCEED;

                        MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2);
                        MpMemCopy((BYTE *) drv->LongName, pbBuf, dwBufBytesCount + 1);
                        goto L_ending_process;
                    }

                    pbBuf[dwBufBytesCount + 1] = sLongName->Name2[i];
                    pbBuf[dwBufBytesCount] = sLongName->Name2[i + 1];
                    dwBufBytesCount += 2;
                }

                dwLongFdbCount -= 1;
            }

            *pdwBufferCount = dwBufBytesCount;
            pbBuf[dwBufBytesCount] = 0x00;
            pbBuf[dwBufBytesCount + 1] = 0x00;
            ret = FS_SUCCEED;

            MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2);
            MpMemCopy((BYTE *) drv->LongName, pbBuf, dwBufBytesCount + 1);
            goto L_ending_process;
        }
    }
#if EXFAT_ENABLE
    else
    {
        /* initialize return values */
        *pbFilenameType = E_FILENAME_UTF16_LONG_NAME; /* exFAT does not support 8.3 short filename */
        *pdwBufferCount = 0;
        MpMemSet(pbBuf, 0, dwMaxBufferSize);

        TYPE_exFAT_Generic_DirEntry_Template *exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) (drv->Node);

        /* check if current FDB node is a file-related exFAT DirEntry */
        if ((exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE) && (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT) &&
            (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE_NAME))
        {
            MP_ALERT("%s: Error! Current FDB node is not a file-related exFAT DirEntry ! its entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
            return ABNORMAL_STATUS;
        }

        if (exfat_node->entryType == EXFAT_ENTRY_TYPE_FILE)
        {
            /* move to corresponding "Stream Extension DirEntry" position of current "File DirEntry" */
            if (NextNode(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! Failed to move to [Stream Extension DirEntry] of this [File DirEntry] !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }

            exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) drv->Node;
        }
        else if (exfat_node->entryType == EXFAT_ENTRY_TYPE_FILE_NAME)
        {
            do
            {
                ret = PreviousNode(drv);
                if (ret == FS_SUCCEED)
                {
                    exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) drv->Node;
                    if (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE_NAME)
                        break;
                }
                else
                {
                    MP_ALERT("%s: Error! Failed to move to [Stream Extension DirEntry] of this [File Name DirEntry] !", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }
            } while (ret == FS_SUCCEED);
        }

        /* now, current FDB node should be an exFAT "Stream Extension DirEntry" */
        if (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT)
        {
            MP_ALERT("%s: Error! Current node is not an exFAT [Stream Extension DirEntry] ! Pls check its file system ...", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            BYTE name_len, fileName_FdbCount;
            WORD name_hash, *wCh_src, *wCh_dst;
            BYTE counted_name_len = 0, j;
            WORD calculated_name_hash;

            name_len = (BYTE) ((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->nameLength;
            name_hash = (WORD) LoadAlien16(&(((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->nameHash));
            if (name_len % EXFAT_ENTRY_FILE_NAME_LEN)
                fileName_FdbCount = (name_len / EXFAT_ENTRY_FILE_NAME_LEN) + 1;
            else
                fileName_FdbCount = (name_len / EXFAT_ENTRY_FILE_NAME_LEN);

        #if 1  /* caching useful info of current file/directory for later reference if needed */
            drv->CacheBufPoint->LongNameFdbCount = fileName_FdbCount;
            drv->CacheBufPoint->nameLength = (BYTE) ((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->nameLength;
            drv->CacheBufPoint->nameHash = (WORD) LoadAlien16(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->nameHash);
            drv->CacheBufPoint->firstCluster = (DWORD) LoadAlien32(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->firstCluster);

            BYTE *Hi32_of_64, *Lo32_of_64;
            Lo32_of_64 = (BYTE *) &((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->dataLength; /* due to little-endian */
            Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
            drv->CacheBufPoint->dataLength = (U64) ((U64) LoadAlien32(Hi32_of_64) << 32) + (U64) LoadAlien32(Lo32_of_64);
        #endif

            for (i = 0; (i < fileName_FdbCount) && (counted_name_len < name_len); i++)
            {
                ret = NextNode(drv);
                if (ret != FS_SUCCEED)
                {
                    MP_ALERT("%s: Error! NextNode() failed on %s drive !", __FUNCTION__, DriveIndex2DrvName(drv->DrvIndex));
                    MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2); /* clean cached partial filename */
                    return ABNORMAL_STATUS;
                }

                /* current drv->Node should be a "File Name DirectoryEntry" of exFAT */
                (TYPE_exFAT_File_Name_DirEntry *) exfat_node = (TYPE_exFAT_File_Name_DirEntry *) (drv->Node);
                if (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE_NAME)
                {
                    MP_ALERT("%s: Error! Current node should be a (File Name DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
                    MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2); /* clean cached partial filename */
                    return ABNORMAL_STATUS;
                }

                for (j = 0; (j < EXFAT_ENTRY_FILE_NAME_LEN) && (counted_name_len < name_len); j++)
                {
                    wCh_src = (WORD *) ((WORD *) ((TYPE_exFAT_File_Name_DirEntry *) exfat_node)->fileName + j);
                    wCh_dst = (WORD *) ((WORD *) drv->LongName + counted_name_len);
                    (WORD) *wCh_dst = LoadAlien16((WORD *) wCh_src);
                    counted_name_len++;
                }
            }

        #if 0 //just for debug purpose
            {
                BYTE tmpBuf[512];
                MpMemSet(tmpBuf, 0, 512);
                mpx_UtilU16ToU08(tmpBuf, (WORD *) drv->LongName); /* convert UTF-16 to UTF-8 string */
                MP_DEBUG("%s: Filename of entry = %s  (name len = %d characters)", __FUNCTION__, tmpBuf, name_len);
            }
        #endif

            /* return values */
            *pbFilenameType = E_FILENAME_UTF16_LONG_NAME; /* exFAT does not support 8.3 short filename */
            *pdwBufferCount = MIN((name_len * 2), dwMaxBufferSize);
            MpMemCopy(pbBuf, (BYTE *) drv->LongName, *pdwBufferCount);

            calculated_name_hash = Cal_exFatFilenameHash((WORD *) drv->LongName, name_len);
            if (calculated_name_hash != name_hash)
            {
                MP_ALERT("%s: Error! calculated filename hash (= 0x%04x) mismatch with filename hash in DirEntry (= 0x%04x) !", __FUNCTION__, calculated_name_hash, name_hash);

                /* reset return values */
                *pdwBufferCount = 0;
                MpMemSet(pbBuf, 0, dwMaxBufferSize);
                return ABNORMAL_STATUS;
            }

            ret = FS_SUCCEED;
        }
    }
#endif

L_ending_process:
    if (f_needRecoverPosition == TRUE)
    {
        /* recover original FDB node position and DirCache cache point info */
        RestoreDirNodePosition(drv, &fdb_pos_info);
    }

    if (ret != FS_SUCCEED)
    {
        /* reset return values */
        if (drv->Flag.FsType != FS_TYPE_exFAT)
            *pbFilenameType = E_FILENAME_8_3_SHORT_NAME;

        *pdwBufferCount = 0;
        MpMemSet(pbBuf, 0, dwMaxBufferSize);
    }

    return ret;
}



///
///@ingroup FILE
///@brief   Get the 8.3 ASCII short filename of current directory entry implicitly pointed to by current FDB node (drv->Node) on the drive. \n
///         The returned 8.3 ASCII short filename is copied from the 8.3 short-name FDB node directly, and cut the space characters off. \n\n
///
///@param   drv                 [IN] The drive on which the specified directory entry resides. \n\n
///
///@param   pbBuf               [OUT] The pointer of an external buffer for storing the output 8.3 ASCII short filename string. \n\n
///
///@param   pdwBufferCount      [OUT] The count of returned filename data bytes (not include string null terminator) in the 'pbBuf' data buffer. \n\n
///
///@param   dwMaxBufferSize     [IN] The maximun number of bytes the external 'pbBuf' data buffer can contains. \n
///                                  We ask this must be at least 13 bytes (=> 8 + 3 + 'dot' + 'null terminator') for 8.3 ASCII short filename. \n
///                                  If the buffer size is less than 13, we will treat it as error directly and return ABNORMAL_STATUS. \n\n
///
///@retval  FS_SUCCEED          Get 8.3 short filename successfully. \n\n
///@retval  ABNORMAL_STATUS     Get 8.3 short filename unsuccessfully due to some error. \n\n
///@retval  INVALID_DRIVE       Invalid drive, or the file system on the drive is exFAT file system case. \n\n
///
///@remark  The directory entry to get its filename is implicitly pointed to by current FDB node (drv->Node) before calling this function.
///
///@remark  Because exFAT file system does not support 8.3 short filename any more, this function always return INVALID_DRIVE for exFAT drive case \n
///         and set output 'pdwBufferCount' value to 0. \n\n
///
SWORD GetAsciiShortFilenameOfCurrentNode(DRIVE * drv, BYTE * pbBuf, DWORD * pdwBufferCount, DWORD dwMaxBufferSize)
{
    DWORD dwBufBytesCount, i;
    FDB   *fdb_node;
    ST_SAVE_FDB_POS  fdb_pos_info;
    BOOL  f_needRecoverPosition;
    int   ret;


    /* initialize for return values */
    *pdwBufferCount = 0;

    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        f_needRecoverPosition = FALSE;
        ret = INVALID_DRIVE;
        goto L_return_ending_process;
    }

    if ((pbBuf == NULL) || (pdwBufferCount == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        f_needRecoverPosition = FALSE;
        ret = ABNORMAL_STATUS;
        goto L_return_ending_process;
    }

    if (dwMaxBufferSize == 0)
    {
        MP_DEBUG("%s: invalid input parameter 'dwMaxBufferSize' == 0 !", __FUNCTION__);
        f_needRecoverPosition = FALSE;
        ret = ABNORMAL_STATUS;
        goto L_return_ending_process;
    }

    if (dwMaxBufferSize < 13) /* 8 + 3 + 'dot' + 'null terminator' == 13 bytes */
    {
        MP_DEBUG("%s: input parameter 'dwMaxBufferSize' less than 13 bytes => buffer may be too small, treat it as error !", __FUNCTION__);
        f_needRecoverPosition = FALSE;
        ret = ABNORMAL_STATUS;
        goto L_return_ending_process;
    }

    f_needRecoverPosition = TRUE;

    /* save current FDB node position and DirCache cache point info */
    SaveCurrentDirNodePosition(drv, &fdb_pos_info);

    MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2);
    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        fdb_node = (FDB *) drv->Node;
        if (fdb_node->Attribute == FDB_LONG_NAME) /* this FDB node is a long name FDB node */
        {
            /* go to the short-name FDB node position of this directory entry */
            if (DirNext(drv) != FS_SUCCEED)
            {
                MP_DEBUG("%s: DirNext() failed !", __FUNCTION__);
                ret = ABNORMAL_STATUS;
                goto L_return_ending_process;
            }
        }
        else /* this FDB node is a 8.3 short name FDB node */
        {
            /* check '.' and '..' node cases */
            if (fdb_node->Name[0] == '.')
            {
                MpMemSet(pbBuf, 0, dwMaxBufferSize);
                if ((fdb_node->Name[1] == ' ') || (fdb_node->Name[1] == 0x00)) /* '.' node */
                {
                    pbBuf[0] = '.';
                    *pdwBufferCount = 1; //only 1 byte
                }
                else if ((fdb_node->Name[1] == '.') && ((fdb_node->Name[2] == ' ') || (fdb_node->Name[2] == 0x00))) /* '..' node */
                {
                    pbBuf[0] = '.';
                    pbBuf[1] = '.';
                    *pdwBufferCount = 2; //only 2 bytes
                }

                ret = FS_SUCCEED;
                f_needRecoverPosition = FALSE; /* this is a single short name FDB node */
                goto L_return_ending_process;
            }

            /* check Volume Label node case */
            if (fdb_node->Attribute & FDB_LABEL)
            {
                BYTE *ch_ptr = (BYTE *) &drv->Node->Name[0];

                /* process filename for return value */
                dwBufBytesCount = 0;
                MpMemSet(pbBuf, 0, dwMaxBufferSize);
                for (i=0; i < 11; i++)
                {
                    if ((ch_ptr[i] != ' ') && (ch_ptr[i] != 0x00))
                        pbBuf[dwBufBytesCount++] = ch_ptr[i];
                    else
                        break;
                }
                pbBuf[dwBufBytesCount] = 0x00; //null terminator of string

                /* return values */
                *pdwBufferCount = dwBufBytesCount;
                ret = FS_SUCCEED;
                f_needRecoverPosition = FALSE; /* this is a single short name FDB node */
                goto L_return_ending_process;
            }
        }

        /* process 8.3 short filename for return value */
        dwBufBytesCount = 0;
        MpMemSet(pbBuf, 0, dwMaxBufferSize);

        for (i=0; i < 8; i++)
        {
            if ((drv->Node->Name[i] != ' ') && (drv->Node->Name[i] != 0x00))
                pbBuf[dwBufBytesCount++] = drv->Node->Name[i];
            else
                break;
        }
        if ((drv->Node->Extension[0] != ' ') && (drv->Node->Extension[0] != 0x00))
        {
            pbBuf[dwBufBytesCount++] = '.';
            for (i=0; i < 3; i++)
            {
                if ((drv->Node->Extension[i] != ' ') && (drv->Node->Extension[i] != 0x00))
                    pbBuf[dwBufBytesCount++] = drv->Node->Extension[i];
                else
                    break;
            }
        }
        pbBuf[dwBufBytesCount] = 0x00; //null terminator of string

        /* return values */
        *pdwBufferCount = dwBufBytesCount;
        ret = FS_SUCCEED;
        goto L_return_ending_process;
    }
#if EXFAT_ENABLE
    else
    {
        MP_ALERT("%s: -I- exFAT file system does not support 8.3 ASCII short filename any more => return.", __FUNCTION__);
        /* reset return values */
        *pdwBufferCount = 0;
        MpMemSet(pbBuf, 0, dwMaxBufferSize);
        return INVALID_DRIVE;
    }
#endif

L_return_ending_process:
    if (f_needRecoverPosition == TRUE)
    {
        /* recover original FDB node position and DirCache cache point info */
        RestoreDirNodePosition(drv, &fdb_pos_info);
    }

    if (ret != FS_SUCCEED)
    {
        /* reset return values */
        *pdwBufferCount = 0;
        MpMemSet(pbBuf, 0, dwMaxBufferSize);
    }

    return ret;
}



///
///@ingroup FILE
///@brief   Get file attributes, date/time, file size and start cluster info of the specified directory entry.
///
///@param   drv                 [IN] The drive on which the FDB node (or DirEntry) resides. \n\n
///
///@param   fdb_ptr             [IN] The FDB node (or exFAT DirEntry) pointer within current DirCache buffer to a specified directory entry to get its info. \n
///                                  For FAT12/16/32, this parameter should be a FDB type pointer. For exFAT, this should be a pointer to any exFAT DirEntry type. \n
///                                  Cast proper type for this parameter if needed.
///
///@param   dir_entry_info      [OUT] Returned file info (attribute/date/time/size/start cluster) in DIR_ENTRY_INFO_TYPE structure type. \n\n
///
///@retval  PASS          Get info successfully. \n\n
///@retval  FAIL          Get info unsuccessfully. \n\n
///
///@remark    For consistency of the DirCache buffer content to include the FDB node specified by the 'fdb_ptr' parameter, we expect the calling
///           task to feed current FDB node (drv->Node) for the 'fdb_ptr' parameter value.
///
SWORD GetFileAttributeAndTimeInfoFromFDB(DRIVE * drv, FDB * fdb_ptr, DIR_ENTRY_INFO_TYPE * dir_entry_info)
{
    if ((drv == NULL) || (fdb_ptr == NULL) || (dir_entry_info == NULL))
    {
        MP_ALERT("%s: Error ! NULL pointer !", __FUNCTION__);
        return FAIL;
    }

    MpMemSet((BYTE *) dir_entry_info, 0, sizeof(DIR_ENTRY_INFO_TYPE));

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        WORD wYear;
        BYTE bMonth, bDay, bHour, bMinute, bSecond;

        /* file attribute flags */
        if (fdb_ptr->Attribute & FDB_READ_ONLY)
            dir_entry_info->fileAttribute.f_ReadOnly = 1;
        if (fdb_ptr->Attribute & FDB_HIDDEN)
            dir_entry_info->fileAttribute.f_Hidden = 1;
        if (fdb_ptr->Attribute & FDB_SYSTEM)
            dir_entry_info->fileAttribute.f_System = 1;
        if (fdb_ptr->Attribute & FDB_ARCHIVE)
            dir_entry_info->fileAttribute.f_Archive = 1;
        if (fdb_ptr->Attribute & FDB_SUB_DIR)
            dir_entry_info->fileAttribute.f_SubDir = 1;

        /* start cluster */
        dir_entry_info->startCluster = (DWORD) (((DWORD) LoadAlien16((void *) (&fdb_ptr->StartHigh)) << 16) + LoadAlien16((void *) &fdb_ptr->StartLow));

        /* file size */
        if (dir_entry_info->fileAttribute.f_SubDir == 0) /* file entry */
            dir_entry_info->fileSize = (DWORD) LoadAlien32(&fdb_ptr->Size);

        /* file date,time info */
        FileGetDate_for_FATxx(fdb_ptr->CreateDate, &wYear, &bMonth, &bDay);
        FileGetTime_for_FATxx(fdb_ptr->CreateTime, &bHour, &bMinute, &bSecond);
        dir_entry_info->create_date_time.year = wYear;
        dir_entry_info->create_date_time.month = bMonth;
        dir_entry_info->create_date_time.day = bDay;
        dir_entry_info->create_date_time.hour = bHour;
        dir_entry_info->create_date_time.minute = bMinute;
        dir_entry_info->create_date_time.second = bSecond;

        FileGetDate_for_FATxx(fdb_ptr->ModifyDate, &wYear, &bMonth, &bDay);
        FileGetTime_for_FATxx(fdb_ptr->ModifyTime, &bHour, &bMinute, &bSecond);
        dir_entry_info->modify_date_time.year = wYear;
        dir_entry_info->modify_date_time.month = bMonth;
        dir_entry_info->modify_date_time.day = bDay;
        dir_entry_info->modify_date_time.hour = bHour;
        dir_entry_info->modify_date_time.minute = bMinute;
        dir_entry_info->modify_date_time.second = bSecond;

        FileGetDate_for_FATxx(fdb_ptr->AccessDate, &wYear, &bMonth, &bDay);
        dir_entry_info->access_date_time.year = wYear;
        dir_entry_info->access_date_time.month = bMonth;
        dir_entry_info->access_date_time.day = bDay;
    }
#if EXFAT_ENABLE
    else
    {
        TYPE_exFAT_Generic_DirEntry_Template *exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) fdb_ptr;
        WORD fileAttributes;

        /* check if current FDB node is a file-related exFAT DirEntry */
        if ((exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE) && (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT) &&
            (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE_NAME))
        {
            MP_ALERT("%s: Error! Current FDB node is not a file-related exFAT DirEntry ! its entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
            return FAIL;
        }

        if (exfat_node->entryType == EXFAT_ENTRY_TYPE_FILE_NAME)
        {
            MP_ALERT("%s: Error! Current node is a [File Name DirEntry]. We expect it to be a [File DirEntry] / [Stream Extension DirEntry] !", __FUNCTION__);
            return FAIL;
        }
        else if (exfat_node->entryType == EXFAT_ENTRY_TYPE_STREAM_EXT)
        {
            if (((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->generalSecondaryFlags & EXFAT_DirEntryFlags_MASK_AllocPossible)
                dir_entry_info->exfat_DirEntryFlags.f_AllocPossible = 1;
            else
                dir_entry_info->exfat_DirEntryFlags.f_AllocPossible = 0;

            if (((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->generalSecondaryFlags & EXFAT_DirEntryFlags_MASK_NoFatChain)
                dir_entry_info->exfat_DirEntryFlags.f_NoFatChain = 1;
            else
                dir_entry_info->exfat_DirEntryFlags.f_NoFatChain = 0;

            /* start cluster is contained in "Stream Extension DirEntry" */
            dir_entry_info->startCluster = (DWORD) LoadAlien32(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->firstCluster);

            /* file size is contained in "Stream Extension DirEntry" */
            U64  dataLength;
            BYTE *Hi32_of_64, *Lo32_of_64;
            Lo32_of_64 = (BYTE *) &((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->dataLength; /* due to little-endian */
            Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */

            dataLength = (U64) ((U64) LoadAlien32(Hi32_of_64) << 32) + (U64) LoadAlien32(Lo32_of_64);
            if ((U64) dataLength <= (U64) 0x00000000FFFFFFFF)
                (DWORD) dir_entry_info->fileSize = (DWORD) dataLength;
            else
            {
                mpDebugPrint("%s: To-Do: 64-bit file size processing !?  Temp use 32-bit file size ...", __FUNCTION__);

                dir_entry_info->fileSize = (DWORD) dataLength; /* temp use 32-bit file size */
            }

            /* move to corresponding "File DirEntry" position of current "Stream Extension DirEntry" */
            if (PreviousNode(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! Failed to move to [File DirEntry] of this [Stream Extension DirEntry] !", __FUNCTION__);
                return FAIL;
            }

            exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) drv->Node;
            if (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE)
            {
                MP_ALERT("%s: Error! Current FDB node should be a [File DirEntry] ! its entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
                return FAIL;
            }
        }
        else
        {
            dir_entry_info->fileSize = 0; /* here, initialize file size to 0 to wait further processing */
        }

        /* now, current FDB node should be an exFAT "File DirEntry" */
        if (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE)
        {
            MP_ALERT("%s: Error! Current node is not an exFAT [File DirEntry] ! Pls check its file system ...", __FUNCTION__);
            return FAIL;
        }

        /* file attribute flags */
        fileAttributes = (WORD) LoadAlien16(&((TYPE_exFAT_File_DirEntry *) exfat_node)->fileAttributes);
        if (fileAttributes & EXFAT_FDB_ATTRIB_RO)
            dir_entry_info->fileAttribute.f_ReadOnly = 1;
        if (fileAttributes & EXFAT_FDB_ATTRIB_HIDDEN)
            dir_entry_info->fileAttribute.f_Hidden = 1;
        if (fileAttributes & EXFAT_FDB_ATTRIB_SYSTEM)
            dir_entry_info->fileAttribute.f_System = 1;
        if (fileAttributes & EXFAT_FDB_ATTRIB_ARCH)
            dir_entry_info->fileAttribute.f_Archive = 1;
        if (fileAttributes & EXFAT_FDB_ATTRIB_DIR)
            dir_entry_info->fileAttribute.f_SubDir = 1;

        /* file date,time info */
        FileGetDateTime_for_exFAT(((TYPE_exFAT_File_DirEntry *) exfat_node)->createTimestamp, &dir_entry_info->create_date_time);
        FileGetDateTime_for_exFAT(((TYPE_exFAT_File_DirEntry *) exfat_node)->lastModifyTimestamp, &dir_entry_info->modify_date_time);
        FileGetDateTime_for_exFAT(((TYPE_exFAT_File_DirEntry *) exfat_node)->lastAccessTimestamp, &dir_entry_info->access_date_time);

        /* file size */
        if (dir_entry_info->fileSize == 0) /* need to move to "Stream Extension DirEntry" to get its file size */
        {
            /* file size is contained in "Stream Extension DirEntry" */
            /* move to corresponding "Stream Extension DirEntry" position of current "File DirEntry" */
            if (NextNode(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! Failed to move to [Stream Extension DirEntry] of this [File DirEntry] !", __FUNCTION__);
                return FAIL;
            }

            exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) drv->Node;
            if (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT)
            {
                MP_ALERT("%s: Error! Current FDB node should be a [Stream Extension DirEntry] ! its entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
                return FAIL;
            }
            else
            {
                if (((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->generalSecondaryFlags & EXFAT_DirEntryFlags_MASK_AllocPossible)
                    dir_entry_info->exfat_DirEntryFlags.f_AllocPossible = 1;
                else
                    dir_entry_info->exfat_DirEntryFlags.f_AllocPossible = 0;

                if (((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->generalSecondaryFlags & EXFAT_DirEntryFlags_MASK_NoFatChain)
                    dir_entry_info->exfat_DirEntryFlags.f_NoFatChain = 1;
                else
                    dir_entry_info->exfat_DirEntryFlags.f_NoFatChain = 0;

                /* start cluster is contained in "Stream Extension DirEntry" */
                dir_entry_info->startCluster = (DWORD) LoadAlien32(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->firstCluster);

                U64  dataLength;
                BYTE *Hi32_of_64, *Lo32_of_64;
                Lo32_of_64 = (BYTE *) &((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->dataLength; /* due to little-endian */
                Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */

                dataLength = (U64) ((U64) LoadAlien32(Hi32_of_64) << 32) + (U64) LoadAlien32(Lo32_of_64);
                if ((U64) dataLength <= (U64) 0x00000000FFFFFFFF)
                    (DWORD) dir_entry_info->fileSize = (DWORD) dataLength;
                else
                {
                    mpDebugPrint("%s: To-Do: 64-bit file size processing !?  Temp use 32-bit file size ...", __FUNCTION__);

                    dir_entry_info->fileSize = (DWORD) dataLength; /* temp use 32-bit file size */
                }
            }
        }

        /* For FileOpen() or other functions relying on drv->Node to work, make sure we go back "Stream Extension DirEntry" position */
        if (exfat_node->entryType == EXFAT_ENTRY_TYPE_FILE)
        {
            /* move to corresponding "Stream Extension DirEntry" position of current "File DirEntry" */
            if (NextNode(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! Failed to move to [Stream Extension DirEntry] of this [File DirEntry] !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
        }
    }
#endif

    return PASS;
}



///
///@ingroup FILE
///@brief   Delete a file or delete a subdirectory (and all files in it) pointed to by current FDB node of the drive (drv->Node).
///
///@param   drv           The drive on which the file or directory resides. \n\n
///
///@retval  PASS          Delete the file or folder successfully. \n\n
///@retval  FAIL          Delete the file or folder unsuccessfully. \n\n
///
///@remark    The file or folder to be deleted is implicitly pointed to by current FDB node of the drive (drv->Node) before calling this function.
///
SWORD DeleteFileOrFolderOfCurrentFDB(DRIVE * drv)
{
    if (drv == NULL)
    {
        MP_ALERT("%s: Error ! NULL pointer !", __FUNCTION__);
        return FAIL;
    }

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        if (drv->Node->Attribute & FDB_SUB_DIR) /* current entry is a folder */
        {
            if (DeleteDir(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! DeleteFile() failed !", __FUNCTION__);
                return FAIL;
            }
        }
        else if (drv->Node->Attribute & FDB_ARCHIVE) /* current entry is a file */
        {
            STREAM *fHandle = FileOpen(drv);
            if (fHandle == NULL)
            {
                MP_ALERT("%s: Error! FileOpen() failed !", __FUNCTION__);
                return FAIL;
            }

            if (DeleteFile(fHandle) != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! DeleteFile() failed !", __FUNCTION__);
                return FAIL;
            }
        }
        else if (drv->Node->Attribute == FDB_LONG_NAME) /* current FDB node is a long name FDB node */
        {
            MP_ALERT("%s: Error! The FDB node is a long name FDB node. We expect it to be the short name FDB of a file or folder !", __FUNCTION__);
            return FAIL;
        }
    }
#if EXFAT_ENABLE
    else
    {
  #if EXFAT_WRITE_ENABLE
        MP_ALERT("%s: To-Do: how to process exFAT Write operations ??", __FUNCTION__);
        return FAIL; //not supported yet
  #else
        MP_ALERT("%s: -I- exFAT Write operations are not supported !", __FUNCTION__);
        return FAIL;
  #endif
    }
#endif

    return PASS;
}



#if Write_To_NAND
#define MAX_SERIAL 3
int File_Serial = 1;
int File_Serial_Index[MAX_SERIAL];

void Reset_File_Serial(void)
{
    File_Serial = 1;
    MpMemSet(File_Serial_Index, '\0', MAX_SERIAL);
}
#endif



///
///@ingroup FILE_BROWSER
///@brief   Open one file specified by a GUI file list entry on the specified drive.
///
///@param   sDrv           The drive to open the file. \n\n
///@param   pSearchInfo    The pointer to the GUI file list entry of the file.
///
///@return  The function call will return NULL if no file handle can use, else return the file handle.
///
STREAM *FileListOpen(DRIVE * sDrv, ST_SEARCH_INFO * pSearchInfo)
{
    STREAM *sHandle;
    FDB *sNode;
    BYTE *pbBuffer, *pbTempS, *pbTempT, i;
    CHAIN *dir, stackTop_dir;
    int ret;
    unsigned char retrycount = 0;
    DWORD dir_size;

    if (sDrv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return NULL;
    }

    if (pSearchInfo == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return NULL;
    }

    if (!(sHandle = GetFreeFileHandle(sDrv)))
    {
        MP_ALERT("%s: No file handle available !", __FUNCTION__);
        return NULL;
    }

#if USBOTG_HOST_PTP
    if ((sDrv->DevID == DEV_USB_HOST_PTP) || (sDrv->DevID == DEV_USBOTG1_HOST_PTP)) //(sDrv->DevID == DEV_USB_HOST_PTP)
    {
        sHandle->DirSector      = pSearchInfo->dwPtpObjectHandle; // use DirSector to store the value of object handler
        sHandle->Chain.Size     = pSearchInfo->dwFileSize;
        sHandle->Chain.Start    = 0;
        sHandle->Chain.Point    = 0; // for PTP, it's index
        sHandle->Flag.f_Utf16_LongName = (pSearchInfo->dwLongFdbCount > 0)? 1:0;
        return sHandle;
    }
#endif

#if (NETWARE_ENABLE || USBOTG_WEB_CAM)
    if ((sDrv->DevID == DEV_USB_WIFI_DEVICE) ||\
        (sDrv->DevID == DEV_CF_ETHERNET_DEVICE) ||\
        (sDrv->DevID == DEV_USB_ETHERNET_DEVICE) ||\
        (sDrv->DevID == DEV_USB_WEBCAM))
    {
        sHandle->DirSector = 0;
        sHandle->Chain.Start = 0;
        sHandle->Chain.Point = 0;
        sHandle->Chain.Size = pSearchInfo->dwFileSize;
        return sHandle;
    }
#endif

    sHandle->DirSector = pSearchInfo->dwDirPoint;
    //weiching 2009.09.17 fix bug
    sHandle->FdbOffsetInDirCache = (((pSearchInfo->dwFdbOffset << 5) & ((1 << sDrv->bSectorExp) - 1)) >> 5); //unit: number of FDB nodes

    sHandle->Flag.SizeChanged = 0;
    sHandle->Flag.f_Utf16_LongName = (pSearchInfo->dwLongFdbCount > 0)? 1:0;

    if (FileDirCaching(sHandle) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E- FileDirCaching() failed !", __FUNCTION__);
        return NULL;
    }

    dir = (CHAIN *) ((BYTE *) sDrv->DirStackBuffer + sDrv->DirStackPoint);

    MpMemCopy((BYTE *) &stackTop_dir, (BYTE *) dir, sizeof(CHAIN));

#if EXFAT_ENABLE
    dir_size = pSearchInfo->dwParentDirSize;
#else
    if (!sDrv->DirStackPoint) /* in RootDir */
        dir_size = GetRootDirSize(sDrv);
    else
        dir_size = dir->Size;
#endif
    MP_DEBUG("%s: parent dir's chain size = %lu", __FUNCTION__, dir_size);

    ChainInit(dir, pSearchInfo->dwDirStart, dir_size);
    ret = ChainSeekForward(sDrv, dir, pSearchInfo->dwFdbOffset << 5);
    if (ret != FS_SUCCEED)
    {
        /* chain seek error case! show important debug info for debugging */
#if EXFAT_ENABLE //extra field for exFAT
        MP_ALERT("### %s: pSearchInfo->dwFdbOffset = %lu, pSearchInfo->dwParentDirSize = %lu", __FUNCTION__, pSearchInfo->dwFdbOffset, pSearchInfo->dwParentDirSize);
#else
        MP_ALERT("### %s: pSearchInfo->dwFdbOffset = %lu", __FUNCTION__, pSearchInfo->dwFdbOffset);
#endif
        MP_ALERT("### %s: ori DirStack top dir (start cluster = %lu, chain size = %lu);", __FUNCTION__, stackTop_dir.Start, stackTop_dir.Size);
        MP_ALERT("### %s: new DirStack top dir (start cluster = %lu, chain size = %lu);", __FUNCTION__, dir->Start, dir->Size);
    }

    if (sDrv->Flag.FsType != FS_TYPE_exFAT)
    {
        ChainInit((CHAIN *) (&sHandle->Chain),
                  ((LoadAlien16((void *) &sDrv->Node->StartHigh) << 16) + LoadAlien16((void *) &sDrv->Node->StartLow)),
                  LoadAlien32(&sDrv->Node->Size) );
    }
#if EXFAT_ENABLE
    else
    {
        TYPE_exFAT_Stream_Extension_DirEntry *exfat_node = (TYPE_exFAT_Stream_Extension_DirEntry *) sDrv->Node;
        BYTE *Hi32_of_64, *Lo32_of_64;
        DWORD dwFileSize = 0;
        U64   u64FileSize;

        if (((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->generalSecondaryFlags & EXFAT_DirEntryFlags_MASK_NoFatChain)
            sHandle->Chain.exfat_DirEntryFlags.f_NoFatChain = 1;
        else
            sHandle->Chain.exfat_DirEntryFlags.f_NoFatChain = 0;

        Lo32_of_64 = (BYTE *) &exfat_node->dataLength; /* due to little-endian */
        Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
        u64FileSize = (U64) ((U64) LoadAlien32(Hi32_of_64) << 32) + (U64) LoadAlien32(Lo32_of_64);
        if ((U64) u64FileSize <= (U64) 0x00000000FFFFFFFF)
        {
            dwFileSize = (DWORD) u64FileSize;
            ChainInit((CHAIN *) (&sHandle->Chain),
                      LoadAlien32(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->firstCluster), dwFileSize);
        }
        else
        {
mpDebugPrint("%s: To-Do: 64-bit size parameter may cause problem during push/pop to/from the stack !?", __FUNCTION__);
mpDebugPrint("%s: To-Do: Modify ChainInit() to accept 64-bit size parameter ...", __FUNCTION__);
            ChainInit((CHAIN *) (&sHandle->Chain),
                      LoadAlien32(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->firstCluster), (U64) u64FileSize);
        }
    }
#endif

    sDrv->CacheBufPoint->LongNameFdbCount = pSearchInfo->dwLongFdbCount;

    return sHandle;
}



///
///@ingroup FILE
///@brief   Locate one DirEntry's position within its parent directory on the specified drive. The DirEntry to locate is specified by a GUI file list entry.
///
///@param   drv            The drive to locate the DirEntry. \n\n
///@param   pSearchInfo    The pointer to a file list entry to locate its DirEntry position within its parent directory.
///
///@retval  FS_SUCCEED         Locate the DirEntry successfully. \n\n
///@retval  OUT_OF_RANGE       Locate the DirEntry unsuccessfully due to out of range of chain size. \n\n
///@retval  ABNORMAL_STATUS    Locate the DirEntry unsuccessfully due to some error. \n\n
///
///@remark    Note there is always a "Upper Folder" entry in the file list when entering a sub-folder via GUI file list,
///           and such pseudo "Upper Folder" entry has no actual position information about the DirEntry's parent directory.
///           Hence, we cannot locate the "Upper Folder" pSearchInfo entry, and we return FILE_NOT_FOUND for such special case.
///
SWORD Locate_DirEntryNode_of_SearchInfo(DRIVE * drv, ST_SEARCH_INFO * pSearchInfo)
{
    CHAIN *dir, stackTop_dir;
    SWORD ret;
    DWORD dir_size;

    if ((drv == NULL) || (pSearchInfo == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if ((pSearchInfo->bParameter == SEARCH_INFO_CHANGE_PATH) && ((pSearchInfo->bName[0] == '.') && (pSearchInfo->bName[1] == '.')))
    {
        MP_ALERT("%s: this ST_SEARCH_INFO entry is an [Upper Folder] pseudo entry.", __FUNCTION__);
        return FILE_NOT_FOUND; /* here, use FILE_NOT_FOUND for [Upper Folder] case */
    }

    if (pSearchInfo->dwDirPoint != drv->DirCachePoint)
    {
        // if cache has been modified then write it back
        if (drv->Flag.DirCacheChanged)
        {
            if (DriveWrite(drv, (BYTE *) drv->DirCacheBuffer, drv->DirCachePoint, 1) != FS_SUCCEED)
            {
                MP_ALERT("%s: DriveWrite() for writing DirCache content back failed !", __FUNCTION__);
                return DRIVE_ACCESS_FAIL;
            }
        }

        if (DriveRead(drv, (BYTE *) drv->DirCacheBuffer, pSearchInfo->dwDirPoint, 1) != FS_SUCCEED)
        {
            MP_ALERT("%s: DriveRead() for DirCache failed !", __FUNCTION__);
            return DRIVE_ACCESS_FAIL;
        }
        drv->Flag.DirCacheChanged = 0;
        drv->DirCachePoint = pSearchInfo->dwDirPoint; // update dir cache lba
    }

    DWORD FdbOffsetInDirCache = (((pSearchInfo->dwFdbOffset << 5) & ((1 << drv->bSectorExp) - 1)) >> 5); //unit: number of FDB nodes
    drv->Node = (FDB *) ((BYTE *) drv->DirCacheBuffer + (FdbOffsetInDirCache << 5));

    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);

    MpMemCopy((BYTE *) &stackTop_dir, (BYTE *) dir, sizeof(CHAIN));

#if EXFAT_ENABLE
    dir_size = pSearchInfo->dwParentDirSize;
#else
    if (!drv->DirStackPoint) /* in RootDir */
        dir_size = GetRootDirSize(drv);
    else
        dir_size = dir->Size;
#endif
    MP_DEBUG("%s: parent dir's chain size = %lu", __FUNCTION__, dir_size);

    ChainInit(dir, pSearchInfo->dwDirStart, dir_size);
    ret = ChainSeekForward(drv, dir, pSearchInfo->dwFdbOffset << 5);
    if (ret != FS_SUCCEED)
    {
        /* chain seek error case! show important debug info for debugging */
#if EXFAT_ENABLE //extra field for exFAT
        MP_ALERT("### %s: pSearchInfo->dwFdbOffset = %lu, pSearchInfo->dwParentDirSize = %lu", __FUNCTION__, pSearchInfo->dwFdbOffset, pSearchInfo->dwParentDirSize);
#else
        MP_ALERT("### %s: pSearchInfo->dwFdbOffset = %lu", __FUNCTION__, pSearchInfo->dwFdbOffset);
#endif
        MP_ALERT("### %s: ori DirStack top dir (start cluster = %lu, chain size = %lu);", __FUNCTION__, stackTop_dir.Start, stackTop_dir.Size);
        MP_ALERT("### %s: new DirStack top dir (start cluster = %lu, chain size = %lu);", __FUNCTION__, dir->Start, dir->Size);
    }

    drv->CacheBufPoint->LongNameFdbCount = pSearchInfo->dwLongFdbCount;

    return ret;
}



static WORD wPrixFixLName[5] = { 0x0043, 0x004f, 0x0050, 0x0059, 0x0028 };	// " COPY( "


/* return the count of characters filled into the buffer */
static DWORD FillNumber(BYTE * pwBuffer, DWORD dwNumber)
{
    DWORD dwtemp, dwCount, i;


    if (pwBuffer == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return 0;
    }

    dwCount = 1;
    dwtemp = dwNumber;

    while (dwtemp >= 10)
    {
        dwtemp = dwtemp / 10;
        dwCount++;
    }

    dwtemp = dwNumber;

    for (i = dwCount; i > 0; i--)
    {
        pwBuffer[i - 1] = ((dwtemp % 10) + 0x30);
        dwtemp = dwtemp / 10;
    }
    return dwCount;
}



// return the number of words filled into the buffer
DWORD FillUniNumber(WORD * pwBuffer, DWORD dwNumber)
{
    DWORD dwtemp, dwCount, i;


    if (pwBuffer == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return 0;
    }

    dwCount = 1;
    dwtemp = dwNumber;

    while (dwtemp >= 10)
    {
        dwtemp = dwtemp / 10;
        dwCount++;
    }

    dwtemp = dwNumber;

    for (i = dwCount; i > 0; i--)
    {
        pwBuffer[i - 1] = ((dwtemp % 10) + 0x0030);
        dwtemp = dwtemp / 10;
    }

    return dwCount;
}



/* This function can be used for both FAT12/16/32 and exFAT drives */
/* note: parameter 'pwLongName' is UTF-16 filename buffer, it should be at least 512 bytes long (256 UTF-16 characters) */
SWORD BuildNewLName(DRIVE * psTargetDrive, DRIVE * psSourceDrive, WORD * pwLongName, DWORD * pdwLNameChged)
{
    DWORD i, dwNumericTail, dwNumericTailLength;
    WORD wTmpName[MAX_L_NAME_LENG], name_len;
    int ret;


    if ((psTargetDrive == NULL) || (psSourceDrive == NULL))
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if ((pwLongName == NULL) || (pdwLNameChged == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    *pdwLNameChged = 0;

    MpMemCopy((BYTE *) pwLongName, (BYTE *) psSourceDrive->LongName, MAX_L_NAME_LENG * 2);
    name_len = StringLength16(pwLongName);

    ret = FileSearchLN(psTargetDrive, pwLongName, name_len, E_BOTH_FILE_AND_DIR_TYPE);
    if (ret == END_OF_DIR) // no same file name found
        return FS_SUCCEED;

    // same file name found, so add COPY(XXX)_ to file name
    // xxx is between 1 and 999999
    *pdwLNameChged = 1;

    for (dwNumericTail = 1; dwNumericTail < 999999; dwNumericTail++)
    {
        for (i = 0; i < 5; i++)
            wTmpName[i] = wPrixFixLName[i];

        dwNumericTailLength = 5;
        dwNumericTailLength += FillUniNumber(&wTmpName[5], dwNumericTail);

        wTmpName[dwNumericTailLength] = 0x0029;	// " ) "
        wTmpName[dwNumericTailLength + 1] = 0x005F;	//"_"
        dwNumericTailLength += 2;

        for (i = 0; i < name_len; i++)
        {
            wTmpName[dwNumericTailLength] = pwLongName[i];
            dwNumericTailLength++;
            if (dwNumericTailLength >= (MAX_L_NAME_LENG - 1))
                break;
        }

        wTmpName[dwNumericTailLength] = 0x0000;

        ret = FileSearchLN(psTargetDrive, (WORD *) (&wTmpName[0]), dwNumericTailLength, E_BOTH_FILE_AND_DIR_TYPE);
        if (ret == END_OF_DIR)	// no same file name found
        {
            for (i = 0; i < dwNumericTailLength; i++)
                pwLongName[i] = wTmpName[i];

            return FS_SUCCEED;
        }
    }

    return FS_SUCCEED;
}



/* note: this function is specific to FAT12/16/32 short name FDB (i.e. DirEntry) processing only. It cannot be used on exFAT drives ! */
SWORD BuildNewFDB(DRIVE * psDrive, FDB * psSrcFdb)
{
    DWORD dwMaxIndex, i, dwNameLength, dwNumericTailLength, dwNumericTail;
    BYTE bTempBuf[8], *pbTemp;
    int ret;


    if (psDrive == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

#if EXFAT_ENABLE
    if (psDrive->Flag.FsType == FS_TYPE_exFAT)
    {
        MP_ALERT("%s: -E- This function cannot be used on exFAT drives !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
#endif

    if (psSrcFdb == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    BYTE pri_name[9], ext_name[4];
    MpMemSet(pri_name, 0, 9);
    MpMemSet(ext_name, 0, 4);
    for (i = 0; i < 8; i++)
    {
        if (psSrcFdb->Name[i] != ' ')
            pri_name[i] = psSrcFdb->Name[i];
        else
            break;
    }
    for (i = 0; i < 3; i++)
    {
        if (psSrcFdb->Extension[i] != ' ')
            ext_name[i] = psSrcFdb->Extension[i];
        else
            break;
    }
    ret = FileSearch(psDrive, pri_name, ext_name, E_BOTH_FILE_AND_DIR_TYPE);
    if (ret == END_OF_DIR)
        return FS_SUCCEED;  // no same file name found

    pbTemp = (BYTE *) (&psSrcFdb->Name[0]);

    dwNameLength = 8;
    dwNumericTailLength = 0;
    for (i = 0; i < 8; i++)
    {
        if (dwNumericTailLength)
        {
            if ((pbTemp[i] < 0x30) || (pbTemp[i] > 0x39))  // "~xxx" and xxx is not all number, then access them as normal file name
            {
                dwNameLength = 8;
                dwNumericTailLength = 0;
            }
        }

        if (pbTemp[i] == 0x20)	// " "
        {
            dwNumericTailLength = 0;
            dwNumericTail = 1;
            dwNameLength = i;
            break;
        }
        else if (pbTemp[i] == 0x7e)	// "~"
        {
            dwNameLength = i;
            dwNumericTailLength = 7 - i;
        }
    }

    if (dwNumericTailLength)
    {
        dwNumericTail = 0;
        for (i = dwNumericTailLength; i > 0; i--)
        {
            dwNumericTail = dwNumericTail * 10;
            dwNumericTail += (pbTemp[8 - i] & 0x0f);
        }
    }
    else
        dwNumericTail = 1;

    while (dwNumericTailLength <= 6)  //numeric-tail range "~1" to "~999999"
    {
        dwNumericTailLength = mp_sprintf(bTempBuf, "%d", dwNumericTail);	// tail length
        if ((8 - dwNameLength) > dwNumericTailLength)
        {
            pbTemp[dwNameLength] = '~';
            StringNCopy08(&pbTemp[dwNameLength + 1], bTempBuf, dwNumericTailLength);
        }
        else
        {
            pbTemp[(8 - (dwNumericTailLength + 1))] = '~';
            StringNCopy08(&pbTemp[(8 - dwNumericTailLength)], bTempBuf, dwNumericTailLength);
        }

        MpMemSet(pri_name, 0, 9);
        for (i = 0; i < 8; i++)
        {
            if (psSrcFdb->Name[i] != ' ')
                pri_name[i] = psSrcFdb->Name[i];
            else
                break;
        }
        ret = FileSearch(psDrive, pri_name, ext_name, E_BOTH_FILE_AND_DIR_TYPE);
        if (ret == END_OF_DIR)
            return FS_SUCCEED;	// no same file name found

        dwNumericTail++;
    }

    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Copy the specified file content to the target drive.
///
///@param   stTargetDrive     DRIVE handle of the target drive. \n\n
///@param   stSourceHandle    File handle of the source file to copy. \n\n
///@param   dwBufAddress      The pointer/address of a prepared temporary buffer to be used during the copy process. \n\n
///@param   dwTempBufSize     Size of the prepared temporary buffer.
///
///@retval  FS_SUCCEED                  Copy file successfully. \n\n
///@retval  ABNORMAL_STATUS             Copy file unsuccessfully. \n\n
///@retval  DISK_FULL                   Copy file unsuccessfully due to no enough free space on the target drive. \n\n
///@retval  INVALID_DRIVE               Invalid drive.
///
SWORD FileCopy(DRIVE * stTargetDrive, STREAM * stSourceHandle, DWORD dwBufAddress, DWORD dwTempBufSize)
{
    CHAIN stTargetChain;
    FDB   sTempFnode;
    FDB   *FCnode;
    DWORD dwCluster;
    DWORD i, j;
    BYTE  *pbTmp;
    SWORD swRet;
    DWORD bufByteCount = 0;
    ENTRY_FILENAME_TYPE  bFilenameType = E_FILENAME_UNKNOWN;
    WORD  *wTmpFCLName;
    BOOL  f_Utf16_LongName;
    BOOL  f_copy_broken = FALSE;
    BOOL  f_broken_need_to_ChainFree = FALSE;
    BYTE  longname_FDB_count = 0;
    WORD  wTmp_LongName_Buf[MAX_L_NAME_LENG];
    DWORD dwLNameChged;
    ST_SAVE_FDB_POS  fdb_pos_info;


    MP_DEBUG("Enter %s()...", __FUNCTION__);

    if (stTargetDrive == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (stSourceHandle == NULL)
    {
        MP_ALERT("%s: Error! NULL file handle !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (stSourceHandle->Flag.f_IsRamFile)
    {
        MP_ALERT("%s: this operation is not supported for \"RamFile\" type files !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
#endif

    if (! SystemCardPresentCheck(stTargetDrive->DrvIndex))
    {
        MP_ALERT("%s: Target drive card not present !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    /* note: special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (! stSourceHandle->Flag.f_IsRamFile)
    {
        if (! SystemCardPresentCheck(stSourceHandle->Drv->DrvIndex))
        {
            MP_ALERT("%s: Source drive card not present !", __FUNCTION__);
            return INVALID_DRIVE;
        }
    }

    //check free space
    if (CheckSpaceIfEnough(stTargetDrive, FileSizeGet(stSourceHandle)) == DISK_FULL)
    {
        MP_ALERT("%s: free disk space not enough !!", __FUNCTION__);
        return DISK_FULL;
    }

    //get file node
    FCnode = &sTempFnode;
    MpMemSet((BYTE *) FCnode, 0, sizeof(FDB));

#if (NETWARE_ENABLE && MAKE_XPG_PLAYER)
    ST_NET_FILEENTRY *NetFileEntry;

    if ((stSourceHandle->Drv->DevID == DEV_USB_WIFI_DEVICE) ||\
        (stSourceHandle->Drv->DevID == DEV_CF_ETHERNET_DEVICE) ||\
        (stSourceHandle->Drv->DevID == DEV_USB_ETHERNET_DEVICE))
    {
        f_Utf16_LongName = 0;
        NetFileEntry = (ST_NET_FILEENTRY *) NetGetFileEntry(NetGetFileIndex());

        if (stTargetDrive->Flag.FsType != FS_TYPE_exFAT)
        {
            for (i = 0; i < 11; i++) // fill all area with space
                FCnode->Name[i] = 0x20;

            pbTmp = (BYTE *)(&NetFileEntry->Name[0]);
            for (i = 0; (i < 8) &&  (*pbTmp != 0); i++)	// write file name
            {
                FCnode->Name[i] = *pbTmp;
                pbTmp++;
            }

            pbTmp = (BYTE *)(&NetFileEntry->ExtName[0]);
            for (i = 0; (i < 3) && (*pbTmp != 0); i++) // write extension
            {
                FCnode->Extension[i] = *pbTmp;
                pbTmp++;
            }

            FCnode->Attribute = FDB_ARCHIVE; // write attribute

            SaveAlien32(&FCnode->Size, NetGetFileSize(NetGetFileIndex()));
        }
  #if EXFAT_ENABLE
        else
        {
            MP_DEBUG("%s: File Copy source is from Wi-Fi, target drive is an exFAT drive.", __FUNCTION__);
    #if EXFAT_WRITE_ENABLE
            MP_ALERT("%s: To-Do: how to process exFAT Write operations ??", __FUNCTION__);
            return ABNORMAL_STATUS; //not supported yet
    #else
            MP_ALERT("%s: -I- exFAT Write operations are not supported !", __FUNCTION__);
            return ABNORMAL_STATUS;
    #endif
        }
  #endif
    }
    else
#endif
    {
        if (stSourceHandle->Drv->Flag.FsType != FS_TYPE_exFAT)
        {
#if USBOTG_HOST_PTP
            if ((stSourceHandle->Drv->DevID == DEV_USB_HOST_PTP) || (stSourceHandle->Drv->DevID == DEV_USBOTG1_HOST_PTP))
            {
            	static FDB sFDB;
                ST_SEARCH_INFO * pSearchInfo;

                pSearchInfo = (ST_SEARCH_INFO *) stSourceHandle->Drv->Node;  /* USB PTP special */
				stSourceHandle->Drv->Node = &sFDB;
                MpMemSet((BYTE *)stSourceHandle->Drv->Node, 0, sizeof(FDB));
                stSourceHandle->Drv->Node->Attribute    = 0x20;
                stSourceHandle->Drv->Node->Size         = byte_swap_of_dword(pSearchInfo->dwFileSize);
                stSourceHandle->Drv->Node->CreateTime   = FileSetTime_for_FATxx(pSearchInfo->DateTime.hour, pSearchInfo->DateTime.minute, pSearchInfo->DateTime.second);
                stSourceHandle->Drv->Node->CreateDate   = FileSetDate_for_FATxx(pSearchInfo->DateTime.year, pSearchInfo->DateTime.month, pSearchInfo->DateTime.day);
                stSourceHandle->Drv->Node->ModifyTime   = FileSetTime_for_FATxx(pSearchInfo->DateTime.hour, pSearchInfo->DateTime.minute, pSearchInfo->DateTime.second);
                stSourceHandle->Drv->Node->ModifyDate   = FileSetDate_for_FATxx(pSearchInfo->DateTime.year, pSearchInfo->DateTime.month, pSearchInfo->DateTime.day);
                stSourceHandle->Drv->CacheBufPoint->LongNameFdbCount = pSearchInfo->dwLongFdbCount;
				MpMemCopy((void *)&stSourceHandle->Drv->Node->Name[0], &pSearchInfo->bName[0], 8);
                MpMemCopy((void *)&stSourceHandle->Drv->Node->Extension[0], &pSearchInfo->bExt[0], 3);
                f_Utf16_LongName = 0;
            }
            else
#endif
            f_Utf16_LongName = stSourceHandle->Flag.f_Utf16_LongName;

            MpMemCopy((BYTE *) FCnode, (BYTE *) stSourceHandle->Drv->Node, sizeof(FDB));

            wTmpFCLName = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
            if (wTmpFCLName == NULL)
            {
                MP_ALERT("%s: -E malloc fail", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
            MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);

#if USBOTG_HOST_PTP
            if ((stSourceHandle->Drv->DevID == DEV_USB_HOST_PTP) || (stSourceHandle->Drv->DevID == DEV_USBOTG1_HOST_PTP))
                bFilenameType = E_FILENAME_8_3_SHORT_NAME;
            else
#endif
            if (GetFilenameOfCurrentNode((DRIVE *) stSourceHandle->Drv, &bFilenameType, (BYTE *) wTmpFCLName, &bufByteCount, 512) != FS_SUCCEED)
            {
                MP_ALERT("%s: GetFilenameOfCurrentNode() failed !", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return ABNORMAL_STATUS;
            }

            if (bFilenameType == E_FILENAME_8_3_SHORT_NAME)
            {
                f_Utf16_LongName = 0;

    #if HELP_DEBUGGING_FILE_COPY_ISSUES
                MP_ALERT("\r\n### %s: file to copy: (8.3 Short) filename = %s", __FUNCTION__, wTmpFCLName);
    #endif
            }
            else if (bFilenameType == E_FILENAME_UTF16_LONG_NAME)
            {
                f_Utf16_LongName = 1;
                MpMemSet((BYTE *) wTmp_LongName_Buf, 0, MAX_L_NAME_LENG * 2);
                MpMemCopy((BYTE *) wTmp_LongName_Buf, (BYTE *) wTmpFCLName, bufByteCount);

    #if HELP_DEBUGGING_FILE_COPY_ISSUES
                MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);
                mpx_UtilU16ToU08(wTmpFCLName, (WORD *) wTmp_LongName_Buf); /* convert UTF-16 to UTF-8 string */
                MP_ALERT("\r\n### %s: file to copy: (Long) filename = %s", __FUNCTION__, wTmpFCLName);
    #endif
            }

            if (! f_Utf16_LongName) /* 8.3 short filename */
            {
                BYTE pri_name[9], ext_name[4];
                MpMemSet(pri_name, 0, 9);
                MpMemSet(ext_name, 0, 4);
                for (i = 0; i < 8; i++)
                {
                    if (stSourceHandle->Drv->Node->Name[i] != ' ')
                        pri_name[i] = stSourceHandle->Drv->Node->Name[i];
                    else
                        break;
                }
                for (i = 0; i < 3; i++)
                {
                    if (stSourceHandle->Drv->Node->Extension[i] != ' ')
                        ext_name[i] = stSourceHandle->Drv->Node->Extension[i];
                    else
                        break;
                }
                if (FileSearch(stTargetDrive, pri_name, ext_name, E_BOTH_FILE_AND_DIR_TYPE) == FS_SUCCEED) /* same file name found */
                {
                    BYTE ascii_full_name[13];
                    MpMemSet(ascii_full_name, 0, 13);
                    strcpy(ascii_full_name, pri_name);
                    strcat(ascii_full_name, ".");
                    strcat(ascii_full_name, ext_name);

                    MpMemSet((BYTE *) stSourceHandle->Drv->LongName, 0, MAX_L_NAME_LENG * 2);
                    mpx_UtilUtf8ToUnicodeU16((WORD *) stSourceHandle->Drv->LongName, ascii_full_name); /* convert to UTF-16 string */
                    f_Utf16_LongName = 1; /* after adding "COPY(XX)_" prefix, it will always become a long filename */
                    MpMemSet((BYTE *) wTmp_LongName_Buf, 0, MAX_L_NAME_LENG * 2);
                    MpMemCopy((BYTE *) wTmp_LongName_Buf, (BYTE *) stSourceHandle->Drv->LongName, StringLength08(ascii_full_name) * 2);
                }
            }

            dwLNameChged = 0;
            if (f_Utf16_LongName)
            {
                MpMemSet((BYTE *) wTmp_LongName_Buf, 0, MAX_L_NAME_LENG * 2);
                MpMemCopy((BYTE *) wTmp_LongName_Buf, (BYTE *) wTmpFCLName, bufByteCount);

                MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);
                if (BuildNewLName(stTargetDrive, (DRIVE *) (stSourceHandle->Drv), wTmpFCLName, &dwLNameChged) != FS_SUCCEED)
                {
                    MP_ALERT("%s: BuildNewLName() failed !", __FUNCTION__);
                    ext_mem_free(wTmpFCLName);
                    return ABNORMAL_STATUS;
                }

                if (StringLength16(wTmpFCLName) % 13)
                    stSourceHandle->Drv->CacheBufPoint->LongNameFdbCount = (StringLength16(wTmpFCLName)/13 + 1);
                else
                    stSourceHandle->Drv->CacheBufPoint->LongNameFdbCount = StringLength16(wTmpFCLName)/13;

                longname_FDB_count = stSourceHandle->Drv->CacheBufPoint->LongNameFdbCount;

            #if 0 //just for debug purpose
                BYTE tmpBuf[512];
                {
                    MpMemSet(tmpBuf, 0, 512);
                    mpx_UtilU16ToU08(tmpBuf, (WORD *) wTmpFCLName); /* convert UTF-16 to UTF-8 string */
                    MP_DEBUG("%s: final target filename = %s", __FUNCTION__, tmpBuf);
                }
            #endif
            }

            // if long name changed, get the new short name
            // because we only add "COPY(XX)_" to long name, and they are all ASCII code
            // so, only get the low byte data
            if (dwLNameChged)
            {
                for (i = 0; i < 6; i++)
                    FCnode->Name[i] = (BYTE) wTmpFCLName[i];

                FCnode->Name[6] = 0x7e;	// " ~ "
                FCnode->Name[7] = 0x31;	// " 1 "
            }
        }
  #if EXFAT_ENABLE
        else
        {
            DWORD bufByteCount = 0;
            ENTRY_FILENAME_TYPE  bFilenameType = E_FILENAME_UNKNOWN;
            SWORD ret;

            wTmpFCLName = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
            if (wTmpFCLName == NULL)
            {
                MP_ALERT("%s: -E malloc fail", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
            MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);

            if (GetFilenameOfCurrentNode((DRIVE *) stSourceHandle->Drv, &bFilenameType, (BYTE *) wTmpFCLName, &bufByteCount, 512) != FS_SUCCEED)
            {
                MP_ALERT("%s: GetFilenameOfCurrentNode() failed !", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return ABNORMAL_STATUS;
            }

            /* actually, exFAT always use UTF-16 filename. here, just for code consistency */
            if (bFilenameType == E_FILENAME_8_3_SHORT_NAME)
                f_Utf16_LongName = 0;
            else if (bFilenameType == E_FILENAME_UTF16_LONG_NAME)
            {
                f_Utf16_LongName = 1;
                MpMemSet((BYTE *) wTmp_LongName_Buf, 0, MAX_L_NAME_LENG * 2);
                MpMemCopy((BYTE *) wTmp_LongName_Buf, (BYTE *) wTmpFCLName, bufByteCount);
            }

            if (f_Utf16_LongName)
            {
                MpMemSet((BYTE *) wTmp_LongName_Buf, 0, MAX_L_NAME_LENG * 2);
                MpMemCopy((BYTE *) wTmp_LongName_Buf, (BYTE *) wTmpFCLName, bufByteCount);

                dwLNameChged = 0;
                MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);
                if (BuildNewLName(stTargetDrive, (DRIVE *) (stSourceHandle->Drv), wTmpFCLName, &dwLNameChged) != FS_SUCCEED)
                {
                    MP_ALERT("%s: BuildNewLName() failed !", __FUNCTION__);
                    ext_mem_free(wTmpFCLName);
                    return ABNORMAL_STATUS;
                }

            #if 0 //just for debug purpose
                BYTE tmpBuf[512];
                {
                    MpMemSet(tmpBuf, 0, 512);
                    mpx_UtilU16ToU08(tmpBuf, (WORD *) wTmpFCLName); /* convert UTF-16 to UTF-8 string */
                    MP_DEBUG("%s: final target filename = %s", __FUNCTION__, tmpBuf);
                }
            #endif
            }
        }
  #endif
    }

    if (stTargetDrive->Flag.FsType != FS_TYPE_exFAT)
    {
        if (stSourceHandle->Drv->Flag.FsType != FS_TYPE_exFAT)
        {
            if (FCnode->Name[0] == 0xE5)
                FCnode->Name[0] = 0x05;

            if (BuildNewFDB(stTargetDrive, FCnode) != FS_SUCCEED)
            {
                MP_ALERT("%s: BuildNewFDB() failed !", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return ABNORMAL_STATUS;
            }

            CheckDeletedCount(stSourceHandle->Drv->CacheBufPoint->LongNameFdbCount + 1);

            if (f_Utf16_LongName)
            {
                /* calculate longname FDB count needed on target drive */
                if (StringLength16(wTmpFCLName) % 13)
                    longname_FDB_count = (StringLength16(wTmpFCLName)/13 + 1);
                else
                    longname_FDB_count = StringLength16(wTmpFCLName)/13;

                if (LongNameCopy(stTargetDrive, wTmpFCLName, FCnode) != FS_SUCCEED)
                {
                    MP_ALERT("%s: LongNameCopy() failed !", __FUNCTION__);
                    ext_mem_free(wTmpFCLName);
                    return ABNORMAL_STATUS;
                }
            }

            ext_mem_free(wTmpFCLName);

            if (FdbCopy(stTargetDrive, FCnode) != FS_SUCCEED)
            {
                MP_ALERT("%s: FdbCopy() failed !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }

            /* keep original FDB position info of the directory */
            SaveCurrentDirNodePosition(stTargetDrive, &fdb_pos_info);

    #if HELP_DEBUGGING_FILE_COPY_ISSUES
            MP_ALERT("+++ %s: SAVING (dwDirStart, dwDirPoint, dwFdbOffset) = (%lu, %lu, %lu)", __FUNCTION__, fdb_pos_info.dwDirStart, fdb_pos_info.dwDirPoint, fdb_pos_info.dwFdbOffset);
    #endif
        }
  #if EXFAT_ENABLE
        else
        {
            /* prepare a 8.3 short name FDB node for the FAT12/16/32 target drive */
            MpMemSet((BYTE *) FCnode, 0, sizeof(FDB));
            MpMemSet((BYTE *) FCnode->Name, 0x20, 11);

            BYTE  fullname_len = StringLength16(wTmpFCLName);
            BYTE  ext_name_len = 0, ext_name_offset = 0, count = 0;

            for (i = fullname_len - 1; i > 0; i--)
            {
                if (wTmpFCLName[i] == 0x002E) //'.' character
                {
                    ext_name_offset = i;
                    ext_name_len = StringLength16(&wTmpFCLName[ext_name_offset + 1]);
                    break;
                }
            }

            /* fill the extension filename field of FDB node */
            if (ext_name_len)
            {
                for (j = ext_name_offset + 1; (j < fullname_len) && (count < ext_name_len) && (count < 3); j++, count++)
                {
                    FCnode->Extension[count] = (BYTE) wTmpFCLName[j];
                    CharUpperCase08((BYTE *) &FCnode->Extension[count]);
                }
            }

            /* fill the primary filename field of FDB node */
            BYTE  fname_len_to_copy = (ext_name_len > 0)? ext_name_offset : fullname_len;
            for (j = 0; (j < fname_len_to_copy) && (j < 8); j++)
            {
                FCnode->Name[j] = (BYTE) wTmpFCLName[j];
                CharUpperCase08((BYTE *) &FCnode->Name[j]);
            }

            if (FCnode->Name[0] == 0xE5)
                FCnode->Name[0] = 0x05;

            /* processing other info fields (file attributes, date/time, ... etc) for the FDB node */
            DIR_ENTRY_INFO_TYPE  dir_entry_info;
            MpMemSet((BYTE *) &dir_entry_info, 0, sizeof(DIR_ENTRY_INFO_TYPE));
            if (GetFileAttributeAndTimeInfoFromFDB((DRIVE *) stSourceHandle->Drv, (FDB *) stSourceHandle->Drv->Node, &dir_entry_info) != PASS)
            {
                MP_ALERT("%s: GetFileAttributeAndTimeInfoFromFDB() failed !", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return ABNORMAL_STATUS;
            }

            if (dir_entry_info.fileAttribute.f_ReadOnly)
                FCnode->Attribute |= FDB_READ_ONLY;
            if (dir_entry_info.fileAttribute.f_Hidden)
                FCnode->Attribute |= FDB_HIDDEN;
            if (dir_entry_info.fileAttribute.f_System)
                FCnode->Attribute |= FDB_SYSTEM;
            if (dir_entry_info.fileAttribute.f_SubDir)
                FCnode->Attribute |= FDB_SUB_DIR;
            if (dir_entry_info.fileAttribute.f_Archive)
                FCnode->Attribute |= FDB_ARCHIVE;

            FCnode->CreateDate = (WORD) FileSetDate_for_FATxx(dir_entry_info.create_date_time.year, dir_entry_info.create_date_time.month, dir_entry_info.create_date_time.day);
            FCnode->CreateTime = (WORD) FileSetTime_for_FATxx(dir_entry_info.create_date_time.hour, dir_entry_info.create_date_time.minute, dir_entry_info.create_date_time.second);
            FCnode->ModifyDate = (WORD) FileSetDate_for_FATxx(dir_entry_info.modify_date_time.year, dir_entry_info.modify_date_time.month, dir_entry_info.modify_date_time.day);
            FCnode->ModifyTime = (WORD) FileSetTime_for_FATxx(dir_entry_info.modify_date_time.hour, dir_entry_info.modify_date_time.minute, dir_entry_info.modify_date_time.second);
            FCnode->AccessDate = (WORD) FileSetDate_for_FATxx(dir_entry_info.access_date_time.year, dir_entry_info.access_date_time.month, dir_entry_info.access_date_time.day);
            FCnode->StartHigh = (WORD) byte_swap_of_word((WORD) (((DWORD) dir_entry_info.startCluster & 0xffff0000) >> 16));
            FCnode->StartLow = (WORD) byte_swap_of_word((WORD) ((DWORD) dir_entry_info.startCluster & 0x0000ffff));

            /* For FAT12/16/32, folder's file size field in the FDB node is always 0; For exFAT, folder's file size is its total clusters length */
            if ((stTargetDrive->Flag.FsType != FS_TYPE_exFAT) && (dir_entry_info.fileAttribute.f_SubDir))
                FCnode->Size = 0;
            else
                FCnode->Size = (DWORD) byte_swap_of_dword((DWORD) dir_entry_info.fileSize);

        #if 0 //just for debug purpose
            {
                BYTE pri_name[9], ext_name[4];
                MpMemSet(pri_name, 0, 9);
                MpMemSet(ext_name, 0, 4);
                for (i = 0; i < 8; i++)
                {
                    if (FCnode->Name[i] != ' ')
                        pri_name[i] = FCnode->Name[i];
                    else
                        break;
                }
                for (i = 0; i < 3; i++)
                {
                    if (FCnode->Extension[i] != ' ')
                        ext_name[i] = FCnode->Extension[i];
                    else
                        break;
                }
                MP_DEBUG("%s: 8.3 short name in FDB: primary = [%s], ext name = [%s]", __FUNCTION__, pri_name, ext_name);
            }
        #endif

            /* build final 8.3 short name FDB for FAT12/16/32 without filename collision */
            if (BuildNewFDB(stTargetDrive, FCnode) != FS_SUCCEED)
            {
                MP_ALERT("%s: BuildNewFDB() failed !", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return ABNORMAL_STATUS;
            }

            /* calculate longname FDB count needed on target drive */
            if (StringLength16(wTmpFCLName) % EXFAT_ENTRY_FILE_NAME_LEN)
                longname_FDB_count = (StringLength16(wTmpFCLName)/EXFAT_ENTRY_FILE_NAME_LEN + 1);
            else
                longname_FDB_count = StringLength16(wTmpFCLName)/EXFAT_ENTRY_FILE_NAME_LEN;

            /* allocate and process long name FDBs for the UTF-16 filename for FAT12/16/32 */
            if (LongNameCopy(stTargetDrive, wTmpFCLName, FCnode) != FS_SUCCEED)
            {
                MP_ALERT("%s: LongNameCopy() failed !", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return ABNORMAL_STATUS;
            }

            ext_mem_free(wTmpFCLName);

            /* allocate and copy the 8.3 short name FDB content for FAT12/16/32 */
            if (FdbCopy(stTargetDrive, FCnode) != FS_SUCCEED)
            {
                MP_ALERT("%s: FdbCopy() failed !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }

            /* keep original FDB position info of the directory */
            SaveCurrentDirNodePosition(stTargetDrive, &fdb_pos_info);

    #if HELP_DEBUGGING_FILE_COPY_ISSUES
            MP_ALERT("+++ %s: SAVING--2 (dwDirStart, dwDirPoint, dwFdbOffset) = (%lu, %lu, %lu)", __FUNCTION__, fdb_pos_info.dwDirStart, fdb_pos_info.dwDirPoint, fdb_pos_info.dwFdbOffset);
    #endif
        }
  #endif
    }
  #if EXFAT_ENABLE
    else
    {
        mpDebugPrint("%s: To-Do: how to process File Copy if target drive is an exFAT drive ??", __FUNCTION__);
        return ABNORMAL_STATUS; //not supported yet
    }
  #endif

#if USBOTG_HOST_PTP
    if ((stSourceHandle->Drv->DevID == DEV_USB_HOST_PTP) || (stSourceHandle->Drv->DevID == DEV_USBOTG1_HOST_PTP)) //(stSourceHandle->Drv->DevID == DEV_USB_HOST_PTP)
    {
        (DWORD) stSourceHandle->Drv->Node = stSourceHandle->DirSector; // this is object ID number
    }
#endif //  USBHOST_PTP

    MpMemSet(&stTargetChain, 0, sizeof(CHAIN));
    if (ChainCopy(stTargetDrive, (CHAIN *) (&stTargetChain), (DRIVE *) (stSourceHandle->Drv),
        (CHAIN *) (&stSourceHandle->Chain), dwBufAddress, dwTempBufSize) != FS_SUCCEED)
    {
        f_copy_broken = TRUE;

        if (! SystemCardPresentCheck(stTargetDrive->DrvIndex))
        {
            MP_ALERT("%s: Target drive card not present !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
            f_broken_need_to_ChainFree = TRUE;
    }

COPY_END:
    dwCluster = stTargetChain.Start;
    if (stTargetDrive->Flag.FsType != FS_TYPE_exFAT)
    {
        /* restore the current FDB node position to the node position previously stored */
        RestoreDirNodePosition(stTargetDrive, &fdb_pos_info);

    #if HELP_DEBUGGING_FILE_COPY_ISSUES
        MP_ALERT("+++ %s: RESTORING (dwDirStart, dwDirPoint, dwFdbOffset) = (%lu, %lu, %lu)", __FUNCTION__, fdb_pos_info.dwDirStart, fdb_pos_info.dwDirPoint, fdb_pos_info.dwFdbOffset);
    #endif

        FCnode = (FDB *) (stTargetDrive->Node);

        if (! f_copy_broken)
        {
            SaveAlien16(&FCnode->StartHigh, (WORD) (dwCluster >> 16));
            SaveAlien16(&FCnode->StartLow, (WORD) (dwCluster));
        }
        else
        {
            if (f_broken_need_to_ChainFree) /* need to release allocated clusters on target drive */
            {
                MP_ALERT("%s: Copy failed => release allocated clusters on target drive ...", __FUNCTION__);
                MP_DEBUG("%s: start cluster of target chain to be ChainFree() = %u", __FUNCTION__, stTargetChain.Start);

                /* remove incomplete chain, when copy failed */
                if (ChainFree(stTargetDrive, (CHAIN *) (&stTargetChain)) != FS_SUCCEED)
                {
                    MP_ALERT("%s: remove incomplete chain, but ChainFree() failed ! => check target drive please ...", __FUNCTION__);
                }
            }

            if (! SystemCardPresentCheck(stTargetDrive->DrvIndex))
            {
                MP_ALERT("%s: Target drive card not present !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }

            MP_ALERT("%s: Copy failed => delete FDB nodes on target drive ...", __FUNCTION__);
            FCnode->Name[0] = 0xE5; /* mark this FDB node as deleted */
            stTargetDrive->Flag.DirCacheChanged = 1; /* set DirCacheChanged flag here to make sure it will be write back when DirCaching() due to PreviousNode()/NextNode() ! */
            for (i = 0; i < longname_FDB_count; i++)
            {
                if (PreviousNode(stTargetDrive) != FS_SUCCEED)
                {
                    MP_ALERT("%s: PreviousNode() failed !", __FUNCTION__);
                    break;
                }

                if ((stTargetDrive->Node->Attribute == FDB_LONG_NAME) && (stTargetDrive->Node->Name[0] != 0xE5))
                {
                    stTargetDrive->Node->Name[0] = 0xE5; /* mark this "longname" FDB node as deleted */
                    stTargetDrive->Flag.DirCacheChanged = 1; /* set DirCacheChanged flag here to make sure it will be write back when DirCaching() due to PreviousNode()/NextNode() ! */
                }
                else
                    break;
            }
        }
    }
  #if EXFAT_ENABLE
    else
    {
        mpDebugPrint("%s: To-Do: final update exFAT DirEntries => how to process File Copy if target drive is an exFAT drive ??", __FUNCTION__);
        return ABNORMAL_STATUS; //not supported yet
    }
  #endif

    /* note: even flag was already set in FdbCopy(), it may be changed during ChainCopy() process. Here, we must set it for DriveRefresh() writing back to disk */
    stTargetDrive->Flag.DirCacheChanged = 1;
    if (DriveRefresh(stTargetDrive) != FS_SUCCEED)
    {
        MP_ALERT("%s: DriveRefresh() failed !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (f_copy_broken)
        return ABNORMAL_STATUS;
    else
        return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Copy whole content of the specified directory from the source drive to the target drive.
///
///@param   stTargetDrive     DRIVE handle of the target drive. \n\n
///@param   stSourceDrive     DRIVE handle of the source drive. \n\n
///@param   dwBufAddress      The pointer/address of a prepared temporary buffer to be used during the copy process. \n\n
///@param   dwTempBufSize     Size of the prepared temporary buffer.
///
///@retval  FS_SUCCEED           Copy successfully. \n\n
///@retval  ABNORMAL_STATUS      Copy unsuccessfully. \n\n
///@retval  INVALID_DRIVE        Copy unsuccessfully due to invalid drive handle specified. \n\n
///@retval  DISK_FULL            Copy unsuccessfully due to no enough free space on the target drive. \n\n
///@retval  USER_CANCEL_COPY     User cancelled the copy operation.
///
///@remark  The source directory to copy is implicitly pointed to by current FDB node of source drive (drv->Node) before calling this function.
///
SWORD DirectoryCopy(DRIVE * stTargetDrive, DRIVE * stSourceDrive, DWORD dwBufAddress, DWORD dwTempBufSize)
{
    FDB   sTempFnode;
    FDB   *FCnode;
    DWORD i;
    SWORD swRet;
    WORD wStartStackPoint;
    STREAM *srcFileHandle;
    DWORD bufByteCount = 0;
    ENTRY_FILENAME_TYPE  bFilenameType = E_FILENAME_UNKNOWN;
    WORD  *wTmpFCLName;
    BOOL  f_Utf16_LongName;
    WORD  wTmp_LongName_Buf[MAX_L_NAME_LENG];
    DWORD dwLNameChged;


    MP_DEBUG("Enter %s()...", __FUNCTION__);

    if ((stTargetDrive == NULL) || (stSourceDrive == NULL))
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (!SystemCardPresentCheck(stTargetDrive->DrvIndex) || !SystemCardPresentCheck(stSourceDrive->DrvIndex))
    {
        MP_ALERT("%s: Directory Copying is not performed due to card not present !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    //check free space, reserved 4 clusters for build subdir
    if (CheckSpaceIfEnough(stTargetDrive, 4 * (stTargetDrive->ClusterSize << stTargetDrive->bSectorExp)) == DISK_FULL)
    {
        MP_ALERT("%s: free disk space not enough !!", __FUNCTION__);
        return DISK_FULL;
    }

    wTmpFCLName = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
    if (wTmpFCLName == NULL)
    {
        MP_ALERT("%s: -E malloc fail", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
    MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);

    if (GetFilenameOfCurrentNode((DRIVE *) stSourceDrive, &bFilenameType, (BYTE *) wTmpFCLName, &bufByteCount, 512) != FS_SUCCEED)
    {
        MP_ALERT("%s: GetFilenameOfCurrentNode() failed !", __FUNCTION__);
        ext_mem_free(wTmpFCLName);
        return ABNORMAL_STATUS;
    }

    if (bFilenameType == E_FILENAME_8_3_SHORT_NAME)
        f_Utf16_LongName = 0;
    else if (bFilenameType == E_FILENAME_UTF16_LONG_NAME)
    {
        f_Utf16_LongName = 1;
        MpMemSet((BYTE *) wTmp_LongName_Buf, 0, MAX_L_NAME_LENG * 2);
        MpMemCopy((BYTE *) wTmp_LongName_Buf, (BYTE *) wTmpFCLName, bufByteCount);
    }

    //get file node
    FCnode = &sTempFnode;
    MpMemSet((BYTE *) FCnode, 0, sizeof(FDB));

    if (stSourceDrive->Flag.FsType != FS_TYPE_exFAT)
    {
        MpMemCopy((BYTE *) FCnode, (BYTE *) stSourceDrive->Node, sizeof(FDB));

        if (! f_Utf16_LongName) /* 8.3 short filename */
        {
            BYTE pri_name[9], ext_name[4];
            MpMemSet(pri_name, 0, 9);
            MpMemSet(ext_name, 0, 4);
            for (i = 0; i < 8; i++)
            {
                if (stSourceDrive->Node->Name[i] != ' ')
                    pri_name[i] = stSourceDrive->Node->Name[i];
                else
                    break;
            }
            for (i = 0; i < 3; i++)
            {
                if (stSourceDrive->Node->Extension[i] != ' ')
                    ext_name[i] = stSourceDrive->Node->Extension[i];
                else
                    break;
            }

            if (FileSearch(stTargetDrive, pri_name, ext_name, E_BOTH_FILE_AND_DIR_TYPE) == FS_SUCCEED) /* same file name found */
            {
                BYTE ascii_full_name[13];
                MpMemSet(ascii_full_name, 0, 13);
                strcpy(ascii_full_name, pri_name);
                if (StringLength08(ext_name))
                {
                    strcat(ascii_full_name, ".");
                    strcat(ascii_full_name, ext_name);
                }

                MpMemSet((BYTE *) stSourceDrive->LongName, 0, MAX_L_NAME_LENG * 2);
                mpx_UtilUtf8ToUnicodeU16((WORD *) stSourceDrive->LongName, ascii_full_name); /* convert to UTF-16 string */
                f_Utf16_LongName = 1; /* after adding "COPY(XX)_" prefix, it will always become a long filename */
                MpMemSet((BYTE *) wTmp_LongName_Buf, 0, MAX_L_NAME_LENG * 2);
                MpMemCopy((BYTE *) wTmp_LongName_Buf, (BYTE *) stSourceDrive->LongName, StringLength08(ascii_full_name) * 2);
            }
        }
        else
        {
            MpMemSet((BYTE *) stSourceDrive->LongName, 0, MAX_L_NAME_LENG * 2);
            MpMemCopy((BYTE *) stSourceDrive->LongName, (BYTE *) wTmp_LongName_Buf, bufByteCount);
        }

        dwLNameChged = 0;
        if (f_Utf16_LongName)
        {
            MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);
            if (BuildNewLName(stTargetDrive, stSourceDrive, wTmpFCLName, &dwLNameChged) != FS_SUCCEED)
            {
                MP_ALERT("%s: BuildNewLName() failed !", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return ABNORMAL_STATUS;
            }

            if (StringLength16(wTmpFCLName) % 13)
                stSourceDrive->CacheBufPoint->LongNameFdbCount = (StringLength16(wTmpFCLName)/13 + 1);
            else
                stSourceDrive->CacheBufPoint->LongNameFdbCount = StringLength16(wTmpFCLName)/13;

        #if 0 //just for debug purpose
            BYTE tmpBuf[512];
            {
                MpMemSet(tmpBuf, 0, 512);
                mpx_UtilU16ToU08(tmpBuf, (WORD *) wTmpFCLName); /* convert UTF-16 to UTF-8 string */
                MP_DEBUG("%s: final target filename = %s", __FUNCTION__, tmpBuf);
            }
        #endif
        }

        // if long name changed, get the new short name
        // because we only add "COPY(XX)_" to long name, and they are all ASCII code
        // so, only get the low byte data
        if (dwLNameChged)
        {
            for (i = 0; i < 6; i++)
                FCnode->Name[i] = (BYTE) wTmpFCLName[i];

            FCnode->Name[6] = 0x7e;	// " ~ "
            FCnode->Name[7] = 0x31;	// " 1 "
        }
    }
#if EXFAT_ENABLE
    else
    {
        MpMemSet((BYTE *) stSourceDrive->LongName, 0, MAX_L_NAME_LENG * 2);
        MpMemCopy((BYTE *) stSourceDrive->LongName, (BYTE *) wTmp_LongName_Buf, bufByteCount);

        dwLNameChged = 0;
        MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);
        if (BuildNewLName(stTargetDrive, (DRIVE *) (stSourceDrive), wTmpFCLName, &dwLNameChged) != FS_SUCCEED)
        {
            MP_ALERT("%s: BuildNewLName() failed !", __FUNCTION__);
            ext_mem_free(wTmpFCLName);
            return ABNORMAL_STATUS;
        }

    #if 0 //just for debug purpose
        BYTE tmpBuf[512];
        {
            MpMemSet(tmpBuf, 0, 512);
            mpx_UtilU16ToU08(tmpBuf, (WORD *) wTmpFCLName); /* convert UTF-16 to UTF-8 string */
            MP_DEBUG("%s: final target filename = %s", __FUNCTION__, tmpBuf);
        }
    #endif
    }
#endif

    if (stTargetDrive->Flag.FsType != FS_TYPE_exFAT)
    {
        if (stSourceDrive->Flag.FsType != FS_TYPE_exFAT)
        {
            if (f_Utf16_LongName)
            {
                MP_DEBUG("%s: => MakeDir_UTF16()...", __FUNCTION__);
                if (MakeDir_UTF16(stTargetDrive, wTmpFCLName) != FS_SUCCEED)
                {
                    MP_ALERT("%s: MakeDir_UTF16() failed !", __FUNCTION__);
                    ext_mem_free(wTmpFCLName);
                    return ABNORMAL_STATUS;
                }
            }
            else
            {
                if (FCnode->Name[0] == 0xE5)
                    FCnode->Name[0] = 0x05;

                BYTE pri_name[9], ext_name[4];
                MpMemSet(pri_name, 0, 9);
                MpMemSet(ext_name, 0, 4);
                for (i = 0; i < 8; i++)
                {
                    if (FCnode->Name[i] != ' ')
                        pri_name[i] = FCnode->Name[i];
                    else
                        break;
                }
                for (i = 0; i < 3; i++)
                {
                    if (FCnode->Extension[i] != ' ')
                        ext_name[i] = FCnode->Extension[i];
                    else
                        break;
                }

                if (StringLength08(ext_name) > 0)
                {
                    MP_ALERT("%s: => MakeDir(\"%s.%s\")...", __FUNCTION__, pri_name, ext_name);
                    swRet = MakeDir(stTargetDrive, pri_name, ext_name);
                }
                else
                {
                    MP_ALERT("%s: => MakeDir(\"%s\")...", __FUNCTION__, pri_name);
                    swRet = MakeDir(stTargetDrive, pri_name, "");
                }

                if (swRet != FS_SUCCEED)
                {
                    MP_ALERT("%s: MakeDir() failed !", __FUNCTION__);
                    ext_mem_free(wTmpFCLName);
                    return ABNORMAL_STATUS;
                }
            }
        }
  #if EXFAT_ENABLE
        else
        {
            MP_DEBUG("%s: => MakeDir_UTF16()...", __FUNCTION__);
            if (MakeDir_UTF16(stTargetDrive, wTmpFCLName) != FS_SUCCEED)
            {
                MP_ALERT("%s: MakeDir_UTF16() failed !", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return ABNORMAL_STATUS;
            }
        }
  #endif

        ext_mem_free(wTmpFCLName);

        if (DriveRefresh(stTargetDrive) != FS_SUCCEED)
        {
            MP_ALERT("%s: DriveRefresh() failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    }
  #if EXFAT_ENABLE
    else
    {
        mpDebugPrint("%s: To-Do: how to process Folder Copy if target drive is an exFAT drive ??", __FUNCTION__);
        return ABNORMAL_STATUS; //not supported yet
    }
  #endif

/*
#if COPY_CANCEL_ENABLE
    if (Polling_Event())
    {
        return USER_CANCEL_COPY;
    }
#endif
*/

    if (CdSub(stTargetDrive) != FS_SUCCEED)
    {
        return ABNORMAL_STATUS;
    }
    if (CdSub(stSourceDrive) != FS_SUCCEED)
    {
        return ABNORMAL_STATUS;
    }

#if COPY_CANCEL_ENABLE //Lighter 3/6
    if (Polling_Event())
    {
        return USER_CANCEL_COPY;
    }
#endif

    wStartStackPoint = stSourceDrive->DirStackPoint;
    swRet = FS_SUCCEED;
    while (swRet == FS_SUCCEED)
    {
        MP_DEBUG("%s: => SweepFiling()...", __FUNCTION__);
        swRet = SweepFiling(stTargetDrive, stSourceDrive, wStartStackPoint);

        if (swRet == FS_SUCCEED)
        {
            srcFileHandle = FileOpen(stSourceDrive);
            if (srcFileHandle == NULL)
            {
                return ABNORMAL_STATUS;
            }

            MP_DEBUG("%s: => FileCopy()...", __FUNCTION__);
            swRet = FileCopy(stTargetDrive, srcFileHandle, dwBufAddress, dwTempBufSize);
            FileClose(srcFileHandle);
        }

        if (swRet == FS_SUCCEED)
        {
            if (fileUiUpdateIconAniPtr)
                fileUiUpdateIconAniPtr();
        }

#if COPY_CANCEL_ENABLE
        if (Polling_Event())
        {
            return USER_CANCEL_COPY;
        }
#endif
        //TaskYield();
    }

    if (swRet == END_OF_DIR)
    {
        CdParent(stSourceDrive);
        CdParent(stTargetDrive);
        swRet = FS_SUCCEED;
    }

    return swRet;
}



///
///@ingroup FILE
///@brief   Copy all files and directories with all the content within the directories from the source drive to the target drive.
///
///@param   stSourceDrive     The drive handle of the source drive. \n\n
///@param   stTargetDrive     The drive handle of the target drive.
///
///@retval  FS_SUCCEED        Copy successfully. \n\n
///@retval  ABNORMAL_STATUS   Copy unsuccessfully. \n\n
///@retval  INVALID_DRIVE     Copy unsuccessfully due to invalid drive handle specified. \n\n
///@retval  DISK_READ_ONLY    Copy unsuccessfully due to the target drive is read-only.
///
SWORD DriveCopyAllFiles(DRIVE * stTargetDrive, DRIVE * stSourceDrive)
{
    SWORD ret = FS_SUCCEED;
    DWORD buffer;
    DWORD TempBufSize = 512 * 1024;
    STREAM *srcFileHandle;


    MP_ALERT("Enter %s()...", __FUNCTION__);

    if ((stTargetDrive == NULL) || (stSourceDrive == NULL))
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (SystemGetFlagReadOnly(stTargetDrive->DrvIndex))
    {
        MP_ALERT("%s: Drive Copying is not performed due to some error (target drive is read-only) !!", __FUNCTION__);
        return DISK_READ_ONLY;
    }

    if (!SystemCardPresentCheck(stTargetDrive->DrvIndex) || !SystemCardPresentCheck(stSourceDrive->DrvIndex))
    {
        MP_ALERT("%s: Drive Copying is not performed due to card not present !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* back to root directory first */
    if (DirReset(stSourceDrive) != FS_SUCCEED)
    {
        MP_ALERT("%s: Drive Copying is not performed due to some error (DirReset() failed) !!", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* back to root directory first */
    if (DirReset(stTargetDrive) != FS_SUCCEED)
    {
        MP_ALERT("%s: Drive Copying is not performed due to some error (DirReset() failed) !!", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    buffer = (DWORD) ext_mem_malloc(TempBufSize);
    if ((BYTE *) buffer == NULL)
    {
        MP_ALERT("%s: malloc failed for buffer size (%d) !!", __FUNCTION__, TempBufSize);
        return ABNORMAL_STATUS;
    }

    ret = DirLast(stTargetDrive);
    if ((ret != FS_SUCCEED) && (ret != END_OF_DIR))
    {
        MP_ALERT("%s: Drive Copying is not performed due to some error (DirLast() failed) !!", __FUNCTION__);
        ext_mem_free((BYTE *) buffer);
        return ABNORMAL_STATUS;
    }

    WORD wStartStackPoint = stSourceDrive->DirStackPoint;

    /* check whether if first node in root directory is a long name FDB, and keep its long name FDBs count */
    ScanFileName(stSourceDrive);

    while (1)
    {
        ret = FS_SUCCEED;
        while (ret == FS_SUCCEED)
        {
            ret = SweepFiling(stTargetDrive, stSourceDrive, wStartStackPoint);
            if (ret == FS_SUCCEED)
            {
                srcFileHandle = FileOpen(stSourceDrive);
                if (srcFileHandle == NULL)
                {
                    MP_ALERT("%s: Drive Copying is not fully complete due to some error !!", __FUNCTION__);
                    ext_mem_free((BYTE *) buffer);
                    return ABNORMAL_STATUS;
                }

                MP_ALERT("%s: => FileCopy()...", __FUNCTION__);
                ret = FileCopy(stTargetDrive, srcFileHandle, (DWORD) buffer, TempBufSize);
                FileClose(srcFileHandle);
            }

            if (ret == FS_SUCCEED)
            {
                if (fileUiUpdateIconAniPtr)
                    fileUiUpdateIconAniPtr();
            }

	#if COPY_CANCEL_ENABLE
            if (Polling_Event())
            {
                MP_ALERT("%s: Drive Copying is cancelled by User !", __FUNCTION__);
                return USER_CANCEL_COPY;
            }
	#endif
        } /* while (ret == FS_SUCCEED) */

        if (ret == END_OF_DIR)
        {
            ret = NextNode(stSourceDrive);
        }

        if (ret != FS_SUCCEED)
        {
            if (ret == END_OF_DIR)
                ret = FS_SUCCEED;

            break;
        }
        //TaskYield();
    } /* while (1) */

    MP_ALERT("%s: Drive Copying finished.", __FUNCTION__);
    ext_mem_free((BYTE *) buffer);
    return ret;
}



///
///@ingroup FILE
///@brief   Get the total number of image files on the specified drive.
///
///@param   sDrv             The drive to get file statistics. \n\n
///@param   pdwTotalFiles    [OUT] The pointer of the returned file statistics.
///
///@retval  FS_SUCCEED        Get info successfully. \n\n
///@retval  INVALID_DRIVE     Invalid drive handle specified. \n\n
///@retval  ABNORMAL_STATUS   Get info unsuccessfully.
///
SWORD GetImageFileStatistics(DRIVE * sDrv, DWORD * pdwTotalFiles)
{
    if (sDrv == NULL)
        return INVALID_DRIVE;

    if (pdwTotalFiles == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    *pdwTotalFiles = sDrv->dwImageTotalFiles;
    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Get the total number of audio files on the specified drive.
///
///@param   sDrv             The drive to get file statistics. \n\n
///@param   pdwTotalFiles    [OUT] The pointer of the returned file statistics.
///
///@retval  FS_SUCCEED        Get info successfully. \n\n
///@retval  INVALID_DRIVE     Invalid drive handle specified. \n\n
///@retval  ABNORMAL_STATUS   Get info unsuccessfully.
///
SWORD GetAudioFileStatistics(DRIVE * sDrv, DWORD * pdwTotalFiles)
{
    if (sDrv == NULL)
        return INVALID_DRIVE;

    if (pdwTotalFiles == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    *pdwTotalFiles = sDrv->dwAudioTotalFiles;
    return FS_SUCCEED;
}



///
///@ingroup FILE
///@brief   Get the total number of movie files on the specified drive.
///
///@param   sDrv            The drive to get file statistics. \n\n
///@param   pdwTotalFiles   [OUT] The pointer of the returned file statistics.
///
///@retval  FS_SUCCEED        Get info successfully. \n\n
///@retval  INVALID_DRIVE     Invalid drive handle specified. \n\n
///@retval  ABNORMAL_STATUS   Get info unsuccessfully.
///
SWORD GetMovieFileStatistics(DRIVE * sDrv, DWORD * pdwTotalFiles)
{
    if (sDrv == NULL)
        return INVALID_DRIVE;

    if (pdwTotalFiles == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    *pdwTotalFiles = sDrv->dwMovieTotalFiles;
    return FS_SUCCEED;
}



STREAM *GetFreeFileHandle(DRIVE *drv)
{
    STREAM *handle;
    int counter;
    BOOL found = FALSE;

    if (HandleCounter == FS_MAX_STREAM)  // if no handle can use then return null
        return NULL;

#if (! RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (!drv)
        return NULL;
#endif

    IntDisable();
    handle = (STREAM *) StreamTable;
    counter = FS_MAX_STREAM;

    while (counter)  // search a free handle in the STREAM table (i.e. File table)
    {
        /* if 'Drv' field in STREAM structure is 0, means this STREAM handle is free/available, except "RamFile" */
        if ((! handle->Drv) && (! handle->Flag.f_IsRamFile))
        {
            found = TRUE;
            handle->Drv = drv; /* set to non-zero, means this STREAM handle is allocated, except "RamFile" */
            handle->Flag.f_ExtraClustersAllocated = 0; /* always set 'f_ExtraClustersAllocated' flag to 0 when get a file handle */
            break;
        }

        handle++;
        counter--;
    }

    if (found == TRUE)
    {
        HandleCounter++;  // increase the opened STREAM handle counter
        IntEnable();

        return handle;     // return STREAM handle pointer
    }

    IntEnable();

    return NULL;
}


/*
*******************************************************************************
*        LOCAL FUNCTIONS
*******************************************************************************
*/

#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup FILE
///@brief   Calculate and separate the data region of a read/write operation to be performed in the file into
///         three segments: head, body and tail parts.\n
///           head part means the beginning fragmented part whose size is less than a sector.\n
///           tail part means the ending fragmented part whose size is less than a sector.\n
///           body part means the middle part whose size is exactly multiples of a sector size.
///
///@param   handle    The handle of the file to access. \n\n
///@param   head      [OUT] Byte size of the head part of this read/write operation to be performed. \n\n
///@param   body      [OUT] Byte size of the body part of this read/write operation to be performed. \n\n
///@param   tail      [OUT] Byte size of the tail part of this read/write operation to be performed. \n\n
///@param   size      [IN]  Byte size of the total data of this read/write operation to be performed.
///
///@return  None.
///
///@remark  The calculated sizes of the head part and the tail part may be 0.
///
#endif
void Segmentize(STREAM * handle, DWORD * head, DWORD * body, DWORD * tail, DWORD size)
{
    WORD offset;
    CHAIN *chain;
    DWORD tmpsize;


    if (handle == NULL)
        return;

    if (handle->Drv == NULL)
        return; // this file has been closed

    if ((head == NULL) || (body == NULL) || (tail == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return;
    }

    tmpsize = size;
    chain = (CHAIN *) (&handle->Chain);
    offset = chain->Point & ((1 << handle->Drv->bSectorExp) - 1); // the byte offset in sector

    if (offset)
    {
        if (offset + tmpsize <= (1 << handle->Drv->bSectorExp))
            *head = tmpsize;
        else
            *head = (1 << handle->Drv->bSectorExp) - offset;

        tmpsize -= *head;
    }
    else
        *head = 0;

    *tail = tmpsize & ((1 << handle->Drv->bSectorExp) - 1);
    *body = tmpsize - *tail;
}



static void SwapData(DWORD DestAddr, DWORD SourceAddr, DWORD dwSize)
{
    BYTE btmpBuffer[MAX_ELEMENT_SIZE];

    if (dwSize > MAX_ELEMENT_SIZE)
    {
        MP_ALERT("%s: -E-  temp buffer size (%u) is less than element size (%u)!  Please modify code to enlarge it !", __FUNCTION__, MAX_ELEMENT_SIZE, dwSize);
        return;
    }

    if (DestAddr != SourceAddr)
    {
        //MP_DEBUG("Swap data: dest(%x), sour(%x) size: %d", DestAddr, SourceAddr, dwSize);
        MpMemCopy((BYTE *) btmpBuffer, (BYTE *) SourceAddr, dwSize);
        MpMemCopy((BYTE *) SourceAddr, (BYTE *) DestAddr, dwSize);
        MpMemCopy((BYTE *) DestAddr,   (BYTE *) btmpBuffer, dwSize);
    }
}



///
///@ingroup FILE
///@brief   Get/extract (year, month, day) date info from a 16-bit little-endian date field value of a FAT12/16/32 file FDB node.
///
///@param   wDate       [IN] The 16-bit little-endian date field value within a FAT12/16/32 file FDB node. \n\n
///@param   wYear       [OUT] Output year value (1980 ~ 2107) of the date. \n\n
///@param   bMonth      [OUT] Output month value of the date. \n\n
///@param   bDay        [OUT] Output day value of the date.
///
///@return  None.
///
///@remark  The input 'wDate' value is 16-bit little-endian because FAT12/16/32 file systems on disk data structure
///         are all little-endian.
///
///@remark  This function is used for FAT12/16/32 file systems only. For exFAT, use FileGetDateTime_for_exFAT() instead.
///
void FileGetDate_for_FATxx(WORD wDate, WORD * wYear, BYTE * bMonth, BYTE * bDay)
{
    WORD wTempValue;


    if ((wYear == NULL) || (bMonth == NULL) || (bDay == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return;
    }

    wTempValue = byte_swap_of_word(wDate);
    *wYear = (WORD) (((wTempValue & 0xfe00) >> 9) + 1980);
    *bMonth = (BYTE) ((wTempValue & 0x01e0) >> 5);
    *bDay = (BYTE) (wTempValue & 0x001f);
}



///
///@ingroup FILE
///@brief   Get/extract (hour, minute, second) time info from a 16-bit little-endian time field value of a FAT12/16/32 file FDB node.
///
///@param   wTime       [IN] The 16-bit little-endian time field value within a FAT12/16/32 file FDB node. \n\n
///@param   bHour       [OUT] Output hour value of the time. \n\n
///@param   bMinute     [OUT] Output minute value of the time. \n\n
///@param   bSecond     [OUT] Output second value of the time.
///
///@return  None.
///
///@remark  The input 'wTime' value is 16-bit little-endian because FAT12/16/32 file systems on disk data structure
///         are all little-endian.
///
///@remark  This function is used for FAT12/16/32 file systems only. For exFAT, use FileGetDateTime_for_exFAT() instead.
///
void FileGetTime_for_FATxx(WORD wTime, BYTE * bHour, BYTE * bMinute, BYTE * bSecond)
{
    WORD wTempValue = byte_swap_of_word(wTime);


    if ((bHour == NULL) || (bMinute == NULL) || (bSecond == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return;
    }

    wTempValue = byte_swap_of_word(wTime);
    *bHour = (BYTE) ((wTempValue & 0xf800) >> 11);
    *bMinute = (BYTE) ((wTempValue & 0x07e0) >> 5);
    *bSecond = (BYTE) ((wTempValue & 0x001f) * 2); //unit: 2 seconds
}



///
///@ingroup FILE
///@brief   Convert the (year, month, day) date to a 16-bit little-endian date field value for FAT12/16/32 file FDB node.
///
///@param   wYear       Input year value (1980 ~ 2107) of the date. \n\n
///@param   bMonth      Input month value of the date. \n\n
///@param   bDay        Input day value of the date.
///
///@return  The converted 16-bit little-endian date field value for setting to a FAT12/16/32 FDB node.
///
///@remark  The returned date value for setting to a FAT12/16/32 FDB node is 16-bit little-endian because FAT12/16/32 file systems on disk
///         data structure are all little-endian.
///
///@remark  This function is used for FAT12/16/32 file systems only. For exFAT, use FileSetDateTime_for_exFAT() instead.
///
WORD FileSetDate_for_FATxx(WORD wYear, BYTE bMonth, BYTE bDay)
{
    WORD wTempValue=0, DateFiled_for_FDB = 0;

    //check for valid range
    if ( (wYear < 1980 || wYear > 2107) || (bMonth < 1 || bMonth > 12) || (bDay < 1 || bDay > 31) )
        return 0;

    wTempValue  = ((wYear - 1980) << 9);  //year:  7-bit
    wTempValue |= (bMonth << 5);          //month: 4-bit
    wTempValue |= bDay;                   //day:   5-bit

    DateFiled_for_FDB = byte_swap_of_word(wTempValue);
    return DateFiled_for_FDB;
}



///
///@ingroup FILE
///@brief   Convert the (hour, minute, second) time to a 16-bit little-endian time field value for FAT12/16/32 file FDB node.
///
///@param   bHour       Input hour value of the time. \n\n
///@param   bMinute     Input minute value of the time. \n\n
///@param   bSecond     Input second value of the time.
///
///@return  The converted 16-bit little-endian time field value for setting to a FAT12/16/32 FDB node.
///
///@remark  The returned time value for setting to a FAT12/16/32 FDB node is 16-bit little-endian because FAT12/16/32 file systems on disk
///         data structure are all little-endian.
///
///@remark  This function is used for FAT12/16/32 file systems only. For exFAT, use FileSetDateTime_for_exFAT() instead.
///
WORD FileSetTime_for_FATxx(BYTE bHour, BYTE bMinute, BYTE bSecond)
{
    WORD wTempValue=0, TimeFiled_for_FDB = 0;

    //check for valid range
    if ( (bHour > 23) || (bMinute > 59) || (bSecond > 59) ) //second: 0 ~ 29 (unit: 2 seconds)
        return 0;

    wTempValue =  ((WORD) bHour << 11);   //hour:   5-bit
    wTempValue |= ((WORD) bMinute << 5);  //minute: 6-bit
    wTempValue |= ((WORD) bSecond / 2);   //second: 5-bit

    TimeFiled_for_FDB = byte_swap_of_word(wTempValue);
    return TimeFiled_for_FDB;
}



///
///@ingroup FILE_BROWSER
///@brief   Get the current configuration value of sorting basis.
///
///@param   None.
///
///@return  Current configuration value of sorting basis.
///
FILE_SORTING_BASIS_TYPE GetCurFileSortingBasis(void)
{
    return g_CurFileSortingBasis;
}



///
///@ingroup FILE_BROWSER
///@brief   Configure the value of sorting basis and perform the sorting for all file lists of GUI.
///
///@param   sort_basis     The value of sorting basis to be used.
///
///@return  None.
///
void SetCurFileSortingBasis(FILE_SORTING_BASIS_TYPE sort_basis)
{
    if ((sort_basis >= FILE_SORTING_BY_83SHORT_NAME) && (sort_basis < FILE_SORTING_MAX) && (g_CurFileSortingBasis != sort_basis))
    {
        g_CurFileSortingBasis = sort_basis;

        /* re-sort each file list of GUI file browser for change on the fly */
        ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
        struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;

        MP_DEBUG("Sort each file list for GUI file browser viewing...");
        int  i, j;
        for (i=0; i < 4; i++)
        {
            if (i == OP_IMAGE_MODE)
            {
    #if (SONY_DCF_ENABLE)
                ST_SEARCH_INFO *pSearchInfo;
                DATE_TIME_INFO_TYPE  exif_date_time;
                FILE_SORTING_BASIS_TYPE  sorting_method;

                if (sort_basis == FILE_SORTING_BY_83SHORT_NAME_REVERSE)  /* reverse order => descending order of SONY file name order */
                    sorting_method = FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE;
                else if (sort_basis == FILE_SORTING_BY_83SHORT_NAME) /* ascending order of SONY file name order */
                    sorting_method = FILE_SORTING_BY_SONY_DCF_NAMING;
                else
                    sorting_method = sort_basis;

                /* For sorting by EXIF date time, get and keep EXIF date time info of each file in the file list first */
                if ((sorting_method == FILE_SORTING_BY_EXIF_DATE_TIME) || (sorting_method == FILE_SORTING_BY_EXIF_DATE_TIME_REVERSE))
                {
                    /* currently, we keep EXIF date time info of each file in the companion DCF info list */
                    if (g_FileList_DCF_Info_List != NULL)
                    {
                        pSearchInfo = (ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE];
                        for (j=0; j < psFileBrowser->dwFileListCount[OP_IMAGE_MODE]; j++)
                        {
                            if (FileBrowserGetImgEXIF_DateTime(((ST_SEARCH_INFO *) pSearchInfo) + j, &exif_date_time) == PASS)
                            {
                                g_FileList_DCF_Info_List[j].Flags.f_have_EXIF_time_info = 1;
                                MpMemCopy(&(g_FileList_DCF_Info_List[j].exif_date_time), &exif_date_time, sizeof(DATE_TIME_INFO_TYPE));
                            }
                            else
                            {
                                g_FileList_DCF_Info_List[j].Flags.f_have_EXIF_time_info = 0;
                                MpMemSet(&(g_FileList_DCF_Info_List[j].exif_date_time), 0, sizeof(DATE_TIME_INFO_TYPE));
                            }
                        }
                    }
                }

                if ((sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING) || (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE))
                {
                    /* Re-arrange the file list to two sets: DCF objects set, and non-DCF files set. And then sort the file list
                     * in SONY DCF file name ascending or descending order.
                     */
                    FileSort_for_Sony_DCF_Name_Order((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[i], psFileBrowser->dwFileListCount[i], sorting_method);
                }
                else
                    FileSort((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[i], psFileBrowser->dwFileListCount[i], sort_basis);
    #else
                FileSort((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[i], psFileBrowser->dwFileListCount[i], sort_basis);
    #endif
            }
            else
                FileSort((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[i], psFileBrowser->dwFileListCount[i], sort_basis);

            psFileBrowser->dwFileListIndex[i] = 0; //reset current item index
        }

        //reset current item index. (for compatible, so set to old vars)
        psFileBrowser->dwAudioCurIndex = psFileBrowser->dwImgAndMovCurIndex = 0;
    }
    return;
}



//---------------------------------------------------------------------------

//New Quick sort algorithm for speeding up		C.W 09/04/09

///
///@ingroup FILE_BROWSER
///@brief   Sort an array which is a GUI file list using QuickSort method.
///
///@param   pBase            The base pointer to the GUI file list for sorting.
///
///@param   dwElementCount   Total number of elements in the file list.
///
///@param   dwElementSize    Size (in bytes) of each element in the file list.
///
///@param   Compare          A function pointer to the comparing function for the file sorting.
///
///@param   reverse_order    A boolean flag to indicate using reverse ordering for file sorting or not.
///
///@return  FS_SUCCEED         Sort file list successfully. \n\n
///@return  ABNORMAL_STATUS    Sort file list unsuccessfully due to some error. \n\n
///
SWORD QuickSort(void * pBase, DWORD dwElementCount, DWORD dwElementSize,
                WORD (* Compare)(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order), BOOL reverse_order)
{
#define STACK_SIZE  (64 * 1024)	/* 64KB for sort stack.  Note: for safety, STACK_SIZE/4 must be larger than max file list entries count ! */
    ST_STACK_INFO *sStackInfo;
    register DWORD dwStackPoint, dwStackCount, dwSize;
    register DWORD i, j;
    BYTE  *bStackBuffer;
    BYTE  bPivotBuffer[MAX_ELEMENT_SIZE];
    SWORD swHigh, swLow;
    DWORD byte_offset1, byte_offset2, tmp_byte_offset;

    /* for updating the companion info list, not only for DCF info list */
    DWORD idx1_for_companion_info_list, idx2_for_companion_info_list, tmp_idx_for_companion_info_list;


    if (pBase == NULL)
    {
        MP_ALERT("%s: Error! NULL base pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (Compare == NULL)
    {
        MP_ALERT("%s: Error! NULL function pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (dwElementCount < 2)
    {
        return FS_SUCCEED;
    }

    if (dwElementSize > MAX_ELEMENT_SIZE)
    {
        MP_ALERT("%s: -E-  MAX_ELEMENT_SIZE (%u) is less than element size (%u)!  Please modify code to enlarge it !", __FUNCTION__, MAX_ELEMENT_SIZE, dwElementSize);
        return ABNORMAL_STATUS;
    }

    bStackBuffer = (BYTE *) ext_mem_malloc(STACK_SIZE);
    if (bStackBuffer == NULL)
    {
        MP_ALERT("%s: -E-  mem alloc failed !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
    MpMemSet(bStackBuffer, 0, STACK_SIZE);

    /* if count of elements to be sorted is very big, show a console message before and after the sorting */
    if (dwElementCount > 1000)
    {
        MP_ALERT("%s: file list sorting ...", __FUNCTION__);
    }

    //initial
    sStackInfo = (ST_STACK_INFO *) ((DWORD) bStackBuffer);
    sStackInfo->swLow = 0;
    sStackInfo->swHigh = (dwElementCount - 1);
    dwStackPoint = sizeof(ST_STACK_INFO);
    dwStackCount = 1;

    while (dwStackCount)
    {
        // get stack
        sStackInfo = (ST_STACK_INFO *) ((DWORD) bStackBuffer + (dwStackPoint - sizeof(ST_STACK_INFO)));
        swHigh = sStackInfo->swHigh;
        swLow = sStackInfo->swLow;

        if (swLow >= swHigh)
        {
            dwStackCount--;
            dwStackPoint -= sizeof(ST_STACK_INFO);
            continue;
        }

        byte_offset2 = swLow * dwElementSize;

        //Using middle element of input sequence as pivot node
        dwSize = (swHigh - swLow);
        if (dwSize > 1)
            tmp_byte_offset = (dwSize >> 1) * dwElementSize;
        else
            tmp_byte_offset = 0;

        SwapData(((DWORD) pBase) + byte_offset2, (((DWORD) pBase) + (byte_offset2 + tmp_byte_offset)), dwElementSize);

        /* Pointer 'pBase' may point to an entry of the file list for sorting the partial file list.
         * So, we need this starting entry index 'g_Sorting_Start_Index_in_Companion_Info_List' for processing its companion info list.
         */
    #if (SONY_DCF_ENABLE)  /* for updating the companion DCF info list */
        if ((g_psSystemConfig->dwCurrentOpMode == OP_IMAGE_MODE) && (g_FileList_DCF_Info_List != NULL) && (g_DCF_Obj_TotalCnt > 0))
        {
            idx2_for_companion_info_list = swLow;
            tmp_idx_for_companion_info_list = (tmp_byte_offset / dwElementSize);
            SwapData((DWORD) &g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx2_for_companion_info_list], (DWORD) &g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx2_for_companion_info_list + tmp_idx_for_companion_info_list], sizeof(DCF_INFO_TYPE));
        }
    #else
#if INFO_LIST_ENBALE
        if ((g_psSystemConfig->dwCurrentOpMode == OP_IMAGE_MODE) && (g_FileList_Extra_Info_List != NULL))
        {
            idx2_for_companion_info_list = swLow;
            tmp_idx_for_companion_info_list = (tmp_byte_offset / dwElementSize);
            SwapData((DWORD) &g_FileList_Extra_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx2_for_companion_info_list], (DWORD) &g_FileList_Extra_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx2_for_companion_info_list + tmp_idx_for_companion_info_list], sizeof(EXTRA_FILE_LIST_INFO_TYPE));
        }
#endif
    #endif

        dwStackPoint -= sizeof(ST_STACK_INFO);
        dwStackCount--;

        //New Quick sort algorithm for speeding up		C.W 09/04/09
        i = swLow + 1;
        j = swHigh;

        MpMemCopy((BYTE *) bPivotBuffer, (((BYTE *) pBase) + byte_offset2), dwElementSize);

        while (1)
        {
            /* Pointer 'pBase' may point to an entry of the file list for sorting the partial file list.
             * Index values (i, j and swLow) here are all "relative" (offset) index value to the file entry pointed to by 'pBase'.
             * If the Compare function needs extra companion info list (ex: DCF sorting), it needs reference to the kept starting entry
             * index 'g_Sorting_Start_Index_in_Companion_Info_List'.
             */
            for (; i <= j; i++)
            {
                if (Compare((void *) pBase, i, swLow, reverse_order) == 0)
                    break;
            }

            for (; i <= j; j--)
            {
                if (Compare((void *) pBase, j, swLow, reverse_order))
                    break;
            }

            if (i < j)
            {
                byte_offset1 = i * dwElementSize;
                byte_offset2 = j * dwElementSize;
                SwapData(((DWORD) pBase) + byte_offset1, ((DWORD) pBase) + byte_offset2, dwElementSize);

            #if (SONY_DCF_ENABLE)  /* for updating the companion DCF info list */
                if ((g_psSystemConfig->dwCurrentOpMode == OP_IMAGE_MODE) && (g_FileList_DCF_Info_List != NULL) && (g_DCF_Obj_TotalCnt > 0))
                {
                    idx1_for_companion_info_list = i;
                    idx2_for_companion_info_list = j;
                    SwapData((DWORD) &g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx1_for_companion_info_list], (DWORD) &g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx2_for_companion_info_list], sizeof(DCF_INFO_TYPE));
                }
            #else
#if INFO_LIST_ENBALE
                if ((g_psSystemConfig->dwCurrentOpMode == OP_IMAGE_MODE) && (g_FileList_Extra_Info_List != NULL))
                {
                    idx1_for_companion_info_list = i;
                    idx2_for_companion_info_list = j;
                    SwapData((DWORD) &g_FileList_Extra_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx1_for_companion_info_list], (DWORD) &g_FileList_Extra_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx2_for_companion_info_list], sizeof(EXTRA_FILE_LIST_INFO_TYPE));
                }
#endif
            #endif
            }
            else
            {
                byte_offset1 = swLow * dwElementSize;
                byte_offset2 = j * dwElementSize;
                SwapData(((DWORD) pBase) + byte_offset1, ((DWORD) pBase) + byte_offset2, dwElementSize);

            #if (SONY_DCF_ENABLE)  /* for updating the companion DCF info list */
                if ((g_psSystemConfig->dwCurrentOpMode == OP_IMAGE_MODE) && (g_FileList_DCF_Info_List != NULL) && (g_DCF_Obj_TotalCnt > 0))
                {
                    idx1_for_companion_info_list = swLow;
                    idx2_for_companion_info_list = j;
                    SwapData((DWORD) &g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx1_for_companion_info_list], (DWORD) &g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx2_for_companion_info_list], sizeof(DCF_INFO_TYPE));
                }
            #else
#if INFO_LIST_ENBALE
                if ((g_psSystemConfig->dwCurrentOpMode == OP_IMAGE_MODE) && (g_FileList_Extra_Info_List != NULL))
                {
                    idx1_for_companion_info_list = swLow;
                    idx2_for_companion_info_list = j;
                    SwapData((DWORD) &g_FileList_Extra_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx1_for_companion_info_list], (DWORD) &g_FileList_Extra_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + idx2_for_companion_info_list], sizeof(EXTRA_FILE_LIST_INFO_TYPE));
                }
#endif
            #endif

                break;
            }
        }

        //put stack
        sStackInfo = (ST_STACK_INFO *) ((DWORD) bStackBuffer + dwStackPoint);
        sStackInfo->swLow = (j + 1);
        sStackInfo->swHigh = swHigh;
        dwStackPoint += sizeof(ST_STACK_INFO);
        dwStackCount++;

        sStackInfo = (ST_STACK_INFO *) ((DWORD) bStackBuffer + dwStackPoint);
        sStackInfo->swLow = swLow;
        sStackInfo->swHigh = (j - 1);
        dwStackPoint += sizeof(ST_STACK_INFO);
        dwStackCount++;

        if (dwStackCount >= (STACK_SIZE >> 2))
        {
            MP_ALERT("%s: dwStackCount (%u) >= (STACK_SIZE >> 2) (%u) => return ABNORMAL_STATUS;", __FUNCTION__, dwStackCount, (STACK_SIZE >> 2));
            ext_mem_free(bStackBuffer);
            return ABNORMAL_STATUS;
        }
    }

    /* if count of elements to be sorted is very big, show a console message before and after the sorting */
    if (dwElementCount > 1000)
    {
        MP_ALERT("%s: file list sorting finished.", __FUNCTION__);
    }

    ext_mem_free(bStackBuffer);
    return FS_SUCCEED;
}



/* function to compare filenames in ST_SEARCH_INFO structures for name ordering by filename */
static WORD CompareFileName(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order)
{
    ST_SEARCH_INFO *pSearchInfo1, *pSearchInfo2;
    int i;


    if (pListBase == NULL)
    {
        MP_ALERT("%s: NULL pointer !", __FUNCTION__);
        return 0;  /* just return, not very care return value in this case */
    }

    pSearchInfo1 = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) pListBase) + element_index1);
    pSearchInfo2 = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) pListBase) + element_index2);

    if ((pSearchInfo1 == NULL) || (pSearchInfo2 == NULL))
    {
        MP_ALERT("%s: NULL pointer element !", __FUNCTION__);
        return ((pSearchInfo2 == NULL)? 1:0);  /* just return, not very care return value in this case */
    }

    /* special processing for ".." folder node, always put ".." item in the beginning of list */
    if ( (pSearchInfo1->bParameter == SEARCH_INFO_CHANGE_PATH) && (pSearchInfo1->bFileType == FILE_OP_TYPE_FOLDER) )
        return 1;

    if ( (pSearchInfo2->bParameter == SEARCH_INFO_CHANGE_PATH) && (pSearchInfo2->bFileType == FILE_OP_TYPE_FOLDER) )
        return 0;

    //fix for folder sorting
    if ((pSearchInfo1->bFileType == FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bFileType != FILE_OP_TYPE_FOLDER))
        return (reverse_order? 0:1);
    if ((pSearchInfo1->bFileType != FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bFileType == FILE_OP_TYPE_FOLDER))
        return (reverse_order? 1:0);

    if ((pSearchInfo1->FsType != FS_TYPE_exFAT) && (pSearchInfo2->FsType != FS_TYPE_exFAT))  /* files not on an exFAT drive */
    {
        BYTE *pName1, *pName2, tmp_ch1, tmp_ch2;

        pName1 = &(pSearchInfo1->bName[0]);
        pName2 = &(pSearchInfo2->bName[0]);
        for (i=0; i < 8; i++)
        {
            tmp_ch1 = toupper(pName1[i]);
            tmp_ch2 = toupper(pName2[i]);

            if (tmp_ch1 < tmp_ch2)
                return (reverse_order? 0:1);
            else if (tmp_ch1 > tmp_ch2)
                return (reverse_order? 1:0);
        }

        pName1 = &(pSearchInfo1->bExt[0]);
        pName2 = &(pSearchInfo2->bExt[0]);
        for (i=0; i < 3; i++)
        {
            tmp_ch1 = toupper(pName1[i]);
            tmp_ch2 = toupper(pName2[i]);

            if (tmp_ch1 < tmp_ch2)
                return (reverse_order? 0:1);
            else if (tmp_ch1 > tmp_ch2)
                return (reverse_order? 1:0);
        }
    }
#if EXFAT_ENABLE
    else  /* files on an exFAT drive */
    {
        DWORD name_len1, name_len2;
        WORD tmp_wCh1, tmp_wCh2;

        if ((pSearchInfo1->utf16_name == NULL) || (pSearchInfo2->utf16_name == NULL))
        {
            MP_ALERT("%s: NULL pointer of filename !", __FUNCTION__);
            return ((pSearchInfo2->utf16_name == NULL)? 1:0);  /* just return, not very care return value in this case */
        }

        name_len1 = StringLength16(pSearchInfo1->utf16_name);
        name_len2 = StringLength16(pSearchInfo2->utf16_name);

        for (i = 0; i < name_len1; i++)
        {
            if (i > name_len2) /* reach end of filename of file_2 */
                return (reverse_order? 1:0);

            /* ignore char case difference, and not to modify original string */
            tmp_wCh1 = (WORD) pSearchInfo1->utf16_name[i];
            tmp_wCh2 = (WORD) pSearchInfo2->utf16_name[i];
            CharUpperCase16(&tmp_wCh1);
            CharUpperCase16(&tmp_wCh2);

            if (tmp_wCh1 < tmp_wCh2)
                return (reverse_order? 0:1);
            else if (tmp_wCh1 > tmp_wCh2)
                return (reverse_order? 1:0);
        }

        if (i <= name_len2) /* reach end of filename of file_1 */
            return (reverse_order? 0:1);
    }
#endif

    return 0;
}



/* function to compare file sizes in ST_SEARCH_INFO structures for file ordering by file size */
static WORD CompareFileSize(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order)
{
    ST_SEARCH_INFO *pSearchInfo1, *pSearchInfo2;


    if (pListBase == NULL)
    {
        MP_ALERT("%s: NULL pointer !", __FUNCTION__);
        return 0;  /* just return, not very care return value in this case */
    }

    pSearchInfo1 = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) pListBase) + element_index1);
    pSearchInfo2 = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) pListBase) + element_index2);

    if ((pSearchInfo1 == NULL) || (pSearchInfo2 == NULL))
    {
        MP_ALERT("%s: NULL pointer element !", __FUNCTION__);
        return ((pSearchInfo2 == NULL)? 1:0);  /* just return, not very care return value in this case */
    }

    /* special processing for ".." folder node, always put ".." item in the beginning of list */
    if ((pSearchInfo1->bFileType == FILE_OP_TYPE_FOLDER) && (pSearchInfo1->bName[0] == '.') && (pSearchInfo1->bName[1] == '.')
         && (pSearchInfo1->bName[2] == ' '))
        return 1;
    if ((pSearchInfo2->bFileType == FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bName[0] == '.') && (pSearchInfo2->bName[1] == '.')
         && (pSearchInfo2->bName[2] == ' '))
        return 0;

    //fix for folder sorting
    if (pSearchInfo1->bFileType == FILE_OP_TYPE_FOLDER)
        pSearchInfo1->dwFileSize = 0;

    if (pSearchInfo2->bFileType == FILE_OP_TYPE_FOLDER)
        pSearchInfo2->dwFileSize = 0;

    DWORD value1, value2;
    value1 = pSearchInfo1->dwFileSize;
    value2 = pSearchInfo2->dwFileSize;

    if (value1 < value2)
        return (reverse_order? 0:1);
    else
        return (reverse_order? 1:0);
}



/* function to compare file date/time in ST_SEARCH_INFO structures for file ordering by file date time */
static WORD CompareFileDateTime(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order)
{
    ST_SEARCH_INFO *pSearchInfo1, *pSearchInfo2;


    if (pListBase == NULL)
    {
        MP_ALERT("%s: NULL pointer !", __FUNCTION__);
        return 0;  /* just return, not very care return value in this case */
    }

    pSearchInfo1 = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) pListBase) + element_index1);
    pSearchInfo2 = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) pListBase) + element_index2);

    if ((pSearchInfo1 == NULL) || (pSearchInfo2 == NULL))
    {
        MP_ALERT("%s: NULL pointer element !", __FUNCTION__);
        return ((pSearchInfo2 == NULL)? 1:0);  /* just return, not very care return value in this case */
    }

    /* special processing for ".." folder node, always put ".." item in the beginning of list */
    if ((pSearchInfo1->bFileType == FILE_OP_TYPE_FOLDER) && (pSearchInfo1->bName[0] == '.') && (pSearchInfo1->bName[1] == '.')
         && (pSearchInfo1->bName[2] == ' '))
        return 1;
    if ((pSearchInfo2->bFileType == FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bName[0] == '.') && (pSearchInfo2->bName[1] == '.')
         && (pSearchInfo2->bName[2] == ' '))
    return 0;

  /* Note: how about (folder vs file) and (folder vs folder) sorting in such sorting method ??
   *
   * If we want to put folders together like PC Windows, we need to enable this block.

   * But, for file sorting by [Date + Calendar] requirement, we must disable this block. Otherwise, files and folders with same date
   * will not located in contiguous entries in the file list !
   */
  #if 0
    //fix for folder sorting
    if ((pSearchInfo1->bFileType == FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bFileType != FILE_OP_TYPE_FOLDER))
        return (reverse_order? 0:1);
    if ((pSearchInfo1->bFileType != FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bFileType == FILE_OP_TYPE_FOLDER))
        return (reverse_order? 1:0);
  #endif

    if (pSearchInfo1->DateTime.year < pSearchInfo2->DateTime.year)
        return (reverse_order? 0:1);
    else if (pSearchInfo1->DateTime.year > pSearchInfo2->DateTime.year)
        return (reverse_order? 1:0);
    else //same year
    {
        if (pSearchInfo1->DateTime.month < pSearchInfo2->DateTime.month)
            return (reverse_order? 0:1);
        else if (pSearchInfo1->DateTime.month > pSearchInfo2->DateTime.month)
            return (reverse_order? 1:0);
        else //same month
        {
            if (pSearchInfo1->DateTime.day < pSearchInfo2->DateTime.day)
                return (reverse_order? 0:1);
            else if (pSearchInfo1->DateTime.day > pSearchInfo2->DateTime.day)
                return (reverse_order? 1:0);
            else //same day
            {
                MP_DEBUG("%s: To-Do: when to add add UTC time offset ??", __FUNCTION__);
                //To-Do: when to add add UTC time offset ??

                if (pSearchInfo1->DateTime.hour < pSearchInfo2->DateTime.hour)
                    return (reverse_order? 0:1);
                else if (pSearchInfo1->DateTime.hour > pSearchInfo2->DateTime.hour)
                    return (reverse_order? 1:0);
                else //same hour
                {
                    if (pSearchInfo1->DateTime.minute < pSearchInfo2->DateTime.minute)
                        return (reverse_order? 0:1);
                    else if (pSearchInfo1->DateTime.minute > pSearchInfo2->DateTime.minute)
                        return (reverse_order? 1:0);
                    else //same minute
                    {
                        if (pSearchInfo1->DateTime.second <= pSearchInfo2->DateTime.second) //less or equal to
                            return (reverse_order? 0:1);
                        else if (pSearchInfo1->DateTime.second > pSearchInfo2->DateTime.second)
                            return (reverse_order? 1:0);
                    }
                }
            }
        }
    }
}



/* function to compare image file EXIF date/time corresponding to the ST_SEARCH_INFO structures for file ordering by EXIF date time */
static WORD CompareImgEXIFDateTime(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order)
{
    ST_SEARCH_INFO *pSearchInfo1, *pSearchInfo2;
    DATE_TIME_INFO_TYPE  exif_date_time_1, exif_date_time_2;
    DATE_TIME_INFO_TYPE  *exif_date_time_ptr1, *exif_date_time_ptr2;
    BOOL f_have_Exif_1 = FALSE, f_have_Exif_2 = FALSE;


    if (pListBase == NULL)
    {
        MP_ALERT("%s: NULL pointer !", __FUNCTION__);
        return 0;  /* just return, not very care return value in this case */
    }

    pSearchInfo1 = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) pListBase) + element_index1);
    pSearchInfo2 = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) pListBase) + element_index2);

    if ((pSearchInfo1 == NULL) || (pSearchInfo2 == NULL))
    {
        MP_ALERT("%s: NULL pointer element !", __FUNCTION__);
        return ((pSearchInfo2 == NULL)? 1:0);  /* just return, not very care return value in this case */
    }

    /* special processing for ".." folder node, always put ".." item in the beginning of list */
    if ((pSearchInfo1->bFileType == FILE_OP_TYPE_FOLDER) && (pSearchInfo1->bName[0] == '.') && (pSearchInfo1->bName[1] == '.')
         && (pSearchInfo1->bName[2] == ' '))
        return 1;
    if ((pSearchInfo2->bFileType == FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bName[0] == '.') && (pSearchInfo2->bName[1] == '.')
         && (pSearchInfo2->bName[2] == ' '))
        return 0;

  /* Note: how about (folder vs file) and (folder vs folder) sorting in such sorting method ??
   *
   * For file sorting by [Date + Calendar] requirement, we must disable this block. Otherwise, files and folders with same date
   * will not located in contiguous entries in the file list !
   *
   * But here, for file sorting by EXIF date time, we want to put folders together like PC Windows. So, we need to enable this block.
   */
  #if 1
    //fix for folder sorting
    if ((pSearchInfo1->bFileType == FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bFileType != FILE_OP_TYPE_FOLDER))
        return (reverse_order? 0:1);
    if ((pSearchInfo1->bFileType != FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bFileType == FILE_OP_TYPE_FOLDER))
        return (reverse_order? 1:0);
  #endif

    exif_date_time_ptr1 = &exif_date_time_1;
    exif_date_time_ptr2 = &exif_date_time_2;

#if (SONY_DCF_ENABLE)
    if (g_FileList_DCF_Info_List != NULL)
    {
        /* Pointer 'pListBase' may point to an entry of the file list for sorting the partial file list.
         * So, we need the kept starting entry index 'g_Sorting_Start_Index_in_Companion_Info_List' for processing its companion info list.
         */
        if (g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index1].Flags.f_have_EXIF_time_info)
        {
            exif_date_time_ptr1 = &(g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index1].exif_date_time);
            f_have_Exif_1 = TRUE;
        }
        if (g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index2].Flags.f_have_EXIF_time_info)
        {
            exif_date_time_ptr2 = &(g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index2].exif_date_time);
            f_have_Exif_2 = TRUE;
        }
    }
#else
#if INFO_LIST_ENBALE
    if (g_FileList_Extra_Info_List != NULL)
    {
        /* Pointer 'pListBase' may point to an entry of the file list for sorting the partial file list.
         * So, we need the kept starting entry index 'g_Sorting_Start_Index_in_Companion_Info_List' for processing its companion info list.
         */
        f_have_Exif_1 = g_FileList_Extra_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index1].Flags.f_have_EXIF_time_info;
        f_have_Exif_2 = g_FileList_Extra_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index2].Flags.f_have_EXIF_time_info;
        if (f_have_Exif_1)
            MpMemCopy(&exif_date_time_1, (BYTE *) &g_FileList_Extra_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index1].exif_date_time, sizeof(DATE_TIME_INFO_TYPE));
        if (f_have_Exif_2)
            MpMemCopy(&exif_date_time_2, (BYTE *) &g_FileList_Extra_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index2].exif_date_time, sizeof(DATE_TIME_INFO_TYPE));
    }
#endif
#endif

L_start_compare_EXIF_time:
    /* note: SONY spec ask to always put (files with EXIF) before (files without EXIF) for sorting by EXIF sorting methods */
    if ((f_have_Exif_1) && (! f_have_Exif_2))  /* file_1 has EXIF date time info, but file_2 does not have */
    {
      #if (SONY_DCF_ENABLE)
        return 1;
      #else
        /* we always put (files with EXIF) before (files without EXIF) when sorting by EXIF date time */
        return 1;
      #endif
    }
    else if ((! f_have_Exif_1) && (f_have_Exif_2))  /* file_1 does not have EXIF date time info, but file_2 has */
    {
      #if (SONY_DCF_ENABLE)
        return 0;
      #else
        /* we always put (files with EXIF) before (files without EXIF) when sorting by EXIF date time */
        return 0;
      #endif
    }
    else if ((f_have_Exif_1) && (f_have_Exif_2))  /* both files have EXIF date time info */
    {
        if (exif_date_time_ptr1->year < exif_date_time_ptr2->year)
            return (reverse_order? 0:1);
        else if (exif_date_time_ptr1->year > exif_date_time_ptr2->year)
            return (reverse_order? 1:0);
        else //same year
        {
            if (exif_date_time_ptr1->month < exif_date_time_ptr2->month)
                return (reverse_order? 0:1);
            else if (exif_date_time_ptr1->month > exif_date_time_ptr2->month)
                return (reverse_order? 1:0);
            else //same month
            {
                if (exif_date_time_ptr1->day < exif_date_time_ptr2->day)
                    return (reverse_order? 0:1);
                else if (exif_date_time_ptr1->day > exif_date_time_ptr2->day)
                    return (reverse_order? 1:0);
                else //same day
                {
                    MP_DEBUG("%s: To-Do: when to add add UTC time offset ??", __FUNCTION__);
                    //To-Do: when to add add UTC time offset ??

                    if (exif_date_time_ptr1->hour < exif_date_time_ptr2->hour)
                        return (reverse_order? 0:1);
                    else if (exif_date_time_ptr1->hour > exif_date_time_ptr2->hour)
                        return (reverse_order? 1:0);
                    else //same hour
                    {
                        if (exif_date_time_ptr1->minute < exif_date_time_ptr2->minute)
                            return (reverse_order? 0:1);
                        else if (exif_date_time_ptr1->minute > exif_date_time_ptr2->minute)
                            return (reverse_order? 1:0);
                        else //same minute
                        {
                            if (exif_date_time_ptr1->second <= exif_date_time_ptr2->second) //less or equal to
                                return (reverse_order? 0:1);
                            else if (exif_date_time_ptr1->second > exif_date_time_ptr2->second)
                                return (reverse_order? 1:0);
                        }
                    }
                }
            }
        }
    }

    /* here, both files have no EXIF date time info => files should be sorted by another sorting criterion if really desired */
#if (SONY_DCF_ENABLE)
    /* for SONY DCF, sort by filename alphabets if both files have no EXIF date time info */
    return CompareFileName(pListBase, element_index1, element_index2, reverse_order);
#else
    //return CompareFileName(pListBase, element_index1, element_index2, reverse_order);  /* compare by file name */
    //return 0;  /* just return, not very care return value in this case */
    return CompareFileDateTime(pListBase, element_index1, element_index2, reverse_order);  /* compare by file date/time */
#endif
}



#if (SONY_DCF_ENABLE)
/* function to compare SONY DCF displayed filenames corresponding to the ST_SEARCH_INFO structures for file ordering by DCF displayed filename */
static WORD CompareDCFFileDisplayName(void *pListBase, DWORD element_index1, DWORD element_index2, BOOL reverse_order)
{
    ST_SEARCH_INFO *pSearchInfo1, *pSearchInfo2;
    int i;


    if (pListBase == NULL)
    {
        MP_ALERT("%s: NULL pointer !", __FUNCTION__);
        return 0;  /* just return, not very care return value in this case */
    }

    pSearchInfo1 = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) pListBase) + element_index1);
    pSearchInfo2 = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) pListBase) + element_index2);

    if ((pSearchInfo1 == NULL) || (pSearchInfo2 == NULL))
    {
        MP_ALERT("%s: NULL pointer !", __FUNCTION__);
        return ((pSearchInfo2 == NULL)? 1:0);  /* just return, not very care return value in this case */
    }

    /* special processing for ".." folder node, always put ".." item in the beginning of list */
    if ( (pSearchInfo1->bParameter == SEARCH_INFO_CHANGE_PATH) && (pSearchInfo1->bFileType == FILE_OP_TYPE_FOLDER) )
        return 1;

    if ( (pSearchInfo2->bParameter == SEARCH_INFO_CHANGE_PATH) && (pSearchInfo2->bFileType == FILE_OP_TYPE_FOLDER) )
        return 0;

    //for folder sorting
    if ((pSearchInfo1->bFileType == FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bFileType != FILE_OP_TYPE_FOLDER))
        return (reverse_order? 0:1);
    if ((pSearchInfo1->bFileType != FILE_OP_TYPE_FOLDER) && (pSearchInfo2->bFileType == FILE_OP_TYPE_FOLDER))
        return (reverse_order? 1:0);

    if ((g_FileList_DCF_Info_List != NULL) && g_DCF_Obj_TotalCnt)
    {
        DWORD DCF_display_integer_1, DCF_display_integer_2;
        BOOL  f_Is_DCF_obj_1 = FALSE, f_Is_DCF_obj_2 = FALSE;

        /* Pointer 'pListBase' may point to an entry of the file list for sorting the partial file list.
         * So, we need the kept starting entry index 'g_Sorting_Start_Index_in_Companion_Info_List' for processing its companion info list.
         */
        if (g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index1].Flags.f_maybe_DCF_Obj)
        {
            f_Is_DCF_obj_1 = TRUE;
            DCF_display_integer_1 = g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index1].merged_display_integer;
        }
        if (g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index2].Flags.f_maybe_DCF_Obj)
        {
            f_Is_DCF_obj_2 = TRUE;
            DCF_display_integer_2 = g_FileList_DCF_Info_List[g_Sorting_Start_Index_in_Companion_Info_List + element_index2].merged_display_integer;
        }

        if ((f_Is_DCF_obj_1) && (! f_Is_DCF_obj_2))  /* file_1 is a DCF object, but file_2 is not */
            return (reverse_order? 0:1);
        else if ((! f_Is_DCF_obj_1) && (f_Is_DCF_obj_2))  /* file_1 is not a DCF object, but file_2 is */
            return (reverse_order? 1:0);
        else if ((f_Is_DCF_obj_1) && (f_Is_DCF_obj_2))  /* both files are DCF objects */
        {
            if (DCF_display_integer_1 < DCF_display_integer_2)
                return (reverse_order? 0:1);
            else if (DCF_display_integer_1 > DCF_display_integer_2)
                return (reverse_order? 1:0);
        }

        /* here, either both files are not DCF objects, or both files are DCF objects and with same DCF displayed number.
         *  => further compare by filename alphabets.
         */
        return CompareFileName(pListBase, element_index1, element_index2, reverse_order);
    }
    else
    {
        /* no any DCF object => compare by filename alphabets */
        return CompareFileName(pListBase, element_index1, element_index2, reverse_order);
    }
}
#endif



///
///@ingroup FILE
///@brief   Check whether if there are enough free clusters on the specified drive for the request size for file usage.
///
///@param   stDrv         The drive to check available free space. \n\n
///@param   dwSize        Byte size that may be requested to use later.
///
///@retval  FS_SUCCEED        There are enough free clusters on the drive. \n\n
///@retval  DISK_FULL         No enough free clusters on the drive. \n\n
///@retval  INVALID_DRIVE     Invalid drive.
///
SWORD CheckSpaceIfEnough(DRIVE * stDrv, DWORD dwSize)
{
    DWORD dwCluster;


    if (stDrv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    dwCluster = (dwSize >> (stDrv->ClusterExp + stDrv->bSectorExp));
    if (dwSize & ((1 << (stDrv->ClusterExp + stDrv->bSectorExp)) - 1))
        dwCluster++;

    if (dwCluster > DriveFreeClustersCountGet(stDrv))
    {
        MP_ALERT("%s: (needed size = %lu), (needed clusters = %lu) > (free clusters = %lu) => DISK FULL !", __FUNCTION__, dwSize, dwCluster, DriveFreeClustersCountGet(stDrv));
        return DISK_FULL;
    }
    return FS_SUCCEED;
}



static SWORD SweepCurrent(SWORD * pswRet, DRIVE * stTargetDrive, DRIVE * stSourceDrive)
{
    DWORD i;
    SWORD ret = FS_SUCCEED;
    FDB *SCnode;
    DWORD bufByteCount = 0;
    ENTRY_FILENAME_TYPE  bFilenameType = E_FILENAME_UNKNOWN;
    WORD  *wTmpFCLName;
    BOOL  f_Utf16_LongName;
    WORD  wTmp_LongName_Buf[MAX_L_NAME_LENG];
    DWORD dwLNameChged;


    if ((stTargetDrive == NULL) || (stSourceDrive == NULL))
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (pswRet == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

  #if EXFAT_ENABLE
    if (stTargetDrive->Flag.FsType == FS_TYPE_exFAT)
    {
        mpDebugPrint("%s: To-Do: how to process if target drive is an exFAT drive ??", __FUNCTION__);
        return ABNORMAL_STATUS; //not supported yet
    }
  #endif

    wTmpFCLName = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
    if (wTmpFCLName == NULL)
    {
        MP_ALERT("%s: -E malloc fail", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
    MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);

    if (GetFilenameOfCurrentNode((DRIVE *) stSourceDrive, &bFilenameType, (BYTE *) wTmpFCLName, &bufByteCount, 512) != FS_SUCCEED)
    {
        MP_ALERT("%s: GetFilenameOfCurrentNode() failed !", __FUNCTION__);
        ext_mem_free(wTmpFCLName);
        return ABNORMAL_STATUS;
    }

    if (bFilenameType == E_FILENAME_8_3_SHORT_NAME)
        f_Utf16_LongName = 0;
    else if (bFilenameType == E_FILENAME_UTF16_LONG_NAME)
    {
        f_Utf16_LongName = 1;
        MpMemSet((BYTE *) wTmp_LongName_Buf, 0, MAX_L_NAME_LENG * 2);
        MpMemCopy((BYTE *) wTmp_LongName_Buf, (BYTE *) wTmpFCLName, bufByteCount);
    }

    SCnode = (FDB *) stSourceDrive->Node;

    if (stSourceDrive->Flag.FsType != FS_TYPE_exFAT)
    {
        if (SCnode->Name[0] == '.')
        {
            // skip '.' and '..' entry
        }
        else if (SCnode->Name[0] == 0xe5)
        {
            // skip deleted entry
        }
        else if (SCnode->Attribute == 0x0f)
        {
            // skip the long file name entry
        }
        else if (SCnode->Attribute & FDB_LABEL)
        {
            // skip disk label entry
        }
        else if (SCnode->Attribute & FDB_SUB_DIR)	// copy sub-directory entry
        {
            MP_ALERT("%s: copy sub-directory entry...", __FUNCTION__);

            //check free space, reserved 4 clusters for build subdir
            if (CheckSpaceIfEnough(stTargetDrive, 4 * (stTargetDrive->ClusterSize << stTargetDrive->bSectorExp)) == DISK_FULL)
            {
                MP_ALERT("%s: free disk space not enough !!", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return DISK_FULL;
            }

            if (f_Utf16_LongName)
            {
                MpMemSet((BYTE *) stSourceDrive->LongName, 0, MAX_L_NAME_LENG * 2);
                MpMemCopy((BYTE *) stSourceDrive->LongName, (BYTE *) wTmp_LongName_Buf, bufByteCount);

                dwLNameChged = 0;
                MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);
                if (BuildNewLName(stTargetDrive, stSourceDrive, wTmpFCLName, &dwLNameChged) != FS_SUCCEED)
                {
                    MP_ALERT("%s: BuildNewLName() failed !", __FUNCTION__);
                    ext_mem_free(wTmpFCLName);
                    return ABNORMAL_STATUS;
                }

                MP_ALERT("%s: => MakeDir_UTF16()...", __FUNCTION__);
                *pswRet = MakeDir_UTF16(stTargetDrive, wTmpFCLName);
            }
            else /* 8.3 short filename */
            {
                BYTE pri_name[9], ext_name[4];
                MpMemSet(pri_name, 0, 9);
                MpMemSet(ext_name, 0, 4);
                for (i = 0; i < 8; i++)
                {
                    if (stSourceDrive->Node->Name[i] != ' ')
                        pri_name[i] = stSourceDrive->Node->Name[i];
                    else
                        break;
                }
                for (i = 0; i < 3; i++)
                {
                    if (stSourceDrive->Node->Extension[i] != ' ')
                        ext_name[i] = stSourceDrive->Node->Extension[i];
                    else
                        break;
                }

                if (StringLength08(ext_name) > 0)
                {
                    MP_ALERT("%s: => MakeDir(\"%s.%s\")...", __FUNCTION__, pri_name, ext_name);
                    *pswRet = MakeDir(stTargetDrive, pri_name, ext_name);
                }
                else
                {
                    MP_ALERT("%s: => MakeDir(\"%s\")...", __FUNCTION__, pri_name);
                    *pswRet = MakeDir(stTargetDrive, pri_name, "");
                }
            }

            if (*pswRet == FS_SUCCEED)
            {
                if (DriveRefresh(stTargetDrive) != FS_SUCCEED)
                {
                    ext_mem_free(wTmpFCLName);
                    return ABNORMAL_STATUS;
                }

                MP_DEBUG("%s: => CdSub() to sub-dir...", __FUNCTION__);
                if ((ret = CdSub(stTargetDrive)) != FS_SUCCEED)
                {
                    ext_mem_free(wTmpFCLName);
                    return ret;
                }
                if ((ret = CdSub(stSourceDrive)) != FS_SUCCEED)
                {
                    ext_mem_free(wTmpFCLName);
                    return ret;
                }
            }
        }
        else
        {
            ext_mem_free(wTmpFCLName);
            return FS_SUCCEED;
        }
    }
  #if EXFAT_ENABLE
    else
    {
        DIR_ENTRY_INFO_TYPE  dir_entry_info;

        MpMemSet((BYTE *) &dir_entry_info, 0, sizeof(DIR_ENTRY_INFO_TYPE));
        if (GetFileAttributeAndTimeInfoFromFDB(stSourceDrive, (FDB *) stSourceDrive->Node, &dir_entry_info) != PASS)
        {
            MP_ALERT("%s: Error! GetFileAttributeAndTimeInfoFromFDB() failed !", __FUNCTION__);
            ext_mem_free(wTmpFCLName);
            return ABNORMAL_STATUS;
        }

        if (dir_entry_info.fileAttribute.f_SubDir)	// copy sub-directory entry
        {
            MP_ALERT("%s: copy sub-directory entry...", __FUNCTION__);

            //check free space, reserved 4 clusters for build subdir
            if (CheckSpaceIfEnough(stTargetDrive, 4 * (stTargetDrive->ClusterSize << stTargetDrive->bSectorExp)) == DISK_FULL)
            {
                MP_ALERT("%s: free disk space not enough !!", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return DISK_FULL;
            }

            MpMemSet((BYTE *) stSourceDrive->LongName, 0, MAX_L_NAME_LENG * 2);
            MpMemCopy((BYTE *) stSourceDrive->LongName, (BYTE *) wTmp_LongName_Buf, bufByteCount);

            dwLNameChged = 0;
            MpMemSet((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);
            if (BuildNewLName(stTargetDrive, stSourceDrive, wTmpFCLName, &dwLNameChged) != FS_SUCCEED)
            {
                MP_ALERT("%s: BuildNewLName() failed !", __FUNCTION__);
                ext_mem_free(wTmpFCLName);
                return ABNORMAL_STATUS;
            }

            MP_ALERT("%s: => MakeDir_UTF16()...", __FUNCTION__);
            *pswRet = MakeDir_UTF16(stTargetDrive, wTmpFCLName);

            ext_mem_free(wTmpFCLName);

            if (*pswRet == FS_SUCCEED)
            {
                if (DriveRefresh(stTargetDrive) != FS_SUCCEED)
                {
                    ext_mem_free(wTmpFCLName);
                    return ABNORMAL_STATUS;
                }

                MP_DEBUG("%s: => CdSub() to sub-dir...", __FUNCTION__);
                if ((ret = CdSub(stTargetDrive)) != FS_SUCCEED)
                {
                    ext_mem_free(wTmpFCLName);
                    return ret;
                }
                if ((ret = CdSub(stSourceDrive)) != FS_SUCCEED)
                {
                    ext_mem_free(wTmpFCLName);
                    return ret;
                }
            }
        }
        else
        {
            ext_mem_free(wTmpFCLName);
            return FS_SUCCEED;
        }
    }
  #endif

    ext_mem_free(wTmpFCLName);
    return END_OF_DIR;
}



static SWORD SweepForward(DRIVE * stTargetDrive, DRIVE * stSourceDrive, WORD wStartStackPoint)
{
    SWORD swRet;

    if ((stTargetDrive == NULL) || (stSourceDrive == NULL))
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (stSourceDrive->Flag.FsType != FS_TYPE_exFAT)
    {
        do
        {
            swRet = NextNode(stSourceDrive);
        } while (ScanFileName(stSourceDrive) == FILENAME_SCAN_CONTINUE);
    }
  #if EXFAT_ENABLE
    else
        swRet = DirNext(stSourceDrive);
  #endif

    while ((swRet == END_OF_DIR) && (stSourceDrive->DirStackPoint > wStartStackPoint))
    {
        MP_DEBUG("%s: => CdParent() to parent dir...", __FUNCTION__);
        CdParent(stSourceDrive);
        CdParent(stTargetDrive);

        if (stSourceDrive->Flag.FsType != FS_TYPE_exFAT)
        {
            do
            {
                swRet = NextNode(stSourceDrive);
            } while (ScanFileName(stSourceDrive) == FILENAME_SCAN_CONTINUE);
        }
  #if EXFAT_ENABLE
        else
            swRet = DirNext(stSourceDrive);
  #endif
    }

    return swRet;
}



// copy from the current node to the end of the directory
static SWORD SweepFiling(DRIVE * stTargetDrive, DRIVE * stSourceDrive, WORD wStartStackPoint)
{
    SWORD swRet;


    if ((stTargetDrive == NULL) || (stSourceDrive == NULL))
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    swRet = FS_SUCCEED;
    while (swRet == FS_SUCCEED)
    {
        swRet = SweepForward(stTargetDrive, stSourceDrive, wStartStackPoint);
        if (swRet != FS_SUCCEED)
        {
            break;
        }

        if (SweepCurrent(&swRet, stTargetDrive, stSourceDrive) == FS_SUCCEED)
        {
            return FS_SUCCEED;
        }
    }

    return swRet;
}



#if 1 //<< File I/O API test >>
/* --- private test code for File I/O API test, not for public usage --- [begin] */

STREAM *Test_OpenFileByNameForRead(BYTE drv_index, const char * filename, const char * ext_name, int * file_size)
{
    STREAM *handle = NULL;
    DRIVE *drv;
    BYTE NameL, ExtenL;
    int  ret;


    if (DriveIndex2PhyDevID(drv_index) == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, drv_index);
        return NULL;
    }

    if ((filename == NULL) || (strcmp(filename, "") == 0))
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return NULL;
    }

    if (file_size == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return NULL;
    }

    NameL = StringLength08((BYTE *) filename);
    ExtenL = StringLength08((BYTE *) ext_name);

    if (ExtenL)
    {
        MP_DEBUG("enter %s(%s, %s)...", __FUNCTION__, filename, ext_name);
    }
    else
    {
        MP_DEBUG("enter %s(%s, NULL)...", __FUNCTION__, filename);
    }

    if (!SystemCardPresentCheck(drv_index))
    {
        DriveAdd(drv_index);
    }
    drv = DriveGet(drv_index);

    if ((NameL <= 8) && (ExtenL <= 3)) /* 8.3 short filename */
        ret = FileSearch(drv, (BYTE *) filename, (BYTE *) ext_name, E_FILE_TYPE);
    else
    {
        BYTE utf8_filename[MAX_L_NAME_LENG];
        WORD utf16_filename[MAX_L_NAME_LENG];
        int total_len = (ExtenL > 0)? (NameL + ExtenL + 1):(NameL);

        MpMemSet(utf8_filename, 0, MAX_L_NAME_LENG);
        MpMemSet((BYTE *) utf16_filename, 0, MAX_L_NAME_LENG * sizeof(WORD));

        strcpy(utf8_filename, filename);
        if (ExtenL)
        {
            strcat(utf8_filename, ".");
            strcat(utf8_filename, ext_name);
        }

        mpx_UtilUtf8ToUnicodeU16(utf16_filename, utf8_filename); /* convert to UTF-16 string */
        ret = FileSearchLN(drv, utf16_filename, total_len, E_FILE_TYPE);
    }

    if (ret != FS_SUCCEED)
    {
        MP_ALERT("%s: File search failed, file not found !", __FUNCTION__);
        return NULL;
    }
    else
    {
        MP_DEBUG("%s: File search OK, file found !", __FUNCTION__);
    }

    handle = FileOpen(drv);
    if (handle == NULL)
    {
        MP_ALERT("%s: FileOpen() failed !", __FUNCTION__);
        return NULL;
    }
    else
    {
        MP_DEBUG("%s: FileOpen() OK !", __FUNCTION__);
    }

    *file_size = FileSizeGet(handle);

    return handle;
}



int Test_CreateAndWriteFileByName(BYTE drv_index, const char * filename, const char * ext_name)
{
    STREAM *handle = NULL;
    DRIVE *drv;
    BYTE NameL, ExtenL;
    char *test_str = "String to be written";
    int  write_len;


    if (DriveIndex2PhyDevID(drv_index) == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, drv_index);
        return 0;
    }

    if ((filename == NULL) || (strcmp(filename, "") == 0))
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return 0;
    }

    NameL = StringLength08((BYTE *) filename);
    ExtenL = StringLength08((BYTE *) ext_name);

    if (ExtenL)
    {
        MP_DEBUG("enter %s(%s, %s)...", __FUNCTION__, filename, ext_name);
    }
    else
    {
        MP_DEBUG("enter %s(%s, NULL)...", __FUNCTION__, filename);
    }

    if (!SystemCardPresentCheck(drv_index))
    {
        DriveAdd(drv_index);
    }
    drv = DriveGet(drv_index);

    /* CreateFile() supports both DOS 8.3 format and long filename format */
    if (CreateFile(drv, (BYTE *) filename, (BYTE *) ext_name) != FS_SUCCEED)
    {
        MP_ALERT("%s: CreateFile() failed, file not created !", __FUNCTION__);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: CreateFile() OK, file created !", __FUNCTION__);
    }

    handle = FileOpen(drv);
    if (handle == NULL)
    {
        MP_ALERT("%s: FileOpen() failed !", __FUNCTION__);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: FileOpen() OK !", __FUNCTION__);
    }

    write_len = FileWrite(handle, test_str, StringLength08(test_str));
    if (write_len != StringLength08(test_str))
    {
        MP_ALERT("%s: FileWrite() failed !", __FUNCTION__);

        FileClose(handle);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: FileWrite() OK !", __FUNCTION__);
    }

    FileClose(handle);
    return 1;
}



/* UTF-16 Unicode version of the Test_CreateAndWriteFileByName() test function */
int Test_CreateAndWriteFileByName_UTF16(BYTE drv_index, const WORD * filename)
{
    STREAM *handle = NULL;
    DRIVE *drv;
    char *test_str = "String to be written";
    int  write_len;


    if (DriveIndex2PhyDevID(drv_index) == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, drv_index);
        return 0;
    }

    if ((filename == NULL) || ((WORD) *filename == 0x0000))
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return 0;
    }

    if (!SystemCardPresentCheck(drv_index))
    {
        DriveAdd(drv_index);
    }
    drv = DriveGet(drv_index);

    if (CreateFile_UTF16(drv, filename) != FS_SUCCEED)
    {
        MP_ALERT("%s: CreateFile_UTF16() failed, file not created !", __FUNCTION__);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: CreateFile_UTF16() OK, file created !", __FUNCTION__);
    }

    handle = FileOpen(drv);
    if (handle == NULL)
    {
        MP_ALERT("%s: FileOpen() failed !", __FUNCTION__);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: FileOpen() OK !", __FUNCTION__);
    }

    write_len = FileWrite(handle, test_str, StringLength08(test_str));
    if (write_len != StringLength08(test_str))
    {
        MP_ALERT("%s: FileWrite() failed !", __FUNCTION__);

        FileClose(handle);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: FileWrite() OK !", __FUNCTION__);
    }

    FileClose(handle);
    return 1;
}



int Test_CreateAndWriteFileByName_WriteBufferData(BYTE drv_index, const char * filename, const char * ext_name, BYTE * data_buf, int len)
{
    STREAM *handle = NULL;
    DRIVE *drv;
    BYTE NameL, ExtenL;
    int  write_len;


    if (DriveIndex2PhyDevID(drv_index) == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, drv_index);
        return 0;
    }

    if ((filename == NULL) || (strcmp(filename, "") == 0))
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return 0;
    }

    NameL = StringLength08((BYTE *) filename);
    ExtenL = StringLength08((BYTE *) ext_name);

    if (ExtenL)
    {
        MP_DEBUG("enter %s(%s, %s)...", __FUNCTION__, filename, ext_name);
    }
    else
    {
        MP_DEBUG("enter %s(%s, NULL)...", __FUNCTION__, filename);
    }

    if (!SystemCardPresentCheck(drv_index))
    {
        DriveAdd(drv_index);
    }
    drv = DriveGet(drv_index);

    /* CreateFile() supports both DOS 8.3 format and long filename format */
    if (CreateFile(drv, (BYTE *) filename, (BYTE *) ext_name) != FS_SUCCEED)
    {
        MP_ALERT("%s: CreateFile() failed, file not created !", __FUNCTION__);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: CreateFile() OK, file created !", __FUNCTION__);
    }

    handle = FileOpen(drv);
    if (handle == NULL)
    {
        MP_ALERT("%s: FileOpen() failed !", __FUNCTION__);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: FileOpen() OK !", __FUNCTION__);
    }

    write_len = FileWrite(handle, data_buf, len);
    if (write_len != len)
    {
        MP_ALERT("%s: FileWrite() failed !", __FUNCTION__);

        FileClose(handle);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: FileWrite() OK !", __FUNCTION__);
    }

    FileClose(handle);
    return 1;
}



/* UTF-16 Unicode version of the Test_CreateAndWriteFileByName_WriteBufferData() test function */
int Test_CreateAndWriteFileByName_UTF16_WriteBufferData(BYTE drv_index, const WORD * filename, BYTE * data_buf, int len)
{
    STREAM *handle = NULL;
    DRIVE *drv;
    int  write_len;


    if (DriveIndex2PhyDevID(drv_index) == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, drv_index);
        return 0;
    }

    if ((filename == NULL) || ((WORD) *filename == 0x0000))
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return 0;
    }

    if (!SystemCardPresentCheck(drv_index))
    {
        DriveAdd(drv_index);
    }
    drv = DriveGet(drv_index);

    if (CreateFile_UTF16(drv, filename) != FS_SUCCEED)
    {
        MP_ALERT("%s: CreateFile() failed, file not created !", __FUNCTION__);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: CreateFile() OK, file created !", __FUNCTION__);
    }

    handle = FileOpen(drv);
    if (handle == NULL)
    {
        MP_ALERT("%s: FileOpen() failed !", __FUNCTION__);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: FileOpen() OK !", __FUNCTION__);
    }

    write_len = FileWrite(handle, data_buf, len);
    if (write_len != len)
    {
        MP_ALERT("%s: FileWrite() failed !", __FUNCTION__);

        FileClose(handle);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: FileWrite() OK !", __FUNCTION__);
    }

    FileClose(handle);
    return 1;
}



int Test_CreateFolder(BYTE drv_index, const char * folder_name, const char * ext_name)
{
    STREAM *handle = NULL;
    DRIVE *drv;
    BYTE NameL, ExtenL;


    if (DriveIndex2PhyDevID(drv_index) == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, drv_index);
        return 0;
    }

    if ((folder_name == NULL) || (strcmp(folder_name, "") == 0))
    {
        MP_ALERT("%s: Invalid folder name string !", __FUNCTION__);
        return 0;
    }

    NameL = StringLength08((BYTE *) folder_name);
    ExtenL = StringLength08((BYTE *) ext_name);

    if (ExtenL)
    {
        MP_DEBUG("enter %s(%s, %s)...", __FUNCTION__, folder_name, ext_name);
    }
    else
    {
        MP_DEBUG("enter %s(%s, NULL)...", __FUNCTION__, folder_name);
    }

    if (!SystemCardPresentCheck(drv_index))
    {
        DriveAdd(drv_index);
    }
    drv = DriveGet(drv_index);

    if (MakeDir(drv, (BYTE *) folder_name, (BYTE *) ext_name) != FS_SUCCEED)
    {
        MP_ALERT("%s: MakeDir() failed !", __FUNCTION__);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: MakeDir() OK !", __FUNCTION__);
    }

    return 1;
}



/* UTF-16 Unicode version of the Test_CreateFolder() test function */
int Test_CreateFolder_UTF16(BYTE drv_index, const WORD * folder_name)
{
    STREAM *handle = NULL;
    DRIVE *drv;


    if (DriveIndex2PhyDevID(drv_index) == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, drv_index);
        return 0;
    }

    if ((folder_name == NULL) || ((WORD) *folder_name == 0x0000))
    {
        MP_ALERT("%s: Invalid folder name string !", __FUNCTION__);
        return 0;
    }

    if (!SystemCardPresentCheck(drv_index))
    {
        DriveAdd(drv_index);
    }
    drv = DriveGet(drv_index);

    if (MakeDir_UTF16(drv, (WORD *) folder_name) != FS_SUCCEED)
    {
        MP_ALERT("%s: MakeDir_UTF16() failed !", __FUNCTION__);
        return 0;
    }
    else
    {
        MP_DEBUG("%s: MakeDir_UTF16() OK !", __FUNCTION__);
    }

    return 1;
}


//<<test code for file I/O non-aligned buffer address DMA issue >>
#define TEST_BUF_LEN  (800 * 600 * 3)  /* about 1.4 MB */
void Test_File_NonAlignedBuffer_DMA_WrongDataIssue(BYTE drv_index)
{
    DRIVE *drv;
    STREAM *shandle;
    const char fname1[]="file1";
    const char fname2[]="file2";
    const char extname[]="out";
    unsigned long len = TEST_BUF_LEN;
    unsigned long i, count=0;


    MP_DEBUG("Begin %s test...", __FUNCTION__);

    if (DriveIndex2PhyDevID(drv_index) == DEV_NULL)
    {
        MP_ALERT("%s: Invalid drive index %d !", __FUNCTION__, drv_index);
        return;
    }

    BYTE *p = (BYTE *) ext_mem_malloc(len);
    if (p == NULL)
    {
        MP_ALERT("%s: malloc fail for (len = %d) !", __FUNCTION__, len);
        return;
    }

    /* write some test data into buffer */
    BYTE *c = p;
    for (i=0; i < len; i++)
    {
        *c++ = count++;
        if (count == 256)
            count = 0;
    }

    if (!SystemCardPresentCheck(drv_index))
    {
        DriveAdd(drv_index);
    }
    drv = DriveGet(drv_index);

    //[ Step 1: create file1 and write full buffer data into it ]

    MP_DEBUG("%s: Create file1 %s.%s ...", __FUNCTION__, fname1, extname);
    if (CreateFile(drv, (BYTE *) fname1, (BYTE *) extname))
    {
        MP_ALERT("%s: Create file1 %s.%s failed !\n", __FUNCTION__, fname1, extname);
        ext_mem_free(p);
        return;
    }

    if (!(shandle = FileOpen(drv)))
    {
        MP_ALERT("%s: Open file1 %s.%s failed !", __FUNCTION__, fname1, extname);
        ext_mem_free(p);
        return;
    }

    DWORD write_len = FileWrite(shandle, p, len); //write full buffer data to file
    if (write_len != len) //write full buffer data to file
    {
        MP_ALERT("%s: Warning! number of written bytes = %d, not meet expected (= %d) \n", __FUNCTION__, write_len, len);
    }
    FileClose(shandle);

    //[ Step 2: create file2 and write some bytes into it several times to generate non-aligned buffer address cases.
    //          Then write full buffer data into it ]

    //write something ahead
    MP_DEBUG("%s: Create file2 %s.%s", __FUNCTION__, fname2, extname);
    if (CreateFile(drv, (BYTE *) fname2, (BYTE *) extname))
    {
        MP_ALERT("%s: Create file2 %s.%s failed !\n", __FUNCTION__, fname2, extname);
        ext_mem_free(p);
        return;
    }

    if (!(shandle = FileOpen(drv)))
    {
        MP_ALERT("%s: Open file2 %s.%s failed !", __FUNCTION__, fname2, extname);
        ext_mem_free(p);
        return;
    }

    FileWrite(shandle, "\n", StringLength08("\n"));
    FileWrite(shandle, "++", 2);
    FileWrite(shandle, "\n", 1);
    FileWrite(shandle, "---", 3);
    FileWrite(shandle, "#####", 5);

    write_len = FileWrite(shandle, p, len); //write full buffer data to file
    if (write_len != len) //write full buffer data to file
    {
        MP_ALERT("%s: Warning! number of written bytes = %d, not meet expected (= %d) \n", __FUNCTION__, write_len, len);
    }
    FileClose(shandle);

    ext_mem_free(p);

    //[ Step 3: Like Step 1 and 2, Test FileRead() for non-aligned buffer address DMA issue:
    //          Read file 2 content, then write to "OutRawFile.BIN" for binary file comparison externally between file1, file2
    //          and "OutRawFile.BIN" files.
    //          But note that, when reading file 2 content, we intendedly read some bytes several times first to generate
    //          non-aligned buffer address cases, then reading all the rest file content.

    MP_DEBUG("\n==============================\n");
    MP_DEBUG("%s: Open file2 %s.%s for reading its content...", __FUNCTION__, fname2, extname);

    BYTE *rawfile_data_buff;
    int file_size, read_len=0;

    len = TEST_BUF_LEN + 100;  /* about 1.4 MB, add extra 100 bytes for safety */
    rawfile_data_buff = (BYTE *) ext_mem_malloc(len);
    if (rawfile_data_buff == NULL)
    {
        MP_ALERT("%s: malloc fail !", __FUNCTION__);
        return;
    }

    shandle = Test_OpenFileByNameForRead(drv_index, fname2, extname, &file_size);
    if (shandle == NULL)
    {
        MP_ALERT("%s: Test_OpenFileByNameForRead() failed !", __FUNCTION__);
        ext_mem_free(rawfile_data_buff);
        return;
    }

    MpMemSet(rawfile_data_buff, 0, len);

    read_len += FileRead(shandle, rawfile_data_buff, 1); //read one byte only for test!!
    read_len += FileRead(shandle, rawfile_data_buff + read_len, 2); //read two bytes only for test!!
    read_len += FileRead(shandle, rawfile_data_buff + read_len, file_size - read_len); //read whole left content of file!!
    MP_ALERT("%s: total %d bytes was read from file!", __FUNCTION__, read_len);

    Test_CreateAndWriteFileByName_WriteBufferData(drv_index, "OutRawFile", "BIN", rawfile_data_buff, read_len);

    if (shandle)
        FileClose(shandle);

    ext_mem_free(rawfile_data_buff);
    MP_DEBUG("End %s test.", __FUNCTION__);
}

/* --- private test code for File I/O API test, not for public usage --- [end] */
#endif //<< File I/O API test >>



#if (SONY_DCF_ENABLE)
/* To-Do: must re-work this code in exFAT case, because exFAT not support 8.3 short name !! */
/* Check whether if the 8-chars filename of a file or folder meet DCF rules */
BOOL Valid_DCF_PrimaryFilename(const BYTE * primary_name, DCF_ENTRY_FILENAMING_TYPE entry_naming_type)
{
    BYTE i;

    if (primary_name == NULL)
    {
        MP_ALERT("%s: NULL filename pointer !", __FUNCTION__);
        return FALSE;
    }

    if (entry_naming_type == E_DCF_ROOT_DIR_NAMING)
    {
        /* first 3 chars must be digit and in "100" to "999" range */
        for (i=0; i < 3; i++)
        {
            if ((! IsDigit08(primary_name[i])) || (primary_name[0] == '0'))
                return FALSE;
        }

        /* last 5 chars must be alphabet */
        for (i=3; i < 8; i++)
        {
            if (! IsAlpha08(primary_name[i]))
                return FALSE;
        }
    }
    else if (entry_naming_type == E_DCF_OBJ_NAMING)
    {
        /* first 3 chars must be must be alphabet */
        for (i=0; i < 3; i++)
        {
            if (! IsAlpha08(primary_name[i]))
                return FALSE;
        }

        /* last 5 chars must be digit */
        for (i=3; i < 8; i++)
        {
            if (! IsDigit08(primary_name[i]))
                return FALSE;
        }
    }
    else
    {
        MP_ALERT("%s: Invalid DCF entry naming type parameter !", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}



/* To-Do: must re-work this code in exFAT case, because exFAT not support 8.3 short name !! */
/* Return extracted 3-digit signed number from the filename. 100 ~ 999 for valid cases, and -1 for invalid case */
SWORD Extract_DigitNum_of_RootDCIM_000AAAAA_Dir(const BYTE * primary_name)
{
    SWORD  digitNum;  /* 100 ~ 999 for valid cases, and -1 for invalid case */
    BYTE   digitStr[4];  /* 3 digit chars + null terminator */

    if (Valid_DCF_PrimaryFilename(primary_name, E_DCF_ROOT_DIR_NAMING))
    {
        memcpy(digitStr, primary_name, 3);
        digitStr[3] = 0;
        digitNum = atoi(digitStr);
        MP_DEBUG("%s: digitNum = %03d", __FUNCTION__, digitNum);
    }
    else
        digitNum = -1; /* -1 means invalid */

    return digitNum;
}



/* To-Do: must re-work this code in exFAT case, because exFAT not support 8.3 short name !! */
/* Return extracted 5-digit signed number from the filename. 00000 ~ 99999 for valid cases, and -1 for invalid case */
int Extract_DigitNum_of_DCF_Obj(const BYTE * primary_name)
{
    int   digitNum;  /* 00000 ~ 99999 for valid cases, and -1 for invalid case */
    BYTE  digitStr[6];  /* 5 digit chars + null terminator */

    if (Valid_DCF_PrimaryFilename(primary_name, E_DCF_OBJ_NAMING))
    {
        memcpy(digitStr, (BYTE *) ((BYTE *) primary_name + 3), 5);
        digitStr[5] = 0;
        digitNum = atoi(digitStr);
        MP_DEBUG("%s: digitNum = %05d", __FUNCTION__, digitNum);
    }
    else
        digitNum = -1; /* -1 means invalid */

    return digitNum;
}



/* Make up the primary part of DCF displayed name (ex: 100-0001.JPG) string for a specified DCF object.
 * Return PASS or FAIL.
 *
 * param   DCF_obj_pri_name         [IN] The original 8-char primary filename for the specified DCF object.
 * param   file_list_index          [IN] The file list entry index for the specified DCF object.
 * param   output_name_buffer       [OUT] Pointer to the buffer for storing output DCF displayed name.
 * param   output_buf_len           [IN] The size of output buffer.
 * param   output_display_integer   [OUT] Output 32-bit integer of the 'merged' interger format for the DCF displayed name. (ex: 100-0001.JPG => integer 1000001)
 */
SWORD Makeup_DCF_Obj_DisplayName(BYTE * DCF_obj_pri_name, DWORD file_list_index, BYTE * output_name_buffer, BYTE output_buf_len, DWORD * output_display_integer)
{
    BYTE display_name[9]; /* 8 chars + null terminator */
    DCF_INFO_TYPE *dcf_info;
    SWORD dir_digitNum;
    DWORD merged_integer;
    BYTE  i;

    if (g_FileList_DCF_Info_List == NULL)  /* no DCF info available */
    {
        MP_ALERT("%s: (g_FileList_DCF_Info_List == NULL) => return FAIL !", __FUNCTION__);
        return FAIL;
    }

    if ((DCF_obj_pri_name) && (file_list_index <= TEMP_FILE_LIST_MAX_ENTRIES_COUNT - 1) && (output_name_buffer) && (output_buf_len >= 8))
    {
        dcf_info = (DCF_INFO_TYPE *) &g_FileList_DCF_Info_List[file_list_index];
        if (dcf_info->Flags.f_maybe_DCF_Obj && dcf_info->RootDCIM_000AAAAA_Dir_DigitNum)
        {
            dir_digitNum = dcf_info->RootDCIM_000AAAAA_Dir_DigitNum;
            display_name[0] = '0' + (dir_digitNum / 100);
            display_name[1] = '0' + ((dir_digitNum % 100) / 10);
            display_name[2] = '0' + (dir_digitNum % 10);
            display_name[3] = '-';
            memcpy((BYTE *) &display_name[4], (BYTE *) &DCF_obj_pri_name[4], 4);
            display_name[8] = 0;

            if (output_buf_len > 8)
                memcpy((BYTE *) output_name_buffer, display_name, 9); /* include null terminator */
            else
                memcpy((BYTE *) output_name_buffer, display_name, 8); /* not include null terminator */

            if (output_display_integer)
            {
                merged_integer = 0;
                for (i=0; i < 8; i++)
                {
                    if ((display_name[i] != '-') && (i != 3))
                        merged_integer = merged_integer * 10 + (display_name[i] - '0');
                }
                *output_display_integer = merged_integer;
            }

            return PASS;
        }
    }

    MP_ALERT("%s: return FAIL !", __FUNCTION__);
    return FAIL;
}



/* Read the bit entry value (0 or 1) from the bitmap table with respect to the /DCIM/000AAAAA format directory digits number.
 * Note: Here, we use return value 0xFF to present error condition occurred.
 */
static BYTE Read_DCIM_000AAAAA_Dir_BitmapEntry(BYTE * bitmap_table, WORD dir_digits_num)
{
    BYTE content_byte;
    BYTE ret_bit;
    WORD byte_offset;
    BYTE  bit_offset;

    if (bitmap_table == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return 0xFF;
    }

    if ((dir_digits_num < 100) || (dir_digits_num > 999))
    {
        MP_ALERT("%s: Error! dir_digits_num parameter value out of range !", __FUNCTION__);
        return 0xFF;
    }

    byte_offset = (dir_digits_num / 8);
    bit_offset = (dir_digits_num % 8);

    if (byte_offset >= DCIM_000AAAAA_DIR_DIGITS_BITMAP_LEN)
    {
        MP_ALERT("%s: Error! dir_digits_num parameter value out of range !", __FUNCTION__);
        return 0xFF;
    }

    content_byte = (BYTE) *((BYTE *) bitmap_table + byte_offset);
    ret_bit = (BYTE) (content_byte & (0x01 << bit_offset)); /* note: bit order for each /DCIM/000AAAAA format directory digits number: from LSB bit to MSB bit */

    return (ret_bit > 0)? 0x01:0x00;
}



/* Set the bit value to the entry for the /DCIM/000AAAAA format directory digits number in the bitmap table.
 *
 * return  FS_SUCCEED         Set value successfully. \n\n
 * return  ABNORMAL_STATUS    Failed due to some error. \n\n
 */
static int Set_DCIM_000AAAAA_Dir_BitmapEntry(BYTE * bitmap_table, WORD dir_digits_num, BYTE bit_value)
{
    BYTE content_byte;
    BYTE *addr;
    WORD byte_offset;
    BYTE  bit_offset;

    if (bitmap_table == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (bit_value > 1)
    {
        MP_ALERT("%s: Error! Invalid input value !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if ((dir_digits_num < 100) || (dir_digits_num > 999))
    {
        MP_ALERT("%s: Error! dir_digits_num parameter value out of range !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    byte_offset = (dir_digits_num / 8);
    bit_offset = (dir_digits_num % 8);

    if (byte_offset >= DCIM_000AAAAA_DIR_DIGITS_BITMAP_LEN)
    {
        MP_ALERT("%s: Error! dir_digits_num parameter value out of range !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    addr = (BYTE *) ((BYTE *) bitmap_table + byte_offset);
    content_byte = (BYTE) *addr;

    /* note: bit order for each /DCIM/000AAAAA format directory digits number: from LSB bit to MSB bit */
    if (bit_value & 0x01)
        (BYTE) *addr = (BYTE) (content_byte | (0x01 << bit_offset));
    else
        (BYTE) *addr = (BYTE) (content_byte & (~(0x01 << bit_offset)));

    return FS_SUCCEED;
}



/* SONY DCF rules only apply on Photo/Image files. So, if not in OP_IMAGE_MODE mode, always return 0 */
DWORD Get_DCF_Obj_TotalCnt(void)
{
    if ((g_psSystemConfig->dwCurrentOpMode != OP_IMAGE_MODE) || (g_FileList_DCF_Info_List == NULL))
        return 0;
    else
        return g_DCF_Obj_TotalCnt;
}



/* Arrange the file list to two sets: DCF objects set, and non-DCF files set. And then sort the files of the file list
 * in SONY DCF file name ascending or descending order.
 *
 * param   sSearchInfo         The pointer of ST_SEARCH_INFO list array to be sorted. \n\n
 * param   dwElementCount      The total count of entries in the list. \n\n
 * param   sorting_basis       The sorting basis to according to.
 *
 * retval  FS_SUCCEED          Sorting successfully. \n\n
 * retval  ABNORMAL_STATUS     Sorting unsuccessfully due to some error.
 */
SWORD FileSort_for_Sony_DCF_Name_Order(ST_SEARCH_INFO * sSearchInfo, DWORD dwElementCount, FILE_SORTING_BASIS_TYPE sorting_basis)
{
    FILE_SORTING_BASIS_TYPE  sorting_method;
    BOOL  f_found_item_for_swap;
    SWORD ret;
    DWORD i, j;


    if (dwElementCount < 2)
    {
        MP_DEBUG("%s: dwElementCount (=%d) < 2 => return.", __FUNCTION__, dwElementCount);
        return FS_SUCCEED;
    }

    if (sSearchInfo == NULL)
    {
        MP_ALERT("%s: Error! NULL ST_SEARCH_INFO pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if ((sorting_basis >= FILE_SORTING_BY_83SHORT_NAME) && (sorting_basis < FILE_SORTING_MAX))
    {
        if ((g_FileList_DCF_Info_List != NULL) && (g_DCF_Obj_TotalCnt > 0))
        {
            if ((sorting_basis == FILE_SORTING_BY_83SHORT_NAME_REVERSE) || (sorting_basis == FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE))  /* reverse order => descending order of SONY file name order */
                sorting_method = FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE;
            else if ((sorting_basis == FILE_SORTING_BY_83SHORT_NAME) || (sorting_basis == FILE_SORTING_BY_SONY_DCF_NAMING))  /* ascending order of SONY file name order */
                sorting_method = FILE_SORTING_BY_SONY_DCF_NAMING;

            MP_ALERT("%s: sort files in SONY DCF file name %s order ...", __FUNCTION__, ((sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE) ? "Descending" : "Ascending"));

            /* arrange file list to two sets: DCF objects set, and non-DCF files set.
             * in ascending order => the 1st half is DCF objects set, the 2nd half is non-DCF files set;
             * in descending order => the 1st half is non-DCF files set, the 2nd half is DCF objects set.
             */
            for (i=0; i < dwElementCount; i++)
            {
                if (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING)  /* in ascending order */
                {
                    if (! g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj)  /* a non-DCF file */
                    {
                        f_found_item_for_swap = FALSE;
                        for (j = (i + 1); j < dwElementCount; j++)
                        {
                            if (g_FileList_DCF_Info_List[j].Flags.f_maybe_DCF_Obj)  /* a DCF object */
                            {
                                f_found_item_for_swap = TRUE;
                                break;
                            }
                        }

                        if (f_found_item_for_swap)
                        {
                            SwapData((DWORD) &sSearchInfo[i], (DWORD) &sSearchInfo[j], sizeof(ST_SEARCH_INFO));
                            SwapData((DWORD) &g_FileList_DCF_Info_List[i], (DWORD) &g_FileList_DCF_Info_List[j], sizeof(DCF_INFO_TYPE));
                        }
                    }
                }
                else if (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE)  /* in descending order */
                {
                    if (g_FileList_DCF_Info_List[i].Flags.f_maybe_DCF_Obj)  /* a DCF object */
                    {
                        f_found_item_for_swap = FALSE;
                        for (j = (i + 1); j < dwElementCount; j++)
                        {
                            if (! g_FileList_DCF_Info_List[j].Flags.f_maybe_DCF_Obj)  /* a non-DCF file */
                            {
                                f_found_item_for_swap = TRUE;
                                break;
                            }
                        }

                        if (f_found_item_for_swap)
                        {
                            SwapData((DWORD) &sSearchInfo[i], (DWORD) &sSearchInfo[j], sizeof(ST_SEARCH_INFO));
                            SwapData((DWORD) &g_FileList_DCF_Info_List[i], (DWORD) &g_FileList_DCF_Info_List[j], sizeof(DCF_INFO_TYPE));
                        }
                    }
                }
            }

            if ((sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING) || (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE))
            {
    /* these 2 methods both work well. It's valuable for reference ... */
    #if 1  /* simply sorting whole file list using the SONY DCF-specific sorting method */
                /* simply sorting whole file list using the SONY DCF-specific sorting method */
                MP_DEBUG("%s: sorting whole file list ...", __FUNCTION__);
                g_Sorting_Start_Index_in_Companion_Info_List = 0;
                ret = FileSort(&sSearchInfo[0], dwElementCount, sorting_method);
    #else  /* sorting DCF objects part and non-DCF files part separately */
                /* if file name ascending order is chosen, sort DCF objects first, and then sort non-DCF files, both in ascending order */
                /* if file name descending order is chosen, sort non-DCF files first, and then sort DCF objects, both in descending order */
                if (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING)  /* in ascending order */
                {
                    MP_DEBUG("%s: sorting DCF objects part ...", __FUNCTION__);
                    g_Sorting_Start_Index_in_Companion_Info_List = 0;
                    ret = FileSort(&sSearchInfo[g_Sorting_Start_Index_in_Companion_Info_List], g_DCF_Obj_TotalCnt, sorting_method);
                    if (ret == FS_SUCCEED)
                    {
                        MP_DEBUG("%s: sorting non-DCF files part ...", __FUNCTION__);
                        g_Sorting_Start_Index_in_Companion_Info_List = g_DCF_Obj_TotalCnt;
                        ret = FileSort(&sSearchInfo[g_Sorting_Start_Index_in_Companion_Info_List], (dwElementCount - g_DCF_Obj_TotalCnt), FILE_SORTING_BY_83SHORT_NAME);
                    }
                }
                else if (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE)  /* in descending order */
                {
                    MP_DEBUG("%s: sorting non-DCF files part ...", __FUNCTION__);
                    g_Sorting_Start_Index_in_Companion_Info_List = 0;
                    ret = FileSort(&sSearchInfo[g_Sorting_Start_Index_in_Companion_Info_List], (dwElementCount - g_DCF_Obj_TotalCnt), FILE_SORTING_BY_83SHORT_NAME_REVERSE);
                    if (ret == FS_SUCCEED)
                    {
                        MP_DEBUG("%s: sorting DCF objects part ...", __FUNCTION__);
                        g_Sorting_Start_Index_in_Companion_Info_List = (dwElementCount - g_DCF_Obj_TotalCnt);
                        ret = FileSort(&sSearchInfo[g_Sorting_Start_Index_in_Companion_Info_List], g_DCF_Obj_TotalCnt, sorting_method);
                    }
                }
    #endif
            }
            else
            {
                MP_ALERT("%s: Error! Improper sorting basis for this function ! => check source code for debugging ...", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
        }
        else
        {
            g_Sorting_Start_Index_in_Companion_Info_List = 0;
            ret = FileSort(&sSearchInfo[0], dwElementCount, sorting_method);
        }

        return ret;
    }
    else
    {
        MP_ALERT("%s: Error! Invalid sorting basis parameter value !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
}



/* Check if a file list entry is within the DCF objects set of the well-arranged file list.
 * The whole file list should be already well-arranged to DCF objects set and non-DCF files set before being called.
 */

/* Check whether if a specified file list entry is within the DCF objects set of the well-arranged file list. And return
 * the file list index corresponding to the specified file list entry.
 *
 * param   pSearchInfo          [IN] The pointer of ST_SEARCH_INFO list array to be sorted. \n\n
 * param   return_list_index    [OUT] The returned file list index corresponding to the specified file list entry. \n\n
 *
 * retval  TRUE      The specified file list entry is a DCF object. \n\n
 * retval  FALSE     The specified file list entry is not a DCF object or some error occurred.
 */
BOOL Check_If_FileListEntry_In_DCF_Objs_Set(ST_SEARCH_INFO * pSearchInfo, DWORD * return_list_index)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;


    if (pSearchInfo == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return FALSE;
    }

    if ((psSysConfig->dwCurrentOpMode == OP_IMAGE_MODE) && (g_FileList_DCF_Info_List != NULL) && (Get_DCF_Obj_TotalCnt() > 0))
    {
        DWORD file_list_index;
        DWORD total_entries_cnt = psFileBrowser->dwFileListCount[psSysConfig->dwCurrentOpMode];

        /* note: if 'pSearchInfo' is a pointer to an extra ST_SEARCH_INFO buffer with copied info, we must not simply use
         *       file_list_index = (ST_SEARCH_INFO *) pSearchInfo - (ST_SEARCH_INFO *) g_WorkingFileList_BasePtr;  to get its
         *       index within the working file list. Because their pointer values (memory addresses) are totally independent !!
         */
        DWORD i;
        for (i=0; i < g_WorkingFileList_TotalCount; i++)
        {
            /* check if exactly same ST_SEARCH_INFO content, but not to use memcmp() for performance issue */
            if ( (pSearchInfo->dwDirPoint == g_WorkingFileList_BasePtr[i].dwDirPoint) &&
                 (pSearchInfo->dwFdbOffset == g_WorkingFileList_BasePtr[i].dwFdbOffset) &&
                 (pSearchInfo->DrvIndex == g_WorkingFileList_BasePtr[i].DrvIndex) )
            {
                file_list_index = i;
                break;
            }
        }

        if (return_list_index)
            *return_list_index = file_list_index;

        FILE_SORTING_BASIS_TYPE  sorting_method = GetCurFileSortingBasis();

        /* only care DCF info under these file sorting methods */
        if ( (sorting_method == FILE_SORTING_BY_83SHORT_NAME) || (sorting_method == FILE_SORTING_BY_83SHORT_NAME_REVERSE) ||
             (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING) || (sorting_method == FILE_SORTING_BY_SONY_DCF_NAMING_REVERSE) )
        {
            /* check if the file list entry is within DCF objects range */
            if ( (file_list_index < total_entries_cnt) && (g_FileList_DCF_Info_List[file_list_index].Flags.f_maybe_DCF_Obj) )
                return TRUE;
            else
                return FALSE;
        }
        else
            return FALSE;
    }
    else
        return FALSE;
}



/* Return the image display order value for the specified file extension name for DCF essential objects */
/* Display order for DCF image:  JPG > TIF > BMP > RAW > (THM) > JPE > JFI  (how about THM, SRF, SR2 ??) */
BYTE DCF_Img_DisplayOrderValue(BYTE *ext_name)
{
    if (ext_name == NULL)
        return DCF_LATEST_DISPLAY_ORDER;

    if (StringNCompare08_CaseInsensitive(ext_name, "JPG", 3) == E_COMPARE_EQUAL)
        return IMAGE_DISPLAY_ORDER_JPG;
    else if (StringNCompare08_CaseInsensitive(ext_name, "TIF", 3) == E_COMPARE_EQUAL)
        return IMAGE_DISPLAY_ORDER_TIF;
    else if (StringNCompare08_CaseInsensitive(ext_name, "BMP", 3) == E_COMPARE_EQUAL)
        return IMAGE_DISPLAY_ORDER_BMP;
    else if (StringNCompare08_CaseInsensitive(ext_name, "ARW", 3) == E_COMPARE_EQUAL) /* how about .SRF & .SR2 ??? */
        return IMAGE_DISPLAY_ORDER_RAW;
    else if (StringNCompare08_CaseInsensitive(ext_name, "THM", 3) == E_COMPARE_EQUAL)
        return IMAGE_DISPLAY_ORDER_THM;
    else if (StringNCompare08_CaseInsensitive(ext_name, "JPE", 3) == E_COMPARE_EQUAL)
        return IMAGE_DISPLAY_ORDER_JPE;
    else if (StringNCompare08_CaseInsensitive(ext_name, "JFI", 3) == E_COMPARE_EQUAL)
        return IMAGE_DISPLAY_ORDER_JFI;
    else
        return DCF_LATEST_DISPLAY_ORDER;
}



/* Return the thumbnail display order value for the specified file extension name for DCF essential objects */
/* Display order for thumbnail:  THM > JPG > TIF > BMP > RAW > JPE > JFI    (when to adopt this order ??) */
BYTE DCF_Thum_DisplayOrderValue(BYTE *ext_name)
{
    if (ext_name == NULL)
        return DCF_LATEST_DISPLAY_ORDER;

    if (StringNCompare08_CaseInsensitive(ext_name, "THM", 3) == E_COMPARE_EQUAL)
        return THUMB_DISPLAY_ORDER_THM;
    else if (StringNCompare08_CaseInsensitive(ext_name, "JPG", 3) == E_COMPARE_EQUAL)
        return THUMB_DISPLAY_ORDER_JPG;
    else if (StringNCompare08_CaseInsensitive(ext_name, "TIF", 3) == E_COMPARE_EQUAL)
        return THUMB_DISPLAY_ORDER_TIF;
    else if (StringNCompare08_CaseInsensitive(ext_name, "BMP", 3) == E_COMPARE_EQUAL)
        return THUMB_DISPLAY_ORDER_BMP;
    else if (StringNCompare08_CaseInsensitive(ext_name, "ARW", 3) == E_COMPARE_EQUAL) /* how about .SRF & .SR2 ??? */
        return THUMB_DISPLAY_ORDER_RAW;
    else if (StringNCompare08_CaseInsensitive(ext_name, "JPE", 3) == E_COMPARE_EQUAL)
        return THUMB_DISPLAY_ORDER_JPE;
    else if (StringNCompare08_CaseInsensitive(ext_name, "JFI", 3) == E_COMPARE_EQUAL)
        return THUMB_DISPLAY_ORDER_JFI;
    else
        return DCF_LATEST_DISPLAY_ORDER;
}
#endif //SONY_DCF_ENABLE

#if (PRODUCT_UI==UI_WELDING)
void CheckAndWriteFile(DRIVE * pDrv, BYTE * name, BYTE * extension,BYTE *pbBuffer,DWORD dwFileSize)
{
	STREAM* handle = NULL;
	DWORD dwLen;

	DirReset(pDrv);
	if(FileSearch(pDrv, name, extension, E_FILE_TYPE) != FS_SUCCEED)
	{
		if( CreateFile(pDrv, name, extension) != FS_SUCCEED)
		{
			MP_ALERT("File: %s.%s, Create Fail!", name, extension);
			return;
		}
	}
	handle = FileOpen(pDrv);
	dwLen = FileWrite(handle, (BYTE *)pbBuffer, dwFileSize);
	FileClose(handle);
		
	if(dwLen != dwFileSize)
		MP_ALERT("File: %s.%s, Write %d bytes fail!(Write %d bytes)", name, extension, dwFileSize, dwLen);

}
#endif

MPX_KMODAPI_SET(PathAPI__Locate_Path);
MPX_KMODAPI_SET(PathAPI__Locate_Path_UTF16);
MPX_KMODAPI_SET(PathAPI__Fopen);
MPX_KMODAPI_SET(PathAPI__Fopen_UTF16);
MPX_KMODAPI_SET(PathAPI__CreateFile);
MPX_KMODAPI_SET(PathAPI__CreateFile_UTF16);
MPX_KMODAPI_SET(PathAPI__RemoveFile);
MPX_KMODAPI_SET(PathAPI__RemoveFile_UTF16);
MPX_KMODAPI_SET(PathAPI__GetFileAttributeAndTimeInfo);
MPX_KMODAPI_SET(PathAPI__GetFileAttributeAndTimeInfo_UTF16);
MPX_KMODAPI_SET(FileSearch);
MPX_KMODAPI_SET(FileSearchLN);
MPX_KMODAPI_SET(CreateFile);
MPX_KMODAPI_SET(CreateFile_UTF16);
MPX_KMODAPI_SET(DeleteFile);
MPX_KMODAPI_SET(FileOpen);
MPX_KMODAPI_SET(FileRead);
MPX_KMODAPI_SET(FileWrite);
MPX_KMODAPI_SET(FileClose);
MPX_KMODAPI_SET(GetFilenameOfCurrentNode);

