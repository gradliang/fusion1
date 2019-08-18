/**
 * \defgroup PPP PPP & AT Commands
 * @{
 *
 */

/**
 * \file
 * AT commands implementation.  These are mainly used with GSM 2G/3G networks.
 *
 */

/** @} */

#define LOCAL_DEBUG_ENABLE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "atcmd_usb.h"
#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"

#include <linux/types.h>
#include <linux/list.h>
#include "sio.h"

bool getcreg();

int AtCmd_Request(char *cmd, char *pattern, char *result, int length, int timeout)
{
#if HAVE_USB_MODEM > 0
	return AtCmd_Request_Usb(cmd, pattern, result, length, timeout);
#else
#endif
}

#if HAVE_USB_MODEM > 0

/*************************/
/*** LOCAL DEFINITIONS ***/
/*************************/

/************************/
/*** LOCAL DATA TYPES ***/
/************************/

/***********************************/
/*** LOCAL FUNCTION DECLARATIONS ***/
/***********************************/


/******************************/
/*** PUBLIC DATA STRUCTURES ***/
/******************************/

int at_bqpvrf()
{
	unsigned char buf[40];
	char resp[32];
	int ret, rc = FALSE;

	sprintf(buf, "%s", "AT$BQPVRF\r");

	ret = __edge_setcmd2(buf, "$BQPVRF:",resp, sizeof resp, 60);
	if ( ret )
    {
		mpDebugPrint("[$BQPVRF]: %s", resp);
    }
	//return rc;
	return ret;
}

int at_cops(cops_format_ format)
{
	unsigned char buf[40];
	char resp[32];
	int ret;

	sprintf(buf, "at+cops=,%d,,\r", format);

	ret = __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("%s OK", buf);
	else
		mpDebugPrint("%s failed", buf);
	return ret;
}

bool at_cops_get(char *operator, int size)
{
	char resp[32];
	bool ret;
	char mode_str[40], oper[16];
	short mode, format;
	int r;


	ret = __edge_setcmd2("at+cops?\r", "+COPS:",resp, sizeof resp, 60);
	if ( ret )
    {
		mpDebugPrint("[+COPS?]: %s", resp);
		r = sscanf(resp, "+COPS: %hd,%hd,\"%[^\"]", &mode, &format, oper);
				
		if (r == 3)
		{
			snprintf(operator,size, oper);
			mpDebugPrint("[AT]: PLMN=%s", operator);
		}
    }
	return ret;
}

/*
 * facility lock
 */
int at_clck()
{
	unsigned char buf[40];
	char resp[32];
	int ret;
	char mode_str[40], oper[40];
	short mode, format;

	sprintf(buf, "at+clck=\"SC\",2\r");

	ret = __edge_setcmd2(buf, "+CLCK:",resp, sizeof resp, 60);
	if ( ret )
    {
		mpDebugPrint("[+CLCK]: %s", resp);
    }
	return ret;
}

/*
 * Dial (D) command
 */
int at_d(int dongle)
{
	unsigned char buf[40];
	char resp[32];
	int ret;
    	int baud_rate;

	if(!dongle)
		sprintf(buf, "atd*99#\r");
	else if(dongle)
		sprintf(buf, "atd*98*1#\r");
	
	ret = __edge_setcmd2(buf, NULL,resp, sizeof resp, 60);
    if (ret)
	{
		ret = sscanf(resp, "CONNECT %u", &baud_rate);
        if (ret == 1)
        {
        	if(!dongle)
			mpDebugPrint("[ATD*99]: OK;Baud=%u", baud_rate);
        	else if(dongle)
        		mpDebugPrint("[ATD*98]: OK;Baud=%u", baud_rate);
        }
        else
        {
        	if(!dongle)
			mpDebugPrint("[ATD*99]: OK");
        	else if(dongle)
        		mpDebugPrint("[ATD*98]: OK");
        }

		return true;
	}
	else
	{
		mpDebugPrint("[ATD*99]: Failed");
		return false;
	}
}

int threeg_GetRegisterState()
{
	return getcreg() ? 0 : -1;
}




#endif /* HAVE_USB_MODEM */

// vim: :noexpandtab:
