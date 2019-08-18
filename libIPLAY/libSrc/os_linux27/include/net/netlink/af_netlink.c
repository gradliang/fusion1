
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/mutex.h>
#include <net/sock.h>
#include <mpTrace_copy.h>
#include <net_netlink.h>

#ifdef PLATFORM_MPIXEL
#include "mpTrace_copy.h"
#include "log.h"
static struct mutex nl_table_cb_mutex;
extern int NetlinkRead(struct sock *sk, struct sk_buff *skb);
#define yield() TaskYield()
static int netlink_dump(struct sock *sk);
#define panic(fmt, ...)  BREAK_POINT()
extern void SockIdSignalRxDone3(U16 sid, void* skb);
#endif

#define NETLINK_KERNEL_SOCKET	0x1
#define NETLINK_RECV_PKTINFO	0x2

extern void genl_rcv(struct sk_buff *skb);

static struct netlink_sock kernel_netlink_socket = {
    .nl_family = AF_NETLINK,
    .pid = 0,
    .u16SockId = 255,        /* use a dummy one */
    .flags = NETLINK_KERNEL_SOCKET,
    .netlink_rcv = genl_rcv
};


static inline struct netlink_sock *nlk_sk(struct sock *sk)
{
	return container_of(sk, struct netlink_sock, sk);
}

static u32 netlink_group_mask(u32 group)
{
	return group ? 1 << (group - 1) : 0;
}

static inline int netlink_capable(struct socket *sock, unsigned int flag)
{
	return true;
}

static void netlink_sock_destruct(struct sock *sk)
{
#ifdef LINUX
	struct netlink_sock *nlk = nlk_sk(sk);

	if (nlk->cb) {
		if (nlk->cb->done)
			nlk->cb->done(nlk->cb);
		netlink_destroy_callback(nlk->cb);
	}

	skb_queue_purge(&sk->sk_receive_queue);

	if (!sock_flag(sk, SOCK_DEAD)) {
		printk(KERN_ERR "Freeing alive netlink socket %p\n", sk);
		return;
	}

	WARN_ON(atomic_read(&sk->sk_rmem_alloc));
	WARN_ON(atomic_read(&sk->sk_wmem_alloc));
	WARN_ON(nlk_sk(sk)->groups);
#else
    MP_ASSERT(0);
#endif
}

static void netlink_table_grab(void)
	__acquires(nl_table_lock)
{
#ifdef LINUX
	write_lock_irq(&nl_table_lock);

	if (atomic_read(&nl_table_users)) {
		DECLARE_WAITQUEUE(wait, current);

		add_wait_queue_exclusive(&nl_table_wait, &wait);
		for (;;) {
			set_current_state(TASK_UNINTERRUPTIBLE);
			if (atomic_read(&nl_table_users) == 0)
				break;
			write_unlock_irq(&nl_table_lock);
			schedule();
			write_lock_irq(&nl_table_lock);
		}

		__set_current_state(TASK_RUNNING);
		remove_wait_queue(&nl_table_wait, &wait);
	}
#endif
}

static void netlink_table_ungrab(void)
	__releases(nl_table_lock)
{
#ifdef LINUX
	write_unlock_irq(&nl_table_lock);
	wake_up(&nl_table_wait);
#endif
}
static inline int netlink_is_kernel(struct sock *sk)
{
	return nlk_sk(sk)->flags & NETLINK_KERNEL_SOCKET;
}

struct nl_pid_hash {
	struct hlist_head *table;
	unsigned long rehash_time;

	unsigned int mask;
	unsigned int shift;

	unsigned int entries;
	unsigned int max_shift;

	u32 rnd;
};

struct netlink_table {
	struct nl_pid_hash hash;
	struct hlist_head mc_list;
	unsigned long *listeners;
	unsigned int nl_nonroot;
	unsigned int groups;
	struct mutex *cb_mutex;
	struct module *module;
	int registered;
};

static struct netlink_table *nl_table;

static inline void
netlink_lock_table(void)
{
	/* read_lock() synchronizes us to netlink_table_grab */

#ifdef LINUX
	read_lock(&nl_table_lock);
	atomic_inc(&nl_table_users);
	read_unlock(&nl_table_lock);
#endif
}

static inline void
netlink_unlock_table(void)
{
#ifdef LINUX
	if (atomic_dec_and_test(&nl_table_users))
		wake_up(&nl_table_wait);
#endif
}

static const struct proto_ops netlink_ops;

static struct proto netlink_proto = {
	.name	  = "NETLINK",
	.owner	  = THIS_MODULE,
	.obj_size = sizeof(struct netlink_sock),
};

static int __netlink_create(struct net *net, struct socket *sock,
			    struct mutex *cb_mutex, int protocol)
{
	struct sock *sk;
	struct netlink_sock *nlk;

	sock->ops = &netlink_ops;

#ifdef LINUX
	sk = sk_alloc(net, PF_NETLINK, GFP_KERNEL, &netlink_proto);
	if (!sk)
		return -ENOMEM;
#else
	sk = sock->sk;
#endif

	sock_init_data(sock, sk);

	nlk = nlk_sk(sk);
	if (cb_mutex)
		nlk->cb_mutex = cb_mutex;
	else {
		nlk->cb_mutex = &nlk->cb_def_mutex;
		mutex_init(nlk->cb_mutex);
	}
	init_waitqueue_head(&nlk->wait);

	sk->sk_destruct = netlink_sock_destruct;
	sk->sk_protocol = protocol;
	return 0;
}

static int netlink_create(struct net *net, struct socket *sock, int protocol)
{
#ifdef LINUX
	struct module *module = NULL;
#endif
	struct mutex *cb_mutex;
	struct netlink_sock *nlk;
	int err = 0;

	sock->state = SS_UNCONNECTED;

	if (sock->type != SOCK_RAW && sock->type != SOCK_DGRAM)
		return -ESOCKTNOSUPPORT;

	if (protocol < 0 || protocol >= MAX_LINKS)
    {
        MP_ASSERT(0);
		return -EPROTONOSUPPORT;
    }

	netlink_lock_table();
#ifdef CONFIG_KMOD
	if (!nl_table[protocol].registered) {
		netlink_unlock_table();
		request_module("net-pf-%d-proto-%d", PF_NETLINK, protocol);
		netlink_lock_table();
	}
#endif
#ifdef LINUX
	if (nl_table[protocol].registered &&
	    try_module_get(nl_table[protocol].module))
		module = nl_table[protocol].module;
	cb_mutex = nl_table[protocol].cb_mutex;
#else
	cb_mutex = &nl_table_cb_mutex;
#endif
	netlink_unlock_table();

	err = __netlink_create(net, sock, cb_mutex, protocol);
	if (err < 0)
		goto out_module;

#ifdef LINUX
	nlk = nlk_sk(sock->sk);
	nlk->module = module;
#endif
out:
	return err;

out_module:
#ifdef LINUX
	module_put(module);
#endif
	goto out;
}

int netlink_rcv_skb(struct sk_buff *skb, int (*cb)(struct sk_buff *,
						     struct nlmsghdr *))
{
	struct nlmsghdr *nlh;
	int err;

//    DBG("skb=%p", skb);
	while (skb->len >= nlmsg_total_size(0)) {
		int msglen;

		nlh = nlmsg_hdr(skb);
		err = 0;

		if (nlh->nlmsg_len < NLMSG_HDRLEN || skb->len < nlh->nlmsg_len)
			return 0;

		/* Only requests are handled by the kernel */
		if (!(nlh->nlmsg_flags & NLM_F_REQUEST))
			goto ack;

//        DBG("nlmsg_type=%d", nlh->nlmsg_type);
		/* Skip control messages */
		if (nlh->nlmsg_type < NLMSG_MIN_TYPE)
			goto ack;

		err = cb(skb, nlh);
        if (err < 0)
            DBG("cb() returns error=%d", err);

		if (err == -EINTR)
        {
			goto skip;
        }

ack:
		if (nlh->nlmsg_flags & NLM_F_ACK || err)
			netlink_ack(skb, nlh, err);

skip:
		msglen = NLMSG_ALIGN(nlh->nlmsg_len);
		if (msglen > skb->len)
			msglen = skb->len;
		skb_pull(skb, msglen);
	}

	return 0;
}

/*
 * Attach a skb to a netlink socket.
 * The caller must hold a reference to the destination socket. On error, the
 * reference is dropped. The skb is not send to the destination, just all
 * all error checks are performed and memory in the queue is reserved.
 * Return values:
 * < 0: error. skb freed, reference to sock dropped.
 * 0: continue
 * 1: repeat lookup - reference dropped while waiting for socket memory.
 */
int netlink_attachskb(struct sock *sk, struct sk_buff *skb,
		      long *timeo, struct sock *ssk)
{
#ifdef LINUX
	struct netlink_sock *nlk;

	nlk = nlk_sk(sk);

	if (atomic_read(&sk->sk_rmem_alloc) > sk->sk_rcvbuf ||
	    test_bit(0, &nlk->state)) {
		DECLARE_WAITQUEUE(wait, current);
		if (!*timeo) {
			if (!ssk || netlink_is_kernel(ssk))
				netlink_overrun(sk);
			sock_put(sk);
			kfree_skb(skb);
			return -EAGAIN;
		}

		__set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&nlk->wait, &wait);

		if ((atomic_read(&sk->sk_rmem_alloc) > sk->sk_rcvbuf ||
		     test_bit(0, &nlk->state)) &&
		    !sock_flag(sk, SOCK_DEAD))
			*timeo = schedule_timeout(*timeo);

		__set_current_state(TASK_RUNNING);
		remove_wait_queue(&nlk->wait, &wait);
		sock_put(sk);

		if (signal_pending(current)) {
			kfree_skb(skb);
			return sock_intr_errno(*timeo);
		}
		return 1;
	}
#endif
	skb_set_owner_r(skb, sk);
	return 0;
}

int netlink_sendskb(struct sock *sk, struct sk_buff *skb)
{
	int len = skb->len;

#ifdef LINUX
	skb_queue_tail(&sk->sk_receive_queue, skb);
	sk->sk_data_ready(sk, len);
#else
    DBG("skb=%p; skb->len=%d", skb, len);
    NetlinkRead(sk, skb);
#endif
	sock_put(sk);
	return len;
}

static inline struct sk_buff *netlink_trim(struct sk_buff *skb,
					   gfp_t allocation)
{
	int delta;

	skb_orphan(skb);

	delta = skb->end - skb->tail;
	if (delta * 2 < skb->truesize)
		return skb;

	if (skb_shared(skb)) {
        MP_ASSERT(0);
		struct sk_buff *nskb = skb_clone(skb, allocation);
		if (!nskb)
			return skb;
		kfree_skb(skb);
		skb = nskb;
	}

	if (!pskb_expand_head(skb, 0, -delta, allocation))
		skb->truesize -= delta;

	return skb;
}

static inline int netlink_unicast_kernel(struct sock *sk, struct sk_buff *skb)
{
	int ret;
	struct netlink_sock *nlk = nlk_sk(sk);

//    DBG("skb=%p", skb);
	ret = -ECONNREFUSED;
    MP_ASSERT(nlk->netlink_rcv);
	if (nlk->netlink_rcv != NULL) {
		ret = skb->len;
		skb_set_owner_r(skb, sk);
		nlk->netlink_rcv(skb);
	}
	kfree_skb(skb);
	sock_put(sk);
	return ret;
}

static void netlink_overrun(struct sock *sk)
{
        MP_ASSERT(0);
	if (!test_and_set_bit(0, &nlk_sk(sk)->state)) {
		sk->sk_err = ENOBUFS;
		sk->sk_error_report(sk);
	}
}

#ifdef PLATFORM_MPIXEL
static __u32 global_nl_pid;
static int global_nl_fd;
void m_global_nl_set_kernel(int fd, __u32 pid)
{
    global_nl_fd = fd;
    global_nl_pid = pid;
}
#endif

static struct sock *netlink_getsockbypid(struct sock *ssk, u32 pid)
{
	struct sock *sock;
	struct netlink_sock *nlk;

#ifdef LINUX
	sock = netlink_lookup(sock_net(ssk), ssk->sk_protocol, pid);
	if (!sock)
		return ERR_PTR(-ECONNREFUSED);

	/* Don't bother queuing skb if kernel socket has no input function */
	nlk = nlk_sk(sock);
	if (sock->sk_state == NETLINK_CONNECTED &&
	    nlk->dst_pid != nlk_sk(ssk)->pid) {
		sock_put(sock);
		return ERR_PTR(-ECONNREFUSED);
	}
#else
    extern struct netlink_sock *netlink_socket_get(int s);
    if (pid == 0)
    {
        sock = &kernel_netlink_socket.sk;
        sock_hold(sock);
    }
    else if (pid == global_nl_pid)
    {
        sock = netlink_socket_get(global_nl_fd);
        sock_hold(sock);
    }
    else
        MP_ASSERT(0);
#endif
	return sock;
}

static void
netlink_update_listeners(struct sock *sk)
{
#ifdef LINUX
	struct netlink_table *tbl = &nl_table[sk->sk_protocol];
	struct hlist_node *node;
	unsigned long mask;
	unsigned int i;

	for (i = 0; i < NLGRPLONGS(tbl->groups); i++) {
		mask = 0;
		sk_for_each_bound(sk, node, &tbl->mc_list) {
			if (i < NLGRPLONGS(nlk_sk(sk)->ngroups))
				mask |= nlk_sk(sk)->groups[i];
		}
		tbl->listeners[i] = mask;
	}
	/* this function is only called with the netlink table "grabbed", which
	 * makes sure updates are visible before bind or setsockopt return. */
#else
#endif
}

static int netlink_insert(struct sock *sk, struct net *net, u32 pid)
{
#ifdef LINUX
	struct nl_pid_hash *hash = &nl_table[sk->sk_protocol].hash;
	struct hlist_head *head;
	int err = -EADDRINUSE;
	struct sock *osk;
	struct hlist_node *node;
	int len;

	netlink_table_grab();
	head = nl_pid_hashfn(hash, pid);
	len = 0;
	sk_for_each(osk, node, head) {
		if (net_eq(sock_net(osk), net) && (nlk_sk(osk)->pid == pid))
			break;
		len++;
	}
	if (node)
		goto err;

	err = -EBUSY;
	if (nlk_sk(sk)->pid)
		goto err;

	err = -ENOMEM;
	if (BITS_PER_LONG > 32 && unlikely(hash->entries >= UINT_MAX))
		goto err;

	if (len && nl_pid_hash_dilute(hash, len))
		head = nl_pid_hashfn(hash, pid);
	hash->entries++;
	nlk_sk(sk)->pid = pid;
	sk_add_node(sk, head);
	err = 0;

err:
	netlink_table_ungrab();
	return err;
#else
	int err = -EADDRINUSE;
	netlink_table_grab();
	nlk_sk(sk)->pid = pid;
	err = 0;
	netlink_table_ungrab();
	return 0;
#endif
}

static void
netlink_update_subscriptions(struct sock *sk, unsigned int subscriptions)
{
#ifdef LINUX
	struct netlink_sock *nlk = nlk_sk(sk);

	if (nlk->subscriptions && !subscriptions)
		__sk_del_bind_node(sk);
	else if (!nlk->subscriptions && subscriptions)
		sk_add_bind_node(sk, &nl_table[sk->sk_protocol].mc_list);
	nlk->subscriptions = subscriptions;
#else
#endif
}

static int netlink_autobind(struct socket *sock)
{
#ifdef LINUX
	struct sock *sk = sock->sk;
	struct net *net = sock_net(sk);
	struct nl_pid_hash *hash = &nl_table[sk->sk_protocol].hash;
	struct hlist_head *head;
	struct sock *osk;
	struct hlist_node *node;
	s32 pid = current->tgid;
	int err;
	static s32 rover = -4097;

retry:
	cond_resched();
	netlink_table_grab();
	head = nl_pid_hashfn(hash, pid);
	sk_for_each(osk, node, head) {
		if (!net_eq(sock_net(osk), net))
			continue;
		if (nlk_sk(osk)->pid == pid) {
			/* Bind collision, search negative pid values. */
			pid = rover--;
			if (rover > -4097)
				rover = -4097;
			netlink_table_ungrab();
			goto retry;
		}
	}
	netlink_table_ungrab();

	err = netlink_insert(sk, net, pid);
	if (err == -EADDRINUSE)
		goto retry;

	/* If 2 threads race to autobind, that is fine.  */
	if (err == -EBUSY)
		err = 0;

	return err;
#else
        MP_ASSERT(0);
#endif
}

static int netlink_bind(struct socket *sock, struct sockaddr *addr,
			int addr_len)
{
	struct sock *sk = sock->sk;
	struct net *net = sock_net(sk);
	struct netlink_sock *nlk = nlk_sk(sk);
	struct sockaddr_nl *nladdr = (struct sockaddr_nl *)addr;
	int err;

	if (nladdr->nl_family != AF_NETLINK)
		return -EINVAL;

    DBG("s=%x,nl_groups=%p,nl_pid=%d", nlk->u16SockId, nladdr->nl_groups, nladdr->nl_pid);
	/* Only superuser is allowed to listen multicasts */
	if (nladdr->nl_groups) {
		if (!netlink_capable(sock, NL_NONROOT_RECV))
			return -EPERM;
		err = netlink_realloc_groups(sk);
		if (err)
			return err;
	}

	if (nlk->pid) {
		if (nladdr->nl_pid != nlk->pid)
			return -EINVAL;
	} else {
		err = nladdr->nl_pid ?
			netlink_insert(sk, net, nladdr->nl_pid) :
			netlink_autobind(sock);
		if (err)
			return err;
	}

	if (!nladdr->nl_groups && (nlk->groups == NULL || !(u32)nlk->groups[0]))
    {
		return 0;
    }

	netlink_table_grab();
	netlink_update_subscriptions(sk, nlk->subscriptions +
					 hweight32(nladdr->nl_groups) -
					 hweight32(nlk->groups[0]));
    MP_ASSERT(nlk->groups);
	nlk->groups[0] = (nlk->groups[0] & ~0xffffffffUL) | nladdr->nl_groups;
	netlink_update_listeners(sk);
	netlink_table_ungrab();

	return 0;
}

struct sk_buff dod11;
int netlink_unicast(struct sock *ssk, struct sk_buff *skb,
		    u32 pid, int nonblock)
{
	struct sock *sk;
	int err;
	long timeo;
    struct sk_buff *dod = skb;

    dod11 = *skb;
//    DBG("skb=%p, skb->len=%d", skb, skb->len);
	skb = netlink_trim(skb, gfp_any());
    if (skb != dod)
        MP_ASSERT(0);

	timeo = sock_sndtimeo(ssk, nonblock);
retry:
	sk = netlink_getsockbypid(ssk, pid);
//    DBG("send to sid=%x,pid=%d", get_netlink_sock_fd(sk), pid);
	if (IS_ERR(sk)) {
		kfree_skb(skb);
		return PTR_ERR(sk);
	}
	if (netlink_is_kernel(sk))
		return netlink_unicast_kernel(sk, skb);

#ifdef LINUX
	if (sk_filter(sk, skb)) {
		err = skb->len;
		kfree_skb(skb);
		sock_put(sk);
		return err;
	}
#endif

	err = netlink_attachskb(sk, skb, &timeo, ssk);
	if (err == 1)
		goto retry;
	if (err)
		return err;

	return netlink_sendskb(sk, skb);
}
EXPORT_SYMBOL(netlink_unicast);

static inline int netlink_broadcast_deliver(struct sock *sk,
					    struct sk_buff *skb)
{
	struct netlink_sock *nlk = nlk_sk(sk);

#ifdef LINUX
	if (atomic_read(&sk->sk_rmem_alloc) <= sk->sk_rcvbuf &&
	    !test_bit(0, &nlk->state)) {
#else
	if (!test_bit(0, &nlk->state)) {        // TODO
#endif
		skb_set_owner_r(skb, sk);
#ifdef LINUX
		skb_queue_tail(&sk->sk_receive_queue, skb);
#else
        SockIdSignalRxDone3(nlk->u16SockId, skb);
		sk->sk_data_ready(sk, skb->len);
#endif
		return atomic_read(&sk->sk_rmem_alloc) > sk->sk_rcvbuf;
	}
	return -1;
}

struct netlink_broadcast_data {
	struct sock *exclude_sk;
	struct net *net;
	u32 pid;
	u32 group;
	int failure;
	int congested;
	int delivered;
	gfp_t allocation;
	struct sk_buff *skb, *skb2;
};

int netlink_broadcast_count;

static inline int do_one_broadcast(struct sock *sk,
				   struct netlink_broadcast_data *p)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	int val;

    DBG("sk=%p, sid=%d, p=%p", sk, get_netlink_sock_fd(sk), p);
	if (p->exclude_sk == sk)
		goto out;

	if (nlk->pid == p->pid || p->group - 1 >= nlk->ngroups ||
	    !test_bit(p->group - 1, nlk->groups))
		goto out;

#ifdef LINUX
	if (!net_eq(sock_net(sk), p->net))
		goto out;
#endif

	if (p->failure) {
        DBG("p->failure is true");
		netlink_overrun(sk);
		goto out;
	}

//    DBG("p->skb2=%p", p->skb2);
	sock_hold(sk);
	if (p->skb2 == NULL) {
		if (skb_shared(p->skb)) {
            DBG("skb_shared is true");
			p->skb2 = skb_clone(p->skb, p->allocation);
		} else {
			p->skb2 = skb_get(p->skb);
			/*
			 * skb ownership may have been set when
			 * delivered to a previous socket.
			 */
			skb_orphan(p->skb2);
		}
	}
	if (p->skb2 == NULL) {
        DBG("p->skb2==NULL");
		netlink_overrun(sk);
		/* Clone failed. Notify ALL listeners. */
		p->failure = 1;
	} else if (sk_filter(sk, p->skb2)) {
        DBG("sk_filter() returns true");
		kfree_skb(p->skb2);
		p->skb2 = NULL;
	} else if ((val = netlink_broadcast_deliver(sk, p->skb2)) < 0) {
        DBG("netlink_broadcast_deliver() returns error %d", val);
		netlink_overrun(sk);
	} else {
		p->congested |= val;
		p->delivered = 1;
		p->skb2 = NULL;
	}
	sock_put(sk);

out:
//    DBG("returns");
	return 0;
}

int netlink_broadcast(struct sock *ssk, struct sk_buff *skb, u32 pid,
		      u32 group, gfp_t allocation)
{
	struct net *net = sock_net(ssk);
	struct netlink_broadcast_data info;
	struct hlist_node *node;
	struct sock *sk;

	skb = netlink_trim(skb, allocation);

	info.exclude_sk = ssk;
	info.net = net;
	info.pid = pid;
	info.group = group;
	info.failure = 0;
	info.congested = 0;
	info.delivered = 0;
	info.allocation = allocation;
	info.skb = skb;
	info.skb2 = NULL;

	/* While we sleep in clone, do not allow to change socket list */

	netlink_lock_table();


	sk_for_each_bound(sk, node, &nl_table[ssk->sk_protocol].mc_list)
		do_one_broadcast(sk, &info);

	kfree_skb(skb);

	netlink_unlock_table();

	if (info.skb2)
		kfree_skb(info.skb2);

	if (info.delivered) {
		if (info.congested && (allocation & __GFP_WAIT))
			yield();
		return 0;
	}
	if (info.failure)
		return -ENOBUFS;

	return -ESRCH;
}
EXPORT_SYMBOL(netlink_broadcast);

void netlink_ack(struct sk_buff *in_skb, struct nlmsghdr *nlh, int err)
{
	struct sk_buff *skb;
	struct nlmsghdr *rep;
	struct nlmsgerr *errmsg;
	size_t payload = sizeof(*errmsg);

	/* error messages get the original request appened */
	if (err)
		payload += nlmsg_len(nlh);

	skb = nlmsg_new(payload, GFP_KERNEL);
	if (!skb) {
		struct sock *sk;

#ifdef LINUX
		sk = netlink_lookup(sock_net(in_skb->sk),
				    in_skb->sk->sk_protocol,
				    NETLINK_CB(in_skb).pid);
		if (sk) {
			sk->sk_err = ENOBUFS;
			sk->sk_error_report(sk);
			sock_put(sk);
		}
#else
        MP_ASSERT(0);
#endif
		return;
	}

	rep = __nlmsg_put(skb, NETLINK_CB(in_skb).pid, nlh->nlmsg_seq,
			  NLMSG_ERROR, sizeof(struct nlmsgerr), 0);
	errmsg = nlmsg_data(rep);
	errmsg->error = err;
    if (err < 0)
    {
        DBG("err=%d", err);
#if 0
        if (err != -EINVAL &&
            err != -ENOENT)
            MP_ASSERT(0);
#endif
    }
	memcpy(&errmsg->msg, nlh, err ? nlh->nlmsg_len : sizeof(*nlh));
    DBG("calls netlink_unicast pid=%d", NETLINK_CB(in_skb).pid);
	netlink_unicast(in_skb->sk, skb, NETLINK_CB(in_skb).pid, MSG_DONTWAIT);
}

int netlink_dump_start(struct sock *ssk, struct sk_buff *skb,
		       struct nlmsghdr *nlh,
		       int (*dump)(struct sk_buff *skb,
				   struct netlink_callback *),
		       int (*done)(struct netlink_callback *))
{
#ifdef LINUX
	struct netlink_callback *cb;
	struct sock *sk;
	struct netlink_sock *nlk;

	cb = kzalloc(sizeof(*cb), GFP_KERNEL);
	if (cb == NULL)
		return -ENOBUFS;

	cb->dump = dump;
	cb->done = done;
	cb->nlh = nlh;
	atomic_inc(&skb->users);
	cb->skb = skb;

	sk = netlink_lookup(sock_net(ssk), ssk->sk_protocol, NETLINK_CB(skb).pid);
	if (sk == NULL) {
		netlink_destroy_callback(cb);
		return -ECONNREFUSED;
	}
	nlk = nlk_sk(sk);
	/* A dump is in progress... */
	mutex_lock(nlk->cb_mutex);
	if (nlk->cb) {
		mutex_unlock(nlk->cb_mutex);
		netlink_destroy_callback(cb);
		sock_put(sk);
		return -EBUSY;
	}
	nlk->cb = cb;
	mutex_unlock(nlk->cb_mutex);

	netlink_dump(sk);
	sock_put(sk);

	/* We successfully started a dump, by returning -EINTR we
	 * signal not to send ACK even if it was requested.
	 */
#endif
	return -EINTR;
}

static inline void netlink_rcv_wake(struct sock *sk)
{
	struct netlink_sock *nlk = nlk_sk(sk);

#ifdef LINUX
	if (skb_queue_empty(&sk->sk_receive_queue))
		clear_bit(0, &nlk->state);
	if (!test_bit(0, &nlk->state))
		wake_up_interruptible(&nlk->wait);
#else
    //MP_ASSERT(0); XXX
#endif
}

static void netlink_cmsg_recv_pktinfo(struct msghdr *msg, struct sk_buff *skb)
{
	struct nl_pktinfo info;

#ifdef LINUX
	info.group = NETLINK_CB(skb).dst_group;
	put_cmsg(msg, SOL_NETLINK, NETLINK_PKTINFO, sizeof(info), &info);
#else
    MP_ASSERT(0);
#endif
}

static int netlink_recvmsg(struct kiocb *kiocb, struct socket *sock,
			   struct msghdr *msg, size_t len,
			   int flags)
{
	struct sock_iocb *siocb = kiocb_to_siocb(kiocb);
#ifdef LINUX
	struct scm_cookie scm;
#endif
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	int noblock = flags&MSG_DONTWAIT;
	size_t copied;
	struct sk_buff *skb;
	int err;

	if (flags&MSG_OOB)
		return -EOPNOTSUPP;

	copied = 0;

	skb = skb_recv_datagram(sk, flags, noblock, &err);
	if (skb == NULL)
		goto out;

	msg->msg_namelen = 0;

    DBG("skb=%p;skb->len=%d,len=%d", skb, skb->len, len);
	copied = skb->len;
	if (len < copied) {
        MP_ASSERT(0);
		msg->msg_flags |= MSG_TRUNC;
		copied = len;
	}
//    DBG("msg->msg_flags=%x,copied=%d", msg->msg_flags, copied);

	skb_reset_transport_header(skb);
	err = skb_copy_datagram_iovec(skb, 0, msg->msg_iov, copied);

	if (msg->msg_name) {
		struct sockaddr_nl *addr = (struct sockaddr_nl *)msg->msg_name;
		addr->nl_family = AF_NETLINK;
		addr->nl_pad    = 0;
		addr->nl_pid	= NETLINK_CB(skb).pid;
		addr->nl_groups	= netlink_group_mask(NETLINK_CB(skb).dst_group);
		msg->msg_namelen = sizeof(*addr);
	}

	if (nlk->flags & NETLINK_RECV_PKTINFO)
		netlink_cmsg_recv_pktinfo(msg, skb);

#ifdef LINUX
	if (NULL == siocb->scm) {
		memset(&scm, 0, sizeof(scm));
		siocb->scm = &scm;
	}
	siocb->scm->creds = *NETLINK_CREDS(skb);
#endif
//    DBG("flags=%x,copied=%d,skb->len=%d", flags, copied, skb->len);
	if (flags & MSG_TRUNC)
		copied = skb->len;
	skb_free_datagram(sk, skb);

	if (nlk->cb && atomic_read(&sk->sk_rmem_alloc) <= sk->sk_rcvbuf / 2)
		netlink_dump(sk);

#ifdef LINUX
	scm_recv(sock, msg, siocb->scm, flags);
#endif
out:
	netlink_rcv_wake(sk);
//    DBG("err=%d,copied=%d", err, copied);
    MP_ASSERT(err == 0);
	return err ? : copied;
}

static const struct proto_ops netlink_ops = {
	.family =	PF_NETLINK,
	.owner =	THIS_MODULE,
#ifdef LINUX
	.release =	netlink_release,
	.bind =		netlink_bind,
	.connect =	netlink_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	sock_no_accept,
	.getname =	netlink_getname,
	.poll =		datagram_poll,
	.ioctl =	sock_no_ioctl,
	.listen =	sock_no_listen,
	.shutdown =	sock_no_shutdown,
	.setsockopt =	netlink_setsockopt,
	.getsockopt =	netlink_getsockopt,
	.sendmsg =	netlink_sendmsg,
	.recvmsg =	netlink_recvmsg,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
#endif
};

static void netlink_destroy_callback(struct netlink_callback *cb)
{
	if (cb->skb)
		kfree_skb(cb->skb);
	kfree(cb);
}

/*
 * It looks a bit ugly.
 * It would be better to create kernel thread.
 */

static int netlink_dump(struct sock *sk)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	struct netlink_callback *cb;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	int len, err = -ENOBUFS;

#ifdef LINUX
	skb = sock_rmalloc(sk, NLMSG_GOODSIZE, 0, GFP_KERNEL);
#else
    MP_ASSERT(0);
#endif
	if (!skb)
		goto errout;

	mutex_lock(nlk->cb_mutex);

	cb = nlk->cb;
	if (cb == NULL) {
		err = -EINVAL;
		goto errout_skb;
	}

	len = cb->dump(skb, cb);

	if (len > 0) {
		mutex_unlock(nlk->cb_mutex);

		if (sk_filter(sk, skb))
			kfree_skb(skb);
		else {
			skb_queue_tail(&sk->sk_receive_queue, skb);
			sk->sk_data_ready(sk, skb->len);
		}
		return 0;
	}

	nlh = nlmsg_put_answer(skb, cb, NLMSG_DONE, sizeof(len), NLM_F_MULTI);
	if (!nlh)
		goto errout_skb;

	memcpy(nlmsg_data(nlh), &len, sizeof(len));

	if (sk_filter(sk, skb))
		kfree_skb(skb);
	else {
		skb_queue_tail(&sk->sk_receive_queue, skb);
		sk->sk_data_ready(sk, skb->len);
	}

	if (cb->done)
		cb->done(cb);
	nlk->cb = NULL;
	mutex_unlock(nlk->cb_mutex);

	netlink_destroy_callback(cb);
	return 0;

errout_skb:
	mutex_unlock(nlk->cb_mutex);
	kfree_skb(skb);
errout:
	return err;
}

static void netlink_data_ready(struct sock *sk, int len)
{
	BUG();
}

int dod74;
struct sock *
netlink_kernel_create(struct net *net, int unit, unsigned int groups,
		      void (*input)(struct sk_buff *skb),
		      struct mutex *cb_mutex, struct module *module)
{
	struct socket *sock;
	struct sock *sk;
	struct netlink_sock *nlk;
	unsigned long *listeners = NULL;
#ifdef LINUX

	BUG_ON(!nl_table);

	if (unit < 0 || unit >= MAX_LINKS)
		return NULL;

	if (sock_create_lite(PF_NETLINK, SOCK_DGRAM, unit, &sock))
		return NULL;

	/*
	 * We have to just have a reference on the net from sk, but don't
	 * get_net it. Besides, we cannot get and then put the net here.
	 * So we create one inside init_net and the move it to net.
	 */

	if (__netlink_create(&init_net, sock, cb_mutex, unit) < 0)
		goto out_sock_release_nosk;

	sk = sock->sk;
	sk_change_net(sk, net);

	if (groups < 32)
		groups = 32;

	listeners = kzalloc(NLGRPSZ(groups), GFP_KERNEL);
	if (!listeners)
		goto out_sock_release;

	sk->sk_data_ready = netlink_data_ready;
	if (input)
		nlk_sk(sk)->netlink_rcv = input;

	if (netlink_insert(sk, net, 0))
		goto out_sock_release;

	nlk = nlk_sk(sk);
	nlk->flags |= NETLINK_KERNEL_SOCKET;

	netlink_table_grab();
	if (!nl_table[unit].registered) {
		nl_table[unit].groups = groups;
		nl_table[unit].listeners = listeners;
		nl_table[unit].cb_mutex = cb_mutex;
		nl_table[unit].module = module;
		nl_table[unit].registered = 1;
	} else {
		kfree(listeners);
		nl_table[unit].registered++;
	}
	netlink_table_ungrab();
#else
    static struct netlink_sock _genl_sock[2];

    sk = (struct sock *)&_genl_sock[dod74];

    dod74++;

    DBG("cnt=%d, proto=%d",dod74,unit);
    MP_ASSERT(dod74 < 3);

    sk->sk_protocol = unit;

	if (groups < 32)
		groups = 32;

	sk->sk_data_ready = netlink_data_ready;
	if (input)
		nlk_sk(sk)->netlink_rcv = input;

	if (netlink_insert(sk, net, 0))
		goto out_sock_release;

	nlk = nlk_sk(sk);
	nlk->flags |= NETLINK_KERNEL_SOCKET;

	netlink_table_grab();
	if (!nl_table[unit].registered) {
		nl_table[unit].groups = groups;
		nl_table[unit].registered = 1;
    }
    else {
		kfree(listeners);
		nl_table[unit].registered++;
    }
	netlink_table_ungrab();
#endif

	return sk;

out_sock_release:
	kfree(listeners);
	netlink_kernel_release(sk);
	return NULL;

out_sock_release_nosk:
	sock_release(sock);
	return NULL;
}
EXPORT_SYMBOL(netlink_kernel_create);


void
netlink_kernel_release(struct sock *sk)
{
	sk_release_kernel(sk);
}
EXPORT_SYMBOL(netlink_kernel_release);
/**
 * netlink_change_ngroups - change number of multicast groups
 *
 * This changes the number of multicast groups that are available
 * on a certain netlink family. Note that it is not possible to
 * change the number of groups to below 32. Also note that it does
 * not implicitly call netlink_clear_multicast_users() when the
 * number of groups is reduced.
 *
 * @sk: The kernel netlink socket, as returned by netlink_kernel_create().
 * @groups: The new number of groups.
 */
int netlink_change_ngroups(struct sock *sk, unsigned int groups)
{
	unsigned long *listeners, *old = NULL;
	struct netlink_table *tbl = &nl_table[sk->sk_protocol];
	int err = 0;

	if (groups < 32)
		groups = 32;

	netlink_table_grab();
#ifdef LINUX
	if (NLGRPSZ(tbl->groups) < NLGRPSZ(groups)) {
		listeners = kzalloc(NLGRPSZ(groups), GFP_ATOMIC);
		if (!listeners) {
			err = -ENOMEM;
			goto out_ungrab;
		}
		old = tbl->listeners;
		memcpy(listeners, old, NLGRPSZ(tbl->groups));
		rcu_assign_pointer(tbl->listeners, listeners);
	}
#endif
	tbl->groups = groups;

 out_ungrab:
	netlink_table_ungrab();
	synchronize_rcu();
	kfree(old);
	return err;
}
EXPORT_SYMBOL(netlink_change_ngroups);

/* must be called with netlink table grabbed */
static void netlink_update_socket_mc(struct netlink_sock *nlk,
				     unsigned int group,
				     int is_new)
{
	int old, new = !!is_new, subscriptions;

	old = test_bit(group - 1, nlk->groups);
	subscriptions = nlk->subscriptions - old + new;
	if (new)
		__set_bit(group - 1, nlk->groups);
	else
		__clear_bit(group - 1, nlk->groups);
#ifdef LINUX
	netlink_update_subscriptions(&nlk->sk, subscriptions);
	netlink_update_listeners(&nlk->sk);
#else
    struct sock *sk = &nlk->sk;
	if (nlk->subscriptions && !subscriptions)
		__sk_del_bind_node(sk);
	else if (!nlk->subscriptions && subscriptions)
    {
		sk_add_bind_node(sk, &nl_table[sk->sk_protocol].mc_list);
    }
	nlk->subscriptions = subscriptions;
	//netlink_update_listeners(&nlk->sk); TODO
#endif
}

static int netlink_realloc_groups(struct sock *sk)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	unsigned int groups;
	unsigned long *new_groups;
	int err = 0;

	netlink_table_grab();

	groups = nl_table[sk->sk_protocol].groups;
	if (!nl_table[sk->sk_protocol].registered) {
        MP_ASSERT(0);
		err = -ENOENT;
		goto out_unlock;
	}

	if (nlk->ngroups >= groups)
    {
		goto out_unlock;
    }

	new_groups = krealloc(nlk->groups, NLGRPSZ(groups), GFP_ATOMIC);
	if (new_groups == NULL) {
        MP_ASSERT(0);
		err = -ENOMEM;
		goto out_unlock;
	}
	memset((char *)new_groups + NLGRPSZ(nlk->ngroups), 0,
	       NLGRPSZ(groups) - NLGRPSZ(nlk->ngroups));

	nlk->groups = new_groups;
	nlk->ngroups = groups;
 out_unlock:
	netlink_table_ungrab();
	return err;
}

static int netlink_setsockopt(struct socket *sock, int level, int optname,
			      char __user *optval, int optlen)
{
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	unsigned int val = 0;
	int err;

    DBG("sk=%p, optname=%d", sk, optname);
	if (level != SOL_NETLINK)
		return -ENOPROTOOPT;

	if (optlen >= sizeof(int) &&
	    get_user(val, (unsigned int __user *)optval))
    {
        MP_ASSERT(0);
		return -EFAULT;
    }

	switch (optname) {
	case NETLINK_PKTINFO:
		if (val)
			nlk->flags |= NETLINK_RECV_PKTINFO;
		else
			nlk->flags &= ~NETLINK_RECV_PKTINFO;
		err = 0;
		break;
	case NETLINK_ADD_MEMBERSHIP:
	case NETLINK_DROP_MEMBERSHIP: {
		if (!netlink_capable(sock, NL_NONROOT_RECV))
			return -EPERM;
		err = netlink_realloc_groups(sk);
		if (err)
        {
            MP_ASSERT(0);
			return err;
        }
		if (!val || val - 1 >= nlk->ngroups)
        {
            MP_ASSERT(0);
			return -EINVAL;
        }
		netlink_table_grab();
		netlink_update_socket_mc(nlk, val,
					 optname == NETLINK_ADD_MEMBERSHIP);
		netlink_table_ungrab();
		err = 0;
		break;
	}
	default:
        MP_ASSERT(0);
		err = -ENOPROTOOPT;
	}
	return err;
}

/**
 * netlink_clear_multicast_users - kick off multicast listeners
 *
 * This function removes all listeners from the given group.
 * @ksk: The kernel netlink socket, as returned by
 *	netlink_kernel_create().
 * @group: The multicast group to clear.
 */
void netlink_clear_multicast_users(struct sock *ksk, unsigned int group)
{
#ifdef LINUX
	struct sock *sk;
	struct hlist_node *node;
	struct netlink_table *tbl = &nl_table[ksk->sk_protocol];

	netlink_table_grab();

	sk_for_each_bound(sk, node, &tbl->mc_list)
		netlink_update_socket_mc(nlk_sk(sk), group, 0);

	netlink_table_ungrab();
#else
    MP_ASSERT(0);
#endif
}
EXPORT_SYMBOL(netlink_clear_multicast_users);

static int __init netlink_proto_init(void)
{
	struct sk_buff *dummy_skb;
	int i;
	unsigned long limit;
	unsigned int order;
#ifdef LINUX
	int err = proto_register(&netlink_proto, 0);
#else
	int err = 0;
#endif

	if (err != 0)
		goto out;

	BUILD_BUG_ON(sizeof(struct netlink_skb_parms) > sizeof(dummy_skb->cb));

	nl_table = kcalloc(MAX_LINKS, sizeof(*nl_table), GFP_KERNEL);
	if (!nl_table)
		goto panic;
#ifdef LINUX

	if (num_physpages >= (128 * 1024))
		limit = num_physpages >> (21 - PAGE_SHIFT);
	else
		limit = num_physpages >> (23 - PAGE_SHIFT);

	order = get_bitmask_order(limit) - 1 + PAGE_SHIFT;
	limit = (1UL << order) / sizeof(struct hlist_head);
	order = get_bitmask_order(min(limit, (unsigned long)UINT_MAX)) - 1;

	for (i = 0; i < MAX_LINKS; i++) {
		struct nl_pid_hash *hash = &nl_table[i].hash;

		hash->table = nl_pid_hash_zalloc(1 * sizeof(*hash->table));
		if (!hash->table) {
			while (i-- > 0)
				nl_pid_hash_free(nl_table[i].hash.table,
						 1 * sizeof(*hash->table));
			kfree(nl_table);
			goto panic;
		}
		hash->max_shift = order;
		hash->shift = 0;
		hash->mask = 0;
		hash->rehash_time = jiffies;
	}

	sock_register(&netlink_family_ops);
	register_pernet_subsys(&netlink_net_ops);
	/* The netlink device handler may be needed early. */
	rtnetlink_init();
#endif
out:
	return err;
panic:
	panic("netlink_init: Cannot allocate nl_table\n");
}

#ifdef PLATFORM_MPIXEL
int m_netlink_create(struct socket *sock, int protocol)
{
    return netlink_create(NULL, sock, protocol);
}

int m_netlink_init(void)
{
    struct mutex *mutex = &nl_table_cb_mutex;
    struct sock *sock;
    int r;
    if (!mutex->semaphore)
    {
        mutex->semaphore = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        atomic_set(&mutex->count, 1);

        MP_ASSERT(mutex->semaphore > 0);
    }

    r = netlink_proto_init();

    /* populate kernel netlink socket */
    sock_init_data(NULL, &kernel_netlink_socket);

    return 0;
}

/* for netlink socket only */
int get_netlink_sock_fd(struct sock *sk)
{
    struct netlink_sock *nlk = nlk_sk(sk);
    MP_ASSERT (nlk->u16SockId > 0);
    return (int)nlk->u16SockId;
}


int m_netlink_recvmsg(struct socket *sock,
			   struct msghdr *msg, size_t len,
			   int flags)
{
    return netlink_recvmsg(NULL, sock, msg, len, flags); 
}

u32 m_netlink_group_mask(u32 group)
{
    return netlink_group_mask(group);
}

int m_netlink_setsockopt(struct socket *sock, int level, int optname,
			      char __user *optval, int optlen)
{
    int r = netlink_setsockopt(sock, level, optname, optval, optlen);
    if (r < 0)
    {
        setErrno(-r);
        return -1;
    }
    else
        return 0;

}

int m_netlink_bind(struct socket *sock, struct sockaddr *addr,
			int addr_len)
{
    return netlink_bind(sock, addr, addr_len);
}
#endif

