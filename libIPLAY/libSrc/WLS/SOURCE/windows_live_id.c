
#include "windows_live.h"
#include "..\..\lwip\include\net_sys.h"
#include "..\..\CURL\include\net_curl_curl.h"
#include "..\..\xml\include\expat.h"
#include "..\..\xml\include\netfs.h"
#include "..\..\xml\include\netfs_pri.h"

extern Net_App_State App_State;
extern windows_live_info_t   windows_live_info;
static html_parser_t wls_html_parser;
static struct netfs_file_entry jpeg_info;

#define CRLF "\r\n"

size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf);

static
int my_trace(CURL *handle, curl_infotype type,
             unsigned char *data, size_t size,
             void *userp)
{
  {
      char *s;
        s = windows_live_malloc(size+1);
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
	case CURLINFO_DATA_OUT:
		  memcpy(s, data, size);
		  s[size] = '\0';
	//		  mpDebugPrint("header out(%d): %s\n", size, s);
			mpDebugPrint("data out(%d):", size);
			  NetAsciiDump(data, size);
	break;
    default: /* nada */
      break;
    }

    return 0;
  }

  return 0;
}

static void wls_start_element_handler(void *user_data, const char *name, const char **attr)
{
	
	//mpDebugPrint("start_element_handler %s",name);
	if (!strcasecmp(name, "psf:credProperty"))
	{
		//mpDebugPrint("start_element_handler %s",name);
		windows_live_info.state = WINDOWS_LIVE_GET_DETAILS;
	}
	

}

static void wls_end_element_handler(void *user_data, const char *name)
{
     
	 //mpDebugPrint("end_element_handler %s",name);
	 if (!strcasecmp(name, "psf:credProperty"))
		windows_live_info.state = WINDOWS_LIVE_SDK_LOGIN;
		
}

static void wls_content_handler(void *user_data, const char *s, int len)
{
    int i,j;
	BYTE cp=0;
	//mpDebugPrint("content_handler %s",s);
	if(windows_live_info.state==WINDOWS_LIVE_GET_DETAILS)
	{
	  //mpDebugPrint("content_handler len %d %s ",len,s);
	  if(len==16)
	  {
	    memcpy(windows_live_info.cid,s,len);
		mpDebugPrint("CID %s ",windows_live_info.cid);
	  }
	}

}

static void wls_html_content_handler(void *user_data, const char *content, int len)
{
	//mpDebugPrint("wls_html_content_handler %d %s",len,content); 	
    windows_live_info_t *wls_info = (windows_live_info_t*)user_data;
	BYTE tmp_buf[8];
	int i,j;
	BYTE cp=0;
	
	if (wls_info->state==WINDOWS_LIVE_ALBUM_FOUND)
	{
	  //mpDebugPrint("wls_html_content_handler %d %s",len,content); 	
	  wls_info->state = WINDOWS_LIVE_ALBUM_LIST_ID;
	}
	else if(wls_info->state==WINDOWS_LIVE_PHOTO_INIT)
	{
	  //mpDebugPrint("wls_html_content_handler %d %s",len,content); 	
	}
	else if(wls_info->state==WINDOWS_LIVE_PHOTO_LOCATION)
	{
	  //mpDebugPrint("wls_html_content_handler %d %s",len,content); 	
	  if (!strcasecmp(content, "Updating tag..."))
	  	wls_info->state = WINDOWS_LIVE_PHOTO_LOCATION_FOUND;
	  
	}
	else if(wls_info->state==WINDOWS_LIVE_GET_HOMEPAGE)
	{
	 // mpDebugPrint("WINDOWS_LIVE_GET_HOMEPAGE %d %s",len,content); 	
	  
	}
	
}

static void wls_html_tag_start_handler(void *user_data, const char *tag_name, char **attr)
{
    windows_live_info_t *wls_info = (windows_live_info_t*)user_data;
	album_entry_t		*album_entry;
	BYTE cp=0,cp1=0;
	int i,j;
	BYTE *tmp_cpbuf;
	
	//mpDebugPrint("wls_html_tag_start_handler state %d",wls_info->state);	

	if(wls_info->state == WINDOWS_LIVE_ALBUM_LIST_ID)
	{
	//mpDebugPrint("wls_html_tag_start_handler %s %s",tag_name,*attr);	
		if (!strcasecmp(tag_name, "div"))
		{
		   while(*attr)
		   	{
		   	  if(!strcasecmp(*attr,"albumTile avNonEmptyAlbum"))
		   	  {
		   	    wls_info->state = WINDOWS_LIVE_ALBUM_INIT;
				break;
		   	  }
		   	  //mpDebugPrint("%s",*attr);
			  attr++;
		   	}
		}
	}
	else if(wls_info->state == WINDOWS_LIVE_ALBUM_INIT)
	{
	  
	  //mpDebugPrint("wls_html_tag_start_handler %s %s",tag_name,*attr);  
	  
	  while(*attr)
	   {
	   
		   if(!strcasecmp(*attr,"albumClip"))
		   {
			   wls_info->state = WINDOWS_LIVE_ALBUM_FOUND;
			   album_entry = (album_entry_t *) windows_live_malloc(sizeof(album_entry_t));
			   MP_ASSERT(album_entry);
			   memset(album_entry, 0, sizeof(album_entry_t));
			   wls_info->cur_album->next = album_entry;
			   wls_info->cur_album = album_entry;
			   attr+=2;
			   memcpy(album_entry->url,*attr,strlen(*attr));
			   for(i=0,j=0;i<strlen(*attr);i++)
			   {
			     if((album_entry->url[i]=='a')&&(album_entry->url[i+1]=='s')&&
				 	(album_entry->url[i+2]=='p')&&(album_entry->url[i+3]=='x'))
			     	{
			     	  i+=5;
					  cp=1;
			     	}
				 if(cp)
				 {
				   album_entry->title[j]=album_entry->id[j]=album_entry->url[i];
				   j++;
				 }
			     
			   }
			   //mpDebugPrint("id %s",album_entry->id);
			   //mpDebugPrint("title %s",album_entry->title);
			   cp=0;
			   break;
			   
		   }
		   attr++;
	   }
	}
	else if(wls_info->state==WINDOWS_LIVE_PHOTO_INIT)
	{
	
	#if 0
		mpDebugPrint("wls_html_tag_start_handler %s %s",tag_name,*attr);	
		
		while(*attr)
		{
		  mpDebugPrint("%s",*attr);
		  attr++;
		}
	#endif	
		
		if (!strcasecmp(tag_name, "img"))
		{
			while(*attr)
			{
			  if(!strcasecmp(*attr,"onload"))
			  {
			  	wls_info->state = WINDOWS_LIVE_PHOTO_FOUND;
				memset(&jpeg_info, 0, sizeof(jpeg_info));
			  	attr+=5;
				//mpDebugPrint("onload %s",*attr);
				memcpy(jpeg_info.thumbnail_url,*attr,strlen(*attr));
				break;
			  }
			  attr++;
			}
		}
	}
	else if(wls_info->state==WINDOWS_LIVE_PHOTO_FOUND)
	{
	#if 0
		mpDebugPrint("wls_html_tag_start_handler %s %s",tag_name,*attr);	
		
		while(*attr)
		{
		  mpDebugPrint("%s",*attr);
		  attr++;
		}
	#endif	
		
		if (!strcasecmp(tag_name, "img"))
		{
			while(*attr)
			{
			  if(!strcasecmp(*attr,"onload"))
			  {
				  wls_info->state = WINDOWS_LIVE_PHOTO_SRC;
				  memset(&jpeg_info, 0, sizeof(jpeg_info));
				  attr+=5;
				  memcpy(jpeg_info.thumbnail_url,*attr,strlen(*attr));
				  break;
			  }
			  attr++;
			}
		}
	}
	else if(wls_info->state==WINDOWS_LIVE_PHOTO_SRC)
	{
	    //mpDebugPrint("wls_html_tag_start_handler %s %s",tag_name,*attr);	
		if (!strcasecmp(tag_name, "a"))
		{
		
			while(*attr)
			{
			  if(!strcasecmp(*attr,"href"))
			  {
				attr++;
				memcpy(jpeg_info.url,*attr,strlen(*attr));
				break;
			  }
			  attr++;
			  TaskYield();
			}
		 
		}
		else if(!strcasecmp(tag_name, "img"))
		{
		  
		  while(*attr)
		  {
			if(!strcasecmp(*attr,"title"))
			{
			  attr++;
			  //mpDebugPrint("title %d %s",strlen(*attr),*attr);
			  if(strlen(*attr)>0)
			  {
			  	tmp_cpbuf = windows_live_malloc(256);
				memset(tmp_cpbuf,0x00,256);
				memcpy(tmp_cpbuf,*attr,strlen(*attr));
			  }
			  for(i=0,j=0;i<strlen(*attr);i++)
			  {
			    if((tmp_cpbuf[i]=='.')&&(tmp_cpbuf[i+1]=='j')&&(tmp_cpbuf[i+2]=='p')&&(tmp_cpbuf[i+3]=='g'))
			    {
					cp=1;
					memcpy(jpeg_info.pathname+i,tmp_cpbuf+i,4);
					i+=4;
						
			    }
				if(cp)
				{
				  if((tmp_cpbuf[i]=='S')&&(tmp_cpbuf[i+1]=='i')&&(tmp_cpbuf[i+2]=='z')&&(tmp_cpbuf[i+3]=='e'))
				  {
                     i+=6;
				     cp1=1;
					 j=0;
				  }
				  if(cp1)
				  {
					if((tmp_cpbuf[i]=='K')&&(tmp_cpbuf[i+1]=='B'))
				    {
				    #if 0
				     mpDebugPrint("jpeg_info.pathname %s",jpeg_info.pathname);
				     mpDebugPrint("jpeg_info.length %s",jpeg_info.length);
				     mpDebugPrint("jpeg_info.url %s",jpeg_info.url);
					 mpDebugPrint("jpeg_info.thumbnail_url %s",jpeg_info.thumbnail_url);
					#endif 
				     break;
				    }
				  	jpeg_info.length[j] =  tmp_cpbuf[i];
					j++;
				  }
				  	
				}
				else
				{
			  		jpeg_info.pathname[j]= tmp_cpbuf[i];
						j++;
				}
					
			  }
			  if(strlen(*attr)>0)
			  {
			  	Net_Xml_parseFileList(&jpeg_info);		
				windows_live_mfree(tmp_cpbuf);
			  }
			}
			//mpDebugPrint("%s",*attr);
			attr++;
			TaskYield();
		  }
		}
	
	}
#if	1
	else if(wls_info->state==WINDOWS_LIVE_GET_HOMEPAGE)
	{
		
		//mpDebugPrint("wls_html_tag_start_handler %s %s",tag_name,*attr);	
		if (!strcasecmp(tag_name, "link"))
		{
			while(*attr)
			{
			
			  //mpDebugPrint("%s",*attr);
			  if (!strcasecmp(*attr, "text/css"))
			  {
			  
			    attr+=2;
				memcpy(wls_info->css_url,*attr,strlen(*attr));
			  	mpDebugPrint("wls_info->css_url %s",wls_info->css_url);
				wls_info->state= WINDOWS_LIVE_GET_JAVASCRIPT;
			  }
			  
			  attr++;
			}
		}
	}
	else if(wls_info->state==WINDOWS_LIVE_GET_JAVASCRIPT)
	{
		   
		   //mpDebugPrint("wls_html_tag_start_handler %s %s",tag_name,*attr);    
		   if (!strcasecmp(tag_name, "script"))
		   {
			   while(*attr)
			   {
			   
			     if (!strcasecmp(*attr, "src"))
			     {
			        attr++;
					memcpy(wls_info->javascript_url,*attr,strlen(*attr));
					mpDebugPrint("wls_info->javascript_url %s",wls_info->javascript_url);
			     }
				 attr++;
			   }
		   }
	}
#endif	
	else if(wls_info->state==WINDOWS_LIVE_PHOTO_LOCATION_FOUND)
	{
		//mpDebugPrint("wls_html_tag_start_handler state %d",wls_info->state);	
		
		//mpDebugPrint("wls_html_tag_start_handler %s %s",tag_name,*attr);	
		if (!strcasecmp(tag_name, "a"))
		{
			while(*attr)
			{
			  //mpDebugPrint("%s",*attr);
			  
			  if(!strcasecmp(*attr,"href"))
			  {
			   //wls_info->state = WINDOWS_LIVE_PHOTO_LOCATION_FOUND;
			   attr++;
			   memcpy(wls_info->photo_url,*attr,strlen(*attr));
			   //mpDebugPrint("%s",wls_info->photo_url);
			   break;
			  }
			  attr++;
			  TaskYield();
			}
		}
	}

}

static void wls_html_tag_end_handler(void *user_data, const char *tag_name)
{
    windows_live_info_t *wls_info = (windows_live_info_t*)user_data;
	
	//mpDebugPrint("wls_html_tag_end_handler %s",tag_name);	
	if(wls_info->state==WINDOWS_LIVE_PHOTO_SRC)
	{
	
	    //mpDebugPrint("wls_html_tag_end_handler %s",tag_name);	
	    if(!strcasecmp(tag_name, "a"))
	    {
			wls_info->state = WINDOWS_LIVE_PHOTO_FOUND;
	    }
	}
	else if(wls_info->state==WINDOWS_LIVE_PHOTO_SRC)
	{
		if(!strcasecmp(tag_name, "input"))
		{
			wls_info->state = WINDOWS_LIVE_PHOTO_INIT;
		}
	}
	else if(wls_info->state==WINDOWS_LIVE_PHOTO_LOCATION)
	{
	  //mpDebugPrint("wls_html_tag_end_handler %s",tag_name); 
	}
	else if(wls_info->state==WINDOWS_LIVE_PHOTO_LOCATION_FOUND)
	{
		 if(!strcasecmp(tag_name, "img"))
		 {
			 wls_info->state = WINDOWS_LIVE_PHOTO_INIT;
		 }
	}

}

/* 
 * Windows live Login API 
 */
int windows_live_id_get_album(windows_live_info_t *windows_live_info, char *errstring)
{
    void *curl;
    CURLcode res;
    int ret = 0;
    int len;
    char *data, *end; 
    char *url; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    struct curl_slist *header=NULL;
    char *bigger_buff=NULL; 


    Xml_BUFF_init(NET_RECVHTTP);	
	mpDebugPrint("windows_live_id_get_album");

    data = windows_live_malloc(512);
	
    url = windows_live_malloc(64);
	
    if (data == NULL)
    {
    	ret = -4; 
        goto exit;
    }

    curl = curl_easy_init();
    if(curl) 
	{
        //url = "http://cid-35d74141169defc4.skydrive.live.com/albums.aspx";

		snprintf(url,64,"http://cid-%s.skydrive.live.com/albums.aspx",windows_live_info->cid);
		
		snprintf(data, 512,"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
		data[511] = '\0';
        header = curl_slist_append(header, data);
		
		snprintf(data, 512,"User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; GTB6; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; FDM; .NET CLR 3.0.04506.648; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729)");
		data[511] = '\0';
        header = curl_slist_append(header, data);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);

        //set follow location
        //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

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
		
        windows_live_mfree(url);

        windows_live_mfree(data);

        mpDebugPrint("windows_live_ClientLogin: http resp code=%d", httpcode);
        if (httpcode == 200)
        {
        #if 0
            ptr = App_State.XML_BUF;
            if (ptr != NULL)
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("windows_live_ClientLogin(): got total data len = %d", len);
                NetAsciiDump(bigger_buff, len);
            }
        #else    
			data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			windows_live_info->state = WINDOWS_LIVE_ALBUM_LIST_ID;
			html_init(&wls_html_parser, windows_live_info);
			html_set_content_handler(&wls_html_parser, wls_html_content_handler);
			html_set_tag_start(&wls_html_parser, wls_html_tag_start_handler);
			html_set_tag_end(&wls_html_parser, wls_html_tag_end_handler);
			html_parse(&wls_html_parser,bigger_buff,len);
			html_exit(&wls_html_parser);
		#endif	
        }
        //else if (httpcode > 0)                  /* TODO */
        else       // jeffery 20090320
        {
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 Access Forbidden */
            {
#if 0 //original code
                data = ptr->BUFF;
                len = ptr->buff_len;
#else //weiching new code
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("windows_live_ClientLogin(): got total data len = %d", len);
				
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
#endif
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
            
			    data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                sprintf(errstring, "HTTP Error %d", httpcode);
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
            }
            ret = -2;
        }
    }

exit:
    //weiching added
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

}

/* 
 * Windows live Login API 
 */
int windows_live_id_get_photo(windows_live_info_t *windows_live_info)
{
    void *curl;
    CURLcode res;
    int ret = 0;
    int len;
    char *data, *end; 
    char *url; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    struct curl_slist *header=NULL;
    char *bigger_buff=NULL; 


    Xml_BUFF_init(NET_RECVHTTP);	
	MP_DEBUG("windows_live_id_login");

    data = windows_live_malloc(512);
	
    url = windows_live_malloc(128);
    if (data == NULL)
    {
    	ret = -4; 
        goto exit;
    }

    curl = curl_easy_init();
    if(curl) 
	{
	    //url = "http://cid-35d74141169defc4.skydrive.live.com/browse.aspx/%e6%96%b0%e7%9b%b8%e7%b0%bf";
		
		snprintf(url,128,"http://cid-%s.skydrive.live.com/browse.aspx/%s",windows_live_info->cid,windows_live_info->cur_album->id);
		
		snprintf(data, 512,"User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; GTB6; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; FDM; .NET CLR 3.0.04506.648; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729)");
		data[511] = '\0';
        header = curl_slist_append(header, data);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);

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
		
        windows_live_mfree(url);

        windows_live_mfree(data);

        mpDebugPrint("windows_live_ClientLogin: http resp code=%d", httpcode);
        if (httpcode == 200)
        {
        #if 0
            ptr = App_State.XML_BUF;
            if (ptr != NULL)
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("windows_live_ClientLogin(): got total data len = %d", len);
                NetAsciiDump(bigger_buff, len);
            }
        #else    
			data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			windows_live_info->state = WINDOWS_LIVE_PHOTO_INIT;
			html_init(&wls_html_parser, windows_live_info);
			html_set_content_handler(&wls_html_parser, wls_html_content_handler);
			html_set_tag_start(&wls_html_parser, wls_html_tag_start_handler);
			html_set_tag_end(&wls_html_parser, wls_html_tag_end_handler);
			html_parse(&wls_html_parser,bigger_buff,len);
			html_exit(&wls_html_parser);
			
			//NetAsciiDump(bigger_buff, len);
		#endif	
        }
        //else if (httpcode > 0)                  /* TODO */
        else       // jeffery 20090320
        {
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 Access Forbidden */
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("windows_live_ClientLogin(): got total data len = %d", len);
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
                data[len] = '\0';
                data = strstr(data, "Error=");
            }
            else
            {
            
			    data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
            }
            ret = -2;
        }
    }

exit:
    //weiching added
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

}

/* 
 * Windows live Login API 
 */
BYTE *windows_live_get_photo_url(BYTE *buf,int len)
{
    BYTE url;
	windows_live_info_t *wls_info = &windows_live_info;
#if 0
     NetAsciiDump(buf, len);
#else    
	wls_info->state = WINDOWS_LIVE_PHOTO_LOCATION;
	html_init(&wls_html_parser, wls_info);
	html_set_content_handler(&wls_html_parser, wls_html_content_handler);
	html_set_tag_start(&wls_html_parser, wls_html_tag_start_handler);
	html_set_tag_end(&wls_html_parser, wls_html_tag_end_handler);
	html_parse(&wls_html_parser,buf,len);
	html_exit(&wls_html_parser);
#endif	

    return wls_info->photo_url;

}

/* 
 * Windows live Login API 
 */
int windows_live_id_sdk_login(windows_live_info_t *windows_live_info, char *email, char *passwd,char *errstring)
{
    void *curl;
    CURLcode res;
    int ret = 0;
    int len;
    char *data, *end; 
    char *url; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    //int auth_len = sizeof(windows_live_info->auth);
    struct curl_slist *header=NULL;
    char *bigger_buff=NULL; 
	XML_Parser              parser;
	
    Xml_BUFF_init(NET_RECVHTTP);	
	mpDebugPrint("windows_live_id_sdk_login %s",email);

    data = windows_live_malloc(2048);
	
    //url = windows_live_malloc(512);
    if (data == NULL)
    {
    	ret = -4; 
        goto exit;
    }

    curl = curl_easy_init();
    if(curl) 
	{
	    url = "https://dev.login.live.com/SDKLogin.srf";
        
		//snprintf(url, 512,"https://login.live.com/ppsecure/post.srf?%s",windows_live_info->post_login);
		//url[511] = '\0';
		
		snprintf(data, 1024,"Accept: text/*");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		
		snprintf(data, 1024,"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; GTB6; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; FDM; .NET CLR 3.0.04506.648; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; IDCRL 4.200.519.1; IDCRL-cfg 6.500.12348.0; App WindowsLiveIDClientSample.exe, 4.200.519.1, Tailspin Toys;user@tailspintoys.com;Tailspin Toys Application)");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		
		snprintf(data, 1024,"Connection: Keep-Alive");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
			
        snprintf(data, 1024,"Cache-Control: no-cache");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		
		snprintf(data, 1024,"Cookie: MSPCID=35d74141169defc4; WLOpt=RDCache=@livemail.tw:4&credtype=1&act=[1]; MSPPre=tcytj@hotmail.com; MUID=01008c24190142589afd465d6fe21ae7; OrigMUID=01008C24190142589AFD465D6FE21AE7; SRCHD=D=670186&AF=MSSRPD; NAP=V=1.9&E=832&C=tz7knQazIicsSYeohogNtbogUvi_Df9BtSacVUl_9ldbkiYHoN9uHQ&W=2; ANON=A=9C36B346CAC40C778C6571F7FFFFFFFF&E=82c&W=1; MH=MH=MSFT; wlidperf=throughput=7&latency=421; wlp=A|/twY-t:a*zc/I._; _wlx_affinity_state_v4.0=YnkxLnB2dC1zdG9yYWdlLmxpdmUuY29tfGJsdTEucHZ0LXN0b3JhZ2UubGl2ZS5jb218cHJveHktYmF5LnB2dC1jb250YWN0cy5tc24uY29tKjEsNDAzMDQzOTc1NjU4MzcyLDAsfDEsMTUwNTMzNTM3OTE2NjYxNzY1OSwxLHwxLDM4Nzk2NDEzNTE4MTExMDA2MTIsMCx8MSwzNUQ3NDE0MTE2OURFRkM0LDAsMg==; s_lastvisit=1246512756967; ck-35D74141169DEFC4=633830349208575912; _wlx_affinity_state_v3.0=Q2lkLDQwMzA0Mzk3NTY1ODM3MiwsYnkxfA=="
			);
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		
        snprintf(data,2048,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<Envelope xmlns=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:wsse=\"http://schemas.xmlsoap.org/ws/2003/06/secext\" xmlns:saml=\"urn:oasis:names:tc:SAML:1.0:assertion\" xmlns:wsp=\"http://schemas.xmlsoap.org/ws/2002/12/policy\" xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/03/addressing\" xmlns:wssc=\"http://schemas.xmlsoap.org/ws/2004/04/sc\" xmlns:wst=\"http://schemas.xmlsoap.org/ws/2004/04/trust\">"CRLF
		"<Header>"CRLF
		"<ps:AuthInfo xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" Id=\"PPAuthInfo\">"CRLF
		"<ps:HostingApp>Tailspin Toys;user@tailspintoys.com;Tailspin Toys Application</ps:HostingApp>"CRLF
		"<ps:BinaryVersion>4</ps:BinaryVersion>"CRLF
		"<ps:UIVersion>1</ps:UIVersion>"CRLF
		"<ps:Cookies></ps:Cookies>"CRLF
		"<ps:RequestParams>AgAAAAIAAABwdgMAAABwdjECAAAAbGMEAAAAMTAyOA==</ps:RequestParams>"CRLF
		"</ps:AuthInfo>"CRLF
		"<wsse:Security>"CRLF
		"<wsse:UsernameToken Id=\"user\">"CRLF
		"<wsse:Username>%s</wsse:Username>"CRLF
		"<wsse:Password>%s</wsse:Password>"CRLF
		"</wsse:UsernameToken>"CRLF
		"</wsse:Security></Header>"CRLF
		"<Body><wst:RequestSecurityToken Id=\"RST0\">"CRLF
		"<wst:RequestType>http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue</wst:RequestType>"CRLF
		"<wsp:AppliesTo><wsa:EndpointReference>"CRLF
		"<wsa:Address>http://Passport.NET/tb</wsa:Address>"CRLF
		"</wsa:EndpointReference></wsp:AppliesTo></wst:RequestSecurityToken></Body></Envelope>",email,passwd);
       
        curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); /* TODO */
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);

        //set follow location
        //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

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
		
        //windows_live_mfree(url);

        windows_live_mfree(data);

        mpDebugPrint("windows_live_ClientLogin: http resp code=%d", httpcode);
        if (httpcode == 200)
        {
        #if 0
            ptr = App_State.XML_BUF;
            if (ptr != NULL)
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                mpDebugPrint("windows_live_ClientLogin: no Auth=");
                NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
                ret = -3;
            }
		#else	
			
			/* Parse XML */
			parser = XML_ParserCreate(NULL);
			windows_live_info->state = WINDOWS_LIVE_SDK_LOGIN;

			XML_SetUserData(parser, windows_live_info);
			XML_SetElementHandler(parser, wls_start_element_handler, wls_end_element_handler);
			XML_SetCharacterDataHandler(parser, wls_content_handler);

			bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			mpDebugPrint("windows_live_id_sdk_login: got total data len = %d", len);

			if (XML_Parse(parser, bigger_buff, len, 1) == XML_STATUS_ERROR)
			{
				mpDebugPrint("windows_live_id_sdk_login: %s at line %d, column %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser),
				XML_GetCurrentColumnNumber(parser));
			}

			if (windows_live_info->error_code > 0)
			{
				ret = -NETFS_APP_ERROR;
				goto exit;
			}
            XML_ParserFree(parser);
		#endif			
        }
        //else if (httpcode > 0)                  /* TODO */
        else       // jeffery 20090320
        {
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 Access Forbidden */
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("windows_live_ClientLogin(): got total data len = %d", len);
				
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
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
            
			    data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                sprintf(errstring, "HTTP Error %d", httpcode);
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
            }
            ret = -2;
        }
    }

exit:
    //weiching added
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

}

/* 
 * Windows live Login API 
 */
int windows_live_get_login_homepage(windows_live_info_t *windows_live_info, char *email, char *passwd,char *errstring)
{
    void *curl;
    CURLcode res;
    int ret = 0;
    int len,dalen;
    char *data, *end; 
    char *url; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    //int auth_len = sizeof(windows_live_info->auth);
    struct curl_slist *header=NULL;
    char *bigger_buff=NULL; 
	XML_Parser              parser;
	
    Xml_BUFF_init(NET_RECVHTTP);	
	mpDebugPrint("windows_live_id_sdk_login %s",email);

    data = windows_live_malloc(1024);
	
    if (data == NULL)
    {
    	ret = -4; 
        goto exit;
    }

    curl = curl_easy_init();
    if(curl) 
	{
        url = "http://login.live.com/login.srf?wa=wsignin1.0&rpsnv=11&ct=1248675927&rver=5.5.4177.0&wp=MBI&wreply=http:%2F%2Fhome.spaces.live.com%2F%3FshowUnauth%3D1&lc=1028&id=73625&mkt=zh-TW";
		snprintf(data, 1024,"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		snprintf(data, 1024,"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; GTB6; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; FDM; .NET CLR 3.0.04506.648; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; IDCRL 4.200.519.1; IDCRL-cfg 6.500.12348.0; App WindowsLiveIDClientSample.exe, 4.200.519.1, Tailspin Toys;user@tailspintoys.com;Tailspin Toys Application)");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		snprintf(data, 1024,"Connection: Keep-Alive");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
        snprintf(data, 1024,"Cache-Control: no-cache");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
        snprintf(data, 1024,"X-LogDigger: logme=0");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		
        curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_ENCODING,"gzip, deflate");			 // Accept-Encoding
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);

        res = curl_easy_perform(curl);
        TaskYield();

        /* check return code of curl_easy_perform() */
        if (res != CURLE_OK)
        {
            mpDebugPrint("windows_live_get_login_homepage: curl_easy_perform failed, res = %d", res);
            httpcode = 0;
            goto Youtube_cleanup_3;
        }

        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
            httpcode = 0;

Youtube_cleanup_3:
        /* always cleanup */
        curl_easy_cleanup(curl);
		
        //windows_live_mfree(url);

        windows_live_mfree(data);

        mpDebugPrint("windows_live_get_login_homepage: http resp code=%d", httpcode);
        if (httpcode == 200)
        {
        #if 0
            ptr = App_State.XML_BUF;
            if (ptr != NULL)
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                mpDebugPrint("windows_live_get_login_homepage: no Auth=");
                NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
                ret = -3;
            }
		#else	
		data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
		windows_live_info->state = WINDOWS_LIVE_GET_HOMEPAGE;
		html_init(&wls_html_parser, windows_live_info);
		html_set_content_handler(&wls_html_parser, wls_html_content_handler);
		html_set_tag_start(&wls_html_parser, wls_html_tag_start_handler);
		html_set_tag_end(&wls_html_parser, wls_html_tag_end_handler);
		html_parse(&wls_html_parser,bigger_buff,len);
		html_exit(&wls_html_parser);
		#endif			
        }
        //else if (httpcode > 0)                  /* TODO */
        else       // jeffery 20090320
        {
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 Access Forbidden */
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("windows_live_ClientLogin(): got total data len = %d", len);
				
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
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
            
			    data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                sprintf(errstring, "HTTP Error %d", httpcode);
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
            }
            ret = -2;
        }
    }

exit:
    //weiching added
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

}

/* 
 * Windows live Login API 
 */
int windows_live_get_css(windows_live_info_t *windows_live_info, char *email, char *passwd,char *errstring)
{
    void *curl;
    CURLcode res;
    int ret = 0;
    int len,dalen;
    char *data, *end; 
    char *url; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    //int auth_len = sizeof(windows_live_info->auth);
    struct curl_slist *header=NULL;
    char *bigger_buff=NULL; 
	XML_Parser              parser;
	
    Xml_BUFF_init(NET_RECVHTTP);	
	mpDebugPrint("windows_live_get_css %s",email);

    data = windows_live_malloc(1024);
	
    url = windows_live_malloc(512);
    if (data == NULL)
    {
    	ret = -4; 
        goto exit;
    }

    curl = curl_easy_init();
    if(curl) 
	{
	    //url = "https://dev.login.live.com/SDKLogin.srf";
        //url = "http://login.live.com/login.srf?wa=wsignin1.0&rpsnv=11&ct=1248675927&rver=5.5.4177.0&wp=MBI&wreply=http:%2F%2Fhome.spaces.live.com%2F%3FshowUnauth%3D1&lc=1028&id=73625&mkt=zh-TW";
        //url = "http://home.spaces.live.com/?showunauth=1&lc=10";
		snprintf(url, 512,"http://login.live.com/pp650/%s",windows_live_info->css_url);
		
		snprintf(data, 1024,"Accept: text/css,*/*;q=0.1");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		snprintf(data, 1024,"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; GTB6; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; FDM; .NET CLR 3.0.04506.648; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; IDCRL 4.200.519.1; IDCRL-cfg 6.500.12348.0; App WindowsLiveIDClientSample.exe, 4.200.519.1, Tailspin Toys;user@tailspintoys.com;Tailspin Toys Application)");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		snprintf(data, 1024,"Connection: Keep-Alive");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
        snprintf(data, 1024,"Cache-Control: no-cache");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
        snprintf(data, 1024,"X-LogDigger: logme=0");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		
        curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_ENCODING,"gzip, deflate");			 // Accept-Encoding
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);

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
		
        windows_live_mfree(url);

        windows_live_mfree(data);

        mpDebugPrint("windows_live_ClientLogin: http resp code=%d", httpcode);
        if (httpcode == 200)
        {
        #if 1
            ptr = App_State.XML_BUF;
            if (ptr != NULL)
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                mpDebugPrint("windows_live_ClientLogin: no Auth=");
                NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
                ret = -3;
            }
		#else	
			
			/* Parse XML */
			parser = XML_ParserCreate(NULL);
			windows_live_info->state = WINDOWS_LIVE_SDK_LOGIN;

			XML_SetUserData(parser, windows_live_info);
			XML_SetElementHandler(parser, wls_start_element_handler, wls_end_element_handler);
			XML_SetCharacterDataHandler(parser, wls_content_handler);

			bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			mpDebugPrint("windows_live_id_sdk_login: got total data len = %d", len);

			if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
			{
				mpDebugPrint("windows_live_id_sdk_login: %s at line %d, column %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser),
				XML_GetCurrentColumnNumber(parser));
			}

			if (windows_live_info->error_code > 0)
			{
				ret = -NETFS_APP_ERROR;
				goto exit;
			}
			XML_Parse(parser, data, 0, 1);
		#endif			
        }
        //else if (httpcode > 0)                  /* TODO */
        else       // jeffery 20090320
        {
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 Access Forbidden */
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("windows_live_ClientLogin(): got total data len = %d", len);
				
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
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
            
			    data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                sprintf(errstring, "HTTP Error %d", httpcode);
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
            }
            ret = -2;
        }
    }

exit:
    //weiching added
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

}

/* 
 * Windows live Login API 
 */
int windows_live_get_javascript(windows_live_info_t *windows_live_info, char *email, char *passwd,char *errstring)
{
    void *curl;
    CURLcode res;
    int ret = 0;
    int len,dalen;
    char *data, *end; 
    char *url; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    //int auth_len = sizeof(windows_live_info->auth);
    struct curl_slist *header=NULL;
    char *bigger_buff=NULL; 
	XML_Parser              parser;
	
    Xml_BUFF_init(NET_RECVHTTP);	
	mpDebugPrint("windows_live_id_sdk_login %s",email);

    data = windows_live_malloc(1024);
	
    url = windows_live_malloc(512);
    if (data == NULL)
    {
    	ret = -4; 
        goto exit;
    }

    curl = curl_easy_init();
    if(curl) 
	{
        //url = "http://login.live.com/login.srf?wa=wsignin1.0&rpsnv=11&ct=1248675927&rver=5.5.4177.0&wp=MBI&wreply=http:%2F%2Fhome.spaces.live.com%2F%3FshowUnauth%3D1&lc=1028&id=73625&mkt=zh-TW";
		snprintf(url, 512,"http://login.live.com/pp650/%s",windows_live_info->javascript_url);
		snprintf(data, 1024,"Accept: */*");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		snprintf(data, 1024,"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; GTB6; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; FDM; .NET CLR 3.0.04506.648; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; IDCRL 4.200.519.1; IDCRL-cfg 6.500.12348.0; App WindowsLiveIDClientSample.exe, 4.200.519.1, Tailspin Toys;user@tailspintoys.com;Tailspin Toys Application)");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		snprintf(data, 1024,"Connection: Keep-Alive");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
        snprintf(data, 1024,"Cache-Control: no-cache");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
        snprintf(data, 1024,"X-LogDigger: logme=0");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		
        curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_ENCODING,"gzip, deflate");			 // Accept-Encoding
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);

        res = curl_easy_perform(curl);
        TaskYield();

        /* check return code of curl_easy_perform() */
        if (res != CURLE_OK)
        {
            mpDebugPrint("windows_live_get_login_homepage: curl_easy_perform failed, res = %d", res);
            httpcode = 0;
            goto Youtube_cleanup_3;
        }

        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
            httpcode = 0;

Youtube_cleanup_3:
        /* always cleanup */
        curl_easy_cleanup(curl);
		
        windows_live_mfree(url);

        windows_live_mfree(data);

        mpDebugPrint("windows_live_get_login_homepage: http resp code=%d", httpcode);
        if (httpcode == 200)
        {
        #if 1
            ptr = App_State.XML_BUF;
            if (ptr != NULL)
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                mpDebugPrint("windows_live_get_login_homepage: no Auth=");
                NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
                ret = -3;
            }
		#else	
			
			/* Parse XML */
			parser = XML_ParserCreate(NULL);
			windows_live_info->state = WINDOWS_LIVE_SDK_LOGIN;

			XML_SetUserData(parser, windows_live_info);
			XML_SetElementHandler(parser, wls_start_element_handler, wls_end_element_handler);
			XML_SetCharacterDataHandler(parser, wls_content_handler);

			bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			mpDebugPrint("windows_live_id_sdk_login: got total data len = %d", len);

			if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
			{
				mpDebugPrint("windows_live_id_sdk_login: %s at line %d, column %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser),
				XML_GetCurrentColumnNumber(parser));
			}

			if (windows_live_info->error_code > 0)
			{
				ret = -NETFS_APP_ERROR;
				goto exit;
			}
			XML_Parse(parser, data, 0, 1);
		#endif			
        }
        //else if (httpcode > 0)                  /* TODO */
        else       // jeffery 20090320
        {
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 Access Forbidden */
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("windows_live_ClientLogin(): got total data len = %d", len);
				
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
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
            
			    data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                sprintf(errstring, "HTTP Error %d", httpcode);
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
            }
            ret = -2;
        }
    }

exit:
    //weiching added
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

}

/* 
 * Windows live Login API 
 */
int windows_live_id_login(windows_live_info_t *windows_live_info, char *email, char *passwd,char *errstring)
{
    void *curl;
    CURLcode res;
    int ret = 0;
    int len;
    char *data, *end; 
    char *url; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    //int auth_len = sizeof(windows_live_info->auth);
    struct curl_slist *header=NULL;
    char *bigger_buff=NULL; 
	XML_Parser              parser;
	
    Xml_BUFF_init(NET_RECVHTTP);	
	mpDebugPrint("windows_live_id_sdk_login %s",email);

    data = windows_live_malloc(512);
	
    //url = windows_live_malloc(512);
    if (data == NULL)
    {
    	ret = -4; 
        goto exit;
    }

    curl = curl_easy_init();
    if(curl) 
	{
        url = "https://login.live.com/pp650/GetUserRealm.srf";
		
		snprintf(data, 512,"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
		data[511] = '\0';
        header = curl_slist_append(header, data);
		
		snprintf(data, 512,"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; GTB6; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; FDM; .NET CLR 3.0.04506.648; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; IDCRL 4.200.519.1; IDCRL-cfg 6.500.12348.0; App WindowsLiveIDClientSample.exe, 4.200.519.1, Tailspin Toys;user@tailspintoys.com;Tailspin Toys Application)");
		data[511] = '\0';
        header = curl_slist_append(header, data);
		
		snprintf(data, 512,"Connection: Keep-Alive");
		data[511] = '\0';
        header = curl_slist_append(header, data);
			
        snprintf(data, 512,"Cache-Control: no-cache");
		data[511] = '\0';
        header = curl_slist_append(header, data);
		
        snprintf(data,512,
			"login=mpx@livemail.tw");
       
        curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_ENCODING,	"gzip, deflate");			 // Accept-Encoding
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); /* TODO */
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);

        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);

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
		
        //windows_live_mfree(url);

        windows_live_mfree(data);

        mpDebugPrint("windows_live_ClientLogin: http resp code=%d", httpcode);
        if (httpcode == 200)
        {
        #if 1
            ptr = App_State.XML_BUF;
            if (ptr != NULL)
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                mpDebugPrint("windows_live_ClientLogin: no Auth=");
                NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
                ret = -3;
            }
		#else	
			
			/* Parse XML */
			parser = XML_ParserCreate(NULL);
			windows_live_info->state = WINDOWS_LIVE_SDK_LOGIN;

			XML_SetUserData(parser, windows_live_info);
			XML_SetElementHandler(parser, wls_start_element_handler, wls_end_element_handler);
			XML_SetCharacterDataHandler(parser, wls_content_handler);

			bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			mpDebugPrint("windows_live_id_sdk_login: got total data len = %d", len);

			if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
			{
				mpDebugPrint("windows_live_id_sdk_login: %s at line %d, column %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser),
				XML_GetCurrentColumnNumber(parser));
			}

			if (windows_live_info->error_code > 0)
			{
				ret = -NETFS_APP_ERROR;
				goto exit;
			}
			XML_Parse(parser, data, 0, 1);
		#endif			
        }
        //else if (httpcode > 0)                  /* TODO */
        else       // jeffery 20090320
        {
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 Access Forbidden */
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("windows_live_ClientLogin(): got total data len = %d", len);
				
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
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
            
			    data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                sprintf(errstring, "HTTP Error %d", httpcode);
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
            }
            ret = -2;
        }
    }

exit:
    //weiching added
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

}


/* 
 * Windows live Login API 
 */
int windows_live_id_login1(windows_live_info_t *windows_live_info, char *email, char *passwd,char *errstring)
{
    void *curl;
    CURLcode res;
    int ret = 0;
    int len;
    char *data, *end; 
    char *url; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    //int auth_len = sizeof(windows_live_info->auth);
    struct curl_slist *header=NULL;
    char *bigger_buff=NULL; 
	XML_Parser              parser;
	
    Xml_BUFF_init(NET_RECVHTTP);	
	mpDebugPrint("windows_live_id_sdk_login %s",email);

    data = windows_live_malloc(2048);
	
    //url = windows_live_malloc(512);
    if (data == NULL)
    {
    	ret = -4; 
        goto exit;
    }

    curl = curl_easy_init();
    if(curl) 
	{
        url = "https://login.live.com/ppsecure/post.srf?wa=wsignin1.0&rpsnv=11&ct=1248675650&rver=5.5.4177.0&wp=MBI&wreply=http:%2F%2Fhome.spaces.live.com%2F&lc=1028&id=73625&bk=1248680793";
		
		snprintf(data, 1024,"Accept: text/*");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		
		snprintf(data, 1024,"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; GTB6; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; FDM; .NET CLR 3.0.04506.648; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; IDCRL 4.200.519.1; IDCRL-cfg 6.500.12348.0; App WindowsLiveIDClientSample.exe, 4.200.519.1, Tailspin Toys;user@tailspintoys.com;Tailspin Toys Application)");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		
		snprintf(data, 1024,"Connection: Keep-Alive");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
			
        snprintf(data, 1024,"Cache-Control: no-cache");
		data[1023] = '\0';
        header = curl_slist_append(header, data);
		
		//snprintf(data, 1024,"Cookie: MUID=7b7eb1bdda0144838aed85b437788af9; wlidperf=throughput=10&latency=295&FR=L&ST=1248675737679; MSPPre=mpx@livemail.tw; MSPCID=00e45f070eca47e1; WLOpt=RDCache=@livemail.tw:4&credtype=1&act=[1]; MH=MSFT; NAP=V=1.9&E=848&C=MI0i8yG4Kts8kYw1IRSucfuXL6jVfzi1LmDMCZi0fmo7JN4CdO36gw&W=1; ANON=A=A7CCEFC1B241957394EDDE70FFFFFFFF&E=8a2&W=1; wlp=A|YUP8-t:a*LwsNAQ._|/twY-t:a*eKHy._; s_lastvisit=1248313558453; _wlx_affinity_state_v3.0=Q2lkLDQwMzA0Mzk3NTY1ODM3MiwsYnkxfENpZCw5MTQ4MDA0MTQ4NDQ2OTQsLGJ5Mnw=; ck-35D74141169DEFC4=633838959534398198; ck-00E45F070ECA47E1=633838960478090308; _wlx_affinity_state_v4.0=YnkyLnB2dC1zdG9yYWdlLmxpdmUuY29tfHByb3h5LWJheS5wdnQtY29udGFjdHMubXNuLmNvbSoxLDY0MjgwNzc4NjA3NTc3MDU3LDAsfDEsRTQ1RjA3MEVDQTQ3RTEsLDE=; mktstate=S=288748089&U=&E=&P=&B=zh-tw; mkt1=norm=zh-tw; MSPRequ=lt=1248675650&id=73625&co=1; MSPOK=$uuid-9bcbcb9c-feed-467b-aab0-6850dc591284$uuid-cc6d463f-dcc7-49f9-b43b-3d8410486506; CkTst=G1248675651977"
		//	);
		//data[1023] = '\0';
        //header = curl_slist_append(header, data);
		
        //snprintf(data,2048,
		//	"idsbho=1&PwdPad=IfYouAreReadingThisYouHaveToo&LoginOptions=3&CS=&FedState=&PPSX=PassportRN&type=11&login=mpx@livemail.tw&passwd=wifi12345678&NewUser=1&PPFT=B6dTafrvcQ5BnBVd6eEpJlHqkxbZJ6QTnr2H4oFuxGFRTd*fqquS0mHBerEDke2PyontHbH9ooopSpMyhsoPkgoKBu6Cq1jU0WgPVCFVy*GuQzuKmfzv%21W3oAbSMXWeGJNR18VVrtBzqbawnTs217ah7uhxOwoVYPeoiWDwk8Cba8f6NcR6rCDbAw%21ZX&i1=0i2=0"
		//	);
        memcpy(data,"idsbho=1&PwdPad=IfYouAreReadingThisYouHaveToo&LoginOptions=3&CS=&FedState=&PPSX=PassportRN&type=11&login=mpx@livemail.tw&passwd=wifi12345678&NewUser=1&PPFT=B6dTafrvcQ5BnBVd6eEpJlHqkxbZJ6QTnr2H4oFuxGFRTd*fqquS0mHBerEDke2PyontHbH9ooopSpMyhsoPkgoKBu6Cq1jU0W",sizeof("idsbho=1&PwdPad=IfYouAreReadingThisYouHaveToo&LoginOptions=3&CS=&FedState=&PPSX=PassportRN&type=11&login=mpx@livemail.tw&passwd=wifi12345678&NewUser=1&PPFT=B6dTafrvcQ5BnBVd6eEpJlHqkxbZJ6QTnr2H4oFuxGFRTd*fqquS0mHBerEDke2PyontHbH9ooopSpMyhsoPkgoKBu6Cq1jU0W"));
		mpDebugPrint("\n%d\n",sizeof("idsbho=1&PwdPad=IfYouAreReadingThisYouHaveToo&LoginOptions=3&CS=&FedState=&PPSX=PassportRN&type=11&login=mpx@livemail.tw&passwd=wifi12345678&NewUser=1&PPFT=B6dTafrvcQ5BnBVd6eEpJlHqkxbZJ6QTnr2H4oFuxGFRTd*fqquS0mHBerEDke2PyontHbH9ooopSpMyhsoPkgoKBu6Cq1jU0W"));
		mpDebugPrint("\n%d\n",sizeof("gPVCFVy*GuQzuKmfzv%21W3oAbSMXWeGJNR18VVrtBzqbawnTs217ah7uhxOwoVYPeoiWDwk8Cba8f6NcR6rCDbAw%21ZX&i1=0i2=0"));
        memcpy(data+sizeof("idsbho=1&PwdPad=IfYouAreReadingThisYouHaveToo&LoginOptions=3&CS=&FedState=&PPSX=PassportRN&type=11&login=mpx@livemail.tw&passwd=wifi12345678&NewUser=1&PPFT=B6dTafrvcQ5BnBVd6eEpJlHqkxbZJ6QTnr2H4oFuxGFRTd*fqquS0mHBerEDke2PyontHbH9ooopSpMyhsoPkgoKBu6Cq1jU0W"),"gPVCFVy*GuQzuKmfzv%21W3oAbSMXWeGJNR18VVrtBzqbawnTs217ah7uhxOwoVYPeoiWDwk8Cba8f6NcR6rCDbAw%21ZX&i1=0i2=0",sizeof("gPVCFVy*GuQzuKmfzv%21W3oAbSMXWeGJNR18VVrtBzqbawnTs217ah7uhxOwoVYPeoiWDwk8Cba8f6NcR6rCDbAw%21ZX&i1=0i2=0"));
		
        curl_easy_setopt(curl, CURLOPT_URL,url);
		curl_easy_setopt(curl, CURLOPT_ENCODING,	"gzip, deflate");			 // Accept-Encoding
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); /* TODO */
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);

        //set follow location
        //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);

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
		
        //windows_live_mfree(url);

        windows_live_mfree(data);

        mpDebugPrint("windows_live_ClientLogin: http resp code=%d", httpcode);
        if (httpcode == 200)
        {
        #if 1
            ptr = App_State.XML_BUF;
            if (ptr != NULL)
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                mpDebugPrint("windows_live_ClientLogin: no Auth=");
                NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
                ret = -3;
            }
		#else	
			
			/* Parse XML */
			parser = XML_ParserCreate(NULL);
			windows_live_info->state = WINDOWS_LIVE_SDK_LOGIN;

			XML_SetUserData(parser, windows_live_info);
			XML_SetElementHandler(parser, wls_start_element_handler, wls_end_element_handler);
			XML_SetCharacterDataHandler(parser, wls_content_handler);

			bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			mpDebugPrint("windows_live_id_sdk_login: got total data len = %d", len);

			if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
			{
				mpDebugPrint("windows_live_id_sdk_login: %s at line %d, column %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser),
				XML_GetCurrentColumnNumber(parser));
			}

			if (windows_live_info->error_code > 0)
			{
				ret = -NETFS_APP_ERROR;
				goto exit;
			}
			XML_Parse(parser, data, 0, 1);
		#endif			
        }
        //else if (httpcode > 0)                  /* TODO */
        else       // jeffery 20090320
        {
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 Access Forbidden */
            {
                data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
			    mpDebugPrint("windows_live_ClientLogin(): got total data len = %d", len);
				
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
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
            
			    data = bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
                sprintf(errstring, "HTTP Error %d", httpcode);
				NetAsciiDump(bigger_buff, len); //dump whole bigger buffer
            }
            ret = -2;
        }
    }

exit:
    //weiching added
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;

}

