#include "global612.h"

#ifndef XPG_STRING_H_73DF5D2BF2678F03
#define XPG_STRING_H_73DF5D2BF2678F03

extern int langid;

enum xpgStringId {
	Str_Null = 0,
	
	Str_FanHui,					//  1
	Str_YongHuWeiHu,			//  2
	Str_RongJieJiLu,			//  3
	Str_RongJieSheZhi,			//  4
	Str_ShouDongMoShi,			//  5
	Str_GongJuXiang,			//  6
	Str_XiTongSheZhi,			//  7
	Str_GongNengPeiZhi,			//  8
	Str_ZiDingYiTuBiao,			//  9
	Str_XianShi,				//  10
	Str_LaLiSheZhi,				//  11
	Str_DuanMianJianCe,			//  12
	Str_ZiDongDuiJiao,			//  13
	Str_JiaoDuJianCe,			//  14
	Str_BaoCunTuXiang,			//  15
	Str_HuiChenJianCe,			//  16
	Str_RongJieZanTing,			//  17
	Str_YunDuanCeLiang,			//  18
	Str_YuJiaRe,				//  19
	Str_LaLiCeShi,				//  20
	Str_FangDianJiaoZheng,		//  21
	Str_HongGuangBi,			//  22
	Str_GuangGongLvJi,			//  23
	Str_YunDuanSheZhi,			//  24
	Str_PingMuYuDaiJi,			//  25
	Str_ShengYinSheZhi,			//  26
	Str_ShiJianYuYuYan,			//  27
	Str_MiMaSheZhi,				//  28
	Str_JieMianFengGe,			//  29
	Str_XiTongXinXi,			//  30
	Str_RongJieMoShi,			//  31
	Str_JiaReSheZhi,			//  32
    
	Str_MAX_ID,
};

const BYTE* getstr(DWORD str_id);
void ChangeLanguage(int lang);
void IduSetFontSize(DWORD fontsize);

/////////////////////////////////////////////////////



#endif	// XPG_STRING_H_73DF5D2BF2678F03




