/* Note: This SMTP client code is mainly ported and mimicked from the open-source mailsend v1.14 package which 
 *       is a simple and basic SMTP client and doesn't support SMTP AUTH command nor ESMTP EHLO command. 
 *       I extended it to support EHLO and SMTP AUTH LOGIN method. Finally, OpenSSL is applied to secure it.
 *
 *       Created: WeiChing 2009.03
 *
 *       Note: Some advanced features of ESMTP commands and Authentication Methods are not supported. 
 *
 */

#ifndef _SMTP_H
#define _SMTP_H


#define SMTP_PORT       25 
#define SMTP_SSL_PORT   465 

#ifndef LONG_NAME_MODE
  #define SUPPORT_LONG_FILENAME_ATTACHMENT       0  /* filename cannot exceed 8 chars in main part and 3 chars in extension part per DOS/FAT 8.3 format */
#else
  //#define SUPPORT_LONG_FILENAME_ATTACHMENT     0  /* filename cannot exceed 8 chars in main part and 3 chars in extension part per DOS/FAT 8.3 format */
  #define SUPPORT_LONG_FILENAME_ATTACHMENT       1  /* support long filename attachment. Note: It may encounter problem when doing FileSearch ! */
#endif

//#define MIME_UNIT_LEN_FOR_BASE64_ENCODE  (3*1024)     /* 3 KB. Set to multiples of 3 for base64 encoding */
//#define MIME_UNIT_BASE64_CODED_BUFF_LEN  (4*1024+64)  /* 4 KB + enough buffer for extra '\n' bytes in email per RFC822 */
#define MIME_UNIT_LEN_FOR_BASE64_ENCODE  (6*1024)      /* 6 KB. Set to multiples of 6 for base64 encoding */
#define MIME_UNIT_BASE64_CODED_BUFF_LEN  (8*1024+128)  /* 8 KB + enough buffer for extra '\n' bytes in email per RFC822 */

#define FILE_SEND_UNIT_SIZE              MIME_UNIT_BASE64_CODED_BUFF_LEN

#define MIME_TEMPFILE_BUFF_SIZE          MIME_UNIT_LEN_FOR_BASE64_ENCODE  /* Set to multiples of 6 for base64 encoding */

#define SMTP_SEND_BUF_LEN                (FILE_SEND_UNIT_SIZE+1)
#define SMTP_RECV_BUF_LEN                (2*1024)


#undef  SOCKET_ID
#define SOCKET_ID  int

#undef  INVALID_SOCKET
#define INVALID_SOCKET  -1 


#define MP622D_SMTP_TEST_VERSION       "@(#) MP622D DPF SMTP client"
#define EMPTY_SUBJECT                  ""

#define CHAR_SPACE                     ' '
#define ASCII_LF                       10 
#define ASCII_CR                       13 
#define ATTACHMENT_FILE_MIMETYPE_SEP   ','  //single char
#define ATTACHMENT_FILE_LIST_SEP       ';'  //single char
#define ADDRESS_SEP_STR                ","  //string with multiple chars
#define DEFAULT_CHARSET                "us-ascii"

#define MUTILS_MAX_TOKEN_LEN           64 

/* the singly linked list structure */
typedef struct _Sll
{
    void *data;          /* void pointer for user data */
    struct _Sll *next;   /* pointer to next node */
} SLIST_TYPE;

typedef struct _Address
{
    /*
    ** label holds strings like "To" "Cc" "Bcc". 
    ** The address is the email address.
    */
    char *label;     /* To: Cc: Bcc: */
    char *address;   /* the email address */
} Address;

typedef struct _Attachment
{
    char *file_path;
    char *file_name;
    char *mime_type;
    char *content_disposition;
} Attachment;


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
**                                            No space char is allowed.
**  char  *inline_filepath_mimetype_str   [optional] single attachment text/html file string to be added (inline) into mail body
**                                        ==> for "Content-Disposition: inline" file case 
**                                            string of the form "filename,mime_type"  ex: "test.txt,text/plain" or "test.html,text/html" ... etc
**                                            No space char is allowed.
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
int  send_the_mail(char *my_username, char *my_password, char *from, char *to, char *cc, char *subject,
                   char *smtp_server, U32 smtp_port, char *helo_domain,
                   char *attach_filename, char *inline_filepath_mimetype_str, char *simple_txt_msg,
                   char *rrr, char *rt, int add_date_flag, char *charset, int ssl_flag);

/* SMTP_Test_Demo() - a demo of send_the_mail() function usage to send mail with file attachment to some mailbox */
void SMTP_Test_Demo(void);

/* OpenFileByNameForRead() - Search and open a file by filename, and return file handle and its size */
STREAM *OpenFileByNameForRead(const char* filename, int *file_size);



#endif /* _SMTP_H */
