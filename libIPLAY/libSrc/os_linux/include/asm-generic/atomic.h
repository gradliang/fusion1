/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 *
 * But use these as seldom as possible since they are much more slower
 * than regular operations.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 97, 99, 2000, 03, 04, 06 by Ralf Baechle
 */
#ifndef _ASM_GENERIC_ATOMIC_H
#define _ASM_GENERIC_ATOMIC_H

//#include <linux/irqflags.h>
//#include <asm/barrier.h>
//#include <asm/cpu-features.h>
//#include <asm/war.h>

#define ATOMIC_INIT(i)    { (i) }

/*
 * atomic_read - read atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically reads the value of @v.
 */
#define atomic_read(v)		((v)->counter)

typedef atomic_t atomic_long_t;

#define ATOMIC_LONG_INIT(i)	ATOMIC_INIT(i)
#define atomic_long_set(v,i)		((v)->counter = (i))
#define atomic_long_read(v)		((v)->counter)

//#include <asm-generic/atomic.h>
#endif /* _ASM_ATOMIC_H */
