/***********************************************************************
 Copyright (C) 2007-2009 AIM Corporation

 This software may not be used in any way or distributed without permission.
 All rights reserved.

 File: aalMs_Net.c
 This is sample code of MagicSync Abstruction Layer for microCOS
***********************************************************************/
#define LOCAL_DEBUG_ENABLE 1
	
#include "global612.h"
#include "mpTrace.h"


#if Make_MagicSync

#define _AALMS_PKTDRV_GLOBAL_DEF_
#define _AALMS_NET_TXPKTDRV_GLOBAL_DEF_
#include "aalMs_NET.h"

#include "net_packet.h"

extern struct net_device NicArray[NIC_DRIVER_MAX];

/***********************************************************************
	Definitions
***********************************************************************/

static a_uchar_t gMacAddress_Dummy[AALMS_NET_IF_ADDR_SIZE_MAC] = {0x00,0x00,0x00,0x00,0x00,0x00};
static a_uchar_t gTarget_MacAddress_Dummy[AALMS_NET_IF_ADDR_SIZE_MAC] = {0x00,0x00,0x00,0x00,0x00,0x00};

static a_uchar_t msync_gPppRxBuff[2048] ;
static a_uint16_t msync_gPppPktSize ;


/*******************************************************************************
	Function	:	aalMs_NET_RxIPpkt()
*******************************************************************************/
aal_msync_error_t	aalMs_NET_RxIPpkt(a_uchar_t *in_buff, a_uint16_t in_size )
{
	a_uchar_t	*frame_buff;
	a_uint16_t	length;
	ST_NET_PACKET *packet;

	if( A_NULL == in_buff ){
		return AALMS_ERROR ;
	}
	if( 1500 < in_size ){
		return AALMS_ERROR ;
	}

	if ((length = in_size + 14) < 64) {
		length = 64;
	}

#if 0
	packet = net_buf_mem_malloc(sizeof(ST_NET_PACKET));
#else
	packet = NetNewPacket(FALSE);
#endif

	memcpy(packet->u08Data[0], gMacAddress_Dummy, 6);
	memcpy(packet->u08Data[6], gTarget_MacAddress_Dummy, 6);
	packet->u08Data[12] = 0x08;		/* Type field:(IPv4=0x0800) */
	packet->u08Data[13] = 0x00;
	memcpy(packet->u08Data[14], in_buff, in_size);

	packet->Net.u16PayloadSize = length;

	IpPacketReceive(packet, 0, &NicArray[NIC_INDEX_PPP]);

	return AALMS_NO_ERROR ;
}

/*******************************************************************************
	Function	:	aalMs_NET_GetTCPRecvTimeoutOccured()
*******************************************************************************/
a_bool_t aalMs_NET_GetTCPRecvTimeoutOccured( void )
{
	return A_FALSE;
}

/*******************************************************************************
	Function	:	aalMs_NET_ClearTCPRecvTimeoutOccured()
*******************************************************************************/
void aalMs_NET_ClearTCPRecvTimeoutOccured( void )
{
	return;
}


/*******************************************************************************
	Function	:	aalMs_NET_GetMacAddress()
*******************************************************************************/
a_uchar_t *aalMs_NET_GetMacAddress(void)
{
	//return NetARP_GetHostAddrPtrHW() ;
	return gMacAddress_Dummy;
}


/*******************************************************************************
	Function	:	aalMs_NET_InitDNS()
*******************************************************************************/
aal_msync_error_t aalMs_NET_InitDNS(a_uint32_t in_dns_addr, a_uint32_t in_dns_addr2)
{
	NetDNSSet(NIC_INDEX_PPP, in_dns_addr, 0);
	NetDNSSet(NIC_INDEX_PPP, in_dns_addr2, 1);
	return AALMS_NO_ERROR;
}

/*******************************************************************************
	Function	:	aalMs_NET_CfgIPAddrThisHost()
*******************************************************************************/
void aalMs_NET_CfgIPAddrThisHost(a_uint32_t in_set_ip, a_uint32_t in_set_mask, a_uint32_t in_set_gateway )
{	
	NetDefaultIpSet(NIC_INDEX_PPP, in_set_ip);
	NetSubnetMaskSet(NIC_INDEX_PPP, in_set_mask);
	NetGatewayIpSet(NIC_INDEX_PPP, in_set_gateway);

	EnableNetWareTask();
	netif_carrier_on(&NicArray[NIC_INDEX_PPP]);
	if(NetDefaultNicGet() == NIC_INDEX_NULL)
		NetDefaultNicSet(NIC_INDEX_PPP);

	return ;
}




/**********************************************************************
	Abstraction handler
	This API is defined in MagicSync Library
**********************************************************************/
aal_msync_error_t	aalMs_NETHandler_TxIPpkt(a_uchar_t *in_buff, a_uint16_t in_size ){
	//dummy
}

#endif

