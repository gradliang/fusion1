#ifndef __EDGE__H__
#define __EDGE__H__

#include "bitsDefine.h"
#include "global612.h"
#include "mpTrace.h"
#include "Net_api.h"
#include "BitsDefine.h"


#if CDPF_ENABLE == 0
#define EDGE_ERR_UNKNOWN		(-2001)
#define EDGE_ERR_CANNOT_REG		(-2002)
#define EDGE_ERR_CONNECT_FAIL	(-2003)
#define EDGE_ERR_EXIT_BY_USER	(-2004)
#define EDGE_ERR_RECONNECT_TOO_FAST (-2005)
#define EDGE_ERR_UN_INIT		(-2006)
#define EDGE_ERR_NO_SIM	    	(-2007)         // no SIM card
#define EDGE_ERR_FETCH_MAIL		(-2008)
#define EDGE_ERR_NO_MAIL		(-2009)			// no mail when SMS enter
#define EDGE_ERR_TIMEOUT		(-2010)			// timeout
#define EDGE_ERR_COULDNT_CONNECT	(-2011)		// couldn't connect to server
#define EDGE_ERR_SIGNAL_BAD		(-2012)
#define EDGE_ERR_UNSUPPORT_JPG	(-2013)
#define EDGE_ERR_DISKFULL		(-2014)
#define EDGE_ERR_FILESYSTEM		(-2015)
#endif

#endif

