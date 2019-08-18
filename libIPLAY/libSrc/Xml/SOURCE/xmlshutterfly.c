/**
 * @file
 *
 * This is an implementation of Shutterfly API.
 * 
 * thumbnails
 *
 * The following APIs are supported:
 *
 *  1) User Authentication - to get an authentication token.
 *
 *          shutterfly_UserAuthentication()
 *
 *     This function must be called first to get a authentication token, which
 *     will be needed in other APIs below.
 *
 *  2) Request a feed that lists all of the albums (private and public ) belonging 
 *     to an user.
 *
 *      (TODO)
 *
 *  3) Request a feed that lists all of the photos in an album with id <albumID>,
 *     belonging to a user <userID>.
 *
 *      (TODO)
 *
 * Copyright (c) 2008 Magic Pixel Inc.
 * All rights reserved.
 */

// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE  0 

#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "netfs.h"
#include "netfs_pri.h"
#include "xmlshutterfly.h"
#ifdef HAVE_LWIP
#include "..\..\lwip\include\net_sys.h"
#else
#include "..\..\uip\include\net_sys.h"
#endif
#include "..\..\CURL\include\net_curl_curl.h"

#ifndef MIN
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#endif

#define SHUTTERFLY_HOST "ws.shutterfly.com"

const char *application_id = "32d06df15c89e5f819f287d577e14a2d";
const char *shared_secret = "ab0293cf0d8baf4e";
char *hash_method = "MD5";
char * GetCurDateTimeW3C(char *clock);

static struct netfs_file_entry jpeg_info;

static char     shutterfly_api_url[MAX_URL];
static shutterfly_info_t   shutterfly_info;

extern Net_App_State App_State;

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */
void *mm_realloc(void *ptr, size_t size);
void mm_free(void *ptr);
void *mm_malloc(size_t size);

#define shutterfly_malloc(sz)   mm_malloc(sz)
#define shutterfly_mfree(ptr)   mm_free(ptr)

int shutterfly_CallSignatureGenerate( const char *url, const char *app_id, const char *secret, const char *timestamp, char *callsig);
int shutterfly_UserAuthentication( shutterfly_info_t *shutterfly_info, const char *email, const char *passwd, char *auth, int auth_len);

//note: fixed to use global my_write_func() for all xmlxxxxxx.c files!
size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf);

static void album_content_handler(void *user_data, const char *s, int len);
static void album_start_element_handler(void *user_data, const char *name, const char **attr);
static void album_end_element_handler(void *user_data, const char *name);

int Shutterfly_PhotoList_Get(BYTE *PhotoSet)
{
	shutterfly_info_t     *pshutterfly_info;
	int     ret;
    	int     count;

	EnableNetWareTask();
	pshutterfly_info = &shutterfly_info;  	
       pshutterfly_info->cur_album = pshutterfly_info->album_list.next;
       
	while(pshutterfly_info->cur_album)
	{
		if(!strcasecmp(pshutterfly_info->cur_album->title, PhotoSet))
		{
			ret = shutterfly_fetch_album(pshutterfly_info, pshutterfly_info->cur_album->id);
            		if (ret < 0)
            		{
                		mpDebugPrint("error code: %d\n", pshutterfly_info->error_code);
                		return ret;
            		}
			break;
		}	
		pshutterfly_info->cur_album = pshutterfly_info->cur_album->next;
	}
	return 0;
}



int shutterfly_init(const char *username, const char *passwd, const char *base_dir)
{
    album_entry_t     *tmp_entry;
    int     ret;
    int     count;

    MP_DEBUG1("request shutterfly photo for '%s'\n", username);

    memset(&shutterfly_info, 0, sizeof(shutterfly_info_t));
    strncpy(shutterfly_info.username, username, MAX_USERNAME);
    shutterfly_info.cur_album = &shutterfly_info.album_list;

    ret = shutterfly_UserAuthentication(&shutterfly_info, username, passwd, shutterfly_info.auth, sizeof shutterfly_info.auth);
    if (ret < 0)
    {
        MP_ALERT("shutterfly_init(): shutterfly_UserAuthentication() error code: %d\n", shutterfly_info.error_code);
        return ret;
    }

    MP_DEBUG("shutterfly_fetch_album_list");
    ret = shutterfly_fetch_album_list(&shutterfly_info);
    if (ret < 0)
    {
        MP_ALERT("shutterfly_init(): shutterfly_fetch_album_list() error code: %d\n", shutterfly_info.error_code);
        return ret;
    }

    count = 0;
    shutterfly_info.cur_album = shutterfly_info.album_list.next;
    tmp_entry = shutterfly_info.cur_album;

    if (tmp_entry)
    {
        while (tmp_entry)
        {
            MP_DEBUG2("Fetch photos from '%s' with id '%s'\n",tmp_entry->title,tmp_entry->id);
            Net_Xml_PhotoSetList(tmp_entry->title,count);	
            count ++;
            tmp_entry = tmp_entry->next;
        }
        Net_PhotoSet_SetCount(count);           /* number of albums */
    }
    else
    {
        mpDebugPrint("No album.\n");
        Net_PhotoSet_SetCount(0);           /* number of albums */
    }

    return 0;
}

void shutterfly_exit(const char *base_dir)
{
   album_entry_t     *tmp_entry;
 /* free resources allocated for all album */
 
    shutterfly_info.cur_album = shutterfly_info.album_list.next;
    while (shutterfly_info.cur_album)
    {
        tmp_entry = shutterfly_info.cur_album;
        shutterfly_info.cur_album = shutterfly_info.cur_album->next;

        shutterfly_mfree(tmp_entry);
    }
}

/**
 *  Get photo set of specified user id
 */
static void authtoken_content_handler(void *user_data, const char *s, int len)
{
    shutterfly_info_t   *shutterfly_info    = (shutterfly_info_t *) user_data;
    char    *token;

    if (shutterfly_info->state == SHUTTERFLY_AUTH_NEWAUTHTOKEN)
    {
        token = shutterfly_info->auth;
        len = MIN(len , MAX_AUTHTOKEN_LEN);
        memcpy(token, s,len);
        token[len] = '\0';
        MP_DEBUG("NETFS_SHUTTERFLY: token %s", token);
    }
}

static void authtoken_start_element_handler(void *user_data, const char *name, const char **attr)
{
    shutterfly_info_t   *shutterfly_info    = (shutterfly_info_t *) user_data;

    if (shutterfly_info->state == SHUTTERFLY_AUTH_INIT)
    {
        if (!strcasecmp(name, "entry"))
        {
            shutterfly_info->state = SHUTTERFLY_AUTH_ENTRY;
        }
    }
    else if (shutterfly_info->state == SHUTTERFLY_AUTH_ENTRY)
    {
        if (!strcasecmp(name, "user:newAuthToken"))
            shutterfly_info->state = SHUTTERFLY_AUTH_NEWAUTHTOKEN;
            
    }
}

static void authtoken_end_element_handler(void *user_data, const char *name)
{
    shutterfly_info_t  *shutterfly_info     = (shutterfly_info_t *) user_data;

    if (shutterfly_info->state == SHUTTERFLY_AUTH_NEWAUTHTOKEN)
    {
        if (!strcasecmp(name, "user:newAuthToken"))
            shutterfly_info->state = SHUTTERFLY_AUTH_ENTRY;
    }
    else if (shutterfly_info->state == SHUTTERFLY_AUTH_ENTRY)
    {
        if (!strcasecmp(name, "entry"))
            shutterfly_info->state = SHUTTERFLY_AUTH_INIT;
    }
}

static void album_list_content_handler(void *user_data, const char *s, int len)
{

}
static void album_list_start_element_handler(void *user_data, const char *name, const char **attr)
{

}
static void album_list_end_element_handler(void *user_data, const char *name)
{

}
/* 
 * shutterfly_fetch_album_list - Get a listing of all albums associated with a user.
 */
int shutterfly_fetch_album_list(shutterfly_info_t *shutterfly_info)
{
    void *curl;
    CURLcode res;
    int                     ret = 0;
    int                     len;
    char *data; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    struct curl_slist *header=NULL;

    Xml_BUFF_init(NET_RECVHTTP);	
	MP_DEBUG("NETFS_SHUTTERFLY: shutterfly_fetch_album_list");

    snprintf(shutterfly_api_url, MAX_URL, 
             "http://%s/user/%s/album",
             SHUTTERFLY_HOST, shutterfly_info->username);

    curl = curl_easy_init();
    if(curl) {
        data = shutterfly_malloc(512);
        if (data == NULL)
            goto exit;
        snprintf(data,512, "Authorization: GoogleLogin auth=%s", shutterfly_info->auth);
        data[511] = '\0';
        header = curl_slist_append(header, data);
        MP_DEBUG("NETFS_SHUTTERFLY: extra header=%d", strlen(data));
        curl_easy_setopt(curl, CURLOPT_URL, shutterfly_api_url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

        res = curl_easy_perform(curl);
        //NetAsciiDump(girbuf, bufindex);
        TaskYield();

        /* check return code of curl_easy_perform() */
        if (res != CURLE_OK)
        {
            MP_DEBUG("shutterfly_fetch_album_list: curl_easy_perform failed, res = %d", res);
            httpcode = 0;
            goto Shutterfly_cleanup_1;
        }

        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
            httpcode = 0;

Shutterfly_cleanup_1:
        /* always cleanup */
        curl_easy_cleanup(curl);

        shutterfly_mfree(data);

        if (httpcode == 200)
        {
            /* Parse XML */            
            shutterfly_info->state = SHUTTERFLY_ALBUM_LIST_INIT;

            MPX_XML_Parse(shutterfly_info
		              , album_list_start_element_handler
		              , album_list_end_element_handler
		              , album_list_content_handler);  		 


            if (shutterfly_info->error_code > 0)
            {
                ret = -NETFS_APP_ERROR;
                goto exit;
            }
        }
        else if (httpcode > 0)                  /* TODO */
        {
            MP_DEBUG("NETFS_SHUTTERFLY: http error resp code=%d", httpcode);
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 login failed */
            {
                MP_ALERT("NETFS_SHUTTERFLY: Server Error %d login failed", httpcode);
            }
            else
            {
                MP_ALERT("NETFS_SHUTTERFLY: Server Error %d login failed", httpcode);
            }
            ret = -1;
        }
    }

exit:

    Xml_BUFF_free(NET_RECVHTTP);
   	return ret;
}

/* 
 * shutterfly_fetch_album - Get a album associated with a user.
 */
int shutterfly_fetch_album(shutterfly_info_t *shutterfly_info, char *album_id)
{
    void *curl;
    CURLcode res;
    int                     ret;
    int                     len;
    char *data; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    struct curl_slist *header=NULL;
    
    Xml_BUFF_init(NET_RECVHTTP);	
	MP_DEBUG("NETFS_SHUTTERFLY: shutterfly_fetch_album");

    snprintf(shutterfly_api_url, MAX_URL, 
             "http://%s/user/%s/albumid/%s",
             SHUTTERFLY_HOST,
             shutterfly_info->username,
             album_id);

    curl = curl_easy_init();
    if(curl) {
        data = shutterfly_malloc(512);
        if (data == NULL)
            goto exit;
        snprintf(data,512, "Authorization: GoogleLogin auth=%s", shutterfly_info->auth);
        data[511] = '\0';
        header = curl_slist_append(header, data);
        MP_DEBUG("NETFS_SHUTTERFLY: extra header=%d", strlen(data));
        curl_easy_setopt(curl, CURLOPT_URL, shutterfly_api_url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

        res = curl_easy_perform(curl);
        TaskYield();

        /* check return code of curl_easy_perform() */
        if (res != CURLE_OK)
        {
            MP_DEBUG("shutterfly_fetch_album: curl_easy_perform failed, res = %d", res);
            httpcode = 0;
            goto Shutterfly_cleanup_2;
        }

        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
            httpcode = 0;

Shutterfly_cleanup_2:
        /* always cleanup */
        curl_easy_cleanup(curl);

        shutterfly_mfree(data);

        MP_DEBUG("NETFS_SHUTTERFLY: http error resp code=%d", httpcode);
        if (httpcode == 200)
        {
            /* Parse XML */            
            shutterfly_info->state = SHUTTERFLY_ALBUM_INIT;

	     MPX_XML_Parse(shutterfly_info
		              , album_start_element_handler
		              , album_end_element_handler
		              , album_content_handler); 		

            if (shutterfly_info->error_code > 0)
            {
                ret = -NETFS_APP_ERROR;
                goto exit;
            }
        }
        else if (httpcode > 0)                  /* TODO */
        {
            MP_DEBUG("NETFS_SHUTTERFLY: http error resp code=%d", httpcode);
            ptr = App_State.XML_BUF;
            if (httpcode == 403)                    /* 403 login failed */
            {
                MP_ALERT("NETFS_SHUTTERFLY: Server Error %d login failed", httpcode);
            }
            else
            {
                MP_ALERT("NETFS_SHUTTERFLY: Server Error %d login failed", httpcode);
            }
            ret = -1;
        }
    }
    
exit:
       
    Xml_BUFF_free(NET_RECVHTTP);
    return ret;
}


/**
 *  Get photo list of specified photo set
 */
static void album_content_handler(void *user_data, const char *s, int len)
{
    shutterfly_info_t   *shutterfly_info    = (shutterfly_info_t *) user_data;
    char    *title;

    if (shutterfly_info->state == SHUTTERFLY_ALBUM_PHOTO_TITLE)
    {
        title = jpeg_info.pathname;
        len = MIN(len, NETFS_MAX_PATHNAME-1);
        memcpy(title, s,len);
        title[len] = '\0';
    }
}

static void album_start_element_handler(void *user_data, const char *name, const char **attr)
{
    shutterfly_info_t   *shutterfly_info    = (shutterfly_info_t *) user_data;
    const char      *url_value      = NULL;
    const char      *file_size_value= NULL;
    int             image_height;
    short           num = 0;
    short           i;

    if (shutterfly_info->state == SHUTTERFLY_ALBUM_INIT)
    {
        if (!strcasecmp(name, "entry"))
        {
            memset(&jpeg_info, 0, sizeof(jpeg_info));
            shutterfly_info->state = SHUTTERFLY_ALBUM_PHOTO;
        }
    }
    else if (shutterfly_info->state == SHUTTERFLY_ALBUM_PHOTO)
    {
        if (!strcasecmp(name, "title"))
        {
            shutterfly_info->state = SHUTTERFLY_ALBUM_PHOTO_TITLE;
        }
        else if (!strcasecmp(name, "media:group"))
        {
            shutterfly_info->state = SHUTTERFLY_ALBUM_PHOTO_MEDIAGROUP;
            shutterfly_info->num_urls = 0;
            for (i=0; i< 2; i++)
            {
                shutterfly_info->url[i][0] = '\0';
                shutterfly_info->height[i] = 0;
            }
        }

    }
    else if (shutterfly_info->state == SHUTTERFLY_ALBUM_PHOTO_MEDIAGROUP)
    {
        if (!strcasecmp(name, "media:content"))
        {
            image_height = 0;
            while (*attr)
            {
                if (!strcasecmp(*attr, "url"))
                {
                    attr ++;
                    url_value = *attr;
                }
                else if (!strcasecmp(*attr, "height"))
                {
                    attr ++;
                    image_height = atoi(*attr);
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }
            
            num = shutterfly_info->num_urls;
            if (url_value && num < 2)
            {
                snprintf(shutterfly_info->url[num], NETFS_MAX_URL, url_value);
                shutterfly_info->height[num] = image_height;
                shutterfly_info->num_urls++;
            }

        }
    }
}

static void album_end_element_handler(void *user_data, const char *name)
{
    shutterfly_info_t   *shutterfly_info    = (shutterfly_info_t *) user_data;
    int len0= 0 , len1 =0 , len2 =0;	
    BYTE *str1,*str2,*str3;
    BYTE filesz[20];
   	

    if (shutterfly_info->state == SHUTTERFLY_ALBUM_PHOTO_TITLE)
    {
        if (!strcasecmp(name, "title"))
        {
            shutterfly_info->state = SHUTTERFLY_ALBUM_PHOTO;
        }
    }
    else if (shutterfly_info->state == SHUTTERFLY_ALBUM_PHOTO)
    {
        if (!strcasecmp(name, "entry"))
        {
            if (shutterfly_info->url_l)
            {
                strcpy(jpeg_info.url, shutterfly_info->url_l);
                MP_DEBUG2("add photo '%s' => '%s'\n",jpeg_info.pathname, jpeg_info.url);
                sprintf(jpeg_info.length,"%d",0);	
                Net_Xml_parseFileList(&jpeg_info);
            }

            shutterfly_info->state = SHUTTERFLY_ALBUM_INIT;
        }
    }
    else if (shutterfly_info->state == SHUTTERFLY_ALBUM_PHOTO_MEDIAGROUP)
    {
        short big, ht, i;
        if (!strcasecmp(name, "media:group"))
        {
              MP_DEBUG2("add photo '%s' => '%s'\n",jpeg_info.pathname, jpeg_info.url);
              sprintf(jpeg_info.length,"%d",0);	

              ht = 0;
              big = 0;
              for (i=0; i<shutterfly_info->num_urls; i++)
              {
                  if (ht < shutterfly_info->height[i])
                  {
                      ht = shutterfly_info->height[i];
                      big = i;
                  }
              }

#if 0
              len = strlen(shutterfly_info->url[big]);
              len = MIN(len, NETFS_MAX_URL-1);
              strncpy(shutterfly_info->url_l, shutterfly_info->height[big], len+1);
              shutterfly_info->url_l[len] = '\0';
#endif
              shutterfly_info->url_l =  shutterfly_info->url[big];
              if (shutterfly_info->num_urls > 1)
              {
                  /* small size */
                  i = 1 - big;
                  shutterfly_info->url_s =  shutterfly_info->url[i];
              }
              else
                  shutterfly_info->url_s =  NULL;
              shutterfly_info->state = SHUTTERFLY_ALBUM_PHOTO;
        }
    }
}

int shutterfly_UserAuthentication(
        shutterfly_info_t *shutterfly_info,
        const char *email,
        const char *passwd,
        char *auth,
        int auth_len)
{
    void *curl;
    CURLcode res;
    int                     ret = 0;
    int                     len;
    char *data; 
    XML_BUFF_link_t *ptr;
    int httpcode;
    struct curl_slist *headers=NULL;
    char clock[32];
    char callsig[33];


    Xml_BUFF_init(NET_RECVHTTP);	
	MP_DEBUG("NETFS_SHUTTERFLY: UserAuthentication");

    data = (char *)shutterfly_malloc(512);
    if (data == NULL)
        goto exit;

    snprintf(shutterfly_api_url, MAX_URL, 
            "https://%s/user/%s/auth", SHUTTERFLY_HOST, email);

    GetCurDateTimeW3C(clock);
    if (strlen(clock) == 0)
        goto exit;

    ret = shutterfly_CallSignatureGenerate(shutterfly_api_url, application_id, shared_secret,
            clock, callsig);
    if (ret < 0)
        goto exit;

    MP_DEBUG("NETFS_SHUTTERFLY: callsig=%s", callsig);

    strcat(shutterfly_api_url, "?oflyAppId=");
    strcat(shutterfly_api_url, application_id);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, shutterfly_api_url);

        len = snprintf(data, 512,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<entry xmlns=\"http://www.w3.org/2005/Atom\""
            "xmlns:user=\"http://user.openfly.shutterfly.com/v1.0\">"
            "<category term=\"user\" scheme=\"http://openfly.shutterfly.com/v1.0\" />"
            "<user:password>%s</user:password>"
            "</entry>",
            passwd);

        MP_ASSERT(len < 512);
        NetAsciiDump(data, len);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        len = snprintf(data,512, 
                "oflyTimestamp: %s", clock);
        MP_DEBUG("NETFS_SHUTTERFLY: extra headers=%s", data);
        headers = curl_slist_append(headers, data);

        len = snprintf(data,512, 
                "oflyApiSig: %s", callsig);
        MP_DEBUG("NETFS_SHUTTERFLY: extra headers=%s", data);
        headers = curl_slist_append(headers, data);

        len = snprintf(data,512, 
                "oflyHashMeth: %s", hash_method);
        MP_DEBUG("NETFS_SHUTTERFLY: extra headers=%s", data);
        headers = curl_slist_append(headers, data);

        len = strcpy(data, "Content-Type: text/xml");
        headers = curl_slist_append(headers, data);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, App_State.XML_BUF);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func); /* for body only */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

        //set follow location
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

        res = curl_easy_perform(curl);
        //NetAsciiDump(girbuf, bufindex);
        TaskYield();

        /* check return code of curl_easy_perform() */
        if (res != CURLE_OK)
        {
            MP_DEBUG("shutterfly_UserAuthentication: curl_easy_perform failed, res = %d", res);
            httpcode = 0;
            goto Shutterfly_cleanup_3;
        }

        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode) != CURLE_OK)
            httpcode = 0;

Shutterfly_cleanup_3:
        /* always cleanup */
        curl_easy_cleanup(curl);

        shutterfly_mfree(data);

        MP_DEBUG("NETFS_SHUTTERFLY: http resp code=%d", httpcode);
        if (httpcode == 201)                    /* 201 created */
        {
            /* Parse XML */            
            shutterfly_info->state = SHUTTERFLY_AUTH_INIT;

	     MPX_XML_Parse(shutterfly_info
		              , authtoken_start_element_handler
		              , authtoken_end_element_handler
		              , authtoken_content_handler);		
		 
        }
        else if (httpcode > 0)                  /* TODO */
        {
            ptr = App_State.XML_BUF;
            //NetAsciiDump(ptr->BUFF, ptr->buff_len);
            if (httpcode == 403)                    /* 403 login failed */
            {
                MP_ALERT("NETFS_SHUTTERFLY: Server Error %d login failed", httpcode);
            }
            else
            {
                MP_ALERT("NETFS_SHUTTERFLY: Server Error %d login failed", httpcode);
            }
            ret = -1;
        }
    }

exit:
    if (data)
        shutterfly_mfree(data);

    Xml_BUFF_free(NET_RECVHTTP);    
    return ret;
}

/* 
 * shutterfly_CallSignatureGenerate - Generate Shutterfly Call Signature.
 *
 * <callsig> has to be at least 33 in length (including terminating char '\0').
 *
 * NOTE: URL parameters in the <url>, if any, must be in alphabetical order.
 * (ref. http://www.shutterfly.com/documentation/OflyCallSignature.sfly)
 */
int shutterfly_CallSignatureGenerate(
        const char *url, 
        const char *app_id,
        const char *secret,
        const char *timestamp,
        char *callsig)
{
  char *md5this;
  unsigned char md5buf[16]; /* 16 bytes/128 bits */
  int len;
  char *uri;
  char *protsep;
  char *parmsep, extra;

  md5this = shutterfly_malloc(MAX_URL);
  if (md5this == NULL)
      return;

  protsep=strstr(url, "://");
  if(!protsep)
      return;
  protsep += 3;

  /* find the slash after the host name */
  uri = strchr(protsep, '/');
  if(!uri)
      return;

  /* any URL parameters ? */
  parmsep = strchr(uri, '?');
  if(!parmsep)
      extra = '?';
  else
      extra = '&';

  MP_DEBUG("uri =%s", uri);
  len = snprintf(md5this, MAX_URL, 
          "%s%s%coflyAppId=%s&oflyHashMeth=%s&oflyTimestamp=%s",
          secret, uri, extra,
          app_id, hash_method, timestamp);
  MP_ASSERT(len < MAX_URL);

  /* ----------  MD5 digest ---------- */

#if HAVE_CURL
  Curl_md5it(md5buf, md5this);
  shutterfly_mfree(md5this);
  md5_to_ascii(md5buf, callsig);
#else
  MP_ASSERT(0);
#endif
}


#if 0 //note: fixed to use global my_write_func() for all xmlxxxxxx.c files!
static size_t my_write_func(void *ptr, size_t size, size_t nmemb, void *buf)
{
    XML_BUFF_link_t *xp = buf;
    size_t len = size * nmemb;
    {
        if (len > 0)
        {
            /* check data length to avoid buffer overflow */
            if (len > IMAGE_BUF - xp->buff_len)
            {
                MP_ALERT("my_write_func: data length exceeds left buffer space");
                MP_DEBUG("=> discard such big data!");
                return 0;
            }

            memcpy(&xp->BUFF[xp->buff_len], ptr, len);
            xp->buff_len += len;
        }
    }

    return len;
}
#endif


