#ifndef __WPS_WINRVP_H__
#define __WPS_WINRVP_H__


#include "common.h"


/* Microsoft's vendor ID */
#define WPS_VID_MICROSOFT  0x0137
#define WPS_VID_MAGICPIXEL  0x0851


/* TLV types used in Rally Vertical Pairing */
enum winrvp_tlv_type {
	TLV_TYPE_VPI  = 0x1001,
	TLV_TYPE_UUID = 0x1002,
};

/* transports supported by Rally Vertical Pairing */
enum winrvp_vpi_transport {
	TRANSPORT_NONE        = 0x00,
	TRANSPORT_DPWS        = 0x01,
	TRANSPORT_UPNP        = 0x02,
	TRANSPORT_SECURE_DPWS = 0x03,
};

enum winrvp_vpi_profile_request {
	PROFILE_NOT_REQUESTED = 0x00,
	PROFILE_REQUESTED     = 0x01,
};


int
wps_build_winrvp_vpi_tlv(struct wpabuf                   *ext,
                         enum winrvp_vpi_transport        trn,
                         enum winrvp_vpi_profile_request  req);

int
wps_build_winrvp_uuid_tlv(struct wpabuf *ext,
                          unsigned char  uuid[16]);


#endif /* __WPS_WINRVP_H__ */
