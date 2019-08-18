#define LOCAL_DEBUG_ENABLE  0 

//#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "global612.h"
#include "mpTrace.h"
//#include "heaputil_mem.h"
#include "netfs.h"
#include "netfs_pri.h"
#include "ndebug.h"
#include "xmlshoutcast.h"
#include "..\..\lwip\include\net_sys.h"
#include "..\..\..\..\libIPLAY\libSrc\netstream\INCLUDE\netstream.h"

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define shoutcast_malloc(sz)   mm_malloc(sz)
#define shoutcast_mfree(ptr)   mm_free(ptr)

extern long gtotaldata;
extern int iradio_wait;
extern int kcurindx;
#if (CHIP_VER_MSB == CHIP_VER_650)
extern unsigned char girbuf[];
#else
extern unsigned char *girbuf;
#endif
extern int shoutcast_conn;

#ifdef HAVE_SHOUTCAST

shoutcast_map_t *s_map;
extern Net_App_State App_State;
extern ST_NET_FILEENTRY * g_FileEntry;
extern ST_NET_FILEBROWSER *g_psNet_FileBrowser;
extern int gtotoalbuf;
extern unsigned char bgInternetRadioPause;
extern unsigned char irready;
#define MIN(a,b)		((a) < (b) ? (a) : (b))

static void shoutcast_stations_start_element_handler(void *user_data, const char *name, const char **attr)
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
							set_entry = (set_entry_t *) shoutcast_malloc(sizeof(set_entry_t));
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

static void shoutcast_stations_end_element_handler(void *user_data, const char *name)
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

static void shoutcast_stations_content_handler(void *user_data, const char *s, int len)
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


int shoutcast_init(const char *shoutcast_url, const char *base_dir)
{
    set_entry_t     *tmp_entry;
    
    int     ret;
    int     count;

	MP_DEBUG("shoutcast_init");
	MP_DEBUG1("%s",shoutcast_url);

    s_map = (shoutcast_map_t *) shoutcast_malloc(sizeof(shoutcast_map_t));
	
    if (!s_map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    memset(s_map, 0, sizeof(shoutcast_map_t));
    
    s_map->base_dir = (char *) shoutcast_malloc(strlen(base_dir) + 1);
    if (!s_map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    strcpy(s_map->base_dir, base_dir);
    
    s_map->prev = shoutcast_map.prev;
    s_map->next = &shoutcast_map;
    shoutcast_map.prev->next = s_map;
    shoutcast_map.prev = s_map;

    s_map->info.cur_set = &s_map->info.set_list;

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

	return ret;
fatal_exit:
	MP_DEBUG("shoutcast fatal_exit\n");
    if (s_map)
    {
        if (s_map->base_dir)
            shoutcast_mfree(s_map->base_dir);
        shoutcast_mfree(s_map);
    }

	return ret;
    //goto exit;

}

void shoutcast_exit(const char *base_dir)
{
     /* free resources allocated for all photoset */
    set_entry_t     *tmp_entry;	
    s_map->info.cur_set = s_map->info.set_list.next;
    while (s_map->info.cur_set)
    {
        tmp_entry = s_map->info.cur_set;
        s_map->info.cur_set = s_map->info.cur_set->next;

        shoutcast_mfree(tmp_entry);
    }
	
    s_map = shoutcast_map.next;
	
    while (s_map != &shoutcast_map)
    {
        if (!strcmp(s_map->base_dir, base_dir))
        {
            s_map->prev->next = s_map->next;
            s_map->next->prev = s_map->prev;
            
            shoutcast_mfree(s_map->base_dir);
            shoutcast_mfree(s_map);
            
            break;
        }

        s_map = s_map->next;
    }
}

static int shoutcast_fetch_stations(shoutcast_map_t *shcast_map,const char *shoutcast_url)
{
    int                     ret;
    shoutcast_info_t           *shoutcast_info    = &shcast_map->info;
	char retrycount = 0;
    DWORD previousTime = 0;
    DWORD currentTime = 0;

    /* Get Data from Remote Site and Parse it */
    
    shoutcast_info->state = SHOUTCAST_STATIONS_INIT;
    Xml_BUFF_init(NET_RECVHTTP);	
	
	MP_DEBUG("Net_Recv_Data NETFS_SHOUTCAST");
retrygetstations:
    ret = Net_Recv_Data(shoutcast_url,NETFS_SHOUTCAST,0,0);
    if(ret <= 0) 
	{
		previousTime = GetSysTime();
		if( retrycount < 5 )
		{
			retrycount++;
			currentTime = GetSysTime();
			if( currentTime > previousTime)
			{
				while( currentTime - previousTime < 100 )
				{
					MP_DEBUG("currentTime %d previousTime %d ",currentTime,previousTime);
					currentTime = GetSysTime();
				}
			}

			TaskYield();
			goto retrygetstations;
		}
    	goto Recv_exit;	
	}

	MPX_XML_Parse(shcast_map
		              , shoutcast_stations_start_element_handler
		              , shoutcast_stations_end_element_handler
		              , shoutcast_stations_content_handler);
	

    if (shoutcast_info->error_code > 0)
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

int iRadio_get_play_list(BYTE *id)
{
	//flickr_map_t *pmap;
    int     count;

	XML_Parser              parser;
    int                     ret;
    int                     len;
    http_info_t             http_info;
    http_header_handler_t   transfer_encoding_handler;
    char *data; 
    XML_BUFF_link_t *ptr;
	char pls_url[128];
	char httprequest[1024];
	char *ulrparm;
    int tcp_connection;
  	DWORD addr;
	BYTE *website;

	BYTE * ipaddr;
	BYTE HostStr[256];
    int num_read;
	//unsigned char buf[8192];
	int bufindex;
	char *FileSite,*FileSiteEND;
	char searchstr[16],searchstrfile[16],searchstrtitle[16];
	int i;
    char *bigger_buff;
	char retrycount = 0;
    DWORD previousTime = 0;
    DWORD currentTime = 0;

	//http://www.shoutcast.com/sbin/shoutcast-playlist.pls?rn=2350&file=filename.pls 

	MP_DEBUG("iRadio_get_play_list");    
    /* Get Data from Remote Site and Parse it */

    snprintf(pls_url, 128, "http://www.shoutcast.com/sbin/shoutcast-playlist.pls?rn=%s&file=filename.pls",id);
    Xml_BUFF_init(NET_RECVHTTP);	
retrygetpls:
    ret = Net_Recv_Data(pls_url,NETFS_SHOUTCAST,0,0);
    if(ret <= 0) 
	{
		previousTime = GetSysTime();
		if( retrycount < 5 )
		{
			retrycount++;
			currentTime = GetSysTime();
			if( currentTime > previousTime)
			{
				while( currentTime - previousTime < 100 )
				{
					MP_DEBUG("currentTime %d previousTime %d ",currentTime,previousTime);
					currentTime = GetSysTime();
				}
			}

			TaskYield();
			goto retrygetpls;
		}
    	goto Recv_PLS_exit;	
	}
	mpDebugPrint("%d\n",App_State.XML_BUF->buff_len);
#if 1
	if( App_State.XML_BUF->buff_len < 64 )
	{
		//App_State.XML_BUF[]
		//&& strcmp(App_State.XML_BUF,"Could not connect") == 0 )
			retrycount++;
			currentTime = GetSysTime();
			goto retrygetpls;
	}
#endif
    bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
	bigger_buff[len] = '\0';
	if( len < 64 )
	{
		if( strcmp(bigger_buff,"Could not connect") == 0 )
		{
			mpDebugPrint("bigger_buff %sAA",bigger_buff);
			ext_mem_free(bigger_buff);
			retrycount++;
			goto retrygetpls;
		}
	}
/*	
    Xml_BUFF_init(NET_RECVHTTP);	
    ret =Net_Recv_Data(httprequest,NETFS_SHOUTCAST_PLS,0,0);

    ptr = App_State.XML_BUF;
	MP_DEBUG1("Kevin %x",ptr);
	if( ptr != NULL )
		data =  ptr->BUFF;
*/
/*    
    while (ptr != NULL)
    {
    	data =  ptr->BUFF;
		len = ptr->buff_len;
		//mpDebugPrint("len = %d ",len); 
           	if (XML_Parse(parser, data, len, 0) == XML_STATUS_ERROR)
       	{      			
      			MP_DEBUG3("rss_init: %s at line %d, column %d\n",
            		XML_ErrorString(XML_GetErrorCode(parser)),
             		XML_GetCurrentLineNumber(parser),
             		XML_GetCurrentColumnNumber(parser));
	 	}	
		ptr = ptr->link;	
	}
*/
	//mpDebugPrint("Kevin %s\n",bigger_buff);
	FileSite = strstr(bigger_buff,"File1");
	i = 1;
	searchstr[0] = 0x0a;
	searchstr[1] = 0;
	sprintf(searchstrfile,"File%d",i);
	while( FileSite )
	{
		FileSiteEND = strstr(FileSite,searchstr);
      	memset((BYTE *)g_FileEntry->Link,0x00,MAX_NET_LINK_LEN);	
		memcpy((BYTE *)g_FileEntry->Link,FileSite+strlen(searchstrfile)+1,FileSiteEND-FileSite-strlen(searchstrfile)-1);
		sprintf(searchstrtitle,"Title%d",i);
		FileSite = strstr(bigger_buff,searchstrtitle);
		FileSiteEND = strstr(FileSite,searchstr);
		memset((BYTE *)g_FileEntry->Name,0x00,MAX_NET_NAME_LEN);
		//if( FileSiteEND )
		memcpy((BYTE *)g_FileEntry->Name,FileSite+strlen(searchstrtitle)+1,FileSiteEND-FileSite-strlen(searchstrtitle)-1); 
		
		memcpy((BYTE *)g_FileEntry->ExtName,"mp3",3); 
		//else
		//	memcpy((BYTE *)g_FileEntry->Name,FileSite,5); 
		mpDebugPrint("Name %s",g_FileEntry->Name);
		mpDebugPrint("Link %s",g_FileEntry->Link);
		g_FileEntry->dwIndex = i-1;
		g_FileEntry++;
		g_psNet_FileBrowser->dwNumberOfFile ++;
		i++;
		sprintf(searchstrfile,"File%d",i);
		FileSite = strstr(bigger_buff,searchstrfile);
		MP_DEBUG1("FileSite %x",FileSite);
	}
    /* parse next page of photolist if it exist. */
    //Xml_BUFF_free(NET_RECVHTTP);    	
Recv_PLS_exit:
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
	MP_DEBUG("KEvin EXIT");
    return ret;

	//return 0;		
	//EnableNetWareTask();
	//flickr_fetch_photolists();
/*
	pmap = map;
       map->info.cur_set = map->info.set_list.next;
       
	while(map->info.cur_set)
	{
		TaskYield();	//Kevin
		if(!strcasecmp(map->info.cur_set->title, PhotoSet))
		{
			ret = flickr_fetch_photolists(map);
            		if (ret < 0)
            		{
                		mpDebugPrint("error code: %d", map->info.error_code);
                		return ret;
            		}
			break;
		}	
		map->info.cur_set = map->info.cur_set->next;
	}
	map = pmap;
	//DisableNetWareTask();  
	TaskYield();	//Kevin
	return 0;
*/
}
#endif

int shoutcast_get_data(BYTE *buf,int len)
{
	SWORD ret;
	int len_ori;
	int dwTmp;
	DWORD *pdwSrc, *pdwTar;
	int numByteRead;
    //mpDebugPrint("shoutcast_get_data");
	if( gtotaldata > IRADIO_DATA_MAX )
	{
		/* ----------  too much queued data  ---------- */
		int off;
		kcurindx += ( gtotaldata - IRADIO_DATA_MAX );
		gtotaldata = IRADIO_DATA_MAX;	   /* TODO mutual exclustion ? */
		if( off = (kcurindx & 3) )
		{
			kcurindx += 4-off;
			gtotaldata -= 4-off;
		}
	}
	if( !iradio_wait && gtotaldata > IRADIO_DATA_LOW )
	{
		TaskYield();
		len_ori = len;
		len = MIN(len,gtotaldata);
		if( ((DWORD)buf & 0x3 == 0) && (len & 0x3 == 0) &&	(kcurindx & 3 == 0) )
		{
			pdwTar = (DWORD *) buf;
			pdwSrc = (DWORD *) &girbuf[kcurindx%(NETSTREAM_MAX_BUFSIZE)];
			for(dwTmp = len; dwTmp >= 4; dwTmp -= 4)
			{
				*pdwTar = *pdwSrc;
				pdwTar++;
				pdwSrc++;
			}
		}
		else
			memcpy(buf,&girbuf[kcurindx%(NETSTREAM_MAX_BUFSIZE)],len);
		numByteRead = len;
		kcurindx += len;
		gtotaldata -= len;
		//mpDebugPrint("FILE_TYPE_MP3 numByteRead %x len %x from %x %x real %x\n",numByteRead,len_ori,kcurindx,gtotoalbuf,kcurindx%(NETSTREAM_MAX_BUFSIZE));
		TaskYield();
	}
	else
	{
		//TaskYield();
		len = 0;
		iradio_wait = 1;
		mpDebugPrint("MIPS_MP3::iradio_wait");
#if HAVE_NETSTREAM
		SockIdSignalTcpFinRecvd(shoutcast_conn);
#endif
#if MAKE_XPG_PLAYER
		xpgPauseAudio();
#endif
		TaskYield();
	}
    return len;
}
