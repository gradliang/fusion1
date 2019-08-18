#ifndef PDBIO_H_
#define PDBIO_H_
/* % -*- mode: Noweb; noweb-code-mode: c-mode -*-                           */
/*                                                                          */
/* \section{Input and output for Pilot Data Base conversion}                */
/*                                                                          */
/* \ifhtml                                                                  */
/* \tableofcontents                                                         */
/* \fi                                                                      */
/*                                                                          */
/* Reading a Pilot database pretty much requires random access, so we       */
/* read the entire thing into a [[Text]] and take it from there.            */
/* Writing it, on the other hand, can be done sequentially (provided        */
/* we're willing to make two passes), so we just                            */
/* use an ordinary file.                                                    */
/*                                                                          */
/* \subsection{Interface}                                                   */
/*                                                                          */
/* A [[Text]] is a string possibly containing zeroes, plus a ``current      */
/* pointer'' that marks a position to read from.                            */
/* It's often useful to check how many characters have been read or         */
/* remain to be read.                                                       */
/*                                                                          */
/* <header>=                                                                */

#include "global612.h"

typedef STREAM FILE;
#define putc pdb_putc
#define getc pdb_getc
#define fprintf pdb_fprintf
typedef struct text {
  char *chars;                  /* pointer to entire text */
  unsigned len;                 /* number of characters in text */
  unsigned char *p;             /* current pointer (changed by i/o) */
} Text;
#define chars_used(T)      ((char *)(T)->p - (T)->chars)
#define chars_remaining(T) ((T)->len - chars_used(T))
/* You can build a [[Text]] by reading in an entire file, or by giving a    */
/* buffer and length.                                                       */
/*                                                                          */
/* <header>=                                                                */
//#include <stdio.h>
extern Text readfile(FILE *fp); /* read all chars left in file */
extern Text text(char *chars, unsigned len); /* create a new one */
/* I/O functions have to modify the current pointer, so they work on a      */
/* pointer to a text.  All of them arrange to call the [[iofail]] macro     */
/* if the [[Text]] is exhausted.                                            */
/* [[iofail]] is a macro so it can give the source location of the          */
/* offending read.                                                          */
/*                                                                          */
/* <header>=                                                                */
#include <stdlib.h>
#define iofail(T,N) \
  ((void)(chars_remaining(T) >= (N) ? 0 : \
      (fprintf(stderr, "%s:%d: exhausted character buffer\n",__FILE__, __LINE__), \
       abort(), 0)))
    
#define get_byte(T)  (iofail(T,1), *(T)->p++)
#define get_short(T) (io_btmp = get_byte(T),  io_btmp <<  8 | get_byte(T))
#define get_long(T)  (io_stmp = get_short(T), io_stmp << 16 | get_short(T))
#define get_chars(D,S,N) (iofail(S,N),get_charsf(D,S,N))
#define get_array(A,T) get_chars(A,T,sizeof(A))
extern void get_charsf(void *dst, Text *src, int n);

#define put_byte(F, N) (putc(N, F) != EOF ? (int)(unsigned char)(N) : (assert(0), EOF))
extern void put_short(FILE *, unsigned short);
extern void put_long (FILE *, unsigned long);
extern void put_chars(FILE *dst, void *src, int n);
#define put_block(DST, SRC) put_chars(DST, SRC, sizeof(SRC))
/* The macros use the byte order of the Pilot (big-endian), regardless      */
/* of the byte order of the host machine.                                   */
/* These two temporaries are used to simplify ordering:                     */
/*                                                                          */
/* <header>=                                                                */
/* temps for macros */
extern unsigned char  io_btmp;
extern unsigned short io_stmp;
#endif
