#ifndef PDBLUA_H_
#define PDBLUA_H_
/* % -*- mode: Noweb; noweb-code-mode: c-mode -*-                           */
/*                                                                          */
/* \section{Lua helper functions for PDB conversion}                        */
/*                                                                          */
/* \ifhtml                                                                  */
/* \tableofcontents                                                         */
/* \fi                                                                      */
/*                                                                          */
/* \subsection{Interface}                                                   */
/* [[quote]] writes a string that Lua can read back, or [[nil]] for a       */
/* null pointer.                                                            */
/* If [[emptyAsNil]] is nonzero, it also writes the empty string as [[nil]]. */
/*                                                                          */
/* <header>=                                                                */
//#include <stdio.h>
#include <lua.h>

extern char *quote(char *s, int emptyAsNil);
#define QUOTE(s) quote(s, 0)
/* [[comma]] helps put commas between list elements without having them     */
/* at the ends of lists.                                                    */
/* [[bitstring]] converts a single bit to a string representing a           */
/* suitable Lua value.                                                      */
/*                                                                          */
/* <header>=                                                                */
#define comma(P) ((P) ? ", " : "")
#define bitstring(B) ((B) ? "1" : "nil")
/* [[write_docs]] prints out indented comments.                             */
/* It takes care of the initial [[--]] and trailing newline.                */
/*                                                                          */
/* <header>=                                                                */
//extern void write_docs(FILE *fp, int indent, char **lines);
/* This group of macros is useful for ``packing'' Lua values into           */
/* internal form.                                                           */
/* They are all macros so that [[S]] can be a literal string combined       */
/* with [["return "]].                                                      */
/* This means if you want to use a non-literal [[char *]] value,  you       */
/* have to use the function ([[*f]]) forms.                                 */
/*                                                                          */
/* <header>=                                                                */
#define objectval(S) myref(objectvalf("return " S, S))
#define objectvalxx(S) objectvalf("return " S, S)
#define bitval(S) (!lua_isnil(objectvalxx(S)))
#define numberval(S) numbervalf(objectvalxx(S), S)
#define numberopt(S,N) numberoptf(objectvalxx(S),N, S)
#define intval(S) intvalf(objectvalxx(S), S)
#define intopt(S,N) intoptf(objectvalxx(S),N, S)
#define stringval(S) stringvalf(objectvalxx(S), S)
#define stringopt(S,N) stringoptf(objectvalxx(S),N, S)
#define setstring(L, S, SIZE) setstringf(L, objectvalxx(S), SIZE, NULL, S)
#define setstringopt(L, S, OPT, SIZE) setstringf(L, objectvalxx(S), SIZE, OPT, S)
#define setblock(L, S) setstring(L, S, sizeof(L))
#define setblockopt(L, S, OPT) setstringopt(L, S, OPT, sizeof(L))
/* Macros with [[opt]] suffixes allow a value to be missing (i.e.,          */
/* nil), and they take an extra argument that is the value to be returned   */
/* in that case.  [[setstring]] is used to get string values of limited     */
/* size, and [[setblock]] is a special case that makes it easy to pack      */
/* fixed-size arrays.                                                       */
/*                                                                          */
/*                                                                          */
/* Here are the function versions of these macros.  Don't call them         */
/* unless you know what you're doing.                                       */
/*                                                                          */
/* <header>=                                                                */
extern lua_Object objectvalf(char *stmt, char *expression);
  /* never call this without wrapping in a ref */
extern float numbervalf(lua_Object o, char *expression);
extern float numberoptf(lua_Object o, float x, char *expression);
extern int intvalf(lua_Object o, char *expression);
extern int intoptf(lua_Object o, int n, char *expression);
extern char *stringvalf(lua_Object o, char *expression);
extern char *stringoptf(lua_Object o, char *s, char *expression);
extern void setstringf(void *dst, lua_Object o, int n, char *optional, char *exp);
/* [[dbcheck]] complains with an error message unless the Lua               */
/* expression~[[E]] is true.                                                */
/*                                                                          */
/* <header>=                                                                */
#define dbcheck(E, MSG) dbcheckf("return " E, MSG)
extern void dbcheckf(char *lua_boolean, char *errmsg);
/* These macros track the locations of Lua ``begin blocks,'' so if you      */
/* forget to close a block, you can find out where the unclosed block was   */
/* opened.                                                                  */
/*                                                                          */
/* <header>=                                                                */
extern char **blockfiles;
extern int *blocklines;
#define lua_beginblock() (*blockfiles++ = __FILE__, *blocklines++ = __LINE__, \
                          lua_beginblock())
#define lua_endblock() (--blockfiles, --blocklines, lua_endblock())
extern void showall(void);
/* \subsection{Implementation}                                              */
/*                                                                          */
/*                                                                          */
/* \subsubsection{Refs hacking}                                             */
/* This was an unnecessary hack used when debugging.  I was trying to see   */
/* if objects got lost, so I made ``refs'' to them.  Don't define           */
/* [[USEREFS]].                                                             */
/*                                                                          */
/* <header>=                                                                */
#ifdef USEREFS
typedef struct myref { int ref; } Ref;
extern Ref myref(lua_Object o);
#define deref(R) lua_getref((R).ref)
#else
typedef lua_Object Ref;
#define myref(_) (_)
#define deref(_) (_)
#define my_unref(_) ((void)(_))
#endif
#endif
