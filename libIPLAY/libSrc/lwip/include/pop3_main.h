#ifndef _POP3_MAIN_H
#define _POP3_MAIN_H

#define POP3_PORT 110
#define POP3_SSL_PORT 995


/* Content-Transfer-Encoding */
enum
{
  ENCOTHER,
  ENC7BIT,
  ENC8BIT,
  ENCQUOTEDPRINTABLE,
  ENCBASE64,
  ENCBINARY,
  ENCUUENCODED
};


#if 1
/* MailNode */
struct MailNode
{
    /* Photo link list */
    struct list_head list;

	int 	updated;

	int 	num;

	void	*buffer;

	int 	buff_len;

#define MAX_SUBJECT_LENGTH 256
	//note: support max 256 bytes length
	char	subject[MAX_SUBJECT_LENGTH];
	
	char	*text;

	int		text_len;

	struct list_head AttachQ;
};

/* AttachNode */
struct AttachNode
{
    /* Attach link list */
    struct list_head list;
	
	char	Name[256];

	int		coding;

	void	*buffer;

	int 	buff_len;
};


#endif


#endif /*_POP3_MAIN_H*/

