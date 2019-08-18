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

#include "config.h"
#if EXCLUDE_SSDP == 0

#include "membuffer.h"
#include "ssdplib.h"
#include <stdio.h>
#include "ThreadPool.h"
#include "miniserver.h"

#include "upnpapi.h"
#include "httpparser.h"
#include "httpreadwrite.h"
#include "global612.h"

#ifdef WIN32
 #include <winsock2.h>
 #include <ws2tcpip.h>
 #include "unixutil.h"
#endif
//Kevin ADD
#define IP_MULTICAST_IF			32
#define IP_MULTICAST_TTL 		33
#define IP_MULTICAST_LOOP 		34
#define IP_ADD_MEMBERSHIP		35
#define IP_DROP_MEMBERSHIP		36
#define IP_UNBLOCK_SOURCE		37
#define IP_BLOCK_SOURCE			38

unsigned char stop_listen_ssdp = 0;
struct ip_mreq 
{
	struct in_addr imr_multiaddr;	/* IP multicast address of group */
	struct in_addr imr_interface;	/* local IP address of interface */
};

#define MAX_TIME_TOREAD  45

CLIENTONLY( SOCKET gSsdpReqSocket = 0;
     )

     void RequestHandler(  );
     Event ErrotEvt;

     enum Listener { Idle, Stopping, Running };

     unsigned short ssdpStopPort;

     struct SSDPSockArray {
         int ssdpSock;          //socket for incoming advertisments and search requests
           CLIENTONLY( int ssdpReqSock;
              )                 //socket for sending search 
             //requests and receiving
             // search replies
     };

#ifdef INCLUDE_DEVICE_APIS
#if EXCLUDE_SSDP == 0

/************************************************************************
* Function : AdvertiseAndReply									
*																	
* Parameters:														
*	IN int AdFlag: -1 = Send shutdown, 0 = send reply, 
*					1 = Send Advertisement
*	IN UpnpDevice_Handle Hnd: Device handle
*	IN enum SsdpSearchType SearchType:Search type for sending replies
*	IN struct sockaddr_in *DestAddr:Destination address
*   IN char *DeviceType:Device type
*	IN char *DeviceUDN:Device UDN
*   IN char *ServiceType:Service type
*	IN int Exp:Advertisement age
*
* Description:														
*	This function sends SSDP advertisements, replies and shutdown messages.
*
* Returns: int
*	UPNP_E_SUCCESS if successful else appropriate error
***************************************************************************/
     int AdvertiseAndReply( IN int AdFlag,
                            IN UpnpDevice_Handle Hnd,
                            IN enum SsdpSearchType SearchType,
                            IN struct sockaddr_in *DestAddr,
                            IN char *DeviceType,
                            IN char *DeviceUDN,
                            IN char *ServiceType,
                            int Exp )
{
    int i,
      j;
    int defaultExp = DEFAULT_MAXAGE;
    struct Handle_Info *SInfo = NULL;
    char UDNstr[100],
      devType[100],
      servType[100];
    IXML_NodeList *nodeList = NULL;
    IXML_NodeList *tmpNodeList = NULL;
    IXML_Node *tmpNode = NULL;
    IXML_Node *tmpNode2 = NULL;
    IXML_Node *textNode = NULL;
    DOMString tmpStr;
    char SERVER[200];

    DBGONLY( const DOMString dbgStr;
             UpnpPrintf( UPNP_ALL, API, __FILE__, __LINE__,
                         "Inside AdvertiseAndReply with AdFlag = %d\n",
                         AdFlag ); )

	HandleLock(  );

    if( GetHandleInfo( Hnd, &SInfo ) != HND_DEVICE ) {
		mpDebugPrint("GetHandleInfo( Hnd, &SInfo ) != HND_DEVICE");
        HandleUnlock(  );
        return UPNP_E_INVALID_HANDLE;
    }

    defaultExp = SInfo->MaxAge;

    //Modifed to prevent more than one thread from accessing the 
    //UpnpDocument stored with the handle at the same time
    HandleUnlock();
    nodeList = NULL;

    //get server info

    //get_sdk_info( SERVER );
	mpDebugPrint("AdvertiseAndReply ==>1 ");
    // parse the device list and send advertisements/replies 
    for( i = 0;; i++ ) {

        DBGONLY( UpnpPrintf( UPNP_ALL, API, __FILE__, __LINE__,
                             "Entering new device list with i = %d\n\n",
                             i );
             )

            tmpNode = ixmlNodeList_item( SInfo->DeviceList, i );
        if( tmpNode == NULL ) {
            DBGONLY( UpnpPrintf( UPNP_ALL, API, __FILE__, __LINE__,
                                 "Exiting new device list with i = %d\n\n",
                                 i );
                 )
                break;
        }

        DBGONLY( dbgStr = ixmlNode_getNodeName( tmpNode );
                 UpnpPrintf( UPNP_INFO, API, __FILE__, __LINE__,
                             "Extracting device type once for %s\n",
                             dbgStr ); )
            // extract device type 
            ixmlNodeList_free( nodeList );
        nodeList = NULL;
        nodeList =
            ixmlElement_getElementsByTagName( ( IXML_Element * ) tmpNode,
                                              "deviceType" );
        if( nodeList == NULL ) {
            continue;
        }

        DBGONLY( dbgStr = ixmlNode_getNodeName( tmpNode );
                 UpnpPrintf( UPNP_ALL, API, __FILE__, __LINE__,
                             "Extracting UDN for %s\n", dbgStr ); )

            DBGONLY( UpnpPrintf( UPNP_ALL, API, __FILE__, __LINE__,
                                 "Extracting device type\n" );
             )

            tmpNode2 = ixmlNodeList_item( nodeList, 0 );
        if( tmpNode2 == NULL ) {
            continue;
        }
        textNode = ixmlNode_getFirstChild( tmpNode2 );
        if( textNode == NULL ) {
            continue;
        }

        DBGONLY( UpnpPrintf( UPNP_ALL, API, __FILE__, __LINE__,
                             "Extracting device type \n" );
             )

            tmpStr = ixmlNode_getNodeValue( textNode );
        if( tmpStr == NULL ) {
            continue;
        }

        strcpy( devType, tmpStr );
		mpDebugPrint("devType %s",devType);
        if( devType == NULL ) {
            continue;
        }

        DBGONLY( UpnpPrintf( UPNP_ALL, API, __FILE__, __LINE__,
                             "Extracting device type = %s\n", devType );
                 if( tmpNode == NULL ) {
                 UpnpPrintf( UPNP_ALL, API, __FILE__, __LINE__,
                             "TempNode is NULL\n" );}
                 dbgStr = ixmlNode_getNodeName( tmpNode );
                 UpnpPrintf( UPNP_ALL, API, __FILE__, __LINE__,
                             "Extracting UDN for %s\n", dbgStr ); )
            // extract UDN 
            ixmlNodeList_free( nodeList );
        nodeList = NULL;
        nodeList = ixmlElement_getElementsByTagName( ( IXML_Element * )
                                                     tmpNode, "UDN" );
        if( nodeList == NULL ) {

            DBGONLY( UpnpPrintf( UPNP_CRITICAL, API, __FILE__,
                                 __LINE__, "UDN not found!!!\n" );
                 )
                continue;
        }
        tmpNode2 = ixmlNodeList_item( nodeList, 0 );
        if( tmpNode2 == NULL ) {

            DBGONLY( UpnpPrintf( UPNP_CRITICAL, API, __FILE__,
                                 __LINE__, "UDN not found!!!\n" );
                 )
                continue;
        }
        textNode = ixmlNode_getFirstChild( tmpNode2 );
        if( textNode == NULL ) {

            DBGONLY( UpnpPrintf( UPNP_CRITICAL, API, __FILE__,
                                 __LINE__, "UDN not found!!!\n" );
                 )
                continue;
        }
        tmpStr = ixmlNode_getNodeValue( textNode );
        if( tmpStr == NULL ) {
            DBGONLY( UpnpPrintf( UPNP_CRITICAL, API, __FILE__, __LINE__,
                                 "UDN not found!!!!\n" );
                 )
                continue;
        }
        strcpy( UDNstr, tmpStr );
        if( UDNstr == NULL ) {
            continue;
        }

        DBGONLY( UpnpPrintf( UPNP_INFO, API, __FILE__, __LINE__,
                             "Sending UDNStr = %s \n", UDNstr );
             )
		mpDebugPrint( "Sending UDNStr = %s %d %d\n", UDNstr,AdFlag,SearchType);
		//AdFlag = 0;
		if( AdFlag ) {
            // send the device advertisement 
            if( AdFlag == 1 ) {
				mpDebugPrint("DeviceAdvertisement 1");
                DeviceAdvertisement( devType, i == 0,UDNstr, SInfo->DescURL, Exp );
				mpDebugPrint("DeviceAdvertisement 2");
            } else              // AdFlag == -1
            {
                DeviceShutdown( devType, i == 0, UDNstr,SERVER, SInfo->DescURL, Exp );
            }
        } else {
            switch ( SearchType ) {

                case SSDP_ALL:
					mpDebugPrint("DeviceReply=========>");
                    DeviceReply( DestAddr,devType, i == 0,UDNstr, SInfo->DescURL, defaultExp );
                    break;

                case SSDP_ROOTDEVICE:
                    if( i == 0 ) {
						mpDebugPrint("SSDP_ROOTDEVICE");
                        SendReply( DestAddr, devType, 1,UDNstr, SInfo->DescURL, defaultExp, 0 );
                    }
                    break;
                case SSDP_DEVICEUDN:
                    {
						mpDebugPrint("DeviceUDN  %s",DeviceUDN );
                        if( DeviceUDN != NULL && strlen( DeviceUDN ) != 0 ) 
						{
                            if( strcasecmp( DeviceUDN, UDNstr ) ) 
							{
									mpDebugPrint("DeviceUDN=%s and search "
                                           "UDN=%s did not match\n",
                                           UDNstr, DeviceUDN );
                                    break;
                            } else {
                                DBGONLY( UpnpPrintf
                                         ( UPNP_INFO, API, __FILE__,
                                           __LINE__,
                                           "DeviceUDN=%s and search "
                                           "UDN=%s MATCH\n", UDNstr,
                                           DeviceUDN );
                                     )
                                    SendReply( DestAddr, devType, 0,UDNstr, SInfo->DescURL,defaultExp, 0 );
                                break;
                            }
                        }
                    }
                case SSDP_DEVICETYPE:
                    {
                        if( !strncasecmp
                            ( DeviceType, devType,
                              strlen( DeviceType ) ) ) {
                            DBGONLY( UpnpPrintf
                                     ( UPNP_INFO, API, __FILE__, __LINE__,
                                       "DeviceType=%s and search devType=%s MATCH\n",
                                       devType, DeviceType );
                                 )
								mpDebugPrint("UU1");
                                SendReply( DestAddr, devType, 0, UDNstr,SInfo->DescURL, defaultExp, 1 );
								mpDebugPrint("UU2");
                        }

                        DBGONLY(
                                    else
                                    UpnpPrintf( UPNP_INFO, API, __FILE__,
                                                __LINE__,
                                                "DeviceType=%s and search devType=%s"
                                                " DID NOT MATCH\n",
                                                devType, DeviceType );
                             )

                            break;
                    }
                default:
                    break;
            }
        }
        // send service advertisements for services corresponding 
        // to the same device 
        DBGONLY( UpnpPrintf( UPNP_INFO, API, __FILE__, __LINE__,
                             "Sending service Advertisement\n" );
             )

            tmpNode = ixmlNodeList_item( SInfo->ServiceList, i );
        if( tmpNode == NULL ) {
            continue;
        }
        ixmlNodeList_free( nodeList );
        nodeList = NULL;
        nodeList = ixmlElement_getElementsByTagName( ( IXML_Element * )
                                                     tmpNode, "service" );
        if( nodeList == NULL ) {
            DBGONLY( UpnpPrintf( UPNP_INFO, API, __FILE__, __LINE__,
                                 "Service not found 3\n" );
                 )
                continue;
        }
        for( j = 0;; j++ ) {
            tmpNode = ixmlNodeList_item( nodeList, j );
            if( tmpNode == NULL )
                break;

            ixmlNodeList_free( tmpNodeList );
            tmpNodeList = NULL;
            tmpNodeList = ixmlElement_getElementsByTagName( ( IXML_Element
                                                              * ) tmpNode,
                                                            "serviceType" );

            if( tmpNodeList == NULL ) {
                DBGONLY( UpnpPrintf
                         ( UPNP_CRITICAL, API, __FILE__, __LINE__,
                           "ServiceType not found \n" );
                     )
                    continue;
            }
            tmpNode2 = ixmlNodeList_item( tmpNodeList, 0 );
            if( tmpNode2 == NULL ) {
                continue;
            }
            textNode = ixmlNode_getFirstChild( tmpNode2 );
            if( textNode == NULL ) {
                continue;
            }
            // servType is of format Servicetype:ServiceVersion
            tmpStr = ixmlNode_getNodeValue( textNode );
            if( tmpStr == NULL ) {
                continue;
            }
            strcpy( servType, tmpStr );
            if( servType == NULL ) {
                continue;
            }

            DBGONLY( UpnpPrintf( UPNP_INFO, API, __FILE__, __LINE__,
                                 "ServiceType = %s\n", servType );
                 )
                if( AdFlag ) {
                if( AdFlag == 1 ) {
                    //ServiceAdvertisement( UDNstr, servType,SInfo->DescURL, Exp );
                } else          // AdFlag == -1
                {
                    //ServiceShutdown( UDNstr, servType,SInfo->DescURL, Exp );
                }

            } else {
                switch ( SearchType ) 
				{
                    case SSDP_ALL:
                    {
                        //ServiceReply( DestAddr, servType,UDNstr, SInfo->DescURL,defaultExp );
                        break;
                    }
                    case SSDP_SERVICE:
					{
                            //if( ServiceType != NULL ) {
                              //  if( !strncasecmp( ServiceType,
                                //                  servType,
                                  //                strlen( ServiceType ) ) )
                                //{
                                    ServiceReply( DestAddr, servType,UDNstr, SInfo->DescURL,defaultExp );
                                //}
                    //}
						break;
					}
                    default:
                        break;
                }               // switch(SearchType)               

            }
        }
		//mpDebugPrint("ixmlNodeList_free a");
        ixmlNodeList_free( tmpNodeList );
        tmpNodeList = NULL;
		//mpDebugPrint("ixmlNodeList_free b");
        ixmlNodeList_free( nodeList );
        nodeList = NULL;
		//mpDebugPrint("ixmlNodeList_free c");
    }
    DBGONLY( UpnpPrintf( UPNP_ALL, API, __FILE__, __LINE__,
                         "Exiting AdvertiseAndReply : \n" );
         )

	//HandleUnlock(  );

    return UPNP_E_SUCCESS;

}  /****************** End of AdvertiseAndReply *********************/

#endif
#endif

/************************************************************************
* Function : Make_Socket_NoBlocking									
*																	
* Parameters:														
*	IN int sock: socket 
*
* Description:														
*	This function makes socket non-blocking.
*
* Returns: int
*	0 if successful else -1 
***************************************************************************/
int
Make_Socket_NoBlocking( int sock )
{
#ifdef WIN32
     u_long val=1;
     return ioctlsocket(sock, FIONBIO, &val);
#else
    int val;
/*Kevin Mark*/
/*
    val = fcntl( sock, F_GETFL, 0 );
    if( fcntl( sock, F_SETFL, val | O_NONBLOCK ) == -1 ) {
        return -1;
    }
*/
#endif
    return 0;
}

/************************************************************************
* Function : unique_service_name								
*																	
* Parameters:														
*	IN char *cmd: Service Name string 
*	OUT SsdpEvent *Evt: The SSDP event structure partially filled 
*						by all the function.
*
* Description:														
*	This function fills the fields of the event structure like DeviceType,
*	Device UDN and Service Type
*
* Returns: int
*	0 if successful else -1 
***************************************************************************/
int
unique_service_name( IN char *cmd,
                     IN SsdpEvent * Evt )
{
    char *TempPtr,
      TempBuf[COMMAND_LEN],
     *Ptr,
     *ptr1,
     *ptr2,
     *ptr3;
    int CommandFound = 0;

    if( ( TempPtr = strstr( cmd, "uuid:schemas" ) ) != NULL ) {

        ptr1 = strstr( cmd, ":device" );
        if( ptr1 != NULL ) {
            ptr2 = strstr( ptr1 + 1, ":" );
        } else {
            return -1;
        }

        if( ptr2 != NULL ) {
            ptr3 = strstr( ptr2 + 1, ":" );
        } else {
            return -1;
        }

        if( ptr3 != NULL ) {
            sprintf( Evt->UDN, "uuid:%s", ptr3 + 1 );
        } else {
            return -1;
        }

        ptr1 = strstr( cmd, ":" );
        if( ptr1 != NULL ) {
            strncpy( TempBuf, ptr1, ptr3 - ptr1 );
            TempBuf[ptr3 - ptr1] = '\0';
            sprintf( Evt->DeviceType, "urn%s", TempBuf );
        } else {
            return -1;
        }
        return 0;
    }

    if( ( TempPtr = strstr( cmd, "uuid" ) ) != NULL ) {
        if( ( Ptr = strstr( cmd, "::" ) ) != NULL ) {
            strncpy( Evt->UDN, TempPtr, Ptr - TempPtr );
            Evt->UDN[Ptr - TempPtr] = '\0';
        } else {
            strcpy( Evt->UDN, TempPtr );
        }
        CommandFound = 1;
    }

    if( strstr( cmd, "urn:" ) != NULL
        && strstr( cmd, ":service:" ) != NULL ) {

        if( ( TempPtr = strstr( cmd, "urn" ) ) != NULL ) {
            strcpy( Evt->ServiceType, TempPtr );
            CommandFound = 1;
        }
    }

    if( strstr( cmd, "urn:" ) != NULL
        && strstr( cmd, ":device:" ) != NULL ) {
        if( ( TempPtr = strstr( cmd, "urn" ) ) != NULL ) {
            strcpy( Evt->DeviceType, TempPtr );
            CommandFound = 1;
        }
    }

    if( CommandFound == 0 ) {

        return -1;
    }

    return 0;
}

/************************************************************************
* Function : ssdp_request_type1								
*																	
* Parameters:														
*	IN char *cmd: command came in the ssdp request
*
* Description:														
*	This function figures out the type of the SSDP search in the 
*	in the request.
*
* Returns: enum SsdpSearchType
*	return appropriate search type else returns SSDP_ERROR
***************************************************************************/
enum SsdpSearchType
ssdp_request_type1( IN char *cmd )
{
    if( strstr( cmd, ":all" ) != NULL ) {
        return SSDP_ALL;
    }
    if( strstr( cmd, ":rootdevice" ) != NULL ) {
        return SSDP_ROOTDEVICE;
    }
    if( strstr( cmd, "uuid:" ) != NULL ) {
        return SSDP_DEVICEUDN;
    }
    if( ( strstr( cmd, "urn:" ) != NULL )
        && ( strstr( cmd, ":device:" ) != NULL ) ) {
        return SSDP_DEVICETYPE;
    }
    if( ( strstr( cmd, "urn:" ) != NULL )
        && ( strstr( cmd, ":service:" ) != NULL ) ) {
        return SSDP_SERVICE;
    }
    return SSDP_SERROR;
}

/************************************************************************
* Function : ssdp_request_type								
*																	
* Parameters:														
*	IN char *cmd: command came in the ssdp request
*	OUT SsdpEvent *Evt: The event structure partially filled by
*		 this function.
*
* Description:														
*	This function starts filling the SSDP event structure based upon the 
*	request received. 
*
* Returns: int
*	0 on success; -1 on error
***************************************************************************/
int
ssdp_request_type( IN char *cmd,
                   OUT SsdpEvent * Evt )
{
    // clear event
    memset( Evt, 0, sizeof( SsdpEvent ) );
    unique_service_name( cmd, Evt );
    Evt->ErrCode = NO_ERROR_FOUND;

    if( ( Evt->RequestType = ssdp_request_type1( cmd ) ) == SSDP_SERROR ) {
        Evt->ErrCode = E_HTTP_SYNTEX;
        return -1;
    }
    return 0;
}

/************************************************************************
* Function : free_ssdp_event_handler_data								
*																	
* Parameters:														
*	IN void *the_data: ssdp_thread_data structure. This structure contains
*			SSDP request message.
*
* Description:														
*	This function frees the ssdp request
*
* Returns: VOID
*	
***************************************************************************/
static void
free_ssdp_event_handler_data( void *the_data )
{
    ssdp_thread_data *data = ( ssdp_thread_data * ) the_data;

    if( data != NULL ) {
        http_message_t *hmsg = &data->parser.msg;

        // free data
        httpmsg_destroy( hmsg );
		if(data)
			ixml_mem_free( data );
    }
}

/************************************************************************
* Function : valid_ssdp_msg								
*																	
* Parameters:														
*	IN void *the_data: ssdp_thread_data structure. This structure contains
*			SSDP request message.
*
* Description:														
*	This function do some quick checking of the ssdp msg
*
* Returns: xboolean
*	returns TRUE if msg is valid else FALSE
***************************************************************************/
static XINLINE xboolean
valid_ssdp_msg( IN http_message_t * hmsg )
{
    memptr hdr_value;

    // check for valid methods - NOTIFY or M-SEARCH
    if( hmsg->method != HTTPMETHOD_NOTIFY &&
        hmsg->method != HTTPMETHOD_MSEARCH
        && hmsg->request_method != HTTPMETHOD_MSEARCH ) {
        return FALSE;
    }
    if( hmsg->request_method != HTTPMETHOD_MSEARCH ) {
        // check PATH == *
        if( hmsg->uri.type != RELATIVE ||
            strncmp( "*", hmsg->uri.pathquery.buff,
                     hmsg->uri.pathquery.size ) != 0 ) {
            return FALSE;
        }
        // check HOST header
        if( ( httpmsg_find_hdr( hmsg, HDR_HOST, &hdr_value ) == NULL ) ||
            ( memptr_cmp( &hdr_value, "239.255.255.250:1900" ) != 0 )
             ) {
            return FALSE;
        }
    }
    return TRUE;                // passed quick check
}

/************************************************************************
* Function : start_event_handler								
*																	
* Parameters:														
*	IN void *the_data: ssdp_thread_data structure. This structure contains
*			SSDP request message.
*
* Description:														
*	This function parses the message and dispatches it to a handler 
*	which handles the ssdp request msg
*
* Returns: int
*	0 if successful -1 if error
***************************************************************************/
static XINLINE int
start_event_handler( void *Data )
{

    http_parser_t *parser = NULL;
    parse_status_t status;
    ssdp_thread_data *data = ( ssdp_thread_data * ) Data;

	//mpDebugPrint("start_event_handler"); 
    parser = &data->parser;
	//mpDebugPrint("start_event_handler 1"); 
    status = parser_parse( parser );
	//mpDebugPrint("start_event_handler 2"); 
    if( status == PARSE_FAILURE ) {
        if( parser->msg.method != HTTPMETHOD_NOTIFY ||
            !parser->valid_ssdp_notify_hack ) {
            DBGONLY( UpnpPrintf( UPNP_INFO, SSDP, __FILE__, __LINE__,
                                 "SSDP recvd bad msg code = %d\n",
                                 status );
                 )
                // ignore bad msg, or not enuf mem
                goto error_handler;
        }
        // valid notify msg
    } else if( status != PARSE_SUCCESS ) {
        DBGONLY( UpnpPrintf( UPNP_INFO, SSDP, __FILE__, __LINE__,
                             "SSDP recvd bad msg code = %d\n", status );
             )

            goto error_handler;
    }
	//mpDebugPrint("start_event_handler 3"); 
    // check msg
    if( !valid_ssdp_msg( &parser->msg ) ) {
        goto error_handler;
    }
    return 0;                   //////// done; thread will free 'data'

  error_handler:
    free_ssdp_event_handler_data( data );
    return -1;
}

/************************************************************************
* Function : ssdp_event_handler_thread								
*																	
* Parameters:														
*	IN void *the_data: ssdp_thread_data structure. This structure contains
*			SSDP request message.
*
* Description:														
*	This function is a thread that handles SSDP requests.
*
* Returns: void
*	
***************************************************************************/
static void
ssdp_event_handler_thread( void *the_data )
{
    ssdp_thread_data *data = ( ssdp_thread_data * ) the_data;
    http_message_t *hmsg = &data->parser.msg;

    if( start_event_handler( the_data ) != 0 ) {
		mpDebugPrint("start_event_handler( the_data ) != 0 "); 
        return;
    }
    // send msg to device or ctrlpt
    if( ( hmsg->method == HTTPMETHOD_NOTIFY ) ||
        ( hmsg->request_method == HTTPMETHOD_MSEARCH ) ) {

		//mpDebugPrint("HTTPMETHOD_NOTIFY || HTTPMETHOD_MSEARCH");
        CLIENTONLY( ssdp_handle_ctrlpt_msg( hmsg, &data->dest_addr,
                                            FALSE, NULL );
             );
    } else {
		//mpDebugPrint("!!!(HTTPMETHOD_NOTIFY || HTTPMETHOD_MSEARCH)");
        DEVICEONLY( ssdp_handle_device_request( hmsg, &data->dest_addr ););
    }
    // free data
    free_ssdp_event_handler_data( data );
}

/************************************************************************
* Function : readFromSSDPSocket								
*																	
* Parameters:														
*	IN SOCKET socket: SSDP socket
*
* Description:														
*	This function reads the data from the ssdp socket.
*
* Returns: void
*	
***************************************************************************/
unsigned int
readFromSSDPSocket( SOCKET socket )
{
    char *requestBuf = NULL;
    char staticBuf[BUFSIZE];
    struct sockaddr_in clientAddr;
    ThreadPoolJob job;
    ssdp_thread_data *data = NULL;
    socklen_t socklen = 0;
    int byteReceived = 0;
	//
    int ftpStatus = NO_ERR;

	struct timeval tv;
    ST_SOCK_SET stReadSet, stWriteSet;
    ST_SOCK_SET *wfds, *rfds;

	//

    requestBuf = staticBuf;

    //in case memory
    //can't be allocated, still drain the 
    //socket using a static buffer
	//KK mpDebugPrint("readFromSSDPSocket %d\n",sizeof( ssdp_thread_data )); 
    socklen = sizeof( struct sockaddr_in );

    data = ( ssdp_thread_data * )
        ixml_mem_malloc( sizeof( ssdp_thread_data ) );

    if( data != NULL ) {
        //initialize parser
		//mpDebugPrint("readFromSSDPSocket 1\n"); 
//#ifdef INCLUDE_CLIENT_APIS
        if( socket == gSsdpReqSocket ) {
			//mpDebugPrint("##################socket == gSsdpReqSocket\n"); 
            parser_response_init( &data->parser, HTTPMETHOD_MSEARCH );
        } else 
		{
			//mpDebugPrint("socket == gSsdpReqSocket else\n"); 
            parser_request_init( &data->parser );
        }
//#else
        //parser_request_init( &data->parser );
//#endif
		//mpDebugPrint("readFromSSDPSocket 2\n"); 
        //set size of parser buffer
		//mpDebugPrint("membuffer_set_size");
        if( membuffer_set_size( &data->parser.msg.msg, BUFSIZE ) == 0 ) {
            //use this as the buffer for recv
            requestBuf = data->parser.msg.msg.buf;

        } else {
			mpDebugPrint("ixml_mem_free KKK");
            ixml_mem_free( data );
            data = NULL;
        }
    }
	//Kevin ADD for Noblocking
	//End Kevin
	//mpDebugPrint("readFromSSDPSocket 3========>\n"); 
	TaskYield();
    MPX_FD_ZERO(&stReadSet);
    MPX_FD_ZERO(&stWriteSet);
    wfds = rfds = NULL;
	rfds = &stReadSet;
	MPX_FD_SET(socket, &stReadSet);

	tv.tv_sec = 2;
	tv.tv_usec = 0;

    ftpStatus = select(0, rfds, NULL, 0, &tv);

	if( ftpStatus <= 0 )
	{
		TaskYield();
		//mpDebugPrint("UPNP::%s ==>NO SSDP Server Message",__FUNCTION__);
		free_ssdp_event_handler_data( data );
		return 0;
	}

    byteReceived = recvfrom( socket, requestBuf,
                             BUFSIZE - 1, 0,
                             ( struct sockaddr * )&clientAddr, &socklen );
	//mpDebugPrint("readFromSSDPSocket 5 byteReceived %x\n",byteReceived); 
    if( byteReceived > 0 ) {

		//mpDebugPrint("===============>KSSDPReply byteReceived %x",byteReceived); 
        requestBuf[byteReceived] = '\0';
		//mpDebugPrint("Received response !!!  %s From host %x \n",
          //                   requestBuf,
            //                 clientAddr.sin_addr); 
		//closesock(socket);
		//mpx_SocketClose(socket);

            //add thread pool job to handle request
            if( data != NULL ) 
			{
				data->parser.msg.msg.length += byteReceived;
				// null-terminate
				data->parser.msg.msg.buf[byteReceived] = 0;
				data->dest_addr = clientAddr;
				ssdp_event_handler_thread(data);
#if 0
				TPJobInit( &job, ( start_routine )
						   ssdp_event_handler_thread, data );
				//Be note it
				TPJobSetFreeFunction( &job, free_ssdp_event_handler_data );
				TPJobSetPriority( &job, MED_PRIORITY );

				if( ThreadPoolAdd( &gRecvThreadPool, &job, NULL ) != 0 ) {
					free_ssdp_event_handler_data( data );

				}
#endif
			}
			return 1;

    } else {
		mpDebugPrint("byteReceived <= 0");
        free_ssdp_event_handler_data( data );
		//ixml_mem_free(data);
		return 0;
    }

}

unsigned int
readFromSSDPSocket_M( SOCKET socket )
{
    char *requestBuf = NULL;
    char staticBuf[BUFSIZE];
    struct sockaddr_in clientAddr;
    ThreadPoolJob job;
    ssdp_thread_data *data = NULL;
    socklen_t socklen = 0;
    int byteReceived = 0;
	//
    int ftpStatus = NO_ERR;

	struct timeval tv;
    ST_SOCK_SET stReadSet, stWriteSet;
    ST_SOCK_SET *wfds, *rfds;

	//

    requestBuf = staticBuf;
    socklen = sizeof( struct sockaddr_in );

    data = ( ssdp_thread_data * )
        ixml_mem_malloc( sizeof( ssdp_thread_data ) );

    if( data != NULL ) {
        //initialize parser
		{
            parser_request_init( &data->parser );
        }
        //set size of parser buffer
		//mpDebugPrint("ZZZ1");
        if( membuffer_set_size( &data->parser.msg.msg, BUFSIZE ) == 0 ) {
            //use this as the buffer for recv
            requestBuf = data->parser.msg.msg.buf;

        } else {
			//mpDebugPrint("ZZZ2");
            ixml_mem_free( data );
            data = NULL;
        }
    }
    byteReceived = recvfrom( socket, requestBuf,
                             BUFSIZE - 1, 0,
                             ( struct sockaddr * )&clientAddr, &socklen );
    if( byteReceived > 0 ) {

		TaskYield();
        requestBuf[byteReceived] = '\0';

            //add thread pool job to handle request
            if( data != NULL ) 
			{
				data->parser.msg.msg.length += byteReceived;
				// null-terminate
				data->parser.msg.msg.buf[byteReceived] = 0;
				data->dest_addr = clientAddr;
				TaskYield();
				ssdp_event_handler_thread(data);
			}
			return 1;

    } else {
		mpDebugPrint("byteReceived <= 0");
        free_ssdp_event_handler_data( data );
		//ixml_mem_free(data);
		return 0;
    }

}

/************************************************************************
* Function : get_ssdp_sockets								
*																	
* Parameters:														
*	OUT MiniServerSockArray *out: Arrays of SSDP sockets
*
* Description:														
*	This function creates the ssdp sockets. It set their option to listen 
*	for multicast traffic.
*
* Returns: int
*	return UPNP_E_SUCCESS if successful else returns appropriate error
***************************************************************************/
int
get_ssdp_sockets( MiniServerSockArray * out )
{
    SOCKET ssdpSock;

    //CLIENTONLY( SOCKET ssdpReqSock;
      //   )
    int onOff = 1;
    u_char ttl = 4;
    struct ip_mreq ssdpMcastAddr;
    struct sockaddr_in ssdpAddr;
    int option = 1;
	BYTE macaddress[6];
	void *iwreq;
#if 0
	mpDebugPrint("get_ssdp_sockets 1");
    CLIENTONLY( if( ( ssdpReqSock = socket( AF_INET, SOCK_DGRAM, 0 ) )
                    == UPNP_INVALID_SOCKET ) {
				mpDebugPrint("get_ssdp_sockets 2");
                DBGONLY( UpnpPrintf( UPNP_CRITICAL,
                                     SSDP, __FILE__, __LINE__,
                                     "Error in socket operation !!!\n" ); )
                return UPNP_E_OUTOF_SOCKET;}
                setsockopt( ssdpReqSock,
                            IPPROTO_IP,
                            IP_MULTICAST_TTL, &ttl, sizeof( ttl ) );
                // just do it, regardless if fails or not.
                Make_Socket_NoBlocking( ssdpReqSock ); gSsdpReqSocket = ssdpReqSock; )  //CLIENTONLY
#endif

        if( ( ssdpSock = socket( AF_INET, SOCK_DGRAM, 0 ) )
            == UPNP_INVALID_SOCKET ) {
			mpDebugPrint("get_ssdp_sockets 3");
            DBGONLY( UpnpPrintf( UPNP_CRITICAL,
                                 SSDP, __FILE__, __LINE__,
                                 "Error in socket operation !!!\n" );
                 )
                //K CLIENTONLY( shutdown( ssdpReqSock, SD_BOTH ) );
            //CLIENTONLY( UpnpCloseSocket( ssdpReqSock ) );
            return UPNP_E_OUTOF_SOCKET;
        }

    onOff = 1;
	//mpDebugPrint("get_ssdp_sockets 5");
/*
    if( setsockopt( ssdpSock, SOL_SOCKET, SO_REUSEADDR,
                    ( char * )&onOff, sizeof( onOff ) ) != 0 ) {

        DBGONLY( UpnpPrintf( UPNP_CRITICAL,
                             SSDP, __FILE__, __LINE__,
                             "Error in set reuse addr !!!\n" );
             )
            //K CLIENTONLY( shutdown( ssdpReqSock, SD_BOTH ) );
        CLIENTONLY( UpnpCloseSocket( ssdpReqSock ) );
        //K shutdown( ssdpSock, SD_BOTH );
        UpnpCloseSocket( ssdpSock );
        return UPNP_E_SOCKET_ERROR;
    }
*/    
    #ifdef __FreeBSD__
    if( setsockopt( ssdpSock, SOL_SOCKET, SO_REUSEPORT,
                    ( char * )&onOff, sizeof( onOff ) ) != 0 ) {

        DBGONLY( UpnpPrintf( UPNP_CRITICAL,
                             SSDP, __FILE__, __LINE__,
                             "Error in set reuse port !!!\n" );
             )
            //K CLIENTONLY( shutdown( ssdpReqSock, SD_BOTH ) );
        CLIENTONLY( UpnpCloseSocket( ssdpReqSock ) );
        //K shutdown( ssdpSock, SD_BOTH );
        UpnpCloseSocket( ssdpSock );
        return UPNP_E_SOCKET_ERROR;
    }
    #endif

    memset( ( void * )&ssdpAddr, 0, sizeof( struct sockaddr_in ) );
	//mpDebugPrint("get_ssdp_sockets 5-1");
    ssdpAddr.sin_family = AF_INET;
    //  ssdpAddr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    ssdpAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    ssdpAddr.sin_port = htons( SSDP_PORT );
	//mpDebugPrint("get_ssdp_sockets 5-2");
    if( bind
        ( ssdpSock, ( struct sockaddr * )&ssdpAddr,
          sizeof( ssdpAddr ) ) != 0 ) {
        DBGONLY( UpnpPrintf
                 ( UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in binding !!!\n" );
             )
            //K shutdown( ssdpSock, SD_BOTH );
        UpnpCloseSocket( ssdpSock );
        //K CLIENTONLY( shutdown( ssdpReqSock, SD_BOTH ) );
        //CLIENTONLY( UpnpCloseSocket( ssdpReqSock ) );
        return UPNP_E_SOCKET_BIND;
    }
	mpDebugPrint("get_ssdp_sockets 5-3");
    memset( ( void * )&ssdpMcastAddr, 0, sizeof( struct ip_mreq ) );
    ssdpMcastAddr.imr_interface.s_addr = htonl( INADDR_ANY );
    ssdpMcastAddr.imr_multiaddr.s_addr = inet_addr( SSDP_IP );
	mpDebugPrint("get_ssdp_sockets 5-5");
	//ret = ioctl(socket_id, RT_PRIV_IOCTL, &wrq);

	macaddress[0] =0x01;
	macaddress[1] =0x00;
	macaddress[2] =0x5E;
	macaddress[3] =0x7F;
	macaddress[4] =0xFF;
	macaddress[5] =0xFA;
	//iwreq = (void *) ixml_mem_malloc(1024);
	//setiwreq_multiparam(iwreq,&macaddress);
	ioctl(ssdpSock, SIOCADDMULTI, &macaddress);  
	//ixml_mem_free(iwreq);
/*
    if( setsockopt( ssdpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                    ( char * )&ssdpMcastAddr,
                    sizeof( struct ip_mreq ) ) != 0 ) {
        DBGONLY( UpnpPrintf
                 ( UPNP_CRITICAL, SSDP, __FILE__, __LINE__,
                   "Error in joining" " multicast group !!!\n" );
             )
            //K shutdown( ssdpSock, SD_BOTH );
        //K CLIENTONLY( shutdown( ssdpReqSock, SD_BOTH ) );
        UpnpCloseSocket( ssdpSock );
        CLIENTONLY( UpnpCloseSocket( ssdpReqSock ) );
        return UPNP_E_SOCKET_ERROR;
    }
*/
	//mpDebugPrint("get_ssdp_sockets 6");
    // result is not checked becuase it will fail in WinMe and Win9x.
    //setsockopt( ssdpSock, IPPROTO_IP,
    //            IP_MULTICAST_TTL, &ttl, sizeof( ttl ) );
    if( setsockopt( ssdpSock, SOL_SOCKET, SO_BROADCAST,
                    ( char * )&option, sizeof( option ) ) != 0 ) 
	{
		mpDebugPrint("Error SO_BROADCAST");
        DBGONLY( UpnpPrintf( UPNP_CRITICAL,
                             SSDP, __FILE__, __LINE__,
                             "Error in setting broadcast !!!\n" );
             )
            //K shutdown( ssdpSock, SD_BOTH );
        //K CLIENTONLY( shutdown( ssdpReqSock, SD_BOTH ) );
        UpnpCloseSocket( ssdpSock );
        //CLIENTONLY( UpnpCloseSocket( ssdpReqSock ) );
        return UPNP_E_NETWORK_ERROR;
    }
	//mpDebugPrint("get_ssdp_sockets 7");
    //CLIENTONLY( out->ssdpReqSock = ssdpReqSock;
      //   );
    out->ssdpSock = ssdpSock;
    return UPNP_E_SUCCESS;
}
#endif // EXCLUDE_SSDP
