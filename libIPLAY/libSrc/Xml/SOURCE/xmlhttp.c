/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1

/************
 * TODO:
 *      1) Support persisent connection to speedup data transfer.
 *      2) Support socks server
 ************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "global612.h"
#include "netfs.h"
#include "netfs_pri.h"
//#include "in.h"
#include "..\..\lwip\include\net_sys.h"

#if NETWARE_ENABLE	
#define SHUT_RDWR SD_BOTH
#define MSG_NOSIGNAL 0
extern Net_App_State App_State;
/*
static u_char charmap[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};
*/
/*
static int inet_aton(const char *cp, struct in_addr *inp)
{
	in_addr_t	addr;

#if 0 //CJ
	addr = inet_addr(cp); //IP address to unsigned long  addr.s_addr = inet_addr("10.90.0.21")
#endif

	if (addr == INADDR_NONE)
	{
		return 0;
	}

	return 1;
}
*/
//#if !define(__MINGW32__)
/*
int strcasecmp(const char *s1, const char *s2)
{    
	return stricmp(s1, s2);
}
*/
//typedef unsigned char u_char;
#if 0
int strcasecmp(const char *s1,const char *s2)
{
	register u_char	*cm = charmap,
			*us1 = (u_char *)s1,
			*us2 = (u_char *)s2;

	while (cm[*us1] == cm[*us2++])
		if (*us1++ == '\0')
			return(0);
	return(cm[*us1] - cm[*--us2]);
}
#endif
/*
int strncasecmp(char *s1, char *s2, register int n)
{
	register u_char	*cm = charmap,
			*us1 = (u_char *)s1,
			*us2 = (u_char *)s2;

	while (--n >= 0 && cm[*us1] == cm[*us2++])
		if (*us1++ == '\0')
			return(0);
	return(n < 0 ? 0 : cm[*us1] - cm[*--us2]);
}*/
/*
int strcasecmp(const char *s1, const char *s2)
{
    return stricmp(s1, s2);
}*/
/*
int strncasecmp(const char *s1, const char *s2, int n)
{
    return strnicmp(s1, s2, n);
}*/
//#endif



/****** GNU Wrapping API ******/

#define O_BINARY 0

static int closesocket(int s)
{
    return close(s);
}

//#endif


enum HTTP_STATE
{
    HTTP_STATE_NULL,
    HTTP_STATE_CONNECTED,
    HTTP_STATE_HEADER,
    HTTP_STATE_BODY,
};


int http_client_init(http_info_t *http_info, const char *url, unsigned char *buffer, int buffer_size, proxy_info_t *proxy_info, socks_info_t *socks_info)
{

    int     ret;
   // struct timeval tv;
   int     port = 80;  /* default port number */
	
 
 //   struct sockaddr_in  remote_saddr;

    url_info_t  url_info;
    const char  *remote_host;
    int         remote_port;

    memset(http_info, 0, sizeof(http_info_t));

    http_info->header_handler.next  = &http_info->header_handler;
    http_info->header_handler.prev  = &http_info->header_handler;
    http_info->buffer		= buffer;
    http_info->buffer_size	= buffer_size;
    http_info->ptr          = buffer;

#if 1 //CJ
    if (proxy_info)
        memcpy(&http_info->proxy, proxy_info, sizeof(proxy_info_t));
	
    if (socks_info)
        memcpy(&http_info->socks, socks_info, sizeof(socks_info_t));

    // Connect to remote server 
	
    //http_info->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    http_info->sock_fd = 0;
    if (http_info->sock_fd < 0)
    {
        mpDebugPrint("create socket fail\n");
        return -NETFS_SOCKET_FAIL;
    }
/*
    tv.tv_sec       = 5;
    tv.tv_usec      = 0;

    setsockopt(http_info->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(http_info->sock_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    memset(&remote_saddr, 0, sizeof(remote_saddr));
*/
	utils_decode_url(url, &url_info);

    if (strncasecmp(url_info.scheme_s, "http://", 7))
    {
        mpDebugPrint("Unsupported method!!");
        //closesocket(http_info->sock_fd);
        return -NETFS_PROTOCOL_NOT_SUPPORT;
    }

    if (url_info.path_s)
        strncat(http_info->path, url_info.path_s, 256);     /* copy all string include search part */
    strncat(http_info->hostname, url_info.host_s, url_info.host_len);
    if (url_info.port_s)
        port = atoi(url_info.port_s);
    http_info->port = port;

    mpDebugPrint("http_client_init <=====   hostname= %s  path =%s port %d",http_info->hostname,http_info->path,http_info->port);

    /* connect to proxy server or http server directly */
    if (http_info->proxy.hostname[0])
    {
        remote_host = http_info->proxy.hostname;
        remote_port = http_info->proxy.port;
    }
    else
    {
        remote_host = http_info->hostname;
        remote_port = http_info->port;
    }
/*
    remote_saddr.sin_family = PF_INET;
    remote_saddr.sin_port = htons(remote_port);

    if (!inet_aton(remote_host, &remote_saddr.sin_addr))
    {
        struct hostent *host;

        // do dns query 
        host = gethostbyname(remote_host);
        if (host && host->h_length >= 4)
        {
            memcpy(&remote_saddr.sin_addr.s_addr, host->h_addr, 4);
        }
        else
        {
            mpDebugPrint("lookup dns fail!!\n");
            closesocket(http_info->sock_fd);
            return -NETFS_HOST_NOT_FOUND;
        }
    }

    ret = connect(http_info->sock_fd, (struct sockaddr *) &remote_saddr, sizeof(remote_saddr));
    if (ret < 0)
    {
        mpDebugPrint("fail to connect to remote peer!!\n");
        closesocket(http_info->sock_fd);
        return -NETFS_CONNECT_FAIL;
    }
*/
#endif

    http_info->state = HTTP_STATE_CONNECTED;











#if 0
    int     ret;
    struct timeval tv;
    int     port = 80;  /* default port number */
	
 
    struct sockaddr_in  remote_saddr;

    url_info_t  url_info;
    const char  *remote_host;
    int         remote_port;

    memset(http_info, 0, sizeof(http_info_t));

    http_info->header_handler.next  = &http_info->header_handler;
    http_info->header_handler.prev  = &http_info->header_handler;
    http_info->buffer		= buffer;
    http_info->buffer_size	= buffer_size;
    http_info->ptr          = buffer;

    if (proxy_info)
        memcpy(&http_info->proxy, proxy_info, sizeof(proxy_info_t));
	
    if (socks_info)
        memcpy(&http_info->socks, socks_info, sizeof(socks_info_t));

    /* Connect to remote server */
	
    http_info->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (http_info->sock_fd < 0)
    {
        printf("create socket fail\n");
        return -NETFS_SOCKET_FAIL;
    }

    tv.tv_sec       = 5;
    tv.tv_usec      = 0;

    setsockopt(http_info->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(http_info->sock_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    memset(&remote_saddr, 0, sizeof(remote_saddr));
    utils_decode_url(url, &url_info);

    if (strncasecmp(url_info.scheme_s, "http://", 7))
    {
        printf("Unsupported method!!\n");
        closesocket(http_info->sock_fd);
        return -NETFS_PROTOCOL_NOT_SUPPORT;
    }

    if (url_info.path_s)
        strncat(http_info->path, url_info.path_s, 256);     /* copy all string include search part */
    strncat(http_info->hostname, url_info.host_s, url_info.host_len);
    if (url_info.port_s)
        port = atoi(url_info.port_s);
    http_info->port = port;

    /* connect to proxy server or http server directly */
    if (http_info->proxy.hostname[0])
    {
        remote_host = http_info->proxy.hostname;
        remote_port = http_info->proxy.port;
    }
    else
    {
        remote_host = http_info->hostname;
        remote_port = http_info->port;
    }

    remote_saddr.sin_family = PF_INET;
    remote_saddr.sin_port = htons(remote_port);

    if (!inet_aton(remote_host, &remote_saddr.sin_addr))
    {
        struct hostent *host;

        /* do dns query */
        host = gethostbyname(remote_host);
        if (host && host->h_length >= 4)
        {
            memcpy(&remote_saddr.sin_addr.s_addr, host->h_addr, 4);
        }
        else
        {
            printf("lookup dns fail!!\n");
            closesocket(http_info->sock_fd);
            return -NETFS_HOST_NOT_FOUND;
        }
    }

    ret = connect(http_info->sock_fd, (struct sockaddr *) &remote_saddr, sizeof(remote_saddr));
    if (ret < 0)
    {
        printf("fail to connect to remote peer!!\n");
        closesocket(http_info->sock_fd);
        return -NETFS_CONNECT_FAIL;
    }

    http_info->state = HTTP_STATE_CONNECTED;


#endif
    return NETFS_OK;
}

 #if 0//CJ 
int http_client_init(http_info_t *http_info, const char *url, unsigned char *buffer, int buffer_size, proxy_info_t *proxy_info, socks_info_t *socks_info)
{


    int     ret;
    struct timeval tv;
    int     port = 80;  /* default port number */
	
 
    struct sockaddr_in  remote_saddr;

    url_info_t  url_info;
    const char  *remote_host;
    int         remote_port;

    memset(http_info, 0, sizeof(http_info_t));

    http_info->header_handler.next  = &http_info->header_handler;
    http_info->header_handler.prev  = &http_info->header_handler;
    http_info->buffer		= buffer;
    http_info->buffer_size	= buffer_size;
    http_info->ptr          = buffer;

    if (proxy_info)
        memcpy(&http_info->proxy, proxy_info, sizeof(proxy_info_t));
	
    if (socks_info)
        memcpy(&http_info->socks, socks_info, sizeof(socks_info_t));

    /* Connect to remote server */
	
    http_info->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (http_info->sock_fd < 0)
    {
        printf("create socket fail\n");
        return -NETFS_SOCKET_FAIL;
    }

    tv.tv_sec       = 5;
    tv.tv_usec      = 0;

    setsockopt(http_info->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(http_info->sock_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    memset(&remote_saddr, 0, sizeof(remote_saddr));
    utils_decode_url(url, &url_info);

    if (strncasecmp(url_info.scheme_s, "http://", 7))
    {
        printf("Unsupported method!!\n");
        closesocket(http_info->sock_fd);
        return -NETFS_PROTOCOL_NOT_SUPPORT;
    }

    if (url_info.path_s)
        strncat(http_info->path, url_info.path_s, 256);     /* copy all string include search part */
    strncat(http_info->hostname, url_info.host_s, url_info.host_len);
    if (url_info.port_s)
        port = atoi(url_info.port_s);
    http_info->port = port;

    /* connect to proxy server or http server directly */
    if (http_info->proxy.hostname[0])
    {
        remote_host = http_info->proxy.hostname;
        remote_port = http_info->proxy.port;
    }
    else
    {
        remote_host = http_info->hostname;
        remote_port = http_info->port;
    }

    remote_saddr.sin_family = PF_INET;
    remote_saddr.sin_port = htons(remote_port);

    if (!inet_aton(remote_host, &remote_saddr.sin_addr))
    {
        struct hostent *host;

        /* do dns query */
        host = gethostbyname(remote_host);
        if (host && host->h_length >= 4)
        {
            memcpy(&remote_saddr.sin_addr.s_addr, host->h_addr, 4);
        }
        else
        {
            printf("lookup dns fail!!\n");
            closesocket(http_info->sock_fd);
            return -NETFS_HOST_NOT_FOUND;
        }
    }

    ret = connect(http_info->sock_fd, (struct sockaddr *) &remote_saddr, sizeof(remote_saddr));
    if (ret < 0)
    {
        printf("fail to connect to remote peer!!\n");
        closesocket(http_info->sock_fd);
        return -NETFS_CONNECT_FAIL;
    }

    http_info->state = HTTP_STATE_CONNECTED;



    return NETFS_OK;
}
 #endif

int http_client_add_header_handler(http_info_t *http_info, http_header_handler_t *handler)
{
    handler->next = &http_info->header_handler;
    handler->prev = http_info->header_handler.prev;
    http_info->header_handler.prev->next = handler;
    http_info->header_handler.prev = handler;

    return NETFS_OK;
}

int http_client_GCE_request(http_info_t *http_info)
{
    int len = 0;
    char request[512];
    int line_length;
    int ret;

    if (http_info->proxy.hostname[0])
    {
        snprintf(request, 512,
                "GET http://%s:%d%s HTTP/1.1\r\n"
                "Referer: http://%s/\r\n"
                "x-device-id: 01-23-45-67-89-AB-CD-EF\r\n"
                "x-device-type: 01-23-45-67\r\n"
                "x-screen-size: 800x480\r\n"
                "x-button-name: center, left, right, down, up\r\n"
                "User-Agent: HalfBrowser/1.0\r\n"
                "Accept: image/gif, image/jpeg, image/png\r\n"
                "\r\n",
                http_info->hostname,
                http_info->port,
                http_info->path,
                http_info->hostname);
    }
    else
    {
        snprintf(request, 512,
                "GET %s HTTP/1.1\r\n"
                "Connection: close\r\n"
                "Host: %s\r\n"
                "Referer: http://%s/\r\n"
                "x-device-id: 01-23-45-67-89-AB-CD-EF\r\n"
                "x-device-type: 01-23-45-67\r\n"
                "x-screen-size: 800x480\r\n"
                "x-button-name: center, left, right, down, up\r\n"
                "User-Agent: HalfBrowser/1.0\r\n"
                "Accept: image/gif, image/jpeg, image/png\r\n"
                "\r\n",
                http_info->path,
                http_info->hostname,
                http_info->hostname);
    }
#if 0 //CJ
    len = send(http_info->sock_fd, request, strlen(request), 0);
#else
#if 0
    App_State.dwState |= NET_RECVHTTP;
    ret = Net_SendHttp(http_info->hostname,http_info->port,http_info->path,0,0);
    if (ret <= 0)
        return -NETFS_SEND_FAIL;
	
    len = App_State.dwTotallen;
	
	App_State.dwState &= ~NET_RECVHTTP;	
#endif
#endif

    if (len <= 0)
        return -NETFS_SEND_FAIL;

    http_info->state = HTTP_STATE_HEADER;

    return len;
}

int http_client_request(http_info_t *http_info)
{
    int len = 0;
    char request[256];
    int line_length;
    int ret;
/*
    if (http_info->proxy.hostname[0])
    {
        snprintf(request, 256,
                "GET http://%s:%d%s HTTP/1.1\r\n"
                "Referer: http://%s/\r\n"
                "\r\n",
                http_info->hostname,
                http_info->port,
                http_info->path,
                http_info->hostname);
    }
    else
    {
        snprintf(request, 256,
                "GET %s HTTP/1.1\r\n"
                "Connection: close\r\n"
                "Host: %s\r\n"
                "Referer: http://%s/\r\n"
                "\r\n",
                http_info->path,
                http_info->hostname,
                http_info->hostname);
    }
 */   
#if 0 //CJ	
    len = send(http_info->sock_fd, request, strlen(request), 0);
#else
#if 0
    App_State.dwState |= NET_RECVHTTP;
    ret = Net_SendHttp(http_info->hostname,http_info->port,http_info->path,0,0);
    App_State.dwState &= ~NET_RECVHTTP;	
    if (ret <= 0)
        return -NETFS_SEND_FAIL;
	
    len = App_State.dwTotallen;	
#endif	
#endif

	if (len <= 0)
        return -NETFS_SEND_FAIL;

    http_info->state = HTTP_STATE_HEADER;

    return len;
}

int http_client_request_partial(http_info_t *http_info, int range1, int range2)
{
    int len = 0;
    int ret ;
    char request[512];
    char *data; 
    XML_BUFF_link_t *ptr;
/*
    if (http_info->proxy.hostname[0])
    {
        snprintf(request, 512,
                "GET http://%s:%d%s HTTP/1.1\r\n"
                "Referer: http://%s/\r\n"
                "Range: bytes=%d-%d\r\n"
                "\r\n",
                http_info->hostname,
                http_info->port,
                http_info->path,
                http_info->hostname,
                range1, range2);
    }
    else
    {
        snprintf(request, 512,
                "GET %s HTTP/1.1\r\n"
                "Connection: close\r\n"
                "Host: %s\r\n"
                "Referer: http://%s/\r\n"
                "Range: bytes=%d-%d\r\n"
                "\r\n",
                http_info->path,
                http_info->hostname,
                http_info->hostname,
                range1, range2);
    }
*/    
#if 0 //CJ	
    len = send(http_info->sock_fd, request, strlen(request), 0);
#else
#if 0
    App_State.dwState |= NET_RECVHTTP;
    ret = Net_SendHttp(http_info->hostname,http_info->port,http_info->path,range1,range2);
    if (ret <= 0)
        return -NETFS_SEND_FAIL;
	
    len = App_State.dwTotallen;
	
	App_State.dwState &= ~NET_RECVHTTP;	
#endif
#endif

	if (len <= 0)
        return -NETFS_SEND_FAIL;

    http_info->state = HTTP_STATE_HEADER;

    return len;
}

static void http_client_process_header_line(http_info_t *http_info, const char *line_buffer, int line_length)
{
    int         namelen;
    const char  *value;
    http_header_handler_t   *header_handler = http_info->header_handler.next;

    while (header_handler != &http_info->header_handler)
    {
        namelen = strlen(header_handler->name);
        if (line_length > namelen &&
            !strncasecmp(header_handler->name, line_buffer, namelen))
        {
            value = line_buffer + namelen;
            if (*value == ':')
            {
                value ++;
                while (*value == ' ')
                    value ++;
                header_handler->handler(http_info, header_handler->name, value);
            }
        }

        header_handler = header_handler->next;
    }
}

static int http_client_buffer_more_data(http_info_t *http_info)
{
    int     readlen;
    int     buffer_space;
    unsigned char *buffer_tail;
    int i;
	BYTE BUFF[10];

    buffer_tail = http_info->ptr + http_info->buffer_length;
    if (buffer_tail >= http_info->buffer + http_info->buffer_size)
        buffer_tail -= http_info->buffer_size;

    buffer_space = http_info->buffer_size - http_info->buffer_length;
    if (buffer_tail + buffer_space > http_info->buffer + http_info->buffer_size)
        buffer_space = http_info->buffer + http_info->buffer_size - buffer_tail;

#if 0 //CJ
    readlen = recv(http_info->sock_fd, buffer_tail, buffer_space, 0);
#else
    readlen = App_State.dwTotallen;
    buffer_tail = App_State.XML_BUF;
#endif

    if (readlen < 0)
        return -NETFS_RECV_FAIL;

    if (readlen > 0)
    {
       /* int i;
        for (i = 0; i < readlen; i++)
            mpDebugPrint("%c", buffer_tail[i]);
        http_info->buffer_length += readlen;
        */
        for (i = 0; i < readlen; i++)
        {
        	sprintf(BUFF,"%c", buffer_tail[i]);
            UartOutText(BUFF);
        }
        http_info->buffer_length += readlen;
    }

    return readlen;
}

/**
 * @retval >=0 copied length
 * @retval < 0 error
 */
static int http_client_getline(http_info_t *http_info, unsigned char *line_buffer, int line_size)
{
    enum LINE_STATE
    {
        LINE_NULL,
        LINE_RETURN,
        LINE_FEED,
    };
    int ret;
    int count = 0;
    int line_state = LINE_NULL;

    while ( count < line_size && line_state != LINE_FEED )
    {
        if (http_info->buffer_length == 0)
        {
            ret = http_client_buffer_more_data(http_info);
            if (ret <= 0)
                return ret;
        }

        if (line_state == LINE_NULL)
        {
            if (*http_info->ptr == '\r')
            {
                line_state = LINE_RETURN;
            }
            else if (*http_info->ptr == '\n')
            {
                line_state = LINE_FEED;
            }
            else
            {
                *line_buffer = *http_info->ptr;
                count ++;
                line_buffer ++;
            }
        }
        else if (line_state == LINE_RETURN)
        {
            if (*http_info->ptr != '\n')
                break;
            line_state = LINE_FEED;
        }

        http_info->buffer_length --;
        http_info->ptr ++;
        if (http_info->ptr == http_info->buffer + http_info->buffer_size)
            http_info->ptr = http_info->buffer;
    }
    *line_buffer = 0;

    return count;
}

/**
 * @retval >=0 copied length
 * @retval < 0 error
 */
static int http_client_getdata(http_info_t *http_info, unsigned char *data, int datalen)
{
    int copied_length = 0;
    int ret;
    int data_space;
    int i;

    copied_length = 0;
    while (datalen > 0)
    {
        if (http_info->buffer_length == 0)
        {
            ret = http_client_buffer_more_data(http_info);
            if (ret < 0)
                return ret;
            if (ret == 0)
                break;
        }

        /* estimate properly coping data length */
        data_space      = datalen;

        if (data_space > http_info->buffer_length)
            data_space      = http_info->buffer_length;

        if (http_info->ptr + data_space > http_info->buffer+http_info->buffer_size)
            data_space      = http_info->buffer+http_info->buffer_size - http_info->ptr;

        /* copy data */
        memcpy(data, http_info->ptr, data_space);

        /* adjust data information */
        http_info->ptr  += data_space;
        while (http_info->ptr >= http_info->buffer + http_info->buffer_size)
            http_info->ptr  -= http_info->buffer_size;

        http_info->buffer_length -= data_space;
        copied_length   += data_space;
        datalen         -= data_space;
        data            += data_space;
    }

    return copied_length;
}

int http_client_recv_header(http_info_t *http_info)
{
    int line_length;
    unsigned char line_buffer[256];
    int header_count = 0;
    const char *status_str;

    /* Get response header */

    /* process Status Line of response data */
    line_length = http_client_getline(http_info, line_buffer, 256);
    if (line_length < 0)
    {
        return line_length;
    }
    status_str = strchr((char *)line_buffer, ' ');
    if (!status_str)
    {
        return -NETFS_PROTOCOL_FAIL;
    }
    http_info->status_code = atoi(status_str+1);

    while (http_info->state == HTTP_STATE_HEADER)
    {
        line_length = http_client_getline(http_info, line_buffer, 256);
        if (line_length < 0)
        {
            return line_length;
        }

        /* empty line */
        if (!line_length)
        {
            http_info->state = HTTP_STATE_BODY;
            continue;
        }

        while (line_length > 0 && http_info->state == HTTP_STATE_HEADER)
        {
            header_count ++;
            http_client_process_header_line(http_info, (char *) line_buffer, line_length);

            line_length = http_client_getline(http_info, line_buffer, 256);
            if (line_length < 0)
            {
                return line_length;
            }

            /* empty line */
            if (!line_length)
            {
                http_info->state = HTTP_STATE_BODY;
                continue;
            }
        }
    }

    return header_count;
}

int http_client_recv_data(http_info_t *http_info, unsigned char *data, int datalen)
{
    int line_length;
    int copied_length = 0;
    unsigned char line_buffer[256];

    /* Get response body */
    if (http_info->state == HTTP_STATE_BODY)
    {
        while (copied_length < datalen)
        {
            int wantlen = datalen;
            int readlen;

            if (http_info->chunked_transfer)
            {
                if (http_info->chunked_size <= 0)
                {
                    /* Try to Read Next Chunk */

                    line_length = http_client_getline(http_info, line_buffer, 256);
                    /* abnormal */
                    if (line_length < 0)
                        break;

                    http_info->chunked_size = strtol((char *)line_buffer, NULL, 16);
                    /* last-chunk */
                    if (!http_info->chunked_size)
                        break;
                }

                if (wantlen >= http_info->chunked_size)
                    wantlen = http_info->chunked_size;
            }

            readlen = http_client_getdata(http_info, data+copied_length, wantlen);
            if (readlen <= 0)
                break;
            copied_length += readlen;
            datalen -= readlen;

            if (http_info->chunked_transfer)
            {
                http_info->chunked_size -= readlen;
                /* discard (0d) (0a) */
                if (http_info->chunked_size <= 0)
                {
                    line_length = http_client_getline(http_info, line_buffer, 256);
                    /* line_length must equal to ZERO */
                }
            }
        }
    }

    return copied_length;
}

int http_client_exit(http_info_t *http_info)
{
    //closesocket(http_info->sock_fd);
    http_info->state = HTTP_STATE_NULL;

    return NETFS_OK;
}

void http_header_location(http_info_t *http_info, const char *name, const char *value)
{
    snprintf(http_info->location, 256, "%s", value);
}

void http_header_last_modified(http_info_t *http_info, const char *name, const char *value)
{
    /* store last-modified date */
    struct tm tm;

    /* eg: "Wed Jun 30 21:49:08 1993\n" */
    memset(&tm, 0, sizeof(struct tm));
    if (strptime(value, "%a %b %d %H:%M:%S %Y", &tm))
        http_info->modified_time = mktime(&tm);
}

void http_header_transfer_encoding(http_info_t *http_info, const char *name, const char *value)
{
    if (!strcasecmp(value, "chunked"))
    {
        http_info->chunked_transfer = 1;
    }
}

void http_header_content_length(http_info_t *http_info, const char *name, const char *value)
{
    if (value)
    {
        http_info->content_length = atoi(value);
    }
}

void http_header_accept_ranges(http_info_t *http_info, const char *name, const char *value)
{
    if (!strncasecmp(value, "bytes", 5))
    {
        http_info->accept_ranges = 1;
    }
}

void http_header_connection(http_info_t *http_info, const char *name, const char *value)
{
	/* TODO: Support persisent connection */
}

const char *utils_decode_as_filepath(const char *file_url)
{
    static char filepath[PATH_MAX];
    char *path;
    char ch;

    if (!file_url)
        return NULL;

    path = filepath;
    while ((ch = *file_url))
    {
        /* Excluded Characters */
        if (ch <= ' ')
            break;
        if (ch == '<' || ch == '>' || ch == '#' || ch == '\"')
            break;
        if (ch == '{' || ch == '}' || ch == '|' || ch == '^')
            break;
        if (ch == '[' || ch == ']' || ch == '`')
            break;

        if (ch == '%')
        {
            /* Escaped Encoding */
            ch = *(file_url+1);
            if ('0' <= ch && ch <= '9')
                *path = (ch - '0')<<4;
            else if ('a' <= ch && ch <= 'z')
                *path = (ch - 'a' + 10)<<4;
            else if ('A' <= ch && ch <= 'Z')
                *path = (ch - 'A' + 10)<<4;

            ch = *(file_url+2);
            if ('0' <= ch && ch <= '9')
                *path |= ch - '0';
            else if ('a' <= ch && ch <= 'z')
                *path |= ch - 'a' + 10;
            else if ('A' <= ch && ch <= 'Z')
                *path |= ch - 'A' + 10;

            file_url += 3;
        }
        else
        {
            *path = *file_url;
            file_url++;
        }

        path ++;
    }
    *path = 0;

    return filepath;
}


void utils_decode_url(const char *old_url, url_info_t *url_info)
{
    const char  *access_s;
    int         access_len;
    const char  *access1_s;
    int         access1_len;

    memset(url_info, 0, sizeof(struct url_info_s));

    /* fetch scheme */
    url_info->scheme_s = old_url;
    while (*old_url && *old_url != ':')
        old_url ++;
    url_info->scheme_len = old_url - url_info->scheme_s;

    /* check delimiter */
    if (strncmp(old_url, "://", 3))
        return;
    old_url += 3;

    /* fetch user, password, host, port */
    access_s = NULL;
    access1_s = NULL;
    while (*old_url && *old_url != '/')
    {
        if (!access_s)
            access_s = old_url;

        if (*old_url == ':')
        {
            access_len = old_url - access_s;

            if (access1_s)
                return;

            access1_s = access_s;
            access1_len = access_len;
            access_s = NULL;
        }
        else if (*old_url == '@')
        {
            if (url_info->host_s)
                return;

            access_len = old_url - access_s;

            /* fetch user, password */
            if (access1_s)
            {
                url_info->user_s = access1_s;
                url_info->user_len = access1_len;
                url_info->password_s = access_s;
                url_info->password_len = access_len;
            }
            else
            {
                url_info->user_s = access_s;
                url_info->user_len = access_len;
            }

            access1_s = NULL;
            access_s = NULL;
        }

        old_url ++;
    }
    access_len = old_url - access_s;

    /* fetch host, port */
    if (access1_s)
    {
        url_info->host_s = access1_s;
        url_info->host_len = access1_len;
        url_info->port_s = access_s;
        url_info->port_len = access_len;
    }
    else
    {
        url_info->host_s = access_s;
        url_info->host_len = access_len;
    }

    /* fetch path */
    if (*old_url)
    {
//        old_url ++;

        url_info->path_s = old_url;
        while (*old_url && *old_url != '?')
            old_url ++;
        url_info->path_len = old_url - url_info->path_s;

        if (*old_url == '?')
        {
            old_url ++;
            url_info->search_s = old_url;
            while (*old_url)
                old_url ++;
            url_info->search_len = old_url - url_info->search_s;
        }
    }
}

/*
char *utils_formalize_url(const char *old_url, struct in_addr server_in)
//char *utils_formalize_url(const char *old_url, struct in_addr server_in)
{
    static char formal_url[128];
    int     index;
    struct url_info_s url_info;

    index = 0;

    utils_decode_url(old_url, &url_info);

    if (!url_info.scheme_s)
        return NULL;

    index = 0;
    formal_url[index] = 0;
    memcpy(formal_url+index, url_info.scheme_s, url_info.scheme_len);
    index += url_info.scheme_len;
    memcpy(formal_url+index, "://", 3);
    index += 3;
    if (url_info.user_s || url_info.password_s)
    {
        if (url_info.user_s)
        {
            memcpy(formal_url+index, url_info.user_s, url_info.user_len);
            index += url_info.user_len;
        }

        if (url_info.password_s)
        {
            memcpy(formal_url+index, ":", 1);
            index++;
            memcpy(formal_url+index, url_info.password_s, url_info.password_len);
            index += url_info.password_len;
        }

        memcpy(formal_url+index, "@", 1);
        index++;
    }

    if (!url_info.host_len)
    {
        url_info.host_s = inet_ntoa(server_in);
        url_info.host_len = strlen(url_info.host_s);
    }
    memcpy(formal_url+index, url_info.host_s, url_info.host_len);
    index += url_info.host_len;
    if (url_info.port_s)
    {
        memcpy(formal_url+index, ":", 1);
        index++;
        memcpy(formal_url+index, url_info.port_s, url_info.port_len);
        index += url_info.port_len;
    }
    memcpy(formal_url+index, "/", 1);
    index++;
    memcpy(formal_url+index, url_info.path_s, url_info.path_len);
    index += url_info.path_len;

    return formal_url;
}
*/
#endif