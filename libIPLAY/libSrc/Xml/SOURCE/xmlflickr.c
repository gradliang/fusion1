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
#if HAVE_FLICKR
#include "xmlflickr.h"
#endif

#include "..\..\lwip\include\net_sys.h"

#if HAVE_FLICKR
static struct netfs_file_entry jpeg_info;
flickr_map_t *map;

char  flickr_api_url[MAX_URL];
#endif

#define API_KEY     "1ce31b08a3301d54b7a660aee3503ab8"

#define FLICKR_HOST "api.flickr.com"

extern Net_App_State App_State;

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define flickr_malloc(sz)   mm_malloc(sz)
#define flickr_mfree(ptr)   mm_free(ptr)

#if 1
void MPX_XML_Parse(void * userdata
						,XML_StartElementHandler start_handler
						,XML_EndElementHandler end_handler
						,XML_CharacterDataHandler content_handler)
{
	extern Net_App_State App_State;
	
	int     len;
    	int isLast = FALSE;
    	XML_BUFF_link_t *ptr;
	
	XML_Parser parser = XML_ParserCreate(NULL);
	
	XML_SetUserData(parser, userdata);
    	XML_SetElementHandler(parser, start_handler, end_handler);
    	XML_SetCharacterDataHandler(parser, content_handler);

	MP_DEBUG("MPX_XML_Parse: got total data len = %d", App_State.dwTotallen);
	ptr = App_State.XML_BUF;
	while (ptr != NULL)
	{
		if (!ptr->link)
			isLast = TRUE;
		if (XML_Parse(parser, ptr->BUFF, ptr->buff_len, isLast) == XML_STATUS_ERROR)
    		{
        		MP_DEBUG3("MPX_XML_Parse: %s at line %d, column %d\n",
        		XML_ErrorString(XML_GetErrorCode(parser)),
        		XML_GetCurrentLineNumber(parser),
        		XML_GetCurrentColumnNumber(parser));
			break;
    		}
		
		ptr = ptr->link;
	}

	XML_ParserFree(parser);
	

}
#else
void MPX_XML_Parse(void * userdata
						,XML_StartElementHandler start_handler
						,XML_EndElementHandler end_handler
						,XML_CharacterDataHandler content_handler)
{
	extern Net_App_State App_State;
	
	int     len;
	char *bigger_buff; //weiching added
	
	XML_Parser parser = XML_ParserCreate(NULL);
	
	XML_SetUserData(parser, userdata);
    	XML_SetElementHandler(parser, start_handler, end_handler);
    	XML_SetCharacterDataHandler(parser, content_handler);

	bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
    	MP_DEBUG("MPX_XML_Parse: got total data len = %d", len);

//   	NetAsciiDump(bigger_buff, len);
    	if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
    	{
        	MP_DEBUG3("MPX_XML_Parse: %s at line %d, column %d\n",
        	XML_ErrorString(XML_GetErrorCode(parser)),
        	XML_GetCurrentLineNumber(parser),
        	XML_GetCurrentColumnNumber(parser));
    	}
		
	XML_Parse(parser, bigger_buff, 0, 1);	

	XML_ParserFree(parser);
	
	//weiching added
	if(bigger_buff != NULL)
        	ext_mem_free(bigger_buff);

}
#endif

#if	HAVE_FLICKR
int flickr_interesting_init(const char *base_dir)
{
    set_entry_t     *tmp_entry;
    
    int     ret;
    int     count;

    mpDebugPrint("request flickr interesting photo");
    map = NULL;
    map = (flickr_map_t *) flickr_malloc(sizeof(flickr_map_t));
    if (!map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    memset(map, 0, sizeof(flickr_map_t));
    
    map->base_dir = (char *) flickr_malloc(strlen(base_dir) + 1);
    if (!map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    strcpy(map->base_dir, base_dir);
    
    map->prev = flickr_map.prev;
    map->next = &flickr_map;
    flickr_map.prev->next = map;
    flickr_map.prev = map;


    map->info.api_key = API_KEY;
    map->info.cur_set = &map->info.set_list;

    count = 0;
    ret = flickr_fetch_interesting(map);
    if (ret < 0)
    {
        mpDebugPrint("error code: %d", map->info.error_code);
        return ret;
    }
    
    ret = NETFS_OK;

exit:
    /* free resources allocated for all photoset */
    map->info.cur_set = map->info.set_list.next;
    while (map->info.cur_set)
    {
        tmp_entry = map->info.cur_set;
        map->info.cur_set = map->info.cur_set->next;

        flickr_mfree(tmp_entry);
    }

    return ret;

fatal_exit:

    if (map)
    {
        if (map->base_dir)
            flickr_mfree(map->base_dir);
        flickr_mfree(map);
    }

    goto exit;
}


int flickr_PhotoList_Get(BYTE *PhotoSet)
{
	flickr_map_t *pmap;
	int     ret;
    	int     count;
		
	EnableNetWareTask();
	pmap = map;
       map->info.cur_set = map->info.set_list.next;
       
	while(map->info.cur_set)
	{
		if(!strcasecmp(map->info.cur_set->title, PhotoSet))
		{
#if 1	//add for GetNetNextPictureIndex
            ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
            struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
            DWORD dwOpMode = psSysConfig->dwCurrentOpMode;
            psFileBrowser->dwFileListIndex[dwOpMode] = 0;
            psFileBrowser->dwFileListCount[dwOpMode] = 0;
            psFileBrowser->dwFileListAddress[dwOpMode] = 0;
            psFileBrowser->dwFileListAddress[dwOpMode] = (DWORD) &psFileBrowser->sSearchFileList[0];
#endif		
			ret = flickr_fetch_photolists(map);
            		if (ret < 0)
            		{
                		mpDebugPrint("error code: %d", map->info.error_code);
                		return ret;
            		}
#if 1	//add for GetNetNextPictureIndex
            psFileBrowser->dwImgAndMovCurIndex = psFileBrowser->dwFileListIndex[dwOpMode];
            psFileBrowser->dwImgAndMovTotalFile = psFileBrowser->dwFileListCount[dwOpMode];
            psFileBrowser->sImgAndMovFileList = (ST_SEARCH_INFO *)psFileBrowser->dwFileListAddress[dwOpMode];
#endif            		
			break;
		}	
		map->info.cur_set = map->info.cur_set->next;
	}
	map = pmap;
	//DisableNetWareTask();  
	return 0;
}


int flickr_init(const char *username, const char *base_dir)
{
    set_entry_t     *tmp_entry;
    
    int     ret;
    int     count;

    mpDebugPrint("request flickr photo for '%s'", username);
    map = NULL;
    map = (flickr_map_t *) flickr_malloc(sizeof(flickr_map_t));
	
    if (!map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    memset(map, 0, sizeof(flickr_map_t));
    
    map->base_dir = (char *) flickr_malloc(strlen(base_dir) + 1);
    if (!map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    strcpy(map->base_dir, base_dir);
    
    map->prev = flickr_map.prev;
    map->next = &flickr_map;
    flickr_map.prev->next = map;
    flickr_map.prev = map;


    map->info.api_key = API_KEY;
    strncat(map->info.username, username, MAX_USERNAME);
    map->info.cur_set = &map->info.set_list;

    MP_DEBUG("1 flickr_fetch_userid  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    ret = flickr_fetch_userid(map);
    if (ret < 0)
    {
        mpDebugPrint("error code: %d", map->info.error_code);
        return ret;
    }
    MP_DEBUG("2 flickr_fetch_friend  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    ret = flickr_fetch_friend(map);
    if (ret < 0)
    {
        mpDebugPrint("friend list not available(error=%d)", map->info.error_code);
    }
    MP_DEBUG("3 flickr_fetch_photosets  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    ret = flickr_fetch_photosets(map);
    if (ret < 0)
    {
        mpDebugPrint("error code: %d", map->info.error_code);
        return ret;
    }

 	
    MP_DEBUG("4 flickr_fetch_photosets  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    count = 0;
    map->info.cur_set = map->info.set_list.next;
    tmp_entry = map->info.cur_set;

	
    if (tmp_entry)
    {
        while (tmp_entry)
        {
            MP_DEBUG2("Fetch photos from '%s' with id '%s'",tmp_entry->title,tmp_entry->id);
#if 0
	    //MP_DEBUG("5 flickr_fetch_photolists  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
           
	     ret = flickr_fetch_photolists(map);
            if (ret < 0)
            {
                mpDebugPrint("error code: %d", map->info.error_code);
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
        MP_DEBUG("No photo set, get all public photo");
	 MP_DEBUG("6 flickr_fetch_allphotolist  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");	
	 NetFileBrowerInitial();
        ret = flickr_fetch_allphotolist(map);
        if (ret < 0)
        {
            mpDebugPrint("error code: %d", map->info.error_code);
            return ret;
        }
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
            flickr_mfree(map->base_dir);
        flickr_mfree(map);
    }

    goto exit;
}

void flickr_exit(const char *base_dir)
{
    //flickr_map_t    *map = flickr_map.next;
    
     /* free resources allocated for all photoset */
    set_entry_t     *tmp_entry;	
    map->info.cur_set = map->info.set_list.next;
    while (map->info.cur_set)
    {
        tmp_entry = map->info.cur_set;
        map->info.cur_set = map->info.cur_set->next;

        flickr_mfree(tmp_entry);
    }
	
    map = flickr_map.next;
	
    while (map != &flickr_map)
    {
        if (!strcmp(map->base_dir, base_dir))
        {
            map->prev->next = map->next;
            map->next->prev = map->prev;
            
            flickr_mfree(map->base_dir);
            flickr_mfree(map);
            
            break;
        }

        map = map->next;
    }
}

/**
 * Get user id
 */
static void userid_start_element_handler(void *user_data, const char *name, const char **attr)
{
    flickr_map_t    *flickr_map     = (flickr_map_t *) user_data;
    flickr_info_t   *flickr_info    = &flickr_map->info;
    const char      *userid_value   = NULL;
    const char      *stat_value     = NULL;
    const char      *code_value     = NULL;

    if (flickr_info->state == FLICKR_USERID_INIT)
    {
        //MP_DEBUG2(" name = %s 0*attr = %s",name,*attr);
        if (!strcasecmp(name, "rsp"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "stat"))
                {
                    attr ++;
		      //MP_DEBUG1("1*attr = %s",*attr);
                    stat_value = *attr;
                }
                else
                {
                    attr ++;
		      //MP_DEBUG1("2*attr = %s",*attr);			
                }

                attr ++;
		  //MP_DEBUG1("3*attr = %s",*attr);		
            }
            
            if (stat_value && !strcasecmp(stat_value, "fail"))
            {
                flickr_info->state = FLICKR_ERROR;
            }
        }
        else if (!strcasecmp(name, "user"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "nsid"))
                {
                    attr ++;
		      //MP_DEBUG1("4*attr = %s",*attr);			
                    userid_value = *attr;
                }
                else
                {
                    attr ++;
			//MP_DEBUG1("5*attr = %s",*attr);		
                }

                attr ++;
		 // MP_DEBUG1("6*attr = %s",*attr);		
            }

            if (userid_value)
            {
                mpDebugPrint("ask userid '%s'", userid_value);
                strncat(flickr_info->userid, userid_value, MAX_USERID);
            }
        }
    }
    else if (flickr_info->state == FLICKR_ERROR)
    {
        if (!strcasecmp(name, "err"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "code"))
                {
                    attr ++;
			//MP_DEBUG1("7*attr = %s",*attr);		
                    code_value = *attr;
                }
                else
                {
                    attr ++;
			//MP_DEBUG1("8*attr = %s",*attr);		
                }
                
                attr ++;
		//MP_DEBUG1("9*attr = %s",*attr);		
            }
            
            if (code_value)
            {
                flickr_info->error_code = atoi(code_value);
            }
        }
    }
}

static int flickr_fetch_userid(flickr_map_t *flickr_map)
{
    int                     ret;
    flickr_info_t           *flickr_info    = &flickr_map->info;
    char *data; 

    /* Get Data from Remote Site and Parse it */
    flickr_info->state = FLICKR_USERID_INIT;

    snprintf(flickr_api_url, MAX_URL, 
             "http://%s/services/rest/?method=%s&username=%s&api_key=%s",
             FLICKR_HOST,
             "flickr.people.findByUsername", 
             flickr_info->username, 
             flickr_info->api_key);

    Xml_BUFF_init(NET_RECVHTTP);	    
    ret = Net_Recv_Data(flickr_api_url,NETFS_FLICKR,0,0);
    if(ret < 0) 
    	goto Recv_exit;	

    MPX_XML_Parse(flickr_map
		               , userid_start_element_handler
		               , NULL
		               , NULL);		


    if (flickr_info->error_code > 0)
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


/**
 *  Get photo set of specified user id
 */
static void photoset_content_handler(void *user_data, const char *s, int len)
{
    flickr_map_t    *flickr_map     = (flickr_map_t *) user_data;
    flickr_info_t   *flickr_info    = &flickr_map->info;
    char    *title;
    int     i;

    if (flickr_info->state == FLICKR_PHOTOSET_TITLE && flickr_info->cur_set)
    {
        title = flickr_info->cur_set->title;
        for (i = strlen(title); i < MAX_TITLE && len > 0; len--, i++)
            title[i] = *s++;
        title[i] = 0;
    }
}

static void photoset_start_element_handler(void *user_data, const char *name, const char **attr)
{
    flickr_map_t    *flickr_map     = (flickr_map_t *) user_data;
    flickr_info_t   *flickr_info    = &flickr_map->info;
    const char      *id_value       = NULL;
    const char      *stat_value     = NULL;
    const char      *code_value     = NULL;

    if (flickr_info->state == FLICKR_PHOTOSET_INIT)
    {
        if (!strcasecmp(name, "rsp"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "stat"))
                {
                    attr ++;
                    stat_value = *attr;
                }
                else
                {
                    attr ++;
                }

                attr ++;
            }
            
            if (stat_value && !strcasecmp(stat_value, "fail"))
            {
                flickr_info->state = FLICKR_ERROR;
            }
        }
        if (!strcasecmp(name, "photoset"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "id"))
                {
                    attr ++;
                    id_value = *attr;
                }
                else
                {
                    attr ++;
                }

                attr ++;
            }

            if (id_value)
            {
                set_entry_t    *set_entry;

                flickr_info->state = FLICKR_PHOTOSET_FOUND;
                set_entry = (set_entry_t *)flickr_malloc(sizeof(set_entry_t));
                memset(set_entry, 0, sizeof(set_entry_t));

                strncat(set_entry->id, id_value, MAX_SETID);

                flickr_info->cur_set->next = set_entry;
                flickr_info->cur_set = set_entry;
            }
        }
    }
    else if (flickr_info->state == FLICKR_PHOTOSET_FOUND)
    {
        if (!strcasecmp(name, "title"))
        {
            flickr_info->state = FLICKR_PHOTOSET_TITLE;
        }
    }
    else if (flickr_info->state == FLICKR_ERROR)
    {
        if (!strcasecmp(name, "err"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "code"))
                {
                    attr ++;
                    code_value = *attr;
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }
            
            if (code_value)
            {
                flickr_info->error_code = atoi(code_value);
            }
        }
    }
}

static void photoset_end_element_handler(void *user_data, const char *name)
{
    flickr_map_t    *flickr_map     = (flickr_map_t *) user_data;
    flickr_info_t   *flickr_info    = &flickr_map->info;

    if (flickr_info->state == FLICKR_PHOTOSET_TITLE)
    {
        if (!strcasecmp(name, "title"))
            flickr_info->state = FLICKR_PHOTOSET_FOUND;

    }
    else if (flickr_info->state == FLICKR_PHOTOSET_FOUND)
    {
        if (!strcasecmp(name, "photoset"))
            flickr_info->state = FLICKR_PHOTOSET_INIT;
    }
}

static int flickr_fetch_photosets(flickr_map_t *flickr_map)
{
    int                     ret;
    flickr_info_t           *flickr_info    = &flickr_map->info;
    char *data; 

    /* Get Data from Remote Site and Parse it */

    flickr_info->state = FLICKR_PHOTOSET_INIT;

    snprintf(flickr_api_url, MAX_URL, 
             "http://%s/services/rest/?method=%s&user_id=%s&api_key=%s",
             FLICKR_HOST,
             "flickr.photosets.getList", 
             flickr_info->userid, 
             flickr_info->api_key);

    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(flickr_api_url,NETFS_FLICKR,0,0);
    if(ret < 0) 
    	goto Recv_exit;	

    MPX_XML_Parse(flickr_map
		               , photoset_start_element_handler
		               , photoset_end_element_handler
		               , photoset_content_handler);	

    if (flickr_info->error_code > 0)
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


/**
 *  Get photo list of specified photo set
 */
static void photolist_start_element_handler(void *user_data, const char *name, const char **attr)
{
    flickr_map_t    *flickr_map     = (flickr_map_t *) user_data;
    flickr_info_t   *flickr_info    = &flickr_map->info;
    const char      *id_value       = NULL;
    const char      *secret_value   = NULL;
    const char      *server_value   = NULL;
    const char      *farm_value     = NULL;
    const char      *title_value    = NULL;
    const char      *stat_value     = NULL;
    const char      *code_value     = NULL;
    int len0= 0 , len1 =0 , len2 =0;	
    BYTE *str1,*str2,*str3;
    BYTE filesz[20];
    
    //MP_DEBUG2("name = %s *attr = %s",name, *attr);
	
    if (flickr_info->state == FLICKR_PHOTOLIST_INIT)
    {
        if (!strcasecmp(name, "rsp"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "stat"))
                {
                    attr ++;
                    stat_value = *attr;
                }
                else
                {
                    attr ++;
                }

                attr ++;
            }
            
            if (stat_value && !strcasecmp(stat_value, "fail"))
            {
                flickr_info->state = FLICKR_ERROR;
            }
        }
        if (!strcasecmp(name, "photoset"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "pages"))
                {
                    attr ++;
                }
                else if (!strcasecmp(*attr, "total"))
                {
                    attr ++;
                }
                
                attr ++;
            }
        }
        if (!strcasecmp(name, "photo"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "id"))
                {
                    attr ++;
                    id_value = *attr;
                }
                else if (!strcasecmp(*attr, "secret"))
                {
                    attr ++;
                    secret_value = *attr;
                }
                else if (!strcasecmp(*attr, "server"))
                {
                    attr ++;
                    server_value = *attr;
                }
                else if (!strcasecmp(*attr, "farm"))
                {
                    attr ++;
                    farm_value = *attr;
                }
                else if (!strcasecmp(*attr, "title"))
                {
                    attr ++;
                    title_value = *attr;
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }
            
            /* if title is empty, we use id as filename */
            if (title_value[0] == 0)
                title_value = id_value;

            /* TODO: workaround, skip '/' */
            while (strchr(title_value, '/'))
                title_value = strchr(title_value, '/') + 1;

            memset(&jpeg_info, 0, sizeof(jpeg_info));
            if (flickr_info->cur_set)
            {
                //snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,"/%s/%s/%s.JPG", flickr_map->base_dir, flickr_info->cur_set->title, title_value);
                snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,"%s.JPG", title_value);
 		  snprintf(jpeg_info.url, NETFS_MAX_URL, "http://farm%s.static.flickr.com/%s/%s_%s.jpg",
                        								farm_value, server_value, id_value, secret_value);
	     }
            else
            {
                //snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,"%s/%s.JPG", flickr_map->base_dir, title_value);
                snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,"%s.JPG", title_value);
		  snprintf(jpeg_info.url, NETFS_MAX_URL, "http://farm%s.static.flickr.com/%s/%s_%s.jpg",
                 								farm_value, server_value, id_value, secret_value);
	     }

	     //MP_DEBUG2("jpeg_info.pathname = %s,jpeg_info.url=%s",jpeg_info.pathname,jpeg_info.url);
	     sprintf(jpeg_info.length,"%d",0);		
#if 0 //cj not need to add file to netfs
	     netfs_add_file(&jpeg_info);            
#endif
	     Net_Xml_parseFileList(&jpeg_info);
		 
        }
    }
    else if (flickr_info->state == FLICKR_ERROR)
    {
        if (!strcasecmp(name, "err"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "code"))
                {
                    attr ++;
                    code_value = *attr;
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }
            
            if (code_value)
            {
                flickr_info->error_code = atoi(code_value);
            }
        }
    }
}

static int flickr_fetch_photolists(flickr_map_t *flickr_map)
{
    int                     ret;
    flickr_info_t           *flickr_info    = &flickr_map->info;
    char *data; 
    
    
    /* Get Data from Remote Site and Parse it */
    flickr_info->state = FLICKR_PHOTOLIST_INIT;

    snprintf(flickr_api_url, MAX_URL, 
             "http://%s/services/rest/?method=%s&user_id=%s&photoset_id=%s&api_key=%s",
             FLICKR_HOST,
             "flickr.photosets.getPhotos", 
             flickr_info->userid,
             flickr_info->cur_set->id, 
             flickr_info->api_key);

    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(flickr_api_url,NETFS_FLICKR,0,0);
    if(ret < 0) 
    	goto Recv_exit;	


    MPX_XML_Parse(flickr_map
		               , photolist_start_element_handler
		               , NULL
		               , NULL);		

    /* parse next page of photolist if it exist. */
    if (flickr_info->error_code > 0)
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

static int flickr_fetch_allphotolist(flickr_map_t *flickr_map)
{
    int                     ret;
    flickr_info_t           *flickr_info    = &flickr_map->info;
    char *data; 


    /* Get Data from Remote Site and Parse it */
    flickr_info->state = FLICKR_PHOTOLIST_INIT;

    snprintf(flickr_api_url, MAX_URL, 
             "http://%s/services/rest/?method=%s&user_id=%s&api_key=%s&per_page=%d",
             FLICKR_HOST,
             "flickr.people.getPublicPhotos", 
             flickr_info->userid,
             flickr_info->api_key,
             500);

    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(flickr_api_url,NETFS_FLICKR,0,0);
    if(ret < 0) 
    	goto Recv_exit;
    	
    MPX_XML_Parse(flickr_map
		               , photolist_start_element_handler
		               , NULL
		               , NULL);		

    /* TODO: parse next page of photolist if it exist. */
    if (flickr_info->error_code > 0)
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


/**
 *  Get friend list of specified user
 */
static void friend_start_element_handler(void *user_data, const char *name, const char **attr)
{
    flickr_map_t    *flickr_map     = (flickr_map_t *) user_data;
    flickr_info_t   *flickr_info    = &flickr_map->info;
    const char      *username_value = NULL;
    const char      *stat_value     = NULL;
    const char      *code_value     = NULL;

    if (flickr_info->state == FLICKR_FRIEND_INIT)
    {
        if (!strcasecmp(name, "rsp"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "stat"))
                {
                    attr ++;
                    stat_value = *attr;
                }
                else
                {
                    attr ++;
                }

                attr ++;
            }
            
            if (stat_value && !strcasecmp(stat_value, "fail"))
            {
                flickr_info->state = FLICKR_ERROR;
            }
        }
        if (!strcasecmp(name, "contact"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "username"))
                {
                    attr ++;
                    
                    username_value = *attr;
                }
                
                attr ++;
            }
        }

        if (username_value && flickr_map->contacts_length < MAX_CONTACTS)
        {
            int len = MAX_CONTACTS-flickr_map->contacts_length;
            char *contacts_tail = flickr_map->contacts+flickr_map->contacts_length;
            
            len = snprintf(contacts_tail, len, "%s\n", username_value);
            flickr_map->contacts_length += len;
        }
    }
    else if (flickr_info->state == FLICKR_ERROR)
    {
        if (!strcasecmp(name, "err"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "code"))
                {
                    attr ++;
                    code_value = *attr;
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }
            
            if (code_value)
            {
                flickr_info->error_code = atoi(code_value);
            }
        }
    }
}
 
static void friend_end_element_handler(void *user_data, const char *name)
{
    flickr_map_t    *flickr_map     = (flickr_map_t *) user_data;
    flickr_info_t   *flickr_info    = &flickr_map->info;

    if (flickr_info->state == FLICKR_FRIEND_INIT)
    {
        if (!strcasecmp(name, "contacts"))
        {
            snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME-1, "%s/friends.txt", 
                    flickr_map->base_dir);
            snprintf(jpeg_info.url, NETFS_MAX_URL-1, "mem://%x:0x%x/",
                    (unsigned int) flickr_map->contacts, flickr_map->contacts_length);
            snprintf(jpeg_info.length, 20, "%d", flickr_map->contacts_length);
	     mpDebugPrint("friend <= netfs add (pathname = %s, length = %s, url = %s)", jpeg_info.pathname,jpeg_info.length,jpeg_info.url);
#if 0 //cj not need to add file to netfs
		netfs_add_file(&jpeg_info);
#endif
        }

    }
}

static int flickr_fetch_friend(flickr_map_t *flickr_map)
{
    int                     ret;
    flickr_info_t           *flickr_info    = &flickr_map->info;
    char *data; 

    /* Get Data from Remote Site and Parse it */
    flickr_info->state = FLICKR_FRIEND_INIT;

    snprintf(flickr_api_url, MAX_URL, 
             "http://%s/services/rest/?method=%s&user_id=%s&api_key=%s",
             FLICKR_HOST,
             "flickr.contacts.getPublicList", 
             flickr_info->userid,
             flickr_info->api_key);

    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(flickr_api_url,NETFS_FLICKR,0,0);
    if(ret < 0) 
    	goto Recv_exit;

    MPX_XML_Parse(flickr_map
		               , friend_start_element_handler
		               , friend_end_element_handler
		               , NULL);		


exit:

    Xml_BUFF_free(NET_RECVHTTP);   	
    return ret;

Recv_exit:
fatal_exit:
    goto exit;
}

/**
 *  Get photo list of specified photo set
 */
static void interesting_start_element_handler(void *user_data, const char *name, const char **attr)
{
    flickr_map_t    *flickr_map     = (flickr_map_t *) user_data;
    flickr_info_t   *flickr_info    = &flickr_map->info;
    const char      *id_value       = NULL;
    const char      *secret_value   = NULL;
    const char      *server_value   = NULL;
    const char      *farm_value     = NULL;
    const char      *title_value    = NULL;
    const char      *stat_value     = NULL;
    const char      *code_value     = NULL;
    

    if (flickr_info->state == FLICKR_INTERESTING_INIT)
    {
        if (!strcasecmp(name, "rsp"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "stat"))
                {
                    attr ++;
                    stat_value = *attr;
                }
                else
                {
                    attr ++;
                }

                attr ++;
            }
            
            if (stat_value && !strcasecmp(stat_value, "fail"))
            {
                flickr_info->state = FLICKR_ERROR;
            }
        }
        if (!strcasecmp(name, "photo"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "id"))
                {
                    attr ++;
                    id_value = *attr;
                }
                else if (!strcasecmp(*attr, "secret"))
                {
                    attr ++;
                    secret_value = *attr;
                }
                else if (!strcasecmp(*attr, "server"))
                {
                    attr ++;
                    server_value = *attr;
                }
                else if (!strcasecmp(*attr, "farm"))
                {
                    attr ++;
                    farm_value = *attr;
                }
                else if (!strcasecmp(*attr, "title"))
                {
                    attr ++;
                    title_value = *attr;
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }
            
            /* if title is empty, we use id as filename */
            if (title_value[0] == 0)
                title_value = id_value;

            /* TODO: workaround, skip '/' */
            while (strchr(title_value, '/'))
                title_value = strchr(title_value, '/') + 1;

            memset(&jpeg_info, 0, sizeof(jpeg_info));
            if (flickr_info->cur_set)
            {
                snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,
                        "%s/%s/%s.JPG",
                        flickr_map->base_dir, flickr_info->cur_set->title, title_value);
                snprintf(jpeg_info.url, NETFS_MAX_URL,
                        "http://farm%s.static.flickr.com/%s/%s_%s.jpg",
                        farm_value, server_value, id_value, secret_value);
            }
            else
            {
                snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,
                        "%s/%s.JPG",
                        flickr_map->base_dir, title_value);
                snprintf(jpeg_info.url, NETFS_MAX_URL,
                        "http://farm%s.static.flickr.com/%s/%s_%s.jpg",
                        farm_value, server_value, id_value, secret_value);
            }
            //mpDebugPrint("interesting < ==== add photo '%s'  url => '%s'",jpeg_info.pathname, jpeg_info.url);
#if 0 //cj not need to add file to netfs
            netfs_add_file(&jpeg_info);
#endif			
        }
    }
    else if (flickr_info->state == FLICKR_ERROR)
    {
        if (!strcasecmp(name, "err"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "code"))
                {
                    attr ++;
                    code_value = *attr;
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }
            
            if (code_value)
            {
                flickr_info->error_code = atoi(code_value);
            }
        }
    }
}

static int flickr_fetch_interesting(flickr_map_t *flickr_map)
{
    int                     ret;
    flickr_info_t           *flickr_info    = &flickr_map->info;

    /* Get Data from Remote Site and Parse it */
    flickr_info->state = FLICKR_INTERESTING_INIT;

    snprintf(flickr_api_url, MAX_URL, 
             "http://%s/services/rest/?method=%s&per_page=500&api_key=%s",
             FLICKR_HOST,
             "flickr.interestingness.getList", 
             flickr_info->api_key);
    MP_DEBUG1("get interesting photo list: %s", flickr_api_url);

    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(flickr_api_url,NETFS_FLICKR,0,0);
    if(ret < 0) 
    		goto Recv_exit;

    MPX_XML_Parse(flickr_map
		               , interesting_start_element_handler
		               , NULL
		               , NULL);			 


    /* parse next page of photolist if it exist. */
    if (flickr_info->error_code > 0)
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

#endif

