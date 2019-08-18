
#ifndef __xml_word_h__
#define __xml_word_h__

//#include "global612.h"

#define MAX_DESC        8192

#define MIN(a,b)		((a) < (b) ? (a) : (b))

typedef struct _XML_info XML_info_t;
typedef struct _XMLCHAPTER XMLCHAPTER_t;
typedef struct _XMLData XMLData_t;
//typedef struct _XMLIMG XMLIMG_t;
typedef struct _XML_BUFF_link _XML_BUFF_link_t;
typedef struct _XML_WORD XML_WORD_t;

enum _XML_STATE
{
	XML_NULL,
	XML_BOOK,
	XML_CHAPTER,
	XML_TITLE,
	XML_FONT,
	XML_PARA,
	XML_IMG,
		
	//Table
	XML_INFOTABLE,
	XML_TGROUP,
	XML_TBODY,
	XML_ROW,
	XML_ENTRY,
	//Order List
	XML_ORDERLIST,
	XML_LISTITEM,
	//SECTTION
	XML_SECT1,
	XML_SECT2,
	XML_SECT3,
	XML_SECT4,
	XML_SECT5,
	//ITEMIZEDLIST
	XML_ITEMIZEDLIST,

	XML_PAGE, //for PDF
	 	
	//for LRF
	XML_LRF_PAGE,
	XML_LRF_HEADER_BLOCK,	
	XML_LRF_BLOCK, 
	XML_LRF_BLOCK_FONT,
	XML_LRF_BLOCK_PARA, 		
	XML_LRF_BLOCK_PRAR_FONT,	
	XML_LRF_BLOCK_IMG,         
};

struct _XMLData
{
	int tagnum;
		
	int fontSize;
	char  fontstyle[32];
	char  fontColor[12];
	char  fontAttr[32];
	
	char frame[12]; //table frame
	int colsep; //table collumn
	int rowsep; //table row
	int cols; //table cols

	char numeration[16]; //orderedlist  frame
	char inheritnum[16]; //inheritnum frame
	char continuation[16]; //continuation frame

	char mark[16]; //itemizedlist mark 
	
	int strlength;
	char *strdata;
	int pagenum; 

	//image
	char imglink[16];
	int 	imgwidth;
	int 	imgheight;
	int 	imghorsizescaled;
	int 	imgversizescaled;
	//LRF
	char imgrect[32];
	char imgsize[32];
	
	XMLData_t *data;	
};

struct _XMLCHAPTER
{
	int tagnum; 
	
	int state;
	
	XMLData_t *data;
	XMLCHAPTER_t *pre,*next;
};

struct _XML_info
{
	enum _XML_STATE  state;	
	
    	int	error_code;
	XMLCHAPTER_t	 tag_list;	
      	XMLCHAPTER_t	 *cur_tag;
		
   	XMLData_t *cur_data;
	unsigned char font[MAX_DESC];
};

struct _XML_BUFF_link
{
	unsigned char BUFF[MAX_DESC];
       int buff_len;
	_XML_BUFF_link_t *link;
};

struct _XML_WORD
{
	int total_len;
	_XML_BUFF_link_t *XML_BUF,*ptr;
};

#endif































