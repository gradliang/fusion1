#ifndef __xml_youku3G_h__
#define __xml_youku3G_h__

#include "netware.h"


typedef struct _video_entry     video_entry_t;
typedef struct _youku3g_info     youku3g_info_t;
typedef struct _youku3g_video     youku3g_video_t;


#define MAX_TITLE       64
#define MAX_USERNAME    64
#define MAX_ALBUMID     64
#define MAX_URL         256
#define MAX_TEXT        1024   
#define MAX_TIME        16
#define MAX_RATING      8
#define MAX_FRM_URL_MAP 5
#define MAX_FRM_URL     512

struct _video_entry
{
    video_entry_t         *next;
    char    id[MAX_ALBUMID];
    char    title[MAX_TITLE];
    char    url[MAX_URL];
    char    thumbnail[MAX_URL];
	int     thumbnailsize;
	char    counthint[MAX_TIME];
	//for video rating
	char	average[MAX_RATING];
	char	rating_max[MAX_RATING];
	char	rating_min[MAX_RATING];
	char	numRaters[MAX_RATING];
	//
	char    duration[MAX_TIME];
	char    viewCount[MAX_TIME];
	char    frm_url_map[MAX_FRM_URL_MAP][MAX_FRM_URL];

	//char    description[MAX_TEXT];
	//char    keywords[MAX_TEXT];
	//char    uploaded[MAX_TITLE];
	//char    credit[MAX_USERNAME];
    //video_entry_t         *prev;
};

enum _YOUKU3G_STATE
{
    YOUKU3G_NULL,
    YOUKU3G_ERROR,
    YOUKU3G_VIDEO_LIST_INIT,
    YOUKU3G_VIDEO_LIST_FOUND,
    YOUKU3G_VIDEO_LIST_TITLE,
    YOUKU3G_VIDEO_LIST_ID,
    YOUKU3G_VIDEO_LIST_DURATION,
    YOUKU3G_VIDEO_LIST_DURATION_CP,
    YOUKU3G_VIDEO_LOCATION_INIT,

};


struct _youku3g_info
{
    enum _YOUKU3G_STATE  state;
    char                username[MAX_USERNAME];
    int                 error_code;
    video_entry_t       video_list;
    video_entry_t       *cur_video;
    char                auth[256+1];
	//char                referer[64];
	//char                token[64];
    int                 video_num;
	char                youku3g_location[1024];

};

struct _youku3g_video
{
	int youku3g_video_size;
	int youku3g_video_start;
	int youku3g_video_data_end;
	int youku3g_video_total;
	int youku3g_video_idx;
	int youku3g_video_ext;
	int youku3g_video_waiting;
	int youku3g_video_stop;
	int youku3g_video_error;
	int youku3g_video_cate;
	int youku3g_video_page;
};

BYTE *youku3g_getvideo_id(DWORD dwIndex);
int youku3g_get_video_location(void);
int youku3g_init(char *username,char *pasword,char youtube_categories,char youtube_pop_most,char youtube_page_idx); //*base_dir);



#endif //__xml_youtube_h__

