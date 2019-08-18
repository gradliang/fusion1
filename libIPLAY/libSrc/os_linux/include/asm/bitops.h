#ifndef _BLACKFIN_BITOPS_H
#define _BLACKFIN_BITOPS_H

/*
 * Copyright 1992, Linus Torvalds.
 */

#include <linux/compiler.h>
#include <asm/byteorder.h>	/* swab32 */
#include <asm/system.h>		/* save_flags */


#ifndef _LINUX_BITOPS_H
#error only <linux/bitops.h> can be included directly
#endif

#include <linux/compiler.h>
#include <linux/irqflags.h>
#include <linux/types.h>
#include <asm/barrier.h>
#include <asm/bug.h>
#include <asm/byteorder.h>		/* sigh ... */
#include <asm/cpu-features.h>
#include <asm/sgidefs.h>
#include <asm/war.h>

static __inline__ void set_bit(int nr, volatile unsigned long *addr)
{
	int *a = (int *)addr;
	int mask;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	*a |= mask;
	local_irq_restore(flags);
}

static __inline__ void __set_bit(int nr, volatile unsigned long *addr)
{
	int *a = (int *)addr;
	int mask;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	*a |= mask;
}

/*
 * clear_bit() doesn't provide any barrier for the compiler.
 */
#define smp_mb__before_clear_bit()	barrier()
#define smp_mb__after_clear_bit()	barrier()

static __inline__ void clear_bit(int nr, unsigned long *addr)
{
	int *a = (int *)addr;
	int mask;
	unsigned long flags;
	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	*a &= ~mask;
	local_irq_restore(flags);
}

static __inline__ void __clear_bit(int nr,  unsigned long *addr)
{
	int *a = (int *)addr;
	int mask;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	*a &= ~mask;
}

static __inline__ void change_bit(int nr, volatile unsigned long *addr)
{
	int mask, flags;
	unsigned long *ADDR = (unsigned long *)addr;

	ADDR += nr >> 5;
	mask = 1 << (nr & 31);
	local_irq_save(flags);
	*ADDR ^= mask;
	local_irq_restore(flags);
}

static __inline__ void __change_bit(int nr, volatile unsigned long *addr)
{
	int mask;
	unsigned long *ADDR = (unsigned long *)addr;

	ADDR += nr >> 5;
	mask = 1 << (nr & 31);
	*ADDR ^= mask;
}

static __inline__ int test_and_set_bit(int nr, void *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	retval = (mask & *a) != 0;
	*a |= mask;
	local_irq_restore(flags);

	return retval;
}

static __inline__ int __test_and_set_bit(int nr, volatile unsigned long *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a |= mask;
	return retval;
}

static __inline__ int test_and_clear_bit(int nr, volatile unsigned long *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	retval = (mask & *a) != 0;
	*a &= ~mask;
	local_irq_restore(flags);

	return retval;
}

static __inline__ int __test_and_clear_bit(int nr, volatile unsigned long *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a &= ~mask;
	return retval;
}

static __inline__ int test_and_change_bit(int nr, volatile unsigned long *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	retval = (mask & *a) != 0;
	*a ^= mask;
	local_irq_restore(flags);
	return retval;
}

static __inline__ int __test_and_change_bit(int nr,
					    volatile unsigned long *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a ^= mask;
	return retval;
}

/*
 * This routine doesn't need to be atomic.
 */
static __inline__ int __constant_test_bit(int nr, const void *addr)
{
	return ((1UL << (nr & 31)) &
		(((const volatile unsigned int *)addr)[nr >> 5])) != 0;
}

static __inline__ int __test_bit(int nr, const void *addr)
{
	int *a = (int *)addr;
	int mask;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	return ((mask & *a) != 0);
}

#define test_bit(nr,addr) \
(__builtin_constant_p(nr) ? \
 __constant_test_bit((nr),(addr)) : \
 __test_bit((nr),(addr)))


#if 1

/*
 * Return the bit position (0..63) of the most significant 1 bit in a word
 * Returns -1 if no 1 bit exists
 */
static inline unsigned long __fls(unsigned long x)
{
	int lz;

	if (sizeof(x) == 4) {
		__asm__(
		"	.set	push					\n"
		"	.set	mips32					\n"
		"	clz	%0, %1					\n"
		"	.set	pop					\n"
		: "=r" (lz)
		: "r" (x));

		return 31 - lz;
	}

	BUG_ON(sizeof(x) != 8);

	__asm__(
	"	.set	push						\n"
	"	.set	mips64						\n"
	"	dclz	%0, %1						\n"
	"	.set	pop						\n"
	: "=r" (lz)
	: "r" (x));

	return 63 - lz;
}

/*
 * __ffs - find first bit in word.
 * @word: The word to search
 *
 * Returns 0..SZLONG-1
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline unsigned long __ffs(unsigned long word)
{
	return __fls(word & -word);
}

/*
 * fls - find last bit set.
 * @word: The word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline int fls(int word)
{
	__asm__("clz %0, %1" : "=r" (word) : "r" (word));

	return 32 - word;
}

#if defined(CONFIG_64BIT) && defined(CONFIG_CPU_MIPS64)
static inline int fls64(__u64 word)
{
	__asm__("dclz %0, %1" : "=r" (word) : "r" (word));

	return 64 - word;
}
#else
#include <asm-generic/bitops/fls64.h>
#endif

/*
 * ffs - find first bit set.
 * @word: The word to search
 *
 * This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */
static inline int ffs(int word)
{
	if (!word)
		return 0;

	return fls(word & -word);
}

#else

#include <asm-generic/bitops/__ffs.h>
#include <asm-generic/bitops/ffs.h>
#include <asm-generic/bitops/fls.h>
#include <asm-generic/bitops/fls64.h>

#endif

#include <asm-generic/bitops/ffz.h>
#include <asm-generic/bitops/find.h>

#endif				/* _BLACKFIN_BITOPS_H */
