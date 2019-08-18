#include <errno.h>
#include "zipint.h"

#ifndef UINT_MAX
#define UINT_MAX	(~0U)
#endif


int pop3read(STREAM * handle, BYTE *da_buf, DWORD size,BYTE *sa_buf)
{
    //mpDebugPrint("%s:%d",__func__,size);
	if ((handle->Chain.Point + size) > handle->Chain.Size)
	{
		size = handle->Chain.Size - handle->Chain.Point;
	}
	memcpy(da_buf, sa_buf+handle->Chain.Point,size);

	handle->Chain.Point += size;
	
	return size;

}
int pop3seek(STREAM * shandle, DWORD position, DWORD origin )
{
    //mpDebugPrint("%s %x %d",__func__,position,origin);
	switch (origin)
	{
		case SEEK_SET:
        	shandle->Chain.Point = position;
			break;
		case SEEK_END:
			shandle->Chain.Point = shandle->Chain.Size;
			break;
	}
	return 0;

}
static void
pop3_set_error(int *zep, struct zip_error *err, int ze)
{
    int se;

    if (err) {
	_zip_error_get(err, &ze, &se);
	if (zip_error_get_sys_type(ze) == ZIP_ET_SYS)
	    errno = se;
    }

    if (zep)
	*zep = ze;
}

/* _zip_readcdir:
   tries to find a valid end-of-central-directory at the beginning of
   buf, and then the corresponding central directory entries.
   Returns a struct zip_cdir which contains the central directory 
   entries, or NULL if unsuccessful. */

static struct zip_cdir *
pop3_zip_readcdir(STREAM *fp, unsigned char *buf, unsigned char *eocd, int buflen,
	      int flags, struct zip_error *error)
{
    struct zip_cdir *cd;
    unsigned char *cdp, **bufp;
    int i, comlen, nentry;
	
    mpDebugPrint("%s",__func__);
		
	comlen = buf + buflen - eocd - EOCDLEN;
	
    if (comlen < 0) {
	/* not enough bytes left for comment */
	_zip_error_set(error, ZIP_ER_NOZIP, 0);
	return NULL;
    }

    /* check for end-of-central-dir magic */
    if (memcmp(eocd, EOCD_MAGIC, 4) != 0) {
	_zip_error_set(error, ZIP_ER_NOZIP, 0);
	return NULL;
    }
	
    if (memcmp(eocd+4, "\0\0\0\0", 4) != 0) {
	_zip_error_set(error, ZIP_ER_MULTIDISK, 0);
	return NULL;
    }

    cdp = eocd + 8;
	
    /* number of cdir-entries on this disk */
    i = _zip_read2(&cdp);
    /* number of cdir-entries */
    nentry = _zip_read2(&cdp);

    if ((cd=_zip_cdir_new(nentry, error)) == NULL)
	return NULL;
	
    cd->size = _zip_read4(&cdp);
    cd->offset = _zip_read4(&cdp);
    cd->comment = NULL;
    cd->comment_len = _zip_read2(&cdp);

    if ((comlen < cd->comment_len) || (cd->nentry != i)) {
	_zip_error_set(error, ZIP_ER_NOZIP, 0);
	free(cd);
	return NULL;
    }
    if ((flags & ZIP_CHECKCONS) && comlen != cd->comment_len) {
	_zip_error_set(error, ZIP_ER_INCONS, 0);
	free(cd);
	return NULL;
    }

    if (cd->comment_len) {
	if ((cd->comment=(char *)_zip_memdup(eocd+EOCDLEN,
					     cd->comment_len, error))
	    == NULL) {
	    free(cd);
	    return NULL;
	}
    }

    cdp = eocd;
    if (cd->size < (unsigned int)(eocd-buf)) {
	/* if buffer already read in, use it */
	cdp = eocd - cd->size;
	bufp = &cdp;
    }
    else {
	/* go to start of cdir and read it entry by entry */
	bufp = NULL;

	//clearerr(fp);
	pop3seek(fp,0,SEEK_SET); //cj add
	
	pop3seek(fp, cd->offset, SEEK_SET);
	/* possible consistency check: cd->offset =
	   len-(cd->size+cd->comment_len+EOCDLEN) ? */
	if ((fp == NULL) || ((unsigned long)ftello(fp) != cd->offset)) {
	    /* seek error or offset of cdir wrong */
	    if (fp==NULL)
		_zip_error_set(error, ZIP_ER_SEEK, errno);
	    else
		_zip_error_set(error, ZIP_ER_NOZIP, 0);
	    free(cd);
	    return NULL;
		
	}
	
    }

    for (i=0; i<cd->nentry; i++) {
	if ((pop3_zip_dirent_read(cd->entry+i, fp, bufp, eocd-cdp, 0,
			      error)) < 0) {
	    cd->nentry = i;
	    _zip_cdir_free(cd);
	    return NULL;
	}
    }
    
    return cd;
}



/* _zip_checkcons:
   Checks the consistency of the central directory by comparing central
   directory entries with local headers and checking for plausible
   file and header offsets. Returns -1 if not plausible, else the
   difference between the lowest and the highest fileposition reached */

static int
pop3_zip_checkcons(STREAM *fp, struct zip_cdir *cd, struct zip_error *error)
{
    int i;
    unsigned int min, max, j;
    struct zip_dirent temp;

    if (cd->nentry) {
	max = cd->entry[0].offset;
	min = cd->entry[0].offset;
    }
    else
	min = max = 0;

    for (i=0; i<cd->nentry; i++) {
	if (cd->entry[i].offset < min)
	    min = cd->entry[i].offset;
	if (min > cd->offset) {
	    _zip_error_set(error, ZIP_ER_NOZIP, 0);
	    return -1;
	}
	
	j = cd->entry[i].offset + cd->entry[i].comp_size
	    + cd->entry[i].filename_len + LENTRYSIZE;
	if (j > max)
	    max = j;
	if (max > cd->offset) {
	    _zip_error_set(error, ZIP_ER_NOZIP, 0);
	    return -1;
	}
	
	if (pop3seek(fp, cd->entry[i].offset, SEEK_SET) != 0) {
	    _zip_error_set(error, ZIP_ER_SEEK, 0);
	    return -1;
	}
	
	if (pop3_zip_dirent_read(&temp, fp, NULL, 0, 1, error) == -1)
	    return -1;
	
	if (pop3_zip_headercomp(cd->entry+i, 0, &temp, 1) != 0) {
	    _zip_error_set(error, ZIP_ER_INCONS, 0);
	    _zip_dirent_finalize(&temp);
	    return -1;
	}
	_zip_dirent_finalize(&temp);
    }

    return max - min;
}


static unsigned char *
pop3_zip_memmem(const unsigned char *big, int biglen, const unsigned char *little, 
       int littlelen)
{
    const unsigned char *p,*q;
	
    //mpDebugPrint("%s",__func__);
		
    if ((biglen < littlelen) || (littlelen == 0))
	    return NULL;
    p = big-1;
    while ((p=(const unsigned char *)
	        memchr(p+1, little[0], (size_t)(big-(p+1)+biglen-littlelen+1)))
	   != NULL) 
	   {
	   
		if (memcmp(p+1, little+1, littlelen-1)==0)
		{
	    	return (unsigned char *)p;
		}
		q= p;
    	}

    return NULL;
}

static struct zip *
pop3_zip_allocate_new(const char *fn, int *zep)
{
    struct zip *za;
    struct zip_error error;

    if ((za=_zip_new(&error)) == NULL) {
	pop3_set_error(zep, &error, 0);
	return NULL;
    }
	
    za->zn = strdup(fn);
    if (!za->zn) {
	_zip_free(za);
	pop3_set_error(zep, NULL, ZIP_ER_MEMORY);
	return NULL;
    }
    return za;
}

static struct zip_cdir *
pop3_zip_find_central_dir(STREAM *fp, int flags, int *zep, off_t len)
{
    struct zip_cdir *cdir, *cdirnew;
    unsigned char *buf, *match;
    int a, best, buflen, i;
    struct zip_error zerr;
	
	//mpDebugPrint("%s",__func__);

 	i = pop3seek(fp, -(len), SEEK_END);
    if (i == -1 && errno != EFBIG) {
	/* seek before start of file on my machine */
	pop3_set_error(zep, NULL, ZIP_ER_SEEK);
	return NULL;
    }

    /* 64k is too much for stack */
    if ((buf=(unsigned char *)malloc(len)) == NULL) {
	pop3_set_error(zep, NULL, ZIP_ER_MEMORY);
	return NULL;
    }

    //clearerr(fp);
    pop3seek(fp,0,SEEK_SET);
    
    //buflen = fread(buf, 1, len, fp);
	buflen = len;
	memcpy(buf,fp->Chain.Start,len);
	
    best = -1;
    cdir = NULL;
    match = buf;
    _zip_error_set(&zerr, ZIP_ER_NOZIP, 0);
	
    while ((match=pop3_zip_memmem(match, buflen-(match-buf)-18,
			      (const unsigned char *)EOCD_MAGIC, 4))!=NULL) 
	{
	/* found match -- check, if good */
	/* to avoid finding the same match all over again */
	match++;
	if ((cdirnew=pop3_zip_readcdir(fp, buf, match-1, buflen, flags,
				   &zerr)) == NULL)
	{
	    mpDebugPrint("continue");
	    continue;
	}

	if (cdir) {
	    if (best <= 0)
		best = pop3_zip_checkcons(fp, cdir, &zerr);
	    a = pop3_zip_checkcons(fp, cdirnew, &zerr);
	    if (best < a) {
		_zip_cdir_free(cdir);
		cdir = cdirnew;
		best = a;
	    }
	    else
		_zip_cdir_free(cdirnew);
	}
	else {
	    cdir = cdirnew;
	    if (flags & ZIP_CHECKCONS)
		best = pop3_zip_checkcons(fp, cdir, &zerr);
	    else
		best = 0;
	}
	cdirnew = NULL;
    }

    free(buf);
    
    if (best < 0) {
	pop3_set_error(zep, &zerr, 0);
	_zip_cdir_free(cdir);
	return NULL;
    }

    return cdir;
}

/* _zip_check_torrentzip:
   check wether ZA has a valid TORRENTZIP comment, i.e. is torrentzipped */

static void
pop3_zip_check_torrentzip(struct zip *za)
{
    uLong crc_got, crc_should;
    char buf[8+1];
    char *end;

    if (za->zp == NULL || za->cdir == NULL)
	return;

    if (za->cdir->comment_len != TORRENT_SIG_LEN+8
	|| strncmp(za->cdir->comment, TORRENT_SIG, TORRENT_SIG_LEN) != 0)
	return;

    memcpy(buf, za->cdir->comment+TORRENT_SIG_LEN, 8);
    buf[8] = '\0';
    errno = 0;
    crc_should = strtoul(buf, &end, 16);
    if ((crc_should == UINT_MAX && errno != 0) || (end && *end))
	return;

    if (_zip_filerange_crc(za->zp, za->cdir->offset, za->cdir->size,
			   &crc_got, NULL) < 0)
	return;

    if (crc_got == crc_should)
	za->flags |= ZIP_AFL_TORRENT;
}




/* _zip_headercomp:
   compares two headers h1 and h2; if they are local headers, set
   local1p or local2p respectively to 1, else 0. Return 0 if they
   are identical, -1 if not. */

static int
pop3_zip_headercomp(struct zip_dirent *h1, int local1p, struct zip_dirent *h2,
	   int local2p)
{
    if ((h1->version_needed != h2->version_needed)
#if 0
	/* some zip-files have different values in local
	   and global headers for the bitflags */
	|| (h1->bitflags != h2->bitflags)
#endif
	|| (h1->comp_method != h2->comp_method)
	|| (h1->last_mod != h2->last_mod)
	|| (h1->filename_len != h2->filename_len)
	|| !h1->filename || !h2->filename
	|| strcmp(h1->filename, h2->filename))
	return -1;

    /* check that CRC and sizes are zero if data descriptor is used */
    if ((h1->bitflags & ZIP_GPBF_DATA_DESCRIPTOR) && local1p
	&& (h1->crc != 0
	    || h1->comp_size != 0
	    || h1->uncomp_size != 0))
	return -1;
    if ((h2->bitflags & ZIP_GPBF_DATA_DESCRIPTOR) && local2p
	&& (h2->crc != 0
	    || h2->comp_size != 0
	    || h2->uncomp_size != 0))
	return -1;
    
    /* check that CRC and sizes are equal if no data descriptor is used */
    if (((h1->bitflags & ZIP_GPBF_DATA_DESCRIPTOR) == 0 || local1p == 0)
	&& ((h2->bitflags & ZIP_GPBF_DATA_DESCRIPTOR) == 0 || local2p == 0)) {
	if ((h1->crc != h2->crc)
	    || (h1->comp_size != h2->comp_size)
	    || (h1->uncomp_size != h2->uncomp_size))
	    return -1;
    }
    
    if ((local1p == local2p)
	&& ((h1->extrafield_len != h2->extrafield_len)
	    || (h1->extrafield_len && h2->extrafield
		&& memcmp(h1->extrafield, h2->extrafield,
			  h1->extrafield_len))))
	return -1;

    /* if either is local, nothing more to check */
    if (local1p || local2p)
	return 0;

    if ((h1->version_madeby != h2->version_madeby)
	|| (h1->disk_number != h2->disk_number)
	|| (h1->int_attrib != h2->int_attrib)
	|| (h1->ext_attrib != h2->ext_attrib)
	|| (h1->offset != h2->offset)
	|| (h1->comment_len != h2->comment_len)
	|| (h1->comment_len && h2->comment
	    && memcmp(h1->comment, h2->comment, h1->comment_len)))
	return -1;

    return 0;
}

ZIP_EXTERN struct zip *
pop3_zip_open(const char *fn, int lengh, char *buf,int *zep)
{
    //mpDebugPrint("%s: %s buf %x",__func__,fn,buf);
    //FILE *fp;
    STREAM *fp;
    int iIndex;
    ST_SEARCH_INFO *pSearchInfo;
    struct zip *za;
    struct zip_cdir *cdir;
    int i;
    off_t len;
    int flags = 0;
	
    if (!(fp = GetFreeFileHandle(DriveGet(DriveCurIdGet()))))
    {
        MP_ALERT("%s: No file handle available !", __FUNCTION__);
        return NULL;
    }
	fp->Chain.Size = lengh;
	fp->Chain.Start = buf;
	fp->Chain.Point = 0;

    pop3seek(fp, 0, SEEK_END);
    len = ftello(fp);
    /* treat empty files as empty archives */
    if (len == 0) {
	if ((za=pop3_zip_allocate_new(fn, zep)) == NULL)
	    fclose(fp);
	else
	    za->zp = fp;
	return za;
    }

    cdir = pop3_zip_find_central_dir(fp, flags, zep, len);
    if (cdir == NULL) {
	fclose(fp);
	return NULL;
    }

    if ((za=pop3_zip_allocate_new(fn, zep)) == NULL) {
	_zip_cdir_free(cdir);
	fclose(fp);
	return NULL;
    }

    za->cdir = cdir;
    za->zp = fp;

    if ((za->entry=(struct zip_entry *)malloc(sizeof(*(za->entry))
					      * cdir->nentry)) == NULL) {
	pop3_set_error(zep, NULL, ZIP_ER_MEMORY);
	_zip_free(za);
	return NULL;
    }
    for (i=0; i<cdir->nentry; i++)
	_zip_entry_new(za);

    pop3_zip_check_torrentzip(za);
    za->ch_flags = za->flags;

    return za;
}




static char *
pop3_zip_readstr(unsigned char **buf, int len, int nulp, struct zip_error *error)
{
    char *r, *o;

    r = (char *)malloc(nulp ? len+1 : len);
    if (!r) {
	_zip_error_set(error, ZIP_ER_MEMORY, 0);
	return NULL;
    }
    
    memcpy(r, *buf, len);
    *buf += len;

    if (nulp) {
	/* replace any in-string NUL characters with spaces */
	r[len] = 0;
	for (o=r; o<r+len; o++)
	    if (*o == '\0')
		*o = ' ';
    }

    return r;
}



static char *
pop3_zip_readfpstr(STREAM *fp, unsigned int len, int nulp, struct zip_error *error,char *sa_buf)
{
    char *r, *o;
	
    //mpDebugPrint("%s",__func__);
		
    r = (char *)malloc(nulp ? len+1 : len);
    if (!r) {
	_zip_error_set(error, ZIP_ER_MEMORY, 0);
	return NULL;
    }

    //if (fread(r, 1, len, fp)<len) 
	if (pop3read(fp,r,len,sa_buf)<len) 	
	{
		free(r);
		_zip_error_set(error, ZIP_ER_READ, errno);
		return NULL;
    }

    if (nulp) {
	/* replace any in-string NUL characters with spaces */
	r[len] = 0;
	for (o=r; o<r+len; o++)
	    if (*o == '\0')
		*o = ' ';
    }
    
    return r;
}


/* _zip_dirent_read(zde, fp, bufp, left, localp, error):
   Fills the zip directory entry zde.

   If bufp is non-NULL, data is taken from there and bufp is advanced
   by the amount of data used; no more than left bytes are used.
   Otherwise data is read from fp as needed.

   If localp != 0, it reads a local header instead of a central
   directory entry.

   Returns 0 if successful. On error, error is filled in and -1 is
   returned.
*/

int
pop3_zip_dirent_read(struct zip_dirent *zde, STREAM *fp,
		 unsigned char **bufp, unsigned int left, int localp,
		 struct zip_error *error,char *sa_buf)
{
    unsigned char buf[CDENTRYSIZE];
    unsigned char *cur;
    unsigned short dostime, dosdate;
    unsigned int size;
	
    //mpDebugPrint("%s",__func__);
		
    if (localp)
	size = LENTRYSIZE;
    else
	size = CDENTRYSIZE;
    
    if (bufp) {
	/* use data from buffer */
	cur = *bufp;
	if (left < size) {
	    _zip_error_set(error, ZIP_ER_NOZIP, 0);
	    return -1;
	}
    }
    else {
	/* read entry from disk */
	//if ((fread(buf, 1, size, fp)<size)) 
	if ((pop3read(fp,buf, size,sa_buf)<size)) 
	{
	    _zip_error_set(error, ZIP_ER_READ, errno);
	    return -1;
	}
	left = size;
	cur = buf;
    }
    //mpDebugPrint("cur %s",cur);
	//NetPacketDump(cur, 4);
	//NetPacketDump((localp ? LOCAL_MAGIC : CENTRAL_MAGIC), 4);
    if (memcmp(cur, (localp ? LOCAL_MAGIC : CENTRAL_MAGIC), 4) != 0) {
	_zip_error_set(error, ZIP_ER_NOZIP, 0);
	return -1;
     }
    cur += 4;

    
    /* convert buffercontents to zip_dirent */
    
    if (!localp)
	zde->version_madeby = _zip_read2(&cur);
    else
	zde->version_madeby = 0;
    zde->version_needed = _zip_read2(&cur);
    zde->bitflags = _zip_read2(&cur);
    zde->comp_method = _zip_read2(&cur);
    
    /* convert to time_t */
    dostime = _zip_read2(&cur);
    dosdate = _zip_read2(&cur);
    //zde->last_mod = _zip_d2u_time(dostime, dosdate);
    zde->last_mod = 0;//_zip_d2u_time(dostime, dosdate);
    zde->crc = _zip_read4(&cur);
    zde->comp_size = _zip_read4(&cur);
    zde->uncomp_size = _zip_read4(&cur);
    
    zde->filename_len = _zip_read2(&cur);
    zde->extrafield_len = _zip_read2(&cur);
    
    if (localp) {
	zde->comment_len = 0;
	zde->disk_number = 0;
	zde->int_attrib = 0;
	zde->ext_attrib = 0;
	zde->offset = 0;
    } else {
	zde->comment_len = _zip_read2(&cur);
	zde->disk_number = _zip_read2(&cur);
	zde->int_attrib = _zip_read2(&cur);
	zde->ext_attrib = _zip_read4(&cur);
	zde->offset = _zip_read4(&cur);
    }

    zde->filename = NULL;
    zde->extrafield = NULL;
    zde->comment = NULL;

    if (bufp) {
		
	if (left < CDENTRYSIZE + (zde->filename_len+zde->extrafield_len
				  +zde->comment_len)) {
	    _zip_error_set(error, ZIP_ER_NOZIP, 0);
	    return -1;
	}

	if (zde->filename_len) {
	    zde->filename = pop3_zip_readstr(&cur, zde->filename_len, 1, error);
	    if (!zde->filename)
		    return -1;
	}

	if (zde->extrafield_len) {
	    zde->extrafield = pop3_zip_readstr(&cur, zde->extrafield_len, 0,
					   error);
	    if (!zde->extrafield)
		return -1;
	}

	if (zde->comment_len) {
	    zde->comment = pop3_zip_readstr(&cur, zde->comment_len, 0, error);
	    if (!zde->comment)
		return -1;
	}
    }
    else {
	if (zde->filename_len) {
	    zde->filename = pop3_zip_readfpstr(fp, zde->filename_len, 1, error,sa_buf);
	    if (!zde->filename)
		    return -1;
	}

	if (zde->extrafield_len) {
	    zde->extrafield = pop3_zip_readfpstr(fp, zde->extrafield_len, 0,error,sa_buf);
	    if (!zde->extrafield)
		return -1;
	}

	if (zde->comment_len) {
	    zde->comment = pop3_zip_readfpstr(fp, zde->comment_len, 0, error,sa_buf);
	    if (!zde->comment)
		return -1;
	}
    }

    if (bufp)
      *bufp = cur;

    return 0;
}

/* _zip_file_get_offset(za, ze):
   Returns the offset of the file data for entry ze.

   On error, fills in za->error and returns 0.
*/

unsigned int
pop3_zip_file_get_offset(struct zip *za, int idx,char *buf)
{
    struct zip_dirent de;
    unsigned int offset;
	
    //mpDebugPrint("%s",__func__);

    offset = za->cdir->entry[idx].offset;

    if (pop3seek(za->zp, offset, SEEK_SET) != 0) {
	_zip_error_set(&za->error, ZIP_ER_SEEK, errno);
	return 0;
    }
    if (pop3_zip_dirent_read(&de, za->zp,NULL, 0, 1, &za->error,buf) != 0)
	return 0;

    offset += LENTRYSIZE + de.filename_len + de.extrafield_len;

    _zip_dirent_finalize(&de);

    return offset;
}

int
pop3_zip_file_fillbuf(void *buf, size_t buflen, struct zip_file *zf,char *sbuf)
{
    int i, j;
	
    //mpDebugPrint("%s",__func__);

    if (zf->error.zip_err != ZIP_ER_OK)
	return -1;

    if ((zf->flags & ZIP_ZF_EOF) || zf->cbytes_left <= 0 || buflen <= 0)
	return 0;
    
    if (pop3seek(zf->za->zp, zf->fpos, SEEK_SET) < 0) {
	_zip_error_set(&zf->error, ZIP_ER_SEEK, errno);
	return -1;
    }
    if (buflen < zf->cbytes_left)
	i = buflen;
    else
	i = zf->cbytes_left;

    //j = fread(buf, 1, i, zf->za->zp);
    j = pop3read(zf->za->zp,buf,i,sbuf);

    if (j == 0) {
	_zip_error_set(&zf->error, ZIP_ER_EOF, 0);
	j = -1;
    }
    else if (j < 0)
	_zip_error_set(&zf->error, ZIP_ER_READ, errno);
    else {
	zf->fpos += j;
	zf->cbytes_left -= j;
    }

    return j;	
}

static struct zip_file *
pop3_zip_file_new(struct zip *za)
{
    struct zip_file *zf, **file;
    int n;
	
    //mpDebugPrint("%s",__func__);

    if ((zf=(struct zip_file *)malloc(sizeof(struct zip_file))) == NULL) {
	_zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
	return NULL;
    }
    
    if (za->nfile >= za->nfile_alloc-1) {
	n = za->nfile_alloc + 10;
	file = (struct zip_file **)realloc(za->file,
					   n*sizeof(struct zip_file *));
	if (file == NULL) {
	    _zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
	    free(zf);
	    return NULL;
	}
	za->nfile_alloc = n;
	za->file = file;
    }

    za->file[za->nfile++] = zf;

    zf->za = za;
    _zip_error_init(&zf->error);
    zf->flags = 0;
    zf->crc = crc32(0L, Z_NULL, 0);
    zf->crc_orig = 0;
    zf->method = -1;
    zf->bytes_left = zf->cbytes_left = 0;
    zf->fpos = 0;
    zf->buffer = NULL;
    zf->zstr = NULL;

    return zf;
}



ZIP_EXTERN struct zip_file *
pop3_zip_fopen_index(struct zip *za, int fileno, int flags,char *buf)
{
    int len, ret;
    int zfflags;
    struct zip_file *zf;
	
    //mpDebugPrint("%s",__func__);
		
    if ((fileno < 0) || (fileno >= za->nentry)) {
	_zip_error_set(&za->error, ZIP_ER_INVAL, 0);
	return NULL;
    }

    if ((flags & ZIP_FL_UNCHANGED) == 0
	&& ZIP_ENTRY_DATA_CHANGED(za->entry+fileno)) {
	_zip_error_set(&za->error, ZIP_ER_CHANGED, 0);
	return NULL;
    }

    if (fileno >= za->cdir->nentry) {
	_zip_error_set(&za->error, ZIP_ER_INVAL, 0);
	return NULL;
    }

    zfflags = 0;
    switch (za->cdir->entry[fileno].comp_method) {
    case ZIP_CM_STORE:
	zfflags |= ZIP_ZF_CRC;
	break;

    case ZIP_CM_DEFLATE:
	if ((flags & ZIP_FL_COMPRESSED) == 0)
	    zfflags |= ZIP_ZF_CRC | ZIP_ZF_DECOMP;
	break;
    default:
	if ((flags & ZIP_FL_COMPRESSED) == 0) {
	    _zip_error_set(&za->error, ZIP_ER_COMPNOTSUPP, 0);
	    return NULL;
	}
	break;
    }

    zf = pop3_zip_file_new(za);

    zf->flags = zfflags;
    /* zf->name = za->cdir->entry[fileno].filename; */
    zf->method = za->cdir->entry[fileno].comp_method;
    zf->bytes_left = za->cdir->entry[fileno].uncomp_size;
    zf->cbytes_left = za->cdir->entry[fileno].comp_size;
    zf->crc_orig = za->cdir->entry[fileno].crc;

    if ((zf->fpos=pop3_zip_file_get_offset(za, fileno,buf)) == 0) {
	zip_fclose(zf);
	return NULL;
    }
    
    if ((zf->flags & ZIP_ZF_DECOMP) == 0)
	zf->bytes_left = zf->cbytes_left;
    else {
	if ((zf->buffer=(char *)malloc(BUFSIZE)) == NULL) {
	    _zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
	    zip_fclose(zf);
	    return NULL;
	}

	len = pop3_zip_file_fillbuf(zf->buffer, BUFSIZE, zf,buf);
	if (len <= 0) {
	    _zip_error_copy(&za->error, &zf->error);
	    zip_fclose(zf);
	return NULL;
	}

	if ((zf->zstr = (z_stream *)malloc(sizeof(z_stream))) == NULL) {
	    _zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
	    zip_fclose(zf);
	    return NULL;
	}
	zf->zstr->zalloc = Z_NULL;
	zf->zstr->zfree = Z_NULL;
	zf->zstr->opaque = NULL;
	zf->zstr->next_in = (Bytef *)zf->buffer;
	zf->zstr->avail_in = len;
	
	/* negative value to tell zlib that there is no header */
	if ((ret=inflateInit2(zf->zstr, -MAX_WBITS)) != Z_OK) {
	    _zip_error_set(&za->error, ZIP_ER_ZLIB, ret);
	    zip_fclose(zf);
	    return NULL;
	}
    }
    
    return zf;
}



ZIP_EXTERN ssize_t
pop3_zip_fread(struct zip_file *zf, void *outbuf, size_t toread,void *sa_buf)
{
    int ret;
    size_t out_before, len;
    int i;

    if (!zf)
	return -1;

    if (zf->error.zip_err != 0)
	return -1;

    if ((zf->flags & ZIP_ZF_EOF) || (toread == 0))
	return 0;

    if (zf->bytes_left == 0) {
	zf->flags |= ZIP_ZF_EOF;
	if (zf->flags & ZIP_ZF_CRC) {
	    if (zf->crc != zf->crc_orig) {
		_zip_error_set(&zf->error, ZIP_ER_CRC, 0);
		return -1;
	    }
	}
	return 0;
    }
    
    if ((zf->flags & ZIP_ZF_DECOMP) == 0) {
	ret = pop3_zip_file_fillbuf(outbuf, toread, zf,sa_buf);
	if (ret > 0) {
	    if (zf->flags & ZIP_ZF_CRC)
		zf->crc = crc32(zf->crc, (Bytef *)outbuf, ret);
	    zf->bytes_left -= ret;
	}
	return ret;
    }
    
    zf->zstr->next_out = (Bytef *)outbuf;
    zf->zstr->avail_out = toread;
    out_before = zf->zstr->total_out;
    
    /* endless loop until something has been accomplished */
    for (;;) {
	ret = inflate(zf->zstr, Z_SYNC_FLUSH);

	switch (ret) {
	case Z_OK:
	case Z_STREAM_END:
	    /* all ok */
	    /* Z_STREAM_END probably won't happen, since we didn't
	       have a header */
	    len = zf->zstr->total_out - out_before;
	    if (len >= zf->bytes_left || len >= toread) {
		if (zf->flags & ZIP_ZF_CRC)
		    zf->crc = crc32(zf->crc, (Bytef *)outbuf, len);
		zf->bytes_left -= len;
	        return len;
	    }
	    break;

	case Z_BUF_ERROR:
	    if (zf->zstr->avail_in == 0) {
		i = pop3_zip_file_fillbuf(zf->buffer, BUFSIZE, zf,sa_buf);
		if (i == 0) {
		    _zip_error_set(&zf->error, ZIP_ER_INCONS, 0);
		    return -1;
		}
		else if (i < 0)
		    return -1;
		zf->zstr->next_in = (Bytef *)zf->buffer;
		zf->zstr->avail_in = i;
		continue;
	    }
	    /* fallthrough */
	case Z_NEED_DICT:
	case Z_DATA_ERROR:
	case Z_STREAM_ERROR:
	case Z_MEM_ERROR:
	    _zip_error_set(&zf->error, ZIP_ER_ZLIB, ret);
	    return -1;
	}
    }
}

unsigned int
pop3_zip_get_unzip_size(struct zip *za, int idx)
{
    return za->cdir->entry[idx].uncomp_size;
}

