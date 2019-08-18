#ifndef _NETFS_H_
#define _NETFS_H_

#include <sys/types.h>
//#include <sys/stat.h>
//#/include <dirent.h>
//#include <unistd.h>

#define S_IFMT  00170000
#define S_IFSOCK 0140000
#define S_IFLNK	 0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000
#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)
#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100
#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010
#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#ifndef NAME_MAX
#define NAME_MAX 256
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* MagicPixel Networking Service Discovery */
typedef struct service_entry    service_entry_t;

struct service_entry
{
    char name_str[32];
    char ip_str[16];
    char url_str[64];

    struct service_entry *next;
    struct service_entry *prev;
};

service_entry_t *do_service_discovery(void);


/* stream-base POSIX-like external API */
typedef enum _NETFS_TYPE	netfs_type_t;

typedef struct _netfs_dir_stream    NETFS_DIR;
typedef struct _netfs_file_stream   NETFS_FILE;

struct stat {
	unsigned int st_dev;
	unsigned long st_ino;
	unsigned int st_mode;
	unsigned short st_nlink;
	unsigned int st_uid;
	unsigned int st_gid;
	short st_rdev;
	long st_size;
	long int st_atime;
	long int st_mtime;
	long int st_ctime;
	long st_blksize;
	long st_blocks;
	unsigned int st_attr;
}; 


enum _NETFS_TYPE
{
    	NETFS_MPX_LIST = 0,        /* MagicPixel In-house File List */
	NETFS_RSS,                        /* RSS / NEWS */	
	NETFS_PHOTOCASTING,	 /* Apple iPhoto Photocasting */
	NETFS_FLICKR,			 /* Flickr */
    	NETFS_FLICKR_7DAYS,      /* Flickr Interesting Last 7 Days */
	NETFS_PICASA,			 /* Google Picasa */
    	NETFS_PHOTOZOU,             /* Japan PhotoZou */
	NETFS_YAHOO,
	NETFS_GOOGLE_VIDEO,		/* Google Video Sharing */
	NETFS_YOUTUBE,			/* Youtube Video Sharing */
	NETFS_WRETCH,			/* Taiwan Wretch */
	NETFS_PIXNET,			/* Taiwan Pixnet */
	NETFS_GCE,                      /*GOOGLE  Consumer Electronics*/
	NETFS_PC,
	NETFS_YOUGOTPHOTO,              /* YouGotPhoto.com */
	NETFS_SHOUTCAST,				/* SHOUTCAST */
	NETFS_SHOUTCAST_PLS,			/* text file for iRadio Play list */
	NETFS_FRAMECHANNEL,
	NETFS_VTUNER,
	NETFS_UPNP_LIST,
	NETFS_SHUTTERFLY,
	NETFS_FRAMEIT,
	NETFS_SNAPFISH,
};

enum NETFS_STATUS
{
    NETFS_OK        = 0,
    NETFS_ERROR,
    NETFS_SOCKET_FAIL,
    NETFS_PROTOCOL_NOT_SUPPORT,
    NETFS_PROTOCOL_FAIL,
    NETFS_HOST_NOT_FOUND,
    NETFS_CONNECT_FAIL,
    NETFS_SEND_FAIL,
    NETFS_RECV_FAIL,
    NETFS_NO_MEMORY,
    NETFS_FILE_SIZE_LIMIT,
    NETFS_HTTP_ERROR,
    NETFS_APP_ERROR,
};

/* Suggested buffer size: file buffer size >= 128KB, http buffer size >= 64KB */
typedef struct _netfs_buffer    netfs_buffer_t;
struct _netfs_buffer
{
    unsigned char   *http_buffer;
    int             http_bufsize;
    unsigned char   *file_buffer;
    int             file_bufsize;
};


int netfs_init(void);
void netfs_exit(void);
int netfs_config_proxy(const char *hostname, int port, char *username, char *password);
int netfs_config_socks(int socks_ver, const char *hostname, char *username, char *password);

int netfs_mount(const char *rss_url, const char *base_dir, netfs_type_t type);
int netfs_umount(const char *base_dir);
NETFS_DIR *netfs_opendir(const char *name);
struct dirent *netfs_readdir(NETFS_DIR *dir);
int netfs_closedir(NETFS_DIR *dir);
int netfs_stat(const char *path, struct stat *buf);
NETFS_FILE *netfs_fopen(const char *path, const char *mode, netfs_buffer_t *netfs_buffer);
void netfs_fsetbuf(NETFS_FILE *stream, netfs_buffer_t *netfs_buffer);
int netfs_fclose(NETFS_FILE *fp);
size_t netfs_fread(void *ptr, size_t size, size_t nmemb, NETFS_FILE *stream);
size_t netfs_fwrite(const void *ptr, size_t size, size_t nmemb, NETFS_FILE *stream);
long netfs_ftell(NETFS_FILE *stream);
int netfs_fseek(NETFS_FILE *stream, long offset, int whence);
int netfs_fgetc(NETFS_FILE *stream);
void netfs_dump_tree(void);


/**
 * Weather Forecast Information
 */

enum _WEATHER_CONDITION
{
    WEATHER_SUNNY,
    WEATHER_CLOUDY,
    WEATHER_AM_SHOWERS,
    WEATHER_PM_SHOWERS,
    WEATHER_PARTLY_CLOUDY,
    WEATHER_MOSTLY_CLOUDY,
    WEATHER_THUNDERSTORMS,
    WEATHER_SCATTERED_THUNDERSTORMS,
    WEATHER_SCATTERED_SHOWERS,
    WEATHER_MOSTLY_CLEAR,
    WEATHER_HEAVY_THUNDERSTORMS,
};


typedef struct _weather_current_info    weather_current_info_t;
struct _weather_current_info
{
    int date;               /* 15-8:hour, 7-0:minute, 10:30 am = 1030 */
    int condition;          /* enum _CONDITION */
    int temperature;        /* C, Celsius */
};


typedef struct _weather_wind_info       weather_wind_info_t;
struct _weather_wind_info
{
    int chill;              /* feeling of cold, http://en.wikipedia.org/wiki/Wind_chill */
    int direction;
    int speed;              /* kilometer per hour */
};


typedef struct _weather_atmosphere_info weather_atmosphere_info_t;
struct _weather_atmosphere_info
{
    int humidity;
    int visibility;         /* kilometer */
};

typedef struct _weather_astronomy_info  weather_astronomy_info_t;
struct _weather_astronomy_info
{
    int sunrise;            /* 15-8:hour, 7-0:minute, 5:14 am = 0514 */
    int sunset;             /* 15-8:hour, 7-0:minute, 6:39 pm = 1839 */
};

typedef struct _weather_forecast_info   weather_forecast_info_t;
struct _weather_forecast_info
{
    int low_temperature;    /* C, Celsius */
    int high_temperature;   /* C, Celsius */
    int condition;          /* enum _CONDITION */
};

int yahoo_weather_init(const char *regional_code);
int yahoo_weather_today_atmosphere(weather_atmosphere_info_t *info);
int yahoo_weather_today_astronomy(weather_astronomy_info_t *info);
int yahoo_weather_today_wind(weather_wind_info_t *info);
int yahoo_weather_current_info(weather_current_info_t *info);
int yahoo_weather_forecast(int day, weather_forecast_info_t *info);



int gce_button_center(void);
int gce_button_up(void);
int gce_button_down(void);
int gce_button_left(void);
int gce_button_right(void);
int gce_get_refresh_interval(void);
int gce_get_picture(char *buf);
int gce_init(const char *url, const char *base_dir);
void gce_exit(const char *base_dir);


#endif /* _NETFS_H_ */
