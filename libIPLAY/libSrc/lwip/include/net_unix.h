#ifndef _NET_UNIX_UN_H
#define _NET_UNIX_UN_H

#define UNIX_PATH_MAX	108

struct sockaddr_un {
	sa_family_t sun_family;	/* AF_UNIX */
	char sun_path[UNIX_PATH_MAX];	/* pathname */
};

typedef struct
{
    U16 u16SockId;
    U08 u08LocalPath[UNIX_PATH_MAX];
    U08 u08PeerPath[UNIX_PATH_MAX];
} ST_UNIX_PCB;

#endif /* _NET_UNIX_UN_H */
