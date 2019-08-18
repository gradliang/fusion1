#ifndef _WEB_PHOTO_H_
#define _WEB_PHOTO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct share_filter     share_filter_t;
typedef struct share_folder     share_folder_t;

struct share_filter
{
    char    *mime;
    char    *ext_name;
};

struct share_folder
{
	char            *pathname;
    int             filter_count;
	share_filter_t  *filters;
};

int server_stop(void);
int server_start(const share_folder_t **search_list);
int server_init(void (*log_func)(const char *msg));
void server_end(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WEB_PHOTO_H_ */
