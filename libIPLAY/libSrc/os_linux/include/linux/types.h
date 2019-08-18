#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#include <sys/types.h>                          /* for MPX */

#include <linux/compat-2.6.h>
#include <linux/posix_types.h>
#include <asm/types.h>

typedef __u32 __kernel_dev_t;

#ifndef _SYS_TYPES_H
typedef __kernel_fd_set		fd_set;
typedef __kernel_dev_t		dev_t;
typedef __kernel_off_t		off_t;
typedef __kernel_mode_t		mode_t;
typedef __kernel_pid_t		pid_t;
typedef __kernel_key_t		key_t;
#endif

typedef __kernel_suseconds_t	suseconds_t;

#ifdef __KERNEL__

#define BITS_TO_LONGS(bits) \
	(((bits)+BITS_PER_LONG-1)/BITS_PER_LONG)
#define DECLARE_BITMAP(name,bits) \
	unsigned long name[BITS_TO_LONGS(bits)]

typedef _Bool			bool;

#ifndef _SYS_TYPES_H
typedef __kernel_uid32_t	uid_t;
typedef __kernel_gid32_t	gid_t;
#endif

typedef u32 resource_size_t;

#else
#ifndef _SYS_TYPES_H
typedef __kernel_uid_t		uid_t;
typedef __kernel_gid_t		gid_t;
#endif
#endif /* __KERNEL__ */

#if defined(__GNUC__)
typedef __kernel_loff_t		loff_t;
#endif

/*
 * The following typedefs are also protected by individual ifdefs for
 * historical reasons:
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef __kernel_size_t		size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
#ifndef _SYS_TYPES_H
typedef __kernel_ssize_t	ssize_t;
#endif
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef __kernel_ptrdiff_t	ptrdiff_t;
#endif

#ifndef _TIME_T
#define _TIME_T
#ifndef _SYS_TYPES_H
typedef __kernel_time_t		time_t;
#endif
#endif

typedef unsigned int gfp_t;

#define __bitwise

//#include "os_defs.h"

/**
 * The type used for indexing onto a disc or disc partition.
 *
 * Linux always considers sectors to be 512 bytes long independently
 * of the devices real block size.
 */
#ifdef CONFIG_LBD
typedef u64 sector_t;
#else
typedef unsigned long sector_t;
#endif

/*
 * The type of the inode's block count.
 */
#ifdef CONFIG_LSF
typedef u64 blkcnt_t;
#else
typedef unsigned long blkcnt_t;
#endif

/*
 * The type of an index into the pagecache.  Use a #define so asm/types.h
 * can override it.
 */
#ifndef pgoff_t
#define pgoff_t unsigned long
#endif
#define __bitwise__
#define __bitwise
#if !defined(__TYPEDEF_H_) || defined(__KERNEL__) 
typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;
typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;
#endif

//#include "osdep_mpixel_service.h"
#include "typedef.h"

#endif

