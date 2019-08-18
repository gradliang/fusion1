/*-----------------------------------------------------------------------------------*/
/* vperf_test.c                                                                      */
/*                                                                                   */
/* Implementation of a receiver for an M-JPEG video performance test. Each JPEG file */
/* is received and displayed on the screen.                                          */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/

#define LOCAL_DEBUG_ENABLE 0
#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "socket.h"
#include <linux/if.h>		/* for "if_req" */
#include "net_socket.h"
#include "netware.h"
#include "list_mpx.h"

#ifdef VPERF_TEST

#define SHOW_JPEG 		    1

#define SEND_TIMEOUT_COUNT 3
#define ADHOC_RECEIVE_SIZE 8192
#define ADHOC_SEND_SIZE_LENGTH  4
#define ADHOC_BUF_SIZE 		    64*1024//64k


static DWORD recv_image_size =0;
static int vperf_Events;
static int vperf_ServerTask, vperf_PlaybackTask;
extern DWORD decode_time;

static void video_add(BYTE *image_buf,DWORD image_size);

static int server_socket_create()
{
    struct sockaddr_in addr, *localAddr = &addr;

    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int ret;
    
    if(s <= 0)
    {
        DPrintf("[VPERF] server socket create fail err=%d", s);
        return 0;
    }

    MP_DEBUG1("[VPERF] server socket id = %d", s);
    
    localAddr->sin_family = AF_INET;
    localAddr->sin_port = 168;//serverRec.port;
    localAddr->sin_addr.s_addr = INADDR_ANY;
//    localAddr->sin_addr.s_addr = 0xc0a80177;

    ret = bind(s, (struct sockaddr *)localAddr, sizeof(*localAddr));
    if (ret < 0)
        MP_ALERT("[VPERF] bind(%d,0x%x,%d) returns = %d", s, localAddr->sin_addr.s_addr,localAddr->sin_port, ret);

    ret = listen(s, 1);
    if (ret < 0)
        MP_ALERT("[VPERF] listen(%d) returns = %d", s, ret);
    
    return s;
}

/*
 * The receiver of video data
 */
static void vperf_Server(void)
{
    int ret;
	int sock, s;
	ST_SOCK_SET stReadSet;
	ST_SOCK_SET *rfds;
	int i;
	struct sockaddr_in peerAddr;
	DWORD dwEvent, dwNextEvent;
	BYTE *image_buf = NULL;
    DWORD image_size = 0;
    DWORD pic_size;
	DWORD idx=0,idx_end=0;
	char response[ADHOC_SEND_SIZE_LENGTH];
	unsigned long non_block = 1;
	DWORD recv_time = 0,cur_time = 0;
	int send_len, resp_len;
	int nfiles;
	
	sock = server_socket_create();
	MP_DEBUG("Server socket created=%d",sock);
    ioctlsocket(sock, FIONBIO, &non_block);

    while (1)
    {
        MPX_FD_ZERO(&stReadSet);
        MPX_FD_SET(sock, &stReadSet);

        rfds = &stReadSet;

        MP_DEBUG("%s:%d select(0,rfds,0,0,0)", __func__, __LINE__);

        ret = select(0, rfds, NULL, 0, NULL);

        MP_DEBUG("select returns 0x%x",ret);
        if( MPX_FD_ISSET(sock, &stReadSet) )
        {
            s = accept(sock, (struct sockaddr *)&peerAddr, NULL);
            MP_DEBUG("%s:%d accept(%d) returns %d", __func__, __LINE__, sock, s);
            MP_DEBUG(" TEST start time %u",GetSysTime());
            ioctlsocket(s, FIONBIO, &non_block);

            recv_time = GetSysTime();
			nfiles = 0;
            while(1)
            {
                MPX_FD_ZERO(&stReadSet);
                MPX_FD_SET(s, &stReadSet);

                ret = select(0, &stReadSet, NULL, 0, NULL);

                if (ret > 0)
                {
                    do
                    {
                        if (image_size == 0)
                        {
                            ret = recv(s,response+resp_len, 4-resp_len, 0);
                            if (ret > 0)
                            {
                                resp_len += ret;
                                if (resp_len == 4)
                                {
                                    image_size = (response[0]&0xFF)<<24|(response[1]&0xFF)<<16|(response[2]&0xFF)<<8|response[3]&0xFF;
                                    MP_DEBUG("image_size %d ",image_size);
                                    resp_len = 0;
                                    if (!image_size)
                                    {
                                      ret = 0;
                                      break;
                                    }
                                    pic_size = image_size;
                                    image_buf = ext_mem_malloc(ADHOC_BUF_SIZE);
                                    recv_image_size = 0;
                                }
                                else
                                    break;
                            }
                            else
                              break;
                        }

                        if (ret > 0)
                        {

                            int read_size = min(ADHOC_BUF_SIZE-recv_image_size, ADHOC_RECEIVE_SIZE);
                            if (read_size > image_size )
                                read_size = image_size ;
                            ret = recv(s,image_buf+recv_image_size, read_size, 0);
                            if(ret > 0)
                            {
                                recv_image_size+=ret;
                                image_size -= ret;

                                if(image_size == 0)
                                {
                                    nfiles++;
                                    MP_DEBUG("Done");
                                    //NetPacketDump(image_buf, 32);
                                    video_add(image_buf,pic_size);
                                    image_buf = NULL;
                                    i=0;

                                    TaskYield();
                                }
                                else
                                  //UartOutText("1");
                                  TaskYield();
                            }
                        }

                        TaskYield();
                    } while (ret > 0);

                    if (ret == 0)
                    {
                        long diff;
                        cur_time = GetSysTime();

                        diff = (long)cur_time - (long)recv_time;

                        send_len = send(s, response, ADHOC_SEND_SIZE_LENGTH, 0);
                        {
                            MP_DEBUG("diff %d total files=%d,%d",diff, nfiles, image_size);
                            if (image_buf)
                                ext_mem_free(image_buf);
                            closesocket(s);
                            s = 0;
                            TaskYield();
#if 0
                            debug_show_buf_leak();
#endif
                            break;
                        }
                        //UartOutText("2");
                        TaskYield();
                    }
                }
                TaskYield();
            }
            //TaskYield();
        }
    }
		
}

#define ADHOC_ADP_COUNT 		48
#define ADHOC_TEST_COUNT  		0 

struct vperf_pic {
	LM_LIST_ENTRY   Link;
	DWORD image_size;
	BYTE *image_buf;
	BYTE  Reserved[7];
};

static LM_LIST_CONTAINER vperf_DecodeList;
DWORD decode_time;
DWORD vpic_idx;
DWORD total_data;
static struct vperf_pic vpic[ADHOC_ADP_COUNT];
	
static void video_playback();

static void
vperf_ListInitList(
PLM_LIST_CONTAINER pList) 
{
	pList->Link.FLink = pList->Link.BLink = (PLM_LIST_ENTRY) 0;
	pList->EntryCount = 0;
} /* vperf_ListInitList */

/******************************************************************************/
/* ListPopHead -- pops the head off the list.                                 */
/******************************************************************************/
static PLM_LIST_ENTRY 
vperf_ListPopHead(
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
static void
vperf_ListPushTail(
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

static void vperf_playback_main(void)
{
	DWORD dwNWEvent; 

	vperf_ListInitList(&vperf_DecodeList);

    while (1)
    {
		EventWait(vperf_Events, BIT4, OS_EVENT_OR, &dwNWEvent);

        if (dwNWEvent & BIT4) {
            video_playback();
        }
    }

}

static void video_add(BYTE *image_buf,DWORD image_size)
{
    MP_ASSERT(!vpic[vpic_idx].image_buf);
	vpic[vpic_idx].image_buf = image_buf;
	vpic[vpic_idx].image_size = image_size;
	vperf_ListPushTail(&vperf_DecodeList,&vpic[vpic_idx].Link);
	
	if(vpic_idx<(ADHOC_ADP_COUNT-1))
		vpic_idx++;
	else
		vpic_idx = 0;
    EventSet(vperf_Events, BIT4);
}

static void video_playback(void)
{
   struct vperf_pic *pic;
   while (1)
   {
   	  (PLM_LIST_ENTRY)pic = vperf_ListPopHead(&vperf_DecodeList);
	  
      if (!pic)
          break;

      IntDisable();
	  ImageAdhocDraw_Decode(pic->image_buf,pic->image_size);
	  IntEnable();

	  ext_mem_free(pic->image_buf);
	  if(ADHOC_TEST_COUNT > 0)
	     total_data+=pic->image_size;
	  pic->image_buf = NULL;
	  decode_time++;
      TaskYield();
   }
   if(ADHOC_TEST_COUNT > 0)
   {
	   if(decode_time > ADHOC_TEST_COUNT)
	   	{
	   	  MP_DEBUG(" DECODE time %x",GetSysTime());
		  MP_DEBUG(" avg size %d",total_data/decode_time);

		  decode_time = 0;
	   	}
   }
}

static void vperf_PlaybackInit()
{
    vperf_PlaybackTask = mpx_TaskCreate(vperf_playback_main, CONTROL_PRIORITY, 0x4000);
	MP_ASSERT(vperf_PlaybackTask > 0);
}

void vperf_Init(void)
{
	vperf_Events = mpx_EventCreate((OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);

	MP_ASSERT(vperf_Events > 0);

	vperf_ServerTask = mpx_TaskCreate(vperf_Server, CONTROL_PRIORITY, 0x4000);

	MP_ASSERT(vperf_ServerTask > 0);

    vperf_PlaybackInit();
}

void vperf_Start(void)
{
    SDWORD r;
    r = TaskStartup(vperf_PlaybackTask);
    MP_DEBUG("[VPERF] %s: r=%d", __func__, r);
    MP_ASSERT(r == OS_STATUS_OK);
    TaskStartup(vperf_ServerTask);
}

#endif

