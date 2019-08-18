/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      hcb.h
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

#ifndef __HCB_H__
#define __HCB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Optimal huffman decoding for AAC taken from:
 *  "SELECTING AN OPTIMAL HUFFMAN DECODER FOR AAC" by
 *  VLADIMIR Z. MESAROVIC , RAGHUNATH RAO, MIROSLAV V. DOKIC, and SACHIN DEO
 *  AES paper 5436
 *
 *   2 methods are used for huffman decoding:
 *   - binary search
 *   - 2-step table lookup
 *
 *   The choice of the "optimal" method is based on the fact that if the
 *   memory size for the Two-step is exorbitantly high then the decision
 *   is Binary search for that codebook. However, for marginally more memory
 *   size, if Twostep outperforms even the best case of Binary then the
 *   decision is Two-step for that codebook.
 *
 *   The following methods are used for the different tables.
 *   codebook   "optimal" method
 *    HCB_1      2-Step
 *    HCB_2      2-Step
 *    HCB_3      Binary
 *    HCB_4      2-Step
 *    HCB_5      Binary
 *    HCB_6      2-Step
 *    HCB_7      Binary
 *    HCB_8      2-Step
 *    HCB_9      Binary
 *    HCB_10     2-Step
 *    HCB_11     2-Step
 *    HCB_SF     Binary
 *
 */


#define ZERO_HCB       0
#define FIRST_PAIR_HCB 5
#define ESC_HCB        11
#define QUAD_LEN       4
#define PAIR_LEN       2
#define NOISE_HCB      13
#define INTENSITY_HCB2 14
#define INTENSITY_HCB  15

/* 1st step table */
typedef struct
{
    BYTE offset;
    BYTE extra_bits;
} hcb;

/* 2nd step table with quadruple data */
typedef struct
{
    BYTE bits;
    int8_t x;
    int8_t y;
} hcb_2_pair;

typedef struct
{
    BYTE bits;
    int8_t x;
    int8_t y;
    int8_t v;
    int8_t w;
} hcb_2_quad;

/* binary search table */
typedef struct
{
    BYTE is_leaf;
    int8_t data[4];
} hcb_bin_quad;

typedef struct
{
    BYTE is_leaf;
    int8_t data[2];
} hcb_bin_pair;

hcb *hw_hcb_table[];
hcb_2_quad *hw_hcb_2_quad_table[];
hcb_2_pair *hw_hcb_2_pair_table[];
hcb_bin_pair *hw_hcb_bin_table[];
BYTE hw_hcbN[];
BYTE hw_unsigned_cb[];
int hw_hcb_2_quad_table_size[];
int hw_hcb_2_pair_table_size[];
int hw_hcb_bin_table_size[];

#include "codebook/hcb_1.h"
#include "codebook/hcb_2.h"
#include "codebook/hcb_3.h"
#include "codebook/hcb_4.h"
#include "codebook/hcb_5.h"
#include "codebook/hcb_6.h"
#include "codebook/hcb_7.h"
#include "codebook/hcb_8.h"
#include "codebook/hcb_9.h"
#include "codebook/hcb_10.h"
#include "codebook/hcb_11.h"
#include "codebook/hcb_sf.h"


#ifdef __cplusplus
}
#endif
#endif
