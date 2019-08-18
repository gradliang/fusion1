/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "sensor.h"
#include "sensor_tool.h"
 
 

#if ((SENSOR_ENABLE == ENABLE) && defined(SENSOR_TYPE_MAGIC_SENSOR))

/*For Debug only*/
#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only

DWORD start_addr_PC_tool_mem[1000]=
{

#if 1 
/*OV5642  720p setting*/	
	0x00000100,/* data-8 bits, Addr- 16 bits*/
	0x01E00280,/*Preview resolution H=480, W=640*/
	0x78,
	521,/* 521cnt reg*/
	
0x00933103,       
0x00823008,       
0x007f3017,       
0x00fc3018,       
0x00c23810,       
0x00f03615,       
0x00003000,       
0x00003001,       
0x00003002,       
0x00003003,       
0x00ff3004,       
0x002b3030,       
0x00103011,       
0x00103010,       
0x00603604,       
0x00603622,       
0x00093621,       
0x00003709,       
0x00214000,       
0x0022401d,       
0x00543600,       
0x00043605,       
0x003f3606,       
0x00803c01,       
0x0022300d,       
0x00223623,       
0x004f5000,       
0x00045020,       
0x00795181,       
0x00005182,       
0x00225185,       
0x00015197,       
0x000a5500,       
0x00005504,       
0x007f5505,       
0x00085080,       
0x0018300e,       
0x00004610,       
0x0005471d,       
0x00064708,       
0x00a0370c,       
0x000a3808,       
0x00203809,       
0x0007380a,       
0x0098380b,       
0x000c380c,       
0x0080380d,       
0x0007380e,       
0x00d0380f,       
0x00945687,       
0x0000501f,       
0x004f5000,       
0x00cf5001,       
0x00304300,       
0x00304300,       
0x0035460b,       
0x0000471d,       
0x000c3002,       
0x00003002,       
0x00034713,       
0x0050471c,       
0x00024721,       
0x00904402,       
0x0022460c,       
0x00443815,       
0x00073503,       
0x00733501,       
0x00803502,       
0x0000350b,       
0x00c83818,       
0x00883801,       
0x00113824,       
0x00783a00,       
0x00043a1a,       
0x00303a13,       
0x00003a18,              
0x007c3a19,       
0x00123a08,       
0x00c03a09,       
0x000f3a0a,       
0x00a03a0b,       
0x0007350c,       
0x00d0350d,       
0x00083a0d,       
0x00063a0e,       
0x00003500,       
0x00003501,       
0x00003502,       
0x0000350a,       
0x0000350b,       
0x00003503,       
0x003c3a0f,       
0x00323a10,       
0x003c3a1b,       
0x00323a1e,       
0x00803a11,       
0x00203a1f,       
0x002b3030,       
0x00003a02,       
0x007d3a03,       
0x00003a04,       
0x00003a14,       
0x007d3a15,       
0x00003a16,       
0x00783a00,       
0x00093a08,       
0x00603a09,       
0x00073a0a,       
0x00d03a0b,       
0x00103a0d,       
0x000d3a0e,       
0x00044407,       
0x00705193,       
0x0000589b,       
0x00c0589a,       
0x00424001,       
0x0006401c,       
0x00ac3825,       
0x000c3827,       
       
0x0001528a,       
0x0004528b,       
0x0008528c,       
0x0010528d,       
0x0020528e,       
0x0028528f,       
0x00305290,       
0x00005292,       
0x00015293,       
0x00005294,       
0x00045295,       
0x00005296,       
0x00085297,       
0x00005298,       
0x00105299,       
0x0000529a,       
0x0020529b,       
0x0000529c,       
0x0028529d,       
0x0000529e,       
0x0030529f,       
0x00005282,       
0x00005300,       
0x00205301,       
0x00005302,       
0x007c5303,       
0x0000530c,       
0x000c530d,       
0x0020530e,       
0x0080530f,       
0x00205310,       
0x00805311,       
0x00205308,       
0x00405309,       
0x00005304,       
0x00305305,       
0x00005306,       
0x00805307,       
0x00085314,       
0x00205315,       
0x00305319,       
0x00105316,       
0x00005317,       
0x00025318,       
0x00015380,       
0x00005381,       
0x00005382,       
0x004e5383,       
0x00005384,       
0x000f5385,       
0x00005386,       
0x00005387,       
0x00015388,       
0x00155389,       
0x0000538a,       
0x0031538b,       
0x0000538c,       
0x0000538d,       
0x0000538e,       
0x000f538f,       
0x00005390,       
0x00ab5391,       
0x00005392,       
0x00a25393,       
0x00085394,       
0x00145480,       
0x00215481,       
0x00365482,       
0x00575483,       
0x00655484,       
0x00715485,       
0x007d5486,       
0x00875487,       
0x00915488,       
0x009a5489,       
0x00aa548a,       
0x00b8548b,       
0x00cd548c,       
0x00dd548d,       
0x00ea548e,       
0x001d548f,       
0x00055490,       
0x00005491,       
0x00045492,       
0x00205493,       
0x00035494,       
0x00605495,       
0x00025496,       
0x00b85497,       
0x00025498,       
0x00865499,       
0x0002549a,       
0x005b549b,       
0x0002549c,       
0x003b549d,       
0x0002549e,       
0x001c549f,       
0x000254a0,       
0x000454a1,       
0x000154a2,       
0x00ed54a3,       
0x000154a4,       
0x00c554a5,       
0x000154a6,       
0x00a554a7,       
0x000154a8,       
0x006c54a9,       
0x000154aa,       
0x004154ab,       
0x000154ac,       
0x002054ad,       
0x000054ae,       
0x001654af,       
0x000154b0,       
0x002054b1,       
0x000054b2,       
0x001054b3,       
0x000054b4,       
0x00f054b5,       
0x000054b6,       
0x00df54b7,       
0x003f5402,       
0x00005403,       
0x00003406,       
0x00ff5180,       
0x00525181,       
0x00115182,       
0x00145183,       
0x00255184,       
0x00245185,       
0x00065186,       
0x00085187,       
0x00085188,       
0x007c5189,       
0x0060518a,       
0x00b2518b,       
0x00b2518c,       
0x0044518d,       
0x003d518e,       
0x0058518f,       
0x00465190,       
0x00f85191,       
0x00045192,       
0x00705193,       
0x00f05194,       
0x00f05195,       
0x00035196,       
0x00015197,       
0x00045198,       
0x00125199,       
0x0004519a,       
0x0000519b,       
0x0006519c,       
0x0082519d,       
0x0000519e,       
0x00805025,       
0x00383a0f,       
0x00303a10,       
0x003a3a1b,       
0x002e3a1e,       
0x00603a11,       
0x00103a1f,       
0x00a65688,       
0x006a5689,       
0x00ea568a,       
0x00ae568b,       
0x00a6568c,       
0x006a568d,       
0x0062568e,       
0x0026568f,       
0x00405583,       
0x00405584,       
0x00025580,       
0x00cf5000,       
0x00275800,       
0x00195801,       
0x00125802,       
0x000f5803,       
0x00105804,       
0x00155805,       
0x001e5806,       
0x002f5807,       
0x00155808,       
0x000d5809,       
0x000a580a,       
0x0009580b,       
0x000a580c,       
0x000c580d,       
0x0012580e,       
0x0019580f,       
0x000b5810,       
0x00075811,       
0x00045812,       
0x00035813,       
0x00035814,       
0x00065815,       
0x000a5816,       
0x000f5817,       
0x000a5818,       
0x00055819,       
0x0001581a,       
0x0000581b,       
0x0000581c,       
0x0003581d,       
0x0008581e,       
0x000c581f,       
0x000a5820,       
0x00055821,       
0x00015822,       
0x00005823,       
0x00005824,       
0x00035825,       
0x00085826,       
0x000c5827,       
0x000e5828,       
0x00085829,       
0x0006582a,       
0x0004582b,       
0x0005582c,       
0x0007582d,       
0x000b582e,       
0x0012582f,       
0x00185830,       
0x00105831,       
0x000c5832,       
0x000a5833,       
0x000b5834,       
0x000e5835,       
0x00155836,       
0x00195837,       
0x00325838,       
0x001f5839,       
0x0018583a,       
0x0016583b,       
0x0017583c,       
0x001e583d,       
0x0026583e,       
0x0053583f,       
0x00105840,       
0x000f5841,       
0x000d5842,       
0x000c5843,       
0x000e5844,       
0x00095845,       
0x00115846,       
0x00105847,       
0x00105848,       
0x00105849,       
0x0010584a,       
0x000e584b,       
0x0010584c,       
0x0010584d,       
0x0011584e,       
0x0010584f,       
0x000f5850,       
0x000c5851,       
0x000f5852,       
0x00105853,       
0x00105854,       
0x000f5855,       
0x000e5856,       
0x000b5857,       
0x00105858,       
0x000d5859,       
0x000d585a,       
0x000c585b,       
0x000c585c,       
0x000c585d,       
0x000b585e,       
0x000c585f,       
0x000c5860,       
0x000c5861,       
0x000d5862,       
0x00085863,       
0x00115864,       
0x00185865,       
0x00185866,       
0x00195867,       
0x00175868,       
0x00195869,       
0x0016586a,       
0x0013586b,       
0x0013586c,       
0x0012586d,       
0x0013586e,       
0x0016586f,       
0x00145870,       
0x00125871,       
0x00105872,       
0x00115873,       
0x00115874,       
0x00165875,       
0x00145876,       
0x00115877,       
0x00105878,       
0x000f5879,       
0x0010587a,       
0x0014587b,       
0x0013587c,       
0x0012587d,       
0x0011587e,       
0x0011587f,       
0x00125880,       
0x00155881,       
0x00145882,       
0x00155883,       
0x00155884,       
0x00155885,       
0x00135886,       
0x00175887,       
0x00103710,       
0x00513632,       
0x00103702,       
0x00b23703,       
0x00183704,       
0x0040370b,       
0x0003370d,       
0x00013631,       
0x00523632,       
0x00243606,       
0x00963620,       
0x00075785,       
0x00303a13,       
0x00523600,       
0x00483604,       
0x001b3606,       
0x000b370d,       
0x00c0370f,       
0x00013709,       
0x00003823,       
0x00005007,       
0x00005009,       
0x00005011,       
0x00005013,       
0x0000519e,       
0x00005086,       
0x00005087,       
0x00005088,       
0x00005089,       
0x0000302b,       
0x00073503,       
0x00103011,     
      
0x0002350c,       
0x00e4350d,       
0x00c93621,       
0x0081370a,       
0x00083803,       
0x00053804,       
0x00003805,       
0x00023806,       
0x00d03807,       
0x00053808,       
0x00003809,       
0x0002380a,       
0x00d0380b,       
0x0008380c,       
0x0072380d,       
0x0002380e,       
0x00e4380f,       
0x00c03810,       
0x00c93818,       
0x0010381c,       
0x00a0381d,       
0x0005381e,       
0x00b0381f,       
0x00003820,       
0x00003821,       
0x00113824,       
0x001b3a08,       
0x00c03a09,       
0x00173a0a,       
0x00203a0b,       
0x00023a0d,       
0x00013a0e,       
0x0004401c,       
0x00055682,       
0x00005683,       
0x00025686,       
0x00cc5687,       
0x007f5001,       
0x0006589b,       
0x00c5589a,       
0x00003503,       
0x00103010,       
0x0020460c, 
0x0037460b,       
0x00d0471c,       
0x0005471d,       
0x00013815,       
0x00d03818,       
0x0000501f,       
0x00304300,       
0x001c3002,       
0x00014721,                  
0x00c13818,                
0x0005380e,    
0x0064380f,                      
0x0003380e,       
0x009d380f,   

#else
/*ov2643*/
	0x00000000,/* data-8 bits, Addr- 8 bits*/
	0x60,
	153,/* 153cnt reg*/


	0x00800012,
	0x001f00c3,
	0x00ff00c4,
	0x0048003d,
	0x00a500dd,
	0x00b4000e,
	0x000a0010,
	0x00000011,
	0x0014000f,
	0x00010020,
	0x00250021,
	0x00000022,
	0x000c0023,
	0x00500024,
	0x002d0026,
	0x00040027,
	0x00060029,
	0x0040002a,
	0x0002002b,
	0x00ee002c,
	0x0004001d,
	0x00040025,
	0x00040027,
	0x00400028,
	0x00480012,
	0x00100039,
	0x001200cd,
	0x00ff0013,
	0x00a70014,
	0x00420015,
	0x00a4003c,
	0x00600018,
	0x00500019,
	0x00e2001a,
	0x00e80037,
	0x00900016,
	0x00000043,
	0x00fb0040,
	0x004400a9,
	0x00ec002f,
	0x00100035,
	0x00100036,
	0x0000000c,
	0x0000000d,
	0x009300d0,
	0x002b00dc,
	0x004100d9,
	0x000200d3,
	0x0008003d,
	0x0000000c,
	0x002c0018,
	0x00240019,
	0x0071001a,
	0x0069009b,
	0x007d009c,
	0x007d009d,
	0x0069009e,
	0x00040035,
	0x00040036,
	0x00120065,
	0x00200066,
	0x00390067,
	0x004e0068,
	0x00620069,
	0x0074006a,
	0x0085006b,
	0x0092006c,
	0x009e006d,
	0x00b2006e,
	0x00c0006f,
	0x00cc0070,
	0x00e00071,
	0x00ee0072,
	0x00f60073,
	0x00110074,
	0x002000ab,
	0x005b00ac,
	0x000500ad,
	0x001b00ae,
	0x007600af,
	0x009000b0,
	0x009000b1,
	0x008c00b2,
	0x000400b3,
	0x009800b4,
	0x0003004c,
	0x0030004d,
	0x0002004e,
	0x005c004f,
	0x00560050,
	0x00000051,
	0x00660052,
	0x00030053,
	0x00300054,
	0x00020055,
	0x005c0056,
	0x00400057,
	0x00000058,
	0x00660059,
	0x0003005a,
	0x0020005b,
	0x0002005c,
	0x005c005d,
	0x003a005e,
	0x0000005f,
	0x00660060,
	0x001f0041,
	0x000100b5,
	0x000200b6,
	0x004000b9,
	0x002800ba,
	0x000c00bf,
	0x003e00c0,
	0x000a00a3,
	0x000f00a4,
	0x000900a5,
	0x001600a6,
	0x000a009f,
	0x000f00a0,
	0x000a00a7,
	0x000f00a8,
	0x001000a1,
	0x000400a2,
	0x000400a9,
	0x00a600aa,
	0x006a0075,
	0x00110076,
	0x00920077,
	0x00210078,
	0x00e10079,
	0x0002007a,
	0x0005007c,
	0x0008007d,
	0x0008007e,
	0x007c007f,
	0x00580080,
	0x002a0081,
	0x00c50082,
	0x00460083,
	0x003a0084,
	0x00540085,
	0x00440086,
	0x00f80087,
	0x00080088,
	0x00700089,
	0x00f0008a,
	0x00f0008b,
	0x00e30090,
	0x00100093,
	0x00200094,
	0x00100095,
	0x00180096,
	0x00B6000E,
#endif	
	
};

#if 1
DWORD start_addr_PC_tool_mem_capture_prev[500]=
{

0x0000013f, /*cnt = 319*//*Capture preview  cnt reg = 319*/
0x00933103,
0x00823008,

0x007f3017,
0x00fc3018,
0x00c23810,
0x00f03615,
0x00003000,
0x00003001,
0x00003002,
0x00003003,
0x00f83000,
0x00483001,
0x005c3002,
0x00023003,
0x00073004,
0x00b73005,
0x00433006,
0x00373007,
0x00103011,
0x00103010,
0x0022460c,
0x00043815,
0x0006370d,
0x00a0370c,
0x00fc3602,
0x00ff3612,
0x00c03634,
0x00003613,
0x007c3605,
0x00093621,
0x00003622,
0x00403604,
0x00a73603,
0x00273603,
0x00214000,
0x0002401d,
0x00543600,
0x00043605,
0x003f3606,
0x00803c01,
0x004f5000,
0x00045020,
0x00795181,
0x00005182,
0x00225185,
0x00015197,
0x00ff5001,
0x000a5500,
0x00005504,
0x007f5505,
0x00085080,
0x0018300e,
0x00004610,
0x0005471d,
0x00064708,
0x00103710,
0x00413632,
0x00403702,
0x00373620,
0x00013631,
0x00023808,
0x00803809,
0x0001380a,
0x00e0380b,
0x0007380e,
0x00d0380f,
0x0000501f,
0x004f5000,
0x00304300,
0x00073503,
0x00733501,
0x00803502,
0x0000350b,
0x00073503,
0x00113824,
0x001e3501,
0x00803502,
0x007f350b,
0x000c380c,
0x0080380d,
0x0003380e,
0x00e8380f,
0x00043a0d,
0x00033a0e,
0x00c13818,
0x00db3705,
0x0081370a,
0x00803801,
0x00c73621,
0x00503801,
0x00083803,
0x00083827,
0x00c03810,
0x00053804,
0x00003805,
0x00055682,
0x00005683,
0x00033806,
0x00c03807,
0x00035686,
0x00c05687,
0x00783a00,
0x00043a1a,
0x00303a13,
0x00003a18,
0x007c3a19,
0x00123a08,
0x00c03a09,
0x000f3a0a,
0x00a03a0b,
0x00ff3004,
0x0007350c,
0x00d0350d,
0x00003500,
0x00003501,
0x00003502,
0x0000350a,
0x0000350b,
0x00003503,
0x0002528a,
0x0004528b,
0x0008528c,
0x0008528d,
0x0008528e,
0x0010528f,
0x00105290,
0x00005292,
0x00025293,
0x00005294,
0x00025295,
0x00005296,
0x00025297,
0x00005298,
0x00025299,
0x0000529a,
0x0002529b,
0x0000529c,
0x0002529d,
0x0000529e,
0x0002529f,
0x003c3a0f,
0x00303a10,
0x003c3a1b,
0x00303a1e,
0x00703a11,
0x00103a1f,
0x000b3030,


0x00003a02,
0x007d3a03,
0x00003a04,
0x00003a14,
0x007d3a15,
0x00003a16,
0x007c3a00,
0x00093a08,
0x00603a09,
0x00073a0a,
0x00d03a0b,
0x00083a0d,
0x00063a0e,
0x00705193,
0x00573620,
0x00983703,
0x001c3704,
0x0004589b,
0x00c5589a,
0x0000528a,
0x0002528b,
0x0008528c,
0x0010528d,
0x0020528e,
0x0028528f,
0x00305290,
0x00005292,
0x00005293,
0x00005294,
0x00025295,
0x00005296,
0x00085297,
0x00005298,
0x00105299,
0x0000529a,
0x0020529b,
0x0000529c,
0x0028529d,
0x0000529e,
0x0030529f,
0x00005282,
0x00005300,
0x00205301,
0x00005302,
0x007c5303,
0x0000530c,
0x000c530d,
0x0020530e,
0x0080530f,
0x00205310,
0x00805311,
0x00205308,
0x00405309,
0x00005304,
0x00305305,
0x00005306,
0x00805307,
0x00085314,
0x00205315,
0x00305319,
0x00105316,
0x00085317,
0x00025318,
0x00015380,
0x00005381,
0x00005382,
0x004e5383,
0x00005384,
0x000f5385,
0x00005386,
0x00005387,
0x00015388,
0x00155389,
0x0000538a,
0x0031538b,
0x0000538c,
0x0000538d,
0x0000538e,
0x000f538f,
0x00005390,
0x00ab5391,
0x00005392,
0x00a25393,
0x00085394,
0x00145480,
0x00215481,
0x00365482,
0x00575483,
0x00655484,
0x00715485,
0x007d5486,
0x00875487,
0x00915488,
0x009a5489,
0x00aa548a,
0x00b8548b,
0x00cd548c,
0x00dd548d,
0x00ea548e,
0x0010548f,
0x00055490,
0x00005491,
0x00045492,
0x00205493,
0x00035494,
0x00605495,
0x00025496,
0x00b85497,
0x00025498,
0x00865499,
0x0002549a,
0x005b549b,
0x0002549c,
0x003b549d,
0x0002549e,
0x001c549f,
0x000254a0,
0x000454a1,
0x000154a2,
0x00ed54a3,
0x000154a4,
0x00c554a5,
0x000154a6,
0x00a554a7,
0x000154a8,
0x006c54a9,
0x000154aa,
0x004154ab,
0x000154ac,
0x002054ad,
0x000054ae,
0x001654af,
0x00003406,
0x00045192,
0x00f85191,
0x00705193,
0x00f05194,
0x00f05195,
0x003d518d,
0x0054518f,
0x003d518e,
0x00545190,
0x00c0518b,
0x00bd518c,
0x00185187,
0x00185188,
0x006e5189,
0x0068518a,
0x001c5186,
0x00505181,
0x00255184,
0x00115182,
0x00145183,
0x00255184,
0x00245185,
0x00825025,
0x007e3a0f,
0x00723a10,
0x00803a1b,
0x00703a1e,
0x00d03a11,
0x00403a1f,
0x00405583,
0x00405584,
0x00025580,
0x00073633,
0x00103702,
0x00b23703,
0x00183704,
0x0040370b,
0x0002370d,
0x00523620,
	
};

#endif

#if 1
DWORD start_addr_PC_tool_mem_capture_flow[500]=
{
	0x1,/*cnt of register Gain*/
	0x0000350B,	
	0x00070007,	

		/*Exposure*/
	0x3,
	0x00003502,/*Exposure Low*/
	0x00070007,
	
	0x00003501,/*Exposure Mid*/
	0x080F0007,

	0x00003500,/*Exposure High*/
	0x10130003,

	/*MAPPING_OF_PRVIEW_MAXLINE 0X28*/
	0X2,
	0X0000350D,
	0X00070007,
	0X0000350C,
	0X080F0007,

	/*MAPPING_OF_VTS  15  0x3C*/
	0x1,
	0x00003503,
	0x00000202,

	/*MAPPING_OF_AGC 18 - 0x48*/
	0x1,
	0x00003503,
	0x00000101,
 
	/*MAPPING_OF_AEC 21-0x54 */
	0x1,
	0x00003503,
	0x00000000,

	/*Capture setting 24-0x60*/
	0x00000058,/*88--0x58*/
		0x00013406,
		0x00003003,
		0x00FF3005,
		0x00FF3006,
		0x003F3007,
		0x00103011,
		0x00003012,
		0x0007350C,
		0x00D0350D,
		0x00E43602,
		0x00AC3612,
		0x00443613,
		0x00093621,
		0x00603622,
		0x00223623,
		0x00603604,
		0x00DA3705,
		0x0080370A,
		0x0003370D,
		0x008A3801,
		0x000A3803,
		0x000A3804,
		0x00203805,
		0x00073806,
		0x00983807,
		0x000A3808,
		0x00203809,
		0x0007380A,
		0x0098380B,
		0x000C380C,
		0x0080380D,
		0x0007380E,
		0x00D0380F,
		0x00113824,
		0x00AC3825,
		0x000A3827,
		0x00093A08,
		0x00603A09,
		0x00073A0A,
		0x00D03A0B,
		0x00103A0D,
		0x000D3A0E,
		0x00043A1A,
		0x0035460B,
		0x0000471D,
		0x00034713,
		0x00FF5001,
		0x0000589B,
		0x00C0589A,
		0x00044407,
		0x0000589B,
		0x00C0589A,
		0x001C3002,
		0x0020460C,
		0x00D0471C,
		0x00014721,
		0x00013815,
		0x0000501F,
		0x00E05002,
		0x00304300,
		0x00c03818,
		0x00C23810,
		0x00703010,
		0x00013800,
		0x008A3801,
		0x00003802,
		0x000A3803,
		0x000A3804,
		0x00203805,
		0x00073806,
		0x00983807,
		0x00023808,
		0x00803809,
		0x0001380A,
		0x00E0380B,
		0x000C380C,
		0x0080380D,
		0x0007380E,
		0x00D0380F,
		0x007F5001,
		0x00005680,
		0x00005681,
		0x000A5682,
		0x00205683,
		0x00005684,
		0x00005685,
		0x00075686,
		0x00985687,
		

	

	
	
	

};
#endif

#endif

#if (MAGIC_SHOP_DEBUG) //for Magic Shop debug only

#if 1
DWORD start_addr_PC_tool_mem_ColorIPU[128]=
{
	0x80000001,
	0x00000000,/*Ipu_reg_10*/	
	0x00000000,/*Ipu_reg_11*/
	0x00000000,/*Ipu_reg_12*/
	0x00000000,/*Ipu_reg_13*/
	0x00000000,/*Ipu_reg_14*/
	0x00000000,/*Ipu_reg_15*/
	0x00000000,/*Ipu_reg_16*/
	0x00000000,/*Ipu_reg_17*/
	0x00000000,/*Ipu_reg_18*/
	0x00000000,/*Ipu_reg_19*/
	0x00000000,/*Ipu_reg_1A*/
	0x00000000,/*Ipu_reg_1B*/
	0x00000000,/*Ipu_reg_1C*/
	0x00000000,/*Ipu_reg_1D*/
	0x00000000,/*Ipu_reg_1E*/
	0x00000000,/*Ipu_reg_1F*/
	
	0x00000000,/*Ipu_reg_20*/
	0x00000000,/*Ipu_reg_21*/
	0x00000000,/*Ipu_reg_22*/
	0x00000000,/*Ipu_reg_23*/
	0x00000000,/*Ipu_reg_24*/
	0x00000000,/*Ipu_reg_25*/
	0x00000000,/*Ipu_reg_26*/
	0x00000000,/*Ipu_reg_27*/
	0x00000000,/*Ipu_reg_28*/
	0x00000000,/*Ipu_reg_29*/
	0x00000000,/*Ipu_reg_2A*/
	0x00000000,/*Ipu_reg_2B*/
	0x00000000,/*Ipu_reg_2C*/
	0x00000000,/*Ipu_reg_2D*/
	0x00000000,/*Ipu_reg_2E*/
	0x00000000,/*Ipu_reg_2F*/

	0X00006000,/*Ipu_reg_30*/
	0x00000000,/*Ipu_reg_31*/
	0x00000000,/*Ipu_reg_32*/
	0x00000000,/*Ipu_reg_33*/
	0x00000000,/*Ipu_reg_34*/
	0x00000000,/*Ipu_reg_35*/
	0x00000000,/*Ipu_reg_36*/
	0x00000000,/*Ipu_reg_37*/
	0x00000000,/*Ipu_reg_38*/
	0x00000000,/*Ipu_reg_39*/
	0x00000000,/*Ipu_reg_3A*/
	0X00000000,/*Ipu_reg_3B*/
	0X40400000,/*Ipu_reg_3C*/
	0X00000100,/*Ipu_reg_3D*/
	0x00000000,/*Ipu_reg_3E*/
	0x00000000,/*Ipu_reg_3F*/

	0x00000000,/*Ipu_reg_40*/
	0x00000000,/*Ipu_reg_41*/
	0x00000000,/*Ipu_reg_42*/
	0x00000000,/*Ipu_reg_43*/
	0x00000000,/*Ipu_reg_44*/
	0x00000000,/*Ipu_reg_45*/
	0x00000000,/*Ipu_reg_46*/
	0x00000000,/*Ipu_reg_47*/
	0x00000000,/*Ipu_reg_48*/
	0x00000000,/*Ipu_reg_49*/
	0x00000000,/*Ipu_reg_4A*/
	0x00000000,/*Ipu_reg_4B*/
	0x00000000,/*Ipu_reg_4C*/
	0x00000000,/*Ipu_reg_4D*/
	0x00000000,/*Ipu_reg_4E*/
	0x00000000,/*Ipu_reg_4F*/

	0x00000000,/*Ipu_reg_50*/
	0x00000000,/*Ipu_reg_51*/
	0x00000000,/*Ipu_reg_52*/
	0x00000000,/*Ipu_reg_53*/
	0x00000000,/*Ipu_reg_54*/
	0x00000000,/*Ipu_reg_55*/
	0x00000000,/*Ipu_reg_56*/
	0x00000000,/*Ipu_reg_57*/
	0x00000000,/*Ipu_reg_58*/
	0x00000000,/*Ipu_reg_59*/
	0x00000000,/*Ipu_reg_5A*/
	0x00000000,/*Ipu_reg_5B*/
	0x00000000,/*Ipu_reg_5C*/
	0x00000000,/*Ipu_reg_5D*/
	0x00000000,/*Ipu_reg_5E*/
	0x00000000,/*Ipu_reg_5F*/

	0x00000081,/*Ipu_reg_60*/
	0X80808080,/*Ipu_reg_61*/
	0X80808080,/*Ipu_reg_62*/
	0XDFBF9F80,/*Ipu_reg_63*/
	0X9FBFDFFF,/*Ipu_reg_64*/
	0X80808080,/*Ipu_reg_65*/
	0X80808080,/*Ipu_reg_66*/
	0X80808080,/*Ipu_reg_67*/
	0X80808080,/*Ipu_reg_68*/
	0X80808080,/*Ipu_reg_69*/
	0X80808080,/*Ipu_reg_6A*/
	0X80808080,/*Ipu_reg_6B*/
	0X80808080,/*Ipu_reg_6C*/
	0XFFFFFFFF,/*Ipu_reg_6D*/
	0XFFFFFFFF,/*Ipu_reg_6E*/
	0XFFFFFFFF,/*Ipu_reg_6F*/
                     
   	0XFFFFFFFF,/*Ipu_reg_70*/
	0X000000FF,/*Ipu_reg_71*/
	0x00000000,/*Ipu_reg_72*/
	0x00000000,/*Ipu_reg_73*/
	0x00000000,/*Ipu_reg_74*/
	0x00000000,/*Ipu_reg_75*/
	0x00000000,/*Ipu_reg_76*/
	0x00000000,/*Ipu_reg_77*/
	0x00000000,/*Ipu_reg_78*/
	0x00000000,/*Ipu_reg_79*/
	0x00000000,/*Ipu_reg_7A*/
	0x00000000,/*Ipu_reg_7B*/
	0x00000000,/*Ipu_reg_7C*/
	0x00000000,/*Ipu_reg_7D*/
	0x00000000,/*Ipu_reg_7E*/
	0x00000000,/*Ipu_reg_7F*/
                                      
    0x00000000,/*Ipu_reg_80*/
	0x00000000,/*Ipu_reg_81*/
	0X01204020,/*Ipu_reg_82*/
	0x00000000,/*Ipu_reg_83*/
	0x00000000,/*Ipu_reg_84*/
	0x00000000,/*Ipu_reg_85*/
	0x00000000,/*Ipu_reg_86*/
	0x00000000,/*Ipu_reg_87*/
	0x00000000,/*Ipu_reg_88*/
	0x00000000,/*Ipu_reg_89*/
	0x00000000,/*Ipu_reg_8A*/
	0x00000000,/*Ipu_reg_8B*/
	0x00000000,/*Ipu_reg_8C*/
	0x00000000,/*Ipu_reg_8D*/
	0x00000000,/*Ipu_reg_8E*/
	0x00000000,/*Ipu_reg_8F*/

	0x00000000,/*Ipu_reg_90*/
	0x00000000,/*Ipu_reg_91*/
	0x00000000,/*Ipu_reg_92*/
	0x00000000,/*Ipu_reg_93*/
	0x00000000,/*Ipu_reg_94*/
	0x00000000,/*Ipu_reg_95*/
	0x00000000,/*Ipu_reg_96*/
	0x00000000,/*Ipu_reg_97*/
	0x00000000,/*Ipu_reg_98*/
	0x00000000,/*Ipu_reg_99*/
	0x00000000,/*Ipu_reg_9A*/
	0x00000000,/*Ipu_reg_9B*/
	0x00000000,/*Ipu_reg_9C*/
	0x00000000,/*Ipu_reg_9D*/
	0x00000000,/*Ipu_reg_9E*/
	0x00000000,/*Ipu_reg_9F*/
                     
   	0x00000000,/*Ipu_reg_A0*/
	0x00000000,/*Ipu_reg_A1*/
	0x00000000,/*Ipu_reg_A2*/
	0x00000000,/*Ipu_reg_A3*/
	0x00000000,/*Ipu_reg_A4*/
	0x00000000,/*Ipu_reg_A5*/
	0x00000000,/*Ipu_reg_A6*/
	0x00000000,/*Ipu_reg_A7*/
	0x00000000,/*Ipu_reg_A8*/
	0x00000000,/*Ipu_reg_A9*/
	0x00000000,/*Ipu_reg_AA*/
	0x00000000,/*Ipu_reg_AB*/
	0x00000000,/*Ipu_reg_AC*/
	0x00000000,/*Ipu_reg_AD*/
	0x00000000,/*Ipu_reg_AE*/
	0x00000000,/*Ipu_reg_AF*/

	0X00000000,/*Ipu_reg_B0*/
	0x00000000,/*Ipu_reg_B1*/
	0x00000000,/*Ipu_reg_B2*/
	0x00000000,/*Ipu_reg_B3*/
	0x00000000,/*Ipu_reg_B4*/
	0x00000000,/*Ipu_reg_B5*/
	0x00000000,/*Ipu_reg_B6*/
	0x00000000,/*Ipu_reg_B7*/
	0x00000000,/*Ipu_reg_B8*/
	0x00000000,/*Ipu_reg_B9*/
	0x00000000,/*Ipu_reg_BA*/
	0x00000000,/*Ipu_reg_BB*/
	0x00000000,/*Ipu_reg_BC*/
	0x00000000,/*Ipu_reg_BD*/
	0x00000000,/*Ipu_reg_BE*/
	0x00000000,/*Ipu_reg_BF*/
                     
                                                       
};
#endif



#endif
/*--------------------------------------------------------*/





extern BYTE sensor_mode;
extern BYTE sensor_IMGSIZE_ID;
extern BYTE sensor_FrameRate;

extern BYTE *g_bImageData_Buf;

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
static void SensorTool_set_176x144(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);		
											
	MP_ALERT("%s: not ready", __FUNCTION__);
	__asm("break 100");                     

}
static void SensorTool_setQVGA_320x240_30FPS(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);		
											
	MP_ALERT("%s: not ready", __FUNCTION__);
	__asm("break 100");                     

}
static void SensorTool_setVGA_640x480_30FPS(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolRecodeBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	
 
#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif
	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


	toolRecodeBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + RECORD_SETTING_BASE);
	cnt_sensor_regs_setting = (toolRecodeBase->b_code1) & 0x0000ffff;
	MP_DEBUG2("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);
 
	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolRecodeBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(RECORD_SETTING_BASE+4));
		  temp =  toolRecodeBase->b_code1;
	   	  temp2 = toolRecodeBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);
}


static void SensorTool_setVGA_640x480(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolCapturePriewBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem_capture_prev;
#else 
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{ 
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif
  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem_capture_prev;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolCapturePriewBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + CAPTURE_PREVIEW_SETTING_BASE);
	cnt_sensor_regs_setting = (toolCapturePriewBase->b_code1) & 0x0000ffff;
	MP_DEBUG2("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);
 
	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolCapturePriewBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(CAPTURE_PREVIEW_SETTING_BASE+4));
		  temp =  toolCapturePriewBase->b_code1;
	   	  temp2 = toolCapturePriewBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);
}


static void SensorTool_setQXGA_2048x1536(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);		
											
	MP_ALERT("%s: not ready", __FUNCTION__);
	__asm("break 100");                     
}
static void SensorTool_set2M_1600x1200(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);		
											
	MP_ALERT("%s: not ready", __FUNCTION__);
	__asm("break 100");                     

}

static void SensorTool_set5M_2560x1920(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);		
											
	MP_ALERT("%s: not ready", __FUNCTION__);
	__asm("break 100");                     

}


static void SensorTool_set_352x240_30FPS(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolRecodeBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


	toolRecodeBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + RECORD_SETTING_BASE);
	cnt_sensor_regs_setting = (toolRecodeBase->b_code1) & 0x0000ffff;
	MP_DEBUG2("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);
 
	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolRecodeBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(RECORD_SETTING_BASE+4));
		  temp =  toolRecodeBase->b_code1;
	   	  temp2 = toolRecodeBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);
}



static void SensorTool_set_720P_1280x720(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolRecodeBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


	toolRecodeBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + RECORD_SETTING_BASE);
	cnt_sensor_regs_setting = (toolRecodeBase->b_code1) & 0x0000ffff;
	MP_DEBUG2("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);
 
	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolRecodeBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(RECORD_SETTING_BASE+4));
		  temp =  toolRecodeBase->b_code1;
	   	  temp2 = toolRecodeBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);
}


static void SensorTool_set_720x480_30FPS(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolRecodeBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


	toolRecodeBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + RECORD_SETTING_BASE);
	cnt_sensor_regs_setting = (toolRecodeBase->b_code1) & 0x0000ffff;
	MP_DEBUG2("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);
 
	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolRecodeBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(RECORD_SETTING_BASE+4));
		  temp =  toolRecodeBase->b_code1;
	   	  temp2 = toolRecodeBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);
}


static void SensorTool_setXGA_1024x768_30FPS(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolRecodeBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


	toolRecodeBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + RECORD_SETTING_BASE);
	cnt_sensor_regs_setting = (toolRecodeBase->b_code1) & 0x0000ffff;
	MP_DEBUG2("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);
 
	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolRecodeBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(RECORD_SETTING_BASE+4));
		  temp =  toolRecodeBase->b_code1;
	   	  temp2 = toolRecodeBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);
}



DWORD SensorTool_Sensor_GetAvgLuminance(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);		
											
	MP_ALERT("%s: not ready", __FUNCTION__);
	__asm("break 100");                     
}





void static SensorTool_CaptureKey_VGA_640x480(void)
{	
	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem_capture_flow;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + CAPTURE_SETTING_BASE);
	cnt_sensor_regs_setting = (toolBase->b_code1) & 0x0000ffff;
	MP_ALERT("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);

	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(CAPTURE_SETTING_BASE+4));
		  temp =  toolBase->b_code1;
	   	  temp2 = toolBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);

}




void static SensorTool_CaptureKey_XGA_1024x768(void)
{

	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem_capture_flow;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + CAPTURE_SETTING_BASE);
	cnt_sensor_regs_setting = (toolBase->b_code1) & 0x0000ffff;
	MP_ALERT("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);

	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(CAPTURE_SETTING_BASE+4));
		  temp =  toolBase->b_code1;
	   	  temp2 = toolBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);
                 
}


void static SensorTool_CaptureKey_SXGA_1280x1024(void)
{

	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem_capture_flow;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + CAPTURE_SETTING_BASE);
	cnt_sensor_regs_setting = (toolBase->b_code1) & 0x0000ffff;
	MP_ALERT("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);

	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(CAPTURE_SETTING_BASE+4));
		  temp =  toolBase->b_code1;
	   	  temp2 = toolBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);

}



void static SensorTool_CaptureKey_UXGA_1600x1200(void)
{

	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem_capture_flow;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + CAPTURE_SETTING_BASE);
	cnt_sensor_regs_setting = (toolBase->b_code1) & 0x0000ffff;
	MP_ALERT("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);

	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(CAPTURE_SETTING_BASE+4));
		  temp =  toolBase->b_code1;
	   	  temp2 = toolBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);

}



void SensorTool_CaptureKey_QXGA_2048x1536(void)
{

	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem_capture_flow;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + CAPTURE_SETTING_BASE);
	cnt_sensor_regs_setting = (toolBase->b_code1) & 0x0000ffff;
	MP_ALERT("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);

	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(CAPTURE_SETTING_BASE+4));
		  temp =  toolBase->b_code1;
	   	  temp2 = toolBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);

}



static void SensorTool_CaptureKey_5M_2560x1920(void)
{

	MP_ALERT("## %s ##", __FUNCTION__);
	  
	SENSOR_REGS sensor_setting;
	U32   cnt_sensor_regs_setting;
	int   i =0;
	TOOL  *toolBase;
	TOOL  *toolAddrTemp;

	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;
	

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);
	

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}

  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);


#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem_capture_flow;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + CAPTURE_SETTING_BASE);
	cnt_sensor_regs_setting = (toolBase->b_code1) & 0x0000ffff;
	MP_ALERT("%s:cnt_sensor_regs_setting = %d", __FUNCTION__, cnt_sensor_regs_setting);

	for (i=0; i<cnt_sensor_regs_setting; i++){
		  toolBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*4) +(CAPTURE_SETTING_BASE+4));
		  temp =  toolBase->b_code1;
	   	  temp2 = toolBase->b_code2;
		     
		  sensor_setting.addr = (unsigned short)(temp & 0x0000ffff);
		  sensor_setting.value= (unsigned short)((temp & 0xffff0000)>>16);
		  sensor_setting.delay= (BYTE) (temp2 & 0x000000ff);

		  Sensor_I2C_WriteFunc(SensorSlaveAddr, sensor_setting.addr, sensor_setting.value);//

#if 0 //For debug only
		  MP_DEBUG1("-- %d --", i);
		  MP_DEBUG1("SensorSlaveAddr = 0x%x",SensorSlaveAddr);
		  MP_DEBUG1("sensor_setting.addr =0x%x",sensor_setting.addr);
		  MP_DEBUG1("sensor_setting.value=0x%x",sensor_setting.value);
		  MP_DEBUG1("sensor_setting.delay=0x%x",sensor_setting.delay);	  
#endif		  
	} 

	MP_DEBUG1("### %s: Set sensor finish. ###", __FUNCTION__);

}




//frank lin capture flow
void SensorTool_Capture_KeySetting(BYTE IMGSIZE_ID)//(BYTE IMGSIZE_ID)
{

  switch(IMGSIZE_ID)
  {
  	case SIZE_VGA_640x480:
         SensorTool_CaptureKey_VGA_640x480();		
		break;
		
    case SIZE_XGA_1024x768:
		SensorTool_CaptureKey_XGA_1024x768();
		break;
	
    case SIZE_SXGA_1280x1024:
		SensorTool_CaptureKey_SXGA_1280x1024();
		break;
	
  	case SIZE_2M_1600x1200:
		SensorTool_CaptureKey_UXGA_1600x1200();
		break;

	case SIZE_QXGA_2048x1536:
		SensorTool_CaptureKey_QXGA_2048x1536();
		break;

	case SIZE_5M_2560x1920:
		SensorTool_CaptureKey_5M_2560x1920();
		break;

	default:
		MP_ALERT("%s: Not support capture size.", __FUNCTION__);
  }
	
}


void SensorTool_Capture_preview(void)
{
	MP_ALERT("## %s ##", __FUNCTION__);			
	SensorTool_setVGA_640x480();											                     
}





#if defined(SENSOR_TYPE_MAGIC_SENSOR)

DWORD SensorTool_Read_I2C(DWORD Mapping_of_Content, BYTE *Debug_mem_addr_ptr)
{	
	int i, j;
    unsigned int shift_bits,src_shift_bits, mask_bits, r_data;
	DWORD cnt_regs;
	BITS_ASSIGN reg;
	DWORD getValue;

	TOOL  *toolAddrTemp;
	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;

	SENSOR_REGS sensor_setting;
	TOOL  *toolRegBase;
	U32   R_Reg_data;
	WORD  Read_Addr;
	DWORD TotalReadValue;


#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
		Sensor_I2C_ReadFunc  = I2C_Read_Addr16_Value8_LoopMode;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
		
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}
  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)Debug_mem_addr_ptr;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolRegBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + Mapping_of_Content);
	R_Reg_data  = (toolRegBase->b_code1) & 0x0000ffff;
	cnt_regs    = R_Reg_data;
	MP_DEBUG2("%s:Read Gain ##cnt_regs = %d", __FUNCTION__, cnt_regs);

	TotalReadValue =0;
	for (i=0; i<cnt_regs; i++){
		MP_DEBUG1("-- %d --", i);
	 	toolRegBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*8) +(Mapping_of_Content+4));
	 	temp =  toolRegBase->b_code1;
	 	temp2 = toolRegBase->b_code2;
		MP_DEBUG2("%s:temp  =0x%x", __FUNCTION__, temp);
 		MP_DEBUG2("%s:temp2 =0x%x", __FUNCTION__, temp2);

	 	Read_Addr = temp;
	 	reg.src_start_bit = (BYTE) ((temp2 & 0xff000000)>>24);
	 	reg.src_end_bit   = (BYTE) ((temp2 & 0x00ff0000)>>16);
	 	reg.start_bit = (BYTE) ((temp2 & 0x0000ff00)>>8);
	 	reg.end_bit   = (BYTE) ((temp2 & 0x000000ff));

	 	MP_DEBUG2("%s:Read_Addr     	= 0x%x", __FUNCTION__, Read_Addr);
	 	MP_DEBUG2("%s:reg.src_start_bit	= 0x%x", __FUNCTION__, reg.src_start_bit);
	 	MP_DEBUG2("%s:reg.src_end_bit   = 0x%x", __FUNCTION__, reg.src_end_bit);
	 	MP_DEBUG2("%s:reg.start_bit     = 0x%x", __FUNCTION__, reg.start_bit);
	 	MP_DEBUG2("%s:reg.end_bit       = 0x%x", __FUNCTION__, reg.end_bit);

		src_shift_bits =reg.src_start_bit;
	 	shift_bits = reg.start_bit;
	 	j = reg.end_bit-reg.start_bit;
	 
		for(j, mask_bits=1; j>0; j--)	{mask_bits<<=1; mask_bits+=1;}

	 	getValue = Sensor_I2C_ReadFunc(SensorSlaveAddr>>1, Read_Addr);

	 	r_data = getValue >> shift_bits;
	 	r_data = r_data & mask_bits;

	 	MP_DEBUG1("r_data =0x%x",r_data);

		TotalReadValue = TotalReadValue |(r_data<<src_shift_bits);

     	MP_DEBUG2("%s: TotalReadValue 0x%x", __FUNCTION__,TotalReadValue);
	
 	} 

	MP_DEBUG2("%s: TotalReadValue 0x%x", __FUNCTION__,TotalReadValue);
  	return TotalReadValue;
//__asm("break 100");
}




DWORD SensorTool_Write_I2C(DWORD Mapping_of_Content, DWORD Total_Write_data, BYTE *Debug_mem_addr_ptr)
{	
	int i, j;
    unsigned int shift_bits,dst_shift_bits, mask_bits, r_data, w_data;
	DWORD cnt_regs;
	BITS_ASSIGN reg;
	DWORD getValue;

	TOOL  *toolAddrTemp;
	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;

	SENSOR_REGS sensor_setting;
	TOOL  *toolRegBase;
	U32   R_Reg_data;
	WORD  Read_Addr;
	DWORD TotalReadValue;
	DWORD mask_total_writereg_data;
	DWORD Total_Writereg_data;


#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif


	toolAddrTemp  = (TOOL *)((U32)start_addr_PC_tool_Ptr + I2C_TYPE);
	temp		  = toolAddrTemp->b_code1;
	I2C_Data_Type = (temp & 0x00ff0000) >> 16;
	I2C_Addr_Type = (temp & 0x0000ff00) >> 8;
	I2C_HW_SW     = (temp & 0x000000ff);
	MP_DEBUG2("%s: I2C_Data_Type = %d", __FUNCTION__, I2C_Data_Type);
	MP_DEBUG2("%s: I2C_Addr_Type = %d", __FUNCTION__, I2C_Addr_Type);
	MP_DEBUG2("%s: I2C_HW_SW     = %d", __FUNCTION__, I2C_HW_SW);

	if((I2C_Addr_Type == I2C_TYPE_BIT16) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg16Data8;
		Sensor_I2C_ReadFunc  = I2C_Read_Addr16_Value8_LoopMode;
	}
	else if((I2C_Addr_Type == I2C_TYPE_BIT8) && (I2C_Data_Type == I2C_TYPE_BIT8)) 
	{
		Sensor_I2C_WriteFunc = I2CM_WtReg8Data8;
		
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}
	else
	{
		MP_ALERT("%s: Not support I2C_Addr_Type = %d, I2C_Addr_Type =%d", __FUNCTION__, I2C_Addr_Type, I2C_Addr_Type);
		__asm("break 100");
	}
  
	toolAddrTemp = (TOOL *)((U32)start_addr_PC_tool_Ptr + SENSOR_SLAVE_DEVICE_WRITE_ADDR);
	SensorSlaveAddr = (BYTE)toolAddrTemp->b_code1;
	MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);

#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	start_addr_PC_tool_Ptr = (BYTE *)Debug_mem_addr_ptr;
#else
	start_addr_PC_tool_Ptr = start_addr_PC_tool;
#endif

	toolRegBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + Mapping_of_Content);
	R_Reg_data  = (toolRegBase->b_code1) & 0x0000ffff;
	cnt_regs    = R_Reg_data;
	MP_DEBUG2("%s:Read  ##cnt_regs = 0x%x", __FUNCTION__, cnt_regs);

	TotalReadValue =0;
	for (i=0; i<cnt_regs; i++){
		MP_DEBUG1("-- %d --", i);
	 	toolRegBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + (i*8) +(Mapping_of_Content+4));
	 	temp =  toolRegBase->b_code1;
	 	temp2 = toolRegBase->b_code2;
		MP_DEBUG2("%s:temp  =0x%x", __FUNCTION__, temp);
 		MP_DEBUG2("%s:temp2 =0x%x", __FUNCTION__, temp2);

	 	Read_Addr = temp;
	 	reg.src_start_bit = (BYTE) ((temp2 & 0xff000000)>>24);
	 	reg.src_end_bit   = (BYTE) ((temp2 & 0x00ff0000)>>16);
	 	reg.start_bit = (BYTE) ((temp2 & 0x0000ff00)>>8);
	 	reg.end_bit   = (BYTE) ((temp2 & 0x000000ff));

	 	MP_DEBUG2("%s:Read_Addr     	= 0x%x", __FUNCTION__, Read_Addr);
	 	MP_DEBUG2("%s:reg.src_start_bit	= 0x%x", __FUNCTION__, reg.src_start_bit);
	 	MP_DEBUG2("%s:reg.src_end_bit   = 0x%x", __FUNCTION__, reg.src_end_bit);
	 	MP_DEBUG2("%s:reg.start_bit     = 0x%x", __FUNCTION__, reg.start_bit);
	 	MP_DEBUG2("%s:reg.end_bit       = 0x%x", __FUNCTION__, reg.end_bit);

	 	getValue = Sensor_I2C_ReadFunc(SensorSlaveAddr>>1, Read_Addr);
		MP_DEBUG2("%s: getValue from sensor =0x%x", __FUNCTION__, getValue);


		dst_shift_bits =reg.start_bit;
	 	shift_bits = reg.src_start_bit;
	 	j = reg.src_end_bit-reg.src_start_bit;
	 
		for(j, mask_bits=1; j>0; j--)	{mask_bits<<=1; mask_bits+=1;}

	 	TotalReadValue = getValue;

		w_data = Total_Write_data >> shift_bits;
	 	w_data = w_data & mask_bits;
		MP_DEBUG2("%s:w_data =0x%x", __FUNCTION__, w_data);

		mask_total_writereg_data = 0xffffffff;
		mask_total_writereg_data &= ~(mask_bits<<dst_shift_bits);

		MP_DEBUG2("%s:mask_total_write_data =0x%x", __FUNCTION__, mask_total_writereg_data);

		Total_Writereg_data = TotalReadValue & mask_total_writereg_data;
		MP_DEBUG2("%s:After mask Total_Writereg_data =0x%x", __FUNCTION__, Total_Writereg_data);

		Total_Writereg_data  = Total_Writereg_data | (w_data<<dst_shift_bits);
		
     	MP_DEBUG2("%s: input Total_Write_data 0x%x", __FUNCTION__,Total_Write_data);	
 		MP_DEBUG2("%s: ##write to reg __Total_Writereg_data =0x%x ###", __FUNCTION__, Total_Writereg_data);
		MP_DEBUG2("%s: SensorSlaveAddr = 0x%x", __FUNCTION__, SensorSlaveAddr);
		MP_DEBUG2("%s: Write I2C addr = 0x%x", __FUNCTION__, Read_Addr);
		MP_DEBUG2("%s: Write data = 0x%x", __FUNCTION__, Total_Writereg_data);
		Sensor_I2C_WriteFunc(SensorSlaveAddr, Read_Addr, Total_Writereg_data);//
		
	
 	} 

  	return PASS;

}





#endif



int Sensor_Capture_SensorTool(STREAM *fileHandle, BYTE Capture_IMGSIZE_ID)
{
	MP_ALERT("## %s ##", __FUNCTION__);		

 //reference  old OV5642	
	MP_DEBUG1("###### %s ######", __FUNCTION__);
	int iErrorCode;
	extern int SensorInputWidth;
    extern int SensorInputHeight;

	//---- OV caputure flow ----
	int Capture_Framerate = 3000;//depend on  Capture setting
	int Lines_10ms;
	int Capture_MaxLines;
	int Preview_FrameRate = 3000;////depend on  Preview setting
	int Preview_Maxlines;
	
	DWORD ulCapture_Exposure;
	DWORD ulCapture_Exposure_Gain;
	DWORD ulPreviewExposure;
	DWORD iCapture_Gain;

	DWORD Gain;
	BYTE ExposureLow;
	BYTE ExposureMid;
	BYTE ExposureHigh;
	BYTE PreviewMaxlineHigh;
	BYTE PreviewMaxlineLow;
	
	BYTE CaptureMaxlineHigh;
	BYTE CaptureMaxlineLow;
	
	DWORD GetValue;

	extern int CaptureCloseDisplay_Flag;

	IPU *ipu = (IPU *) IPU_BASE;

#if defined(SENSOR_TYPE_MAGIC_SENSOR)	

	int i, j;
    unsigned int shift_bits,src_shift_bits, mask_bits, r_data;
	DWORD cnt_regs;
	BITS_ASSIGN reg;
	DWORD getValue;

	TOOL  *toolAddrTemp;
	U32   temp, temp1, temp2;
	BYTE  *start_addr_PC_tool_Ptr;
	BYTE  SensorSlaveAddr;
	BYTE  I2C_Data_Type, I2C_Addr_Type, I2C_HW_SW;

	SENSOR_REGS sensor_setting;
	TOOL  *toolRegBase;
	U32   R_Reg_data;
	WORD  Read_Addr;
#endif	
	
	//stop AEC/AGC 
	MP_DEBUG1("%s: stop AEC/AGC", __FUNCTION__);
	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_VTS, 1, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_VTS, 1, NULL);
	#endif

	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_AGC, 1, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_AGC, 1, NULL);
	#endif

	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_AEC, 1, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_AEC, 1, NULL);
	#endif



	//read the register 0x350*

	//	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x350b);
	//	Gain = (BYTE) (GetValue & 0xff);
#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
	Gain = SensorTool_Read_I2C(MAPPING_OF_GAIN, start_addr_PC_tool_mem_capture_flow);
#else
	Gain = SensorTool_Read_I2C(MAPPING_OF_GAIN, NULL);
#endif
	MP_DEBUG2("%s:Gain = 0x%x", __FUNCTION__, Gain);

	/*-ulPreviewExposure-*/
	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		GetValue = SensorTool_Read_I2C(MAPPING_OF_EXPOSURE, start_addr_PC_tool_mem_capture_flow);
	#else
		GetValue = SensorTool_Read_I2C(MAPPING_OF_EXPOSURE, NULL);
	#endif
 
	ulPreviewExposure = GetValue >> 4;

	MP_DEBUG1("--Sensor Tool-ulPreviewExposure =0x%x",ulPreviewExposure);


	/*Preview_Maxlines*/
#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		GetValue = SensorTool_Read_I2C(MAPPING_OF_ADJ_MAXLINE, start_addr_PC_tool_mem_capture_flow);
#else
		GetValue = SensorTool_Read_I2C(MAPPING_OF_ADJ_MAXLINE , NULL);
#endif
	Preview_Maxlines = GetValue;
	MP_DEBUG2("%s: Preview_Maxlines =0x%x", __FUNCTION__, Preview_Maxlines);


	switch(Capture_IMGSIZE_ID)
	{
		case SIZE_5M_2560x1920:
	   		sensor_IMGSIZE_ID = SIZE_5M_2560x1920;//SIZE_2M_1600x1200;
	   		SensorInputWidth  = 2560;
	   		SensorInputHeight = 1920;
	   		break;

		case SIZE_QXGA_2048x1536:
			sensor_IMGSIZE_ID  = SIZE_QXGA_2048x1536;
	   		SensorInputWidth   = 2048;
	   		SensorInputHeight  = 1536;
			break;

		case SIZE_2M_1600x1200:
	   		sensor_IMGSIZE_ID = SIZE_2M_1600x1200;
	   		SensorInputWidth  = 1600;
	   		SensorInputHeight = 1200;
	   		break;

		case SIZE_SXGA_1280x1024:
		   	sensor_IMGSIZE_ID = SIZE_SXGA_1280x1024;
	   		SensorInputWidth  = 1280;
	   		SensorInputHeight = 1024;
	   		break;

       	case SIZE_XGA_1024x768:
	   		sensor_IMGSIZE_ID = SIZE_XGA_1024x768;
	   		SensorInputWidth  = 1024;
	   		SensorInputHeight = 768;
	   		break;

	    case SIZE_VGA_640x480:
	   		sensor_IMGSIZE_ID = SIZE_VGA_640x480;
	   		SensorInputWidth  = 640;
	   		SensorInputHeight = 480;
	   		break;
	   
		default:
			MP_ALERT("%s:Not Support capture IMGSIZE. ID=%d", __FUNCTION__,Capture_IMGSIZE_ID);
			goto Error;
	}

    
    /*Before change to big resolution need to free the older memory*/
	/*If Usign double buffer need to add free memory here.*/
    extern ST_IMGWIN SensorInWin[3];
	ipu->Ipu_reg_100 &= ~BIT0; //close SenInEn
	while(1)
	{
		if((ipu->Ipu_reg_100&BIT0)==0)//確定有寫進去
			break;
	}

    if(sensor_mode==MODE_CAPTURE)
	{
		if(g_bImageData_Buf != NULL)
		{
			ext_mem_free(g_bImageData_Buf);
			g_bImageData_Buf        = NULL;
			SensorInWin[0].pdwStart = NULL;
		}
    }
	else
	{
		MP_ALERT("%s:Error Not caputre mode (sensor_mode =%d)", __FUNCTION__, sensor_mode);
		goto Error;
	}

	
    /*During capture , disable the display path*/	
	CaptureCloseDisplay_Flag = 1;
		Set_Display_flag(DISABLE);/*close IPW2*/
		
		//change resolution from VGA to ....
		SensorTool_Capture_KeySetting(sensor_IMGSIZE_ID);//input ImageSize

	   	Sensor_Run_capture();
	   	Global_Sensor_Run();
	CaptureCloseDisplay_Flag = 0;
	
	//------------

    //stop AEC/AGC 
#if 0    
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);
#else
	
	MP_DEBUG1("%s: stop AEC/AGC", __FUNCTION__);
	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_VTS, 1, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_VTS, 1, NULL);
	#endif

	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_AGC, 1, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_AGC, 1, NULL);
	#endif

	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_AEC, 1, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_AEC, 1, NULL);
	#endif

#endif
	
	//read 0x350c 0x350d   
#if 0	
	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x350c);
	CaptureMaxlineLow = (BYTE) (GetValue & 0xff);
	   
	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x350d);
	CaptureMaxlineLow = (BYTE) (GetValue & 0xff);
	
	//Preview_Maxlines = 256*PreviewMaxlineHigh + PreviewMaxlineLow;
	Capture_MaxLines = 256*CaptureMaxlineHigh + CaptureMaxlineLow;
#else

	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		GetValue = SensorTool_Read_I2C( MAPPING_OF_ADJ_MAXLINE, start_addr_PC_tool_mem_capture_flow);
	#else
		GetValue = SensorTool_Read_I2C( MAPPING_OF_ADJ_MAXLINE , NULL);
	#endif

	Capture_MaxLines  = GetValue;
	MP_DEBUG2("%s:Capture_MaxLines = 0x%x", __FUNCTION__, Capture_MaxLines);
#endif
	
	if(1)//(m_60Hz==true)
	{
	 Lines_10ms = Capture_Framerate*Capture_MaxLines/12000;
	}
	else
	{
	 Lines_10ms = Capture_Framerate * Capture_MaxLines/10000;
	}

	//Lines_10ms *= 6;//franklin test
	//ulPreviewExposure = ((DWORD)(ExposureHigh))<<12 ;
	//ulPreviewExposure += ((DWORD)ExposureMid)<<4 ;
	//ulPreviewExposure += (ExposureLow >>4);
	
	if(0 == Preview_Maxlines ||0 ==Preview_FrameRate ||0== Lines_10ms)
	{
	  goto Error;
	}
	
    ulCapture_Exposure =(ulPreviewExposure*(Capture_Framerate)*(Capture_MaxLines))/(((Preview_Maxlines)*(Preview_FrameRate)));
	iCapture_Gain = (Gain & 0x0f) + 16;
	if (Gain & 0x10)
	{
	  iCapture_Gain = iCapture_Gain << 1;
	}
	if (Gain & 0x20)
	{
	  iCapture_Gain = iCapture_Gain << 1;
	}
	if (Gain & 0x40)
	{
	  iCapture_Gain = iCapture_Gain << 1;
	}
	if (Gain & 0x80)
	{
	  iCapture_Gain = iCapture_Gain << 1;
	}
	ulCapture_Exposure_Gain = ulCapture_Exposure * iCapture_Gain;
	if(ulCapture_Exposure_Gain < ((LONG)(Capture_MaxLines)*16))
	{
	  ulCapture_Exposure = ulCapture_Exposure_Gain/16;
	  if (ulCapture_Exposure > Lines_10ms)
	  {
		ulCapture_Exposure /= Lines_10ms;
		ulCapture_Exposure *= Lines_10ms;
	  }
	}
	else
	{
		ulCapture_Exposure = Capture_MaxLines;
	} 
	
	if(ulCapture_Exposure == 0)
	{
	  ulCapture_Exposure = 1;
	}
	iCapture_Gain = (ulCapture_Exposure_Gain*2/ulCapture_Exposure + 1)/2;
	
    //ExposureLow = ((BYTE)ulCapture_Exposure)<<4;
	//ExposureMid = (BYTE)(ulCapture_Exposure >> 4) & 0xff;
	//ExposureHigh = (BYTE)(ulCapture_Exposure >> 12);
	  
	#if 0 
	  ulCapture_Exposure_end=ulCapture_Exposure;
	  iCapture_Gain_end=iCapture_Gain;
	#endif 
    MP_DEBUG1("-----### ulCapture_Exposure=%d",ulCapture_Exposure);
    MP_DEBUG1("-----### iCapture_Gain=%d", iCapture_Gain);
	
	Gain = 0;
	if (iCapture_Gain > 31)
	{
	  Gain |= 0x10;
	  iCapture_Gain = iCapture_Gain >> 1;
	}
	if (iCapture_Gain > 31)
	{
	  Gain |= 0x20;
	  iCapture_Gain = iCapture_Gain >> 1;
	}
	if (iCapture_Gain > 31)
	{
	  Gain |= 0x40;
      iCapture_Gain = iCapture_Gain >> 1;
	}
	if (iCapture_Gain > 31)
	{
	  Gain |= 0x80;
	  iCapture_Gain = iCapture_Gain >> 1;
	}
	if (iCapture_Gain > 16)
	{
	  Gain |= ((iCapture_Gain -16) & 0x0f);
	}
	if(Gain==0x10)
	 {Gain=0x11;}
	
	//write the gain and exposoure to 0x350*
#if 0	
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,Gain);  
#else
	MP_DEBUG2("%s:Write to sensro Gain= 0x%x ", __FUNCTION__, Gain);
	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_GAIN, Gain, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_GAIN, Gain, NULL);
	#endif
#endif

#if 0
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,ExposureLow);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,ExposureMid);  
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3500,ExposureHigh);
#else
	MP_DEBUG2("%s:Write to sensro Gain= 0x%x ", __FUNCTION__, Gain);
	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_EXPOSURE, ulCapture_Exposure, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_EXPOSURE, ulCapture_Exposure, NULL);
	#endif


#endif	  
    //---- OV caputure flow End---- 

	iErrorCode = Sensor_Capture_Encode(fileHandle, SensorInputWidth, SensorInputHeight,sensor_IMGSIZE_ID);	
    /*After here the big memory for capture. the memory is free after Sensor_Capture_Encode*/

	if(iErrorCode == PASS)
	{
        MP_ALERT("%s:Capture end!! (Successful)", __FUNCTION__);
	}
	else
	{
		MP_ALERT("%s:Capture end!! (Fail)", __FUNCTION__);
	}
	
    sensor_IMGSIZE_ID = SIZE_VGA_640x480;
	SensorInputWidth  = 640;
	SensorInputHeight = 480;

	//start AEC/AGC 
#if 0	
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);
#else
	MP_DEBUG1("%s: start AEC/AGC", __FUNCTION__);
	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_VTS, 0, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_VTS, 0, NULL);
	#endif

	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_AGC, 0, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_AGC, 0, NULL);
	#endif

	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		SensorTool_Write_I2C(MAPPING_OF_AEC, 0, start_addr_PC_tool_mem_capture_flow);
	#else
		SensorTool_Write_I2C(MAPPING_OF_AEC, 0, NULL);
	#endif

#endif
	//----Back to preview mode  ------
	SensorTool_Capture_preview();
	Set_Display_flag(ENABLE);/*Open IPW2*/
	Sensor_Run();
	Global_Sensor_Run();

	ipu->Ipu_reg_100 |= BIT0; //SenInEn

	//----Back to preview mode  ------
	
 
	return iErrorCode;

Error:
	//API_Sensor_Stop();
	/*When exit caputer mod, need to call API_Sensor_Stop() to free preview memoy */
	return FAIL;
	
}





void Drive_Sensor_SensorTool(void)
{

		I2CM_FreqChg(0x78, 300000);

		if(sensor_mode==MODE_RECORD)
		{
				if(sensor_IMGSIZE_ID==SIZE_CIF_352x240)
				{
						SensorTool_set_352x240_30FPS();
				}else	if(sensor_IMGSIZE_ID==SIZE_VGA_640x480)
				{
						SensorTool_setVGA_640x480_30FPS();
				}else if(sensor_IMGSIZE_ID==SIZE_480P_720x480)
				{
						SensorTool_set_720x480_30FPS();
				}else if(sensor_IMGSIZE_ID==SIZE_XGA_1024x768)
				{
						SensorTool_setXGA_1024x768_30FPS();
				}else{//default 720p
						SensorTool_set_720P_1280x720();
				}

    
		}else if(sensor_mode==MODE_CAPTURE)
		{
            //capture preview
            MP_ALERT("########  capture preview mode (VGA)------"); 
			SensorTool_Capture_preview();

		}else if(sensor_mode==MODE_PCCAM)
		{
				if(sensor_IMGSIZE_ID==SIZE_QVGA_320x240)
						SensorTool_setQVGA_320x240_30FPS();
				else
						SensorTool_set_176x144();
		}else
		{
			MP_ALERT("%s: Not support sensor_mode=%d", __FUNCTION__, sensor_mode);
			__asm("break 100");
		}

}
#endif

#endif

