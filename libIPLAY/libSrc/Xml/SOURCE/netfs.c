/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
#include "global612.h"
#include "mpTrace.h"
#include "dirent.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fcntl.h"
#include <time.h>
//#include <ainet.h>
//#include <types.h>
//#include <sys/socket.h>
#include "netfs.h"
#include "netfs_pri.h"
#if NETWARE_ENABLE	
Net_Photo_Entry photo_info;
unsigned char media_file_type;
/* for networking virtual filesystem */
#if 0	//Put it to netfs_pri.h
typedef struct _netfs_file_entry    netfs_file_entry_t;
typedef struct _netfs_meta_entry    netfs_meta_entry_t;
typedef struct _netfs_mount_point   netfs_mount_point_t;
#endif
#if 0
struct _netfs_file_entry
{
    char    open_count;

    char    *url;
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
};

struct _netfs_meta_entry
{
    char    is_dir;
    char    *name;      /* filename / directory name */

    netfs_file_entry_t	*file_entry;  /* valid for file */
    netfs_meta_entry_t	*child;     /* valid for directory */

    netfs_meta_entry_t	*next;      /* next file entry with the same parent */
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
#endif
static netfs_info_t     netfs_info;
proxy_info_t        netfs_proxy_info;
socks_info_t        netfs_socks_info;

static netfs_mount_point_t *netfs_add_mount_point(const char *path)
{
    netfs_mount_point_t *point;
    const char *delimiter;
    const char *base_dir;
    int         base_dir_len;

    point = (netfs_mount_point_t *) ext_mem_malloc(sizeof(netfs_mount_point_t));

    if (!point)
        return NULL;
    memset(point, 0, sizeof(netfs_mount_point_t));

    /* currenty working directory has changed to this path */
    if (netfs_info.cwd_mount_point != &netfs_info.mount_root && *path != '/')
        return netfs_info.cwd_mount_point;

    {
        /* skip leading '/' */
        while (*path && *path == '/')
            path ++;

        base_dir = path;
        delimiter = strchr(base_dir, '/');
    }

    if (delimiter)
        base_dir_len = delimiter - base_dir;
    else
        base_dir_len = strlen(base_dir);

    point->base_dir = (char *)ext_mem_malloc(base_dir_len+1);
    if (!point->base_dir)
    {
        ext_mem_free(point);
        return NULL;
    }
    strcpy(point->base_dir, base_dir);

    point->next = &netfs_info.mount_root;
    point->prev = netfs_info.mount_root.prev;
    netfs_info.mount_root.prev->next = point;
    netfs_info.mount_root.prev = point;
    
    return point;
}

static netfs_mount_point_t *netfs_search_mount_point(const char *path)
{
    netfs_mount_point_t *point;
    const char *delimiter;
    const char *base_dir;
    int         base_dir_len;
    
    /* currenty working directory has changed to this path */
    if (netfs_info.cwd_mount_point != &netfs_info.mount_root && *path != '/')
        return netfs_info.cwd_mount_point;

    {
        /* skip leading '/' */
        while (*path && *path == '/')
            path ++;

        base_dir = path;
        delimiter = strchr(base_dir, '/');
    }
    
    if (delimiter)
        base_dir_len = delimiter - base_dir;
    else
        base_dir_len = strlen(base_dir);

    point = netfs_info.mount_root.next;
    while (point != &netfs_info.mount_root)
    {
        if (!strncmp(base_dir, point->base_dir, base_dir_len))
        {
            break;
        }
        
        point = point->next;
    }
    
    if (point == &netfs_info.mount_root)
        point = NULL;
    
    return point;
}

static int netfs_delete_mount_point(const char *base_dir)
{
    netfs_mount_point_t *point;
    
    point = netfs_search_mount_point(base_dir);
    if (!point)
        return -NETFS_ERROR;
    
    point->next->prev = point->prev;
    point->prev->next = point->next;
    
    ext_mem_free(point->base_dir);
    ext_mem_free(point);
    
    return NETFS_OK;
}

void netfs_set_startup_time(time_t tm, const char *base_dir)
{
    netfs_mount_point_t *point;
    
    point = netfs_search_mount_point(base_dir);
    if (!point)
        return;
        
    point->startup_time = tm;
    
	netfs_info.startup_time = tm;
}

static netfs_meta_entry_t *netfs_create_file_entry(const char *dirname, struct netfs_file_entry *new_file)
{
    netfs_meta_entry_t  *meta_entry = NULL;
    netfs_file_entry_t  *file_entry = NULL;
    url_info_t          url_info;

	//mpDebugPrint("###############media_file_type %d",media_file_type);
    /* insert new file entry */
    meta_entry = (netfs_meta_entry_t *) ext_mem_malloc(sizeof(netfs_meta_entry_t));
    if (!meta_entry)
        goto fatal_exit;
    memset(meta_entry, 0, sizeof(netfs_meta_entry_t));

    meta_entry->name = (char *) ext_mem_malloc(strlen(dirname)+1);
    if (!meta_entry->name)
        goto fatal_exit;
    memset(meta_entry->name, 0, strlen(dirname)+1);

    file_entry = (netfs_file_entry_t *) ext_mem_malloc(sizeof(netfs_file_entry_t));
    if (!file_entry)
        goto fatal_exit;
    memset(file_entry, 0, sizeof(netfs_file_entry_t));

    /* fill all fields of file_entry */
    file_entry->url = (char *) ext_mem_malloc(strlen(new_file->url) + 1);
    if (!file_entry->url)
        goto fatal_exit;
	
    strcpy(file_entry->url, new_file->url);
	/* add thumbnailurl */

	if( media_file_type == IMAGE_FILE_TYPE )
	{
		MP_DEBUG("media_file_type == IMAGE_FILE_TYPE");
		file_entry->thumbnail_url = (char *) ext_mem_malloc(strlen(new_file->thumbnail_url) + 1);
		MP_DEBUG("file_entry->thumbnail_url %x",file_entry->thumbnail_url);
		if (!file_entry->thumbnail_url)
			goto fatal_exit;
		strcpy(file_entry->thumbnail_url, new_file->thumbnail_url);
	}

    utils_decode_url(file_entry->url, &url_info);
    if (url_info.host_len > 0)
    {
        file_entry->hostname = (char *) ext_mem_malloc(url_info.host_len + 1);
        if (!file_entry->hostname)
            goto fatal_exit;
        strncpy(file_entry->hostname, url_info.host_s, url_info.host_len);
        file_entry->hostname[url_info.host_len] = 0;
    }
    if (url_info.path_len > 0)
    {
        file_entry->path = (char *) ext_mem_malloc(url_info.path_len + 1 + url_info.search_len + 1);
        if (!file_entry->path)
            goto fatal_exit;
        strncpy(file_entry->path, url_info.path_s, url_info.path_len);
        file_entry->path[url_info.path_len] = 0;
        if (url_info.search_s)
        {
            strncat(file_entry->path, "?", 1);
            strncat(file_entry->path, url_info.search_s, url_info.search_len);
        }
    }
    if (url_info.port_len > 0)
        file_entry->port = atoi(url_info.port_s);
    else
        file_entry->port = 80;
    if (new_file->length[0])
        file_entry->file_length = atoi(new_file->length);

	file_entry->filetype = media_file_type;
	
//modified by CJ for filetype because of the file type is different for file_size   	
    if((file_entry->filetype == IMAGE_FILE_TYPE) ||
	(file_entry->filetype == AUDIO_FILE_TYPE))	
		file_entry->file_length = new_file->file_size;

	MP_DEBUG("file_entry->filetype %x",file_entry->filetype);
	MP_DEBUG("file_entry: hostname='%s',port='%d',path='%s,length='%d'\n",
            file_entry->hostname,
            file_entry->port,
            file_entry->path,
	     file_entry->file_length);

    /* complete meta_entry */
    meta_entry->is_dir = 0;
    strcpy(meta_entry->name, dirname);
    meta_entry->file_entry = file_entry;

    return meta_entry;

fatal_exit:
    if (meta_entry)
    {
        if (meta_entry->name)
            ext_mem_free(meta_entry->name);
        ext_mem_free(meta_entry);
    }
    if (file_entry)
    {
        if (file_entry->url)			
            ext_mem_free(file_entry->url);
        if (file_entry->thumbnail_url)
            ext_mem_free(file_entry->url);
        if (file_entry->hostname)
            ext_mem_free(file_entry->hostname);
        if (file_entry->path)
            ext_mem_free(file_entry->path);
        ext_mem_free(file_entry);
    }

    return NULL;
}

static netfs_meta_entry_t *netfs_create_dir_entry(const char *dirname)
{
    netfs_meta_entry_t  *meta_entry = NULL;

    //mpDebugPrint("netfs_create_dir_entry(%s)\n", dirname);
    /* insert new dir into netfs */
    meta_entry = (netfs_meta_entry_t *) ext_mem_malloc(sizeof(netfs_meta_entry_t));
    if (!meta_entry)
        goto fatal_exit;
    memset(meta_entry, 0, sizeof(netfs_meta_entry_t));

    meta_entry->name = (char *) ext_mem_malloc(strlen(dirname)+1);
    if (!meta_entry->name)
        goto fatal_exit;
    memset(meta_entry->name, 0, strlen(dirname)+1);

    meta_entry->is_dir = 1;
    strcpy(meta_entry->name, dirname);
    
    return meta_entry;

fatal_exit:
    if (meta_entry)
    {
        if (meta_entry->name)
            ext_mem_free(meta_entry->name);
        ext_mem_free(meta_entry);
    }

    return NULL;
}

netfs_meta_entry_t *netfs_create_dir_child(netfs_meta_entry_t *parent_directory)
{
    netfs_meta_entry_t  *meta_entry = NULL;
    /* create ".." as first child entry of directory entry */
    meta_entry = (netfs_meta_entry_t *) ext_mem_malloc(sizeof(netfs_meta_entry_t));
    if (!meta_entry)
        goto fatal_exit;
    memset(meta_entry, 0, sizeof(netfs_meta_entry_t));

    meta_entry->name = (char *) ext_mem_malloc(2+1);
    if (!meta_entry->name)
        goto fatal_exit;
    memset(meta_entry->name, 0, 2+1);

    meta_entry->is_dir = 1;
    strcpy(meta_entry->name, "..");

    if (parent_directory)
        meta_entry->child = parent_directory;
    else
        meta_entry->child = &netfs_info.root_entry;

    return meta_entry;

fatal_exit:
    if (meta_entry)
    {
        if (meta_entry->name)
            ext_mem_free(meta_entry->name);
        ext_mem_free(meta_entry);
    }

    return NULL;
}

/**
 * Client uses this interface to create new file (include directory) in the NETFS.
 * 
 */
int netfs_add_file(struct netfs_file_entry *new_file)
{
    netfs_meta_entry_t  *meta_entry = NULL;
    netfs_meta_entry_t  *previous_entry = NULL;
    netfs_meta_entry_t  *parent_directory = NULL;  /* point to the parent entry */
    netfs_meta_entry_t  *current_directory = NULL;
    const char          *path = NULL;
   	int		            dirlen;
   	char	            dirname[NAME_MAX];

    current_directory = &netfs_info.root_entry;
    parent_directory = NULL;

    path = new_file->pathname;
    previous_entry = NULL;
    dirlen = 0;
    while (*path != '\0')
    {
        if (*path == '/')
        {
            if (dirlen > 0)
            {
                dirname[dirlen] = 0;

                /* search dirname in current working directory */
                meta_entry = current_directory->child;
                if (meta_entry)
                {
                    previous_entry = meta_entry;
                    do
                    {
                        if (meta_entry->is_dir && !strcmp(dirname, meta_entry->name))
                            break;
                        previous_entry = meta_entry;
                        meta_entry = meta_entry->next;
                    } while (meta_entry);
                }

                /* insert new dir if not exist */
                if (!meta_entry)
                {
                    meta_entry = netfs_create_dir_entry(dirname);
                    if (!meta_entry)
                        goto fatal_exit;

                    /* we create previous_entry if this is the first entry in this directory */
                    if (!current_directory->child)
                    {
                        previous_entry = netfs_create_dir_child(/*parent_directory*/current_directory);
                        if (!previous_entry)
                            goto fatal_exit;
                        current_directory->child = previous_entry;
                    }

                    previous_entry->next = meta_entry;
					meta_entry->prev = previous_entry;
                }

                parent_directory = current_directory;
                current_directory = meta_entry;

                dirlen = 0;
            }
        }
        else
        {
            dirname[dirlen] = *path;
            dirlen ++;
        }

        path ++;
    }

    if (dirlen > 0)
    {
        dirname[dirlen] = 0;

        /* search filename in current working directory */
        meta_entry = current_directory->child;
        if (meta_entry)
        {
            previous_entry = meta_entry;
            do
            {
                if (!strcmp(dirname, meta_entry->name))
                    break;
                previous_entry = meta_entry;
                meta_entry = meta_entry->next;
            } while (meta_entry);
        }

        if (meta_entry)
        {
            /* error: filename conflict */
            return -1;
        }

        /* insert new file entry */
        media_file_type = 0;
        meta_entry = netfs_create_file_entry(dirname, new_file);
        if (!meta_entry)
            goto fatal_exit;

        /* we create previous_entry if this is the first entry in this directory */
        if (!current_directory->child)
        {
            previous_entry = netfs_create_dir_child(/*parent_directory*/current_directory);
            if (!previous_entry)
                goto fatal_exit;
            current_directory->child = previous_entry;
        }
        previous_entry->next = meta_entry;
		meta_entry->prev = previous_entry;
    }

    return 0;

fatal_exit:
    if (meta_entry)
    {
        if (meta_entry->name)
            ext_mem_free(meta_entry->name);
        ext_mem_free(meta_entry);
    }

    return -1;
}

int netfs_add_forder(struct netfs_file_entry *new_file,unsigned short childcount)
{
    netfs_meta_entry_t  *meta_entry = NULL;
    netfs_meta_entry_t  *previous_entry = NULL;
    netfs_meta_entry_t  *parent_directory = NULL;  /* point to the parent entry */
    netfs_meta_entry_t  *current_directory = NULL;
    const char          *path = NULL;
   	int		            dirlen;
   	char	            dirname[NAME_MAX];

    current_directory = &netfs_info.root_entry;
    parent_directory = NULL;

    path = new_file->pathname;
    previous_entry = NULL;
    dirlen = 0;
    while (*path != '\0')
    {
        if (*path == '/')
        {
            if (dirlen > 0)
            {
                dirname[dirlen] = 0;

                /* search dirname in current working directory */
                meta_entry = current_directory->child;
                if (meta_entry)
                {
                    previous_entry = meta_entry;
                    do
                    {
                        if (meta_entry->is_dir && !strcmp(dirname, meta_entry->name))
                            break;
                        previous_entry = meta_entry;
                        meta_entry = meta_entry->next;
                    } while (meta_entry);
                }

                /* insert new dir if not exist */
                if (!meta_entry)
                {
                    meta_entry = netfs_create_dir_entry(dirname);
                    if (!meta_entry)
                        goto fatal_exita;
					//Kevin Add Set containder:id
					meta_entry->containerid = ext_mem_malloc(strlen(new_file->url)+1);
					if( !meta_entry->containerid )
						goto fatal_exita;
					strcpy(meta_entry->containerid,new_file->url);
					meta_entry->childcount = childcount;
					meta_entry->curchildcount = 0;
					//End Kevin
                    /* we create previous_entry if this is the first entry in this directory */
                    if (!current_directory->child)
                    {
                        previous_entry = netfs_create_dir_child(/*parent_directory*/current_directory);
                        if (!previous_entry)
                            goto fatal_exita;
                        current_directory->child = previous_entry;
                    }

                    previous_entry->next = meta_entry;
					meta_entry->prev = previous_entry;
                }

                parent_directory = current_directory;
                current_directory = meta_entry;

                dirlen = 0;
            }
        }
        else
        {
            dirname[dirlen] = *path;
            dirlen ++;
        }

        path ++;
    }

#if 0
    if (dirlen > 0)
    {
        dirname[dirlen] = 0;

        /* search filename in current working directory */
        meta_entry = current_directory->child;
        if (meta_entry)
        {
            previous_entry = meta_entry;
            do
            {
                if (!strcmp(dirname, meta_entry->name))
                    break;
                previous_entry = meta_entry;
                meta_entry = meta_entry->next;
            } while (meta_entry);
        }

        if (meta_entry)
        {
            /* error: filename conflict */
            return -1;
        }

        /* insert new file entry */
		media_file_type = file_type;
        meta_entry = netfs_create_file_entry(dirname, new_file);
        if (!meta_entry)
            goto fatal_exita;

        /* we create previous_entry if this is the first entry in this directory */
        if (!current_directory->child)
        {
            previous_entry = netfs_create_dir_child(parent_directory);
            if (!previous_entry)
                goto fatal_exita;
            current_directory->child = previous_entry;
        }

        previous_entry->next = meta_entry;
		meta_entry->prev = previous_entry;
    }
#endif
    return 0;

fatal_exita:

    if (meta_entry)
    {
        if (meta_entry->name)
            ext_mem_free(meta_entry->name);
        ext_mem_free(meta_entry);
    }

    return -1;
}

int netfs_add_file_with_type(struct netfs_file_entry *new_file,unsigned char file_type)
{
    netfs_meta_entry_t  *meta_entry = NULL;
    netfs_meta_entry_t  *previous_entry = NULL;
    netfs_meta_entry_t  *parent_directory = NULL;  /* point to the parent entry */
    netfs_meta_entry_t  *current_directory = NULL;
    const char          *path = NULL;
   	int		            dirlen;
   	char	            dirname[NAME_MAX];

    current_directory = &netfs_info.root_entry;
    parent_directory = NULL;

    path = new_file->pathname;
    previous_entry = NULL;
    dirlen = 0;
    while (*path != '\0')
    {
        if (*path == '/')
        {
            if (dirlen > 0)
            {
                dirname[dirlen] = 0;

                /* search dirname in current working directory */
                meta_entry = current_directory->child;
                if (meta_entry)
                {
                    previous_entry = meta_entry;
                    do
                    {
                        if (meta_entry->is_dir && !strcmp(dirname, meta_entry->name))
                            break;
                        previous_entry = meta_entry;
                        meta_entry = meta_entry->next;
                    } while (meta_entry);
                }

                /* insert new dir if not exist */
                if (!meta_entry)
                {
                    meta_entry = netfs_create_dir_entry(dirname);
                    if (!meta_entry)
                        goto fatal_exit;

                    /* we create previous_entry if this is the first entry in this directory */
                    if (!current_directory->child)
                    {
                        previous_entry = netfs_create_dir_child(/*parent_directory*/current_directory);
                        if (!previous_entry)
                            goto fatal_exit;
                        current_directory->child = previous_entry;
                    }

                    previous_entry->next = meta_entry;
					meta_entry->prev = previous_entry;
                }

                parent_directory = current_directory;
                current_directory = meta_entry;

                dirlen = 0;
            }
        }
        else
        {
            dirname[dirlen] = *path;
            dirlen ++;
        }

        path ++;
    }

    if (dirlen > 0)
    {
        dirname[dirlen] = 0;

        /* search filename in current working directory */
        meta_entry = current_directory->child;

        if (meta_entry)
        {
            previous_entry = meta_entry;
            do
            {
#if 0
                if (!strcmp(dirname, meta_entry->name))
                    break;
#endif
                previous_entry = meta_entry;
                meta_entry = meta_entry->next;
            } while (meta_entry);
        }

        if (meta_entry)
        {
            /* error: filename conflict */
            return -1;
        }
        /* insert new file entry */
		media_file_type = file_type;
        meta_entry = netfs_create_file_entry(dirname, new_file);
        if (!meta_entry)
            goto fatal_exit;

        /* we create previous_entry if this is the first entry in this directory */
        if (!current_directory->child)
        {
            previous_entry = netfs_create_dir_child(/*parent_directory*/current_directory);
            if (!previous_entry)
                goto fatal_exit;
            current_directory->child = previous_entry;
        }
        previous_entry->next = meta_entry;
		meta_entry->prev = previous_entry;
		//MP_DEBUG1("meta_entry->prev %x",meta_entry->prev);
    }

    return 0;

fatal_exit:

    if (meta_entry)
    {
        if (meta_entry->name)
            ext_mem_free(meta_entry->name);
        ext_mem_free(meta_entry);
    }

    return -1;
}

#if NETFS_DUMP_TREE
static void netfs_dump_entry(netfs_meta_entry_t *current_directory, int depth)
{
    netfs_meta_entry_t  *meta_entry;
    int tab;

    meta_entry = current_directory;
    while (meta_entry)
    {
        for (tab = 0; tab < depth; tab ++)
            mpDebugPrint("\t");
         mpDebugPrint("'%s' is %s\n", meta_entry->name, meta_entry->is_dir ? "directory" : "regular file");
        if (meta_entry->is_dir)
        {
            if (!strcmp(meta_entry->name, ".."))
                 mpDebugPrint("child name '%s'\n", meta_entry->child->name);
            else
                netfs_dump_entry(meta_entry->child, depth + 1);
        }

        meta_entry = meta_entry->next;
    }
}

void netfs_dump_tree(void)
{
    netfs_dump_entry(&netfs_info.root_entry, 0);
}
#endif
static void netfs_free_entry(netfs_meta_entry_t *current_directory)
{
    netfs_meta_entry_t  *meta_entry;
    netfs_meta_entry_t  *next_entry;

    meta_entry = current_directory;
    while (meta_entry)
    {
        next_entry = meta_entry->next;

        if (meta_entry->is_dir)
        {
            if (strcmp(meta_entry->name, ".."))
                netfs_free_entry(meta_entry->child);
        }
        else
        {
            ext_mem_free(meta_entry->file_entry->url);
            ext_mem_free(meta_entry->file_entry->path);
            ext_mem_free(meta_entry->file_entry->hostname);
            ext_mem_free(meta_entry->file_entry);
        }

        if (meta_entry != &netfs_info.root_entry)
        {
            ext_mem_free(meta_entry->name);
			ext_mem_free(meta_entry->containerid);
            ext_mem_free(meta_entry);
        }
        meta_entry = next_entry;
    }
}

int netfs_reset(void)
{
    netfs_free_entry(&netfs_info.root_entry);
    
    return 0;
}


/*
 * If path is a directory, we return the first entry within this directory.
 * If path is a file, we return the this file's meta entry.
 */
static netfs_meta_entry_t *netfs_search_path(const char *path)
{
    netfs_meta_entry_t  *current_directory;  /* point to the parent entry */
    netfs_meta_entry_t  *meta_entry = NULL;
    int     dirlen;
    char    dirname[NAME_MAX];

    if (!path[0])
        return NULL;

    if (path[0] == '/')
        current_directory = &netfs_info.root_entry;
    else
        current_directory = netfs_info.cwd_entry;
        
    dirlen = 0;
    while (*path != '\0')
    {
        if (*path == '/')
        {
            if (dirlen > 0)
            {
                dirname[dirlen] = 0;

                if (!strcmp(dirname, "."))
                {
                    current_directory = current_directory;
                }
                else
                {
                    /* search dirname in current working directory */
                    meta_entry = current_directory->child;
                    while (meta_entry)
                    {
                        /* this entry must be a directory */
                        if (meta_entry->is_dir && !strcmp(dirname, meta_entry->name))
                            break;
                        meta_entry = meta_entry->next;
                    }

                    if (!meta_entry)
                        break;

                    current_directory = meta_entry;
                }
            }
            dirlen = 0;
        }
        else
        {
            dirname[dirlen] = *path;
            dirlen ++;
        }

        path ++;
    }

    if (dirlen > 0)
    {
        dirname[dirlen] = 0;

        if (!strcmp(dirname, "."))
        {
            meta_entry = current_directory;
        }
        else
        {
            /* search dirname in current working directory */
            meta_entry = current_directory->child;
            while (meta_entry)
            {
                /* this entry can be file or directory */
                if (!strcmp(dirname, meta_entry->name))
                    break;
                meta_entry = meta_entry->next;
            }
        }
    }
    else
    {
        /* we are opening a directory */
        meta_entry = current_directory;
    }

    return meta_entry;
}

static void netfs_content_range(http_info_t *http_info, const char *name, const char *value)
{
	netfs_file_entry_t	*file_entry;
	int		offset1;
	int		offset2;
	int		file_size;

	file_entry = (netfs_file_entry_t *) http_info->client_data;
	if (!file_entry || file_entry->file_length > 0)
	{
		return;
	}

	/* if file length is unknown, we try to get file length from http header */
	offset1		= 0;
	offset2		= 0;
	file_size	= 0;
	if (!strncasecmp(value, "bytes", 5))
	{
		value += 5;
		/* skip space */
		while (*value && *value==' ')
			value ++;

		while (*value && *value!='-')
		{
			offset1 = offset1 * 10 + (*value - '0');
			value ++;
		}
		if (*value == '-')
		{
			value ++;
			while (*value && *value!='/')
			{
				offset2 = offset2 * 10 + (*value - '0');
				value ++;
			}
			if (*value == '/')
			{
                value ++;
				while (*value)
				{
					file_size = file_size * 10 + (*value-'0');
					value ++;
				}
			}
		}
	}

	file_entry->file_length = file_size;
}

#if 1

static int netfs_http_file_buffering(netfs_mount_point_t *mount_point, netfs_file_entry_t *file_entry, int file_pos)
{
    int             readlen = -1;
    int             ret = -1;
    http_info_t     *http_info;     /* allocated in fopen() */
    BYTE         http_request[256];
    int             buffering_length;
#if 0
    http_header_handler_t   last_modified_handler;
    http_header_handler_t   content_range_handler;
    http_header_handler_t   accept_ranges_handler;
    http_header_handler_t   content_length_handler;
    http_header_handler_t   location_handler;
#endif

    BYTE *str;	
    int len ;
    file_entry->bufpos = (file_pos / file_entry->bufsize) * file_entry->bufsize;

    http_info = file_entry->http_info;       

    memset(http_request, 0x00, 256);	

    len = 	strlen(file_entry->url);
    strncpy(http_request,file_entry->url,len);

    
    
    MP_DEBUG1("file_entry->url <= , %s",file_entry->url);

    memset(&photo_info, 0, sizeof(photo_info));	
    if(((str = strstr(http_request,".jpg")) != NULL) || ((str = strstr(http_request,".gif")) != NULL))
    {
	*(str+4) = 0x00; //delete after the http the x y jpg scaler size 
	 MP_DEBUG1("http_request <= , %s",http_request);
	 sprintf(photo_info.length,"%d",0);	
        strcpy(photo_info.pathname,"rss.jpg");
        strcpy(photo_info.url, http_request);
	 Net_Xml_parseFileList(&photo_info);
	 len = strlen(http_request);
    }
    else
	len = 0;
MP_DEBUG("111111111111@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ = %d",len);
    return len; // 	file_entry->url len	
#if 0	
    http_header_handler_t   last_modified_handler;
    http_header_handler_t   content_range_handler;
    http_header_handler_t   accept_ranges_handler;
    http_header_handler_t   content_length_handler;
    http_header_handler_t   location_handler;


    file_entry->bufpos = (file_pos / file_entry->bufsize) * file_entry->bufsize;

    http_info = file_entry->http_info;

start_http:

	
    ret = http_client_init(http_info, file_entry->url, file_entry->http_buffer, file_entry->http_bufsize, &netfs_proxy_info, &netfs_socks_info);
    if (ret < 0)
    {
        readlen = ret;
        goto fatal_exit;
    }

    http_info->client_data = (void *) file_entry;
    if (file_entry->file_length <= 0)
    {
        /* try to get file length from http response */
        content_range_handler.name     = "Content-Range";
        content_range_handler.handler  = netfs_content_range;
        http_client_add_header_handler(http_info, &content_range_handler);
    }

    accept_ranges_handler.name      = "Accept-Ranges";
    accept_ranges_handler.handler   = http_header_accept_ranges;
    http_client_add_header_handler(http_info, &accept_ranges_handler);

    last_modified_handler.name      = "Last-Modified";
    last_modified_handler.handler   = http_header_last_modified;
    http_client_add_header_handler(http_info, &last_modified_handler);

    location_handler.name           = "Location";
    location_handler.handler        = http_header_location;
    http_client_add_header_handler(http_info, &location_handler);

    content_length_handler.name     = "Content-Length";
    content_length_handler.handler  = http_header_content_length;
    http_client_add_header_handler(http_info, &content_length_handler);

    buffering_length = file_entry->bufsize;
    if (file_entry->file_length > 0)
    {
        if (buffering_length + file_entry->bufpos >= file_entry->file_length)
            buffering_length = file_entry->file_length - file_entry->bufpos;
    }

    ret = http_client_request_partial(http_info, file_entry->bufpos, file_entry->bufpos+buffering_length-1);
    if (ret < 0)
    {
        readlen = ret;
        goto fatal_exit;
    }

    ret = http_client_recv_header(http_info);
    if (ret < 0)
    {
        readlen = ret;
        goto fatal_exit;
    }

    if ((http_info->status_code/100) == 3)
    {
        /* 3xx: HTTP redirection response */

        mem_free(file_entry->url);
        if (!strncasecmp("http://", http_info->location, 7))
        {
            file_entry->url = (char *) mem_malloc(strlen(http_info->location)+1);
            if (!file_entry->url)
            {
                readlen = -NETFS_NO_MEMORY;
                goto fatal_exit;
            }
            strcpy(file_entry->url, http_info->location);
        }
        else
        {
            file_entry->url = (char *) mem_malloc(strlen(http_info->location)+128);
            if (!file_entry->url)
            {
                readlen = -NETFS_NO_MEMORY;
                goto fatal_exit;
            }
            if (http_info->port == 80)
                snprintf(file_entry->url, 256, "http://%s%s", http_info->hostname, http_info->location);
            else
                snprintf(file_entry->url, 256, "http://%s:%d%s", http_info->hostname, http_info->port, http_info->location);
        }
        file_entry->file_length = 0;
        
        http_client_exit(file_entry->http_info);

         mpDebugPrint("redirect to '%s'\n", file_entry->url);
        goto start_http;
    }
    else if ((http_info->status_code/100) == 4 || (http_info->status_code/100) == 5)
    {
        readlen = -NETFS_HTTP_ERROR;
        goto fatal_exit;
    }

    if (!http_info->accept_ranges)
    {
        /* this web site don't support range */
        if (file_entry->file_length <= 0)
            file_entry->file_length = http_info->content_length;

        if (file_entry->file_length > file_entry->bufsize)
        {
            readlen = -NETFS_FILE_SIZE_LIMIT;
             mpDebugPrint("size limit\n");
            goto fatal_exit;
        }
//         mpDebugPrint("buffer %d-%d\n", 0, file_entry->file_length-1);
    }
    else
    {
//         mpDebugPrint("buffer %d-%d\n", file_entry->bufpos, file_entry->bufpos+buffering_length-1);
    }
    
    readlen = http_client_recv_data(http_info, file_entry->buffer, file_entry->bufsize);
    if (readlen >= 0)
        file_entry->buflen = readlen;

//     mpDebugPrint("readlen:%d, file_length:%d\n", readlen, file_entry->file_length);
    /* if we can't get file length here, just assume this file length has read completely */
    if (!file_entry->file_length)
        file_entry->file_length = file_entry->buflen;

    if (http_info->modified_time > mount_point->startup_time)
    {
        mount_point->remote_refresh = 1;
    }

    /* 0 means there will no data in future request */
    if (readlen <= 0)
        goto fatal_exit;

buf_exit:

    http_client_exit(file_entry->http_info);

    return readlen;

fatal_exit:
    goto buf_exit;
#endif	
}
#else

static int netfs_http_file_buffering(netfs_mount_point_t *mount_point, netfs_file_entry_t *file_entry, int file_pos)
{
    int             readlen = -1;
    int             ret = -1;
    http_info_t     *http_info;     /* allocated in fopen() */
    char            http_request[256];
    int             buffering_length;
    http_header_handler_t   last_modified_handler;
    http_header_handler_t   content_range_handler;
    http_header_handler_t   accept_ranges_handler;
    http_header_handler_t   content_length_handler;
    http_header_handler_t   location_handler;


    file_entry->bufpos = (file_pos / file_entry->bufsize) * file_entry->bufsize;

    http_info = file_entry->http_info;

start_http:
    ret = http_client_init(http_info, file_entry->url, file_entry->http_buffer, file_entry->http_bufsize, &netfs_proxy_info, &netfs_socks_info);
    if (ret < 0)
    {
        readlen = ret;
        goto fatal_exit;
    }

    http_info->client_data = (void *) file_entry;
    if (file_entry->file_length <= 0)
    {
        /* try to get file length from http response */
        content_range_handler.name     = "Content-Range";
        content_range_handler.handler  = netfs_content_range;
        http_client_add_header_handler(http_info, &content_range_handler);
    }

    accept_ranges_handler.name      = "Accept-Ranges";
    accept_ranges_handler.handler   = http_header_accept_ranges;
    http_client_add_header_handler(http_info, &accept_ranges_handler);

    last_modified_handler.name      = "Last-Modified";
    last_modified_handler.handler   = http_header_last_modified;
    http_client_add_header_handler(http_info, &last_modified_handler);

    location_handler.name           = "Location";
    location_handler.handler        = http_header_location;
    http_client_add_header_handler(http_info, &location_handler);

    content_length_handler.name     = "Content-Length";
    content_length_handler.handler  = http_header_content_length;
    http_client_add_header_handler(http_info, &content_length_handler);

    buffering_length = file_entry->bufsize;
    if (file_entry->file_length > 0)
    {
        if (buffering_length + file_entry->bufpos >= file_entry->file_length)
            buffering_length = file_entry->file_length - file_entry->bufpos;
    }

    ret = http_client_request_partial(http_info, file_entry->bufpos, file_entry->bufpos+buffering_length-1);
    if (ret < 0)
    {
        readlen = ret;
        goto fatal_exit;
    }

    ret = http_client_recv_header(http_info);
    if (ret < 0)
    {
        readlen = ret;
        goto fatal_exit;
    }

    if ((http_info->status_code/100) == 3)
    {
        /* 3xx: HTTP redirection response */

        mem_free(file_entry->url);
        if (!strncasecmp("http://", http_info->location, 7))
        {
            file_entry->url = (char *) mem_malloc(strlen(http_info->location)+1);
            if (!file_entry->url)
            {
                readlen = -NETFS_NO_MEMORY;
                goto fatal_exit;
            }
            strcpy(file_entry->url, http_info->location);
        }
        else
        {
            file_entry->url = (char *) mem_malloc(strlen(http_info->location)+128);
            if (!file_entry->url)
            {
                readlen = -NETFS_NO_MEMORY;
                goto fatal_exit;
            }
            if (http_info->port == 80)
                snprintf(file_entry->url, 256, "http://%s%s", http_info->hostname, http_info->location);
            else
                snprintf(file_entry->url, 256, "http://%s:%d%s", http_info->hostname, http_info->port, http_info->location);
        }
        file_entry->file_length = 0;
        
        http_client_exit(file_entry->http_info);

         mpDebugPrint("redirect to '%s'\n", file_entry->url);
        goto start_http;
    }
    else if ((http_info->status_code/100) == 4 || (http_info->status_code/100) == 5)
    {
        readlen = -NETFS_HTTP_ERROR;
        goto fatal_exit;
    }

    if (!http_info->accept_ranges)
    {
        /* this web site don't support range */
        if (file_entry->file_length <= 0)
            file_entry->file_length = http_info->content_length;

        if (file_entry->file_length > file_entry->bufsize)
        {
            readlen = -NETFS_FILE_SIZE_LIMIT;
             mpDebugPrint("size limit\n");
            goto fatal_exit;
        }
//         mpDebugPrint("buffer %d-%d\n", 0, file_entry->file_length-1);
    }
    else
    {
//         mpDebugPrint("buffer %d-%d\n", file_entry->bufpos, file_entry->bufpos+buffering_length-1);
    }
    
    readlen = http_client_recv_data(http_info, file_entry->buffer, file_entry->bufsize);
    if (readlen >= 0)
        file_entry->buflen = readlen;

//     mpDebugPrint("readlen:%d, file_length:%d\n", readlen, file_entry->file_length);
    /* if we can't get file length here, just assume this file length has read completely */
    if (!file_entry->file_length)
        file_entry->file_length = file_entry->buflen;

    if (http_info->modified_time > mount_point->startup_time)
    {
        mount_point->remote_refresh = 1;
    }

    /* 0 means there will no data in future request */
    if (readlen <= 0)
        goto fatal_exit;

buf_exit:

    http_client_exit(file_entry->http_info);

    return readlen;

fatal_exit:
    goto buf_exit;
}
#endif


static int netfs_mem_file_buffering(netfs_file_entry_t *file_entry, int file_pos)
{
    url_info_t  url_info;
    char        *src_buf;
    int         src_buflen;
    int         buffering_length;
    
    file_entry->bufpos = (file_pos / file_entry->bufsize) * file_entry->bufsize;

    utils_decode_url(file_entry->url, &url_info);
    src_buf = (char *) strtoul(url_info.host_s, NULL, 16);
    src_buflen = strtol(url_info.port_s, NULL, 16);
    
    /* HERE, src_buflen must equal to file_entry->file_length */
    buffering_length = file_entry->bufsize;
    if (buffering_length + file_entry->bufpos >= file_entry->file_length)
        buffering_length = file_entry->file_length - file_entry->bufpos;
    
		memcpy(file_entry->buffer, src_buf+file_entry->bufpos, buffering_length);

    file_entry->buflen  = buffering_length;
    
    return buffering_length;
}

static int netfs_file_buffering(NETFS_FILE *stream)
{
    int         readlen;
    netfs_file_entry_t  *file_entry;
    netfs_mount_point_t *mount_point;
    int         retry;

    mount_point = stream->mount_point;
    file_entry = stream->file_entry;
    if (file_entry->bufpos <= stream->curpos && stream->curpos < file_entry->bufpos+file_entry->buflen)
        return NETFS_OK;

    readlen = -NETFS_PROTOCOL_NOT_SUPPORT;
    //for (retry = 0; retry < 3; retry ++)
    //{
        if (!strncasecmp(file_entry->url, "http://", 7))
        {
        	//MP_DEBUG("netfs_file_buffering < ======  http://");
              readlen = netfs_http_file_buffering(mount_point, file_entry, stream->curpos);
        }
	 else if (!strncasecmp(file_entry->url, "mem://", 6))
	 {
	 	//MP_DEBUG("netfs_file_buffering < ====== mem://");
	 	readlen = netfs_mem_file_buffering(file_entry, stream->curpos);
	 }
	 else
	 {
	 	//MP_DEBUG("netfs_file_buffering < ====== Nono ");
		goto fatal_exit;
	 }
        /* 0 means there will no data in future request */
        //if (readlen <= 0)
            //continue;

/*   CJ 100907
	if (file_entry->bufpos > stream->curpos || stream->curpos >= file_entry->bufpos+file_entry->buflen)
            continue;
*/
        //break;
    //}

    if (mount_point->remote_refresh)
    {
        /* TODO: refresh file system */
         mpDebugPrint("remote filesystem update1111111!!\n");
    }

    if (readlen <= 0)
        goto fatal_exit;
/*
    if (file_entry->bufpos > stream->curpos || stream->curpos >= file_entry->bufpos+file_entry->buflen)
    {
    	mpDebugPrint("remote filesystem update222222222!!\n");
        readlen = -NETFS_ERROR;
        goto fatal_exit;
    }
*/
buf_exit:
    return readlen;

fatal_exit:
    goto buf_exit;
}

int netfs_init(void)
{
     int	ret;	
    memset(&netfs_info, 0, sizeof(netfs_info_t));
    netfs_info.root_entry.is_dir    = 1;
    netfs_info.root_entry.name      = "netfs_root";

    netfs_info.cwd_entry        = &netfs_info.root_entry;
    netfs_info.cwd_mount_point  = &netfs_info.mount_root;
    netfs_info.mount_root.next  = &netfs_info.mount_root;
    netfs_info.mount_root.prev  = &netfs_info.mount_root;
    return NETFS_OK;
}

void netfs_exit(void)
{
    //reset_service_list();
    netfs_reset();
}

#if 0
int netfs_config_proxy(const char *hostname, int port, char *username, char *password)
{
    memset(&netfs_proxy_info, 0, sizeof(proxy_info_t));
    
    if (!hostname || !hostname[0])
        return -NETFS_ERROR;

    strncat(netfs_proxy_info.hostname, hostname, 63);
    netfs_proxy_info.port = port;
    if (username)
        strncat(netfs_proxy_info.username, username, 31);
    if (password)
        strncat(netfs_proxy_info.password, password, 31);

    return NETFS_OK;
}

int netfs_config_socks(int socks_ver, const char *hostname, char *username, char *password)
{
    memset(&netfs_socks_info, 0, sizeof(socks_info_t));
    
    if (!hostname || !hostname[0])
        return -NETFS_ERROR;

    strncat(netfs_socks_info.hostname, hostname, 63);
    netfs_socks_info.version = socks_ver;
    if (username)
        strncat(netfs_socks_info.username, username, 31);
    if (password)
        strncat(netfs_socks_info.password, password, 31);

    return -NETFS_ERROR;
}
#endif

typedef struct
{
	enum _NETFS_TYPE type;
	char * pbMountStr;
	
	int (*mf_init)(const char *, const char *);
	void (*mf_exit)(const char *);
} ST_MOUNT_INFO;

#if NET_UPNP
void upnp_init(const char * url, const char * base_dir)
    {
	upnp_fs_init(base_dir);
    }
#endif

// ???  shutterfly ??? 

ST_MOUNT_INFO g_mountArry[]=
    {
	{NETFS_MPX_LIST			, "/smb"			,magicpixel_init		, magicpixel_exit}
	,{NETFS_RSS				, "/rss"			, rss_init				, rss_exit}
#if HAVE_FLICKR
	#if HAVE_FACEBOOK
	,{NETFS_FLICKR			, "/facebook"			, facebook_init			, facebook_exit}
	#else
	,{NETFS_FLICKR			, "/flickr"		, flickr_init		, flickr_exit}
	#endif
#endif
#if Make_CURL
	,{NETFS_PICASA			, "/picasa"		, picasa_init			, picasa_exit}
#endif
#if YOUGOTPHOTO
	,{NETFS_YOUGOTPHOTO	, "/ygp"			, ygp_init			, ygp_exit}
#endif	
#if HAVE_VTUNER
	,{NETFS_VTUNER			, "/vtuner"	 	, vtuner_init			, vtuner_exit}
#endif
#ifdef HAVE_SHOUTCAST
	,{NETFS_SHOUTCAST		, "/shoutcast" 	, shoutcast_init		, shoutcast_exit}
#endif
#if HAVE_FRAMECHANNEL
	,{NETFS_FRAMECHANNEL	, "/FrameChannel"	, framechannel_init	, framechannel_exit}
#endif	
#if HAVE_FRAMEIT
	,{NETFS_FRAMEIT			, "/frameit"		, frameit_init			, frameit_exit}
#endif
#if HAVE_SNAPFISH
	,{NETFS_SNAPFISH			, "/Snapfish"		, snapfish_init		, snapfish_exit}
#endif
#if NET_UPNP
	,{NETFS_UPNP_LIST		, "/Upnp"			, upnp_init			, NULL}
#endif
};

static ST_MOUNT_INFO * g_pstMount_Info = NULL;  
char * getMountStr()
	{
	if (! g_pstMount_Info)
		return NULL;

	return g_pstMount_Info->pbMountStr;
	}
		
ST_MOUNT_INFO * Mount_Info_Find(enum _NETFS_TYPE type)
	{
	int i;
	DWORD count = sizeof(g_mountArry) / sizeof(ST_MOUNT_INFO);
	ST_MOUNT_INFO * pInfo = g_mountArry;
        
	for (i = 0; i < count; i++)
    {
		if (pInfo->type == type)
			return pInfo;	
		pInfo++;
    }
	return NULL;
    }   

int netfs_mount(const char *url, const char *base_dir, netfs_type_t type)
    {
    int ret = NETFS_OK;
    netfs_mount_point_t *point;

    ST_MOUNT_INFO * pInfo = Mount_Info_Find(type);

    if (! pInfo)
		return -NETFS_ERROR;

    char * mount_str = base_dir;
    if (! mount_str)
	    mount_str = pInfo->pbMountStr;	 	    
    
    point = netfs_add_mount_point(mount_str);
    if (!point)
	{
        return -NETFS_NO_MEMORY;
	}
        
    point->type = type;
    netfs_info.type = type;

   g_pstMount_Info = pInfo;
   
   if (pInfo->mf_init)
	{
	  ret = pInfo->mf_init(url, mount_str);
        if (ret < 0)
            return ret;
	}

    return NETFS_OK;
}

int netfs_umount(const char *base_dir)
{
    netfs_mount_point_t *point;
    netfs_meta_entry_t  *meta_entry;

    point = netfs_search_mount_point(base_dir);
    if (!point)
    {
        return -NETFS_ERROR;
    }


#if 0 // CJ modify 031809
    meta_entry = netfs_search_path(base_dir);
    if (!meta_entry)
        return -1;
    netfs_free_entry(meta_entry->child->next);
    meta_entry->child->next = NULL;
#else
    meta_entry = netfs_search_path(base_dir);
    if (meta_entry)
    {
    	netfs_free_entry(meta_entry->child->next);
    	meta_entry->child->next = NULL;
    }
#endif

    /* TODO: Do we need to free meta-entry itself ? */
    ST_MOUNT_INFO * pInfo = Mount_Info_Find(point->type);

    if (pInfo)
    {
	   	if (pInfo->mf_exit)
    		{
			pInfo->mf_exit(base_dir);
    		}	
    }	

    netfs_delete_mount_point(base_dir);

    return NETFS_OK;
}

/*
DESCRIPTION
       The  opendir()  function  opens a directory stream corresponding to the
       directory name, and returns a pointer to  the  directory  stream.   The
       stream is positioned at the first entry in the directory.

RETURN VALUE
       The  opendir()  function returns a pointer to the directory stream.  On
       error, NULL is returned, and errno is set appropriately.
 */
NETFS_DIR *netfs_opendir(const char *path)
{
    netfs_meta_entry_t  *meta_entry;
    netfs_dir_stream_t  *dir_stream = NULL;

    meta_entry = netfs_search_path(path);
    if (!meta_entry || !meta_entry->is_dir)
        return NULL;

    dir_stream = (netfs_dir_stream_t  *)ext_mem_malloc(sizeof(netfs_dir_stream_t));
    if (dir_stream)
    {
        memset(dir_stream, 0, sizeof(netfs_dir_stream_t));

        dir_stream->pos             = 0;
        dir_stream->first_entry		= meta_entry->child;
        dir_stream->current_entry	= meta_entry->child;
    }

    return dir_stream;
}

/*
DESCRIPTION
       The  readdir()  function returns a pointer to a dirent structure repre-
       senting the next directory entry in the directory stream pointed to  by
       dir.   It  returns  NULL  on  reaching  the  end-of-file or if an error
       occurred.

       On Linux, the dirent structure is defined as follows:

          struct dirent {
              ino_t          d_ino;       // inode number
              off_t          d_off;       // offset to the next dirent
              unsigned short d_reclen;    // length of this record
              unsigned char  d_type;      // type of file
              char           d_name[256]; // filename
          };

       According to POSIX, the dirent structure contains a field char d_name[]
       of  unspecified  size,  with  at most NAME_MAX characters preceding the
       terminating null character.  POSIX 1003.1-2001 also documents the field
       ino_t  d_ino  as  an  XSI extension.  Use of other fields will harm the
       portability of your programs.

       The data returned by readdir() may be overwritten by  subsequent  calls
       to readdir() for the same directory stream.

RETURN VALUE
       The readdir() function returns a pointer to a dirent structure, or NULL
       if an error occurs or end-of-file is reached.  On error, errno  is  set
       appropriately.
 */
struct dirent *netfs_readdir(NETFS_DIR *dir)
{
    if (!dir->current_entry)
        return NULL;

    if (dir->pos == 0)
    {
        snprintf(netfs_info.dirent.d_name, 256, ".");
    }
    else
    {
        snprintf(netfs_info.dirent.d_name, 256, dir->current_entry->name);

        dir->current_entry = dir->current_entry->next;
    }
    
    dir->pos ++;
    
    return &netfs_info.dirent;
}

/*
DESCRIPTION
       The  closedir()  function  closes  the directory stream associated with
       dir.  The directory stream descriptor dir is not available  after  this
       call.

RETURN VALUE
       The  closedir()  function  returns  0  on  success.   On  error,  -1 is
       returned, and errno is set appropriately.
 */
int netfs_closedir(NETFS_DIR *dir)
{
    /* always success */
    ext_mem_free(dir);
    //fflush(NULL);

    return 0;
}

/*
       stat() stats the file pointed to by path and fills in buf.
 */
#if 1 
int netfs_stat(const char *path, struct stat *stat_buf)
{
    netfs_meta_entry_t  *meta_entry;

    meta_entry = netfs_search_path(path);
    if (!meta_entry)
        return -1;

    memset(stat_buf, 0, sizeof(struct stat));
    if (meta_entry->is_dir)
        stat_buf->st_mode |= S_IFDIR;
    else
        stat_buf->st_mode |= S_IFREG;

    /* support read only */
    stat_buf->st_mode |= S_IRUSR|S_IRGRP|S_IROTH;

    return 0;
}
#endif
/*
DESCRIPTION
       The fopen() function opens the file whose name is the string pointed to
       by path and associates a stream with it.

RETURN VALUE
       Upon  successful  completion  fopen(),  fdopen() and freopen() return a
       FILE pointer.  Otherwise, NULL is  returned  and  the  global  variable
       errno is set to indicate the error.

 *******************************************************************************
  netfs has built-in buffers: file buffer is 16KB, http buffer is 8KB.
  The most optimize buffer size: file buffer is 128KB, http buffer is 64KB.
 *******************************************************************************
 */
 
NETFS_FILE *netfs_fopen(const char *path, const char *mode, netfs_buffer_t *netfs_buffer)
{
    netfs_file_entry_t  *file_entry;
    netfs_meta_entry_t  *meta_entry;
    NETFS_FILE          *stream;

    /* support read only */
    while (mode && *mode)
    {
        char ch = *mode;
        if (ch == '+')
            return NULL;
        else if (ch == 'w')
            return NULL;
        else if (ch == 'a')
            return NULL;

        mode ++;
    }
   MP_DEBUG1("fopen(%s)\n", path);

    meta_entry = netfs_search_path(path);
    if (!meta_entry || meta_entry->is_dir)
     {
              //MP_DEBUG("meta_entry<<<<<<<<<");
		return NULL;

	}
    /* initial stream */
    stream = (netfs_file_stream_t *) ext_mem_malloc(sizeof(netfs_file_stream_t));
    if (!stream)
	{
	//MP_DEBUG("stream <<<<<<<<<");
        return NULL;
    	}
    memset(stream, 0, sizeof(NETFS_FILE));
    stream->file_entry  = meta_entry->file_entry;
    //MP_DEBUG1("meta_entry->file_entry->url %s", meta_entry->file_entry->url  );
	
    stream->mount_point = netfs_search_mount_point(path);
    stream->curpos      = 0;

    file_entry = stream->file_entry;

    file_entry->open_count ++;
    if (file_entry->open_count == 1)
    {
         //MP_DEBUG("initialize net file\n");
        /* initial file_entry */

        if (netfs_buffer)
        {
           //MP_DEBUG("initialize net file <= netfs_buffer yes");
            file_entry->bufsize         = netfs_buffer->file_bufsize;
            file_entry->buffer          = netfs_buffer->file_buffer;
            file_entry->http_bufsize    = netfs_buffer->http_bufsize;
            file_entry->http_buffer     = netfs_buffer->http_buffer;
            file_entry->buflen          = 0;
            file_entry->bufpos          = 0;
        }
        else
        {
            //MP_DEBUG("initialize net file <= netfs_buffer no");
            file_entry->bufsize         = FILE_BUFFER_SIZE;
            file_entry->buffer          = netfs_info.file_buffer;
            file_entry->http_bufsize    = HTTP_BUFFER_SIZE;
            file_entry->http_buffer     = netfs_info.http_buffer;
            file_entry->buflen          = 0;
            file_entry->bufpos          = 0;
        }

        file_entry->http_info   = (http_info_t *) ext_mem_malloc(sizeof(http_info_t));

        if (!file_entry->buffer || !file_entry->http_info)
        {
            //MP_DEBUG("11");	
            netfs_fclose(stream);
            return NULL;
        }

        if (netfs_file_buffering(stream) < 0)
        {
            //MP_DEBUG("22");
            netfs_fclose(stream);
            return NULL;
        }
        
    }

    return stream;
}

/*
 */
void netfs_fsetbuf(NETFS_FILE *stream, netfs_buffer_t *netfs_buffer)
{
    netfs_file_entry_t  *file_entry;

    if (!stream)
        return;
    if (!netfs_buffer)
        return;

    file_entry = stream->file_entry;

    if (!file_entry->open_count)
        return;

    file_entry->bufsize         = netfs_buffer->file_bufsize;
    file_entry->buffer          = netfs_buffer->file_buffer;
    file_entry->http_bufsize    = netfs_buffer->http_bufsize;
    file_entry->http_buffer     = netfs_buffer->http_buffer;
    file_entry->buflen          = 0;
    file_entry->bufpos          = 0;

    if (netfs_file_buffering(stream) < 0)
    {
        netfs_fclose(stream);
        return;
    }
}

/*
DESCRIPTION
       The  fclose()  function will flush the stream pointed to by fp (writing
       any buffered output data using fflush(3)) and close the underlying file
       descriptor.

RETURN VALUE
       Upon  successful  completion 0 is returned.  Otherwise, EOF is returned
       and the global variable errno is set to indicate the error.  In  either
       case  any  further  access  (including another call to fclose()) to the
       stream results in undefined behaviour.
 */
int netfs_fclose(NETFS_FILE *fp)
{
    netfs_file_entry_t  *file_entry;

    if (!fp)
        return EOF;
    file_entry = fp->file_entry;
    if (!file_entry->open_count)
        return EOF;

    file_entry->open_count --;
    if (!file_entry->open_count)
    {
        if (file_entry->http_info)
        {
            ext_mem_free(file_entry->http_info);
            file_entry->http_info = NULL;
        }
//        if (file_entry->buffer)
//        {
//            mem_free(file_entry->buffer);
//            file_entry->buffer = NULL;
//        }
    }

    ext_mem_free(fp);

    return 0;
}

/*
DESCRIPTION
       The function fread() reads nmemb elements  of  data,  each  size  bytes
       long,  from  the stream pointed to by stream, storing them at the loca-
       tion given by ptr.

RETURN VALUE
       fread() and fwrite() return the number of items  successfully  read  or
       written  (i.e.,  not the number of characters).  If an error occurs, or
       the end-of-file is reached, the return value is a short item count  (or
       zero).

       fread() does not distinguish between end-of-file and error, and callers
       must use feof(3) and ferror(3) to determine which occurred.

 */
size_t netfs_fread(void *ptr, size_t size, size_t nmemb, NETFS_FILE *stream)
{
    netfs_file_entry_t  *file_entry;
    int     length;
    int     offset;
    unsigned char   *dstbuf = (unsigned char *)ptr;
    int     data_length;
    int     readlen;

    if (!stream)
        return 0;
    if (!stream->file_entry->open_count)
        return 0;

    /* netfs try to get file_length at initial stage or file open stage */
    if (stream->file_entry->file_length <= 0)
    	return 0;

    file_entry = stream->file_entry;
    length = size * nmemb;

    if (stream->curpos >= file_entry->file_length)
   	    return 0;

    if (stream->curpos + length > file_entry->file_length)
   	    length = file_entry->file_length - stream->curpos;

    readlen = 0;
     //mpDebugPrint("file length: %d, curpos:%d, length:%d\n", file_entry->file_length, stream->curpos, length);
    offset = stream->curpos % file_entry->bufsize;
    while (length > 0)
    {
        if (netfs_file_buffering(stream) < 0)
            break;

        data_length = file_entry->buflen - offset;
        if (data_length > length)
            data_length = length;
         //mpDebugPrint("data length:%d, length:%d, buflen:%d\n", data_length, length, file_entry->buflen);
		 	memcpy(dstbuf, file_entry->buffer + offset, data_length);

        offset      = 0;
        length      -= data_length;
        readlen     += data_length;
        dstbuf      += data_length;
        stream->curpos  += data_length;
    }

    return readlen / size;
}

/*
DESCRIPTION
       The function fwrite() writes nmemb elements of data,  each  size  bytes
       long, to the stream pointed to by stream, obtaining them from the loca-
       tion given by ptr.

RETURN VALUE
       fread() and fwrite() return the number of items  successfully  read  or
       written  (i.e.,  not the number of characters).  If an error occurs, or
       the end-of-file is reached, the return value is a short item count  (or
       zero).
 */
size_t netfs_fwrite(const void *ptr, size_t size, size_t nmemb, NETFS_FILE *stream)
{
    /* not support */
    return 0;
}

/*
DESCRIPTION
       The ftell() function obtains the current value  of  the  file  position
       indicator for the stream pointed to by stream.

RETURN VALUE
       The  rewind()  function  returns no value.  Upon successful completion,
       fgetpos(), fseek(), fsetpos() return 0, and ftell() returns the current
       offset.  Otherwise, -1 is returned and the global variable errno is set
       to indicate the error.
 */
long netfs_ftell(NETFS_FILE *stream)
{
    if (!stream)
        return -1;
    if (!stream->file_entry->open_count)
        return -1;
    /* netfs try to get file_length at initial stage or file open stage */
    if (stream->file_entry->file_length <= 0)
    	return -1;


    return stream->curpos;
}

/*
DESCRIPTION
       The  fseek()  function  sets the file position indicator for the stream
       pointed to by stream.  The new position, measured in bytes, is obtained
       by  adding offset bytes to the position specified by whence.  If whence
       is set to SEEK_SET, SEEK_CUR, or SEEK_END, the offset  is  relative  to
       the  start of the file, the current position indicator, or end-of-file,
       respectively.  A successful call to the  fseek()  function  clears  the
       end-of-file  indicator  for  the  stream  and undoes any effects of the
       ungetc(3) function on the same stream.

RETURN VALUE
       The  rewind()  function  returns no value.  Upon successful completion,
       fgetpos(), fseek(), fsetpos() return 0, and ftell() returns the current
       offset.  Otherwise, -1 is returned and the global variable errno is set
       to indicate the error.
 */
int netfs_fseek(NETFS_FILE *stream, long offset, int whence)
{
    netfs_file_entry_t  *file_entry;

    if (!stream)
        return -1;
    if (!stream->file_entry->open_count)
        return -1;

    /* netfs try to get file_length at initial stage or file open stage */
    file_entry = stream->file_entry;
    if (file_entry->file_length <= 0)
    	return -1;

    if (whence == SEEK_CUR)
        offset = stream->curpos + offset;
    else if (whence == SEEK_END)
        offset = file_entry->file_length + offset;

    if (offset > file_entry->file_length)
        return -1;

    stream->curpos = offset;

    return 0;
}

/*
DESCRIPTION
       fgetc() reads the next character from  stream  and  returns  it  as  an
       unsigned char cast to an int, or EOF on end of file or error.

RETURN VALUE
       fgetc(),  getc() and getchar() return the character read as an unsigned
       char cast to an int or EOF on end of file or error.
 */
int netfs_fgetc(NETFS_FILE *stream)
{
    netfs_file_entry_t  *file_entry;
    int     offset;
    int     ch;

    if (!stream)
        return EOF;
    if (!stream->file_entry->open_count)
        return EOF;
    /* netfs try to get file_length at initial stage or file open stage */
    if (stream->file_entry->file_length <= 0)
    	return 0;


    file_entry = stream->file_entry;
    if (stream->curpos >= file_entry->file_length)
        return EOF;

    if (netfs_file_buffering(stream) < 0)
        return EOF;

    offset = stream->curpos % file_entry->buflen;
    ch = file_entry->buffer[offset];

    stream->curpos ++;

    return ch;
}
#endif
