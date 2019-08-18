/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
 *
* All rights reserved. Magic Pixel's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in
* deciphering, decoding, reverse engineering or in ay way altering the source
* code is strictly prohibited, unless the prior written consent of Magic
* Pixel is obtained.
 *
* Filename      : memory.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
/*
// Include section
*/
#define LOCAL_DEBUG_ENABLE 0

#include "mpTrace.h"

#include "utiltypedef.h"
#include "bitsdefine.h"
#include "flagdefine.h"
#include "system.h"
#include "mem.h"
#include "os.h"

#define ENABLE_KERNEL_MALLOC_TO_UNCACHE_REGION
//#define ENABLE_BREAKPOINT_WHEN_KERNEL_MALLOC_FAIL
// constant define for debug memory

#define DEBUG_MEM_ENABLE        EXT_MEMORY_LEAK_DEBUG
#define POOL_MEM_ENABLE         1

#if DEBUG_MEM_ENABLE

#include "dbmem.h"

#define MAX_DM_NUM              0x10    // number of chain can be created
#define MAX_CHAIN_NUM           0x100   // number of entry in dm chain
#define DM_ENTRY_SIZE           0x10    // the size of dm chain entry

typedef struct st_dm_entry ST_DM_ENTRY;

typedef struct st_dm_entry
{
    DWORD addr;
    char *file;
    int line;
    ST_DM_ENTRY *next;
};

typedef struct st_dm_header
{
    BYTE TaskId;
    BYTE reserved;
    WORD FreeEntryCount;
    DWORD AlloEntryCount;
    ST_DM_ENTRY *dm_free_chain;
    ST_DM_ENTRY *dm_allo_chain;
} ST_DM_HEADER;

ST_DM_HEADER dm_array[MAX_DM_NUM];
BYTE NullStr[] = "NULL";

#endif  //#if DEBUG_MEM_ENABLE


#if POOL_MEM_ENABLE

#define TYPE_64B            0
#define TYPE_256B           1

#define MAX_PO0L_ENTRY      128
#define POOL_HEADER_SIZE    136         //MAX_PO0L_ENTRY + 8

#define ENTRY_64B_SIZE      64
#define ENTRY_64BAHD_SIZE   80          //64+16
#define POOL_64B_SIZE       (ENTRY_64BAHD_SIZE * MAX_PO0L_ENTRY + POOL_HEADER_SIZE)

#define ENTRY_256B_SIZE     256
#define ENTRY_256BAHD_SIZE  272         //256+16
#define POOL_256B_SIZE      (ENTRY_256BAHD_SIZE * MAX_PO0L_ENTRY + POOL_HEADER_SIZE)

#define ENTRY_2KB_SIZE      2048
#define ENTRY_2KBAHD_SIZE   2064        //2048+16
#define POOL_2KB_SIZE       (ENTRY_2KBAHD_SIZE * MAX_PO0L_ENTRY + POOL_HEADER_SIZE)

#define NUM_OF_POOL         3
#define MAX_POOL_SIZE       2048        // define the maximun request size to use pool


typedef struct
{
    DWORD EntrySize;        // define the entry size
    DWORD EntryAddHD;       // define the size of pool entry + 16Byte ( for heap memory header)
    DWORD PoolContainSize;  // define the unit size of pool
} ST_POOL_INFO;


typedef struct st_pool ST_POOL_MEM;
struct st_pool
{
    DWORD freecount;
    ST_POOL_MEM *next;
    BYTE used[MAX_PO0L_ENTRY];          // not used : 0, used : taskId
};



// Note : The array must be arranged from small value to large value
static ST_POOL_INFO PoolInfoArray[NUM_OF_POOL] = {
    {ENTRY_64B_SIZE,    ENTRY_64BAHD_SIZE,  POOL_64B_SIZE},
    {ENTRY_256B_SIZE,   ENTRY_256BAHD_SIZE, POOL_256B_SIZE},
    {ENTRY_2KB_SIZE,    ENTRY_2KBAHD_SIZE,  POOL_2KB_SIZE}
};

static ST_POOL_MEM *PoolArray[NUM_OF_POOL];

#endif  //#if POOL_MEM_ENABLE

//Make sure the sizeof(struct mem) is 16
typedef struct
{
    WORD tag;
    BYTE used;
    BYTE TaskId;
    DWORD size;
    DWORD next, prev;
} ST_HEAP_MEM;

#define MIN_SPACE           64
#define SIZEOF_STRUCT_MEM   16

#define KERNEL_HEAP         0
#define MAIN_HEAP           1

// for kernel heap memory management
#pragma alignvar(4)
static BYTE *ker_ram;
static ST_HEAP_MEM *ker_ram_end;
static DWORD dwKerMemHeapSize;
static ST_HEAP_MEM *ker_lfree;          /* pointer to the lowest free block */
static WORD wKerTag;


// for main heap memory management
#pragma alignvar(4)
static BYTE *ram;
static ST_HEAP_MEM *ram_end;
static ST_HEAP_MEM *lfree;              /* pointer to the lowest free block */
static WORD wTag;


/*-----------------------------------------------------------------------------------*/
static void _mem_init_(BYTE mode)
{
    ST_HEAP_MEM *mem, *mem_end;
    DWORD HeapSize;

    if (SIZEOF_STRUCT_MEM != sizeof(ST_HEAP_MEM))
    {
        MP_ALERT("-E- SIZEOF_STRUCT_MEM define wrong value");
    }

    switch(mode)
    {
    case KERNEL_HEAP:
        wKerTag++;
        dwKerMemHeapSize = SystemGetMemSize(OS_BUF_MEM_ID) - SIZEOF_STRUCT_MEM;
        ker_ram = (BYTE *)SystemGetMemAddr(OS_BUF_MEM_ID);
        ker_ram_end = (ST_HEAP_MEM *) (ker_ram + dwKerMemHeapSize);
        ker_lfree = (ST_HEAP_MEM *) ker_ram;

        mem = (ST_HEAP_MEM *) ker_ram;
        mem_end = ker_ram_end;
        HeapSize = dwKerMemHeapSize;
        break;

    case MAIN_HEAP:
        wTag++;
        HeapSize = SystemGetMemSize(HEAP_BUF_MEM_ID) - SIZEOF_STRUCT_MEM;
        ram = (BYTE *)SystemGetMemAddr(HEAP_BUF_MEM_ID);
        ram_end = (ST_HEAP_MEM *) (ram + HeapSize);
        lfree = (ST_HEAP_MEM *) ram;

        mem = (ST_HEAP_MEM *) ram;
        mem_end = ram_end;
        break;

    default:
        MP_ALERT("Init wrong memory area");

        return;
    }

    memset((BYTE *) mem, 0, (HeapSize + SIZEOF_STRUCT_MEM));

    // init end of list
    mem_end->used = 1;
    mem_end->TaskId = 0;
    mem_end->next = (DWORD)mem;
    mem_end->prev = (DWORD)mem;
    mem_end->size = 0;

    // init head of list
    mem->used = 0;
    mem->next = (DWORD)mem_end;
    mem->prev = (DWORD)mem_end;
    mem->size = HeapSize - SIZEOF_STRUCT_MEM;

    MP_DEBUG("mem_init ram=0x%X, end=0x%X", (DWORD) mem, (DWORD) mem_end);
}



/*-----------------------------------------------------------------------------------*/
static void *_mem_malloc_(BYTE mode, DWORD size ,BYTE taskId)
{
    ST_HEAP_MEM *mem, *tmpmem;
    ST_HEAP_MEM *mem_start, *mem_end;
    WORD tag;

    if (size == 0)
    {
        MP_ALERT("--E-- %s size = 0", __FUNCTION__);

        return NULL;
    }

    // Expand the size of the allocated memory region so that we can
    // adjust for alignment.
    if (size & MEM_ALIGN_MASK)
    {
        size += MEM_ALIGNMENT - (size & MEM_ALIGN_MASK);
    }

    switch(mode)
    {
    case KERNEL_HEAP:
        if (ker_lfree >= ker_ram_end)
            ker_lfree = (ST_HEAP_MEM *) ker_ram;

        mem_start = ker_lfree;
        mem_end = ker_ram_end;
        tag = wKerTag;
        break;

    case MAIN_HEAP:
        if (lfree >= ram_end)
            lfree = (ST_HEAP_MEM *) ram;

        mem_start = lfree;
        mem_end = ram_end;
        tag = wTag;
        break;

    default:
        MP_ALERT("--E-- %s wrong memory area", __FUNCTION__);

        return NULL;
        break;
    }

    MP_DEBUG("mem_malloc mode %d", mode);
    MP_DEBUG("size %d, 0x%X", size, (DWORD) mem_start);

    // find suitable space
    tmpmem = NULL;

    for (mem = mem_start; (mem != NULL) && (mem < mem_end); mem = (ST_HEAP_MEM *) mem->next)
    {
        if ((mem->used == 0) && (mem->size >= size))
        {
            if ((tmpmem == NULL) || (tmpmem->size > mem->size))
                tmpmem = mem;
        }
    }

    if (tmpmem == NULL)
    {
        if (mode == KERNEL_HEAP)
        {
            MP_ALERT("--E-- mem_malloc Fail at KERNEL_HEAP by Task-%d!!", taskId);
        }
        else
        {
            MP_ALERT("--E-- mem_malloc Fail at MAIN_HEAP by Task-%d, need %d!!", taskId, size);
        }

        return NULL;
    }
    else
    {
        tmpmem->used = 1;
        tmpmem->tag = tag;
        tmpmem->TaskId = taskId;

        // Create new entry for rest of space more than MIN_SPACE, after alloc size from mem
        if (tmpmem->size > (size + SIZEOF_STRUCT_MEM + MIN_SPACE))
        {
            ST_HEAP_MEM *mem2 = (ST_HEAP_MEM *)((DWORD)tmpmem + SIZEOF_STRUCT_MEM + size);

            // Insert to list between mem and mem->next
            mem2->prev = (DWORD)tmpmem;
            mem2->next = tmpmem->next;
            mem2->size = tmpmem->size - (size + SIZEOF_STRUCT_MEM);
            mem2->used = 0;

            tmpmem->next = (DWORD)mem2;
            tmpmem->size = size;

            if (mem2->next != NULL)
            {
                ((ST_HEAP_MEM *) (mem2->next))->prev = (DWORD) mem2;
            }

            // If free point is the selected area, move to next
            if (mem_start == tmpmem)
            {
                if(mode == KERNEL_HEAP)
                    ker_lfree = mem2;
                else if(mode == MAIN_HEAP)
                    lfree = mem2;
            }
        }

        MP_DEBUG("mem_malloc at 0x%X", (DWORD) tmpmem);

        return (void *)((DWORD)tmpmem + SIZEOF_STRUCT_MEM);
    }
}



/*-----------------------------------------------------------------------------------*/
static void _mem_free_(BYTE mode, void *rmem)
{
    ST_HEAP_MEM *mem;
    ST_HEAP_MEM *free;
    WORD tag;

    MP_DEBUG("<< mem_free rmem 0x%X", rmem);

    rmem = (void *)(((DWORD) rmem) & ~BIT29);
    mem = (ST_HEAP_MEM *) ((DWORD)rmem - SIZEOF_STRUCT_MEM);

    if(mem->used == 0)
    {
        MP_ALERT("--E-- %s: Wrong address to be free !!!", __FUNCTION__);

        return;
    }

    switch(mode)
    {
    case KERNEL_HEAP:
        if (((DWORD) rmem < ((DWORD) ker_ram + SIZEOF_STRUCT_MEM)) ||
            ((DWORD) rmem >= (DWORD) ker_ram_end))
        {
            MP_ALERT("--E-- %s: Wrong KERNEL_HEAP address to be free !!!", __FUNCTION__);

            return;
        }

        tag = wKerTag;
        free = ker_lfree;
        break;

    case MAIN_HEAP:
        if (((DWORD) rmem < ((DWORD) ram + SIZEOF_STRUCT_MEM)) ||
            ((DWORD) rmem >= (DWORD) ram_end))
        {
            MP_ALERT("--E-- %s: Wrong MAIN_HEAP address to be free !!!", __FUNCTION__);

            return;
        }

        tag = wTag;
        free = lfree;
        break;

    default:
        MP_ALERT("--E-- %s: Wrong memory area !!!", __FUNCTION__);

        return;
        break;
    }

    if (mem->tag != tag)
    {
        MP_ALERT("--E-- %s: Wrong memory tag !!!", __FUNCTION__);

        return;
    }

    mem->used = 0;

    // if free space pointer is bigger than mem, set free space pointer to mem
    if (free > mem)
    {
        if(mode == KERNEL_HEAP)
            ker_lfree = mem;
        else //if(mode == MAIN_HEAP)
            lfree = mem;
    }

    ST_HEAP_MEM *nmem;

    // check chain link
    nmem = (ST_HEAP_MEM *)mem->next;

    if (nmem->prev != (DWORD) mem)
    {
        MP_ALERT("--E-- %s: Next link error !!!", __FUNCTION__);

        return;
    }

    nmem = (ST_HEAP_MEM *) mem->prev;

    if(nmem->next != (DWORD) mem)
    {
        MP_ALERT("--E-- %s: Prev link error !!!", __FUNCTION__);

        return;
    }

    // merge memory space
    /* plug hole forward - if next mem is empty, merge together */
    nmem = (ST_HEAP_MEM *) mem->next;

    if ((nmem != NULL) && (mem != nmem) && (nmem->used == 0))
    {
        mem->next = nmem->next;
        mem->size += nmem->size + SIZEOF_STRUCT_MEM;
        ((ST_HEAP_MEM *)nmem->next)->prev = (DWORD)mem;
    }

    /* plug hole backward - if previous mem is empty, merge together */
    nmem = (ST_HEAP_MEM *) mem->prev;

    if ((nmem != NULL) && (nmem != mem) && (nmem->used == 0))
    {
        if (free > nmem)
        {
            if (mode == KERNEL_HEAP)
                ker_lfree = nmem;
            else //if(mode == MAIN_HEAP)
                lfree = nmem;
        }

        nmem->next = mem->next;
        nmem->size += mem->size + SIZEOF_STRUCT_MEM;
        ((ST_HEAP_MEM *) mem->next)->prev = (DWORD) nmem;
    }
}


#if POOL_MEM_ENABLE
/*-----------------------------------------------------------------------------------*/
/*                          function for extend pool                                 */
/*-----------------------------------------------------------------------------------*/
static void *pool_alloc(BYTE PoolId)
{
    ST_POOL_MEM *tmppool;
    DWORD size;

    if (PoolId >= NUM_OF_POOL)
    {
        MP_ALERT("--E-- %s - PoolId exceed to NUM_OF_POOL !!!", __FUNCTION__);

        return NULL;
    }

    size = PoolInfoArray[PoolId].PoolContainSize;
    tmppool = (ST_POOL_MEM *) _mem_malloc_(MAIN_HEAP, size, 1);

    if (tmppool == NULL)
    {
        MP_ALERT("--E-- Out of memory for tmppool !!!");

        return NULL;
    }

    tmppool->freecount = MAX_PO0L_ENTRY;
    tmppool->next = NULL;

    for (size = 0; size < MAX_PO0L_ENTRY; size++)
        tmppool->used[size] = 0;

    return tmppool;
}



static void *pool_mem_malloc(BYTE PoolId, DWORD size)
{
    ST_POOL_MEM *nextpool, *tmppool, *CurPool;
    ST_HEAP_MEM *hp;
    DWORD i;
    BYTE TaskID;

    if (PoolId >= NUM_OF_POOL)
    {
        MP_ALERT("--E-- %s - PoolId exceed to NUM_OF_POOL !!!", __FUNCTION__);

        return NULL;
    }

    CurPool = PoolArray[PoolId];
    TaskID = TaskGetId();

    if (CurPool == NULL)                    // no pool allocated
    {
        CurPool = pool_alloc(PoolId);

    if (CurPool == NULL)
        {
            MP_ALERT("--E-- Out of memory for CurPool !!!");

            return NULL;
        }

        PoolArray[PoolId] = CurPool;
        CurPool->used[0] = TaskID;
        CurPool->freecount--;
        hp = (ST_HEAP_MEM *)((DWORD)CurPool + POOL_HEADER_SIZE);

        goto HP_ACCESS;
    }

    tmppool = CurPool;

    while (tmppool)
    {
        if (tmppool->freecount)
        {
            for (i = 0; i < MAX_PO0L_ENTRY; i++)
            {
                if (tmppool->used[i] == 0)
                {
                    tmppool->used[i] = TaskID;
                    tmppool->freecount--;
                    hp = (ST_HEAP_MEM *)((DWORD)tmppool + POOL_HEADER_SIZE + (i*PoolInfoArray[PoolId].EntryAddHD));

                    goto HP_ACCESS;
                }
            }

            tmppool->freecount = 0;
        }

        nextpool = tmppool->next;

        if (nextpool == NULL)               // all pool are used
        {
            nextpool = pool_alloc(PoolId);

            if (nextpool == NULL)
            {
                MP_ALERT("--E-- Out of memory for nextpool !!!");

                return NULL;
            }

            tmppool->next = nextpool;
            nextpool->used[0] = TaskID;
            nextpool->freecount--;
            hp = (ST_HEAP_MEM *)((DWORD)nextpool + POOL_HEADER_SIZE);

            goto HP_ACCESS;
        }

        tmppool = nextpool;
    }

    return NULL;

HP_ACCESS:
    hp->size = size;
    hp->used = 1;
    hp->tag = wTag;
    hp->TaskId = TaskID;

    return (void *)((DWORD)hp + SIZEOF_STRUCT_MEM);
}



void pool_mem_free(BYTE PoolId, void *rmem)
{
    ST_POOL_MEM *tmppool, *prevpool, *CurPool;
    ST_POOL_INFO *poolinfo;
    ST_HEAP_MEM *hp;
    DWORD memaddr, dwtmp;
    BYTE i;

    if (PoolId >= NUM_OF_POOL)
    {
        MP_ALERT("--E-- %s - PoolId exceed to NUM_OF_POOL !!!", __FUNCTION__);

        return;
    }

    CurPool = PoolArray[PoolId];
    poolinfo = (ST_POOL_INFO *)(&PoolInfoArray[PoolId]);
    memaddr = ((DWORD) rmem) & ~BIT29;
    tmppool = CurPool;
    prevpool = CurPool;

    while ((DWORD) tmppool > (DWORD) ram)
    {
        dwtmp = ((DWORD) tmppool) & ~BIT29;

        if ((memaddr > dwtmp) && (memaddr < (dwtmp+poolinfo->PoolContainSize)))     // memory in the pool
        {
            dwtmp = (memaddr - dwtmp - POOL_HEADER_SIZE - SIZEOF_STRUCT_MEM) / poolinfo->EntryAddHD;

            if (tmppool->used[dwtmp])
            {
                hp = (ST_HEAP_MEM *) (memaddr - SIZEOF_STRUCT_MEM);
                hp->used = 0;
                tmppool->used[dwtmp] = 0;
                tmppool->freecount++;

                if (tmppool->freecount == MAX_PO0L_ENTRY)                               // The pool is empty, free the pool from ext_heap
                {
                    if ( (((DWORD) tmppool) & ~BIT29) == (((DWORD) CurPool) & ~BIT29))  // First pool empty
                    {
                        if (tmppool->next == NULL)                                      // no pool needed
                            CurPool = NULL;
                        else
                            CurPool = (ST_POOL_MEM *)tmppool->next;
                    }
                    else
                    {
                        prevpool->next = tmppool->next;
                    }

                    _mem_free_(MAIN_HEAP, tmppool);
                }
            }

            PoolArray[PoolId] = CurPool;

            return;
        }
        else
        {
            prevpool = tmppool;
            tmppool = prevpool->next;
        }
    }

    PoolArray[PoolId] = CurPool;

    return;
}



void pool_free_task_mem(BYTE TaskId)
{
    ST_POOL_MEM *tmppool, *prevpool, *CurPool;
    DWORD i, j;

    MP_DEBUG("free task %d extend pool memory", TaskId);
    IntDisable();

    for (j = 0; j < NUM_OF_POOL; j++)
    {
        CurPool = PoolArray[j];

        if (CurPool)
        {
            tmppool = CurPool;
            prevpool = CurPool;

            while(tmppool)
            {
                for (i = 0; i < MAX_PO0L_ENTRY; i++)
                {
                    if (tmppool->used[i] == TaskId)
                    {
                        tmppool->used[i] = 0;
                        tmppool->freecount++;
                    }
                }

                if (tmppool->freecount == MAX_PO0L_ENTRY)
                {
                    if (tmppool == CurPool)                                     // First pool empty
                    {
                        if (tmppool->next == NULL)                              // no pool needed
                            CurPool = NULL;
                        else
                            CurPool = (ST_POOL_MEM *)tmppool->next;

                        _mem_free_(MAIN_HEAP, tmppool);
                        tmppool = CurPool;
                        prevpool = CurPool;
                    }
                    else
                    {
                        prevpool->next = tmppool->next;
                        _mem_free_(MAIN_HEAP, tmppool);
                        tmppool = prevpool->next;
                    }
                }
                else
                {
                    prevpool = tmppool;
                    tmppool = prevpool->next;
                }
            }
        }

        PoolArray[j] = CurPool;
    }

    IntEnable();
}

#endif      //#if POOL_MEM_ENABLE


/*-----------------------------------------------------------------------------------*/
/*                          function for main heap                                   */
/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
void *mem_Zones(DWORD SelMemNum)
{
    ST_HEAP_MEM *mem, *mem_start, *mem_end;

    if ((SelMemNum == 0) || (SelMemNum == 2))
    {
        mem_start = (ST_HEAP_MEM *) ker_ram;
        mem_end = ker_ram_end;
        IntDisable();

        for (mem = mem_start; mem < mem_end ; mem = (ST_HEAP_MEM *) mem->next)
        {
            if (mem->used == 0)
                MP_ALERT("Ker free :start addr=0x%X, size=0x%X", (DWORD) mem + SIZEOF_STRUCT_MEM, mem->size);
            else
                MP_ALERT("Ker used :TaskId=%d, start addr=0x%X, size=0x%X", mem->TaskId, (DWORD) mem + SIZEOF_STRUCT_MEM, mem->size);
        }

        IntEnable();
    }

    if ((SelMemNum == 1) || (SelMemNum == 2))
    {
        mem_start = (ST_HEAP_MEM *) ram;
        mem_end = ram_end;
        IntDisable();

        for (mem = mem_start; mem < mem_end; mem = (ST_HEAP_MEM *) mem->next)
        {            			
            if(mem->used == 0)
                MP_ALERT("Main free :start addr=0x%X, size=0x%X", (DWORD)mem + SIZEOF_STRUCT_MEM, mem->size);
            else
            {
                MP_ALERT("Main used :TaskId=%d, start addr=0x%X, size=0x%X", mem->TaskId, (DWORD) mem + SIZEOF_STRUCT_MEM, mem->size);
				if (mem->tag != wTag)
				    MP_ALERT("--E-- %s: Wrong memory tag !!!", __FUNCTION__);
            }	
        }

        IntEnable();
    }
}



/*-----------------------------------------------------------------------------------*/
void *mem_malloc(DWORD size)
{
    BYTE *ptr, TaskId, i;

    IntDisable();
    TaskId = TaskGetId();

#if DEBUG_MEM_ENABLE
    if (dm_chk_created(TaskId) != NULL)
        return dm_ext_mem_malloc(size, NullStr, 0);
#endif

#if POOL_MEM_ENABLE
    for (i = 0; i < NUM_OF_POOL; i++)
    {
        if (size <= PoolInfoArray[i].EntrySize)
        {
            ptr = pool_mem_malloc(i, size);

            if (!ptr)
                break;

            IntEnable();

            return ptr;
        }
    }
#endif  //#if POOL_MEM_ENABLE

    ptr = _mem_malloc_(MAIN_HEAP, size, TaskId);
    if (!ptr)
    {
        MP_ALERT("--E-- %s: malloc fail for Task-%d, need %u", __FUNCTION__, TaskId, size);
        MP_ALERT("      Free memory space is %u", mem_get_free_space());
    }

    IntEnable();

    return ptr;
}



/*-----------------------------------------------------------------------------------*/
void *mem_calloc(DWORD element, DWORD size)
{
    BYTE *ptr, TaskId, i;
    DWORD dwSize;

    IntDisable();
    TaskId = TaskGetId();

#if DEBUG_MEM_ENABLE
    if (dm_chk_created(TaskId) != NULL)
        return dm_ext_mem_calloc(element, size, NullStr, 0);
#endif

    dwSize = element*size;

#if POOL_MEM_ENABLE
    for (i = 0; i < NUM_OF_POOL; i++)
    {
        if (dwSize <= PoolInfoArray[i].EntrySize)
        {
            ptr = pool_mem_malloc(i, dwSize);
            break;
        }
    }

    if (i == NUM_OF_POOL)
        ptr = _mem_malloc_(MAIN_HEAP, dwSize, TaskId);
#else
    ptr = _mem_malloc_(MAIN_HEAP, size, TaskId);
#endif  //#if POOL_MEM_ENABLE

    if (ptr != NULL)
        memset (ptr, 0, dwSize);
    else
        MP_ALERT("--E-- %s: memory calloc fail !!!");

    IntEnable();

    return ptr;
}



/*-----------------------------------------------------------------------------------*/
void mem_free(void *rmem)
{
    ST_HEAP_MEM *mem;
    DWORD size;
    BYTE i;

    IntDisable();

#if DEBUG_MEM_ENABLE
    if(dm_chk_created(TaskGetId()) != NULL)
    {
        dm_ext_mem_free(rmem, NullStr, 0);

        return;
    }
#endif

    mem = (ST_HEAP_MEM *) ((DWORD)rmem - SIZEOF_STRUCT_MEM);
    DWORD dwmem = mem;
    if(dwmem & 3)
    {
        MP_ALERT("%s: mem(0x%X) & 3 is true, it cause Exception 1003 error, return fail!", __func__, mem);
        return;
    }
    size = mem->size;

#if POOL_MEM_ENABLE
    for (i = 0; i < NUM_OF_POOL; i++)
    {
        if (size <= PoolInfoArray[i].EntrySize)
        {
            pool_mem_free(i, rmem);
            IntEnable();

            return;
        }
    }
#endif  //#if POOL_MEM_ENABLE

    _mem_free_(MAIN_HEAP, rmem);

    IntEnable();
}


/*-----------------------------------------------------------------------------------*/
void *mem_reallocm(void *rmem, DWORD newsize)
{
    ST_HEAP_MEM *pNewMem;

    IntDisable();

#if DEBUG_MEM_ENABLE
    if (dm_chk_created(TaskGetId()) != NULL)
        return dm_ext_mem_reallocm(rmem, newsize, NullStr, 0);
#endif

    // if pointer is null, alloc new mem
    if (rmem == NULL)
    {
        pNewMem = mem_malloc(newsize);
        IntEnable();

        return pNewMem;
    }

    if (newsize == 0)
    {
        mem_free(rmem);
        IntEnable();

        return NULL;
    }

    // if new size is less than or equal to old size, direct return old address
    pNewMem = (ST_HEAP_MEM *) ((DWORD) rmem - SIZEOF_STRUCT_MEM);

    if (newsize <= pNewMem->size)
    {
        IntEnable();

        return rmem;
    }

    // alloc new mem
    pNewMem = mem_malloc(newsize);

    if (pNewMem == NULL)
    {
        IntEnable();

        return NULL;
    }

    // copy old content to new memory buffer
#if (CHIP_VER_MSB == CHIP_VER_615)
    memcpy((BYTE *) pNewMem, (BYTE *) rmem, newsize);
#else
    mmcp_memcpy((BYTE *) pNewMem, (BYTE *) rmem, newsize);
#endif

    mem_free(rmem);
    IntEnable();

    return pNewMem;
}

static DWORD mem_get_free_space_inter(const int total)
{
	DWORD dwMaxSize = SIZEOF_STRUCT_MEM;
	DWORD dwFreeSize = 0;
	ST_HEAP_MEM *mem;

	IntDisable();
	MP_DEBUG("%s -", __FUNCTION__);

	for (mem = (ST_HEAP_MEM *)ram; mem < ram_end; mem = (ST_HEAP_MEM *)(mem->next))
	{
		if (mem->used) continue;
		if (lfree > mem) lfree = mem;

		dwFreeSize += mem->size;

		if (mem->size >= dwMaxSize)
		{
			dwMaxSize = mem->size;
		}
	}

	MP_DEBUG("total free size %u", dwFreeSize);
	MP_DEBUG("max free block size %u", dwMaxSize - SIZEOF_STRUCT_MEM);
	IntEnable();

	if (total)
		return dwFreeSize;
	else
		return dwMaxSize - SIZEOF_STRUCT_MEM;

}

/*-----------------------------------------------------------------------------------*/
DWORD mem_get_free_space()
{
	//max free block size
    return mem_get_free_space_inter(0);
}

DWORD mem_get_free_space_total()
{
	//total free size
	return mem_get_free_space_inter(1);
}


/*-----------------------------------------------------------------------------------*/
void mem_free_task_mem(BYTE TaskId)
{
    ST_HEAP_MEM *mem;

    MP_DEBUG("free task %d memory", TaskId);

    IntDisable();

    for (mem = (ST_HEAP_MEM *)ram; mem < ram_end; mem = (ST_HEAP_MEM *)(mem->next))
    {
        if ((mem->TaskId != TaskId) || (mem->used == 0))
            continue;

        mem_free((BYTE *)mem + SIZEOF_STRUCT_MEM);
    }

    IntEnable();
}



/*-----------------------------------------------------------------------------------*/
/*                          function for extend heap                                 */
/*-----------------------------------------------------------------------------------*/

void *ext_mem_malloc(DWORD size)
{
    return mem_malloc(size);
}



void *ext_mem_calloc(DWORD element, DWORD size)
{
    return mem_calloc(element, size);
}



void ext_mem_free(void *rmem)
{
    mem_free(rmem);
}



void *ext_mem_reallocm(void *rmem, DWORD newsize)
{
    return mem_reallocm(rmem, newsize);
}



DWORD ext_mem_get_free_space()
{
    return mem_get_free_space();
}



void ext_mem_free_task_mem(BYTE TaskId)
{
    mem_free_task_mem(TaskId);
}



int User_mem_init(void)
{
    BYTE i;

    IntDisable();
    _mem_init_(MAIN_HEAP);

#if POOL_MEM_ENABLE
    for (i = 0; i < NUM_OF_POOL; i++)
    {
        PoolArray[i] = NULL;
    }
#endif  //#if POOL_MEM_ENABLE

#if DEBUG_MEM_ENABLE
	dm_init();
#endif	//#if DEBUG_MEM_ENABLE

    IntEnable();

    return PASS;
}



void FreeTaskUserMem(BYTE TaskId)
{
#if POOL_MEM_ENABLE
    pool_free_task_mem(TaskId);
#endif  //#if POOL_MEM_ENABLE

    mem_free_task_mem(TaskId);
}



/*-----------------------------------------------------------------------------------*/
/*                          function for kernel heap                                 */
/*-----------------------------------------------------------------------------------*/
void ker_mem_init(void)
{
    IntDisable();
    _mem_init_(KERNEL_HEAP);
    IntEnable();
}



void *ker_mem_malloc(DWORD size, BYTE taskId)
{
    BYTE *ptr;

    MP_DEBUG("Kernel free space is %dKB", ker_mem_get_free_space() >> 10);

    IntDisable();

    ptr = _mem_malloc_(KERNEL_HEAP, size, taskId);

    if (ptr)
    {
#ifdef ENABLE_KERNEL_MALLOC_TO_UNCACHE_REGION
        ptr = (BYTE *) (BIT29 | (DWORD) ptr);
#endif
    }
    else
    {
        MP_ALERT("--E-- ker_mem_malloc malloc fail by Task-%2d, need %u bytes!!!", taskId, size);
#ifdef ENABLE_BREAKPOINT_WHEN_KERNEL_MALLOC_FAIL
        __asm("break 100");
#endif
    }

    IntEnable();

    return (void *) ptr;
}



void ker_mem_free(void *rmem)
{
    IntDisable();
    _mem_free_(KERNEL_HEAP, rmem);
    IntEnable();
}



/*-----------------------------------------------------------------------------------*/
// if old memory is NULL, use taskid as new allocated memory taskid.
// others use old taskid as new allocated memory taskid.
void *ker_mem_reallocm(void *rmem, DWORD newsize, BYTE taskid)
{
    ST_HEAP_MEM *pNewMem;

    IntDisable();

    // if pointer is null, alloc new mem
    if (rmem == NULL)
    {
        pNewMem = ker_mem_malloc(newsize, taskid);
        IntEnable();

        return pNewMem;
    }

    if (newsize == 0)
    {
        ker_mem_free(rmem);
        IntEnable();

        return NULL;
    }

    // if new size is less than or equal to old size, direct return old address
    pNewMem = (ST_HEAP_MEM *) ((DWORD)rmem - SIZEOF_STRUCT_MEM);

    if (newsize <= pNewMem->size)
    {
        IntEnable();

        return rmem;
    }

    // alloc new mem
    pNewMem = ker_mem_malloc(newsize, pNewMem->TaskId);

    if (pNewMem == NULL)
    {
        IntEnable();

        return rmem;
    }

    // copy old content to new memory buffer
    memcpy(pNewMem, rmem, newsize);
    ker_mem_free(rmem);

    IntEnable();

    return pNewMem;
}



DWORD ker_mem_get_free_space()
{
    DWORD dwMaxSize = SIZEOF_STRUCT_MEM;
    DWORD dwFreeSize = 0;
    ST_HEAP_MEM *mem;

    IntDisable();
    MP_DEBUG("%s -", __FUNCTION__);

    for (mem = (ST_HEAP_MEM *)ker_ram; mem < ker_ram_end; mem = (ST_HEAP_MEM *)(mem->next))
    {
        if (mem->used) continue;
        if (ker_lfree > mem) ker_lfree = mem;

        dwFreeSize += mem->size;

        if (mem->size >= dwMaxSize)
        {
            dwMaxSize = mem->size;
        }
    }

    MP_DEBUG("free size %u", dwFreeSize);
    MP_DEBUG("max free block size %u", dwMaxSize - SIZEOF_STRUCT_MEM);
    IntEnable();

    return (dwMaxSize - SIZEOF_STRUCT_MEM);
}



void ker_mem_free_task_mem(BYTE TaskId)
{
    ST_HEAP_MEM *mem;

    MP_DEBUG("free task %d kernel memory", TaskId);
    IntDisable();

    for (mem = (ST_HEAP_MEM *)ker_ram; mem < ker_ram_end; mem = (ST_HEAP_MEM *)(mem->next))
    {
        if ((mem->TaskId != TaskId) || (mem->used == 0))
            continue;

        ker_mem_free((BYTE *) mem + SIZEOF_STRUCT_MEM);
    }

    IntEnable();
}



/*-----------------------------------------------------------------------------------*/
/*                          function for Debug                                       */
/*-----------------------------------------------------------------------------------*/
#if DEBUG_MEM_ENABLE
ST_DM_HEADER *dm_chk_created(BYTE TaskId)
{
    DWORD i;

    for (i = 0; i < MAX_DM_NUM; i++)
    {
        if (dm_array[i].TaskId == TaskId)
            return &dm_array[i];
    }

    return NULL;
}



ST_DM_ENTRY *dm_chk_free_chain(ST_DM_HEADER *dm_header, DWORD rmem)
{
    ST_DM_ENTRY *entry;
    DWORD i, count;

    count = dm_header->FreeEntryCount;
    entry = (ST_DM_ENTRY *)((DWORD)dm_header->dm_free_chain + count * DM_ENTRY_SIZE);

    for (i = 0; i < MAX_CHAIN_NUM; i++)
    {
        if(count == 0)
        {
            count = MAX_CHAIN_NUM;
            entry = (ST_DM_ENTRY *)((DWORD)dm_header->dm_free_chain + count * DM_ENTRY_SIZE);
        }

        count--;
        entry--;

        if (entry->addr == rmem)
            return entry;
    }

    return NULL;
}



void *dm_ext_mem_malloc(DWORD size, char* file, int line)
{
    BYTE *ptr, TaskId, i;
    ST_DM_HEADER *dm_header;
    ST_DM_ENTRY *dm_entry;

    IntDisable();

    TaskId = TaskGetId();

#if POOL_MEM_ENABLE
    for (i = 0; i < NUM_OF_POOL; i++)
    {
        if (size <= PoolInfoArray[i].EntrySize)
        {
            ptr = pool_mem_malloc(i, size);

            break;
        }
    }

    if (i == NUM_OF_POOL)
        ptr = _mem_malloc_(MAIN_HEAP, size, TaskId);
#else
    ptr = _mem_malloc_(MAIN_HEAP, size, TaskId);
#endif  //#if POOL_MEM_ENABLE

    // allocate memory fail
    if (ptr == NULL)
    {
        MP_ALERT("File %s\r\nLine %d\r\nAllocate %d Fail", file, line, size);
        IntEnable();

        return ptr;
    }

    // Check if dm created for the task
    dm_header = dm_chk_created(TaskId);

    if (dm_header == NULL)                  // dm not created for the task
    {
        IntEnable();

        return ptr;
    }

    // insert the allocate into allocate chain
#if POOL_MEM_ENABLE
    for (i = 0; i < NUM_OF_POOL; i++)
    {
        if(DM_ENTRY_SIZE <= PoolInfoArray[i].EntrySize)
        {
            dm_entry = (ST_DM_ENTRY *)pool_mem_malloc(i, DM_ENTRY_SIZE);
            break;
        }
    }

    if (i == NUM_OF_POOL)
        dm_entry = (ST_DM_ENTRY *) _mem_malloc_(MAIN_HEAP, DM_ENTRY_SIZE, TaskId);

#else
    dm_entry = (ST_DM_ENTRY *) _mem_malloc_(MAIN_HEAP, DM_ENTRY_SIZE, TaskId);
#endif  //#if POOL_MEM_ENABLE

    if (dm_entry)
    {
        dm_entry->next = dm_header->dm_allo_chain;
        dm_entry->addr = ((DWORD) ptr) & ~BIT29;
        dm_entry->file = file;
        dm_entry->line = line;
        dm_header->dm_allo_chain = (ST_DM_ENTRY *) (((DWORD) dm_entry) & ~BIT29);
        dm_header->AlloEntryCount++;
    }

    IntEnable();

    return ptr;
}



void *dm_ext_mem_calloc(DWORD element, DWORD size, char* file, int line)
{
    BYTE *ptr, TaskId, i;
    ST_DM_HEADER *dm_header;
    ST_DM_ENTRY *dm_entry;
    DWORD dwsize;

    IntDisable();

    dwsize = element * size;
    TaskId = TaskGetId();

#if POOL_MEM_ENABLE
    for (i = 0; i < NUM_OF_POOL; i++)
    {
        if (size <= PoolInfoArray[i].EntrySize)
        {
            ptr = pool_mem_malloc(i, size);

            break;
        }
    }

    if (i == NUM_OF_POOL)
        ptr = _mem_malloc_(MAIN_HEAP, size, TaskId);
#else
    ptr = _mem_malloc_(MAIN_HEAP, size, TaskId);
#endif  //#if POOL_MEM_ENABLE

    // allocate memory fail
    if (ptr == NULL)
    {
        MP_ALERT("File %s\r\nLine %d\r\nconstatnt Allocate %d Fail", file, line, dwsize);
        IntEnable();

        return ptr;
    }

    memset (ptr, 0, dwsize);

    // Check if dm created for the task
    dm_header = dm_chk_created(TaskId);

    if (dm_header == NULL)                  // dm not created for the task
    {
        IntEnable();

        return ptr;
    }

    // insert the allocate into allocate chain
#if POOL_MEM_ENABLE
    for (i = 0; i < NUM_OF_POOL; i++)
    {
        if (DM_ENTRY_SIZE <= PoolInfoArray[i].EntrySize)
        {
            dm_entry = (ST_DM_ENTRY *)pool_mem_malloc(i, DM_ENTRY_SIZE);
            break;
        }
    }

    if (i == NUM_OF_POOL)
        dm_entry = (ST_DM_ENTRY *)_mem_malloc_(MAIN_HEAP, DM_ENTRY_SIZE, TaskId);
#else
    dm_entry = (ST_DM_ENTRY *)_mem_malloc_(MAIN_HEAP, DM_ENTRY_SIZE, TaskId);
#endif  //#if POOL_MEM_ENABLE

    if (dm_entry)
    {
        dm_entry->next = dm_header->dm_allo_chain;
        dm_entry->addr = (DWORD)ptr & (~BIT29);
        dm_entry->file = file;
        dm_entry->line = line;
        dm_header->dm_allo_chain = (ST_DM_ENTRY *)((DWORD)dm_entry & (~BIT29));
        dm_header->AlloEntryCount++;
    }

    IntEnable();

    return ptr;
}



void dm_ext_mem_free(void *rmem, char* file, int line)
{
    ST_HEAP_MEM *pNewMem;
    ST_DM_HEADER *dm_header;
    ST_DM_ENTRY *entry;
    BYTE TaskId, j;
    DWORD i;

    IntDisable();

    rmem = (void *) (((DWORD) rmem) & ~BIT29);
    TaskId = TaskGetId();

    // check if rmem in extend heap range
    if((rmem < ram) || (rmem > ram_end))
    {
        MP_ALERT("File %s\r\nLine %d\r\nextend free over range !!", file, line);
        IntEnable();

        return;
    }

    // check ST_HEAP_MEM structure
    pNewMem = (ST_HEAP_MEM *) ((DWORD)rmem - SIZEOF_STRUCT_MEM);

    if (pNewMem->TaskId != TaskId)
    {
        MP_ALERT("File %s\r\nLine %d\r\nfree the memory not belong to the task !!", file, line);
    }

    dm_header = dm_chk_created(pNewMem->TaskId);

    if ((pNewMem->used == 0) || (pNewMem->tag != wTag))
    {
        MP_ALERT("File %s\r\nLine %d\r\nfree a invalid memory !!", file, line);

        if(dm_header != NULL)
        {
            entry = dm_chk_free_chain(dm_header, rmem);

            if (entry)
            {
                MP_ALERT("It is freed by\r\nFile %s\n\rLine %d", entry->file, entry->line);
            }
        }

        IntEnable();

        return;
    }

    // check if in extend heap chain, if yes, free the rmem
#if POOL_MEM_ENABLE
    for (j = 0; j < NUM_OF_POOL; j++)
    {
        if (pNewMem->size <= PoolInfoArray[j].EntrySize)
        {
            pool_mem_free(j, rmem);

            break;
        }
    }

    if (j == NUM_OF_POOL)
        _mem_free_(MAIN_HEAP, rmem);
#else
    _mem_free_(MAIN_HEAP, rmem);
#endif  //#if POOL_MEM_ENABLE

    if (dm_header == NULL)
        return;

    // insert the free to free chain
    entry = (ST_DM_ENTRY *)((DWORD)dm_header->dm_free_chain + dm_header->FreeEntryCount * DM_ENTRY_SIZE);
    entry->addr = (DWORD)rmem;
    entry->file = file;
    entry->line = line;
    dm_header->FreeEntryCount++;

    if (dm_header->FreeEntryCount == MAX_CHAIN_NUM)
        dm_header->FreeEntryCount = 0;

    // check if in dm_allo_chain, if yes, free from the chain
    entry = dm_header->dm_allo_chain;

    if(entry->addr == rmem)
    {
        rmem = entry->next;
        dm_header->dm_allo_chain = rmem;
        pNewMem = (ST_HEAP_MEM *) ((DWORD)entry - SIZEOF_STRUCT_MEM);

#if POOL_MEM_ENABLE
        for (j = 0; j < NUM_OF_POOL; j++)
        {
            if (pNewMem->size <= PoolInfoArray[j].EntrySize)
            {
                pool_mem_free(j, entry);
                break;
            }
        }

        if (j == NUM_OF_POOL)
            _mem_free_(MAIN_HEAP, entry);
#else
        _mem_free_(MAIN_HEAP, entry);
#endif  //#if POOL_MEM_ENABLE
        dm_header->AlloEntryCount--;
        IntEnable();

        return;
    }

    for (i = 1; i < dm_header->AlloEntryCount; i++)
    {
        if (entry->next->addr == rmem)
        {
            rmem = entry->next;
            pNewMem = (ST_HEAP_MEM *) ((DWORD)rmem - SIZEOF_STRUCT_MEM);

#if POOL_MEM_ENABLE
            for (j = 0; j < NUM_OF_POOL; j++)
            {
                if (pNewMem->size <= PoolInfoArray[j].EntrySize)
                {
                    pool_mem_free(j, rmem);
                    break;
                }
            }

            if (j == NUM_OF_POOL)
                _mem_free_(MAIN_HEAP, rmem);
#else
            _mem_free_(MAIN_HEAP, rmem);
#endif  //#if POOL_MEM_ENABLE
            entry->next = entry->next->next;
            dm_header->AlloEntryCount--;

            break;
        }

        entry = entry->next;
    }

    IntEnable();
}



void *dm_ext_mem_reallocm(void *rmem, DWORD newsize, char* file, int line)
{
    ST_HEAP_MEM *pNewMem;

    IntDisable();

    // if pointer is null, alloc new mem
    if (rmem == NULL)
    {
        pNewMem = dm_ext_mem_malloc(newsize, file, line);
        IntEnable();

        return pNewMem;
    }

    if (newsize == 0)
    {
        dm_ext_mem_free(rmem, file, line);
        IntEnable();

        return NULL;
    }

    // if new size is less than or equal to old size, direct return old address
    pNewMem = (ST_HEAP_MEM *) ((DWORD)rmem - SIZEOF_STRUCT_MEM);

    if (newsize <= pNewMem->size)
    {
        IntEnable();

        return rmem;
    }

    // alloc new mem
    pNewMem = dm_ext_mem_malloc(newsize, file, line);

    if (pNewMem == NULL)
    {
        IntEnable();

        return NULL;
    }

    // copy old content to new memory buffer
    memcpy(pNewMem, rmem, newsize);
    dm_ext_mem_free(rmem, file, line);
    IntEnable();

    return pNewMem;
}



// Dump all the allocate chain of dm created by specific task
// Please note that, the function is different to MemLeakChk().
// The MemLeakChk() is dump heap and pool area, no matter dm created or not.
// But dm_dump_allo_chain() is dump allocate chain of dm.
void dm_dump_allo_chain(BYTE TaskId)
{
    ST_DM_HEADER *dm_header;
    ST_DM_ENTRY *entry;

    dm_header = dm_chk_created(TaskId);

    if (dm_header == NULL)
        return;

    entry = dm_header->dm_allo_chain;

    while (entry)
    {
        MP_ALERT("File : %s\n\rLine : %d not freed !!", entry->file, entry->line);
        entry = entry->next;
    }
}



int dm_creat(BYTE TaskId)
{
    DWORD i;

    IntDisable();

    for (i = 0; i < MAX_DM_NUM; i++)
    {
        if (dm_array[i].TaskId == TaskId)
        {
            IntEnable();

            return PASS;
        }
        else if(dm_array[i].TaskId == 0)
        {
            dm_array[i].FreeEntryCount = 0;
            dm_array[i].AlloEntryCount = 0;
            dm_array[i].dm_allo_chain = NULL;
            dm_array[i].dm_free_chain = (ST_DM_HEADER *) _mem_malloc_(MAIN_HEAP, MAX_CHAIN_NUM * DM_ENTRY_SIZE, TaskId);

            if (dm_array[i].dm_free_chain == NULL)
            {
                IntEnable();

                return FAIL;
            }

            dm_array[i].TaskId = TaskId;
            memset (dm_array[i].dm_free_chain, 0, MAX_CHAIN_NUM * DM_ENTRY_SIZE);
            IntEnable();

            return PASS;
        }
    }

    IntEnable();

    return FAIL;
}



int dm_release(BYTE TaskId)
{
    DWORD i;
    ST_DM_ENTRY *tmpentry, *nextentry;
    ST_HEAP_MEM *pNewMem;
    BYTE j;

    IntDisable();

    for (i = 0; i < MAX_DM_NUM; i++)
    {
        if (dm_array[i].TaskId == TaskId)
        {
            _mem_free_(MAIN_HEAP, dm_array[i].dm_free_chain);       // free free_chain memory
            tmpentry = dm_array[i].dm_allo_chain;

            while (tmpentry != NULL)                                // free allocate_chain memory
            {
                nextentry = tmpentry->next;
                pNewMem = (ST_HEAP_MEM *) ((DWORD)tmpentry - SIZEOF_STRUCT_MEM);

#if POOL_MEM_ENABLE
                for (j = 0; j < NUM_OF_POOL; j++)
                {
                    if (pNewMem->size <= PoolInfoArray[j].EntrySize)
                    {
                        pool_mem_free(j, tmpentry);
                        break;
                    }
                }

                if (j == NUM_OF_POOL)
                    _mem_free_(MAIN_HEAP, tmpentry);
#else
                _mem_free_(MAIN_HEAP, tmpentry);
#endif  //#if POOL_MEM_ENABLE

                tmpentry = nextentry;
            }

            dm_array[i].dm_free_chain = NULL;
            dm_array[i].dm_allo_chain = NULL;
            dm_array[i].FreeEntryCount = 0;
            dm_array[i].AlloEntryCount = 0;
            dm_array[i].TaskId = 0;
            IntEnable();

            return PASS;
        }
    }

    IntEnable();

    return FAIL;
}



int dm_init()
{
    DWORD i;

    for (i = 0; i < MAX_DM_NUM; i++)
    {
        dm_array[i].dm_free_chain = NULL;
        dm_array[i].dm_allo_chain = NULL;
        dm_array[i].FreeEntryCount = 0;
        dm_array[i].AlloEntryCount = 0;
        dm_array[i].TaskId = 0;
    }

    return PASS;
}
#endif


// Relink the heap with just one broken point.
// The size will be set to 257 (if less than 257, it must be in pool, not in heap link).
// The taskId will be set to the task that check the link.
int MemLinkRelink(BYTE mode)
{
    ST_HEAP_MEM *mem_h, *mem_t, *mem_start, *mem_end;
    ST_HEAP_MEM *head, *tail;
    WORD tag;

    IntDisable();

    switch(mode)
    {
    case KERNEL_HEAP:
        mem_start = (ST_HEAP_MEM *) ker_ram;
        mem_end = (ST_HEAP_MEM *) ker_ram_end;
        tag = wKerTag;
        break;

    case MAIN_HEAP:
        mem_start = (ST_HEAP_MEM *) ram;
        mem_end = (ST_HEAP_MEM *) ram_end;
        tag = wTag;
        break;

    default:
        MP_ALERT("--E-- %s - Wrong memory area", __FUNCTION__);
        IntEnable();

        return PASS;
        break;
    }

    // Get broken point from head
    head = mem_start;
    for (mem_h = mem_start; mem_h < mem_end; mem_h = (ST_HEAP_MEM *)(mem_h->next))
    {
        if (mem_h == NULL)
            break;

        if ((mem_h->next < (DWORD) mem_start) || (mem_h->next > (DWORD) mem_end))
            break;

        if ((mem_h->prev < (DWORD) mem_start) || (mem_h->prev > (DWORD) mem_end))
            break;

        if ((mem_h->used) && (mem_h->tag != tag))
            break;

        head = mem_h;
    }

    // Get broken point from tail
    tail = (ST_HEAP_MEM *) mem_end->prev;

    for (mem_t = (ST_HEAP_MEM *) mem_end->prev; mem_t > mem_start; mem_t = (ST_HEAP_MEM *) (mem_t->prev))
    {
        if (mem_t == NULL)
            break;

        if ((mem_t->next < (DWORD) mem_start) || (mem_t->next > (DWORD) mem_end))
            break;

        if ((mem_t->prev < (DWORD) mem_start) || (mem_t->prev > (DWORD) mem_end))
            break;

        if ((mem_t->used) && (mem_t->tag != tag))
            break;

        tail = mem_t;
    }

    // Check if the broken point is the same
    if((mem_t != mem_h) || (mem_t == NULL) || (mem_h == NULL))
    {
        MP_ALERT("--E-- More than one broken point, can not relink !!");
        IntEnable();

        return FAIL;
    }

    // relink the heap chain
    mem_t->tag = tag;
    mem_t->used = 1;
    mem_t->TaskId = TaskGetId();
#if POOL_MEM_ENABLE
    mem_t->size = MAX_POOL_SIZE + 1;
#else
    mem_t->size = 0;
#endif  //#if POOL_MEM_ENABLE

    mem_t->next = (DWORD) tail;
    mem_t->prev = (DWORD) head;

    IntEnable();

    return PASS;
}



// The function will check the specific heap memory,
// and print out all error item
int MemLinkChk(BYTE mode)
{
    ST_HEAP_MEM *mem, *mem_start, *mem_end;
    WORD tag;

    IntDisable();

    MP_DEBUG("MemLinkChk mode = %d", mode);

    switch(mode)
    {
    case KERNEL_HEAP:
        mem_start = (ST_HEAP_MEM *) ker_ram;
        mem_end = (ST_HEAP_MEM *) ker_ram_end;
        tag = wKerTag;
        break;

    case MAIN_HEAP:
        mem_start = (ST_HEAP_MEM *) ram;
        mem_end = (ST_HEAP_MEM *) ram_end;
        tag = wTag;
        break;

    default:
        MP_ALERT("%s - Wrong memory area", __FUNCTION__);
        IntEnable();

        return PASS;
        break;
    }

    for (mem = mem_start; mem < mem_end; mem = (ST_HEAP_MEM *)(mem->next))
    {
        if(mem == NULL)
        {
            MP_ALERT("-E- MemLinkChk error 0x%08X", (size_t)mem);
            IntEnable();

            return FAIL;
        }

        if (mem->used)
        {
            if ((mem < mem_start) || (mem > mem_end) || (mem->tag != tag))
            {
                MP_ALERT("-E- MemLinkChk error 0x%08X", (size_t)mem);
                IntEnable();

                return FAIL;
            }
        }
    }

    MP_DEBUG("PASS !!");
    IntEnable();

    return PASS;
}



// The function will check the specific extend pool,
// and print out all error item
int MemPoolChk(BYTE PoolId)
{
#if POOL_MEM_ENABLE
    ST_POOL_MEM *tmppool;
    ST_HEAP_MEM *tmpheap;
    DWORD i, entrysize, tmpaddr;

    if (PoolId >= NUM_OF_POOL)
        return PASS;

    IntDisable();

    MP_ALERT("MemPoolChk PoolId = %d", PoolId);

    tmppool = PoolArray[PoolId];
    entrysize = PoolInfoArray[PoolId].EntryAddHD;

    while (tmppool != NULL)
    {
        tmpaddr = (DWORD)tmppool + POOL_HEADER_SIZE;

        for (i = 0; i < MAX_PO0L_ENTRY ; i++)
        {
            if (tmppool->used[i])
            {
                tmpheap = (ST_HEAP_MEM *)tmpaddr;

                if ((tmpheap->size > entrysize) ||
                    (tmpheap->used == 0) ||
                    (tmpheap->tag != wTag) ||
                    (tmpheap->TaskId != tmppool->used[i]))
                {
                    MP_ALERT("Pool 0x%08X entry %d error !!", (DWORD) tmppool, i);
                    MP_ALERT("%04d %08d, %4d", tmppool->used[i], tmpheap->size, tmpheap->used, tmpheap->tag, tmpheap->TaskId);
                }
            }

            tmpaddr += entrysize;
        }

        tmppool = tmppool->next;
    }

    MP_DEBUG("End check !!");

    IntEnable();
#endif  //#if POOL_MEM_ENABLE

    return PASS;
}



// The function will print out all memory (main heap and pool)
// allocated but not freed by the TaskId
int MemLeakChk(BYTE TaskID)
{
    ST_HEAP_MEM *mem;
    DWORD tmpaddr, i;
    BYTE j;

    IntDisable();

    MP_DEBUG("Check task ID %d memory leak", TaskID);

    for (mem = (ST_HEAP_MEM *) ram; mem < (ST_HEAP_MEM *) ram_end; mem = (ST_HEAP_MEM *) (mem->next))
    {
        if ((mem->used) && (mem->TaskId == TaskID))
        {
            MP_ALERT("Heap address 0x%08X not freed !!", (DWORD) mem);
            MemoryAddressToInfo((BYTE *)mem + 0x10, 1); // 0x10 - header, 1-showInfo
        }
    }

#if POOL_MEM_ENABLE
    ST_POOL_MEM *tmppool;

    for (j = 0; j < NUM_OF_POOL; j++)
    {
        tmppool = PoolArray[j];

        while (tmppool != NULL)
        {
            tmpaddr = (DWORD) tmppool + POOL_HEADER_SIZE;

            for (i = 0; i < MAX_PO0L_ENTRY ; i++)
            {
                if (tmppool->used[i] == TaskID)
                {
                    MP_ALERT("%d Id pool, address 0x%08X not freed !!", j, tmpaddr);
                }

                tmpaddr += PoolInfoArray[j].EntryAddHD;;
            }

            tmppool = tmppool->next;
        }
    }
#endif  //#if POOL_MEM_ENABLE

    IntEnable();

    return PASS;
}



int ext_mem_check()
{
    return MemLinkChk(MAIN_HEAP);
}



int ker_mem_check()
{
    return MemLinkChk(KERNEL_HEAP);
}



int mem_Relink()
{
return MemLinkRelink(MAIN_HEAP);
}



int ker_mem_Relink()
{
return MemLinkRelink(KERNEL_HEAP);
}




/*-----------------------------------------------------------------------------------*/
/*                          function for other purpos                                */
/*-----------------------------------------------------------------------------------*/
void SetDataCacheInvalid()
{
    IntDisable();
    FlushDataCache();
    IntEnable();
}



#ifdef __MONITOR_RESERVED_MEMORY_REGION
static DWORD reservedRegionLen = 0;
static DWORD *reservedRegionData = 0;

BOOL mem_ReservedRegionInfoSet(DWORD len, DWORD *dataPtr)
{
    reservedRegionLen = len;
    reservedRegionData = dataPtr;
}



BOOL mem_ReservedRegionCheck(void)
{
    BOOL needBreak = FALSE;
    DWORD i;

    for (i = 1; i < reservedRegionLen; i++)
    {
        if (reservedRegionData[i] != *(volatile DWORD *) (0xA0000000 + (i << 2)))
        {
            MP_ALERT("\r\n--E-- Addr 0x%08X 0x%08X was changed to 0x%08X at Task-%d", i, (DWORD) reservedRegionData[i], *(volatile DWORD *) (0xA0000000 + (i << 2)), TaskGetId());
            *(volatile DWORD *) (0xA0000000 + (i << 2)) = reservedRegionData[i];
            needBreak = TRUE;
        }
    }

    return needBreak;
}
#endif

//------------------------------------------------------------------------------
// memory EXtend debug functions
//------------------------------------------------------------------------------
#define MALLOC_EXTEND 0
#define MALLOC_INFO_MAX 1024
#if MALLOC_EXTEND
typedef struct
{
    BYTE * mem;  
    DWORD size;
    char *file;
    WORD line;
    WORD task;
}ST_MALLOC;
ST_MALLOC stMallocInfo[MALLOC_INFO_MAX];
WORD countMallocInfo = 0;
#endif
void *mem_callocEx(DWORD element, DWORD size, char * file, WORD line)
{
    return mem_mallocEx(element * size, file, line);
}
void *mem_mallocEx(DWORD size, char * file, WORD line)
{
    BYTE *mem = (BYTE *)mem_malloc(size);
#if MALLOC_EXTEND    
    if(mem != NULL)
    {
        stMallocInfo[countMallocInfo].mem  = mem;
        stMallocInfo[countMallocInfo].size = size;
        stMallocInfo[countMallocInfo].file = file;
        stMallocInfo[countMallocInfo].line = line;
        stMallocInfo[countMallocInfo].task = TaskGetId();
    }
    //mpDebugPrint("mallocEx() mem:0x%X, size:%d, file:%s, line:%d, task:%d", mem, size, file, line, TaskGetId());
    countMallocInfo ++;
    //mpDebugPrint("count %d", countMallocInfo);
    if(countMallocInfo >= MALLOC_INFO_MAX)
    {
        MP_ALERT("ALERT!!! countMallocInfo >= MALLOC_INFO_MAX ! Need to check !");  
        while(1);
    }
#endif    
    return mem; //ext_mem_malloc(size);
}
void *ext_mem_callocEx(DWORD element, DWORD size, char * file, WORD line)
{
    return ext_mem_mallocEx(element * size, file, line);
}
void *ext_mem_mallocEx(DWORD size, char * file, WORD line)
{
    BYTE *mem = (BYTE *)ext_mem_malloc(size);
#if MALLOC_EXTEND    
    if(mem != NULL)
    {
        stMallocInfo[countMallocInfo].mem  = mem;
        stMallocInfo[countMallocInfo].size = size;
        stMallocInfo[countMallocInfo].file = file;
        stMallocInfo[countMallocInfo].line = line;
        stMallocInfo[countMallocInfo].task = TaskGetId();
    }
    //mpDebugPrint("mallocEx() mem:0x%X, size:%d, file:%s, line:%d, task:%d", mem, size, file, line, TaskGetId());
    countMallocInfo ++;
    //mpDebugPrint("count %d", countMallocInfo);
    if(countMallocInfo >= MALLOC_INFO_MAX)
    {
        MP_ALERT("ALERT!!! countMallocInfo >= MALLOC_INFO_MAX ! Need to check !");  
        while(1);
    }
#endif    
    return mem; //ext_mem_malloc(size);
}


WORD MemoryAddressToInfo(BYTE *rmem, BYTE showInfo)
{
#if MALLOC_EXTEND
    if(showInfo)
        mpDebugPrint("MemoryAddressToInfo(BYTE *rmem = 0x%X)", rmem);
    BYTE * tmpmem = (BYTE *)rmem;
    BYTE *memInfo;
    WORD i;
    if(showInfo)
        mpDebugPrint("countMallocInfo = %d", countMallocInfo);
    for(i=0; i<countMallocInfo; i++)
    {
        if(showInfo)
            mpDebugPrint("stMallocInfo[%d].mem = 0x%X, size:%d, file:%s, line:%d, task:%d", i, stMallocInfo[i].mem, stMallocInfo[i].size, stMallocInfo[i].file, stMallocInfo[i].line, stMallocInfo[i].task);
        memInfo = stMallocInfo[i].mem;
        if(memInfo == tmpmem) //if(stMallocInfo[countMallocInfo].mem == tmpmem)
        {
            if(showInfo)
                MP_ALERT("Find stMallocInfo[%d] - size:%d, file:%s, line:%d, task:%d", i, stMallocInfo[i].size, stMallocInfo[i].file, stMallocInfo[i].line, stMallocInfo[i].task);
            break;
        }
    }
    return i;
#else
    return 0;  
#endif      
}


#if MALLOC_EXTEND
void freeMallocInfo(BYTE *rmem)
{
    //mpDebugPrint("freeMallocInfo(rmem=0x%X), countMallocInfo=%d", rmem, countMallocInfo);
    int i,j;
    i = MemoryAddressToInfo(rmem, 0);
    //mpDebugPrint("i=%d", i);
    if(i < countMallocInfo)
    {
        for(j = i; j<(countMallocInfo-1); j++ )
            memcpy((BYTE *)&stMallocInfo[j], (BYTE *)&stMallocInfo[j+1], sizeof(ST_MALLOC) );
            
        countMallocInfo--;
        //mpDebugPrint("Success! %d", countMallocInfo);
        //for(i= 0; i<countMallocInfo; i++) // to verify remaining records
        //    mpDebugPrint("%d mem 0x%X, size:%d", i, stMallocInfo[i].mem, stMallocInfo[i].size);
    }
    else // Not found!
    {
        if(i > 0) // if i == 0 then ??? TODO - check why!
        {
            MP_ALERT("NOT Found!!!");
            // dump remaining records
            mpDebugPrint("i = %d, countMallocInfo = %d", i, countMallocInfo);
            for(i= 0; i<countMallocInfo; i++)
                mpDebugPrint("stMallocInfo[%d].mem = 0x%X, size:%d, file:%s, line:%d, task:%d", i, stMallocInfo[i].mem, stMallocInfo[i].size, stMallocInfo[i].file, stMallocInfo[i].line, stMallocInfo[i].task);
        }    
    }
}
#endif
void ext_mem_freeEx(void *rmem, char * file, WORD line)
{
    DWORD dwmem = rmem;
    if(dwmem & 3)
    {
        MP_ALERT("%s: rmem(0x%X) & 3 is true, it makse Exception 1003 error, return fail! Calling is in file %s line %d", __func__, rmem, file, line);
        return;
    }
    ext_mem_free(rmem);
#if MALLOC_EXTEND    
    //mpDebugPrint("freeEx(rmem = 0x%X, file:%s, line:%d)", rmem, file, line);
    freeMallocInfo(rmem);
#endif    
}
void mem_freeEx(void *rmem, char * file, WORD line)
{
    mem_free(rmem);
#if MALLOC_EXTEND    
    //mpDebugPrint("freeEx(rmem = 0x%X, file:%s, line:%d)", rmem, file, line);
    freeMallocInfo(rmem);
#endif    
}

////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(ext_mem_malloc);
MPX_KMODAPI_SET(ext_mem_free);
#endif

