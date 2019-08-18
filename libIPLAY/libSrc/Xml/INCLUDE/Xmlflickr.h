
#ifndef __xml_flickr_h__
#define __xml_flickr_h__
/*
 * 
 * --------------------------------------------------------------------------------
 * 1st level directory is the photo set list
 * 2nd level directory is the photo list under photo set
 * --------------------------------------------------------------------------------
 * 
 * Flickr Document
 *  1) Photo Source URLs - http://www.flickr.com/services/api/misc.urls.html
 *      http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{secret}.jpg
 *      http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{secret}_[mstb].jpg
 *      http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{o-secret}_o.(jpg|gif|png) *  2) 
 * 
 *  2) Buddyicons - http://www.flickr.com/services/api/misc.buddyicons.html
 *      http://farm{icon-farm}.static.flickr.com/{icon-server}/buddyicons/{nsid}.jpg
 * 
 * --------------------------------------------------------------------------------
 * 
 * Everyone's Photo:
 *      http://www.flickr.com/services/feeds/photos_public.gne
 * 
 * Service URL:
 * http://api.flickr.com/services/rest/
 * 
 * 
 * Returns the list of interesting photos:
 *      method=flickr.interestingness.getList
 *      date(option)=YYYY-MM-DD
 * 
 * get user id:
 *      method=flickr.people.findByUsername
 *      username=
 * 
 * photo set list:
 *      method=flickr.photosets.getList
 *      user_id=
 * 
 * photo list:
 *      method=flickr.photosets.getPhotos
 *      photoset_id=
 *      user_id=
 * 
 * photo data:
 *    1. get http://farm1.static.flickr.com/%PhotoPath%
 * 
 * PhotoPath suffixs with 
 *      _s: 75x75, _t: 100x75, _m: 240x180, (null): 500x375, _b: 1024x768, _o: 1600x1200(original)
 *      eg. http://farm1.static.flickr.com/187/437732996_2824ad9d7a_s.jpg
 */

typedef struct _set_entry       set_entry_t;
typedef struct _flickr_info     flickr_info_t;
typedef struct _flickr_map      flickr_map_t;

#define MAX_TITLE       64
#define MAX_USERNAME    64
#define MAX_USERID      64
#define MAX_SETID       64
#define MAX_URL         256
#define MAX_CONTACTS    4096

struct _set_entry
{
    set_entry_t         *next;

    char    id[MAX_SETID];
    char    title[MAX_TITLE];
    char    *dirname;
};

enum _FLICKR_STATE
{
    FLICKR_NULL,
    FLICKR_ERROR,
    FLICKR_INTERESTING_INIT,
    FLICKR_FRIEND_INIT,
    FLICKR_USERID_INIT,
    FLICKR_PHOTOSET_INIT,
    FLICKR_PHOTOSET_FOUND,
    FLICKR_PHOTOSET_TITLE,
    FLICKR_PHOTOLIST_INIT,
    FLICKR_PHOTOLIST_PAGES,
    FLICKR_PHOTOLIST_THUMB_START,
    FLICKR_PHOTOLIST_THUMB_ANCHOR,
};


struct _flickr_info
{
    enum _FLICKR_STATE  state;
    char                username[MAX_USERNAME];
    char                userid[MAX_USERID];
    char                *api_key;
    int                 error_code;
    set_entry_t         set_list;
    set_entry_t         *cur_set;
};

struct _flickr_map
{
    flickr_info_t   info;
    char            *base_dir;
    char            contacts[MAX_CONTACTS];
    int             contacts_length;
    
    flickr_map_t    *next;
    flickr_map_t    *prev;
};

static flickr_map_t     flickr_map =
{
    .next   = &flickr_map,
    .prev   = &flickr_map,
};

static int flickr_fetch_userid(flickr_map_t *flickr_map);
static int flickr_fetch_photosets(flickr_map_t *flickr_map);
static int flickr_fetch_photolists(flickr_map_t *flickr_map);
static int flickr_fetch_allphotolist(flickr_map_t *flickr_map);
static int flickr_fetch_friend(flickr_map_t *flickr_map);
static int flickr_fetch_interesting(flickr_map_t *flickr_map);

#endif //__xml_flickr_h__

