#ifndef __LIST_H
#define __LIST_H

/******************************************************************************/
/* Doubly linked list structure used for linking entries.                     */
/******************************************************************************/
typedef struct _LM_LIST_ENTRY {
    struct _LM_LIST_ENTRY *FLink;
    struct _LM_LIST_ENTRY *BLink;
} LM_LIST_ENTRY, *PLM_LIST_ENTRY;

/******************************************************************************/
/* List head structure.                                                       */
/******************************************************************************/
typedef struct {
    LM_LIST_ENTRY   Link;
    unsigned long   EntryCount;
} LM_LIST_CONTAINER, *PLM_LIST_CONTAINER;

#endif
