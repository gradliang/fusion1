
#ifndef __LINUX_RCUPDATE_H
#define __LINUX_RCUPDATE_H

#include <linux/cache.h>
#include <linux/spinlock.h>
#include <linux/completion.h>
#define rcu_assign_pointer(p, v) \
	({ \
		(p) = (v); \
	})
#include <linux/rculist.h>

#define rcu_read_lock() 
#define rcu_read_unlock()

#define rcu_dereference(p)     p

#define synchronize_rcu(p)

#endif

