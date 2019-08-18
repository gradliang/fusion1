/*
 * mm-implicit.c -  Simple allocator based on implicit free lists,
 *                  first fit placement, and boundary tag coalescing.
 *
 * Each block has header and footer of the form:
 *
 *      31                     3  2  1  0
 *      -----------------------------------
 *     | s  s  s  s  ... s  s  s  0  0  a/f
 *      -----------------------------------
 *
 * where s are the meaningful size bits and a/f is set
 * iff the block is allocated. The list has the following form:
 *
 * begin                                                          end
 * heap                                                           heap 
 *  -----------------------------------------------------------------  
 * |  pad   | hdr(8:a) | ftr(8:a) | zero or more usr blks | hdr(8:a) |
 *  -----------------------------------------------------------------
 *          |       prologue      |                       | epilogue |
 *          |         block       |                       | block    |
 *
 * The allocated prologue and epilogue blocks are overhead that
 * eliminate edge conditions during coalescing.
 */
#include <stdio.h>

#define LOCAL_DEBUG_ENABLE 1
#include "global612.h"
#include "mpTrace.h"
#include <linux/kernel.h>
#include <linux/list.h>
#include "net_packet.h"
#include "ndebug.h"

#ifndef LINUX
#define printf  mpDebugPrint//DPRINTF
#endif

/*
 * If NEXT_FIT defined use next fit search, else use first fit search
 */
#define NEXT_FITx


/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE       4       /* word size (bytes) */ 
#define DSIZE       8       /* doubleword size (bytes) */
#define CHUNKSIZE  (1<<18)  /* initial heap size (bytes) */
#define OVERHEAD    8       /* overhead of header and footer (bytes) */

#ifndef MAX
#define MAX(x, y) ((x) > (y)? (x) : (y)) 
#endif

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(size_t *)(p))
#define PUT(p, val)  (*(size_t *)(p) = (val)) 

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE) 
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
/* $end mallocmacros */

/* Global variables */
static char mm_memory[4*WSIZE+CHUNKSIZE];
static char *heap_listp;  /* pointer to first block */ 
#ifdef NEXT_FIT
static char *rover;       /* next fit rover */
#endif

/* function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkblock(void *bp);

extern int HeapSemid;
#ifdef HAVE_SMB
#define LARGE_MEMORY_SIZE   (64 * 1024)
struct mm_list_struct {
    struct list_head list;
    int cnt;
} mmlarge_list;
struct mm_struct {
    struct list_head entry;
    void *ptr;
    size_t size;
};
void *ext_mem_malloc(DWORD size);
#endif
/*
 * mm_init - Initialize the memory manager
 */
/* $begin mminit */
int mm_init(void)
{
#if 0
    /* create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == NULL)
        return -1;
    PUT(heap_listp, 0);                        /* alignment padding */
    PUT(heap_listp+WSIZE, PACK(OVERHEAD, 1));  /* prologue header */
    PUT(heap_listp+DSIZE, PACK(OVERHEAD, 1));  /* prologue footer */
    PUT(heap_listp+WSIZE+DSIZE, PACK(0, 1));   /* epilogue header */
    heap_listp += DSIZE;

#ifdef NEXT_FIT
    rover = heap_listp;
#endif

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
#else
    heap_listp = mm_memory;
    PUT(heap_listp, 0);                        /* alignment padding */
    PUT(heap_listp+WSIZE, PACK(OVERHEAD, 1));  /* prologue header */
    PUT(heap_listp+DSIZE, PACK(OVERHEAD, 1));  /* prologue footer */
    PUT(heap_listp+WSIZE+DSIZE, PACK(0, 1));   /* epilogue header */
    heap_listp += DSIZE;

#ifdef NEXT_FIT
    rover = heap_listp;
#endif

    char *bp = heap_listp + DSIZE;
    size_t size = CHUNKSIZE;
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */

    /* Coalesce if the previous block was free */
    coalesce(bp);
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        printblock(bp);
    }

    if (!HeapSemid)
    {
		HeapSemid = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(HeapSemid > 0);
    }

#endif
#ifdef HAVE_SMB
    INIT_LIST_HEAD(&mmlarge_list.list);
#endif
    return 0;
}
/* $end mminit */

/*
 * mm_malloc - Allocate a block with at least size bytes of payload
 */
/* $begin mmmalloc */
/**
 * @ingroup NET_HEAP
 * @brief Allocate a block with at least @p size bytes of payload
 * 
*/
void *mm_malloc(size_t size)
{
    size_t asize;      /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char *bp;     
    short i;

#ifdef HAVE_SMB
    if (size >= LARGE_MEMORY_SIZE)
    {
        struct mm_struct *lm = mm_zalloc(sizeof(struct mm_struct));
        MP_ASSERT(lm);
        if (!lm)
            return NULL;
        bp = ext_mem_malloc(size);
        if (bp)
        {
            lm->size = size;
            lm->ptr = bp;
            INIT_LIST_HEAD(&lm->entry);

            SemaphoreWait(HeapSemid);	
            list_add_tail(&lm->entry, &mmlarge_list.list);
            mmlarge_list.cnt++;
            SemaphoreRelease(HeapSemid);
            printf("%s: cnt=%d\n", __FUNCTION__, mmlarge_list.cnt);
        }
        else
        {
            mm_free(lm);
            MP_ASSERT(bp);
        }
        return bp;
    }
#endif
    /* Ignore spurious requests */
    if (size <= 0)
    {
        MP_ASSERT(0);
        return NULL;
    }

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = DSIZE + OVERHEAD;
    else
        asize = DSIZE * ((size + (OVERHEAD) + (DSIZE-1)) / DSIZE);
   
	SemaphoreWait(HeapSemid);	
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
		SemaphoreRelease(HeapSemid);
        return bp;
    }
    SemaphoreRelease(HeapSemid);

#if 0
    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
#else
    MP_ASSERT(0);
    return NULL;
#endif
}
/* $end mmmalloc */

/*
 * mm_free - Free a block
 */
/* $begin mmfree */
void NetPacketDump(unsigned long address, unsigned long size);
/**
 * @ingroup NET_HEAP
 * @brief Free a block of memory.
 * 
 * @retval None
*/
void mm_free(const void *bp)
{
    if (bp == NULL)
        return;
#ifdef HAVE_SMB
    if (GET_SIZE(HDRP(bp)) >= LARGE_MEMORY_SIZE)
    {
        struct list_head *ptr;
        struct mm_struct *mm;
        SemaphoreWait(HeapSemid);	
        list_for_each(ptr, &mmlarge_list.list) {
            mm = list_entry(ptr, struct mm_struct, entry);
            if (mm->ptr == bp)
                break;
        }
        SemaphoreRelease(HeapSemid);
        if (ptr == &mmlarge_list.list)
        {
            MP_ASSERT(0);
        }
        else
        {
            SemaphoreWait(HeapSemid);	
            list_del_init(ptr);
            MP_ASSERT(mmlarge_list.cnt > 0);
            mmlarge_list.cnt--;
            SemaphoreRelease(HeapSemid);
            mm_free(mm);
            ext_mem_free(bp);
            printf("%s: cnt=%d\n", __FUNCTION__, mmlarge_list.cnt);
        }
        return;
    }
#endif
    if ((unsigned long)bp < (unsigned long)mm_memory ||
        (unsigned long)bp > (unsigned long)(mm_memory + sizeof mm_memory) )
    {
        MP_DEBUG("mm_free: bp=%x s=%x,e=%x", bp, (long)mm_memory, 
                (long)(mm_memory + sizeof mm_memory));  
        NetPacketDump(NET_PACKET_ETHER(bp), 64);
        MP_ASSERT(0);
    }

    size_t size = GET_SIZE(HDRP(bp));

	SemaphoreWait(HeapSemid);	
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
    SemaphoreRelease(HeapSemid);
}

/* $end mmfree */

/*
 * mm_realloc - naive implementation of mm_realloc
 */
/**
 * @ingroup NET_HEAP
 * @brief Reallocate a memory block.
 * 
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *newp;
    size_t copySize;

    if (size == 0)
    {
        if (ptr)
            mm_free(ptr);
        return NULL;
    }
    if ((newp = mm_malloc(size)) == NULL) {
        printf("ERROR: mm_malloc failed in mm_realloc\n");
        MP_ASSERT(0);
        //exit(1);
        return NULL;
    }
    if (!ptr)
        return newp;
    copySize = GET_SIZE(HDRP(ptr));
    if (size < copySize)
      copySize = size;
		memcpy(newp, ptr, copySize);
    mm_free(ptr);
    return newp;
}

/*
 * mm_zalloc - naive implementation of zalloc
 */
/**
 * @ingroup NET_HEAP
 * @brief Allocate a memory block and zero out its content.
 * 
 */
void *mm_zalloc(size_t size)
{
    void *newp;
    size_t copySize;

    if (size == 0)
    {
        return NULL;
    }
    if ((newp = mm_malloc(size)) == NULL) {
        printf("ERROR: mm_malloc failed in mm_zalloc\n");
        MP_ASSERT(0);
        return NULL;
    }
    memset(newp, 0, size);
    return newp;
}

/*
 * mm_checkheap - Check the heap for consistency
 */
void mm_checkheap(int verbose)
{
    char *bp = heap_listp;

    if (verbose)
	printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");
    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose)
            printblock(bp);
        checkblock(bp);
    }
    
    if (verbose)
        printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");
}

/* The remaining routines are internal helper routines */

/*
 * extend_heap - Extend heap with free block and return its block =
pointer
 */
/* $begin mmextendheap */
#if 0
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((int)(bp = mem_sbrk(size)) < 0)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}
#endif
/* $end mmextendheap */

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
/* $begin mmplace */
/* $begin mmplace-proto */
static void place(void *bp, size_t asize)
/* $end mmplace-proto */
{
    size_t csize = GET_SIZE(HDRP(bp));  

    if ((csize - asize) >= (DSIZE + OVERHEAD)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
    }
    else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}
/* $end mmplace */

/*
 * find_fit - Find a fit for a block with asize bytes
 */
static void *find_fit(size_t asize)
{
#ifdef NEXT_FIT
    /* next fit search */
    char *oldrover = rover;

    /* search from the rover to the end of list */
    for ( ; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    /* search from start of list to old rover */
    for (rover =D heap_listp; rover < oldrover; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    return NULL;  /* no fit found */
#else
    /* first fit search */
    void *bp;
    int cnt = 0;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
    return NULL; /* no fit */
#endif
}

/*
 * coalesce - boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else {                                     /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

#ifdef NEXT_FIT
    /* Make sure the rover isn't pointing into the free block */
    /* that we just coalesced */
    if ((rover > (char *)bp) && (rover < NEXT_BLKP(bp)))
        rover = bp;
#endif

    return bp;
}


static void printblock(void *bp)
{
    size_t hsize, halloc, fsize, falloc;

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp)); 
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp)); 
   
    if (hsize == 0) {
	printf("%p: EOL\n", bp);
	return;
    }

    printf("%p: header: [%d:%c] footer: [%d:%c]\n", bp,
	   hsize, (halloc ? 'a' : 'f'),
	   fsize, (falloc ? 'a' : 'f'));
}

static void checkblock(void *bp)
{
    if ((size_t)bp % 8)
	printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
	printf("Error: header does not match footer\n");
}

