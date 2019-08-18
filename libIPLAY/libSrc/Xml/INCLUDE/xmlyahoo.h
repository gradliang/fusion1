#ifndef __xml_youtube_h__
#define __xml_youtube_h__

#include "netware.h"

#define MAX_TITAL       4
#define MAX_USERNAME    64
#define MAX_URL         256
#define MAX_TIME        16


char title_preview[MAX_TITAL][MAX_URL];

typedef struct _class_entry    class_entry_t;
typedef struct _yahoo_info     yahoo_info_t;
struct _class_entry
{
    class_entry_t  *next;
    char title[MAX_USERNAME];
    char href[MAX_USERNAME];
    char td[MAX_USERNAME];
    char span[MAX_USERNAME];
    char vol[MAX_USERNAME];
    char percent[MAX_USERNAME];
    char measure[MAX_USERNAME];
};

enum _YAHOO_STATE
{
    YAHOO_NULL,
    YAHOO_ERROR,
    YAHOO_FIND_TITAL,
    YAHOO_FIND_TBODY,
    YAHOO_SHOCK,
    YAHOO_SHOCK_ID,
    YAHOO_SHOCK_TIME,
    YAHOO_SHOCK_BARGAIN,
    YAHOO_SHOCK_INVEST,
    YAHOO_SHOCK_SALE,
    YAHOO_SHOCK_FLUCTUATION,
    YAHOO_SHOCK_PIECE,
    YAHOO_SHOCK_PAST,
    YAHOO_SHOCK_OPENING,
    YAHOO_SHOCK_TIPTOP,
    YAHOO_SHOCK_LOWEST,
};

struct _yahoo_info
{
    enum _YAHOO_STATE  state;
    char  lang[MAX_USERNAME];//language;
    class_entry_t       class_list;
    class_entry_t       *cur_class;
	char  id[MAX_USERNAME];//ID;
	char  time[MAX_TIME];//TIME;
	char  bargain[MAX_TIME];//Bargain;
	char  invest[MAX_TIME];//Invest;
	char  sale[MAX_TIME];//Sale;
	char  fluctuation[MAX_TIME];//Fluctuation;
	char  piece[MAX_TIME];//Piece;
	char  past[MAX_TIME];//Past;
	char  opening[MAX_TIME];//Opening;
	char  tiptop[MAX_TIME];//Tiptop;
	char  lowest[MAX_TIME];//Lowest;

};

#endif
