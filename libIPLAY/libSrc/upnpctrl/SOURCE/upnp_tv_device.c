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
#define LOCAL_DEBUG_ENABLE 1
#include "upnp_tv_device.h"
#include "..\..\..\..\libIPLAY\libSrc\lwip\INCLUDE\net_sys.h"
#include "taskid.h"
#define DEFAULT_WEB_DIR "./web"

#define DESC_URL_SIZE 200

extern Net_App_State App_State;
extern unsigned char UPNP_Server_Event;

extern char gupnpfilename[16];
extern BYTE g_bXpgStatus;

#if 1//DLNA_1_5
char *gfriendlyname=NULL;
char bchange = 0;
char *CurrentActionName;
char bPlay = 0;
char gMute = 0;

#endif

/*
   Device type for tv device 
 */
//char TvDeviceType[] = "urn:schemas-upnp-org:device:tvdevice:1";
char MsDeviceType[] = "urn:schemas-upnp-org:device:MediaServer:1";
/*
   Service types for tv services
 */
/*
char *TvServiceType[] = 
{ 
	"urn:schemas-upnp-org:service:tvcontrol:1",
    "urn:schemas-upnp-org:service:tvpicture:1"
};
*/
char *MsServiceType[] = 
{
    "urn:schemas-upnp-org:service:ContentDirectory:1",
	"urn:schemas-upnp-org:service:ConnectionManager:1"
};

char *DmrServiceType[] = 
{
    "urn:schemas-upnp-org:service:ConnectionManager:1",
	"urn:schemas-upnp-org:service:AVTransport:1",
	"urn:schemas-upnp-org:service:RenderingControl:1"
};

/*
   Global arrays for storing Tv Control Service
   variable names, values, and defaults 
 */
/*
char *tvc_varname[] = { "Power", "Channel", "Volume" };
char tvc_varval[TV_CONTROL_VARCOUNT][TV_MAX_VAL_LEN];
char *tvc_varval_def[] = { "1", "1", "5" };
*/
char *dmr_rc_varrname[] = { "Mute", "Volume" };
char dmr_rc_varval[DMR_RC_VARCOUNT][DMR_RC_MAX_VAL_LEN];
char *dmr_rc_varval_def[] = { "0", "6" };

//Kevin ADD
char *mscd_varname[] = { "Browse","CreateObject"};
//char mscd_varval[TV_CONTROL_VARCOUNT][TV_MAX_VAL_LEN];
//char *tvc_varval_def[] = { "1", "1", "5" };



/*
   The amount of time (in seconds) before advertisements
   will expire 
 */
int default_advr_expire = 100;

/*
   Global structure for storing the state table for this device 
 */
struct TvService tv_service_table[2];

struct TvService *dmr_service_table;
/*
   Device handle supplied by UPnP SDK 
 */
UpnpDevice_Handle device_handle = -1;

/*
   Mutex for protecting the global state table data
   in a multi-threaded, asynchronous environment.
   All functions should lock this mutex before reading
   or writing the state table data. 
 */
//ithread_mutex_t TVDevMutex;

//Color constants
#define MAX_COLOR 10
#define MIN_COLOR 1

//Brightness constants
#define MAX_BRIGHTNESS 10
#define MIN_BRIGHTNESS 1

//Power constants
#define POWER_ON 1
#define POWER_OFF 0

//Tint constants
#define MAX_TINT 10
#define MIN_TINT 1

//Volume constants
#define MAX_VOLUME 10
#define MIN_VOLUME 1

//Contrast constants
#define MAX_CONTRAST 10
#define MIN_CONTRAST 1

//Channel constants
#define MAX_CHANNEL 100
#define MIN_CHANNEL 1

//Kevin ADD
int
MsBrowseAction( IN IXML_Document * in,
                    OUT IXML_Document ** out,
                    OUT char **errorString );
int
MsCreateObjectAction( IN IXML_Document * in,
                    OUT IXML_Document ** out,
                    OUT char **errorString );

int DMRCM_GetProtocolInfo( IXML_Document * in, IXML_Document ** out, char **errorString );
static int DMRAVT_Play_Stop( IXML_Document * in, IXML_Document ** out, char **errorString );
int DMRAVT_GetTransportInfo( IXML_Document * in, IXML_Document ** out, char **errorString );
int DMRAVT_GetPositionInfo( IXML_Document * in, IXML_Document ** out, char **errorString );
int DMRAVT_SetAVTransportURI( IXML_Document * in, IXML_Document ** out, char **errorString );
int DMRRC_GetMute( IXML_Document * in, IXML_Document ** out, char **errorString );
int DMRRC_SetMute( IXML_Document * in, IXML_Document ** out, char **errorString );
int DMRRC_GetVolume( IXML_Document * in, IXML_Document ** out, char **errorString );
int DMRRC_SetVolume( IXML_Document * in, IXML_Document ** out, char **errorString );
char *gavtransuri;
unsigned char bdlnadownload = 0;

/******************************************************************************
 * SetServiceTable
 *
 * Description: 
 *       Initializes the service table for the specified service.
 *       Note that 
 *       knowledge of the service description is
 *       assumed. 
 * Parameters:
 *   int serviceType - one of TV_SERVICE_CONTROL or, TV_SERVICE_PICTURE
 *   const char * UDN - UDN of device containing service
 *   const char * serviceId - serviceId of service
 *   const char * serviceTypeS - service type (as specified in Description
 *                                             Document) 
 *   struct TvService *out - service containing table to be set.
 *
 *****************************************************************************/
int
SetServiceTable( IN int serviceType,
                 IN const char *UDN,
                 IN const char *serviceId,
                 IN const char *serviceTypeS,
                 INOUT struct TvService *out )
{
    unsigned int i = 0;

    strcpy( out->UDN, UDN );
    strcpy( out->ServiceId, serviceId );
    strcpy( out->ServiceType, serviceTypeS );

    switch ( serviceType ) {
/*
        case MS_SERVICE_CONTENTDIRECTORY:
            out->VariableCount = MS_SERVICE_CONTENTDIRECTORY_VAR_COUNT;
			mpDebugPrint("tv_service_table[MS_SERVICE_CONTENTDIRECTORY].VariableCount %d",tv_service_table[MS_SERVICE_CONTENTDIRECTORY].VariableCount);
            for( i = 0;
                 i < tv_service_table[MS_SERVICE_CONTENTDIRECTORY].VariableCount;
                 i++ ) 
			{
                tv_service_table[MS_SERVICE_CONTENTDIRECTORY].VariableName[i]
                    = mscd_varname[i];
                //tv_service_table[TV_SERVICE_CONTROL].VariableStrVal[i]
                  //  = 0;
                //strcpy( tv_service_table[TV_SERVICE_CONTROL].
                  //      VariableStrVal[i], tvc_varval_def[i] );
            }

            break;
*/
		case DMR_SERVICE_AVTRANSPORT:
			//mpDebugPrint("DMR_SERVICE_AVTRANSPORT");
			out->VariableCount = 0;//DMR_SERVICE_CONNECTIONMANAGERMENT_VAR_COUNT;
			break;
		case DMR_SERVICE_RENDERINGCONTROL:
			mpDebugPrint("DMR_SERVICE_RENDERINGCONTROL");
			out->VariableCount = DMR_RC_VARCOUNT;
			for( i = 0 ; i < out->VariableCount; i++ ) 
			{
				dmr_service_table[2].VariableName[i] = dmr_rc_varrname[i];
				dmr_service_table[2].VariableStrVal[i] = dmr_rc_varval[i];
				strcpy( dmr_service_table[2].VariableStrVal[i], dmr_rc_varval_def[i] );
			}
			break;

#if 0
        case TV_SERVICE_PICTURE:
            out->VariableCount = TV_PICTURE_VARCOUNT;

            for( i = 0;
                 i < tv_service_table[TV_SERVICE_PICTURE].VariableCount;
                 i++ ) {
                tv_service_table[TV_SERVICE_PICTURE].VariableName[i] =
                    tvp_varname[i];
                tv_service_table[TV_SERVICE_PICTURE].VariableStrVal[i] =
                    tvp_varval[i];
                strcpy( tv_service_table[TV_SERVICE_PICTURE].
                        VariableStrVal[i], tvp_varval_def[i] );
            }

            break;
        default:
            //assert( 0 );
			mpDebugPrint("UDN %s##",UDN);
			break;
#endif
    }

    return SetActionTable( serviceType, out );

}

/******************************************************************************
 * SetActionTable
 *
 * Description: 
 *       Initializes the action table for the specified service.
 *       Note that 
 *       knowledge of the service description is
 *       assumed.  Action names are hardcoded.
 * Parameters:
 *   int serviceType - one of TV_SERVICE_CONTROL or, TV_SERVICE_PICTURE
 *   struct TvService *out - service containing action table to set.
 *
 *****************************************************************************/
int
SetActionTable( IN int serviceType,
                INOUT struct TvService *out )
{
#if	0//HAVE_UPNP_MEDIA_SERVER
	if( serviceType ==  MS_SERVICE_CD )
	{
		mpDebugPrint("serviceType ==  MS_SERVICE_CD");
        out->ActionNames[0] = "Browse";
        out->actions[0] = MsBrowseAction;
        out->ActionNames[1] = "CreateObject";
        out->actions[1] = MsCreateObjectAction;
        out->ActionNames[2] = NULL;
        return 1;
	}
	else if( serviceType == MS_SERVICE_CM )
	{
		mpDebugPrint("serviceType == MS_SERVICE_CM");
        out->ActionNames[0] = "GetCurrentConnectionIDs";
        out->actions[0] = TvDeviceSetColor;
        out->ActionNames[1] = NULL;
        return 1;
	}
#else	
    out->ActionNames[0] = NULL;
    out->actions[0] = NULL;
#endif
	if( serviceType == DMR_SERVICE_CONNECTIONMANAGERMENT )
	{
		//mpDebugPrint("serviceType == DMR_SERVICE_CONNECTIONMANAGERMENT ");
		//ptr = ixml_mem_malloc(32);
		//strcpy(ptr,"GetProtocolInfo");
        out->ActionNames[0] = "GetProtocolInfo";
        out->actions[0] = DMRCM_GetProtocolInfo;
        //out->ActionNames[1] = "PrepareForConnection";
        //out->actions[1] = DMRCM_PrepareForConnection;
        out->ActionNames[1] = NULL;
        return 1;

	}
	else if( serviceType == DMR_SERVICE_AVTRANSPORT )
	{
		mpDebugPrint("serviceType == DMR_SERVICE_AVTRANSPORT ");
		//ptr = ixml_mem_malloc(32);
		//strcpy(ptr,"GetProtocolInfo");
        out->ActionNames[0] = "Stop";
        out->actions[0] = DMRAVT_Play_Stop;
        out->ActionNames[1] = "GetTransportInfo";
        out->actions[1] = DMRAVT_GetTransportInfo;
        out->ActionNames[2] = "GetPositionInfo";
        out->actions[2] = DMRAVT_GetPositionInfo;
        out->ActionNames[3] = "SetAVTransportURI";
        out->actions[3] = DMRAVT_SetAVTransportURI;
        out->ActionNames[4] = "Play";
        out->actions[4] = DMRAVT_Play_Stop;
        out->ActionNames[5] = "Pause";
        out->actions[5] = DMRAVT_Play_Stop;
		out->ActionNames[6] = NULL;
        return 1;

	}
	else if( serviceType == DMR_SERVICE_RENDERINGCONTROL )
	{
        out->ActionNames[0] = "GetVolume";
        out->actions[0] = DMRRC_GetVolume;
        out->ActionNames[1] = "SetVolume";
        out->actions[1] = DMRRC_SetVolume;
        out->ActionNames[2] = "GetMute";
        out->actions[2] = DMRRC_GetMute;
        out->ActionNames[3] = "SetMute";
        out->actions[3] = DMRRC_SetMute;
        out->ActionNames[4] = NULL;

        return 1;
	}


/*
    if( serviceType == TV_SERVICE_CONTROL ) {
        out->ActionNames[0] = "PowerOn";
        out->actions[0] = TvDevicePowerOn;
        out->ActionNames[1] = "PowerOff";
        out->actions[1] = TvDevicePowerOff;
        out->ActionNames[2] = "SetChannel";
        out->actions[2] = TvDeviceSetChannel;
        out->ActionNames[3] = "IncreaseChannel";
        out->actions[3] = TvDeviceIncreaseChannel;
        out->ActionNames[4] = "DecreaseChannel";
        out->actions[4] = TvDeviceDecreaseChannel;
        out->ActionNames[5] = "SetVolume";
        out->actions[5] = TvDeviceSetVolume;
        out->ActionNames[6] = "IncreaseVolume";
        out->actions[6] = TvDeviceIncreaseVolume;
        out->ActionNames[7] = "DecreaseVolume";
        out->actions[7] = TvDeviceDecreaseVolume;
        out->ActionNames[8] = NULL;
        return 1;
    } else if( serviceType == TV_SERVICE_PICTURE ) {
        out->ActionNames[0] = "SetColor";
        out->ActionNames[1] = "IncreaseColor";
        out->ActionNames[2] = "DecreaseColor";
        out->actions[0] = TvDeviceSetColor;
        out->actions[1] = TvDeviceIncreaseColor;
        out->actions[2] = TvDeviceDecreaseColor;
        out->ActionNames[3] = "SetTint";
        out->ActionNames[4] = "IncreaseTint";
        out->ActionNames[5] = "DecreaseTint";
        out->actions[3] = TvDeviceSetTint;
        out->actions[4] = TvDeviceIncreaseTint;
        out->actions[5] = TvDeviceDecreaseTint;

        out->ActionNames[6] = "SetBrightness";
        out->ActionNames[7] = "IncreaseBrightness";
        out->ActionNames[8] = "DecreaseBrightness";
        out->actions[6] = TvDeviceSetBrightness;
        out->actions[7] = TvDeviceIncreaseBrightness;
        out->actions[8] = TvDeviceDecreaseBrightness;

        out->ActionNames[9] = "SetContrast";
        out->ActionNames[10] = "IncreaseContrast";
        out->ActionNames[11] = "DecreaseContrast";

        out->actions[9] = TvDeviceSetContrast;
        out->actions[10] = TvDeviceIncreaseContrast;
        out->actions[11] = TvDeviceDecreaseContrast;
        return 1;
    }
*/
    return 0;

}

/******************************************************************************
 * TvDeviceStateTableInit
 *
 * Description: 
 *       Initialize the device state table for 
 * 	 this TvDevice, pulling identifier info
 *       from the description Document.  Note that 
 *       knowledge of the service description is
 *       assumed.  State table variables and default
 *       values are currently hardcoded in this file
 *       rather than being read from service description
 *       documents.
 *
 * Parameters:
 *   DescDocURL -- The description document URL
 *
 *****************************************************************************/
int
TvDeviceStateTableInit( IN char *DescDocURL )
{
    IXML_Document *DescDoc = NULL;
    int ret = UPNP_E_SUCCESS;
    char *servid_ctrl = NULL,
     *evnturl_ctrl = NULL,
     *ctrlurl_ctrl = NULL;
    char *servid_pict = NULL,
     *evnturl_pict = NULL,
     *ctrlurl_pict = NULL;
    char *udn = NULL;

    //Download description document
    if( UpnpDownloadXmlDoc( DescDocURL, &DescDoc,1 ) != UPNP_E_SUCCESS ) {
        SampleUtil_Print( "TvDeviceStateTableInit -- Error Parsing %s\n",
                          DescDocURL );
        ret = UPNP_E_INVALID_DESC;
        goto error_handler;
    }
	//mpDebugPrint("AAA5");
    udn = SampleUtil_GetFirstDocumentItem( DescDoc, "UDN" );

    /*
       Find the Tv Control Service identifiers 
     */
	//mpDebugPrint("Find the Tv Control Service identifiers");
    if( !SampleUtil_FindAndParseService( DescDoc, DescDocURL,
                                         /*TvServiceType[TV_SERVICE_CONTROL],*/
										 MsServiceType[MS_SERVICE_CD],
                                         &servid_ctrl, &evnturl_ctrl,
                                         &ctrlurl_ctrl ) ) {

        ret = UPNP_E_INVALID_DESC;
        goto error_handler;
    }
	//mpDebugPrint("AAA5-2");
	SampleUtil_Print( "MsDeviceStateTableInit -- Found:"
                          " Service: %s\n",
                          MsServiceType[MS_SERVICE_CD] );
	
    //set control service table
    //SetServiceTable( TV_SERVICE_CONTROL, udn, servid_ctrl,
                     //TvServiceType[TV_SERVICE_CONTROL],
                     //&tv_service_table[TV_SERVICE_CONTROL] );
	//mpDebugPrint("AAA5-3");
    SetServiceTable( MS_SERVICE_CONTENTDIRECTORY, udn, servid_ctrl,
                     MsServiceType[MS_SERVICE_CD],
                     //&tv_service_table[TV_SERVICE_CONTROL] );
                     &tv_service_table[MS_SERVICE_CD] );

#if 0
    /*
       Find the Tv Picture Service identifiers 
     */
    if( !SampleUtil_FindAndParseService( DescDoc, DescDocURL,
                                         /*TvServiceType[TV_SERVICE_PICTURE],*/
										 MsServiceType[MS_SERVICE_CM],
                                         &servid_pict, &evnturl_pict,
                                         &ctrlurl_pict ) ) {

        ret = UPNP_E_INVALID_DESC;
        goto error_handler;
    }
    //set picture service table
    SetServiceTable( TV_SERVICE_PICTURE, udn, servid_pict,
                     TvServiceType[MS_SERVICE_CM],
                     //&tv_service_table[TV_SERVICE_PICTURE] );
					&tv_service_table[MS_SERVICE_CM] );
#endif
  error_handler:

    //clean up
    if( udn )
        ixml_mem_free( udn );
    if( servid_ctrl )
        ixml_mem_free( servid_ctrl );
    if( evnturl_ctrl )
        ixml_mem_free( evnturl_ctrl );
    if( ctrlurl_ctrl )
        ixml_mem_free( ctrlurl_ctrl );
    if( servid_pict )
        ixml_mem_free( servid_pict );
    if( evnturl_pict )
        ixml_mem_free( evnturl_pict );
    if( ctrlurl_pict )
        ixml_mem_free( ctrlurl_pict );
    if( DescDoc )
        ixmlDocument_free( DescDoc );

    return ( ret );
}

int DmrDeviceStateTableInit( IN char *DescDocURL )
{
    IXML_Document *DescDoc = NULL;
    int ret = UPNP_E_SUCCESS;
    char *servid_ctrl = NULL,
     *evnturl_ctrl = NULL,
     *ctrlurl_ctrl = NULL;
    char *servid_pict = NULL,
     *evnturl_pict = NULL,
     *ctrlurl_pict = NULL;
    char *udn = NULL;
	int i;
    //Download description document
    if( UpnpDownloadXmlDoc( DescDocURL, &DescDoc,1 ) != UPNP_E_SUCCESS ) {
        SampleUtil_Print( "TvDeviceStateTableInit -- Error Parsing %s\n",
                          DescDocURL );
        ret = UPNP_E_INVALID_DESC;
        goto error_handler;
    }

    udn = SampleUtil_GetFirstDocumentItem( DescDoc, "UDN" );
    gfriendlyname = SampleUtil_GetFirstDocumentItem( DescDoc, "friendlyName" );
	//mpDebugPrint("udn %s",udn);
    /*
       Find the Tv Control Service identifiers 
     */
	dmr_service_table = ixml_mem_malloc(16*1024);
	for( i =0 ; i < 3 ; i++ )
	{
		//mpDebugPrint("Find the Tv Control Service identifiers");
		if( !SampleUtil_FindAndParseService( DescDoc, DescDocURL,
											 DmrServiceType[i],
											 &servid_ctrl, &evnturl_ctrl,
											 &ctrlurl_ctrl ) ) {

			ret = UPNP_E_INVALID_DESC;
			goto error_handler;
		}
		SampleUtil_Print( "DmrDeviceStateTableInit -- Found:"
							  " Service: %s\n",
							  DmrServiceType[0] );

		//set control service table
		SetServiceTable( DMR_SERVICE_CONNECTIONMANAGERMENT+i, udn, servid_ctrl,
						 DmrServiceType[i],
						 &dmr_service_table[i] );
		if( servid_ctrl )
			ixml_mem_free( servid_ctrl );
		if( evnturl_ctrl )
			ixml_mem_free( evnturl_ctrl );
		if( ctrlurl_ctrl )
			ixml_mem_free( ctrlurl_ctrl );

		servid_ctrl = evnturl_ctrl = ctrlurl_ctrl = NULL;
	}
#if 0
	mpDebugPrint("HHH %s",dmr_service_table[0].ActionNames[0]);
	mpDebugPrint("HHH %x",dmr_service_table[0].actions[0]);
	mpDebugPrint("HHH %s",dmr_service_table[1].ActionNames[0]);
	mpDebugPrint("HHH %x",dmr_service_table[1].actions[0]);
	mpDebugPrint("HHH %s",dmr_service_table[2].ActionNames[0]);
	mpDebugPrint("HHH %x",dmr_service_table[2].actions[0]);
#endif
  error_handler:

    //clean up
    if( udn )
        ixml_mem_free( udn );
    if( servid_ctrl )
        ixml_mem_free( servid_ctrl );
    if( evnturl_ctrl )
        ixml_mem_free( evnturl_ctrl );
    if( ctrlurl_ctrl )
        ixml_mem_free( ctrlurl_ctrl );
    if( servid_pict )
        ixml_mem_free( servid_pict );
    if( evnturl_pict )
        ixml_mem_free( evnturl_pict );
    if( ctrlurl_pict )
        ixml_mem_free( ctrlurl_pict );
    if( DescDoc )
        ixmlDocument_free( DescDoc );

    return ( ret );
}

/******************************************************************************
 * TvDeviceHandleSubscriptionRequest
 *
 * Description: 
 *       Called during a subscription request callback.  If the
 *       subscription request is for this device and either its
 *       control service or picture service, then accept it.
 *
 * Parameters:
 *   sr_event -- The subscription request event structure
 *
 *****************************************************************************/
int
TvDeviceHandleSubscriptionRequest( IN struct Upnp_Subscription_Request
                                   *sr_event )
{
    unsigned int i = 0;         //,j=0;

    // IXML_Document *PropSet=NULL;

    //lock state mutex
    //KK ithread_mutex_lock( &TVDevMutex );
	mpDebugPrint("TvDeviceHandleSubscriptionRequest###########");
	mpDebugPrint("%s",sr_event->UDN);
	mpDebugPrint("%s",tv_service_table[i].UDN);
#if 0
    for( i = 0; i < 3; i++ ) 
	{
		//mpDebugPrint("%s",sr_event->ServiceId);
		//mpDebugPrint("%s",tv_service_table[i].ServiceId);
        if( ( strcmp( sr_event->UDN, dmr_service_table[i].UDN ) == 0 ) &&
            ( strcmp( sr_event->ServiceId, dmr_service_table[i].ServiceId )
              == 0 ) ) {

            /*
               PropSet = NULL;

               for (j=0; j< tv_service_table[i].VariableCount; j++)
               {
               //add each variable to the property set
               //for initial state dump
               UpnpAddToPropertySet(&PropSet, 
               tv_service_table[i].VariableName[j],
               tv_service_table[i].VariableStrVal[j]);
               }

               //dump initial state 
               UpnpAcceptSubscriptionExt(device_handle, sr_event->UDN, 
               sr_event->ServiceId,
               PropSet,sr_event->Sid);
               //free document
               Document_free(PropSet);

             */
			mpDebugPrint("UpnpAcceptSubscription###########");
            UpnpAcceptSubscription( device_handle,
                                    sr_event->UDN,
                                    sr_event->ServiceId,
                                    ( const char ** )dmr_service_table[i].
                                    VariableName,
                                    ( const char ** )dmr_service_table[i].
                                    VariableStrVal,
                                    dmr_service_table[i].VariableCount,
                                    sr_event->Sid );

        }
    }
#endif
#if 0
    for( i = 0; i < MS_SERVICE_SERVCOUNT; i++ ) 
	{
		mpDebugPrint("%s",sr_event->ServiceId);
		mpDebugPrint("%s",tv_service_table[i].ServiceId);
        if( ( strcmp( sr_event->UDN, tv_service_table[i].UDN ) == 0 ) &&
            ( strcmp( sr_event->ServiceId, tv_service_table[i].ServiceId )
              == 0 ) ) {

            /*
               PropSet = NULL;

               for (j=0; j< tv_service_table[i].VariableCount; j++)
               {
               //add each variable to the property set
               //for initial state dump
               UpnpAddToPropertySet(&PropSet, 
               tv_service_table[i].VariableName[j],
               tv_service_table[i].VariableStrVal[j]);
               }

               //dump initial state 
               UpnpAcceptSubscriptionExt(device_handle, sr_event->UDN, 
               sr_event->ServiceId,
               PropSet,sr_event->Sid);
               //free document
               Document_free(PropSet);

             */
			mpDebugPrint("UpnpAcceptSubscription###########");
            UpnpAcceptSubscription( device_handle,
                                    sr_event->UDN,
                                    sr_event->ServiceId,
                                    ( const char ** )tv_service_table[i].
                                    VariableName,
                                    ( const char ** )tv_service_table[i].
                                    VariableStrVal,
                                    tv_service_table[i].VariableCount,
                                    sr_event->Sid );

        }
    }
#endif
    //KK ithread_mutex_unlock( &TVDevMutex );

    return ( 1 );
}

/******************************************************************************
 * TvDeviceHandleGetVarRequest
 *
 * Description: 
 *       Called during a get variable request callback.  If the
 *       request is for this device and either its control service
 *       or picture service, then respond with the variable value.
 *
 * Parameters:
 *   cgv_event -- The control get variable request event structure
 *
 *****************************************************************************/
int
TvDeviceHandleGetVarRequest( INOUT struct Upnp_State_Var_Request
                             *cgv_event )
{
    unsigned int i = 0,
      j = 0;
    int getvar_succeeded = 0;

    cgv_event->CurrentVal = NULL;

    //KK ithread_mutex_lock( &TVDevMutex );

    //for( i = 0; i < MS_SERVICE_SERVCOUNT; i++ ) 
	for( i = 0; i < MS_SERVICE_SERVCOUNT; i++ ) 
	{
		mpDebugPrint("check udn and service id");
        //check udn and service id
        if( ( strcmp( cgv_event->DevUDN, tv_service_table[i].UDN ) == 0 )
            && ( strcmp( cgv_event->ServiceID, tv_service_table[i].ServiceId ) == 0 ) 
			) 
		{
            //check variable name
            for( j = 0; j < tv_service_table[i].VariableCount; j++ ) {
                if( strcmp( cgv_event->StateVarName,
                            tv_service_table[i].VariableName[j] ) == 0 ) {
                    getvar_succeeded = 1;
                    cgv_event->CurrentVal =
                        ixmlCloneDOMString( tv_service_table[i].
                                            VariableStrVal[j] );
                    break;
                }
            }
        }
    }

    if( getvar_succeeded ) {
        cgv_event->ErrCode = UPNP_E_SUCCESS;
    } else {
        SampleUtil_Print
            ( "Error in UPNP_CONTROL_GET_VAR_REQUEST callback:\n" );
        SampleUtil_Print( "   Unknown variable name = %s\n",
                          cgv_event->StateVarName );
        cgv_event->ErrCode = 404;
        strcpy( cgv_event->ErrStr, "Invalid Variable" );
    }

    //KK ithread_mutex_unlock( &TVDevMutex );

    return ( cgv_event->ErrCode == UPNP_E_SUCCESS );
}

/******************************************************************************
 * TvDeviceHandleActionRequest
 *
 * Description: 
 *       Called during an action request callback.  If the
 *       request is for this device and either its control service
 *       or picture service, then perform the action and respond.
 *
 * Parameters:
 *   ca_event -- The control action request event structure
 *
 *****************************************************************************/
int
TvDeviceHandleActionRequest( INOUT struct Upnp_Action_Request *ca_event )
{

    /*
       Defaults if action not found 
     */
    int action_found = 0;
    int i = 0;
    int service = -1;
    int retCode = 0;
    char *errorString = NULL;

    ca_event->ErrCode = 0;
    ca_event->ActionResult = NULL;

    if( ( strcmp( ca_event->DevUDN,
                  tv_service_table[TV_SERVICE_CONTROL].UDN ) == 0 ) &&
        ( strcmp
          ( ca_event->ServiceID,
            tv_service_table[TV_SERVICE_CONTROL].ServiceId ) == 0 ) ) {
        /*
           Request for action in the TvDevice Control Service 
         */
        service = TV_SERVICE_CONTROL;
    } else if( ( strcmp( ca_event->DevUDN,
                         tv_service_table[TV_SERVICE_PICTURE].UDN ) == 0 )
               &&
               ( strcmp
                 ( ca_event->ServiceID,
                   tv_service_table[TV_SERVICE_PICTURE].ServiceId ) ==
                 0 ) ) {
        /*
           Request for action in the TvDevice Picture Service 
         */
        service = TV_SERVICE_PICTURE;
    }
    //Find and call appropriate procedure based on action name
    //Each action name has an associated procedure stored in the
    //service table. These are set at initialization.

	//mpDebugPrint("Search Action Name");
    for( i = 0; ( ( i < 2/*TV_MAXACTIONS*/ ) &&
                  ( tv_service_table[service].ActionNames[i] != NULL ) );
         i++ ) {

        if( !strcmp( ca_event->ActionName,
                     tv_service_table[service].ActionNames[i] ) ) {
/*
            if( ( !strcmp( tv_service_table[TV_SERVICE_CONTROL].
                           VariableStrVal[TV_CONTROL_POWER], "1" ) )
                || ( !strcmp( ca_event->ActionName, "PowerOn" ) 
				) ) 
*/
			{
                retCode =
                    tv_service_table[service].actions[i] ( ca_event->
                                                           ActionRequest,
                                                           &ca_event->
                                                           ActionResult,
                                                           &errorString );
            } 
/*
			else {
                errorString = "Power is Off";
                retCode = UPNP_E_INTERNAL_ERROR;
            }
*/
            action_found = 1;
            break;
        }
    }

    if( !action_found ) {
		mpDebugPrint("##################!action_found###############");
        ca_event->ActionResult = NULL;
        strcpy( ca_event->ErrStr, "Invalid Action" );
        ca_event->ErrCode = 401;
    } else {
        if( retCode == UPNP_E_SUCCESS ) {
            ca_event->ErrCode = UPNP_E_SUCCESS;
        } else {
            //copy the error string 
            strcpy( ca_event->ErrStr, errorString );
            switch ( retCode ) {
                case UPNP_E_INVALID_PARAM:
                    {
                        ca_event->ErrCode = 402;
                        break;
                    }
                case UPNP_E_INTERNAL_ERROR:
                default:
                    {
                        ca_event->ErrCode = 501;
                        break;
                    }

            }
        }
    }

    return ( ca_event->ErrCode );
}

int
DmrDeviceHandleActionRequest( INOUT struct Upnp_Action_Request *ca_event )
{

    /*
       Defaults if action not found 
     */
    int action_found = 0;
    int i = 0;
    int service = -1;
    int retCode = 0;
    char *errorString = NULL;
	//unsigned long long k1 =0;
    ca_event->ErrCode = 0;
    ca_event->ActionResult = NULL;

	//k1 = 1;
	//mpDebugPrint("%x %lx",k1,k1);
	//mpDebugPrint("ca_event->DevUDN %s",ca_event->DevUDN);
	//mpDebugPrint("ca_event->DevUDN %s",dmr_service_table[0].UDN);
    if( ( strcmp( ca_event->DevUDN,
                  dmr_service_table[0].UDN ) == 0 ) &&
        ( strcmp
          ( ca_event->ServiceID,
            dmr_service_table[0].ServiceId ) == 0 ) ) {
        /*
           Request for action in the TvDevice Control Service 
         */
        service = 0;
		//mpDebugPrint("AAA");
    } else if( ( strcmp( ca_event->DevUDN,
                         dmr_service_table[1].UDN ) == 0 )
               &&
               ( strcmp
                 ( ca_event->ServiceID,
                   dmr_service_table[1].ServiceId ) ==
                 0 ) ) {
        /*
           Request for action in the TvDevice Picture Service 
         */
        service = 1;
		//mpDebugPrint("BBB");
    } else if( ( strcmp( ca_event->DevUDN,
                         dmr_service_table[2].UDN ) == 0 )
               &&
               ( strcmp
                 ( ca_event->ServiceID,
                   dmr_service_table[2].ServiceId ) ==
                 0 ) ) {
        /*
           Request for action in the TvDevice Picture Service 
         */
        service = 2;
		//mpDebugPrint("BBB");
    }

    //Find and call appropriate procedure based on action name
    //Each action name has an associated procedure stored in the
    //service table. These are set at initialization.

	//mpDebugPrint("i = %d Kevin Search Action Name %s ",service,ca_event->ActionName);
	//mpDebugPrint("LL%s",dmr_service_table[0].ActionNames[0]);
    for( i = 0; ( ( i < 6/*TV_MAXACTIONS*/ ) &&
                  ( dmr_service_table[service].ActionNames[i] != NULL ) );
         i++ ) {
		//mpDebugPrint("GG%s",dmr_service_table[service].ActionNames[i]);
        if( !strcmp( ca_event->ActionName,
                     dmr_service_table[service].ActionNames[i] ) ) 
		{
			//mpDebugPrint("KKK %x",dmr_service_table[1].actions[0]);
			//mpDebugPrint("KKK %x",dmr_service_table[service].actions[i]);
			{

				CurrentActionName = ca_event->ActionName;
                retCode =
                    dmr_service_table[service].actions[i] ( ca_event->
                                                           ActionRequest,
                                                           &ca_event->
                                                           ActionResult,
                                                           &errorString );
            } 
			//mpDebugPrint("JJ");
/*
			else {
                errorString = "Power is Off";
                retCode = UPNP_E_INTERNAL_ERROR;
            }
*/
            action_found = 1;
            break;
        }
		//mpDebugPrint("OO");
    }

    if( !action_found ) {
		mpDebugPrint("##################!action_found###############");
        ca_event->ActionResult = NULL;
        strcpy( ca_event->ErrStr, "Invalid Action" );
        ca_event->ErrCode = 401;
    } else {
        if( retCode == UPNP_E_SUCCESS ) {
			//mpDebugPrint("Kevin ##################retCode == UPNP_E_SUCCESS###############");
            ca_event->ErrCode = UPNP_E_SUCCESS;
        } else {
            //copy the error string 
			mpDebugPrint("retCode %d",retCode );
            strcpy( ca_event->ErrStr, errorString );
            switch ( retCode ) {
				case -304:
					{
                        ca_event->ErrCode = 702;
                        break;
					}

                case UPNP_E_INVALID_PARAM:
                    {
                        ca_event->ErrCode = 402;
                        break;
                    }
	            case UPNP_E_INTERNAL_ERROR:
                default:
                    {
                        ca_event->ErrCode = 501;
                        break;
                    }

            }
        }
    }

    return ( ca_event->ErrCode );
}

/******************************************************************************
 * TvDeviceSetServiceTableVar
 *
 * Description: 
 *       Update the TvDevice service state table, and notify all subscribed 
 *       control points of the updated state.  Note that since this function
 *       blocks on the mutex TVDevMutex, to avoid a hang this function should 
 *       not be called within any other function that currently has this mutex 
 *       locked.
 *
 * Parameters:
 *   service -- The service number (TV_SERVICE_CONTROL or TV_SERVICE_PICTURE)
 *   variable -- The variable number (TV_CONTROL_POWER, TV_CONTROL_CHANNEL,
 *                   TV_CONTROL_VOLUME, TV_PICTURE_COLOR, TV_PICTURE_TINT,
 *                   TV_PICTURE_CONTRAST, or TV_PICTURE_BRIGHTNESS)
 *   value -- The string representation of the new value
 *
 *****************************************************************************/
int
TvDeviceSetServiceTableVar( IN unsigned int service,
                            IN unsigned int variable,
                            IN char *value )
{
    //IXML_Document  *PropSet= NULL;
#if 0
    if( ( service >= MS_SERVICE_SERVCOUNT )
        || ( variable >= tv_service_table[service].VariableCount )
        || ( strlen( value ) >= TV_MAX_VAL_LEN ) ) {
        return ( 0 );
    }

    //KK ithread_mutex_lock( &TVDevMutex );

    strcpy( tv_service_table[service].VariableStrVal[variable], value );

    /*
       //Using utility api
       PropSet= UpnpCreatePropertySet(1,tv_service_table[service].
       VariableName[variable], 
       tv_service_table[service].
       VariableStrVal[variable]);

       UpnpNotifyExt(device_handle, tv_service_table[service].UDN, 
       tv_service_table[service].ServiceId,PropSet);

       //Free created property set
       Document_free(PropSet);
     */

    UpnpNotify( device_handle,
                tv_service_table[service].UDN,
                tv_service_table[service].ServiceId,
                ( const char ** )&tv_service_table[service].
                VariableName[variable],
                ( const char ** )&tv_service_table[service].
                VariableStrVal[variable], 1 );

    //KK ithread_mutex_unlock( &TVDevMutex );
#endif
    return ( 1 );

}

/******************************************************************************
 * TvDeviceSetPower
 *
 * Description: 
 *       Turn the power on/off, update the TvDevice control service
 *       state table, and notify all subscribed control points of the
 *       updated state.
 *
 * Parameters:
 *   on -- If 1, turn power on.  If 0, turn power off.
 *
 *****************************************************************************/
int
TvDeviceSetPower( IN int on )
{
    char value[TV_MAX_VAL_LEN];
    int ret = 0;

    if( on != POWER_ON && on != POWER_OFF ) {
        SampleUtil_Print( "error: can't set power to value %d\n", on );
        return ( 0 );
    }

    /*
       Vendor-specific code to turn the power on/off goes here 
     */

    sprintf( value, "%d", on );
    ret = TvDeviceSetServiceTableVar( TV_SERVICE_CONTROL, TV_CONTROL_POWER,
                                      value );

    return ( ret );
}

/******************************************************************************
 * TvDevicePowerOn
 *
 * Description: 
 *       Turn the power on.
 *
 * Parameters:
 *
 *    IXML_Document * in - document of action request
 *    IXML_Document **out - action result
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDevicePowerOn( IN IXML_Document * in,
                 OUT IXML_Document ** out,
                 OUT char **errorString )
{
    ( *out ) = NULL;
    ( *errorString ) = NULL;

    if( TvDeviceSetPower( POWER_ON ) ) {
        //create a response

        if( UpnpAddToActionResponse( out, "PowerOn",
                                     MsServiceType[TV_SERVICE_CONTROL],
                                     "Power", "1" ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            return UPNP_E_INTERNAL_ERROR;
        }
        return UPNP_E_SUCCESS;
    } else {
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }

}

/******************************************************************************
 * TvDevicePowerOff
 *
 * Description: 
 *       Turn the power off.
 *
 * Parameters:
 *    
 *    IXML_Document * in - document of action request
 *    IXML_Document **out - action result
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDevicePowerOff( IN IXML_Document * in,
                  OUT IXML_Document ** out,
                  OUT char **errorString )
{
    ( *out ) = NULL;
    ( *errorString ) = NULL;
    if( TvDeviceSetPower( POWER_OFF ) ) {
        //create a response

        if( UpnpAddToActionResponse( out, "PowerOff",
                                     MsServiceType[TV_SERVICE_CONTROL],
                                     "Power", "0" ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            return UPNP_E_INTERNAL_ERROR;
        }

        return UPNP_E_SUCCESS;
    }

    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
}

/******************************************************************************
 * TvDeviceSetChannel
 *
 * Description: 
 *       Change the channel, update the TvDevice control service
 *       state table, and notify all subscribed control points of the
 *       updated state.
 *
 * Parameters:
 *    
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceSetChannel( IN IXML_Document * in,
                    OUT IXML_Document ** out,
                    OUT char **errorString )
{

    char *value = NULL;

    int channel = 0;

    ( *out ) = NULL;
    ( *errorString ) = NULL;

    if( !( value = SampleUtil_GetFirstDocumentItem( in, "Channel" ) ) ) {
        ( *errorString ) = "Invalid Channel";
        return UPNP_E_INVALID_PARAM;
    }

    channel = atoi( value );

    if( channel < MIN_CHANNEL || channel > MAX_CHANNEL ) {

        free( value );
        SampleUtil_Print( "error: can't change to channel %d\n", channel );
        ( *errorString ) = "Invalid Channel";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the channel goes here 
     */

    if( TvDeviceSetServiceTableVar( TV_SERVICE_CONTROL,
                                    TV_CONTROL_CHANNEL, value ) ) {
        if( UpnpAddToActionResponse( out, "SetChannel",
                                     MsServiceType[TV_SERVICE_CONTROL],
                                     "NewChannel",
                                     value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            free( value );
            return UPNP_E_INTERNAL_ERROR;
        }
        free( value );
        return UPNP_E_SUCCESS;
    } else {
        free( value );
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }

}

/******************************************************************************
 * IncrementChannel
 *
 * Description: 
 *       Increment the channel.  Read the current channel from the state
 *       table, add the increment, and then change the channel.
 *
 * Parameters:
 *   incr -- The increment by which to change the channel.
 *      
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *****************************************************************************/
int
IncrementChannel( IN int incr,
                  IN IXML_Document * in,
                  OUT IXML_Document ** out,
                  OUT char **errorString )
{
    int curchannel,
      newchannel;

    char *actionName = NULL;
    char value[TV_MAX_VAL_LEN];

    if( incr > 0 ) {
        actionName = "IncreaseChannel";
    } else {
        actionName = "DecreaseChannel";
    }

    //KK ithread_mutex_lock( &TVDevMutex );
    curchannel = atoi( tv_service_table[TV_SERVICE_CONTROL].
                       VariableStrVal[TV_CONTROL_CHANNEL] );
    //KK ithread_mutex_unlock( &TVDevMutex );

    newchannel = curchannel + incr;

    if( newchannel < MIN_CHANNEL || newchannel > MAX_CHANNEL ) {
        SampleUtil_Print( "error: can't change to channel %d\n",
                          newchannel );
        ( *errorString ) = "Invalid Channel";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the channel goes here 
     */

    sprintf( value, "%d", newchannel );

    if( TvDeviceSetServiceTableVar( TV_SERVICE_CONTROL,
                                    TV_CONTROL_CHANNEL, value ) ) {
        if( UpnpAddToActionResponse( out, actionName,
                                     MsServiceType[TV_SERVICE_CONTROL],
                                     "Channel", value ) != UPNP_E_SUCCESS )
        {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            return UPNP_E_INTERNAL_ERROR;
        }
        return UPNP_E_SUCCESS;
    } else {
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }
}

/******************************************************************************
 * TvDeviceDecreaseChannel
 *
 * Description: 
 *       Decrease the channel.  
 *
 * Parameters:
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceDecreaseChannel( IN IXML_Document * in,
                         OUT IXML_Document ** out,
                         OUT char **errorString )
{
    return IncrementChannel( -1, in, out, errorString );

}

/******************************************************************************
 * TvDeviceIncreaseChannel
 *
 * Description: 
 *       Increase the channel.  
 *
 * Parameters:
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceIncreaseChannel( IN IXML_Document * in,
                         OUT IXML_Document ** out,
                         OUT char **errorString )
{
    return IncrementChannel( 1, in, out, errorString );

}

/******************************************************************************
 * TvDeviceSetVolume
 *
 * Description: 
 *       Change the volume, update the TvDevice control service
 *       state table, and notify all subscribed control points of the
 *       updated state.
 *
 * Parameters:
 *  
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceSetVolume( IN IXML_Document * in,
                   OUT IXML_Document ** out,
                   OUT char **errorString )
{

    char *value = NULL;

    int volume = 0;

    ( *out ) = NULL;
    ( *errorString ) = NULL;

    if( !( value = SampleUtil_GetFirstDocumentItem( in, "Volume" ) ) ) {
        ( *errorString ) = "Invalid Volume";
        return UPNP_E_INVALID_PARAM;
    }

    volume = atoi( value );

    if( volume < MIN_VOLUME || volume > MAX_VOLUME ) {
        SampleUtil_Print( "error: can't change to volume %d\n", volume );
        ( *errorString ) = "Invalid Volume";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the volume goes here 
     */

    if( TvDeviceSetServiceTableVar( TV_SERVICE_CONTROL,
                                    TV_CONTROL_VOLUME, value ) ) {
        if( UpnpAddToActionResponse( out, "SetVolume",
                                     MsServiceType[TV_SERVICE_CONTROL],
                                     "NewVolume",
                                     value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            free( value );
            return UPNP_E_INTERNAL_ERROR;
        }
        free( value );
        return UPNP_E_SUCCESS;
    } else {
        free( value );
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }

}

/******************************************************************************
 * IncrementVolume
 *
 * Description: 
 *       Increment the volume.  Read the current volume from the state
 *       table, add the increment, and then change the volume.
 *
 * Parameters:
 *   incr -- The increment by which to change the volume.
 *      
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
IncrementVolume( IN int incr,
                 IN IXML_Document * in,
                 OUT IXML_Document ** out,
                 OUT char **errorString )
{
    int curvolume,
      newvolume;
    char *actionName = NULL;
    char value[TV_MAX_VAL_LEN];

    if( incr > 0 ) {
        actionName = "IncreaseVolume";
    } else {
        actionName = "DecreaseVolume";
    }

    //KK ithread_mutex_lock( &TVDevMutex );
    curvolume = atoi( tv_service_table[TV_SERVICE_CONTROL].
                      VariableStrVal[TV_CONTROL_VOLUME] );
    //KK ithread_mutex_unlock( &TVDevMutex );

    newvolume = curvolume + incr;

    if( newvolume < MIN_VOLUME || newvolume > MAX_VOLUME ) {
        SampleUtil_Print( "error: can't change to volume %d\n",
                          newvolume );
        ( *errorString ) = "Invalid Volume";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the channel goes here 
     */

    sprintf( value, "%d", newvolume );

    if( TvDeviceSetServiceTableVar( TV_SERVICE_CONTROL,
                                    TV_CONTROL_VOLUME, value ) ) {
        if( UpnpAddToActionResponse( out, actionName,
                                     MsServiceType[TV_SERVICE_CONTROL],
                                     "Volume", value ) != UPNP_E_SUCCESS )
        {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            return UPNP_E_INTERNAL_ERROR;
        }
        return UPNP_E_SUCCESS;
    } else {
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }

}

/******************************************************************************
 * TvDeviceIncrVolume
 *
 * Description: 
 *       Increase the volume. 
 *
 * Parameters:
 *   
 *
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *****************************************************************************/
int
TvDeviceIncreaseVolume( IN IXML_Document * in,
                        OUT IXML_Document ** out,
                        OUT char **errorString )
{

    return IncrementVolume( 1, in, out, errorString );

}

/******************************************************************************
 * TvDeviceDecreaseVolume
 *
 * Description: 
 *       Decrease the volume.
 *
 * Parameters:
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceDecreaseVolume( IN IXML_Document * in,
                        OUT IXML_Document ** out,
                        OUT char **errorString )
{

    return IncrementVolume( -1, in, out, errorString );

}

/******************************************************************************
 * TvDeviceSetColor
 *
 * Description: 
 *       Change the color, update the TvDevice picture service
 *       state table, and notify all subscribed control points of the
 *       updated state.
 *
 * Parameters:
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceSetColor( IN IXML_Document * in,
                  OUT IXML_Document ** out,
                  OUT char **errorString )
{

    char *value = NULL;

    int color = 0;

    ( *out ) = NULL;
    ( *errorString ) = NULL;
    if( !( value = SampleUtil_GetFirstDocumentItem( in, "Color" ) ) ) {
        ( *errorString ) = "Invalid Color";
        return UPNP_E_INVALID_PARAM;
    }

    color = atoi( value );

    if( color < MIN_COLOR || color > MAX_COLOR ) {
        SampleUtil_Print( "error: can't change to color %d\n", color );
        ( *errorString ) = "Invalid Color";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the volume goes here 
     */

    if( TvDeviceSetServiceTableVar( TV_SERVICE_PICTURE,
                                    TV_PICTURE_COLOR, value ) ) {
        if( UpnpAddToActionResponse( out, "SetColor",
                                     MsServiceType[TV_SERVICE_PICTURE],
                                     "NewColor",
                                     value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            free( value );
            return UPNP_E_INTERNAL_ERROR;
        }
        free( value );
        return UPNP_E_SUCCESS;
    } else {
        free( value );
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }

}

/******************************************************************************
 * IncrementColor
 *
 * Description: 
 *       Increment the color.  Read the current color from the state
 *       table, add the increment, and then change the color.
 *
 * Parameters:
 *   incr -- The increment by which to change the color.
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *****************************************************************************/

int
IncrementColor( IN int incr,
                IN IXML_Document * in,
                OUT IXML_Document ** out,
                OUT char **errorString )
{
    int curcolor,
      newcolor;

    char *actionName;
    char value[TV_MAX_VAL_LEN];

    if( incr > 0 ) {
        actionName = "IncreaseColor";
    } else {
        actionName = "DecreaseColor";
    }

    //KK ithread_mutex_lock( &TVDevMutex );
    curcolor = atoi( tv_service_table[TV_SERVICE_PICTURE].
                     VariableStrVal[TV_PICTURE_COLOR] );
    //KK ithread_mutex_unlock( &TVDevMutex );

    newcolor = curcolor + incr;

    if( newcolor < MIN_COLOR || newcolor > MAX_COLOR ) {
        SampleUtil_Print( "error: can't change to color %d\n", newcolor );
        ( *errorString ) = "Invalid Color";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the channel goes here 
     */

    sprintf( value, "%d", newcolor );

    if( TvDeviceSetServiceTableVar( TV_SERVICE_PICTURE,
                                    TV_PICTURE_COLOR, value ) ) {
        if( UpnpAddToActionResponse( out, actionName,
                                     MsServiceType[TV_SERVICE_PICTURE],
                                     "Color", value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            return UPNP_E_INTERNAL_ERROR;
        }
        return UPNP_E_SUCCESS;
    } else {
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }
}

/******************************************************************************
 * TvDeviceDecreaseColor
 *
 * Description: 
 *       Decrease the color.  
 *
 * Parameters:
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *****************************************************************************/
int
TvDeviceDecreaseColor( IN IXML_Document * in,
                       OUT IXML_Document ** out,
                       OUT char **errorString )
{

    return IncrementColor( -1, in, out, errorString );
}

/******************************************************************************
 * TvDeviceIncreaseColor
 *
 * Description: 
 *       Increase the color.
 *
 * Parameters:
 *
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *****************************************************************************/
int
TvDeviceIncreaseColor( IN IXML_Document * in,
                       OUT IXML_Document ** out,
                       OUT char **errorString )
{

    return IncrementColor( 1, in, out, errorString );
}

/******************************************************************************
 * TvDeviceSetTint
 *
 * Description: 
 *       Change the tint, update the TvDevice picture service
 *       state table, and notify all subscribed control points of the
 *       updated state.
 *
 * Parameters:
 *
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceSetTint( IN IXML_Document * in,
                 OUT IXML_Document ** out,
                 OUT char **errorString )
{

    char *value = NULL;

    int tint = -1;

    ( *out ) = NULL;
    ( *errorString ) = NULL;

    if( !( value = SampleUtil_GetFirstDocumentItem( in, "Tint" ) ) ) {
        ( *errorString ) = "Invalid Tint";
        return UPNP_E_INVALID_PARAM;
    }

    tint = atoi( value );

    if( tint < MIN_TINT || tint > MAX_TINT ) {
        SampleUtil_Print( "error: can't change to tint %d\n", tint );
        ( *errorString ) = "Invalid Tint";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the volume goes here 
     */

    if( TvDeviceSetServiceTableVar( TV_SERVICE_PICTURE,
                                    TV_PICTURE_TINT, value ) ) {
        if( UpnpAddToActionResponse( out, "SetTint",
                                     MsServiceType[TV_SERVICE_PICTURE],
                                     "NewTint", value ) != UPNP_E_SUCCESS )
        {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            free( value );
            return UPNP_E_INTERNAL_ERROR;
        }
        free( value );
        return UPNP_E_SUCCESS;
    } else {
        free( value );
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }

}

/******************************************************************************
 * IncrementTint
 *
 * Description: 
 *       Increment the tint.  Read the current tint from the state
 *       table, add the increment, and then change the tint.
 *
 * Parameters:
 *   incr -- The increment by which to change the tint.
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *****************************************************************************/
int
IncrementTint( IN int incr,
               IN IXML_Document * in,
               OUT IXML_Document ** out,
               OUT char **errorString )
{
    int curtint,
      newtint;

    char *actionName = NULL;
    char value[TV_MAX_VAL_LEN];

    if( incr > 0 ) {
        actionName = "IncreaseTint";
    } else {
        actionName = "DecreaseTint";
    }

    //KK ithread_mutex_lock( &TVDevMutex );
    curtint = atoi( tv_service_table[TV_SERVICE_PICTURE].
                    VariableStrVal[TV_PICTURE_TINT] );
    //KK ithread_mutex_unlock( &TVDevMutex );

    newtint = curtint + incr;

    if( newtint < MIN_TINT || newtint > MAX_TINT ) {
        SampleUtil_Print( "error: can't change to tint %d\n", newtint );
        ( *errorString ) = "Invalid Tint";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the channel goes here 
     */

    sprintf( value, "%d", newtint );

    if( TvDeviceSetServiceTableVar( TV_SERVICE_PICTURE,
                                    TV_PICTURE_TINT, value ) ) {
        if( UpnpAddToActionResponse( out, actionName,
                                     MsServiceType[TV_SERVICE_PICTURE],
                                     "Tint", value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            return UPNP_E_INTERNAL_ERROR;
        }
        return UPNP_E_SUCCESS;
    } else {
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }

}

/******************************************************************************
 * TvDeviceIncreaseTint
 *
 * Description: 
 *       Increase tint.
 *
 * Parameters:
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceIncreaseTint( IN IXML_Document * in,
                      OUT IXML_Document ** out,
                      OUT char **errorString )
{

    return IncrementTint( 1, in, out, errorString );
}

/******************************************************************************
 * TvDeviceDecreaseTint
 *
 * Description: 
 *       Decrease tint.
 *
 * Parameters:
 *  
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceDecreaseTint( IN IXML_Document * in,
                      OUT IXML_Document ** out,
                      OUT char **errorString )
{

    return IncrementTint( -1, in, out, errorString );
}

/*****************************************************************************
 * TvDeviceSetContrast
 *
 * Description: 
 *       Change the contrast, update the TvDevice picture service
 *       state table, and notify all subscribed control points of the
 *       updated state.
 *
 * Parameters:
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 ****************************************************************************/
int
TvDeviceSetContrast( IN IXML_Document * in,
                     OUT IXML_Document ** out,
                     OUT char **errorString )
{

    char *value = NULL;
    int contrast = -1;

    ( *out ) = NULL;
    ( *errorString ) = NULL;

    if( !( value = SampleUtil_GetFirstDocumentItem( in, "Contrast" ) ) ) {
        ( *errorString ) = "Invalid Contrast";
        return UPNP_E_INVALID_PARAM;
    }

    contrast = atoi( value );

    if( contrast < MIN_CONTRAST || contrast > MAX_CONTRAST ) {
        SampleUtil_Print( "error: can't change to contrast %d\n",
                          contrast );
        ( *errorString ) = "Invalid Contrast";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the volume goes here 
     */

    if( TvDeviceSetServiceTableVar( TV_SERVICE_PICTURE,
                                    TV_PICTURE_CONTRAST, value ) ) {
        if( UpnpAddToActionResponse( out, "SetContrast",
                                     MsServiceType[TV_SERVICE_PICTURE],
                                     "NewContrast",
                                     value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            free( value );
            return UPNP_E_INTERNAL_ERROR;
        }
        free( value );
        return UPNP_E_SUCCESS;
    } else {
        free( value );
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }

}

/******************************************************************************
 * IncrementContrast
 *
 * Description: 
 *       Increment the contrast.  Read the current contrast from the state
 *       table, add the increment, and then change the contrast.
 *
 * Parameters:
 *   incr -- The increment by which to change the contrast.
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *****************************************************************************/
int
IncrementContrast( IN int incr,
                   IN IXML_Document * in,
                   OUT IXML_Document ** out,
                   OUT char **errorString )
{
    int curcontrast,
      newcontrast;

    char *actionName = NULL;
    char value[TV_MAX_VAL_LEN];

    if( incr > 0 ) {
        actionName = "IncreaseContrast";
    } else {
        actionName = "DecreaseContrast";
    }

    //KK ithread_mutex_lock( &TVDevMutex );
    curcontrast = atoi( tv_service_table[TV_SERVICE_PICTURE].
                        VariableStrVal[TV_PICTURE_CONTRAST] );
    //KK ithread_mutex_unlock( &TVDevMutex );

    //KK newcontrast = curcontrast + incr;

    if( newcontrast < MIN_CONTRAST || newcontrast > MAX_CONTRAST ) {
        SampleUtil_Print( "error: can't change to contrast %d\n",
                          newcontrast );
        ( *errorString ) = "Invalid Contrast";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the channel goes here 
     */

    sprintf( value, "%d", newcontrast );

    if( TvDeviceSetServiceTableVar( TV_SERVICE_PICTURE,
                                    TV_PICTURE_CONTRAST, value ) ) {
        if( UpnpAddToActionResponse( out, actionName,
                                     MsServiceType[TV_SERVICE_PICTURE],
                                     "Contrast",
                                     value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            return UPNP_E_INTERNAL_ERROR;
        }
        return UPNP_E_SUCCESS;
    } else {
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }
}

/******************************************************************************
 * TvDeviceIncreaseContrast
 *
 * Description: 
 *
 *      Increase the contrast.
 *
 * Parameters:
 *       
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceIncreaseContrast( IN IXML_Document * in,
                          OUT IXML_Document ** out,
                          OUT char **errorString )
{

    return IncrementContrast( 1, in, out, errorString );
}

/******************************************************************************
 * TvDeviceDecreaseContrast
 *
 * Description: 
 *      Decrease the contrast.
 *
 * Parameters:
 *          
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceDecreaseContrast( IXML_Document * in,
                          IXML_Document ** out,
                          char **errorString )
{
    return IncrementContrast( -1, in, out, errorString );
}

/******************************************************************************
 * TvDeviceSetBrightness
 *
 * Description: 
 *       Change the brightness, update the TvDevice picture service
 *       state table, and notify all subscribed control points of the
 *       updated state.
 *
 * Parameters:
 *   brightness -- The brightness value to change to.
 *
 *****************************************************************************/
int
TvDeviceSetBrightness( IN IXML_Document * in,
                       OUT IXML_Document ** out,
                       OUT char **errorString )
{

    char *value = NULL;
    int brightness = -1;

    ( *out ) = NULL;
    ( *errorString ) = NULL;

    if( !( value = SampleUtil_GetFirstDocumentItem( in, "Brightness" ) ) ) {
        ( *errorString ) = "Invalid Brightness";
        return UPNP_E_INVALID_PARAM;
    }

    brightness = atoi( value );

    if( brightness < MIN_BRIGHTNESS || brightness > MAX_BRIGHTNESS ) {
        SampleUtil_Print( "error: can't change to brightness %d\n",
                          brightness );
        ( *errorString ) = "Invalid Brightness";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the volume goes here 
     */

    if( TvDeviceSetServiceTableVar( TV_SERVICE_PICTURE,
                                    TV_PICTURE_BRIGHTNESS, value ) ) {
        if( UpnpAddToActionResponse( out, "SetBrightness",
                                     MsServiceType[TV_SERVICE_PICTURE],
                                     "NewBrightness",
                                     value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            free( value );
            return UPNP_E_INTERNAL_ERROR;
        }
        free( value );
        return UPNP_E_SUCCESS;
    } else {
        free( value );
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }

}

/******************************************************************************
 * IncrementBrightness
 *
 * Description: 
 *       Increment the brightness.  Read the current brightness from the state
 *       table, add the increment, and then change the brightness.
 *
 * Parameters:
 *   incr -- The increment by which to change the brightness.
 *   
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *****************************************************************************/
int
IncrementBrightness( IN int incr,
                     IN IXML_Document * in,
                     OUT IXML_Document ** out,
                     OUT char **errorString )
{
    int curbrightness,
      newbrightness;
    char *actionName = NULL;
    char value[TV_MAX_VAL_LEN];

    if( incr > 0 ) {
        actionName = "IncreaseBrightness";
    } else {
        actionName = "DecreaseBrightness";
    }

    //KK ithread_mutex_lock( &TVDevMutex );
    curbrightness = atoi( tv_service_table[TV_SERVICE_PICTURE].
                          VariableStrVal[TV_PICTURE_BRIGHTNESS] );
    //KK ithread_mutex_unlock( &TVDevMutex );

    newbrightness = curbrightness + incr;

    if( newbrightness < MIN_BRIGHTNESS || newbrightness > MAX_BRIGHTNESS ) {
        SampleUtil_Print( "error: can't change to brightness %d\n",
                          newbrightness );
        ( *errorString ) = "Invalid Brightness";
        return UPNP_E_INVALID_PARAM;
    }

    /*
       Vendor-specific code to set the channel goes here 
     */

    sprintf( value, "%d", newbrightness );

    if( TvDeviceSetServiceTableVar( TV_SERVICE_PICTURE,
                                    TV_PICTURE_BRIGHTNESS, value ) ) {
        if( UpnpAddToActionResponse( out, actionName,
                                     MsServiceType[TV_SERVICE_PICTURE],
                                     "Brightness",
                                     value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            return UPNP_E_INTERNAL_ERROR;
        }
        return UPNP_E_SUCCESS;
    } else {
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }
}

/******************************************************************************
 * TvDeviceIncreaseBrightness
 *
 * Description: 
 *       Increase brightness.
 *
 * Parameters:
 *
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceIncreaseBrightness( IN IXML_Document * in,
                            OUT IXML_Document ** out,
                            OUT char **errorString )
{
    return IncrementBrightness( 1, in, out, errorString );
}

/******************************************************************************
 * TvDeviceDecreaseBrightness
 *
 * Description: 
 *       Decrease brightnesss.
 *
 * Parameters:
 *    IXML_Document * in -  action request document
 *    IXML_Document **out - action result document
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/
int
TvDeviceDecreaseBrightness( IN IXML_Document * in,
                            OUT IXML_Document ** out,
                            OUT char **errorString )
{
    return IncrementBrightness( -1, in, out, errorString );
}

/******************************************************************************
 * TvDeviceCallbackEventHandler
 *
 * Description: 
 *       The callback handler registered with the SDK while registering
 *       root device.  Dispatches the request to the appropriate procedure
 *       based on the value of EventType. The four requests handled by the 
 *       device are: 
 *                   1) Event Subscription requests.  
 *                   2) Get Variable requests. 
 *                   3) Action requests.
 *
 * Parameters:
 *
 *   EventType -- The type of callback event
 *   Event -- Data structure containing event data
 *   Cookie -- Optional data specified during callback registration
 *
 *****************************************************************************/
int
TvDeviceCallbackEventHandler( Upnp_EventType EventType,
                              void *Event,
                              void *Cookie )
{

	mpDebugPrint("#######TvDeviceCallbackEventHandler EventType %x",EventType);
    switch ( EventType ) {

        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
			mpDebugPrint("TvDeviceCallbackEventHandler UPNP_EVENT_SUBSCRIPTION_REQUEST");
            TvDeviceHandleSubscriptionRequest( ( struct
                                                 Upnp_Subscription_Request
                                                 * )Event );
            break;

        case UPNP_CONTROL_GET_VAR_REQUEST:
			//mpDebugPrint("ZZZZZZZZZZZZZZZZZZ##UPNP_CONTROL_GET_VAR_REQUEST");
            TvDeviceHandleGetVarRequest( ( struct Upnp_State_Var_Request
                                           * )Event );
            break;

        case UPNP_CONTROL_ACTION_REQUEST:
			//mpDebugPrint("ZZZZZZZZZZZZZZZZZZ##UPNP_CONTROL_ACTION_REQUEST %d",g_bXpgStatus);
			if( g_bXpgStatus == XPG_MODE_DLNA_1_5_DMR )
				DmrDeviceHandleActionRequest( ( struct Upnp_Action_Request * )
											 Event );
			else
			
				TvDeviceHandleActionRequest( ( struct Upnp_Action_Request * )
                                         Event );
            break;

            /*
               ignore these cases, since this is not a control point 
             */
        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
        case UPNP_DISCOVERY_SEARCH_RESULT:
        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
        case UPNP_CONTROL_ACTION_COMPLETE:
        case UPNP_CONTROL_GET_VAR_COMPLETE:
        case UPNP_EVENT_RECEIVED:
        case UPNP_EVENT_RENEWAL_COMPLETE:
        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
            break;

        default:
            SampleUtil_Print
                ( "Error in TvDeviceCallbackEventHandler: unknown event type %d\n",
                  EventType );
    }

    /*
       Print a summary of the event received 
     */
    //SampleUtil_PrintEvent( EventType, Event );

    return ( 0 );
}

/******************************************************************************
 * TvDeviceStop
 *
 * Description: 
 *       Stops the device. Uninitializes the sdk. 
 *
 * Parameters:
 *
 *****************************************************************************/
int
TvDeviceStop(  )
{
    UpnpUnRegisterRootDevice( device_handle );
    //UpnpFinish(  );
    //SampleUtil_Finish(  );
    //KK ithread_mutex_destroy( &TVDevMutex );
    return UPNP_E_SUCCESS;
}

unsigned char advertizeexpire = 0;
typedef enum {HND_INVALID=-1,HND_CLIENT,HND_DEVICE} Upnp_Handle_Type;
void AutoAdvertisetimer(void)
{
	int ret;
    UpnpDevice_Handle device_handle;
    struct Handle_Info *handle_info;
	mpDebugPrint("##############AutoAdvertisetimer");
    if( GetDeviceHandleInfo( &device_handle, &handle_info ) != HND_DEVICE )
		mpDebugPrint("###############GetDeviceHandleInfo Fail");

	if( ( ret = UpnpSendAdvertisement( device_handle, default_advr_expire ) ) != UPNP_E_SUCCESS )
	{
		mpDebugPrint( "Error sending advertisements : %d\n", ret );
	}
	//AddTimerProc(30000,AutoAdvertisetimer);
	mpDebugPrint("AutoAdvertisetimer Sucess");
	advertizeexpire = 1;
}

/******************************************************************************
 * TvDeviceStart
 *
 * Description: 
 *      Initializes the UPnP Sdk, registers the device, and sends out 
 *      advertisements.  
 *
 * Parameters:
 *
 *   ip_address - ip address to initialize the sdk (may be NULL)
 *                if null, then the first non null loopback address is used.
 *   port       - port number to initialize the sdk (may be 0)
 *                if zero, then a random number is used.
 *   desc_doc_name - name of description document.
 *                   may be NULL. Default is tvdevicedesc.xml
 *   web_dir_path  - path of web directory.
 *                   may be NULL. Default is ./web (for Linux) or ../tvdevice/web
 *                   for windows.
 *   pfun          - print function to use.  
 *
 *****************************************************************************/
int
TvDeviceStart( char *ip_address,
               unsigned short port,
               char *desc_doc_name,
               char *web_dir_path,
               print_string pfun )
{
    int ret = UPNP_E_SUCCESS;

    char desc_doc_url[DESC_URL_SIZE];

    //KK ithread_mutex_init( &TVDevMutex, NULL );
	MP_DEBUG("TvDeviceStart==>");
    SampleUtil_Initialize( pfun );

    SampleUtil_Print
        ( "Initializing UPnP Sdk with \n \t ipaddress = %s port = %d\n",
          ip_address, port );

    if( ( ret = UpnpInit( ip_address, port ) ) != UPNP_E_SUCCESS ) {
		mpDebugPrint("Error with UpnpInit -- %d\n",ret);
        SampleUtil_Print( "Error with UpnpInit -- %d\n", ret );
        UpnpFinish(  );
        return ret;
    }
    if( ip_address == NULL ) {
        ip_address = UpnpGetServerIpAddress(  );
    }

    if( port == 0 ) {
        port = UpnpGetServerPort(  );
    }
    SampleUtil_Print( "UPnP Initialized\n \t ipaddress= %s port = %d\n",
                      ip_address, port );

    if( desc_doc_name == NULL )
	{
		if( g_bXpgStatus != XPG_MODE_DLNA_1_5_DMR )
			desc_doc_name = "mrdesc.xml";
		else
			//desc_doc_name = "tvdesc.xml";//"mrdesc.xml";//"tvdevicedesc.xml";
			desc_doc_name = "dlna_dmr.xml";
	}

    if( web_dir_path == NULL )
        web_dir_path = DEFAULT_WEB_DIR;

    snprintf( desc_doc_url, DESC_URL_SIZE, "http://%s:%d/%s", ip_address,
              port, desc_doc_name );

    SampleUtil_Print( "Specifying the webserver root directory -- %s\n",
                      web_dir_path );
    if( ( ret = UpnpSetWebServerRootDir( web_dir_path ) ) != UPNP_E_SUCCESS ) 
	{

		mpDebugPrint("Error specifying webserver root directory -- %s: %d\n",
              web_dir_path, ret);

        UpnpFinish();
        return ret;
    }

    MP_DEBUG( "Registering the RootDevice\n\t with desc_doc_url: %s\n",desc_doc_url );
	
    if( ( ret = UpnpRegisterRootDevice( desc_doc_url,
                                        TvDeviceCallbackEventHandler,
                                        &device_handle, &device_handle ) )
        != UPNP_E_SUCCESS ) 
	{
        SampleUtil_Print( "Error registering the rootdevice : %d\n", ret );
        UpnpFinish(  );
        return ret;
    } else {
        SampleUtil_Print( "RootDevice Registered\n" );

        SampleUtil_Print( "Initializing State Table\n" );
		if( g_bXpgStatus != XPG_MODE_DLNA_1_5_DMR )
			TvDeviceStateTableInit( desc_doc_url );
		else
			DmrDeviceStateTableInit( desc_doc_url );
        SampleUtil_Print( "State Table Initialized\n" );
		mpDebugPrint("State Table Initialized");
        if( ( ret =
              UpnpSendAdvertisement( device_handle, default_advr_expire ) )
            != UPNP_E_SUCCESS ) {
            SampleUtil_Print( "Error sending advertisements : %d\n", ret );
            UpnpFinish(  );
            return ret;
        }

        //SampleUtil_Print( "Advertisements Sent\n" );
		mpDebugPrint("Advertisements Sent");
    }
    return UPNP_E_SUCCESS;
}

//===============================================
//Kevin ADD
#if HAVE_UPNP_MEDIA_SERVER
int
MsCreateObjectAction( IN IXML_Document * in,
                    OUT IXML_Document ** out,
                    OUT char **errorString )
{
    char *value = NULL;
    char *bf_value = NULL;
    char *resp_value = NULL;
	DRIVE *sDrv;
	STREAM *shandle;
	int ret;
	XML_ImageBUFF_link_t *qtr;

    int channel = 0;
    IXML_Document *respDoc = NULL;
	char httpurl[1024];
	char str_objectid[32];
    void *xp, *prev;
	int total_data_len;
	static unsigned int itemidindex = 1;
	char *itemid,*resultbuf;
    ( *out ) = NULL;
    ( *errorString ) = NULL;

	mpDebugPrint("MsCreateObjectAction =============>");
    if( !( value = SampleUtil_GetFirstDocumentItem( in, "ContainerID" ) ) ) {
        ( *errorString ) = "Invalid ObjectID";
        return UPNP_E_INVALID_PARAM;
    }
	mpDebugPrint("value is ##%s##",value);
	ixml_mem_free(value);
    if( !( bf_value = SampleUtil_GetFirstDocumentItem( in, "Elements" ) ) ) {
        ( *errorString ) = "Invalid ObjectID";
        return UPNP_E_INVALID_PARAM;
    }
	mpDebugPrint("bf_value is ##%s##",bf_value);
	//strstr(bf_value,)
    if( ixmlParseBufferEx( bf_value, &respDoc ) == IXML_INVALID_PARAMETER )
		mpDebugPrint("IXML_INVALID_PARAMETER");
	else
	{
		mpDebugPrint("ixmlParseBufferEx sucess");
		resp_value =  SampleUtil_GetFirstDocumentItem( respDoc, "dc:title" );
		if( resp_value )
		{
			mpDebugPrint("resp_value %s",resp_value);
			ixml_mem_free(resp_value);
		}
		resp_value =  SampleUtil_GetFirstDocumentItem( respDoc, "res" );
		strcpy(httpurl,resp_value);
		if( resp_value )
		{
			mpDebugPrint("resp_value %s",resp_value);
			ixml_mem_free(resp_value);
		}
	}
	itemid = ixml_mem_malloc(32);
	sprintf(itemid,"I3$%d",itemidindex++);
	ixml_mem_free(bf_value);
	resultbuf = ixml_mem_malloc(1024);
	sprintf(resultbuf,
		"<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:pns=\"http://www.philips.com/streamiumns/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\">"
		"<item id=\"%s\" parentID=\"MPX$SD$1\" restricted=\"0\" dlna:dlnaManaged=\"00000004\">"
		"<upnp:class>object.item.imageItem.photo</upnp:class><dc:title>Kevin1.jpg</dc:title>"
		"<upnp:album>Unknown</upnp:album><upnp:genre>Unknown</upnp:genre>"
		"<res importUri=\"http://172.31.100.111:49153/createobject/OI3$2\" protocolInfo=\"http:*:image/jpeg:*\"></res>"
		"</item>"
		"</DIDL-Lite>",itemid);
	//sprintf(str_objectid,"MPX$SD$%d");
    if( UpnpAddToActionResponseExt( out, "CreateObject",
                                 MsServiceType[MS_SERVICE_CD],
								 2,
                                 "ObjectID","MPX$SD$1",
								 "Result",resultbuf
								 ) != UPNP_E_SUCCESS ) 
	{
		mpDebugPrint("UpnpAddToActionResponse error");
		( *out ) = NULL;
		( *errorString ) = "Kevin Internal Error";
		ixml_mem_free( value );
		ixml_mem_free(resultbuf);
		return UPNP_E_INTERNAL_ERROR;
	}
	ixml_mem_free(resultbuf);
#if 0
	App_State.dwState =  NET_RECVFLICKR;
    (XML_BUFF_link_t *)xp = ixml_mem_malloc(sizeof(XML_BUFF_link_t));
    memset(xp, 0, sizeof(XML_BUFF_link_t));

	mpDebugPrint("CURL httpurl %s",httpurl);
	total_data_len = Get_Image_File(httpurl, ((XML_BUFF_link_t *)xp)->BUFF);
	mpDebugPrint("total_data_len %d",total_data_len);
	//ixml_mem_free(xp);
#endif

	Xml_BUFF_init(NET_RECVUPNP);
	ret = Net_Recv_Data(httpurl, NET_RECVUPNP,0,0);
	total_data_len = ret;
	mpDebugPrint("total_data_len %d",total_data_len);
	if( total_data_len > 0)
	{
	    sDrv=DriveGet(SD_MMC);
        ret=CreateFile(sDrv, itemid, "jpg");
        if (ret) UartOutText("create file fail\r\n");
        shandle=FileOpen(sDrv);
        if(!shandle) UartOutText("open file fail\r\n");

		//ret=FileWrite(shandle, "KEvin Test", strlen("KEvin Test"));
		qtr = App_State.XML_BUF1;
		while( qtr )
		{
			mpDebugPrint("qtr->buff_len %d",qtr->buff_len);
			ret=FileWrite(shandle, qtr->BUFF, qtr->buff_len);
		    if(!ret) 
			{
				UartOutText("write file fail\r\n");
				break;
			}
			qtr = qtr->link;
		}

        FileClose(shandle);
        UartOutText("\n\rfile close\n\r");
	}
	Xml_BUFF_free(NET_RECVUPNP);
	ixml_mem_free(itemid);
	//ext_mem_free(xp);
#if HAVE_UPNP_MEDIA_SERVER
	sprintf(gupnpfilename,"%s.jpg",itemid);
	MP_DEBUG("UPNP_Server_Event %d",UPNP_Server_Event);
	mpx_EventSet(UPNP_Server_Event, 0x1);
	MP_DEBUG("UpnpAddToActionResponse UPNP_E_SUCCESS");
#endif
	return UPNP_E_SUCCESS;
}

int
MsBrowseAction( IN IXML_Document * in,
                    OUT IXML_Document ** out,
                    OUT char **errorString )
{

    char *value = NULL;
    char *bf_value = NULL;

    int channel = 0;

    ( *out ) = NULL;
    ( *errorString ) = NULL;

	mpDebugPrint("MsBrowseAction =============>");
    if( !( value = SampleUtil_GetFirstDocumentItem( in, "ObjectID" ) ) ) {
        ( *errorString ) = "Invalid ObjectID";
        return UPNP_E_INVALID_PARAM;
    }

	mpDebugPrint("value is ##%s##",value);
	ixml_mem_free(value);
    if( !( bf_value = SampleUtil_GetFirstDocumentItem( in, "BrowseFlag" ) ) ) {
        ( *errorString ) = "Invalid ObjectID";
        return UPNP_E_INVALID_PARAM;
    }
	mpDebugPrint("bf_value is ##%s##",bf_value);
/*
    if( UpnpAddToActionResponseExt( out, "Browse",
                                 MsServiceType[MS_SERVICE_CD],
								 4,
                                 "TotalMatches","1",
								 "UpdateID","1",
								 "NumberReturned","1",
								 "Result",
								"<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\"xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:pns=\"http://www.magicpixel.com.tw/streamiumns/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\">"
								"<container id=\"music:0\" parentID=\"0\" childCount=\"3\" restricted=\"false\" searchable=\"false\">"
								"<dc:title>Music</dc:title><desc nameSpace=\"http://www.simpledevices.com/ns/upnp/id=\"fixedContainer\">music</desc><upnp:class>object.container</upnp:class>"
								"<description>Your music library</description></container></DIDL-Lite>"
								 ) != UPNP_E_SUCCESS ) 
*/
	if( strcmp(bf_value,"BrowseMetadata") ==0 )
	{
		if( UpnpAddToActionResponseExt( out, "Browse",
									 MsServiceType[MS_SERVICE_CD],
									 4,
									 "TotalMatches","1",
									 "UpdateID","4",
									 "NumberReturned","1",
									 "Result",
									"<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:pns=\"http://www.philips.com/streamiumns/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\">"
									"<container id=\"0\" parentID=\"-1\" childCount=\"2\" restricted=\"false\" searchable=\"false\">"
									"<dc:title>Kevin</dc:title><upnp:class>object.container.root</upnp:class>"
									"</container></DIDL-Lite>"//<description>Your music library</description>
									 ) != UPNP_E_SUCCESS ) 

//<Result>&lt;DIDL-Lite xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/" xmlns:pns="http://www.philips.com/streamiumns/" xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
//&gt;&lt;container id="0" parentID="-1" childCount="4" restricted="false" searchable="false"&gt;&lt;dc:title&gt;Philips Media Manager&lt;/dc:titl
//e&gt;&lt;upnp:class&gt;object.container.root&lt;/upnp:class&gt;&lt;/container&gt;&lt;/DIDL-Lite&gt;</Result></n0:BrowseResponse>
//</s:Body></s:Envelope>

/*
    if( UpnpAddToActionResponse( out, "Browse",
                                 MsServiceType[MS_SERVICE_CD],
                                 "TotalMatches","1"
								) != UPNP_E_SUCCESS ) 
*/
		{
			mpDebugPrint("UpnpAddToActionResponse error");
			( *out ) = NULL;
			( *errorString ) = "Kevin Internal Error";
			ixml_mem_free( bf_value );
			return UPNP_E_INTERNAL_ERROR;
		}
	}
	if( strcmp(bf_value,"BrowseDirectChildren") ==0 )
	{
    if( UpnpAddToActionResponseExt( out, "Browse",
                                 MsServiceType[MS_SERVICE_CD],
								 4,
                                 "TotalMatches","2",
								 "UpdateID","4",
								 "NumberReturned","2",
								 "Result",
								"<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:pns=\"http://www.philips.com/streamiumns/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\">"
								"<container id=\"music:0\" parentID=\"0\" childCount=\"1\" restricted=\"false\" searchable=\"false\">"
								"<dc:title>Music</dc:title><upnp:class>object.container</upnp:class>"
								"<desc nameSpace=\"http://www.simpledevices.com/ns/upnp/id=\"fixedContainer\">music</desc>"
								"<description>Your music library</description></container>"
								"<container id=\"photo:0\" parentID=\"0\" childCount=\"2\" restricted=\"false\" searchable=\"false\">"
								"<dc:title>Photo</dc:title><upnp:class>object.container</upnp:class>"
								"<desc nameSpace=\"http://www.simpledevices.com/ns/upnp/id=\"fixedContainer\">photo</desc>"
								"<description>Your photo library</description></container>"
								"</DIDL-Lite>"
								 ) != UPNP_E_SUCCESS ) 

//<Result>&lt;DIDL-Lite xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/" xmlns:pns="http://www.philips.com/streamiumns/" xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
//&gt;&lt;container id="0" parentID="-1" childCount="4" restricted="false" searchable="false"&gt;&lt;dc:title&gt;Philips Media Manager&lt;/dc:titl
//e&gt;&lt;upnp:class&gt;object.container.root&lt;/upnp:class&gt;&lt;/container&gt;&lt;/DIDL-Lite&gt;</Result></n0:BrowseResponse>
//</s:Body></s:Envelope>

/*
    if( UpnpAddToActionResponse( out, "Browse",
                                 MsServiceType[MS_SERVICE_CD],
                                 "TostalMatches","1"
								) != UPNP_E_SUCCESS ) 
*/
		{
			mpDebugPrint("UpnpAddToActionResponse error");
			( *out ) = NULL;
			( *errorString ) = "Kevin Internal Error";
			ixml_mem_free( bf_value );
			return UPNP_E_INTERNAL_ERROR;
		}
	}
	ixml_mem_free(bf_value);
	mpDebugPrint("UpnpAddToActionResponse UPNP_E_SUCCESS");
	return UPNP_E_SUCCESS;
    //channel = atoi( value );
/*
    if( channel < MIN_CHANNEL || channel > MAX_CHANNEL ) {

        free( value );
        SampleUtil_Print( "error: can't change to channel %d\n", channel );
        ( *errorString ) = "Invalid Channel";
        return UPNP_E_INVALID_PARAM;
    }
*/
    /*
       Vendor-specific code to set the channel goes here 
     */
/*
    if( TvDeviceSetServiceTableVar( TV_SERVICE_CONTROL,
                                    TV_CONTROL_CHANNEL, value ) ) {
        if( UpnpAddToActionResponse( out, "SetChannel",
                                     MsServiceType[TV_SERVICE_CONTROL],
                                     "NewChannel",
                                     value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
            free( value );
            return UPNP_E_INTERNAL_ERROR;
        }
        ixml_mem_free( value );
        return UPNP_E_SUCCESS;
    } else {
        free( value );
        ( *errorString ) = "Internal Error";
        return UPNP_E_INTERNAL_ERROR;
    }
*/
}
#endif
unsigned char bstreaming = 0;
char gduration[16];
char *gstreamingurl = NULL;

int DMRCM_GetProtocolInfo( IXML_Document * in, IXML_Document ** out, char **errorString )
{
	mpDebugPrint("DMRCM_GetProtocolInfo");
    if( UpnpAddToActionResponseExt( out, "GetProtocolInfo",
                                 DmrServiceType[0],
								 2,
                                 "Source","",
								 "Sink",
								"http-get:*:image/bmp:*,http-get:*:image/gif:*,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=8c900000000000000000000000000000,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN;DLNA.ORG_OP=01;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=8c900000000000000000000000000000,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM;DLNA.ORG_OP=01;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=8c900000000000000000000000000000"
								 ) != UPNP_E_SUCCESS ) 

	{
		mpDebugPrint("DMRCM_GetProtocolInfo respondse error");
		( *out ) = NULL;
		( *errorString ) = "Kevin Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	mpDebugPrint("DMRCM_GetProtocolInfo UPNP_E_SUCCESS");
	return UPNP_E_SUCCESS;
}

static int DMRAVT_Play_Stop( IXML_Document * in, IXML_Document ** out, char **errorString )
{
	mpDebugPrint("DMRAVT_Play_Stop %s",CurrentActionName);
	if( (strcmp(CurrentActionName,"Play") == 0)  && (gstreamingurl== NULL) && (gavtransuri == NULL) )
	{
		mpDebugPrint("#############No Content########");
		( *out ) = NULL;
		( *errorString ) = "No contents";
		return -304;//UPNP_E_USER_DEFINE;
	}
    if( UpnpAddToActionResponseExt( out, CurrentActionName,
                                 DmrServiceType[1],
								 0
								 ) != UPNP_E_SUCCESS ) 

	{
		mpDebugPrint("DMRAVT_Stop respondse error");
		( *out ) = NULL;
		( *errorString ) = "Kevin Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}
	if( strcmp(CurrentActionName,"Stop") == 0 )
	{
		if( bPlay && bstreaming )
		{
			xpgStopAudio();
			//g_internetradiorun = 0;
			bstreaming = 0;
		}
		bPlay = 0;

	}	
	if( strcmp(CurrentActionName,"Play") == 0 )
	{
		bPlay = 1;
		//if( bdlnadownload == 0 )
		{
			mpDebugPrint("bdlnadownload == 0");
			if( bstreaming )
			{
				xpgCb_MusicPlay();
			}
			else
			{
				//gavtransuri = current_uri;
				bdlnadownload = 1;
				EventSet(UPNP_START_EVENT, 0x00010000);
			}
			//mpx_EventSet(UPNP_Server_Event, 0x1);
			//mpDebugPrint("END SET INTERNET_RADIO_EVENT");	
		}
/*
		else
		{
			mpDebugPrint("Still in download process");
			ixml_mem_free(current_uri);
		}
*/
	}
	if( strcmp(CurrentActionName,"Pause") == 0 )
	{
		if( bPlay == 1 )
			bPlay = 2;
	}

	mpDebugPrint("DMRAVT_Stop UPNP_E_SUCCESS");
	return UPNP_E_SUCCESS;
}
int DMRAVT_GetTransportInfo( IXML_Document * in, IXML_Document ** out, char **errorString )
{
	char tmp[32];
	char respstr[32];
	//mpDebugPrint("DMRAVT_GetTransportInfo %d",bdlnadownload);
	strcpy(respstr,"OK");
	if( bPlay == 1 )
	{
		if( bdlnadownload & 0x1 )
			//strcpy(tmp,"TRANSITIONING");
			strcpy(tmp,"PLAYING");
		else if( bdlnadownload & 0x80 )
		{
			strcpy(tmp,"STOPPED");
			strcpy(respstr,"ERROR_OCCURRED");
		}
		else
			strcpy(tmp,"PLAYING");
	}
	else if( bPlay == 0 )
	{
		//if( bdlnadownload )
		//	strcpy(tmp,"TRANSITIONING");
		//else
		if( bdlnadownload & 0x1 )
			strcpy(tmp,"PLAYING");
		else if( bdlnadownload & 0x80 )
		{
			strcpy(tmp,"STOPPED");
			strcpy(respstr,"ERROR_OCCURRED");
		}
		else
			strcpy(tmp,"STOPPED");
	}
	else if ( bPlay == 2 )
		strcpy(tmp,"PAUSED_PLAYBACK");

    if( UpnpAddToActionResponseExt( out, "GetTransportInfo",
                                 DmrServiceType[1],
								 3,
								 "CurrentTransportState",tmp,//"CurrentTransportState","STOPPED",
								 "CurrentTransportStatus",respstr,//"OK",
								 "CurrentSpeed","1"
								 ) != UPNP_E_SUCCESS ) 

	{
		mpDebugPrint("DMRAVT_GetTransportInfo");
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	//mpDebugPrint("DMRAVT_GetTransportInfo UPNP_E_SUCCESS");
	return UPNP_E_SUCCESS;
}
int DMRAVT_GetPositionInfo( IXML_Document * in, IXML_Document ** out, char **errorString )
{
	unsigned long dwSec;
	char *trackuri;
	//char *nullurl = "";
	char duration[16];
	char curtime[16];
	//if( bstreaming )
	//	strcpy(duration,gduration);
	//else
		strcpy(duration,"00:00:00");
	dwSec = 0;
#if 0
	if( gstreamingurl && bPlay )
	{
		trackuri = gstreamingurl;
		//if( g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAY )
		//if( g_psSystemConfig->sVideoPlayer.dwPlayerStatus == MOVIE_STATUS_PLAYER
		if( gmediastart )
		{
			dwSec = ( AudioGetPlayPTS() + 500 ) / 1000;
			if( dwSec >= g_dwTotalSeconds )
			{
				xpgStopAudio();
				gmediastart = 0;
				bPlay = 0;
				dwSec == g_dwTotalSeconds;
				AudioSetPlayPTS(0);
			}
		}
	}
	else
#endif
		trackuri = "";
	//dwSec = Get_AviVideoCurrentTime();
	sprintf(curtime,"00:%02d:%02d",dwSec/60,dwSec%60);
    if( UpnpAddToActionResponseExt( out, "GetPositionInfo",
                                 DmrServiceType[1],
								 8,
								 "Track","0",
								 "TrackDuration",duration,			//"00:00:00",
								 "TrackMetaData","NOT_IMPLEMENTED",
								 "TrackURI",trackuri,				//"",
								 "RelTime","00:00:00",
								 "AbsTime","00:00:00",
								 "RelCount","NOT_IMPLEMENTED",
								 "AbsCount","NOT_IMPLEMENTED"

								 ) != UPNP_E_SUCCESS ) 

	{
		( *out ) = NULL;
		( *errorString ) = "Kevin Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	return UPNP_E_SUCCESS;

}
int DMRAVT_SetAVTransportURI( IXML_Document * in, IXML_Document ** out, char **errorString )
{
	char *current_uri,*current_uri_metadata;
    IXML_Document *respDoc = NULL;
	char *p1,*p2,*p3,*p4;
	IXML_Node *childnode,*childchildnode;
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;
	unsigned char bclearuri = 0;

	bstreaming = 0;
	//printixmlpool();
	//mpDebugPrint("DMRAVT_SetAVTransportURI");
    if( !( current_uri = SampleUtil_GetFirstDocumentItem( in, "CurrentURI" ) ) ) {
		mpDebugPrint("Get current URI fail");
	}
	else
	{
		//mpDebugPrint("current_uri %s len %d",current_uri,strlen(current_uri));
		if( strlen(current_uri) < 5 )
		{
			mpDebugPrint("############# clear URI");
			bclearuri = 1;
			ixml_mem_free(current_uri);
			return;
		}
	}

    if( !( current_uri_metadata = SampleUtil_GetFirstDocumentItem( in, "CurrentURIMetaData" ) ) ) {
		mpDebugPrint("Get current_uri_metadata fail");
	}
	else
	{
		//printixmlpool();
		//mpDebugPrint("current_uri_metadata sucess");
		if( ixmlParseBufferEx( current_uri_metadata, &respDoc ) == IXML_INVALID_PARAMETER )
			mpDebugPrint("IXML_INVALID_PARAMETER");
		else
		{
			//mpDebugPrint("ixmlParseBufferEx sucess");
			//p1 =  SampleUtil_GetFirstDocumentItem( respDoc, "item id");
			p2 =  SampleUtil_GetFirstDocumentItem( respDoc, "res");
			//mpDebugPrint("p1 %s",p1);
			//ixml_mem_free(p1);
			//mpDebugPrint("p2 %s",p2);
			if( p2 )
			{
				ixml_mem_free(p2);
				//mpDebugPrint("get res");
				nodeList = ixmlDocument_getElementsByTagName( respDoc, "res" );
				if( nodeList ) {
					if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) ) {
						//mpDebugPrint("ixmlElement_getAttribute");
						p3 = ixmlElement_getAttribute(tmpNode,"protocolInfo");
						//mpDebugPrint("p3 %s",p3);
						if( strstr(p3,"image/jpeg") )
							bstreaming = 0;
						else
							bstreaming = 1;
						if( bstreaming )
						{
							p3 = ixmlElement_getAttribute(tmpNode,"duration");
							p4 = strstr(p3,".");
							if( p4 )
								*p4 = '\0';
							//mpDebugPrint("p3 %s",p3);
							strcpy(gduration,p3);
							if( gstreamingurl )
								ixml_mem_free(gstreamingurl);
							gstreamingurl = current_uri;
							//bstreaming = 1;
						}
						else
						{
							if( gavtransuri )
							{
								ixml_mem_free(gavtransuri);
								gavtransuri = NULL;
							}
							gavtransuri = current_uri;
						}
						//textNode = ixmlNode_getFirstChild( tmpNode );

						//ret = ixml_strdup( ixmlNode_getNodeValue( textNode ) );
					}
				}
				if( nodeList )
					ixmlNodeList_free( nodeList );

/*
				childnode = ixmlNode_getFirstChild( (IXML_Node *) (respDoc) );
				while( childnode )
				{
					childnode = ixmlNode_getNextSibling(childnode);
					p3 = ixmlNode_getNodeValue(childnode);
					mpDebugPrint("p3 %s",p3);
					if( childnode )
					{
						childchildnode = ixmlNode_getFirstChild( childnode );
						p3 = ixmlNode_getNodeValue(childnode);
						mpDebugPrint("p3a %s",p3);

					}
				}
*/

			}
			//p2 =  SampleUtil_GetFirstDocumentItem( respDoc, "protocolInfo");
			//p3 =  SampleUtil_GetFirstDocumentItem( respDoc, "duration");
			//mpDebugPrint("p1 %s p2 %s",p1,p2);
			//mpDebugPrint("free respDoc");
		    ixmlDocument_free( respDoc );	
			//printixmlpool();
		}
		//mpDebugPrint("free current_uri_metadata");
		ixml_mem_free(current_uri_metadata);
		//printixmlpool();
	}
	//ixml_mem_free(current_uri);

#if 0
	if( bdlnadownload == 0 )
	{
		mpDebugPrint("bdlnadownload == 0");
		if( bstreaming )
		{
			xpgCb_MusicPlay();
		}
		else
		{
			gavtransuri = current_uri;
			bdlnadownload = 1;
			EventSet(INTERNET_RADIO_EVENT, 0x1);
		}
		//mpx_EventSet(UPNP_Server_Event, 0x1);
		//mpDebugPrint("END SET INTERNET_RADIO_EVENT");	
	}
	else
	{
		mpDebugPrint("Still in download process");
		ixml_mem_free(current_uri);
	}
#endif	
    if( UpnpAddToActionResponseExt( out, "SetAVTransportURI",
                                 DmrServiceType[1],
								 0
								 ) != UPNP_E_SUCCESS ) 

	{
		mpDebugPrint("DMRAVT_SetAVTransportURI respondse error");
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	//mpDebugPrint("DMRAVT_SetAVTransportURI UPNP_E_SUCCESS");
	//printixmlpool1();
	return UPNP_E_SUCCESS;

}


extern BYTE g_bVolumeIndex;
extern BOOL bSetUpChg;
int DMRRC_GetVolume( IXML_Document * in, IXML_Document ** out, char **errorString )
{
	char volume[8];
	//mpDebugPrint("DMRAVT_GetVolume %d",g_bVolumeIndex);
	if( g_bVolumeIndex << 2 > 100 )
		sprintf(volume,"%d",100);
	else
		sprintf(volume,"%d",g_bVolumeIndex<<2);
    if( UpnpAddToActionResponseExt( out, "GetVolume",
                                 DmrServiceType[1],
								 1,
								 "CurrentVolume",volume//dmr_service_table[2].VariableStrVal[DMR_RC_VOLUME]
								 ) != UPNP_E_SUCCESS ) 

	{
		mpDebugPrint("DMRAVT_Stop respondse error");
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	//mpDebugPrint("DMRAVT_Stop UPNP_E_SUCCESS");
	return UPNP_E_SUCCESS;
}

int DMRRC_SetVolume( IXML_Document * in, IXML_Document ** out, char **errorString )
{
	//mpDebugPrint("DMRAVT_GetVolume %d",g_bVolumeIndex);
	char *desire_vol;
	char vol;
    if( !( desire_vol = SampleUtil_GetFirstDocumentItem( in, "DesiredVolume" ) ) ) {
        ( *errorString ) = "Invalid Args";
        return UPNP_E_INVALID_PARAM;
    }
	//mpDebugPrint("desire_vol %s",desire_vol);
	vol = (char ) strtol(desire_vol,NULL,10);
	//mpDebugPrint("########vol %d ",vol);
	if ((vol & 0x3f) < VOLUME_DEGREE)
	{
		if( vol == 100 )
			g_bVolumeIndex = 0x3f;
		else
			vol = (vol >> 1);
		g_bVolumeIndex = (vol & 0x3f);
	}
	if ((g_bVolumeIndex & 0x3f) > 0)
		g_bVolumeIndex &= ~0x80;
	MX6xx_AudioSetVolume(g_bVolumeIndex);
	//bSetUpChg = 1;				//Mason 11/20

	strcpy(dmr_service_table[2].VariableStrVal[DMR_RC_VOLUME],desire_vol);
	ixml_mem_free(desire_vol);


    if( UpnpAddToActionResponseExt( out, "SetVolume",
                                 DmrServiceType[1],
								 0
								 ) != UPNP_E_SUCCESS ) 

	{
		mpDebugPrint("DMRAVT_Stop respondse error");
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	mpDebugPrint("DMRRC_SetVolume UPNP_E_SUCCESS");
	return UPNP_E_SUCCESS;
}

int DMRRC_GetMute( IXML_Document * in, IXML_Document ** out, char **errorString )
{
	//char tmp[16];

	//sprintf(tmp,"%d",gMute);
	//mpDebugPrint("DMRRC_GetMute %s",dmr_service_table[2].VariableStrVal[DMR_RC_MUTE]);
    if( UpnpAddToActionResponseExt( out, "GetMute",
                                 DmrServiceType[2],
								 1,
								 "CurrentMute",dmr_service_table[2].VariableStrVal[DMR_RC_MUTE]
								 ) != UPNP_E_SUCCESS ) 

	{
		mpDebugPrint("DMRRC_GetMute respondse error");
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	//mpDebugPrint("DMRRC_GetMute UPNP_E_SUCCESS");
	return UPNP_E_SUCCESS;
}


int DMRRC_SetMute( IXML_Document * in, IXML_Document ** out, char **errorString )
{
	char *desire_mute;
    if( !( desire_mute = SampleUtil_GetFirstDocumentItem( in, "DesiredMute" ) ) ) {
        ( *errorString ) = "Invalid Args";
        return UPNP_E_INVALID_PARAM;
    }
	//mpDebugPrint("desire_mute %s",desire_mute);
	gMute = (char) strtol(desire_mute,NULL,10);
	//mpDebugPrint("gMute %d ",gMute);
	strcpy(dmr_service_table[2].VariableStrVal[DMR_RC_MUTE],desire_mute);
	ixml_mem_free(desire_mute);
	//mpDebugPrint("DMRAVT_GetVolume %d",g_bVolumeIndex);
    if( UpnpAddToActionResponseExt( out, "SetMute",
                                 DmrServiceType[2],
								 0
								 ) != UPNP_E_SUCCESS ) 

	{
		mpDebugPrint("DMRAVT_SetMute respondse error");
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	//mpDebugPrint("DMRAVT_SetMute UPNP_E_SUCCESS");
	return UPNP_E_SUCCESS;
}
