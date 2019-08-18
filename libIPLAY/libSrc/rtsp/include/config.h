#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
//#include <time.h>
#include <sys/types.h>
//#include <errno.h>   // TODO
//typedef unsigned long clockid_t;
//#define __restrict

#include <mpTrace_copy.h>
#include <log.h>


extern int errno;
struct _BPConsumer {
    int dod;
};

typedef struct _BPConsumer BPConsumer;

#define PACKAGE "MagicRtsp"
#define VERSION "0.1"

#define ATTR_UNUSED

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifndef ntohs
# define ntohl(x)	(x)
# define ntohs(x)	(x)
# define htonl(x)	(x)
# define htons(x)	(x)
#endif

#define time(tloc) mpx_time(tloc)

#define bind(s,n,l)   lwip_bind(s,n,l)
#define close(s)   lwip_closesocket(s)
#define recv(s,d,l,f)   lwip_recv(s,d,l,f)
#define recvfrom(s,d,l,f,fro,len)   lwip_recvfrom(s,d,l,f,fro,len)
#define sendto(s,d,sz,f,t,l)   lwip_sendto(s,d,sz,f,t,l)
#define send(s,d,sz,f)   lwip_send(s,d,sz,f)
#define setsockopt(s,l,o,v,len) lwip_setsockopt(s,l,o,v,len) 
#define listen(s,b) lwip_listen(s,b) 
#define accept(s,a,l) lwip_accept(s,a,l) 
#define connect(s,a,l) lwip_connect(s,a,l) 
#define getsockname(s,n,l) lwip_getsockname(s,n,l) 

#define free(p) mm_free(p)


extern char * rtsp_strdup(const char *s);
#define strdup(s) rtsp_strdup(s)

//#include "glib/gtypes.h"

/* Define this if live streaming is supported */
#define LIVE_STREAMING 1

#define _POSIX_MESSAGE_PASSING  1

//#define AV_DEBUG  1

#endif
