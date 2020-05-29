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
	Str_GuanBiKaiJiMiMa,			//   64
	Str_GengGaiKaiJiMiMa,			//   65
	Str_GuanBiZuJieMiMa,			//   66
	Str_GengGaiZuJieMiMa,			//   67
	Str_ZuJieRiQi,					//   68
	Str_SuoDingRongJieCiShu,		//   69
	Str_Zhi,						//   70
	Str_Ci,							//   71
	Str_ZhuJieMianFengGe,			//   72
	Str_YangShiYanSe,				//   73
	Str_JianYueMoRen,				//   74
	Str_DongGan,					//   75
	Str_JianYue,					//   76
	Str_MoHuan,						//   77
	Str_XingKong,					//   78
	Str_XuanCai,					//   79
	Str_GuanYuBenJi,				//   80
	Str_DianJiBangXinXi,			//   81
	Str_WenDuXinXi,					//   82
	Str_DianChiXinXi,				//   83
	Str_RongJieZongCiShu,			//   84
	Str_GuJianBanBenHao,			//   85
	Str_XiTongBanBenHao,			//   86
	Str_XingHao,					//   87
	Str_ShengYuCiShu,				//   88
	Str_NeiBuWenDu,					//   89
	Str_DianChiRongLiang,			//   90
	Str_ZiDongRongJieMoShi,			//   91
	Str_GongChangTiaoXinMoShi,		//   92
	Str_FangDianJiaoZhengMoShi,		//   93
	Str_PingMuHuiChenJianCe,		//   94
	Str_GongChangMoShi,				//   95
	Str_DianJiBangJiHuo,			//   96
	Str_TianJiaKongZhi,				//   97
	Str_Str_DuanMianJianCeYi,		//   98
	Str_ZuoDianJi,					//   99
	Str_YouDianJi,					//   100
	Str_SuDuMaoHao,					//   101
	Str_BuShuMaoHao,				//   102
	Str_DuC,						//   103
	Str_YuReMoShi,					//   104
	Str_JiaReFangShi,				//   105
	Str_ReSuGuanSheZhi,				//   106
	Str_JiaReWenDu,					//   107
	Str_JiaReShiJian,				//   108
	Str_ZiDong,						//   109
	Str_ShouDong,					//   110
	Str_ZiDingYi,					//   111
	Str_YiBan,						//   112
	Str_BiaoZhun,					//   113
	Str_JingXi,						//   114
	Str_XianXin,					//   115
	Str_BaoCeng,					//   116
	Str_XPing,						//   117
	Str_YPing,						//   118
	Str_XYPing,						//   119
	Str_XYJiaoTi,					//   120
	Str_MoShiCanShu,				//   121
	Str_FangDianZhongXin,			//   122
	Str_RongJieDianYa,				//   123
	Str_YuRongDianYa,				//   124
	Str_ChuChenDianYa,				//   125
	Str_RongJieChongDieLiang,		//   126
	Str_DuiJiaoMuBiaoZhi,			//   127
	Str_RongJieShiJian,				//   128
	Str_YuRongShiJian,				//   129
	Str_ChuChenShiJian,				//   130
	Str_QieGeJiaoDuShangXian,		//   131
	Str_FangDianJiaoDuShangXian,	//   132
	Str_FangDianJiaoZhengMuBiaoZhi,	//   133
	Str_Du,							//   134
	Str_ShuZhiXiuGai,				//   135
	Str_RongJieZhiLiang,			//   136
	Str_DuiXianFangShi,				//   137
	Str_PingXianFangShi,			//   138
	Str_HuiFuMoRenZhi,				//   139
	Str_BaoCun,						//   140
	Str_LingCunWei,					//   141
	Str_ShanChu,					//   142
	Str_QueRen,						//   143
	Str_QuXiao,						//   144
	Str_ZhiZaoShang,				//   145
	Str_GongSiMing,					//   146
	Str_BanBen,						//   147
	Str_XuLieHao,					//   148
	Str_JiHuoRiQi,					//   149
	Str_HuanJingWenDu,				//   150
	Str_HuanJingShiDu,				//   151
	Str_QiYa,						//   152
	Str_FangDianCiShu,				//   153
	Str_ShiYongShiJian,				//   154
	Str_DaiJiShiJian,				//   155
	Str_FenZhong,					//   156
	Str_XiaoShi,					//   157
	Str_Year,						//   158
	Str_Month,						//   159
	Str_Day,						//   160
	Str_QingShuRuMiMa,				//   161
	Str_ZaiQueRenMiMa,				//   162
	Str_OPMKongZhiMianBan,			//   163
	Str_LocalOPM,					//   164
	Str_CloudOPM,					//   165
	Str_LocalOPMCtrlPanel,			//   166
	Str_RemoteOPMCtrlPanel,			//   167
	Str_BenDiShuJu,					//   168
	Str_YunDuanShuJu,				//   169
	Str_BenDiShuJuBiao,				//   170
	Str_YunDuanShuJuBiao,			//   171
	Str_QingKong,					//   172
	Str_ShuaXin,					//   173
	Str_GongDuoShaoTiaoJiLu,		//   174
	Str_ShouYe,						//   175
	Str_PrevPage,					//   176
	Str_NextPage,					//   177
	Str_WeiYe,						//   178
	Str_YeShu,						//   179
	Str_Near3Days,					//   180
	Str_NearOneWeak,				//   181
	Str_AllRecord,					//   182
	Str_XuHao,						//   183
	Str_BiaoTi,						//   184
	Str_SunHao,						//   185
	Str_GengDuoXinXi,				//   186
	Str_XuanZeKaiShiShiJian,		//   187
	Str_XuanZeJieShuShiJian,		//   188
	Str_ChaKan,						//   189
	Str_SuoDingMiMa,				//   190
	Str_GengGaiSuoDingMiMa,			//   191
	Str_SuoDingRiQi,				//   192
	Str_WangJiMiMa,					//   193
	Str_QueDing,					//   194
	Str_ZhiNengBeiGuang_DESC,		//   195
	Str_ZiDongGuanJi_DESC,			//   196
	Str_ClickCanSetupFunction,		//   197
	Str_Display_CanChoose6Icons,    //   198
	Str_DianJiShengYuCiShu,         //   199
	Str_RongJieZongCiShu2,			//   200
	Str_OPM_YunDuanOPM,             //   201
	Str_KeShiHuaHongGuangYuan,      //   202
	Str_SheXiangTouGuZhang,      //   203
	Str_DianQiBangGuZhang,      //   204
	Str_ShuRuKaiJiMiMa,      //   205
	Str_ShuRuZhuJieMiMa,      //   206
	
	
	Str_MAX_ID,
};

BYTE* getstr(DWORD str_id);
void ChangeLanguage(int lang);
void IduSetFontSize(DWORD fontsize);

/////////////////////////////////////////////////////





#endif	// XPG_STRING_H_73DF5D2BF2678F03




