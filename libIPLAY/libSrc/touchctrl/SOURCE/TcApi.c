///
///@ingroup     iMagic Touch Screen & HWR APIs
///@defgroup    Tc_api   api_TcHwr
///
/// The API interface of the Touch Screen and the HWR
///
/// This file provides api interface for Touch screen control and Hand write recognize.
///

#define LOCAL_DEBUG_ENABLE 0

#include <string.h>

#include "global612.h"
#include "mpTrace.h"
#include "os.h"
#include "taskid.h"
#include "tcapi.h"

#if (TOUCH_CONTROLLER_ENABLE == ENABLE)

static ST_TC_FUNC stTcFunc;
static ST_TC_FUNC *stTcFuncPtr = NULL;


///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    cbFunc    parameters that is needed for callback function
///
///@retval COMMAND_COMPLETED :start Tc task successfully
///
SDWORD mpx_TcFunctionsStartup(TC_GET_DATA_CALLBACK cbFunc)
{
    TcFuncConstruct(&stTcFunc);
    stTcFuncPtr = &stTcFunc;
    stTcFuncPtr->FuncInit();
    if(cbFunc)
        stTcFuncPtr->FuncGetDataCallBackReg(cbFunc);


    return 0;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    width    LCD panel width
///@param    height    LCD panel height
///
///@retval COMMAND_COMPLETED :start Tc Postion reset successfully
///
SDWORD mpx_TcFunctionsPosReset(SWORD x_off, SWORD y_off, WORD x_max, WORD y_max, WORD x_min, WORD y_min)
{
    if(stTcFuncPtr)
    {
        stTcFuncPtr->FuncPosReset(x_off, y_off, x_max, y_max, x_min, y_min);
        return NO_ERR;
    }
    else
    {
        return ERR_NO_TC_TC_NOT_EXIST;
    }
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    tcpara    buffer for getting position
///
///@retval COMMAND_COMPLETED :start TC Postion reset successfully
///
SDWORD mpx_TcFunctionsPosGet(ST_TC_PARA* tcpara)
{
    if(stTcFuncPtr)
    {
        stTcFuncPtr->FuncPosGet(tcpara);
        return NO_ERR;
    }
    else
    {
        return ERR_NO_TC_TC_NOT_EXIST;
    }
}


///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    cross_width    Adjustment Cross width
///@param    cross_single_width    Adjustment Cross height
///
///@retval COMMAND_COMPLETED :start TC adjust successfully
///
SDWORD mpx_TcFunctionsAdjust(WORD cross_width, WORD cross_single_width)
{
    ST_TC_MSG tcMessage;

    if(stTcFuncPtr)
    {
        *(stTcFuncPtr->pCross_width) = cross_width;
        *(stTcFuncPtr->pCross_single_width) = cross_single_width;
        tcMessage.status = TC_ADJUST;
        MessageDrop(stTcFuncPtr->FuncGetMsgId(), (BYTE *) & tcMessage, 1);

        return NO_ERR;
    }
    else
        return ERR_NO_TC_TC_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    cross_width    Test Cross width
///@param    cross_single_width    Test Cross height
///
///@retval COMMAND_COMPLETED :start TC test successfully
///
SDWORD mpx_TcFunctionsTest(WORD cross_width, WORD cross_single_width)
{
    ST_TC_MSG tcMessage;

    if(stTcFuncPtr)
    {
        *(stTcFuncPtr->pCross_width) = cross_width;
        *(stTcFuncPtr->pCross_single_width) = cross_single_width;
        tcMessage.status = TC_TEST;
        MessageDrop(stTcFuncPtr->FuncGetMsgId(), (BYTE *) & tcMessage, 1);

        return NO_ERR;
    }
    else
        return ERR_NO_TC_TC_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    tick    tick for time out
///
///@retval COMMAND_COMPLETED :adjust waiting successfully
///
SDWORD mpx_TcFunctionsAdjustWait(U32 tick)
{
    ST_TC_MSG tcMessage;
    SDWORD sts;

    if(stTcFuncPtr)
    {
        sts = MessageReceiveWithTO((BYTE) stTcFuncPtr->FuncGetRetMsgId(), (BYTE *) & tcMessage, tick);
        if(tcMessage.status == TC_ADJUSTFAIL)
            return ERR_NO_TC_ADJUST_FAIL;
        else if (sts == OS_STATUS_TIMEOUT)
            return ERR_NO_TC_ADJUST_TIMEOUT;
        else
            return NO_ERR;
    }
    else
        return ERR_NO_TC_TC_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///
///@retval COMMAND_COMPLETED :Poll timer setting successfully
///
SDWORD mpx_TcFunctionsPollSet(U32 set)
{
    if(stTcFuncPtr)
    {
        *(stTcFuncPtr->pPoll_timer) = set;
        return NO_ERR;
    }
    else
        return ERR_NO_TC_TC_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///
///@retval COMMAND_COMPLETED :stop successfully
///
SDWORD mpx_TcFunctionsStop(void)
{
    if(stTcFuncPtr)
    {
        stTcFuncPtr->FuncClose();
        return NO_ERR;
    }
    else
        return ERR_NO_TC_TC_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///
///@retval COMMAND_COMPLETED :Pause successfully
///
SDWORD mpx_TcFunctionsPause(void)
{
    if(stTcFuncPtr)
    {
        stTcFuncPtr->FuncPause();
        return NO_ERR;
    }
    else
        return ERR_NO_TC_TC_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///
///@retval COMMAND_COMPLETED :Resume successfully
///
SDWORD mpx_TcFunctionsResume(void)
{
    if(stTcFuncPtr)
    {
        stTcFuncPtr->FuncResume();
        return NO_ERR;
    }
    else
        return ERR_NO_TC_TC_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    x_sen : Sensitivity of X-axix .
///@param    y_sen : Sensitivity of Y-axix .
///@param    z_sen : Sensitivity of Z-axix .
///
///@retval COMMAND_COMPLETED :Resume successfully
///
SDWORD mpx_TcFunctionsChgSensitivity(DWORD x_sen, DWORD y_sen, DWORD z_sen)
{
    if(stTcFuncPtr)
    {
        stTcFuncPtr->FuncChgSensitivity(x_sen, y_sen, z_sen);
        return NO_ERR;
    }
    else
        return ERR_NO_TC_TC_NOT_EXIST;
}



#ifdef HAND_WRITE_RECOGNITION

static ST_HWR_FUNC *stHWRFuncPtr = NULL;
static ST_HWR_FUNC stHWRFunc;



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    x    New point x
///@param    y    New point y
///
///@retval COMMAND_COMPLETED :new point successfully
///
SDWORD mpx_TcHwrNewPoint(WORD x, WORD y)
{
    if(stHWRFuncPtr)
    {
        stHWRFuncPtr->FuncNewPoint(x, y);
        return NO_ERR;
    }
    else
        return ERR_NO_TC_HWR_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    Candidates pointer of Candidateslist
///
///@retval COMMAND_COMPLETED :start Recognize successfully
///
SDWORD mpx_TcHwrRecognize(WORD * Candidates)
{
    if(stHWRFuncPtr)
    {
        stHWRFuncPtr->FuncRecognize();
        memcpy(Candidates, stHWRFuncPtr->Candidates, 50);
        return NO_ERR;
    }
    else
        return ERR_NO_TC_HWR_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    input[array of points and strikes], Candidates pointer of Candidateslist
///
///@retval COMMAND_COMPLETED :start Recognize successfully
///
SDWORD mpx_TcHwrRecognizeX(WORD * input, WORD * Candidates)
{
    if(stHWRFuncPtr)
    {
        stHWRFuncPtr->FuncRecognizeX(input);
        memcpy(Candidates, stHWRFuncPtr->Candidates, 50);
        return NO_ERR;
    }
    else
        return ERR_NO_TC_HWR_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    path    parameters that is needed for HWR lib path
///@param    Flag    parameters that is needed for HWR lib Flag
///
///@retval COMMAND_COMPLETED :start HWR successfully
///
SDWORD mpx_HwrFunctionsStartup(BYTE * path, WORD Flag)
{
    ST_REGISTRY *func;

    if((func = mpx_RegistryFirstGet("HWR", "FUNC")))
    {
        ((HWR_FUNC_CONSTRUCTOR) func->u32Function) (&stHWRFunc);
        stHWRFuncPtr = &stHWRFunc;
        stHWRFuncPtr->FuncInit(path, Flag);

        return NO_ERR;
    }
    else
        return ERR_NO_TC_HWR_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@retval COMMAND_COMPLETED :stop HWR successfully
///
SDWORD mpx_HwrFunctionsStop(void)
{
    if(stHWRFuncPtr)
    {
        stHWRFuncPtr->FuncDeinit();
        stHWRFuncPtr= NULL;
        return NO_ERR;
    }
    else
        return ERR_NO_TC_HWR_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param
///
///@retval COMMAND_COMPLETED : successfully
///
SDWORD mpx_TcFunctionsHwrTimerSet(U32 set)
{
    if(stTcFuncPtr)
    {
        *(stTcFuncPtr->pHwr_timer) = set;
        return NO_ERR;
    }
    else
        return ERR_NO_TC_TC_NOT_EXIST;
}



///
///@ingroup  api_TcHwr
///
///@brief
///
///@param    x    New stroke x
///@param    y    New stroke y
///
///@retval COMMAND_COMPLETED :new stroke successfully
///
SDWORD mpx_TcHwrNewStroke(WORD x, WORD y)
{
    if(stHWRFuncPtr)
    {
        stHWRFuncPtr->FuncNewStroke(x, y);
        return NO_ERR;
    }
    else
        return ERR_NO_TC_HWR_NOT_EXIST;
}


#endif // #ifdef HAND_WRITE_RECOGNITION



#endif // #if (TOUCH_CONTROLLER_ENABLE == ENABLE)

