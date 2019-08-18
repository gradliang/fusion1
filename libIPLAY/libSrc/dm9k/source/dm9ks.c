#include "global612.h"
#include "mpTrace.h"
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include "netware.h"
#include <net_device.h>
#include "net_packet.h"
#include "net_nic.h"

#include "devio.h"
#include "..\..\mcard\INCLUDE\Mcard.h"
#include "..\..\mcard\INCLUDE\uti.h"
#include "..\..\mcard\INCLUDE\cf.h"
#include "UtilRegFile.h"

#include "peripheral.h"
#include "hal_gpio.h"
#include "dm9ks.h"
#include "taskid.h"

/*---------------------------------------------*/
//Kevin Add
#define DM9KS_REG05		0x30	/* SKIP_CRC/SKIP_LONG */ 
#define DM9KS_REGFF		0xA3	/* IMR */
//End Kevin
#define DM9KS_DISINTR               0x80
#define DM9KS_PHY                   0x40    /* PHY address 0x01 */
#define DM9KS_RX_INTR               0x01
#define DM9KS_TX_INTR               0x02
#define DM9KS_LINK_INTR             0x20

//struct net_device g_eth_dev;
//For MP650 
#define DM9KS_CS GPIO_FGPIO_35
#define OneBit 1
#define ETHERNET_DMA_READ	1
#define ETHERNET_DMA_WRITE	1
#define ETHERNET_MAX_PACKET_SIZE	1518
//#define DM9KS_CHIP_ENABLE GPIO_DATA_LOW
#define CF_CLOCK_KHZ		99000	// never bigger than 100MHz
//Kevin Add
volatile static U32 iobase = MC_CFCOM_BASE;
volatile static U32 cmdiobase = MC_CFATR_BASE;
static void dv9kiow(U08 index_reg);
static unsigned char ior(unsigned char reg);
void dmfe_init_dm9000( struct net_device *dev );
int dmfe_probe(struct net_device *dev);
int dmfe_open(struct net_device *dev);
static int dmfe_start_xmit(ST_NET_PACKET *, struct net_device *);
static void dmfe_interrupt(U32 param);
static int dmfe_packet_receive(struct net_device *dev);
static void dmfe_tx_done(unsigned long unused);
struct net_device* dmfe_dev = NULL;
u8 gethernet_is_link_up =0;
char setrecveventcount = 0;
char setarprecveventcount = 0;
char GetMACAddress(U08 *mac_addr);
//ZZ
void dmfeCmdWrite(U08 cmd);
static void iow(unsigned char reg, unsigned char value);
typedef struct recv_net_packet RECT_NET_PACKET;
struct recv_net_packet
{
	RECT_NET_PACKET *next;
	ST_NET_PACKET *packetdata;
};

RECT_NET_PACKET *free_recv_net_packet_pool_head = NULL;
RECT_NET_PACKET *free_recv_net_packet_pool_tail = NULL;
RECT_NET_PACKET *in_recv_net_packet_pool_head = NULL;
RECT_NET_PACKET *in_recv_net_packet_pool_tail = NULL;
RECT_NET_PACKET netrecvpool[256];
#define MAX_ARP_PACKET 64
RECT_NET_PACKET *free_recv_net_arppacket_pool_head = NULL;
RECT_NET_PACKET *free_recv_net_arppacket_pool_tail = NULL;
RECT_NET_PACKET *in_recv_net_arppacket_pool_head = NULL;
RECT_NET_PACKET *in_recv_net_arppacket_pool_tail = NULL;
RECT_NET_PACKET netarprecvpool[MAX_ARP_PACKET];


int total_new_packet = 0;
char bgetmacaddress = 0;
U08 gmacaddress[6];

typedef struct new_net_packet NEW_NET_PACKET;
struct new_net_packet
{
	RECT_NET_PACKET *next;
	ST_NET_PACKET *packetdata;
};
NEW_NET_PACKET *new_net_packet_pool_head = NULL;
NEW_NET_PACKET *new_net_packet_pool_tail = NULL;

//Original
#define MAX_ADDR_LEN                32        /* Largest hardware address length */
/* Structure/enum declaration ------------------------------- */
typedef struct board_info {
    U32 io_addr;            /* Register I/O base address */
    U32 io_data;            /* Data I/O address */
    U08 op_mode;            /* PHY operation mode */
    U08 io_mode;            /* 0:word, 2:byte */
    U08 device_wait_reset;        /* device state */
    U08 Speed;            /* current speed */
    U08 dev_addr[MAX_ADDR_LEN];    /* hw address, (before bcast because most packets are unicast) */
	S32 tx_pkt_cnt;
} board_info_t;

enum DM9KS_PHY_mode {
    DM9KS_10MHD   = 0, 
    DM9KS_100MHD  = 1, 
    DM9KS_10MFD   = 4,
    DM9KS_100MFD  = 5, 
    DM9KS_AUTO    = 8, 
};

typedef struct _RX_DESC
{
    U08 rxbyte;
    U08 status;
    U16 length;
}RX_DESC;

typedef union{
    U08 buf[4];
    RX_DESC desc;
} rx_t;

#define check_rx_ready(a)    ((a) == 0x01)
static S32 media_mode = DM9KS_100MFD;//DM9KS_AUTO;//DM9KS_100MFD;
extern BYTE myethaddr[6];
struct ethernetconfig* dm9netconfig = NULL;

#define dmfe_select(void) Gpio_Config2GpioFunc(DM9KS_CS,GPIO_OUTPUT_MODE,DM9KS_CHIP_ENABLE,OneBit)
/*
{
	Gpio_Config2GpioFunc(DM9KS_CS,GPIO_OUTPUT_MODE,DM9KS_CHIP_ENABLE,OneBit);
}
*/
#define dmfe_deselect(void) Gpio_Config2GpioFunc(DM9KS_CS,GPIO_OUTPUT_MODE,DM9KS_CHIP_DISABLE,OneBit)
/*
{
	Gpio_Config2GpioFunc(DM9KS_CS,GPIO_OUTPUT_MODE,DM9KS_CHIP_DISABLE,OneBit);
}
*/

/*
  Set DM9000A/DM9010 multicast address
*/

void CFEthernetInit(ST_MCARD_DEV * sDev)
{
	sDev->pbDescriptor = NULL;
	sDev->wMcardType = DEV_CF_ETHERNET_DEVICE;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = NULL;

}

static void dm9000_hash_table(struct net_device *dev)
{
    board_info_t *db = (board_info_t *)dev->priv;
    U16 i, oft;

    mpDebugPrint("%s===>",__FUNCTION__);
	if( bgetmacaddress )
	{
		memcpy(dev->dev_addr,gmacaddress,6);
	}
	else
	{
		dev->dev_addr[0] = 0x00;
		dev->dev_addr[1] = 0x02;
		dev->dev_addr[2] = 0x72;
		dev->dev_addr[3] = 0x73;
		dev->dev_addr[4] = 0x52;
		dev->dev_addr[5] = 0x04;
	}
	//mpDebugPrint(" Get systime %d",GetSysTime());
    DPrintf("my mac = %2x:%2x:%2x:\\-",
            dev->dev_addr[0],
            dev->dev_addr[1],
            dev->dev_addr[2]);
    DPrintf("%2x:%2x:%2x",
            dev->dev_addr[3],
            dev->dev_addr[4],
            dev->dev_addr[5]);
    memcpy(myethaddr,dev->dev_addr,6);
    for (i = 0, oft = 0x10; i < 6; i++, oft++){
        iow(oft, dev->dev_addr[i]);
        udelay(1);
    }

    db->dev_addr[0] = 0x00;
    db->dev_addr[1] = 0x00;
    db->dev_addr[2] = 0x00;
    db->dev_addr[3] = 0x00;
    db->dev_addr[4] = 0x00;
    db->dev_addr[5] = 0x00;
    db->dev_addr[6] = 0x00;
    db->dev_addr[7] = 0x80;

    /* Set Node address */
    for (i = 0, oft = 0x16; i < 8; i++, oft++){
        iow(oft, db->dev_addr[i]);
        udelay(1);
    }
}

void Dm9ksConfig(struct net_device *dev){
    dev->init = (void *) dmfe_probe;
    dev->open = (void *) dmfe_open;
    //dev->stop = dmfe_stop;
    dev->hard_start_xmit = (void *) dmfe_start_xmit;
    dev->hard_receive = (void *) dmfe_packet_receive;
    //dev->hard_int_handler = (void *) dmfe_interrupt;
    //dev->hard_int_enable = dmfe_int_enable;
}

int dmfe_probe(struct net_device *dev)
{
    struct board_info *db;    /* Point a board information structure */
    U16 dm9000_found = FALSE;
	U32 u32Dm9ksId;
	unsigned char iomode;
	DWORD oriclock; 
    //DMFE_DBUG(0, "dmfe_probe()",0);

	bgetmacaddress = GetMACAddress(gmacaddress);
	dm9netconfig= GetEtherNetConfig();
	mpDebugPrint("netconfig macaddr %x %x %x %x %x %x",dm9netconfig->macaddr[0],dm9netconfig->macaddr[1],dm9netconfig->macaddr[2],dm9netconfig->macaddr[3],dm9netconfig->macaddr[4],dm9netconfig->macaddr[5]);
	mpDebugPrint("netconfig dhcpflag %x",dm9netconfig->dhcpflag);
	mpDebugPrint("netconfig ipaddr %x",dm9netconfig->ipaddr);
	mpDebugPrint("netconfig netmask %x",dm9netconfig->netmask);
	mpDebugPrint("netconfig gateway %x",dm9netconfig->gateway);
	mpDebugPrint("netconfig p2ptestflag %x",dm9netconfig->p2ptestflag);
	mpDebugPrint("netconfig targetip %x",dm9netconfig->targetip);

		
	mpDebugPrint("bgetmacaddress %d",bgetmacaddress);
    //REG_POINT(pstSiu, SIU);
	oriclock = GetMcardClock();
	mpDebugPrint("oriclock %d",oriclock);
	SetMcardClock(CF_CLOCK_KHZ);
	mpDebugPrint("Ethernet::call CalculatePulseTimingforEthernet");
	CalculatePulseTimingforEthernet();
	mpDebugPrint("Ethernet::call Mcard_Ethernet_Active(INITIAL_MEMORY)");
	Mcard_Ethernet_Active(INITIAL_MEMORY);

    dmfe_select();
    TaskContextDeactive();
	mpDebugPrint("Set CMD to LOW for index mode");
	Gpio_Config2GpioFunc(DM9KS_CS,GPIO_OUTPUT_MODE,GPIO_DATA_LOW,1);
	Gpio_Config2GpioFunc(GPIO_FGPIO_11,GPIO_OUTPUT_MODE,GPIO_DATA_LOW,1);
	//sCfata->Data = 0x38;
	mpDebugPrint("Ethernet::@@@@@@@@Set iobase %x register index to 0x38 after 5 secs",iobase);
	//TimerDelay(5000);
	//mpDebugPrint("%x ",*((volatile U32*)iobase));
    //*((volatile unsigned char*)iobase) = 0x38;//<< 8) & 0xff00);//0x38 ;//
	*((volatile unsigned char*)cmdiobase) = DM9000_BUSCR;//0x38;//<< 8) & 0xff00);//0x38 ;//
	IODelay( 1000 );
	mpDebugPrint("Ethernet::Set CMD to HIGH for index mode");
	Gpio_Config2GpioFunc(DM9KS_CS,GPIO_OUTPUT_MODE,GPIO_DATA_LOW,1);
	Gpio_Config2GpioFunc(GPIO_FGPIO_11,GPIO_OUTPUT_MODE,GPIO_DATA_HIGH,1);
	//sCfata->ErrorFeature = 0x41;
	mpDebugPrint("Ethernet::@@@@@@@@Set iobase %x data to 0x41 after 5 secs",iobase);
    *((volatile unsigned char *)(iobase)) = 0x41;//sigWORDLittleEndian(0x41) & 0xffff;//0x4100;//
	dmfeCmdWrite(0x28);
    u32Dm9ksId = *((volatile unsigned char*)(iobase));//*((volatile unsigned char*)(iobase)) /*& 0x0000ffff*/;
	mpDebugPrint("Ethernet::Read the data from 0x28 is %x",u32Dm9ksId);
	dmfeCmdWrite(0x29);
    u32Dm9ksId |= *((volatile unsigned char*)(iobase)) << 8;//*((volatile unsigned char*)(iobase)) ;
	mpDebugPrint("Ethernet::Read the data from 0x29 is %x",u32Dm9ksId);
	dmfeCmdWrite(0x2a);
    u32Dm9ksId |= *((volatile unsigned char*)(iobase)) << 16;//*((volatile unsigned char*)(iobase)) ;
	mpDebugPrint("Ethernet::Read the data from 0x2a is %x",u32Dm9ksId);
	dmfeCmdWrite(0x2b);
    u32Dm9ksId |= *((volatile unsigned char*)(iobase)) << 24;//*((volatile unsigned char*)(iobase)) /*& 0x0000ffff*/;
	mpDebugPrint("Ethernet::Read the data from 0x2b is %x",u32Dm9ksId);
    TaskContextActive();
    DPrintf("[DM9KS] id = %x", u32Dm9ksId);
    if((u32Dm9ksId == DM9KS_ID) ) 
	{

        dm9000_found = TRUE;

        dev->priv = (void *) ext_mem_malloc(sizeof(struct board_info));
		mpDebugPrint("dev->priv %x",dev->priv);
        memset(dev->priv, 0, sizeof(struct board_info));
        db = (board_info_t *)dev->priv;
        dmfe_dev    = dev;
        db->io_addr  = iobase;
        db->io_data = iobase;

    }
    else
        DPrintf("<DM9KS> access fail");
    dmfe_deselect();
	SetMcardClock(oriclock);
    return dm9000_found ? 1:0;
}

unsigned char dmfeCmdRead()
{
	return *((volatile unsigned char*)(iobase));
}

void dmfeDataWrite(unsigned char data)
{
	*((volatile unsigned char *)iobase) = data;
}

void dmfeCmdWrite(U08 cmd)
{
#if (DEMO_PID==0)
	IODelay( 1 );
#endif
	Gpio_Config2GpioFunc(DM9KS_CS,GPIO_OUTPUT_MODE,GPIO_DATA_LOW,1);
	Gpio_Config2GpioFunc(GPIO_FGPIO_11,GPIO_OUTPUT_MODE,GPIO_DATA_LOW,1);
    *((volatile unsigned char*)cmdiobase) = cmd;
}

static void iow(unsigned char reg, unsigned char value)
{
    dmfeCmdWrite(reg);//CH kv_iow(reg);
    dmfeDataWrite(value);
}

/* 0531 */
static unsigned char ior(unsigned char reg)
{
    dmfeCmdWrite(reg);//CH (reg);
#if (DEMO_PID==0)
	IODelay( 1 );
#endif
	//udelay(10);
    return *((volatile unsigned char *)iobase);
}

static void phy_write(board_info_t *db, int reg, U16 value)
{
    /* Fill the phyxcer register into REG_0C */
    iow(DM9000_EPAR, DM9KS_PHY | reg);
    udelay(10);

    /* Fill the written data into REG_0D & REG_0E */
    iow(DM9000_EPDRL, (value & 0xff));
    udelay(10);
    iow(DM9000_EPDRH, ((value >> 8) & 0xff));
    udelay(10);

    iow(DM9000_EPCR, 0x8 | 0x2);    /* Issue phyxcer write command */
    udelay(500);
    iow(DM9000_EPCR, 0x8);    /* Clear phyxcer write command */
}


/* Set PHY operationg mode
*/
static void set_PHY_mode(board_info_t *db)
{
    U16 phy_reg0 = 0x1200;        /* Auto-negotiation & Restart Auto-negotiation */
    U16 phy_reg4 = 0x01e1;        /* Default flow control disable*/

	mpDebugPrint("##########set_PHY_mode %d#########",db->op_mode);
    if ( !(db->op_mode & DM9KS_AUTO) ) // op_mode didn't auto sense */
    { 
        switch(db->op_mode){
            case DM9KS_10MHD:  phy_reg4 = 0x21; 
                        phy_reg0 = 0x1000;
                        break;
            case DM9KS_10MFD:  phy_reg4 = 0x41; 
                        phy_reg0 = 0x1100;
                        break;
            case DM9KS_100MHD: phy_reg4 = 0x81; 
                   phy_reg0 = 0x3000;
                       break;
            case DM9KS_100MFD: phy_reg4 = 0x101; 
                   phy_reg0 = 0x3100;
                      break;
            default: 
                   break;
        }
    } 
    phy_write(db, 0, phy_reg0);
    phy_write(db, 4, phy_reg4);
}


/* 
	Initilize dm9000
*/
void dmfe_init_dm9000(struct net_device *dev)
{

	//struct net_device *net_dev;
    //netpool_mem_init(AR2524_MAX_NUM_BUFFERS,AR2524_MAX_NETPOOL_ALLOC_SIZE);//Kevin Mark AA
	u8 i;
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

	for( i= 0 ; i < MAX_ARP_PACKET ; i++ )
	{
		if( i == 0 )
		{
			free_recv_net_arppacket_pool_head = &netarprecvpool[i];
			free_recv_net_arppacket_pool_tail = &netarprecvpool[i];
		}
		else
		{
			free_recv_net_arppacket_pool_tail->next = &netarprecvpool[i];
			free_recv_net_arppacket_pool_tail = &netarprecvpool[i];
		}
		netarprecvpool[i].next = NULL;
		netarprecvpool[i].packetdata = NULL;
	}

    board_info_t *db = (board_info_t *) (dev->priv);
    dmfe_select();
    TaskContextDeactive();
    
    /* set the internal PHY power-on, GPIOs normal, and wait 2ms */
    iow(DM9000_GPR, 1);     /* Power-Down PHY */
    udelay(500);
    iow(DM9000_GPR, 0);    /* GPR (reg_1Fh)bit GPIO0=0 pre-activate PHY */
    udelay(20);        /* wait 2ms for PHY power-on ready */

    /* do a software reset and wait 20us */
    iow(DM9000_NCR, 3);
    udelay(100);        /* wait 20us at least for software reset ok */
    iow(DM9000_NCR, 3);    /* NCR (reg_00h) bit[0] RST=1 & Loopback=1, reset on */
    udelay(100);        /* wait 20us at least for software reset ok */

    /* I/O mode */
    db->io_mode = ior(DM9000_ISR) >> 6; /* ISR bit7:6 keeps I/O mode */

    /* Set PHY */
    db->op_mode = media_mode;

    set_PHY_mode(db);

    /* Program operating register */
    iow(DM9000_NCR, 0);
    udelay(1);
    iow(DM9000_TCR, 0);        /* TX Polling clear */
    udelay(1);
    iow(DM9000_BPTR, 0x3f);    /* Less 3kb, 600us */
    udelay(1);
	iow(DM9000_FCR, 0xff);	/* Flow Control */
	udelay(1);
    iow(DM9000_SMCR, 0);        /* Special Mode */
    udelay(1);
    /* clear TX status */
    iow(DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
    udelay(1);
    iow(DM9000_ISR, ISR_CLR_STATUS); /* Clear interrupt status */
    udelay(1);
    
    /* Added by jackal at 03/29/2004 */
#if defined(CHECKSUM)
    iow(DM9000_TCCR, 0x07);    /* TX UDP/TCP/IP checksum enable */
    iow(DM9000_RCSR, 0x02);    /*Receive checksum enable */
#endif

#if defined(ETRANS)
    iow(DM9000_ETXCSR, 0x83);
#endif
 
    /* Set address filter table */
    dm9000_hash_table(dev);

    /* Activate DM9000A/DM9010 */
    iow(DM9000_RCR, RCR_DIS_CRC | RCR_DIS_LONG | RCR_RXEN);
    udelay(1);
    /* Enable TX/RX/LINKCHANGE interrupt mask */
    iow(DM9000_IMR, IMR_PAR | IMR_PRM | IMR_LNKCHGI);
    udelay(1);

	db->tx_pkt_cnt = 0;

    TaskContextActive();
    dmfe_deselect();
}



/*
  Open the interface.
  The interface is opened whenever "ifconfig" actives it.
*/
int dmfe_open(struct net_device *dev)
{
    board_info_t *db = (board_info_t *)dev->priv;
    U08 reg_nsr;
    int i;
	unsigned int int_status;
	DRIVE *sDrv;
	STREAM *w_shandle;
	char filebuffer[128];
	U08 mac_addr[6];
	char *macstring,*tmpptr;
	unsigned char tmphex;
	BYTE  curDrvId;
	//mpDebugPrint("##########dmfe_open ===>");
    /* Initilize DM910X board */
	//Kevin Add for TMP
	m9ks_netpool_init();
    dmfe_init_dm9000(dev);
#if 0
	curDrvId = DriveCurIdGet();
	DriveChange(SD_MMC);
	sDrv = DriveGet(DriveCurIdGet());
	//sDrv = DriveGet(SD_MMC);//SD_MMC//NAND
	if (FS_SUCCEED == FileSearch(sDrv, "netconfig", "txt", E_FILE_TYPE))
	{
		w_shandle = FileOpen(sDrv);
		if(w_shandle)
		{
			FileRead(w_shandle,filebuffer,128);
			if( macstring = strstr(filebuffer,"mac:") )
			{
				for(i=0;i<6;i++)
				{
					tmpptr = macstring+4+3*i;
					*(tmpptr+2) = '\0';
					tmphex = strtol(tmpptr,NULL,16);
					mpDebugPrint("tmphex GGG %x",tmphex);
				}
			}
			FileClose(w_shandle);
		}
	}
	else
	{
		mpDebugPrint("Can not find netconfig.txt file in SD.");
	}
	DriveChange(curDrvId);
#endif

#if 0
    /* check link state and media speed */
    db->Speed =10;
    i=0;
    do{
		IntDisable();
        dmfe_select();
        reg_nsr = ior(0x1);
		//mpDebugPrint("reg_nsr %x",reg_nsr);
        if(reg_nsr & 0x40){ /* link OK!! */

            /* wait for detected Speed */
            mdelay(50);
			mpDebugPrint("link OK!! ###");
			mdelay(1);
			mpDebugPrint("link OK!! @@@");
            reg_nsr = ior(0x1);
			mpDebugPrint("reg_nsr %02x",reg_nsr);
			netif_carrier_on(dev);
            dmfe_deselect();
            IntEnable();
            
            if(reg_nsr & 0x80)
                db->Speed =10;
            else
                db->Speed =100;
			mpDebugPrint("db->Speed %d",db->Speed);
			dev->ifindex = NIC_INDEX_ETHER;
			gethernet_is_link_up = 1;
			mpDebugPrint("call NetLinkStateEventSet");
			NetLinkStateEventSet(dev);
            break;
            
        }
        i++;
        mdelay(10);
        IntEnable();
        dmfe_deselect();
    }while(i<3000);    /* wait 3 second  */
    
    if(i >= 3000)
        return 0;
	//Set INT service Routine
    dmfe_select();
#endif
	//EventSet(ETHERNET_RECV_EVENT,1);
    TaskContextDeactive();
	Gpio_Config2GpioFunc(GPIO_GPIO_3,GPIO_INPUT_MODE,0,1);
	Gpio_IntConfig(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3),GPIO_ACTIVE_HIGH,GPIO_LEVEL_TRIGGER);
	Gpio_IntCallbackFunRegister(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3),(void *) dmfe_interrupt);
	Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
/*
    Gpio_IntConfig(GPINT_CF_CARD_DETECT, GPIO_ACTIVE_HIGH, GPIO_LEVEL_TRIGGER);
    Gpio_IntCallbackFunRegister(GPINT_CF_CARD_DETECT, dmfe_interrupt);
    Gpio_IntEnable(GPINT_CF_CARD_DETECT);
*/
    TaskContextActive();
	//EventSet(ETHERNET_RECV_EVENT,1);
	//dmfe_deselect();
#if 0
	//Kevin Add for DHCP
	NetDriverUpEventSet();
	NetLinkStateEventSet(dev);
	//End Kevin
#endif

#if 0
    //mpx_GpioIntEnable(DM9KS_INT);
	mpDebugPrint("Config OK");
	mpDebugPrint("Read Int Reg");
	mdelay(1);
	mpDebugPrint("Read Int Reg");
    /* Disable all interrupt */
    iow(DM9000_IMR, DM9KS_DISINTR);
    
    /* Got DM9000A/DM9010 interrupt status */
    int_status = ior(DM9000_ISR);        /* Got ISR */

	mpDebugPrint("int_status %x",int_status);
	dmfe_packet_receive(dev);
#endif
    return 1;
}

/*
  DM9000 insterrupt handler
  receive the packet to upper layer, free the transmitted packet
*/
static void dmfe_interrupt(U32 param)
{
    U08 reg_nsr;
    U08 reg_save;
	DWORD oriclock;
    register U32 int_status;
    struct net_device *dev = dmfe_dev;
    board_info_t *db = (board_info_t *)dev->priv;
	oriclock = GetMcardClock();
	SetMcardClock(CF_CLOCK_KHZ);
	CalculatePulseTimingforEthernet();
	Mcard_Ethernet_Active(INITIAL_MEMORY);

    dmfe_select();
    
    reg_save = dmfeCmdRead();

    /* Disable all interrupt */
    iow(DM9000_IMR, DM9KS_DISINTR);
    
    /* Got DM9000A/DM9010 interrupt status */
    int_status = ior(DM9000_ISR);        /* Got ISR */
    iow(DM9000_ISR, int_status);        /* Clear ISR status */ 
    
    if(int_status & DM9KS_LINK_INTR)        /* Link status change */
	{
        //outRequest->u32Request |= NIC_REQUEST_LINK_STATE_CHANGED;
		mpDebugPrint("Link status change");
		//Kevin Add
        reg_nsr = ior(0x1);
		mpDebugPrint("reg_nsr %x",reg_nsr);
		if( reg_nsr & 0x40 )
		{
			mpDebugPrint("PHY Link UP %p",dmfe_dev);
            if(reg_nsr & 0x80)
                db->Speed =10;
            else
                db->Speed =100;
			mpDebugPrint("db->Speed %d",db->Speed);
			dev->ifindex = NIC_INDEX_ETHER;
			set_bit(__LINK_STATE_START, &dev->state);
			gethernet_is_link_up = 1;
			netif_carrier_on(dev);
			mpDebugPrint("call NetLinkStateEventSet");
			NetDriverUpEventSet(NIC_INDEX_ETHER);
			NetLinkStateEventSet(dev);

		}
		else if( reg_nsr == 0 )
		{
			mpDebugPrint("PHY Link DWON");
			netif_carrier_off(dev);
			gethernet_is_link_up = 0;
			tcpCloseAll();
		}



	}   
    if(int_status & DM9KS_RX_INTR)          /* receive packet */
    {
		//mpDebugPrint("receive packet ");
		dmfe_packet_receive(dev);
    }
    
    if(int_status & DM9KS_TX_INTR)          /* transmit packet check */
	{
        //outRequest->u32Request |= NIC_REQUEST_INFORM_SEND_DONE;
		dmfe_tx_done(0);
    }
    ///* Re-enable interrupt mask */    
    //iow(DM9000_IMR, IMR_PAR | IMR_PTM | IMR_PRM | IMR_LNKCHGI);

	//Kevin ADD
	/* Re-enable interrupt mask */ 
	iow(DM9000_IMR, DM9KS_REGFF);
	//IODelay( 1 );
	//END Kevin

    /* Restore previous register address */
    //dmfeCmdWrite(reg_save);

    
    dmfe_deselect();
	SetMcardClock(oriclock);    
    //if(outRequest->u32Request == 0)
      //  DPrintf("dummy interrupt\n");
	return;
}

/*
  Received a packet and pass to upper layer
*/
static int dmfe_packet_receive(struct net_device *dev)
{
    U08 rxbyte, val;
    U16 i, GoodPacket, tmplen = 0, temp;
    ST_NET_PACKET* newPacket = NULL;
    rx_t rx;
    U16 * ptr = (U16*)&rx;
    U08* rdptr;
    U16 tempVal;
    U16* tempPtr;
    U16 startAddr;
	U08 a,b,c,d;
	RECT_NET_PACKET *inrecvpacket;
	//unsigned char tmpbuf[2048];
	char tmpprintbuf[16],r_checkarp;
	U32 dmareadcnt,reminder;
#if ETHERNET_DMA_READ
	U32 data_ptr_dw;
#endif

    do
	{
		newPacket = NetNewPacket(FALSE);
        if(!newPacket)
        {
            DPrintf("dm9ks allock buffer fail");
            return -1;
        }
		//mpDebugPrint("ior DM9000_MRCMDX");
        /*store the value of Memory Data Read address register*/
        ior(DM9000_MRCMDX);        /* Dummy read */
        rxbyte = *((volatile unsigned char*)(iobase));/* Got most updated data */ 
        rdptr = NET_PACKET_ETHER(newPacket);
        
        if(!(val = check_rx_ready(rxbyte)))
        {
            NetFreePacket(newPacket);
            break;
        }
        
        /* A packet ready now  & Get status/length */
        GoodPacket = TRUE;
        dmfeCmdWrite(DM9000_MRCMD);//CH kv_iow(DM9000_MRCMD);
		
        
        /* Read packet status & length */
        //*ptr = dmfeSDRAMRead();
        //*(ptr+1) = dmfeDataRead();

		a = *((volatile unsigned char*)(iobase));
		b = *((volatile unsigned char*)(iobase));
		c = *((volatile unsigned char*)(iobase));
		d = *((volatile unsigned char*)(iobase));
		tmplen = c | (d << 8);
		rx.desc.length = tmplen;
		//mpDebugPrint("tmplen %d",tmplen);
		if( b & 0xbf )
		{
            GoodPacket = FALSE;
			mpDebugPrint("GoodPacket = FALSE");
		}
		else
		{
			Gpio_Config2GpioFunc(GPIO_FGPIO_11,GPIO_OUTPUT_MODE,GPIO_DATA_HIGH,1);
			dmareadcnt = 0;
#if ETHERNET_DMA_READ
			if( tmplen < 512 )
			{
				dmareadcnt = 0;
				reminder = tmplen;
			}
			else if( tmplen <= 1023 )
			{
				dmareadcnt = 512;
				reminder = tmplen - 512;
			}
			else
			{
				dmareadcnt = 1024;
				reminder = tmplen - 1024;
			}
			if( dmareadcnt )
			{
				data_ptr_dw = ((U32) rdptr) | 0x20000000;
				EthernetRead(data_ptr_dw,dmareadcnt);
			}
			if( reminder )
#endif
			{
				for (i = dmareadcnt; i < tmplen; i++){
					rdptr[i] = *((volatile unsigned char*)(iobase));

				}
			}
			//Add this for close the bus transaction
			Gpio_Config2GpioFunc(DM9KS_CS,GPIO_OUTPUT_MODE,GPIO_DATA_HIGH,1);
			Gpio_Config2GpioFunc(GPIO_FGPIO_11,GPIO_OUTPUT_MODE,GPIO_DATA_LOW,1);
		}
        newPacket->Net.u08NetIndex = NIC_INDEX_ETHER;
        newPacket->Net.u16PayloadSize = rx.desc.length - ETHERNET_HEADER_SIZE;
        r_checkarp = CheckisARP(newPacket);
		if(  r_checkarp == 0 )
		{
			if( free_recv_net_packet_pool_tail )
			{
				inrecvpacket = free_recv_net_packet_pool_head;
				free_recv_net_packet_pool_head = free_recv_net_packet_pool_head->next;
				if( free_recv_net_packet_pool_head == NULL )
					free_recv_net_packet_pool_tail = NULL;
				inrecvpacket->next = NULL;
				inrecvpacket->packetdata = newPacket;
				if( in_recv_net_packet_pool_tail == NULL )
				{
					in_recv_net_packet_pool_head = in_recv_net_packet_pool_tail = inrecvpacket;
				}
				else
				{
					in_recv_net_packet_pool_tail->next = inrecvpacket;
					in_recv_net_packet_pool_tail = inrecvpacket;
				}
			}
			else
			{
				mpDebugPrint("free_recv_net_packet_pool_tail is NULL");
				__asm("break 100");
			}
			if( setrecveventcount < 2 )
			{
				EventSet(ETHERNET_RECV_EVENT,1);
				setrecveventcount++;
			}
			else
			{
				setrecveventcount = 1;
				EventClear(ETHERNET_RECV_EVENT,0);
				EventSet(ETHERNET_RECV_EVENT,1);
				UartOutText("&");
			}

		}
		else if( r_checkarp == 1 )
		{
			//mpDebugPrint("ARP packet");
			if( free_recv_net_arppacket_pool_tail )
			{
				inrecvpacket = free_recv_net_arppacket_pool_head;
				free_recv_net_arppacket_pool_head = free_recv_net_arppacket_pool_head->next;
				if( free_recv_net_arppacket_pool_head == NULL )
					free_recv_net_arppacket_pool_tail = NULL;
				inrecvpacket->next = NULL;
				inrecvpacket->packetdata = newPacket;
				if( in_recv_net_arppacket_pool_tail == NULL )
				{
					in_recv_net_arppacket_pool_head = in_recv_net_arppacket_pool_tail = inrecvpacket;
				}
				else
				{
					in_recv_net_arppacket_pool_tail->next = inrecvpacket;
					in_recv_net_arppacket_pool_tail = inrecvpacket;
				}
			}
			else
			{
				mpDebugPrint("#############free_recv_net_arppacket_pool_tail is NULL##############");
				__asm("break 100");
			}

			if( setarprecveventcount < 2 )
			{
				EventSet(ETHERNET_ARPRECV_EVENT,1);
				setarprecveventcount++;
			}
			else
			{
				setarprecveventcount = 1;
				EventClear(ETHERNET_ARPRECV_EVENT,0);
				EventSet(ETHERNET_ARPRECV_EVENT,1);
				//UartOutText("AR");
			}

		}
		else// -1 drop this packet
            NetFreePacket(newPacket);
		//SemaphoreRelease(IXMLSemid);
		//End Kevin
#if (DEMO_PID==0)
        TaskYield();
#endif
    }while((rxbyte & 0x01) == DM9000_PKT_RDY);
    //dmfe_deselect();
nopacketin:
    return NO_ERR;
}

/*
  Hardware start transmission.
  Send a packet to media from the upper layer.
*/
static int dmfe_start_xmit(ST_NET_PACKET *packet, struct net_device *dev)
{
    U08 *data_ptr;
	U32 data_ptr_dw;
    int i;
	unsigned int tmplen,reminder;
    U16 sendSize = 0;
	board_info_t *db = (board_info_t *)dev->priv;
    data_ptr = NET_PACKET_ETHER(packet);
	char tmpbuf[16];
	//u8 times_of_512;
	unsigned int dmawritecnt;
	U32 oriclock;

	//mpDebugPrint("###########dmfe_start_xmit############");
	if ( !gethernet_is_link_up )
	{
		mpDebugPrint("ethernet link down!!!");
		return 0;
	}
	if(db->Speed == 10)
	{
		while(db->tx_pkt_cnt >=1)
		{
			TaskYield();
		}
	}
	else
	{
		if( db->tx_pkt_cnt >= 2 )
			mpDebugPrint("###############db->tx_pkt_cnt %d",db->tx_pkt_cnt);
		while(db->tx_pkt_cnt >=2)
		{
	#if (DEMO_PID==0)
			TaskYield();
	#endif

		}
	}
	//IntDisable();
	SemaphoreWait(CFETHERNET_MCARD_SEMA);
	Gpio_IntDisable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
    dmfe_select();
	oriclock = GetMcardClock();
	SetMcardClock(CF_CLOCK_KHZ);
	CalculatePulseTimingforEthernet();
	Mcard_Ethernet_Active(INITIAL_MEMORY);


    iow(DM9000_IMR, DM9KS_DISINTR);

    /* Set TX length to reg. 0xfc & 0xfd */
    iow(DM9000_TXPLL, packet->Net.len & 0x00ff);
    iow(DM9000_TXPLH, (packet->Net.len & 0xff00) >> 8);

    dmfeCmdWrite(DM9000_MWCMD); // Write data into SRAM trigger
	IODelay( 1 );
	Gpio_Config2GpioFunc(DM9KS_CS,GPIO_OUTPUT_MODE,GPIO_DATA_LOW,1);
	IODelay( 1 );
	Gpio_Config2GpioFunc(GPIO_FGPIO_11,GPIO_OUTPUT_MODE,GPIO_DATA_HIGH,1);
	dma_invalid_dcache();
    tmplen = (packet->Net.len);
#if ETHERNET_DMA_WRITE
	if( tmplen < 512 )
	{
		dmawritecnt = 0;
		reminder = tmplen;
	}
	else if( tmplen <= 1023 )
	{
		dmawritecnt = 512;
		reminder = tmplen - 512;
	}
	else if( tmplen <= 1535 )
	{
		dmawritecnt = 1024;
		reminder = tmplen - 1024;
	}
	else if( tmplen <= 2047 )
	{
		dmawritecnt = 1536;
		reminder = tmplen - 1536;
	}

	if( dmawritecnt )
	{
		data_ptr_dw = ((U32) data_ptr) | 0x20000000;
		EthernetWrite(data_ptr_dw,dmawritecnt);
	}

	if( reminder )
	{
		for (i = dmawritecnt ; i < tmplen; i++)
		{
			dmfeDataWrite(data_ptr[i]);
		}
	}
#else
	for (i = 0 ; i < tmplen; i++)
	{
		dmfeDataWrite(data_ptr[i]);
	}
#endif
	IODelay( 1 );
	Gpio_Config2GpioFunc(DM9KS_CS,GPIO_OUTPUT_MODE,GPIO_DATA_HIGH,1);
	IODelay( 1 );
	Gpio_Config2GpioFunc(GPIO_FGPIO_11,GPIO_OUTPUT_MODE,GPIO_DATA_LOW,1);
    /* Issue TX polling command */

    iow(DM9000_TCR, 0x1); /* Cleared after TX complete*/
    sendSize = packet->Net.len - ETHERNET_HEADER_SIZE;

	db->tx_pkt_cnt++;

	/* Re-enable interrupt mask */ 
	iow(DM9000_IMR, DM9KS_REGFF);
    dmfe_deselect();	
	SetMcardClock(oriclock);
	//IntEnable();	
	Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
	SemaphoreRelease(CFETHERNET_MCARD_SEMA);
    return sendSize;
}

static void dmfe_tx_done(unsigned long unused)
{
	struct net_device *dev = dmfe_dev;
	board_info_t *db = (board_info_t *)dev->priv;
	int  nsr;

	//DMFE_DBUG(0, "dmfe_tx_done()", 0);
	//mpDebugPrint("dmfe_tx_done");
	
	nsr = ior(DM9000_NSR);
	//mpDebugPrint("nsr %x",nsr);
	if (nsr & 0x0c)
	{
		if(nsr & 0x04) db->tx_pkt_cnt--;
		if(nsr & 0x08) db->tx_pkt_cnt--;
		if(db->tx_pkt_cnt < 0)
		{
			while(ior(DM9000_TCR) & 0x1)
			{
				UartOutText("WZ");
			}
			db->tx_pkt_cnt = 0;
		}
			
	}else
	{
		while(ior(DM9000_TCR) & 0x1)
		{
			UartOutText("K");
		}
		mpDebugPrint("End");
		db->tx_pkt_cnt = 0;
	}
	
	return;
}

void Ethernet_CheckArp(void)
{
	unsigned int mevent;
	int ret;
    struct net_device *dev = dmfe_dev;
	RECT_NET_PACKET *inrecvpacket;
	ST_NET_PACKET *inPacket;
	mpDebugPrint("Ethernet_CheckArp GO GO GO");
	while(1)
	{
		ret = EventWait(ETHERNET_ARPRECV_EVENT, 0xffffffff, OS_EVENT_OR, &mevent);
		SemaphoreWait(CFETHERNET_MCARD_SEMA);
		Gpio_IntDisable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
		setarprecveventcount--;
		if( setarprecveventcount < 0 )
			setarprecveventcount = 0;
		if( in_recv_net_arppacket_pool_head )
		{
			inrecvpacket = in_recv_net_arppacket_pool_head;
			in_recv_net_arppacket_pool_head = in_recv_net_arppacket_pool_head->next;
			if( in_recv_net_arppacket_pool_head == NULL )
				in_recv_net_arppacket_pool_tail = NULL;
			inPacket = inrecvpacket->packetdata;
			inrecvpacket->next = NULL;
			inrecvpacket->packetdata = NULL;
			if( free_recv_net_arppacket_pool_tail == NULL )
			{
				free_recv_net_arppacket_pool_head = free_recv_net_arppacket_pool_tail = inrecvpacket;
			}
			else
			{
				free_recv_net_arppacket_pool_tail->next = inrecvpacket;
				free_recv_net_arppacket_pool_tail = inrecvpacket;
			}
			//IntEnable();
			Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
			SemaphoreRelease(CFETHERNET_MCARD_SEMA);
			//mpDebugPrint("indicate Packet");
			if (!ArpPacketReceive2(inPacket))
			{
				//do nothing
			}
			else
			{
				//mpDebugPrint("it is not arp packet!!!!!!!");
				NetPacketReceive(inPacket);

			}
			//IntDisable();
			SemaphoreWait(CFETHERNET_MCARD_SEMA);
			Gpio_IntDisable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
			if( in_recv_net_arppacket_pool_head )
			{
				if( setarprecveventcount < 2 )
				{
					EventSet(ETHERNET_ARPRECV_EVENT,1);
					setarprecveventcount++;
				}
				//else
				//	UartOutText("AP");
			}
			//IntEnable();			
			Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
			SemaphoreRelease(CFETHERNET_MCARD_SEMA);
		}
		else
		{
			//IntEnable();
			Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
			SemaphoreRelease(CFETHERNET_MCARD_SEMA);
		}
	}
	
}
void Ethernet_RecvTask(void)
{
	unsigned int mevent;
	int ret;
    struct net_device *dev = dmfe_dev;
	RECT_NET_PACKET *inrecvpacket;
	ST_NET_PACKET *inPacket;
	mpDebugPrint("Ethernet_RecvTask GO GO GO");
	while(1)
	{
		//if( gethernet_is_link_up )
		//	ret = EventWaitWithTO(ETHERNET_RECV_EVENT,0xffffffff,OS_EVENT_OR,&mevent,500);
		//else
			ret = EventWait(ETHERNET_RECV_EVENT, 0xffffffff, OS_EVENT_OR, &mevent);

		//IntDisable();
		SemaphoreWait(CFETHERNET_MCARD_SEMA);
		Gpio_IntDisable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
		setrecveventcount--;
		if( setrecveventcount < 0 )
			setrecveventcount = 0;
		if( in_recv_net_packet_pool_head )
		{
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
			//IntEnable();
			Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
			SemaphoreRelease(CFETHERNET_MCARD_SEMA);
			//mpDebugPrint("indicate Packet");
			NetPacketReceive(inPacket);
			//IntDisable();
			SemaphoreWait(CFETHERNET_MCARD_SEMA);
			Gpio_IntDisable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
			if( in_recv_net_packet_pool_head )
			{
				if( setrecveventcount < 2 )
				{
					EventSet(ETHERNET_RECV_EVENT,1);
					setrecveventcount++;
				}
				else
					UartOutText("#");
			}
			//IntEnable();			
			Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
			SemaphoreRelease(CFETHERNET_MCARD_SEMA);
		}
		else
		{
			//IntEnable();
			Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
			SemaphoreRelease(CFETHERNET_MCARD_SEMA);
		}
	#if (DEMO_PID==0)
		 TaskYield();
	#endif
		
	}
}

char GetMACAddress(U08 *mac_addr)
{
	int i;
	DRIVE *sDrv;
	STREAM *w_shandle;
	char filebuffer[128];
	//U08 mac_addr[6];
	char *macstring,*tmpptr;
	unsigned char tmphex;
	BYTE  curDrvId;
	char result = 0;

	curDrvId = DriveCurIdGet();
	DriveChange(SD_MMC);
	sDrv = DriveGet(DriveCurIdGet());
	//sDrv = DriveGet(SD_MMC);//SD_MMC//NAND
	if (FS_SUCCEED == FileSearch(sDrv, "netconfig", "txt", E_FILE_TYPE))
	{
		w_shandle = FileOpen(sDrv);
		if(w_shandle)
		{
			FileRead(w_shandle,filebuffer,128);
			if( macstring = strstr(filebuffer,"mac:") )
			{
				for(i=0;i<6;i++)
				{
					tmpptr = macstring+4+3*i;
					*(tmpptr+2) = '\0';
					tmphex = strtol(tmpptr,NULL,16);
					*(mac_addr+i) = tmphex;
					mpDebugPrint("tmphex GGG %x",tmphex);
				}
				result = 1;
			}
			FileClose(w_shandle);
		}
	}
	else
	{
		mpDebugPrint("Can not find netconfig.txt file in SD.");
	}
	DriveChange(curDrvId);
	return result;
}
struct ethernetconfig *GetEtherNetConfig()
{
	int i;
	DRIVE *sDrv;
	STREAM *w_shandle;
	char *filebuffer=NULL;
	char *tempbuf=NULL;
	char tmpip[32];
	char *configstring,*tmpptr;
	unsigned char tmphex;
	BYTE  curDrvId;
	struct ethernetconfig *netconfig;
	unsigned int iplen=0,ip1=0,ip2=0;
	
	filebuffer = ext_mem_malloc(256);
	tempbuf = ext_mem_malloc(256);
    netconfig = ext_mem_malloc(sizeof(struct ethernetconfig));
	if(filebuffer==NULL)
		return NULL;
	
	memset(filebuffer,0x00,256);
	memset(netconfig,0x00,sizeof(struct ethernetconfig));
	
	curDrvId = DriveCurIdGet();
	DriveChange(SD_MMC);
	sDrv = DriveGet(DriveCurIdGet());
	//sDrv = DriveGet(SD_MMC);//SD_MMC//NAND
	if (FS_SUCCEED == FileSearch(sDrv, "netconfig", "txt", E_FILE_TYPE))
	{
		w_shandle = FileOpen(sDrv);
		if(w_shandle)
		{
			FileRead(w_shandle,filebuffer,256);
			memcpy(tempbuf,filebuffer,256);
			if( configstring = strstr(tempbuf,"mac:") )
			{
				for(i=0;i<6;i++)
				{
					tmpptr = configstring+4+3*i;
					*(tmpptr+2) = '\0';
					tmphex = strtol(tmpptr,NULL,16);
					netconfig->macaddr[i]= tmphex;
				}
			}
			memcpy(tempbuf,filebuffer,256);
			if( configstring = strstr(tempbuf,"dhcp:") )
			{
			  if((strncmp(configstring,"dhcp:on",strlen("dhcp:on"))==0)||(strncmp(configstring,"dhcp:ON",strlen("dhcp:ON"))==0))
			  	netconfig->dhcpflag = FALSE;//ON
			  else
			  	netconfig->dhcpflag = TRUE;//OFF
			  	
			}
			memcpy(tempbuf,filebuffer,256);
			if( configstring = strstr(tempbuf,"ipaddress:") )
			{
				ip1 = (unsigned int)(configstring+strlen("ipaddress:"));
			}
			memcpy(tempbuf,filebuffer,256);
			if( configstring = strstr(tempbuf,"netmask:") )
			{
				ip2 = (unsigned int)configstring;
				iplen = ip2-ip1;
				memset(tmpip,0x00,32);
				memcpy(tmpip,ip1,iplen-2);
			    netconfig->ipaddr = StringToIp(tmpip);
			}
			memcpy(tempbuf,filebuffer,256);
			if( configstring = strstr(tempbuf,"gateway:") )
			{
				ip1 = (unsigned int)configstring;
				iplen = ip1-(ip2+strlen("netmask:"));
				memset(tmpip,0x00,32);
				memcpy(tmpip,(ip2+strlen("netmask:")),iplen-2);
			    netconfig->netmask= StringToIp(tmpip);
			}
			
			memcpy(tempbuf,filebuffer,256);
			if( configstring = strstr(tempbuf,"p2ptest:") )
			{
				ip2 = (unsigned int)configstring;
				iplen = ip2-(ip1+strlen("gateway:"));
				memset(tmpip,0x00,32);
				memcpy(tmpip,(ip1+strlen("gateway:")),iplen-2);
			    netconfig->gateway= StringToIp(tmpip);
				if((strncmp(configstring,"p2ptest:on",strlen("p2ptest:on"))==0)||(strncmp(configstring,"p2ptest:ON",strlen("p2ptest:ON"))==0))
			  		netconfig->p2ptestflag = TRUE;
			  	else
			  		netconfig->p2ptestflag = FALSE;

				
			}
			memcpy(tempbuf,filebuffer,256);
			if( configstring = strstr(tempbuf,"target:") )
			{
			    iplen = strlen(configstring)-strlen("target:");
				memset(tmpip,0x00,32);
				memcpy(tmpip,(configstring+strlen("target:")),iplen-2);
			    netconfig->targetip= StringToIp(tmpip);
			}

			FileClose(w_shandle);
		}
	}
	else
	{
		mpDebugPrint("Can not find netconfig.txt file in SD.");
	}
	if(filebuffer)
		ext_mem_free(filebuffer);
	if(tempbuf)
		ext_mem_free(tempbuf);
	
	DriveChange(curDrvId);
	return (struct ethernetconfig*)netconfig;
}
BYTE GetNetConfigDhcpFlag()
{
   //mpDebugPrint("%s %d",__func__,dm9netconfig->dhcpflag);
   return dm9netconfig->dhcpflag;
}
int GetNetConfigIPaddr()
{
   return dm9netconfig->ipaddr;
}
int GetNetConfigNetmask()
{
   return dm9netconfig->netmask;
}
int GetNetConfigGateway()
{
   return dm9netconfig->gateway;
}
BYTE GetNetConfigP2PTestFlag()
{
   return dm9netconfig->p2ptestflag;
}
int GetNetConfigTarget()
{
   return dm9netconfig->targetip;
}
extern U08 u08TcpTimerId;
extern void NetTimerHandle();
void dm9ks_stop_rx()
{
	iow(DM9000_IMR, IMR_PAR);	/* Disable all interrupt */
	udelay(1);
	iow(DM9000_RCR, 0x00);	/* Disable RX */
    udelay(1);
	dmfe_deselect();
	NetTimerStop(u08TcpTimerId);
    SysTimerProcPause(NetTimerHandle);
	DisableNetWareTask();

}
void dm9ks_start_rx()
{
    EnableNetWareTask();
    SysTimerProcResume(NetTimerHandle);

    NetTimerRun(u08TcpTimerId);

    dmfe_select();
    /* Activate DM9000A/DM9010 */
    iow(DM9000_RCR, RCR_DIS_CRC | RCR_DIS_LONG | RCR_RXEN);
    udelay(1);
    /* Enable TX/RX/LINKCHANGE interrupt mask */
    iow(DM9000_IMR, IMR_PAR | IMR_PRM | IMR_LNKCHGI);
    udelay(1);
}

