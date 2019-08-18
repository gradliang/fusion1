#ifndef __NET_STREAM__H__
#define __NET_STREAM__H__

#if (HAVE_YOUTUBE||HAVE_YOUKU3G)
#define NETSTREAM_MAX_BUFSIZE  (1024*1024)//(512*1024)//256*1024//(512*1024)
#else
#define NETSTREAM_MAX_BUFSIZE  (512*1024)
#endif
void NetStream(void);

/*
 * Event defines for NETWORK_STREAM_EVENT
 */
#define audio_event       	BIT0
#define video_event       	BIT1
#define video_ready_event   BIT2
#define network_link_down   BIT3
#define network_receive     BIT4
//for audio
#define	IRADIO_DATA_HIGH	(240 * 0x400)
#define	IRADIO_DATA_LOW		(24 * 0x400)
#define	IRADIO_DATA_MAX		(500 * 0x400)
#define	UPNP_DATA_HIGH		(120 * 0x400)
#define IRADIO_MAX_BUFSIZE_3_O_4 ((NETSTREAM_MAX_BUFSIZE *3)/4)

#define ASF_RECEIVE_MAX  18 
#define ASF_RECEIVE_Min  8 

#define	IRADIO_VTUNER_DATA_HIGH	(ASF_RECEIVE_MAX * ASF_HEADER_SIZE)// 8 *8192
#define	IRADIO_VTUNER_DATA_LOW	(ASF_RECEIVE_Min * ASF_HEADER_SIZE)//  4* 8192

#define SOCKET_WRITE_READY  0x01
#define SOCKET_READ_READY   0x02

typedef struct asx_href_s {
    char    href[256+1];
}asx_href_t;

struct asx_control_s {
    U08 ASX_tag;
    U08 ENTRY_tag;
    U08 REF_tag;
    asx_href_t    asx_href[2];
};

#endif
