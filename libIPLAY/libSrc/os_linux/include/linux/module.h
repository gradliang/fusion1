#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H

#include <linux/list.h>
#include <linux/stat.h>
#include <linux/compiler.h>
#include <linux/kmod.h>
#include <linux/moduleparam.h>

#define MODULE_NAME_LEN (64 - sizeof(unsigned long))

#define MODULE_LICENSE(s)
#define MODULE_DESCRIPTION(s)
#define MODULE_AUTHOR(s)
#define MODULE_VERSION(s)
#define MODULE_PARM_DESC(_parm, desc)
#define MODULE_DEVICE_TABLE(a,b)

struct module
{
	/* Unique handle for this module */
	char name[MODULE_NAME_LEN];

};

#define THIS_MODULE ((struct module *)0)
#define EXPORT_SYMBOL(sym)
#define EXPORT_SYMBOL_GPL(sym)
#define EXPORT_SYMBOL_GPL_FUTURE(sym)
#define EXPORT_UNUSED_SYMBOL(sym)
#define EXPORT_UNUSED_SYMBOL_GPL(sym)

static inline void __module_get(struct module *module)
{
}

static inline int try_module_get(struct module *module)
{
    return 1;
}

static inline void module_put(struct module *module)
{
}
#endif

