/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
 
	Module Name:
	rtmp.h

	Abstract:

	Revision History:
	Who			When		What
	--------	----------	----------------------------------------------
*/

#ifndef __RTMP_H__
#define __RTMP_H__

#include "link_list.h"

#include "mlme.h"
#include "oid.h"
#include "wpa.h"
#include "usb.h"
#include "atomic.h"
#include "wireless.h"
#include "lwip_incl.h"

//
// Extern
//
extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
extern ULONG BIT32[32];
//extern UCHAR BIT8[8];
extern char* CipherName[];

extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR EAPOL_LLC_SNAP[8];
extern UCHAR EAPOL[2];
extern UCHAR IPX[2];
extern UCHAR APPLE_TALK[2];
extern UCHAR RateIdToPlcpSignal[12]; // see IEEE802.11a-1999 p.14
extern UCHAR OfdmSignalToRateId[16] ;
extern UCHAR default_cwmin[4];
extern UCHAR default_cwmax[4];
extern UCHAR default_sta_aifsn[4];
extern UCHAR MapUserPriorityToAccessCategory[8];

extern UCHAR  Phy11BNextRateDownward[];
extern UCHAR  Phy11BNextRateUpward[];
extern UCHAR  Phy11BGNextRateDownward[];
extern UCHAR  Phy11BGNextRateUpward[];
extern UCHAR  Phy11ANextRateDownward[];
extern UCHAR  Phy11ANextRateUpward[];
extern CHAR   RssiSafeLevelForTxRate[];
extern UCHAR  RateIdToMbps[];
extern USHORT RateIdTo500Kbps[];

extern UCHAR  CipherSuiteWpaNoneTkip[];
extern UCHAR  CipherSuiteWpaNoneTkipLen;

extern UCHAR  CipherSuiteWpaNoneAes[];
extern UCHAR  CipherSuiteWpaNoneAesLen;

extern UCHAR  SsidIe;
extern UCHAR  SupRateIe;
extern UCHAR  ExtRateIe;
extern UCHAR  ErpIe;
extern UCHAR  DsIe;
extern UCHAR  TimIe;
extern UCHAR  WpaIe;
extern UCHAR  Wpa2Ie;
extern UCHAR  IbssIe;

extern UCHAR  WPA_OUI[];
extern UCHAR  RSN_OUI[];
extern UCHAR  WME_INFO_ELEM[];
extern UCHAR  WME_PARM_ELEM[];
extern UCHAR  RALINK_OUI[];

//extern struct usb_device_id rtusb_usb_id[];
//extern INT const rtusb_usb_id_len;

//
// MACRO for linux usb
//
typedef struct urb *purbb_t;
typedef struct usb_ctrlrequest devctrlrequest;

// for vendor-specific control operations
#define CONTROL_TIMEOUT_MS		(1000)	 /* msec */  // lengthen timeout for loading firmware
#define CONTROL_TIMEOUT_JIFFIES ((CONTROL_TIMEOUT_MS * HZ)/1000)

//#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,7)
//#define RTUSB_UNLINK_URB(urb)	usb_kill_urb(urb)
//#else
//#define RTUSB_UNLINK_URB(urb)	usb_unlink_urb(urb)
//#endif

/* map devrequest fields onto usb_ctrlrequest's */
#define DEVREQ_REQUEST(x)		((x)->bRequest)
#define DEVREQ_REQUESTTYPE(x)	((x)->bRequestType)
#define DEVREQ_VALUE(x) 		((x)->wValue)
#define DEVREQ_INDEX(x) 		((x)->wIndex)
#define DEVREQ_LENGTH(x)		((x)->wLength)

#define PURB		purbb_t
#define PIRP		PVOID
#define PMDL		PVOID
#define NDIS_OID	UINT	

#define STATUS_SUCCESS		0x00
#define STATUS_UNSUCCESSFUL 0x01

typedef LONG		NTSTATUS;
typedef NTSTATUS	*PNTSTATUS;


typedef struct net_device	* PNET_DEV;
typedef struct sk_buff		* PNDIS_PACKET;
typedef struct sk_buff		  NDIS_PACKET;
typedef PNDIS_PACKET		* PPNDIS_PACKET;

static inline void NdisGetSystemUpTime(ULONG *time)
{
	*time = jiffies;
}

//
//	Queue structure and macros
//
typedef struct	_QUEUE_ENTRY	{
	struct _QUEUE_ENTRY 	*Next;
}	QUEUE_ENTRY, *PQUEUE_ENTRY;

// Queue structure
typedef struct	_QUEUE_HEADER	{
	PQUEUE_ENTRY	Head;
	PQUEUE_ENTRY	Tail;
	ULONG			Number;
}	QUEUE_HEADER, *PQUEUE_HEADER;

#define InitializeQueueHeader(QueueHeader)				\
{														\
	(QueueHeader)->Head = (QueueHeader)->Tail = NULL;	\
	(QueueHeader)->Number = 0;							\
}

#define RemoveHeadQueue(QueueHeader)				\
(QueueHeader)->Head;								\
{													\
	PQUEUE_ENTRY pNext; 							\
	if ((QueueHeader)->Head != NULL)				\
	{												\
		pNext = (QueueHeader)->Head->Next;			\
		(QueueHeader)->Head = pNext;				\
		if (pNext == NULL)							\
			(QueueHeader)->Tail = NULL; 			\
		(QueueHeader)->Number--;					\
	}												\
}

#define InsertHeadQueue(QueueHeader, QueueEntry)			\
{															\
	((PQUEUE_ENTRY)QueueEntry)->Next = (QueueHeader)->Head; \
	(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);		\
	if ((QueueHeader)->Tail == NULL)						\
		(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);	\
	(QueueHeader)->Number++;								\
}

#define InsertTailQueue(QueueHeader, QueueEntry)				\
{																\
	((PQUEUE_ENTRY)QueueEntry)->Next = NULL;					\
	if ((QueueHeader)->Tail)									\
		(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry); \
	else														\
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);		\
	(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);			\
	(QueueHeader)->Number++;									\
}


//
//	Macro for debugging information
//
#define KERN_DEBUG
#ifdef DBG
extern ULONG	RTDebugLevel;

#define DBGPRINT(Level, fmt, args...)		\
{									\
	if (Level <= RTDebugLevel)		\
	{								\
		printk(NIC_DBG_STRING); 	\
		printk(KERN_DEBUG fmt, ## args);	\
	}									\
}										

#define DBGPRINT_ERR(fmt, args...)			\
{											\
	printk("ERROR!!! ");					\
	printk(KERN_DEBUG fmt, ## args);		\
}

#define DBGPRINT_RAW(Level, fmt, args...)	\
{											\
	if (Level <= RTDebugLevel)				\
	{										\
		printk(" ");						\
		printk(KERN_DEBUG fmt, ## args);	\
	}										\
}
#ifdef PLATFORM_MPIXEL
void NetPacketDump(unsigned long address, unsigned long size);
#define DBGPRINT_DUMP(Level, address, size)	\
{											\
	if (Level <= RTDebugLevel)				\
	{										\
		NetPacketDump(address, size);		\
	}										\
}
#endif
#else
#define DBGPRINT(Level, fmt, args...)
#define DBGPRINT_ERR(fmt, args...)
#define DBGPRINT_RAW(Level, fmt, args...)
#ifdef PLATFORM_MPIXEL
#define DBGPRINT_DUMP(Level, address, size)
#endif
#endif
/* remove by johnli
#  define assert(expr)											\
		if(unlikely(!(expr))) {									\
		printk(KERN_ERR "Assertion failed! %s,%s,%s,line=%d\n",	\
		#expr,__FILE__,__FUNCTION__,__LINE__);					\
		}
*/

//
//	spin_lock enhanced for Nested spin lock
//
#ifdef LINUX
#define	NdisAllocateSpinLock(lock)	\
{									\
	spin_lock_init(lock);			\
}
#else
#define	NdisAllocateSpinLock(lock)	\
{									\
    signed long _id;					\
    _id = mpx_SemaphoreCreate(OS_ATTR_FIFO, 1); 		\
    /*if (_id < 0)*/ 					\
	/*	MP_ASSERT(0);*/				\
	/*else*/	                        \  
	if(_id >= 0)					\
		*(lock) = (spinlock_t)_id;				\
}
#endif
#if 0
#define NdisReleaseSpinLock(lock, flagg)    \
{											\
	if (in_interrupt())						\
		spin_unlock_irqrestore(lock, flagg);\
	else									\
		spin_unlock(lock);					\
}

#define NdisAcquireSpinLock(lock, flagg)    \
{											\
	if (in_interrupt())						\
		spin_lock_irqsave(lock, flagg);		\
	else									\
		spin_lock(lock);					\
}
#else
#define NdisReleaseSpinLock(lock, flagg)	\
{											\
    if (*(lock) > 0)			 			\
		SemaphoreRelease(*(lock)); 			\
}
#define NdisAcquireSpinLock(lock, flagg)	\
{											\
    if (*(lock) > 0)			 			\
		SemaphoreWait(*(lock)); 			\
}
#endif

#define NdisFreeSpinLock(lock)			\
{										\
}

#define RTUSBFreeSkbBuffer(skb)				\
{											\
	netBufFree(skb);						\
}

#define RTUSBMlmeUp(pAd)	        \
{								    \
    SemaphoreRelease((pAd->mlme_semaphore)); \
}
//2008/01/07:KH add to solve the racing condition of Mac Registers
#define RTUSBMacRegDown(pAd) \
{								    \
        SemaphoreWait(pAd->MaCRegWrite_semaphore); \
        pAd->MacRegWrite_Processing=1;\
}
#define RTUSBMacRegUp(pAd)	        \
{								    \
	pAd->MacRegWrite_Processing=0;\
        SemaphoreRelease((pAd->MaCRegWrite_semaphore)); \
}
#define RTUSBCMDUp(pAd)	                \
{									    \
	if(pAd->RTUSBCmdThr_pid>0)		    \
		SemaphoreRelease((pAd->RTUSBCmd_semaphore)); \
}

//Setup Packet used in Ctrl urb's filler....
#define FILL_REQUEST(a,aa,ab,ac,ad,ae)		\
  do {										\
	  (a)->devreq->request = aa;			\
	  (a)->devreq->requesttype = ab;		\
	  (a)->devreq->value = cpu_to_le16(ac); \
	  (a)->devreq->index = cpu_to_le16(ad); \
	  (a)->devreq->length = cpu_to_le16(ae);\
  }while(0);


// direction is specified in TransferFlags
#define URB_FUNCTION_RESERVED0						0x0016

//
// These are for sending vendor and class commands
// on the default pipe
//
// direction is specified in TransferFlags
#define URB_FUNCTION_VENDOR_DEVICE					 0x0017
#define URB_FUNCTION_VENDOR_INTERFACE				 0x0018
#define URB_FUNCTION_VENDOR_ENDPOINT				 0x0019
#define URB_FUNCTION_VENDOR_OTHER					 0x0020

#define URB_FUNCTION_CLASS_DEVICE					 0x001A
#define URB_FUNCTION_CLASS_INTERFACE				 0x001B
#define URB_FUNCTION_CLASS_ENDPOINT 				 0x001C
#define URB_FUNCTION_CLASS_OTHER					 0x001F

//
// Reserved function codes
//																										  
#define URB_FUNCTION_RESERVED						 0x001D

#define URB_FUNCTION_GET_CONFIGURATION				 0x0026
#define URB_FUNCTION_GET_INTERFACE					 0x0027
					
#define URB_FUNCTION_LAST							 0x0029


//
//	Assert MACRO to make sure program running
//
#undef	ASSERT
#define	KERN_WARNING
#define ASSERT(x)																\
{																				\
	if (!(x))																	\
	{																			\
		printk(KERN_WARNING __FILE__ ":%d assert " #x "failed\n", __LINE__);	\
		BREAK_POINT();															\
	}																			\
}

//
//	Macros for flag and ref count operations
//
#define RTMP_SET_FLAG(_M, _F)		((_M)->Flags |= (_F))
#define RTMP_CLEAR_FLAG(_M, _F) 	((_M)->Flags &= ~(_F))
#define RTMP_CLEAR_FLAGS(_M)		((_M)->Flags = 0)
#define RTMP_TEST_FLAG(_M, _F)		(((_M)->Flags & (_F)) != 0)
#define RTMP_TEST_FLAGS(_M, _F) 	(((_M)->Flags & (_F)) == (_F))

// Flags control for RT2500 USB bulk out frame type
#define RTUSB_SET_BULK_FLAG(_M, _F)				((_M)->BulkFlags |= (_F))
#define RTUSB_CLEAR_BULK_FLAG(_M, _F)			((_M)->BulkFlags &= ~(_F))
#define RTUSB_TEST_BULK_FLAG(_M, _F)			(((_M)->BulkFlags & (_F)) != 0)

#define OPSTATUS_SET_FLAG(_pAd, _F) 	((_pAd)->PortCfg.OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG(_pAd, _F)	((_pAd)->PortCfg.OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG(_pAd, _F)	(((_pAd)->PortCfg.OpStatusFlags & (_F)) != 0)

#define CLIENT_STATUS_SET_FLAG(_pEntry,_F)		((_pEntry)->ClientStatusFlags |= (_F))
#define CLIENT_STATUS_CLEAR_FLAG(_pEntry,_F)	((_pEntry)->ClientStatusFlags &= ~(_F))
#define CLIENT_STATUS_TEST_FLAG(_pEntry,_F) 	(((_pEntry)->ClientStatusFlags & (_F)) != 0)


#define INC_RING_INDEX(_idx, _RingSize)    \
{										   \
	(_idx)++;							   \
	if ((_idx) >= (_RingSize)) _idx=0;	   \
}

// Increase TxTsc value for next transmission
// TODO: 
// When i==6, means TSC has done one full cycle, do re-keying stuff follow specs
// Should send a special event microsoft defined to request re-key
#define INC_TX_TSC(_tsc)								\
{														\
	int i=0;											\
	while (++_tsc[i] == 0x0)							\
	{													\
		i++;											\
		if (i == 6) 									\
			break;										\
	}													\
}


#undef	NdisMoveMemory
#undef	NdisZeroMemory
#undef	NdisFillMemory
#undef	NdisEqualMemory

#define NdisMoveMemory(Destination, Source, Length) RTMPMoveMemory(Destination, Source, Length)
#define NdisZeroMemory(Destination, Length) 		RTMPZeroMemory(Destination, Length)
#define NdisFillMemory(Destination, Length, Fill)	RTMPFillMemory(Destination, Length, Fill)
#define NdisEqualMemory(Source1, Source2, Length)	RTMPEqualMemory(Source1, Source2, Length)

#define MAC_ADDR_EQUAL(pAddr1,pAddr2)				RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define COPY_MAC_ADDR(Addr1, Addr2) 				memcpy((Addr1), (Addr2), MAC_ADDR_LEN)
#define SSID_EQUAL(ssid1, len1, ssid2, len2)		((len1==len2) && (RTMPEqualMemory(ssid1, ssid2, len1)))

#define NdisMSleep                                  mdelay


#define 	MAP_CHANNEL_ID_TO_KHZ(ch, khz)	{				\
				switch (ch) 								\
				{											\
					case 1: 	khz = 2412000;	 break; 	\
					case 2: 	khz = 2417000;	 break; 	\
					case 3: 	khz = 2422000;	 break; 	\
					case 4: 	khz = 2427000;	 break; 	\
					case 5: 	khz = 2432000;	 break; 	\
					case 6: 	khz = 2437000;	 break; 	\
					case 7: 	khz = 2442000;	 break; 	\
					case 8: 	khz = 2447000;	 break; 	\
					case 9: 	khz = 2452000;	 break; 	\
					case 10:	khz = 2457000;	 break; 	\
					case 11:	khz = 2462000;	 break; 	\
					case 12:	khz = 2467000;	 break; 	\
					case 13:	khz = 2472000;	 break; 	\
					case 14:	khz = 2484000;	 break; 	\
					case 36:  /* UNII */  khz = 5180000;   break;	  \
					case 40:  /* UNII */  khz = 5200000;   break;	  \
					case 44:  /* UNII */  khz = 5220000;   break;	  \
					case 48:  /* UNII */  khz = 5240000;   break;	  \
					case 52:  /* UNII */  khz = 5260000;   break;	  \
					case 56:  /* UNII */  khz = 5280000;   break;	  \
					case 60:  /* UNII */  khz = 5300000;   break;	  \
					case 64:  /* UNII */  khz = 5320000;   break;	  \
					case 149: /* UNII */  khz = 5745000;   break;	  \
					case 153: /* UNII */  khz = 5765000;   break;	  \
					case 157: /* UNII */  khz = 5785000;   break;	  \
					case 161: /* UNII */  khz = 5805000;   break;	  \
					case 165: /* UNII */  khz = 5825000;   break;	  \
					case 100: /* HiperLAN2 */  khz = 5500000;	break;	   \
					case 104: /* HiperLAN2 */  khz = 5520000;	break;	   \
					case 108: /* HiperLAN2 */  khz = 5540000;	break;	   \
					case 112: /* HiperLAN2 */  khz = 5560000;	break;	   \
					case 116: /* HiperLAN2 */  khz = 5580000;	break;	   \
					case 120: /* HiperLAN2 */  khz = 5600000;	break;	   \
					case 124: /* HiperLAN2 */  khz = 5620000;	break;	   \
					case 128: /* HiperLAN2 */  khz = 5640000;	break;	   \
					case 132: /* HiperLAN2 */  khz = 5660000;	break;	   \
					case 136: /* HiperLAN2 */  khz = 5680000;	break;	   \
					case 140: /* HiperLAN2 */  khz = 5700000;	break;	   \
					case 34:  /* Japan MMAC */	 khz = 5170000;   break;   \
					case 38:  /* Japan MMAC */	 khz = 5190000;   break;   \
					case 42:  /* Japan MMAC */	 khz = 5210000;   break;   \
					case 46:  /* Japan MMAC */	 khz = 5230000;   break;   \
					default:	khz = 2412000;	 break; 	\
				}											\
			}

#define 	MAP_KHZ_TO_CHANNEL_ID(khz, ch)	{				\
				switch (khz)								\
				{											\
					case 2412000:	 ch = 1;	 break; 	\
					case 2417000:	 ch = 2;	 break; 	\
					case 2422000:	 ch = 3;	 break; 	\
					case 2427000:	 ch = 4;	 break; 	\
					case 2432000:	 ch = 5;	 break; 	\
					case 2437000:	 ch = 6;	 break; 	\
					case 2442000:	 ch = 7;	 break; 	\
					case 2447000:	 ch = 8;	 break; 	\
					case 2452000:	 ch = 9;	 break; 	\
					case 2457000:	 ch = 10;	 break; 	\
					case 2462000:	 ch = 11;	 break; 	\
					case 2467000:	 ch = 12;	 break; 	\
					case 2472000:	 ch = 13;	 break; 	\
					case 2484000:	 ch = 14;	 break; 	\
					case 5180000:	 ch = 36;  /* UNII */  break;	  \
					case 5200000:	 ch = 40;  /* UNII */  break;	  \
					case 5220000:	 ch = 44;  /* UNII */  break;	  \
					case 5240000:	 ch = 48;  /* UNII */  break;	  \
					case 5260000:	 ch = 52;  /* UNII */  break;	  \
					case 5280000:	 ch = 56;  /* UNII */  break;	  \
					case 5300000:	 ch = 60;  /* UNII */  break;	  \
					case 5320000:	 ch = 64;  /* UNII */  break;	  \
					case 5745000:	 ch = 149; /* UNII */  break;	  \
					case 5765000:	 ch = 153; /* UNII */  break;	  \
					case 5785000:	 ch = 157; /* UNII */  break;	  \
					case 5805000:	 ch = 161; /* UNII */  break;	  \
					case 5825000:	 ch = 165; /* UNII */  break;	  \
					case 5500000:	 ch = 100; /* HiperLAN2 */	break;	   \
					case 5520000:	 ch = 104; /* HiperLAN2 */	break;	   \
					case 5540000:	 ch = 108; /* HiperLAN2 */	break;	   \
					case 5560000:	 ch = 112; /* HiperLAN2 */	break;	   \
					case 5580000:	 ch = 116; /* HiperLAN2 */	break;	   \
					case 5600000:	 ch = 120; /* HiperLAN2 */	break;	   \
					case 5620000:	 ch = 124; /* HiperLAN2 */	break;	   \
					case 5640000:	 ch = 128; /* HiperLAN2 */	break;	   \
					case 5660000:	 ch = 132; /* HiperLAN2 */	break;	   \
					case 5680000:	 ch = 136; /* HiperLAN2 */	break;	   \
					case 5700000:	 ch = 140; /* HiperLAN2 */	break;	   \
					case 5170000:	 ch = 34;  /* Japan MMAC */   break;   \
					case 5190000:	 ch = 38;  /* Japan MMAC */   break;   \
					case 5210000:	 ch = 42;  /* Japan MMAC */   break;   \
					case 5230000:	 ch = 46;  /* Japan MMAC */   break;   \
					default:		 ch = 1;	 break; 	\
				}											\
			}

#define VIRTUAL_IF_INC(pAd) ((pAd)->VirtualIfCnt++) 
#define VIRTUAL_IF_DEC(pAd) ((pAd)->VirtualIfCnt--)
#define VIRTUAL_IF_NUM(pAd) ((pAd)->VirtualIfCnt)
  
//
// Common fragment list structure -  Identical to the scatter gather frag list structure
//
#define NIC_MAX_PHYS_BUF_COUNT				8

typedef struct _RTMP_SCATTER_GATHER_ELEMENT {
	PVOID		Address;
	ULONG		Length;
	PULONG		Reserved;
} RTMP_SCATTER_GATHER_ELEMENT, *PRTMP_SCATTER_GATHER_ELEMENT;

typedef struct _RTMP_SCATTER_GATHER_LIST {
	ULONG  NumberOfElements;
	PULONG Reserved;
	RTMP_SCATTER_GATHER_ELEMENT Elements[NIC_MAX_PHYS_BUF_COUNT];
} RTMP_SCATTER_GATHER_LIST, *PRTMP_SCATTER_GATHER_LIST;


//
//	Some utility macros
//
#ifndef min
#define min(_a, _b) 	(((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b) 	(((_a) > (_b)) ? (_a) : (_b))
#endif

#define INC_COUNTER(Val)		(Val.QuadPart++)
#define	INC_COUNTER64(Val)		(Val.QuadPart++)

#define INFRA_ON(_p)			(OPSTATUS_TEST_FLAG(_p, fOP_STATUS_INFRA_ON))
#define ADHOC_ON(_p)			(OPSTATUS_TEST_FLAG(_p, fOP_STATUS_ADHOC_ON))



#define RTMP_SET_PACKET_MOREDATA(_p, _morebit)	((_p)->cb[6] = _morebit)
#define RTMP_GET_PACKET_MOREDATA(_p)			((_p)->cb[6])

// b0-b3 as User Priority
#define RTMP_SET_PACKET_UP(_p, _prio)				((_p)->cb[8] = ((_p)->cb[8] & 0xf0) | (_prio))
#define RTMP_GET_PACKET_UP(_p)						((_p)->cb[8] & 0x0f)

// b4-b7 as fragment #
#define RTMP_SET_PACKET_FRAGMENTS(_p, number)		((_p)->cb[9] = ((_p)->cb[9] & 0x0f) | (number << 4))
#define RTMP_GET_PACKET_FRAGMENTS(_p)				(((_p)->cb[9] & 0xf0) >> 4)

// 0x0 ~0x7f: TX to AP's own BSS which has the specified AID (this value also as MAC table index)
// 0x80~0xff: TX to a WDS link. b0~6: WDS index
#define RTMP_SET_PACKET_WCID(_p, _wcid)				((_p)->cb[10] = _wcid)
#define RTMP_GET_PACKET_WCID(_p) 					((_p)->cb[10])

// 0xff: PKTSRC_NDIS, others: local TX buffer index. This value affects how to a packet
#define RTMP_SET_PACKET_SOURCE(_p, _pktsrc) 		((_p)->cb[11]= _pktsrc)
#define RTMP_GET_PACKET_SOURCE(_p)					((_p)->cb[11])

// b0~2: RTS/CTS-to-self protection method
#define RTMP_SET_PACKET_RTS(_p, _num)				((_p)->cb[12] = ((_p)->cb[12] & 0xf8) | (_num))
#define RTMP_GET_PACKET_RTS(_p) 					((_p)->cb[12] & 0x07)

// b3~7: TX rate index
#define RTMP_SET_PACKET_TXRATE(_p, _rate)			((_p)->cb[13] = ((_p)->cb[13] & 0x07) | (_rate << 3))
#define RTMP_GET_PACKET_TXRATE(_p)					(((_p)->cb[13] & 0xf8) >> 3)




#define PKTSRC_NDIS 			0x7f
#define PKTSRC_DRIVER			0x0f

#define	MAKE_802_3_HEADER(_p, _pMac1, _pMac2, _pType)					\
{																		\
	NdisMoveMemory(_p, _pMac1, MAC_ADDR_LEN);							\
	NdisMoveMemory((_p + MAC_ADDR_LEN), _pMac2, MAC_ADDR_LEN);			\
	NdisMoveMemory((_p + MAC_ADDR_LEN * 2), _pType, LENGTH_802_3_TYPE);	\
}

// if pData has no LLC/SNAP (neither RFC1042 nor Bridge tunnel), keep it that way.
// else if the received frame is LLC/SNAP-encaped IPX or APPLETALK, preserve the LLC/SNAP field 
// else remove the LLC/SNAP field from the result Ethernet frame
// Patch for WHQL only, which did not turn on Netbios but use IPX within its payload
// Note:
//	   _pData & _DataSize may be altered (remove 8-byte LLC/SNAP) by this MACRO
//	   _pRemovedLLCSNAP: pointer to removed LLC/SNAP; NULL is not removed
#define CONVERT_TO_802_3(_p8023hdr, _pDA, _pSA, _pData, _DataSize, _pRemovedLLCSNAP)	  \
{																		\
	char LLC_Len[2];													\
																		\
	_pRemovedLLCSNAP = NULL;											\
	if (NdisEqualMemory(SNAP_802_1H, _pData, 6)  || 					\
		NdisEqualMemory(SNAP_BRIDGE_TUNNEL, _pData, 6)) 				\
	{																	\
		PUCHAR pProto = _pData + 6; 									\
																		\
		if ((NdisEqualMemory(IPX, pProto, 2) || NdisEqualMemory(APPLE_TALK, pProto, 2)) &&	\
			NdisEqualMemory(SNAP_802_1H, _pData, 6))					\
		{																\
			LLC_Len[0] = (UCHAR)(_DataSize / 256);						\
			LLC_Len[1] = (UCHAR)(_DataSize % 256);						\
			MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);			\
		}																\
		else															\
		{																\
			MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, pProto);			\
			_pRemovedLLCSNAP = _pData;									\
			_DataSize -= LENGTH_802_1_H;								\
			_pData += LENGTH_802_1_H;									\
		}																\
	}																	\
	else																\
	{																	\
		LLC_Len[0] = (UCHAR)(_DataSize / 256);							\
		LLC_Len[1] = (UCHAR)(_DataSize % 256);							\
		MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);				\
	}																	\
}

#define RECORD_LATEST_RX_DATA_RATE(_pAd, _pRxD) 								\
{																				\
	if ((_pRxD)->Ofdm)															\
		(_pAd)->LastRxRate = OfdmSignalToRateId[(_pRxD)->PlcpSignal & 0x0f];	\
	else if ((_pRxD)->PlcpSignal == 10) 										\
		(_pAd)->LastRxRate = RATE_1;											\
	else if ((_pRxD)->PlcpSignal == 20) 										\
		(_pAd)->LastRxRate = RATE_2;											\
	else if ((_pRxD)->PlcpSignal == 55) 										\
		(_pAd)->LastRxRate = RATE_5_5;											\
	else																		\
		(_pAd)->LastRxRate = RATE_11;											\
}
 
// INFRA mode- Address 1 - AP, Address 2 - this STA, Address 3 - DA
// ADHOC mode- Address 1 - DA, Address 2 - this STA, Address 3 - BSSID
//iverson 2007 1126
#ifdef WMM_SUPPORT
#define MAKE_802_11_HEADER(_pAd, _80211hdr, _pDA, _seq) 						\
{																				\
	NdisZeroMemory(&_80211hdr, sizeof(HEADER_802_11));							\
	if (INFRA_ON(_pAd)) 														\
	{																			\
		COPY_MAC_ADDR(_80211hdr.Addr1, _pAd->PortCfg.Bssid);					\
		COPY_MAC_ADDR(_80211hdr.Addr3, _pDA);									\
		_80211hdr.FC.ToDs = 1;													\
	}																			\
	else																		\
	{																			\
		COPY_MAC_ADDR(_80211hdr.Addr1, _pDA);									\
		COPY_MAC_ADDR(_80211hdr.Addr3, _pAd->PortCfg.Bssid);					\
	}																			\
	COPY_MAC_ADDR(_80211hdr.Addr2, _pAd->CurrentAddress);						\
	_80211hdr.Sequence = _seq;													\
	_80211hdr.FC.Type = BTYPE_DATA; 											\
                                                                                \
    if (_pAd->PortCfg.bAPSDForcePowerSave)									    \
    {																			\
    	_80211hdr.FC.PwrMgmt = PWR_SAVE;									    \
    }																			\
    else																		\
    {																			\
        _80211hdr.FC.PwrMgmt = (_pAd->PortCfg.Psm == PWR_SAVE);                 \
    }																			\
}
#else
#define MAKE_802_11_HEADER(_pAd, _80211hdr, _pDA, _seq) 						\
{																				\
	NdisZeroMemory(&_80211hdr, sizeof(HEADER_802_11));							\
	if (INFRA_ON(_pAd)) 														\
	{																			\
		COPY_MAC_ADDR(_80211hdr.Addr1, _pAd->PortCfg.Bssid);					\
		COPY_MAC_ADDR(_80211hdr.Addr3, _pDA);									\
		_80211hdr.FC.ToDs = 1;													\
	}																			\
	else																		\
	{																			\
		COPY_MAC_ADDR(_80211hdr.Addr1, _pDA);									\
		COPY_MAC_ADDR(_80211hdr.Addr3, _pAd->PortCfg.Bssid);					\
	}																			\
	COPY_MAC_ADDR(_80211hdr.Addr2, _pAd->CurrentAddress);						\
	_80211hdr.Sequence = _seq;													\
	_80211hdr.FC.Type = BTYPE_DATA; 											\
	_80211hdr.FC.PwrMgmt = (_pAd->PortCfg.Psm == PWR_SAVE); 					\
}
#endif 
//Need to collect each ant's rssi concurrently
//rssi1 is report to pair2 Ant and rss2 is reprot to pair1 Ant when 4 Ant
#define COLLECT_RX_ANTENNA_AVERAGE_RSSI(_pAd, _rssi1, _rssi2)					\
{																				\
	SHORT	AvgRssi;															\
	UCHAR	UsedAnt;															\
	if (_pAd->RxAnt.EvaluatePeriod == 0)									\
	{																		\
		UsedAnt = _pAd->RxAnt.Pair1PrimaryRxAnt;							\
		AvgRssi = _pAd->RxAnt.Pair1AvgRssi[UsedAnt];						\
		if (AvgRssi < 0)													\
			AvgRssi = AvgRssi - (AvgRssi >> 3) + _rssi1;					\
		else																\
			AvgRssi = _rssi1 << 3;											\
		_pAd->RxAnt.Pair1AvgRssi[UsedAnt] = AvgRssi;						\
	}																		\
	else																	\
	{																		\
		UsedAnt = _pAd->RxAnt.Pair1SecondaryRxAnt;							\
		AvgRssi = _pAd->RxAnt.Pair1AvgRssi[UsedAnt];						\
		if ((AvgRssi < 0) && (_pAd->RxAnt.FirstPktArrivedWhenEvaluate))		\
			AvgRssi = AvgRssi - (AvgRssi >> 3) + _rssi1;					\
		else																\
		{																	\
			_pAd->RxAnt.FirstPktArrivedWhenEvaluate = TRUE;					\
			AvgRssi = _rssi1 << 3;											\
		}																	\
		_pAd->RxAnt.Pair1AvgRssi[UsedAnt] = AvgRssi;						\
		_pAd->RxAnt.RcvPktNumWhenEvaluate++;								\
	}																		\
}

#define RELEASE_NDIS_PACKET(_pAd, _pSkb)								\
{																		\
	if (RTMP_GET_PACKET_SOURCE(_pSkb) == PKTSRC_NDIS)					\
	{																	\
		RTUSBFreeSkbBuffer(_pSkb);										\
		_pAd->RalinkCounters.PendingNdisPacketCount --; 				\
	}																	\
	else																\
		RTUSBFreeSkbBuffer(_pSkb);										\
}

#define EnqueueCmd(cmdq, cmdqelmt)		\
{										\
	if (cmdq->size == 0)				\
		cmdq->head = cmdqelmt;			\
	else								\
		cmdq->tail->next = cmdqelmt;	\
	cmdq->tail = cmdqelmt;				\
	cmdqelmt->next = NULL;				\
	cmdq->size++;						\
}

// Free Tx ring descriptor MACRO
// This can only called from complete function since it will change the IO counters
#define	FREE_TX_RING(_p, _b, _t)			\
{										\
	(_t)->InUse 	 = FALSE;			\
	(_t)->LastOne	 = FALSE;			\
	(_t)->IRPPending = FALSE;			\
	(_t)->bWaitingBulkOut = FALSE;		\
	(_t)->BulkOutSize= 0;				\
	(_p)->NextBulkOutIndex[_b] = (((_p)->NextBulkOutIndex[_b] + 1) % TX_RING_SIZE);	\
	atomic_dec(&(_p)->TxCount); \
}


//2007/12/12:KH add to fix compiled bug(START)
#if 0
 #if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
 #undef __wait_event_interruptible_timeout
 #undef wait_event_interruptible_timeout
 #define __wait_event_interruptible_timeout(wq, condition, ret)		\
 do {									\
 	wait_queue_t __wait;						\
 	init_waitqueue_entry(&__wait, current);				\
 									\
 	add_wait_queue(&wq, &__wait);					\
 	for (;;) {							\
 		set_current_state(TASK_INTERRUPTIBLE);			\
 		if (condition)						\
 			break;						\
 		if (!signal_pending(current)) {				\
 			ret = schedule_timeout(ret);			\
 			if (!ret)					\
 				break;					\
 			continue;					\
 		}							\
 		ret = -ERESTARTSYS;					\
 		break;							\
 	}								\
 	current->state = TASK_RUNNING;					\
 	remove_wait_queue(&wq, &__wait);				\
 } while (0)

 #define wait_event_interruptible_timeout(wq, condition, timeout)	\
 ({									\
 	long __ret = timeout;						\
 	if (!(condition))						\
 		__wait_event_interruptible_timeout(wq, condition, __ret); \
 	__ret;								\
 })
 #endif
 #endif
 //2007/12/12:KH add to fix compiled bug(END)


#define	LOCAL_TX_RING_EMPTY(_p, _i)		(((_p)->TxContext[_i][(_p)->NextBulkOutIndex[_i]].InUse) == FALSE)

typedef	struct _CmdQElmt	{
	UINT				command;
	PVOID				buffer;
	ULONG				bufferlength;
	BOOLEAN				CmdFromNdis;
	BOOLEAN				SetOperation;
	BOOLEAN				InUse;
	struct _CmdQElmt	*next;
}	CmdQElmt, *PCmdQElmt;

typedef	struct	_CmdQ	{
	UINT		size;
	CmdQElmt	*head;
	CmdQElmt	*tail;
}	CmdQ, *PCmdQ;


////////////////////////////////////////////////////////////////////////////
// The TX_BUFFER structure forms the transmitted USB packet to the device
////////////////////////////////////////////////////////////////////////////
typedef struct __TX_BUFFER{
	TXD_STRUC		TxDesc;
	union	{
		UCHAR			WirelessPacket[2342];
		HEADER_802_11	NullFrame;
		PSPOLL_FRAME	PsPollPacket;
		RTS_FRAME		RTSFrame;
//	};
	}u;  // edit by johnli, fix compiler error for some platforms
} TX_BUFFER, *PTX_BUFFER;

////////////////////////////////////////////////////////////////////////////
// The RTS_BUFFER structure forms the transmitted USB packet to the device
////////////////////////////////////////////////////////////////////////////
typedef struct __RTS_BUFFER{
	TXD_STRUC	   TxDesc;
	UCHAR		   RTSPacket[16];
} RTS_BUFFER, *PRTS_BUFFER;

// used to track driver-generated write irps 
typedef struct _TX_CONTEXT
{
	PVOID			pAd;			//Initialized in MiniportInitialize
	PURB			pUrb;			//Initialized in MiniportInitialize
//	PIRP			pIrp;			//Initialized in MiniportInitialize, used to cancel pending bulk out
	PTX_BUFFER		TransferBuffer;	//Initialized in MiniportInitialize
	ULONG			BulkOutSize;
	UCHAR			BulkOutPipeId;
	BOOLEAN			InUse;
	BOOLEAN			bWaitingBulkOut;
	BOOLEAN			IRPPending;
	BOOLEAN			LastOne;
}	TX_CONTEXT, *PTX_CONTEXT, **PPTX_CONTEXT;

typedef enum _Pendingirp {
	NONEPENDING,
	IRP0PENDING,
	IRP1PENDING
}	PendingIRP;

typedef enum _BEACON_INDEX {
	BEACON0,
	BEACON1
}	BEACON_INDEX;


#define   IRPLOCK_COMPLETED 		0
#define   IRPLOCK_CANCELABLE		1
#define   IRPLOCK_CANCE_START		2
#define   IRPLOCK_CANCE_COMPLETE	3
//
// Structure to keep track of receive packets and buffers to indicate
// receive data to the protocol.
//
typedef struct _RX_CONTEXT
{
	PUCHAR				TransferBuffer; 
	PVOID				pAd;
//	PIRP				pIrp;//used to cancel pending bulk in.
	PURB				pUrb;
	BOOLEAN				InUse;
	BOOLEAN				IRPPending;		// TODO: To be removed
	atomic_t			IrpLock;
}	RX_CONTEXT, *PRX_CONTEXT;

//
// Register set pair for initialzation register set definition
//
typedef struct	_RTMP_REG_PAIR
{
	ULONG	Register;
	ULONG	Value;
}	RTMP_REG_PAIR, *PRTMP_REG_PAIR;

typedef struct	_BBP_REG_PAIR
{
	UCHAR	Register;
	UCHAR	Value;
}	BBP_REG_PAIR, *PBBP_REG_PAIR;

//
// Register set pair for initialzation register set definition
//
typedef struct	_RTMP_RF_REGS
{
	UCHAR	Channel;
	ULONG	R1;
	ULONG	R2;
	ULONG	R3;
	ULONG	R4;
}	RTMP_RF_REGS, *PRTMP_RF_REGS;

//
//	Statistic counter structure
//
typedef struct _COUNTER_802_3
{
	// General Stats
	ULONG		GoodTransmits;
	ULONG		GoodReceives;
	ULONG		TxErrors;
	ULONG		RxErrors;
	ULONG		RxNoBuffer;

	// Ethernet Stats
	ULONG		RcvAlignmentErrors;
	ULONG		OneCollision;
	ULONG		MoreCollisions;

}	COUNTER_802_3, *PCOUNTER_802_3;

typedef struct _COUNTER_802_11 {
	ULONG			Length;
	LARGE_INTEGER	TransmittedFragmentCount;
	LARGE_INTEGER	MulticastTransmittedFrameCount;
	LARGE_INTEGER	FailedCount;
	LARGE_INTEGER	NoRetryCount;
	LARGE_INTEGER	RetryCount;
	LARGE_INTEGER	MultipleRetryCount;
	LARGE_INTEGER	RTSSuccessCount;
	LARGE_INTEGER	RTSFailureCount;
	LARGE_INTEGER	ACKFailureCount;
	LARGE_INTEGER	FrameDuplicateCount;
	LARGE_INTEGER	ReceivedFragmentCount;
	LARGE_INTEGER	MulticastReceivedFrameCount;
	LARGE_INTEGER	FCSErrorCount;
} COUNTER_802_11, *PCOUNTER_802_11;

typedef struct _COUNTER_RALINK {
	ULONG			TransmittedByteCount;	// both successful and failure, used to calculate TX throughput
	ULONG			ReceivedByteCount;		// both CRC okay and CRC error, used to calculate RX throughput
	ULONG			BeenDisassociatedCount;
	ULONG			BadCQIAutoRecoveryCount;
	ULONG			PoorCQIRoamingCount;
	ULONG			MgmtRingFullCount;
	ULONG			RxCount;
	ULONG			RxRingErrCount;
	ULONG			KickTxCount;
	ULONG			TxRingErrCount;
	LARGE_INTEGER	RealFcsErrCount;
	ULONG			PendingNdisPacketCount;

	ULONG			OneSecOsTxCount[NUM_OF_TX_RING];
	ULONG			OneSecDmaDoneCount[NUM_OF_TX_RING];
	ULONG			OneSecTxDoneCount;
	ULONG			OneSecTxAggregationCount;
	ULONG			OneSecRxAggregationCount;

	ULONG			OneSecTxNoRetryOkCount;
	ULONG			OneSecTxRetryOkCount;
	ULONG			OneSecTxFailCount;
	ULONG			OneSecFalseCCACnt;		// CCA error count, for debug purpose, might move to global counter
	ULONG			OneSecRxOkCnt;			// RX without error
	ULONG			OneSecRxFcsErrCnt;		// CRC error
	ULONG			OneSecBeaconSentCnt;
} COUNTER_RALINK, *PCOUNTER_RALINK;

typedef struct _COUNTER_DRS {
	// to record the each TX rate's quality. 0 is best, the bigger the worse.
	USHORT			TxQuality[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR			PER[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR			TxRateUpPenalty;	  // extra # of second penalty due to last unstable condition
	ULONG			CurrTxRateStableTime; // # of second in current TX rate
	BOOLEAN 		fNoisyEnvironment;
	UCHAR			LastSecTxRateChangeAction; // 0: no change, 1:rate UP, 2:rate down
} COUNTER_DRS, *PCOUNTER_DRS;

typedef struct _COUNTER_QA {
	LARGE_INTEGER	CRCErrorCount;
	LARGE_INTEGER	RXOverFlowCount;
	LARGE_INTEGER	PHYErrorCount;
	LARGE_INTEGER	FalseCCACount;
	LARGE_INTEGER	U2MDataCount;
	LARGE_INTEGER	OtherDataCount;
	LARGE_INTEGER	BeaconCount;
	LARGE_INTEGER	othersCount;
}	COUNTER_QA, *PCOUNTER_QA;

//
//	Arcfour Structure Added by PaulWu
//
typedef struct	_ARCFOUR
{
	UINT			X;
	UINT			Y;
	UCHAR			STATE[256];
}	ARCFOURCONTEXT, *PARCFOURCONTEXT;

typedef	struct PACKED _IV_CONTROL_
{
	union PACKED
	{
		struct PACKED 
		{
			UCHAR		rc0;
			UCHAR		rc1;
			UCHAR		rc2;

			union PACKED
			{
				struct PACKED
				{
#ifdef BIG_ENDIAN
					UCHAR	KeyID:2;
					UCHAR	ExtIV:1;
					UCHAR	Rsvd:5;
#else
					UCHAR	Rsvd:5;
					UCHAR	ExtIV:1;
					UCHAR	KeyID:2;
#endif
				}	field;
				UCHAR		Byte;
			}	CONTROL;
		}	field;
		
		ULONG	word;
	}	IV16;
	
	ULONG	IV32;
}	TKIP_IV, *PTKIP_IV;


// Shared key data structure
typedef struct	_WEP_KEY {
	UCHAR	KeyLen; 					// Key length for each key, 0: entry is invalid
	UCHAR	Key[MAX_LEN_OF_KEY];		// right now we implement 4 keys, 128 bits max
}	WEP_KEY, *PWEP_KEY;

#if 0 // remove to rtmp_type.h
typedef struct _CIPHER_KEY {
	UCHAR	BssId[6];
	UCHAR	CipherAlg;			// 0-none, 1:WEP64, 2:WEP128, 3:TKIP, 4:AES, 5:CKIP64, 6:CKIP128
	UCHAR	KeyLen; 			// Key length for each key, 0: entry is invalid
	UCHAR	Key[16];			// right now we implement 4 keys, 128 bits max
	UCHAR	RxMic[8];
	UCHAR	TxMic[8];
	UCHAR	TxTsc[6];			// 48bit TSC value
	UCHAR	RxTsc[6];			// 48bit TSC value
	UCHAR	Type;				// Indicate Pairwise/Group when reporting MIC error
}	CIPHER_KEY, *PCIPHER_KEY;
#endif // remove

typedef struct _BBP_TUNING_STRUCT {
	BOOLEAN 	Enable;
	UCHAR		FalseCcaCountUpperBound;  // 100 per sec
	UCHAR		FalseCcaCountLowerBound;  // 10 per sec
	UCHAR		R17LowerBound;			  // specified in E2PROM
	UCHAR		R17UpperBound;			  // 0x68 according to David Tung
	UCHAR		CurrentR17Value;
} BBP_TUNING, *PBBP_TUNING;

typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
	UCHAR	  EvaluatePeriod;		 // 0:not evalute status, 1: evaluate status, 2: switching status
	UCHAR	  Pair1PrimaryRxAnt;	 // 0:Ant-E1, 1:Ant-E2
	UCHAR	  Pair1SecondaryRxAnt;	 // 0:Ant-E1, 1:Ant-E2
	UCHAR	  Pair2PrimaryRxAnt;	 // 0:Ant-E3, 1:Ant-E4
	UCHAR	  Pair2SecondaryRxAnt;	 // 0:Ant-E3, 1:Ant-E4
	SHORT	  Pair1AvgRssi[2];		 // AvgRssi[0]:E1, AvgRssi[1]:E2
	SHORT	  Pair2AvgRssi[2];		 // AvgRssi[0]:E3, AvgRssi[1]:E4
	SHORT	  Pair1LastAvgRssi; 	 // 
	SHORT	  Pair2LastAvgRssi; 	 // 
	ULONG	  RcvPktNumWhenEvaluate;
	BOOLEAN   FirstPktArrivedWhenEvaluate;
	RALINK_TIMER_STRUCT    RxAntDiversityTimer;
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;

typedef struct {
	BOOLEAN 	Enable;
	UCHAR		Delta;
	BOOLEAN 	PlusSign;
} CCK_TX_POWER_CALIBRATE, *PCCK_TX_POWER_CALIBRATE;

//
// Receive Tuple Cache Format
//
typedef struct	_TUPLE_CACHE	{
	BOOLEAN 		Valid;
	UCHAR			MacAddress[MAC_ADDR_LEN];
	USHORT			Sequence; 
	USHORT			Frag;
}	TUPLE_CACHE, *PTUPLE_CACHE;

//
// Fragment Frame structure
//
typedef struct	_FRAGMENT_FRAME {
	UCHAR		Header802_11[LENGTH_802_11];  // add by johnli, fix WPS & WPAPSK/WPA2PSK bugs for receiving EAPoL fragmentation packets
	UCHAR		Header802_3[LENGTH_802_3];
	UCHAR		Header_LLC[LENGTH_802_1_H];
	UCHAR		Buffer[LENGTH_802_3 + MAX_FRAME_SIZE];	// Add header to prevent NETBUEI continuous buffer isssue
	ULONG		RxSize;
	USHORT		Sequence;
	USHORT		LastFrag;
	ULONG		Flags;			// Some extra frame information. bit 0: LLC presented
}	FRAGMENT_FRAME, *PFRAGMENT_FRAME;

//
// Tkip Key structure which RC4 key & MIC calculation
//
typedef struct	_TKIP_KEY_INFO	{
	UINT		nBytesInM;	// # bytes in M for MICKEY
	ULONG		IV16;
	ULONG		IV32;	
	ULONG		K0; 		// for MICKEY Low
	ULONG		K1; 		// for MICKEY Hig
	ULONG		L;			// Current state for MICKEY
	ULONG		R;			// Current state for MICKEY
	ULONG		M;			// Message accumulator for MICKEY
	UCHAR		RC4KEY[16];
	UCHAR		MIC[8];
}	TKIP_KEY_INFO, *PTKIP_KEY_INFO;

//
// Private / Misc data, counters for driver internal use
//
typedef struct	__PRIVATE_STRUC {
	ULONG		SystemResetCnt; 		// System reset counter
	ULONG		TxRingFullCnt;			// Tx ring full occurrance number
	ULONG		PhyRxErrCnt;			// PHY Rx error count, for debug purpose, might move to global counter
	// Variables for WEP encryption / decryption in rtmp_wep.c
	ULONG			FCSCRC32;
	ARCFOURCONTEXT	WEPCONTEXT;
	// Tkip stuff
	TKIP_KEY_INFO	Tx;
	TKIP_KEY_INFO	Rx;
}	PRIVATE_STRUC, *PPRIVATE_STRUC;

#ifdef RALINK_ATE
typedef	struct _ATE_INFO {
	UCHAR	Mode;
	CHAR	TxPower;
	UCHAR	Addr1[6];
	UCHAR	Addr2[6];
	UCHAR	Addr3[6];
	UCHAR	Channel;
	ULONG	TxLength;
	ULONG	TxCount;
	ULONG	TxDoneCount;
	ULONG	TxRate;
	ULONG	RFFreqOffset;
}	ATE_INFO, *PATE_INFO;
#endif	// RALINK_ATE

// structure to tune BBP R17 "RX AGC VGC init"
typedef struct _BBP_R17_TUNING {
	BOOLEAN 	bEnable;
	UCHAR		R17LowerBoundG;
	UCHAR		R17LowerBoundA;
	UCHAR		R17UpperBoundG;
	UCHAR		R17UpperBoundA;
//	  UCHAR 	  LastR17Value;
//	  SHORT 	  R17Dec;	  // R17Dec = 0x79 - RssiToDbm, for old version R17Dec = 0.
//							  // This is signed value
	USHORT		FalseCcaLowerThreshold;  // default 100
	USHORT		FalseCcaUpperThreshold;  // default 512
	UCHAR		R17Delta;				 // R17 +- R17Delta whenever false CCA over UpperThreshold or lower than LowerThreshold
	UCHAR		R17CurrentValue;
	BOOLEAN		R17LowerUpperSelect; //Before LinkUp, Used LowerBound or UpperBound as R17 value.
} BBP_R17_TUNING, *PBBP_R17_TUNING;

// structure to store channel TX power
typedef struct _CHANNEL_TX_POWER {
	UCHAR	   Channel;
	CHAR	   Power;
}	CHANNEL_TX_POWER, *PCHANNEL_TX_POWER;

typedef enum _ABGBAND_STATE_ {
	UNKNOWN_BAND,
	BG_BAND,
	A_BAND,
} ABGBAND_STATE;

typedef struct _MLME_MEMORY_STRUCT {
	VOID *							AllocVa;	//Pointer to the base virtual address of the allocated memory
	struct _MLME_MEMORY_STRUCT		*Next;		//Pointer to the next virtual address of the allocated memory
}	MLME_MEMORY_STRUCT, *PMLME_MEMORY_STRUCT;

typedef struct	_MLME_MEMORY_HANDLER {
	BOOLEAN 				MemRunning; 		//The flag of the Mlme memory handler's status
	UINT					MemoryCount;		//Total nonpaged system-space memory not size
	UINT					InUseCount; 		//Nonpaged system-space memory in used counts
	UINT					UnUseCount; 		//Nonpaged system-space memory available counts
	UINT					PendingCount;		//Nonpaged system-space memory for free counts
	PMLME_MEMORY_STRUCT 	pInUseHead; 		//Pointer to the first nonpaed memory not used
	PMLME_MEMORY_STRUCT 	pInUseTail; 		//Pointer to the last nonpaged memory not used
	PMLME_MEMORY_STRUCT 	pUnUseHead; 		//Pointer to the first nonpaged memory in used
	PMLME_MEMORY_STRUCT 	pUnUseTail; 		//Pointer to the last nonpaged memory in used
	PULONG					MemFreePending[MAX_MLME_HANDLER_MEMORY];   //an array to keep pending free-memory's pointer (32bits)
}	MLME_MEMORY_HANDLER, *PMLME_MEMORY_HANDLER;

typedef struct _MLME_STRUCT {

	STATE_MACHINE			CntlMachine;
	STATE_MACHINE			AssocMachine;
	STATE_MACHINE			AuthMachine;
	STATE_MACHINE			AuthRspMachine;
	STATE_MACHINE			SyncMachine;
	STATE_MACHINE			WpaPskMachine;
	STATE_MACHINE_FUNC		AssocFunc[ASSOC_FUNC_SIZE];
	STATE_MACHINE_FUNC		AuthFunc[AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC		AuthRspFunc[AUTH_RSP_FUNC_SIZE];
	STATE_MACHINE_FUNC		SyncFunc[SYNC_FUNC_SIZE];
	STATE_MACHINE_FUNC		WpaPskFunc[WPA_PSK_FUNC_SIZE];

	ULONG					ChannelQuality;  // 0..100, Channel Quality Indication for Roaming
	ULONG					Now32;			 // latch the value of NdisGetSystemUpTime() 

	unsigned char 				bRunning;
	//spinlock_t				TaskLock;
	MLME_QUEUE				Queue;

	UINT					ShiftReg;
	
	RALINK_TIMER_STRUCT 	PeriodicTimer;
	RALINK_TIMER_STRUCT 	LinkDownTimer;
	ULONG					PeriodicRound;
	
	MLME_MEMORY_HANDLER 	MemHandler; 		//The handler of the nonpaged memory inside MLME  
} MLME_STRUCT, *PMLME_STRUCT;

//
// Management ring buffer format
//
typedef	struct	PACKED _MGMT_STRUC	{
	unsigned char		Valid;
	unsigned char*		pBuffer;
	unsigned int		Length;
}	MGMT_STRUC, *PMGMT_STRUC;

// structure for radar detection and channel switch
typedef struct _RADAR_DETECT_STRUCT {
	unsigned char		CSCount;			//Channel switch counter
	unsigned char		CSPeriod;			//Channel switch period (beacon count)
	unsigned char		RDCount;			//Radar detection counter
	unsigned char		RDMode;				//Radar Detection mode
	unsigned char		BBPR16;
	unsigned char		BBPR17;
	unsigned char		BBPR18;
	unsigned char		BBPR21;
	unsigned char		BBPR22;
	unsigned char		BBPR64;
	unsigned int		InServiceMonitorCount; // unit: sec
} RADAR_DETECT_STRUCT, *PRADAR_DETECT_STRUCT;

//
//	configuration and status
//
typedef struct _PORT_CONFIG {

	// MIB:ieee802dot11.dot11smt(1).dot11StationConfigTable(1)
	unsigned short		Psm;				  // power management mode	 (PWR_ACTIVE|PWR_SAVE)
	unsigned short		DisassocReason;
	unsigned char		DisassocSta[MAC_ADDR_LEN];
	unsigned short		DeauthReason;
	unsigned char		DeauthSta[MAC_ADDR_LEN];
	unsigned short		AuthFailReason;
	unsigned char		AuthFailSta[MAC_ADDR_LEN];

	NDIS_802_11_AUTHENTICATION_MODE 	AuthMode;		// This should match to whatever microsoft defined
	NDIS_802_11_WEP_STATUS				WepStatus;
	NDIS_802_11_WEP_STATUS				OrigWepStatus;	// Original wep status set from OID

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	int     	wx_auth_alg;	 // OPEN, SHARED, LEAP
	int     	wx_key_mgmt;	 // WPA-PSK, WPA-EAP, IEEE8021X, NONE
	int			wx_wpa_version; // WPA(IEEE802.11i/D3), RSN(WPA2/IEEE802.11i)
	int			wx_pairwise;    // CCMP, TKIP, NONE;
	int			wx_groupCipher; // CCMP, TKIP, WEP104, WEP40
//	UCHAR       wx_RSN_IE[MAX_WPA_IE_LEN];
//	UCHAR       wx_RSNIE_Len;		 // Used to save WPA/RSN IE temporary.
//	UCHAR		wx_psk[65];     // WPA pre-shared key.(64 hex-digitial or 8~63 ascii)
//	int			wx_eapol_flags; // bit(0): require dynamically generated unicast WEP key
								     // bit(1):require dynamically generated broadcast WEP key
									 // bit(2): Require both.
	unsigned char		wx_need_sync;	 //Notify that need to sync the wpa_supplicant configuration with our driver.
	unsigned char		bNativeWpa;	 // add by johnli, enable/disable native wpa supplicant
#endif
	// Add to support different cipher suite for WPA2/WPA mode
	NDIS_802_11_ENCRYPTION_STATUS		GroupCipher;		// Multicast cipher suite
	NDIS_802_11_ENCRYPTION_STATUS		PairCipher;			// Unicast cipher suite
	unsigned char								bMixCipher;			// Indicate current Pair & Group use different cipher suites
	unsigned short								RsnCapability;
	
	// MIB:ieee802dot11.dot11smt(1).dot11WEPDefaultKeysTable(3)
	CIPHER_KEY	PskKey; 				// WPA PSK mode PMK
	unsigned char		PTK[64];				// WPA PSK mode PTK
	BSSID_INFO	SavedPMK[PMKID_NO];
	unsigned int		SavedPMKNum;			// Saved PMKID number

	// WPA 802.1x port control, WPA_802_1X_PORT_SECURED, WPA_802_1X_PORT_NOT_SECURED
	unsigned char		PortSecured;
	unsigned char		RSN_IE[44];
	unsigned char		RSN_IELen;

//#ifdef RALINK_WPA_SUPPLICANT_SUPPORT	
	unsigned char     IEEE8021X;				// Enable or disable IEEE 802.1x 
	CIPHER_KEY	DesireSharedKey[4];		// Record user desired WEP keys	
	unsigned char		IEEE8021x_required_keys;				// Enable or disable dynamic wep key updating
	unsigned char     WPA_Supplicant;         // Enable or disable WPA_SUPPLICANT 
//#endif

	// For WPA countermeasures
	unsigned int		LastMicErrorTime;	// record last MIC error time
	unsigned int		MicErrCnt;			// Should be 0, 1, 2, then reset to zero (after disassoiciation).
	unsigned char 	bBlockAssoc;		// Block associate attempt for 60 seconds after counter measure occurred.
	// For WPA-PSK supplicant state
	WPA_STATE	WpaState;			// Default is SS_NOTUSE and handled by microsoft 802.1x
	unsigned char		ReplayCounter[8];
	unsigned char		ANonce[32]; 		// ANonce for WPA-PSK from aurhenticator
	unsigned char		SNonce[32]; 		// SNonce for WPA-PSK
	
	// MIB:ieee802dot11.dot11smt(1).dot11PrivacyTable(5)
	unsigned char								DefaultKeyId;
	NDIS_802_11_PRIVACY_FILTER			PrivacyFilter;	// PrivacyFilter enum for 802.1X


	// MIB:ieee802dot11.dot11mac(2).dot11OperationTable(1)
	unsigned short		RtsThreshold;			// in unit of BYTE
	unsigned short		FragmentThreshold;		// in unit of BYTE
	unsigned char 	bFragmentZeroDisable;	// Microsoft use 0 as disable 
	
	// MIB:ieee802dot11.dot11phy(4).dot11PhyTxPowerTable(3)
	unsigned char		TxPower;				// in unit of mW
	unsigned int		TxPowerPercentage;		// 0~100 %
	unsigned int		TxPowerDefault; 		// keep for TxPowerPercentage

	// MIB:ieee802dot11.dot11phy(4).dot11PhyDSSSTable(5)
	unsigned char		Channel;		  // current (I)BSS channel used in the station
	unsigned char       AdhocChannel;     // current (I)BSS channel used in the station
	unsigned char		CountryRegion;	  // Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel
	unsigned char		CountryRegionForABand;	// Enum of country region for A band

	
	// Copy supported rate from desired AP's beacon. We are trying to match
	// AP's supported and extended rate settings.
	unsigned char		SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	unsigned char		SupRateLen;
	unsigned char		ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	unsigned char		ExtRateLen;
	unsigned char		ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];

	unsigned int		BasicRateBitmap;		// backup basic ratebitmap

	//
	// other parameters not defined in standard MIB
	//
	unsigned char		DesireRate[MAX_LEN_OF_SUPPORTED_RATES]; 	 // OID_802_11_DESIRED_RATES
	unsigned char		MaxDesiredRate;
	unsigned char		DefaultMaxDesiredRate;
    unsigned char       BasicMlmeRate;          // Default Rate for sending MLME frames
	unsigned char		MlmeRate;
	unsigned char		RtsRate;				// RATE_xxx
	unsigned char		TxRate; 				// RATE_1, RATE_2, RATE_5_5, RATE_11, ...
	unsigned char		MaxTxRate;				// RATE_1, RATE_2, RATE_5_5, RATE_11

	unsigned char		Bssid[MAC_ADDR_LEN];
	unsigned short		BeaconPeriod; 
//2007/12/27:KH add one to the length of SSID to fix the bug of the maximum SSIDLen of SSID
	CHAR		Ssid[MAX_LEN_OF_SSID+1];		// NOT NULL-terminated
	unsigned char		SsidLen;					// the actual ssid length in used
	unsigned char		LastSsidLen;				// the actual ssid length in used
	//2007/12/27:KH add one to the length of SSID to fix the bug of the maximum SSIDLen of SSID
	CHAR		LastSsid[MAX_LEN_OF_SSID+1];	// NOT NULL-terminated
	unsigned char		LastBssid[MAC_ADDR_LEN];

	unsigned char		BssType;				// BSS_INFRA or BSS_ADHOC
	unsigned short		AtimWin;				// used when starting a new IBSS
	
	unsigned char		RssiTrigger;
	unsigned char		RssiTriggerMode;		// RSSI_TRIGGERED_UPON_BELOW_THRESHOLD or RSSI_TRIGGERED_UPON_EXCCEED_THRESHOLD
	unsigned short		DefaultListenCount; 	// default listen count;
	unsigned int		WindowsPowerMode;			// Power mode for AC power
	unsigned int		WindowsBatteryPowerMode;	// Power mode for battery if exists
	unsigned char 	bWindowsACCAMEnable;		// Enable CAM power mode when AC on
	unsigned char 	bAutoReconnect; 		// Set to TRUE when setting OID_802_11_SSID with no matching BSSID
	
	unsigned char		LastRssi;				// last received BEACON's RSSI
	unsigned char		LastRssi2;				// last received BEACON's RSSI for smart antenna
	unsigned short		AvgRssi;				// last 8 BEACON's average RSSI
	unsigned short		AvgRssiX8;				// sum of last 8 BEACON's RSSI
	unsigned int		NumOfAvgRssiSample;

	unsigned int		LastBeaconRxTime;		// OS's timestamp of the last BEACON RX time
	unsigned int		Last11bBeaconRxTime;	// OS's timestamp of the last 11B BEACON RX time
	unsigned int		LastScanTime;			// Record last scan time for issue BSSID_SCAN_LIST
	unsigned int		ScanCnt;			  // Scan counts since most recent SSID, BSSID, SCAN OID request
	unsigned char 	bSwRadio;				// Software controlled Radio On/Off, TRUE: On
	unsigned char 	bHwRadio;				// Hardware controlled Radio On/Off, TRUE: On
	unsigned char 	bRadio; 				// Radio state, And of Sw & Hw radio state
	unsigned char 	bHardwareRadio; 		// Hardware controlled Radio enabled
	unsigned char 	bShowHiddenSSID;	  // Show all known SSID in SSID list get operation


	// PHY specification
	unsigned char	  PhyMode;			// PHY_11A, PHY_11B, PHY_11BG_MIXED, PHY_ABG_MIXED
	unsigned short	  Dsifs;			// in units of usec
	unsigned short	  TxPreamble;		// Rt802_11PreambleLong, Rt802_11PreambleShort, Rt802_11PreambleAuto

	// New for WPA, windows want us to to keep association information and
	// Fixed IEs from last association response
	NDIS_802_11_ASSOCIATION_INFORMATION 	AssocInfo;
	unsigned char					ReqVarIELen;				// Length of next VIE include EID & Length
	unsigned char					ReqVarIEs[MAX_VIE_LEN];
	unsigned char					ResVarIELen;				// Length of next VIE include EID & Length
	unsigned char					ResVarIEs[MAX_VIE_LEN];

	unsigned int					EnableTurboRate;	  // 1: enable 72/100 Mbps whenever applicable, 0: never use 72/100 Mbps
	unsigned int					UseBGProtection;	  // 0:AUTO, 1-always ON,2-always OFF
	unsigned int					UseShortSlotTime;	  // 0: disable, 1 - use short slot (9us)


	// EDCA Qos
	unsigned char 				bWmmCapable;		// 0:disable WMM, 1:enable WMM
	QOS_CAPABILITY_PARM		APQosCapability;	// QOS capability of the current associated AP
	EDCA_PARM				APEdcaParm; 		// EDCA parameters of the current associated AP
	QBSS_LOAD_PARM			APQbssLoad; 		// QBSS load of the current associated AP

	unsigned char					bEnableTxBurst; 		// 0: disable, 1: enable TX PACKET BURST
	unsigned char					bAggregationCapable;	// 1: enable TX aggregation when the peer supports it
	unsigned char 				bUseZeroToDisableFragment;			// Microsoft use 0 as disable
	unsigned char 				bIEEE80211H;			// 1: enable IEEE802.11h spec.

	// a bitmap of unsigned char flags. each bit represent an operation status of a particular 
	// unsigned char control, either ON or OFF. These flags should always be accessed via
	// OPSTATUS_TEST_FLAG(), OPSTATUS_SET_FLAG(), OP_STATUS_CLEAR_FLAG() macros.
	// see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition
	unsigned int					OpStatusFlags;

	unsigned char					AckPolicy[4];		// ACK policy of the specified AC. see ACK_xxx
		
	ABGBAND_STATE			BandState;			// For setting BBP used on B/G or A mode

	unsigned int					AdhocMode;			// 0:WIFI mode (11b rates only), 1: b/g mixed, 2: 11g only, 3: 11a only, 4: 11abg mixed
	
	RALINK_TIMER_STRUCT		QuickResponeForRateUpTimer;
	unsigned char					QuickResponeForRateUpTimerRunning;
	RALINK_TIMER_STRUCT	WpaDisassocAndBlockAssocTimer; //BensonLiu 07-11-22 add for countermeasure

    // Fast Roaming
    unsigned char                 bFastRoaming;       // 0:disable fast roaming, 1:enable fast roaming
    unsigned int                   dBmToRoam;          // the condition to roam when receiving Rssi less than this value. It's negative value.

    RADAR_DETECT_STRUCT	    RadarDetect;
	
    unsigned char                 bGetAPConfig;


} PORT_CONFIG, *PPORT_CONFIG;


// This data structure keep the current active BSS/IBSS's configuration that this STA
// had agreed upon joining the network. Which means these parameters are usually decided
// by the BSS/IBSS creator instead of user configuration. Data in this data structurre 
// is valid only when either ADHOC_ON(pAd) or INFRA_ON(pAd) is TRUE.
// Normally, after SCAN or failed roaming attempts, we need to recover back to
// the current active settings.
typedef struct _ACTIVE_CONFIG {
	unsigned short		Aid;
	unsigned short		AtimWin;				// in kusec; IBSS parameter set element
	unsigned short		CapabilityInfo;
	unsigned short		CfpMaxDuration;
	unsigned short		CfpPeriod;
	
	// Copy supported rate from desired AP's beacon. We are trying to match
	// AP's supported and extended rate settings.
	unsigned char		SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	unsigned char		ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	unsigned char		SupRateLen;
	unsigned char		ExtRateLen;
} ACTIVE_CONFIG, *PACTIVE_CONFIG;

#ifdef BLOCK_NET_IF
typedef struct _BLOCK_QUEUE_ENTRY
{
	unsigned char SwTxQueueBlockFlag;
	LIST_HEADER NetIfList;
} BLOCK_QUEUE_ENTRY, *PBLOCK_QUEUE_ENTRY;
#endif // BLOCK_NET_IF //

#ifndef IW_ESSID_MAX_SIZE
/* Maximum size of the ESSID and NICKN strings */
#define IW_ESSID_MAX_SIZE	32
#endif

//
//	The miniport adapter structure
//
typedef struct _RTMP_ADAPTER
{	
	//----------------------------
	// Linux specific 
	//----------------------------
	//CHAR							nickn[IW_ESSID_MAX_SIZE+1]; // nickname, only used in the iwconfig i/f 
	struct usb_device				*pUsb_Dev;
	struct net_device				*net_dev;
	//struct tasklet_struct			rx_bh;		
	//struct usb_config_descriptor	*config;
	//devctrlrequest					*devreq;
	/* The device we're working with
	 * It's important to note:
	 *	  (o) you must hold dev_semaphore to change pUsb_Dev
	 */
	//struct semaphore	usbdev_semaphore;		/* protect	usb */
	// Thread
#ifdef LINUX
	struct semaphore	mlme_semaphore;			/* to sleep thread on	*/
	struct semaphore	RTUSBCmd_semaphore;		/* to sleep thread on	*/
#else
	int	mlme_semaphore;			/* to sleep thread on	*/
	int	RTUSBCmd_semaphore;		/* to sleep thread on	*/
#endif
	//2008/01/07:KH add to solve the racing condition of Mac Registers
	int MaCRegWrite_semaphore;
	//struct completion	MlmeThreadNotify;					/* thread begin/end	 */
	//struct completion	CmdThreadNotify;					/* thread begin/end	 */
	pid_t				MLMEThr_pid;
	pid_t				RTUSBCmdThr_pid;
	//wait_queue_head_t	*wait;
	
#if WIRELESS_EXT >= 12
	struct iw_statistics iw_stats;
#else
#error "Invalid WIRELESS_EXT"
#endif
//	struct net_device_stats stats;

	ULONG				VendorDesc;		// VID/PID
	INT					ioctl_if_type;
	
	// resource for software backlog queues
	//QUEUE_HEADER			TxSwQueue[NUM_OF_TX_RING];	// 4 AC + 1 HCCA
	MGMT_STRUC				MgmtRing[MGMT_RING_SIZE];
	ULONG					MaxTxQueueSize;

	// outgoing BEACON frame buffer and corresponding TXD 
	TXD_STRUC				BeaconTxD;
	unsigned char					BeaconBuf[256]; // NOTE: BeaconBuf should be 4-byte aligned

	// pre-build PS-POLL and NULL frame upon link up. for efficiency purpose.
	PSPOLL_FRAME			PsPollFrame;
	HEADER_802_11			NullFrame;

	// configuration: read from Registry & E2PROM
	BOOLEAN 				bLocalAdminMAC; 					// Use user changed MAC
	UCHAR					PermanentAddress[MAC_ADDR_LEN]; 	// Factory default MAC address
	UCHAR					CurrentAddress[MAC_ADDR_LEN];		// User changed MAC address

	MLME_STRUCT 			Mlme;

	// ---------------------------
	// STA specific configuration
	// ---------------------------
	PORT_CONFIG 			PortCfg;			// user desired settings
	ACTIVE_CONFIG			ActiveCfg;			// valid only when ADHOC_ON(pAd) || INFRA_ON(pAd)
	MLME_AUX				MlmeAux;			// temporary settings used during MLME state machine
	BSS_TABLE				ScanTab;			// store the latest SCAN result

	// encryption/decryption KEY tables
	CIPHER_KEY				SharedKey[4];
//	CIPHER_KEY				PairwiseKey[64];		// for AP only

	// Boolean control for packet filter
	BOOLEAN 				bAcceptDirect;
	BOOLEAN 				bAcceptMulticast;
	BOOLEAN 				bAcceptBroadcast;
	BOOLEAN 				bAcceptAllMulticast;
	
	// 802.3 multicast support
	ULONG					NumberOfMcastAddresses; 	// Number of mcast entry exists
	UCHAR					McastTable[MAX_MCAST_LIST_SIZE][MAC_ADDR_LEN];		// Mcast list


	// RX Tuple chahe for duplicate frame check
	TUPLE_CACHE				TupleCache[MAX_CLIENT];		// Maximum number of tuple caches, only useful in Ad-Hoc
	UCHAR					TupleCacheLastUpdateIndex;	// 0..MAX_CLIENT-1

	// RX re-assembly buffer for fragmentation
	FRAGMENT_FRAME			FragFrame;					// Frame storage for fragment frame
	
	// various Counters 
	COUNTER_802_3			Counters8023;				// 802.3 counters
	COUNTER_802_11			WlanCounters;				// 802.11 MIB counters
	COUNTER_RALINK			RalinkCounters;				// Ralink propriety counters
	COUNTER_DRS 			DrsCounters;				// counters for Dynamic TX Rate Switching
	PRIVATE_STRUC			PrivateInfo;				// Private information & counters

	// Counters for 802.3 & generic.
	// Add 802.11 specific counters later
	COUNTER_802_3			Counters;					// 802.3 counters
	//COUNTER_QA				QACounters;				// Ralink propriety counters

	// flags, see fRTMP_ADAPTER_xxx flags
	ULONG					Flags;						// Represent current device status

	// current TX sequence #
	USHORT					Sequence;

	// Control disconnect / connect event generation
	ULONG					LinkDownTime;
	ULONG					LastRxRate;
	BOOLEAN 				bConfigChanged; 		// Config Change flag for the same SSID setting

	ULONG					ExtraInfo;				// Extra information for displaying status
	ULONG					SystemErrorBitmap;		// b0: E2PROM version error
	
	// ---------------------------
	// E2PROM
	// ---------------------------
	ULONG					EepromVersion;			// byte 0: version, byte 1: revision, byte 2~3: unused
	UCHAR					EEPROMAddressNum;		// 93c46=6	93c66=8
	USHORT					EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];

	ULONG					FirmwareVersion;		// byte 0: Minor version, byte 1: Major version, otherwise unused.
	
	// ---------------------------
	// BBP Control
	// ---------------------------
	UCHAR					BbpWriteLatch[110]; 	// record last BBP register value written via BBP_IO_WRITE/BBP_IO_WRITE_VY_REG_ID
	UCHAR					BbpRssiToDbmDelta;
	BBP_R17_TUNING			BbpTuning;

	// ----------------------------
	// RFIC control
	// ----------------------------
	UCHAR					RfIcType;		// RFIC_xxx
	ULONG					RfFreqOffset;	// Frequency offset for channel switching
	BOOLEAN 				bAutoTxAgc;		// Enable driver auto Tx Agc control
	RTMP_RF_REGS			LatchRfRegs;	// latch th latest RF programming value since RF IC doesn't support READ
//	  CCK_TX_POWER_CALIBRATE  CckTxPowerCalibrate;	  // 2004-05-25 add CCK TX power caliberation based on E2PROM settings

	
	UCHAR					RFProgSeq;
	EEPROM_ANTENNA_STRUC	Antenna;							// Since ANtenna definition is different for a & g. We need to save it for future reference.
	EEPROM_NIC_CONFIG2_STRUC	NicConfig2;
	CHANNEL_TX_POWER		TxPower[MAX_NUM_OF_CHANNELS];		// Store Tx power value for all channels.
	CHANNEL_TX_POWER		ChannelList[MAX_NUM_OF_CHANNELS];	// list all supported channels for site survey
	UCHAR		            ChannelListNum; 		            // number of channel in ChannelList[]
    EEPROM_TXPOWER_DELTA_STRUC  TxPowerDeltaConfig;				// Compensate the Tx power BBP94 with this configurate value
	UCHAR					    Bbp94;
	BOOLEAN					    BbpForCCK;

//	UCHAR		ChannelTssiRef[MAX_NUM_OF_CHANNELS];		// Store Tssi Reference value for all channels.
//	UCHAR		ChannelTssiDelta;							// Store Tx TSSI delta increment / decrement value

	// This soft Rx Antenna Diversity mechanism is used only when user set 
	// RX Antenna = DIVERSITY ON
	SOFT_RX_ANT_DIVERSITY	RxAnt;

	
	BOOLEAN 	bAutoTxAgcA;				// Enable driver auto Tx Agc control
	UCHAR		TssiRefA;					// Store Tssi reference value as 25 tempature.	
	UCHAR		TssiPlusBoundaryA[5];		// Tssi boundary for increase Tx power to compensate.
	UCHAR		TssiMinusBoundaryA[5];		// Tssi boundary for decrease Tx power to compensate.
	UCHAR		TxAgcStepA;					// Store Tx TSSI delta increment / decrement value
	CHAR		TxAgcCompensateA;			// Store the compensation (TxAgcStep * (idx-1))

	BOOLEAN 	bAutoTxAgcG;				// Enable driver auto Tx Agc control
	UCHAR		TssiRefG;					// Store Tssi reference value as 25 tempature.	
	UCHAR		TssiPlusBoundaryG[5];		// Tssi boundary for increase Tx power to compensate.
	UCHAR		TssiMinusBoundaryG[5];		// Tssi boundary for decrease Tx power to compensate.
	UCHAR		TxAgcStepG;					// Store Tx TSSI delta increment / decrement value
	CHAR		TxAgcCompensateG;			// Store the compensation (TxAgcStep * (idx-1))
	
	CHAR		BGRssiOffset1;				// Store B/G RSSI#1 Offset value on EEPROM 0x9Ah
	CHAR		BGRssiOffset2;				// Store B/G RSSI#2 Offset value 
	CHAR		ARssiOffset1;				// Store A RSSI#1 Offset value on EEPROM 0x9Ch
	CHAR		ARssiOffset2;


	// ----------------------------
	// LED control
	// ----------------------------
	MCU_LEDCS_STRUC		LedCntl;
	USHORT				LedIndicatorStrength;


	// ----------------------------
	// DEBUG paramerts
	// ----------------------------
//	ULONG					DebugSetting[4];

#ifdef RALINK_ATE
	ATE_INFO				ate;
	BOOLEAN					ContinBulkOut;		//ATE bulk out control
	atomic_t				BulkOutRemained;
	BOOLEAN					ContinBulkIn;		//ATE bulk in control
	atomic_t				BulkInRemained;
#endif	// RALINK_ATE



	//////////////////////////////////////////////////////////////////////
	//	USB
	//////////////////////////////////////////////////////////////////////
#ifdef WMM_SUPPORT
	USHORT            NumberOfPipes;
	UINT                BulkInPipe;
	UINT                BulkOutPipe[4];	// There are 4 bulk for each AC categories
#endif
	USHORT				BulkOutMaxPacketSize;	   // 64 in XP
	//unsigned short				BulkInMaxPacketSize;

	CmdQ				CmdQ;
	CmdQElmt			CmdQElements[COMMAND_QUEUE_SIZE];
	//unsigned char				CmdHandlerIsRunning;

	BOOLEAN				DeQueueRunning[4];		// for ensuring RTUSBDeQueuePacket get call once
	BOOLEAN				DeMGMTQueueRunning;		// for ensuring RTUSBDeQueuePacket get call once
	
	// SpinLocks
	spinlock_t			SendTxWaitQueueLock[4]; // SendTxWaitQueue spinlock

	//spinlock_t			DataQLock[4];
	spinlock_t			DeQueueLock[4];
	//spinlock_t			DeMGMTQueueLock;	// for ensuring RTUSBDeQueuePacket get call once
	//spinlock_t			MLMEWaitQueueLock;	// SendTxWaitQueue spinlock
	spinlock_t			CmdQLock;			// SendTxWaitQueue spinlock
	spinlock_t			BulkOutLock[4];		// SendTxWaitQueue spinlock for 4 ACs
	
//	spinlock_t			ControlLock;		// SendTxWaitQueue spinlock
	spinlock_t			MLMEQLock;			// SendTxWaitQueue spinlock
	//spinlock_t			GenericLock;		// SendTxWaitQueue spinlock
    //spinlock_t                      TxRingLock;            // SendTxWaitQueue spinlock // BensonLiu modify

	/////////////////////
	// Transmit Path
	/////////////////////
	TX_CONTEXT				MLMEContext[PRIO_RING_SIZE];
	//TX_CONTEXT				BeaconContext[BEACON_RING_SIZE];
	TX_CONTEXT				NullContext;
	TX_CONTEXT				PsPollContext;
	TX_CONTEXT				RTSContext;
//	PUCHAR					TxBuffer;
//	TX_BUFFER				TxMgmtBuf;
//	PURB					pTxMgmtUrb;
//	PIRP					pTxMgmtIrp;
	QUEUE_HEADER			SendTxWaitQueue[4];
	
	UINT32					TxRingTotalNumber[4];
	UCHAR					NextTxIndex[4];				// Next TxD write pointer

	UCHAR					NextMLMEIndex;				// Next PrioD write pointer
	UCHAR					PushMgmtIndex;				// Next SW management ring index
	UCHAR					PopMgmtIndex;				// Next SW management ring index
	atomic_t				MgmtQueueSize;				// Number of Mgmt request stored in MgmtRing
	UCHAR					NextRxBulkInIndex;

	// 4 sets of Bulk Out index and pending flag
	UCHAR					NextBulkOutIndex[4];
	BOOLEAN					BulkOutPending[4];

//	BOOLEAN					ControlPending;
	ULONG					PrioRingTxCnt;
	UCHAR					PrioRingFirstIndex;

	atomic_t				TxCount;		// Number of Bulkout waiting to be send.
	LONG					PendingTx;

	// Data related context and AC specified, 4 AC supported
	//TX_CONTEXT				TxContext[4][TX_RING_SIZE];
	TX_CONTEXT				TxContext[1][TX_RING_SIZE];
	//LONG					NumPacketsQueued[4];
//	PURB					pTxUrb[4];
//	PIRP					pTxIrp[4];

	/////////////////////
	// Receive Path
	/////////////////////
	RX_CONTEXT				RxContext[RX_RING_SIZE];
//	PURB					pRxUrb;
//	PIRP					pRxIrp;
//	PUCHAR					RxBuffer;
	atomic_t				PendingRx;

	/////////////////////
	//	Control Flags
	/////////////////////
//	atomic_t				PendingIoCount;	
	// Flags for bulk out data priority
	ULONG					BulkFlags;

//	spinlock_t				MemLock;	// need to check
	ULONG					BulkOutDataOneSecCount;
	ULONG					BulkInDataOneSecCount;
	ULONG					BulkLastOneSecCount; // BulkOutDataOneSecCount + BulkInDataOneSecCount


#ifdef BLOCK_NET_IF
	BLOCK_QUEUE_ENTRY		blockQueueTab[NUM_OF_TX_RING];
#endif // BLOCK_NET_IF //

	// used to record how many null frame send success
	int null_frame_counter;

	// used to record how many virtual interfaces been upped already.
	ULONG					VirtualIfCnt;
	USHORT					MacRegWrite_Processing;
	BOOLEAN		ProbeFinish;
	
	//LM_LIST_CONTAINER pUrbOutList;
	//LM_LIST_CONTAINER pUrbInList;
	
	//LM_LIST_CONTAINER pUrbInCompleteList;
	
}	RTMP_ADAPTER, *PRTMP_ADAPTER;

/* Define in md5.h */
//
// SHA context
//
//typedef struct
//{
//	ULONG		H[5];
//	ULONG		W[80];
//	INT 		lenW;
//	ULONG		sizeHi, sizeLo;
//}	SHA_CTX;

#ifdef RALINK_ATE
extern RTMP_RF_REGS RF2528RegTable[];
extern RTMP_RF_REGS RF5226RegTable[];
extern RTMP_RF_REGS RF5225RegTable[];
extern UCHAR	NUM_OF_2528_CHNL;
extern UCHAR	NUM_OF_5226_CHNL;
extern UCHAR	NUM_OF_5225_CHNL;
#endif

//
// Prototypes of function definition
//

//
// Miniport routines in rtmp_main.c
//
VOID RTUSBHalt(
		PRTMP_ADAPTER	pAd, 
		unsigned char 		IsFree);
	
VOID CMDHandler(
	 PRTMP_ADAPTER pAd);

#ifdef LINUX
INT MlmeThread(
    IN void * Context);
#else
void MlmeThread();
#endif

INT RTUSBCmdThread();    
	
#if WIRELESS_EXT >= 12
struct iw_statistics *rt73_get_wireless_stats(
		struct net_device *net_dev);
#endif

struct net_device_stats *rt73_get_ether_stats(
		struct net_device *net_dev);

long rt_abs(long arg);


//
// Routines in rtmp_init.c
//
VOID CreateThreads( struct net_device *net_dev );

NDIS_STATUS NICInitTransmit(
		PRTMP_ADAPTER	 pAd );

NDIS_STATUS NICInitRecv(
		PRTMP_ADAPTER	pAd);

VOID ReleaseAdapter(
		PRTMP_ADAPTER pAd, 
      unsigned char         IsFree,
      unsigned char         IsOnlyTx);

NDIS_STATUS	RTMPInitAdapterBlock(
		PRTMP_ADAPTER	pAd);

NDIS_STATUS	RTUSBWriteHWMACAddress(
		PRTMP_ADAPTER		pAd);

VOID NICReadEEPROMParameters(
		PRTMP_ADAPTER	pAd);

VOID NICInitAsicFromEEPROM(
		PRTMP_ADAPTER	pAd);

NDIS_STATUS	NICInitializeAsic(
		PRTMP_ADAPTER	pAd);

VOID NICIssueReset(
		PRTMP_ADAPTER	pAd);

unsigned char	NICCheckForHang(
		PRTMP_ADAPTER	pAd);

VOID NICUpdateRawCounters(
	 PRTMP_ADAPTER pAd);

VOID NICResetFromError(
		PRTMP_ADAPTER	pAd);

NDIS_STATUS NICLoadFirmware(
	 PRTMP_ADAPTER pAd);

unsigned char* RTMPFindSection(
	IN	PCHAR	buffer);
	
INT RTMPGetKeyParameter(
	IN	PCHAR	key,
	OUT PCHAR	dest,	
	IN	INT 	destsize,
	IN	PCHAR	buffer);

VOID RTMPReadParametersFromFile(
		PRTMP_ADAPTER pAd);

#ifndef BIG_ENDIAN
unsigned int	RTMPEqualMemory(
		VOID *	pSrc1,
		VOID *	pSrc2,
		unsigned int	Length);
#endif

ULONG	RTMPCompareMemory(
	IN	PVOID	pSrc1,
	IN	PVOID	pSrc2,
	IN	ULONG	Length);

VOID	RTMPZeroMemory(
	IN	PVOID	pSrc,
	IN	ULONG	Length);

VOID	RTMPFillMemory(
	IN	PVOID	pSrc,
	IN	ULONG	Length,
	IN	UCHAR	Fill);

VOID	RTMPMoveMemory(
	OUT PVOID	pDest,
	IN	PVOID	pSrc,
	IN	ULONG	Length);

VOID	PortCfgInit(
		PRTMP_ADAPTER pAd);

unsigned char BtoH(
	 CHAR		ch);

VOID AtoH(
	 CHAR		*src,
	 unsigned char	*dest,
	 INT		destlen);

VOID	RTMPPatchMacBbpBug(
		PRTMP_ADAPTER	pAd);

VOID	RTMPusecDelay(
	IN	ULONG	usec);

VOID	RTMPSetLED(
	 PRTMP_ADAPTER	pAd, 
	 unsigned char			Status);

VOID RTMPSetSignalLED(
	 PRTMP_ADAPTER	pAd, 
	 NDIS_802_11_RSSI Dbm);

VOID RTMPCckBbpTuning(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UINT			TxRate);

VOID	RTMPInitTimer(
		PRTMP_ADAPTER			pAd,
		PRALINK_TIMER_STRUCT	pTimer,
		VOID *					pTimerFunc,
		VOID *					pData,
	  unsigned char					Repeat);

VOID	RTMPSetTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	IN	ULONG					Value);

VOID	RTMPModTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	IN	ULONG					Value);

VOID RTMPCancelTimer(
	  PRALINK_TIMER_STRUCT    pTimer,
	 unsigned char                 *pCancelled);

char * rtstrstr(const char * s1,const char * s2);


//
// MLME routines
//

// Asic/RF/BBP related functions
VOID AsicSwitchChannel(
	 PRTMP_ADAPTER pAd, 
	 unsigned char		 Channel); 

VOID AsicLockChannel(
	 PRTMP_ADAPTER pAd, 
	 unsigned char		 Channel); 

VOID AsicAntennaSelect(
		PRTMP_ADAPTER	pAd,
		unsigned char			Channel); 

VOID AsicAntennaSetting(
		PRTMP_ADAPTER	pAd,
		ABGBAND_STATE	BandState);

VOID AsicAdjustTxPower(
	 PRTMP_ADAPTER pAd); 

VOID AsicSleepThenAutoWakeup(
	 PRTMP_ADAPTER pAd, 
	 unsigned short		 TbttNumToNextWakeUp); 

VOID AsicForceSleep(
	 PRTMP_ADAPTER pAd);

VOID AsicForceWakeup(
	 PRTMP_ADAPTER pAd);

VOID AsicSetBssid(
	 PRTMP_ADAPTER pAd, 
	 unsigned char*		 pBssid); 

VOID AsicDisableSync(
	 PRTMP_ADAPTER pAd); 

VOID AsicEnableBssSync(
	 PRTMP_ADAPTER pAd); 

VOID AsicEnableIbssSync(
	 PRTMP_ADAPTER pAd);

VOID AsicSetEdcaParm(
	 PRTMP_ADAPTER pAd,
	 PEDCA_PARM	 pEdcaParm);
	
VOID AsicSetSlotTime(
	 PRTMP_ADAPTER pAd,
	 unsigned char		 bUseShortSlotTime); 

VOID AsicBbpTuning(
	 PRTMP_ADAPTER pAd);

VOID AsicAddSharedKeyEntry(
	 PRTMP_ADAPTER pAd,
	 unsigned char		 BssIndex,
	 unsigned char		 KeyIdx,
	 unsigned char		 CipherAlg,
	 unsigned char*		 pKey,
	 unsigned char*		 pTxMic,
	 unsigned char*		 pRxMic);

VOID AsicRemoveSharedKeyEntry(
	 PRTMP_ADAPTER pAd,
	 unsigned char		 BssIndex,
	 unsigned char		 KeyIdx);

VOID AsicAddPairwiseKeyEntry(
	 PRTMP_ADAPTER pAd,
	 unsigned char*		 pAddr,
	 unsigned char		 KeyIdx,
	 unsigned char		 CipherAlg,
	 unsigned char*		 pKey,
	 unsigned char*		 pTxMic,
	 unsigned char*		 pRxMic);

VOID AsicRemovePairwiseKeyEntry(
	 PRTMP_ADAPTER pAd,
	 unsigned char		 KeyIdx);

VOID	RTMPCheckRates(
			PRTMP_ADAPTER	pAd,
	 	unsigned char			SupRate[],
	 	unsigned char			*SupRateLen);

VOID AsicSetRxAnt(
	 PRTMP_ADAPTER	pAd,
	 unsigned char			Pair1,
	 unsigned char			Pair2);

VOID AsicEvaluateSecondaryRxAnt(
	 PRTMP_ADAPTER pAd); 

VOID AsicRxAntEvalTimeout(
	 VOID * SystemSpecific1,
	 VOID * FunctionContext,
	 VOID * SystemSpecific2,
	 VOID * SystemSpecific3);

VOID AsicRxAntEvalAction(
	 PRTMP_ADAPTER pAd);

unsigned char RandomByte(
	 PRTMP_ADAPTER pAd); 

VOID StaQuickResponeForRateUpExec(
	 VOID * SystemSpecific1,
	 VOID * FunctionContext,
	 VOID * SystemSpecific2,
	 VOID * SystemSpecific3);
	
VOID BssTableInit(
	 BSS_TABLE *Tab); 

ULONG BssTableSearch(
	IN BSS_TABLE *Tab, 
	IN PUCHAR	 pBssid,
	IN UCHAR	 Channel);

ULONG BssSsidTableSearch(
	 BSS_TABLE *Tab, 
	 unsigned char*	 pBssid,
	 unsigned char*	 pSsid,
	 unsigned char	 SsidLen,
	 unsigned char	 Channel);

ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab, 
	IN PUCHAR	 Bssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel);

VOID BssTableDeleteEntry(
	 	BSS_TABLE *Tab, 
			unsigned char*	  pBssid,
			unsigned char	  Channel); 

VOID BssEntrySet(
		PRTMP_ADAPTER	pAd, 
	 BSS_ENTRY *pBss, 
	 unsigned char* pBssid, 
	 CHAR Ssid[], 
	 unsigned char SsidLen, 
	 unsigned char BssType, 
	 unsigned short BeaconPeriod, 
	 PCF_PARM pCfParm, 
	 unsigned short AtimWin, 
	 unsigned short CapabilityInfo, 
	 unsigned char SupRate[], 
	 unsigned char SupRateLen,
	 unsigned char ExtRate[], 
	 unsigned char ExtRateLen,
	 unsigned char Channel,
	 unsigned char Rssi,
	 LARGE_INTEGER TimeStamp,
	 unsigned char CkipFlag,
	 PEDCA_PARM pEdcaParm,
	 PQOS_CAPABILITY_PARM pQosCapability,
	 PQBSS_LOAD_PARM pQbssLoad,
//	 unsigned char LengthVIE,
	 unsigned short LengthVIE,  // edit by johnli, variable ie length could be > 256
	 PNDIS_802_11_VARIABLE_IEs pVIE); 

ULONG BssTableSetEntry(
	IN	PRTMP_ADAPTER	pAd, 
	OUT BSS_TABLE *Tab, 
	IN PUCHAR pBssid, 
	IN CHAR Ssid[], 
	IN UCHAR SsidLen, 
	IN UCHAR BssType, 
	IN USHORT BeaconPeriod, 
	IN CF_PARM *CfParm, 
	IN USHORT AtimWin, 
	IN USHORT CapabilityInfo, 
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN UCHAR ChannelNo,
	IN UCHAR Rssi,
	IN LARGE_INTEGER TimeStamp,
	IN UCHAR CkipFlag,
	IN PEDCA_PARM pEdcaParm,
	IN PQOS_CAPABILITY_PARM pQosCapability,
	IN PQBSS_LOAD_PARM pQbssLoad,
//	IN UCHAR LengthVIE,
	IN USHORT LengthVIE,  // edit by johnli, variable ie length could be > 256
	IN PNDIS_802_11_VARIABLE_IEs pVIE);

VOID BssTableSsidSort(
		PRTMP_ADAPTER	pAd, 
	 BSS_TABLE *OutTab, 
		CHAR Ssid[], 
		unsigned char SsidLen);

VOID BssTableSortByRssi(
	  BSS_TABLE *OutTab); 

VOID BssCipherParse(
	 	PBSS_ENTRY	pBss);

VOID MacAddrRandomBssid(
	 PRTMP_ADAPTER pAd, 
	 unsigned char* pAddr); 

VOID MgtMacHeaderInit(
		PRTMP_ADAPTER	pAd, 
	  PHEADER_802_11 pHdr80211, 
	 unsigned char SubType, 
	 unsigned char ToDs, 
	 unsigned char* pDA, 
	 unsigned char* pBssid);

ULONG MakeOutgoingFrame(
	OUT CHAR *Buffer, 
	OUT ULONG *FrameLen, ...);

NDIS_STATUS MlmeInit(
	 PRTMP_ADAPTER pAd); 

VOID MlmeHandler(
	 PRTMP_ADAPTER pAd);

VOID MlmeHalt(
	 PRTMP_ADAPTER pAd) ;

VOID MlmeSuspend(
	 PRTMP_ADAPTER pAd,
	 unsigned char linkdown);

VOID MlmeResume(
		PRTMP_ADAPTER	pAd);

VOID MlmePeriodicExec(
	 VOID * SystemSpecific1,
	 VOID * FunctionContext,
	 VOID * SystemSpecific2,
	 VOID * SystemSpecific3);

VOID STAMlmePeriodicExec(
		PRTMP_ADAPTER pAd);

VOID LinkDownExec(
	 VOID * SystemSpecific1,
	 VOID * FunctionContext,
	 VOID * SystemSpecific2,
	 VOID * SystemSpecific3);

VOID MlmeAutoScan(
	 PRTMP_ADAPTER pAd);

VOID MlmeAutoRecoverNetwork(
	 PRTMP_ADAPTER pAd);

VOID MlmeAutoReconnectLastSSID(
	 PRTMP_ADAPTER pAd);
	
unsigned char	MlmeValidateSSID(
	 unsigned char*	pSsid,
	 unsigned char	SsidLen);

VOID MlmeCheckForRoaming(
	IN PRTMP_ADAPTER pAd,
	IN ULONG	Now32);

VOID MlmeCheckForFastRoaming(
	IN	PRTMP_ADAPTER	pAd,
	IN	ULONG			Now);

VOID MlmeCalculateChannelQuality(
	IN PRTMP_ADAPTER pAd,
	IN ULONG Now32);

VOID MlmeDynamicTxRateSwitching(
	 PRTMP_ADAPTER pAd);

VOID MlmeCheckPsmChange(
	IN PRTMP_ADAPTER pAd,
	IN ULONG	Now32);

VOID MlmeSetPsmBit(
	 PRTMP_ADAPTER pAd, 
	 unsigned short psm);

VOID MlmeSetTxPreamble(
	 PRTMP_ADAPTER pAd, 
	 unsigned short TxPreamble);

VOID MlmeUpdateTxRates(
	 PRTMP_ADAPTER pAd,
	 unsigned char		 bLinkUp);

VOID MlmeRadioOff(
	 PRTMP_ADAPTER pAd);

VOID MlmeRadioOn(
	 PRTMP_ADAPTER pAd);	  

NDIS_STATUS MlmeQueueInit(
	 MLME_QUEUE *Queue); 

unsigned char MlmeEnqueue(
	IN	PRTMP_ADAPTER	pAd,
	IN ULONG Machine, 
	IN ULONG MsgType, 
	IN ULONG MsgLen, 
	IN VOID *Msg);

unsigned char MlmeEnqueueForRecv(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PFRAME_802_11	p80211hdr,  // add by johnli, fix WPS & WPAPSK/WPA2PSK bugs for receiving EAPoL fragmentation packets
	IN UCHAR Rssi, 
	IN ULONG MsgLen, 
	IN VOID *Msg,
	IN UCHAR Signal);

unsigned char MlmeDequeue(
	 MLME_QUEUE *Queue, 
	 MLME_QUEUE_ELEM **Elem); 

VOID MlmeRestartStateMachine(
		PRTMP_ADAPTER	pAd);

VOID MlmePostRestartStateMachine(
		PRTMP_ADAPTER	pAd);

unsigned char MlmeQueueEmpty(
	 MLME_QUEUE *Queue);

unsigned char MlmeQueueFull(
	 MLME_QUEUE *Queue); 

VOID MlmeQueueDestroy(
	 MLME_QUEUE *pQueue); 

unsigned char MsgTypeSubst(
	 PRTMP_ADAPTER  pAd,
	// edit by johnli, fix WPAPSK/WPA2PSK bugs for receiving EAPoL fragmentation packets
//	 PFRAME_802_11 pFrame, 
	 PFRAME_802_11 p80211hdr, 
	 unsigned char* pFrame, 
	// end johnli
	 INT *Machine, 
	 INT *MsgType); 

VOID StateMachineInit(
	IN STATE_MACHINE *S, 
	IN STATE_MACHINE_FUNC Trans[], 
	IN ULONG StNr, 
	IN ULONG MsgNr, 
	IN STATE_MACHINE_FUNC DefFunc, 
	IN ULONG InitState, 
	IN ULONG Base);

VOID StateMachineSetAction(
	IN STATE_MACHINE *S, 
	IN ULONG St, 
	IN ULONG Msg, 
	IN STATE_MACHINE_FUNC Func);

VOID StateMachinePerformAction(
		PRTMP_ADAPTER	pAd, 
	 STATE_MACHINE *S, 
	 MLME_QUEUE_ELEM *Elem); 

VOID Drop(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID LfsrInit(
	IN PRTMP_ADAPTER pAd, 
	IN ULONG Seed);

NDIS_STATUS MlmeAllocateMemory(
	 PRTMP_ADAPTER pAd,
	 VOID *		 *AllocVa);
	
VOID	MlmeFreeMemory(
	 PRTMP_ADAPTER pAd,
	 VOID *		 AllocVa);

NDIS_STATUS MlmeInitMemoryHandler(
	IN PRTMP_ADAPTER pAd,
	IN UINT  Number,
	IN UINT  Size);

VOID MlmeFreeMemoryHandler(
	 PRTMP_ADAPTER pAd);

VOID RadarDetectionStart(
	 PRTMP_ADAPTER	pAd);

unsigned char RadarDetectionStop(
	 PRTMP_ADAPTER	pAd);

unsigned char RadarChannelCheck(
	 PRTMP_ADAPTER	pAd,
	 unsigned char			Ch);


// Assoc/Auth/Auth_rsp related functions
VOID AssocStateMachineInit(
		PRTMP_ADAPTER	pAd, 
		STATE_MACHINE *S, 
	 STATE_MACHINE_FUNC Trans[]);
	
VOID AssocTimeout(
	 VOID * SystemSpecific1,
	 VOID * FunctionContext,
	 VOID * SystemSpecific2,
	 VOID * SystemSpecific3);

VOID ReassocTimeout(
	 VOID * SystemSpecific1,
	 VOID * FunctionContext,
	 VOID * SystemSpecific2,
	 VOID * SystemSpecific3);

VOID DisassocTimeout(
	 VOID * SystemSpecific1,
	 VOID * FunctionContext,
	 VOID * SystemSpecific2,
	 VOID * SystemSpecific3);

VOID MlmeAssocReqAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem);

VOID MlmeReassocReqAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem);

VOID MlmeDisassocReqAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem);

VOID PeerAssocRspAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem);

VOID PeerReassocRspAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem);

VOID AssocPostProc(
	 PRTMP_ADAPTER pAd, 
	 unsigned char* pAddr2, 
	 unsigned short CapabilityInfo, 
	 unsigned short Aid, 
	 unsigned char SupRate[], 
	 unsigned char SupRateLen,
	 unsigned char ExtRate[],
	 unsigned char ExtRateLen,
	 PEDCA_PARM pEdcaParm);

VOID PeerDisassocAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem);

VOID AssocTimeoutAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem);

VOID ReassocTimeoutAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem);

VOID DisassocTimeoutAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenAssoc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID InvalidStateWhenReassoc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 
	
VOID InvalidStateWhenDisassociate(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID Cls3errAction(
	 PRTMP_ADAPTER pAd, 
	 unsigned char*	   pAddr); 

VOID AuthStateMachineInit(
	 PRTMP_ADAPTER pAd, 
	 PSTATE_MACHINE Sm, 
	 STATE_MACHINE_FUNC Trans[]);

VOID AuthTimeout(
	 VOID * SystemSpecific1,
	 VOID * FunctionContext,
	 VOID * SystemSpecific2,
	 VOID * SystemSpecific3);

VOID MlmeAuthReqAction(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq2Action(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq4Action(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID AuthTimeoutAction(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID Cls2errAction(
		PRTMP_ADAPTER	pAd, 
		unsigned char* pAddr);

VOID MlmeDeauthReqAction(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenAuth(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID AuthRspStateMachineInit(
	 PRTMP_ADAPTER pAd, 
	 PSTATE_MACHINE Sm, 
	 STATE_MACHINE_FUNC Trans[]);
	
VOID PeerAuthSimpleRspGenAndSend(
	 PRTMP_ADAPTER pAd, 
	 PHEADER_802_11 pHdr80211,
	 unsigned short Alg, 
	 unsigned short Seq, 
	 unsigned short Reason, 
	 unsigned short Status);

VOID PeerDeauthAction(
	 PRTMP_ADAPTER pAd, 
	 PMLME_QUEUE_ELEM Elem);
	
VOID MlmeCntlInit(
	 PRTMP_ADAPTER pAd, 
	 STATE_MACHINE *S, 
	 STATE_MACHINE_FUNC Trans[]); 

VOID MlmeCntlMachinePerformAction(
	 PRTMP_ADAPTER pAd, 
	 STATE_MACHINE *S, 
	 MLME_QUEUE_ELEM *Elem); 

VOID CntlIdleProc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID CntlOidScanProc(
	 PRTMP_ADAPTER pAd,
	 MLME_QUEUE_ELEM *Elem);
	
VOID CntlOidSsidProc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM * Elem); 

VOID CntlOidRTBssidProc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM * Elem); 

VOID CntlMlmeRoamingProc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID CntlWaitDisassocProc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID CntlWaitJoinProc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID CntlWaitStartProc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID CntlWaitAuthProc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID CntlWaitAuthProc2(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID CntlWaitAssocProc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID CntlWaitReassocProc(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID LinkUp(
	 PRTMP_ADAPTER pAd,
	 unsigned char BssType); 

VOID LinkDown(
	 PRTMP_ADAPTER pAd,
		unsigned char 	 IsReqFromAP);

VOID IterateOnBssTab(
	 PRTMP_ADAPTER pAd); 

VOID IterateOnBssTab2(
	 PRTMP_ADAPTER pAd); 

VOID JoinParmFill(
	 PRTMP_ADAPTER pAd, 
	  MLME_JOIN_REQ_STRUCT *JoinReq, 
	 unsigned int BssIdx); 

VOID AssocParmFill(
	 PRTMP_ADAPTER pAd, 
	  MLME_ASSOC_REQ_STRUCT *AssocReq, 
	 unsigned char*					  pAddr, 
	 unsigned short					  CapabilityInfo, 
	 unsigned int					  Timeout, 
	 unsigned short					  ListenIntv); 

VOID ScanParmFill(
	 PRTMP_ADAPTER pAd, 
	  MLME_SCAN_REQ_STRUCT *ScanReq, 
	 CHAR Ssid[], 
	 unsigned char SsidLen, 
	 unsigned char BssType, 
	 unsigned char ScanType); 

VOID DisassocParmFill(
	 PRTMP_ADAPTER pAd, 
	  MLME_DISASSOC_REQ_STRUCT *DisassocReq, 
	 unsigned char* pAddr, 
	 unsigned short Reason); 

VOID StartParmFill(
	 PRTMP_ADAPTER pAd, 
	  MLME_START_REQ_STRUCT *StartReq, 
	 CHAR Ssid[], 
	 unsigned char SsidLen); 

VOID AuthParmFill(
	 PRTMP_ADAPTER pAd, 
	  MLME_AUTH_REQ_STRUCT *AuthReq, 
	 unsigned char* pAddr, 
	 unsigned short Alg); 

VOID ComposePsPoll(
	 PRTMP_ADAPTER pAd);

VOID ComposeNullFrame(
	 PRTMP_ADAPTER pAd);

ULONG MakeIbssBeacon(
	IN PRTMP_ADAPTER pAd);

//
// Private routines  Sync.c
//
VOID SyncStateMachineInit(
	 PRTMP_ADAPTER pAd, 
	 STATE_MACHINE *Sm, 
	 STATE_MACHINE_FUNC Trans[]); 

VOID BeaconTimeout(
	 VOID * SystemSpecific1,
	 VOID * FunctionContext,
	 VOID * SystemSpecific2,
	 VOID * SystemSpecific3);

VOID ScanTimeout(
	 VOID * SystemSpecific1,
	 VOID * FunctionContext,
	 VOID * SystemSpecific2,
	 VOID * SystemSpecific3);

VOID MlmeScanReqAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem);

VOID MlmeJoinReqAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID MlmeStartReqAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID PeerBeaconAtScanAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID PeerBeaconAtJoinAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID PeerBeacon(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 
 
VOID PeerProbeReqAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID BeaconTimeoutAtJoinAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID ScanTimeoutAction(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID ScanNextChannel(
	 PRTMP_ADAPTER pAd);

VOID InvalidStateWhenScan(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID InvalidStateWhenJoin(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID InvalidStateWhenStart(
	 PRTMP_ADAPTER pAd, 
	 MLME_QUEUE_ELEM *Elem); 

VOID EnqueuePsPoll(
	 PRTMP_ADAPTER pAd); 

VOID EnqueueBeaconFrame(
	 PRTMP_ADAPTER pAd); 

VOID EnqueueProbeRequest(
	 PRTMP_ADAPTER pAd); 

VOID BuildChannelList(
	 PRTMP_ADAPTER pAd);

unsigned char NextChannel(
	 PRTMP_ADAPTER pAd, 
	 unsigned char channel);

unsigned char FirstChannel(
	 PRTMP_ADAPTER pAd);

CHAR	ConvertToRssi(
	 PRTMP_ADAPTER pAd,
		unsigned char	Rssi,
		unsigned char	RssiNumber);

//
// prototypes  sanity.c
//
unsigned char MlmeScanReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT UCHAR *pBssType, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
    OUT UCHAR *pScanType);

unsigned char MlmeStartReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen);

unsigned char MlmeAssocReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pApAddr, 
    OUT USHORT *pCapabilityInfo, 
    OUT ULONG *pTimeout, 
    OUT USHORT *pListenIntv);

unsigned char MlmeAuthReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr, 
    OUT ULONG *pTimeout, 
    OUT USHORT *pAlg);

BOOLEAN PeerAssocRspSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *pMsg, 
	IN ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT USHORT *pCapabilityInfo, 
	OUT USHORT *pStatus, 
	OUT USHORT *pAid, 
	OUT UCHAR SupRate[], 
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[], 
	OUT UCHAR *pExtRateLen,
	OUT PEDCA_PARM pEdcaParm);

unsigned char PeerDisassocSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pReason);

unsigned char PeerDeauthSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pReason);
//iverson 2007 1109
NTSTATUS RTUSBSingleWrite(
    IN     PRTMP_ADAPTER   pAd,
    IN  USHORT                   Offset,
    IN  USHORT                   Value); 

unsigned char PeerAuthSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr, 
    OUT USHORT *pAlg, 
    OUT USHORT *pSeq, 
    OUT USHORT *pStatus, 
		CHAR *pChlgText);

unsigned char PeerProbeReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen);

BOOLEAN PeerBeaconAndProbeRspSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT PUCHAR pBssid, 
	OUT CHAR Ssid[], 
	OUT UCHAR *pSsidLen, 
	OUT UCHAR *pBssType, 
	OUT USHORT *pBeaconPeriod, 
	OUT UCHAR *pChannel,
	OUT UCHAR *pNewChannel,
	OUT LARGE_INTEGER *pTimestamp, 
	OUT CF_PARM *pCfParm, 
	OUT USHORT *pAtimWin, 
	OUT USHORT *pCapabilityInfo, 
	OUT UCHAR *pErp,
	OUT UCHAR *pDtimCount, 
	OUT UCHAR *pDtimPeriod, 
	OUT UCHAR *pBcastFlag, 
	OUT UCHAR *pMessageToMe, 
	OUT UCHAR SupRate[],
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
	OUT	UCHAR *pCkipFlag,
	OUT	UCHAR *pAironetCellPowerLimit,
	OUT PEDCA_PARM		 pEdcaParm,
	OUT PQBSS_LOAD_PARM  pQbssLoad,
	OUT PQOS_CAPABILITY_PARM pQosCapability,
	OUT ULONG *pRalinkIe,
//	OUT UCHAR *LengthVIE,	
	OUT USHORT *LengthVIE,	// edit by johnli, variable ie length could be > 256
	OUT	PNDIS_802_11_VARIABLE_IEs pVIE);

unsigned char GetTimBit(
	 CHAR *Ptr, 
	 unsigned short Aid, 
	 unsigned char *TimLen, 
	 unsigned char *BcastFlag, 
	 unsigned char *DtimCount, 
	 unsigned char *DtimPeriod,
	 unsigned char *MessageToMe); 

unsigned char ChannelSanity(
	 PRTMP_ADAPTER pAd, 
	 unsigned char channel);

NDIS_802_11_NETWORK_TYPE NetworkTypeInUseSanity(
	 unsigned char  Channel,
	 unsigned char  SupRate[],
	 unsigned char  SupRateLen,
	 unsigned char  ExtRate[],
	 unsigned char  ExtRateLen);

 unsigned char PeerTxTypeInUseSanity(
	 unsigned char  Channel,
	 unsigned char  SupRate[],
	 unsigned char  SupRateLen,
	 unsigned char  ExtRate[],
	 unsigned char  ExtRateLen);

NDIS_STATUS	RTMPWPAWepKeySanity(
		PRTMP_ADAPTER	pAd,
		VOID *			pBuf);

NDIS_STATUS	RTMPRemoveKeySanity(
		PRTMP_ADAPTER	pAd,
		VOID *			pBuf);


//
// prototypes  rtusb_bulk.c
//	
VOID	RTUSBBulkOutDataPacket(
		PRTMP_ADAPTER	pAd,
		unsigned char			BulkOutPipeId,
		unsigned char			Index);

VOID	RTUSBBulkOutNullFrame(
		PRTMP_ADAPTER	pAd);

VOID	RTUSBBulkOutRTSFrame(
		PRTMP_ADAPTER	pAd);

VOID	RTUSBBulkOutMLMEPacket(
		PRTMP_ADAPTER	pAd,
		unsigned char			Index);

VOID	RTUSBBulkOutPsPoll(
		PRTMP_ADAPTER	pAd);

VOID	RTUSBBulkReceive(
		PRTMP_ADAPTER	pAd);

VOID RTUSBBulkCompleteTask( void
	 /*unsigned long data*/);

VOID RTUSBBulkRxHandle(
		unsigned long data);

VOID	RTUSBKickBulkOut(
		PRTMP_ADAPTER pAd);

VOID	RTUSBCleanUpDataBulkOutQueue(
		PRTMP_ADAPTER	pAd);

VOID	RTUSBCleanUpMLMEBulkOutQueue(
		PRTMP_ADAPTER	pAd);

VOID	RTUSBCancelPendingIRPs(
		PRTMP_ADAPTER	pAd);

VOID RTUSBCancelPendingBulkOutIRP(
		PRTMP_ADAPTER	pAd);
	
VOID	RTUSBCancelPendingBulkInIRP(
		PRTMP_ADAPTER	pAd);


// macro definitions and prototypes of completion funuc.
#if 0 //LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#define RTUSBBulkOutDataPacketComplete(purb, pt_regs)    RTUSBBulkOutDataPacketComplete(purb)
#define RTUSBBulkOutNullFrameComplete(pUrb, pt_regs)     RTUSBBulkOutNullFrameComplete(pUrb)
#define RTUSBBulkOutRTSFrameComplete(pUrb, pt_regs)      RTUSBBulkOutRTSFrameComplete(pUrb)
#define RTUSBBulkOutMLMEPacketComplete(pUrb, pt_regs)    RTUSBBulkOutMLMEPacketComplete(pUrb)
#define RTUSBBulkOutPsPollComplete(pUrb, pt_regs)        RTUSBBulkOutPsPollComplete(pUrb)
#define RTUSBBulkRxComplete(pUrb, pt_regs)               RTUSBBulkRxComplete(pUrb)
#endif
struct pt_regs;
VOID RTUSBBulkOutDataPacketComplete(purbb_t purb, struct pt_regs *pt_regs);
VOID RTUSBBulkOutNullFrameComplete(purbb_t pUrb, struct pt_regs *pt_regs);
VOID RTUSBBulkOutRTSFrameComplete(purbb_t pUrb, struct pt_regs *pt_regs);
VOID RTUSBBulkOutMLMEPacketComplete(purbb_t pUrb, struct pt_regs *pt_regs);
VOID RTUSBBulkOutPsPollComplete(purbb_t pUrb, struct pt_regs *pt_regs);
VOID RTUSBBulkRxComplete(purbb_t pUrb, struct pt_regs *pt_regs);
void dump_urb(struct urb* purb);

//
// prototypes  rtusb_io.c
//
NTSTATUS	RTUSBFirmwareRun(
		PRTMP_ADAPTER	pAd);

NTSTATUS	RTUSBMultiRead(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUCHAR			pData,
	IN	USHORT			length);

NTSTATUS	RTUSBMultiWrite(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData,
	IN	USHORT			length);

NTSTATUS	RTUSBReadMACRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PULONG			pValue);

NTSTATUS	RTUSBWriteMACRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	ULONG			Value);

NTSTATUS	RTUSBSetLED(
	IN	PRTMP_ADAPTER		pAd,
	IN	MCU_LEDCS_STRUC		LedStatus,
	IN	USHORT				LedIndicatorStrength);

NTSTATUS	RTUSBReadBBPRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Id,
	IN	PUCHAR			pValue);

NTSTATUS	RTUSBWriteBBPRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Id,
	IN	UCHAR			Value);

NTSTATUS	RTUSBWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	ULONG			Value);

NTSTATUS	RTUSBReadEEPROM(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUCHAR			pData,
	IN	USHORT			length);

NTSTATUS	RTUSBWriteEEPROM(
		PRTMP_ADAPTER	pAd,
		unsigned short			Offset,
		unsigned char*			pData,
		unsigned short			length);

NTSTATUS RTUSBPutToSleep(
		PRTMP_ADAPTER	pAd);

NTSTATUS RTUSBWakeUp(
		PRTMP_ADAPTER	pAd);

VOID	RTUSBInitializeCmdQ(
		PCmdQ	cmdq);

NDIS_STATUS		RTUSBEnqueueCmdFromNdis(
	IN	PRTMP_ADAPTER	pAd,
	IN	NDIS_OID		Oid,
	IN	BOOLEAN			SetInformation,
	IN	PVOID			pInformationBuffer,
	IN	ULONG			InformationBufferLength);

VOID	RTUSBEnqueueInternalCmd(
	IN	PRTMP_ADAPTER	pAd,
	IN	NDIS_OID		Oid);

VOID	RTUSBDequeueCmd(
	IN	PCmdQ		cmdq,
	OUT	PCmdQElmt	*pcmdqelmt);

INT		RTUSB_VendorRequest(
	IN	PRTMP_ADAPTER	pAd,
	IN	ULONG			TransferFlags,
	IN	UCHAR			RequestType,
	IN	UCHAR			Request,
	IN	USHORT			Value,
	IN	USHORT			Index,
	IN	PVOID			TransferBuffer,
	IN	ULONG			TransferBufferLength);

NTSTATUS	RTUSB_ResetDevice(
		PRTMP_ADAPTER	pAd);
#ifdef DBG
NDIS_STATUS 	RTUSBQueryHardWareRegister(
		PRTMP_ADAPTER	pAd,
		VOID *			pBuf);

NDIS_STATUS 	RTUSBSetHardWareRegister(
		PRTMP_ADAPTER	pAd,
		VOID *			pBuf);
#endif

//
// prototypes   rtusb_data.c
//
NDIS_STATUS Sniff2BytesFromNdisBuffer(
		struct sk_buff	*pFirstSkb,
		unsigned char			DesiredOffset,
	 unsigned char*			pByte0,
	 unsigned char*			pByte1);

NDIS_STATUS	RTMPSendPacket(
		PRTMP_ADAPTER	pAd,
		struct sk_buff	*pSkb);

INT RTMPSendPackets(
		struct sk_buff		*pSkb,
		struct net_device	*net_dev);

#ifdef BIG_ENDIAN	
static inline
#endif
NDIS_STATUS RTUSBHardTransmit(
		PRTMP_ADAPTER	pAd,
		struct sk_buff	*pSkb,
		unsigned char			NumberRequired,
		unsigned char			QueIdx);

VOID	RTUSBMlmeHardTransmit(
		PRTMP_ADAPTER	pAd,
		PMGMT_STRUC		pMgmt);

NDIS_STATUS	RTUSBFreeDescriptorRequest(
		PRTMP_ADAPTER	pAd,
		unsigned char			RingType,
		unsigned char			BulkOutPipeId,
		unsigned char			NumberRequired);

VOID	RTUSBRejectPendingPackets(
		PRTMP_ADAPTER	pAd);

VOID	RTUSBWriteTxDescriptor(
	IN	PRTMP_ADAPTER pAd,
	IN	PTXD_STRUC	pSourceTxD,
	IN	UCHAR		CipherAlg,
	IN	UCHAR		KeyTable,
	IN	UCHAR		KeyIdx,
	IN	BOOLEAN		Ack,
	IN	BOOLEAN		Fragment,
	IN	BOOLEAN 	InsTimestamp,
	IN	UCHAR		RetryMode,
	IN	UCHAR		Ifs,
	IN	UINT		Rate,
	IN	ULONG		Length,
	IN	UCHAR		QueIdx,
	IN	UCHAR		PID,
    IN  BOOLEAN     bAfterRTSCTS);
	
VOID	RTMPDeQueuePacket(
		PRTMP_ADAPTER	pAd,
		unsigned char			BulkOutPipeId);

VOID	RTUSBRxPacket(
	 IN	 unsigned long data);

VOID	RTUSBDequeueMLMEPacket(
		PRTMP_ADAPTER	pAd);

VOID	RTUSBCleanUpMLMEWaitQueue(
		PRTMP_ADAPTER	pAd);

VOID	RTUSBSuspendMsduTransmission(
		PRTMP_ADAPTER	pAd);

VOID	RTUSBResumeMsduTransmission(
		PRTMP_ADAPTER	pAd);

VOID	MiniportMMRequest(
		PRTMP_ADAPTER	pAd,
		unsigned char			QueIdx,
		VOID *			pBuffer,
		unsigned int			Length);

unsigned char	RTMPSearchTupleCache(
		PRTMP_ADAPTER	pAd,
		PHEADER_802_11	pHeader);

VOID	RTMPUpdateTupleCache(
		PRTMP_ADAPTER	pAd,
		PHEADER_802_11	pHeader);

NDIS_STATUS	RTMPApplyPacketFilter(
		PRTMP_ADAPTER	pAd, 
		PRXD_STRUC		pRxD, 
		PHEADER_802_11	pHeader);

NDIS_STATUS	RTMPCheckRxDescriptor(
		PRTMP_ADAPTER	pAd,
		PHEADER_802_11	pHeader,	
		PRXD_STRUC	pRxD);

VOID	RTMPReportMicError(
		PRTMP_ADAPTER	pAd, 
		PCIPHER_KEY		pWpaKey);

VOID	RTMPSendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate, 
	IN	BOOLEAN			bQosNull); 

VOID	RTMPSendRTSCTSFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pDA,
	IN	ULONG			NextMpduSize,
	IN	UCHAR			TxRate,
	IN	UCHAR			RTSRate,
	IN	USHORT			AckDuration,
	IN	UCHAR			QueIdx,
	IN	UCHAR			FrameGap,
	IN	UCHAR			Type);

USHORT	RTMPCalcDuration(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Rate,
	IN	ULONG			Size);

unsigned char 	RTMPCheckDHCPFrame(
		PRTMP_ADAPTER	pAd, 
		struct sk_buff	*pSkb);

	
//
// prototypes in rtmp_wep.c
//
VOID	RTMPInitWepEngine(
	IN	PRTMP_ADAPTER	pAdapter,	
	IN	PUCHAR			pKey,
	IN	UCHAR			KeyId,
	IN	UCHAR			KeyLen, 
	IN OUT	PUCHAR		pDest);

VOID	RTMPEncryptData(
	IN	PRTMP_ADAPTER	pAdapter,	
	IN	PUCHAR			pSrc,
	IN	PUCHAR			pDest,
	IN	UINT			Len);

BOOLEAN	RTMPDecryptData(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PUCHAR			pSrc,
	IN	UINT			Len);

VOID	ARCFOUR_INIT(
	IN	PARCFOURCONTEXT Ctx,
	IN	PUCHAR			pKey,
	IN	UINT			KeyLen);

UCHAR	ARCFOUR_BYTE(
	IN	PARCFOURCONTEXT 	Ctx);

VOID	ARCFOUR_DECRYPT(
	IN	PARCFOURCONTEXT Ctx,
	IN	PUCHAR			pDest, 
	IN	PUCHAR			pSrc,
	IN	UINT			Len);

VOID	ARCFOUR_ENCRYPT(
	IN	PARCFOURCONTEXT Ctx,
	IN	PUCHAR			pDest,
	IN	PUCHAR			pSrc,
	IN	UINT			Len);

ULONG	RTMP_CALC_FCS32(
	IN	ULONG	Fcs,
	IN	PUCHAR	Cp,
	IN	INT 	Len);

VOID	RTMPSetICV(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PUCHAR	pDest);
	
//
// prototypes  rtmp_tkip.c
//
VOID	RTMPInitTkipEngine(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PUCHAR			pTKey,
	IN	UCHAR			KeyId,
	IN	PUCHAR			pTA,
	IN	PUCHAR			pMICKey,
	IN	PUCHAR			pTSC,
	OUT PULONG			pIV16,
	OUT PULONG			pIV32);

VOID	RTMPInitMICEngine(
		PRTMP_ADAPTER	pAdapter,	
		unsigned char*			pKey,
		unsigned char*			pDA,
		unsigned char*			pSA,
		unsigned char			UserPriority,
		unsigned char*			pMICKey);

BOOLEAN RTMPTkipCompareMICValue(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PUCHAR			pSrc,
	IN	PUCHAR			pDA,
	IN	PUCHAR			pSA,
	IN	PUCHAR			pMICKey,
	IN	UINT			Len);

VOID	RTMPCalculateMICValue(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct sk_buff	*pSkb,
	IN	PUCHAR			pEncap,
	IN	PCIPHER_KEY		pKey);

BOOLEAN RTMPSoftDecryptTKIP(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pData,
	IN ULONG	DataByteCnt, 
	IN UCHAR    UserPriority,
	IN PCIPHER_KEY	pWpaKey);

BOOLEAN RTMPSoftDecryptAES(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pData,
	IN ULONG	DataByteCnt, 
	IN PCIPHER_KEY	pWpaKey);

BOOLEAN	RTMPTkipCompareMICValueWithLLC(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PUCHAR			pLLC,
	IN	PUCHAR			pSrc,
	IN	PUCHAR			pDA,
	IN	PUCHAR			pSA,
	IN	PUCHAR			pMICKey,
	IN	UINT			Len);

VOID	RTMPTkipAppend( 
	IN	PTKIP_KEY_INFO	pTkip,	
	IN	PUCHAR			pSrc,
	IN	UINT			nBytes);

VOID	RTMPTkipGetMIC( 
		PTKIP_KEY_INFO	pTkip);

//
// prototypes  wpa.c
//
unsigned char WpaMsgTypeSubst(
	IN	UCHAR	EAPType,
	OUT	ULONG	*MsgType);	

VOID WpaPskStateMachineInit(
		PRTMP_ADAPTER		pAd, 
		STATE_MACHINE		*S, 
	 STATE_MACHINE_FUNC Trans[]);

VOID WpaEAPOLKeyAction(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID	WpaPairMsg1Action(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID	WpaPairMsg3Action(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem); 

VOID	WpaGroupMsg1Action(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID	WpaMacHeaderInit(
			PRTMP_ADAPTER	pAd, 
	 	PHEADER_802_11	pHdr80211, 
			unsigned char			wep, 
			unsigned char*			pAddr1); 

VOID	Wpa2PairMsg1Action(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID	Wpa2PairMsg3Action(
		PRTMP_ADAPTER	pAd, 
		MLME_QUEUE_ELEM *Elem);

VOID ParseKeyData(
		PRTMP_ADAPTER	pAd,
		unsigned char*			pKeyData,
		unsigned char			KeyDataLen,
		unsigned char			KeyIdx,
		unsigned char			IsGroupMsg);
	
VOID WPAMake8023Hdr(
    IN PRTMP_ADAPTER    pAd, 
    IN PCHAR            pDAddr, 
    IN OUT PCHAR        pHdr);
	
VOID RTMPToWirelessSta(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pFrame,
    IN  UINT            FrameLen);

VOID	HMAC_SHA1(
	IN	UCHAR	*text,
	IN	UINT	text_len,
	IN	UCHAR	*key,
	IN	UINT	key_len,
	IN	UCHAR	*digest);

VOID	PRF(
		unsigned char	*key,
		INT 	key_len,
		unsigned char	*prefix,
		INT 	prefix_len,
		unsigned char	*data,
		INT 	data_len,
	 unsigned char	*output,
		INT 	len);

VOID WpaCountPTK(
		unsigned char	*PMK,
		unsigned char	*ANonce,
		unsigned char	*AA,
		unsigned char	*SNonce,
		unsigned char	*SA,
	 unsigned char	*output,
		unsigned int	len);

VOID	GenRandom(
		PRTMP_ADAPTER	pAd, 
	 unsigned char			*random);

VOID	AES_GTK_KEY_UNWRAP( 
		unsigned char	*key,
	 unsigned char	*plaintext,
	 unsigned char	c_len,
		unsigned char	*ciphertext);

//#ifdef RALINK_WPA_SUPPLICANT_SUPPORT
INT	RTMPCheckWPAframeForEapCode(
	IN PRTMP_ADAPTER   		pAd,
	IN PUCHAR				pFrame,
	IN ULONG				FrameLen,
	IN ULONG				OffSet);
//#endif
//Benson 07-11-22 add for countermeasure
VOID	WpaMicFailureReportFrame(
	  PRTMP_ADAPTER   pAd,
	 MLME_QUEUE_ELEM *Elem);

VOID    WpaDisassocApAndBlockAssoc(
      VOID * SystemSpecific1, 
      VOID * FunctionContext, 
      VOID * SystemSpecific2, 
      VOID * SystemSpecific3);
//Benson 07-11-22 end for countermeasure

//
// prototypes for *iwpriv*  rtmp_info.c
//
INT rt73_ioctl(
		struct net_device	*net_dev, 
			struct ifreq	*rq, 
		INT					cmd);
	
INT RTMPSetInformation(
	IN	PRTMP_ADAPTER pAdapter,
	IN	OUT struct ifreq	*rq,
	IN	INT 			cmd);

INT RTMPQueryInformation(
		PRTMP_ADAPTER pAdapter,
		 struct ifreq	*rq,
		INT 			cmd);

CHAR *GetEncryptType(
	CHAR enc, 
	CHAR encBitMode);

CHAR *GetAuthMode(
	CHAR auth,
	CHAR authBitMode);

NDIS_STATUS	RTMPWPAAddKeyProc(
		PRTMP_ADAPTER	pAd,
		VOID *			pBuf);

NDIS_STATUS	RTMPWPARemoveKeyProc(
		PRTMP_ADAPTER	pAd,
		VOID *			pBuf);

VOID	RTMPWPARemoveAllKeys(
		PRTMP_ADAPTER	pAd);

VOID RTMPIndicateWPA2Status(
		PRTMP_ADAPTER  pAd);

VOID	RTMPSetPhyMode(
		PRTMP_ADAPTER	pAd,
		ULONG			phymode);

VOID	RTMPSetDesiredRates(
		PRTMP_ADAPTER	pAdapter,
		LONG			Rates);
	
INT Set_DriverVersion_Proc(
		PRTMP_ADAPTER	pAd, 
		unsigned char*			arg);

INT Set_CountryRegion_Proc(
		PRTMP_ADAPTER	pAd, 
		unsigned char*			arg);

INT Set_CountryRegionABand_Proc(
		PRTMP_ADAPTER	pAd, 
		unsigned char*			arg);

INT Set_SSID_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_WirelessMode_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_TxRate_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_AdhocModeRate_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_Channel_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

#ifdef DBG
INT	Set_Debug_Proc(
		PRTMP_ADAPTER	pAd, 
		unsigned char*			arg);
#endif	//#ifdef DBG  

INT Set_BGProtection_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_TxPreamble_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_RTSThreshold_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_FragThreshold_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_TxBurst_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

#ifdef AGGREGATION_SUPPORT
INT	Set_PktAggregate_Proc(
		PRTMP_ADAPTER	pAd, 
		unsigned char*			arg);
#endif	/* AGGREGATION_SUPPORT */

INT Set_TurboRate_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

#if 0
INT	Set_WmmCapable_Proc(
		PRTMP_ADAPTER	pAd, 
		unsigned char*			arg);
#endif

INT Set_ShortSlot_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_IEEE80211H_Proc(
		PRTMP_ADAPTER	pAd, 
		unsigned char*			arg);

INT Set_NetworkType_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_AuthMode_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_EncrypType_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_DefaultKeyID_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_Key1_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_Key2_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_Key3_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_Key4_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT Set_WPAPSK_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_ResetStatCounter_Proc(
		PRTMP_ADAPTER	pAd, 
		unsigned char*			arg);
	
INT Set_PSMode_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_TxQueSize_Proc(
		PRTMP_ADAPTER	pAd, 
		unsigned char*			arg);

#ifdef DBG
VOID RTMPIoctlBBP(
		PRTMP_ADAPTER	pAdapter,
		struct iwreq	*wrq);

VOID RTMPIoctlMAC(
		PRTMP_ADAPTER	pAdapter, 
		struct iwreq	*wrq);

#ifdef RALINK_ATE
VOID RTMPIoctlE2PROM(
		PRTMP_ADAPTER	pAdapter, 
		struct iwreq	*wrq);
#endif // RALINK_ATE

#endif //#ifdef DBG

VOID RTMPIoctlStatistics(
		PRTMP_ADAPTER	pAd, 
		struct iwreq	*wrq);

VOID RTMPIoctlGetSiteSurvey(
		PRTMP_ADAPTER	pAdapter, 
		struct iwreq	*wrq);

VOID RTMPMakeRSNIE(
		PRTMP_ADAPTER	pAdapter,
		unsigned char			GroupCipher);

NDIS_STATUS RTMPWPANoneAddKeyProc(
		PRTMP_ADAPTER	pAd,
		VOID *			pBuf);

#ifdef RALINK_ATE
INT	Set_ATE_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_ATE_DA_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_ATE_SA_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_ATE_BSSID_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_ATE_CHANNEL_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_ATE_TX_POWER_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_ATE_TX_FREQOFFSET_Proc(
		PRTMP_ADAPTER	pAd, 
		unsigned char*			arg);
	
INT	Set_ATE_TX_LENGTH_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_ATE_TX_COUNT_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

INT	Set_ATE_TX_RATE_Proc(
		PRTMP_ADAPTER	pAdapter, 
		unsigned char*			arg);

VOID RTMPStationStop(
      PRTMP_ADAPTER   pAd);

VOID RTMPStationStart(
      PRTMP_ADAPTER   pAd);

VOID ATEAsicSwitchChannel(
     PRTMP_ADAPTER pAd, 
     unsigned char Channel);

VOID ATE_RTUSBBulkOutDataPacketComplete(
	purbb_t purb, struct pt_regs *pt_regs);

VOID ATE_RTUSBBulkOutDataPacket(
		PRTMP_ADAPTER	pAd);


VOID ATE_RTMPSendNullFrame(
		PRTMP_ADAPTER	pAd); 

#endif  // RALINK_ATE

#if 1
INT RTMPIoctlSetAuth(	
		PRTMP_ADAPTER	pAdapter, 
		struct iwreq	*wrq);

INT RTMPIoctlSetKeyId(
		PRTMP_ADAPTER	pAdapter, 
		struct iwreq	*wrq);

INT RTMPIoctlSetEncryp(
		PRTMP_ADAPTER	pAdapter, 
		struct iwreq	*wrq);

INT RTMPIoctlSetWpapsk(
		PRTMP_ADAPTER	pAdapter, 
		struct iwreq	*wrq);

INT RTMPIoctlSetPsm(
		PRTMP_ADAPTER	pAdapter, 
		struct iwreq	*wrq);
#endif

char * rstrtok(char * s,const char * ct);

#ifdef BIG_ENDIAN
static inline VOID	RTMPDescriptorEndianChange(
		unsigned char*			pData,
		unsigned int			DescriptorType)
{
	int size = (DescriptorType == TYPE_TXD) ? TXD_SIZE : RXD_SIZE;
	int i;
	for (i=1; i<size/4; i++) {
		/*
		 * Handle IV and EIV with little endian
		 */
		if (DescriptorType == TYPE_TXD) {		
			 /* Skip Word 3 IV and Word 4 EIV of TXD */			
			if (i==3||i==4)  
				continue; 
		}
		else {	
			 /* Skip Word 2 IV and Word 3 EIV of RXD */	
			if (i==2||i==3)  
				continue; 
		}
		*((unsigned int *)(pData + i*4)) = SWAP32(*((unsigned int *)(pData + i*4))); 
	}
	*(unsigned int *)pData = SWAP32(*(unsigned int *)pData);	// Word 0; this must be swapped last

}

static inline void	WriteBackToDescriptor(
		unsigned char*			Dest,
		unsigned char*			Src,
		unsigned char			DoEncrypt,
		unsigned int			DescriptorType)
{
	unsigned int* p1;
	unsigned int* p2;
	unsigned char i;
	int size = (DescriptorType == TYPE_TXD) ? TXD_SIZE : RXD_SIZE;

	p1 = ((unsigned int*)Dest) + 1;
	p2 = ((unsigned int*)Src) + 1;
	for(i = 1;i < size/4 ;i++)
	{
		*p1++ = *p2++;
	}
	*(unsigned int*)Dest = *(unsigned int*)Src;		// Word 0; this must be written back last
}

static inline VOID	RTMPFrameEndianChange(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PUCHAR			pData, 
	IN	ULONG			Dir,
	IN	BOOLEAN 		FromRxDoneInt)
{
	PFRAME_802_11	pFrame;
	unsigned char*			pMacHdr;

	// swab 16 bit fields - Frame Control field
	if(Dir == DIR_READ)
	{
		*(unsigned short *)pData = SWAP16(*(unsigned short *)pData);
	}

	pFrame = (PFRAME_802_11) pData;
	pMacHdr = (unsigned char*) pFrame;

	// swab 16 bit fields - Duration/ID field
	*(unsigned short *)(pMacHdr + 2) = SWAP16(*(unsigned short *)(pMacHdr + 2));

	// swab 16 bit fields - Sequence Control field
	*(unsigned short *)(pMacHdr + 22) = SWAP16(*(unsigned short *)(pMacHdr + 22));
	
	//mpDebugPrint("pFrame->Hdr.FC.Type %x",pFrame->Hdr.FC.Type);
	//mpDebugPrint("pFrame->Hdr.FC.SubType %x",pFrame->Hdr.FC.SubType);

	if(pFrame->Hdr.FC.Type == BTYPE_MGMT)
	{
		switch(pFrame->Hdr.FC.SubType)
		{
			case SUBTYPE_ASSOC_REQ:
			case SUBTYPE_REASSOC_REQ:
				
				//mpDebugPrint("SUBTYPE_ASSOC_REQ ");
				// swab 16 bit fields - CapabilityInfo field
				pMacHdr += LENGTH_802_11;
				*(unsigned short *)pMacHdr = SWAP16(*(unsigned short *)pMacHdr);

				// swab 16 bit fields - Listen Interval field
				pMacHdr += 2;
				*(unsigned short *)pMacHdr = SWAP16(*(unsigned short *)pMacHdr);
				break;

			case SUBTYPE_ASSOC_RSP:
			case SUBTYPE_REASSOC_RSP:
				
				mpDebugPrint("SUBTYPE_ASSOC_RSP ");
				// swab 16 bit fields - CapabilityInfo field
				pMacHdr += LENGTH_802_11;
				*(unsigned short *)pMacHdr = SWAP16(*(unsigned short *)pMacHdr);

				// swab 16 bit fields - Status Code field
				pMacHdr += 2;
				*(unsigned short *)pMacHdr = SWAP16(*(unsigned short *)pMacHdr);
				
				// swab 16 bit fields - AID field
				pMacHdr += 2;
				*(unsigned short *)pMacHdr = SWAP16(*(unsigned short *)pMacHdr);
				break;

			case SUBTYPE_AUTH:
				
				//K mpDebugPrint("SUBTYPE_AUTH ");
				// If from RTMPHandleRxDoneInterrupt routine, it is still a encrypt format.
				// The convertion is delayed to RTMPHandleDecryptionDoneInterrupt.
				if(!FromRxDoneInt && pFrame->Hdr.FC.Wep != 1)
				{
					// swab 16 bit fields - Auth Alg No. field
					pMacHdr += LENGTH_802_11;
					*(unsigned short *)pMacHdr = SWAP16(*(unsigned short *)pMacHdr);

					// swab 16 bit fields - Auth Seq No. field
					pMacHdr += 2;
					*(unsigned short *)pMacHdr = SWAP16(*(unsigned short *)pMacHdr);

					// swab 16 bit fields - Status Code field
					pMacHdr += 2;
					*(unsigned short *)pMacHdr = SWAP16(*(unsigned short *)pMacHdr);
				}
				break;

			case SUBTYPE_BEACON:
				//K7 mpDebugPrint("SUBTYPE_BEACON ");
			case SUBTYPE_PROBE_RSP:
				//K7 mpDebugPrint("SUBTYPE_PROBE_RSP");
				// swab 16 bit fields - BeaconInterval field
				pMacHdr += LENGTH_802_11 + TIMESTAMP_LEN;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				// swab 16 bit fields - CapabilityInfo field
				pMacHdr += sizeof(USHORT);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_DEAUTH:
			case SUBTYPE_DISASSOC:
				
				mpDebugPrint("SUBTYPE_DEAUTH");
				// swab 16 bit fields - Reason code field
				pMacHdr += LENGTH_802_11;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;
		}
	}
	else if(pFrame->Hdr.FC.Type == BTYPE_DATA)
	{
#if 0
		mpDebugPrint("Hdr.FC.Type == BTYPE_DATA");
		
		mpDebugPrint("addr1 %x %x %x %x %x %x",pFrame->Hdr.Addr1[0],
			pFrame->Hdr.Addr1[1],pFrame->Hdr.Addr1[2],pFrame->Hdr.Addr1[3],
			pFrame->Hdr.Addr1[4],pFrame->Hdr.Addr1[5]);
		//mpDebugPrint("addr2 %x %x %x %x %x %x",pFrame->Hdr.Addr2[0],
		//	pFrame->Hdr.Addr2[1],pFrame->Hdr.Addr2[2],pFrame->Hdr.Addr2[3],
		//	pFrame->Hdr.Addr2[4],pFrame->Hdr.Addr2[5]);
		mpDebugPrint("addr3 %x %x %x %x %x %x",pFrame->Hdr.Addr3[0],
			pFrame->Hdr.Addr3[1],pFrame->Hdr.Addr3[2],pFrame->Hdr.Addr3[3],
			pFrame->Hdr.Addr3[4],pFrame->Hdr.Addr3[5]);
#endif
	}
	else if(pFrame->Hdr.FC.Type == BTYPE_CNTL)
	{
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,"Invalid Frame Type!!!\n");
	}

	// swab 16 bit fields - Frame Control
	if(Dir == DIR_WRITE)
	{
		*(unsigned short *)pData = SWAP16(*(unsigned short *)pData);
	}
}

static inline unsigned int   RTMPEqualMemory(
		VOID *	pSrc1,
		VOID *	pSrc2,
		unsigned int	Length)
{
	unsigned char*	pMem1;
	unsigned char*	pMem2;
	unsigned int	Index = 0;

	pMem1 = (unsigned char*) pSrc1;
	pMem2 = (unsigned char*) pSrc2;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] != pMem2[Index])
		{
			break;
		}
	}

	if (Index == Length)
	{
		return (1);
	}
	else
	{
		return (0);
	}
}
#endif

VOID RTMPIoctlGetRaAPCfg(	
		PRTMP_ADAPTER	pAdapter, 	
		struct iwreq	*wrq);

BOOLEAN BackDoorProbeRspSanity(    
	IN PRTMP_ADAPTER pAd,     
	IN VOID *Msg,     
	IN ULONG MsgLen,    
	OUT CHAR *pCfgDataBuf);

#endif /* __RTMP_H__ */

#define simple_strtol(cp, endp, base) simple_mp_strtol(cp, endp, base) 


