/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0                
*                                REL-4 Version 4.1.0                
*
********************************************************************************
*
*      File             : typedef.c
*      Purpose          : Basic types.
*
********************************************************************************
*/
#ifndef typedef_h
#define typedef_h  

/* start of USER profile define, made via Richard*/
#define VAD1            /* using VAD1 */
#define CODEC           /* merge coder and decoder */

#define DOWN_TO_13BIT

#ifdef PCSIM
#define REGISTER 
#else                /* form Lx218xm opt */
#define REGISTER register
#endif

#ifndef NULL
#define NULL 0
#endif

//Honda: Function option

#define OPT_DTX 0
#define OPT_475 0    //bit-exact ok
#define OPT_515 0    //bit-exact ok
#define OPT_59  0    //bit-exact ok
#define OPT_67  0    //bit-exact ok
#define OPT_74  0    //bit-exact ok
#define OPT_795 0   //bit-exact ok
#define OPT_102 1     //bit-exact ok
#define OPT_122 0     //bit-exact ok


#define dohoming
#define SRAMLimit


/* end of USER profile define, made via Richard*/

//#undef ORIGINAL_TYPEDEF_H /* define to get "original" ETSI version of typedef.h */
#define ORIGINAL_TYPEDEF_H

#ifdef ORIGINAL_TYPEDEF_H
typedef unsigned char UWord8;
typedef char Word8;
typedef short Word16;
typedef long Word32;
typedef int Flag;
#endif

/* Macro define */

#define MABS(x) ((x ^ (((int)x) >> 31)) - (((int)x) >> 31))



#define rounding
#define jacky

#endif
