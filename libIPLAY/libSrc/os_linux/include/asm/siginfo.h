/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1998, 1999, 2001, 2003 Ralf Baechle
 * Copyright (C) 2000, 2001 Silicon Graphics, Inc.
 */
#ifndef _ASM_SIGINFO_H
#define _ASM_SIGINFO_H


#define __ARCH_SIGEV_PREAMBLE_SIZE (sizeof(long) + 2*sizeof(int))
#undef __ARCH_SI_TRAPNO	/* exception code needs to fill this ...  */

#define HAVE_ARCH_SIGINFO_T

/*
 * We duplicate the generic versions - <asm-generic/siginfo.h> is just borked
 * by design ...
 */
#define HAVE_ARCH_COPY_SIGINFO
struct siginfo;

/*
 * Careful to keep union _sifields from shifting ...
 */
#ifdef CONFIG_32BIT
#define __ARCH_SI_PREAMBLE_SIZE (3 * sizeof(int))
#endif
#ifdef CONFIG_64BIT
#define __ARCH_SI_PREAMBLE_SIZE (4 * sizeof(int))
#endif

#include <asm-generic/siginfo.h>


/*
 * si_code values
 * Again these have been choosen to be IRIX compatible.
 */
#undef SI_ASYNCIO
#undef SI_TIMER
#undef SI_MESGQ
#define SI_ASYNCIO	-2	/* sent by AIO completion */
#define SI_TIMER __SI_CODE(__SI_TIMER, -3) /* sent by timer expiration */
#define SI_MESGQ __SI_CODE(__SI_MESGQ, -4) /* sent by real time mesq state change */

#ifdef __KERNEL__

/*
 * Duplicated here because of <asm-generic/siginfo.h> braindamage ...
 */
#include <linux/string.h>


#endif

#endif /* _ASM_SIGINFO_H */
