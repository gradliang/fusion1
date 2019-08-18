#ifndef NET_COMPAT_H
#define NET_COMPAT_H


struct sock;

#if defined(CONFIG_COMPAT)

#include <linux/compat.h>

struct compat_msghdr {
	compat_uptr_t	msg_name;	/* void * */
	compat_int_t	msg_namelen;
	compat_uptr_t	msg_iov;	/* struct compat_iovec * */
	compat_size_t	msg_iovlen;
	compat_uptr_t	msg_control;	/* void * */
	compat_size_t	msg_controllen;
	compat_uint_t	msg_flags;
};

struct compat_cmsghdr {
	compat_size_t	cmsg_len;
	compat_int_t	cmsg_level;
	compat_int_t	cmsg_type;
};

extern int compat_sock_get_timestamp(struct sock *, struct timeval __user *);
extern int compat_sock_get_timestampns(struct sock *, struct timespec __user *);

#else /* defined(CONFIG_COMPAT) */
#define compat_msghdr	msghdr		/* to avoid compiler warnings */
#endif /* defined(CONFIG_COMPAT) */


#endif /* NET_COMPAT_H */
