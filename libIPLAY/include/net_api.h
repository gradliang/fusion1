#ifndef __net_api_h__
#define __net_api_h__

//#include "netware.h"

#define NETFUNCMAXLEN 		20
#define RSS_Channel_name	64//16//32 //64
#define Feed_len 128

enum
{
    NETFUNC_PC = 0,           
    NETFUNC_RSS,          
    NETFUNC_FLICKR,		 
    NETFUNC_PICASA,    	 
    NETFUNC_YOUGOTPHOTO, 
    NETFUNC_INTERNETRADIO, 
    NETFUNC_FRAMECHANNEL, 
    NETFUNC_VTUNER, 
    NETFUNC_UPNP,
    NETFUNC_FRAMEIT, 
    NETFUNC_SNAPFISH, 
#if HAVE_SHUTTERFLY
    NETFUNC_SHUTTERFLY, 
#endif
    NETFUNC_UPNP_SERVER,
    NETFUNC_YOUTUBE_THUMB,
    NETFUNC_YOUTUBE_FAVOR,
    NETFUNC_YAHOOKIMO,
    NETFUNCNUM,

    // upper must match with the ui. 

    // other state
    
    NETFUNC_VTUNER_LOCATION,

};

enum
{
    NET_EDIT_NEW = 0,           
    NET_EDIT_RENAME,          
    NET_EDIT_DELETE,		 
    NET_EDIT_DELETEALL,    
    NET_EDIT_RSS_LIST,
    NET_EDIT_RSS_LINK,
    NET_EDIT_RSS_RENAME_LIST,     
    NET_EDIT_RSS_RENAME_LINK,
    NET_EDIT_RSS_DELETE,		 
    NET_EDIT_RSS_DELETEALL,  
    NET_EDIT_MAX,
};

enum
{
    NET_STATUS_NULL = 0,    
    NET_STATUS_NOREGIST,           
    NET_STATUS_NOPICTURE_GOTO_YOUGOTPHOTO,          
    NET_STATUS_SEARCH,		 
    NET_STATUS_CONNECTING,    
    NET_STATUS_SUCCESS_CONNECT,
    NET_STATUS_NOPICTURE,
    NET_STATUS_FAILKEY,   
    NET_STATUS_FAILCONNECT,     
    NET_STATUS_WEAK_RECONNECT,
    NET_STATUS_NO_PICTURE,     
    NET_STATUS_NO_REGISTER,	
    NET_STATUS_MAX,
};

typedef struct 
{
	DWORD dwCurrentIndex;
	DWORD dwTotal;
#if 0
	DWORD dwFirstIndex;
	DWORD dwListIndex;
	DWORD dwNumber;
	DWORD dwCurrentIndex;	
	BYTE      bNETFUNCLIST[NETFUNCNUM];
#endif
}NETFUNC_t;

#define FEED_TITLE_LEN		64//16//32 //64
#define FEED_LINK_LEN		128//64//16//64//256

enum Net_List_type{
	none_type,
	RSS_type,
	Flickr_type,
};

typedef struct {
	BYTE * pTitle;
	BYTE * pLink;
} ST_FEED;

typedef struct {
	BYTE bUserdefine[RSS_Channel_name];
	BYTE bRSS_Feed[Feed_len];
} RSS_FEED;

void Net_SetupList_NEW();
void Net_SetupList_RENAME();
void Net_SetupList_DELETE();
void Net_SetupList_DELETEALL();

enum
{
	Setup_NOKEY = 0,
	Setup_WEP = 1,
	Setup_WPA = 2,
	Setup_WPA2= 3,
};

enum
{	
	Setup_OFF = 0,
	Setup_ON,	
	Setup_ASCII,
	Setup_HEX,	
	Setup_64bit,
	Setup_128bit,
};

enum
{
	Setup_SSID_X = 345,
	Setup_SSID_Y = 	160,
	Setup_KEY_X = 345,
	Setup_KEY_Y = 286,
};

enum
{
       WirelessNetworkSetup_ITEM_NetName = 0,
	WirelessNetworkSetup_ITEM_WEP = 1,	
	WirelessNetworkSetup_ITEM_NWKey = 2,	
	WirelessNetworkSetup_ITEM_KeyLegth = 3,
	WirelessNetworkSetup_ITEM_Key = 4,
	WirelessNetworkSetup_ITEM_TOTAL = 5,
   	WirelessNetworkSetup_ITEM_KeyASCII = 5,	
	WirelessNetworkSetup_ITEM_KeyHex = 6,
	WirelessNetworkSetup_ITEM_KeyLegth64bit = 7,
	WirelessNetworkSetup_ITEM_KeyLegth128bit = 8,	
	WirelessNetworkSetup_ITEM_WPA = 9,
    WirelessNetworkSetup_ITEM_WPS = 10,
};

enum
{
	KeyPadText_IP = 0,
	KeyPadText_Default_Getway,
	KeyPadText_SubNet_Mask,
	KeyPadText_DNS_Server,
	KeyPadText_DHCPSetupTOTAL,
};

enum
{
	DHCP_STATUS_CONNECTING = 0,  
	DHCP_STATUS_SUCCESS_CONNECT,
    	DHCP_STATUS_TOTAL,
};

/*
enum
{
	DHCP_IP_X = 256,
	DHCP_IP_Y = 221,
	DHCP_GATEWAY_X = 557,
	DHCP_GATEWAY_Y = 221,
	DHCP_SUBMASK_X = 256,
	DHCP_SUBMASK_Y = 	253,
	DHCP_DNS_X = 557,
	DHCP_DNS_Y = 253,
};*/
enum
{
	DHCP_IP_X = 256,
	DHCP_IP_Y = 170,
	DHCP_GATEWAY_X = 557,
	DHCP_GATEWAY_Y = 170,
	DHCP_SUBMASK_X = 256,
	DHCP_SUBMASK_Y = 198,
	DHCP_DNS_X = 557,
	DHCP_DNS_Y = 198,
};

enum
{
	DHCPSetup_IPAddressAuto =0,
	DHCPSetup_IPAddress = 1,
	DHCPSetup_DefaufGateway =2,
	DHCPSetup_SubnetMask = 3,
	DHCPSetup_DNSServer = 4,
	DHCPSetup_Total = 5,
	DHCPSetup_ITEM_On = 5,
	DHCPSetup_ITEM_Off = 6,	
};	

enum
{
	USEREDIT_NEWNAME = 0,
	USEREDIT_RENAME = 1,
	USEREDIT_DELNAME = 2,
	USEREDIT_DELALL = 3,
};
/*
enum
{
	USEREDIT_USERNAME = 0,
	USEREDIT_RSSFEEDS = 1,	
};
*/
enum
{
	USEREDIT_USER= 0,
	USEREDIT_RSSFEED = 1,
	USEREDIT_EMAILADDRESS = 2,
	USEREDIT_PASSWORD = 3,	
};

enum
{
	USEREDIT_KEYEDIT0 = 0,
	USEREDIT_KEYEDIT1 = 1,
	USEREDIT_KEYEDIT2 = 2,
	USEREDIT_KEYEDIT3 = 3,
	
};
enum
{
	WSC_BUTTON_UP =  0,
	WSC_BUTTON_DOWN =	1,
	WSC_BUTTON_OK =	2,
	WSC_BUTTON_NEXT = 3,
	WSC_BUTTON_BACK = 4,
};	
enum
{
	WSC_IDLE=  0,
	WSC_PBC_START =  1,
	WSC_PIN_START =  2,
	WSC_RECV_CRED =  3,
};	

enum
{
	WIFI_MODE_INFRASTRUCTURE =  0,
	WIFI_MODE_IBSS =  1,
};	

typedef struct _WNSetup
{
	BYTE SSID[128];
	DWORD Ecrypted;	
	DWORD WNKey;	
	DWORD KEY_Length;
	DWORD KEY_Num;
	BYTE Key[128];
	
	DWORD DHCP_IPAddessAuto;
	BYTE IP[32];
	BYTE GateWay[32];
	BYTE SubMask[32];
	BYTE DNS[32];	
}WNSetup;

typedef struct _SNAPFISH_PreSet
{
	BYTE Email[128];
	BYTE Password[64];
	
}SF_PreSet;

typedef struct _WirelessNetworkSetup
{
	BYTE Press;
	BYTE Connect;
	BYTE DHCP;
	BYTE AutoNetSearch[128]; 
	BYTE PtoPconfig[128];      //stage 8 PtoPconfig
	BYTE SDCardConfig;
	WNSetup ManualSetup;        //stage 13 MainMenu
	//BYTE ConnectionStatus;
	//BYTE WirelessRestart;
	SF_PreSet SF;
#if Make_ADHOC
	DWORD Mode;
	DWORD Channel;
#endif

	
}WirelessNetworkSetup;

WirelessNetworkSetup WirelessNetworkSetupITEM;


DWORD Net_RSS_GetTotalIndex();
DWORD Net_RSS_GetCurrentIndex();
void Net_RSS_SetCurrentIndex(DWORD index);

DWORD Net_PhotoSet_GetCount();
DWORD Net_PhotoSet_GetCurrentindex();
void Net_PhotoSet_SetCurrentindex(DWORD index);

#if 0
DWORD Net_Flickr_GetTotalIndex();
DWORD Net_Flickr_GetCurrentIndex();
void Net_Flickr_SetCurrentIndex(DWORD index);

DWORD Net_Picasa_GetTotalIndex();
DWORD Net_Picasa_GetCurrentIndex();
void Net_Picasa_SetCurrentIndex(DWORD index);

DWORD Net_FrameIt_GetTotalIndex();
DWORD Net_FrameIt_GetCurrentIndex();
void Net_FrameIt_SetCurrentIndex(DWORD index);

DWORD Net_Ygp_GetTotalIndex();
DWORD Net_Ygp_GetCurrentIndex();
void Net_Ygp_SetCurrentIndex(DWORD index);

DWORD Net_InternetRadio_GetTotalIndex();
DWORD Net_InternetRadio_GetCurrentIndex();
void Net_InternetRadio_SetCurrentIndex(DWORD index);

DWORD Net_FrameChannel_GetTotalIndex();
DWORD Net_FrameChannel_GetCurrentIndex();
void Net_FrameChannel_SetCurrentIndex(DWORD index);
#endif

DWORD Net_Vtuner_GetTotalIndex();
DWORD Net_Vtuner_GetCurrentIndex();
void Net_Vtuner_SetCurrentIndex(DWORD index);

#if 0
DWORD Net_Snapfish_GetTotalIndex();
DWORD Net_Snapfish_GetCurrentIndex();
void Net_Snapfish_SetCurrentIndex(DWORD index);
#endif

#endif //__net_api_h__

