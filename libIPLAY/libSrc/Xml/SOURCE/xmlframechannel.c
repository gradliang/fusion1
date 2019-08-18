/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0 
#include "corelib.h"

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
#include "xmlframechannel.h"

#include "..\..\lwip\include\net_sys.h"

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define framechannel_malloc(sz)   mm_malloc(sz)
#define framechannel_mfree(ptr)   mm_free(ptr)

#if HAVE_FRAMECHANNEL
static struct netfs_file_entry jpeg_info;
framechannel_map_t *map;

char  framechannel_api_url[MAX_URL];

#define FRAMECHANNEL_PASSWORD     "1234"

#define FRAMECHANNEL_HOST "www.framechannel.com"

extern Net_App_State App_State;


int framechannel_PhotoList_Get(BYTE *PhotoSet)
{
	framechannel_map_t *pmap;
	int     ret;
    int     count;
		
	EnableNetWareTask();
	pmap = map;
       map->info.cur_set = map->info.set_list.next;
       
	while(map->info.cur_set)
	{
		if(!strcasecmp(map->info.cur_set->title, PhotoSet))
		{
			ret = framechannel_fetch_photolists(map);
            		if (ret < 0)
            		{
                		//mpDebugPrint("error code: %d", map->info.error_code);
                		return ret;
            		}
			break;
		}	
		map->info.cur_set = map->info.cur_set->next;
	}
	map = pmap;
	//DisableNetWareTask();  
	return 0;
}


int framechannel_init(const char *username, const char *base_dir)
{
    set_entry_t     *tmp_entry;
    
    int     ret;
    int     count;

    mpDebugPrint("request flickr photo for '%s'", username);
    map = NULL;
    map = (framechannel_map_t *) framechannel_malloc(sizeof(framechannel_map_t));
	
    if (!map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    memset(map, 0, sizeof(framechannel_map_t));
    
    map->base_dir = (char *) framechannel_malloc(strlen(base_dir) + 1);
    if (!map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    strcpy(map->base_dir, base_dir);

    map->info.password = FRAMECHANNEL_PASSWORD;
    strncat(map->info.username, username, MAX_USERNAME);

    MP_DEBUG("1 framechannel_fetch_photosets  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    ret = framechannel_fetch_photolists(map);
    if (ret < 0)
    {
        //mpDebugPrint("error code: %d", map->info.error_code);
        return ret;
    }
	
    ret = NETFS_OK;

exit:

#if 0	
    /* free resources allocated for all photoset */
    map->info.cur_set = map->info.set_list.next;
    while (map->info.cur_set)
    {
        tmp_entry = map->info.cur_set;
        map->info.cur_set = map->info.cur_set->next;

        mem_free(tmp_entry);
    }
#endif
    return ret;


fatal_exit:

    if (map)
    {
        if (map->base_dir)
            framechannel_mfree(map->base_dir);
        framechannel_mfree(map);
    }

    goto exit;

}

void framechannel_exit(const char *base_dir)
{
     /* free resources allocated for all photoset */
    set_entry_t     *tmp_entry;	
    map->info.cur_set = map->info.set_list.next;
    while (map->info.cur_set)
    {
        tmp_entry = map->info.cur_set;
        map->info.cur_set = map->info.cur_set->next;

        framechannel_mfree(tmp_entry);
    }
	
    map = framechannel_map.next;
	
    while (map != &framechannel_map)
    {
        if (!strcmp(map->base_dir, base_dir))
        {
            map->prev->next = map->next;
            map->next->prev = map->prev;
            
            framechannel_mfree(map->base_dir);
            framechannel_mfree(map);
            
            break;
        }

        map = map->next;
    }
}


/**
 *  Get photo set of specified user id
 */
static void photoset_content_handler(void *user_data, const char *s, int len)
{
		framechannel_map_t	  *framechannel_map 	= (framechannel_map_t *) user_data;
		framechannel_info_t   *framechannel_info	= &framechannel_map->info;
		char	*title;
		int 	i, not_equal = 0, found = 0;	
		const char		*title_value	= NULL;
		char link[len+1];
	
		if (framechannel_info->state == FRAMECHANNEL_PHOTOLIST_LINKGET)
		{
			char link[len+1];
	
			memset(link, 0, len+1);
			strncpy(link, s, len);
			title_value = &link;
	
			/* TODO: workaround, skip '/' */
			while (strchr(title_value, '/'))
				title_value = strchr(title_value, '/') + 1;
	
			memset(&jpeg_info, 0, sizeof(jpeg_info));
			
			snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,"%s.JPG", title_value);
			snprintf(jpeg_info.url, NETFS_MAX_URL, "%s", link);

			MP_DEBUG("value = %s", title_value);
	
			sprintf(jpeg_info.length,"%d",0);		
#if 0 //cj not need to add file to netfs
		 netfs_add_file(&jpeg_info);			
#endif
		 Net_Xml_parseFileList(&jpeg_info);
	
		}

}

static void photoset_start_element_handler(void *user_data, const char *name, const char **attr)
{
    framechannel_map_t    *framechannel_map     = (framechannel_map_t *) user_data;
    framechannel_info_t   *framechannel_info    = &framechannel_map->info;
    const char      *id_value       = NULL;
    const char      *stat_value     = NULL;
    const char      *code_value     = NULL;

    if (framechannel_info->state == FRAMECHANNEL_CHANNEL_INIT)
    {
        if (!strcasecmp(name, "item"))
        {
			framechannel_info->state = FRAMECHANNEL_PHOTOLIST_ITEM;
        }
    }
	else if (framechannel_info->state == FRAMECHANNEL_PHOTOLIST_ITEM)
	{
		if (!strcasecmp(name, "link"))
		{
			framechannel_info->state = FRAMECHANNEL_PHOTOLIST_LINKGET;
		}
	}

}

static void photoset_end_element_handler(void *user_data, const char *name)
{
    framechannel_map_t    *framechannel_map     = (framechannel_map_t *) user_data;
    framechannel_info_t   *framechannel_info    = &framechannel_map->info;

	if (framechannel_info->state == FRAMECHANNEL_PHOTOLIST_LINKGET)
    {
        if (!strcasecmp(name, "link"))
            framechannel_info->state = FRAMECHANNEL_CHANNEL_INIT;
    }
}

static int framechannel_fetch_photosets(framechannel_map_t *framechannel_map)
{
    int                     ret;
    framechannel_info_t     *framechannel_info = &framechannel_map->info;

    /* Get Data from Remote Site and Parse it */
    framechannel_info->state = FRAMECHANNEL_CHANNEL_INIT;

    snprintf(framechannel_api_url, MAX_URL, 
             "http://%s/feeds/feed.php?user=%s&pin=%s",
             FRAMECHANNEL_HOST,
             framechannel_info->username, 
             framechannel_info->password);

    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(framechannel_api_url,NETFS_FRAMECHANNEL,0,0);
    if(ret < 0) 
        goto Recv_exit;	

    MPX_XML_Parse(framechannel_map
		               , photoset_start_element_handler
		               , photoset_end_element_handler
		               , photoset_content_handler);		

exit:

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

Recv_exit:
    goto exit;
}

/**
 *  Get photo set of specified user id
 */
static void photolist_content_handler(void *user_data, const char *s, int len)
{
    framechannel_map_t    *framechannel_map     = (framechannel_map_t *) user_data;
    framechannel_info_t   *framechannel_info    = &framechannel_map->info;
    char    *title;
    int     i, not_equal = 0, found = 0;	
    const char      *title_value    = NULL;
	char link[len+1];

	if (framechannel_info->state == FRAMECHANNEL_PHOTOLIST_LINKGET)
    {
    	char link[len+1];

		memset(link, 0, len+1);
		strncpy(link, s, len);
		title_value = &link;

		/* TODO: workaround, skip '/' */
		while (strchr(title_value, '/'))
			title_value = strchr(title_value, '/') + 1;

		memset(&jpeg_info, 0, sizeof(jpeg_info));
		
		snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,"%s.JPG", title_value);
	  	snprintf(jpeg_info.url, NETFS_MAX_URL, "%s", link);

	 	sprintf(jpeg_info.length,"%d",0);		
#if 0 //cj not need to add file to netfs
	 netfs_add_file(&jpeg_info);			
#endif
	 Net_Xml_parseFileList(&jpeg_info);

	}
}


/**
 *  Get photo list of specified photo set
 */
static void photolist_start_element_handler(void *user_data, const char *name, const char **attr)
{
    framechannel_map_t    *framechannel_map     = (framechannel_map_t *) user_data;
    framechannel_info_t   *framechannel_info    = &framechannel_map->info;
	    	
    if (framechannel_info->state == FRAMECHANNEL_PHOTOLIST_INIT)
    {
        if (!strcasecmp(name, "item"))
        {
			framechannel_info->state = FRAMECHANNEL_PHOTOLIST_ITEM;
        }
    }
#if 0
	else if (framechannel_info->state == FRAMECHANNEL_PHOTOLIST_ITEM)
	{
		if (!strcasecmp(name, "title"))
		{
			framechannel_info->state = FRAMECHANNEL_PHOTOLIST_TITLE;
		}
	}
	else if (framechannel_info->state == FRAMECHANNEL_PHOTOLIST_LINK)
#else
	else if (framechannel_info->state == FRAMECHANNEL_PHOTOLIST_ITEM)
#endif
	{
		if (!strcasecmp(name, "link"))
		{
			framechannel_info->state = FRAMECHANNEL_PHOTOLIST_LINKGET;
		}
	}
}

static void photolist_end_element_handler(void *user_data, const char *name)
{
    framechannel_map_t    *framechannel_map     = (framechannel_map_t *) user_data;
    framechannel_info_t   *framechannel_info    = &framechannel_map->info;

	if (framechannel_info->state == FRAMECHANNEL_PHOTOLIST_LINKGET)
    {
        if (!strcasecmp(name, "link"))
            framechannel_info->state = FRAMECHANNEL_PHOTOLIST_INIT;
    }
}

static int framechannel_fetch_photolists(framechannel_map_t *framechannel_map)
{
    int                     ret;
    framechannel_info_t     *framechannel_info = &framechannel_map->info;
  
    /* Get Data from Remote Site and Parse it */
    framechannel_info->state = FRAMECHANNEL_PHOTOLIST_INIT;

    snprintf(framechannel_api_url, MAX_URL, 
             "http://%s/feeds/feed.php?user=%s&pin=%s",
             FRAMECHANNEL_HOST,
             framechannel_info->username, 
             framechannel_info->password);


    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(framechannel_api_url,NETFS_FRAMECHANNEL,0,0);
    if(ret < 0) 
        goto Recv_exit;	

    MPX_XML_Parse(framechannel_map
		               , photolist_start_element_handler
		               , photolist_end_element_handler
		               , photolist_content_handler);		


    /* parse next page of photolist if it exist. */
    //if (framechannel_info->error_code > 0)
    //{
    //    ret = -NETFS_APP_ERROR;
    //    goto fatal_exit;
    //}

exit:

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

Recv_exit:
fatal_exit:
    goto exit;
}

static int framechannel_fetch_allphotolist(framechannel_map_t *framechannel_map)
{
    int                     ret;
    framechannel_info_t     *framechannel_info = &framechannel_map->info;

    /* Get Data from Remote Site and Parse it */
    framechannel_info->state = FRAMECHANNEL_CHANNEL_INIT;

    snprintf(framechannel_api_url, MAX_URL, 
             "http://%s/feeds/feed.php?user=%s&pin=%s",
             FRAMECHANNEL_HOST,
             framechannel_info->username, 
             framechannel_info->password);

    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(framechannel_api_url,NETFS_FRAMECHANNEL,0,0);
    if(ret < 0) 
        goto Recv_exit;

    MPX_XML_Parse(framechannel_map
		               , photolist_start_element_handler
		               , NULL
		               , NULL);	

    /* TODO: parse next page of photolist if it exist. */
    //if (framechannel_info->error_code > 0)
    //{
    //    ret = -NETFS_APP_ERROR;
    //    goto fatal_exit;
    //}

exit:

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

Recv_exit:
fatal_exit:
    goto exit;
}
#endif

#endif
