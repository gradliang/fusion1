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
#include <stdio.h>
#include "sample_util.h"
#include "upnp_tv_ctrlpt.h"
#include <string.h>
//#include <pthread.h>

#include "..\..\..\..\libIPLAY\libSrc\LWIP\INCLUDE\net_autosearch.h"
#include "global612.h"
#include "ui.h"
#include "taskid.h"
//extern Flickr_CHANNEL_LIST g_Flickr_CHANNEL_LIST;
char *ixml_mem_block = NULL;
extern BYTE g_bXpgStatus;
extern char *Search_DeviceType;
//#include "netfs.h"
//#include "upnpfs.h"
#define UPNP_PRINT_MSG 0
unsigned char DeviceSearchType = 0;
extern unsigned char NatDeviceCout;
extern char *upnpcurdir;
extern unsigned char bupnpdownload;
extern int UpnpSdkInit;
/*
   Tags for valid commands issued at the command prompt 
 */
enum cmdloop_tvcmds {
    PRTHELP = 0, PRTFULLHELP, POWON, POWOFF,
    SETCHAN, SETVOL, SETCOL, SETTINT, SETCONT, SETBRT,
    CTRLACTION, PICTACTION, CTRLGETVAR, PICTGETVAR,
    PRTDEV, LSTDEV, REFRESH, EXITCMD,STOPSSDP
};

/*
   Data structure for parsing commands from the command line 
 */
struct cmdloop_commands {
    char *str;                  // the string 
    int cmdnum;                 // the command
    int numargs;                // the number of arguments
    char *args;                 // the args
} cmdloop_commands;

/*
   Mappings between command text names, command tag,
   and required command arguments for command line
   commands 
 */
static struct cmdloop_commands cmdloop_cmdlist[] = {
    {"Help", PRTHELP, 1, ""},
    {"HelpFull", PRTFULLHELP, 1, ""},
    {"ListDev", LSTDEV, 1, ""},
    {"Refresh", REFRESH, 1, ""},
    {"PrintDev", PRTDEV, 2, "<devnum>"},
    {"PowerOn", POWON, 2, "<devnum>"},
    {"PowerOff", POWOFF, 2, "<devnum>"},
    {"SetChannel", SETCHAN, 3, "<devnum> <channel (int)>"},
    {"SetVolume", SETVOL, 3, "<devnum> <volume (int)>"},
    {"SetColor", SETCOL, 3, "<devnum> <color (int)>"},
    {"SetTint", SETTINT, 3, "<devnum> <tint (int)>"},
    {"SetContrast", SETCONT, 3, "<devnum> <contrast (int)>"},
    {"SetBrightness", SETBRT, 3, "<devnum> <brightness (int)>"},
    {"CtrlAction", CTRLACTION, 2, "<devnum> <action (string)>"},
    {"PictAction", PICTACTION, 2, "<devnum> <action (string)>"},
    {"CtrlGetVar", CTRLGETVAR, 2, "<devnum> <varname (string)>"},
    {"PictGetVar", PICTGETVAR, 2, "<devnum> <varname (string)>"},
    {"Exit", EXITCMD, 1, ""},
    {"StopSSDP", STOPSSDP, 2, "<devnum>"}
};

void
linux_print( const char *string )
{
#if 1//UPNP_PRINT_MSG
	mpDebugPrint(string);
#endif
    //K puts( string );
}

/********************************************************************************
 * TvCtrlPointPrintHelp
 *
 * Description: 
 *       Print help info for this application.
 ********************************************************************************/

/********************************************************************************
 * TvCtrlPointPrintCommands
 *
 * Description: 
 *       Print the list of valid command line commands to the user
 *
 * Parameters:
 *   None
 *
 ********************************************************************************/
void
TvCtrlPointPrintCommands(  )
{
    int i;
    int numofcmds = sizeof( cmdloop_cmdlist ) / sizeof( cmdloop_commands );

    SampleUtil_Print( "Valid Commands:" );
    for( i = 0; i < numofcmds; i++ ) {
        SampleUtil_Print( "  %-14s %s", cmdloop_cmdlist[i].str,
                          cmdloop_cmdlist[i].args );
    }
    SampleUtil_Print( "" );
}

/********************************************************************************
 * TvCtrlPointCommandLoop
 *
 * Description: 
 *       Function that receives commands from the user at the command prompt
 *       during the lifetime of the control point, and calls the appropriate
 *       functions for those commands.
 *
 * Parameters:
 *    None
 *
 ********************************************************************************/
/*
void *
TvCtrlPointCommandLoop( void *args )
{
	char *retstr;
    char cmdline[100];

    while( 1 ) {
		memset(cmdline,0,100);
        //SampleUtil_Print( "\n>> " );
		//fflush(stdin);
        retstr = fgets( cmdline, 100, stdin );
		if( retstr == NULL )
		{
			sleep(1);
			continue;
		}
		//sleep(3);
        TvCtrlPointProcessCommand( cmdline );
		//fflush(stdin);
		sleep(1);
    }

    return NULL;
}
*/
extern SERVER_BROWSER ServerBrowser;
int upnp_start( void )
{
    int rc;
    //K ithread_t cmdloop_thread;
    int code;
	int taskid;
	int i;

	/*for Upnp Device*/
    unsigned int portTemp = 0;
    char *ip_address = NULL,*desc_doc_name = NULL,*web_dir_path = NULL;
    unsigned int port = 0;


	ixml_mem_block = (char *) ext_mem_malloc(1600*1024);
	MP_DEBUG1("ixml_mem_block %x",ixml_mem_block);
	if( ixml_mem_block )
		ixml_mem_init(ixml_mem_block);
	else
		return -1;
	bupnpdownload = FALSE;
    SampleUtil_Initialize( linux_print );
	UPNP_Set_SearchType(0);
	/*Upnp Device Start*/
	mpDebugPrint("UpnpSdkInit %d",UpnpSdkInit);
    port = 49152;//( unsigned short )portTemp;
	UpnpSdkInit = 0;
    TvDeviceStart( ip_address, port, desc_doc_name, web_dir_path,
                   linux_print );
	mpDebugPrint("TvDeviceStart sucess");
	if( g_bXpgStatus != XPG_MODE_DLNA_1_5_DMR )
	{
		upnpcurdir = (char *) ixml_mem_malloc(2048);
		mpDebugPrint("g_bXpgStatus != XPG_MODE_DLNA_1_5_DMR");
		/*Init ServerBrowser struct*/
		ServerBrowser.bState = AS_STATE_INIT;
		ServerBrowser.dwNumberOfServer = 0;
		ServerBrowser.dwCurrentServer = 0;
		ServerBrowser.dwFirstList = 0;
		ServerBrowser.dwListIndex = 0;
		
		for(i=0; i<MAX_NUM_SERVER; i++)
		{
			ServerBrowser.ServerList[i].bState = NW_INVALID;
		}
		ServerBrowser.bState = AS_STATE_START;

		//K strcpy(upnp_map.base_dir,"UPNP Content DIR");
		//
		EventSet(UPNP_START_EVENT,0x100);
		TaskYield();
		rc = TvCtrlPointStart( linux_print, NULL );
		mpDebugPrint("UPNP TvCtrlPointStart END"); 

		if( rc != TV_SUCCESS ) {
			//SampleUtil_Print( "Error starting UPnP TV Control Point" );
			//exit( rc );
			mpDebugPrint("################Error starting UPnP TV Control Point##################");
		}
	}
    // start a command loop thread
    //K code = ithread_create( &cmdloop_thread, NULL, TvCtrlPointCommandLoop,NULL );
    /*
       Catch Ctrl-C and properly shutdown 
     */
    //K sigemptyset( &sigs_to_catch );
    //K sigaddset( &sigs_to_catch, SIGINT );
    //K sigwait( &sigs_to_catch, &sig );

    //K SampleUtil_Print( "Shutting down on signal %d...", sig );
/*
    rc = TvCtrlPointStop(  );
    exit( rc );
*/
}

int upnp_nat_t_start( void )
{
    int rc;
    //K ithread_t cmdloop_thread;
    int code;
	int taskid;
	int i;

	/*for Upnp Device*/
    unsigned int portTemp = 0;
    char *ip_address = NULL,*desc_doc_name = NULL,*web_dir_path = NULL;
    unsigned int port = 0;
	NatDeviceCout = 0;

	ixml_mem_block = (char *) ext_mem_malloc(1024*1024);
	MP_DEBUG1("ixml_mem_block %x",ixml_mem_block);
	if( ixml_mem_block )
		ixml_mem_init(ixml_mem_block);
	else
		return -1;

    SampleUtil_Initialize( linux_print );
	UPNP_Set_SearchType(1);

	EventSet(/*UPNP_REPLY_TASK_START*/NETWORK_STREAM_EVENT,0x100);
	//TaskYield();
	mpDebugPrint("UPNP NAT-T Client Start");
	rc = TvCtrlPointStart( linux_print, NULL );
	MP_DEBUG("UPNP TvCtrlPointStart END"); 

	if( rc != TV_SUCCESS ) {
		//SampleUtil_Print( "Error starting UPnP TV Control Point" );
		//exit( rc );
		mpDebugPrint("################Error starting UPnP TV Control Point##################");
	}
}
