/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      layer3.h
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Brenda Li    first file
****************************************************************
*/

# ifndef LIBMAD_LAYER3_H
# define LIBMAD_LAYER3_H

//# include "stream.h"
# include "frame.h"

int mad_layer_III(struct mad_stream *, struct mad_frame *,unsigned char*);

static inline void SetRegisterBits ( unsigned int reg, 
                             unsigned int bit_mask, 
                             unsigned int value )  
{ 
    *((unsigned int*)reg) = (*((unsigned int*)reg) & (~bit_mask) ) | (value & bit_mask);
}


# endif
