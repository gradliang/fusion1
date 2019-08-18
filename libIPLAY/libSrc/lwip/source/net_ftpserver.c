#define LOCAL_DEBUG_ENABLE 0

//#define WIFI_TESTPLAN   1

#include <string.h>
#include <sys/time.h>
#include "linux/types.h"
#include "typedef.h"
#include "net_ftp.h"
#include "ftp_auth.h"
#include "ftp_strings.h"
#include "taskid.h"
#include "global612.h"


ST_FTP_STATE g_stFTPState2[MAX_FTP_SESSIONS+1];
static BYTE g_mbSendBuf[FTP_STR_BUF_SIZE];
static BYTE *g_pbCmdBuf, *g_pbFileNameBuf, *g_pbFileExtBuf, *g_pbFileBuf;
static DWORD g_dwFileBufSize[MAX_FTP_SESSIONS+1], g_dwFileBufUsedSize[MAX_FTP_SESSIONS+1], g_dwHandlePtr[MAX_FTP_SESSIONS+1], g_dwFileTotalSize[MAX_FTP_SESSIONS+1];

void ftpserverSendFinished(void *ctx);
void ftpserver_SetReply(int msgId, void *ctx);
void ftp_GetLocalIpAddress(U32 *ipaddr, void *ctx);
void get_Dummy_File(char *buf);

int ftpserver_SendMsg(BYTE *pbMsg, WORD wCnt, void *ctx)
{
    int ret;
    wCnt = (wCnt < FTP_STR_BUF_SIZE - 2) ? wCnt : (FTP_STR_BUF_SIZE - 2);
    pbMsg[wCnt ++] = ISO_CR;
    pbMsg[wCnt ++] = ISO_LF;
    
    ret = ftp_send(pbMsg, wCnt, TRUE, ctx);

    pbMsg[wCnt - 1] = 0;
    MP_DEBUG1("  -REPLY- %s", pbMsg);
    return ret;
}

int ftpserver_SendMsg2(BYTE *pbMsg, WORD wCnt, int sid)
{
    int ret;
    wCnt = (wCnt < FTP_STR_BUF_SIZE - 2) ? wCnt : (FTP_STR_BUF_SIZE - 2);
    pbMsg[wCnt ++] = ISO_CR;
    pbMsg[wCnt ++] = ISO_LF;
    
    ret = ftp_send2(pbMsg, wCnt, sid);

    pbMsg[wCnt - 1] = 0;
    MP_DEBUG1("  -REPLY- %s", pbMsg);
    return ret;
}
SWORD ftpserver_SendReplyMsg(WORD wMsgNum, void *ctx)
{    
    WORD i, wCnt = 0;

    for (i = 0; i < FTP_REPLY_MSG_NUM; i ++)
    {
        if (m_wFTP_Reply_Code_Mapping[i] == wMsgNum)
            break;
    }
    if (FTP_REPLY_MSG_NUM <= i)
    { 
        MP_DEBUG1("-E- Reply message %d can't be found!", wMsgNum);
        return FAIL;
    }
    
    wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "%d ", wMsgNum, wMsgNum);
    wCnt += mp_sprintf(&g_mbSendBuf[wCnt], m_pbFTP_Reply_Strings[i]);

    ftpserver_SendMsg(g_mbSendBuf, wCnt, ctx);
    return PASS;
}

SWORD ftpserver_SendReplyMsg2(WORD wMsgNum, int sid)
{    
    WORD i, wCnt = 0;

    for (i = 0; i < FTP_REPLY_MSG_NUM; i ++)
    {
        if (m_wFTP_Reply_Code_Mapping[i] == wMsgNum)
            break;
    }
    if (FTP_REPLY_MSG_NUM <= i)
    { 
        MP_DEBUG1("-E- Reply message %d can't be found!", wMsgNum);
        return FAIL;
    }
    
    wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "%d ", wMsgNum, wMsgNum);
    wCnt += mp_sprintf(&g_mbSendBuf[wCnt], m_pbFTP_Reply_Strings[i]);

    ftpserver_SendMsg2(g_mbSendBuf, wCnt, sid);
    return PASS;
}
void ftpserver_SendCurPathMsg(void *ctx,BYTE idx)
{
    WORD wCnt = 0;

    wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "257 Current path is \"/");
    switch (FTP_ST_CUR_PATH(g_stFTPState2[idx]))
    {
        case FTP_PATH_PHOTO:
            wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "Photo");
            break;
        case FTP_PATH_MUSIC:
            wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "Music");
            break;
        case FTP_PATH_VIDEO:
            wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "Video");
            break;
        default:
            break;
    }
    g_mbSendBuf[wCnt ++] = '"';
    ftpserver_SendMsg(g_mbSendBuf, wCnt, ctx);
}

void ftpserver_ScanFileList(BYTE bPath,BYTE idx)
{
    MP_DEBUG1("-I- Scan file list - %d", bPath);
    if (FTP_PATH_ROOT == FTP_ST_CUR_PATH(g_stFTPState2[idx]))
        FTP_ST_SCAN_MEDIA(g_stFTPState2[idx]) = TRUE;

    if (FTP_ST_SCAN_MEDIA(g_stFTPState2[idx]))
        return;
    
	DRIVE *pCurDrive = DriveChange(FTP_CONN_DRIVE);
	if (DirReset(pCurDrive) != FS_SUCCEED)
    {   
        //DriveChange(SDIO);
		DriveChange(USB_WIFI_DEVICE);
		return;
    }
    if (pCurDrive->DirStackPoint != 0) // not at root dir  abel 2006.12.20
        UpdateChain(pCurDrive);
    if (FTP_PATH_PHOTO == bPath)
        g_psSystemConfig->dwCurrentOpMode = OP_IMAGE_MODE;
    else if (FTP_PATH_MUSIC == bPath)
        g_psSystemConfig->dwCurrentOpMode = OP_AUDIO_MODE;
    if (FTP_PATH_VIDEO == bPath)
        g_psSystemConfig->dwCurrentOpMode = OP_MOVIE_MODE;
 	DoSearch(pCurDrive, SEARCH_TYPE);
    
    //DriveChange(SDIO);
    DriveChange(USB_WIFI_DEVICE);

    g_psSystemConfig->dwCurrentOpMode = OP_NETWARE_MODE;
    FTP_ST_SCAN_MEDIA(g_stFTPState2[idx]) = TRUE;
}
void ftpserver_ChangeDirectory(BYTE bCmdIndex, BYTE *pbData, void *ctx,BYTE idx)
{
    BYTE bPrevPath = FTP_ST_CUR_PATH(g_stFTPState2[idx]);
    while ('/' == *pbData)
        pbData ++;
    
    if (FTP_PATH_ROOT == FTP_ST_CUR_PATH(g_stFTPState2[idx]))
    {
        if (0 == strncmp(pbData, "Photo", 5))
            FTP_ST_CUR_PATH(g_stFTPState2[idx]) = FTP_PATH_PHOTO;
        else if (0 == strncmp(pbData, "Music", 5))
            FTP_ST_CUR_PATH(g_stFTPState2[idx]) = FTP_PATH_MUSIC;
        else if (0 == strncmp(pbData, "Video", 5))
            FTP_ST_CUR_PATH(g_stFTPState2[idx]) = FTP_PATH_VIDEO;
    }
    else if ((0 == strncmp(pbData, "..", 2)) || (FTP_CMD_CDUP == bCmdIndex))
            FTP_ST_CUR_PATH(g_stFTPState2[idx]) = FTP_PATH_ROOT;

    if (bPrevPath != FTP_ST_CUR_PATH(g_stFTPState2[idx]))
    {
        EventSet(FTP_SERVER_EVENT, FTP_EVENT_SCAN_LIST);
        FTP_ST_SCAN_MEDIA(g_stFTPState2[idx]) = FALSE;
    }
    
    if (FTP_CMD_CDUP == bCmdIndex)
        ftpserver_SetReply(200, ctx);
    else
        ftpserver_SetReply(250, ctx);
}

void ftpserver_EnterPassiveMode(void *ctx)
{
    WORD wCnt = 0;
    WORD myipaddr[2];
    U16 lport;

    ftp_GetLocalIpAddress((U32 *)myipaddr, ctx);
    
    lport = mpx_NewLocalPort();

    wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "227 Entering Passive Mode ");
    wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "(%d,%d,%d,%d,%d,%d)", 
        ((myipaddr[0] & 0xff00) >> 8), (myipaddr[0] & 0xff), 
        ((myipaddr[1] & 0xff00) >> 8), (myipaddr[1] & 0xff),
        ((lport & 0xff00) >> 8), (lport & 0xff));
    ftpserver_SendMsg(g_mbSendBuf, wCnt, ctx);

    ftpServerEnterPASVMode(ctx);
}

void ftpserver_DP_SendListFinish(void *ctx,BYTE idx)
{
    MP_DEBUG("-I- Send list finish. %d",idx);
    ftpserverSendFinished(ctx);

    if (g_pbFileBuf)
    {
        ftp_mfree(g_pbFileBuf);
        g_pbFileBuf = NULL;
    }
    g_dwFileBufSize[idx] = g_dwFileTotalSize[idx] = 0;
}
int ftpserver_DP_WriteFileList(void *ctx, BYTE bRestart,BYTE idx)
{
    BYTE m_bName[14];
    static WORD wLen = 0, wTotalFile = 0, wFileIndex = 0;
    static ST_SEARCH_INFO *pstSearchInfo;
    ST_SYSTEM_CONFIG *pstSysConfig = g_psSystemConfig;
    int len;

    if (bRestart)
    {
        MP_DEBUG("-I- Write file list. %d",idx);
        g_dwFileBufUsedSize[idx] = wLen = wTotalFile = wFileIndex = 0;
        switch (FTP_ST_CUR_PATH(g_stFTPState2[idx]))
        {
            case FTP_PATH_PHOTO:
                wTotalFile = pstSysConfig->sFileBrowser.dwImgAndMovTotalFile;
                pstSearchInfo = pstSysConfig->sFileBrowser.sImgAndMovFileList;
                break;
            case FTP_PATH_MUSIC:
                wTotalFile = pstSysConfig->sFileBrowser.dwAudioTotalFile;
                pstSearchInfo = pstSysConfig->sFileBrowser.sAudioFileList;
                break;
            case FTP_PATH_VIDEO:
                wTotalFile = pstSysConfig->sFileBrowser.dwImgAndMovTotalFile;
                pstSearchInfo = pstSysConfig->sFileBrowser.sImgAndMovFileList;
                break;
            default: 
                return 0;            
        }
        g_dwFileBufSize[idx] = FTP_DEFAULT_MSS;
        if ((g_dwFileBufSize[idx] > FTP_STR_BUF_SIZE) && (g_dwFileBufSize < mem_get_free_space()))
        {
            g_pbFileBuf = ftp_malloc(g_dwFileBufSize[idx]);
        }
        else
        {
            g_pbFileBuf = NULL;
            g_dwFileBufSize[idx] = 0;
        }
        MP_DEBUG2("-I- File buf:0x%x Size:%d bytes.", g_pbFileBuf, g_dwFileBufSize[idx]);
        MP_DEBUG2("Mode = %d, Total = %d", pstSysConfig->dwCurrentOpMode, wTotalFile);   
    }

    MP_DEBUG("wFileIndex = %d",wFileIndex);
    while(wFileIndex < wTotalFile)
    {
    MP_DEBUG("wFileIndex = %d",wFileIndex);
        wLen = 0;
        //directory?//privilege//num??//owner//group//file size
        wLen += mp_sprintf(&g_mbSendBuf[wLen], "-rwxr--r--   0 ftp     ftp %d", pstSearchInfo->dwFileSize);

        //month
        switch(pstSearchInfo->DateTime.month)
        {
            case 1:  wLen += mp_sprintf(&g_mbSendBuf[wLen], " Jan"); break;
            case 2:  wLen += mp_sprintf(&g_mbSendBuf[wLen], " Feb"); break;
            case 3:  wLen += mp_sprintf(&g_mbSendBuf[wLen], " Mar"); break;
            case 4:  wLen += mp_sprintf(&g_mbSendBuf[wLen], " Apr"); break;
            case 5:  wLen += mp_sprintf(&g_mbSendBuf[wLen], " May"); break;
            case 6:  wLen += mp_sprintf(&g_mbSendBuf[wLen], " Jun"); break;
            case 7:  wLen += mp_sprintf(&g_mbSendBuf[wLen], " Jul"); break;
            case 8:  wLen += mp_sprintf(&g_mbSendBuf[wLen], " Aug"); break;
            case 9:  wLen += mp_sprintf(&g_mbSendBuf[wLen], " Sep"); break;
            case 10: wLen += mp_sprintf(&g_mbSendBuf[wLen], " Oct"); break;
            case 11: wLen += mp_sprintf(&g_mbSendBuf[wLen], " Nov"); break;
            case 12: wLen += mp_sprintf(&g_mbSendBuf[wLen], " Dec"); break;
            default: wLen += mp_sprintf(&g_mbSendBuf[wLen], " Jan"); break;
        }
        
        //date//time//file name
        memcpy(&m_bName[0], &pstSearchInfo->bName, 8);
        memcpy(&m_bName[9], &pstSearchInfo->bExt, 4);
        m_bName[8] = '.';
        m_bName[13] = 0;
        wLen += mp_sprintf(&g_mbSendBuf[wLen], " %2d %2d:%2d %s\r\n", pstSearchInfo->DateTime.day, 12, 00, &m_bName);
        
        MP_DEBUG(&g_mbSendBuf);
        wFileIndex ++;
        pstSearchInfo ++;
        
        if (g_dwFileBufSize[idx])
        {
            if((g_dwFileBufUsedSize[idx] + wLen) > g_dwFileBufSize[idx])
            {
                len = ftp_send(g_pbFileBuf, g_dwFileBufUsedSize[idx], FALSE, ctx);

                memcpy(g_pbFileBuf, &g_mbSendBuf, wLen);
                g_dwFileBufUsedSize[idx] = wLen;
                return len;            
            }
            memcpy(g_pbFileBuf + g_dwFileBufUsedSize[idx], &g_mbSendBuf, wLen);
            g_dwFileBufUsedSize[idx] += wLen;
        }
        else
        {
            len = ftp_send(&g_mbSendBuf, wLen, FALSE, ctx);

            return len;            
        }
    }
    
    if(g_dwFileBufUsedSize[idx])
    {
        len = ftp_send(g_pbFileBuf, g_dwFileBufUsedSize[idx], FALSE, ctx);

        g_dwFileBufUsedSize[idx] = 0;

        return len;            
    }
    
    ftpserver_SetReply(226, ctx);
    ftpserver_DP_SendListFinish(ctx,idx);
    return 0;
}

int ftpserver_DP_SendList(int outLen, void *ctx,BYTE idx)
{   
    WORD wCnt = 0;
	DRIVE *sDrv;
	STREAM *w_shandle;
	int find_file=0;

    MP_DEBUG("-I- Send list. %d",idx);
//__asm("break 100");
    switch (FTP_ST_CUR_PATH(g_stFTPState2[idx]))
    {
        case FTP_PATH_ROOT:
            if (outLen == 0)
            {
#ifndef WIFI_TESTPLAN
#if 0
                wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "drwxr--r--   0 ftp     ftp 0 Jan 01 12:00 Photo\r\n");
                wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "drwxr--r--   0 ftp     ftp 0 Jan 01 12:00 Music\r\n");
                wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "drwxr--r--   0 ftp     ftp 0 Jan 01 12:00 Video\r\n");
#else
				sDrv = DriveGet(SD_MMC);//SD_MMC//NAND
				if (FS_SUCCEED == FileSearch(sDrv, "test", "mp3", E_FILE_TYPE))
				{
					mpDebugPrint("Find test.mp3");
					
					w_shandle = FileOpen(sDrv);
					wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "-rwxr--r--	 1 ftp	   ftp %d Jan 01 12:00 test.mp3\r\n",FileSizeGet(w_shandle));
					FileClose(w_shandle);
					find_file++;
				}
				
				if (FS_SUCCEED == FileSearch(sDrv, "test", "tmp", E_FILE_TYPE))
				{
					mpDebugPrint("Find test.tmp");
					
					w_shandle = FileOpen(sDrv);
					wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "-rwxr--r--	 1 ftp	   ftp %d Jan 01 12:00 test.tmp\r\n",FileSizeGet(w_shandle));
					FileClose(w_shandle);
					
					find_file++;
				}
				DriveChange(USB_WIFI_DEVICE);
				
				if(!find_file)
				 {
					 wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "drwxr--r--   0 ftp 	ftp 0 Jan 01 12:00 Photo\r\n");
					 wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "drwxr--r--   0 ftp 	ftp 0 Jan 01 12:00 Music\r\n");
					 wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "drwxr--r--   0 ftp 	ftp 0 Jan 01 12:00 Video\r\n");
				 }
#endif				
#else
                wCnt += mp_sprintf(&g_mbSendBuf[wCnt], "-rwxr--r--   1 ftp     ftp 8388608 Jan 01 12:00 file_8mb.txt\r\n");
#endif
                wCnt = ftp_send(&g_mbSendBuf, wCnt, FALSE, ctx);
            }
            else
            {
                ftpserver_SetReply(226, ctx);
                ftpserver_DP_SendListFinish(ctx,idx);
            }
            break;
        default:
            if (outLen == 0)
                wCnt = ftpserver_DP_WriteFileList(ctx, 1,idx);
            else
                wCnt = ftpserver_DP_WriteFileList(ctx, 0,idx);
            break;
    }

    return wCnt;
}
void ftpserver_ParseFileName(BYTE *pbCmgBuf)
{
    DWORD i;
	BYTE *tmpfile,*tmpextfile;
	
	g_pbFileNameBuf = ftp_malloc(8);
	g_pbFileExtBuf = ftp_malloc(4);
	memset(g_pbFileNameBuf,0x00,8);
    memset(g_pbFileExtBuf,0x00,4);
	
    while ('/' == *pbCmgBuf)
        pbCmgBuf ++;
	
    tmpfile = pbCmgBuf;
    for (i = 0; (i < FTP_STR_BUF_SIZE) && (0 != *(pbCmgBuf + i)); i ++);
        *(pbCmgBuf + i) = 0;
    tmpextfile = pbCmgBuf + i;
    for (; i > 0; i --)
    {
        if ('.' == *(pbCmgBuf + i))
        {
            *(pbCmgBuf + i) = 0;
            tmpextfile = pbCmgBuf + i + 1;
            break;
        } 
    }
	strcpy(g_pbFileNameBuf,tmpfile);
	strcpy(g_pbFileExtBuf,tmpextfile);
}
SWORD ftpserver_CreateFile(BYTE *pbName, BYTE *pbExt)
{
    DRIVE *sTrgDrive = DriveChange(FTP_CONN_DRIVE);
    int iRet = CreateFile(sTrgDrive, pbName, pbExt);
    if (FS_SUCCEED == iRet)
    {
        //strncpy(g_pbFileNameBuf, &sTrgDrive->Node->Name[0], 8);
        //strncpy(g_pbFileExtBuf, &sTrgDrive->Node->Extension[0], 3);
        g_dwHandlePtr[0] = 0;
        MP_DEBUG("-I- FDB built.");
    }
    //DriveChange(SDIO);
	DriveChange(USB_WIFI_DEVICE);
    return iRet;
}
void ftpserver_DP_SendFileFinish(void *ctx,BYTE idx)
{
    MP_DEBUG("-I- Send file finish.");
    ftpserverSendFinished(ctx);

    if (g_pbFileBuf)
    {
        ftp_mfree(g_pbFileBuf);
        g_pbFileBuf = NULL;
    }
	if(g_pbFileNameBuf)
	{
		ftp_mfree(g_pbFileNameBuf);
		g_pbFileNameBuf = NULL;
	}
	
	if(g_pbFileExtBuf)
	{
		ftp_mfree(g_pbFileExtBuf);
		g_pbFileExtBuf = NULL;
	}

    g_dwFileBufSize[idx] = g_dwFileTotalSize[idx] = 0;
#ifndef WIFI_TESTPLAN
    //DriveChange(SDIO);
    DriveChange(USB_WIFI_DEVICE);
#endif
}

STREAM *ftp_sHandle = NULL;
DRIVE *ftp_sDrv;

int ftpserver_DP_SendFileChain(void *ctx,BYTE idx)
{
    DWORD dwSendSize = 0;
    int len;
	DRIVE *sDrv;
    //mpDebugPrint("g_dwFileBufUsedSize = %d, g_dwFileBufSize = %d, g_dwFileTotalSize = %d",g_dwFileBufUsedSize,g_dwFileBufSize,g_dwFileTotalSize);
    //mpDebugPrint("ftpserver_DP_SendFileChain %d",idx);
    do {
        if (g_dwFileBufUsedSize[idx] < g_dwFileBufSize[idx])
        {
            dwSendSize = FTP_DEFAULT_MSS;
            if ((g_dwFileBufSize[idx] - g_dwFileBufUsedSize[idx]) < dwSendSize)
                dwSendSize = g_dwFileBufSize[idx] - g_dwFileBufUsedSize[idx];

            len = ftp_send(g_pbFileBuf + g_dwFileBufUsedSize[idx], dwSendSize, FALSE, ctx);

            MP_ASSERT(len != 0);
            if (len > 0)
            {
                g_dwFileBufUsedSize[idx] += len;
            }
            //UartOutText(".");
            break;
        }
        else
        {
            if (g_dwFileTotalSize[idx] == 0) 
            {
                ftpserver_SetReply(226,ctx);
                ftpserver_DP_SendFileFinish(ctx,idx);
                return 0;
            }      
            if (g_dwFileBufSize[idx] > g_dwFileTotalSize[idx])
                g_dwFileBufSize[idx] = g_dwFileTotalSize[idx];
            g_dwFileBufUsedSize[idx] = 0;
            g_dwFileTotalSize[idx]-= g_dwFileBufSize[idx];
            MP_DEBUG1("-I- Read %d bytes data to buffer for sending.", g_dwFileBufSize[idx]);

#ifndef WIFI_TESTPLAN
			ftp_sDrv = DriveGet(SD_MMC); //SD_MMC //NAND
			if (FS_SUCCEED == FileSearch(ftp_sDrv, g_pbFileNameBuf, g_pbFileExtBuf, E_FILE_TYPE))
			{
	            ftp_sHandle = FileOpen(ftp_sDrv);
				
				if (NULL == ftp_sHandle)
				{
				    mpDebugPrint("open %s FAIL",g_pbFileNameBuf);
				    goto SEND_FILE_CHAIN_FAIL;
				}
				//mpDebugPrint("g_dwHandlePtr[%d] %x",idx,g_dwHandlePtr[idx]);
	            if (FS_SUCCEED != Seek(ftp_sHandle, g_dwHandlePtr[idx]))
	            {
	            
				   mpDebugPrint("Seek %x  %d FAIL",g_dwHandlePtr[idx],idx);
                goto SEND_FILE_CHAIN_FAIL;
	            }
	            if (g_dwFileBufSize[idx] != FileRead(ftp_sHandle, g_pbFileBuf, g_dwFileBufSize[idx]))
	            {
	                mpDebugPrint("g_dwFileBufSize[idx] != FileRead");
                goto SEND_FILE_CHAIN_FAIL;
	            }
	            g_dwHandlePtr[idx] = FilePosGet(ftp_sHandle);
	            FileClose(ftp_sHandle);
			}
            //DriveChange(SDIO);
			DriveChange(USB_WIFI_DEVICE);
#else   //This part for test only.
//            memset(g_pbFileBuf, 'x', g_dwFileBufSize);
#endif
        }
    } while (1);
    return len;
SEND_FILE_CHAIN_FAIL:
#ifndef WIFI_TESTPLAN
    if (ftp_sHandle)
	   FileClose(ftp_sHandle);
    //DriveChange(SDIO);
    DriveChange(USB_WIFI_DEVICE);
#endif
    ftpserver_SetReply(450,ctx);
    ftpserver_DP_SendFileFinish(ctx,idx);
    return -1;
}
void ftpserver_DP_SendFile(void *ctx,BYTE idx)
{    
    MP_DEBUG("-I- Send File.");
    
    mpDebugPrint("ftpserver_DP_SendFile %d",idx);
    g_dwFileBufUsedSize[idx] = g_dwFileBufSize[idx] = FTP_DEFAULT_MSS;
    if (g_dwFileBufSize[idx])
    {
        g_pbFileBuf = ftp_malloc(g_dwFileBufSize[idx]);
        if (g_pbFileBuf == NULL)
        {
            MP_ALERT("[FTP] -E- Out of memory");
            goto SEND_FILE_FAIL;
        }
        get_Dummy_File(g_pbFileBuf);
    }
    else
		goto SEND_FILE_FAIL;
    g_dwHandlePtr[idx] = 0;
    MP_DEBUG2("-I- File buf:0x%x Size:%d bytes.", g_pbFileBuf, g_dwFileBufSize[idx]);

    return;
SEND_FILE_FAIL:
    ftpserver_SetReply(450,ctx);
    ftpserver_DP_SendFileFinish(ctx,idx);
}

void ftpserver_DP_GetFileFinish(BYTE idx)
{
#ifndef WIFI_TESTPLAN
    //mpDebugPrint("ftpserver_DP_GetFileFinish %d",idx);
    if (g_dwFileBufUsedSize[idx])
    {
        ftp_sDrv = DriveGet(SD_MMC); //SD_MMC //NAND
        if (FS_SUCCEED == FileSearch(ftp_sDrv, g_pbFileNameBuf, g_pbFileExtBuf, E_FILE_TYPE))
        {
	        ftp_sHandle = FileOpen(ftp_sDrv);
	        if (ftp_sHandle)
	        {
	            Seek(ftp_sHandle, g_dwHandlePtr[idx]);
	            FileWrite(ftp_sHandle, g_pbFileBuf, g_dwFileBufUsedSize[idx]);
	            FileClose(ftp_sHandle);
	        }
        }
    }
    FTP_ST_SCAN_MEDIA(g_stFTPState2[idx]) = FALSE;
    FileBrowserResetFileList();
    ftpserver_ScanFileList(FTP_ST_CUR_PATH(g_stFTPState2[idx]),idx);
    //DriveChange(SDIO);
    DriveChange(USB_WIFI_DEVICE);
#endif
    if (g_pbFileBuf)
    {
        ftp_mfree(g_pbFileBuf);
        g_pbFileBuf = NULL;
    }
	if(g_pbFileNameBuf)
	{
		ftp_mfree(g_pbFileNameBuf);
		g_pbFileNameBuf = NULL;
	}
	
	if(g_pbFileExtBuf)
	{
		ftp_mfree(g_pbFileExtBuf);
		g_pbFileExtBuf = NULL;
	}
    g_dwFileBufSize[idx] = g_dwFileBufUsedSize[idx] = 0;
}

void ftpserver_DP_GetFileChain(BYTE *pbBuf, DWORD dwSize, void *ctx,BYTE idx)
{
#ifdef WIFI_TESTPLAN
    /* we don't store the uploaded file */
	return;
#else
    //mpDebugPrint("ftpserver_DP_GetFileChain dwSize %d %d",dwSize,idx);
    DWORD len;
    while (dwSize > 0)
    {
        if (dwSize <= (g_dwFileBufSize[idx] - g_dwFileBufUsedSize[idx]))
        {
            memcpy(g_pbFileBuf + g_dwFileBufUsedSize[idx], pbBuf, dwSize);
            g_dwFileBufUsedSize[idx] += dwSize;
            dwSize = 0;
        }
        else
        {
        
	    	ftp_sDrv=DriveGet(SD_MMC);//SD_MMC//NAND
	    	
			//mpDebugPrint("ftpserver_DP_GetFileChain g_pbFileNameBuf %s %s",g_pbFileNameBuf,g_pbFileExtBuf);
			if (FS_SUCCEED == FileSearch(ftp_sDrv,g_pbFileNameBuf,g_pbFileExtBuf, E_FILE_TYPE))
			{
	            ftp_sHandle = FileOpen(ftp_sDrv);
	            if (NULL == ftp_sHandle)   
                goto GET_FILE_CHAIN_FAIL;
				
				//mpDebugPrint("g_dwHandlePtr[idx] %x",g_dwHandlePtr[idx]);
	            if (FS_SUCCEED != Seek(ftp_sHandle, g_dwHandlePtr[idx]))    
                goto GET_FILE_CHAIN_FAIL;
				//mpDebugPrint("FileWrite g_pbFileBuf %x g_dwFileBufUsedSize[idx] %d",g_pbFileBuf,g_dwFileBufUsedSize[idx]);
	            if (g_dwFileBufUsedSize[idx] != FileWrite(ftp_sHandle, g_pbFileBuf, g_dwFileBufUsedSize[idx])) 
                goto GET_FILE_CHAIN_FAIL;
	            g_dwHandlePtr[idx] = FilePosGet(ftp_sHandle);
	            FileClose(ftp_sHandle);
        	}
            //DriveChange(SDIO);
			DriveChange(USB_WIFI_DEVICE);
            len = (dwSize < g_dwFileBufSize[idx]) ? dwSize : g_dwFileBufSize[idx];
            memcpy(g_pbFileBuf, pbBuf, len);
            dwSize -= len;
            g_dwFileBufUsedSize[idx] = len;
        }
        //UartOutText("*");
        //mpDebugPrint("g_dwFileBufUsedSize[idx] %d",g_dwFileBufUsedSize[idx]);
	//	TaskYield();
    }
	return;
GET_FILE_CHAIN_FAIL:
    if (ftp_sHandle)
    FileClose(ftp_sHandle);
    //DriveChange(SDIO);
	DriveChange(USB_WIFI_DEVICE);
    ftpserver_DP_GetFileFinish(idx);
    ftpserver_SetReply(550,ctx);
#endif
}

int ftpserver_DP_GetFile(void *ctx,BYTE idx)
{   
    //mpDebugPrint("ftpserver_DP_GetFile %d",idx);
    if (strlen(g_pbFileExtBuf) > 0)
    {
        MP_DEBUG2("-I- User upload new file : \"%s.%s\".", g_pbCmdBuf, g_pbFileExtBuf);
    }
    else
    MP_DEBUG1("-I- User upload new file : \"%s\".", g_pbCmdBuf);

    g_dwFileBufUsedSize[idx] = 0;
    g_dwFileBufSize[idx] = 4096;//FTP_DEFAULT_MSS;
    if (g_dwFileBufSize[idx])
    {
        g_pbFileBuf = ftp_malloc(g_dwFileBufSize[idx]);
        if (!g_pbFileBuf)
        {
            MP_ALERT("[FTP] -E- Out of memory");
            goto GET_FILE_FAIL;
        }
    }
    else
		goto GET_FILE_FAIL;
    MP_DEBUG2("-I- File buf:0x%x Size:%d bytes.", g_pbFileBuf, g_dwFileBufSize[idx]);
    return FTP_OK;
GET_FILE_FAIL:
    ftpserver_DP_GetFileFinish(idx);
    ftpserver_SetReply(550,ctx);
    return FTP_ERROR;
}


SWORD ftpserver_CheckLogon(BYTE bCmdIndex, BYTE *pbData, void *ctx,BYTE idx)
{
    ST_FTP_AUTH *pstAuthArry;
     
    if (FTP_CMD_USER == bCmdIndex)
    {
        MP_DEBUG2("-I- User %s is trying to logon. %d", pbData,idx);
        pstAuthArry = &m_stAuthArry[0];
        while (pstAuthArry->m_bUsername[0])
        {
            if (0 == strcmp(pbData, &pstAuthArry->m_bUsername))
            {
                FTP_ST_USER_OK(g_stFTPState2[idx]) = TRUE;
                FTP_ST_PASS_OK(g_stFTPState2[idx]) = FALSE;
                g_stFTPState2[idx].pstCurUser = pstAuthArry;
                ftpserver_SetReply(331,ctx);
                MP_DEBUG("-I- User name has found, need password!");
                return PASS;
            }
            pstAuthArry ++;
        }
    }
    
    if (!FTP_ST_USER_OK(g_stFTPState2[idx]))
        return FAIL;
    
    if (FTP_CMD_PASS == bCmdIndex)
    {
        if (0 == strcmp(pbData, &g_stFTPState2[idx].pstCurUser->m_bPassword) ||
            0 == strcmp("anonymous", &g_stFTPState2[idx].pstCurUser->m_bUsername))
        {
            FTP_ST_PASS_OK(g_stFTPState2[idx]) = TRUE;
            ftpserver_SetReply(230,ctx);
            MP_DEBUG("-I- Password correct, user logon!");
            return PASS;
        }
    }

    if (!FTP_ST_PASS_OK(g_stFTPState2[idx]))
        return FAIL;

    return PASS;
}
void ftpserver_TransToCapital(BYTE *pbData, WORD wLen)
{
    WORD i;
    for (i = 0; i < wLen; i ++)
    {
        if ((*(pbData + i) >= 'a') && (*(pbData + i) <= 'z'))
            *(pbData + i) -= ('a' - 'A'); 
    }
}
SWORD ftpserver_ParseCommand(BYTE *pbData, void *ctx,BYTE idx)
{
    BYTE m_bCmd[5], bCmdLen = 0, bCmdIndex;
    WORD wOffset = 0;
    mpDebugPrint("ftpserver_ParseCommand %d",idx);
    while (*pbData && (*pbData != ISO_SP) && (*pbData != ISO_CR))
        m_bCmd[bCmdLen  ++] = *(pbData ++);
     m_bCmd[bCmdLen] = 0;
     pbData ++;

    if (bCmdLen > 4)
        goto CMD_NOT_EXIST;

    ftpserver_TransToCapital(&m_bCmd, bCmdLen);    
    for (bCmdIndex = 0; bCmdIndex < FTP_CMD_TOTAL_NUM; bCmdIndex ++)
    {
        if (0 == strcmp(&m_bCmd, m_pbFTP_Commands[bCmdIndex]))
            break;
    }

    if (FTP_CMD_TOTAL_NUM <= bCmdIndex)
        goto CMD_NOT_EXIST;
    
    while ((wOffset < FTP_STR_BUF_SIZE) && 
        ((ISO_CR != *(pbData + wOffset)) || (ISO_LF != *(pbData + wOffset + 1))))
        wOffset ++;
    *(pbData + wOffset) = 0;
    MP_DEBUG2("-I- Received command - %s %s", &m_bCmd, pbData);

    FTP_ML_PREV_CMD(g_stFTPState2[idx]) = bCmdIndex;
    g_pbCmdBuf = pbData;
    
    if (!(FTP_ST_USER_OK(g_stFTPState2[idx]) && FTP_ST_PASS_OK(g_stFTPState2[idx])))
    {
        if (FAIL == ftpserver_CheckLogon(bCmdIndex, pbData, ctx,idx))
        {
            FTP_ST_USER_OK(g_stFTPState2[idx]) = FTP_ST_PASS_OK(g_stFTPState2[idx]) = FALSE;
            ftpserver_SetReply(530,ctx);
            return FAIL;
        }
        else
            return PASS;
    }
    if ((FTP_CMD_USER == bCmdIndex) || (FTP_CMD_PASS == bCmdIndex))
        ftpserver_CheckLogon(bCmdIndex, pbData, ctx,idx);
    else if (FTP_CMD_SYST == bCmdIndex)
        ftpserver_SetReply(215,ctx);
    else if (FTP_CMD_PWD == bCmdIndex)
        ftpserver_SendCurPathMsg(ctx,idx);
    //else if (FTP_CMD_PASV == bCmdIndex) no support pasv mode
    //    ftpserver_EnterPassiveMode(ctx);
    else if (FTP_CMD_ABOR == bCmdIndex)
        ftpserver_Abort(ctx);
    else if (FTP_CMD_QUIT == bCmdIndex)
    {
        ftpserver_SetReply(221,ctx);
    }
    else if (FTP_CMD_NOOP == bCmdIndex)
        ftpserver_SetReply(200,ctx);
    else if (FTP_CMD_LIST == bCmdIndex)
    {
        ftpserver_SetReply(150,ctx);
        ftpserver_ScanFileList(FTP_ST_CUR_PATH(g_stFTPState2[idx]),idx);

        ftpServerHandleCmdLIST(ctx);
    }
    else if (FTP_CMD_STOR == bCmdIndex)
    {
        ftpserver_ParseFileName(g_pbCmdBuf);

#ifndef WIFI_TESTPLAN
        if (FS_SUCCEED != ftpserver_CreateFile(g_pbFileNameBuf, g_pbFileExtBuf))
        {
            ftpserver_SetReply(550,ctx);
        }
        else 
#endif
        {
            ftpserver_SetReply(150,ctx);
            ftpServerHandleCmdSTOR(ctx,idx);
        }

    }
    else if (FTP_CMD_RETR == bCmdIndex)
    {
        ftpserver_ParseFileName(g_pbCmdBuf);

#ifndef WIFI_TESTPLAN
        //mpDebugPrint("g_pbFileNameBuf %s",g_pbFileNameBuf);
        //mpDebugPrint("g_pbFileExtBuf %s",g_pbFileExtBuf);
        if (FS_SUCCEED != FileSearch(DriveChange(FTP_CONN_DRIVE), g_pbFileNameBuf, g_pbFileExtBuf, E_FILE_TYPE))
        {
            ftpserver_SetReply(550,ctx);
        }
        else
#endif
        {
            ftpserver_SetReply(150,ctx);

#ifndef WIFI_TESTPLAN
            g_dwFileTotalSize[idx] = LoadAlien32((void *) (&DriveGet(FTP_CONN_DRIVE)->Node->Size));
            //mpDebugPrint("\ng_dwFileTotalSize %d\n",g_dwFileTotalSize[idx]);
#else
            g_dwFileTotalSize[idx] = 8 * 1024 * 1024;    /* 8 MB */
//            g_dwFileTotalSize = 1024 * 1024;    /* 1 MB */
#endif

            ftpServerHandleCmdRETR(ctx,idx);

#ifndef WIFI_TESTPLAN
            //DriveChange(SDIO);
            DriveChange(USB_WIFI_DEVICE);
#endif
        }
    }
    else if ((FTP_CMD_CDUP == bCmdIndex) || (FTP_CMD_CWD == bCmdIndex))
        ftpserver_ChangeDirectory(bCmdIndex, pbData, ctx,idx);
    else if (FTP_CMD_TYPE == bCmdIndex)
    {
        if ((*pbData == 'a') || (*pbData == 'A'))
            FTP_ST_CUR_TYPE(g_stFTPState2[idx]) = FTP_DATA_TYPE_ASCII;
        else if ((*pbData == 'i') || (*pbData == 'I'))
            FTP_ST_CUR_TYPE(g_stFTPState2[idx]) = FTP_DATA_TYPE_IMAGE;
        else        
        {
            ftpserver_SetReply(501,ctx);
            return FAIL;
        }
        ftpserver_SetReply(200,ctx);
    }
    else if (FTP_CMD_MODE == bCmdIndex)
    {
        if ((*pbData == 's') || (*pbData == 'S'))
            FTP_ST_CUR_MODE(g_stFTPState2[idx]) = FTP_DATA_MODE_STREAM;
        else        
        {
            ftpserver_SetReply(501,ctx);
            return FAIL;
        }
        ftpserver_SetReply(200,ctx);
    }
    else if (FTP_CMD_STRU == bCmdIndex)
    {
        if ((*pbData == 'f') || (*pbData == 'F'))
            FTP_ST_CUR_MODE(g_stFTPState2[idx]) = FTP_DATA_STRU_FILE;
        else if ((*pbData == 'r') || (*pbData == 'R'))
            FTP_ST_CUR_MODE(g_stFTPState2[idx]) = FTP_DATA_STRU_RECORD;
        else        
        {
            ftpserver_SetReply(501,ctx);
            return FAIL;
        }
        ftpserver_SetReply(200,ctx);
    }
    else if (FTP_CMD_PORT == bCmdIndex)
    {
        ftpParsePort(pbData, ctx);
        ftpserver_SetReply(200,ctx);
    }
    else
        goto CMD_NOT_EXIST;

    return PASS;

CMD_NOT_EXIST:
    MP_DEBUG("-E- Command isn't existed");
    ftpserver_SetReply(502,ctx);
    return FAIL;
}

void ftpserver_Connected(BYTE idx)
{
    MP_DEBUG("-I- FTP command port %d is connected!",idx);
    memset (&g_stFTPState2[idx], 0, sizeof(ST_FTP_STATE));
}
void ftpserver_Closed(BYTE idx)
{
    MP_DEBUG("-I- FTP command port %d is closed!",idx);
    memset(&g_stFTPState2[idx], 0, sizeof(ST_FTP_STATE));
}

void get_Dummy_File(char *buf)
{
    short i, pos=0;
    /* 45 * 32 = 1440 bytes */
    for (i=0; i < 45; i++)
            {
        strcpy(&buf[pos], "abcdefghijklmnopqrstuvwxyz0000\r\n");
        pos += 32;
	    }
} 
