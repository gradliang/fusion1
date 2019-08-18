#define LOCAL_DEBUG_ENABLE 0

#include <string.h>
#include "global612.h"

#if (DISPLAY_VIDEO_SUBTITLE || LYRIC_ENABLE)

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#include "mpTrace.h"


#include "stream_type.h"

#include "Sub_common.h"



#ifndef xpg__H__
#include "xpg.h"
#endif



S_SUB_NODE_T* g_SubNodeHead;
S_SUB_NODE_T* g_SubNodeTail;
S_SUB_NODE_T* g_SubNodeCur;
extern XPG_FUNC_PTR* g_pXpgFuncPtr;
S_SUBTITLE_INFO_T g_sSubtitleInfo;
static E_CHAR_ENCODING g_eSubCharEncoding;

int sub_get_subtitle_info(S_SUBTITLE_INFO_T* p_subtitle_info)
{
    memcpy(p_subtitle_info, &g_sSubtitleInfo, sizeof(g_sSubtitleInfo));
	return PASS;
}

E_CHAR_ENCODING sub_get_char_encoding(void)
{
    return g_eSubCharEncoding;
}

int sub_read_file(STREAM *fd, int subtitle_sync_align)
{
    int sub_format = SUB_INVALID;
    int uses_time = 0;
	int utf16_src;
	stream_t *st = sub_stream_create(fd);
	if (NULL == st)
	{
        mpDebugPrint("sub_stream_create(0x%x) fail", fd);
		return FAIL;
	}	
	
	for (utf16_src = 0; sub_format == SUB_INVALID && utf16_src < 3; utf16_src++) 
	{
        sub_format = sub_auto_detect(st, &uses_time, (E_CHAR_ENCODING) utf16_src);		    
        sub_stream_reset(st);
        sub_stream_seek_long(st, 0);
		if (sub_format != SUB_INVALID)
			break;
    }
    g_eSubCharEncoding = (E_CHAR_ENCODING) utf16_src;
	
	g_sSubtitleInfo.lang_index = LANGUAGE_UNKNOWN; 
	if (SUB_SUBRIP == sub_format)
	{
		SubRip_read_file(st, (E_CHAR_ENCODING) utf16_src, subtitle_sync_align);
	}
	else if (SUB_SAMI == sub_format)
	{
		SAMI_read_file(st, (E_CHAR_ENCODING) utf16_src, subtitle_sync_align);
	}
	
	return PASS;	
}

int sub_auto_detect (stream_t* st, int *uses_time, E_CHAR_ENCODING utf16_src) 
{
    char line[SUB_LINE_LEN+1];
    int i,j=0;
    char p;
    
	    	
    while (j < 100) 
	{
	    j++;
	    if (!sub_read_line(st, line, SUB_LINE_LEN, utf16_src, CHAR_ENCODING_UTF_8))
	        return SUB_INVALID;

        int num;

	    if (num = sscanf(line, "%d:%d:%d,%d --> %d:%d:%d,%d",     &i, &i, &i, &i, &i, &i, &i, &i)==8)
	    {
	        *uses_time=1;
		    return SUB_SUBRIP;
	    }
	
	    if (strstr (line, "<SAMI>"))
		{
		    *uses_time=1; 
			return SUB_SAMI;
		}
	
    }
    return SUB_INVALID;  // too many bad lines
}


unsigned char* sub_read_line(stream_t *s, unsigned char* mem, int max, E_CHAR_ENCODING utf16_src, E_CHAR_ENCODING utf16_dst) 
{
    //MP_DEBUG("sub_stream_read_line");
	
    int len, l;
    unsigned char* end,*ptr = mem;


    do 
    {
        len = s->buf_len-s->buf_pos;
		//mpDebugPrint("s->buf_len = %d, s->buf_pos = %d, len = %d", s->buf_len, s->buf_pos, len);
        // try to fill the buffer
        if(len <= 0 && 
        	 (!sub_stream_fill_buffer(s) ||
           (len = s->buf_len-s->buf_pos) <= 0)) 
           break;
           
        //end = (unsigned char*) memchr((void*)(s->buffer+s->buf_pos),'\n',len);
        end = sub_find_newline(s->buffer+s->buf_pos, len, utf16_src);
		//mpDebugPrint("end = 0x%x", end);
        if (end) 
        	  len = end - (s->buffer+s->buf_pos) + 1;

		#if 0
        if (len > 0 && max > 1) 
        {
           int l = len > max-1 ? max-1 : len;
           memcpy(ptr,s->buffer+s->buf_pos,l);
           max -= l;
           ptr += l;
        }
		#else
		if (len > 0 && max > 0) 
		{
           l = sub_copy_characters(ptr, max, s->buffer+s->buf_pos, &len, utf16_src, utf16_dst);
           max -= l;
           ptr += l;
           if (!len)
              break;
        }
		#endif
		
        s->buf_pos += len;
     } while(!end);
     
     if (s->eof && ptr == mem) 
     {
         //mpDebugPrint("s->eof = 0x%x", s->eof);
		 return NULL;
     }	  
     
     if(max > 0) ptr[0] = 0;

     return mem;
}

int sub_pos_seek(DWORD ms, BOOL forward)
{
    MP_DEBUG2("sub_pos_seek ms = %d, forward = %d", ms, forward);
    if (forward)
    {
        //mpDebugPrint("g_SubNodeTail = 0x%x", g_SubNodeTail);
        while (g_SubNodeCur->s_sub.start < ms)
        {
            //mpDebugPrint("g_SubNodeCur = 0x%x, g_SubNodeCur->s_sub.start = %d, ms = %d", g_SubNodeCur, g_SubNodeCur->s_sub.start, ms);
            if (g_SubNodeCur->s_sub.displaying)
            {
                g_SubNodeCur->s_sub.displaying = FALSE;
#if DISPLAY_VIDEO_SUBTITLE
                if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateOsdSRT)
				    g_pXpgFuncPtr->xpgUpdateOsdSRT("");
#endif
            }	
			if (g_SubNodeCur != g_SubNodeTail)
               g_SubNodeCur = g_SubNodeCur->p_next;
			else
			   break;	
        } 
    }
	else // backward
	{
	    //BOOL match = FALSE;
	    while (g_SubNodeCur->s_sub.start > ms)
        {
            
            if (g_SubNodeCur->s_sub.displaying)
            {
                g_SubNodeCur->s_sub.displaying = FALSE;
#if DISPLAY_VIDEO_SUBTITLE
                if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateOsdSRT)
				    g_pXpgFuncPtr->xpgUpdateOsdSRT("");
#endif
            }	
			if (g_SubNodeCur != g_SubNodeHead)
                g_SubNodeCur = g_SubNodeCur->p_prev;
			else
				break;
			//mpDebugPrint("g_SubNodeCur = 0x%x, g_SubNodeCur->s_sub.start = %d, ms = %d", g_SubNodeCur, g_SubNodeCur->s_sub.start, ms);
        } 

		if ((g_SubNodeCur->s_sub.end < ms) && (g_SubNodeCur != g_SubNodeTail))
		   g_SubNodeCur = g_SubNodeCur->p_next;
	}
	
	return PASS;
}

/**
 * Find a newline character in buffer
 * \param buf buffer to search
 * \param len amount of bytes to search in buffer, may not overread
 * \param utf16 chose between UTF-8/ASCII/other and LE and BE UTF-16
 *              0 = UTF-8/ASCII/other, 1 = UTF-16-LE, 2 = UTF-16-BE
 */
static const uint8_t *sub_find_newline(const uint8_t *buf, int len, int utf16)
{    
    uint32_t c;
    const uint8_t *end = buf + len;
    switch (utf16) 
	{
       case 0:
           return (uint8_t *)memchr(buf, '\n', len);

	   case 1:
           while (buf < end - 1) 
		   {
               GET_UTF16(c, buf < end - 1 ? get_le16_inc(&buf) : 0, return NULL;)
               if (buf <= end && c == '\n')
                   return buf - 1;
           }
       break;
	   
       case 2:
           while (buf < end - 1) 
		   {
               GET_UTF16(c, buf < end - 1 ? get_be16_inc(&buf) : 0, return NULL;)
               if (buf <= end && c == '\n')
                   return buf - 1;
           }
       break;
    }
    return NULL;
}

/**
 * Copy a number of bytes, converting to UTF-8 if input is UTF-16
 * \param dst buffer to copy to
 * \param dstsize size of dst buffer
 * \param src buffer to copy from
 * \param len amount of bytes to copy from src
 * \param utf16 chose between UTF-8/ASCII/other and LE and BE UTF-16
 *              0 = UTF-8/ASCII/other, 1 = UTF-16-LE, 2 = UTF-16-BE
 */
static int sub_copy_characters(uint8_t *dst, int dstsize, 
								const uint8_t *src, int *len, E_CHAR_ENCODING utf16_src, E_CHAR_ENCODING utf16_dst)
{	
    uint32_t c;
    uint8_t *dst_end = dst + dstsize;
    const uint8_t *end = src + *len;

	
	MP_DEBUG1("%s", __func__);
	MP_DEBUG2("utf16_src=%d ,utf16_dst=%d",utf16_src,utf16_dst);
	#if 0
	MP_DEBUG2("dst= %d ,len= %d", dst,dstsize);
	MP_DEBUG1("dst_end = %d",dst_end);
	MP_DEBUG2("src= %d ,len= %d",src, len);
	MP_DEBUG1("end= %d",end);
	#endif
    switch (utf16_src) 
    {
        case CHAR_ENCODING_UTF_8: // UTF-8/ASCII/other
            MP_DEBUG("UTF-8");
			if (CHAR_ENCODING_UTF_8 == utf16_dst) // UTF-8/ASCII/other
            {
                if (*len > dstsize)
                   *len = dstsize;
                memcpy(dst, src, *len);
                return *len;
            }
			else if (CHAR_ENCODING_UTF_16_BE == utf16_dst) // UTF-16-BE
            {     
                src[*len] = '\0';
			    sub_Utf8ToUtf16Be(dst, src);
			    return(sub_wcslen(dst));
			}
			
        case CHAR_ENCODING_UTF_16_LE: // UTF-16-LE
       		MP_DEBUG("UTF-16-LE");
            if (CHAR_ENCODING_UTF_8 == utf16_dst) // UTF-8/ASCII/other
            {
                while (src < end - 1 && dst_end - dst > 8) 
			    {
                    uint8_t tmp;
                    GET_UTF16(c, src < end - 1 ? get_le16_inc(&src) : 0, ;)
                    PUT_UTF8(c, tmp, *dst++ = tmp;)
                }
                *len -= end - src;
                return dstsize - (dst_end - dst);
            }	
			else if (CHAR_ENCODING_UTF_16_BE == utf16_dst) // UTF-16-BE
            {
                src[*len] = '\0';
				src[*len+1] = '\0';
			    sub_Utf16LeToUtf16Be(dst, src);
			    return(sub_wcslen(dst)); 
			}
				

	   case CHAR_ENCODING_UTF_16_BE: // UTF-16-BE
	        MP_DEBUG("UTF-16-BE");
	   	    if (CHAR_ENCODING_UTF_8 == utf16_dst) // UTF-8/ASCII/other
            {
                while (src < end - 1 && dst_end - dst > 8) 
		        {
                   uint8_t tmp;
                   GET_UTF16(c, src < end - 1 ? get_be16_inc(&src) : 0, ;)
                   PUT_UTF8(c, tmp, *dst++ = tmp;)
                }
                *len -= end - src;
                return dstsize - (dst_end - dst);
	   	    }	
		    else if (CHAR_ENCODING_UTF_16_BE == utf16_dst) // UTF-16-BE
		    {
		        if (*len > dstsize)
                *len = dstsize;
                memcpy(dst, src, *len);
                return *len;
		    }	
    }
    return 0;
}


/*--------------------------------------------------------------------*/
/*                                         String handling                                                    */
/*--------------------------------------------------------------------*/
size_t sub_wcslen(const char* s)
{
	const char *save;

	if (s == 0)
		return 0;
	for (save = s; *save || *(save+1); ++save);
	return save-s;
}


char *sub_wcscat(char* s1, char* s2)
{
	char *cp;

	cp = s1;
	while (*cp != '\0' || *(cp+1) != '\0')
		cp++;
	while (*s2 != '\0' || *(s2+1) != '\0')
	{
	    *cp++ = *s2++;
	}		
    *cp = '\0';
	*(cp+1) = '\0';
	return s1;
}

/* Remove leading and trailing space */
void sub_trail_space(char *s) 
{
	int i = 0;
	while (isspace(s[i])) ++i;
	if (i) strcpy(s, s + i);
	i = strlen(s) - 1;
	while (i > 0 && isspace(s[i])) s[i--] = '\0';
}

/* Remove leading and trailing \r */
void sub_trail_return_char(char *s) 
{
	int i = 0;
	while (s[i] == '\r') ++i;
	if (i) strcpy(s, s + i);
	i = strlen(s) - 1;
	while (i > 0 && s[i] == '\r') s[i--] = '\0';
}


char *sub_stristr(const char *haystack, const char *needle) 
{
    int len = 0;
    const char *p = haystack;

    if (!(haystack && needle)) return NULL;

    len= strlen(needle);
    while (*p != '\0') 
	{
	    if (strncasecmp(p, needle, len) == 0) return (char*)p;
	    p++;
    }

    return NULL;
}

char* sub_strdup(char const* s)
{
	size_t siz = 0;
	char* result = NULL;

	siz = strlen(s) + 1;
	result = (char*) mem_malloc(siz);

	if (result) 
	{
		memcpy(result, s, siz);
		result[siz]='\0';
	}
	return result;
}

char* sub_strdup_utf8_to_utf16be(char const* str_utf_8)
{
	char* str_utf_16_be = (char*) mem_malloc(SUB_LINE_LEN+2);
	int len = strlen(str_utf_8);
	sub_copy_characters(str_utf_16_be, SUB_LINE_LEN+2, str_utf_8, &len, CHAR_ENCODING_UTF_8, CHAR_ENCODING_UTF_16_BE);
	 
	return str_utf_16_be;
}

int lrc_utf16_to_utf8(uint8_t* str_dst, int str_dst_len, const uint8_t* str_src, int* str_src_len, E_CHAR_ENCODING utf16_src, E_CHAR_ENCODING utf16_dst)
{
	MP_DEBUG1("%s", __func__);
	int new_str_dst_len = 0;
	//new_str_dst_len = sub_copy_characters((uint8_t*) str_dst, (int ) str_dst_len, (const uint8_t *) str_src, (int *) str_src_len,(E_CHAR_ENCODING) utf16_src, (E_CHAR_ENCODING) utf16_dst);
	new_str_dst_len = sub_copy_characters(str_dst, str_dst_len, str_src, str_src_len, utf16_src, utf16_dst);
	/*
	MP_DEBUG2("str_src_len= %d, str_dst_len = %d",str_src_len, str_dst_len);
	MP_DEBUG2("utf16_src=%d ,utf16_dst=%d", utf16_src,utf16_dst);
 	MP_DEBUG1("new_str_dst_len = %d",new_str_dst_len);
 	*/
	return new_str_dst_len;
}



/*--------------------------------------------------------------------------*/
/*                                                        Unicode                                                        */
/*--------------------------------------------------------------------------*/
static inline int8_t U_FAILURE(UErrorCode code) 
{ 
    return (int8_t)(code>U_ZERO_ERROR); 
}

#if 0
static E_CHAR_ENCODING sub_detectUnicodeSignature(const char* source,
                             int32_t sourceLength,
                             int32_t* signatureLength,
                             UErrorCode* pErrorCode) {
    int32_t dummy;

    /* initial 0xa5 bytes: make sure that if we read <SIG_MAX_LEN
     * bytes we don't misdetect something 
     */
    char start[SIG_MAX_LEN]={ '\xa5', '\xa5', '\xa5', '\xa5', '\xa5' };
    int i = 0;

    if ((pErrorCode==NULL) || U_FAILURE(*pErrorCode))
	{
        return NULL;
    }
    
    if (source == NULL || sourceLength < -1)
	{
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return NULL;
    }

    if (signatureLength == NULL) 
	{
        signatureLength = &dummy;
    }

    if (sourceLength==-1)
	{
        sourceLength=(int32_t) strlen(source);
    }

    
    while (i<sourceLength&& i<SIG_MAX_LEN)
	{
        start[i]=source[i];
        i++;
    }

    if (start[0] == '\xFE' && start[1] == '\xFF') 
	{
        *signatureLength = 2;
        return CHAR_ENCODING_UTF_16_BE;
    } 
	else if (start[0] == '\xFF' && start[1] == '\xFE') 
	{
        if (start[2] == '\x00' && start[3] =='\x00') 
		{
            *signatureLength = 4;
            return CHAR_ENCODING_UTF_32_LE;
        } 
		else 
		{
            *signatureLength = 2;
            return CHAR_ENCODING_UTF_16_LE;
        }
    } 
	else if (start[0] == '\xEF' && start[1] == '\xBB' && start[2] == '\xBF') 
	{
        *signatureLength = 3;
        return CHAR_ENCODING_UTF_8;
    } 
	else if (start[0] == '\x00' && 
		     start[1] == '\x00' && 
             start[2] == '\xFE' && 
             start[3] == '\xFF'
            ) 
    {
        *signatureLength = 4;
        return CHAR_ENCODING_UTF_32_BE;
    } 
	else if(start[0] == '\x0E' && start[1] == '\xFE' && start[2] == '\xFF') 
	{
        *signatureLength = 3;
        return CHAR_ENCODING_SCSU;
    } 
	else if(start[0] == '\xFB' && start[1] == '\xEE' && start[2] == '\x28') 
	{
        *signatureLength = 3;
        return CHAR_ENCODING_BOCU_1;
    } 
	else if (start[0] == '\x2B' && start[1] == '\x2F' && start[2] == '\x76') 
	{
        /*
         * UTF-7: Initial U+FEFF is encoded as +/v8  or  +/v9  or  +/v+  or  +/v/
         * depending on the second UTF-16 code unit.
         * Detect the entire, closed Unicode mode sequence +/v8- for only U+FEFF
         * if it occurs.
         *
         * So far we have +/v
         */
        if (start[3] == '\x38' && start[4] == '\x2D') 
		{
            /* 5 bytes +/v8- */
            *signatureLength = 5;
            return CHAR_ENCODING_UTF_7;
        } else if(start[3] == '\x38' || start[3] == '\x39' || start[3] == '\x2B' || start[3] == '\x2F') {
            /* 4 bytes +/v8  or  +/v9  or  +/v+  or  +/v/ */
            *signatureLength = 4;
            return CHAR_ENCODING_UTF_7;
        }
    } 
	else if(start[0]=='\xDD' && start[1]== '\x73'&& start[2]=='\x66' && start[3]=='\x73')
	{
        *signatureLength=4;
        return CHAR_ENCODING_UTF_EBCDIC;
    }


    /* no known Unicode signature byte sequence recognized */
    *signatureLength=0;
    return NULL;
}
#endif

/**
 * Helper function to read/write 16 bits little-endian and advance pointer
 */
static uint8_t get_8_inc(const uint8_t **buf)
{
    uint8_t v = *buf;
    *buf += 1;
    return v;
}


/**
 * Helper function to read/write 16 bits little-endian and advance pointer
 */
static uint16_t get_le16_inc(const uint8_t **buf)
{
    uint16_t v = AV_RL16(*buf);
    *buf += 2;
    return v;
}

static void set_le16_inc(uint16_t v, uint8_t **buf)
{
    AV_WL16(*buf, v);
    *buf += 2;
}

/**
 * Helper function to read/write 16 bits big-endian and advance pointer
 */
static uint16_t get_be16_inc(const uint8_t **buf)
{
    uint16_t v = AV_RB16(*buf);
    *buf += 2;
    return v;
}

static void set_be16_inc(uint16_t v, uint8_t **buf)
{
    AV_WB16(*buf, v);
    *buf += 2;
}

void sub_Utf8ToUtf16Be(uint8_t* target, uint8_t* source)
{       
	mpx_UtilUtf8ToUnicodeU16((uint16_t *)target, source);
}

void sub_Utf16LeToUtf16Be(uint8_t* target, uint8_t* source)
{    
	int i;
	for (i=0; i<sub_wcslen(source); i=i+2)
	{
	    target[i] = source[i+1];
		target[i+1] = source[i];
	}	
}

int sub_clz(unsigned int  x)
{
    int i;
	int count = 0;
	for (i=0; i<32; i++)
	{
	    int shift = 32 - i - 1;
	    if ((x >> shift) == 0)
	    {
	        count++;
	    }
		else
		{
		    break;
		}
	}
	return count;
}
#endif

