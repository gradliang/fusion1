#ifndef _WINDOWS_LIVE_H
#define _WINDOWS_LIVE_H

#include "ndebug.h"

#define windows_live_malloc(sz)   mm_malloc(sz)
#define windows_live_mfree(ptr)   mm_free(ptr)

#define MAX_URL         256
#define MAX_TITLE       64
#define MAX_USERNAME    64
#define MAX_ALBUMID     64

typedef struct _album_entry     album_entry_t;

struct _album_entry
{
    album_entry_t         *next;

    char    id[MAX_ALBUMID];
    char    title[MAX_TITLE];
    char    url[MAX_URL];
};

enum _WINDOWS_LIVE_STATE
{
    WINDOWS_LIVE_ID,
    WINDOWS_LIVE_SDK_LOGIN,
    WINDOWS_LIVE_GET_DETAILS,
    WINDOWS_LIVE_GET_HOMEPAGE,
    WINDOWS_LIVE_GET_JAVASCRIPT,
    WINDOWS_LIVE_ALBUM_LIST_ID,
    WINDOWS_LIVE_ALBUM_INIT,
    WINDOWS_LIVE_ALBUM_FOUND,
    WINDOWS_LIVE_PHOTO_INIT,
    WINDOWS_LIVE_PHOTO_FOUND,
    WINDOWS_LIVE_PHOTO_SRC,
    WINDOWS_LIVE_PHOTO_LOCATION,
    WINDOWS_LIVE_PHOTO_LOCATION_FOUND,
};

struct _windows_live_info
{
    enum _WINDOWS_LIVE_STATE  state;
    int                 error_code;
    char                username[MAX_USERNAME];
    char                cid[32];
    album_entry_t       album_list;
    album_entry_t       *cur_album;
    char                photo_url[MAX_URL];
    char                css_url[MAX_TITLE];
    char                javascript_url[MAX_TITLE];
};

typedef struct _windows_live_info     windows_live_info_t;



#endif // _WINDOWS_LIVE_H


