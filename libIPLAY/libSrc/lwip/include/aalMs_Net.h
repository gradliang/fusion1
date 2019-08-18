/***********************************************************************
 Copyright (C) 2007-2009 AIM Corporation

 This software may not be used in any way or distributed without permission.
 All rights reserved.

 File: aalMs_Net.h
***********************************************************************/
#ifndef _AALMS_NET_
#define _AALMS_NET_

#ifdef __cplusplus
extern "C"{
#endif 

/* dependencies */
#include "aim_types.h"
#include "aalMs_err.h"



#define AALMS_NET_IF_ADDR_SIZE_MAC	(6)		/* 48-bit MAC/net addr size. */

/***********************************************************************
	Function Prototypes
***********************************************************************/
#ifdef  _AALMS_NET_GLOBAL_DEF_
#define AALMS_NET_FUNC
#else	/* _AALMS_NET_GLOBAL_DEF_ */
#define AALMS_NET_FUNC	extern
#endif	/* _AALMS_NET_GLOBAL_DEF_ */

AALMS_NET_FUNC aal_msync_error_t	aalMs_NET_RxIPpkt(a_uchar_t *in_buff, a_uint16_t in_size );
AALMS_NET_FUNC aal_msync_error_t aalMs_NET_TCPRxSignal(void);
AALMS_NET_FUNC a_bool_t aalMs_NET_GetTCPRecvTimeoutOccured( void ) ;
AALMS_NET_FUNC void aalMs_NET_ClearTCPRecvTimeoutOccured( void ) ;
AALMS_NET_FUNC a_uchar_t *aalMs_NET_GetMacAddress(void) ;
AALMS_NET_FUNC aal_msync_error_t aalMs_NET_InitDNS(a_uint32_t in_dns_addr, a_uint32_t in_dns_addr2) ;
AALMS_NET_FUNC void aalMs_NET_CfgIPAddrThisHost(a_uint32_t in_set_ip, a_uint32_t in_set_mask, a_uint32_t in_set_gateway ) ;


#ifdef  _AALMS_NET_TXPKTDRV_GLOBAL_DEF_
#define AALMS_NET_TXPKTDRV_FUNC
#else	/* _AALMS_NET_TXPKTDRV_GLOBAL_DEF_ */
#define AALMS_NET_TXPKTDRV_FUNC	extern
#endif	/* _AALMS_NET_TXPKTDRV_GLOBAL_DEF_ */

AALMS_NET_TXPKTDRV_FUNC aal_msync_error_t	aalMs_NETHandler_TxIPpkt(a_uchar_t *in_buff, a_uint16_t in_size ) ;


#ifdef __cplusplus
}
#endif 

	
#endif /* _AALMS_NET_ */
