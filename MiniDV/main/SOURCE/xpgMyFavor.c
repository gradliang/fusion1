
#define LOCAL_DEBUG_ENABLE   0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "xpgFunc.h"
#include "os.h"
#include "display.h"
#include "devio.h"
#include "taskid.h"
#include "fs.h"
#include "ui.h"
#include "filebrowser.h"
#include "imageplayer.h"
#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

#if FAVOR_HISTORY_SIZE&&MAKE_XPG_PLAYER

extern DWORD g_dwImageRandomHistory[FAVOR_HISTORY_SIZE];
#if AUDIO_ON
extern DWORD g_dwMusicRandomHistory[FAVOR_HISTORY_SIZE];
extern DWORD g_dwMusicFavoriteHistory[FAVOR_HISTORY_SIZE];
#endif
extern DWORD g_dwPicFavoriteHistory[FAVOR_HISTORY_SIZE];
#if VIDEO_ON
extern DWORD g_dwVideoFavoriteHistory[FAVOR_HISTORY_SIZE];
#endif

/* for MyFavor of Photo UI */
DWORD dwMyFavorFileListIndex = 0;
DWORD dwMyFavorFileListCount = 0;
ST_SEARCH_INFO  MyFavorFileList[FAVOR_HISTORY_SIZE];

static ST_SEARCH_INFO  BackupOriFileList[FAVOR_HISTORY_SIZE];
static DWORD dwBackupOriFileListCount;
static DWORD dwSwapFileListCount;

#define FAVOR_PREFIX              "MyFavor"
#define FAVOR_FILENAME_MAX_LEN    32  /* 32 characters should be enough */
#define FAVOR_SET_MIN_INDEX       1
#define FAVOR_SET_MAX_INDEX       4

/* local usage for temp filename buffer of MyFavor setting file */
static BYTE  FavorSetting_filename[FAVOR_FILENAME_MAX_LEN];
static WORD  FavorSetting_UTF16_filename[FAVOR_FILENAME_MAX_LEN];

BYTE xpgGetMyFavorIndex();
void RestoreCurrentSearchInfo();


/*
 *@brief   Build filename of the specified MyFavor setting file.
 *          This function puts returned ASCII and UTF-16 filenames in buffer FavorSetting_filename[] and FavorSetting_UTF16_filename[].
 *
 *@param   favor_set_idx      The index of specified MyFavor photo set.
 *                            Range of valid value is FAVOR_SET_MIN_INDEX ~ FAVOR_SET_MAX_INDEX.
 *
 *@retval  PASS         Build filename of the specified MyFavor setting file successfully.
 *@retval  FAIL         Failed to build filename of the specified MyFavor setting file.
 */
static int FavorSettingFilename(BYTE favor_set_idx)
{
    BYTE  index_str[2] = {0, 0};


    if ((favor_set_idx < FAVOR_SET_MIN_INDEX) || (favor_set_idx > FAVOR_SET_MAX_INDEX))
    {
        MP_ALERT("%s: Invalid favor_set_idx = %d !", __FUNCTION__, favor_set_idx);
        return FAIL;
    }
    sprintf(index_str, "%d", favor_set_idx);

    memset((BYTE *) FavorSetting_filename, 0, FAVOR_FILENAME_MAX_LEN);
    StringCopy08(FavorSetting_filename, FAVOR_PREFIX);
    strcat(FavorSetting_filename, index_str);
    MP_DEBUG("%s: FavorSetting_filename = %s", __FUNCTION__, FavorSetting_filename);

    memset((BYTE *) FavorSetting_UTF16_filename, 0, FAVOR_FILENAME_MAX_LEN * sizeof(WORD));
    mpx_UtilUtf8ToUnicodeU16(FavorSetting_UTF16_filename, FavorSetting_filename); /* convert to UTF-16 string */
    return PASS;
}



///
///@ingroup xpgFuncEntry
///@brief   Load MyFavor photo items info from stored MyFavor setting file, and compare with current photo file list to \n
///         find matched photo items. And then copy matched photo items to MyFavorFileList[] array for GUI display.
///
///@param   favor_set_idx      The index of specified MyFavor photo set. \n
///                            Range of valid value is FAVOR_SET_MIN_INDEX ~ FAVOR_SET_MAX_INDEX. \n\n
///
///@retval  PASS           Load MyFavor setting successfully. \n\n
///@retval  END_OF_DIR     MyFavor setting file of the specified MyFavor set is not found. \n\n
///@retval  FAIL           Failed to load MyFavor setting due to some error. \n\n
///
int LoadFavorPicSetting(BYTE favor_set_idx)
{
    STREAM *sHandle;
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
    DRIVE *drv;
    DWORD file_size, read_len;
    BYTE  *buffer;
    int   ret, i, stored_favor_entries_cnt = 0;
    ST_SEARCH_INFO *list_item;


    if (SYS_DRV_ID != NULL_DRIVE)
        drv = DriveGet(SYS_DRV_ID);
    else
        drv = DriveGet(psSysConfig->sStorage.dwCurStorageId); /* if System Drive not defined, use current drive */

    if ((favor_set_idx < FAVOR_SET_MIN_INDEX) || (favor_set_idx > FAVOR_SET_MAX_INDEX))
    {
        MP_ALERT("%s: Invalid favor_set_idx = %d !", __FUNCTION__, favor_set_idx);
        return FAIL;
    }

    if (FavorSettingFilename(favor_set_idx) == FAIL)
    {
        MP_ALERT("%s: Error! FavorSettingFilename(%d) failed !", __FUNCTION__, favor_set_idx);
        return FAIL;
    }

    /* back to beginning of root directory */
    if (DirReset(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! DirReset() back to beginning of root directory failed !", __FUNCTION__);
        return FAIL;
    }

    ret = FileSearchLN(drv, FavorSetting_UTF16_filename, StringLength16(FavorSetting_UTF16_filename), E_FILE_TYPE);
    if (ret == END_OF_DIR) /* file not found */
    {
        return END_OF_DIR;
    }
    else if (ret == FS_SUCCEED) /* file found */
    {
        sHandle = FileOpen(drv);
        file_size = FileSizeGet(sHandle);
        buffer = (BYTE *) ext_mem_malloc(file_size);
        if (buffer == NULL)
        {
            MP_ALERT("%s: malloc failed for buffer size (%d) !!", __FUNCTION__, file_size);
            return FAIL;
        }

        read_len = FileRead(sHandle, buffer, file_size);
        if (read_len != file_size)
        {
            MP_ALERT("%s: Error! Length of read bytes (=%d) < expected read bytes (= %d) => check storage drive !", __FUNCTION__, read_len, file_size);
            ext_mem_free(buffer);
            return FAIL;
        }

        stored_favor_entries_cnt = file_size/sizeof(ST_SEARCH_INFO);
        MP_DEBUG("%s: count of stored photo entries in MyFavor_%d Set = %d", __FUNCTION__, favor_set_idx, stored_favor_entries_cnt);
    }
    else
    {
        MP_ALERT("%s: Error! FileSearchLN() failed due to some error !", __FUNCTION__);
        return FAIL;
    }

    /* prepare MyFavor of Photo file list */
    dwMyFavorFileListIndex = 0;
    dwMyFavorFileListCount = 0;
    memset((BYTE *) MyFavorFileList, 0, FAVOR_HISTORY_SIZE * sizeof(ST_SEARCH_INFO));

    ST_SEARCH_INFO *favor_item;
    int j;

    for (i=0; (i < stored_favor_entries_cnt); i++)
    {
        int idx_in_OriFileList = -1;

        favor_item = (((ST_SEARCH_INFO *) buffer) + i);

        for (j=0; j < psFileBrowser->dwFileListCount[OP_IMAGE_MODE]; j++)
        {
            list_item = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE]) + j);

            if (favor_item->FsType != FS_TYPE_exFAT)
            {
                /* here, comparing their 8.3 short filenames is enough. No need to use UTF-16 long filename */
                if ( (memcmp(favor_item->bName, list_item->bName, 8) == 0) && (memcmp(favor_item->bExt, list_item->bExt, 4) == 0) &&
                     (favor_item->dwDirStart == list_item->dwDirStart) )
                {
                    idx_in_OriFileList = j;
                    break;
                }
            }
    #if EXFAT_ENABLE
            else
            {
                /* exFAT does not support 8.3 short filename. Use filename hash values to compare */
                if ( (favor_item->exfat_NameHash == list_item->exfat_NameHash) && (favor_item->dwDirStart == list_item->dwDirStart) )
                {
                    idx_in_OriFileList = j;
                    break;
                }
            }
    #endif
        }

        if (idx_in_OriFileList >= 0)
        {
            memcpy((BYTE *) &MyFavorFileList[dwMyFavorFileListCount++], (BYTE *) list_item, sizeof(ST_SEARCH_INFO));
            MP_DEBUG("%s: MyFavor_%d: item[%d] => Matched idx_in_OriFileList = %d", __FUNCTION__, favor_set_idx, i, idx_in_OriFileList);
        }
        else
        {
            MP_DEBUG("%s: MyFavor_%d: item[%d] => Not found in current Photo file list.", __FUNCTION__, favor_set_idx, i);
        }
    }

#if 1 //just for debug purpose
    BYTE   pbBuf[512];
    DWORD  pdwBufferCount;
    ENTRY_FILENAME_TYPE  bFilenameType;

    MP_DEBUG("%s: MyFavor of Photo file list - count = %d", __FUNCTION__, dwMyFavorFileListCount);
    for (i=0; i < dwMyFavorFileListCount; i++)
    {
        memset(pbBuf, 0, 512);
        pdwBufferCount = 0;
        bFilenameType = E_FILENAME_UNKNOWN;

        ret = Locate_DirEntryNode_of_SearchInfo(DriveGet(MyFavorFileList[i].DrvIndex), (ST_SEARCH_INFO *) &MyFavorFileList[i]);
        if ((ret != FS_SUCCEED) && (ret != FILE_NOT_FOUND))
        {
            mpDebugPrint("%s: Locate_DirEntryNode_of_SearchInfo() failed !", __FUNCTION__);
            break;
        }
        else if (ret == FILE_NOT_FOUND) /* here, use FILE_NOT_FOUND for [Upper Folder] case */
        {
            continue;
        }

        ret = GetFilenameOfCurrentNode(DriveGet(MyFavorFileList[i].DrvIndex), &bFilenameType, pbBuf, &pdwBufferCount, 512);
        if (ret != FS_SUCCEED)
        {
            mpDebugPrint("%s: GetFilenameOfCurrentNode() failed !", __FUNCTION__);
            break;
        }

        if (bFilenameType == E_FILENAME_8_3_SHORT_NAME)
        {
            mpDebugPrint("%s: item[%d] in MyFavor file list = %s  (ASCII short name, len = %d bytes)", __FUNCTION__, i, pbBuf, pdwBufferCount);
        }
        else if (bFilenameType == E_FILENAME_UTF16_LONG_NAME)
        {
            BYTE tmpBuf[512];
            memset(tmpBuf, 0, 512);
            mpx_UtilU16ToU08(tmpBuf, (WORD *) pbBuf); /* convert UTF-16 to UTF-8 string */
            mpDebugPrint("%s: item[%d] in MyFavor file list = %s  (UTF-16 long name, len = %d bytes)", __FUNCTION__, i, tmpBuf, pdwBufferCount);
        }
    }
#endif

    ext_mem_free(buffer);
    FileClose(sHandle);
    return PASS;
}



///
///@ingroup xpgFuncEntry
///@brief   Save selected photo items to specified MyFavor set, and store into its setting file.
///
///@param   favor_set_idx      The index of specified MyFavor photo set. \n
///                            Range of valid value is FAVOR_SET_MIN_INDEX ~ FAVOR_SET_MAX_INDEX. \n\n
///
///@retval  PASS         Save MyFavor setting successfully. \n\n
///@retval  FAIL         Failed to save MyFavor setting. \n\n
///
int SaveFavorPicSetting(BYTE favor_set_idx)
{
    STREAM *sHandle;
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
    DRIVE *drv;
    DWORD stored_favor_entries_cnt, selected_favor_entries_cnt = 0;
    DWORD *pdwFavor;
    int   ret, i;


    if (SYS_DRV_ID != NULL_DRIVE)
        drv = DriveGet(SYS_DRV_ID);
    else
        drv = DriveGet(psSysConfig->sStorage.dwCurStorageId); /* if System Drive not defined, use current drive */

    /* reset matched file list w.r.t MyFavor of Photo */
    dwMyFavorFileListIndex = 0;
    dwMyFavorFileListCount = 0;
    memset((BYTE *) MyFavorFileList, 0, FAVOR_HISTORY_SIZE * sizeof(ST_SEARCH_INFO));

    if ((favor_set_idx < FAVOR_SET_MIN_INDEX) || (favor_set_idx > FAVOR_SET_MAX_INDEX))
    {
        MP_ALERT("%s: Invalid favor_set_idx = %d !", __FUNCTION__, favor_set_idx);
        return FAIL;
    }

    if (FavorSettingFilename(favor_set_idx) == FAIL)
    {
        MP_ALERT("%s: Error! FavorSettingFilename(%d) failed !", __FUNCTION__, favor_set_idx);
        return FAIL;
    }

    /* back to beginning of root directory */
    if (DirReset(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! DirReset() back to beginning of root directory failed !", __FUNCTION__);
        return FAIL;
    }

    ret = FileSearchLN(drv, FavorSetting_UTF16_filename, StringLength16(FavorSetting_UTF16_filename), E_FILE_TYPE);
    if (ret == END_OF_DIR) /* file not found */
    {
        MP_DEBUG("%s: MyFavor_%d setting file not found => create a new one ...", __FUNCTION__, favor_set_idx);
        if (CreateFile_UTF16(drv, FavorSetting_UTF16_filename) != FS_SUCCEED)
        {
            MP_ALERT("%s: Error! CreateFile_UTF16() failed !", __FUNCTION__);
            return FAIL;
        }
        sHandle = FileOpen(drv);
        stored_favor_entries_cnt = 0;
    }
    else if (ret == FS_SUCCEED) /* file found */
    {
        sHandle = FileOpen(drv);
        stored_favor_entries_cnt = FileSizeGet(sHandle)/sizeof(ST_SEARCH_INFO);
        MP_DEBUG("%s: count of stored photo entries in MyFavor_%d Set = %d", __FUNCTION__, favor_set_idx, stored_favor_entries_cnt);
    }
    else
    {
        MP_ALERT("%s: Error! FileSearchLN() failed due to some error !", __FUNCTION__);
        return FAIL;
    }

    pdwFavor = &g_dwPicFavoriteHistory[0];
    if (pdwFavor != NULL)
    {
        for (i=0; i < FAVOR_HISTORY_SIZE; i++)
        {
            if (GetBitByOffset(pdwFavor, i) == 0) //selected item
                selected_favor_entries_cnt++;
        }
        MP_DEBUG("%s: count of selected photo entries for MyFavor = %d", __FUNCTION__, selected_favor_entries_cnt);
    }

    if (selected_favor_entries_cnt)
    {
        DWORD file_size, read_len;
        ST_SEARCH_INFO *myFavorBuf = NULL;
        DWORD len_to_write, written_len;
        DWORD final_favor_entries_cnt = stored_favor_entries_cnt;

        if (stored_favor_entries_cnt == 0)
        {
            for (i=0; i < FAVOR_HISTORY_SIZE; i++)
            {
                if (GetBitByOffset(pdwFavor, i) == 0) //selected item
                {
                    DWORD idx_in_OriFileList = i;

                    len_to_write = sizeof(ST_SEARCH_INFO);
                    written_len = FileWrite(sHandle, (BYTE *) (((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE]) + idx_in_OriFileList), len_to_write);
                    if (written_len != len_to_write)
    	            {
                        MP_ALERT("%s: Error! Length of written bytes (=%d) < expected written bytes (= %d) => check storage drive !", __FUNCTION__, written_len, len_to_write);
                        break;
    	            }
                }
            }
        }
        else
        {
            /* load original saved MyFavor setting content to buffer first */
            myFavorBuf = (ST_SEARCH_INFO *) ext_mem_malloc(FAVOR_HISTORY_SIZE * sizeof(ST_SEARCH_INFO));
            if (myFavorBuf == NULL)
            {
                MP_ALERT("%s: malloc failed for buffer size (%d) !!", __FUNCTION__, FAVOR_HISTORY_SIZE * sizeof(ST_SEARCH_INFO));
                FileClose(sHandle);
                return FAIL;
            }
            memset((BYTE *) myFavorBuf, 0, FAVOR_HISTORY_SIZE * sizeof(ST_SEARCH_INFO));

            file_size = FileSizeGet(sHandle);
            read_len = FileRead(sHandle, (BYTE *) myFavorBuf, file_size);
            if (read_len != file_size)
            {
                MP_ALERT("%s: Error! Length of read bytes (=%d) < expected read bytes (= %d) => check storage drive !", __FUNCTION__, read_len, file_size);
                ext_mem_free((BYTE *) myFavorBuf);
                FileClose(sHandle);
                return FAIL;
            }

            /* append selected items into existing MyFavor setting file */
            for (i=0; i < FAVOR_HISTORY_SIZE; i++)
            {
                BOOL f_IsNewItem = TRUE;

                if (GetBitByOffset(pdwFavor, i) == 0) //selected item
                {
                    DWORD idx_in_OriFileList = i;
                    int j;
                    ST_SEARCH_INFO *favor_item, *select_item;

                    select_item = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE]) + idx_in_OriFileList);

                    for (j=0; j < final_favor_entries_cnt; j++)
                    {
                        favor_item = (ST_SEARCH_INFO *) (((ST_SEARCH_INFO *) myFavorBuf) + j);

                        if (favor_item->FsType != FS_TYPE_exFAT)
                        {
                            /* here, comparing their 8.3 short filenames is enough. No need to use UTF-16 long filename */
                            if ( (memcmp(favor_item->bName, select_item->bName, 8) == 0) && (memcmp(favor_item->bExt, select_item->bExt, 4) == 0) &&
                                 (favor_item->dwDirStart == select_item->dwDirStart) )
                            {
                                f_IsNewItem = FALSE;
                                break; /* selected item already in the MyFavor setting file */
                            }
                        }
    #if EXFAT_ENABLE
                        else
                        {
                            /* exFAT does not support 8.3 short filename. Use filename hash values to compare */
                            if ( (favor_item->exfat_NameHash == select_item->exfat_NameHash) && (favor_item->dwDirStart == select_item->dwDirStart) )
                            {
                                f_IsNewItem = FALSE;
                                break; /* selected item already in the MyFavor setting file */
                            }
                        }
    #endif
                    }

                    if (f_IsNewItem) /* new item to be added into the MyFavor setting file */
                    {
                        if (final_favor_entries_cnt < FAVOR_HISTORY_SIZE - 1) /* avoid overflow */
                        {
                            memcpy((BYTE *) (((ST_SEARCH_INFO *) myFavorBuf) + final_favor_entries_cnt), (BYTE *) select_item, sizeof(ST_SEARCH_INFO));
                            final_favor_entries_cnt++;
                        }
                        else
                        {
                            MP_ALERT("%s: Warning: number of total favor items reaches it limit !!", __FUNCTION__);
                        }
                    }
                }
            }

            /* save MyFavor setting into the file */
            SeekSet(sHandle);
            len_to_write = final_favor_entries_cnt * sizeof(ST_SEARCH_INFO);
            written_len = FileWrite(sHandle, (BYTE *) myFavorBuf, len_to_write);
            if (written_len != len_to_write)
    	    {
                MP_ALERT("%s: Error! Length of written bytes (=%d) < expected written bytes (= %d) => check storage drive !", __FUNCTION__, written_len, len_to_write);
                ext_mem_free((BYTE *) myFavorBuf);
                FileClose(sHandle);
                return FAIL;
    	    }

            MP_DEBUG("%s: count of final stored photo entries in MyFavor_%d Set = %d", __FUNCTION__, favor_set_idx, final_favor_entries_cnt);
            ext_mem_free((BYTE *) myFavorBuf);
        }
    }

    FileClose(sHandle);
    return PASS;
}



///
///@ingroup xpgFuncEntry
///@brief   Remove selected photo items within a specific MyFavor set from its MyFavor setting file.
///
///@param   favor_set_idx      The index of specified MyFavor photo set. \n
///                            Range of valid value is FAVOR_SET_MIN_INDEX ~ FAVOR_SET_MAX_INDEX. \n\n
///
///@retval  PASS         Remove selected items and update its MyFavor setting file successfully. \n\n
///@retval  END_OF_DIR   MyFavor setting file of the specified MyFavor set is not found => Nothing can be removed. \n\n
///@retval  FAIL         Failed to remove selected items and update its MyFavor setting file. \n\n
///
int RemoveFromFavorPicSetting(BYTE favor_set_idx)
{
    STREAM *sHandle;
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
    DRIVE *drv;
    int   ret, i, stored_favor_entries_cnt, selected_favor_entries_cnt = 0;
    DWORD *pdwFavor;


    if (SYS_DRV_ID != NULL_DRIVE)
        drv = DriveGet(SYS_DRV_ID);
    else
        drv = DriveGet(psSysConfig->sStorage.dwCurStorageId); /* if System Drive not defined, use current drive */

    /* reset matched file list w.r.t MyFavor of Photo */
    dwMyFavorFileListIndex = 0;
    dwMyFavorFileListCount = 0;
    memset((BYTE *) MyFavorFileList, 0, FAVOR_HISTORY_SIZE * sizeof(ST_SEARCH_INFO));

    if ((favor_set_idx < FAVOR_SET_MIN_INDEX) || (favor_set_idx > FAVOR_SET_MAX_INDEX))
    {
        MP_ALERT("%s: Invalid favor_set_idx = %d !", __FUNCTION__, favor_set_idx);
        return FAIL;
    }

    if (FavorSettingFilename(favor_set_idx) == FAIL)
    {
        MP_ALERT("%s: Error! FavorSettingFilename(%d) failed !", __FUNCTION__, favor_set_idx);
        return FAIL;
    }

    /* back to beginning of root directory */
    if (DirReset(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! DirReset() back to beginning of root directory failed !", __FUNCTION__);
        return FAIL;
    }

    ret = FileSearchLN(drv, FavorSetting_UTF16_filename, StringLength16(FavorSetting_UTF16_filename), E_FILE_TYPE);
    if (ret == END_OF_DIR) /* file not found */
    {
        MP_ALERT("%s: Error! MyFavor_%d setting file not found, nothing can be removed.", __FUNCTION__, favor_set_idx);
        return END_OF_DIR;
    }
    else if (ret == FS_SUCCEED) /* file found */
    {
        sHandle = FileOpen(drv);
        stored_favor_entries_cnt = FileSizeGet(sHandle)/sizeof(ST_SEARCH_INFO);
        MP_DEBUG("%s: count of stored photo entries in MyFavor_%d Set = %d", __FUNCTION__, favor_set_idx, stored_favor_entries_cnt);

        if (stored_favor_entries_cnt == 0)
        {
            MP_ALERT("%s: Empty content in MyFavor_%d setting file ! => delete the setting file ...", __FUNCTION__, favor_set_idx);
            if (DeleteFile(sHandle) == FS_SUCCEED)
            {
                MP_ALERT("%s: delete empty MyFavor_%d setting file successfully.", __FUNCTION__, favor_set_idx);
                return PASS;
            }
            else
            {
                MP_ALERT("%s: Error! Failed to delete empty MyFavor_%d setting file !", __FUNCTION__, favor_set_idx);
                FileClose(sHandle); /* just for safety, if file was not closed during DeleteFile() */
                return FAIL;
            }
        }
    }
    else
    {
        MP_ALERT("%s: Error! FileSearchLN() failed due to some error !", __FUNCTION__);
        return FAIL;
    }

    pdwFavor = &g_dwPicFavoriteHistory[0];
    if (pdwFavor != NULL)
    {
        for (i=0; i < FAVOR_HISTORY_SIZE; i++)
        {
            if (GetBitByOffset(pdwFavor, i) == 0) //selected item
                selected_favor_entries_cnt++;
        }
        MP_DEBUG("%s: count of selected photo entries to remove from MyFavor = %d", __FUNCTION__, selected_favor_entries_cnt);
    }

    if (selected_favor_entries_cnt)
    {
        DWORD file_size, read_len;
        ST_SEARCH_INFO *myFavorBuf = NULL;
        DWORD len_to_write, written_len;
        DWORD final_favor_entries_cnt = stored_favor_entries_cnt;

        /* load original saved MyFavor setting content to buffer first */
        myFavorBuf = (ST_SEARCH_INFO *) ext_mem_malloc(FAVOR_HISTORY_SIZE * sizeof(ST_SEARCH_INFO));
        if (myFavorBuf == NULL)
        {
            MP_ALERT("%s: malloc failed for buffer size (%d) !!", __FUNCTION__, FAVOR_HISTORY_SIZE * sizeof(ST_SEARCH_INFO));
            FileClose(sHandle);
            return FAIL;
        }
        memset((BYTE *) myFavorBuf, 0, FAVOR_HISTORY_SIZE * sizeof(ST_SEARCH_INFO));

        file_size = FileSizeGet(sHandle);
        read_len = FileRead(sHandle, (BYTE *) myFavorBuf, file_size);
        if (read_len != file_size)
        {
            MP_ALERT("%s: Error! Length of read bytes (=%d) < expected read bytes (= %d) => check storage drive !", __FUNCTION__, read_len, file_size);
            ext_mem_free((BYTE *) myFavorBuf);
            FileClose(sHandle);
            return FAIL;
        }

        /* remove selected items from the MyFavor setting list */
        for (i=0; (i < FAVOR_HISTORY_SIZE) && (i < stored_favor_entries_cnt); i++)
        {
            DWORD idx_in_FavorFileList;
            DWORD len_to_write, written_len;

            if (GetBitByOffset(pdwFavor, i) == 0) //selected item
            {
                idx_in_FavorFileList = i;

                /* clear the selected MyFavor entry item in the list buffer */
                memset((BYTE *) (((ST_SEARCH_INFO *) myFavorBuf) + idx_in_FavorFileList), 0, sizeof(ST_SEARCH_INFO));
                final_favor_entries_cnt--;

                if (final_favor_entries_cnt == 0) /* MyFavor list becomes empty */
                {
                    MP_ALERT("%s: Empty content left in MyFavor_%d setting file ! => delete the setting file ...", __FUNCTION__, favor_set_idx);
                    ext_mem_free((BYTE *) myFavorBuf);
                    if (DeleteFile(sHandle) == FS_SUCCEED)
                    {
                        MP_ALERT("%s: delete empty MyFavor_%d setting file successfully.", __FUNCTION__, favor_set_idx);
                        return PASS;
                    }
                    else
                    {
                        MP_ALERT("%s: Error! Failed to delete empty MyFavor_%d setting file !", __FUNCTION__, favor_set_idx);
                        FileClose(sHandle); /* just for safety, if file was not closed during DeleteFile() */
                        return FAIL;
                    }
                }
            }
        }

        /* save/update left content into the MyFavor setting file */
        if (final_favor_entries_cnt > 0)
        {
            DWORD found_entries = 0, j;
            ST_SEARCH_INFO zero_entry;
            memset((BYTE *) &zero_entry, 0, sizeof(ST_SEARCH_INFO));

            /* re-arrange to compact the MyFavor setting list */
            for (i=0; i < stored_favor_entries_cnt; i++)
            {
                if (memcmp((BYTE *) (((ST_SEARCH_INFO *) myFavorBuf) + i), (BYTE *) &zero_entry, sizeof(ST_SEARCH_INFO)) != 0)
                {
                	found_entries++;
                }
            	else /* this is a zero entry */
            	{
                    for (j=i+1; j < stored_favor_entries_cnt; j++)
                    {
                        if (memcmp((BYTE *) (((ST_SEARCH_INFO *) myFavorBuf) + j), (BYTE *) &zero_entry, sizeof(ST_SEARCH_INFO)) != 0)
                        {
                            memcpy((BYTE *) (((ST_SEARCH_INFO *) myFavorBuf) + i), (BYTE *) (((ST_SEARCH_INFO *) myFavorBuf) + j), sizeof(ST_SEARCH_INFO));
                            memset((BYTE *) (((ST_SEARCH_INFO *) myFavorBuf) + j), 0, sizeof(ST_SEARCH_INFO));
                	        found_entries++;
                            break;
                        }
                    }
            	}

            	if (found_entries == final_favor_entries_cnt) /* MyFavor setting list is already compact */
            	{
            	    break;
            	}
            }

            /* save MyFavor setting into the file */
            ChainInit((CHAIN *) &sHandle->Chain, sHandle->Chain.Start, 0); /* reset file content and file size */
            len_to_write = final_favor_entries_cnt * sizeof(ST_SEARCH_INFO);
            written_len = FileWrite(sHandle, (BYTE *) myFavorBuf, len_to_write);
            if (written_len != len_to_write)
            {
                MP_ALERT("%s: Error! Length of written bytes (=%d) < expected written bytes (= %d) => check storage drive !", __FUNCTION__, written_len, len_to_write);
                ext_mem_free((BYTE *) myFavorBuf);
                FileClose(sHandle);
                return FAIL;
            }

            MP_DEBUG("%s: count of final stored photo entries in MyFavor_%d Set = %d", __FUNCTION__, favor_set_idx, final_favor_entries_cnt);
        }
        ext_mem_free((BYTE *) myFavorBuf);
    }

    FileClose(sHandle);
    return PASS;
}



///
///@ingroup xpgFuncEntry
///@brief   Reset the saved MyFavor photo setting. Actually, this function simply deletes the MyFavor setting file.
///
///@param   favor_set_idx      The index of specified MyFavor photo set. \n
///                            Range of valid value is FAVOR_SET_MIN_INDEX ~ FAVOR_SET_MAX_INDEX. \n\n
///
///@retval  PASS         Reset the specified MyFavor setting successfully. \n\n
///@retval  END_OF_DIR   MyFavor setting file of the specified MyFavor set is not found => No need to reset/delete it. \n\n
///@retval  FAIL         Failed to reset the specified MyFavor setting due to some error. \n\n
///
int ResetFavorPicSetting(BYTE favor_set_idx)
{
    STREAM *sHandle;
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
    DRIVE *drv;
    int ret;


    if (SYS_DRV_ID != NULL_DRIVE)
        drv = DriveGet(SYS_DRV_ID);
    else
        drv = DriveGet(psSysConfig->sStorage.dwCurStorageId); /* if System Drive not defined, use current drive */

    /* reset matched file list w.r.t MyFavor of Photo */
    dwMyFavorFileListIndex = 0;
    dwMyFavorFileListCount = 0;
    memset((BYTE *) MyFavorFileList, 0, FAVOR_HISTORY_SIZE * sizeof(ST_SEARCH_INFO));

    if ((favor_set_idx < FAVOR_SET_MIN_INDEX) || (favor_set_idx > FAVOR_SET_MAX_INDEX))
    {
        MP_ALERT("%s: Invalid favor_set_idx = %d !", __FUNCTION__, favor_set_idx);
        return FAIL;
    }

    if (FavorSettingFilename(favor_set_idx) == FAIL)
    {
        MP_ALERT("%s: Error! FavorSettingFilename(%d) failed !", __FUNCTION__, favor_set_idx);
        return FAIL;
    }

    /* back to beginning of root directory */
    if (DirReset(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: Error! DirReset() back to beginning of root directory failed !", __FUNCTION__);
        return FAIL;
    }

    ret = FileSearchLN(drv, FavorSetting_UTF16_filename, StringLength16(FavorSetting_UTF16_filename), E_FILE_TYPE);
    if (ret == END_OF_DIR) /* file not found */
    {
        MP_ALERT("%s: MyFavor_%d setting file not found, no need to delete.", __FUNCTION__, favor_set_idx);
        return END_OF_DIR;
    }
    else if (ret == FS_SUCCEED) /* file found */
    {
        sHandle = FileOpen(drv);
        if (DeleteFile(sHandle) == FS_SUCCEED)
        {
            MP_DEBUG("%s: delete MyFavor_%d setting file successfully.", __FUNCTION__, favor_set_idx);
            return PASS;
        }
        else
        {
            MP_ALERT("%s: Error! Failed to delete MyFavor_%d setting file !", __FUNCTION__, favor_set_idx);
            FileClose(sHandle); /* just for safety, if file was not closed during DeleteFile() */
            return FAIL;
        }
    }
    else
    {
        MP_ALERT("%s: Error! FileSearchLN() failed due to some error !", __FUNCTION__);
        return FAIL;
    }
}

// -----------------------------------------------------------------------------
// - MyFavor UI
// -----------------------------------------------------------------------------
#define PHOTO_TYPE  1
void MyFavor1AddFile()
{
    MP_DEBUG("%s()", __FUNCTION__);
    BYTE i = GetFavoriteHistoryCount(PHOTO_TYPE); // PHOTO
    MP_DEBUG("%s: FavoriteHistory count = %d", __FUNCTION__, i);
    if (i <= 0)
        return;
    SaveFavorPicSetting(1);
}

void MyFavor2AddFile()
{
    MP_DEBUG("%s()", __FUNCTION__);
    BYTE i = GetFavoriteHistoryCount(PHOTO_TYPE); // PHOTO
    MP_DEBUG("%s: FavoriteHistory count = %d", __FUNCTION__, i);
    if (i <= 0)
        return;
    SaveFavorPicSetting(2);
}

void MyFavor3AddFile()
{
    MP_DEBUG("%s()", __FUNCTION__);
    BYTE i = GetFavoriteHistoryCount(PHOTO_TYPE); // PHOTO
    MP_DEBUG("%s: FavoriteHistory count = %d", __FUNCTION__, i);
    if (i <= 0)
        return;
    SaveFavorPicSetting(3);
}

void MyFavor4AddFile()
{
    MP_DEBUG("%s()", __FUNCTION__);
    BYTE i = GetFavoriteHistoryCount(PHOTO_TYPE); // PHOTO
    MP_DEBUG("%s: FavoriteHistory count = %d", __FUNCTION__, i);
    if (i <= 0)
        return;
    SaveFavorPicSetting(4);
}

void MyFavorDeleteFile()
{
    MP_DEBUG("%s: xpgGetMyFavorIndex() = %d", __FUNCTION__, xpgGetMyFavorIndex());
    if (g_bXpgStatus != XPG_MODE_MYFAVORPHOTO)
        return;

    BYTE i = GetFavoriteHistoryCount(PHOTO_TYPE); // PHOTO
    MP_DEBUG("FavoriteHistory count = %d", i);
    if (i <= 0)
    {
        MP_DEBUG("%s: FavoriteCount = 0, => do nothing.", __FUNCTION__);
        return;
    }

    int ret = RemoveFromFavorPicSetting(xpgGetMyFavorIndex());
    if (ret == FAIL)
    {
        MP_ALERT("%s: RemoveFromFavorPicSetting() failed !", __FUNCTION__);
        return;
    }
    else if (ret == END_OF_DIR)
    {
        MP_DEBUG("%s: the MyFavor setting file not found.", __FUNCTION__);
        return;
    }

    ret = LoadFavorPicSetting(xpgGetMyFavorIndex());
    if (ret == FAIL)
    {
        MP_ALERT("%s: LoadFavorPicSetting() failed !", __FUNCTION__);
        return;
    }
    else if (ret == END_OF_DIR)
    {
        MP_DEBUG("%s: the MyFavor setting file not found.", __FUNCTION__);
        return;
    }

    MP_DEBUG("%s: after RemoveFromFavorPicSetting(), dwMyFavorFileListCount = %d", __FUNCTION__, dwMyFavorFileListCount);
    g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile = dwMyFavorFileListCount;
    g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex = 0;

    RestoreCurrentSearchInfo();
    g_bXpgStatus = XPG_MODE_MYFAVOR;
    g_boNeedRepaint = true;
    xpgSearchAndGotoPage("MyFavor");
    xpgUpdateStage();
}

void MyFavorReset1()
{
    MP_DEBUG("%s: xpgGetMyFavorIndex() = %d, g_bXpgStatus = %d", __FUNCTION__, xpgGetMyFavorIndex(), g_bXpgStatus);
    int ret = ResetFavorPicSetting(1);
    if (ret == FAIL)
    {
        MP_ALERT("%s: ResetFavorPicSetting() failed !", __FUNCTION__);
        return;
    }
    else if (ret == END_OF_DIR)
    {
        MP_DEBUG("%s: the MyFavor setting file not found.", __FUNCTION__);
        return;
    }
}

void MyFavorReset2()
{
    MP_DEBUG("%s: xpgGetMyFavorIndex() = %d, g_bXpgStatus = %d", __FUNCTION__, xpgGetMyFavorIndex(), g_bXpgStatus);
    int ret = ResetFavorPicSetting(2);
    if (ret == FAIL)
    {
        MP_ALERT("%s: ResetFavorPicSetting() failed !", __FUNCTION__);
        return;
    }
    else if (ret == END_OF_DIR)
    {
        MP_DEBUG("%s: the MyFavor setting file not found.", __FUNCTION__);
        return;
    }
}

void MyFavorReset3()
{
    MP_DEBUG("%s: xpgGetMyFavorIndex() = %d, g_bXpgStatus = %d", __FUNCTION__, xpgGetMyFavorIndex(), g_bXpgStatus);
    int ret = ResetFavorPicSetting(3);
    if (ret == FAIL)
    {
        MP_ALERT("%s: ResetFavorPicSetting() failed !", __FUNCTION__);
        return;
    }
    else if (ret == END_OF_DIR)
    {
        MP_DEBUG("%s: the MyFavor setting file not found.", __FUNCTION__);
        return;
    }
}

void MyFavorReset4()
{
    MP_DEBUG("%s: xpgGetMyFavorIndex() = %d, g_bXpgStatus = %d", __FUNCTION__, xpgGetMyFavorIndex(), g_bXpgStatus);
    int ret = ResetFavorPicSetting(4);
    if (ret == FAIL)
    {
        MP_ALERT("%s: ResetFavorPicSetting() failed !", __FUNCTION__);
        return;
    }
    else if (ret == END_OF_DIR)
    {
        MP_DEBUG("%s: the MyFavor setting file not found.", __FUNCTION__);
        return;
    }
}

//---- MyFavor -----------------------------------------------------------------
#define FAVOR_SET_NULL_INDEX       0
BYTE myFavorIndex = FAVOR_SET_NULL_INDEX;
BYTE xpgGetMyFavorIndex()
{
    return myFavorIndex;
}

BYTE xpgResetMyFavorIndex()
{
    myFavorIndex = FAVOR_SET_NULL_INDEX;
    MP_DEBUG("%s: myFavorIndex = %d", __FUNCTION__, myFavorIndex);
}

void BackupCurrentSearchInfo()
{
    MP_DEBUG("%s()", __FUNCTION__);
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
    dwBackupOriFileListCount = psFileBrowser->dwFileListCount[OP_IMAGE_MODE];
    dwSwapFileListCount = dwMyFavorFileListCount;
    MP_DEBUG("dwSwapFileListCount=%d", dwSwapFileListCount);
    memcpy((BYTE *) BackupOriFileList, (BYTE *) ((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE]), dwSwapFileListCount * sizeof(ST_SEARCH_INFO));
}

void RestoreCurrentSearchInfo()
{
    MP_DEBUG("%s()", __FUNCTION__);
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;

    psFileBrowser->dwFileListCount[OP_IMAGE_MODE] = dwBackupOriFileListCount;
    MP_DEBUG("dwSwapFileListCount=%d", dwSwapFileListCount);
    memcpy((BYTE *) ((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE]), (BYTE *) BackupOriFileList, dwSwapFileListCount * sizeof(ST_SEARCH_INFO));
    dwSwapFileListCount = 0;
    dwBackupOriFileListCount = 0;
}

void xpgCb_MyFavorEnter(void)
{
    MP_DEBUG("%s: g_bXpgStatus = %d, myFavorIndex = %d", __FUNCTION__, g_bXpgStatus, myFavorIndex);

    // Save file list parameters
    SetSave_dwImgAndMovTotalFile(g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile);
    SetSave_dwImgAndMovCurIndex(g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex);
    SetSave_bFileSortBasis(GetCurFileSortingBasis());
    SetSave_wCount(g_pstBrowser->wCount);
    SetSave_wIndex(g_pstBrowser->wIndex);
    SetSave_wListCount(g_pstBrowser->wListCount);
    SetSave_wListIndex(g_pstBrowser->wListIndex);
    SetSave_wListFirstIndex(g_pstBrowser->wListFirstIndex);

    myFavorIndex = FAVOR_SET_MIN_INDEX;

    g_bXpgStatus = XPG_MODE_MYFAVOR;
    g_boNeedRepaint = true;
    xpgSearchAndGotoPage("MyFavor");
    xpgUpdateStage();
}

void xpgCb_MyFavorExit(void)
{
    MP_DEBUG("%s()", __FUNCTION__);
    myFavorIndex = 0;

    Idu_OsdErase();
    g_boNeedRepaint = true;
    g_bXpgStatus = XPG_MODE_PHOTOMENU;
    xpgSearchAndGotoPage("Mode_Photo");
    //xpgUpdateStage(); // it makes show "Mode_Photo" twice!"
    xpgCb_EnterPhotoMenu();
}

void xpgCb_MyFavorUp(void)
{
    MP_DEBUG("%s()", __FUNCTION__);
    if (myFavorIndex > FAVOR_SET_MIN_INDEX)
        myFavorIndex--;
    else
        myFavorIndex = FAVOR_SET_MAX_INDEX;

    g_bXpgStatus = XPG_MODE_MYFAVOR;
    g_boNeedRepaint = true;
    xpgSearchAndGotoPage("MyFavor");
    xpgUpdateStage();
}

void xpgCb_MyFavorDown(void)
{
    MP_DEBUG("%s()", __FUNCTION__);
    if (myFavorIndex < FAVOR_SET_MAX_INDEX)
        myFavorIndex++;
    else
        myFavorIndex = FAVOR_SET_MIN_INDEX;

    g_bXpgStatus = XPG_MODE_MYFAVOR;
    g_boNeedRepaint = true;
    xpgSearchAndGotoPage("MyFavor");
    xpgUpdateStage();
}

void xpgCb_MyFavorLeft(void)
{
    MP_DEBUG("%s()", __FUNCTION__);
    g_boNeedRepaint = false;
}

void xpgCb_MyFavorRight(void)
{
    MP_DEBUG("%s()", __FUNCTION__);
    g_boNeedRepaint = false;
}

// MyFavorPhoto ----------------------------------------------------------------
void xpgCb_MyFavorPhotoEnter(void)
{
    MP_DEBUG("%s: g_bXpgStatus = %d, myFavorIndex = %d", __FUNCTION__, g_bXpgStatus, myFavorIndex);

    int ret = LoadFavorPicSetting(xpgGetMyFavorIndex());
    if (ret == FAIL)
    {
        MP_ALERT("%s: LoadFavorPicSetting() failed !", __FUNCTION__);
        return;
    }
    else if (ret == END_OF_DIR)
    {
        MP_DEBUG("%s: the MyFavor setting file not found.", __FUNCTION__);
        return;
    }

    if (dwMyFavorFileListCount == 0) /* no matched list entry */
    {
        mpDebugPrint("%s: (g_bXpgStatus == XPG_MODE_MYFAVOR), dwMyFavorFileListCount = %d", __FUNCTION__, dwMyFavorFileListCount);
        return;
    }

    BackupCurrentSearchInfo();
    // duplicate MyFavorX to FilelIst
    ST_SEARCH_INFO *pSearchInfo;
    pSearchInfo = (ST_SEARCH_INFO *) FileGetSearchInfo(0); //FileGetCurSearchInfo();
    memcpy((BYTE *) pSearchInfo, (BYTE *) MyFavorFileList, dwMyFavorFileListCount*sizeof(ST_SEARCH_INFO));

    // re-arrange PHOTO file list parameters
    MP_DEBUG("%s: dwMyFavorFileListCount = %d", __FUNCTION__, dwMyFavorFileListCount);
    g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile = dwMyFavorFileListCount;
    g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex = 0;
    g_pstBrowser->wCount = dwMyFavorFileListCount;
    g_pstBrowser->wIndex = 0;

    g_bXpgStatus = XPG_MODE_MYFAVORPHOTO;
    g_boNeedRepaint = true;
    xpgSearchAndGotoPage("Mode_PhotoFavor");
    xpgUpdateStage();
}

void xpgCb_MyFavorPhotoExit(void)
{
    MP_DEBUG("%s()", __FUNCTION__);
    RestoreCurrentSearchInfo(); // not by xpgCb_PageGlobalEnd()

    Idu_OsdErase();
    g_boNeedRepaint = true;
    g_bXpgStatus = XPG_MODE_MYFAVOR;
    xpgSearchAndGotoPage("MyFavor");
    xpgUpdateStage();
}

#endif

