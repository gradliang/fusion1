#ifndef PDBREP_H_
#define PDBREP_H_
/* % -*- mode: Noweb; noweb-code-mode: c-mode -*-                           */
/*                                                                          */
/* % l2h macro module 1 <a href="#$1.html"><tt>#1.nw</tt></a>               */
/*                                                                          */
/* \section{Representation of the generic PDB information}                  */
/*                                                                          */
/* \ifhtml                                                                  */
/* \tableofcontents                                                         */
/* \fi                                                                      */
/*                                                                          */
/* \subsection{Interface}                                                   */
/* We begin with the I/O functions.                                         */
/* [[read_db]] reads in an entire database from a file.                     */
/* Some pieces are uninterpreted [[Text]] and must be dealt with later,     */
/* by specialized converter modules.                                        */
/* [[write_dbinfo]] writes out only the informational header,               */
/* and [[write_record_header]] writes out a record header.                  */
/*                                                                          */
/* <header>=                                                                */
#include "pdbio.h"
#include <time.h>
struct DBInfo; /* defined below */
struct record; /* defined below */
extern struct pdb read_db (FILE *fp); /* reads file, fills in data structures */
extern int write_dbinfo (FILE *fp, struct DBInfo *info,
                         unsigned long appoff, unsigned long sortoff);
         /* writes file, returns # of bytes written */
extern void write_record_header(FILE *fp, struct record *rec, unsigned offset);
/* \subsubsection{Database header}                                          */
/* Here is the standard database information.                               */
/* The [[appInfo]] and [[sortInfo]] fields, as well as the records,         */
/* are application-specific, so they are represented here as arbitrary      */
/* strings of characters (see \module{pdbio} for the definition of [[Text]]). */
/*                                                                          */
/* <header>=                                                                */
struct DBInfo {
  char name[33];                /* name of database, plus null */
  unsigned resource:1;          /* resource DB, not record db */
  unsigned readonly:1;
  unsigned dirtyAppInfo:1;
  unsigned backup:1;    /* no conduit exists; use standard backup */
  unsigned installOver:1;       /* ok to install over open DB */
  unsigned reset:1;             /* force pilot to reset after installing */
  unsigned version;             /* application-dependent */
  time_t createDate,modifyDate,backupDate;
  unsigned long modnum;         /* modification number */
  Text appInfo;
  Text sortInfo;
  char type[5];                 /* 4-byte type, plus trailing null */
  char creator[5];              /* 4-byte creator ID, plus trailing null */
  unsigned long uidSeed;        /* seed for creating unique record IDs */
  unsigned long nextRecordListID; /* set to zero? */
  unsigned short numrecs;       /* number of records */
};
/* The entire database is laid out in the file like this.                   */
/* The role of the filler is a mystery, but we include it here so we can    */
/* read a binary PDB file, convert it, and write back the original          */
/* unchanged---it's a good test.                                            */
/*                                                                          */
/* <header>=                                                                */
typedef struct pdb {
  struct DBInfo info;
  unsigned short filler;
  struct record *records;       /* list of all the records */
  Text image;                   /* the whole file as it appeared on disk */
} DB;
/* \subsubsection{Record header}                                            */
/* Each record has a category and some flag bits in addition to its         */
/* application-dependent data.                                              */
/*                                                                          */
/* <header>=                                                                */
typedef struct record {
  Text data;
  unsigned secret:1;
  unsigned busy:1;
  unsigned dirty:1;
  unsigned delete:1;
  unsigned category:4;
  unsigned uid:24;              /* 24-bit unique id -- zero for new recs */
} Record;
/* \subsubsection{Commonly used category information}                       */
/* Finally, many applications store the same kind of category               */
/* information, so we provide hooks for it here:                            */
/*                                                                          */
/* <header>=                                                                */
struct PDBCategoryAppInfo {
  unsigned int renamed[16]; /* Boolean array of categories with changed names */
  char name[16][16]; /* 16 categories of 15 characters+nul each */
  unsigned char ID[16]; 
  unsigned char lastUniqueID; /* Each category gets a unique ID, for sync tracking
                                 purposes. Those from the Pilot are between 0 & 127.
                                 Those from the PC are between 128 & 255. I'm not
                                 sure what role lastUniqueID plays. */
};
/* We provide read, write, pack, and unpack functions for the category      */
/* stuff.                                                                   */
/*                                                                          */
/* <header>=                                                                */
extern struct PDBCategoryAppInfo catread(Text *);
extern void unpack_categories(FILE *out, struct PDBCategoryAppInfo *cat, int cols);
struct PDBCategoryAppInfo pack_categories(void);
int catwrite(FILE *fp, struct PDBCategoryAppInfo *ai);
#endif
