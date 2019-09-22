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
    Str_YunDuanMoShi,			//  33
	Str_MAD,					//  34
	Str_YunDuanOPMZhuangTai,	//  35
	Str_ZaiXian,				//  36
	Str_SaoMiaoErWeiMa,			//  37
	Str_ZhiNengBeiGuang,		//   38
	Str_YiFenZhongWuCaoZuo,	//39
	Str_LiangDuTiaoJie,			//   40
	Str_ZiDongGuanJi,			//   41
	Str_DaoShiZiDongGuanJi,		//  42
	Str_GuanJiShiJian,				//   43
	Str_min,						//   44
	Str_ChuPingShengYin,			//   45
	Str_YinLiangTiaoJie,			//   46
	Str_24XiaoShiZhi,				//   47
	Str_ShiJianSheZhi,				//   48
	Str_RiQiSheZhi,					//   49
	Str_RiQiGeShi,					//   50
	Str_XiTongYuYan,				//   51
	Str_JianTiZhongWen,				//   52
	Str_YingWen,					//   53
	Str_PuTaoYaWen,					//   54
	Str_XiBanYaWen,					//   55
	Str_EWen,						//   56
	Str_FaWen,						//   57
	Str_TaiWen,						//   58
	Str_ALaBoWen,					//   59
	Str_KaiJiMiMa,					//   60
	Str_SheZhiKaiJiMiMa,			//   61
	Str_ZuJieMiMa,					//   62
	Str_SheZhiZuJieMiMa,			//   63
	Str_ZuJieRiQi,					//   64
	Str_SuoDingRongJieCiShu,		//   65
	Str_Zhi,						//   66
	Str_Ci,							//   67
	Str_ZhuJieMianFengGe,			//   68
	Str_YangShiYanSe,				//   69
	Str_JianYueMoRen,				//   70
	Str_DongGan,					//   71
	Str_JianYue,					//   72
	Str_MoHuan,						//   73
	Str_XingKong,					//   74
	Str_XuanCai,					//   75
	Str_GuanYuBenJi,				//   76
	Str_DianJiBangXinXi,			//   77
	Str_WenDuXinXi,					//   78
	Str_DianChiXinXi,				//   79
	Str_RongJieZongCiShu,			//   80
	Str_GuJianBanBenHao,			//   81
	Str_XiTongBanBenHao,			//   82
	Str_XingHao,					//   83
	Str_ShengYuCiShu,				//   84
	Str_NeiBuWenDu,					//   85
	Str_DianChiRongLiang,			//   86
	Str_ZiDongRongJieMoShi,			//   87
	Str_GongChangTiaoXinMoShi,		//   88
	Str_FangDianJiaoZhengMoShi,		//   89
	Str_PingMuHuiChenJianCe,		//   90
	Str_GongChangMoShi,				//   91
	Str_DianJiBangJiHuo,			//   92
	Str_TianJiaKongZhi,				//   93
	Str_Str_DuanMianJianCeYi,		//   94
	Str_ZuoDianJi,					//   95
	Str_YouDianJi,					//   96
	Str_SuDuMaoHao,					//   97
	Str_BuShuMaoHao,				//   98
	Str_DuC,						//   99
	Str_YuReMoShi,					//   100
	Str_JiaReFangShi,				//   101
	Str_ReSuGuanSheZhi,				//   102
	Str_JiaReWenDu,					//   103
	Str_JiaReShiJian,				//   104
	Str_ZiDong,						//   105
	Str_ShouDong,					//   106
	Str_ZiDingYi,					//   107
	Str_YiBan,						//   108
	Str_BiaoZhun,					//   109
	Str_JingXi,						//   110
	Str_XianXin,					//   111
	Str_BaoCeng,					//   112
	Str_XPing,						//   113
	Str_YPing,						//   114
	Str_XYPing,						//   115
	Str_XYJiaoTi,					//   116
	Str_MoShiCanShu,				//   117
	Str_FangDianZhongXin,			//   118
	Str_RongJieDianYa,				//   119
	Str_YuRongDianYa,				//   120
	Str_ChuChenDianYa,				//   121
	Str_RongJieChongDieLiang,		//   122
	Str_DuiJiaoMuBiaoZhi,			//   123
	Str_RongJieShiJian,				//   124
	Str_YuRongShiJian,				//   125
	Str_ChuChenShiJian,				//   126
	Str_QieGeJiaoDuShangXian,		//   127
	Str_FangDianJiaoDuShangXian,	//   128
	Str_FangDianJiaoZhengMuBiaoZhi,	//   129
	Str_Du,							//   130
	Str_ShuZhiXiuGai,				//   131
	Str_RongJieZhiLiang,			//   132
	Str_DuiXianFangShi,				//   133
	Str_PingXianFangShi,			//   134
	Str_HuiFuMoRenZhi,				//   135
	Str_BaoCun,						//   136
	Str_LingCunWei,					//   137
	Str_ShanChu,					//   138
	
	
	
	
	Str_MAX_ID,
};

BYTE* getstr(DWORD str_id);
void ChangeLanguage(int lang);
void IduSetFontSize(DWORD fontsize);

/////////////////////////////////////////////////////





#endif	// XPG_STRING_H_73DF5D2BF2678F03




