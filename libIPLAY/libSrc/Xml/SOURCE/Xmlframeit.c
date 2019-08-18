
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0 
#include "corelib.h"

//#include "corelib.h"

#if NETWARE_ENABLE

//#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "global612.h"
#include "mpTrace.h"
//#include "heaputil_mem.h"
#include "netfs.h"
#include "netfs_pri.h"
#include "ndebug.h"
#include "xmlframeit.h"

#include "..\..\lwip\include\net_sys.h"
#include "..\..\CURL\include\net_curl_curl.h"

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define frameit_malloc(sz)   mm_malloc(sz)
#define frameit_mfree(ptr)   mm_free(ptr)

//note: fixed to use global my_write_func() for all xmlxxxxxx.c files!
size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf);

static struct netfs_file_entry jpeg_info;
//frameit_map_t *map;
static frameit_info_t    frameit_info;
static const char   *frameit_base_dir;
char  Frameit_Request[MAX_CONTACTS];

#define Frameit_manufacturerId "MagicPixel"
#define Frameit_serialNumber "1234567890"
#define FRAMEIT_HOST "http://frameit.live.com"

extern Net_App_State App_State;
extern DWORD g_recvsize;
extern char uip_recvbuff[4*1400];
extern void *uip_appdata;   
extern U16 uip_len, uip_slen;

static void Frameit_content_handler(void *user_data, const char *str, int len)
{
	frameit_info_t   *frameit_info = (frameit_info_t *) user_data;
    	 
	char    *title;
    	const char      *title_value    = NULL;
	int i;
	
	//mpDebugPrint("333 frameit_info->state = %d,s=%s,len=%d",frameit_info->state,str,len);
		
	switch(frameit_info->state)
	{
//=================================================	
		case FRAMEIT_GetClaimToken_ClaimToken:
			strncpy(frameit_info->ClaimToken,str,len);
			mpDebugPrint("1 frameit_info->ClaimToken = %s",frameit_info->ClaimToken);
			break;
			
		case FRAMEIT_GetClaimToken_ClaimUrl:
			strncpy(frameit_info->ClaimUrl,str,len);
			//mpDebugPrint("1 frameit_info->ClaimUrl = %s",frameit_info->ClaimUrl);
			break;
			
		case FRAMEIT_GetClaimToken_ResponseCode:
			frameit_info->ResponseCode = atoi(str);
			//mpDebugPrint("1 frameit_info->ResponseCode = %d",frameit_info->ResponseCode);
			break;
//=================================================			
		case FRAMEIT_DeviceBind_DeviceId:
			strncpy(frameit_info->DeviceId,str,len);
			//mpDebugPrint("2 frameit_info->DeviceId = %s",frameit_info->DeviceId);			
			break;
			
		case FRAMEIT_DeviceBind_ResponseCode:
			frameit_info->ResponseCode = atoi(str);
			//mpDebugPrint("2 frameit_info->ResponseCode = %d",frameit_info->ResponseCode);
			break;
//=================================================	
		case FRAMEIT_DeviceBindUser_DeviceId:
			
			break;
		case FRAMEIT_DeviceBindUser_ResponseCode:
			
			break;
//=================================================	
		case FRAMEIT_CollectionInfo_Name:
			//frameit_info->cur_item->Namelen = len;
			//strncpy(frameit_info->cur_item->Name,str,len);
			if (frameit_info->state == FRAMEIT_CollectionInfo_Name && frameit_info->cur_item)
    			{
        			title = frameit_info->cur_item->Name;
        			for (i = strlen(title); i < MAX_TITLE && len > 0; len--, i++)
            			title[i] = *str++;
        			title[i] = 0;
    			}

			//mpDebugPrint("3 FRAMEIT_CollectionInfo_Name = %s",frameit_info->cur_item->Name);
			break;
		case FRAMEIT_CollectionInfo_FeedUrl:
			frameit_info->cur_item->FeedUrllen= len;
			strncpy(frameit_info->cur_item->FeedUrl,str,len);
			//mpDebugPrint("3 FRAMEIT_CollectionInfo_FeedUrl = %s",frameit_info->cur_item->FeedUrl);
			break;
		case FRAMEIT_CollectionInfo:
			//mpDebugPrint("3 FRAMEIT_CollectionInfo = %s",str);
			break;
		case FRAMEIT_CollectionInfoList:
			//mpDebugPrint("3 FRAMEIT_CollectionInfoList = %s",str);
			break;			
		case FRAMEIT_GetCollectionInfo_ResponseCode:			
			frameit_info->ResponseCode = atoi(str);
			//mpDebugPrint("3 frameit_info->ResponseCode = %d",frameit_info->ResponseCode);
			break;
//=================================================	
		case FRAMEIT_CollectionInfoUser_Name:
			        		
			break;
		case FRAMEIT_CollectionInfoUser_FeedUrl:
			
			break;
		case FRAMEIT_CollectionInfoUser:
			
			break;
		case FRAMEIT_CollectionInfoUserList:
			
			break;
		case FRAMEIT_CollectionInfoUser_ResponseCode:
			
			break;
	}
	
}

static void Frameit_start_element_handler(void *user_data, const char *name, const char **attr)
{
	frameit_info_t   *frameit_info = (frameit_info_t *) user_data;
    
	item_entry_t    *item_entry;	
  	const char      *url_value      = NULL;	
   	const char      *length_value = NULL;
 
		
	switch(frameit_info->state)
	{

//===============================================	
		case FRAMEIT_GetClaimTokenResult:
    	
    			if (!strcasecmp(name, "ClaimToken"))
        		{
				frameit_info->state = FRAMEIT_GetClaimToken_ClaimToken;
				frameit_info->ClaimToken[0] = '\0';
        		}
			else if (!strcasecmp(name, "ClaimUrl"))
        		{	
        			frameit_info->state = FRAMEIT_GetClaimToken_ClaimUrl;
				frameit_info->ClaimUrl[0] = '\0';
			}		
			else if (!strcasecmp(name, "ResponseCode"))
        		{	
        			frameit_info->state = FRAMEIT_GetClaimToken_ResponseCode;
			}
			break;
			
//==============================================
		case FRAMEIT_DeviceBindResult:
			if (!strcasecmp(name, "DeviceId"))
        		{
				frameit_info->state = FRAMEIT_DeviceBind_DeviceId;
				frameit_info->DeviceId[0] = '\0';
        		}
			else if (!strcasecmp(name, "ResponseCode"))
        		{	
        			frameit_info->state = FRAMEIT_DeviceBind_ResponseCode;				
			}		
			break;
//===============================================			
		case FRAMEIT_DeviceBindUserResult:
			if (!strcasecmp(name, "DeviceId"))
        		{
				frameit_info->state = FRAMEIT_DeviceBindUser_DeviceId;
				frameit_info->DeviceId[0] = '\0';
			}
			else if (!strcasecmp(name, "ResponseCode"))
        		{	
        			frameit_info->state = FRAMEIT_DeviceBindUser_ResponseCode;	
			}		
			break;
//===============================================			
		case FRAMEIT_GetCollectionInfoResult:
			if (!strcasecmp(name, "CollectionInfoList"))
        		{
				frameit_info->state = FRAMEIT_CollectionInfoList;
			}
			else if (!strcasecmp(name, "ResponseCode"))
        		{
				frameit_info->state = FRAMEIT_GetCollectionInfo_ResponseCode;
	      		}
			break;
			
		case FRAMEIT_CollectionInfoList:
			if (!strcasecmp(name, "CollectionInfo"))
        		{	
        			frameit_info->state = FRAMEIT_CollectionInfo;
			}		
			break;

		case FRAMEIT_CollectionInfo:
			if (!strcasecmp(name, "Name"))
        		{	        		
        			item_entry_t    *set_entry;
					
        			frameit_info->state = FRAMEIT_CollectionInfo_Name;

				set_entry = (item_entry_t *)frameit_malloc(sizeof(item_entry_t));

				memset(set_entry, 0x00, sizeof(item_entry_t));
                		
                		frameit_info->cur_item->next = set_entry;
                		frameit_info->cur_item = set_entry;	
			}		
			else if (!strcasecmp(name, "FeedUrl"))
        		{	
        			frameit_info->state = FRAMEIT_CollectionInfo_FeedUrl;
			}	
			break;
//=================================================			
		case FRAMEIT_GetCollectionInfoUserResult:
			if (!strcasecmp(name, "CollectionInfoList"))
        		{        		
        			frameit_info->state = FRAMEIT_CollectionInfoUserList;
			}		
			else if (!strcasecmp(name, "ResponseCode"))
        		{	
        			frameit_info->state = FRAMEIT_CollectionInfoUser_ResponseCode;
			}	
			break;
			
		case FRAMEIT_CollectionInfoUserList:
			if (!strcasecmp(name, "CollectionInfo"))
        		{	
        			frameit_info->state = FRAMEIT_CollectionInfoUser;
			}		
			break;

		case FRAMEIT_CollectionInfoUser:
			if (!strcasecmp(name, "Name"))
        		{	
        			frameit_info->state = FRAMEIT_CollectionInfoUser_Name;
			}		
			else if (!strcasecmp(name, "FeedUrl"))
        		{	
        			frameit_info->state = FRAMEIT_CollectionInfoUser_FeedUrl;
			}	
			break;	
			
    	}
	
}

static void Frameit_end_element_handler(void *user_data, const char *name)
{
	frameit_info_t    *frameit_info     = (frameit_info_t *) user_data;
    	
	//mpDebugPrint("222 frameit_info->state = %d,name=%s",frameit_info->state,name);
	
	switch(frameit_info->state)
	{
//=================================================	
		case FRAMEIT_GetClaimToken_ClaimToken:
			if (!strcasecmp(name, "ClaimToken"))
			{
				frameit_info->state = FRAMEIT_GetClaimTokenResult;							
			}
			break;
			
		case FRAMEIT_GetClaimToken_ClaimUrl:
			if (!strcasecmp(name, "ClaimUrl"))
        		{	
        			frameit_info->state = FRAMEIT_GetClaimTokenResult;
			}
			break;
			
		case FRAMEIT_GetClaimToken_ResponseCode:
			if (!strcasecmp(name, "ResponseCode"))
        		{	
        			frameit_info->state = FRAMEIT_GetClaimTokenResult;
			}
			break;
//=================================================			
		case FRAMEIT_DeviceBind_DeviceId:
			if (!strcasecmp(name, "DeviceId"))
        		{
				frameit_info->state = FRAMEIT_DeviceBindResult;
        		}
			break;
		case FRAMEIT_DeviceBind_ResponseCode:
			if (!strcasecmp(name, "ResponseCode"))
        		{	
        			frameit_info->state = FRAMEIT_DeviceBindResult;
			}	
			break;
//=================================================	
		case FRAMEIT_DeviceBindUser_DeviceId:
			if (!strcasecmp(name, "DeviceId"))
        		{
				frameit_info->state = FRAMEIT_DeviceBindUserResult;
        		}
			break;
		case FRAMEIT_DeviceBindUser_ResponseCode:
			if (!strcasecmp(name, "ResponseCode"))
        		{	
        			frameit_info->state = FRAMEIT_DeviceBindUserResult;
			}	
			break;
//=================================================	
		case FRAMEIT_CollectionInfo_Name:
			if (!strcasecmp(name, "Name"))
        		{
				frameit_info->state = FRAMEIT_CollectionInfo_FeedUrl;
        		}
			break;
		case FRAMEIT_CollectionInfo_FeedUrl:
			if (!strcasecmp(name, "FeedUrl"))
        		{
				frameit_info->state = FRAMEIT_CollectionInfo;
        		}
			break;
		case FRAMEIT_CollectionInfo:
			if (!strcasecmp(name, "CollectionInfo"))
        		{
				frameit_info->state = FRAMEIT_CollectionInfoList;
        		}
			break;
		case FRAMEIT_CollectionInfoList:
			if (!strcasecmp(name, "CollectionInfoList"))
        		{
				frameit_info->state = FRAMEIT_GetCollectionInfoResult;
			}
			break;			
		case FRAMEIT_GetCollectionInfo_ResponseCode:
			if (!strcasecmp(name, "ResponseCode"))
        		{
				frameit_info->state = FRAMEIT_GetCollectionInfoResult;
			}
			break;
//=================================================	
		case FRAMEIT_CollectionInfoUser_Name:
			if (!strcasecmp(name, "Name"))
        		{	
        			frameit_info->state = FRAMEIT_CollectionInfoUser_FeedUrl;
			}
			break;
		case FRAMEIT_CollectionInfoUser_FeedUrl:
			if (!strcasecmp(name, "FeedUrl"))
        		{	
        			frameit_info->state = FRAMEIT_CollectionInfoUser;
			}	
			break;
		case FRAMEIT_CollectionInfoUser:
			if (!strcasecmp(name, "CollectionInfo"))
        		{
				frameit_info->state = FRAMEIT_CollectionInfoUserList;
        		}
			break;
		case FRAMEIT_CollectionInfoUserList:
			if (!strcasecmp(name, "CollectionInfoList"))
        		{
				frameit_info->state = FRAMEIT_GetCollectionInfoUserResult;
			}
			break;
		case FRAMEIT_CollectionInfoUser_ResponseCode:
			if (!strcasecmp(name, "ResponseCode"))
        		{
				frameit_info->state = FRAMEIT_GetCollectionInfoUserResult;
			}
			break;
	}
	
}

int frameit_FetchFeed(char *ttl,char *pubDate)
{
	int len ;
	BYTE request[MAX_CONTACTS];
	
	memset(Frameit_Request,0x00,MAX_CONTACTS);
	memset(request,0x00,MAX_CONTACTS);
	
	len = snprintf(request,MAX_CONTACTS,
			"<?xml version=\"1.0\" ?>"
			"<rss version=\"2.0\" xmlns:media=\"http://search.yahoo.com/mrss/\">"
			"<channel>"
			"<ttl>%s</ttl>"
			"<title>Demo Slide Show</title>"
			"<link>http://frameit.live.com</link>"
			"<generator>http://frameit.live.com</generator>"
			"<lastBuildDate>Thu, 10 Apr 2008 13:20:47 -07:00</lastBuildDate>"
			"<pubDate>%s</pubDate>"
			"<description></description>"	
			"</channel>"
			"</rss>",
              	ttl,
              	pubDate);
			
	return len;	
}

static int frameit_GetClaimToken(frameit_info_t  *frameit_info)
{	
	int   ret = FAIL;
	int   len;
	char data[MAX_CONTACTS];  
	char *url; 	
	int httpcode;

    	url = "http://frameit.live.com/service/devicesvc.asmx";
	memset(data,0x00,MAX_CONTACTS);	
	len = snprintf(data,MAX_CONTACTS,			
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
  			"<soap:Body>"
    			"<GetClaimToken xmlns=\"http://frameit.live.com/service/device/1.0/\">"
      			"<manufacturerId>%s</manufacturerId>"
      			"<serialNumber>%s</serialNumber>"
    			"</GetClaimToken>"
  			"</soap:Body>"
			"</soap:Envelope>",
              	Frameit_manufacturerId,
              	Frameit_serialNumber);
		
	char * pHeader1 = "SOAPAction: \"http://frameit.live.com/service/device/1.0/GetClaimToken\"";
	char * pHeader2 = "Content-Type: text/xml; charset=utf-8";

	Xml_BUFF_init(NET_RECVHTTP);	
	
	httpcode = mpx_curl_excute(url, data, pHeader1, pHeader2);
	
	if (httpcode == 200)
	{	    		
		frameit_info->state = FRAMEIT_GetClaimTokenResult;
		
		MPX_XML_Parse(frameit_info
		               		, Frameit_start_element_handler
		               		, Frameit_end_element_handler
		               		, Frameit_content_handler);	
		
		if(frameit_info->ResponseCode)
			ret = FAIL;
    		else
			ret = PASS;		
	}		

	Xml_BUFF_free(NET_RECVHTTP);
	return ret;
}

static int frameit_DeviceBind(frameit_info_t  *frameit_info)
{	
	int   ret = FAIL;
	int   len;
	char data[MAX_CONTACTS];  
	char *url; 	
	int httpcode;
    	
	url = "http://frameit.live.com/service/devicesvc.asmx";
	memset(data,0x00,MAX_CONTACTS);	
	 len = snprintf(data,MAX_CONTACTS,
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
  			"<soap:Body>"
    			"<DeviceBind xmlns=\"http://frameit.live.com/service/device/1.0/\">"
      			"<claimToken>%s</claimToken>"
      			"<manufacturerId>%s</manufacturerId>"
      			"<serialNumber>%s</serialNumber>"
    			"</DeviceBind>"
  			"</soap:Body>"
			"</soap:Envelope>",
			frameit_info->ClaimToken,
            		Frameit_manufacturerId,
            		Frameit_serialNumber);
		
	char * pHeader1 = "SOAPAction: \"http://frameit.live.com/service/device/1.0/DeviceBind\"";
	char * pHeader2 = "Content-Type: text/xml; charset=utf-8";

	Xml_BUFF_init(NET_RECVHTTP);	
	
	httpcode = mpx_curl_excute(url, data, pHeader1, pHeader2);
	
	if (httpcode == 200)
	{
		frameit_info->state = FRAMEIT_DeviceBindResult;

		MPX_XML_Parse(frameit_info
		               , Frameit_start_element_handler
		               , Frameit_end_element_handler
		               , Frameit_content_handler);		
		
		if(frameit_info->ResponseCode)
			ret = FAIL;
    		else
			ret = PASS;
	}
	
	Xml_BUFF_free(NET_RECVHTTP);
    	return ret;
}

int  frameit_DeviceBindUser(frameit_info_t  *frameit_info)
{
	int   ret = FAIL;
	int   len;
	char data[MAX_CONTACTS];  
	char *url; 	
	int httpcode;	

    	url = "http://frameit.live.com/service/devicesvc.asmx";
	memset(data,0x00,MAX_CONTACTS);	
	len = snprintf(data,MAX_CONTACTS,
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
  			"<soap:Body>"
    			"<DeviceBindUser xmlns=\"http://frameit.live.com/service/device/1.0/\">"
      			"<manufacturerId>%s</manufacturerId>"
      			"<serialNumber>%s</serialNumber>"
    			"</DeviceBindUser>"
  			"</soap:Body>"
			"</soap:Envelope>",
			Frameit_manufacturerId,
            		Frameit_serialNumber);
		
	 char * pHeader1 = "SOAPAction: \"http://frameit.live.com/service/device/1.0/DeviceBindUser\"";
	 char * pHeader2 =  "Content-Type: text/xml; charset=utf-8";
	
	Xml_BUFF_init(NET_RECVHTTP);	
	
	httpcode = mpx_curl_excute(url, data, pHeader1, pHeader2);
	
	if (httpcode == 200)
	{
		frameit_info->state = FRAMEIT_DeviceBindUserResult;

		MPX_XML_Parse(frameit_info
		               , Frameit_start_element_handler
		               , Frameit_end_element_handler
		               , Frameit_content_handler);		
		
		if(frameit_info->ResponseCode)
			ret = FAIL;
    		else
			ret = PASS;			
	}
	
	Xml_BUFF_free(NET_RECVHTTP);
    	return ret;
}

static int frameit_GetCollectionInfo(frameit_info_t  *frameit_info)
{	
	int   ret = FAIL;
	int   len;
	char data[MAX_CONTACTS];  
	char *url; 	
	int httpcode;		
    	
    	url = "http://frameit.live.com/service/devicesvc.asmx";
	memset(data,0x00,MAX_CONTACTS);	
	len = snprintf(data,MAX_CONTACTS,
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
  			"<soap:Body>"
    			"<GetCollectionInfo xmlns=\"http://frameit.live.com/service/device/1.0/\">"
      			"<deviceId>%s</deviceId>"
    			"</GetCollectionInfo>"
  			"</soap:Body>"
			"</soap:Envelope>",
			frameit_info->DeviceId);

	 char * pHeader1 =  "SOAPAction: \"http://frameit.live.com/service/device/1.0/GetCollectionInfo\"";
	 char * pHeader2 =   "Content-Type: text/xml; charset=utf-8";

	 Xml_BUFF_init(NET_RECVHTTP);	
	
	httpcode = mpx_curl_excute(url, data, pHeader1, pHeader2);
	
	if (httpcode == 200)
	{
		frameit_info->state = FRAMEIT_GetCollectionInfoResult;

		MPX_XML_Parse(frameit_info
		               , Frameit_start_element_handler
		               , Frameit_end_element_handler
		               , Frameit_content_handler);		
		
		if(frameit_info->ResponseCode)
			ret = FAIL;
    		else
			ret = PASS;	

	}
	
	Xml_BUFF_free(NET_RECVHTTP);
    	return ret;
}

int  frameit_GetCollectionInfoUser(frameit_info_t  *frameit_info)
{
	int   ret = FAIL;
	int   len;
	char data[MAX_CONTACTS];  
	char *url; 	
	int httpcode;		
    
    	url = "http://frameit.live.com/service/devicesvc.asmx";
	memset(data,0x00,MAX_CONTACTS);	
	len = snprintf(data,MAX_CONTACTS,
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
  			"<soap:Body>"
			"<GetCollectionInfoUser xmlns=\"http://frameit.live.com/service/device/1.0/\" />"
      			"</soap:Body>"
			"</soap:Envelope>"
			);

	char * pHeader1 = "SOAPAction: \"http://frameit.live.com/service/device/1.0/GetCollectionInfoUser\"";
	char * pHeader2 =  "Content-Type: text/xml; charset=utf-8";

	Xml_BUFF_init(NET_RECVHTTP);	
	
	httpcode = mpx_curl_excute(url, data, pHeader1, pHeader2);
	
	if (httpcode == 200)
	{		
		frameit_info->state = FRAMEIT_GetCollectionInfoUserResult;
		
		MPX_XML_Parse(frameit_info
		               , Frameit_start_element_handler
		               , Frameit_end_element_handler
		               , Frameit_content_handler);		
		
		if(frameit_info->ResponseCode)
			ret = -1;
    		else
			ret = 0;	
	}
	
	Xml_BUFF_free(NET_RECVHTTP);
    	return ret;		
}


/**
 *  Get photo set of specified user id
 */
static void photolist_content_handler(void *user_data, const char *s, int len)
{
	frameit_info_t    *frameit_info     = (frameit_info_t *) user_data;
    
	char *title;
    	char *title_value    = NULL;
	char data[len+1];

	 if (frameit_info->state == FRAMEIT_TTL)
    	{
			// to do;.....
			memset(data,0x00,len+1);
			strncpy(data,s,len);
			mpDebugPrint("FRAMEIT_TTL = %s",data);
	 }
	 else if (frameit_info->state == FRAMEIT_ITEM_TITLE)
    	{       	
		strncpy(jpeg_info.pathname,s,len);
		strcat(jpeg_info.pathname,".JPG");	
		//mpDebugPrint("photolist_content_handler = %s",jpeg_info.pathname);
    	}
}


/**
 *  Get photo list of specified photo set
 */
static void photolist_start_element_handler(void *user_data, const char *name, const char **attr)
{
    	frameit_info_t    *frameit_info     = (frameit_info_t *) user_data;
    
	//item_entry_t    *item_entry;	
  	const char      *url_value      = NULL;	
   	//const char      *length_value = NULL;
		int i;
	if (frameit_info->state == FRAMEIT_NULL)
    	{
        	if (!strcasecmp(name, "rss"))
        	{        		
			frameit_info->state = FRAMEIT_START;
			//mpDebugPrint("0 rss");
        	}
    	}
	else if(frameit_info->state == FRAMEIT_START)
	{
		if (!strcasecmp(name, "channel"))
        	{        		
			frameit_info->state = FRAMEIT_CHANNELs;
			//mpDebugPrint("0 channel");
        	}
	}
	else if(frameit_info->state == FRAMEIT_CHANNELs)
	{
		if(!strcasecmp(name, "ttl"))
        	{        		
			frameit_info->state = FRAMEIT_TTL;
			//mpDebugPrint("0 ttl");
        	}
	/*	else if(!strcasecmp(name, "title"))
        	{        		
			frameit_info->state = FRAMEIT_TITLE;
			mpDebugPrint("0 title");
        	}
        */	
	/*	else if(!strcasecmp(name, "description"))
        	{        		
			frameit_info->state = FRAMEIT_DESC;
			mpDebugPrint("0 description");
        	}*/
		else if (!strcasecmp(name, "item"))
        	{        		
        			frameit_info->state = FRAMEIT_ITEM;
			memset(&jpeg_info, 0, sizeof(jpeg_info));	  		
			sprintf(jpeg_info.length,"%d",0);
			//mpDebugPrint("0 item");
		}		
	}
	else if(frameit_info->state == FRAMEIT_ITEM)
	{
		if(!strcasecmp(name, "title"))
        	{        		
			frameit_info->state = FRAMEIT_ITEM_TITLE;
			//mpDebugPrint("1 title");
        	}
		else if(!strcasecmp(name, "description"))
        	{        		
			frameit_info->state = FRAMEIT_ITEM_DESCRIPTION;
			//mpDebugPrint("1 description");
        	}
       	
		else if(!strcasecmp(name, "pubDate"))
        	{        		
			frameit_info->state = FRAMEIT_ITEM_PUBDATE;
			//mpDebugPrint("1 pubDate");
        	}
		else if(!strcasecmp(name, "enclosure"))
        	{        		
        		//mpDebugPrint("1 enclosure");
        		while (*attr)
            		{
                		if (!strcasecmp(*attr, "url"))
                		{
                    			attr++;
                    			url_value = *attr;
                		}
                		else
                		{
                    			attr ++;
                		}                
                		attr ++;
            		}
			//mpDebugPrint("url_value = %s",url_value);	
			strncpy(jpeg_info.url,url_value,strlen(url_value));		
			Net_Xml_parseFileList(&jpeg_info);
		}
		
	}	
}
 

static void photolist_end_element_handler(void *user_data, const char *name)
{

	frameit_info_t    *frameit_info     = (frameit_info_t *) user_data;
  		
	if (frameit_info->state == FRAMEIT_NULL)
    	{
        
    	}
	else if(frameit_info->state == FRAMEIT_START)
	{	
		if (!strcasecmp(name, "rss"))
        	{
			frameit_info->state = FRAMEIT_NULL;
			//mpDebugPrint("2 rss");
			/* finish */
       	}
	}	
	else if(frameit_info->state == FRAMEIT_CHANNELs)
	{
		if (!strcasecmp(name, "channel"))
		{
   			frameit_info->state = FRAMEIT_START;
			//mpDebugPrint("2 channel");
		}		
	}
	else  if(frameit_info->state == FRAMEIT_TTL)
       {        		
		if(!strcasecmp(name, "ttl"))	
		{
			frameit_info->state = FRAMEIT_CHANNELs;
			//mpDebugPrint("0 ttl");
		}	
       }		
	/*else if(frameit_info->state == FRAMEIT_TITLE)
	{
		if (!strcasecmp(name, "title"))
		{
			frameit_info->state = FRAMEIT_CHANNELs;
			mpDebugPrint("2 title");
		}
	}*/			
	/*else if(frameit_info->state == FRAMEIT_DESC)
	{
		if (!strcasecmp(name, "description"))
		{
   			frameit_info->state = RSS_CHANNELs;
			mpDebugPrint("2 description");
		}
	}*/
	else if(frameit_info->state == FRAMEIT_ITEM)
	{
		if (!strcasecmp(name, "item"))
		{
			frameit_info->state = FRAMEIT_CHANNELs;
			//mpDebugPrint("3 item");
		}
	}
	else if(frameit_info->state == FRAMEIT_ITEM_TITLE)
	{
		if (!strcasecmp(name, "title"))
		{
   			frameit_info->state = FRAMEIT_ITEM;
			//mpDebugPrint("3 title");
		}		
	}
	else if (frameit_info->state == FRAMEIT_ITEM_PUBDATE)
    	{
		if (!strcasecmp(name, "pubDate"))
        	{
			frameit_info->state = FRAMEIT_ITEM;
			//mpDebugPrint("3 pubDate");
		}
	}

	else if (frameit_info->state == FRAMEIT_ITEM_DESCRIPTION)
    	{
		if (!strcasecmp(name, "description"))
        	{
			frameit_info->state = FRAMEIT_ITEM;
			//mpDebugPrint("3 description");
		}
	}		
	else if (frameit_info->state == FRAMEIT_ITEM_ENCLOSURE)
    	{
		;
	}

}

static int frameit_fetch_photolists(frameit_info_t *frameit_info)
{
    int                     ret;

    // Get Data from Remote Site and Parse it 
    frameit_info->state = FRAMEIT_NULL;

    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(frameit_info->cur_item->FeedUrl,NETFS_FRAMEIT,0,0);
    if(ret < 0) 
        goto Recv_exit;	

    MPX_XML_Parse(frameit_info
		               , photolist_start_element_handler
		               , photolist_end_element_handler
		               , photolist_content_handler);
	
exit:

    Xml_BUFF_free(NET_RECVHTTP);   	
    return ret;

Recv_exit:
fatal_exit:
    goto exit;
}

int frameit_init(const char *username, const char *base_dir)
{
    item_entry_t     *tmp_entry;
    
    int     ret;
    int     count;   
	
    memset(&frameit_info, 0x00, sizeof(frameit_info_t));
    strncat(frameit_info.username, username, MAX_USERNAME);
    frameit_info.cur_item = &frameit_info.item_list;
    frameit_base_dir = base_dir;
	
    MP_DEBUG("1 frameit_fetch_photosets  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    	
    ret = frameit_GetClaimToken(&frameit_info);
    if (ret < 0)
    {
        //mpDebugPrint("error code: %d", frameit_info->info.error_code);
        return ret;
    }
	
    return NETFS_OK;
}

char *frameit_GetToken()
{
	return frameit_info.ClaimToken;
}

int frameit_GetResponse()
{
	return frameit_info.ResponseCode;
}

int frameit_ConfirmPairingRequestDID(void)
{
	int     ret;
	
	ret = frameit_DeviceBind(&frameit_info);
    	if (ret < 0)
    	{
        	//mpDebugPrint("error code: %d", map->info.error_code);
        	return ret;
    	}
	ret = NETFS_OK;	

	return ret;
}

int frameit_RequestUserCollections(void)
{
	int ret;
	int     count;
	item_entry_t *tmp_entry;
		
	ret = frameit_GetCollectionInfo(&frameit_info);
    	if (ret < 0)
    	{
        	//mpDebugPrint("error code: %d", map->info.error_code);
        	return ret;
    	}
		
	 count = 0;

	frameit_info.cur_item = frameit_info.item_list.next;
		
    	tmp_entry = frameit_info.cur_item;

	while (tmp_entry)
       {
  		Net_Xml_PhotoSetList(tmp_entry->Name,count);			

		count ++;
     		tmp_entry = tmp_entry->next;
	}
	Net_PhotoSet_SetCount(count);	
	
	
	ret = NETFS_OK;	

	return ret;
}


int frameit_PhotoList_Get(BYTE *PhotoSet)
{
	frameit_info_t *fframeit_info;
	int     ret;
    	int     count;
       //image_entry_t *image_entry;
	   	
	fframeit_info = &frameit_info;  	
       fframeit_info->cur_item= fframeit_info->item_list.next;
       
	while(fframeit_info->cur_item)
	{
		if(!strcasecmp(fframeit_info->cur_item->Name, PhotoSet))
		{
			//image_entry = fframeit_info->cur_item;
			
			ret = frameit_fetch_photolists(fframeit_info);
            		if (ret < 0)
            		{
                		mpDebugPrint("error code: %d\n", fframeit_info->error_code);
                		return ret;
            		}
			break;
		}	
		fframeit_info->cur_item = fframeit_info->cur_item->next;
	}
	mpDebugPrint("33 frameit_info");
	return 0;


	
}


void frameit_exit(const char *base_dir)
{
   // free resources allocated for all photoset 
     
     item_entry_t     *tmp_entry;
 /* free resources allocated for all album */
 
    frameit_info.cur_item = frameit_info.cur_item->next;
    while (frameit_info.cur_item)
    {
        tmp_entry = frameit_info.cur_item;
        frameit_info.cur_item = frameit_info.cur_item->next;

        frameit_mfree(tmp_entry);
    }
}

// ------------------------------------------------------------------
BYTE * frameit_GetDeviceID()
{
	return frameit_info.DeviceId;
}

void frameit_SetDeviceID(BYTE * pbDeviceID)
{
	strncpy(frameit_info.DeviceId, pbDeviceID, MAX_URL);
}

void frameit_SetToken(BYTE * pbToken)
{
	strncpy(frameit_info.ClaimToken,pbToken, MAX_DATA);
}

int frameit_RequestToken(void)
{
	return frameit_GetClaimToken(&frameit_info);
}

#endif

