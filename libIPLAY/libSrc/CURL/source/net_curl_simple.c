/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * $Id: simple.c,v 1.6 2004/08/23 14:22:52 bagder Exp $
 */
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"

#include "net_curl_setup.h"

#include <stdio.h>
#include "net_curl_curl.h"

#include <string.h>
#include "net_socket.h"


#if 1
#if  Make_LWIP
  #include "..\..\lwip\include\net_sys.h"
#else
  #include "..\..\uip\include\net_sys.h"
#endif
extern Net_App_State App_State;
size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf); //global function to replace Curl_fwrite()
size_t my_write_func2(void *ptr, size_t size, size_t nmemb, void *buf); //global function to replace Curl_fwrite()
static int prog_cb (void *p, long dltotal, long dlnow, long ult, long uln);
size_t range_write_func(void *ptr, size_t size, size_t nmemb, void *buf); //global function to replace Curl_fwrite()
size_t range_write_func1(void *ptr, size_t size, size_t nmemb, void *buf);
#endif


//U32 ImageFileBufferSize = 512*1024; //512K

//U32 LinkFileBufferSize = 64*1024; //4K

//U32 LinkBuffer = 0;

//U32 ImageBuffer = 0;
BYTE  *ImageBuffer;
U32 ImageBufferCounter = 0;

U08 Curl_imageOpened = FALSE;
struct SessionHandle *g_GCE_curl;

/* http proxy config data */
static char my_proxy_login[160];
extern char *my_proxy_host;
extern u32 my_proxy_port;
extern char *my_proxy_password;
extern char *my_proxy_username;

U32 NetDefaultIpGet();
#if 0
void OPEN_GCE_FILE()
{
	 g_GCE_curl = curl_easy_init();
	 if(!g_GCE_curl)
	 	return ;

	//set follow location
	 
	curl_easy_setopt(g_GCE_curl, CURLOPT_FOLLOWLOCATION, 1);
	// set CURLOPT_COOKIEJAR
	curl_easy_setopt(g_GCE_curl, CURLOPT_COOKIEJAR, 1);
	curl_easy_setopt(g_GCE_curl, CURLOPT_COOKIESESSION, 1);
				
	//set cert verify false
	curl_easy_setopt(g_GCE_curl, CURLOPT_SSL_VERIFYPEER, FALSE);
}


void CLOSE_GCE_FILE()
{
	/* always cleanup */
	if(!g_GCE_curl)
		return;
	  
	curl_easy_cleanup(g_GCE_curl);
	g_GCE_curl = NULL;	
}

int Get_GCE_File(BYTE* url,U08* Buffer)
{
	if(!g_GCE_curl)
		return 0;

	ImageBuffer = Buffer;
	ImageBufferCounter = 0;
	
	//set URL
	curl_easy_setopt(g_GCE_curl, CURLOPT_USERAGENT, "GCEBrowser/2.0");
        
	curl_easy_setopt(g_GCE_curl, CURLOPT_ENCODING,  "gzip, deflate");            // Accept-Encoding

	curl_easy_setopt(g_GCE_curl, CURLOPT_X_GCE_SCREEN_SIZE, "800x600");
 	curl_easy_setopt(g_GCE_curl, CURLOPT_X_GCE_BUTTONS, "Up, Down, Left, Right, OK, Esc");
  	curl_easy_setopt(g_GCE_curl, CURLOPT_X_GCE_DEVICE_ID, "Magicpixel DPF 800x600");
  	curl_easy_setopt(g_GCE_curl, CURLOPT_X_GCE_DEVICE_NICKNAME,"Magicpixel DPF " );
	curl_easy_setopt(g_GCE_curl, CURLOPT_URL, url);    		
	curl_easy_perform(g_GCE_curl);	
	
	MP_DEBUG("==============================================================");
	MP_DEBUG2(" Get_GCE_File url  = %s,  ImageBufferCounter %d",url,ImageBufferCounter);
	MP_DEBUG("==============================================================");

	return ImageBufferCounter;
}
#endif
#if USB_WIFI && (USBOTG_HOST_CDC==ENABLE) // Assume that using 3.5G dongle
#define DEFAULT_TRANSFER_TIMEOUT 1800
#else	
#define DEFAULT_TRANSFER_TIMEOUT 90
#endif
static int curl_timeout = DEFAULT_TRANSFER_TIMEOUT;  /* timeout period for transfer */

static CURLcode curl_result;                    /* result from last curl transfer */

void * __get_xml_buffer(void ** ptr,DWORD size)
{
	void * xp = *ptr;
//	DWORD size = sizeof(XML_ImageBUFF_link_t);

	if (!xp)
	{
		xp = ext_mem_malloc(size);
		if (! xp)
		{
			MP_DEBUG("Get_Image_File(): mem alloc failed!");		
                	return NULL;	
		}
		memset(xp, 0, size);
		*ptr = xp;
	}	
	return xp; 	
}


//note: here, when my_write_func() is used to replace Curl_fwrite(), global (ImageBuffer, ImageBufferCounter) pair is useless for buffer data copy!!
//      and then, argument 'Buffer' is also useless!!
int Get_Image_File(void* URL, U08* Buffer, DWORD timeout)
{
    CURL *curl;
    CURLcode res;
    size_t index;
    int expire;

    void *xml_buf_list= NULL;
    
    //allocate image buffer
    ImageBuffer = Buffer; //note: when my_write_func() is used to replace Curl_fwrite(), global ImageBuffer is useless here
    ImageBufferCounter = 0;

    App_State.dwTotallen = 0;

    if(0UL == NetDefaultIpGet())         /* detect if network is down here */
    {
        /* Don't enter CURL if network is already down. */
        curl_result = CURLE_COULDNT_CONNECT;
        return ImageBufferCounter;
    }

    curl = curl_easy_init();
    if(curl)
    {
        //set URL
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        expire = timeout ? timeout : curl_timeout;

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, expire);

        if (App_State.dwState &  NET_RECVHTTP)
        {
	     	xml_buf_list = __get_xml_buffer(&App_State.XML_BUF, sizeof(XML_BUFF_link_t));		 
        }
	 else if ((App_State.dwState & NET_RECVBINARYDATA)
				|| (App_State.dwState & NET_RECVUPNP)
        )    
	 {
	 	xml_buf_list = __get_xml_buffer(&App_State.XML_BUF1, sizeof(XML_ImageBUFF_link_t));		 
        }

	 if (! xml_buf_list)
                return 0;

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, xml_buf_list);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, prog_cb);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &App_State);

        /* support http proxy */
        if (*my_proxy_host)
        {
            int len;
//                mpDebugPrint("curl_easy_setopt(CURLOPT_PROXY) - %s", my_proxy_host);
            res = curl_easy_setopt(curl, CURLOPT_PROXY, my_proxy_host);
            if (res != CURLE_OK)
                mpDebugPrint("curl_easy_setopt(CURLOPT_PROXY) returns error - %d", res);
            res = curl_easy_setopt(curl, CURLOPT_PROXYPORT, my_proxy_port);
            if (res != CURLE_OK)
                mpDebugPrint("curl_easy_setopt(CURLOPT_PROXYPORT) returns error - %d", res);

            len = strlen(my_proxy_username) + strlen(my_proxy_password);
            if (len > 0)
            {
                sprintf(my_proxy_login, "%s:%s", my_proxy_username, my_proxy_password);
                res = curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, my_proxy_login);
                if (res != CURLE_OK)
                    mpDebugPrint("curl_easy_setopt(CURLOPT_PROXYUSERPWD) returns error - %d", res);
            }
        }

        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		//set CURLOPT_FAILONERROR 
//		if( App_State.dwState &  NET_RECVHTTP )
			curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
		
        res = curl_easy_perform(curl);
        TaskYield();
        if(res != CURLE_OK)
        {
            MP_ALERT("%s: curl_easy_perform returns %d", __func__, res);
            curl_result = res;
            ImageBufferCounter = 0;
        } else
            ImageBufferCounter = App_State.dwTotallen; //total length of buffers was already cumulated in my_write_func()

        CURLPRINTF("ImageBufferCounter %d", ImageBufferCounter);
            			
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return ImageBufferCounter;
}

int Get_Image_File2(void* URL, U08* Buffer, DWORD timeout, char *if_name, Net_App_State *app)
{
    CURL *curl;
    size_t index;
    int expire;
    U32 tot;
    int httpcode;

    //allocate image buffer
    ImageBuffer = Buffer; //note: when my_write_func() is used to replace Curl_fwrite(), global ImageBuffer is useless here
    tot = 0;

    app->dwTotallen = 0;

    if(if_name && !NetInterfaceIpGet(if_name))         /* detect if network is down here */
    {
        curl_result = CURLE_INTERFACE_FAILED;
        return 0;
    }

    curl_result = CURLE_FAILED_INIT;

    curl = curl_easy_init();
    if(curl)
    {

        //set proxy
        //curl_easy_setopt(curl, CURLOPT_PROXYTYPE, 0);
        //curl_easy_setopt(curl, CURLOPT_PROXY, "192.168.47.10");
        //curl_easy_setopt(curl, CURLOPT_PROXYPORT, 8002);
		//curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, "USERNAME:PASSWORD");


        //set URL
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        expire = timeout ? timeout : curl_timeout;

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, expire);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, app);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func2);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, prog_cb);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, app);

        //set interface to use
		if (if_name)
			curl_easy_setopt(curl, CURLOPT_INTERFACE, if_name);

        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		//set CURLOPT_FAILONERROR 
//		if( app->dwState &  NET_RECVHTTP )
			curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
		
        curl_result = curl_easy_perform(curl);
        TaskYield();
        if(curl_result != CURLE_OK)
        {
            MP_ALERT("%s: curl_easy_perform returns %d", __func__, curl_result);
            log_printf(1, "%s: curl_easy_perform returns %d", __func__, curl_result);
//            ntrace_printf(1, "[HTTP] curl_easy_perform returns %d", curl_result);
            if (curl_result == CURLE_GOT_NOTHING)
            {
                if (app->XML_BUF1 && app->dwTotallen > 0)
                {
//                    ntrace_printf(1, "[HTTP] 052: HTTP RESP(%u)=", app->dwTotallen);
                    ntrace_dump(1, &app->XML_BUF1->BUFF[0], MIN(128,app->dwTotallen));
                    log_printf(1, "052: HTTP RESP(%u)=%128s", app->dwTotallen,
                            &app->XML_BUF1->BUFF[0]);
                    NetAsciiDump(&app->XML_BUF1->BUFF[0], app->dwTotallen);
                }
            }
            else if (curl_result == CURLE_HTTP_RETURNED_ERROR)
            {
                if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
                    httpcode = 0;
                if (httpcode != 200)
                {
                    MP_ALERT("%s: server returns HTTP code %d", __func__, httpcode);
                    log_printf(1, "%s: server returns HTTP code=%d", __func__, httpcode);
//                    ntrace_printf(1, "[HTTP] server returns HTTP code=%d", httpcode);
                    if (app->XML_BUF1 && app->dwTotallen > 0)
                    {
//                        ntrace_printf(1, "[HTTP] HTTP RESP(%u)=", app->dwTotallen);
                        ntrace_dump(1, &app->XML_BUF1->BUFF[0], MIN(128,app->dwTotallen));
                        log_printf(1, "HTTP RESP(%u)=%128s", app->dwTotallen,
                                &app->XML_BUF1->BUFF[0]);
                        NetAsciiDump(&app->XML_BUF1->BUFF[0], app->dwTotallen);
                    }
                    curl_result = httpcode;
                }
            }
            tot = 0;
        }
	 else
        {
            if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
                httpcode = 0;
            if (httpcode != 200)
            {
                MP_ALERT("%s: server returns HTTP code %d", __func__, httpcode);
                log_printf(1, "%s: server returns HTTP code=%d", __func__, httpcode);
            }
            tot = app->dwTotallen; //total length of buffers was already cumulated in my_write_func()
        }

        CURLPRINTF("total received %d", tot);
            			
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return tot;
}

//note: This global function is modified and moved from static my_write_func() in xmlpicasa2.c
//      We use my_write_func() to replace Curl_fwrite() callback function when HAVE_CURL is enabled
size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf)
{
    XML_BUFF_link_t *xp = buf;
    size_t len = size * nmemb;
    size_t write_len;		
    while(xp && xp->link)
        xp = xp->link;

    if (len > 0)
    {
        /* check data length to avoid buffer overflow */
        if (len <= IMAGE_BUF - xp->buff_len)
        {
			memcpy(&xp->BUFF[xp->buff_len], ptr, len);
            write_len = len;
            xp->buff_len += write_len;
            //MP_DEBUG("my_write_func(): write (%d) bytes into buffer", write_len);
            //cj add for check get file size from network 020409
            App_State.dwTotallen += len;	
#if MAKE_XPG_PLAYER
	     NetListProcess(App_State.dwTotallen, 0, 0, 2);		
#endif
	     MP_DEBUG1("1 my_write_func(): write App_State.dwTotallen (%d) ", App_State.dwTotallen); 
        }
        else
        {
            XML_BUFF_link_t *extra_xp;
            size_t extra_len;
                
            //MP_ALERT("my_write_func: data length exceeds left buffer space"); //cj modify
		MP_DEBUG("my_write_func: data length exceeds left buffer space");
		extra_xp = (XML_BUFF_link_t *)ext_mem_malloc(sizeof(XML_BUFF_link_t));
            if (!extra_xp)
            {
                //MP_ALERT("my_write_func(): cannot allocate memory => discard such big data!"); //cj modify
		MP_DEBUG("my_write_func(): cannot allocate memory => discard such big data!");
		return 0;
            } else
                memset(extra_xp, 0, sizeof(XML_BUFF_link_t));

				memcpy(&xp->BUFF[xp->buff_len], ptr, IMAGE_BUF - xp->buff_len);
            write_len = IMAGE_BUF - xp->buff_len;
            extra_len = len - write_len;
            xp->buff_len = IMAGE_BUF;
            xp->link = extra_xp;    	      	
	     //cj add for check get file size from network 020409
#if MAKE_XPG_PLAYER
	     NetListProcess(App_State.dwTotallen, 0, 0, 2);
#endif
	     App_State.dwTotallen += write_len;		
	     MP_DEBUG1("2 my_write_func(): write App_State.dwTotallen (%d) ", App_State.dwTotallen); 	
	     write_len += my_write_func(((BYTE *)ptr) + write_len, 1, extra_len, extra_xp);
	 }
		
    }
    //mpDebugPrint("1 my_write_func(): write (%d) bytes into buffer.", write_len);
    return write_len;
}

size_t my_write_func2(void *ptr, size_t size, size_t nmemb, void *buf)
{
    XML_BUFF_link_t *xp;
    size_t len = size * nmemb;
    size_t write_len;		
    Net_App_State *app = buf;                   /* a net application handle */

	if (!app)
		return 0;

	if (app->dwState &  NET_RECVHTTP)
        xp = app->XML_BUF;
	else
        xp = app->XML_BUF1;

	MP_ASSERT(xp);

    while(xp && xp->link)
        xp = xp->link;

    if (len > 0)
    {
        /* check data length to avoid buffer overflow */
        if (len <= (IMAGE_BUF - xp->buff_len) )
        {
            memcpy(&xp->BUFF[xp->buff_len], ptr, len);
            write_len = len;
            xp->buff_len += write_len;
            //MP_DEBUG("my_write_func(): write (%d) bytes into buffer", write_len);
            //cj add for check get file size from network 020409
            app->dwTotallen += len;	
#if MAKE_XPG_PLAYER
			NetListProcess(app->dwTotallen, 0, 0, 2);		
#endif
			MP_DEBUG1("1 my_write_func(): write app->dwTotallen (%d) ", app->dwTotallen); 
        }
        else
        {
            XML_BUFF_link_t *extra_xp;
            size_t extra_len;
                
            //MP_ALERT("my_write_func: data length exceeds left buffer space"); //cj modify
		MP_DEBUG("my_write_func: data length exceeds left buffer space");
		extra_xp = (XML_BUFF_link_t *)ext_mem_malloc(sizeof(XML_BUFF_link_t));
            if (!extra_xp)
            {
                //MP_ALERT("my_write_func(): cannot allocate memory => discard such big data!"); //cj modify
		MP_DEBUG("my_write_func(): cannot allocate memory => discard such big data!");
		return 0;
            } else
                memset(extra_xp, 0, sizeof(XML_BUFF_link_t));

			mpDebugPrint("IMAGE_BUF %d",IMAGE_BUF);
            memcpy(&xp->BUFF[xp->buff_len], ptr, IMAGE_BUF - xp->buff_len);
            write_len = IMAGE_BUF - xp->buff_len;
            extra_len = len - write_len;
            xp->buff_len = IMAGE_BUF;
            xp->link = extra_xp;    	      	
	     //cj add for check get file size from network 020409
#if MAKE_XPG_PLAYER
	     NetListProcess(app->dwTotallen, 0, 0, 2);
#endif
	     app->dwTotallen += write_len;		
	     MP_DEBUG1("2 my_write_func(): write app->dwTotallen (%d) ", app->dwTotallen); 	
	     write_len += my_write_func(((BYTE *)ptr) + write_len, 1, extra_len, extra_xp);
	 }
		
    }
    //mpDebugPrint("1 my_write_func(): write (%d) bytes into buffer.", write_len);
    return write_len;
}

//Kevin Add for partial read file
extern int gcurl_filesize;

size_t range_write_func(void *ptr, size_t size, size_t nmemb, void *buf)
{
    size_t len = size * nmemb;
    size_t write_len;		

    if (len > 0)
    {
        /* check data length to avoid buffer overflow */
        {
			//mpDebugPrint("range_write_func %p len %d %d",buf,len,App_State.dwTotallen);
			memcpy(ImageBuffer+App_State.dwTotallen, ptr, len);
            write_len = len;
            App_State.dwTotallen += len;
#if MAKE_XPG_PLAYER
			NetListProcess(App_State.dwOffset+App_State.dwTotallen, 0, 0, 2);
#endif
			MP_DEBUG1("1 my_write_func(): write App_State.dwTotallen (%d) ", App_State.dwTotallen); 
        }
		
    }
    //mpDebugPrint("1 my_write_func(): write (%d) bytes into buffer.", write_len);
    return write_len;
}

size_t range_write_func1(void *ptr, size_t size, size_t nmemb, void *buf)
{
    size_t len = size * nmemb;
    size_t write_len;		

    if (len > 0)
    {
        /* check data length to avoid buffer overflow */
        {
			//mpDebugPrint("range_write_func1 %p %p len %d",buf,ptr,len);
			//NetAsciiDump(ptr,len);
			if(!strncasecmp(ptr,"Location: ",10))
               memcpy(buf, ptr, len);
            write_len = len;
			buf+= len;
			MP_DEBUG1("1 my_write_func(): write App_State.dwTotallen (%d) ", App_State.dwTotallen); 
        }
		
    }
    //mpDebugPrint("1 my_write_func(): write (%d) bytes into buffer.", write_len);
    return write_len;
}

size_t curl_get_header(char *buffer,size_t size,size_t nitems,size_t outstream)
{ 
	char *ptr;
	char *dptr = (char *) outstream;
	//mpDebugPrint("size %d %d",size,nitems);
	//mpDebugPrint("curl_get_header %s",buffer);
	if( strstr(buffer,"CONTENT-RANGE:") || strstr(buffer,"Content-Range:"))
	{
		if( ptr = strstr(buffer,"/") )
		{
			ptr++;
			gcurl_filesize  = strtol(ptr,NULL,10);
			mpDebugPrint("gcurl_filesize %d",gcurl_filesize);
		}
	}
	return size*nitems;
}

int Get_Image_File_Range(void* URL, U08* Buffer,int start,int end,char bgetfilesize)
{
    CURL *curl;
    CURLcode res;
    size_t index;
	char parmbuf[64];


    //allocate image buffer
    ImageBuffer = Buffer; //note: when my_write_func() is used to replace Curl_fwrite(), global ImageBuffer is useless here
    ImageBufferCounter = 0;

    App_State.dwTotallen = 0;
	App_State.dwOffset = start;
    if(0UL == NetDefaultIpGet())         /* detect if network is down here */
    {
        /* Don't enter CURL if network is already down. */
        curl_result = CURLE_COULDNT_CONNECT;
        return ImageBufferCounter;
    }

    curl = curl_easy_init();
    if(curl)
    {
        //set URL
        curl_easy_setopt(curl, CURLOPT_URL, URL);
		//Kevin add to set total transation timeout to 30 seconds
		Set_Recv_Data_Timeout(30);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, curl_timeout);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, ImageBuffer);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, range_write_func);

        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		//Kevin add for get range
		sprintf(parmbuf,"%d-%d",start,end);
        curl_easy_setopt(curl, CURLOPT_RANGE, parmbuf);	//256*1024 "0-262143" "0-8192"
		if( bgetfilesize )
		{
			gcurl_filesize = 0;
			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_get_header );
		}
		//End Kevin		
        res = curl_easy_perform(curl);
        TaskYield();
        if(res != CURLE_OK)
        {
            MP_ALERT("%s: curl_easy_perform returns %d", __func__, res);
            curl_result = res;
            ImageBufferCounter = 0;
        } else
            ImageBufferCounter = App_State.dwTotallen; //total length of buffers was already cumulated in my_write_func()

        CURLPRINTF("ImageBufferCounter %d", ImageBufferCounter);
            			
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return ImageBufferCounter;
}


//End Kevin Add for partial read file

//note: we treat XML_BUFF_link_t and XML_ImageBUFF_link_t as same type
BYTE *Merge_ListData_to_SingleBuffer(XML_BUFF_link_t *list_head, int *total_data_len)
{
    char *bigger_buff;
    int total_len=0, curr_buff_len=0;
    XML_BUFF_link_t *ptr;

    if(list_head == NULL)
    {
        MP_DEBUG("Merge_ListData_to_SingleBuffer(): warning: input empty list!");
        *total_data_len = 0;
        return NULL;
    }

    ptr = list_head;
    while (ptr != NULL)
    {
        total_len += ptr->buff_len;
        ptr = ptr->link;
    }

    bigger_buff = ext_mem_malloc(total_len+1);
    if (bigger_buff == NULL)
    {
        //MP_ALERT("Merge_ListData_to_SingleBuffer(): mem alloc (%d) bytes failed!", total_len); //cj modify
	MP_DEBUG1("Merge_ListData_to_SingleBuffer(): mem alloc (%d) bytes failed!", total_len);	
        *total_data_len = 0;
        return NULL;
    }

    ptr = list_head;
    while (ptr != NULL)
    {
    
        mmcp_memcpy((char *)bigger_buff + curr_buff_len, ptr->BUFF, ptr->buff_len);
        curr_buff_len += ptr->buff_len;
        ptr = ptr->link;
    }
    MP_DEBUG("Merge_ListData_to_SingleBuffer : write (%d) bytes into buffer.", total_len);
    *total_data_len = total_len;
    bigger_buff[total_len] = '\0';
    return bigger_buff;
}

/*
 * Get_Recv_Data_Errno
 *
 * Get specific error code for last Net_Recv_Data call.
 * CURL's error codes are converted to error code defined in <errno.h>.
 * This function works only with CURL enabled.
 *
 * @retval  
 *              -ENETDOWN        Network subsystem is down.  Any active slide 
 *                               show should be stopped.
 *              -EHOSTUNREACH    Can't connect to the host.
 *              -ETIME           The timeout time was reached.
 *              -ENOBUFS         Out of memory
 *              -200             Others
 *              -201             Partial file
 */
int Get_Recv_Data_Errno(void)
{
    int r;
    switch (curl_result)
    {
        case CURLE_COULDNT_CONNECT:
            if(0 == NetDefaultIpGet())
                r = -ENETDOWN;
            else
                r = -EHOSTUNREACH;
            break;
        case CURLE_OPERATION_TIMEOUTED:
            r = -ETIME;
            break;
        case CURLE_OUT_OF_MEMORY:
            r = -ENOBUFS;
            break;
        case CURLE_PARTIAL_FILE:
            r = -201;
            break;
        case 0:
            r = 0;
            break;
        default:
            r = -200;
            break;
    }

    return r;
}

int Get_Curl_Result(void)
{
    return curl_result;
}

/*
 * Set_Recv_Data_Timeout
 *
 * Set HTTP transfer default timeout period, in number of seconds.
 * This function works only with CURL enabled.
 */
void Set_Recv_Data_Timeout(int secs)
{
    curl_timeout = secs;
}

//int data_address[2048 * 1024];
//int index = 0;

int my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
	CURLPRINTF("Write");
	
  //memcpy(data_address[index], buffer , size * nmemb);
  //index += size * nmemb;
#if 1
  STREAM* handle = stream;
  int ret;
  
  ret = FileWrite(handle,buffer,size*nmemb);
#endif
  return size * nmemb;
}


int Get_Ftp_File(void)
{
  CURL *curl;
  CURLcode res;

#if 1
	  BYTE name[32];
	  int CurId = -1;
	  STREAM* handle = NULL;
#endif


  curl = curl_easy_init();
  if(curl) {
    /*
     * Get curl 7.9.2 from sunet.se's FTP site. curl 7.9.2 is most likely not
     * present there by the time you read this, so you'd better replace the
     * URL with one that works!
     */
    curl_easy_setopt(curl, CURLOPT_URL,
                     "ftp://192.168.0.101/success.txt");

#if 1
	memset(name, '\0', 32);
	strcpy(name, "success.txt");
	MP_DEBUG("name = %s", name);
	handle = OpenFile(name, &CurId);
	if (! handle)
    {
		MP_DEBUG("File Open Fail");
		MP_ASSERT(0);
	}
#endif

	/* set username and password */
	curl_easy_setopt(curl, CURLOPT_USERPWD, "mpx:");
	
    /* Define our callback to get called when there's data to be written */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
#if 1
    /* Set a pointer to our struct to pass to the callback */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, handle);
#endif
    /* Switch on full protocol/debug output */
   // curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);

    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);

    if(CURLE_OK != res) {
      /* we failed */
      curl_result = res;
      fprintf(stderr, "curl told us %d\n", res);
    }
  }

#if 1
  CloseFile(handle,CurId);
#endif


  //curl_global_cleanup();

  return 0;
}

/* CURLOPT_PROGRESSFUNCTION */
/* 
 * According to CURL doc, this callback is called once every second.
 */
static int prog_cb (void *p, long dltotal, long dlnow, long ult, long uln)
{
  Net_App_State *conn = (Net_App_State *)p;
  if (conn->flags & HTTP_TRANSFER_CANCEL)
  {
      conn->flags = 0;
      return -1;                                /* terminate the transfer */
  }
  else
  {
#if HAVE_CDPF 
      extern u_long		ppp_last_recv;
      if (conn->flags & HTTP_TRANSFER_PPP)
      {
		ppp_last_recv = SystemGetTimeStamp();
		if (!NetInterfaceIpGet("ppp0"))         /* detect if network is down here */
			return -1;                          /* terminate the transfer */
      }
#endif

#if 0
#if HAVE_CDPF			
	if ( ! (conn->flags & HTTP_TRANSFER_PPP))
#endif
	{
		mpDebugPrint("KK check");		
		if( !NetDevicePresent() || !NetConnected() )
		{
   		//	TimerDelay(1000); 
    			conn->flags = 0;
      			return -1;                                /* terminate the transfer */
  		}	
	}		
#endif  	   
       return 0;

  }

}

BOOL HttpTransfer_CancelByUser(void)
{
	return (curl_result == CURLE_ABORTED_BY_CALLBACK) ? TRUE : FALSE;
}

BOOL HttpTransfer_CouldntConnect(void)
{
	return (curl_result == CURLE_COULDNT_CONNECT) ? TRUE : FALSE;
}

BOOL HttpTransfer_Timedout(void)
{
	return (curl_result == CURLE_OPERATION_TIMEOUTED) ? TRUE : FALSE;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
static char * g_str_if_name = "wlan0";
char * if_name_get()
{
	return g_str_if_name;	
}

void if_name_set(const char * if_name)
{
	g_str_if_name = if_name;
}

void if_name_clear()
{
	
	g_str_if_name = NULL;
}
// ---------------------------------------------------------------------
void if_name_set_wlan0()
{
	g_str_if_name = "wlan0";
}

void if_name_set_ppp0()
{
	g_str_if_name = "ppp0";
}

// ---------------------------------------------------------------------
int mpx_curl_excute(const char * url,
						const char * data, 
						const char * pHeader1, 
						const char * pHeader2)
{
	void *curl;
	CURLcode res;
	int httpcode = 0;
	struct curl_slist *header=NULL;	

	char * if_name = g_str_if_name;
	
#if 0
	if(if_name && !NetInterfaceIpGet(if_name))         /* detect if network is down here */
    	{
        	curl_result = 
        	g_curl_result = CURLE_INTERFACE_FAILED;
        	return 0;
    	}
    	curl_result = 0;
#endif
		
	curl = curl_easy_init();

	if (! curl)
	{
		mpDebugPrint("Init curl fail");
		return 0;
	}
	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	if (pHeader1)	
		header = curl_slist_append(header, pHeader1);

	if (pHeader2)	
		header = curl_slist_append(header, pHeader2);

	if (header)
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

	 if (data)
	       curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	 
       curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
       curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
       curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); /* TODO */
#if 0
	//set interface to use
	if (if_name)
		curl_easy_setopt(curl, CURLOPT_INTERFACE, if_name);
#endif

       //set follow location
       curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

       res = curl_easy_perform(curl);
       //NetAsciiDump(girbuf, bufindex);
       TaskYield();

	if (res == CURLE_OK)
	{
       	if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
       		httpcode = 0;
	}
	else
	{
		mpDebugPrint("curl_easy_perform fail, res = %d", res);
	}
	
       /* always cleanup */
       curl_easy_cleanup(curl);

	curl_slist_free_all(header);
	
	return httpcode;
}

int mpx_easy_curl(const char * url, const char * data) 
					
{
	void *curl;
	CURLcode res;
	int httpcode = 0;
	struct curl_slist *header=NULL;	

	char * if_name = g_str_if_name;

#if 0
	if(if_name && !NetInterfaceIpGet(if_name))         /* detect if network is down here */
    	{
        	curl_result = 
        	g_curl_result = CURLE_INTERFACE_FAILED;
        	return 0;
    	}
    	curl_result = 0;
#endif		
	
	curl = curl_easy_init();

	if (! curl)
	{
		mpDebugPrint("Init curl fail");
		return 0;
	}
	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	
       curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
       curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
       curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
       curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); /* TODO */
#if 0
	//set interface to use
	if (if_name)
		curl_easy_setopt(curl, CURLOPT_INTERFACE, if_name);
#endif

       //set follow location
       curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

       /* support http proxy */
       if (*my_proxy_host)
       {
            int len;
//                mpDebugPrint("curl_easy_setopt(CURLOPT_PROXY) - %s", my_proxy_host);
            res = curl_easy_setopt(curl, CURLOPT_PROXY, my_proxy_host);
            if (res != CURLE_OK)
                mpDebugPrint("curl_easy_setopt(CURLOPT_PROXY) returns error - %d", res);
            res = curl_easy_setopt(curl, CURLOPT_PROXYPORT, my_proxy_port);
            if (res != CURLE_OK)
                mpDebugPrint("curl_easy_setopt(CURLOPT_PROXYPORT) returns error - %d", res);

            len = strlen(my_proxy_username) + strlen(my_proxy_password);
            if (len > 0)
            {
                sprintf(my_proxy_login, "%s:%s", my_proxy_username, my_proxy_password);
                res = curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, my_proxy_login);
                if (res != CURLE_OK)
                    mpDebugPrint("curl_easy_setopt(CURLOPT_PROXYUSERPWD) returns error - %d", res);
            }
       }

       res = curl_easy_perform(curl);
       //NetAsciiDump(girbuf, bufindex);
       TaskYield();

       if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
       	httpcode = 0;

       /* always cleanup */
       curl_easy_cleanup(curl);
			
	return httpcode;
}

//============================================================
