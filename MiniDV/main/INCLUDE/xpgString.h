#include "global612.h"

#ifndef XPG_STRING_H_73DF5D2BF2678F03
#define XPG_STRING_H_73DF5D2BF2678F03

extern int langid;

enum xpgStringId {
	Str_Null = 0,
	
	Str_FanHui,
	Str_YongHuWeiHu,
	Str_RongJieJiLu,
	Str_RongJieSheZhi,
	Str_ShouDongMoShi,
	Str_GongJuXiang,
	Str_XiTongSheZhi,
	Str_GongNengPeiZhi,
	Str_ZiDingYiTuBiao,
	Str_XianShi,
	Str_LaLiSheZhi,
	Str_DuanMianJianCe,
	Str_ZiDongDuiJiao,
	Str_JiaoDuJianCe,
	Str_BaoCunTuXiang,
	Str_HuiChenJianCe,
	Str_RongJieZanTing,
	Str_YunDuanCeLiang,
	Str_YuJiaRe,
	Str_LaLiCeShi,
	Str_FangDianJiaoZheng,
	Str_HongGuangBi,
	Str_GuangGongLvJi,
	Str_YunDuanSheZhi,
	Str_PingMuYuDaiJi,
	Str_ShengYinSheZhi,
	Str_ShiJianYuYuYan,
	Str_MiMaSheZhi,
	Str_JieMianFengGe,
	Str_XiTongXinXi,
	Str_RongJieMoShi,
	Str_JiaReSheZhi,
    
	Str_MAX_ID,
};

const BYTE* getstr(DWORD str_id);
void ChangeLanguage(int lang);
void IduSetFontSize(DWORD fontsize);

/////////////////////////////////////////////////////



#endif	// XPG_STRING_H_73DF5D2BF2678F03




