#ifndef __xml_rss_h__
#define __xml_rss_h__

#include "netware.h"

typedef struct _image_entry image_entry_t;
typedef struct _item_entry  item_entry_t;
typedef struct _rss_info    rss_info_t;
typedef struct _rss_map     rss_map_t;


#define MAX_URL         256
#define MAX_TITLE       1024
#define MAX_DESC        8192        /* 4K is not enough for yahoo blog */
#define MAX_DATE        32
#define MAX_ENCODING    16

struct _image_entry
{
    char            url[MAX_URL];

    image_entry_t   *next;
};

struct _item_entry  
{
    char    *desc;
    int     desclen;
    char    *title;
    int     titlelen;
    char    pubdate[MAX_DATE];
    char    media_url[MAX_URL];
    int     media_length;

    image_entry_t   *image;

    item_entry_t    *next;
};

#if 0 //CJ move  to netware
enum RSS_STATE
{
    RSS_NULL,
    RSS_START,
    RSS_CHANNEL,
    RSS_TITLE,
    RSS_DESC,
    RSS_ITEM,
    RSS_ITEM_PUBDATE,
    RSS_ITEM_TITLE,
    RSS_ITEM_DESCRIPTION,
    RSS_TITLE_CONTENT,
    RSS_DESC_CONTENT,
    RSS_DESC_IMG,
    RSS_DESC_IMG_SRC,
};
#endif

struct _rss_info
{
    enum RSS_STATE state;
    
    int             item_no;
    item_entry_t    *cur_item;
    item_entry_t    item_list;
    char            has_space;
    char            has_line;

    char            *title;
    char            *description;

    char    desc_buffer[MAX_DESC];      /* temp buffer used in XML parser */
    char    desc_content[MAX_DESC];     /* temp buffer used in HTML parser */
};

struct _rss_map
{
    char            *base_dir;
    char            encoding[MAX_ENCODING];
    rss_info_t      info;
    
    rss_map_t       *next;
    rss_map_t       *prev;
};


static rss_map_t        rss_map = 
{
    .next   = &rss_map,
    .prev   = &rss_map
};


#endif //__xml_rss_h__
