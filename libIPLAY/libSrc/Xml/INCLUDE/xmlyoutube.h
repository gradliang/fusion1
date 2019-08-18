#ifndef __xml_youtube_h__
#define __xml_youtube_h__

#include "netware.h"


typedef struct _video_entry     video_entry_t;
typedef struct _youtube_info     youtube_info_t;
typedef struct _youtube_video     youtube_video_t;


#define MAX_TITLE       64
#define MAX_USERNAME    64
#define MAX_ALBUMID     64
#define MAX_URL         256
#define MAX_TEXT        1024   
#define MAX_TIME        16
#define MAX_RATING      8
#define MAX_FRM_URL_MAP 4
#define MAX_FRM_URL     512

struct _video_entry
{
    video_entry_t         *next;
    char    id[MAX_ALBUMID];
    char    title[MAX_TITLE];
    char    url[MAX_URL];
    char    thumbnail[MAX_URL];
	int     thumbnailsize;
	//for video rating
	char    duration[MAX_TIME];
	char    frm_url_map[MAX_FRM_URL_MAP][MAX_FRM_URL];

};

enum _YOUTUBE_STATE
{
    YOUTUBE_NULL,
    YOUTUBE_ERROR,
    YOUTUBE_VIDEO_LIST_INIT,
    YOUTUBE_VIDEO_LIST_FOUND,
    YOUTUBE_VIDEO_LIST_TITLE,
    YOUTUBE_VIDEO_LIST_ID,
    YOUTUBE_VIDEO_LIST_UPLOADED,
    YOUTUBE_VIDEO_LIST_CREDIT,
    YOUTUBE_VIDEO_LIST_DESCRIPTION,
    YOUTUBE_VIDEO_LIST_KEYWORDS,
    YOUTUBE_VIDEO_LIST_DURATION,
};


struct _youtube_info
{
    enum _YOUTUBE_STATE  state;
    char                username[MAX_USERNAME];
    int                 error_code;
    video_entry_t       video_list;
    video_entry_t       *cur_video;
    char                auth[256+1];
	//char                referer[64];
	//char                token[64];
    int                 video_num;
	char                youtube_location[1024];

};

struct _youtube_video
{
	int youtube_video_size;
	int youtube_video_start;
	int youtube_video_data_end;
	int youtube_video_total;
	int youtube_video_idx;
	int youtube_video_ext;
	int youtube_video_waiting;
	int youtube_video_stop;
	int youtube_video_error;
	int youtube_video_cate;
	int youtube_video_page;
};

BYTE *youtube_getvideo_id(DWORD dwIndex);
int youtube_get_video_location(void);
int youtube_init(char *username,char *pasword,char youtube_categories,char youtube_pop_most,char youtube_page_idx); //*base_dir);



#endif //__xml_youtube_h__

