/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Ralf Baechle
 */
#ifndef _ASM_PGTABLE_H
#define _ASM_PGTABLE_H

#ifdef CONFIG_32BIT
#include <asm/pgtable-32.h>
#endif
#ifdef CONFIG_64BIT
#include <asm/pgtable-64.h>
#endif

#include <asm/io.h>
#include <asm/pgtable-bits.h>

struct mm_struct;
struct vm_area_struct;

#define PAGE_NONE	__pgprot(_PAGE_PRESENT | _CACHE_CACHABLE_NONCOHERENT)
#define PAGE_SHARED	__pgprot(_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | \
				 _page_cachable_default)
#define PAGE_COPY	__pgprot(_PAGE_PRESENT | _PAGE_READ | \
				 _page_cachable_default)
#define PAGE_READONLY	__pgprot(_PAGE_PRESENT | _PAGE_READ | \
				 _page_cachable_default)
#define PAGE_KERNEL	__pgprot(_PAGE_PRESENT | __READABLE | __WRITEABLE | \
				 _PAGE_GLOBAL | _page_cachable_default)
#define PAGE_USERIO	__pgprot(_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | \
				 _page_cachable_default)
#define PAGE_KERNEL_UNCACHED __pgprot(_PAGE_PRESENT | __READABLE | \
			__WRITEABLE | _PAGE_GLOBAL | _CACHE_UNCACHED)

/*
 * MIPS can't do page protection for execute, and considers that the same like
 * read. Also, write permissions imply read permissions. This is the closest
 * we can get by reasonable means..
 */

/*
 * Dummy values to fill the table in mmap.c
 * The real values will be generated at runtime
 */
#define __P000 __pgprot(0)
#define __P001 __pgprot(0)
#define __P010 __pgprot(0)
#define __P011 __pgprot(0)
#define __P100 __pgprot(0)
#define __P101 __pgprot(0)
#define __P110 __pgprot(0)
#define __P111 __pgprot(0)

#define __S000 __pgprot(0)
#define __S001 __pgprot(0)
#define __S010 __pgprot(0)
#define __S011 __pgprot(0)
#define __S100 __pgprot(0)
#define __S101 __pgprot(0)
#define __S110 __pgprot(0)
#define __S111 __pgprot(0)

extern unsigned long _page_cachable_default;

/*
 * ZERO_PAGE is a global shared page that is always zero; used
 * for zero-mapped memory areas etc..
 */

extern unsigned long empty_zero_page;
extern unsigned long zero_page_mask;

#define ZERO_PAGE(vaddr) \
	(virt_to_page((void *)(empty_zero_page + (((unsigned long)(vaddr)) & zero_page_mask))))

extern void paging_init(void);

/*
 * Conversion functions: convert a page and protection to a page entry,
 * and a page entry and page directory to the page they refer to.
 */
#define pmd_phys(pmd)		virt_to_phys((void *)pmd_val(pmd))
#define pmd_page(pmd)		(pfn_to_page(pmd_phys(pmd) >> PAGE_SHIFT))
#define pmd_page_vaddr(pmd)	pmd_val(pmd)

#if defined(CONFIG_64BIT_PHYS_ADDR) && defined(CONFIG_CPU_MIPS32)

#define pte_none(pte)		(!(((pte).pte_low | (pte).pte_high) & ~_PAGE_GLOBAL))
#define pte_present(pte)	((pte).pte_low & _PAGE_PRESENT)

static inline void set_pte(pte_t *ptep, pte_t pte)
{
	ptep->pte_high = pte.pte_high;
	smp_wmb();
	ptep->pte_low = pte.pte_low;
	//printk("pte_high %x pte_low %x\n", ptep->pte_high, ptep->pte_low);

	if (pte.pte_low & _PAGE_GLOBAL) {
		pte_t *buddy = ptep_buddy(ptep);
		/*
		 * Make sure the buddy is global too (if it's !none,
		 * it better already be global)
		 */
		if (pte_none(*buddy)) {
			buddy->pte_low  |= _PAGE_GLOBAL;
			buddy->pte_high |= _PAGE_GLOBAL;
		}
	}
}
#define set_pte_at(mm, addr, ptep, pteval) set_pte(ptep, pteval)

static inline void pte_clear(struct mm_struct *mm, unsigned long addr, pte_t *ptep)
{
	pte_t null = __pte(0);

	/* Preserve global status for the pair */
	if (ptep_buddy(ptep)->pte_low & _PAGE_GLOBAL)
		null.pte_low = null.pte_high = _PAGE_GLOBAL;

	set_pte_at(mm, addr, ptep, null);
}
#else

#define pte_none(pte)		(!(pte_val(pte) & ~_PAGE_GLOBAL))
#define pte_present(pte)	(pte_val(pte) & _PAGE_PRESENT)

#define set_pte_at(mm, addr, ptep, pteval) set_pte(ptep, pteval)

#endif

/*
 * (pmds are folded into puds so this doesn't get actually called,
 * but the define is needed for a generic inline function.)
 */
#define set_pmd(pmdptr, pmdval) do { *(pmdptr) = (pmdval); } while(0)

#ifdef CONFIG_64BIT
/*
 * (puds are folded into pgds so this doesn't get actually called,
 * but the define is needed for a generic inline function.)
 */
#define set_pud(pudptr, pudval) do { *(pudptr) = (pudval); } while(0)
#endif

#define PGD_T_LOG2	(__builtin_ffs(sizeof(pgd_t)) - 1)
#define PMD_T_LOG2	(__builtin_ffs(sizeof(pmd_t)) - 1)
#define PTE_T_LOG2	(__builtin_ffs(sizeof(pte_t)) - 1)

/*
 * We used to declare this array with size but gcc 3.3 and older are not able
 * to find that this expression is a constant, so the size is dropped.
 */
extern pgd_t swapper_pg_dir[];

/*
 * The following only work if pte_present() is true.
 * Undefined behaviour if not..
 */
#if defined(CONFIG_64BIT_PHYS_ADDR) && defined(CONFIG_CPU_MIPS32)
static inline int pte_write(pte_t pte)	{ return pte.pte_low & _PAGE_WRITE; }
static inline int pte_dirty(pte_t pte)	{ return pte.pte_low & _PAGE_MODIFIED; }
static inline int pte_young(pte_t pte)	{ return pte.pte_low & _PAGE_ACCESSED; }
static inline int pte_file(pte_t pte)	{ return pte.pte_low & _PAGE_FILE; }

static inline pte_t pte_wrprotect(pte_t pte)
{
	pte.pte_low  &= ~(_PAGE_WRITE | _PAGE_SILENT_WRITE);
	pte.pte_high &= ~_PAGE_SILENT_WRITE;
	return pte;
}

static inline pte_t pte_mkclean(pte_t pte)
{
	pte.pte_low  &= ~(_PAGE_MODIFIED | _PAGE_SILENT_WRITE);
	pte.pte_high &= ~_PAGE_SILENT_WRITE;
	return pte;
}

static inline pte_t pte_mkold(pte_t pte)
{
	pte.pte_low  &= ~(_PAGE_ACCESSED | _PAGE_SILENT_READ);
	pte.pte_high &= ~_PAGE_SILENT_READ;
	return pte;
}

static inline pte_t pte_mkwrite(pte_t pte)
{
	pte.pte_low |= _PAGE_WRITE;
	if (pte.pte_low & _PAGE_MODIFIED) {
		pte.pte_low  |= _PAGE_SILENT_WRITE;
		pte.pte_high |= _PAGE_SILENT_WRITE;
	}
	return pte;
}

static inline pte_t pte_mkdirty(pte_t pte)
{
	pte.pte_low |= _PAGE_MODIFIED;
	if (pte.pte_low & _PAGE_WRITE) {
		pte.pte_low  |= _PAGE_SILENT_WRITE;
		pte.pte_high |= _PAGE_SILENT_WRITE;
	}
	return pte;
}

static inline pte_t pte_mkyoung(pte_t pte)
{
	pte.pte_low |= _PAGE_ACCESSED;
	if (pte.pte_low & _PAGE_READ) {
		pte.pte_low  |= _PAGE_SILENT_READ;
		pte.pte_high |= _PAGE_SILENT_READ;
	}
	return pte;
}
#else
#endif


#include <asm-generic/pgtable.h>

/*
 * We provide our own get_unmapped area to cope with the virtual aliasing
 * constraints placed on us by the cache architecture.
 */
#define HAVE_ARCH_UNMAPPED_AREA

/*
 * No page table caches to initialise
 */
#define pgtable_cache_init()	do { } while (0)

#endif /* _ASM_PGTABLE_H */
