#ifndef __NET_DHCP_H
#define __NET_DHCP_H

#include "net_socket.h"
#include "net_packet.h"
#include "error.h"
#include "net_device.h"

#define NET_PACKET_DHCP(p)                     NET_PACKET_UDP_DATA(p)

#define DHCP_CHADDR_LEN                     16
#define DHCP_SNAME_LEN                      64
#define DHCP_FILE_LEN                       128

//data fields offset
#define DHCP_OP_CODE                        0
#define DHCP_HW_TYPE                        1
#define DHCP_HW_ADDR_LEN                    2
#define DHCP_HOPS                           3
#define DHCP_TRANS_ID                       4
#define DHCP_SECONDS                        8
#define DHCP_FLAGS                          10
#define DHCP_CLIENT_IP                      12
#define DHCP_YOUR_IP                        16
#define DHCP_SERVER_IP                      20
#define DHCP_RELAY_IP                       24
#define DHCP_CLIENT_HW_ADDR                 28
#define DHCP_SERVER_HOST_NAME               (DHCP_CLIENT_HW_ADDR+DHCP_CHADDR_LEN)
#define DHCP_BOOT_FILE_NAME                 (DHCP_SERVER_HOST_NAME+DHCP_SNAME_LEN)
#define DHCP_COOKIE                         (DHCP_BOOT_FILE_NAME+DHCP_FILE_LEN)
#define DHCP_OPTION                         (DHCP_COOKIE+4)


#define DHCP_HEADER_LEN                     DHCP_OPTION
#define DHCP_OPTIONS_LEN                    68


//DHCP fields definition
#define DHCP_BOOT_REQUEST                   1
#define DHCP_BOOT_REPLY                     2
                                            
#define DHCP_HW_TYPE_ETHER                  1
                                            
#define DHCP_HW_ADDR_LENGTH                 6

#define DHCP_DISCOVER                       1
#define DHCP_OFFER                          2
#define DHCP_REQUEST                        3
#define DHCP_DECLINE                        4
#define DHCP_ACK                            5
#define DHCP_NAK                            6
#define DHCP_RELEASE                        7
#define DHCP_INFORM                         8


//BootP option
#define DHCP_OPTION_PAD                     0
#define DHCP_OPTION_SUBNET_MASK             1 /* RFC 2132 3.3 */
#define DHCP_OPTION_ROUTER                  3
#define DHCP_OPTION_DNS_SERVER              6 
#define DHCP_OPTION_HOSTNAME                12
#define DHCP_OPTION_IP_TTL                  23
#define DHCP_OPTION_MTU                     26
#define DHCP_OPTION_BROADCAST               28
#define DHCP_OPTION_TCP_TTL                 37
#define DHCP_OPTION_END                     255


//DHCP option related
#define DHCP_OPTION_HOST_NAME               12 /* */
#define DHCP_OPTION_REQUESTED_IP            50 /* RFC 2132 9.1, requested IP address */
#define DHCP_OPTION_LEASE_TIME              51 /* RFC 2132 9.2, time in seconds, in 4 bytes */
#define DHCP_OPTION_OVERLOAD                52 /* RFC2132 9.3, use file and/or sname field for options */
                                            
#define DHCP_OPTION_MESSAGE_TYPE            53 /* RFC 2132 9.6, important for DHCP */
#define DHCP_OPTION_MESSAGE_TYPE_LEN        1


#define DHCP_OPTION_SERVER_ID               54 /* RFC 2132 9.7, server IP address */
#define DHCP_OPTION_PARAMETER_REQUEST_LIST  55 /* RFC 2132 9.8, requested option types */

#define DHCP_OPTION_MAX_MSG_SIZE            57 /* RFC 2132 9.10, message size accepted >= 576 */
#define DHCP_OPTION_MAX_MSG_SIZE_LEN        2

#define DHCP_OPTION_T1                      58 /* T1 renewal time */
#define DHCP_OPTION_T2                      59 /* T2 rebinding time */
#define DHCP_OPTION_CLIENT_ID               61
#define DHCP_OPTION_TFTP_SERVERNAME         66
#define DHCP_OPTION_BOOTFILE                67


//DHCP port number
#define DHCP_SERVER_PORT                    67
#define DHCP_CLIENT_PORT                    68


//DHCP return code
#define DHCP_ERR_NO_ERR                     NO_ERR
#define DHCP_ERR_START_FAIL                 -1


//DHCP client state
#define DHCP_STATE_REQUESTING               1
#define DHCP_STATE_INIT                     2
#define DHCP_STATE_REBOOTING                3
#define DHCP_STATE_REBINDING                4
#define DHCP_STATE_RENEWING                 5
#define DHCP_STATE_DISCOVERING              6
#define DHCP_STATE_INFORMING                7
#define DHCP_STATE_CHECKING                 8
#define DHCP_STATE_PERMANENT                9
#define DHCP_STATE_BOUND                    10
#define DHCP_STATE_BACKING_OFF              12
#define DHCP_STATE_OFF                      13


// possible combinations of overloading the file and sname fields with options */
#define DHCP_OVERLOAD_NONE                  0
#define DHCP_OVERLOAD_FILE                  1
#define DHCP_OVERLOAD_SNAME                 2
#define DHCP_OVERLOAD_SNAME_FILE            3

#define DHCP_MAX_DNS                        2

typedef struct
{
    U08 u08State;
    U08 u08DnsCount;
    U08 u08Reserved[2];
    U32 u32Xid;
    
    //dhcp ip info
    U32 u32ServerIpAddr;    //dhcp server ip
    U32 u32OfferedIpAddr;   //my ip
    U32 u32OfferedSnMask;   //subnet mask
    U32 u32OfferedGwAddr;   //gateway router
    U32 u32OfferedBcAddr;   //broadcast address
    U32 u32OfferedDnsAddr[DHCP_MAX_DNS];
    
    //dhcp time info
    U32 u32OfferedLeaseTime;
    U32 u32OfferedRenewTime;
    U32 u32OfferedRebindTime;
    
    U32 u32RequestTimeout;
    U32 u32RebindTimeout;
    U32 u32RenewTimeout;
    U32 u32LeaseTimeout;                        /* remaining lease time */

    int u08RetryCount;
    U08 u08NextState;
    U08 u08UseManualIp;

    U08 u08LinkUp;
    U32 u32LinkTimeout;               /* waiting time after link is 
                                         down before release a DHCP lease */
    U08 u08FastRenew;

    struct net_device *dhcpDev;       /* network device this DHCP instance is for */
} ST_DHCP;

typedef void (*DHCP_STATUS_CALLBACK)(U32 status);

#define DHCP_STATUS_GET_IP_FINISH       0

#define DHCP_REQUEST_TIMEOUT            20 //20 seconds

#define DHCP_MAX_RETRIES            2

//DHCP api
S32 mpx_DhcpStart(int nic);
S32 mpx_DhcpManualIpSet(int ifindex, U32 u32IpAddr, U32 u32SubnetMask, U32 u32GatewayIp, U32 u32DnsServerIp);
//TBD: S32 mpx_DhcpInform(U32 ipAddr); //set fix ip address
S32 mpx_DhcpStatusCallbackSet(DHCP_STATUS_CALLBACK statusCallback);
U32 mpx_DhcpIpAddrGet();
U32 mpx_DhcpSubnetMaskGet();
U32 mpx_DhcpGatewayAddrGet();
U32 mpx_DhcpServerIpAddrGet();
U32 mpx_DhcpLeaseTimeGet();
U32 mpx_DhcpRemainedLeaseTimeGet();
U32 mpx_DhcpRemainedRenewTimeGet();
U08 mpx_DhcpDnsAddrNumGet();
U32 mpx_DhcpDnsAddrGet(U08 index);
int mpx_DhcpRun(struct net_device *dev);
BOOL mpx_DhcpEnableGet(struct net_device *dev);
void mpx_DhcpEnableSet(BOOL dhcp_enabled, struct net_device *dev);
void mpx_DhcpLinkEventSend(BOOL link_up, struct net_device *dev);
void mpx_DhcpInit(struct net_device *dev);

void DhcpInit();
#endif
