
#ifndef __xml_upnp_h__
#define __xml_upnp_h__
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

typedef struct _set_entry		set_entry_t;
typedef struct _upnp_info		upnp_info_t;
typedef struct _upnp_map		upnp_map_t;

#define UPNP_MAX_TITLE       64
#define UPNP_MAX_USERNAME    64
#define UPNP_MAX_USERID      64
//#define MAX_SETID       64
#define UPNP_MAX_SETID       32
#define UPNP_MAX_URL         256
#define UPNP_MAX_CONTACTS    4096

struct _set_entry
{
    set_entry_t         *next;

    char    id[UPNP_MAX_SETID];
    char    title[UPNP_MAX_TITLE];
    char    *dirname;
	DWORD	streamtype;
};

enum _UPNP_STATE
{
    UPNP_NULL,
    UPNP_ERROR,
    UPNP_INTERESTING_INIT,
    UPNP_FRIEND_INIT,
    UPNP_USERID_INIT,
    UPNP_STATIONS_INIT,
    UPNP_STATIONS_INIT_OK,
    UPNP_STATIONS_FOUND,
    UPNP_STATIONS_TITLE,
    UPNP_PHOTOLIST_INIT,
    UPNP_PHOTOLIST_PAGES,
    UPNP_PHOTOLIST_THUMB_START,
    UPNP_PHOTOLIST_THUMB_ANCHOR,
};


struct _upnp_info
{
    enum _UPNP_STATE	state;
    char                username[UPNP_MAX_USERNAME];
    char                userid[UPNP_MAX_USERID];
    char                *api_key;
    int                 error_code;
    set_entry_t         set_list;
    set_entry_t         *cur_set;
};

struct _upnp_map
{
    upnp_info_t		info;
    char            *base_dir;
    char            contacts[UPNP_MAX_CONTACTS];
    int             contacts_length;
    
    upnp_map_t		*next;
    upnp_map_t		*prev;
};

//static upnp_map_t     *pupnp_map;
/*
=
{
    .next   = &upnp_map,
    .prev   = &upnp_map,
};
*/
//static int shoutcast_fetch_stations(shoutcast_map_t *shoutcast_map,const char *shoutcast_url);
//static int flickr_fetch_userid(flickr_map_t *flickr_map);
//static int flickr_fetch_photosets(flickr_map_t *flickr_map);
//static int flickr_fetch_photolists(flickr_map_t *flickr_map);
//static int flickr_fetch_allphotolist(flickr_map_t *flickr_map);
//static int flickr_fetch_friend(flickr_map_t *flickr_map);
//static int flickr_fetch_interesting(flickr_map_t *flickr_map);
#endif //__xml_upnp_h__

