
#ifndef __xml_framechannel_h__
#define __xml_framechannel_h__
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
typedef struct _framechannel_info     framechannel_info_t;
typedef struct _framechannel_map      framechannel_map_t;

#define MAX_TITLE       64
#define MAX_USERNAME    64
#define MAX_USERID      64
#define MAX_SETID       64
#define MAX_URL         256
#define MAX_CONTACTS    4096

struct _set_entry
{
    set_entry_t         *next;

    char    title[MAX_TITLE];
};

enum _FRAMECHANNEL_STATE
{
    FRAMECHANNEL_CHANNEL_INIT,
    FRAMECHANNEL_CATEGORY_FOUND,
    FRAMECHANNEL_PHOTOLIST_INIT,
	FRAMECHANNEL_PHOTOLIST_ITEM,
	FRAMECHANNEL_PHOTOLIST_TITLE,
	FRAMECHANNEL_PHOTOLIST_LINK,
	FRAMECHANNEL_PHOTOLIST_LINKGET,
};


struct _framechannel_info
{
    enum _FRAMECHANNEL_STATE  state;
    char                username[MAX_USERNAME];
    char                *password;
    set_entry_t         set_list;
    set_entry_t         *cur_set;
};

struct _framechannel_map
{
    framechannel_info_t   info;
    char            	*base_dir;
    char            contacts[MAX_CONTACTS];
    int             contacts_length;
    
    framechannel_map_t    *next;
    framechannel_map_t    *prev;
};

static framechannel_map_t     framechannel_map =
{
    .next   = &framechannel_map,
    .prev   = &framechannel_map,
};

static int framechannel_fetch_photosets(framechannel_map_t *framechannel_map);
static int framechannel_fetch_photolists(framechannel_map_t *framechannel_map);
static int framechannel_fetch_allphotolist(framechannel_map_t *framechannel_map);

#endif

