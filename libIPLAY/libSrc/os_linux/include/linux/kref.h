
#ifndef _KREF_H_
#define _KREF_H_

struct kref {
	atomic_t refcount;
};
#endif

