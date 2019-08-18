/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright 1994 - 2000, 2002 Ralf Baechle (ralf@gnu.org)
 * Copyright 2000 Silicon Graphics, Inc.
 */
#ifndef _ASM_PARAM_H
#define _ASM_PARAM_H

#ifdef __KERNEL__

#ifdef LINUX
# define HZ		CONFIG_HZ	/* Internal kernel timer frequency */
#endif
# define USER_HZ	100		/* .. some user interfaces are in "ticks" */
#ifdef LINUX
# define CLOCKS_PER_SEC	(USER_HZ)	/* like times() */
#endif
#endif

#ifndef HZ
#define HZ 250
#endif

#define EXEC_PAGESIZE	65536

#ifndef NOGROUP
#define NOGROUP		(-1)
#endif

#define MAXHOSTNAMELEN	64	/* max length of hostname */

#endif /* _ASM_PARAM_H */
