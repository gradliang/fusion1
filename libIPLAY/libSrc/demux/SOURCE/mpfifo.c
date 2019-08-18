/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mpfifo.h
*
* Programmer:    Liwu Huang
*             
*
* Created: 	 05/25/2010
*
* Description:
*
*
****************************************************************
*/
#include "global612.h"
#if (VIDEO_ON || AUDIO_ON)

#define LOCAL_DEBUG_ENABLE 0

#include "mptrace.h"
#include "mpfifo.h"

static pts_entry_t pts_entry_init(pts_entry_t * const entry)
{
	entry->next = NULL;
	entry->pts = 0;
	entry->picture_coding_type_d = INVALID_TYPE;
}
static frame_entry_t frame_entry_init(frame_entry_t * const entry)
{
	entry->next = entry->buffer = NULL;
	entry->pts = entry->frame_no = 0;
}

static void pts_entry_print(pts_entry_t * const entry)
{
	mpDebugPrintN("(%d,%s)", entry->pts, CODING_TYPE(entry->picture_coding_type_d));
}
static void frame_entry_print(frame_entry_t * const entry)
{
	//mpDebugPrintN("(%d,%s)", entry->pts, CODING_TYPE(entry->picture_coding_type_d));
}


void pts_entry_t_init_instance(pts_entry_t * const entry)
{
	entry->init = pts_entry_init;
	entry->print = pts_entry_print;
	//entry->init(entry);
}

void frame_entry_t_init_instance(frame_entry_t * const entry)
{
	entry->init = frame_entry_init;
	entry->print = frame_entry_print;
	//entry->init(entry);
}


#define	FIFO_INIT(fifo)	fifo->head = fifo->tail = NULL; fifo->count = 0;
static void pts_fifo_init(pts_fifo_t * const fifo)
{
	FIFO_INIT(fifo);
}
static void frame_fifo_init(frame_fifo_t * const fifo)
{
	FIFO_INIT(fifo);
}

#define	FIFO_ADD(fifo, entry, entry_type)	\
	entry_type * const add_entry = (entry_type *)mem_malloc(sizeof(entry_type));	\
	if (!add_entry)	return 0;	\
	memcpy(add_entry, entry, sizeof(entry_type));	\
	add_entry->next = NULL;	\
	if (fifo->tail) fifo->tail->next = add_entry;	\
	fifo->tail = add_entry;	\
	if (!fifo->head) fifo->head = add_entry;	\
	fifo->count++;	\
	return 1;	\

static int pts_fifo_add(pts_fifo_t * const fifo, const pts_entry_t * const entry)
{
	FIFO_ADD(fifo, entry, pts_entry_t);
}
static int frame_fifo_add(frame_fifo_t * const fifo, const frame_entry_t * const entry)
{
	FIFO_ADD(fifo, entry, frame_entry_t);
}


/*!
	Normal first in first out
	@return fifo 1st entry
*/
#define	FIFO_GET(fifo, entry_type)	\
	entry_type * const entry = fifo->head;	\
	entry_type	ret_entry;	\
	entry_type##_init_instance(&ret_entry);	\
	ret_entry.init(&ret_entry);	\
	if (entry)	\
	{	\
		fifo->head = fifo->head->next;	\
		ret_entry = *entry;	\
		mem_free(entry);	\
		fifo->count--;	\
	}	\
	if (!fifo->head)	fifo->tail = NULL;	\
	ret_entry.next = NULL;	\
	return ret_entry;	\
	
static pts_entry_t pts_fifo_get(pts_fifo_t * const fifo)
{
	FIFO_GET(fifo, pts_entry_t);
}
static frame_entry_t frame_fifo_get(frame_fifo_t * const fifo)
{
	FIFO_GET(fifo, frame_entry_t);
}

/*!
	glance first item, but not remove from fifo
	@return fifo 1st entry
*/
#define	FIFO_GLANCE_HEAD(fifo, entry_type)	\
	entry_type * const entry = fifo->head;	\
	entry_type ret_entry;	\
	entry_type##_init_instance(&ret_entry);	\
	ret_entry.init(&ret_entry);	\
	if (entry)	ret_entry = *entry;	\
	ret_entry.next = NULL;	\
	return ret_entry;	\

static pts_entry_t pts_fifo_glance_head(pts_fifo_t * const fifo)
{
	FIFO_GLANCE_HEAD(fifo, pts_entry_t);
}
static frame_entry_t frame_fifo_glance_head(frame_fifo_t * const fifo)
{
	FIFO_GLANCE_HEAD(fifo, frame_entry_t);
}


/*!
	@return 2nd entry with B_TYPE first, or the 1st entry
*/
static pts_entry_t pts_fifo_get_display_order(pts_fifo_t * const fifo)
{
	pts_entry_t * entry = fifo->head;
	pts_entry_t ret_entry;
	pts_entry_t_init_instance(&ret_entry);
	ret_entry.init(&ret_entry);
	if (entry)
	{
		if (entry->picture_coding_type_d != B_TYPE && entry->next && entry->next->picture_coding_type_d == B_TYPE)
		{
			entry = entry->next;
			fifo->head->next=entry->next;
			ret_entry = *entry;
			if (entry == fifo->tail)	fifo->tail = fifo->head;
		}
		else
		{
			ret_entry = *entry;
			fifo->head = fifo->head->next;
		}
		mem_free(entry);
		fifo->count--;
	}
	if (!fifo->head)	fifo->tail = NULL;
	ret_entry.next = NULL;
	return ret_entry;
}

#define	FIFO_FLUSH(fifo, entry_type)	\
	entry_type * entry;	\
	while (entry = fifo->head)	\
	{	\
		fifo->head = fifo->head->next;	\
		mem_free(entry);	\
	}	\
	fifo->init(fifo);	\

static void pts_fifo_flush(pts_fifo_t * const fifo)
{
	MP_DEBUG("pts_fifo_flush");
	FIFO_FLUSH(fifo, pts_entry_t);
}
static void frame_fifo_flush(frame_fifo_t * const fifo)
{
	MP_DEBUG("frame_fifo_flush");
	FIFO_FLUSH(fifo, frame_entry_t);
}

#define	FIFO_PRINTALL(fifo, entry_type)	\
	entry_type * entry = fifo->head;	\
	while(entry)	\
	{	\
		entry->print(entry);	\
		entry = entry->next;		\
	}	\
	mpDebugPrintN("\r\n");	\

static void pts_fifo_printall(pts_fifo_t * const fifo)
{
	mpDebugPrintN("pts_fifo contain %d: ", fifo->count);
	FIFO_PRINTALL(fifo, pts_entry_t);
}
static void frame_fifo_printall(frame_fifo_t * const fifo)
{
	mpDebugPrintN("frame_fifo contain %d: ", fifo->count);
	FIFO_PRINTALL(fifo, frame_entry_t);
}

#define	FIFO_GET_COUNT(fifo)	return fifo->count;
static int pts_fifo_get_count(pts_fifo_t * const fifo)
{
	FIFO_GET_COUNT(fifo);
}
static int frame_fifo_get_count(frame_fifo_t * const fifo)
{
	FIFO_GET_COUNT(fifo);
}


#define	FIFO_INIT_INSTANCE(fifo, prefix)	\
	fifo->init				=	prefix##_init;	\
	fifo->add				=	prefix##_add;	\
	fifo->get				=	prefix##_get;	\
	fifo->glance_head		=	prefix##_glance_head;	\
	fifo->flush				=	prefix##_flush;	\
	fifo->printall			=	prefix##_printall;	\
	fifo->get_count			=	prefix##_get_count;	\
	fifo->init(fifo);	\

void pts_fifo_init_instance(pts_fifo_t * const fifo)
{
	fifo->get_display_order	=	pts_fifo_get_display_order;
	FIFO_INIT_INSTANCE(fifo, pts_fifo);
}
void frame_fifo_init_instance(frame_fifo_t * const fifo)
{
	FIFO_INIT_INSTANCE(fifo, frame_fifo);
}

#endif
