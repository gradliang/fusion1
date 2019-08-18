#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/time.h>
#include <time.h>
#include "netfs.h"
#include "netfs_pri.h"
#include "..\..\xml\include\in.h"

#include "mpTrace.h"


typedef int socklen_t;
typedef unsigned long in_addr_t;

#define SHUT_RDWR SD_BOTH
#define MSG_NOSIGNAL 0

#if 0
static int inet_aton(const char *cp, struct in_addr *inp)
{
	in_addr_t	addr;

	addr = inet_addr(cp);

	if (addr == INADDR_NONE)
	{
		return 0;
	}

	return 1;
}
#endif

enum Packet_Type
{
    DISCOVERY_PACKET,
    RESPONSE_PACKET,
};

enum Attribute_Type
{
    NAME_ATTRIBUTE,
    URL_ATTRIBUTE,
};

struct discovery_packet
{
    unsigned char   version;
    unsigned char   type;
    unsigned short  length;
};

struct response_packet
{
    unsigned char   version;
    unsigned char   type;
    unsigned short  length;
    unsigned char   name_attr;
    unsigned char   name_length;
    char        name_str[32];
    unsigned char   url_attr;
    unsigned char   url_length;
    char        url_str[64];
};

static struct service_entry    service_list = {
    .next   = &service_list,
    .prev   = &service_list,
};

static int service_count;

static int add_service_list(struct service_entry *new_service)
{
    struct service_entry *service_info;

    service_info = (struct service_entry *) ext_mem_malloc(sizeof(struct service_entry));
    if (!service_info)
    {
        MP_ALERT("error no malloc\n");
        return -1;
    }
    memcpy(service_info, new_service, sizeof(struct service_entry));

    service_info->next = &service_list;
    service_info->prev = service_list.prev;
    service_list.prev->next = service_info;
    service_list.prev = service_info;

    service_count ++;

    MP_DEBUG("Add service %d from '%s' name: '%s' url: '%s'\n",
           service_count,
           new_service->ip_str,
           new_service->name_str,
           new_service->url_str);
    //fflush(NULL);
    
    return 0;
}

int reset_service_list(void)
{
    struct service_entry *service, *next;
    int count;

    count = 0;
    service = service_list.next;
    while (service != &service_list)
    {
        next = service->next;
        ext_mem_free(service);
        count ++;

        service = next;
    }

    service_list.next = &service_list;
    service_list.prev = &service_list;

    service_count = 0;

    return count;
}
#if 0
static int discovery_client(int port)
{
    int sockfd;
    struct sockaddr_in saddr;
    int ret = 0;

    service_list.next = service_list.prev = &service_list;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "%s: Can't create socket.\n", strerror(errno));
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(sockfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
    if (ret)
    {
        fprintf(stderr, "Bind error\n");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

static int send_discovery_packet(int sockfd, int port)
{
    struct discovery_packet discovery_data;
    struct sockaddr_in discovery_saddr;
    socklen_t discovery_slen;
    const int on = 1;
    int ret;

    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

    /* send discovery packet */
    memset(&discovery_data, 0, sizeof discovery_data);
    memset(&discovery_saddr, 0, sizeof discovery_saddr);
    discovery_saddr.sin_family = PF_INET;
    discovery_saddr.sin_port = htons(port);
    discovery_saddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    ret = sendto(sockfd, (char *)&discovery_data, sizeof discovery_data, 0,
                 (struct sockaddr *)&discovery_saddr, sizeof discovery_saddr);
    if (ret <= 0)
    {
        perror("send error");
        return -1;
    }

    return 0;
}

/**
 * return 0: terminate
 * return >0: continue wait
 * return <0: error
 */
static int wait_service_packet(int sockfd, struct timeval *timeout, struct response_packet **service_data, struct in_addr *in)
{
    static unsigned char response_buffer[512];
    struct sockaddr_in server_saddr;
    socklen_t server_slen;
    int ret;
    fd_set rfds;

    /* Wait response packet */
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    ret = select(sockfd+1, &rfds, NULL, NULL, timeout);
    if (ret < 0)
    {
        perror("select error");
        return -1;
    }
    if (ret == 0)
    {
        return 0;
    }

    /* Get response packet */
    memset(&server_saddr, 0, sizeof server_saddr);
    server_slen = sizeof server_saddr;
    ret = recvfrom(sockfd, response_buffer, 512, 0,
                    (struct sockaddr *)&server_saddr, &server_slen);
    if (ret < 0)
    {
        perror("recv error");
        return -1;
    }

    *in = server_saddr.sin_addr;
    *service_data = (struct response_packet *)&response_buffer;

    return 1;
}

static void discovery_start(int sockfd, int port)
{
    struct service_entry    service_info;
    struct response_packet *service_data;
    struct in_addr server_in;
    struct timeval tv1, tv2;
    int scan_msec;
    int ret;

    ret = send_discovery_packet(sockfd, port);
    if (ret < 0)
    {
        return;
    }

    /* receive response packet for 5 sec */
    scan_msec = 5000;
    gettimeofday(&tv1, NULL);
    while (1)
    {
        struct timeval timeout;
        int elapse_msec;

        /* Calculate discovery timeout */
        gettimeofday(&tv2, NULL);
        memset(&timeout, 0, sizeof timeout);
        elapse_msec = (tv2.tv_sec - tv1.tv_sec) * 1000;
        if (tv2.tv_usec > tv1.tv_usec)
            elapse_msec = (tv2.tv_usec - tv1.tv_usec) / 1000;
        timeout.tv_sec = (scan_msec - elapse_msec) / 1000;
        elapse_msec += timeout.tv_sec*1000;
        if (elapse_msec < scan_msec)
            timeout.tv_usec = (scan_msec - elapse_msec) * 1000;
        printf("timeout sec:%ld, usec:%ld\n", timeout.tv_sec, timeout.tv_usec);

        ret = wait_service_packet(sockfd, &timeout, &service_data, &server_in);
        if (ret <= 0)
            break;

        if (service_data && service_data->version == 0 && service_data->type == 1)
        {
            strncpy(service_info.name_str, service_data->name_str, 32);
            strncpy(service_info.ip_str, inet_ntoa(server_in), 16);
            strncpy(service_info.url_str, utils_formalize_url(service_data->url_str, server_in), 64);
//            strncpy(service_info.url_str, service_data->url_str, 64);

            add_service_list(&service_info);
        }
    }

    printf("search discovery end\n");
    fflush(NULL);
}
#endif
service_entry_t *do_service_discovery(void)
{
    int discovery_fd;

    reset_service_list();
/*
    discovery_fd = discovery_client(6151);

    discovery_start(discovery_fd, 6150);

    closesocket(discovery_fd);
*/
    return &service_list;
}
