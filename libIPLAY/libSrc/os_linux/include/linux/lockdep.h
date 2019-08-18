/*
 * Runtime locking correctness validator
 *
 *  Copyright (C) 2006,2007 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 *  Copyright (C) 2007 Red Hat, Inc., Peter Zijlstra <pzijlstr@redhat.com>
 *
 * see Documentation/lockdep-design.txt for more details.
 */
#ifndef __LINUX_LOCKDEP_H
#define __LINUX_LOCKDEP_H

struct task_struct;
struct lockdep_map;

/*
 * The class key takes no space if lockdep is disabled:
 */
struct lock_class_key { };

#endif /* __LINUX_LOCKDEP_H */
