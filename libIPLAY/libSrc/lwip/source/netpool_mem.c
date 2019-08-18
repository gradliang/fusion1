/*-----------------------------------------------------------------------------------*/
/* mem.c
 *
 * Movie Player Memory Dynamic Allocation manager.
 *
 * 07.15.2006 Modified by Athena
 */ 
/*-----------------------------------------------------------------------------------*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "typedef.h"
#include "string.h" 
//#include "os_defs.h" 
#include "SysConfig.h" 

#include "taskid.h"
#include "linux/skbuff.h"
#include "net_device.h"

#define NETBUF_OVERHEAD 32
#define MAX_NETPOOLS 8

//Make sure the sizeof(struct mem) is 16
typedef struct st_net_mem ST_NET_MEM;
struct st_net_mem {
/*
	WORD tag;
	BYTE used;
	BYTE bReserved;
	DWORD size;
	mem_size_t next, prev;
*/
	//u32_t used;
	//u32_t dummy;
	ST_NET_MEM *next;
	void *data;
	BOOLEAN alloc;
	BOOLEAN pool_id;
	size_t size;
};

typedef struct st_net_pool {
	size_t size;
    int nBufs;
    int nFreeBufs;
} ST_NET_POOL;

int HeapSemid = 0;
static int num_net_pools;
static ST_NET_POOL net_pool[MAX_NETPOOLS];
ST_NET_MEM *net_pool_head[MAX_NETPOOLS];
ST_NET_MEM *net_pool_tail[MAX_NETPOOLS];
void *net_buf_start;
void *net_buf_end;                              /* end of buffer pool */
void *net_buf_last;                             /* the last buffer */
static uint32_t max_size = RALINK_MAX_NETPOOL_BUFFER_SIZE;
/*-----------------------------------------------------------------------------------*/
void *net_buf_mem_malloc(size_t size)
{
    int i;

	MP_ASSERT(HeapSemid > 0);
#if 0
	SemaphoreWait(HeapSemid);	
#else
    IntDisable();
#endif
	ST_NET_MEM *cur_net_mem;
	if( size > max_size )
	{
		mpDebugPrint("##############################Allocate too large %d",size);
#if 0
		SemaphoreRelease(HeapSemid);
#else
        IntEnable();
#endif
		MP_ASSERT(0);
		return NULL;
	}

    for (i=0; i < num_net_pools; i++)
    {
        if( size <= net_pool[i].size && net_pool[i].nFreeBufs)
            break;
    }

	if (i == num_net_pools)
	{
		MP_ASSERT(0);
#if 0
		SemaphoreRelease(HeapSemid);
#else
        IntEnable();
#endif
		return NULL;
	}
	//Check the memmory pool , if the pool is empty , get the large size
	//Before 7489 version the net_pool_head[0] run out of the pool, and crash
	if( net_pool_head[i] == NULL )
	{
		i++;
		for( ; i < num_net_pools && net_pool_head[i] == NULL ; i++ )
		{
		}
	}
    MP_ASSERT(net_pool_head[i]);
    MP_ASSERT(net_pool[i].nFreeBufs > 0);

	cur_net_mem = net_pool_head[i];
	net_pool_head[i] = cur_net_mem->next;
	net_pool[i].nFreeBufs--;
	if (net_pool[i].nFreeBufs == 0)
		net_pool_tail[i] = NULL;
#if 0
	SemaphoreRelease(HeapSemid);
#else
    IntEnable();
#endif
	cur_net_mem->alloc = TRUE;
//	cur_net_mem->size = size;
	cur_net_mem->size = net_pool[i].size;
	cur_net_mem->pool_id = i;
	return cur_net_mem->data;
}

void net_buf_mem_free(void *rmem)
{
	ST_NET_MEM *cur_net_mem;
    short i;
#if 0
	SemaphoreWait(HeapSemid);	
#else
    IntDisable();
#endif
	cur_net_mem = (ST_NET_MEM *) (rmem-0x20);
	//mpDebugPrint("rmem %x %x %x",rmem,(rmem-0x10),cur_net_mem);
	if( (DWORD)cur_net_mem < (DWORD)net_buf_start || (DWORD)cur_net_mem > (DWORD)net_buf_last)
	{
		mpDebugPrint("rmem %x cur_net_mem %x ",rmem,cur_net_mem);
#if 0
		SemaphoreRelease(HeapSemid);
#else
        IntEnable();
#endif
		MP_ASSERT(0);
		return;
	}
	if (cur_net_mem->alloc == FALSE)
    {
		mpDebugPrint("rmem %p cur_net_mem %p already freed",rmem,cur_net_mem);
        IntEnable();
		return;
    }
    MP_ASSERT(cur_net_mem->alloc);
	cur_net_mem->alloc = FALSE;
	//if( rmem )
	i = cur_net_mem->pool_id;
	if( net_pool_tail[i] )
	{
		net_pool_tail[i]->next = cur_net_mem;
		cur_net_mem->next = NULL;
		net_pool_tail[i] = cur_net_mem;
        net_pool[i].nFreeBufs++;
	}
	else
	{
		cur_net_mem->next = NULL;
		net_pool_tail[i] = cur_net_mem;
		net_pool[i].nFreeBufs++;
		net_pool_head[i] = cur_net_mem;
	}
#if 0
	SemaphoreRelease(HeapSemid);
#else
    IntEnable();
#endif
}

void *net_buf_mem_zalloc(size_t size)
{
	void *ptr;
#ifdef DEBUG_SKBUFF_LEAKAGE
    ptr = _net_buf_mem_malloc(size, __location__);
#else
	ptr = net_buf_mem_malloc(size);
#endif
	if (ptr)
		memset(ptr, 0, size);
	return ptr;
}

/*-----------------------------------------------------------------------------------*/
void *net_buf_mem_reallocm(void *rmem, DWORD newsize)
{
	ST_NET_MEM *pNewMem;

	// if pointer is null, alloc new mem
	if (rmem == NULL)
	{
		pNewMem = net_buf_mem_malloc(newsize);
		if (pNewMem == NULL)
		{
			return NULL;
		}
		else
			return pNewMem;
	}

	// alloc new mem
	pNewMem = net_buf_mem_malloc(newsize);
	if (pNewMem == NULL)
	{
		//SystemSetErrEvent(ERR_AV_STYSTEM);
		return NULL;
	}

	return pNewMem;
}
int netpool_create(char *chunk, unsigned int nbufs, unsigned int bufsz)
{
	ST_NET_MEM *cur_net_mem;
    char *bp;
    int i;
    size_t asize;
    short pid = num_net_pools;

    if (num_net_pools >= MAX_NETPOOLS)
        return -1;

    asize = bufsz + NETBUF_OVERHEAD;
    bp = chunk;

	for ( i = 0 ; i < nbufs ; i++ )
	{
		cur_net_mem = (ST_NET_MEM *) bp;
		cur_net_mem->data = bp+NETBUF_OVERHEAD;
		if( net_pool_head[pid] == NULL )
		{
			net_pool_head[pid] = net_pool_tail[pid] = cur_net_mem;
			net_pool_head[pid]->next = NULL;
		}
		else
		{		
			net_pool_tail[pid]->next = cur_net_mem;
			net_pool_tail[pid] = cur_net_mem;
	}

		bp += asize;
	}
    net_pool_tail[pid]->next = NULL;

	mpDebugPrint("[NETPOOL] %d: head %x tail %x", pid, net_pool_head[pid] ,net_pool_tail[pid]);
	mpDebugPrint("[NETPOOL] %d: head->next %x tail->next %x", pid, net_pool_head[pid]->next ,net_pool_tail[pid]->next);

    net_pool[pid].size = bufsz;
    net_pool[pid].nBufs = nbufs;
    net_pool[pid].nFreeBufs = nbufs;

	if ((uint32_t)net_buf_last < (uint32_t)net_pool_tail[pid])
		net_buf_last = net_pool_tail[pid];
	if (max_size < bufsz)
		max_size = bufsz;
	
    num_net_pools++;
    return 0;
}

int netpool_destroy(void)
{
    num_net_pools = 0;
	memset(net_pool, 0, sizeof net_pool);
	memset(net_pool_head, 0, sizeof net_pool_head);
	memset(net_pool_tail, 0, sizeof net_pool_tail);
	net_buf_start =
	net_buf_end =
	net_buf_last = NULL;
    max_size = RALINK_MAX_NETPOOL_BUFFER_SIZE;
}

size_t net_buf_size_get(void *rmem)
{
	ST_NET_MEM *cur_net_mem;
	if (rmem)
    {
		cur_net_mem = (ST_NET_MEM *) (rmem-0x20);
        return cur_net_mem->size;
    }
    else
        return 0;
}

