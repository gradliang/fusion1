
#define LOCAL_DEBUG_ENABLE  0 

#include <string.h>
#include <expat.h>
#include "global612.h"
#include "mpTrace.h"
//#include "..\..\curl\include\net_curl_curl.h"
//#include "heaputil_mem.h"
#include "netfs.h"
#include "netfs_pri.h"
#include "ndebug.h"
#include "xmlvtuner.h"
#include "..\..\netstream\include\blowfish.h"
#include "..\..\lwip\include\net_sys.h"
#include "..\..\CURL\include\net_curl_curl.h"
#include "..\..\libIPLAY\libsrc\demux\include\filetype.h"

#define VTUNER_SEARCH_LEVEL_MAX 5
#define VTUNER_SEARCH_AREA_MAX 20
#define VTUNER_SEARCH_AREA_LEVEL1_MAX 100
#define VTUNER_ALLSTATIONS_MAX 100

#ifndef SUCCESS
#define SUCCESS	0
#endif

#ifndef MIN
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#endif

void NetAsciiDump(unsigned long address, unsigned long size);
static size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *stream);

#define vtuner_malloc(sz)   ext_mem_malloc(sz)
#define vtuner_mfree(ptr)   ext_mem_free(ptr)
vtuner_map_t *s_map;
extern Net_App_State App_State;
extern ST_NET_FILEENTRY * g_FileEntry;
extern ST_NET_FILEBROWSER *g_psNet_FileBrowser;
//extern ST_NET_FILEBROWSER Net_FileBrowser;

//char Elementname[128];
char vtuner_id[32];
//char pls_url[256];
char station_name[128];
char title_name[64];
char menu_id = 0;
char menu_level = 0;

char search_level = 0;
char search_area = 0;
char search_area1 = 0;
unsigned char cookie_num[128];

//vtuner_search_t vsearch_area[VTUNER_SEARCH_AREA_MAX];
//vtuner_search_t vsearch_area_level1[VTUNER_SEARCH_AREA_LEVEL1_MAX];

char vtuner_mac_info[96];
static html_parser_t vtuner_html_parser;
struct SessionHandle *g_vtuner_curl;


extern BYTE myethaddr[6];
extern ST_FEED Vtuner_Feed[];
extern ST_FEED Vtuner_Location_Feed[];
//extern ST_FEED Vtuner_Location_List_Feed[];
unsigned char html_capture_flag = 0;
unsigned char vtuner_connect_stste = 0;
char station_location[256];
char station_location_id[64];
char station_location_mime[32];
char station_location_name[64];
//extern int audio_type;

#if HAVE_VTUNER
/****************************************************************************
 **
 ** NAME:           StrToInt
 **
 ** PARAMETERS:     pointer of charactor
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Convert a HEX string to a unsigned interger.
 **
 ****************************************************************************/
DWORD VtunerStrToInt(BYTE * ptr)
{
    SDWORD       i, val = 0;
    BYTE       c;
	
    if(ptr[0] == '0' && ptr[1] == 'x')  //Hex
    {
        for(i = 2; i < 10; i++)
        {
            c = ptr[i];
            if(c >= '0' && c <= '9')
                c -= '0';
            else if(c >= 'A' && c <= 'F')
                c = c - 'A' + 10;
            else if(c >= 'a' && c <= 'f')
                c = c - 'a' + 10;
            else
                break;
	
            val = val * 16 + c;
}
    }
    else
    {
        for(i = 0; i < 10; i++)
{
            c = ptr[i];
            if(c >= '0' && c <= '9')
                c -= '0';
            else
                break;
	
            val = val * 10 + c;
}
    }
    return (val);
}

static void vtuner_desc_content_handler(void *user_data, char *content, int len)
{
	//mpDebugPrint("vtuner_desc_content_handler %d",len); 	

	BYTE *url_referer="company.vtuner.com/setupapp/sample/asp/Favorites/BrowseFavorites.asp?sFavoriteGroupName=";
    vtuner_map_t    *vtuner_map = (vtuner_map_t *) user_data;
    vtuner_info_t   *vtuner_info    = &vtuner_map->info;
	set_entry_t    *set_entry;
	int i,j;
	BYTE go = 0;
	
	if (!strcasecmp(content, "My Favourite Groups"))
	{
		html_capture_flag = 1;
	}
	
	switch(html_capture_flag)
	{
	
		case 10://space
		case 8:
		case 6:
		case 4:
			html_capture_flag++;
			//mpDebugPrint("%s",content); 
			break;
		case 11://stream
			//mpDebugPrint("%s",content); 
			memset(station_location_mime,0x00,32);
			memcpy(station_location_mime,content,strlen(content));
			html_capture_flag++;
			break;
		case 9://genre
		case 7://location
			html_capture_flag++;
			break;
		case 5://name
			//mpDebugPrint("%s",content); 
			memset(station_location_name,0x00,64);
			memcpy(station_location_name,content,strlen(content));
			html_capture_flag++;
			break;
		case 3:
			set_entry = (set_entry_t *)mem_malloc(sizeof(set_entry_t));
			memset(set_entry, 0, sizeof(set_entry_t));
			memcpy(set_entry->title,content,strlen(content));
			if( strlen(content) >= 32 )
			{
				strcpy((&set_entry->title[30]),"...");
			}
			memcpy(url_referer+strlen(url_referer),content,strlen(content));
			memcpy(set_entry->url,url_referer,strlen(url_referer));
			memcpy(set_entry->id,&menu_id,1);
			menu_id++;
			//MP_DEBUG2("======================= station_name = %s, vtuner_id= %s ",station_name,vtuner_id);
			vtuner_info->cur_set->next = set_entry;
			vtuner_info->cur_set = set_entry;
			
			//mpDebugPrint("%s",set_entry->title); 
			html_capture_flag = 1;
			break;
		case 12:
			//mpDebugPrint("%s",content); 
			set_entry = (set_entry_t *)mem_malloc(sizeof(set_entry_t));
			memset(set_entry, 0, sizeof(set_entry_t));
			memcpy(set_entry->title,station_location_name,strlen(station_location_name));
			if( strlen(content) >= 32 )
			{
				strcpy((&set_entry->title[30]),"...");
			}
			memcpy(set_entry->mime,station_location_mime,strlen(station_location_mime));
			memcpy(set_entry->id,station_location_id,strlen(station_location_id));
			//MP_DEBUG2("======================= station_name = %s, vtuner_id= %s ",station_name,vtuner_id);
			vtuner_info->cur_set->next = set_entry;
			vtuner_info->cur_set = set_entry;
			html_capture_flag = 1;
			break;
			
		
	}

}

static void vtuner_desc_tag_start_handler(void *user_data, char *tag_name, char **attr)
{
    BYTE go = 0;
	BYTE copy_id = 0;
	BYTE copy_name = 0;
    BYTE *tmp;
    BYTE *tmpname;
    BYTE *tmplocation;
	int i,j;

    tmp = vtuner_malloc(256);
	tmpname = vtuner_malloc(64);
	tmplocation=vtuner_malloc(256);
	//mpDebugPrint("vtuner_desc_tag_start_handler %d",vtuner_connect_stste); 	
	switch(vtuner_connect_stste)
	{
		case VTUNER_CONNECT_HOME_PAGE_GET_STATION:
		//mpDebugPrint("--> %s",tag_name);

		if (!strcasecmp(tag_name, "a"))
		  {
		      memset(tmp,0x00,256);
		  while (*attr)
		  {  
		  
			  if(go == 1)
			  {
				 //mpDebugPrint("Attr %s",*attr); 	  
				 memcpy(tmp,*attr,strlen(*attr));
				 for(i=0;i<strlen(*attr);i++)
			 	{
			 	    if((tmp[i]=='&')&&(tmp[i+1]=='i')&&(tmp[i+2]=='d')&&(tmp[i+3]=='='))
			 	    {
						//mpDebugPrint(" %2c ",tmp[i]);		 
						memset(station_location_id,0x00,64);
						i+=4;
						copy_id = 1;
						j=0;
			 	    }
					if(copy_id)
					{
					 station_location_id[j] = tmp[i];
					 j++;
					}
					
			 	    if((tmp[i]=='&')&&(tmp[i+1]=='s')&&(tmp[i+2]=='S')&&(tmp[i+3]=='t')&&
						(tmp[i+4]=='a')&&(tmp[i+5]=='t')&&(tmp[i+6]=='i')&&(tmp[i+7]=='o')&&
						(tmp[i+8]=='n')&&(tmp[i+9]=='N')&&(tmp[i+10]=='a')&&(tmp[i+11]=='m')&&
						(tmp[i+12]=='e')&&(tmp[i+13]=='='))
			 	    	{
			 	    	  i+=14;
						  copy_name = 1;
						  j=0;
			 	    	}
					
					if(copy_name)
					{
					 if(tmp[i] == '&')
					 	break;
					 tmpname[j] = tmp[i];
					 j++;
					}
					
			 	}
				 
				 if(copy_name)
				 {
				 	mpDebugPrint("copy_name %s",tmpname); 	  
					html_capture_flag = 4;
				 }
				 go = 0;
			  }
			  if (!strcasecmp(*attr, "href"))
			  {
			    go = 1;
			  }
			  attr ++;
		  }
		}
		break;
	    case VTUNER_CONNECT_HOME_PAGE_GET_LIST:
			if (!strcasecmp(tag_name, "font"))
				{
					if(html_capture_flag == 1)
					{
						html_capture_flag = 2;
					}
				}

			if (!strcasecmp(tag_name, "A"))
				{
					if(html_capture_flag == 2)
					html_capture_flag=3;
				}
			break;
		case VTUNER_CONNECT_HOME_PAGE_GET_STATION_LOCATION:
			memset(station_location,0x00,256);
			//mpDebugPrint("--> %s",tag_name);
			while (*attr)
			{  
				//mpDebugPrint("Attr %s",*attr); 	
				if (!strcasecmp(*attr, "HREF"))
				{
				   attr ++;
				   memcpy(station_location,*attr,strlen(*attr));
			   	   //mpDebugPrint("location %s",*attr);    
				
				}
				attr ++;
			}
			break;
	}
	vtuner_mfree(tmp);
	vtuner_mfree(tmpname);
	vtuner_mfree(tmplocation);

}

static void vtuner_desc_tag_end_handler(void *user_data, char *tag_name)
{
	//mpDebugPrint("vtuner_desc_tag_end_handler"); 	
	
	//mpDebugPrint("<--  %s",tag_name); 	
}

static void vtuner_stations_content_handler(void *user_data, char *s, int len)
{
    vtuner_map_t    *vtuner_map = (vtuner_map_t *) user_data;
    vtuner_info_t   *vtuner_info    = &vtuner_map->info;
    char    *title;
    int     i=0,j=0;
	char *vtuner_token_url;
	set_entry_t    *set_entry;
	//char mime[32];
   	mpDebugPrint("vtuner_stations_content_handler %d",len);
#if	0
    mpDebugPrint("%s",Elementname);
   if (!strcasecmp(Elementname, "Title"))
   	{
   	
	   memset(title_name,0x00,64);
   	   memset(vsearch_area[search_area].title,0x00,64);
   	   memset(vsearch_area_level1[search_area1].title,0x00,64);
	   i=0;
	   while(s[i]!=0x3c)
		{
		  //mpDebugPrint("%2c ",s[i]);
		  title_name[i]=s[i]; 
		  if(search_level == 0)
		  	vsearch_area[search_area].title[i]=s[i];
		  else if(search_level == 1)
		  	vsearch_area_level1[search_area1].title[i]=s[i];
		  i++;
		} 
	   
   	}
   
   if (!strcasecmp(Elementname, "StationMime"))
   	{
   	    //memset(vtuner_mime,0x00,32);
		i=0;
		while(s[i]!=0x3c)
		 {
		   //mpDebugPrint("%2c ",s[i]);
		   //vtuner_mime[i] = s[i];
		   i++;
		 }
		
//		if((s[0]=='M')&&(s[1]=='P')&&(s[2]=='3'))
		//if((s[0]=='W')&&(s[1]=='M')&&(s[2]=='A'))
		{
		
		    //if((!strcasecmp(vtuner_id, "1568"))||(!strcasecmp(vtuner_id, "7252")))
		    //if((!strcasecmp(vtuner_id, "8080"))||(!strcasecmp(vtuner_id, "7252")))
		    if(entry_cnt < VTUNER_ALLSTATIONS_MAX)
	  		{
				set_entry = (set_entry_t *)vtuner_malloc(sizeof(set_entry_t));

				entry_cnt++;

				memset(set_entry, 0, sizeof(set_entry_t));
				memcpy(set_entry->title,station_name,32);
				
				if( strlen(station_name) >= 32 )
				{
					strcpy((&set_entry->title[30]),"...");
				}
				memcpy(set_entry->id,vtuner_id,strlen(vtuner_id));
				
				//mpDebugPrint("vtuner_id %d ",strlen(vtuner_id));
				//mpDebugPrint("set_entry->id %d ",strlen(set_entry->id));
				//mpDebugPrint("XXX-%s",s);
 			    memcpy(set_entry->url,pls_url,strlen(pls_url));
                if (len < sizeof set_entry->mime);
                {
                    memcpy(set_entry->mime,s,len);
                    set_entry->mime[len] = '\0';
                }
                //MP_DEBUG2("======================= station_name = %s, vtuner_id= %s ",station_name,vtuner_id);
				vtuner_info->cur_set->next = set_entry;
				vtuner_info->cur_set = set_entry;
	  		}
		}
		
   	}
   
   if (!strcasecmp(Elementname, "StationUrl"))
   	{
   	
		memset(pls_url,0x00,256);
		i=0;
		j=0;
		while(s[i]!=0x3c)
		{
		   
		   if((s[i]=='a')&&(s[i+1]=='m')&&(s[i+2]=='p'))
		   {
		     i+=4;
		   }
    	   pls_url[j] = s[i];
		   i++;
		   j++;
		 } 
		 pls_url[j] = 0;
	
	 // mpDebugPrint("%s ",pls_url);
	
   	}
   
   if (!strcasecmp(Elementname, "UrlDir"))
   	{
   	    memset(pls_url,0x00,256);
		i=0;
		j=0;
		while(s[i]!=0x3c)
		{
		   
		   if((s[i]=='&')&&(s[i+1]=='a')&&(s[i+2]=='m')&&(s[i+3]=='p'))
		   {
		     break;
		   }
    	   pls_url[j] = s[i];
		   
		   if(search_level == 0)
		   	vsearch_area[search_area].url[j] = s[i];
		   else if(search_level == 1)
			   vsearch_area_level1[search_area1].url[j] = s[i];
		   i++;
		   j++;
		 } 
		 pls_url[j] = 0;
	
	   if(search_level == 0)
		  search_area++;
	   else if(search_level == 1)
	      search_area1++;
	  //mpDebugPrint("%s ",pls_url);
	
   	}
   if (!strcasecmp(Elementname, "StationId"))
   	{
   		memset(vtuner_id,0x00,32);
   	
		i=0;
		while((s[i]!=0x3c)&&((s[i]>='0')&&(s[i]<='9')))
		 {
		   //mpDebugPrint("%2c ",s[i]);
		   vtuner_id[i] = s[i];
		   i++;
		 } 
		
		vtuner_id[i] = 0;
		
   	}
   
   if (!strcasecmp(Elementname, "StationName"))
   	{
   	
	   memset(station_name,0x00,128);
		i=0;
		while(s[i]!=0x3c)
		 {
		   //mpDebugPrint("%2c ",s[i]);
		   station_name[i] = s[i];
		   i++;
		 } 
		station_name[i] = 0;
		
   	}
    memset(Elementname,0,strlen(Elementname)+1);
	#endif
}

static void vtuner_start_element_handler(void *user_data, char *name, char **attr)
{
    	vtuner_map_t    *vtuner_map = (vtuner_map_t *) user_data;
    	vtuner_info_t   *vtuner_info    = &vtuner_map->info;
	switch(menu_level)
   	{
	  case 0:
	  case 1:
	  case 2:
		if(!strcasecmp(name,"Title"))
			vtuner_info->state = VTUNER_GET_LOCATION_TITLE;
		else if(!strcasecmp(name,"UrlDir"))
			vtuner_info->state = VTUNER_GET_LOCATION_URL;
		else if(!strcasecmp(name,"ItemCount"))
			vtuner_info->state = VTUNER_GET_ITEMCOUNT;
		break;
	  case 3:
	  	if(!strcasecmp(name,"ItemCount"))
			vtuner_info->state = VTUNER_GET_ITEMCOUNT;
		else if(!strcasecmp(name,"StationId"))
		{
		  vtuner_info->state = VTUNER_GET_STATION_ID;
		} 
		else if(!strcasecmp(name,"StationName"))
		 {
		  vtuner_info->state = VTUNER_GET_STATION_NAME;
   	}
		else if(!strcasecmp(name,"StationUrl"))
   	{
		  vtuner_info->state = VTUNER_GET_STATION_URL;
	  	}
		else if(!strcasecmp(name,"StationMime"))
		 {
		  vtuner_info->state = VTUNER_GET_STATION_MLME;
	  	}
		//mpDebugPrint("vtuner_location_element_handler %s",name);  
	  	break;
		 }
		
			}
			
static void vtuner_end_element_handler(void *user_data, char *name)
			{
    vtuner_map_t    *vtuner_map     = (vtuner_map_t *) user_data;
    vtuner_info_t   *vtuner_info    = &vtuner_map->info;
	switch(menu_level)
				{
	  case 3:
	  	//mpDebugPrint("vtuner_end_element_handler %s",name); 	
	  break;
				}
				
					
	  		} 		
			
static void vtuner_content_handler(void *user_data, char *s, int len)
		{
    	vtuner_map_t    *vtuner_map = (vtuner_map_t *) user_data;
    	vtuner_info_t   *vtuner_info    = &vtuner_map->info;
    	char    *title;
    int     i=0,j=0;
	char *vtuner_token_url;
	set_entry_t    *set_entry;
	int item_cnt=0;
	switch(menu_level)
			{
	    case 0:
		case 1:
		case 2:	
		if(vtuner_info->state == VTUNER_GET_LOCATION_TITLE)
		{
		  switch(menu_level)
			{
			  case 0:
			  	if(Vtuner_Feed[menu_id].pTitle == NULL)
			  	Vtuner_Feed[menu_id].pTitle = vtuner_malloc(32);
				memset(Vtuner_Feed[menu_id].pTitle,0x00,32);
				memcpy(Vtuner_Feed[menu_id].pTitle,s,len); 
				mpDebugPrint("Vtuner_Feed[%d].pTitle %s",menu_id,Vtuner_Feed[menu_id].pTitle);
				break;
			  case 1:
			  	if(Vtuner_Location_Feed[menu_id].pTitle == NULL)
			  	Vtuner_Location_Feed[menu_id].pTitle = vtuner_malloc(32);
				memset(Vtuner_Location_Feed[menu_id].pTitle,0x00,32);
				memcpy(Vtuner_Location_Feed[menu_id].pTitle,s,len); 
				mpDebugPrint("Vtuner_Location_Feed[%d].pTitle %s",menu_id,Vtuner_Location_Feed[menu_id].pTitle);
				break;
			 case 2:
#if		0	 	
			    Vtuner_Location_List_Feed[menu_id].pTitle = vtuner_malloc(32);
			    memset(Vtuner_Location_List_Feed[menu_id].pTitle,0x00,32);
			    memcpy(Vtuner_Location_List_Feed[menu_id].pTitle,s,len); 
			    mpDebugPrint("Vtuner_Location_List_Feed[%d].pTitle %s",menu_id,Vtuner_Location_List_Feed[menu_id].pTitle);
#endif				
			    break;
					
			
		  	}
		  if(vtuner_info->state == VTUNER_GET_LOCATION_TITLE)
			  vtuner_info->state = VTUNER_GET_MENU;
		}
		else if(vtuner_info->state == VTUNER_GET_LOCATION_URL)
	  		{
				
		  switch(menu_level)
				{
		    case 0:
				if(Vtuner_Feed[menu_id].pLink == NULL)
			  Vtuner_Feed[menu_id].pLink = vtuner_malloc(128);
			  memset(Vtuner_Feed[menu_id].pLink,0x00,128);
			  memcpy(Vtuner_Feed[menu_id].pLink,s,len); 
			  mpDebugPrint("Vtuner_Feed[%d].pLink %s",menu_id,Vtuner_Feed[menu_id].pLink);
			  break;
				
		    case 1:
				if(Vtuner_Location_Feed[menu_id].pLink == NULL)
			  Vtuner_Location_Feed[menu_id].pLink = vtuner_malloc(128);
			  memset(Vtuner_Location_Feed[menu_id].pLink,0x00,128);
			  memcpy(Vtuner_Location_Feed[menu_id].pLink,s,len); 
#if 1
			  vtuner_change_level(Vtuner_Location_Feed[menu_id].pLink,len);

       		  //strncat(Vtuner_Location_Feed[menu_id].pLink,"-AllStations",12);
#endif
			  //mpDebugPrint("Vtuner_Location_Feed[%d].pLink %s",menu_id,Vtuner_Location_Feed[menu_id].pLink);
			  break;
					
	 	  case 2:
#if	0	  	
	 		Vtuner_Location_List_Feed[menu_id].pLink = vtuner_malloc(128);
	 		memset(Vtuner_Location_List_Feed[menu_id].pLink,0x00,128);
	 		memcpy(Vtuner_Location_List_Feed[menu_id].pLink,s,len); 
	 		mpDebugPrint("Vtuner_Location_Feed[%d].pLink %s",menu_id,Vtuner_Location_List_Feed[menu_id].pLink);
#endif			
	 		break;
		}
		  menu_id++;
		  if(vtuner_info->state == VTUNER_GET_LOCATION_URL)
			  vtuner_info->state = VTUNER_GET_MENU;
	  		} 		
		else if(vtuner_info->state == VTUNER_GET_ITEMCOUNT)
   	{
   	          item_cnt = VtunerStrToInt(s);
		      mpDebugPrint("VTUNER_GET_ITEMCOUNT %d",item_cnt);
			  Net_Vtuner_SetTotalIndex(item_cnt);
			  vtuner_info->state = VTUNER_GET_MENU;
		}
		break;
        case 3:		
		 //mpDebugPrint("vtuner_content_handler len %d %s",len,s);   
		 //mpDebugPrint("vtuner_info->state %d ",vtuner_info->state);   
		
		 if(vtuner_info->state == VTUNER_GET_ITEMCOUNT)
   	{
			   //Net_FileBrowser.dwNumberOfFile = VtunerStrToInt(s);
			   Net_PhotoSet_SetCount(VtunerStrToInt(s));
			   mpDebugPrint("VTUNER_GET_ITEMCOUNT %d",VtunerStrToInt(s));
			   vtuner_info->state = VTUNER_GET_MENU;
		 }else if(vtuner_info->state == VTUNER_GET_STATION_ID)
		 {
		     memset(station_location_id,0x00,64);
		 	 memcpy(station_location_id,s,len);
			 vtuner_info->state = VTUNER_GET_MENU;
		 } 
		 else if(vtuner_info->state ==VTUNER_GET_STATION_NAME)
		 {
		   memset(station_location_name,0x00,64);
		   memcpy(station_location_name,s,len);
		   vtuner_info->state = VTUNER_GET_MENU;
   	}
		 else if(vtuner_info->state ==VTUNER_GET_STATION_URL)
   	{
		   memset(station_location,0x00,256);
		   memcpy(station_location,s,len);
		   vtuner_info->state = VTUNER_GET_MENU;
	}
		 else if(vtuner_info->state ==VTUNER_GET_STATION_MLME)
   	{
			 set_entry = (set_entry_t *)vtuner_malloc(sizeof(set_entry_t));
			 memset(set_entry, 0, sizeof(set_entry_t));
		 	 memcpy(set_entry->id,station_location_id,strlen(station_location_id));
			 memcpy(set_entry->title,station_location_name,strlen(station_location_name));
			 snprintf(set_entry->url,256,"%s&id=%s",station_location,station_location_id);
			 memcpy(set_entry->mime,s,len);
			 vtuner_info->cur_set->next = set_entry;
			 vtuner_info->cur_set = set_entry;
			 //mpDebugPrint("set_entry->id %s",set_entry->id);
			 //mpDebugPrint("set_entry->title %s",set_entry->title);
			 //mpDebugPrint("set_entry->url %s",set_entry->url);
			 //mpDebugPrint("set_entry->mime %s",set_entry->mime);
   	
			 vtuner_info->state = VTUNER_GET_MENU;
		 } 
		break;
	}
}

int vtuner_init(char *vtuner_url, char *base_dir)
{
    set_entry_t     *tmp_entry;
    
    int     ret = NETFS_OK;
    int     count;
#if DM9KS_ETHERNET_ENABLE||DM9621_ETHERNET_ENABLE
#if (P2P_TEST == 0&&MAKE_XPG_PLAYER)
    if(GetNetConfigP2PTestFlag())
#endif		
    {
	mpDebugPrint("P2P_TEST");
	  return ret;
    }
#endif
	mpDebugPrint("vtuner_init");
	mpDebugPrint("%s",vtuner_url);
    s_map = (vtuner_map_t *) vtuner_malloc(sizeof(vtuner_map_t));
	
    if (!s_map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    memset(s_map, 0, sizeof(vtuner_map_t));
    
    s_map->base_dir = (char *) vtuner_malloc(strlen(base_dir) + 1);
    if (!s_map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    strcpy(s_map->base_dir, base_dir);
    
    s_map->prev = vtuner_map.prev;
    s_map->next = &vtuner_map;
    vtuner_map.prev->next = s_map;
    vtuner_map.prev = s_map;

    s_map->info.cur_set = &s_map->info.set_list;

	vtuner_fetch_stations(vtuner_url,3);
    s_map->info.cur_set = s_map->info.set_list.next;
    //tmp_entry = s_map->info.cur_set;
    count = 0;
   
    //mpDebugPrint("tmp_entry %x",tmp_entry);
    {
		//mpDebugPrint("Matt AAAAAAA");
        while (s_map->info.cur_set)
        {
	        tmp_entry = s_map->info.cur_set;
			mpDebugPrint("Fetch iRadio from '%s' with id '%s' Mime '%s'",tmp_entry->title,tmp_entry->id,tmp_entry->mime);
		#if	0
			if(vtuner_iRadio_get_play_location(g_FileEntry,tmp_entry)< 0 )
			{
			    mpDebugPrint("g_FileEntry->Link==NULL");
				vtuner_mfree(tmp_entry);
			}
			else
		#else
			memcpy(g_FileEntry->Name,tmp_entry->title,strlen(tmp_entry->title));
		    memcpy(g_FileEntry->Link,tmp_entry->url,MAX_NET_LINK_LEN);
			memcpy(g_FileEntry->ExtName,tmp_entry->mime,strlen(tmp_entry->mime));
		#endif		
			{
				g_FileEntry++;
				g_psNet_FileBrowser->dwNumberOfFile ++;
			}

			//mpDebugPrint("g_FileEntry->Link %s",g_FileEntry->Link);
			tmp_entry->title[32] = '\0';
			count ++;

	        s_map->info.cur_set = s_map->info.cur_set->next;
			if( count > VTUNER_ALLSTATIONS_MAX )
				break;
#if	Make_CURL
			xpgUpdateYouTubeLoading(1);
#endif
		}
	}
#if	Make_CURL
	xpgUpdateYouTubeLoading(0);
#endif
	mpDebugPrint("End +++++++++++++++  Matt AAAAAAA %d",count);
	return ret;
fatal_exit:
	mpDebugPrint("vtuner fatal_exit\n");
    if (s_map)
    {
        if (s_map->base_dir)
            vtuner_mfree(s_map->base_dir);
        vtuner_mfree(s_map);
    }
	return ret;
}

void vtuner_exit(char *base_dir)
{
    set_entry_t     *tmp_entry;

	mpDebugPrint("End vtuner_exit");

    if (s_map)
    {
	    while (s_map->info.cur_set)
	    {
	        tmp_entry = s_map->info.cur_set;
	        s_map->info.cur_set = s_map->info.cur_set->next;

	        vtuner_mfree(tmp_entry);
	    }
        if (s_map->base_dir)
            vtuner_mfree(s_map->base_dir);
        vtuner_mfree(s_map);
    }
}

int vtuner_fetch_stations(char *vtuner_url,char level)
{
    XML_Parser              parser;
    int                     ret;
#if Make_CURL
    int                     len;
    http_info_t             http_info;
    http_header_handler_t   transfer_encoding_handler;
    vtuner_info_t           *vtuner_info    = &s_map->info;
    //char *data; 
    //XML_BUFF_link_t *ptr;
	int i=0,url_len = 0;
	char *bigger_buff=NULL;

    /* Get Data from Remote Site and Parse it */
	mpDebugPrint("vtuner_fetch_stations %d",level);
	menu_level = level; 
	menu_id = 0;
    parser = XML_ParserCreate(NULL);
    vtuner_info->state = VTUNER_GET_MENU;
    XML_SetUserData(parser, s_map);
    XML_SetElementHandler(parser, vtuner_start_element_handler, vtuner_end_element_handler);
    XML_SetCharacterDataHandler(parser, vtuner_content_handler);
	Xml_BUFF_init(NET_RECVHTTP);	
	mpDebugPrint("vtuner_url %s",vtuner_url);
    ret = Net_Recv_Data(vtuner_url,NET_RECVHTTP,0, 0);
	
	mpDebugPrint("******************Net_Recv_Data %x*************",ret);
		
    if(ret <0) 
    	goto exit;	

	bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
	if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
    {            	
  		mpDebugPrint("vtuner_init: %s at line %d, column %d\n",
          XML_ErrorString(XML_GetErrorCode(parser)),
          XML_GetCurrentLineNumber(parser),
          XML_GetCurrentColumnNumber(parser));			
    }

    if (vtuner_info->error_code > 0)
    {
        ret = -NETFS_APP_ERROR;
        goto exit;
    }
    XML_Parse(parser, bigger_buff, 0, 1);	
exit:
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);
    Xml_BUFF_free(NET_RECVHTTP);
#endif
    return ret;
}

int vtuner_connect(void)
{
	char *vtuner_url;
	int     ret,len;
#if Make_CURL
	int i,j;
	char *data; 
	//for blowfish
	unsigned char aucKey[24];
	unsigned char aucPlainText[32];
	unsigned char token[32];
	char szHex[49];
	szHex[16] = 0;
	unsigned char aucCipherText[16], iv[8];
	unsigned char aucDataOutText[16];
   	struct SBlock bs_blk;
	char copy = 0;
	//set_key test data
	//char *test = "Batalha";
	char* szKey = "42CF702F6EAA46B11EA03EC4712D9F2B" ;         /* key */
	char* sziv = "5B26EF037ABF275D" ;                                          /* iv */
	
	char* szdata[] =
	{
		//"00E04C1187216869BD2EFF4D00000000"
		"00195BEE27176869BD2EFF4D00000000"
		//"00120E6BD4EF6869BD2EFF4D00000000"
	};
	mpDebugPrint("vtuner_connect");
	
	Xml_BUFF_init(NET_RECVHTTP);	
	vtuner_url = "http://Company.vtuner.com/setupapp/Company/asp/browsexpa/loginXML.asp?token=0";
    
	len = Get_Image_File(vtuner_url, App_State.XML_BUF);
	//for key transfer	
	strcpy(szHex, szKey);
	HexStr2CharStr(szHex, aucKey, 16);
	aucKey[16] = 0;
	strcpy(szHex, sziv);
	HexStr2CharStr(szHex, iv, 8);
	bs_blk.m_uil = 0;
	bs_blk.m_uir = 0;
	BytesToBlock(iv, &bs_blk);
	//mpDebugPrint("bs_blk.m_uil %x",bs_blk.m_uil);
	//mpDebugPrint("bs_blk.m_uir %x",bs_blk.m_uir);
	m_oChain.m_uil = bs_blk.m_uil;
	m_oChain.m_uir = bs_blk.m_uir;
	mpDebugPrint("mac = %2x:%2x:%2x:%2x:%2x:%2x:",myethaddr[0],myethaddr[1],myethaddr[2],	myethaddr[3],myethaddr[4],myethaddr[5]);
	CharStr2HexStr(myethaddr, szHex, 6);
	//mpDebugPrint(" %s ",szHex);
	for(i=0;i<strlen(szHex);i++)
	{
	  szdata[0][i] = szHex[i];
	}
	mpDebugPrint(" %s ",szdata[0]);
	strcpy(szHex, szdata[0]);
	HexStr2CharStr(szHex, token, 16);
	strcpy(szHex, szdata[0]);
	HexStr2CharStr(szHex, aucPlainText, 16);
	CBlowFish(aucKey, 16, bs_blk);
	//mpDebugPrint("bf_block.m_uil %x",bf_block.m_uil);
	//mpDebugPrint("bf_block.m_uir %x",bf_block.m_uir);
	Encrypt_IO(aucPlainText, aucCipherText,16,CBC);
	CharStr2HexStr(aucCipherText, szHex, 16);
	//mpDebugPrint("Encrypt");
	mpDebugPrint(" %s ",szHex);
	//Decrypt_IO(aucCipherText, aucDataOutText,16,CBC);	
	//CharStr2HexStr(aucDataOutText, szHex, 16);	
	//mpDebugPrint("Decrypt");
	//mpDebugPrint(" %s ",szHex);
	vtuner_url = "http://Company.vtuner.com/setupapp/Company/asp/BrowseXpa/loginXML.asp?mac=2D1DAAE5F1CE7278112C5EC8F8DF3BD2&dlang=eng&fver=1.754333";
	//COPY  MAC  Excrypted combined string
	for(i=0;i<strlen(vtuner_url);i++)
	{
	  	if((vtuner_url[i] == 'm')&&(vtuner_url[i+1] == 'a')&&(vtuner_url[i+2] == 'c')&&(vtuner_url[i+3] == '='))
		{
		    	i+=4;
			for(j=0;j<strlen(szHex);j++)
			{
				vtuner_url[i+j] = szHex[j]; 
			}
		   	break;
		}
	}
	mpDebugPrint(" %s ",vtuner_url);
    len = Get_Image_File(vtuner_url, App_State.XML_BUF);
	for(i=0,j=0;i<strlen(vtuner_url);i++)
	{
	
		if((vtuner_url[i] == '?')&&(vtuner_url[i+1] == 'm')&&
			(vtuner_url[i+2] == 'a')&&(vtuner_url[i+3] == 'c')&&
			(vtuner_url[i+4] == '='))
		{
			copy = 1;
		}
		if(copy == 1)
		{
		
		    //mpDebugPrint("vtuner_mac_info copy");
			if(j==0)
				vtuner_mac_info[j]= '&';
			else
				vtuner_mac_info[j]= vtuner_url[i];
			j++;
		}
		
	}
	
	mpDebugPrint("vtuner_mac_info %s",vtuner_mac_info);
	mpDebugPrint("+++++++++++++++Get_Image_File %d+++++++++++++++++++++",len);


    Xml_BUFF_free(NET_RECVHTTP);    
	
	//vtuner_search(0,test);
#endif
	return ret;
}

int vtuner_iRadio_get_play_location(ST_NET_FILEENTRY *file_entry ,set_entry_t *tmp_entry)
{
    int     count;
	int                     ret = 0;
	int                     len;
	http_info_t             http_info;
	http_header_handler_t   transfer_encoding_handler;
	char *data; 
	//char pls_url[128];
	char httprequest[1024];
	char *ulrparm;
	int tcp_connection;
  	DWORD addr;
	BYTE *website;

	BYTE * ipaddr;
	//BYTE HostStr[256];
	int num_read;
	unsigned char buf[1024];
	int bufindex;
	char *FileSite,*FileSiteEND;
	char searchstr[16],searchstrfile[16],searchstrtitle[16];
	int i;
	int mime_type = 0;
	#if 0
	if(!strcasecmp(tmp_entry->mime, "MP3"))
	{
		audio_type = FILE_TYPE_MP3;
	}
	else if(!strcasecmp(tmp_entry->mime, "WMA"))
	{
		audio_type = FILE_TYPE_WMA;
	}
	#endif	
			website =(BYTE *) Net_GetWebSite(tmp_entry->url);
			ipaddr = (BYTE *)SearchServerByName(website);
			
			if( ipaddr )
			{
				addr = *((DWORD *) ipaddr);
				ret = mpx_DoConnect(addr, 80, TRUE);
			}
			else
			{
				addr=inet_addr(website);
				ret = mpx_DoConnect(addr, 80, TRUE);
			}
			
			snprintf(httprequest, 1024,
						"GET %s HTTP/1.1\r\n"
						"Host: %s\r\n"
						"Connection: close\r\n"
						"Referer: http://%s/\r\n"
						"\r\n",
						tmp_entry->url,
						website,
						website
						);
			if( ret > 0 )
			{
				tcp_connection = ret;
			}
			else
			{
				closesocket(tcp_connection);
				return;
			}
			//for get link location
			send( tcp_connection, httprequest, strlen(httprequest) , 0);	
			bufindex = 0;
			TaskYield();
			while(1)
			{
				num_read = recv( tcp_connection, &buf[bufindex] , 8192,0);
				bufindex += num_read;
				if( num_read <= 0 )
					break;
			}
			buf[bufindex] = '\0';
	//NetAsciiDump(buf, bufindex);
	memset((BYTE *)file_entry->Link,0x00,MAX_NET_LINK_LEN);	
	ret = vtuner_iRadio_get_link(buf,file_entry->Link,bufindex);
			
    memset((BYTE *)file_entry->Name,0x00,MAX_NET_NAME_LEN);
    memcpy((BYTE *)file_entry->Name,tmp_entry->title,strlen(tmp_entry->title)); 
    memcpy(file_entry->ExtName, tmp_entry->mime, 3);
			
	closesocket(tcp_connection);
    return ret;
}

int vtuner_iRadio_get_mp3_play_location(ST_NET_FILEENTRY *file_entry)
{
    int     count;
	int                     ret = 0;
	int                     len;
	http_info_t             http_info;
	http_header_handler_t   transfer_encoding_handler;
	char *data; 
	//char pls_url[128];
	char httprequest[1024];
	char *ulrparm;
	int tcp_connection;
  	DWORD addr;
	BYTE *website;

	BYTE * ipaddr;
	//BYTE HostStr[256];
	int num_read;
	unsigned char buf[1024];
	int bufindex;
	char *FileSite,*FileSiteEND;
	char searchstr[16],searchstrfile[16],searchstrtitle[16];
	int i;
	int mime_type = 0;
	WORD port = 0;
	
	website =(BYTE *) Net_GetWebSite(file_entry->Link);
	port = Net_GetWebSitePort(file_entry->Link);
	ipaddr = (BYTE *)SearchServerByName(website);
	if(!port)
		port=80;

	if( ipaddr )
	{
		addr = *((DWORD *) ipaddr);
		ret = mpx_DoConnect(addr, port, TRUE);
	}
	else
	{
		addr=inet_addr(website);
		ret = mpx_DoConnect(addr, port, TRUE);
	}
			
		snprintf(httprequest, 1024,
					"GET %s HTTP/1.1\r\n"
					"Host: %s\r\n"
					"Connection: close\r\n"
					"Referer: http://%s/\r\n"
					"\r\n",
					file_entry->Link,
					website,
					website
					);
		if( ret > 0 )
		{
			tcp_connection = ret;
		}
		else
		{
			closesocket(tcp_connection);
			return;
		}
		//for get link location
		send( tcp_connection, httprequest, strlen(httprequest) , 0);	
		bufindex = 0;
		TaskYield();
		while(1)
		{
			num_read = recv( tcp_connection, &buf[bufindex] , 8192,0);
			bufindex += num_read;
			if( num_read <= 0 )
				break;
		}
		buf[bufindex] = '\0';
		//NetAsciiDump(buf, bufindex);
		memset((BYTE *)file_entry->Link,0x00,MAX_NET_LINK_LEN);	
		ret = vtuner_iRadio_get_link(buf,file_entry->Link,bufindex);
		closesocket(tcp_connection);
    	return ret;
}

int vtuner_iRadio_get_link(BYTE *in,BYTE *out,int len)
{
	int i,j,ret = -1;
	char go = 0;
	
	mpDebugPrint("vtuner_iRadio_get_link");
	for(i=0;i<len;i++)
	{
	      //mpDebugPrint("%c",in[i]);
		  if((in[i]=='h')&&(in[i+1]=='t')&&
			(in[i+2]=='t')&&(in[i+3]=='p')&&
			(in[i+4]==':')&&(in[i+5]=='/'))
			{
			  //mpDebugPrint("http://");
			  go =1;
			  j = 0;
			  ret= 0;
			}
		  else if((in[i]=='m')&&(in[i+1]=='m')&&
			(in[i+2]=='s')&&(in[i+3]==':')&&
			(in[i+4]=='/'))
		  	{
		  	  //mpDebugPrint("mms://");
			  go =0;
			  j = 0;
			  break;
		  	}
		  	
		  if(go == 1)
		  {
			if(in[i] == 0x0D)//"/"
			{
			  go =0;
			  break;
			}
			if(in[i]>0x20)
			{
			  out[j] = in[i];
			  j++;
			}
		
		  }
		
	}
	
	out[j] = 0;
	
	mpDebugPrint("%s ",out);
    return ret;
}

///////////////////////////////////////////
// Type:
//        VTUNER_SEARCH_NORMAL
//        VTUNER_SEARCH_LOCATION
//        VTUNER_SEARCH_GENRE
//Name:
//        Search name
//Search result :
//                    Put into s_map->info.set_list 
//return result:
//                   0:success 
///////////////////////////////////////////
int vtuner_search(BYTE type,BYTE *name)
{
	int	   ret = SUCCESS;
#if 0
	BYTE url[256]="http://Company.vtuner.com/setupapp/Company/asp/browsexpa/search.asp?sSearchtype=2&search=";
	BYTE *url_location = "http://company.vtuner.com/setupapp/company/asp/Browsexpa/navXML.asp?gofile=LocationLevelTwo";
	BYTE *url_genre = "http://company.vtuner.com/setupapp/company/asp/Browsexpa/navXML.asp?gofile=GenreLevelTwo";
	BYTE *url_allstations = "-AllStations";
	set_entry_t	   *tmp_entry;
	char *base_dir;
	int	   count;
	int i=0,j=0,k=0;;
	BYTE search_result = 0;

	mpDebugPrint("vtuner_search_name %s ",name); 

	s_map = (vtuner_map_t *) vtuner_malloc(sizeof(vtuner_map_t));
  
	if (!s_map)
	{
		ret = -NETFS_NO_MEMORY;
		return ret;
	}
	memset(s_map, 0, sizeof(vtuner_map_t));

	s_map->base_dir = (char *) vtuner_malloc(strlen(base_dir) + 1);
	if (!s_map)
	{
		ret = -NETFS_NO_MEMORY;
		return ret;
	}
	strcpy(s_map->base_dir, base_dir);

	s_map->prev = vtuner_map.prev;
	s_map->next = &vtuner_map;
	vtuner_map.prev->next = s_map;
	vtuner_map.prev = s_map;

	s_map->info.cur_set = &s_map->info.set_list;

	switch(type)
	{
	case  VTUNER_SEARCH_NORMAL:
		ret = vtuner_search_level(type,name,url,search_level);
		break;
	case  VTUNER_SEARCH_LOCATION:
		ret = vtuner_search_level(type,name,url_location,search_level);
		break;
	case  VTUNER_SEARCH_GENRE:
		ret = vtuner_search_level(type,name,url_genre,search_level);
		break;
	}
   
   s_map->info.cur_set = s_map->info.set_list.next;
   tmp_entry = s_map->info.cur_set;
   
   if((tmp_entry==NULL)&&(type != VTUNER_SEARCH_NORMAL))
	  {
   		//reinit map
		s_map->prev = vtuner_map.prev;
		s_map->next = &vtuner_map;
		vtuner_map.prev->next = s_map;
		vtuner_map.prev = s_map;
		
		s_map->info.cur_set = &s_map->info.set_list;
		
	    //mpDebugPrint("search_area %d",search_area);
		
		if(type==VTUNER_SEARCH_LOCATION)
			search_level=1;
		
		  
   		for(i=0;i<search_area;i++)
		  {
			  
			memset(url,0x00,256);
			memcpy(url,vsearch_area[i].url,strlen(vsearch_area[i].url));
			search_area1 = 0;

			if(type==VTUNER_SEARCH_LOCATION)
{
				vtuner_search_level(type,name,url,search_level);
				//mpDebugPrint("search_area1 %d",search_area1);
  
				for(j = 0;j<search_area1;j++)
		  {
	  
				    //mpDebugPrint("%s",vsearch_area_level1[j].title);
					if (!strcasecmp(vsearch_area_level1[j].title,name))
	  {
						//mpDebugPrint("%s",url);
						search_result = 1;
	  
						memset(url,0x00,256);
						memcpy(url,vsearch_area_level1[j].url,strlen(vsearch_area_level1[j].url));
		  
						//change url level 
						for(k=0;k<strlen(url);k++)
		  {
						  if((url[k]=='F')&&(url[k+1]=='o')&&(url[k+2]=='u')&&(url[k+3]=='r'))
			  {
						    url[k+1]='i';
							url[k+2]='v';
							url[k+3]='e';
							k+=4;
			  }
						  if((url[k]== name[1])&&(url[k+1]==name[2])&&(url[k+2]==name[3])&&(url[k+3]==name[4]))
			  {   
						  	  k--;
                              k+=strlen(name);
							  //clear url
							  memset(url+k,0x00,(strlen(url)-k));
							  //set url all stations	
							  memcpy(url+strlen(url),url_allstations,strlen(url_allstations));
							  break;
			  
			  }
		  }
						ret = vtuner_search_level(type,name,url,search_level);
						
						break;
	  	}
	 }
 }
			else if(type==VTUNER_SEARCH_GENRE)
			{
				//mpDebugPrint("VTUNER_SEARCH_GENRE");

				//mpDebugPrint("%s",vsearch_area[i].title);
				if (!strcasecmp(vsearch_area[i].title,name))
{
					//mpDebugPrint("MATCH LOCATION");
					search_result = 1;
  
					memset(url,0x00,256);
					memcpy(url,vsearch_area[i].url,strlen(vsearch_area[i].url));
					//change url level 
					for(k=0;k<strlen(url);k++)
  {
					  if((url[k]=='T')&&(url[k+1]=='h')&&(url[k+2]=='r')&&(url[k+3]=='e'))
	  	{
					    url[k]='F';
						url[k+1]='o';
						url[k+2]='u';
						url[k+3]='r';
						url[k+4]='-';
						k+=5;
						//clear url
						memset(url+k,0x00,(strlen(url)-k));
						//set url name	
						memcpy(url+k,name,strlen(name));
						//set url all stations	
						memcpy(url+strlen(url),url_allstations,strlen(url_allstations));
						break;
		  }
	  }
					ret = vtuner_search_level(type,name,url,search_level);
	  
					break;
		  }
	  
		  
		  }
            if(search_result)
		      break;
	  }
	  
		s_map->info.cur_set = s_map->info.set_list.next;
		tmp_entry = s_map->info.cur_set;
	  	}
	  
   count = 0;
		  
   //mpDebugPrint("tmp_entry %x",tmp_entry);
	if (tmp_entry)
	{
	   //mpDebugPrint("Matt AAAAAAA");
		   while (tmp_entry)
		  { 
			   //TaskYield();    
			   mpDebugPrint("Fetch Search iRadio from '%s' with id %d '%s' Mime '%s'",tmp_entry->title,strlen(tmp_entry->id),tmp_entry->id,tmp_entry->mime);
			   tmp_entry->title[32] = '\0';
			   Net_Xml_PhotoSetList_AND_Id(tmp_entry->title,tmp_entry->id,0,count); 			//MP3
			   count ++;
					   tmp_entry = tmp_entry->next;
			   if( count > VTUNER_ALLSTATIONS_MAX)
				   break;
		  }
			   Net_PhotoSet_SetCount(count);   
  	}
	if (s_map)
	{
		if (s_map->base_dir)
		   vtuner_mfree(s_map->base_dir);
		vtuner_mfree(s_map);
	}
   return ret;
#endif
}
int vtuner_search_level(BYTE type,BYTE *name,BYTE *url,BYTE level)
{   
   int len;
   XML_Parser	parser;
   XML_BUFF_link_t *ptr;
   vtuner_map_t *vtuner_map;
   int	ret = SUCCESS;
  #if Make_CURL
 
   char *data; 

   mpDebugPrint("vtuner_search_level %d ",level); 
				  
   parser = XML_ParserCreate(NULL);
   XML_SetUserData(parser, s_map);
   XML_SetElementHandler(parser, vtuner_start_element_handler, vtuner_end_element_handler);
   XML_SetCharacterDataHandler(parser, vtuner_stations_content_handler);
	
   Xml_BUFF_init(NET_RECVHTTP);    
   switch(type)
   {
   	  case VTUNER_SEARCH_NORMAL:
		   memcpy(url+strlen(url),name,strlen(name));
		   url[strlen(url)] = '\0';
		ret = Net_Recv_Data(url,NETFS_VTUNER,0, 0);
		   break;
      case VTUNER_SEARCH_LOCATION:
      case VTUNER_SEARCH_GENRE:
	    ret = Net_Recv_Data(url,NETFS_VTUNER,0, 0);
		   break;

}

   ptr = App_State.XML_BUF;
  
   while (ptr != NULL)
  {
	  data =  ptr->BUFF;
	  len = ptr->buff_len;
	  //mpDebugPrint("len = %d ",len); 
		if (XML_Parse(parser, data, len, 0) == XML_STATUS_ERROR)
	    {
		   mpDebugPrint("vtuner_init: %s at line %d, column %d\n",
			 XML_ErrorString(XML_GetErrorCode(parser)),
			 XML_GetCurrentLineNumber(parser),
			 XML_GetCurrentColumnNumber(parser));		   
	    }
	   ptr = ptr->link; 	   
	   
	   TaskYield(); 
	    }
			
   XML_Parse(parser, data, 0, 1);  
									
    Xml_BUFF_free(NET_RECVHTTP);	
#endif	
   return ret;
			}
//Get location to 
void vtuner_location_list(BYTE *url,BYTE state)
			{   
//------------------------------------------------------
#if Make_CURL

  XML_Parser			  parser;
  int					  ret;
  int					  len;
  http_info_t			  http_info;
  http_header_handler_t   transfer_encoding_handler;
  vtuner_info_t 		  *vtuner_info	  = &s_map->info;
  int i=0,url_len = 0;
  char *bigger_buff=NULL;
  int	count;		
  /* Get Data from Remote Site and Parse it */
			
  menu_id = 0;
			
  parser = XML_ParserCreate(NULL);
  vtuner_info->state = state;
  XML_SetUserData(parser, s_map);
  XML_SetElementHandler(parser, vtuner_start_element_handler, vtuner_end_element_handler);
  XML_SetCharacterDataHandler(parser, vtuner_content_handler);
  Xml_BUFF_init(NET_RECVHTTP);	  
  ret = Net_Recv_Data(url,NETFS_VTUNER,0, 0);
									
  mpDebugPrint("******************Net_Recv_Data %x*************",ret);
	
  if(ret <0) 
	  goto exit; 
  
  bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
									
	
	 //mpDebugPrint("len = %d ",len); 
	 //TaskYield(); 
	if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
	    	{
		mpDebugPrint("vtuner_location_list: %s at line %d, column %d\n",
		XML_ErrorString(XML_GetErrorCode(parser)),
		XML_GetCurrentLineNumber(parser),
		XML_GetCurrentColumnNumber(parser));		  
				
	    	}
	
  if (vtuner_info->error_code > 0)
		{
	  ret = -NETFS_APP_ERROR;
	  goto exit;
			}
  XML_Parse(parser, bigger_buff, 0, 1);  
exit:
				
	if(bigger_buff != NULL)
	    ext_mem_free(bigger_buff);
  	Xml_BUFF_free(NET_RECVHTTP);	  
 	return ret;
#endif	
		}
	
//*******************************************************************************
//connect to vtuner home page http://company.vtuner.com/setupapp/sample/asp/BrowseStations
///StartPage.asp
//
//*******************************************************************************
int vtuner_connect_home_page(void)
{
	BYTE *website;
	BYTE *ipaddr;
  	DWORD addr;
	int   ret = SUCCESS;
#if Make_CURL
	char httprequest[1024];
	int tcp_connection;
	int num_read;
	int bufindex;
	int i,j,k,l;
	int go = 0,num = 0;
	unsigned char html_buf[2048];
	unsigned char longin_mac[4];
	unsigned char longin_space[4] = "%3A";
	
	BYTE login_url[256]= "/setupapp/sample/asp/CheckMac/CheckMacAndSend.asp?SetupType=Basic&sBigMac=";
	BYTE *vtuner_website = "http://company.vtuner.com";
	BYTE *login_referer="company.vtuner.com/setupapp/sample/asp/AuthLogin/SignIn.asp?";
	
	mpDebugPrint("vtuner_connect_home_page");
	
	memset(cookie_num,0x00,500);
	vtuner_connect_stste = VTUNER_CONNECT_HOME_PAGE_LOGIN;
		
	website =(BYTE *) Net_GetWebSite(vtuner_website);
	ipaddr = (BYTE *)SearchServerByName(website);
	mpDebugPrint("website %s",website);
	if( ipaddr )
	{
		mpDebugPrint("ipaddr %x",ipaddr[0]);
		mpDebugPrint("if( ipaddr )");
		addr = *((DWORD *) ipaddr);
		mpDebugPrint("addr1 %x",addr);
		ret = mpx_DoConnect(addr, 80, TRUE);
	}
	else
	{
		addr=inet_addr(website);
		mpDebugPrint("addr2 %x",addr);
		ret = mpx_DoConnect(addr, 80, TRUE);
	}
    //copy mac address to login information
    for(i=0;i<6;i++)
    {
    	Char2Hex(myethaddr[i],longin_mac);
		memcpy(login_url+strlen(login_url),longin_mac,strlen(longin_mac));
		if(i<5)
		{
			memcpy(login_url+strlen(login_url),longin_space,strlen(longin_space));
		}
		
    }
	mpDebugPrint("%s",login_url);

	
	snprintf(httprequest, 1024,
				"GET %s HTTP/1.1\r\n"
				"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/msword, application/xaml+xml, application/vnd.ms-xpsdocument, application/x-ms-xbap, application/x-ms-application, application/x-silverlight, */*\r\n"
				"Referer: http://%s\r\n"
				"Accept-Language: zh-tw\r\n"
				"Host: %s\r\n"
				"Connection: Keep-Alive\r\n"
				"Cookie: rsSaveLoginCookie=No; rsSerialStatus=PREMIUM; rsAuthCookie=; rsCookieOnOffChecker=TrueCookie; ASPSESSIONIDCSAABCSQ=GLOMOHBDDFLCMKAEEPNDJNJ\r\n"
				"\r\n",
				login_url,
				login_referer,
				website
				);
					
			
		
	if( ret > 0 )
	{
		tcp_connection = ret;
	}
	else
	{
		DPrintf("############### Vtuner home page connect fail ###########");
		closesocket(tcp_connection);
		return;
	}
	//TaskYield();
	//for get link location
	send( tcp_connection, httprequest, strlen(httprequest) , 0);	
	bufindex = 0;
	TaskYield();
	num_read = recv( tcp_connection, &html_buf[0] , 8192,0);
	bufindex += num_read;
	//MP_DEBUG1("num_read %d",num_read);
	html_buf[bufindex] = '\0';
		
    //keep cookie
	for(i=0,j=0;i<num_read;i++)
	{
	  if((html_buf[i]=='r')&&(html_buf[i+1]=='s')&&(html_buf[i+2]=='A')&&(html_buf[i+3]=='u')&&
	  	(html_buf[i+4]=='t')&&(html_buf[i+5]=='h')&&(html_buf[i+6]=='C')&&(html_buf[i+7]=='o')&&
	    (html_buf[i+8]=='o')&&(html_buf[i+9]=='k')&&(html_buf[i+10]=='i')&&(html_buf[i+11]=='e'))
	  	{
	  		i+=13;
			go = 1;
	  	}
	  if(go)
	  	{
	  	  if(html_buf[i]==0x3b)
	  	  {
	  	    go = 0;
			break;
	  	  }
		  cookie_num[j]=html_buf[i];
		  j++;
	  	 //mpDebugPrint(" %2c ",buf[i]);
	  	}
	}
#endif	
    return ret;
}
//*******************************************************************************
//Get My favorite group list
//
//
//*******************************************************************************

int vtuner_get_favorite_group(void)
{
	BYTE *website;
	BYTE *ipaddr;
  	DWORD addr;
	int   ret = SUCCESS;
	char httprequest[1024];
	int tcp_connection;
	unsigned char html_buf[8192];
	int num_read;
	int bufindex;
	BYTE *vtuner_website = "http://company.vtuner.com";
	BYTE *login_referer="company.vtuner.com/setupapp/sample/asp/AuthLogin/SignIn.asp?";
	BYTE *url="/setupapp/sample/asp/BrowseStations/StartPage.asp?sSpot=sSpotD&lngy=eng";
    //vtuner_info_t  *vtuner_info;
	
	vtuner_connect_stste = VTUNER_CONNECT_HOME_PAGE_GET_LIST;
	
    html_init(&vtuner_html_parser, s_map);
    html_set_content_handler(&vtuner_html_parser, vtuner_desc_content_handler);
    html_set_tag_start(&vtuner_html_parser, vtuner_desc_tag_start_handler);
    html_set_tag_end(&vtuner_html_parser, vtuner_desc_tag_end_handler);
	
	website =(BYTE *) Net_GetWebSite(vtuner_website);
	ipaddr = (BYTE *)SearchServerByName(website);
	//mpDebugPrint("website %s",website);
	if( ipaddr )
	{
		//mpDebugPrint("ipaddr %x",ipaddr[0]);
		//mpDebugPrint("if( ipaddr )");
		addr = *((DWORD *) ipaddr);
		//mpDebugPrint("addr1 %x",addr);
		ret = mpx_DoConnect(addr, 80, TRUE);
	}
	else
	{
		addr=inet_addr(website);
		//mpDebugPrint("addr2 %x",addr);
		ret = mpx_DoConnect(addr, 80, TRUE);
	}
	snprintf(httprequest, 1024,
				"GET %s HTTP/1.1\r\n"
				"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/msword, application/xaml+xml, application/vnd.ms-xpsdocument, application/x-ms-xbap, application/x-ms-application, application/x-silverlight, */*\r\n"
				"Referer: http://%s\r\n"
				"Host: %s\r\n"
				"Connection: Keep-Alive\r\n"
				"Accept-Language: zh-tw\r\n"
				"Cookie: rsSaveLoginCookie=YES; rsSerialStatus=PREMIUM; rsAuthCookie=%s; rsCookieOnOffChecker=TrueCookie; ASPSESSIONIDCSAABCSQ=GLOMOHBDDFLCMKAEEPNDJNJ\r\n"
				"\r\n",
				url,
				login_referer,
				website,
				cookie_num
				);
				
	if( ret > 0 )
	{
		tcp_connection = ret;
	}
	else
	{
		DPrintf("##### vtuner_get_favorite_group fail ######");
		closesocket(tcp_connection);
		return;
	}
	//TaskYield();
	//for get link location
	send( tcp_connection, httprequest, strlen(httprequest) , 0);	
	bufindex = 0;
	TaskYield();

    while(1)
   	{
		num_read = recv( tcp_connection, &html_buf[0] , 8192,0);
		bufindex += num_read;
		html_parse(&vtuner_html_parser,html_buf,num_read);
		//MP_DEBUG1("num_read %d",num_read);
		
		//MP_DEBUG1("tcp_connection %x",tcp_connection);
		if( num_read < 1400 )//last one packet
			break;
   	}


	html_exit(&vtuner_html_parser);
	
	closesocket(tcp_connection);
	return ret;

}
//*******************************************************************************
//Get My favorite group  station list and put into s_map 
//favoritelist: 
//		My favorite group name
//
//
//*******************************************************************************

int vtuner_get_favorite_station(BYTE *favoritelist)
{
	BYTE *website;
	BYTE *ipaddr;
  	DWORD addr;
	int   ret = SUCCESS;
	char httprequest[1024];
	int tcp_connection;
	unsigned char html_buf[8192];
	int num_read;
	int bufindex;
	int i;
	BYTE *vtuner_website = "http://company.vtuner.com";
	BYTE *url_referer="company.vtuner.com/setupapp/sample/asp/BrowseStations/StartPage.asp?sSpot=sSpotD&lngy=eng";
	BYTE favoritegroup[256] ="/setupapp/sample/asp/Favorites/BrowseFavorites.asp?sFavoriteGroupName=";
    //vtuner_info_t  *vtuner_info;
	
	//mpDebugPrint("vtuner_get_favorite_station %s",favoritelist);
	
	vtuner_connect_stste = VTUNER_CONNECT_HOME_PAGE_GET_STATION;
	
    html_init(&vtuner_html_parser, s_map);
    html_set_content_handler(&vtuner_html_parser, vtuner_desc_content_handler);
    html_set_tag_start(&vtuner_html_parser, vtuner_desc_tag_start_handler);
    html_set_tag_end(&vtuner_html_parser, vtuner_desc_tag_end_handler);
	
	website =(BYTE *) Net_GetWebSite(vtuner_website);
	ipaddr = (BYTE *)SearchServerByName(website);
	//mpDebugPrint("website %s",website);
	if( ipaddr )
	{
		//mpDebugPrint("ipaddr %x",ipaddr[0]);
		//mpDebugPrint("if( ipaddr )");
		addr = *((DWORD *) ipaddr);
		//mpDebugPrint("addr1 %x",addr);
		ret = mpx_DoConnect(addr, 80, TRUE);
	}
	else
	{
		addr=inet_addr(website);
		//mpDebugPrint("addr2 %x",addr);
		ret = mpx_DoConnect(addr, 80, TRUE);
	}
    //copy favoritegroup information
	memcpy(favoritegroup+strlen(favoritegroup),favoritelist,strlen(favoritelist));
		
	//mpDebugPrint("%s",favoritegroup);
				
	snprintf(httprequest, 1024,
				"GET %s HTTP/1.1\r\n"
				"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/msword, application/xaml+xml, application/vnd.ms-xpsdocument, application/x-ms-xbap, application/x-ms-application, application/x-silverlight, */*\r\n"
				"Referer: http://%s\r\n"
				"Accept-Language: zh-tw\r\n"
				"Host: %s\r\n"
				"Connection: Keep-Alive\r\n"
				"Cookie: rsSaveLoginCookie=YES; rsSerialStatus=PREMIUM; rsAuthCookie=%s; rsCookieOnOffChecker=TrueCookie; ASPSESSIONIDCSAABCSQ=GLOMOHBDDFLCMKAEEPNDJNJ\r\n"
				"\r\n",
				favoritegroup,
				url_referer,
				website,
				cookie_num
				);
				
	if( ret > 0 )
	{
		tcp_connection = ret;
	}
	else
	{
		DPrintf("############### Vtuner home page connect fail ###########");
		closesocket(tcp_connection);
		return;
	}
	//TaskYield();
	//for get link location
	send( tcp_connection, httprequest, strlen(httprequest) , 0);	
	bufindex = 0;
	TaskYield();

    while(1)
   	{
		num_read = recv( tcp_connection, &html_buf[0] , 8192,0);
		bufindex += num_read;
		html_parse(&vtuner_html_parser,html_buf,num_read);
		//MP_DEBUG1("num_read %d",num_read);
		
		//MP_DEBUG1("tcp_connection %x",tcp_connection);
		if( num_read < 1400 )//last one packet
			break;
   	}


	html_exit(&vtuner_html_parser);
	
	closesocket(tcp_connection);
	return ret;

}
//*******************************************************************************
//Get My favorite group  station location
//favoritelist: 
//		My favorite group name
//station_name:
//             station name
//station_id:
//               station id
//
//*******************************************************************************

int vtuner_get_favorite_station_location(BYTE *favoritelist,BYTE *station_name,BYTE *station_id)
{
	BYTE *website;
	BYTE *ipaddr;
  	DWORD addr;
	int   ret = SUCCESS;
	char httprequest[1024];
	int tcp_connection;
	unsigned char html_buf[8192];
	int num_read;
	int bufindex;
	int i;
	BYTE *vtuner_website = "http://company.vtuner.com";
	BYTE url_referer[256]="company.vtuner.com/setupapp/sample/asp/Favorites/BrowseFavorites.asp?sFavoriteGroupName=";
	BYTE station_link[256] ="/setupapp/sample/asp/func/dynampls.asp?link=1&id=";
    //vtuner_info_t  *vtuner_info;
	
	//mpDebugPrint("vtuner_get_favorite_station %s",favoritelist);
	
	vtuner_connect_stste = VTUNER_CONNECT_HOME_PAGE_GET_STATION_LOCATION;
	
    html_init(&vtuner_html_parser, s_map);
    html_set_content_handler(&vtuner_html_parser, vtuner_desc_content_handler);
    html_set_tag_start(&vtuner_html_parser, vtuner_desc_tag_start_handler);
    html_set_tag_end(&vtuner_html_parser, vtuner_desc_tag_end_handler);
	
	website =(BYTE *) Net_GetWebSite(vtuner_website);
	ipaddr = (BYTE *)SearchServerByName(website);
	//mpDebugPrint("website %s",website);
	if( ipaddr )
	{
		//mpDebugPrint("ipaddr %x",ipaddr[0]);
		//mpDebugPrint("if( ipaddr )");
		addr = *((DWORD *) ipaddr);
		//mpDebugPrint("addr1 %x",addr);
		ret = mpx_DoConnect(addr, 80, TRUE);
	}
	else
	{
		addr=inet_addr(website);
		//mpDebugPrint("addr2 %x",addr);
		ret = mpx_DoConnect(addr, 80, TRUE);
	}
    //copy favoritegroup information
    
	memcpy(url_referer+strlen(url_referer),favoritelist,strlen(favoritelist));
	memcpy(station_link+strlen(station_link),station_id,strlen(station_id));
		
	//mpDebugPrint("%s",favoritegroup);
				
	snprintf(httprequest, 1024,
				"GET %s HTTP/1.1\r\n"
				"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/msword, application/xaml+xml, application/vnd.ms-xpsdocument, application/x-ms-xbap, application/x-ms-application, application/x-silverlight, */*\r\n"
				"Referer: http://%s\r\n"
				"Accept-Language: zh-tw\r\n"
				"Host: %s\r\n"
				"Connection: Keep-Alive\r\n"
				"Cookie: rsSaveLoginCookie=YES; rsSerialStatus=PREMIUM; rsAuthCookie=%s; rsCookieOnOffChecker=TrueCookie; ASPSESSIONIDCSAABCSQ=GLOMOHBDDFLCMKAEEPNDJNJ\r\n"
				"\r\n",
				station_link,
				url_referer,
				website,
				cookie_num
				);
				
	if( ret > 0 )
	{
		tcp_connection = ret;
	}
	else
	{
		DPrintf("############### Vtuner home page connect fail ###########");
		closesocket(tcp_connection);
		return;
	}
	//TaskYield();
	//for get link location
	send( tcp_connection, httprequest, strlen(httprequest) , 0);	
	bufindex = 0;
	TaskYield();

    while(1)
   	{
		num_read = recv( tcp_connection, &html_buf[0] , 8192,0);
		bufindex += num_read;
		html_parse(&vtuner_html_parser,html_buf,num_read);
		//MP_DEBUG1("num_read %d",num_read);
		//MP_DEBUG1("tcp_connection %x",tcp_connection);
		if( num_read < 1400 )//last one packet
			break;
   	}
	
	html_exit(&vtuner_html_parser);
	
	closesocket(tcp_connection);
	return ret;

}
//*******************************************************************************
//Get My favorite group  station location real link address
//favoritelist: 
//		My favorite group name
//location:
//             station location
//
//*******************************************************************************

int vtuner_get_favorite_station_location_link(BYTE *favoritelist,BYTE *location)
{
	BYTE *website;
	BYTE *ipaddr;
  	DWORD addr;
	int   ret = SUCCESS;
	char httprequest[1024];
	int tcp_connection;
	unsigned char html_buf[8192];
	int num_read;
	int bufindex;
	int i;
	BYTE tempbuf[256];
	BYTE *vtuner_website = "http://company.vtuner.com";
	BYTE url_referer[256]="company.vtuner.com//setupapp/sample/asp/Favorites/BrowseFavorites.asp?sFavoriteGroupName=";
    vtuner_info_t  *vtuner_info;
	
	mpDebugPrint("vtuner_get_favorite_station_location_link %s",location);
	
	website =(BYTE *) Net_GetWebSite(vtuner_website);
	ipaddr = (BYTE *)SearchServerByName(website);
	//mpDebugPrint("website %s",website);
	if( ipaddr )
	{
		//mpDebugPrint("ipaddr %x",ipaddr[0]);
		//mpDebugPrint("if( ipaddr )");
		addr = *((DWORD *) ipaddr);
		//mpDebugPrint("addr1 %x",addr);
		ret = mpx_DoConnect(addr, 80, TRUE);
	}
	else
	{
		addr=inet_addr(website);
		//mpDebugPrint("addr2 %x",addr);
		ret = mpx_DoConnect(addr, 80, TRUE);
	}
    //copy favoritegroup information
	memcpy(url_referer+strlen(url_referer),favoritelist,strlen(favoritelist));
	snprintf(httprequest, 1024,
				"GET %s HTTP/1.1\r\n"
				"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/msword, application/xaml+xml, application/vnd.ms-xpsdocument, application/x-ms-xbap, application/x-ms-application, application/x-silverlight, */*\r\n"
				"Referer: http://%s\r\n"
				"Accept-Language: zh-tw\r\n"
				"Host: %s\r\n"
				"Connection: Keep-Alive\r\n"
				"Cookie: rsSaveLoginCookie=YES; rsSerialStatus=PREMIUM; rsAuthCookie=%s; rsCookieOnOffChecker=TrueCookie; ASPSESSIONIDCSAABCSQ=GLOMOHBDDFLCMKAEEPNDJNJ\r\n"
				"\r\n",
				location,
				url_referer,
				website,
				cookie_num
				);
				
	if( ret > 0 )
	{
		tcp_connection = ret;
	}
	else
	{
		DPrintf("############### Vtuner home page connect fail ###########");
		closesocket(tcp_connection);
		return;
	}
	//TaskYield();
	//for get link location
	send( tcp_connection, httprequest, strlen(httprequest) , 0);	
	bufindex = 0;
	TaskYield();

    while(1)
   	{
		num_read = recv( tcp_connection, &html_buf[0] , 8192,0);
		bufindex += num_read;
		vtuner_iRadio_get_link(html_buf,tempbuf,num_read);
		MP_DEBUG1("num_read %d",num_read);
		//MP_DEBUG1("tcp_connection %x",tcp_connection);
		if( num_read < 1400 )//last one packet
			break;
   	}
	closesocket(tcp_connection);
	return ret;

}
//*******************************************************************************
//Get My favorite from internet and put into s_map list
//           
//
//
//           
//
//*******************************************************************************

int vtuner_my_favorite(void)
{
	set_entry_t	   *tmp_entry;
	char *base_dir;
	int	   ret = SUCCESS;
	int	   count;
	int i=0,j=0,k=0;;
	BYTE search_result = 0;

	s_map = (vtuner_map_t *) vtuner_malloc(sizeof(vtuner_map_t));
  
	if (!s_map)
	{
		ret = -NETFS_NO_MEMORY;
		return ret;
	}
	memset(s_map, 0, sizeof(vtuner_map_t));

	s_map->base_dir = (char *) vtuner_malloc(strlen(base_dir) + 1);
	if (!s_map)
	{
		ret = -NETFS_NO_MEMORY;
		return ret;
	}
	strcpy(s_map->base_dir, base_dir);

	s_map->prev = vtuner_map.prev;
	s_map->next = &vtuner_map;
	vtuner_map.prev->next = s_map;
	vtuner_map.prev = s_map;

	s_map->info.cur_set = &s_map->info.set_list;
	
	vtuner_connect_home_page();
	vtuner_get_favorite_group();
	//for test
	//vtuner_get_favorite_station("1234");
	//vtuner_get_favorite_station_location("1234","181 FM The Buzz","21378");
	//vtuner_get_favorite_station_location_link("1234",station_location);
   
   s_map->info.cur_set = s_map->info.set_list.next;
   tmp_entry = s_map->info.cur_set;
  
   count = 0;
		  
    //mpDebugPrint("tmp_entry %x",tmp_entry);
	if (tmp_entry)
	{
	     //mpDebugPrint("Matt AAAAAAA");
		   while (tmp_entry)
		  { 
			   //TaskYield();    
			   mpDebugPrint("vtuner_my_favorite '%s'",tmp_entry->title);
			   tmp_entry->title[32] = '\0';
			   mpDebugPrint("vtuner_my_favorite url '%s'",tmp_entry->url);
			   //mpDebugPrint("vtuner_my_favorite id '%s'",tmp_entry->id);
			   //mpDebugPrint("vtuner_my_favorite mime '%s'",tmp_entry->mime);
			   Net_Xml_PhotoSetList_AND_Id(tmp_entry->title,tmp_entry->id,0,count); 			//MP3
			   count ++;
			   tmp_entry = tmp_entry->next;
			   if( count > VTUNER_ALLSTATIONS_MAX)
				   break;
		  }
			   Net_PhotoSet_SetCount(count);   
  	}
	if (s_map)
	{
		if (s_map->base_dir)
		   vtuner_mfree(s_map->base_dir);
		vtuner_mfree(s_map);
	}
   return ret;
}
void vtuner_location_free(ST_FEED *location_feed)
{
    int i;
	for(i=0;i<Vtuner_location_list_Max;i++)
	{
	  if(location_feed->pTitle)
	  {
	    //mpDebugPrint("location_feed->pTitle %x",location_feed->pTitle);
		//mpDebugPrint("location_feed->pLink %x",location_feed->pLink);
	   	vtuner_mfree(location_feed->pTitle);
	  	vtuner_mfree(location_feed->pLink);
	  }
	}

}
void vtuner_change_level(BYTE *url,int len)
{
   int len_backup = len;
   BYTE flag = 0;
   BYTE *tmpdata = mm_malloc(len+12);
   int i;
   BYTE *str="-AllStations";
   memcpy(tmpdata,url,len);
   memset(url,0x00,len);
   i=0;
   while(len)
   {
     if(!strncmp(&tmpdata[i],"LocationLevel",strlen("LocationLevel")))
     {
       i+=strlen("LocationLevel");
	   flag=1 ;
     }
	 if(flag)
	 {
	   switch(tmpdata[i])
	   { 
	     case 'o':
	     	tmpdata[i]='i';
			break;
		 case 'u':
			tmpdata[i]='v';
		    break;
	     case 'r':
			tmpdata[i]='e';
			flag = 0;
		    break;
	   }
	   	
	 }

	 i++;
     len--;
   }
   memcpy(tmpdata+len_backup,str,12);
   tmpdata[len_backup+12]='\0';
   //mpDebugPrint("tmpdata %s",tmpdata);
   memcpy(url,tmpdata,strlen(tmpdata));
   mm_free(tmpdata);
}
BYTE vtuner_get_menu_level(void)
{
   return menu_level;
}
void vtuner_Set_menu_level(BYTE level)
{
    menu_level = level;
}
void vtuner_free_entry(void)
{
    set_entry_t     *tmp_entry;

	mpDebugPrint("vtuner_free_entry");

	while (s_map->info.cur_set)
	{
		tmp_entry = s_map->info.cur_set;
		s_map->info.cur_set = s_map->info.cur_set->next;

		vtuner_mfree(tmp_entry);
	}

}

#endif
