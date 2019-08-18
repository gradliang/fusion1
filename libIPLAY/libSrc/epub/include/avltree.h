#ifndef _AvlTree_H
#define _AvlTree_H

#include "xmlepub.h"
//typedef int ElementType;
typedef XML_Tag_t *ElementType;

/* START: fig4_35.txt */
        
        #define _AvlTree_H

        struct AvlNode;
        typedef struct AvlNode *Position;
        typedef struct AvlNode *AvlTree;

        AvlTree MakeEmpty( AvlTree T );
        Position Find(char *, AvlTree T );
        Position FindMin( AvlTree T );
        Position FindMax( AvlTree T );
        AvlTree Insert( ElementType X, AvlTree T );
        AvlTree Delete( ElementType X, AvlTree T );
        ElementType Retrieve( Position P );

        #endif  /* _AvlTree_H */
/* END */
		

