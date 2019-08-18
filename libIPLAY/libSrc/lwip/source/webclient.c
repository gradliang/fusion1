/**
 * \addtogroup apps
 * @{
 */

/**
 * \defgroup webclient Web client
 * @{
 *
 * This example shows a HTTP client that is able to download web pages
 * and files from web servers. It requires a number of callback
 * functions to be implemented by the module that utilizes the code:
 * webclient_datahandler(), webclient_connected(),
 * webclient_timedout(), webclient_aborted(), webclient_closed().
 */

/**
 * \file
 * Implementation of the HTTP client.
 * \author Adam Dunkels <adam@dunkels.com>
 */

/*
 * Copyright (c) 2002, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: webclient.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 *
 */


// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE 0 

#include <string.h>
#include <sys/time.h>
#include "linux/types.h"
#include "typedef.h"
#include "socket.h"
#include "net_ns.h"
#include "net_netdb.h"
#include "global612.h"
#include "mpTrace.h"

#include "uip.h"
#include "uiplib.h"
#include "net_sys.h"
#include "webclient-strings.h"
#include "netware.h"
#include "..\..\xml\include\netfs.h"
//#include "resolv.h"

//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>

#define WEBCLIENT_TIMEOUT 32				// abel 20070928

#define WEBCLIENT_STATE_STATUSLINE 0
#define WEBCLIENT_STATE_HEADERS    1
#define WEBCLIENT_STATE_DATA       2
#define WEBCLIENT_STATE_CLOSE      3

#define HTTPFLAG_NONE   0
#define HTTPFLAG_OK     1
#define HTTPFLAG_MOVED  2
#define HTTPFLAG_ERROR  3


#define ISO_nl       0x0a
#define ISO_cr       0x0d
#define ISO_space    0x20

#define REQBUF_LEN 1024

static web_client_state s;
extern Net_App_State App_State;

/*-----------------------------------------------------------------------------------*/
char *webclient_mimetype(void)
{
  	return s.mimetype;
}

/*-----------------------------------------------------------------------------------*/
char *webclient_filename(void)
{
  	return s.file;
}


/*-----------------------------------------------------------------------------------*/
char * webclient_hostname(void)
{
  	return s.host;
}


/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
void webclient_init(void)
{

}


/*-----------------------------------------------------------------------------------*/
void init_connection(BYTE *req)
{
	char request[REQBUF_LEN];

	s.state = WEBCLIENT_STATE_STATUSLINE;

	memset(request,0x00,REQBUF_LEN);   
#if 0
	if(App_State.dwState & NET_RECVGCE)
	{
		 snprintf(request, REQBUF_LEN,	
				"GET %s HTTP/1.1\r\n"
		 		"Accept: image/jpeg, image/gif, image/png\r\n"
			 	"x-GCE-Vendor-Profile: MajorLeagueBaseball;Game Schedule\r\n"
				"x-GCE-Device-ID: 0a:20:35:18:26:ab\r\n"
				"x-GCE-Device-Type: Google 30 Digital Photo Frame\r\n"
				"x-GCE-Device-Nickname: Grandma's Photo Frame\r\n"
				"x-GCE-Screen-Size: 800x600\r\n"
				"x-GCE-Buttons: Up, Down, Left, Right, OK, Esc, TouchScreen\r\n"
				"Accept-Encoding: gzip, deflate\r\n"
		  		"User-Agent: GCEBrowser/2.0\r\n"
		 		"Host: www.halfbrowser.com\r\n"
				"Connection: close\r\n"
		 		"Cookie: GCE-Authentication=d996797806a1414fac73d9be5145735608CA62689075429008CA626890754290\r\n"
		  		"\r\n",
		  		s.file);		  		
	}
	else
#endif		
	{
		 snprintf(request, REQBUF_LEN,
                "GET %s HTTP/1.1\r\n"
                "Connection: close\r\n"
                "Host: %s\r\n"                
		  "\r\n",
                s.file,
                s.host);
	}
       s.getrequestleft = strlen(request);
       MP_DEBUG1("length = %d",s.getrequestleft);
	s.getrequestptr = 0;
  	s.httpheaderlineptr = 0;
	MP_DEBUG1("init_connection = %s ",request);   

}


/*-----------------------------------------------------------------------------------*/
void webclient_close(void)
{
 	s.state = WEBCLIENT_STATE_CLOSE;
}


/*-----------------------------------------------------------------------------------*/
unsigned char webclient_get(char *host, u16_t port, char *file, DWORD start, DWORD end)
{
  	uip_conn *conn;
  	DWORD addr;
  	BYTE i;
    struct hostent *h = NULL;
  	DWORD tmp;
	DWORD ip;
	BYTE *str;
    int ret;
	unsigned char retrycount = 0;
  
  	/* First check if the host is an IP address. */
       //MP_DEBUG5("webclient_get >> Host:%s Port: %d Path: %s start: 0x%x end: 0x%x\n", host,port,file,start,end);
	MP_DEBUG3("webclient_get >> Path: %s start: 0x%x end: 0x%x\n",file,start,end);
	
  	if((addr=inet_addr(host)) == INADDR_NONE) 
    {
        struct in_addr *curr;
        MP_DEBUG1("gethostbyname >> Host: %s\n",host);
        h = gethostbyname(host);
        MP_DEBUG1("gethostbyname returns %x\n",(U32)h);
        if(!h)
            return 0;

        if (curr = (struct in_addr *)h->h_addr_list[0])
            addr = ntohl(curr->s_addr);
    }
retry_connect:
	MP_DEBUG("webclient_get 1");
    ret = mpx_DoConnect(addr, port, TRUE); /* use blocking socket */
	MP_DEBUG1("webclient_get mpx_DoConnect returns %d",ret);
  	if(ret <= 0) 
  	{	
		retrycount++;
		if( retrycount <= 3)
			goto retry_connect;
        return 0;
  	}

  		s.port = port;
  		strncpy(s.file, file, sizeof(s.file));
  		memset(s.host,0x00,256);
		snprintf(s.host,256,"%s:%d",host,port);
		//snprintf(s.host,256,"%s",host);
  		s.dwStart = start;
  		s.dwEnd = end;

       	MP_DEBUG1("s.host = %s",s.host);
  		//init_connection();  //cj move to 
  	return ret;
}
	
/*-----------------------------------------------------------------------------------*/
void webclient_senddata(int sid, BYTE *req)
{
    int len, ret;
    char request[REQBUF_LEN];
    char start[16];
    char end[16];

    request[REQBUF_LEN-1] = '\0';
#if 0
    if(App_State.dwState & NET_RECVGCE)
      {
		if(s.getrequestleft > 0) 
  		{
  			  len = snprintf(request, REQBUF_LEN,	
				"GET %s HTTP/1.1\r\n"
		 		"Accept: image/jpeg, image/gif, image/png\r\n"
			 	"x-GCE-Vendor-Profile: MajorLeagueBaseball;Game Schedule\r\n"
				"x-GCE-Device-ID: 0a:20:35:18:26:ab\r\n"
				"x-GCE-Device-Type: Google 30 Digital Photo Frame\r\n"
				"x-GCE-Device-Nickname: Grandma's Photo Frame\r\n"
				"x-GCE-Screen-Size: 800x600\r\n"
				"x-GCE-Buttons: Up, Down, Left, Right, OK, Esc, TouchScreen\r\n"
				"Accept-Encoding: gzip, deflate\r\n"
		  		"User-Agent: GCEBrowser/2.0\r\n"
		 		"Host: www.halfbrowser.com\r\n"
				"Connection: close\r\n"
		 		"Cookie: GCE-Authentication=d996797806a1414fac73d9be5145735608CA62689075429008CA626890754290\r\n"
		  		"\r\n",
		  		s.file);
			//MP_DEBUG1("111 senddata = %s ",request);
      		}
    }   
    else
#endif		
    {
        if(s.dwEnd != 0)		// fill in range
        {
            memset(start,0x00,16);
            sprintf(start,"%d",s.dwStart);
            memset(end,0x00,16);
            sprintf(end,"%d",s.dwEnd);

            len = snprintf(request, REQBUF_LEN,
                    "GET %s HTTP/1.0\r\n"
                    "Connection: close\r\n"
                    "Host: %s\r\n"
                    "Range: bytes=%s-%s\r\n"             		       	
                    "\r\n",	
                    s.file,
                    s.host,
                    start,
                    end	); 
        }			
        else
        {
            len = snprintf(request, REQBUF_LEN,
                    "GET %s HTTP/1.0\r\n"
                    "Connection: close\r\n"
                    "Host: %s\r\n"                		      	
                    "\r\n",	
                    s.file,
                    s.host);
        } 
        MP_DEBUG1("senddata = %s ",request);
        if (len == REQBUF_LEN || len < 0)
        {
            DPrintf("webclient_senddata: string is truncated");
            BREAK_POINT();
        }
    }	

    ret = send(sid, request, len, 0);

    if (ret < 0)
    {
        MP_DEBUG2("webclient_senddata: send(%d) returns err=%d", sid,getErrno());
    }
}


/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
static u16_t parse_statusline(u16_t len)
{
  	char *cptr;

  while(len > 0 && s.httpheaderlineptr < sizeof(s.httpheaderline)) 
  {
    	s.httpheaderline[s.httpheaderlineptr] = *(char *)uip_appdata;
    	++((char *)uip_appdata);
    	--len;
    	if(s.httpheaderline[s.httpheaderlineptr] == ISO_nl) 
    	{

      		if((strncmp(s.httpheaderline, http_10,
		  			sizeof(http_10) - 1) == 0) ||
	 			(strncmp(s.httpheaderline, http_11,
		  		sizeof(http_11) - 1) == 0)) 
		  	{
				cptr = &(s.httpheaderline[9]);
				s.httpflag = HTTPFLAG_NONE;
				if(strncmp(cptr, http_200, sizeof(http_200) - 1) == 0) 
				{
	  				/* 200 OK */
	  				s.httpflag = HTTPFLAG_OK;
				} 
				else if(strncmp(cptr, http_301, sizeof(http_301) - 1) == 0 ||
		  			strncmp(cptr, http_302, sizeof(http_302) - 1) == 0) 
		  		{
	  				/* 301 Moved permanently or 302 Found. Location: header line
	     			will contain thw new location. */
	  				s.httpflag = HTTPFLAG_MOVED;
				} 
				else 
				{
	  				s.httpheaderline[s.httpheaderlineptr - 1] = 0;
				}
      		} 
      		else 
      		{
				webclient_aborted();
				return 0;
      		}
      
      		/* We're done parsing the status line, so we reset the pointer
	 		and start parsing the HTTP headers.*/
      		s.httpheaderlineptr = 0;
      		s.state = WEBCLIENT_STATE_HEADERS;
      		break;
    	} 
    	else 
    	{
      		++s.httpheaderlineptr;
    	}
  }
  	
  	return len;
}


/*-----------------------------------------------------------------------------------*/
static char casecmp(char *str1, const char *str2, char len)
{
  	static char c;
  
  	while(len > 0) 
  	{
    	c = *str1;
    	/* Force lower-case characters. */
    	if(c & 0x40) 
    	{
      		c |= 0x20;
    	}
    	
    	if(*str2 != c) 
    	{
      		return 1;
    	}
    	
    	++str1;
    	++str2;
    	--len;
  	}
  	
  	return 0;
}


/*-----------------------------------------------------------------------------------*/

static u16_t parse_headers(u16_t len)
{
  	char *cptr;
  	static unsigned char i;
	BYTE BUFF[16];
	BYTE *str;
	int index;

       App_State.HttpStart = 0;
	App_State.HttpEnd = 0;
	App_State.HttpContentLength = 0;
	App_State.HttpFileSize =0;
	  	
  	while(len > 0 && s.httpheaderlineptr < sizeof(s.httpheaderline)) 
  	{
    		s.httpheaderline[s.httpheaderlineptr] = *(char *)uip_appdata;
    		++((char *)uip_appdata);
    		--len;
    		if(s.httpheaderline[s.httpheaderlineptr] == ISO_nl) 
    		{
      			/* We have an entire HTTP header line in s.httpheaderline, so we parse it. */
      			if(s.httpheaderline[0] == ISO_cr) 
      			{
				/* This was the last header line (i.e., and empty "\r\n"), so
	   			we are done with the headers and proceed with the actual data. */
				s.state = WEBCLIENT_STATE_DATA;
				return len;
      			}

      			s.httpheaderline[s.httpheaderlineptr - 1] = 0;
      			/* Check for specific HTTP header fields for filter out the http header information*/	//cj add 09132007
#if 0
			if(App_State.dwState & NET_RECVGCE)
			{
#if HAVE_GCE
#if 1			
				if(casecmp(s.httpheaderline, X_GCE_Button_OK,sizeof(X_GCE_Button_OK) - 1) == 0) 
				{
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(X_GCE_Button_OK) - 1, sizeof(s.mimetype));	
					gce_header_center_button(s.mimetype+1);
					//MP_DEBUG1("X_GCE_Button_OK < ==== %s ",s.mimetype);
				      
				}
				else if(casecmp(s.httpheaderline, X_GCE_Button_Up,sizeof(X_GCE_Button_Up) - 1) == 0)
				{
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(X_GCE_Button_Up) - 1, sizeof(s.mimetype));
					gce_header_up_button(s.mimetype+1);
					//MP_DEBUG1("X_GCE_Button_Up < ==== %s ",s.mimetype);

				}
				else if(casecmp(s.httpheaderline, X_GCE_Button_Down,sizeof(X_GCE_Button_Down) - 1) == 0)
				{
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(X_GCE_Button_Down) - 1, sizeof(s.mimetype));
					gce_header_down_button(s.mimetype+1);
					//MP_DEBUG1("X_GCE_Button_Down < ==== %s ",s.mimetype);


				}
				else if(casecmp(s.httpheaderline, X_GCE_Button_Left,sizeof(X_GCE_Button_Left) - 1) == 0)
				{
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(X_GCE_Button_Left) - 1, sizeof(s.mimetype));		
					gce_header_left_button(s.mimetype+1);
					//MP_DEBUG1("X_GCE_Button_Left < ==== %s ",s.mimetype);


				}
				else if(casecmp(s.httpheaderline, X_GCE_Button_Right,sizeof(X_GCE_Button_Right) - 1) == 0)
				{

					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(X_GCE_Button_Right) - 1, sizeof(s.mimetype));		
					gce_header_right_button(s.mimetype+1);
					//MP_DEBUG1("X_GCE_Button_Right < ==== %s ",s.mimetype);

				}
				else if(casecmp(s.httpheaderline, Refresh,sizeof(Refresh) - 1) == 0)
				{	
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(Refresh) - 1, sizeof(s.mimetype));	

					cptr++;
									
					while(*cptr == ' ') 
						cptr++;
									
					if(!strncmp(cptr,"URL=",4))
					{
						cptr+=4;
						//MP_DEBUG2("%%%% Refresh < ==== %s, %s ",s.mimetype,cptr);	
						//gce_header_refresh(s.mimetype,cptr);
											
					}
					
				}					
#else
				if(casecmp(s.httpheaderline, http_x_button_center,sizeof(http_x_button_center) - 1) == 0) 
				{
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(http_x_button_center) - 1, sizeof(s.mimetype));	
					gce_header_center_button(s.mimetype+1);
					//MP_DEBUG1("http_x_button_center < ==== %s ",s.mimetype);
				      
				}
				else if(casecmp(s.httpheaderline, http_x_button_up,sizeof(http_x_button_up) - 1) == 0)
				{
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(http_x_button_up) - 1, sizeof(s.mimetype));
					gce_header_up_button(s.mimetype+1);
					//MP_DEBUG1("http_x_button_up < ==== %s ",s.mimetype);

				}
				else if(casecmp(s.httpheaderline, http_x_button_down,sizeof(http_x_button_down) - 1) == 0)
				{
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(http_x_button_down) - 1, sizeof(s.mimetype));
					gce_header_down_button(s.mimetype+1);
					//MP_DEBUG1("http_x_button_down < ==== %s ",s.mimetype);


				}
				else if(casecmp(s.httpheaderline, http_x_button_left,sizeof(http_x_button_left) - 1) == 0)
				{
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(http_x_button_left) - 1, sizeof(s.mimetype));		
					gce_header_left_button(s.mimetype+1);
					//MP_DEBUG1("http_x_button_left < ==== %s ",s.mimetype);


				}
				else if(casecmp(s.httpheaderline, http_x_button_right,sizeof(http_x_button_right) - 1) == 0)
				{

					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(http_x_button_right) - 1, sizeof(s.mimetype));		
					gce_header_right_button(s.mimetype+1);
					//MP_DEBUG1("http_x_button_right < ==== %s ",s.mimetype);

				}
				else if(casecmp(s.httpheaderline, http_refresh,sizeof(http_refresh) - 1) == 0)
				{	
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(http_refresh) - 1, sizeof(s.mimetype));	
					gce_header_refresh(s.mimetype+1);
					//MP_DEBUG1("http_refresh < ==== %s ",s.mimetype);
				}	
				else if(casecmp(s.httpheaderline, http_content_length,sizeof(http_content_length) - 1) == 0) 
				{
					// Found Content-type field. 
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(http_content_length) - 1, sizeof(s.mimetype));		
					//MP_DEBUG1("header < ==== %s ",s.httpheaderline);
							
					App_State.HttpContentLength = atoi(s.mimetype);
					//MP_DEBUG2("GCE http_content_length < ==== %s <= %d ",s.mimetype,App_State.HttpContentLength);			
				} 	
#endif
#endif					

			}
			else
#endif				
			{
              		if(casecmp(s.httpheaderline, http_content_range, sizeof(http_content_range) - 1) == 0) 
		    		{
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(http_content_range) - 1, sizeof(s.mimetype));					
					//MP_DEBUG1("header < ==== %s ",s.httpheaderline);
				
					str = (s.mimetype+6);
					memset(BUFF,0x00,16);
					index = 0;
					while(*str != '-')
						BUFF[index++] = *str++; 
					App_State.HttpStart = atoi(BUFF);
					str++;
					memset(BUFF,0x00,16);
					index = 0;
					while(*str != '/')
						BUFF[index++] = *str++; 
					App_State.HttpEnd = atoi(BUFF);
					str++;
					memset(BUFF,0x00,16);
					index = 0;
			
					while(*str != '\0')
						BUFF[index++] = *str++; 
					App_State.HttpFileSize = atoi(BUFF);

/*
					MP_DEBUG5("<<<<<<<<<<<<<<<<<<UIP http_content_range < ==== %s <=  %d   %d   %d  %d",
					                       s.mimetype,
					                       App_State.HttpStart,
					                       App_State.HttpEnd,
					                       App_State.HttpEnd -App_State.HttpStart,
					                       App_State.HttpFileSize);				
*/					
				} 
				else if(casecmp(s.httpheaderline, http_content_length,sizeof(http_content_length) - 1) == 0) 
				{
					// Found Content-type field. 
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
					strncpy(s.mimetype, s.httpheaderline + sizeof(http_content_length) - 1, sizeof(s.mimetype));		
					//MP_DEBUG1("header < ==== %s ",s.httpheaderline);
							
					App_State.HttpContentLength = atoi(s.mimetype);
					//MP_DEBUG2("<<<<<<<<<<<<<<<<<<UIP  http_content_length < ==== %s <= %d ",s.mimetype,App_State.HttpContentLength);			

				} 	
				else if(casecmp(s.httpheaderline, http_content_type,	sizeof(http_content_type) - 1) == 0) 
		       	{
					/* Found Content-type field. */
					cptr = strchr(s.httpheaderline, ';');
					if(cptr != NULL) 
					{
	  					*cptr = 0;
					}
				
					strncpy(s.mimetype, s.httpheaderline + sizeof(http_content_type) - 1, sizeof(s.mimetype));
					
      				} 
      				else if(casecmp(s.httpheaderline, http_location, sizeof(http_location) - 1) == 0) 
				{
					cptr = s.httpheaderline + sizeof(http_location) - 1;
	
					if(strncmp(cptr, http_http, 7) == 0) 
					{
	  					cptr += 7;
	  					for(i = 0; i < s.httpheaderlineptr - 7; ++i) 
	  					{
	    						if(*cptr == 0 || *cptr == '/' || *cptr == ' ' || *cptr == ':') 
	    						{
	      							s.host[i] = 0;
	      							break;
	    						}
	    				
	    						s.host[i] = *cptr;
	    						++cptr;
	  					}
					}
				
					strncpy(s.file, cptr, sizeof(s.file));
				
					/*	s.file[s.httpheaderlineptr - i] = 0;*/
      				}
			}
      			/* We're done parsing, so we reset the pointer and start the next line. */
      			s.httpheaderlineptr = 0;
    		} 
    		else 
    		{
      			++s.httpheaderlineptr;
    		}
    	
  	}
  	
  	return len;
}

#if HAVE_CURL == 0
/*-----------------------------------------------------------------------------------*/
void webclient_newdata(void)
{
  	u16_t len;

  	len = uip_datalen();

	if(s.state == WEBCLIENT_STATE_STATUSLINE) 
  	{
    	len = parse_statusline(len);
  	}
  
  	if(s.state == WEBCLIENT_STATE_HEADERS && len > 0) 
  	{
    	len = parse_headers(len);
  	}

  	if(len > 0 && s.state == WEBCLIENT_STATE_DATA && s.httpflag != HTTPFLAG_MOVED) 
  	{
    	webclient_datahandler((char *)uip_appdata, len);
  	}
}
#endif


/*-----------------------------------------------------------------------------------*/
void webclient_connected(void)
{
    s.state = WEBCLIENT_STATE_STATUSLINE;
}

/*-----------------------------------------------------------------------------------*/

/** @} */
/** @} */
