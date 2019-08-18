#ifndef _TC_API_H_
#define _TC_API_H_

#include "touchCtrller.h"

#define SYS_TC_TAG      "TCPARA"




S32 mpx_TsFunctionsStartup(TC_GET_DATA_CALLBACK);
S32 mpx_TsFunctionsPosReset(SWORD x_off, SWORD y_off, WORD x_max, WORD y_max, WORD x_min, WORD y_min);
S32 mpx_TsFunctionsPosGet(ST_TC_PARA*);
S32 mpx_TsFunctionsAdjust( WORD cross_width, WORD cross_single_width );
S32 mpx_TsFunctionsTest(WORD cross_width, WORD cross_single_width);
S32 mpx_TsFunctionsAdjustWait(DWORD tick);
S32 mpx_TsFunctionsStop( void );
S32 mpx_TsFunctionsPause(void);
S32 mpx_TsFunctionsResume(void);
S32 mpx_TsFunctionsPollSet( DWORD set );
S32 mpx_TsFunctionsHwrTimerSet( DWORD set );
#ifdef HAND_WRITE_RECOGNITION
S32 mpx_HwrFunctionsStartup(U08 * path, WORD Flag);
S32 mpx_HwrFunctionsStop(void);
S32 mpx_TsHwrNewStroke( WORD x, WORD y );
S32 mpx_TsHwrNewPoint( WORD x, WORD y );
S32 mpx_TsHwrRecognize( WORD * Candidates );
S32 mpx_TsHwrRecognize( WORD * Candidates );
S32 mpx_TsHwrRecognizeX(WORD * input, WORD * Candidates);
#endif
#endif

