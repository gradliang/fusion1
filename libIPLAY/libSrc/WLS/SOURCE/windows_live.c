#include "windows_live.h"
#include "utiltypedef.h"

static char     windows_live_api_url[MAX_URL];
windows_live_info_t   windows_live_info;
static const char   *windows_live_base_dir;

int windows_live_init(const char *username, const char *base_dir)
{
    album_entry_t     *tmp_entry;
    int     ret;
    int     count;
    char    error[32];
	
	mpDebugPrint("\nwindows_live_init %s\n",username);

    memset(&windows_live_info, 0, sizeof(windows_live_info_t));
    strncpy(windows_live_info.username, username, MAX_USERNAME);
    windows_live_info.cur_album = &windows_live_info.album_list;
    windows_live_base_dir = base_dir;
#if	0
	ret = windows_live_get_login_homepage(&windows_live_info, username, "wifi12345678",error);
	ret = windows_live_get_css(&windows_live_info, username, "wifi12345678",error);
	ret = windows_live_get_javascript(&windows_live_info, username, "wifi12345678",error);
#else 
	ret = windows_live_id_sdk_login(&windows_live_info, username, "wifi12345678",error);
	//ret = windows_live_id_get_jpg(&windows_live_info,error);
    if (ret < 0)
    {
        if (ret == -2)
            mpDebugPrint("Login failed due to '%s'\n", error);
        return ret;
    }
	ret = windows_live_id_get_album(&windows_live_info,error);
	
    if (ret < 0)
    {
        if (ret == -2)
            mpDebugPrint("windows_live_id_get_album failed due to '%s'\n", error);
        return ret;
    }

    count = 0;
    windows_live_info.cur_album = windows_live_info.album_list.next;
    tmp_entry = windows_live_info.cur_album;
	
    if (tmp_entry)
    {
        while (tmp_entry)
        {
            mpDebugPrint("Fetch photos from '%s' with id '%s'\n",tmp_entry->title,tmp_entry->id);
			mpDebugPrint("counter %d",count);
            Net_Xml_PhotoSetList(tmp_entry->title,count);	
            count ++;
            tmp_entry = tmp_entry->next;
        }
        Net_PhotoSet_SetCount(count);	
    }
    else
    {
        mpDebugPrint("No album.\n");
    }
#endif
    return 0;
}

void windows_live_exit(const char *base_dir)
{
	mpDebugPrint("End windows_live_exit");
}

int windows_live_photolist_get(BYTE *PhotoSet)
{
	windows_live_info_t     *pwls_info;
	int     ret;
    int     count;
	
	mpDebugPrint("windows_live_photolist_get %s",PhotoSet);
	
	EnableNetWareTask();
	pwls_info = &windows_live_info;  	
    pwls_info->cur_album = pwls_info->album_list.next;
       
	while(pwls_info->cur_album)
	{
		if(!strcasecmp(pwls_info->cur_album->title, PhotoSet))
		{
			
			ret = windows_live_id_get_photo(pwls_info);
			
            if (ret < 0)
            {
                mpDebugPrint("error code: %d\n", pwls_info->error_code);
                return ret;
            }
            break;
        }	
		pwls_info->cur_album = pwls_info->cur_album->next;
	}
	return 0;
}

