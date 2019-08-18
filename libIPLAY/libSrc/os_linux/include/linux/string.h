#ifndef _LINUX_STRING_H_
#define _LINUX_STRING_H_

/* We don't want strings.h stuff being user by user stuff by accident */

#ifndef __KERNEL__
#include <string.h>
#else

#include <linux/compiler.h>	/* for inline */
#include <linux/types.h>	/* for size_t */
#include <linux/stddef.h>	/* for NULL */

extern char *strndup_user(const char __user *, long);

/*
 * Include machine specific inline routines
 */
#include <asm/string.h>

#endif

#endif /* _LINUX_STRING_H_ */
