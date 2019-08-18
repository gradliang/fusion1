#ifndef __XPG__GPRS__FUNC__H__
#define __XPG__GPRS__FUNC__H__

#include "xpg.h"
#include "ui.h"

enum {
	GPRS_OFF = 0,
	GPRS_DEV_FOUND,
	GPRS_AT_CONNECT,
	GPRS_PIN_REQ,
	GPRS_PIN_CHK,
	GPRS_PPP_CONNECT,
	GPRS_CONNECTED,
	GPRS_ERR_AT,
	GPRS_ERR_PIN,
	GPRS_ERR_PPP,
	GPRS_ERR_UNKNOWN,
	// for test
	GPRS_TEST_PPP_DISCONNECT,
	GPRS_TEST_PPP_DISCONNECT_DONE,
};

// For control
void GPRS_BtnLeft();
void GPRS_BtnRight();
void GPRS_BtnUp();
void GPRS_BtnDown();
void GPRS_BtnEnter();
void GPRS_BtnExit();

void GPRS_TestPPPDisconnect();
	
#endif

