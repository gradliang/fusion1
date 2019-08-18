#define LOCAL_DEBUG_ENABLE 0

#include "linux/types.h"
#include "global612.h"
#include "mpTrace.h"
#include <string.h>
#include <sys/time.h>
#include "typedef.h"
#include "socket.h"
#include "net_packet.h"
#include "net_socket.h"
#include "net_ns.h"
#include "net_device.h"
#include "net_netdb.h"
#include "os.h"
#include "ndebug.h"
#include "net_nic.h"


#define	MAXADDRS	35

int h_errno;

void HEXDUMP_lwip(char *prompt, BYTE* buf, int len)
{
#if 1	
	int i = 0;
	BYTE BUF[100];
  	sprintf(BUF,"%s: ", prompt);
  	UartOutText(BUF);
	for (i = 0; i < len; i ++)
	{
		sprintf(BUF,"%02x ",*buf++);
    		UartOutText(BUF);
  	}
  	UartOutText("\r\n");
#endif  	
}

static char *h_addr_ptrs[MAXADDRS + 1];
static struct in_addr addr;	/* IPv4 or IPv6 */
static struct hostent host;
int resolv_socket;

struct hostent * mpx_gethostbyname(S08* name)
{
    U08 *sendBuf = 0, *recvBuf = 0, *tmpBuf = 0, *cp = 0;
    S32 err = NO_ERR;
    U16 sockId = 0;
    ST_SOCK_ADDR localAddr, peerAddr;
    ST_SOCK_ADDR peerAddr2, *pPeerAddr2;
    //U32 u32PeerAddr = 0xc0a82f02;
    //U32 u32PeerAddr = 0xc0a80001;
    NS_QUERY_HEADER* q_header;
    U16 select = 0;
    ST_SOCK_SET stReadSet;
    U16 u16RcvLength = 0;
    U32 SendLength;
    U08 index, i;
    U32 TargetIp = 0;
    U08 SendCounter = 0;
    int ret;

    if(NetDefaultIpGet() == 0){
        DPrintf("no IP Address, please set IP Address again");
        return 0;
    }

    if(NetDNSGet(0) == INADDR_NONE){
        DPrintf("no DNS server, please set DNS server again");
        return 0;
    }

    if(NetDNSGet(1) == INADDR_NONE)
    {
        pPeerAddr2 = NULL;
    }
    else
    {
        mpx_SockAddrSet(&peerAddr2, NetDNSGet(1), NS_DEFAULTPORT);
        pPeerAddr2 = &peerAddr2;
    }

    sendBuf = (U08*)mpx_Malloc(NS_PACKETSZ);
    if(!sendBuf)
    {
        err = ERR_OUT_OF_MEMORY;
        goto ERROR;
    }
    memset(sendBuf, 0, NS_PACKETSZ);

    recvBuf = (U08*)mpx_Malloc(NS_PACKETSZ);
    if(!recvBuf)
    {
        err = ERR_OUT_OF_MEMORY;
        goto ERROR;
    }
    memset(recvBuf, 0, NS_PACKETSZ);

    err = mpx_Socket(AF_INET, SOCK_DGRAM);
    if(err <= 0){
        DPrintf("no Socket ID");
        goto ERROR;
    }

    sockId = (U16)err;
    mpx_SockAddrSet(&peerAddr, NetDNSGet(0), NS_DEFAULTPORT);
    mpx_SockAddrSet(&localAddr, NetDefaultIpGet(), mpx_NewLocalPort()); 
    mpx_Bind(sockId, &localAddr);        

    q_header = (NS_QUERY_HEADER*)sendBuf;
    
    q_header->id = (U16)(mpx_SystemTickerGet() & 0xffff);
    q_header->rd = 1;
    q_header->qdcount = 1;

    tmpBuf = sendBuf + sizeof(NS_QUERY_HEADER);

    tmpBuf++;
    memcpy(tmpBuf, name, UtilStringLength08(name));
    HEXDUMP_lwip("1 tmpBuf =",tmpBuf,strlen(tmpBuf));	
    cp = tmpBuf;
    tmpBuf--;

    i = 0;
    int len = UtilStringLength08(name)+1;
    for(index = 0; index <= len ; index++){
        if(*cp == 0x0){
            *tmpBuf = i;
            tmpBuf += i+1;
            break;
        }

        if(*cp == 0x2e){
            *tmpBuf = i;
            tmpBuf += i+1;
            i = 0;
            cp++;
        }
        else{
            i++;
            cp++;
        }
    }
 
    tmpBuf++;
//    HEXDUMP_lwip("2 tmpBuf =",tmpBuf,strlen(tmpBuf)); /* billwang - cause system hang */
    NetPutDW(tmpBuf, 0x00010001);

    SendLength = sizeof(NS_QUERY_HEADER) + UtilStringLength08(name) + 6;

Send:
    
    ret = mpx_SendTo(sockId, sendBuf, SendLength, 0, &peerAddr);
    if (ret < 0)
        goto ERROR;
    SendCounter++;

    mpx_TaskYield(10);
    if (pPeerAddr2)
        ret = mpx_SendTo(sockId, sendBuf, SendLength, 0, pPeerAddr2);
    else
        ret = mpx_SendTo(sockId, sendBuf, SendLength, 0, &peerAddr);
    if (ret < 0)
        goto ERROR;

    do
    {
    	//MP_DEBUG("666666666666666666666666   mpx_gethostbyname");
        select = 0;
        MPX_FD_ZERO(&stReadSet);
        MPX_FD_SET(sockId, &stReadSet);
        err = mpx_Select(&stReadSet, 0, 0, 4000); //3 seconds
        
        if(err > 0)
        {
            S32 status;
            U16 type;
            U16 length;
            
            select = (U16)err;
            status = mpx_RecvFrom(sockId, recvBuf, NS_PACKETSZ, 0, NULL);
            if(status > 0)
                u16RcvLength = (U16)status;
            else{
				DPrintf("selected but nothing Recv");
				BREAK_POINT();
                //select = 0;
            }

            q_header = (NS_QUERY_HEADER*)recvBuf;

            if(q_header->rcode != 0)
                goto ERROR;

            tmpBuf = recvBuf;
            tmpBuf += sizeof(NS_QUERY_HEADER);
			
			//Get Question
            tmpBuf += UtilStringLength08(name) + 6;

			//Get Answer
			if(q_header->ancount != 0){
				U08 Answers = q_header->ancount;
				while(Answers != 0){
					if(*tmpBuf & 0xc0){
						//compression
                    tmpBuf += 2;
                    }
                    else{
						//not compression
						U08 LabelLength = *tmpBuf;
						while(LabelLength != 0){
							if(LabelLength > 63){
								DPrintf("LabelLength %x is error, max is 63", LabelLength);
								BREAK_POINT();
							}
							tmpBuf += LabelLength + 1;
							LabelLength = *tmpBuf;
						}
						tmpBuf++;
					}
					type = NetGetW(tmpBuf);
					tmpBuf += 2;
					if(type == 0x05){
						//C_NAME
                        tmpBuf += 6;
                        length = NetGetW(tmpBuf);
                        tmpBuf +=length + 2;
                    }
					else if(type == 0x01){
						//A
						tmpBuf += 6;
                    	length = NetGetW(tmpBuf);
						tmpBuf += 2;
						if(length == 4){
						TargetIp = NetGetDW(tmpBuf);
							tmpBuf += length;
						NetDebugPrintIP(TargetIp);
                }
                else{
							DPrintf("type = %x but length != 4", type);
							BREAK_POINT();
						}
					}
					else{
						DPrintf("Current not support type %x\n", type);
						BREAK_POINT();
					}
					Answers--;
                }
            }
        }
        else
        {
            DPrintf("NS wait %1d response timeout", SendCounter);
            if(SendCounter == 3)
                goto ERROR;
            else
                goto Send;
        }
    }while(!select);

    //yuming, fetch information for backward compatible
    addr.s_addr = TargetIp;
    
    host.h_addrtype = AF_INET;
    host.h_addr_list = h_addr_ptrs;

    h_addr_ptrs[0] = (char *)&addr;
    h_addr_ptrs[1] = NULL;

	// free
    if(sockId)
    {
        mpx_SocketClose(sockId);
    }
    
    if(sendBuf)
        mpx_Free(sendBuf);

    if(recvBuf)
        mpx_Free(recvBuf);

    
    return &host;
 
ERROR:
    if(sockId)
    {
        mpx_SocketClose(sockId);
    }
    
    if(sendBuf)
        mpx_Free(sendBuf);

    if(recvBuf)
        mpx_Free(recvBuf);
    
    return 0;
}

S08* mpx_gethostbyaddr(U32 TargerIp)
{
    U08 *sendBuf = 0, *recvBuf = 0, *tmpBuf = 0, *cp = 0;
    S32 err = NO_ERR;
    U16 sockId = 0;
    ST_SOCK_ADDR localAddr, peerAddr;
    ST_SOCK_ADDR peerAddr2, *pPeerAddr2;
    NS_QUERY_HEADER* q_header;
    U16 found = 0;
    ST_SOCK_SET stReadSet;
    U16 u16RcvLength = 0;
    U32 SendLength;
    U08 index, i, ip;
    static char Host_name[NS_MAXDNAME];
    U08 SendCounter = 0;
    int ret;

    if(NetDefaultIpGet() == 0){
        DPrintf("no IP Address, please set IP Address again");
        return 0;
    }

    if(NetDNSGet(0) == INADDR_NONE){
        DPrintf("no DNS server, please set DNS server again");
        return 0;
    }

    if(NetDNSGet(1) == INADDR_NONE)
    {
        pPeerAddr2 = NULL;
    }
    else
    {
        mpx_SockAddrSet(&peerAddr2, NetDNSGet(1), NS_DEFAULTPORT);
        pPeerAddr2 = &peerAddr2;
    }
    memset(Host_name, 0, sizeof(Host_name));

    sendBuf = (U08*)mpx_Malloc(NS_PACKETSZ);
    if(!sendBuf)
    {
        err = ERR_OUT_OF_MEMORY;
        goto ERROR;
    }
    memset(sendBuf, 0, NS_PACKETSZ);

    recvBuf = (U08*)mpx_Malloc(NS_PACKETSZ);
    if(!recvBuf)
    {
        err = ERR_OUT_OF_MEMORY;
        goto ERROR;
    }
    memset(recvBuf, 0, NS_PACKETSZ);

    err = mpx_Socket(AF_INET, SOCK_DGRAM);
    if(err <= 0)
        goto ERROR;

    sockId = (U16)err;
    mpx_SockAddrSet(&peerAddr, NetDNSGet(0), NS_DEFAULTPORT);
    mpx_SockAddrSet(&localAddr, NetDefaultIpGet(), mpx_NewLocalPort());
    mpx_Bind(sockId, &localAddr);        

    q_header = (NS_QUERY_HEADER*)sendBuf;
    
    q_header->id = (U16)(mpx_SystemTickerGet() & 0xffff);
    q_header->rd = 1;
    q_header->qdcount = 1;

    tmpBuf = sendBuf + sizeof(NS_QUERY_HEADER);

    i = 0;
    for(index = 0; index <= 24; index += 8){
        if(((TargerIp>>index) & 0xff) >= 100){
            *tmpBuf = 0x03;
            tmpBuf++;
            ip = ((TargerIp>>index) & 0xff);
            sprintf(tmpBuf, "%d", ip);
            tmpBuf += 3;
            i += 4;
        }
        else if(((TargerIp>>index) & 0xff) >= 10){
            *tmpBuf = 0x02;
            tmpBuf++;
            ip = ((TargerIp>>index) & 0xff);
            sprintf(tmpBuf, "%d", ip);
            tmpBuf += 2;
            i += 3;
        }
        else{
            *tmpBuf = 0x01;
            tmpBuf++;
            ip = ((TargerIp>>index) & 0xff);
            sprintf(tmpBuf, "%d", ip);
            tmpBuf += 1;
            i += 2;
        }
    }

    NetPutDW(tmpBuf, 0x07696e2d);               /* in-addr */
    tmpBuf += 4;
    NetPutDW(tmpBuf, 0x61646472);
    tmpBuf += 4;
    NetPutDW(tmpBuf, 0x04617270);               /* arpa */
    tmpBuf += 4;
    *tmpBuf = 0x61;

    i += 14;
    tmpBuf += 2;
    NetPutDW(tmpBuf, 0x000c0001);               /* PTR IN */

    SendLength = sizeof(NS_QUERY_HEADER) + i + 4;

Send:

    ret = mpx_SendTo(sockId, sendBuf, SendLength, 0, &peerAddr);
    if (ret < 0)
        goto ERROR;
    SendCounter++;

    mpx_TaskYield(10);
    if (pPeerAddr2)
        ret = mpx_SendTo(sockId, sendBuf, SendLength, 0, pPeerAddr2);
    else
        ret = mpx_SendTo(sockId, sendBuf, SendLength, 0, &peerAddr);
    if (ret < 0)
        goto ERROR;

    do
    {
        struct timeval tv;
        found = 0;
        MPX_FD_ZERO(&stReadSet);
        MPX_FD_SET(sockId, &stReadSet);
		tv.tv_sec = 4;
		tv.tv_usec = 0;
        err = select(0, &stReadSet, NULL, NULL, &tv);
        if(err > 0)
        {
            S32 status;
            U16 type;
            U16 length;
            
            found = (U16)err;
            status = mpx_RecvFrom(sockId, recvBuf, NS_PACKETSZ, 0, NULL);
            if(status > 0)
                u16RcvLength = (U16)status;
            else
                found = 0;

            q_header = (NS_QUERY_HEADER*)recvBuf;

            if(q_header->rcode != 0)
                goto ERROR;

            tmpBuf = recvBuf;
            tmpBuf += sizeof(NS_QUERY_HEADER);
            tmpBuf += i + 4;
            while(*Host_name == 0){
                if(*tmpBuf = 0xc0)
                    tmpBuf += 2;
                type = NetGetW(tmpBuf);
                tmpBuf += 2;
                if(type == 0x0c){
                    tmpBuf += 6;
                    length = NetGetW(tmpBuf);
                    tmpBuf += 2;
                    cp = tmpBuf;
                    i = *cp;
                    cp += i + 1;
                    while(*cp != 0){
                        i = *cp;
                        *cp = 0x2e;
                        cp += i + 1;
                    }
                    memcpy(Host_name, tmpBuf +1 , length -2);
                }
                else{
                    tmpBuf += 6;
                    length = NetGetW(tmpBuf);
                    tmpBuf +=length + 2;
                }
            }
        }
        else
        {
            DPrintf("NS wait response timeout");
            if(SendCounter == 3)
                goto ERROR;
            else
                goto Send;
        }
    }while(!found);

	// free
    if(sockId)
    {
        mpx_SocketClose(sockId);
    }
    
    if(sendBuf)
        mpx_Free(sendBuf);

    if(recvBuf)
        mpx_Free(recvBuf);

    return (S08 *)Host_name;
 
ERROR:
    if(sockId)
    {
        mpx_SocketClose(sockId);
    }
    
    if(sendBuf)
        mpx_Free(sendBuf);

    if(recvBuf)
        mpx_Free(recvBuf);
    
    return 0;
}

/* 
 * An internal function
 */
int get_hostinfo(S08* name, struct hostent *h, u32_t *pttl, u8_t seqno)
{
    U08 *sendBuf = 0, *recvBuf = 0, *tmpBuf = 0, *cp = 0;
    S32 err = NO_ERR;
    U16 sockId = 0;
    ST_SOCK_ADDR localAddr, peerAddr;
    ST_SOCK_ADDR peerAddr2, *pPeerAddr2;
    ST_SOCK_ADDR peerAddr3, *pPeerAddr3 = NULL;
    ST_SOCK_ADDR peerAddr4, *pPeerAddr4 = NULL;
    NS_QUERY_HEADER* q_header;
    U16 num = 0;
    ST_SOCK_SET stReadSet;
    U16 u16RcvLength = 0;
    U32 SendLength;
    U08 index, i;
    U32 TargetIp = 0;
    U08 SendCounter = 0;
    int ret;
    U32 ipaddr;
    int sockId2 = 0;

    if(NetDefaultIpGet() == 0){
        DPrintf("no IP Address, please set IP Address again");
        return -1;
    }

    if(NetDNSGet(0) == INADDR_NONE){
        DPrintf("no DNS server, please set DNS server again");
        return -1;
    }

    if(NetDNSGet(1) == INADDR_NONE)
    {
        pPeerAddr2 = NULL;
    }
    else
    {
        mpx_SockAddrSet(&peerAddr2, NetDNSGet(1), NS_DEFAULTPORT);
        pPeerAddr2 = &peerAddr2;
    }

    /* Use 2nd network interface if it exists */

    if (mpx_NetLinkStatusGet(NIC_INDEX_PPP) && 
        mpx_NetLinkStatusGet(NIC_INDEX_WIFI))
    {
        if((ipaddr = NetDNSGet2(NIC_INDEX_PPP, 0)) != INADDR_NONE)
        {
            mpx_SockAddrSet(&peerAddr3, ipaddr, NS_DEFAULTPORT);
            pPeerAddr3 = &peerAddr3;

            if((ipaddr = NetDNSGet2(NIC_INDEX_PPP, 1)) != INADDR_NONE)
            {
                mpx_SockAddrSet(&peerAddr4, ipaddr, NS_DEFAULTPORT);
                pPeerAddr4 = &peerAddr4;
            }
        }
    }

    sendBuf = (U08*)mpx_Malloc(NS_PACKETSZ);
    if(!sendBuf)
    {
        err = ERR_OUT_OF_MEMORY;
        goto ERROR;
    }
    memset(sendBuf, 0, NS_PACKETSZ);

    recvBuf = (U08*)mpx_Malloc(NS_PACKETSZ);
    if(!recvBuf)
    {
        err = ERR_OUT_OF_MEMORY;
        goto ERROR;
    }
    memset(recvBuf, 0, NS_PACKETSZ);

	err = mpx_Socket(AF_INET, SOCK_DGRAM);
	if(err <= 0){
		DPrintf("no Socket ID");
		goto ERROR;
	}

	sockId = (U16)err;
	resolv_socket = sockId;

	mpx_SockAddrSet(&peerAddr, NetDNSGet(0), NS_DEFAULTPORT);
	mpx_SockAddrSet(&localAddr, NetDefaultIpGet(), mpx_NewLocalPort()); 
	mpx_Bind(sockId, &localAddr);        

    if (pPeerAddr3)
    {
        if ((sockId2 = socket(AF_INET, SOCK_DGRAM, 0)) > 0)
        {
            if (ipaddr = NetInterfaceIpGet("ppp0"))
            {
                mpx_SockAddrSet(&localAddr, ipaddr, mpx_NewLocalPort()); 
                if (bind(sockId2, &localAddr, sizeof localAddr) < 0)
                {
                    closesocket(sockId2);
                    sockId2 = 0;
                }
            }
        }
    }

    q_header = (NS_QUERY_HEADER*)sendBuf;
    
    q_header->id = seqno;
    q_header->rd = 1;
    q_header->qdcount = 1;

    tmpBuf = sendBuf + sizeof(NS_QUERY_HEADER);

    tmpBuf++;
    memcpy(tmpBuf, name, UtilStringLength08(name));
    //HEXDUMP_lwip("1 tmpBuf =",tmpBuf,strlen(tmpBuf));	
    cp = tmpBuf;
    tmpBuf--;

    i = 0;
    int len = UtilStringLength08(name)+1;
    for(index = 0; index <= len ; index++){
        if(*cp == 0x0){
            *tmpBuf = i;
            tmpBuf += i+1;
            break;
        }

        if(*cp == 0x2e){
            *tmpBuf = i;
            tmpBuf += i+1;
            i = 0;
            cp++;
        }
        else{
            i++;
            cp++;
        }
    }

    tmpBuf++;
//    HEXDUMP_lwip("2 tmpBuf =",tmpBuf,strlen(tmpBuf)); /* billwang - cause system hang */
    NetPutDW(tmpBuf, 0x00010001);

    SendLength = sizeof(NS_QUERY_HEADER) + UtilStringLength08(name) + 6;

Send:
    
    SendCounter++;
    if(SendCounter >= 3)
    {
        DPrintf("NS wait %1d response timeout", SendCounter);
        goto ERROR;
    }

    ret = mpx_SendTo(sockId, sendBuf, SendLength, 0, &peerAddr);
    DPrintf("mpx_SendTo to=0x%x, ret=%d", *(U32 *)&peerAddr.u08Address[0], ret);
    if (ret < 0)
        goto ERROR;

    mpx_TaskYield(10);
    if (pPeerAddr2)
        ret = mpx_SendTo(sockId, sendBuf, SendLength, 0, pPeerAddr2);
    else
        ret = mpx_SendTo(sockId, sendBuf, SendLength, 0, &peerAddr);
    if (ret < 0)
        goto ERROR;

    if (pPeerAddr3)
        mpx_SendTo(sockId2, sendBuf, SendLength, 0, pPeerAddr3);
    if (pPeerAddr4)
        mpx_SendTo(sockId2, sendBuf, SendLength, 0, pPeerAddr4);

    do
    {
#if  1  //Bill switch in between mpx_Select and select 1: use select 0:mpx_Select
        struct timeval _tv;
        _tv.tv_sec = 60;    // 60 seconds
        _tv.tv_usec = 0;
#endif
        num = 0;
        MPX_FD_ZERO(&stReadSet);
        MPX_FD_SET(sockId, &stReadSet);
        if (sockId2 > 0)
            MPX_FD_SET(sockId2, &stReadSet);
#if  0   //Bill switch in between mpx_Select and select 0: use select 1:mpx_Select
        err = mpx_Select(&stReadSet, 0, 0, 4000); //3 seconds
#else
		err = select(0, &stReadSet, NULL, NULL, &_tv);
#endif
        
        for (; err > 0; err--)
        {
            S32 status;
            U16 type;
            U16 length;
            int s;
            
            if (MPX_FD_ISSET(sockId, &stReadSet))
            {
                s = sockId;
                MPX_FD_CLR(sockId, &stReadSet);
            }
            else if (sockId2>0 && MPX_FD_ISSET(sockId2, &stReadSet))
            {
                s = sockId2;
                MPX_FD_CLR(sockId2, &stReadSet);
            }
            else
                break;

            if (s)
            {
                status = recvfrom(s, recvBuf, NS_PACKETSZ, 0, NULL, NULL);
            if(status > 0)
                u16RcvLength = (U16)status;
            else{
				DPrintf("selected but nothing Recv");
                    mpDebugPrint("Error: get_hostinfo: selected but nothing Recv");
                    break;
                }
            }

            q_header = (NS_QUERY_HEADER*)recvBuf;

            if(q_header->rcode != 0)
            {
                if (sockId2 <= 0)               /* only one interface */
                    goto ERROR;
                else
                    continue;
            }

            tmpBuf = recvBuf;
            tmpBuf += sizeof(NS_QUERY_HEADER);
			
			//Get Question
            tmpBuf += UtilStringLength08(name) + 6;

			//Get Answer
			if(q_header->ancount != 0){
				U08 Answers = q_header->ancount;
				while(Answers != 0){
					if(*tmpBuf & 0xc0){
						//compression
                    tmpBuf += 2;
                    }
                    else{
						//not compression
						U08 LabelLength = *tmpBuf;
						while(LabelLength != 0){
							if(LabelLength > 63){
								DPrintf("LabelLength %x is error, max is 63", LabelLength);
								BREAK_POINT();
							}
							tmpBuf += LabelLength + 1;
							LabelLength = *tmpBuf;
						}
						tmpBuf++;
					}
					type = NetGetW(tmpBuf);
					tmpBuf += 2;
					if(type == 0x05){
						//C_NAME
                        tmpBuf += 6;
                        length = NetGetW(tmpBuf);
                        tmpBuf +=length + 2;
                    }
					else if(type == 0x01){
						//A
						tmpBuf += 2;
						*pttl = NetGetDW(tmpBuf);
						tmpBuf += 4;
                    	length = NetGetW(tmpBuf);
						tmpBuf += 2;
						if(length == 4){
						TargetIp = NetGetDW(tmpBuf);
							tmpBuf += length;
						NetDebugPrintIP(TargetIp);
                            num = 1;
                }
                else{
							DPrintf("type = %x but length != 4", type);
							BREAK_POINT();
						}
					}
					else{
						DPrintf("Current not support type %x\n", type);
						BREAK_POINT();
					}
					Answers--;
                }
            }
        }
    }while(0);

    if (!num)
        goto Send;

    //yuming, fetch information for backward compatible
    addr.s_addr = TargetIp;
    
    h->h_addrtype = AF_INET;
    h->h_addr_list = h_addr_ptrs;

    h_addr_ptrs[0] = (char *)&addr;
    h_addr_ptrs[1] = NULL;

	// free
    if(sockId)
    {
        resolv_socket = 0;
        closesocket(sockId);
    }
    
    if(sockId2 > 0)
    {
        closesocket(sockId2);
    }
    
    if(sendBuf)
        mpx_Free(sendBuf);

    if(recvBuf)
        mpx_Free(recvBuf);

    
    return 0;
 
ERROR:
    if(sockId)
    {
        resolv_socket = 0;
        closesocket(sockId);
    }
    
    if(sockId2 > 0)
    {
        closesocket(sockId2);
    }
    
    if(sendBuf)
        mpx_Free(sendBuf);

    if(recvBuf)
        mpx_Free(recvBuf);
    
    return -1;
}


U32 resolv_query(char *name);
U32 resolv_lookup(char *name);

/**
 * @ingroup NET_SOCKET
 * @brief Retrieves host information corresponding to a host name from a 
 * host database.
 *
 * @param name Pointer to the null-terminated name of the host to resolve.
 *
 * This function is different from mpx_gethostbyname in that it uses DNS 
 * resolver cache.  If the name is not found in the cache, DNS queries will be
 * sent just like what mpx_gethostbyname does.
 *
 * The 'ttl' value in the DNS responses determines the lifetime of an entry in
 * DNS resolver cache.
 */
struct hostent* gethostbyname(const char* name)
{
    U32 TargetIp;
    if ((TargetIp=resolv_lookup(name)) == INADDR_NONE)
    {
        if ((TargetIp=resolv_query(name)) == INADDR_NONE)
        {
            h_errno = HOST_NOT_FOUND;
            return NULL;
        }
    }

    addr.s_addr = htonl(TargetIp);
    host.h_addrtype = AF_INET;
    host.h_addr_list = h_addr_ptrs;

    h_addr_ptrs[0] = (char *)&addr;
    h_addr_ptrs[1] = NULL;
    return &host;
}

