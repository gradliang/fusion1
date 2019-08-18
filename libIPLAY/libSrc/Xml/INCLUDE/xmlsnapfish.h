#ifndef __xml_snapfish_h__
#define __xml_snapfish_h__

#include "netware.h"

//typedef struct _set_entry       set_entry_t;
typedef struct _snapfish_info     snapfish_info_t;
typedef struct _snapfish_map      snapfish_map_t;

typedef struct _SF_Login      	SF_Login_t;
typedef struct _SF_GetAlbums 	SF_GetAlbums_t;
typedef struct _SF_GetAlbumInfo SF_GetAlbumInfo_t;

#define MAX_TITLE        64
#define MAX_USERNAME     64
#define MAX_USERPASSWORD 64
//#define MAX_SETID        32
#define MAX_MIME         32
#define MAX_URL          256
#define MAX_AUTHCODE     256
//#define MAX_CONTACTS     4096

/*
struct _set_entry
{
    set_entry_t         *next;
    char    id[MAX_SETID];
    char    title[MAX_TITLE];
    char    url[MAX_URL];
    char    *dirname;
    char    mime[MAX_MIME];
};
 */
 
enum _SNAPFISH_STATE
{
   	SNAPFISH_NULL = 0,
	SNAPFISH_LOGIN,
	SNAPFISH_GOT_AUTHCODE,
	SNAPFISH_GOT_PODHOST,
	SNAPFISH_GOT_ADHOST,
	SNAPFISH_GOT_SMARTHOST,
	SNAPFISH_GOT_PRICEVERSION,

	SNAPFISH_RESP_GETALBUMS,
	SNAPFISH_GETALBUMS,	
	SNAPFISH_GOT_TOTAL,
	SNAPFISH_GOT_OWNED,
	SNAPFISH_GOT_SHARED,
	SNAPFISH_GETALBUM,
	SNAPFISH_GOT_ID,
	SNAPFISH_GOT_NAME,
	SNAPFISH_GOT_ACL,
	SNAPFISH_GOT_FSTTNURL,
	SNAPFISH_GOT_NUMIMAGES,

	SNAPFISH_RESP_GETALBUMINFO,
	//SNAPFISH_GOT_ID,
	//SNAPFISH_GOT_NAME,
	//SNAPFISH_GOT_ACL,
	SNAPFISH_GET_PICTURES,
	SNAPFISH_GET_PICTURE,
	SNAPFISH_GOT_PIC_ID,
	SNAPFISH_GOT_PIC_CAPTION,
	SNAPFISH_GOT_PIC_TNURL,
	SNAPFISH_GOT_PIC_SRURL,
	SNAPFISH_GOT_PIC_WIDTH,
	SNAPFISH_GOT_PIC_HEIGHT,
	
	
/*		
	SNAPFISH_GOT_AUTHCODE,
	SNAPFISH_GET_USERINFO,
	SNAPFISH_GOT_PODHOST,
	SNAPFISH_GOT_ADHOST,
	SNAPFISH_GOT_SMARTHOST,
	SNAPFISH_GOT_PRICEVERSION,
	SNAPFISH_GOT_EMAIL,
	SNAPFISH_GOT_PASSWORD,
	SNAPFISH_GOT_FNAME,
	SNAPFISH_GOT_LNAME,
	SNAPFISH_GOT_ALBUMID,
	SNAPFISH_GOT_TNURL,
	SNAPFISH_GOT_SRURL,
	SNAPFISH_GOT_SESSIONID,
*/
    SNAPFISH_ERROR,
 
};

struct _SF_Login
{
//	int authcode; 
	//char preferencedate;
	char podhost[MAX_URL];
	char adhost[MAX_URL];
	char smarthost[MAX_URL];
	char priceversion[MAX_USERNAME]; 	
};

struct _SF_GetAlbums
{
	int 	total;
	int   owned;
	int   shared;
	
	int id;
	char name[MAX_USERNAME];
	int acl;
	char firsttnurl [MAX_URL]; 
	int numberImages; 
	SF_GetAlbums_t *next;
};

struct _SF_GetAlbumInfo
{
	int id;
	char name[MAX_USERNAME];
	int acl;
	
	int	pic_id; 
	char caption[MAX_USERNAME];
	char tnurl[MAX_URL]; 
	char srurl[MAX_URL]; 
	int  width;
	int height;
	SF_GetAlbumInfo_t *next;
};

struct _snapfish_info
{
    enum _SNAPFISH_STATE  state;

    SF_Login_t 		*SF_Login;
    SF_GetAlbums_t *SF_GetAlbums;
    SF_GetAlbumInfo_t *SF_GetAlbumInfo;

    int subscriberid;
    char email[MAX_USERNAME];
    char password[MAX_USERPASSWORD];
    char partnerid[MAX_USERNAME];
    char mobileno[MAX_USERNAME];	

    char authcode[MAX_AUTHCODE]; 
    int type; 
    BOOL excludeVirtualAlbums;
    int AlbumId ;

    char fname[MAX_USERNAME];
    char lname[MAX_USERNAME];
   
    SF_GetAlbums_t   *curr_Albums;
    SF_GetAlbumInfo_t  *curr_AlbumInfo;
	
};

struct _snapfish_map
{
    snapfish_info_t   info;
    char            *base_dir;
//    snapfish_map_t		*next;
//    snapfish_map_t		*prev;
};

//static snapfish_map_t     snapfish_map;

#if 0
int snapfish_init(char * snapfish_url, char * base_dir);
void snapfish_exit(char * base_dir);
#endif

int snapfish_login(BYTE *username,BYTE *password);
int snapfish_PhotoList_Get(BYTE *PhotoSet);

#endif //__xml_snapfish_h__

