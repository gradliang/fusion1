/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0 
#include "corelib.h"

#if NETWARE_ENABLE

#include "global612.h"
#include "mpTrace.h"
#include "expat.h"
#include "netfs_pri.h"
#include "netware.h"
#include "netfs.h"
#include "..\..\lwip\include\net_sys.h"
 
Net_Photo_Entry jpeg_info;

static const char   *mpx_base_dir;

extern Net_App_State App_State;

DWORD *pdwExtArray;
static DWORD  g_check;

static void data_handler(void *user_data, const char *name, int len)
{
    enum RSS_STATE  *rss_state = (enum RSS_STATE *) user_data;
    char ch[5];
	
    if (*rss_state == RSS_ITEM_TITLE)
    {
	//strncpy(jpeg_info.filename,name,len);	
	strncpy(jpeg_info.pathname,name,len);	
	g_check = 1;
		//MP_DEBUG2("1 RSS_ITEM_TITLE = %s  length = %d,",jpeg_info.pathname,len);	       
    }
    else if (*rss_state == RSS_ITEM_LINK)
    {
    	 if (strlen(jpeg_info.url)+len+1 > NETFS_MAX_URL)
            len = NETFS_MAX_URL - strlen(jpeg_info.url) - 1;

		strncat(jpeg_info.url, name, len);
              //cj add to check the 4096byte xml parser problem
		ch[0] = *(name+len-4);
	 	ch[1] = *(name+len-3);
	 	ch[2] = *(name+len-2);
	 	ch[3] = *(name+len-1);
	 	ch[4] = '\0';
	
		//MP_DEBUG1("======> %s",ch);
	 	if (!strcasecmp(ch, ".JPG"))
	 		g_check++;
	    	//MP_DEBUG2 ("2 RSS_ITEM_LINK = %s %d",jpeg_info.url , len);	
    }
    else if (*rss_state == RSS_ITEM_SIZE)
    {
        if (strlen(jpeg_info.length)+len+1 > 20)
            len = 20 - strlen(jpeg_info.length) - 1;
        	strncat(jpeg_info.length, name, len);
		g_check++;
		//MP_DEBUG1("3 RSS_ITEM_SIZE = %s",jpeg_info.length);			
    }   
    	
    if(g_check <3)
		return;
    else
    {
		g_check = 0;
		Net_Xml_parseFileList(&jpeg_info);
    }	
	
}

static void start_element_handler(void *user_data, const char *name, const char **attr)
{
    enum RSS_STATE  *rss_state = (enum RSS_STATE *) user_data;

    if (*rss_state == RSS_NULL)
    {
        if (!strcasecmp(name, "rss"))
            *rss_state = RSS_START;
    }
    else if (*rss_state == RSS_START)
    {
        if (!strcasecmp(name, "channel"))
        {
            *rss_state = RSS_CHANNELs;
        }
    }
    else if (*rss_state == RSS_CHANNELs)
    {
        if (!strcasecmp(name, "item"))
        {
            *rss_state = RSS_ITEM;
            memset(&jpeg_info, 0, sizeof jpeg_info);
        }
    }
    else if (*rss_state == RSS_ITEM)
    {
//        if (!strcasecmp(name, "enclosure"))
//        {
//            while (*attr)
//            {
//                if (!strcasecmp(*attr, "url"))
//                {
//                    attr ++;
//                    url_value = *attr;
//                }
//                else if (!strcasecmp(*attr, "length"))
//                {
//                    attr ++;
//                    length_value = *attr;
//                }
//                else if (!strcasecmp(*attr, "type"))
//                {
//                    attr ++;
//                    mime_value = *attr;
//                }
//                else
//                {
//                    attr ++;
//                }
//                
//                attr ++;
//            }
//        }
        
        if (!strcasecmp(name, "title"))
        {
            *rss_state = RSS_ITEM_TITLE;
        }
        else if (!strcasecmp(name, "link"))
        {
            *rss_state = RSS_ITEM_LINK;
        }
        else if (!strcasecmp(name, "size"))
        {
            *rss_state = RSS_ITEM_SIZE;
        }
    }
}

static void end_element_handler(void *user_data, const char *name)
{
    enum RSS_STATE  *rss_state = (enum RSS_STATE *) user_data;
    const char      *url;
    const char      *path;

    if (*rss_state == RSS_NULL)
    {
    }
    else if (*rss_state == RSS_START)
    {
        if (!strcasecmp(name, "rss"))
        {
            *rss_state = RSS_NULL;
            /* finish */
        }
    }
    else if (*rss_state == RSS_CHANNELs)
    {
        if (!strcasecmp(name, "channel"))
        {
            *rss_state = RSS_START;
        }
    }
    else if (*rss_state == RSS_ITEM)
    {
        if (!strcasecmp(name, "item"))
        {
            *rss_state = RSS_CHANNELs;

            url = strchr(jpeg_info.url+7, '/');
			
            path = utils_decode_as_filepath(url);
            snprintf(jpeg_info.pathname, NETFS_MAX_PATHNAME, "%s%s",  mpx_base_dir, path);
#if 0 //cj not need to add netfs			
            netfs_add_file(&jpeg_info);
#endif
	//mpDebugPrint("path = %s",jpeg_info.pathname); 
	}
    }
    else if (*rss_state == RSS_ITEM_TITLE)
    {
        if (!strcasecmp(name, "title"))
        {
            *rss_state = RSS_ITEM;
        }
    }
    else if (*rss_state == RSS_ITEM_LINK)
    {
        if (!strcasecmp(name, "link"))
        {
            *rss_state = RSS_ITEM;
        }
    }
    else if (*rss_state == RSS_ITEM_SIZE)
    {
        if (!strcasecmp(name, "size"))
        {
            *rss_state = RSS_ITEM;
        }
    }
}

int magicpixel_init(const char *url, const char *base_dir)
{
#if Make_CURL
    enum RSS_STATE  rss_state;
    XML_Parser parser;
    int ret;
    int len;
    
    char *data; 
    XML_BUFF_link_t *ptr;

    char *bigger_buff;
    
    mpx_base_dir = base_dir;

    /* Get Data from Remote Site and Parse it */
    //MP_DEBUG1("fetch:%s", url);

    parser = XML_ParserCreate(NULL);
    rss_state = RSS_NULL;

    XML_SetUserData(parser, &rss_state);
    XML_SetElementHandler(parser, start_element_handler, end_element_handler);
    XML_SetCharacterDataHandler(parser, data_handler);

    /* LAN only, bypass proxy and socks */
	  g_check = 0;
    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(url,NETFS_MPX_LIST,0,0);
    if(ret <0) 
    	goto Recv_exit;	

#if 0 //original code
    ptr = App_State.XML_BUF;
    while (ptr != NULL)
    {
        data =  ptr->BUFF;
        len = ptr->buff_len;
        if (XML_Parse(parser, data, len, 0) == XML_STATUS_ERROR)
        {
           	    MP_DEBUG3("rss_init: %s at line %d, column %d\n",
                  XML_ErrorString(XML_GetErrorCode(parser)),
                  XML_GetCurrentLineNumber(parser),
                  XML_GetCurrentColumnNumber(parser));
        }
        ptr = ptr->link;		
    }
#else //new code
    bigger_buff = Merge_ListData_to_SingleBuffer(App_State.XML_BUF, &len);
    MP_DEBUG("magicpixel_init: got total data len = %d", len);

    if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
    {
        MP_DEBUG3("magicpixel_init: %s at line %d, column %d\n",
        XML_ErrorString(XML_GetErrorCode(parser)),
        XML_GetCurrentLineNumber(parser),
        XML_GetCurrentColumnNumber(parser));
    }
#endif

    XML_Parse(parser, data, 0, 1);

exit:
    if(bigger_buff != NULL)
        ext_mem_free(bigger_buff);
   
    Xml_BUFF_free(NET_RECVHTTP);    
    return ret;   

Recv_exit:
fatal_exit:
    ret = -1;
    goto exit;
#else
   return 0;
#endif
}

void magicpixel_exit(const char *base_dir)
{
}
#endif
