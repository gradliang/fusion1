// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE 0

#include	"UtilTypeDef.h"
#include    "linux/usb.h"
#include 	"devio.h"
#include    "taskid.h"
#include    "wlan_sys.h"
#include    "os_mp52x.h"
#include    "mpTrace.h"

#include	"net_device.h"
#include	"list_mpx.h"
#include	"linux/netdevice.h"
#include	"linux/completion.h"
#include "net_nic.h"
#include	"dm9621.h"

#ifndef SKB_OPT
#error "Error: SKB_OPT must be defined"
#endif

/* control requests */
#define DM_READ_REGS	0x00
#define DM_WRITE_REGS	0x01
#define DM_READ_MEMS	0x02
#define DM_WRITE_REG	0x03
#define DM_WRITE_MEMS	0x05
#define DM_WRITE_MEM	0x07

/* registers */
#define DM_NET_CTRL	0x00
#define DM_RX_CTRL	0x05
#define DM_SHARED_CTRL	0x0b
#define DM_SHARED_ADDR	0x0c
#define DM_SHARED_DATA	0x0d	/* low + high */
#define DM_WAKEUP_CTRL  0x0f
#define DM_PHY_ADDR	0x10	/* 6 bytes */
#define DM_MCAST_ADDR	0x16	/* 8 bytes */
#define DM_GPR_CTRL	0x1e
#define DM_GPR_DATA	0x1f
#define DM_SMIREG       0x91

#define DM_MODE9620     0x80

#define DM_MCAST_SIZE	8
#define USB_MAX_EP_INT_BUFFER		64

struct net_device* dmfe_dev = NULL;
unsigned char ghashtable[DM_MCAST_SIZE];
unsigned char gusbethernet_is_link_up = 0;
unsigned char gusbethernet_initialized = 0;
//unsigned char hardwarelikeup = 0;
//#define DM9621_TOTAL_RECV_BULK 8
#define DM9621_TOTAL_RECV_BULK 20
struct urb *ginturb=NULL;
struct urb *grecvurb[DM9621_TOTAL_RECV_BULK];

/* endpoints */
#define EP_DATA_IN  1
#define EP_DATA_OUT 2
#define EP_INT_IN   3

extern BYTE myethaddr[6];
extern int usb_sema;

//Kevin Add this
typedef struct recv_net_packet RECT_NET_PACKET;
struct recv_net_packet
{
	RECT_NET_PACKET *next;
	ST_NET_PACKET *packetdata;
};

struct dm9621_statistics dm9621_stat;

/* Generic MII registers. */

#define MII_BMCR            0x00        /* Basic mode control register */
#define MII_BMSR            0x01        /* Basic mode status register  */
#define MII_PHYSID1         0x02        /* PHYS ID 1                   */
#define MII_PHYSID2         0x03        /* PHYS ID 2                   */
#define MII_ADVERTISE       0x04        /* Advertisement control reg   */
#define MII_LPA             0x05        /* Link partner ability reg    */
#define MII_EXPANSION       0x06        /* Expansion register          */
#define MII_CTRL1000        0x09        /* 1000BASE-T control          */
#define MII_STAT1000        0x0a        /* 1000BASE-T status           */
#define MII_ESTATUS	    0x0f	/* Extended Status */
#define MII_DCOUNTER        0x12        /* Disconnect counter          */
#define MII_FCSCOUNTER      0x13        /* False carrier counter       */
#define MII_NWAYTEST        0x14        /* N-way auto-neg test reg     */
#define MII_RERRCOUNTER     0x15        /* Receive error counter       */
#define MII_SREVISION       0x16        /* Silicon revision            */
#define MII_RESV1           0x17        /* Reserved...                 */
#define MII_LBRERROR        0x18        /* Lpback, rx, bypass error    */
#define MII_PHYADDR         0x19        /* PHY address                 */
#define MII_RESV2           0x1a        /* Reserved...                 */
#define MII_TPISTATUS       0x1b        /* TPI status for 10mbps       */
#define MII_NCONFIG         0x1c        /* Network interface config    */

/* Advertisement control register. */
#define ADVERTISE_SLCT          0x001f  /* Selector bits               */
#define ADVERTISE_CSMA          0x0001  /* Only selector supported     */
#define ADVERTISE_10HALF        0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL     0x0020  /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL        0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF     0x0040  /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF       0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE    0x0080  /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL       0x0100  /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM 0x0100  /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4      0x0200  /* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP     0x0400  /* Try for pause               */
#define ADVERTISE_PAUSE_ASYM    0x0800  /* Try for asymetric pause     */
#define ADVERTISE_RESV          0x1000  /* Unused...                   */
#define ADVERTISE_RFAULT        0x2000  /* Say we can detect faults    */
#define ADVERTISE_LPACK         0x4000  /* Ack link partners response  */
#define ADVERTISE_NPAGE         0x8000  /* Next page bit               */

#define ADVERTISE_FULL (ADVERTISE_100FULL | ADVERTISE_10FULL | \
			ADVERTISE_CSMA)
#define ADVERTISE_ALL (ADVERTISE_10HALF | ADVERTISE_10FULL | \
                       ADVERTISE_100HALF | ADVERTISE_100FULL)
#define BMCR_RESET              0x8000  /* Reset the DP83840           */
RECT_NET_PACKET *free_recv_net_packet_pool_head = NULL;
RECT_NET_PACKET *free_recv_net_packet_pool_tail = NULL;
RECT_NET_PACKET *in_recv_net_packet_pool_head = NULL;
RECT_NET_PACKET *in_recv_net_packet_pool_tail = NULL;
RECT_NET_PACKET netrecvpool[64];
unsigned int txcount = 0;
unsigned int rxcount = 0;
#define USB_MAX_RX_SIZE						1600
#define DM_TIMEOUT	1000

static int mp_usb_init();
void dm_int_urb_complete(struct urb *urb/*, struct pt_regs *pt_regs*/);
static void dm9621_RecvTask(struct urb *urb);

static int dm_write_reg(/*struct usbnet *dev*/struct usb_device *udev, u16 reg, u16 value)
{
	//devdbg(dev, "dm_write_reg() reg=0x%02x, value=0x%02x", reg, value);
	int ret;
	ret= usb_control_msg(udev,
			       usb_sndctrlpipe(udev, 0),
			       DM_WRITE_REG,
			       USB_DIR_OUT | USB_TYPE_VENDOR ,
			       value, reg, NULL, 0, 1000 * HZ);

	return ret;
}

static int dm_write(struct usb_device *udev, u8 reg, u16 length, void *data)
{
	int ret;
	ret= usb_control_msg(udev,
			       usb_sndctrlpipe(udev, 0),
			       DM_WRITE_REGS,
			       USB_DIR_OUT | USB_TYPE_VENDOR,
			       0, reg, data, length, 1000 * HZ);
	return ret;
}



static int dm_read(/*struct usbnet *dev*/struct usb_device *udev, u16 reg, u16 length, void *data)
{
	//devdbg(dev, "dm_read() reg=0x%02x length=%d", reg, length);
	int ret ;
	ret  = usb_control_msg(udev,
			       usb_rcvctrlpipe(udev, 0),
			       DM_READ_REGS,
			       USB_DIR_IN | USB_TYPE_VENDOR ,
			       0, reg, data, length, 1000 * HZ);
	return ret;
}

static int dm_read_reg(struct usb_device *udev, u8 reg, u8 *value)
{
	return dm_read(udev, reg, 1, value);
}

//unsigned char ethernetrecvcommandcomplete=0;
static void dm9621RxComplete(struct urb *urb)
{
	const u8 *buffer;
	unsigned int length;
	ST_NET_PACKET *inPacket;
	RECT_NET_PACKET *inrecvpacket;
	rxcount --;
	//mpDebugPrint("RTUSBBulkRxComplete a");
	switch (urb->status) {
	case 0:
		break;
	case -ESHUTDOWN:
	case -EINVAL:
	case -ENODEV:
	case -ENOENT:
	case -ECONNRESET:
	case -EPIPE:
		mpDebugPrint("error %d\n", urb->status);
		return;
	default:
		mpDebugPrint("error %d\n", urb->status);
		goto resubmit;
	}

	if( (length = urb->actual_length) > 0 )
	{
		void *newbuf = usb_buffer_alloc(urb->dev, USB_MAX_RX_SIZE, GFP_KERNEL,
							  &urb->transfer_dma);
        if (!newbuf)
        {
            dm9621_stat.rx_dropped++;
            goto resubmit;                      /* out of buffer */
        }

		buffer = urb->transfer_buffer;
		urb->transfer_buffer = newbuf;

		inPacket = NetNewPacket(FALSE);

        if (!inPacket)
        {
            usb_buffer_free(urb->dev, USB_MAX_RX_SIZE, newbuf,
							  &urb->transfer_dma);
            urb->transfer_buffer = buffer;
            dm9621_stat.rx_dropped++;
            goto resubmit;                      /* out of buffer */
        }

		//inPacket = (ST_NET_PACKET *) (buffer - sizeof(ST_HEADER));
        inPacket->Net.u08NetIndex = NIC_INDEX_ETHER;
        inPacket->Net.u16PayloadSize = length- 4 - ETHERNET_HEADER_SIZE;

        NetFreePacketMem(inPacket);

        inPacket->Net.data = buffer+4;
        inPacket->Net.head = buffer;

        U08* dstMac = (U08*)NET_PACKET_ETHER(inPacket);
        BOOL bcast = NetMacAddrComp(dstMac, NetBroadcastMacGet());

        if (!ArpPacketReceive2(inPacket))
        {
            dm9621_stat.rx_pkts++;
            if (bcast)
                dm9621_stat.rx_bcast++;
            else
                dm9621_stat.rx_ucast++;
            goto resubmit;
        }

		SemaphoreWait(CFETHERNET_MCARD_SEMA);
		if( free_recv_net_packet_pool_head )
		{
			inrecvpacket = free_recv_net_packet_pool_head;
			free_recv_net_packet_pool_head = free_recv_net_packet_pool_head->next;
			if( free_recv_net_packet_pool_head == NULL )
				free_recv_net_packet_pool_tail = NULL;
			inrecvpacket->next = NULL;
			inrecvpacket->packetdata = inPacket;
			if( in_recv_net_packet_pool_tail == NULL )
			{
				in_recv_net_packet_pool_head = in_recv_net_packet_pool_tail = inrecvpacket;
			}
			else
			{
				in_recv_net_packet_pool_tail->next = inrecvpacket;
				in_recv_net_packet_pool_tail = inrecvpacket;
			}

			SemaphoreRelease(CFETHERNET_MCARD_SEMA);
			
			dm9621_stat.rx_pkts++;
			if (bcast)
				dm9621_stat.rx_bcast++;
			else
				dm9621_stat.rx_ucast++;

			EventSet(ETHERNET_RECV_EVENT,1);
		}
		else
		{
			SemaphoreRelease(CFETHERNET_MCARD_SEMA);
			mpDebugPrint("%S::#############out of buffers##############",__FUNCTION__);
			NetFreePacket(inPacket);
			BREAK_POINT();
            dm9621_stat.rx_dropped++;
		}
	}


resubmit:

	if( gusbethernet_initialized )
		usb_submit_urb(urb, GFP_ATOMIC);

	//usb_free_urb(urb);
}

static void dm9621TxComplete(struct urb *urb)
{
	NetFreePacket((ST_NET_PACKET *)urb->context);
	txcount--;
	usb_free_urb(urb);

}

static int dm9621_start_xmit(ST_NET_PACKET *packet, struct net_device *dev)
{
	//board_info_t *db = (board_info_t *)dev->priv;
    struct usb_interface *intf = getUsbInterface();
	struct usb_device *udev = interface_to_usbdev(intf);

	int ret = 0;

    U08 *data_ptr;
	unsigned int len;
    data_ptr = NET_PACKET_ETHER(packet);
    len = (packet->Net.len);
	txcount++;
	struct urb *urb;
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		mpDebugPrint("allocate memory fail 1");
		ret = -ENOMEM;
        MP_ASSERT(0);
		//goto out;
		return 1;
	}

	data_ptr[-2]= len & 0xff;
	data_ptr[-1]= len >> 8 ;
//	mmcp_memcpy((unsigned char *) (buffer+2),(unsigned char *) data_ptr,len);
	usb_fill_bulk_urb(urb,udev,usb_sndbulkpipe(udev, EP_DATA_OUT),data_ptr-2,len+2,dm9621TxComplete,packet);

	if((ret = usb_submit_urb(urb, GFP_ATOMIC))!=0)
	{
		mpDebugPrint("Submit Tx URB failed %d\n", ret);
		usb_free_urb(urb);
		return 1;
	}
	//dm9621_RecvTask();
	return 0;

}

static void dm9621_RecvTask(struct urb *urb)
{
    struct usb_interface *intf = getUsbInterface();
	struct usb_device *udev = interface_to_usbdev(intf);

    int ret,i;
	//struct urb *urb;
	unsigned char *buffer;

	rxcount++;
	{
/*
		urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!urb) {
			mpDebugPrint("allocate memory fail 3");
			ret = -ENOMEM;
			MP_ASSERT(0);
			return;
		}
*/
		buffer = usb_buffer_alloc(udev, USB_MAX_RX_SIZE, GFP_KERNEL,
							  &urb->transfer_dma);
resubmit_urb:
		usb_fill_bulk_urb(urb,udev,usb_rcvbulkpipe(udev, EP_DATA_IN),buffer,USB_MAX_RX_SIZE,dm9621RxComplete,NULL);
		ret = usb_submit_urb(urb, GFP_ATOMIC);
		if (ret) {
			mpDebugPrint("error resubmit urb %p %d\n", urb, ret);
			goto resubmit_urb;
		}
	}
}

struct usb_ctrlrequest {
	u8 bRequestType;
	u8 bRequest;
	unsigned short wValue;
	unsigned short wIndex;
	unsigned short wLength;
} __attribute__ ((packed));
#if 0
static inline void usb_fill_control_urb(struct urb *urb,
					struct usb_device *dev,
					unsigned int pipe,
					unsigned char *setup_packet,
					void *transfer_buffer,
					int buffer_length,
					usb_complete_t complete_fn,
					void *context)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->setup_packet = setup_packet;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
}
#endif

struct usb_ctrlrequest kreq1;

static void dm_write_async_callback(struct urb *urb)
{
	struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)urb->context;

	if (urb->status < 0)
		mpDebugPrint("dm_write_async_callback() failed with %d\n",urb->status);

}

static void dm_write_async_helper(/*struct usbnet *dev,*/ u8 reg, u8 value,
				  u16 length, void *data)
{
    struct usb_interface *intf = getUsbInterface();
	struct usb_device *udev = interface_to_usbdev(intf);

	struct usb_ctrlrequest *req;
	int status;
	//mpDebugPrint("length %x swap %x",length,byte_swap_of_word(length));
	//mpDebugPrint("value %x swap %x",value,byte_swap_of_word(value));
	req = &kreq1;
	req->bRequestType = USB_DIR_OUT | USB_TYPE_VENDOR /*| USB_RECIP_DEVICE*/;
	req->bRequest = length ? DM_WRITE_REGS : DM_WRITE_REG;
	req->wValue = value;//byte_swap_of_word(value);//cpu_to_le16(value);
	req->wIndex = reg;//byte_swap_of_word(reg);//cpu_to_le16(reg);
	req->wLength = length;//cpu_to_le16(length);
#if 0
	usb_fill_control_urb(urb, dev->udev,
			     usb_sndctrlpipe(dev->udev, 0),
			     (void *)req, data, length,
			     dm_write_async_callback, req);
#endif
	if( length )
	{
		status = usb_control_msg(udev,
					   usb_sndctrlpipe(udev, 0),
					   DM_WRITE_REGS,
					   USB_DIR_OUT | USB_TYPE_VENDOR /*|USB_RECIP_DEVICE*/,
					   value, reg, data, length, 1000 * HZ);
	}
	else
	{
		status = usb_control_msg(udev,
					   usb_sndctrlpipe(udev, 0),
					   DM_WRITE_REG,
					   USB_DIR_OUT | USB_TYPE_VENDOR /*|USB_RECIP_DEVICE*/,
					   value, reg, data, length, 1000 * HZ);

	}

	if (status < 0) {
		mpDebugPrint("Error submitting the control message: status=%d");
	}

}

static void dm_write_reg_async(/*struct usbnet *dev,*/ u8 reg, u8 value)
{
	//mpDebugPrint("dm_write_reg_async() reg=0x%02x value=0x%02x",reg, value);

	dm_write_async_helper(reg, value, 0, NULL);
}

static void dm_write_async(/*struct usbnet *dev, */u8 reg, u16 length, void *data)
{
	//mpDebugPrint("KKK dm_write_async() reg=0x%02x length=%d", reg, length);

	dm_write_async_helper(reg, 0, length, data);
}

static void dm9620_set_multicast(void)
{
	//struct usbnet *dev = netdev_priv(net);
	/* We use the 20 byte dev->data for our 8 byte filter buffer
	 * to avoid allocating memory that is tricky to free later */
    struct usb_interface *intf = getUsbInterface();
	struct usb_device *udev = interface_to_usbdev(intf);

	u8 *hashes = ghashtable;
	u8 rx_ctl = 0x31;

	memset(hashes, 0x00, DM_MCAST_SIZE);
	hashes[DM_MCAST_SIZE - 1] |= 0x80;	/* broadcast address */
/*
	if (net->flags & IFF_PROMISC) {
		rx_ctl |= 0x02;
	} else if (net->flags & IFF_ALLMULTI || net->mc_count > DM_MAX_MCAST) {
		rx_ctl |= 0x04;
	} else if (net->mc_count) {
		struct dev_mc_list *mc_list = net->mc_list;
		int i;

		for (i = 0; i < net->mc_count; i++, mc_list = mc_list->next) {
			u32 crc = ether_crc(ETH_ALEN, mc_list->dmi_addr) >> 26;
			hashes[crc >> 3] |= 1 << (crc & 0x7);
		}
	}
*/
	dm_write_async(DM_MCAST_ADDR, DM_MCAST_SIZE, hashes);
	dm_write_reg_async(DM_RX_CTRL, rx_ctl);
}

static int dm_write_shared_word(int phy, u8 reg, u16 value)
{
	int ret, i;
    struct usb_interface *intf = getUsbInterface();
	struct usb_device *udev = interface_to_usbdev(intf);

	//mutex_lock(&dev->phy_mutex);

	ret = dm_write(udev, DM_SHARED_DATA, 2, &value);
	if (ret < 0)
	{
		mpDebugPrint("dm_write\n");
		goto dm_write_shared_word_out;
	}
	udelay(20);
	ret = dm_write_reg(udev, DM_SHARED_ADDR, phy ? (reg | 0x40) : reg);
	udelay(20);
	ret = dm_write_reg(udev, DM_SHARED_CTRL, phy ? 0x0a : 0x02);
	for (i = 0; i < 20; i++) {
		u8 tmp;

		udelay(10);
		ret = dm_read_reg(udev, DM_SHARED_CTRL, &tmp);
		if (ret < 0)
			goto dm_write_shared_word_out;

		/* ready */
		if ((tmp & 1) == 0)
			break;
	}

	if (i == 20) {
		mpDebugPrint("%s write timed out!", phy ? "phy" : "eeprom");
		ret = -1;
		goto dm_write_shared_word_out;
	}

	dm_write_reg(udev, DM_SHARED_CTRL, 0x0);

dm_write_shared_word_out:
	return ret;
}
static int dm_read_shared_word(int phy, u8 reg, u16 *value)
{
	int ret, i;
    struct usb_interface *intf = getUsbInterface();
	struct usb_device *udev = interface_to_usbdev(intf);

	//mutex_lock(&dev->phy_mutex);

	dm_write_reg(udev, DM_SHARED_ADDR, phy ? (reg | 0x40) : reg);
	dm_write_reg(udev, DM_SHARED_CTRL, phy ? 0xc : 0x4);

	for (i = 0; i < DM_TIMEOUT; i++) {
		u8 tmp;

		udelay(1);
		ret = dm_read_reg(udev, DM_SHARED_CTRL, &tmp);
		if (ret < 0)
			goto out;

		/* ready */
		if ((tmp & 1) == 0)
			break;
	}

	if (i == DM_TIMEOUT) {
		mpDebugPrint("%s read timed out!", phy ? "phy" : "eeprom");
		ret = -1;
		goto out;
	}

	dm_write_reg(udev, DM_SHARED_CTRL, 0x0);
	ret = dm_read(udev, DM_SHARED_DATA, 2, value);


 out:
	//mutex_unlock(&dev->phy_mutex);
	return ret;
}

static int dm9620_mdio_read(int phy_id, int loc)
{
    struct usb_interface *intf = getUsbInterface();
	struct usb_device *udev = interface_to_usbdev(intf);


	//__le16 res;
	u16 res;
/*
	if (phy_id) {
		devdbg(dev, "Only internal phy supported");
		return 0;
	}
*/
	dm_read_shared_word(1, loc, &res);

	//devdbg(dev,
	  //     "dm9620_mdio_read() phy_id=0x%02x, loc=0x%02x, returns=0x%04x",
	    //   phy_id, loc, le16_to_cpu(res));

	return res;//le16_to_cpu(res);
}

static void dm9620_mdio_write(int phy_id, int loc,u16 val)
{
	//__le16 res = cpu_to_le16(val);
	int mdio_val;
	u16 res;

	res = (val& 0xff00)>> 8 | (val& 0x00ff)<< 8;
	//res = val;


	if (phy_id) {
		mpDebugPrint("Only internal phy supported");
		return;
	}

	//devdbg(dev,"dm9620_mdio_write() phy_id=0x%02x, loc=0x%02x, val=0x%04x",
	  //     phy_id, loc, val);

	dm_write_shared_word(1, loc, res);
	udelay(1000);
	mdio_val = dm9620_mdio_read(phy_id, loc);

}

int dm9620_bind(int eWhichOtg)
{
	int ret = 0;
	unsigned char dev_addr[64];
    //struct usb_interface *intf = getUsbInterface();
    //struct net_device *netdev;
	//struct usb_device *udeva = usbnet_wifi_dev[eWhichOtg];
    struct usb_interface *intf = getUsbInterface();
	struct usb_device *udev = interface_to_usbdev(intf);
	u8 temp;
	int i;
	u8 opt;
    struct net_device *dev;
	int mdio_val;
	unsigned char *buffer;

    mp_usb_init();

	dmfe_dev = &NicArray[NIC_INDEX_ETHER];
	strcpy(dmfe_dev->name,"eth0");
    dev = dmfe_dev;

	dev->flags		= IFF_BROADCAST|IFF_MULTICAST;

    dev->hard_start_xmit = (void *) dm9621_start_xmit;
    //dev->hard_receive = (void *) dmfe_packet_receive;
	for( i= 0 ; i < 64 ; i++ )
	{
		if( i == 0 )
		{
			free_recv_net_packet_pool_head = &netrecvpool[i];
			free_recv_net_packet_pool_tail = &netrecvpool[i];
		}
		else
		{
			free_recv_net_packet_pool_tail->next = &netrecvpool[i];
			free_recv_net_packet_pool_tail = &netrecvpool[i];
		}
		netrecvpool[i].next = NULL;
		netrecvpool[i].packetdata = NULL;
	}

    if(UsbOtgCheckPlugOut(eWhichOtg)) // device plug out
    {
		mpDebugPrint("device unplugged");
        ret = -1;
        return ret;
    }
/*
	mpDebugPrint("usb_reset_device");
	ret = usb_reset_device(udev);
	if (ret) {
		mpDebugPrint("couldn't reset usb device. Error number %d\n",ret);
	}
	mpDebugPrint("end usb_reset_device");
*/
	ret = dm_write_reg(udev, DM_NET_CTRL, 1);
	udelay(20);
	/* Add V1.1, Enable auto link while plug in RJ45, Hank July 20, 2009*/
	ret = dm_write_reg(udev, 0xf4, 0x20); 

	memset(dev_addr,0,6);
	//ETH_ALEN

	if (dm_read(udev, DM_PHY_ADDR, 6, dev_addr) < 0) {
		mpDebugPrint("Error reading MAC address\n");
		ret = -ENODEV;
	}
#if 0//Remember to remark this
	dev_addr[0]=0x00;
	dev_addr[1]=0x90;
	dev_addr[2]=0xCC;
	dev_addr[3]=0xE5;
	dev_addr[4]=0x15;
	dev_addr[5]=0xB7;
#endif
	mpDebugPrint("%x %x %x %x %x %x",dev_addr[0],dev_addr[1],dev_addr[2],dev_addr[3],dev_addr[4],dev_addr[5]);
	memcpy(dev->dev_addr,dev_addr,6);
    memcpy(myethaddr,dev->dev_addr,6);
	//For DM9620 chip
	dm_write_async(DM_PHY_ADDR, ETH_ALEN, dev_addr);
	for(i=0;i<1000;i++)
		TaskYield();


	dm_write_reg(udev, 0x16, 0);
	dm_read_reg(udev, 0x16, &temp); 
	ret = dm_read_reg(udev, DM_SMIREG, &temp);

	if (ret<0) {
		mpDebugPrint("Error read SMI register\n");
	}
	//else
	//	mpDebugPrint("#########SMI register %x %x\n",temp,temp & DM_MODE9620);
	/* power up phy */
	dm_write_reg(udev, DM_GPR_CTRL, 1);
	dm_write_reg(udev, DM_GPR_DATA, 0);

	/* Init tx/rx checksum */
	dm_write_reg(udev, 0x31, 7);
	dm_write_reg(udev, 0x32, 3);
	//dm_write_async(DM_PHY_ADDR, ETH_ALEN, myethaddr);
	/* receive broadcast packets */
	//mpDebugPrint("receive broadcast packets ");
	dm9620_set_multicast();
	dm9620_mdio_write(0, MII_BMCR, BMCR_RESET);
	dm9620_mdio_write(0, 20, 0x880);
	ret = dm9620_mdio_read(0, 20);
	mdio_val = ((ret&0x00ff)<<8) | ((ret&0xff00)>>8);
	dm9620_mdio_write(0, MII_ADVERTISE,ADVERTISE_ALL | ADVERTISE_CSMA | ADVERTISE_PAUSE_CAP);

#if 0
	opt =0;
	if (dm_read_reg(udev, DM_WAKEUP_CTRL, &opt) < 0) {
		mpDebugPrint("Fail to Read DM_WAKEUP_CTRL");
	}
	else
	{
		mpDebugPrint("Read DM_WAKEUP_CTRL opt %02x",opt);
	}
#endif

	dev->ifindex = NIC_INDEX_ETHER;
	set_bit(__LINK_STATE_START, &dev->state);
	dev->flags |= IFF_UP;
	rfc2863_policy(dev);
	NetDriverUpEventSet(NIC_INDEX_ETHER);
	gusbethernet_initialized = 1;

	MP_DEBUG("Kevin aaaq");
	for(i=0;i<DM9621_TOTAL_RECV_BULK;i++)
	{
		grecvurb[i] = usb_alloc_urb(0, GFP_KERNEL);
		dm9621_RecvTask(grecvurb[i]);
	}
	MP_DEBUG("Kevin aaaqaaa");
	/*
	 * interrupt-in pipe
	 */

	struct urb *urb;
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		mpDebugPrint("usb_alloc_urb failed");
		errno = -ENOMEM;
		MP_ASSERT(0);
		return -1;
	}
	ginturb = urb;
	buffer = usb_buffer_alloc(udev, USB_MAX_EP_INT_BUFFER, GFP_KERNEL,
			&urb->transfer_dma);
	if (!buffer) {
		mpDebugPrint("usb_buffer_alloc failed");
		errno = -ENOMEM;
		MP_ASSERT(0);
		usb_free_urb(urb);
		ginturb = NULL;
		return -1;
	}

	usb_fill_int_urb(urb, udev, usb_rcvintpipe(udev, EP_INT_IN),buffer, USB_MAX_EP_INT_BUFFER,dm_int_urb_complete, NULL,4);
	if((ret = usb_submit_urb(urb, GFP_ATOMIC))!=0)
	{
		MP_ALERT("Submit Tx URB failed %d\n", ret);
	}

    return 0;
}

void Ethernet_RecvTask(void)
{

	unsigned int mevent;
	int ret;
    struct net_device *dev = dmfe_dev;
	RECT_NET_PACKET *inrecvpacket;
	ST_NET_PACKET *inPacket;
	int i;
	mpDebugPrint("Ethernet_RecvTask starts");
	while(1)
	{
		//if( gusbethernet_is_link_up )
			//ret = EventWaitWithTO(ETHERNET_RECV_EVENT,0xffffffff,OS_EVENT_OR,&mevent,500);
		//else
		ret = EventWait(ETHERNET_RECV_EVENT, 0xffffffff, OS_EVENT_OR, (void *)&mevent);


		//mpDebugPrint("in_recv_net_packet_pool_head %p",in_recv_net_packet_pool_head);

		for( i = 0 ; (i < 10) && in_recv_net_packet_pool_head ; i++ )
		{
			SemaphoreWait(CFETHERNET_MCARD_SEMA);

			inrecvpacket = in_recv_net_packet_pool_head;
			in_recv_net_packet_pool_head = in_recv_net_packet_pool_head->next;
			if( in_recv_net_packet_pool_head == NULL )
				in_recv_net_packet_pool_tail = NULL;
			inPacket = inrecvpacket->packetdata;
			inrecvpacket->next = NULL;
			inrecvpacket->packetdata = NULL;
			if( free_recv_net_packet_pool_tail == NULL )
			{
				free_recv_net_packet_pool_head = free_recv_net_packet_pool_tail = inrecvpacket;
			}
			else
			{
				free_recv_net_packet_pool_tail->next = inrecvpacket;
				free_recv_net_packet_pool_tail = inrecvpacket;
			}
			SemaphoreRelease(CFETHERNET_MCARD_SEMA);

			NetPacketReceive(inPacket);

		}
		
		if (in_recv_net_packet_pool_head)
			EventSet(ETHERNET_RECV_EVENT,1);

		TaskYield();

	}

}
void dm_int_urb_complete(struct urb *urb)
{
	int r;
	u8 *databuffer;
    struct net_device *dev=&NicArray[NIC_INDEX_ETHER];;
	int i,ret = 0;
    struct usb_interface *intf = getUsbInterface();
	struct usb_device *udev = interface_to_usbdev(intf);
    MP_ASSERT(!urb->status);
	switch (urb->status) {
	case 0:
		break;
	case -ESHUTDOWN:
	case -EINVAL:
	case -ENODEV:
	case -ENOENT:
	case -ECONNRESET:
	case -EPIPE:
		goto resubmit;
	default:
		goto resubmit;
		break;
	}
	if (urb->actual_length > 0) 
	{
		if ( gusbethernet_initialized )
		{
			databuffer = urb->transfer_buffer;
			if( databuffer[0] & 0x40 )
			{
				if( !gusbethernet_is_link_up )
				{
					gusbethernet_is_link_up = 1;
					netif_carrier_on(dev);
				}

			}
			else
			{
				if( gusbethernet_is_link_up )
				{
#if 0
					if( g_bAniFlag & ANI_SLIDE )
						xpgCb_SlideExit();
#endif
					netif_carrier_off(dev);
					gusbethernet_is_link_up = 0;
					tcpCloseAll();
				}

			}

		}
	}

resubmit:
	if( gusbethernet_initialized )
	{
		if((ret = usb_submit_urb(urb, GFP_ATOMIC))!=0)
		{
			MP_ALERT("Submit Tx URB failed %d\n", ret);
		}
		TaskYield();
	}

}


BYTE GetNetConfigP2PTestFlag()
{
   return 0;
}
int GetNetConfigTarget()
{
   return 0xc0a80164;//192.168.1.100
}

static int mp_usb_init()
{
    int ret;

    if(!usb_sema)
    {
        ret = (int)SemaphoreCreate(USB_CONTROL_SEMA, OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret == 0);
        SemaphoreWait(USB_CONTROL_SEMA);

        ret = (int)mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        if (ret <= 0)
            return -1;
        usb_sema = ret;
    }
    else
        SemaphorePolling(USB_CONTROL_SEMA);

    return 0;
}

void NetEthernetDongleUnplug()
{
    struct net_device *dev=dmfe_dev;
	int i;

	MP_DEBUG("Ethernet Dongle Uplug !!\n");

	netif_carrier_off(dev);
	clear_bit(__LINK_STATE_START, &dev->state);
	dev->flags &= ~IFF_UP;


	SemaphoreWait(CFETHERNET_MCARD_SEMA);

	tcpCloseAll();

	wifi_device_type = WIFI_USB_DEVICE_NONE;
	MP_DEBUG("Set gusbethernet_initialized to zero\n");
	gusbethernet_initialized = 0;
	gusbethernet_is_link_up = 0;
	//hardwarelikeup = 0;
	dmfe_dev = NULL;
	if(ginturb)
		usb_free_urb(ginturb);
	for(i=0;i<DM9621_TOTAL_RECV_BULK;i++)
	{
		MP_DEBUG("grecvurb[i] %p",grecvurb[i]);
		usb_free_urb(grecvurb[i]);
	}
	SemaphoreRelease(CFETHERNET_MCARD_SEMA);
	MP_DEBUG("rxcount %d",rxcount);
}
