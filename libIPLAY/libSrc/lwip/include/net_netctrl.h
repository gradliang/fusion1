#ifndef __NET_NETCTRL_H

#include "net_packet.h"
#include "os.h"

//net buffer control
typedef struct
{
    U32 next;
    U32 prev;
    U32 u32Data;
    U16 u16DstPort;
    U16 u16Reserved;
} ST_NET_BUF_NODE;

typedef struct
{
    U32 packet;
    U08* dataPtr;
    U16 u16TotalLength; //total data length in this packet
    U16 u16DataLength; //un-read data length
    U08 u08LastPacket;                          /* indicates TCP FIN is received */
    U08 u08Reserved;
    U16 u16FromPort;
    U32 u32FromIp;
    U08 u08Address[8];                          /* support packet socket */
} ST_NET_BUF_ACCESS_INFO;

typedef struct
{
    ST_NET_BUF_NODE* head;
    ST_NET_BUF_NODE* tail;
    U16 u16ItemNum;
    U32 u32DataLen;                             /* total data length in all input buffers */
    U16 u16Type;
    U16 u16Family;                          /* support packet socket */
    ST_NET_BUF_ACCESS_INFO stAccessInfo;
} ST_NET_BUF_LIST;

#define NET_BUFFER_MAX_PACKET_NUM           64


// message definition
#define NETCTRL_MSG_PACKET_RECEIVE          0x00000001
#define NETCTRL_MSG_NET_PACKET_SEND         0x00000002
#define NETCTRL_MSG_IP_PACKET_SEND          0x00000003
#define NETCTRL_MSG_UDP_PACKET_SEND         0x00000004
#define NETCTRL_MSG_TCP_FAST_TIMER          0x00000005
#define NETCTRL_MSG_TCP_SLOW_TIMER          0x00000006
#define NETCTRL_MSG_TCP_RECEIVED            0x00000007
#define NETCTRL_MSG_GENERAL_CALLBACK        0x00000008


typedef void (*NET_CALLBACK_PROC)();
typedef void (*TCP_FAST_TIMER_PROC)();
typedef void (*TCP_SLOW_TIMER_PROC)();
typedef void (*TCP_ACK_NOW_CALLBACK)(U32 param);
typedef void (*TCP_OUTPUT_CALLBACK)(U32 param);
typedef void (*TCP_RECEIVED_CALLBACK)(U32 param, U16 param2);

//net timer
#define NET_TIMER_MAX       6
#ifdef MP600
//#define NET_TIMER_250_MS    2
#define NET_TIMER_250_MS    1             /* on MP65x/MP66x platforms */
#else
#define NET_TIMER_250_MS    1
#endif
#define NET_TIMER_500_MS    (NET_TIMER_250_MS*2)
#define NET_TIMER_1_SEC     (NET_TIMER_250_MS*4)


typedef void (*NET_TIMER_CALLBACK)();

typedef struct
{
    BOOL used;
    BOOL running;
    NET_TIMER_CALLBACK proc;
    U32 u32Interval;
    U32 u32Counter;
} ST_NET_TIMER_REC;

/* layer 2 */
struct l2_statistics {
	u32 rx_pkts;
	u32 rx_dropped;
	u32 rx_arp;
	u32 rx_bcast_arp;
	u32 rx_ucast_arp;
};
extern struct l2_statistics l2_stat;

S32 NetCtrlInit();

S32 NetCtrlStart();
void NetCtrlStop();
S32 NetTimerInstall(NET_TIMER_CALLBACK proc, U32 interval);
void NetTimerRun(U08 u08TimerId);
void NetTimerStop(U08 u08TimerId);
void NetTimerDelete(U08 u08TimerId);

U08 NetCtrlTaskIdGet();
U08 NetCtrlMessageIdGet();

//net buffer
void NetBufferInit();
void NetBufferTypeSet(U16 sid, U16 u16Type);
void NetBufferFamilySet(U16 sid, U16 u16Family);
S32 NetBufferPacketPush(U16 sid, U32 packet);
U32 NetBufferPacketPop(U16 sid);
U16 NetBufferPacketNumGet(U16 sid);
U16 NetBufferFromPortGet(U16 sid);
U32 NetBufferFromIpGet(U16 sid);
U32 NetBufferFromMacAddrGet(U16 sid);
S32 NetBufferDataRead(U16 sid, U08* buffer, U32 bufferSize, struct sockaddr *from);
void NetBufferRelease(U16 sid);
U32 NetBufferDataLengthGet(u16 sid);

S32 NetBufferPacketReceive(ST_NET_PACKET* packet);

#endif
