#include <stdio.h>
#include "typedef.h"
#include "util_queue.h"
#include "ndebug.h"
#include "taskid.h"

ST_QUEUE* mpx_QueueCreate()
{
    ST_QUEUE* queue = mpx_Malloc(sizeof(ST_QUEUE));

    if(!queue)
        return 0;

    queue->head = queue->tail = 0;
    queue->length = 0;

    return queue;
}

void mpx_QueueDelete(ST_QUEUE* queue)
{
    if(queue)
    {
        mpx_SemaphoreWait(NET_SEM_ID);

        while(queue->length)
        {
            ST_NODE* node = queue->head;

            queue->head = (ST_NODE *) node->next;
            mpx_Free(node);
            queue->length--;
        }

        mpx_SemaphoreRelease(NET_SEM_ID);

        mpx_Free(queue);
    }
}

U32 mpx_QueueLengthGet(ST_QUEUE* queue)
{
    return queue->length;
}

S32 mpx_QueuePush(ST_QUEUE* queue, U32 item)
{
    ST_NODE* node;

    if(!queue || !item)
        return -1;

    node = mpx_Malloc(sizeof(ST_NODE));
    if(!node)
        return -1;

    node->data = item;
    node->next = 0;

    mpx_SemaphoreWait(NET_SEM_ID);

    if(queue->length == 0)
    {
        node->prev = 0;
        queue->head = queue->tail = node;
    }
    else
    {
        node->prev = (U32) queue->tail;
        queue->tail->next = (U32) node;
        queue->tail = node;
    }
    queue->length++;

    mpx_SemaphoreRelease(NET_SEM_ID);

    return 0;
}

S32 mpx_QueuePop(ST_QUEUE* queue, U32* item)
{
    ST_NODE* node;

    if(!queue || !item)
        return -1;

    if(queue->length == 0)
        return -1;

    mpx_SemaphoreWait(NET_SEM_ID);

    node = queue->head;

    if(queue->length == 1)
    {
        queue->head = queue->tail = 0;
    }
    else
    {
        queue->head = (ST_NODE *) node->next;
        queue->head->prev = 0;
    }

    queue->length--;

    mpx_SemaphoreRelease(NET_SEM_ID);

    *item = node->data;
    mpx_Free(node);

    return 0;
}

U32 mpx_QueueItemGet(ST_QUEUE* queue, U32 index)
{
    ST_NODE* node = queue->head;

    if(index >= queue->length)
        return 0;

    while(index)
    {
        node = (ST_NODE *) node->next;
        index--;
    }

    return node->data;
}
