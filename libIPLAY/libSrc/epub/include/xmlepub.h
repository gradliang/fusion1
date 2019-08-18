#ifndef XMLEPUB__H                                            
#define XMLEPUB__H     

#include "..\..\epub\include\epub_shared.h"
#include "Epublib.h"      

#define EPUB_DEBUG 0 //if EPUB_DEBUG 1 ,then output file
#define EPUB_MASK  1 //if EPUB_MASK 1 ,then support mask

#define MAX_DESC        8192*2 

typedef struct _XML_Epub_info 	XML_Epub_info_t;
typedef struct _XML_Iterator 	XML_Iterator_t;
typedef struct _XML_NAV 		XML_NAV_t;
typedef struct _XML_Matadata 	XML_Matadata_t;
typedef struct _XML_Data 		XML_Data_t;
typedef struct _XML_Tag			XML_Tag_t;
typedef struct _XML_Mask  		XML_Mask;
typedef struct _XML_Font		XML_Font_t;
	
struct _XML_Mask 
{
	short x;
	short y;
	BYTE mask;
};

enum _XML_EPUB_STATE
{
	XML_NULL = 0,
	XML_A,
	XML_ABBR,
	XML_ACRONYM,
	XML_ADDRESS,
	XML_APPLET,
	XML_AREA,
	XML_B,
	XML_BASE,
	XML_BASEFONT,      
	XML_BDO,                //10
	XML_BIG,
	XML_BLOCKQUOTE,
	XML_BODY,
	XML_BR,
	XML_BUTTON,
	XML_CAPTION,
	XML_CENTER,
	XML_CITE,
	XML_CODE,
	XML_COL,                //20
	XML_COLGROUP,
	XML_DD,
	XML_DEL,
	XML_DFN,
	XML_DIR,
	XML_DIV,
	XML_DL,
	XML_DT,
	XML_EM,
	XML_FIELDSET,         //30
	XML_FONT,
	XML_FORM,
	XML_FRAME,
	XML_FRAMESET,
	XML_HEAD,
	XML_H1,
	XML_H2,
	XML_H3,
	XML_H4,
	XML_H5,                   //40
	XML_H6,
	XML_HR,
	XML_HTML,
	XML_I,
	XML_IFRAME,
	XML_IMG,
	XML_INPUT,
	XML_INS,
	XML_KBD,
	XML_LABEL,            //50
	XML_LEGEND,
	XML_LI,
	XML_LINK,
	XML_MAP,
	XML_MENU,
	XML_META,
	XML_NOFRAMES,
	XML_NOSCRIPT,
	XML_OBJECT,
	XML_OL,                   //60
	XML_OPTGROUP,
	XML_OPTION,
	XML_P,
	XML_PARAM,
	XML_PRE,
	XML_Q,
	XML_S,
	XML_SAMP,
	XML_SCRIPT,
	XML_SELECT,            //70
	XML_SMALL,
	XML_SPAN,
	XML_STRIKE,
	XML_STRONG,
	XML_STYLE,
	XML_SUB,
	XML_SUP,
	XML_TABLE,
	XML_TBODY,
	XML_TD,                    //80
	XML_TEXTAREA,
	XML_TFOOT,
	XML_TH,
	XML_THEAD,
	XML_TITLE,
	XML_TR,
	XML_TT,
	XML_U,
	XML_UL,
	XML_VAR,                   //90
	XML_TOTAL
};

struct _XML_Tag
{	
	char *tag;
	int tagnum;
};

		struct _a
		{
			//Optional Attributes
			char * charset;
			char * coords;
			char * href;
			char * hreflang;
			char * name;
			char * rel;
			char * rev;
			char * shape;
			char * target;

			//Standard Attributes
			char * accesskey;
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * tabindex;
			char * title;
			char * xml_lang;
		};

		struct _abbr
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _acronym
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _address
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _applet
		{
			//Required Attributes
			char * code;
			char * object;

			//Optional Attributes
			char * align;
			char * alt;
			char * archive;
			char * codebase;
			char * height;
			char * hspace;
			char * name;
			char * vspace;
			char * width;
			
			//Standard Attributes
			char * class;
			char * id;
			char * style;
			char * title;			
		};		
		
		struct _area
		{
			//Required Attributes
			char * alt;
			//Required Attributes
			char * coords;
			char * href;
			char * nohref;
			char * shape;
			char * target;
			//Standard Attributes
			char * accesskey;
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * tabindex;
			char * title;
			char * xml_lang;
		};
	
		struct _tt
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};


		struct _i
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _b
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _big
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		
		struct _small
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
			
		struct _base
		{
			//Attributes
			char * href;
			char * target;
		};
		
		struct _basefont
		{	
			//Optional Attributes
			char * color;
			char * face;
			char * size;
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;			
		};
		
		struct _bdo
		{
			//Required Attributes
			char * dir;
			//Standard Attributes
			char * class;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _blockquote
		{
			//Optional Attributes
			char * cite;
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _body
		{
			//Optional Attributes
			char * alink;
			char * background;
			char * bgcolor;
			char * link;
			char * text;
			char * vlink;
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;			
		};

		struct _br
		{
			//Standard Attributes
			char * class;
			char * id;
			char * style;
			char * title;
		};

		struct _button
		{
			//Optional Attributes
			char * disabled;
			char * name;
			char * type;
			char * value;
			//Standard Attributes
			char * accesskey;
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * tabindex;
			char * title;
			char * xml_lang;
		};

		struct _caption
		{
			//Optional Attributes
			char * align;
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};

		struct _center
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
		};

		struct _em
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};

		struct _strong
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};
		
		struct _dfn
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};
		
		struct _code
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};
		
		struct _samp
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};
		
		struct _kbd
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};
		
		struct _cite
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};
		
		struct _col
		{
			//Optional Attributes
			char * align;
			char * charc;
			char * charoff;
			char * span;
			char * valign;
			char * width;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};

		struct _colgroup
		{
			//Optional Attributes
			char * align;
			char * charc;
			char * charoff;
			char * span;
			char * valign;
			char * width;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};

		struct _dd
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};
		
		struct _del
		{
			//Optional Attributes
			char * cite;
			char * datetime;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;		
		};	

		struct _dir
		{
			//Optional Attributes
			char * compact;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
		};	

		struct _div
		{
			//Optional Attributes
			char * align;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;	
		};	

		struct _dl
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;	
		};

		struct _dt
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;	
		};

		struct _fieldset
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;	
		};

		struct _font
		{
			//Optional Attributes
			char * color;
			char * face;
			char * size;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
		};

		struct _form
		{
			//Required Attributes
			char * action;

			//Optional Attributes
			char * accept;
			char * accept_charset;
			char * enctype;
			char * method;
			char * name;
			char * target;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _frame
		{
			//Optional Attributes
			char * frameborder;
			char * longdesc;
			char * marginheight;
			char * marginwidth;
			char * name;
			char * noresize;
			char * scrolling;
			char * src;
			
			//Standard Attributes
			char * class;
			char * id;
			char * style;
			char * title;
		};

		struct _frameset
		{
			//Optional Attributes
			char * cols;
			char * rows;
			
			//Standard Attributes
			char * class;
			char * id;
			char * style;
			char * title;
		};

		struct _head
		{
			//Optional Attributes
			char * profile;
			
			//Standard Attributes
			char * dir;
			char * lang;
			char * xml_lang;
		};

		struct _h1
		{
			//Optional Attributes
			char * align;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _h2
		{
			//Optional Attributes
			char * align;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _h3
		{
			//Optional Attributes
			char * align;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _h4
		{
			//Optional Attributes
			char * align;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _h5
		{
			//Optional Attributes
			char * align;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _h6
		{
			//Optional Attributes
			char * align;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _hr
		{
			//Optional Attributes
			char * align;
			char * noshade;
			char * size;
			char * width;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _html
		{
			//Required  Attributes
			char * xmlns;
			
			//Standard Attributes
			char * dir;
			char * lang;
			char * xml_lang;
		};

		struct _iframe
		{
			//Optional Attributes
			char * align;
			char * frameborder;
			char * height;
			char * longdesc;
			char * marginheight;
			char * marginwidth;
			char * name;
			char * scrolling;
			char * src;
			char * width;			
			
			//Standard Attributes
			char * class;
			char * id;
			char * style;
			char * title;
		};		

		struct _img
		{
			//Required  Attributes
			char * alt;
			char * src;

			//Optional Attributes
			char * align;
			char * border;
			char * height;
			char * hspace;
			char * ismap;
			char * longdesc;
			char * usemap;
			char * vspace;
			char * width;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _input
		{
			//Optional Attributes
			char * accept;
			char * align;
			char * alt;
			char * checked;			
			char * disabled;
			char * maxlength;
			char * name;
			char * readonly;
			char * size;
			char * src;
			char * type;
			char * value;
			
			//Standard Attributes
			char * accesskey;
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * tabindex;
			char * title;
			char * xml_lang;
		};

		struct _ins
		{
			//Optional Attributes
			char * forc;
			char * datetime;
						
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _label
		{
			//Optional Attributes
			char * forc;
						
			//Standard Attributes
			char * accesskey;
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _legend
		{
			//Optional Attributes
			char * align;
						
			//Standard Attributes
			char * accesskey;
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _li
		{
			//Optional Attributes
			char * type;
			char * value;
						
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _link
		{
			//Optional Attributes
			char * charset;
			char * href;
			char * hreflang;
			char * media;
			char * rel;
			char * rev;
			char * target;
			char * type;
						
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _map
		{
			//Required  Attributes
			char * name;

			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _menu
		{
			//Optional Attributes
			char * compact;
						
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
		};

		struct _meta
		{
			//Required  Attributes
			char * content;
			
			//Optional Attributes
			char * http_equiv;
			char * name;
			char * scheme;
						
			//Standard Attributes
			char * dir;
			char * lang;
			char * xml_lang;
		};

		struct _noframes
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _noscript
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _object
		{
			//Optional Attributes
			char * align;
			char * archive;
			char * border;
			char * classid;
			char * codebase;
			char * codetype;
			char * data;
			char * declare;
			char * height;
			char * hspace;
			char * name;
			char * standby;
			char * type;
			char * usemap;
			char * vspace;
			char * width;		
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * tabindex;
			char * title;
			char * xml_lang;
		};

		struct _ol
		{
			//Optional Attributes
			char * compact;
			char * start;
			char * type;
						
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _optgroup
		{
			//Required  Attributes
			char * label;
			
			//Optional Attributes
			char * disabled;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _option
		{
			//Optional Attributes
			char * disabled;
			char * label;
			char * selected;
			char * value;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _p
		{
			//Optional Attributes
			char * align;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _param
		{
			//Required  Attributes
			char * name;
			
			//Optional Attributes
			char * type;
			char * value;
			char * valuetype;
			
			//Standard Attributes
			char * id;
		};

		struct _pre
		{
			//Optional Attributes
			char * width;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _q
		{
			//Optional Attributes
			char * cite;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _s
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
		};

		struct _strike
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
		};

		struct _script
		{
			//Required  Attributes
			char * type;
			
			//Optional Attributes
			char * charset;
			char * defer;
			char * src;
			char * xml_space;
		};

		struct _select
		{
			//Optional Attributes
			char * disabled;
			char * multiple;
			char * name;
			char * size;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * tabindex;
			char * title;
			char * xml_lang;
		};

		struct _span
		{			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _style
		{
			//Required  Attributes
			char * type;
			
			//Optional Attributes
			char * media;
			
			//Standard Attributes
			char * dir;
			char * lang;
			char * title;
			char * xml_lang;
		};

		struct _sub
		{			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _sup
		{			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _table
		{
			//Optional Attributes
			char * align;
			char * bgcolor;
			char * border;
			char * cellpadding;
			char * cellspacing;
			char * frame;
			char * rules;
			char * summary;
			char * width;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _tbody
		{
			//Optional Attributes
			char * align;
			char * charc;
			char * charoff;
			char * valign;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
	
		struct _td
		{
			//Optional Attributes
			char * abbr;
			char * align;
			char * axis;
			char * bgcolor;
			char * charc;
			char * charoff;
			char * colspan;
			char * headers;
			char * height;
			char * nowrap;
			char * rowspan;
			char * scope;
			char * valign;
			char * width;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _textarea
		{
			//Required  Attributes
			char * cols;
			char * rows;
			
			//Optional Attributes
			char * disabled;
			char * name;
			char * readonly;			
			
			//Standard Attributes
			char * accesskey;
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * tabindex;
			char * title;			
			char * xml_lang;
		};

		struct _tfoot
		{
			//Optional Attributes
			char * align;
			char * charc;
			char * charoff;
			char * valign;			
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _th
		{
			//Optional Attributes
			char * abbr;
			char * align;
			char * axis;
			char * bgcolor;
			char * charc;
			char * charoff;
			char * colspan;
			char * headers;
			char * height;
			char * nowrap;
			char * rowspan;
			char * scope;
			char * valign;
			char * width;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _thead
		{
			//Optional Attributes
			char * align;
			char * charc;
			char * charoff;
			char * valign;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
		struct _title
		{
			//Standard Attributes
			char * dir;
			char * lang;
			char * xml_lang;
		};

		struct _tr
		{
			//Optional Attributes
			char * align;
			char * bgcolor;
			char * charc;
			char * charoff;
			char * valign;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};

		struct _u
		{
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
		};

		struct _var
		{
			//Optional Attributes
			char * compact;
			char * type;
			
			//Standard Attributes
			char * class;
			char * dir;
			char * id;
			char * lang;
			char * style;
			char * title;
			char * xml_lang;
		};
		
struct _XML_Data
{
	int tagnum;	
	int   depth;
	BYTE tagdata;
	int fontsize;
	
	int 	imgwidth;
	int 	imgheight;
	int 	imghorsizescaled;
	int 	imgversizescaled;
	
	 void *tag;

	int tablecase;
	int tablecaselevel;
	int strlength;
	BYTE GlyphSetMode;
	char *strdata;
	long *lstrdata;
	XML_Mask *pmask;
	XML_Data_t *pre,*next;	
};

struct _XML_Matadata
{
	char cEPUB_ID[256]; 			/**< ebook id*/                                       
  	char cEPUB_TITLE[256]; 		/**< ebook title*/                                 
  	char cEPUB_CREATOR[256]; 	/**< ebook creator*/                             
  	char cEPUB_CONTRIB[256]; 	/**< ebook contributor*/                         
  	char cEPUB_SUBJECT[256]; 	/**< ebook subject*/                             
  	char cEPUB_PUBLISHER[256]; 	/**< ebook publisher*/                         
  	char cEPUB_DESCRIPTION[256]; /**< ebook description*/                     
  	char cEPUB_DATE[256]; 		/**< ebook data */                                  
  	char cEPUB_TYPE[256]; 		/**< ebook type */                                  
  	char cEPUB_FORMAT[256]; 	/**< ebook format */                              
  	char cEPUB_SOURCE[256]; 		/**< ebook source */                              
  	char cEPUB_LANG[256]; 		/**<  ebook language */                             
  	char cEPUB_RELATION[256]; 	/**< ebook relation*/                           
  	char cEPUB_COVERAGE[256]; 	/**< ebook coverage*/                           
  	char cEPUB_RIGHTS[256];		/**< ebook rights */                               
  	char cEPUB_META[256]; 		/**< ebook extra metadata*/         
};

struct _XML_NAV
{
	char *label;
	char *link;
	XML_NAV_t *pre,*next;
};


struct _XML_Iterator
{
	char *urlpath;
	char *urldata;

	TTF_Font_t *ttfpathlink;
	XML_Data_t	tag_data;
	XML_Data_t	*cur_data;
	XML_Iterator_t *pre,*next;	
};

struct _XML_Epub_info
{	
	//enum _XML_EPUB_STATE  state;

	int   depth[64];
	int   depth_index;
	int	error_code;
	
	XML_Matadata_t	 MataData;
	XML_Iterator_t	 tag_iterator;	//for xmlparser to parser xml tag for epub_typesetting 
      	XML_Iterator_t	 *cur_iterator;    //for epub chapter file name and raw html data
		
	XML_NAV_t 		tag_nav;
	XML_NAV_t          	*cur_nav;

	//XML_Data_t		tag_data;
	//XML_Data_t		*cur_data;
	
	char font[MAX_DESC];
};

#endif

