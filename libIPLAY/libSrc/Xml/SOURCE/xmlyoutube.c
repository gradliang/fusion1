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
#define LOCAL_DEBUG_ENABLE 0

#include <stdio.h>
#include <string.h>
#include <linux/types.h>
#include <expat.h>
#include "netfs.h"
#include "netfs_pri.h"
#include "xmlyoutube.h"
#include "..\..\lwip\include\net_sys.h"
#include "..\..\lWIP\include\net_socket.h"
#include "..\..\CURL\include\net_curl_curl.h"
#include "..\..\netstream\include\netstream.h"
#include "taskid.h"

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
DWORD youtubestrtoint(BYTE * ptr)
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

#if HAVE_YOUTUBE

#define VIDEO_WRITE_SD   0

static char     youtube_api_url[MAX_URL];
youtube_info_t   youtube_info;
static const char   *youtube_base_dir;
//static DWORD youtube_description_idx =0;
static html_parser_t youtube_html_parser;
#if VIDEO_WRITE_SD||HAVE_HTTP_SERVER
STREAM *w_shandle;
DRIVE *w_sDrv;
#endif
static youtube_video_t youtube_video;
int youtube_video_show_load = 0;
int iradio_conn;

#define YOUTUBE_HOST "www.youtube.com"
#define youtube_malloc(sz)   ext_mem_malloc(sz)
#define youtube_mfree(ptr)   ext_mem_free(ptr)
#define YOUTUBE_ID_LENGTH     16
#define YOUTUBE_BUFFER_MAX     (768*1024)
#define YOUTUBE_BUFFER_HIGH2     YOUTUBE_BUFFER_MAX
#define YOUTUBE_BUFFER_HIGH     (512*1024)
#define YOUTUBE_BUFFER_LOW     (64*1024)
#define YOUTUBE_VIDEO_NUM     30
#define YOUTUBE_VIDEO_SUPPORT_MAX_ITAG     22
#define YOUTUBE_VIDEO_SUPPORT_MIN_ITAG     18

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

int youtube_ClientLogin(youtube_info_t *youtube_info, char *email, char *passwd, char *source, char *errstring);

int youtube_get_network_states()
{
    return youtube_video.youtube_video_error;
}

int youtube_get_video_index()
{
    return youtube_video.youtube_video_idx;
}
void youtube_set_video_index(int idx)
{
    youtube_video.youtube_video_idx = idx;
}
void youtube_set_video_stop(int flag)
{
    youtube_video.youtube_video_stop = flag;
}

#if NETWARE_ENABLE
#if VIDEO_WRITE_SD||HAVE_HTTP_SERVER
DWORD sd_write_len = 0;
DWORD mp4_file_size = 0;

BYTE mp4_header_flag = FALSE;
BYTE get_range_flag = FALSE;
BYTE get_range_data = FALSE;
BYTE get_range_check = FALSE;

size_t sd_write_func(void *ptr, size_t size, size_t nmemb, void *buf)
{
    size_t len = size * nmemb;
    size_t write_len;		
	BYTE bMcardId = SD_MMC;
	BYTE tmp[32];
	DWORD mp4_header[2] = {0x0000001C,0x00000018};
	DWORD ret =0;
	WORD mp4_end = 0x0d0a;
    BYTE *target;
    if (len > 0)
    {
        /* check data length to avoid buffer overflow */
        {
			//mpDebugPrint("range_write_func1 %p %p len %d",buf,ptr,len);

			if(!strncasecmp(ptr,"Content-Length: ",strlen("Content-Length: ")))
			{
			    if(!get_range_flag)
			    {
				    strcpy(tmp,ptr+strlen("Content-Length: "));
					
					mp4_file_size = youtubestrtoint(tmp);
					mpDebugPrint("mp4_file_size %d",mp4_file_size);
			    }

			}else if((!memcmp(ptr,&mp4_header[0],4))||(!memcmp(ptr,&mp4_header[1],4)))
			{
				mpDebugPrint("MP4 File Header");
				w_sDrv = DriveGet(bMcardId);
				ret = CreateFile(w_sDrv, "video","tmp");
				if (ret)
				{
						UartOutText("create file fail\r\n");
				}
				else
				{
					w_shandle = FileOpen(w_sDrv);
					if (!w_shandle)
						UartOutText("open file fail\r\n");
					else
					{
						mp4_header_flag = TRUE;
						sd_write_len+=len;
						ret = FileWrite(w_shandle, ptr, len);
						if (!ret) UartOutText("write file fail\r\n");
					}
				}

					
			}else if(mp4_header_flag)
			{
			  if(get_range_flag)
			  {
			  	if(!strncasecmp(ptr,"Content-Range: ",strlen("Content-Range: ")))
			  	{
					get_range_data = TRUE;
					target = stristr(ptr, "/");
		            target += strlen("/");
					mp4_file_size = youtubestrtoint(target);
					mpDebugPrint("mp4_file_size %d",mp4_file_size);

					NetAsciiDump(ptr, len);
					
			  	}else if(get_range_data)
				{
				    //NetPacketDump(ptr, len);

				    if((!memcmp(ptr,&mp4_end,2))&&(len==2)&&(!get_range_check))
				    {
						get_range_check = TRUE;
				    }else if(get_range_check)
				    {
				    	sd_write_len+=len;
				  		ret = FileWrite(w_shandle, ptr, len);
						if (!ret) UartOutText("write file fail\r\n");
					}

				}
			  }
			  else
			  {
				  sd_write_len+=len;
				  ret = FileWrite(w_shandle, ptr, len);
					if (!ret) UartOutText("write file fail\r\n");
			  }

			  //mpDebugPrint("sd_write_len %d",sd_write_len);
			}

            write_len = len;
			buf+= len;
        }
		
    }
    //mpDebugPrint("1 my_write_func(): write (%d) bytes into buffer.", write_len);
    return write_len;
}
#endif
BYTE *youtube_get_video_ext()
{
  BYTE *youtube_ext;
  //youtube_video_ext = 1;
  switch(youtube_video.youtube_video_ext)
  {
    case 0:
		youtube_ext = "flv";
		break;
	case 1:
		youtube_ext = "mp4";
		break;
		
	case 2:
		youtube_ext = "264";
		break;
  }
  mpDebugPrint("youtube_ext %s",youtube_ext);
  return youtube_ext;
}
youtube_info_t * getYouTubeInfo()
{
    return &youtube_info;
}
video_entry_t *youtube_get_video_entry(BYTE idx)
{
    video_entry_t       *youtube_video;
	int i;

	youtube_video = youtube_info.cur_video;
	if(idx == 0)
		return youtube_video;

	if((idx>0)&&(idx < youtube_info.video_num))
	{
	    for(i=0;i<idx;i++)
	    {
	    	youtube_video = youtube_video->next;
	    }

	}
	else
		youtube_video = NULL;

    return youtube_video;
}
int youtube_get_video_total(void)
{
    return youtube_info.video_num;
}

STREAM * getYouTubehandle()
{
    STREAM *youtube_shandle;
#if DM9KS_ETHERNET_ENABLE
    DRIVE *youtube_sDrv = DriveGet(CF_ETHERNET_DEVICE);
#else
    DRIVE *youtube_sDrv = DriveGet(USB_WIFI_DEVICE);
#endif

    youtube_sDrv->Flag.Present = 1;
    if (!(youtube_shandle = GetFreeFileHandle(youtube_sDrv)))
        return NULL;

    youtube_shandle->DirSector = 0;
    youtube_shandle->Chain.Start = 0;
    youtube_shandle->Chain.Point = 0;
    youtube_shandle->Chain.Size = youtube_video.youtube_video_size;
    g_psNet_FileBrowser->dwNumberOfFile = 1;

    return youtube_shandle;
}

BYTE *youtube_getvideo_id(DWORD dwIndex)
{
	video_entry_t		*video_entry;
	int i=0;

	video_entry = youtube_info.cur_video;
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
 int youtube_get_tag(BYTE *url, int len)
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
	   tag_num = youtubestrtoint(tmpbuf);
		 }
		 }

   return tag_num;
 }
 int youtube_get_video_idx()
		 {
 	youtube_info_t *youtube = &youtube_info;
    int i;
	int tag_num[MAX_FRM_URL_MAP],tag_min,tag_max;
	int video_num=0;

	for(i=0;i<MAX_FRM_URL_MAP;i++)
		tag_num[i]=0;

	for(i=0;i<MAX_FRM_URL_MAP;i++)
			 {
	    if(youtube->cur_video->frm_url_map[i]==NULL)
			break;
		tag_num[i] = youtube_get_tag(youtube->cur_video->frm_url_map[i],strlen(youtube->cur_video->frm_url_map[i]));
		video_num++;
			 }
	
  	tag_min = YOUTUBE_VIDEO_SUPPORT_MIN_ITAG;
  	for(i=0;i<MAX_FRM_URL_MAP;i++)
				  {
  	  if(tag_num[i]==0)
					break;
	  if(tag_num[i]==tag_min)
	  	{
	  	   return i;

				  }
	}
	tag_max = YOUTUBE_VIDEO_SUPPORT_MAX_ITAG;
  	for(i=0;i<MAX_FRM_URL_MAP;i++)
			 {
  	  if(tag_num[i]==0)
	  	break;
	  if(tag_num[i]==tag_max)
	  	{
	  	   return i;

		 }

	 }
	if(i==MAX_FRM_URL_MAP)
		return i;
	
 }

 int youtube_first_packet(unsigned long address, unsigned long size)
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
 static void youtube_html_content_handler(void *user_data, const char *content, int len)
 {

     youtube_info_t   *youtube_info    = (youtube_info_t *) user_data;
	 //mpDebugPrint("youtube_html_content_handler %d %s",len,content);
     if(youtube_info->state == YOUTUBE_VIDEO_LIST_DURATION)
     {
		strcpy(youtube_info->cur_video->duration,content);
		youtube_info->state = YOUTUBE_VIDEO_LIST_FOUND;
     }

 }

 static void youtube_html_tag_start_handler(void *user_data, const char *tag_name, char **attr)
 {
	 youtube_info_t   *youtube_info    = (youtube_info_t *) user_data;
	 char *title;
	 int len,i,cp_len;
	 char tmpbuf[256];
	 char cpbuf[256];
	 int start_cp=0;
	 //mpDebugPrint("youtube_html_tag_start_handler user_data %s",user_data);
     //mpDebugPrint("youtube_html_tag_start_handler tag_name %s",tag_name);
     if(youtube_video_show_load >10)
     {
#if MAKE_XPG_PLAYER
	 	xpgUpdateYouTubeLoading(1);
#endif
		youtube_video_show_load=0;
     }
	 else
	 	youtube_video_show_load++;
	 #if 1
	 if(!strncasecmp(tag_name,"html",4))
	 {
		 while (*attr)
		 {

			 //mpDebugPrint(" %s ",*attr);
			 attr ++;
		 }
	 }
	 else if(!strncasecmp(tag_name,"img",3))
	 {

		 video_entry_t		 *video_entry;

		 while (*attr)
		 {
		     if(!strncasecmp(*attr,"title",5))
		     {
		        if(youtube_info->video_num > YOUTUBE_VIDEO_NUM)
					break;
		       attr ++;
			   memcpy(youtube_info->cur_video->title,*attr,strlen(*attr));
			   youtube_info->cur_video->title[strlen(*attr)] = '\0';
			   MP_DEBUG1("title %s ",youtube_info->cur_video->title);

		     }
			 else if(!strncasecmp(*attr,"src",3))
			 {
			   attr ++;

			   memset(tmpbuf,0x00,256);
			   memset(cpbuf,0x00,256);
			   //check HQdefault.jpg

			   //mpDebugPrint("*attr src %s ",*attr);
			   memcpy(tmpbuf,*attr,256);
			   if((tmpbuf[strlen(*attr)-3]=='j')&&(tmpbuf[strlen(*attr)-2]=='p')&&
			   	(tmpbuf[strlen(*attr)-1]=='g'))
			   	{
					video_entry = (video_entry_t *) youtube_malloc(sizeof(video_entry_t));
					MP_ASSERT(video_entry);
					memset(video_entry, 0, sizeof(video_entry_t));
					youtube_info->cur_video->next = video_entry;
					youtube_info->cur_video = video_entry;
					youtube_info->state = YOUTUBE_VIDEO_LIST_FOUND;
					youtube_info->video_num++;
					MP_DEBUG1("video_num %d ",youtube_info->video_num);


			   memset(youtube_info->cur_video->thumbnail,0x00,256);
			   	   if((tmpbuf[strlen(*attr)-13]=='h')&&(tmpbuf[strlen(*attr)-12]=='q'))
			   	   {
				     memcpy(cpbuf,*attr,strlen(*attr)-13);
					 cpbuf[strlen(*attr)-13]='d';
					 cpbuf[strlen(*attr)-12]='e';
					 cpbuf[strlen(*attr)-11]='f';
					 cpbuf[strlen(*attr)-10]='a';
					 cpbuf[strlen(*attr)-9]='u';
					 cpbuf[strlen(*attr)-8]='l';
					 cpbuf[strlen(*attr)-7]='t';
					 cpbuf[strlen(*attr)-6]='.';
					 cpbuf[strlen(*attr)-5]='j';
					 cpbuf[strlen(*attr)-4]='p';
					 cpbuf[strlen(*attr)-3]='g';
					 cpbuf[strlen(*attr)-2]='\0';
				     memcpy(youtube_info->cur_video->thumbnail,cpbuf,strlen(cpbuf));
			   	   }
				   else
				   {
			   memcpy(youtube_info->cur_video->thumbnail,*attr,strlen(*attr));
					 MP_DEBUG1("src %s ",youtube_info->cur_video->thumbnail);

		 	 }
			   	}
		 	 }
			 else if(!strncasecmp(*attr,"thumb",5))
			 {
			   attr ++;

				 memset(tmpbuf,0x00,256);
				 memset(cpbuf,0x00,256);
				 //check HQdefault.jpg

				 //mpDebugPrint("*attr src %s ",*attr);
				 memcpy(tmpbuf,*attr,256);
				 if((tmpbuf[strlen(*attr)-3]=='j')&&(tmpbuf[strlen(*attr)-2]=='p')&&
				  (tmpbuf[strlen(*attr)-1]=='g'))
				  {
				  	video_entry = (video_entry_t *) youtube_malloc(sizeof(video_entry_t));
					MP_ASSERT(video_entry);
					memset(video_entry, 0, sizeof(video_entry_t));
					youtube_info->cur_video->next = video_entry;
					youtube_info->cur_video = video_entry;
					youtube_info->state = YOUTUBE_VIDEO_LIST_FOUND;
					youtube_info->video_num++;
					MP_DEBUG1("video_num %d ",youtube_info->video_num);

			   memset(youtube_info->cur_video->thumbnail,0x00,256);
					 if((tmpbuf[strlen(*attr)-13]=='h')&&(tmpbuf[strlen(*attr)-12]=='q'))
					 {
					   memcpy(cpbuf,*attr,strlen(*attr)-13);
					   cpbuf[strlen(*attr)-13]='d';
					   cpbuf[strlen(*attr)-12]='e';
					   cpbuf[strlen(*attr)-11]='f';
					   cpbuf[strlen(*attr)-10]='a';
					   cpbuf[strlen(*attr)-9]='u';
					   cpbuf[strlen(*attr)-8]='l';
					   cpbuf[strlen(*attr)-7]='t';
					   cpbuf[strlen(*attr)-6]='.';
					   cpbuf[strlen(*attr)-5]='j';
					   cpbuf[strlen(*attr)-4]='p';
					   cpbuf[strlen(*attr)-3]='g';
					   cpbuf[strlen(*attr)-2]='\0';
					   memcpy(youtube_info->cur_video->thumbnail,cpbuf,strlen(cpbuf));
					 }
					 else
					 {
			   memcpy(youtube_info->cur_video->thumbnail,*attr,strlen(*attr));
					 }
				  }
		 	 }
			 //mpDebugPrint(" %s ",*attr);

			 attr ++;
		 }
	 }
	 else if(!strncasecmp(tag_name,"span",4))
	 {
		 while (*attr)
		 {
			 if(!strncasecmp(*attr,"video-time",10))
			 {
			   youtube_info->state = YOUTUBE_VIDEO_LIST_DURATION;
			 }

			 attr ++;
		 }
	 }
	 else if(!strncasecmp(tag_name,"a",1))
	 {
	 //mpDebugPrint("youtube_info->state %d",youtube_info->state);
	 	if(youtube_info->state == YOUTUBE_VIDEO_LIST_FOUND)
	 		{
	 while (*attr)
	 {
		 	if(!strncasecmp(*attr,"href",4))
		 	{
		 	attr ++;
			memset(tmpbuf,0x00,256);
		 	memcpy(tmpbuf,*attr,strlen(*attr));
		 	//mpDebugPrint("1 %s %d",tmpbuf,strlen(tmpbuf));
		 	for(i=0;i<strlen(tmpbuf);i++)
		 	{
		 	  if(tmpbuf[i]=='=')
		 	  {
		 	    i++;
			  	break;
		 	  }
		 	}
			memcpy(youtube_info->cur_video->id,&tmpbuf[i],strlen(tmpbuf)-i);
			//mpDebugPrint("id %s ",youtube_info->cur_video->id);
			youtube_info->state = YOUTUBE_VIDEO_LIST_ID;
		 	}
			//mpDebugPrint(" %s ",*attr);

		 attr ++;
		 }
	 		}
		else
			{
				  while (*attr)
				 {

					 //mpDebugPrint(" %s ",*attr);
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

 static void youtube_html_tag_end_handler(void *user_data, const char *tag_name)
 {
     youtube_info_t   *youtube_info    = (youtube_info_t *) user_data;
	 //mpDebugPrint("youtube_html_tag_end_handler %s",tag_name);
     if(youtube_info->state == YOUTUBE_VIDEO_LIST_ID)
     {
		 youtube_info->state = YOUTUBE_VIDEO_LIST_INIT;
     }

 }
 /*******************************************************
 /note:For Youtube get preview picture
 /
 /
 /*******************************************************/
 char * youtube_get_video_pricture_buffer_by_index(video_entry_t  *tmp_entry, int index)
 {
     int i,len;
	 #if DM9KS_ETHERNET_ENABLE||DM9621_ETHERNET_ENABLE
	 #if (P2P_TEST == 0&&MAKE_XPG_PLAYER)
         if(GetNetConfigP2PTestFlag())
	 #endif	 	
         {
	 char *youtube_buf;
 	 youtube_buf = NULL;
			 return youtube_buf;
         }
	 #endif

     Xml_BUFF_init(NET_RECVHTTP);

     i=index;
     {
      tmp_entry->thumbnailsize = Get_Image_File(tmp_entry->thumbnail, App_State.XML_BUF);
     }

        char *youtube_buf;
        youtube_buf = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);

  //mpDebugPrint("youtube_buf=0x%X, size %d",youtube_buf, len);
  //mpDebugPrint("youtube_buf = %2X %2X %2X %2X %2X %2X %2X %2X", *youtube_buf, *(youtube_buf+1), *(youtube_buf+2), *(youtube_buf+3), *(youtube_buf+4), *(youtube_buf+5), *(youtube_buf+6), *(youtube_buf+7) );
     Xml_BUFF_free(NET_RECVHTTP);
     return youtube_buf;

 }

 void youtube_get_video_pricture_buffer_free(char *youtube_buf)
 {

	if(youtube_buf != NULL)
		ext_mem_free(youtube_buf);
 }
int youtube_get_video_info(DWORD dwIndex)
{
	youtube_info_t *youtube = &youtube_info;
    void *curl;
    char *data;
    char *bigger_buff=NULL;
	BYTE *youtube_id;
	BYTE youtube_url[256];
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
        data = youtube_malloc(512);
        if (data == NULL)
            goto exit;
		youtube_id=youtube_getvideo_id(dwIndex);

		snprintf(youtube_url, 256,
				 "http://www.youtube.com/watch?v=%s&fmt=18",
				 youtube_id);

			//mpDebugPrint("youtube_url %s",youtube_url);
			snprintf(data, 512,"Accept: text/html, */*");
			data[511] = '\0';
			header = curl_slist_append(header, data);
			snprintf(data, 512,"Referer: http://www.youtube.com");
			data[511] = '\0';
			header = curl_slist_append(header, data);

	        //mpDebugPrint("youtube_get_video_info: extra header=%d", strlen(data));
	        curl_easy_setopt(curl, CURLOPT_URL, youtube_url);
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
			}
            else
			if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
				httpcode = 0;

			/* always cleanup */
			curl_easy_cleanup(curl);

			curl_slist_free_all(header);

			youtube_mfree(data);

			mpDebugPrint("youtube_get_video_info: http error resp code=%d", httpcode);
			if (httpcode == 200)
			{

				bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
				mpDebugPrint("youtube_get_video_info: got total data len = %d", len);
				//NetAsciiDump(bigger_buff,len);
				memset(youtube_url,0x00,256);
				for(i=0;i<MAX_FRM_URL_MAP;i++)
					memset(youtube->cur_video->frm_url_map[i],0x00,512);

		     for(i=0;i<len;i++)
		     {
		       if((bigger_buff[i]=='s')&&(!findflag))
		       {
			   	memcpy(youtube_url,&bigger_buff[i],8);
				i+=8;
		       }

			   if(!strncasecmp(youtube_url,"swfHTML",7))
			   {
			     findflag = 1;
				 mpDebugPrint("FOUND %s",youtube_url);
				 memset(youtube_url,0x00,256);
			   }
			   if(findflag)
				{
					if((bigger_buff[i]=='f')&&(findmap==0))
						  memcpy(youtube_url,&bigger_buff[i],12);
					if((bigger_buff[i]=='h')&&(findmap==1)&&(findhttp==0))
						  memcpy(youtube_url,&bigger_buff[i],4);

					if(!strncasecmp(youtube_url,"fmt_url_map=",12))
						{
				 		  memset(youtube_url,0x00,256);
						  mpDebugPrint("FOUND fmt_url_map!!!");
						  findmap = 1;
						  i+=12;
						  j=0;

					}
					else if(!strncasecmp(youtube_url,"http",4))
						  {
					    memset(youtube_url,0x00,256);
						transfer_char = 0;
						transfer_num = 0;
						k=0;
						findhttp = 1;
					}
					if(findhttp)
					{
					  cp =0;
                      switch(bigger_buff[i])
							 {
                        case '%':
							transfer_char++;
							break;
						case '2':
							if(transfer_char==1)
							{
								transfer_num = 2;
								transfer_char++;
							 }
							else if(transfer_char==2)
							{

						  }
							else
							{
								transfer_word = bigger_buff[i];
								cp = 1;
							}
 						    break;
						case '3':
							if(transfer_char==1)
							{
								transfer_num = 3;
								transfer_char++;
							}
							else if(transfer_char==2)
							{

							}
							else
							{
								transfer_word = bigger_buff[i];
								cp = 1;
							}
 						    break;
						case '5':
							if(transfer_char==1)
							{
								transfer_num = 5;
								transfer_char++;
							}
							else if(transfer_char==2)
							{
							  switch(transfer_num)
							  {
							    case 2:
							  	  transfer_char = 0;
							  	  transfer_word = '%';
								  cp = 1;
							  	  break;
							  }
							}
							else
							{
							    {
									transfer_word = bigger_buff[i];
									cp=1;
							    }
							}
							break;
						case '6':
							if(transfer_char==1)
							{
								transfer_num = 6;
								transfer_char++;
							}
							else if(transfer_char==2)
							{
							  switch(transfer_num)
							  {
							    case 2:
							  	  transfer_char = 0;
							  	  transfer_word = '&';
								  cp = 1;
							  	  break;
							  }
							}
							else
							{
								transfer_word = bigger_buff[i];
								cp=1;
							}

							break;
						case '7':
							if(transfer_char==1)
							{
								transfer_num = 7;
								transfer_char++;
							}
							else if(transfer_char==2)
							{
							  switch(transfer_num)
							  {
							    case 2:
							  	  transfer_char = 0;
							  	  transfer_word = '&';
								  cp = 1;
							  	  break;
							  }
							}
							else
								{
								transfer_word = bigger_buff[i];
								cp=1;
								}
							break;
						case '9':
							{
								transfer_word = bigger_buff[i];
								cp=1;
							}
							break;
						case 'A':
							if(transfer_char)
							{
							  case 3:
							  	transfer_char = 0;
							  	transfer_word = ':';
								cp = 1;
							  	break;
							}
							else
								{
								transfer_word = bigger_buff[i];
								cp=1;
								}
							break;
						case 'C':
							if(transfer_char)
							{
								switch(transfer_num)
								{
								  case 2:
								  	transfer_char = 0;
								  	transfer_word = ' ';
									cp = 1;
									findhttp = 0;
									j++;
									if(j>=MAX_FRM_URL_MAP)
										goto out;

									mpDebugPrint("j = %d",j);
						  break;

								  case 7:
								  	transfer_char = 0;
								  	transfer_word = ' ';
									cp = 1;
								  	break;
								}
							}
							else
								{
								transfer_word = bigger_buff[i];
								cp=1;
								}
							break;
						case 'D':
							if(transfer_char)
							{
								switch(transfer_num)
								{
								  case 3:
								  	transfer_char = 0;
								  	transfer_word = '=';
									cp = 1;
								  	break;
						}
							}
							else
								{
								transfer_word = bigger_buff[i];
								cp=1;
								}
							break;
						case 'F':
							if(transfer_char)
							{
								switch(transfer_num)
								{
								  case 2:
								  	transfer_char = 0;
								  	transfer_word = '/';
									cp = 1;
								  	break;
								  case 3:
								  	transfer_char = 0;
								  	transfer_word = '?';
									cp = 1;
								  	break;

								}
							}
							else
								{
								transfer_word = bigger_buff[i];
								cp=1;
								}
							break;

						default:
							    if(bigger_buff[i]!='&')
						    	{
									transfer_word = bigger_buff[i];
									cp=1;
						    	}
							break;

                      }
					  if(cp)
					  {
					  	//mpDebugPrint("%c",transfer_word);
					    youtube->cur_video->frm_url_map[j][k]=transfer_word;
						k++;
					  }

					  if(bigger_buff[i]=='&')
					  	break;


			   	}

			   	}
			   TaskYield();
					}
out:			 
	         for(i=0;i< MAX_FRM_URL_MAP;i++)
	         {
				 mpDebugPrint("youtube->cur_video->frm_url_map[%d] %d",i,strlen(youtube->cur_video->frm_url_map[i]));
				 mpDebugPrint("%s",youtube->cur_video->frm_url_map[i]);
				 mpDebugPrint("%s",youtube->cur_video->frm_url_map[i]+256);
	         }


			}
			else if (httpcode > 0)					/* TODO */
			{
				mpDebugPrint("youtube_get_video_info: http error resp code=%d", httpcode);
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
int youtube_get_video_location(void)
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
	BYTE *youtube_url = NULL;//[1024];
	BYTE *youtube_id;
	short pr = FALSE;
	U08* extptr;
	youtube_info_t *youtube = &youtube_info;
	BYTE video_idx=0;
	u8 *tmpbuf;

	youtube_url = (BYTE *) ext_mem_malloc(1024);
	tmpbuf = (u8 *) ext_mem_malloc(8*1024);
	memset(youtube_url,0x00,1024);
    Xml_BUFF_init(NET_RECVHTTP);

#if VIDEO_WRITE_SD
	BYTE get_range[256];
	BYTE start_range[32];
	BYTE tmp_range[4];
	BYTE bMcardId = SD_MMC;
	DWORD filesize = 0,value32 = 0;
	BYTE cp=0,shiftbyte=0,shiftbytecp=0;
	int j;
	
	memset(start_range,0x00,32);

	w_sDrv = DriveChange(bMcardId);
	if (DirReset(w_sDrv) != FS_SUCCEED)
		return;

	w_sDrv = DriveGet(bMcardId);
  	w_shandle = FileSearch(w_sDrv, "video","tmp", E_FILE_TYPE);

	if(w_shandle == NULL)
	{
		w_sDrv = DriveGet(bMcardId);
		w_shandle = FileOpen(w_sDrv);
		filesize = FileSizeGet(w_shandle);
		
		if(!sd_write_len)
			sd_write_len+=filesize;
		
		Seek(w_shandle,filesize);	
		mp4_header_flag = TRUE;
		get_range_flag = TRUE;
        for(i=0;i<5;i++)
        {
           switch(i)
           {
             case 4:
			 	value32 = filesize;
			 	break;
			 case 3:
			 	value32 = filesize/100;
			 	break;
             case 2:
			 	value32 = filesize/10000;
			 	break;
             case 1:
			 	value32 = filesize/1000000;
			 	break;
             case 0:
			 	value32 = filesize/100000000;
			 	break;

           }
          DecString(tmp_range,value32,2,0);
		  if((tmp_range[0]!=0x30)&&(!cp))
		  {
		  	cp=1;
			j=0;
		  }
		  else if((tmp_range[1]!=0x30)&&(!cp))
		  {
		  	memcpy(start_range,&tmp_range[1],1);
			shiftbyte++;
			shiftbytecp = 1;
			cp =1;
			j=0;

		  }
		  
		  if(cp)
		  {
		    if(shiftbytecp)
			  shiftbytecp = 0;
			else
			{
		      memcpy(start_range+(j*2)+shiftbyte,tmp_range,2);
			  j++;
			}
			
		  }
        
        }
		NetPacketDump(start_range,10);
			
	    snprintf(get_range, 256,"%s-4294967295",start_range);
	    mpDebugPrint("get_range %s",get_range);


	}
	else
		mpDebugPrint("FileSearch VIDEO.tmp file fail!!");

#endif
	//mpDebugPrint("youtube_get_video_location referer %s\n",youtube->referer);
    curl = curl_easy_init();
    if(curl) {
        data = youtube_malloc(512);
        if (data == NULL)
            goto exit;
	video_idx = youtube_get_video_idx();
	if(video_idx==MAX_FRM_URL_MAP)
			mpDebugPrint("out video_idx %d",video_idx);

		
	mpDebugPrint("video_idx %d",video_idx);
    snprintf(youtube_url, 1024,"%s",youtube->cur_video->frm_url_map[video_idx]);

	mpDebugPrint("youtube_url %s",youtube_url);
	snprintf(data, 512,"x-flash-version:   10,0,22,87");
	data[511] = '\0';
    header = curl_slist_append(header, data);
	snprintf(data, 512,"Accept: text/html, */*");
    data[511] = '\0';
    header = curl_slist_append(header, data);
    //mpDebugPrint("youtube_get_video_location: extra header=%d", strlen(data));
    curl_easy_setopt(curl, CURLOPT_URL, youtube_url);
	curl_easy_setopt(curl, CURLOPT_ENCODING,"gzip, deflate");			 // Accept-Encoding
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, App_State.XML_BUF);

#if VIDEO_WRITE_SD
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sd_write_func);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
	if(start_range[0]==0x00)
		curl_easy_setopt(curl, CURLOPT_RANGE, "0-4294967295");	//256*1024 "0-262143" "0-8192"
	else
		curl_easy_setopt(curl, CURLOPT_RANGE, get_range);	//256*1024 "0-262143" "0-8192"
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, tmpbuf);
	mpDebugPrint("call curl_easy_perform %p %p" ,tmpbuf,App_State.XML_BUF);
	//set follow location
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
#else 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, range_write_func1);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
	curl_easy_setopt(curl, CURLOPT_RANGE, "0-8191");	//256*1024 "0-262143" "0-8192"
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, tmpbuf);
	mpDebugPrint("call curl_easy_perform %p %p" ,tmpbuf,App_State.XML_BUF);
#endif

    res = curl_easy_perform(curl);
    TaskYield();

    /* check return code of curl_easy_perform() */
    if (res != CURLE_OK)
    {
        mpDebugPrint("youtube_get_video_location: curl_easy_perform failed, res = %d", res);
        httpcode = 0;
        goto Youtube_cleanup_1;
    }

    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
        httpcode = 0;

Youtube_cleanup_1:
    /* always cleanup */
    curl_easy_cleanup(curl);

    curl_slist_free_all(header);

    youtube_mfree(data);
#if VIDEO_WRITE_SD
    if((httpcode!=0)&&(sd_write_len==mp4_file_size))
    {
		FileExtRename(w_shandle,"mp4");	
	    FileClose(w_shandle);
    }
	else
	{
		mp4_header_flag = FALSE;
		get_range_data = FALSE;
		get_range_check = FALSE;
		FileClose(w_shandle);
	}

	mpDebugPrint("sd_write_len %d",sd_write_len);

#endif	
    mpDebugPrint("youtube_get_video_location: http error resp code=%d", httpcode);

    if (httpcode == 200 || httpcode == 206)
    {

		mpDebugPrint("youtube_get_video_location: got total data len = %d", len);
		strcpy(youtube->youtube_location,youtube_url);
		httpcode = 303;

    }else if (httpcode == 303||httpcode == 302)
    {
				   memset(youtube->youtube_location,0x00,1024);
		mpDebugPrint("App_State.XML_BUF->BUFF = %d", strlen(App_State.XML_BUF->BUFF));
		mmcp_memcpy(youtube->youtube_location,&App_State.XML_BUF->BUFF[10],strlen(App_State.XML_BUF->BUFF)-10);
		httpcode = 303;
    }
    else if (httpcode > 0)                  /* TODO */
    {
        mpDebugPrint("youtube_get_video_location: http error resp code=%d", httpcode);
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
	if( youtube_url )
		ext_mem_free(youtube_url);
	if( tmpbuf )
		ext_mem_free(tmpbuf);
   	return httpcode;
}
int youtube_get_data(BYTE *gbuf,DWORD start,DWORD END)
{
   int video_cp_size;
   int mv_data_size = 0;
   int mv_qdata_size = 0;
   DWORD dwEvent;
   //mpDebugPrint("youtube_get_data idx %d total %d",youtube_video.youtube_video_start,youtube_video.youtube_video_total);
   //The video want to get size
   video_cp_size = (END - start)+1;

   if (video_cp_size == 0)
     goto get_end;

   if(youtube_video.youtube_video_error||(App_State.dwState&NET_LINKUP)!=NET_LINKUP)
   	{
	   	mv_data_size = -1;
		mpDebugPrint("Net work LINK DOWN");
		youtube_video.youtube_video_error = 1;
		SockIdSignalTcpFinRecvd(iradio_conn);
		goto get_end;
   	}
   if(((youtube_video.youtube_video_total < YOUTUBE_BUFFER_LOW)&&(!youtube_video.youtube_video_data_end))||youtube_video.youtube_video_waiting)
   {
#if P2P_TEST
        if((youtube_video.youtube_video_total > (YOUTUBE_BUFFER_HIGH/2))||youtube_video.youtube_video_data_end)
#else
        if((youtube_video.youtube_video_total > YOUTUBE_BUFFER_HIGH)||youtube_video.youtube_video_data_end)
#endif
        {
          youtube_video.youtube_video_waiting = 0;
        }
		else
   {
			youtube_video.youtube_video_waiting = 1;
   		mv_data_size = 0;
   		goto get_end;
   }
   }

   if(youtube_video.youtube_video_total > video_cp_size)
   {
   	mv_data_size =  video_cp_size;
   }
   else
   	{
   	 mv_data_size = youtube_video.youtube_video_total;
	 //Clear move video data size
	 //youtube_video_total = 0;
   	}

   //mpDebugPrint("youtube_video_start %d ",youtube_video_start);

   //mpDebugPrint("mv_data_size %d ",mv_data_size);
   //Girbuf ring end point
   if(bufindex_end!=0)
   {
   	   if((youtube_video.youtube_video_start+mv_data_size)> bufindex_end)
       {
         mv_qdata_size = (youtube_video.youtube_video_start+mv_data_size)- bufindex_end;

		 //mpDebugPrint("mv_qdata_size %d",mv_qdata_size);
		 //mv_data_size-=mv_qdata_size;

		 mmcp_memcpy(gbuf,girbuf+youtube_video.youtube_video_start,mv_data_size-mv_qdata_size);

		 //Clear video get ring buf index
		 youtube_video.youtube_video_start = 0;
		 mmcp_memcpy(gbuf+(mv_data_size-mv_qdata_size),girbuf+youtube_video.youtube_video_start,mv_qdata_size);
		 youtube_video.youtube_video_start += mv_qdata_size;
		 bufindex_end = 0;
		 }
		 else
		 {
		mmcp_memcpy(gbuf,girbuf+youtube_video.youtube_video_start,mv_data_size);
		   youtube_video.youtube_video_start+=mv_data_size;
		 }
	   }
   else
   	{
   //memcpy(gbuf,girbuf+youtube_video_start,mv_data_size);
      //mpDebugPrint("1 youtube_video.youtube_video_start %d",youtube_video.youtube_video_start);
	   
	  if((youtube_video.youtube_video_start+mv_data_size)> NETSTREAM_MAX_BUFSIZE)
	  		youtube_video.youtube_video_start=0;
	  
   mmcp_memcpy(gbuf,girbuf+youtube_video.youtube_video_start,mv_data_size);
   youtube_video.youtube_video_start+=mv_data_size;
   	}

   youtube_video.youtube_video_total -= mv_data_size;

   EventSet(NETWORK_STREAM_EVENT, network_receive);

get_end:

   if (mv_data_size == 0)
	 EventWait(NETWORK_STREAM_EVENT, video_ready_event, OS_EVENT_OR, &dwEvent);
   else
       TaskYield();
   return mv_data_size;
}
#if	DM9KS_ETHERNET_ENABLE||DM9621_ETHERNET_ENABLE
int youtube_p2p_video_download(DWORD dwIndex,BYTE hq)
{
    int ret = 0;
	DWORD check_len = 0;
	int send_len = 0;
	DWORD get_len = 0;
	BYTE denominator = 1;
	BYTE send_time = 0;
#if P2P_TEST
	DWORD addr = 0xc0a80164;//192.168.1.100
#else
#if MAKE_XPG_PLAYER
	DWORD addr = GetNetConfigTarget();
#else
	DWORD addr = 0xc0a80164;//192.168.1.100
#endif
#endif
	U16 port = 80;
	int num_bytes_recv,recv_len;
	BYTE video_request[8];
	DWORD video_len = 0,video_total=0;
	BYTE video_play=0,send_flag = 0,check_flag = 0;

	mpDebugPrint("P2P TEST youtube_video_download!!");
	
//	memset(girbuf,0x00,NETSTREAM_MAX_BUFSIZE+8192);
	youtube_video.youtube_video_total = 0;
	ret = mpx_DoConnect(addr, port, TRUE);
	if( ret > 0 )
	{
		iradio_conn = ret;
	}
	else
	{
		mpDebugPrint("P2P TEST youtube_video_download connect fail!!");
		return -1;
	}
	youtube_video.youtube_video_start = 0;
	bufindex = 0;
	bufindex_end = 0;
	youtube_video.youtube_video_data_end = 0;
	youtube_video.youtube_video_error = 0;
	youtube_set_video_stop(0);
	do{
		num_bytes_recv = recv( iradio_conn, video_request, 8,0);
		TaskYield();
	}while(num_bytes_recv < 0);
	video_len = video_request[0]<<24|video_request[1]<<16|video_request[2]<<8|video_request[3];
	NetPacketDump(video_request, 8);
	mpDebugPrint("video_len %d",video_len);
    youtube_video.youtube_video_size = video_len;
	//youtube_video.youtube_video_ext = 1;//mp4
	youtube_video.youtube_video_ext = video_request[4];//264

	while(1)
	{
		ST_SOCK_SET stReadSet;

        MPX_FD_ZERO(&stReadSet);
        MPX_FD_SET(iradio_conn, &stReadSet);

        ret = select(0, &stReadSet, NULL, NULL, NULL);

		if (ret == 0)
		{
			continue;
		}
		else if (ret < 0)
		{
			continue;
		}

		do
		{
		  while (youtube_video.youtube_video_total > YOUTUBE_BUFFER_HIGH2)
		  {
			DWORD dwEvent;
			EventWait(NETWORK_STREAM_EVENT, network_receive, OS_EVENT_OR, &dwEvent);
		  }

	      if(video_play)
             num_bytes_recv = MIN(8192,get_len);
		  else
			  num_bytes_recv = 8192;
		  num_bytes_recv = recv( iradio_conn, &girbuf[bufindex], num_bytes_recv,0);
		  //mpDebugPrint("num_bytes_recv %d",num_bytes_recv);
		  if( num_bytes_recv > 0 )
		  {
				bufindex+=num_bytes_recv;
				youtube_video.youtube_video_total+=num_bytes_recv;
				video_total+=num_bytes_recv;
				get_len-=num_bytes_recv;

				if(check_flag)
					check_len+=num_bytes_recv;

			    if(!video_play)
			    {
			  if((youtube_video.youtube_video_total > YOUTUBE_BUFFER_HIGH)||(youtube_video.youtube_video_total>=youtube_video.youtube_video_size))
		            {
		               mpDebugPrint("Turn on video play %d",NETWORK_STREAM_EVENT);
#if MAKE_XPG_PLAYER
                       EventSet(NETWORK_STREAM_EVENT, video_ready_event);
					   xpgUpdateYouTubeLoading(0);
					   get_len = YOUTUBE_BUFFER_MAX;
#endif

					   video_play = 1;
		            }
			    }
				if(bufindex >= NETSTREAM_MAX_BUFSIZE)
				{
					mpDebugPrint("Set index %d to 0!!",bufindex);
					bufindex_end = bufindex;
					bufindex = 0;
					check_flag = 1;
				}
				if(send_flag)
					send_flag = 0;
				//mpDebugPrint("bufindex %d",bufindex);
					
				if(video_total >= video_len)
				{
				  //mpDebugPrint("End bufindex %d",bufindex);
				  youtube_video.youtube_video_data_end = 1; 
				  closesocket(iradio_conn);
				  break;
				}
		  }
		} while (num_bytes_recv);
	 
		if( num_bytes_recv == 0 )               /* socket closed by peer */
		{
			youtube_video.youtube_video_data_end = 1; 
			break;
        }
		  
			if(check_flag)
			{
			//mpDebugPrint("(bufindex%YOUTUBE_BUFFER_HIGH) %d",bufindex%YOUTUBE_BUFFER_HIGH);

			if(!(check_len%(YOUTUBE_BUFFER_HIGH/denominator)))
				{
				if(youtube_video.youtube_video_total <= YOUTUBE_BUFFER_HIGH)
					{
						//mpDebugPrint("youtube_video.youtube_video_total %d",youtube_video.youtube_video_total);

					    if(send_flag==0)
					    {
					        send_flag = 1;
							if(youtube_video.youtube_video_data_end==0)
							{
							 mpDebugPrint("send %d %d",youtube_video.youtube_video_total,video_total);
							 do{
							 	send_len = send(iradio_conn,video_request,4,0);
								TaskYield();
							 }while(send_len <=0);
							 get_len = YOUTUBE_BUFFER_MAX;
							 if(send_time)
							 {
								 if(denominator <= 32)
								 {
									 denominator*=2;
								 }
							 }
							 send_time=1;

							 //mpDebugPrint("denominator %d",denominator);


							}
					    }
					}
				}
			}

	  
			TaskYield();

	}

	if (iradio_conn > 0)
	{
	  closesocket(iradio_conn);
	  iradio_conn = 0;
	}


}
#endif
#if HAVE_HTTP_SERVER
int youtube_get_http_server_file(void)
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
	BYTE *youtube_url = "http://211.154.137.38/download/VIDEO.MP4?loginName=admin&password=\
4297f44b13955235245b2497399d7a93&clientCode=clientno&clientPass=123123";
	BYTE *youtube_id;
	short pr = FALSE;
	U08* extptr;
	u8 *tmpbuf;
	
    mpDebugPrint("%s",__func__);
	//youtube_url = (BYTE *) ext_mem_malloc(1024);
	tmpbuf = (u8 *) ext_mem_malloc(8*1024);
	//memset(youtube_url,0x00,1024);
    Xml_BUFF_init(NET_RECVHTTP);
	
	BYTE get_range[256];
	BYTE start_range[32];
	BYTE tmp_range[4];
	BYTE bMcardId = SD_MMC;
	DWORD filesize = 0,value32 = 0;
	BYTE cp=0,shiftbyte=0,shiftbytecp=0;
	int j;
	
	memset(start_range,0x00,32);

	w_sDrv = DriveChange(bMcardId);
	if (DirReset(w_sDrv) != FS_SUCCEED)
		return;

	w_sDrv = DriveGet(bMcardId);
  	w_shandle = FileSearch(w_sDrv, "video","tmp", E_FILE_TYPE);

	if(w_shandle == NULL)
	{
		w_sDrv = DriveGet(bMcardId);
		w_shandle = FileOpen(w_sDrv);
		filesize = FileSizeGet(w_shandle);
		
		if(!sd_write_len)
			sd_write_len+=filesize;
		
		Seek(w_shandle,filesize);	
		mp4_header_flag = TRUE;
		get_range_flag = TRUE;
        for(i=0;i<5;i++)
        {
           switch(i)
           {
             case 4:
			 	value32 = filesize;
			 	break;
			 case 3:
			 	value32 = filesize/100;
			 	break;
             case 2:
			 	value32 = filesize/10000;
			 	break;
             case 1:
			 	value32 = filesize/1000000;
			 	break;
             case 0:
			 	value32 = filesize/100000000;
			 	break;

           }
          DecString(tmp_range,value32,2,0);
		  if((tmp_range[0]!=0x30)&&(!cp))
		  {
		  	cp=1;
			j=0;
		  }
		  else if((tmp_range[1]!=0x30)&&(!cp))
		  {
		  	memcpy(start_range,&tmp_range[1],1);
			shiftbyte++;
			shiftbytecp = 1;
			cp =1;
			j=0;

		  }
		  
		  if(cp)
		  {
		    if(shiftbytecp)
			  shiftbytecp = 0;
			else
			{
		      memcpy(start_range+(j*2)+shiftbyte,tmp_range,2);
			  j++;
			}
			
		  }
        
        }
		NetPacketDump(start_range,10);
			
	    snprintf(get_range, 256,"%s-4294967295",start_range);
	    mpDebugPrint("get_range %s",get_range);


	}
	else
		mpDebugPrint("FileSearch VIDEO.tmp file fail!!");

	//mpDebugPrint("youtube_get_video_location referer %s\n",youtube->referer);
    curl = curl_easy_init();
    if(curl) {
        data = youtube_malloc(512);
        if (data == NULL)
            goto exit;
		
	mpDebugPrint("youtube_url %s",youtube_url);
	snprintf(data, 512,"x-flash-version:   10,0,22,87");
	data[511] = '\0';
    header = curl_slist_append(header, data);
	snprintf(data, 512,"Accept: text/html, */*");
    data[511] = '\0';
    header = curl_slist_append(header, data);
    //mpDebugPrint("youtube_get_video_location: extra header=%d", strlen(data));
    curl_easy_setopt(curl, CURLOPT_URL, youtube_url);
	curl_easy_setopt(curl, CURLOPT_ENCODING,"gzip, deflate");			 // Accept-Encoding
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, App_State.XML_BUF);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sd_write_func);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
	if(start_range[0]==0x00)
		curl_easy_setopt(curl, CURLOPT_RANGE, "0-4294967295");	//256*1024 "0-262143" "0-8192"
	else
		curl_easy_setopt(curl, CURLOPT_RANGE, get_range);	//256*1024 "0-262143" "0-8192"
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, tmpbuf);
	mpDebugPrint("call curl_easy_perform %p %p" ,tmpbuf,App_State.XML_BUF);
	//set follow location
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    res = curl_easy_perform(curl);
    TaskYield();
	
    /* check return code of curl_easy_perform() */
    if (res != CURLE_OK)
    {
        mpDebugPrint("youtube_get_http_server_file: curl_easy_perform failed, res = %d", res);
        httpcode = 0;
        goto Youtube_cleanup_1;
    }

    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
        httpcode = 0;

Youtube_cleanup_1:
    /* always cleanup */
    curl_easy_cleanup(curl);

    curl_slist_free_all(header);

    youtube_mfree(data);
    if((httpcode!=0)&&(sd_write_len==mp4_file_size))
    {
		FileExtRename(w_shandle,"mp4");	
	    FileClose(w_shandle);
    }
	else
	{
		mp4_header_flag = FALSE;
		get_range_data = FALSE;
		get_range_check = FALSE;
		FileClose(w_shandle);
	}
	
	mpDebugPrint("sd_write_len %d",sd_write_len);

    mpDebugPrint("youtube_get_http_server_file: http error resp code=%d", httpcode);

    if (httpcode == 200 || httpcode == 206)
    {

		mpDebugPrint("youtube_get_http_server_file: got total data len = %d", len);
		httpcode = 303;

    }else if (httpcode == 303||httpcode == 302)
    {
		mpDebugPrint("App_State.XML_BUF->BUFF = %d", strlen(App_State.XML_BUF->BUFF));
		httpcode = 303;
    }
    else if (httpcode > 0)                  /* TODO */
    {
        mpDebugPrint("youtube_get_http_server_file: http error resp code=%d", httpcode);
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
	if( tmpbuf )
		ext_mem_free(tmpbuf);
   	return httpcode;
}
#endif
#if VIDEO_WRITE_SD
int youtube_video_download(DWORD dwIndex,BYTE hq)
{
   return 2;
}

#else
int youtube_video_download(DWORD dwIndex,BYTE hq)
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
		youtube_info_t *youtube = &youtube_info;

		//mpDebugPrint("youtube_video_download %d",dwIndex);
	YOUTUBE_SEE_OTHER:
		memset(girbuf,0x00,NETSTREAM_MAX_BUFSIZE+8192);
		memset(request,0x00,1024);
		youtube_video.youtube_video_total = 0;
		//check location id
		memset(request,0x00,1024);

		website =(BYTE *) Net_GetWebSite(youtube->youtube_location);
		port = Net_GetWebSitePort(youtube->youtube_location);
		//mpDebugPrint("youtube_location %s",youtube_location);

		urlparm = strstr(youtube->youtube_location+strlen(website),"/");
		//mpDebugPrint("website %s",website);
		//mpDebugPrint("port %d",port);
		//mpDebugPrint("urlparm %s",urlparm);

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
			iradio_conn = ret;
		}
		else
		{
			mpDebugPrint("iRadio::Internet Radio connect fail, retry");
			closesocket(iradio_conn);
			return -1;
		}
		snprintf(request, 1024,
					"GET %s HTTP/1.1\r\n"
					"Accept: */*\r\n"
					"x-flash-version: 10,0,45,2\r\n"
					"Accept-Language:zh-tw,en-us;q=0.7,en;q=0.3\r\n"
					"User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.1)\r\n"
					"Accept-Charset:UTF-8,*\r\n"
					"Keep-Alive:115\r\n"
					"Connection: Keep-Alive\r\n"
					"Host: %s\r\n"
					"\r\n",
					urlparm,website);
		mpDebugPrint("InterRadioStart ret %x\n",ret);
		MPX_FD_ZERO(&stReadSet);
		MPX_FD_ZERO(&stWriteSet);
		wfds = rfds = NULL;
		rfds = &stReadSet;
		MPX_FD_SET(iradio_conn, &stReadSet);
		send( iradio_conn, request, strlen(request) , 0);
		mpDebugPrint("send request\n");
		idx = 0;
		retrycount = 0;
		mpDebugPrint("send request AA\n");
		num_bytes_recv = 0;
		num_bytes_recv = recv( iradio_conn,girbuf,1024,0);
		recv_len = num_bytes_recv;
		len = 0;
		idx = 0;
		pr = FALSE;
		//NetPacketDump(girbuf,num_bytes_recv);
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
				    memset(youtube->youtube_location,0x00,1024);
					//check location

					for(i=0;i<strlen(request);i++)
					{
					  if(request[i]==0x0D&&request[i+1]==0x0A&&
						request[i+2]==0x0D&&request[i+2]==0x0A)
					  {
						break;
					  }
					}
					mmcp_memcpy(youtube->youtube_location,request+10,i);
					mpDebugPrint("%s",youtube->youtube_location);
					mpDebugPrint("%s",youtube->youtube_location+256);
					closesocket(iradio_conn);
					goto YOUTUBE_SEE_OTHER;
				 }
				 else if(!strncasecmp(request,"Content-Type: ",14))
				 {
					extptr = strstr(request,"/");

				    //mpDebugPrint("1 extptr %s", extptr);
					if(!strncasecmp(extptr,"/mp4",4))
					{
						youtube_video.youtube_video_ext = 1;
					}
					else
						youtube_video.youtube_video_ext = 0;
				 }
				 else if(!strncasecmp(request,"Content-Length: ",16))
				 {
					 //mpDebugPrint("1 %s", buf);
					 //NetPacketDump(buf,strlen(buf));
					 youtube_video.youtube_video_size = youtubestrtoint(request+16);
					 //mpDebugPrint("youtube_video_size %d ", youtube_video.youtube_video_size);
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
		bufindex = youtube_first_packet(girbuf,num_bytes_recv);
		youtube_video.youtube_video_start = bufindex;
		mpDebugPrint("youtube_video_start %d bufindex %d",youtube_video.youtube_video_start,bufindex);
		youtube_video.youtube_video_total+=num_bytes_recv-bufindex;
		video_index = num_bytes_recv-bufindex;

		//bufindex = 0;
		bufindex = num_bytes_recv;
		//Turn on video play
		//EventSet(NETWORK_STREAM_EVENT, 0x3);
		bufindex_end = 0;
		youtube_video.youtube_video_data_end = 0;
		youtube_video.youtube_video_error = 0;
		youtube_set_video_stop(0);
		while( 1 )
		{

		    if(youtube_video.youtube_video_data_end)
				goto wait_video_get_end;
		    //Check wifi recive packet too fast
		    if(youtube_video.youtube_video_total < YOUTUBE_BUFFER_MAX)
		    	{
				    //if((NETSTREAM_MAX_BUFSIZE-bufindex) >1024)
					//	num_bytes_recv = MIN(8192, (NETSTREAM_MAX_BUFSIZE-bufindex));
					//else
					num_bytes_recv = 8192;
			num_bytes_recv = recv( iradio_conn, &girbuf[bufindex], num_bytes_recv,0);


			//mpDebugPrint("num_bytes_recv %d",video_index);
			if( num_bytes_recv > 0 )
			{
				//mpDebugPrint("###num_read %d %x %x\n",num_bytes_recv,gtotoalbuf,gtotaldata);
				failcount = 0;

                bufindex+=num_bytes_recv;
				video_index+=num_bytes_recv;

				youtube_video.youtube_video_total+=num_bytes_recv;
				//NetGetFile(bufindex,girbuf,0,0);
				if((youtube_video.youtube_video_total > YOUTUBE_BUFFER_HIGH)&&(!video_play)||(youtube_video.youtube_video_total>=youtube_video.youtube_video_size))
                {
                   mpDebugPrint("Turn on video play %d",NETWORK_STREAM_EVENT);

                   EventSet(NETWORK_STREAM_EVENT, video_ready_event);
				   //NetPacketDump(girbuf,youtube_video_start+128);
				   video_play = 1;
#if MAKE_XPG_PLAYER
				   xpgUpdateYouTubeLoading(0);
#endif

                }else if(!video_play)
                {
                   if(youtube_video_show_load >10)
                   {
#if MAKE_XPG_PLAYER
                		xpgUpdateYouTubeLoading(1);
#endif
						youtube_video_show_load=0;
                   }
				   else
				   	youtube_video_show_load++;

                }

				if(bufindex >= NETSTREAM_MAX_BUFSIZE)
				{

				    mpDebugPrint("Set index %d to 0!!",bufindex);
					//NetPacketDump(girbuf+(bufindex-num_bytes_recv),num_bytes_recv);
					bufindex_end = bufindex;
					bufindex = 0;
				}

				if(video_index >= youtube_video.youtube_video_size)
				{
					youtube_video.youtube_video_data_end = 1;
				}

			}
			else
			{
				TaskYield();
				failcount ++;
				mpDebugPrint("failcount %x",failcount);
				breconect = 1;
				if(video_play)
					youtube_video.youtube_video_error = 2;
				else
				youtube_video.youtube_video_error = 1;

						break;
					}
			}
wait_video_get_end:
			if((bufindex == youtube_video.youtube_video_start)||youtube_video.youtube_video_stop)
				break;
			TaskYield();
		}

		closesocket(iradio_conn);
		return youtube_video.youtube_video_error;
}
#endif
int youtube_get_url_len(BYTE *url)
{
  int i;
  for(i=0;i<strlen(url);i++)
  {
    if(url[i]=='&')
    {
		break;
    }
  }
  return i;
}

/**
 *  Get photo set of specified user id
 */
static void video_list_content_handler(void *user_data, const char *s, int len)
{
    youtube_info_t   *youtube_info    = (youtube_info_t *) user_data;
    char    *title;
    char    *id;
    int     i;

	//mpDebugPrint("content_handler %s\n",s);
	if(youtube_video_show_load >10)
	{
#if MAKE_XPG_PLAYER
		 xpgUpdateYouTubeLoading(1);
#endif
		 youtube_video_show_load=0;
	}
	else
	 youtube_video_show_load++;

    if (youtube_info->state == YOUTUBE_VIDEO_LIST_TITLE && youtube_info->cur_video)
    {
        title = youtube_info->cur_video->title;
        len = (len >= MAX_TITLE)? (MAX_TITLE-1) :len;
        mmcp_memcpy(title, s,len);
        title[len] = '\0';
        //mpDebugPrint("\nYOUTUBE_VIDEO_LIST_TITLE: %s\n", title);
    }
    else if (youtube_info->state == YOUTUBE_VIDEO_LIST_ID && youtube_info->cur_video)
    {
        id = youtube_info->cur_video->id;
        len = (len >= MAX_ALBUMID)? (MAX_ALBUMID-1) :len;
        mmcp_memcpy(id, s,len);
        id[len] = '\0';
        //mpDebugPrint("YOUTUBE_VIDEO_LIST_ID: %s\n", id);
    }
    else if (youtube_info->state == YOUTUBE_VIDEO_LIST_CREDIT)
    {
	   //memset(youtube_info->cur_video->credit,0x00,MAX_USERNAME);
	   //memcpy(youtube_info->cur_video->credit,s,len);
       //mpDebugPrint("YOUTUBE_VIDEO_LIST_CREDIT: %s\n",youtube_info->cur_video->credit);
   	}
    else if (youtube_info->state == YOUTUBE_VIDEO_LIST_DESCRIPTION)
    {
       //mpDebugPrint("YOUTUBE_VIDEO_LIST_DESCRIPTION %s",s);
      #if 0
       if(youtube_description_idx==0)
       {
	   	memset(youtube_info->cur_video->description,0x00,MAX_TEXT);
       }
	   	memcpy(youtube_info->cur_video->description+youtube_description_idx,s,len);
	    youtube_description_idx+=len;
		#endif
   	}
    else if (youtube_info->state == YOUTUBE_VIDEO_LIST_KEYWORDS)
    {
	   //memset(youtube_info->cur_video->keywords,0x00,MAX_TEXT);
	   //memcpy(youtube_info->cur_video->keywords,s,len);
       //mpDebugPrint("YOUTUBE_VIDEO_LIST_KEYWORDS: %s\n",youtube_info->cur_video->keywords);
   	}
    else if (youtube_info->state == YOUTUBE_VIDEO_LIST_UPLOADED)
    {
	   //mpDebugPrint("YOUTUBE_VIDEO_LIST_UPDATED len %d\n",len);
	   //memset(youtube_info->cur_video->uploaded,0x00,MAX_TITLE);
	   //memcpy(youtube_info->cur_video->uploaded,s,len);
       //mpDebugPrint("YOUTUBE_VIDEO_LIST_UPLOADED: %s\n",youtube_info->cur_video->uploaded);
    }
}

static void video_list_start_element_handler(void *user_data, const char *name, const char **attr)
{
    youtube_info_t   *youtube_info    = (youtube_info_t *) user_data;
	int url_len =0;
	BYTE cp=0;
	//mpDebugPrint("youtube_info->state %x",youtube_info->state);
	//mpDebugPrint("start_element_handler %s \n",name);
	switch(youtube_info->state)
    {
	  case YOUTUBE_VIDEO_LIST_INIT:

        if (!strcasecmp(name, "entry"))
        {
            /* append album list */

			//mpDebugPrint("entry\n");
            video_entry_t       *video_entry;

            youtube_info->state = YOUTUBE_VIDEO_LIST_FOUND;
            video_entry = (video_entry_t *) youtube_malloc(sizeof(video_entry_t));
            MP_ASSERT(video_entry);
            memset(video_entry, 0, sizeof(video_entry_t));

            youtube_info->cur_video->next = video_entry;
            youtube_info->cur_video = video_entry;

    }
	  	break;
	  case YOUTUBE_VIDEO_LIST_FOUND:

        if (!strcasecmp(name, "title"))
        {
			youtube_info->state = YOUTUBE_VIDEO_LIST_TITLE;
        }
		else if(!strcasecmp(name, "link"))
		{

			while (*attr)
			{

				//mpDebugPrint(" %s \n",*attr);
				if(!strcasecmp(*attr, "alternate"))
					cp = 1;

				if((!strcasecmp(*attr, "href"))&&cp)
				{
				    memset(youtube_info->cur_video->url,0x00,MAX_URL);
			    	attr ++;
					url_len = youtube_get_url_len(*attr);
					mmcp_memcpy(youtube_info->cur_video->url,*attr,url_len);
					cp=0;
					//mpDebugPrint("cp url %s \n",youtube_info->cur_video->url);
				}

			    attr ++;
			}
		}
		else if(!strcasecmp(name, "yt:videoid"))
		{
		  youtube_info->state = YOUTUBE_VIDEO_LIST_ID;
		}
		else if(!strcasecmp(name, "media:content"))
		{

			while (*attr)
    {

				//mpDebugPrint(" %s \n",*attr);
				if(!strcasecmp(*attr, "url"))
        {
				  attr ++;
				  //mpDebugPrint(" %s \n",*attr);
				}

				attr ++;
			}
		}
        else if (!strcasecmp(name, "media:credit"))
        {
			youtube_info->state = YOUTUBE_VIDEO_LIST_CREDIT;
        }
        else if (!strcasecmp(name, "media:description"))
        {
			youtube_info->state = YOUTUBE_VIDEO_LIST_DESCRIPTION;
			//youtube_description_idx = 0;
        }
        else if (!strcasecmp(name, "media:keywords"))
        {
			youtube_info->state = YOUTUBE_VIDEO_LIST_KEYWORDS;
        }
        else if (!strcasecmp(name, "media:thumbnail"))
        {

		    //mpDebugPrint(" media:thumbnail \n");
			while (*attr)
			{
				//mpDebugPrint(" %s \n",*attr);
				if(!strcasecmp(*attr, "url"))
				{
				  attr ++;
				  strcpy(youtube_info->cur_video->thumbnail,*attr);
				  break;
				}
				attr ++;
    }
}
        else if (!strcasecmp(name, "yt:duration"))
    {

			while (*attr)
			{
				//mpDebugPrint(" %s \n",*attr);
				if(!strcasecmp(*attr, "seconds"))
				{
					attr ++;
					strcpy(youtube_info->cur_video->duration,*attr);
					//mpDebugPrint("seconds %s \n",youtube_info->cur_video->duration);
				}

				attr ++;
			}
    }
        else if (!strcasecmp(name, "yt:uploaded"))
        {
			youtube_info->state = YOUTUBE_VIDEO_LIST_UPLOADED;
        }
		break;
}

}

static void video_list_end_element_handler(void *user_data, const char *name)
{
    youtube_info_t  *youtube_info     = (youtube_info_t *) user_data;

    if (youtube_info->state == YOUTUBE_VIDEO_LIST_TITLE)
    {
        if (!strcasecmp(name, "title"))
            youtube_info->state = YOUTUBE_VIDEO_LIST_FOUND;

    }
    else if (youtube_info->state == YOUTUBE_VIDEO_LIST_ID)
    {
        if (!strcasecmp(name, "yt:videoid"))
            youtube_info->state = YOUTUBE_VIDEO_LIST_INIT;
    }
	else if((youtube_info->state == YOUTUBE_VIDEO_LIST_CREDIT)||
		(youtube_info->state == YOUTUBE_VIDEO_LIST_DESCRIPTION)||
		(youtube_info->state == YOUTUBE_VIDEO_LIST_KEYWORDS)||
		(youtube_info->state == YOUTUBE_VIDEO_LIST_UPLOADED))
    {
	  if(youtube_info->state == YOUTUBE_VIDEO_LIST_DESCRIPTION)
    {
	  	mpDebugPrint("YOUTUBE_VIDEO_LIST_DESCRIPTION:\n");
		//NetAsciiDump(youtube_info->cur_video->description,strlen(youtube_info->cur_video->description));
	  }

	  youtube_info->state = YOUTUBE_VIDEO_LIST_FOUND;
    }
}

int youtube_fetch_video_list(youtube_info_t *youtube_info,BYTE *username,BYTE youtube_mode)
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
	char paser_use_http = 0;
	char *youtube_p;
	char *youtube_m;

    Xml_BUFF_init(NET_RECVHTTP);
	mpDebugPrint("NETFS_YOUTUBE: youtube_fetch_video_list %d",youtube_video.youtube_video_page);
	if(!strncmp(username,"MPXHOMEPAGE",11))
	{
	    //for video
	    switch(youtube_mode)
	    {
			case 0:
				youtube_m="pop";
				break;
			case 1:
				youtube_m= "mp";
				break;
			degault:
				youtube_m = "pop";
				break;
	    }
	    switch(youtube_video.youtube_video_page)
	    {
			case 1:
				youtube_p = "1";
              break;
			case 2:

				youtube_p = "2";
				break;
			case 3:

				youtube_p = "3";
				break;
			case 4:

				youtube_p = "4";
				break;
			case 5:

				youtube_p = "5";
				break;
			default:

				youtube_p = "1";
				break;

	    }
	    switch(youtube_video.youtube_video_cate)
	    	{

			   case 0:
	    		//snprintf(youtube_api_url, MAX_URL,
	            // "http://www.youtube.com/videos?s=%s&gl=US&hl=en&p=%s",youtube_m,youtube_p);
	    		snprintf(youtube_api_url, MAX_URL,
	             "http://www.youtube.com/charts/videos_views?t=t&p=%s&gl=US&hl=en",youtube_p);

				break ;

				case 1:
				 snprintf(youtube_api_url, MAX_URL,
				  //"http://www.youtube.com/videos?s=pop&gl=US&hl=en&p=%s",youtube_page);
				  "http://www.youtube.com/videos?s=%s&gl=US&hl=en&c=2&p=%s",youtube_m,youtube_p);
				 break ;
				case 2:
	    snprintf(youtube_api_url, MAX_URL,
				  //"http://www.youtube.com/videos?s=pop&gl=US&hl=en&p=%s",youtube_page);
				  "http://www.youtube.com/comedy?s=%s&gl=US&hl=en&p=%s",youtube_m,youtube_p);
				 break ;

				case 3:
					snprintf(youtube_api_url, MAX_URL,
					"http://www.youtube.com/education?gl=US&hl=en&p=%s",youtube_p);
					break ;

				case 4:
					snprintf(youtube_api_url, MAX_URL,
					"http://www.youtube.com/videos?s=%s&gl=US&hl=en&c=24&p=%s",youtube_m,youtube_p);
					break ;

				case 5:
					snprintf(youtube_api_url, MAX_URL,
					"http://www.youtube.com/videos?s=%s&gl=US&hl=en&c=1&p=%s",youtube_m,youtube_p);
					break ;

				case 6:
					snprintf(youtube_api_url, MAX_URL,
					"http://www.youtube.com/gaming?s=%s&gl=US&hl=en&p=%s",youtube_m,youtube_p);
					break ;
				case 7:
					snprintf(youtube_api_url, MAX_URL,
					"http://www.youtube.com/videos?s=%s&gl=US&hl=en&c=26&p=%s",youtube_m,youtube_p);
					break ;
				case 8:
					snprintf(youtube_api_url, MAX_URL,
					"http://www.youtube.com/music?s=%s&gl=US&hl=en&p=%s",youtube_m,youtube_p);
					break ;
				case 9:
					snprintf(youtube_api_url, MAX_URL,
					"http://www.youtube.com/news?s=%s&gl=US&hl=en&p=%s",youtube_m,youtube_p);
					break ;

				case 10:
					snprintf(youtube_api_url, MAX_URL,
					//"http://www.youtube.com/videos?s=pop&gl=US&hl=en&p=%s",youtube_page);
					"http://www.youtube.com/videos?s=%s&gl=US&hl=en&c=29&p=%s",youtube_m,youtube_p);
					break ;
				case 11:
					snprintf(youtube_api_url, MAX_URL,
					//"http://www.youtube.com/videos?s=pop&gl=US&hl=en&p=%s",youtube_page);
					"http://www.youtube.com/videos?s=%s&gl=US&hl=en&c=22&p=%s",youtube_m,youtube_p);
					break ;
				case 12:
					snprintf(youtube_api_url, MAX_URL,
					//"http://www.youtube.com/videos?s=pop&gl=US&hl=en&p=%s",youtube_page);
					"http://www.youtube.com/videos?s=%s&gl=US&hl=en&c=15&p=%s",youtube_m,youtube_p);
					break ;
				case 13:
					snprintf(youtube_api_url, MAX_URL,
					//"http://www.youtube.com/videos?s=pop&gl=US&hl=en&p=%s",youtube_page);
					"http://www.youtube.com/videos?s=%s&gl=US&hl=en&c=28&p=%s",youtube_m,youtube_p);
					break ;

				case 14:
					snprintf(youtube_api_url, MAX_URL,
					//"http://www.youtube.com/videos?s=pop&gl=US&hl=en&p=%s",youtube_page);
					"http://www.youtube.com/sports?s=%s&gl=US&hl=en&p=%s",youtube_m,youtube_p);
					break ;
				case 15:
					snprintf(youtube_api_url, MAX_URL,
					//"http://www.youtube.com/videos?s=pop&gl=US&hl=en&p=%s",youtube_page);
					"http://www.youtube.com/videos?s=%s&gl=US&hl=en&c=19&p=%s",youtube_m,youtube_p);
					break ;
	    	}
		//for movies
	    //snprintf(youtube_api_url, MAX_URL,
	    //         "http://www.youtube.com/movies?s=mvp&b=0");

		paser_use_http = 1;
	}
	else
	{
    snprintf(youtube_api_url, MAX_URL,
             "http://gdata.youtube.com/feeds/api/users/%s/favorites?v=2",
             username);
	}
	mpDebugPrint("youtube_api_url %s",youtube_api_url);
    curl = curl_easy_init();
    if(curl) {
        data = youtube_malloc(512);
        if (data == NULL)
            goto exit;
        snprintf(data, 512, "Authorization: GoogleLogin auth=%s", youtube_info->auth);
        data[511] = '\0';
        header = curl_slist_append(header, data);
        mpDebugPrint("YOUTUBE: extra header=%d", strlen(data));
        curl_easy_setopt(curl, CURLOPT_URL, youtube_api_url);
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

        youtube_mfree(data);

        mpDebugPrint("youtube_fetch_video_list: http error resp code=%d", httpcode);
        if (httpcode == 200)
        {
            if(paser_use_http)
        	{

				bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
				mpDebugPrint("youtube_fetch_video_list: got total data len = %d", len);
				youtube_info->state = YOUTUBE_VIDEO_LIST_INIT;
				html_init(&youtube_html_parser, youtube_info);
				html_set_content_handler(&youtube_html_parser, youtube_html_content_handler);
				html_set_tag_start(&youtube_html_parser, youtube_html_tag_start_handler);
				html_set_tag_end(&youtube_html_parser, youtube_html_tag_end_handler);
				html_parse(&youtube_html_parser,bigger_buff,len);
				html_exit(&youtube_html_parser);
#if MAKE_XPG_PLAYER
				xpgUpdateYouTubeLoading(0);
#endif
        	}
			else
			{
            /* Parse XML */
            parser = XML_ParserCreate(NULL);
            youtube_info->state = YOUTUBE_VIDEO_LIST_INIT;

            XML_SetUserData(parser, youtube_info);
            XML_SetElementHandler(parser, video_list_start_element_handler, video_list_end_element_handler);
            XML_SetCharacterDataHandler(parser, video_list_content_handler);

            bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			mpDebugPrint("youtube_fetch_video_list: got total data len = %d", len);

            if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
            {
				mpDebugPrint("youtube_fetch_video_list: %s at line %d, column %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser),
				XML_GetCurrentColumnNumber(parser));
            }

            if (youtube_info->error_code > 0)
            {
                ret = -NETFS_APP_ERROR;
                goto exit;
            }
            XML_Parse(parser, data, 0, 1);
#if MAKE_XPG_PLAYER
			xpgUpdateYouTubeLoading(0);
#endif
        }
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

/*
 * Google ClientLogin API for Youtube
 */
int youtube_ClientLogin(youtube_info_t *youtube_info, char *email, char *passwd, char *source, char *errstring)
{
    void *curl;
    CURLcode res;
    int ret = 0;
    int len;
    char *data, *end;
    char *url;
    XML_BUFF_link_t *ptr;
    int httpcode;
    int auth_len = sizeof(youtube_info->auth);

    char *bigger_buff=NULL;


    Xml_BUFF_init(NET_RECVHTTP);
	mpDebugPrint("NETFS_YOUTUBE: ClientLogin");

    data = youtube_malloc(512);
    if (data == NULL)
    {
    	ret = -4;
        goto exit;
    }

    curl = curl_easy_init();
    if(curl)
	{
        url = "https://www.google.com/youtube/accounts/ClientLogin";
        snprintf(data,512, "Email=%s&Passwd=%s&service=youtube&source=%s", email, passwd, source);
        data[511] = '\0';
		mpDebugPrint("data %s",data);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		// set CURLOPT_COOKIEJAR
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); /* TODO */

        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

        res = curl_easy_perform(curl);
        TaskYield();

        /* check return code of curl_easy_perform() */
        if (res != CURLE_OK)
        {
            mpDebugPrint("youtube_ClientLogin: curl_easy_perform failed, res = %d", res);
            httpcode = 0;
            goto Youtube_cleanup_3;
        }

        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
            httpcode = 0;

Youtube_cleanup_3:
        /* always cleanup */
        curl_easy_cleanup(curl);

        youtube_mfree(data);

        mpDebugPrint("youtube_ClientLogin: http resp code=%d", httpcode);
        if (httpcode == 200)
        {
            ptr = App_State.XML_BUF;
            if (ptr != NULL)
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("youtube_ClientLogin(): got total data len = %d", len);
                if (data[len-1] == '\n')
                    data[len-1] = '\0';             /* remove LF */
                else
                    data[len] = '\0';
                data = strstr(data, "Auth=");
                if (data)
                {
                    strncpy(youtube_info->auth, data+5, auth_len);
                    if (strlen(data+5) >= auth_len)
                    {
                        youtube_info->auth[auth_len - 1] = '\0';
                        ret = -1;
                        MP_ASSERT(0);
                    }
                    //NetAsciiDump(data, auth_len+5); //only dump "Auth=" info
                }
                else
                {
                    MP_DEBUG("youtube_ClientLogin: no Auth=");
                    NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
                    ret = -3;
                }
            }
        }
        else
        {
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 Access Forbidden */
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("youtube_ClientLogin(): got total data len = %d", len);
                data[len] = '\0';
                data = strstr(data, "Error=");
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
    }

exit:
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

}

int youtube_init(char *username,char *password,char youtube_categories,char youtube_pop_most,char youtube_page_idx) // *base_dir
{
    video_entry_t     *tmp_entry;
    int     ret,i,len,count;
    char    error[32];
	BYTE testflag=0;
#if	DM9KS_ETHERNET_ENABLE||DM9621_ETHERNET_ENABLE
#if (P2P_TEST == 0&&MAKE_XPG_PLAYER)
	if(GetNetConfigP2PTestFlag())
#endif		
		testflag = 1;
#endif	
#if HAVE_HTTP_SERVER
      testflag = 1;
#endif
    if(testflag)
    {
    mpDebugPrint("P2P_TEST youtube_init %s",username);
    memset(&youtube_info, 0, sizeof(youtube_info_t));
	memset(&youtube_video, 0, sizeof(youtube_video_t));
    strncpy(youtube_info.username, username, MAX_USERNAME);
    youtube_info.cur_video = &youtube_info.video_list;
	
	tmp_entry = (video_entry_t *) youtube_malloc(sizeof(video_entry_t));
	memset(tmp_entry, 0, sizeof(video_entry_t));
	youtube_info.cur_video->next = tmp_entry;
	youtube_info.cur_video = tmp_entry;
	youtube_info.video_num++;
	memcpy(youtube_info.cur_video->title,"TEST",strlen("TEST"));
    }
	else
	{
	mpDebugPrint("youtube_init %s",username);
    memset(&youtube_info, 0, sizeof(youtube_info_t));
	memset(&youtube_video, 0, sizeof(youtube_video_t));
    strncpy(youtube_info.username, username, MAX_USERNAME);
    youtube_info.cur_video = &youtube_info.video_list;
    youtube_base_dir = "MPXWIFI"; //base_dir;

    //username= "MPXHOMEPAGE";

	if(!strncmp(username,"MPXHOMEPAGE",11))
	{
		mpDebugPrint("Youtube go to homepage categories %d",youtube_categories);
	}
	else
	{
    //ret = youtube_ClientLogin(&youtube_info, username, "wifi12345678", "MapicPixel-DPF-0.1", error);
    ret = youtube_ClientLogin(&youtube_info, username, password, "MapicPixel-DPF-0.1", error);

    if (ret < 0)
    {
        if (ret == -2)
            mpDebugPrint("Login failed due to '%s'\n", error);
        return ret;
    }
	}
    MP_DEBUG("1 picasa_fetch_album_list2 @@@@@@@@@@@@@@@@@@@@@@");
	youtube_video.youtube_video_page = youtube_page_idx;
	youtube_video.youtube_video_cate = youtube_categories;
    ret = youtube_fetch_video_list(&youtube_info,username,youtube_pop_most);
    if (ret < 0)
    {
        MP_ALERT("picasa_fetch_album_list2(): error code: %d\n", youtube_info.error_code);
        return ret;
    }
#if 1
    count = 0;
    youtube_info.cur_video = youtube_info.video_list.next;
    tmp_entry = youtube_get_video_entry(0);//youtube_info.cur_video;

    if (tmp_entry)
    {

        //while (tmp_entry)
        for(i=1;i<youtube_info.video_num;i++)
        {
        	//mpDebugPrint("counter %d",count);
            //mpDebugPrint("Fetch photos from '%s' with id '%s'\n",tmp_entry->title,tmp_entry->id);
			//mpDebugPrint("Fetch photos thumbnail '%s'\n",tmp_entry->thumbnail);
            Net_Xml_PhotoSetList(tmp_entry->title,count);
            count ++;
            tmp_entry = youtube_get_video_entry(i);//tmp_entry->next;
        }
        Net_PhotoSet_SetCount(count);
    }
    else
    {
        mpDebugPrint("No video.\n");
    }
#endif
	}
    return 0;
}

void youtube_exit(const char *base_dir)
{
   video_entry_t     *tmp_entry;
 /* free resources allocated for all album */

 	//Xml_BUFF_free(NET_YOUTUBE);
    //youtube_info.cur_video = youtube_info.video_list.next;
    while (youtube_info.cur_video)
    {
        tmp_entry = youtube_info.cur_video;
        youtube_info.cur_video = youtube_info.cur_video->next;

        youtube_mfree(tmp_entry);
    }
}

#endif
#else
int youtube_get_data(BYTE *gbuf,DWORD start,DWORD END)
{
 return 0;
}
#endif//HAVE_YOUTUBE
