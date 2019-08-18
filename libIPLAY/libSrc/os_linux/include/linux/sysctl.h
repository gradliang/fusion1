/*
 * sysctl.h: General linux system control interface
 *
 * Begun 24 March 1995, Stephen Tweedie
 *
 ****************************************************************
 ****************************************************************
 **
 **  WARNING:
 **  The values in this file are exported to user space via 
 **  the sysctl() binary interface.  Do *NOT* change the
 **  numbering of any existing values here, and do not change
 **  any numbers within any one set of values.  If you have to
 **  redefine an existing interface, use a new number for it.
 **  The kernel will then return -ENOTDIR to any application using
 **  the old binary interface.
 **
 **  For new interfaces unless you really need a binary number
 **  please use CTL_UNNUMBERED.
 **
 ****************************************************************
 ****************************************************************
 */

#ifndef _LINUX_SYSCTL_H
#define _LINUX_SYSCTL_H

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/compiler.h>

struct file;
struct completion;

#define CTL_MAXNAME 10		/* how many path components do we allow in a
				   call to sysctl?   In other words, what is
				   the largest acceptable value for the nlen
				   member of a struct __sysctl_args to have? */


#endif /* _LINUX_SYSCTL_H */
