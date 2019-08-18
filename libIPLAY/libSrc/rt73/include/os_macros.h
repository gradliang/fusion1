
#ifndef	_OS_MACROS_H
#define _OS_MACROS_H

#include "net_device.h"
#if 0
#define os_time_get()	jiffies

extern spinlock_t driver_lock;
extern unsigned long driver_flags;
#define OS_INT_DISABLE	spin_lock_irqsave(&driver_lock, driver_flags)
#define	OS_INT_RESTORE	spin_unlock_irqrestore(&driver_lock, driver_flags); \
			driver_lock = SPIN_LOCK_UNLOCKED

#define UpdateTransStart(dev) { \
	dev->trans_start = jiffies; \
}

#define OS_SET_THREAD_STATE(x)		set_current_state(x)

#define MODULE_GET	try_module_get(THIS_MODULE)
#define MODULE_PUT	module_put(THIS_MODULE)

#define OS_INIT_SEMAPHORE(x)    	init_MUTEX(x)
#define OS_ACQ_SEMAPHORE_BLOCK(x)	down_interruptible(x)
#define OS_ACQ_SEMAPHORE_NOBLOCK(x)	down_trylock(x)
#define OS_REL_SEMAPHORE(x) 		up(x)

/* Definitions below are needed for other OS like threadx */
#define	TX_DISABLE
#define TX_RESTORE
#define	ConfigureThreadPriority()
#define OS_INTERRUPT_SAVE_AREA
#define OS_FREE_LOCK(x)
#define TX_EVENT_FLAGS_SET(x, y, z)

static inline void
os_sched_timeout(u32 millisec)
{
    set_current_state(TASK_INTERRUPTIBLE);

    schedule_timeout((millisec * HZ) / 1000);
}

static inline void
os_schedule(u32 millisec)
{
    schedule_timeout((millisec * HZ) / 1000);
}
#endif

#if 0
static inline u32
get_utimeofday(void)
{
    struct timeval t;
    u32 ut;

    do_gettimeofday(&t);
    ut = (u32) t.tv_sec * 1000000 + ((u32) t.tv_usec);
    return ut;
}

static inline int
os_upload_rx_packet(wlan_private * priv, struct sk_buff *skb)
{

#define IPFIELD_ALIGN_OFFSET	2
    PRINTM(INFO, "skb->data=%p\n", skb->data);

    skb->dev = priv->wlan_dev.netdev;
    skb->protocol = eth_type_trans(skb, priv->wlan_dev.netdev);
    skb->ip_summed = CHECKSUM_UNNECESSARY;

    netif_rx(skb);

    return 0;
}

static inline void
os_free_tx_packet(wlan_private * priv)
{
    ulong flags;

    spin_lock_irqsave(&priv->adapter->CurrentTxLock, flags);

    if (priv->adapter->CurrentTxSkb) {
        kfree_skb(priv->adapter->CurrentTxSkb);
    }

    priv->adapter->CurrentTxSkb = NULL;

    spin_unlock_irqrestore(&priv->adapter->CurrentTxLock, flags);
}

/* netif carrier_on/off and start(wake)/stop_queue handling
		carrier_on	carrier_off	start_queue	stop_queue
 open		x(connect)	x(disconnect)	x
 close				x				x
 assoc		x				x
 adhoc-start	x				x
 adhoc-join	x				x
 scan-begin			x				x
 scan-end	x				x
 deauth				x				x
 ps-sleep			x				x
 ps-awake	x				x
 ds-enter			x				x
 ds-exit	x				x
 xmit								x
 xmit-done					x
 tx-timeout
 */
#endif

void netif_carrier_on(struct net_device *dev);
void netif_carrier_off(struct net_device *dev);
static inline void
os_carrier_on(PRTMP_ADAPTER priv)
{
    if (!netif_carrier_ok(priv->net_dev) &&
        (OPSTATUS_TEST_FLAG(priv, fOP_STATUS_MEDIA_STATE_CONNECTED)))
        netif_carrier_on(priv->net_dev);
}

static inline void
os_carrier_off(PRTMP_ADAPTER priv)
{
    if (netif_carrier_ok(priv->net_dev))
        netif_carrier_off(priv->net_dev);
}

#if 0
static inline void
os_start_queue(wlan_private * priv)
{
    if (netif_queue_stopped(priv->wlan_dev.netdev) &&
        (priv->adapter->MediaConnectStatus == WlanMediaStateConnected)) {
        netif_wake_queue(priv->wlan_dev.netdev);
    }
}

static inline void
os_stop_queue(wlan_private * priv)
{
    if (!netif_queue_stopped(priv->wlan_dev.netdev)) {
        netif_stop_queue(priv->wlan_dev.netdev);
    }
}

static inline int
os_queue_is_active(wlan_private * priv)
{
    return (netif_carrier_ok(priv->wlan_dev.netdev)
            && !netif_queue_stopped(priv->wlan_dev.netdev));
}
#endif

#endif /* _OS_MACROS_H */
