
#ifndef __AUTO_SEARCH__
#define __AUTO_SEARCH__
#include "uip.h"

#define MAX_NUM_SERVER		20
#define MAX_S_NAME_LENGTH	256
#define MAX_URL_NAME_LENGTH	(2 * 256)

#define NWSERVER state
#define NW_INVALID		0
#define NW_VALID		1

typedef struct {
	BYTE 		ServerName[MAX_S_NAME_LENGTH];
	BYTE		UrlName[MAX_URL_NAME_LENGTH];
	uip_ipaddr_t   ipaddr;
	WORD 		wPort;
	BYTE		bState;
	BYTE  		bVersion;
} NWSERVER;


// define auto search state
#define AS_STATE_INIT		0
#define AS_STATE_START		1
#define AS_STATE_SEARCHING	2
#define AS_STATE_READY		3

typedef struct {
	DWORD 		dwNumberOfServer;		// number of server found
	DWORD 		dwCurrentServer;		// The selected server
	DWORD 		dwFirstList;			// The first list index
	DWORD 		dwListIndex;			// The list offset from first list
	NWSERVER 	ServerList[MAX_NUM_SERVER];
	BYTE  		bState;
} SERVER_BROWSER;





void *StartAutoSearch();
DWORD GetNumberOfServer();
DWORD GetCurrentServerIndex();
void SetCurrentServerIndex(DWORD index);
#endif
