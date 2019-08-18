
#include "global612.h"
#include "mpTrace.h"
#include "SPI_Ex.h"

BOOL spifs_inited = FALSE;
SPI_FAT_TABLE  spi_fat;
WORD  wTotalPage;
WORD  wFreePage;
BYTE  pagebuf[SPI_PAGE_SIZE];
#if 0
// 注意 : 一次只能操作一个文件
struct __SPI_FILE_
{
    int         open_flag;          // the lock
    int         filehandle;         // filehandle为目录项中的ID号
    DWORD       pointer;
    int         last_found_id;
    BYTE *      cache_buf;
};
typedef struct __SPI_FILE_ SPI_FILE;
SPI_FILE spi_file;
#endif

#if SAVE_FILE_TO_SPI

DWORD spifs_GetTotalDataSpaceSize()
{
    return (SPI_FLASH_SIZE - SPI_FS_DATA_ADDR);
}

WORD spifs_GetTotalPageCount()
{
    return (spifs_GetTotalDataSpaceSize()/SPI_PAGE_SIZE);
}

WORD spifs_GetFreePageCount()
{
    WORD i, count;
    WORD wTotalPage;
    
    if (!spifs_inited)
        return 0;

    wTotalPage = spifs_GetTotalPageCount();
    count = 0;
    for (i = 0; i < wTotalPage; i++)
    {
        if (spi_fat.tables[i] == SPI_FS_PAGE_NULL)
            count ++;
        //mpDebugPrint("page[%d] = 0x%04x", i, spi_fat.tables[i]);
    }

    return count;
}

WORD spifs_GetFreePageCount2()
{
    return wFreePage;
}

int spifs_ReadFileInfoBlock(BYTE * buffer)
{
    DWORD addr;
    int ret;

    if (buffer == NULL)
        return ERR_SPIFS_UNKNOWN;

    addr = SPI_FS_INFO_ADDR;
    if (addr >= SPI_FLASH_SIZE) {
        mpDebugPrint("-E- %s(), out of range, 0x%08X", __FUNCTION__, addr);
        return ERR_SPIFS_OUT_OF_RANGE;
    }

    ret = spi_rcmd(buffer, SPI_BLOCK_SIZE, addr);
    if (ret < 0) {
        mpDebugPrint("SPI FS read file info blcok error");
        return ERR_SPIFS_READ_ERROR;
    }
    
    return PASS;
}

int spifs_WriteFileInfoBlock(BYTE * buffer)
{
    DWORD addr;
    int ret;
    
    if (buffer == NULL)
        return ERR_SPIFS_UNKNOWN;
    
    addr = SPI_FS_INFO_ADDR;
    if (addr >= SPI_FLASH_SIZE) {
        mpDebugPrint("-E- %s(), out of range, 0x%08X", __FUNCTION__, addr);
        return ERR_SPIFS_OUT_OF_RANGE;
    }

    ret = spi_block_erase(addr, 1);
    if (ret < 0) {
        mpDebugPrint("SPI FS erase error");
        return ERR_SPIFS_WRITE_ERROR;
    }
    
    ret = spi_wcmd(buffer, SPI_BLOCK_SIZE, addr);
    if (ret < 0) {
        mpDebugPrint("SPI FS write file info blcok error");
        return ERR_SPIFS_WRITE_ERROR;
    }

    return PASS;
}

int spifs_WriteBackFATTable()
{
    spifs_WriteFileInfoBlock((BYTE*)(&spi_fat));
}

int spifs_ReadPage(WORD page_id, BYTE * buffer)
{
    DWORD addr;
    int ret;

    if (buffer == NULL)
        return ERR_SPIFS_UNKNOWN;

    addr = SPI_FS_DATA_ADDR + (page_id * SPI_PAGE_SIZE);
    if (addr >= SPI_FLASH_SIZE) {
        mpDebugPrint("-E- %s(), out of range, 0x%08X", __FUNCTION__, addr);
        return ERR_SPIFS_OUT_OF_RANGE;
    }

    ret = spi_rcmd(buffer, SPI_PAGE_SIZE, addr);
    if (ret < 0) {
        mpDebugPrint("SPI FS read error");
        return ERR_SPIFS_READ_ERROR;
    }
    
    return PASS;
}

int spifs_WritePage(WORD page_id, BYTE * buffer)
{
    DWORD addr;
    int ret;
    
    if (buffer == NULL)
        return ERR_SPIFS_UNKNOWN;
    
    addr = SPI_FS_DATA_ADDR + (page_id * SPI_PAGE_SIZE);
    if (addr >= SPI_FLASH_SIZE) {
        mpDebugPrint("-E- %s(), out of range, 0x%08X", __FUNCTION__, addr);
        return ERR_SPIFS_OUT_OF_RANGE;
    }

    ret = spi_sector_erase(addr, 1);
    if (ret < 0) {
        mpDebugPrint("SPI FS erase error");
        return ERR_SPIFS_WRITE_ERROR;
    }
    
    ret = spi_wcmd(buffer, SPI_PAGE_SIZE, addr);
    if (ret < 0) {
        mpDebugPrint("SPI FS write error");
        return ERR_SPIFS_WRITE_ERROR;
    }

    return PASS;
}

int spifs_FileSystemInit()
{
    int ret;
    int formated;

    mpDebugPrint("%s()", __FUNCTION__);
    
    ret = spifs_ReadFileInfoBlock((BYTE*)(&spi_fat));
    if (ret < 0) {
        mpDebugPrint("%s()  Fail !!!", __FUNCTION__);
        return -1;
    }

    formated = 1;
        mpDebugPrint("%s()  flag=0x%x  filetype=0x%x", __FUNCTION__,spi_fat.fentry[0].flag,spi_fat.fentry[0].filetype);
    if (spi_fat.fentry[0].flag != SPI_FS_ENTRY_FORMATED)
        formated = 0;
    if (spi_fat.fentry[0].filetype != SPI_FILE_TYPE_FORMATED_FLAG)
        formated = 0;

    if (formated == 0)      // unformated
    {
        mpDebugPrint("SPI unformated, need to format.");
        if (spifs_Format() != PASS) {
            mpDebugPrint("-E- Format fail !!!");
            return -1;
        }
    }

    spifs_inited = TRUE;

    wTotalPage = spifs_GetTotalPageCount();
    wFreePage = spifs_GetFreePageCount();

    mpDebugPrint("Total page = %d, free page = %d", wTotalPage, wFreePage);
    return PASS;
}

int spifs_Format()
{
    int ret;
    WORD i;

    mpDebugPrint("%s()", __FUNCTION__);
    
    memset(&spi_fat, 0xff, sizeof(spi_fat));

    for (i = 0; i < SPI_MAX_FILE_COUNT; i++)
    {
        spi_fat.fentry[i].filesize = 0;
        spi_fat.fentry[i].flag = SPI_FS_ENTRY_NULL;
        spi_fat.fentry[i].filetype = SPI_FILE_TYPE_NONE;
        spi_fat.fentry[i].start_page_id = SPI_FS_PAGE_EOF;
    }
    spi_fat.fentry[0].flag = SPI_FS_ENTRY_FORMATED;                 // format flag
    spi_fat.fentry[0].filetype = SPI_FILE_TYPE_FORMATED_FLAG;       // format flag   

    //////////
    wTotalPage = spifs_GetTotalPageCount();
    
    for (i = 0; i < wTotalPage; i++)
        spi_fat.tables[i] = SPI_FS_PAGE_NULL;
    
    for (;i < sizeof(spi_fat.tables)/sizeof(WORD); i++)
        spi_fat.tables[i] = SPI_FS_PAGE_NOT_EXIST;

    ret = spifs_WriteFileInfoBlock((BYTE*)(&spi_fat));
    if (ret < 0)
    {
        mpDebugPrint("format fail.");
        return (DWORD)(-1);
    }
    wFreePage = spifs_GetFreePageCount();

    return PASS;    
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
static int _spifs_GetNullEntry()
{
    DWORD i;
    SPI_FILE_ENTRY * entry;
    if (!spifs_inited)
        return -1;
    for (i = 0; i < SPI_MAX_FILE_COUNT; i++)
    {
        entry = & (spi_fat.fentry[i]);
        if (entry->flag == SPI_FS_ENTRY_NULL)
            return (int)i;
    }
    mpDebugPrint("spi : No null entry");
    return -1;
}

DWORD spifs_GetFileSize(DWORD dwFileIndex)
{
    SPI_FILE_ENTRY * entry;
    if (!spifs_inited)
        return 0;
    if (dwFileIndex >= SPI_MAX_FILE_COUNT) {
        mpDebugPrint("out of file count, in %s()", __FUNCTION__);
        return 0;
    }
    entry = & (spi_fat.fentry[dwFileIndex]);
    if (entry->flag != SPI_FS_ENTRY_EXIST_FILE)
        return 0;
    return entry->filesize;
}

DWORD spifs_ReadFileToBuffer(DWORD dwFileIndex, BYTE* buffer, int *pError)
{
    SPI_FILE_ENTRY * entry;

    mpDebugPrint("%s", __FUNCTION__);
    if (pError == NULL || buffer == NULL) 
    {
        mpDebugPrint("pError == NULL, or buffer == NULL");
        return 0;
    }
    
    if (!spifs_inited) {
        *pError = ERR_SPIFS_UNINIT;
        return 0;
    }

    if (dwFileIndex >= SPI_MAX_FILE_COUNT) {
        mpDebugPrint("out of file count, in %s()", __FUNCTION__);
        *pError = ERR_SPIFS_OUT_OF_FILECOUNT;
        return 0;
    }
    
    entry = & (spi_fat.fentry[dwFileIndex]);
    if (entry->filesize == 0) {
        return 0;
    }

    //////////////////////////////////////////////
    WORD wReadPageCnt, page;
    DWORD dwReadSize, dwCurReadSize;
    DWORD fcnt;
    WORD pageid;
    wReadPageCnt = entry->filesize / SPI_PAGE_SIZE;
    if (entry->filesize % SPI_PAGE_SIZE)
        wReadPageCnt ++;

    dwReadSize = entry->filesize;
    fcnt = 0;
    pageid = entry->start_page_id;
    for (page = 0; page < wReadPageCnt; page++)
    {
        if (pageid == SPI_FS_PAGE_EOF || pageid == SPI_FS_PAGE_NOT_EXIST 
            || pageid == SPI_FS_PAGE_NULL)
            break;
        dwCurReadSize = SPI_PAGE_SIZE;
        if (dwReadSize < dwCurReadSize)
            dwCurReadSize = dwReadSize;

        spifs_ReadPage(pageid, pagebuf); 

        //mpDebugPrint("read page id = %d", pageid);
        memcpy(&buffer[fcnt], pagebuf, dwCurReadSize);

        fcnt += dwCurReadSize;
        dwReadSize -= dwCurReadSize;
        pageid = spi_fat.tables[pageid];
    }
    *pError = 0;

    mpDebugPrint("read filesize = %d", fcnt);
   
    return fcnt;
}

static WORD _spifs_AllocateNewPage()
{
    WORD i;
    WORD wTotalPage;

    wTotalPage = spifs_GetTotalPageCount();
    for (i = 0; i < wTotalPage; i++)
    {
        if (spi_fat.tables[i] == SPI_FS_PAGE_NULL)
            return i;
    }
    return (WORD)-1;
}


DWORD spifs_WriteBufferToNewFile(DWORD* pdwFileIndex, BYTE* buffer, DWORD dwBuffSize, BYTE filetype, int *pError)
{
    mpDebugPrint("%s", __FUNCTION__);

    if (pdwFileIndex == NULL || pError == NULL || buffer == NULL) 
    {
        mpDebugPrint("pError==NULL, or buffer==NULL, or pdwFileIndex==NULL");
        return 0;
    }
    if (dwBuffSize == 0)
        return 0;
    if (!spifs_inited)
        return 0;
    
    //////////////////////
    int iEntryId;
    SPI_FILE_ENTRY * entry;
    WORD pageid, oldid;
    WORD writePages, page;
    DWORD size, writeSize;
    DWORD fcnt;

    iEntryId = _spifs_GetNullEntry();
    if (iEntryId < 0) {
        mpDebugPrint("SPIFS - No empty file entry.");
        *pError = ERR_SPIFS_NO_EMPTY_ENTRY;
        return 0;
    }

    if (dwBuffSize > wFreePage * SPI_PAGE_SIZE)
    {
        mpDebugPrint("No enough space to save file");
        *pError = ERR_SPIFS_NO_ENOUGH_SPACE;
        return 0;
    }

    entry = & (spi_fat.fentry[iEntryId]);
    SystemTimeGet(&entry->time);
    entry->filetype = filetype;
    entry->filesize = 0;
    entry->flag = SPI_FS_ENTRY_EXIST_FILE;

    pageid = _spifs_AllocateNewPage();
    if (pageid == (WORD)-1)
    {
        mpDebugPrint("No free page");
        *pError = ERR_SPIFS_NO_FREE_PAGE;
        return 0;
    }
    entry->start_page_id = pageid;
    spi_fat.tables[pageid] = SPI_FS_PAGE_EOF;

    writePages = dwBuffSize / SPI_PAGE_SIZE;
    if (dwBuffSize % SPI_PAGE_SIZE)
        writePages ++;

    fcnt = 0;
    writeSize = dwBuffSize;
    for (page = 0; page < writePages; page++)
    {
        size = SPI_PAGE_SIZE;
        if (writeSize < size)
            size = writeSize;
        
        spifs_WritePage(pageid, &buffer[fcnt]);
        if (wFreePage)
            wFreePage --;
        
        oldid = pageid;

        pageid = _spifs_AllocateNewPage();
        if (pageid == (WORD)-1)
        {
            mpDebugPrint("No free page");
            *pError = ERR_SPIFS_NO_FREE_PAGE;
            return fcnt;
        }
        spi_fat.tables[oldid] = pageid;
        spi_fat.tables[pageid] = SPI_FS_PAGE_EOF;
        
        fcnt += size;
        writeSize -= size;
        entry->filesize += size;

        //if (wFreePage==0)
        //{
        //}
    }

    wFreePage = spifs_GetFreePageCount();                   // re-calculate free page count
    mpDebugPrint("write filesize = %d", entry->filesize);

    spifs_WriteFileInfoBlock((BYTE*)(&spi_fat));
    *pError = 0;
    
    return fcnt;
}

DWORD spifs_DeleteFile(DWORD dwFileIndex, BOOL isWriteBackFAT)
{
    WORD old_id, id;
    SPI_FILE_ENTRY * entry;

    if (!spifs_inited)
        return 0;
    if (dwFileIndex >= SPI_MAX_FILE_COUNT) {
        mpDebugPrint("out of file count, in %s()", __FUNCTION__);
        return 0;
    }

    if (dwFileIndex == 0){
        mpDebugPrint("Cannot delete file id==0.");
        return 0;
    }

    entry = & (spi_fat.fentry[dwFileIndex]);
    if (entry->flag != SPI_FS_ENTRY_EXIST_FILE)
    {
        mpDebugPrint("The entry %d not exist file.", dwFileIndex);
        return 0;
    }

    entry->flag = SPI_FS_ENTRY_NULL;
    entry->filesize = 0;
    id = entry->start_page_id;
    entry->start_page_id = SPI_FS_PAGE_EOF;

    while (id != SPI_FS_PAGE_EOF)
    {
        old_id = id;
        id = spi_fat.tables[id];
        spi_fat.tables[old_id] = SPI_FS_PAGE_NULL;
        
    }
    
    wFreePage = spifs_GetFreePageCount();               // re-calculate free page count

    if (isWriteBackFAT)
        spifs_WriteFileInfoBlock((BYTE*)(&spi_fat));
    
    return 0;
}

static void _CopySearchInfoItem(ST_SEARCH_INFO* pDest, ST_SEARCH_INFO* pSrc)
{
	int i;
	BYTE * pbDest, *pbSrc;

	pbDest = (BYTE*)pDest;
	pbSrc = (BYTE*)pSrc;
	for ( i = 0; i < sizeof(ST_SEARCH_INFO); i++ )
		pbDest[i] = pbSrc[i];
}

static void _SwapSearchInfoItem(ST_SEARCH_INFO* item1, ST_SEARCH_INFO* item2)
{
	ST_SEARCH_INFO temp;
	_CopySearchInfoItem(&temp, item1);
	_CopySearchInfoItem(item1, item2);
	_CopySearchInfoItem(item2, &temp);
}

static int _CompareSearchInfoItem(ST_SEARCH_INFO* item1, ST_SEARCH_INFO* item2) // if item1 smaller than item2, return non-zero
{
    DWORD dwIndex1, dwIndex2;
    SPI_FILE_ENTRY *entry1, *entry2;
    dwIndex1 = *((DWORD*)(item1->bName));
    dwIndex2 = *((DWORD*)(item2->bName));

    entry1 = & (spi_fat.fentry[dwIndex1]);
    entry2 = & (spi_fat.fentry[dwIndex2]);

    if (entry1->time.u32TimeStamp < entry2->time.u32TimeStamp)
        return 1;
    else if (entry1->time.u32TimeStamp > entry2->time.u32TimeStamp)
        return 0;
    else
    {
        if (dwIndex1 < dwIndex2)
            return 1;
        else
            return 0;
    }
}

static void _SortSPISearchInfoItems(ST_SEARCH_INFO *pList, DWORD dwTotalCnt)
{
	int i, j;

	if ( dwTotalCnt == 0 )
		return;
	for ( j = (int)(dwTotalCnt - 1); j > 0; j-- )
	{
		for ( i = 0; i < j; i++ )
		{
			if ( _CompareSearchInfoItem(&(pList[i]), &(pList[i+1])) )
				_SwapSearchInfoItem(&(pList[i]), &(pList[i+1]));
		}
	}
}


SDWORD spifs_FileExtSearch(BYTE filetype, ST_SEARCH_INFO * sSearchInfo, DWORD dwMaxElement, DWORD * pdwRecordElement)
{
    DWORD dwIndex;
    SPI_FILE_ENTRY * entry;
    DWORD dwFileCount;
    ST_SEARCH_INFO * pstWorkingInfo;
        
    if (sSearchInfo == NULL || pdwRecordElement == NULL)  {
        mpDebugPrint("sSearchInfo==NULL or pdwRecordElement==NULL");
        return FAIL;
    }
    
    if (!spifs_inited) {
        return FAIL;
    }

    dwFileCount = 0;
    for (dwIndex = 0; dwIndex < SPI_MAX_FILE_COUNT; dwIndex++)
    {
        entry = & (spi_fat.fentry[dwIndex]);
        if (entry->flag == SPI_FS_ENTRY_NULL)
            continue;
        if (entry->flag == SPI_FS_ENTRY_FORMATED)
            continue;
        if (entry->flag == SPI_FS_ENTRY_EXIST_FILE && entry->filetype == filetype)
        {
            DWORD * pdwSaveIndex;
            pstWorkingInfo = & sSearchInfo[dwFileCount];
            pdwSaveIndex = (DWORD*)(&(pstWorkingInfo->bName[0]));
            *pdwSaveIndex = dwIndex;
            dwFileCount++;
            if (dwFileCount >= dwMaxElement)
                break;
        }
    }
    _SortSPISearchInfoItems(sSearchInfo, dwFileCount);

    *pdwRecordElement = dwFileCount;
    return FS_SUCCEED;
}

void GetFileTime(DWORD dwFileIndex, ST_SYSTEM_TIME *FileTime)
{
	SPI_FILE_ENTRY *entry1, *entry2;
	ST_SEARCH_INFO* item1  = FileGetSearchInfo(dwFileIndex);
	DWORD dwIndex1;
	dwIndex1 = *((DWORD*)(item1->bName));
	entry1 = & (spi_fat.fentry[dwIndex1]);
	SystemTimeSecToDateConv(entry1->time.u32TimeStamp,FileTime);

}


#endif























