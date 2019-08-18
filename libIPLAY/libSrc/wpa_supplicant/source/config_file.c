/*
 * WPA Supplicant / Configuration backend: text file
 * Copyright (c) 2003-2008, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This file implements a configuration backend for text files. All the
 * configuration information is stored in a text file that uses a format
 * described in the sample configuration file, wpa_supplicant.conf.
 */

#include "includes.h"

#include "common.h"
#include "config.h"
#include "base64.h"
#include "uuid.h"
#include "eap_methods.h"
#include "net_api.h"
#include "global612.h"
#include "netware.h"

char connect_ssid[32];  /**< ESSID */
int connect_ssidlen;    /**< Length of ESSID */
char connect_psk[64];   /**< PSK/Passphrase for WPA-PSK/WPA2-PSK */
int key_mgmt = -1;      /**< Key management */
int wlan_security;      /**< Wireless security mode */
char wep_key[NUM_WEP_KEYS][MAX_WEP_KEY_LEN];        /**< WEP keys */
short wep_key_len[NUM_WEP_KEYS];                    /**< WEP key length */
int wep_tx_keyidx = -1;                             /**< WEP transmit key index */
int wlan_mode;                                      /**< wireless mode ESS(0)/IBSS(1) */
int wlan_channel;                                   /**< channel for IBSS only */

static int wpa_config_validate_network(struct wpa_ssid *ssid, int line)
{
	int errors = 0;

	if (ssid->passphrase) {
		if (ssid->psk_set) {
			wpa_printf(MSG_ERROR, "Line %d: both PSK and "
				   "passphrase configured.", line);
			errors++;
		}
		wpa_config_update_psk(ssid);
	}

	if ((ssid->key_mgmt & (WPA_KEY_MGMT_PSK | WPA_KEY_MGMT_FT_PSK |
			       WPA_KEY_MGMT_PSK_SHA256)) &&
	    !ssid->psk_set) {
		wpa_printf(MSG_ERROR, "Line %d: WPA-PSK accepted for key "
			   "management, but no PSK configured.", line);
		errors++;
	}

	if ((ssid->group_cipher & WPA_CIPHER_CCMP) &&
	    !(ssid->pairwise_cipher & WPA_CIPHER_CCMP) &&
	    !(ssid->pairwise_cipher & WPA_CIPHER_NONE)) {
		/* Group cipher cannot be stronger than the pairwise cipher. */
		wpa_printf(MSG_DEBUG, "Line %d: removed CCMP from group cipher"
			   " list since it was not allowed for pairwise "
			   "cipher", line);
		ssid->group_cipher &= ~WPA_CIPHER_CCMP;
	}

	return errors;
}


struct global_parse_data {
	char *name;
	int (*parser)(const struct global_parse_data *data,
		      struct wpa_config *config, int line, const char *value);
	void *param1, *param2, *param3;
};


static int wpa_config_parse_int(const struct global_parse_data *data,
				struct wpa_config *config, int line,
				const char *pos)
{
	int *dst;
	dst = (int *) (((u8 *) config) + (long) data->param1);
	*dst = atoi(pos);
	wpa_printf(MSG_DEBUG, "%s=%d", data->name, *dst);

	if (data->param2 && *dst < (long) data->param2) {
		wpa_printf(MSG_ERROR, "Line %d: too small %s (value=%d "
			   "min_value=%ld)", line, data->name, *dst,
			   (long) data->param2);
		*dst = (long) data->param2;
		return -1;
	}

	if (data->param3 && *dst > (long) data->param3) {
		wpa_printf(MSG_ERROR, "Line %d: too large %s (value=%d "
			   "max_value=%ld)", line, data->name, *dst,
			   (long) data->param3);
		*dst = (long) data->param3;
		return -1;
	}

	return 0;
}


static int wpa_config_parse_str(const struct global_parse_data *data,
				struct wpa_config *config, int line,
				const char *pos)
{
	size_t len;
	char **dst, *tmp;

	len = os_strlen(pos);
	if (data->param2 && len < (size_t) data->param2) {
		wpa_printf(MSG_ERROR, "Line %d: too short %s (len=%lu "
			   "min_len=%ld)", line, data->name,
			   (unsigned long) len, (long) data->param2);
		return -1;
	}

	if (data->param3 && len > (size_t) data->param3) {
		wpa_printf(MSG_ERROR, "Line %d: too long %s (len=%lu "
			   "max_len=%ld)", line, data->name,
			   (unsigned long) len, (long) data->param3);
		return -1;
	}

	tmp = os_strdup(pos);
	if (tmp == NULL)
		return -1;

	dst = (char **) (((u8 *) config) + (long) data->param1);
	os_free(*dst);
	*dst = tmp;
	wpa_printf(MSG_DEBUG, "%s='%s'", data->name, *dst);

	return 0;
}


static int wpa_config_process_country(const struct global_parse_data *data,
				      struct wpa_config *config, int line,
				      const char *pos)
{
	if (!pos[0] || !pos[1]) {
		wpa_printf(MSG_DEBUG, "Invalid country set");
		return -1;
	}
	config->country[0] = pos[0];
	config->country[1] = pos[1];
	wpa_printf(MSG_DEBUG, "country='%c%c'",
		   config->country[0], config->country[1]);
	return 0;
}


static int wpa_config_process_load_dynamic_eap(
	const struct global_parse_data *data, struct wpa_config *config,
	int line, const char *so)
{
	int ret;
	wpa_printf(MSG_DEBUG, "load_dynamic_eap=%s", so);
	ret = eap_peer_method_load(so);
	if (ret == -2) {
		wpa_printf(MSG_DEBUG, "This EAP type was already loaded - not "
			   "reloading.");
	} else if (ret) {
		wpa_printf(MSG_ERROR, "Line %d: Failed to load dynamic EAP "
			   "method '%s'.", line, so);
		return -1;
	}

	return 0;
}


#ifdef CONFIG_WPS

static int wpa_config_process_uuid(const struct global_parse_data *data,
				   struct wpa_config *config, int line,
				   const char *pos)
{
	char buf[40];
	if (uuid_str2bin(pos, config->uuid)) {
		wpa_printf(MSG_ERROR, "Line %d: invalid UUID", line);
		return -1;
	}
	uuid_bin2str(config->uuid, buf, sizeof(buf));
	wpa_printf(MSG_DEBUG, "uuid=%s", buf);
	return 0;
}


static int wpa_config_process_os_version(const struct global_parse_data *data,
					 struct wpa_config *config, int line,
					 const char *pos)
{
	if (hexstr2bin(pos, config->os_version, 4)) {
		wpa_printf(MSG_ERROR, "Line %d: invalid os_version", line);
		return -1;
	}
	wpa_printf(MSG_DEBUG, "os_version=%08x",
		   WPA_GET_BE32(config->os_version));
	return 0;
}

#endif /* CONFIG_WPS */


#ifdef OFFSET
#undef OFFSET
#endif /* OFFSET */
/* OFFSET: Get offset of a variable within the wpa_config structure */
#define OFFSET(v) ((void *) &((struct wpa_config *) 0)->v)

#define FUNC(f) #f, wpa_config_process_ ## f, OFFSET(f), NULL, NULL
#define FUNC_NO_VAR(f) #f, wpa_config_process_ ## f, NULL, NULL, NULL
#define _INT(f) #f, wpa_config_parse_int, OFFSET(f)
#define INT(f) _INT(f), NULL, NULL
#define INT_RANGE(f, min, max) _INT(f), (void *) min, (void *) max
#define _STR(f) #f, wpa_config_parse_str, OFFSET(f)
#define STR(f) _STR(f), NULL, NULL
#define STR_RANGE(f, min, max) _STR(f), (void *) min, (void *) max

static const struct global_parse_data global_fields[] = {
#ifdef CONFIG_CTRL_IFACE
	{ STR(ctrl_interface) },
	{ STR(ctrl_interface_group) } /* deprecated */,
#endif /* CONFIG_CTRL_IFACE */
	{ INT_RANGE(eapol_version, 1, 2) },
	{ INT(ap_scan) },
	{ INT(fast_reauth) },
#ifdef EAP_TLS_OPENSSL
	{ STR(opensc_engine_path) },
	{ STR(pkcs11_engine_path) },
	{ STR(pkcs11_module_path) },
#endif /* EAP_TLS_OPENSSL */
	{ STR(driver_param) },
	{ INT(dot11RSNAConfigPMKLifetime) },
	{ INT(dot11RSNAConfigPMKReauthThreshold) },
	{ INT(dot11RSNAConfigSATimeout) },
#ifndef CONFIG_NO_CONFIG_WRITE
	{ INT(update_config) },
#endif /* CONFIG_NO_CONFIG_WRITE */
	{ FUNC_NO_VAR(load_dynamic_eap) },
#ifdef CONFIG_WPS
	{ FUNC(uuid) },
	{ STR_RANGE(device_name, 0, 32) },
	{ STR_RANGE(manufacturer, 0, 64) },
	{ STR_RANGE(model_name, 0, 32) },
	{ STR_RANGE(model_number, 0, 32) },
	{ STR_RANGE(serial_number, 0, 32) },
	{ STR(device_type) },
	{ FUNC(os_version) },
#endif /* CONFIG_WPS */
	{ FUNC(country) }
};

#undef FUNC
#undef _INT
#undef INT
#undef INT_RANGE
#undef _STR
#undef STR
#undef STR_RANGE
#define NUM_GLOBAL_FIELDS (sizeof(global_fields) / sizeof(global_fields[0]))


static int wpa_config_process_global(struct wpa_config *config, char *pos,
				     int line)
{
	size_t i;
	int ret = 0;

	for (i = 0; i < NUM_GLOBAL_FIELDS; i++) {
		const struct global_parse_data *field = &global_fields[i];
		size_t flen = os_strlen(field->name);
		if (os_strncmp(pos, field->name, flen) != 0 ||
		    pos[flen] != '=')
			continue;

		if (field->parser(field, config, line, pos + flen + 1)) {
			wpa_printf(MSG_ERROR, "Line %d: failed to "
				   "parse '%s'.", line, pos);
			ret = -1;
		}
		break;
	}
	if (i == NUM_GLOBAL_FIELDS) {
		wpa_printf(MSG_ERROR, "Line %d: unknown global field '%s'.",
			   line, pos);
		ret = -1;
	}

	return ret;
}
BYTE read_flag=FALSE;
struct wpa_config * wpa_config_read(const char *name)
{
    struct wpa_config *config;
    struct wpa_ssid *ssid, *tail = NULL, *head = NULL;
	
	mpDebugPrint("\n*************wpa_config_read\n");
	
    config = wpa_config_alloc_empty(NULL, NULL);
    if (config == NULL)
        return NULL;

#if DEMO_PID
    if(!read_flag)
    {
	if(wpa_get_config_file())
		{
		goto skip;
		}
		else
			read_flag = TRUE;
    }
#else
    /* 
     * No wireless network is configured during system startup.  Use the 
     * control interface of WPA Supplicant to create networks later.
     */
    goto skip;

#endif

	ssid = os_zalloc(sizeof(*ssid));
	if (ssid == NULL)
		return NULL;

	if (head == NULL) {
		head = tail = ssid;
	} else {
		tail->next = ssid;
		tail = ssid;
	}

	mpDebugPrint("ap->ssid = %s len = %d", connect_ssid, strlen(connect_ssid));

	ssid->disabled = 1;
	wpa_config_set_network_defaults(ssid);
	ssid->id = 0;
	ssid->ssid = connect_ssid;
	ssid->ssid_len = connect_ssidlen;
	//memcpy(ssid->psk, connect_psk, strlen(connect_psk));
	ssid->priority = 5;

	//auth_alg
	ssid->auth_alg = WPA_AUTH_ALG_OPEN;
	if(key_mgmt != -1){
		//Plaintext
		ssid->key_mgmt = key_mgmt;
		if(wep_tx_keyidx != -1){
			//WEP
			ssid->wep_tx_keyidx = wep_tx_keyidx;
			memcpy(ssid->wep_key[wep_tx_keyidx], wep_key[wep_tx_keyidx], strlen(wep_key[wep_tx_keyidx]));
			ssid->wep_key_len[wep_tx_keyidx] = strlen(wep_key[wep_tx_keyidx]);
			
		}

	}
	else{
		//WPA & WPA2
		ssid->passphrase = connect_psk;
		wpa_config_update_psk(ssid);

	}

	ssid->disabled = 0;
	if (wpa_config_add_prio_network(config, ssid)) {
		mpDebugPrint("failed to add "
			   "network block to priority list.");
	}

	config->ap_scan = 1;

	config->ssid = head;

skip:
#ifdef CONFIG_CTRL_IFACE
    os_free(config->ctrl_interface);
    config->ctrl_interface = os_malloc(sizeof("wlan0"));
    strcpy(config->ctrl_interface, "wlan0");
    wpa_printf(MSG_DEBUG, "ctrl_interface='%s'", config->ctrl_interface);
#endif /* CONFIG_CTRL_IFACE */

	return config;

}
#define wpa_read  0
#define wpa_write 1

int wpa_config_write(const char *name, struct wps_credential *cred)
{
	static int ret = 1;
	mpDebugPrint("\n**********wpa_config_write %s\n",name);
	ret = wpa_config_file(name,cred,wpa_write);
	return ret;
}

int wpa_config_file(const char *name, struct wps_credential *cred,BYTE rw)
{
	STREAM *handle = NULL;
    BOOL boDriveAdded = FALSE;
    BOOL boXpgLoadOK = FALSE;
	BYTE bMcardId = SD_MMC;
	DWORD dwSize_proc,dwSize;
	static DRIVE *sDrv;
	static int ret = 1;
	int readsize =0,writesize =0;

    if (SystemCardPlugInCheck(bMcardId))
	{
		SystemDeviceInit(bMcardId);
		if (!SystemCardPresentCheck(bMcardId))
		{	   
			mpDebugPrint("-E- SystemDeviceInit fail");
		}
		else
		{
			if (!(boDriveAdded = DriveAdd(bMcardId)))
				mpDebugPrint("-E- DriveAdd fail");
		}
		   
		if (boDriveAdded)
		{
		
			mpDebugPrint("-I- SD Found");
			if (!boXpgLoadOK)
			{
				handle = FileExtSearchOpen(bMcardId, "DAT", 32);
				if (handle != NULL) {
					dwSize = FileSizeGet(handle);
					mpDebugPrint("dwSize %d",dwSize);
					if(!rw)//read
					{
						  mpDebugPrint("%s",name);
						  	readsize=FileRead(handle,cred,sizeof(struct wps_credential));
							FileClose(handle);
							if(!strcmp(name,cred->ssid))
							{
								mpDebugPrint("cred SSID match!!");
								
								mpDebugPrint("cred->auth_type %d",cred->auth_type);
								mpDebugPrint("cred->encr_type %d",cred->encr_type);
								memcpy(WirelessNetworkSetupITEM.ManualSetup.SSID,cred->ssid,cred->ssid_len);
								connect_ssidlen = cred->ssid_len;
								memcpy(connect_ssid,cred->ssid,cred->ssid_len);
								WirelessNetworkSetupITEM.ManualSetup.KEY_Num = cred->key_len;
								WirelessNetworkSetupITEM.ManualSetup.KEY_Length = cred->key_len;
								memcpy(WirelessNetworkSetupITEM.ManualSetup.Key,cred->key,cred->key_len);
								if((cred->auth_type == WPS_AUTH_OPEN)||(cred->auth_type == WPS_AUTH_SHARED))
								{
								
									wep_tx_keyidx = cred->key_idx;
									switch(cred->key_len)
									{
										
										case 5:
											WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_ASCII;
											WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_64bit;
											break;
										case 10:
											WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_HEX;
											WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_64bit;
											break;
										case 13:
											WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_ASCII;
											WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_128bit;
											break;	
										case 26:
											WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_HEX;
											WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_128bit;
										break;	
									}
								}
									
							}
						  
					}
					else
					{
					        
							writesize = FileWrite(handle,cred,sizeof(struct wps_credential));
							mpDebugPrint("writesize = %d",writesize);
							FileClose(handle);
							memcpy(WirelessNetworkSetupITEM.ManualSetup.SSID,cred->ssid,cred->ssid_len);
							connect_ssidlen = cred->ssid_len;
							memcpy(connect_ssid,cred->ssid,cred->ssid_len);
							WirelessNetworkSetupITEM.ManualSetup.KEY_Num= cred->key_len;
							WirelessNetworkSetupITEM.ManualSetup.KEY_Length = cred->key_len;
							memcpy(WirelessNetworkSetupITEM.ManualSetup.Key,cred->key,cred->key_len);
							
							//NetPacketDump(cred->key,cred->key_len);
							
							if((cred->auth_type == WPS_AUTH_OPEN)||(cred->auth_type == WPS_AUTH_SHARED))
							{
								wep_tx_keyidx = cred->key_idx;
								switch(cred->key_len)
								{
								    
									case 5:
										WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_ASCII;
										WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_64bit;
										break;
									case 10:
										WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_HEX;
										WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_64bit;
										break;
									case 13:
										WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_ASCII;
										WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_128bit;
										break;	
									case 26:
										WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_HEX;
										WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_128bit;
									break;	
								}
							}
						}
					}
				else
				{
					sDrv=DriveGet(bMcardId);
					ret=CreateFile(sDrv, "wps", "dat");
					if (ret)
						mpDebugPrint("create file fail\r\n");
					handle=FileOpen(sDrv);
					if(!handle)
						mpDebugPrint("open file fail\r\n");
					else
					{
						mpDebugPrint("FileWrite");
						FileWrite(handle,cred,sizeof(struct wps_credential));
						FileClose(handle);
						
						memcpy(WirelessNetworkSetupITEM.ManualSetup.SSID,cred->ssid,cred->ssid_len);
						connect_ssidlen = cred->ssid_len;
						memcpy(connect_ssid,cred->ssid,cred->ssid_len);
						WirelessNetworkSetupITEM.ManualSetup.KEY_Num= cred->key_len;
						WirelessNetworkSetupITEM.ManualSetup.KEY_Length = cred->key_len;
						memcpy(WirelessNetworkSetupITEM.ManualSetup.Key,cred->key,cred->key_len);
						if((cred->auth_type == WPS_AUTH_OPEN)||(cred->auth_type == WPS_AUTH_SHARED))
						{
						
							wep_tx_keyidx = cred->key_idx;
							switch(cred->key_len)
							{
								case 5:
									WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_ASCII;
									WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_64bit;
									break;
								case 10:
									WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_HEX;
									WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_64bit;
									break;
								case 13:
									WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_ASCII;
									WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_128bit;
									break;	
								case 26:
									WirelessNetworkSetupITEM.ManualSetup.WNKey = Setup_HEX;
									WirelessNetworkSetupITEM.ManualSetup.KEY_Length = Setup_128bit;
								break;	
							}
						}
					}
				}
			}
		}
		DriveChange(USB_WIFI_DEVICE);
	}
	
 return 0;
}
unsigned int hextoi(char *hexstring)
{
	register char			*h;
	register unsigned int	c, v;

	v = 0;
	h = hexstring;
	if (*h == '0' && (*(h+1) == 'x' || *(h+1) == 'X')) {
		h += 2;
	}
	while ((c = (unsigned int)*h++) != 0) {
		if (c >= '0' && c <= '9') {
			c -= '0';
		} else if (c >= 'a' && c <= 'f') {
			c = (c - 'a') + 10;
		} else if (c >=  'A' && c <= 'F') {
			c = (c - 'A') + 10;
		} else {
			break;
		}
		v = (v * 0x10) + c;
	}
	return v;
}

int wpa_get_config_file()
{
 	int i;
	DRIVE *sDrv;
	STREAM *w_shandle;
	char *filebuffer=NULL;
	char *configstring=NULL,*tmpptr=NULL;
	BYTE  curDrvId;
	int ret = 0;
	DWORD file_size = 0,cplen = 0;
	BYTE keyid[2];
	
	//TaskSleep(100);
		
	curDrvId = DriveCurIdGet();
	sDrv = DriveChange(SD_MMC);
	sDrv = DriveGet(DriveCurIdGet());
	if (FS_SUCCEED == FileSearch(sDrv, "wpa_supp","cfg",E_FILE_TYPE))
	{
		w_shandle = FileOpen(sDrv);
		if(w_shandle)
		{
		    file_size = FileSizeGet(w_shandle);
			//mpDebugPrint("file_size %d",file_size);
			filebuffer = ext_mem_malloc(file_size);
			if(filebuffer==NULL)
				return 1;
	        memset(filebuffer,0x00,file_size);

			FileRead(w_shandle,filebuffer,file_size);
			if( configstring = strstr(filebuffer,"ssid=") )
			{
				configstring+=strlen("ssid=");
				//mpDebugPrint("ssid %s",configstring);
				configstring++;
				tmpptr = strstr(configstring,"\"");
				//mpDebugPrint("tmpptr %s",tmpptr);
				if(tmpptr!=NULL)
				{
					cplen = (DWORD)(tmpptr - configstring);
					memcpy(connect_ssid,configstring,cplen);
					connect_ssidlen = cplen;
				}

			}
			if( configstring = strstr(filebuffer,"key_mgmt=") )
			{
				configstring+=strlen("key_mgmt=");
				//mpDebugPrint("key_mgmt= %s",configstring);
				if(!strncmp(configstring,"WPA-PSK",strlen("WPA-PSK")))
					{
					   wlan_security = WIFI_WPA;
                   	   key_mgmt = -1;
					}
				if(!strncmp(configstring,"WPA2-PSK",strlen("WPA2-PSK")))
				{
				   	wlan_security = WIFI_WPA2;
                   key_mgmt = -1;
				}
				if(!strncmp(configstring,"NONE",strlen("NONE")))
                   key_mgmt = WPA_KEY_MGMT_NONE;
			}
			if( configstring = strstr(filebuffer,"wep_tx_keyidx=") )
			{
				configstring+=strlen("wep_tx_keyidx=");
				//mpDebugPrint("wep_tx_keyidx %s",configstring);
				if(!strncmp(configstring,"-1",strlen("-1")))
					wep_tx_keyidx = -1;
				else
					{
						memcpy(keyid,configstring,1);
						wep_tx_keyidx = hextoi(keyid);
					   	wlan_security = WIFI_WEP;

					}
				//mpDebugPrint("wep_tx_keyidx %x",wep_tx_keyidx);
			}
			if( configstring = strstr(filebuffer,"wep_key0=") )
			{
				configstring+=strlen("wep_key0=");
				//mpDebugPrint("wep_key0= %s",configstring);
				configstring++;
				tmpptr = strstr(configstring,"\"");
				//mpDebugPrint("tmpptr %s",tmpptr);
				if(tmpptr!=NULL)
				{
					cplen = (DWORD)(tmpptr - configstring);
					
					switch(cplen)
					{
					   case 10:
					   	   wep_key_len[wep_tx_keyidx] = 5;
						   break;
					   case 26:
					   	   wep_key_len[wep_tx_keyidx] = 13;
					   	   break;
					}
						
					if(wep_tx_keyidx!=-1)
					{
						for(i=0;i<wep_key_len[wep_tx_keyidx];i++)
						{
							memcpy(keyid,configstring,2);
							wep_key[wep_tx_keyidx][i]= hextoi(keyid);
							//mpDebugPrint("wep_key[wep_tx_keyidx][%d] %x",i,wep_key[wep_tx_keyidx][i]);
							configstring+=2;
							//mpDebugPrint("configstring %s",configstring);

						}
						//mpDebugPrint("key len %d",wep_key_len[wep_tx_keyidx]);
					}
				}

			}
		
			if( configstring = strstr(filebuffer,"psk=") )
			{
				configstring+=strlen("psk=");
				//mpDebugPrint("psk= %s",configstring);
				configstring++;
				tmpptr = strstr(configstring,"\"");
				//mpDebugPrint("tmpptr %s",tmpptr);
				if(tmpptr!=NULL)
				{
					cplen = (DWORD)(tmpptr - configstring);
					memcpy(connect_psk,configstring,cplen);
				}

			}

				
			//NetAsciiDump(filebuffer,file_size);
			FileClose(w_shandle);
		}
	}
	else
	{
		mpDebugPrint("Can not find wpa_supplicant.conf file in SD.");
		ret = 1;
	}
	if(filebuffer)
		ext_mem_free(filebuffer);
	
	DriveChange(curDrvId);
	return ret;

}


