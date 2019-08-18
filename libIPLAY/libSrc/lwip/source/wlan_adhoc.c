#define LOCAL_DEBUG_ENABLE 0

#include <stdio.h>
#include <stdarg.h>
#include <linux/types.h>
#include "global612.h"
#include "typedef.h"
#include "mpTrace.h"
#include "typedef.h"
#include "os_mp52x.h"
#include "os.h"
#include "ndebug.h"

#include "taskid.h"
#include "list_mpx.h"
#include "wlan_adhoc.h"
#include "netware.h"

//ADHOC_TEST_COUNT    0: always run
#define ADHOC_TEST_COUNT  		0 
#define ADHOC_TEST_CYCLE  		200//30 

#define ADHOC_SEND_SIZE  		8192 

static BYTE MPX_ADHOC_SSID[]="MPX_ADHOC";
static int adhoc_channel = 6;
static BYTE adhoc_security = WIFI_NO_SECURITY;

typedef struct 
{
	BYTE mac[6];
} MacAddr_t;

struct adhoc_pic
{
	LM_LIST_ENTRY   Link;
	DWORD image_size;
	BYTE *image_buf;
	BYTE  Reserved[7];
};

extern MacAddr_t	mBssId;
extern struct net_device   *netdev_global;
extern BYTE mBssCnt;
extern BYTE myethaddr[6];
static BYTE ethaddr[6];
DWORD adhoc_create = FALSE;
DWORD adhoc_beacon_lost_cnt =0;
LM_LIST_CONTAINER AdhocList;
LM_LIST_CONTAINER AdhocDecodeList;

int adhoc_socket = 0;
DWORD decode_time = 0;
DWORD adp_idx = 0;
DWORD total_data = 0;
struct adhoc_pic adp[ADHOC_ADP_COUNT];
	
void
AdhocListInitList(
PLM_LIST_CONTAINER pList) 
{
	pList->Link.FLink = pList->Link.BLink = (PLM_LIST_ENTRY) 0;
	pList->EntryCount = 0;
} /* ListInitList */

/******************************************************************************/
/* ListPopHead -- pops the head off the list.                                 */
/******************************************************************************/
PLM_LIST_ENTRY 
AdhocListPopHead(
PLM_LIST_CONTAINER pList) 
{
    PLM_LIST_ENTRY pEntry;
	IntDisable();
    pEntry = pList->Link.FLink;

    if(pEntry) {
        pList->Link.FLink = pEntry->FLink;
        if(pList->Link.FLink == (PLM_LIST_ENTRY) 0) {
            pList->Link.BLink = (PLM_LIST_ENTRY) 0;
        } else {
            pList->Link.FLink->BLink = (PLM_LIST_ENTRY) 0;
        }

        pList->EntryCount--;
    } /* if(pEntry) */
	 IntEnable();

     return pEntry;
} /* ListPopHead */

/******************************************************************************/
/* ListPushTail -- puts an element at the tail of the list.                   */
/******************************************************************************/
void
AdhocListPushTail(
PLM_LIST_CONTAINER pList, 
PLM_LIST_ENTRY pEntry) 
{
    IntDisable();
    pEntry->BLink = pList->Link.BLink;

    if(pList->Link.BLink) {
        pList->Link.BLink->FLink = pEntry;
    } else {
        pList->Link.FLink = pEntry;
		
    }

    pList->Link.BLink = pEntry;
    pEntry->FLink = (PLM_LIST_ENTRY) 0;

    pList->EntryCount++;
	IntEnable();

} /* ListPushTail */
void Adhoc_Put_Decode_List(BYTE *image_buf,DWORD image_size)
{
	//struct adhoc_pic *adp=NULL;
	//mpDebugPrint("Adhoc_Put_Decode_List");
	//(BYTE*)adp = ext_mem_malloc(sizeof(struct adhoc_pic));
	//memset(adp,0x00,sizeof(struct adhoc_pic));
	adp[adp_idx].image_buf = image_buf;
	adp[adp_idx].image_size = image_size;
	AdhocListPushTail(&AdhocDecodeList,&adp[adp_idx].Link);
	
	if(adp_idx<(ADHOC_ADP_COUNT-1))
		adp_idx++;
	else
		adp_idx = 0;
    TaskYield();
}
void Adhoc_Decode()
{
   struct adhoc_pic *adp_get=NULL;
   int i=0;
   for(i=0;i<AdhocDecodeList.EntryCount;i++)
   {
   	  (PLM_LIST_ENTRY)adp_get = AdhocListPopHead(&AdhocDecodeList);
	  
	  //mpDebugPrint("D %x",adp_get->image_buf);
	  
      IntDisable();
	  ImageAdhocDraw_Decode(adp_get->image_buf,adp_get->image_size);
	  IntEnable();

	  ext_mem_free(adp_get->image_buf);
	  if(ADHOC_TEST_COUNT > 0)
	     total_data+=adp_get->image_size;
	  //mpDebugPrint("D %d",decode_time);
	  decode_time++;
      TaskYield();
   }
   if(ADHOC_TEST_COUNT > 0)
   {
	   if(decode_time > ADHOC_TEST_COUNT)
	   	{
	   	  mpDebugPrint(" DECODE time %x",GetSysTime());
		  mpDebugPrint(" avg size %d",total_data/decode_time);

		  decode_time = 0;
	   	}
   }
}

BYTE *Adhoc_Get_Create_BSSID()
{
#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI
  return mBssId.mac;
#else
  return &myethaddr[0];
#endif
}

void Adhoc_Set_SA(BYTE *SA)
{
  memcpy(ethaddr,SA,6);
}
void Adhoc_Set_Beacon_Lost_Count(DWORD cnt)
{
  if(cnt==0)
  	adhoc_beacon_lost_cnt = 0;
  else	
  	adhoc_beacon_lost_cnt += cnt;
}
DWORD Adhoc_Get_Beacon_Lost_Count()
{
  return adhoc_beacon_lost_cnt;
}

int Adhoc_Get_IPAddress(BYTE addr)
{
   int ipaddress = 0;
	switch(addr%10)
	{
	  case 0://192.168.1.100
	  	ipaddress = 0xc0a80164;
	  	break;
	  case 1://192.168.1.101
		  	ipaddress = 0xc0a80165;
	  	break;
	  case 2://192.168.1.102
		  	ipaddress = 0xc0a80166;
	  	break;
	  case 3://192.168.1.103
		  	ipaddress = 0xc0a80167;
	  	break;
	  case 4://192.168.1.104
		  	ipaddress = 0xc0a80168;
	  	break;
	  case 5://192.168.1.105
		  	ipaddress = 0xc0a80169;
	  	break;
	  case 6://192.168.1.106
		  	ipaddress = 0xc0a8016A;
	  	break;
	  case 7://192.168.1.107
		  	ipaddress = 0xc0a8016B;
	  	break;
	  case 8://192.168.1.108
		  	ipaddress = 0xc0a8016C;
	  	break;
	  case 9://192.168.1.109
		  	ipaddress = 0xc0a8016D;
	  	break;
	}
	return ipaddress;

}

int Adhoc_ClientSend(int sid,DWORD size,BYTE *buf)
{
	char request[ADHOC_SEND_SIZE_LENGTH];
	int ret = 0,recv_len=0;
	DWORD send_len = 0,buf_idx = 0,send_time = 0;
	DWORD cur_time = 0;
	DWORD len_send = ADHOC_SEND_SIZE;
	DWORD time0=0,time1=0;
	//mpDebugPrint("S buf %x %d",buf,size);
	//memset(request,0x00,ADHOC_SEND_SIZE_LENGTH);
	request[0] = (size&0xFF000000)>>24;
	request[1] = (size&0x00FF0000)>>16;
	request[2] = (size&0x0000FF00)>>8;
	request[3] = (size&0x000000FF);
	//UartOutText(" 1 ");
	send_time = GetSysTime();
	do{
	     ret = send(sid, request, ADHOC_SEND_SIZE_LENGTH, 0);//start and send jpg size
	     //mpDebugPrint("ret %x",ret);
	     //check send time out
	     cur_time = GetSysTime();
		time0 = max(cur_time, send_time);
		time1 = min(cur_time, send_time);

		 if((time0-time1) > ADHOC_TCP_TIMEOUT)
		 {
		 	mpDebugPrint("(time0-time1) %d",(time0-time1));
			send_time = GetSysTime();
			//SockSignalTxDone(sid);
			//__asm("break 100");
			TaskYield();
			return -2;

		 }
		 
	 	 TaskYield();

		}while(ret < 0);
	//UartOutText(" 2 ");
	//if send one picture need software workaround;
	//buf[0] = 0xff;
	//buf[1] = 0xd8;
	//buf[2] = 0xff;
	//buf[3] = 0xe0;
	//UartOutText(" 3 ");
	send_len = size;
	send_time = GetSysTime();

	do{
		len_send = MIN(send_len,ADHOC_SEND_SIZE);
		ret = send(sid, buf+buf_idx, len_send,0);
		

			if(ret > 0)
			{
				send_len-=ret;
				buf_idx+=ret;
				send_time = GetSysTime();
				//mpDebugPrint("ret %x",ret);

			}
			else
			{
			     cur_time = GetSysTime();
				 time0 = max(cur_time, send_time);
	             time1 = min(cur_time, send_time);
				 if((time0-time1) > ADHOC_TCP_TIMEOUT)
				 {
				 	mpDebugPrint("1(time0-time1) %d",(time0-time1));
					send_time = GetSysTime();
					TaskYield();
					return -1;

				 }
			}
			
		 TaskYield();
		}while(send_len);
	//UartOutText(" 4 ");
	cur_time = GetSysTime();
	do{
		recv_len = recv(sid, buf, ADHOC_SEND_SIZE_LENGTH, 0);
		time0 = max(cur_time, send_time);
	    time1 = min(cur_time, send_time);

		if((ret < 0)&&((time0-time1) > ADHOC_TCP_TIMEOUT))
		{
		 	mpDebugPrint("2(time0-time1) %d",(time0-time1));
			send_time = GetSysTime();
			TaskYield();
			return -3;

		 }
		//mpDebugPrint("recv_len %x",recv_len);
		TaskYield();
	}while(recv_len<0);
	return ret;
	//UartOutText(" 5 ");
}

int Adhoc_Send_Tick()
{
   struct adhoc_pic *adp_get=NULL;
   int i=0,ret = 0;
   for(i=0;i<AdhocList.EntryCount;i++)
   {
      (PLM_LIST_ENTRY)adp_get = AdhocListPopHead(&AdhocList);
      ret = Adhoc_ClientSend(adhoc_socket,adp_get->image_size,adp_get->image_buf);
	  ext_mem_free(adp_get->image_buf);
	  if(ret < 0)
	  	break;
	  TaskYield();
   }
   return ret;

}

/*
 * Schedule a picture for sending
 */
int Adhoc_Send_PIC(BYTE *image_buf,DWORD image_size)
{
   adp[adp_idx].image_buf = image_buf;
   adp[adp_idx].image_size = image_size;
   AdhocListPushTail(&AdhocList,&adp[adp_idx].Link);
   if(adp_idx<(ADHOC_ADP_COUNT-1))
	adp_idx++;
   else
	adp_idx = 0;

   return Adhoc_Send_Tick();
}
BYTE *Adhoc_Set_IPAddress(BYTE addr)
{
   BYTE *ipstr = NULL;
	 switch(addr%10)
   	{
   	  case 0:
	  	ipstr = "192.168.1.100";
	  	break;
	  case 1:
	  	ipstr = "192.168.1.101";
  		break;
   	  case 2:
	  	ipstr = "192.168.1.102";
	  	break;
   	  case 3:
	  	ipstr = "192.168.1.103";
	  	break;
   	  case 4:
	  	ipstr = "192.168.1.104";
	  	break;
   	  case 5:
	  	ipstr = "192.168.1.105";
	  	break;
   	  case 6:					  	
	  	ipstr = "192.168.1.106";
	  	break;
   	  case 7:
	  	ipstr = "192.168.1.107";
	  	break;
   	  case 8:
	  	ipstr = "192.168.1.108";
	  	break;
   	  case 9:
	  	ipstr = "192.168.1.109";
	  	break;

   	}
    return ipstr;

}

/*--------------------------------------------------------------------*/
/* User-supplied functions                                            */
/*--------------------------------------------------------------------*/

char *Adhoc_Ssid(void)
{
	return MPX_ADHOC_SSID;
}

int Adhoc_Channel(void)
{
	return adhoc_channel;
}
int Adhoc_Security(void)
{
	return adhoc_security;
}
#if 0
void Adhoc_Main()
{
	DWORD dwNWEvent; 
	DWORD adhoc_cnt =0;
	DWORD adhoc_scan =0;
	DWORD ipaddress = 0;
	DWORD file_size = 0;
	DWORD read_size = 0;
	DWORD file_idx = 1;
	DWORD test_count = 0;
	BYTE *file_buf=NULL;
	static DRIVE *sDrv;
	STREAM *handle = NULL;
	int ret = 0;
#if 1	
	BYTE file_num[4];
	BYTE file_name[32];
#else
	BYTE *file_name;
#endif	
	
	BYTE CurId = DriveCurIdGet();
    AdhocListInitList(&AdhocList);
	AdhocListInitList(&AdhocDecodeList);

	while(1)
	{
	    //mpDebugPrint("Wait ADHOC event!!");
		EventWait(WPA_EVENT, 0xFFFFFFFF, OS_EVENT_OR, &dwNWEvent);
		switch(dwNWEvent)
		{
		   case 0x01://scan
			   	mpDebugPrint("adhoc_main SCAN");
			   	NetScanRequestEventSet();
			   	break;
		   case 0x02://adhoc connect
			   	if((!adhoc_connect)&&adhoc_create)
			   	{
					netif_carrier_on(netdev_global);
					NetInterfaceEventSet(); /* notify network subsystem */
					ipaddress = Adhoc_Get_IPAddress(ethaddr[5]);
					NetDNSSet(2, ipaddress, 0);
					NetDNSSet(2, ipaddress, 1);
					xpgGotoNetFunction();
					EventSet(NETWORK_STREAM_EVENT, BIT0);
			   	}
				if(!adhoc_connect)
					adhoc_connect = 1;
		   		break;
		   case 0x04://adhoc disconnect
			   	mpDebugPrint("adhoc disconnect!!");
				Adhoc_Set_Beacon_Lost_Count(1);
	#if	0		
			   	if(adhoc_connect&&(Adhoc_Get_Beacon_Lost_Count()>3))
			   	{
			   	   netif_carrier_off(netdev_global);
				   adhoc_connect = 0;
				   mBssCnt = 0;
				  	if(!adhoc_create)
					{
					  if(adhoc_socket)
					  	closesocket(adhoc_socket);
					  zd_IbssConnect();
					}
					xpgGotoMainpage();
	                EventSet(WPA_EVENT, BIT0);

			   	}
	#endif			
		   		break;
			case 0x08:
				if(adhoc_create)
					break;

				mpDebugPrint("ADHOC RUN!!!");
				test_count = 0;
				
				if((!adhoc_create)&&(adhoc_socket == 0))
				{
					ipaddress = Adhoc_Get_IPAddress(ethaddr[5]);
					adhoc_socket =  mpx_DoConnect(ipaddress, 168, FALSE); /* use blocking socket */
					if(adhoc_socket > 0)
						mpDebugPrint("mpx_DoConnect OK");
				}
		#if	0	
				SetSensorInterfaceMode(MODE_PCCAM);
				SetImageSize(SIZE_176x144);
				SetSensorOverlayEnable(DISABLE);
				API_Sensor_Initial();
				BYTE *SensorInJpegBuffer=(BYTE*)ext_mem_malloc(176*144*2);
				DWORD jpegsize=GetSensorJpegImage(SensorInJpegBuffer);
		#endif		

resend:			
   	            //mpDebugPrint("1 get time %x",GetSysTime());

				sDrv=DriveGet(SD_MMC);
				DecString(file_num,file_idx,3,0);
				//mpDebugPrint("file_num %s",file_num);
				snprintf(file_name,32,"k%s",file_num);
				//mpDebugPrint("file_name %s",file_name);
				(int)handle = FileSearch(sDrv,file_name, "jpg", E_FILE_TYPE);
				
                if(handle!=NULL)
					 mpDebugPrint("FileSearch FILE FAIL!!");
				else
				{
					handle = FileOpen(sDrv);
					//mpDebugPrint("handle %x",handle);
					file_size = FileSizeGet(handle);
					(void*)file_buf = ext_mem_malloc(file_size);
					read_size = FileRead(handle,file_buf, file_size);
					//mpDebugPrint("FileRead %d",read_size);
					FileClose(handle);
				}
   	            //mpDebugPrint("2 get time %x",GetSysTime());
	send_one:			
				if(file_buf)
					ret = Adhoc_Send_PIC(file_buf,read_size);
				if(ret < 0)
				{
					//TaskSleep(50);
				   	closesocket(adhoc_socket);
					adhoc_socket =  mpx_DoConnect(ipaddress, 168, FALSE); /* use blocking socket */
					if(adhoc_socket > 0)
						mpDebugPrint("mpx_DoConnect OK!!");
					else
						mpDebugPrint("mpx_DoConnect FALSE!!");
				}
				//mpDebugPrint("3 get time %x",GetSysTime());

				if(file_idx < ADHOC_TEST_CYCLE)
					file_idx++;
				else
					file_idx = 1;
				
			    if(ADHOC_TEST_COUNT > 0)
			    {
					if(test_count < ADHOC_TEST_COUNT)
					{
					    test_count++ ;
					    //mpDebugPrint("t  %d",test_count);
						//TaskYield();
	                    //goto send_one;
						goto resend;
	 				}
			    }
				else
					goto resend;

				mpDebugPrint("TEST %d END",test_count);

				//if(adhoc_socket)
				//	closesocket(adhoc_socket);

		   	  break;
          case 0x10:
				Adhoc_Decode();
			  break;

		}
		TaskYield();
	}
}
#endif

