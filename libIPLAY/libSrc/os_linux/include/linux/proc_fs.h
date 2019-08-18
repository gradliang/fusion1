#ifndef _LINUX_PROC_FS_H
#define _LINUX_PROC_FS_H

#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/magic.h>
#include <asm/atomic.h>
typedef	int (read_proc_t)(char *page, char **start, off_t off,
			  int count, int *eof, void *data);
typedef	int (write_proc_t)(struct file *file, const char __user *buffer,
			   unsigned long count, void *data);
typedef int (get_info_t)(char *, char **, off_t, int);

struct proc_dir_entry {
	unsigned int low_ino;
	unsigned short namelen;
	const char *name;
	mode_t mode;
	nlink_t nlink;
	uid_t uid;
	gid_t gid;
	loff_t size;
	const struct inode_operations *proc_iops;
	/*
	 * NULL ->proc_fops means "PDE is going away RSN" or
	 * "PDE is just created". In either case, e.g. ->read_proc won't be
	 * called because it's too late or too early, respectively.
	 *
	 * If you're allocating ->proc_fops dynamically, save a pointer
	 * somewhere.
	 */
	const struct file_operations *proc_fops;
	get_info_t *get_info;
	struct module *owner;
	struct proc_dir_entry *next, *parent, *subdir;
	void *data;
	read_proc_t *read_proc;
	write_proc_t *write_proc;
	atomic_t count;		/* use count */
	int deleted;		/* delete flag */
	int pde_users;	/* number of callers into module in progress */
	spinlock_t pde_unload_lock; /* proc_fops checks and pde_users bumps */
	struct completion *pde_unload_completion;
};


extern struct proc_dir_entry *proc_net;

static inline struct proc_dir_entry *create_proc_entry(const char *name,
	mode_t mode, struct proc_dir_entry *parent) { return NULL; }

#define remove_proc_entry(name, parent) do {} while (0)

static inline struct proc_dir_entry *create_proc_read_entry(const char *name,
	mode_t mode, struct proc_dir_entry *base, 
	read_proc_t *read_proc, void * data) { return NULL; }

#endif

