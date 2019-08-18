/*
 * default memory allocator for libavutil
 * Copyright (c) 2002 Fabrice Bellard
 *
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * default memory allocator for libavutil
 */

#include "config.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "avutil.h"
#include "intreadwrite.h"
#include "mem.h"

/* here we can use OS-dependent allocation functions */
#undef free
#undef malloc
#undef realloc

#ifdef MALLOC_PREFIX

#define malloc         AV_JOIN(MALLOC_PREFIX, malloc)
#define memalign       AV_JOIN(MALLOC_PREFIX, memalign)
#define posix_memalign AV_JOIN(MALLOC_PREFIX, posix_memalign)
#define realloc        AV_JOIN(MALLOC_PREFIX, realloc)
#define free           AV_JOIN(MALLOC_PREFIX, free)

void *malloc(size_t size);
void *memalign(size_t align, size_t size);
int   posix_memalign(void **ptr, size_t align, size_t size);
void *realloc(void *ptr, size_t size);
void  free(void *ptr);

#endif /* MALLOC_PREFIX */

/* You can redefine av_malloc and av_free in your project to use your
 * memory allocator. You do not need to suppress this file because the
 * linker will do it automatically. */

#ifdef LINUX
void *av_malloc(size_t size)
{
    void *ptr = NULL;
#if CONFIG_MEMALIGN_HACK
    long diff;
#endif

    /* let's disallow possible ambiguous cases */
    if (size > (INT_MAX - 32) || !size)
        return NULL;

#if CONFIG_MEMALIGN_HACK
    ptr = malloc(size + 32);
    if (!ptr)
        return ptr;
    diff              = ((-(long)ptr - 1) & 31) + 1;
    ptr               = (char *)ptr + diff;
    ((char *)ptr)[-1] = diff;
#elif HAVE_POSIX_MEMALIGN
    if (posix_memalign(&ptr, 32, size))
        ptr = NULL;
#elif HAVE_ALIGNED_MALLOC
    ptr = _aligned_malloc(size, 32);
#elif HAVE_MEMALIGN
    ptr = memalign(32, size);
    /* Why 64?
     * Indeed, we should align it:
     *   on  4 for 386
     *   on 16 for 486
     *   on 32 for 586, PPro - K6-III
     *   on 64 for K7 (maybe for P3 too).
     * Because L1 and L2 caches are aligned on those values.
     * But I don't want to code such logic here!
     */
    /* Why 32?
     * For AVX ASM. SSE / NEON needs only 16.
     * Why not larger? Because I did not see a difference in benchmarks ...
     */
    /* benchmarks with P3
     * memalign(64) + 1          3071, 3051, 3032
     * memalign(64) + 2          3051, 3032, 3041
     * memalign(64) + 4          2911, 2896, 2915
     * memalign(64) + 8          2545, 2554, 2550
     * memalign(64) + 16         2543, 2572, 2563
     * memalign(64) + 32         2546, 2545, 2571
     * memalign(64) + 64         2570, 2533, 2558
     *
     * BTW, malloc seems to do 8-byte alignment by default here.
     */
#else
#ifdef LINUX
    ptr = malloc(size);
#else
    ptr = mm_malloc(size);
#endif
#endif
    return ptr;
}
#endif

#ifdef LINUX
void *av_realloc(void *ptr, size_t size)
{
#if CONFIG_MEMALIGN_HACK
    int diff;
#endif

    /* let's disallow possible ambiguous cases */
    if (size > (INT_MAX - 16))
        return NULL;

#if CONFIG_MEMALIGN_HACK
    //FIXME this isn't aligned correctly, though it probably isn't needed
    if (!ptr)
        return av_malloc(size);
    diff = ((char *)ptr)[-1];
    return (char *)realloc((char *)ptr - diff, size + diff) + diff;
#elif HAVE_ALIGNED_MALLOC
    return _aligned_realloc(ptr, size, 32);
#else
#ifdef LINUX
    return realloc(ptr, size);
#else
    return mm_realloc(ptr, size);
#endif
#endif
}

void av_free(void *ptr)
{
#if CONFIG_MEMALIGN_HACK
    if (ptr)
        free((char *)ptr - ((char *)ptr)[-1]);
#elif HAVE_ALIGNED_MALLOC
    _aligned_free(ptr);
#else
#ifdef LINUX
    free(ptr);
#else
    mm_free(ptr);
#endif
#endif
}
#endif

void av_freep(void *arg)
{
    void **ptr = (void **)arg;
    av_free(*ptr);
    *ptr = NULL;
}

#ifdef LINUX
void *av_mallocz(size_t size)
{
    void *ptr = av_malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}
#else
#endif

char *av_strdup(const char *s)
{
    char *ptr = NULL;
    if (s) {
        int len = strlen(s) + 1;
        ptr = av_malloc(len);
        if (ptr)
            memcpy(ptr, s, len);
    }
    return ptr;
}

void av_memcpy_backptr(uint8_t *dst, int back, int cnt)
{
    const uint8_t *src = &dst[-back];
    if (back == 1) {
        memset(dst, *src, cnt);
    } else {
        if (cnt >= 4) {
            AV_COPY16U(dst,     src);
            AV_COPY16U(dst + 2, src + 2);
            src += 4;
            dst += 4;
            cnt -= 4;
        }
        if (cnt >= 8) {
            AV_COPY16U(dst,     src);
            AV_COPY16U(dst + 2, src + 2);
            AV_COPY16U(dst + 4, src + 4);
            AV_COPY16U(dst + 6, src + 6);
            src += 8;
            dst += 8;
            cnt -= 8;
        }
        if (cnt > 0) {
            int blocklen = back;
            while (cnt > blocklen) {
                memcpy(dst, src, blocklen);
                dst       += blocklen;
                cnt       -= blocklen;
                blocklen <<= 1;
            }
            memcpy(dst, src, cnt);
        }
    }
}
