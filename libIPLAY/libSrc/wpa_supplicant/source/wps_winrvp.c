#include "common.h"
#include "wps_winrvp.h"


/* -----------------------------------------------------------------------------
 * Build Microsoft Windows Rally Vertical Pairing (winrvp) VPI TLV
 * -----------------------------------------------------------------------------
 */
int
wps_build_winrvp_vpi_tlv(struct wpabuf                   *ext,
                         enum winrvp_vpi_transport        trn,
                         enum winrvp_vpi_profile_request  req)
{
	wpa_printf(MSG_DEBUG, "Rally:  * VPI TLV");

	wpabuf_put_be16(ext, TLV_TYPE_VPI);
	wpabuf_put_be16(ext, 2);
	wpabuf_put_u8  (ext, trn);
	wpabuf_put_u8  (ext, req);

	return 0;
}


/* -----------------------------------------------------------------------------
 * Build UUID TLV
 * -----------------------------------------------------------------------------
 */
int
wps_build_winrvp_uuid_tlv(struct wpabuf *ext,
                          unsigned char  uuid[16])
{
	int  i;

	wpa_printf(MSG_DEBUG, "Rally:  * Transport UUID TLV");

	wpabuf_put_be16(ext, TLV_TYPE_UUID);
	wpabuf_put_be16(ext, 16);

	for (i = 0; i < 16; i++)
		wpabuf_put_u8(ext, uuid[i]);

	return 0;
}


/* $Id$ */
