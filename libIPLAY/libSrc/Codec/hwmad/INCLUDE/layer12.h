/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      layer12.h
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

# ifndef LIBMAD_LAYER12_H
# define LIBMAD_LAYER12_H

//# include "stream.h"
# include "frame.h"

int mad_layer_I(struct mad_stream *, struct mad_frame *,unsigned char*);
int mad_layer_II(struct mad_stream *, struct mad_frame *,unsigned char*);

# endif
