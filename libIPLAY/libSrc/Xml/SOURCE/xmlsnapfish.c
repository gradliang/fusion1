/**
 * @file
 *
 * Routines for access of Snapfish.com
 * 
 * Copyright (c) 2009 Magic Pixel Inc.
 * All rights reserved.
 */

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
#include "corelib.h"

#if NETWARE_ENABLE

#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "netfs.h"
#include "netfs_pri.h"
#include "netware.h"
#include "xmlsnapfish.h"
#include "global612.h"
#include "ndebug.h"

#include "../../lwip/include/net_sys.h"
#include "../../CURL/include/net_curl_curl.h"

#ifndef SUCCESS
#define SUCCESS 0
#endif

//#define TEST_API

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */
#if 1
#define snapfish_malloc(sz)   mm_malloc(sz)
#define snapfish_mfree(ptr)   mm_free(ptr)
#else
#define snapfish_malloc(sz)   ext_mem_malloc(sz)
#define snapfish_mfree(ptr)   ext_mem_free(ptr)

#endif

//note: fixed to use global my_write_func() for all xmlxxxxxx.c files!
size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf);

static snapfish_map_t   sf_info;
static struct netfs_file_entry jpeg_info;
extern Net_App_State App_State;

//------------------------------------------------------------------------------------------------
static void clientlogin_start_element_handler(void *user_data, const char *name, const char **attr)
{
    snapfish_info_t   *snapfish_info    = (snapfish_info_t *) user_data;
		
		if(snapfish_info->state == SNAPFISH_LOGIN)
		{
			if (!strcasecmp(name, "authcode"))
			{
				SF_Login_t *login_entry;

				login_entry = (SF_Login_t *)snapfish_malloc(sizeof(SF_Login_t));

				memset(login_entry, 0x00, sizeof(login_entry));

				snapfish_info->SF_Login = login_entry;
				snapfish_info->state = SNAPFISH_GOT_AUTHCODE;
			}
			else if(!strcasecmp(name, "podhost"))
			{	
				snapfish_info->state = SNAPFISH_GOT_PODHOST;
			}
			else if(!strcasecmp(name, "adhost"))
			{
				snapfish_info->state = SNAPFISH_GOT_ADHOST;
			}
			else if(!strcasecmp(name, "smarthost"))
			{
				snapfish_info->state = SNAPFISH_GOT_SMARTHOST;
			}
			else if(!strcasecmp(name, "priceversion"))
			{
				snapfish_info->state = SNAPFISH_GOT_PRICEVERSION;
			}
		}	
}

static void clientlogin_end_element_handler(void *user_data, const char *name)
{
    snapfish_info_t  *snapfish_info     = (snapfish_info_t *) user_data;
		//mpDebugPrint("222 snapfish_end_element_handler = %d",snapfish_info->state);
		if(snapfish_info->state == SNAPFISH_LOGIN)
		{
			snapfish_info->state = SNAPFISH_NULL;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_AUTHCODE)
		{
			snapfish_info->state = SNAPFISH_LOGIN;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_PODHOST)
		{
			snapfish_info->state = SNAPFISH_LOGIN;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_ADHOST)
		{
			snapfish_info->state = SNAPFISH_LOGIN;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_SMARTHOST)
		{
			snapfish_info->state = SNAPFISH_LOGIN;			
		}
		else if(snapfish_info->state == SNAPFISH_GOT_PRICEVERSION)
		{
			snapfish_info->state = SNAPFISH_LOGIN;
		}
}

static void clientlogin_content_handler(void *user_data, const char *s, int len)
{
    snapfish_info_t  *snapfish_info     = (snapfish_info_t *) user_data;

	//mpDebugPrint("333 snapfish_content_handler = %d",snapfish_info->state);
	
	switch(snapfish_info->state)
	{
		case SNAPFISH_GOT_AUTHCODE:
				strncpy(snapfish_info->authcode,s,len);
				MP_DEBUG1("snapfish_info->authcode = %s",snapfish_info->authcode);
				break;				
		case SNAPFISH_GOT_PODHOST:
				strncpy(snapfish_info->SF_Login->podhost,s,len);
				MP_DEBUG1("snapfish_info->SF_Login->podhost = %s",snapfish_info->SF_Login->podhost);
				break;
		case SNAPFISH_GOT_ADHOST:
				strncpy(snapfish_info->SF_Login->adhost,s,len);
				MP_DEBUG1("snapfish_info->SF_Login->adhost = %s",snapfish_info->SF_Login->adhost);
				break;
		case SNAPFISH_GOT_SMARTHOST:
				strncpy(snapfish_info->SF_Login->smarthost,s,len);
				MP_DEBUG1("snapfish_info->SF_Login->smarthost = %s",snapfish_info->SF_Login->smarthost);
				break;
		case SNAPFISH_GOT_PRICEVERSION:
				strncpy(snapfish_info->SF_Login->priceversion,s,len);
				MP_DEBUG1("snapfish_info->SF_Login->priceversion = %s",snapfish_info->SF_Login->priceversion);
				break;
	
	}


}
//----------------------------------------------------------------------------------------------
static void getalbums_start_element_handler(void *user_data, const char *name, const char **attr)
{
    snapfish_info_t   *snapfish_info    = (snapfish_info_t *) user_data;

		if(snapfish_info->state == SNAPFISH_RESP_GETALBUMS)
		{
			if(!strcasecmp(name, "SOAP-ENV:Body"))
			{

			}
			else if(!strcasecmp(name, "total"))
			{				
				snapfish_info->state = SNAPFISH_GOT_TOTAL;
			}
			else if(!strcasecmp(name, "owned"))
			{	
				snapfish_info->state = SNAPFISH_GOT_OWNED;
			}
			else if(!strcasecmp(name, "shared"))
			{
				snapfish_info->state = SNAPFISH_GOT_SHARED;
			}
			else if(!strcasecmp(name, "albums"))
			{
				snapfish_info->state = SNAPFISH_GETALBUMS;
			}
		}	
		else if(snapfish_info->state == SNAPFISH_GETALBUMS)
		{
			if(!strcasecmp(name, "album"))
		 	{
		 		SF_GetAlbums_t *getAlbums_entry;

				getAlbums_entry = (SF_GetAlbums_t *)snapfish_malloc(sizeof(SF_GetAlbums_t));

				memset(getAlbums_entry, 0, sizeof(SF_GetAlbums_t));
	
				if(snapfish_info->SF_GetAlbums == NULL)
				{
					snapfish_info->SF_GetAlbums = getAlbums_entry;
					snapfish_info->curr_Albums = getAlbums_entry;
				}
				else
				{
					snapfish_info->curr_Albums->next = getAlbums_entry;
					snapfish_info->curr_Albums =  getAlbums_entry;
				}
		 		snapfish_info->state = SNAPFISH_GETALBUM;
			}
		}
		else if(snapfish_info->state == SNAPFISH_GETALBUM)
		{
		 	if (!strcasecmp(name, "id"))
			{
				snapfish_info->state = SNAPFISH_GOT_ID;
			}
			else if(!strcasecmp(name, "name"))
			{
				snapfish_info->state = SNAPFISH_GOT_NAME;
			}
			else if(!strcasecmp(name, "acl"))
			{
				snapfish_info->state = SNAPFISH_GOT_ACL;
			}
			else if(!strcasecmp(name, "firsttnurl"))
			{
				snapfish_info->state = SNAPFISH_GOT_FSTTNURL;
			}
			else if(!strcasecmp(name, "numberImages"))
			{
				snapfish_info->state = SNAPFISH_GOT_NUMIMAGES;
			}			
		}
}

static void getalbums_end_element_handler(void *user_data, const char *name)
{
    snapfish_info_t  *snapfish_info     = (snapfish_info_t *) user_data;

		if(snapfish_info->state == SNAPFISH_RESP_GETALBUMS)
		{
			if(!strcasecmp(name,"SOAP-ENV:Body"))
				snapfish_info->state = SNAPFISH_NULL;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_TOTAL)
		{
			if(!strcasecmp(name, "total"))
				snapfish_info->state = SNAPFISH_RESP_GETALBUMS;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_OWNED)
		{
			if(!strcasecmp(name, "owned"))
				snapfish_info->state = SNAPFISH_RESP_GETALBUMS;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_SHARED)
		{
			if(!strcasecmp(name, "shared"))
				snapfish_info->state = SNAPFISH_RESP_GETALBUMS;
		}	
		else if(snapfish_info->state == SNAPFISH_GOT_ID)
		{
			if(!strcasecmp(name, "id"))
				snapfish_info->state = SNAPFISH_GETALBUM;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_NAME)
		{
			 if(!strcasecmp(name, "name"))
				snapfish_info->state = SNAPFISH_GETALBUM;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_ACL)
		{
			 if(!strcasecmp(name, "acl"))
				snapfish_info->state = SNAPFISH_GETALBUM;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_FSTTNURL)
		{
			 if(!strcasecmp(name, "firsttnurl"))
				snapfish_info->state = SNAPFISH_GETALBUM;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_NUMIMAGES)
		{
			if(!strcasecmp(name, "numberImages"))
				snapfish_info->state = SNAPFISH_GETALBUM;
		}
		else if(snapfish_info->state == SNAPFISH_GETALBUM)
		{
			if(!strcasecmp(name, "album"))
				snapfish_info->state = SNAPFISH_GETALBUMS;
		}
		
		else if(snapfish_info->state == SNAPFISH_GETALBUMS)
		{
			if(!strcasecmp(name, "albums"))
				snapfish_info->state = SNAPFISH_RESP_GETALBUMS;
		}
}

static char data[256];
static void getalbums_content_handler(void *user_data, const char *s, int len)
{
    snapfish_info_t  *snapfish_info     = (snapfish_info_t *) user_data;
	
	//mpDebugPrint("333 snapfish_content_handler = %d",snapfish_info->state);
	
	switch(snapfish_info->state)
	{
		case SNAPFISH_GOT_TOTAL:
			memset(data,0,256);
			strncpy(data,s,len);
			snapfish_info->SF_GetAlbums->total = atoi(data);
			MP_DEBUG1("snapfish_info->SF_GetAlbums->total = %d",snapfish_info->SF_GetAlbums->total);
			break;
		case SNAPFISH_GOT_OWNED:
			memset(data,0,256);
			strncpy(data,s,len);
			snapfish_info->SF_GetAlbums->owned = atoi(data);
			MP_DEBUG1("snapfish_info->SF_GetAlbums->owned = %d",snapfish_info->SF_GetAlbums->owned);
			break;
		case SNAPFISH_GOT_SHARED:
			memset(data,0,256);
			strncpy(data,s,len);
			snapfish_info->SF_GetAlbums->shared = atoi(data);
			MP_DEBUG1("snapfish_info->SF_GetAlbums->shared = %d",snapfish_info->SF_GetAlbums->shared);
			break;
		case SNAPFISH_GOT_ID:			
			memset(data,0,256);
			strncpy(data,s,len);
			snapfish_info->curr_Albums->id = atoi(data);
			MP_DEBUG1("snapfish_info->curr_Albums->id = %d",snapfish_info->curr_Albums->id);
			break;
		case SNAPFISH_GOT_NAME:			
			strncpy(snapfish_info->curr_Albums->name,s,len);
			MP_DEBUG1("snapfish_info->curr_Albums->name = %s",snapfish_info->curr_Albums->name);
			break;
		case SNAPFISH_GOT_ACL:
			memset(data,0,256);
			strncpy(data,s,len);
			snapfish_info->curr_Albums->acl= atoi(data);
			MP_DEBUG1("snapfish_info->curr_Albums->acl = %d",snapfish_info->curr_Albums->acl);
			break;
		case SNAPFISH_GOT_FSTTNURL:
			strncpy(snapfish_info->curr_Albums->firsttnurl,s,len);
			MP_DEBUG1("snapfish_info->curr_Albums->firsttnurl = %s",snapfish_info->curr_Albums->firsttnurl);
			break;
		case SNAPFISH_GOT_NUMIMAGES:
			memset(data,0,256);
			strncpy(data,s,len);
			snapfish_info->curr_Albums->numberImages= atoi(data);
			MP_DEBUG1("snapfish_info->curr_Albums->numberImages = %d",snapfish_info->curr_Albums->numberImages);
			break;		
	}

}

//-----------------------------------------------------------------------------------------------
static void registeruser_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void registeruser_end_element_handler(void *user_data, const char *name)
{
}

static void registeruser_content_handler(void *user_data, const char *s, int len)
{
}
//------------------------------------------------------------------------------------------------
static void clearshoppingcart_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void clearshoppingcart_end_element_handler(void *user_data, const char *name)
{
}

static void clearshoppingcart_content_handler(void *user_data, const char *s, int len)
{
}
//------------------------------------------------------------------------------------------------
static void getaccounthosts_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void getaccounthosts_end_element_handler(void *user_data, const char *name)
{
}

static void getaccounthosts_content_handler(void *user_data, const char *s, int len)
{
}
//------------------------------------------------------------------------------------------------
static void getalbuminfo_start_element_handler(void *user_data, const char *name, const char **attr)
{
	 snapfish_info_t   *snapfish_info    = (snapfish_info_t *) user_data;
	 
		if(snapfish_info->state == SNAPFISH_RESP_GETALBUMINFO)
		{
			if(!strcasecmp(name, "SOAP-ENV:Body"))
			{

			}
			else if(!strcasecmp(name, "id"))
			{				
				snapfish_info->state = SNAPFISH_GOT_ID;
			}
			else if(!strcasecmp(name, "name"))
			{	
				snapfish_info->state = SNAPFISH_GOT_NAME;
			}
			else if(!strcasecmp(name, "acl"))
			{
				snapfish_info->state = SNAPFISH_GOT_ACL;
			}
			else if(!strcasecmp(name, "pictures"))
			{
				snapfish_info->state = SNAPFISH_GET_PICTURES;
			}
		}	
		else if(snapfish_info->state == SNAPFISH_GET_PICTURES)
		{
			if(!strcasecmp(name, "picture"))
		 	{
		 		SF_GetAlbumInfo_t *getAlbumInfo_entry;

				getAlbumInfo_entry = (SF_GetAlbumInfo_t *)snapfish_malloc(sizeof(SF_GetAlbumInfo_t));

				memset(getAlbumInfo_entry, 0, sizeof(SF_GetAlbumInfo_t));
								
				if(snapfish_info->SF_GetAlbumInfo == NULL)
				{
					snapfish_info->SF_GetAlbumInfo = getAlbumInfo_entry;
					snapfish_info->curr_AlbumInfo = getAlbumInfo_entry;
				}
				else
				{
					snapfish_info->curr_AlbumInfo->next = getAlbumInfo_entry;
					snapfish_info->curr_AlbumInfo =  getAlbumInfo_entry;
				}
				memset(&jpeg_info, 0, sizeof(jpeg_info));	  		
				sprintf(jpeg_info.length,"%d",0);
			
		 		snapfish_info->state = SNAPFISH_GET_PICTURE;
			}
		}
		else if(snapfish_info->state == SNAPFISH_GET_PICTURE)
		{
		 	if (!strcasecmp(name, "id"))
			{
				snapfish_info->state = SNAPFISH_GOT_PIC_ID;
			}
			else if(!strcasecmp(name, "caption"))
			{
				snapfish_info->state = SNAPFISH_GOT_PIC_CAPTION;
			}
			else if(!strcasecmp(name, "tnurl"))
			{
				snapfish_info->state = SNAPFISH_GOT_PIC_TNURL;
			}
			else if(!strcasecmp(name, "srurl"))
			{
				snapfish_info->state = SNAPFISH_GOT_PIC_SRURL;
			}
			else if(!strcasecmp(name, "width"))
			{
				snapfish_info->state = SNAPFISH_GOT_PIC_WIDTH;
			}
			else if(!strcasecmp(name, "height"))
			{
				snapfish_info->state = SNAPFISH_GOT_PIC_HEIGHT;
			}	
		}
}

static void getalbuminfo_end_element_handler(void *user_data, const char *name)
{	
 	snapfish_info_t  *snapfish_info     = (snapfish_info_t *) user_data;

		if(snapfish_info->state == SNAPFISH_RESP_GETALBUMINFO)
		{
			if(!strcasecmp(name,"SOAP-ENV:Body"))
				snapfish_info->state = SNAPFISH_NULL;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_ID)
		{
			if(!strcasecmp(name, "id"))
				snapfish_info->state = SNAPFISH_RESP_GETALBUMINFO;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_NAME)
		{
			if(!strcasecmp(name, "name"))
				snapfish_info->state = SNAPFISH_RESP_GETALBUMINFO;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_ACL)
		{
			if(!strcasecmp(name, "acl"))
				snapfish_info->state = SNAPFISH_RESP_GETALBUMINFO;
		}	
		else if(snapfish_info->state == SNAPFISH_GOT_PIC_ID)
		{
			if(!strcasecmp(name, "id"))
				snapfish_info->state = SNAPFISH_GET_PICTURE;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_PIC_CAPTION)
		{
			 if(!strcasecmp(name, "caption"))
				snapfish_info->state = SNAPFISH_GET_PICTURE;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_PIC_TNURL)
		{
			 if(!strcasecmp(name, "tnurl"))
				snapfish_info->state = SNAPFISH_GET_PICTURE;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_PIC_SRURL)
		{
			 if(!strcasecmp(name, "srurl"))
				snapfish_info->state = SNAPFISH_GET_PICTURE;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_PIC_WIDTH)
		{
			if(!strcasecmp(name, "width"))
				snapfish_info->state = SNAPFISH_GET_PICTURE;
		}
		else if(snapfish_info->state == SNAPFISH_GOT_PIC_HEIGHT)
		{
			if(!strcasecmp(name, "height"))
				snapfish_info->state = SNAPFISH_GET_PICTURE;
		}
		else if(snapfish_info->state == SNAPFISH_GET_PICTURE)
		{
			if(!strcasecmp(name, "picture"))
				snapfish_info->state = SNAPFISH_GET_PICTURES;
		}
		else if(snapfish_info->state == SNAPFISH_GET_PICTURES)
		{
			if(!strcasecmp(name, "pictures"))
				snapfish_info->state = SNAPFISH_RESP_GETALBUMINFO;
		}
}

static void getalbuminfo_content_handler(void *user_data, const char *s, int len)
{
	snapfish_info_t  *snapfish_info     = (snapfish_info_t *) user_data;
	
	//mpDebugPrint("333 snapfish_content_handler = %d",snapfish_info->state);
	memset(data,0,256);
	
	switch(snapfish_info->state)
	{
		case SNAPFISH_GOT_ID:			
			strncpy(data,s,len);
			snapfish_info->curr_AlbumInfo->id = atoi(data);
			MP_DEBUG1("snapfish_info->curr_AlbumInfo->id = %d",snapfish_info->curr_AlbumInfo->id);
			break;
		case SNAPFISH_GOT_NAME:			
			strncpy(snapfish_info->curr_AlbumInfo->name,s,len);
			MP_DEBUG1("snapfish_info->curr_AlbumInfo->name = %s",snapfish_info->curr_AlbumInfo->name);
			break;
		case SNAPFISH_GOT_ACL:
			strncpy(data,s,len);
			snapfish_info->curr_AlbumInfo->acl= atoi(data);
			MP_DEBUG1("snapfish_info->curr_AlbumInfo->acl = %d",snapfish_info->curr_AlbumInfo->acl);
			break;
			
		case SNAPFISH_GOT_PIC_ID:
			strncpy(data,s,len);
			snapfish_info->curr_AlbumInfo->pic_id = atoi(data);
			MP_DEBUG1("snapfish_info->curr_AlbumInfo->pic_id = %d",snapfish_info->curr_AlbumInfo->pic_id);
			break;
		case SNAPFISH_GOT_PIC_CAPTION:
			strncpy(snapfish_info->curr_AlbumInfo->caption,s,len);
			MP_DEBUG1("snapfish_info->curr_AlbumInfo->caption = %s",snapfish_info->curr_AlbumInfo->caption);
			strncpy(jpeg_info.pathname,s,len);
			strcat(jpeg_info.pathname,".JPG");	
			break;
		case SNAPFISH_GOT_PIC_TNURL:
			strncpy(snapfish_info->curr_AlbumInfo->tnurl,s,len);
			MP_DEBUG1("snapfish_info->curr_AlbumInfo->tnurl = %s",snapfish_info->curr_AlbumInfo->tnurl);
			break;
		
		case SNAPFISH_GOT_PIC_SRURL:
			strncpy(snapfish_info->curr_AlbumInfo->srurl,s,len);
			MP_DEBUG1("snapfish_info->curr_AlbumInfo->srurl= %s",snapfish_info->curr_AlbumInfo->srurl);
			strncpy(jpeg_info.url,s,len);		
			Net_Xml_parseFileList(&jpeg_info);			
			break;
		case SNAPFISH_GOT_PIC_WIDTH:
			strncpy(data,s,len);
			snapfish_info->curr_AlbumInfo->width= atoi(data);
			MP_DEBUG1("snapfish_info->curr_AlbumInfo->width = %d",snapfish_info->curr_AlbumInfo->width);
			break;		
		case SNAPFISH_GOT_PIC_HEIGHT:
			strncpy(data,s,len);
			snapfish_info->curr_AlbumInfo->height= atoi(data);
			MP_DEBUG1("snapfish_info->curr_AlbumInfo->height = %d",snapfish_info->curr_AlbumInfo->height);
			break;		
	}

}


//------------------------------------------------------------------------------------------------
static void getalbumurl_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void getalbumurl_end_element_handler(void *user_data, const char *name)
{
}

static void getalbumurl_content_handler(void *user_data, const char *s, int len)
{
}	

//------------------------------------------------------------------------------------------------
static void getallprices_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void getallprices_end_element_handler(void *user_data, const char *name)
{
}

static void getallprices_content_handler(void *user_data, const char *s, int len)
{
}	

//------------------------------------------------------------------------------------------------
static void getbuddies_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void getbuddies_end_element_handler(void *user_data, const char *name)
{
}

static void getbuddies_content_handler(void *user_data, const char *s, int len)
{
}	

//------------------------------------------------------------------------------------------------
static void getphotogifts_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void getphotogifts_end_element_handler(void *user_data, const char *name)
{
}

static void getphotogifts_content_handler(void *user_data, const char *s, int len)
{
}	

	//------------------------------------------------------------------------------------------------
static void geturlalbum_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void geturlalbum_end_element_handler(void *user_data, const char *name)
{
}

static void geturlalbum_content_handler(void *user_data, const char *s, int len)
{
}

	//------------------------------------------------------------------------------------------------
static void getuserinfo_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void getuserinfo_end_element_handler(void *user_data, const char *name)
{
}

static void getuserinfo_content_handler(void *user_data, const char *s, int len)
{
}

	//------------------------------------------------------------------------------------------------
static void isshoppingcartempty_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void isshoppingcartempty_end_element_handler(void *user_data, const char *name)
{
}

static void isshoppingcartempty_content_handler(void *user_data, const char *s, int len)
{
}

	//------------------------------------------------------------------------------------------------
static void requestmailer_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void requestmailer_end_element_handler(void *user_data, const char *name)
{
}

static void requestmailer_content_handler(void *user_data, const char *s, int len)
{
}

//------------------------------------------------------------------------------------------------
static void updateuser_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void updateuser_end_element_handler(void *user_data, const char *name)
{
}

static void updateuser_content_handler(void *user_data, const char *s, int len)
{
}
//------------------------------------------------------------------------------------------------
static void upload_start_element_handler(void *user_data, const char *name, const char **attr)
{
}

static void upload_end_element_handler(void *user_data, const char *name)
{
}

static void upload_content_handler(void *user_data, const char *s, int len)
{
}

#if 1 //cj modify
char * username = "mpxwifi@gmail.com";
char * password = "1mpxwifi1";	

int snapfish_init(char *snapfish_url, char *base_dir)
{
	snapfish_info_t *snapfish_info = &sf_info.info;
    	int  ret = NETFS_OK;
//	BYTE *testemail = "mattytj@yahoo.com.tw";
//	BYTE *testpassord = "Img1107-0828g";
//	BYTE *testefname = "TC";
//	BYTE *testlname = "YTJ";
//	BYTE *albumid = "3298266";
//	BYTE *data;
    mpDebugPrint("snapfish_init '%s'", snapfish_url);

    memset(&sf_info, 0, sizeof(sf_info));

//    sf_info.base_dir = (char *) mem_malloc(strlen(base_dir) + 1);
    sf_info.base_dir = (char *) snapfish_malloc(strlen(base_dir) + 1);							
    if (sf_info.base_dir==NULL)
    {
        ret = -NETFS_NO_MEMORY;
    }
    strcpy(sf_info.base_dir, base_dir);
#if 0	
    sf_info.prev = snapfish_map.prev;
    sf_info.next = &snapfish_map;
    snapfish_map.prev->next = &sf_info;
    snapfish_map.prev = &sf_info;
#endif

    return snapfish_login(username, password);
	
//    return ret;
}


#else
int snapfish_init(char *snapfish_url, char *base_dir)
{
    int  ret = NETFS_OK;
	BYTE *testemail = "mattytj@yahoo.com.tw";
	BYTE *testpassord = "Img1107-0828g";
	BYTE *testefname = "TC";
	BYTE *testlname = "YTJ";
	BYTE *albumid = "3298266";
	BYTE *data;
    mpDebugPrint("snapfish_init '%s'", snapfish_url);

    memset(&sf_info, 0, sizeof(snapfish_map_t));

    sf_info.base_dir = (char *) mem_malloc(strlen(base_dir) + 1);
    if (sf_info.base_dir==NULL)
    {
        ret = -NETFS_NO_MEMORY;
    }
    strcpy(sf_info.base_dir, base_dir);
    sf_info.prev = snapfish_map.prev;
    sf_info.next = &snapfish_map;
    snapfish_map.prev->next = &sf_info;
    snapfish_map.prev = &sf_info;

    sf_info.info.cur_set = &sf_info.info.set_list;
	memcpy(&sf_info.info.usermail,testemail,strlen(testemail));
	memcpy(&sf_info.info.userpassword,testpassord,strlen(testpassord));
	sf_info.info.subscriberid = "1000000";
	mpDebugPrint("usermail %s",sf_info.info.usermail);
	mpDebugPrint("userpassword %s",sf_info.info.userpassword);
	if(snapfish_clientlogin(&sf_info.info)!=0)
		return NETFS_ERROR;
	#ifdef TEST_API
	//if(snapfish_getuserinfo(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	//if(snapfish_getaccounthosts(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	//if(snapfish_getallprices(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	
	if(snapfish_upload(&sf_info.info,"TEST")!=0)
		return NETFS_ERROR;
	if(snapfish_upload_file(&sf_info.info,data)!=0)
		return NETFS_ERROR;
	//memcpy(&sf_info.info.AlbumId,albumid,strlen(albumid));
	//if(snapfish_getalbuminfo(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	//snapfish_get_thumbnail_image(&sf_info.info);
	//snapfish_get_display_image(&sf_info.info);
	//if(snapfish_getbuddies(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	//if(snapfish_isshoppingcartempty(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	//memcpy(&sf_info.info.fname,testefname,strlen(testefname));
	//memcpy(&sf_info.info.lname,testlname,strlen(testlname));
	//if(snapfish_updateuser(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	//if(snapfish_getuserinfo(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	
	//if(snapfish_getphotogifts(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	
	
	//memcpy(&sf_info.info.AlbumId,albumid,strlen(albumid));
	//if(snapfish_geturlalbum(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	//if(snapfish_getalbumurl(&sf_info.info)!=0)
		//return NETFS_ERROR;
	//sf_info.info.type = "0";
	//if(snapfish_getalbums(&sf_info.info)!=0)
	//	return NETFS_ERROR;
		
	//if(snapfish_getalbuminfo(&sf_info.info)!=0)
	//	return NETFS_ERROR;
	#endif
	return ret;

}
#endif

void snapfish_exit(char *base_dir)
{
	snapfish_info_t *snapfish_info = &sf_info.info;

	SF_GetAlbums_t *temp_entry;
	
    	//mpDebugPrint("snapfish_exit");	
	     
      	/* free resources allocated for all albums */
 
    	snapfish_info->curr_Albums = snapfish_info->SF_GetAlbums;

	snapfish_info->SF_GetAlbumInfo = NULL;
	
   	while (snapfish_info->curr_Albums)
    	{
    		temp_entry = snapfish_info->curr_Albums;
       	snapfish_info->curr_Albums = snapfish_info->curr_Albums->next;
		snapfish_mfree(temp_entry);
    	}	
	snapfish_mfree(sf_info.base_dir);
	sf_info.base_dir = NULL;
}

int snapfish_parse_xml(const char * url
	                                 , const char * data
	                                 ,XML_StartElementHandler start_handler
						,XML_EndElementHandler end_handler
						,XML_CharacterDataHandler content_handler)
{
	int  ret = FAIL;
	int httpcode;
	
	Xml_BUFF_init(NET_RECVHTTP);		

	httpcode = mpx_easy_curl(url, data);
	
	if (httpcode == 200)
       {	
        	MPX_XML_Parse(&sf_info
						,start_handler
						,end_handler
						,content_handler);
		
		if(sf_info.info.state == SNAPFISH_ERROR)
			ret = FAIL;
		else
			ret = PASS;
       }	

    	Xml_BUFF_free(NET_RECVHTTP);    
    	return ret;
}

/* 
 *  ClientLogin API for Snapfish
 */
int snapfish_clientlogin(snapfish_info_t *snapfish_info)
{    
    char data[1024];
    char *url; 

        url = "http://www4.snapfish.com/externalapi/v2";
        snprintf(data,1024,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:Login xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<subscriberid>%d</subscriberid>"
			"<email>%s</email>"
			"<password>%s</password>"
			"</e:Login>"
			"</soap:Body></soap:Envelope>",
			snapfish_info->subscriberid,
			snapfish_info->email,
			snapfish_info->password);
        
       data[1023] = '\0';
	   
	snapfish_info->state = SNAPFISH_LOGIN;

	return snapfish_parse_xml(url
		  					, data
		              			, clientlogin_start_element_handler
		              			, clientlogin_end_element_handler
		              			, clientlogin_content_handler); 
			
}

/* 
 *  Register User API for Snapfish
 */
int snapfish_register_user(snapfish_info_t *snapfish_info)
{
    char data[1024];
    char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:RegisterUser xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<subscriberid>1000000</subscriberid>"
			"<deviceid>TST</deviceid>"
			"<UserInfo>"
			"<email>%s</email>"
			"<password>%s</password>"
			"<fname>%s</fname>"
			"<lname>%s</lname>"
			"</UserInfo>"
			"</e:RegisterUser>"
			"</soap:Body></soap:Envelope>",snapfish_info->email,snapfish_info->password,snapfish_info->fname,snapfish_info->lname);
        
        data[1023] = '\0';
		
	snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data	       
		              , registeruser_start_element_handler
		              , registeruser_end_element_handler
		              , registeruser_content_handler); 
}

/* 
 *  AddBuddy API for Snapfish
 */
int snapfish_addbuddy(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;

}
/* 
 *  ClearShoppingCart API for Snapfish
 */
int snapfish_clearshoppingcart(snapfish_info_t *snapfish_info )
{
    char data[1024];
    char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:ClearShoppingCart xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"</e:ClearShoppingCart>"
			"</soap:Body></soap:Envelope>",snapfish_info->authcode);
			
        data[1023] = '\0';
			
	snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data
		              , clearshoppingcart_start_element_handler
		              , clearshoppingcart_end_element_handler
		              , clearshoppingcart_content_handler); 		
}

/* 
 *  GetAccountHosts API for Snapfish
 */
int snapfish_getaccounthosts(snapfish_info_t *snapfish_info )
{
    char data[1024];
    	 char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:GetAccountHosts xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"</e:GetAccountHosts>"
			"</soap:Body></soap:Envelope>",snapfish_info->authcode);
			
        data[1023] = '\0';
			
	snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data		
		              , getaccounthosts_start_element_handler
		              , getaccounthosts_end_element_handler
		              , getaccounthosts_content_handler);			
				
}

/* 
 *  GetAlbumInfo API for Snapfish
 */
int snapfish_getalbuminfo(snapfish_info_t *snapfish_info)
{
    int ret = FAIL;
    int httpcode;	
    char data[1024];
    char *url; 

        url = "http://www4.snapfish.com/externalapi/v2";
		memset(data,0x00,1024);	
              snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
			"<soap:Body>"
			"<e:GetAlbumInfo xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"<AlbumId>%d</AlbumId>"
			"</e:GetAlbumInfo>"
			"</soap:Body>"
			"</soap:Envelope>",
			snapfish_info->authcode,
			snapfish_info->curr_Albums->id);
			
        data[1023] = '\0';
			
	char * pHeader1 = "SOAPAction: http://www.snapfish.com/externalapi/GetAlbumInfo";
	char * pHeader2 = "Content-Type: text/xml; charset=utf-8";

	Xml_BUFF_init(NET_RECVHTTP);	

	httpcode = mpx_curl_excute(url, data,pHeader1, pHeader2);			

        if (httpcode == 200)
        {
            sf_info.info.state = SNAPFISH_RESP_GETALBUMINFO;

	     MPX_XML_Parse(&sf_info
		              , getalbuminfo_start_element_handler
		              , getalbuminfo_end_element_handler
		              , getalbuminfo_content_handler); 			 
				
            if(sf_info.info.state == SNAPFISH_ERROR)
                ret = FAIL;
            else
			ret = PASS;
        }

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;
}

/* 
 *  GetAlbumURL API for Snapfish
 */
int snapfish_getalbumurl(snapfish_info_t *snapfish_info)
{
    char data[1024];
    	 char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:GetAlbumURL xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"<AlbumId>%s</AlbumId>"
			"</e:GetAlbumURL>"
			"</soap:Body></soap:Envelope>",snapfish_info->authcode,snapfish_info->AlbumId);
			
        data[1023] = '\0';
			
	snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data		
		              , getalbumurl_start_element_handler
		              , getalbumurl_end_element_handler
		              , getalbumurl_content_handler); 		
				
}

/* 
 *  GetAlbums API for Snapfish
 */
int snapfish_getalbums(snapfish_info_t *snapfish_info)
{
    int ret = FAIL;
    int httpcode;	
    char data[1024];
    char *url; 
    
        url = "http://www4.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:GetAlbums xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"<type>%d</type>"
			"</e:GetAlbums>"
			"</soap:Body></soap:Envelope>",snapfish_info->authcode,snapfish_info->type);
			
        data[1023] = '\0';
			
	char * pHeader1 = "SOAPAction: http://www.snapfish.com/externalapi/GetAlbums";
	char * pHeader2 = "Content-Type: text/xml; charset=utf-8";		

	Xml_BUFF_init(NET_RECVHTTP);	

        httpcode = mpx_curl_excute(url, data,pHeader1, pHeader2);	

        if (httpcode == 200)
        {
            snapfish_info->state = SNAPFISH_RESP_GETALBUMS;
	                
	     MPX_XML_Parse(&sf_info
		              , getalbums_start_element_handler
		              , getalbums_end_element_handler
		              , getalbums_content_handler); 				
       
            if(sf_info.info.state == SNAPFISH_ERROR)
                ret = FAIL;
            else
			ret = PASS;
    }

	
		Xml_BUFF_free(NET_RECVHTTP);
		return ret;
}

/* 
 *  GetAllPrices API for Snapfish
 */
int snapfish_getallprices(snapfish_info_t *snapfish_info)
{
    char data[1024];
    char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:GetAllPrices xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<subscriberid>%s</subscriberid>"
			"</e:GetAllPrices>"
			"</soap:Body></soap:Envelope>",snapfish_info->subscriberid);
			
        data[1023] = '\0';
			
	snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data
		              , getallprices_start_element_handler
		              , getallprices_end_element_handler
		              , getallprices_content_handler); 		
        
}

/* 
 *  GetAvilableShippingOptionCodes API for Snapfish
 */
int snapfish_getavilableshippingoptioncodes(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;

}
/* 
 *  GetBuddies API for Snapfish
 */
int snapfish_getbuddies(snapfish_info_t *snapfish_info )
{
    char data[1024];
    char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:GetBuddies xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"</e:GetBuddies>"
			"</soap:Body></soap:Envelope>",snapfish_info->authcode);
			
        data[1023] = '\0';
			
	snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data
		              			, getbuddies_start_element_handler
		              			, getbuddies_end_element_handler
		              			, getbuddies_content_handler); 	

}

/* 
 *  GetPhotogifts API for Snapfish
 */
int snapfish_getphotogifts(snapfish_info_t *snapfish_info)
{
    char data[1024];
    char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:GetPhotogifts xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<subscriberid>%s</subscriberid>"
			"</e:GetPhotogifts>"
			"</soap:Body></soap:Envelope>",snapfish_info->subscriberid);
			
        data[1023] = '\0';
			
    	snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data
		              , getphotogifts_start_element_handler
		              , getphotogifts_end_element_handler
		              , getphotogifts_content_handler);
		
}

/* 
 *  GetPhotogiftsPrices API for Snapfish
 */
int snapfish_getphotogiftsprices(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;

}
/* 
 *  GetPhotogiftsSentiments API for Snapfish
 */
int snapfish_getphotogiftssentiments(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;

}
/* 
 *  GetPrices API for Snapfish
 */
int snapfish_getprices(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;

}
/* 
 *  GetURLAlbum API for Snapfish
 */
int snapfish_geturlalbum(snapfish_info_t *snapfish_info)
{
    char data[1024];
    char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:GetURLAlbum xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"<AlbumId>%s</AlbumId>"
			"</e:GetURLAlbum>"
			"</soap:Body></soap:Envelope>",snapfish_info->authcode,snapfish_info->AlbumId);
			
        data[1023] = '\0';
			
    	snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data
		              , geturlalbum_start_element_handler
		              , geturlalbum_end_element_handler
		              , geturlalbum_content_handler); 	
}

/* 
 *  GetURLOrderPrints API for Snapfish
 */
int snapfish_geturlorderprints(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;

}

/* 
 *  GetUserInfo API for Snapfish
 */
int snapfish_getuserinfo(snapfish_info_t *snapfish_info)
{
    char data[1024];
    char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:GetUserInfo xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"</e:GetUserInfo>"
			"</soap:Body></soap:Envelope>",snapfish_info->authcode);
			
        data[1023] = '\0';
			
       snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data
		              , getuserinfo_start_element_handler
		              , getuserinfo_end_element_handler
		              , getuserinfo_content_handler); 	
}

/* 
 *  IsShoppingCartEmpty API for Snapfish
 */
int snapfish_isshoppingcartempty(snapfish_info_t *snapfish_info )
{
    char data[1024];
    char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:IsShoppingCartEmpty xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"</e:IsShoppingCartEmpty>"
			"</soap:Body></soap:Envelope>",snapfish_info->authcode);
			
        data[1023] = '\0';
			
        snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data
		              , isshoppingcartempty_start_element_handler
		              , isshoppingcartempty_end_element_handler
		              , isshoppingcartempty_content_handler);	
}

/* 
 *  PlacePhotogiftsOrder API for Snapfish
 */
int snapfish_placephotogiftsorder(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;

}
/* 
 *  PlaceReprintOrder API for Snapfish
 */
int snapfish_placereprintorder(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;

}
/* 
 *  PricePhotogiftsOrder API for Snapfish
 */
int snapfish_pricephotogiftsorder(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;

}

/* 
 *  PriceReprintOrder API for Snapfish
 */
int snapfish_pricereprintorder(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;
}

/* 
 *  RemoveBuddy API for Snapfish
 */
int snapfish_removebuddy(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;
}

/* 
 *  RequestMailer API for Snapfish
 */
int snapfish_requestmailer(snapfish_info_t *snapfish_info )
{
    	char data[1024];
    	char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:RequestMailers xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"</e:RequestMailers>"
			"</soap:Body></soap:Envelope>",snapfish_info->authcode);
			
        data[1023] = '\0';
			
       snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data
		              , requestmailer_start_element_handler
		              , requestmailer_end_element_handler
		              , requestmailer_content_handler);	

}

/* 
 *  ShareAlbums API for Snapfish
 */
int snapfish_sharealbums(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;

}
/* 
 *  SharePhoto API for Snapfish
 */
int snapfish_sharephoto(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;
}

/* 
 *  UpdateCredits API for Snapfish
 */
int snapfish_updatecredits(snapfish_info_t *snapfish_info)
{
    int                     ret = 0;
    return ret;
}

/* 
 *  UpdateUser API for Snapfish
 */
int snapfish_updateuser(snapfish_info_t *snapfish_info)
{
    	char data[1024];
    char *url; 

        url = "http://www.sfus7.qa.snapfish.com/externalapi/v2";
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>"
			"<e:UpdateUser xmlns:e=\"http://www.snapfish.com/externalapi\">"
			"<authcode>%s</authcode>"
			"<UserInfo>"
			"<email>%s</email>"
			"<fname>%s</fname>"
			"<lname>%s</lname>"
			"<password>%s</password>"
			"</UserInfo>"
			"</e:UpdateUser>"
			"</soap:Body></soap:Envelope>",snapfish_info->authcode,snapfish_info->email,snapfish_info->fname,snapfish_info->lname,snapfish_info->password);
			
        data[1023] = '\0';
			
	snapfish_info->state = SNAPFISH_NULL;	

	return snapfish_parse_xml(url
		  					, data
		              , updateuser_start_element_handler
		              , updateuser_end_element_handler
		              , updateuser_content_handler);		
				
}

/* 
 *  Upload API for Snapfish
 */
int snapfish_upload(snapfish_info_t *snapfish_info,BYTE *filename)
{
    int 					ret = 0;
    int 					len;
    char url[1024];

    int httpcode;
		
    Xml_BUFF_init(NET_RECVHTTP);	
		
    snprintf(url,1024,"http://upload1.sfus7.qa.snapfish.com/startsession.suup?AlbumCaption=\"%s\"&ExpectedImages=1&Src=TST&authcode=%s",filename,snapfish_info->authcode);
	
    mpDebugPrint("url %s",url);
	
    ret = Net_Recv_Data(url,NET_RECVHTTP,0,0);

	sf_info.info.state = SNAPFISH_NULL;		
	MPX_XML_Parse(&sf_info
		              , upload_start_element_handler
		              , upload_end_element_handler
		              , upload_content_handler);

exit:

    Xml_BUFF_free(NET_RECVHTTP);
    return 0;
}

#if 0
int snapfish_upload_file(snapfish_info_t *snapfish_info,BYTE *file)
{
    void *curl;
    CURLcode res;
    int 					ret = PASS;
    int 					len;
    char *data, *end; 
    char *url,*sent_url; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    XML_Parser	   parser;

    char *bigger_buff;

    Xml_BUFF_init(NET_RECVHTTP);	
    mpDebugPrint("NETFS_SNAPFISH: snapfish_getuserinfo ");
		
    //DecString(str,WpsPinCode,8,0);

    data = snapfish_malloc(1024);
    if (data == NULL)
        goto exit;
		
    url = snapfish_malloc(1024);
    if (url == NULL)
        goto exit;
    curl = curl_easy_init();
    if(curl) {
        snprintf(url,1024,"http://upload1.sfus7.qa.snapfish.com/uploadimage.suup?AlbumId=\"%s\"&SequenceNumber=1&Src=TST&authcode=%s&SessionId=%s",
                snapfish_info->AlbumId,snapfish_info->authcode,snapfish_info->sessionid);
        mpDebugPrint("url %s",url);
        memcpy(data,file,1024);
        //NetAsciiDump(data, strlen(data));
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); /* TODO */

        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

        res = curl_easy_perform(curl);
        TaskYield();

        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
            httpcode = 0;

        /* always cleanup */
        curl_easy_cleanup(curl);
			
        snapfish_mfree(url);
        snapfish_mfree(data);

        MP_DEBUG("NETFS_SNAPFISH: http resp code=%d", httpcode);
        if (httpcode == 200)
        {
            /* Parse XML */
            parser = XML_ParserCreate(NULL);
            sf_info.info.state = SNAPFISH_NULL;
				
            XML_SetUserData(parser,&sf_info);
            XML_SetElementHandler(parser, snapfish_start_element_handler, snapfish_end_element_handler);
            XML_SetCharacterDataHandler(parser, snapfish_content_handler);

#if 0 //original code
            ptr = App_State.XML_BUF;
            while(ptr != NULL)
            {		 
                data = ptr->BUFF;
                len = ptr->buff_len;
                data[len] = '\0';
                mpDebugPrint("len = %d ",len); 
					
                NetAsciiDump(data, strlen(data));
                if (XML_Parse(parser, data, len, 0) == XML_STATUS_ERROR)
                {
                    mpDebugPrint("XML_STATUS_ERROR"); 
                }
                ptr = ptr->link;		
					
                TaskYield();
            }
#else //new code
            bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
            MP_DEBUG("snapfish_upload_file: got total data len = %d", len);

            if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
            {
                MP_DEBUG3("snapfish_upload_file: %s at line %d, column %d\n",
                    XML_ErrorString(XML_GetErrorCode(parser)),
                    XML_GetCurrentLineNumber(parser),
                    XML_GetCurrentColumnNumber(parser));
            }
#endif

            if(sf_info.info.state == SNAPFISH_ERROR)
                ret = FAIL;
				
            XML_Parse(parser, data, 0, 1);
        }
        else if (httpcode > 0)					/* TODO */
        {
            mpDebugPrint("NETFS_SNAPFISH: http error resp code=%d", httpcode);
            ptr = App_State.XML_BUF;
            if (httpcode == 403)					/* 403 login failed */
            {
                mpDebugPrint("NETFS_SNAPFISH: Server Error %d login failed", httpcode);
            }
            else
            {
                mpDebugPrint("NETFS_SNAPFISH: Server Error %d login failed", httpcode);
            }
        }
    }

exit:
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;
}

#endif

#if 0
/* 
 *  Get Image API for Snapfish
 */
int snapfish_get_thumbnail_image(snapfish_info_t *snapfish_info)
{
		int   ret = 0;
		char *data, *end; 
		int len;

		Xml_BUFF_init(NET_RECVHTTP);	
		mpDebugPrint("NETFS_SNAPFISH: snapfish_get_thumbnail_image ");
		if (snapfish_info->SF_GetAlbumInfo->tnurl== NULL)
			goto exit;
		mpDebugPrint("snapfish_info->tnurl %s",snapfish_info->SF_GetAlbumInfo->tnurl);
		
		len = Net_Recv_Data(snapfish_info->SF_GetAlbumInfo->tnurl,NET_RECVHTTP,0,0);
			
		//NetPacketDump(App_State.XML_BUF->BUFF, len);

	exit:
		Xml_BUFF_free(NET_RECVHTTP);	
			
		return ret;

}

/* 
 *  Get Image API for Snapfish
 */
int snapfish_get_display_image(snapfish_info_t *snapfish_info)
{
		int   ret = 0;
		char *data, *end; 
		int len;

		Xml_BUFF_init(NET_RECVHTTP);	
		mpDebugPrint("NETFS_SNAPFISH: snapfish_get_display_image ");
		if (snapfish_info->SF_GetAlbumInfo->srurl== NULL)
			goto exit;
		mpDebugPrint("snapfish_info->srurl %s",snapfish_info->SF_GetAlbumInfo->srurl);
		
		len = Net_Recv_Data(snapfish_info->SF_GetAlbumInfo->srurl,NET_RECVHTTP,0,0);
			
		//NetPacketDump(App_State.XML_BUF->BUFF, len);

	exit:
		Xml_BUFF_free(NET_RECVHTTP);	
			
		return ret;

}
#endif

int snapfish_login(BYTE *username,BYTE *password)
{

	snapfish_info_t *snapfish_info = &sf_info.info;
	int  ret = NETFS_OK;

	strncpy(snapfish_info->email,username,strlen(username));
	strncpy(snapfish_info->password,password,strlen(password));
	snapfish_info->subscriberid = 1000000;
	MP_DEBUG3("sf_info.info.email = %s,sf_info.info.password = %s,sf_info.info.subscriberid = %d",
				    snapfish_info->email,
		                  snapfish_info->password,
		                  snapfish_info->subscriberid);


	if(snapfish_clientlogin(snapfish_info)!=0)
	{
		return -NETFS_ERROR;
	}
	mpDebugPrint("2 %s", __func__);
	
	sf_info.info.type = 0; //0 = all albums 1 = all owned albums 2 = all shared albums default is 0. 
	if(snapfish_getalbums(snapfish_info)!=0)
	{
		return -NETFS_ERROR;
	}
	mpDebugPrint("3 %s", __func__);		
	
	snapfish_Request_getalbums();
	mpDebugPrint("4 %s", __func__);
	return ret;
}

int snapfish_Request_getalbums(void)
{
	snapfish_info_t *snapfish_info = &sf_info.info;
	SF_GetAlbums_t *temp_entry,*temp_entry1;
	
	int  ret = NETFS_OK;
	int count ;
	
	temp_entry = snapfish_info->SF_GetAlbums;
	count = 0;
	
    	while (temp_entry)
       {
  		Net_Xml_PhotoSetList(temp_entry->name,count);	
		temp_entry = temp_entry->next;		
		count++;
	}
	Net_PhotoSet_SetCount(count);	

	return ret;
}

int snapfish_PhotoList_Get(BYTE *PhotoSet)
{
	snapfish_info_t *snapfish_info = &sf_info.info;
		
	int  ret = NETFS_OK;
	int count ;
	
	snapfish_info->curr_Albums = snapfish_info->SF_GetAlbums;
	
    	while (snapfish_info->curr_Albums)
       {
		if(!strcasecmp(snapfish_info->curr_Albums->name, PhotoSet))
		{
			ret = snapfish_getalbuminfo(snapfish_info);
			
            		if (ret < 0)
            		{
                		//mpDebugPrint("error code: %d\n", ->error_code);
				mpDebugPrint("error code:");
				return ret;
            		}
			break;
		}	
		snapfish_info->curr_Albums = snapfish_info->curr_Albums->next;
    	}

	return ret;
}

#endif

