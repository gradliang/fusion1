#ifndef CAMERA_H_
#define CAMERA_H_

#include "global612.h"
#include "xpg.h"


typedef enum {
    CAMPREVIEW,      
    CAMRECORDING,
    CAMRECORDSTOP,
    CAMPAUSE,       
    CAMRESUME,      
    CAMCAPTURE,     
    CAMERAPREVIEW,  
    CAMERACAPTURE,  
    CAMERASTOP,    
    PLAYBACK,       
    RESOLUTION,     
    DELETING,       
    VIDEOPLAY,      
    AMRRECORDING,
    AUTORECORD
} E_MINI_DV_MODE;

#if THUMB_COUNT
struct video_thumbs_t {
    BOOL        enable[THUMB_COUNT];
    ST_IMGWIN   wins[THUMB_COUNT];
    DWORD *     buffer[THUMB_COUNT];
};
#endif
////////////////////////////////////////////////////////////////////////////////////
void xpgCb_CamCoderRecordStart();
void xpgCb_CamCoderRecordStop();
void xpgCb_CameraPreviewAndCapture();
void xpgCb_EnterCamcoderPreview();
void CamCamCoderEnter();
void xpgCb_CamCoderAutoRecordStop();
int FormatCurrStorageCard();
void ui_PreviewToMainPage();
void mpCopyWinAreaToWinArea(ST_IMGWIN * pDstWin, ST_IMGWIN * pSrcWin, SWORD wDx, SWORD wDy, SWORD wSx, SWORD wSy, WORD wW,WORD wH);

#if VIDEO_ON
int DecodeVideoFirstFrameThumb(ST_SEARCH_INFO * pSearchInfo, ST_IMGWIN* pWin);
int Draw4VideoFilesThumb();
int Free4VideoFilesThumb();
void ui_EnterVideoPage();
void ui_SelectPrevVideo();
void ui_SelectNextVideo();
#endif

#endif

