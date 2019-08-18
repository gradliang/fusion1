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

#include "sample_util.h"
#include <stdarg.h>

//#include "netfs_pri.h"
//#include "netfs.h"
//#include "upnpfs.h"

#if !UPNP_HAVE_TOOLS
#	error "Need upnptools.h to compile samples ; try ./configure --enable-tools"
#endif

//K static struct netfs_file_entry jpeg_info;
/*
   Function pointer to use for displaying formatted
   strings. Set on Initialization of device. 
 */
int initialize = 1;
print_string gPrintFun = NULL;
state_update gStateUpdateFun = NULL;

//mutex to control displaying of events
//K ithread_mutex_t display_mutex;

/********************************************************************************
 * SampleUtil_Initialize
 *
 * Description: 
 *     Initializes the sample util. Must be called before any sample util 
 *     functions. May be called multiple times.
 *
 * Parameters:
 *   print_function - print function to use in SampleUtil_Print
 *
 ********************************************************************************/
int
SampleUtil_Initialize( print_string print_function )
{
    if( initialize ) {
        //K ithread_mutexattr_t attr;

        gPrintFun = print_function;
        //K ithread_mutexattr_init( &attr );
        //K ithread_mutexattr_setkind_np( &attr, ITHREAD_MUTEX_RECURSIVE_NP );
        //K ithread_mutex_init( &display_mutex, &attr );
        //K ithread_mutexattr_destroy( &attr );
        initialize = 0;
    }
    return UPNP_E_SUCCESS;
}

/********************************************************************************
 * SampleUtil_RegisterUpdateFunction
 *
 * Description: 
 *
 * Parameters:
 *
 ********************************************************************************/
int
SampleUtil_RegisterUpdateFunction( state_update update_function )
{
    static int initialize = 1;  //only intialize once

    if( initialize ) {
        gStateUpdateFun = update_function;
        initialize = 0;
    }
    return UPNP_E_SUCCESS;
}

/********************************************************************************
 * SampleUtil_Finish
 *
 * Description: 
 *     Releases Resources held by sample util.
 *
 * Parameters:
 *
 ********************************************************************************/
int
SampleUtil_Finish(  )
{
    //K ithread_mutex_destroy( &display_mutex );
    gPrintFun = NULL;
    initialize = 1;
    return UPNP_E_SUCCESS;
}

/********************************************************************************
 * SampleUtil_GetElementValue
 *
 * Description: 
 *       Given a DOM node such as <Channel>11</Channel>, this routine
 *       extracts the value (e.g., 11) from the node and returns it as 
 *       a string. The string must be freed by the caller using 
 *       free.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the value
 *
 ********************************************************************************/

char *
SampleUtil_GetElementValue( IN IXML_Element * element )
{

    IXML_Node *child = ixmlNode_getFirstChild( ( IXML_Node * ) element );

    char *temp = NULL;

    if( ( child != 0 ) && ( ixmlNode_getNodeType( child ) == eTEXT_NODE ) ) {
        temp = strdup( ixmlNode_getNodeValue( child ) );
    }

    return temp;
}

/********************************************************************************
 * SampleUtil_GetFirstServiceList
 *
 * Description: 
 *       Given a DOM node representing a UPnP Device Description Document,
 *       this routine parses the document and finds the first service list
 *       (i.e., the service list for the root device).  The service list
 *       is returned as a DOM node list.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the service list
 *
 ********************************************************************************/
IXML_NodeList *
SampleUtil_GetFirstServiceList( IN IXML_Document * doc )
{
    IXML_NodeList *ServiceList = NULL;
    IXML_NodeList *servlistnodelist = NULL;
    IXML_Node *servlistnode = NULL;

	//mpDebugPrint("SampleUtil_GetFirstServiceList 1\n");
    servlistnodelist =
        ixmlDocument_getElementsByTagName( doc, "serviceList" );
    if( servlistnodelist && ixmlNodeList_length( servlistnodelist ) ) {

		//mpDebugPrint("SampleUtil_GetFirstServiceList 2\n");
        /*
           we only care about the first service list, from the root device 
         */
        servlistnode = ixmlNodeList_item( servlistnodelist, 0 );

        /*
           create as list of DOM nodes 
         */
        ServiceList =
            ixmlElement_getElementsByTagName( ( IXML_Element * )
                                              servlistnode, "service" );
    }

    if( servlistnodelist )
        ixmlNodeList_free( servlistnodelist );

	//mpDebugPrint("SampleUtil_GetFirstServiceList 3\n");

    return ServiceList;
}

/********************************************************************************
 * SampleUtil_GetFirstDocumentItem
 *
 * Description: 
 *       Given a document node, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *       String must be freed by caller using free.
 * Parameters:
 *   doc -- The DOM document from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char *
SampleUtil_GetFirstDocumentItem( IN IXML_Document * doc,
                                 IN const char *item )
{
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    char *ret = NULL;
    nodeList = ixmlDocument_getElementsByTagName( doc, ( char * )item );
    if( nodeList ) {
        if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) ) {
            textNode = ixmlNode_getFirstChild( tmpNode );

            ret = ixml_strdup( ixmlNode_getNodeValue( textNode ) );
        }
    }
    if( nodeList )
        ixmlNodeList_free( nodeList );
    return ret;
}

/********************************************************************************
 * SampleUtil_GetFirstElementItem
 *
 * Description: 
 *       Given a DOM element, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *       The string must be freed using free.
 * Parameters:
 *   node -- The DOM element from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char *
SampleUtil_GetFirstElementItem( IN IXML_Element * element,
                                IN const char *item )
{
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    char *ret = NULL;
	char *kevintmp;

    nodeList = ixmlElement_getElementsByTagName( element, ( char * )item );

    if( nodeList == NULL ) {
        SampleUtil_Print( "Error finding %s in XML Node\n", item );
        return NULL;
    }

    if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) == NULL ) {
        SampleUtil_Print( "Error finding %s value in XML Node\n", item );
        ixmlNodeList_free( nodeList );
        return NULL;
    }
    textNode = ixmlNode_getFirstChild( tmpNode );
    ret = ixml_strdup( ixmlNode_getNodeValue( textNode ) );
    if( !ret ) {
        SampleUtil_Print( "Error allocating memory for %s in XML Node\n",
                          item );
        ixmlNodeList_free( nodeList );
		mpDebugPrint("return NULL\n");
        return NULL;
    }
    ixmlNodeList_free( nodeList );
    return ret;
}

/********************************************************************************
 * SampleUtil_PrintEventType
 *
 * Description: 
 *       Prints a callback event type as a string.
 *
 * Parameters:
 *   S -- The callback event
 *
 ********************************************************************************/
void
SampleUtil_PrintEventType( IN Upnp_EventType S )
{
    switch ( S ) {

        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
            SampleUtil_Print( "UPNP_DISCOVERY_ADVERTISEMENT_ALIVE\n" );
            break;
        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
            SampleUtil_Print( "UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE\n" );
            break;
        case UPNP_DISCOVERY_SEARCH_RESULT:
            //SampleUtil_Print( "UPNP_DISCOVERY_SEARCH_RESULT\n" );
            break;
        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
            SampleUtil_Print( "UPNP_DISCOVERY_SEARCH_TIMEOUT\n" );
            break;

            /*
               SOAP Stuff 
             */
        case UPNP_CONTROL_ACTION_REQUEST:
            SampleUtil_Print( "UPNP_CONTROL_ACTION_REQUEST\n" );
            break;
        case UPNP_CONTROL_ACTION_COMPLETE:
            SampleUtil_Print( "UPNP_CONTROL_ACTION_COMPLETE\n" );
            break;
        case UPNP_CONTROL_GET_VAR_REQUEST:
            SampleUtil_Print( "UPNP_CONTROL_GET_VAR_REQUEST\n" );
            break;
        case UPNP_CONTROL_GET_VAR_COMPLETE:
            SampleUtil_Print( "UPNP_CONTROL_GET_VAR_COMPLETE\n" );
            break;

            /*
               GENA Stuff 
             */
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
            SampleUtil_Print( "UPNP_EVENT_SUBSCRIPTION_REQUEST\n" );
            break;
        case UPNP_EVENT_RECEIVED:
            SampleUtil_Print( "UPNP_EVENT_RECEIVED\n" );
            break;
        case UPNP_EVENT_RENEWAL_COMPLETE:
            SampleUtil_Print( "UPNP_EVENT_RENEWAL_COMPLETE\n" );
            break;
        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
            SampleUtil_Print( "UPNP_EVENT_SUBSCRIBE_COMPLETE\n" );
            break;
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
            SampleUtil_Print( "UPNP_EVENT_UNSUBSCRIBE_COMPLETE\n" );
            break;

        case UPNP_EVENT_AUTORENEWAL_FAILED:
            SampleUtil_Print( "UPNP_EVENT_AUTORENEWAL_FAILED\n" );
            break;
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
            SampleUtil_Print( "UPNP_EVENT_SUBSCRIPTION_EXPIRED\n" );
            break;

    }
}

void KevinRecusivePrintf( IXML_Node *knode )
{
	//if(  KevinRecusivePrintf )
	IXML_Node	*ktmp;
	IXML_Node	*ntmp;
	ktmp = knode->firstChild;
	if( ktmp )
	{
		mpDebugPrint("nodetype %d nodeName %s nodeValue%s\n",ktmp->nodeType,ktmp->nodeName,ktmp->nodeValue );
		KevinRecusivePrintf( ktmp );
	}
	ntmp = ixmlNode_getNextSibling(knode);
	if( ntmp )
	{
		mpDebugPrint("next sib\n");
		mpDebugPrint("sib nodetype %d nodeName %s nodeValue%s\n",ntmp->nodeType,ntmp->nodeName,ntmp->nodeValue );
		KevinRecusivePrintf( ntmp );
	}
	return;
}

int GetTotalMatches( IXML_Node *knode )
{
	IXML_Node	*ktmp;
	IXML_Node	*ntmp;
	ktmp = knode->firstChild;
	if( ktmp )
	{
		KevinRecusivePrintf( ktmp );
		mpDebugPrint("nodetype %d nodeName %s nodeValue%s\n",ktmp->nodeType,ktmp->nodeName,ktmp->nodeValue );

	}
	ntmp = ixmlNode_getNextSibling(knode);
	return;
	
}
/********************************************************************************
 * SampleUtil_PrintEvent
 *
 * Description: 
 *       Prints callback event structure details.
 *
 * Parameters:
 *   EventType -- The type of callback event
 *   Event -- The callback event structure
 *
 ********************************************************************************/
int
SampleUtil_PrintEvent( IN Upnp_EventType EventType,
                       IN void *Event )
{
	IXML_Node	*ktmp;
	IXML_NodeList *knodelist;
	IXML_NodeList *kknl;
	int totalmt,totalrt,totalchd;
	char *nptr,*kptr,*kptr1,*kptr2;
	int klen;
	char kcontainid[32],ktitle[32],childcount[32],tmpprotocol[128];
	int i;
	struct upnpfs_entry *ptmp_upnpfs,*pcur_upnpfs;
	unsigned char bForder;
    //K upnp_info_t   *upnp_info = &(upnp_map.info);

    //K ithread_mutex_lock( &display_mutex );

	mpDebugPrint("SampleUtil_PrintEvent"); 
    switch ( EventType ) {

            /*
               SSDP 
             */
        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
        case UPNP_DISCOVERY_SEARCH_RESULT:
            {
/*
                struct Upnp_Discovery *d_event =
                    ( struct Upnp_Discovery * )Event;

                SampleUtil_Print( "ErrCode     =  %d\n",
                                  d_event->ErrCode );
                SampleUtil_Print( "Expires     =  %d\n",
                                  d_event->Expires );
                SampleUtil_Print( "DeviceId    =  %s\n",
                                  d_event->DeviceId );
                SampleUtil_Print( "DeviceType  =  %s\n",
                                  d_event->DeviceType );
                SampleUtil_Print( "ServiceType =  %s\n",
                                  d_event->ServiceType );
                SampleUtil_Print( "ServiceVer  =  %s\n",
                                  d_event->ServiceVer );
                SampleUtil_Print( "Location    =  %s\n",
                                  d_event->Location );
                SampleUtil_Print( "OS          =  %s\n", d_event->Os );
                SampleUtil_Print( "Ext         =  %s\n", d_event->Ext );
*/
            }
            break;

        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
            // Nothing to print out here
            break;

            /*
               SOAP 
             */
        case UPNP_CONTROL_ACTION_REQUEST:
            {
                struct Upnp_Action_Request *a_event =
                    ( struct Upnp_Action_Request * )Event;
                char *xmlbuff = NULL;

                SampleUtil_Print( "ErrCode     =  %d\n",
                                  a_event->ErrCode );
                SampleUtil_Print( "ErrStr      =  %s\n", a_event->ErrStr );
                SampleUtil_Print( "ActionName  =  %s\n",
                                  a_event->ActionName );
                SampleUtil_Print( "UDN         =  %s\n", a_event->DevUDN );
                SampleUtil_Print( "ServiceID   =  %s\n",
                                  a_event->ServiceID );
                if( a_event->ActionRequest ) {
                    xmlbuff = ixmlPrintNode( ( IXML_Node * ) a_event->ActionRequest );
                    if( xmlbuff )
                        SampleUtil_Print( "ActRequest  =  %s\n", xmlbuff );
                    if( xmlbuff )
                        ixmlFreeDOMString( xmlbuff );
                    xmlbuff = NULL;
                } else {
                    SampleUtil_Print( "ActRequest  =  (null)\n" );
                }

                if( a_event->ActionResult ) {
                    xmlbuff = ixmlPrintNode( ( IXML_Node * ) a_event->ActionResult );
                    if( xmlbuff )
                        SampleUtil_Print( "ActResult   =  %s\n", xmlbuff );
                    if( xmlbuff )
                        ixmlFreeDOMString( xmlbuff );
                    xmlbuff = NULL;
                } else {
                    SampleUtil_Print( "ActResult   =  (null)\n" );
                }
            }
            break;

        case UPNP_CONTROL_ACTION_COMPLETE:
            {

            }
            break;

        case UPNP_CONTROL_GET_VAR_REQUEST:
            {
                struct Upnp_State_Var_Request *sv_event =
                    ( struct Upnp_State_Var_Request * )Event;

                SampleUtil_Print( "ErrCode     =  %d\n",
                                  sv_event->ErrCode );
                SampleUtil_Print( "ErrStr      =  %s\n",
                                  sv_event->ErrStr );
                SampleUtil_Print( "UDN         =  %s\n",
                                  sv_event->DevUDN );
                SampleUtil_Print( "ServiceID   =  %s\n",
                                  sv_event->ServiceID );
                SampleUtil_Print( "StateVarName=  %s\n",
                                  sv_event->StateVarName );
                SampleUtil_Print( "CurrentVal  =  %s\n",
                                  sv_event->CurrentVal );
            }
            break;

        case UPNP_CONTROL_GET_VAR_COMPLETE:
            {
                struct Upnp_State_Var_Complete *sv_event =
                    ( struct Upnp_State_Var_Complete * )Event;

                SampleUtil_Print( "ErrCode     =  %d\n",
                                  sv_event->ErrCode );
                SampleUtil_Print( "CtrlUrl     =  %s\n",
                                  sv_event->CtrlUrl );
                SampleUtil_Print( "StateVarName=  %s\n",
                                  sv_event->StateVarName );
                SampleUtil_Print( "CurrentVal  =  %s\n",
                                  sv_event->CurrentVal );
            }
            break;

            /*
               GENA 
             */
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
            {
                struct Upnp_Subscription_Request *sr_event =
                    ( struct Upnp_Subscription_Request * )Event;

                SampleUtil_Print( "ServiceID   =  %s\n",
                                  sr_event->ServiceId );
                SampleUtil_Print( "UDN         =  %s\n", sr_event->UDN );
                SampleUtil_Print( "SID         =  %s\n", sr_event->Sid );
            }
            break;

        case UPNP_EVENT_RECEIVED:
            {
                struct Upnp_Event *e_event = ( struct Upnp_Event * )Event;
                char *xmlbuff = NULL;

                SampleUtil_Print( "SID         =  %s\n", e_event->Sid );
                SampleUtil_Print( "EventKey    =  %d\n",
                                  e_event->EventKey );
                xmlbuff = ixmlPrintNode( ( IXML_Node * ) e_event->ChangedVariables );
                SampleUtil_Print( "ChangedVars =  %s\n", xmlbuff );
                ixmlFreeDOMString( xmlbuff );
                xmlbuff = NULL;
            }
            break;

        case UPNP_EVENT_RENEWAL_COMPLETE:
            {
                struct Upnp_Event_Subscribe *es_event =
                    ( struct Upnp_Event_Subscribe * )Event;

                SampleUtil_Print( "SID         =  %s\n", es_event->Sid );
                SampleUtil_Print( "ErrCode     =  %d\n",
                                  es_event->ErrCode );
                SampleUtil_Print( "TimeOut     =  %d\n",
                                  es_event->TimeOut );
            }
            break;

        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
            {
                struct Upnp_Event_Subscribe *es_event =
                    ( struct Upnp_Event_Subscribe * )Event;

                SampleUtil_Print( "SID         =  %s\n", es_event->Sid );
                SampleUtil_Print( "ErrCode     =  %d\n",
                                  es_event->ErrCode );
                SampleUtil_Print( "PublisherURL=  %s\n",
                                  es_event->PublisherUrl );
                SampleUtil_Print( "TimeOut     =  %d\n",
                                  es_event->TimeOut );
            }
            break;

        case UPNP_EVENT_AUTORENEWAL_FAILED:
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
            {
                struct Upnp_Event_Subscribe *es_event =
                    ( struct Upnp_Event_Subscribe * )Event;

                SampleUtil_Print( "SID         =  %s\n", es_event->Sid );
                SampleUtil_Print( "ErrCode     =  %d\n",
                                  es_event->ErrCode );
                SampleUtil_Print( "PublisherURL=  %s\n",
                                  es_event->PublisherUrl );
                SampleUtil_Print( "TimeOut     =  %d\n",
                                  es_event->TimeOut );
            }
            break;

    }
    //K ithread_mutex_unlock( &display_mutex );
    return 1;
}

/********************************************************************************
 * SampleUtil_FindAndParseService
 *
 * Description: 
 *       This routine finds the first occurance of a service in a DOM representation
 *       of a description document and parses it.  
 *
 * Parameters:
 *   DescDoc -- The DOM description document
 *   location -- The location of the description document
 *   serviceSearchType -- The type of service to search for
 *   serviceId -- OUT -- The service ID
 *   eventURL -- OUT -- The event URL for the service
 *   controlURL -- OUT -- The control URL for the service
 *
 ********************************************************************************/
int
SampleUtil_FindAndParseService( IN IXML_Document * DescDoc,
                                IN char *location,
                                IN char *serviceType,
                                OUT char **serviceId,
                                OUT char **eventURL,
                                OUT char **controlURL )
{
    int i,
      length,
      found = 0;
    int ret;
    char *tempServiceType = NULL;
    char *baseURL = NULL;
    char *base;
    char *relcontrolURL = NULL,
     *releventURL = NULL;
    IXML_NodeList *serviceList = NULL;
    IXML_Element *service = NULL;

    baseURL = SampleUtil_GetFirstDocumentItem( DescDoc, "URLBase" );

    if( baseURL )
        base = baseURL;
    else
        base = location;

    serviceList = SampleUtil_GetFirstServiceList( DescDoc );
    length = ixmlNodeList_length( serviceList );
    for( i = 0; i < length; i++ ) {
        service = ( IXML_Element * ) ixmlNodeList_item( serviceList, i );
		//mpDebugPrint("service %x %s\n",service,service->tagName);

        tempServiceType =
            SampleUtil_GetFirstElementItem( ( IXML_Element * ) service,
                                            "serviceType" );
        if( strcmp( tempServiceType, serviceType ) == 0 ) {
			//mpDebugPrint("Found service: %s\n", serviceType);
            SampleUtil_Print( "Found service: %s\n", serviceType );
            *serviceId = SampleUtil_GetFirstElementItem( service, "serviceId" );
            SampleUtil_Print( "serviceId: %s\n", ( *serviceId ) );
            relcontrolURL =
                SampleUtil_GetFirstElementItem( service, "controlURL" );
            releventURL =
                SampleUtil_GetFirstElementItem( service, "eventSubURL" );

            *controlURL =
                ixml_mem_malloc( strlen( base ) + strlen( relcontrolURL ) + 1 );
            if( *controlURL ) {
                ret = UpnpResolveURL( base, relcontrolURL, *controlURL );
                if( ret != UPNP_E_SUCCESS )
                    SampleUtil_Print
                        ( "Error generating controlURL from %s + %s\n",
                          base, relcontrolURL );
            }
            *eventURL =
                ixml_mem_malloc( strlen( base ) + strlen( releventURL ) + 1 );
            if( *eventURL ) {
                ret = UpnpResolveURL( base, releventURL, *eventURL );
                if( ret != UPNP_E_SUCCESS )
                    SampleUtil_Print
                        ( "Error generating eventURL from %s + %s\n", base,
                          releventURL );
            }

            if( relcontrolURL )
                ixml_mem_free( relcontrolURL );
            if( releventURL )
                ixml_mem_free( releventURL );
            relcontrolURL = releventURL = NULL;
            found = 1;
            break;
        }
		//mpDebugPrint("Kevin XX\n");
        if( tempServiceType )
            ixml_mem_free( tempServiceType );
        tempServiceType = NULL;
    }
    if( tempServiceType )
        ixml_mem_free( tempServiceType );
    if( serviceList )
        ixmlNodeList_free( serviceList );
    if( baseURL )
        ixml_mem_free( baseURL );
	mpDebugPrint("return ( found ) %x",found);
    return ( found );
}

SampleUtil_FindAndParseServiceNat( IN IXML_Document * DescDoc,
                                IN char *location,
                                IN char *serviceType,
                                OUT char **serviceId,
                                OUT char **eventURL,
                                OUT char **controlURL )
{
	int i,	length,	found = 0;
	int ret;
	char *tempServiceType = NULL;
	char *baseURL = NULL;
	char *base;
	char *relcontrolURL = NULL,
	*releventURL = NULL;
	IXML_NodeList *serviceList = NULL,*deviceList = NULL;
	IXML_Element *service = NULL;
	unsigned long ki=0;
	IXML_Node	*ntmp,*tnode;
	IXML_Node *childNode=NULL,*childchildNode=NULL;
	baseURL = SampleUtil_GetFirstDocumentItem( DescDoc, "URLBase" );

	if( baseURL )
		base = baseURL;
	else
		base = location;

	mpDebugPrint("base %s",base);
	deviceList = ixmlDocument_getElementsByTagName( DescDoc, "device" );

	if( deviceList )
	{
	
		do
		{
			tnode = ixmlNodeList_item( deviceList, ki++ );
			if( !tnode )
			{
				mpDebugPrint("Get Nothing ki %d",ki);
				break;
			}
			childNode = ixmlNode_getFirstChild( tnode );
			if( childNode )
			{
				mpDebugPrint("if( childNode )\n");
				childchildNode = ixmlNode_getFirstChild( childNode );
				if( childchildNode )
				{
					mpDebugPrint("childNode %d nodeName %s nodeValue %s\n",childchildNode->nodeType,childchildNode->nodeName,childchildNode->nodeValue );
					//ixml_mem_free(childchildNode);
					//if(strcmp(childchildNode->nodeValue,"schemas-upnp-org:device:WANConnectionDevice:1" )==0 )
					//chemas-upnp-org:device:WANConnectionDevice:1
					if(strcmp(childchildNode->nodeValue,"urn:schemas-upnp-org:device:WANConnectionDevice:1") == 0 )
					{
						mpDebugPrint("Get WANConnectionDevice");
						i =0;
						//ixmlNode_getElementsByTagName(tnode,"serviceType",&serviceList);
						ixmlNode_getElementsByTagName(tnode,"serviceList",&serviceList);
						if(serviceList)
						{
							do
							{
						        service = ( IXML_Element * ) ixmlNodeList_item( serviceList, i ++);
								if(service)	
								{
								        tempServiceType = SampleUtil_GetFirstElementItem( ( IXML_Element * ) service,"serviceType" );
										mpDebugPrint("tempServiceType %s",tempServiceType );
								        if( strcmp( tempServiceType, serviceType ) == 0 ) 
							        	{
										mpDebugPrint("Found service: %s\n", serviceType);
										*serviceId = SampleUtil_GetFirstElementItem( ( IXML_Element * )service, "serviceId" );
										mpDebugPrint("serviceId: %s\n", *serviceId);
										relcontrolURL = SampleUtil_GetFirstElementItem( ( IXML_Element * ) service, "controlURL" );
										mpDebugPrint("relcontrolURL %s",relcontrolURL);
										releventURL = SampleUtil_GetFirstElementItem( ( IXML_Element * )service, "eventSubURL" );
										mpDebugPrint("releventURL %s",releventURL);
										*controlURL = ixml_mem_malloc( strlen( base ) + strlen( relcontrolURL ) + 1 );
										if( *controlURL ) {
										    ret = UpnpResolveURL( base, relcontrolURL, *controlURL );
										    if( ret != UPNP_E_SUCCESS )
										        SampleUtil_Print
										            ( "Error generating controlURL from %s + %s\n",
										              base, relcontrolURL );
										}
										*eventURL = ixml_mem_malloc( strlen( base ) + strlen( releventURL ) + 1 );
										if( *eventURL ) {
											ret = UpnpResolveURL( base, releventURL, *eventURL );
											if( ret != UPNP_E_SUCCESS )
											SampleUtil_Print( "Error generating eventURL from %s + %s\n", base,	releventURL );
										}
										if( relcontrolURL )
									    		ixml_mem_free( relcontrolURL );
										if( releventURL )
									    		ixml_mem_free( releventURL );
										relcontrolURL = releventURL = NULL;
										found = 1;
										break;
								      }
										
						        	}
							}	while(service);
							if( serviceList )
							{
								ixmlNodeList_free(serviceList);												
								serviceList = NULL;
							}
						}							
					}						
				}
			}				
			if( found)
				break;
		}
		while(1);
		ixmlNodeList_free(deviceList);					
	}
	else
		mpDebugPrint("can not get device list");

    if( serviceList )
        ixmlNodeList_free( serviceList );
    if( tempServiceType )
        ixml_mem_free( tempServiceType );
    if( baseURL )
        ixml_mem_free( baseURL );

    return ( found );
}

#if 0
// printf wrapper for portable code
int
uprint1( char *fmt,
         ... )
{
    va_list ap;
    char *buf = NULL;
    int size;

    va_start( ap, fmt );
    size = vsnprintf( buf, 0, fmt, ap );
    va_end( ap );

    if( size > 0 ) {
        buf = ( char * )malloc( size + 1 );
        if( vsnprintf( buf, size + 1, fmt, ap ) != size ) {
            free( buf );
            buf = NULL;
        }
    }

    if( buf ) {
        //K ithread_mutex_lock( &display_mutex );
        if( gPrintFun )
            gPrintFun( buf );
        //K ithread_mutex_unlock( &display_mutex );
        free( buf );
    }

    return size;
}
#endif
/********************************************************************************
 * SampleUtil_Print
 *
 * Description: 
 *      Provides platform-specific print functionality.  This function should be
 *      called when you want to print content suitable for console output (i.e.,
 *      in a large text box or on a screen).  If your device/operating system is 
 *      not supported here, you should add a port.
 *
 * Parameters:
 *		Same as printf()
 *
 ********************************************************************************/
int
SampleUtil_Print( char *fmt,
                  ... )
{
#if 1//UPNP_DBG
    va_list ap;
    char buf[200];
    int rc;

    va_start( ap, fmt );
    rc = vsnprintf( buf, 200, fmt, ap );
    va_end( ap );

    //K ithread_mutex_lock( &display_mutex );
    if( gPrintFun )
        gPrintFun( buf );
    //K ithread_mutex_unlock( &display_mutex );
    return rc;
#else
	return 0;
#endif
}

/********************************************************************************
 * SampleUtil_StateUpdate
 *
 * Description: 
 *
 * Parameters:
 *
 ********************************************************************************/
void
SampleUtil_StateUpdate( const char *varName,
                        const char *varValue,
                        const char *UDN,
                        eventType type )
{
    if( gStateUpdateFun )
        gStateUpdateFun( varName, varValue, UDN, type );
    // TBD: Add mutex here?
}
