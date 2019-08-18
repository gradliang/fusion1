/*
 * Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
 */

#ifndef _OS_HEADER_
#define _OS_HEADER_

#include "typedef.h"
#include "ndebug.h"
//added by cjHuang

#ifndef NULL
#define NULL 0
#endif

//typedef unsigned char  U08;	/* 8-bit	*/
//typedef unsigned short U16;	/* 16-bit	*/
//typedef unsigned long   U32;	/* 32-bit	*/
//typedef unsigned long long U64;	/* 64-bit	*/
//typedef char	S08;			/* 8-bit	*/
//typedef short	S16;			/* 16-bit	*/
//typedef long	S32;			/* 32-bit	*/
//typedef long	long S64;			/* 64-bit	*/

/* add by yuming */
//typedef U08 u8;
//typedef U16 u16;
//typedef U32 u32;
//typedef S08 s8;
//typedef U64 u64;
//typedef S64 s64;
//typedef S16 s16;
//typedef S32 s32;
#ifndef __KERNEL__
typedef U08 __u8;
typedef U16 __u16;
typedef U32 __u32;
typedef S08 __s8;
typedef U64 __u64;
typedef S16 __s16;
typedef S32 __s32;
//typedef unsigned int uint;
typedef unsigned long ulong;

typedef __u16 __le16;
typedef __u16 __be16;
typedef __u32 __le32;
//typedef __u32 __be32;
typedef __u64 __le64;
typedef __u16 __sum16;
#endif

#define __nocast
#define __bitwise__

#include <errno.h>
#include "typedef.h"

//typedef LONG *PLONG;
//typedef PLONG LONG_PTR;
typedef u32 *ULONG_PTR;
typedef u32 *Pu32;
typedef int WLAN_STATUS;
//typedef u8 BOOLEAN;
//typedef BOOLEAN *PBOOLEAN;
typedef u32 WLAN_OID;
//typedef unsigned short WCHAR; TODO openssl

//typedef unsigned char       u8_t;
//typedef unsigned short      u16_t;
//typedef unsigned long       u32_t;
//typedef unsigned long long  u64_t;

//typedef char *caddr_t;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef __user
#define __user
#endif


#ifndef MIN
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)		((a) > (b) ? (a) : (b))
#endif

#ifndef LINUX
/* from Linux's linux/errno.h */
#define ENOTSUPP	524	/* Operation is not supported */
#endif


//#define udelay(a) UtilWait(a)

//#define mdelay(a) UtilWait(a)

typedef int irqreturn_t;
#ifdef __KERNEL__
typedef irqreturn_t IRQ_RET_TYPE;
#define IRQ_RET		return IRQ_HANDLED
#else
#define IRQ_NONE	(0)
#define IRQ_HANDLED	(1)
#define IRQ_RETVAL(x)	((x) != 0)

typedef irqreturn_t IRQ_RET_TYPE;
#define IRQ_RET		return IRQ_HANDLED
#endif /* __KERNEL__ */
#if Make_DM9KS_ETHERNET
#define copy_to_user(to,from,size) memcpy(to,from,size)

#define copy_from_user(to,from,size) memcpy(to,from,size)

#define get_user(x,p) memcpy(&(x), p, sizeof(*(p)))
#define put_user(x,p) memcpy(p, &(x), sizeof(*(p)))

#else
#define copy_to_user(to,from,size) mmcp_memcpy(to,from,size)

#define copy_from_user(to,from,size) mmcp_memcpy(to,from,size)

#define get_user(x,p) mmcp_memcpy(&(x), p, sizeof(*(p)))
#define put_user(x,p) mmcp_memcpy(p, &(x), sizeof(*(p)))
#endif

//#define kmalloc(n, level)  OsUserBlkAllocate(n)
//#define kfree(n) OsUserBlkRelease(n)
//#define kfree_skb(n) OsUserBlkRelease(n)
#define printk mpDebugPrint
#define KERN_ERR
#define KERN_EMERG
#define KERN_INFO
#define KERN_WARNING
#define KERN_DEBUG
#define KERN_NOTICE
//#define PRINTM(buf,n) UartOutText(n);

#define wake_up_interruptible(n) 

#define wait_event_interruptible(a,b)

#include "param.h"

#define jiffies mpx_SystemTickerGet()


//cjHuang Modify
#if 0
#define OS_INT_DISABLE IntDisable()
#define OS_INT_RESTORE IntEnable()
else
#define OS_INT_DISABLE SDIO_MP615_DisEna();
#define OS_INT_RESTORE SDIO_MP615_IntEna();
#endif


#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_ZLEN	60		/* Min. octets in frame sans FCS */
#define ETH_DATA_LEN	1500		/* Max. octets in payload	 */
#define ETH_FRAME_LEN	1514		/* Max. octets in frame sans FCS */

int mp_OsSemaphore(void);
int mp_OsEvent(BYTE attr, U32 flags, BYTE curr_evt);

#endif /* _OS_HEADER1 */
