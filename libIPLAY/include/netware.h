
#ifndef __NET_WARE__H__
#define __NET_WARE__H__

#include "bitsDefine.h"
#include "global612.h"
#include "mpTrace.h"
#include "Net_api.h"
#include "..\..\libIPLAY\libSrc\Xml\INCLUDE\Netfs_pri.h"	//Gino added at 081114 for upnp

#if HAVE_CURL
  //#define IMAGE_BUF 1024*1024 //for  preload file
#if NET_UPNP
	#define IMAGE_BUF (128*1024) //4096      //for  preload file
#else
  #define IMAGE_BUF 8192 //4096      //for  preload file
#endif
#else
  #define IMAGE_BUF 4096      //for  preload file
#endif

// define event for NETWARE_EVENT
#define EVENT_NW_INIT				BIT0		// init nerware system
#define EVENT_NW_PHYSICAL			BIT1		// from physical layer
#define EVENT_NW_GETIP				BIT2		// get IP address by dhcp
#define EVENT_NW_SETIP				BIT3		// get IP address by setting (fixed ip)
#define EVENT_NW_DISABLE			BIT4		// stop all netware action
#define EVENT_NW_RX     				BIT5		// Rescive all netware action
#define EVENT_NW_PERIODIC_TIMER 	BIT6
#define EVENT_NW_ARP_TIMER    		BIT7
#define EVENT_AP_SEARCH	    		BIT8		// start wifi ap search
#define EVENT_FILE_CACHE    			BIT9		// for net file cache
#define EVENT_NW_TIMER    			BIT10		// network timer
#define EVENT_NW_ENABLE			BIT11		// stop all netware action
#define EVENT_LINKSTATE			BIT12		// L1 link state
#define EVENT_NW_INTERFACE		BIT13		// network interface status
#define EVENT_NW_DRIVER_UP		BIT14		// network interface status
#define EVENT_NW_SCAN_DONE		BIT15
#define EVENT_NW_CONNECT_AP		BIT16	
#define EVENT_NW_DISCONNECT_AP	BIT17	
#define EVENT_NW_SCAN_REQ			BIT18
#define EVENT_NET_CONFIGURED     BIT19
#define EVENT_NW_CONFIG_CHANGED     BIT20

// define net ware setup structure
typedef struct {
	BYTE	EncryptionMode;		// disable, WEP, WPA, WPA2
	BYTE	ConnectMode;		// Infrastructure, Ad Hoc
	BYTE	Configuration;		// DHCP, fixed IP
	BYTE	HostIP[4];
	BYTE	NetMask[4];
	BYTE	Gateway[4];
	BYTE	DNSAddr[4];
} ST_NET_SETTING;


//////////////////////////////////////////////////////////////////
////     define for WiFi AP 
#define MAX_SSID_LENG		128
#define MAX_KEY_LENG		32
#define MAX_NUM_WIFI_AP		32

//===============================================================
//Note: added by CJ for Wifi Security 030608
//
//        Security            Privacy  Wpa_supplicant.Wpa_ie[0]  Wpa_supplicant.Wpa2_ie[0]
//    No Security                  0          0                                        0
//    WEP/WEP CKIP            1          0                                        0
//    WPA TKIP                    1          221                                     0
//    WPA2 AES-CCMP         1          221                                     48 
//
#define WIFI_NO_SECURITY 0
#define WIFI_WEP                1
#define WIFI_WPA                2
#define WIFI_WPA2              3

/**
 * @ingroup WlanScan
 * @brief Stores the information of a wireless AP.
*/
typedef struct Get_APINFO_tag{
	DWORD Channel;              /**< Wireless channel 1-14 */
	DWORD Privacy;              /**< True if security mode is enabled */
	DWORD Rssi;                 /**< Receive signal strength */
	BYTE  Ssid[MAX_SSID_LENG];  /**< ESSID */
	BYTE  SsidLen;              /**< Length of ESSID */
	BYTE  Key[MAX_KEY_LENG];    /**< not used */
	BYTE  MAC[6];               /**< BSSID */
	BYTE  Wpa[256];             /**< not used */
	BYTE  WpaLen;               /**< not used */
	BYTE  Wpa2[256];	        /**< not used */
	BYTE  Wpa2Len;	            /**< not used */
	BYTE  Security;	            /**< One of WIFI_NO_SECURITY, WIFI_WEP, WIFI_WPA, WIFI_WPA2 */
	BYTE  KeyIndex;             /**< not used */
	BYTE  KeyLength;            /**< not used */
	BYTE  keyready;             /**< not used */
	BYTE  AuthMode[32];         /**< An ASCII string of the name of the security mode */
  //BYTE  InfrastructureMode;
  //BYTE  NetworkTypeInUse;
    DWORD  Mode;
} WIFI_APINFO;


/**
 * @ingroup WlanScan
 * @brief The data structure that holds the results of wireless scan (or site
 * survey)
 *
 * This data structure is mainly used by the UI subsystem.
*/
typedef struct {
	DWORD 		dwNumberOfAP;			/**< Number of wireless APs found */
	DWORD 		dwCurrentAP;			/**< Selected AP (used by UI only) */
//	DWORD 		dwFirstList;			/**< The first list index (used by UI only) */
//	DWORD 		dwListIndex;			/**< The list offset from first list (used by UI only) */
	DWORD		dwState;			    /**< not used */
	WIFI_APINFO WiFiAPList[MAX_NUM_WIFI_AP];			    /**< list of APs found */
} WIFI_AP_BROWSER;


#define MAX_NET_NAME_LEN	256
#define MAX_NET_LINK_LEN	256
#define MAX_NET_FILE_NUM	800
#define MAX_SETID       		64
#define MAX_TITLE       64

typedef struct {
	BYTE Name[MAX_NET_NAME_LEN];
	BYTE Link[MAX_NET_LINK_LEN];
	DWORD dwIndex;
	DWORD dwMediaInfo;
	DWORD size;
	BYTE ExtName[3];
	BYTE state;
} ST_NET_FILEENTRY;

struct info_mem {
	struct info_mem *next;
    void *mem;
};

typedef struct {
	DWORD		dwState;
	DWORD 		dwNumberOfFile;		// number of file
	DWORD 		dwCurrentFile;		// The selected file
#if 0
	DWORD 		dwFirstList;		// The first list index
	DWORD 		dwListIndex;		// The list offset from first list
	DWORD 		dwListCount;		// The number of list in page
	DWORD 		dwLineCount;		// The number of list per line
#endif
	void *FileEntry;
	ST_NET_FILEENTRY *FileInfoStart, *FileInfoEnd;
	struct info_mem mem_allocated;
} ST_NET_FILEBROWSER;

enum RSS_STATE
{   
    	RSS_NULL,
    	RSS_START,
    	RSS_CHANNELs,
    	RSS_TITLE,
    	RSS_DESC,
    	RSS_ITEM, 
    	RSS_ITEM_LINK,
    	RSS_ITEM_SIZE,
    	RSS_ITEM_CHANNEL,
    	RSS_ITEM_PUBDATE,
    	RSS_ITEM_TITLE,
    	RSS_ITEM_DESCRIPTION,
    	RSS_TITLE_CONTENT,
    	RSS_DESC_CONTENT,
    	RSS_DESC_IMG,
    	RSS_DESC_IMG_SRC,
};

#define RSS_ENCODING_UNKNOW          0
#define RSS_ENCODING_UTF8                1
#define RSS_ENCODING_BIG5                2
#define RSS_ENCODING_ISO                  3

#define MAX_TAG_NUM		       128
#define MAX_TAG_LEN                    256
#define RSS_Channel_Max		10

#define Flickr_list_Max			10
#define Picasa_list_Max		10
#define YouGotPhoto_list_Max			10
#define XML_SIZE  20*1024
#define PHOTOSET_list_Max	128
#define InternetRadio_list_Max	128
#define FrameChannel_list_Max	10
#define FrameIt_list_Max	10
#define Vtuner_list_Max	10
#define Vtuner_location_list_Max	32
#define Photo_list_Max		10
#define Snapfish_list_Max	10

typedef struct NETFS_RSSINFO   
{
     	BYTE   *pbUserdefine; 
	BYTE   *pbRSS_Feed;
	BYTE   RSSTag[MAX_TAG_NUM][MAX_TAG_LEN];	
	BYTE   RSSImage[MAX_TAG_NUM][MAX_TAG_LEN];
	
/*	BYTE RSSTag[MAX_TAG_NUM][MAX_TAG_LEN];	
	BYTE RSSTitle[MAX_TAG_NUM][MAX_TAG_LEN];	
	BYTE RSSDescription[MAX_TAG_NUM][MAX_TAG_LEN];	
	BYTE RSSImage[MAX_TAG_NUM][MAX_TAG_LEN];	
*/
}RSSINFO_t;

typedef struct
{
	BYTE* pTitle; //pbUserdefine;
	BYTE* pLink; //pbEntry 
}INFO_t;

typedef struct
{
	DWORD   dwTotalChannel;		// number of Channel found
	DWORD   dwCurChannel;		// The selected Channel
#if 0	
	DWORD   dwFirstchannel;      	// The first channel index
	DWORD   dwListchannelIndex;	// The list offset from first channel
#endif	
	DWORD   dwTotalTitle;	    	// number of Title found
	DWORD   dwCurTitle;		       // The selected title
#if 0	
	DWORD   dwFirstTitle;      		// The first title index
	DWORD   dwListTitleIndex;	// The list offset from first title
#endif	
       //BYTE   bEncodeType;
	//BYTE   bState; 
	RSSINFO_t RSSFeed[RSS_Channel_Max];//MAX_RSS_channel];
    //INFO_t RSSFeed[RSS_Channel_Max];//MAX_RSS_channel]; 	  
} RSS_CHANNEL_LIST;

typedef struct
{
	DWORD dwCount;
	DWORD dwIndex;
	ST_FEED *pFeedArry;
	
//	INFO_t * pFeedArry;
} ST_USER_LIST;

typedef struct _set_info
{
    	BYTE     id[MAX_SETID];
    	BYTE    title[MAX_TITLE];
		DWORD	streamtype;
}SET_INFO_t;

typedef struct
{
	BYTE dwCount;
	BYTE dwIndex;
	BYTE Location_dwCount;
	BYTE Location_dwIndex;
	BYTE Location_List_dwCount;
	BYTE Location_List_dwIndex;

} VTUNER_USER_LIST;

typedef struct  _WIFI_PHOTOSET_BROWSER{
	DWORD 		dwNumberOfSet;			// number of Set found
	DWORD 		dwCurrentIndex;			// The selected Set
//	DWORD 		dwFirstList;				// The first list index
//	DWORD 		dwListIndex;			// The list offset from first list
	DWORD		dwState;
	SET_INFO_t    Photoset[PHOTOSET_list_Max];
} WIFI_PHOTOSET_BROWSER;

typedef struct 
{
	BYTE func[64];
	BYTE user[64];
	BYTE photoset[64];	
}XPG_CURDIRECT;

typedef struct {

	BYTE SSID[128];
	BYTE MAC[32];
	BYTE IP[32];
	BYTE SubMask[32];
	BYTE DefaultGetway[32];
	BYTE DNS[2][32];
	BYTE OK;
	
}NetWorkInfo;

typedef struct {
	DWORD dwTotalFile;
	DWORD dwCurrentIndex;
	BYTE bListIndex;
	BYTE bTreeLevel;
	netfs_meta_entry_t *pt_FirstEntry;
	netfs_meta_entry_t *pt_CurEntry;		//Doesn't use in the current state
	netfs_meta_entry_t *pt_ListFirstEntry;
	netfs_meta_entry_t *pt_TreeFirstEntry;
}Upnp_file_list_t;

//#if NETWARE_ENABLE

typedef struct{
	BYTE bStatusFlag;
	BYTE *page_name;	//char page_name[21];
	BYTE bName_len;
	void (*hCommand)(void);
}ST_NET_ACTFUNC;

#if 0
typedef struct{
	BYTE bStatusFlag;
	DWORD (*hGetTotalIndex)(void);
	DWORD (*hGetCurrentIndex)(void);
	void (*hSetListIndex)(DWORD);
	void (*hSetFirstListIndex)(DWORD);
	void (*hSetCurrentIndex)(DWORD);
	
}ST_NET_BTN;
#endif
//#endif

#endif

