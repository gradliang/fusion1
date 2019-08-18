
#ifndef ICON_H
#define ICON_H


// define driver icon dimension
// note: driver icons are arranged horizontal
//       If you want to change to vertical arrangment, 
//       modify ReFlashDrvIcon() in main\source\driverSel.c
//       
#define DRV_ICON_START_X		80
#define DRV_ICON_START_Y		60
#define DRV_ICON_AREA_WIDTH		600
#define DRV_ICON_WIDTH 			100
#define DRV_ICON_HEIGHT			60
#define DRV_MARK_COLOR			1
#define DRV_MARK_LINEWIDTH		4


// define player icon dimension
// note: player icons are arranged horizontal
//       If you want to change to vertical arrangment, 
//       modify PutPlayerIcon() in main\source\PlayerSel.c
//   
#define NUM_OF_PLAYER			5
    
#define PLR0_ICON_START_X		50
#define PLR1_ICON_START_X		130
#define PLR2_ICON_START_X		210
#define PLR3_ICON_START_X		290
#define PLR4_ICON_START_X		370
#define PLR_ICON_START_Y		160
#define PLR_ICON_WIDTH 			60
#define PLR_ICON_HEIGHT			60
#define PLR_MARK_COLOR			1
#define PLR_MARK_LINEWIDTH		4


// define image thombnail win 
#define	TUB_PER_COLUMN			3			// number of image per column
#define	TUB_PER_ROW				3			// number of image per row
#define TUB_PER_WIN				TUB_PER_COLUMN * TUB_PER_ROW
#define	TUB_ICON_WIDTH			60			// width of thombnail image
#define TUB_ICON_HEIGHT			60			// height of thombnail image
#define	TUB_START_X				200			// horizontal start point
#define TUB_START_Y				100			// vertical start point
#define	TUB_V_GAP				20			// vertical gap
#define	TUB_H_GAP				20			// horizontal gap


// define image static win
#define STATIC_WIDTH			640
#define STATIC_HEIGHT			480
#define STATIC_START_X			200
#define STATIC_START_Y          100


// define color
#define BG_COLOR				0x00af9196
#define IMG_FONT_COLOR			0xff
#define OSD_FONT_COLOR			0x08

#define DBCS_START				1
#define FONT_ASCII				0

#endif  //ICON_H

