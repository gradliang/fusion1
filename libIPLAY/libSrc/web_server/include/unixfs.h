#ifndef __UNIXFS_H__
#define __UNIXFS_H__


#ifndef NAME_MAX
#define NAME_MAX 256
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


#if defined(gclose)
  #undef  gclose
  #define gclose    unixfs_close
#endif

#if defined(glseek)
  #undef  glseek
  #define glseek    unixfs_lseek
#endif

#if defined(gopen)
  #undef  gopen
  #define gopen     unixfs_open
#endif

#if defined(gread)
  #undef  gread
  #define gread     unixfs_read
#endif

#if defined(grename)
  #undef  grename
  #define grename   unixfs_rename
#endif

#if defined(gstat)
  #undef  gstat
  #define gstat     unixfs_stat
#endif

#if defined(gunlink)
  #undef  gunlink
  #define gunlink   unixfs_unlink
#endif

#if defined(gwrite)
  #undef  gwrite
  #define gwrite    unixfs_write
#endif


int
unixfs_open(const char *pathname, int flags, mode_t mode);

int
unixfs_lseek(int fd, off_t offset, int whence);

int
unixfs_close(int fd);

ssize_t
unixfs_read(int fd, void *buf, size_t count);

ssize_t
unixfs_write(int fd, const void *buf, size_t count);

int
unixfs_unlink(const char *pathname);

int
unixfs_stat(const char *file_name, struct stat *buf);


#endif /* __FILE_H__ */


/* $Id$ */
