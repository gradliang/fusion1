/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      huffman.h
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

#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

static INLINE void huffman_sign_bits(bitfile *ld, int16_t *sp, BYTE len);
static INLINE int16_t huffman_getescape(bitfile *ld, int16_t sp);
static BYTE huffman_2step_quad(BYTE cb, bitfile *ld, int16_t *sp);
static BYTE huffman_2step_quad_sign(BYTE cb, bitfile *ld, int16_t *sp);
static BYTE huffman_2step_pair(BYTE cb, bitfile *ld, int16_t *sp);
static huffman_2step_pair_sign(BYTE cb, bitfile *ld, int16_t *sp);
static BYTE huffman_binary_quad(BYTE cb, bitfile *ld, int16_t *sp);
static BYTE huffman_binary_quad_sign(BYTE cb, bitfile *ld, int16_t *sp);
static BYTE huffman_binary_pair(BYTE cb, bitfile *ld, int16_t *sp);
static BYTE huffman_binary_pair_sign(BYTE cb, bitfile *ld, int16_t *sp);
static int16_t huffman_codebook(BYTE i);

int8_t hw_huffman_scale_factor(bitfile *ld);
BYTE hw_huffman_spectral_data(BYTE cb, bitfile *ld, int16_t *sp);

#endif
