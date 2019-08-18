#ifndef __NET_UTILITY_H
#define __NET_UTILITY_H

#include "typedef.h"
#include "socket.h"

U16 InetCsum(void *start, U16 length);
inline U32 NetGetDW(U08* addr);
inline U16 NetGetW(U08* addr);
inline void NetPutDW(U08* addr, U32 value);
inline void NetPutW(U08* addr, U16 value);
BOOL NetMacAddrComp(U08* addr1, U08* addr2);
void NetMacAddrCopy(U08* destAddr, U08* srcAddr);
void NetDebugPrintIP(U32 ip);
BOOL NetIpMaskCompare(U32 ip1, U32 ip2, U32 mask);
void DPRINTF(const char * format, ...);
void NetDumpData(int level, const uint8_t *buf, int len);

/* 
 * inet_addr
 *
 * The function converts a string containing an (IPv4) Internet 
 * Protocol dotted address into a actual IP address in a binary 
 * representation.  The address is returned in network byte order.
 * 
 * If the string does not contain a legitimate IP address, INADDR_NONE
 * is returned.
 */
unsigned long inet_addr(const char *cp);

struct in_addr;
/*
 * inet_ntoa
 *
 * Convert an Internet (IPv4) network address into a string in Internet 
 * standard dotted format, "a.b.c.d".
 *
 * NOTE: This function is not re-entrant.
 */
char *inet_ntoa(struct in_addr in);

#endif
