#ifndef __TC_GT9911_H
#define __TC_GT9911_H


#define GT911_I2C_ADDRESS      0xba // 0xba or 0x28

//TOUCH_PANEL_DRIVER_GT911
// Registers define
#define GTP_READ_COOR_ADDR    0x814E
#define GTP_REG_SLEEP         0x8040
#define GTP_REG_SENSOR_ID     0x814A
#define GTP_REG_CONFIG_DATA   0x8047
#define GTP_REG_VERSION       0x8140

#define RESOLUTION_LOC        3
#define TRIGGER_LOC           8

#define GTP_ADDR_LENGTH       2
#define GTP_CONFIG_MIN_LENGTH 186
#define GTP_CONFIG_MAX_LENGTH 240

#define	TOUCH_ROTATE											0
#define	TOUCH_ENTER_DEVELOP								50
#define TOUCH_MAX_POINT											2



#endif

