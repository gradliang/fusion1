#ifndef __xml_shutterfly_h__
#define __xml_shutterfly_h__

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
typedef struct _shutterfly_info     shutterfly_info_t;


#define MAX_TITLE       64
#define MAX_USERNAME    64
#define MAX_ALBUMID     64
#define MAX_URL         256
#define MAX_AUTHTOKEN_LEN     256

struct _album_entry
{
    album_entry_t         *next;

    char    id[MAX_ALBUMID];
    char    title[MAX_TITLE];
};

enum _PICASA_STATE
{
    SHUTTERFLY_AUTH_INIT,
    SHUTTERFLY_AUTH_ENTRY,
    SHUTTERFLY_AUTH_NEWAUTHTOKEN,
    SHUTTERFLY_ALBUM_LIST_INIT,
    SHUTTERFLY_ALBUM_INIT,
    SHUTTERFLY_ALBUM_PHOTO,
    SHUTTERFLY_ALBUM_PHOTO_TITLE,
    SHUTTERFLY_ALBUM_PHOTO_MEDIAGROUP,
};


struct _shutterfly_info
{
    enum _PICASA_STATE  state;
    char                username[MAX_USERNAME+1]; /* email address */
    int                 error_code;
    album_entry_t       album_list;
    album_entry_t       *cur_album;
    char                auth[MAX_AUTHTOKEN_LEN+1];          /* auth token */
    char                title[NETFS_MAX_PATHNAME];          /* image title */
    char                url[2][NETFS_MAX_URL];              /* image urls */
    int                 height[2];                          /* image height */
    int                 num_urls;                          /* image height */
    char                *url_l;              /* image urls */
    char                *url_s;              /* image urls */
};

int shutterfly_fetch_album_list(shutterfly_info_t *shutterfly_info);
int shutterfly_fetch_album(shutterfly_info_t *shutterfly_info, char *album_id);

#endif //__xml_shutterfly_h__

