#ifndef __LINUX_COMPILER_H
#define __LINUX_COMPILER_H

# define __user
# define __kernel
# define __safe
# define __force
# define __nocast

# include <linux/compiler-gcc3.h>

# define __iomem
# define __acquires(x)
# define __releases(x)

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#ifdef __KERNEL__

#ifndef __must_check
#define __must_check
#endif

#endif /* __KERNEL__ */

#endif

