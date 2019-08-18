#ifndef _LINUX_BUG_H
#define _LINUX_BUG_H

#include <linux/module.h>
#include <asm/bug.h>

enum bug_trap_type {
	BUG_TRAP_TYPE_NONE = 0,
	BUG_TRAP_TYPE_WARN = 1,
	BUG_TRAP_TYPE_BUG = 2,
};

struct pt_regs;

#endif	/* _LINUX_BUG_H */
