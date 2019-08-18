
/******************************************************************************
* rt73_mp.c                                                                                                                                 *
*                                                                                                                                          *
* Description :                                                                                                                       *
*                                                                                                                                           *
* Author :                                                                                                                       *
*                                                                                                                                         *
* History :                                                          
*                                                                                                                                          *
*******************************************************************************/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1
/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "netware.h"
//#include "mlme.h"
#include "rt_config.h"
//#include "osdep_mpixel_service.h"
//#include    "..\..\libIPLAY\libsrc\lwip\include\os_mp52x.h"
extern WIFI_AP_BROWSER WiFiApBrowser;
void MP_Wifi_APInfo(BSS_TABLE *Tab)
{
    u8 ireg = 0;
    int i,j;
    int len;
    int wsc = 0;

       i = Tab->BssNr;
	if(WiFiApBrowser.dwNumberOfAP >= MAX_NUM_WIFI_AP)
	{
		MP_DEBUG("AP dwNumberOfAP >= MAX_NUM_WIFI_AP");
		return;
	}		
/*
    	i =0;
	 	
	while( i < WiFiApBrowser.dwNumberOfAP)
	{
		if(memcmp(WiFiApBrowser.WiFiAPList[i].MAC,Tab->BssEntry[Tab->BssNr].Bssid, 6) == 0)
			return;                             
		else
		{
            		i++;
		}
	}

	MP_DEBUG("===================================================");
*/
    if(Tab->BssEntry[i].SsidLen== 0)
    {
        strcpy(WiFiApBrowser.WiFiAPList[i].Ssid,"hidden SSID");     
        //MP_DEBUG1("SSid = %s\r\n",WiFiApBrowser.WiFiAPList[i].Ssid);
    }	
    else
    {
        len = Tab->BssEntry[i].SsidLen;
        if(len > 32)
            len = 32;
        memcpy(WiFiApBrowser.WiFiAPList[i].Ssid,Tab->BssEntry[i].Ssid, len);     

	if (len < sizeof(WiFiApBrowser.WiFiAPList[i].Ssid))
            WiFiApBrowser.WiFiAPList[i].Ssid[len] = '\0';
        //MP_DEBUG1("SSid = %s\r\n",WiFiApBrowser.WiFiAPList[i].Ssid);
    }
    WiFiApBrowser.WiFiAPList[i].Privacy = Tab->BssEntry[i].Privacy;
    WiFiApBrowser.WiFiAPList[i].Rssi = Tab->BssEntry[i].Rssi; 
    WiFiApBrowser.WiFiAPList[i].Channel = Tab->BssEntry[i].Channel;
	
    memcpy(WiFiApBrowser.WiFiAPList[i].MAC, Tab->BssEntry[i].Bssid, 6);
    memset(WiFiApBrowser.WiFiAPList[i].AuthMode,0x00,32);	

	
    switch(Tab->BssEntry[i].AuthMode)
    {
	  case Ndis802_11AuthModeOpen:
			
			if(WiFiApBrowser.WiFiAPList[i].Privacy == 0)
			{
			    if(wsc)
					memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"Open NoKey(WPS)",strlen("Open NoKey(WPS)"));
				else
				memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"Open NoKey",strlen("Open NoKey"));
				WiFiApBrowser.WiFiAPList[i].Security = WIFI_NO_SECURITY;
			}
			else
			{
			
				if(wsc)
					memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"Open WEP(WPS)",strlen("Open WEP(WPS)"));
				else
				memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"Open WEP",strlen("Open WEP"));
				WiFiApBrowser.WiFiAPList[i].Security = WIFI_WEP;
			}
			break;
			
	  case Ndis802_11AuthModeShared:
	  		if(WiFiApBrowser.WiFiAPList[i].Privacy == 0)
	  		{
	  			if(wsc)
	  				memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"Shared Nokey(WPS)",strlen("Shared NoKey(WPS)"));
				else
	  			memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"Shared Nokey",strlen("Shared NoKey"));
				WiFiApBrowser.WiFiAPList[i].Security = WIFI_NO_SECURITY;
	  		}
			else
			{
			    if(wsc)
				   memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"Shared WEP(WPS)",strlen("Shared WEP(WPS)"));
				else
				memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"Shared WEP",strlen("Shared WEP"));
				WiFiApBrowser.WiFiAPList[i].Security = WIFI_WEP;
			}
			
			break;
			
	  case Ndis802_11AuthModeAutoSwitch:
	  		if(wsc)
	  			memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"AutoSwitch(WPS)",strlen("AutoSwitch(WPS)"));
			else
	  		memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"AutoSwitch",strlen("AutoSwitch"));
			WiFiApBrowser.WiFiAPList[i].Security = WIFI_WEP;
			
			break;
			
	  case Ndis802_11AuthModeWPA:
	 		if(wsc) 	
	  			memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"WPA(WPS)",strlen("WPA(WPS)"));
			else
	  		memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"WPA",strlen("WPA"));
				
			WiFiApBrowser.WiFiAPList[i].Security = WIFI_WPA;
			
	  	       break;
			 
    	  case Ndis802_11AuthModeWPAPSK:
		  	if(wsc)
		  		memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"WPA PSK(WPS)",strlen("WPA PSK(WPS)"));
			else
		  	memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"WPA PSK",strlen("WPA PSK"));
			WiFiApBrowser.WiFiAPList[i].Security = WIFI_WPA;
			
		  	break;
			
    	  case Ndis802_11AuthModeWPANone:
		  	if(wsc)
		  		memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"WPA None(WPS)",strlen("WPA None(WPS)"));	
			else
		  	memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"WPA None",strlen("WPA None"));	
			WiFiApBrowser.WiFiAPList[i].Security = WIFI_WPA;
			
		  	break;
			
    	  case Ndis802_11AuthModeWPA2:
		  	if(wsc)
		  		memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"WPA2(WPS)",strlen("WPA2(WPS)"));
			else
		  	memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"WPA2",strlen("WPA2"));
			WiFiApBrowser.WiFiAPList[i].Security = WIFI_WPA2;
			
		  	break;
			
    	  case Ndis802_11AuthModeWPA2PSK: 
		  	if(wsc)
		  		memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"WPA2 PSK(WPS)",strlen("WPA2 PSK(WPS)"));
			else
		  	memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"WPA2 PSK",strlen("WPA2 PSK"));
			WiFiApBrowser.WiFiAPList[i].Security = WIFI_WPA2;
			
			break;
			
	  default:
	 		memcpy(WiFiApBrowser.WiFiAPList[i].AuthMode,"Non-Know",strlen("Non-Know"));
			WiFiApBrowser.WiFiAPList[i].Security = WIFI_NO_SECURITY;
	 		break;

    }	
	
	MP_DEBUG6("AP Mac= %02x:%02x:%02x:%02x:%02x:%02x",WiFiApBrowser.WiFiAPList[i].MAC[0], 
											  WiFiApBrowser.WiFiAPList[i].MAC[1], 
											  WiFiApBrowser.WiFiAPList[i].MAC[2], 
											  WiFiApBrowser.WiFiAPList[i].MAC[3], 
											  WiFiApBrowser.WiFiAPList[i].MAC[4], 
											  WiFiApBrowser.WiFiAPList[i].MAC[5]);	
	
	MP_DEBUG4("%s,%x,%x,%x",WiFiApBrowser.WiFiAPList[i].Ssid, 
						     	   WiFiApBrowser.WiFiAPList[i].Privacy,
						          WiFiApBrowser.WiFiAPList[i].Rssi,
						          WiFiApBrowser.dwNumberOfAP);
	MP_DEBUG1("security =%s",WiFiApBrowser.WiFiAPList[i].AuthMode);
	MP_DEBUG1("Channel=%d",WiFiApBrowser.WiFiAPList[i].Channel);

	WiFiApBrowser.dwNumberOfAP++;

}
#if Make_USB == RALINK_WIFI
void MP_Wifi_APInfo_init()
{
	int i;

	WiFiApBrowser.dwNumberOfAP = 0;
	WiFiApBrowser.dwCurrentAP = 0;
	WiFiApBrowser.dwFirstList = 0;
	WiFiApBrowser.dwListIndex = 0;
	WiFiApBrowser.dwState = 0;

	for(i = 0; i < MAX_NUM_WIFI_AP ;i++)
	{
		WiFiApBrowser.WiFiAPList[i].Channel = 0;
		WiFiApBrowser.WiFiAPList[i].Privacy  = 0;
		WiFiApBrowser.WiFiAPList[i].Rssi = 0;
		memset(WiFiApBrowser.WiFiAPList[i].Ssid,0x00,MAX_SSID_LENG);
		memset(WiFiApBrowser.WiFiAPList[i].Key,0x00,MAX_KEY_LENG);
		memset(WiFiApBrowser.WiFiAPList[i].MAC,0x00,6);
	}
};
#endif





























