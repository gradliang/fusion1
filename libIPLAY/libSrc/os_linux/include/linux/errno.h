#ifndef __ERRNO_H__
#define __ERRNO_H__

typedef int error_t;

#include <sys/errno.h>

#ifdef __KERNEL__
/*
 * These should never be seen by user programs.  To return one of ERESTART*
 * codes, signal_pending() MUST be set.  Note that ptrace can observe these
 * at syscall exit tracing, but they will never be left for the debugged user
 * process to see.
 */
#define ERESTARTSYS	512
#define ENOTSUPP	524	/* Operation is not supported */
#endif

#endif /* !__ERRNO_H__ */
