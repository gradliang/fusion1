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
#include "xmlyahoo.h"
#include "..\..\lwip\include\net_sys.h"
#include "..\..\lWIP\include\net_socket.h"
#include "..\..\CURL\include\net_curl_curl.h"

#define YAHOO_HOST "www.yahoo.com"
#define yahoo_malloc(sz)   mm_malloc(sz)
#define yahoo_mfree(ptr)   mm_free(ptr)

static html_parser_t yahoo_html_parser;
yahoo_info_t   yahoo_info;
int check_point = 0;
int check_stop = 0;
BYTE pass_count = 0;
extern Net_App_State App_State;
//note: fixed to use global my_write_func() for all xmlxxxxxx.c files!
size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf);
BYTE *Merge_ListData_to_SingleBuffer(XML_BUFF_link_t *list_head, int *total_data_len);

yahoo_info_t * getYahooInfo()
{
    return &yahoo_info;
}

char * yahoo_get_pricture_buffer_by_index(BYTE index)
{
    mpDebugPrint("yahoo_get_pricture_buffer_by_index(index=%d)", index);
    //mpDebugPrint("title_preview[index]=%s)", title_preview[index]);
    if(index==4) memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=%5EN225&t=1268046164", 56);
    if(index==5) memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=%5EKS11&t=1268046164", 56);
    if(index==6) memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=%5EHSI&t=1268046164", 56);
    if(index==7) memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=000001.SS&t=1268046164", 56);

    if(index==8) memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=%5EFTSE&t=1268046164", 56); 
    if(index==9) memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=%5EGDAXI&t=1268046164", 56);
    if(index==10)memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=%5EFCHI&t=1268046164", 56);
    if(index==11)memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=%5EAEX&t=1268046164", 56);
 
    if(index==12)memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=%5EDJI&t=1268046164", 56); 
    if(index==13)memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=^IXIC&t=1268046164", 56);
    if(index==14)memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=%5EGSPC&t=1268046164", 56);
    if(index==15)memcpy(title_preview[index], "http://ichart.finance.yahoo.com/t?s=%5ERUT&t=1268046164", 56);
       
    return yahoo_get_pricture_buffer(title_preview[index]);
}

char * yahoo_get_pricture_buffer(BYTE *url)
{
	int i,len;
	int length;
	char *yahoo_buf;
#if Make_CURL
	//mpDebugPrint("yahoo_get_pricture_buffer %s",url);
	Xml_BUFF_init(NET_RECVHTTP);
	
	length = Get_Image_File(url, App_State.XML_BUF);
	//mpDebugPrint("length %d",length);
    yahoo_buf = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
	   
	Xml_BUFF_free(NET_RECVHTTP);
#endif	
	return yahoo_buf;

}

void yahoo_get_pricture_buffer_free(char *yahoo_buf)
{
   
   if(yahoo_buf != NULL)
	   ext_mem_free(yahoo_buf);
}

class_entry_t	*yahoo_get_class_entry(BYTE idx)
{
	class_entry_t *class_entry;
	int i=0;
	
	class_entry = yahoo_info.cur_class;
	while(class_entry)
	{
	  if(idx==0)
	  {
	  	break;
	  }
	  else
	  {
	  	class_entry = class_entry->next;
		i++;
		if(i==idx)
			break;
			
	  }
	  	
	}

 	return   class_entry;
}

static void yahoo_html_content_handler(void *user_data, const char *content, int len)
{
	yahoo_info_t	 *yahoo_info	  = (yahoo_info_t *) user_data;

    //mpDebugPrint("yahoo_html_content_handler %d %s",len,content);	

	if(yahoo_info->state == YAHOO_FIND_TBODY)
	{
	  switch(check_point)
	 {
	    case 0:
	  		memcpy(yahoo_info->cur_class->title,content,len);
	  		break;
		case 1:
			memcpy(yahoo_info->cur_class->href,content,len);
			break;
		case 2:
			memcpy(yahoo_info->cur_class->td,content,len);
			break;
		case 3:
		    if(len>2)
			     memcpy(yahoo_info->cur_class->span,content,len);
			break;
		case 4:
		    if(len>2)
			     memcpy(yahoo_info->cur_class->vol,content,len-2);
			break;
			
	 }
	}
	else if(yahoo_info->state == YAHOO_SHOCK_TIME)
	{
	  if(len == 5)
	  	{
	  	  memcpy(yahoo_info->time,content,len);
		  mpDebugPrint("yahoo_info->time %s",yahoo_info->time);	
	  	}
	}else if(yahoo_info->state == YAHOO_SHOCK_ID)
	{
	  memcpy(yahoo_info->id,content,len);
      mpDebugPrint("yahoo_info->id %s",yahoo_info->id);	

	}else if(yahoo_info->state == YAHOO_SHOCK_BARGAIN)
	{
		memcpy(yahoo_info->bargain,content,len);
		mpDebugPrint("yahoo_info->bargain %s",yahoo_info->bargain);	
	}
    else if(yahoo_info->state == YAHOO_SHOCK_INVEST)
	{
	    if(pass_count==0)
			pass_count++;
		else
		{
			memcpy(yahoo_info->invest,content,len);
			mpDebugPrint("yahoo_info->invest %s",yahoo_info->invest);	
			yahoo_info->state = YAHOO_SHOCK_SALE;
			pass_count = 0;
		}
	}
    else if(yahoo_info->state == YAHOO_SHOCK_SALE)
	{
		//mpDebugPrint("1yahoo_html_content_handler %d %s",len,content);	
		if(pass_count==0)
			pass_count++;
		else
		{
  	        memcpy(yahoo_info->sale,content,len);
			mpDebugPrint("yahoo_info->sale %s",yahoo_info->sale);	
			pass_count = 0;
		}

	}
    else if(yahoo_info->state == YAHOO_SHOCK_FLUCTUATION)
	{
		if(pass_count==0)
		{
			pass_count++;
			memcpy(yahoo_info->piece,content,len);
			mpDebugPrint("yahoo_info->piece %s",yahoo_info->piece);	

		}
		else
		{
  	        memcpy(yahoo_info->fluctuation,content,len);
			mpDebugPrint("yahoo_info->fluctuation %s",yahoo_info->fluctuation);	
			pass_count = 0;
		}

	}
	else if(yahoo_info->state == YAHOO_SHOCK_PAST)
	{
	     if(pass_count==0)
	     {
		 	pass_count++;
			memcpy(yahoo_info->past,content,len);
			mpDebugPrint("yahoo_info->past %s",yahoo_info->past);	
	     }
		 else
		 {
			 pass_count = 0;
		 }
	}
	else if(yahoo_info->state == YAHOO_SHOCK_OPENING)
	{
		 if(pass_count==0)
		 {
		 	pass_count++;
			memcpy(yahoo_info->opening,content,len);
			mpDebugPrint("yahoo_info->opening %s",yahoo_info->opening);	
		 }
		 else
		 {
		    pass_count = 0;
		 }
	}
	else if(yahoo_info->state == YAHOO_SHOCK_TIPTOP)
	{
		 if(pass_count==0)
		 {
		 	pass_count++;
			memcpy(yahoo_info->tiptop,content,len);
			mpDebugPrint("yahoo_info->tiptop %s",yahoo_info->tiptop);	
		 }
		 else
		 {
			pass_count = 0;
		 }
	}
	else if(yahoo_info->state == YAHOO_SHOCK_LOWEST)
	{
		 if(pass_count==0)
		 {
		 	pass_count++;
			memcpy(yahoo_info->lowest,content,len);
			mpDebugPrint("yahoo_info->lowest %s",yahoo_info->lowest);	
		 }
		 else
		{
			pass_count = 0;
		}
	}



		

	
}

static void yahoo_html_tag_start_handler(void *user_data, const char *tag_name, char **attr)
{
	yahoo_info_t	 *yahoo_info	  = (yahoo_info_t *) user_data;

	class_entry_t		*class_entry;
	class_entry_t		*tmp_entry;
	BYTE tmpbuf[2];
	//mpDebugPrint("yahoo_html_tag_start_handler tag_name %s",tag_name); 

	if(!strncasecmp(tag_name,"h3",2))
	{
	while (*attr)
	{
	
			if(!strncasecmp(*attr,"title",5))
			{
				yahoo_info->state = YAHOO_FIND_TITAL;
				break;
			}
		attr ++;
	}
	}
	else if(!strncasecmp(tag_name,"tbody",5))
	{
	
	    if(yahoo_info->state == YAHOO_FIND_TITAL)
	    {
			yahoo_info->state = YAHOO_FIND_TBODY;
			//mpDebugPrint(" %s ",*attr);
			class_entry = (class_entry_t *) yahoo_malloc(sizeof(class_entry_t));
			MP_ASSERT(class_entry);
			memset(class_entry, 0, sizeof(class_entry_t));
			yahoo_info->cur_class->next = class_entry;
			yahoo_info->cur_class = class_entry;
	    }
		
	}
	else if(!strncasecmp(tag_name,"td",2))
	{
	    
	    if(yahoo_info->state == YAHOO_FIND_TBODY)
	    {
			check_point++;
			
	    }
	}
	else if(!strncasecmp(tag_name,"img",3))
	{
	    
	    if(yahoo_info->state == YAHOO_FIND_TITAL)
	    {
	    
			while (*attr)
			{
				if(!strncasecmp(*attr,"src",3))
				{
					attr ++;
					if((check_point < MAX_TITAL)&&(!check_stop))
					{
					
					    //mpDebugPrint("check_point  %d!!",check_point);
					    memset(title_preview[check_point],0x00,MAX_URL);
						memcpy(title_preview[check_point],*attr,strlen(*attr));
						//mpDebugPrint("title_preview %s ",title_preview[check_point]);
						if(check_point==3)
							check_stop = 1; 
						else
							check_point++;
					}
					break;
				}
				attr ++;
			}
			
	    }
	}
	else if(yahoo_info->state == YAHOO_SHOCK)
	{
			
	  //mpDebugPrint("yahoo_html_tag_start_handler tag_name %s",tag_name); 
		if(!strncasecmp(tag_name,"a",1))
	{
	  while (*attr)
	  {
		  	  if(!strncasecmp(*attr,"href",6))
		  	  {
		  	    attr ++;
		  	  	memset(tmpbuf, 0, 2);
				memcpy(tmpbuf,*attr,2);
	  
		  	   if(!strncasecmp(tmpbuf,"/q",2))
					yahoo_info->state = YAHOO_SHOCK_ID;
		  	  }
		  //mpDebugPrint(" %s ",*attr);
	
		  attr ++;
	  }
	}
	}
    else if(yahoo_info->state == YAHOO_SHOCK_TIME)
    {
         if(strlen(tag_name)==1)
         {
		 	if(!strncasecmp(tag_name,"b",1))
				yahoo_info->state = YAHOO_SHOCK_BARGAIN;

         }

    }
	else if(yahoo_info->state == YAHOO_SHOCK_SALE)
    {
    	 //mpDebugPrint("yahoo_html_tag_start_handler tag_name strlen %d %s",strlen(tag_name),tag_name); 
		 if(!strncasecmp(tag_name,"font",4))
			     yahoo_info->state = YAHOO_SHOCK_FLUCTUATION;

    }



	
}

static void yahoo_html_tag_end_handler(void *user_data, const char *tag_name)
{
	yahoo_info_t	 *yahoo_info	  = (yahoo_info_t *) user_data;
	//mpDebugPrint("youtube_html_tag_end_handler %s",tag_name); 
	
	if(yahoo_info->state == YAHOO_SHOCK_BARGAIN)
	{
	   yahoo_info->state = YAHOO_SHOCK_INVEST;
	}
	else if(yahoo_info->state == YAHOO_SHOCK_ID)
	{
		yahoo_info->state = YAHOO_SHOCK_TIME;
	}
	else if(yahoo_info->state == YAHOO_SHOCK_FLUCTUATION)
	{
      if(!strncasecmp(tag_name,"td",2))
	  	yahoo_info->state = YAHOO_SHOCK_PAST;
	}
	else if(yahoo_info->state == YAHOO_SHOCK_PAST)
	{
      if(!strncasecmp(tag_name,"td",2))
	  	yahoo_info->state = YAHOO_SHOCK_OPENING;
	}
	else if(yahoo_info->state == YAHOO_SHOCK_OPENING)
	{
      if(!strncasecmp(tag_name,"td",2))
	  	yahoo_info->state = YAHOO_SHOCK_TIPTOP;
	}
	else if(yahoo_info->state == YAHOO_SHOCK_TIPTOP)
	{
      if(!strncasecmp(tag_name,"td",2))
	  	yahoo_info->state = YAHOO_SHOCK_LOWEST;
	}
	else if(yahoo_info->state == YAHOO_SHOCK_LOWEST)
	{
      if(!strncasecmp(tag_name,"td",2))
	  	yahoo_info->state = YAHOO_NULL;
	}

	
	if(!strncasecmp(tag_name,"tbody",5))
	{
		if(yahoo_info->state == YAHOO_FIND_TBODY)
			yahoo_info->state = YAHOO_FIND_TITAL;
		check_point = 0;
	}
	else if(!strncasecmp(tag_name,"label",5))
	{
	  //mpDebugPrint("!!clear check_point!!");
	  check_point = 0;
	}
}

int yahoo_get_homepage(void)
{
     int httpcode;
#if Make_CURL

    void *curl;
    char *data; 
    char *bigger_buff; 
	BYTE yahoo_url[256];
    struct curl_slist *header=NULL;
    int  ret = 0;
	
	CURLcode res;
    XML_BUFF_link_t *ptr;
    int len;
	
	mpDebugPrint("yahoo_get_homepage");
		
	
	Xml_BUFF_init(NET_RECVHTTP);    
    curl = curl_easy_init();
    if(curl) {
        data = yahoo_malloc(512);
        if (data == NULL)
            goto exit;
		
		snprintf(yahoo_url, 256, 
				 "http://tw.stock.yahoo.com/");
    	
			mpDebugPrint("youtube_url %s",yahoo_url);
			snprintf(data, 512,"Accept: text/html, */*");
			data[511] = '\0';
			header = curl_slist_append(header, data);
			snprintf(data, 512,"Referer: http://www.youtube.com");
			data[511] = '\0';
			header = curl_slist_append(header, data);
		
	        //mpDebugPrint("youtube_get_video_info: extra header=%d", strlen(data));
	        curl_easy_setopt(curl, CURLOPT_URL, yahoo_url);
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
				goto Youtube_cleanup_1;
			}
		
			if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
				httpcode = 0;
		
		Youtube_cleanup_1:
			/* always cleanup */
			curl_easy_cleanup(curl);
		
			curl_slist_free_all(header);
		
			yahoo_mfree(data);
			
			mpDebugPrint("yahoo_get_video_info: http error resp code=%d", httpcode);
			if (httpcode == 200)
			{
			    
				bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
				//NetAsciiDump(bigger_buff,len);
				yahoo_info.state = YAHOO_NULL;
				html_init(&yahoo_html_parser, &yahoo_info);
				html_set_content_handler(&yahoo_html_parser, yahoo_html_content_handler);
				html_set_tag_start(&yahoo_html_parser, yahoo_html_tag_start_handler);
				html_set_tag_end(&yahoo_html_parser, yahoo_html_tag_end_handler);
				html_parse(&yahoo_html_parser,bigger_buff,len);
				html_exit(&yahoo_html_parser);
		
			}
			else if (httpcode > 0)					/* TODO */
			{
				mpDebugPrint("yahoo_get_video_info: http error resp code=%d", httpcode);
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
		#endif	
			return httpcode;
    		
}
int yahoo_get_shock(BYTE *id)
{
    int httpcode;
#if Make_CURL
    void *curl;
    char *data; 
    char *bigger_buff; 
	BYTE yahoo_url[256];
    struct curl_slist *header=NULL;
    int  ret = 0;
	CURLcode res;
    XML_BUFF_link_t *ptr;
    int len;

  	Xml_BUFF_init(NET_RECVHTTP);    
    curl = curl_easy_init();
    if(curl) {
        data = yahoo_malloc(512);
        if (data == NULL)
            goto exit;
		
		snprintf(yahoo_url, 256, 
				 "http://tw.stock.yahoo.com/q/q?s=%s",id);
    	
			mpDebugPrint("youtube_url %s",yahoo_url);
			snprintf(data, 512,"Accept: text/html, */*");
			data[511] = '\0';
			header = curl_slist_append(header, data);
			snprintf(data, 512,"Referer: http://www.youtube.com");
			data[511] = '\0';
			header = curl_slist_append(header, data);
		
	        //mpDebugPrint("youtube_get_video_info: extra header=%d", strlen(data));
	        curl_easy_setopt(curl, CURLOPT_URL, yahoo_url);
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
				mpDebugPrint("youtube_get_stock_location: curl_easy_perform failed, res = %d", res);
				httpcode = 0;
				goto Youtube_cleanup_1;
			}
		
			if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
				httpcode = 0;
		
		Youtube_cleanup_1:
			/* always cleanup */
			curl_easy_cleanup(curl);
		
			curl_slist_free_all(header);
		
			yahoo_mfree(data);
			
			mpDebugPrint("yahoo_get_stock_info: http error resp code=%d", httpcode);
			if (httpcode == 200)
			{
			    
				bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
				//NetAsciiDump(bigger_buff,len);
				yahoo_info.state = YAHOO_SHOCK;
				html_init(&yahoo_html_parser, &yahoo_info);
				html_set_content_handler(&yahoo_html_parser, yahoo_html_content_handler);
				html_set_tag_start(&yahoo_html_parser, yahoo_html_tag_start_handler);
				html_set_tag_end(&yahoo_html_parser, yahoo_html_tag_end_handler);
				html_parse(&yahoo_html_parser,bigger_buff,len);
				html_exit(&yahoo_html_parser);
		
			}
			else if (httpcode > 0)					/* TODO */
			{
				mpDebugPrint("yahoo_get_stock_info: http error resp code=%d", httpcode);
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
#endif			
			return httpcode;

}

char    *stockid = "2332";
void setstockid(char *inid)
{
    memcpy(stockid, inid, 4);
}

int yahoo_init(char *username,char *password,char youtube_categories,char youtube_pop_most,char youtube_page_idx) // *base_dir
{
    class_entry_t     *tmp_entry;

    int     ret,i,len,count;
    char    error[32];

	mpDebugPrint("yahoo_init %s",username);
	
    yahoo_info.cur_class = &yahoo_info.class_list;
	
	yahoo_get_homepage();
	
    count = 0;
    yahoo_info.cur_class = yahoo_info.class_list.next;
    tmp_entry = yahoo_get_class_entry(count);//yahoo_info.cur_class;
    if (tmp_entry)
    {
        while (tmp_entry)
        {
            mpDebugPrint("Class %s %s %s %s %s\n",tmp_entry->title,tmp_entry->href,tmp_entry->td,tmp_entry->span,tmp_entry->vol);
			mpDebugPrint("counter %d",count);
			if(count < MAX_TITAL)
			{
				mpDebugPrint("url %s",title_preview[count]);
				//yahoo_get_pricture_buffer(title_preview[count]);
			}
            count ++;
            tmp_entry = yahoo_get_class_entry(count);
        }
        //setstockid("2332");
		if(yahoo_get_shock(stockid)==200)
			mpDebugPrint("yahoo_get_stock OK !!");
    }
    else
    {
        mpDebugPrint("No CLASS.\n");
    }
    return -1;
}

void yahoo_exit(const char *base_dir)
{
	  class_entry_t 	*tmp_entry;
	/* free resources allocated for all album */
	
	   yahoo_info.cur_class = yahoo_info.class_list.next;
	   while (yahoo_info.cur_class)
	   {
		   tmp_entry = yahoo_info.cur_class;
		   yahoo_info.cur_class = yahoo_info.cur_class->next;
	
		   yahoo_mfree(tmp_entry);
	   }


}

