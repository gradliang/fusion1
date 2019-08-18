///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000-2003 Intel Corporation 
// All rights reserved. 
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met: 
//
// * Redistributions of source code must retain the above copyright notice, 
// this list of conditions and the following disclaimer. 
// * Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation 
// and/or other materials provided with the distribution. 
// * Neither name of Intel Corporation nor the names of its contributors 
// may be used to endorse or promote products derived from this software 
// without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

/************************************************************************
* Purpose: This file implements the functionality and utility functions
* used by the Miniserver module.
************************************************************************/
#define LOCAL_DEBUG_ENABLE 0
#include "config.h"

#ifndef WIN32
 //#include <arpa/inet.h>
 //#include <netinet/in.h>
 #include "UtilTypeDef.h"
 #include "net_in.h"
 //#include <sys/socket.h>
 #include <linux/types.h>
 #include "../../LWIP/INCLUDE/net_socket.h"
 #include "../../LWIP/INCLUDE/net_socket2.h"
 #include <sys/wait.h>
 #include <unistd.h>
 #include <sys/time.h>
#else
 #include <winsock2.h>

 #define socklen_t int
 #define EAFNOSUPPORT 97
#endif
#include "unixutil.h"
#include "ithread.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ssdplib.h"

#include "util.h"
#include "miniserver.h"
#include "ThreadPool.h"
#include "httpreadwrite.h"
#include "statcodes.h"
#include "upnp.h"
#include "upnpapi.h"

#include "taskid.h"
#include "..\..\..\..\libIPLAY\libSrc\netstream\INCLUDE\netstream.h"
#include "global612.h"

#define APPLICATION_LISTENING_PORT 49152
#define	EADDRINUSE	125	/* Address already in use */
#define UPNP_AUDIO_BLOCK_SIZE 8192*8
struct mserv_request_t {
    int connfd;                 // connection handle
    struct in_addr foreign_ip_addr;
    unsigned short foreign_ip_port;
};

extern int errno;
typedef enum { MSERV_IDLE, MSERV_RUNNING, MSERV_STOPPING } MiniServerState;

unsigned short miniStopSockPort;
//unsigned char gssdp_send_discover = 0;
////////////////////////////////////////////////////////////////////////////
// module vars
static MiniServerCallback gGetCallback = NULL;
static MiniServerCallback gSoapCallback = NULL;
static MiniServerCallback gGenaCallback = NULL;
MiniServerState gMServState = MSERV_IDLE;

unsigned char bexit_upupctrl;

u8 bupnpdownload;
u8 quitpnpdownload = TRUE;
extern u8 bdlnadownload;
extern char *gavtransuri;

//For UPNP Audio
#if HAVE_NETSTREAM
#if (CHIP_VER_MSB == CHIP_VER_650)
extern unsigned char girbuf[NETSTREAM_MAX_BUFSIZE+8192];
#else
extern unsigned char *girbuf;//[NETSTREAM_MAX_BUFSIZE+8192];
#endif
extern int gtotoalbuf;
extern unsigned char irready;
extern long gtotaldata;
extern int iradio_wait;
extern int bufindex;
#else
unsigned char girbuf[NETSTREAM_MAX_BUFSIZE+8192];
int gtotoalbuf;
unsigned char irready;
long gtotaldata;
int iradio_wait;
int bufindex;
int asf_curindx;
int bufindex_end;
#endif
int last_one_kbytes_index;
int last_one_kbytes_start_addr;
unsigned char *last_one_kbytes_buf;
extern int kcurindx;

/************************************************************************
*	Function :	SetHTTPGetCallback
*
*	Parameters :
*		MiniServerCallback callback ; - HTTP Callback to be invoked 
*
*	Description :	Set HTTP Get Callback
*
*	Return :	void
*
*	Note :
************************************************************************/
void
SetHTTPGetCallback( MiniServerCallback callback )
{
    gGetCallback = callback;
}

/************************************************************************
*	Function :	SetSoapCallback
*
*	Parameters :
*		MiniServerCallback callback ; - SOAP Callback to be invoked 
*
*	Description :	Set SOAP Callback
*
*	Return :	void
*
*	Note :
************************************************************************/
void
SetSoapCallback( MiniServerCallback callback )
{
    gSoapCallback = callback;
}

/************************************************************************
*	Function :	SetGenaCallback
*
*	Parameters :
*		MiniServerCallback callback ; - GENA Callback to be invoked
*
*	Description :	Set GENA Callback
*
*	Return :	void
*
*	Note :
************************************************************************/
void
SetGenaCallback( MiniServerCallback callback )
{
    gGenaCallback = callback;
}

/************************************************************************
*	Function :	dispatch_request
*
*	Parameters :
*		IN SOCKINFO *info ;		 Socket Information object.
*		http_parser_t* hparser ; HTTP parser object.
*
*	Description :	Based on the type pf message, appropriate callback 
*		is issued
*
*	Return : int ;
*		0 - On Success
*		HTTP_INTERNAL_SERVER_ERROR - Callback is NULL
*
*	Note :
************************************************************************/
static int
dispatch_request( IN SOCKINFO * info,
                  http_parser_t * hparser )
{
    MiniServerCallback callback;
	mpDebugPrint("dispatch_request %x %x",hparser->msg.method,HTTPMETHOD_SUBSCRIBE);
    switch ( hparser->msg.method ) {
            //Soap Call
			//mpDebugPrint("Soap Call");
        case SOAPMETHOD_POST:
        case HTTPMETHOD_MPOST:
            callback = gSoapCallback;
            break;

            //Gena Call
        case HTTPMETHOD_NOTIFY:
        case HTTPMETHOD_SUBSCRIBE:
        case HTTPMETHOD_UNSUBSCRIBE:
			mpDebugPrint("#############Gena Call");
            DBGONLY( UpnpPrintf
                     ( UPNP_INFO, MSERV, __FILE__, __LINE__,
                       "miniserver %d: got GENA msg\n", info->socket );
                 )
                callback = gGenaCallback;
            break;

            //HTTP server call
        case HTTPMETHOD_GET:
        case HTTPMETHOD_POST:
        case HTTPMETHOD_HEAD:
        case HTTPMETHOD_SIMPLEGET:
			mpDebugPrint("HTTP server call");
            callback = gGetCallback;
            break;

        default:
            callback = NULL;
    }

    if( callback == NULL ) {
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    callback( hparser, &hparser->msg, info );
    return 0;
}

/************************************************************************
*	Function :	handle_error
*
*	Parameters :
*		
*		IN SOCKINFO *info ;		Socket Inforamtion Object
*		int http_error_code ;	HTTP Error Code
*		int major ;				Major Version Number
*		int minor ;				Minor Version Number
*
*	Description :	Send Error Message
*
*	Return : void;
*
*	Note :
************************************************************************/
static XINLINE void
handle_error( IN SOCKINFO * info,
              int http_error_code,
              int major,
              int minor )
{
    http_SendStatusResponse( info, http_error_code, major, minor );
}

/************************************************************************
*	Function :	free_handle_request_arg
*
*	Parameters :
*		void *args ; Request Message to be freed
*
*	Description :	Free memory assigned for handling request and unitial-
*	-ize socket functionality
*
*	Return :	void
*
*	Note :
************************************************************************/
static void
free_handle_request_arg( void *args )
{
    struct mserv_request_t *request = ( struct mserv_request_t * )args;

    //shutdown( request->connfd, SD_BOTH );
    closesocket( request->connfd );
    ixml_mem_free( request );
}

/************************************************************************
*	Function :	handle_request
*
*	Parameters :
*		void *args ;	Request Message to be handled
*
*	Description :	Receive the request and dispatch it for handling
*
*	Return :	void
*
*	Note :
************************************************************************/
static void
handle_request( void *args )
{
    SOCKINFO info;
    int http_error_code;
    int ret_code;
    int major = 1;
    int minor = 1;
    http_parser_t parser;
    http_message_t *hmsg = NULL;
    int timeout = HTTP_DEFAULT_TIMEOUT;
    struct mserv_request_t *request = ( struct mserv_request_t * )args;
    int connfd = request->connfd;

    DBGONLY( UpnpPrintf
             ( UPNP_INFO, MSERV, __FILE__, __LINE__,
               "miniserver %d: READING\n", connfd );
         )
        //parser_request_init( &parser ); ////LEAK_FIX_MK
	hmsg = &parser.msg;
	//mpDebugPrint("sock_init_with_ip");
    if( sock_init_with_ip( &info, connfd, request->foreign_ip_addr,
                           request->foreign_ip_port ) != UPNP_E_SUCCESS ) {
        ixml_mem_free( request );
        httpmsg_destroy( hmsg );
        return;
    }
    // read
	mpDebugPrint("http_RecvMessage");
    ret_code = http_RecvMessage( &info, &parser, HTTPMETHOD_UNKNOWN,&timeout, &http_error_code );
    if( ret_code != 0 ) {
        goto error_handler;
    }
	mpDebugPrint("QQ1");
    DBGONLY( UpnpPrintf
             ( UPNP_INFO, MSERV, __FILE__, __LINE__,
               "miniserver %d: PROCESSING...\n", connfd );
         )
	// dispatch
	http_error_code = dispatch_request( &info, &parser );
	mpDebugPrint("QQ2 %d",http_error_code);
    if( http_error_code != 0 ) {
        goto error_handler;
    }
    http_error_code = 0;

  error_handler:
    if( http_error_code > 0 ) {
        if( hmsg ) {
            major = hmsg->major_version;
            minor = hmsg->minor_version;
        }
        handle_error( &info, http_error_code, major, minor );
    }
    DBGONLY( UpnpPrintf
             ( UPNP_INFO, MSERV, __FILE__, __LINE__,
               "miniserver %d: COMPLETE\n", connfd );
         )
	sock_destroy( &info, SD_BOTH ); //should shutdown completely


    httpmsg_destroy( hmsg );
    //ixml_mem_free( request );
}

/************************************************************************
*	Function :	schedule_request_job
*
*	Parameters :
*		IN int connfd ;	Socket Descriptor on which connection is accepted
*		IN struct sockaddr_in* clientAddr ;	Clients Address information
*
*	Description :	Initilize the thread pool to handle a request.
*		Sets priority for the job and adds the job to the thread pool
*
*
*	Return :	void
*
*	Note :
************************************************************************/
static XINLINE void
schedule_request_job( IN int connfd,
                      IN struct sockaddr_in *clientAddr )
{
    struct mserv_request_t *request;
    //ThreadPoolJob job;

    request =
        ( struct mserv_request_t * )
        ixml_mem_malloc( sizeof( struct mserv_request_t ) );
    if( request == NULL ) {
        DBGONLY( UpnpPrintf
                 ( UPNP_INFO, MSERV, __FILE__, __LINE__,
                   "mserv %d: out of memory\n", connfd );
             )
            //shutdown( request->connfd, SD_BOTH );
        UpnpCloseSocket( connfd );
        return;
    }

    request->connfd = connfd;
    request->foreign_ip_addr = clientAddr->sin_addr;
    request->foreign_ip_port = ntohs( clientAddr->sin_port );
	handle_request(( void * )request);
	//Kevin Add
	free_handle_request_arg(( void * )request);
/*
    TPJobInit( &job, ( start_routine ) handle_request, ( void * )request );
    TPJobSetFreeFunction( &job, free_handle_request_arg );
    TPJobSetPriority( &job, MED_PRIORITY );
*/
	//KK8mpDebugPrint("########gRecvThreadPool###########");
/*
    if( ThreadPoolAdd( &gRecvThreadPool, &job, NULL ) != 0 ) {
        DBGONLY( UpnpPrintf
                 ( UPNP_INFO, MSERV, __FILE__, __LINE__,
                   "mserv %d: cannot schedule request\n", connfd );
             )
            ixml_mem_free( request );
        //shutdown( connfd, SD_BOTH );
        UpnpCloseSocket( connfd );
        return;
    }
*/
}

/************************************************************************
*	Function :	RunMiniServer
*
*	Parameters :
*		MiniServerSockArray *miniSock ;	Socket Array
*
*	Description :	Function runs the miniserver. The MiniServer accepts a 
*		new request and schedules a thread to handle the new request.
*		Checks for socket state and invokes appropriate read and shutdown 
*		actions for the Miniserver and SSDP sockets 
*
*	Return :	void
*
*	Note :
************************************************************************/
MiniServerSockArray *gminisocket;
void RunMiniServer( void/*MiniServerSockArray * miniSock*/ )
{
	MiniServerSockArray *miniSock;
    struct sockaddr_in clientAddr;
    socklen_t clientLen;
    SOCKET miniServSock,
      connectHnd;
    SOCKET miniServStopSock;
    SOCKET ssdpSock;

    ST_SOCK_SET stReadSet, stWriteSet;
    ST_SOCK_SET *wfds, *rfds;

    //CLIENTONLY( SOCKET ssdpReqSock;
      //   )

    fd_set expSet;
    fd_set rdSet;
    unsigned int maxMiniSock;
    int byteReceived;
    char requestBuf[256];
	//static unsigned short wcount = 0;
	//KK gssdp_send_discover = 1;
	//Kevin Add
	unsigned int mevent;
	struct timeval tv;
	int selstatus;
waitrunminiserver:
	mevent = 0x8;
	dma_invalid_dcache();
	EventWait(UPNP_START_EVENT, 0x200, OS_EVENT_OR, &mevent);
	mpDebugPrint("UPNP::StartMiniServer::mevent %x ",mevent);
    gMServState = MSERV_RUNNING;
	miniSock = gminisocket;
	//mpDebugPrint("miniSock %p",miniSock);
    miniServSock = miniSock->miniServerSock;
    //miniServStopSock = miniSock->miniServerStopSock;

    //ssdpSock = miniSock->ssdpSock;

    //CLIENTONLY( ssdpReqSock = miniSock->ssdpReqSock;
      //   );

    //maxMiniSock = max( miniServSock, miniServStopSock );
	//maxMiniSock = miniServSock;
    //maxMiniSock = max( maxMiniSock, ( SOCKET ) ( ssdpSock ) );


    while( TRUE ) {

		miniServSock = miniSock->miniServerSock;

		MPX_FD_ZERO(&stReadSet);
		MPX_FD_ZERO(&stWriteSet);
		wfds = rfds = NULL;
		rfds = &stReadSet;
		MPX_FD_SET(miniServSock, &stReadSet);

		//mpDebugPrint("select=============>");
		tv.tv_sec = 3;
		tv.tv_usec = 0;

		selstatus = select(0, rfds, NULL, 0, &tv);
		if( selstatus > 0 )
		{
			mpDebugPrint("Kevin FD_ISSET========>");
            clientLen = sizeof( struct sockaddr_in );
            connectHnd = accept( miniServSock,
                                 ( struct sockaddr * )&clientAddr,
                                 &clientLen );
			mpDebugPrint("Kevin FD_ISSET========>22 %d",connectHnd);
            if( connectHnd == UPNP_INVALID_SOCKET ) {
				mpDebugPrint("connectHnd == UPNP_INVALID_SOCKET");
                DBGONLY( UpnpPrintf
                         ( UPNP_INFO, MSERV, __FILE__, __LINE__,
                           "miniserver: Error"
                           " in accepting connection\n" );
                     )
                    continue;
            }
			mpDebugPrint("schedule_request_job 1");
            schedule_request_job( connectHnd, &clientAddr );
			mpDebugPrint("schedule_request_job 2");
		}
		else
		{
			MP_DEBUG("No Packet");
		}
		if( bexit_upupctrl & 0x1)
		{
			bexit_upupctrl |= 0x4;
//			mpDebugPrint("bexit_upupctrl %x",bexit_upupctrl);
			break;
		}
		TaskYield();
    }
    //shutdown( miniServSock, SD_BOTH );
	//shutdown(miniServSock,1);
	//TaskYield();
    closesocket( miniServSock );
    //shutdown( miniServStopSock, SD_BOTH );
    //UpnpCloseSocket( miniServStopSock );
    //shutdown( ssdpSock, SD_BOTH );
    //closesocket( ssdpSock );
    //CLIENTONLY( shutdown( ssdpReqSock, SD_BOTH ) );
    //CLIENTONLY( UpnpCloseSocket( ssdpReqSock ) );

    ixml_mem_free( miniSock );

    gMServState = MSERV_IDLE;
	MP_DEBUG("goto waitrunminiserver");
	goto waitrunminiserver;
    return;

}

void RunSSDPServer( void )
{
    SOCKET ssdpSock;
	unsigned int mevent;
	struct timeval tv;
	int selstatus;
    ST_SOCK_SET stReadSet, stWriteSet;
    ST_SOCK_SET *wfds, *rfds;
	u8 adcount = 0 ;


waitrunssdpserver:
	mevent = 0x2;
	dma_invalid_dcache();
	//EventClear(UPNP_REPLY_TASK_START,0);
	//mpDebugPrint("RunSSDPServer UPNP_REPLY_TASK_START %x",UPNP_REPLY_TASK_START);
	//mpx_EventWait(UPNP_REPLY_TASK_START, mevent, OS_EVENT_OR, &mevent);
	EventWait(UPNP_START_EVENT, 0x400, OS_EVENT_OR, &mevent);
	mpDebugPrint("RunSSDPServer %x",mevent);

    ssdpSock = gminisocket->ssdpSock;		


	while(1)
	{
		MPX_FD_ZERO(&stReadSet);
		MPX_FD_ZERO(&stWriteSet);
		wfds = rfds = NULL;
		rfds = &stReadSet;
		MPX_FD_SET(ssdpSock, &stReadSet);

		tv.tv_sec = 3;
		tv.tv_usec = 0;

		selstatus = select(0, rfds, NULL, 0, &tv);
		if( selstatus > 0 )
		{
			MP_DEBUG("RunSSDPServer selstatus %x",selstatus);
			//readFromSSDPSocket( ssdpSock );
			readFromSSDPSocket_M(ssdpSock );
			MP_DEBUG("selstatus exit");
		}
		else
			MP_DEBUG("NO Nulticast Packet");
//		mpDebugPrint("bexit_upupctrl %x",bexit_upupctrl);
		if( bexit_upupctrl & 0x1)
		{
			bexit_upupctrl |= 0x2;
			mpDebugPrint("bexit_upupctrl %x",bexit_upupctrl);
			closesocket(ssdpSock);
			break;
		}
		TaskYield();
		adcount++;
		if( g_bXpgStatus == XPG_MODE_DLNA_1_5_DMR )
		{
			if( (adcount > 10)  )
			{
				mpDebugPrint("Set AutoAdvertisetimer");
				//AddTimerProc(5000,AutoAdvertisetimer);
				AutoAdvertisetimer();
				//advertizeexpire = 0;
				adcount = 0;
			}
		}

	}
	goto waitrunssdpserver;
}

/************************************************************************
*	Function :	get_port
*
*	Parameters :
*		int sockfd ; Socket Descriptor 
*
*	Description :	Returns port to which socket, sockfd, is bound.
*
*	Return :	int, 
*		-1 on error; check errno
*		 > 0 means port number
*
*	Note :
************************************************************************/
/*KK
static int
get_port( int sockfd )
{
    struct sockaddr_in sockinfo;
    socklen_t len;
    int code;
    int port;

    len = sizeof( struct sockaddr_in );
    code = getsockname( sockfd, ( struct sockaddr * )&sockinfo, &len );
    if( code == -1 ) {
        return -1;
    }

    port = ntohs( sockinfo.sin_port );
    DBGONLY( UpnpPrintf
             ( UPNP_INFO, MSERV, __FILE__, __LINE__,
               "sockfd = %d, .... port = %d\n", sockfd, port );
         )

        return port;
}
*/
/************************************************************************
*	Function :	get_miniserver_sockets
*
*	Parameters :
*		MiniServerSockArray *out ;	Socket Array
*		unsigned short listen_port ; port on which the server is listening 
*									for incoming connections	
*
*	Description :	Creates a STREAM socket, binds to INADDR_ANY and 
*		listens for incoming connecttions. Returns the actual port which 
*		the sockets sub-system returned. 
*		Also creates a DGRAM socket, binds to the loop back address and 
*		returns the port allocated by the socket sub-system.
*
*	Return :	int : 
*		UPNP_E_OUTOF_SOCKET - Failed to create a socket
*		UPNP_E_SOCKET_BIND - Bind() failed
*		UPNP_E_LISTEN	- Listen() failed	
*		UPNP_E_INTERNAL_ERROR - Port returned by the socket layer is < 0
*		UPNP_E_SUCCESS	- Success
*		
*	Note :
************************************************************************/

int
get_miniserver_sockets( MiniServerSockArray * out,
                        unsigned short listen_port )
{
    struct sockaddr_in serverAddr;
    int listenfd;
    int success;
    unsigned short actual_port;
    int reuseaddr_on = 0;
    int sockError = UPNP_E_SUCCESS;
    int errCode = 0;
    int miniServerStopSock;

    listenfd = socket( AF_INET, SOCK_STREAM, 0 );
    if( listenfd < 0 ) {
        return UPNP_E_OUTOF_SOCKET; // error creating socket
    }
    // As per the IANA specifications for the use of ports by applications
    // override the listen port passed in with the first available 
    if( listen_port < APPLICATION_LISTENING_PORT )
        listen_port = APPLICATION_LISTENING_PORT;

    memset( &serverAddr, 0, sizeof( serverAddr ) );
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl( INADDR_ANY );

    // Getting away with implementation of re-using address:port and instead 
    // choosing to increment port numbers.
    // Keeping the re-use address code as an optional behaviour that can be 
    // turned on if necessary. 
    // TURN ON the reuseaddr_on option to use the option.
    if( reuseaddr_on ) {
        //THIS IS ALLOWS US TO BIND AGAIN IMMEDIATELY
        //AFTER OUR SERVER HAS BEEN CLOSED
        //THIS MAY CAUSE TCP TO BECOME LESS RELIABLE
        //HOWEVER IT HAS BEEN SUGESTED FOR TCP SERVERS

        DBGONLY( UpnpPrintf( UPNP_INFO, MSERV, __FILE__, __LINE__,
                             "mserv start: resuseaddr set\n" );
             )

            sockError = setsockopt( listenfd,
                                    SOL_SOCKET,
                                    SO_REUSEADDR,
                                    ( const char * )&reuseaddr_on,
                                    sizeof( int )
             );
        if( sockError == UPNP_SOCKETERROR ) {
            shutdown( listenfd, SD_BOTH );
            UpnpCloseSocket( listenfd );
            return UPNP_E_SOCKET_BIND;
        }

        sockError = bind( listenfd,
                          ( struct sockaddr * )&serverAddr,
                          sizeof( struct sockaddr_in )
             );
    } else {
        do {
            serverAddr.sin_port = htons( listen_port++ );
            sockError = bind( listenfd,
                              ( struct sockaddr * )&serverAddr,
                              sizeof( struct sockaddr_in )
                 );
            if( sockError == UPNP_SOCKETERROR ) {
                if( errno == EADDRINUSE )
                    errCode = 1;
            } else
                errCode = 0;

        } while( errCode != 0 );
    }

    if( sockError == UPNP_SOCKETERROR ) {
        DBGONLY( perror( "mserv start: bind failed" );
             )
            shutdown( listenfd, SD_BOTH );
        UpnpCloseSocket( listenfd );
        return UPNP_E_SOCKET_BIND;  // bind failed
    }

    DBGONLY( UpnpPrintf( UPNP_INFO, MSERV, __FILE__, __LINE__,
                         "mserv start: bind success\n" );
         )

        success = listen( listenfd, SOMAXCONN );
    if( success == UPNP_SOCKETERROR ) {
        shutdown( listenfd, SD_BOTH );
        UpnpCloseSocket( listenfd );
        return UPNP_E_LISTEN;   // listen failed
    }

    //actual_port = get_port( listenfd );
	actual_port = 49152;
    if( actual_port <= 0 ) {
        shutdown( listenfd, SD_BOTH );
        UpnpCloseSocket( listenfd );
        return UPNP_E_INTERNAL_ERROR;
    }

    out->miniServerPort = actual_port;
#if 0
    if( ( miniServerStopSock = socket( AF_INET, SOCK_DGRAM/*SOCK_STREAM*/, 0 ) ) ==
        UPNP_INVALID_SOCKET ) {
        DBGONLY( UpnpPrintf( UPNP_CRITICAL,
                             MSERV, __FILE__, __LINE__,
                             "Error in socket operation !!!\n" );
             )
            shutdown( listenfd, SD_BOTH );
        UpnpCloseSocket( listenfd );
        return UPNP_E_OUTOF_SOCKET;
    }

    // bind to local socket
    memset( ( char * )&serverAddr, 0, sizeof( struct sockaddr_in ) );
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

    if( bind( miniServerStopSock, ( struct sockaddr * )&serverAddr,
              sizeof( serverAddr ) ) == UPNP_SOCKETERROR ) {

        DBGONLY( UpnpPrintf( UPNP_CRITICAL,
                             MSERV, __FILE__, __LINE__,
                             "Error in binding localhost!!!\n" );
             )
            shutdown( listenfd, SD_BOTH );
        UpnpCloseSocket( listenfd );
        shutdown( miniServerStopSock, SD_BOTH );
        UpnpCloseSocket( miniServerStopSock );
        return UPNP_E_SOCKET_BIND;
    }

    //KK miniStopSockPort = get_port( miniServerStopSock );
	miniStopSockPort = 49153;
    if( miniStopSockPort <= 0 ) {
        shutdown( miniServerStopSock, SD_BOTH );
        shutdown( listenfd, SD_BOTH );
        UpnpCloseSocket( miniServerStopSock );
        UpnpCloseSocket( listenfd );
        return UPNP_E_INTERNAL_ERROR;
    }

    out->stopPort = miniStopSockPort;
#endif
    out->miniServerSock = listenfd;
    //out->miniServerStopSock = miniServerStopSock;

    return UPNP_E_SUCCESS;

}

/************************************************************************
*	Function :	StartMiniServer
*
*	Parameters :
*		unsigned short listen_port ; Port on which the server listens for 
*									incoming connections
*
*	Description :	Initialize the sockets functionality for the 
*		Miniserver. Initialize a thread pool job to run the MiniServer
*		and the job to the thread pool. If listen port is 0, port is 
*		dynamically picked
*
*		Use timer mechanism to start the MiniServer, failure to meet the 
*		allowed delay aborts the attempt to launch the MiniServer.
*
*	Return : int ;
*		Actual port socket is bound to - On Success: 
*		A negative number UPNP_E_XXX - On Error   			
*	Note :
************************************************************************/

int
StartMiniServer( unsigned short listen_port )
{

    int success;

    int count;
    int max_count = 10000;
    int status;
    MiniServerSockArray *miniSocket;
    ThreadPoolJob job;
    DWORD previousTime = 0;
    DWORD currentTime = 0;
    gMServState = MSERV_IDLE;
	MP_DEBUG("StartMiniServer====> %x %x",gMServState,MSERV_IDLE);
    if( gMServState != MSERV_IDLE ) 
	{
        return UPNP_E_INTERNAL_ERROR;   // miniserver running
    }
	mpDebugPrint("###################StartMiniServer 1");
    miniSocket =
        ( MiniServerSockArray * ) ixml_mem_malloc( sizeof( MiniServerSockArray ) );
    if( miniSocket == NULL )
        return UPNP_E_OUTOF_MEMORY;
	mpDebugPrint("#####################StartMiniServer 2");
    if( ( success = get_miniserver_sockets( miniSocket, listen_port ) )
        != UPNP_E_SUCCESS ) {
        ixml_mem_free( miniSocket );
        return success;
    }
    if( ( success = get_ssdp_sockets( miniSocket ) ) != UPNP_E_SUCCESS ) {

        shutdown( miniSocket->miniServerSock, SD_BOTH );
        UpnpCloseSocket( miniSocket->miniServerSock );
        //shutdown( miniSocket->miniServerStopSock, SD_BOTH );
        //UpnpCloseSocket( miniSocket->miniServerStopSock );

        ixml_mem_free( miniSocket );

        return success;
    }
	mpDebugPrint("StartMiniServer");
/*
    TPJobInit( &job, ( start_routine ) RunMiniServer,
               ( void * )miniSocket );
    TPJobSetPriority( &job, MED_PRIORITY );

    TPJobSetFreeFunction( &job, ( free_routine ) free );

    success = ThreadPoolAddPersistent( &gRecvThreadPool, &job, NULL );

    if( success < 0 ) {
        shutdown( miniSocket->miniServerSock, SD_BOTH );
        shutdown( miniSocket->miniServerStopSock, SD_BOTH );
        shutdown( miniSocket->ssdpSock, SD_BOTH );
        CLIENTONLY( shutdown( miniSocket->ssdpReqSock, SD_BOTH ) );
        UpnpCloseSocket( miniSocket->miniServerSock );
        UpnpCloseSocket( miniSocket->miniServerStopSock );
        UpnpCloseSocket( miniSocket->ssdpSock );

        CLIENTONLY( UpnpCloseSocket( miniSocket->ssdpReqSock ) );

        return UPNP_E_OUTOF_MEMORY;
    }
*/
	gminisocket = miniSocket;
	dma_invalid_dcache();
	EventSet(UPNP_START_EVENT, 0x200);
	TaskYield();
	EventSet(UPNP_START_EVENT, 0x400);


	//TaskCreate(30, RunMiniServer,2,0x1000);
	//TaskStartup(INTERNET_RADIO_TASK);
	//RunMiniServer();
	//RunMiniServer(miniSocket);
    // wait for miniserver to start
    count = 0;
	//previousTime = GetSysTime();
    while( gMServState != MSERV_RUNNING && count < max_count ) {
        //usleep( 50 * 1000 );    // 0.05s
		//Kevin Change
		TaskYield();
		//mpx_EventSet(UPNP_REPLY_TASK_START, 0x8);
        count++;
    }
    // taking too long to start that thread
	if( gMServState != MSERV_RUNNING )
	{

		mpDebugPrint("taking too long to start that thread");
        shutdown( miniSocket->miniServerSock, SD_BOTH );
        //shutdown( miniSocket->miniServerStopSock, SD_BOTH );
        shutdown( miniSocket->ssdpSock, SD_BOTH );
        //CLIENTONLY( shutdown( miniSocket->ssdpReqSock, SD_BOTH ) );

        UpnpCloseSocket( miniSocket->miniServerSock );
        //UpnpCloseSocket( miniSocket->miniServerStopSock );
        UpnpCloseSocket( miniSocket->ssdpSock );
        //CLIENTONLY( UpnpCloseSocket( miniSocket->ssdpReqSock ) );

        return UPNP_E_INTERNAL_ERROR;
    }
    return miniSocket->miniServerPort;
}

void UpnpdownloadTask(void)
{

	DWORD dwEvent, dwNextEvent;
	//netfs_meta_entry_t *cur_netfs_entry;
	int mfilesize; 
	unsigned int offset1,offset2;
	char *upnp_url;
	int range_size;
	u8 mretry;
	u16 i;

wait_to_start:
	dwNextEvent = 0;
	dwEvent = 0;
	EventWait(UPNP_START_EVENT, 0x00010000, OS_EVENT_OR, &dwEvent);
	MP_DEBUG("UpnpdownloadTask Task Go Go Go bdlnadownload %d %d\n",bdlnadownload,dwEvent);

	if( g_bXpgStatus == XPG_MODE_DLNA_1_5_DMR )
	{
		if( bdlnadownload )
		{
			xpgViewPhoto();
			bdlnadownload = 0;
		}
		goto wait_to_start;
		
	}
	else if( g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
	{
		//cur_netfs_entry = (netfs_meta_entry_t *) Net_Get_UpnpCurEntry();
		upnp_url = Net_Get_UpnpCurURL();
		mfilesize = Net_Get_UpnpCurFileSize();
		mpDebugPrint("url %s",upnp_url);
		mpDebugPrint("filesize %d",mfilesize);
		mretry = 0;
		bufindex = 0;
		gtotoalbuf = 0;
		last_one_kbytes_buf = (unsigned char *) ixml_mem_malloc(1024);
		range_size = Get_Image_File_Range(upnp_url,last_one_kbytes_buf,mfilesize-1024,mfilesize-1,FALSE);
		mpDebugPrint("range_size %d",range_size);
		last_one_kbytes_index = 1024;
		last_one_kbytes_start_addr = mfilesize-1024;
		if( range_size != 1024 )
		{
			memset(last_one_kbytes_buf,0,1024);
		}
		if( NetConnected() == FALSE )
		{
			if (g_bAniFlag & ANI_AUDIO)
				xpgStopAudio();
			quitpnpdownload = TRUE;
			goto exitupnpdownload;
		}
		mpDebugPrint("%02x %02x",last_one_kbytes_buf[0],last_one_kbytes_buf[1]);
		offset1 = 0;
		offset2 = UPNP_AUDIO_BLOCK_SIZE -1;
		if( offset2 > mfilesize - 1 )
			offset2 = mfilesize - 1;
		do{
			if( offset1 >= mfilesize )
				break;
			mretry = 0;
upnp_retry:
			mpDebugPrint("offset1 %x",offset1);
			range_size = Get_Image_File_Range(upnp_url,&(girbuf[bufindex]),offset1,offset2,FALSE);
			//MP_DEBUG("range_size %d %d",range_size,bufindex);
			if( NetConnected() == FALSE )
			{
				quitpnpdownload = TRUE;
				break;
			}
			if( range_size != ( offset2 - offset1 +1 ) && mretry < 3 )
			{
				mretry++;
				goto upnp_retry;
			}
            bufindex += (offset2 - offset1 +1);
			gtotoalbuf += (offset2 - offset1 +1);
			if( !irready && ( bufindex > 128 * 1024) )
				irready = 1;
            if( bufindex >= NETSTREAM_MAX_BUFSIZE )
            {
                bufindex = (bufindex % (NETSTREAM_MAX_BUFSIZE));
			}
			while( quitpnpdownload == FALSE )
			{
				if( NetConnected() == FALSE )
				{
					quitpnpdownload = TRUE;
				}
				if( (offset1 - kcurindx) < 129 *1024 )
				{
					//Continue to recv the audio data.
					break;
				}

#if 0
				if( i % 1024 == 0 )
				{
					dma_invalid_dcache();
					UartOutText("W");
				}
				i++;
#endif
				TaskYield();
			}
			offset1 = offset2 +1;
			offset2 += UPNP_AUDIO_BLOCK_SIZE;
			if( offset2 > mfilesize - 1 )
				offset2 = mfilesize - 1;
			TaskYield();
		}
		while( quitpnpdownload == FALSE );
	}
exitupnpdownload:
	if( last_one_kbytes_buf )
		ixml_mem_free(last_one_kbytes_buf);
	last_one_kbytes_buf = NULL;
	last_one_kbytes_index = 0;
	bupnpdownload = FALSE;
	if( NetConnected() == FALSE )
	{
		if( !bexit_upupctrl )
			bexit_upupctrl = 1;
	}
	goto wait_to_start;
	return;
}

int UPNP_Stream_File_Read(BYTE *buf,int len)
{
	SWORD ret;
	int len_ori;
	int dwTmp;
	DWORD *pdwSrc, *pdwTar;
	int numByteRead;

	if( (kcurindx > gtotoalbuf)  )
	{
		mpDebugPrint("kcurindx %d %d",kcurindx,last_one_kbytes_start_addr);
		last_one_kbytes_index = kcurindx - last_one_kbytes_start_addr;
		dma_invalid_dcache();
		mpDebugPrint("len %d %d",len,1024 - last_one_kbytes_index);
		if( len > (1024 - last_one_kbytes_index ) )
		{
			mpDebugPrint("len > 1024 - last_one_kbytes_index");
			memcpy(buf,last_one_kbytes_buf+last_one_kbytes_index,1024 - last_one_kbytes_index);
			memset(buf+1024-last_one_kbytes_index,0,len-(1024-last_one_kbytes_index));
		}
		else
			memcpy(buf,last_one_kbytes_buf+last_one_kbytes_index,len);
		mpDebugPrint("%02x %02x %02x %02x",buf[0],buf[1],buf[len-2],buf[len-1]);
		return len;
	}
	if( /*!iradio_wait &&*/ (kcurindx+len) < gtotoalbuf )
	{
		TaskYield();
		len_ori = len;
		//len = MIN(len,gtotaldata);
		dma_invalid_dcache();
		memcpy(buf,&girbuf[kcurindx%(NETSTREAM_MAX_BUFSIZE)],len);
		numByteRead = len;
		kcurindx += len;
		gtotaldata -= len;
		MP_DEBUG("FILE_TYPE_MP3 numByteRead %x len %x from %x %x real %x\n",numByteRead,len_ori,kcurindx,gtotoalbuf,kcurindx%(NETSTREAM_MAX_BUFSIZE));
		TaskYield();
	}
	else
	{
		TaskYield();
		len = 0;
		iradio_wait = 1;
		MP_DEBUG("MIPS_MP3::iradio_wait");
		TaskYield();
	}
    return len;
}

/************************************************************************
*	Function :	StopMiniServer
*
*	Parameters :
*		void ;	
*
*	Description :	Stop and Shutdown the MiniServer and free socket 
*		resources.
*
*	Return : int ;
*		Always returns 0 
*
*	Note :
************************************************************************/
/*
int
StopMiniServer( void )
{

    int socklen = sizeof( struct sockaddr_in ),
      sock;
    struct sockaddr_in ssdpAddr;
    char buf[256] = "ShutDown";
    int bufLen = strlen( buf );

    if( gMServState == MSERV_RUNNING )
        gMServState = MSERV_STOPPING;
    else
        return 0;

    sock = socket( AF_INET, SOCK_DGRAM, 0 );
    if( sock == UPNP_INVALID_SOCKET ) {
        DBGONLY( UpnpPrintf
                 ( UPNP_INFO, SSDP, __FILE__, __LINE__,
                   "SSDP_SERVER:StopSSDPServer: Error in socket operation !!!\n" );
             )
            return 0;
    }

    while( gMServState != MSERV_IDLE ) {
        ssdpAddr.sin_family = AF_INET;
        ssdpAddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
        ssdpAddr.sin_port = htons( miniStopSockPort );
        sendto( sock, buf, bufLen, 0, ( struct sockaddr * )&ssdpAddr,
                socklen );
        usleep( 1000 );
        if( gMServState == MSERV_IDLE )
            break;
        isleep( 1 );
    }
    //shutdown( sock, SD_BOTH );
    UpnpCloseSocket( sock );
    return 0;
}
*/
