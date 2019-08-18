/*-----------------------------------------------------------------------------------*/
/* net_init.c
 *
 * Several network related initialization routines.
 *
 */ 
/*-----------------------------------------------------------------------------------*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0


/*
// Include section 
*/
#include <stdlib.h>
#include <linux/types.h>
#include "global612.h"
#include "mpTrace.h"
#include "string.h" 

#include "linux/list.h"
#include "ndebug.h"
#include "dev_ath9k.h"

#define NETBUF_OVERHEAD 32

extern int HeapSemid;
extern void *net_buf_start;
extern void *net_buf_end;                              /* end of buffer pool */

int netpool_create(char *chunk, unsigned int nbufs, unsigned int bufsz);
int netpool_destroy(void);
void UsbOtgHostSetSwapBuffer2RangeDisable(WHICH_OTG eWhichOtg);
void http_proxy_config_get(void);
int mpx_ar9271_usb_module_init(unsigned int debug);

#if NETWARE_ENABLE

#if (Make_USB || PPP_ENABLE)
void USBDongleWifiInit(void)
{
    DRIVE *sDrv;

    ntrace_init();
    DriveAdd(USB_WIFI_DEVICE);
    sDrv = DriveGet(USB_WIFI_DEVICE);
    MP_DEBUG("driveAdd end");
    DirReset(sDrv);
    DirFirst(sDrv);

    DriveAdd(USB_PPP);
    sDrv = DriveGet(USB_PPP);
    MP_DEBUG("driveAdd end");
    DirReset(sDrv);
    DirFirst(sDrv);

#if Make_USB == AR9271_WIFI
	/* 
	 * Set debug mask for ar9271 driver.
	 * ATH_DBG_DEFAULT will print out mininum debug messages.
	 */
//	const unsigned int debug = ATH_DBG_XMIT|ATH_DBG_BEACON|ATH_DBG_CONFIG|ATH_DBG_FATAL|ATH_DBG_PS|ATH_DBG_HWTIMER;
	const unsigned int debug = ATH_DBG_DEFAULT;
	mpx_ar9271_usb_module_init(debug);
#endif
}
#endif


struct parse_data {
	/* Configuration variable name */
	char *name;

	/* Parser function for this variable */
	int (*parser)(const struct parse_data *data, const char *value);
};

#define CR '\r'
#define LF '\n'
#define READ_BUFFER_SIZE 80

#if Make_CURL
/* 
 * HTTP proxy config 
 *
 */

char _my_proxy_host[64];
char _my_proxy_password[64];
char _my_proxy_username[64];

#if 1
char *my_proxy_host = "";
u32 my_proxy_port;
char *my_proxy_username = "";
char *my_proxy_password = "";
#else
/* 
 * To enable hard-coded http proxy,
 * uncomment the following code fragment and 
 * put hard-coded http proxy info here 
 */
char *my_proxy_host = "192.168.47.10";
u32 my_proxy_port = 8002;
char *my_proxy_username = "bar";
char *my_proxy_password = "foo";
#endif

static int http_config_parse_proxyhost(const struct parse_data *data, const char *value)
{
    int len = strlen(value);
    short i,k = 0;
    for (i=0; i<len; i++)
    {
        char v = value[i];
        if (v == '"')
            continue;
        else
            _my_proxy_host[k++] = v;
    }
    _my_proxy_host[k] = '\0';
    my_proxy_host = _my_proxy_host;

    MP_DEBUG("%s:%u proxy_host=%s", __func__, __LINE__, my_proxy_host);

    return 0;
}

static int http_config_parse_proxyport(const struct parse_data *data, const char *value)
{
	char *endp;
    int len = strlen(value);
    short i=0,k;
    long v= value[0];
    if (v == '"')
        i++;
    v = strtol(value+i, &endp, 10);

    my_proxy_port = (u32)v;
    MP_DEBUG("%s:%u proxy_port=%u", __func__, __LINE__, my_proxy_port);
    return 0;
}

static int http_config_parse_proxyusername(const struct parse_data *data, const char *value)
{
    int len = strlen(value);
    short i,k = 0;
    for (i=0; i<len; i++)
    {
        char v = value[i];
        if (v == '"')
            continue;
        else
            _my_proxy_username[k++] = v;
    }
    _my_proxy_username[k] = '\0';
    my_proxy_username = _my_proxy_username;
    MP_DEBUG("%s:%u proxy_password=%s", __func__, __LINE__, my_proxy_username);

    return 0;
}

static int http_config_parse_proxypassword(const struct parse_data *data, const char *value)
{
    int len = strlen(value);
    short i,k = 0;
    for (i=0; i<len; i++)
    {
        char v = value[i];
        if (v == '"')
            continue;
        else
            _my_proxy_password[k++] = v;
    }
    _my_proxy_password[k] = '\0';
    my_proxy_password = _my_proxy_password;
    MP_DEBUG("%s:%u proxy_password=%s", __func__, __LINE__, my_proxy_password);

    return 0;
}

static const struct parse_data http_fields[] = {
	{ "proxy_host", http_config_parse_proxyhost },
	{ "proxy_port", http_config_parse_proxyport },
	{ "proxy_username", http_config_parse_proxyusername },
	{ "proxy_password", http_config_parse_proxypassword },
};
#define NUM_SSID_FIELDS (sizeof(http_fields) / sizeof(http_fields[0]))

void http_proxy_config_get(void)
{
    BYTE CurId = DriveCurIdGet();
    DRIVE *drv = DriveGet(SD_MMC_PART1);
    u8 tok[80];
    u8 var[80];
    u8 val[80];
    short i, k = 0;

	int ret = FileSearch(drv, "NET", "CFG", E_FILE_TYPE);
	if(ret != 0)
        MP_ALERT("%s:%u FileSearch(NET.CFG) returns error - %d", __func__, __LINE__, ret);
    else
	{
        STREAM *handle;
        DWORD file_read_size;
        u8 read_buf[READ_BUFFER_SIZE];

        handle = FileOpen(drv);
        if (!handle)
        {
            MP_ALERT("%s:%u FileOpen(NET.CFG) failed", __func__, __LINE__);
            goto error;
        }

        SeekSet(handle);
		
		do
		{
            file_read_size=FileRead(handle,read_buf,READ_BUFFER_SIZE);
			mpDebugPrint("file_read_size %u",file_read_size);

            for (i=0; i < file_read_size; i++) 
            {
                int v = read_buf[i];
                short j;

                if (v == LF)
                {
                    if (k > 0)
                    {
                        /* end of a line (a value) */
                        if (k < sizeof(tok))
                            tok[k++] = '\0';
                        else
                            tok[sizeof(tok)-1] = '\0';

                        strcpy(val, tok);
                        MP_DEBUG("val=%s",val);

                        for (j=0;j < NUM_SSID_FIELDS;j++)
                        {
                            const struct parse_data *field = &http_fields[j];
                            if (strcmp(var, field->name) == 0)
                            {
                                field->parser(field, val);
                                break;
                            }
                        }

                        k = 0;
                    }
                }
                else if (isspace(v))
                    continue;
                else if (v == '=')
                {
                    if (k > 0)
                    {
                        /* end of a variable name */
                        if (k < sizeof(tok))
                            tok[k++] = '\0';
                        else
                            tok[sizeof(tok)-1] = '\0';

                        strcpy(var, tok);
                        MP_DEBUG("var=%s",var);

                        k = 0;
                    }
                }
                else
                {
                    if (k < sizeof(tok))
                        tok[k++] = v;
                }
            }

		} while (file_read_size > 0);

		FileClose(handle);
	}

error:
    DriveChange(CurId);
}
#endif

#ifdef HAVE_HOSTAPD

void NetConfigChanged(const u8 *ipv4_method, const u8 *ipv4_address, const u8 *ipv4_netmask);

static char _my_ipv4_method[8];
static char _my_ipv4_address[16];
static char _my_ipv4_netmask[16];
static char _my_ssid[32+1];
char *my_ipv4_method = "";
char *my_ipv4_address = "";
char *my_ipv4_netmask = "";
char *my_ssid = "";
int my_channel = 0;
static int hostapd_config_parse_ipv4_method(const struct parse_data *data, const char *value)
{
    int len = strlen(value);
    short i,k = 0;
    for (i=0; i<len; i++)
    {
        char v = value[i];
        if (v == '"')
            continue;
        else
            _my_ipv4_method[k++] = v;
    }
    _my_ipv4_method[k] = '\0';
    my_ipv4_method = _my_ipv4_method;

    MP_DEBUG("%s:%u ipv4 method=%s", __func__, __LINE__, my_ipv4_method);

    return 0;
}

static int hostapd_config_parse_ipv4_address(const struct parse_data *data, const char *value)
{
    int len = strlen(value);
    short i,k = 0;
    for (i=0; i<len; i++)
    {
        char v = value[i];
        if (v == '"')
            continue;
        else
            _my_ipv4_address[k++] = v;
    }
    _my_ipv4_address[k] = '\0';
    if (strlen(_my_ipv4_address) > 15)
    {
        my_ipv4_address = "";
        return -EINVAL;
    }

    my_ipv4_address = _my_ipv4_address;

    MP_DEBUG("%s:%u ipv4 address=%s", __func__, __LINE__, my_ipv4_address);

    return 0;
}

static int hostapd_config_parse_ipv4_netmask(const struct parse_data *data, const char *value)
{
    int len = strlen(value);
    short i,k = 0;
    for (i=0; i<len; i++)
    {
        char v = value[i];
        if (v == '"')
            continue;
        else
            _my_ipv4_netmask[k++] = v;
    }
    _my_ipv4_netmask[k] = '\0';
    if (strlen(_my_ipv4_netmask) > 15)
    {
        my_ipv4_netmask = "";
        return -EINVAL;
    }

    my_ipv4_netmask = _my_ipv4_netmask;

    MP_DEBUG("%s:%u ipv4 netmask=%s", __func__, __LINE__, my_ipv4_netmask);

    return 0;
}

static int hostapd_config_parse_ssid(const struct parse_data *data, const char *value)
{
    int len = strlen(value);
    short i,k = 0;
    for (i=0; i<len; i++)
    {
        char v = value[i];
        if (v == '"')           /* skip double quote (") */
            continue;
        else
            _my_ssid[k++] = v;
    }
    _my_ssid[k] = '\0';
    my_ssid = _my_ssid;

    MP_DEBUG("%s:%u ssid=%s", __func__, __LINE__, my_ssid);

    return 0;
}

static int hostapd_config_parse_channel(const struct parse_data *data, const char *value)
{
    my_channel = atoi(value);

    MP_DEBUG("%s:%u channel=%d", __func__, __LINE__, my_channel);

    return 0;
}

static const struct parse_data hostapd_fields[] = {
	{ "ipv4.method", hostapd_config_parse_ipv4_method },
	{ "ipv4.address", hostapd_config_parse_ipv4_address },
	{ "ipv4.netmask", hostapd_config_parse_ipv4_netmask },
	{ "ssid", hostapd_config_parse_ssid },
	{ "channel", hostapd_config_parse_channel },
};
#define NUM_HOSTAPD_FIELDS (sizeof(hostapd_fields) / sizeof(hostapd_fields[0]))

int hostapd_config_get(const char *filename)
{
    BYTE CurId = DriveCurIdGet();
    DRIVE *drv = DriveGet(SD_MMC_PART1);
    u8 tok[80];
    u8 var[80];
    u8 val[80];
    short i, k;
    char name[8+1], ext[3+1], *p;
    size_t len;
	int ret;
    bool space_ok;
    short stage;

    MP_DEBUG("%s:%u File %s", __func__, __LINE__, filename);
	p = strchr(filename,'.');	
    len = ((long)p-(long)filename);
    if (len > 8)
    {
        ret =  -ENAMETOOLONG;
		MP_ASSERT(0);
        goto error;
    }

    memcpy(name, filename, len);
    name[len] = '\0';

    len = strlen(p+1);
    if (len > 3)
    {
        ret =  -ENAMETOOLONG;
		MP_ASSERT(0);
        goto error;
    }
    memcpy(ext, p+1, len);
    ext[len] = '\0';

	ret = FileSearch(drv, name, ext, E_FILE_TYPE);
	if(ret != 0)
    {
        MP_ALERT("%s:%u File %s not found (error code=%d)", __func__, __LINE__, filename, ret);
        ret = -ENOENT;
        goto error;
    }
    else
	{
        STREAM *handle;
        DWORD file_read_size;
        u8 read_buf[READ_BUFFER_SIZE];

        handle = FileOpen(drv);
        if (!handle)
        {
            MP_ALERT("%s:%u FileOpen(%s) failed", __func__, __LINE__, filename);
            ret = -ENOENT;
            goto error;
        }

        SeekSet(handle);
		
        ret = 0;

        k = 0;
        var[0] = '\0';
        stage = 0;
		do
		{
            file_read_size=FileRead(handle,read_buf,READ_BUFFER_SIZE);
			MP_DEBUG("FileRead returns %u",file_read_size);

            for (i=0; i < file_read_size; i++) 
            {
                u16 v = read_buf[i];
                u16 j;

                if (v == LF)
                {
                    if (k > 0)
                    {
                        if (var[0] == '#')  /* a comment line */
                        {
                            k = 0;
                            var[0] = '\0';
                            stage = 0;
                            continue;
                        }

                        /* end of a line (a value) */
                        if (k < sizeof(tok))
                            tok[k++] = '\0';
                        else
                            tok[sizeof(tok)-1] = '\0';

                        strcpy(val, tok);
                        MP_DEBUG("val=%s",val);

                        for (j=0;j < NUM_HOSTAPD_FIELDS;j++)
                        {
                            const struct parse_data *field = &hostapd_fields[j];
                            if (strcmp(var, field->name) == 0)
                            {
                                field->parser(field, val);
                                break;
                            }
                        }

                        k = 0;
                        var[0] = '\0';
                        stage = 0;
                    }
                }
                else if (isspace(v))
                {
                    if (stage == 1 && (space_ok))
                    {
                        if (k < sizeof(tok))
                            tok[k++] = v;
                    }
                    else
                        continue;
                }
                else if (v == '"')
                {
                    if (stage == 1)
                    {
                        if (k == 0)
                            space_ok = TRUE;
                        else if (space_ok)          /* ending quote */
                            space_ok = FALSE;
                    }
                }
                else if (v == '=')
                {
                    if (k > 0)
                    {
                        /* end of a variable name */
                        if (k < sizeof(tok))
                            tok[k++] = '\0';
                        else
                            tok[sizeof(tok)-1] = '\0';

                        strcpy(var, tok);
                        MP_DEBUG("var=%s",var);
                        stage = 1;
                        space_ok = FALSE;

                        k = 0;
                    }
                }
                else
                {
                    if (k < sizeof(tok))
                        tok[k++] = v;
                }
            }

		} while (file_read_size > 0);

		FileClose(handle);
	}

error:
    DriveChange(CurId);
    return ret;
}

extern int hostapd_queue_global_lock;
void m_hostapd_init(void)
{
    hostapd_queue_global_lock = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
    hostapd_config_get("magic.cfg");
    NetConfigChanged(my_ipv4_method, my_ipv4_address, my_ipv4_netmask);
}

#endif

#endif



#if Make_USB == AR9271_WIFI
void ar9271_netpool_init(WHICH_OTG eWhichOtg)
{
	u8_t *tmp;
    int ret;
    static bool netpool_initialized;
    size_t tot = 0, sz;
    int n;

	MP_DEBUG("\n\r!!!===============ar9271_netpool_init==================!!!\n\r");

    if (netpool_initialized)
        return;
    netpool_initialized = TRUE;

    if (!HeapSemid)
    {
		HeapSemid = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(HeapSemid > 0);
    }

	netpool_destroy();

	tmp = (u8_t *)SystemGetMemAddr(NETPOOL_BUF_ID);

	net_buf_start = tmp;

    /*
     * NETPOOL_BUF_SIZE = 96 * 4864 = 466944
     *
     * small: 32 * 512 = 16384
     * medium: 96 * 2688 = 258048 
     * large: 16 * (0x4000 + 3 * 128) = 318592
     */
    n = 88 + 232;
    sz = 544;
    netpool_create(tmp, n, sz);
    tmp += n * (sz + NETBUF_OVERHEAD);
    tot += n * (sz + NETBUF_OVERHEAD);

	n = 36;
    sz = 4800;
    netpool_create(tmp, n, sz);
    tmp += n * (sz + NETBUF_OVERHEAD);
    tot += n * (sz + NETBUF_OVERHEAD);

    n = 16;
    sz = (0x4000 + 3 * 128);
    netpool_create(tmp, n, sz - NETBUF_OVERHEAD);
    tmp += n * sz;
    tot += n * sz;

	net_buf_end = tmp;
	mpDebugPrint("netpool: net_buf_start %x net_buf_end %x",net_buf_start,net_buf_end);
	mpDebugPrint("netpool: total memory=%d",tot);
    MP_ASSERT(((long)net_buf_end - (long)net_buf_start) <= NETPOOL_BUF_SIZE);

	UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)net_buf_start,(DWORD)net_buf_end, eWhichOtg);
}
#endif

#if DM9621_ETHERNET_ENABLE
void dm9621_netpool_init(int udev_num)
{
#define NUM_BUFFERS 300
	u8_t *tmp;
    int ret;
    WHICH_OTG eWhichOtg;
	
    eWhichOtg = udev_num;

	MP_DEBUG("\n\r!!!===============???ar2524_netpool_init???==================!!!\n\r");

    if (!HeapSemid)
    {
		HeapSemid = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(HeapSemid > 0);
    }

	netpool_destroy();

	tmp = (u8_t *)SystemGetMemAddr(NETPOOL_BUF_ID);

	net_buf_start = tmp;

    /*
     * NETPOOL_SIZE = 330 * (1728 + 32) = 528000
     *
     * sizeof(ST_NET_PACKET) == 1824
     * 1952 = 1824 + 128
     */
    netpool_create(tmp, NUM_BUFFERS, 1952);
    tmp += NUM_BUFFERS * (1952 + NETBUF_OVERHEAD);

	net_buf_end = tmp;
	mpDebugPrint("net_buf_start %x net_buf_end %x",net_buf_start,net_buf_end);

	UsbOtgHostSetSwapBuffer2RangeEnable(net_buf_start,net_buf_end,eWhichOtg);
}
#endif

#if Make_USB == REALTEK_RTL8188CU
/* RealTek 8188CU USB WiFI dongle */
void rtl8188c_netpool_init(WHICH_OTG eWhichOtg)
{
	u8_t *tmp;
	int ret;

	MP_DEBUG("\n\r!!!===============???rtl8188c_netpool_init???==================!!!\n\r");

	if(!HeapSemid)
	{
		HeapSemid = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(HeapSemid > 0);
	}
	netpool_destroy();
	tmp = (u8_t *)SystemGetMemAddr(NETPOOL_BUF_ID);
	net_buf_start = tmp;

	/*	
	 *	NETPOOL_BUF_SIZE = 2560 * 512 = 1310720
	 *	NETBUF_OVERHEAD = 32
	 *	MAX_XMIT_EXTBUF_SZ = 2048
	 *	MAX_RECVBUF_SZ = 15360	 
	 *	MAX_XMITBUF_SZ = 20480
	 *	XMITBUF_ALIGN_SZ = CMDBUFF_ALIGN_SZ = 512
	 *	80   -> buffer
	 *	280 -> buffer
	 *	Small -- for tx 80211 probe request, rx beacon packets, probe response packets ...etc:
	 *		4 * (MAX_RECVBUF_SZ/24 + 280 = 920)
	 *	Medium -- for  ...etc: 
	 *		8 * (MAX_RECVBUF_SZ/2 = 7680)
	 *	Large:
	 *		16 * (MAX_RECVBUF_SZ + CMDBUFF_ALIGN_SZ + 80 = 15952)	 
	 *	Huge -- for initiation tx buffer and receive skb:
	 *		24 * (MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ + 80)
	*/
#if 0	
	netpool_create(tmp, 4, ((15360/24) + 280 - NETBUF_OVERHEAD));
	tmp += 4 * 920;
	netpool_create(tmp, 8, ((15360/2) - NETBUF_OVERHEAD));
	tmp += 8 * 7680;
	netpool_create(tmp, 40, (15360 + 512 + 80 - NETBUF_OVERHEAD));
	tmp += 32 * 15952;
	netpool_create(tmp, 32, (20480 + 512 + 80  - NETBUF_OVERHEAD));
	tmp += 24 * 21072;
#else
	netpool_create(tmp, 48, (1024 - NETBUF_OVERHEAD));
	tmp += 48 * 1024;
	netpool_create(tmp, 48, ((15360/2) - NETBUF_OVERHEAD));
	tmp += 48 * 7680;
	netpool_create(tmp, 480, (15936 - NETBUF_OVERHEAD));
	tmp += 480 * 15936;
	netpool_create(tmp, 8, (21056 - NETBUF_OVERHEAD));
	tmp += 8 * 21056;
#endif

	net_buf_end = tmp;
	mpDebugPrint("net_buf_start = %x	net_buf_end = %x", net_buf_start, net_buf_end);
	MP_ASSERT(((long)net_buf_end - (long)net_buf_start) <= NETPOOL_BUF_SIZE);
	UsbOtgHostSetSwapBuffer2RangeEnable(net_buf_start, net_buf_end, eWhichOtg);
}
#endif	//REALTEK_RTL8188CU

