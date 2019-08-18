/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "netfs.h"
#include "netfs_pri.h"
#include "xmlpicasa.h"
//#ifdef HAVE_LWIP
#include "..\..\lwip\include\net_sys.h"
//#else
//#include "..\..\uip\include\net_sys.h"
//#endif
#if NETWARE_ENABLE	
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
void *mm_realloc(void *ptr, size_t size);
void mm_free(void *ptr);
void *mm_malloc(size_t size);

#define picasa_malloc(sz)   mm_malloc(sz)
#define picasa_mfree(ptr)   mm_free(ptr)


int Picasa_PhotoList_Get(BYTE *PhotoSet)
{
	picasa_info_t     *ppicasa_info;
	int     ret;
    	int     count;

	//EnableNetWareTask();
	ppicasa_info = &picasa_info;  	
       ppicasa_info->cur_album = ppicasa_info->album_list.next;
       
	while(ppicasa_info->cur_album)
	{
		if(!strcasecmp(ppicasa_info->cur_album->title, PhotoSet))
		{
			ret = picasa_fetch_album(ppicasa_info);
            		if (ret < 0)
            		{
                		mpDebugPrint("error code: %d\n", ppicasa_info->error_code);
                		return ret;
            		}
			break;
		}	
		ppicasa_info->cur_album = ppicasa_info->cur_album->next;
	}
	//picasa_info = ppicasa_info;
	//DisableNetWareTask();  
	return 0;
}


int picasa_init(const char *username, const char *base_dir)
{
    album_entry_t     *tmp_entry;
    int     ret;
    int     count;

    MP_DEBUG1("request picasa photo for '%s'\n", username);

    memset(&picasa_info, 0, sizeof(picasa_info_t));
    strncat(picasa_info.username, username, MAX_USERNAME);
    picasa_info.cur_album = &picasa_info.album_list;
    picasa_base_dir = base_dir;

    MP_DEBUG("1 picasa_fetch_album_list @@@@@@@@@@@@@@@@@@@@@@");
    ret = picasa_fetch_album_list(&picasa_info);
    if (ret < 0)
    {
        MP_ALERT("picasa_fetch_album_list(): error code: %d\n", picasa_info.error_code);
        return ret;
    }

    count = 0;
    picasa_info.cur_album = picasa_info.album_list.next;
    tmp_entry = picasa_info.cur_album;
#if 1	
    if (tmp_entry)
    {
        while (tmp_entry)
        {
            MP_DEBUG2("Fetch photos from '%s' with id '%s'\n",tmp_entry->title,tmp_entry->id);
#if 0
            //MP_DEBUG("2 picasa_fetch_album @@@@@@@@@@@@@@@@@@@@@@");
            ret = picasa_fetch_album(&picasa_info);
            if (ret < 0)
            {
                mpDebugPrint("error code: %d\n", picasa_info.error_code);
                return ret;
            }
 #else
            Net_Xml_PhotoSetList(tmp_entry->title,count);	
#endif             
	    count ++;
            tmp_entry = tmp_entry->next;
        }
	 Net_PhotoSet_SetCount(count);	
    }
    else
    {
        mpDebugPrint("No album.\n");
    }
#else
 if (picasa_info.cur_album)
    {
        while (picasa_info.cur_album)
        {
            MP_DEBUG2("Fetch photos from '%s' with id '%s'\n",
                    picasa_info.cur_album->title,
                    picasa_info.cur_album->id);

            //MP_DEBUG("2 picasa_fetch_album @@@@@@@@@@@@@@@@@@@@@@");
            ret = picasa_fetch_album(&picasa_info);
            if (ret < 0)
            {
                mpDebugPrint("error code: %d\n", picasa_info.error_code);
                return ret;
            }

            count ++;
            picasa_info.cur_album = picasa_info.cur_album->next;
        }
    }
    else
    {
        mpDebugPrint("No album.\n");
    }
#endif

#if 0
    /* free resources allocated for all album */
    picasa_info.cur_album = picasa_info.album_list.next;
    while (picasa_info.cur_album)
    {
        tmp_entry = picasa_info.cur_album;
        picasa_info.cur_album = picasa_info.cur_album->next;

        mem_free(tmp_entry);
    }
#endif
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
        for (i = strlen(title); i < MAX_TITLE && len > 0; len--, i++)
            title[i] = *s++;
        title[i] = 0;
    }
    else if (picasa_info->state == PICASA_ALBUM_LIST_ID && picasa_info->cur_album)
    {
        id = picasa_info->cur_album->id;
        for (i = strlen(id); i < MAX_ALBUMID && len > 0; len--, i++)
            id[i] = *s++;
        id[i] = 0;
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

static int picasa_fetch_album_list(picasa_info_t *picasa_info)
{
    int                     ret;
	
    /* Get Data from Remote Site and Parse it */

    picasa_info->state = PICASA_ALBUM_LIST_INIT;

    snprintf(picasa_api_url, MAX_URL, 
             "http://%s/data/feed/api/user/%s/?kind=album&alt=rss&access=public",
             PICASA_HOST,
             picasa_info->username);

    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(picasa_api_url,NETFS_PICASA,0,0);
    if(ret < 0) 
        goto Recv_exit;

    MPX_XML_Parse(picasa_info
		               , album_list_start_element_handler
		               , album_list_end_element_handler
		               , album_list_content_handler);	

    if (picasa_info->error_code > 0)
    {
        ret = -NETFS_APP_ERROR;
        goto fatal_exit;
    }

exit:

    Xml_BUFF_free(NET_RECVHTTP);
   	return ret;

Recv_exit:
fatal_exit:
    goto exit;
}

#if 1
/**
 *  Get photo list of specified photo set
 */
static void album_content_handler(void *user_data, const char *s, int len)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
    char    *title;
    int     i;
   //cj modify 031809
   #if 0
    if (picasa_info->state == PICASA_ALBUM_PHOTO_TITLE && picasa_info->cur_album)
    {
        title = jpeg_info.pathname;
        for (i = strlen(title); i < NETFS_MAX_PATHNAME && len > 0; len--, i++)
            title[i] = *s++;
        title[i] = 0;
    }
#else
    if (picasa_info->state == PICASA_ALBUM_PHOTO_TITLE)
    {
		strncpy(jpeg_info.pathname,s,len);
		MP_DEBUG1("jpeg_info.pathname = %s",jpeg_info.pathname);

    }
#endif
/*
    if (picasa_info->state == PICASA_ALBUM_PHOTO_TITLE && picasa_info->cur_album)
    {
        MP_DEBUG2("album_content_handler 'l=%d' '%s'",len, s);
        title = jpeg_info.pathname;
        len = (len >= NETFS_MAX_PATHNAME)? (NETFS_MAX_PATHNAME-1) :len;
        memcpy(title, s,len);
        title[len] = '\0';
    }
*/
	if(picasa_info->state == PICASA_ALBUM_PHOTO_COUNTS)
	{
       MP_DEBUG1("PICASA_ALBUM_PHOTO_COUNTS== %s",s);
//	   Album_Photo_count= atoi(s);
	   MP_DEBUG1("Album_Photo_count== %d",Album_Photo_count);
	}
}

BYTE thumbnailcount=1;
static void album_start_element_handler(void *user_data, const char *name, const char **attr)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
    const char      *url_value      = NULL;
    const char      *file_size_value= NULL;
	const char      *url_value1     = NULL;

    if (picasa_info->state == PICASA_ALBUM_INIT)
    {

        if (!strcasecmp(name, "item"))
        {
            memset(&jpeg_info, 0, sizeof(jpeg_info));
            picasa_info->state = PICASA_ALBUM_PHOTO_FOUND;
        }
	if(!strcasecmp(name, "gphoto:numphotos"))
        {
            picasa_info->state = PICASA_ALBUM_PHOTO_COUNTS;
        }
    }
    else if (picasa_info->state == PICASA_ALBUM_PHOTO_FOUND)
    {
        if (!strcasecmp(name, "title"))
        {
            picasa_info->state = PICASA_ALBUM_PHOTO_TITLE;
            snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME, "%s",picasa_info->cur_album->title);
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
            #if 0  //payton 20090506 replace snprintf with strcpy
                snprintf(jpeg_info.url, NETFS_MAX_URL, url_value);
            #else
				strcpy(jpeg_info.url,url_value);
			#endif
            }
        }
        else if (!strcasecmp(name, "media:thumbnail")) //Payton to catch url of thumbnail 
        {
            MP_DEBUG("media:thumbnail");
			if(thumbnailcount>3)
              thumbnailcount=1;
			
			while (*attr)
            {
             MP_DEBUG1("thumbnailcount%d",thumbnailcount);
             if(thumbnailcount== 2)
             {
                if (!strcasecmp(*attr, "url"))
                {
                    attr ++;
                    url_value1 = *attr;
                }
                else
                {
                    attr ++;
                }
             }

                attr ++;
            }

            if (url_value1)
            {
            #if 0  //payton 20090506 replace snprintf with strcpy
                snprintf(jpeg_info.thumbnail_url, NETFS_MAX_URL, url_value1);
			#else
                strcpy(jpeg_info.thumbnail_url,url_value1);
            #endif
            }
         	thumbnailcount++;
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
	else if(picasa_info->state ==PICASA_ALBUM_PHOTO_COUNTS)
	{
        if(!strcasecmp(name, "gphoto:numphotos"))
        {
            picasa_info->state = PICASA_ALBUM_INIT;
        }

	}
    else if (picasa_info->state == PICASA_ALBUM_PHOTO_FOUND)
    {
        if (!strcasecmp(name, "item"))
        {
              MP_DEBUG2("add photo '%s' => '%s'\n",jpeg_info.pathname, jpeg_info.url);
		sprintf(jpeg_info.length,"%d",0);	
#if 0 //cj not need to add file to netfs		
		//netfs_add_file(&jpeg_info);
#endif
		Net_Xml_parseFileList(&jpeg_info);

              picasa_info->state = PICASA_ALBUM_INIT;
        }
    }
     
}

#else // origin
/**
 *  Get photo list of specified photo set
 */
static void album_content_handler(void *user_data, const char *s, int len)
{
    picasa_info_t   *picasa_info    = (picasa_info_t *) user_data;
    char    *title;
    int     i;
	//cj modify 031809
#if 0
    if (picasa_info->state == PICASA_ALBUM_PHOTO_TITLE && picasa_info->cur_album)
    {
        title = jpeg_info.pathname;
        for (i = strlen(title); i < NETFS_MAX_PATHNAME && len > 0; len--, i++)
            title[i] = *s++;
        title[i] = 0;
    }
#else
    if (picasa_info->state == PICASA_ALBUM_PHOTO_TITLE)
    {
		strncpy(jpeg_info.pathname,s,len);
		MP_DEBUG1("jpeg_info.pathname = %s",jpeg_info.pathname);
 	
    }
#endif
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
#if 0			
            snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME, "%s/%s/", 
                     picasa_base_dir, picasa_info->cur_album->title);
#endif
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
#endif

static int picasa_fetch_album(picasa_info_t *picasa_info)
{
    int                     ret;

    /* Get Data from Remote Site and Parse it */
    picasa_info->state = PICASA_ALBUM_INIT;

    snprintf(picasa_api_url, MAX_URL, 
             "http://%s/data/feed/api/user/%s/albumid/%s?alt=rss",
             PICASA_HOST,
             picasa_info->username,
             picasa_info->cur_album->id);

    Xml_BUFF_init(NET_RECVHTTP);	
	
    ret = Net_Recv_Data(picasa_api_url,NETFS_PICASA,0,0);
    if(ret < 0)
        goto Recv_exit;

    MPX_XML_Parse(picasa_info
		               , album_start_element_handler
		               , album_end_element_handler
		               , album_content_handler);

    /* parse next page of photolist if it exist. */
    if (picasa_info->error_code > 0)
    {
        ret = -NETFS_APP_ERROR;
        goto fatal_exit;
    }

exit:

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

Recv_exit:
fatal_exit:
    goto exit;
}
#endif