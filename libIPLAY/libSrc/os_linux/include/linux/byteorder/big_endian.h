#ifndef _LINUX_BYTEORDER_BIG_ENDIAN_H
#define _LINUX_BYTEORDER_BIG_ENDIAN_H

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 4321
#endif
#ifndef __BIG_ENDIAN_BITFIELD
#define __BIG_ENDIAN_BITFIELD
#endif

#include <linux/types.h>
#include <linux/byteorder/swab.h>

#define __constant_htonl(x) ((__force __be32)(__u32)(x))
#define __constant_ntohl(x) ((__force __u32)(__be32)(x))
#define __constant_htons(x) ((__force __be16)(__u16)(x))
#define __constant_ntohs(x) ((__force __u16)(__be16)(x))
#define __constant_cpu_to_le64(x) ((__force __le64)___constant_swab64((x)))
#define __constant_le64_to_cpu(x) ___constant_swab64((__force __u64)(__le64)(x))
#define __constant_cpu_to_le32(x) ((__force __le32)___constant_swab32((x)))
#define __constant_le32_to_cpu(x) ___constant_swab32((__force __u32)(__le32)(x))
#define __constant_cpu_to_le16(x) ((__force __le16)___constant_swab16((x)))
#define __constant_le16_to_cpu(x) ___constant_swab16((__force __u16)(__le16)(x))
#define __constant_cpu_to_be64(x) ((__force __be64)(__u64)(x))
#define __constant_be64_to_cpu(x) ((__force __u64)(__be64)(x))
#define __constant_cpu_to_be32(x) ((__force __be32)(__u32)(x))
#define __constant_be32_to_cpu(x) ((__force __u32)(__be32)(x))
#define __constant_cpu_to_be16(x) ((__force __be16)(__u16)(x))
#define __constant_be16_to_cpu(x) ((__force __u16)(__be16)(x))
#define __cpu_to_le64(x) ((__force __le64)__swab64((x)))
#define __le64_to_cpu(x) __swab64((__force __u64)(__le64)(x))
#define __cpu_to_le32(x) ((__force __le32)__swab32((x)))
#define __le32_to_cpu(x) __swab32((__force __u32)(__le32)(x))
#define __cpu_to_le16(x) ((__force __le16)__swab16((x)))
#define __le16_to_cpu(x) __swab16((__force __u16)(__le16)(x))
#define __cpu_to_be64(x) ((__force __be64)(__u64)(x))
#define __be64_to_cpu(x) ((__force __u64)(__be64)(x))
#define __cpu_to_be32(x) ((__force __be32)(__u32)(x))
#define __be32_to_cpu(x) ((__force __u32)(__be32)(x))
#define __cpu_to_be16(x) ((__force __be16)(__u16)(x))
#define __be16_to_cpu(x) ((__force __u16)(__be16)(x))

#define __cpu_to_le64p(x) \
({ \
	__u64 __x = (*x); \
	((__u64)( \
		(__u64)(((__u64)(__x) & (__u64)0x00000000000000ffULL) << 56) | \
		(__u64)(((__u64)(__x) & (__u64)0x000000000000ff00ULL) << 40) | \
		(__u64)(((__u64)(__x) & (__u64)0x0000000000ff0000ULL) << 24) | \
		(__u64)(((__u64)(__x) & (__u64)0x00000000ff000000ULL) <<  8) | \
	  (__u64)(((__u64)(__x) & (__u64)0x000000ff00000000ULL) >>  8) | \
		(__u64)(((__u64)(__x) & (__u64)0x0000ff0000000000ULL) >> 24) | \
		(__u64)(((__u64)(__x) & (__u64)0x00ff000000000000ULL) >> 40) | \
		(__u64)(((__u64)(__x) & (__u64)0xff00000000000000ULL) >> 56) )); \
})
#define __le64_to_cpup(x) \
({ \
	__u64 __x = *(x); \
	((__u64)( \
		(__u64)(((__u64)(__x) & (__u64)0x00000000000000ffULL) << 56) | \
		(__u64)(((__u64)(__x) & (__u64)0x000000000000ff00ULL) << 40) | \
		(__u64)(((__u64)(__x) & (__u64)0x0000000000ff0000ULL) << 24) | \
		(__u64)(((__u64)(__x) & (__u64)0x00000000ff000000ULL) <<  8) | \
		(__u64)(((__u64)(__x) & (__u64)0x0000ff0000000000ULL) >> 24) | \
		(__u64)(((__u64)(__x) & (__u64)0x00ff000000000000ULL) >> 40) | \
		(__u64)(((__u64)(__x) & (__u64)0xff00000000000000ULL) >> 56) )); \
})
#define __cpu_to_le32p(x)  \
({ \
	__u32 __x = (*x); \
	((__u32)( \
		(((__u32)(__x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(__x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(__x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(__x) & (__u32)0xff000000UL) >> 24) )); \
})
#define __le32_to_cpup(x)  \
({ \
	__u32 __x = *(x); \
	((__u32)( \
		(((__u32)(__x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(__x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(__x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(__x) & (__u32)0xff000000UL) >> 24) )); \
})
#define __cpu_to_le16p(x) \
({ \
	__u16 __x = *(x); \
	((__u16)( \
		(((__u16)(__x) & (__u16)0x00ffU) << 8) | \
		(((__u16)(__x) & (__u16)0xff00U) >> 8) )); \
})
#define __le16_to_cpup(x) \
({ \
	__u16 __x = *(x); \
	((__u16)( \
		(((__u16)(__x) & (__u16)0x00ffU) << 8) | \
		(((__u16)(__x) & (__u16)0xff00U) >> 8) )); \
})
static inline __be64 __cpu_to_be64p(const __u64 *p)
{
	return (__force __be64)*p;
}
static inline __u64 __be64_to_cpup(const __be64 *p)
{
	return (__force __u64)*p;
}
static inline __be32 __cpu_to_be32p(const __u32 *p)
{
	return (__force __be32)*p;
}
static inline __u32 __be32_to_cpup(const __be32 *p)
{
	return (__force __u32)*p;
}
static inline __be16 __cpu_to_be16p(const __u16 *p)
{
	return (__force __be16)*p;
}
static inline __u16 __be16_to_cpup(const __be16 *p)
{
	return (__force __u16)*p;
}
#define __cpu_to_le64s(x) __swab64s((x))
#define __le64_to_cpus(x) __swab64s((x))
#define __cpu_to_le32s(x) __swab32s((x))
#define __le32_to_cpus(x) __swab32s((x))
#define __cpu_to_le16s(x) __swab16s((x))
#define __le16_to_cpus(x) __swab16s((x))
#define __cpu_to_be64s(x) do { (void)(x); } while (0)
#define __be64_to_cpus(x) do { (void)(x); } while (0)
#define __cpu_to_be32s(x) do { (void)(x); } while (0)
#define __be32_to_cpus(x) do { (void)(x); } while (0)
#define __cpu_to_be16s(x) do { (void)(x); } while (0)
#define __be16_to_cpus(x) do { (void)(x); } while (0)

#ifdef __KERNEL__
#include <linux/byteorder/generic.h>
#endif

#endif /* _LINUX_BYTEORDER_BIG_ENDIAN_H */
