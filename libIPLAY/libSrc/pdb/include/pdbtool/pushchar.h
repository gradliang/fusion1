#ifndef PUSHCHAR_H_
#define PUSHCHAR_H_
/* % -*- mode: Noweb; noweb-code-mode: c-mode -*-                           */
/*                                                                          */
/* \section{Temporary null termination of strings}                          */
/* This hack allows you to put a zero after a string \emph{temporarily},    */
/* i.e., just long enough to use [[printf]].                                */
/* Calling [[pushchar(p)]] is equivalent to writing \mbox{[[*p = '\0']]},   */
/* except that calling [[popchar()]] undoes the effect of the last          */
/* [[pushchar]].                                                            */
/* The stack is shallow, and the program halts with an assertion failure    */
/* if it overflows.                                                         */
/*                                                                          */
/* <header>=                                                                */
extern void pushchar(char *p);
extern void popchar(void);
#endif
