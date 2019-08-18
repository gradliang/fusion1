/*
 *	2007/11/01
 *
 *	This file is used to tune slide show effect finely
 *	Revise defines about target slide show for fine tuning. 
 *
 */

#define ENABLE_ALL_SLIDEEFFECT		ENABLE

#if ENABLE_ALL_SLIDEEFFECT

#define  SLIDE_TRANSITION_FADE      	ENABLE
#define  SLIDE_TRANSITION_SHUTTER   	DISABLE
#define  SLIDE_TRANSITION_CROSS     	DISABLE
#define  SLIDE_TRANSITION_MASK      	DISABLE
#define  SLIDE_TRANSITION_BRICK     	DISABLE		//unstable need to check more
#define  SLIDE_TRANSITION_DISSOLVE  	DISABLE
#define  SLIDE_TRANSITION_BAR       	DISABLE
#define  SLIDE_TRANSITION_EXPANSION 	ENABLE
#define  SLIDE_TRANSITION_SILK      	DISABLE
#define  SLIDE_TRANSITION_SKETCH    	ENABLE
#define  SLIDE_TRANSITION_SCROLL    	ENABLE 
#define  SLIDE_TRANSITION_GRID    		ENABLE
#define  SLIDE_TRANSITION_KEN_BURNS    	ENABLE
#define  SLIDE_TRANSITION_BULLENTINBOARD DISABLE
#define  SLIDE_TRANSITION_3DCUBE    	ENABLE
#define  SLIDE_TRANSITION_3DFLIP    	ENABLE
#define  SLIDE_TRANSITION_3DSWAP    	ENABLE
#define  SLIDE_TRANSITION_PUSH			ENABLE
#define  SLIDE_TRANSITION_MULTIEFFECT	DISABLE

#else

#define  SLIDE_TRANSITION_FADE      	DISABLE
#define  SLIDE_TRANSITION_SHUTTER   	DISABLE
#define  SLIDE_TRANSITION_CROSS    		DISABLE
#define  SLIDE_TRANSITION_MASK      	DISABLE
#define  SLIDE_TRANSITION_BRICK     	DISABLE
#define  SLIDE_TRANSITION_DISSOLVE  	DISABLE
#define  SLIDE_TRANSITION_BAR       	DISABLE
#define  SLIDE_TRANSITION_EXPANSION 	DISABLE
#define  SLIDE_TRANSITION_SILK      	DISABLE
#define  SLIDE_TRANSITION_SKETCH    	DISABLE
#define  SLIDE_TRANSITION_SCROLL    	DISABLE
#define  SLIDE_TRANSITION_GRID    		DISABLE
#define  SLIDE_TRANSITION_KEN_BURNS    	DISABLE
#define  SLIDE_TRANSITION_BULLENTINBOARD DISABLE
#define  SLIDE_TRANSITION_3DCUBE    	DISABLE
#define  SLIDE_TRANSITION_3DFLIP    	DISABLE
#define  SLIDE_TRANSITION_3DSWAP    	DISABLE	
#define  SLIDE_TRANSITION_PUSH			DISABLE
#define  SLIDE_TRANSITION_MULTIEFFECT	DISABLE

#endif
 

#define SLIDEEFFECTTIME     2

/***** Fade *****/
#define FADE_DELAY			60	

/***** Shutter *****/
#define BLINDS_SLIDE_STEP	12
#define SHUTTER_SLIDE_STEP	13	//Default value : 13
#define BLINDS_DELAY		52
#define SHUTTER_DELAY		52

/***** Cross Comb *****/
#define CROSSCOMBO_DELAY	4
#define CROSSCOMBO_WIDTH	4
#define CROSSCOMBO_HEIGHT	30	//Default value : 16

/***** Mask *****/
#define MASK_DELAY			40


/***** Brick *****/

#define BRICK_ROW_COUNT		10
#define BRICK_COLUMN_COUNT	10

#define BRICK_DELAY			12
#define BRICK_COUNT			(BRICK_ROW_COUNT * BRICK_COLUMN_COUNT)

/***** Dissolve *****/
#define DISSOLVE_DELAY		4

/***** Bar *****/
#define BAR_TOPDOWN_HEIGHT	20
#define BAR_LR_WIDTH		40	
#define BAR_INOUT_Width		16	// (Window's Width/2) % BAR_INOUT_Width      should equal 0    
#define BAR_INOUT_HEIGHT	2	// (Window's heigth/2) % BAR_INOUT_HEIGHT  should equal 0
#define RANDOMBAR_STEP		32	// default value : 37

#define BAR_TOPDOWN_DELAY	40
#define BAR_LR_DELAY		40
#define BAR_INOUTH_DELAY	20
#define BAR_INOUTV_DELAY	20

#define RANDOMBAR_DELAY		12

/***** Expansion *****/
#define SLIDE_LINEEXPANSION_STEP	40	//default value :40
#define LINEEXPANSION_DELAY			12
#define SLIDE_EXPAN_STEP			40	//default value :20
#define CENTRALEXPANSION_DELAY		16

/***** Silk *****/
#define SILK_DELAY 8

/*****Ken Burns Effects *****/
#define FACE_DETECTION ENABLE	//If need to use, add libvision in makefile
#define MEASURE_DETECTION_TIME DISABLE

#define KEN_BURNS_STEP		10
#define KEN_BURNS_DELAY 	20
#define KEN_BURNS_DELAY_VIEW 	1000
#define KEN_BURNS_HELF_VIEW_X 	200
#define KEN_BURNS_HELF_VIEW_Y 	120
#define KEN_BURNS_TRACE_NUMBER 	6

#define KEN_BURNS_INTERPOLUTION_LOOP 9
#define KEN_BURNS_INTERPOLUTION_STEP 2


/***** Grid *****/
#define SGRID_DELAY 20


/***** Scroll *****/
#define SCROLL_DELAY 40
#define SCROLL_WIDTH 32



/*****Crawling *****/
#define MOVE_TOP_DOWN		1
#define MOVE_BOTTOM_UP		2
#define MOVE_LEFT_RIGHT		3
#define MOVE_RIGHT_LEFT		0

/***** 3D Cube *****/
#define CUBE3D_DELAY 20
#define CUBE3D_STEP 40

/***** 3D Flip *****/
#define FLIP3D_STEP 40

/***** 3D Swap *****/
#define SWAP3D_DELAY 40

/***** Push *****/
#define PUSH_H_STEP		40
#define PUSH_V_STEP		30
#define PUSH_SPEED_LINE	16


/***** MultiEffect *****/
#define MULTIEFFECT_PUSH_DELAY 	16
#define MULTIEFFECT_ROTATE_DELAY 	8


/***** GRID *****/
#define GRID_ROWS 		4
#define GRID_COLUMES    8
#define GRID_LINEWIDTH  4
#define GRID_LINECOLOR      0x00008080
#define GRID_LINECOLOR2     0xffff8080
#define COLOR_BACKGROUND    0x00008080

#define EFFECT_STEP 12
#define HALF_STEP 	(EFFECT_STEP / 2)
#define GRID_DELAY	4


/***** SKETCH *****/
#define SKETCH_COLORMIX_RANGE	0x7F	// 0x01 ~ 0x7F
#define SKETCH_COLORMIX_STEP	4		// min 1


enum{
    SETUP_MENU_TRANSITION_OFF       ,   // 0
#if SLIDE_TRANSITION_FADE
    SETUP_MENU_TRANSITION_FADE      ,   // 1
#endif
#if SLIDE_TRANSITION_SHUTTER
    SETUP_MENU_TRANSITION_SHUTTER   ,   // 2
#endif
#if SLIDE_TRANSITION_CROSS
    SETUP_MENU_TRANSITION_CROSS     ,   // 3
#endif
#if SLIDE_TRANSITION_MASK
    SETUP_MENU_TRANSITION_MASK      ,   // 4
#endif
#if SLIDE_TRANSITION_BRICK
    SETUP_MENU_TRANSITION_BRICK     ,   // 5
#endif
#if SLIDE_TRANSITION_DISSOLVE
    SETUP_MENU_TRANSITION_DISSOLVE  ,   // 6
#endif
#if SLIDE_TRANSITION_BAR
    SETUP_MENU_TRANSITION_BAR       ,   // 7
#endif
#if SLIDE_TRANSITION_EXPANSION
    SETUP_MENU_TRANSITION_EXPANSION ,   // 8
#endif
#if SLIDE_TRANSITION_SILK
    SETUP_MENU_TRANSITION_SILK      ,   // 9
#endif
#if SLIDE_TRANSITION_SKETCH
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))//TYChen add Sketch Slideshow
    SETUP_MENU_TRANSITION_SKETCH    ,   // 10
#endif
#endif

#if SLIDE_TRANSITION_GRID
	SETUP_MENU_TRANSITION_GRID	,		//11
#endif
#if SLIDE_TRANSITION_SCROLL
	SETUP_MENU_TRANSITION_SCROLL	,	//12
#endif
#if SLIDE_TRANSITION_KEN_BURNS
	SETUP_MENU_TRANSITION_KEN_BURNS	,	//13
#endif
#if SLIDE_TRANSITION_BULLENTINBOARD
	SETUP_MENU_TRANSITION_BULLENTIN_BOARD,//14
#endif
#if SLIDE_TRANSITION_3DCUBE
	SETUP_MENU_TRANSITION_3DCUBE,		//15
#endif
#if SLIDE_TRANSITION_3DFLIP
	SETUP_MENU_TRANSITION_3DFLIP,		//16
#endif
#if SLIDE_TRANSITION_3DSWAP
	SETUP_MENU_TRANSITION_3DSWAP,		//17
#endif
#if SLIDE_TRANSITION_PUSH
	SETUP_MENU_TRANSITION_PUSH,			//18
#endif
#if SLIDE_TRANSITION_MULTIEFFECT
	SETUP_MENU_TRANSITION_MULTIEFFECT,	//19
#endif
    SETUP_MENU_TRANSITION_RANDOM    ,   //20    // This must be the last one
};


// in Material

void Img_SetTransition(BYTE bFlag);
BYTE Img_GetTransitionStatus();
int SIN_S(int Angle);
int COS_S(int Angle);
void __Block_Copy(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin, 
				  int sx, int sy, int sw, int sh, int tx, int ty);
void WinCopy_Clip(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin,
				  int sx, int sy, int sw, int sh, int tx, int ty);
void RingCopy(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin, WORD x1, WORD y1, WORD w1, WORD h1, WORD x2, WORD y2, WORD w2, WORD h2);
void Slide_WinCopy(ST_IMGWIN * sWin, ST_IMGWIN * tWin);
BOOL CheckDimension(ST_IMGWIN * win);
void TransWinReset(ST_IMGWIN * srcWin, BYTE * buffer);
DWORD RandomGen();
ST_IMGWIN * Win_New(int wWidth, int wHeight);
void Win_Free(ST_IMGWIN * pWin);
BOOL __SlideReturn(DWORD dwStartTime, DWORD dwDelayTime);



