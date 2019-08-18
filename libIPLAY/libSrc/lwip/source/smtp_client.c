/* Note: This SMTP client code is mainly ported and mimicked from the open-source mailsend v1.14 package which 
 *       is a simple and basic SMTP client and doesn't support SMTP AUTH command nor ESMTP EHLO command. 
 *       I extended it to support EHLO and SMTP AUTH LOGIN method. Finally, OpenSSL is applied to secure it.
 *
 *       Created: WeiChing 2009.03
 *
 *       Note: Some advanced features of ESMTP commands and Authentication Methods are not supported. 
 *
 */

#ifndef _SMTP_C_ 
#define _SMTP_C_ 

#define LOCAL_DEBUG_ENABLE  0 


#include <string.h>

#include <linux/types.h>
#include "global612.h"
#include "mpTrace.h"

#include "typedef.h"
#include "net_packet.h"
#include "socket.h"
#include "net_socket.h"
#include "net_ns.h"
#include "net_device.h"
#include "net_netdb.h"
#include "linux/list.h"
#include "../../wpa_supplicant/include/os.h"
#include "../../wpa_supplicant/include/base64.h"
#include "clock.h"  /* for clock_time() */
#include "ndebug.h"

#include "openssl/rsa.h"
#include "openssl/crypto.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#include "smtp.h"

static char *rawfile_data_buff, *mime_tmpfile_buff;
static int  rawfile_len, mime_tmpbuff_len;

static char send_buf[SMTP_SEND_BUF_LEN], recv_buf[SMTP_RECV_BUF_LEN];

static int server_support_ESMTP, server_support_AUTH, server_support_AUTH_LOGIN;

static SLIST_TYPE *attachment_list_head, *addr_list_head;

static char g_charset[33];

#define MAX_ADDR_STR_LEN  128 


#define TASK_YIELD_IN_PROCESSING_LOOP   1  /* performance issue: TaskYield() in loop is better ?? */


/* borrowed from pop3_main.c for OpenSSL usage */
typedef struct ppp_connect
{
    SSL_CTX* ctx;
    SSL*     handle;
    X509*    server_cert;
    u32      flags;
#define FLAG_NO_SSL  0x00
#define FLAG_SSL     0x01
} SSL_ConnectCTX; /* use a better type naming */

extern int SSL_init(void);
extern int SSL_close(SSL_ConnectCTX *conn);
extern void pop3_seed(void);

static SSL_ConnectCTX smtp_ssl_conn;


#define CHECK_MALLOC(x) \
do \
{ \
    if (x == NULL) \
    { \
        MP_ALERT("%s (%d) - Memory allocation failed!", __FILE__, __LINE__); \
    } \
} while(0)

#define MUTILS_CHECK_MALLOC(p) \
do \
{ \
    if (p == NULL) \
    { \
        MP_ALERT("%s (%d) - memory allocation problem!", __FILE__, __LINE__); \
        goto ExitProcessing; \
    } \
} while(0)


#define smtp_malloc(sz)   mm_malloc(sz)
#define smtp_mfree(ptr)   mm_free(ptr)

static BOOL emptyList(SLIST_TYPE *list);

/* note: this function is not intended to release dynamically allocated memory buffers */
static void data_init_clean(void)
{
	server_support_ESMTP = server_support_AUTH = server_support_AUTH_LOGIN = 0;

    attachment_list_head = addr_list_head = NULL;

    rawfile_data_buff = mime_tmpfile_buff = NULL;
    rawfile_len = mime_tmpbuff_len = 0;

    memset(send_buf, 0, SMTP_SEND_BUF_LEN);
    memset(recv_buf, 0, SMTP_RECV_BUF_LEN);    

    memset(g_charset, 0, sizeof(g_charset));
    strcpy(g_charset, DEFAULT_CHARSET);

    memset(&smtp_ssl_conn, 0, sizeof(SSL_ConnectCTX));
    return;
}


/*
**  allocateNode() - allocate a new node.
**
**  Parameters:
**  void    *data       a generic pointer to object data
**
**  Return Values:
**  pointer to SLIST_TYPE if succeeds, NULL otherwise
*/
static SLIST_TYPE *allocateNode(void *data)
{
    SLIST_TYPE *list = (SLIST_TYPE *) smtp_malloc(sizeof(SLIST_TYPE));

    if (list == NULL)
    {
        MP_ALERT("allocateNode(): memory allocation failed");
        return(NULL);
    }

    list->data = data;
    list->next = NULL;
    return(list);
}

/*
**  appendNode() - appends a node to the end of a list
**
**  Parameters:
**  SLIST_TYPE **head      - modify the list
**  SLIST_TYPE **new_node  - appends this node
*/
static void appendNode(SLIST_TYPE **head, SLIST_TYPE **new_node)
{
    SLIST_TYPE *tmp;

    if (emptyList(*head) == TRUE)
        (*head) = (*new_node);
    else
    {
        for (tmp=(*head); tmp->next != NULL; tmp=tmp->next)
            ;
        tmp->next = (*new_node);
    }
}

/* emptyList() - check if a list variable is NULL */
static BOOL emptyList(SLIST_TYPE *list)
{
    return((list == NULL)?TRUE:FALSE);
}


U32 AddrToIP(char *addr_str)
{
    struct hostent *host;
    U32  ip_addr = 0x00000000;

    if (! isdigit(addr_str[0]))
    {
        struct in_addr *curr;

        host = gethostbyname(addr_str);
        if(host == NULL)
        {
            MP_DEBUG("AddrToIP(): gethostbyname() can't find host.");
            return(0);
        }
        if (curr = (struct in_addr *)host->h_addr_list[0])
            ip_addr = ntohl(curr->s_addr);
    } else
        ip_addr = inet_addr(addr_str);

    return(ip_addr);
}


/* connect to SMTP server and returns the socket fd */
static SOCKET_ID smtpConnect(U32 ip_addr, int port)
{
    SOCKET_ID sock_id;

    sock_id = mpx_DoConnect(ip_addr, port, TRUE);
    if(sock_id < 0)
    {
        MP_ALERT("smtpConnect(): failed to connect to SMTP server (ip = 0x%x) at port %d", ip_addr, port);
        return(INVALID_SOCKET);
    }

    if(smtp_ssl_conn.flags == FLAG_SSL)
    {
        SSL_METHOD *req_method = NULL;
        int err;
    
        SSL_init();

        pop3_seed();

        req_method = SSLv23_client_method();

        smtp_ssl_conn.ctx = SSL_CTX_new(req_method);
        if(! smtp_ssl_conn.ctx)
        {
            MP_ALERT("smtpConnect(): SSL_CTX_new() failed to create a context!");
            closesocket(sock_id);
            return(INVALID_SOCKET);
        }

        /* OpenSSL contains code to work-around lots of bugs and flaws in various
           SSL-implementations. SSL_CTX_set_options() is used to enabled those
           work-arounds. The man page for this option states that SSL_OP_ALL enables
           all the work-arounds and that "It is usually safe to use SSL_OP_ALL to
           enable the bug workaround options if compatibility with somewhat broken
           implementations is desired."
        */
        SSL_CTX_set_options(smtp_ssl_conn.ctx, SSL_OP_ALL);

        smtp_ssl_conn.handle = SSL_new(smtp_ssl_conn.ctx);
        if(! smtp_ssl_conn.handle)
        {
            MP_ALERT("smtpConnect(): SSL_new() failed to create a context (handle)!");
            SSL_close(&smtp_ssl_conn);
            closesocket(sock_id);
            return(INVALID_SOCKET);
        }

        SSL_set_connect_state(smtp_ssl_conn.handle);

        /* pass the raw socket into the SSL layers */
        if (! SSL_set_fd(smtp_ssl_conn.handle, sock_id))
        {
            MP_ALERT("smtpConnect(): SSL_set_fd() failed: %s", ERR_error_string(ERR_get_error(), NULL));
            SSL_close(&smtp_ssl_conn);
            closesocket(sock_id);
            return(INVALID_SOCKET);
        }

        err = SSL_connect(smtp_ssl_conn.handle);
        if(err != 1)
        {
            MP_ALERT("smtpConnect(): SSL_connect() failed to set up connection!");
            SSL_close(&smtp_ssl_conn);
            closesocket(sock_id);
            return(INVALID_SOCKET);
        }
    }

    return(sock_id);
}


/* sockWrite() - writes a character string out to a socket */
static int sockWrite(SOCKET_ID sock, char *str, size_t count)
{
    size_t bytesSent = 0;
    int thisWrite;

    while (bytesSent < count)
    {
        if(smtp_ssl_conn.flags == FLAG_SSL)
            thisWrite = SSL_write(smtp_ssl_conn.handle, str, count - bytesSent);
        else
            thisWrite = send(sock, str, count - bytesSent, 0);

        if (thisWrite <= 0)
            return(-1);

        bytesSent += thisWrite;
        str += thisWrite;
    }
    return(count);
}

static int sockPuts(SOCKET_ID sock, char *str)
{
    return(sockWrite(sock, str, strlen(str)));
}

static int sockGets(SOCKET_ID sockfd, char *str, size_t count)
{
    int bytesRead;
    int totalCount = 0;
    char buf[1], *currentPosition;
    char lastRead = 0;

    currentPosition = str;

    while (lastRead != ASCII_LF)
    {
        bytesRead = recv(sockfd, buf, 1, 0);
        if (bytesRead <= 0)
        {
            /* the other side may have closed unexpectedly */
            return(-1);
        }
        lastRead = buf[0];

        if ((totalCount < count) && (lastRead != ASCII_LF) && (lastRead != ASCII_CR))
        {
            *currentPosition = lastRead;
            currentPosition++;
            totalCount++;
        }
    }
    if (count > 0)
        *currentPosition = 0;

    return(totalCount);
}


static char base64_chars[64] = 
{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
  'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
  't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', '+', '/'
};

/*
**  mutilsGenerateMIMEBoundary() - generate unique string for boundary for MIME tag
**
**  Parameters:
**      char *boundary - NULL terminated boundary string - returns
**      int len        - size of boundary
**
**  Limitations and Comments:
**      boundary must have len bytes in it to store the boundary. the function
**      calls rand() for random number, so the caller should call srand()
**      before calling this function.
*/
#define BOUNDARY_LEN    16 
static void mutilsGenerateMIMEBoundary(char *boundary, int len)
{
    char *p;
    int i;

    memset(boundary, 0, len);
    p = boundary;
    for (i=0; i < BOUNDARY_LEN; i++)
    {
        if (i >= (len-1))
            break;
        *p++ = base64_chars[rand() % sizeof(base64_chars)];
    }
    *p='\0';
}

/* duplicate a string (memory allocated), caller is responsible to free it */
static char *xStrdup(char *string)
{
    char *tmp;

    if (string == NULL || *string == '\0')
        return(NULL);

    tmp = smtp_malloc(strlen(string)+1);
    if (tmp == NULL)
    {
        MP_ALERT("xStrdup(): memory allocation failed");
    }

    /* it's safe to copy this way */
    strcpy(tmp, string);
    return(tmp);
}

/* Initialize Address structure. returns pointer to the Address sturcture on success, NULL on failure. */
static Address *newAddress(void)
{
    Address *addr = (Address *) smtp_malloc(sizeof(Address));
    if (addr == NULL)
    {
        MP_ALERT("newAddress(): memory allocation failed");
        return(NULL);
    }
    memset(addr, 0, sizeof(Address));
    return(addr);
}

static int validateSmtpMusts(char *from, char *smtp_server, char *helo_domain)
{
    int err = 0;
    SLIST_TYPE *addrlist = getAddressList();

    if (addrlist == NULL)
    {
        MP_ALERT("Invalid: no To address specified");
        err++;
    }

    if (from == NULL || from[0] == '\0')
    {
        MP_ALERT("Invalid: no From address specified");
        err++;
    }

    if (smtp_server == NULL || smtp_server[0] == '\0')
    {
        MP_ALERT("Invalid: no SMTP server address specified");
        err++;
    }

    if (helo_domain == NULL || helo_domain[0] == '\0')
    {
        MP_ALERT("Invalid: no Domain specified");
        err++;
    }

    if (err)
        return(-1);
    else
        return(0);
}

/*
 * return filepath and mime_type
 * @param str   String of the form "file,mime_type" 
 *              example: "c:\usr\local\foo.txt,text/plain"
 *                       "/usr/local/foo.html,text/html"
 * @param filepath returns
 * @param fp_size  size of the buffer filepath
 * @param mime_type returns
 * @param mt_size size of the buffer mime_type
 *
 * @return 0 on success, -1 otherwise
 */
static int get_filepath_mimetype(char *str, char *filepath, int fp_size, char *mype_type, int mt_size)
{
    int rc = 0;
    char *fp, *mt;

    if ((mt = strchr(str, ATTACHMENT_FILE_MIMETYPE_SEP)))
    {
        *mt++ = '\0';
    }
    else
    {
        MP_DEBUG("get_filepath_mimetype(): could not determine mime-type from input: %s", str);
        return(-1);
    }
    mutilsStrncpy(mype_type, mt, mt_size);

    /* get the filepath out */
    fp = str;
    mutilsStrncpy(filepath, fp, fp_size);

    return(rc);
}

/*
 * Calculate Date for RFC822 Date: header
 *
 * @param when     time since unix epoch
 * @param datebuf  returns NULL terminated date string.
 *                  Example: Wed, 17 May 2006 13:55:35 -0400
 * @param bufsiz   size of buffer dates. It's callers responsibily to make sure 
 *                 datebuf contains enough space. It must be at least 64 bytes. 
 *                 Usually it'll contain 31 bytes.
 *
 * returns 0 on success, -1 on failure
 */
static int rfc822_date(time_t when, char *datebuf, int bufsiz)
{
#define DAY_MIN     (24 * HOUR_MIN)   /* minutes in a day */
#define HOUR_MIN    60                /* minutes in an hour */
#define MIN_SEC     60                /* seconds in a minute */

    char ts1[32], ts2[32];
    struct tm  *lt;
    struct tm  gmt;
    int gmtoff;

    if (bufsiz < 64)
    {
        MP_DEBUG("rfc822_date(): buffer size of date must be > 31, it is %d", bufsiz);
        return(-1);
    }

    memset(datebuf, 0, bufsiz);
    memset(ts1, 0, sizeof(ts1));
    memset(ts2, 0, sizeof(ts2));

    gmt = *gmtime(&when);
    lt = localtime(&when);

    gmtoff = (lt->tm_hour - gmt.tm_hour) * HOUR_MIN + lt->tm_min - gmt.tm_min;

    if (lt->tm_year < gmt.tm_year)
        gmtoff -= DAY_MIN;
    else if (lt->tm_year > gmt.tm_year)
        gmtoff += DAY_MIN;
    else if (lt->tm_yday < gmt.tm_yday)
        gmtoff -= DAY_MIN;
    else if (lt->tm_yday > gmt.tm_yday)
        gmtoff += DAY_MIN;

    if (lt->tm_sec <= gmt.tm_sec - MIN_SEC)
        gmtoff -= 1;
    else if (lt->tm_sec >= gmt.tm_sec + MIN_SEC)
        gmtoff += 1;

    /* day 1 - 9 can be written as 01 - 09 */
    strftime(ts1, sizeof(ts1)-1, "%a, %d %b %Y %H:%M:%S ", lt);
    if (gmtoff < -DAY_MIN || gmtoff > DAY_MIN)
    {
        MP_DEBUG("rfc822_date(): UTC time offset %d is larger than one day", gmtoff);
        return(-1);
    }

    snprintf(ts2, sizeof(ts2)-1, "%+03d%02d", (int) (gmtoff / HOUR_MIN), (int) (abs(gmtoff) % HOUR_MIN));

    /* put everything in the buffer. it's usually 31 bytes */
    snprintf(datebuf, bufsiz, "%s%s", ts1, ts2);

    /* not add timezone name, it's not required */

    return(0);
}


/*
 * Free memory associated with tokens
 * @param tokens    The tokens to free
 * @param ntokens   Number of tokens in tokens
 */
static void mutilsFreeTokens(char **tokens, int ntokens)
{
    int i;

    /* free memory allocated for each token */
    for (i=0; i < ntokens; i++)
    {
        if (tokens[i])
            smtp_mfree(tokens[i]);
    }

    /* free the tokens itself */
    if (tokens)
        smtp_mfree(tokens);
}

/*
 * Tokenizes a string separated by delimiter
 * @param str       The string to tokenize
 * @param delip     The delimeter e.g. ' ' 
 * @param ntoken    Number of tokens in the string (returns)
 * @return tokens   on success, NULL on failure
 * 
 * The caller should free the tokens by calling mutilsFreeTokens(tokens, ntokens)
 * Note: A token can be of MUTILS_MAX_TOKEN_LEN long
 */
static char **mutilsTokenize(char *str, int delim, int *ntokens)
{
    char tbuf[MUTILS_MAX_TOKEN_LEN], **tokens = NULL;
    int i = 0, j = 0, count = 0, allocated = 0;
    char *p;

    *ntokens = 0;
    if (str == NULL || *str == '\0')
        return(NULL);

    /* count how many token there first */
    for (p=str; *p; p++)
    {
        if (*p == delim)
        {
            count++;
        }
    }
    count++;
    /* allocate memory for tokens */
    tokens = (char **) smtp_malloc(count * sizeof(char *));
    MUTILS_CHECK_MALLOC(tokens);

    allocated = 0;
    for (i=0; i < count; i++)
    {
        /* allocate memory for each token string, a token can be of MUTILS_MAX_TOKEN_LEN characters long maximum */
        tokens[i] = (char *) smtp_malloc(MUTILS_MAX_TOKEN_LEN*sizeof(char));
        MUTILS_CHECK_MALLOC(tokens[i]);
        allocated++;
    }

    j = 0;
    count = 0;
    for (p=str; *p; p++)
    {
        if (*p != delim && *p != '\0')
        {
            if (j >= MUTILS_MAX_TOKEN_LEN)
            {
                MP_DEBUG("mutilsTokenize(): Buffer overflow detected");
                /* buffer overfow */
                goto ExitProcessing;
            }
            tbuf[j++] = *p;
        }
        else
        {
            /* we're in a new token */
            tbuf[j++] = '\0';
            strcpy(tokens[count], tbuf);
            count++;
            j = 0;
        }
    }
    if (j > 0 && j < MUTILS_MAX_TOKEN_LEN)
    {
        tbuf[j] = '\0';
        mutilsStrncpy(tokens[count], tbuf, MUTILS_MAX_TOKEN_LEN-1);
    }

    count++;

    *ntokens = count;
    return(tokens);

ExitProcessing:
    mutilsFreeTokens(tokens, allocated);

    return(NULL);
}


/* add the file to attachment list */
static int add_attachment_to_list(char *file_path_mime)
{
    int ntokens;
    SLIST_TYPE *node = NULL;
    char **tokens = NULL,
         *file_path = NULL,
         *file_name = NULL,
         *mime_type = NULL,
         *content_disposition = "attachment";

    char **files_info = NULL;
    int nfiles = 0, i;
    Attachment *attach = NULL;

    if (file_path_mime == NULL || *file_path_mime == '\0')
        return(-1);

    /* Tokenize the string of the form "file1;file2;file3;..." i.e. seperated by ';' char */
    files_info = mutilsTokenize(file_path_mime, ATTACHMENT_FILE_LIST_SEP ,&nfiles);
    if (files_info == NULL || nfiles == 0)
    {
        MP_DEBUG("add_attachment_to_list(): could not parse attachment string: \"%s\"", file_path_mime);
        return(-1);
    }

    for(i=0; i < nfiles; i++)
    {
        tokens = file_path = file_name = NULL;

        /* Tokenize the string of a single file of "file,mime_type,something" format */
        tokens = mutilsTokenize(files_info[i], ',' ,&ntokens);
        if (tokens == NULL)
        {
            MP_DEBUG("add_attachment_to_list(): could not parse attachment string: \"%s\"", files_info[i]);
            return(-1);
        }

        /* get the file name out */
        file_path = tokens[0];
        if ((file_name = strrchr(file_path, '/')) || (file_name = strrchr(file_path, '\\')))
            file_name++;
        else
            file_name = file_path;

        attach = (Attachment *) smtp_malloc(sizeof(Attachment));
        CHECK_MALLOC(attach);

        attach->file_path = xStrdup(file_path);
        attach->file_name = xStrdup(file_name);

        switch (ntokens)
        {
            case 1: /* Only File_path/name given */
                mime_type = "application/octet-stream";
                attach->mime_type = xStrdup(mime_type);
                attach->content_disposition = xStrdup("attachment");
                break;

            case 2: /* filepath/name, mime_type given */
                mime_type = tokens[1];
                attach->mime_type = xStrdup(mime_type);
                attach->content_disposition = xStrdup("attachment");
                break;

            case 3:
                mime_type = tokens[1];
                attach->mime_type = xStrdup(mime_type);
                content_disposition = tokens[2];
                if (*content_disposition == 'a')
                    attach->content_disposition = xStrdup("attachment");
                else if (*content_disposition == 'i')
                    attach->content_disposition = xStrdup("inline");
                else
                    attach->content_disposition = xStrdup("attachment");
                break;

            default:
                MP_DEBUG("add_attachment_to_list(): invalid string specified: \"%s\"", files_info[i]);
                return(-1);
        }

        node = allocateNode((void *) attach);
        CHECK_MALLOC(node);
        appendNode(&attachment_list_head, &node);

        mutilsFreeTokens(tokens, ntokens);
    }
    mutilsFreeTokens(files_info, nfiles);

    return(0);
}

static int addAddressToList(char *addr_str, char *label)
{
    char *p;
    SLIST_TYPE *new_node = NULL;
    Address *address = NULL;

    if (addr_str == NULL || *addr_str == '\0')
        return(-1);

    if (label == NULL || *label == '\0')
        label = "To";

    /* if comma separated, tokenize them */
    p = mutilsStrtok(addr_str, ADDRESS_SEP_STR);
    if (p != NULL)
    {
        address = newAddress();
        if (address == NULL)
        {
            MP_ALERT("addAddressToList(): memory allocation problem for newAddress()");
            return(-1);
        }

        address->label = xStrdup(label);
        address->address = xStrdup(p);

        new_node = allocateNode((void *) address);
        if (new_node == NULL)
        {
            MP_ALERT("addAddressToList(): memory allocation problem with allocateNode()");
            smtp_mfree(address);
            return(-1);
        }
        appendNode(&addr_list_head, &new_node);
    }

    while ((p = mutilsStrtok(NULL, ADDRESS_SEP_STR)) != NULL)
    {
        if (p != NULL)
        {
            address = newAddress();
            if (address == NULL)
            {
                MP_ALERT("addAddressToList(): memory allocation problem for newAddress()");
                return(-1);
            }
            address->label = xStrdup(label);
            address->address = xStrdup(p);
            new_node = allocateNode((void *) address);
            if (new_node == NULL)
            {
                MP_ALERT("addAddressToList(): memory allocation problem with allocateNode()");
                smtp_mfree(address);
                return(-1);
            }
            appendNode(&addr_list_head, &new_node);
        }
    }

    return(1);
}

static SLIST_TYPE *getAddressList(void)
{
    return(addr_list_head);
}

static SLIST_TYPE *get_attachment_list(void)
{
    return(attachment_list_head);
}

/* mutilsStrncpy() - copy at most n characters of string src to dst */
static char *mutilsStrncpy(char *dst, char *src, int n)
{
    register char *dscan;
    register char *sscan;
    register int count;

    dscan = dst;
    sscan = src;
    count = n;

    while (--count >= 0 && (*dscan++ = *sscan++) != '\0')
        continue;

    while (--count >= 0)
        *dscan++ = '\0';
    return(dst);
}

/*
**  mutilsStrcasecmp() - case in-sensitive string comparison
**
**  Return Values:
**  < 0 if a < b
**  > 0 if a > b
**  0   if a = b
*/
static int mutilsStrcasecmp(char *a, char *b)
{
    register char ac, bc;

    for(;;)
    {
        ac = *a++;
        bc = *b++;

        if(ac == 0)
        {
            if(bc == 0)
                return(0);
            else
                return(-1);
        }
        else
        {
            if(bc == 0)
                return(1);
            else
            {
                if(ac != bc)
                {
                    if(islower(ac)) ac = toupper(ac);
                    if(islower(bc)) bc = toupper(bc);
                    if(ac != bc)
                        return(ac - bc);
                }
            }
        }
    }
}

/*
**  mutilsStrncasecmp() - case in-sensitive comparison wth first n bytes of fist string
**
**  Return Values:
**  < 0 if first n bytes of a < b
**  > 0 if first n bytes of a > b
**  0   if first n bytes of a = b
*/
static int mutilsStrncasecmp(char *s1, char *s2, int n)
{
    register char *scan1, *scan2;
    int count;

    scan1 = s1;
    scan2 = s2;
    count = n;

    while (--count >= 0 && *scan1 != '\0' && tolower(*scan1) == tolower(*scan2))
    {
        scan1++;
        scan2++;
    }
    if (count < 0)
        return(0);

    return(tolower(*scan1) - tolower(*scan2));
}

static char *mutilsStrtok(char *s, char *delim)
{
    register char *spanp;
    register int c, sc;
    char *tok;
    static char *last;

    if (s == NULL && (s = last) == NULL)
        return(NULL);

     /* Skip (span) leading delimiters (s += strspn(s, delim), sort of). */
cont:
    c = *s++;
    for (spanp = delim; (sc = *spanp++) != 0;)
    {
        if (c == sc)
            goto cont;
    }

    if (c == 0) /* no non-delimiter characters */
    {
        last = NULL;
        return(NULL);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;)
    {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c)
            {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = '\0';
                last = s;
                return(tok);
            }
        } while (sc != 0);
    }
}


/* read SMTP response. returns number of received bytes on success, -1 on failure */
static int smtpResponse(int sock_id)
{
    int bytesRead;

    memset(recv_buf, 0, SMTP_RECV_BUF_LEN);

    if(smtp_ssl_conn.flags == FLAG_NO_SSL)
#if 0 //original code
        bytesRead = sockGets(sock_id, recv_buf, SMTP_RECV_BUF_LEN-1);
#else //enhance performance
        bytesRead = recv(sock_id, recv_buf, SMTP_RECV_BUF_LEN-1, 0);
#endif
    else
        bytesRead = SSL_read(smtp_ssl_conn.handle, recv_buf, SMTP_RECV_BUF_LEN-1);

    if (bytesRead <= 0)
    {
        /* the other side may have closed unexpectedly */
        return(-1);
    }

    MP_DEBUG("smtpResponse(): %s", recv_buf);

    /* check SMTP 3-digit response code. "4xx" or "5xx" code means error, we do not further check those cases */
    if ((*recv_buf == '1' || *recv_buf == '2' || *recv_buf == '3') && 
        (*(recv_buf+3) == CHAR_SPACE || *(recv_buf+3) == '-'))
        return(bytesRead);
    else
        return(-1);
}

/* SMTP: EHLO preferred, if server not support it, use HELO instead */
static int smtpHelo(int sock_id, char *helo_domain)
{
    int rc;
    char *ptr;
    BOOL LastLine_found;
    
    /* read off the server greeting */
    rc = smtpResponse(sock_id);
    if(rc == -1)
        return(-1);

    if(mutilsStrncasecmp(recv_buf, "220", 3) != 0)
    {
        MP_ALERT("smtpHelo(): SMTP server is not ready!");
        return(-1);
    }

    memset(send_buf, 0, SMTP_SEND_BUF_LEN);
    snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "EHLO %s\r\n", helo_domain);
    MP_DEBUG("smtpHelo(): >>> %s", send_buf);
    sockPuts(sock_id, send_buf);

    /* check multiple response lines from ESMTP server */
    LastLine_found = FALSE;
    do {
        rc = smtpResponse(sock_id);
        if(rc == -1)
        {
            /* server not support EHLO command, send HELO instead */
            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "HELO %s\r\n", helo_domain);
            MP_DEBUG("smtpHelo(): >>> %s", send_buf);
            sockPuts(sock_id, send_buf);

            return(smtpResponse(sock_id));
        }

        if(mutilsStrncasecmp(recv_buf, "250", 3) == 0)
        {
            server_support_ESMTP = 1; /* server supports EHLO command */

            /* check SMTP AUTH methods that server supported */
            strtoupper(recv_buf);
            if((ptr = strstr(recv_buf+3, "AUTH")) != NULL)
            {
                server_support_AUTH = 1; /* server supports AUTH command */

                if(strstr(ptr, "LOGIN") != NULL)
                    server_support_AUTH_LOGIN = 1;
            }

            if((ptr = strstr(recv_buf, "250 ")) != NULL)
                LastLine_found = TRUE;
        }
    } while ((rc != -1) && (*(recv_buf+3) == '-') && (LastLine_found == FALSE));

    return (rc);
}

/* SMTP: AUTH LOGIN */
static int smtpAuthLogin(int sock_id)
{
    memset(send_buf, 0, SMTP_SEND_BUF_LEN);
    snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "AUTH LOGIN\r\n");
    MP_DEBUG("smtpAuthLogin(): >>> %s", send_buf);

    sockPuts(sock_id, send_buf);
    return(smtpResponse(sock_id));
}

/* SMTP: (Base64 encoded user info) */
static int smtpAuth_B64_UserInfo(int sock_id, const char *plain_user_info)
{
    char *b64_enc_str = NULL;
    int  b64_enc_len = 0;

    b64_enc_str = base64_encode(plain_user_info, strlen(plain_user_info), &b64_enc_len);
    if(b64_enc_str == NULL || b64_enc_len == 0)
    {
        MP_ALERT("smtpAuth_B64_UserInfo(): base64 encode failed !");
        return(-1);
    }
    b64_enc_str[b64_enc_len-1] = '\0'; /* note: it will get authentication failed from server, if without this line! */
    MP_DEBUG("smtpAuth_B64_UserInfo(): base64 encoded ==> %s  ,length=%d", b64_enc_str, b64_enc_len);

    memset(send_buf, 0, SMTP_SEND_BUF_LEN);
    snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "%s\r\n", b64_enc_str);
    MP_DEBUG("smtpAuth_B64_UserInfo(): >>> %s", send_buf);

    sockPuts(sock_id, send_buf);
    base64_mfree(b64_enc_str); /* To be consistent with the malloc function in base64_en/decode */
    return(smtpResponse(sock_id));
}

/* SMTP: MAIL FROM */
static int smtpMailFrom(int sock_id, char *from)
{
    memset(send_buf, 0, SMTP_SEND_BUF_LEN);
    snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "MAIL FROM: <%s>\r\n", from);
    MP_DEBUG("smtpMailFrom(): >>> %s", send_buf);

    sockPuts(sock_id, send_buf);
    return(smtpResponse(sock_id));
}

/* SMTP: QUIT */
static int smtpQuit(int sock_id)
{
    MP_DEBUG("smtpQuit(): QUIT");

    sockPuts(sock_id, "QUIT\r\n");
    return(smtpResponse(sock_id));
}

/* SMTP: RSET */
/* aborts current mail transaction and cause both ends to reset */
static int smtpRset(int sock_id)
{
    MP_DEBUG("smtpRset(): RSET");

    sockPuts(sock_id, "RSET\r\n");
    return(smtpResponse(sock_id));
}

/* SMTP: RCPT TO */
static int smtpRcptTo(int sock_id)
{
    SLIST_TYPE *list, *addrlist;
    Address *addr;

    addrlist = getAddressList();
    for (list=addrlist; list; list=list->next)
    {
        addr = (Address *) list->data;
        if (! addr)
            return(-1);
        if (! addr->address)
            return(-1);

        memset(send_buf, 0, SMTP_SEND_BUF_LEN);
        snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "RCPT TO: <%s>\r\n", addr->address);
        MP_DEBUG("smtpRcptTo(): >>> %s", send_buf);

        sockPuts(sock_id, send_buf);
        smtpResponse(sock_id);
    }
    return (0);
}

/* SMTP: DATA */
static int smtpData(int sock_id)
{
    MP_DEBUG("smtpData(): >>> DATA");

    sockPuts(sock_id, "DATA\r\n");
    return(smtpResponse(sock_id));
}

/* SMTP: EOM */
int smtpEom(int sock_id)
{
    MP_DEBUG("smtpEom(): >>> .");

    sockPuts(sock_id, "\r\n.\r\n");
    return(smtpResponse(sock_id));
}

/* SMTP: mail */
static int smtpMail(int sock_id, char *to, char *cc, char *from, char *rrr, char *rt,
                    char *subject, char *inline_filepath_mimetype_str,
                    char *simple_txt_msg, int is_mime_flag, int add_date_flag)
{
    char boundary[BOUNDARY_LEN+1];
    SLIST_TYPE *attach_list;
    int rc;

    STREAM *handle = NULL;
    int file_size, bytesRead, bytesSent_of_whole_file;

    attach_list = get_attachment_list();
    if (attach_list)
        is_mime_flag = 1;

    if (subject)
    {
        memset(send_buf, 0, SMTP_SEND_BUF_LEN);
        snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Subject: %s\r\n", subject);
        sockPuts(sock_id, send_buf);
        MP_DEBUG(send_buf);
    }

    if (from)
    {
        memset(send_buf, 0, SMTP_SEND_BUF_LEN);
        snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "From: %s\r\n", from);
        sockPuts(sock_id, send_buf);
        MP_DEBUG(send_buf);
    }

    if (add_date_flag)
    {
        char datebuf[65];

        memset(datebuf, 0, sizeof(datebuf));
        if (rfc822_date(clock_time(), datebuf, sizeof(datebuf)-1) == 0)
        {
            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Date: %s\r\n", datebuf);
            sockPuts(sock_id, send_buf);
            MP_DEBUG(send_buf);
        }
    }

    if (to)
    {
        memset(send_buf, 0, SMTP_SEND_BUF_LEN);
        snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "To: %s\r\n", to);
        sockPuts(sock_id, send_buf);
        MP_DEBUG(send_buf);
    }

    if (cc)
    {
        memset(send_buf, 0, SMTP_SEND_BUF_LEN);
        snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Cc: %s\r\n", cc);
        sockPuts(sock_id, send_buf);
        MP_DEBUG(send_buf);
    }

    if (rt != NULL)
    {
        memset(send_buf, 0, SMTP_SEND_BUF_LEN);
        snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Reply-To: %s\r\n", rt);
        sockPuts(sock_id, send_buf);
        MP_DEBUG(send_buf);
    }

    if (rrr != NULL)
    {
        memset(send_buf, 0, SMTP_SEND_BUF_LEN);
        snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Disposition-Notification-To: %s\r\n", rrr);
        sockPuts(sock_id, send_buf);
        MP_DEBUG(send_buf);
    }

    memset(send_buf, 0, SMTP_SEND_BUF_LEN);
    snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "X-Mailer: %s\r\n", MP622D_SMTP_TEST_VERSION);
    sockPuts(sock_id, send_buf);
    MP_DEBUG(send_buf);

    if (is_mime_flag)
    {
        srand(clock_time());

        memset(boundary, 0, sizeof(boundary));
        mutilsGenerateMIMEBoundary(boundary, sizeof(boundary));

        memset(send_buf, 0, SMTP_SEND_BUF_LEN);
        snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Content-type: multipart/mixed; boundary=\"%s\"\r\n", boundary);
        sockPuts(sock_id, send_buf);
        MP_DEBUG(send_buf);

        memset(send_buf, 0, SMTP_SEND_BUF_LEN);
        snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Mime-version: 1.0\r\n");
        sockPuts(sock_id, send_buf);
        MP_DEBUG(send_buf);
    }

    sockPuts(sock_id, "\r\n");

    if (is_mime_flag)
    {
        int bytesSent, piece_len_to_send;

        /* if there a txt file or a one line messgae, attach them first */
        if (simple_txt_msg)
        {
            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "\r\n--%s\r\n", boundary);
            sockPuts(sock_id, send_buf);
            MP_DEBUG(send_buf);

            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Content-Type: text/plain; charset=%s\r\n", g_charset);
            sockPuts(sock_id, send_buf);
            MP_DEBUG(send_buf);

            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            strcpy(send_buf, "Content-Disposition: inline\r\n");
            sockPuts(sock_id, send_buf);
            MP_DEBUG(send_buf);

            sockPuts(sock_id, "\r\n");

            mutilsStrncpy(send_buf, simple_txt_msg, SMTP_SEND_BUF_LEN-1);
            sockPuts(sock_id, send_buf);
            MP_DEBUG(">>> %s", simple_txt_msg); 

            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "\r\n\r\n");
            sockPuts(sock_id, send_buf);
            MP_DEBUG(send_buf);
        }

        if (inline_filepath_mimetype_str)
        {  
            char mime_type[32], filename[128];

            rc = get_filepath_mimetype(inline_filepath_mimetype_str, filename, sizeof(filename)-1, mime_type, sizeof(mime_type)-1);
            if(rc == -1)
                return(-1);
            
           /*
            * The file should not have binary characters in it.
            * It's user's responsibilty to make sure file is not binary.
            * If file is binary mail will have garbage as I'll not convert
            * the content to base64
            */

            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "\r\n--%s\r\n", boundary);
            sockPuts(sock_id, send_buf);
            MP_DEBUG(send_buf);

            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Content-Type: %s; charset=%s\r\n", mime_type, g_charset);
            sockPuts(sock_id, send_buf);
            MP_DEBUG(send_buf);

            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            strcpy(send_buf, "Content-Disposition: inline\r\n");
            sockPuts(sock_id, send_buf);
            MP_DEBUG(send_buf);

            sockPuts(sock_id, "\r\n");

            rawfile_len = 0;

            /* read file content */

            rawfile_data_buff = ext_mem_malloc(MIME_TEMPFILE_BUFF_SIZE);
            if(rawfile_data_buff == NULL)
            {
                MP_ALERT("smtpMail(): failed processing file due to memory allocation problem !");
                return(-1);
            }

            mpDebugPrint("smtpMail(): read file (%s) content to buffer...", filename);

            /* read file content piece by piece with small buffer usage */

            handle = OpenFileByNameForRead(filename, &file_size);
            if(handle == NULL)
            {
                MP_ALERT("smtpMail(): failed to read file (%s) content !", filename);
                /* remember to release allocated memory of raw file buffer */
                ext_mem_free(rawfile_data_buff);
                rawfile_data_buff = NULL;
                rawfile_len = 0;
                return(-1);
            }

            bytesRead = 0;
            bytesSent_of_whole_file = 0;
            while(bytesRead < file_size)
            {
            	int read_len;

                memset(rawfile_data_buff, 0, MIME_TEMPFILE_BUFF_SIZE);            	
            	read_len = FileRead(handle, rawfile_data_buff, MIME_TEMPFILE_BUFF_SIZE);
                if((read_len == 0) && (bytesRead < file_size))
                {
                    MP_ALERT("smtpMail(): failed to read file (%s) content !", filename);                	
                    MP_DEBUG("smtpMail(): only partial (%d) bytes was read from file (%s) !", bytesRead, filename);
                    FileClose(handle);
                    /* remember to release allocated memory of raw file buffer */
                    ext_mem_free(rawfile_data_buff);
                    rawfile_data_buff = NULL;
                    rawfile_len = 0;
                    return(-1);
                }
                bytesRead += read_len;

                /* No MIME Base64 processing in such case */
                bytesSent = 0;
                while(bytesSent < read_len)
                {
                    piece_len_to_send = ((read_len - bytesSent) >= FILE_SEND_UNIT_SIZE) ? FILE_SEND_UNIT_SIZE : (read_len - bytesSent);
                    memset(send_buf, 0, SMTP_SEND_BUF_LEN);

                    memcpy(send_buf, (rawfile_data_buff + bytesSent), piece_len_to_send);
                    send_buf[piece_len_to_send] = '\0';

                    bytesSent += sockWrite(sock_id, send_buf, piece_len_to_send);
                    //MP_DEBUG("smtpMail(): accumulated bytesSent of this file piece = %d", bytesSent);

                #if (TASK_YIELD_IN_PROCESSING_LOOP)  /* performance issue: TaskYield() in loop is better ?? */
                    TaskYield();
                #endif
                }
                bytesSent_of_whole_file += bytesSent;
                //MP_DEBUG("smtpMail(): accumulated bytesSent of whole file (%s) = %d", filename, bytesSent_of_whole_file);
            }

            FileClose(handle);
            MP_DEBUG("smtpMail(): (%d) bytes was read from file (%s) !", bytesRead, filename);

            /* we can release allocated memory for raw file content now */
            ext_mem_free(rawfile_data_buff);
            rawfile_data_buff = NULL;
            rawfile_len = 0;

            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "\r\n\r\n");
            sockPuts(sock_id, send_buf);
            MP_DEBUG(send_buf);
        }

        /* handle MIME */
        {
            Attachment *attach;
            SLIST_TYPE *list;

            for (list=attach_list; list; list=list->next)
            {
                attach = (Attachment *) list->data;
                if (attach == NULL)
                    continue;

                memset(send_buf, 0, SMTP_SEND_BUF_LEN);
                snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "--%s\r\n", boundary);
                sockPuts(sock_id, send_buf);
                MP_DEBUG(send_buf);

                if (mutilsStrcasecmp(attach->mime_type, "text/plain") == 0)
                {
                    memset(send_buf, 0, SMTP_SEND_BUF_LEN);
                    snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Content-Type: text/plain; charset=%s\r\n", g_charset);
                    sockPuts(sock_id, send_buf);
                    MP_DEBUG(send_buf);

                    memset(send_buf, 0, SMTP_SEND_BUF_LEN);
                    snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Content-Disposition: %s; filename=\"%s\"\r\n", attach->content_disposition, attach->file_name);
                    sockPuts(sock_id, send_buf);
                    MP_DEBUG(send_buf);

                    sockPuts(sock_id, "\r\n");
                }
                else
                {
                    memset(send_buf, 0, SMTP_SEND_BUF_LEN);
                    snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Content-Type: %s; name=%s\r\n", attach->mime_type, attach->file_name);
                    sockPuts(sock_id, send_buf);
                    MP_DEBUG(send_buf);

                    memset(send_buf, 0, SMTP_SEND_BUF_LEN);
                    snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "Content-Disposition: %s; filename=\"%s\"\r\n", attach->content_disposition, attach->file_name);
                    sockPuts(sock_id, send_buf);
                    MP_DEBUG(send_buf);

                    sockPuts(sock_id, "Content-Transfer-Encoding: base64\r\n\r\n");
                    MP_DEBUG("Content-Transfer-Encoding: base64");
                }

                rawfile_len = 0;

                /* read the MIME input file content */

                rawfile_data_buff = ext_mem_malloc(MIME_TEMPFILE_BUFF_SIZE);
                if(rawfile_data_buff == NULL)
                {
                    MP_ALERT("smtpMail(): failed processing file due to memory allocation problem !");
                    return(-1);
                }

                mpDebugPrint("smtpMail(): read file (%s) content to buffer...", attach->file_path);

                /* read file content piece by piece with small buffer usage */

                handle = OpenFileByNameForRead(attach->file_path, &file_size);
                if(handle == NULL)
                {
                    MP_ALERT("smtpMail(): failed to read file (%s) content !", attach->file_path);
                    /* remember to release allocated memory of raw file buffer */
                    ext_mem_free(rawfile_data_buff);
                    rawfile_data_buff = NULL;
                    rawfile_len = 0;
                    return(-1);
                }

                bytesRead = 0;
                bytesSent_of_whole_file = 0;
                while(bytesRead < file_size)
                {
            	    int read_len;

                    memset(rawfile_data_buff, 0, MIME_TEMPFILE_BUFF_SIZE);            	
            	    read_len = FileRead(handle, rawfile_data_buff, MIME_TEMPFILE_BUFF_SIZE);
                    if((read_len == 0) && (bytesRead < file_size))
                    {
                        MP_ALERT("smtpMail(): failed to read file (%s) content !", attach->file_path);                	
                        MP_DEBUG("smtpMail(): only partial (%d) bytes was read from file (%s) !", bytesRead, attach->file_path);
                        FileClose(handle);
                        /* remember to release allocated memory of raw file buffer */
                        ext_mem_free(rawfile_data_buff);
                        rawfile_data_buff = NULL;
                        rawfile_len = 0;
                        return(-1);
                    }
                    bytesRead += read_len;

                    /* base64 encode only if it is not MIME type of text/plain */

                    if (mutilsStrcasecmp(attach->mime_type, "text/plain") != 0) /* Need to be base64 encoded */
                    {
                        int bytesProcessed = 0, piece_len_to_process = 0;

                        bytesSent = 0;
                        while(bytesProcessed < read_len)
                        {
                            piece_len_to_process = ((read_len - bytesProcessed) >= MIME_UNIT_LEN_FOR_BASE64_ENCODE) ? MIME_UNIT_LEN_FOR_BASE64_ENCODE : (read_len - bytesProcessed);

                            /* prepare a temp memory buffer for writing MIME Base64 content */
                            if(mime_tmpfile_buff != NULL && mime_tmpbuff_len > 0)
                            {
                                base64_mfree(mime_tmpfile_buff); /* To be consistent with the malloc function in base64_en/decode */
                                mime_tmpfile_buff = NULL;
                                mime_tmpbuff_len = 0;
                            }

                            mime_tmpfile_buff = base64_encode((rawfile_data_buff + bytesProcessed), piece_len_to_process, &mime_tmpbuff_len);
                            if (mime_tmpfile_buff == NULL || mime_tmpbuff_len == 0)
                            {
                                MP_ALERT("smtpMail(): failed processing MIME due to memory allocation problem !");
                                FileClose(handle);
                                /* remember to release allocated memory of raw file buffer */
                                ext_mem_free(rawfile_data_buff);
                                rawfile_data_buff = NULL;
                                rawfile_len = 0;
                                return(-1);
                            }
 
                            bytesSent += sockWrite(sock_id, mime_tmpfile_buff, mime_tmpbuff_len);
                   
                            bytesProcessed += piece_len_to_process;
                            //MP_DEBUG("smtpMail(): bytesProcessed of raw file piece = %d, bytesSent of base64 encoded = %d", bytesProcessed, bytesSent);

                            /* we can release allocated memory for MIME file content now */
                            base64_mfree(mime_tmpfile_buff); /* To be consistent with the malloc function in base64_en/decode */
                            mime_tmpfile_buff = NULL;
                            mime_tmpbuff_len = 0;

                        #if (TASK_YIELD_IN_PROCESSING_LOOP)  /* performance issue: TaskYield() in loop is better ?? */
                            TaskYield();
                        #endif
                        }
                    }
                    else /* No base64 encode is needed */
                    {
                        /* No MIME Base64 processing in such case */
                        bytesSent = 0;
                        while(bytesSent < read_len)
                        {
                            piece_len_to_send = ((read_len - bytesSent) >= FILE_SEND_UNIT_SIZE) ? FILE_SEND_UNIT_SIZE : (read_len - bytesSent);
                            memset(send_buf, 0, SMTP_SEND_BUF_LEN);

                            memcpy(send_buf, (rawfile_data_buff + bytesSent), piece_len_to_send);
                            send_buf[piece_len_to_send] = '\0';

                            bytesSent += sockWrite(sock_id, send_buf, piece_len_to_send);
                            //MP_DEBUG("smtpMail(): accumulated bytesSent of this file piece = %d", bytesSent);

                        #if (TASK_YIELD_IN_PROCESSING_LOOP)  /* performance issue: TaskYield() in loop is better ?? */
                            TaskYield();
                        #endif
                        }
                    }

                    bytesSent_of_whole_file += bytesSent;
                    //MP_DEBUG("smtpMail(): accumulated bytesSent of whole file (%s) = %d", attach->file_path, bytesSent_of_whole_file);
                }

                FileClose(handle);
                MP_DEBUG("smtpMail(): (%d) bytes was read from file (%s) !", bytesRead, attach->file_path);

                /* we can release allocated memory for raw file content now */
                ext_mem_free(rawfile_data_buff);
                rawfile_data_buff = NULL;
                rawfile_len = 0;

                /* now, processing of this attachment file is completely finished */

processing_next_file:
                continue;
            }

            memset(send_buf, 0, SMTP_SEND_BUF_LEN);
            snprintf(send_buf, SMTP_SEND_BUF_LEN-1, "\r\n--%s--\r\n", boundary);
            sockPuts(sock_id, send_buf);
            MP_DEBUG(send_buf);
        } /* end of MIME handling */
    }

    return(0);
}


/*
**  send_the_mail() - main SMTP processing function
**
**  Parameters:
**  char  *my_username                    user account name
**  char  *my_password                    user password
**  char  *from                           email address of mail sender
**  char  *to                             email address (list) of mail recipient(s)
**  char  *cc                             [optional] email address (list) for server to also send an mail instance too 
**  char  *subject                        [optional] mail subject string
**  char  *smtp_server                    SMTP mail server address string
**  U32   smtp_port                       SMTP mail server port
**  char  *helo_domain                    mail client's domain
**  char  *attach_filename                attachment binary file (list) string
**                                        ==> for "Content-Disposition: attachment" file case
**                                            string of the form "file1;file2;file3;..." i.e. seperated by ';' char
**                                            No space char is allowed
**  char  *inline_filepath_mimetype_str   [optional] single attachment text/html file string to be added (inline) into mail body
**                                        ==> for "Content-Disposition: inline" file case 
**                                            string of the form "filename,mime_type"  ex: "test.txt,text/plain" or "test.html,text/html" ... etc
**                                            No space char is allowed
**  char  *simple_txt_msg                 [optional] simple text string to be added (inline) into mail body
**                                        ==> for "Content-Disposition: inline" simple text case
**  char  *rrr                            [optional] email address string for "Disposition-Notification-To:" field
**  char  *rt                             [optional] email address string for "Reply-To:" field
**  int   add_date_flag                   flag indicating to add Date header info or not
**  char  *charset                        [optional] default char set in MIME content
**  int   ssl_flag                        flag indicating to use SSL connection or not
**
**  Return Values:
**  returns 0 on success, -1 on failure
*/
int send_the_mail(char *my_username, char *my_password, char *from, char *to, char *cc, char *subject,
                  char *smtp_server, U32 smtp_port, char *helo_domain,
                  char *attach_filename, char *inline_filepath_mimetype_str, char *simple_txt_msg,
                  char *rrr, char *rt, int add_date_flag, char *charset, int ssl_flag)
{
    SOCKET_ID sock_id;
    SLIST_TYPE *list, *prev;
    int rc;
    char ori_To_addr[MAX_ADDR_STR_LEN], ori_Cc_addr[MAX_ADDR_STR_LEN];
    int is_mime_flag = 0;
    U32 svr_ip_addr = 0x00000000;

    BYTE oriDriveId = DriveCurIdGet();
    MP_DEBUG("send_the_mail(): original Drive Id = %d", oriDriveId);

    data_init_clean();
    smtp_ssl_conn.flags = (ssl_flag > 0)? FLAG_SSL:FLAG_NO_SSL;

    /* we do not support other kind of char set yet */
    if(strcmp(g_charset, charset) != 0)
        strcpy(g_charset, charset);
    
 #if 1 
    MP_DEBUG("From: %s", from);
    MP_DEBUG("To: %s", to);
    MP_DEBUG("Cc: %s", cc);
    MP_DEBUG("Subject: %s", subject);
    MP_DEBUG("smtp server: %s", smtp_server);
    MP_DEBUG("smtp port: %d", smtp_port);
    MP_DEBUG("domain: %s", helo_domain);
    MP_DEBUG("rrr: %s", rrr); /* for "Disposition-Notification-To: */
    MP_DEBUG("rt: %s", rt); /* for "Reply-To:" */
    MP_DEBUG("attach_filename: %s", attach_filename); /* for "Content-Disposition: attachment" file case */
    MP_DEBUG("inline_filepath_mimetype_str: %s", inline_filepath_mimetype_str); /* for "Content-Disposition: inline" file case */
    MP_DEBUG("simple_txt_msg: %s", simple_txt_msg); /* for "Content-Disposition: inline" simple text case */
    MP_DEBUG("add_date_flag: %d", add_date_flag);
    MP_DEBUG("charset: %s", g_charset);
    MP_DEBUG("SSL flag: %d", smtp_ssl_conn.flags);
 #endif

    if((attach_filename != NULL && attach_filename[0] != '\0') || 
       (inline_filepath_mimetype_str != NULL && inline_filepath_mimetype_str[0] != '\0') || 
       (simple_txt_msg != NULL && simple_txt_msg[0] != '\0')) 
        is_mime_flag = 1;

    /* prepare address list */
    /* note: addAddressToList() may modify original string content when tokenizing */
    memset(ori_To_addr, 0, MAX_ADDR_STR_LEN);
    strcpy(ori_To_addr, to);
    memset(ori_Cc_addr, 0, MAX_ADDR_STR_LEN);
    strcpy(ori_Cc_addr, cc);
    
    if(to != NULL && to[0] != '\0')
        addAddressToList(to, "To");
    if(cc != NULL && cc[0] != '\0')
        addAddressToList(cc, "To");

    /* prepare attachment list */
    if(attach_filename != NULL && attach_filename[0] != '\0')
        add_attachment_to_list(attach_filename);

    rc = validateSmtpMusts(from, smtp_server, helo_domain);
    if (rc == -1)
        goto ending_data_clean;

    if (smtp_port == 0)
        smtp_port = SMTP_PORT;

    if (subject == NULL)
        subject = EMPTY_SUBJECT;

    svr_ip_addr = AddrToIP(smtp_server);
    if (svr_ip_addr == 0x00000000)
    {
        MP_ALERT("send_the_mail(): invalid server address: %s", smtp_server);
        rc = -1;
        goto ending_data_clean;
    }

    /* open the network connection */
    sock_id = smtpConnect(svr_ip_addr, smtp_port);
    if (sock_id == INVALID_SOCKET)
    {
        MP_ALERT("send_the_mail(): could not connect to SMTP server \"%s\" at port %d", smtp_server, smtp_port);
        rc = -1;
        goto ending_data_clean;
    }

    /* start SMTP processing */
    rc = smtpHelo(sock_id, helo_domain);
    if(rc == -1) goto cancel_mail_process;

    /* check SMTP server's authentication capability */
    /* currently, we only support AUTH LOGIN method now */ 
    if(server_support_AUTH)
    {
        if(server_support_AUTH_LOGIN)
        {
            MP_DEBUG("\n send_the_mail(): the SMTP server supports AUTH LOGIN method.\n");
        }
        else
        {
            MP_DEBUG("\n send_the_mail(): the SMTP server supports AUTH command, but not support AUTH LOGIN method.\n");
        }
    }
    else
    {
        MP_DEBUG("\n send_the_mail(): the SMTP server do not support AUTH command.\n");
    }

    if(server_support_AUTH_LOGIN)
    {
        char *b64_enc_str, *b64_dec_str = NULL;
        int   b64_enc_len, b64_dec_len = 0;

        rc = smtpAuthLogin(sock_id);
        if(rc == -1) goto cancel_mail_process;

        if(mutilsStrncasecmp(recv_buf, "334", 3) == 0)
        {
            b64_enc_str = (recv_buf + 4);
            b64_enc_len = strlen(recv_buf) - 4;

            b64_dec_str = base64_decode(b64_enc_str, b64_enc_len, &b64_dec_len);
            if(b64_dec_str == NULL || b64_dec_len == 0)
            {
                MP_ALERT("send_the_mail(): base64_decode() failed !");
                rc = -1;
                goto cancel_mail_process;
            }
            b64_dec_str[b64_dec_len] = '\0';
            MP_DEBUG("send_the_mail(): base64_decode() ==> %s  ,length=%d", b64_dec_str, b64_dec_len);

            /* check 334 string description ? */

            base64_mfree(b64_dec_str); /* To be consistent with the malloc function in base64_en/decode */
            b64_dec_str = NULL;
            b64_dec_len = 0;

            rc = smtpAuth_B64_UserInfo(sock_id, my_username);
            if(rc == -1)
            {
                MP_ALERT("send_the_mail(): user authentication failed !");
                goto cancel_mail_process;
            }

            if(mutilsStrncasecmp(recv_buf, "334", 3) == 0)
            {
                b64_enc_str = (recv_buf + 4);
                b64_enc_len = strlen(recv_buf) - 4;

                b64_dec_str = base64_decode(b64_enc_str, b64_enc_len, &b64_dec_len);
                if(b64_dec_str == NULL || b64_dec_len == 0)
                {
                    MP_ALERT("send_the_mail(): base64_decode() failed !");
                    rc = -1;
                    goto cancel_mail_process;
                }
                b64_dec_str[b64_dec_len] = '\0';
                MP_DEBUG("send_the_mail(): base64_decode() ==> %s  ,length=%d", b64_dec_str, b64_dec_len);

                /* check 334 string description ? */

                base64_mfree(b64_dec_str); /* To be consistent with the malloc function in base64_en/decode */
                b64_dec_str = NULL;
                b64_dec_len = 0;

                rc = smtpAuth_B64_UserInfo(sock_id, my_password);
                if(rc == -1)
                {
                    MP_ALERT("send_the_mail(): user authentication failed !");
                    goto cancel_mail_process;
                }

                if(mutilsStrncasecmp(recv_buf, "235", 3) == 0)
                {
                    MP_DEBUG("send_the_mail(): user authentication OK !");
                }
                else
                {
                    MP_ALERT("send_the_mail(): user authentication failed !");
                    rc = -1;
                    goto cancel_mail_process;
                }
            }
            else
            {
                MP_ALERT("Mail server not support or not allow SMTP AUTH LOGIN method !");
                rc = -1;
                goto cancel_mail_process;
            }
        }
        else
        {
            MP_ALERT("Mail server not support or not allow SMTP AUTH LOGIN method !");
            rc = -1;
            goto cancel_mail_process;
        }
    }

    rc = smtpMailFrom(sock_id, from);
    if(rc == -1) goto cancel_mail_process;

    rc = smtpRcptTo(sock_id);
    if(rc == -1) goto cancel_mail_process;

    rc = smtpData(sock_id);
    if(rc == -1) goto cancel_mail_process;

    if(mutilsStrncasecmp(recv_buf, "354", 3) == 0)
    {
        rc = smtpMail(sock_id, ori_To_addr, ori_Cc_addr, from, rrr, rt, subject, inline_filepath_mimetype_str, simple_txt_msg, is_mime_flag, add_date_flag);
        if(rc == -1)
        {
            smtpEom(sock_id); /* force to end of mail */
            goto cancel_mail_process;
        }
    }
    else
    {
        MP_ALERT("send_the_mail(): SMTP server rejects my mail sending request!");
        rc = -1;
        goto cancel_mail_process;
    }

    rc = smtpEom(sock_id); /* normal end of mail */
    if(rc == -1) goto cancel_mail_process;

    rc = smtpQuit(sock_id);
    if(rc == -1)
        goto cancel_mail_process;
    else
        goto normal_ending;

cancel_mail_process:
    smtpRset(sock_id);

normal_ending:
    if(smtp_ssl_conn.flags == FLAG_SSL)
        SSL_close(&smtp_ssl_conn);

    closesocket(sock_id);

ending_data_clean:
    /* clear and free address list */
    list = getAddressList();
    while(list != NULL)
    {
        prev = list; 
        list = list->next;
        if(prev->data != NULL)
        {
            if(((Address *)prev->data)->label)
                smtp_mfree(((Address *)prev->data)->label);

            if(((Address *)prev->data)->address)
                smtp_mfree(((Address *)prev->data)->address);

            smtp_mfree(prev->data);
        }
        smtp_mfree(prev);
    }
    addr_list_head = NULL;

    /* clear and free attachment list */
    list = get_attachment_list();
    while(list != NULL)
    {
        prev = list; 
        list = list->next;
        if(prev->data != NULL)
        {
            if(((Attachment *)prev->data)->file_path)
                smtp_mfree(((Attachment *)prev->data)->file_path);

            if(((Attachment *)prev->data)->file_name)
                smtp_mfree(((Attachment *)prev->data)->file_name);

            if(((Attachment *)prev->data)->mime_type)
                smtp_mfree(((Attachment *)prev->data)->mime_type);

            if(((Attachment *)prev->data)->content_disposition)
                smtp_mfree(((Attachment *)prev->data)->content_disposition);

            smtp_mfree(prev->data);
        }
        smtp_mfree(prev);
    }
    attachment_list_head = NULL;

    DriveChange(oriDriveId);
    MP_DEBUG("send_the_mail(): restore to original Drive ID");

    return(rc);
}


/* OpenFileByNameForRead() - Search and open a file by filename, and return file handle and its size */
STREAM *OpenFileByNameForRead(const char* filename, int *file_size)
{
    BOOL boDriveAdded = FALSE;
    BYTE bMcardId = SD_MMC;
    STREAM *handle = NULL;
    DRIVE *sDrv;
    char* file_ext;

    char temp_filename[64]; //not to modify input filename string

#if (SUPPORT_LONG_FILENAME_ATTACHMENT)  /* support long filename attachment */
    int i, total_len;

    memset(temp_filename, 0, sizeof(temp_filename));
    total_len = strlen(filename);

    /* MagicPixel's card storage use WORD type array to store Unicode/Long filename internally ! */
    for(i=0; filename[i] != '\0'; i++)
        temp_filename[2*i+1] = filename[i];
#else  /* filename cannot exceed 8 chars in main part and 3 chars in extension part per DOS/FAT 8.3 format */
    memset(temp_filename, 0, sizeof(temp_filename));
    strcpy(temp_filename, filename);

    file_ext = temp_filename;
    while(*file_ext != '.')
        file_ext++;
    *file_ext = '\0';
    file_ext++;
#endif

    if (SystemCardPlugInCheck(bMcardId))
    {	
        SystemDeviceInit(bMcardId);

        if (!SystemCardPresentCheck(bMcardId))
        {	   
            MP_ALERT("%s: Mcard %d not present !", __FUNCTION__, bMcardId);
            return(0);
        }
        else
        {
            if (!(boDriveAdded = DriveAdd(bMcardId)))
            {
                MP_ALERT("%s: DriveAdd() failed !", __FUNCTION__);
                return(0);
            }
        }

        if (boDriveAdded)
        {
            DRIVE *drv;
            drv = DriveChange(bMcardId);
            if (DirReset(drv) != FS_SUCCEED)
            {
                MP_ALERT("%s: DirReset() failed !", __FUNCTION__);
                return(0);
            }

#if (SUPPORT_LONG_FILENAME_ATTACHMENT)  /* support long filename attachment */
            if (FileSearchLN(drv, temp_filename, total_len, E_FILE_TYPE) != FS_SUCCEED)
#else  /* filename cannot exceed 8 chars in main part and 3 chars in extension part per DOS/FAT 8.3 format */
            if (FileSearch(drv, temp_filename, file_ext, E_FILE_TYPE) != FS_SUCCEED)
#endif
            {
                MP_ALERT("%s: FileSearch failed, file (%s) not found !", __FUNCTION__, filename);
                return(0);
            }

            sDrv = DriveGet(bMcardId);
            handle = FileOpen(sDrv);
            if(handle == NULL)
            {
                MP_ALERT("%s: open file failed !", __FUNCTION__);
                return(0);
            }

            *file_size = FileSizeGet(handle);
            return(handle);
        }
        else
            return(0);
    }
    else
    {
        MP_ALERT("%s: Mcard %d not plugged-in !", __FUNCTION__, bMcardId);
        return(0);
    }
}


/* SMTP_Test_Demo() - a demo of send_the_mail() function usage to send mail with file attachment to some mailbox */
void  SMTP_Test_Demo(void)
{
#define Mailbox_MagicPixel    1 
#define Mailbox_GoogleMail    2 
#define Mailbox_YahooMail     3 
#define Mailbox_MsLiveMail    4 

    char *my_username, *my_password,
         *smtp_server, *from, *to, *cc, *rrr, *rt,
         *helo_domain,
         *subject,
         *charset;

    /* attach_filename  =>  for "Content-Disposition: attachment" file case */
    /* String of the form "file1;file2;file3;..." i.e. seperated by ';' char */
    /* note: No space char is allowed */
    char *attach_filename;

    /* inline_filepath_mimetype_str  =>  for "Content-Disposition: inline" file case */
    /* String of the form "filename,mime_type"  ex: "test.txt,text/plain" or "test.html,text/html" ... etc */
    /* note: No space char is allowed */
    char *inline_filepath_mimetype_str;

    /* simple_txt_msg  =>  for "Content-Disposition: inline" simple text case */
    char *simple_txt_msg;

    int  smtp_port;
    int  ssl_flag; //flag for using SSL connection or not
    int  add_date_flag; //add Date header info or not
    int  rc;

    int  test_mailbox = Mailbox_GoogleMail; //select mailbox to test

    /* Set username and password of your own email account for each mailbox to be test.
       And set the mail To/Cc addresses to your own ones. Pls don't send garbage to my addresses */
    if(test_mailbox == Mailbox_MagicPixel)
    {
    /* note: for MagicPixel mailbox, no need to do Authentication. And SMTP SSL is not supported (or port 465 is closed !?) */

        /* your personal data and test addresses. Change these by yourself */
        my_username = "weichingtu";
        my_password = "wxyz12345";
        //to = "weiching.tu@gmail.com"; //single recipient
        to = "weiching.tu@gmail.com,weichingtu@magicpixel.com.tw"; //multiple recipients
        //cc = "weichingtu@livemail.tw"; //single recipient
        cc = "weichingtu@livemail.tw,wctu@pchome.com.tw"; //multiple recipients

        /* real SMTP server info */
        smtp_server = "mail.magicpixel.com.tw"; //FQDN and dotted IP string (ex: "192.168.47.2")
        smtp_port = SMTP_PORT;
        ssl_flag = FLAG_NO_SSL;
    }
    else if(test_mailbox == Mailbox_GoogleMail)
    {
    /* note: for Gmail mailbox, need to enable Authentication and SSL */

        /* your personal data and test addresses. Change these by yourself */
        my_username = "weiching.tu";
        my_password = "wxyz12345";
        //to = "weichingtu@magicpixel.com.tw"; //single recipient
        to = "weichingtu@magicpixel.com.tw,weiching.tu@gmail.com"; //multiple recipients
        //cc = "weichingtu@livemail.tw"; //single recipient
        cc = "weichingtu@livemail.tw,wctu@pchome.com.tw"; //multiple recipients

        /* real SMTP server info */
        smtp_server = "smtp.gmail.com"; //FQDN and dotted IP string (ex: "192.168.47.2")
        smtp_port = SMTP_SSL_PORT;
        ssl_flag = FLAG_SSL;
    }
    else if(test_mailbox == Mailbox_YahooMail)
    {
    /* note: for Yahoo mailbox, need enable Authentication and SSL. Only premium users can access it ! */

        /* your personal data and test addresses. Change these by yourself */
        my_username = "weichingtu";
        my_password = "wxyz12345";
        //to = "weichingtu@magicpixel.com.tw"; //single recipient
        to = "weichingtu@magicpixel.com.tw,weiching.tu@gmail.com"; //multiple recipients
        //cc = "weichingtu@livemail.tw"; //single recipient
        cc = "weichingtu@livemail.tw,wctu@pchome.com.tw"; //multiple recipients

        /* real SMTP server info */
        smtp_server = "plus.smtp.mail.yahoo.com"; //FQDN and dotted IP string (ex: "192.168.47.2")
        smtp_port = SMTP_SSL_PORT;
        ssl_flag = FLAG_SSL;
    }
    else if(test_mailbox == Mailbox_MsLiveMail)
    {
    /* note: for Hotmail/LiveMail mailbox, need enable Authentication and SSL. */
    /* But, only users in some countries can access it, Taiwan is not in the supported countries list yet! */

        /* your personal data and test addresses. Change these by yourself */
        my_username = "weichingtu@livemail.tw"; /* note: full Hotmail/LiveMail email address, per the guideline on Windows Live web site */
        my_password = "wxyz12345";
        //to = "weichingtu@magicpixel.com.tw"; //single recipient
        to = "weichingtu@magicpixel.com.tw,weiching.tu@gmail.com"; //multiple recipients
        //cc = "wctu@pchome.com.tw"; //single recipient
        cc = "wctu@pchome.com.tw,weichingtu@ymail.com"; //multiple recipients

        /* real SMTP server info */
        smtp_server = "smtp.live.com";
        smtp_port = 25; /* note: 25 or 587, per the guideline on Windows Live web site */
        ssl_flag = FLAG_SSL;
    }

    /* fake client info */
    from = "SMTP_Test@MP622D_DPF.MPX.com";
    helo_domain = "MP622D_DPF.MPX.com"; //a fake client domain

    subject = "SMTP testing";
    add_date_flag = 0;
    charset = "us-ascii"; //char set
    rrr = NULL; //optional
    rt = NULL;  //optional

    /* attach_filename  =>  for "Content-Disposition: attachment" file case */
    /* String of the form "file1;file2;file3;..." i.e. seperated by ';' char */
    /* note: No space char is allowed */
#if (SUPPORT_LONG_FILENAME_ATTACHMENT)  /* support long filename attachment */
    attach_filename = "Pic_LongName12345678.jpg;picTest.jpg";
#else  /* filename cannot exceed 8 chars in main part and 3 chars in extension part per DOS/FAT 8.3 format */
    attach_filename = "pic1.jpg;picTest.jpg";
#endif

    /* inline_filepath_mimetype_str  =>  for "Content-Disposition: inline" file case */
    /* String of the form "filename,mime_type"  ex: "test.txt,text/plain" or "test.html,text/html" ... etc */
    /* note: No space char is allowed */
    inline_filepath_mimetype_str = NULL;

    /* simple_txt_msg  =>  for "Content-Disposition: inline" simple text case */
    simple_txt_msg = "This is multiple lines text string in mail body\r\n"
                     "\t The 2nd line to be displayed\r\n"
                     "\t The 3rd line to be displayed\r\n";


    mpDebugPrint("\n Test_SMTP_send(): begin to test SMTP send mail...\n");

    /* main SMTP processing function */
    rc = send_the_mail(my_username, my_password, from, to, cc, subject, 
                  smtp_server, smtp_port, helo_domain,
                  attach_filename, inline_filepath_mimetype_str, simple_txt_msg,
                  rrr, rt, add_date_flag, charset, ssl_flag);

    if(rc == -1)
    {
        mpDebugPrint("\n Test_SMTP_send(): SMTP send mail failed !\n");
    }
    else
    {
        mpDebugPrint("\n Test_SMTP_send(): SMTP send mail successfully !\n");
    }

    return;
}



#endif /* _SMTP_C_ */
