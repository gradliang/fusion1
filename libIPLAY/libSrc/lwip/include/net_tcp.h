#ifndef __NET_TCP_H
#define __NET_TCP_H

#include "typedef.h"
//#include "net_nic.h"
#include "net_ip.h"

#define TCP_SEQ_LT(a,b)     ((S32)((a)-(b)) < 0)
#define TCP_SEQ_LEQ(a,b)    ((S32)((a)-(b)) <= 0)
#define TCP_SEQ_GT(a,b)     ((S32)((a)-(b)) > 0)
#define TCP_SEQ_GEQ(a,b)    ((S32)((a)-(b)) >= 0)
#define TCP_SEQ_BETWEEN(a,b,c) (TCP_SEQ_GEQ(a,b) && TCP_SEQ_LEQ(a,c))

#define MIN_TCP_HLEN            (sizeof(ST_TCP_PACKET) - MAX_TCP_DATA)
#define TCP_DEF_MTU             512

#define I_AM_DEBUG_SYMBOL       0x1022

#ifndef TCP_MSS
#define TCP_MSS                 1460
#endif

/** This returns a TCP header option for MSS in an u32_t */
#define TCP_BUILD_MSS_OPTION()  htonl(((u32_t)2 << 24) | \
                                ((u32_t)4 << 16) | \
                                (((u32_t)TCP_MSS / 256) << 8) | \
                                (TCP_MSS & 255))

//TCP config for ethernet
#define TCP_WINDOW_SIZE       (16*TCP_MSS)
#define TCP_SEND_BUFFER       (16*TCP_MSS)
#define TCP_SEND_QUEUE_LEN    (8*(TCP_SEND_BUFFER/TCP_MSS))

#define TCP_WND                 TCP_WINDOW_SIZE

#define TCP_TIMER_INTERVAL      250 //ms
#define TCP_FAST_INTERVAL       TCP_TIMER_INTERVAL
#define TCP_SLOW_INTERVAL       (2*TCP_TIMER_INTERVAL)

#define TCP_KEEPDEFAULT         7200000                       /* KEEPALIVE timer in miliseconds */
#define TCP_KEEPINTVL           75000                         /* Time between KEEPALIVE probes in miliseconds */
#define TCP_KEEPCNT             9                             /* Counter for KEEPALIVE probes */
#define TCP_MAXIDLE             (TCP_KEEPCNT * TCP_KEEPINTVL) /* Maximum KEEPALIVE probe time */

#define TCP_FIN_WAIT_TIMEOUT    20000 /* milliseconds */
#define TCP_SYN_RCVD_TIMEOUT    20000 /* milliseconds */

#define TCP_OOSEQ_TIMEOUT       6U

#define TCP_MSL                 60000

// TCP option flags
#define TCP_FIN                 0x01
#define TCP_SYN                 0x02
#define TCP_RST                 0x04
#define TCP_PUSH                0x08
#define TCP_ACK                 0x10
#define TCP_URGE                0x20

// TCP status
#define TCP_NO_ERR              NO_ERR
#define TCP_ABORT               -1
#define TCP_RESET               -2
#define TCP_CLOSED              -3

/* Definitions for error constants. */

#define ERR_OK    0      /* No error, everything OK. */
#define ERR_MEM  -1      /* Out of memory error.     */
#define ERR_BUF  -2      /* Buffer error.            */


#define ERR_ABRT -3      /* Connection aborted.      */
#define ERR_RST  -4      /* Connection reset.        */
#define ERR_CLSD -5      /* Connection closed.       */
#define ERR_CONN -6      /* Not connected.           */

#define ERR_VAL  -7      /* Illegal value.           */

#define ERR_ARG  -8      /* Illegal argument.        */

#define ERR_RTE  -9      /* Routing problem.         */

#define ERR_USE  -10     /* Address in use.          */

#define ERR_IF   -11     /* Low-level netif error    */
#define ERR_ISCONN -12   /* Already connected.       */

#define TCP_HEADER(seg) NET_PACKET_TCP((seg)->packet)
#define TCPH_SEQNO(phdr) NetGetDW((U08 *)(phdr) + TCP_SEQUENCE)
#define TCPH_FLAGS(phdr)  (NetGetW((U08 *)(phdr) + TCP_LENGTH_FLAG) & 0x3f)

#define TCP_TCPLEN(seg) ((seg)->u16Length + ((TCPH_FLAGS(TCP_HEADER(seg)) & TCP_FIN || \
          TCPH_FLAGS(TCP_HEADER(seg)) & TCP_SYN)? 1: 0))

// TCP states
typedef enum
{
    E_TCP_STATE_CLOSED          = 0,
    E_TCP_STATE_LISTEN          = 1,
    E_TCP_STATE_SYN_SENT        = 2,
    E_TCP_STATE_SYN_RCVD        = 3,
    E_TCP_STATE_ESTABLISHED     = 4,
    E_TCP_STATE_FIN_WAIT_1      = 5,
    E_TCP_STATE_FIN_WAIT_2      = 6,
    E_TCP_STATE_CLOSE_WAIT      = 7,
    E_TCP_STATE_CLOSING         = 8,
    E_TCP_STATE_LAST_ACK        = 9,
    E_TCP_STATE_TIME_WAIT       = 10
} E_TCP_STATE;

typedef struct
{
    U16 u16SrcPort;             // source port number
    U16 u16DstPort;             // destination port number
    U32 u32Sequence;            // sequence number
    U32 u32Acknowledge;         // acknowledge number
    //U08 u08Length;              // TCP header length
    //U08 u08Flags;
    U16 u16LengthFlag;          // tcp header length & flag
    U16 u16Window;              // number of byte in a window
    U16 u16Checksum;
    U16 u16Urgent;
} ST_TCP_HEADER;

typedef struct
{
    U08 reserved[8];
    U32 u32SrcIp;
    U32 u32DstIp;
    U08 u08Zero;
    U08 u08Protocol;
    U16 u16Length;
} ST_TCP_PSEUDO;

typedef struct
{
    ST_HEADER       Net;
    ST_ETHER_HEADER Ether;
    ST_TCP_PSEUDO   Pseudo;
    ST_TCP_HEADER   Tcp;
    U08             u08Data[MAX_TCP_DATA - ETHERNET_HEADER_SIZE -32];
} ST_TCP_PACKET;

//tcp header filed offset
#define TCP_SRC_PORT            0
#define TCP_DST_PORT            (TCP_SRC_PORT+2)
#define TCP_SEQUENCE            (TCP_DST_PORT+2)
#define TCP_ACKNOWLEDGE         (TCP_SEQUENCE+4)
#define TCP_LENGTH_FLAG         (TCP_ACKNOWLEDGE+4)
#define TCP_WINDOW              (TCP_LENGTH_FLAG+2)
#define TCP_CHECKSUM            (TCP_WINDOW+2)
#define TCP_URGENT              (TCP_CHECKSUM+2)
#define TCP_DATA                (TCP_URGENT+2)

typedef S32 (*TCP_ACCEPT_CALLBACK_PROC)(U16 u16ListenSid, U32 param);

/* This structure represents a TCP segment on the unsent and unacked queues */
typedef struct tcp_seg
{
    struct tcp_seg* next;
    U16 u16Length;
    void* packet;
} ST_TCP_SEG;

typedef struct
{
    U32 packet;
    U08 u08Flag;
    U08 u08HdrLength;
    U16 u16TcpLength;
    U16 u16DataLength;
    U32 u32Sequence;
    U32 u32Acknowledge;
    U16 u16Window;
    
    U32 u32IpAddr;
    U16 u16Port;
    
    U08 u08RcvFlag;                             /* TF_XXX flags */
    U32 u32RcvData;
} ST_INPUT_PACKET;

#define TCP_QUEUE_OOSEQ             1

typedef struct tcp_pcb
{
    E_TCP_STATE eState;
    
    U16 u16SockId;
    U16 u16ListenSockId;
    U16 u16SockOption;
    
    struct tcp_pcb* next;
    
    U32 u32LocalIp;
    U32 u32PeerIp;
    
    U16 u16LocalPort;
    U16 u16PeerPort;
    
    U08 u08Flag;
    U08 u08StatusFlag;
    
    // timer
    U32 u32Timer;
    U08 u08PollTimer;
    U08 u08PollInterval;
    
    S16 u16RetranTime; // retransmission timer  (rtime)
    
    // reveiver variables
    U32 u32RcvNext;
    U16 u16RcvWindow;
    u16_t rcv_ann_wnd; /* receiver window to announce */
    u32_t rcv_ann_right_edge; /* announced right edge of window */
    
    U16 u16Mss; // maximum segment size
    
    U32 u32RTTest;
    U32 u32RTSeq;
    S16 s16Sa;
    S16 s16Sv;
    
    S16 u16RetranTimeout;
    U08 u08RetranNumber;
    
    // fast retransmit/recovery
    U32 u32LastAck; // highest acknowledged seqno.
    U08 u08DupPacks;
    
    // congestion avoidance/control variables
    U16 u16CWindow;
    U16 u16SSThresh;
    
    // sender variables
    U32 u32SendNext; // next new seqno to be sent
    U32 u32SendMax; // highest seqno sent
    U32 u32SendWindow; // sender window
    U32 u32SendWL1; //Sequence and acknowledgement numbers of last window update
    U32 u32SendWL2;
    U32 u32SendLBB; // Sequence number of next byte to be buffered
    
    U16 u16Acked;
    
    U16 u16SendBuffer; // Available buffer space for sending (in bytes)
    U08 u08SendQueueLen; // Available buffer space for sending (in tcp_segs)
    
    ST_TCP_SEG* pstUnsent; // Unsent (queued) segments
    ST_TCP_SEG* pstUnacked; // Sent but unacknowledged segments
#if (TCP_QUEUE_OOSEQ == 1)
    ST_TCP_SEG* pstOOSeq;
    ST_TCP_SEG* pstConcateSeq;
    U16 ooseqLen;
    U16 concateLen;
#endif
    
    U32 u32KeepAlive; // idle time before KEEPALIVE is sent
    U08 u08KeepCounter; // KEEPALIVE counter
    
    //for input packet
    ST_INPUT_PACKET stInput;
    U32 u32PacketQueue;
    
    //callback function
    TCP_ACCEPT_CALLBACK_PROC    accept;
} ST_TCP_PCB;


// tcp pcb status flag
#define TCP_PCB_CONNECTED           0x01
#define TCP_PCB_RESET               0x02
#define TCP_PCB_CLOSED              0x04

// tcp pcb flag
#define TF_ACK_DELAY          0x01        // Delayed ACK
#define TF_ACK_NOW            0x02        // Immediate ACK
#define TF_INFR               0x04        // In fast recovery
#define TF_RESET              0x08        // Connection was reset
#define TF_CLOSED             0x10        // Connection was sucessfully closed
#define TF_GOT_FIN            0x20        // Connection was closed by the remote end
#define TF_NO_DELAY           0x40        // Disable Nagle algorithm

#define TCP_PSEUDO_HEADER_LENGTH    12

void TcpInit();
ST_TCP_PCB* TcpNew();
S32 TcpClose(ST_TCP_PCB* pcb);
void TcpAbort(ST_TCP_PCB* pcb);
S32 TcpBind(ST_TCP_PCB* pcb, U32 ipAddr, U16 port);
S32 TcpBind2(ST_TCP_PCB* pcb, U32 ipAddr, U16 port);
S32 TcpConnect(ST_TCP_PCB* pcb, U32 ipAddr, U16 port);
S32 TcpConnect2(ST_TCP_PCB* pcb, U32 ipAddr, U16 port);
S32 TcpDisconnect(ST_TCP_PCB* pcb);
S32 TcpWrite(ST_TCP_PCB* pcb, U08* data, S32 size);
void TcpReceived(ST_TCP_PCB* pcb, U16 length, int s);
void TcpFree(ST_TCP_PCB* pcb);
S32 TcpListen(ST_TCP_PCB* pcb);
void TcpAcceptCallbackSet(ST_TCP_PCB* pcb, TCP_ACCEPT_CALLBACK_PROC acceptProc);
S32 TcpAccept(ST_TCP_PCB* pcb, U16 u16SockId);

void TcpConfigWindowSizeSet(U16 size);
void TcpConfigSendBufferSizeSet(U16 size);
void TcpConfigSendQueueLenSet(U08 size);
static void tcpSendReset(ST_TCP_PCB* pcb, ST_NET_PACKET *packet, U32 srcAddr, U32 destAddr);
#endif
