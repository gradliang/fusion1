/*
 * include/linux/backing-dev.h
 *
 * low-level device information and state which is propagated up through
 * to high-level code.
 */

#ifndef _LINUX_BACKING_DEV_H
#define _LINUX_BACKING_DEV_H

//#include <linux/percpu_counter.h>
#include <linux/log2.h>
//#include <linux/proportions.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/atomic.h>

struct page;
struct device;
struct dentry;

/*
 * Bits in backing_dev_info.state
 */
enum bdi_state {
	BDI_pdflush,		/* A pdflush thread is working this device */
	BDI_write_congested,	/* The write queue is getting full */
	BDI_read_congested,	/* The read queue is getting full */
	BDI_unused,		/* Available bits start here */
};

typedef int (congested_fn)(void *, int);

enum bdi_stat_item {
	BDI_RECLAIMABLE,
	BDI_WRITEBACK,
	NR_BDI_STAT_ITEMS
};

#define BDI_STAT_BATCH (8*(1+ilog2(nr_cpu_ids)))

struct backing_dev_info {
	unsigned long ra_pages;	/* max readahead in PAGE_CACHE_SIZE units */
	unsigned long state;	/* Always use atomic bitops on this */
	unsigned int capabilities; /* Device capabilities */
	congested_fn *congested_fn; /* Function pointer if device is md/dm */
	void *congested_data;	/* Pointer to aux data for congested func */
	void (*unplug_io_fn)(struct backing_dev_info *, struct page *);
	void *unplug_io_data;

//	struct percpu_counter bdi_stat[NR_BDI_STAT_ITEMS];

//	struct prop_local_percpu completions;
	int dirty_exceeded;

	unsigned int min_ratio;
	unsigned int max_ratio, max_prop_frac;

	struct device *dev;

#ifdef CONFIG_DEBUG_FS
	struct dentry *debug_dir;
	struct dentry *debug_stats;
#endif
};

int bdi_init(struct backing_dev_info *bdi);
void bdi_destroy(struct backing_dev_info *bdi);

int bdi_register(struct backing_dev_info *bdi, struct device *parent,
		const char *fmt, ...);
int bdi_register_dev(struct backing_dev_info *bdi, dev_t dev);
void bdi_unregister(struct backing_dev_info *bdi);

static inline void __add_bdi_stat(struct backing_dev_info *bdi,
		enum bdi_stat_item item, s64 amount)
{
//	__percpu_counter_add(&bdi->bdi_stat[item], amount, BDI_STAT_BATCH);
}

static inline void __inc_bdi_stat(struct backing_dev_info *bdi,
		enum bdi_stat_item item)
{
//	__add_bdi_stat(bdi, item, 1);
}

static inline void inc_bdi_stat(struct backing_dev_info *bdi,
		enum bdi_stat_item item)
{
	unsigned long flags;

	local_irq_save(flags);
	__inc_bdi_stat(bdi, item);
	local_irq_restore(flags);
}

static inline void __dec_bdi_stat(struct backing_dev_info *bdi,
		enum bdi_stat_item item)
{
	__add_bdi_stat(bdi, item, -1);
}

static inline void dec_bdi_stat(struct backing_dev_info *bdi,
		enum bdi_stat_item item)
{
	unsigned long flags;

	local_irq_save(flags);
	__dec_bdi_stat(bdi, item);
	local_irq_restore(flags);
}


#endif		/* _LINUX_BACKING_DEV_H */
