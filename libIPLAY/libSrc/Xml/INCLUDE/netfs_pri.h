#ifndef _NETFS_PRI_H_
#define _NETFS_PRI_H_

//#include <netinet/in.h>
#include "netfs.h"
#include "dirent.h"
typedef struct _http_info			http_info_t;
typedef struct _http_header_handler	http_header_handler_t;
typedef struct _proxy_info          proxy_info_t;
typedef struct _socks_info          socks_info_t;

typedef void (*header_func_t)(http_info_t *http_info, const char *name, const char *value);

struct _http_header_handler
{
    http_header_handler_t 	*next;
    http_header_handler_t	*prev;

    const char      *name;
    header_func_t   handler;
};

struct _proxy_info
{
    char        hostname[64];
    int         port;
    char        username[32];
    char        password[32];
};

struct _socks_info
{
    char        hostname[64];
    int         version;
    char        username[32];
    char        password[32];
};

struct _http_info
{
    int         state;
    int         sock_fd;
    unsigned char   *buffer;
    unsigned char   *ptr;
    int         buffer_size;
    int         buffer_length;
    int         status_code;

    char        hostname[64];
    int         port;
    char        path[256];          /* include path and search */

    proxy_info_t  proxy;
    socks_info_t  socks;
    
    http_header_handler_t   header_handler;
    time_t      modified_time;      /* this field filled by http_header_last_modified() */
    char        accept_ranges;      /* this field filled by http_header_accept_ranges() */
    char        chunked_transfer;   /* this field filled by http_header_transfer_encoding() */
    int         chunked_size;       /* this field filled by http_header_transfer_encoding() */
    int         content_length;     /* this field filled by http_header_content_length() */
    char        location[256];      /* this field filled by http_header_location() */

    void		*client_data;
};


struct url_info_s
{
    const char  *scheme_s;
    int         scheme_len;
    const char  *user_s;
    int         user_len;
    const char  *password_s;
    int         password_len;
    const char  *host_s;
    int         host_len;
    const char  *port_s;
    int         port_len;
    const char  *path_s;
    int         path_len;
    const char  *search_s;  
    int         search_len;
};

typedef struct url_info_s   url_info_t;

void utils_decode_url(const char *old_url, url_info_t *url_info);
//char *utils_formalize_url(const char *old_url, struct in_addr server_in);
//char *utils_formalize_url(const char *old_url, in_addr_st server_in); 
const char *utils_decode_as_filepath(const char *file_url);

int http_client_init(http_info_t *http_info, const char *url, unsigned char *buffer, int buffer_size, proxy_info_t *proxy_info, socks_info_t *socks_info);
int http_client_exit(http_info_t *http_info);
int http_client_add_header_handler(http_info_t *http_info, http_header_handler_t *handler);
int http_client_GCE_request(http_info_t *http_info);
int http_client_request(http_info_t *http_info);
int http_client_request_partial(http_info_t *http_info, int range1, int range2);
int http_client_recv_header(http_info_t *http_info);
int http_client_recv_data(http_info_t *http_info, unsigned char *data, int datalen);
void http_header_location(http_info_t *http_info, const char *name, const char *value);
void http_header_connection(http_info_t *http_info, const char *name, const char *value);
void http_header_transfer_encoding(http_info_t *http_info, const char *name, const char *value);
void http_header_last_modified(http_info_t *http_info, const char *name, const char *value);
void http_header_content_length(http_info_t *http_info, const char *name, const char *value);
void http_header_accept_ranges(http_info_t *http_info, const char *name, const char *value);



typedef void (*html_content_handler_t)(void *user_data, const char *content, int len);
typedef void (*html_tag_start_handler_t)(void *user_data, const char *tag_name, char **attr);
typedef void (*html_tag_end_handler_t)(void *user_data, const char *tag_name);
typedef struct _html_parser html_parser_t;
typedef enum _HTML_STATE    HTML_STATE;

enum _HTML_STATE
{
    HTML_NULL,
    HTML_CONTENT,
    HTML_TAG_NAME,
    HTML_TAG_ATTR,
    HTML_PI,
    HTML_PI_E1,
    HTML_CDATA_COMMENT,
    HTML_CDATA_S1,
    HTML_CDATA_S2,
    HTML_CDATA_S3,
    HTML_CDATA_S4,
    HTML_CDATA_S5,
    HTML_CDATA_S6,
    HTML_CDATA,
    HTML_CDATA_E1,      /* match 1st ']' */
    HTML_CDATA_E2,      /* match 2nd ']' */
    HTML_COMMENT_S1,
    HTML_COMMENT,
    HTML_COMMENT_E1,
    HTML_COMMENT_E2,
};

struct tag_entry
{
    char name[64];
    
    struct tag_entry *next;
    struct tag_entry *prev;
};

/* flag bit mask */
enum _HTML_FLAG
{
    HTML_FLAG_TAG_CLOSE         = 1,
    HTML_FLAG_ATTR_DOUBLE_QUOTE = 2,
    HTML_FLAG_ATTR_QUOTE        = 4,
    HTML_FLAG_ATTR_NAME         = 8,
    HTML_FLAG_ATTR_VALUE        = 16,
};

#define MAX_TAG_NAME    32
#define MAX_ATTR_DATA   1024    /* for both name, value */
#define MAX_ATTRS       16      /* total count of name plus value */
#define MAX_CONTENT     1024

struct _html_parser
{
    html_content_handler_t      content_handler;
    html_tag_start_handler_t    tag_start_handler;
    html_tag_end_handler_t      tag_end_handler;
    HTML_STATE      state;
    unsigned int    flag;

    char    *last_content;
    char    *last_tag_name;
    char    *last_attr_data;
    char    content[MAX_CONTENT];
    char    tag_name[MAX_TAG_NAME];
    char    attr_data[MAX_ATTR_DATA];
    char    *attrs[MAX_ATTRS+1];

    struct tag_entry    tag_list;

    void    *user_data;
};

void html_init(html_parser_t *parser, void *user_data);
void html_set_content_handler(html_parser_t *parser, html_content_handler_t content_handler);
void html_set_tag_start(html_parser_t *parser, html_tag_start_handler_t tag_start_handler);
void html_set_tag_end(html_parser_t *parser, html_tag_end_handler_t tag_end_handler);
void html_parse(html_parser_t *parser, const char *html_input, int len);
void html_exit(html_parser_t *parser);



#define NETFS_MAX_PATHNAME    256
#define NETFS_MAX_URL         512
#define  NETFS_MAX_FILENAME          256

typedef struct netfs_file_entry Net_Photo_Entry;

struct netfs_file_entry
{
    //unsigned char    filename[NETFS_MAX_FILENAME];	
    char    pathname[NETFS_MAX_PATHNAME];
    char    length[20];
    char    url[NETFS_MAX_URL];
    char    thumbnail_url[NETFS_MAX_URL];
	int		file_size;
};

//Kevin Move
typedef struct _netfs_file_entry    netfs_file_entry_t;
typedef struct _netfs_meta_entry    netfs_meta_entry_t;
typedef struct _netfs_mount_point   netfs_mount_point_t;

#define UNKNOW_FILE_TYPE	0
#define IMAGE_FILE_TYPE		1
#define AUDIO_FILE_TYPE		2
#define VIDEO_FILE_TYPE		3
#define RSS_FILE_TYPE		4

struct _netfs_file_entry
{
    char    open_count;

    char    *url;
    char    *thumbnail_url;
    char    *hostname;
    char    *path;
    int     port;
    int     file_length;

    http_info_t     *http_info;
    unsigned char   *http_buffer;
    int             http_bufsize;

    unsigned char   *buffer;
    int     bufpos;
    int     buflen;
    int     bufsize;
	unsigned char filetype;
};

struct _netfs_meta_entry
{
    char    is_dir;
    char    *name;      /* filename / directory name */
    char    *containerid;      /* valid for upnp directory */
    netfs_file_entry_t	*file_entry;  /* valid for file */
    netfs_meta_entry_t	*child;     /* valid for directory */

    netfs_meta_entry_t	*next;      /* next file entry with the same parent */
    netfs_meta_entry_t	*prev;      /* next file entry with the same parent */
	unsigned short childcount;
	unsigned short curchildcount;
};

struct _netfs_mount_point
{
    char                *base_dir;
    
    netfs_type_t        type;
    time_t              startup_time;
    int                 remote_refresh;
    
    struct _netfs_mount_point   *next;
    struct _netfs_mount_point   *prev;
};

/* for stream-base POSIX internal API */
struct _netfs_dir_stream
{
    int     pos;
    netfs_meta_entry_t  *first_entry;
    netfs_meta_entry_t  *current_entry;
};

typedef struct _netfs_dir_stream    netfs_dir_stream_t;

struct _netfs_file_stream
{
    netfs_file_entry_t  *file_entry;
    netfs_mount_point_t *mount_point;

    int     curpos;
};

typedef struct _netfs_file_stream   netfs_file_stream_t;

#define HTTP_BUFFER_SIZE    (4096*2)
#define FILE_BUFFER_SIZE    (4096*4)

typedef struct _netfs_info  netfs_info_t;

struct _netfs_info
{
    netfs_meta_entry_t  root_entry;
    netfs_meta_entry_t  *cwd_entry;
    unsigned char       http_buffer[HTTP_BUFFER_SIZE];
    unsigned char       file_buffer[FILE_BUFFER_SIZE];
    struct dirent       dirent;
    
    netfs_mount_point_t mount_root;
    netfs_mount_point_t *cwd_mount_point;
    
    netfs_type_t        type;
    time_t              startup_time;
    int                 remote_refresh;
};

//End Kevin
int magicpixel_init(const char *url, const char *base_dir);
void magicpixel_exit(const char *base_dir);
int flickr_init(const char *username, const char *base_dir);
int flickr_interesting_init(const char *base_dir);
void flickr_exit(const char *base_dir);
int wretch_init(const char *username, const char *base_dir);
void wretch_exit(const char *base_dir);
int rss_init(const char *url, const char *base_dir);
void rss_exit(const char *base_dir);
int picasa_init(const char *username, const char *base_dir);
void picasa_exit(const char *base_dir);

int ygp_init(const char * username, const char * base_dir);
void ygp_exit(const char * base_dir);

//int vtuner_init(const char *vtuner_url, const char *base_dir);
int vtuner_init(char *vtuner_url, char *base_dir);

//void vtuner_exit(const char *base_dir);
void vtuner_exit(char *base_dir);

int shoutcast_init(const char *shoutcast_url, const char *base_dir);
void shoutcast_exit(const char *base_dir);

int framechannel_init(const char *url, const char *base_dir);
void framechannel_exit(const char *base_dir);

int frameit_init(const char *url, const char *base_dir);
void frameit_exit(const char *base_dir);

int snapfish_init(char * snapfish_url, char * base_dir);
void snapfish_exit(char * base_dir);

int reset_service_list(void);


int facebook_init(const char *username, const char *base_dir);
void facebook_exit(const char *base_dir);

extern proxy_info_t        netfs_proxy_info;
extern socks_info_t        netfs_socks_info;

//void netfs_set_startup_time(time_t tm, const char *base_dir);
//int netfs_add_file(struct netfs_file_entry *new_file);

#endif /* _NETFS_PRI_H_ */
