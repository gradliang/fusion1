#ifndef __xml_gce_h__
#define __xml_gce_h__

//#define MAX_FILE_BUFFER     (4096*32)
#define MAX_URL         256

typedef enum _BUTTON_ID BUTTON_ID_e;

enum _BUTTON_ID
{
    BUTTON_ID_NULL = -1,     /* special button id */	
    BUTTON_ID_CENTER = 0,
    BUTTON_ID_LEFT,
    BUTTON_ID_RIGHT,
    BUTTON_ID_DOWN,
    BUTTON_ID_UP,    
    BUTTON_ID_ESC,  
    BUTTON_COUNT,       /* total button count */    
};


typedef struct _gce_info    gce_info_t;

struct _gce_info
{
    char    button_url[BUTTON_COUNT][MAX_URL];
    char    refresh_url[MAX_URL];
    char    *url;
    BUTTON_ID_e  current_button;
    
    DWORD     refresh_time;
};




#endif //__xml_gce_h__
