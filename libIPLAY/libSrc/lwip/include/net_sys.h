

#ifndef __UIPSYS_H__
#define __UIPSYS_H__
#include "UtilTypeDef.h"
#include "global612.h"
#include "netware.h"

/*------------------------------------------------------------------------------*/

/**
 * \name Appication specific configurations
 * @{
 *
 * An uIP application is implemented using a single application
 * function that is called by uIP whenever a TCP/IP event occurs. The
 * name of this function must be registered with uIP at compile time
 * using the UIP_APPCALL definition.
 *
 * uIP applications can store the application state within the
 * uip_conn structure by specifying the type of the application
 * structure by typedef:ing the type uip_tcp_appstate_t and uip_udp_appstate_t.
 *
 * The file containing the definitions must be included in the
 * uipopt.h file.
 *
 * The following example illustrates how this can look.
 */

//#include "os_defs.h"

#define WEBCLIENT_CONF_MAX_URLLEN 256

typedef struct {
  u16_t port;
  u16_t getrequestptr;
  u16_t getrequestleft;
 
  u16_t httpheaderlineptr;

  char httpheaderline[256];

  char host[256];
  char file[WEBCLIENT_CONF_MAX_URLLEN];

  char mimetype[256];

  DWORD dwStart;	// abel 20070413 add for range
  DWORD dwEnd;		// abel 20070413 add for range

  u8_t timer;
  u8_t state;
  u8_t httpflag;
  
} web_client_state;


/**
 * \var #define UIP_APPCALL
 *
 * The name of the application function that uIP should call in
 * response to TCP/IP events.
 *
 */

/**
 * \var typedef uip_tcp_appstate_t
 *
 * The type of the application state that is to be stored in the
 * uip_conn structure. This usually is typedef:ed to a struct holding
 * application state information.
 */

/**
 * \var typedef uip_udp_appstate_t
 *
 * The type of the application state that is to be stored in the
 * uip_conn structure. This usually is typedef:ed to a struct holding
 * application state information.
 */
/** @} */

/**
 * Callback function that is called from the webclient code when HTTP
 * data has been received.
 *
 * This function must be implemented by the module that uses the
 * webclient code. The function is called from the webclient module
 * when HTTP data has been received. The function is not called when
 * HTTP headers are received, only for the actual data.
 *
 * \note This function is called many times, repetedly, when data is
 * being received, and not once when all data has been received.
 *
 * \param data A pointer to the data that has been received.
 * \param len The length of the data that has been received.
 */
void webclient_datahandler(char *data, u16_t len);

/**
 * Callback function that is called from the webclient code when the
 * HTTP connection has been connected to the web server.
 *
 * This function must be implemented by the module that uses the
 * webclient code.
 */
void webclient_connected(void);

/**
 * Callback function that is called from the webclient code if the
 * HTTP connection to the web server has timed out.
 *
 * This function must be implemented by the module that uses the
 * webclient code.
 */
void webclient_timedout(void);

/**
 * Callback function that is called from the webclient code if the
 * HTTP connection to the web server has been aborted by the web
 * server.
 *
 * This function must be implemented by the module that uses the
 * webclient code.
 */
void webclient_aborted(void);

/**
 * Callback function that is called from the webclient code when the
 * HTTP connection to the web server has been closed.
 *
 * This function must be implemented by the module that uses the
 * webclient code.
 */
void webclient_closed(void);



/**
 * Initialize the webclient module.
 */
void webclient_init(void);

/**
 * Open an HTTP connection to a web server and ask for a file using
 * the GET method.
 *
 * This function opens an HTTP connection to the specified web server
 * and requests the specified file using the GET method. When the HTTP
 * connection has been connected, the webclient_connected() callback
 * function is called and when the HTTP data arrives the
 * webclient_datahandler() callback function is called.
 *
 * The callback function webclient_timedout() is called if the web
 * server could not be contacted, and the webclient_aborted() callback
 * function is called if the HTTP connection is aborted by the web
 * server.
 *
 * When the HTTP request has been completed and the HTTP connection is
 * closed, the webclient_closed() callback function will be called.
 *
 * \note If the function is passed a host name, it must already be in
 * the resolver cache in order for the function to connect to the web
 * server. It is therefore up to the calling module to implement the
 * resolver calls and the signal handler used for reporting a resolv
 * query answer.
 *
 * \param host A pointer to a string containing either a host name or
 * a numerical IP address in dotted decimal notation (e.g., 192.168.23.1).
 *
 * \param port The port number to which to connect, in host byte order.
 *
 * \param file A pointer to the name of the file to get.
 *
 * \retval 0 if the host name could not be found in the cache, or
 * if a TCP connection could not be created.
 *
 * \retval 1 if the connection was initiated.
 */
unsigned char webclient_get(char *host, u16_t port, char *file, DWORD start, DWORD end);

/**
 * Close the currently open HTTP connection.
 */
void webclient_close(void);
void webclient_appcall(void);

/**
 * Obtain the MIME type of the current HTTP data stream.
 *
 * \return A pointer to a string contaning the MIME type. The string
 * may be empty if no MIME type was reported by the web server.
 */
char *webclient_mimetype(void);

/**
 * Obtain the filename of the current HTTP data stream.
 *
 * The filename of an HTTP request may be changed by the web server,
 * and may therefore not be the same as when the original GET request
 * was made with webclient_get(). This function is used for obtaining
 * the current filename.
 *
 * \return A pointer to the current filename.
 */
char *webclient_filename(void);

/**
 * Obtain the hostname of the current HTTP data stream.
 *
 * The hostname of the web server of an HTTP request may be changed
 * by the web server, and may therefore not be the same as when the
 * original GET request was made with webclient_get(). This function
 * is used for obtaining the current hostname.
 *
 * \return A pointer to the current hostname.
 */
char *webclient_hostname(void);

/**
 * Obtain the port number of the current HTTP data stream.
 *
 * The port number of an HTTP request may be changed by the web
 * server, and may therefore not be the same as when the original GET
 * request was made with webclient_get(). This function is used for
 * obtaining the current port number.
 *
 * \return The port number of the current HTTP data stream, in host byte order.
 */
unsigned short webclient_port(void);

// define the constant of State for Net_App_State structure
#define NET_NULL				0x00000000
#define NET_ENABLE				0x00000001
#define NET_PPP				    0x00000002
#define NET_CONFIGED			0x00000004
#define NET_TX_RX_DATA			0x00000008

#define NET_HOST_NOT_FOUND		0x00000010
#define NET_DISCONNECT			0x00000020
#define NET_TIMEOUT				0x00000040
#define NET_SKIP_ALL			0x00000080

#define NET_RECVHTTP			0x00100000
#if 0  // using --> NET_RECVBINARYDATA
#define NET_RECVPICASA			0x00200000
#define NET_RECVFLICKR			0x00400000		
#define NET_RECVGCE				0x00800000

#define NET_RECVPC				0x01000000
#define NET_RECVRSS				0x02000000
#define NET_RECVYOUGOTPHOTO		0x04000000
#define NET_RECVFRAMECHANNEL	0x00010000
#define NET_RECVFRAMEIT			0x00020000
#define NET_RECVSNAPFISH		0x00040000
#endif

#define NET_RECVUPNP			0x10000000
#define NET_RECVTEXTDATA		NET_RECVHTTP    /* to receive text data */
#define NET_RECVBINARYDATA		0x00008000      /* to receive binary data */
#define NET_LINKUP				0x08000000
#define NET_WLS                 0x20000000
#define NET_YOUTUBE             0x40000000
#define NET_YOUKU3G             0x80000000


// define the constant of dwNWEvent
#define NET_EVENT_PERIODIC_TIMER	0x00000001
#define NET_EVENT_ARP_TIMER			0x00000002


/**
 * @ingroup    NET_HTTP
 *
 * The buffer structure for HTTP transfer of a text file (e.g., XML file)
 */
typedef struct XML_BUFF_link XML_BUFF_link_t;
/**
 * @ingroup    NET_HTTP
 *
 * The buffer structure for HTTP transfer of a binary file (e.g., JPEG file)
 */
typedef struct XML_ImageBUFF_link XML_ImageBUFF_link_t;

struct XML_BUFF_link
{
	BYTE BUFF[IMAGE_BUF];
       int buff_len;
	struct XML_BUFF_link *link;
};

struct XML_ImageBUFF_link
{
	BYTE BUFF[IMAGE_BUF];
       int buff_len;
	struct XML_ImageBUFF_link *link;
};

typedef struct {
	DWORD dwDataPtr;					// The pointer of data
	DWORD dwDataLength;					// The length of data
	void (*Net_AP_CallBack) (void);		// The call back function for datNet_App_Statea access
	DWORD dwBuffer;
	DWORD dwOffset;
	DWORD dwState;						// The state of net AP
	DWORD dwEndToggle;	
	DWORD dwNWEvent;	
	
	DWORD dwTotallen;
       DWORD dwReconnect;
	int type;
	int  HttpStart;
	int  HttpEnd;
	int  HttpContentLength;
	int  HttpFileSize;
	XML_ImageBUFF_link_t *XML_BUF1,*qtr;
	XML_BUFF_link_t *XML_BUF,*ptr;
	
	int flags;                                  /* bit-mapped flags */
#define HTTP_TRANSFER_CANCEL 1                  /* http transfer is canceled */
#define HTTP_TRANSFER_PPP    2                  /* http transfer via PPP interface */
} Net_App_State;

/*
 * A network application instance
 */
typedef struct {
    int evt;
} Net_App;

/*
 * This structure holds all callbacks
 */
typedef struct {
	void (*netEvNetworkUp)(void *userdata);
} Net_Event;

int Net_Recv_Data(BYTE *url, BYTE type, DWORD size, DWORD timeout);
void Xml_BUFF_init(DWORD status);
void Xml_BUFF_free(DWORD status);

int Net_Recv_Data2(Net_App_State *app, BYTE *url, BYTE type, DWORD timeout, char *if_name);

#endif /* __UIPSYS_H__ */


