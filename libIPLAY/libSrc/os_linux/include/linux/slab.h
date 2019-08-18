
#ifndef _LINUX_SLAB_H
#define	_LINUX_SLAB_H

#include <linux/gfp.h>
#include <linux/types.h>
#include "ndebug.h"

static inline void kfree(const void *objp)
{
    mm_free(objp);
}

static inline void *kmalloc(size_t size, gfp_t flags)
{
    return mm_malloc(size);
}
/**
 * kzalloc - allocate memory. The memory is set to zero.
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kzalloc(size_t size, gfp_t flags)
{
	return mm_zalloc(size);
}

/**
 * kcalloc - allocate memory for an array. The memory is set to zero.
 * @n: number of elements.
 * @size: element size.
 * @flags: Not used
 */
static inline void *kcalloc(size_t n, size_t size, gfp_t flags)
{
	return mm_zalloc(n * size);
}
#endif

