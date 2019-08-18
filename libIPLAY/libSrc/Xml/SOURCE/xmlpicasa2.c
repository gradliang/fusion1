/**
 * @file
 *
 * This is an implementation of Picasa Web Albums Data API.
 * 
 * The following APIs are supported:
 *
 *  1) Authentication - Support for ClientLogin only.  We don't support AuthSub.
 *
 *          picasa_ClientLogin()
 *
 *     This function must be called first to get a authentication token, which
 *     will be needed in other APIs below.
 *
 *  2) Request a feed that lists all of the albums (private and public ) belonging 
 *     to an user.
 *
 *          picasa_fetch_album_list2()
 *
 *     Equivalent HTTP Request:
 *
 *          GET http://picasaweb.google.com/data/feed/api/user/default
 *
 *  3) Request a feed that lists all of the photos in an album with id <albumID>,
 *     belonging to a user <userID>.
 *
 *          picasa_fetch_album2()
 *
 *     Equivalent HTTP Request:
 *
 *          GET http://picasaweb.google.com/data/feed/api/user/<userID>/albumid/<albumID>
 *
 * picasa_init() is one example that uses picasa_ClientLogin and 
 * picasa_fetch_album_list2.  But the user's password is hard-coded.  For actual
 * applications, the password has to be passed from calling function.
 *
 * Copyright (c) 2008 Magic Pixel Inc.
 * All rights reserved.
 */

// define this module show debug message or not,  0 : disable, 1 : enable
#define LOCAL_DEBUG_ENABLE 0
#include "corelib.h"

#if NETWARE_ENABLE
#if Make_CURL

#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "netfs.h"
#include "netfs_pri.h"
#include "xmlpicasa.h"
#include "ndebug.h"

#include "..\..\lwip\include\net_sys.h"
#include "..\..\CURL\include\net_curl_curl.h"

static struct netfs_file_entry jpeg_info;

static char     picasa_api_url[MAX_URL];
static picasa_info_t   picasa_info;
static const char   *picasa_base_dir;

#define PICASA_HOST "picasaweb.google.com"

extern Net_App_State App_State;

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define picasa_malloc(sz)   mm_malloc(sz)
#define picasa_mfree(ptr)   mm_free(ptr)

int picasa_fetch_album_list2(picasa_info_t *picasa_info);
int picasa_fetch_album2(picasa_info_t *picasa_info);

//note: fixed to use global my_write_func() for all xmlxxxxxx.c files!
size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf);
BYTE *Merge_ListData_to_SingleBuffer(XML_BUFF_link_t *list_head, int *total_data_len);

int picasa_ClientLogin(picasa_info_t *picasa_info, char *email, char *passwd, char *source, char *errstring);

#define CRLF "\r\n"

int Picasa_PhotoList_Get(BYTE *PhotoSet)
{
	picasa_info_t     *ppicasa_info;
	int     ret;
    int     count;

	EnableNetWareTask();
	ppicasa_info = &picasa_info;  	
    ppicasa_info->cur_album = ppicasa_info->album_list.next;
       
	while(ppicasa_info->cur_album)
	{
		if(!strcasecmp(ppicasa_info->cur_album->title, PhotoSet))
		{
#if 1
            ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
            struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
            DWORD dwOpMode = psSysConfig->dwCurrentOpMode;
            psFileBrowser->dwFileListIndex[dwOpMode] = 0;
            psFileBrowser->dwFileListCount[dwOpMode] = 0;
            psFileBrowser->dwFileListAddress[dwOpMode] = 0;
            psFileBrowser->dwFileListAddress[dwOpMode] = (DWORD) &psFileBrowser->sSearchFileList[0];
#endif
			
		#if ATOM_MODE
			//ret = picasa_atom_edit_album(ppicasa_info,ppicasa_info->username,ppicasa_info->cur_album->id,ppicasa_info->cur_album->title,"test0083");
			ret = picasa_atom_fetch_album(ppicasa_info);
		#else
			ret = picasa_fetch_album2(ppicasa_info);
		#endif
			
            if (ret < 0)
            {
                mpDebugPrint("error code: %d\n", ppicasa_info->error_code);
                return ret;
            }
#if 1
            psFileBrowser->dwImgAndMovCurIndex = psFileBrowser->dwFileListIndex[dwOpMode];
            psFileBrowser->dwImgAndMovTotalFile = psFileBrowser->dwFileListCount[dwOpMode];
            psFileBrowser->sImgAndMovFileList = (ST_SEARCH_INFO *)psFileBrowser->dwFileListAddress[dwOpMode];
#endif
            break;
        }	
		ppicasa_info->cur_album = ppicasa_info->cur_album->next;
	}
	return 0;
}

#if 1 // jeffery 20081208, 

#define MAX_PASSWORD 64
static BYTE Picasa_UserPassword[MAX_PASSWORD];

void Picasa_SetUser_Password(BYTE *str)
{
	if (! str)
		return;

	int len = sizeof(Picasa_UserPassword);
 	strncpy(Picasa_UserPassword, str, len);
	Picasa_UserPassword[len-1] = 0;	
}
#endif

/*
 * IMPORTANT NOTE: This function requires a valid password for Picasa user
 * account.  The 3rd argument to picasa_ClientLogin is the password, 
 * which must be supplied to make the login work.
 */
int picasa_init(const char *username, const char *base_dir)
{
    album_entry_t     *tmp_entry;
    int     ret;
    int     count;
    char    error[32];

    MP_DEBUG1("request picasa photo for '%s'\n", username);

    memset(&picasa_info, 0, sizeof(picasa_info_t));
    strncpy(picasa_info.username, username, MAX_USERNAME);
    picasa_info.cur_album = &picasa_info.album_list;
    picasa_base_dir = base_dir;

    /* ======================================================================== 
     * IMPORTANT NOTE: The 3rd argument (marked XXXXXX) to picasa_ClientLogin 
     * is the password, which must be supplied to make the login work.
     *======================================================================== */
//    ret = picasa_ClientLogin(&picasa_info, username, "XXXXXX", "MapicPixel-DPF-0.1", error);
//    ret = picasa_ClientLogin(&picasa_info, username, "duwc1204", "MapicPixel-DPF-0.1", error);
    //ret = picasa_ClientLogin(&picasa_info, username, Picasa_UserPassword, "MapicPixel-DPF-0.1", error);
    if(!strcmp(username,"mpxwifi"))
    	ret = picasa_ClientLogin(&picasa_info, username, "1mpxwifi1", "MapicPixel-DPF-0.1", error);
    else
    	ret = picasa_ClientLogin(&picasa_info, username, "2130482", "MapicPixel-DPF-0.1", error);

    if (ret < 0)
    {
        if (ret == -2)
            mpDebugPrint("Login failed due to '%s'\n", error);
        return ret;
    }
#if ATOM_MODE
    MP_DEBUG("1 picasa_atom_fetch_album @@@@@@@@@@@@@@@@@@@@@@");
	ret = picasa_atom_fetch_album_list(&picasa_info);
    //ret = picasa_atom_add_album(&picasa_info,username,"20090617");
	//ret = picasa_atom_delete_album(&picasa_info,picasa_info.cur_album->edit);
	//ret = picasa_atom_edit_album(&picasa_info,picasa_info.cur_album->title,picasa_info.cur_album->edit);
#else
    MP_DEBUG("1 picasa_fetch_album_list2 @@@@@@@@@@@@@@@@@@@@@@");
    ret = picasa_fetch_album_list2(&picasa_info);
#endif
    if (ret < 0)
    {
        MP_ALERT("picasa_fetch_album_list2(): error code: %d\n", picasa_info.error_code);
        return ret;
    }

    count = 0;
    picasa_info.cur_album = picasa_info.album_list.next;
    tmp_entry = picasa_info.cur_album;

    if (tmp_entry)
    {
        while (tmp_entry)
        {
            MP_DEBUG2("Fetch photos from '%s' with id '%s'\n",tmp_entry->title,tmp_entry->id);
			MP_DEBUG1("counter %d",count);
            Net_Xml_PhotoSetList(tmp_entry->title,count);	
            count ++;
            tmp_entry = tmp_entry->next;
        }
        Net_PhotoSet_SetCount(count);	
    }
    else
    {
        mpDebugPrint("No album.\n");
    }

    return 0;
}

void picasa_exit(const char *base_dir)
{
   album_entry_t     *tmp_entry;
 /* free resources allocated for all album */
 
    picasa_info.cur_album = picasa_info.album_list.next;
    while (picasa_info.cur_album)
    {
        tmp_entry = picasa_info.cur_album;
        picasa_info.cur_album = picasa_info.cur_album->next;

        picasa_mfree(tmp_entry);
    }
}

/**
 *  Get photo set of specified user id
 */
static void album_list_content_handler(void *user_data, const char *s, int len)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
    char    *title;
    char    *id;
    int     i;

    if (picasa_info->state == PICASA_ALBUM_LIST_TITLE && picasa_info->cur_album)
    {
        title = picasa_info->cur_album->title;
        len = (len >= MAX_TITLE)? (MAX_TITLE-1) :len;
        memcpy(title, s,len);
        title[len] = '\0';
        MP_DEBUG("NETFS_PICASA: title %s", title);
    }
    else if (picasa_info->state == PICASA_ALBUM_LIST_ID && picasa_info->cur_album)
    {
        id = picasa_info->cur_album->id;
        len = (len >= MAX_ALBUMID)? (MAX_ALBUMID-1) :len;
        memcpy(id, s,len);
        id[len] = '\0';
        MP_DEBUG("NETFS_PICASA: id %s", id);
    }
}

static void album_list_start_element_handler(void *user_data, const char *name, const char **attr)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;

    if (picasa_info->state == PICASA_ALBUM_LIST_INIT)
    {
        if (!strcasecmp(name, "item"))
        {
            /* append album list */
            album_entry_t       *album_entry;
            
            picasa_info->state = PICASA_ALBUM_LIST_FOUND;
            album_entry = (album_entry_t *) picasa_malloc(sizeof(album_entry_t));
            MP_ASSERT(album_entry);
            memset(album_entry, 0, sizeof(album_entry_t));

            picasa_info->cur_album->next = album_entry;
            picasa_info->cur_album = album_entry;
        }
    }
    else if (picasa_info->state == PICASA_ALBUM_LIST_FOUND)
    {
        if (!strcasecmp(name, "title"))
            picasa_info->state = PICASA_ALBUM_LIST_TITLE;
        if (!strcasecmp(name, "gphoto:id"))
            picasa_info->state = PICASA_ALBUM_LIST_ID;
            
    }
}

static void album_list_end_element_handler(void *user_data, const char *name)
{
    picasa_info_t  *picasa_info     = (picasa_info_t *) user_data;

    if (picasa_info->state == PICASA_ALBUM_LIST_TITLE)
    {
        if (!strcasecmp(name, "title"))
            picasa_info->state = PICASA_ALBUM_LIST_FOUND;

    }
    else if (picasa_info->state == PICASA_ALBUM_LIST_ID)
    {
        if (!strcasecmp(name, "gphoto:id"))
            picasa_info->state = PICASA_ALBUM_LIST_FOUND;
    }
    else if (picasa_info->state == PICASA_ALBUM_LIST_FOUND)
    {
        if (!strcasecmp(name, "item"))
            picasa_info->state = PICASA_ALBUM_LIST_INIT;
    }
}

#if ATOM_MODE
/**
 *  Get photo set of specified user id
 */
static void atom_album_list_content_handler(void *user_data, const char *s, int len)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
    char    *title;
    char    *id;
    int     i;

	//mpDebugPrint("atom_album_list_content_handler %s",s);

    if (picasa_info->state == PICASA_ALBUM_LIST_TITLE&& picasa_info->cur_album)
    {
        title = picasa_info->cur_album->title;
        len = (len >= MAX_TITLE)? (MAX_TITLE-1) :len;
        memcpy(title, s,len);
        title[len] = '\0';
        mpDebugPrint("NETFS_PICASA: title %s", title);
		picasa_info->state = PICASA_ATOM_ALBUM_ENTRY;
    }
    else if (picasa_info->state == PICASA_ALBUM_LIST_ID && picasa_info->cur_album)
    {
        id = picasa_info->cur_album->id;
        len = (len >= MAX_ALBUMID)? (MAX_ALBUMID-1) :len;
        memcpy(id, s,len);
        id[len] = '\0';
        //mpDebugPrint("NETFS_PICASA: id %s", id);
		picasa_info->state = PICASA_ATOM_ALBUM_ENTRY;
    }
}

static void atom_album_list_start_element_handler(void *user_data, const char *name, const char **attr)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
	BYTE cpy_url=0;
	int i;
	
	//mpDebugPrint("start_element_handler %s  %s",name,*attr);

    if (picasa_info->state == PICASA_ALBUM_LIST_INIT)
    {
        if(!strcasecmp(name, "entry"))
        {
		    //mpDebugPrint("start entry");
        	picasa_info->state = PICASA_ATOM_ALBUM_ENTRY;
        }
		
    }
	else if(picasa_info->state == PICASA_ATOM_ALBUM_ENTRY)
	{
		if (!strcasecmp(name, "link"))
		{
			while (*attr)
			{
			
		    	//mpDebugPrint(" %s ",*attr);
				
				if (!strcasecmp(*attr, "edit"))
				{
				
					/* append album list */
					album_entry_t		*album_entry;
					album_entry = (album_entry_t *) picasa_malloc(sizeof(album_entry_t));
					MP_ASSERT(album_entry);
					memset(album_entry, 0, sizeof(album_entry_t));
					
					picasa_info->cur_album->next = album_entry;
					picasa_info->cur_album = album_entry;
		    		//mpDebugPrint("cpy_url = 1");
					cpy_url = 1;
				}
				if((!strcasecmp(*attr, "href"))&&cpy_url)
				{
				  attr ++;
				  strcpy(picasa_info->cur_album->edit,*attr);
				  
				  for(i=0;i<strlen(picasa_info->cur_album->edit);i++)
				  {
				    if(picasa_info->cur_album->edit[i] == '?')
				    {
				        memset(picasa_info->cur_album->edit+i,0x00,strlen(picasa_info->cur_album->edit)-i);
				        picasa_info->cur_album->edit[i] = '\0';
						break; 
				    }
				  }
				  //mpDebugPrint("edit %s ",picasa_info->album_list.edit);
				}
				attr ++;
			}
			
		}
	    if (!strcasecmp(name, "gphoto:id"))
	   	{
			picasa_info->state = PICASA_ALBUM_LIST_ID;
	    }
		if (!strcasecmp(name, "media:title"))
		{

			//mpDebugPrint("title %s",*attr);
			
			picasa_info->state = PICASA_ALBUM_LIST_TITLE;
		}
	   
	}
}

static void atom_album_list_end_element_handler(void *user_data, const char *name)
{
    picasa_info_t  *picasa_info     = (picasa_info_t *) user_data;
	
	//mpDebugPrint(" end_element_handler %s",name);

    if (picasa_info->state == PICASA_ATOM_ALBUM_ENTRY)
    {
        if (!strcasecmp(name, "entry"))
        {
		    //mpDebugPrint("\n end entry\n");
			picasa_info->state = PICASA_ALBUM_LIST_INIT;
        }

    }
}
#endif

int picasa_fetch_album_list2(picasa_info_t *picasa_info)
{
    int                     ret = 0; 
    char header[512]; 
    int httpcode;
	
    Xml_BUFF_init(NET_RECVHTTP);	

    snprintf(picasa_api_url, MAX_URL, 
             "http://%s/data/feed/api/user/default?alt=rss",
             PICASA_HOST);
	
        snprintf(header,512, "Authorization: GoogleLogin auth=%s", picasa_info->auth);
        header[511] = '\0';

	httpcode = mpx_curl_excute(picasa_api_url, NULL, header, NULL);
        
        if (httpcode == 200)
        {
            /* Parse XML */          
            picasa_info->state = PICASA_ALBUM_LIST_INIT;

		MPX_XML_Parse(picasa_info
			, album_list_start_element_handler 	
			, album_list_end_element_handler
			, album_list_content_handler);  		

            if (picasa_info->error_code > 0)
            {
                ret = -NETFS_APP_ERROR;                
            }
        }  
        else
        {
            ret = -1;
        }
    
    	Xml_BUFF_free(NET_RECVHTTP);
   	return ret;
}

static
int my_trace(CURL *handle, curl_infotype type,
             unsigned char *data, size_t size,
             void *userp)
{
  {
      char *s;
        s = picasa_malloc(size+1);
        if (!s)
            return 0;
    switch(type) {
    case CURLINFO_HEADER_OUT:
        memcpy(s, data, size);
        s[size] = '\0';
//          mpDebugPrint("header out(%d): %s\n", size, s);
          mpDebugPrint("header out(%d):", size);
            NetAsciiDump(data, size);
      break;
    default: /* nada */
      break;
    }

    return 0;
  }

  return 0;
}

/**
 *  Get photo list of specified photo set
 */
static void album_content_handler(void *user_data, const char *s, int len)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;

    if (picasa_info->state == PICASA_ALBUM_PHOTO_TITLE)
    {
		strncpy(jpeg_info.pathname,s,len);
		MP_DEBUG1("jpeg_info.pathname = %s",jpeg_info.pathname);
 	
    }

}

static void album_start_element_handler(void *user_data, const char *name, const char **attr)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
    const char      *url_value      = NULL;
    const char      *file_size_value= NULL;

    if (picasa_info->state == PICASA_ALBUM_INIT)
    {
        if (!strcasecmp(name, "item"))
        {
            memset(&jpeg_info, 0, sizeof(jpeg_info));
            picasa_info->state = PICASA_ALBUM_PHOTO_FOUND;
        }
    }
    else if (picasa_info->state == PICASA_ALBUM_PHOTO_FOUND)
    {
        if (!strcasecmp(name, "title"))
        {
            picasa_info->state = PICASA_ALBUM_PHOTO_TITLE;
	    //cj mask 031809		
            //snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME, "%s",picasa_info->cur_album->title);
        }
        else if (!strcasecmp(name, "media:content"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "url"))
                {
                    attr ++;
                    url_value = *attr;
                }
                else if (!strcasecmp(*attr, "fileSize"))
                {
                    attr ++;
                    file_size_value = *attr;
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }
            
            if (file_size_value)
            {
                strncat(jpeg_info.length, file_size_value, 20-1);
            }
            if (url_value)
            {
            	  //cj modify 031809
                sprintf(jpeg_info.length,"%d",0);
		  MP_DEBUG2("url_value = %s, len = %d",url_value,strlen(url_value));
		  strncpy(jpeg_info.url,url_value,strlen(url_value));		  
 		  Net_Xml_parseFileList(&jpeg_info);		
            }
        }

    }
}

static void album_end_element_handler(void *user_data, const char *name)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
    int len0= 0 , len1 =0 , len2 =0;	
    BYTE *str1,*str2,*str3;
    BYTE filesz[20];
   	

    if (picasa_info->state == PICASA_ALBUM_PHOTO_TITLE)
    {
        if (!strcasecmp(name, "title"))
        {
            //snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,"/%s/%s",picasa_info->cur_album->title,title_value);
            picasa_info->state = PICASA_ALBUM_PHOTO_FOUND;
        }
    }
    else if (picasa_info->state == PICASA_ALBUM_PHOTO_FOUND)
    {
        if (!strcasecmp(name, "item"))
        {
        //cj modify 031809
              //MP_DEBUG2("add photo '%s' => '%s'\n",jpeg_info.pathname, jpeg_info.url);
		//sprintf(jpeg_info.length,"%d",0);	
#if 0 //cj not need to add file to netfs		
		//netfs_add_file(&jpeg_info);
#endif
		//Net_Xml_parseFileList(&jpeg_info);

              picasa_info->state = PICASA_ALBUM_INIT;
        }
    }
     
}

int picasa_fetch_album2(picasa_info_t *picasa_info)
{
    int	ret = 0;    
    char 	header[512];     
    int 	httpcode; 

    /* Use RSS.  We don't support Atom yet */
    snprintf(picasa_api_url, MAX_URL, 
             "http://%s/data/feed/api/user/%s/albumid/%s?alt=rss",
             PICASA_HOST,
             picasa_info->username,
             picasa_info->cur_album->id);

    snprintf(header,512, "Authorization: GoogleLogin auth=%s", picasa_info->auth);
    header[511] = '\0';

	Xml_BUFF_init(NET_RECVHTTP);		
	
    	httpcode = mpx_curl_excute(picasa_api_url, NULL, header, NULL);
		
       if (httpcode == 200)
       {
       	picasa_info->state = PICASA_ALBUM_INIT;

		MPX_XML_Parse(picasa_info
			, album_start_element_handler 	
			, album_end_element_handler
			, album_content_handler);  
		
            	if (picasa_info->error_code > 0)
            	{
             		ret = -NETFS_APP_ERROR;
            	}
	#if 0			
		else if(Album_Photo_count==0)
		{
         		ret=-1;  
		}
	#endif	
        }
        else
        {
            ret = -1;
        } 

    	Xml_BUFF_free(NET_RECVHTTP);
    	return ret;
}

/* 
 * Google ClientLogin API for Picasa
 */
static int parse_picasa_auth(picasa_info_t *picasa_info,int httpcode,char * errstring)
{
	int ret = 0;
	int len;
	char * data;	
	XML_BUFF_link_t *ptr;
	int auth_len = sizeof(picasa_info->auth);
	char *end; 
	
	 if (httpcode == 200)
       {
            ptr = App_State.XML_BUF;

            if (ptr != NULL)
            {        
                data =  ptr->BUFF;
                len = ptr->buff_len;
                if (data[len-1] == '\n')
                    data[len-1] = '\0';             /* remove LF */
                else
                    data[len] = '\0';
                //mpDebugPrint("len = %d ",len); 
                data=strstr(data, "Auth=");

                if (data)
                {            	
                    strncpy(picasa_info->auth, data+5, auth_len);
                    if (strlen(data+5) >= auth_len)
                    {
                        picasa_info->auth[auth_len - 1] = '\0';
                        ret = -1;
                        MP_ASSERT(0);
                    }
#if 0 //original code
                    NetAsciiDump(data, len);
#else //weiching new code
//                    NetAsciiDump(data, auth_len+5); //only dump "Auth=" info
#endif                
                }
                else
                {
                    MP_DEBUG("picasa_ClientLogin: no Auth=");
#if 0
                    NetAsciiDump(ptr->BUFF, len);
#endif
                    ret = -3;
                }
            }
        }
        //else if (httpcode > 0)                  /* TODO */
        else       // jeffery 20090320
        {
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 Access Forbidden */
            {
                data =  ptr->BUFF;
                len = ptr->buff_len;
                data[len] = '\0';
                data=strstr(data, "Error=");
                if (data)
                {
                    data += 6;
                    end = strchr(data, '\n');
                    if (end)
                    {
                        if (end[-1] == '\r')        /* no CR here, but just in case */
                            end--;
                        end[0] = '\0';
                    }
                    sprintf(errstring, "%s", data);
                }
                else
                    sprintf(errstring, "UnknownError");
            }
            else
            {
                sprintf(errstring, "HTTP Error %d", httpcode);
            }
            ret = -2;
        }
	 return ret;
}

int picasa_ClientLogin(picasa_info_t *picasa_info, char *email, char *passwd, char *source, char *errstring)
{
    int   ret = 0;
    char data[512];	
    char *url; 
    int httpcode;
    
    MP_DEBUG("NETFS_PICASA: ClientLogin");
    
	url = "https://www.google.com/accounts/ClientLogin";
       snprintf(data,512, "accountType=GOOGLE&Email=%s@gmail.com&Passwd=%s"
                "&service=lh2&source=%s", email, passwd,source);
       data[511] = '\0';

	Xml_BUFF_init(NET_RECVHTTP);   

	httpcode = mpx_easy_curl(url, data);
	ret = parse_picasa_auth(picasa_info,httpcode,errstring);

    	Xml_BUFF_free(NET_RECVHTTP);    
    	return ret;

}


#if ATOM_MODE
static void atom_album_content_handler(void *user_data, const char *s, int len)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
    char    *title;
    int     i;
    if (picasa_info->state == PICASA_ALBUM_PHOTO_TITLE)
    {
		strncpy(jpeg_info.pathname,s,len);
		//mpDebugPrint("jpeg_info.pathname = %s",jpeg_info.pathname);
 	
    }
}

static void atom_album_start_element_handler(void *user_data, const char *name, const char **attr)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
    const char      *url_value      = NULL;
    const char      *file_size_value= NULL;
	BYTE cp = 0,cp_id = 0;
	int i,j;
	
	//mpDebugPrint("atom_album_start_element_handler %s %s",name,*attr);

    if (picasa_info->state == PICASA_ALBUM_INIT)
    {
        if (!strcasecmp(name, "entry"))
        {
			//mpDebugPrint("photo %s",name);
            memset(&jpeg_info, 0, sizeof(jpeg_info));
            picasa_info->state = PICASA_ALBUM_PHOTO_FOUND;
        }
    }
    else if (picasa_info->state == PICASA_ALBUM_PHOTO_FOUND)
    {
        if (!strcasecmp(name, "title"))
        {
            picasa_info->state = PICASA_ALBUM_PHOTO_TITLE;
        }
        else if (!strcasecmp(name, "media:content"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "url"))
                {
                    attr ++;
                    url_value = *attr;
                }
                else if (!strcasecmp(*attr, "fileSize"))
                {
                    attr ++;
                    file_size_value = *attr;
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }
			
            if (file_size_value)
            {
                strncat(jpeg_info.length, file_size_value, 20-1);
            }
            if (url_value)
            {
                //sprintf(jpeg_info.length,"%d",0);
				//mpDebugPrint("url_value = %s, len = %d",url_value,strlen(url_value));
				//strncpy(jpeg_info.url,url_value,strlen(url_value));		
				memset(jpeg_info.length,0,sizeof(jpeg_info.length));
				memcpy(jpeg_info.url,url_value,strlen(url_value));
				Net_Xml_parseFileList(&jpeg_info);		
            }
        }		
		else if (!strcasecmp(name, "link"))
		{
			//mpDebugPrint(" %s ",name);
			while (*attr)
			{
			
				if(!strcasecmp(*attr, "edit"))
			    {
			         attr +=4;
					 memcpy(jpeg_info.thumbnail_url,*attr,strlen(*attr));	   
			    	 //mpDebugPrint("jpeg_info.thumbnail_url  %s ",jpeg_info.thumbnail_url);
			    }
				
				attr ++;
				
			}
		}

    }
}

static void atom_album_end_element_handler(void *user_data, const char *name)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
    int len0= 0 , len1 =0 , len2 =0;	
    BYTE *str1,*str2,*str3;
    BYTE filesz[20];
   	
	//mpDebugPrint(" atom_album_end_element_handler %s",name);

    if (picasa_info->state == PICASA_ALBUM_PHOTO_TITLE)
    {
        if (!strcasecmp(name, "title"))
        {
            //snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,"/%s/%s",picasa_info->cur_album->title,title_value);
            picasa_info->state = PICASA_ALBUM_PHOTO_FOUND;
        }
    }
    else if (picasa_info->state == PICASA_ALBUM_PHOTO_FOUND)
    {
        if (!strcasecmp(name, "entry"))
        {
              picasa_info->state = PICASA_ALBUM_INIT;
        }
    }
     
}
#endif

#if NET_NAPI == 1
extern ST_NET_FILEBROWSER *g_psNet_FileBrowser;
int net_schedule_work(ST_NET_WORK *work);
void picasa_init_todo(ST_NET_WORK *work)
{
	int ret;
	if (!work)
		return -1;
	ret = picasa_init((char *)work->data1, (char *)work->data2);
	return ret;
            }

int schedule_picasa_init(const char *url, const char *base_dir)
    {
    ST_NET_WORK *work = &g_psNet_FileBrowser->work;

	work->data1 = (DWORD) url;
	work->data2 = (DWORD) base_dir;
	work->func =  picasa_init_todo;
	ret = net_schedule_work(work);
	return ret ? 0 : -1;
}
#endif
#if ATOM_MODE
int picasa_atom_fetch_album(picasa_info_t *picasa_info)
        {
    void *curl;
    CURLcode res;
    XML_Parser              parser;
    int                     ret = 0;
    int                     len;
    char *data; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    struct curl_slist *header=NULL;

    char *bigger_buff;


    Xml_BUFF_init(NET_RECVHTTP);	
	mpDebugPrint("NETFS_PICASA: picasa_atom_fetch_album");
	mpDebugPrint("picasa_info->username: %s",picasa_info->username);
	mpDebugPrint("picasa_info->cur_album->id: %s",picasa_info->cur_album->id);

    /* Use RSS.  We don't support Atom yet */
    snprintf(picasa_api_url, MAX_URL, 
             "http://%s/data/feed/api/user/%s/albumid/%s",
             PICASA_HOST,
             picasa_info->username,
             picasa_info->cur_album->id);

    curl = curl_easy_init();
    if(curl) {
        data = picasa_malloc(512);
        if (data == NULL)
            goto exit;
        snprintf(data,512, "Authorization: GoogleLogin auth=%s", picasa_info->auth);
        data[511] = '\0';
        header = curl_slist_append(header, data);
        MP_DEBUG("NETFS_PICASA: extra header=%d", strlen(data));
        curl_easy_setopt(curl, CURLOPT_URL, picasa_api_url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

        res = curl_easy_perform(curl);
        TaskYield();

        /* check return code of curl_easy_perform() */
        if (res != CURLE_OK)
            {
            MP_DEBUG("picasa_atom_fetch_album: curl_easy_perform failed, res = %d", res);
            httpcode = 0;
            goto Picasa_cleanup_2;
            }

        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
            httpcode = 0;

Picasa_cleanup_2:
        /* always cleanup */
        curl_easy_cleanup(curl);

        picasa_mfree(data);

        MP_DEBUG("picasa_fetch_album2: http error resp code=%d", httpcode);
        if (httpcode == 200)
        {
            /* Parse XML */
            parser = XML_ParserCreate(NULL);
            picasa_info->state = PICASA_ALBUM_INIT;

            XML_SetUserData(parser, picasa_info);
            XML_SetElementHandler(parser, atom_album_start_element_handler, atom_album_end_element_handler);
            XML_SetCharacterDataHandler(parser, atom_album_content_handler);

            bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			MP_DEBUG("picasa_fetch_album2: got total data len = %d", len);

            if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
            {
				MP_DEBUG3("picasa_fetch_album2: %s at line %d, column %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser),
				XML_GetCurrentColumnNumber(parser));
        }

            if (picasa_info->error_code > 0)
            {
                ret = -NETFS_APP_ERROR;
                goto exit;
            }
            XML_Parse(parser, data, 0, 1);
        }
        else if (httpcode > 0)                  /* TODO */
        {
            MP_DEBUG("picasa_fetch_album2: http error resp code=%d", httpcode);
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 login failed */
            {
                sprintf(ptr, "Server Error %d login failed", httpcode);
            }
            else
            {
                sprintf(ptr, "HTTP Error %d", httpcode);
            }
            ret = -1;
        }
    }

exit:
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;
}

int picasa_atom_fetch_album_list(picasa_info_t *picasa_info)
{
		void *curl;
		CURLcode res;
		XML_Parser				parser;
		int 					ret = 0;
		int 					len;
		char *data; 
		XML_BUFF_link_t *ptr;
		int httpcode;
		struct curl_slist *header=NULL;
	
		char *bigger_buff; 
		
		Xml_BUFF_init(NET_RECVHTTP);	
		MP_DEBUG("NETFS_PICASA: picasa_atom_fetch_album_list");
	
		snprintf(picasa_api_url, MAX_URL, 
				 "http://%s/data/feed/api/user/default",
				 PICASA_HOST);
		curl = curl_easy_init();
		if(curl) {
			data = picasa_malloc(512);
			if (data == NULL)
				goto exit;
			snprintf(data, 512, "Authorization: GoogleLogin auth=%s", picasa_info->auth);
			data[511] = '\0';
			header = curl_slist_append(header, data);
			MP_DEBUG("NETFS_PICASA: extra header=%d", strlen(data));
			curl_easy_setopt(curl, CURLOPT_URL, picasa_api_url);
		#if	 0
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
			// set CURLOPT_COOKIEJAR
			curl_easy_setopt(curl, CURLOPT_COOKIEJAR, 1);
			curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);
#endif

			//set cert verify false
			//curl_easy_setopt(g_GCE_curl, CURLOPT_SSL_VERIFYPEER, FALSE);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
	
			//set follow location
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	
			res = curl_easy_perform(curl);
			TaskYield();
	
			/* check return code of curl_easy_perform() */
			if (res != CURLE_OK)
			{
				MP_DEBUG("picasa_atom_fetch_album_list: curl_easy_perform failed, res = %d", res);
				httpcode = 0;
				goto Picasa_cleanup_1;
			}
	
			if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
				httpcode = 0;
	
	Picasa_cleanup_1:
			/* always cleanup */
			curl_easy_cleanup(curl);
	
			picasa_mfree(data);
	
			MP_DEBUG("picasa_atom_fetch_album_list: http error resp code=%d", httpcode);
			if (httpcode == 200)
			{
				/* Parse XML */
				parser = XML_ParserCreate(NULL);
				picasa_info->state = PICASA_ALBUM_LIST_INIT;
	
				XML_SetUserData(parser, picasa_info);
				XML_SetElementHandler(parser, atom_album_list_start_element_handler, atom_album_list_end_element_handler);
				XML_SetCharacterDataHandler(parser, atom_album_list_content_handler);
	
				bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
				MP_DEBUG("picasa_fetch_album_list2: got total data len = %d", len);
	
				if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
				{
					MP_DEBUG3("picasa_fetch_album_list2: %s at line %d, column %d\n",
					XML_ErrorString(XML_GetErrorCode(parser)),
					XML_GetCurrentLineNumber(parser),
					XML_GetCurrentColumnNumber(parser));
				}
	
				if (picasa_info->error_code > 0)
				{
					ret = -NETFS_APP_ERROR;
					goto exit;
				}
				XML_Parse(parser, data, 0, 1);
			}
			else if (httpcode > 0)					/* TODO */
			{
				MP_DEBUG("NETFS_PICASA: http error resp code=%d", httpcode);
				ptr = App_State.XML_BUF;
				if (httpcode == 403)					/* 403 login failed */
				{
					sprintf(ptr, "Server Error %d login failed", httpcode);
				}
				else
				{
					sprintf(ptr, "HTTP Error %d", httpcode);
				}
				ret = -1;
			}
		}
	
	exit:
		if(bigger_buff != NULL)
			ext_mem_free(bigger_buff);
	
		Xml_BUFF_free(NET_RECVHTTP);
		return ret;

}

int picasa_atom_add_album(picasa_info_t *picasa_info,BYTE *user_name,BYTE *album_name)
{
    void *curl;
    CURLcode res;
    XML_Parser              parser;
    int                     ret = 0;
    int                     len;
    char *data,*header_data; 
    XML_BUFF_link_t *ptr;
    int httpcode;

    char *bigger_buff;
    struct curl_slist *header=NULL;

	Xml_BUFF_init(NET_RECVHTTP);
	
    snprintf(picasa_api_url, MAX_URL, 
             "http://%s/data/feed/api/user/%s",
             PICASA_HOST,user_name);
	
	curl = curl_easy_init();
	if(curl) {
		data = picasa_malloc(1024);
		header_data = picasa_malloc(512);
		if (data == NULL)
			goto exit;
		//memset(data,0x00,1024);	
        snprintf(data,1024,
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>" CRLF
			"<entry xmlns=\"http://www.w3.org/2005/Atom\""
			 " xmlns:media=\"http://search.yahoo.com/mrss/\""
			 " xmlns:gphoto=\"http://schemas.google.com/photos/2007\">" CRLF
			 "<title type=\"text\">%s</title>" CRLF
			 "<summary type=\"text\">This was the recent trip I took to Italy.</summary>" CRLF
			 "<gphoto:location>Italy</gphoto:location>" CRLF
			 "<gphoto:access>public</gphoto:access>" CRLF
			 "<gphoto:commentingEnabled>true</gphoto:commentingEnabled>" CRLF
			 "<gphoto:timestamp>1152255600000</gphoto:timestamp>" CRLF
			 "<media:group>"  CRLF
			 "<media:keywords>italy, vacation</media:keywords>" CRLF
			 "</media:group>" CRLF
			 "<category scheme=\"http://schemas.google.com/g/2005#kind\""
			 " term=\"http://schemas.google.com/photos/2007#album\"></category>" CRLF
			 "</entry>",album_name);
			
        data[1023] = '\0';
		
		//NetAsciiDump(data,strlen(data));
		
		snprintf(header_data,512, "Content-Type: application/atom+xml");
		header_data[511] = '\0';
		header = curl_slist_append(header, "Content-Type: application/atom+xml");
		MP_ASSERT(header);
		MP_DEBUG("NETFS_PICASA: auth-Length=%d", strlen(picasa_info->auth));
		snprintf(header_data,512, "Authorization: GoogleLogin auth=%s", picasa_info->auth);
//	  __asm("break 100");
		header = curl_slist_append(header, header_data);
		MP_ASSERT(header);
		snprintf(header_data,512, "Content-Length: %d", strlen(data));
		MP_DEBUG("NETFS_PICASA: Content-Length=%s", header_data);
		header = curl_slist_append(header, header_data);
		MP_ASSERT(header);
		curl_easy_setopt(curl, CURLOPT_URL, picasa_api_url);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);

		//set follow location
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

		res = curl_easy_perform(curl);
		TaskYield();

		/* check return code of curl_easy_perform() */
		if (res != CURLE_OK)
		{
			MP_DEBUG("picasa_fetch_album_list3: curl_easy_perform failed, res = %d", res);
			httpcode = 0;
			goto Picasa_cleanup_1;
		}

		if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
			httpcode = 0;

Picasa_cleanup_1:
		/* always cleanup */
		curl_easy_cleanup(curl);

		curl_slist_free_all(header);
		picasa_mfree(header_data);
		picasa_mfree(data);

		MP_DEBUG("picasa_fetch_album_list3: http error resp code=%d", httpcode);
		MP_DEBUG("picasa_fetch_album_list3: App_State.dwTotallen=%d", App_State.dwTotallen);
		if (App_State.dwTotallen > 0)
		{
			ptr = App_State.XML_BUF;
			NetAsciiDump(ptr->BUFF, ptr->buff_len);
		}
		if (httpcode == 200)
		{
			/* Parse XML */
			parser = XML_ParserCreate(NULL);
			picasa_info->state = PICASA_ALBUM_LIST_INIT;

			XML_SetUserData(parser, picasa_info);
			XML_SetElementHandler(parser, album_list_start_element_handler, album_list_end_element_handler);
			XML_SetCharacterDataHandler(parser, album_list_content_handler);

			bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			MP_DEBUG("picasa_fetch_album_list3: got total data len = %d", len);

			if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
			{
				MP_DEBUG3("picasa_fetch_album_list3: %s at line %d, column %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser),
				XML_GetCurrentColumnNumber(parser));
			}

			if (picasa_info->error_code > 0)
			{
				ret = -NETFS_APP_ERROR;
				goto exit;
			}
			XML_Parse(parser, NULL, 0, 1);
		}
		else if (httpcode > 0)					/* TODO */
		{
			MP_DEBUG("NETFS_PICASA: http error resp code=%d", httpcode);
			ptr = App_State.XML_BUF;
			if (httpcode == 403)					/* 403 login failed */
			{
				sprintf(ptr, "Server Error %d login failed", httpcode);
			}
			else
			{
				sprintf(ptr, "HTTP Error %d", httpcode);
			}
			ret = -1;
		}
	}

exit:
	if(bigger_buff != NULL)
		ext_mem_free(bigger_buff);

	Xml_BUFF_free(NET_RECVHTTP);
	return ret;

}

int picasa_atom_edit_album(picasa_info_t *picasa_info,BYTE *user_name,BYTE *album_id,BYTE *album_name,BYTE *new_album_name)
{
		void *curl;
		CURLcode res;
		XML_Parser				parser;
		int 					ret = 0;
		int 					len;
		char *data,*header_data; 
		XML_BUFF_link_t *ptr;
		int httpcode;
	
		char *bigger_buff;
		int i,j;
		
		char tmp_cookie[1024]="_uploader=ACTIVEX; _rtok=vimelBjMxnMj;"
				" lh=DQAAAGsAAAB_Ja3bBGVgDRTC7JQXL7QehqksrNX95msks1dnzQQoB_cHWaeSBjmRU6W5"
				"G0uHSNgUZub6KFlvNsarE5TDwavwSm7iA6ZtDfqbcgjRcn_dmt2wpA6o-axol3NgCKBGQI3Se"
				"C8A0_PFI66vFKltf_h0; S=photos_html=aSqEik1OTLVHfoHYnIPwNw; rememberme=false;"
				" PREF=ID=58aaeec00c19df83:TB=5:TM=1245230900:LM=1245279938:S=r_k1N3ix535X-Epv;"
				" NID=23=l8tpb3D7YKPavV9hDfW-H7oUJIVHkE8mybQheah-E79mCHYloEatp5ZYJyhchsgXq2l1LPA"
				"XUuCfFh1NYYsA3WP2FTM47SQz5idPLOnMuM15uB6bfTkNygH1I9rsiQS-; SID=DQAAAGoAAABTuVbJU"
				"TZs4DDziERoHlNms4f8VFjEheJL-tUgJWAOz3hFQoMs1y960JoPwg8tTpng2QfMgU1JSHf0aQKESo0Abl"
				"6fYgO2tfc7_jrLs";
		struct curl_slist *header=NULL;
		Xml_BUFF_init(NET_RECVHTTP);
		snprintf(picasa_api_url, MAX_URL, 
				 "http://%s/lh/updateAlbum?tok=7aq6IX_HREgO1qjF43V-er2Pob4&uname=%s&aid=%s",
				 PICASA_HOST,user_name,album_id);
		
		curl = curl_easy_init();
		if(curl) {
			data = picasa_malloc(1024);
			header_data = picasa_malloc(512);
			if (data == NULL)
				goto exit;
			
			strcat(data, "uname=");
			strcat(data, user_name);
			strcat(data, "&dialogName=updateAlbumDialog&redir=SAME_ALBUM&dest=&photoop=&selectedphotos=&srcAid=&aid=");
			strcat(data, album_id);
			strcat(data, "&albumop=&title=");
			strcat(data, new_album_name);
			strcat(data, "&date=6%2F18%2F2009&dateDay=18&dateMonth=5&dateYear=2009&description=test1&location=%E6%96%B0%E7%AB%B9&lat=24.703281&lon=121.125214&latspan=0.3143944209730269&lonspan=0.6097412109374973&removegeo=false&access=public");
            				
			data[1023] = '\0';
			
			snprintf(header_data,512, "Accept:   image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/msword, application/xaml+xml, application/vnd.ms-xpsdocument, application/x-ms-xbap, application/x-ms-ap"
				"plication, application/x-silverlight, */*");
			header_data[511] = '\0';
			header = curl_slist_append(header,header_data);
			MP_ASSERT(header);
			
			snprintf(header_data,512, "Referer:   http://picasaweb.google.com/%s/%s",user_name,album_name);
			header_data[511] = '\0';
			header = curl_slist_append(header,header_data);
			MP_ASSERT(header);
			
			snprintf(header_data,512, "Content-Type:   application/x-www-form-urlencoded");
			header_data[511] = '\0';
			header = curl_slist_append(header,header_data);
			MP_ASSERT(header);
			snprintf(header_data,512, "Cache-Control:   no-cache");
			header_data[511] = '\0';
			header = curl_slist_append(header,header_data);
			MP_ASSERT(header);
			snprintf(header_data,512, "Content-Length: %d", strlen(data));
			MP_DEBUG("NETFS_PICASA: Content-Length=%s", header_data);
			header_data[511] = '\0';
			header = curl_slist_append(header, header_data);
			MP_ASSERT(header);
			
			curl_easy_setopt(curl, CURLOPT_URL, picasa_api_url);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));
			curl_easy_setopt(curl, CURLOPT_COOKIE,tmp_cookie);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
			//set follow location
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	
			res = curl_easy_perform(curl);
			TaskYield();
	
			/* check return code of curl_easy_perform() */
			if (res != CURLE_OK)
			{
				MP_DEBUG("picasa_fetch_album_list3: curl_easy_perform failed, res = %d", res);
				httpcode = 0;
				goto Picasa_cleanup_1;
			}
	
			if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
				httpcode = 0;
	
	Picasa_cleanup_1:
			/* always cleanup */
			curl_easy_cleanup(curl);
	
			curl_slist_free_all(header);
			picasa_mfree(header_data);
			picasa_mfree(data);
	
			MP_DEBUG("picasa_fetch_album_list3: http error resp code=%d", httpcode);
			MP_DEBUG("picasa_fetch_album_list3: App_State.dwTotallen=%d", App_State.dwTotallen);
			if (App_State.dwTotallen > 0)
			{
				ptr = App_State.XML_BUF;
				NetAsciiDump(ptr->BUFF, ptr->buff_len);
			}
			if (httpcode == 200)
			{
				/* Parse XML */
				parser = XML_ParserCreate(NULL);
				picasa_info->state = PICASA_ALBUM_LIST_INIT;
	
				XML_SetUserData(parser, picasa_info);
				XML_SetElementHandler(parser, album_list_start_element_handler, album_list_end_element_handler);
				XML_SetCharacterDataHandler(parser, album_list_content_handler);
	
				bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
				MP_DEBUG("picasa_fetch_album_list3: got total data len = %d", len);
	
				if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
				{
					MP_DEBUG3("picasa_fetch_album_list3: %s at line %d, column %d\n",
					XML_ErrorString(XML_GetErrorCode(parser)),
					XML_GetCurrentLineNumber(parser),
					XML_GetCurrentColumnNumber(parser));
				}
	
				if (picasa_info->error_code > 0)
				{
					ret = -NETFS_APP_ERROR;
					goto exit;
				}
				XML_Parse(parser, NULL, 0, 1);
			}
			else if (httpcode > 0)					/* TODO */
			{
				MP_DEBUG("NETFS_PICASA: http error resp code=%d", httpcode);
				ptr = App_State.XML_BUF;
				if (httpcode == 403)					/* 403 login failed */
				{
					sprintf(ptr, "Server Error %d login failed", httpcode);
				}
				else
				{
					sprintf(ptr, "HTTP Error %d", httpcode);
				}
				ret = -1;
			}
		}
	
	exit:
		if(bigger_buff != NULL)
			ext_mem_free(bigger_buff);
	
		Xml_BUFF_free(NET_RECVHTTP);
		return ret;

}
int picasa_atom_delete_album(picasa_info_t *picasa_info,BYTE *edit)
{
		void *curl;
		CURLcode res;
		XML_Parser				parser;
		int 					ret = 0;
		int 					len,i;
		char *data; 
		XML_BUFF_link_t *ptr;
		int httpcode;
	
		char *bigger_buff;
		struct curl_slist *header=NULL;
	
		Xml_BUFF_init(NET_RECVHTTP);
		
		//snprintf(picasa_api_url, MAX_URL, 
		//		 "http://%s/data/entry/api/user/%s/albumid/%s/24",
		//		 PICASA_HOST,user_name,album_id);
		//__asm("break 100");
		memcpy(picasa_api_url,edit,strlen(edit));
		
		mpDebugPrint("DELETE %s",picasa_api_url);
			
		curl = curl_easy_init();
		if(curl) {
			data = picasa_malloc(512);
			if (data == NULL)
				goto exit;
			
			snprintf(data,512, "Content-Type: application/atom+xml");
			data[511] = '\0';
			header = curl_slist_append(header, "Content-Type: application/atom+xml");
			MP_ASSERT(header);
			header = curl_slist_append(header, "Content-Length: 0");
			MP_ASSERT(header);
			snprintf(data,512, "x-http-method-override:DELETE");
			header = curl_slist_append(header, data);
			MP_ASSERT(header);
			MP_DEBUG("NETFS_PICASA: auth-Length=%d", strlen(picasa_info->auth));
			snprintf(data,512, "Authorization: GoogleLogin auth=%s", picasa_info->auth);
	//	  __asm("break 100");
			header = curl_slist_append(header, data);
			MP_ASSERT(header);
			
			//snprintf(data,512, "Content-Length: 0");
			//MP_DEBUG("NETFS_PICASA: Content-Length=%s", data);
			memset(data,0x00,512);
			data[511] = '\0';
			curl_easy_setopt(curl, CURLOPT_URL, picasa_api_url);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
	
			//set follow location
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	
			res = curl_easy_perform(curl);
			TaskYield();
	
			/* check return code of curl_easy_perform() */
			if (res != CURLE_OK)
			{
				MP_DEBUG("picasa_fetch_album_list3: curl_easy_perform failed, res = %d", res);
				httpcode = 0;
				goto Picasa_cleanup_1;
			}
	
			if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
				httpcode = 0;
	
	Picasa_cleanup_1:
			/* always cleanup */
			curl_easy_cleanup(curl);
	
			curl_slist_free_all(header);
			picasa_mfree(data);
	
			MP_DEBUG("picasa_fetch_album_list3: http error resp code=%d", httpcode);
			MP_DEBUG("picasa_fetch_album_list3: App_State.dwTotallen=%d", App_State.dwTotallen);
			if (App_State.dwTotallen > 0)
			{
				ptr = App_State.XML_BUF;
				NetAsciiDump(ptr->BUFF, ptr->buff_len);
			}
			if (httpcode == 200)
			{
				/* Parse XML */
				parser = XML_ParserCreate(NULL);
				picasa_info->state = PICASA_ALBUM_LIST_INIT;
	
				XML_SetUserData(parser, picasa_info);
				XML_SetElementHandler(parser, album_list_start_element_handler, album_list_end_element_handler);
				XML_SetCharacterDataHandler(parser, album_list_content_handler);
	
				bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
				MP_DEBUG("picasa_fetch_album_list3: got total data len = %d", len);
	
				if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
				{
					MP_DEBUG3("picasa_fetch_album_list3: %s at line %d, column %d\n",
					XML_ErrorString(XML_GetErrorCode(parser)),
					XML_GetCurrentLineNumber(parser),
					XML_GetCurrentColumnNumber(parser));
				}
	
				if (picasa_info->error_code > 0)
				{
					ret = -NETFS_APP_ERROR;
					goto exit;
				}
				XML_Parse(parser, NULL, 0, 1);
			}
			else if (httpcode > 0)					/* TODO */
			{
				MP_DEBUG("NETFS_PICASA: http error resp code=%d", httpcode);
				ptr = App_State.XML_BUF;
				if (httpcode == 403)					/* 403 login failed */
				{
					sprintf(ptr, "Server Error %d login failed", httpcode);
				}
				else
				{
					sprintf(ptr, "HTTP Error %d", httpcode);
				}
				ret = -1;
			}
		}
	
	exit:
		if(bigger_buff != NULL)
			ext_mem_free(bigger_buff);
	
		Xml_BUFF_free(NET_RECVHTTP);
		return ret;
}
int picasa_atom_delete_Photo(picasa_info_t *picasa_info,BYTE *edit)
{
		void *curl;
		CURLcode res;
		XML_Parser				parser;
		int 					ret = 0;
		int 					len,i;
		char *data; 
		XML_BUFF_link_t *ptr;
		int httpcode;
	
		char *bigger_buff;
		struct curl_slist *header=NULL;
	
		Xml_BUFF_init(NET_RECVHTTP);
		
		//snprintf(picasa_api_url, MAX_URL, 
		//		 "http://%s/data/entry/api/user/%s/albumid/%s/24",
		//		 PICASA_HOST,user_name,album_id);
		//__asm("break 100");
		memcpy(picasa_api_url,edit,strlen(edit));
		
		mpDebugPrint("DELETE %s",picasa_api_url);
			
		curl = curl_easy_init();
		if(curl) {
			data = picasa_malloc(512);
			if (data == NULL)
				goto exit;
			
			snprintf(data,512, "Content-Type: application/atom+xml");
			data[511] = '\0';
			header = curl_slist_append(header, "Content-Type: application/atom+xml");
			MP_ASSERT(header);
			header = curl_slist_append(header, "Content-Length: 0");
			MP_ASSERT(header);
			snprintf(data,512, "x-http-method-override:DELETE");
			header = curl_slist_append(header, data);
			MP_ASSERT(header);
			MP_DEBUG("NETFS_PICASA: auth-Length=%d", strlen(picasa_info->auth));
			snprintf(data,512, "Authorization: GoogleLogin auth=%s", picasa_info->auth);
	//	  __asm("break 100");
			header = curl_slist_append(header, data);
			MP_ASSERT(header);
			
			//snprintf(data,512, "Content-Length: 0");
			//MP_DEBUG("NETFS_PICASA: Content-Length=%s", data);
			memset(data,0x00,512);
			data[511] = '\0';
			curl_easy_setopt(curl, CURLOPT_URL, picasa_api_url);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
	
			//set follow location
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	
			res = curl_easy_perform(curl);
			TaskYield();
	
			/* check return code of curl_easy_perform() */
			if (res != CURLE_OK)
			{
				MP_DEBUG("picasa_fetch_album_list3: curl_easy_perform failed, res = %d", res);
				httpcode = 0;
				goto Picasa_cleanup_1;
			}
	
			if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
				httpcode = 0;
	
	Picasa_cleanup_1:
			/* always cleanup */
			curl_easy_cleanup(curl);
	
			curl_slist_free_all(header);
			picasa_mfree(data);
	
			MP_DEBUG("picasa_fetch_album_list3: http error resp code=%d", httpcode);
			MP_DEBUG("picasa_fetch_album_list3: App_State.dwTotallen=%d", App_State.dwTotallen);
			if (App_State.dwTotallen > 0)
			{
				ptr = App_State.XML_BUF;
				NetAsciiDump(ptr->BUFF, ptr->buff_len);
			}
			if (httpcode == 200)
			{
				/* Parse XML */
				parser = XML_ParserCreate(NULL);
				picasa_info->state = PICASA_ALBUM_LIST_INIT;
	
				XML_SetUserData(parser, picasa_info);
				XML_SetElementHandler(parser, album_list_start_element_handler, album_list_end_element_handler);
				XML_SetCharacterDataHandler(parser, album_list_content_handler);
	
				bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
				MP_DEBUG("picasa_fetch_album_list3: got total data len = %d", len);
	
				if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
				{
					MP_DEBUG3("picasa_fetch_album_list3: %s at line %d, column %d\n",
					XML_ErrorString(XML_GetErrorCode(parser)),
					XML_GetCurrentLineNumber(parser),
					XML_GetCurrentColumnNumber(parser));
				}
	
				if (picasa_info->error_code > 0)
				{
					ret = -NETFS_APP_ERROR;
					goto exit;
				}
				XML_Parse(parser, NULL, 0, 1);
			}
			else if (httpcode > 0)					/* TODO */
			{
				MP_DEBUG("NETFS_PICASA: http error resp code=%d", httpcode);
				ptr = App_State.XML_BUF;
				if (httpcode == 403)					/* 403 login failed */
				{
					sprintf(ptr, "Server Error %d login failed", httpcode);
				}
				else
				{
					sprintf(ptr, "HTTP Error %d", httpcode);
				}
				ret = -1;
			}
		}
	
	exit:
		if(bigger_buff != NULL)
			ext_mem_free(bigger_buff);
	
		Xml_BUFF_free(NET_RECVHTTP);
		return ret;
}

#endif

#endif

#endif

