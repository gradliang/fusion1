
#ifndef _PROTOCOL_H
#define _PROTOCOL_H


/* This is used to register socket interfaces for IP protocols.  */
struct inet_protosw {
	struct list_head list;

        /* These two fields form the lookup key.  */
	unsigned short	 type;	   /* This is the 2nd argument to socket(2). */
	unsigned short	 protocol; /* This is the L4 protocol number.  */

	struct proto	 *prot;
	const struct proto_ops *ops;
  
	int              capability; /* Which (if any) capability do
				      * we need to use this socket
				      * interface?
                                      */
	char             no_check;   /* checksum on rcv/xmit/none? */
	unsigned char	 flags;      /* See INET_PROTOSW_* below.  */
};
#endif	/* _PROTOCOL_H */
