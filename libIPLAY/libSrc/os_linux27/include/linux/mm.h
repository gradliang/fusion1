#ifndef _LINUX_MM_H
#define _LINUX_MM_H

#include <linux/errno.h>

#ifdef __KERNEL__

#include <linux/gfp.h>
#include <linux/list.h>
#include <linux/mmzone.h>
#include <linux/rbtree.h>
#include <linux/prio_tree.h>
#include <linux/debug_locks.h>
#include <linux/mm_types.h>

struct mempolicy;
struct anon_vma;
struct file_ra_state;
struct user_struct;
struct writeback_control;


const char * arch_vma_name(struct vm_area_struct *vma);
void print_vma_addr(char *prefix, unsigned long rip);

struct page *sparse_mem_map_populate(unsigned long pnum, int nid);
pgd_t *vmemmap_pgd_populate(unsigned long addr, int node);
void *vmemmap_alloc_block(unsigned long size, int node);
int vmemmap_populate_basepages(struct page *start_page,
						unsigned long pages, int node);
int vmemmap_populate(struct page *start_page, unsigned long pages, int node);
void vmemmap_populate_print_last(void);

#endif /* __KERNEL__ */
#endif /* _LINUX_MM_H */
