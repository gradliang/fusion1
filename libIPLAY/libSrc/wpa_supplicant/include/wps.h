/*
 * Wi-Fi Protected Setup
 * Copyright (c) 2007-2008, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef WPS_H
#define WPS_H

#include "wps_defs.h"
#ifndef ETH_ALEN
#define ETH_ALEN	6
#endif
/**
 * enum wsc_op_code - EAP-WSC OP-Code values
 */
enum wsc_op_code {
	WSC_Start = 0x01,
	WSC_ACK = 0x02,
	WSC_NACK = 0x03,
	WSC_MSG = 0x04,
	WSC_Done = 0x05,
	WSC_FRAG_ACK = 0x06
};

struct wps_registrar;

/**
 * struct wps_credential - WPS Credential
 * @ssid: SSID
 * @ssid_len: Length of SSID
 * @auth_type: Authentication Type (WPS_AUTH_OPEN, .. flags)
 * @encr_type: Encryption Type (WPS_ENCR_NONE, .. flags)
 * @key_idx: Key index
 * @key: Key
 * @key_len: Key length in octets
 * @mac_addr: MAC address of the peer
 */
struct wps_credential {
	unsigned char ssid[32];
	size_t ssid_len;
	unsigned short auth_type;
	unsigned short encr_type;
	unsigned char key_idx;
	unsigned char key[64];
	size_t key_len;
	unsigned char mac_addr[ETH_ALEN];
};

/**
 * struct wps_device_data - WPS Device Data
 * @mac_addr: Device MAC address
 * @device_name: Device Name (0..32 octets encoded in UTF-8)
 * @manufacturer: Manufacturer (0..64 octets encoded in UTF-8)
 * @model_name: Model Name (0..32 octets encoded in UTF-8)
 * @model_number: Model Number (0..32 octets encoded in UTF-8)
 * @serial_number: Serial Number (0..32 octets encoded in UTF-8)
 * @categ: Primary Device Category
 * @oui: Primary Device OUI
 * @sub_categ: Primary Device Sub-Category
 * @os_version: OS Version
 * @rf_bands: RF bands (WPS_RF_24GHZ, WPS_RF_50GHZ flags)
 */
struct wps_device_data {
	unsigned char mac_addr[ETH_ALEN];
	char *device_name;
	char *manufacturer;
	char *model_name;
	char *model_number;
	char *serial_number;
	unsigned short categ;
	unsigned long oui;
	unsigned short sub_categ;
	unsigned long os_version;
	unsigned char rf_bands;
};

/**
 * struct wps_config - WPS configuration for a single registration protocol run
 */
struct wps_config {
	/**
	 * wps - Pointer to long term WPS context
	 */
	struct wps_context *wps;

	/**
	 * registrar - Whether this end is a Registrar
	 */
	int registrar;

	/**
	 * pin - Enrollee Device Password (%NULL for Registrar or PBC)
	 */
	const unsigned char *pin;

	/**
	 * pin_len - Length on pin in octets
	 */
	size_t pin_len;

	/**
	 * pbc - Whether this is protocol run uses PBC
	 */
	int pbc;

	/**
	 * assoc_wps_ie: (Re)AssocReq WPS IE (in AP; %NULL if not AP)
	 */
	const struct wpabuf *assoc_wps_ie;
};

struct wps_data * wps_init(const struct wps_config *cfg);

void wps_deinit(struct wps_data *data);

/**
 * enum wps_process_res - WPS message processing result
 */
enum wps_process_res {
	/**
	 * WPS_DONE - Processing done
	 */
	WPS_DONE,

	/**
	 * WPS_CONTINUE - Processing continues
	 */
	WPS_CONTINUE,

	/**
	 * WPS_FAILURE - Processing failed
	 */
	WPS_FAILURE
};
enum wps_process_res wps_process_msg(struct wps_data *wps,
				     enum wsc_op_code op_code,
				     const struct wpabuf *msg);

struct wpabuf * wps_get_msg(struct wps_data *wps, enum wsc_op_code *op_code);

int wps_is_selected_pbc_registrar(const struct wpabuf *msg);
int wps_is_selected_pin_registrar(const struct wpabuf *msg);
const unsigned char * wps_get_uuid_e(const struct wpabuf *msg);

struct wpabuf * wps_build_assoc_req_ie(enum wps_request_type req_type);
struct wpabuf * wps_build_probe_req_ie(int pbc, struct wps_device_data *dev,
				       const unsigned char *uuid,
				       enum wps_request_type req_type);


/**
 * struct wps_registrar_config - WPS Registrar configuration
 */
struct wps_registrar_config {
	/**
	 * new_psk_cb - Callback for new PSK
	 * @ctx: Higher layer context data (cb_ctx)
	 * @mac_addr: MAC address of the Enrollee
	 * @psk: The new PSK
	 * @psk_len: The length of psk in octets
	 * Returns: 0 on success, -1 on failure
	 *
	 * This callback is called when a new per-device PSK is provisioned.
	 */
	int (*new_psk_cb)(void *ctx, const unsigned char *mac_addr, const unsigned char *psk,
			  size_t psk_len);

	/**
	 * set_ie_cb - Callback for WPS IE changes
	 * @ctx: Higher layer context data (cb_ctx)
	 * @beacon_ie: WPS IE for Beacon
	 * @beacon_ie_len: WPS IE length for Beacon
	 * @probe_resp_ie: WPS IE for Probe Response
	 * @probe_resp_ie_len: WPS IE length for Probe Response
	 * Returns: 0 on success, -1 on failure
	 *
	 * This callback is called whenever the WPS IE in Beacon or Probe
	 * Response frames needs to be changed (AP only).
	 */
	int (*set_ie_cb)(void *ctx, const unsigned char *beacon_ie, size_t beacon_ie_len,
			 const unsigned char *probe_resp_ie, size_t probe_resp_ie_len);

	/**
	 * pin_needed_cb - Callback for requesting a PIN
	 * @ctx: Higher layer context data (cb_ctx)
	 * @uuid_e: UUID-E of the unknown Enrollee
	 * @dev: Device Data from the unknown Enrollee
	 *
	 * This callback is called whenever an unknown Enrollee requests to use
	 * PIN method and a matching PIN (Device Password) is not found in
	 * Registrar data.
	 */
	void (*pin_needed_cb)(void *ctx, const unsigned char *uuid_e,
			      const struct wps_device_data *dev);

	/**
	 * cb_ctx: Higher layer context data for Registrar callbacks
	 */
	void *cb_ctx;
};


/**
 * enum wps_event - WPS event types
 */
enum wps_event {
	/**
	 * WPS_EV_M2D - M2D received (Registrar did not know us)
	 */
	WPS_EV_M2D,

	/**
	 * WPS_EV_FAIL - Registration failed
	 */
	WPS_EV_FAIL,

	/**
	 * WPS_EV_SUCCESS - Registration succeeded
	 */
	WPS_EV_SUCCESS
};

/**
 * union wps_event_data - WPS event data
 */
union wps_event_data {
	/**
	 * struct wps_event_m2d - M2D event data
	 */
	struct wps_event_m2d {
		unsigned short config_methods;
		const unsigned char *manufacturer;
		size_t manufacturer_len;
		const unsigned char *model_name;
		size_t model_name_len;
		const unsigned char *model_number;
		size_t model_number_len;
		const unsigned char *serial_number;
		size_t serial_number_len;
		const unsigned char *dev_name;
		size_t dev_name_len;
		const unsigned char *primary_dev_type; /* 8 octets */
		unsigned short config_error;
		unsigned short dev_password_id;
	} m2d;

	/**
	 * struct wps_event_fail - Registration failure information
	 * @msg: enum wps_msg_type
	 */
	struct wps_event_fail {
		int msg;
	} fail;
};

/**
 * struct wps_context - Long term WPS context data
 *
 * This data is stored at the higher layer Authenticator or Supplicant data
 * structures and it is maintained over multiple registration protocol runs.
 */
struct wps_context {
	/**
	 * ap - Whether the local end is an access point
	 */
	int ap;

	/**
	 * registrar - Pointer to WPS registrar data from wps_registrar_init()
	 */
	struct wps_registrar *registrar;

	/**
	 * wps_state - Current WPS state
	 */
	enum wps_state wps_state;

	/**
	 * ap_setup_locked - Whether AP setup is locked (only used at AP)
	 */
	int ap_setup_locked;

	/**
	 * uuid - Own UUID
	 */
	unsigned char uuid[16];

	/**
	 * ssid - SSID
	 *
	 * This SSID is used by the Registrar to fill in information for
	 * Credentials. In addition, AP uses it when acting as an Enrollee to
	 * notify Registrar of the current configuration.
	 */
	unsigned char ssid[32];

	/**
	 * ssid_len - Length of ssid in octets
	 */
	size_t ssid_len;

	/**
	 * dev - Own WPS device data
	 */
	struct wps_device_data dev;

	/**
	 * config_methods - Enabled configuration methods
	 *
	 * Bit field of WPS_CONFIG_*
	 */
	unsigned short config_methods;

	/**
	 * encr_types - Enabled encryption types (bit field of WPS_ENCR_*)
	 */
	unsigned short encr_types;

	/**
	 * auth_types - Authentication types (bit field of WPS_AUTH_*)
	 */
	unsigned short auth_types;

	/**
	 * network_key - The current Network Key (PSK) or %NULL to generate new
	 *
	 * If %NULL, Registrar will generate per-device PSK. In addition, AP
	 * uses this when acting as an Enrollee to notify Registrar of the
	 * current configuration.
	 */
	unsigned char *network_key;

	/**
	 * network_key_len - Length of network_key in octets
	 */
	size_t network_key_len;

	/**
	 * cred_cb - Callback to notify that new Credentials were received
	 * @ctx: Higher layer context data (cb_ctx)
	 * @cred: The received Credential
	 * Return: 0 on success, -1 on failure
	 */
	int (*cred_cb)(void *ctx, const struct wps_credential *cred);

	/**
	 * event_cb - Event callback (state information about progress)
	 * @ctx: Higher layer context data (cb_ctx)
	 * @event: Event type
	 * @data: Event data
	 */
	void (*event_cb)(void *ctx, enum wps_event event,
			 union wps_event_data *data);

	/**
	 * cb_ctx: Higher layer context data for callbacks
	 */
	void *cb_ctx;
};


struct wps_registrar *
wps_registrar_init(struct wps_context *wps,
		   const struct wps_registrar_config *cfg);
void wps_registrar_deinit(struct wps_registrar *reg);
int wps_registrar_add_pin(struct wps_registrar *reg, const unsigned char *uuid,
			  const unsigned char *pin, size_t pin_len);
int wps_registrar_invalidate_pin(struct wps_registrar *reg, const unsigned char *uuid);
int wps_registrar_unlock_pin(struct wps_registrar *reg, const unsigned char *uuid);
int wps_registrar_button_pushed(struct wps_registrar *reg);
void wps_registrar_probe_req_rx(struct wps_registrar *reg, const unsigned char *addr,
				const struct wpabuf *wps_data);

unsigned int wps_pin_checksum(unsigned int pin);
unsigned int wps_pin_valid(unsigned int pin);
unsigned int wps_generate_pin(void);

#endif /* WPS_H */
