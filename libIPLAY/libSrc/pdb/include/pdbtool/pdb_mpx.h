#ifndef PDB_MPX__H                                            
#define PDB_MPX__H

#define PDB_DEBUG 0 //if PDB_DEBUG 1 ,then output file

enum
{
	PDB_NAME = 0,
	PDB_TYPE,
	PDB_CREATOR,
	PDB_DATA,
};


typedef struct _PDB_INFO PDB_INFO_t;
typedef struct _PDB_DATA PDB_DATA_t;

struct _PDB_DATA
{
	int numrecs;
	char *data;
	int datalen;
	PDB_DATA_t *pre,*next;
};

struct _PDB_INFO
{
	int total_numrecs;	
	
	char *pdb_name;
	char *pdb_type;
	char *pdb_creator;
	
	PDB_DATA_t tag_pdb;
	PDB_DATA_t *cur_pdb;
};
	
#endif //PDB_MPX__H 

