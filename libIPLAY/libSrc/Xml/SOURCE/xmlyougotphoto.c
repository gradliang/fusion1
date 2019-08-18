/**
 * @file
 *
 * Routines for access of YouGotPhoto.com
 * 
 * Copyright (c) 2007 Magic Pixel Inc.
 * All rights reserved.
 */

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  1 

#include "corelib.h"

#if NETWARE_ENABLE

#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "netfs.h"
#include "netfs_pri.h"
#include "netware.h"
#include "ndebug.h"

#include "..\..\lwip\include\net_sys.h"

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define yougotphoto_malloc(sz)   mm_malloc(sz)
#define yougotphoto_mfree(ptr)   mm_free(ptr)

#define YGP_HOST        "api.yougotphoto.com"
#define YGP_API_KEY		"f3042bd9def60ce431f11bc3f892ec70"
#define YGP_API_SECRECT	"3da64d790246601fe298e1cdd88a3f4d"
#define YGP_SERIAL_NO	"00066e00182a"
#define MAX_PHOTOS      32
#define MAX_URL         256

enum YGP_ACTION{
	YGP_GET_DEVICE_INFO,
	YGP_GET_DEVICE_PHOTO,
	YGP_GET_DEVICE_NEW_PHOTO,
	YGP_MARK_PHOTO,
	YGP_UNMARK_PHOTO,
	YGP_LOGOUT_DEVICE
};

enum YGP_STATE {
    YGP_START,
    YGP_SLIDESHOW,
};

struct ygp_info_s {
    enum YGP_STATE  state;
    int             error_code;
    char            *base_dir;
    unsigned long   slide_show;                 /* photo change time in slideshow (in seconds) */
};
typedef struct ygp_info_s     ygp_info_t;

static ygp_info_t   ygp_info;

static struct netfs_file_entry jpeg_info;
static char     ygp_api_url[MAX_URL];

struct ygphoto_s {
    char photo_url[MAX_URL];
    uint32_t  photo_uid;
};

struct ygphoto_s photos[MAX_PHOTOS];            /* TODO */
uint32_t photo_index;

extern Net_App_State App_State;

#ifndef MIN
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#endif

static int ygp_get_device_photo(ygp_info_t *ygp_info);
static int ygp_get_device_info(ygp_info_t *ygp_info);

int ygp_init(const char *username, const char *base_dir)
{
    int     ret = NETFS_OK;
    int     count;

    mpDebugPrint("request YouGotPhoto for '%s'", username);

    memset(&ygp_info, 0, sizeof(ygp_info_t));

    ygp_info.base_dir = (char *) yougotphoto_malloc(strlen(base_dir) + 1);
    if (ygp_info.base_dir == NULL)
    {
        ret = -NETFS_NO_MEMORY;
        goto err_exit;
    }
    strcpy(ygp_info.base_dir, base_dir);

    MP_DEBUG("Fetch device photos\n");
	
#if 1	//add for GetNetNextPictureIndex
            ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
            struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
            DWORD dwOpMode = psSysConfig->dwCurrentOpMode;
            psFileBrowser->dwFileListIndex[dwOpMode] = 0;
            psFileBrowser->dwFileListCount[dwOpMode] = 0;
            psFileBrowser->dwFileListAddress[dwOpMode] = 0;
            psFileBrowser->dwFileListAddress[dwOpMode] = (DWORD) &psFileBrowser->sSearchFileList[0];
#endif	

    ret = ygp_get_device_photo(&ygp_info);
    if (ret < 0)
    {
        mpDebugPrint("ygp_init: error code: %d\n", ygp_info.error_code);
        return ret;
    }
	
#if 1	//add for GetNetNextPictureIndex
            psFileBrowser->dwImgAndMovCurIndex = psFileBrowser->dwFileListIndex[dwOpMode];
            psFileBrowser->dwImgAndMovTotalFile = psFileBrowser->dwFileListCount[dwOpMode];
            psFileBrowser->sImgAndMovFileList = (ST_SEARCH_INFO *)psFileBrowser->dwFileListAddress[dwOpMode];
#endif 	

    ret = ygp_get_device_info(&ygp_info);
    if (ret < 0)
    {
        mpDebugPrint("ygp_init: error code: %d\n", ygp_info.error_code);
        return ret;
    }

err_exit:
    return ret;
}

void ygp_exit(const char *base_dir)
{
}

static void ygp_start_element_handler(void *user_data, const char *name, const char **attr)
{
    ygp_info_t   *yi    = (ygp_info_t *) user_data;
    const char      *num_photosp      = NULL;
    const char      *photo_url= NULL;
    const char      *uid_p= NULL;
    const char      *stat= NULL;
    int num_photos;

    if (!strcasecmp(name, "rsp"))
    {
        while (*attr)
        {
            if (!strcasecmp(*attr, "stat"))
            {
                attr ++;
                stat = *attr;
            }
            else
            {
                attr ++;
            }

            attr ++;
        }

        if (stat)
        {
            if (!strcasecmp(stat, "ok"))
                yi->error_code = 0;
            else
            {
                yi->error_code = -1;
                MP_DEBUG("Request YouGotPhoto failed. (%s)\n", stat);
            }
        }

        photo_index = 0;
    }
    else if (!strcasecmp(name, "device_serial_no"))
    {
        while (*attr)
        {
            if (!strcasecmp(*attr, "photos"))
            {
                attr ++;
                num_photosp = *attr;
            }
            else
            {
                attr ++;
            }

            attr ++;
        }

        if (num_photosp)
        {
            num_photos = atoi(num_photosp);
        }
        photo_index = 0;
    }
    else if (!strcasecmp(name, "photo"))
    {
        while (*attr)
        {
            if (!strcasecmp(*attr, "photo_url"))
            {
                attr ++;
                photo_url = *attr;
            }
            else if (!strcasecmp(*attr, "url_no"))
            {
                attr ++;
                uid_p = *attr;
            }
            else
            {
                attr ++;
            }

            attr ++;
        }

        if (photo_url)
        {
            strncpy(photos[photo_index].photo_url, photo_url, MAX_URL);
            photos[photo_index].photo_url[MAX_URL-1] = '\0';
        }
        else
            return;

        if (uid_p)
        {
            photos[photo_index].photo_uid = atoi(uid_p);
        }
        else
            return;

        photo_index++;

        memset(&jpeg_info, 0, sizeof(jpeg_info));

        //snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,"/%s/%s.JPG", yi->base_dir, uid_p);
	 snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME,"%s.JPG", uid_p);
	 snprintf(jpeg_info.url, NETFS_MAX_URL, "%s", photo_url);
        MP_DEBUG("ygp_start_element_handler: url=%s", jpeg_info.url);
        sprintf(jpeg_info.length,"%d",0);		
#if 0 //CJ not need to add netfs
        netfs_add_file(&jpeg_info);            
#endif
        Net_Xml_parseFileList(&jpeg_info);
    }

}

static void ygp_end_element_handler(void *user_data, const char *name)
{
    ygp_info_t   *yi    = (ygp_info_t *) user_data;
    
    yi->state = YGP_START;
}

static void ygp_get_device_info_handler(void *user_data, const char *name, const char **attr)
{
    ygp_info_t   *yi    = (ygp_info_t *) user_data;
    const char      *num_photosp      = NULL;
    const char      *photo_url= NULL;
    const char      *uid_p= NULL;
    const char      *stat= NULL;
    int num_photos;

    if (!strcasecmp(name, "rsp"))
    {
        while (*attr)
        {
            if (!strcasecmp(*attr, "stat"))
            {
                attr ++;
                stat = *attr;
            }
            else
            {
                attr ++;
            }

            attr ++;
        }

        if (stat)
        {
            if (!strcasecmp(stat, "ok"))
                yi->error_code = 0;
            else
            {
                yi->error_code = -1;
                MP_DEBUG("Request YouGotPhoto failed. (%s)\n", stat);
            }
        }

        photo_index = 0;
        return;
    }

    if (yi->error_code < 0)
        return;

    if (!strcasecmp(name, "slide_show"))
    {
        yi->state = YGP_SLIDESHOW;
    }

}

static void ygp_get_device_info_content_handler(void *user_data, const char *s, int len)
{
    ygp_info_t   *yi    = (ygp_info_t *) user_data;
    char val[12];
    if (yi->state == YGP_SLIDESHOW)
    {
        len = MIN(len,sizeof val);
        memcpy(val, s,len);
        if (len < sizeof val)
            val[len] = '\0';
        else
            val[sizeof val - 1] = '\0';
        yi->slide_show = atoi(val);
        //MP_DEBUG("handler: slide=%d", yi->slide_show);
    }
}

/* 
 * ygp_get_device_photo:
 *
 * Retrieve list of photo associated with this device
 *
 * This is one of YouGotPhoto.com's APIs
 */
static int ygp_get_device_photo(ygp_info_t *ygp_info)
{
    int                     ret;

	char md5_temp[MAX_URL];
	char cksn[16];
	char hex_cksn[33];

    
    /* Get Data from Remote Site and Parse it */

    /* ----------  URL is hard-coded for now  ---------- */

	memset(md5_temp, 0, MAX_URL);
	memset(cksn, 0, 16);
	memset(hex_cksn, '\0', 33);

	snprintf(md5_temp, MAX_URL, "%sactionget_device_photoapi_key%sserial_no%s"
		, YGP_API_SECRECT, YGP_API_KEY, YGP_SERIAL_NO);
	#if Make_CURL
	Curl_md5it(cksn,md5_temp);
	#endif
	ascii_to_hex(cksn, hex_cksn,16);

	//b4dd29e3090340cf468fdc5c9bfeb10e

    snprintf(ygp_api_url, MAX_URL, 
             "http://%s/device/?action=get_device_photo&api_key=%s"
             "&serial_no=%s&cksn=%s"
             , YGP_HOST, YGP_API_KEY, YGP_SERIAL_NO, hex_cksn
             );

    MP_DEBUG("ygp_get_device_photo: ");

    Xml_BUFF_init(NET_RECVHTTP);	
	#if Make_CURL
    ret = Net_Recv_Data(ygp_api_url,NETFS_YOUGOTPHOTO,0,0);
	#endif
    if(ret <0) 
        goto err_exit;

    MPX_XML_Parse(ygp_info
		              , ygp_start_element_handler
		              , ygp_end_element_handler
		              , NULL);	

    if (ygp_info->error_code < 0)
        ret = -NETFS_APP_ERROR;
    else
        ret = NETFS_OK;
    MP_DEBUG1("ygp_get_device_photo: returns %d", ret);
       
err_exit:

    Xml_BUFF_free(NET_RECVHTTP);
    return ret;
}

/* 
 * ygp_get_device_info:
 *
 * Getting information related to specific device such as device name, number 
 * of second till next photo etc.
 *
 * This is one of YouGotPhoto.com's APIs
 */
static int ygp_get_device_info(ygp_info_t *ygp_info)
{
    int                     ret;

	char md5_temp[MAX_URL];
	char cksn[16];
	char hex_cksn[33];

    /* Get Data from Remote Site and Parse it */

    ygp_info->state = YGP_START;

    /* ----------  URL is hard-coded for now  ---------- */
	memset(md5_temp, 0, MAX_URL);
	memset(cksn, 0, 16);
	memset(hex_cksn, '\0', 33);

	snprintf(md5_temp, MAX_URL, "%sactionget_device_infoapi_key%sserial_no%s"
		, YGP_API_SECRECT, YGP_API_KEY, YGP_SERIAL_NO);
	#if Make_CURL
	Curl_md5it(cksn,md5_temp);
	#endif
	ascii_to_hex(cksn, hex_cksn,16);

	//98d91a3dceaf9c020f7234e392979fe4

    snprintf(ygp_api_url, MAX_URL, 
             "http://%s/device/?action=get_device_info&api_key=%s"
             "&serial_no=%s&cksn=%s"
             , YGP_HOST, YGP_API_KEY, YGP_SERIAL_NO, hex_cksn
             );

    MP_DEBUG("ygp_get_device_info: ");

    Xml_BUFF_init(NET_RECVHTTP);	
	#if Make_CURL
    ret = Net_Recv_Data(ygp_api_url,NETFS_YOUGOTPHOTO,0,0);
	#endif
    if(ret <0) 
        goto err_exit;

    MPX_XML_Parse(ygp_info
		              , ygp_get_device_info_handler
		              , ygp_end_element_handler
		              , ygp_get_device_info_content_handler);		


    if (ygp_info->error_code < 0)
        ret = -NETFS_APP_ERROR;
    else
        ret = NETFS_OK;
    MP_DEBUG1("ygp_get_device_info: returns %d", ret);

err_exit:

    Xml_BUFF_free(NET_RECVHTTP);    
    return ret;
}

#endif

