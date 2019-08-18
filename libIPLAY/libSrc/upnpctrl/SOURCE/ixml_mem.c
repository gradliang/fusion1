/*-----------------------------------------------------------------------------------*/
/* ixml_mem.c
 *
 * UPNP SDK Memory Dynamic Allocation manager.
 *
 * 03.12.2010 Modified by Kevin Huang
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

#include "taskid.h"

#define MEM_RESERVE_SIZE 32
#define MEM_LARGE_SIZE (33792 - MEM_RESERVE_SIZE)
#define MEM_MEDIA_SIZE (6144 - MEM_RESERVE_SIZE)
#define MEM_KEDIA_SIZE (256 - MEM_RESERVE_SIZE)
#define MEM_SMALL_SIZE (128 - MEM_RESERVE_SIZE)
#define TOTAL_LARGE_POOL 10
#define TOTAL_MEDIA_POOL 100
#define TOTAL_KEDIA_POOL 500
#define TOTAL_SMALL_POOL 3000
#define TOTAL_MEM_SIZE (TOTAL_LARGE_POOL*(MEM_LARGE_SIZE+MEM_RESERVE_SIZE) + TOTAL_MEDIA_POOL *(MEM_MEDIA_SIZE+MEM_RESERVE_SIZE) + TOTAL_KEDIA_POOL *(MEM_KEDIA_SIZE+MEM_RESERVE_SIZE) + TOTAL_SMALL_POOL *(MEM_SMALL_SIZE+MEM_RESERVE_SIZE))

//Make sure the sizeof(struct mem) is 16
typedef struct st_ixml_mem ST_IXML_MEM;
struct st_ixml_mem
{
	ST_IXML_MEM *next;
	void *data;
	unsigned long alloc;
	size_t size;
};

#pragma alignvar(32)
int IXMLSemid;

ST_IXML_MEM *ixml_pool_head_s = NULL;
ST_IXML_MEM *ixml_pool_tail_s = NULL;
int ixml_total_pool_s = 0;
void *ixml_buf_start_s;
void *ixml_buf_end_s;                              /* end of buffer pool */
void *ixml_buf_last_s;                             /* the last buffer */

ST_IXML_MEM *ixml_pool_head_k = NULL;
ST_IXML_MEM *ixml_pool_tail_k = NULL;
int ixml_total_pool_k = 0;
void *ixml_buf_start_k;
void *ixml_buf_end_k;                              /* end of buffer pool */
void *ixml_buf_last_k;                             /* the last buffer */


ST_IXML_MEM *ixml_pool_head_m = NULL;
ST_IXML_MEM *ixml_pool_tail_m = NULL;
int ixml_total_pool_m = 0;
void *ixml_buf_start_m;
void *ixml_buf_end_m;                              /* end of buffer pool */
void *ixml_buf_last_m;                             /* the last buffer */

ST_IXML_MEM *ixml_pool_head_l = NULL;
ST_IXML_MEM *ixml_pool_tail_l = NULL;
int ixml_total_pool_l = 0;
void *ixml_buf_start_l;
void *ixml_buf_end_l;                              /* end of buffer pool */
void *ixml_buf_last_l;                             /* the last buffer */

/*-----------------------------------------------------------------------------------*/
void ixml_mem_init(void * mem_addr)
{
	//ST_HEAP_MEM *mem;
	u8_t *tmp;
	unsigned int i;
	ST_IXML_MEM *cur_net_mem;
    int ret;

	MP_DEBUG("\n\r!!!=============== UPNP SDK mem_init ==================!!!\n\r");

	ixml_pool_head_s = ixml_pool_tail_s = NULL;
	ixml_pool_head_k = ixml_pool_tail_k = NULL;
	ixml_pool_head_m = ixml_pool_tail_m = NULL;
	ixml_pool_head_l = ixml_pool_tail_l = NULL;
	//SMALL
	tmp = (u8_t *) mem_addr;
	for ( i = 0 ; i < TOTAL_SMALL_POOL ; i++ )
	{
		cur_net_mem = (ST_IXML_MEM *) (tmp+i*(MEM_SMALL_SIZE+MEM_RESERVE_SIZE));
		cur_net_mem->data = tmp+i*(MEM_SMALL_SIZE+MEM_RESERVE_SIZE)+MEM_RESERVE_SIZE;
		cur_net_mem->alloc = FALSE;
		if( ixml_pool_head_s == NULL )
		{
			ixml_pool_head_s = ixml_pool_tail_s = cur_net_mem;
			ixml_pool_head_s->next = NULL;
		}
		else
		{
			ixml_pool_tail_s->next = cur_net_mem;
			ixml_pool_tail_s = cur_net_mem;
		}
	}
	ixml_pool_tail_s->next = NULL;
	ixml_buf_start_s = ixml_pool_head_s;
	ixml_buf_last_s = ixml_pool_tail_s;
	ixml_buf_end_s = (char *)ixml_buf_last_s + (MEM_SMALL_SIZE+MEM_RESERVE_SIZE);
	ixml_total_pool_s = TOTAL_SMALL_POOL;
	//KEDIA
	tmp = (char *) ixml_buf_start_s + (MEM_SMALL_SIZE+MEM_RESERVE_SIZE) * TOTAL_SMALL_POOL;
	for ( i = 0 ; i < TOTAL_KEDIA_POOL ; i++ )
	{
		cur_net_mem = (ST_IXML_MEM *) (tmp+i*(MEM_KEDIA_SIZE+MEM_RESERVE_SIZE));
		cur_net_mem->data = tmp+i*(MEM_KEDIA_SIZE+MEM_RESERVE_SIZE)+MEM_RESERVE_SIZE;
		cur_net_mem->alloc = FALSE;
		if( ixml_pool_head_k == NULL )
		{
			ixml_pool_head_k = ixml_pool_tail_k = cur_net_mem;
			ixml_pool_head_k->next = NULL;
		}
		else
		{
			ixml_pool_tail_k->next = cur_net_mem;
			ixml_pool_tail_k = cur_net_mem;
		}
	}
	ixml_pool_tail_k->next = NULL;
	ixml_buf_start_k = ixml_pool_head_k;
	ixml_buf_last_k = ixml_pool_tail_k;
	ixml_buf_end_k = (char *)ixml_buf_last_k + (MEM_KEDIA_SIZE+MEM_RESERVE_SIZE);
	ixml_total_pool_k = TOTAL_KEDIA_POOL;
	
	//MEDIA
	tmp = (char *) ixml_buf_start_k + (MEM_KEDIA_SIZE+MEM_RESERVE_SIZE) * TOTAL_KEDIA_POOL;
	for ( i = 0 ; i < TOTAL_MEDIA_POOL ; i++ )
	{
		cur_net_mem = (ST_IXML_MEM *) (tmp+i*(MEM_MEDIA_SIZE+MEM_RESERVE_SIZE));
		cur_net_mem->data = tmp+i*(MEM_MEDIA_SIZE+MEM_RESERVE_SIZE)+MEM_RESERVE_SIZE;
		cur_net_mem->alloc = FALSE;
		if( ixml_pool_head_m == NULL )
		{
			ixml_pool_head_m = ixml_pool_tail_m = cur_net_mem;
			ixml_pool_head_m->next = NULL;
		}
		else
		{
			ixml_pool_tail_m->next = cur_net_mem;
			ixml_pool_tail_m = cur_net_mem;
		}
	}
	ixml_pool_tail_m->next = NULL;
	ixml_buf_start_m = ixml_pool_head_m;
	ixml_buf_last_m = ixml_pool_tail_m;
	ixml_buf_end_m = (char *)ixml_buf_last_m + (MEM_MEDIA_SIZE+MEM_RESERVE_SIZE);
	ixml_total_pool_m = TOTAL_MEDIA_POOL;
	//LARGE
	tmp = (char *)ixml_buf_start_m + (MEM_MEDIA_SIZE+MEM_RESERVE_SIZE) * TOTAL_MEDIA_POOL;
	for ( i = 0 ; i < TOTAL_LARGE_POOL ; i++ )
	{
		cur_net_mem = (ST_IXML_MEM *) (tmp+i*(MEM_LARGE_SIZE+MEM_RESERVE_SIZE));
		cur_net_mem->data = tmp+i*(MEM_LARGE_SIZE+MEM_RESERVE_SIZE)+MEM_RESERVE_SIZE;
		cur_net_mem->alloc = FALSE;
		if( ixml_pool_head_l == NULL )
		{
			ixml_pool_head_l = ixml_pool_tail_l = cur_net_mem;
			ixml_pool_head_l->next = NULL;
		}
		else
		{
			ixml_pool_tail_l->next = cur_net_mem;
			ixml_pool_tail_l = cur_net_mem;
		}
	}
	ixml_pool_tail_l->next = NULL;
	cur_net_mem = ixml_pool_head_l;
	ixml_buf_start_l = ixml_pool_head_l;
	ixml_buf_last_l = ixml_pool_tail_l;
	ixml_buf_end_l = (char *)ixml_buf_last_l + MEM_LARGE_SIZE+MEM_RESERVE_SIZE;
	ixml_total_pool_l = TOTAL_LARGE_POOL;
	mpDebugPrint("total mem size is %d",TOTAL_MEM_SIZE);
	MP_DEBUG("##########end ixm_malloc########");
}

void *ixml_mem_malloc(size_t size)
{
	MP_ASSERT(IXMLSemid > 0);
	SemaphoreWait(IXMLSemid);
	ST_IXML_MEM *cur_net_mem;

	int i =0;
	char *ptrc;


	//mpDebugPrint("##############################ixml_mem_malloc=====> %d",size);
	if( size > MEM_LARGE_SIZE )
	{
		mpDebugPrint("##############################iXML Allocate too large %d\n",size);
		mpDebugPrint("\n");
		mpDebugPrint("##############################iXML Allocate too large \n");
		SemaphoreRelease(IXMLSemid);
		__asm("break 100");
		MP_ASSERT(0);
		return NULL;
	}
	if( size > MEM_MEDIA_SIZE )
	{
		//mpDebugPrint("ixml_mem_malloc size %d",size);
		cur_net_mem = ixml_pool_head_l;
		//mpDebugPrint("############################## %d > 224 ixml_mem_malloc=====> %x ixml_total_pool_l %d",size,ixml_pool_head_l,ixml_total_pool_l);
		//mpDebugPrint("ixml_total_pool %d %d %d %d",ixml_total_pool_l,ixml_total_pool_m,ixml_total_pool_k,ixml_total_pool_s);
		if(ixml_pool_head_l == NULL )
		{
			MP_ASSERT(0);
			return NULL;
		}
		ixml_pool_head_l = cur_net_mem->next;
		ixml_total_pool_l--;
		//mpDebugPrint("%x ixml_total_pool_l %x",cur_net_mem,ixml_total_pool_l);
		SemaphoreRelease(IXMLSemid);
		if( cur_net_mem->alloc == TRUE )
		{
			MP_DEBUG("!(cur_net_mem->alloc)");
			MP_ASSERT(cur_net_mem->alloc);
		}
		cur_net_mem->alloc = TRUE;
		cur_net_mem->size = size;
	}
	else if( size > MEM_KEDIA_SIZE || (ixml_pool_head_k == NULL) )
	{
		cur_net_mem = ixml_pool_head_m;
		if(ixml_pool_head_m == NULL )
		{
			mpDebugPrint("ixml_total_pool %d %d %d %d",ixml_total_pool_l,ixml_total_pool_m,ixml_total_pool_k,ixml_total_pool_s);
			mpDebugPrint("ixml_pool_head_m == NULL ");
			__asm("break 100");
			MP_ASSERT(0);
			return NULL;
		}
		ixml_pool_head_m = cur_net_mem->next;
		ixml_total_pool_m--;
		//mpDebugPrint("%x ixml_total_pool %x",cur_net_mem,ixml_total_pool);
		SemaphoreRelease(IXMLSemid);
		if( cur_net_mem->alloc == TRUE )
		{
			MP_DEBUG("!(cur_net_mem->alloc)");
			MP_ASSERT(cur_net_mem->alloc);
		}
		cur_net_mem->alloc = TRUE;
		cur_net_mem->size = size;
	}
	else if( size > MEM_SMALL_SIZE || (ixml_pool_head_s == NULL) )
	{
		cur_net_mem = ixml_pool_head_k;
		if(ixml_pool_head_k == NULL )
		{
			mpDebugPrint("ixml_total_pool %d %d %d ",ixml_total_pool_l,ixml_total_pool_m,ixml_total_pool_s);
			mpDebugPrint("ixml_pool_head_m == NULL ");
			__asm("break 100");
			MP_ASSERT(0);
			return NULL;
		}
		ixml_pool_head_k = cur_net_mem->next;
		ixml_total_pool_k--;
		//mpDebugPrint("%x ixml_total_pool %x",cur_net_mem,ixml_total_pool);
		SemaphoreRelease(IXMLSemid);
		if( cur_net_mem->alloc == TRUE )
		{
			MP_DEBUG("!(cur_net_mem->alloc)");
			MP_ASSERT(cur_net_mem->alloc);
		}
		cur_net_mem->alloc = TRUE;
		cur_net_mem->size = size;
	}
	else
	{
		cur_net_mem = ixml_pool_head_s;
		if( ixml_total_pool_s < 3)
			mpDebugPrint("############################## ixml_total_pool_s < 10 ixml_mem_malloc=====> %x %x",ixml_pool_head_s,cur_net_mem);
		//mpDebugPrint("############################## small ixml_mem_malloc=====> %x %x",ixml_pool_head_s,cur_net_mem);
		if(ixml_pool_head_s == NULL )
		{
			mpDebugPrint("ixml_pool_head_s == NULL ");
			//MP_ASSERT(0);
			__asm("break 100");
			return NULL;
		}
		ixml_pool_head_s = cur_net_mem->next;
		ixml_total_pool_s--;
		//mpDebugPrint("%x ixml_total_pool %x",cur_net_mem,ixml_total_pool);
		SemaphoreRelease(IXMLSemid);
		if( cur_net_mem->alloc == TRUE )
		{
			MP_DEBUG("!(cur_net_mem->alloc)");
			MP_ASSERT(cur_net_mem->alloc);
		}
		cur_net_mem->alloc = TRUE;
		//cur_net_mem->size = size;
	}
	return cur_net_mem->data;
}

void ixml_mem_free(void *rmem)
{
	ST_IXML_MEM *cur_net_mem;
	SemaphoreWait(IXMLSemid);	
	cur_net_mem = (ST_IXML_MEM *) (rmem-MEM_RESERVE_SIZE);
	if( (DWORD)cur_net_mem < (DWORD)ixml_buf_start_s || (DWORD)cur_net_mem > (DWORD)ixml_buf_last_l)
	{
		mpDebugPrint("rmem %x cur_net_mem %x %x %x",rmem,cur_net_mem,ixml_buf_start_s,ixml_buf_last_l);
		mpDebugPrint("KKKKKKKKK");
		SemaphoreRelease(IXMLSemid);
		//MP_ASSERT(0);
		__asm("break 100");
		return;
	}

	if (cur_net_mem->alloc == FALSE)
		mpDebugPrint("rmem %p cur_net_mem %p ",rmem,cur_net_mem);
	if( !(cur_net_mem->alloc) )
	{
		MP_DEBUG("!(cur_net_mem->alloc)");
		MP_ASSERT(cur_net_mem->alloc);
	}
	cur_net_mem->alloc = FALSE;

	if( (void *) cur_net_mem >= ixml_buf_start_l )
	{
		if( ixml_pool_tail_l )
		{
			ixml_pool_tail_l->next = cur_net_mem;
			cur_net_mem->next = NULL;
			ixml_pool_tail_l = cur_net_mem;
			ixml_total_pool_l++;
		}
		else
		{
			mpDebugPrint("Something Error=================>");
			MP_ASSERT(0);
			__asm("break 100");
		}
	}
	else if( (void *) cur_net_mem >= ixml_buf_start_m )
	{
		if( ixml_pool_tail_m )
		{
			ixml_pool_tail_m->next = cur_net_mem;
			cur_net_mem->next = NULL;
			ixml_pool_tail_m = cur_net_mem;
			ixml_total_pool_m++;
		}
		else
		{
			mpDebugPrint("Something Error=================>");
			MP_ASSERT(0);
			__asm("break 100");
		}
	}
	else if( (void *) cur_net_mem >= ixml_buf_start_k )
	{
		if( ixml_pool_tail_k )
		{
			ixml_pool_tail_k->next = cur_net_mem;
			cur_net_mem->next = NULL;
			ixml_pool_tail_k = cur_net_mem;
			ixml_total_pool_k++;
		}
		else
		{
			mpDebugPrint("Something Error=================>");
			MP_ASSERT(0);
			__asm("break 100");
		}
	}
	else
	{
		if( ixml_pool_tail_s )
		{
			ixml_pool_tail_s->next = cur_net_mem;
			cur_net_mem->next = NULL;
			ixml_pool_tail_s = cur_net_mem;
			ixml_total_pool_s++;
		}
		else
		{
			mpDebugPrint("Something Error=================>");
			MP_ASSERT(0);
		}
	}
	SemaphoreRelease(IXMLSemid);
}

void *ixml_mem_zalloc(size_t size)
{
	void *ptr;
	ptr = ixml_mem_malloc(size);
	if (ptr)
		memset(ptr, 0, size);
	return ptr;
}


void printixmlpool()
{
	ST_IXML_MEM *cur_net_mem;
	int i =0;
	char *ptrc;
	char tmpbuf[32];
	SemaphoreWait(IXMLSemid);
	if ( ixml_total_pool_l < 10 )
	{
		mpDebugPrint("ixml_total_pool_l %d %d %d %d",ixml_total_pool_l,ixml_total_pool_m,ixml_total_pool_k,ixml_total_pool_s);
		cur_net_mem = (ST_IXML_MEM *) ixml_buf_start_l;
#if 0
		while( i < 10 )
		{
			//mpDebugPrint("data %x",cur_net_mem->data);
			ptrc = (char *) cur_net_mem->data;
			if( cur_net_mem->alloc == TRUE )
			{
				//memcpy(tmpbuf,ptrc,31);
				//mpDebugPrint("%c%c%c%c%c",ptrc[0],ptrc[1],ptrc[2],ptrc[3],ptrc[4]);
				//tmpbuf[31] = '\0';
				//mpDebugPrint(tmpbuf);
			}
			i++;
			cur_net_mem = (char *) cur_net_mem + 32768;
		}
#endif
	}
	else
	{
		mpDebugPrint("ixml_total_pool_l %d %d %d %d",ixml_total_pool_l,ixml_total_pool_m,ixml_total_pool_k,ixml_total_pool_s);
		cur_net_mem = (ST_IXML_MEM *) ixml_buf_start_s;
/*
		while( i < 1000 )
		{
			//mpDebugPrint("data %x",cur_net_mem->data);
			ptrc = (char *) cur_net_mem->data;
			if( cur_net_mem->alloc == TRUE )
				mpDebugPrint("%d %s",i,ptrc);
			i++;
			cur_net_mem = (char *) cur_net_mem + 128;
		}
*/
	}
	SemaphoreRelease(IXMLSemid);
}
