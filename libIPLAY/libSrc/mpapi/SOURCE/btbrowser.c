
#define LOCAL_DEBUG_ENABLE 0

#define _BT_USE_ORI_FUNC_FLOW_    0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "taskid.h"
#include "ui.h"
#include "mpapi.h"
#include "filebrowser.h"
#include "os.h"
#include "BtApi.h"


#pragma alignvar(4)

static STREAM *FileHandle;

static DWORD TotalByte;
static DWORD WriteOffset;
static DWORD ReadOffset;
static DWORD ReadRemain;
static BOOL blFileCreated = 0;


#define xpgStringBuffer_len  (MAX_L_NAME_LENG * 2)
BYTE xpgStringBuffer[xpgStringBuffer_len]; /* for max 256 UTF-16 characters */


#if _BT_USE_ORI_FUNC_FLOW_
static BOOL StringCompare(void * string, WORD stringlen, void * name);
static int BT_LongNameCopy(DRIVE * drv, WORD * longname, FDB * node);
int BT_CdParent(DRIVE * drv);

static BYTE BT_Browser_Get_LNFDB_Count(WORD name_size);

static BYTE Check_Chinese(WORD * name, WORD len);
static BYTE Check_Lower_Case(WORD *name, WORD len, BYTE dot, BYTE type);
static BYTE Check_blank(WORD * name, WORD len);
static void Change_File_Lower_Case_Name_2_Upper(BYTE * bNode_Name, WORD * Name, WORD namesize, BYTE dot);
static void Change_Lower_Case_Name_2_Upper(BYTE * bNode_Name, WORD * Name, WORD namesize);
static int BT_Browser_Make_Dir(DRIVE * drv, WORD * name, WORD namesize);
static int BT_Current_Dir_Size_Reset(DRIVE * drv);
static SWORD BT_FileGetLongName(DRIVE * Drv, ST_SEARCH_INFO * sSearchInfo, BYTE * pbBuf);
int BT_CreateFile(DRIVE * drv, WORD * full_name, WORD name_size);

 #if 0 // Not used functions, can mark now ------------- [begin]
  static DWORD BT_Browser_Get_Info_Index(void);
  static BOOL BT_Browser_Is_In_Root(void);
  BOOL BT_Browser_Is_Sinfo_Init(void);
  void HLSwitch(WORD * buf, WORD size);
  void Change_Name_Byte_2_Word(BYTE * pbName, WORD namesize, WORD * pwbuf, WORD * word_name_size);
  BYTE *Change_Name_Word_2_Byte(WORD * pwbuf, WORD * namesize);
 #endif // Not used functions, can mark now ------------- [end]

#endif //_BT_USE_ORI_FUNC_FLOW_


static BT_FileBrowserGetFileName(ST_SEARCH_INFO * psSearchList, BYTE * pbNameBuffer);
static SWORD BT_Browser_Refresh_File_List(void);
static void BT_Browser_Reset_Info_Index(void);
static BOOL BT_Browser_Name_Search(BYTE * pbSourceName, DWORD * filesize, BYTE type, WORD namesize);
static BYTE Check_Dot(WORD * name, WORD len);

void BtBrowserFileClose(STREAM * phandle);
BYTE *BT_Browser_Get_Cur_Info_name(BYTE type);
BOOL BT_Browser_Next_Info_Index(void);
STREAM *BtBrowserFileCreate(WORD * pwSname);
BOOL BT_Browser_Switch_2_Root(void);
SWORD BT_Browser_Create_New_Folder(WORD * name, WORD len);
SWORD BT_Browser_Switch_2New_Folder(BYTE * Sname, BYTE type, WORD namesize, BYTE Flags);
BYTE BT_Browser_Get_Cur_Type(void);
void BT_Browser_Decimal2Char(DWORD size, BYTE * buffer, BYTE arrysize);
DWORD BT_Browser_Cur_File_Size(void);
SWORD BtApiFstoreOpen(DWORD handle,BYTE * pbName, DWORD * filesize, BYTE namesize);
SWORD BtApiFstoreDelete(BYTE * Name, BYTE namesize);
DWORD BtApiFstoreRead(DWORD handle,BYTE * pbuff, DWORD size);
DWORD BtApiFstoreWrite(DWORD handle,BYTE * buffer, DWORD len);


// Bt special

#if M_PROFILE
static BYTE * pwBuf;
static BYTE *prBuf;
#endif

DWORD BtApiFstoreWrite(DWORD handle,BYTE * buffer, DWORD len)
{
    DWORD dwByteCount = 0;
    DWORD dwleft;
    BYTE *pBackbuf = NULL;
    BYTE *pSourcebuf = buffer;
    BYTE offset;
#if M_WRITE


#if M_PROFILE
    pBackbuf = pwBuf = GetBufByDlci(GetCurrentDlci());
    if(pBackbuf == 0)
    {
        mpDebugPrint("BtApiFstoreWrite get buffer by dlci fail");
        return 0;
    }
#else//M_PROFILE
    pBackbuf = BTA(pWBuf0);
#endif//M_PROFILE

    dwleft = (MAX_FILE_LENGTH - WriteOffset); 
    if (dwleft >= len)
    {
        memcpy(pBackbuf + WriteOffset,buffer,len);
        WriteOffset += len;
        TotalByte += len;
        return len;
    }
    else
    {
        memcpy(pBackbuf + WriteOffset, buffer, dwleft);
        WriteOffset += dwleft;
        dwByteCount = FileWrite((STREAM *)handle, pBackbuf, WriteOffset); 
        if (dwByteCount == WriteOffset)
        {
            WriteOffset = 0;
            memset(pBackbuf, 0, MAX_FILE_LENGTH);
            memcpy(pBackbuf + WriteOffset, (buffer + dwleft), (len - dwleft));
            WriteOffset += (len - dwleft);
            TotalByte += len;
            return len;
        }
        else
        {
            MP_ALERT("%s: FileWrite() failed !\r\n", __FUNCTION__);
            return 0;
        }
    }

    if (dwByteCount == len)
    {
        return dwByteCount;
    }    
#else//M_WRITE
        dwByteCount = FileWrite(handle, buffer, len);     
        if(dwByteCount == len)
        {
            return len;
        }
        else
        {
            mpDebugPrint("BtApiFstoreWrite fail");
            return 0;
        }
#endif//M_WRITE
}


//rick add 0610
SWORD BtCheckSpace(DWORD dwSize)
{
    DRIVE *drv = FileBrowserGetCurDrive();
    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }
    else
        return CheckSpaceIfEnough(drv, dwSize);
}



//////////// Original BTbrowser functions, redundant w.r.t file system APIs [begin] ////////////
#if _BT_USE_ORI_FUNC_FLOW_

// local variable for create file
const SBYTE _ReplaceGlyph[] = "+,;=[]";

// subfunc of btbrowser
BOOL StringCompare(void * string, WORD stringlen, void * name)
{
    SBYTE *pstring;
    WORD *pname;
    WORD i, j;
    pstring = (SBYTE *)string;
    pname = (WORD *)name;
    i=0;
    while (pname[i] != 0)
    {
        for (j=0; j < stringlen; j++)
        {
            if (pstring[j] == (BYTE)(pname[i] & 0x00ff))
                return TRUE;
        }
        i++;
    }
    return FALSE;
}


// can be replaced by CdParent()
int BT_CdParent(DRIVE * drv)
{
    MP_DEBUG("BT_CdParent");
    WORD i = 0;
    DWORD start, cluster, size, point;
    CHAIN *dir;

    if (!drv->DirStackPoint) // at root dir
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
        size = 0;
        cluster = start;
        while (cluster != 0xffffffff)
        {
            if (! SystemCardPresentCheck(drv->DrvIndex))
            {
                MP_ALERT("%s: Card not present !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }

            size += drv->ClusterSize;
            cluster = drv->FatRead(drv, cluster);

            if (drv->StatusCode == FS_SCAN_FAIL)
            {
                MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
        }
        size = size << drv->bSectorExp;
        ChainInit(dir, start, size);
        ChainSeekForward(drv, dir, point);
    } 

    if (DirCaching(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;

    return FS_SUCCEED;
}


// subfunc of btbrowser
int BT_LongNameCopy(DRIVE * drv, WORD * longname, FDB * node)
{
    LONG_NAME lnode;
    DWORD count, length, length0;
    WORD *point16;
    BYTE *point08, checksum, i, *j;

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

    length = StringLength16(longname);
    if (length % 13)
        length += 1;

    count = 0;
    while (length > 13)
    {
        count++;
        length -= 13;
    }
    memset(&lnode, 0xff, sizeof(LONG_NAME));
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


// subfunc of btbrowser
BYTE BT_Browser_Get_LNFDB_Count(WORD name_size)
{
    if (name_size / 13)
        return ((name_size / 13) + 1);
    else
    {
        if (name_size > 8)
            return 1;
        else
            return 0;
    }
}


// subfunc of btbrowser
BYTE Check_Chinese(WORD * name, WORD len)
{
    BYTE i;
    WORD wlen;
    BYTE ch = 0;

    i = 0;
    wlen = len;
    while (wlen)
    {
        wlen -= 2;
        if ((name[i++] & 0xff00) != 0)
        {        
            ch = 1;
            return ch;
        }
    }
    return ch;
}


// subfunc of btbrowser
BYTE Check_Lower_Case(WORD * name, WORD len, BYTE dot, BYTE type)
{
    BYTE i;
    WORD wlen;
    BYTE lcase = 0;

    i = 0;
    wlen = len;
    while (wlen)
    {
        if ((type == SEARCH_INFO_FILE) && (name[i] == '.'))
        {
            if (dot > 1)
                dot--;
            else
                return lcase;
        }
        if ((lcase & 0x03) == 0x03)
        {
            return lcase;
        }
        if ((name[i] >= 'a') && (name[i] <= 'z'))
            lcase |= 0x01;
        else if ((name[i] >= 'A') && (name[i] <= 'Z'))
            lcase |= 0x02;
        i++;
        wlen -= 2;
    }
    return lcase;
}


// subfunc of btbrowser
BYTE Check_blank(WORD * name, WORD len)
{
    BYTE i;
    WORD wlen;
    BYTE blank = 0;

    i = 0;
    wlen = len;
    while (wlen)
    {
        if ((name[i++] & 0x00ff) == ' ')
        {
            MP_DEBUG("\r\n blank space \r\n");
            blank = 1;
            return blank;
        }
        wlen -= 2;
    }
    return blank;
}


// subfunc of btbrowser
void Change_File_Lower_Case_Name_2_Upper(BYTE * bNode_Name, WORD * Name, WORD namesize, BYTE dot)
{
    WORD nsize = 0;
    WORD byte_index = 0;
    WORD word_index = 0;
    nsize = namesize;

    while ((Name[word_index] != 0) && (byte_index < 8) && (dot >= 1))
    {
        if (Name[word_index] & 0xff00)
        {
            //change unicode to big 5
        }
        else if ((Name[word_index] == '.'))
        {
            if (dot > 1)
            {
                dot--;
                bNode_Name[byte_index] = (BYTE)Name[word_index];
            }
            else
                break;
        }
        else if ((Name[word_index] >= 'a') && (Name[word_index] <= 'z'))
        {
            bNode_Name[byte_index] = (BYTE)Name[word_index] - 0x20;
        }
        else
            bNode_Name[byte_index] = (BYTE)Name[word_index];

        word_index++;
        byte_index++;
        nsize -= 2;
    }

//    while (Name[word_index++]!= '.');
    while (dot >= 1)
    {
        if(Name[word_index]=='.')
        {
            dot--;
        }
        word_index ++;
        nsize -= 2;
    }
    byte_index = 0;
//    nsize = 3;// sometime extension name not nly have 3 character
    while ((Name[word_index] != 0) && (nsize > 0))
    {
        if ((Name[word_index] >= 'a') && (Name[word_index] <= 'z'))
            bNode_Name[byte_index+8] = (BYTE)Name[word_index]-0x20;
        else
            bNode_Name[byte_index+8] = (BYTE)Name[word_index];

        word_index++;
        byte_index++;
        nsize -= 2;
    }
}


// subfunc of btbrowser
void Change_Lower_Case_Name_2_Upper(BYTE * bNode_Name, WORD * Name, WORD namesize)
{
    WORD nsize = 0;
    WORD byte_index = 0;
    WORD word_index = 0;
    nsize = namesize;

    while ((Name[word_index] != 0) && (byte_index < 8))
    {
        if (Name[word_index] & 0xff00)
        {
//            bNode_Name[byte_index]= (BYTE)((Name[word_index]&0xff00) >> 8);
//            byte_index++;
//            bNode_Name[byte_index]= (BYTE)Name[word_index] & 0x00ff;
        }
        else if((Name[word_index] >= 'a')&&(Name[word_index] <= 'z'))
        {
            bNode_Name[byte_index] = (BYTE)Name[word_index] - 0x20;
        }
        else
            bNode_Name[byte_index] = (BYTE)Name[word_index];

        word_index++;
        byte_index++;
        nsize -= 2;
    }
}


// local variable of btbrowser.c
WORD LNBuf[MAX_L_NAME_LENG];
DWORD LongNameFdbCount;


// can be replaced by MakeDir_UTF16()/MakeDir()
int BT_Browser_Make_Dir(DRIVE * drv, WORD * name, WORD namesize)
{
    CHAIN *dir;
    volatile FDB *node;
    FDB *pFnode;
    FDB sFnode;
    FDB tempnode;
    DWORD addr, i, dwLNchang;
    BYTE *pbname = (BYTE *)name;
    BYTE lowwer_case = 0;
    BYTE blank_space = 0;    
    BYTE chinese = 0;
    BYTE long_name = 0;
    BYTE dot = 0;
    WORD *pwname = LNBuf;

    pFnode = (FDB *) (&sFnode.Name[0]);
    BT_Current_Dir_Size_Reset(drv);
    dot = Check_Dot(name, namesize);
    chinese = Check_Chinese(name, namesize);
    blank_space = Check_blank(name, namesize);
    lowwer_case = Check_Lower_Case(name, namesize, dot, SEARCH_INFO_FOLDER);
    if (BT_Browser_Get_LNFDB_Count((namesize - 2) >> 1) || (chinese == 1) || (blank_space == 1) || (lowwer_case == 3))
    {
        long_name = 1;
    }
    memset(&pFnode->Name, 0x20, 11);

    Change_Lower_Case_Name_2_Upper((BYTE *)&pFnode->Name, name, namesize);
    if (long_name)
    {
        pFnode->Name[6] = '~';
        pFnode->Name[7] = '1';
        lowwer_case = 0;
        if (LongNameCopy(FileBrowserGetCurDrive(), name, pFnode) != FS_SUCCEED)
        {
            MP_ALERT("%s: LongNameCopy() failed !\r\n", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    }

    // set the sub dir attribure and give it the address            
    addr = DriveNewClusGet(drv);
    if (addr == 0xffffffff)
    {
        MP_ALERT("%s: DISK FULL !\r\n", __FUNCTION__);
        return DISK_FULL;
    }
    if (drv->FatWrite(drv, addr, 0xffffffff) != FS_SUCCEED)
    {
        MP_ALERT("%s: drv->FatWrite() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    CheckDeletedCount(1);
    if (GetNewNode(drv) != FS_SUCCEED)
    {
        if (GetDeletedNode(drv) != FS_SUCCEED)
        {
            MP_ALERT("%s: GetDeletedNode() failed !\r\n", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    }
    node = (volatile FDB *) (drv->Node);
    memcpy(&node->Name, &pFnode->Name, 11);
    node->Attribute = FDB_SUB_DIR;
    if (lowwer_case == 1)
    {
        node->Attribute2 |= 0x0800;
    }
    drv->Flag.DirCacheChanged = 1;
    SaveAlien16((void *) &node->StartLow, addr);
    SaveAlien16((void *) &node->StartHigh, addr >> 16);
    memcpy(&tempnode, (char *) node, sizeof(FDB));
    // reset the the new sub-directory's cluster
    addr = drv->DataStart + ((addr - 2) << drv->ClusterExp);
    if (SectorClear(drv, addr, drv->ClusterSize) != FS_SUCCEED)
    {
        MP_ALERT("%s: SectorClear() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    // get the current name
    ScanFileName(drv);
    if (CdSub(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: CdSub() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;	// change to the new sub-directory
    }
    node = (FDB *) memcpy((char *) drv->DirCacheBuffer, &tempnode, sizeof(FDB));
    node = memset((char *) node, 0x20, 11);
    node->Name[0] = '.';

    node = (FDB *) memcpy((char *) (node + 1), (char *) node, sizeof(FDB));
    node->Name[1] = '.';

    // if the parent dir that '..' point to is root then it's value must be 0
    // else point to the parent directory's start cluster
    if (drv->DirStackPoint - sizeof(CHAIN))
    {
        dir = (CHAIN *) ((BYTE *) drv->DirStackBuffer + drv->DirStackPoint - sizeof(CHAIN));
        addr = dir->Start;
        SaveAlien16((void *) &node->StartLow, addr);
        addr >>= 16;
        SaveAlien16((void *) &node->StartHigh, addr);
    }
    else
    {
        node->StartLow = 0;
        node->StartHigh = 0;
    }

    drv->Flag.DirCacheChanged = 1;
    if (BT_CdParent(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: BT_CdParent() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
    if (DriveRefresh(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: DriveRefresh() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;
    }
    LongNameFdbCount = 0;
    return FS_SUCCEED;
}


// subfunc of btbrowser
int BT_Current_Dir_Size_Reset(DRIVE * drv)
{
    DWORD start, size,point;
    CHAIN *dir;
    dir = (CHAIN *)((DWORD)drv->DirStackBuffer + drv->DirStackPoint);

  #if EXFAT_ENABLE
    if ((drv->Flag.FsType == FS_TYPE_FAT32) || (drv->Flag.FsType == FS_TYPE_exFAT))
  #else
    if (drv->Flag.FsType == FS_TYPE_FAT32)
  #endif
    {
        size = 0;
        //start = drv->RootStart;//rick
        start = dir->Start;
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
                MP_ALERT("%s: the cluster number == 0 !!", __FUNCTION__);
                return ABNORMAL_STATUS;
            }
        }

        //start = drv->RootStart;//rick
        start = dir->Start;
        size = size << drv->bSectorExp;		// byte size
    }
    else
    {
        if(drv->DirStackPoint == 0)
        {
            size = (drv->DataStart - drv->RootStart) << drv->bSectorExp;	// byte size
            //start = 0;
        }
        else
        {
            size = dir->Size;
        }
    }
    start = dir->Start;
    point = dir->Point;
    ChainInit((CHAIN *) dir, start, size);
    ChainSeekForward(drv, dir, point);

//    if (DirCaching(drv) != FS_SUCCEED)
//        return DRIVE_ACCESS_FAIL;

    return FS_SUCCEED;
}


// subfunc of btbrowser
SWORD BT_FileGetLongName(DRIVE * Drv, ST_SEARCH_INFO * sSearchInfo, BYTE * pbBuf)
{
    register STREAM *psHandle;
    register DWORD dwLongFdbCount, i,j,k, dwBufCount;
    register LONG_NAME *sLongName;
    int ret;
    BYTE *pbBuffer;

    psHandle = FileListOpen(Drv, sSearchInfo);
    if (!psHandle)
    {
        MP_ALERT("%s: FileListOpen() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    dwBufCount = 0;
    dwLongFdbCount = sSearchInfo->dwLongFdbCount;
    while (dwLongFdbCount)
    {
        ret = PreviousNode(Drv);
        if (ret != FS_SUCCEED)	// begin of FDB or access fail
        {
            FileClose(psHandle);
            MP_ALERT("%s: PreviousNode() failed !\r\n", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
        sLongName = (LONG_NAME *) (psHandle->Drv->Node);
        for (j=0; j < 3; j++)
        {
            if (j==0)
            {
                pbBuffer = (BYTE *)&sLongName->Name0;
                k = 10;
            }
            else if (j == 1)
            {
                pbBuffer = (BYTE *)&sLongName->Name1;
                k = 12;
            }
            else if (j == 2)
            {
                pbBuffer = (BYTE *)&sLongName->Name2;
                k = 4;
            }            

            for (i = 0; i < k; i += 2)
            {
                if (((pbBuffer[i] == 0x00) && (pbBuffer[i + 1] == 0x00))|| (dwBufCount > (xpgStringBuffer_len - 2)))
                {
                    pbBuf[dwBufCount] = 0x00;
                    pbBuf[dwBufCount + 1] = 0x00;
                    FileClose(psHandle);
                    return FS_SUCCEED;
                }
                pbBuf[dwBufCount + 1] = pbBuffer[i];
                pbBuf[dwBufCount] = pbBuffer[i + 1];
                dwBufCount += 2;
            }
        }
        dwLongFdbCount -= 1;
    }
    pbBuf[dwBufCount] = 0x00;
    pbBuf[dwBufCount + 1] = 0x00;
    FileClose(psHandle);
    return FS_SUCCEED;
}


int BT_CreateFile(DRIVE * drv, WORD * full_name, WORD name_size)
{
    FDB *pFnode;
    FDB sFnode;    
    LONG_NAME *sLongName;
    DWORD dwAddr, filesize;
    SWORD i, j;
    BYTE bChkSum;
    BOOL blLossyFlag = 0;
    BYTE chinese = 0, blank_space = 0, lowwer_case = 0, long_name = 0, dot = 0;
    BYTE *pbuf;
    pFnode = (FDB *) (&sFnode.Name[0]);

    pbuf = full_name;
 #if 0
    mpDebugPrint("\r\n BT_CreateFile %s %d", full_name, name_size);
    for (i=0; i < 20; i++)
    {
        UartOutValue(pbuf[i], 2);
        UartOutText(" ");
    }
    mpDebugPrint("\r\n");
 #endif

    if (BT_Browser_Name_Search((BYTE *)full_name, (DWORD *)&filesize, (SEARCH_INFO_FILE), name_size - 2) == 1)
    {
        if (BtApiFstoreDelete((BYTE *)full_name, (BYTE)name_size) == FAIL)
        {
            MP_ALERT("%s: BtApiFstoreDelete() failed !\r\n", __FUNCTION__);
            return FAIL;
        }
    }

    BT_Current_Dir_Size_Reset(drv);
    dot = Check_Dot(full_name, name_size);
    chinese = Check_Chinese(full_name, name_size);
    
    blank_space = Check_blank(full_name, name_size);
    
    lowwer_case = Check_Lower_Case(full_name, name_size, dot, SEARCH_INFO_FILE);
    
    blLossyFlag = StringCompare((void *)_ReplaceGlyph, 6, full_name);
    
    if ((name_size > 26) || (chinese == 1) || (blank_space == 1) || (lowwer_case == 3) || (blLossyFlag == 1))
    {
        MP_DEBUG("\r\n long name \r\n");
        long_name = 1;
    }
    memset(&pFnode->Name, 0x20, 11);
    Change_File_Lower_Case_Name_2_Upper((BYTE *)&pFnode->Name, full_name, name_size, dot);
    if (long_name == 1)
    {
        pbuf = (BYTE *)full_name;
        for (i = 0; i < (name_size-2); i++)
        {
            bChkSum = ((bChkSum & 1) ? 0x80 : 0) + (bChkSum >> 1) + pbuf[i];
        }
        pFnode->Name[6] = '~';
        pFnode->Name[7] = bChkSum;
        lowwer_case = 0;
        bChkSum = 0;
        if (BT_LongNameCopy(FileBrowserGetCurDrive(), full_name, pFnode) != FS_SUCCEED)
        {
            MP_ALERT("%s: BT_LongNameCopy() failed !\r\n", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    }

    if (GetNewNode(drv))
    {
        if (GetDeletedNode(drv))
        {
            MP_ALERT("%s: GetDeletedNode() failed !\r\n", __FUNCTION__);
            return ABNORMAL_STATUS;
        }
    }
    memcpy((FDB *) (drv->Node),pFnode,11);
    dwAddr = DriveNewClusGet(drv);
    if (dwAddr == 0xffffffff)
    {
        MP_ALERT("%s: DISK FULL !\r\n", __FUNCTION__);
        return DISK_FULL;
    }

    drv->Node->Attribute = FDB_ARCHIVE;

    if (lowwer_case == 1)
        drv->Node->Attribute2 |= 0x0800;
    if (long_name == 0)
        drv->Node->Attribute2 |= 0x1000;    

    SaveAlien16((void *)&drv->Node->StartLow, dwAddr);
    dwAddr >>= 16;
    SaveAlien16((void *)&drv->Node->StartHigh, dwAddr);
    SaveAlien32((void *)&drv->Node->Size, 0);
    drv->Flag.DirCacheChanged = 1;

    if (DriveRefresh(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: DriveRefresh() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    return FS_SUCCEED;
}

#endif //_BT_USE_ORI_FUNC_FLOW_
//////////// Original BTbrowser functions, redundant w.r.t file system APIs [end] ////////////



#if _BT_USE_ORI_FUNC_FLOW_
// shall be replaced by FileBrowserGetFileName()
BT_FileBrowserGetFileName(ST_SEARCH_INFO * psSearchList, BYTE * pbNameBuffer)
{
    DWORD tempIndex;
    BYTE *pbTempBuffer = (BYTE *) pbNameBuffer;
    WORD *pwTempBuffer = (WORD *) pbNameBuffer;
    DRIVE *pCurDrive = FileBrowserGetCurDrive();

    if (pCurDrive == NULL || psSearchList == NULL)
    {
        MP_ALERT("%s: Null pointer !\r\n", __FUNCTION__);
        return FAIL;
    }

    if (psSearchList->bParameter & SEARCH_INFO_CHANGE_PATH) 
    {
        BYTE *pbUF = "Upper Folder";

        memcpy(pbTempBuffer, pbUF, StringLength08(pbUF));
        return PASS;
    }

    if (psSearchList->dwLongFdbCount == 0)
    {
        for (tempIndex = 0; (tempIndex < 8) && (psSearchList->bName[tempIndex] != 0x20);tempIndex++)
        {
            pwTempBuffer[tempIndex] = psSearchList->bName[tempIndex];
        }

        if ( (psSearchList->bExt[0] != 0) && (psSearchList->bExt[0] != 0x20)) 
        {
            pwTempBuffer[(tempIndex + 0)] = '.';
            pwTempBuffer[(tempIndex + 1)] = psSearchList->bExt[0];
            pwTempBuffer[(tempIndex + 2)] = psSearchList->bExt[1];
            pwTempBuffer[(tempIndex + 3)] = psSearchList->bExt[2];
            pwTempBuffer[(tempIndex + 4)] = 0;
        }
        else
        {
            pwTempBuffer[(tempIndex + 0)] = 0;
        }
    }
    else
    {
    #if _BT_USE_ORI_FUNC_FLOW_
        if (FS_SUCCEED != BT_FileGetLongName(pCurDrive, psSearchList, pbTempBuffer))
    #else
        DWORD pdwBufferCount;
        if (FS_SUCCEED != FileGetLongName(pCurDrive, psSearchList, pbTempBuffer, &pdwBufferCount, xpgStringBuffer_len))
    #endif
        {
            MP_ALERT("%s: get long filename failed !\r\n", __FUNCTION__);
            return FAIL;
        }
    }
    return PASS;
}
#else
/* get full filename of the ST_SEARCH_INFO entry, and convert it to UTF-16 string */
/* note: here, buffer length should be xpgStringBuffer_len */
BT_FileBrowserGetFileName(ST_SEARCH_INFO * psSearchList, BYTE * pbNameBuffer)
{
#define EXT_NAME_LENG  5  /* for FileBrowserGetFileName() to consider ex: ".JPG" and string null terminator */
    BYTE Ext[EXT_NAME_LENG];
    BYTE tmpBuf[xpgStringBuffer_len];
    MpMemSet(Ext, 0, EXT_NAME_LENG);
    MpMemSet(tmpBuf, 0, xpgStringBuffer_len);

    if (psSearchList == NULL)
    {
        MP_ALERT("%s: Null pointer !\r\n", __FUNCTION__);
        return FAIL;
    }

    if (psSearchList->bParameter & SEARCH_INFO_CHANGE_PATH) 
    {
        BYTE *pbUF = "Upper Folder";

        memcpy(pbNameBuffer, pbUF, StringLength08(pbUF));
        return PASS;
    }

    if (FileBrowserGetFileName(psSearchList, tmpBuf, xpgStringBuffer_len, Ext) != PASS)
    {
        MP_ALERT("%s: FileBrowserGetFileName() failed !", __FUNCTION__);
        return FAIL;
    }

    MpMemSet(pbNameBuffer, 0, xpgStringBuffer_len);
    if (psSearchList->dwLongFdbCount > 0) /* tmpBuf[] contains UTF-16 primary part of filename string */
    {
        if (StringLength08(Ext))
            StringNCopy0816((WORD *) tmpBuf + StringLength16((WORD *) tmpBuf), (BYTE *) Ext, StringLength08(Ext)); /* append ASCII extension name to UTF-16 primary name to form UTF-16 full filename */

        MpMemCopy(pbNameBuffer, (BYTE *) tmpBuf, xpgStringBuffer_len);
    }
    else /* tmpBuf[] contains ASCII primary part of filename string */
    {
        if (StringLength08(Ext))
            strcat((BYTE *) tmpBuf, (BYTE *) Ext);

        mpx_UtilUtf8ToUnicodeU16((WORD *) pbNameBuffer, (BYTE *) tmpBuf); /* convert to UTF-16 string */
    }
    return PASS;
}
#endif


// bt special
SWORD BT_Browser_Refresh_File_List(void)
{
    DRIVE *pCurDrive = (DRIVE *)FileBrowserGetCurDrive();
    FileBrowserResetFileList();
    if (DoSearch(pCurDrive, LOCAL_SEARCH_INCLUDE_FOLDER) == PASS)
    {
        return PASS;
    }
    else
    {
        MP_ALERT("%s: DoSearch() failed !", __FUNCTION__);
        return FAIL;
    }
}


// bt special
void BtBrowserFileClose(STREAM * phandle)
{
    BYTE *pbuf;

#if M_PROFILE
    pbuf = GetBufByDlci(GetCurrentDlci());
#else//M_PROFILE
    pbuf = BTA(pWBuf0);
#endif//M_PROFILE
    DWORD dwWrite;
    if (blFileCreated == 1)
    {
        if (MAX_FILE_LENGTH > WriteOffset)
        {
            dwWrite = FileWrite(phandle, pbuf, WriteOffset); 
            if (dwWrite != WriteOffset)
            {
                MP_ALERT("%s: FileWrite() failed !", __FUNCTION__);
            }
        }
    }
    FileClose(phandle);

    blFileCreated = 0;
    TotalByte = 0;
    ReadOffset = 0;
    WriteOffset = 0;
    ReadRemain = 0;
#if M_PROFILE
    ResetBufByDlci(GetCurrentDlci());
#endif//M_PROFILE

#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
    if (BTFTPS(ftp_fstore_read) == 0)
        BT_Browser_Refresh_File_List();
#endif
}


// bt special
void BtUnicode2UTF8(BYTE * UTF8, WORD * Uni, DWORD number)
{
    WORD count,index;
    BYTE data0, data1, data2;
    index = 0;
    for (count=0; count < number; count++)
    {
        UTF8[index] = ((Uni[count] & 0xf000) >> 12) | 0xE0;
        index++;
        UTF8[index] = ((Uni[count] & 0x0fc0) >> 6) | 0x80;
        index++;
        UTF8[index] = (Uni[count] & 0x003f) | 0x80;
        index++;
    }
}


// bt special
BYTE *BT_Browser_Get_Cur_Info_name(BYTE type)
{
    ST_SEARCH_INFO *psSearchInfo = FileGetCurSearchInfo();
    BYTE *pbTempBuffer = (BYTE *) ((DWORD) xpgStringBuffer | 0xa0000000);
    BYTE chinese = 0;
    WORD *pwbuf = (WORD *)pbTempBuffer;
    WORD i,j;
    BYTE * pUTF8,*pUTF8Temp;

    memset(pbTempBuffer, 0, xpgStringBuffer_len);
    if (PASS == BT_FileBrowserGetFileName(psSearchInfo, pbTempBuffer))
    {
        if (type == 1)
        {
            i = 0;
            while ((pwbuf[i] != 0) && (i < (xpgStringBuffer_len >> 1)))
            {
                if (pbTempBuffer[i << 1] != 0)
                {
                    chinese = 1;
                    break;
                }
                i++;
            }                

            if (chinese == 1)
            {
                pUTF8Temp = pUTF8 = (BYTE *)ext_mem_malloc(xpgStringBuffer_len * 3 / 2);
                memset((BYTE *)pUTF8, 0, xpgStringBuffer_len * 3 / 2);
                i = 0;
                while (pwbuf[i] != 0)
                {
                    if (pbTempBuffer[i << 1] != 0)
                    {
                        BtUnicode2UTF8(pUTF8Temp, (WORD *) (pbTempBuffer + i * 2), 1);
                        pUTF8Temp += 3;    
                    }
                    else
                    {
                        *pUTF8Temp = pbTempBuffer[(i << 1) + 1];
                        pUTF8Temp += 1;
                    }
                    i++;
            	}
                memset(pbTempBuffer, 0, xpgStringBuffer_len);
                memcpy(pbTempBuffer, pUTF8, xpgStringBuffer_len);
                ext_mem_free(pUTF8);
                return pbTempBuffer;
            }
            j = 0;
            for (i=0; i < (xpgStringBuffer_len >> 1); i++)
            {
                if ((pwbuf[i] & 0xff00) == 0)
                {
                    pbTempBuffer[j] = (BYTE)pwbuf[i];
                    j++;
                }
            }    
        }
        return pbTempBuffer;
    }
    else
    {
        MP_ALERT("%s: BT_FileBrowserGetFileName() failed !", __FUNCTION__);
    }
}


// bt special
void BT_Browser_Reset_Info_Index(void)
{
    register ST_SYSTEM_CONFIG *psSysConfig;
    psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    psFileBrowser->dwImgAndMovCurIndex = 0;
}


// bt special
BOOL BT_Browser_Next_Info_Index(void)
{
    register ST_SYSTEM_CONFIG *psSysConfig;
    psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;

    psFileBrowser->dwImgAndMovCurIndex += 1;
    if (psFileBrowser->dwImgAndMovCurIndex >= psFileBrowser->dwImgAndMovTotalFile)
        psFileBrowser->dwImgAndMovCurIndex = 0;  
    if (psFileBrowser->dwImgAndMovCurIndex == 0)
        return FALSE;
    else
        return TRUE; // still have file not deliver
}


// subfunc of btbrowser
BOOL BT_Browser_Name_Search(BYTE * pbSourceName, DWORD * filesize, BYTE type, WORD namesize)
{
    BOOL found = 0;
    BYTE index, temp1, temp2;
    BYTE dot_count = 0, extension = 0;
    BYTE *pbSearchName;
    DRIVE *drv;
    ST_SEARCH_INFO *psSearchInfo;

    index = 0;
    BT_Browser_Reset_Info_Index();
    drv = FileBrowserGetCurDrive();
    while (found == 0)
    {
        psSearchInfo = FileGetCurSearchInfo();
        pbSearchName = BT_Browser_Get_Cur_Info_name(0);

    #if _BT_USE_ORI_FUNC_FLOW_
        BT_Current_Dir_Size_Reset(drv);
    #endif

        if (psSearchInfo->bParameter & SEARCH_INFO_CHANGE_PATH)
        {
            //modify "Upper Folder" back to ".."
            memset(pbSearchName, 0, 32); 
            pbSearchName[0] = pbSearchName[2] = ' ';
            pbSearchName[1] = pbSearchName[3] = '.';
        }

        if ((psSearchInfo->bParameter & type) == 0)
            goto L_next;

        found = 1;
        if (psSearchInfo->bParameter & SEARCH_INFO_FILE)
            dot_count = Check_Dot((WORD *)pbSourceName, namesize);
        for (index = 0; index < namesize; index++)
        {
            if ((psSearchInfo->bParameter & SEARCH_INFO_FILE) && (pbSearchName[index]== '.'))
            {
                if (dot_count == 1)
                    extension = 1;
                else
                    dot_count--;
            }
            if (pbSearchName[index] != pbSourceName[index])
            {
                temp1 = pbSearchName[index];
                temp2 = pbSourceName[index];
                if ((((temp1 > temp2) && (temp1 - temp2 == 0x20)) || ((temp2 > temp1) && (temp2 - temp1 == 0x20))) && (extension == 1))
                { // jpg should be equle to JPG case
                    //MP_DEBUG("\r\n compare extension part\r\n");    
                }
                else
                {
                    found = 0;
                    break;
                }
            }
        }

        // This is for  existing file name length is longer than file which has been written.
        if (pbSearchName[index] != 0)
            found = 0;
        if (found == 1)
            *filesize = psSearchInfo->dwFileSize;

L_next:
        if (found == 0)
        {
            if (BT_Browser_Next_Info_Index() == 0)
            {
                //MP_DEBUG("\r\nSearch end , not found ...\r\n");
                return 0;
            }
        }
    }
    //MP_DEBUG("found\r\n");

    return found;
}


// bt special
SWORD BtApiFstoreDelete(BYTE * Name, BYTE namesize)
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    ST_SEARCH_INFO *pSearchInfo;
    BYTE bDriveId = psSysConfig->sStorage.dwCurStorageId;
    STREAM *sHandle;
    SWORD swRet = FS_SUCCEED;
    DWORD filesize = NULL;
    static BOOL isDirNeedToRestore = FALSE;
    DWORD dwTotalFile = 0;
    DRIVE *pCurDrive = FileBrowserGetCurDrive();
    STREAM *pHandle;    

    if (pCurDrive == NULL)
    {
        MP_ALERT("%s: Null DRIVE pointer !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    /* check Read-Only flag by underlying Mcard layer info => physical lock latch or Mcard layer lock */
    if (SystemGetFlagReadOnly(bDriveId))
    {
        MP_ALERT("%s: -E- The drive is Read-Only (in Mcard layer) !", __FUNCTION__);
        return DISK_READ_ONLY;
    }

    /* check Read-Only flag by file system layer => logical or controlled by S/W */
    if (pCurDrive->Flag.ReadOnly)
    {
        MP_ALERT("%s: -E- The drive is Read-Only (controlled in file system layer) !", __FUNCTION__);
    #if EXFAT_ENABLE
        if (pCurDrive->Flag.FsType == FS_TYPE_exFAT)
        {
            MP_DEBUG("%s: The drive is exFAT file system.", __FUNCTION__);
        }
    #endif
        return DISK_READ_ONLY;
    }

    if (BT_Browser_Name_Search(Name, (DWORD *)&filesize, (SEARCH_INFO_FILE | SEARCH_INFO_FOLDER), namesize - 2) == 0)
    {
        MP_ALERT("%s: BT_Browser_Name_Search() failed !\r\n", __FUNCTION__);
        return FAIL;
    }

    pSearchInfo = (ST_SEARCH_INFO *) FileGetCurSearchInfo();
    if (pSearchInfo != NULL)
    {
        SystemSetStatus(SYS_STATUS_DELETE);
        pHandle = FileListOpen(pCurDrive, pSearchInfo);
        if (pHandle)
        {
            if (pSearchInfo->bParameter & SEARCH_INFO_FOLDER) // 06.26.2006 for use bParameter record invalid file
                swRet = DeleteDir(pCurDrive);
            else if (pSearchInfo->bParameter & SEARCH_INFO_CHANGE_PATH) // 06.26.2006 for use bParameter record invalid file
                return swRet;
            else
                swRet = DeleteFile(pHandle);

            FileClose(pHandle);
            BT_Browser_Refresh_File_List();
            if (swRet == END_OF_DIR)
                swRet = FS_SUCCEED;
            if (swRet == FS_SUCCEED)
            {
                SystemClearStatus(SYS_STATUS_DELETE);
            }
        }
        else
            swRet = FILE_NOT_FOUND;
    }
    else
        swRet = FAIL;

    return swRet;
    //SystemSetErrEvent(ERR_FILE_SYSTEM);
}


// bt special
STREAM *BtBrowserFileCreate(WORD * pwSname)
{
    WORD i, offset;
    TotalByte = 0;
    STREAM *pFileHandle;    
    DRIVE *sDrv = FileBrowserGetCurDrive();

    if (CreateFile_UTF16(sDrv, pwSname) != FS_SUCCEED)
    {
        MP_ALERT("%s: CreateFile_UTF16() failed !", __FUNCTION__);
        return 0;
    }
    pFileHandle = (STREAM *) FileOpen(sDrv);

    if (!pFileHandle)
    {
        FileClose(pFileHandle);
        MP_ALERT("%s: FileOpen() failed !", __FUNCTION__);
        return 0;
    }

    TotalByte = 0;
    WriteOffset = 0;
    blFileCreated = 1;
    
    return pFileHandle;
}


// bt special
BOOL BT_Browser_Switch_2_Root(void)
{
    SWORD Status;
    register ST_SYSTEM_CONFIG *psSysConfig;
    psSysConfig = g_psSystemConfig;
    BYTE bDriveId = psSysConfig->sStorage.dwCurStorageId;
    struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
    psSysConfig->dwCurrentOpMode = OP_IMAGE_MODE;
    DRIVE *pCurDrive = FileBrowserGetCurDrive();

    FileBrowserResetFileList();
    if (pCurDrive == NULL)
    {
        MP_ALERT("%s: Null DRIVE pointer !", __FUNCTION__);
        return FAIL;
    }
    if (DirReset(pCurDrive) != FS_SUCCEED)
    {
        MP_ALERT("%s: DirReset() failed !", __FUNCTION__);
        return FAIL;
    }

    if (pCurDrive->DirStackPoint != 0) // not at root dir  abel 2006.12.20
    {
        MP_DEBUG("Not in Root ...");
        UpdateChain(pCurDrive);
    }
    psFileBrowser->dwSearchFileCount = 0;
    Status = DoSearch(pCurDrive, LOCAL_SEARCH_INCLUDE_FOLDER);
    BT_Browser_Reset_Info_Index();
    return (Status == PASS ? TRUE:FALSE);
}


// subfunc of btbrowser
BYTE Check_Dot(WORD * name, WORD len)
{
    BYTE i;
    WORD wlen;
    BYTE dot = 0;

    i = 0;
    wlen = len;
    while (wlen)
    {
        if ((name[i] & 0x00ff) == '.')
        {
            dot++;
        }
        wlen -= 2;
        i++;
    }
    return dot;
}


// bt special
SWORD BT_Browser_Create_New_Folder(WORD * name, WORD len)
{
    TotalByte = 0;
    BYTE bDriveId = g_psSystemConfig->sStorage.dwCurStorageId;
    DRIVE *drv = FileBrowserGetCurDrive();

    /* check Read-Only flag by underlying Mcard layer info => physical lock latch or Mcard layer lock */
    if (SystemGetFlagReadOnly(bDriveId))
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

    //check free space, reserved 4 clusters for build subdir
    if (CheckSpaceIfEnough(drv, 4 * (drv->ClusterSize << drv->bSectorExp)) == DISK_FULL)
    {
        MP_ALERT("%s: -E- Disk full", __FUNCTION__);
        return DISK_FULL;
    }

#if (_BT_USE_ORI_FUNC_FLOW_ == 0)
    if (MakeDir_UTF16(drv, (WORD *) name) != FS_SUCCEED)
    {
        MP_ALERT("%s: MakeDir_UTF16() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (CdSub(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: CdSub() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    FileBrowserResetFileList();
#else
    if (BT_Browser_Make_Dir(drv, (WORD *) name, len) != FS_SUCCEED)
    {
        MP_ALERT("%s: BT_Browser_Make_Dir() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    if (CdSub(drv) != FS_SUCCEED)
    {
        MP_ALERT("%s: CdSub() failed !\r\n", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    FileBrowserResetFileList();

    if (drv->DirStackPoint != 0) // not at root dir  abel 2006.12.20
    {
        MP_DEBUG("Not in Root ...");
        UpdateChain(drv);
    }
#endif //_BT_USE_ORI_FUNC_FLOW_

    if (DoSearch(drv, LOCAL_SEARCH_INCLUDE_FOLDER) != PASS)
    {
        MP_ALERT("%s: DoSearch() failed !\r\n", __FUNCTION__);
        return FAIL;
    }

    BT_Browser_Reset_Info_Index();
    return PASS;
}


// bt sspecial
SWORD BT_Browser_Switch_2New_Folder(BYTE * Sname, BYTE type, WORD namesize, BYTE Flags)
{
    DWORD filesize = 0;
    ST_SEARCH_INFO *psSearchList;
    if (BT_Browser_Name_Search(Sname, (DWORD *)&filesize, type, namesize - 2) == 0)
    {
        MP_ALERT("%s: BT_Browser_Name_Search() failed !", __FUNCTION__);
        return FAIL;
    }
    psSearchList = FileGetCurSearchInfo();
    MP_ASSERT(psSearchList != NULL);
    if ((Flags & 0x02)== 0x02) //do not creat
    {
    	if (FileChangeDirAndResearch(psSearchList) == FAIL)
        {
            MP_ALERT("%s: FileChangeDirAndResearch() failed !", __FUNCTION__);
            return FAIL;
        }
    }
    return PASS;
}


#define FILE_ATTRIBUTE_ARCHIVE      0x00
#define FILE_ATTRIBUTE_NORMAL       0x01  
#define FILE_ATTRIBUTE_DIRECTORY    0x02


// bt special
BYTE BT_Browser_Get_Cur_Type(void)
{
    ST_SEARCH_INFO *psSearchList = FileGetCurSearchInfo();
    BYTE bType;
    bType = psSearchList->bParameter;
    switch(bType)
    {
        case SEARCH_INFO_CHANGE_PATH :
            MP_DEBUG("Change Path Type not supported !\r\n");
            bType = 0xFF;
            break;

        case SEARCH_INFO_FOLDER :
            bType = FILE_ATTRIBUTE_DIRECTORY;
            break;

        case SEARCH_INFO_FILE :
            bType = FILE_ATTRIBUTE_NORMAL;
            break;

        default :
            MP_DEBUG("Unknown Type, not supported !\r\n");
//            __asm("break 100");
            bType = 0xFF;
            break;
    }
    return bType;
}


// bt special
void BT_Browser_Decimal2Char(DWORD size, BYTE * buffer, BYTE arrysize)
{
    DWORD temp;
    BYTE index=0,j;
    memset(buffer, 0, arrysize);
    while (size)
    {
        buffer[0] = ((size % 10) + 0x30);
        size = size / 10;
        if (size == 0)
            return;
        index++;
        for (j = index; j > 0; j--)
        {
            buffer[j] = buffer[j - 1];
        }
    }
    buffer[0] = 0x30;
    return;
}


// bt special
DWORD BT_Browser_Cur_File_Size(void)
{
    ST_SEARCH_INFO *psSearchInfo;
    psSearchInfo = FileGetCurSearchInfo();  
    return psSearchInfo->dwFileSize;
}


// bt special
STREAM *FileOpenBySerachInfo(void)
{
    return FileListOpen(DriveGet(DriveCurIdGet()), FileGetSearchInfo(GetFileIndex()));
}


// bt special
#if (BT_PROFILE_TYPE & BT_FTP_SERVER)
SWORD BtApiFstoreOpen(DWORD handle,BYTE * pbName, DWORD * filesize, BYTE namesize)
{
    BYTE *pbuf;
    WORD *pwbuf;
#if M_PROFILE
    pbuf = GetBufByDlci(GetCurrentDlci());
#else//M_PROFILE
    pbuf = BTA(pWBuf0);
#endif//M_PROFILE

    WORD index;
    pwbuf = (WORD *)pbName;

    if (BT_Browser_Name_Search(pbName, filesize, SEARCH_INFO_FILE, namesize) == 0)
    {
        MP_ALERT("%s: BT_Browser_Name_Search() failed !", __FUNCTION__);
        return FAIL;
    }
    FileHandle = FileListOpen(DriveGet(DriveCurIdGet()), FileGetCurSearchInfo());
    SetFstoreFilehandle(FileHandle);   
    if (FileHandle == NULL)
    {
        MP_ALERT("%s: FileListOpen() failed !", __FUNCTION__);
    }

    TotalByte = BT_Browser_Cur_File_Size();
#if M_WRITE
    ReadOffset = 0;
    ReadRemain = 0;

    if (TotalByte >= MAX_FILE_LENGTH)
        ReadRemain = MAX_FILE_LENGTH;
    else
        ReadRemain = TotalByte;

    if (FileRead(FileHandle, pbuf, ReadRemain) != ReadRemain)
    {
        MP_ALERT("%s: FileRead() failed !", __FUNCTION__);
    }
    TotalByte -= ReadRemain;     
#endif

    return PASS;
}
#endif

// bt special
DWORD BtApiFstoreRead(DWORD handle,BYTE * pbuff, DWORD size)
{
    BYTE *pSBuf;
    DWORD dwTemp;
    DWORD len;
#if M_PROFILE
    pSBuf = GetBufByDlci(GetCurrentDlci());
#else//M_PROFILE
    pSBuf = BTA(pWBuf0);
#endif//M_PROFILE
    if (handle != NULL)
    {
#if M_WRITE
        if (ReadRemain > size)
        {
            ReadRemain -= size;
            memcpy(pbuff, pSBuf + ReadOffset, size);
            ReadOffset += size;
            return size;
        }
        else
        {
            dwTemp = size;
            memcpy(pbuff, pSBuf + ReadOffset, ReadRemain);
            pbuff += ReadRemain;
            dwTemp -= ReadRemain;
            if (TotalByte >= MAX_FILE_LENGTH)
                ReadRemain = MAX_FILE_LENGTH;
            else
                ReadRemain = TotalByte;
            if (FileRead(FileHandle, pSBuf, ReadRemain) != ReadRemain)
            {
                MP_ALERT("%s: FileRead() failed !", __FUNCTION__);
                return 0;
            }
            ReadOffset = 0; 
            TotalByte -= ReadRemain;
            if (dwTemp)
            {
                memcpy(pbuff, pSBuf + ReadOffset, dwTemp);
                ReadRemain -= dwTemp;
                ReadOffset += dwTemp;
            }
            return size;
        }
#else//M_WRITE
        len = FileRead(handle, pbuff, size); 
        if ( len != size)
        {
            mpDebugPrint("BtApiFstoreRead fail");
            mpDebugPrint("len %d",len);
            return len;
    }
    else
            return size;
#endif//M_WRITE
    }
        MP_ALERT("%s: no file handle !", __FUNCTION__);
    return 0;
}







#if 0 // Not used functions, can mark now ------------- [begin]

// can mark now
DWORD BT_Browser_Get_Info_Index(void)
{
    return FileBrowserGetCurIndex();
}


// can mark now
BOOL BT_Browser_Is_In_Root(void)
{
    DRIVE *pCurDrive = FileBrowserGetCurDrive();
    if (pCurDrive->DirStackPoint == 0)
        return TRUE;
    else
        return FALSE;
}


// can mark now
BOOL BT_Browser_Is_Sinfo_Init(void)
{
    register ST_SYSTEM_CONFIG *psSysConfig;
    psSysConfig = g_psSystemConfig;
    register struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;

    if (psFileBrowser->dwValid == 1)
    {
        MP_DEBUG("file browser init ok\r\n");
        return TRUE;
    }
    else
    {
        MP_DEBUG("file browser not init\r\n");
        return FALSE;
    }
}


// can mark now
void HLSwitch(WORD * buf, WORD size)
{
    WORD x, y;
    for (x=0; x < size; x++)
    {
        y = ((buf[x] << 8) | (buf[x] >> 8));
        buf[x] = y;
    }
}


// can mark now
void Change_Name_Byte_2_Word(BYTE * pbName, WORD namesize, WORD * pwbuf, WORD * word_name_size)
{
    *word_name_size = 0;
    WORD i=0;
    while (namesize--)
    {
        pwbuf[i] = pbName[i];
        (*word_name_size) += 2;
        i++;
    }
    pwbuf[i] = 0x0000;
    *word_name_size += 2;
}


// can mark now
BYTE *Change_Name_Word_2_Byte(WORD * pwbuf, WORD * namesize)
{
    *namesize = 0;
    BYTE *pbbuf = (BYTE *) pwbuf;
    while (pwbuf[(*namesize)] != 0)
    {
        pbbuf[(*namesize)] = (BYTE) pwbuf[(*namesize)];
        (*namesize)++;
    }
    memset(pbbuf + (*namesize), 0, *namesize);
    return (BYTE *) pbbuf;
}

#endif // Not used functions, can mark now ------------- [end]


