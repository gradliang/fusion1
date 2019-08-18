#ifndef PDBCONV_H_
#define PDBCONV_H_
/* % -*- mode: Noweb; noweb-code-mode: c-mode -*-                           */
/*                                                                          */
/* \section{Information for converting PDB files}                           */
/*                                                                          */
/* You have to define a [[struct PDBConv]] for every specialized PDB        */
/* file.                                                                    */
/* It contains functions that know how to pack and unpack three             */
/* things:                                                                  */
/* the application info, the sort area, and a record.                       */
/* It also contains documents ([[char **]]) that explain to somebody        */
/* reading the ASCII what the particular conventions are.                   */
/* It is particularly important to document packing, because many of the    */
/* fields written out by the unpacker are probably optional.                */
/*                                                                          */
/* We recognize a file by its type and creator, so the converter must       */
/* contain those too, to indicate what it applies to.                       */
/*                                                                          */
/* Pack methods will be called twice: once with null argument, and once     */
/* without.                                                                 */
/* In both cases they must return the number of bytes that would be (or are) written. */
/*                                                                          */
/* <header>=                                                                */
#include "pdbio.h"
struct pdb;                     /* pointer to PDB database */

typedef struct PDBConv {
  char *application;            /* identifying noun for comment */
  char type[5];                 /* includes null terminator */
  char creator[5];              /* includes null terminator */
  struct infoconv {
    char **docs;
    void (*unpack)(FILE *lua, struct pdb *db, Text image);
                                /* read image from pdb file, write Lua code */
    int (*pack)(FILE *pdb, int appinfo);
                                /* read info from Lua value Database, write if pdb 
                                   not null, and return number of bytes written.
                                   appinfo is nonzero for application info,
                                   zero for sort info */
  } appInfo, sortInfo;
  struct recordconv {
    char **docs;
    void (*unpack)(FILE *lua, struct pdb *db, int recnum);
                                /* read image db record area, write Lua code */
    int (*pack)(FILE *pdb, int recnum);
                                /* read info from Lua value Database,
                                   write if pdb not null,
                                   and return number of bytes written */
  } record;
  char **trailers;              /* useful functions or whatever */
} *PDBConverter;
/* This module includes support for packing and unpacking dates.            */
/*                                                                          */
/* <header>=                                                                */
#include <time.h>
#include "pdblua.h"
extern void unpack_date(FILE *out, time_t secs);
extern time_t pack_date(Ref o);
extern char **date_docs;
/* It also provides a generic converter, which uses these generic packing   */
/* and unpacking functions.                                                 */
/*                                                                          */
/* <header>=                                                                */
extern PDBConverter generic_convert;
extern void generic_unpackInfo  (FILE *lua, struct pdb *db, Text image);
extern void generic_unpackRecord(FILE *lua, struct pdb *db, int recnum);
extern int generic_packInfo  (FILE *pdb, int appinfo);
extern int generic_packRecord(FILE *pdb, int recnum);
#endif
