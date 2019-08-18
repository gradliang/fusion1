#define LOCAL_DEBUG_ENABLE 1
#include "platform_config.h"
#include "global612.h"
#include "mpTrace.h"
#include "netstream.h"
#include "taskid.h"
#include "netware.h"
#include "..\..\lwip\include\net_sys.h"
#include "..\..\lwip\include\net_socket.h"
#include "..\..\libIPLAY\libsrc\demux\include\filetype.h"
#include "..\..\CURL\include\net_curl_curl.h"
#include "mmsh.h"
#include "xine.h"
#include "socket.h"

BOOLEAN is_asf;
int asf_curindx = 0;
unsigned char asf_href = 0;
mmsh_t *asf_this = NULL;
int mmsh_connection = 0;;
unsigned char pause_flag = 0;
unsigned int buf_pre_space = 0;
int asf_preindx = 0;
int buf_pre_index = 0;

struct connect_info_s {
    char    url[MAX_NET_LINK_LEN+1];
};

struct connect_info_s conn;
#if (CHIP_VER_MSB == CHIP_VER_650)
unsigned char girbuf[NETSTREAM_MAX_BUFSIZE+8192];
#else
unsigned char *girbuf = NULL;
#endif
unsigned char irready = 0;
unsigned char g_internetradiorun = 0;
int gtotoalbuf = 0;

long gtotaldata = 0;                                /* Total amount of data in buffer */
int iradio_wait;
DWORD giRadioStreamtype = 0;
int bufindex = 0;
int bufindex_end = 0;

unsigned int g_index_internetradio;
unsigned char bgInternetRadioPause;
int kcurindx;
unsigned mpause = 0;
//extern ST_NET_FILEENTRY * g_FileEntry;
extern ST_NET_FILEBROWSER *g_psNet_FileBrowser;
int audio_type = FILE_TYPE_MP3;


BYTE go_flag = TRUE;
int manage_count = 0;
long gtotalbackup = 0;                                
unsigned char error_flag = 0;

extern DRIVE *w_sDrv;
extern STREAM *w_shandle;
extern BYTE g_bXpgStatus;
extern Net_App_State App_State;
extern Audio_dec Media_data;
#ifndef MIN
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#endif

#define E_CONNECT_FAIL	0x00000001
#define E_CONNECTION_DISCONNECTED 0x00000002
#define BRECONNECTED 0x00000010

#define MAX_REQUEST_SIZE 1024


#define	ETIMEDOUT	145	/* Connection timed out */

#define	VTURN_RETRY	10	
#define	SHOUTCAST_RETRY	10	

#if HAVE_VTUNER
static size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf)
{
    char *wr_buf = buf;
    size_t len = MIN((NETSTREAM_MAX_BUFSIZE-gtotaldata), size * nmemb);
	unsigned char dbcount = 0;
    mpDebugPrint("my_write_func");
    if (is_asf)
    {
        /* It's Microsoft ASF file */
        memcpy(&wr_buf[bufindex], ptr, len);
        bufindex += len;
    }
    else
    {
        if (len > 0)
        {
            memcpy(&wr_buf[bufindex], ptr, len);
            gtotaldata += len;
            bufindex += len;

            if( irready == 0 )
            {
                if( (gtotaldata > IRADIO_DATA_HIGH) )
                {
                    irready  = 1;
                }
                if( iradio_wait )
                {
                    if( (gtotaldata >= IRADIO_DATA_HIGH) )
                    {
                        mpDebugPrint("iradio: high watermark");
                        iradio_wait  = 0;
                    }
                }
            }

            if( bufindex > NETSTREAM_MAX_BUFSIZE )
            {
                bufindex = (bufindex % (NETSTREAM_MAX_BUFSIZE));
                {
                    int dwTmp;
                    DWORD *pdwSrc, *pdwTar;

                    pdwTar = (DWORD *) &girbuf[0];
                    pdwSrc = (DWORD *) &girbuf[NETSTREAM_MAX_BUFSIZE];
                    for(dwTmp = bufindex; dwTmp > 0; dwTmp -= 4)
                    {
                        *pdwTar = *pdwSrc;
                        pdwTar++;
                        pdwSrc++;
                    }
                }
            }

            if( dbcount % 20 )
                TaskYield();
            dbcount++;
        }

        if( g_internetradiorun == 0 )
            return 0;                               /* terminate the transfer */
    }

    return len;
}

static size_t my_write_func2(void *ptr, size_t size, size_t nmemb, void *buf) 
{
	int r;
    char *ctype, type_str[80],subtype_str[80];
    size_t len = MIN(256, size * nmemb);
    char *wr_buf = buf;
    BOOLEAN is_ok = FALSE;

    mpDebugPrint("my_write_func2");
    memcpy(wr_buf, ptr, len);
    wr_buf[len] = '\0';

    ctype = mpx_Malloc(80);
    if (!ctype)
        return size * nmemb;

	r = sscanf(wr_buf, "Content-Type: %s\n", ctype);

	if (r)
    {
		mpDebugPrint("Content-Type: %s", ctype);
        is_asf = FALSE;

        subtype_str[0] = type_str[0] = '\0';
        r = sscanf(ctype, "%s", type_str);
        if (r == 1)
        {
            char *chp = strstr(type_str, "/");
            if (chp)
            {
                *chp = '\0';
                strncpy(subtype_str, chp+1, sizeof subtype_str);
                subtype_str[sizeof subtype_str - 1] = '\0';
                is_ok = TRUE;
            }
        }
        mpDebugPrint("1=%s,2=%s,3=%d", type_str, subtype_str, r);
        if (is_ok)
        {
            if (!strcasecmp(type_str, "video") ||
                !strcasecmp(type_str, "audio"))
            {
                if (!strcasecmp(subtype_str, "x-ms-asf") ||
                    !strcasecmp(subtype_str, "x-ms-wax") ||
                    !strcasecmp(subtype_str, "x-ms-wma") ||
                    !strcasecmp(subtype_str, "x-ms-wmv") /* TODO */
                    )
                {
                    mpDebugPrint("It's ASF");
                    is_asf = TRUE;
                }
            }
        }
    }

    mpx_Free(ctype);
	return size * nmemb;
}
static void asx_content_handler(void *user_data, const char *content, int len)
{
	//mpDebugPrint("asx_content_handler %s",content); 	

	int i,j;
    struct asx_control_s *ctrl = user_data;
	
    if (ctrl->REF_tag)
    {
    }
}
static void asx_tag_start_handler(void *user_data, const char *tag_name, char **attr)
{
    struct asx_control_s *ctrl = user_data;
	mpDebugPrint("asx_tag_start_handler %s",tag_name); 	
    if (!strcasecmp(tag_name, "ASX"))
    {
        ctrl->ASX_tag = TRUE;
    }
    else if (!strcasecmp(tag_name, "ENTRY"))
    {
        ctrl->ENTRY_tag = TRUE;
    }
    else if (!strcasecmp(tag_name, "REF"))
    {
        char *attrp;
        char **ap = attr;

        ctrl->REF_tag = TRUE;
        ctrl->asx_href[asf_href].href[0] = '\0'; /* initial value */

#if 0
        if (ap)
        {
            while (attrp = *ap++)
                MP_DEBUG("asx attr=%s",attrp); 	
        }
#endif

        while (attrp = *ap++)
        {
            MP_DEBUG("asx_tag_start_handler attr=%s",attrp); 	
            if (!strcasecmp(attrp, "HREF"))
            {
                attrp = *ap++;
                if (!attrp)
                    break;
                strncpy(ctrl->asx_href[asf_href].href, attrp, sizeof(ctrl->asx_href[asf_href].href));
                ctrl->asx_href[asf_href].href[sizeof(ctrl->asx_href[asf_href].href) - 1] = '\0';
				if(asf_href < 2)
				asf_href++;
                MP_DEBUG("asx_tag_start_handler href=%s",ctrl->asx_href[asf_href].href); 	
            }
        }
    }
    else if (!strcasecmp(tag_name, "Reference"))
    {

			char *attrp;
			char **ap = attr;
	
			ctrl->REF_tag = TRUE;
			ctrl->asx_href[asf_href].href[0] = '\0'; /* initial value */
	
	
			while (attrp = *ap++)
			{
				mpDebugPrint("asx_tag_start_handler attr=%s",attrp);	
				if (!strcasecmp(attrp, "Ref"))
				{
					attrp = *ap++;
					if (!attrp)
						break;
					strncpy(ctrl->asx_href[asf_href].href, attrp, sizeof(ctrl->asx_href[asf_href].href));
					ctrl->asx_href[asf_href].href[sizeof(ctrl->asx_href[asf_href].href) - 1] = '\0';
					if(asf_href < 2)
						asf_href++;
					mpDebugPrint("asx_tag_start_handler href=%s",ctrl->asx_href[asf_href].href);	
				}
			}


	
    }

}

static void asx_tag_end_handler(void *user_data, const char *tag_name)
{
    struct asx_control_s *ctrl = user_data;
	//mpDebugPrint("asx_tag_end_handler"); 	
	
    if (!strcasecmp(tag_name, "ASX"))
    {
        ctrl->ASX_tag = FALSE;
    }
    else if (!strcasecmp(tag_name, "ENTRY"))
    {
        ctrl->ENTRY_tag = FALSE;
    }
    else if (!strcasecmp(tag_name, "REF"))
        ctrl->REF_tag = FALSE;

	//mpDebugPrint("<--  %s",tag_name); 	
}
/*
Parse asx file reference data
*/
void asx_parse_reference(BYTE *url,BYTE *buf,int len,BYTE idx)
{
  //mpDebugPrint("asx_parse_reference len %d",len);
  //mpDebugPrint(" %s ",buf);
  int i,j;
  BYTE go =0;
  BYTE cpy =0;
  BYTE *mms_url = "mmsh";
  
  for(i=0;i<len;i++)
  {
  
    //mpDebugPrint("%2c",buf[i]);
  	switch(idx)
  	{
  	  case 0:
		if((buf[i]=='R')&&(buf[i+1]=='e')&&(buf[i+2]=='f')&&(buf[i+3]=='1'))
		{
		
		  //mpDebugPrint("cpy");
		  cpy = 1;
		}
	  	break;
	  case 1:
		if((buf[i]=='R')&&(buf[i+1]=='e')&&(buf[i+2]=='f')&&(buf[i+3]=='2'))
		{
		  cpy = 1;
		}
	  	break;
  	
  	}
    if(cpy)
    {
		if((buf[i]=='h')&&(buf[i+1]=='t')&&(buf[i+2]=='t')&&(buf[i+3]=='p'))
		{
		   memset(url,0,strlen(url));
		   go = 1;
		   j = 0;
		   i+=4;
		   memcpy(url,mms_url,strlen(mms_url));
		   j+=strlen(mms_url);
		}
		else if((buf[i]=='m')&&(buf[i+1]=='m')&&(buf[i+2]=='s'))
		{
			memset(url,0,strlen(url));
			go = 1;
			j = 0;
		}
		
		
		if(go)
		{
			if((buf[i+1] == 'M')&&(buf[i+2] == 'S')&&
			   (buf[i+3] == 'W')&&(buf[i+4] == 'M')&&
			   (buf[i+5] == 'E'))
				break;
			url[j] = buf[i];
			j++;
			
		}
    }
  
  }
  
}
#endif

int mpx_TcpConnect(char *host, int port)
{
	BYTE * ipaddr;
  	DWORD addr;
    int sock;

    MP_DEBUG("mpx_TcpConnect");
  	if((addr=inet_addr(host)) == INADDR_NONE) 
    {
        ipaddr = (BYTE *)SearchServerByName(host);
        if( ipaddr )
            addr = *((DWORD *) ipaddr);
    }

    MP_DEBUG("addr %x",addr);
    if (addr == INADDR_NONE)
        return -1;

    sock = mpx_DoConnect(addr, port, TRUE);
    if (sock <= 0)
        return -2;
    else
        return sock;
}
int mpx_UrlParse(char *url, char **proto, char** host, int *port,
                         char **uri) 
{
    char   *start;
    char   *slash;
    char   *portcolon  = NULL;
    char   *strtol_err = NULL;
    char   *end;

    MP_DEBUG("mpx_UrlParse");
    if (!host)  goto failed;
    if (!port)  goto failed;
    if (!uri)   goto failed;
    if (!proto)   goto failed;

    *host = NULL;
    *port = 0;
    *uri = NULL;
    *proto = NULL;

    start = strstr(url, "://");
    if (!start || (start == url))
        goto error;

    end  = start + strlen(start) - 1;
    *proto = strndup(url, start - url);

    start += 3;
    slash = strchr(start, '/');
    portcolon = strchr(start, ':');

    if (slash) {
      if (portcolon && portcolon < slash) {
        *host = strndup(start, portcolon - start);
        if (portcolon == start) goto error;
        *port = strtol(portcolon + 1, &strtol_err, 10);
        if ((strtol_err != slash) || (strtol_err == portcolon + 1))
          goto error;
      } else {
        *host = strndup(start, slash - start);
        if (slash == start) goto error;
      }
    } else {
      if (portcolon) {
        *host = strndup(start, portcolon - start);
        if (portcolon < end) {
          *port = strtol(portcolon + 1, &strtol_err, 10);
          if (*strtol_err != '\0') goto error;
        } else {
          goto error;
        }
      } else {
        if (*start == '\0') goto error;
        *host = strdup(start);
      }
    }

    /* uri */
    start = slash;
    if (start) {
        {
            static const char toescape[] = " #";
            char *it = start;
            unsigned int escapechars = 0;

            while( it && *it ) {
                if ( strchr(toescape, *it) != NULL )
                    escapechars++;
                it++;
            }

            if ( escapechars == 0 )
                *uri = strdup(start);
            else {
                const size_t len = strlen(start);
                size_t i;

                *uri = malloc(len + 1 + escapechars*2);
                it = *uri;

                for(i = 0; i < len; i++, it++) {
                    if ( strchr(toescape, start[i]) != NULL ) {
                        it[0] = '%';
                        it[1] = ( (start[i] >> 4) > 9 ) ? 'A' + ((start[i] >> 4)-10) : '0' + (start[i] >> 4);
                        it[2] = ( (start[i] & 0x0f) > 9 ) ? 'A' + ((start[i] & 0x0f)-10) : '0' + (start[i] & 0x0f);
                        it += 2;
                    } else
                        *it = start[i];
                }
            }
        }
    } else {
        *uri = strdup("/");
    }

	MP_DEBUG("host %s uri %s port %d proto %s",*host,*uri, *port, *proto);

    return 0;

error:
  if (*proto) {
    free (*proto);
    *proto = NULL;
  }
  if (*host) {
    free (*host);
    *host = NULL;
  }
  if (*port) {
    *port = 0;
  }
  if (*uri) {
    free (*uri);
    *uri = NULL;
  }
  return -1;
failed:
  return -2;
}

/*
 * waittime - in milliseconds
 */
int mpx_SocketSelect(int sock, int flags, int waittime)
{
    ST_SOCK_SET stReadSet, stWriteSet;
    ST_SOCK_SET *wfds, *rfds;
	struct timeval tv;
    int ret;

    MP_DEBUG("mpx_SocketSelect");
    MPX_FD_ZERO(&stReadSet);
    MPX_FD_ZERO(&stWriteSet);

    wfds = rfds = NULL;
    if (flags & SOCKET_WRITE_READY)
        wfds = &stWriteSet;
    if (flags & SOCKET_READ_READY)
        rfds = &stReadSet;

    if (wfds)
        MPX_FD_SET(sock, wfds);
    if (rfds)
        MPX_FD_SET(sock, rfds);


    tv.tv_sec = waittime/1000;
    tv.tv_usec = (waittime % 1000) * 1000;

    ret = select(0, rfds, wfds, 0, &tv);

    if (ret > 0)
        return 0;
    else if (ret == 0)
        return -ETIMEDOUT;
    else
        return -getErrno();
}

void init_global_param(
	void
	)
{
	gtotoalbuf = 0;
	gtotaldata = 0;
	kcurindx = 0;
	irready = 0;
	bufindex = 0;
}


int InterRadioCheckType(BYTE *url)
{
  int url_len = strlen(url);
  BYTE tmp[4];
  mpDebugPrint("InterRadioCheckType url %s",url);
  
  mpDebugPrint("url_len %d",url_len);
  memcpy(tmp,&url[url_len-4],4);
  if((!strcmp(tmp,".mp3"))||(!strcmp(tmp,".MP3")))
  	return FILE_TYPE_MP3;
  
  if((!strcmp(tmp,".wma"))||(!strcmp(tmp,".WMA")))
  	return FILE_TYPE_WMA;
}

int shoutcast_conn = 0;
int iradio_download(
	char *website,
	char *urlparm,
	char *ipaddr,
	U16 port )
{
  	DWORD addr;
	char request[MAX_REQUEST_SIZE];
	int idx;
    //char buf[8 * 1024];
	unsigned char retrycount = 0;
	unsigned char breconect = 0;
	unsigned char b_ir_pause = 0;
	unsigned int failcount = 0,selectfailcount = 0;
    ST_SOCK_SET stReadSet, stWriteSet;
    ST_SOCK_SET *wfds, *rfds;
    int ftpStatus = NO_ERR;
	unsigned long val = 0;
	struct timeval tv;
	int ret;
	//int iradio_conn;
    int num_bytes_recv;
	init_global_param();

	if( ipaddr )
	{
		MP_DEBUG1("ipaddr %x",ipaddr[0]);
		addr = *((DWORD *) ipaddr);
		ret = mpx_DoConnect(addr, port, TRUE);
	}
	else
	{
		addr=inet_addr(website);
		ret = mpx_DoConnect(addr, port, TRUE);
	}
    if( ret > 0 )
	{
		shoutcast_conn = ret;
	}
	else
	{
        DPrintf("iRadio::Internet Radio connect fail, retry");
		closesocket(shoutcast_conn);
		DPrintf("iRadio::Connect Fail\n");
		irready = 0xff;
		return (E_CONNECT_FAIL|BRECONNECTED);
	}
	snprintf(request, MAX_REQUEST_SIZE,
                "GET %s HTTP/1.1\r\n"
				"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/xaml+xml, application/vnd.ms-xpsdocument, application/x-ms-xbap, application/x-ms-application, */*\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n"
                "Referer: http://%s/\r\n"
                "\r\n",
				urlparm,
				website,
				website
				);
	MP_DEBUG("InterRadioStart ret %x\n",ret);
    MPX_FD_ZERO(&stReadSet);
    MPX_FD_ZERO(&stWriteSet);
    wfds = rfds = NULL;
	rfds = &stReadSet;
	MPX_FD_SET(shoutcast_conn, &stReadSet);
	send( shoutcast_conn, request, strlen(request) , 0);
	MP_DEBUG("send request\n");
	idx = 0;
	retrycount = 0;
	g_internetradiorun = 1;
	//send( tcp_connection, request, strlen(request) , 0);
	MP_DEBUG("send request AA\n");
	//val = 1;
    //ioctlsocket(iradio_conn, FIONBIO, &val);
	while( 1 )
	{
          
        num_bytes_recv = MIN(8192, (NETSTREAM_MAX_BUFSIZE-gtotaldata));

		if( NETSTREAM_MAX_BUFSIZE-gtotaldata < 8192 )
			mpDebugPrint("NETSTREAM_MAX_BUFSIZE-gtotaldata %d",NETSTREAM_MAX_BUFSIZE-gtotaldata);
		
		//SockIdSignalTcpFinRecvd(iradio_conn);

		num_bytes_recv = recv( shoutcast_conn, &girbuf[bufindex], num_bytes_recv,0);
		if( num_bytes_recv > 0 )
		{
			//MP_DEBUG("###num_read %d %x %x\n",num_bytes_recv,gtotoalbuf,gtotaldata);
			//NetAsciiDump(girbuf,num_bytes_recv);
			failcount = 0;
		}
		else
		{
			if(!(App_State.dwState & NET_CONFIGED))
			{
				mpDebugPrint("!!!!NET LINK DOWN!!!!!");
				//waitting link up
				while(!(App_State.dwState & NET_CONFIGED))
				{
					TaskYield();
				}
			
			}
			//closesocket(shoutcast_conn);
			//return BRECONNECTED;
#if 1
			//TaskYield();
			failcount ++;
			mpDebugPrint("failcount %x",failcount);
			if( failcount > 10)
			{
				closesocket(shoutcast_conn);
				MP_DEBUG("iRadio::Connection was closed.");
				//xpgStopAudio();
				init_global_param();
				breconect = 1;

				break;
			}
			continue;
#endif			
		}
		if( num_bytes_recv > 0 )
		{
			{
				gtotoalbuf += num_bytes_recv;
				gtotaldata += num_bytes_recv;
				bufindex += num_bytes_recv;
			}
			if( irready == 0 )
			{
				if( (gtotoalbuf > IRADIO_DATA_HIGH) )
				//if( (gtotoalbuf > 100*1024) )
				{
					mpDebugPrint("irready  = 1 %x girbuf %x\n",gtotoalbuf,girbuf);
					irready  = 1;
					TaskYield();
#if	MAKE_XPG_PLAYER						
					if(Media_data.fending)
						xpgPlayAudio();
#endif					
				}
			}
			if( gtotaldata  > IRADIO_MAX_BUFSIZE_3_O_4 )
			{
				mpDebugPrint("xpgCb_MusicForeward");
#if	MAKE_XPG_PLAYER						
				xpgCb_MusicForward();
#endif
			}
			if( iradio_wait )
			{
				if( (gtotaldata >= IRADIO_DATA_HIGH) )
				{
					mpDebugPrint("iradio: high watermark");
					iradio_wait  = 0;
				}
				mpDebugPrint("bpause %x",b_ir_pause);
				if( b_ir_pause  == 0 )
				{
#if	MAKE_XPG_PLAYER						
					xpgPauseAudio();
#endif
					b_ir_pause = 1;
				}
			}
		}
		if( b_ir_pause )
		{
			if( (gtotoalbuf - kcurindx) > (IRADIO_DATA_HIGH+20*1024) )
			{
				MP_DEBUG("xpgResumeAudio");
#if	MAKE_XPG_PLAYER						
				xpgResumeAudio();
#endif
				b_ir_pause = 0;
			}
		}
		
		if( bufindex > NETSTREAM_MAX_BUFSIZE )
		{
			bufindex = (gtotoalbuf % (NETSTREAM_MAX_BUFSIZE));
			{
				int dwTmp;
				DWORD *pdwSrc, *pdwTar;

				pdwTar = (DWORD *) &girbuf[0];
				pdwSrc = (DWORD *) &girbuf[NETSTREAM_MAX_BUFSIZE];
				for(dwTmp = bufindex; dwTmp > 0; dwTmp -= 4)
				{
					*pdwTar = *pdwSrc;
					pdwTar++;
					pdwSrc++;
				}

			}
		}

		if( g_internetradiorun == 0 )
		{
			closesocket(shoutcast_conn);
			break;
		}
		//TaskYield();
	}
	if(breconect)
		return BRECONNECTED;
	else
	return 0;
}
#if HAVE_VTUNER
static int asf_buf_space(int idx1,int idx2,int buf_len)
{
  int space_len = 0;
  
  if(idx1 > idx2)
  {
    space_len = idx1 - idx2;
  }
  else
  {
    space_len = (buf_len- idx2)+idx1;
  }
  	
  	
  return space_len;
}

static BOOLEAN asf_buf_manage(mmsh_t *this)
{
	
	unsigned int buf_space = 0;
	
  UartOutText(" * ");
  
 while(!(App_State.dwState & NET_CONFIGED))
 {
 	TaskYield();
 }

  if(!asf_this->playing)
  	go_flag = 0;
		
	/*check the decode point with receive data point*/
  if(irready&&(!pause_flag))
  {
	  buf_space = asf_buf_space(bufindex,asf_curindx,NETSTREAM_MAX_BUFSIZE);
	  //space increase
	  if(buf_pre_space < buf_space)
	  {
	    if(buf_space > IRADIO_VTUNER_DATA_HIGH)
	    {
	      //mpDebugPrint("buf_space %d > %d",buf_space,IRADIO_VTUNER_DATA_HIGH);
	      //mpDebugPrint("asf_preindx %d asf_curindx %d",asf_preindx,asf_curindx);
		  //mpDebugPrint("go_flag %d",go_flag);
          //If audio run and turn on net receive
			if((asf_preindx != asf_curindx)&&(buf_pre_index!=bufindex))
			{
            	//stop net receive
				go_flag = 1;
				
				if(g_bAniFlag & ANI_PAUSE)
				{
					   //audio resume
#if	MAKE_XPG_PLAYER						
						xpgResumeAudio();
#endif
				}
			}
			else if((asf_preindx != asf_curindx)&&(buf_pre_index == bufindex)) 
			{
			   go_flag = 0;
			   if(!(g_bAniFlag & ANI_PAUSE))
			   {
				   //audio pause
#if	MAKE_XPG_PLAYER						
					xpgPauseAudio();
#endif
			   }
			
			}else
			{
				//stop net receive
				go_flag = 1;
				
				if(g_bAniFlag & ANI_PAUSE)
				{
					   //audio resume
#if	MAKE_XPG_PLAYER						
						xpgResumeAudio();
#endif
				}
			}
		  	
	    }
		else
		{
			//turn on net receive or audio decode
		   if(buf_space >= (IRADIO_VTUNER_DATA_HIGH - 8192))
		   	{
		   	
				 if(go_flag)
				 {
				    go_flag = 0;
				 }
				 else if(g_bAniFlag & ANI_PAUSE)
				 {
						//audio resume
#if	MAKE_XPG_PLAYER						
						 xpgResumeAudio();
#endif
				 }
		   	}
	
		}
  
	  }
	  else//space reduce
	  {
	    if(buf_space < IRADIO_VTUNER_DATA_LOW)
	    {
			mpDebugPrint("buf_space %d < %d",buf_space,IRADIO_VTUNER_DATA_LOW);
			//first start net receive
			if((asf_preindx != asf_curindx)&&(buf_pre_index!=bufindex))
			{
			  go_flag = 0;
			  
			  if(!(g_bAniFlag & ANI_PAUSE))
			  {
				  //audio pause
#if	MAKE_XPG_PLAYER						
				   xpgPauseAudio();
#endif
			  }
			}
			else if(asf_preindx == asf_curindx)
			{
			  go_flag = 1;
			  if(g_bAniFlag & ANI_PAUSE)
			  {
#if	MAKE_XPG_PLAYER						
				  xpgResumeAudio();
#endif
			  }
			}else
			{
				
				go_flag = 0;
				
				if(!(g_bAniFlag & ANI_PAUSE))
				{
					//audio pause
#if	MAKE_XPG_PLAYER						
					 xpgPauseAudio();
#endif
				}
			}
	    }
		else
		{
		   if(buf_space <= ((IRADIO_VTUNER_DATA_HIGH+IRADIO_VTUNER_DATA_LOW)/2))
		   	{
		   	     // turn on net receive or audio decode
				 if(go_flag)
				 {
				    go_flag = 0;
				 }
				 else if(g_bAniFlag & ANI_PAUSE)
				 {
						//audio resume
#if	MAKE_XPG_PLAYER						
						 xpgResumeAudio();
#endif
				 }
		   	}
	
		}
	  
	  }

	  buf_pre_index = bufindex;
	  asf_preindx = asf_curindx;
	  buf_pre_space = buf_space;
	  
	  TaskYield();
  }
  
  if(pause_flag)
  {
	  go_flag = 1;
  }
		
  TaskYield();
  	
  return !go_flag;
}

int vtuner_download(void)
{
	ST_NET_FILEENTRY *pFileEntry;
    void *curl;
	char request[1024];
    CURLcode res;
    struct asx_control_s parser_ctrl;
	html_parser_t asx_parser;
    int len;
	unsigned char go_hrref = 0;
	int asf_rec_cnt = 0;
    int num_read;
	int mmsh_rev_len = ASF_HEADER_SIZE;
	xine_stream_t *xinestream = NULL;
	int ret = 0;
	int	link_error = 0;
	
	mpDebugPrint("vtuner_download");
		
	pFileEntry = (ST_NET_FILEENTRY *)(g_psNet_FileBrowser->FileEntry);
	mpDebugPrint(" %s ",pFileEntry[g_index_internetradio].Link);
wma_start:	
	curl = curl_easy_init();
	if(curl)
	{
		//set URL
		curl_easy_setopt(curl, CURLOPT_URL, pFileEntry[g_index_internetradio].Link);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, girbuf);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, request);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, my_write_func2);
			
		//set follow location
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
			
		res = curl_easy_perform(curl);
		//NetAsciiDump(girbuf, bufindex);
				
		gtotaldata = 0;
		irready = 0;
		asf_curindx = 0;
		TaskYield();
								
		/* always cleanup */
		curl_easy_cleanup(curl);
				
	}
	if (is_asf)
	{
			
		//MP_DEBUG("iRadio:: ASF file");
		
		memset(&parser_ctrl, 0, sizeof parser_ctrl);
		asf_href = 0;
		/* parse ASX file */
	
		html_init(&asx_parser, &parser_ctrl);
		html_set_content_handler(&asx_parser, asx_content_handler);
		html_set_tag_start(&asx_parser, asx_tag_start_handler);
		html_set_tag_end(&asx_parser, asx_tag_end_handler);
		//for MMS parse
		html_parse(&asx_parser,girbuf,bufindex);
		html_exit(&asx_parser);
		if(asf_href == NULL)
		{
			asx_parse_reference(parser_ctrl.asx_href[0].href,girbuf,bufindex,0);
			mpDebugPrint("parser_ctrl.href.asx_href[0] %s",parser_ctrl.asx_href[0].href);
			asx_parse_reference(parser_ctrl.asx_href[1].href,girbuf,bufindex,1);
			mpDebugPrint("parser_ctrl.href.asx_href[1] %s",parser_ctrl.asx_href[1].href);
			if((parser_ctrl.asx_href[0].href==NULL)||(parser_ctrl.asx_href[1].href==NULL))
			{
	           irready = 0xff;
			   ret = -1;
			   goto download_exit;
			}
		}
				
reconnect:	
//		mmcp_memset(girbuf,0x00,NETSTREAM_MAX_BUFSIZE+8192);

		TaskYield();

		if ((len = strlen(parser_ctrl.asx_href[0/*go_hrref*/].href)) > 0 && len <= MAX_NET_LINK_LEN)
		{
			strcpy(conn.url, parser_ctrl.asx_href[0/*go_hrref*/].href);
			asf_this = mmsh_connect(xinestream,parser_ctrl.asx_href[0/*go_hrref*/].href, 0);
			if(asf_this!=NULL)
			
			{
				bufindex = 0;//clear buffer index
				asf_rec_cnt = 0;
			g_internetradiorun = 1;
				while(1)
				{
					   
				/*connect mmsh server and receive mmsh packets*/
						//check the audio is decode
				if(asf_buf_manage(asf_this))
				{

					num_read = mmsh_read(asf_this,girbuf+bufindex,mmsh_rev_len);
					
					if(num_read == 0)
					{
					  mpDebugPrint("num_read == 0");
					  TaskSleep(100);
					}
					
					/*for receive data len to ASF_HEADER_SIZE*/
					if(num_read!=mmsh_rev_len)
					{
					  mmsh_rev_len -= num_read;
					}
					else
						mmsh_rev_len = ASF_HEADER_SIZE;
					
					mpDebugPrint("bufindex %d",bufindex);
					/*return to buffer cycle*/
					if((bufindex+ASF_HEADER_SIZE) >= NETSTREAM_MAX_BUFSIZE)// IRADIO_MAX_BUFSIZE /8192 = 64
					{
					    bufindex_end = bufindex+num_read;
						bufindex = 0;
					}
					else
					{
						bufindex+=num_read;
#if	MAKE_XPG_PLAYER						
						
						if(!irready)
							xpgUpdateYouTubeLoading(1);
#endif
					}
				}
							
				/*Receive data to buffer and turn on radio*/
				if(asf_rec_cnt < (ASF_RECEIVE_MAX - 2))
							asf_rec_cnt++;
				
						else if((asf_this->playing)&&(irready == 0))
						{
								mpDebugPrint("turn on irready!!");
								irready = 1;
#if	MAKE_XPG_PLAYER						
								xpgUpdateYouTubeLoading(0);
#endif
	
								//NetPacketDump(girbuf,bufindex);
								mmsh_set_start_time(asf_this,GetSysTime());
	
								mmsh_connection = 1;
#if	MAKE_XPG_PLAYER						
										
								if(Media_data.fending)
										xpgPlayAudio();
#endif
								
								if(error_flag)
								{
								   if(link_error)
								   	{
#if	MAKE_XPG_PLAYER						
										xpgPlayAudio(); 
#endif
										link_error = 0;
								   	}
									error_flag = 0;
								}
									}
									/*Exit while loop*/
							if((g_internetradiorun == 0 )||(pause_flag)||(error_flag&&irready))
									{
download_exit:				
							   if(!(App_State.dwState & NET_CONFIGED))
							   {
								 mpDebugPrint("!!!!NET LINK DOWN!!!!!");
#if	MAKE_XPG_PLAYER						
								 xpgStopAudio();//To stop audio
#endif								 
								 //waitting link up
								 while(!(App_State.dwState & NET_CONFIGED))
								 {
								 	TaskYield();
								 }
                                 link_error = 1;
							   }
										mpDebugPrint("Exit while loop!!");
								mpDebugPrint("g_internetradiorun %d",g_internetradiorun);
								mpDebugPrint("pause_flag %d",pause_flag);
								mpDebugPrint("error_flag %d",error_flag);
								irready = 0;
								mmsh_connection = 0;
								pause_flag = 0;
								buf_pre_space = 0;
								asf_curindx = 0;

								//if(!link_error)
								//	xpgPlayAudio();//To stop audio
								mmsh_close(asf_this);
								//receive data packet error or audio error reconnect 
								if(Media_data.fending)
								{
								  goto reconnect;// wma_start;

								}

								if(error_flag)
								{
									bufindex = 0;
									if(!UsbOtgWifiPlugin()||!wpa_supplicant_get_run_status())
									{
									  if(!link_error)
									{
									  ret = 0;
									  irready = 0xff;
									  break;
									}
									}
									mpDebugPrint("\n Goto WMA_START\n");
									goto reconnect;// wma_start;
								}
								break;
							}
							else if(error_flag)
							{
							   if(!(App_State.dwState & NET_CONFIGED))
							   {
								 mpDebugPrint("!!!!NET LINK DOWN!!!!!");
								 //waitting link up
								 while(!(App_State.dwState & NET_CONFIGED))
								 {
								 	TaskYield();
								 }
								 link_error = 1;
								 goto reconnect;
							
								}
							}
							TaskYield();
						}
						
					}
						else
						{
	
							//if(go_hrref == 0)
							if(go_hrref < VTURN_RETRY)
							{
								go_hrref++;
								goto reconnect;
							}
							else
							{
								go_hrref = 0;
								MP_DEBUG("MMSH connect fial!!");
								irready = 0xff;
							}
						}
						
					}
					else
						irready = 0xff;
				}
				else
				{
				    ret = -1;
					irready = 0xff;
				}
   return ret;
}
#endif
int NetStrean_Buffer_Alloc(int size)
{
#if (CHIP_VER_MSB == CHIP_VER_660)
  girbuf = ext_mem_malloc(size);
  if(girbuf)
  	return TRUE;
  else
  	return FALSE;
#else
   return TRUE;
#endif
}

/*
 * Media streaming thread
 */
void NetStream(void)
{
	char *urlparm;
	char urlparmdefault[]="/";
  	DWORD addr;
	BYTE *website;
	U16 port;
    int ret;
	DWORD dwEvent, dwNextEvent;
	BYTE * ipaddr;
	ST_NET_FILEENTRY *pFileEntry;
	int retcode;
	unsigned char mretrycount;
	int audio_type = FILE_TYPE_MP3;
	BYTE need_get_flag=TRUE;
	//Xml_BUFF_init(NET_YOUTUBE);

wait_to_start:
	dwNextEvent = 0;
	dwEvent = 0;
	g_internetradiorun = 0;
	mretrycount = 0;
	MP_DEBUG("iRadio::NetStream Task Start\n");
	EventWait(NETWORK_STREAM_EVENT, video_event|audio_event, OS_EVENT_OR, &dwEvent);
	
	MP_DEBUG1("NetStream dwEvent %x\n",dwEvent);
	
#if HAVE_YOUTUBE
	if(dwEvent&video_event)
	{
#if	DM9KS_ETHERNET_ENABLE||DM9621_ETHERNET_ENABLE
#if (P2P_TEST == 0&&MAKE_XPG_PLAYER)
	     if(GetNetConfigP2PTestFlag())
#endif		 	
	     {
	       need_get_flag = FALSE;
	     }
#endif	 
#if HAVE_HTTP_SERVER
         need_get_flag = FALSE;
#endif
     	 if(need_get_flag)
	     {
			if(youtube_get_video_info(youtube_get_video_index())!=200)
			{
			    EventSet(NETWORK_STREAM_EVENT, network_link_down);//to exit xpg wait even
				goto wait_to_start;
			}
			
			if(youtube_get_video_location()!=303)
			{
				EventSet(NETWORK_STREAM_EVENT, network_link_down);//to exit xpg wait even
				goto wait_to_start;
			}
		    Xml_BUFF_init(NET_YOUTUBE);
			retcode = youtube_video_download(youtube_get_video_index(),0);

	     }
#if	DM9KS_ETHERNET_ENABLE ||DM9621_ETHERNET_ENABLE
		 else 
		 {
			Xml_BUFF_init(NET_YOUTUBE);
			retcode = youtube_p2p_video_download(youtube_get_video_index(),0);
		 }
#endif
#if HAVE_HTTP_SERVER
		 retcode = youtube_get_http_server_file();
		 EventSet(NETWORK_STREAM_EVENT, network_link_down);//to exit xpg wait even
#endif
		 if(retcode==2){
			 mpDebugPrint("[NetStream] youtube_video_donwload return youtube_video.youtube_video_error is %d", retcode);
			 mpDebugPrint("NET_YOUTUBE have free");
			 Xml_BUFF_free(NET_YOUTUBE);
		 }
			
		 //Xml_BUFF_free(NET_YOUTUBE);
		 TaskYield();
		 goto wait_to_start;
	}
#elif HAVE_YOUKU3G
	if(dwEvent&video_event)
	{
			if(youku3g_get_video_info(youku3g_get_video_index())!=200)
			{
			    EventSet(NETWORK_STREAM_EVENT, network_link_down);//to exit xpg wait even
				goto wait_to_start;
			}
		
			if(youku3g_get_video_location()!=303)
				{
				EventSet(NETWORK_STREAM_EVENT, network_link_down);//to exit xpg wait even
				goto wait_to_start;
			}
		    Xml_BUFF_init(NET_YOUKU3G);
			retcode = youku3g_video_donwload(youku3g_get_video_index(),0);
#if 0
			if(retcode == 1)
				EventSet(NETWORK_STREAM_EVENT, network_link_down);//to exit xpg wait even
			//else if(retcode == 2)
			//{
				//mpDebugPrint("g_bXpgStatus %d",g_bXpgStatus);
				//g_bXpgStatus = XPG_MODE_YOUTUBEFAVORSHOW;
				//xpgYouTubeEnd();
			//}
			//add for fix bug 208 "Can't show the photo after re-plug in the wifi dongle"
			else
#endif
                        if(retcode==2){
				mpDebugPrint("[NetStream] youtube_video_donwload return youtube_video.youtube_video_error is %d", retcode);
				mpDebugPrint("NET_YOUTUBE have free");
				Xml_BUFF_free(NET_YOUKU3G);
			}
				
		    //Xml_BUFF_free(NET_YOUTUBE);
		    TaskYield();
			goto wait_to_start;
	}
#endif	

	if(dwEvent&audio_event)
	{
		init_global_param();
		MP_DEBUG1("InterRadioStart dwEvent %x\n",dwEvent);
		
	    switch(giRadioStreamtype)
	    {
		  case 0://shoutcast
			//MP_DEBUG1("%s",Internet_Radio_Feed[g_index_internetradio].bRadio_Feed);
			if( g_bXpgStatus == XPG_MODE_NET_PC_AUDIO )
			{
				pFileEntry = (ST_NET_FILEENTRY *)(g_psNet_FileBrowser->FileEntry);
				  //memset(pFileEntry[g_index_internetradio].Link,0x00,256);
				  //memcpy(pFileEntry[g_index_internetradio].Link,"http://www.chinesemusicworld.com:8202/",strlen("http://www.chinesemusicworld.com:8202/"));
				MP_DEBUG1("%s",pFileEntry[g_index_internetradio].Link);
			}
		
retry_con:
			MP_DEBUG("InterRadioStart\n");
			if( g_bXpgStatus == XPG_MODE_NET_PC_AUDIO )
			{
				mpDebugPrint("##Kevin Link %d %s",g_index_internetradio,pFileEntry[g_index_internetradio].Link);
				website =(BYTE *) Net_GetWebSite(pFileEntry[g_index_internetradio].Link);
				port = Net_GetWebSitePort(pFileEntry[g_index_internetradio].Link);
				urlparm = strstr(pFileEntry[g_index_internetradio].Link,"//");
			}
			urlparm+=2;
			urlparm = strstr(urlparm,"/");
			if( urlparm == NULL )
				urlparm = urlparmdefault;
			//MP_DEBUG2("urlparm %s %d",urlparm,port);
			//ulrparm;
			MP_DEBUG2("website %s urlparm %s",website,urlparm);
			//MP_DEBUG("SearchServerByName");
			ipaddr = (BYTE *)SearchServerByName(website);
			//giRadioStreamtype = 1;
			{
				retcode = iradio_download(website,urlparm,ipaddr,port);
			}
			mpDebugPrint("mretrycount %d",mretrycount);
			if ( (retcode &= BRECONNECTED) && mretrycount< SHOUTCAST_RETRY)
			{
			#if 0
				if( g_bXpgStatus == XPG_MODE_NET_PC_AUDIO )
				{
					if( g_index_internetradio < g_psNet_FileBrowser->dwNumberOfFile )
						g_index_internetradio++;
				}
			#endif	
				//mretrycount++;
				MP_DEBUG("iRadio::goto retry_con");
				goto retry_con;
			}
			if(Media_data.fending)
			{
			    mpDebugPrint("Media_data.fending = 1 goto retry_con");
				goto retry_con;
			}
			MP_DEBUG("iRadio::goto wait_to_start");
#ifdef HAVE_SHOUTCAST
			if(mretrycount< SHOUTCAST_RETRY)
			shoutcast_exit("/shoutcast");
#endif
			goto wait_to_start;
			return;
#if HAVE_VTUNER
	      case 1://vtuner
			pFileEntry = (ST_NET_FILEENTRY *)(g_psNet_FileBrowser->FileEntry);
			if(!strncmp(pFileEntry->ExtName,"WMA",3))
				audio_type = FILE_TYPE_WMA;
			
		    switch(audio_type)
		    	{
		    	case FILE_TYPE_MP3:
		retry_con1:
			MP_DEBUG("InterRadioStart\n");
			vtuner_iRadio_get_mp3_play_location(pFileEntry);

			if( g_bXpgStatus == XPG_MODE_NET_PC_AUDIO )
			{
				mpDebugPrint("Link %d %s",g_index_internetradio,pFileEntry[g_index_internetradio].Link);
				website =(BYTE *) Net_GetWebSite(pFileEntry[g_index_internetradio].Link);
				port = Net_GetWebSitePort(pFileEntry[g_index_internetradio].Link);
				urlparm = strstr(pFileEntry[g_index_internetradio].Link,"//");
			}
			urlparm+=2;
			urlparm = strstr(urlparm,"/");
			if( urlparm == NULL )
				urlparm = urlparmdefault;
			MP_DEBUG2("website %s urlparm %s",website,urlparm);
			ipaddr = (BYTE *)SearchServerByName(website);
			retcode = iradio_download(website,urlparm,ipaddr,port);
			mpDebugPrint("mretrycount %d",mretrycount);
			if ( (retcode &= BRECONNECTED) && mretrycount< 3)
			{
			#if 0
				if( g_bXpgStatus == XPG_MODE_NET_PC_AUDIO )
				{
					if( g_index_internetradio < g_psNet_FileBrowser->dwNumberOfFile )
						g_index_internetradio++;
				}
			#endif	
				mretrycount++;
				MP_DEBUG("iRadio::goto retry_con");
				goto retry_con1;
			}
			MP_DEBUG("iRadio::goto wait_to_start");
			if(retcode==0)
			vtuner_exit("/vtuner");
			goto wait_to_start;
			return;
		    	
			case FILE_TYPE_WMA:
		      	  if(vtuner_download()==0) 
				  vtuner_exit("/vtuner");
				  goto wait_to_start;
				  return;
	    }
#endif				  
			
		}
	}
	
}


