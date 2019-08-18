/**
 * @file
 *
 * This is an implementation of Facebook Old REST API.
 * (URL: http://developers.facebook.com/docs/reference/rest/)
 * 
 * The following APIs are supported:
 *
 *  1) Create a access token - "Auth.createToken" as defined in REST API.  Access
 *     token is used in login, authorization, and the API, "auth.getSession".
 *
 *  2) Authentication - This part is not defined in REST API.  Normally, a 
 *     browser is required.  This implementation is to emulate what a browser
 *     would do for login of a Facebook user.
 *
 *  3) Authorization - Before the DPF ("the application") can access a Facebook 
 *     user's photos, the user has to authroze the application.  This part is 
 *     not defined in REST API spec.  Normally, this part requires a browser to 
 *     be accomplished.  This implementation tries to emulate a browser to 
 *     reproduce several HTTP request/response exchange between a browser and 
 *     the Facebook server.
 *
 *     Authorization needs to be done only once.
 *
 *  4) Create a session - The API is "auth.getSession".  The session key returned
 *     from the server should be saved for later use.
 *
 *     With the session key, photo albums can be accessed directly without login
 *     procedure in the future.
 *
 *  5) A test application is registered with Facebook to test this implementation.
 *     The 3 data for this application (API key, application secret, and 
 *     application ID) should be replaced with the ones for production code.
 *
 * 			_my_api_key[]
 * 			_my_secret[]
 * 			_my_application_id[]
 *
 *  6) A Facebook account is also required to test this application.  Create
 *     your account for testing.  For production, user account should be
 *     entered by the user via UI.
 *
 * 			str_fb_email
 * 			str_fb_pass
 *
 *  7) After a Facebook user account is created, you can create more than one
 *     photo album to test.
 *
 * Copyright (c) 2010 Magic Pixel Inc.
 * All rights reserved.
 */

// define this module show debug message or not,  0 : disable, 1 : enable
#define LOCAL_DEBUG_ENABLE 1
#include "corelib.h"

#if NETWARE_ENABLE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <expat.h>
#include <errno.h>
#include "netfs.h"
#include "netfs_pri.h"
#include "facebookapi.h"
#include "global612.h"
#include "md5.h"
#include "ndebug.h"

#include "..\..\lwip\include\net_sys.h"
#include "..\..\CURL\include\net_curl_curl.h"

static struct netfs_file_entry jpeg_info;

#define FACEBOOK_API_HOST "api.facebook.com"
#define FACEBOOK_WWW_HOST "www.facebook.com"
#define FACEBOOK_LOGIN_URL "https://www.facebook.com/login.php"
#define FACEBOOK_UISERVER_URL "http://www.facebook.com/connect/uiserver.php"

extern Net_App_State App_State;

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define facebook_malloc(sz)   mm_malloc(sz)
#define facebook_free(ptr)    mm_free(ptr)
#define facebook_realloc(ptr, sz)   mm_realloc(ptr, sz)

#ifndef SAFE_FREE
#define SAFE_FREE(x) do { if(x) {facebook_free(x); x=NULL;} } while(0)
#endif

#define CRLF "\r\n"

#define ACCESS_TOKEN_LENGTH 32
#define FACEBOOK_API_KEY_LENGTH 32

enum FACEBOOK_STATE {
    FB_START,
    FB_AUTH_CREATETOKEN_RESPONSE,
    FB_ALBUM_LIST_INIT,		
    FB_ALBUM_LIST_FOUND,	
    FB_ALBUM_AID_FOUND,
    FB_ALBUM_COVER_PID_FOUND,	
    FB_ALBUM_OWNER_FOUND,
    FB_ALBUM_NAME_FOUND,
    FB_ALBUM_LINK_FOUND,
    FB_PHOTO_NUM_FOUND, 
    FB_ALBUM_INIT,		
    FB_PHOTO_FOUND,	
    FB_PHOTO_PID_FOUND,
    FB_PHOTO_SRC_FOUND,
    FB_USERS_GETLOGGEDINUSER_RESPONSE,
    FB_FRIENDS_GET_INIT,
    FB_FRIENDS_GET_RESPONSE,
    FB_FRIEND_FOUND,
};

static char my_access_token[ACCESS_TOKEN_LENGTH+1];
/* _my_api_key, _my_secret and _my_application_id offered from the Facebook. */
/* Any facebook's user can apply for this information in the Facebook. */
static const char _my_api_key[] = "d5b2fe3be96ba096db60ac48fbaac7f1";
static const char _my_secret[] = "79996c61501051a792341bd8f5106f42";
static const char _my_application_id[] = "156545854385266";
static const char _my_version[] = "1.0";
/* str_fb_email and str_fb_pass is for a Facebook user account. */
static const char *str_fb_email = "";
static const char *str_fb_pass = "";

struct facebook_struct {
	void *curl;
	const char *my_api_key;
	const char *my_secret;
	const char *my_application_id;
	const char *my_version;
	char *access_token;
	char *session_key;
	char *session_secret;
	char * uid;		
	bool has_session;
	enum	enumFacebookStatusCode	enumState;
	fb_album_t fb_album_list;
	fb_album_t *fb_cur_album;

	fb_friend_t fb_friend_list;
	fb_friend_t *fb_cur_friend;
	
	fb_album_t fb_friend_album_list;
	fb_album_t *fb_friend_cur_album;
};

typedef struct _fb_info {
	void	*context;
	enum FACEBOOK_STATE	state;                 /* xml parse state */
	char	*resp;
	size_t resp_len;
	int error_code;	
} fb_info_t;


typedef struct facebook_struct facebook_struct_t;
static facebook_struct_t *facebook;

static char *str_post_form_id_granted;
static char *str_fb_dtsg_granted;
static char *curl_userpass;                        /* username:password for curl proxy */

static char *str_query_string_granted;	
static int fb_user_album_num = 0;	// User's Album number	
static char *str_album_aid;	
static int str_album_aid_len=0;
static int fb_friend_num = 0;
static char *str_friend_uid;
static int fb_friend_album_num = 0;	//Friend's Album number

//note: fixed to use global my_write_func() for all xmlxxxxxx.c files!
size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf);

char *facebook_get_signature(struct facebook_struct *ctx, char *str);
int request(struct facebook_struct *ctx, char *method, void *resp);
static XML_BUFF_link_t *xml_buff_alloc();
static void xml_buff_free(XML_BUFF_link_t *xp);
static void fb_start_element_handler(void *user_data, const char *name, const char **attr);
static void fb_end_element_handler(void *user_data, const char *name);
static void fb_content_handler(void *user_data, const char *s, int len);
static void mpx_xml_parse(void * userdata, XML_BUFF_link_t *resp, XML_StartElementHandler start_handler
        ,XML_EndElementHandler end_handler ,XML_CharacterDataHandler content_handler);
int curlSetProxy( CURL *curl, char *host, int port, char *user, char *pass );
static bool web_login_response(struct facebook_struct *ctx, char *url, XML_BUFF_link_t *result, char *post_data);
extern BYTE *Merge_ListData_to_SingleBuffer(XML_BUFF_link_t *list_head, int *total_data_len);

static void fb_album_list_start_element_handler(void *user_data, const char *name, const char **attr);
static void fb_album_list_end_element_handler(void *user_data, const char *name, const char **attr);
static void fb_album_list_content_handler(void *user_data, const char *s, int len);
static void fb_album_start_element_handler(void *user_data, const char *name, const char **attr);
static void fb_album_end_element_handler(void *user_data, const char *name, const char **attr);
static void fb_album_content_handler(void *user_data, const char *s, int len);
static void free_album(const fb_album_t  *tmp_entry);
static void fb_LoggedInUser_start_element_handler(void *user_data, const char *name, const char **attr);
static void fb_LoggedInUser_end_element_handler(void *user_data, const char *name, const char **attr);
static void fb_LoggedInUser_content_handler(void *user_data, const char *s, int len);
static bool userGetLoggedInUser(struct facebook_struct *ctx, char  **puid );

int Facebook_PhotoList_Get(BYTE *PhotoSet);	
int fb_get_photo_list(struct facebook_struct *ctx, int user_friend_flag);

static void fb_friend_start_element_handler(void *user_data, const char *name, const char **attr);
static void fb_friend_end_element_handler(void *user_data, const char *name, const char **attr);
static void fb_friend_content_handler(void *user_data, const char *s, int len);

static void fb_friend_album_list_start_element_handler(void *user_data, const char *name, const char **attr);
static void fb_friend_album_list_end_element_handler(void *user_data, const char *name, const char **attr);
static void fb_friend_album_list_content_handler(void *user_data, const char *s, int len);

static inline bool is_linear(XML_BUFF_link_t *list_head)
{
	if (list_head)
	{
		if (list_head->link)
		{
			if (list_head->link->buff_len > 0)
				return false;
		}
	}
	return true;
}

bool fb_authenticate(struct facebook_struct *ctx)
{
	int ret;
	XML_BUFF_link_t *resp;
	fb_info_t	fb_info;

	ctx->access_token = NULL;

	resp = xml_buff_alloc();
	if (!resp)
	{
		errno = -ENOMEM;
		goto fail;
	}

	ctx->curl = curl_easy_init();

	MP_TRACE_LINE();
	ret = request(ctx, "Auth.createToken", resp);
	if (ret < 0)
		goto fail;

	/* parse xml response for "Auth_createToken_response" */
	memset(&fb_info, 0, sizeof(fb_info));
	fb_info.state = FB_START;
	fb_info.context = ctx;

	mpx_xml_parse(&fb_info
            , resp 	
            , fb_start_element_handler 	
            , fb_end_element_handler
            , fb_content_handler);  		

	if (!ctx->access_token)
		goto fail;

	xml_buff_free(resp);
	return true;
fail:
    	xml_buff_free(resp);
	return false;
}

extern char *my_proxy_host;
extern uint32_t my_proxy_port;
extern char *my_proxy_password;
extern char *my_proxy_username;
static char my_proxy_login[160];

static XML_BUFF_link_t  *login_get(struct facebook_struct *ctx, char *url)
{
	int ret;
	fb_info_t	fb_info;
	XML_BUFF_link_t *resp;
	CURL *curl = ctx->curl;
	CURLcode res;

	MP_TRACE_LINE();
	curl_easy_reset( curl );

	curlSetProxy(curl, my_proxy_host, my_proxy_port, my_proxy_username, my_proxy_password );
	resp = xml_buff_alloc();
	if (!resp)
    	{
		errno = -ENOMEM;
        	goto fail;
	}

	curl_easy_setopt( curl, CURLOPT_URL, url );

	MP_TRACE_LINE();
	struct curl_slist *headers=NULL;
	/* pass our list of custom made headers */
	curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, my_write_func );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, resp );
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, CONNECTION_TIME_OUT );
	//curl_easy_setopt( curl, CURLOPT_HEADER, 1);	

	MP_TRACE_LINE();
	res = curl_easy_perform( curl );

	/* free the header list */
	curl_slist_free_all(headers);

    	MP_DEBUG( "HTTP request's (login_get) result : %d\n", res );

	int http_code = 0;
	curl_easy_getinfo ( curl, CURLINFO_HTTP_CODE, &http_code );
	MP_DEBUG("http_code: %d\n", http_code);
	if(  !( http_code == 200 || http_code == 302 ) )
	{
		res = CURLE_OBSOLETE;
		if(  http_code != 200 )
		{
        		MP_DEBUG( "HTTP request's (login_get) http_code : %d\n", http_code );
//        mpDebugPrint("%s:%u resp len=%d", __func__, __LINE__, resp->buff_len);
//        if (resp->buff_len > 0)
//            NetAsciiDump(resp->BUFF, resp->buff_len);
    		}
	}

	if (res != CURLE_OK)
      		goto fail;

	return resp;
fail:
	xml_buff_free(resp);
	return NULL;
}

static int login_post(struct facebook_struct *ctx)
{
	int	ret;
	fb_info_t	fb_info;
	char	*data = NULL;         /* post data */
	XML_BUFF_link_t	*resp;
	CURL *curl = ctx->curl;
	CURLcode res;
	char *url;

	curl_easy_reset( curl );

	curlSetProxy(curl, my_proxy_host, my_proxy_port, my_proxy_username, my_proxy_password );

	resp = xml_buff_alloc();
	if (!resp)
	{
		errno = -ENOMEM;
        	goto fail;
    	}

    	url = facebook_malloc(128);
    	if (!url)
    	{
		errno = -ENOMEM;
		goto fail;
	}
	snprintf(url, 128, FACEBOOK_LOGIN_URL "?login_attempt=1&local=en_US");
	MP_DEBUG("Confirm url: %s\n", url);
	
	curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1);
    	curl_easy_setopt( curl, CURLOPT_URL, url );

    	/* Prepare Post Data*/
	data = facebook_malloc(256);
	if ( !data )
		goto fail;
	snprintf(data,256,
		"email=%s"
		"&pass=%s"
		"&login=Login", str_fb_email, str_fb_pass);
	data[256-1] = '\0';
	MP_DEBUG("Login Post data:%s\n", data);

	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));

	/* init to NULL is important */
	struct curl_slist *headers=NULL;
	/* pass our list of custom made headers */
	curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, my_write_func );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, resp );
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, CONNECTION_TIME_OUT );
	//curl_easy_setopt( curl, CURLOPT_HEADER, 1);	
	res = curl_easy_perform( curl );

	MP_DEBUG( "HTTP request's (login_post) result : %d\n", res );

	/* free the header list */
	curl_slist_free_all(headers);

	int http_code = 0;
	curl_easy_getinfo ( curl, CURLINFO_HTTP_CODE, &http_code );
	MP_DEBUG("http_code: %d\n", http_code);
	if(  !( http_code == 200 || http_code == 302 ) )
		res = CURLE_OBSOLETE;

	if (res != CURLE_OK)
		goto fail;

	SAFE_FREE(data);
	SAFE_FREE(url);
	xml_buff_free(resp);
	return 0;
fail:
	SAFE_FREE(data);
	SAFE_FREE(url);
	xml_buff_free(resp);
	return -1;
}

static bool web_login_response(struct facebook_struct *ctx, char *url, XML_BUFF_link_t *result, char *post_data)
{
	int ret;
	fb_info_t fb_info;
	CURLcode res = CURLE_OBSOLETE;
	CURL *curl = ctx->curl;

	curl_easy_reset( curl );
	curlSetProxy(curl, my_proxy_host, my_proxy_port, my_proxy_username, my_proxy_password );

	curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt( curl, CURLOPT_UNRESTRICTED_AUTH, 1);
	curl_easy_setopt( curl, CURLOPT_MAXREDIRS, -1);
	
	curl_easy_setopt( curl, CURLOPT_URL, url );

	/* init to NULL is important */
	struct curl_slist *headers=NULL;
	curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers);

	if (*post_data)
	{
	/* if not empty */
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(post_data));
	}
	else
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);

	/* assign callback function & store data */
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, my_write_func );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, result );
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, CONNECTION_TIME_OUT );
	
	res = curl_easy_perform( curl );
	
	mpDebugPrint( "HTTP request's (web_login_response) result : %d\n", res );

	/* free the header list */
	curl_slist_free_all(headers);

	int http_code = 0;
	curl_easy_getinfo ( curl, CURLINFO_HTTP_CODE, &http_code );
	mpDebugPrint("http_code: %d\n", http_code);
	if(  !( http_code == 200 || http_code == 302 ) )
		res = CURLE_OBSOLETE;

}

bool launch_authorize(struct facebook_struct *ctx, const char *email, const char *pass)
{
	char *str_uid = "";
	char *url;
    	int ret;
    	int len;
    	XML_BUFF_link_t *resp = NULL;
    	unsigned char *bigger_buff = NULL, *html_resp;
    	char *post_data = NULL;

    	char *str_email = email;
    	char *str_pass = pass;
	MP_DEBUG("Confirm email: %s	pass: %s\n", str_email, str_pass);

	/* Send GET Login request */
    	url = facebook_malloc(256);
    	if (!url)
    	{
		errno = -ENOMEM;
        	goto fail;
    	}

    	snprintf(url, 256, FACEBOOK_LOGIN_URL "?login_attempt=1&local=en_US");
    	MP_DEBUG("Confirm GET Login url: %s\n", url);
    	resp = login_get(ctx, url);
    	if (!resp)
        	goto fail;

	xml_buff_free(resp);
	resp = NULL;
    	SAFE_FREE(url);

	/* Send POST Login request */
	MP_TRACE_LINE();
    	ret = login_post(ctx);
    	if (ret < 0)
        	goto fail;    	

	/* Send GET Authorization webpage*/    	
    	url = facebook_realloc(url, 512);
    	if (!url)
    	{
        	errno = -ENOMEM;
        	goto fail;
    	}

    	ret = snprintf(url,512,
            FACEBOOK_UISERVER_URL 
            "?app_id=%s"
            "&next=http://www.facebook.com/desktopapp.php%%3Fapi_key%%%s"
            "&display=page"
            "&locale=en_US"
            "&canvas=0"
            "&legacy_return=1"
            "&auth_token=%s"
            "&method=permissions.request"
            "&from_login=1",
            ctx->my_application_id, ctx->my_api_key, ctx->access_token);
    	if (ret >= 512)
    	{
        	MP_ALERT("%s:%u ERROR - snprintf buffer overflow", __func__, __LINE__);
        	goto fail;
    	}
    	MP_DEBUG("Confirm API Authorization url: %s\n", url);

    	resp = login_get(ctx, url);
    	if (!resp)
        	goto fail;

    	if (!is_linear(resp))
    	{
        	bigger_buff = Merge_ListData_to_SingleBuffer(resp, &len);
        	if (!bigger_buff)
            		goto fail;

        	xml_buff_free(resp);
        	resp = NULL;
        	html_resp = bigger_buff;
    	}
    	else
    	{
        	html_resp = resp->BUFF;
        	len = (resp->buff_len >= sizeof(resp->BUFF)) ? (sizeof(resp->BUFF) - 1) : resp->buff_len;
        	html_resp[len] = '\0';
    	}

	MP_DEBUG("%s:%u resp len=%d", __func__, __LINE__, len);
	//if (resp->buff_len > 0)
        	//NetAsciiDump(html_resp, resp->buff_len);

	if (resp->buff_len == 0 ) 
	{
		MP_DEBUG( "User already authorize the agent.\n");
		goto done;
    	}

	/*
	* parse post_form_id
	*/
	char *pattern = "<input type=\"hidden\" autocomplete=\"off\" name=\"post_form_id\" value=\"";
	char *start = strstr(html_resp, pattern);
	char *end = NULL;
	len = 0;
	SAFE_FREE(str_post_form_id_granted);
	if (start)
	{
		start += strlen(pattern);
		end = strchr(start , '"');
		if (end) 
		{
			len = (int)(end - start);
			str_post_form_id_granted = facebook_realloc(str_post_form_id_granted, len+1 );
			if (str_post_form_id_granted)
			{
				memcpy(str_post_form_id_granted, start, len);
				str_post_form_id_granted[len] = '\0';
            		}
        	}
    	}
	
	MP_DEBUG("++>launch_authorize str_post_form_id_granted length: %d\n", len);
	MP_DEBUG("++>launch_authorize str_post_form_id_granted: %s\n", str_post_form_id_granted);

	if (len == 0 ) 
	{
        	MP_DEBUG( "User already authorize the agent.\n");
        	goto done;
    	}

	/*
	* parse fb_dtsg
	*/
	pattern = "<input type=\"hidden\" name=\"fb_dtsg\" value=\"";
	start = strstr(html_resp, pattern);
	end = NULL;
	len = 0;
	SAFE_FREE(str_fb_dtsg_granted);
	if (start) 
	{
		start += strlen(pattern);
		end = strchr(start , '"');
		if (end) 
		{
			len = (int)(end - start);
			str_fb_dtsg_granted = facebook_malloc(len + 1);
			if (str_fb_dtsg_granted)
			{
				memcpy(str_fb_dtsg_granted, start, len);
				str_fb_dtsg_granted[len] = '\0';
            		}
		}
		if (len == 0 ) 
		{
			ctx->enumState = enumIncorrectUserNameOrPassword;
			MP_DEBUG( "Can't get fb_dtsg_granted in from home.php\n");
			goto fail;
		}
    	}	

	/*
	* parse uid
	*/
	start = strstr( html_resp, "profile.php?id=");
	end = NULL;
	len = 0;
	if( start ) 
	{
		start += strlen( "profile.php?id=" );
		end = strchr( start, '"' );
		len = (int)(end - start);
		if( len <= 0 ) 
			str_uid = "";
		else 
		{
            		str_uid = facebook_malloc(len+1);
            		if (str_uid) 
            		{
				memcpy(str_uid, start, len);
				str_uid[len] = '\0';
            		}
        	}
    	}else
        	str_uid = "";
    	MP_DEBUG("\n----->Uid: %s\n", str_uid);

    	xml_buff_free(resp);
	resp = NULL;
    	SAFE_FREE(url);
    	if (bigger_buff)
    {
		ext_mem_free(bigger_buff);
        bigger_buff = NULL;
    }

	/* Send POST Authorization Allow webpage*/
    	post_data = facebook_malloc(512);
	if (!post_data)
	{
		errno = -ENOMEM;
		goto fail;
	}

    	ret = snprintf(post_data,512,
            "post_form_id=%s"
            "&fb_dtsg=%s"
            "&app_id=%s"
            "&display=page"
            "&next=http://www.facebook.com/desktopapp.php?api_key%%%s"
            "&locale=en_US"
            "&canvas=0"
            "&legacy_return=1"
            "&auth_token=%s"
            "&from_login=1"
            "&*app_id*=%s"
            "&*user*=%s"
            "&from_post=1"
            "&__uiserv_method=permissions.request"
            "&grant_clicked=Allow",
            str_post_form_id_granted,
            str_fb_dtsg_granted,
            ctx->my_application_id,
            ctx->my_api_key,
            ctx->access_token,
            ctx->my_application_id,
            str_uid);
    if (ret >= 512)
	{
		MP_ALERT("%s:%u ERROR - snprintf buffer overflow", __func__, __LINE__);
		goto fail;
    	}
	post_data[512-1] = '\0';

	MP_DEBUG("++>launch_authorize post_data: %s\n", post_data);

	if( !web_login_response(ctx, FACEBOOK_UISERVER_URL, resp, post_data ) ) 
	{
		if( ctx->enumState == enumIncorrectUserNameOrPassword ) 
			MP_DEBUG( "Incorrect user name or password in method web_login_response of launch_authorize.\n" );
		else 
		{
			ctx->enumState = enumUnableToConnectToNetwork;
			MP_DEBUG( "Unable to connect to Network in method web_login_response of launch_authorize.\n" );
		}
		goto fail;
	}

	xml_buff_free(resp);
	resp = NULL;
	SAFE_FREE(post_data);

	/* Send GET Extended Permission webpage*/
	url = facebook_realloc(url, 512);
    	if (!url)
    	{
        	errno = -ENOMEM;
        	goto fail;
    	}
	ret = snprintf(url,512,
            "http://www.facebook.com/connect/prompt_permissions.php" 
            "?api_key=%s"
            "&ext_perm=offline_access,user_photos,user_videos,friends_photos,friends_videos"
            "&next=http://m.facebook.com/connect/login_success.html"
            "&cancel=http://m.facebook.com/connect/login_success.html"
            "&display=wap"
            "&locale=en_US"
            "&v=%s"
            "&from_login=1"
            "&legacy_return=1",
            ctx->my_api_key, ctx->my_version);
    	if (ret >= 512)
    	{
        	MP_ALERT("%s:%u ERROR - snprintf buffer overflow", __func__, __LINE__);
        	goto fail;
    	}
    	MP_DEBUG("Confirm API Extended Permission Authorization url: %s\n", url);

	resp = xml_buff_alloc();
	if (!resp)
	{
		errno = -ENOMEM;
		goto fail;
  	}

    	resp = app_auth_get(ctx, url);
    	if (!resp)
    	{	
    		MP_DEBUG("Unable to connect to Network in method app_authorize_get of app_authorize.\n");
    		ctx->enumState = enumUnableToConnectToNetwork;
        	goto fail;
    	}

    	if (!is_linear(resp))
    	{
        	bigger_buff = Merge_ListData_to_SingleBuffer(resp, &len);
        	if (!bigger_buff)
            		goto fail;

        	xml_buff_free(resp);
        	resp = NULL;
        	html_resp = bigger_buff;
    	}
    	else
    	{
        	html_resp = resp->BUFF;
        	len = (resp->buff_len >= sizeof(resp->BUFF)) ? (sizeof(resp->BUFF) - 1) : resp->buff_len;
        	html_resp[len] = '\0';
    	}

	//mpDebugPrint("%s:%u resp len=%d", __func__, __LINE__, resp->buff_len);
	//if (resp->buff_len > 0)
        	//NetAsciiDump(html_resp, resp->buff_len);

	/**
	* parse query string
	*/
	sprintf(pattern,"a href=\"http://www.facebook.com/apps/application.php?id=%s&amp;", ctx->my_application_id);
	start = strstr(html_resp, pattern);
	end = NULL;
	len = 0;
	SAFE_FREE(str_query_string_granted);
	if (start) 
	{
		start += strlen(pattern);
		end = strchr(start , '"');
		if (end) 
		{
			len = (int)(end - start);
			str_query_string_granted = facebook_malloc(len + 1);
			if (str_query_string_granted)
			{
				memcpy(str_query_string_granted, start, len);
				str_query_string_granted[len] = '\0';
            		}
		}
		if (len == 0 ) 
		{
			ctx->enumState = enumIncorrectUserNameOrPassword;
			MP_DEBUG( "Can't get query string from home.php\n");
			goto fail;
		}
	}
	MP_DEBUG("Parse string: %s\n", str_query_string_granted);

	xml_buff_free(resp);
	SAFE_FREE(url);
	if (bigger_buff)
    {
		ext_mem_free(bigger_buff);
		bigger_buff = NULL;
    }
	MP_TRACE_LINE();
	/* Send POST Extended Permission Allow */
    	post_data = facebook_malloc(512);
	if (!post_data)
	{
		errno = -ENOMEM;
		goto fail;
	}

    	ret = snprintf(post_data,512,
            "post_form_id=%s"
            "&fb_dtsg=%s"
            "&app_id=%s"
            "&display=wap"
            "&next=http://m.facebook.com/connect/login_success.html"
            "&cancel_url=http://m.facebook.com/connect/login_success.html"
            "&locale=en_US"
            "&perms=offline_access,user_photos,user_videos,friends_photos,friends_videos"
            "&legacy_return=1"
            "&from_post=1"
            "&__uiserv_method=permissions.request"
            "&grant_clicked=Allow",
            str_post_form_id_granted,
            str_fb_dtsg_granted,
            ctx->my_application_id);
    if (ret >= 512)
	{
		MP_ALERT("%s:%u ERROR - snprintf buffer overflow", __func__, __LINE__);
		goto fail;
    	}
	post_data[512-1] = '\0';

	MP_DEBUG("POST Extended Permission post_data: %s\n", post_data);

	url = facebook_realloc(url, 128);
    	if (!url)
    	{
        	errno = -ENOMEM;
        	goto fail;
    	}
	ret = snprintf(url,128,
           FACEBOOK_UISERVER_URL
           "?%s",
            str_query_string_granted);
    	if (ret >= 128)
    	{
        	MP_ALERT("%s:%u ERROR - snprintf buffer overflow", __func__, __LINE__);
        	goto fail;
    	}
	
	resp = xml_buff_alloc();
	if (!resp)
	{
		errno = -ENOMEM;
		goto fail;
    	}

	if( !app_auth_post(ctx, url, resp, post_data ) ) 
	{
		MP_DEBUG("Unable to connect to Network in method app_authorize_post of launch_authorize.\n");
		ctx->enumState = enumUnableToConnectToNetwork;
		goto fail;
    	}
	
	MP_TRACE_LINE();
	SAFE_FREE(str_fb_dtsg_granted);
	if (*str_uid)
	SAFE_FREE(str_uid);
	SAFE_FREE(str_query_string_granted);
	
done:
	SAFE_FREE(url);
	if (bigger_buff)
		ext_mem_free(bigger_buff);
    	SAFE_FREE(post_data);
	xml_buff_free(resp);
	SAFE_FREE(str_post_form_id_granted);
	return true;
fail:
	SAFE_FREE(url);
	if (bigger_buff)
		ext_mem_free(bigger_buff);
    	SAFE_FREE(post_data);
	xml_buff_free(resp);
	SAFE_FREE(str_post_form_id_granted);
	SAFE_FREE(str_fb_dtsg_granted);
	if (*str_uid)
	SAFE_FREE(str_uid);
	SAFE_FREE(str_query_string_granted);
	return false;
}

int facebook_init(const char *username, const char *base_dir)
{
	//create the facebook
	struct facebook_struct *ctx = facebook_malloc(sizeof(struct facebook_struct));
	fb_album_t *fb_tmp_album_entry;	
	int count=0;
	fb_friend_t *fb_tmp_friend_entry;
	fb_album_t *fb_tmp_friend_album_entry;

	/* save session_key to SD card */
	int cret = 0, ret = 0;
	DRIVE *sdrv;
	STREAM *handle;
	
	if (!ctx)
	{
		errno = -ENOMEM;
		goto fail;
	}

	MP_TRACE_LINE();
	memset(ctx, 0, sizeof(*ctx));
	facebook = ctx;

	ctx->my_api_key = _my_api_key;
	ctx->my_secret = _my_secret;
	ctx->my_application_id = _my_application_id;
	ctx->my_version = _my_version;

	ctx->has_session = false;
	ctx->fb_cur_album = &ctx->fb_album_list;	

	ctx->fb_cur_friend = &ctx->fb_friend_list;

	ctx->fb_friend_cur_album = &ctx->fb_friend_album_list;

	/* setup proxy "username:password" for curl */
    	curl_userpass = facebook_realloc(curl_userpass, strlen(my_proxy_username)+strlen(my_proxy_password)+2);
    	if (!curl_userpass)
    	{
		errno = -ENOMEM;
        	goto fail;
    	}
    	sprintf(curl_userpass, "%s:%s", my_proxy_username, my_proxy_password);
    	MP_DEBUG("Confirm curl_userpass: %s\n", curl_userpass);

	/* read session_key from Facebook_session.txt */
	sdrv = DriveGet( SD_MMC );	
	ret = FileSearch(sdrv, "Facebook_session", "txt", E_FILE_TYPE);
	if (ret == 0)	// Facebook_session.txt exists, to read session_key
	{
		handle = FileOpen(sdrv);
		if(!handle)
			MP_DEBUG(" Fileopen FAIL!!");
		else
		{
			MP_TRACE_LINE();
			DWORD file_size, read_len;			
			file_size= FileSizeGet(handle);
			char *tmp_session_key =  facebook_malloc( file_size );
			MP_DEBUG(" Fileopen OK!!");
			
			read_len = FileRead( handle, tmp_session_key, file_size );
			tmp_session_key[file_size - 1] = '\0';
			MP_DEBUG("file_size= %d, read_len=%d", file_size, read_len);
			MP_DEBUG("session_key= %s", tmp_session_key);
			if( file_size == read_len )
			{
				ctx->session_key= facebook_realloc(ctx->session_key, file_size );
				strncpy(ctx->session_key, tmp_session_key, read_len);
				SAFE_FREE(tmp_session_key);
				
				if (!userGetLoggedInUser(ctx, &ctx->uid ))
				{
					MP_DEBUG( "userGetLoggedInUser failed" );
					ctx->has_session = false;
				}
				else
				{
			    		MP_DEBUG("has_session: uid=%s\n", ctx->uid);
					ctx->has_session = true;
				}
			}			
			FileClose( handle );

			if (ctx->has_session)                       /* have a valid session key */
				goto get_album;
		}			
	}
	else
		MP_DEBUG("Facebook_session.txt doesn't exist!");
	

	/*
	 * method 3 to combine the the flow to authorize application
	 * step 1 : create token (get Token)
	 * step 2 : verify authorize application, set extended_permission of offline_access
	 * step 3 : get session and store inportant parameters
	 * step 4 : get User's Friend info/get User's Albums and Photos/get Friend's Albums and Photots
	 */

	mpDebugPrint( "=====step 1 : always start with Auth.createToken=====\n" );
    	if (!fb_authenticate(ctx))
    	{
		MP_DEBUG( "Unable to connect to facebook" );
		ctx->enumState = enumUnableToConnectToFacebook;
        	goto fail;
    	}

    	char *str_email = str_fb_email;
	char *str_pass = str_fb_pass;
	MP_DEBUG("User email: %s", str_email );
	MP_DEBUG("User pass: %s\n", str_pass );

	mpDebugPrint( "=====step 2 : verify authorize application=====\n" );
	/**
	 * Using web login to authentication and authorization.
	 * Include Extended Permission request.
	 */
	if( !launch_authorize( ctx, str_email, str_pass ) ) 
	{
		if ( ctx->enumState == enumUnableToConnectToFacebook )
			MP_DEBUG( "Unable to connect to facebook\n" );
		else if ( ctx->enumState == enumIncorrectUserNameOrPassword )
			MP_DEBUG( "Incorrect user name or password\n" );
        	else
			MP_DEBUG( "launch_authorize failed\n" );

		ctx->enumState = enumUnableToConnectToFacebook;
		goto fail;
	}

	mpDebugPrint( "=====step 3 : starting Auth.getSession=====\n" );
	if( !fb_get_session(ctx) )
	{
		ctx->enumState = enumUnableToConnectToFacebook;
		MP_DEBUG("[fb_get_session]Unable to connect to facebook.\n");
		goto fail;
	}
	else
	{
		mpDebugPrint("Success to get session key=%s, key len=%d\n", ctx->session_key, strlen(ctx->session_key));
		ctx->has_session = true;
		/* save session_key to SD card */
		sdrv = DriveGet( SD_MMC );		
		ret = FileSearch(sdrv, "Facebook_session", "txt", E_FILE_TYPE);
		if ( ret != 0 )
		{
			cret = CreateFile(sdrv, "Facebook_session", "txt"); 
			TaskYield();
			if(cret)
			{
				MP_DEBUG(" CreateFile FAIL!!");
				goto get_album;
			}
			else
				MP_DEBUG(" CreateFile OK!!");
		}

		handle = FileOpen(sdrv);
		if(!handle)
		{
			MP_DEBUG(" Fileopen FAIL!!");
			goto get_album;
		}
		else
		{
			MP_DEBUG(" Fileopen OK!!");

			ctx->session_key[strlen(ctx->session_key)] = '\0';
			ret = FileWrite( handle, ctx->session_key, strlen(ctx->session_key)+1 );
			if(!ret)
				MP_DEBUG(" Filewrite FAIL!!");
			else
				MP_DEBUG(" Filewrite OK!!");

			FileClose( handle );
		}		
	}

get_album:
	mpDebugPrint( "=====step 4 : api calls with permanently session=====\n" );

	mpDebugPrint( "-----section 1 : get friend's uid-----\n" );
	/* 11-15 every time need to update user's friends info */
	if( !fb_get_friends(ctx) )
	{
		ctx->enumState = enumUnableToConnectToFacebook;
		MP_DEBUG("[fb_get_friends]Unable to connec to facebook.\n");
		goto fail;
	}
	else
	{
		fb_tmp_friend_entry = ctx->fb_friend_list.next;
		while(fb_tmp_friend_entry)
		{
			fb_tmp_friend_entry->friend_num = fb_friend_num;
			fb_tmp_friend_entry = fb_tmp_friend_entry->next;
		}		
	}

	mpDebugPrint( "-----section 2 : get/show user's album list-----\n" );
	if( !fb_get_album_list(ctx) )
	{
		ctx->enumState = enumUnableToConnectToFacebook;
		MP_DEBUG("[fb_get_album_list]Unable to connec to facebook.\n");
		goto fail;
	}
	else
	{
		/* Show User's Album list */
		fb_tmp_album_entry = ctx->fb_album_list.next;

		while (fb_tmp_album_entry)
		{
			fb_tmp_album_entry->album_num = fb_user_album_num;
			
			MP_DEBUG("===========================================\n");
			MP_DEBUG("The Album's aid= %s ;len=%d\n", fb_tmp_album_entry->aid, fb_tmp_album_entry->aid_len);
			MP_DEBUG("The Album's cover_pid= %s\n", fb_tmp_album_entry->cover_pid);
			MP_DEBUG("The Album's name= %s\n", fb_tmp_album_entry->name);
			MP_DEBUG("The Album's link= %s\n", fb_tmp_album_entry->link);
			MP_DEBUG("The Album have %d photos.\n", fb_tmp_album_entry->photo_num );
			MP_DEBUG("===========================================\n");
			MP_TRACE_LINE();			
			Net_Xml_PhotoSetList(fb_tmp_album_entry->name,count);	
			count ++;
			fb_tmp_album_entry = fb_tmp_album_entry->next;
        	}
		MP_TRACE_LINE();
		Net_PhotoSet_SetCount(count);
	}

	/* If the user have friends
	* To show friend's Album list.
	*/
	mpDebugPrint( "-----section 3 : show friend's album list-----\n" );
	if((ctx->fb_friend_list.next) && (fb_friend_num > 0))
	{
		fb_tmp_friend_entry = ctx->fb_friend_list.next;
		while(fb_tmp_friend_entry)
		{
			MP_DEBUG("check friend_uid= %s\n", fb_tmp_friend_entry->friend_uid);
			/* To get Friend's Album info */
			if( !fb_get_friend_album_list(ctx, fb_tmp_friend_entry->friend_uid) )
			{
				ctx->enumState = enumUnableToConnectToFacebook;
				MP_DEBUG("[fb_get_friend_album_list]Unable to connec to facebook.\n");
				goto fail;
			}
			else
			{	/* allocate album_num to right user */
				fb_tmp_friend_album_entry = ctx->fb_friend_album_list.next;
				while(fb_tmp_friend_album_entry)
				{
					if( !strcasecmp(fb_tmp_friend_album_entry->owner, fb_tmp_friend_entry->friend_uid) )
						fb_tmp_friend_album_entry->album_num = fb_tmp_friend_entry->friend_album_num;
					fb_tmp_friend_album_entry = fb_tmp_friend_album_entry->next;
				}
				MP_TRACE_LINE();
			}
			fb_tmp_friend_entry = fb_tmp_friend_entry->next;
		}
		/* Show Friend's Album list */
		fb_tmp_friend_album_entry = ctx->fb_friend_album_list.next;
		while(fb_tmp_friend_album_entry)
		{
			MP_DEBUG("===========================================\n");
			MP_DEBUG("Friend %s have %d albums\n", fb_tmp_friend_album_entry->owner, fb_tmp_friend_album_entry->album_num);
			MP_DEBUG("The Friend Album's aid= %s ;len=%d\n", fb_tmp_friend_album_entry->aid, fb_tmp_friend_album_entry->aid_len);
			MP_DEBUG("The Friend Album's cover_pid= %s\n", fb_tmp_friend_album_entry->cover_pid);
			MP_DEBUG("The Friend Album's name= %s\n", fb_tmp_friend_album_entry->name);
			MP_DEBUG("The Friend Album's link= %s\n", fb_tmp_friend_album_entry->link);
			MP_DEBUG("The Friend Album have %d photos.\n", fb_tmp_friend_album_entry->photo_num );
			MP_DEBUG("===========================================\n");
			MP_TRACE_LINE();
			Net_Xml_PhotoSetList(fb_tmp_friend_album_entry->name,count);
			count ++;
			fb_tmp_friend_album_entry = fb_tmp_friend_album_entry->next;
        	}
		MP_TRACE_LINE();
		Net_PhotoSet_SetCount(count);
	}

	MP_TRACE_LINE();

    	return 0;
fail:
	return -1;
}

void facebook_exit(const char *base_dir)
{
	fb_album_t     *tmp_entry;
	/* free resources allocated for all album */
	MP_TRACE_LINE();
	facebook->fb_cur_album = facebook->fb_album_list.next;
	facebook->fb_friend_cur_album = facebook->fb_friend_album_list.next;
	
	while (facebook->fb_cur_album)
	{
		tmp_entry = facebook->fb_cur_album;
		facebook->fb_cur_album = facebook->fb_cur_album->next;
		free_album(tmp_entry);
		SAFE_FREE(tmp_entry);
	}

	while(facebook->fb_friend_cur_album)
	{
		tmp_entry = facebook->fb_friend_cur_album;
		facebook->fb_friend_cur_album = facebook->fb_friend_cur_album->next;
		free_album(tmp_entry);
		SAFE_FREE(tmp_entry);
	}

	//free resources allocated for friend info
	SAFE_FREE(facebook->fb_cur_friend->friend_uid);
	
	MP_TRACE_LINE();
	SAFE_FREE(str_post_form_id_granted);
	SAFE_FREE(str_fb_dtsg_granted);
	SAFE_FREE(curl_userpass);
	MP_TRACE_LINE();
	if (facebook)
	{
		if (facebook->curl)
			curl_easy_cleanup( facebook->curl );
		SAFE_FREE(facebook->session_key);
		SAFE_FREE(facebook->uid);
		SAFE_FREE(facebook->session_secret);
		SAFE_FREE(facebook);
	}
	MP_TRACE_LINE();
	fb_user_album_num = 0;
	fb_friend_num = 0;
	fb_friend_album_num = 0;
}

static void free_album(const fb_album_t  *tmp_entry)
{
	if (!tmp_entry)
		return;
	facebook_free(tmp_entry->aid);
	facebook_free(tmp_entry->cover_pid);
	facebook_free(tmp_entry->owner);
	facebook_free(tmp_entry->name);
	facebook_free(tmp_entry->link);
}

int Facebook_Cleanup(char *api_key, char *secret, char *app_id, char *ver)
{
    struct facebook_struct *ctx = facebook_malloc(sizeof(struct facebook_struct));

    return 0;
fail:
    return -1;
}

/* send an HTTP request */
static int request(struct facebook_struct *ctx, char *method, void *resp)
{
	char *url = facebook_malloc(512);
	char *params = facebook_malloc(256);
	CURL *curl = ctx->curl;
	CURLcode res;

	MP_TRACE_LINE();
    	if (!url || !params)
		goto fail;

    	curlSetProxy(curl, my_proxy_host, my_proxy_port, my_proxy_username, my_proxy_password );
	
	if( !strcasecmp(method, "Auth.createToken") )
	{
		snprintf(params, 256,
			"api_key=%slocale=en_USmethod=%sv=%s",
			ctx->my_api_key, 
			method, 
			ctx->my_version);
		snprintf(url, 512,
			"http://%s/restserver.php"
			"?api_key=%s"
			"&locale=en_US"
			"&method=%s"
			"&sig=%s"
			"&v=%s",
			FACEBOOK_API_HOST, 
			ctx->my_api_key, 
			method, 
			facebook_get_signature(ctx, params),
			ctx->my_version);
	}
	else if( !strcasecmp(method, "auth.getSession") )
	{
		/* The params needs sort by yourself.*/
		snprintf(params, 256,
			"api_key=%sauth_token=%sgenerate_session_secret=1locale=en_USmethod=%sv=%s",
			ctx->my_api_key,
			ctx->access_token, 
			method, 
			ctx->my_version);
		snprintf(url, 512,
			"http://%s/restserver.php"
			"?api_key=%s"
			"&auth_token=%s"
			"&generate_session_secret=1"
			"&locale=en_US"
			"&method=%s"
			"&sig=%s"
			"&v=%s",
			FACEBOOK_API_HOST, 
			ctx->my_api_key, 
			ctx->access_token,
			method, 
			facebook_get_signature(ctx, params),
			ctx->my_version);
	}
	else if( (!strcasecmp(method, "Photos.getAlbums") ) && (!str_friend_uid) )
	{
		snprintf(params, 256,
			"api_key=%slocale=en_USmethod=%ssession_key=%suid=%sv=%s",
			ctx->my_api_key,
			method, 
			ctx->session_key,
			ctx->uid,
			ctx->my_version);
		snprintf(url, 512,
			"http://%s/restserver.php"
			"?api_key=%s"
			"&locale=en_US"
			"&method=%s"
			"&session_key=%s"
			"&uid=%s"
			"&v=%s"
			"&sig=%s",
			FACEBOOK_API_HOST, 
			ctx->my_api_key, 
			method,
			ctx->session_key,
			ctx->uid,
			ctx->my_version,
			facebook_get_signature(ctx, params) );
	}
	else if( !strcasecmp( method, "Photos.get" ) )
	{
		if( !str_album_aid )
			goto fail;
		
		snprintf(params, 256,
			"aid=%sapi_key=%smethod=%ssession_key=%sv=%s",
			str_album_aid,
			ctx->my_api_key,
			method, 
			ctx->session_key,
			ctx->my_version);
		snprintf(url, 512,
			"http://%s/restserver.php"
			"?aid=%s"
			"&api_key=%s"
			"&method=%s"
			"&session_key=%s"
			"&v=%s"
			"&sig=%s",
			FACEBOOK_API_HOST,
			str_album_aid,
			ctx->my_api_key, 
			method,
			ctx->session_key,
			ctx->my_version,
			facebook_get_signature(ctx, params) );
	}
	else if( !strcasecmp( method, "users.getLoggedInUser" ) )
	{
		snprintf(params, 256,
			"api_key=%smethod=%ssession_key=%sv=%s",
			ctx->my_api_key,
			method, 
			ctx->session_key,
			ctx->my_version);
		snprintf(url, 512,
			"http://%s/restserver.php"
			"?api_key=%s"
			"&method=%s"
			"&session_key=%s"
			"&v=%s"
			"&sig=%s",
			FACEBOOK_API_HOST,
			ctx->my_api_key, 
			method,
			ctx->session_key,
			ctx->my_version,
			facebook_get_signature(ctx, params) );
	}
	else if( !strcasecmp(method, "Friends.get") )
	{
		snprintf(params, 256,
			"api_key=%smethod=%ssession_key=%suid=%sv=%s",
			ctx->my_api_key,
			method, 
			ctx->session_key,
			ctx->uid,
			ctx->my_version);
		snprintf(url, 512,
			"http://%s/restserver.php"
			"?api_key=%s"
			"&method=%s"
			"&session_key=%s"
			"&uid=%s"
			"&v=%s"
			"&sig=%s",
			FACEBOOK_API_HOST, 
			ctx->my_api_key, 
			method,
			ctx->session_key,
			ctx->uid,
			ctx->my_version,
			facebook_get_signature(ctx, params) );
	}
	else if( (!strcasecmp(method, "Photos.getAlbums") ) && str_friend_uid )
	{		
		snprintf(params, 256,
			"api_key=%slocale=en_USmethod=%ssession_key=%suid=%sv=%s",
			ctx->my_api_key,
			method, 
			ctx->session_key,
			str_friend_uid,
			ctx->my_version);
		snprintf(url, 512,
			"http://%s/restserver.php"
			"?api_key=%s"
			"&locale=en_US"
			"&method=%s"
			"&session_key=%s"
			"&uid=%s"
			"&v=%s"
			"&sig=%s",
			FACEBOOK_API_HOST, 
			ctx->my_api_key, 
			method,
			ctx->session_key,
			str_friend_uid,
			ctx->my_version,
			facebook_get_signature(ctx, params) );
	}
    	MP_DEBUG("url: %s\n", url );

	MP_TRACE_LINE();
    	curl_easy_setopt( curl, CURLOPT_URL, url );
	
	struct curl_slist *headers=NULL;
	headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

	/* pass our list of custom made headers */
	curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers);
	
    	/* assign callback function & store data */
    	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, my_write_func );
    	curl_easy_setopt( curl, CURLOPT_WRITEDATA, resp );
    	curl_easy_setopt( curl, CURLOPT_TIMEOUT, CONNECTION_TIME_OUT );

	MP_TRACE_LINE();
    	res = curl_easy_perform( curl );

    	MP_DEBUG( "HTTP request's result : %d\n", res );

	/* free the header list */
    	curl_slist_free_all(headers);

	int http_code = 0;
	curl_easy_getinfo ( curl, CURLINFO_HTTP_CODE, &http_code );
	MP_DEBUG("http_code: %d\n", http_code);
	if(  !( http_code == 200 || http_code == 302 ) )
		goto fail;
	
	if (res != CURLE_OK)
		goto fail;
	
    	SAFE_FREE(params);
    	SAFE_FREE(url);
    	return 0;
    	
fail:
	MP_TRACE_LINE();
    	SAFE_FREE(params);
    	SAFE_FREE(url);
    	return -1;
}

// GET /restserver.php?api_key=%s&locale=en_US&method=Auth.createToken&sig=%s&v=1.0 HTTP/1.1

/* calc the sig */
char *facebook_get_signature(struct facebook_struct *ctx, char *str)
{
    	char *md5_in;
    	int len;
    	uint8_t md5_out[16];
    	static unsigned char ha1[33];

	MP_TRACE_LINE();
	len = strlen(str) + strlen(ctx->my_secret) + 2;
	md5_in = facebook_malloc(len);

    	snprintf(md5_in, len, "%s%s",
            str, ctx->my_secret);

#if LOCAL_DEBUG_ENABLE
    	NetAsciiDump(md5_in, strlen(md5_in));
#endif
    	Curl_md5it(md5_out, md5_in);
//            __asm("break 100");

    	facebook_free(md5_in);

    	md5_to_ascii(md5_out, ha1);
	
    	return ha1;
}

static XML_BUFF_link_t *xml_buff_alloc(void)
{
    XML_BUFF_link_t *ptr;
    ptr = (XML_BUFF_link_t *)ext_mem_malloc(sizeof(XML_BUFF_link_t));
    if (!ptr)
    {
        BREAK_POINT();
        return NULL;
    }

    ptr->link = NULL;
    ptr->buff_len = 0;	
    ptr->BUFF[0] = '\0';	

    return ptr;
}

static void xml_buff_free(XML_BUFF_link_t *xp)
{
	XML_BUFF_link_t *next;
	
	while(xp)
	{
		next = xp->link;
		ext_mem_free(xp);
		xp = next;		
	}
}

static void mpx_xml_parse(void * userdata
        ,XML_BUFF_link_t *resp
        ,XML_StartElementHandler start_handler
        ,XML_EndElementHandler end_handler
        ,XML_CharacterDataHandler content_handler)
{
	int isLast = FALSE;
	XML_BUFF_link_t *ptr;
	XML_Parser parser = XML_ParserCreate(NULL);

	XML_SetUserData(parser, userdata);
	XML_SetElementHandler(parser, start_handler, end_handler);
	XML_SetCharacterDataHandler(parser, content_handler);

	MP_DEBUG("mpx_xml_parse: XML response=");
	NetAsciiDump(resp->BUFF, resp->buff_len);
	ptr = (XML_BUFF_link_t *)resp;
	while (ptr)
	{
		if (!ptr->link)
			isLast = TRUE;
		if (XML_Parse(parser, ptr->BUFF, ptr->buff_len, isLast) == XML_STATUS_ERROR)
		{
			MP_DEBUG("mpx_xml_parse: %s at line %d, column %d\n",
			XML_ErrorString(XML_GetErrorCode(parser)),
			XML_GetCurrentLineNumber(parser),
			XML_GetCurrentColumnNumber(parser));
            		break;
        	}
        	ptr = ptr->link;
    	}

    	XML_ParserFree(parser);
}

static void fb_start_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t *info = (fb_info_t *) user_data;

	if ( !strcasecmp(name, "Auth_createToken_response") )
	{
		info->state = FB_AUTH_CREATETOKEN_RESPONSE;
	}
}

static void fb_end_element_handler(void *user_data, const char *name)
{
	fb_info_t *info = (fb_info_t *) user_data;

	if (info->state == FB_AUTH_CREATETOKEN_RESPONSE )
	{
		info->state = FB_START;
	}
}

static void fb_content_handler(void *user_data, const char *s, int len)
{
	fb_info_t *info = (fb_info_t *) user_data;
	struct facebook_struct *ctx = (struct facebook_struct *)info->context;

	if (info->state == FB_AUTH_CREATETOKEN_RESPONSE)
	{
		if (len >= sizeof(my_access_token))
			len = sizeof(my_access_token) - 1;
		memcpy(my_access_token, s, len);
		my_access_token[len] = '\0';
		ctx->access_token = my_access_token;
		MP_DEBUG("Access Token=%s", my_access_token);
	}
}

static XML_BUFF_link_t  *app_auth_get(struct facebook_struct *ctx, char *url)
{
	int ret;
	fb_info_t	fb_info;
	XML_BUFF_link_t *resp;
	CURL *curl = ctx->curl;
	CURLcode res;

	MP_TRACE_LINE();
	curl_easy_reset( ctx->curl );
	curlSetProxy(ctx->curl, my_proxy_host, my_proxy_port, my_proxy_username, my_proxy_password );
	resp = xml_buff_alloc();
	if (!resp)
    	{
		errno = -ENOMEM;
        	goto fail;
	}

	/* make url */
	curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt( curl, CURLOPT_MAXREDIRS, 2);
	curl_easy_setopt( curl, CURLOPT_URL, url );

	MP_TRACE_LINE();
	struct curl_slist *headers=NULL;
	/* pass our list of custom made headers */
	curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, my_write_func );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, resp );
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, CONNECTION_TIME_OUT );

	MP_TRACE_LINE();
	res = curl_easy_perform( curl );

	/* free the header list */
	curl_slist_free_all(headers);	

    	MP_DEBUG( "HTTP request's (app_auth_get) result : %d\n", res );

	int http_code = 0;
	curl_easy_getinfo ( curl, CURLINFO_HTTP_CODE, &http_code );
	MP_DEBUG("http_code: %d\n", http_code);
	if(  !( http_code == 200 ) )	
		res = CURLE_OBSOLETE;
	
	if (res != CURLE_OK)
      		goto fail;
	
	return resp;
fail:
	xml_buff_free(resp);
	return NULL;
}

static int app_auth_post(struct facebook_struct *ctx, char *url, XML_BUFF_link_t *result, char *post_data)	
{
	int     ret;
	fb_info_t fb_info;
	CURLcode res = CURLE_OBSOLETE;
	CURL *curl = ctx->curl;

	curl_easy_reset( curl );
	curlSetProxy(curl, my_proxy_host, my_proxy_port, my_proxy_username, my_proxy_password );

	curl_easy_setopt( curl, CURLOPT_URL, url );

	/* Tell libcurl to *not* verify the peer, to resolve peer SSL Certificate Verification */
	curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 2 );

	/* init to NULL is important */
	struct curl_slist *headers=NULL;
	curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers);

	if (*post_data)
	{
	/* if not empty */
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(post_data));
	}
	else
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);

	/* assign callback function & store data */
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, my_write_func );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, result );
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, CONNECTION_TIME_OUT );	
	res = curl_easy_perform( curl );
	
	MP_DEBUG( "HTTP request's (app_auth_post) result : %d\n", res );

	/* free the header list */
	curl_slist_free_all(headers);

	int http_code = 0;
	curl_easy_getinfo ( curl, CURLINFO_HTTP_CODE, &http_code );
	MP_DEBUG("http_code: %d\n", http_code);
	if(  !( http_code == 200 || http_code == 302 ) )
		res = CURLE_OBSOLETE;

	return (res == 0);

}

bool fb_get_session(struct facebook_struct *ctx)	
{
	int ret;
	XML_BUFF_link_t *resp;
	int len;
	unsigned char *bigger_buff = NULL, *html_resp = NULL;

	/* Send GET Auth.getSession request */
	resp = xml_buff_alloc();
	if (!resp)
	{
		errno = -ENOMEM;
		goto fail;
	}

	curl_easy_reset( ctx->curl );

	MP_TRACE_LINE();
	ret = request(ctx, "auth.getSession", resp);
	if (ret < 0)
		goto fail;
	
	if (!is_linear(resp))
    	{
        	bigger_buff = Merge_ListData_to_SingleBuffer(resp, &len);
        	if (!bigger_buff)
            		goto fail;

        	xml_buff_free(resp);
        	resp = NULL;
        	html_resp = bigger_buff;
    	}
    	else
    	{
        	html_resp = resp->BUFF;
        	len = (resp->buff_len >= sizeof(resp->BUFF)) ? (sizeof(resp->BUFF) - 1) : resp->buff_len;
        	html_resp[len] = '\0';
    	}

	//mpDebugPrint("%s:%u resp len=%d", __func__, __LINE__, resp->buff_len);
	//if (resp->buff_len > 0)
        	//NetAsciiDump(html_resp, resp->buff_len);

	/* parse xml response for "session_key", "uid" and "secret". */
	char *pattern = "<session_key>";
	char *start = strstr(html_resp, pattern);
	char *end = NULL;
	len = 0;
	SAFE_FREE(ctx->session_key);
	if (start)
	{
		start += strlen(pattern);
		end = strchr(start , '<');
		if (end) 
		{
			len = (int)(end - start);
			ctx->session_key= facebook_realloc(ctx->session_key, len+1 );
			if (ctx->session_key)
			{
				memcpy(ctx->session_key, start, len);
				ctx->session_key[len] = '\0';
            		}
        	}
    	}
	else 
		goto fail;
	MP_DEBUG("Session Key= %s\n", ctx->session_key);
	if (!ctx->session_key)
		goto fail;

	pattern = "<uid>";
	start = strstr(html_resp, pattern);
	end = NULL;
	if(start)
	{
		start += strlen(pattern);
		end = strchr(start, '<');
		if(end)
		{
			len = (int)(end - start);
			ctx->uid= facebook_realloc(ctx->uid, len+1 );
			if (ctx->uid)
			{
				memcpy(ctx->uid, start, len);
				ctx->uid[len] = '\0';
            		}
		}
	}
	else
		goto fail;
	MP_DEBUG("Uid= %s\n", ctx->uid);
	if (!ctx->uid)
		goto fail;

	pattern = "<secret>";
	start = strstr(html_resp, pattern);
	end = NULL;
	if(start)
	{
		start += strlen(pattern);
		end = strchr(start, '<');
		if(end)
		{
			len = (int)(end - start);
			ctx->session_secret= facebook_realloc(ctx->session_secret, len+1 );
			if (ctx->session_secret)
			{
				memcpy(ctx->session_secret, start, len);
				ctx->session_secret[len] = '\0';
            		}
		}
	}
	else 
		goto fail;
	MP_DEBUG("Session Secret= %s\n", ctx->session_secret);
	if (!ctx->session_secret)
		goto fail;

	if (bigger_buff)
		ext_mem_free(bigger_buff);
	xml_buff_free(resp);
	resp=NULL;
	ctx->enumState = enumStateNormal;
	return true;
fail:
	if (bigger_buff)
		ext_mem_free(bigger_buff);
	xml_buff_free(resp);
	resp=NULL;
	return false;
}

bool fb_get_album_list(struct facebook_struct *ctx)
{
	XML_BUFF_link_t *resp;
	int ret;
	fb_info_t	fb_info;
	
	/* Send GET Photos.getAlbums request */
	resp = xml_buff_alloc();
	if (!resp)
	{
		errno = -ENOMEM;
		goto fail;
	}
	
	curl_easy_cleanup( ctx->curl );
	ctx->curl = 0;
	ctx->curl = curl_easy_init();

	MP_TRACE_LINE();
	ret = request(ctx, "Photos.getAlbums", resp);
	if (ret < 0)
		goto fail;
	
	/* parse "Photos.getAlbums" xml response */
	memset(&fb_info, 0, sizeof(fb_info));
	fb_info.state = FB_ALBUM_LIST_INIT;
	fb_info.context = ctx;

	mpx_xml_parse(&fb_info
            , resp 	
            , fb_album_list_start_element_handler 	
            , fb_album_list_end_element_handler
            , fb_album_list_content_handler);
    	
	MP_DEBUG("%s have %d albums\n", ctx->uid, fb_user_album_num);

	if( fb_user_album_num == 0 ) 
	{
		ctx->enumState = enumNoFacebookContentFound;
		MP_DEBUG( "Well, the API have not authorized to access the user's albums and photos. \n" );
	}
	//because Photos.getAlbums return albums include Profile Pictures
	else if( fb_user_album_num == 1 )
	{
		ctx->enumState = enumNoFacebookContentFound;
		MP_DEBUG( "The user have no albums!\n" );
	}
	
	xml_buff_free(resp);
	resp=NULL;
    	curl_easy_cleanup( ctx->curl );
	ctx->curl=0;
	ctx->enumState = enumStateNormal;
	return true;

fail:
	xml_buff_free(resp);
	resp=NULL;
    	fb_user_album_num=0;
    	curl_easy_cleanup( ctx->curl );
	ctx->curl=0;
	return false;
}

static void fb_album_list_start_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t	*info = (fb_info_t *) user_data;
	struct facebook_struct *ctx = (struct facebook_struct *)info->context;	
//	mpDebugPrint("start name= %s\n", name);
	if( info->state == FB_ALBUM_LIST_INIT )
	{
		if( !strcasecmp(name, "album") )
		{
			fb_user_album_num++;			
			MP_DEBUG("Album[ %d ] have found!\n", fb_user_album_num-1 );
			
			info->state = FB_ALBUM_LIST_FOUND;

			//because Photos.getAlbums return albums include Profile Pictures
			if( fb_user_album_num > 1)
			{
				/* append album list */
				fb_album_t *fb_album_entry;
				fb_album_entry = (fb_album_t *) facebook_malloc(sizeof(fb_album_t));
				MP_ASSERT(fb_album_entry);
				memset(fb_album_entry, 0, sizeof(fb_album_t));
				ctx->fb_cur_album->next = fb_album_entry;
				ctx->fb_cur_album= fb_album_entry;
			}
		}			
	}
	else if( info->state == FB_ALBUM_LIST_FOUND )
	{
		if( !strcasecmp(name, "aid"))
		{
			info->state = FB_ALBUM_AID_FOUND;
		}
		else if( !strcasecmp(name, "cover_pid") )
		{
			info->state = FB_ALBUM_COVER_PID_FOUND;
		}
		else if( !strcasecmp(name, "owner") )
		{
			info->state = FB_ALBUM_OWNER_FOUND;
		}
		else if( !strcasecmp(name, "name") )
		{
			info->state = FB_ALBUM_NAME_FOUND;
		}
		else if( !strcasecmp(name, "link") )
		{
			info->state = FB_ALBUM_LINK_FOUND;
		}
		else if( !strcasecmp(name, "size") )
		{
			info->state = FB_PHOTO_NUM_FOUND; 
		}
	}
}

static void fb_album_list_end_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t	*info = (fb_info_t *) user_data;
//	mpDebugPrint("end name= %s\n", name);
	if( info->state == FB_ALBUM_LIST_FOUND )
	{
		if( !strcasecmp( name, "album" ) )
			info->state = FB_ALBUM_LIST_INIT;
	}
	else if( info->state == FB_ALBUM_AID_FOUND )
	{
		if( !strcasecmp( name, "aid" ) )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
	else if( info->state == FB_ALBUM_COVER_PID_FOUND )
	{
		if( !strcasecmp( name, "cover_pid" ) )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
	else if( info->state == FB_ALBUM_OWNER_FOUND )
	{
		if( !strcasecmp(name, "owner") )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
	else if( info->state == FB_ALBUM_NAME_FOUND )
	{
		if( !strcasecmp( name, "name" ) )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
	else if( info->state == FB_ALBUM_LINK_FOUND )
	{
		if( !strcasecmp( name, "link" ) )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
	else if( info->state == FB_PHOTO_NUM_FOUND )
	{
		if( !strcasecmp( name, "size" ) )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
}

static void fb_album_list_content_handler(void *user_data, const char *s, int len)
{
	fb_info_t	*info = (fb_info_t *) user_data;
	struct facebook_struct *ctx = (struct facebook_struct *)info->context;
	
//	mpDebugPrint("s= %s\n", s);
//	mpDebugPrint("len= %d\n", len);
	if( (info->state == FB_ALBUM_AID_FOUND) && ctx->fb_cur_album )
	{
		//because Photos.getAlbums return albums include Profile Pictures
		if( fb_user_album_num > 1 )
		{
			ctx->fb_cur_album->aid = facebook_malloc(len+1);
			memcpy( ctx->fb_cur_album->aid, s, len );
			ctx->fb_cur_album->aid[len] = '\0';
			MP_DEBUG("aid= %s\n", ctx->fb_cur_album->aid);
			ctx->fb_cur_album->aid_len = len;
		}
	}
	else if( ( info->state == FB_ALBUM_COVER_PID_FOUND ) && ctx->fb_cur_album )
	{
		if( fb_user_album_num > 1 )
		{
			ctx->fb_cur_album->cover_pid= facebook_malloc(len+1);
			memcpy( ctx->fb_cur_album->cover_pid, s, len );
			ctx->fb_cur_album->cover_pid[len] = '\0';
			MP_DEBUG("cover_pid= %s\n", ctx->fb_cur_album->cover_pid);
		}
	}
	else if( (info->state == FB_ALBUM_OWNER_FOUND ) && ctx->fb_cur_album )
	{
		if( fb_user_album_num > 1 )
		{
			ctx->fb_cur_album->owner = facebook_malloc(len+1);
			memcpy( ctx->fb_cur_album->owner, s, len );
			ctx->fb_cur_album->owner[len] = '\0';
			MP_DEBUG("owner= %s\n", ctx->fb_cur_album->owner);
		}
	}
	else if( ( info->state == FB_ALBUM_NAME_FOUND ) && ctx->fb_cur_album )
	{
		if( fb_user_album_num > 1 )
		{
			char *tmp_album_name;
			char *append = "_";
			tmp_album_name = facebook_malloc(len+1);
			memcpy(tmp_album_name, s, len);
			tmp_album_name[len] = '\0';
			ctx->fb_cur_album->name = facebook_malloc(len+strlen(ctx->fb_cur_album->owner)+2);
			sprintf(ctx->fb_cur_album->name, "%s%s%s", ctx->fb_cur_album->owner, append, tmp_album_name);
			ctx->fb_cur_album->name[len+strlen(ctx->fb_cur_album->owner)+1] = '\0';
			MP_DEBUG("name= %s\n", ctx->fb_cur_album->name);
			SAFE_FREE(tmp_album_name);
		}
	}
	else if( ( info->state == FB_ALBUM_LINK_FOUND ) && ctx->fb_cur_album )
	{
		if( fb_user_album_num > 1 )
		{
			if( strstr(s, "http://www.facebook.com/album.php?aid=") && (len  > 40))
			{
				char *temp_link = facebook_malloc( len + 1);
				ctx->fb_cur_album->link = facebook_malloc( len + strlen(ctx->uid) + 5 );
				memcpy( temp_link, s, len );			
				temp_link[len] = '\0';
				snprintf(ctx->fb_cur_album->link, (len + strlen(ctx->uid) + 5), "%s&id=%s", temp_link, ctx->uid );
				SAFE_FREE(temp_link);
			}
		}
	}
	else if( ( info->state == FB_PHOTO_NUM_FOUND ) && ctx->fb_cur_album )
	{
		if( fb_user_album_num > 1 )
		{
			char *str = facebook_malloc(len+1);
			memcpy( str, s, len );
			ctx->fb_cur_album->photo_num = atoi(str);
			MP_DEBUG("photo_num= %d\n", ctx->fb_cur_album->photo_num);
			facebook_free(str);
		}
	}
}

int Facebook_PhotoList_Get(BYTE *PhotoSet)
{
	struct facebook_struct *pctx;
	int ret;
	
	EnableNetWareTask();
	pctx = facebook;
	pctx->fb_cur_album = pctx->fb_album_list.next;
	pctx->fb_friend_cur_album = pctx->fb_friend_album_list.next;

	//MP_DEBUG("Operator select %s\n", PhotoSet);

	while(pctx->fb_cur_album)
	{
		if(!strcasecmp(pctx->fb_cur_album->name, PhotoSet))
		{
			ret = fb_get_photo_list(pctx, 0);
			if (ret < 0)
			{
				MP_DEBUG("error code: %d\n", ret);
				return ret;
			}
			break;
		}
		pctx->fb_cur_album = pctx->fb_cur_album->next;
	}
	while(pctx->fb_friend_cur_album)
	{
		if( !strcasecmp(pctx->fb_friend_cur_album->name, PhotoSet) )
		{
			ret = fb_get_photo_list(pctx, 1);
			if (ret < 0)
			{
				MP_DEBUG("error code: %d\n", ret);
				return ret;
			}
			break;
		}
		pctx->fb_friend_cur_album = pctx->fb_friend_cur_album->next;
	}
	return 0;
}

int fb_get_photo_list(struct facebook_struct *ctx, int user_friend_flag)
{
	XML_BUFF_link_t *resp = NULL;
	int ret;
	fb_info_t	fb_info;

	if( !user_friend_flag )
	{
	MP_DEBUG("The select Album's aid= %s; %d\n", ctx->fb_cur_album->aid, ctx->fb_cur_album->aid_len);
	SAFE_FREE( str_album_aid );
	str_album_aid = facebook_realloc(str_album_aid, ctx->fb_cur_album->aid_len+1 );
	memcpy(str_album_aid, ctx->fb_cur_album->aid, ctx->fb_cur_album->aid_len);
	str_album_aid[ctx->fb_cur_album->aid_len] = '\0';
	}
	else
	{
		MP_DEBUG("The select Album's aid= %s; %d\n", ctx->fb_friend_cur_album->aid, ctx->fb_friend_cur_album->aid_len);
		SAFE_FREE( str_album_aid );
		str_album_aid = facebook_realloc(str_album_aid, ctx->fb_friend_cur_album->aid_len+1 );
		memcpy(str_album_aid, ctx->fb_friend_cur_album->aid, ctx->fb_friend_cur_album->aid_len);
		str_album_aid[ctx->fb_friend_cur_album->aid_len] = '\0';
	}
	
	MP_DEBUG("str_album_aid= %s\n", str_album_aid);
	
	/* Send POST Photos.get request*/
	resp = xml_buff_alloc();
	if (!resp)
	{
		errno = -ENOMEM;
		goto fail;
	}
	
	curl_easy_cleanup( ctx->curl );
	ctx->curl = curl_easy_init();

	MP_TRACE_LINE();
	ret = request(ctx, "Photos.get", resp);
	if (ret < 0)
		goto fail;

	//mpDebugPrint("%s:%u resp len=%d", __func__, __LINE__, resp->buff_len);
	//if (resp->buff_len > 0)
        	//NetAsciiDump(html_resp, resp->buff_len);
	SAFE_FREE( str_album_aid );
	
	/*
	* pase all photo's src and title in the selected album.
	*/
	memset(&fb_info, 0, sizeof(fb_info));
	fb_info.state = FB_ALBUM_INIT;
	fb_info.context = ctx;

	mpx_xml_parse(&fb_info
            , resp 	
            , fb_album_start_element_handler 	
            , fb_album_end_element_handler
            , fb_album_content_handler);
	
	xml_buff_free(resp);
	return 0;
	
fail:
	xml_buff_free(resp);
	return -1;
}

static bool userGetLoggedInUser(struct facebook_struct *ctx, char  **puid ) 
{
	XML_BUFF_link_t *resp = NULL;
	int ret;
	fb_info_t	fb_info;

	resp = xml_buff_alloc();
	if (!resp)
	{
		errno = -ENOMEM;
		goto fail;
	}

	if (!ctx->curl)
		ctx->curl = curl_easy_init();

	ret = request(ctx, "users.getLoggedInUser", resp);
	if (ret < 0)
		goto fail;

	/* parse XML */
	memset(&fb_info, 0, sizeof(fb_info));
	fb_info.state = FB_ALBUM_INIT;

	mpx_xml_parse(&fb_info
            , resp 	
            , fb_LoggedInUser_start_element_handler 	
            , fb_LoggedInUser_end_element_handler
            , fb_LoggedInUser_content_handler);

	*puid = fb_info.resp;

	if (!fb_info.resp)
		goto fail;

	xml_buff_free(resp);
	curl_easy_cleanup( ctx->curl );
	ctx->curl = 0;
	return true;
	
fail:
	xml_buff_free(resp);
	if (facebook->curl)
	{
		curl_easy_cleanup( facebook->curl );
		ctx->curl = 0;
	}
	return false;
}

static void fb_album_start_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t *info = (fb_info_t *) user_data;
	if( info->state == FB_ALBUM_INIT )
	{
		if( !strcasecmp(name, "photo") )
		{
			info->state = FB_PHOTO_FOUND;
		}
	}
	else if( info->state == FB_PHOTO_FOUND )
	{
		if( !strcasecmp(name, "pid") )
		{
			info->state = FB_PHOTO_PID_FOUND;
		}
		else if( !strcasecmp(name, "src_big") )
		{
			info->state = FB_PHOTO_SRC_FOUND;
		}
	}

}

static void fb_album_end_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t *info = (fb_info_t *) user_data;
	if( info->state == FB_PHOTO_FOUND )
	{
		if( !strcasecmp(name, "photo") )
		{
			info->state = FB_ALBUM_INIT;
		}
	}
	else if( info->state == FB_PHOTO_PID_FOUND )
	{
		if( !strcasecmp(name, "pid") )
		{
			info->state = FB_PHOTO_FOUND;
		}
	}
	else if( info->state == FB_PHOTO_SRC_FOUND )
	{
		if( !strcasecmp(name, "src_big") )
		{
			info->state = FB_PHOTO_FOUND;
		}
	}
}

static void fb_album_content_handler(void *user_data, const char *s, int len)
{
	fb_info_t *info = (fb_info_t *) user_data;
	struct facebook_struct *ctx = (struct facebook_struct *)info->context;

	if( (info->state == FB_ALBUM_INIT ) && (ctx->fb_cur_album || ctx->fb_friend_cur_album) )
	{
		memset(&jpeg_info, 0, sizeof(jpeg_info));
	}
	else if( (info->state == FB_PHOTO_PID_FOUND) && (ctx->fb_cur_album || ctx->fb_friend_cur_album) )
	{
		//mpDebugPrint("s= %s\n", s);
		//mpDebugPrint("len= %d\n", len);
		char *photo_name;
		char *append = ".jpg";
		photo_name = facebook_malloc(len+5);
		memcpy( photo_name, s, len);
		photo_name[len] = '\0';
		strcat( photo_name, append );
		photo_name[len+4] = '\0';
		MP_DEBUG("photo_name=%s\n", photo_name);
		strncpy( jpeg_info.pathname, photo_name, len+4 );
		MP_DEBUG("jpeg_info.pathname = %s", jpeg_info.pathname);
		SAFE_FREE(photo_name);
	}
	else if( (info->state == FB_PHOTO_SRC_FOUND) && (ctx->fb_cur_album || ctx->fb_friend_cur_album) )
	{
		sprintf( jpeg_info.length, "%d", 0 );
		strncpy( jpeg_info.url, s, len );
		//mpDebugPrint("jpeg_info.url = %s ;len=%d\n", jpeg_info.url, len);
		Net_Xml_parseFileList(&jpeg_info);
	}	
}

static void fb_LoggedInUser_start_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t *info = (fb_info_t *) user_data;

	if ( !strcasecmp(name, "users_getLoggedInUser_response") )
	{
		info->state = FB_USERS_GETLOGGEDINUSER_RESPONSE;
	}
}

static void fb_LoggedInUser_end_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t *info = (fb_info_t *) user_data;

	if (info->state == FB_USERS_GETLOGGEDINUSER_RESPONSE )
	{
		info->state = FB_START;
	}
}

static void fb_LoggedInUser_content_handler(void *user_data, const char *s, int len)
{
	fb_info_t *info = (fb_info_t *) user_data;

	if (info->state == FB_USERS_GETLOGGEDINUSER_RESPONSE)
	{
		char *buf = facebook_malloc(len+1);
		memcpy(buf, s, len);
		buf[len] = '\0';
		info->resp = buf;
	}
}

int curlSetProxy( CURL *curl, char *host, int port, char *user, char *pass )
{
	CURLcode res;

	if (*host)
	{
//                mpDebugPrint("curl_easy_setopt(CURLOPT_PROXY) - %s", host);
        	res = curl_easy_setopt(curl, CURLOPT_PROXY, host);
        	if (res != CURLE_OK)
			MP_DEBUG("curl_easy_setopt(CURLOPT_PROXY) returns error - %d", res);

        	res = curl_easy_setopt(curl, CURLOPT_PROXYPORT, my_proxy_port);
        	if (res != CURLE_OK)
            		MP_DEBUG("curl_easy_setopt(CURLOPT_PROXYPORT) returns error - %d", res);

		res = curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, curl_userpass);
		if (res != CURLE_OK)
			MP_DEBUG("curl_easy_setopt(CURLOPT_PROXYUSERPWD) returns error - %d", res);
    }

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

    /* User-Agent */
    curl_easy_setopt( curl, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; GTB6.5; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; InfoPath.1)");

    /* enable write cookies */
    curl_easy_setopt( curl, CURLOPT_COOKIEFILE, "");

    return res;
}

bool fb_get_friends(struct facebook_struct *ctx)
{
	XML_BUFF_link_t *resp;
	int ret;
	fb_info_t	fb_info;

	/* Send GET Photos.getAlbums request */
	resp = xml_buff_alloc();
	if (!resp)
	{
		errno = -ENOMEM;
		goto fail;
	}
	
	curl_easy_cleanup( ctx->curl );
	ctx->curl = 0;
	ctx->curl = curl_easy_init();

	MP_TRACE_LINE();
	ret = request(ctx, "Friends.get", resp);
	if (ret < 0)
		goto fail;

	/* parse "Friends.get" xml response */
	memset(&fb_info, 0, sizeof(fb_info));
	fb_info.state = FB_FRIENDS_GET_INIT;
	fb_info.context = ctx;

	mpx_xml_parse(&fb_info
            , resp 	
            , fb_friend_start_element_handler 	
            , fb_friend_end_element_handler
            , fb_friend_content_handler);

	//MP_DEBUG("%s have %d friends.\n", ctx->uid, fb_friend_num);

	xml_buff_free(resp);
	resp=NULL;
    	curl_easy_cleanup( ctx->curl );
	ctx->curl=0;
	return true;

fail:
	xml_buff_free(resp);
	resp = NULL;
    	curl_easy_cleanup( ctx->curl );
	ctx->curl = 0;
	fb_friend_num = 0;
	return false;
}

static void fb_friend_start_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t *info = (fb_info_t *) user_data;
	struct facebook_struct *ctx = (struct facebook_struct *)info->context;

	if( info->state == FB_FRIENDS_GET_INIT )
	{
		if ( !strcasecmp(name, "Friends_get_response") )
		{
			info->state = FB_FRIENDS_GET_RESPONSE;
		}
	}
	else if( info->state == FB_FRIENDS_GET_RESPONSE )
	{
		//if( !strcasecmp(name, "Friends_get_response_elt") )	// marked by Joyce in 2011-01-10, Facebook correcting
		if( !strcasecmp(name, "uid") )	
		{
			fb_friend_num++;

			/* append friend list */
			fb_friend_t *fb_friend_entry;
			fb_friend_entry = (fb_friend_t *) facebook_malloc(sizeof(fb_friend_t));
			MP_ASSERT(fb_friend_entry);
			memset(fb_friend_entry, 0, sizeof(fb_friend_t));
			ctx->fb_cur_friend->next = fb_friend_entry;
			ctx->fb_cur_friend= fb_friend_entry;
			
			info->state = FB_FRIEND_FOUND;
		}
	}
}

static void fb_friend_end_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t *info = (fb_info_t *) user_data;

	if ( info->state == FB_FRIENDS_GET_RESPONSE )
	{
		if ( !strcasecmp(name, "Friends_get_response") )
		{
			info->state = FB_FRIENDS_GET_INIT;
		}		
	}
	else if( info->state == FB_FRIEND_FOUND)
	{
		//if( !strcasecmp(name, "Friends_get_response_elt") )	// marked by Joyce in 2011-01-10, Facebook correcting
		if( !strcasecmp(name, "uid") )
		{
			info->state = FB_FRIENDS_GET_RESPONSE;
		}		
	}
}

static void fb_friend_content_handler(void *user_data, const char *s, int len)
{
	fb_info_t *info = (fb_info_t *) user_data;
	struct facebook_struct *ctx = (struct facebook_struct *)info->context;

	if((info->state == FB_FRIEND_FOUND) && ctx->fb_cur_friend)
	{
		ctx->fb_cur_friend->friend_uid = facebook_malloc(len+1);
		memcpy( ctx->fb_cur_friend->friend_uid, s, len );
		ctx->fb_cur_friend->friend_uid[len] = '\0';
		//MP_DEBUG("friend's uid = %s\n", ctx->fb_cur_friend->friend_uid);
	}
}

bool fb_get_friend_album_list(struct facebook_struct *ctx, char *friend_uid)
{
	XML_BUFF_link_t *resp;
	int ret;
	fb_info_t	fb_info;

	SAFE_FREE(str_friend_uid);
	str_friend_uid = facebook_malloc(strlen(friend_uid)+1);
	memcpy( str_friend_uid, friend_uid, strlen(friend_uid) );
	str_friend_uid[strlen(friend_uid)] = '\0';
	MP_DEBUG("[fb_get_friend_album_list]Uid= %s\n", str_friend_uid);
	
	/* Send GET Photos.getAlbums request */
	resp = xml_buff_alloc();
	if (!resp)
	{
		errno = -ENOMEM;
		goto fail;
	}
	
	curl_easy_cleanup( ctx->curl );
	ctx->curl = 0;
	ctx->curl = curl_easy_init();

	MP_TRACE_LINE();
	ret = request(ctx, "Photos.getAlbums", resp);
	if (ret < 0)
		goto fail;

	//mpDebugPrint("%s:%u resp len=%d", __func__, __LINE__, resp->buff_len);
	//if (resp->buff_len > 0)
		//NetAsciiDump(resp->BUFF, resp->buff_len);
	
	/* parse "Photos.getAlbums" xml response */
	memset(&fb_info, 0, sizeof(fb_info));
	fb_info.state = FB_ALBUM_LIST_INIT;
	fb_info.context = ctx;

	mpx_xml_parse(&fb_info
            , resp 	
            , fb_friend_album_list_start_element_handler 	
            , fb_friend_album_list_end_element_handler
            , fb_friend_album_list_content_handler);
    	
	MP_DEBUG("%s have %d albums\n", friend_uid, fb_friend_album_num);

	if( fb_friend_album_num == 0 ) 
	{
		ctx->enumState = enumNoFacebookContentFound;
		MP_DEBUG( "Well, the API have not authorized to access the user's albums and photos. \n" );
	}
	//because Photos.getAlbums return albums include Profile Pictures
	else if( fb_friend_album_num == 1 )
	{
		ctx->enumState = enumNoFacebookContentFound;
		MP_DEBUG( "The user have no albums!\n" );
	}
	
	/* allocate fb_friend_album_num to the right user */
	fb_friend_t *fb_tmp_friend_entry;
	fb_tmp_friend_entry = ctx->fb_friend_list.next;
	while(fb_tmp_friend_entry)
	{
		if(!strcasecmp(friend_uid, fb_tmp_friend_entry->friend_uid))
			fb_tmp_friend_entry->friend_album_num = fb_friend_album_num;		
		fb_tmp_friend_entry = fb_tmp_friend_entry->next;
	}	
	
	xml_buff_free(resp);
	resp=NULL;
    	curl_easy_cleanup( ctx->curl );
	ctx->curl=0;
	ctx->enumState = enumStateNormal;

	SAFE_FREE(str_friend_uid);
	fb_friend_album_num = 0;
	return true;

fail:
	xml_buff_free(resp);
	resp=NULL;
    	fb_friend_album_num=0;
    	curl_easy_cleanup( ctx->curl );
	ctx->curl=0;
	return false;
}

static void fb_friend_album_list_start_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t	*info = (fb_info_t *) user_data;
	struct facebook_struct *ctx = (struct facebook_struct *)info->context;	
//	mpDebugPrint("start name= %s\n", name);
	if( info->state == FB_ALBUM_LIST_INIT )
	{
		if( !strcasecmp(name, "album") )
		{
			fb_friend_album_num++;			
			MP_DEBUG("Album[ %d ] have found!\n", fb_friend_album_num-1 );
			
			info->state = FB_ALBUM_LIST_FOUND;

			//because Photos.getAlbums return albums include Profile Pictures
			if( fb_friend_album_num > 1)
			{
				/* append album list */
				fb_album_t *fb_album_entry;
				fb_album_entry = (fb_album_t *) facebook_malloc(sizeof(fb_album_t));
				MP_ASSERT(fb_album_entry);
				memset(fb_album_entry, 0, sizeof(fb_album_t));
				ctx->fb_friend_cur_album->next = fb_album_entry;
				ctx->fb_friend_cur_album= fb_album_entry;
			}
		}			
	}
	else if( info->state == FB_ALBUM_LIST_FOUND )
	{
		if( !strcasecmp(name, "aid"))
		{
			info->state = FB_ALBUM_AID_FOUND;
		}
		else if( !strcasecmp(name, "cover_pid") )
		{
			info->state = FB_ALBUM_COVER_PID_FOUND;
		}
		else if( !strcasecmp(name, "owner") )
		{
			info->state = FB_ALBUM_OWNER_FOUND;
		}
		else if( !strcasecmp(name, "name") )
		{
			info->state = FB_ALBUM_NAME_FOUND;
		}
		else if( !strcasecmp(name, "link") )
		{
			info->state = FB_ALBUM_LINK_FOUND;
		}
		else if( !strcasecmp(name, "size") )
		{
			info->state = FB_PHOTO_NUM_FOUND; 
		}
	}
}

static void fb_friend_album_list_end_element_handler(void *user_data, const char *name, const char **attr)
{
	fb_info_t	*info = (fb_info_t *) user_data;
//	mpDebugPrint("end name= %s\n", name);
	if( info->state == FB_ALBUM_LIST_FOUND )
	{
		if( !strcasecmp( name, "album" ) )
			info->state = FB_ALBUM_LIST_INIT;
	}
	else if( info->state == FB_ALBUM_AID_FOUND )
	{
		if( !strcasecmp( name, "aid" ) )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
	else if( info->state == FB_ALBUM_COVER_PID_FOUND )
	{
		if( !strcasecmp( name, "cover_pid" ) )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
	else if( info->state == FB_ALBUM_OWNER_FOUND )
	{
		if( !strcasecmp(name, "owner") )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
	else if( info->state == FB_ALBUM_NAME_FOUND )
	{
		if( !strcasecmp( name, "name" ) )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
	else if( info->state == FB_ALBUM_LINK_FOUND )
	{
		if( !strcasecmp( name, "link" ) )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
	else if( info->state == FB_PHOTO_NUM_FOUND )
	{
		if( !strcasecmp( name, "size" ) )
		{
			info->state = FB_ALBUM_LIST_FOUND;
		}
	}
}

static void fb_friend_album_list_content_handler(void *user_data, const char *s, int len)
{
	fb_info_t	*info = (fb_info_t *) user_data;
	struct facebook_struct *ctx = (struct facebook_struct *)info->context;
	
//	mpDebugPrint("s= %s\n", s);
//	mpDebugPrint("len= %d\n", len);
	if( (info->state == FB_ALBUM_AID_FOUND) && ctx->fb_friend_cur_album )
	{
		//because Photos.getAlbums return albums include Profile Pictures
		if( fb_friend_album_num > 1 )
		{
			ctx->fb_friend_cur_album->aid = facebook_malloc(len+1);
			memcpy( ctx->fb_friend_cur_album->aid, s, len );
			ctx->fb_friend_cur_album->aid[len] = '\0';
			MP_DEBUG("aid= %s\n", ctx->fb_friend_cur_album->aid);
			ctx->fb_friend_cur_album->aid_len = len;
		}
	}
	else if( ( info->state == FB_ALBUM_COVER_PID_FOUND ) && ctx->fb_friend_cur_album )
	{
		if( fb_friend_album_num > 1 )
		{
			ctx->fb_friend_cur_album->cover_pid = facebook_malloc(len+1);
			memcpy( ctx->fb_friend_cur_album->cover_pid, s, len );
			ctx->fb_friend_cur_album->cover_pid[len] = '\0';
			MP_DEBUG("cover_pid= %s\n", ctx->fb_friend_cur_album->cover_pid);
		}
	}
	else if( (info->state == FB_ALBUM_OWNER_FOUND ) && ctx->fb_friend_cur_album )
	{
		if( fb_friend_album_num > 1 )
		{
			ctx->fb_friend_cur_album->owner = facebook_malloc(len+1);
			memcpy( ctx->fb_friend_cur_album->owner, s, len );
			ctx->fb_friend_cur_album->owner[len] = '\0';
			MP_DEBUG("owner= %s\n", ctx->fb_friend_cur_album->owner);
		}
	}
	else if( ( info->state == FB_ALBUM_NAME_FOUND ) && ctx->fb_friend_cur_album )
	{
		if( fb_friend_album_num > 1 )
		{
			char *tmp_album_name;
			char *append = "_";
			tmp_album_name = facebook_malloc(len+1);
			memcpy(tmp_album_name, s, len);
			tmp_album_name[len] = '\0';
			ctx->fb_friend_cur_album->name = facebook_malloc(len+strlen(ctx->fb_friend_cur_album->owner)+2);
			sprintf(ctx->fb_friend_cur_album->name, "%s%s%s", ctx->fb_friend_cur_album->owner, append, tmp_album_name);
			ctx->fb_friend_cur_album->name[len+strlen(ctx->fb_friend_cur_album->owner)+1] = '\0';
			MP_DEBUG("name= %s\n", ctx->fb_friend_cur_album->name);
			SAFE_FREE(tmp_album_name);
		}
	}
	else if( ( info->state == FB_ALBUM_LINK_FOUND ) && ctx->fb_friend_cur_album )
	{
		if( fb_friend_album_num > 1 )
		{
			if( strstr(s, "http://www.facebook.com/album.php?aid=") && (len  > 40))
			{
				char *temp_link = facebook_malloc( len + 1);
				ctx->fb_friend_cur_album->link = facebook_malloc( len + strlen(ctx->uid) + 5 );
				memcpy( temp_link, s, len );			
				temp_link[len] = '\0';
				snprintf(ctx->fb_friend_cur_album->link, (len + strlen(ctx->fb_friend_cur_album->owner) + 5), "%s&id=%s", temp_link, ctx->fb_friend_cur_album->owner );
				SAFE_FREE(temp_link);
			}
		}
	}
	else if( ( info->state == FB_PHOTO_NUM_FOUND ) && ctx->fb_friend_cur_album )
	{
		if( fb_friend_album_num > 1 )
		{
			char *str = facebook_malloc(len+1);
			memcpy( str, s, len );
			ctx->fb_friend_cur_album->photo_num = atoi(str);
			MP_DEBUG("photo_num= %d\n", ctx->fb_friend_cur_album->photo_num);
			facebook_free(str);
		}
	}
}

#endif

