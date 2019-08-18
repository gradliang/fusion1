#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H

/*
 * include/linux/spinlock.h - generic spinlock/rwlock declarations
 *
 * here's the role of the various spinlock/rwlock related include files:
 *
 * on SMP builds:
 *
 *  asm/spinlock_types.h: contains the raw_spinlock_t/raw_rwlock_t and the
 *                        initializers
 *
 *  linux/spinlock_types.h:
 *                        defines the generic type and initializers
 *
 *  asm/spinlock.h:       contains the __raw_spin_*()/etc. lowlevel
 *                        implementations, mostly inline assembly code
 *
 *   (also included on UP-debug builds:)
 *
 *  linux/spinlock_api_smp.h:
 *                        contains the prototypes for the _spin_*() APIs.
 *
 *  linux/spinlock.h:     builds the final spin_*() APIs.
 *
 * on UP builds:
 *
 *  linux/spinlock_type_up.h:
 *                        contains the generic, simplified UP spinlock type.
 *                        (which is an empty structure on non-debug builds)
 *
 *  linux/spinlock_types.h:
 *                        defines the generic type and initializers
 *
 *  linux/spinlock_up.h:
 *                        contains the __raw_spin_*()/etc. version of UP
 *                        builds. (which are NOPs on non-debug, non-preempt
 *                        builds)
 *
 *   (included on UP-non-debug builds:)
 *
 *  linux/spinlock_api_up.h:
 *                        builds the _spin_*() APIs.
 *
 *  linux/spinlock.h:     builds the final spin_*() APIs.
 */
#include <asm/atomic.h>

#include <linux/typecheck.h>
#include <linux/preempt.h>
#include <linux/linkage.h>
#include <linux/compiler.h>
#include <linux/thread_info.h>
#include <linux/kernel.h>
#include <linux/stringify.h>
//#include <linux/bottom_half.h>

#include <asm/system.h>

/*
 * Pull the raw_spinlock_t and raw_rwlock_t definitions:
 */
#include <linux/spinlock_types.h>

typedef unsigned char spinlock_t;

# define rwlock_init(lock)					\
	do { *(lock) = RW_LOCK_UNLOCKED; } while (0)

#define write_lock(lock)
#define read_lock(lock)

#define read_unlock_irqrestore(lock, flags) 
#define spin_lock_irqsave(lock, flags) \
	do {						\
		mpx_SemaphoreWait(*lock); 		\
	} while (0)

#define spin_unlock_irqrestore(lock, flags)  \
	do {						\
		mpx_SemaphoreRelease(*lock); 		\
	} while (0)

#define spin_lock(lock)     spin_lock_irqsave(lock,0)
#define spin_unlock(lock)   spin_unlock_irqrestore(lock,0)

#define spin_lock_bh(lock)      \
	do {						\
		mpx_SemaphoreWait(*lock); 		\
	} while (0)
#define spin_unlock_bh(lock)    \
	do {						\
		mpx_SemaphoreRelease(*lock); 		\
	} while (0)

#define spin_lock_irq(lock)     spin_lock_bh(lock)
#define spin_unlock_irq(lock)   spin_unlock_bh(lock)

#define	spin_lock_init(lock)	\
{									\
    int _id;					\
    _id = mp_OsSemaphore(); 		\
    mpDebugPrint("spin_lock_init: id=%d", _id); \
    *(lock) = (spinlock_t)_id;				\
}

# define read_unlock(lock)
# define write_unlock(lock)
# define read_unlock_irq(lock)
# define write_unlock_irq(lock)
/**
 * atomic_dec_and_lock - lock on reaching reference count zero
 * @atomic: the atomic counter
 * @lock: the spinlock in question
 *
 * Decrements @atomic by 1.  If the result is 0, returns true and locks
 * @lock.  Returns false for all other cases.
 */
extern int _atomic_dec_and_lock(atomic_t *atomic, spinlock_t *lock);
#define atomic_dec_and_lock(atomic, lock) \
		_atomic_dec_and_lock(atomic, lock)

#endif /* __LINUX_SPINLOCK_TYPES_H */

