#ifndef TXT_MPX__H                                            
#define TXT_MPX__H

#define TXT_DEBUG 0 //if TXT_DEBUG 1 ,then output file

enum
{
	TXT_NAME = 0,
	TXT_TYPE,
	TXT_CREATOR,
	TXT_DATA,
};


typedef struct _TXT_INFO TXT_INFO_t;
typedef struct _TXT_DATA TXT_DATA_t;

struct _TXT_DATA
{
	int numrecs;
	char *data;
	int datalen;
	TXT_DATA_t *pre,*next;
};

struct _TXT_INFO
{
	int total_numrecs;	
	
	char *txt_name;
	char *txt_type;
	char *txt_creator;
	
	TXT_DATA_t tag_txt;
	TXT_DATA_t *cur_txt;
};
	
#endif //TXT_MPX__H 

