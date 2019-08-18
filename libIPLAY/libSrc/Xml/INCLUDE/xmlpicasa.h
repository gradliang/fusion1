#ifndef __xml_picasa_h__
#define __xml_picasa_h__

#define ATOM_MODE        0

#include "netware.h"
/**
 * Picasa Web Albums Data API Developer's Guide:
 *      http://code.google.com/apis/picasaweb/gdata.html
 * =======================================================================
 * 
 * http://picasaweb.google.com/data/feed/base/user/nickhtc?kind=album&alt=rss&access=public
 * http://picasaweb.google.com/data/feed/base/user/nickhtc/albumid/5044405688276656273?alt=rss
 * 
 */


typedef struct _album_entry     album_entry_t;
typedef struct _picasa_info     picasa_info_t;


#define MAX_TITLE       64
#define MAX_USERNAME    64
#define MAX_ALBUMID     64
#define MAX_URL         256

struct _album_entry
{
    album_entry_t         *next;

    char    id[MAX_ALBUMID];
    char    title[MAX_TITLE];
#if ATOM_MODE	
    char    edit[MAX_URL];
#endif
};

enum _PICASA_STATE
{
    PICASA_NULL,
    PICASA_ERROR,
    PICASA_ALBUM_LIST_INIT,
    PICASA_ALBUM_LIST_FOUND,
    PICASA_ALBUM_LIST_TITLE,
    PICASA_ALBUM_LIST_ID,
    PICASA_ALBUM_INIT,
    PICASA_ALBUM_PHOTO_FOUND,
    PICASA_ALBUM_PHOTO_TITLE,
    PICASA_ALBUM_PHOTO_COUNTS,
    PICASA_ALBUM_PHOTO_URL,
#if ATOM_MODE	
    //For ATOM
    PICASA_ATOM_ALBUM_ENTRY,
    PICASA_ATOM_PHOTO_ENTRY,
#endif    
};


struct _picasa_info
{
    enum _PICASA_STATE  state;
    char                username[MAX_USERNAME];
    int                 error_code;
    album_entry_t       album_list;
    album_entry_t       *cur_album;
    char                auth[512+1];
};

static int picasa_fetch_album_list(picasa_info_t *picasa_info);
static int picasa_fetch_album(picasa_info_t *picasa_info);

#endif //__xml_picasa_h__

