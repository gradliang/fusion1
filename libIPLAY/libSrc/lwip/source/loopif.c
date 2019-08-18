#include <string.h>
#include "linux/types.h"
#include "lwip_config.h"
#include "typedef.h"
#include "lwip_opt.h"
#include "os.h"
#include "socket.h"
#include "net_ip.h"
#include "net_udp.h"
#include "net_netctrl.h"
#include "net_device.h"
#include "net_nic.h"
#include <linux/if_ether.h>


#if ENABLE_LOOPBACK
/**
 * Send an IP packet to be received on the same netif (loopif-like).
 *
 * @param destAddr the ip address to send the packet to (not used)
 */
int _loop_output(struct net_device   *_dev, ST_NET_PACKET *packet, U32 destAddr)
{
    struct net_device   *dev = _dev;
//     __asm__("break 100");
    if (!dev) {
#if (DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE)
        dev = &NicArray[NIC_INDEX_ETHER];
#else
        dev = &NicArray[NIC_INDEX_WIFI];
#endif
        packet->Net.dev = dev;
    }

    /* ether type is required later */
    U08* ptr = NET_PACKET_ETHER(packet);
    NetMacAddrCopy(ptr, NetLocalMACGet());      /* to self - required in NetBufferPacketReceive() */
    ptr += 12;
    NetPutW(ptr, ETHERNET_TYPE_IP);

    packet->Net.len = packet->Net.u16PayloadSize + ETHERNET_HEADER_SIZE;

    return netReceive(dev, (struct sk_buff *)packet);

}
#endif
