/**
 * @file
 *
 * This is an implementation of Youtube Data API.
 *
 * The following APIs are supported:
 *
 *  1) Authentication - Support for ClientLogin only.  We don't support AuthSub.
 *
 *          youtube_ClientLogin()
 *
 *     This function must be called first to get a authentication token, which
 *     will be needed in other APIs below.
 *
 *  2) Request a feed that lists all of the video (private and public ) belonging
 *     to an user.
 *
 *          youtube_fetch_video_list()
 *
 *     Equivalent HTTP Request:
 *     For my favorites:
 *          GET http://gdata.youtube.com/feeds/api/users/<username>/favorites?v=2
 *
 *
 * youtube_init() is one example that uses picasa_ClientLogin and
 * youtube_fetch_video_list.  But the user's password is hard-coded.  For actual
 * applications, the password has to be passed from calling function.
 *
 * Copyright (c) 2009 Magic Pixel Inc.
 * All rights reserved.
 */

// define this module show debug message or not,  0 : disable, 1 : enable
#define LOCAL_DEBUG_ENABLE 1

#include <stdio.h>
#include <string.h>
#include <linux/types.h>
#include <expat.h>
#include "netfs.h"
#include "netfs_pri.h"
#include "xmlyouku3G.h"
#include "..\..\lwip\include\net_sys.h"
#include "..\..\lWIP\include\net_socket.h"
#include "..\..\CURL\include\net_curl_curl.h"
#include "..\..\netstream\include\netstream.h"
#include "taskid.h"

#if HAVE_YOUKU3G

static char     youku3g_api_url[MAX_URL];
youku3g_info_t   youku3g_info;
static const char   *youku3g_base_dir;
//static DWORD youtube_description_idx =0;
static html_parser_t youku3g_html_parser;
youku3g_video_t youku3g_video;
int youku3g_video_show_load = 0;
int youku3g_conn;
BYTE youku3g_tv_show_id[MAX_ALBUMID];
int youku3g_play_mode = 0;

#define YOUKU3G_HOST "3g.youku.com"
#define youku3g_malloc(sz)   ext_mem_malloc(sz)
#define youku3g_mfree(ptr)   ext_mem_free(ptr)
#define YOUKU3G_ID_LENGTH     16
#define YOUKU3G_BUFFER_MAX     768*1024
#define YOUKU3G_BUFFER_MIN     512*1024
#define YOUKU3G_BUFFER_LOW     64*1024
#define YOUKU3G_VIDEO_NUM     30
#define YOUKU3G_VIDEO_SUPPORT_ITAG     22

extern Net_App_State App_State;
extern ST_NET_FILEENTRY * g_FileEntry;
extern ST_NET_FILEBROWSER *g_psNet_FileBrowser;
#if (CHIP_VER_MSB == CHIP_VER_650)
extern unsigned char girbuf[];
#else
extern unsigned char *girbuf;
#endif
extern int bufindex;
extern int bufindex_end;

//note: fixed to use global my_write_func() for all xmlxxxxxx.c files!
size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf);
size_t range_write_func1(void *ptr, size_t size, size_t nmemb, void *buf);
BYTE *Merge_ListData_to_SingleBuffer(XML_BUFF_link_t *list_head, int *total_data_len);

int youku3g_get_network_states()
{
    return youku3g_video.youku3g_video_error;
}

int youku3g_get_video_index()
{
    return youku3g_video.youku3g_video_idx;
}
void youku3g_set_video_index(int idx)
{
    youku3g_video.youku3g_video_idx = idx;
}
void youku3g_set_video_stop(int flag)
{
    youku3g_video.youku3g_video_stop = flag;
}

#if NETWARE_ENABLE

BYTE *youku3g_get_video_ext()
{
  BYTE *youku3g_ext;
  //youtube_video_ext = 1;
  switch(youku3g_video.youku3g_video_ext)
  {
    case 0:
		youku3g_ext = "3gpp";
		break;
	case 1:
		youku3g_ext = "mp4";
		break;
  }
  mpDebugPrint("youtube_ext %s",youku3g_ext);
  return youku3g_ext;
}
youku3g_info_t * getYouKu3GInfo()
{
    return &youku3g_info;
}
video_entry_t *youku3g_get_video_entry(BYTE idx)
{
    video_entry_t       *youku3g_video;
	int i;

	youku3g_video = youku3g_info.cur_video;
	if(idx == 0)
		return youku3g_video;

	if((idx>0)&&(idx < youku3g_info.video_num))
	{
	    for(i=0;i<idx;i++)
	    {
	    	youku3g_video = youku3g_video->next;
	    }

	}
	else
		youku3g_video = NULL;

    return youku3g_video;
}
int youku3g_get_video_total(void)
{
    return youku3g_info.video_num;
}

STREAM * getYouKu3Ghandle()
{
    STREAM *youku3g_shandle;
    DRIVE *youku3g_sDrv = DriveGet(USB_WIFI_DEVICE);

    youku3g_sDrv->Flag.Present = 1;
    if (!(youku3g_shandle = GetFreeFileHandle(youku3g_sDrv)))
        return NULL;

    youku3g_shandle->DirSector = 0;
    youku3g_shandle->Chain.Start = 0;
    youku3g_shandle->Chain.Point = 0;
    youku3g_shandle->Chain.Size = youku3g_video.youku3g_video_size;
    g_psNet_FileBrowser->dwNumberOfFile = 1;

    return youku3g_shandle;
}

BYTE *youku3g_getvideo_id(DWORD dwIndex)
{
	video_entry_t		*video_entry;
	int i=0;

	video_entry = youku3g_info.cur_video;
    while(video_entry)
    {
        if(i==dwIndex)
			break;
		mpDebugPrint("video_entry->id i %d %s",i,video_entry->id);
		video_entry = video_entry->next;
		i++;
    }
	return video_entry->id;
}

/****************************************************************************
 **
 ** NAME:           YoutubeStrToInt
 **
 ** PARAMETERS:     pointer of charactor
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Convert a HEX string to a unsigned interger.
 **
 ****************************************************************************/
DWORD youku3gstrtoint(BYTE * ptr)
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
 int youku3g_get_tag(BYTE *url, int len)
 {
   int tag_num = 0,i;
   BYTE tmpbuf[32];
   for(i=0;i<len;i++)
	 {
     if(url[i]=='i')
	 	memcpy(tmpbuf,&url[i],5);
	 if(!strncasecmp(tmpbuf,"itag=",5))
		 {
	   i+=5;
	   memset(tmpbuf,0x00,32);
	   memcpy(tmpbuf,&url[i],2);
	   tag_num = youku3gstrtoint(tmpbuf);
		 }
		 }

   return tag_num;
 }
 int youku3g_get_video_idx()
		 {
 	youku3g_info_t *youku3g = &youku3g_info;
    int video_idx = 0,i,video_flv_idx = 0;
	int tag_num[MAX_FRM_URL_MAP],tag_min,tag_max,tag_m;
	int video_num=0;

	for(i=0;i<MAX_FRM_URL_MAP;i++)
		tag_num[i]=0;

	for(i=0;i<MAX_FRM_URL_MAP;i++)
			 {
	    if(youku3g->cur_video->frm_url_map[i]==NULL)
			break;
		tag_num[i] = youku3g_get_tag(youku3g->cur_video->frm_url_map[i],strlen(youku3g->cur_video->frm_url_map[i]));
		video_num++;
			 }
  	tag_min = tag_num[0];
  	for(i=1;i<MAX_FRM_URL_MAP;i++)
				  {
  	  if(tag_num[i]==0)
					break;
	  tag_max = tag_num[i];
  	  tag_min = MIN(tag_min,tag_max);
	  if(tag_min==tag_max)
	     video_flv_idx = i;
				  }
	tag_m = tag_num[0];
  	for(i=1;i<MAX_FRM_URL_MAP;i++)
			 {
  	  if(tag_num[i]==tag_min)
	  	i++;
  	  if(tag_num[i]==0)
	  	break;

	  tag_max = tag_num[i];
  	  tag_m = MIN(tag_m,tag_max);
	  if(tag_m==tag_max)
	  	video_idx = i;
		 }
    if(tag_m > YOUKU3G_VIDEO_SUPPORT_ITAG)
	    return video_flv_idx;
		 else
   		return video_idx;
 }

 int youku3g_first_packet(unsigned long address, unsigned long size)
 {

	 U08* ptr = (U08*)address;
	 int len, bytecnt;
	 int val;
	 int cp = 0;

	 len = 0;
	 bytecnt = 0;
	 while(size > 0)
	 {
		 size--;
		 val = *ptr;
		switch(val)
		{
			case 0x0D:
				cp++;
				break;
			case 0x0A:
				cp++;
				break;
			default:

				if (cp!=4)
					cp=0;
				break;

		}

		 if (cp==4)
		 {
		   bytecnt++;
		   break;
		 }

		 ptr++;
		 bytecnt++;
	 }
	 return bytecnt;
 }
 static void youku3g_html_content_handler(void *user_data, const char *content, int len)
 {

     youku3g_info_t   *youku3g_info    = (youku3g_info_t *) user_data;
	 //   mpDebugPrint("youtube_html_content_handler %d %s",len,content);
	 
     if(youku3g_info->state == YOUKU3G_VIDEO_LIST_TITLE)
     {
		strcpy(youku3g_info->cur_video->title,content);
		mpDebugPrint("Title %s",youku3g_info->cur_video->title);
		youku3g_info->state = YOUKU3G_VIDEO_LIST_DURATION;
     }
	 else if(youku3g_info->state == YOUKU3G_VIDEO_LIST_DURATION_CP) 
	 {
	  	strcpy(youku3g_info->cur_video->duration,content);
		mpDebugPrint("Duration %s",youku3g_info->cur_video->duration);
		youku3g_info->state = YOUKU3G_VIDEO_LIST_FOUND;

	 }

 }

 static void youku3g_html_tag_start_handler(void *user_data, const char *tag_name, char **attr)
 {
	 youku3g_info_t   *youku3g_info    = (youku3g_info_t *) user_data;
	 char *title;
	 int len,i,cp_len;
	 int start_cp=0;
	 //mpDebugPrint("youtube_html_tag_start_handler user_data %s",user_data);
     if(youku3g_video_show_load >10)
     {
	 	xpgUpdateYouTubeLoading(1);
		youku3g_video_show_load=0;
     }
	 else
	 	youku3g_video_show_load++;
	 #if 1
	 if(!strncasecmp(tag_name,"h3",2))
	 {
	   youku3g_info->state = YOUKU3G_VIDEO_LIST_FOUND;
	 }
	 if(youku3g_info->state	==YOUKU3G_VIDEO_LIST_FOUND)
	 {
		 if(!strncasecmp(tag_name,"img",3))
		 {

			 video_entry_t		 *video_entry;

			 while (*attr)
			 {
				 if(!strncasecmp(*attr,"src",3))
				 {
				   attr ++;

				   mpDebugPrint("*attr src %s ",*attr);
				   if(!strncasecmp(*attr,"http",4))
				   {
						video_entry = (video_entry_t *) youku3g_malloc(sizeof(video_entry_t));
						MP_ASSERT(video_entry);
						memset(video_entry, 0, sizeof(video_entry_t));
						youku3g_info->cur_video->next = video_entry;
						youku3g_info->cur_video = video_entry;
						youku3g_info->state = YOUKU3G_VIDEO_LIST_ID;
						youku3g_info->video_num++;
						MP_DEBUG1("video_num %d ",youku3g_info->video_num);
					    memset(youku3g_info->cur_video->thumbnail,0x00,256);
					    memcpy(youku3g_info->cur_video->thumbnail,*attr,strlen(*attr));
					    MP_DEBUG1("src %s ",youku3g_info->cur_video->thumbnail);
				   }

			 	 }

				 attr ++;
			 }
		 }
		 else if(!strncasecmp(tag_name,"div",3))
		 {
		 		
			  while (*attr)
			 {
				 //mpDebugPrint(" %s ",*attr);
				 attr ++;
			 }

		}
		 else if(!strncasecmp(tag_name,"a",1))
		 {
		 		
			  while (*attr)
			 {
				 //mpDebugPrint(" %s ",*attr);
				 attr ++;
			 }

		}


 	}
	else if(youku3g_info->state	==YOUKU3G_VIDEO_LIST_ID)
	{	
	     if(!strncasecmp(tag_name,"a",1))
		 {
		 	  char *ptr;
			  while (*attr)
			 {
				 //mpDebugPrint(" %s ",*attr);
				 if(!strncasecmp(*attr,"href",4))
				 {
				    attr ++;
					memset(youku3g_info->cur_video->id,0x00,MAX_ALBUMID);
					ptr = strstr(*attr,"id=");
				    memcpy(youku3g_info->cur_video->id,ptr+3,strlen(ptr));
					mpDebugPrint("youku3g_info->cur_video->id %s ",youku3g_info->cur_video->id);
					youku3g_info->state	=YOUKU3G_VIDEO_LIST_TITLE;

				 }


				 attr ++;
			 }

		}
		 else if(!strncasecmp(tag_name,"div",3))
		 {
		 		
			  while (*attr)
			 {
				 //mpDebugPrint(" %s ",*attr);
				 attr ++;
			 }

		}
		 else if(!strncasecmp(tag_name,"a",1))
		 {
		 		
			  while (*attr)
			 {
				 //mpDebugPrint(" %s ",*attr);
				 attr ++;
			 }

		}


	}
	else if(youku3g_info->state	==YOUKU3G_VIDEO_LIST_DURATION)
	{
	  	if(!strncasecmp(tag_name,"span",4))
	  	{
	  	   	 while (*attr)
			 {
				 //mpDebugPrint(" %s ",*attr);
				 if(!strncasecmp(*attr,"num",3))
				 	youku3g_info->state	=YOUKU3G_VIDEO_LIST_DURATION_CP;

				 attr ++;
			 }
	  	}

	}

    #else
    while (*attr)
	 {

		 mpDebugPrint(" %s ",*attr);
		 attr ++;
	 }
	 #endif


 }

 static void youku3g_html_tag_end_handler(void *user_data, const char *tag_name)
 {

 }
 /*******************************************************
 /note:For Youtube get preview picture
 /
 /
 /*******************************************************/
 char * youku3g_get_video_pricture_buffer_by_index(video_entry_t  *tmp_entry, int index)
 {
     int i,len;
     Xml_BUFF_init(NET_RECVHTTP);

     i=index;
     {
      tmp_entry->thumbnailsize = Get_Image_File(tmp_entry->thumbnail, App_State.XML_BUF);
     }

        char *youku3g_buf;
        youku3g_buf = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);

  //mpDebugPrint("youtube_buf=0x%X, size %d",youtube_buf, len);
  //mpDebugPrint("youtube_buf = %2X %2X %2X %2X %2X %2X %2X %2X", *youtube_buf, *(youtube_buf+1), *(youtube_buf+2), *(youtube_buf+3), *(youtube_buf+4), *(youtube_buf+5), *(youtube_buf+6), *(youtube_buf+7) );
     Xml_BUFF_free(NET_RECVHTTP);

     return youku3g_buf;

 }

 void youku3g_get_video_pricture_buffer_free(char *youku3g_buf)
 {

	if(youku3g_buf != NULL)
		ext_mem_free(youku3g_buf);
 }
int youku3g_get_video_info(DWORD dwIndex)
{
	youku3g_info_t *youku3g = &youku3g_info;
    void *curl;
    char *data;
    char *bigger_buff=NULL;
	BYTE *youku3g_id;
	BYTE youku3g_url[256];
    struct curl_slist *header=NULL;
    int  ret = 0;
	CURLcode res;
    int httpcode;
    XML_BUFF_link_t *ptr;
    int len;
	U32 i,j,k;
	BYTE findflag=0,findmap=0,findhttp=0;
	BYTE transfer_word,transfer_char,transfer_num,cp;

	//mpDebugPrint("youtube_get_video_info %d",dwIndex);

	Xml_BUFF_init(NET_RECVHTTP);

    curl = curl_easy_init();
    if(curl) {
        data = youku3g_malloc(512);
        if (data == NULL)
            goto exit;
		youku3g_id=youku3g_getvideo_id(dwIndex);
		
		mpDebugPrint("youku3g_id %s",youku3g_id);
		if(youku3g_play_mode)
			snprintf(youku3g_url, 256,
				 "http://%s/wap2/video.jsp?showid=%s",YOUKU3G_HOST,youku3g_id);
		else
		snprintf(youku3g_url, 256,
				 "http://%s/wap2/video.jsp?id=%s",YOUKU3G_HOST,youku3g_id);


			//mpDebugPrint("youtube_url %s",youtube_url);
			snprintf(data, 512,"User-Agent:Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16");
			data[511] = '\0';
			header = curl_slist_append(header, data);
			snprintf(data, 512,"Accept:text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
			data[511] = '\0';
			header = curl_slist_append(header, data);
			if(youku3g_play_mode)
			{
				snprintf(data, 512,"Referer: http://3g.youku.com/wap2/tvshow.jsp?showid=%s",youku3g_tv_show_id);
				data[511] = '\0';
				header = curl_slist_append(header, data);
			}
			else	
			{
			snprintf(data, 512,"Referer: http://3g.youku.com/wap2/index.jsp");
			data[511] = '\0';
			header = curl_slist_append(header, data);
			}

	        //mpDebugPrint("youtube_get_video_info: extra header=%d", strlen(data));
	        curl_easy_setopt(curl, CURLOPT_URL, youku3g_url);
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
				mpDebugPrint("youtube_get_video_location: curl_easy_perform failed, res = %d", res);
				httpcode = 0;
				goto YouKU3G_cleanup_1;
			}

			if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
				httpcode = 0;

		YouKU3G_cleanup_1:
			/* always cleanup */
			curl_easy_cleanup(curl);

			curl_slist_free_all(header);

			youku3g_mfree(data);

			mpDebugPrint("youku3g_get_video_info: http error resp code=%d", httpcode);
			if (httpcode == 200)
			{
				bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
				mpDebugPrint("youku3g_get_video_info: got total data len = %d", len);
				//NetAsciiDump(bigger_buff,len);
				for(i=0,k=0;i<len;i++)
				{
				  if(!strncasecmp(bigger_buff+i,"play",4))
				  {
				  	//mpDebugPrint("!!!!PLAY!!!!");
					findflag = 1;
				  }
				  if(findflag)
				  {
				    if(!strncasecmp(bigger_buff+i,"http://",7))
				    {
						findhttp = 1;
						j=0;
				    }

				  }
				  if(findhttp)
				  {
				  	if(bigger_buff[i]=='"')
				  	{
				  	    findflag = 0;
						findhttp = 0;
						//mpDebugPrint("youku3g->youku3g_location %s",youku3g->youku3g_location);
						youku3g->youku3g_location[j]='\0';
						k++;
						if(k>1)
							break;
						continue;
				  	}
					youku3g->youku3g_location[j]=bigger_buff[i];
					j++;
				  }
				  
				
				}
				//mpDebugPrint("youku3g->youku3g_location %s",youku3g->youku3g_location);

			}
			else if (httpcode > 0)					/* TODO */
			{
				mpDebugPrint("youku3g_get_video_info: http error resp code=%d", httpcode);
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
			return httpcode;

}
int youku3g_get_video_location(void)
{

    void *curl;
    CURLcode res;
    XML_Parser              parser;
    int                     ret = 0;
    int                     len=0,i=0,buf_len=0;
    char *data;
    XML_BUFF_link_t *ptr;
    int httpcode;
    struct curl_slist *header=NULL;
	BYTE *youku3g_url = NULL;//[1024];
	BYTE *youku3g_id;
	short pr = FALSE;
	U08* extptr;
	youku3g_info_t *youku3g = &youku3g_info;
	BYTE video_idx=0;
	u8 *tmpbuf;
	youku3g_url = (BYTE *) ext_mem_malloc(1024);
	tmpbuf = (u8 *) ext_mem_malloc(8*1024);
	memset(youku3g_url,0x00,1024);
    Xml_BUFF_init(NET_RECVHTTP);

    curl = curl_easy_init();
    if(curl) {
        data = youku3g_malloc(512);
        if (data == NULL)
            goto exit;
	video_idx = youku3g_get_video_idx();
	youku3g_id = youku3g_getvideo_id(video_idx);
	mpDebugPrint("youku3g_id %s",youku3g_id);
	mpDebugPrint("youku3g->youku3g_location %s",youku3g->youku3g_location);

    snprintf(youku3g_url, 1024,"%s",youku3g->youku3g_location);

	mpDebugPrint("youku3g_url %s",youku3g_url);
	snprintf(data, 512,"User-Agent:Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16");
	data[511] = '\0';
    header = curl_slist_append(header, data);
	snprintf(data, 512,"Accept:text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    data[511] = '\0';
    header = curl_slist_append(header, data);
	
	snprintf(data, 512,"Accept-Charset:UTF-8,*");
    data[511] = '\0';
    header = curl_slist_append(header, data);

	snprintf(data, 512,"Keep-Alive:115");
    data[511] = '\0';
    header = curl_slist_append(header, data);
	
	snprintf(data, 512,"Accept-Language:zh-tw,en-us;q=0.7,en;q=0.3");
    data[511] = '\0';
    header = curl_slist_append(header, data);

	snprintf(data, 512,"Referer:http://3g.youku.com/wap2/video.jsp?id=%s",youku3g_id);
    data[511] = '\0';
    header = curl_slist_append(header, data);

    //mpDebugPrint("youtube_get_video_location: extra header=%d", strlen(data));
    curl_easy_setopt(curl, CURLOPT_URL, youku3g_url);
	curl_easy_setopt(curl, CURLOPT_ENCODING,"gzip, deflate");			 // Accept-Encoding
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, App_State.XML_BUF);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, range_write_func1);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
	//curl_easy_setopt(curl, CURLOPT_RANGE, "0-8191");	//256*1024 "0-262143" "0-8192"
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, tmpbuf);
	mpDebugPrint("call curl_easy_perform %p %p" ,tmpbuf,App_State.XML_BUF);
    res = curl_easy_perform(curl);
    TaskYield();

    /* check return code of curl_easy_perform() */
    if (res != CURLE_OK)
    {
        mpDebugPrint("youku3g_get_video_location: curl_easy_perform failed, res = %d", res);
        httpcode = 0;
        goto YouKu3G_cleanup_1;
    }

    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
        httpcode = 0;

YouKu3G_cleanup_1:
    /* always cleanup */
    curl_easy_cleanup(curl);

    curl_slist_free_all(header);

    youku3g_mfree(data);

    mpDebugPrint("youku3g_get_video_location: http error resp code=%d", httpcode);
    if (httpcode == 200 || httpcode == 206)
    {

		mpDebugPrint("youku3g_get_video_location: got total data len = %d", len);
		//strcpy(youku3g->youku3g_location,youku3g_url);
		httpcode = 303;

    }else if (httpcode == 303||httpcode == 302)
    {
		memset(youku3g->youku3g_location,0x00,1024);
		mpDebugPrint("App_State.XML_BUF->BUFF = %d", strlen(App_State.XML_BUF->BUFF));
		mpDebugPrint("App_State.XML_BUF->BUFF = %s", App_State.XML_BUF->BUFF);
		mmcp_memcpy(youku3g->youku3g_location,&App_State.XML_BUF->BUFF[10],strlen(App_State.XML_BUF->BUFF)-10);
		httpcode = 303;
    }
    else if (httpcode > 0)                  /* TODO */
    {
        mpDebugPrint("youku3g_get_video_location: http error resp code=%d", httpcode);
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
    Xml_BUFF_free(NET_RECVHTTP);
	if( youku3g_url )
		ext_mem_free(youku3g_url);
	if( tmpbuf )
		ext_mem_free(tmpbuf);
   	return httpcode;
}
int youku3g_get_data(BYTE *gbuf,DWORD start,DWORD END)
{
   int video_cp_size = 0;
   int mv_data_size = 0;
   int mv_qdata_size = 0;
   //mpDebugPrint("youku3g_get_data idx %d total %d",youku3g_video.youku3g_video_start,youku3g_video.youku3g_video_total);
   //The video want to get size
   video_cp_size = (END - start)+1;

   if(youku3g_video.youku3g_video_error||(App_State.dwState&NET_LINKUP)!=NET_LINKUP)
   	{
	   	mv_data_size = -1;
		mpDebugPrint("Net work LINK DOWN");
		youku3g_video.youku3g_video_error = 1;
		SockIdSignalTcpFinRecvd(youku3g_conn);
		goto get_end;
   	}
   if(((youku3g_video.youku3g_video_total < YOUKU3G_BUFFER_LOW)&&(!youku3g_video.youku3g_video_data_end))||youku3g_video.youku3g_video_waiting)
   {
        if(!youku3g_video.youku3g_video_waiting)
			youku3g_video.youku3g_video_waiting = 1;

        if((youku3g_video.youku3g_video_total > YOUKU3G_BUFFER_MIN)||youku3g_video.youku3g_video_data_end)
        {
          youku3g_video.youku3g_video_waiting = 0;
        }
		else
   {
   		mv_data_size = 0;
   		goto get_end;
   }
   }

   if(youku3g_video.youku3g_video_total > video_cp_size)
   {
   	mv_data_size =  video_cp_size;
   }
   else
   	{
   	 mv_data_size = youku3g_video.youku3g_video_total;
	 //Clear move video data size
	 //youtube_video_total = 0;
   	}

   //mpDebugPrint("youtube_video_start %d ",youtube_video_start);

   //mpDebugPrint("mv_data_size %d ",mv_data_size);
   //Girbuf ring end point
   if(bufindex_end!=0)
   {
   	   if((youku3g_video.youku3g_video_start+mv_data_size)> bufindex_end)
       {
         mv_qdata_size = (youku3g_video.youku3g_video_start+mv_data_size)- bufindex_end;

		 //mpDebugPrint("mv_qdata_size %d",mv_qdata_size);
		 //mv_data_size-=mv_qdata_size;
		 mmcp_memcpy(gbuf,girbuf+youku3g_video.youku3g_video_start,mv_data_size-mv_qdata_size);
		 //Clear video get ring buf index
		 youku3g_video.youku3g_video_start = 0;
		 mmcp_memcpy(gbuf+(mv_data_size-mv_qdata_size),girbuf+youku3g_video.youku3g_video_start,mv_qdata_size);
		 youku3g_video.youku3g_video_start += mv_qdata_size;
		 bufindex_end = 0;
		 }
		 else
		 {
			mmcp_memcpy(gbuf,girbuf+youku3g_video.youku3g_video_start,mv_data_size);
		   youku3g_video.youku3g_video_start+=mv_data_size;
		 }
	   }
   else
   	{
   //memcpy(gbuf,girbuf+youtube_video_start,mv_data_size);
   mmcp_memcpy(gbuf,girbuf+youku3g_video.youku3g_video_start,mv_data_size);
   youku3g_video.youku3g_video_start+=mv_data_size;
   	}

   youku3g_video.youku3g_video_total -= mv_data_size;

get_end:

   TaskYield();
   return mv_data_size;
}


int youku3g_video_donwload(DWORD dwIndex,BYTE hq)
{

		DWORD addr;
		char request[1024];
		int idx;
		//char buf[1024];
		unsigned char retrycount = 0;
		unsigned char breconect = 0;
		unsigned char b_ir_pause = 0;
		unsigned int failcount = 0,selectfailcount = 0;
		ST_SOCK_SET stReadSet, stWriteSet;
		ST_SOCK_SET *wfds, *rfds;
		unsigned long val = 0;
		int ret;
		int num_bytes_recv,recv_len;
		int i,j=0,cp=0;
		BYTE *website;
		U16 port;
		char *urlparm;
		BYTE * ipaddr;
		int video_play = 0;
		int video_index = 0;
		short pr = FALSE;
	 	U08* extptr;
		int video_index_debug = 0;
		int len,location_len=0;
		BYTE found_id=0;
		youku3g_info_t *youku3g = &youku3g_info;

		mpDebugPrint("youku3g_video_donwload %d",dwIndex);
	YOUKU3G_SEE_OTHER:
		memset(girbuf,0x00,NETSTREAM_MAX_BUFSIZE+8192);
		memset(request,0x00,1024);
		youku3g_video.youku3g_video_total = 0;
		//check location id
		memset(request,0x00,1024);
		for(i=0;i<strlen(youku3g->youku3g_location);i++)
		{
		  if(youku3g->youku3g_location[i]==0x0D)
		  	youku3g->youku3g_location[i] = 0;
		  if(youku3g->youku3g_location[i]==0x0A)
		  	youku3g->youku3g_location[i] = 0;

		}

		website =(BYTE *) Net_GetWebSite(youku3g->youku3g_location);
		port = Net_GetWebSitePort(youku3g->youku3g_location);
		//mpDebugPrint("youtube_location %s",youtube_location);

		urlparm = strstr(youku3g->youku3g_location+strlen(website),"/");
		mpDebugPrint("website %s",website);
		mpDebugPrint("port %d",port);
		mpDebugPrint("urlparm %s",urlparm);

		ipaddr = (BYTE *)SearchServerByName(website);

		if( ipaddr )
		{
			//mpDebugPrint("ipaddr %x",ipaddr[0]);
			addr = *((DWORD *) ipaddr);
			ret = mpx_DoConnect(addr, port, TRUE);
		}
		else
		{
			addr=inet_addr(website);
			ret = mpx_DoConnect(addr, port, TRUE);
		}
		//transfer IP address
		//mpDebugPrint("addr %x",addr);
		if( ret > 0 )
		{
			youku3g_conn = ret;
		}
		else
		{
			mpDebugPrint("youku3g connect fail, retry");
			closesocket(youku3g_conn);
			return -1;
		}
		snprintf(request, 1024,
					"GET %s HTTP/1.1\r\n"
					"User-Agent:Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16\r\n"
					//"Accept-Language:zh-tw,en-us;q=0.7,en;q=0.3\r\n"
					"Accept:text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
					"Proxy-Connection: Keep-Alive\r\n"
					"Host: %s\r\n"
					"Referer:http://3g.youku.com/wap2/video.jsp?id=%s\r\n"
					"\r\n",
					urlparm,website,youku3g_getvideo_id(dwIndex));
		mpDebugPrint("InterRadioStart ret %x\n",ret);
		MPX_FD_ZERO(&stReadSet);
		MPX_FD_ZERO(&stWriteSet);
		wfds = rfds = NULL;
		rfds = &stReadSet;
		MPX_FD_SET(youku3g_conn, &stReadSet);
		send( youku3g_conn, request, strlen(request) , 0);
		mpDebugPrint("send request\n");
		idx = 0;
		retrycount = 0;
		mpDebugPrint("send request AA\n");
		num_bytes_recv = 0;
		num_bytes_recv = recv( youku3g_conn,girbuf,1024,0);
		recv_len = num_bytes_recv;
		len = 0;
		idx = 0;
		pr = FALSE;
		NetAsciiDump(girbuf,num_bytes_recv);
		memset(request,0x00,1024);

		 while(recv_len > 0)
		 {
			 recv_len--;

			 if(girbuf[idx]==0x0D)
			 {
			 	pr = TRUE;

			 	request[len] = '\0';
			 }
			 else
			 {
			 	request[len] = girbuf[idx];
				//mpDebugPrint("buf[%d] %c",len,val);
			 }

			 if (pr)
		{

				 //mpDebugPrint("0 %s", request);

				 if(!strncasecmp(request,"HTTP/1.1 303",12))
				 {
				 	mpDebugPrint("1 %s", request);
				 }
				 if(!strncasecmp(request,"HTTP/1.1 302",12))
				 {
				 	 mpDebugPrint("2 %s", request);
		}
				 else if(!strncasecmp(request,"Location: ",10))
				 {
				    memset(youku3g->youku3g_location,0x00,1024);
					//check location

					for(i=0;i<strlen(request);i++)
					{
					  if(request[i]==0x0D&&request[i+1]==0x0A&&
						request[i+2]==0x0D&&request[i+2]==0x0A)
					  {
						break;
					  }
					}
					mmcp_memcpy(youku3g->youku3g_location,request+10,i);
					mpDebugPrint("%s",youku3g->youku3g_location);
					mpDebugPrint("%s",youku3g->youku3g_location+256);
					closesocket(youku3g_conn);
					goto YOUKU3G_SEE_OTHER;
				 }
				 else if(!strncasecmp(request,"Content-Type: ",14))
				 {
					extptr = strstr(request,"/");

				    //mpDebugPrint("1 extptr %s", extptr);
					if(!strncasecmp(extptr,"/mp4",4))
					{
						youku3g_video.youku3g_video_ext = 1;
					}
					else
						youku3g_video.youku3g_video_ext = 0;
				 }
				 else if(!strncasecmp(request,"Content-Length: ",16))
				 {
					 //mpDebugPrint("1 %s", buf);
					 //NetPacketDump(buf,strlen(buf));
					 youku3g_video.youku3g_video_size = youku3gstrtoint(request+16);
					 mpDebugPrint("youku3g_video_size %d ", youku3g_video.youku3g_video_size);
				 }
				 else
				 	memset(request,0x00,1024);
				 len = 0;
				 pr = FALSE;
				 idx++;
			 }
			 else
			 	len++;
			 idx++;
		 }
		//NetPacketDump(girbuf,num_bytes_recv);
		bufindex = 0;
		bufindex = youku3g_first_packet(girbuf,num_bytes_recv);
		youku3g_video.youku3g_video_start = bufindex;
		mpDebugPrint("youku3g_video_start %d",youku3g_video.youku3g_video_start);
		youku3g_video.youku3g_video_total+=num_bytes_recv-bufindex;
		video_index = num_bytes_recv-bufindex;

		//bufindex = 0;
		bufindex = num_bytes_recv;
		//Turn on video play
		//EventSet(NETWORK_STREAM_EVENT, 0x3);
		bufindex_end = 0;
		youku3g_video.youku3g_video_data_end = 0;
		youku3g_video.youku3g_video_error = 0;
		youku3g_set_video_stop(0);
		while( 1 )
		{

		    if(youku3g_video.youku3g_video_data_end)
				goto wait_video_get_end;
		    //Check wifi recive packet too fast
		    if(youku3g_video.youku3g_video_total < YOUKU3G_BUFFER_MAX)
		    	{
					num_bytes_recv = 8192;
			num_bytes_recv = recv( youku3g_conn, &girbuf[bufindex], num_bytes_recv,0);

			//mpDebugPrint("num_bytes_recv %d",video_index);
			if( num_bytes_recv > 0 )
			{
				//mpDebugPrint("###num_read %d %x %x\n",num_bytes_recv,gtotoalbuf,gtotaldata);
				failcount = 0;
                bufindex+=num_bytes_recv;
				video_index+=num_bytes_recv;

				youku3g_video.youku3g_video_total+=num_bytes_recv;
				//NetGetFile(bufindex,girbuf,0,0);
		                if((youku3g_video.youku3g_video_total > YOUKU3G_BUFFER_MIN)&&(!video_play)||(youku3g_video.youku3g_video_total>=youku3g_video.youku3g_video_size))
                {
                   mpDebugPrint("Turn on video play");
				   EventSet(NETWORK_STREAM_EVENT, BIT2);
				   //NetPacketDump(girbuf,youtube_video_start+128);
				   video_play = 1;
				   xpgUpdateYouTubeLoading(0);

                }else if(!video_play)
                {
                   if(youku3g_video_show_load >10)
                   {
                		xpgUpdateYouTubeLoading(1);
						youku3g_video_show_load=0;
                   }
				   else
				   	youku3g_video_show_load++;

                }

				if(bufindex >= NETSTREAM_MAX_BUFSIZE)
				{

				    mpDebugPrint("Set index %d to 0!!",bufindex);
					//NetPacketDump(girbuf+(bufindex-num_bytes_recv),num_bytes_recv);
					bufindex_end = bufindex;
					bufindex = 0;
				}

				if(video_index >= youku3g_video.youku3g_video_size)
				{
					youku3g_video.youku3g_video_data_end = 1;
				}

			}
			else
			{
				TaskYield();
				failcount ++;
				mpDebugPrint("failcount %x",failcount);
				breconect = 1;
				if(video_play)
					youku3g_video.youku3g_video_error = 2;
				else
				youku3g_video.youku3g_video_error = 1;

						break;
					}
			}
wait_video_get_end:
			if((bufindex == youku3g_video.youku3g_video_start)||youku3g_video.youku3g_video_stop)
				break;
			TaskYield();
		}

		closesocket(youku3g_conn);
		return youku3g_video.youku3g_video_error;
}

int youku3g_init(char *username,char *password,char youku3g_categories,char youku3g_pop_most,char youku3g_page_idx) // *base_dir
{
    video_entry_t     *tmp_entry;
    int     ret,i,len,count;
    char    error[32];

	mpDebugPrint("youku3g_init %s youku3g_categories %d ,youku3g_pop_most %d ,youku3g_page_idx %d",username,youku3g_categories,youku3g_pop_most,youku3g_page_idx);
    memset(&youku3g_info, 0, sizeof(youku3g_info_t));
	memset(&youku3g_video, 0, sizeof(youku3g_video_t));
    strncpy(youku3g_info.username, username, MAX_USERNAME);
    youku3g_info.cur_video = &youku3g_info.video_list;
    youku3g_base_dir = "MPXWIFI"; //base_dir;

    //username= "MPXHOMEPAGE";

	mpDebugPrint("YouKu3G go to homepage categories %d",youku3g_categories);
	
    MP_DEBUG("1 picasa_fetch_album_list2 @@@@@@@@@@@@@@@@@@@@@@");
	youku3g_video.youku3g_video_page = youku3g_page_idx;
	youku3g_video.youku3g_video_cate = youku3g_categories;
	if(!strncasecmp(username,"MPXHOMEPAGE",11))
	{
		youku3g_play_mode = 0;
	for(i=1;i<5;i++)
	{
		    ret = youku3g_fetch_video_list(&youku3g_info,username,i,youku3g_categories);
    if (ret < 0)
    {
        MP_ALERT("picasa_fetch_album_list2(): error code: %d\n", youku3g_info.error_code);
        return ret;
    }
	}
	}
	else
	{
		username = "TVSHOW";
		youku3g_play_mode = 1;
		//get tv id list
		ret = youku3g_fetch_video_list(&youku3g_info,username,0,0);
		if (ret < 0)
		{
			MP_ALERT("picasa_fetch_album_list2(): error code: %d\n", youku3g_info.error_code);
			return ret;
		}
		//get tv list by id
		ret = youku3g_fetch_video_list(&youku3g_info,username,1,0);
		if (ret < 0)
		{
			MP_ALERT("picasa_fetch_album_list2(): error code: %d\n", youku3g_info.error_code);
			return ret;
		}


	}

#if 1
    count = 0;
    youku3g_info.cur_video = youku3g_info.video_list.next;
    tmp_entry = youku3g_get_video_entry(0);//youtube_info.cur_video;

    if (tmp_entry)
    {

        //while (tmp_entry)
        for(i=1;i<youku3g_info.video_num;i++)
        {
        	MP_DEBUG1("counter %d",count);
            MP_DEBUG2("Fetch photos from '%s' with id '%s'\n",tmp_entry->title,tmp_entry->id);
			MP_DEBUG1("Fetch photos thumbnail '%s'\n",tmp_entry->thumbnail);
            Net_Xml_PhotoSetList(tmp_entry->title,count);
            count ++;
            tmp_entry = youku3g_get_video_entry(i);//tmp_entry->next;
        }
        Net_PhotoSet_SetCount(count);
    }
    else
    {
        mpDebugPrint("No video.\n");
    }
#endif
    return 0;
}

void youku3g_exit(const char *base_dir)
{
   video_entry_t     *tmp_entry;
 /* free resources allocated for all album */

 	//Xml_BUFF_free(NET_YOUTUBE);
    //youtube_info.cur_video = youtube_info.video_list.next;
    while (youku3g_info.cur_video)
    {
        tmp_entry = youku3g_info.cur_video;
        youku3g_info.cur_video = youku3g_info.cur_video->next;

        youku3g_mfree(tmp_entry);
    }
}

int youku3g_fetch_video_list(youku3g_info_t *youku3g_info,BYTE *username,BYTE youku3g_mode,BYTE youku3g_categories)
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
    char *bigger_buff=NULL;
	char *youku3g_p;
	char *youku3g_m;
	int i,j;
	BYTE found_flag=0;

    Xml_BUFF_init(NET_RECVHTTP);
	mpDebugPrint("NETFS_YOUKU3G: youku3g_fetch_video_list %d",youku3g_mode);
	if(!strncasecmp(username,"MPXHOMEPAGE",11))
	{

	switch(youku3g_mode)
	{
	  case 1:
	  	youku3g_p = "1";
		break;
	  case 2:
	  	youku3g_p = "2";
		break;
	  case 3:
	  	youku3g_p = "3";
		break;
	  case 4:
	  	youku3g_p = "4";
		break;
	  case 5:
	  	youku3g_p = "5";
		break;

	}
	switch(youku3g_categories)
	{
	  case 2:
	  	youku3g_m = "94";
		break;
	  case 4:
	  	youku3g_m = "86";
		break;
	  case 6:
	  	youku3g_m = "99";
		break;
	
	  case 8:
	  	youku3g_m = "95";
		break;
	  default:
	  	youku3g_m = "0";
		break;

	}
	
	snprintf(youku3g_api_url, MAX_URL,"http://%s/wap2/index.jsp?cid=%s&pg=%s",YOUKU3G_HOST,youku3g_m,youku3g_p);
	}
	else if(!strncasecmp(username,"TVSHOW",6)) 
	{
		switch(youku3g_mode)
		{
		  case 0:
		  	snprintf(youku3g_api_url, MAX_URL,"http://%s/wap2/tvshow.jsp?",YOUKU3G_HOST);
			break;
		  case 1:
		  	snprintf(youku3g_api_url, MAX_URL,"http://%s/wap2/tvshow.jsp?showid=%s",YOUKU3G_HOST,youku3g_tv_show_id);
			break;

		}

	}

	mpDebugPrint("youku3g_api_url %s",youku3g_api_url);
    curl = curl_easy_init();
    if(curl) {
        data = youku3g_malloc(512);
        if (data == NULL)
            goto exit;
		
		snprintf(data, 512,"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
		data[511] = '\0';
        header = curl_slist_append(header, data);

        snprintf(data, 512, "User-Agent:Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16
");
        data[511] = '\0';
        header = curl_slist_append(header, data);
        mpDebugPrint("YOUTUBE: extra header=%d", strlen(data));
        curl_easy_setopt(curl, CURLOPT_URL, youku3g_api_url);
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
            mpDebugPrint("youtube_fetch_album_list2: curl_easy_perform failed, res = %d", res);
            httpcode = 0;
            goto Youtube_cleanup_1;
        }

        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
            httpcode = 0;

Youtube_cleanup_1:
        /* always cleanup */
        curl_easy_cleanup(curl);

        curl_slist_free_all(header);

        youku3g_mfree(data);

        mpDebugPrint("youtube_fetch_video_list: http error resp code=%d", httpcode);
        if (httpcode == 200)
        {
			bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			mpDebugPrint("youtube_fetch_video_list: got total data len = %d", len);
			//NetAsciiDump(bigger_buff, len);
			if((!strncasecmp(username,"TVSHOW",6))&&(!youku3g_mode)) 
			{
				//NetAsciiDump(bigger_buff, len);
				for(i=0;i<len;i++)
				{
					if(!strncasecmp(bigger_buff+i,"showid=",7))
					{
					  //mpDebugPrint("FOUND SHOWID %d !!!",i);
					  memset(youku3g_tv_show_id,0x00,MAX_ALBUMID);
					  i+=7;
					  found_flag = 1;
					  j=0;
					}
					if(found_flag)
					{
 					  //mpDebugPrint("bigger_buff[%d] = %c ",i,bigger_buff[i]);
					  if(bigger_buff[i]=='"')
					  {
					     mpDebugPrint("youku3g_tv_show_id %s ",youku3g_tv_show_id);
					  	 break;
					  }
					  youku3g_tv_show_id[j] = bigger_buff[i];
					  j++;
					}
					
				}

			}
			else
			{
			youku3g_info->state = YOUKU3G_VIDEO_LIST_INIT;
			html_init(&youku3g_html_parser, youku3g_info);
			html_set_content_handler(&youku3g_html_parser, youku3g_html_content_handler);
			html_set_tag_start(&youku3g_html_parser, youku3g_html_tag_start_handler);
			html_set_tag_end(&youku3g_html_parser, youku3g_html_tag_end_handler);
			html_parse(&youku3g_html_parser,bigger_buff,len);
			html_exit(&youku3g_html_parser);
			}
			xpgUpdateYouTubeLoading(0);
			
        }
        else if (httpcode > 0)                  /* TODO */
        {
            mpDebugPrint("NETFS_YOUTUBE: http error resp code=%d", httpcode);
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

#endif
#endif//HAVE_YOUKU3G
