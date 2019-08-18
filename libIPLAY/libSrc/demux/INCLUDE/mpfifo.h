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

#ifndef __MP_FIFO_H_
#define	__MP_FIFO_H_

//avoid different definition from different codec standard, define here only for display
typedef enum {INVALID_TYPE=-999,I_TYPE=0, P_TYPE=1, B_TYPE=2} picture_coding_type_d_T;
#define	CODING_TYPE(type)	((type)==I_TYPE?"I":((type)==P_TYPE?"P":((type)==B_TYPE?"B":"O")))

#define	FIFO_T(fifo_type, entry_type)	\
	entry_type *head, *tail;	\
	int count;	\
	void		(*init)			(fifo_type * const fifo);	\
	int			(*add)			(fifo_type * const fifo, const entry_type * const entry);	\
	entry_type	(*get)			(fifo_type * const fifo);	\
	entry_type	(*glance_head)	(fifo_type * const fifo);	\
	void		(*flush)		(fifo_type * const fifo);	\
	void 		(*printall)		(fifo_type * const fifo);	\
	int			(*get_count)	(fifo_type * const fifo);	\

#define	ENTRY_T(entry_type)	\
	entry_type *next;	\
	unsigned int pts;	\
	entry_type	(*init)	(entry_type * const entry);	\
	void 		(*print)(entry_type * const entry);	\

typedef struct _pts_entry
{
	ENTRY_T(struct _pts_entry);
	picture_coding_type_d_T picture_coding_type_d;
} pts_entry_t;
typedef struct _pts_fifo_t
{
	pts_entry_t (*get_display_order)	(struct _pts_fifo_t * const fifo);
	FIFO_T(struct _pts_fifo_t, pts_entry_t);
} pts_fifo_t;

typedef struct _frame_entry
{
	ENTRY_T(struct _frame_entry);
	void *buffer;
	unsigned int frame_no;//buffer number of video_mpi
} frame_entry_t;
typedef struct _frame_fifo_t
{
	FIFO_T(struct _frame_fifo_t, frame_entry_t);
} frame_fifo_t;


void pts_entry_t_init_instance(pts_entry_t * const entry);
void frame_entry_t_init_instance(frame_entry_t * const entry);
void pts_fifo_init_instance(pts_fifo_t * const fifo);
void frame_fifo_init_instance(frame_fifo_t * const fifo);


#endif

