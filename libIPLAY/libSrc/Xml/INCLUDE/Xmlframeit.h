
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
#define MAX_TITLE       64
#define MAX_USERNAME    64
#define MAX_USERID      64
#define MAX_SETID       64
#define MAX_URL         256
#define MAX_CONTACTS    1024
//#define MAX_DESC        8192        /* 4K is not enough for yahoo blog */
#define MAX_DATA        32
#define MAX_ENCODING    16

enum _FRAMEIT_STATE
{
	FRAMEIT_NULL,
    	FRAMEIT_START,
    	FRAMEIT_CHANNELs,
    	FRAMEIT_TTL,
    	FRAMEIT_TITLE,
    	FRAMEIT_DESC,
    	FRAMEIT_ITEM, 
    	FRAMEIT_ITEM_LINK,
    	FRAMEIT_ITEM_SIZE,
    	FRAMEIT_ITEM_CHANNEL,
    	FRAMEIT_ITEM_PUBDATE,
       FRAMEIT_ITEM_TITLE,
    	FRAMEIT_ITEM_DESCRIPTION,
    	FRAMEIT_TITLE_CONTENT,
    	FRAMEIT_DESC_CONTENT,
    	FRAMEIT_DESC_IMG,
    	FRAMEIT_DESC_IMG_SRC,
    	FRAMEIT_GET,
    	FRAMEIT_POST,
    	FRAMEIT_ITEM_ENCLOSURE,    	

	FRAMEIT_GetClaimTokenResult,
	FRAMEIT_GetClaimToken_ClaimToken,
	FRAMEIT_GetClaimToken_ClaimUrl,
	FRAMEIT_GetClaimToken_ResponseCode,

	FRAMEIT_DeviceBindResult,
	FRAMEIT_DeviceBind_DeviceId,
	FRAMEIT_DeviceBind_ResponseCode,

	FRAMEIT_DeviceBindUserResult,
	FRAMEIT_DeviceBindUser_DeviceId,
	FRAMEIT_DeviceBindUser_ResponseCode,

	FRAMEIT_GetCollectionInfoResult,
	FRAMEIT_CollectionInfoList,
	FRAMEIT_CollectionInfo,
	FRAMEIT_CollectionInfo_Name,
	FRAMEIT_CollectionInfo_FeedUrl,	
	FRAMEIT_GetCollectionInfo_ResponseCode,
	
	FRAMEIT_GetCollectionInfoUserResult,
	FRAMEIT_CollectionInfoUserList,
	FRAMEIT_CollectionInfoUser,
	FRAMEIT_CollectionInfoUser_Name,
	FRAMEIT_CollectionInfoUser_FeedUrl,	
	FRAMEIT_CollectionInfoUser_ResponseCode,	

};
//typedef struct _image_entry image_entry_t;
typedef struct _item_entry       item_entry_t;
typedef struct _frameit_info     frameit_info_t;
/*
struct _image_entry
{
    char  title[MAX_URL];;
    int     titlelen;	
    char            url[MAX_URL];
    image_entry_t   *next;
};
*/
struct _item_entry  
{
    item_entry_t   *next;	
    //  char    *desc;
    // int     desclen;
    char    *title;
    int     titlelen;	
    char    Name[MAX_URL];
    int     Namelen;
    char    FeedUrl[MAX_URL];
    int     FeedUrllen;  
   // image_entry_t   *cur_image;	
  //  image_entry_t   *image;	  
};

struct _frameit_info
{
    enum _FRAMEIT_STATE  state;
    char                username[MAX_USERNAME];	
    int             			item_no;
    int                 	 	error_code;
    item_entry_t         	item_list;
    item_entry_t         	*cur_item;
    int       ttl;	
    char    ClaimToken[MAX_DATA];     
    char    ClaimUrl[MAX_URL];      
    char    DeviceId[MAX_URL];     
    int    ResponseCode;    
};


#endif
