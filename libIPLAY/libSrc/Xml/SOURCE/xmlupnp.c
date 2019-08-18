#define LOCAL_DEBUG_ENABLE 1



//#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "global612.h"
#include "mpTrace.h"
//#include "heaputil_mem.h"
#include "netfs.h"
#include "netfs_pri.h"
#include "xmlupnp.h"
#include "ndebug.h"
#include "..\..\lwip\include\net_sys.h"

#if NET_UPNP
upnp_map_t *upnp_s_map;

extern Net_App_State App_State;

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define upnp_malloc(sz)   mm_malloc(sz)
#define upnp_mfree(ptr)   mm_free(ptr)

extern ST_NET_FILEENTRY * g_FileEntry;
extern ST_NET_FILEBROWSER *g_psNet_FileBrowser;
#if 0
static void upnp_stations_start_element_handler(void *user_data, const char *name, const char **attr)
{
    shoutcast_map_t    *shcast_map     = (shoutcast_map_t *) user_data;
    shoutcast_info_t   *shcast_info    = &shcast_map->info;
    const char      *id_value       = NULL;
    const char      *stat_value     = NULL;
    const char      *code_value     = NULL;
	char title_name[256];
	char id[32];
	char mt[16];
	set_entry_t    *set_entry;
    if (shcast_info->state == SHOUTCAST_STATIONS_INIT)
    {
		if (!strcasecmp(name, "stationlist"))
		{
			shcast_info->state = SHOUTCAST_STATIONS_INIT_OK;
		}
    }
    else if (shcast_info->state == SHOUTCAST_STATIONS_INIT_OK)
	{
		//MP_DEBUG("OK %s",name);		
		if (!strcasecmp(name, "station"))
		{
			//
            while (*attr)
			{
				//MP_DEBUG("Attr %s",*attr);		
                if (!strcasecmp(*attr, "name")||!strcasecmp(*attr, "id")||!strcasecmp(*attr, "mt"))
				{
					//MP_DEBUG("name %s",*attr);
					if( !strcasecmp(*attr, "name") )
					{
						attr ++;
						strcpy(title_name,*attr);
						//MP_DEBUG("title_name %s",title_name);
					}
					else if(!strcasecmp(*attr, "mt"))
					{
						attr ++;
						strcpy(mt,*attr);
						//MP_DEBUG("mt %s",mt);

					}							
					else if(!strcasecmp(*attr, "id"))
					{
						attr ++;
						strcpy(id,*attr);
						//MP_DEBUG("id %s",id);
						shcast_info->state == SHOUTCAST_STATIONS_FOUND;
						{
							set_entry = (set_entry_t *) upnp_malloc(sizeof(set_entry_t));
							memset(set_entry, 0, sizeof(set_entry_t));
							if( strstr(mt,"aac") )
							{
								set_entry->streamtype = 1;			//AAC
							}
							else
							{
								set_entry->streamtype = 0;			//MP3
							}

							memcpy(set_entry->title,title_name,32);
							if( strlen(title_name) >= 32 )
							{
								strcpy((&set_entry->title[30]),"...");
							}
							memcpy(set_entry->id,id,32);
							shcast_info->cur_set->next = set_entry;
							shcast_info->cur_set = set_entry;
						}
					}
				}
				else
				{
					attr ++;
					//MP_DEBUG("what %s",*attr);
				}
                attr ++;
			}
			//if( shcast_info->state == SHOUTCAST_STATIONS_FOUND )
		}
	}
    else if (shcast_info->state == SHOUTCAST_STATIONS_FOUND)
    {
		MP_DEBUG("FOUND %s",name);		
        //if (!strcasecmp(name, "title"))
        {
            //flickr_info->state = FLICKR_PHOTOSET_TITLE;
        }
    }
    else if (shcast_info->state == SHOUTCAST_ERROR)
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
                shcast_info->error_code = atoi(code_value);
            }
        }
    }
}

static void upnp_stations_end_element_handler(void *user_data, const char *name)
{
    //shoutcast_map_t    *shcast_map = (shoutcast_map_t *) user_data;
    //shoutcast_info_t   *shcast_info    = &shcast_map->info;
	//MP_DEBUG("end_element_handler");
/*
    if (shcast_info->state == SHOUTCAST_STATIONS_TITLE)
    {
        if (!strcasecmp(name, "title"))
            shcast_info->state = SHOUTCAST_STATIONS_FOUND;

    }
    else if (shcast_info->state == SHOUTCAST_STATIONS_FOUND)
    {
        if (!strcasecmp(name, "photoset"))
            shcast_info->state = SHOUTCAST_STATIONS_INIT;
    }
*/
}

static void upnp_stations_content_handler(void *user_data, const char *s, int len)
{
    shoutcast_map_t    *shcast_map = (shoutcast_map_t *) user_data;
    shoutcast_info_t   *shcast_info    = &shcast_map->info;
    char    *title;
    int     i;
	//MP_DEBUG("shoutcast_stations_content_handler");
	if( shcast_info->state == SHOUTCAST_STATIONS_FOUND )
		shcast_info->state = SHOUTCAST_STATIONS_INIT_OK;
/*
    if (shcast_info->state == FLICKR_PHOTOSET_TITLE && flickr_info->cur_set)
    {
        title = flickr_info->cur_set->title;
        for (i = strlen(title); i < MAX_TITLE && len > 0; len--, i++)
            title[i] = *s++;
        title[i] = 0;
    }
*/
}
#endif

int upnp_fs_init( const char *base_dir )
{
    set_entry_t     *tmp_entry;
    
    int     ret;
    int     count;

	MP_DEBUG("upnp_fs_init");

    upnp_s_map = (upnp_map_t *) upnp_malloc(sizeof(upnp_map_t));
	
    if (!upnp_s_map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    memset(upnp_s_map, 0, sizeof(upnp_map_t));
    
    upnp_s_map->base_dir = (char *) upnp_malloc(strlen(base_dir) + 1);
    if (!upnp_s_map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    strcpy(upnp_s_map->base_dir, base_dir);
    upnp_s_map->prev = upnp_s_map;
    upnp_s_map->next = upnp_s_map;
/*
    s_map->prev = upnp_map.prev;
    s_map->next = &upnp_map;
    upnp_map.prev->next = s_map;
    upnp_map.prev = s_map;
*/
    upnp_s_map->info.cur_set = &upnp_s_map->info.set_list;
	Upnp_Browse_root_dir(upnp_s_map);

/*
	shoutcast_fetch_stations(s_map,shoutcast_url);
    s_map->info.cur_set = s_map->info.set_list.next;
    tmp_entry = s_map->info.cur_set;
	count = 0;
    if (tmp_entry)
    {
		MP_DEBUG("Kevin AAAAAAA");
        while (tmp_entry)
        {

			TaskYield();	//Kevin
			MP_DEBUG2("Fetch iRadio from '%s' with id '%s'",tmp_entry->title,tmp_entry->id);
			tmp_entry->title[32] = '\0';
			Net_Xml_PhotoSetList_AND_Id(tmp_entry->title,tmp_entry->id,tmp_entry->streamtype,count);
			count ++;
            tmp_entry = tmp_entry->next;
			if( count > 100 )
				break;
		}
		Net_PhotoSet_SetCount(count);	
	}
*/
	return ret;
fatal_exit:
    if (upnp_s_map)
    {
        if (upnp_s_map->base_dir)
            upnp_mfree(upnp_s_map->base_dir);
        upnp_mfree(upnp_s_map);
    }
	return ret;
    //goto exit;

}
#endif
