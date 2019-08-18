#define LOCAL_DEBUG_ENABLE 0
#include "global612.h"
#include "mpTrace.h"
//#include "interro.h"
#include "taskid.h"
#include "socket.h"
#include <linux/if.h>		/* for "if_req" */
#include "net_socket.h"
//#include "lwip_incl.h"
#include "netware.h"
#if Make_ADHOC
#include "wlan_adhoc.h"
#endif
//InterRadioStart 

static S32 TestServerSocketCreate()
{
    struct sockaddr_in *localAddr;
    struct ifreq ifr;

    int ftpStatus = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int ret;
    
    if(ftpStatus <= 0)
    {
        DPrintf("[FTP] server socket create fail err=%d", ftpStatus);
        return 0;
    }
    
    MP_DEBUG1("[FTP] server socket id = %d", ftpStatus);
    
    if ((ret = ioctlsocket(ftpStatus, SIOCGIFADDR, (unsigned long *)&ifr)) != 0)
    {
        DPrintf("[FTP] ioctlsocket return err=%d", ret);
        return 0;
    }

    localAddr =  (struct sockaddr_in *)&ifr.ifr_addr;
    localAddr->sin_port = 5001;//serverRec.port;

    ret = bind(ftpStatus, (struct sockaddr *)localAddr, sizeof(*localAddr));
    MP_DEBUG4("[FTP] bind(%d,0x%x,%d) returns = %d", ftpStatus, localAddr->sin_addr.s_addr,localAddr->sin_port, ret);

    ret = listen(ftpStatus, 0);
    MP_DEBUG2("[FTP] listen(%d) returns = %d", ftpStatus, ret);
    
    return ftpStatus;
}

void Iperf_ClientStart(void)
{
    int ret;
	int count = 0;
	//char request[4096];
	char *request = NULL;
	DWORD dwEvent, dwNextEvent;
	DWORD testda = 0xC0A8026C;//0xAC1F647e;172.31.100.115
	struct in_addr in;
	in.s_addr = testda;
	MP_DEBUG("[IPERF] client connects to %s", inet_ntoa(in));
	request = ext_mem_malloc(4096);
	memset(request,0,4096);
	ret =  mpx_DoConnect(testda, 5001, TRUE); /* use blocking socket */
    	if( ret > 0 )
	{
		while( 1 )
		{
			if(count > 5000 )
				break;
			send( ret, request, 4096, 0);
			count++;
		}
	}
	closesocket(ret);
	if(request)
		ext_mem_free(request);
	//while(1)
	//	TaskYield();
}

void Iperf_ServerStart(void)
{
    	int ret;
	//172.31.100.115
	int count = 0;
	//char recvbuf[4096];
	char *recvbuf = NULL;

	int sock;
    	ST_SOCK_SET stReadSet, stWriteSet;
    	ST_SOCK_SET *wfds, *rfds;
	int serverstatus;
    	struct sockaddr_in peerAddr;
	DWORD dwEvent, dwNextEvent;
	recvbuf = ext_mem_malloc(4096);
	memset(recvbuf,0,4096);

	mpDebugPrint("TestServerSocketCreate");
	sock = TestServerSocketCreate();
	mpDebugPrint("TestServerSocketCreate End sock %x",sock);
    	MPX_FD_ZERO(&stReadSet);
    	MPX_FD_ZERO(&stWriteSet);
    	wfds = rfds = NULL;

    	MPX_FD_SET(sock, &stReadSet);
    	rfds = &stReadSet;

    	MPX_FD_SET(sock, &stWriteSet);
    	wfds = &stWriteSet;
	
    	MP_DEBUG("FtpTask: select(0,rfds,wfds,0,0)");
    	serverstatus = select(0, rfds, wfds, 0, NULL);
	mpDebugPrint("select end serverstatus %x",serverstatus);
    	serverstatus = accept(sock, (struct sockaddr *)&peerAddr, NULL);
    	MP_DEBUG2("FtpTask: accept(%d) returns %d", sock,serverstatus);
	while(1)
	{
		//if( MPX_FD_ISSET(sock, &stReadSet) )
		{
			ret = recv(serverstatus, recvbuf, 4096, 0);
			//if( ret > 0 )
				//MP_DEBUG2("FtpTask: recv(%d) returns %d", serverstatus,ret);
			if(ret <= 0)
			{
				mpDebugPrint("socket close");
				closesocket(serverstatus);
				break;
			}
		}
		//UartOutText("A");
	}
    if(recvbuf)
		ext_mem_free(recvbuf);

/*
	memset(request,0,4096);
	ret =  mpx_DoConnect(0xAC1F6473, 5001, TRUE); 
    if( ret > 0 )
	{
		while( 1 )
		{
			if(count > 5000 )
				break;
			send( ret, request, 4096, 0);
			count++;
		}
	}
	closesocket(ret);
*/
	//while(1)
		//TaskYield();
}
#if Make_ADHOC

#define SEND_TIMEOUT_COUNT 3
#define ADHOC_RECEIVE_SIZE 8192

DWORD recv_image_size =0;
extern DWORD decode_time;
static S32 AdhocServerSocketCreate()
{
    struct sockaddr_in *localAddr;
    struct ifreq ifr;

    int ftpStatus = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int ret;
    
    if(ftpStatus <= 0)
    {
        DPrintf("[ADHOC] server socket create fail err=%d", ftpStatus);
        return 0;
    }

    MP_DEBUG1("[ADHOC] server socket id = %d", ftpStatus);
    
    if ((ret = ioctlsocket(ftpStatus, SIOCGIFADDR, (unsigned long *)&ifr)) != 0)
    {
        DPrintf("[FTP] ioctlsocket return err=%d", ret);
        return 0;
    }

    localAddr =  (struct sockaddr_in *)&ifr.ifr_addr;
    localAddr->sin_port = 168;//serverRec.port;

    ret = bind(ftpStatus, (struct sockaddr *)localAddr, sizeof(*localAddr));
    mpDebugPrint("[ADHOC] bind(%d,0x%x,%d) returns = %d", ftpStatus, localAddr->sin_addr.s_addr,localAddr->sin_port, ret);

    ret = listen(ftpStatus, 0);
    mpDebugPrint("[ADHOC] listen(%d) returns = %d", ftpStatus, ret);
    
    return ftpStatus;
}

void Adhoc_ServerStart(void)
{
    int ret = 0;
	int sock;
	ST_SOCK_SET stReadSet, stWriteSet;
	ST_SOCK_SET *wfds, *rfds;
	int serverstatus,i=0;
	struct sockaddr_in peerAddr;
	DWORD dwEvent, dwNextEvent;
	BYTE *image_buf = NULL;
    DWORD image_size = 0;
	DWORD idx=0,idx_end=0;
	char respond[ADHOC_SEND_SIZE_LENGTH];
	unsigned long non_block = 1;
	DWORD recv_time = 0,cur_time = 0;
	DWORD time0=0,time1=0;
	int send_len = 0;
	
	mpDebugPrint("Wait NETWORK_STREAM_EVENT!!");
	EventWait(NETWORK_STREAM_EVENT, 0x0000000f, OS_EVENT_OR, &dwEvent);
	mpDebugPrint("AdhocServerSocketCreate");
	sock = AdhocServerSocketCreate();
	mpDebugPrint("AdhocServerSocketCreate End sock %x",sock);
wait_connect:
	MPX_FD_ZERO(&stReadSet);
	MPX_FD_ZERO(&stWriteSet);
	wfds = rfds = NULL;

	MPX_FD_SET(sock, &stReadSet);
	rfds = &stReadSet;

	MPX_FD_SET(sock, &stWriteSet);
	wfds = &stWriteSet;
	//mpDebugPrint("Adhoc_ServerStart: select(0,rfds,wfds,0,0)");
	serverstatus = select(0, rfds, wfds, 0, NULL);
	//mpDebugPrint("select end serverstatus %x",serverstatus);
	serverstatus = accept(sock, (struct sockaddr *)&peerAddr, NULL);
	//mpDebugPrint("Adhoc_ServerStart: accept(%d) returns %d", sock,serverstatus);
	mpDebugPrint(" TEST start time %x",GetSysTime());
    ioctlsocket(serverstatus, FIONBIO, &non_block);

	if( MPX_FD_ISSET(sock, &stReadSet) )
    {
        recv_time = GetSysTime();
		while(1)
		{
            //memset(respond,0x00,ADHOC_SEND_SIZE_LENGTH);
			ret = recv(serverstatus,respond, 4, 0);
			if(ret > 0)
			{

			 	image_size = (respond[0]&0xFF)<<24|(respond[1]&0xFF)<<16|(respond[2]&0xFF)<<8|respond[3]&0xFF;
			 	//mpDebugPrint("image_size %d ",image_size);
				(void*)image_buf = ext_mem_malloc(ADHOC_BUF_SIZE);
				//mpDebugPrint("image_buf %x ",image_buf);
				while(1)
				{
					ret = recv(serverstatus,image_buf+recv_image_size, ADHOC_RECEIVE_SIZE, 0);
	                if(ret > 0)
	                {
						recv_image_size+=ret;
						recv_time = GetSysTime();
						//mpDebugPrint("image_size %d ",image_size);
						//mpDebugPrint("%ret %d recv_image_size %d",ret,recv_image_size);
				
						if(recv_image_size==image_size)
						{
							//mpDebugPrint("recv_image_size %d image_size %d ",recv_image_size,image_size);
							recv_image_size = 0;
							//NetPacketDump(image_buf, 32);
							Adhoc_Put_Decode_List(image_buf,image_size);
							i=0;
						    do{
								send_len = send(serverstatus, respond, ADHOC_SEND_SIZE_LENGTH, 0);
								i++;
								if(i >= SEND_TIMEOUT_COUNT)
									mpDebugPrint("SEND_TIMEOUT_COUNT");
								TaskYield();
						    }while((send_len<0)&&(i<SEND_TIMEOUT_COUNT));
							
							if(i<SEND_TIMEOUT_COUNT)
							{
								EventSet(WPA_EVENT, BIT4);
								TaskYield();
							}
							TaskYield();
				  			break;
						}
						//UartOutText("1");
						TaskYield();
	              	}
					else
					{
						cur_time = GetSysTime();
						
						time0 = max(cur_time, recv_time);
						time1 = min(cur_time, recv_time);

					 	if((time0-time1) > (ADHOC_TCP_TIMEOUT/2))
					 	{
					 		mpDebugPrint("(time0-time1) %d",(time0-time1));
							recv_time = GetSysTime();
							recv_image_size = 0;
							ext_mem_free(image_buf);
							TaskYield();
							break;
					 	}
						//UartOutText("2");
						TaskYield();
					}
					
					TaskYield();
				}
			}
			else
			{
			   cur_time = GetSysTime();
				time0 = max(cur_time, recv_time);
				time1 = min(cur_time, recv_time);

			 	if((time0-time1) > (ADHOC_TCP_TIMEOUT/2))
			 	{
			 		mpDebugPrint("1(time0-time1) %d",(time0-time1));
					recv_time = GetSysTime();
					closesocket(serverstatus);
					TaskYield();
					goto wait_connect;
			 	}
			}
			TaskYield();
	   	}
		//TaskYield();
	}
		
}

#endif

#define READ_BUFFER_SIZE (1024*1024)
int p2p_socket =0;
static S32 ISocketCreate(void)
{
    struct sockaddr_in *localAddr;
    struct ifreq ifr;

    int Status = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int ret;
    
    if(Status <= 0)
    {
        DPrintf("[ISocketCreate] server socket create fail err=%d", Status);
        return 0;
    }

    MP_DEBUG1("[ISocketCreate] server socket id = %d", Status);
    
    if ((ret = ioctlsocket(Status, SIOCGIFADDR, (unsigned long *)&ifr)) != 0)
    {
        DPrintf("[ISocketCreate] ioctlsocket return err=%d", ret);
        return 0;
    }

    localAddr =  (struct sockaddr_in *)&ifr.ifr_addr;
    localAddr->sin_port = 80;//serverRec.port;

    ret = bind(Status, (struct sockaddr *)localAddr, sizeof(*localAddr));
    mpDebugPrint("[ISocketCreate] bind(%d,0x%x,%d) returns = %d", Status, localAddr->sin_addr.s_addr,localAddr->sin_port, ret);

    ret = listen(Status, 0);
    mpDebugPrint("[ISocketCreate] listen(%d) returns = %d", Status, ret);
    
    return Status;
}

int IGet_SDFile(void)
{
	//int sock;
	DWORD file_size=0,file_read_size = 0,file_get=0,file_get_total=0;
	BYTE  *read_buf;
	ST_SOCK_SET stReadSet, stWriteSet;
	ST_SOCK_SET *wfds, *rfds;
	int serverstatus = 0;
	struct sockaddr_in peerAddr;
	BYTE request_buf[8];
	BYTE Send_video_len;
	int recv_len = 0;
	BYTE first_time;
	BYTE file_ext = 1;//mp4
	int send_len = 0,send_l = 0;
	DRIVE *pCurDrive;
	BYTE file_num[32];
	BYTE file_name[32];
	BYTE file_idx = 0;
	STREAM *handle;

RESENT_VIDEO:	
	Send_video_len = 0;
	first_time = 0;
	if(p2p_socket==0)
	{
		mpDebugPrint("IGet_SDFile");
		p2p_socket = ISocketCreate();
		mpDebugPrint("ISocketCreate End sock %x",p2p_socket);
	}
	MPX_FD_ZERO(&stReadSet);
	MPX_FD_ZERO(&stWriteSet);
	wfds = rfds = NULL;

	MPX_FD_SET(p2p_socket, &stReadSet);
	rfds = &stReadSet;

	MPX_FD_SET(p2p_socket, &stWriteSet);
	wfds = &stWriteSet;
	mpDebugPrint("IGet_SDFile: select(0,rfds,wfds,0,0)");
	serverstatus = select(0, rfds, wfds, 0, NULL);
	mpDebugPrint("select end serverstatus %x",serverstatus);
	serverstatus = accept(p2p_socket, (struct sockaddr *)&peerAddr, NULL);
	mpDebugPrint("IGet_SDFile: accept(%d) returns %d", p2p_socket,serverstatus);

	read_buf = ext_mem_malloc(READ_BUFFER_SIZE);
#if	1
	pCurDrive=DriveGet(SD_MMC);
RESEARCH:
	DecString(file_num,file_idx,3,0);
	//mpDebugPrint("file_num %s",file_num);
	snprintf(file_name,32,"VIDEO%s",file_num);
	mpDebugPrint("file_name %s",file_name);
	(int)handle = FileSearch(pCurDrive,file_name, "mp4", E_FILE_TYPE);
    /*
	DRIVE *pCurDrive = DriveChange(SD_MMC_PART1);
	if (DirReset(pCurDrive) != FS_SUCCEED)
	{   
		DriveChange(CF_ETHERNET_DEVICE);
		return;
	}*/

	if (handle==NULL){
		handle = FileOpen(pCurDrive);
	}
	else{
		
		file_idx = 0;
		goto RESEARCH;
	}

#else
		handle = FileExtSearchOpen(SD_MMC_PART1, "MP4", 32);

    if(handle==NULL)
    {
		handle = FileExtSearchOpen(SD_MMC_PART1, "264", 32);
		file_ext = 2;
    }
#endif	

	if(handle!=NULL)
	{
	    //mpDebugPrint("FOUND MP4");
	    file_size = FileSizeGet(handle);
		mpDebugPrint("file_size %d",file_size);
		request_buf[0] = (file_size&0xFF000000)>>24;
		request_buf[1] = (file_size&0x00FF0000)>>16;
		request_buf[2] = (file_size&0x0000FF00)>>8;
		request_buf[3] = (file_size&0x000000FF);
		request_buf[4] = file_ext;
		NetPacketDump(request_buf, 8);
		do
		{
		    if(first_time==0)
		    {
		    	file_get = MIN(file_size,READ_BUFFER_SIZE);
				first_time++;
		    }
			else{
					switch(first_time)
					{
					   case 1:
					   	  file_get = (READ_BUFFER_SIZE/2);
						  first_time++;
						  break;
  					   case 2:
					   	  file_get = (READ_BUFFER_SIZE/4);
						  first_time++;
						  break;
					   case 3:
					   	  file_get = (READ_BUFFER_SIZE/8);
						  first_time++;
						  break;
					   case 4:
					   	  file_get = (READ_BUFFER_SIZE/16);
						  first_time++;
						  break;
					   case 5:
					   	  file_get = (READ_BUFFER_SIZE/32);
						  first_time++;
						  break;
					   case 6:
					   	  file_get = (READ_BUFFER_SIZE/64);
						  first_time++;
						  break;
					   case 7:
					   	  file_get = MIN(file_size,READ_BUFFER_SIZE/128);
						  //first_time++;
						  break;

					}
		    }

			//mpDebugPrint("file_get %d",file_get);
	    	file_read_size=FileRead(handle,read_buf,file_get);
			//mpDebugPrint("file_read_size %d",file_read_size);
			file_size-=file_read_size;
			file_get_total+=file_read_size;
			//mpDebugPrint("file_size %d",file_size);
			//mpDebugPrint("file_get_total %d",file_get_total);
			if(!Send_video_len)
			{
				send(serverstatus,request_buf,8,0);
				Send_video_len = 1;
			}
			//UartOutText(" T ");
			send(serverstatus,read_buf,file_get,0);
			if(file_size)
			{
				//UartOutText(" R ");
				recv_len = recv(serverstatus,request_buf,4,0);
				mpDebugPrint("recv_len %d",recv_len);
			}
			TaskYield();
			//TaskSleep(100);
		}while(file_size);
		FileClose(handle);
		ext_mem_free(read_buf);
	}
    closesocket(serverstatus);
	//DriveChange(CF_ETHERNET_DEVICE);
	file_idx++;
goto RESENT_VIDEO;
}

#if SHOW_FRAME_TEST
#include "list_mpx.h"
#define miniDVWiFi_SEND_SIZE  		8192
#define miniDVWiFi_PHOTO_COUNT 	5
extern struct net_device   *netdev_global;
static BYTE ethaddr[6];
LM_LIST_CONTAINER miniDVWiFiList;

void miniDV_test_WiFi_task(void);
static BYTE u08miniDVtestWiFi_MessageId;
static BYTE u08miniDVtestWiFi_TaskId;
static BYTE u08miniDVtestWiFi_sendPhotoId;
int connect_socket = 0;
int miniDVWiFiSemid;
int countingSemid;

struct wifi_photo{
	LM_LIST_ENTRY Link;
	DWORD image_size;
	BYTE *image_buf;
	BYTE  Reserved[7];
};

/* photo queue init */
void
PhotoListInitList(
PLM_LIST_CONTAINER pList) 
{
	pList->Link.FLink = pList->Link.BLink = (PLM_LIST_ENTRY) 0;
	pList->EntryCount = 0;
} /* ListInitList */

/******************************************************************************/
/* ListPopHead -- pops the head off the list.                                 */
/******************************************************************************/
PLM_LIST_ENTRY 
PhotoListPopHead(
PLM_LIST_CONTAINER pList) 
{
    	PLM_LIST_ENTRY pEntry;
    
    	SemaphoreWait(miniDVWiFiSemid);
    
    	pEntry = pList->Link.FLink;

    	if(pEntry) 
    	{
        	pList->Link.FLink = pEntry->FLink;
        	if(pList->Link.FLink == (PLM_LIST_ENTRY) 0) 
        	{
            		pList->Link.BLink = (PLM_LIST_ENTRY) 0;
      		}
		else
		{
      			pList->Link.FLink->BLink = (PLM_LIST_ENTRY) 0;
		}

      		pList->EntryCount--;
    	} /* if(pEntry) */
    		
	SemaphoreRelease(miniDVWiFiSemid);
	
	return pEntry;
} /* ListPopHead */

/******************************************************************************/
/* ListPushTail -- puts an element at the tail of the list.                   */
/******************************************************************************/
void
PhotoListPushTail(
PLM_LIST_CONTAINER pList, 
PLM_LIST_ENTRY pEntry) 
{
	SemaphoreWait(miniDVWiFiSemid);

	pEntry->BLink = pList->Link.BLink;

	if(pList->Link.BLink) 
	{
		pList->Link.BLink->FLink = pEntry;
	} 
	else 
	{
		pList->Link.FLink = pEntry;
	}

	pList->Link.BLink = pEntry;
	pEntry->FLink = (PLM_LIST_ENTRY) 0;

	pList->EntryCount++;

	SemaphoreRelease(miniDVWiFiSemid);
} /* ListPushTail */

int miniDVWiFi_ClientSend(int sid,DWORD size,BYTE *buf)
{
	char request[ADHOC_SEND_SIZE_LENGTH];
	int ret = 0,recv_len=0;
	DWORD send_len = 0,buf_idx = 0;
	DWORD len_send = miniDVWiFi_SEND_SIZE;

	MP_DEBUG("%s", __func__);
	
	request[0] = (size&0xFF000000)>>24;
	request[1] = (size&0x00FF0000)>>16;
	request[2] = (size&0x0000FF00)>>8;
	request[3] = (size&0x000000FF);
	//UartOutText(" 1 ");
	do{
		MP_DEBUG("T : image_size = %u", size);
		ret = send(sid, request, ADHOC_SEND_SIZE_LENGTH, 0);//start and send jpg size
		MP_DEBUG("1ret %d",ret);
		TaskYield();
	}while(ret < 0);
	
	//UartOutText(" 2 ");
	
	send_len = size;
	do{
		len_send = MIN(send_len,miniDVWiFi_SEND_SIZE);
		ret = send(sid, buf+buf_idx, len_send,0);		

		if(ret >= 0)
		{
			send_len-=ret;
			buf_idx+=ret;
			MP_DEBUG("3ret %d\r\nsend_len %d", ret, send_len);
		}
		TaskYield();
	}while(send_len);
	
	//UartOutText(" 3 ");
	return ret;	
}

/*
 * Pop photo entry from the queue, then to call send function
 */
void miniDVWiFi_sendPhoto(void)
{
	struct wifi_photo *wp_get=NULL;
	int i = 0,ret = 0;
	BYTE *tmp_image_buf = NULL;
	DWORD tmp_image_size = 0;

	MP_DEBUG("%s", __func__);
	
	while(1)
	{
		mpx_SemaphoreWait(countingSemid);
		wp_get = (struct wifi_photo *)PhotoListPopHead(&miniDVWiFiList);
		MP_DEBUG("[%s] wp_get pointer = %x", __func__, wp_get);
		MP_DEBUG("[%s] buf pointer = %x", __func__, wp_get->image_buf);
		MP_DEBUG("[%s] size = %u", __func__, wp_get->image_size);
		if(!wp_get)
			__asm("break 100");
		
		/* copy pop frame's pointer and size */
		tmp_image_buf = wp_get->image_buf;
		tmp_image_size = wp_get->image_size;

		ext_mem_free(wp_get);
						
		ret = miniDVWiFi_ClientSend(connect_socket, tmp_image_size, tmp_image_buf);

		if(tmp_image_buf)
		{
			ext_mem_free(tmp_image_buf);
			tmp_image_buf = NULL;
			tmp_image_size = 0;
		}
		
		TaskYield();
	}
}

/*
 * Schedule a picture to photo queue for sending
 */
void miniDV_Send_Queue(BYTE *image_buf,DWORD image_size)
{
	MP_DEBUG("%s", __func__);

	struct wifi_photo *wp;
	struct wifi_photo *drop_wp;

	/* to check frame is too old. if it is true, release old frame */
	if(ListGetSize(&miniDVWiFiList) > miniDVWiFi_PHOTO_COUNT)
	{
	//UartOutText(" Q ");
		MP_DEBUG(" %u ", ListGetSize(&miniDVWiFiList));
		mpx_SemaphoreWait(countingSemid);
		drop_wp = (struct wifi_photo *)PhotoListPopHead(&miniDVWiFiList);
		ext_mem_free(drop_wp->image_buf);
		ext_mem_free(drop_wp);
	}

	wp = ext_mem_malloc(sizeof(struct wifi_photo));
	if(wp == NULL)
	{
		mpDebugPrint("[%s] alloc ext memory failed", __func__);
		__asm("break 100");
	}

	wp->image_size = image_size;

	wp->image_buf = ext_mem_malloc(wp->image_size);
	if(wp->image_buf == NULL)
	{
		mpDebugPrint("[%s] alloc ext memory failed", __func__);
		__asm("break 100");
	}
	
	MP_DEBUG("wp ptr = %x ; buf ptr = %x ; size = %u", wp, wp->image_buf, wp->image_size);

	mmcp_memcpy(wp->image_buf, image_buf, wp->image_size);

	PhotoListPushTail(&miniDVWiFiList,&wp->Link);

	SemaphoreRelease(countingSemid);

	TaskYield();
}

/* called in SysytemInit() */
void miniDV_test_wifi_init(void)
{
	int dRet=0;

	mpDebugPrint("%s", __func__);
	
	if(!u08miniDVtestWiFi_MessageId)
	{
		dRet = mpx_MessageCreate(OS_ATTR_FIFO, 640);
		if(dRet < 0)
		{
			mpDebugPrint("[%s] mpx_MessageCreate return %d ; u08miniDVtestWiFi_MessageId mail box create fail!", __func__, dRet);
			__asm("break 100");
		}
		else
			u08miniDVtestWiFi_MessageId = (BYTE)dRet;
	}

	PhotoListInitList(&miniDVWiFiList);

	if(!u08miniDVtestWiFi_sendPhotoId)
	{
		dRet = mpx_TaskCreate(miniDVWiFi_sendPhoto, ISR_PRIORITY+1, 0x4000);
		if(dRet < 0)
		{
			mpDebugPrint("[%s] mpx_TaskCreate return %d ; u08miniDVtestWiFi_sendPhotoId create fail!", __func__, dRet);
			__asm("break 100");
		}
		else
			u08miniDVtestWiFi_sendPhotoId = (BYTE)dRet;		
	}

	miniDVWiFiSemid = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
	MP_ASSERT(miniDVWiFiSemid > 0);

	countingSemid = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 0);
	MP_ASSERT(countingSemid > 0);
	SemaphoreSet(countingSemid, 255);
	
	if(!u08miniDVtestWiFi_TaskId)
	{
		dRet = mpx_TaskCreate(miniDV_test_WiFi_task, CONTROL_PRIORITY, 0x3000);
		if(dRet < 0)
		{
			mpDebugPrint("[%s] mpx_TaskCreate return %d ; u08miniDVtestWiFi_TaskId task create fail!", __func__, dRet);
			__asm("break 100");
		}
		else
			u08miniDVtestWiFi_TaskId = (BYTE)dRet;

		dRet = mpx_TaskStartup(u08miniDVtestWiFi_TaskId);
		if(dRet < 0)
		{
			mpDebugPrint("[%s] mpx_TaskStartup return %d ; miniDV_test_WiFi_task start-up failed", __func__, dRet);
			__asm("break 100");
		}
	}
}

int miniDVWiFi_start_sendTask(void)
{
	/* start-up task for send photo entry to connection socket */
	int dRet = mpx_TaskStartup(u08miniDVtestWiFi_sendPhotoId);
	if(dRet < 0)
	{
		mpDebugPrint("[%s] mpx_TaskStartup return %d ; u08miniDVtestWiFi_sendPhotoId start-up failed", __func__, dRet);
		__asm("break 100");
	}//if(dRet < 0)

	return dRet;
}

extern BYTE auto_scan;
void miniDV_test_WiFi_task(void)
{
	S32 status;
	uint32_t u32Message[8];
	DWORD ipaddress = 0;
	int ret;
	
	mpDebugPrint("%s", __func__);
	
	while(1)
	{	
		status = mpx_MessageReceive(u08miniDVtestWiFi_MessageId, (U08*)u32Message);
        	if(status > 0)
		{
			switch(u32Message[0])
			{
				case 2:	//scan and create MPX_ADHOC dummy
					mpDebugPrint("[%s] to call NetScanRequestEventSet", __func__);
					NetScanRequestEventSet();
					break;
				case 3:	//auto connect Ad Hoc network
					mpDebugPrint("[%s] Auto start to connect MPX_ADHOC\n", __func__);	
					WirelessNetworkSetupITEM.Mode = WIFI_MODE_IBSS;
					NetInterfaceEventSet(); /* notify network subsystem */				
					break;
				case 4:	//create socket to connect the receiver
					mpDebugPrint("[%s] to call miniDV_create_socket", __func__);
					ret=miniDV_create_socket();
					//Ui_TimerProcAdd(500, miniDVWiFi_start_sendTask);
					if(ret)
					miniDVWiFi_start_sendTask();
					break;
			}	//switch(u32Message[0])
		}	//if(status > 0)
		TaskYield();
	}	//while(1)
}
void sentTo_testWiFi_task(BYTE index)
{
	uint32_t message[2];
	int ret_val=0;
	
	mpDebugPrint("%s ; index = %d", __func__, index);

	message[0] = index;

	ret_val = mpx_MessageSend(u08miniDVtestWiFi_MessageId, (BYTE *)message, sizeof(message));
	if(ret_val < 0)
	{
		mpDebugPrint("[%s] send message failed!", __func__);
		__asm("break 100");
	}
}
int miniDV_create_socket(void)
{
	DWORD destIP = 0xC0A80164;	//It is the receiver's IP address. 192.168.1.100
retry:
	/* To connect the receiver */
	connect_socket =  mpx_DoConnect(destIP, 168, 1); /* use blocking socket */
	if(connect_socket > 0)
	{
		mpDebugPrint("[%s] mpx_DoConnect OK! Socket_id = %d", __func__, connect_socket);
		//miniDV_send_JPG();	//mark for test record task
		
		return 1;
	}
	else
	{
		mpDebugPrint("[%s] mpx_DoConnect Failed!", __func__);
		//__asm("break 100");
		goto retry;
	}
}
int miniDV_send_JPG(void)
{
	DWORD dwStartTime, dwReadFileTime;

	static DRIVE *sDrv;
	STREAM *handle = NULL;
	DWORD file_size = 0, read_size = 0;
	BYTE *file_buf = NULL;

	int ret_Val = 0;

	dwStartTime = GetSysTime();
	mpDebugPrint("1 dwStartTime = %x", dwStartTime);
#if 1	/* To get one JPG file lalabear_images1.jpg from SD card */	
	sDrv=DriveGet(SD_MMC);
	(int)handle = FileSearch(sDrv,"lalabear_images1", "jpg", E_FILE_TYPE);
	
	if(handle!=NULL)
		 mpDebugPrint("FileSearch FILE FAIL!!");
	else
	{
		mpDebugPrint("FileSearch FILE OK!!");
		
		handle = FileOpen(sDrv);
		mpDebugPrint("handle %x",handle);
		file_size = FileSizeGet(handle);
		(void*)file_buf = ext_mem_malloc(file_size);
		read_size = FileRead(handle,file_buf, file_size);
		mpDebugPrint("FileRead %d",read_size);
		FileClose(handle);
	}
#else	//to get jpg from sensor
	SetSensorInterfaceMode(3);	// 3 is MODE_PCCAM 
	SetImageSize(0);	//0 is SIZE_176x144; 1 is SIZE_VGA_640x480
	SetSensorOverlayEnable(DISABLE);
	//API_Sensor_Stop();
	//TaskSleep(100);
	API_Sensor_Initial();
	BYTE *SensorInJpegBuffer=(BYTE*)ext_mem_malloc(176*144*2);
	DWORD jpegsize=GetSensorJpegImage(SensorInJpegBuffer);
	if(SensorInJpegBuffer == NULL)
	{
		mpDebugPrint("[%s] cannot allocate memory", __func__);
		BREAK_POINT();
	}
	else
	{
		memset(SensorInJpegBuffer, 0x0, sizeof SensorInJpegBuffer);
		mpDebugPrint("[%s] alloctae memory success\r\nsize = %u", __func__, jpegsize);
	}
#endif

	dwReadFileTime = GetSysTime();
	mpDebugPrint("2 dwReadFileTime = %x", dwReadFileTime);

resend:
#if 1	
	/* To send lalabear_images1.jpg to the receiver */
	if(file_buf)
	{
		mpDebugPrint("[%s] read lalabear_images1.jpg OK. To call Adhoc_Send_PIC", __func__);
		miniDV_Send_Queue(file_buf,read_size);
	}
#else
	if(SensorInJpegBuffer)
	{
		mpDebugPrint("[%s] read lalabear_images1.jpg OK. To call Adhoc_Send_PIC", __func__);
		ret_Val = Adhoc_Send_PIC(SensorInJpegBuffer,jpegsize, connect_socket);
	}
#endif
	
#if 1
	ext_mem_free(file_buf);
#else
	ext_mem_free(SensorInJpegBuffer);
#endif	
}
#endif

