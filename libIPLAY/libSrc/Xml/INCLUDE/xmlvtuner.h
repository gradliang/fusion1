
#ifndef __xml_vtuner_h__
#define __xml_vtuner_h__
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
typedef struct _vtuner_info     vtuner_info_t;
typedef struct _vtuner_map      vtuner_map_t;
typedef struct _vtuner_search_area_info vtuner_search_t;

#define MAX_TITLE       64
#define MAX_USERNAME    64
#define MAX_USERID      64
//#define MAX_SETID       64
#define MAX_SETID       32
#define MAX_MIME        32
#define MAX_URL         256
#define MAX_CONTACTS    4096
#define MAX_DESC        8192        /* 4K is not enough for yahoo blog */

struct _set_entry
{
    set_entry_t         *next;

    char    id[MAX_SETID];
    char    title[MAX_TITLE];
	char    url[MAX_URL];
    char    *dirname;
    char    mime[MAX_MIME];
};

enum _VTUNER_STATE
{
    VTUNER_NULL,
    VTUNER_ERROR,
    VTUNER_GET_MENU,
    VTUNER_GET_ITEMCOUNT,
    VTUNER_GET_LOCATION_TITLE,
    VTUNER_GET_LOCATION_URL,
    VTUNER_GET_LOCATION_LIST,
    VTUNER_GET_STATION_ID,
    VTUNER_GET_STATION_NAME,
    VTUNER_GET_STATION_URL,
    VTUNER_GET_STATION_MLME,
};


struct _vtuner_info
{
    enum _VTUNER_STATE  state;
    int                 item_num;
    char                username[MAX_USERNAME];
    char                userid[MAX_USERID];
    int                 error_code;
    set_entry_t         set_list;
    set_entry_t         *cur_set;
	
};

struct _vtuner_map
{
    vtuner_info_t   info;
    char            *base_dir;
    char            contacts[MAX_CONTACTS];
    int             contacts_length;
    vtuner_map_t		*next;
    vtuner_map_t		*prev;
};
struct _vtuner_search_area_info
{
    char    title[MAX_TITLE];
	char    url[MAX_URL];
};

static vtuner_map_t     vtuner_map =
{
    .next   = &vtuner_map,
    .prev   = &vtuner_map,
};

enum _VTUNER_SEARCH_TYPE
{
    VTUNER_SEARCH_NORMAL,
    VTUNER_SEARCH_LOCATION,
    VTUNER_SEARCH_GENRE,
};
enum _VTUNER_CONNECT_HOME_PAGE_STATE
{
    VTUNER_CONNECT_HOME_PAGE_LOGIN,
    VTUNER_CONNECT_HOME_PAGE_GET_LIST,
    VTUNER_CONNECT_HOME_PAGE_GET_STATION,
    VTUNER_CONNECT_HOME_PAGE_GET_STATION_LOCATION,
    VTUNER_CONNECT_HOME_PAGE,
};

int vtuner_fetch_stations(char *vtuner_url,char level);
int vtuner_connect(void);
void vtuner_location_menu(void);

#endif //__xml_vtuner_h__

