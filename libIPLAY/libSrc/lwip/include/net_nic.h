
#ifndef __NET_NIC_H
#define __NET_NIC_H

#include "typedef.h"
#include "socket.h"
#include "net_packet.h"
#include "net_device.h"
#include "net_const.h"

#define NIC_INDEX_NULL          0
#define NIC_INDEX_PPP           1
#define NIC_INDEX_WIFI          2
#define NIC_INDEX_ETHER         3
#define NIC_DRIVER_MAX          4

void net_buf_mem_free(void *rmem);
#define	IFNAMSIZ	16
#ifdef MP600
//#include "wireless.h"
#ifdef __KERNEL__
#include "linux/skbuff.h"
#endif
#endif

#ifndef MP600
struct ifreq{
#define IFHWADDRLEN	6
	union
	{
		char	ifrn_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	} ifr_ifrn;
	
	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		struct	sockaddr ifru_netmask;
		struct  sockaddr ifru_hwaddr;
		short	ifru_flags;
		int	ifru_ivalue;
		int	ifru_mtu;
		char	ifru_slave[IFNAMSIZ];	/* Just fits the size */
		char	ifru_newname[IFNAMSIZ];
		void *	ifru_data;
	} ifr_ifru;
};

#define ifr_name	ifr_ifrn.ifrn_name	/* interface name 	*/
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface	*/
#define	ifr_addr	ifr_ifru.ifru_addr	/* address		*/
#endif

#define MAX_ADDR_LEN	32		/* Largest hardware address length */

/* Standard interface flags (netdevice->flags). */
#define	IFF_BROADCAST	0x2		/* broadcast address valid	*/
#define	IFF_PROMISC	0x100		/* receive all packets		*/
#define	IFF_ALLMULTI	0x200		/* receive all multicast packets*/
#define	IFF_MULTICAST	0x1000		/* Supports multicast		*/

#define IFF_LOWER_UP	0x10000		/* driver signals L1 up		*/

/*
 *	Network device statistics. Akin to the 2.0 ether stats but
 *	with byte counters.
 */
 
#ifndef MP600
struct net_device_stats
{
	U32	rx_packets;		/* total packets received	*/
	U32	tx_packets;		/* total packets transmitted	*/
	U32	rx_bytes;		/* total bytes received 	*/
	U32	tx_bytes;		/* total bytes transmitted	*/
	U32	rx_errors;		/* bad packets received		*/
	U32	tx_errors;		/* packet transmit problems	*/
	U32	rx_dropped;		/* no space in linux buffers	*/
	U32	tx_dropped;		/* no space available in linux	*/
	U32	multicast;		/* multicast packets received	*/
	U32	collisions;

	/* detailed rx_errors: */
	U32	rx_length_errors;
	U32	rx_over_errors;		/* receiver ring buff overflow	*/
	U32	rx_crc_errors;		/* recved pkt with crc error	*/
	U32	rx_frame_errors;	/* recv'd frame alignment error */
	U32	rx_fifo_errors;		/* recv'r fifo overrun		*/
	U32	rx_missed_errors;	/* receiver missed packet	*/

	/* detailed tx_errors */
	U32	tx_aborted_errors;
	U32	tx_carrier_errors;
	U32	tx_fifo_errors;
	U32	tx_heartbeat_errors;
	U32	tx_window_errors;
	
	/* for cslip etc */
	U32	rx_compressed;
	U32	tx_compressed;
};
#endif

typedef struct
{
    U08 dev_addr[MAX_ADDR_LEN];
    U08 Rssi;
    U08 ssid_length;
    U08 RESERVE1[2];
    U08 ssid[32];
    U32 Privacy;
    U32 Channel;
    U08 InfrastructureMode;
    U08 RESERVED2[3];
    U08 Wpa_ie[256];
    U08 Wpa_ie_len;
    U08 Wpa2_ie[256];
    U08 Wpa2_ie_len;
} wlan_ap;

typedef struct wlan_ap_table_s
{
    U08 NumInScanTable;
    U08 Reserved[3];
#define MAX_SCAN_AP 20
    wlan_ap ap[MAX_SCAN_AP];
} wlan_ap_table;


#define NIC_REQUEST_INTERRUPT                                       0x01
#define NIC_REQUEST_SEND                                            0x02
#define NIC_REQUEST_RECEIVE                                         0x04
#define NIC_REQUEST_INFORM_SEND_DONE                                0x08
#define NIC_REQUEST_LINK_STATE_CHANGED                              0x10
#define NIC_REQUEST_ALL                                             (NIC_REQUEST_INTERRUPT|NIC_REQUEST_RECEIVE|NIC_REQUEST_LINK_STATE_CHANGED)

typedef struct
{
    struct net_device   *Nic;
    U32                 u32Request;
    ST_NET_PACKET       *Packet;
} ST_NIC_REQUEST;



typedef void (*NET_DRIVER_CONSTRUCTOR)(struct net_device *);

extern struct net_device NicArray[NIC_DRIVER_MAX];
extern BYTE wifi_device_type;

// net receive task
#define NET_PACKET_RECEIVE          0x00000001

//nic apis
S32 mpx_NetworkStart(U08 u08NicIdx);
S32 mpx_NetworkStop();
BOOL mpx_NetLinkStatusGet(U08 if_index);
S32 mpx_NetOpen();
S32 mpx_NetClose();

#define mpx_NicMACAddressGet    NetLocalMACGet

//Wlan API
S32 mpx_WlanOpen();
S32 mpx_WlanClose();

S32 mpx_WlanGetMacAddress(U08*);

S32 mpx_WlanScan(wlan_ap_table*);
S32 mpx_WlanSpecificSSIDScan(U08*, wlan_ap_table*);

#if 0
S32 mpx_WlanConnect(U08* ssid, U08 key_mgmt, U08 auth_alg, U08 key_index, U08* wep_key, U08* psk);
#else
S32 mpx_WlanConnect(U08* ssid);
#endif
S32 mpx_WlanDisconnect();

/*
 * @ingroup    WIFI
 * @brief    Set wireless mode of operation
 *
 * @param    mode    Mode of operation
 * 
 * @retval  0   Success.
 * @retval  -EBUSY Wireless hardware is busy.
 * @retval  -EINVAL Unsupported mode.
 * @retval  WLAN_STATUS_FAILURE Command failed.
 * 
 * @remark  The <mode> can be IW_MODE_ADHOC, IW_MODE_INFRA, or IW_MODE_AUTO.
 */
S32 mpx_WlanSetMode(U32 mode);

S32 mpx_WlanInfo();

S32 mpx_WlanSetWEP(U32 flags, U08 key_index, U08* wep_key);

ST_NET_PACKET* NetNewPacket(BOOL clear);
ST_NET_PACKET* NetNewPacketWithSize(U16 size, BOOL clear);
void NetFreePacket(ST_NET_PACKET* packet);

#ifdef __KERNEL__
void net_buf_mem_free(void *rmem);
/**
 * @ingroup NET_BUFFER
 * @brief Free a network buffer
 * 
 * @retval None
*/
static inline void netBufFree(struct sk_buff *skb)
{
	if(wifi_device_type == WIFI_USB_DEVICE_RT73)
	{
	    int free_address;
	    if(skb->truesize == 1)
	    {
	      //BREAK_POINT();
	      free_address = (DWORD)(skb->data)-0x3A;
		  //mpDebugPrint("free skb->data %x",free_address);
	      net_buf_mem_free((void *)free_address);
		  skb->truesize = 0;
	    }
	}
	if(skb)
	{
#ifndef SKB_OPT
    	net_buf_mem_free(skb);
#else
        if (skb->data_allocated && skb->head)
            net_buf_mem_free(skb->head);
        mm_free(skb);
#endif
	}
}
#endif

void NetDefaultIpSet(U08 if_index, U32 ipAddr);
U32  NetDefaultIpGet();
void NetSubnetMaskSet(U08 if_index, U32 snMask);
U32  NetSubnetMaskGet();
void NetGatewayIpSet(U08 if_index, U32 gwAddr);
U32  NetGatewayIpGet();
void NetDNSSet(U08 if_index, U32 DnsAddr,U08 index);
U32  NetDNSGet(U08 index);
U32  NetDNSGet2(U08 if_index, U08 index);

void NetNicInit();
struct net_device * NetNicDeviceGet(int if_index);
U08  NetDefaultNicGet();
U08 NetNicGet(U32 ipAddr);
U08* NetNullMACGet();

void *NetDhcpSet(U32 dhcp, int if_idx);
U32  NetDhcpGet(int if_idx);

/*
 * NetHostNameSet:
 *
 * Set Internet host name of this system.
 *
 * Host names up to 255 characters are supported.  This host name will be
 * passed to DHCP server.
 *
 * Return values:
 *   NONE
 */
void NetHostNameSet(char *nm);

/*
 * NetHostNameGet:
 *
 * Retrieve the Internet host name of this system.
 *
 * Return values:
 *   non-NULL     = string of host name is returned.
 *   NULL         = No host name defined.
 */
char *NetHostNameGet(void);

U08 NetNicSelectGet();

void NetNicDataRateGet(U32* outTxBytes, U32* outRxBytes);
struct net_device *NetDeviceGetByIfname(U08* ifname);

/* Set a certain interface flag. 
 *
 * ioctl - SIOCSIFFLAGS
 * flags supported include:
 *  IFF_PROMISC
 *  IFF_ALLMULTI
 */
int if_set_flag(int if_index, short flag);

/* Clear a certain interface flag. 
 *
 * ioctl - SIOCSIFFLAGS
 * flags supported include:
 *  IFF_PROMISC
 *  IFF_ALLMULTI
 */
int if_clr_flag(int if_index, short flag);

/* Add hardware multicast address to device's multicast filter list.
 * ioctl - SIOCADDMULTI
 * You can enter multiple multicast addresses by calling this routine many 
 * times.  But you have to issue a call like the following:
 *
 *      if_add_mc_address(if_index, NULL, 0);
 *
 * after all addresses are entered.  Then driver will write all entered addresses 
 * to the WLAN card.
 */
int if_add_mc_address(int if_index, struct sockaddr *if_hwaddr, int addrlen);

BOOL mpx_NetLinkStatusGet(U08 if_index);
void mpx_NetLinkStatusSet(U08 if_index, BOOL conn);
U32 NetInterfaceIpGet(char *if_name);

#endif

