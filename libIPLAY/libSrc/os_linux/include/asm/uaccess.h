
#ifndef _ASM_UACCESS_H
#define _ASM_UACCESS_H

#include <linux/kernel.h>
#include <linux/errno.h>

#define copy_to_user(to,from,size) (mmcp_memcpy(to,from,size), 0)

#define copy_from_user(to,from,size) (mmcp_memcpy(to,from,size), 0)

#define get_user(x,p) mmcp_memcpy(&(x), p, sizeof(*(p)))
#define put_user(x,p) mmcp_memcpy(p, &(x), sizeof(*(p)))

#define __range_ok(addr,size)	(0)

#define access_ok(type,addr,size)	(__range_ok(addr,size) == 0)

#endif /* _ASM_UACCESS_H */
