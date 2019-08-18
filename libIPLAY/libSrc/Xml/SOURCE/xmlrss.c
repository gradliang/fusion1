/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  1  
#include "corelib.h"

#if NETWARE_ENABLE

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <expat.h>
#include <time.h>
#include "ndebug.h"

#include "global612.h"
#include "mpTrace.h"
//#include "taskid.h"
//#include "heaputil_mem.h"
#include "xmlrss.h"
#include "netfs_pri.h"
#include "netfs.h"
#include "..\..\lwip\include\net_sys.h"

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define rss_malloc(sz)   mm_malloc(sz)
#define rss_mfree(ptr)   mm_free(ptr)

struct  netfs_file_entry xmlrss_info;
static html_parser_t html_parser;

static struct tm   tm_pub;
//static unsigned char http_buffer[4096];
extern Net_App_State App_State;
rss_map_t       *map = NULL;

static void desc_content_handler(void *user_data, const char *content, int len)
{
    rss_info_t      *rss_info = (rss_info_t *) user_data;
    char            *output = rss_info->desc_content;
    const char      *input = content;
    char            ch;

    if (rss_info->state == RSS_DESC_CONTENT)
    {
        
        while (*output && (output-rss_info->desc_content+1) < MAX_DESC)
            output ++;

        while ((output-rss_info->desc_content+1) < MAX_DESC && len > 0)
        {
            ch = *input;
            
            /* HTML 4.01: chapter 9.1 */
            if (ch == '\r' || ch == '\n')
                ch = ' ';
            else if (ch == '\t' || ch == '\f')
                ch = ' ';

            if ( !(ch == ' ' && (rss_info->has_space||rss_info->has_line)) )
            {
                *output = ch;
                output ++;
            }
            
            if (ch == ' ')
                rss_info->has_space = 1;
            else
            {
                rss_info->has_space = 0;
                rss_info->has_line = 0;
            }

            input ++;
            len --;
        }
        *output = 0;
    }
}

static void desc_tag_start_handler(void *user_data, const char *tag_name, char **attr)
{
    rss_info_t      *rss_info = (rss_info_t *) user_data;
    const char      *src_value = NULL;
    
    if (rss_info->state == RSS_DESC_CONTENT)
    {
        if (!strcasecmp(tag_name, "img"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "src"))
                {
                    attr ++;
                    
                    src_value = *attr;
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }
            
            if (src_value)
            {
                image_entry_t   *new_image;
                
                new_image = (image_entry_t *) rss_malloc(sizeof(image_entry_t));
                if (new_image)
                {
                    memset(new_image, 0, sizeof(image_entry_t));
                    snprintf(new_image->url, MAX_URL, src_value);
                    
                    new_image->next = rss_info->cur_item->image;
                    rss_info->cur_item->image = new_image;
                    //MP_DEBUG1("img='%s'\n", new_image->url);
                }
            }
        }
        if (!strcasecmp(tag_name, "br"))
        {
            if (rss_info->desc_content[0] != 0 && !rss_info->has_line)
            {
                strncat(rss_info->desc_content, "\n", 1);
            }
            rss_info->has_line = 1;
        }
        if (!strcasecmp(tag_name, "p"))
        {
            if (rss_info->desc_content[0] != 0 && !rss_info->has_line)
            {
                strncat(rss_info->desc_content, "\n\n", 2);
            }
            rss_info->has_line = 1;
        }
    }

}

static void desc_tag_end_handler(void *user_data, const char *tag_name)
{
    rss_info_t      *rss_info = (rss_info_t *) user_data;

    if (!strcasecmp(tag_name, "description"))
    {
        rss_info->cur_item->desclen = strlen(rss_info->desc_content);
        rss_info->cur_item->desc = (char *) rss_malloc(strlen(rss_info->desc_content)+1);
        strcpy(rss_info->cur_item->desc, rss_info->desc_content);
    }
}

static int decode_description(rss_info_t *rss_info, const char *desc)
{
    int ret = 0;
    
    html_init(&html_parser, rss_info);
    html_set_content_handler(&html_parser, desc_content_handler);
    html_set_tag_start(&html_parser, desc_tag_start_handler);
    html_set_tag_end(&html_parser, desc_tag_end_handler);

    rss_info->state = RSS_DESC_CONTENT,
    rss_info->desc_content[0] = 0;
    html_parse(&html_parser, desc, strlen(desc));

    html_exit(&html_parser);

exit:

    return ret;

fatal_exit:
    goto exit;
}

static void title_content_handler(void *user_data, const char *content, int len)
{
    rss_info_t      *rss_info = (rss_info_t *) user_data;
    char            *output = rss_info->desc_content;
    const char      *input = content;
    char            ch;

    if (rss_info->state == RSS_TITLE_CONTENT)
    {
        while (*output && (rss_info->desc_content-output+1) < MAX_DESC)
            output ++;

        while ((rss_info->desc_content-output+1) < MAX_DESC && len > 0)
        {
            ch = *input;
            
            /* HTML 4.01: chapter 9.1 */
            if (ch == '\r' || ch == '\n')
                ch = ' ';
            else if (ch == '\t' || ch == '\f')
                ch = ' ';

            *output = ch;
            output ++;
            input ++;
            len --;
        }
        *output = 0;
    }
}

static void title_tag_start_handler(void *user_data, const char *tag_name, char **attr)
{
    rss_info_t      *rss_info = (rss_info_t *) user_data;
    const char      *src_value = NULL;
    
    if (rss_info->state == RSS_TITLE_CONTENT)
    {
    }

}

static void title_tag_end_handler(void *user_data, const char *tag_name)
{
    rss_info_t      *rss_info = (rss_info_t *) user_data;

    if (!strcasecmp(tag_name, "title"))
    {
        //mpDebugPrint("1 title='%s'\n", rss_info->desc_content);
        rss_info->cur_item->titlelen = strlen(rss_info->desc_content);
        rss_info->cur_item->title = (char *) rss_malloc(strlen(rss_info->desc_content)+1);
        strcpy(rss_info->cur_item->title, rss_info->desc_content);
    }
}

static int decode_title(rss_info_t *rss_info, const char *title)
{
    int ret = 0;
    
    html_init(&html_parser, rss_info);
    html_set_content_handler(&html_parser, title_content_handler);
    html_set_tag_start(&html_parser, title_tag_start_handler);
    html_set_tag_end(&html_parser, title_tag_end_handler);

    rss_info->state = RSS_TITLE_CONTENT,
    rss_info->desc_content[0] = 0;
    html_parse(&html_parser, title, strlen(title));

exit:

    return ret;

fatal_exit:
    goto exit;
}

static void data_handler(void *user_data, const char *name, int len)
{
    rss_map_t       *rss_map = (rss_map_t *) user_data;
    rss_info_t      *rss_info = &rss_map->info;

    if (rss_info->state == RSS_TITLE)
    {
        if (strlen(rss_info->desc_buffer)+len+1 > MAX_DESC)
            len = MAX_DESC - strlen(rss_info->desc_buffer) - 1;
        strncat(rss_info->desc_buffer, name, len);
    }
    else if (rss_info->state == RSS_DESC)
    {
        if (strlen(rss_info->desc_buffer)+len+1 > MAX_DESC)
            len = MAX_DESC - strlen(rss_info->desc_buffer) - 1;
        strncat(rss_info->desc_buffer, name, len);
    }
    else if (rss_info->state == RSS_ITEM_TITLE)
    {
        if (strlen(rss_info->desc_buffer)+len+1 > MAX_DESC)
            len = MAX_DESC - strlen(rss_info->desc_buffer) - 1;
        strncat(rss_info->desc_buffer, name, len);
    }
    else if (rss_info->state == RSS_ITEM_DESCRIPTION)
    {
        if (strlen(rss_info->desc_buffer)+len+1 > MAX_DESC)
            len = MAX_DESC - strlen(rss_info->desc_buffer) - 1;
        strncat(rss_info->desc_buffer, name, len);
    }
    else if (rss_info->state == RSS_ITEM_PUBDATE)
    {
        if (strlen(rss_info->desc_buffer)+len+1 > MAX_DESC)
            len = MAX_DESC - strlen(rss_info->desc_buffer) - 1;
        strncat(rss_info->desc_buffer, name, len);
    }
}

static void start_element_handler(void *user_data, const char *name, const char **attr)
{
    rss_map_t       *rss_map = (rss_map_t *) user_data;
    rss_info_t      *rss_info = &rss_map->info;
    item_entry_t    *item_entry;
    const char      *url_value = NULL;
    const char      *length_value = NULL;

    if (rss_info->state == RSS_NULL)
    {
        if (!strcasecmp(name, "rss"))
            rss_info->state = RSS_START;
    }
    else if (rss_info->state == RSS_START)
    {
        if (!strcasecmp(name, "channel"))
        {
            rss_info->state = RSS_CHANNELs;
        }
    }
    else if (rss_info->state == RSS_CHANNELs)
    {
        if (!strcasecmp(name, "title"))
        {
            rss_info->state = RSS_TITLE;
            
            rss_info->desc_buffer[0] = 0;
        }
        else if (!strcasecmp(name, "description"))
        {
            rss_info->state = RSS_DESC;
            
            rss_info->desc_buffer[0] = 0;
        }
        else if (!strcasecmp(name, "item"))
        {
            rss_info->state = RSS_ITEM;

            memset(&xmlrss_info, 0, sizeof xmlrss_info);

            item_entry = (item_entry_t *) rss_malloc(sizeof(item_entry_t));
            memset(item_entry, 0, sizeof(item_entry_t));

            rss_info->item_no ++;
            rss_info->cur_item->next = item_entry;
            rss_info->cur_item = item_entry;
        }
    }
    else if (rss_info->state == RSS_ITEM)
    {
        if (!strcasecmp(name, "title"))
        {
            rss_info->state = RSS_ITEM_TITLE;
            sprintf(rss_info->desc_buffer, "<title>");
        }
        else if (!strcasecmp(name, "description"))
        {
            rss_info->state = RSS_ITEM_DESCRIPTION;
            sprintf(rss_info->desc_buffer, "<description>");
        }
        else if (!strcasecmp(name, "pubDate"))
        {
            rss_info->desc_buffer[0] = 0;
            rss_info->state = RSS_ITEM_PUBDATE;
        }
        else if (!strcasecmp(name, "enclosure"))
        {
            while (*attr)
            {
                if (!strcasecmp(*attr, "url"))
                {
                    attr ++;
                    url_value = *attr;
                }
                else if (!strcasecmp(*attr, "length"))
                {
                    attr ++;
                    length_value = *attr;
                }
                else
                {
                    attr ++;
                }
                
                attr ++;
            }

            rss_info->cur_item->media_url[0] = 0;
            rss_info->cur_item->media_length = 0;

            strncat(rss_info->cur_item->media_url, url_value, MAX_URL-1);
            if (length_value)
                rss_info->cur_item->media_length = atoi(length_value);;
        }
    }
}

static void end_element_handler(void *user_data, const char *name)
{
    rss_map_t       *rss_map = (rss_map_t *) user_data;
    rss_info_t      *rss_info = &rss_map->info;
    const char      *url;
    const char      *path;

    if (rss_info->state == RSS_NULL)
    {
    }
    else if (rss_info->state == RSS_START)
    {
        if (!strcasecmp(name, "rss"))
        {
            rss_info->state = RSS_NULL;
            /* finish */
        }
    }
    else if (rss_info->state == RSS_CHANNELs)
    {
        if (!strcasecmp(name, "channel"))
        {
            rss_info->state = RSS_START;
        }
    }
    else if (rss_info->state == RSS_TITLE)
    {
        if (!strcasecmp(name, "title"))
        {
            int title_len;
             
            title_len = strlen(rss_info->desc_buffer);
            rss_info->title = rss_malloc(title_len+1);
         
            if (rss_info->title)
            {
                strcpy(rss_info->title, rss_info->desc_buffer);
 		 		
                snprintf(xmlrss_info.pathname, NETFS_MAX_PATHNAME-1, "%s/title.txt", rss_map->base_dir);
		  //MP_DEBUG1("rss_info->title (%s)", xmlrss_info.pathname);		
                snprintf(xmlrss_info.url, NETFS_MAX_URL-1, "mem://%x:%d/", (unsigned int) rss_info->title, title_len);     
		 // MP_DEBUG1("rss_info->title (%s)", xmlrss_info.url);			
		  snprintf(xmlrss_info.length, 20, "%d", title_len);
		  //MP_DEBUG1("rss_info->title (%s)", xmlrss_info.length);
#if 0  
                netfs_add_file(&xmlrss_info);
#endif
            }

            rss_info->state = RSS_CHANNELs;
        }
    }
    else if (rss_info->state == RSS_DESC)
    {
        if (!strcasecmp(name, "description"))
        {
            int description_len;
            
	     description_len = strlen(rss_info->desc_buffer);
            rss_info->description = rss_malloc(description_len+1);
            if (rss_info->description)
            {
                strcpy(rss_info->description, rss_info->desc_buffer);
		
                snprintf(xmlrss_info.pathname, NETFS_MAX_PATHNAME-1, "%s/description.txt", rss_map->base_dir);
		  //MP_DEBUG1("rss_info->description < =%s ", xmlrss_info.pathname);
                snprintf(xmlrss_info.url, NETFS_MAX_URL-1, "mem://%x:%d/", (unsigned int) rss_info->description, description_len);
		  //MP_DEBUG1("rss_info->description < =%s ", xmlrss_info.url);		
                snprintf(xmlrss_info.length, 20, "%d", description_len);
		  //MP_DEBUG1("rss_info->description < =%s ", xmlrss_info.length);
#if 0
                netfs_add_file(&xmlrss_info);
#endif
		}

            rss_info->state = RSS_CHANNELs;
        }
    }
    else if (rss_info->state == RSS_ITEM)
    {

	
        if (!strcasecmp(name, "item"))
        {
        	MP_DEBUG("+++++++++++++++++++++++++++++++++++++++++++++++++");
            if (rss_info->cur_item->title)
            {
               snprintf(xmlrss_info.pathname, NETFS_MAX_PATHNAME-1, "%s/item%02d-%s/title.txt", rss_map->base_dir, rss_info->item_no, rss_info->cur_item->pubdate);
		 MP_DEBUG1("rss_info->cur_item->title < =%s ", xmlrss_info.pathname);
	        snprintf(xmlrss_info.url, NETFS_MAX_URL-1, "mem://%x:0x%x/", (unsigned int) rss_info->cur_item->title, rss_info->cur_item->titlelen);
		 MP_DEBUG1("rss_info->cur_item->title  < =%s ", xmlrss_info.url);	
		 MP_DEBUG1("title='%s'\n", rss_info->cur_item->title);
		 snprintf(xmlrss_info.length, 20, "%d", rss_info->cur_item->titlelen);
	        //MP_DEBUG1("rss_info->cur_item->title  < =%s ", xmlrss_info.length); 
 #if 1 
                netfs_add_file_with_type(&xmlrss_info,RSS_FILE_TYPE);
#endif
	     }

            if (rss_info->cur_item->desc)
            {
                snprintf(xmlrss_info.pathname, NETFS_MAX_PATHNAME-1, "%s/item%02d-%s/description.txt", rss_map->base_dir, rss_info->item_no, rss_info->cur_item->pubdate);
		  MP_DEBUG1("rss_info->cur_item->desc < =%s ", xmlrss_info.pathname);
		  snprintf(xmlrss_info.url, NETFS_MAX_URL-1, "mem://%x:%d/", (unsigned int) rss_info->cur_item->desc, rss_info->cur_item->desclen);
		  MP_DEBUG1("rss_info->cur_item->desc < =%s ", xmlrss_info.url);	
		  //MP_DEBUG1("content='%s'\n", rss_info->cur_item->desc);
		  snprintf(xmlrss_info.length, 20, "%d", rss_info->cur_item->desclen);
		  //MP_DEBUG1("rss_info->cur_item->desc < =%s ", xmlrss_info.length);
 #if 1
                netfs_add_file_with_type(&xmlrss_info,RSS_FILE_TYPE);
#endif
	         }

            image_entry_t   *cur_image = rss_info->cur_item->image;
            while (cur_image)
            {
                url_info_t      url_info;
                const char      *tail;
                const char      *filename;
                image_entry_t   *next_image;
                
                next_image = cur_image->next;
                           
		  xmlrss_info.length[0] = 0;
                MP_DEBUG("------------------------------------------------------------");		 

		#if 0
			snprintf(xmlrss_info.pathname, NETFS_MAX_PATHNAME-1, "%s/item%02d-%s/img.txt",rss_map->base_dir, rss_info->item_no, rss_info->cur_item->pubdate);
			MP_DEBUG1("xmlrss_info.pathname < =%s ", xmlrss_info.pathname);	


		#else
                snprintf(xmlrss_info.url, NETFS_MAX_URL-1, "%s", cur_image->url);
		  MP_DEBUG1("cur_image->url < =%s ", xmlrss_info.url);	
                snprintf(xmlrss_info.pathname, NETFS_MAX_PATHNAME-1, "%s/item%02d-%s/", 
                         rss_map->base_dir, rss_info->item_no, rss_info->cur_item->pubdate);
		  MP_DEBUG1("xmlrss_info.pathname < =%s ", xmlrss_info.pathname);	
		  
                utils_decode_url(xmlrss_info.url, &url_info);
                filename = url_info.path_s;
                tail     = url_info.path_s + url_info.path_len;
                while (strchr(filename, '/'))
                    filename = strchr(filename, '/') + 1;
                strncat(xmlrss_info.pathname, filename, tail-filename);
                MP_DEBUG1("netfs add (%s)\n", xmlrss_info.pathname);
		#endif
				
#if 1
                netfs_add_file_with_type(&xmlrss_info,RSS_FILE_TYPE);
#endif                
                /* we free memory since this image has added into filesystem */
                rss_mfree(cur_image);

                cur_image = next_image;
            }
            rss_info->cur_item->image = NULL;
            
            if (rss_info->cur_item->media_url[0])
            {
                url_info_t  url_info;
                const char    *tail;
                const char    *filename;
                
                MP_DEBUG("********************************************************************");
		  snprintf(xmlrss_info.length, 20, "%d", rss_info->cur_item->media_length);
                
                snprintf(xmlrss_info.url, NETFS_MAX_URL-1, "%s", rss_info->cur_item->media_url);
			MP_DEBUG1("rss_info->cur_item->media_url< =%s ", xmlrss_info.url);	
                snprintf(xmlrss_info.pathname, NETFS_MAX_PATHNAME-1, "%s/item%02d-%s/", 
                         rss_map->base_dir, rss_info->item_no, rss_info->cur_item->pubdate);
                utils_decode_url(xmlrss_info.url, &url_info);
			MP_DEBUG1("xmlrss_info.pathname < =%s ", xmlrss_info.pathname);		
                filename = url_info.path_s;
                tail     = url_info.path_s + url_info.path_len;
                while (strchr(filename, '/'))
                    filename = strchr(filename, '/') + 1;
                strncat(xmlrss_info.pathname, filename, tail-filename);
                MP_DEBUG1("netfs add (%s)\n", xmlrss_info.pathname);
#if 1 
                netfs_add_file_with_type(&xmlrss_info,RSS_FILE_TYPE);
#endif
		}

            rss_info->state = RSS_CHANNELs;
        }
    }
    else if (rss_info->state == RSS_ITEM_TITLE)
    {
        if (!strcasecmp(name, "title"))
        {
            strcat(rss_info->desc_buffer, "</title>");
            decode_title(rss_info, rss_info->desc_buffer);
            rss_info->state = RSS_ITEM;
        }
    }
    else if (rss_info->state == RSS_ITEM_PUBDATE)
    {
        if (!strcasecmp(name, "pubDate"))
        {
            /* RSS support date format: 
             *  1. Thu, 19 Apr 2007 06:08:02 GMT
             *  2. Wed, 16 May 2007 11:00 am CST
             *  3. Thu 19 Apr 2007 02:09:03 GMT
             *  4. Sun Nov  6 08:49:37 1994
             *  5. 2007/5/17 12/08/43
             *  6. 2007/05/11 08:21:10
             */
            if (!strptime(rss_info->desc_buffer, "%a, %d %b %Y %H:%M:%S", &tm_pub))
            {
                if (!strptime(rss_info->desc_buffer, "%a, %d %b %Y %H:%M %p", &tm_pub))
                {
                    if (!strptime(rss_info->desc_buffer, "%a %d %b %Y %H:%M:%S", &tm_pub))
                    {
                        if (!strptime(rss_info->desc_buffer, "%a %b %d %H:%M:%S %Y", &tm_pub))
                        {
                            if (!strptime(rss_info->desc_buffer, "%Y/%m/%d %H/%M/%S", &tm_pub))
                            {
                                if (!strptime(rss_info->desc_buffer, "%Y/%m/%d %H:%M:%S", &tm_pub))
                                {
                                    strptime("Mon, 7 Jan 1991 12:34:56", "%a, %d %b %Y %H:%M:%S", &tm_pub);
                                }
                            }
                        }
                    }
                }
            }

            strptime(rss_info->desc_buffer, "%a %d %b %Y %H:%M:%S ", &tm_pub);
            strftime(rss_info->cur_item->pubdate, MAX_DATE, "%Y%m%d%H%M%S", &tm_pub);
            rss_info->state = RSS_ITEM;
        }
    }
    else if (rss_info->state == RSS_ITEM_DESCRIPTION)
    {
        if (!strcasecmp(name, "description"))
        {
            strcat(rss_info->desc_buffer, "</description>");
            decode_description(rss_info, rss_info->desc_buffer);
            rss_info->state = RSS_ITEM;
        }
    }
}

static void decl_handler(void *user_data, const char *version, const char *encoding, int standalone)
{
    rss_map_t       *rss_map = (rss_map_t *) user_data;
    int             encoding_length;
    
    strncpy(rss_map->encoding, encoding, MAX_ENCODING);

    encoding_length = strlen(rss_map->encoding);
    snprintf(xmlrss_info.pathname, NETFS_MAX_PATHNAME-1, "%s/encoding.txt", 
            rss_map->base_dir);
    snprintf(xmlrss_info.url, NETFS_MAX_URL-1, "mem://%x:%d/",
            (unsigned int) rss_map->encoding, encoding_length);

    snprintf(xmlrss_info.length, 20, "%d", encoding_length);
#if 1 // CJ
    netfs_add_file_with_type(&xmlrss_info,RSS_FILE_TYPE);
#endif
}


int rss_init(const char *url, const char *base_dir)
{
    XML_Parser      parser;
    int ret;
#if Make_CURL
    int len;
    int i;

    rss_map_t       *map = NULL;
    DWORD flag;	
    char *data; 
    XML_BUFF_link_t *ptr;

    char *bigger_buff;

    /* Get Data from Remote Site and Parse it */
    map = (rss_map_t *) rss_malloc(sizeof(rss_map_t));

    if (!map)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    memset(map, 0, sizeof(rss_map_t));
    
    map->base_dir = (char *) rss_malloc(strlen(base_dir) + 1);
    if (!map->base_dir)
    {
        ret = -NETFS_NO_MEMORY;
        goto fatal_exit;
    }
    strcpy(map->base_dir, base_dir);
    
    map->prev = rss_map.prev;
    map->next = &rss_map;
    rss_map.prev->next = map;
    rss_map.prev = map;

    parser = XML_ParserCreate(NULL);
    map->info.state      = RSS_NULL;
    map->info.cur_item   = &map->info.item_list;

    XML_SetUserData(parser, map);
    XML_SetElementHandler(parser, start_element_handler, end_element_handler);
    XML_SetCharacterDataHandler(parser, data_handler);
    XML_SetXmlDeclHandler(parser, decl_handler);

    Xml_BUFF_init(NET_RECVHTTP);	
    ret = Net_Recv_Data(url,NETFS_RSS,0,0);
    if(ret <0) 
    		goto Recv_exit;

#if 0 //original code
    ptr = App_State.XML_BUF;
    while (ptr != NULL)
    {
		data = ptr->BUFF;
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
    MP_DEBUG("rss_init: got total data len = %d", len);

    if (XML_Parse(parser, bigger_buff, len, 0) == XML_STATUS_ERROR)
    {
        MP_DEBUG3("rss_init: %s at line %d, column %d\n",
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
    if (map)
    {
        if (map->base_dir)
            rss_mfree(map->base_dir);
        rss_mfree(map);
    }
    goto exit;
#else
       return ret;
#endif	

}	

void rss_exit(const char *base_dir)
{
    rss_map_t       *map;
    item_entry_t    *tmp_entry;
    
    map = rss_map.next;
    while (map != &rss_map)
    {
        if (!strcmp(map->base_dir, base_dir))
            break;

        map = map->next;
    }
    
    if (map == &rss_map)
        return;

    if (map->info.description)
        rss_mfree(map->info.description);

    if (map->info.title)
        rss_mfree(map->info.title);

    map->info.cur_item = map->info.item_list.next;
    while (map->info.cur_item)
    {
        tmp_entry = map->info.cur_item;
        map->info.cur_item = map->info.cur_item->next;

        if (tmp_entry->title)
            rss_mfree(tmp_entry->title);
        if (tmp_entry->desc)
            rss_mfree(tmp_entry->desc);
        rss_mfree(tmp_entry);
    }
    
    if (map)
    {
        map->prev->next = map->next;
        map->next->prev = map->prev;

        if (map->base_dir)
            rss_mfree(map->base_dir);
        rss_mfree(map);
    }
}
#endif

