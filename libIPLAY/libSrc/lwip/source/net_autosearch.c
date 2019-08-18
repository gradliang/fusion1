
// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE 0

#include <errno.h>
#include "linux/types.h"
#include "socket.h"
#include "net_netdb.h"
#include "global612.h"
#include "mpTrace.h"

#include "uip.h"
#include "net_autosearch.h"
#include "ui.h"
#include "taskid.h"
#include "..\..\xml\include\netfs.h"
#include "netware.h"
#include "net_sys.h"
#include "net_netctrl.h"
#include "net_socket.h"
#include "net_ns.h"
#pragma alignvar(4)
SERVER_BROWSER ServerBrowser;
XML_ImageBUFF_link_t *g_qtr;
extern Net_App_State App_State;

void * SmbCli_BrowseReq(void);





/*---------------------------------------------------------------------------*/
u16_t * SearchServerByName(BYTE *name)
{
    u16_t *ipaddr = NULL;
    static u16_t u32Ipaddr[2];
    struct hostent *h;
    struct in_addr *curr;

//    h = mpx_gethostbyname(name);
    h = gethostbyname(name);
    if(!h)
        return NULL;

    if (curr = (struct in_addr *)h->h_addr_list[0])
    {
        ipaddr = u32Ipaddr;
		MP_DEBUG1("SearchServerByName %x", &curr->s_addr);
		//MP_DEBUG1("%x",curr->s_addr);
        *ipaddr = curr->s_addr>>16;
		*(ipaddr+1) = curr->s_addr;
		//memcpy(ipaddr,curr->s_addr,2);
        MP_DEBUG2("SearchServerByName ====>ipaddr %2x %2x", *ipaddr,*(ipaddr+1));
    }

    return ipaddr;
}


/*---------------------------------------------------------------------------*/
static void *AutoSearch_init()
{
	WORD i;

	ServerBrowser.bState = AS_STATE_INIT;
	ServerBrowser.dwNumberOfServer = 0;
	ServerBrowser.dwCurrentServer = 0;
	ServerBrowser.dwFirstList = 0;
	ServerBrowser.dwListIndex = 0;
	
	for(i=0; i<MAX_NUM_SERVER; i++)
	{
		ServerBrowser.ServerList[i].bState = NW_INVALID;
	}
	ServerBrowser.bState = AS_STATE_START;

#ifdef HAVE_SMB
	return SmbCli_BrowserReq();
#else
	return NULL;
#endif
}


void StopAutoSearch()
{
//	DisableNetWareTask();
	ServerBrowser.bState = AS_STATE_READY;
}
void *StartAutoSearch()
{
	mpDebugPrint("StartAutoSearch");
	return AutoSearch_init();	
}


BOOL Net_RecvType_isImage(DWORD status)
{
#if 1
	return ((status == NET_RECVBINARYDATA)
		||  (status == NET_RECVUPNP)
		    );
#else
	if ( (status == NET_RECVPICASA) || 
		      (status == NET_RECVFLICKR) || 
		      (status == NET_RECVGCE) || 
		      (status == NET_RECVPC) ||
		      (status == NET_RECVRSS) ||
#if YOUGOTPHOTO
			(status == NET_RECVYOUGOTPHOTO) ||
#endif
#if HAVE_FRAMECHANNEL
			(status == NET_RECVFRAMECHANNEL) ||
#endif
#if HAVE_FRAMEIT
		 	(status == NET_RECVFRAMEIT) ||
#endif
#if HAVE_SNAPFISH
		 	(status == NET_RECVSNAPFISH) ||
#endif
#if NET_UPNP
			(status == NET_RECVUPNP) ||
#endif
#if Make_WLS
			(status == NET_WLS) ||
#endif

			(status == NET_RECVBINARYDATA) 
    )
	{
		return TRUE;		
	}

  	return FALSE;	
#endif	
}

BOOL Net_AppState_isImage(DWORD status)
{
#if 0
	if ( (status & NET_RECVPICASA) || 
		      (status & NET_RECVFLICKR) || 
		      (status & NET_RECVGCE) || 
		      (status & NET_RECVPC) ||
		      (status & NET_RECVRSS) ||
#if YOUGOTPHOTO
			(status & NET_RECVYOUGOTPHOTO) ||
#endif
#if HAVE_FRAMECHANNEL
			(status & NET_RECVFRAMECHANNEL) ||
#endif
#if HAVE_FRAMEIT
		 	(status & NET_RECVFRAMEIT) ||
#endif
#if HAVE_SNAPFISH
		 	(status & NET_RECVSNAPFISH) ||
#endif
#if NET_UPNP
			(status & NET_RECVUPNP) ||
#endif
#if Make_WLS
			(status & NET_WLS) ||
#endif

			(status & NET_RECVBINARYDATA) 
    )
	{
		return TRUE;		
	}
#endif  
  	return FALSE;	
}

static void Xml_Http_buff_init()
{
	XML_BUFF_link_t *ptr;
	ptr = (XML_BUFF_link_t *)ext_mem_malloc(sizeof(XML_BUFF_link_t));
        if (!ptr)
            BREAK_POINT();
		
	memset(ptr->BUFF, 0x00, IMAGE_BUF);
	ptr->link = NULL;
	ptr->buff_len = 0;	
	App_State.XML_BUF = ptr;
       App_State.ptr = ptr;
}

static void Xml_Image_buff_init()
{
	XML_ImageBUFF_link_t *qtr;
	
	qtr = (XML_ImageBUFF_link_t *)ext_mem_malloc(sizeof(XML_ImageBUFF_link_t));
       if (!qtr)
       	BREAK_POINT();
	   
	memset(qtr->BUFF, 0x00, IMAGE_BUF);
	qtr->link = NULL;
	qtr->buff_len = 0;	
	App_State.XML_BUF1 = qtr;
       App_State.qtr = qtr;
	g_qtr = App_State.XML_BUF1;	
}

/**
 * @ingroup    NET_HTTP
 *
 * Initialize App_State for a URL transfer
 *
 * The fields of App_State are initialized and a buffer of an XML_BUFF_link_t 
 * or XML_ImageBUFF_link_t structure is also allocated from ext_mem memory pool 
 * for the storage of transfered data.
 *
 * @param status State of the application (or type of the transfer).  If used
 * for the state of the application, then its value can be one of:
 *
 *              @li <b>NET_RECVPICASA</b>       Picasa
 *              @li <b>NET_RECVFLICKR</b>       Flickr
 *              @li <b>NET_RECVHTTP</b>         XML/HTML
 *              @li <b>NET_RECVPC</b>           PC Sharing
 *              @li <b>NET_RECVRSS</b>          RSS
 *              @li <b>NET_RECVFRAMECHANNEL</b> Frame Channel
 *              @li <b>NET_RECVFRAMEIT</b>      FrameIt
 *              @li <b>NET_RECVSNAPFISH</b>     Snapfish
 *              @li <b>NET_RECVUPNP</b>         UPnP
 *
 * If used for the type of transfer, then its value can be one of:
 *
 *              @li <b>NET_RECVTEXTDATA</b>     Transfer text data.
 *              @li <b>NET_RECVBINARYDATA</b>   Transfer binary data.
 *
 * @retval None
 */
void Xml_BUFF_init(DWORD status)
{	
	App_State.dwTotallen = 0;	
	App_State.dwReconnect = 0;
	App_State.dwEndToggle = 0;
	App_State.dwOffset = 0; //cj add
	App_State.dwState |= status;		// abel 20071107 modify

	if(status == NET_RECVHTTP)
	{		
		Xml_Http_buff_init();
	}
	else if ( Net_RecvType_isImage(status))
	{			
		Xml_Image_buff_init();
	}
}

static void Xml_Image_buff_free()
{	
	XML_ImageBUFF_link_t * xp = App_State.XML_BUF1; 
	XML_ImageBUFF_link_t * next;	
	
	while(xp)
	{			
		next = xp->link;
		ext_mem_free(xp);
		xp = next;		
	}
	App_State.qtr = 
	App_State.XML_BUF1 = NULL;
	
#ifdef NET_NAPI
        wakeup_net_todo();
#endif	
}

static void Xml_Http_buff_free()
{	
	XML_BUFF_link_t *xp = App_State.XML_BUF;
	XML_BUFF_link_t *next;
	
	while(xp)
	{
		next = xp->link;
		ext_mem_free(xp);
		xp = next;		
	}
	App_State.ptr = 
	App_State.XML_BUF = NULL;
}

void Reset_App_state(DWORD status)
{
	App_State.dwTotallen = 0;	
	App_State.dwReconnect = 0;
	App_State.dwEndToggle = 0;
	App_State.dwOffset = 0; //cj add
	App_State.dwState &= ~status;
}

/**
 * @ingroup    NET_HTTP
 *
 * Free all the buffer memory of App_State after transfered URL data 
 * (XML/HTML or JPEG file) is no longer needed.
 *
 * @param status State of the application (or type of the transfer).  Please
 * see Xml_BUFF_init.
 *
 * @retval None
 */
void Xml_BUFF_free(DWORD status)
{
	if(status == NET_RECVHTTP)
	{		
		Xml_Http_buff_free();
	}	
	else if (Net_RecvType_isImage(status))
	{
		Xml_Image_buff_free();
	}	

	Reset_App_state(status);		
}

int Xml_BUFF_isFree(void)
{
    return App_State.XML_BUF1 ? false : true;
}

int NetFS_Mount(BYTE *item, BYTE type)
{
	//EnableNetWareTask();
	int ret = netfs_mount(item, NULL, type);
		
#if 0
#if HAVE_SHUTTERFLY
	if(type ==NETFS_SHUTTERFLY)			// Shutterfly
	{
		ret = netfs_mount(item, "/Shutterfly", NETFS_SHUTTERFLY); 
	}
#endif
#endif
   	//DisableNetWareTask();   
	return ret;
}

/*---------------------------------------------------------------------------*/
// return the pointer of name, if 0, means not found
BYTE * GetServerName(WORD Index)
{
	if(ServerBrowser.ServerList[Index].bState == NW_INVALID)
		return 0;
	
	return (BYTE *)(&ServerBrowser.ServerList[Index].ServerName[0]);
}

/*---------------------------------------------------------------------------*/
// return the pointer of name, if 0, means not found
BYTE * GetUrlName(WORD Index)
{
	if(ServerBrowser.ServerList[Index].bState == NW_INVALID)
		return 0;

	return (BYTE *)(&ServerBrowser.ServerList[Index].UrlName[0]);
}

/*---------------------------------------------------------------------------*/
DWORD GetNumberOfServer()
{
	return ServerBrowser.dwNumberOfServer;
	
}

/*---------------------------------------------------------------------------*/
DWORD GetCurrentServerIndex()
{
	return ServerBrowser.dwCurrentServer;
	
}


/*---------------------------------------------------------------------------*/
void SetCurrentServerIndex(DWORD index)
{
	ServerBrowser.dwCurrentServer = index;
	
}


//void *pstMovie;
//int add_server_cnt;
NWSERVER *add_server(char *hostname, char *url)
{
    int  i, tot;
    NWSERVER 	*srv;

    if(ServerBrowser.dwNumberOfServer >= MAX_NUM_SERVER)
        return NULL;

    tot = ServerBrowser.dwNumberOfServer;
    srv = &ServerBrowser.ServerList[0];
    for(i = 0;i<tot;i++, srv++)                 /* check for duplicate */
    {
        if(strlen(srv->ServerName) == strlen(hostname) &&
           strcmp(srv->ServerName,hostname) == 0)
            return srv;
    }

    srv = &ServerBrowser.ServerList[ServerBrowser.dwNumberOfServer++];

    memset(srv, 0, sizeof(*srv));

    snprintf(srv->ServerName, sizeof(srv->ServerName), hostname);
    snprintf(srv->UrlName, sizeof(srv->UrlName), url);
    srv->bState= NW_VALID;

    MP_DEBUG2("Server = %s,  URL = %s", srv->ServerName, srv->UrlName);

    return srv;
}

bool AutoSearch_IsDone(void)
{
    return (ServerBrowser.bState == AS_STATE_READY);
}
