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
* Filename      : dir.c
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
#include "utiltypedef.h"
#include "global612.h"
#include "mpTrace.h"
#include "dir.h"
#include "fat.h"
#include "exfat.h"
#include "drive.h"
#include "chain.h"
#include "file.h"
#include "index.h"
#include "os.h"
#include <string.h>
#include "taskid.h"


static DWORD g_dwDeleteFDBCount;
static DWORD g_dwDeletedNodeCount;


int FdbCopy(DRIVE * drv, FDB * external);
int NextNode(DRIVE * drv);
extern BOOL (*fileUiUpdateIconAniPtr)(void);


/////////////////////////////////////////////////////////////////////////////
//         The following function calls are for directory access           //
/////////////////////////////////////////////////////////////////////////////



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to walk through the entire path (UTF-8 string) to change directory into it.
///
///@param   utf8_path_str        Pointer to the input UTF-8 or ASCII path string. \n\n
///
///@retval  FS_SUCCEED           Change directory to the path successfully. \n\n
///@retval  ABNORMAL_STATUS      Failed to change directory to the path. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
int PathAPI__Cd_Path(const BYTE * utf8_path_str)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;


    if ((utf8_path_str == NULL) || (strcmp(utf8_path_str, "") == 0))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path(utf8_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/directory cannot be located due to some error !", __FUNCTION__);
        }
        else
        {
            MP_ALERT("%s: -I- target directory of the path not found.", __FUNCTION__);
        }
        return ABNORMAL_STATUS;
    }
    else
    {
        MP_DEBUG("%s: locate path OK, and changed to the path !", __FUNCTION__);
    }

    return FS_SUCCEED;
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to walk through the entire path (UTF-16 string) to change directory into it. \n\n
///         This function is the UTF-16 Unicode version of PathAPI__Cd_Path() function.
///
///@param   utf16_path_str    Pointer to the input UTF-16 path string. \n\n
///
///@retval  FS_SUCCEED           Change directory to the path successfully. \n\n
///@retval  ABNORMAL_STATUS      Failed to change directory to the path. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
///@remark   This function call will modify the content of the input UTF-16 path string while processing tokens. \n\n
///
int PathAPI__Cd_Path_UTF16(const WORD * utf16_path_str)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/directory cannot be located due to some error !", __FUNCTION__);
        }
        else
        {
            MP_ALERT("%s: -I- target directory of the path not found.", __FUNCTION__);
        }
        return ABNORMAL_STATUS;
    }
    else
    {
        MP_DEBUG("%s: locate path OK, and changed to the path !", __FUNCTION__);
    }

    return FS_SUCCEED;
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement directory creation function with path supported.\n
///         If any midway directory and/or the final directory in the path does not exist, this function will create it and enter it. \n
///         If the midway directories and the final directory in the path already exist, this function will just enter it. \n\n
///
///@param   utf8_path_str    The UTF-8 or ASCII path string of a directory to be created. \n\n
///
///@retval  FS_SUCCEED         Create the subdirectory successfully. \n\n
///@retval  ABNORMAL_STATUS    Failed to create the subdirectory. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
///@remark   If any midway directory and/or the final directory in the path does not exist, this function will create it and enter it. \n
///          If the midway directories and the final directory in the path already exist, this function will just enter it. \n\n
///
int PathAPI__MakeDir(const BYTE * utf8_path_str)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    BOOL   dir_found = FALSE;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;


    if ((utf8_path_str == NULL) || (strcmp(utf8_path_str, "") == 0))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path(utf8_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_DIR, E_AUTO_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/directory cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    }
    else
    {
        MP_ALERT("%s: locate path OK.", __FUNCTION__);
        dir_found = TRUE;
    }

    return FS_SUCCEED;
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement directory creation function with path supported.\n
///         If any midway directory and/or the final directory in the path does not exist, this function will create it and enter it. \n
///         If the midway directories and the final directory in the path already exist, this function will just enter it. \n\n
///         This function is the UTF-16 Unicode version of PathAPI__MakeDir() function.
///
///@param   utf16_path_str   The UTF-16 path string of a directory to be created. \n\n
///
///@retval  FS_SUCCEED         Create the subdirectory successfully. \n\n
///@retval  ABNORMAL_STATUS    Failed to create the subdirectory. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
///@remark   1. The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   2. If any midway directory and/or the final directory in the path does not exist, this function will create it and enter it. \n
///          If the midway directories and the final directory in the path already exist, this function will just enter it. \n\n
///
int PathAPI__MakeDir_UTF16(const WORD * utf16_path_str)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    BOOL   dir_found = FALSE;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_DIR, E_AUTO_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/directory cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    }
    else
    {
        MP_ALERT("%s: locate path OK.", __FUNCTION__);
        dir_found = TRUE;
    }

    return FS_SUCCEED;
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to delete a directory and all files/subdirectories within it with path supported.
///
///@param   utf8_path_str       The UTF-8 or ASCII path string of a directory to be deleted. \n\n
///
///@retval  FS_SUCCEED          Delete successfully. \n\n
///@retval  ABNORMAL_STATUS     Delete unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      Directory not found, no need to delete it. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
int PathAPI__RemoveDir(const BYTE * utf8_path_str)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    WORD   *wUtf16_TempBuf;
    BYTE   *bUtf8_TempBuf;
    int    ret;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;


    if ((utf8_path_str == NULL) || (strcmp(utf8_path_str, "") == 0))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path(utf8_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/directory cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: final directory not found in the path !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        MP_DEBUG("%s: final directory found in the path !", __FUNCTION__);
        wUtf16_TempBuf = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
        if (wUtf16_TempBuf == NULL)
        {
            MP_ALERT("-E %s() malloc fail", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        drv = DriveGet(new_drv_id);
        ret = CdParent(drv);
        if (ret != FS_SUCCEED)
        {
            MP_ALERT("%s: CdParent() failed !", __FUNCTION__);
            ext_mem_free(wUtf16_TempBuf);
            return ABNORMAL_STATUS;
        }

        MpMemSet((BYTE *) wUtf16_TempBuf, 0, MAX_L_NAME_LENG * sizeof(WORD));

        if ( (BYTE) *((BYTE *) utf8_path_str + (final_token_offset + final_token_strlen)) == 0x00 )
            mpx_UtilUtf8ToUnicodeU16(wUtf16_TempBuf, (BYTE *) utf8_path_str + final_token_offset); /* convert to UTF-16 string */
        else
        {
            /* trim off tailing spaces */
            bUtf8_TempBuf = (BYTE *) ext_mem_malloc(MAX_L_NAME_LENG);
            if (bUtf8_TempBuf == NULL)
            {
                MP_ALERT("-E %s() malloc fail", __FUNCTION__);
                ext_mem_free(wUtf16_TempBuf);
                return ABNORMAL_STATUS;
            }
            MpMemSet(bUtf8_TempBuf, 0, MAX_L_NAME_LENG);
            MpMemCopy(bUtf8_TempBuf, ((BYTE *) utf8_path_str + final_token_offset), final_token_strlen);

            mpx_UtilUtf8ToUnicodeU16(wUtf16_TempBuf, bUtf8_TempBuf); /* convert to UTF-16 string */
            ext_mem_free(bUtf8_TempBuf);
        }

        if (FileSearchLN(drv, wUtf16_TempBuf, StringLength16(wUtf16_TempBuf), E_DIR_TYPE) != FS_SUCCEED)
        {
            MP_ALERT("%s: FileSearchLN() failed to find the final directory !", __FUNCTION__);
            ext_mem_free(wUtf16_TempBuf);
            return ABNORMAL_STATUS;
        }

        MP_ALERT("%s: directory found => delete it ...", __FUNCTION__);
        ret = DeleteDir(drv);

        ext_mem_free(wUtf16_TempBuf);
        return ret;
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to delete a directory and all files/subdirectories within it with path supported. \n
///         This function is the UTF-16 Unicode version of PathAPI__RemoveDir_UTF16() function.
///
///@param   utf16_path_str   The UTF-16 path string of a directory to be deleted. \n\n
///
///@retval  FS_SUCCEED          Delete successfully. \n\n
///@retval  ABNORMAL_STATUS     Delete unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      Directory not found, no need to delete it. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
///@remark   The path string must be converted to UTF-16 before calling this function. \n\n
///
int PathAPI__RemoveDir_UTF16(const WORD * utf16_path_str)
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

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/directory cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: final directory not found in the path !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        MP_DEBUG("%s: final directory found in the path !", __FUNCTION__);
        drv = DriveGet(new_drv_id);
        ret = CdParent(drv);
        if (ret != FS_SUCCEED)
        {
            MP_ALERT("%s: CdParent() failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

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
            MP_ALERT("%s: FileSearchLN() failed to find the final directory !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        MP_ALERT("%s: directory found => delete it ...", __FUNCTION__);
        ret = DeleteDir(drv);
        return ret;
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to get the count of directory entries in a directory specified by the path string.
///
///@param   utf8_path_str       [IN] The UTF-8 or ASCII path string of a directory to get the count of directory entries within it. \n\n
///@param   entries_count       [OUT] Returned count of directory entries in the directory. \n\n
///
///@retval  FS_SUCCEED          Get the count of directory entries successfully. \n\n
///@retval  ABNORMAL_STATUS     Failed to get the count of directory entries due to some error. \n\n
///@retval  FILE_NOT_FOUND      The specified directory not found, or nothing in the specified directory. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
///@remark   The count of directory entries in the directory excludes the "." and ".." directory entries if exist.
///
int PathAPI__GetEntriesCountInDir(const BYTE * utf8_path_str, DWORD * entries_count)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    int    ret;


    if ((utf8_path_str == NULL) || (strcmp(utf8_path_str, "") == 0))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (entries_count == NULL)
    {
        MP_ALERT("%s: Error ! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
    *entries_count = 0;

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path(utf8_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/directory cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: final directory not found in the path !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        MP_DEBUG("%s: final directory found in the path !", __FUNCTION__);
        drv = DriveGet(new_drv_id);

        ret = DirFirst(drv);
        if (ret == END_OF_DIR)
        {
            MP_ALERT("%s: No file or sub-dir in the current directory !", __FUNCTION__);
            MP_ALERT("%s: count of directory entries in the sub-directory = %u \r\n", __FUNCTION__, (*entries_count));
            return FILE_NOT_FOUND;
        }
        else if (ret != FS_SUCCEED)
        {
            MP_ALERT("%s: DirFirst() failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            do
            {
        #if EXFAT_ENABLE
                if (drv->Flag.FsType == FS_TYPE_exFAT) /* note: exFAT directories have no '.' and '..' entries */
                    (*entries_count)++;
                else
        #endif
                {
                    if (drv->Node->Name[0] == '.') /* if '.' or '..' then bypass */
                        continue;
                	else
                        (*entries_count)++;
                }
            } while (DirNext(drv) == FS_SUCCEED);
        }

        MP_ALERT("%s: count of directory entries in the sub-directory = %u \r\n", __FUNCTION__, (*entries_count));
        return FS_SUCCEED;
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to get the count of directory entries in a directory specified by the path string. \n
///         This function is the UTF-16 Unicode version of PathAPI__GetEntriesCountInDir() function.
///
///@param   utf16_path_str      [IN] The UTF-16 path string of a directory to get the count of directory entries within it. \n\n
///@param   entries_count       [OUT] Returned count of directory entries in the directory. \n\n
///
///@retval  FS_SUCCEED          Get the count of directory entries successfully. \n\n
///@retval  ABNORMAL_STATUS     Failed to get the count of directory entries due to some error. \n\n
///@retval  FILE_NOT_FOUND      The specified directory not found, or nothing in the specified directory. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
///@remark   The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   The count of directory entries in the directory excludes the "." and ".." directory entries if exist.
///
int PathAPI__GetEntriesCountInDir_UTF16(const WORD * utf16_path_str, DWORD * entries_count)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    DRIVE  *drv;
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    int    ret;


    if ((utf16_path_str == NULL) || ((WORD) *utf16_path_str == 0x0000))
    {
        MP_ALERT("%s: Invalid input path string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (entries_count == NULL)
    {
        MP_ALERT("%s: Error ! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
    *entries_count = 0;

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, E_PATH_TARGET_IS_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("%s: Error! path/directory cannot be located due to some error !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("%s: final directory not found in the path !", __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        MP_DEBUG("%s: final directory found in the path !", __FUNCTION__);
        drv = DriveGet(new_drv_id);

        ret = DirFirst(drv);
        if (ret == END_OF_DIR)
        {
            MP_ALERT("%s: No file or sub-dir in the current directory !", __FUNCTION__);
            MP_ALERT("%s: count of directory entries in the sub-directory = %u \r\n", __FUNCTION__, (*entries_count));
            return FILE_NOT_FOUND;
        }
        else if (ret != FS_SUCCEED)
        {
            MP_ALERT("%s: DirFirst() failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            do
            {
        #if EXFAT_ENABLE
                if (drv->Flag.FsType == FS_TYPE_exFAT) /* note: exFAT directories have no '.' and '..' entries */
                    (*entries_count)++;
                else
        #endif
                {
                    if (drv->Node->Name[0] == '.') /* if '.' or '..' then bypass */
                        continue;
                	else
                        (*entries_count)++;
                }
            } while (DirNext(drv) == FS_SUCCEED);
        }

        MP_ALERT("%s: count of directory entries in the sub-directory = %u \r\n", __FUNCTION__, (*entries_count));
        return FS_SUCCEED;
    }
}



#if FS_REENTRANT_API

///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to walk through the entire path (UTF-16 string) to change directory into it. \n\n
///         This function is the "re-entrant / thread-safe" version of PathAPI__Cd_Path_UTF16() !
///
///@param   utf16_path_str    Pointer to the input UTF-16 path string. \n\n
///
///@param   ext_working_drv   Drive handle of the external 'working DRIVE' for the calling task to work for "re-entrant / thread-safe". \n\n
///
///@retval  FS_SUCCEED           Change directory to the path successfully. \n\n
///@retval  ABNORMAL_STATUS      Failed to change directory to the path. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
///@remark   1. The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   2. For "re-entrant / thread-safe" to be called by multiple tasks concurrently, a 'working DRIVE' handle parameter must be feed by each task to work ! \n\n
///
int PathAPI__Cd_Path_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    BYTE  task_id = TaskGetId();
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
    if (PathAPI__Locate_Path_UTF16_r(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, ext_working_drv, E_PATH_TARGET_IS_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("task ID %d: %s: Error! path/directory cannot be located due to some error !", task_id, __FUNCTION__);
        }
        else
        {
            MP_ALERT("task ID %d: %s: -I- target directory of the path not found.", task_id, __FUNCTION__);
        }
        return ABNORMAL_STATUS;
    }
    else
    {
        MP_DEBUG("task ID %d: %s: locate path OK, and changed to the path !", task_id, __FUNCTION__);
    }

    return FS_SUCCEED;
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to implement directory creation function with path supported.\n
///         If any midway directory and/or the final directory in the path does not exist, this function will create it and enter it. \n
///         If the midway directories and the final directory in the path already exist, this function will just enter it. \n\n
///         This function is the "re-entrant / thread-safe" version of PathAPI__MakeDir_UTF16() !
///
///@param   utf16_path_str   The UTF-16 path string of a directory to be created. \n\n
///
///@param   ext_working_drv  Drive handle of the external 'working DRIVE' for the calling task to work for "re-entrant / thread-safe". \n\n
///
///@retval  FS_SUCCEED         Create the subdirectory successfully. \n\n
///@retval  ABNORMAL_STATUS    Failed to create the subdirectory. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
///@remark   1. The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   2. If any midway directory and/or the final directory in the path does not exist, this function will create it and enter it. \n
///          If the midway directories and the final directory in the path already exist, this function will just enter it. \n\n
///
///@remark   3. For "re-entrant / thread-safe" to be called by multiple tasks concurrently, a 'working DRIVE' handle parameter must be feed by each task to work ! \n\n
///
int PathAPI__MakeDir_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    BOOL   dir_found = FALSE;
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
    if (PathAPI__Locate_Path_UTF16_r(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, ext_working_drv, E_PATH_TARGET_IS_DIR, E_AUTO_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("task ID %d: %s: Error! path/directory cannot be located due to some error!", task_id, __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    }
    else
    {
        MP_ALERT("task ID %d: %s: locate path OK.", task_id, __FUNCTION__);
        dir_found = TRUE;
    }

    return FS_SUCCEED;
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to delete a directory and all files/subdirectories within it with path supported. \n
///         This function is the "re-entrant / thread-safe" version of PathAPI__RemoveDir_UTF16() !
///
///@param   utf16_path_str   The UTF-16 path string of a directory to be deleted. \n\n
///
///@param   ext_working_drv  Drive handle of the external 'working DRIVE' for the calling task to work for "re-entrant / thread-safe". \n\n
///
///@retval  FS_SUCCEED          Delete successfully. \n\n
///@retval  ABNORMAL_STATUS     Delete unsuccessfully. \n\n
///@retval  FILE_NOT_FOUND      Directory not found, no need to delete it. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
///@remark   1. The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   2. For "re-entrant / thread-safe" to be called by multiple tasks concurrently, a 'working DRIVE' handle parameter must be feed by each task to work ! \n\n
///
int PathAPI__RemoveDir_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    int    ret;
    BYTE   task_id = TaskGetId();
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    WORD   *wUtf16_TempBuf;


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
    if (PathAPI__Locate_Path_UTF16_r(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, ext_working_drv, E_PATH_TARGET_IS_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("task ID %d: %s: Error! path/directory cannot be located due to some error !", task_id, __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("task ID %d: %s: final directory not found in the path !", task_id, __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        MP_DEBUG("task ID %d: %s: final directory found in the path !", task_id, __FUNCTION__);
        ret = CdParent(ext_working_drv);
        if (ret != FS_SUCCEED)
        {
            MP_ALERT("%s: CdParent() failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

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
            MP_ALERT("%s: FileSearchLN() failed to find the final directory !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        MP_ALERT("%s: directory found => delete it ...", __FUNCTION__);
        ret = DeleteDir(ext_working_drv);
        return ret;
    }
}



///
///@ingroup FS_Wrapper
///@brief   This higher-level wrapper function is to get the count of directory entries in a directory specified by the path string. \n
///         This function is the "re-entrant / thread-safe" version of PathAPI__GetEntriesCountInDir() !
///
///@param   utf16_path_str      [IN] The UTF-16 path string of a directory to get the count of directory entries within it. \n\n
///@param   ext_working_drv     [IN] Drive handle of the external 'working DRIVE' for the calling task to work for "re-entrant / thread-safe". \n\n
///@param   entries_count       [OUT] Returned count of directory entries in the directory. \n\n
///
///@retval  FS_SUCCEED          Get the count of directory entries successfully. \n\n
///@retval  ABNORMAL_STATUS     Failed to get the count of directory entries due to some error. \n\n
///@retval  FILE_NOT_FOUND      The specified directory not found, or nothing in the specified directory. \n\n
///
///@remark   Permitted path string format examples: \n
///          [a]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder   (search beginning from CF root directory) \n
///          [b]   CF:/test_folder_1/sub2/longname_folder_level3/ABCD_folder/  (search beginning from CF root directory) \n
///          [c]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder    (search beginning from CF current directory) \n
///          [d]   CF:test_folder_1/sub2/longname_folder_level3/ABCD_folder/   (search beginning from CF current directory) \n
///          [e]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder      (search beginning from current drive root directory) \n
///          [f]   /test_folder_1/sub2/longname_folder_level3/ABCD_folder/     (search beginning from current drive root directory) \n
///          [g]   test_folder_1/sub2/longname_folder_level3/ABCD_folder       (search beginning from current drive current directory) \n
///          [h]   test_folder_1/sub2/longname_folder_level3/ABCD_folder/      (search beginning from current drive current directory) \n\n
///
///@remark   1. The path string must be converted to UTF-16 before calling this function. \n\n
///
///@remark   2. For "re-entrant / thread-safe" to be called by multiple tasks concurrently, a 'working DRIVE' handle parameter must be feed by each task to work ! \n\n
///
///@remark   3. The count of directory entries in the directory excludes the "." and ".." directory entries if exist.
///
int PathAPI__GetEntriesCountInDir_UTF16_r(const WORD * utf16_path_str, DRIVE * ext_working_drv, DWORD * entries_count)
{
    E_DRIVE_INDEX_ID  new_drv_id;
    BYTE   task_id = TaskGetId();
    PATH_TARGET_TYPE  found_entry_type;
    DWORD  final_token_offset, final_token_strlen;
    int    ret;


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

    if (entries_count == NULL)
    {
        MP_ALERT("task ID %d: %s: Error ! NULL pointer !", task_id, __FUNCTION__);
        return ABNORMAL_STATUS;
    }
    *entries_count = 0;

    /* analyze the drive and directory location */
    if (PathAPI__Locate_Path_UTF16_r(utf16_path_str, &new_drv_id, &found_entry_type, &final_token_offset, &final_token_strlen, ext_working_drv, E_PATH_TARGET_IS_DIR, E_NOT_CREATE_MIDWAY_DIR) != FS_SUCCEED)
    {
        if ((new_drv_id == NULL_DRIVE) || (final_token_offset == 0xFFFFFFFF) || (final_token_strlen == 0)) /* fatal error */
        {
            MP_ALERT("task ID %d: %s: Error! path/directory cannot be located due to some error !", task_id, __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            MP_DEBUG("task ID %d: %s: final directory not found in the path !", task_id, __FUNCTION__);
            return FILE_NOT_FOUND;
        }
    }
    else
    {
        MP_DEBUG("task ID %d: %s: final directory found in the path !", task_id, __FUNCTION__);

        ret = DirFirst(ext_working_drv);
        if (ret == END_OF_DIR)
        {
            MP_ALERT("task ID %d: %s: No file or sub-dir in the current directory !", task_id, __FUNCTION__);
            MP_ALERT("task ID %d: %s: count of directory entries in the sub-directory = %u \r\n", task_id, __FUNCTION__, (*entries_count));
            return FILE_NOT_FOUND;
        }
        else if (ret != FS_SUCCEED)
        {
            MP_ALERT("task ID %d: %s: DirFirst() failed !", task_id, __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        else
        {
            do
            {
        #if EXFAT_ENABLE
                if (ext_working_drv->Flag.FsType == FS_TYPE_exFAT) /* note: exFAT directories have no '.' and '..' entries */
                    (*entries_count)++;
                else
        #endif
                {
                    if (ext_working_drv->Node->Name[0] == '.') /* if '.' or '..' then bypass */
                        continue;
                    else
                        (*entries_count)++;
                }
            } while (DirNext(ext_working_drv) == FS_SUCCEED);
        }

        MP_ALERT("task ID %d: %s: count of directory entries in the sub-directory = %u \r\n", task_id, __FUNCTION__, (*entries_count));
        return FS_SUCCEED;
    }
}

#endif //FS_REENTRANT_API



#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup DIRECTORY
///@brief   Update the CHAIN structure information of the current directory on the specified drive.
///
///@param   drv      The drive to update its current directory info.
///
///@retval  FS_SUCCEED         Update successfully. \n\n
///@retval  ABNORMAL_STATUS    Update unsuccessfully. \n\n
///@retval  INVALID_DRIVE      Invalid drive.
///
#endif
int UpdateChain(DRIVE * drv)
{
    CHAIN *dir;
    CHAIN update_dir;
    DWORD dir_size;
    BOOL f_chain_fragmented;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
    MpMemCopy((BYTE *) &update_dir, (BYTE *) dir, sizeof(CHAIN));

    if (drv->Flag.FsType != FS_TYPE_exFAT)
        f_chain_fragmented = TRUE;
#if EXFAT_ENABLE
    else
    {
        if (dir->exfat_DirEntryFlags.f_NoFatChain) /* chain is not fragmented */
            f_chain_fragmented = FALSE;
        else
        {
            if (! SystemCardPresentCheck(drv->DrvIndex))
            {
                MP_ALERT("%s: Card not present !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }

            if (drv->FatRead(drv, dir->Start) == 0) /* chain is not fragmented */
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
        dir_size = dir->Size; /* cannot get total size by traversing FAT => use kept info in DirStack */
    else
        dir_size = GetChainTotalSizeByTraverseFatTable(drv, dir->Start);

    dir->Start = update_dir.Start;
    dir->Current = update_dir.Current;
    dir->Point = update_dir.Point;
    dir->Size = dir_size;

    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Enter the subdirectory.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED                   Enter the subdirectory successfully. \n\n
///@retval  ABNORMAL_STATUS              Change directory unsuccessfully. \n\n
///@retval  INVALID_DRIVE                Invalid drive. \n\n
///@retval  FS_DIR_STACK_LIMIT_REACHED   Depth limit of Drive's DirStack buffer has been reached, cannot enter the subdirectory.
///
///@remark  The subdirectory to enter is implicitly pointed to by current node (drv->Node) before calling this function.
///
int CdSub(DRIVE * drv)
{
    CHAIN *dir;
    volatile FDB *node;
    DWORD start, size;
    WORD i = 0;


    MP_DEBUG("Enter %s() ...", __FUNCTION__);
    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    /* for root directory, special process to keep its chain size in the DirStack entry [0] */
    if (!drv->DirStackPoint) /* in RootDir */
    {
        dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);

        size = GetRootDirSize(drv);

        dir->Size = size;
    #if EXFAT_ENABLE
        dir->exfat_DirEntryFlags.f_NoFatChain = 0; /* exFAT has non-zero cluster number value in the FAT entries for root directory */
    #endif
    }

    node = (volatile FDB *) (drv->Node);

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        if (!(node->Attribute & FDB_SUB_DIR))
        {
            MP_ALERT("%s: Error! Current FDB node is not a directory !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        /* check to avoid DirStack buffer overflow */
        if (drv->DirStackPoint >= (DIR_STACK_DEEP - 1) * sizeof(CHAIN))
        {
            MP_ALERT("%s: -I- DirStack buffer limit is reached !", __FUNCTION__);
            MP_DEBUG("%s: drv->DirStackPoint = %lu, sizeof(CHAIN) = %lu, DIR_STACK_DEEP = %lu", __FUNCTION__, drv->DirStackPoint, sizeof(CHAIN), DIR_STACK_DEEP);
            return FS_DIR_STACK_LIMIT_REACHED;
        }
        else
            drv->DirStackPoint += sizeof(CHAIN);

        dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
    #if EXFAT_ENABLE
        dir->exfat_DirEntryFlags.f_NoFatChain = 0; /* FAT12/16/32 always need to query FAT table content */
    #endif
        start = (LoadAlien16((void *) &node->StartHigh) << 16) + LoadAlien16((void *) &node->StartLow);

        size = GetChainTotalSizeByTraverseFatTable(drv, start);
        if (size == 0)
        {
            MP_ALERT("%s: GetChainTotalSizeByTraverseFatTable() failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    }
#if EXFAT_ENABLE
    else
    {
    #if 0  /* We should use GetFileAttributeAndTimeInfoFromFDB() to get file attributes */
        /* Be careful !  File attributes, filename, ... etc. of current DirEntry are not always cached in drv->CacheBufPoint->XXXX cache buffer */
        /* These file-related info are cached during DirNext() & DirPrevious() & ScanFileName() ...etc. */
        if (!(drv->CacheBufPoint->fileAttributes & EXFAT_FDB_ATTRIB_DIR))
        {
            MP_ALERT("%s: Error! Current FDB node is not a directory !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    #else
        DIR_ENTRY_INFO_TYPE  dir_entry_info;
        MpMemSet((BYTE *) &dir_entry_info, 0, sizeof(DIR_ENTRY_INFO_TYPE));
        if (GetFileAttributeAndTimeInfoFromFDB(drv, (FDB *) drv->Node, &dir_entry_info) != PASS)
        {
            MP_ALERT("%s: Error! GetFileAttributeAndTimeInfoFromFDB() failed !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }

        if (! dir_entry_info.fileAttribute.f_SubDir)
        {
            MP_ALERT("%s: Error! Current DirEntry is not a directory !", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    #endif

        /* check to avoid DirStack buffer overflow */
        if (drv->DirStackPoint >= (DIR_STACK_DEEP - 1) * sizeof(CHAIN))
        {
            MP_ALERT("%s: -I- DirStack buffer limit is reached !", __FUNCTION__);
            MP_DEBUG("%s: drv->DirStackPoint = %lu, sizeof(CHAIN) = %lu, DIR_STACK_DEEP = %lu", __FUNCTION__, drv->DirStackPoint, sizeof(CHAIN), DIR_STACK_DEEP);
            return ABNORMAL_STATUS;
        }
        else
            drv->DirStackPoint += sizeof(CHAIN);

        dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);

        dir->exfat_DirEntryFlags.f_NoFatChain = dir_entry_info.exfat_DirEntryFlags.f_NoFatChain;
        start = (DWORD) dir_entry_info.startCluster;
        size = (DWORD) dir_entry_info.fileSize;
    }
#endif

    ChainInit(dir, start, size);

    if (DirCaching(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! DirCaching() failed !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Exit the current directory and return to its parent directory.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED         Change directory successfully. \n\n
///@retval  ABNORMAL_STATUS    Change directory unsuccessfully. \n\n
///@retval  INVALID_DRIVE      Invalid drive.
///
int CdParent(DRIVE * drv)
{
    WORD i = 0;
    DWORD start, cluster, size, point;
    CHAIN *dir;


    MP_DEBUG("Enter %s() ...", __FUNCTION__);
    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (!drv->DirStackPoint) // at root directory
        return FS_SUCCEED;

    drv->DirStackPoint -= sizeof(CHAIN);

#if EXFAT_ENABLE
    if ((drv->DirStackPoint != 0) || ((drv->Flag.FsType == FS_TYPE_FAT32) || (drv->Flag.FsType == FS_TYPE_exFAT)))
#else
    if ((drv->DirStackPoint != 0) || (drv->Flag.FsType == FS_TYPE_FAT32))
#endif
    {
        dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
        point =  dir->Point;
        start = dir->Start;
        size = dir->Size; /* kept chain size of the directory in the DirStack */

        ChainInit(dir, start, size);
        ChainSeekForward(drv, dir, point);
    }

    if (DirCaching(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Create a subdirectory in current directory.
///
///@param   drv           The drive to access. \n\n
///@param   name          The pointer of new directory name primary part, which is an ASCII string. \n\n
///@param   extension     The pointer of new directory name extension part, which is an ASCII string.
///
///@retval  FS_SUCCEED         Create the subdirectory successfully. \n\n
///@retval  DISK_FULL          Create the subdirectory unsuccessfully due to drive is full. \n\n
///@retval  DISK_READ_ONLY     Create the subdirectory unsuccessfully due to the drive is Read-Only. \n\n
///@retval  ABNORMAL_STATUS    Create the subdirectory unsuccessfully due to other errors. \n\n
///@retval  INVALID_DRIVE      Invalid drive.
///
///@remark  The function call create one new subdirectory in current directory.
///         But the point is still in current directory, must call CdSub() to enter the new subdirectory.
///
int MakeDir(DRIVE * drv, BYTE * name, BYTE * extension)
{
    FDB *sNode, sTempNode;
    DWORD dwAddr;
    CHAIN *dir;

    ST_SYSTEM_TIME  sys_time;
    WORD file_date, file_time;
    DWORD lba_of_start_clus;

    LONG_NAME *sLongName;
    DWORD dwNameLength, dwFdbCount, dwRemainFdbCount, dwNumericTail, dwNumericTailLength, i, j;
    BYTE bTempBuf1[32]; //enough buffer for 13 UTF-16 words in a long name FDB. One more word in buffer for easy string processing
    BYTE bChkSum;
    BYTE bShortNameBuf[12], bExt[4]; //for 8.3 format short name. One more byte in each buffer for easy string processing
    BYTE *bLongNameBuf, *bTempBuf;
    BYTE *pbTempBufPtr;
    BOOL blLossyFlag;
    BYTE NameL, ExtenL;
    extern const SBYTE ReplaceGlyph[];


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
        MP_ALERT("-E %s() malloc fail", __FUNCTION__);
        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

    bTempBuf = (BYTE *) ker_mem_malloc(MAX_L_NAME_LENG, TaskGetId());
    if (bTempBuf == NULL)
    {
        MP_ALERT("-E %s() malloc fail", __FUNCTION__);
        ker_mem_free(bLongNameBuf);
        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

#if FS_REENTRANT_API
    Set_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    dwAddr = DriveNewClusGet(drv);
    if (dwAddr == 0xffffffff)
    {
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

    sNode = (FDB *) (drv->Node);
    /* note: add pointer check for safety ! Because MpMemSet() may cause "break 100" when memory buffer was already released first by Card Out, for example. */
    if ((BYTE *) sNode != NULL)
        MpMemSet(sNode, 0, sizeof(FDB));

    for (i = 0; i < 11; i++)
        sNode->Name[i] = bShortNameBuf[i];

    if (sNode->Name[0] == 0xE5)
        sNode->Name[0] = 0x05;

    /* fill in the create date/time and modified date */

    SystemTimeGet(&sys_time);
    file_date = FileSetDate_for_FATxx(sys_time.u16Year, sys_time.u08Month, sys_time.u08Day);
    file_time = FileSetTime_for_FATxx(sys_time.u08Hour, sys_time.u08Minute, sys_time.u08Second);
    sNode->CreateTime = file_time;
    sNode->CreateDate = file_date;
    sNode->AccessDate = file_date;
    sNode->ModifyTime = file_time;
    sNode->ModifyDate = file_date;
    MP_DEBUG("date/time info: %04d-%02d-%02d %02d:%02d:%02d", sys_time.u16Year, sys_time.u08Month, sys_time.u08Day, sys_time.u08Hour, sys_time.u08Minute, sys_time.u08Second);
    MP_DEBUG("==> file_date = 0x%0x;  file_time = 0x%0x", file_date, file_time);

    sNode->Attribute = FDB_SUB_DIR;

    // fill in the start cluster
    SaveAlien16(&sNode->StartLow, dwAddr);
    SaveAlien16(&sNode->StartHigh, dwAddr >> 16);

    MpMemCopy(&sTempNode, sNode, sizeof(FDB));

    // reset the new sub-directory's cluster
    lba_of_start_clus = drv->DataStart + ((dwAddr - 2) << drv->ClusterExp);
    if (SectorClear(drv, lba_of_start_clus, drv->ClusterSize) != FS_SUCCEED)
    {
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

    // get the current name
    ScanFileName(drv);

    // set drv->Flag.DirCacheChanged flag, then DirCaching() will write back cache data
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

    if (CdSub(drv) != FS_SUCCEED)
    {
        ker_mem_free(bLongNameBuf);
        ker_mem_free(bTempBuf);

#if FS_REENTRANT_API
        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;	// change to the new sub-directory
    }

    sNode = &sTempNode;
    MpMemSet(sNode, 0x20, 11);
    sNode->Name[0] = '.';
    MpMemCopy((BYTE *) drv->DirCacheBuffer, (BYTE *) sNode, sizeof(FDB));

    sNode->Name[1] = '.';

    // if the parent dir that '..' point to is root then it's value must be 0
    // else point to the parent directory's start cluster
    if (drv->DirStackPoint - sizeof(CHAIN))
    {
        dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint - sizeof(CHAIN));
        DWORD start_clus = dir->Start;
        SaveAlien16(&sNode->StartLow, start_clus);
        start_clus >>= 16;
        SaveAlien16(&sNode->StartHigh, start_clus);
    }
    else
    {
        sNode->StartLow = 0;
        sNode->StartHigh = 0;
    }

    MpMemCopy((BYTE *) (((BYTE *)drv->DirCacheBuffer) + sizeof(FDB)), (BYTE *) sNode, sizeof(FDB));
    drv->DirCachePoint = lba_of_start_clus;
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

    if (CdParent(drv) != FS_SUCCEED)
    {
        ker_mem_free(bLongNameBuf);
        ker_mem_free(bTempBuf);

#if FS_REENTRANT_API
        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

    // force to write back cached data if any
    if (DriveRefresh(drv) != FS_SUCCEED)
    {
        ker_mem_free(bLongNameBuf);
        ker_mem_free(bTempBuf);

#if FS_REENTRANT_API
        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

#if FS_REENTRANT_API
    Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

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
///@ingroup DIRECTORY
///@brief   Create a subdirectory in current directory with specified UTF-16 Unicode filename. \n
///         This function is the UTF-16 Unicode version of the MakeDir() function.
///
///@param   drv           The drive to access. \n\n
///@param   name          The pointer of new directory name, which is an UTF-16 Unicode string with 0x0000 null terminator. \n\n
///
///@retval  FS_SUCCEED         Create the subdirectory successfully. \n\n
///@retval  DISK_FULL          Create the subdirectory unsuccessfully due to drive is full. \n\n
///@retval  DISK_READ_ONLY     Create the subdirectory unsuccessfully due to the drive is Read-Only. \n\n
///@retval  ABNORMAL_STATUS    Create the subdirectory unsuccessfully due to other errors. \n\n
///@retval  INVALID_DRIVE      Invalid drive.
///
///@remark  The function call create one new subdirectory in current directory.
///         But the point is still in current directory, must call CdSub() to enter the new subdirectory.
///
int MakeDir_UTF16(DRIVE * drv, WORD * name)
{
    FDB *sNode, sTempNode;
    DWORD dwAddr;
    CHAIN *dir;
    DWORD ret;

    ST_SYSTEM_TIME  sys_time;
    WORD file_date, file_time;
    DWORD lba_of_start_clus;

    LONG_NAME *sLongName;
    DWORD dwNameLength, dwFdbCount, dwRemainFdbCount, dwNumericTail, dwNumericTailLength, i, j;
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

    wLongNameBuf = (WORD *) ker_mem_malloc(MAX_L_NAME_LENG * 2, TaskGetId());
    if (wLongNameBuf == NULL)
    {
        MP_ALERT("-E %s() malloc fail", __FUNCTION__);
        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

    wTempNameBuf = (WORD *) ker_mem_malloc(MAX_L_NAME_LENG * 2, TaskGetId());
    if (wTempNameBuf == NULL)
    {
        MP_ALERT("-E %s() malloc fail", __FUNCTION__);
        ker_mem_free(wLongNameBuf);
        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

    ori_filenameL = StringLength16(name);

    MpMemSet(ext_word, 0, EXT_NAME_LEN * 2);
    MpMemSet(bExt, 0, EXT_NAME_LEN);
    MpMemSet(bTempExt, 0, EXT_NAME_LEN);

    MpMemSet(wTempNameBuf, 0, MAX_L_NAME_LENG * 2);
    MpMemCopy((BYTE *) wTempNameBuf, (BYTE *) name, (ori_filenameL + 1) * 2); //including tailing 0x0000 terminator

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
        {
            ext_word[i] = ((WORD *) tmp_word)[i];
        }
    }

    ori_primary_nameL = StringLength16(wTempNameBuf);
    ori_ext_name = (ori_ext_nameL > 0)? ((WORD *) ((WORD *) name + ori_primary_nameL + 1)) : NULL;

    // convert UTF-16 extension name to ASCII for 8.3 format short name to retain file extension type (ex: "JPG") in short name FDB
    mpx_UtilU16ToU08(bTempExt, ext_word);
    ExtenL = StringLength08(bTempExt);

#if FS_REENTRANT_API
    Set_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Set_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    dwAddr = DriveNewClusGet(drv);
    if (dwAddr == 0xffffffff)
    {
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
    ret = FileSearchLN(drv, name, final_nameL, E_BOTH_FILE_AND_DIR_TYPE);
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

    BYTE  priname_len_limit, valid_char_count_in_priname = 0;
    BYTE  *bTmp = (BYTE *) wLongNameBuf;
    WORD  *wTmp = (WORD *) wLongNameBuf;
    for (i=0; i < (final_nameL * 2); i++)
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

    if (valid_char_count_in_priname <= 8) /* no need to reserve 2-byte space for "~1" when doing first time FileSearch() */
    {
        priname_len_limit = 8;

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

        if (FileSearch(drv, bShortNameBuf, bExt, E_BOTH_FILE_AND_DIR_TYPE) != FS_SUCCEED)
        {
            if (StringLength08(bExt) > 0)
                StringNCopy08(&bShortNameBuf[8], bExt, 3);
            else
                MpMemSet(&bShortNameBuf[8], 0x20, 3);

            bShortNameBuf[11] = 0x00;
            goto L_do_folder_checksum;
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

L_do_folder_checksum:
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

        MP_DEBUG("%s(): space in the directory not enough => ChainExtending()...", __FUNCTION__);
        ret = ChainExtending(drv, dir);
        if (ret != FS_SUCCEED)
        {
            MP_ALERT("%s(): ChainExtending() failed! => return ret=0x%x;", __FUNCTION__, ret);
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

    sNode = (FDB *) (drv->Node);
    /* note: add pointer check for safety ! Because MpMemSet() may cause "break 100" when memory buffer was already released first by Card Out, for example. */
    if ((BYTE *) sNode != NULL)
        MpMemSet(sNode, 0, sizeof(FDB));

    for (i = 0; i < 11; i++)
        sNode->Name[i] = bShortNameBuf[i];

    if (sNode->Name[0] == 0xE5)
        sNode->Name[0] = 0x05;

    /* fill in the create date/time and modified date */

    SystemTimeGet(&sys_time);
    file_date = FileSetDate_for_FATxx(sys_time.u16Year, sys_time.u08Month, sys_time.u08Day);
    file_time = FileSetTime_for_FATxx(sys_time.u08Hour, sys_time.u08Minute, sys_time.u08Second);
    sNode->CreateTime = file_time;
    sNode->CreateDate = file_date;
    sNode->AccessDate = file_date;
    sNode->ModifyTime = file_time;
    sNode->ModifyDate = file_date;
    MP_DEBUG("date/time info: %04d-%02d-%02d %02d:%02d:%02d", sys_time.u16Year, sys_time.u08Month, sys_time.u08Day, sys_time.u08Hour, sys_time.u08Minute, sys_time.u08Second);
    MP_DEBUG("==> file_date = 0x%0x;  file_time = 0x%0x", file_date, file_time);

    sNode->Attribute = FDB_SUB_DIR;

    // fill in the start cluster
    SaveAlien16(&sNode->StartLow, dwAddr);
    SaveAlien16(&sNode->StartHigh, dwAddr >> 16);

    MpMemCopy(&sTempNode, sNode, sizeof(FDB));

    // reset the new sub-directory's cluster
    lba_of_start_clus = drv->DataStart + ((dwAddr - 2) << drv->ClusterExp);
    if (SectorClear(drv, lba_of_start_clus, drv->ClusterSize) != FS_SUCCEED)
    {
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

    // get the current name
    ScanFileName(drv);

    // set drv->Flag.DirCacheChanged flag, then DirCaching() will write back cache data
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

    if (CdSub(drv) != FS_SUCCEED)
    {
        ker_mem_free(wLongNameBuf);
        ker_mem_free(wTempNameBuf);

#if FS_REENTRANT_API
        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;	// change to the new sub-directory
    }

    sNode = &sTempNode;
    MpMemSet(sNode, 0x20, 11);
    sNode->Name[0] = '.';
    MpMemCopy((BYTE *) drv->DirCacheBuffer, sNode, sizeof(FDB));

    sNode->Name[1] = '.';

    // if the parent dir that '..' point to is root then it's value must be 0
    // else point to the parent directory's start cluster
    if (drv->DirStackPoint - sizeof(CHAIN))
    {
        dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint - sizeof(CHAIN));
        DWORD start_clus = dir->Start;
        SaveAlien16(&sNode->StartLow, start_clus);
        start_clus >>= 16;
        SaveAlien16(&sNode->StartHigh, start_clus);
    }
    else
    {
        sNode->StartLow = 0;
        sNode->StartHigh = 0;
    }

    MpMemCopy((BYTE *) (((BYTE *)drv->DirCacheBuffer) + sizeof(FDB)), sNode, sizeof(FDB));
    drv->DirCachePoint = lba_of_start_clus;
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

    if (CdParent(drv) != FS_SUCCEED)
    {
        ker_mem_free(wLongNameBuf);
        ker_mem_free(wTempNameBuf);

#if FS_REENTRANT_API
        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

    // force to write back cached data if any
    if (DriveRefresh(drv) != FS_SUCCEED)
    {
        ker_mem_free(wLongNameBuf);
        ker_mem_free(wTempNameBuf);

#if FS_REENTRANT_API
        Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

        SemaphoreRelease(FILE_WRITE_SEMA_ID);
        return ABNORMAL_STATUS;
    }

#if FS_REENTRANT_API
    Reset_UpdatingFAT_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Reset_UpdatingFAT_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

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
///@ingroup DIRECTORY
///@brief   Delete a subdirectory and all files in it.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED          Delete successfully. \n\n
///@retval  ABNORMAL_STATUS     Delete unsuccessfully. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
///@remark  The subdirectory to delete is implicitly pointed to by current node (drv->Node) before calling this function.
///         The function call will delete the subdirectory and all files in the subdirectory. After the deletion,
///         the dir->Point of the working directory is at the node deleted.
///
int DeleteDir(DRIVE * drv)
{
    int ret;
    volatile FDB *node;
    DWORD stack;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    CdSub(drv);
    stack = drv->DirStackPoint;
    ScanFileName(drv);
    ret = FS_SUCCEED;

    while (ret == FS_SUCCEED)
    {
        node = (volatile FDB *) (drv->Node);
        if (node->Name[0] == '.');	// skip '.' and '..' entry
        else if (node->Name[0] == 0xe5);	// skip deleted entry
        else if (node->Attribute == 0x0f);
        else if (node->Attribute & FDB_LABEL);	// skip disk label entry
        else if (node->Attribute & FDB_SUB_DIR)
        {
            if (CdSub(drv) != FS_SUCCEED)
                return ABNORMAL_STATUS;
        }
        else if (Unlink(drv) != FS_SUCCEED)
            return ABNORMAL_STATUS;

        if (ret == FS_SUCCEED)
        {
            ret = NextNode(drv);
            ScanFileName(drv);

            while (ret == END_OF_DIR && drv->DirStackPoint > stack)
            {
                if (CdParent(drv) != FS_SUCCEED)
                    return ABNORMAL_STATUS;
                if (Unlink(drv) != FS_SUCCEED)
                    return ABNORMAL_STATUS;

                ret = NextNode(drv);

                if (fileUiUpdateIconAniPtr)
                    fileUiUpdateIconAniPtr();
            }
        }
    }

    if (CdParent(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    if (Unlink(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    if (DriveRefresh(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Delete all content (all files and sub-folders) within a directory but retain the directory itself.
///         This function is different from API DeleteDir() which also deletes the directory itself.
///
///@param   drv      The drive to access.
///
///@retval  PASS          Delete content of the directory successfully. \n\n
///@retval  FAIL          Delete content of the directory unsuccessfully. \n\n
///
///@remark    The subdirectory to delete all its content is implicitly pointed to by current node (drv->Node) before calling this function.
///
///@remark    After the deletion, this function will restore the current node (drv->Node) to the original FDB position of the directory.
///
int DeleteAllContentWithinDir(DRIVE * drv)
{
    int ret;
    CHAIN *dir;
    DWORD dir_size = 0;
    ST_SAVE_FDB_POS  fdb_pos_info;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    /* keep original FDB position info of the directory */
    SaveCurrentDirNodePosition(drv, &fdb_pos_info);

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        if (drv->Node->Attribute & FDB_SUB_DIR)
        {
            if (CdSub(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! CdSub() failed !", __FUNCTION__);

                /* restore the current FDB node position to the node position previously stored */
                RestoreDirNodePosition(drv, &fdb_pos_info);

                return FAIL;
            }

            ret = DirFirst(drv);
            if (ret == END_OF_DIR)
            {
                MP_ALERT("%s: No file or sub-dir in the directory.", __FUNCTION__);
                ret = PASS;
                goto L_restore_ori_dir_and_position;
            }
            else if (ret != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! DirFirst() failed !", __FUNCTION__);
                ret = FAIL;
                goto L_restore_ori_dir_and_position;
            }

            do
            {
                ret = DeleteFileOrFolderOfCurrentFDB(drv);
                if (ret != PASS)
                {
                    MP_ALERT("%s: Error! DeleteFileOrFolderOfCurrentFDB() failed !", __FUNCTION__);
                    ret = FAIL;
                    goto L_restore_ori_dir_and_position;
                }
            } while (DirNext(drv) == FS_SUCCEED);

L_restore_ori_dir_and_position:
            /* go back to original FDB position of the directory */
            if (CdParent(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: Error! CdParent() failed !", __FUNCTION__);
                return FAIL;
            }

            /* restore the current FDB node position to the node position previously stored */
            RestoreDirNodePosition(drv, &fdb_pos_info);

            return ret;
        }
        else
        {
            MP_ALERT("%s: Error! Current entry is not a directory !", __FUNCTION__);
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



///
///@ingroup DIRECTORY
///@brief   Refresh content of the internal directory cache buffer of the drive for status of the specified directory.
///
///@param   drv      The drive to refresh its directory content cache. \n\n
///@param   dir      The CHAIN pointer of a specified directory.
///
///@retval  FS_SUCCEED           Refresh successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL    Refresh unsuccessfully due to drive access failed. \n\n
///@retval  ABNORMAL_STATUS      Refresh unsuccessfully due to some error.
///
///@remark  The sector point on the storage to be cached to buffer is specified in the CHAIN structure of the specified directory. \n\n
///
int DirCaching_for_DirChain(DRIVE * drv, CHAIN * dir)
{
    DWORD sector;


    if ((drv == NULL) || (dir == NULL))
    {
        MP_ALERT("%s: Error! NULL handle !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

#if FS_REENTRANT_API
    BOOL f_Is_ext_working_drv;
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);
    BYTE curr_task_id = TaskGetId();

    /* check whether if this drive handle is an external working drive copy */
    f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;
#endif

    sector = ChainGetLba(drv, dir);
    if (sector == 0xFFFFFFFF)
    {
        MP_ALERT("%s: -I-  EOC reached and at cluster boundary => out of chain range.", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    // if cache has been modified then write it back
    if (drv->Flag.Present && drv->Flag.DirCacheChanged)
    {
/* note: must sync Dir cache buffer content before DriveWrite() I/O because DriveWrite() will cause task context switch !! */
#if FS_REENTRANT_API
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        {
            CHAIN *dir_of_working_drv, *dir_of_ori_drv;

            dir_of_working_drv = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
            dir_of_ori_drv = (CHAIN *) ((BYTE *) ori_drv_in_drive_table->DirStackBuffer + ori_drv_in_drive_table->DirStackPoint);

            /* sync Dir cache buffer content if in same working directory */
            if (dir_of_working_drv->Start == dir_of_ori_drv->Start)  //in same working directory
            {
                /* note: here is "write" case, we directly update the DirCacheBuffer and DirCachePoint of the ori drive to sync with working drive ! */
                /* sync: copy from working drive to ori drive */
                MpMemCopy((BYTE *) ori_drv_in_drive_table->DirCacheBuffer, (BYTE *) drv->DirCacheBuffer, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT);
                ori_drv_in_drive_table->DirCachePoint = drv->DirCachePoint;

                /* sync: copy from working drive to all other tasks' working drives w.r.t same storage drive if same directory is cached */
                TASK_WORKING_DRIVE_TYPE *list_ptr = Tasks_WorkingDrv_List;
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
        }
#endif

        if (DriveWrite(drv, (BYTE *) drv->DirCacheBuffer, drv->DirCachePoint, 1) != FS_SUCCEED)
            return DRIVE_ACCESS_FAIL;

        drv->Flag.DirCacheChanged = 0;
    }

    if (sector != drv->DirCachePoint)
    {
#if FS_REENTRANT_API
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        {
            if (sector == ori_drv_in_drive_table->DirCachePoint)
            {
                /* note: Because DriveRead()/DriveWrite() will make task context switch, we always update cache of ori drive prior to disk write.
                 *       So, we can now copy cache content from ori drive for latest updated info in multi-task concurrent accesses case !
                 */
                /* sync: copy from ori drive to working drive */
                MpMemCopy((BYTE *) drv->DirCacheBuffer, (BYTE *) ori_drv_in_drive_table->DirCacheBuffer, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT);
            }
            else
            {
                /* note: we always read/cache one sector directory content into DirCacheBuffer when DirCaching() ! */
                if (DriveRead(drv, (BYTE *) drv->DirCacheBuffer, sector, 1) != FS_SUCCEED)
                    return DRIVE_ACCESS_FAIL;
            }
        }
        else
#endif
        {
            /* note: we always read/cache one sector directory content into DirCacheBuffer when DirCaching() ! */
            if (DriveRead(drv, (BYTE *) drv->DirCacheBuffer, sector, 1) != FS_SUCCEED)
                return DRIVE_ACCESS_FAIL;
        }

        drv->Flag.DirCacheChanged = 0;
        drv->DirCachePoint = sector; // update dir cache lba
    }

    drv->Node = (FDB *) ((BYTE *) drv->DirCacheBuffer + (dir->Point & ((1 << drv->bSectorExp) - 1)));
    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Refresh content of the internal directory cache buffer of the drive for status of current directory.
///
///@param   drv      The drive to refresh its directory content cache.
///
///@retval  FS_SUCCEED           Refresh successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL    Refresh unsuccessfully due to drive access failed. \n\n
///@retval  ABNORMAL_STATUS      Refresh unsuccessfully due to some error.
///
///@remark  The sector point on the storage to be cached to buffer is specified in the CHAIN structure of current directory. \n\n
///
int DirCaching(DRIVE * drv)
{
    CHAIN *dir;
    DWORD sector;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

#if FS_REENTRANT_API
    BOOL f_Is_ext_working_drv;
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);
    BYTE curr_task_id = TaskGetId();

    /* check whether if this drive handle is an external working drive copy */
    f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;
#endif

    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
    sector = ChainGetLba(drv, dir);
    if (sector == 0xFFFFFFFF)
    {
        MP_ALERT("%s: -I-  EOC reached and at cluster boundary => out of chain range.", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    // if cache has been modified then write it back
    if (drv->Flag.Present && drv->Flag.DirCacheChanged)
    {
/* note: must sync Dir cache buffer content before DriveWrite() I/O because DriveWrite() will cause task context switch !! */
#if FS_REENTRANT_API
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        {
            CHAIN *dir_of_working_drv, *dir_of_ori_drv;

            dir_of_working_drv = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
            dir_of_ori_drv = (CHAIN *) ((BYTE *) ori_drv_in_drive_table->DirStackBuffer + ori_drv_in_drive_table->DirStackPoint);

            /* sync Dir cache buffer content if in same working directory */
            if (dir_of_working_drv->Start == dir_of_ori_drv->Start)  //in same working directory
            {
                /* note: here is "write" case, we directly update the DirCacheBuffer and DirCachePoint of the ori drive to sync with working drive ! */
                /* sync: copy from working drive to ori drive */
                MpMemCopy((BYTE *) ori_drv_in_drive_table->DirCacheBuffer, (BYTE *) drv->DirCacheBuffer, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT);
                ori_drv_in_drive_table->DirCachePoint = drv->DirCachePoint;

                /* sync: copy from working drive to all other tasks' working drives w.r.t same storage drive if same directory is cached */
                TASK_WORKING_DRIVE_TYPE *list_ptr = Tasks_WorkingDrv_List;
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
        }
#endif

        if (DriveWrite(drv, (BYTE *) drv->DirCacheBuffer, drv->DirCachePoint, 1) != FS_SUCCEED)
            return DRIVE_ACCESS_FAIL;

        drv->Flag.DirCacheChanged = 0;
    }

    if (sector != drv->DirCachePoint)
    {
#if FS_REENTRANT_API
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        {
            if (sector == ori_drv_in_drive_table->DirCachePoint)
            {
                /* note: Because DriveRead()/DriveWrite() will make task context switch, we always update cache of ori drive prior to disk write.
                 *       So, we can now copy cache content from ori drive for latest updated info in multi-task concurrent accesses case !
                 */
                /* sync: copy from ori drive to working drive */
                MpMemCopy((BYTE *) drv->DirCacheBuffer, (BYTE *) ori_drv_in_drive_table->DirCacheBuffer, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT);
            }
            else
            {
                /* note: we always read/cache one sector directory content into DirCacheBuffer when DirCaching() ! */
                if (DriveRead(drv, (BYTE *) drv->DirCacheBuffer, sector, 1) != FS_SUCCEED)
                    return DRIVE_ACCESS_FAIL;
            }
        }
        else
#endif
        {
            /* note: we always read/cache one sector directory content into DirCacheBuffer when DirCaching() ! */
            if (DriveRead(drv, (BYTE *) drv->DirCacheBuffer, sector, 1) != FS_SUCCEED)
                return DRIVE_ACCESS_FAIL;
        }

        drv->Flag.DirCacheChanged = 0;
        drv->DirCachePoint = sector; // update dir cache lba
    }

    drv->Node = (FDB *) ((BYTE *) drv->DirCacheBuffer + (dir->Point & ((1 << drv->bSectorExp) - 1)));
    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Refresh content of the internal directory cache buffer of the drive for the specified file.
///
///@param   sHandle      The specified file to refresh drive's directory content cache to cover it.
///
///@retval  FS_SUCCEED           Refresh successfully. \n\n
///@retval  DRIVE_ACCESS_FAIL    Refresh unsuccessfully due to drive access failed. \n\n
///@retval  ABNORMAL_STATUS      Refresh unsuccessfully due to some error.
///
int FileDirCaching(STREAM * sHandle)
{
    DRIVE *drv;

    if (sHandle == NULL)
        return ABNORMAL_STATUS;

#if (RAM_FILE_ENABLE)
    /* special processing for "RamFile" type files */
    if (sHandle->Flag.f_IsRamFile)
    {
        MP_ALERT("%s: this operation is not supported for \"RamFile\" type files !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
#endif

    /* special processing for "RamFile" type files: whose 'Drv' field in STREAM structure must be 0 */
    if (! sHandle->Flag.f_IsRamFile)
    {
        if (sHandle->Drv == NULL)
            return ABNORMAL_STATUS; // this file has been closed
    }

    drv = (DRIVE *) (sHandle->Drv);

    if (sHandle->DirSector != drv->DirCachePoint)
    {
        // if cache has been modified then write it back
        if (drv->Flag.DirCacheChanged)
        {
            if (DriveWrite(drv, (BYTE *) drv->DirCacheBuffer, drv->DirCachePoint, 1) != FS_SUCCEED)
                return DRIVE_ACCESS_FAIL;
        }

        if (DriveRead(drv, (BYTE *) drv->DirCacheBuffer, sHandle->DirSector, 1) != FS_SUCCEED)
        {
            return DRIVE_ACCESS_FAIL;
        }
        drv->Flag.DirCacheChanged = 0;
        drv->DirCachePoint = sHandle->DirSector; // update dir cache lba
    }

    drv->Node = (FDB *) ((BYTE *) drv->DirCacheBuffer + (sHandle->FdbOffsetInDirCache << 5));
    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Reset/change the working directory and the current FDB node of the specified drive to the root directory and its first FDB node.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED          Reset/change successfully. \n\n
///@retval  ABNORMAL_STATUS     Reset/change unsuccessfully, for some abnormal case. \n\n
///@retval  DRIVE_ACCESS_FAIL   Reset/change unsuccessfully. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
int DirReset(DRIVE * drv)
{
    DWORD start, size;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

#if USBOTG_HOST_PTP
    if ((drv->DevID == DEV_USB_HOST_PTP) || (drv->DevID == DEV_USBOTG1_HOST_PTP)) //(drv->DevID == DEV_USB_HOST_PTP)
        return FS_SUCCEED;
#endif

#if (NETWARE_ENABLE ||USBOTG_WEB_CAM)
    if((drv->DevID == DEV_USB_WIFI_DEVICE)    ||\
       (drv->DevID == DEV_CF_ETHERNET_DEVICE) ||\
       (drv->DevID == DEV_USB_WEBCAM))
    {
        return FS_SUCCEED;
    }
#endif

    drv->Flag.DirCacheChanged = 0;
    drv->DirStackPoint = 0;
    drv->DirCachePoint = 0xffffffff;
    drv->Flag.LongNameHit = 0;

#if EXFAT_ENABLE
    if ((drv->Flag.FsType == FS_TYPE_FAT32) || (drv->Flag.FsType == FS_TYPE_exFAT))
#else
    if (drv->Flag.FsType == FS_TYPE_FAT32)
#endif
    {
        size = 0;
        start = drv->RootStart; //note: for FAT32 and exFAT, drv->RootStart is cluster number of the first cluster of root directory

        while (start != 0xffffffff)
        {
            if (! SystemCardPresentCheck(drv->DrvIndex))
            {
                MP_ALERT("%s: Card not present !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }

            size += drv->ClusterSize;
            start = drv->FatRead(drv, start);

            if (drv->StatusCode == FS_SCAN_FAIL)
            {
                MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }

            if (!start)
            {
                MP_ALERT("%s: Error! cluster number in the chain of root directory == 0 !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
        }

        start = drv->RootStart;
        size = size << drv->bSectorExp; //byte size
        MP_DEBUG("%s: Root Dir chain size = %lu", __FUNCTION__, size);
    }
    else
    {
        //note: for FAT12/16, drv->RootStart is lba of first sector of root directory
        size = (drv->DataStart - drv->RootStart) << drv->bSectorExp; // byte size
        start = 0;
    }

    ChainInit((CHAIN *) (drv->DirStackBuffer), start, size);

    if (DirCaching(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! DirCaching() failed !", __FUNCTION__);
        return DRIVE_ACCESS_FAIL;
    }

    return FS_SUCCEED;
}



/////////////////////////////////////////////////////////////////////////////
//         Follows function calls are for node access                      //
/////////////////////////////////////////////////////////////////////////////


///
///@ingroup DIRECTORY
///@brief   Move to the first node in current directory.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED          Move successfully. \n\n
///@retval  ABNORMAL_STATUS     Move unsuccessfully. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
int FirstNode(DRIVE * drv)
{
    CHAIN *dir;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (!drv->Flag.Present)
        return ABNORMAL_STATUS;

    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
    ChainSeekSet(dir);

    if (DirCaching(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Move to previous node in current directory.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED        Move successfully. \n\n
///@retval  BEGIN_OF_DIR      Move unsuccessfully, already at the first node. \n\n
///@retval  ABNORMAL_STATUS   Move unsuccessfully. \n\n
///@retval  INVALID_DRIVE     Invalid drive.
///
int PreviousNode(DRIVE * drv)
{
    CHAIN *dir;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (!drv->Flag.Present)
        return ABNORMAL_STATUS;

#if FS_REENTRANT_API
    BOOL f_Is_ext_working_drv;
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);
    BYTE curr_task_id = TaskGetId();

    /* check whether if this drive handle is an external working drive copy */
    f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;

    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
    {
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

        /* sync Dir cache buffer content if in same working directory and same DirCachePoint */
        if (dir_of_working_drv->Start == dir_of_ori_drv->Start)  //in same working directory
        {
            /* note: this works because we always read/cache one sector directory content into DirCacheBuffer when DirCaching() ! */
            if (drv->DirCachePoint == ori_drv_in_drive_table->DirCachePoint)
            {
                /* sync: copy from ori drive to working drive */
                MpMemCopy((BYTE *) drv->DirCacheBuffer, (BYTE *) ori_drv_in_drive_table->DirCacheBuffer, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT);
                drv->DirCachePoint = ori_drv_in_drive_table->DirCachePoint;
            }
        }
    }
#endif

    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);

    // if at the first node then return 0
    if (!dir->Point)
        return BEGIN_OF_DIR;

    if (ChainSeek(drv, dir, dir->Point - sizeof(FDB)) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    if (DirCaching(drv) == FS_SUCCEED)
        return FS_SUCCEED;

    return ABNORMAL_STATUS;
}



///
///@ingroup DIRECTORY
///@brief   Move to next node in current directory.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED        Move successfully. \n\n
///@retval  END_OF_DIR        Move unsuccessfully, already at the last node. \n\n
///@retval  ABNORMAL_STATUS   Move unsuccessfully. \n\n
///@retval  OUT_OF_RANGE      Move unsuccessfully, out of range of the directory's cluster chain. \n\n
///@retval  INVALID_DRIVE     Invalid drive.
///
int NextNode(DRIVE * drv)
{
    CHAIN *dir;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (!drv->Flag.Present)
        return ABNORMAL_STATUS;

#if FS_REENTRANT_API
    BOOL f_Is_ext_working_drv;
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);
    BYTE curr_task_id = TaskGetId();

    /* check whether if this drive handle is an external working drive copy */
    f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;

    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
    {
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

        /* sync Dir cache buffer content if in same working directory and same DirCachePoint */
        if (dir_of_working_drv->Start == dir_of_ori_drv->Start)  //in same working directory
        {
            /* note: this works because we always read/cache one sector directory content into DirCacheBuffer when DirCaching() ! */
            if (drv->DirCachePoint == ori_drv_in_drive_table->DirCachePoint)
            {
                /* sync: copy from ori drive to working drive */
                MpMemCopy((BYTE *) drv->DirCacheBuffer, (BYTE *) ori_drv_in_drive_table->DirCacheBuffer, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT);
                drv->DirCachePoint = ori_drv_in_drive_table->DirCachePoint;
            }
        }
    }
#endif

    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);

    if (!drv->Node)
        return END_OF_DIR;

    // if (Name[0] == 0) means there is no subsequent entries after this FDB node in the directory
    if (!drv->Node->Name[0])
        return END_OF_DIR;

    // if at the end of chain then quit
    if (dir->Point + sizeof(FDB) == dir->Size)
        return END_OF_DIR;

    if (dir->Point + sizeof(FDB) > dir->Size)
        return OUT_OF_RANGE;

    if (ChainSeek(drv, dir, dir->Point + sizeof(FDB)) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    if (DirCaching(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    // if (Name[0] == 0) means there is no subsequent entries after this FDB node in the directory
    if (!drv->Node->Name[0])
        return END_OF_DIR;

    return FS_SUCCEED;
}



#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup DIRECTORY
///@brief   Get one free node.
///
///@param   drv      The drive to access.
///
///@return  FS_SUCCEED         Get new node successfully. \n\n
///@return  ABNORMAL_STATUS    Cannot find free node. \n\n
///@retval  INVALID_DRIVE      Invalid drive.
///
///@remark   The function call will extend one cluster for subdirectory if no free node can get.
///
#endif
int GetNewNode(DRIVE * drv)
{
    CHAIN *dir;
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

#if 0 /* Disable this due to already add into lower-layer functions NextNode() and PreviousNode() */
//#if FS_REENTRANT_API
    BOOL f_Is_ext_working_drv;
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);
    BYTE curr_task_id = TaskGetId();

    /* check whether if this drive handle is an external working drive copy */
    f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;

    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
    {
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

        /* sync Dir cache buffer content if in same working directory and same DirCachePoint */
        if (dir_of_working_drv->Start == dir_of_ori_drv->Start)  //in same working directory
        {
            /* note: this works because we always read/cache one sector directory content into DirCacheBuffer when DirCaching() ! */
            if (drv->DirCachePoint == ori_drv_in_drive_table->DirCachePoint)
            {
                /* sync: copy from ori drive to working drive */
                MpMemCopy((BYTE *) drv->DirCacheBuffer, (BYTE *) ori_drv_in_drive_table->DirCacheBuffer, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT);
                drv->DirCachePoint = ori_drv_in_drive_table->DirCachePoint;
            }
        }
    }
#endif

    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);

    // go to the end of current DIR
    do
    {
        ret = NextNode(drv);
        if (ret == ABNORMAL_STATUS)
            return ABNORMAL_STATUS;
    } while (ret == FS_SUCCEED);

    if (drv->Node->Name[0])
    {
        // if first byte is not zero mean at the last sector of last cluster, so need to
        // get a new cluster, add it into the current dir cluster-chain
        // and clear all it's content

        ret = ChainExtending(drv, dir);
        if (ret != FS_SUCCEED)
            return ret;

        NextNode(drv);

        if (DirCaching(drv) != FS_SUCCEED)
            return ABNORMAL_STATUS;
    }

    return FS_SUCCEED;
}



//Bit 31 Setting to 0 mean system use Deleted FDB
void CheckDeletedCount(DWORD dwCount)
{
    g_dwDeleteFDBCount = (dwCount+1) | 0x80000000;
    //mpDebugPrint("DeletedCount %x", g_dwDeleteFDBCount);
}



#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup DIRECTORY
///@brief   Move to next deleted node.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED        Move successfully. \n\n
///@retval  END_OF_DIR        Move unsuccessfully, already at the last node of the directory. \n\n
///@retval  ABNORMAL_STATUS   Move unsuccessfully. \n\n
///@retval  OUT_OF_RANGE      Move unsuccessfully, out of range of the directory's cluster chain. \n\n
///@retval  INVALID_DRIVE     Invalid drive.
///
#endif
int NextDeletedNode(DRIVE * drv)
{
    CHAIN *dir;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (!drv->Flag.Present)
        return ABNORMAL_STATUS;

    if (!g_dwDeleteFDBCount)
        return ABNORMAL_STATUS;

/* Flow of NextDeletedNode() differs from GetNewNode() which bases on lower-layer function NextNode() => still need this. To be checked ~~ */
#if FS_REENTRANT_API
    BOOL f_Is_ext_working_drv;
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);
    BYTE curr_task_id = TaskGetId();

    /* check whether if this drive handle is an external working drive copy */
    f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;

    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
    {
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

        /* sync Dir cache buffer content if in same working directory and same DirCachePoint */
        if (dir_of_working_drv->Start == dir_of_ori_drv->Start)  //in same working directory
        {
            /* note: this works because we always read/cache one sector directory content into DirCacheBuffer when DirCaching() ! */
            if (drv->DirCachePoint == ori_drv_in_drive_table->DirCachePoint)
            {
                /* sync: copy from ori drive to working drive */
                MpMemCopy((BYTE *) drv->DirCacheBuffer, (BYTE *) ori_drv_in_drive_table->DirCacheBuffer, sizeof(FDB) * DIR_CACHE_IN_FDB_COUNT);
                drv->DirCachePoint = ori_drv_in_drive_table->DirCachePoint;
            }
        }
    }
#endif

    if (g_dwDeleteFDBCount & 0x80000000)
    {
        g_dwDeleteFDBCount &= 0x7FFFFFFF;
        BYTE Count = (g_dwDeleteFDBCount-1);
        BYTE Count1 = Count;
        DWORD Count2 = 0;

        FirstNode(drv);
        do
        {
            dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);

            // if at the end of chain then quit
            if (dir->Point + sizeof(FDB) == dir->Size)
                return END_OF_DIR;

            if (dir->Point + sizeof(FDB) > dir->Size)
                return OUT_OF_RANGE;

            if (ChainSeek(drv, dir, dir->Point + sizeof(FDB)) != FS_SUCCEED)
                return ABNORMAL_STATUS;

            if (DirCaching(drv) != FS_SUCCEED)
                return ABNORMAL_STATUS;

            Count2++;

            // if Name[0]=0xE5 mean this is deleted node
            if (drv->Node->Name[0] == 0xE5)
            {
                if (Count1 == Count)
                    g_dwDeletedNodeCount = Count2;
                Count1--;
            }
            else
                Count1 = Count;
        } while(Count1 != 0);
    }

    FirstNode(drv);
    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
    if (ChainSeek(drv, dir, dir->Point + (sizeof(FDB) * g_dwDeletedNodeCount)) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    if (DirCaching(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    g_dwDeletedNodeCount++;
    g_dwDeleteFDBCount--;

    return FS_SUCCEED;
}



#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup DIRECTORY
///@brief   Get one deleted node and move to it.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED        Move successfully. \n\n
///@retval  END_OF_DIR        Move unsuccessfully, already at the last node of the directory. \n\n
///@retval  ABNORMAL_STATUS   Move unsuccessfully. \n\n
///@retval  OUT_OF_RANGE      Move unsuccessfully, out of range of the directory's cluster chain. \n\n
///@retval  INVALID_DRIVE     Invalid drive.
///
///@remark  This function actually calls NextDeletedNode() to work.
///
#endif
int GetDeletedNode(DRIVE * drv)
{
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    ret = NextDeletedNode(drv);

    if (ret != FS_SUCCEED)
        g_dwDeleteFDBCount = 0;

    return ret;
}



///
///@ingroup DIRECTORY
///@brief   Move to first available file or sub-dir in the current directory.
///
///@param   drv      The drive to access.
///
///@return  FS_SUCCEED         Move successfully. \n\n
///@return  END_OF_DIR         No file or sub-dir in the current directory. \n\n
///@return  ABNORMAL_STATUS    Move unsuccessfully. \n\n
///@retval  INVALID_DRIVE      Invalid drive.
///
///@remark    Finding the first available file or sub-dir in the current directory of the specified drive,
///           the current node point will move to the found node or at the end of the directory if file not found.
///
int DirFirst(DRIVE * drv)
{
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

#if USBOTG_HOST_PTP
    if ((drv->DevID == DEV_USB_HOST_PTP) || (drv->DevID == DEV_USBOTG1_HOST_PTP)) //(drv->DevID == DEV_USB_HOST_PTP)
        return FS_SUCCEED;
#endif

#if NETWARE_ENABLE
    if ((drv->DevID == DEV_USB_WIFI_DEVICE) || (drv->DevID == DEV_CF_ETHERNET_DEVICE))
        return FS_SUCCEED;
#endif

    FirstNode(drv);

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        FDB *node = (FDB *) (drv->Node);
        if (!node->Name[0])
            return END_OF_DIR;

        while (ScanFileName(drv) == FILENAME_SCAN_CONTINUE)
        {
            ret = NextNode(drv);
            if (ret != FS_SUCCEED)
                return ret;
        }
    }
#if EXFAT_ENABLE
    else
    {
        ret = DirNext(drv);
        return ret;
    }
#endif

    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Move to next available file or sub-dir in the current directory.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED          Move successfully. \n\n
///@retval  ABNORMAL_STATUS     Move unsuccessfully. \n\n
///@retval  END_OF_DIR          No more file or sub-dir in the current directory. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
///@remark    Finding next available file or sub-dir in the current directory of the specified drive,
///           the current node point will move to the found node or at the end of the directory if file not found.
///
int DirNext(DRIVE * drv)
{
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        do
        {
            ret = NextNode(drv);
            if (ret != FS_SUCCEED)
                return ret;
        } while (ScanFileName(drv) == FILENAME_SCAN_CONTINUE);
    }
#if EXFAT_ENABLE
    else
    {
        TYPE_exFAT_Generic_DirEntry_Template *exfat_node;

        /* check if current drv->Node is a "File DirectoryEntry" of exFAT. This is for meeting DirFirst() */
        (TYPE_exFAT_File_DirEntry *) exfat_node = (TYPE_exFAT_File_DirEntry *) (drv->Node);
        if (exfat_node->entryType == EXFAT_ENTRY_TYPE_FILE) /* starting DirectoryEntry of a 'file or directory' */
        {
            ret = FS_SUCCEED;
            goto L_file_directory_entries_processing;
        }

        do
        {
            ret = NextNode(drv);
            if (ret != FS_SUCCEED)
                return ret;
        } while ((ret = ScanFileName(drv)) == FILENAME_SCAN_CONTINUE);

L_file_directory_entries_processing:
        if (ret == FS_SUCCEED)
        {
            /* current drv->Node should be a "File DirectoryEntry" of exFAT */
            (TYPE_exFAT_File_DirEntry *) exfat_node = (TYPE_exFAT_File_DirEntry *) (drv->Node);
            if (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE)
            {
                MP_ALERT("%s: Error! Current node should be a (File DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
                return ABNORMAL_STATUS;
            }
            else
            {
                /* keep current file/directory info for later SearchInfo reference */
                drv->CacheBufPoint->fileAttributes =  (WORD) LoadAlien16(&((TYPE_exFAT_File_DirEntry *) exfat_node)->fileAttributes);
                drv->CacheBufPoint->createTimestamp = (DWORD) ((TYPE_exFAT_File_DirEntry *) exfat_node)->createTimestamp; /* keep it little-endian */
                drv->CacheBufPoint->lastModifyTimestamp = (DWORD) ((TYPE_exFAT_File_DirEntry *) exfat_node)->lastModifyTimestamp; /* keep it little-endian */
                drv->CacheBufPoint->createUtcOffset = (BYTE) ((TYPE_exFAT_File_DirEntry *) exfat_node)->createUtcOffset;
                drv->CacheBufPoint->lastModifyUtcOffset = (BYTE) ((TYPE_exFAT_File_DirEntry *) exfat_node)->lastModifyUtcOffset;
            }

            ret = NextNode(drv);
            if (ret != FS_SUCCEED)
                return ret;

            /* current drv->Node should be a "Stream Extension DirectoryEntry" of exFAT */
            (TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node = (TYPE_exFAT_Stream_Extension_DirEntry *) (drv->Node);
            if (exfat_node->entryType != EXFAT_ENTRY_TYPE_STREAM_EXT)
            {
                MP_ALERT("%s: Error! Current node should be a (Stream Extension DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
                return ABNORMAL_STATUS;
            }

            if (((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->generalSecondaryFlags & EXFAT_DirEntryFlags_MASK_AllocPossible)
                drv->CacheBufPoint->exfat_DirEntryFlags.f_AllocPossible = 1;
            else
                drv->CacheBufPoint->exfat_DirEntryFlags.f_AllocPossible = 0;

            if (((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->generalSecondaryFlags & EXFAT_DirEntryFlags_MASK_NoFatChain)
                drv->CacheBufPoint->exfat_DirEntryFlags.f_NoFatChain = 1;
            else
                drv->CacheBufPoint->exfat_DirEntryFlags.f_NoFatChain = 0;

            BYTE name_len, fileName_FdbCount;
            WORD name_hash, *wCh_src, *wCh_dst;
            BYTE counted_name_len = 0, i, j;

            name_len = (BYTE) ((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->nameLength;
            name_hash = (WORD) LoadAlien16(&(((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->nameHash));
            if (name_len % EXFAT_ENTRY_FILE_NAME_LEN)
                fileName_FdbCount = (name_len / EXFAT_ENTRY_FILE_NAME_LEN) + 1;
            else
                fileName_FdbCount = (name_len / EXFAT_ENTRY_FILE_NAME_LEN);

            /* keep current file/directory info for later SearchInfo reference */
            drv->CacheBufPoint->LongNameFdbCount = fileName_FdbCount;
            drv->CacheBufPoint->nameLength = (BYTE) ((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->nameLength;
            drv->CacheBufPoint->nameHash = (WORD) LoadAlien16(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->nameHash);
            drv->CacheBufPoint->firstCluster = (DWORD) LoadAlien32(&((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->firstCluster);

            BYTE *Hi32_of_64, *Lo32_of_64;
            Lo32_of_64 = (BYTE *) &((TYPE_exFAT_Stream_Extension_DirEntry *) exfat_node)->dataLength; /* due to little-endian */
            Hi32_of_64 = (BYTE *) (((BYTE *) Lo32_of_64) + 4); /* due to little-endian */
            drv->CacheBufPoint->dataLength = (U64) ((U64) LoadAlien32(Hi32_of_64) << 32) + (U64) LoadAlien32(Lo32_of_64);

            MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2);
            for (i = 0; (i < fileName_FdbCount) && (counted_name_len < name_len); i++)
            {
                ret = NextNode(drv);
                if (ret != FS_SUCCEED)
                {
                    MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2);
                    return ret;
                }

                /* current drv->Node should be a "File Name DirectoryEntry" of exFAT */
                (TYPE_exFAT_File_Name_DirEntry *) exfat_node = (TYPE_exFAT_File_Name_DirEntry *) (drv->Node);
                if (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE_NAME)
                {
                    MP_ALERT("%s: Error! Current node should be a (File Name DirectoryEntry), but entryType = 0x%02x", __FUNCTION__, exfat_node->entryType);
                    MpMemSet((BYTE *) drv->LongName, 0, MAX_L_NAME_LENG * 2);
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

        #if 1 //just for debug purpose
            {
                BYTE tmpBuf[512];
                MpMemSet(tmpBuf, 0, 512);
                mpx_UtilU16ToU08(tmpBuf, (WORD *) drv->LongName); /* convert UTF-16 to UTF-8 string */
                MP_DEBUG("%s: Filename of entry = %s  (name len = %d characters)", __FUNCTION__, tmpBuf, name_len);
            }
        #endif

            /* go back to the "Stream Extension DirectoryEntry" position of this entry */
            for (j = 0; j < fileName_FdbCount; j++)
            {
                ret = PreviousNode(drv);
                if (ret != FS_SUCCEED)
                {
                    MP_ALERT("%s: PreviousNode() failed ! Cannot go back to Stream Extension DirectoryEntry position of this entry !", __FUNCTION__);
                    return ret;
                }
            }

            return FS_SUCCEED;
        }
        else
            return ret;
    }
#endif

    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Move to previous file or sub-dir in the current directory.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED        Move successfully. \n\n
///@retval  BEGIN_OF_DIR      Move unsuccessfully, already at the first node. \n\n
///@retval  ABNORMAL_STATUS   Move unsuccessfully. \n\n
///@retval  INVALID_DRIVE     Invalid drive.
///
///@remark    Finding previous available file or sub-dir in the current directory of the specified drive,
///           the current node point will move to the found node or at the start of the directory if file not found.
///
int DirPrevious(DRIVE * drv)
{
    BYTE first, attr;
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        do
        {
            /* if no next node can read then return fail */
            if (PreviousNode(drv) != FS_SUCCEED)
                return BEGIN_OF_DIR;

            first = drv->Node->Name[0];
            attr = drv->Node->Attribute;

            /* check if the node is a valid file or sub-dir */
        } while (first == 0xe5 || first == 0x2e || (attr & FDB_LABEL) || attr == 0x0f);

        if (PreviousNode(drv) == BEGIN_OF_DIR)
            ret = DirFirst(drv);
        else
        {
            ret = FS_SUCCEED;
            while (drv->Node->Attribute == 0x0f && ret == FS_SUCCEED)
                ret = PreviousNode(drv);

            if (ret == FS_SUCCEED)
                ret = DirNext(drv);
            else
                ret = DirFirst(drv);
        }
    }
#if EXFAT_ENABLE
    else
    {
        TYPE_exFAT_Generic_DirEntry_Template *exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) (drv->Node);

        /* check if current drv->Node position is a part of a file or directory and not its first DirEntry */
        if ((exfat_node->entryType == EXFAT_ENTRY_TYPE_STREAM_EXT) || (exfat_node->entryType == EXFAT_ENTRY_TYPE_FILE_NAME))
        {
            do
            {
                /* if no next node can read then return fail */
                if (PreviousNode(drv) != FS_SUCCEED)
                    return BEGIN_OF_DIR;

                exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) (drv->Node);
                if ((exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE) && (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE_NAME))
                {
                    MP_ALERT("%s: Error! The exFAT file system on the drive has problem! Please use chkdsk.exe to check it !", __FUNCTION__);
                    return ABNORMAL_STATUS;
                }
            } while (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE);
        }

        /* go to find previous EXFAT_ENTRY_TYPE_FILE DirEntry */
        do
        {
            /* if no next node can read then return fail */
            if (PreviousNode(drv) != FS_SUCCEED)
                return BEGIN_OF_DIR;

            exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) (drv->Node);
        } while (exfat_node->entryType != EXFAT_ENTRY_TYPE_FILE);

        /* current drv->Node is already a "File DirectoryEntry" of exFAT */
        ret = DirNext(drv);
    }
#endif

    return ret;
}



///
///@ingroup DIRECTORY
///@brief   Move to the last file or sub-dir in the current directory.
///
///@param   drv      The drive to access.
///
///@retval  FS_SUCCEED          Move successfully. \n\n
///@retval  ABNORMAL_STATUS     Move unsuccessfully. \n\n
///@retval  END_OF_DIR          No more file or sub-dir in the current directory. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
///@remark   Move to the last file or sub-dir in the current directory of the specified drive,
///          the current node point will move to the found node or at the start of the directory if file not found.
///
int DirLast(DRIVE * drv)
{
    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    MP_DEBUG("%s: => DirFirst()...", __FUNCTION__);
    if (DirFirst(drv) == END_OF_DIR)
        return END_OF_DIR;

    while (NextNode(drv) == FS_SUCCEED);

    MP_DEBUG("%s: END_OF_DIR reached => DirPrevious()...", __FUNCTION__);
    DirPrevious(drv);

    return FS_SUCCEED;
}



// scanning down the Directory Entry one by one to extract out the file name
int ScanFileName(DRIVE * drv)
{
    WORD *point16;
    BYTE *name;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (drv->Flag.FsType != FS_TYPE_exFAT)
    {
        FDB *node = (FDB *) (drv->Node);
        LONG_NAME *lnode = (LONG_NAME *) (drv->Node);
        int victim = node->Name[0];

        if (victim == 0x2e || victim == 0xe5) //note: 0x2E == '.' char. (Name[0] == 0xE5) means free/deleted FDB node.
            return FILENAME_SCAN_CONTINUE;	// if '.' or '..' or deleted then bypass
        else if (lnode->Attribute1 == 0x0f && lnode->Attribute2 == 0)
        {
            // if first = 0x4n mean this node is the last node of this long file name, n is its order
            if (victim & 0x40)
            {
                if ((victim - 0x40) > 20) //violate FAT spec (exceed max filename length)
                {
                    MP_ALERT("%s: ### Sequence number of the long-name FDB is too big (%u) => Check filename processing code of file system !!", __FUNCTION__, victim);
                    drv->DirAuxPoint = 0; //reset for safety, avoiding drv->DirAuxPoint become too big and cause subsequent problem
                    drv->CacheBufPoint->LongNameFdbCount = 0; //treat it as short filename
                }
                else  //normal cases
                {
                    drv->DirAuxPoint = (victim - 0x41) * 13;
                    drv->CacheBufPoint->LongNameFdbCount = victim - 0x40;
                }
            }
            else
            {
                if ((drv->DirAuxPoint < 13) || (drv->DirAuxPoint > (255 - 13)))
                {
                    MP_ALERT("%s: ### Something wrong ! (drv->DirAuxPoint = %u) => Check filename processing code of file system !!", __FUNCTION__, drv->DirAuxPoint);
                    drv->DirAuxPoint = 0; //reset for safety, avoiding drv->DirAuxPoint become too big and cause subsequent problem
                }
                else
                    drv->DirAuxPoint -= 13;
            }

            point16 = (WORD *) (drv->LongName + drv->DirAuxPoint);
            point16 = LoadAlienArray16(point16, lnode->Name0, 10);
            point16 = LoadAlienArray16(point16, lnode->Name1, 12);
            point16 = LoadAlienArray16(point16, lnode->Name2, 4);

            if (victim & 0x40)
            {
                *point16 = 0;
                point16 -= 13;
                while (*point16 != 0x0000 && *point16 != 0xffff)
                    point16++;
                *point16 = 0;

                drv->Flag.LongNameHit = 1;
            }

            return FILENAME_SCAN_CONTINUE;
        }
        else if (node->Attribute & FDB_LABEL)
            return FILENAME_SCAN_CONTINUE;	// if this node is a sub-dir then bypass
        else
        {
            if (!drv->Flag.LongNameHit)	// if long filename string not exist then read the short name
            {
                point16 = (WORD *) (drv->LongName);
                name = &node->Name[0];
                for (victim = 0; victim < 8 && *name != 0x20; victim++)
                {
                    *point16 = *name;
                    point16++;
                    name++;
                }

                if (node->Extension[0] != 0x20)
                {
                    *point16 = '.';
                    *point16++;

                    name = &node->Extension[0];
                    for (victim = 0; victim < 3 && *name != 0x20; victim++)
                    {
                        *point16 = *name;
                        point16++;
                        name++;
                    }
                }

                *point16 = 0;		// add a string terminator
                drv->CacheBufPoint->LongNameFdbCount = 0; /* long filename not hit */
            }
            else
            {
                drv->Flag.LongNameHit = 0;
            }

            return FS_SUCCEED;
        }
    }
#if EXFAT_ENABLE
    else
    {
        TYPE_exFAT_Generic_DirEntry_Template *exfat_node;

        exfat_node = (TYPE_exFAT_Generic_DirEntry_Template *) (drv->Node);
        switch (exfat_node->entryType)
        {
            case EXFAT_ENTRY_TYPE_EOD: /* end of directory */
                MP_DEBUG("%s: reach End of Directory !", __FUNCTION__);
                return END_OF_DIR;
                break;

            case EXFAT_ENTRY_TYPE_BITMAP:
                MP_DEBUG("%s: case (Allocation Bitmap DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;

            case EXFAT_ENTRY_TYPE_UPCASE_TABLE:
                MP_DEBUG("%s: case (Up-case Table DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;

            case EXFAT_ENTRY_TYPE_VOL_LABEL:
                MP_DEBUG("%s: case (Volume Label DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;

            case EXFAT_ENTRY_TYPE_FILE:
                MP_DEBUG("%s: case (File DirectoryEntry)", __FUNCTION__);
                TYPE_exFAT_File_DirEntry *info_node = (TYPE_exFAT_File_DirEntry *) exfat_node;

                /* keep current file/directory info for later SearchInfo reference */
                drv->CacheBufPoint->fileAttributes =  (WORD) LoadAlien16(&info_node->fileAttributes);
                drv->CacheBufPoint->createTimestamp = (DWORD) info_node->createTimestamp; /* keep it little-endian */
                drv->CacheBufPoint->lastModifyTimestamp = (DWORD) info_node->lastModifyTimestamp; /* keep it little-endian */
                drv->CacheBufPoint->createUtcOffset = (BYTE) info_node->createUtcOffset;
                drv->CacheBufPoint->lastModifyUtcOffset = (BYTE) info_node->lastModifyUtcOffset;

                return FS_SUCCEED; /* a file or a directory */
                break;

            case EXFAT_ENTRY_TYPE_VOL_GUID:
                MP_DEBUG("%s: case (GUID DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;

            case EXFAT_ENTRY_TYPE_STREAM_EXT:
                MP_DEBUG("%s: case (Stream Extension DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;

            case EXFAT_ENTRY_TYPE_FILE_NAME:
                MP_DEBUG("%s: case (File Name DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;

            case EXFAT_ENTRY_TYPE_CONTINU_INFO_MANAGE:
                MP_DEBUG("%s: case (Continuous Information Manage DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;

            case EXFAT_ENTRY_TYPE_CONTINU_INFO:
                MP_DEBUG("%s: case (Continuous Information DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;

/* note: Observed DirectoryEntry type values using WinHex, not found in exFAT Spec (Ver 3.00 Draft 1.00) from SD Card Association */
#if 1
            case EXFAT_ENTRY_TYPE_FILE_DELETED:
                MP_DEBUG("%s: case (Deleted File DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;

            case EXFAT_ENTRY_TYPE_STREAM_EXT_DELETED:
                MP_DEBUG("%s: case (Deleted Stream Extension DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;

            case EXFAT_ENTRY_TYPE_FILE_NAME_DELETED:
                MP_DEBUG("%s: case (Deleted File Name DirectoryEntry)", __FUNCTION__);
                return FILENAME_SCAN_CONTINUE;
                break;
#endif

            default:
                MP_DEBUG("%s: Unknown DirectoryEntry: entryType = 0x%02x", __FUNCTION__, (BYTE) exfat_node->entryType);
                return FILENAME_SCAN_CONTINUE;
                break;
        }
    }
#endif
}



int LongNameCopy(DRIVE * drv, WORD * longname, FDB * node)
{
    LONG_NAME lnode;
    DWORD count, length, length0;
    WORD *point16;
    BYTE *point08, checksum, i, *j;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if ((longname == NULL) || ((WORD) *longname == 0x0000))
    {
        MP_ALERT("%s: Invalid filename string !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (node == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    // calculate the 8.3 shortname checksum
    point08 = (BYTE *) node;
    checksum = *point08;
    count = 10;
    while (count)
    {
        count--;
        point08++;
        checksum = ((checksum & 1) << 7) | (checksum >> 1);
        checksum += *point08;
    }

    length = StringLength16(longname) + 1;	// including the null terminator
    count = 0;
    while (length > 13)
    {
        count++;
        length -= 13;
    }

    MpMemSet(&lnode, 0xff, sizeof(LONG_NAME));
    lnode.Number = 0x41 + count;
    lnode.Attribute1 = 0x0f;
    lnode.Attribute2 = 0;
    lnode.CheckSum = checksum;
    lnode.res[0] = 0;
    lnode.res[1] = 0;

    point16 = longname + count * 13;
    if (length >= 5)
        length0 = 5;
    else
        length0 = length;

    j = (BYTE *) point16;
    for (i = 0; i < (length0 * 2); i += 2)
    {
        lnode.Name0[i] = j[i + 1];
        lnode.Name0[i + 1] = j[i];
    }

    point16 += length0;
    length -= length0;

    if (length >= 6)
        length0 = 6;
    else
        length0 = length;

    j = (BYTE *) point16;
    for (i = 0; i < (length0 * 2); i += 2)
    {
        lnode.Name1[i] = j[i + 1];
        lnode.Name1[i + 1] = j[i];
    }

    point16 += length0;
    length -= length0;

    if (length)
    {
        j = (BYTE *) point16;
        for (i = 0; i < (length * 2); i += 2)
        {
            lnode.Name2[i] = j[i + 1];
            lnode.Name2[i + 1] = j[i];
        }

        point16 += 2;
    }

    FdbCopy(drv, (FDB *) & lnode);

    while (count)
    {
        lnode.Number = count;
        point16 = longname + (count - 1) * 13;
        j = (BYTE *) point16;
        for (i = 0; i < 10; i += 2)
        {
            lnode.Name0[i] = j[i + 1];
            lnode.Name0[i + 1] = j[i];
        }

        point16 += 5;
        j = (BYTE *) point16;
        for (i = 0; i < 12; i += 2)
        {
            lnode.Name1[i] = j[i + 1];
            lnode.Name1[i + 1] = j[i];
        }

        point16 += 6;
        j = (BYTE *) point16;
        for (i = 0; i < 4; i += 2)
        {
            lnode.Name2[i] = j[i + 1];
            lnode.Name2[i + 1] = j[i];
        }

        point16 += 2;

        FdbCopy(drv, (FDB *) & lnode);
        count--;
    }

    return FS_SUCCEED;
}



#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@ingroup DIRECTORY
///@brief   Copy an external FDB node and move current node position of the drive onto it.
///
///@param   drv           The drive to access. \n\n
///@param   external      The input FDB node to copy.
///
///@retval  FS_SUCCEED        Copy successfully. \n\n
///@retval  DISK_FULL         Copy unsuccessfully due to disk full (no FDB node available). \n\n
///@retval  ABNORMAL_STATUS   Copy unsuccessfully due to disk access problem. \n\n
///@retval  INVALID_DRIVE     Invalid drive.
///
#endif
int FdbCopy(DRIVE * drv, FDB * external)
{
    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    if (external == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (GetNewNode(drv) != FS_SUCCEED)
    {
        if (GetDeletedNode(drv) != FS_SUCCEED)
            return DISK_FULL;
    }

    MpMemCopy((char *) drv->Node, external, sizeof(FDB));
    drv->Flag.DirCacheChanged = 1;

#if FS_REENTRANT_API
    /* FDB nodes are allocated now => force to write Dir cache to device right away !
     * note: this is important to sync Dir cache and FDBs info for concurrent multiple file/folder creation API invoking case.
     */
    if (DirCaching(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E DirCaching() failed !! => return ABNORMAL_STATUS;", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
#endif

    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Free all the allocated clusters of the file or directory pointed to by current FDB node (drv->Node) of the drive.
///         And then mark all the FDB nodes of such file or directory to be deleted nodes.
///
///@param   drv        The drive to unlink the file or directory.
///
///@retval  FS_SUCCEED          Unlink successfully. \n\n
///@retval  ABNORMAL_STATUS     Unlink unsuccessfully. \n\n
///@retval  INVALID_DRIVE       Invalid drive.
///
///@remark  The file or directory to be unlinked/deleted is implicitly specified by the current FDB node (drv->Node)
///         before calling this function.
///
int Unlink(DRIVE * drv)
{
    volatile FDB *node;
    LONG_NAME *lnode;
    CHAIN chain;
    DWORD counter;
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

#if FS_REENTRANT_API
    BOOL f_Is_ext_working_drv;
    DRIVE *ori_drv_in_drive_table = DriveGet(drv->DrvIndex);

    /* check whether if this drive handle is an external working drive copy */
    f_Is_ext_working_drv = (drv != ori_drv_in_drive_table)? TRUE:FALSE;

    Set_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Set_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    node = (volatile FDB *) (drv->Node);
    ChainInit(&chain, (LoadAlien16((void *) (&node->StartHigh)) << 16) + LoadAlien16((void *) &node->StartLow), LoadAlien32((void *) &node->Size));

    if (ChainFree(drv, &chain) != FS_SUCCEED)
    {
        MP_ALERT("%s: ChainFree() failed !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    node->Name[0] = 0xe5;
    drv->Flag.DirCacheChanged = 1;

    counter = 0;
    while (1)
    {
        counter++;

        ret = PreviousNode(drv);
        if (ret == ABNORMAL_STATUS)
        {
#if FS_REENTRANT_API
            Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

            return ABNORMAL_STATUS;
        }
        else if (ret == BEGIN_OF_DIR)
        {
            counter--;
            break;
        }

        node = (FDB *) (drv->Node);
        lnode = (LONG_NAME *) (drv->Node);

        if (lnode->Attribute1 == 0x0f)
            node->Name[0] = 0xe5;
        else
            break;

        drv->Flag.DirCacheChanged = 1;
    }

#if FS_REENTRANT_API
    /* FDB nodes are allocated now => force to write Dir cache to device right away !
     * note: this is important to sync Dir cache and FDBs info for concurrent multiple file/folder creation API invoking case.
     */
    if (DirCaching(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: -E DirCaching() failed !! => return ABNORMAL_STATUS;", __FUNCTION__);

        Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
        if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
            Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);

        return ABNORMAL_STATUS;
    }
#endif

    while (counter)
    {
        if (NextNode(drv) != FS_SUCCEED)
        {
#if FS_REENTRANT_API
            Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
            if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
                Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

            return ABNORMAL_STATUS;
        }
        counter--;
    }

#if FS_REENTRANT_API
    Reset_UpdatingFdbNodes_Status(drv, (BYTE *) __FUNCTION__);
    if (f_Is_ext_working_drv == TRUE) /* the input 'drv' is an external working drive copy */
        Reset_UpdatingFdbNodes_Status(ori_drv_in_drive_table, (BYTE *) __FUNCTION__);
#endif

    return FS_SUCCEED;
}



///
///@ingroup DIRECTORY
///@brief   Save current FDB node (drv->Node) position info into a ST_SAVE_FDB_POS structure variable.
///
///@param   drv            [IN] The drive to save its current FDB node (drv->Node) position info. \n\n
///@param   fdb_pos_info   [OUT] Pointer of a ST_SAVE_FDB_POS structure variable to store the position info.
///
///@retval  PASS       Save the position info successfully. \n\n
///@retval  FAIL       Save the position info unsuccessfully. \n\n
///
SWORD SaveCurrentDirNodePosition(DRIVE * drv, ST_SAVE_FDB_POS * fdb_pos_info)
{ 
    CHAIN *dir;

    if ((drv == NULL) || (fdb_pos_info == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return FAIL;
    }

    MpMemSet((BYTE *) fdb_pos_info, 0, sizeof(ST_SAVE_FDB_POS));
    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
    fdb_pos_info->dwDirStart = dir->Start;
    fdb_pos_info->dwDirPoint = drv->DirCachePoint;
    fdb_pos_info->dwFdbOffset = (dir->Point >> 5);
#if EXFAT_ENABLE
    fdb_pos_info->exfat_DirEntryFlags.f_NoFatChain = dir->exfat_DirEntryFlags.f_NoFatChain;
#endif

    return PASS;
}



///
///@ingroup DIRECTORY
///@brief   Restore the current FDB node (drv->Node) position to the node position previously stored in a ST_SAVE_FDB_POS structure variable.
///
///@param   drv            The drive to restore its FDB node position. \n\n
///@param   fdb_pos_info   Pointer of a ST_SAVE_FDB_POS structure variable storing the position info to be restored.
///
///@retval  PASS       Restore the position info successfully. \n\n
///@retval  FAIL       Restore the position info unsuccessfully. \n\n
///
SWORD  RestoreDirNodePosition(DRIVE * drv, ST_SAVE_FDB_POS * fdb_pos_info)
{
    CHAIN *dir;
    DWORD dir_size;

    if ((drv == NULL) || (fdb_pos_info == NULL))
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return FAIL;
    }

    dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint);
#if EXFAT_ENABLE
    dir->exfat_DirEntryFlags.f_NoFatChain = fdb_pos_info->exfat_DirEntryFlags.f_NoFatChain;

    if (dir->exfat_DirEntryFlags.f_NoFatChain) /* chain is not fragmented */
        dir_size = dir->Size; /* cannot get total size by traversing FAT => use kept info in DirStack */
    else
#endif
    {
        dir_size = GetChainTotalSizeByTraverseFatTable(drv, fdb_pos_info->dwDirStart);
        if (dir_size == 0)
        {
            MP_ALERT("%s: Error! GetChainTotalSizeByTraverseFatTable() failed !", __FUNCTION__);
            return FAIL;
        }
    }

    ChainInit(dir, fdb_pos_info->dwDirStart, dir_size);

    if (ChainSeekForward(drv, dir, fdb_pos_info->dwFdbOffset << 5) != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! ChainSeekForward() failed !", __FUNCTION__);
        return FAIL;
    }

    if (DirCaching_for_DirChain(drv, dir) != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! DirCaching_for_DirChain() failed !", __FUNCTION__);
        return FAIL;
    }

    return PASS;
}



///
///@ingroup DIRECTORY
///@brief   Calculate the length of root directory of the drive.
///
///@param   drv        The drive to access. \n\n
///
///@return  PASS       The length of root directory. \n\n
///
DWORD  GetRootDirSize(DRIVE * drv)
{
    DWORD root_dir_size = 0;

    if ((drv->Flag.FsType == FS_TYPE_FAT12) || (drv->Flag.FsType == FS_TYPE_FAT16))
    {
        /* note: for FAT12/16, drv->RootStart is lba of first sector of root directory */
        root_dir_size = (drv->DataStart - drv->RootStart) << drv->bSectorExp;
    }
    else
        root_dir_size = GetChainTotalSizeByTraverseFatTable(drv, drv->RootStart);

    return root_dir_size;
}



MPX_KMODAPI_SET(PathAPI__Cd_Path);
MPX_KMODAPI_SET(PathAPI__Cd_Path_UTF16);
MPX_KMODAPI_SET(PathAPI__MakeDir);
MPX_KMODAPI_SET(PathAPI__MakeDir_UTF16);
MPX_KMODAPI_SET(PathAPI__RemoveDir);
MPX_KMODAPI_SET(PathAPI__RemoveDir_UTF16);
MPX_KMODAPI_SET(DirReset);
MPX_KMODAPI_SET(DirFirst);
MPX_KMODAPI_SET(DirNext);
MPX_KMODAPI_SET(DirPrevious);
MPX_KMODAPI_SET(CdSub);
MPX_KMODAPI_SET(CdParent);

