/*
********************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
*
* $Id$
* $HeadURL$
*
* Filename      : file.c
* Programmer(s) : 
* Created       : 
* Descriptions  : File system support for the Web Server. This is the wrapper
*                 of Magic Pixel's file system APIs to standard ANSI C APIs.
*
********************************************************************************
*/
#define LOCAL_DEBUG_ENABLE 1  /* enable debug messages             */

#include <string.h>           /* for string manupliation           */
#include <fcntl.h>           /* for string manupliation           */
#include "iplaysysconfig.h"   /* for drive ID definitions          */
#include "mpTrace.h"          /* for debug messages                */
#include "fs.h"               /* for MagicPixel's file system APIs */
#include "config.h"           /* for web-server-dependent features */
#include "unixfs.h"           /* for 'struct stat'                 */


#define ARRAY_LENGTH(_x_)  (sizeof(_x_)/sizeof(_x_[0]))


static const struct drive_map_s {
    char *name;
    int   id;
}
drive_map[] = {
    {"NULL_DEVICE",     NULL_DEVICE    },
    {"USB_HOST_ID1",    USB_HOST_ID1   },
    {"USB_HOST_ID2",    USB_HOST_ID2   },
    {"USB_HOST_ID3",    USB_HOST_ID3   },
    {"USB_HOST_ID4",    USB_HOST_ID4   },
    {"USB_HOST_PTP",    USB_HOST_PTP   },
    {"NAND",            NAND           },
    {"SM",              SM             },
    {"XD",              XD             },
    {"MS",              MS             },
    {"SD_MMC",          SD_MMC         },
    {"CF",              CF             },
    {"NAND_B",          NAND_B         },
#if HD_ENABLE
    {"HD",              HD             },
    {"HD2",             HD2            },
    {"HD3",             HD3            },
    {"HD4",             HD4            },
#endif
    {"SDIO",            SDIO           },
    {"USB_RTL8711",     USB_RTL8711    },
    {"USB_RT73",        USB_RT73       },
    {"USB_AR2524",      USB_AR2524     },
    {"USB_PPP",         USB_PPP        },
    {"USB_WIFI_DEVICE", USB_WIFI_DEVICE},
    {"CF_ETHERNET_DEVICE", CF_ETHERNET_DEVICE},
};


/* Arguments:
 *     drive_name -- (I) drive name
 * 
 * Return:
 *     drive ID (as defined in iplayconfig.h)
 */
static int
get_drive_id(const char *drive_name)
{
    unsigned int i;

    for (i = 0; i < ARRAY_LENGTH(drive_map); i++) {
        if (strcmp(drive_name, drive_map[i].name) == 0)
            return drive_map[i].id;
    }

    MP_DEBUG("[UNIXFS] WARNING: unknown drive name! (%s)(a file name?)\n", drive_name);
    return -1;
}


/* Arguments:
 *     pathname -- (I) path name, (O) file name
 * 
 * Return:
 *     current drive ID or -1 (error encountered)
 */
static int
locate_file(char **pathname)
{
    char  *token;
    int    curid;
    DRIVE *drive;

    /* walk through the entire path to locate a directory */
    token = strtok(*pathname, "/");
    do {
        /* if the token is a drive name... */
        {
            if ((curid = get_drive_id(token)) >= 0) {
                drive = DriveChange(curid);
                if ((drive->StatusCode != FS_SUCCEED) || !drive->Flag.Present) {
                    Mcard_DeviceInit(curid);
                    if (DriveAdd(curid) == FS_SUCCEED) {
                        MP_DEBUG("[UNIXFS] ERROR: add drive failed!\n");
                        return -1;
                    }
                }
                DirReset(drive);
                continue;
            } else {
                curid = DriveCurIdGet();
                drive = DriveGet(curid);
            }
        }

        /* if the token is a directory... */
        {
            if (FileSearch(drive, token, "   ", E_BOTH_FILE_AND_DIR_TYPE) == FS_SUCCEED) {
                if (drive->Node->Attribute & FDB_SUB_DIR) {
                    if (CdSub(drive) != FS_SUCCEED) {
                        MP_DEBUG("[UNIXFS] ERROR: cannot change to sub directory!\n");
                        return -1;
                    }
                    MP_DEBUG("[UNIXFS] DEBUG: directory found!\n");
                    continue;
                } else {
                    MP_DEBUG("[UNIXFS] DEBUG: a file found...\n");
					break;
                }
            }
        
            /* same level hierarchy */
            else if (strcmp(token, ".") == 0) {
                MP_DEBUG("[UNIXFS] DEBUG: same directory...\n");
                continue;
            }
        
            /* go to parent directory */
            else if (strcmp(token, "..") == 0) {
                if (CdParent(drive) != FS_SUCCEED) {
                    MP_DEBUG("[UNIXFS] ERROR: cannot change to parent directory!\n");
                    return -1;
                }
                MP_DEBUG("[UNIXFS] DEBUG: go to parent directory...\n");
                continue;
            } else {
                MP_DEBUG("[UNIXFS] DEBUG: a file found or a new file!\n");
				break;
			}
        }
    }
    while (token = strtok(NULL, "/"));

    *pathname = token;
    return curid;
}


/* -----------------------------------------------------------------------------
 * Open a File
 * -----------------------------------------------------------------------------
 * NOTE :
 *     The arguments 'flags' and 'mode' are not currently used.
 */
#define WEBS_MAX_FILES 8

static struct stream_map_s {
    char     path[256];
    STREAM  *stream;
}
stream_map[WEBS_MAX_FILES];

int
unixfs_open(const char *pathname, int flags, mode_t mode)
{
    int    prvid;
    char   path[256];
	char  *token;
    DRIVE *drive;
    int    fd;

    /* backup current drive ID to restore later */
    prvid = DriveCurIdGet();

    /* analyze the drive and directory location */
    {
        int curid;

		strncpy(path, pathname, (strlen(pathname) + 1));
		token = path;
        if ((curid = locate_file(&token)) < 0) {
            MP_DEBUG("[UNIXFS] ERROR: file cannot be located!\n");
            return -1;
        }
        drive = DriveGet(curid);
    }

    /* open the existing or create a new file */
    {
        char *fname;
        char *fext;
        int   i;

        fname = strtok(token, ".");
        fext  = strtok(NULL,  ".");
        if (fext == NULL)
            fext = "   ";

        if (FileSearch(drive, fname, fext, E_FILE_TYPE) != FS_SUCCEED) {
            if (CreateFile(drive, fname, fext) != FS_SUCCEED) {
				MP_DEBUG("[UNIXFS] ERROR: cannot create a file!\n");
                DriveChange(prvid);
                return -1;
            }
        }

        for (i = 1; i <= ARRAY_LENGTH(stream_map); i++) {
            if (stream_map[i-1].stream == NULL) {
				STREAM *handle;

				handle = FileOpen(drive);
				SeekSet(handle);

                stream_map[i-1].stream = handle;
                strncpy(stream_map[i-1].path, pathname, (strlen(pathname) + 1));
                break;
            }
        }

        fd = (i <= ARRAY_LENGTH(stream_map))? i : -1;
    }

    /* store to previous-called drive ID */
    DriveChange(prvid);
    return fd;
}


/* -----------------------------------------------------------------------------
 * Close a File
 * -----------------------------------------------------------------------------
 */
int
unixfs_close(int fd)
{
    int prvid;

    /* backup current drive ID to restore later */
    prvid = DriveCurIdGet();

    {
        STREAM *handle;

        handle = stream_map[fd-1].stream;
        if (FileClose(handle) != FS_SUCCEED) {
            MP_DEBUG("[UNIXFS] ERROR: cannot close the file!\n");
            DriveChange(prvid);
            return -1;
        }

        memset(stream_map[fd-1].path, 0x00, sizeof(stream_map[fd-1].path));
        stream_map[fd-1].stream = NULL;
    }

    /* store to previous-called drive ID */
    DriveChange(prvid);
    return 0;
}


/* -----------------------------------------------------------------------------
 * Read Data from a File
 * -----------------------------------------------------------------------------
 */
ssize_t
unixfs_read(int fd, void *buf, size_t count)
{
    int     prvid;
    ssize_t length;

    /* backup current drive ID to restore later */
    prvid = DriveCurIdGet();

    {
        STREAM *handle;

        handle = stream_map[fd-1].stream;
        length = FileRead(handle, buf, count);
        if (length < count)
            MP_DEBUG(0, ("[UNIXFS] ERROR: errors occur in read()!\n"));
    }

    /* store to previous-called drive ID */
    DriveChange(prvid);
    return length;
}


/* -----------------------------------------------------------------------------
 * Write Data to a File
 * -----------------------------------------------------------------------------
 */
ssize_t
unixfs_write(int fd, const void *buf, size_t count)
{
    int     prvid;
    ssize_t length;

    /* backup current drive ID to restore later */
    prvid = DriveCurIdGet();

    {
        STREAM *handle;

        handle = stream_map[fd-1].stream;
        length = FileWrite(handle, (BYTE *)buf, count);
        if (length < count)
            MP_DEBUG(0, ("[UNIXFS] ERROR: errors occur in write()!\n"));
    }

    /* store to previous-called drive ID */
    DriveChange(prvid);
    return length;
}


/* -----------------------------------------------------------------------------
 * Seek a File
 * -----------------------------------------------------------------------------
 */
int
unixfs_lseek(int fd, off_t offset, int whence)
{
	int prvid;

	prvid = DriveCurIdGet();

	{
		STREAM *handle;

		handle = stream_map[fd-1].stream;

		switch (whence) {
			case SEEK_SET :
				SeekSet(handle);
				if (Seek(handle, offset) != FS_SUCCEED) {
					DriveChange(prvid);
					return -1;
				}
				break;

			case SEEK_CUR :
				if (Seek(handle, offset) != FS_SUCCEED) {
					DriveChange(prvid);
					return -1;
				}
				break;

			case SEEK_END :
				EndOfFile(handle);
				if (Seek(handle, offset) != FS_SUCCEED) {
					DriveChange(prvid);
					return -1;
				}
				break;

			default :
				MP_DEBUG("[UNIXFS] ERROR: invalid arguement!\n");
				DriveChange(prvid);
				return -1;
		}
	}

	DriveChange(prvid);
	return 0;
}


/* -----------------------------------------------------------------------------
 * Unlink a File
 * -----------------------------------------------------------------------------
 */
#if 0
int
unixfs_unlink(const char *pathname)
{
    int    prvid;
    DRIVE *drive;
    char  *token;

    /* backup current drive ID to restore later */
    prvid = DriveCurIdGet();

    /* analyze the drive and directory location */
    {
        int curid;

        token = (char *)pathname;
        if ((curid = locate_file(&token)) < 0) {
            MP_DEBUG("[UNIXFS] ERROR: file cannot be located!\n");
            return -1;
        }
        drive = DriveGet(curid);
    }

    /* search the file and delete it */
    {
        char *fname;
        char *fext;
        int   i;

        fname = strtok(token, ".");
        fext  = strtok(NULL,  ".");

        if (fext == NULL)
            fext = "   ";

        if (FileSearch(drive, fname, fext, E_FILE_TYPE) != FS_SUCCEED) {
            MP_DEBUG("[UNIXFS] ERROR: fine not found!\n");
            DriveChange(prvid);
            return -1;
        }

        if (DeleteFile(FileOpen(drive)) != FS_SUCCEED) {
            MP_DEBUG("[UNIXFS] ERROR: errors occur in unlink()!\n");
            DriveChange(prvid);
            return -1;
        }
    }

    /* store to previous-called drive ID */
    DriveChange(prvid);
    return 0;
}
#else
int
unixfs_unlink(const char *pathname)
{
    return 0;
}
#endif


/* -----------------------------------------------------------------------------
 * Get Status of a File
 * -----------------------------------------------------------------------------
 */
#if 0
int
unixfs_stat(const char *file_name, struct stat *buf)
{
    int    prvid;
    DRIVE *drive;
    char  *token;

    /* backup current drive ID to restore later */
    prvid = DriveCurIdGet();

    /* analyze the drive and directory location */
    {
        int curid;

        token = (char *)file_name;
        if ((curid = locate_file(&token)) < 0) {
            MP_DEBUG("[UNIXFS] ERROR: file cannot be located!\n");
            return -1;
        }
        drive = DriveGet(curid);
    }

    {
        char   *fname;
        char   *fext;
        STREAM *handle;

        fname = strtok(token, ".");
        fext  = strtok(NULL,  ".");

        if (fext == NULL)
            fext = "   ";

        if (FileSearch(drive, fname, fext, E_FILE_TYPE) != FS_SUCCEED) {
            MP_DEBUG("[UNIXFS] ERROR: fine not found!\n");
            DriveChange(prvid);
            return -1;
        }

        handle = FileOpen(drive);

        memset(buf, 0x00, sizeof(buf));
        buf->st_size = FileSizeGet(handle);
        FileClose(handle);
    }

    /* store to previous-called drive ID */
    DriveChange(prvid);
    return 0;
}
#else
int
unixfs_stat(const char *file_name, struct stat *buf)
{
    return 0;
}
#endif


/* -----------------------------------------------------------------------------
 * Rename a File
 * -----------------------------------------------------------------------------
 */
#if 0
int
unixfs_rename(const char *oldpath, const char *newpath)
{
    return 0;
}
#else
int
unixfs_rename(const char *oldpath, const char *newpath)
{
    return 0;
}
#endif


/* $Id$ */
