/*
 * Mutexes: blocking mutual exclusion locks
 *
 * started by Ingo Molnar:
 *
 *  Copyright (C) 2004, 2005, 2006 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 *
 * This file contains the main data structure and API definitions.
 */
#ifndef __LINUX_MUTEX_H
#define __LINUX_MUTEX_H

#include <linux/list.h>
#include <linux/spinlock_types.h>
#include <linux/linkage.h>
#include <linux/lockdep.h>

#include <asm/atomic.h>

/*
 * Simple, straightforward mutexes with strict semantics:
 *
 * - only one task can hold the mutex at a time
 * - only the owner can unlock the mutex
 * - multiple unlocks are not permitted
 * - recursive locking is not permitted
 * - a mutex object must be initialized via the API
 * - a mutex object must not be initialized via memset or copying
 * - task may not exit with mutex held
 * - memory areas where held locks reside must not be freed
 * - held mutexes must not be reinitialized
 * - mutexes may not be used in irq contexts
 *
 * These semantics are fully enforced when DEBUG_MUTEXES is
 * enabled. Furthermore, besides enforcing the above rules, the mutex
 * debugging code also implements a number of additional features
 * that make lock debugging easier and faster:
 *
 * - uses symbolic names of mutexes, whenever they are printed in debug output
 * - point-of-acquire tracking, symbolic lookup of function names
 * - list of all locks held in the system, printout of them
 * - owner tracking
 * - detects self-recursing locks and prints out all relevant info
 * - detects multi-task circular deadlocks and prints out all affected
 *   locks and tasks (and only those tasks)
 */
struct mutex {
	/* 1: unlocked, 0: locked, negative: locked, possible waiters */
	atomic_t		count;
//	spinlock_t		wait_lock;
//	struct list_head	wait_list;
    unsigned char  semaphore;
};

# define mutex_init(mutex) \
do {							\
    int _id;					\
    _id = mp_OsSemaphore(); 	\
    mpDebugPrint("mutex_init: id=%d", _id); \
    (mutex)->semaphore = _id;				\
	atomic_set(&(mutex)->count, 1); \
} while (0)
# define mutex_destroy(mutex)				do { } while (0)

# define mutex_lock(mutex) \
do {							\
    MP_ASSERT((mutex)->semaphore > 0);	\
    if ((mutex)->semaphore > 0)	\
	{	\
		SemaphoreWait((mutex)->semaphore); \
		atomic_dec(&(mutex)->count); \
	}	\
} while (0)

# define mutex_unlock(mutex) \
do {							\
    if ((mutex)->semaphore > 0)	\
	{ \
		SemaphoreRelease((mutex)->semaphore); \
		atomic_inc(&(mutex)->count); \
	} \
} while (0)

#define __MUTEX_INITIALIZER(lockname) \
		{ .count = ATOMIC_INIT(1) \
        , .semaphore = 0 \
		}
#define DEFINE_MUTEX(mutexname) \
	struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

/**
 * mutex_is_locked - is the mutex locked
 * @lock: the mutex to be queried
 *
 * Returns 1 if the mutex is locked, 0 if unlocked.
 */
static inline int mutex_is_locked(struct mutex *lock)
{
	return atomic_read(&lock->count) != 1;
}
#endif
