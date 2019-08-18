/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#if NETWARE_ENABLE

#include "global612.h"
#include "mpTrace.h"
//#include "heaputil_mem.h"
#include "netfs.h"
#include "netfs_pri.h"
#include "xmlgce.h"
#include "net_api.h"
#include "..\..\lwip\include\net_sys.h"
#include "../../STD_DPF/main/include/ui_timer.h"


Net_Photo_Entry photo_info;

char  gce_api_url[MAX_URL];
static gce_info_t   gce_info;

//const char *refresh_flash = "https://www.halfbrowser.com/device ";
const char *refresh_url0  = "http://gce.sshen.com/demo-2";// "https://www.halfbrowser.com/device";
const char *refresh_url1 = "https://www.halfbrowser.com/device/Default.aspx";
const char *refresh_url2 =  "https://www.halfbrowser.com/device/sdk/Default.aspx";

const char *half0 = "https://www.halfbrowser.com/device";
const char *half1 =  "https://www.halfbrowser.com/device/Default.aspx";
const char *half2 = "https://www.halfbrowser.com/device/sdk/Default.aspx";
 
static const char *refresh_url = "http://220.132.173.57/home?actiontype=button&action=down&state=y:0,x:0";
//const char *refresh_url = "http://gce.google.com/device/home";
static const BYTE *filename = "gce.jpeg";
/**
 * dedicate function or integrate into netfs
 */
int gce_get_picture(char *url)
{
    	int len,len1,len2;
    	BYTE filesz[16];
   	
   	memset(&photo_info, 0x00, sizeof(photo_info));	
		
   	sprintf(photo_info.length,"%d",0);	
   	strncpy(photo_info.pathname,filename,strlen(filename));
   	strncpy(photo_info.url, url,strlen(url));
	gce_info.url = url;
		
   	MP_DEBUG1(">>>>>>>>>>>>>>>>>    gce_get_picture = %s",photo_info.url);
   	Net_Xml_parseFileList(&photo_info);
}

void gce_URL_refresh()
{
	MP_DEBUG("RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR");
	MP_DEBUG2("gce_URL_refresh  = %s    time = %d",gce_info.refresh_url,gce_get_refresh_interval());
	//RemoveTimerProc(gce_URL_refresh);
	NetFileBrowerInitial();
	gce_get_picture(gce_info.refresh_url);	
	xpgSearchAndGotoPage("GoogleViewer",12);	
	xpgViewPhoto();
	//AddTimerProc(gce_get_refresh_interval, gce_URL_refresh); 
} 

void gce_header_refresh_time(char *time,BYTE *http_header)
{
	gce_info.refresh_time = atoi(time);	
	MP_DEBUG1("gce_info.refresh_time = %d",gce_info.refresh_time);
}

void gce_header_refresh_URL(BYTE *http_header)
{
	memset(gce_info.refresh_url,0x00,MAX_URL);
	snprintf(gce_info.refresh_url,MAX_URL,"%s",http_header);
}

void gce_header_center_button(BYTE *http_header)
{
	memset(gce_info.button_url[BUTTON_ID_CENTER],0x00,MAX_URL);
    	snprintf(gce_info.button_url[BUTTON_ID_CENTER],MAX_URL,"%s",http_header);
	MP_DEBUG1("gce_header_center_button, url = '%s' ", gce_info.button_url[BUTTON_ID_CENTER]);	  	
}

void gce_header_left_button(BYTE *http_header)
{
	memset(gce_info.button_url[BUTTON_ID_LEFT],0x00,MAX_URL);
    	snprintf(gce_info.button_url[BUTTON_ID_LEFT],MAX_URL,"%s",http_header);
	MP_DEBUG1("gce_header_left_button, url = '%s' ", gce_info.button_url[BUTTON_ID_LEFT]);	
}

void gce_header_right_button(BYTE *http_header)
{
	memset(gce_info.button_url[BUTTON_ID_RIGHT],0x00,MAX_URL);
    	snprintf(gce_info.button_url[BUTTON_ID_RIGHT],MAX_URL,"%s",http_header);
	MP_DEBUG1("gce_header_right_button, url = '%s' ", gce_info.button_url[BUTTON_ID_RIGHT]);	
}

void gce_header_down_button(BYTE *http_header)
{
	memset(gce_info.button_url[BUTTON_ID_DOWN],0x00,MAX_URL);
    	snprintf(gce_info.button_url[BUTTON_ID_DOWN],MAX_URL,"%s",http_header);
	MP_DEBUG1("gce_header_down_button, url = '%s' ", gce_info.button_url[BUTTON_ID_DOWN]);	
}

void gce_header_up_button(BYTE *http_header)
{
	memset(gce_info.button_url[BUTTON_ID_UP],0x00,MAX_URL);
    	snprintf(gce_info.button_url[BUTTON_ID_UP],MAX_URL,"%s",http_header);
	MP_DEBUG1("gce_header_up_button, url = '%s' ", gce_info.button_url[BUTTON_ID_UP]);	
} 

void gce_header_esc_button(BYTE *http_header)
{
	memset(gce_info.button_url[BUTTON_ID_ESC],0x00,MAX_URL);
    	snprintf(gce_info.button_url[BUTTON_ID_ESC],MAX_URL,"%s",http_header);
	MP_DEBUG1("gce_header_esc_button, url = '%s' ", gce_info.button_url[BUTTON_ID_ESC]);	
}

int gce_button_center(void)
{
	gce_info.current_button = BUTTON_ID_CENTER; 
	gce_get_picture(gce_info.button_url[gce_info.current_button]);
	return 0;
}

int gce_button_up(void)
{
	gce_info.current_button = BUTTON_ID_UP;      
	gce_get_picture(gce_info.button_url[gce_info.current_button]);
	return 0;
}

int gce_button_down(void)
{
	gce_info.current_button = BUTTON_ID_DOWN;    
	gce_get_picture(gce_info.button_url[gce_info.current_button]);	
	return 0;
}

int gce_button_left(void)
{
	gce_info.current_button = BUTTON_ID_LEFT;    
	gce_get_picture(gce_info.button_url[gce_info.current_button]);
	return 0;
}

int gce_button_right(void)
{
	gce_info.current_button = BUTTON_ID_RIGHT;    
	gce_get_picture(gce_info.button_url[gce_info.current_button]);
	return 0;
}

int gce_button_esc(void)
{
	gce_info.current_button = BUTTON_ID_ESC;    
	gce_get_picture(gce_info.button_url[gce_info.current_button]);
	return 0;
}

int gce_get_refresh_interval()
{
    	return gce_info.refresh_time*10; //ms => min
}

BYTE *gce_get_refresh_url()
{
	return gce_info.refresh_url;	
}

BYTE *gce_get_current_url()
{
	return gce_info.url; 
}

int gce_init(const char *url, const char *base_dir)
{
	//RemoveTimerProc(gce_URL_refresh);
       memset(&gce_info, 0x00, sizeof(gce_info_t));
    	gce_info.current_button = BUTTON_ID_NULL;
       gce_get_picture(url);	
}

void gce_exit(const char *base_dir)
{
}

#endif











































