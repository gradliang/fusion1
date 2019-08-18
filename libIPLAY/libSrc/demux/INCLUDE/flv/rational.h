/*****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2009, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      rational.h
*
* Programmer:    Mingfen lin
*                MPX E360 division
*
* Created: 	 04/28/2009
*
* Description: AV util rational include file :  rational numbers
*              
*        
* Change History (most recent first):
*     <1>     04/28/2009    Mingfen Lin    first file
*****************************************************************/

#ifndef AVUTIL_RATIONAL_H
#define AVUTIL_RATIONAL_H

//#include <stdint.h>
//#include "common.h"

/**
 * rational number numerator/denominator
 */
typedef struct AVRational{
    int num; ///< numerator
    int den; ///< denominator
} AVRational;
#if 0
/**
 * Compares two rationals.
 * @param a first rational
 * @param b second rational
 * @return 0 if a==b, 1 if a>b and -1 if a<b
 */
static inline int av_cmp_q(AVRational a, AVRational b){
    const int64_t tmp= a.num * (int64_t)b.den - b.num * (int64_t)a.den;

    if(tmp) return (tmp>>63)|1;
    else    return 0;
}

/**
 * Converts rational to double.
 * @param a rational to convert
 * @return (double) a
 */
static inline double av_q2d(AVRational a){
    return a.num / (double) a.den;
}

/**
 * Reduces a fraction.
 * This is useful for framerate calculations.
 * @param dst_num destination numerator
 * @param dst_den destination denominator
 * @param num source numerator
 * @param den source denominator
 * @param max the maximum allowed for dst_num & dst_den
 * @return 1 if exact, 0 otherwise
 */
int av_reduce(int *dst_num, int *dst_den, int64_t num, int64_t den, int64_t max);

/**
 * Multiplies two rationals.
 * @param b first rational
 * @param c second rational
 * @return b*c
 */
AVRational av_mul_q(AVRational b, AVRational c) av_const;

/**
 * Divides one rational by another.
 * @param b first rational
 * @param c second rational
 * @return b/c
 */
AVRational av_div_q(AVRational b, AVRational c) av_const;

/**
 * Adds two rationals.
 * @param b first rational
 * @param c second rational
 * @return b+c
 */
AVRational av_add_q(AVRational b, AVRational c) av_const;

/**
 * Subtracts one rational from another.
 * @param b first rational
 * @param c second rational
 * @return b-c
 */
AVRational av_sub_q(AVRational b, AVRational c) av_const;

/**
 * Converts a double precision floating point number to a rational.
 * @param d double to convert
 * @param max the maximum allowed numerator and denominator
 * @return (AVRational) d
 */
AVRational av_d2q(double d, int max) av_const;

/**
 * @return 1 if \q1 is nearer to \p q than \p q2, -1 if \p q2 is nearer
 * than \p q1, 0 if they have the same distance.
 */
int av_nearer_q(AVRational q, AVRational q1, AVRational q2);

/**
 * Finds the nearest value in \p q_list to \p q.
 * @param q_list an array of rationals terminated by {0, 0}
 * @return the index of the nearest value found in the array
 */
int av_find_nearest_q_idx(AVRational q, const AVRational* q_list);
#endif
#endif /* AVUTIL_RATIONAL_H */
