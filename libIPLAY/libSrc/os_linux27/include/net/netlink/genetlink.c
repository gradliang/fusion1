/*
 * =====================================================================================
 *
 *       Filename:  genetlink.c
 *
 *    Description:  
 *
 *        Company:  Magic Pixel, Inc.
 *
 * =====================================================================================
 */

#include <linux/types.h>
#include <linux/socket.h>
#include <linux/mutex.h>
#include <net/sock.h>
#include <net/genetlink.h>

#ifdef PLATFORM_MPIXEL
#define LOCAL_DEBUG_ENABLE 0
#include "mpTrace_copy.h"
#include "log.h"
#define panic(fmt, ...)  BREAK_POINT()
#endif

struct sock *genl_sock = NULL;

static DEFINE_MUTEX(genl_mutex); /* serialization of message processing */

static inline void genl_lock(void)
{
	mutex_lock(&genl_mutex);
}

static inline void genl_unlock(void)
{
	mutex_unlock(&genl_mutex);
}

#ifdef PLATFORM_MPIXEL
void genl_lock_init(void)
{
    if (genl_mutex.semaphore == 0)
    {
        genl_mutex.semaphore = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        atomic_set(&genl_mutex.count, 1);
    }
}

#define NETLINK_NL80211_PID 111
#define NETLINK_NLCTRL_PID 222

#define NETLINK_MLME_PID        17          /* GENL_MIN_ID + 1 */
#define NETLINK_SCAN_PID        18
#define NETLINK_CONFIG_PID      19
#define NETLINK_REGULATORY_PID  20

#endif

#define GENL_FAM_TAB_SIZE	16
#define GENL_FAM_TAB_MASK	(GENL_FAM_TAB_SIZE - 1)

static struct list_head family_ht[GENL_FAM_TAB_SIZE];

/*
 * Bitmap of multicast groups that are currently in use.
 *
 * To avoid an allocation at boot of just one unsigned long,
 * declare it global instead.
 * Bit 0 is marked as already used since group 0 is invalid.
 */
static unsigned long mc_group_start = 0x1;
static unsigned long *mc_groups = &mc_group_start;
static unsigned long mc_groups_longs = 1;
extern struct genl_family *m_nl80211_family();

static struct genl_family *genl_family_find_byid(unsigned int id)
{
	struct genl_family *f;

#ifdef LINUX
	list_for_each_entry(f, genl_family_chain(id), family_list)
		if (f->id == id)
			return f;
#else
#if CONFIG_LIB80211
    if (id == NETLINK_NL80211_PID)
        return m_nl80211_family();
    else if (id == GENL_ID_GENERATE)
    {
    }
    else
        MP_ASSERT(0);
#endif
#endif

	return NULL;
}

static struct genl_ops *genl_get_cmd(u8 cmd, struct genl_family *family)
{
	struct genl_ops *ops;

	list_for_each_entry(ops, &family->ops_list, ops_list)
		if (ops->cmd == cmd)
			return ops;

	return NULL;
}
static int genl_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
	struct genl_ops *ops;
	struct genl_family *family;
	struct genl_info info;
	struct genlmsghdr *hdr = nlmsg_data(nlh);
	int hdrlen, err;

//    DBG("skb=%p", skb);
	family = genl_family_find_byid(nlh->nlmsg_type);
	if (family == NULL)
    {
        MP_ASSERT(0);
		return -ENOENT;
    }

	hdrlen = GENL_HDRLEN + family->hdrsize;
	if (nlh->nlmsg_len < nlmsg_msg_size(hdrlen))
    {
        MP_ASSERT(0);
		return -EINVAL;
    }

	ops = genl_get_cmd(hdr->cmd, family);
	if (ops == NULL)
    {
        MP_ASSERT(0);
		return -EOPNOTSUPP;
    }
    DBG("ops=%p,cmd=%d", ops, hdr->cmd);

	if ((ops->flags & GENL_ADMIN_PERM) &&
	    security_netlink_recv(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (nlh->nlmsg_flags & NLM_F_DUMP) {
		if (ops->dumpit == NULL)
        {
            MP_ASSERT(0);
			return -EOPNOTSUPP;
        }

		genl_unlock();
		err = netlink_dump_start(genl_sock, skb, nlh,
					 ops->dumpit, ops->done);
		genl_lock();
		return err;
	}

	if (ops->doit == NULL)
    {
        MP_ASSERT(0);
		return -EOPNOTSUPP;
    }

	if (family->attrbuf) {
		err = nlmsg_parse(nlh, hdrlen, family->attrbuf, family->maxattr,
				  ops->policy);
		if (err < 0)
        {

            MP_ASSERT(0);
			return err;
        }
	}

	info.snd_seq = nlh->nlmsg_seq;
	info.snd_pid = NETLINK_CB(skb).pid;
	info.nlhdr = nlh;
	info.genlhdr = nlmsg_data(nlh);
	info.userhdr = nlmsg_data(nlh) + GENL_HDRLEN;
	info.attrs = family->attrbuf;

//    DBG("doit=%p", ops->doit);
	return ops->doit(skb, &info);
}

void genl_rcv(struct sk_buff *skb)
{
	genl_lock();
	netlink_rcv_skb(skb, &genl_rcv_msg);
	genl_unlock();
//    DBG("returns");
}

/**************************************************************************
 * Controller
 **************************************************************************/

static struct genl_family genl_ctrl = {
	.id = GENL_ID_CTRL,
	.name = "nlctrl",
	.version = 0x2,
	.maxattr = CTRL_ATTR_MAX,
};

static struct genl_multicast_group notify_grp;

/**
 * genl_register_mc_group - register a multicast group
 *
 * Registers the specified multicast group and notifies userspace
 * about the new group.
 *
 * Returns 0 on success or a negative error code.
 *
 * @family: The generic netlink family the group shall be registered for.
 * @grp: The group to register, must have a name.
 */
int genl_register_mc_group(struct genl_family *family,
			   struct genl_multicast_group *grp)
{
	int id;
	unsigned long *new_groups;
	int err;

	BUG_ON(grp->name[0] == '\0');

	genl_lock();

	/* special-case our own group */
	if (grp == &notify_grp)
		id = GENL_ID_CTRL;
	else
#ifdef LINUX
		id = find_first_zero_bit(mc_groups,
					 mc_groups_longs * BITS_PER_LONG);
#else
    {
        if (strcmp(grp->name, "mlme") == 0)
            id = NETLINK_MLME_PID;
        else if (strcmp(grp->name, "config") == 0)
            id = NETLINK_CONFIG_PID;
        else if (strcmp(grp->name, "scan") == 0)
            id = NETLINK_SCAN_PID;
        else if (strcmp(grp->name, "regulatory") == 0)
            id = NETLINK_REGULATORY_PID;
        else
            MP_ASSERT(0);
    }
#endif


#ifdef LINUX
	if (id >= mc_groups_longs * BITS_PER_LONG) {
		size_t nlen = (mc_groups_longs + 1) * sizeof(unsigned long);

		if (mc_groups == &mc_group_start) {
			new_groups = kzalloc(nlen, GFP_KERNEL);
			if (!new_groups) {
				err = -ENOMEM;
				goto out;
			}
			mc_groups = new_groups;
			*mc_groups = mc_group_start;
		} else {
			new_groups = krealloc(mc_groups, nlen, GFP_KERNEL);
			if (!new_groups) {
				err = -ENOMEM;
				goto out;
			}
			mc_groups = new_groups;
			mc_groups[mc_groups_longs] = 0;
		}
		mc_groups_longs++;
	}
#else
	if (id >= mc_groups_longs * BITS_PER_LONG) {
        MP_ASSERT(0);
	}
#endif

	err = netlink_change_ngroups(genl_sock,
				     mc_groups_longs * BITS_PER_LONG);
	if (err)
		goto out;

	grp->id = id;
	set_bit(id, mc_groups);
	list_add_tail(&grp->list, &family->mcast_groups);
	grp->family = family;

	genl_ctrl_event(CTRL_CMD_NEWMCAST_GRP, grp);
 out:
	genl_unlock();
	return err;
}
EXPORT_SYMBOL(genl_register_mc_group);

static void __genl_unregister_mc_group(struct genl_family *family,
				       struct genl_multicast_group *grp)
{
	BUG_ON(grp->family != family);
	netlink_clear_multicast_users(genl_sock, grp->id);
	clear_bit(grp->id, mc_groups);
	list_del(&grp->list);
	genl_ctrl_event(CTRL_CMD_DELMCAST_GRP, grp);
	grp->id = 0;
	grp->family = NULL;
}

/**
 * genl_unregister_mc_group - unregister a multicast group
 *
 * Unregisters the specified multicast group and notifies userspace
 * about it. All current listeners on the group are removed.
 *
 * Note: It is not necessary to unregister all multicast groups before
 *       unregistering the family, unregistering the family will cause
 *       all assigned multicast groups to be unregistered automatically.
 *
 * @family: Generic netlink family the group belongs to.
 * @grp: The group to unregister, must have been registered successfully
 *	 previously.
 */
void genl_unregister_mc_group(struct genl_family *family,
			      struct genl_multicast_group *grp)
{
	genl_lock();
	__genl_unregister_mc_group(family, grp);
	genl_unlock();
}

static void genl_unregister_mc_groups(struct genl_family *family)
{
	struct genl_multicast_group *grp, *tmp;

	list_for_each_entry_safe(grp, tmp, &family->mcast_groups, list)
		__genl_unregister_mc_group(family, grp);
}

/**
 * genl_register_ops - register generic netlink operations
 * @family: generic netlink family
 * @ops: operations to be registered
 *
 * Registers the specified operations and assigns them to the specified
 * family. Either a doit or dumpit callback must be specified or the
 * operation will fail. Only one operation structure per command
 * identifier may be registered.
 *
 * See include/net/genetlink.h for more documenation on the operations
 * structure.
 *
 * Returns 0 on success or a negative error code.
 */
int genl_register_ops(struct genl_family *family, struct genl_ops *ops)
{
	int err = -EINVAL;

	if (ops->dumpit == NULL && ops->doit == NULL)
		goto errout;

	if (genl_get_cmd(ops->cmd, family)) {
		err = -EEXIST;
		goto errout;
	}

	if (ops->dumpit)
		ops->flags |= GENL_CMD_CAP_DUMP;
	if (ops->doit)
		ops->flags |= GENL_CMD_CAP_DO;
	if (ops->policy)
		ops->flags |= GENL_CMD_CAP_HASPOL;

	genl_lock();
	list_add_tail(&ops->ops_list, &family->ops_list);
	genl_unlock();

#ifdef LINUX
	genl_ctrl_event(CTRL_CMD_NEWOPS, ops);
#endif
	err = 0;
errout:
	return err;
}

/**
 * genl_register_family - register a generic netlink family
 * @family: generic netlink family
 *
 * Registers the specified family after validating it first. Only one
 * family may be registered with the same family name or identifier.
 * The family id may equal GENL_ID_GENERATE causing an unique id to
 * be automatically generated and assigned.
 *
 * Return 0 on success or a negative error code.
 */
#ifdef LINUX
int genl_register_family(struct genl_family *family)
#else
int genl_register_family_kernel(struct genl_family *family)
#endif
{
	int err = -EINVAL;

	if (family->id && family->id < GENL_MIN_ID)
		goto errout;

	if (family->id > GENL_MAX_ID)
		goto errout;

	INIT_LIST_HEAD(&family->ops_list);
	INIT_LIST_HEAD(&family->mcast_groups);

	genl_lock();

#ifdef LINUX
	if (genl_family_find_byname(family->name)) {
		err = -EEXIST;
		goto errout_locked;
	}
#endif

	if (genl_family_find_byid(family->id)) {
		err = -EEXIST;
		goto errout_locked;
	}

	if (family->id == GENL_ID_GENERATE) {
#ifdef LINUX
		u16 newid = genl_generate_id();
#else
		u16 newid;
        if (strcpy(family->name, "nl80211"))
            newid = NETLINK_NL80211_PID;
        else
            MP_ASSERT(0);
#endif

		if (!newid) {
			err = -ENOMEM;
			goto errout_locked;
		}

		family->id = newid;
	}

	if (family->maxattr) {
		family->attrbuf = kmalloc((family->maxattr+1) *
					sizeof(struct nlattr *), GFP_KERNEL);
		if (family->attrbuf == NULL) {
			err = -ENOMEM;
			goto errout_locked;
		}
	} else
		family->attrbuf = NULL;

#ifdef LINUX
	list_add_tail(&family->family_list, genl_family_chain(family->id));
#endif
	genl_unlock();

#ifdef LINUX
	genl_ctrl_event(CTRL_CMD_NEWFAMILY, family);
#endif

	return 0;

errout_locked:
	genl_unlock();
errout:
	return err;
}

static struct genl_multicast_group notify_grp = {
	.name		= "notify",
};

/**
 * genl_unregister_family - unregister generic netlink family
 * @family: generic netlink family
 *
 * Unregisters the specified family.
 *
 * Returns 0 on success or a negative error code.
 */
#ifdef LINUX
int genl_unregister_family(struct genl_family *family)
#else
int genl_unregister_family_kernel(struct genl_family *family)
#endif
{
#ifdef LINUX
	struct genl_family *rc;

	genl_lock();

	genl_unregister_mc_groups(family);

	list_for_each_entry(rc, genl_family_chain(family->id), family_list) {
		if (family->id != rc->id || strcmp(rc->name, family->name))
			continue;

		list_del(&rc->family_list);
		INIT_LIST_HEAD(&family->ops_list);
		genl_unlock();

		kfree(family->attrbuf);
		genl_ctrl_event(CTRL_CMD_DELFAMILY, family);
		return 0;
	}

	genl_unlock();
#else
    MP_ASSERT(0);
#endif

	return -ENOENT;
}
static int __init genl_init(void)
{
    int err;
	/* we'll bump the group number right afterwards */
	genl_sock = netlink_kernel_create(&init_net, NETLINK_GENERIC, 0,
					  genl_rcv, &genl_mutex, THIS_MODULE);
	if (genl_sock == NULL)
		panic("GENL: Cannot initialize generic netlink\n");

	err = genl_register_mc_group(&genl_ctrl, &notify_grp);
	if (err < 0)
		goto errout_register;
    return 0;
errout_register:
#ifdef LINUX
	genl_unregister_family(&genl_ctrl);
#else
	genl_unregister_family_kernel(&genl_ctrl);
#endif
errout:
	panic("GENL: Cannot register controller: %d\n", err);
}

static int genl_ctrl_event(int event, void *data)
{
	struct sk_buff *msg;

	if (genl_sock == NULL)
		return 0;

#ifdef LINUX
	switch (event) {
	case CTRL_CMD_NEWFAMILY:
	case CTRL_CMD_DELFAMILY:
		msg = ctrl_build_family_msg(data, 0, 0, event);
		if (IS_ERR(msg))
			return PTR_ERR(msg);

		genlmsg_multicast(msg, 0, GENL_ID_CTRL, GFP_KERNEL);
		break;
	case CTRL_CMD_NEWMCAST_GRP:
	case CTRL_CMD_DELMCAST_GRP:
		msg = ctrl_build_mcgrp_msg(data, 0, 0, event);
		if (IS_ERR(msg))
			return PTR_ERR(msg);

		genlmsg_multicast(msg, 0, GENL_ID_CTRL, GFP_KERNEL);
		break;
	}
#endif

	return 0;
}

void genl_lock_init(void);
int m_genl_init(void)
{
    genl_lock_init();
    if (!genl_sock)
        return genl_init();
    else
        return 0;
}
