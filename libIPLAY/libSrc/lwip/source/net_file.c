
// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE 	0

/*
// Include section
*/
//#include <string.h>
#include "iplaysysconfig.h"
#if ___PLATFORM___ == 0x652
    #include "../../STD_DPF/include/Platform_MP652.h"      
#elif ___PLATFORM___ == 0x660
    #include "../../STD_DPF/include/Platform_MP660.h"        
#endif

#include "global612.h"
#include "mpTrace.h"
#include <linux/types.h>
#include "xpg.h"
#include "taskid.h"
#include "uip.h"
#include "net_autosearch.h"
#include "flagdefine.h"
#include "..\..\..\..\libIPLAY\libSrc\Xml\INCLUDE\Netfs_pri.h"
#include "..\..\..\..\libIPLAY\libSrc\Xml\INCLUDE\Netfs.h"

//#define MpMemCopy(d,s,l)		MEMCPY(d,s,l)

#define NET_CACHE_SIZE		0x80000
BYTE XML_BUFF[XML_SIZE];
extern BYTE g_bXpgStatus;
extern XML_ImageBUFF_link_t *g_qtr;
extern DWORD g_recvsize;

void DisableNetFileCache();

// define constant for state of NET_CACHE_ENTRY
#define NET_CACHE_EMPTY		0
#define NET_CACHE_LOADING	1
#define NET_CACHE_READY		2

typedef struct {
	DWORD Buffer;
	DWORD Start;
	DWORD End;
	DWORD State;
} NET_CACHE_ENTRY;


// define constant for state of NET_CACHE
#define NET_CACHE_ENABLE	0x00000001
#define NET_CACHE_REFLASH	0x00000002

typedef struct {
	DWORD ServerIndex;
	DWORD FileIndex;
	DWORD FileSize;
	DWORD CurEntry;
	DWORD ReflashStart;
	DWORD State;
	NET_CACHE_ENTRY entry[2];
} NET_CACHE;


#pragma alignvar(4)
NET_CACHE NetCache;

#pragma alignvar(4)
ST_NET_FILEBROWSER Net_FileBrowser;
#if MAKE_XPG_PLAYER
extern ST_NET_FILEBROWSER *g_psNet_FileBrowser;
#endif

extern Net_App_State App_State;
//don't work
//TODO with UI
//extern ST_SYSTEM_CONFIG g_sSystemConfig;
//extern SERVER_BROWSER ServerBrowser;
extern ST_NET_FILEENTRY * g_FileEntry;
extern DWORD *g_pdwExtArray;

extern void MoveNetData2Buf();
BYTE *g_NetGetFileBuff = NULL;
#if MAKE_XPG_PLAYER
extern ST_SYSTEM_CONFIG *g_psSysConfig;
#endif
#if NET_UPNP
extern Upnp_file_list_t g_Upnp_File_List;
extern int gcurl_filesize;
char *gcurrent_url;
extern char *gavtransuri;
extern unsigned char bexit_upupctrl;
#endif

void ConvertIP2Str(BYTE *ipAddr, BYTE *HostStr)
{
	BYTE i, j, *ip, *str;

	ip = ipAddr;
	str = HostStr;

	for(i=0; i<4; i++)
	{
		j = *ip;
		*str = 0x30 + j / 100;
		j = j % 100;
		str++;
		*str = 0x30 + j / 10;
		str++;
		*str = 0x30 + j % 10;
		str ++;
		*str = '.';
		str ++;
		ip ++;
	}

	str--;
	*str = 0;
}

#if Write_To_NAND
	extern int File_Serial;
	extern int File_Serial_Index[];
#endif

#if MAKE_XPG_PLAYER
//don't work
//TODO with UI
void NetFileBrowerInitial()
{
	int i;
	STXPGMOVIE *pstMovie = &g_stXpgMovie;

	g_psSysConfig = g_psSystemConfig;
	g_psSystemConfig->dwCurrentOpMode = OP_IMAGE_MODE;

	DisableNetFileCache();
	FileBrowserInitExtArray(OP_IMAGE_MODE);
	TaskYield();

	//don't work
//TODO with UI
#if 0
       Net_FileBrowser.FileEntry = (ST_NET_FILEENTRY *)(&g_psSystemConfig->sFileBrowser.sSearchFileList[0]);
#else
	Net_FileBrowser.FileEntry = (void *)(&g_psSystemConfig->sFileBrowser.sSearchFileList[0]);
#endif
	g_psNet_FileBrowser = (ST_NET_FILEBROWSER *) ((DWORD) & Net_FileBrowser | 0xa0000000);

//	g_psNet_FileBrowser->dwListCount = pstMovie->m_dwListCount;	//Gino add at 071005 for photo in list mode
	g_psNet_FileBrowser->dwCurrentFile = 0;
	g_psNet_FileBrowser->dwNumberOfFile = 0;
//	g_psNet_FileBrowser->dwListIndex = 0;

	g_FileEntry = (ST_NET_FILEENTRY *)(g_psNet_FileBrowser->FileEntry);
	g_pdwExtArray = (DWORD *)(&g_psSysConfig->sFileBrowser.dwFileExtArray);

	g_psNet_FileBrowser->FileInfoStart =
	g_psNet_FileBrowser->FileInfoEnd = NULL;

	memset(&g_psNet_FileBrowser->mem_allocated, 0, sizeof(g_psNet_FileBrowser->mem_allocated));

#if Write_To_NAND
	Reset_File_Serial();
#endif


}
#endif	/*MAKE_XPG_PLAYER*/

#if NET_UPNP
void NetUpnpBrowerInitial()
{
	extern int gdir_child_total_count;
	extern netfs_meta_entry_t *gnetfs_dir;
	Upnp_file_list_t *pt_upnp_file_list;

	pt_upnp_file_list = &g_Upnp_File_List;

	pt_upnp_file_list->dwTotalFile = (DWORD) gdir_child_total_count;
	pt_upnp_file_list->bListIndex = 0;
	pt_upnp_file_list->dwCurrentIndex = 0;
	pt_upnp_file_list->bTreeLevel = 0;
	pt_upnp_file_list->pt_FirstEntry = gnetfs_dir;		//Direct to ".." to simplify the action while folder is empty
	pt_upnp_file_list->pt_ListFirstEntry = gnetfs_dir;
	pt_upnp_file_list->pt_CurEntry = gnetfs_dir->next;	//It may be NULL while the folder is empty
	pt_upnp_file_list->pt_TreeFirstEntry = gnetfs_dir;
#if MAKE_XPG_PLAYER
	g_psNet_FileBrowser->dwNumberOfFile = (DWORD)gdir_child_total_count;
#endif
	//Net_Scan_UpnpTotalFile(Net_Get_UpnpTreeFirstEntry());
}
#endif

void *strtoupper(char fstr[])
{
          int i;
          for (i=0;i<=strlen(fstr);i++) {
              if (fstr[i]>=97 && fstr[i]<=122) {
                  if ((fstr[i]-32>=65) && (fstr[i]-32<=90)) {
                      fstr[i]-=32;
                  }
              }
          }
          return fstr;
 }

int strpos( char *haystack,  char *needle )
{
	char *pDest;
	int position;

	#ifndef UNICODE
		pDest = (char *) strstr( haystack, needle );
	#else
		pDest = (char *) wcsstr( haystack, needle );
	#endif /* UNICODE */

	if( pDest )
		position = pDest - haystack;
	else
	{
		return -1;
	}
	return position;

}

#if MAKE_XPG_PLAYER
BOOL NetSetFileIndex(DWORD dwIndex)
{
	if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
		return false;

	g_psNet_FileBrowser->dwCurrentFile = dwIndex;
	return true;
}

#if 1
DWORD NetAddCurIndex(SWORD i)
{
	DWORD dwCurIndex;

	if(g_psNet_FileBrowser->dwNumberOfFile == 0)		// abel 20070930
		return 0;

	dwCurIndex = g_psNet_FileBrowser->dwCurrentFile;

	if (i < 0)
	{
		if (dwCurIndex >= -i)
			dwCurIndex += i;
		else if (g_psNet_FileBrowser->dwNumberOfFile != 0)
			dwCurIndex = g_psNet_FileBrowser->dwNumberOfFile - 1;
	}
	else
	{
		dwCurIndex += i;
		while (dwCurIndex >= g_psNet_FileBrowser->dwNumberOfFile)		// abel 20070930
			dwCurIndex -= g_psNet_FileBrowser->dwNumberOfFile;

	}
	return dwCurIndex;
}
#endif

DWORD NetGetFileIndex()
{
	return g_psNet_FileBrowser->dwCurrentFile;
}

DWORD NetGetTotalFile()
{
#if 0//NET_UPNP
	if (g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST)
		return g_Upnp_File_List.dwTotalFile;
	else
#endif
		return g_psNet_FileBrowser->dwNumberOfFile;
}

#if 1 //CJ modify 020409 copy the serial 8k linklist data to fill up BUFFER_FILL_SIZE(now 256k) in mpStreamApi.c to read for each time to decode
int NetGetFile(DWORD dwIndex, BYTE *pbBuffer, DWORD Start, DWORD End)
{
     XML_ImageBUFF_link_t *ptr;
#if Write_To_NAND
	if(App_State.dwState &  NET_RECVPICASA)
	{
		STREAM *handle = NULL;
		static DRIVE *sDrv;
		char* file_ext;
		BYTE bMcardId = NAND;
		char name[32];
		int size;

		ST_NET_FILEENTRY * FileEntry;

		if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
			return 0;

		FileEntry = (ST_NET_FILEENTRY *)(&g_psNet_FileBrowser->FileEntry[dwIndex]);

		memset(name, '\0', 32);
		sprintf(name, "%d.jpg", FileEntry->state);
		MP_DEBUG("=============Open %s=============", name);

		file_ext = name;

		while(*file_ext != 0x2e){
			file_ext++;
		}
		*file_ext = '\0';
		file_ext++;

		strtoupper(name);
		strtoupper(file_ext);


		BYTE CurId = DriveCurIdGet();
		MP_DEBUG("curr id = %d", DriveCurIdGet());

		DRIVE *drv;
		drv = DriveChange(bMcardId);
		if (DirReset(drv) != FS_SUCCEED)
			return 0;

		handle = FileSearch(drv, name, file_ext, E_FILE_TYPE);
		if (handle != NULL){
			MP_DEBUG("error file");

			drv = DriveChange(CurId);
			if (DirReset(drv) != FS_SUCCEED)
				return 0;

			return 0;
		}
		else{
			sDrv=DriveGet(bMcardId);
			handle = FileOpen(sDrv);
			MP_DEBUG("start = %d ,end = %d", Start,End);
			SemaphoreRelease(FILE_READ_SEMA_ID);
			Seek(handle, Start);
			size = FileRead(handle,pbBuffer, 256 << 10);
			SemaphoreWait(FILE_READ_SEMA_ID);

			MP_DEBUG("size = %d", size);

			FileClose(handle);

			drv = DriveChange(CurId);
			if (DirReset(drv) != FS_SUCCEED)
				return 0;

			return size;
		}
	}
	else
#endif
	if(App_State.dwState&NET_YOUTUBE)
	{
#if HAVE_YOUTUBE
	    //mpDebugPrint("NetGetFile NET_YOUTUBE");
		return youtube_get_data(pbBuffer,Start,End);
#else
    	return 0;
#endif
	}
	else if(App_State.dwState&NET_YOUKU3G)
	{
#if HAVE_YOUKU3G
	    //mpDebugPrint("NetGetFile NET_YOUTUBE");
		return youku3g_get_data(pbBuffer,Start,End);
#else
    	return 0;
#endif
	}

	else
	{
		ST_NET_FILEENTRY * FileEntry;
		int size,btestserver;
		u8 mretry = 0,mcheckserver=0;
		unsigned int i;

		//mpDebugPrint("%d %d",dwIndex,g_psNet_FileBrowser->dwNumberOfFile);
		if( (g_bXpgStatus != XPG_MODE_DLNA_1_5_DMR) && (g_bXpgStatus != XPG_MODE_UPNP_FILE_LIST) )
			if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
				return 0;

		if(Start >= End)
			return 0;
#if NET_UPNP
		if( (g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST) || (g_bXpgStatus == XPG_MODE_DLNA_1_5_DMR) )
		{
do_retry:
			for(i =0 ; i< 10000 ; i++ )
				TaskYield();
			mpDebugPrint("NetGetFile =====> Start = %d , End = %d %s",Start,End,gcurrent_url);
			size = Get_Image_File_Range(gcurrent_url, pbBuffer,Start,End,FALSE,30);
			if( size != (End - Start +1) && mretry < 3 )
			{
				if( NetConfiged() == FALSE  )
				{
					if( NetDevicePresent()== FALSE )
						return 0;
					else
					{

						for( i = 0 ; i < 30 ; i++ )
						{
							mpDebugPrint("call TaskSleep 10 sec a %d",i);
							TaskSleep(20000);
							if( NetConfiged() || (NetDevicePresent() == FALSE) )
								break;
							mpDebugPrint("exit call TaskSleep 10 sec a");
						}
						if( NetDevicePresent() == FALSE )
						{
								return 0;
						}
						if( !NetConfiged() )
						{
							mpDebugPrint("!NetConfiged()");
							if( Check_UPNP_Server() < 0 )
								return 0;
						}
					}
				}
				else
				{
					mretry++;
					if( g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
					{
retry_check_server:
						btestserver = Test_UPNP_Server();
						if( btestserver >= 0 )
							goto do_retry;
						if( btestserver < 0 && mcheckserver < 3  )
						{
							if( NetConfiged() == FALSE  )
							{
								if( NetDevicePresent()== FALSE )
									return 0;
								else
								{
									mpDebugPrint("call TaskSleep 10 sec a2 %d",i);
									for( i = 0 ; i < 12 ; i++ )
									{
										TaskSleep(10000);
										if( NetConfiged() || (NetDevicePresent() == FALSE) )
											break;
									}
									mpDebugPrint("call TaskSleep 10 sec a2");
								}
								mcheckserver++;
								goto retry_check_server;
							}
							else
							{
								mcheckserver++;
								goto retry_check_server;
							}
						}
						else
						{
							if( Check_UPNP_Server() < 0 )
								return 0;
							else
								mpDebugPrint("goto do_retry");
						}

					}
				}
				goto do_retry;
			}
			//Dealy
			for(i =0 ; i< 10000 ; i++ )
				TaskYield();
			UartOutText("# ");
			for(i =0 ; i< 10000 ; i++ )
				TaskYield();
			UartOutText("# ");
			MP_DEBUG("size %d",size);
			return size;
		}
		else
#endif			
		{
		    int i=0,link_buf_idx=0;
			FileEntry = NetGetFileEntry(dwIndex);
			//mpDebugPrint("NetGetFile start %d end %d",Start,End);
			//mpDebugPrint("FileEntry->size %d ",FileEntry->size);
			link_buf_idx = Start%IMAGE_BUF;
			//mpDebugPrint("link_buf_idx %d",link_buf_idx);
			while(g_qtr!= NULL)	
			{
				if(App_State.dwOffset >= (256 << 10))
				{
					break;
				}
				switch(i)
				{
				  case 0:
				  	if(g_qtr->buff_len>link_buf_idx)
				  	{
				   		//mpDebugPrint("NetGetFile 0 =====> offset = %d , len = %d",App_State.dwOffset,g_qtr->buff_len-link_buf_idx);
				   		MpMemCopy((BYTE *)(pbBuffer+App_State.dwOffset), &g_qtr->BUFF[link_buf_idx], g_qtr->buff_len-link_buf_idx);
				   		App_State.dwOffset += g_qtr->buff_len-link_buf_idx;
				  	}
					//else
						//mpDebugPrint("NetGetFile 0 =====> offset = %d ,g_qtr len = %d",App_State.dwOffset,g_qtr->buff_len);	
				   break;
				  case 1:
				   //mpDebugPrint("NetGetFile 1 =====> offset = %d , len = %d",App_State.dwOffset,g_qtr->buff_len);
				   MpMemCopy((BYTE *)(pbBuffer+App_State.dwOffset), &g_qtr->BUFF, g_qtr->buff_len);
				   App_State.dwOffset += g_qtr->buff_len;
				   break;
				  case 2:
				   //mpDebugPrint("NetGetFile 2 =====> offset = %d , len = %d",App_State.dwOffset,link_buf_idx);
				   MpMemCopy((BYTE *)(pbBuffer+App_State.dwOffset), &g_qtr->BUFF, link_buf_idx);
				   App_State.dwOffset += link_buf_idx;
				   link_buf_idx = 0;
				   break;


				   
				}
				if(g_qtr->link != NULL)
				{
				    g_qtr = g_qtr->link;
					
				}
				else
				{
					break;
				}
				i++;
			}
			//mpDebugPrint("App_State.dwOffset %d",App_State.dwOffset);
			if((End+1)>=FileEntry->size)
			{
				g_qtr = App_State.XML_BUF1;
			}
			size = App_State.dwOffset;
			App_State.dwOffset = 0 ;
			return size;
		}
	}

}

#else

#define PERFORMANCE_TEST		0
static int offset = 0;

int NetGetFile(DWORD dwIndex, DWORD dwBuffer, DWORD Start, DWORD End)
{
	ST_NET_FILEENTRY * FileEntry;

	BYTE HostStr[16];
	BYTE *website;
	BYTE * ipaddr;
#if PERFORMANCE_TEST
	DWORD dwTimerCount;

	dwTimerCount = GetSysTime();
#endif
#if 1//DLNA_1_5
	mpDebugPrint("xx %d %d ",dwIndex,g_psNet_FileBrowser->dwNumberOfFile);
	if( (g_bXpgStatus != XPG_MODE_DLNA_1_5_DMR) && (g_bXpgStatus != XPG_MODE_UPNP_FILE_LIST) )
#endif
		if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
			return FAIL;

	if(Start > End)  // abel 20071002
		return PASS;

	App_State.dwBuffer = dwBuffer;
	App_State.dwReconnect  = 0;
	App_State.dwOffset = 0;
	App_State.dwTotallen = 0;
	g_recvsize = 0;
NetGetFile_again:

   	if(App_State.dwReconnect > 4)  // retry 4 times
		return FAIL;

	FileEntry = (ST_NET_FILEENTRY *)(&g_psNet_FileBrowser->FileEntry[dwIndex]);
	App_State.dwState |= NET_TX_RX_DATA;
	App_State.dwEndToggle = 0;

	if(g_bXpgStatus == XPG_MODE_NET_PC_PHOTO)
	{
#if 0
		ConvertIP2Str((BYTE *)(&ServerBrowser.ServerList[ServerBrowser.dwCurrentServer].ipaddr[0]), (BYTE *)(&HostStr[0]));
		webclient_get((BYTE *)(&HostStr[0]), MPX_DATA_PORT, (BYTE *)(&FileEntry->Link[0]), (Start+App_State.dwOffset), End);
#else
    #if 0 //original code
		MpMemCopy((BYTE *)dwBuffer, &g_qtr->BUFF, IMAGE_BUF);
		if(g_qtr->link != NULL)
			g_qtr = g_qtr->link;
    #else //new code
		while(g_qtr != NULL)
		{
			if(offset >= ((256 << 10)-1))
			{
				offset = 0 ;
				break;
			}
			MP_DEBUG("NetGetFile =====> offset = %d , len = %d",offset,g_qtr->buff_len  );
			MpMemCopy((BYTE *)(dwBuffer+offset), &g_qtr->BUFF, g_qtr->buff_len);
			offset += g_qtr->buff_len;
			if(g_qtr->link != NULL)
				g_qtr = g_qtr->link;
			else
			{
				offset = 0 ;
				break;
			}
		}
    #endif
		return PASS;
#endif
	}
	else if(g_bXpgStatus == XPG_MODE_NET_FLICKR)
	{
#if 0
		website =(BYTE *) Net_GetWebSite(FileEntry->Link);
		ipaddr = (BYTE *)SearchServerByName(website);
		ConvertIP2Str(ipaddr, HostStr);
		webclient_get(HostStr, WEB_SERVER_PORT, FileEntry->Link, (Start+App_State.dwOffset), End);
#else
    #if 0 //original code
		MpMemCopy((BYTE *)dwBuffer, &g_qtr->BUFF, IMAGE_BUF);
		if(g_qtr->link != NULL)
			g_qtr = g_qtr->link;
    #else //new code
		while(g_qtr != NULL)
		{
			MpMemCopy((BYTE *)(dwBuffer+offset), &g_qtr->BUFF, g_qtr->buff_len);
			offset += g_qtr->buff_len;
			if(g_qtr->link != NULL)
				g_qtr = g_qtr->link;
			else
				break;
		}
    #endif
		return PASS;
#endif
	}
	else if((g_bXpgStatus == XPG_MODE_NET_PICASA) ||
		    (g_bXpgStatus == XPG_MODE_NET_GCE) ||
		    (g_bXpgStatus == XPG_MODE_RSS_TITLE))
	{
    #if 0 //original code
		MpMemCopy((BYTE *)dwBuffer, &g_qtr->BUFF, IMAGE_BUF);
		if(g_qtr->link != NULL)
			g_qtr = g_qtr->link;
    #else //new code
		while(g_qtr != NULL)
		{
			MpMemCopy((BYTE *)(dwBuffer+offset), &g_qtr->BUFF, g_qtr->buff_len);
			offset += g_qtr->buff_len;
			if(g_qtr->link != NULL)
				g_qtr = g_qtr->link;
			else
				break;
		}
    #endif
		return PASS;
	}

#if YOUGOTPHOTO
	else if(g_bXpgStatus == XPG_MODE_NET_YOUGOTPHOTO)
	{
#if 0
       	website =(BYTE *) Net_GetWebSite(FileEntry->Link);
		ipaddr = (BYTE *)SearchServerByName(website);
		ConvertIP2Str(ipaddr, HostStr);
		webclient_get(HostStr, WEB_SERVER_PORT, FileEntry->Link, (Start+App_State.dwOffset), End);
#else
    #if 0 //original code
		MpMemCopy((BYTE *)dwBuffer, &g_qtr->BUFF, IMAGE_BUF);
		if(g_qtr->link != NULL)
			g_qtr = g_qtr->link;
    #else //new code
		while(g_qtr != NULL)
		{
			MpMemCopy((BYTE *)(dwBuffer+offset), &g_qtr->BUFF, g_qtr->buff_len);
			offset += g_qtr->buff_len;
			if(g_qtr->link != NULL)
				g_qtr = g_qtr->link;
			else
				break;
		}
    #endif
		return PASS;
#endif
	}
#endif
#if HAVE_FRAMECHANNEL
	else if(g_bXpgStatus == XPG_MODE_FRAMECHANNEL)
	{
    #if 0 //original code
		MpMemCopy((BYTE *)dwBuffer, &g_qtr->BUFF, IMAGE_BUF);
		if(g_qtr->link != NULL)
			g_qtr = g_qtr->link;
    #else //new code
		while(g_qtr != NULL)
		{
			MpMemCopy((BYTE *)(dwBuffer+offset), &g_qtr->BUFF, g_qtr->buff_len);
			offset += g_qtr->buff_len;
			if(g_qtr->link != NULL)
				g_qtr = g_qtr->link;
			else
				break;
		}
    #endif
		return PASS;
	}
#endif
#if HAVE_FRAMEIT
	else if(g_bXpgStatus == XPG_MODE_FRAMEIT)
	{
    #if 0 //original code
		MpMemCopy((BYTE *)dwBuffer, &g_qtr->BUFF, IMAGE_BUF);
		if(g_qtr->link != NULL)
			g_qtr = g_qtr->link;
    #else //new code
		while(g_qtr != NULL)
		{
			if(offset >= ((256 << 10)-1))
			{
				offset = 0 ;
				break;
			}
			MP_DEBUG("NetGetFile =====> offset = %d , len = %d",offset,g_qtr->buff_len  );
			MpMemCopy((BYTE *)(dwBuffer+offset), &g_qtr->BUFF, g_qtr->buff_len);
			offset += g_qtr->buff_len;
			if(g_qtr->link != NULL)
				g_qtr = g_qtr->link;
			else
			{
				offset = 0 ;
				break;
			}
		}
    #endif
		return PASS;
	}
#endif
	while(App_State.dwState & NET_TX_RX_DATA)	// wait transfer end
	{
		if(App_State.dwState & NET_TIMEOUT)
		{
			App_State.dwReconnect++;
			App_State.dwState &= ~NET_TIMEOUT;

			TaskYield();
			TaskYield();

#if (Make_SDIO == MARVELL_WIFI)
			wlan_tx_timeout();
#endif

			if(App_State.dwReconnect <4)
            {
#if 0
				if (App_State.dwState & NET_RECVPC)
             	{
					Xml_BUFF_free(NET_RECVPC);
					Xml_BUFF_init(NET_RECVPC);
					App_State.dwOffset = 0;
       				App_State.dwTotallen = 0;
             	}
				else if(App_State.dwState & NET_RECVRSS)
				{
					Xml_BUFF_free(NET_RECVRSS);
					Xml_BUFF_init(NET_RECVRSS);
					App_State.dwOffset = 0;
       				App_State.dwTotallen = 0;
				}
				else if(App_State.dwState & NET_RECVPICASA)
				{
					Xml_BUFF_free(NET_RECVPICASA);
					Xml_BUFF_init(NET_RECVPICASA);
					App_State.dwOffset = 0;
       				App_State.dwTotallen = 0;
				}
				else if(App_State.dwState & NET_RECVGCE)
				{
					Xml_BUFF_free(NET_RECVGCE);
					Xml_BUFF_init(NET_RECVGCE);
					App_State.dwOffset = 0;
       				App_State.dwTotallen = 0;
				}
				else if(App_State.dwState & NET_RECVHTTP)
				{
					Xml_BUFF_free(NET_RECVHTTP);
					Xml_BUFF_init(NET_RECVHTTP);
					App_State.dwOffset = 0;
       				App_State.dwTotallen = 0;
				}
				else if(App_State.dwState & NET_RECVFLICKR)
				{
					App_State.dwOffset = 0;
       				App_State.dwTotallen = 0;
				}
				else

				if(App_State.dwState & NET_RECVYOUGOTPHOTO)
				{
					App_State.dwTotallen = App_State.dwOffset;
				}

#endif
				App_State.dwReconnect = 0;
				App_State.dwEndToggle = 0;

				goto NetGetFile_again;
			}
		}

		if(App_State.dwState & NET_DISCONNECT)
			return FAIL;
		TaskYield();
	}

#if PERFORMANCE_TEST
	dwTimerCount = GetSysTime() - dwTimerCount;
	UartOutText("Total =");
	UartOutValue(dwTimerCount, 8);
	UartOutText(" *100 ms\r\n");
#endif

	return PASS;
}
#endif

void EnterNetPhotoMenu()
{
	int i;
	STXPGMOVIE *pstMovie = &g_stXpgMovie;

	g_psSysConfig = g_psSystemConfig;

	DisableNetFileCache();
	FileBrowserInitExtArray(OP_IMAGE_MODE);
	TaskYield();

	g_psSystemConfig->dwCurrentOpMode = OP_IMAGE_MODE;

	g_psNet_FileBrowser->dwCurrentFile = 0;
	g_psNet_FileBrowser->dwNumberOfFile = 0;
//	g_psNet_FileBrowser->dwListIndex = 0;

	g_FileEntry = (ST_NET_FILEENTRY *)(g_psNet_FileBrowser->FileEntry);
	g_pdwExtArray = (DWORD *)(&g_psSysConfig->sFileBrowser.dwFileExtArray);


}
#endif	/*MAKE_XPG_PLAYER*/

void EnterNetMusicMenu()
{
	int i;
	STXPGMOVIE *pstMovie = &g_stXpgMovie;
	FileBrowserInitExtArray(OP_AUDIO_MODE);
	TaskYield();
	while(!NetConfiged())
		TaskYield();

	if(!(App_State.dwState & NET_ENABLE))
		return;
/*
	if(ScanNetFileList(OP_AUDIO_MODE) == FAIL)
	{
		mpDebugPrint(" xxxxxxx         ScanNetFileList(OP_AUDIO_MODE) == FAIL  xxxxxxxxxxx");
		xpgSearchAndGotoPage("NetworkSTab", 11);
		StartAutoSearch();

	}
*/
	mpDebugPrint(" xxxxxxx         ScanNetFileList OK    xxxxxxxxxxx");
	g_psSystemConfig->dwCurrentOpMode = OP_AUDIO_MODE;
//	g_psNet_FileBrowser->dwListCount = pstMovie->m_dwListCount;
	g_bXpgStatus = XPG_MODE_NET_PC_AUDIO;

}

#if MAKE_XPG_PLAYER
BYTE *NetGetFileName(DWORD dwIndex)
{
	ST_SEARCH_INFO * sinfo;
	ST_NET_FILEENTRY * pFileEntry;

//    mpDebugPrint("NetGetFileName: dwIndex = %d", dwIndex);
	if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
		return 0;

	sinfo = (ST_SEARCH_INFO *)g_psNet_FileBrowser->FileEntry;
	sinfo = &sinfo[dwIndex];

	memcpy(&pFileEntry, sinfo->bName,4);

	return (BYTE *)(&pFileEntry->Name[0]);

}


ST_NET_FILEENTRY *NetGetFileEntry(DWORD dwIndex)
{
	ST_SEARCH_INFO * sinfo;
	ST_NET_FILEENTRY * pFileEntry;
//    mpDebugPrint("NetGetFileEntry: dwIndex = %d", dwIndex);
	if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
		return 0;

	sinfo = (ST_SEARCH_INFO *)g_psNet_FileBrowser->FileEntry;
	sinfo = &sinfo[dwIndex];

	memcpy(&pFileEntry, sinfo->bName,4);

	return pFileEntry;
}


ST_SEARCH_INFO *NetGetCurFileEntry()		// abel 20070930
{
	ST_SEARCH_INFO * sinfo;
	if(g_psNet_FileBrowser->dwNumberOfFile == 0)
		return NULL;

	sinfo = (ST_SEARCH_INFO *)g_psNet_FileBrowser->FileEntry;
	sinfo = &sinfo[g_psNet_FileBrowser->dwCurrentFile];

	return sinfo;
}


ST_SEARCH_INFO *NetGetFileSearchInfo(DWORD dwIndex)
{
	ST_SEARCH_INFO * sinfo;
//    mpDebugPrint("NetGetFileSearchInfo: dwIndex = %d", dwIndex);
	if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
		return 0;

	sinfo = (ST_SEARCH_INFO *)g_psNet_FileBrowser->FileEntry;
	sinfo = &sinfo[dwIndex];

	return sinfo;
}

BYTE *NetGetFileExt(DWORD dwIndex)
{
	ST_SEARCH_INFO * sinfo;

//    mpDebugPrint("NetGetFileExt: dwIndex = %d", dwIndex);
	if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
		return 0;

	sinfo = (ST_SEARCH_INFO *)g_psNet_FileBrowser->FileEntry;
	sinfo = &sinfo[dwIndex];

	return (BYTE *)(&sinfo->bExt[0]);

}
#endif	/*MAKE_XPG_PLAYER*/

//////////////////////////////////
//
//          Get Upnp File Entry
//
/////////////////////////////////
#if NET_UPNP
BYTE *NetGetUpnpFileName(DWORD dwIndex, netfs_meta_entry_t *upnp_file_entry)
{
	BYTE i = 0;
	BYTE *name;
	netfs_meta_entry_t *pt_next;

	pt_next = upnp_file_entry->next;

	while ((pt_next != NULL) && (i < dwIndex))
	{
		pt_next = pt_next->next;
		i++;
	}
	if (pt_next == NULL)
		return NULL;
	else
	{
		name = (BYTE *) pt_next->name;
		return name;
	}
}

//----------------------------------------------------------------------------------------------
//
//		The return entry will be the current entry->prev
//
//----------------------------------------------------------------------------------------------
netfs_meta_entry_t *NetGet_UpnpPreEntry(DWORD dwIndex, netfs_meta_entry_t *upnp_file_entry)
{
	BYTE i = 0;
	netfs_meta_entry_t *pt_pre;

	pt_pre = upnp_file_entry;	//Get current entry

	while ((pt_pre != NULL) && (i < dwIndex))
	{
		pt_pre = pt_pre->prev;
		i++;
	}

	return pt_pre;
}

netfs_meta_entry_t *NetGet_UpnpNextEntry(DWORD dwIndex, netfs_meta_entry_t *upnp_file_entry)
{
	BYTE i = 0;
	netfs_meta_entry_t *pt_next;

	pt_next = upnp_file_entry;

	while ((pt_next != NULL) && (i < dwIndex))
	{
		pt_next = pt_next->next;
		i++;
	}
	return pt_next;
}
#endif
//----------------------------------------------------------------------------------------------

#if MAKE_XPG_PLAYER
DWORD NetGetFileSize(DWORD dwIndex)
{
	if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
		return 0;

#if 0
	return g_psNet_FileBrowser->FileEntry[dwIndex].size;
#else
	ST_SEARCH_INFO * sinfo;

	sinfo = (ST_SEARCH_INFO *)g_psNet_FileBrowser->FileEntry;
	sinfo = &sinfo[dwIndex];

	return sinfo->dwFileSize;
#endif

}
#endif	/*MAKE_XPG_PLAYER*/

////////////////////////////////////////////////////////////////////
//        Net File cache
// The cache only cache one file at the same time. It use two 128kByte cache
// to cache data.
NET_CACHE_ENTRY *GetCurNetCacheEntry()
{
	if(NetCache.CurEntry)
		return (NET_CACHE_ENTRY *)(&NetCache.entry[1]);
	else
		return (NET_CACHE_ENTRY *)(&NetCache.entry[0]);
}


NET_CACHE_ENTRY *GetNextNetCacheEntry()
{
	if(NetCache.CurEntry)
		return (NET_CACHE_ENTRY *)(&NetCache.entry[0]);
	else
		return (NET_CACHE_ENTRY *)(&NetCache.entry[1]);

}

void ChgNetCacheEntry()
{
	if(NetCache.CurEntry)
		NetCache.CurEntry = 0;
	else
		NetCache.CurEntry = 1;
}


void InitNetCache()
{
	NetCache.ServerIndex = 0xffffffff;
	NetCache.FileIndex = 0xffffffff;
	NetCache.FileSize = 0;
	NetCache.CurEntry = 1;		// start cache to entry 0, so set CurEntry to 1
	NetCache.entry[0].Start = 0;
	NetCache.entry[0].End = 0;
	NetCache.entry[0].State = NET_CACHE_EMPTY;


#if 0
	 NetCache.entry[0].Buffer = SystemGetMemAddr(JPEG_SOURCE_MEM_ID);
#else
	 NetCache.entry[0].Buffer =(DWORD)XML_BUFF;
#endif

	NetCache.entry[1].Start = 0;
	NetCache.entry[1].End = 0;
	NetCache.entry[1].State = NET_CACHE_EMPTY;
#if 0
	 NetCache.entry[1].Buffer = SystemGetMemAddr(JPEG_SOURCE_MEM_ID)+ NET_CACHE_SIZE;
#else

#endif
	NetCache.ReflashStart = 0;
	NetCache.State = 0;
}



void DisableNetFileCache()
{
	NetCache.State &= ~NET_CACHE_ENABLE;
}



void ReflashNetCacheBuffer(DWORD start)
{
	if(NetCache.State & NET_CACHE_ENABLE)
	{
		NetCache.ReflashStart = start;
		NetCache.State |= NET_CACHE_REFLASH;
		EventSet(NETWARE_EVENT, EVENT_FILE_CACHE);
	}
}

#if MAKE_XPG_PLAYER
int FillNetCacheBuffer()
{
	NET_CACHE_ENTRY *entryC, *entryN;

	entryC = GetCurNetCacheEntry();
	entryN = GetNextNetCacheEntry();

	if(NetCache.State & NET_CACHE_REFLASH)
	{
		entryN->Start = NetCache.ReflashStart;

		if((NetCache.FileSize - entryN->Start) > NET_CACHE_SIZE)
			entryN->End = entryN->Start + NET_CACHE_SIZE - 1;
		else
			entryN->End = NetCache.FileSize - 1;

	}
	else
	{
		if(entryC->End >= (NetCache.FileSize-1))
			return;

		entryN->Start = entryC->End + 1;

		if((NetCache.FileSize - entryC->End) > NET_CACHE_SIZE)
			entryN->End = entryC->End + NET_CACHE_SIZE;
		else
			entryN->End = NetCache.FileSize - 1;
	}

	entryN->State = NET_CACHE_LOADING;

	NetGetFile(NetCache.FileIndex, entryN->Buffer, entryN->Start, entryN->End);

	entryN->State = NET_CACHE_READY;
	if(NetCache.State & NET_CACHE_REFLASH)
	{
		NetCache.State &= ~NET_CACHE_REFLASH;
		ChgNetCacheEntry();
		EventSet(NETWARE_EVENT, EVENT_FILE_CACHE);
	}

}

void StartNetFileCache()
{
	InitNetCache();
	NetCache.ServerIndex = GetCurrentServerIndex();
	NetCache.FileIndex = NetGetFileIndex();
	NetCache.FileSize = NetGetFileSize(NetCache.FileIndex);
	NetCache.State = NET_CACHE_REFLASH;
	FillNetCacheBuffer();
	NetCache.State = NET_CACHE_ENABLE;
}

int NetFileRead(DWORD dwIndex, BYTE *pbBuffer, DWORD Start, DWORD End)
{
	int ret;
	NET_CACHE_ENTRY *entry;
	DWORD len;
	DWORD tmpStart, tmpEnd;
	BYTE *pbAddr;

	//mpDebugPrint("NetFileRead dwIndex %d %d",dwIndex,g_psNet_FileBrowser->dwNumberOfFile);
	if( g_bXpgStatus != XPG_MODE_DLNA_1_5_DMR )
		if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
			return ABNORMAL_STATUS;

	tmpStart = Start;
	tmpEnd = End;
	//pbAddr = pbBuffer;

	//MP_DEBUG("NetFileRead 1");

#if 0 //cj
	if((NetCache.State&NET_CACHE_ENABLE) && (NetCache.FileIndex == dwIndex))
	{
		// check current cache buffer
		MP_DEBUG("NetFileRead 2");
		entry = GetCurNetCacheEntry();
		if(entry->State == NET_CACHE_READY)
		{
			// move current net cache to buffer
			if((tmpStart >= entry->Start) && (tmpStart <= entry->End))
			{
				if(tmpEnd <= entry->End)
					len = tmpEnd - tmpStart + 1;
				else
					len = entry->End - tmpStart + 1;

				MpMemCopy(pbAddr, (BYTE *)(entry->Buffer + tmpStart - entry->Start), len);

				pbAddr += len;
				tmpStart += len;

				if(tmpStart >= tmpEnd)
					return PASS;
			}
		}

		// check current cache buffer
		entry = GetNextNetCacheEntry();
		if(entry->State == NET_CACHE_READY)
		{
			// move current net cache to buffer
			if((tmpStart >= entry->Start) && (tmpStart <= entry->End))
			{
				ChgNetCacheEntry();

				if(tmpEnd <= entry->End)
					len = tmpEnd - tmpStart + 1;
				else
					len = entry->End - tmpStart + 1;

				MpMemCopy(pbAddr, (BYTE *)(entry->Buffer + tmpStart - entry->Start), len);

				pbAddr += len;
				tmpStart += len;

				if(tmpStart >= tmpEnd)
				{
					EventSet(NETWARE_EVENT, EVENT_FILE_CACHE);
					return PASS;
				}
			}
		}
	}
#endif

	SemaphoreWait(NET_CACHE_SEMA_ID);
	ret = NetGetFile(dwIndex, pbBuffer, tmpStart, tmpEnd);
	SemaphoreRelease(NET_CACHE_SEMA_ID);

	if(NetCache.State & NET_CACHE_ENABLE)
		ReflashNetCacheBuffer(tmpStart);

	return ret;
}
#endif	/*MAKE_XPG_PLAYER*/

//------------------------------------------------------
//
//                          UPNP
//
//-----------------------------------------------------
#if NET_UPNP
netfs_meta_entry_t *Net_Get_UpnpCurEntry()//netfs_meta_entry_t *upnp_file_entry)
{
	//netfs_meta_entry_t *pt_entry;
	//pt_entry = upnp_file_entry;

	//return upnp_file_entry->next;
	return g_Upnp_File_List.pt_CurEntry;
}

netfs_meta_entry_t *Net_Get_UpnpChildEntry(netfs_meta_entry_t *file_entry)
{
	//mpDebugPrint("entry name = %s   child name = %s", file_entry->name, file_entry->child->name);
	return file_entry->child;
}

netfs_meta_entry_t *Net_Get_UpnpFirstEntry()
{
	return g_Upnp_File_List.pt_FirstEntry;
}

netfs_meta_entry_t *Net_Get_UpnpListFirstEntry()
{
	return g_Upnp_File_List.pt_ListFirstEntry;
}

netfs_meta_entry_t *Net_Get_UpnpTreeFirstEntry()
{
	return g_Upnp_File_List.pt_TreeFirstEntry;
}

char NetGet_UpnpFileIsDir(DWORD dwIndex, netfs_meta_entry_t *upnp_file_entry)
{
	BYTE i = 0;
	char IsDir;

	netfs_meta_entry_t *pt_next;

	pt_next = upnp_file_entry->next;

	while ((pt_next != NULL) && (i < dwIndex))
	{
		pt_next = pt_next->next;
		i++;
	}
	if (pt_next == NULL)
		return FAIL;
	else
	{
		IsDir = pt_next->is_dir;
		return IsDir;
	}
}

void Net_Add_UpnpTreeLevel(BYTE index)
{
	g_Upnp_File_List.bTreeLevel += index;
}

BYTE Net_Get_UpnpTreeLevel()
{
	return g_Upnp_File_List.bTreeLevel;
}

void Net_Add_UpnpCurIndex(DWORD index)
{
	g_Upnp_File_List.dwCurrentIndex += index;
}

void Net_Add_UpnpListIndex(BYTE index)
{
	g_Upnp_File_List.bListIndex += index;
}

DWORD Net_Get_UpnpCurIndex()
{
	return g_Upnp_File_List.dwCurrentIndex;
}

void Net_Set_UpnpListIndex(BYTE index)
{
	g_Upnp_File_List.bListIndex = index;
}

void Net_Set_UpnpCurIndex(DWORD index)
{
	g_Upnp_File_List.dwCurrentIndex = index ;
}

BYTE Net_Get_UpnpListIndex()
{
	return g_Upnp_File_List.bListIndex;
}

void Set_Upnp_ListFirstEntry(netfs_meta_entry_t *file_entry)
{
	g_Upnp_File_List.pt_ListFirstEntry = file_entry;
}

void Set_Upnp_TreeFirstEntry(netfs_meta_entry_t *file_entry)
{
	g_Upnp_File_List.pt_TreeFirstEntry = file_entry;
}

void Set_Upnp_CurEntry(netfs_meta_entry_t *file_entry)
{
	g_Upnp_File_List.pt_CurEntry = file_entry;
}

char *Net_Get_UpnpUrl(netfs_meta_entry_t *upnp_file_entry)
{
	netfs_meta_entry_t *pt_entry;

	pt_entry = upnp_file_entry;
	return pt_entry->file_entry->url;
}

char *Net_Get_UpnpCurURL(void)
{
	return g_Upnp_File_List.pt_CurEntry->file_entry->url;
}

int Net_Get_UpnpCurFileSize(void)
{
	return g_Upnp_File_List.pt_CurEntry->file_entry->file_length;
}

netfs_meta_entry_t *Net_Upnp_SearchTreeFirstEntry(netfs_meta_entry_t *file_entry)
{
	DWORD i = 0;
	netfs_meta_entry_t *pt_entry;

	pt_entry = file_entry;

	while ((pt_entry->prev != NULL)/*  && (i <= g_Upnp_File_List.dwTotalFile)*/)
	{
		pt_entry = pt_entry->prev;
		i ++;
	}

	if (i == 0)
		Net_Set_UpnpCurIndex(0);
	else
		Net_Set_UpnpCurIndex(i -1);	//Add in this to set the current index
	return pt_entry;
}
void Net_Upnp_SearchPrevEntry(unsigned char type)
{
	netfs_meta_entry_t *pt_entry;
	DWORD i = 0;
	DWORD total_count;

	pt_entry = Net_Get_UpnpCurEntry();
	total_count = NetGetTotalFile();

	if ((type == IMAGE_FILE_TYPE) || (type == AUDIO_FILE_TYPE))
		i = total_count;
	else
		i = 1;

	while ((i > 0) && (pt_entry->prev->prev != NULL))
	{
		pt_entry = pt_entry->prev;
		Net_Add_UpnpCurIndex(-1);
		Set_Upnp_CurEntry(pt_entry);
		i --;
		if (pt_entry->file_entry->filetype == type)
			break;
	}

#if 0
	if (pt_entry->prev->prev != NULL)
	{
		pt_entry = pt_entry->prev;
		Net_Add_UpnpCurIndex(-1);
		Set_Upnp_CurEntry(pt_entry);
	}
	else
	{
		pt_entry = Net_Get_UpnpTreeFirstEntry();
		while ((pt_entry->next != NULL) && (i < g_psNet_FileBrowser->dwNumberOfFile))
		{
			pt_entry = pt_entry->next;
			i ++;
		}
		Set_Upnp_CurEntry(pt_entry);
		if (i == 0)
			Net_Set_UpnpCurIndex(0);
		else
	 		Net_Set_UpnpCurIndex(i - 1);
	}
#endif
}

void Net_Upnp_SearchNextEntry(unsigned char type)
{
	netfs_meta_entry_t *pt_entry;
	netfs_meta_entry_t *tree_entry;
	DWORD cur_index;
	DWORD total_count, i;

	pt_entry = Net_Get_UpnpCurEntry();
	cur_index = Net_Get_UpnpCurIndex();
	total_count = NetGetTotalFile();

#if 0
	if ((pt_entry->next != NULL) && (cur_index < g_psNet_FileBrowser->dwNumberOfFile))
	{
		pt_entry = pt_entry->next;
		Net_Add_UpnpCurIndex(1);
		Set_Upnp_CurEntry(pt_entry);
	}
	else
	{
		pt_entry = Net_Get_UpnpTreeFirstEntry();
		pt_entry = pt_entry->next;
		Set_Upnp_CurEntry(pt_entry);
		Net_Set_UpnpCurIndex(0);
	}
#else
	if ((type == IMAGE_FILE_TYPE) || (type == AUDIO_FILE_TYPE))
		i = total_count;
	else
		i = 1;

	do
	{
		if (cur_index < NetGetTotalFile() - 1)
		{
			if (pt_entry->next == NULL)
			{

				tree_entry = Net_Get_UpnpTreeFirstEntry();
				tree_entry = tree_entry->child;
				Upnp_Browse_dir(tree_entry->containerid,Net_Get_UpnpCurIndex() + 1,tree_entry);
			}
			pt_entry = pt_entry->next;
			Net_Add_UpnpCurIndex(1);
			Set_Upnp_CurEntry(pt_entry);
		}
		else
		{
			pt_entry = Net_Get_UpnpTreeFirstEntry();
			pt_entry = pt_entry->next;
			Set_Upnp_CurEntry(pt_entry);
			Net_Set_UpnpCurIndex(0);
		}
		cur_index = Net_Get_UpnpCurIndex();
		i --;
	}while((i > 0) && (pt_entry->file_entry->filetype != type));
#endif
}
#endif

#ifdef NET_NAPI
int NetIsFileEntryReady(DWORD dwIndex)
{
	if(dwIndex >= g_psNet_FileBrowser->dwNumberOfFile)
		return 0;

	return g_psNet_FileBrowser->FileEntry[dwIndex].state ? true : false;
}
#endif

STREAM * __Net_FileOpen(DWORD net_recv_type, enum _NETFS_TYPE netfs_type, STREAM *sHandle,ST_NET_FILEENTRY *pNetFileEntry)
{
	MP_DEBUG("%s \n", __func__);
#if Make_CURL
#if MAKE_XPG_PLAYER
	NetListProcess(0, 0, pNetFileEntry->Name, 0);
#endif

#ifndef NET_NAPI

	Xml_BUFF_init(net_recv_type);
       int size = Net_Recv_Data(pNetFileEntry->Link, netfs_type, 0, 0);

       if (size < 0)
       {
             FileClose(sHandle);
             Xml_BUFF_free(net_recv_type);
             return NULL;
        }

        pNetFileEntry->size = size;
#else
       MP_ASSERT(pNetFileEntry->size > 0);
#endif

	sHandle->Chain.Size = pNetFileEntry->size;
	sHandle->Chain.Start = 0;
	sHandle->Chain.Point = 0;
#endif	
	return sHandle;
}

BOOL boNet_GeneralFileOpen()
{
	switch(g_bXpgStatus)
	{
		case XPG_MODE_NET_PICASA:
		case XPG_MODE_NET_FLICKR:
		case XPG_MODE_RSS_TITLE:
		case XPG_MODE_NET_PC_PHOTO:
#if YOUGOTPHOTO
		case XPG_MODE_NET_YOUGOTPHOTO:
#endif
#if HAVE_FRAMECHANNEL
		case XPG_MODE_FRAMECHANNEL:
#endif
#if HAVE_FRAMEIT
		case XPG_MODE_FRAMEIT:
#endif
#if HAVE_SNAPFISH
  		case XPG_MODE_SNAPFISH:
#endif
#if HAVE_SHUTTERFLY
		case XPG_MODE_SHUTTERFLY:
#endif
			return TRUE;
           		//return __Net_FileOpen(NET_RECVBINARYDATA, NETFS_SHUTTERFLY, sHandle, pNetFileEntry);
	}
	return FALSE;
	//return NULL;
}

#if MAKE_XPG_PLAYER
STREAM *Net_FileListOpen(DRIVE * sDrv, ST_SEARCH_INFO * pSearchInfo)
{
    STREAM *sHandle;
#if Make_CURL
    int ret;
    unsigned char retrycount = 0;

    if (sDrv->Flag.Present == 0)
   	    sDrv->Flag.Present = 1;

    if (!(sHandle = GetFreeFileHandle(sDrv)))
        return NULL;

#if USBOTG_HOST_PTP
    if ((sDrv->DevID == DEV_USB_HOST_PTP) || (sDrv->DevID == DEV_USBOTG1_HOST_PTP)) //(sDrv->DevID == DEV_USB_HOST_PTP)
    {
        sHandle->DirSector      = pSearchInfo->dwPtpObjectHandle; // use DirSector to store the value of object handler
        sHandle->Chain.Size     = pSearchInfo->dwFileSize;
        sHandle->Chain.Start    = 0;
        sHandle->Chain.Point    = 0; // for PTP, it's index
        return sHandle;
    }
#endif

    sHandle->Flag.SizeChanged = 0;

    //FileDirCaching(sHandle);	//No need to Caching for netfile.

// CHAIN *dir = (CHAIN *) ((BYTE *) sDrv->DirStackBuffer + sDrv->DirStackPoint);

#if NETWARE_ENABLE
	MP_DEBUG("Net_FileListOpen: --> status = %d, Dev = %d", g_bXpgStatus, sDrv->DevID);

    if (! pSearchInfo)
#if ( DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE )
		sDrv->DevID = DEV_CF_ETHERNET_DEVICE;
#else
		sDrv->DevID = DEV_USB_WIFI_DEVICE;
#endif

    if (sDrv->DevID == DEV_USB_WIFI_DEVICE || sDrv->DevID == DEV_CF_ETHERNET_DEVICE )
    {
        ST_NET_FILEENTRY *pNetFileEntry;
  #if NET_UPNP
        netfs_meta_entry_t *pNetUpnpFileEntry;
  #endif
        //EnableNetWareTask();
        if (pSearchInfo == NULL || (g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST) )
        {
  #if NET_UPNP
			u8 len;
            if (g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST)
            {
                pNetUpnpFileEntry = Net_Get_UpnpCurEntry();
                sHandle->DirSector = Net_Get_UpnpCurIndex();
                mpDebugPrint("Cur Index = %d", Net_Get_UpnpCurIndex());
                mpDebugPrint("Current file name = %s", pNetUpnpFileEntry->name);
                mpDebugPrint("sHandle = 0#%08x", sHandle);
				len = strlen(pNetUpnpFileEntry->file_entry->url);
				strcpy(pSearchInfo->bExt,&pNetUpnpFileEntry->file_entry->url[len-3]);
                mpDebugPrint("pSearchInfo->bExt %s",pSearchInfo->bExt);
            }
            else
  #endif
            {
                pSearchInfo = NetGetFileSearchInfo(FileBrowserGetCurIndex());
                pNetFileEntry = NetGetFileEntry(FileBrowserGetCurIndex());
                sHandle->DirSector = pNetFileEntry->dwIndex; // use DirSector to store the value of object handler
            }

	     if (boNet_GeneralFileOpen())
		 {
			sHandle = __Net_FileOpen(NET_RECVBINARYDATA, 0, sHandle, pNetFileEntry);
			pSearchInfo->dwFileSize = pNetFileEntry->size;
			return sHandle;
		 }
            else if (g_bXpgStatus == XPG_MODE_NET_PICASA)
            {
    #if MAKE_XPG_PLAYER
                NetListProcess(0, 0, pNetFileEntry->Name, 0);
	#endif

    #if Write_To_NAND
                if (pNetFileEntry->state != 0 && File_Serial_Index[pNetFileEntry->state - 1] == NetGetFileIndex())
                {
                }
                else
    #endif
                {
    #ifndef NET_NAPI
                    Xml_BUFF_init(NET_RECVBINARYDATA);
                    ret = Net_Recv_Data(pNetFileEntry->Link, NETFS_PICASA, 0, 0);
                    //MP_DEBUG1("PICASA File size = %d", ret);
                    if (ret < 0)
                    {
                        FileClose(sHandle);
                        Xml_BUFF_free(NET_RECVBINARYDATA);
                        return NULL;
                    }

                    pNetFileEntry->size = ret;
                    pSearchInfo->dwFileSize = ret;
    #else
                    MP_ASSERT(pNetFileEntry->size > 0);
    #endif
                    sHandle->Chain.Size = pNetFileEntry->size;
    #if Write_To_NAND
                    {
                        BYTE name[32];
                        //unsigned long time = 0, elapsetime = 0;;

                        memset(name, '\0', 32);
                        sprintf(name, "%d.jpg", File_Serial);
                        pNetFileEntry->state = File_Serial;
                        File_Serial_Index[File_Serial - 1] = NetGetFileIndex();
                        MP_DEBUG("=============Save %s=============", name);
                        //time = SystemGetTimeStamp();
                        Save_Jpeg_Files(name, ret, App_State.qtr);
                        //elapsetime = SystemGetElapsedTime(time);
                        //if (pNetFileEntry->size/elapsetime > max_times)
                        //	max_times = pNetFileEntry->size/elapsetime;
                        //if (pNetFileEntry->size/elapsetime < min_times)
                        //	min_times = pNetFileEntry->size/elapsetime;
						//if (!avg_times)
                        //{
                        //	time_value = pNetFileEntry->size/elapsetime;
                        //	avg_times = 1;
                        //}
                        //else
                        //{
                        //	time_value += pNetFileEntry->size/elapsetime;
                        //	avg_times++;
                        //}
                        //mpDebugPrint("max = %d==avg = %d ====min = %d", max_times, time_value/avg_times, min_times);
                        File_Serial++;
                        if (File_Serial > MAX_SERIAL)
                            File_Serial = 1;
                    }
    #endif
                }
            }
  #if NET_UPNP
            else if (g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST || g_bXpgStatus ==XPG_MODE_DLNA_1_5_DMR)
            {
				/*Only Get file size here*/
				char *tmpptr;
                //Xml_BUFF_init(NET_RECVUPNP);
				gcurl_filesize = 0;
				if( bexit_upupctrl )	//Has quit UPNP mode
				{
					mpDebugPrint("Has quit UPNP mode.");
                    FileClose(sHandle);
					return NULL;			
				}
				if( g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
					gcurrent_url = pNetUpnpFileEntry->file_entry->url;
				else
					gcurrent_url = gavtransuri;
				tmpptr = (char *) ixml_mem_malloc(8192);
				if( tmpptr )
				{
					ret = Get_Image_File_Range(gcurrent_url, tmpptr,0,8192,TRUE,10);
					if( ret > 0 )
					{
						MP_DEBUG("UPNP File size = %d",gcurl_filesize);
						sHandle->Chain.Size = gcurl_filesize;
					}
					ixml_mem_free(tmpptr);
				}
				if( gcurl_filesize <= 0 )
				{
                    FileClose(sHandle);
                    return NULL;
				}
            }
  #endif
            else //other XPG_MODE_NET_XXX
            {
                sHandle->Chain.Size = pNetFileEntry->size;
            }

            sHandle->Chain.Start = 0;
            sHandle->Chain.Point = 0;
            //DisableNetWareTask();
        }
        else
        {
            sHandle->DirSector = ((ST_NET_FILEENTRY *)pSearchInfo)->dwIndex; // use DirSector to store the value of object handler
            sHandle->Chain.Size = ((ST_NET_FILEENTRY *)pSearchInfo)->size;
            sHandle->Chain.Start = 0;
            sHandle->Chain.Point = 0;
        }

        return sHandle;
    }
#endif
#else
 return sHandle;
#endif
}
#endif


