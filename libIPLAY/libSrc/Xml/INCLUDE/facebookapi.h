#ifndef __xml_facebook_h__
#define __xml_facebook_h__

#include "netware.h"

/*
 * define HTTP connection time out
 */
#define CONNECTION_TIME_OUT 30

typedef struct fb_album fb_album_t;
typedef struct fb_friend fb_friend_t;

enum enumFacebookStatusCode {
    enumStateNormal = 0,					/* state normal*/
    enumUnableToConnectToNetwork,		/* Unable to connect to network */
    enumUnableToConnectToFacebook,		/* Unable to connect to facebook */
    enumIncorrectUserNameOrPassword,	/* Incorrect user name or password */
    enumNoFacebookContentFound,		/* No facebook Content found */
    enumUseCachedStatusData				/* Unable to connect to network, use cached status information*/

};

/*
* Album
*/
struct fb_album{
	fb_album_t *next;

	int album_num;
	char *aid;		//The ID of the album being queried.
	int aid_len;		//Tha aid's string length.
	char *cover_pid;	//The ID of the photo used as the cover for the album being queried.
	char *name;		//The name of the album being queried.
	int photo_num;	//The number of photos in the album being queried.
	char *link; 		//The URL to the album being queried.
	char *owner; 	//The user ID of the owner of the album being queried.
	char *src;		//The URL to the album view version of the photo being queried. The image can have a maximum width or height of 130px. This URL may be blank.
	char *src_big;	//The URL to the full-sized version of the photo being queried. The image can have a maximum width or height of 720px. This URL may be blank.
	char *src_small;	//The URL to the thumbnail version of the photo being queried. The image can have a maximum width of 75px and a maximum height of 225px. This URL may be blank.
	char *caption;	//The caption for the photo being queried.		
};

/*
* Friend
*/
struct fb_friend{
	fb_friend_t *next;

	int friend_num;
	char *friend_uid;	//The user ID for the user whose friends you want to return.
	int friend_album_num;	//The friend own album number.

};
#endif //__xml_facebook_h__

