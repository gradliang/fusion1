#ifndef _NET_DEVICE_H
#define _NET_DEVICE_H

#if 0 //cjhuang mask
struct divert_blk;
struct vlan_group;
struct ethtool_ops;
struct netpoll_info;
#endif

#include <if.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/cache.h>
#ifdef __KERNEL__
#include <linux/device.h>
#endif
					/* source back-compat hooks */
#define SET_ETHTOOL_OPS(netdev,ops) \
	( (netdev)->ethtool_ops = (ops) )

#define HAVE_ALLOC_NETDEV		/* feature macro: alloc_xxxdev
					   functions are available. */
#define HAVE_FREE_NETDEV		/* free_netdev() */
#define HAVE_NETDEV_PRIV		/* netdev_priv() */

#define NET_XMIT_SUCCESS	0
#define NET_XMIT_DROP		1	/* skb dropped			*/
#define NET_XMIT_CN		2	/* congestion notification	*/
#define NET_XMIT_POLICED	3	/* skb is shot by police	*/
#define NET_XMIT_BYPASS		4	/* packet does not leave via dequeue;
					   (TC use only - dev_queue_xmit
					   returns this as NET_XMIT_SUCCESS) */

/* Backlog congestion levels */
#define NET_RX_SUCCESS		0   /* keep 'em coming, baby */
#define NET_RX_DROP		1  /* packet dropped */
#define NET_RX_CN_LOW		2   /* storm alert, just in case */
#define NET_RX_CN_MOD		3   /* Storm on its way! */
#define NET_RX_CN_HIGH		4   /* The storm is here */
#define NET_RX_BAD		5  /* packet dropped due to kernel error */

#define net_xmit_errno(e)	((e) != NET_XMIT_CN ? -ENOBUFS : 0)


#define MAX_ADDR_LEN	32		/* Largest hardware address length */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31))
/* Driver transmit return codes */
#define NETDEV_TX_OK 0		/* driver took care of packet */
#define NETDEV_TX_BUSY 1	/* driver tx path was busy*/
#define NETDEV_TX_LOCKED -1	/* driver tx lock was already taken */
#endif

/*
 *	Compute the worst case header length according to the protocols
 *	used.
 */
 
#if !defined(CONFIG_AX25) && !defined(CONFIG_AX25_MODULE) && !defined(CONFIG_TR)
#define LL_MAX_HEADER	32
#else
#if defined(CONFIG_AX25) || defined(CONFIG_AX25_MODULE)
#define LL_MAX_HEADER	96
#else
#define LL_MAX_HEADER	48
#endif
#endif

#if !defined(CONFIG_NET_IPIP) && \
    !defined(CONFIG_IPV6) && !defined(CONFIG_IPV6_MODULE)
#define MAX_HEADER LL_MAX_HEADER
#else
#define MAX_HEADER (LL_MAX_HEADER + 48)
#endif

//#ifndef WIRELESS_EXT
//#define WIRELESS_EXT	21
//#endif
//for Wifi USB
//#if Make_USB
int errno;

extern BYTE wifi_device_type;

#include "net_const.h"

#define AR2524_MAX_NETPOOL_BUFFER_SIZE 4800
#define AR2524_MAX_NETPOOL_ALLOC_SIZE (AR2524_MAX_NETPOOL_BUFFER_SIZE+32)
#define AR2524_MAX_NUM_BUFFERS (NETPOOL_BUF_SIZE/ AR2524_MAX_NETPOOL_ALLOC_SIZE)

#define RALINK_MAX_NETPOOL_ALLOC_SIZE 2560
#define RALINK_MAX_NETPOOL_BUFFER_SIZE (RALINK_MAX_NETPOOL_ALLOC_SIZE-32)
#define RALINK_MAX_NUM_BUFFERS 500
//#endif
struct wlan_ap_table_s;
struct sk_buff;

/* These flag bits are private to the generic network queueing
 * layer, they may not be explicitly referenced by any other
 * code.
 */

enum netdev_state_t
{
	__LINK_STATE_START,
	__LINK_STATE_PRESENT,
	__LINK_STATE_NOCARRIER,
	__LINK_STATE_LINKWATCH_PENDING,
	__LINK_STATE_DORMANT,
};
/*
 *	Network device statistics. Akin to the 2.0 ether stats but
 *	with byte counters.
 */
 
struct net_device_stats
{
	unsigned long	rx_packets;		/* total packets received	*/
	unsigned long	tx_packets;		/* total packets transmitted	*/
	unsigned long	rx_bytes;		/* total bytes received 	*/
	unsigned long	tx_bytes;		/* total bytes transmitted	*/
	unsigned long	rx_errors;		/* bad packets received		*/
	unsigned long	tx_errors;		/* packet transmit problems	*/
	unsigned long	rx_dropped;		/* no space in linux buffers	*/
	unsigned long	tx_dropped;		/* no space available in linux	*/
	unsigned long	multicast;		/* multicast packets received	*/
	unsigned long	collisions;

	/* detailed rx_errors: */
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;		/* receiver ring buff overflow	*/
	unsigned long	rx_crc_errors;		/* recved pkt with crc error	*/
	unsigned long	rx_frame_errors;	/* recv'd frame alignment error */
	unsigned long	rx_fifo_errors;		/* recv'r fifo overrun		*/
	unsigned long	rx_missed_errors;	/* receiver missed packet	*/

	/* detailed tx_errors */
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
	unsigned long	tx_window_errors;
	
	/* for cslip etc */
	unsigned long	rx_compressed;
	unsigned long	tx_compressed;
};
/*
 *	The DEVICE structure.
 *	Actually, this whole structure is a big mistake.  It mixes I/O
 *	data with strictly "high-level" data, and it has to know about
 *	almost every data structure used in the INET module.
 *
 *	FIXME: cleanup struct net_device such that network protocol info
 *	moves out.
 */

struct net_device
{
    char			name[IFNAMSIZ];

#define NETIF_F_SG		1	/* Scatter/gather IO. */
#define NETIF_F_IP_CSUM		2	/* Can checksum TCP/UDP over IPv4. */
#define NETIF_F_NO_CSUM		4	/* Does not require checksum. F.e. loopack. */
#define NETIF_F_HW_CSUM		8	/* Can checksum all the packets. */
#define NETIF_F_IPV6_CSUM	16	/* Can checksum TCP/UDP over IPV6 */
#define NETIF_F_HIGHDMA		32	/* Can DMA to high memory. */
#define NETIF_F_FRAGLIST	64	/* Scatter/gather IO. */
#define NETIF_F_HW_VLAN_TX	128	/* Transmit VLAN hw acceleration */
#define NETIF_F_HW_VLAN_RX	256	/* Receive VLAN hw acceleration */
#define NETIF_F_HW_VLAN_FILTER	512	/* Receive filtering on VLAN */
#define NETIF_F_VLAN_CHALLENGED	1024	/* Device cannot handle VLAN packets */
#define NETIF_F_GSO		2048	/* Enable software GSO. */
#define NETIF_F_LLTX		4096	/* LockLess TX - deprecated. Please */
					/* do not use LLTX in new drivers */
#define NETIF_F_NETNS_LOCAL	8192	/* Does not change network namespaces */
#define NETIF_F_LRO		32768	/* large receive offload */

	/* Segmentation offload features */
#define NETIF_F_GSO_SHIFT	16
#define NETIF_F_GSO_MASK	0xffff0000
#define NETIF_F_TSO		(SKB_GSO_TCPV4 << NETIF_F_GSO_SHIFT)
#define NETIF_F_UFO		(SKB_GSO_UDP << NETIF_F_GSO_SHIFT)
#define NETIF_F_GSO_ROBUST	(SKB_GSO_DODGY << NETIF_F_GSO_SHIFT)
#define NETIF_F_TSO_ECN		(SKB_GSO_TCP_ECN << NETIF_F_GSO_SHIFT)
#define NETIF_F_TSO6		(SKB_GSO_TCPV6 << NETIF_F_GSO_SHIFT)

	/* List of features with software fallbacks. */
#define NETIF_F_GSO_SOFTWARE	(NETIF_F_TSO | NETIF_F_TSO_ECN | NETIF_F_TSO6)


#define NETIF_F_GEN_CSUM	(NETIF_F_NO_CSUM | NETIF_F_HW_CSUM)
#define NETIF_F_V4_CSUM		(NETIF_F_GEN_CSUM | NETIF_F_IP_CSUM)
#define NETIF_F_V6_CSUM		(NETIF_F_GEN_CSUM | NETIF_F_IPV6_CSUM)
#define NETIF_F_ALL_CSUM	(NETIF_F_V4_CSUM | NETIF_F_V6_CSUM)
	int			ifindex;

	unsigned long		state;

    /* The device initialization function. Called only once. */
    int			(*init)(struct net_device *dev);

	/* Net device features */
	unsigned long		features;

	struct net_device_stats	stats;

	/* Hardware header description */
	const struct header_ops *header_ops;

    unsigned int		flags;	/* interface flags (a la BSD)	*/

        unsigned short          priv_flags; /* Like 'flags' but invisible to userspace. */

	unsigned char		operstate; /* RFC2863 operstate */
	unsigned char		link_mode; /* mapping policy to operstate */

	unsigned		mtu;	/* interface MTU value		*/
	unsigned short		type;	/* interface hardware type	*/

	/* extra head- and tailroom the hardware may need, but not in all cases
	 * can this be guaranteed, especially tailroom. Some cases also use
	 * LL_MAX_HEADER instead to allocate the skb.
	 */
	unsigned short		needed_headroom;
	unsigned short		needed_tailroom;

	/* Interface address info. */
	unsigned char		perm_addr[MAX_ADDR_LEN]; /* permanent hw address */
	unsigned char		addr_len;	/* hardware address length	*/

	spinlock_t		addr_list_lock;

	struct wireless_dev	*ieee80211_ptr;	/* IEEE 802.11 specific data,
						   assign before registering */

	unsigned long		last_rx;	/* Time of last Rx	*/

	unsigned char		broadcast[MAX_ADDR_LEN];	/* hw bcast add	*/
#if Make_USB != REALTEK_RTL8188CU
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
	struct netdev_queue	*_tx ____cacheline_aligned_in_smp;
#endif
#endif

	/* Number of TX queues allocated at alloc_netdev_mq() time  */
	unsigned int		num_tx_queues;

	spinlock_t		tx_global_lock;

    struct dev_addr_list	*mc_list;	/* Multicast mac addresses	*/
    int			mc_count;	/* Number of installed mcasts	*/

    U08		        dev_addr[MAX_ADDR_LEN];	/* hw address, (before bcast 
    						because most packets are unicast) */

    struct net_device_stats* (*get_stats)(struct net_device *dev);
    
//#ifdef WIRELESS_EXT
#ifdef CONFIG_WIRELESS_EXT
	/* List of functions to handle Wireless Extensions (instead of ioctl).
	 * See <net/iw_handler.h> for details. Jean II */
	const struct iw_handler_def *	wireless_handlers;

    int 		wireless_event_sock;
#else
#error "CONFIG_WIRELESS_EXT is not defined"
#endif

    void			*priv;	/* pointer to private data	*/
    int			(*hard_start_xmit) (struct sk_buff *skb,
    					    struct net_device *dev);
	/* These may be needed for future network-power-down code. */
	unsigned long		trans_start;	/* Time (in jiffies) of last Tx	*/

/*
 * refcnt is a very hot point, so align it on SMP
 */
	/* Number of references to this device */
	atomic_t		refcnt ____cacheline_aligned_in_smp;

    S32			(*hard_receive) (struct net_device *dev);
    
	/* register/unregister state machine */
	enum { NETREG_UNINITIALIZED=0,
	       NETREG_REGISTERED,	/* completed register_netdevice */
	       NETREG_UNREGISTERING,	/* called unregister_netdevice */
	       NETREG_UNREGISTERED,	/* completed unregister todo */
	       NETREG_RELEASED,		/* called free_netdev */
	} reg_state;

	/* Called after device is detached from network. */
	void			(*uninit)(struct net_device *dev);
	/* Called after last user reference disappears. */
	void			(*destructor)(struct net_device *dev);

    /* Pointers to interface service routines.	*/
    S32			(*open)(struct net_device *dev);
    S32			(*stop)(struct net_device *dev);

#define HAVE_MULTICAST			 
	void			(*set_multicast_list)(struct net_device *dev);
#define HAVE_SET_MAC_ADDR  		 
    S32			(*set_mac_address)(struct net_device *dev,
						   U08 *addr);
#define HAVE_VALIDATE_ADDR
	int			(*validate_addr)(struct net_device *dev);
#define HAVE_PRIVATE_IOCTL
	int			(*do_ioctl)(struct net_device *dev,
					    struct ifreq *ifr, int cmd);

#define HAVE_CHANGE_MTU
	int			(*change_mtu)(struct net_device *dev, int new_mtu);

    S32			(*get_mac_address)(struct net_device *dev,
						   U08 *addr);
#define HAVE_TX_TIMEOUT
    void			(*tx_timeout) (struct net_device *dev);
    
    BOOL        boolConnected;
    
    //for DHCP control
    U32         u32Dhcp;
    
    //ip info
    U32         u32DefaultIp;
    U32         u32SubnetMask;
    U32         u32GatewayAddr;
    U32 		u32DnsAddr[2];

    //wlan API
    S32			(*wlan_scan)(struct net_device *dev, struct wlan_ap_table_s * ap_table);
    S32			(*wlan_specificSSID_scan)(struct net_device *dev, U08* ssid, struct wlan_ap_table_s * ap_table);
#if 0
    S32			(*wlan_connect)(struct net_device *dev, U08* ssid, U08 key_mgmt, U08 auth_alg, U08 key_index, U08* wep_key, U08* psk);
#else
    S32			(*wlan_connect)(struct net_device *dev, U08* ssid);
#endif
    S32			(*wlan_disconnect)(struct net_device *dev);
#define mpx_AdHoc               1
#define mpx_Infrastructure    2
    S32			(*wlan_setmode)(struct net_device *dev, U32 mode);
    S32			(*wlan_info)(struct net_device *dev);    

#define mpx_AuthShared     0x4000
#define mpx_WEPDisable     0x8000
    S32			(*wlan_setWEP)(struct net_device *dev, U32 flags, U08 key_index, U08* wep_key);
    
    void        (*wlan_custom_proc)(void* pData);

	u16			(*select_queue)(struct net_device *dev,
						struct sk_buff *skb);

	/* class/net/name entry */
	struct device		dev;
};

static inline void *netdev_priv(const struct net_device *dev)
{
	return dev->priv;
}

struct dev_addr_list
{
	struct dev_addr_list	*next;
	BYTE		da_addr[MAX_ADDR_LEN];
	BYTE		da_addrlen;
	BYTE		da_synced;
	int			da_users;
	int			da_gusers;
};

#define dev_mc_list	dev_addr_list
#define dmi_addr	da_addr
#define dmi_addrlen	da_addrlen
#define dmi_users	da_users
#define dmi_gusers	da_gusers

void NetLinkStateEventSet(struct net_device *dev);
void rfc2863_policy(struct net_device *dev);
static inline void netif_dormant_on(struct net_device *dev)
{
	if (!(dev->flags & IFF_DORMANT))
    {
        dev->flags |= IFF_DORMANT;
        rfc2863_policy(dev);
        NetLinkStateEventSet(dev);
    }
}

static inline void netif_dormant_off(struct net_device *dev)
{
	if ((dev->flags & IFF_DORMANT))
    {
        dev->flags &= ~IFF_DORMANT;
        rfc2863_policy(dev);
        NetLinkStateEventSet(dev);
    }
}

static inline int netif_dormant(const struct net_device *dev)
{
    if (dev->flags & IFF_DORMANT)
    {
        //MP_DEBUG("netif_dormant TRUE");
        return TRUE;
    }
    else
    {
        //MP_DEBUG("netif_dormant FALSE");
        return FALSE;
    }
}


static inline int netif_running(const struct net_device *dev)
{
#if	Make_USB
	return test_bit(__LINK_STATE_START, &dev->state);
#else
	return TRUE;
#endif
}

static inline int netif_oper_up(const struct net_device *dev) 
{
	return (dev->operstate == IF_OPER_UP ||
		dev->operstate == IF_OPER_UNKNOWN /* backward compat */);
}

/**
 *	netif_device_present - is device available or removed
 *	@dev: network device
 *
 * Check if device has not been removed from system.
 */
static inline int netif_device_present(struct net_device *dev)
{
	return test_bit(__LINK_STATE_PRESENT, &dev->state);
}
#endif	/* _NET_DEVICE_H */

