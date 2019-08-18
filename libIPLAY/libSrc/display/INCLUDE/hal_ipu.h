/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in
* deciphering, decoding, reverse engineering or in ay way altering the source
* code is strictly prohibited, unless the prior written consent of Magic
* Pixel is obtained.
*
* Filename      : hal_IPU.h
* Programmer(s) : Mico 
* Created       : Mico
* Descriptions  :
*******************************************************************************
*/

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))

//***********************************************************************
//                                              Function
//***********************************************************************
///
///
int Ipu_CSC0Set();
int Ipu_CSC1Set();
void Ipu_Sketch();

int Ipu_SetAutoEdgeFumc(char SelEdgeMode, char use_p1low, char edge_threshold, char edge_Mhight , char edge_MLow, 
	                    char Y_low_alpha, char EdgeGain, char YC_AlphaValue );
int Ipu_SetEdgeFumc(char SelEdgeMode, char EdgeGain, char YC_AlphaValue );
int Ipu_SetBlur(char BlurValue);
int Ipu_SetBrightness(DWORD YOffsetValue);
int Ipu_SetContrast(BYTE Yintgain , BYTE Yfloatgain) ;
int Ipu_SetCbCrOffset(BYTE CbOffsetValue,BYTE CrOffsetValue);
int Ipu_SetIpuGlobalEnable(BOOL yuvse_en,BOOL yuvcs_en,BOOL yuvsp_ahead);
int Ipu_SetBlackWitheEffect(BOOL CbCrBWFixValue_en,BYTE Cb_BWFixValue,BYTE Cr_BWFixValue,
                            BOOL sebw_en,BYTE Cb_BWGain,BYTE Cr_BWGain);  //not verify
int Ipu_SetHighGainEffect(BOOL HG_en,DWORD HG_offset,BYTE HG_intgain , BYTE HG_floatgain);  //not verify
int Ipu_SetColorMixEffect(BOOL CM_en,BYTE Y_gain,BOOL Y_fixGain_en,BYTE Y_fixGain,BOOL Cbr_reverse,BYTE Cb_Tone,BYTE Cr_Tone);  
int Ipu_SetSEBIANEffect(BOOL Noise_en,BYTE Noise_level,BOOL Bilevel_en,BOOL Bilevel_cbralso,BYTE Bilevel,BOOL RevPixel);
int Ipu_SetSpecialFumc(char SelEdgeMode, BOOL specialmoge, BOOL sensingmode);
int Ipu_SetYGammaEnable(BOOL GammaEnable,BOOL SelLevel ); 
int Ipu_SetYGammaTable(DWORD *GammaValue);



#endif


