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
#define LOCAL_DEBUG_ENABLE 0
#include <pthread.h>
#include "upnp_tv_ctrlpt.h"
#include "upnp.h"

#include "..\..\..\..\libIPLAY\libSrc\LWIP\INCLUDE\net_autosearch.h"
#include "..\..\..\..\libIPLAY\libSrc\XML\INCLUDE\xmlupnp.h"
#include "global612.h"
#include "ui.h"
#include "taskid.h"
#include "..\..\..\..\libIPLAY\libSrc\XML\INCLUDE\netfs.h"
#include "..\..\..\..\libIPLAY\libSrc\XML\INCLUDE\netfs_pri.h"
//#include "..\..\..\..\libIPLAY\demux\include\filetype.h"

//typedef struct _netfs_mount_point   netfs_mount_point_t;
char *upnpcurdir = NULL;
netfs_meta_entry_t  *gnetfs_dir = NULL;
int gdir_child_total_count = 0;
int gdir_child_cur_count = 0;
//char upnpcurdir[2048];//[NETFS_MAX_PATHNAME];
extern unsigned char gupnp_index;
extern unsigned char DeviceSearchType;
unsigned char bAccess_Deny = 0;
//unsigned char gbMicrosoftServer = 0;
enum UpnpServerType gServerType = Others;
unsigned char NatDeviceCout = 0;
unsigned char UPNPNATCOMMAND = 0;
int UpnpReturnCode = 0;
char gExternalIp[32];
unsigned int gExternalPort;
extern char LOCAL_HOST[LINE_SIZE];
extern SERVER_BROWSER ServerBrowser;
extern ST_NET_FILEENTRY *g_FileEntry;
extern netfs_info_t     netfs_info;
extern BYTE g_bXpgStatus;
unsigned char gDeviceAdd = 0;

UpnpClient_Handle ctrlpt_handle = -1;

#if 0
char TvDeviceType[] = "urn:schemas-upnp-org:device:tvdevice:1";
char *TvServiceType[] = {
    "urn:schemas-upnp-org:service:tvcontrol:1",
    "urn:schemas-upnp-org:service:tvpicture:1"
};
#endif
//char *TvServiceName[] = { "Control", "Picture" };

//Kevin Add
char *Search_DeviceType;
char MmDeviceType[] = "urn:schemas-upnp-org:device:MediaServer:1";
char *MmServiceType[] = {
    "urn:schemas-upnp-org:service:ContentDirectory:1",
	/*"urn:schemas-upnp-org:service:ConnectionManager:1",*/
	"urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1"
};
char *MmServiceName[] = { "ContentDirectory", "ConnectionManager" };

//Kevin Add for InternetGatewayDevice
char IGDDeviceType[] = "urn:schemas-upnp-org:device:InternetGatewayDevice:1";
char *IGDServiceType[] = {
	"urn:schemas-upnp-org:service:WANIPConnection:1"
};


void UPNP_Set_SearchType(unsigned char type )
{
	DeviceSearchType = type;
	if( type == 0 )
	{
		Search_DeviceType = MmDeviceType;
	}
	if( type == 1 )
	{
		Search_DeviceType = IGDDeviceType;
	}

}
/*
   Global arrays for storing variable names and counts for 
   TvControl and TvPicture services 
 */
//char TvVarCount[MS_SERVICE_SERVCOUNT] =
//    { TV_CONTROL_VARCOUNT, TV_PICTURE_VARCOUNT };

/*
   Timeout to request during subscriptions 
 */
int default_timeout = 1801;

/*
   The first node in the global device list, or NULL if empty 
 */
struct TvDeviceNode *GlobalDeviceList = NULL;
/********************************************************************************
 * TvCtrlPointDeleteNode
 *
 * Description: 
 *       Delete a device node from the global device list.  Note that this
 *       function is NOT thread safe, and should be called from another
 *       function that has already locked the global device list.
 *
 * Parameters:
 *   node -- The device node
 *
 ********************************************************************************/
int
TvCtrlPointDeleteNode( struct TvDeviceNode *node )
{
    int rc,
      service,
      var;

    if( NULL == node ) {
        SampleUtil_Print( "ERROR: TvCtrlPointDeleteNode: Node is empty" );
        mpDebugPrint( "ERROR: TvCtrlPointDeleteNode: Node is empty" );
        return TV_ERROR;
    }
#if 0
    for( service = 0; service < MS_SERVICE_SERVCOUNT; service++ ) {
        /*
           If we have a valid control SID, then unsubscribe 
         */
        if( strcmp( node->device.TvService[service].SID, "" ) != 0 ) {
			MP_DEBUG("call UpnpUnSubscribe");

            rc = UpnpUnSubscribe( ctrlpt_handle,
                                  node->device.TvService[service].SID );
			MP_DEBUG("exit call UpnpUnSubscribe");
            if( UPNP_E_SUCCESS == rc ) {
                SampleUtil_Print
                    ( "Unsubscribed from Tv %s EventURL with SID=%s",
                      TvServiceName[service],
                      node->device.TvService[service].SID );
            } else {
                SampleUtil_Print
                    ( "Error unsubscribing to Tv %s EventURL -- %d",
                      TvServiceName[service], rc );
            }
        }
		MP_DEBUG("Kevin Free 2\n");
    }
#endif
    //Notify New Device Added
    //K SampleUtil_StateUpdate( NULL, NULL, node->device.UDN, DEVICE_REMOVED );
	mpDebugPrint("ext_mem_free node");
    ext_mem_free( node );
    node = NULL;

    return TV_SUCCESS;
}

/********************************************************************************
 * TvCtrlPointRemoveDevice
 *
 * Description: 
 *       Remove a device from the global device list.
 *
 * Parameters:
 *   UDN -- The Unique Device Name for the device to remove
 *
 ********************************************************************************/
int
TvCtrlPointRemoveDevice( char *UDN )
{
    struct TvDeviceNode *curdevnode,
     *prevdevnode;
	unsigned char server_index = 0;
	unsigned char i,bfound = 0;

    //K ithread_mutex_lock( &DeviceListMutex );
	MP_DEBUG("TvCtrlPointRemoveDevice\n");
    curdevnode = GlobalDeviceList;
    if( !curdevnode ) {
        SampleUtil_Print
            ( "WARNING: TvCtrlPointRemoveDevice: Device list empty" );
    } else {
        if( 0 == strcmp( curdevnode->device.UDN, UDN ) ) {
            GlobalDeviceList = curdevnode->next;
			mpDebugPrint("TvCtrlPointDeleteNode 1");
            TvCtrlPointDeleteNode( curdevnode );
			bfound = 1;
        } else {
            prevdevnode = curdevnode;
            curdevnode = curdevnode->next;
			server_index ++;
            while( curdevnode ) {
                if( strcmp( curdevnode->device.UDN, UDN ) == 0 ) {
                    prevdevnode->next = curdevnode->next;
					mpDebugPrint("TvCtrlPointDeleteNode 2 server_index %d",server_index);
                    TvCtrlPointDeleteNode( curdevnode );
					bfound = 1;
                    break;
                }
				server_index ++;
                prevdevnode = curdevnode;
                curdevnode = curdevnode->next;
            }
        }
    }
	if( bfound == 1 )
	{
		mpDebugPrint("if( bfound == 1 )");
		//Refresh ServerBrowse
		ServerBrowser.dwNumberOfServer = 0;
		ServerBrowser.dwCurrentServer = 0;
		ServerBrowser.dwFirstList = 0;
		ServerBrowser.dwListIndex = 0;
		
		for(i=0; i<MAX_NUM_SERVER; i++)
		{
			ServerBrowser.ServerList[i].bState = NW_INVALID;
		}
		//ServerBrowser.bState = AS_STATE_START;
		//ServerBrowser.dwNumberOfServer = 0;		
		curdevnode = GlobalDeviceList;
		while(curdevnode)
		{
			memcpy(&ServerBrowser.ServerList[ServerBrowser.dwNumberOfServer].ServerName,curdevnode->device.FriendlyName,strlen(curdevnode->device.FriendlyName)+1);
			memcpy(&ServerBrowser.ServerList[ServerBrowser.dwNumberOfServer].UrlName,curdevnode->device.PresURL,strlen(curdevnode->device.PresURL)+1);	
			ServerBrowser.ServerList[ServerBrowser.dwNumberOfServer].bState = NW_VALID;
			ServerBrowser.dwNumberOfServer++;		
            curdevnode = curdevnode->next;
		}
		mpDebugPrint("Device have been removed 2 %d",ServerBrowser.dwNumberOfServer);
		//EventSet(UI_EVENT, EVENT_SERVER_IN);
		if( g_bXpgStatus == XPG_MODE_UPNP_USER_LIST)
			EventSet(UI_EVENT, EVENT_SERVER_IN);
		if( server_index == gupnp_index )
		{
			if (g_bAniFlag & ANI_AUDIO)
				xpgStopAudio();

			MP_DEBUG("Device have been removed 1");
			if( g_bAniFlag & ANI_SLIDE )
				xpgCb_SlideExit();
			else
				ImageReleaseAllBuffer();
			while(g_bXpgStatus!=XPG_MODE_UPNP_USER_LIST)
			{
				xpgCb_NetBTNExit();
			}
			MP_DEBUG("Device have been removed 1-1");
		}
		else if( server_index < gupnp_index )
			gupnp_index --;

		MP_DEBUG("Device have been removed 3");
	}
    //K ithread_mutex_unlock( &DeviceListMutex );

    return TV_SUCCESS;
}

/********************************************************************************
 * TvCtrlPointRemoveAll
 *
 * Description: 
 *       Remove all devices from the global device list.
 *
 * Parameters:
 *   None
 *
 ********************************************************************************/
int
TvCtrlPointRemoveAll( void )
{
    struct TvDeviceNode *curdevnode,
     *next;

    //K ithread_mutex_lock( &DeviceListMutex );

    curdevnode = GlobalDeviceList;
    GlobalDeviceList = NULL;

	//MP_DEBUG("TvCtrlPointRemoveAll 1"); 
    while( curdevnode ) {
		MP_DEBUG("TvCtrlPointRemoveAll 2"); 
        next = curdevnode->next;
        TvCtrlPointDeleteNode( curdevnode );
        curdevnode = next;
    }

    //K ithread_mutex_unlock( &DeviceListMutex );
	//MP_DEBUG("TvCtrlPointRemoveAll END"); 
    return TV_SUCCESS;
}

/********************************************************************************
 * TvCtrlPointRefresh
 *
 * Description: 
 *       Clear the current global device list and issue new search
 *	 requests to build it up again from scratch.
 *
 * Parameters:
 *   None
 *
 ********************************************************************************/
int
TvCtrlPointRefresh( void )
{
    int rc;

	MP_DEBUG("%d",__FUNCTION__); 
    TvCtrlPointRemoveAll(  );

    /*
       Search for all devices of type tvdevice version 1, 
       waiting for up to 5 seconds for the response 
     */
    rc = UpnpSearchAsync( ctrlpt_handle, 5, Search_DeviceType, NULL );
    if( UPNP_E_SUCCESS != rc ) {
		MP_DEBUG("Error sending search request%d", rc ); 
        //SampleUtil_Print( "Error sending search request%d", rc );
        return TV_ERROR;
    }
    return TV_SUCCESS;
}

/********************************************************************************
 * TvCtrlPointGetVar
 *
 * Description: 
 *       Send a GetVar request to the specified service of a device.
 *
 * Parameters:
 *   service -- The service
 *   devnum -- The number of the device (order in the list,
 *             starting with 1)
 *   varname -- The name of the variable to request.
 *
 ********************************************************************************/
#if 0
int
TvCtrlPointGetVar( int service,
                   int devnum,
                   char *varname )
{
    struct TvDeviceNode *devnode;
    int rc;

    //K ithread_mutex_lock( &DeviceListMutex );

    rc = TvCtrlPointGetDevice( devnum, &devnode );

    if( TV_SUCCESS == rc ) {
        rc = UpnpGetServiceVarStatusAsync( ctrlpt_handle,
                                           devnode->device.
                                           TvService[service].ControlURL,
                                           varname,
                                           TvCtrlPointCallbackEventHandler,
                                           NULL );
        if( rc != UPNP_E_SUCCESS ) {
            SampleUtil_Print
                ( "Error in UpnpGetServiceVarStatusAsync -- %d", rc );
            rc = TV_ERROR;
        }
    }

    //K ithread_mutex_unlock( &DeviceListMutex );

    return rc;
}
#endif
/********************************************************************************
 * TvCtrlPointSendAction
 *
 * Description: 
 *       Send an Action request to the specified service of a device.
 *
 * Parameters:
 *   service -- The service
 *   devnum -- The number of the device (order in the list,
 *             starting with 1)
 *   actionname -- The name of the action.
 *   param_name -- An array of parameter names
 *   param_val -- The corresponding parameter values
 *   param_count -- The number of parameters
 *
 ********************************************************************************/
int
TvCtrlPointSendAction( int service,
                       int devnum,
                       char *actionname,
                       char **param_name,
                       char **param_val,
                       int param_count,
					   void *pmeta_entry)
{
    struct TvDeviceNode *devnode;
    IXML_Document *actionNode = NULL;
    int rc = TV_SUCCESS;
    int param;

	char startindex[10];
    //K ithread_mutex_lock( &DeviceListMutex );

    rc = TvCtrlPointGetDevice( devnum, &devnode );
	//devnode = GlobalDeviceList;
    if( TV_SUCCESS == rc ) 
	{
		//gbMicrosoftServer = devnode->device.bMicrosoftServer;
		gServerType = devnode->device.upupservertype;
        if( 0 == param_count ) 
		{
			actionNode = UpnpMakeAction( actionname, MmServiceType[service], 5, "ObjectID" , "0" ,"BrowseFlag","BrowseDirectChildren","Filter","*","StartingIndex","0","RequestedCount","10",NULL);
        } else {
			sprintf(startindex,"%d",gdir_child_cur_count);
			//MP_DEBUG("parm %s startindex %d\n",param_name[0],startindex);
			actionNode = UpnpMakeAction( actionname, MmServiceType[service], 5, "ObjectID" , param_name[0] ,"BrowseFlag","BrowseDirectChildren","Filter","*","StartingIndex",startindex,"RequestedCount","5",NULL);
        }
        rc = UpnpSendActionAsync( ctrlpt_handle,
                                  devnode->device.TvService[service].
                                  ControlURL, MmServiceType[service],
                                  NULL, actionNode,
                                  TvCtrlPointCallbackEventHandler, pmeta_entry );

        if( rc != UPNP_E_SUCCESS ) {
            SampleUtil_Print( "Error in UpnpSendActionAsync -- %d", rc );
            rc = TV_ERROR;
        }
    }
	//mpDebugPrint("TvCtrlPointSendAction AA");
    //K ithread_mutex_unlock( &DeviceListMutex );

    if( actionNode )
        ixmlDocument_free( actionNode );
	//mpDebugPrint("TvCtrlPointSendAction BB");
    return rc;
}

int
NatCtrlPointSendAction( int service,
                       int devnum,
                       char *actionname,
                       char **param_name,
                       char **param_val,
                       int param_count,
					   void *pnatretdata)
{
    struct TvDeviceNode *devnode;
    IXML_Document *actionNode = NULL;
    int rc = TV_SUCCESS;
    int param;

	char startindex[10];
    //K ithread_mutex_lock( &DeviceListMutex );

    rc = TvCtrlPointGetDevice( devnum, &devnode );
	//devnode = GlobalDeviceList;
	mpDebugPrint("param_count %d",param_count);
    if( TV_SUCCESS == rc ) 
	{
        if( 0 == param_count ) 
		{
			actionNode = UpnpMakeAction( actionname, IGDServiceType[service], 1 , NULL);
        } 
		else if(3 == param_count)
		{
			//MP_DEBUG("parm %s startindex %d\n",param_name[0],startindex);
			mpDebugPrint("param_name[0] %s",param_name[0]);
			mpDebugPrint("param_value[0] %s",param_val[0]);
			mpDebugPrint("param_name %x %x %x",param_name,param_name[0],param_name[1]);
			actionNode = UpnpMakeAction( actionname, IGDServiceType[service], 3, param_name[0] , param_val[0] , param_name[1] , param_val[1],param_name[2] , param_val[2], NULL);
			//actionNode = UpnpMakeAction( actionname, MmServiceType[service], 5, "ObjectID" , param_name[0] ,"BrowseFlag","BrowseDirectChildren","Filter","*","StartingIndex",startindex,"RequestedCount","5",NULL);
        }
		else if(8 == param_count)
		{
			//MP_DEBUG("parm %s startindex %d\n",param_name[0],startindex);
			mpDebugPrint("param_name[0] %s",param_name[0]);
			mpDebugPrint("param_value[0] %s",param_val[0]);
			mpDebugPrint("param_name %x %x %x",param_name,param_name[0],param_name[1]);
			actionNode = UpnpMakeAction( actionname, IGDServiceType[service], 8, param_name[0] , param_val[0] , 
																				param_name[1] , param_val[1],
																				param_name[2] , param_val[2], 
																				param_name[3] , param_val[3], 
																				param_name[4] , param_val[4], 
																				param_name[5] , param_val[5], 
																				param_name[6] , param_val[6], 
																				param_name[7] , param_val[7], NULL);
			//actionNode = UpnpMakeAction( actionname, MmServiceType[service], 5, "ObjectID" , param_name[0] ,"BrowseFlag","BrowseDirectChildren","Filter","*","StartingIndex",startindex,"RequestedCount","5",NULL);
        }

        rc = UpnpSendActionAsync( ctrlpt_handle,
                                  devnode->device.TvService[service].
                                  ControlURL, IGDServiceType[service],
                                  NULL, actionNode,
                                  NatCtrlPointCallbackEventHandler, pnatretdata);

        if( rc != UPNP_E_SUCCESS ) {
            SampleUtil_Print( "Error in UpnpSendActionAsync -- %d", rc );
            rc = TV_ERROR;
        }
    }

    //K ithread_mutex_unlock( &DeviceListMutex );

    if( actionNode )
        ixmlDocument_free( actionNode );

    return rc;
}

/********************************************************************************
 * TvCtrlPointSendActionNumericArg
 *
 * Description:Send an action with one argument to a device in the global device list.
 *
 * Parameters:
 *   devnum -- The number of the device (order in the list, starting with 1)
 *   service -- TV_SERVICE_CONTROL or TV_SERVICE_PICTURE
 *   actionName -- The device action, i.e., "SetChannel"
 *   paramName -- The name of the parameter that is being passed
 *   paramValue -- Actual value of the parameter being passed
 *
 ********************************************************************************/
#if 0
int
TvCtrlPointSendActionNumericArg( int devnum,
                                 int service,
                                 char *actionName,
                                 char *paramName,
                                 int paramValue )
{
    char param_val_a[50];
    char *param_val = param_val_a;

    sprintf( param_val_a, "%d", paramValue );

    return TvCtrlPointSendAction( service, devnum, actionName, &paramName,
                                  &param_val, 1 );
}
#endif
void TvCtrlStopSSDP( int devnum )
{
    //return TvCtrlPointSendAction( TV_SERVICE_CONTROL, devnum, "PowerOn",
      //                            NULL, NULL, 0 );
	MP_DEBUG("TvCtrlStopSSDP\n");
	gDeviceAdd = 1;
	return;
}


/********************************************************************************
 * TvCtrlPointGetDevice
 *
 * Description: 
 *       Given a list number, returns the pointer to the device
 *       node at that position in the global device list.  Note
 *       that this function is not thread safe.  It must be called 
 *       from a function that has locked the global device list.
 *
 * Parameters:
 *   devnum -- The number of the device (order in the list,
 *             starting with 1)
 *   devnode -- The output device node pointer
 *
 ********************************************************************************/
int
TvCtrlPointGetDevice( int devnum,
                      struct TvDeviceNode **devnode )
{
    int count = devnum;
    struct TvDeviceNode *tmpdevnode = NULL;

    if( count )
        tmpdevnode = GlobalDeviceList;

    while( --count && tmpdevnode ) {
        tmpdevnode = tmpdevnode->next;
    }

    if( !tmpdevnode ) {
        //SampleUtil_Print( "Error finding TvDevice number -- %d", devnum );
		MP_DEBUG( "Error finding TvDevice number -- %d", devnum );
        return TV_ERROR;
    }

    *devnode = tmpdevnode;
    return TV_SUCCESS;
}

/********************************************************************************
 * TvCtrlPointPrintList
 *
 * Description: 
 *       Print the universal device names for each device in the global device list
 *
 * Parameters:
 *   None
 *
 ********************************************************************************/
int
TvCtrlPointPrintList(  )
{
    struct TvDeviceNode *tmpdevnode;
    int i = 0;

    //K ithread_mutex_lock( &DeviceListMutex );

    SampleUtil_Print( "TvCtrlPointPrintList:" );
    tmpdevnode = GlobalDeviceList;
    while( tmpdevnode ) {
        SampleUtil_Print( " %3d -- %s", ++i, tmpdevnode->device.UDN );
        tmpdevnode = tmpdevnode->next;
    }
    SampleUtil_Print( "" );
    //K ithread_mutex_unlock( &DeviceListMutex );

    return TV_SUCCESS;
}

/********************************************************************************
 * TvCtrlPointPrintDevice
 *
 * Description: 
 *       Print the identifiers and state table for a device from
 *       the global device list.
 *
 * Parameters:
 *   devnum -- The number of the device (order in the list,
 *             starting with 1)
 *
 ********************************************************************************/
int
TvCtrlPointPrintDevice( int devnum )
{
    struct TvDeviceNode *tmpdevnode;
    int i = 0,
      service,
      var;
    char spacer[15];

    if( devnum <= 0 ) {
        SampleUtil_Print
            ( "Error in TvCtrlPointPrintDevice: invalid devnum = %d",
              devnum );
        return TV_ERROR;
    }

    //K ithread_mutex_lock( &DeviceListMutex );

    SampleUtil_Print( "TvCtrlPointPrintDevice:" );
    tmpdevnode = GlobalDeviceList;
    while( tmpdevnode ) {
        i++;
        if( i == devnum )
            break;
        tmpdevnode = tmpdevnode->next;
    }

    if( !tmpdevnode ) {
        SampleUtil_Print
            ( "Error in TvCtrlPointPrintDevice: invalid devnum = %d  --  actual device count = %d",
              devnum, i );
    } else {
        SampleUtil_Print( "  TvDevice -- %d", devnum );
        SampleUtil_Print( "    |                  " );
        SampleUtil_Print( "    +- UDN        = %s",
                          tmpdevnode->device.UDN );
        SampleUtil_Print( "    +- DescDocURL     = %s",
                          tmpdevnode->device.DescDocURL );
        SampleUtil_Print( "    +- FriendlyName   = %s",
                          tmpdevnode->device.FriendlyName );
        SampleUtil_Print( "    +- PresURL        = %s",
                          tmpdevnode->device.PresURL );
        SampleUtil_Print( "    +- Adver. TimeOut = %d",
                          tmpdevnode->device.AdvrTimeOut );
    }

    SampleUtil_Print( "" );
    //K ithread_mutex_unlock( &DeviceListMutex );

    return TV_SUCCESS;
}

/********************************************************************************
 * TvCtrlPointAddDevice
 *
 * Description: 
 *       If the device is not already included in the global device list,
 *       add it.  Otherwise, update its advertisement expiration timeout.
 *
 * Parameters:
 *   DescDoc -- The description document for the device
 *   location -- The location of the description document URL
 *   expires -- The expiration time for this advertisement
 *
 ********************************************************************************/
void
TvCtrlPointAddDevice( IXML_Document * DescDoc,
                      char *location,
                      int expires )
{
    char *deviceType = NULL;
    char *friendlyName = NULL;
    char presURL[200];
    char *baseURL = NULL;
    char *relURL = NULL;
    char *UDN = NULL;
    char *manufacturer = NULL;
    char *serviceId[MS_SERVICE_SERVCOUNT] = { NULL, NULL };
    char *eventURL[MS_SERVICE_SERVCOUNT] = { NULL, NULL };
    char *controlURL[MS_SERVICE_SERVCOUNT] = { NULL, NULL };
    Upnp_SID eventSID[MS_SERVICE_SERVCOUNT];
    int TimeOut[MS_SERVICE_SERVCOUNT] =
        { default_timeout, default_timeout };
    struct TvDeviceNode *deviceNode;
    struct TvDeviceNode *tmpdevnode;
    int ret = 1;
    int found = 0;
    int service,var;
    char *KevinTest = NULL;
	int i;
	//char m_bMicrosoftServer = 0;
	enum UpnpServerType ServerType = Others;
    //K ithread_mutex_lock( &DeviceListMutex );

    /*
       Read key elements from description document 
     */
    UDN = SampleUtil_GetFirstDocumentItem( DescDoc, "UDN" );
    //deviceType = SampleUtil_GetFirstDocumentItem( DescDoc, "deviceType" );
    friendlyName = SampleUtil_GetFirstDocumentItem( DescDoc, "friendlyName" );
    baseURL = SampleUtil_GetFirstDocumentItem( DescDoc, "URLBase" );
    relURL = SampleUtil_GetFirstDocumentItem( DescDoc, "presentationURL" );
    manufacturer = SampleUtil_GetFirstDocumentItem( DescDoc, "manufacturer" );
	//modelname = SampleUtil_GetFirstDocumentItem( DescDoc, "manufacturer" );
	if( manufacturer )
	{
		mpDebugPrint("manufacturer %s %s",manufacturer,friendlyName);
		if( strcmp(manufacturer,"Microsoft") == 0 )
			ServerType = Microsoft;
		else if( strcmp(manufacturer,"PacketVideo") == 0 )
			ServerType = PacketVideo;
		else if( strcmp(manufacturer,"Google") == 0 )
			ServerType = Google;
	}


    ret =
        UpnpResolveURL( ( baseURL ? baseURL : location ), relURL,
                        presURL );

	{
		MP_DEBUG("CP Add Device %s GlobalDeviceList %x\n",friendlyName,GlobalDeviceList);
        tmpdevnode = GlobalDeviceList;
        while( tmpdevnode ) {
            if( strcmp( tmpdevnode->device.UDN, UDN ) == 0 ) {
                found = 1;
                break;
            }
            tmpdevnode = tmpdevnode->next;
        }

        if( found ) {
            // The device is already there, so just update 
            // the advertisement timeout field
            tmpdevnode->device.AdvrTimeOut = expires;
        } 
		else 
		{
			for( service = 0; service < 1 ; service++ ) 
			{

                if( SampleUtil_FindAndParseService
                    ( DescDoc, location, MmServiceType[service],
                      &serviceId[service], &eventURL[service],
                      &controlURL[service] ) ) 
				{
                    mpDebugPrint( "Subscribing to EventURL %s...",eventURL[service] );
					
                    ret =
                        UpnpSubscribe( ctrlpt_handle, eventURL[service],
                                       &TimeOut[service],
                                       eventSID[service] );
					mpDebugPrint("ret %x",ret);
                    if( ret == UPNP_E_SUCCESS ) {
                        SampleUtil_Print
                            ( "Subscribed to EventURL with SID=%s",
                              eventSID[service] );
                    } else {
                        SampleUtil_Print
                            ( "Error Subscribing to EventURL -- %d", ret );
                        strcpy( eventSID[service], "" );
						goto free_resource;
                    }

                } else {
                    SampleUtil_Print( "Error: Could not find Service: %s",
                                      Search_DeviceType[service] );
					goto free_resource;
                }
			
			}
            /*
               Create a new device node 
             */
			mpDebugPrint("Create a new device node ");
            deviceNode =
                ( struct TvDeviceNode * )
                ext_mem_malloc( sizeof( struct TvDeviceNode ) );
            strcpy( deviceNode->device.UDN, UDN );
            strcpy( deviceNode->device.DescDocURL, location );
            strcpy( deviceNode->device.FriendlyName, friendlyName );
            strcpy( deviceNode->device.PresURL, presURL );
            deviceNode->device.AdvrTimeOut = expires;
			deviceNode->device.upupservertype = ServerType;
            for( service = 0; service < 1; service++ ) {
                strcpy( deviceNode->device.TvService[service].ServiceId,
                        serviceId[service] );
                strcpy( deviceNode->device.TvService[service].ServiceType,
                        MmServiceType[service] );
                strcpy( deviceNode->device.TvService[service].ControlURL,
                        controlURL[service] );
                strcpy( deviceNode->device.TvService[service].EventURL,
                        eventURL[service] );
                strcpy( deviceNode->device.TvService[service].SID,
                        eventSID[service] );
            }
            deviceNode->next = NULL;
			mpDebugPrint("ServerBrowser.dwNumberOfServer %d",ServerBrowser.dwNumberOfServer);
			memcpy(&ServerBrowser.ServerList[ServerBrowser.dwNumberOfServer].ServerName,deviceNode->device.FriendlyName,strlen(deviceNode->device.FriendlyName)+1);
			memcpy(&ServerBrowser.ServerList[ServerBrowser.dwNumberOfServer].UrlName,deviceNode->device.PresURL,strlen(deviceNode->device.PresURL)+1);	
			ServerBrowser.ServerList[ServerBrowser.dwNumberOfServer].bState = NW_VALID;
			ServerBrowser.dwNumberOfServer++;		
			if( g_bXpgStatus == XPG_MODE_UPNP_USER_LIST)
				xpgUpdateStage();
				//EventSet(UI_EVENT, EVENT_SERVER_IN);

			// Insert the new device node in the list
			if( ( tmpdevnode = GlobalDeviceList ) ) {

				while( tmpdevnode ) {
					if( tmpdevnode->next ) {
						tmpdevnode = tmpdevnode->next;
					} else {
						tmpdevnode->next = deviceNode;
						break;
					}
				}
			} else {
				GlobalDeviceList = deviceNode;
			}

            //Notify New Device Added
			mpDebugPrint("Notify New Device Added");
            SampleUtil_StateUpdate( NULL, NULL, deviceNode->device.UDN,
                                    DEVICE_ADDED );
		}				
	}

free_resource:
	mpDebugPrint("free resource");
    if( deviceType )
        ixml_mem_free( deviceType );
    if( friendlyName )
        ixml_mem_free( friendlyName );
    if( UDN )
        ixml_mem_free( UDN );
    if( baseURL )
        ixml_mem_free( baseURL );
    if( relURL )
        ixml_mem_free( relURL );
	if ( manufacturer )
        ixml_mem_free( manufacturer );

    for( service = 0; service < MS_SERVICE_SERVCOUNT; service++ ) {
        if( serviceId[service] )
   			ixml_mem_free(serviceId[service]);
        if( controlURL[service] )
            ixml_mem_free( controlURL[service] );
        if( eventURL[service] )
            ixml_mem_free( eventURL[service] );
    }
	mpDebugPrint("free resource end");
}


void
NatCtrlPointAddDevice( IXML_Document * DescDoc,
                      char *location,
                      int expires )
{
    char *deviceType = NULL;
    char *friendlyName = NULL;
    char presURL[200];
    char *baseURL = NULL;
    char *relURL = NULL;
    char *UDN = NULL;
    char *manufacturer = NULL;
    char *serviceId[MS_SERVICE_SERVCOUNT] = { NULL, NULL };
    char *eventURL[MS_SERVICE_SERVCOUNT] = { NULL, NULL };
    char *controlURL[MS_SERVICE_SERVCOUNT] = { NULL, NULL };
    Upnp_SID eventSID[MS_SERVICE_SERVCOUNT];
    int TimeOut[MS_SERVICE_SERVCOUNT] =
        { default_timeout, default_timeout };
    struct TvDeviceNode *deviceNode;
    struct TvDeviceNode *tmpdevnode;
    int ret = 1;
    int found = 0;
    int service,var;
    char *KevinTest = NULL;
	int i;
	//char m_bMicrosoftServer = 0;
	enum UpnpServerType ServerType = Others;
	char *pparamname[8];
	char *pparamvalue[8];
	char paramname[8][64];
	char paramvalue[8][64];
    int rc = TV_SUCCESS;
	struct NatReturnData *pnatretdata;
    unsigned int ipaddr;
	unsigned char bsetmapping = 0;
    //K ithread_mutex_lock( &DeviceListMutex );

    /*
       Read key elements from description document 
     */
	mpDebugPrint("NatCtrlPointAddDevice");
    UDN = SampleUtil_GetFirstDocumentItem( DescDoc, "UDN" );
    //deviceType = SampleUtil_GetFirstDocumentItem( DescDoc, "deviceType" );
    friendlyName = SampleUtil_GetFirstDocumentItem( DescDoc, "friendlyName" );
    baseURL = SampleUtil_GetFirstDocumentItem( DescDoc, "URLBase" );
    relURL = SampleUtil_GetFirstDocumentItem( DescDoc, "presentationURL" );
    manufacturer = SampleUtil_GetFirstDocumentItem( DescDoc, "manufacturer" );
	//modelname = SampleUtil_GetFirstDocumentItem( DescDoc, "manufacturer" );
	if( manufacturer )
	{
		mpDebugPrint("manufacturer %s %s",manufacturer,friendlyName);
	}


    ret =
        UpnpResolveURL( ( baseURL ? baseURL : location ), relURL,
                        presURL );
	//if( strstr(friendlyName,"PMM") )
	{
		//K strcpy(upnp_info->currentdir,friendlyName);
		//MP_DEBUG("Kevin Add Device %s GlobalDeviceList %x\n",friendlyName,GlobalDeviceList);
        tmpdevnode = GlobalDeviceList;
        while( tmpdevnode ) {
            if( strcmp( tmpdevnode->device.UDN, UDN ) == 0 ) {
                found = 1;
                break;
            }
            tmpdevnode = tmpdevnode->next;
        }

        if( found ) {
            // The device is already there, so just update 
            // the advertisement timeout field
            tmpdevnode->device.AdvrTimeOut = expires;
        } 
		else 
		{

            if( SampleUtil_FindAndParseServiceNat
                ( DescDoc, location, IGDServiceType[0],
                  &serviceId[0], &eventURL[0],
                  &controlURL[0] ) ) 
			{
				mpDebugPrint("Subscribing to EventURL %s...",eventURL[service] );
                ret = UpnpSubscribe( ctrlpt_handle, eventURL[0],
                                   &TimeOut[0],
                                   eventSID[0] );
                if( ret == UPNP_E_SUCCESS ) {
                    SampleUtil_Print
                        ( "Subscribed to EventURL with SID=%s",
                          eventSID[service] );
                } else {
                    SampleUtil_Print
                        ( "Error Subscribing to EventURL -- %d", ret );
                    strcpy( eventSID[service], "" );
                }

            } else {
                SampleUtil_Print( "Error: Could not find Service: %s",
                                  Search_DeviceType[service] );
            }

            /*
               Create a new device node 
             */
			mpDebugPrint("###########event URL CD %s",eventURL[0]);
            deviceNode =
                ( struct TvDeviceNode * )
                ext_mem_malloc( sizeof( struct TvDeviceNode ) );
            strcpy( deviceNode->device.UDN, UDN );
            strcpy( deviceNode->device.DescDocURL, location );
            strcpy( deviceNode->device.FriendlyName, friendlyName );
            strcpy( deviceNode->device.PresURL, presURL );
            deviceNode->device.AdvrTimeOut = expires;
			deviceNode->device.upupservertype = ServerType;
            for( service = 0; service < 1; service++ ) {
                strcpy( deviceNode->device.TvService[service].ServiceId,
                        serviceId[service] );
                strcpy( deviceNode->device.TvService[service].ServiceType,
                        MmServiceType[service] );
                strcpy( deviceNode->device.TvService[service].ControlURL,
                        controlURL[service] );
                strcpy( deviceNode->device.TvService[service].EventURL,
                        eventURL[service] );
                strcpy( deviceNode->device.TvService[service].SID,
                        eventSID[service] );
            }
            deviceNode->next = NULL;
/*
			mpDebugPrint("ServerBrowser.dwNumberOfServer %d",ServerBrowser.dwNumberOfServer);
			memcpy(&ServerBrowser.ServerList[ServerBrowser.dwNumberOfServer].ServerName,deviceNode->device.FriendlyName,strlen(deviceNode->device.FriendlyName)+1);
			memcpy(&ServerBrowser.ServerList[ServerBrowser.dwNumberOfServer].UrlName,deviceNode->device.PresURL,strlen(deviceNode->device.PresURL)+1);	
			ServerBrowser.ServerList[ServerBrowser.dwNumberOfServer].bState = NW_VALID;
			ServerBrowser.dwNumberOfServer++;		
			if( g_bXpgStatus == XPG_MODE_UPNP_USER_LIST)
				EventSet(UI_EVENT, EVENT_SERVER_IN);
*/
			// Insert the new device node in the list
			if( ( tmpdevnode = GlobalDeviceList ) ) {

				while( tmpdevnode ) {
					if( tmpdevnode->next ) {
						tmpdevnode = tmpdevnode->next;
					} else {
						tmpdevnode->next = deviceNode;
						break;
					}
				}
			} else {
				GlobalDeviceList = deviceNode;
			}
			
            //Notify New Device Added
            SampleUtil_StateUpdate( NULL, NULL, deviceNode->device.UDN,
                                    DEVICE_ADDED );
		}				
	}
	NatDeviceCout++;
    if( deviceType )
        ixml_mem_free( deviceType );
    if( friendlyName )
        ixml_mem_free( friendlyName );
    if( UDN )
        ixml_mem_free( UDN );
    if( baseURL )
        ixml_mem_free( baseURL );
    if( relURL )
        ixml_mem_free( relURL );
	if ( manufacturer )
        ixml_mem_free( manufacturer );

    for( service = 0; service < 1; service++ ) {
        if( serviceId[service] )
   			ixml_mem_free(serviceId[service]);
        if( controlURL[service] )
            ixml_mem_free( controlURL[service] );
        if( eventURL[service] )
            ixml_mem_free( eventURL[service] );
    }
	//printixmlpool();
	pnatretdata = (struct NatReturnData *) ixml_mem_malloc(sizeof(struct NatReturnData));
	for(i =0 ;i < 8 ; i++ )
	{
		pparamname[i] = paramname[i];
		pparamvalue[i] = paramvalue[i];
	}
	pnatretdata->ExternalIP = NULL;
	pnatretdata->PortMappingDescription = NULL;
	pnatretdata->InternalClient = NULL;

	gExternalPort = 0;
	UPNPNATCOMMAND = 0;
	NatCtrlPointSendAction( 0, NatDeviceCout, "GetExternalIPAddress",NULL, NULL, 0 ,pnatretdata);	
	if( UpnpReturnCode == 0 )
	{
		mpDebugPrint("ExternalIP %s",pnatretdata->ExternalIP);
		strcpy(gExternalIp,pnatretdata->ExternalIP);
	}
	strcpy(paramname[0],"NewRemoteHost");
	strcpy(paramvalue[0],"");
	strcpy(paramname[1],"NewExternalPort");
	strcpy(paramvalue[1],"5050");
	strcpy(paramname[2],"NewProtocol");
	strcpy(paramvalue[2],"TCP");
	UPNPNATCOMMAND = 1;
	//NatCtrlPointSendAction( 0, NatDeviceCout, "GetGenericPortMappingEntry",(char **) paramname, (char **) paramvalue, 1 ,NULL);
	UpnpReturnCode = 0;
	rc = NatCtrlPointSendAction( 0, NatDeviceCout, "GetSpecificPortMappingEntry",(char **) pparamname, (char **) pparamvalue, 3 ,pnatretdata);

	mpDebugPrint("rc %d",rc);
	if( rc == UPNP_E_SUCCESS )
	{
	    //ipaddr = NetDefaultIpGet();
		//mpDebugPrint("ipaddr %x",ipaddr);
		//IpToString(ipaddr,LOCAL_HOST);
		mpDebugPrint("LOCAL_HOST %s",LOCAL_HOST);
		if( UpnpReturnCode == 0 )
		{
			if( strcmp(pnatretdata->PortMappingDescription,"MagicPixel TCP") == 0 && strcmp(pnatretdata->InternalClient,LOCAL_HOST) == 0 )
			{
				mpDebugPrint("Already Done");
				gExternalPort = 5050;
			}
			else
			{
				mpDebugPrint("Should DeletePortMapping");
				UPNPNATCOMMAND = 8;
				rc = NatCtrlPointSendAction( 0, NatDeviceCout, "DeletePortMapping",(char **) pparamname, (char **) pparamvalue, 3 ,pnatretdata);
				if( UpnpReturnCode == 0 )
					bsetmapping = 1;
			}
			
		}
		else if(UpnpReturnCode == 714 )
		{
			bsetmapping = 1;
		}
		if(	bsetmapping  == 1 )
		{
			mpDebugPrint("Add Port Mapping");
			strcpy(paramname[0],"NewRemoteHost");
			strcpy(paramvalue[0],"");
			strcpy(paramname[1],"NewExternalPort");
			strcpy(paramvalue[1],"5050");
			strcpy(paramname[2],"NewProtocol");
			strcpy(paramvalue[2],"TCP");
			strcpy(paramname[3],"NewInternalPort");
			strcpy(paramvalue[3],"5151");
			strcpy(paramname[4],"NewInternalClient");
			strcpy(paramvalue[4],LOCAL_HOST);
			strcpy(paramname[5],"NewEnabled");
			strcpy(paramvalue[5],"1");
			strcpy(paramname[6],"NewPortMappingDescription");
			strcpy(paramvalue[6],"MagicPixel TCP");
			strcpy(paramname[7],"NewLeaseDuration");
			strcpy(paramvalue[7],"0");
			UPNPNATCOMMAND = 2;
			NatCtrlPointSendAction( 0, NatDeviceCout, "AddPortMapping",(char **) pparamname, (char **) pparamvalue, 8 ,NULL);
			gExternalPort = 5050;
		}
	}
	if( pnatretdata->ExternalIP )
		ixml_mem_free(pnatretdata->ExternalIP);
	if( pnatretdata->PortMappingDescription )
		ixml_mem_free(pnatretdata->PortMappingDescription);
	if( pnatretdata->InternalClient )
		ixml_mem_free(pnatretdata->InternalClient);
	if( pnatretdata )
		ixml_mem_free(pnatretdata);
}

/********************************************************************************
 * TvStateUpdate
 *
 * Description: 
 *       Update a Tv state table.  Called when an event is
 *       received.  Note: this function is NOT thread save.  It must be
 *       called from another function that has locked the global device list.
 *
 * Parameters:
 *   UDN     -- The UDN of the parent device.
 *   Service -- The service state table to update
 *   ChangedVariables -- DOM document representing the XML received
 *                       with the event
 *   State -- pointer to the state table for the Tv  service
 *            to update
 *
 ********************************************************************************/
#if 0
void
TvStateUpdate( char *UDN,
               int Service,
               IXML_Document * ChangedVariables,
               char **State )
{
    IXML_NodeList *properties,
     *variables;
    IXML_Element *property,
     *variable;
    int length,
      length1;
    int i,
      j;
    char *tmpstate = NULL;

    SampleUtil_Print( "Tv State Update (service %d): ", Service );
	//return;

    /*
       Find all of the e:property tags in the document 
     */
    properties =
        ixmlDocument_getElementsByTagName( ChangedVariables,
                                           "e:property" );
    if( NULL != properties ) {
        length = ixmlNodeList_length( properties );
        for( i = 0; i < length; i++ ) { /* Loop through each property change found */
            property =
                ( IXML_Element * ) ixmlNodeList_item( properties, i );

            /*
               For each variable name in the state table, check if this
               is a corresponding property change 
             */
            for( j = 0; j < TvVarCount[Service]; j++ ) {
                variables =
                    ixmlElement_getElementsByTagName( property,
                                                      TvVarName[Service]
                                                      [j] );

                /*
                   If a match is found, extract the value, and update the state table 
                 */
                if( variables ) {
                    length1 = ixmlNodeList_length( variables );
                    if( length1 ) {
                        variable =
                            ( IXML_Element * )
                            ixmlNodeList_item( variables, 0 );
                        tmpstate = SampleUtil_GetElementValue( variable );

                        if( tmpstate ) {
                            strcpy( State[j], tmpstate );
                            SampleUtil_Print
                                ( " Variable Name: %s New Value:'%s'",
                                  TvVarName[Service][j], State[j] );
                        }

                        if( tmpstate )
                            free( tmpstate );
                        tmpstate = NULL;
                    }

                    ixmlNodeList_free( variables );
                    variables = NULL;
                }
            }

        }
        ixmlNodeList_free( properties );
    }
	MP_DEBUG("Kevin TvStateUpdate END\n");
}
#endif
/********************************************************************************
 * TvCtrlPointHandleEvent
 *
 * Description: 
 *       Handle a UPnP event that was received.  Process the event and update
 *       the appropriate service state table.
 *
 * Parameters:
 *   sid -- The subscription id for the event
 *   eventkey -- The eventkey number for the event
 *   changes -- The DOM document representing the changes
 *
 ********************************************************************************/
void
TvCtrlPointHandleEvent( Upnp_SID sid,
                        int evntkey,
                        IXML_Document * changes )
{
    struct TvDeviceNode *tmpdevnode;
    int service;

    //K ithread_mutex_lock( &DeviceListMutex );

    tmpdevnode = GlobalDeviceList;
#if 0
    while( tmpdevnode ) {
        for( service = 0; service < MS_SERVICE_SERVCOUNT; service++ ) {
            if( strcmp( tmpdevnode->device.TvService[service].SID, sid ) ==
                0 ) {
                SampleUtil_Print( "Received Tv %s Event: %d for SID %s",
                                  TvServiceName[service], evntkey, sid );

                TvStateUpdate( tmpdevnode->device.UDN, service, changes,
                               ( char ** )&tmpdevnode->device.
                               TvService[service].VariableStrVal );
                break;
            }
        }
        tmpdevnode = tmpdevnode->next;
    }
#endif
    //K ithread_mutex_unlock( &DeviceListMutex );
}

/********************************************************************************
 * TvCtrlPointHandleSubscribeUpdate
 *
 * Description: 
 *       Handle a UPnP subscription update that was received.  Find the 
 *       service the update belongs to, and update its subscription
 *       timeout.
 *
 * Parameters:
 *   eventURL -- The event URL for the subscription
 *   sid -- The subscription id for the subscription
 *   timeout  -- The new timeout for the subscription
 *
 ********************************************************************************/
void
TvCtrlPointHandleSubscribeUpdate( char *eventURL,
                                  Upnp_SID sid,
                                  int timeout )
{
    struct TvDeviceNode *tmpdevnode;
    int service;

    //K ithread_mutex_lock( &DeviceListMutex );

    tmpdevnode = GlobalDeviceList;
    while( tmpdevnode ) {
        for( service = 0; service < MS_SERVICE_SERVCOUNT; service++ ) {

            if( strcmp
                ( tmpdevnode->device.TvService[service].EventURL,
                  eventURL ) == 0 ) {
                strcpy( tmpdevnode->device.TvService[service].SID, sid );
                break;
            }
        }

        tmpdevnode = tmpdevnode->next;
    }

    //K ithread_mutex_unlock( &DeviceListMutex );
}


/********************************************************************************
 * TvCtrlPointCallbackEventHandler
 *
 * Description: 
 *       The callback handler registered with the SDK while registering
 *       the control point.  Detects the type of callback, and passes the 
 *       request on to the appropriate function.
 *
 * Parameters:
 *   EventType -- The type of callback event
 *   Event -- Data structure containing event data
 *   Cookie -- Optional data specified during callback registration
 *
 ********************************************************************************/
int
TvCtrlPointCallbackEventHandler( Upnp_EventType EventType,
                                 void *Event,
                                 void *Cookie )
{
	Net_Photo_Entry file_info;
	netfs_meta_entry_t *path_entry;
	netfs_dir_stream_t  *netfs_dir;
	netfs_meta_entry_t  *next_netfs_dir;
	netfs_meta_entry_t  *next_netfs_dir_prev;
	netfs_meta_entry_t  *pmeta_entry = (netfs_meta_entry_t *)Cookie;
    //SampleUtil_PrintEvent( EventType, Event );
    switch ( EventType ) {
            /*
               SSDP Stuff 
             */
        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
        case UPNP_DISCOVERY_SEARCH_RESULT:
            {
				int found = 0;
				struct TvDeviceNode *tmpdevnode = NULL;
                struct Upnp_Discovery *d_event =
                    ( struct Upnp_Discovery * )Event;
                IXML_Document *DescDoc = NULL;
                int ret;

				//MP_DEBUG("UPNP_DISCOVERY_SEARCH_RESULT 1 %x",d_event); 
                if( d_event->ErrCode != UPNP_E_SUCCESS ) {
                    SampleUtil_Print( "Error in Discovery Callback -- %d",
                                      d_event->ErrCode );
                }

				if( ( tmpdevnode = GlobalDeviceList ) )
				{
					tmpdevnode = GlobalDeviceList;
					while( tmpdevnode ) {

						if( strcmp( tmpdevnode->device.DescDocURL, d_event->Location ) == 0 ) {
							found = 1;
							break;
						}
						tmpdevnode = tmpdevnode->next;
					}

				}
				if( found || (g_bXpgStatus != XPG_MODE_UPNP_USER_LIST) )
				{
					MP_DEBUG("######%s Already exit.",d_event->Location );
					printixmlpool();
					goto foundandexit;
				}
				mpDebugPrint("call UpnpDownloadXmlDoc a");
				printixmlpool();
                if( ( ret = UpnpDownloadXmlDoc( d_event->Location,&DescDoc,0 ) ) !=UPNP_E_SUCCESS ) 
				{
					mpDebugPrint("Error obtaining device description "); 
                } else {
					mpDebugPrint("DeviceSearchType %d ",DeviceSearchType);
					if( DeviceSearchType == 0 )
						TvCtrlPointAddDevice( DescDoc, d_event->Location,
											  d_event->Expires );
					else 
						NatCtrlPointAddDevice( DescDoc, d_event->Location,
											  d_event->Expires );

                }
				printixmlpool();
foundandexit:
				mpDebugPrint("DescDoc %p",DescDoc);
                if( DescDoc )
				{
                    ixmlDocument_free( DescDoc );
				}
				//printixmlpool();
                //TvCtrlPointPrintList(  );
                break;
            }

        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
            /*
               Nothing to do here... 
             */
            break;

        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
            {
                struct Upnp_Discovery *d_event =
                    ( struct Upnp_Discovery * )Event;

                if( d_event->ErrCode != UPNP_E_SUCCESS ) {
                    SampleUtil_Print
                        ( "Error in Discovery ByeBye Callback -- %d",
                          d_event->ErrCode );
                }
				MP_DEBUG("Received ByeBye for Device %s",d_event->DeviceId);
                SampleUtil_Print( "Received ByeBye for Device: %s",
                                  d_event->DeviceId );
                TvCtrlPointRemoveDevice( d_event->DeviceId );

                SampleUtil_Print( "After byebye:" );
                TvCtrlPointPrintList(  );

                break;
            }

            /*
               SOAP Stuff 
             */
        case UPNP_CONTROL_ACTION_COMPLETE:
            {
				char *resulptr,*tmpptr1,*tmpptr2,*ixmlthumbnailptr;
				char extname[16];
				int i,j,cl;
				unsigned char file_type;
				unsigned int filesize;
				unsigned short childcount;
				char c;
				char filteroutnotimage = 0;
				unsigned fordercount = 0;
				netfs_meta_entry_t *cur_netfs_entry;
                struct Upnp_Action_Complete *a_event =
                    ( struct Upnp_Action_Complete * )Event;
				unsigned char bisroot = 0;
				unsigned char m_number_return;
				//static unsigned char Connect_to_Server_Fail_count = 0;
				IXML_Document *respDoc = NULL;
				IXML_Node *tmpNode,*nextNode,*mchild;
				char *tmpptr;
				u8 rescount;
				u8 thumbnal_founnd;

				mpDebugPrint("UPNP_CONTROL_ACTION_COMPLETE");
                if( a_event->ErrCode != UPNP_E_SUCCESS ) {
                    SampleUtil_Print
                        ( "Error in  Action Complete Callback -- %d",
                          a_event->ErrCode );
					if( a_event->ErrCode == 801 )
					{
						mpDebugPrint("Access Deny.");
						bAccess_Deny = 1;
					}
					else if( a_event->ErrCode == UPNP_E_SOCKET_ERROR )
					{
						mpDebugPrint("UPNP Error Code %d",a_event->ErrCode);
						//Connect_to_Server_Fail_count++;
						//if( Connect_to_Server_Fail_count > 3 )
						{
							struct TvDeviceNode *devnode;
							int rc;
							MP_DEBUG("TvCtrlPointGetDevice");
							rc = TvCtrlPointGetDevice( gupnp_index+1, &devnode );
							MP_DEBUG("TvCtrlPointGetDevice rc %d",rc);
							if( TV_SUCCESS == rc )
							{
								TvCtrlPointRemoveDevice(devnode->device.UDN);
							}
						}
					}
					else
					{
						cur_netfs_entry = pmeta_entry;
						mpDebugPrint("UPNP Error Code %d",a_event->ErrCode);
						if( cur_netfs_entry )
						{
							if( !cur_netfs_entry->child )
							{
								cur_netfs_entry->child = netfs_create_dir_child(cur_netfs_entry);
							}
						}
					}
					return a_event->ErrCode;
					//break;
                }
				else
					bAccess_Deny = 0;
				//Connect_to_Server_Fail_count = 0;
                /*g
                   No need for any processing here, just print out results.  Service state
                   table updates are handled by events. 
                 */
				cur_netfs_entry = pmeta_entry;
#if 0
				if( cur_netfs_entry )
				{
					mpDebugPrint("cur_netfs_entry %x %s",cur_netfs_entry,cur_netfs_entry->containerid );
				}
#endif
				resulptr = SampleUtil_GetFirstDocumentItem(a_event->ActionResult,"TotalMatches");
				if( resulptr )
				{
					mpDebugPrint("resulptr %s",resulptr);
					gdir_child_total_count = (int ) strtol(resulptr,NULL,10);
					mpDebugPrint("GetTotalMatches %d",gdir_child_total_count);
					mpDebugPrint("gServerType %d",gServerType);
					ixml_mem_free(resulptr);
					if( gServerType == Google )
						cur_netfs_entry->childcount = gdir_child_total_count;

				}
				else
				{
/*
					snprintf(file_info.pathname, NETFS_MAX_PATHNAME,"/%s/null.null", upnpcurdir, "null");
					netfs_add_forder(&file_info,0);
					netfs_dir = netfs_opendir(upnpcurdir);
					//mpDebugPrint("path_entry %x",path_entry);
					mpDebugPrint("netfs_dir %x",netfs_dir);
					next_netfs_dir = netfs_dir->first_entry;
					gnetfs_dir = next_netfs_dir;
*/
					if( cur_netfs_entry )
					{
						mpDebugPrint("Nothing!! %s",cur_netfs_entry->containerid,cur_netfs_entry->child);
						if( !cur_netfs_entry->child )
						{
							cur_netfs_entry->child = netfs_create_dir_child(cur_netfs_entry);
						}
					}

					break;
				}
				//resulptr = SampleUtil_GetFirstDocumentItem(a_event->ActionResult,"childCount");
				resulptr = SampleUtil_GetFirstDocumentItem(a_event->ActionResult,"NumberReturned");
				//mpDebugPrint("NumberReturned resulptr %s",resulptr);
				m_number_return = (unsigned char) strtol(resulptr,NULL,10);
				gdir_child_cur_count += m_number_return;
				if( cur_netfs_entry )
				{
					cur_netfs_entry->curchildcount += (unsigned short) strtol(resulptr,NULL,10);
					gdir_child_cur_count = cur_netfs_entry->curchildcount;
					//mpDebugPrint("gdir_child_cur_count %d",gdir_child_cur_count);
					//kcur_netfs_entry = cur_netfs_entry;
				}
				else
				{
					bisroot = 1;
					mpDebugPrint("root dir========>then add root dir");
					dma_invalid_dcache();
					mpDebugPrint("upnpcurdir %s %p",upnpcurdir,&upnpcurdir);
					strcpy(upnpcurdir,"/Upnp");
					snprintf(file_info.pathname, NETFS_MAX_PATHNAME,"/%s/%s/%s.null", upnpcurdir, "root", "null");
					strcpy(file_info.url,"0");
					netfs_add_forder(&file_info,gdir_child_total_count);
					strcat(upnpcurdir,"/root");
					//kcur_netfs_entry = NULL;
				}
				if( resulptr )
					ixml_mem_free(resulptr);

				if( m_number_return == 0 )
				{
					//mpDebugPrint("Nothing!!");
					if( cur_netfs_entry )
					{
						//mpDebugPrint("Nothing!! %s",cur_netfs_entry->containerid,cur_netfs_entry->child);
						if( !cur_netfs_entry->child )
						{
							cur_netfs_entry->child = netfs_create_dir_child(cur_netfs_entry);
						}
					}
/*
					snprintf(file_info.pathname, NETFS_MAX_PATHNAME,"/%s/null/%s.null", upnpcurdir, "null");
					strcpy(file_info.url,"0");
					netfs_add_forder(&file_info,0);
					strcat(upnpcurdir,"/null");
					mpDebugPrint("Nothingaa!!");
*/
					break;
				}
				resulptr = SampleUtil_GetFirstDocumentItem(a_event->ActionResult,"Result");
				mpDebugPrint("Result1 resulptr %x",resulptr);
				//printixmlpool();
				if( resulptr )
				{
					//release some resource
	                ixmlDocument_free( a_event->ActionRequest );
					a_event->ActionRequest = NULL;
	                ixmlDocument_free( a_event->ActionResult );
					a_event->ActionResult = NULL;
					printixmlpool();
					if( ixmlParseBufferEx( resulptr, &respDoc ) == IXML_INVALID_PARAMETER )
						mpDebugPrint("IXML_INVALID_PARAMETER");
					else
					{
						printixmlpool();
						tmpNode = respDoc->n.firstChild->firstChild;
						while(tmpNode)
						{
							mpDebugPrint("%s %s",tmpNode->nodeName,tmpNode->nodeValue);
							if( strcmp(tmpNode->nodeName,"container") == 0 )	//forder
							{
								tmpptr = ixmlElement_getAttribute((IXML_Element *) tmpNode,"id");
								if( tmpptr )
								{
									mpDebugPrint("id %s",tmpptr);
									strcpy(file_info.url,tmpptr);
									tmpptr = ixmlElement_getAttribute((IXML_Element *) tmpNode,"childCount");
									//mpDebugPrint("childCount %s",tmpptr);
									if( tmpptr )
										childcount  = (unsigned short) strtol(tmpptr,NULL,10);
									else
										childcount = 0;
									//snprintf(file_info.pathname, NETFS_MAX_PATHNAME,"/%s/%s/%s.null", upnpcurdir, dctitle/*upnp_map->cur_set->title*/, "null");
									if( tmpNode->firstChild )
									{
										//mpDebugPrint("child %s %s",tmpNode->firstChild->nodeName,tmpNode->firstChild->nodeValue);
										if( tmpNode->firstChild->firstChild )
										{
											//mpDebugPrint("child %s %s",tmpNode->firstChild->firstChild->nodeName,tmpNode->firstChild->firstChild->nodeValue);
											dma_invalid_dcache();
											snprintf(file_info.pathname, NETFS_MAX_PATHNAME,"/%s/%s/%s.null", upnpcurdir, tmpNode->firstChild->firstChild->nodeValue, "null");
										}
									}
									mpDebugPrint("netfs_add %s %d",file_info.pathname,childcount);
									netfs_add_forder(&file_info,childcount);
									//mpDebugPrint("Result1b resulptr %x",resulptr);
								}
							}
							else if	(strcmp(tmpNode->nodeName,"item") == 0 )//file
							{
								tmpptr = ixmlElement_getAttribute((IXML_Element *) tmpNode,"id");
								if( tmpptr )
								{
									if( tmpNode->firstChild )
									{
										//mpDebugPrint("child %s %s",tmpNode->firstChild->nodeName,tmpNode->firstChild->nodeValue);
										if( tmpNode->firstChild->firstChild )
										{
											//mpDebugPrint("childchild %s %s",tmpNode->firstChild->firstChild->nodeName,tmpNode->firstChild->firstChild->nodeValue);
										}
										mchild = tmpNode->firstChild->nextSibling;
										mpDebugPrint("child %s %s",mchild->nodeName,mchild->nodeValue);
										rescount = 0;
										while(mchild)
										{
											thumbnal_founnd = 0;
											if( strcmp(mchild->nodeName,"res") == 0 )
											{
												tmpptr = ixmlElement_getAttribute((IXML_Element *) mchild,"size");
												file_info.file_size = ( int ) strtol(tmpptr,NULL,10);
												mpDebugPrint("tmpptr %s %d",tmpptr,file_info.file_size);
												tmpptr = ixmlElement_getAttribute((IXML_Element *) mchild,"protocolInfo");
												mpDebugPrint("tmpptr %s",tmpptr);
												tmpptr2 = strstr(tmpptr,"image/");
												memset(extname,0,16);
												if( tmpptr2 )
												{
													file_type = IMAGE_FILE_TYPE;
													if( strstr(tmpptr,"image/jpeg") )
														strcpy(extname,"jpg");
													else
													{
														i = strlen("image/");
														j = 0;
														while( tmpptr2[i] != ':' )
														{
															MP_DEBUG1("%c",tmpptr2[i]);
															extname[j++] = tmpptr2[i++];
														}
													}
													MP_DEBUG1("extname %s",extname);
												}
												else if( tmpptr2 = strstr(tmpptr,"audio/") )
												{
													file_type = AUDIO_FILE_TYPE;
													i = strlen("audio/");
													j = 0;
													while( tmpptr2[i] != ':' )
													{
														mpDebugPrint("%c",tmpptr2[i]);
														extname[j++] = tmpptr2[i++];
													}
													if( strstr(tmpptr,"mpeg") )
													{
														strcpy(extname,"mp3");
													}
													else if(strstr(extname,"x-ms-wma"))
													{
														strcpy(extname,"wma");
													}
													MP_DEBUG1("extname %s",extname);

												}
												else if( tmpptr2 = strstr(tmpptr,"video/") )
												{
													file_type = VIDEO_FILE_TYPE;
													i = strlen("video/");
													j = 0;
													while( tmpptr2[i] != ':' )
													{
														MP_DEBUG1("%c",tmpptr2[i]);
														extname[j++] = tmpptr2[i++];
													}
													if(strstr(extname,"x-ms-wmv"))
														strcpy(extname,"wmv");

												}
												else if( strstr(tmpptr,"application/octet-stream") )
												{
													strcpy(extname,"bin");
													file_type = UNKNOW_FILE_TYPE;
												}
												while( tmpptr2 = strstr(tmpNode->firstChild->firstChild->nodeValue,"/") )
												{
													*tmpptr2 = '_';		//change '/' to '_'
												}
												if( rescount == 0 )
												{
													dma_invalid_dcache();
													if( gServerType != Google )
														snprintf(file_info.pathname, NETFS_MAX_PATHNAME,"/%s/%s.%s", upnpcurdir, tmpNode->firstChild->firstChild->nodeValue,extname);
													else
														snprintf(file_info.pathname, NETFS_MAX_PATHNAME,"/%s/%s", upnpcurdir, tmpNode->firstChild->firstChild->nodeValue);
													mpDebugPrint("childchild %s %s",mchild->firstChild->nodeName,mchild->firstChild->nodeValue);
													strcpy(file_info.url,mchild->firstChild->nodeValue);
													if( file_type != IMAGE_FILE_TYPE)
														break;
												}													
												else
												{
													if( strstr(tmpptr,"_TN") )
													{
														//mpDebugPrint("tmpptr %s",tmpptr);
														mpDebugPrint("thumbnail_url %s %s",mchild->firstChild->nodeName,mchild->firstChild->nodeValue);
														strcpy(file_info.thumbnail_url,mchild->firstChild->nodeValue);
														thumbnal_founnd = 1;
														break;
													}
												}
												rescount++;
											}
											mchild = mchild->nextSibling;
										}//while(mchild)
										if( (file_type == IMAGE_FILE_TYPE) && (thumbnal_founnd == 0) )
											strcpy(file_info.thumbnail_url,file_info.url);
										//file_info.file_size = 10240;
										netfs_add_file_with_type(&file_info,file_type);
										dma_invalid_dcache();

									}									
								}
							}
							tmpNode = tmpNode->nextSibling;
						}
						ixmlDocument_free( respDoc );	
						printixmlpool();
					}
				}
				MP_DEBUG("upnpcurdir len %d",strlen(upnpcurdir));
				netfs_dir = netfs_opendir(upnpcurdir);
				next_netfs_dir = netfs_dir->first_entry;
				gnetfs_dir = next_netfs_dir;
				if( resulptr )
				{
					mpDebugPrint("free resulptr %x",resulptr);
					ixml_mem_free(resulptr);
				}
				dma_invalid_dcache();
                break;
            }
            /*
               GENA Stuff 
             */
        case UPNP_EVENT_RECEIVED:
            {
                struct Upnp_Event *e_event = ( struct Upnp_Event * )Event;

                TvCtrlPointHandleEvent( e_event->Sid, e_event->EventKey,
                                        e_event->ChangedVariables );
                break;
            }

        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
        case UPNP_EVENT_RENEWAL_COMPLETE:
            {
                struct Upnp_Event_Subscribe *es_event =
                    ( struct Upnp_Event_Subscribe * )Event;

                if( es_event->ErrCode != UPNP_E_SUCCESS ) {
                    SampleUtil_Print
                        ( "Error in Event Subscribe Callback -- %d",
                          es_event->ErrCode );
                } else {
                    TvCtrlPointHandleSubscribeUpdate( es_event->
                                                      PublisherUrl,
                                                      es_event->Sid,
                                                      es_event->TimeOut );
                }

                break;
            }

        case UPNP_EVENT_AUTORENEWAL_FAILED:
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
            {
                int TimeOut = default_timeout;
                Upnp_SID newSID;
                int ret;

                struct Upnp_Event_Subscribe *es_event =
                    ( struct Upnp_Event_Subscribe * )Event;

                ret =
                    UpnpSubscribe( ctrlpt_handle, es_event->PublisherUrl,
                                   &TimeOut, newSID );

                if( ret == UPNP_E_SUCCESS ) {
                    SampleUtil_Print( "Subscribed to EventURL with SID=%s",
                                      newSID );
                    TvCtrlPointHandleSubscribeUpdate( es_event->
                                                      PublisherUrl, newSID,
                                                      TimeOut );
                } else {
                    SampleUtil_Print
                        ( "Error Subscribing to EventURL -- %d", ret );
                }
                break;
            }

            /*
               ignore these cases, since this is not a device 
             */
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
        case UPNP_CONTROL_GET_VAR_REQUEST:
        case UPNP_CONTROL_ACTION_REQUEST:
            break;
    }
    //K ithread_mutex_unlock( &DeviceAddMutex );
	//mpDebugPrint("Kevin exit");
    return 0;
}


//
int
NatCtrlPointCallbackEventHandler( Upnp_EventType EventType,
                                 void *Event,
                                 void *Cookie )
{
    //SampleUtil_PrintEvent( EventType, Event );
    switch ( EventType ) {
            /*
               SSDP Stuff 
             */
        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
        case UPNP_DISCOVERY_SEARCH_RESULT:
            {
				int found = 0;
				struct TvDeviceNode *tmpdevnode = NULL;
                struct Upnp_Discovery *d_event =
                    ( struct Upnp_Discovery * )Event;
                IXML_Document *DescDoc = NULL;
                int ret;

				//MP_DEBUG("UPNP_DISCOVERY_SEARCH_RESULT 1 %x",d_event); 
                if( d_event->ErrCode != UPNP_E_SUCCESS ) {
                    SampleUtil_Print( "Error in Discovery Callback -- %d",
                                      d_event->ErrCode );
                }

				if( ( tmpdevnode = GlobalDeviceList ) )
				{
					tmpdevnode = GlobalDeviceList;
					while( tmpdevnode ) {

						if( strcmp( tmpdevnode->device.DescDocURL, d_event->Location ) == 0 ) {
							found = 1;
							break;
						}
						tmpdevnode = tmpdevnode->next;
					}

				}
				if( found )
				{
					//MP_DEBUG("######%s Already exit.",d_event->Location );
					goto foundandexit;
				}

                if( ( ret = UpnpDownloadXmlDoc( d_event->Location,&DescDoc,0 ) ) !=UPNP_E_SUCCESS ) 
				{
					mpDebugPrint("Error obtaining device description "); 
                } else {
					mpDebugPrint("DeviceSearchType %d ",DeviceSearchType);
					if( DeviceSearchType == 0 )
						TvCtrlPointAddDevice( DescDoc, d_event->Location,
											  d_event->Expires );
					else 
						NatCtrlPointAddDevice( DescDoc, d_event->Location,
											  d_event->Expires );

                }
foundandexit:
                if( DescDoc )
				{
                    ixmlDocument_free( DescDoc );
				}
				//printixmlpool();
                //TvCtrlPointPrintList(  );
                break;
            }

            /*
               SOAP Stuff 
             */
        case UPNP_CONTROL_ACTION_COMPLETE:
            {
                struct Upnp_Action_Complete *a_event =
                    ( struct Upnp_Action_Complete * )Event;

				char *externipaddress;
				struct NatReturnData *pnatretdata;
				pnatretdata = (struct NatReturnData *) (Cookie);
				//static unsigned char Connect_to_Server_Fail_count = 0;

                if( a_event->ErrCode != UPNP_E_SUCCESS ) {
					UpnpReturnCode = a_event->ErrCode;
                    SampleUtil_Print
                        ( "Error in  Action Complete Callback -- %d",
                          a_event->ErrCode );
					if( a_event->ErrCode == 714 )
					{
						mpDebugPrint("NoSuchEntryInArray");
					}
					else
					{
						mpDebugPrint("UPNP Error Code %d",a_event->ErrCode);
					}
					return a_event->ErrCode;
					//break;
                }
				UpnpReturnCode = 0;
				//Connect_to_Server_Fail_count = 0;
                /*g
                   No need for any processing here, just print out results.  Service state
                   table updates are handled by events. 
                 */
				if( UPNPNATCOMMAND == 0 )
				{
					externipaddress = SampleUtil_GetFirstDocumentItem(a_event->ActionResult,"NewExternalIPAddress");
					pnatretdata->ExternalIP = externipaddress;
					if( externipaddress )
					{
						mpDebugPrint("externipaddress %s",externipaddress);
						//ixml_mem_free(externipaddress);
					}
				}
				else if( UPNPNATCOMMAND == 1 )
				{
					pnatretdata->PortMappingDescription = SampleUtil_GetFirstDocumentItem(a_event->ActionResult,"NewPortMappingDescription");
					pnatretdata->InternalClient = SampleUtil_GetFirstDocumentItem(a_event->ActionResult,"NewInternalClient");
					mpDebugPrint("pnatretdata->PortMappingDescription %s",pnatretdata->PortMappingDescription);
					mpDebugPrint("pnatretdata->InternalClient %s",pnatretdata->InternalClient);
				}
				break;
            }

        case UPNP_EVENT_AUTORENEWAL_FAILED:
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
            {
                int TimeOut = default_timeout;
                Upnp_SID newSID;
                int ret;

                struct Upnp_Event_Subscribe *es_event =
                    ( struct Upnp_Event_Subscribe * )Event;

                ret =
                    UpnpSubscribe( ctrlpt_handle, es_event->PublisherUrl,
                                   &TimeOut, newSID );

                if( ret == UPNP_E_SUCCESS ) {
                    SampleUtil_Print( "Subscribed to EventURL with SID=%s",
                                      newSID );
                    TvCtrlPointHandleSubscribeUpdate( es_event->
                                                      PublisherUrl, newSID,
                                                      TimeOut );
                } else {
                    SampleUtil_Print
                        ( "Error Subscribing to EventURL -- %d", ret );
                }
                break;
            }

            /*
               ignore these cases, since this is not a device 
             */
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
        case UPNP_CONTROL_GET_VAR_REQUEST:
        case UPNP_CONTROL_ACTION_REQUEST:
            break;
    }
    //K ithread_mutex_unlock( &DeviceAddMutex );
	//mpDebugPrint("Kevin exit");
    return 0;
}

//
/********************************************************************************
 * TvCtrlPointVerifyTimeouts
 *
 * Description: 
 *       Checks the advertisement  each device
 *        in the global device list.  If an advertisement expires,
 *       the device is removed from the list.  If an advertisement is about to
 *       expire, a search request is sent for that device.  
 *
 * Parameters:
 *    incr -- The increment to subtract from the timeouts each time the
 *            function is called.
 *
 ********************************************************************************/
void
TvCtrlPointVerifyTimeouts( int incr )
{
    struct TvDeviceNode *prevdevnode,
     *curdevnode;
    int ret;

    //K ithread_mutex_lock( &DeviceListMutex );

    prevdevnode = NULL;
    curdevnode = GlobalDeviceList;
	//Frequence call this function???
	MP_DEBUG("TvCtrlPointVerifyTimeouts");
    while( curdevnode ) {
        curdevnode->device.AdvrTimeOut -= incr;
        //SampleUtil_Print("Advertisement Timeout: %d\n", curdevnode->device.AdvrTimeOut);

        if( curdevnode->device.AdvrTimeOut <= 0 ) {
            /*
               This advertisement has expired, so we should remove the device
               from the list 
             */

            if( GlobalDeviceList == curdevnode )
                GlobalDeviceList = curdevnode->next;
            else
                prevdevnode->next = curdevnode->next;
            TvCtrlPointDeleteNode( curdevnode );
            if( prevdevnode )
                curdevnode = prevdevnode->next;
            else
                curdevnode = GlobalDeviceList;
        } else {

            if( curdevnode->device.AdvrTimeOut < 2 * incr ) {
                /*
                   This advertisement is about to expire, so send
                   out a search request for this device UDN to 
                   try to renew 
                 */
                ret = UpnpSearchAsync( ctrlpt_handle, incr,
                                       curdevnode->device.UDN, NULL );
                if( ret != UPNP_E_SUCCESS )
                    SampleUtil_Print
                        ( "Error sending search request for Device UDN: %s -- err = %d",
                          curdevnode->device.UDN, ret );
            }

            prevdevnode = curdevnode;
            curdevnode = curdevnode->next;
        }

    }
    //K ithread_mutex_unlock( &DeviceListMutex );

}

/********************************************************************************
 * TvCtrlPointTimerLoop
 *
 * Description: 
 *       Function that runs in its own thread and monitors advertisement
 *       and subscription timeouts for devices in the global device list.
 *
 * Parameters:
 *    None
 *
 ********************************************************************************/
void *
TvCtrlPointTimerLoop( void *args )
{
    int incr = 30;              // how often to verify the timeouts, in seconds

	MP_DEBUG("TvCtrlPointTimerLoop\n");
    while( 1 ) {
        //isleep( incr );
        TvCtrlPointVerifyTimeouts( incr );
    }

    return NULL;
}

/********************************************************************************
 * TvCtrlPointStart
 *
 * Description: 
 *		Call this function to initialize the UPnP library and start the TV Control
 *		Point.  This function creates a timer thread and provides a callback
 *		handler to process any UPnP events that are received.
 *
 * Parameters:
 *		None
 *
 * Returns:
 *		TV_SUCCESS if everything went well, else TV_ERROR
 *
 ********************************************************************************/
int
TvCtrlPointStart( print_string printFunctionPtr,
                  state_update updateFunctionPtr )
{
    //K ithread_t timer_thread;
    int rc;
    short int port = 0;
    char *ip_address = NULL;

    SampleUtil_Initialize( printFunctionPtr );
    SampleUtil_RegisterUpdateFunction( updateFunctionPtr );

    //K ithread_mutex_init( &DeviceListMutex, 0 );

    //K ithread_mutex_init( &DeviceAddMutex, 0 );
	//MP_DEBUG("Intializing UPnP with ipaddress");
	MP_DEBUG("Intializing UPnP with ipaddress"); 
    //SampleUtil_Print( "Intializing UPnP with ipaddress=%s port=%d",
                      //ip_address, port );
    rc = UpnpInit( ip_address, port );
/*
    if( UPNP_E_SUCCESS != rc ) {
        //SampleUtil_Print( "WinCEStart: UpnpInit() Error: %d", rc );
		mpDebugPrint("WinCEStart: UpnpInit() Error: %d", rc); 
        UpnpFinish(  );
        return TV_ERROR;
    }
*/
    if( NULL == ip_address )
        ip_address = UpnpGetServerIpAddress(  );
    if( 0 == port )
        port = UpnpGetServerPort(  );

    //SampleUtil_Print( "UPnP Initialized (%s:%d)", ip_address, port );
	MP_DEBUG2("UPnP Initialized (%s:%d)", ip_address, port); 

    //SampleUtil_Print( "Registering Control Point" );
	MP_DEBUG("Registering Control Point" ); 
	if( DeviceSearchType == 0 )
		rc = UpnpRegisterClient( TvCtrlPointCallbackEventHandler,
                             &ctrlpt_handle, &ctrlpt_handle );
	else
		rc = UpnpRegisterClient( NatCtrlPointCallbackEventHandler,
                             &ctrlpt_handle, &ctrlpt_handle );

    if( UPNP_E_SUCCESS != rc ) {
        //SampleUtil_Print( "Error registering CP: %d", rc );
		MP_DEBUG1("Error registering CP: %d", rc); 
        UpnpFinish(  );
        return TV_ERROR;
    }

    //SampleUtil_Print( "Control Point Registered" );
	MP_DEBUG("TvCtrlPointRefresh"); 
    TvCtrlPointRefresh(  );
	MP_DEBUG("ithread_create"); 
    // start a timer thread
    //ithread_create( &timer_thread, NULL, TvCtrlPointTimerLoop, NULL );
	MP_DEBUG("TvCtrlPointStart END"); 
    return TV_SUCCESS;
}

int
TvCtrlPointStop( void )
{
    TvCtrlPointRemoveAll(  );
	//mpDebugPrint("call UpnpUnRegisterClient");
    UpnpUnRegisterClient( ctrlpt_handle );
	//mpDebugPrint("call UpnpFinish");
    UpnpFinish(  );
	//mpDebugPrint("SampleUtil_Finish");
    SampleUtil_Finish(  );

    return TV_SUCCESS;
}

/*
extern upnp_map_t *upnp_s_map;

void Upnp_action(void)
{
	DWORD dwEvent;
	mpDebugPrint("Upnp_action========>");
wait_action:
	EventWait(UPNP_START_EVENT, 0x0000001000, OS_EVENT_OR, &dwEvent);
	mpDebugPrint("dwEvent %d %p",dwEvent,upnp_s_map);
	Upnp_Browse_root_dir(upnp_s_map);
	EventSet(UPNP_START_EVENT, 0x00002000);
	goto wait_action;
}
*/
void Upnp_Browse_root_dir(upnp_map_t *s_map)
{
	//unsigned char gupnp_index;
	unsigned char kconut = 0;
	//mpDebugPrint("gupnp_index %d",gupnp_index);
	bAccess_Deny = 0;
	mpDebugPrint("Upnp_Browse_root_dir %s ",s_map->base_dir);
	strcpy(upnpcurdir,s_map->base_dir);
	mpDebugPrint("Upnp_Browse_root_dir upnpcurdir %s ",upnpcurdir);
	do
	{
		TvCtrlPointSendAction( 0, gupnp_index+1, "Browse",NULL, NULL, 0 ,NULL);	
		TaskYield();
		UartOutText("Send ");
		kconut++;
		if( kconut > 3 )
			break;		
	}while(bAccess_Deny);

}

int Upnp_Browse_dir(char *con_id,unsigned short index,netfs_meta_entry_t *pmeta_entry)
{
	char *tmp[2];
	int rc = 0;

	gdir_child_total_count = 0;
	//gdir_child_cur_count = 0;
	tmp[0] = con_id;
	tmp[1] = "10";
	//do
	{
		gdir_child_cur_count = index;
		rc = TvCtrlPointSendAction( 0, gupnp_index+1, "Browse",tmp, NULL, 1 ,pmeta_entry);	
		//mpDebugPrint("#### %d %d",gdir_child_cur_count,gdir_child_total_count);
	}
	if( rc != 0 )
		mpDebugPrint("rc = %d",rc);
	mpDebugPrint("End Upnp_Browse_dir");
	return rc;
	//while( gdir_child_cur_count < gdir_child_total_count);
}

int Test_UPNP_Server()
{
	int ret;
    int TimeOut[MS_SERVICE_SERVCOUNT] =
        { default_timeout, default_timeout };
	struct TvDeviceNode *devnode;
	int rc;
	mpDebugPrint("TvCtrlPointGetDevice");
	rc = TvCtrlPointGetDevice( gupnp_index+1, &devnode );
	mpDebugPrint("TvCtrlPointGetDevice rc %d",rc);
	if( TV_SUCCESS == rc )
	{
		mpDebugPrint("UpnpSubscribe");
		ret =
			UpnpSubscribe( ctrlpt_handle, devnode->device.TvService[0].EventURL,
						   &TimeOut[0],
						   devnode->device.TvService[0].SID );
		mpDebugPrint("ret %d",ret);
		return ret;
	}
}

int Check_UPNP_Server()
{
	int ret;
    int TimeOut[MS_SERVICE_SERVCOUNT] =
        { default_timeout, default_timeout };
	struct TvDeviceNode *devnode;
	int rc;
	mpDebugPrint("TvCtrlPointGetDevice");
	rc = TvCtrlPointGetDevice( gupnp_index+1, &devnode );
	mpDebugPrint("TvCtrlPointGetDevice rc %d",rc);
	if( TV_SUCCESS == rc )
	{
		mpDebugPrint("UpnpSubscribe");
		ret =
			UpnpSubscribe( ctrlpt_handle, devnode->device.TvService[0].EventURL,
						   &TimeOut[0],
						   devnode->device.TvService[0].SID );
		mpDebugPrint("ret %d",ret);
		if( ret != UPNP_E_SUCCESS ) 	//Server has been shutdown.
			TvCtrlPointRemoveDevice(devnode->device.UDN);
		return ret;
	}
	else
	{
		if( g_bAniFlag & ANI_SLIDE )
			xpgCb_SlideExit();
		return -1;
	}

}

