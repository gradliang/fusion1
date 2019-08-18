#ifndef __NET_ICMP_H
#define __NET_ICMP_H

#include "net_packet.h"

#define ICMP_TYPE_ER        0       /* echo reply */
#define ICMP_TYPE_DUR       3       /* destination unreachable */
#define ICMP_TYPE_SQ        4       /* source quench */
#define ICMP_TYPE_RD        5       /* redirect */
#define ICMP_TYPE_ECHO      8       /* echo */
#define ICMP_TYPE_TE        11      /* time exceeded */
#define ICMP_TYPE_PP        12      /* parameter problem */
#define ICMP_TYPE_TS        13      /* timestamp */
#define ICMP_TYPE_TSR       14      /* timestamp reply */
#define ICMP_TYPE_IRQ       15      /* information request */
#define ICMP_TYPE_IR        16      /* information reply */

typedef struct
{
    U08 u08Type;
    U08 u08Code;
    U16 u16Checksum;
} ST_ICMP_HEADER;

#define ICMP_TYPE           0
#define ICMP_CODE           1
#define ICMP_CHECKSUM       2

//echo request/reply
#define ICMP_ECHO_ID        4
#define ICMP_ECHO_SEQNO     6

#endif
