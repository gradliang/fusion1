#ifndef _LINUX_POLL_H
#define _LINUX_POLL_H

#include <asm/poll.h>

#ifdef __KERNEL__

typedef struct poll_table_struct {
} poll_table;
#endif /* KERNEL */

#endif /* _LINUX_POLL_H */
