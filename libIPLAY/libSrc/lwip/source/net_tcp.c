#define LOCAL_DEBUG_ENABLE 0

#include <string.h>
#include <sys/time.h>
#include "global612.h"
#include "typedef.h"
#include "linux/types.h"
#include "ndebug.h"
#include "lwip_config.h"
#include "net_packet.h"
#include "socket.h"
#include "net_socket.h"
#include "net_netctrl.h"
#include "error.h"
#include "util_queue.h"
#include "net_nic.h"
//#include "os_defs.h"
#include "lwip_opt.h"
#include "lwip_debug.h"

#if TCP_SNDLOWAT >= TCP_SND_BUF
  #error "lwip_sanity_check: WARNING: TCP_SNDLOWAT must be less than TCP_SND_BUF. If you know what you are doing, define LWIP_DISABLE_TCP_SANITY_CHECKS to 1 to disable this error."
#endif

U32 u32SeqNumberBase = 0x0f800000; //0x012DAE8E;
U32 u32TcpTicks = 0;
U32 u32TcpTimer = 0;
int slowcnt2;
int slowcnt1;
U32 u32TcpOff;

const U08 TCP_BACKOFF[13] = { 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7};
const U08 TCP_BACKOFF2[13] = { 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7};

U08 u08TcpTimerId = 0;
static U08 u08TcpEventId;
static BOOL boolTcpFastTimer;
static BOOL boolTcpSlowTimer;

static U16 u16TcpWindowSize = TCP_WINDOW_SIZE;
static U16 u16TcpSendBufferSize = TCP_SEND_BUFFER;
static U08 u08TcpSendQueueLen = TCP_SEND_QUEUE_LEN;

#define TCP_SEMAPHORE
#ifdef TCP_SEMAPHORE
    U08 u08TcpSemaphoreId = 0;
    
    #define TCP_ENTER_CRITICAL          mpx_SemaphoreWait(u08TcpSemaphoreId)
    #define TCP_EXIT_CRITICAL           mpx_SemaphoreRelease(u08TcpSemaphoreId)
#else
    #define TCP_ENTER_CRITICAL
    #define TCP_EXIT_CRITICAL
#endif


static ST_TCP_PCB* tcpActivePcb = 0;
static ST_TCP_PCB* tcpTimeWaitPcb = 0;
static ST_TCP_PCB* tcpBoundPcb;
static ST_TCP_PCB* tcpListenPcb = 0;

static void tcpTimerProc();
static void tcpFastTimer(); //250ms
static void tcpSlowTimer(); //500ms

static U32 tcpNextISS();
static S32 tcpEnqueue(ST_TCP_PCB* pcb, U08 u08Flag, U08* pu08Arg, U16 u16ArgLen, U08* pu08OptData, U08 u08OptLen);
static ST_TCP_SEG* tcpNewSegment();
static void tcpFreeSegment(ST_TCP_SEG* seg);
static void tcpSegmentsFree(ST_TCP_SEG* seg);
static S32 tcpOutput(ST_TCP_PCB* pcb);
static void tcpOutputSegment(ST_TCP_PCB* pcb, ST_TCP_SEG* segment);
static void tcpPcbRegister(ST_TCP_PCB** pcbList, ST_TCP_PCB* pcb);
static void tcpPcbRemove(ST_TCP_PCB** pcbList, ST_TCP_PCB* pcb);
static void tcpPcbPurge(ST_TCP_PCB* pcb);
static void tcpRemove(ST_TCP_PCB** pcbList, ST_TCP_PCB* pcb);
static S32 tcpProcess(ST_TCP_PCB* pcb);
static void tcpParseOption(ST_TCP_PCB* pcb, U08* optData, U08 optLength);
static void tcpAck(ST_TCP_PCB* pcb);
static U16 tcpPseudoChecksum(ST_NET_PACKET* packet, U32 srcIp, U32 dstIp);
static void tcpAck(ST_TCP_PCB* pcb);
static void tcpAckNow(ST_TCP_PCB* pcb);
static void tcpReset(U32 seqno, U32 ackno, U32 srcIp, U32 dstIp, U16 srcPort, U16 dstPort);
static BOOL tcpWaitConnected(ST_TCP_PCB* pcb);
static BOOL tcpWaitClosed(ST_TCP_PCB* pcb);
static BOOL tcpPacketReceive(ST_TCP_PCB* pcb);
static void tcpRexmit(ST_TCP_PCB* pcb);
static S32 tcpSendControl(ST_TCP_PCB* pcb, U08 flag);
static void tcpPacketDeflate(ST_NET_PACKET* packet, U16 size);
static void tcpPacketDeflate2(ST_NET_PACKET* packet, U16 size);
static S32 tcpTimeWaitInput(ST_TCP_PCB* pcb);
static S32 tcpListenInput(ST_TCP_PCB* pcb, U32 destAddr);
void tcpRexmitRto(ST_TCP_PCB* pcb);
static void tcpSelfAbort(ST_TCP_PCB* pcb, int reset);
static S32 tcpPacketConcatenate(ST_NET_PACKET** headPacket, ST_NET_PACKET* tailPacket);
u32_t            tcp_update_rcv_ann_wnd(struct tcp_pcb *pcb);
static u8_t *tcp_output_set_header(struct tcp_pcb *pcb, ST_NET_PACKET *pkt, int optlen, u32_t seqno_be);

#if TCP_QUEUE_OOSEQ
static ST_TCP_SEG* tcpSegCopy(ST_NET_PACKET** packet);
#endif

void TcpInit()
{
    S32 status;
    
    tcpActivePcb = 0;
    tcpTimeWaitPcb = 0;
    tcpBoundPcb = 0;
    tcpListenPcb = 0;
    
    u32TcpTicks = 0;
    u32TcpTimer = 0;

    status = NetTimerInstall(tcpTimerProc, NET_TIMER_250_MS);
    if(status >= 0)
    {
        u08TcpTimerId = (U08)status;
        NetTimerRun(u08TcpTimerId);
    }
    else
    {
        DPrintf("[TCP] tcp timer create fail");
        BREAK_POINT();
    }

#ifdef TCP_SEMAPHORE
    status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
    if(status >= 0)
    {
        u08TcpSemaphoreId = (U08)status;
        DPrintf("[TCP] semaphore id = %d", u08TcpSemaphoreId);
    }
    else
    {
        DPrintf("[TCP] semaphore create fail");
        BREAK_POINT();
    }
#endif

    status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_EVENT_CLEAR, 0);
    if(status < 0)
    {
        DPrintf("[TCP] event create fail mpx_EventCreate returns %d", status);
        BREAK_POINT();
    }
    u08TcpEventId = (U08)status;


    return;
}

ST_TCP_PCB* TcpNew()
{
    ST_TCP_PCB* pcb = mpx_Malloc(sizeof(ST_TCP_PCB));
    
    if(pcb)
    {
        U32 iss = tcpNextISS();
        
        memset(pcb, 0, sizeof(ST_TCP_PCB));
        pcb->u16SendBuffer = u16TcpSendBufferSize;
        pcb->u08SendQueueLen = 0;
        pcb->u16RcvWindow = u16TcpWindowSize;
        pcb->rcv_ann_wnd = u16TcpWindowSize;
        pcb->u16Mss = (TCP_MSS > 536) ? 536 : TCP_MSS; /* TODO */
        pcb->u16RetranTimeout = 3000 / TCP_SLOW_INTERVAL;
        pcb->u16RetranTime = -1;
        pcb->u16CWindow = 1;
        pcb->u32SendWL2 = iss;
        pcb->u32SendNext = iss;
        pcb->u32LastAck = iss;
        pcb->u32SendLBB = iss;
        pcb->u32Timer = u32TcpTicks;
        pcb->u08PollTimer = 0;
        pcb->u32KeepAlive = TCP_KEEPDEFAULT;
        pcb->u08KeepCounter = 0;
        pcb->u08DupPacks = 0;
        pcb->u08RetranNumber = 0;
        pcb->s16Sa = 0;
        pcb->s16Sv = 3000 / TCP_SLOW_INTERVAL;
        pcb->u08PollInterval = 4;
    }
    
    return pcb;
}

void TcpFree(ST_TCP_PCB* pcb)
{
    //TCP_ENTER_CRITICAL;
    
    if(pcb)
    {
#ifdef NEW_NETSOCKET
        sock_Detach(pcb->u16SockId);
#endif
        mpx_Free(pcb);
    }
    
    //TCP_EXIT_CRITICAL;
}

/* tcp lock should be held before entering this function */
S32 _TcpClose(ST_TCP_PCB* pcb)
{
    S32 err = NO_ERR;

    MP_DEBUG("[TCP] _TcpClose st=%d", pcb->eState);
    switch(pcb->eState)
    {
        case E_TCP_STATE_CLOSED:
            tcpPcbRemove(&tcpBoundPcb, pcb);
            TcpFree(pcb);
            pcb = 0;
        break;
        
        case E_TCP_STATE_LISTEN:
            //DPrintf("[TCP] _TcpClose unimplemented state");
            tcpPcbRemove(&tcpListenPcb, pcb);
            TcpFree(pcb);
            pcb = 0;
        break;
        
        case E_TCP_STATE_SYN_SENT:
            tcpPcbRemove(&tcpActivePcb, pcb);
            TcpFree(pcb);
            pcb = 0;
        break;
        
        case E_TCP_STATE_SYN_RCVD:
            err = tcpSendControl(pcb, TCP_FIN);
            if(err == NO_ERR)
            {
                pcb->eState = E_TCP_STATE_FIN_WAIT_1;
            }
        break;
        
        case E_TCP_STATE_ESTABLISHED:
            //DPrintf("[TCP] E_TCP_STATE_ESTABLISHED, send FIN (%d)", pcb->u16SockId);
            //DPrintf("[TCP] state E_TCP_STATE_ESTABLISHED changed to E_TCP_STATE_FIN_WAIT_1");
            err = tcpSendControl(pcb, TCP_FIN);
            if(err == NO_ERR)
            {
                pcb->eState = E_TCP_STATE_FIN_WAIT_1;
            }
        break;
        
        case E_TCP_STATE_CLOSE_WAIT:
            //DPrintf("[TCP] E_TCP_STATE_CLOSE_WAIT, send FIN (%d)", pcb->u16SockId);
            //DPrintf("[TCP] state E_TCP_STATE_CLOSE_WAIT changed to E_TCP_STATE_LAST_ACK");
            err = tcpSendControl(pcb, TCP_FIN);
            if(err == NO_ERR)
            {
                pcb->eState = E_TCP_STATE_LAST_ACK;
            }
        break;
        
        default:
            /* Has already been closed, do nothing. */
            err = NO_ERR;
            pcb = 0;
        break;
    }
    
    if((pcb != 0) && (err == NO_ERR))
    {
//        pcb->u16SockId = 0;
        tcpOutput(pcb);
        MP_ASSERT(pcb->eState != E_TCP_STATE_ESTABLISHED);        /* E_TCP_STATE_ESTABLISHED */
    }
    
    return err;
}

S32 TcpClose(ST_TCP_PCB* pcb)
{
    S32 err = NO_ERR;

    TCP_ENTER_CRITICAL;
    
    MP_DEBUG("[TCP] TcpClose st=%d", pcb->eState);
    switch(pcb->eState)
    {
        case E_TCP_STATE_CLOSED:
            tcpPcbRemove(&tcpBoundPcb, pcb);
            TcpFree(pcb);
            pcb = 0;
        break;
        
        case E_TCP_STATE_LISTEN:
            //DPrintf("[TCP] TcpClose unimplemented state");
            tcpPcbRemove(&tcpListenPcb, pcb);
            TcpFree(pcb);
            pcb = 0;
        break;
        
        case E_TCP_STATE_SYN_SENT:
            tcpPcbRemove(&tcpActivePcb, pcb);
            TcpFree(pcb);
            pcb = 0;
        break;
        
        case E_TCP_STATE_SYN_RCVD:
            err = tcpSendControl(pcb, TCP_FIN);
            if(err == NO_ERR)
            {
                pcb->eState = E_TCP_STATE_FIN_WAIT_1;
            }
        break;
        
        case E_TCP_STATE_ESTABLISHED:
            //DPrintf("[TCP] E_TCP_STATE_ESTABLISHED, send FIN (%d)", pcb->u16SockId);
            //DPrintf("[TCP] state E_TCP_STATE_ESTABLISHED changed to E_TCP_STATE_FIN_WAIT_1");
            err = tcpSendControl(pcb, TCP_FIN);
            if(err == NO_ERR)
            {
                pcb->eState = E_TCP_STATE_FIN_WAIT_1;
            }
        break;
        
        case E_TCP_STATE_CLOSE_WAIT:
            //DPrintf("[TCP] E_TCP_STATE_CLOSE_WAIT, send FIN (%d)", pcb->u16SockId);
            //DPrintf("[TCP] state E_TCP_STATE_CLOSE_WAIT changed to E_TCP_STATE_LAST_ACK");
            err = tcpSendControl(pcb, TCP_FIN);
            if(err == NO_ERR)
            {
                pcb->eState = E_TCP_STATE_LAST_ACK;
            }
        break;
        
        default:
            /* Has already been closed, do nothing. */
            err = NO_ERR;
            pcb = 0;
        break;
    }
    
    if((pcb != 0) && (err == NO_ERR))
    {
//        pcb->u16SockId = 0;
        tcpOutput(pcb);
        MP_ASSERT(pcb->eState != E_TCP_STATE_ESTABLISHED);        /* E_TCP_STATE_ESTABLISHED */
    }

    TCP_EXIT_CRITICAL;
    
    return err;
}

S32 TcpDisconnect(ST_TCP_PCB* pcb)
{
    S32 err = NO_ERR;
    
    switch(pcb->eState)
    {
        case E_TCP_STATE_SYN_RCVD:
        case E_TCP_STATE_ESTABLISHED:
        case E_TCP_STATE_CLOSE_WAIT:
            err = TcpClose(pcb);
            if(err == NO_ERR)
            {
                mpx_EventSet(u08TcpEventId, 1UL << pcb->u16SockId);
            }
        break;
        case E_TCP_STATE_CLOSED:
        case E_TCP_STATE_LISTEN:
        case E_TCP_STATE_SYN_SENT:
            err = TcpClose(pcb);
        break;
        default:
            err = NO_ERR;
        break;
    }
    
    return err;
}
S32 TcpBind(ST_TCP_PCB* pcb, U32 ipAddr, U16 port)
{
    if(!ipAddr || !port)
        return -1;
    
    TCP_ENTER_CRITICAL;
    
    pcb->u32LocalIp = ipAddr;
    pcb->u16LocalPort = port;
    
    TCP_EXIT_CRITICAL;
    
    return 0;
}

#ifdef NEW_NETSOCKET
/* 
 * Local IP address can be INADDR_ANY
 */
S32 TcpBind2(ST_TCP_PCB* pcb, U32 ipAddr, U16 port)
{
    ST_TCP_PCB *cpcb = 0, *prevPcb = 0;
    if(!port)
        return -1;
    
    LWIP_ERROR("tcp_bind: can only bind in state CLOSED", pcb->eState == E_TCP_STATE_CLOSED, return ERR_ISCONN);

    TCP_ENTER_CRITICAL;
    
    /* Check if the address already is in use. */
    /* Check the listen pcbs. */
    for(cpcb = tcpListenPcb; cpcb; cpcb = cpcb->next)
    {
        if (cpcb->u16LocalPort == port) {
            if (cpcb->u32LocalIp==INADDR_ANY ||
                (ipAddr==INADDR_ANY) ||
                (cpcb->u32LocalIp == ipAddr)) {
                TCP_EXIT_CRITICAL;
                return ERR_USE;
            }
        }
    }
    /* Check the connected pcbs. */
    for(cpcb = tcpActivePcb; cpcb; cpcb = (ST_TCP_PCB*)cpcb->next)
    {
        if (cpcb->u16LocalPort == port) {
            if (cpcb->u32LocalIp==INADDR_ANY ||
                (ipAddr==INADDR_ANY) ||
                (cpcb->u32LocalIp == ipAddr)) {
                TCP_EXIT_CRITICAL;
                return ERR_USE;
            }
        }
    }
    /* Check the bound, not yet connected pcbs. */
    for(cpcb = tcpBoundPcb; cpcb; cpcb = (ST_TCP_PCB*)cpcb->next)
    {
        if (cpcb->u16LocalPort == port) {
            if (cpcb->u32LocalIp==INADDR_ANY ||
                (ipAddr==INADDR_ANY) ||
                (cpcb->u32LocalIp == ipAddr)) {
                TCP_EXIT_CRITICAL;
                return ERR_USE;
            }
        }
    }
    /* @todo: until SO_REUSEADDR is implemented (see task #6995 on savannah),
     * we have to check the pcbs in TIME-WAIT state, also: */
    for(cpcb = tcpTimeWaitPcb; cpcb; cpcb = (ST_TCP_PCB*)cpcb->next)
    {
        if (cpcb->u16LocalPort == port) {
            if ((cpcb->u32LocalIp == ipAddr)) {
                TCP_EXIT_CRITICAL;
                return ERR_USE;
            }
        }
    }

    pcb->u32LocalIp = ipAddr;
    pcb->u16LocalPort = port;
    
    tcpPcbRegister(&tcpBoundPcb, pcb);

    TCP_EXIT_CRITICAL;
    LWIP_DEBUGF(TCP_DEBUG, ("tcp_bind: bind to port %d\n", port));
    
    return 0;
}
#endif

S32 TcpConnect(ST_TCP_PCB* pcb, U32 ipAddr, U16 port)
{
    S32 status = NO_ERR;
    U32 iss;
    U32 optData[2];
    
    /*
    DPrintf("[TCP] connecting to \\-");
    NetDebugPrintIP(ipAddr);
    DPrintf("   port: %d", port);
    */
    
    if(!ipAddr || !port)
        return -1;
    
    TCP_ENTER_CRITICAL;
    
    pcb->u16PeerPort = port;
    pcb->u32PeerIp = ipAddr;
    
    iss = tcpNextISS();
    pcb->u32RcvNext = 0;
    pcb->u32SendNext = iss;
    pcb->u32LastAck = iss - 1;
    pcb->u32SendLBB = iss - 1;
    pcb->u16RcvWindow = u16TcpWindowSize;
    pcb->rcv_ann_wnd = u16TcpWindowSize;
    pcb->u32SendWindow = u16TcpWindowSize;
    pcb->u16Mss = (TCP_MSS > 536) ? 536 : TCP_MSS;
    pcb->u16CWindow = 1;
    pcb->u16SSThresh = pcb->u16Mss * 10;
    pcb->eState = E_TCP_STATE_SYN_SENT;
    
    tcpRemove(&tcpBoundPcb, pcb);
    tcpPcbRegister(&tcpActivePcb, pcb);
    
    //MSS option
    optData[0] = TCP_BUILD_MSS_OPTION();
    optData[1] = 0x01010101;                    /* TODO */
    
    status = tcpEnqueue(pcb, TCP_SYN, 0, 0, (U08*)optData, 8);
    if(status == NO_ERR)
    {
        status = tcpOutput(pcb);
        TCP_EXIT_CRITICAL;
        
        if(tcpWaitConnected(pcb))
            status = NO_ERR;
        else
        {
            status = -1;
            //maybe we have to send reset here??
        }
    }
    else
        TCP_EXIT_CRITICAL;
    
    return status;
}

#ifdef NEW_NETSOCKET
S32 TcpConnect2(ST_TCP_PCB* pcb, U32 ipAddr, U16 port)
{
    S32 status = NO_ERR;
    U32 iss;
    U32 optData[2];
    
    /*
    DPrintf("[TCP] connecting to \\-");
    NetDebugPrintIP(ipAddr);
    DPrintf("   port: %d", port);
    */
    
    if(!ipAddr || !port)
        return -1;
    
    TCP_ENTER_CRITICAL;
    
    pcb->u16PeerPort = port;
    pcb->u32PeerIp = ipAddr;
    
    iss = tcpNextISS();
    pcb->u32RcvNext = 0;
    pcb->u32SendNext = iss;
    pcb->u32LastAck = iss - 1;
    pcb->u32SendLBB = iss - 1;
    pcb->u16RcvWindow = u16TcpWindowSize;
    pcb->rcv_ann_wnd = u16TcpWindowSize;
    pcb->rcv_ann_right_edge = pcb->u32RcvNext;
    pcb->u32SendWindow = u16TcpWindowSize;
    pcb->u16Mss = (TCP_MSS > 536) ? 536 : TCP_MSS;
    pcb->u16CWindow = 1;
    pcb->u16SSThresh = pcb->u16Mss * 10;
    pcb->eState = E_TCP_STATE_SYN_SENT;
    
    tcpRemove(&tcpBoundPcb, pcb);
    tcpPcbRegister(&tcpActivePcb, pcb);
    
    //MSS option
    optData[0] = TCP_BUILD_MSS_OPTION();
    optData[1] = 0x01010101;                    /* TODO */
    
    status = tcpEnqueue(pcb, TCP_SYN, 0, 0, (U08*)optData, 8);
    if(status == NO_ERR)
    {
#if TRACETCP > 0
        ntrace_printf(0, "[TCP] xmit SYN,%u,%u,%u", pcb->u16SockId, pcb->u16LocalPort, pcb->u16PeerPort);
#endif
        status = tcpOutput(pcb);
        TCP_EXIT_CRITICAL;
        
#if 0
        /* ----------  no waiting here  ---------- */
        if(tcpWaitConnected(pcb))
            status = NO_ERR;
        else
        {
            status = -1;
            //maybe we have to send reset here??
        }
#endif
    }
    else
        TCP_EXIT_CRITICAL;
    
    return status;
}
#endif

static void tcpPcbRegister(ST_TCP_PCB** pcbList, ST_TCP_PCB* pcb)
{
    if(!pcb)
        return;
    
    if(*pcbList == 0)
        *pcbList = pcb;
    else
    {
        pcb->next = (*pcbList);
        *pcbList = pcb;
    }
}

static void tcpRemove(ST_TCP_PCB** pcbList, ST_TCP_PCB* pcb)
{
    ST_TCP_PCB* tempPcb;
    
    if(*pcbList == pcb)
        *pcbList = (*pcbList)->next;
    else
    {
        for(tempPcb = *pcbList; tempPcb != 0; tempPcb = tempPcb->next)
        {
            if((tempPcb->next) && (tempPcb->next == pcb))
            {
                tempPcb->next = pcb->next;
                break;
            }
        }
    }
    pcb->next = 0;
}

static void tcpPcbRemove(ST_TCP_PCB** pcbList, ST_TCP_PCB* pcb)
{
    LWIP_DEBUGF(TCP_DEBUG, ("tcpPcbRemove: st=%d.\n", pcb->eState));
    tcpRemove(pcbList, pcb);
    tcpPcbPurge(pcb);
    
    if((pcb->eState != E_TCP_STATE_TIME_WAIT) && (pcb->eState != E_TCP_STATE_LISTEN)
        && (pcb->u08Flag & TF_ACK_DELAY))
    {
        pcb->u08Flag |= TF_ACK_NOW;
        tcpOutput(pcb);
    }
    
    if (pcb->eState != E_TCP_STATE_LISTEN) {
        LWIP_ASSERT("unsent segments leaking", pcb->pstUnsent == NULL);
        LWIP_ASSERT("unacked segments leaking", pcb->pstUnacked == NULL);
#if TCP_QUEUE_OOSEQ
        LWIP_ASSERT("ooseq segments leaking", pcb->pstOOSeq == NULL);
#endif /* TCP_QUEUE_OOSEQ */
    }

    pcb->eState = E_TCP_STATE_CLOSED;
}

static void tcpPcbPurge(ST_TCP_PCB* pcb)
{
    if((pcb->eState != E_TCP_STATE_CLOSED) && (pcb->eState != E_TCP_STATE_TIME_WAIT) &&
        (pcb->eState != E_TCP_STATE_LISTEN))
    {
        /* Stop the retransmission timer as it will expect data on unacked
           queue if it fires */
        pcb->u16RetranTime = -1;

#if TCP_QUEUE_OOSEQ
        tcpSegmentsFree(pcb->pstOOSeq);
        pcb->pstOOSeq = NULL;
#endif

        tcpSegmentsFree(pcb->pstUnsent);
        tcpSegmentsFree(pcb->pstUnacked);
        pcb->pstUnsent = pcb->pstUnacked = NULL;
    }
}

U32 tcpNextISS()
{
    return (u32SeqNumberBase += u32TcpTicks);
}

static ST_TCP_SEG* tcpNewSegment()
{
    ST_TCP_SEG* seg = mpx_Malloc(sizeof(ST_TCP_SEG));
    if(seg)
    {
        seg->next = 0;
        seg->u16Length = 0;
        seg->packet = 0;
        
        seg->packet = NetNewPacket(TRUE);
        if(!seg->packet)
        {
            DPrintf("[TCP] allocate packet fail");
            
            mpx_Free(seg);
            seg = 0;
        }
    }
    
    return seg;
}

static void tcpFreeSegment(ST_TCP_SEG* seg)
{
    if(seg)
    {
        if(seg->packet)
            NetFreePacket(seg->packet);
        mpx_Free(seg);
    }
}

static void tcpSegmentsFree(ST_TCP_SEG* seg)
{
    ST_TCP_SEG* next;
    U08 count = 0;
    
    while(seg)
    {
        next = seg->next;
        tcpFreeSegment(seg);
        count++;
        seg = next;
    }
}

static S32 tcpEnqueue(ST_TCP_PCB* pcb, U08 u08Flag, U08* pu08Arg, U16 u16ArgLen, U08* pu08OptData, U08 u08OptLen)
{
    U32 left = 0, seqno;
    U08* pArgData = pu08Arg;
    U16 u16SegLength = 0;
    S32 error = NO_ERR;
    U08 queueLen;
    ST_TCP_SEG *segQueue = 0, *segment = 0, *segPtr = 0;
    
    if(u16ArgLen > pcb->u16SendBuffer)
    {
        error = -1;
        DPrintf("[TCP] data length overflow");
        goto ERROR;
    }
    
    left = u16ArgLen;
    seqno = pcb->u32SendLBB;
    
    queueLen = pcb->u08SendQueueLen;
    if(queueLen >= u08TcpSendQueueLen)
    {
        DPrintf("[TCP] queue full, queueLen = %d", queueLen);
        DPrintf("[TCP] send queue len = %d", u08TcpSendQueueLen);
        error = -1;
        goto ERROR;
    }
    
    segQueue = segment = segPtr = 0;
    u16SegLength = 0;
    
    while((!segQueue) || (left > 0))
    {
        U16 lenFlag = u08Flag;
        ST_NET_PACKET* tcpPacket;
        U08 *tcpData, *tcpHdr;
        
        u16SegLength = left > pcb->u16Mss ? pcb->u16Mss : left;
        
        segment = tcpNewSegment();
        if(!segment)
        {
            DPrintf("[TCP] tcp allocate segment buffer fail");
            error = -1;
            goto ERROR;
        }
        
        if(!segQueue)
            segQueue = segment; //this segment is the head
        else
            segPtr->next = segment;
            
        segPtr = segment;
        
        tcpPacket = (ST_NET_PACKET*)segment->packet;
        tcpHdr = NET_PACKET_TCP(tcpPacket);
        tcpData = NET_PACKET_TCP_DATA(tcpPacket);
        tcpPacket->Net.u16PayloadSize = 0;
        
        if(pu08OptData)
        {
            lenFlag |= (((5 + (u08OptLen >> 2)) & 0xf) << 12);
			
				mmcp_memcpy(tcpData, pu08OptData, u08OptLen);
            tcpPacket->Net.u16PayloadSize = u08OptLen;
            
            queueLen++;
        }
        else
        {
            lenFlag |= (5 << 12);
            
            if(pu08Arg)
            {
            
					mmcp_memcpy(tcpData, pArgData, u16SegLength);
            }
            tcpPacket->Net.u16PayloadSize = u16SegLength;
            
            queueLen++;
        }
        
        segment->u16Length = u16SegLength;
        tcpPacket->Net.u16PayloadSize += sizeof(ST_TCP_HEADER);
        
        //build tcp header
        NetPutW(&tcpHdr[TCP_SRC_PORT], pcb->u16LocalPort);
        NetPutW(&tcpHdr[TCP_DST_PORT], pcb->u16PeerPort);
        NetPutDW(&tcpHdr[TCP_SEQUENCE], seqno);
        //leave the 3 fields until later
        //tcpHdr[TCP_ACKNOWLEDGE]
        //tcpHdr[TCP_WINDOW]
        //tcpHdr[TCP_CHECKSUM]
        NetPutW(&tcpHdr[TCP_LENGTH_FLAG], lenFlag);
        NetPutW(&tcpHdr[TCP_URGENT], 0);
        
        left -= u16SegLength;
        seqno += u16SegLength;
        pArgData += u16SegLength;
    }
    
    if(!pcb->pstUnsent)
    {
        segPtr = 0;
    }
    else
    {
        for(segPtr = pcb->pstUnsent; segPtr->next != 0; segPtr = segPtr->next); //find the tail of the queue
    }
    
    {
        if(segPtr)
            segPtr->next = segQueue;
        else
            pcb->pstUnsent = segQueue;
        
        if((u08Flag & TCP_SYN) || (u08Flag & TCP_FIN))
            u16ArgLen++;
        
        pcb->u32SendLBB += u16ArgLen;
        pcb->u16SendBuffer -= u16ArgLen;
        if(pcb->u16SendBuffer == 0)
        {
            mpx_EventClear(u08TcpEventId, ~(0x00000001 << pcb->u16SockId));
        }
        
        pcb->u08SendQueueLen = queueLen;
        
        if(segment && (u16SegLength > 0))
        {
            U08* tcpHdr = NET_PACKET_TCP(segment->packet);
            U16 flagLen = NetGetW(&tcpHdr[TCP_LENGTH_FLAG]);
            
            flagLen |= TCP_PUSH;
            NetPutW(&tcpHdr[TCP_LENGTH_FLAG], flagLen);
        }
    }

ERROR:
    if(error)
    {
#if 0
        DPrintf("[TCP] segment queue release");
        for(segPtr = segQueue; segPtr != 0; segPtr = segPtr->next)
            tcpFreeSegment(segPtr);
#endif
    }
    
    return error;
}

static S32 tcpOutput(ST_TCP_PCB* pcb)
{
    U32 u32Window;
    ST_TCP_SEG *segment, *segPtr;
    U08* tcp = 0;
    U32 seqno = 0;
    U16 lenFlag = 0;
    u32_t snd_nxt;
    
    u32Window = pcb->u32SendWindow > pcb->u16CWindow ? pcb->u16CWindow : pcb->u32SendWindow;
    segment = pcb->pstUnsent;
    
    segPtr = pcb->pstUnacked;
    if(segPtr)
    {
        for(; segPtr->next != 0; segPtr = segPtr->next);
    }
    
    if(pcb->u08Flag & TF_ACK_NOW)
    {
        if(segment)
        {
            MP_DEBUG("[TCP] ACK_NOW unsent packet");
            
            tcp = NET_PACKET_TCP(segment->packet);
            seqno = NetGetDW(&tcp[TCP_SEQUENCE]);
        }
        
        if((segment == 0) || (segment && ((seqno - pcb->u32LastAck + segment->u16Length) > u32Window)))
        {
            //create an empty ACK segment and send it
            ST_NET_PACKET* ackPacket = NetNewPacket(FALSE);
            U16 checksum;
            
            if(!ackPacket)
            {
                DPrintf("[TCP] allocate packet fail");
                return -1;
            }
            
            pcb->u08Flag &= ~(TF_ACK_NOW | TF_ACK_DELAY);
            
            tcp = tcp_output_set_header(pcb, ackPacket, 0, pcb->u32SendNext);
            
//            MP_DEBUG2("tcpOutput: ACK_NOW seq=0x%x ack=%x", pcb->u32SendNext,pcb->u32RcvNext);
            //DPrintf("ack %x", pcb->u32RcvNext);
            
            ackPacket->Net.u16PayloadSize = sizeof(ST_TCP_HEADER);
            
            checksum = tcpPseudoChecksum(ackPacket, pcb->u32LocalIp, pcb->u32PeerIp);
            NetPutW(&tcp[TCP_CHECKSUM], checksum);
            
//            MP_DEBUG2("[TCP] ACK NOW, seq = %x, ack = %x", NetGetDW(&tcp[TCP_SEQUENCE]), NetGetDW(&tcp[TCP_ACKNOWLEDGE]));
            //DPrintf("%5x", NetGetDW(&tcp[TCP_SEQUENCE]));
            //DPrintf("o seq = %x, ack = %x", NetGetDW(&tcp[TCP_SEQUENCE]), NetGetDW(&tcp[TCP_ACKNOWLEDGE]));
            IpPacketSend(ackPacket, IP_PROTOCOL_TCP, pcb->u32LocalIp, pcb->u32PeerIp);
#ifndef DRIVER_FREE_TXBUFFER
            NetFreePacket(ackPacket);
#endif
            
            return NO_ERR;
        }
    }
    
    int cnt = 0;
    while(segment != 0)
    {
        ST_NET_PACKET* packet = (ST_NET_PACKET*)segment->packet;
        U16 dataLength;
        U08 headerLength;
        
        tcp = NET_PACKET_TCP(packet);
        lenFlag = NetGetW(&tcp[TCP_LENGTH_FLAG]);
        seqno = NetGetDW(&tcp[TCP_SEQUENCE]);
        headerLength = ((NetGetW(&tcp[TCP_LENGTH_FLAG]) >> 12) & 0xf) << 2;
        dataLength = packet->Net.u16PayloadSize - headerLength;
        if((lenFlag & TCP_SYN) || (lenFlag & TCP_FIN))
            dataLength++;
        
        if((seqno - pcb->u32LastAck + segment->u16Length) > u32Window)
            break;
            
        //MP_DEBUG3("tcpOutput: while seq=0x%x len=%d,cnt=%d", seqno, segment->u16Length, cnt++);

        pcb->pstUnsent = segment->next;
        
        if(pcb->eState != E_TCP_STATE_SYN_SENT)
        {
            lenFlag |= TCP_ACK;
            NetPutW(&tcp[TCP_LENGTH_FLAG], lenFlag);
            
            pcb->u08Flag &= ~(TF_ACK_DELAY|TF_ACK_NOW);
        }

        tcpOutputSegment(pcb, segment);
        snd_nxt = NET_TCP_SEQNO(segment) + dataLength;
        if (TCP_SEQ_LT(pcb->u32SendNext, snd_nxt)) {
            pcb->u32SendNext = snd_nxt;
        }
        
            
        /* put segment on unacknowledged list if length > 0 */
        if(dataLength > 0)
        {
            segment->next = 0;
            if(pcb->pstUnacked == 0)
            {
                pcb->pstUnacked = segment;
                segPtr = segment;
            }
            else
            {
                ST_NET_PACKET* unackPacket = (ST_NET_PACKET*)segPtr->packet;
                U08* unackTcp = NET_PACKET_TCP(unackPacket);
                U32 unackSeqno = NetGetDW(&unackTcp[TCP_SEQUENCE]);
                
                if (TCP_SEQ_LT(seqno, unackSeqno))
                {
                    struct tcp_seg **cur_seg = &(pcb->pstUnacked);
                    while (*cur_seg &&
                            TCP_SEQ_LT(NET_TCP_SEQNO(*cur_seg), NET_TCP_SEQNO(segment))) {
                        cur_seg = &((*cur_seg)->next );
                    }
                    segment->next = (*cur_seg);
                    (*cur_seg) = segment;
                }
                else
                {
                    segPtr->next = segment;
                    segPtr = segment;
                }
            }
        }
        else
        {
            tcpFreeSegment(segment);
        }

        segment = pcb->pstUnsent;
    }
    
    return NO_ERR;
}

static void tcpOutputSegment(ST_TCP_PCB* pcb, ST_TCP_SEG* segment)
{
#ifdef DRIVER_FREE_TXBUFFER
    /*
     * Use a duplicate packet since TCP needs to do re-transmission.
     */
    ST_NET_PACKET* packet = NetNewPacket(FALSE);
    if (!packet)
        return;
    packet->Net.u16PayloadSize = ((ST_NET_PACKET*)segment->packet)->Net.u16PayloadSize;
	
    mmcp_memcpy(NET_PACKET_TCP(packet), NET_PACKET_TCP(segment->packet), 
            ((ST_NET_PACKET*)segment->packet)->Net.u16PayloadSize);
#else
    ST_NET_PACKET* packet = (ST_NET_PACKET*)segment->packet;
#endif
    U08* tcp = NET_PACKET_TCP(packet);
    U16 checksum;
    U16 payloadSize = 0;
    
    NetPutDW(&tcp[TCP_ACKNOWLEDGE], pcb->u32RcvNext);
    
    /* advertise our receive window size in this TCP segment */
    NetPutW(&tcp[TCP_WINDOW], pcb->rcv_ann_wnd);
        
    pcb->rcv_ann_right_edge = pcb->u32RcvNext + pcb->rcv_ann_wnd;
        
    /* Set retransmission timer running if it is not currently enabled */
    if(pcb->u16RetranTime == -1)
    pcb->u16RetranTime = 0;
    
    if(pcb->u32RTTest == 0)
    {
        pcb->u32RTTest = u32TcpTicks;
        pcb->u32RTSeq = NetGetDW(&tcp[TCP_SEQUENCE]);
    }
    
    NetPutW(&tcp[TCP_CHECKSUM], 0);
    checksum = tcpPseudoChecksum(packet, pcb->u32LocalIp, pcb->u32PeerIp);
    NetPutW(&tcp[TCP_CHECKSUM], checksum);
    
    MP_DEBUG5("tcpOutputSegment: seq=0x%x ack=0x%x scnt1=%d,scnt2=%d,tick=%d", pcb->u32SendNext,pcb->u32RcvNext,
            slowcnt1,slowcnt2,mpx_SystemTickerGet());
    //DPrintf("[TCP] output packet, seq = %x, ack = %x", NetGetDW(&tcp[TCP_SEQUENCE]), NetGetDW(&tcp[TCP_ACKNOWLEDGE]));
    //DPrintf("o seq = %x, ack = %x", NetGetDW(&tcp[TCP_SEQUENCE]), NetGetDW(&tcp[TCP_ACKNOWLEDGE]));
    payloadSize = packet->Net.u16PayloadSize;
    IpPacketSend(packet, IP_PROTOCOL_TCP, pcb->u32LocalIp, pcb->u32PeerIp);
#ifndef DRIVER_FREE_TXBUFFER
    packet->Net.u16PayloadSize = payloadSize;
#endif
}

void TcpPacketInput(ST_NET_PACKET *packet, U32 srcAddr, U32 destAddr)
{
    U08* tcp = NET_PACKET_TCP(packet);
    U16 lenFlag = NetGetW(&tcp[TCP_LENGTH_FLAG]);
    U16 checksum;
    U08 hederLen = 0;
    U16 srcPort, dstPort;
    ST_TCP_PCB *pcb = 0, *prevPcb = 0;
    int ret;
    
    if(destAddr == 0xffffffff)
    {
        DPrintf("[TCP] don't process incoming broadcasts");
        netBufFree(packet);
        return;
    }
    
    checksum = tcpPseudoChecksum(packet, srcAddr, destAddr);
    if(checksum != 0)
    {
        DPrintf("[TCP] checksum error(%d)", checksum);
        DPrintf("From \\-");
        NetDebugPrintIP(srcAddr);
        DPrintf("To \\-");
        NetDebugPrintIP(destAddr);
        
        //DPrintf("size = %d", packet->Net.u16PayloadSize);
        //NetPacketDump(tcp, packet->Net.u16PayloadSize);
        
        netBufFree(packet);
        return;
    }
    
    TCP_ENTER_CRITICAL;
    
    hederLen = ((lenFlag >> 12) & 0xf) << 2;
    srcPort = NetGetW(&tcp[TCP_SRC_PORT]);
    dstPort = NetGetW(&tcp[TCP_DST_PORT]);
    
    pcb = 0;
    prevPcb = 0;
    
    //find the corresponding active pcb to the handle this packet
    for(pcb = tcpActivePcb; pcb != 0; pcb = (ST_TCP_PCB*)pcb->next)
    {
#ifdef NEW_NETSOCKET
        if((pcb->u16LocalPort == dstPort) && (pcb->u16PeerPort == srcPort) &&
            ((pcb->u32LocalIp == 0) || (pcb->u32LocalIp == destAddr)) && (pcb->u32PeerIp == srcAddr))
#else
        if((pcb->u16LocalPort == dstPort) && (pcb->u16PeerPort == srcPort) &&
            (pcb->u32LocalIp == destAddr) && (pcb->u32PeerIp == srcAddr))
#endif

        {
            if(prevPcb)
            {
                prevPcb->next = pcb->next;
                pcb->next = tcpActivePcb;
                tcpActivePcb = pcb;
            }
            break;
        }
        
        prevPcb = pcb;
    }
    
    if(!pcb)
    {
        for(pcb = tcpTimeWaitPcb; pcb != 0; pcb = pcb->next);
        {
#ifdef NEW_NETSOCKET
            if((pcb->u16LocalPort == dstPort) && (pcb->u16PeerPort == srcPort) &&
                ((pcb->u32LocalIp == 0) || (pcb->u32LocalIp == destAddr)) && (pcb->u32PeerIp == srcAddr))
#else
            if((pcb->u16LocalPort == dstPort) && (pcb->u16PeerPort == srcPort) &&
                (pcb->u32LocalIp == destAddr) && (pcb->u32PeerIp == srcAddr))
#endif
            {
                DPrintf("[TCP] timewait pcb process packet (%d)", pcb->u16SockId);
                
                pcb->stInput.packet = (U32)packet;
                pcb->stInput.u08Flag = (NetGetW(&tcp[TCP_LENGTH_FLAG]) & 0x3f);
                pcb->stInput.u08HdrLength = ((NetGetW(&tcp[TCP_LENGTH_FLAG]) >> 12) & 0xf) << 2;
                pcb->stInput.u16DataLength = packet->Net.u16PayloadSize - pcb->stInput.u08HdrLength;
                pcb->stInput.u32Sequence = NetGetDW(&tcp[TCP_SEQUENCE]);
                pcb->stInput.u32Acknowledge = NetGetDW(&tcp[TCP_ACKNOWLEDGE]);
                pcb->stInput.u16Window = NetGetW(&tcp[TCP_WINDOW]);
                
                pcb->stInput.u08RcvFlag = 0;
                pcb->stInput.u32RcvData = 0;
                
                if((pcb->stInput.u08Flag & TCP_FIN) || (pcb->stInput.u08Flag & TCP_SYN))
                    pcb->stInput.u16TcpLength = pcb->stInput.u16DataLength + 1;
                else
                    pcb->stInput.u16TcpLength = pcb->stInput.u16DataLength;
                    
                tcpTimeWaitInput(pcb);
            
                netBufFree(packet);
                goto FINISH;
            }
        }
        
        //DPrintf("[TCP] check listen pcb list, destAddr = %x, dstPort = %d", destAddr, dstPort);
        prevPcb = 0;
        for(pcb = tcpListenPcb; pcb != 0; pcb = pcb->next)
        {
#ifdef NEW_NETSOCKET
            if((srcAddr) && (srcPort) &&
                    (pcb->u16LocalPort == dstPort) && ((pcb->u32LocalIp == 0) || (pcb->u32LocalIp == destAddr)))
#else
            if((pcb->u16LocalPort == dstPort) && (pcb->u32LocalIp == destAddr))
#endif
            {
                if(prevPcb != 0)
                {
                    prevPcb->next = pcb->next;
                    pcb->next = tcpListenPcb;
                    tcpListenPcb = pcb;
                }
                
                //DPrintf("[TCP] listen pcb process packet (%d)", pcb->u16SockId);
                
                pcb->stInput.packet = (U32)packet;
                pcb->stInput.u08Flag = (NetGetW(&tcp[TCP_LENGTH_FLAG]) & 0x3f);
                pcb->stInput.u08HdrLength = ((NetGetW(&tcp[TCP_LENGTH_FLAG]) >> 12) & 0xf) << 2;
                //pcb->stInput.u16TcpLength = packet->Net.u16PayloadSize - pcb->stInput.u08HdrLength;
                pcb->stInput.u16DataLength = packet->Net.u16PayloadSize - pcb->stInput.u08HdrLength;
                pcb->stInput.u32Sequence = NetGetDW(&tcp[TCP_SEQUENCE]);
                pcb->stInput.u32Acknowledge = NetGetDW(&tcp[TCP_ACKNOWLEDGE]);
                pcb->stInput.u16Window = NetGetW(&tcp[TCP_WINDOW]);
                
                pcb->stInput.u32IpAddr = srcAddr;
                pcb->stInput.u16Port = srcPort;
                
                pcb->stInput.u08RcvFlag = 0;
                pcb->stInput.u32RcvData = 0;
                
                if((pcb->stInput.u08Flag & TCP_FIN) || (pcb->stInput.u08Flag & TCP_SYN))
                    pcb->stInput.u16TcpLength = pcb->stInput.u16DataLength + 1;
                else
                    pcb->stInput.u16TcpLength = pcb->stInput.u16DataLength;
                
                ret = tcpListenInput(pcb, destAddr);
                if (ret == NO_ERR)
                    netBufFree(packet);
                goto FINISH;
            }
            
            prevPcb = pcb;
        }
    }
    
    if(!pcb) //no active pcb found
    {
        /* If no matching PCB was found, send a TCP RST (reset) to the
           sender. */
        LWIP_DEBUGF(TCP_RST_DEBUG, ("tcp_input: no PCB match found, resetting.\n"));
        //NetPacketDump(tcp, packet->Net.u16PayloadSize);
        U16 flags = TCPH_FLAGS(tcp);
        if (!(flags & TCP_RST)) {
            U32 seqno, ackno;
            U16 tcplen;
            //TCP_STATS_INC(tcp.proterr);
            //TCP_STATS_INC(tcp.drop);
            seqno = NetGetDW(&tcp[TCP_SEQUENCE]);
            ackno = NetGetDW(&tcp[TCP_ACKNOWLEDGE]);
            tcplen = packet->Net.u16PayloadSize - (((NetGetW(&tcp[TCP_LENGTH_FLAG]) >> 12) & 0xf) << 2);
            if((flags & TCP_FIN) || (flags & TCP_SYN))
                tcplen++;
            tcpReset(ackno, seqno + tcplen,
                    destAddr, srcAddr,
                    dstPort, srcPort);
        }
        netBufFree(packet);
        goto FINISH;
    }
    
    if(pcb)
    {
        S32 error;
        U08* tcp = NET_PACKET_TCP(packet);
        
        pcb->stInput.packet = (U32)packet;
        pcb->stInput.u08Flag = (NetGetW(&tcp[TCP_LENGTH_FLAG]) & 0x3f);
        pcb->stInput.u08HdrLength = ((NetGetW(&tcp[TCP_LENGTH_FLAG]) >> 12) & 0xf) << 2;
        //pcb->stInput.u16TcpLength = packet->Net.u16PayloadSize - pcb->stInput.u08HdrLength;
        pcb->stInput.u16DataLength = packet->Net.u16PayloadSize - pcb->stInput.u08HdrLength;
        pcb->stInput.u32Sequence = NetGetDW(&tcp[TCP_SEQUENCE]);
        pcb->stInput.u32Acknowledge = NetGetDW(&tcp[TCP_ACKNOWLEDGE]);
        pcb->stInput.u16Window = NetGetW(&tcp[TCP_WINDOW]);
        
        //DPrintf("i, seq = %x, ack = %x", pcb->stInput.u32Sequence, pcb->stInput.u32Acknowledge);
        
        pcb->stInput.u08RcvFlag = 0;
        pcb->stInput.u32RcvData = 0;
        
        if((pcb->stInput.u08Flag & TCP_FIN) || (pcb->stInput.u08Flag & TCP_SYN))
            pcb->stInput.u16TcpLength = pcb->stInput.u16DataLength + 1;
        else
            pcb->stInput.u16TcpLength = pcb->stInput.u16DataLength;

        error = tcpProcess(pcb);
        /* A return value of ERR_ABRT means that tcp_abort() was called
        and that the pcb has been freed. If so, we don't do anything. */
        if(error != ERR_ABRT){
            if(pcb->stInput.u08RcvFlag & TF_RESET){
                U16 sid = pcb->u16SockId;       /* save sockid before memory free */
                /* TF_RESET means that the connection was reset by the other
                end. We then call the error callback to inform the
                application that the connection is dead before we
                deallocate the PCB. */
                if(pcb->stInput.packet)
                {
                    netBufFree(pcb->stInput.packet);
                    pcb->stInput.packet = 0;
                }
                MP_DEBUG("[TCP] TF_RESET");
                if (sid == 0 && pcb->u16ListenSockId > 0 && pcb->eState == E_TCP_STATE_ESTABLISHED)
                {
                    tcpPcbRemove(&tcpActivePcb, pcb);
                    //TcpFree(pcb);           /* can't free it; this pcb is in connect_queue for socket layer */
                }
                else
                {
                    tcpPcbRemove(&tcpActivePcb, pcb);
                    TcpFree(pcb);
                }

                if (sid > 0)
                    mpx_EventSet(u08TcpEventId, 0x00000001 << sid);
#ifdef NEW_NETSOCKET
                if (sid)
                {
                    sockErrorSet(sid, ECONNRESET);
                    sockIsDisconnected(sid);
                }
#endif
            }
            else if(pcb->stInput.u08RcvFlag & TF_CLOSED){
                U16 sid = pcb->u16SockId;
                /* The connection has been closed and we will deallocate the
                PCB. */
                if(pcb->stInput.packet)
                {
                    netBufFree(pcb->stInput.packet);
                    pcb->stInput.packet = 0;
                }
                MP_DEBUG("[TCP] TF_CLOSED");
                tcpPcbRemove(&tcpActivePcb, pcb);
                TcpFree(pcb);
                //pcb->u08StatusFlag |= TCP_PCB_CLOSED;

#ifdef NEW_NETSOCKET
                if (sid)
                {
                    sockErrorSet(sid, 0);
                    sockIsDisconnected(sid);
                }
#endif
                if (sid > 0)
                    mpx_EventSet(u08TcpEventId, 0x00000001 << sid);
            }
            else{
                error = NO_ERR;
                
                if(pcb->u16Acked > 0)
                {
                }
                
                if(pcb->stInput.u32RcvData)
                {
                    if(pcb->u16SockId)
                    {
#ifdef NEW_NETSOCKET
                        if (sockIsCantRecvMore(pcb->u16SockId))
                        {
                            tcpSendReset(pcb, packet, srcAddr, destAddr);
                            netBufFree(packet);
                        }
                        else
#endif
                        SockIdSignalRxDone(pcb->u16SockId, (void*)packet);
#if TCP_QUEUE_OOSEQ
                        if(pcb->pstConcateSeq)
                        {
                            ST_TCP_SEG* cseg = pcb->pstConcateSeq;
                            
                            //DPrintf("%d packets in concate queue", pcb->concateLen);
                            
                            while(pcb->pstConcateSeq)
                            {
                                SockIdSignalRxDone(pcb->u16SockId, pcb->pstConcateSeq->packet);
                                cseg = pcb->pstConcateSeq;
                                pcb->pstConcateSeq = pcb->pstConcateSeq->next;
                                pcb->concateLen--;
                                mpx_Free(cseg);
                            }
                            
                            //DPrintf("%d packets remained in concate queue", pcb->concateLen);
                        }
#endif
                    }
                    else
                    {
                        if(pcb->u16ListenSockId)
                        {
                            if(!pcb->u32PacketQueue)
                            {
                                pcb->u32PacketQueue = (U32)mpx_QueueCreate();
                                if(!pcb->u32PacketQueue)
                                {
                                    DPrintf("[TCP] queue create fail");
                                    netBufFree(packet);
                                }
                            }
                            
                            if(0 != mpx_QueuePush((ST_QUEUE*)(pcb->u32PacketQueue), (U32)packet))
                            {
                                DPrintf("[TCP] queue push fail");
                                netBufFree(packet);
                            }
#if TCP_QUEUE_OOSEQ
                            while(pcb->pstConcateSeq)
                            {
                                ST_TCP_SEG* cseg;
                                if(0 != mpx_QueuePush((ST_QUEUE*)(pcb->u32PacketQueue), (U32)pcb->pstConcateSeq->packet))
                                {
                                    DPrintf("[TCP] packet queue failed");
                                    netBufFree(pcb->pstConcateSeq->packet);
                                }
                                cseg = pcb->pstConcateSeq;
                                pcb->pstConcateSeq = pcb->pstConcateSeq->next;
                                pcb->concateLen--;
                                mpx_Free(cseg);
                            }
#endif
                        }
                        else
                        {
                            tcpSendReset(pcb, packet, srcAddr, destAddr);
                            netBufFree(packet);
                        }
                    }
                }
                
                if(pcb->stInput.u08RcvFlag & TF_GOT_FIN)
                {
                    //DPrintf("[TCP] got fin, s=%d", pcb->u16SockId);
                    if (pcb->u16SockId)
                    {
                        /* -----  make sure socket is not closed yet  ----- */
                        mpx_EventSet(u08TcpEventId, 0x00000001 << pcb->u16SockId);
                        SockIdSignalTcpFinRecvd(pcb->u16SockId);
                    }
                }
                
                if(error == NO_ERR)
                    tcpOutput(pcb);
        
                if(pcb->stInput.packet)
                {
                    netBufFree(pcb->stInput.packet);
                    pcb->stInput.packet = 0;
                }
            }

        }
        
    }

FINISH:
    TCP_EXIT_CRITICAL;
    
    return;
}

static S32 tcpProcess(ST_TCP_PCB* pcb)
{
    ST_NET_PACKET* packet = (ST_NET_PACKET*)pcb->stInput.packet;
    U08* tcp = NET_PACKET_TCP(packet);
    U08   acceptable = 0;

    /* Process incoming RST segments. */
    if(pcb->stInput.u08Flag & TCP_RST)
    {
    /* First, determine if the reset is acceptable. */
        if (pcb->eState == E_TCP_STATE_SYN_SENT)
        {
            if(pcb->stInput.u32Acknowledge == pcb->u32SendNext)
                acceptable = 1;
        } 
        else
        {
            //if (TCP_SEQ_BETWEEN(pcb->stInput.u32Sequence, pcb->u32RcvNext, pcb->u32RcvNext+pcb->u16RcvWindow))
            //if((pcb->stInput.u32Sequence >= pcb->u32RcvNext) && ((pcb->stInput.u32Sequence - pcb->u32RcvNext) <= pcb->u16RcvWindow))
            if (TCP_SEQ_BETWEEN(pcb->stInput.u32Sequence, pcb->u32RcvNext, pcb->u32RcvNext+pcb->u16RcvWindow))
                acceptable = 1;
        }
        
        if(acceptable)
        {
            pcb->stInput.u08RcvFlag |= TF_RESET;
            pcb->u08Flag &= ~TF_ACK_DELAY;
            return ERR_RST;
        } 
        else
            return ERR_OK;
    }
    
    if ((pcb->stInput.u08Flag & TCP_SYN) && (pcb->eState != E_TCP_STATE_SYN_SENT && pcb->eState != E_TCP_STATE_SYN_RCVD)) { 
        /* Cope with new connection attempt after remote end crashed */
        tcpAckNow(pcb);
        return ERR_OK;
    }

    pcb->u32Timer = u32TcpTicks;
    pcb->u08KeepCounter = 0;
    
    //LWIP_DEBUGF(TCP_DEBUG, ("tcpProcess: st=%d (before).\n", pcb->eState));
    switch(pcb->eState)
    {
        case E_TCP_STATE_CLOSE_WAIT:
        case E_TCP_STATE_ESTABLISHED:
        {
            //DPrintf("[TCP] state: E_TCP_STATE_ESTABLISHED (%d)", pcb->u16SockId);
            tcpPacketReceive(pcb);
            if(pcb->stInput.u08RcvFlag & TF_GOT_FIN)
            {
                if (pcb->eState == E_TCP_STATE_ESTABLISHED)
                {
                    MP_DEBUG("[TCP] state E_TCP_STATE_ESTABLISHED change to E_TCP_STATE_CLOSE_WAIT (%d)", pcb->u16SockId);
                }
                tcpAckNow(pcb);
                pcb->eState = E_TCP_STATE_CLOSE_WAIT;
            }
        }
        break;
        
        case E_TCP_STATE_SYN_SENT:
        {
            ST_TCP_SEG* unackSeg = pcb->pstUnacked;
            U08* tcpUnacked = NET_PACKET_TCP(unackSeg->packet);
            
            if((pcb->stInput.u08Flag & TCP_ACK) && (pcb->stInput.u08Flag & TCP_SYN) && 
                (pcb->stInput.u32Acknowledge == (NetGetDW(&tcpUnacked[TCP_SEQUENCE]) + 1)))
            {
                pcb->u16SendBuffer++;
                if(pcb->u16SendBuffer)
                {
                    if (pcb->u16SockId > 0)
                        mpx_EventSet(u08TcpEventId, 0x00000001 << pcb->u16SockId);
                    else
                        DPrintf("[TCP] st=%d,pcb->u16SockId = 0", pcb->eState);
                }
                
                pcb->u32RcvNext = pcb->stInput.u32Sequence + 1;
                pcb->rcv_ann_right_edge = pcb->u32RcvNext;
                pcb->u32LastAck = pcb->stInput.u32Acknowledge;
                pcb->u32SendWindow = NetGetW(&tcp[TCP_WINDOW]);
                pcb->u32SendWL1 = pcb->stInput.u32Sequence - 1;
                pcb->eState = E_TCP_STATE_ESTABLISHED;

                if(pcb->stInput.u08HdrLength > sizeof(ST_TCP_HEADER))
                    tcpParseOption(pcb, NET_PACKET_TCP_DATA(packet), pcb->stInput.u08HdrLength - sizeof(ST_TCP_HEADER));
                
                /* Set ssthresh again after changing pcb->mss (already set in tcp_connect
                 * but for the default value of pcb->mss) */
                pcb->u16SSThresh = pcb->u16Mss * 10;

                pcb->u16CWindow = ((pcb->u16CWindow == 1) ? (pcb->u16Mss * 2) : pcb->u16Mss);
                
                pcb->u08SendQueueLen--;
                //DPrintf("[TCP] sock %d, queue len = %d", pcb->u16SockId, pcb->u08SendQueueLen);
                
                pcb->pstUnacked = unackSeg->next;
                
                /* If there's nothing left to acknowledge, stop the retransmit
                   timer, otherwise reset it to start again */
                if(pcb->pstUnacked == NULL)
                    pcb->u16RetranTime = -1;
                else
                {
                    pcb->u16RetranTime = 0;
                    pcb->u08RetranNumber = 0;
                }

                tcpFreeSegment(unackSeg);
                
                //tcpAck(pcb);
                tcpAckNow(pcb);
                pcb->u08StatusFlag |= TCP_PCB_CONNECTED;
#ifdef NEW_NETSOCKET
                //sockErrorSet(pcb->u16SockId, 0);
                sockIsConnected(pcb->u16SockId);
#endif
            }
            else if(pcb->stInput.u08Flag & TCP_ACK) 
            {
                pcb->u08StatusFlag |= TCP_PCB_RESET; /* TODO */
                tcpReset(pcb->stInput.u32Acknowledge, pcb->stInput.u32Sequence + pcb->stInput.u16TcpLength, 
                            pcb->u32LocalIp, pcb->u32PeerIp, pcb->u16LocalPort, pcb->u16PeerPort);
            }
        }
        break;
        
        case E_TCP_STATE_SYN_RCVD:
        {
            if(pcb->stInput.u08Flag & TCP_ACK)
            {
                if(TCP_SEQ_BETWEEN(pcb->stInput.u32Acknowledge, pcb->u32LastAck+1, pcb->u32SendNext))
                {
                    u16_t old_cwnd;
                    //DPrintf("[TCP] connected from \\-");
                    //NetDebugPrintIP(pcb->u32PeerIp);
                    pcb->eState = E_TCP_STATE_ESTABLISHED;
                    
                    if(NO_ERR != pcb->accept(pcb->u16ListenSockId, pcb))
                    {
                        DPrintf("[TCP] self abort");
                        tcpSelfAbort(pcb, 1);
                        return ERR_ABRT;
                    }
                    old_cwnd = pcb->u16CWindow;
                    
                    tcpPacketReceive(pcb);

                    /* Prevent ACK for SYN to generate a sent event */
                    if(pcb->u16Acked > 0) {
                        pcb->u16Acked--;
                    }

                    pcb->u16CWindow = ((old_cwnd == 1) ? (pcb->u16Mss * 2) : pcb->u16Mss);

                    if (pcb->stInput.u08RcvFlag & TF_GOT_FIN) { /* TODO which patch exactly ? */
                        tcpAckNow(pcb);
                        pcb->eState = E_TCP_STATE_CLOSE_WAIT;
                    }
                }
                /* incorrect ACK number */
                else{
                    /* send RST */
                    tcpReset(pcb->stInput.u32Acknowledge, pcb->stInput.u32Sequence + pcb->stInput.u16TcpLength, 
                                pcb->u32LocalIp, pcb->u32PeerIp, pcb->u16LocalPort, pcb->u16PeerPort);
                }
            }
            else if ((pcb->stInput.u08Flag & TCP_SYN) && (pcb->stInput.u32Sequence == pcb->u32RcvNext - 1)) {
                /* Looks like another copy of the SYN - retransmit our SYN-ACK */
                tcpRexmit(pcb);
            }
        }
        break;
        
        case E_TCP_STATE_FIN_WAIT_1:
        {
            tcpPacketReceive(pcb);
            if(pcb->stInput.u08RcvFlag & TF_GOT_FIN)
            {
                MP_DEBUG("[TCP] E_TCP_STATE_FIN_WAIT_1, ack = %x, sendnext = %x", pcb->stInput.u32Acknowledge, pcb->u32SendNext);
                if((pcb->stInput.u08Flag & TCP_ACK) && (pcb->stInput.u32Acknowledge == pcb->u32SendNext))
                {
                    LWIP_DEBUGF(TCP_DEBUG,
                            ("TCP connection closed %u -> %u.\n", NetGetW(&tcp[TCP_SRC_PORT]), NetGetW(&tcp[TCP_DST_PORT])));
                    tcpAckNow(pcb);
                    tcpPcbPurge(pcb);
                    tcpRemove(&tcpActivePcb, pcb);
                    pcb->eState = E_TCP_STATE_TIME_WAIT;
                    tcpPcbRegister(&tcpTimeWaitPcb, pcb);
                    //DPrintf("[TCP] state E_TCP_STATE_FIN_WAIT_1 change to E_TCP_STATE_TIME_WAIT");
#ifdef NEW_NETSOCKET
                    if (pcb->u16SockId)
                    {
                        int sid = pcb->u16SockId;
                        sock_Detach(sid);
                        sockErrorSet(sid, 0);
                        sockIsDisconnected(sid);
                        pcb->u16SockId = 0;
                    }
#endif
                }
                else
                {
                    tcpAckNow(pcb);
                    pcb->eState = E_TCP_STATE_CLOSING; /* TODO 20091002 reverse order from prev line */
                    //DPrintf("[TCP] state E_TCP_STATE_FIN_WAIT_1 change to E_TCP_STATE_CLOSING");
                }
            } 
            else if((pcb->stInput.u08Flag & TCP_ACK) && (pcb->stInput.u32Acknowledge == pcb->u32SendNext))
            {
                pcb->eState = E_TCP_STATE_FIN_WAIT_2;
            }
        }
        break;
        
        case E_TCP_STATE_FIN_WAIT_2:
        {
            tcpPacketReceive(pcb);
            if(pcb->stInput.u08RcvFlag & TF_GOT_FIN)
            {
                LWIP_DEBUGF(TCP_DEBUG, ("TCP connection closed %u-> %u.\n", NetGetW(&tcp[TCP_SRC_PORT]), NetGetW(&tcp[TCP_DST_PORT])));
                tcpAckNow(pcb);
                tcpPcbPurge(pcb);
                tcpRemove(&tcpActivePcb, pcb);
                pcb->eState = E_TCP_STATE_TIME_WAIT;
                tcpPcbRegister(&tcpTimeWaitPcb, pcb);
#ifdef NEW_NETSOCKET
                if (pcb->u16SockId)
                {
                    int sid = pcb->u16SockId;
                    sock_Detach(sid);
                    sockErrorSet(sid, 0);
                    sockIsDisconnected(sid);
                    pcb->u16SockId = 0;
                }
#endif
                //DPrintf("[TCP] state E_TCP_STATE_FIN_WAIT_2 change to E_TCP_STATE_TIME_WAIT");
            }
        }
        break;
        
        case E_TCP_STATE_CLOSING:
        {
            tcpPacketReceive(pcb);
            if((pcb->stInput.u08Flag & TCP_ACK) && (pcb->stInput.u32Acknowledge == pcb->u32SendNext))
            {
                LWIP_DEBUGF(TCP_DEBUG, ("TCP connection closed %u-> %u.\n", NetGetW(&tcp[TCP_SRC_PORT]), NetGetW(&tcp[TCP_DST_PORT])));
                //tcpAckNow(pcb);
                tcpPcbPurge(pcb);
                tcpRemove(&tcpActivePcb, pcb);
                pcb->eState = E_TCP_STATE_TIME_WAIT;
                tcpPcbRegister(&tcpTimeWaitPcb, pcb);
                //DPrintf("[TCP] state E_TCP_STATE_CLOSING change to E_TCP_STATE_TIME_WAIT");
#ifdef NEW_NETSOCKET
                if (pcb->u16SockId)
                {
                    int sid = pcb->u16SockId;
                    sock_Detach(sid);
                    sockErrorSet(sid, 0);
                    sockIsDisconnected(sid);
                    pcb->u16SockId = 0;
                }
#endif
            }
        }
        break;
        
        case E_TCP_STATE_LAST_ACK:
        {
            //DPrintf("[TCP] state: E_TCP_STATE_LAST_ACK (%d)", pcb->u16SockId);
            tcpPacketReceive(pcb);
            //DPrintf("   input ack = %x, sendnext = %x", pcb->stInput.u32Acknowledge, pcb->u32SendNext);
            if((pcb->stInput.u08Flag & TCP_ACK) && (pcb->stInput.u32Acknowledge == pcb->u32SendNext))
            {
                LWIP_DEBUGF(TCP_DEBUG, ("TCP connection closed %u-> %u.\n", NetGetW(&tcp[TCP_SRC_PORT]), NetGetW(&tcp[TCP_DST_PORT])));
                //DPrintf("[TCP] state E_TCP_STATE_LAST_ACK change to E_TCP_STATE_CLOSED (%d)", pcb->u16SockId);
                pcb->eState = E_TCP_STATE_CLOSED; /* TODO not in LWIP's code ??? */
                pcb->stInput.u08RcvFlag |= TF_CLOSED;
            }
        }
        break;
        
        default:
        break;
    }
    //LWIP_DEBUGF(TCP_DEBUG, ("tcpProcess: st=%d (after).\n", pcb->eState));
    return ERR_OK;
}

static void tcpParseOption(ST_TCP_PCB* pcb, U08* optData, U08 optLength)
{
    U08 c;
    U08 opt;
    U16 mss;
    
    //DPrintf("[TCP] option length = %d", optLength);
    for(c = 0; c < optLength;)
    {
        opt = optData[c];
        if(opt == 0x00) // end of option
            break;
        else if(opt == 0x01) //NOP option
            c++;
        else if((opt == 0x02) && (optData[c+1] == 0x04)) //MSS
        {
            mss = NetGetW(&optData[c+2]);
            /* Limit the mss to the configured TCP_MSS and prevent division by zero */
            pcb->u16Mss = ((mss > TCP_MSS) || (mss == 0)) ? TCP_MSS : mss;
            break;
        }
        else
        {
            if(optData[c+1] == 0)
                break;
            
            //skip this option
            c += optData[c+1];
        }
    }
}

static U16 tcpPseudoChecksum(ST_NET_PACKET* packet, U32 srcIp, U32 dstIp)
{
    U08* pseudo = NET_PACKET_PSEUDO(packet);
    
    NetPutDW(&pseudo[PSEUDO_SRC_IP], srcIp);
    NetPutDW(&pseudo[PSEUDO_DST_IP], dstIp);
    pseudo[PSEUDO_ZERO] = 0;
    pseudo[PSEUDO_PROTOCOL] = IP_PROTOCOL_TCP;
    NetPutW(&pseudo[PSEUDO_LENGTH], packet->Net.u16PayloadSize);
    
    return InetCsum(&pseudo[PSEUDO_SRC_IP], packet->Net.u16PayloadSize + TCP_PSEUDO_HEADER_LENGTH);
}

static void tcpAck(ST_TCP_PCB* pcb)
{
//#define REDUCE_ACK_NUM
#ifndef REDUCE_ACK_NUM
    if(pcb->u08Flag & TF_ACK_DELAY)
    {
        pcb->u08Flag &= ~TF_ACK_DELAY;
        pcb->u08Flag |= TF_ACK_NOW;
        
        tcpOutput(pcb);
    }
    else
#endif
    {
        pcb->u08Flag |= TF_ACK_DELAY;
    }
}

static void tcpAckNow(ST_TCP_PCB* pcb)
{
    pcb->u08Flag |= TF_ACK_NOW;
    tcpOutput(pcb);
}

static void tcpReset(U32 seqno, U32 ackno, U32 srcIp, U32 dstIp, U16 srcPort, U16 dstPort)
{
    ST_NET_PACKET* rstPacket = NetNewPacket(FALSE);
    U08* tcpHdr;
    U16 checksum;
    
    if(!rstPacket)
    {
        DPrintf("[TCP] tcpReset allocate buffer fail");
        return;
    }
    
    tcpHdr = NET_PACKET_TCP(rstPacket);
    
    NetPutW(&tcpHdr[TCP_SRC_PORT], srcPort);
    NetPutW(&tcpHdr[TCP_DST_PORT], dstPort);
    NetPutDW(&tcpHdr[TCP_SEQUENCE], seqno);
    NetPutDW(&tcpHdr[TCP_ACKNOWLEDGE], ackno);
    NetPutW(&tcpHdr[TCP_LENGTH_FLAG], (U16)(0x5000 | TCP_ACK | TCP_RST));
    NetPutW(&tcpHdr[TCP_WINDOW], u16TcpWindowSize);
    NetPutW(&tcpHdr[TCP_URGENT], 0);
    
    rstPacket->Net.u16PayloadSize = sizeof(ST_TCP_HEADER);
    NetPutW(&tcpHdr[TCP_CHECKSUM], 0);
    checksum = tcpPseudoChecksum(rstPacket, srcIp, dstIp);
    NetPutW(&tcpHdr[TCP_CHECKSUM], checksum);
    
    IpPacketSend(rstPacket, IP_PROTOCOL_TCP, srcIp, dstIp);
#ifndef DRIVER_FREE_TXBUFFER
    NetFreePacket(rstPacket);
#endif
}

static void tcpTimerProc()
{
    //MP_DEBUG("[TCP] tcpTimerProc");
    tcpFastTimer();
    
    if(u32TcpTimer++ & 1)
        tcpSlowTimer();
}

void TCPFastTimerProc()
{
    ST_TCP_PCB* pcb;
    
    TCP_ENTER_CRITICAL;
    for(pcb = tcpActivePcb; pcb != 0; pcb = (ST_TCP_PCB*)pcb->next)
    {
        if(pcb->u08Flag & TF_ACK_DELAY)
        {
            pcb->u08Flag &= ~TF_ACK_DELAY;
            tcpAckNow(pcb);
        }
    }
    TCP_EXIT_CRITICAL;
    boolTcpFastTimer = FALSE;
}

#ifdef HAVE_RTSP
BYTE tcp_recv_task_id, tcp_recv_event_id;
#endif
static void tcpFastTimer()
{
#ifndef HAVE_RTSP
    U32 message[2];
    
    if(!boolTcpFastTimer)
    {
        boolTcpFastTimer = TRUE;
        message[0] = NETCTRL_MSG_TCP_FAST_TIMER;
        message[1] = (U32)TCPFastTimerProc;
        //mpx_MessageSend(NetCtrlMessageIdGet(), message, sizeof(message));
        if(NO_ERR != mpx_MessageDrop(NetCtrlMessageIdGet(), message, sizeof(message)))
        {
            DPrintf("[TCP] tcpFastTimer message drop fail");
        }
    }
#else
    EventSet(tcp_recv_event_id, 8);
#endif
}

static void TcpSlowTimerProc()
{
    ST_TCP_PCB *pcb, *pcb2, *prev;
    U32 effWindow;
    U08 pcbRemove;   /* flag if a PCB should be removed */
    S32 err = NO_ERR;
    U16 tcp_close_all = FALSE;
    
    slowcnt2++;
    
    TCP_ENTER_CRITICAL;
    
    u32TcpTicks++;
#ifdef PLATFORM_MPIXEL
    if (u32TcpOff > 0)
    {
        DPrintf("[TCP] TcpSlowTimerProc: %d", u32TcpOff);
        u32TcpTicks += u32TcpOff;
        u32TcpOff = 0;
        tcp_close_all = TRUE;
    }
#endif
    prev = 0;
    pcb = tcpActivePcb;
    
#if 1
    while(pcb)
    {
        pcbRemove = 0;
        if (tcp_close_all)
        {
            short rtn;
            if (pcb->eState == E_TCP_STATE_SYN_SENT)
                rtn = TCP_SYNMAXRTX;
            else
                rtn = TCP_MAXRTX;
			if(pcb->u32LocalIp == NicArray[NIC_INDEX_WIFI].u32DefaultIp && !netif_carrier_ok(&NicArray[NIC_INDEX_WIFI]))
            	pcb->u08RetranNumber = rtn;
			if(pcb->u32LocalIp == NicArray[NIC_INDEX_PPP].u32DefaultIp && !netif_carrier_ok(&NicArray[NIC_INDEX_PPP]))
            	pcb->u08RetranNumber = rtn;

        }
        
#ifdef LINUX
        if((pcb->eState == E_TCP_STATE_SYN_SENT) && (pcb->u08RetranNumber == TCP_SYNMAXRTX))
        {
            pcbRemove++;
            LWIP_DEBUGF(TCP_DEBUG, ("tcp_slowtmr: max SYN retries reached\n"));
#if TRACETCP > 0
            ntrace_printf(0, "[TCP] tcp_slowtmr: max SYN retries reached %u,%u,%u",
                    pcb->u16SockId, pcb->u16LocalPort, pcb->u16PeerPort);
            ntrace_write2sd();
#endif
        }
        else 
#endif
            if(pcb->u08RetranNumber == TCP_MAXRTX)
        {
            pcbRemove++;
            LWIP_DEBUGF(TCP_DEBUG, ("tcp_slowtmr: max DATA retries reached\n"));
#if TRACETCP > 0
            ntrace_printf(0, "[TCP] tcp_slowtmr: max DATA retries reached %u,%u,%u",
                    pcb->u16SockId, pcb->u16LocalPort, pcb->u16PeerPort);
            ntrace_write2sd();
#endif
        }
        else
        {
            /* Increase the retransmission timer if it is running */
            if(pcb->u16RetranTime >= 0)
            pcb->u16RetranTime++;
//            if (pcb->u16SockId == 2)
//                MP_DEBUG("[TCP] u16RetranTime %d", pcb->u16RetranTime);
            
            //MP_DEBUG2("[TCP] TcpSlowTimerProc %d,%d", pcb->pstUnacked?1:0, pcb->u16RetranTime);
            if(pcb->pstUnacked && (pcb->u16RetranTime >= pcb->u16RetranTimeout))
            {
                MP_DEBUG2("[TCP] rexmit rto=%d rt=%d", pcb->u16RetranTimeout, pcb->u16RetranTime);
                if(pcb->eState != E_TCP_STATE_SYN_SENT)
                    pcb->u16RetranTimeout = ((pcb->s16Sa >> 3) + pcb->s16Sv) << TCP_BACKOFF[pcb->u08RetranNumber];
#if 1                                           /* added billwang 2010/04/30 */
                else if (pcb->u08RetranNumber < TCP_SYNMAXRTX)
                {
                    pcb->u16RetranTimeout = (3000 / TCP_SLOW_INTERVAL) << TCP_BACKOFF2[pcb->u08RetranNumber];
                    if (pcb->u16RetranTimeout > 18000/TCP_SLOW_INTERVAL)
                        pcb->u16RetranTimeout = 18000/TCP_SLOW_INTERVAL;
                }
                else                            /* pcb->u08RetranNumber == TCP_SYNMAXRTX */
                {
                    pcbRemove++;
                    LWIP_DEBUGF(TCP_DEBUG, ("tcp_slowtmr: max SYN retries reached\n"));
#if TRACETCP > 0
                    ntrace_printf(0, "[TCP] tcp_slowtmr: max SYN retries reached %u,%u,%u",
                            pcb->u16SockId, pcb->u16LocalPort, pcb->u16PeerPort);
                    ntrace_write2sd();
#endif
                    goto out1;
                }
#endif
                
                /* Reset the retransmission timer. */
                pcb->u16RetranTime = 0;
                
                effWindow = pcb->u16CWindow > pcb->u32SendWindow ? pcb->u32SendWindow : pcb->u16CWindow;
                pcb->u16SSThresh = effWindow >> 1;
                if(pcb->u16SSThresh < pcb->u16Mss)
                    pcb->u16SSThresh = pcb->u16Mss << 1;
                pcb->u16CWindow = pcb->u16Mss;
                
                MP_DEBUG1("[TCP] rexmit new rto=%d", pcb->u16RetranTimeout);
#if TRACETCP > 0
                if (pcb->eState == E_TCP_STATE_SYN_SENT)
                {
                    ntrace_printf(0, "[TCP] re-xmit SYN,%u,%u,%u", pcb->u16SockId, pcb->u16LocalPort, pcb->u16PeerPort);
                }
#endif
                tcpRexmitRto(pcb);
            }
        }
        
out1:
        
        if(pcb->eState == E_TCP_STATE_FIN_WAIT_2)
        {
            //DPrintf("[TCP] state E_TCP_STATE_FIN_WAIT_2");
            if((U32)((S32)u32TcpTicks - (S32)pcb->u32Timer) > (TCP_FIN_WAIT_TIMEOUT / TCP_SLOW_INTERVAL))
                pcbRemove++;
        }
        
        //keepalive not imlemented
        //////////////////////////
        
		/* If this PCB has queued out of sequence data, but has been
		   inactive for too long, will drop the data (it will eventually
		   be retransmitted). */
#if 0
#if TCP_QUEUE_OOSEQ
        if((pcb->pstOOSeq) && ((U32)((S32)u32TcpTicks - (S32)pcb->u32Timer) >= (pcb->u16RetranTimeout * TCP_OOSEQ_TIMEOUT)))
        {
            DPrintf("[TCP OOSEQ] OOSEQ timeout; pcb=%p", pcb);
            tcpSegmentsFree(pcb->pstOOSeq);
            pcb->pstOOSeq = NULL;
        }
#endif
#endif
        
		/* Check if this PCB has stayed too long in SYN-RCVD */
        if(pcb->eState == E_TCP_STATE_SYN_RCVD)
        {
            if((U32)((S32)u32TcpTicks - (S32)pcb->u32Timer) > (TCP_SYN_RCVD_TIMEOUT / TCP_SLOW_INTERVAL))
                pcbRemove++;
        }
        
        if(pcb->eState == E_TCP_STATE_LAST_ACK)
        {
            //DPrintf("[TCP] state E_TCP_STATE_LAST_ACK");
            if((U32)((S32)u32TcpTicks - (S32)pcb->u32Timer) > (2 * TCP_MSL / TCP_SLOW_INTERVAL))
                pcbRemove++;
        }
        
        if(pcbRemove)
        {
#ifdef NEW_NETSOCKET
            int sid = pcb->u16SockId;
#endif
            LWIP_DEBUGF(TCP_DEBUG, ("TcpSlowTimerProc removes st=%d.\n", pcb->eState));
            tcpPcbPurge(pcb);
            if(prev)
                prev->next = pcb->next;
            else
                tcpActivePcb = pcb->next;
                
            pcb2 = pcb->next;
            //remove pcb here??
            //DPrintf("[TCP] active pcb freed, must notify upper layer");
            TcpFree(pcb);
            pcb = pcb2;
#ifdef NEW_NETSOCKET
            if (sid)
            {
                sockErrorSet(sid, ETIMEDOUT);
                sockIsDisconnected(sid); // TODO
            }
#endif
        }
        else
        {
            pcb->u08PollTimer++;
            if(pcb->u08PollTimer >= pcb->u08PollInterval)
            {
                pcb->u08PollTimer = 0;
                //poll callback??
                //DPrintf("[TCP] notify!!");
                tcpOutput(pcb);
            }
            
            prev = pcb;
            pcb = pcb->next;
        }
    }
    
    prev = 0;
    pcb = tcpTimeWaitPcb;
    while(pcb)
    {
        pcbRemove = 0;
        if(((S32)u32TcpTicks - (S32)pcb->u32Timer) > (2 * TCP_MSL / TCP_SLOW_INTERVAL))
            pcbRemove++;
        
        if(pcbRemove)
        {
#ifdef NEW_NETSOCKET
            int sid = pcb->u16SockId;
#endif
            LWIP_DEBUGF(TCP_DEBUG,
                    ("TcpSlowTimerProc: timewait TCP freed %u -> %u.\n", pcb->u16LocalPort, pcb->u16PeerPort));
            tcpPcbPurge(pcb);
            if(prev)
                prev->next = pcb->next;
            else
                tcpTimeWaitPcb = pcb->next;
            
            MP_DEBUG("[TCP] timewait pcb freed");
            pcb2 = pcb->next;
            TcpFree(pcb);
            pcb = pcb2;
#ifdef NEW_NETSOCKET
            if (sid)
            {
                sockErrorSet(sid, 0);
                sockIsDisconnected(sid);
            }
#endif
        }
        else
        {
            prev = pcb;
            pcb = pcb->next;
        }
    }
#endif

    TCP_EXIT_CRITICAL;
    boolTcpSlowTimer = FALSE;
}

static void tcpSlowTimer()
{
#ifndef HAVE_RTSP
    U32 message[2];
    
    if(!boolTcpSlowTimer)
    {
        boolTcpSlowTimer = TRUE;
        slowcnt1++;
//        if ((slowcnt1++ % 100) == 0)
//            DPrintf("##### [TCP] tcpSlowTimer cnt1=%d,cnt2=%d", slowcnt1,slowcnt2);
        message[0] = NETCTRL_MSG_TCP_SLOW_TIMER;
        message[1] = (U32)TcpSlowTimerProc;
        //mpx_MessageSend(NetCtrlMessageIdGet(), message, sizeof(message));
        if(NO_ERR != mpx_MessageDrop(NetCtrlMessageIdGet(), message, sizeof(message)))
        {
            DPrintf("[TCP] tcpSlowTimer message drop fail");
        }
    }
#else
    EventSet(tcp_recv_event_id, 0x10);
#endif
}

static BOOL tcpWaitConnected(ST_TCP_PCB* pcb)
{
    U32 timeout = 2000;
    
    while(timeout)
    {
        if(pcb->u08StatusFlag & TCP_PCB_CONNECTED)
        {
            return TRUE;
        }
        
        if(pcb->u08StatusFlag & TCP_PCB_RESET)
            return FALSE;
        
        timeout--;
        mpx_TaskYield(1);
    }
    
    return FALSE;
}

static BOOL tcpWaitClosed(ST_TCP_PCB* pcb)
{
    U32 timeout = 500;
    
    while(timeout)
    {
        if(pcb->u08StatusFlag & TCP_PCB_CLOSED)
        {
            DPrintf("[TCP] closed");
            return TRUE;
        }
        
        timeout--;
        mpx_TaskYield(1);
    }
    
    DPrintf("[TCP] wait close timeout");
    return TRUE;
}

#if TCP_QUEUE_OOSEQ
/**
 * Insert segment into the list (segments covered with new one will be deleted)
 *
 * Called from tcp_receive()
 */
static void
tcp_oos_insert_segment(struct tcp_seg *cseg, struct tcp_seg *next)
{
  int off;
  char *tcpHdr = NET_PACKET_TCP(cseg->packet);
  U32 seqno = NetGetDW(&tcpHdr[TCP_SEQUENCE]);
  if (TCPH_FLAGS(tcpHdr) & TCP_FIN) {
    /* received segment overlaps all following segments */
    tcpSegmentsFree(next);
    next = NULL;
  }
  else {
    /* delete some following segments */
    while (next) {
        char *next_tcpHdr = NET_PACKET_TCP(next->packet);
        U32 next_seqno = NetGetDW(&next_tcpHdr[TCP_SEQUENCE]);

        if (TCP_SEQ_GT((seqno + cseg->u16Length),
                      (next_seqno + next->u16Length)))
        {
            struct tcp_seg *old_seg = next;
            next = next->next;
            tcpFreeSegment(old_seg);
        }
        else
            break;
    }
    if (next)
    {
        char *next_tcpHdr = NET_PACKET_TCP(next->packet);
        U32 next_seqno = NetGetDW(&next_tcpHdr[TCP_SEQUENCE]);
        if (TCP_SEQ_GT(seqno + cseg->u16Length, next_seqno)) {
            /* We need to trim the incoming segment. */
            off = cseg->u16Length;
            cseg->u16Length = (u16_t)(next_seqno - seqno);
            //pbuf_realloc(cseg->p, cseg->len);
            tcpPacketDeflate2((ST_NET_PACKET*)cseg->packet, off-cseg->u16Length);
        }
    }
  }
  cseg->next = next;
}
#endif

/* tcpPacketReceive:
 *
 * Called by tcp_process. Checks if the given segment is an ACK for outstanding
 * data, and if so frees the memory of the buffered data. Next, is places the
 * segment on any of the receive queues (pcb->recved or pcb->ooseq). If the segment
 * is buffered, the pbuf is referenced by pbuf_ref so that it will not be freed until
 * i it has been removed from the buffer.
 *
 * If the incoming segment constitutes an ACK for a segment that was used for RTT
 * estimation, the RTT is estimated here as well.
 *
 * @return 1 if 
 */
static BOOL tcpPacketReceive(ST_TCP_PCB* pcb)
{
    U32 rWindowEdge;
    ST_NET_PACKET* packet;
    U08* tcpHdr = 0;
    U32 seqno = 0;
    U16 tcpLen = 0;
    U08 flag;
    ST_TCP_SEG* next;
    S16 m;
    S32 off;
    BOOL accepted = FALSE;
#if TCP_QUEUE_OOSEQ
    ST_TCP_SEG *prev, *cseg;
    U16 diff;
#endif
    u16_t tcplen;
    
    if(pcb->stInput.u08Flag & TCP_ACK)
    {
        rWindowEdge = pcb->u32SendWindow + pcb->u32SendWL2;
        
        //DPrintf("u32SendWL1 = %x, u32SendWL2 = %x", pcb->u32SendWL1, pcb->u32SendWL2);
        //DPrintf("a = %x, l = %x, s = %x,", pcb->stInput.u32Acknowledge, pcb->u32LastAck, pcb->u32SendMax);
        
        /* Update window. */
        if(TCP_SEQ_LT(pcb->u32SendWL1, pcb->stInput.u32Sequence) || 
            ((pcb->u32SendWL1 == pcb->stInput.u32Sequence) && TCP_SEQ_LT(pcb->u32SendWL2, pcb->stInput.u32Acknowledge)) ||
            ((pcb->u32SendWL2 == pcb->stInput.u32Acknowledge) && (pcb->stInput.u16Window > pcb->u32SendWindow)))
        {
            pcb->u32SendWindow = pcb->stInput.u16Window;
            pcb->u32SendWL1 = pcb->stInput.u32Sequence;
            pcb->u32SendWL2 = pcb->stInput.u32Acknowledge;

            LWIP_DEBUGF(TCP_WND_DEBUG, ("tcp_receive: window update %u\n", pcb->u32SendWindow));
#if TCP_WND_DEBUG
        } else {
            if (pcb->snd_wnd != tcphdr->wnd) {
                LWIP_DEBUGF(TCP_WND_DEBUG, 
                        ("tcp_receive: no window update lastack %x ackno %x" 
                         " wl1 %u seqno %x wl2 %u\n",
                         pcb->u32LastAck, pcb->stInput.u32Acknowledge, pcb->u32SendWL1, pcb->stInput.u32Sequence, pcb->u32SendWL2));
            }
#endif /* TCP_WND_DEBUG */
        }
        
        if(pcb->u32LastAck == pcb->stInput.u32Acknowledge)
        {
            pcb->u16Acked = 0;
            
            if((pcb->u32SendWL2 + pcb->u32SendWindow) == rWindowEdge)
            {
                pcb->u08DupPacks++;
                if((pcb->u08DupPacks >= 3) && (pcb->pstUnacked != 0))
                {
                    if(!(pcb->u08Flag & TF_INFR))
                    {
                        MP_DEBUG("[TCP] rexmit");
                        tcpRexmit(pcb);
                        
                        if(pcb->u16CWindow > pcb->u32SendWindow)
                            pcb->u16SSThresh = pcb->u32SendWindow >> 1;
                        else
                            pcb->u16SSThresh = pcb->u16CWindow >> 1;
                            
                        pcb->u16CWindow = pcb->u16SSThresh + 3 * pcb->u16Mss;
                        pcb->u08Flag |= TF_INFR;
                    }
                    else
                    {
                        if(((U16)(pcb->u16CWindow + pcb->u16Mss)) > pcb->u16CWindow)
                            pcb->u16CWindow += pcb->u16Mss;
                    }
                }
            }
            else {
                LWIP_DEBUGF(TCP_FR_DEBUG, ("tcp_receive: dupack averted %u %u\n",
                            pcb->u32SendWL2 + pcb->u32SendWindow, rWindowEdge));
            }
        }
        else if (TCP_SEQ_BETWEEN(pcb->stInput.u32Acknowledge, pcb->u32LastAck+1, pcb->u32SendNext))
        {
            if(pcb->u08Flag & TF_INFR)
            {
                pcb->u08Flag &= ~TF_INFR;
                pcb->u16CWindow = pcb->u16SSThresh;
            }
            
            pcb->u08RetranNumber = 0;
            pcb->u16RetranTimeout = (pcb->s16Sa >> 3) + pcb->s16Sv;
            pcb->u16Acked = (long)pcb->stInput.u32Acknowledge - (long)pcb->u32LastAck;
            
            pcb->u16SendBuffer += pcb->u16Acked;
            if(pcb->u16SendBuffer >= (u16TcpSendBufferSize*3/4))
            {
                if (pcb->u16SockId > 0)
                mpx_EventSet(u08TcpEventId, 0x00000001 << pcb->u16SockId);
            }
            
            pcb->u08DupPacks = 0;
            pcb->u32LastAck = pcb->stInput.u32Acknowledge;
            
            if(pcb->eState >= E_TCP_STATE_ESTABLISHED)
            {
                if(pcb->u16CWindow < pcb->u16SSThresh)
                {
                    if(((U16)(pcb->u16CWindow + pcb->u16Mss)) > pcb->u16CWindow)
                        pcb->u16CWindow += pcb->u16Mss;
                }
                else
                {
                    U16 newCWindow = (pcb->u16CWindow + pcb->u16Mss * pcb->u16Mss / pcb->u16CWindow);
                    if(newCWindow > pcb->u16CWindow)
                        pcb->u16CWindow = newCWindow;
                }
            }
            LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcpPacketReceive: ACK for %x, unacked->seqno %x:%x\n",
                        pcb->stInput.u32Acknowledge,
                        pcb->pstUnacked != NULL?
                        (TCPH_SEQNO(TCP_HEADER(pcb->pstUnacked))): 0,
                        pcb->pstUnacked != NULL?
                        (TCPH_SEQNO(TCP_HEADER(pcb->pstUnacked))) + TCP_TCPLEN(pcb->pstUnacked): 0));
            LWIP_DEBUGF(TCP_DEBUG,
                    ("tcpPacketReceive:  %u <- %u.\n", pcb->u16LocalPort, pcb->u16PeerPort));
            
            while(1)
            {
                if(pcb->pstUnacked == 0)
                    break;
                    
                packet = (ST_NET_PACKET*)pcb->pstUnacked->packet;
                tcpHdr = NET_PACKET_TCP(packet);
                seqno = NetGetDW(&tcpHdr[TCP_SEQUENCE]);
                flag = (NetGetW(&tcpHdr[TCP_LENGTH_FLAG]) & 0x3f);
                tcpLen = pcb->pstUnacked->u16Length;
                if((flag & TCP_SYN) || (flag & TCP_FIN))
                    tcpLen++;
                
                if((seqno + tcpLen) > pcb->stInput.u32Acknowledge)
                {
                    if((seqno > pcb->stInput.u32Acknowledge) && ((seqno - pcb->stInput.u32Acknowledge) > 0x7fffffff))
                    {
                        if((seqno + tcpLen) > seqno)
                        {
                        }
                        else
                        {
                            U32 tempVal = seqno + tcpLen;
                            
                            if(tempVal > pcb->stInput.u32Acknowledge)
                            {
                                //DPrintf("1 s = %x, a = %x, t = %d", seqno, pcb->stInput.u32Acknowledge, tcpLen);
                                break;
                            }
                        }
                    }
                    else
                    {
                        //DPrintf("2 s = %x, a = %x, t = %d", seqno, pcb->stInput.u32Acknowledge, tcpLen);
                        break;
                    }
                }
                
                next = pcb->pstUnacked;
                pcb->pstUnacked = pcb->pstUnacked->next;
                
                /* Prevent ACK for FIN to generate a sent event */
                if ((pcb->u16Acked != 0) && ((flag & TCP_FIN) != 0)) {
                    pcb->u16Acked--;
                }

                pcb->u08SendQueueLen--;
                //DPrintf("[TCP] sock %d, queue len = %d", pcb->u16SockId, pcb->u08SendQueueLen);
                tcpFreeSegment(next);
            }
            
            /* If there's nothing left to acknowledge, stop the retransmit
               timer, otherwise reset it to start again */
            if(pcb->pstUnacked == NULL)
                pcb->u16RetranTime = -1;
            else
                pcb->u16RetranTime = 0;

            pcb->u08PollTimer = 0;
        }
        else {
            /* Fix bug bug #21582: out of sequence ACK, didn't really ack anything */
            pcb->u16Acked = 0;
        }
        
        while(1)
        {
            if(pcb->pstUnsent == 0)
                break;
            
            packet = (ST_NET_PACKET*)pcb->pstUnsent->packet;
            tcpHdr = NET_PACKET_TCP(packet);
            seqno = NetGetDW(&tcpHdr[TCP_SEQUENCE]);
            flag = (NetGetW(&tcpHdr[TCP_LENGTH_FLAG]) & 0x3f);
            tcpLen = pcb->pstUnsent->u16Length;
            if((flag & TCP_SYN) || (flag & TCP_FIN))
                tcpLen++;
                
            if((pcb->stInput.u32Acknowledge < (seqno + tcpLen)) || (pcb->stInput.u32Acknowledge > pcb->u32SendNext))
                break;
                
            next = pcb->pstUnsent;
            pcb->pstUnsent = pcb->pstUnsent->next;
            
            /* Prevent ACK for FIN to generate a sent event */
            if ((pcb->u16Acked != 0) && ((flag & TCP_FIN) != 0)) {
                pcb->u16Acked--;
            }
            pcb->u08SendQueueLen--;
            //DPrintf("[TCP] sock %d, queue len = %d", pcb->u16SockId, pcb->u08SendQueueLen);
            tcpFreeSegment(next);
        }
        
        /* RTT estimation calculations. This is done by checking if the
           incoming segment acknowledges the segment we use to take a
           round-trip time measurement. */
        if(pcb->u32RTTest && TCP_SEQ_LT(pcb->u32RTSeq, pcb->stInput.u32Acknowledge))
        {
            m = (S32)u32TcpTicks - (S32)pcb->u32RTTest;
            m = m - (pcb->s16Sa >> 3);
            pcb->s16Sa += m;
            if(m < 0)
                m = -m;
            
            m = m - (pcb->s16Sv >> 2);
            pcb->s16Sv += m;
            pcb->u16RetranTimeout = (pcb->s16Sa >> 3) + pcb->s16Sv;
            pcb->u32RTTest = 0;
        }
    }
    
    if(pcb->stInput.u16TcpLength > 0)
    {
        //DPrintf("[TCP] tcp input data length = %d", pcb->stInput.u16TcpLength);
        //DPrintf("[TCP] receive next = %x, input sequence = %x", pcb->u32RcvNext, pcb->stInput.u32Sequence);
        //DPrintf("seq %x", pcb->stInput.u32Sequence);
        
        //if((pcb->u32RcvNext > pcb->stInput.u32Sequence) && 
        //    ((pcb->u32RcvNext - pcb->stInput.u32Sequence) <= (pcb->stInput.u16TcpLength - 1)))
        if(TCP_SEQ_BETWEEN(pcb->u32RcvNext, pcb->stInput.u32Sequence+1, pcb->stInput.u32Sequence + pcb->stInput.u16TcpLength - 1))
        {
            U16 newDataLen;
            U08* tcp = NET_PACKET_TCP(pcb->stInput.packet);
            
            DPrintf("[TCP] notify!! defalte packet size");
            DPrintf("   rcvnext = %x, seq = %x, tcplen = %x", pcb->u32RcvNext, pcb->stInput.u32Sequence, pcb->stInput.u16TcpLength);
            
            off = (long)pcb->u32RcvNext - (long)pcb->stInput.u32Sequence;
            
            pcb->stInput.u16TcpLength -= off;
            pcb->stInput.u16DataLength -= off;
            tcpPacketDeflate((ST_NET_PACKET*)pcb->stInput.packet, (U16)off);
            
            pcb->stInput.u32Sequence = pcb->u32RcvNext;
            NetPutDW(&tcp[TCP_SEQUENCE], pcb->stInput.u32Sequence);
        }
        else
        {
            if (TCP_SEQ_LT(pcb->stInput.u32Sequence, pcb->u32RcvNext))
            {
                /* the whole segment is < rcv_nxt */
                /* must be a duplicate of a packet that has already been correctly handled */
                MP_DEBUG("[TCP] receive duplicate seqno: %x, receive next: %x", pcb->stInput.u32Sequence, pcb->u32RcvNext);
                LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: duplicate seqno %x\n", pcb->stInput.u32Sequence));
                tcpAckNow(pcb);
            }
        }
        
		/* The sequence number must be within the window (above rcv_nxt
		   and below rcv_nxt + rcv_wnd) in order to be further
		   processed. */
        //if(((pcb->stInput.u32Sequence >= pcb->u32RcvNext) && ((pcb->stInput.u32Sequence - pcb->u32RcvNext) <= (pcb->u16RcvWindow - 1))) ||
        //    ((pcb->stInput.u32Sequence < pcb->u32RcvNext) && ((pcb->u32RcvNext - pcb->stInput.u32Sequence) > 0x7fffffff) && ((pcb->stInput.u32Sequence + (0xffffffff - pcb->u32RcvNext)) <= (pcb->u16RcvWindow - 1))))
        if(TCP_SEQ_BETWEEN(pcb->stInput.u32Sequence, pcb->u32RcvNext, pcb->u32RcvNext + pcb->u16RcvWindow - 1))
        {
            if(pcb->u32RcvNext == pcb->stInput.u32Sequence)
            {
                accepted = TRUE;
                
				/* The incoming segment is the next in sequence. We check if
				   we have to trim the end of the segment and update rcv_nxt
				   and pass the data to the application. */
                tcplen = pcb->stInput.u16TcpLength;

                if (tcplen > pcb->u16RcvWindow) {
                    LWIP_DEBUGF(TCP_INPUT_DEBUG, 
                            ("tcp_receive: other end overran receive window"
                             "seqno %x len %u right edge %x\n",
                             pcb->stInput.u32Sequence, tcplen, pcb->u32RcvNext + pcb->u16RcvWindow));
                    if (pcb->stInput.u08Flag & TCP_FIN) {
                        /* Must remove the FIN from the header as we're trimming 
                         * that byte of sequence-space from the packet */
                        pcb->stInput.u08Flag &= ~TCP_FIN;
                    }
                    /* Adjust length of segment to fit in the window. */
                    off = pcb->stInput.u16DataLength;
                    pcb->stInput.u16DataLength = pcb->u16RcvWindow;
                    if (pcb->stInput.u08Flag & TCP_SYN) {
                        pcb->stInput.u16DataLength -= 1;
                    }
                    off -= pcb->stInput.u16DataLength;
                    tcpPacketDeflate2((ST_NET_PACKET*)pcb->stInput.packet, (U16)off);
                    tcplen = pcb->stInput.u16DataLength;
                    if((pcb->stInput.u08Flag & TCP_FIN) || (pcb->stInput.u08Flag & TCP_SYN))
                        tcplen += 1;
                    LWIP_ASSERT("tcp_receive: segment not trimmed correctly to rcv_wnd\n",
                            (pcb->stInput.u32Sequence + tcplen) == (pcb->u32RcvNext + pcb->u16RcvWindow));
                }
                pcb->stInput.u16TcpLength = tcplen;
#if TCP_QUEUE_OOSEQ
				if (pcb->pstOOSeq != NULL) 
				{
                    next = pcb->pstOOSeq;
                    packet = next->packet;
                    tcpHdr = NET_PACKET_TCP(packet);
                    seqno = NetGetDW(&tcpHdr[TCP_SEQUENCE]);
                    
                    if (pcb->stInput.u08Flag & TCP_FIN) {
                        LWIP_DEBUGF(TCP_INPUT_DEBUG, 
                                ("tcp_receive: received in-order FIN, binning ooseq queue\n"));
                        /* Received in-order FIN means anything that was received
                         * out of order must now have been received in-order, so
                         * bin the ooseq queue */
                        while (pcb->pstOOSeq != NULL) {
                            struct tcp_seg *old_ooseq = pcb->pstOOSeq;
                            pcb->pstOOSeq = pcb->pstOOSeq->next;
                            tcpFreeSegment(old_ooseq);
                        }               
                    } 
                    else {
                        struct tcp_seg *old_seg;
                        while (next &&
                                TCP_SEQ_GEQ(pcb->stInput.u32Sequence + pcb->stInput.u16TcpLength,
                                    seqno + next->u16Length)) {
                            /* inseg doesn't have FIN (already processed) */
                            if (TCPH_FLAGS(tcpHdr) & TCP_FIN &&
                                    (pcb->stInput.u08Flag & TCP_SYN) == 0) {
                                pcb->stInput.u08Flag |= TCP_FIN;
                                pcb->stInput.u16TcpLength++;
                            }
                            old_seg = next;
                            next = next->next;
                            tcpFreeSegment(old_seg);
                        }

                        if (next &&
                                TCP_SEQ_GT(pcb->stInput.u32Sequence + pcb->stInput.u16TcpLength,
                                    seqno)) {
                            /* FIN in inseg already handled by dropping whole ooseq queue */
                            off = pcb->stInput.u16DataLength;
                            pcb->stInput.u16DataLength = (u16_t)(seqno - pcb->stInput.u32Sequence);
                            if (pcb->stInput.u08Flag & TCP_SYN) {
                                pcb->stInput.u16DataLength -= 1;
                            }
                            //pbuf_realloc(inseg.p, inseg.len);
                            tcpPacketDeflate2((ST_NET_PACKET*)pcb->stInput.packet, off-pcb->stInput.u16DataLength);
                            pcb->stInput.u16TcpLength = pcb->stInput.u16DataLength;
                            if((pcb->stInput.u08Flag & TCP_FIN) || (pcb->stInput.u08Flag & TCP_SYN))
                                pcb->stInput.u16TcpLength += 1;
                            LWIP_ASSERT("tcp_receive: segment not trimmed correctly to ooseq queue\n",
                                    (pcb->stInput.u32Sequence + pcb->stInput.u16TcpLength) == seqno);
                        }
                        pcb->pstOOSeq = next;
					}
				}
#endif /* TCP_QUEUE_OOSEQ */
                pcb->u32RcvNext = pcb->stInput.u32Sequence + pcb->stInput.u16TcpLength;
                
                /* Update the receiver's (our) window. */
                LWIP_ASSERT("tcp_receive: tcplen > rcv_wnd\n", pcb->u16RcvWindow >= tcplen);
                pcb->u16RcvWindow -= tcplen;
                
                tcp_update_rcv_ann_wnd(pcb);
                
//                if(pcb->stInput.u16TcpLength > 0)
                if(pcb->stInput.u16DataLength > 0)
                {
                    pcb->stInput.u32RcvData = pcb->stInput.packet;
                    pcb->stInput.packet = 0;
                }
                
                if(pcb->stInput.u08Flag & TCP_FIN)
                {
                    pcb->stInput.u08RcvFlag |= TF_GOT_FIN;
                    LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: received FIN.\n"));
                    if (pcb->u16SockId > 0)
                        mpx_EventSet(u08TcpEventId, 0x00000001 << pcb->u16SockId);
                }
                
#if TCP_QUEUE_OOSEQ
				/* We now check if we have segments on the ->ooseq queue that
				   is now in sequence. */
                ST_TCP_SEG* concateSeg = pcb->pstConcateSeq;
                
                while(pcb->pstOOSeq != NULL)
                {
                    BOOL concate = FALSE;
                    
                    packet = pcb->pstOOSeq->packet;
                    tcpHdr = NET_PACKET_TCP(packet);
                    seqno = NetGetDW(&tcpHdr[TCP_SEQUENCE]);
                    
                    if(seqno != pcb->u32RcvNext)
                    {
                        break;
                    }
                    
                    cseg = pcb->pstOOSeq;
                    pcb->stInput.u32Sequence = seqno;
                    
                    flag = (NetGetW(&tcpHdr[TCP_LENGTH_FLAG]) & 0x3f);
                    tcpLen = cseg->u16Length;
                    if((flag & TCP_SYN) || (flag & TCP_FIN))
                        tcpLen++;
                    
                    pcb->u32RcvNext += tcpLen;
                    if(pcb->u16RcvWindow < tcpLen)
                        pcb->u16RcvWindow = 0;
                    else
                        pcb->u16RcvWindow -= tcpLen;
                        
                    tcp_update_rcv_ann_wnd(pcb);

                    if(tcpLen > 0)
                    {
                        if(pcb->stInput.u32RcvData)
                        {
                            #if 1
							MP_DEBUG("concate = TRUE");
                            concate = TRUE;
                            #else
                            DPrintf("[TCP OOSEQ] packet concatenate");
                            DPrintf("[TCP OOSEQ] origin size = %d, input size = %d", 
                                    ((ST_NET_PACKET*)pcb->stInput.u32RcvData)->Net.u16PayloadSize, 
                                    ((ST_NET_PACKET*)cseg->packet)->Net.u16PayloadSize);
                            tcpPacketConcatenate(&(pcb->stInput.u32RcvData), cseg->packet);
                            DPrintf("[TCP OOSEQ] new packet payload size = %d",
                                    ((ST_NET_PACKET*)pcb->stInput.u32RcvData)->Net.u16PayloadSize);
                            #endif
                        }
                        else
                        {
                            pcb->stInput.u32RcvData = cseg->packet;
                            cseg->packet = 0;
                        }
                    }
                    
                    if(flag & TCP_FIN)
                    {
                        pcb->stInput.u08RcvFlag |= TF_GOT_FIN;
                        if(pcb->eState == E_TCP_STATE_ESTABLISHED)
                        {
                            LWIP_DEBUGF(TCP_DEBUG, ("tcpPacketReceive: FIN rxed estab->close_wait."));
                            pcb->eState = E_TCP_STATE_CLOSE_WAIT;
                        }
                        if (pcb->u16SockId > 0)
                            mpx_EventSet(u08TcpEventId, 0x00000001 << pcb->u16SockId);
                    }
                    
                    pcb->pstOOSeq = cseg->next;
                    
                    if(concate)
                    {
                        cseg->next = 0;
                        
                        if(!pcb->pstConcateSeq)
                            pcb->pstConcateSeq = cseg;
                        else
                            concateSeg->next = cseg;
                        
                        pcb->concateLen++;
                        
                        concateSeg = cseg;
                    }
                    else
                    {
                        tcpFreeSegment(cseg);
                    }
                }
#endif
                
                //tcpAckNow(pcb);
                tcpAck(pcb);
            }
            else
            {
				/* We get here if the incoming segment is out-of-sequence. */
                //DPrintf("s = %x, rn = %x", pcb->stInput.u32Sequence, pcb->u32RcvNext);
                tcpAckNow(pcb);

#if TCP_QUEUE_OOSEQ
				/* We queue the segment on the ->ooseq queue. */
                if(pcb->pstOOSeq == NULL) {
                    MP_DEBUG("[TCP OOSEQ]");
                    pcb->pstOOSeq = tcpSegCopy(&(pcb->stInput.packet));
				} else {
					/* If the queue is not empty, we walk through the queue and
					   try to find a place where the sequence number of the
					   incoming segment is between the sequence numbers of the
					   previous and the next segment on the ->ooseq queue. That is
					   the place where we put the incoming segment. If needed, we
					   trim the second edges of the previous and the incoming
					   segment so that it will fit into the sequence.
					
					   If the incoming segment has the same sequence number as a
					   segment on the ->ooseq queue, we discard the segment that
					   contains less data. */
                    
                    prev = NULL;
                    for(next = pcb->pstOOSeq; next != 0; next = next->next)
                    {
                        U08 nextFlag;
                        U16 nextTcplen;
                        
                        tcpHdr = NET_PACKET_TCP(next->packet);
                        seqno = NetGetDW(&tcpHdr[TCP_SEQUENCE]);
                        nextFlag = (NetGetW(&tcpHdr[TCP_LENGTH_FLAG]) & 0x3f);
                        nextTcplen = next->u16Length;
                        if((nextFlag & TCP_SYN) || (nextFlag & TCP_FIN))
                            nextTcplen++;
                        
                        if(pcb->stInput.u32Sequence == seqno)
                        {
                            /* The sequence number of the incoming segment is the
                               same as the sequence number of the segment on
                               ->ooseq. We check the lengths to see which one to
                               discard. */
                            if(pcb->stInput.u16DataLength > next->u16Length)
                            {
                                /* The incoming segment is larger than the old
                                   segment. We replace some segments with the new
                                   one. */
                                cseg = tcpSegCopy(&(pcb->stInput.packet));
                                if(cseg)
                                {
                                    if(prev != NULL)
                                        prev->next = cseg;
                                    else
                                        pcb->pstOOSeq = cseg;
                                        
                                    tcp_oos_insert_segment(cseg, next);
                                }
                                break;
                            }
                            else {
                                /* Either the lenghts are the same or the incoming
                                   segment was smaller than the old one; in either
                                   case, we ditch the incoming segment. */
                                break;
                            }
                        }
                        else
                        {
                            if(prev == 0)
                            {
                                if(TCP_SEQ_LT(pcb->stInput.u32Sequence, seqno))
                                {
                                    /* The sequence number of the incoming segment is lower
                                       than the sequence number of the first segment on the
                                       queue. We put the incoming segment first on the
                                       queue. */
                                    cseg = tcpSegCopy(&(pcb->stInput.packet));
                                    if(cseg)
                                    {
                                        pcb->pstOOSeq = cseg;
                                        
                                        tcp_oos_insert_segment(cseg, next);
                                    }
                                    break;
                                }
                            }
                            else
                            {
                                U32 prevSeq;
                                U08 prevFlag;
                                U16 prevTcplen;
                                
                                tcpHdr = NET_PACKET_TCP(prev->packet);
                                prevSeq = NetGetDW(&tcpHdr[TCP_SEQUENCE]);
                                prevFlag = (NetGetW(&tcpHdr[TCP_LENGTH_FLAG]) & 0x3f);
                                prevTcplen  = prev->u16Length;
                                if((prevFlag & TCP_SYN) || (prevFlag & TCP_FIN))
                                    prevTcplen++;
                                
                                if (TCP_SEQ_BETWEEN(pcb->stInput.u32Sequence, prevSeq+1, seqno-1)) {
                                    /* The sequence number of the incoming segment is in
                                       between the sequence numbers of the previous and
                                       the next segment on ->ooseq. We trim trim the previous
                                       segment, delete next segments that included in received segment
                                       and trim received, if needed. */
                                    cseg = tcpSegCopy(&(pcb->stInput.packet));
                                    if(cseg)
                                    {
                                        if (TCP_SEQ_GT(prevSeq + prev->u16Length, pcb->stInput.u32Sequence)) {
                                            /* We need to trim the prev segment. */
                                            off = prev->u16Length;
                                            prev->u16Length = (u16_t)(pcb->stInput.u32Sequence - prevSeq);
                                            //pbuf_realloc(prev->p, prev->len);
                                            tcpPacketDeflate2((ST_NET_PACKET*)prev->packet, off-prev->u16Length);
                                        }
                                        prev->next = cseg;
                                        tcp_oos_insert_segment(cseg, next);
                                    }
                                    break;
                                }
                            }
                            
                            /* If the "next" segment is the last segment on the
                               ooseq queue, we add the incoming segment to the end
                               of the list. */
                            if(next->next == 0)
                            {
                                if(pcb->stInput.u32Sequence > seqno)
                                {
                                    tcpHdr = NET_PACKET_TCP(next->packet);
                                    if (TCPH_FLAGS(tcpHdr) & TCP_FIN) {
                                        /* segment "next" already contains all data */
                                        break;
                                    }
                                    next->next = tcpSegCopy(&(pcb->stInput.packet));
                                    if(next->next)
                                    {
                                        if (TCP_SEQ_GT(seqno + next->u16Length, pcb->stInput.u32Sequence)) {
                                            /* We need to trim the last segment. */
                                            off = next->u16Length;
                                            next->u16Length = (u16_t)(pcb->stInput.u32Sequence - seqno);
                                            //pbuf_realloc(next->p, next->len);
                                            tcpPacketDeflate2((ST_NET_PACKET*)next->packet, off-next->u16Length);
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        
                        prev = next;
                    }
                }
#endif /* TCP_QUEUE_OOSEQ */
            }
        }
        else
        {
            //if(!((pcb->stInput.u32Sequence >= pcb->u32RcvNext) &&
            //    ((pcb->stInput.u32Sequence - pcb->u32RcvNext) <= (pcb->u16RcvWindow - 1))))
                tcpAckNow(pcb);
        }
    }
    else
    {
        /* Segments with length 0 is taken care of here. Segments that
           fall out of the window are ACKed. */
        if(!TCP_SEQ_BETWEEN(pcb->stInput.u32Sequence, pcb->u32RcvNext, pcb->u32RcvNext + pcb->u16RcvWindow-1))
        {
            tcpAckNow(pcb);
        }
    }
    
    return accepted;
}

static void tcpRexmit(ST_TCP_PCB* pcb)
{
    struct tcp_seg *seg;
    struct tcp_seg **cur_seg;
    
    if(pcb->pstUnacked == 0)
        return;
    
    //move the first unacked segment to the unsent queue
    /* Keep the unsent queue sorted. */
    seg = pcb->pstUnacked;
    pcb->pstUnacked = seg->next;
    
    cur_seg = &(pcb->pstUnsent);
    while (*cur_seg &&
            TCP_SEQ_LT(NET_TCP_SEQNO(*cur_seg), NET_TCP_SEQNO(seg))) {
        cur_seg = &((*cur_seg)->next );
    }
    seg->next = *cur_seg;
    *cur_seg = seg;
    
    pcb->u08RetranNumber++;
    /* Don't take any RTT measurements after retransmitting. */
    pcb->u32RTTest = 0;
    
    tcpOutput(pcb);
}

static u8_t *
tcp_output_set_header(struct tcp_pcb *pcb, ST_NET_PACKET *pkt, int optlen,
                      u32_t seqno_be /* already in network byte order */)
{
  u8_t* tcp;
  u16_t lenFlag;
  tcp = NET_PACKET_TCP(pkt);

  NetPutW(&tcp[TCP_SRC_PORT], pcb->u16LocalPort);
  NetPutW(&tcp[TCP_DST_PORT], pcb->u16PeerPort);
  NetPutDW(&tcp[TCP_SEQUENCE], seqno_be);
  NetPutDW(&tcp[TCP_ACKNOWLEDGE], pcb->u32RcvNext);
  lenFlag = 0x5000 | TCP_ACK;
  NetPutW(&tcp[TCP_LENGTH_FLAG], lenFlag);
  NetPutW(&tcp[TCP_WINDOW], pcb->rcv_ann_wnd);
  NetPutW(&tcp[TCP_URGENT], 0);
  NetPutW(&tcp[TCP_CHECKSUM], 0);

  /* If we're sending a packet, update the announced right window edge */
  pcb->rcv_ann_right_edge = pcb->u32RcvNext + pcb->rcv_ann_wnd;

  return tcp;
}

S32 TcpWrite(ST_TCP_PCB* pcb, U08* data, S32 size)
{
    S32 error = NO_ERR;
    BOOL finish = FALSE;
    U16 segSize;
    U16 sendSize = 0;
    U16 sid = pcb->u16SockId;
    
//    DPrintf("%s len=%d", __FUNCTION__, size);
    do
    {
        if (!Conn(sid))
        {
            error = -ENOTCONN;
            break;
        }
        
        if (size > pcb->u16SendBuffer)
        {
            error = -ENOMEM;
                break;
            }
        
        TCP_ENTER_CRITICAL;
        
        segSize = size;
        
        if((pcb->eState == E_TCP_STATE_ESTABLISHED) || (pcb->eState == E_TCP_STATE_CLOSE_WAIT) ||
            (pcb->eState == E_TCP_STATE_SYN_SENT) || (pcb->eState == E_TCP_STATE_SYN_RCVD))
        {
            error = tcpEnqueue(pcb, 0, data, segSize, 0, 0);
            if(error == NO_ERR)
            {
                error = tcpOutput(pcb);
                if (error)
                {
                DPrintf("%s tcpOutput returns %d", __FUNCTION__, error);
                error = -EFAULT;
                }
                size -= segSize;
                data += segSize;
                sendSize += segSize;
            }
            else
            {
                size = 0;
                DPrintf("[TCP] tcpEnqueue return error");
                error = -EFAULT;
            }
        }
        else
        {
            error = -ENOTCONN;
            size = 0;
        }
        
        TCP_EXIT_CRITICAL;
    } while (0);
    
    if(error < 0)
        return error;
    else
        return sendSize;
}

static S32 tcpSendControl(ST_TCP_PCB* pcb, U08 flag)
{
    return tcpEnqueue(pcb, flag, 0, 0, 0, 0);
}

static void tcpSelfAbort(ST_TCP_PCB* pcb, int reset)
{
    U32 seqno, ackno;

    MP_DEBUG("[TCP] tcpSelfAbort: st=%d", pcb->eState);
    if(pcb->eState == E_TCP_STATE_TIME_WAIT)
    {
        tcpPcbRemove(&tcpTimeWaitPcb, pcb);
        TcpFree(pcb);
    }
    else
    {
        seqno = pcb->u32SendNext;
        ackno = pcb->u32RcvNext;
        
        tcpPcbRemove(&tcpActivePcb, pcb);
        if(pcb->pstUnacked)
        {
            tcpSegmentsFree(pcb->pstUnacked);
            pcb->pstUnacked = 0;
        }
        if(pcb->pstUnsent)
        {
            tcpSegmentsFree(pcb->pstUnsent);
            pcb->pstUnsent = 0;
        }
        
#if TCP_QUEUE_OOSEQ
        if(pcb->pstOOSeq)
        {
            tcpSegmentsFree(pcb->pstOOSeq);
            pcb->pstOOSeq = NULL;
        }
#endif
        
        if (reset) {
            tcpReset(seqno, ackno, pcb->u32LocalIp, pcb->u32PeerIp, pcb->u16LocalPort, pcb->u16PeerPort);
        }
        TcpFree(pcb);
    }
}

void TcpAbort(ST_TCP_PCB* pcb)
{
    TCP_ENTER_CRITICAL;
    tcpSelfAbort(pcb, 1);
    TCP_EXIT_CRITICAL;
}

/**
 * Abandons a connection and optionally sends a RST to the remote
 * host.  Deletes the local protocol control block. This is done when
 * a connection is killed because of shortage of memory.
 *
 * @param pcb the tcp_pcb to abort
 * @param reset boolean to indicate whether a reset should be sent
 */
void tcp_abandon(ST_TCP_PCB* pcb, int reset)
{
    tcpSelfAbort(pcb, reset);
}

/** 
 * Update the state that tracks the available window space to advertise.
 *
 * Returns how much extra window would be advertised if we sent an
 * update now.
 */
u32_t tcp_update_rcv_ann_wnd(struct tcp_pcb *pcb)
{
  u32_t new_right_edge = pcb->u32RcvNext + pcb->u16RcvWindow;

  if (TCP_SEQ_GEQ(new_right_edge, pcb->rcv_ann_right_edge + pcb->u16Mss)) {
    /* we can advertise more window */
    pcb->rcv_ann_wnd = pcb->u16RcvWindow;
//    mpDebugPrint("rcv_ann_wnd=%u", pcb->rcv_ann_wnd);
    return new_right_edge - pcb->rcv_ann_right_edge;
  } else {
    if (TCP_SEQ_GT(pcb->u32RcvNext, pcb->rcv_ann_right_edge)) {
      /* Can happen due to other end sending out of advertised window,
       * but within actual available (but not yet advertised) window */
      MP_ASSERT(0);
      pcb->rcv_ann_wnd = 0;
    } else {
      /* keep the right edge of window constant */
      pcb->rcv_ann_wnd = pcb->rcv_ann_right_edge - pcb->u32RcvNext;
    }
//    mpDebugPrint("rcv_ann_wnd=%u", pcb->rcv_ann_wnd);
    return 0;
  }
}

/**
 * This function should be called by the application when it has
 * processed the data. The purpose is to advertise a larger window
 * when the data has been processed.
 *
 * @param pcb the tcp_pcb for which data is read
 * @param length the amount of bytes that have been read by the application
 */
void tcpReceived(ST_TCP_PCB* pcb, U16 length)
{
    int wnd_inflation;

    LWIP_ASSERT("tcp_recved: len would wrap rcv_wnd\n",
            length <= 0xffff - pcb->u16RcvWindow );

#ifndef HAVE_RTSP
    TCP_ENTER_CRITICAL;
#endif
    
        pcb->u16RcvWindow += length;
    if(((U32)pcb->u16RcvWindow) > u16TcpWindowSize)
    {
        pcb->u16RcvWindow = u16TcpWindowSize;
    }
    
    wnd_inflation = tcp_update_rcv_ann_wnd(pcb);

    /* If the change in the right edge of window is significant (default
     * watermark is TCP_WND/2), then send an explicit update now.
     * Otherwise wait for a packet to be sent in the normal course of
     * events (or more window to be available later) */
    if (wnd_inflation >= TCP_WND_UPDATE_THRESHOLD) 
        tcpAckNow(pcb);
    
#ifndef HAVE_RTSP
    TCP_EXIT_CRITICAL;
#endif

//    LWIP_DEBUGF(TCP_DEBUG, ("tcp_recved: recveived %u bytes, wnd %u (%u).\n",
//                length, pcb->u16RcvWindow, TCP_WND - pcb->u16RcvWindow));
}

void TcpReceived(ST_TCP_PCB* pcb, U16 length, int s)
{
//    U32 message[4];
    U32 message[5];
    
    message[0] = NETCTRL_MSG_TCP_RECEIVED;
    message[1] = (U32)tcpReceived;
    message[2] = (U32)pcb;
    message[3] = (U32)length;
    message[4] = (U32)s;
    mpx_MessageSend(NetCtrlMessageIdGet(), message, sizeof(message));
}

static void tcpPacketDeflate(ST_NET_PACKET* packet, U16 size)
{
    U08* tcp = NET_PACKET_TCP(packet);
    U08* tcpData = NET_PACKET_TCP_DATA(packet);
    U16 hdrLength = ((NetGetW(&tcp[TCP_LENGTH_FLAG]) >> 12) & 0xf) << 2;
    U16 dataLength = packet->Net.u16PayloadSize - hdrLength;
    U16 i;
    U08 *pDest, *pSrc;
    short len;
    
    pDest = tcpData;
    pSrc = tcpData + size;
    len = dataLength - size;
    for(i = 0; i < len; i++)
        *pDest++ = *pSrc++;
    //memcpy(tcpData, tcpData+size, dataLength - size);
    packet->Net.u16PayloadSize -= size;
}

/* 
 * trim the 2nd edge of segment
 */
static void tcpPacketDeflate2(ST_NET_PACKET* packet, U16 size)
{
    packet->Net.u16PayloadSize -= size;
}

static S32 tcpTimeWaitInput(ST_TCP_PCB* pcb)
{
    DPrintf("[TCP] timewait inupt");
    if((pcb->stInput.u32Sequence + pcb->stInput.u16TcpLength) > pcb->u32RcvNext)
    {
        pcb->u32RcvNext = pcb->stInput.u32Sequence + pcb->stInput.u16TcpLength;
    }
    
    if(pcb->stInput.u16TcpLength > 0)
        tcpAckNow(pcb);
        
    return tcpOutput(pcb);
}

void tcpRexmitRto(ST_TCP_PCB* pcb)
{
    ST_TCP_SEG* segment;
    
    if(pcb->pstUnacked == 0)
        return;
    
    for(segment = pcb->pstUnacked; segment->next != 0; segment = segment->next);
    
    segment->next = pcb->pstUnsent;
    pcb->pstUnsent = pcb->pstUnacked;
    pcb->pstUnacked = 0;
    
    
    pcb->u08RetranNumber++;
    /* Don't take any RTT measurements after retransmitting. */
    pcb->u32RTTest = 0;
    
    tcpOutput(pcb);
}

S32 TcpListen(ST_TCP_PCB* pcb)
{
    if(!pcb)
        return -1;
    
    TCP_ENTER_CRITICAL;
    
    if(pcb->eState == E_TCP_STATE_LISTEN)
    {
        TCP_EXIT_CRITICAL;
        return NO_ERR;
    }
    
    tcpPcbRemove(&tcpBoundPcb, pcb);
    pcb->eState = E_TCP_STATE_LISTEN;
    tcpPcbRegister(&tcpListenPcb, pcb);
    
    TCP_EXIT_CRITICAL;
    
    return NO_ERR;
}

static S32 tcpListenInput(ST_TCP_PCB* pcb, U32 destAddr)
{
    S32 rc;
    if(pcb->stInput.u08Flag & TCP_ACK)
    {
        tcpReset(pcb->stInput.u32Acknowledge+1, pcb->stInput.u32Sequence + pcb->stInput.u16TcpLength,
                    pcb->u32LocalIp, pcb->stInput.u32IpAddr, pcb->u16LocalPort, pcb->stInput.u16Port); 
    }
    else if(pcb->stInput.u08Flag & TCP_SYN)
    {
        ST_TCP_PCB* newPcb = TcpNew();
        U32 optionData;
        
        if(!newPcb)
        {
            DPrintf("[TCP] tcpListenInput allocate buffer fail");
            return -1;
        }
        
        //DPrintf("[TCP] incoming connection...");
        
#ifdef NEW_NETSOCKET
        if (pcb->u32LocalIp == 0)
            newPcb->u32LocalIp = destAddr;
        else
#endif
        newPcb->u32LocalIp = pcb->u32LocalIp;
        newPcb->u32PeerIp = pcb->stInput.u32IpAddr;
        newPcb->u16LocalPort = pcb->u16LocalPort;
        newPcb->u16PeerPort = pcb->stInput.u16Port;
        newPcb->eState = E_TCP_STATE_SYN_RCVD;
        newPcb->u32RcvNext = pcb->stInput.u32Sequence + 1;
        newPcb->rcv_ann_right_edge = newPcb->u32RcvNext;
        newPcb->u32SendWindow = pcb->stInput.u16Window;
        newPcb->u16SSThresh = newPcb->u32SendWindow;
        newPcb->u32SendWL1 = pcb->stInput.u32Sequence - 1;
        
        //newPcb->u16SockId = pcb->u16SockId;
        newPcb->u16ListenSockId = pcb->u16SockId;
        newPcb->u16SockId = 0;
        newPcb->accept = pcb->accept;
        
        tcpPcbRegister(&tcpActivePcb, newPcb);
        
        if(pcb->stInput.u08HdrLength > sizeof(ST_TCP_HEADER))
            tcpParseOption(newPcb, NET_PACKET_TCP_DATA(pcb->stInput.packet), pcb->stInput.u08HdrLength - sizeof(ST_TCP_HEADER));
    
        optionData =  TCP_BUILD_MSS_OPTION();
        
        rc = tcpEnqueue(newPcb, TCP_SYN | TCP_ACK, 0, 0, (U08*)&optionData, 4);
        if (rc != ERR_OK) {
            tcp_abandon(newPcb, 0);
            return rc;
        }
        return tcpOutput(newPcb);
    }
    
    return NO_ERR;
}

void TcpAcceptCallbackSet(ST_TCP_PCB* pcb, TCP_ACCEPT_CALLBACK_PROC acceptProc)
{
    if(pcb && acceptProc)
    {
        pcb->accept = acceptProc;
    }
}

/* tcp lock should be held before calling this function */
S32 _TcpAccept(ST_TCP_PCB* pcb, U16 u16SockId)
{
    U16 i;
    ST_QUEUE* packetQueue = (ST_QUEUE*)(pcb->u32PacketQueue);
    
    pcb->u16SockId = u16SockId;
    pcb->u16ListenSockId = 0;
    
    if(packetQueue)
    {
        U32 queueLen = mpx_QueueLengthGet(packetQueue);
        
        for(i = 0; i < queueLen; i++)
        {
            U32 packet = 0;
            
            mpx_QueuePop(packetQueue, &packet);
#ifdef HAVE_LWIP_SOCKET
            if (u16SockId >= 0x100)
            {
                DPrintf("[TCP] TcpAccept: calls SockIdSignalRxDone3");
                SockIdSignalRxDone3(pcb->u16SockId, packet);
            }
            else
#endif
            SockIdSignalRxDone(pcb->u16SockId, packet);
        }
        
        mpx_QueueDelete(packetQueue);
        pcb->u32PacketQueue = 0;
    }
    
    
    
    return NO_ERR;
}
S32 TcpAccept(ST_TCP_PCB* pcb, U16 u16SockId)
{
    U16 i;
    ST_QUEUE* packetQueue = (ST_QUEUE*)(pcb->u32PacketQueue);
    
    TCP_ENTER_CRITICAL;
    
    pcb->u16SockId = u16SockId;
    pcb->u16ListenSockId = 0;
    
    if(packetQueue)
    {
        U32 queueLen = mpx_QueueLengthGet(packetQueue);
        
        for(i = 0; i < queueLen; i++)
        {
            U32 packet = 0;
            
            mpx_QueuePop(packetQueue, &packet);
#ifdef HAVE_LWIP_SOCKET
            if (u16SockId >= 0x100)
            {
                //DPrintf("[TCP] TcpAccept: calls SockIdSignalRxDone3");
                SockIdSignalRxDone3(pcb->u16SockId, packet);
            }
            else
#endif
            SockIdSignalRxDone(pcb->u16SockId, packet);
        }
        
        mpx_QueueDelete(packetQueue);
        pcb->u32PacketQueue = 0;
    }
    
    
    TCP_EXIT_CRITICAL;
    
    return NO_ERR;
}

#if 0
static S32 tcpPacketConcatenate(ST_NET_PACKET** headPacket, ST_NET_PACKET* tailPacket)
{
    ST_NET_PACKET* newPacket = 0;
    U32 newPacketSize = 0;
    U16 tailDataLen = 0;
    U08* tcpHdr = NET_PACKET_TCP(tailPacket);
    U08 tailHdrLen = ((NetGetW(&tcpHdr[TCP_LENGTH_FLAG]) >> 12) & 0xf) << 2;
    U08 tailFlag = (NetGetW(&tcpHdr[TCP_LENGTH_FLAG]) & 0x3f);
    
    tailDataLen = tailPacket->Net.u16PayloadSize - tailHdrLen;
    newPacketSize = (**headPacket).Net.u16PayloadSize + tailDataLen;
    
    newPacket = mpx_Malloc(newPacketSize);
    if(!newPacket)
        return -1;
        
    memcpy(NET_PACKET_TCP(newPacket), NET_PACKET_TCP(*headPacket), (**headPacket).Net.u16PayloadSize);
    memcpy(NET_PACKET_TCP(newPacket) + (**headPacket).Net.u16PayloadSize, tcpHdr + tailHdrLen, tailDataLen);
    newPacket->Net.u16PayloadSize += tailDataLen;
    
    if(tailFlag & TCP_FIN)
    {
        U16 lenFlag;
        
        tcpHdr = NET_PACKET_TCP(newPacket);
        lenFlag = NetGetW(&tcpHdr[TCP_LENGTH_FLAG]);
        lenFlag |= TCP_FIN;
        NetPutW(&tcpHdr[TCP_LENGTH_FLAG], lenFlag);
    }
    
    NetFreePacket(*headPacket);
    *headPacket = newPacket;
    
    return NO_ERR;
}
#endif

#if TCP_QUEUE_OOSEQ
static ST_TCP_SEG* tcpSegCopy(ST_NET_PACKET** packet)
{
    ST_TCP_SEG* newSeg = mpx_Malloc(sizeof(ST_TCP_SEG));
    
    if(newSeg)
    {
        U08* tcp = NET_PACKET_TCP(*packet);
        U16 hdrLength = ((NetGetW(&tcp[TCP_LENGTH_FLAG]) >> 12) & 0xf) << 2;
        
        newSeg->next = 0;
        newSeg->packet = *packet;
        newSeg->u16Length = (*packet)->Net.u16PayloadSize - hdrLength;
        
        *packet = 0;
    }
    
    return newSeg;
}
#endif


void TcpConfigWindowSizeSet(U16 size)
{
    u16TcpWindowSize = size;
}

void TcpConfigSendBufferSizeSet(U16 size)
{
    u16TcpSendBufferSize = size;
}

void TcpConfigSendQueueLenSet(U08 size)
{
    u08TcpSendQueueLen = size;
}

static void tcpSendReset(ST_TCP_PCB* pcb, ST_NET_PACKET *packet, U32 srcAddr, U32 destAddr)
{
    U32 seqno, ackno;
    U16 tcplen;
    U08* tcp = NET_PACKET_TCP(packet);
    U16 flags = TCPH_FLAGS(tcp);
    U16 srcPort = NetGetW(&tcp[TCP_SRC_PORT]);
    U16 dstPort = NetGetW(&tcp[TCP_DST_PORT]);

    if (!(flags & TCP_RST)) {
        //TCP_STATS_INC(tcp.proterr);
        //TCP_STATS_INC(tcp.drop);
        seqno = NetGetDW(&tcp[TCP_SEQUENCE]);
        ackno = NetGetDW(&tcp[TCP_ACKNOWLEDGE]);
        tcplen = packet->Net.u16PayloadSize - (((NetGetW(&tcp[TCP_LENGTH_FLAG]) >> 12) & 0xf) << 2);
        if((flags & TCP_FIN) || (flags & TCP_SYN))
            tcplen++;
        tcpReset(ackno, seqno + tcplen,
                destAddr, srcAddr,
                dstPort, srcPort);
    }
}

void tcpCloseAll(void)
{
    u32TcpOff = 2 * TCP_MSL / TCP_SLOW_INTERVAL * 2;
}

void TcpLock(void)
{
    TCP_ENTER_CRITICAL;
}

void TcpUnlock(void)
{
    TCP_EXIT_CRITICAL;
}

#ifdef HAVE_RTSP
extern int m_rtsp_client_out_queue_len(void);
int tcp_writable(ST_TCP_PCB *pcb)
{
    if (!pcb)
        return 0;

    return ( (pcb->u16SendBuffer > TCP_SNDLOWAT) &&  (pcb->u08SendQueueLen < TCP_SNDQUEUELOWAT)) ? 1 : 0;
}

int tcp_writable2(ST_TCP_PCB *pcb)
{
    if (!pcb)
        return 0;

    return ( (pcb->u16SendBuffer > TCP_SNDLOWAT) &&  ((pcb->u08SendQueueLen + m_rtsp_client_out_queue_len()) < TCP_SNDQUEUELOWAT)) ? 1 : 0;
}

int tcp_writable3(ST_TCP_PCB *pcb)
{
    if (!pcb)
        return 0;

    return ( (pcb->u16SendBuffer > TCP_MSS) &&  (pcb->u08SendQueueLen < 48)) ? 1 : 0;
}

int tcp_writable4(void)
{
    return ((m_rtsp_client_out_queue_len()) < 64) ? 1 : 0;
//    return ((m_rtsp_client_out_queue_len()) < 128) ? 1 : 0;
}

int m_rtsp_tcp_send_queue_length(void)
{
    TcpConfigSendQueueLenSet(TCP_SND_QUEUELEN);

    return TCP_SND_QUEUELEN;
}

/* ------------- for received TCP packet ------------ */

#include "taskid.h"

static struct sk_buff_head tcp_recv_queue;
static struct sk_buff_head tcp_xmit_queue;
static BYTE tcp_recv_lock;

extern int g_rtsp_sd;
extern ST_TCP_PCB *m_socket_tcp_pcb(int s);
extern void *net_buf_start;
extern void *net_buf_end;
extern void *net_buf_last;                             /* the last buffer */

static S32 tcpEnqueue2(ST_TCP_PCB* pcb, U08 u08Flag, U08* pu08Arg, U16 u16ArgLen, U08* pu08OptData, U08 u08OptLen)
{
    U32 left = 0, seqno;
    U08* pArgData = pu08Arg;
    U16 u16SegLength = 0;
    S32 error = NO_ERR;
    U08 queueLen;
    ST_TCP_SEG *segQueue = 0, *segment = 0, *segPtr = 0;
    BOOL from_netpool = FALSE;
    
    if(pu08Arg)
    {
//        MP_ASSERT( (DWORD)pu08OptData >= (DWORD)net_buf_start && (DWORD)pu08OptData <= (DWORD)net_buf_last);
        if ( (DWORD)pu08Arg >= (DWORD)net_buf_start && (DWORD)pu08Arg <= (DWORD)net_buf_last)
            from_netpool = TRUE;
    }

    if(u16ArgLen > pcb->u16SendBuffer)
    {
        MP_ASSERT(0);
        error = -1;
        DPrintf("[TCP] data length overflow");
        goto ERROR;
    }
    
    left = u16ArgLen;
    seqno = pcb->u32SendLBB;
    
    queueLen = pcb->u08SendQueueLen;
    if(queueLen >= u08TcpSendQueueLen)
    {
        DPrintf("[TCP] queue full, queueLen = %d", queueLen);
        DPrintf("[TCP] send queue len = %d", u08TcpSendQueueLen);
        error = -1;
        goto ERROR;
    }
    
    segQueue = segment = segPtr = 0;
    u16SegLength = 0;
    
    while((!segQueue) || (left > 0))
    {
        U16 lenFlag = u08Flag;
        ST_NET_PACKET* tcpPacket;
        U08 *tcpData, *tcpHdr;
        
        u16SegLength = left > pcb->u16Mss ? pcb->u16Mss : left;
        
        segment = tcpNewSegment();
        if(!segment)
        {
            DPrintf("[TCP] tcp allocate segment buffer fail");
            error = -1;
            goto ERROR;
        }
        
        if(!segQueue)
            segQueue = segment; //this segment is the head
        else
            segPtr->next = segment;
            
        segPtr = segment;
        
        tcpPacket = (ST_NET_PACKET*)segment->packet;
#if 0
#else
        if (from_netpool)
        {
            net_buf_mem_free(tcpPacket->Net.head);
//            MP_ASSERT(0);
            tcpPacket->Net.head = pu08Arg - 128;
            tcpPacket->Net.data = pu08Arg - 54;
            //            tcpPacket->Net.tail = XXX;
            tcpPacket->Net.end = pu08Arg+u16ArgLen;
        }
#endif
        tcpHdr = NET_PACKET_TCP(tcpPacket);
        tcpData = NET_PACKET_TCP_DATA(tcpPacket);
        tcpPacket->Net.u16PayloadSize = 0;
        
        if(pu08OptData)
        {
            lenFlag |= (((5 + (u08OptLen >> 2)) & 0xf) << 12);

            mmcp_memcpy(tcpData, pu08OptData, u08OptLen);
            tcpPacket->Net.u16PayloadSize = u08OptLen;
            
            queueLen++;
        }
        else
        {
            lenFlag |= (5 << 12);
            
            if(pu08Arg)
            {
            
#if 1
                if (!from_netpool)
#endif
					mmcp_memcpy(tcpData, pArgData, u16SegLength);
            }
            tcpPacket->Net.u16PayloadSize = u16SegLength;
            
            queueLen++;
        }
        
        segment->u16Length = u16SegLength;
        tcpPacket->Net.u16PayloadSize += sizeof(ST_TCP_HEADER);
        
        //build tcp header
        NetPutW(&tcpHdr[TCP_SRC_PORT], pcb->u16LocalPort);
        NetPutW(&tcpHdr[TCP_DST_PORT], pcb->u16PeerPort);
        NetPutDW(&tcpHdr[TCP_SEQUENCE], seqno);
        //leave the 3 fields until later
        //tcpHdr[TCP_ACKNOWLEDGE]
        //tcpHdr[TCP_WINDOW]
        //tcpHdr[TCP_CHECKSUM]
        NetPutW(&tcpHdr[TCP_LENGTH_FLAG], lenFlag);
        NetPutW(&tcpHdr[TCP_URGENT], 0);
        
        left -= u16SegLength;
        seqno += u16SegLength;
        pArgData += u16SegLength;
    }
    
    if(!pcb->pstUnsent)
    {
        segPtr = 0;
    }
    else
    {
        for(segPtr = pcb->pstUnsent; segPtr->next != 0; segPtr = segPtr->next); //find the tail of the queue
    }
    
    {
        if(segPtr)
            segPtr->next = segQueue;
        else
            pcb->pstUnsent = segQueue;
        
        if((u08Flag & TCP_SYN) || (u08Flag & TCP_FIN))
            u16ArgLen++;
        
        pcb->u32SendLBB += u16ArgLen;
        pcb->u16SendBuffer -= u16ArgLen;
        if(pcb->u16SendBuffer == 0)
        {
            mpx_EventClear(u08TcpEventId, ~(0x00000001 << pcb->u16SockId));
        }
        
        MP_ASSERT(queueLen <= u08TcpSendQueueLen);
        pcb->u08SendQueueLen = queueLen;
        
        if(segment && (u16SegLength > 0))
        {
            U08* tcpHdr = NET_PACKET_TCP(segment->packet);
            U16 flagLen = NetGetW(&tcpHdr[TCP_LENGTH_FLAG]);
            
            flagLen |= TCP_PUSH;
            NetPutW(&tcpHdr[TCP_LENGTH_FLAG], flagLen);
        }
    }

ERROR:
    if(error)
    {
        MP_ASSERT(0);
#if 0
        DPrintf("[TCP] segment queue release");
        for(segPtr = segQueue; segPtr != 0; segPtr = segPtr->next)
            tcpFreeSegment(segPtr);
#endif
    }
    
    return error;
}


/*
 * Don't call tcpOutput here
 */
S32 TcpWrite2(ST_TCP_PCB* pcb, U08* data, S32 size)
{
    S32 error = NO_ERR;
    BOOL finish = FALSE;
    U16 segSize;
    U16 sendSize = 0;
    U16 sid = pcb->u16SockId;
    int d1, d2, d3;
    
//    DPrintf("%s len=%d", __FUNCTION__, size);
    do
    {
        if (!Conn(sid))
        {
            error = -ENOTCONN;
            break;
        }
        
        if (size > pcb->u16SendBuffer)
            {
        MP_ASSERT(0);
            error = -ENOMEM;
                break;
            }
        
        d1 = GetSysTime();
#if 1
#if 0
        int r = SemaphorePolling(u08TcpSemaphoreId);
        if (r < 0)
        {
            MP_ASSERT(0);
            TCP_ENTER_CRITICAL;
        }
#else
        TCP_ENTER_CRITICAL;
#endif
#else
        SemaphoreWait(pcb->u08KeepCounter);
#endif
        d3 = GetSysTime();
        
        segSize = size;
        
        if((pcb->eState == E_TCP_STATE_ESTABLISHED) || (pcb->eState == E_TCP_STATE_CLOSE_WAIT) ||
            (pcb->eState == E_TCP_STATE_SYN_SENT) || (pcb->eState == E_TCP_STATE_SYN_RCVD))
        {
            error = tcpEnqueue2(pcb, 0, data, segSize, 0, 0);
            if(error == NO_ERR)
            {
#if 1
                error = tcpOutput(pcb);
                if (error)
                {
                DPrintf("%s tcpOutput returns %d", __FUNCTION__, error);
                error = -EFAULT;
                }
#endif
                size -= segSize;
                data += segSize;
                sendSize += segSize;
            }
            else
            {
                size = 0;
                DPrintf("[TCP] tcpEnqueue return error");
                error = -EFAULT;
            }
        }
        else
        {
            error = -ENOTCONN;
            size = 0;
        }
        
        d2 = GetSysTime();
#if 1
        TCP_EXIT_CRITICAL;
#else
        SemaphoreRelease(pcb->u08KeepCounter);
#endif
    } while (0);
    
    if(error < 0)
        return error;
    else
    {
        EventSet(tcp_recv_event_id, 2);
        return sendSize;
    }
}

S32 TcpWrite3(ST_TCP_PCB* pcb, U08* data, S32 size)
{
    S32 error = NO_ERR;
    BOOL finish = FALSE;
    U16 segSize;
    U16 sendSize = 0;
    U16 sid = pcb->u16SockId;
    int d1, d2;
    
//    DPrintf("%s len=%d", __FUNCTION__, size);
    do
    {
        if (!Conn(sid))
        {
            error = -ENOTCONN;
            break;
        }
        
        if (size > pcb->u16SendBuffer)
            {
        MP_ASSERT(0);
            error = -ENOMEM;
                break;
            }
        
        d1 = GetSysTime();
        TCP_ENTER_CRITICAL;
        
        segSize = size;
        
        if((pcb->eState == E_TCP_STATE_ESTABLISHED) || (pcb->eState == E_TCP_STATE_CLOSE_WAIT) ||
            (pcb->eState == E_TCP_STATE_SYN_SENT) || (pcb->eState == E_TCP_STATE_SYN_RCVD))
        {
            error = tcpEnqueue2(pcb, 0, data, segSize, 0, 0);
            if(error == NO_ERR)
            {
                size -= segSize;
                data += segSize;
                sendSize += segSize;
            }
            else
            {
                size = 0;
                DPrintf("[TCP] tcpEnqueue return error");
                error = -EFAULT;
            }
        }
        else
        {
            error = -ENOTCONN;
            size = 0;
        }
        
        d2 = GetSysTime();
        TCP_EXIT_CRITICAL;
    } while (0);
    
    if(error < 0)
        return error;
    else
    {
        EventSet(tcp_recv_event_id, 2);
        return sendSize;
    }
}

void tcp_queue_recv(struct sk_buff *skb)
{
    skb_queue_tail(&tcp_recv_queue, skb);
    EventSet(tcp_recv_event_id, 1);
}

void tcp_dequeue_recv(void)
{
    struct sk_buff *skb;
    skb = skb_dequeue(&tcp_recv_queue);
}

static void tcp_recv_task(void)
{
    struct sk_buff *skb;
    uint32_t evts;
    int r;
    int cnt;

    while (1) {

        EventWait(tcp_recv_event_id, 0xff, OS_EVENT_OR, &evts);

        if (evts & 2)
        {
            /* handle transmit */

            if (g_rtsp_sd)
            {
                ST_TCP_PCB *pcb = m_socket_tcp_pcb(g_rtsp_sd);
                if (pcb)
                {
                    TCP_ENTER_CRITICAL;
                    r = tcpOutput(pcb);
                    TCP_EXIT_CRITICAL;
                    if (r)
                        MP_ASSERT(0);
                }
            }
        }

        if (evts & 4)
        {
            /* handle transmit */

            if (g_rtsp_sd)
            {
                rtsp_tcp_write_cb2();
            }
        }

        if (evts & 1)
        {
            /* handle receive */

            cnt = 0;
            while ((skb = skb_dequeue(&tcp_recv_queue))) {
                //        MP_ASSERT(0);

                cnt++;
                IpPacketReceive((ST_NET_PACKET *)skb, false, &NicArray[NIC_INDEX_WIFI]);
            }

            if (cnt > 0)
            {
                if (g_rtsp_sd)
                {
                    rtsp_tcp_write_cb2();
                }
            }
        }

        if (evts & 8)
        {
            /* handle fast timer */

            if(!boolTcpFastTimer)
            {
                boolTcpFastTimer = TRUE;
                TCPFastTimerProc();
            }
        }

        if (evts & 0x10)
        {
            /* handle slow timer */

            if(!boolTcpSlowTimer)
            {
                boolTcpSlowTimer = TRUE;
                TcpSlowTimerProc();
            }
        }

    }

}

void tcp_recv_init(void)
{
    int ret;

#if 1
    ret = mpx_TaskCreate(tcp_recv_task, DRIVER_PRIORITY+2, 0x4000);
//    ret = mpx_TaskCreate(tcp_recv_task, DRIVER_PRIORITY+3, 0x4000);
#else
    ret = mpx_TaskCreate(tcp_recv_task, DRIVER_PRIORITY+1, 0x4000);
#endif
    MP_ASSERT(ret > 0);
    tcp_recv_task_id = ret;

    ret = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_SINGLE|OS_ATTR_EVENT_CLEAR, 0);
    MP_ASSERT(ret > 0);
    tcp_recv_event_id = ret;

    ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
    MP_ASSERT(ret > 0);
    tcp_recv_queue.lock =
    tcp_recv_lock = ret;

	skb_queue_head_init(&tcp_recv_queue);

    TaskStartup((BYTE)tcp_recv_task_id);
}

#endif
