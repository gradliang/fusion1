#ifndef __UTIL_QUEUE_H
#define __UTIL_QUEUE_H

#include "typedef.h"
#include "util_node.h"

typedef struct
{
    ST_NODE* head;
    ST_NODE* tail;
    U32 length; //size of this container
} ST_QUEUE;

ST_QUEUE* mpx_QueueCreate();
void mpx_QueueDelete(ST_QUEUE* queue);
S32 mpx_QueuePush(ST_QUEUE* queue, U32 item);
S32 mpx_QueuePop(ST_QUEUE* queue, U32* item);
U32 mpx_QueueItemGet(ST_QUEUE* queue, U32 index);
U32 mpx_QueueLengthGet(ST_QUEUE* queue);
#endif
