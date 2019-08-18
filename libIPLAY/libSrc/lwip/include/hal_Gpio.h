#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H

#include "typedef.h"

#define HAL_GPIO_GROUP_MASK     0xffff0000
#define HAL_GPIO_NUMBER_MASK    0x0000ffff

#define HAL_GPIO_GROUP_INDEX    0x00000000
#define HAL_SGPIO_GROUP_INDEX   0x00010000
#define HAL_VGPIO_GROUP_INDEX   0x00020000
#define HAL_AGPIO_GROUP_INDEX   0x00040000
#define HAL_FGPIO_GROUP_INDEX   0x00080000
#define HAL_UGPIO_GROUP_INDEX   0x00100000
#define HAL_EGPIO_GROUP_INDEX   0x00200000
#define HAL_HGPIO_GROUP_INDEX   0x00400000
#define HAL_TGPIO_GROUP_INDEX   0x00800000
#define HAL_MGPIO_GROUP_INDEX   0x01000000

#define DATA_LOW                0
#define DATA_HIGH               1

#ifndef POSITIVE_ACTIVE
#define POSITIVE_ACTIVE         0
#define NEGATIVE_ACTIVE         1

#define LEVEL_TRIGGER           0
#define EDGE_TRIGGER            1
#endif

enum
{
    GPIO_0=HAL_GPIO_GROUP_INDEX,
    GPIO_1,
    GPIO_2,
    GPIO_3,
    GPIO_4,
    GPIO_5,
    GPIO_6,
    GPIO_7,
    GPIO_8,
    GPIO_9,
    GPIO_10,
    GPIO_11,
    GPIO_12,
    GPIO_13,
    GPIO_14,
    GPIO_15,

    SGPIO_0=(HAL_SGPIO_GROUP_INDEX+(GPIO_15&HAL_GPIO_NUMBER_MASK)+1),
    SGPIO_1,
    SGPIO_2,
    SGPIO_3,
    SGPIO_4,
    SGPIO_5,
    SGPIO_6,
    SGPIO_7,
    SGPIO_8,

    VGPIO_0=(HAL_VGPIO_GROUP_INDEX+(SGPIO_8&HAL_GPIO_NUMBER_MASK)+1),
    VGPIO_1,
    VGPIO_2,
    VGPIO_3,
    VGPIO_4,
    VGPIO_5,
    VGPIO_6,
    VGPIO_7,
    VGPIO_8,
    VGPIO_9,
    VGPIO_10,
    VGPIO_11,
    VGPIO_12,
    VGPIO_13,
    VGPIO_14,
    VGPIO_15,
    VGPIO_16,
    VGPIO_17,
    VGPIO_18,
    VGPIO_19,
    VGPIO_20,
    VGPIO_21,
    VGPIO_22,
    VGPIO_23,
    VGPIO_24,
    VGPIO_25,
    VGPIO_26,
    VGPIO_27,

    AGPIO_0=(HAL_AGPIO_GROUP_INDEX+(VGPIO_27&HAL_GPIO_NUMBER_MASK)+1),
    AGPIO_1,
    AGPIO_2,
    AGPIO_3,

    FGPIO_0=(HAL_FGPIO_GROUP_INDEX+(AGPIO_3&HAL_GPIO_NUMBER_MASK)+1),
    FGPIO_1,
    FGPIO_2,
    FGPIO_3,
    FGPIO_4,
    FGPIO_5,
    FGPIO_6,
    FGPIO_7,
    FGPIO_8,
    FGPIO_9,
    FGPIO_10,
    FGPIO_11,
    FGPIO_12,
    FGPIO_13,
    FGPIO_14,
    FGPIO_15,
    FGPIO_16,
    FGPIO_17,
    FGPIO_18,
    FGPIO_19,
    FGPIO_20,
    FGPIO_21,
    FGPIO_22,
    FGPIO_23,

    UGPIO_0=(HAL_UGPIO_GROUP_INDEX+(FGPIO_23&HAL_GPIO_NUMBER_MASK)+1),
    UGPIO_1,
    UGPIO_2,
    UGPIO_3,

    EGPIO_0=(HAL_EGPIO_GROUP_INDEX+(UGPIO_3&HAL_GPIO_NUMBER_MASK)+1),
    EGPIO_1,
    EGPIO_2,
    EGPIO_3,
    EGPIO_4,
    EGPIO_5,
    EGPIO_6,
    EGPIO_7,

    HGPIO_0=(HAL_HGPIO_GROUP_INDEX+(EGPIO_7&HAL_GPIO_NUMBER_MASK)+1),
    HGPIO_1,
    HGPIO_2,
    HGPIO_3,
    HGPIO_4,
    HGPIO_5,
    HGPIO_6,
    HGPIO_7,
    HGPIO_8,
    HGPIO_9,
    HGPIO_10,
    HGPIO_11,
    HGPIO_12,
    HGPIO_13,
    HGPIO_14,
    HGPIO_15,
    HGPIO_16,
    HGPIO_17,
    HGPIO_18,
    HGPIO_19,
    HGPIO_20,
    HGPIO_21,
    HGPIO_22,
    HGPIO_23,
    HGPIO_24,
    HGPIO_HIU,

    TGPIO_0 = (HAL_TGPIO_GROUP_INDEX + (HGPIO_HIU & HAL_GPIO_NUMBER_MASK) + 1),
    TGPIO_1,
    TGPIO_2,
    TGPIO_3,
    TGPIO_4,
    TGPIO_5,
    TGPIO_6,
    TGPIO_7,
    TGPIO_8,
    TGPIO_9,
    TGPIO_10,
    TGPIO_11,

    MAX_GPIO_GROUP_INDEX = (0xFFFF + (TGPIO_11 & HAL_GPIO_NUMBER_MASK) + 1)
};


typedef struct
{
    U32 u32GroupAddr;
    U08 u08BitOffset;
    U08 bGpDir          : 1;
    U08 bGpDat          : 1;
    U08 bGpCfg          : 2;
    U08 bGpIntMode      : 1;
    U08 bGpIntPolarity  : 1;
    U08 bGpIntEnable    : 1;
} ST_GPIO_CONFIG_INFO;



///
///@ingroup	GPIO_CONTROL
///@brief	  Set a specific GPIO pin to the default setting
///
///@param	  gpioNum             the GPIO number
///
///@retval	NO_ERR			               No Error
///@retval	ERR_GPIO_INVALID_NUMBER    Invalid GPIO number
///
///@remark	This API would set the specific GPIO setting to the default setting.
///
S32 mpx_GpioDefaultSettingSet(U32 gpioNum);



///
///@ingroup	GPIO_CONTROL
///@brief	  Set the output of the GPIO pin to high pulse or low pulse
///
///@param	  gpioNum             the GPIO number
///@param   data                the high or low pulse
///
///@retval	NO_ERR			               No Error
///@retval	ERR_GPIO_INVALID_NUMBER    Invalid GPIO number
///
///@remark	If we want to pull high the GPIO pin, the data would be set to 1. And setting to 0 would pull the GPIO pin to low.
///
S32 mpx_GpioDataSet(U32 gpioNum, U32 data);



///
///@ingroup	GPIO_CONTROL
///@brief	  Get the input pulse of the GPIO pin
///
///@param	  gpioNum             the GPIO number
///@param   data                the high or low pulse
///
///@retval	NO_ERR			               No Error
///@retval	ERR_GPIO_INVALID_NUMBER    Invalid GPIO number
///
///@remark	The API would get the input pulse of the specific GPIO pin. The data value would return the pulse level. HIGH is 1 and Low is 0.
///
S32 mpx_GpioDataGet(U32 gpioNum, U32 *data);



///
///@ingroup	GPIO_CONTROL
///@brief	  Set the GPIO pin configuration
///
///@param	  gpioNum             the GPIO number
///@param   con                 the configuration of the GPIO
///
///@retval	NO_ERR			               No Error
///@retval	ERR_GPIO_INVALID_NUMBER    Invalid GPIO number
///
///@remark Many GPIOs could be configed to other functional pins when executing some functions.
///        This API is used to set the pins to represent the GPIO pins or other funtional pins.
///
S32 mpx_GpioConfiguraionSet(U32 gpioNum, U32 con);



///
///@ingroup	GPIO_CONTROL
///@brief	  Set the GPIO pin to default GPIO function
///
///@param	  gpioNum             the GPIO number
///
///@retval	NO_ERR			               No Error
///@retval	ERR_GPIO_INVALID_NUMBER    Invalid GPIO number
///
///
S32 mpx_GpioConfig2GpioFunc(U32 gpioNum);



///
///@ingroup	GPIO_CONTROL
///@brief	  Get the GPIO pin configuration
///
///@param	  gpioNum             the GPIO number
///@param   con                 the configuration of the GPIO
///
///@retval	NO_ERR			               No Error
///@retval	ERR_GPIO_INVALID_NUMBER    Invalid GPIO number
///
///@remark Many GPIOs could be configed to other functional pins when executing some functions.
///        This API is used to get the pins configuration that representedd the GPIO pins or other funtional pins.
///
S32 mpx_GpioConfiguraionGet(U32 gpioNum, U32 *con);



///
///@ingroup	GPIO_INIT
///@brief	  Initialize the GPIO setting and it's ISR routine.
///
///@retval	NO_ERR			            No Error
///
///@remark	This call initialize all GPIO setting and objects. It should be called first before iMagic startup.
///
S32 mpx_GpioIntInit(void);



///
///@ingroup	GPIO_INT
///@brief	  Config a specific GPIO pin to trigger a GPIO interrupt
///
///@param	  gpioNum             the GPIO number
///@param	  triggerPolarity     the polarity of trigger
///@param	  triggerMode         the trigger mode
///
///@retval	NO_ERR			               No Error
///@retval	ERR_GPIO_INVALID_NUMBER    Invalid GPIO number
///
///@remark	There are two polarity setting of the trigger. One is the POSITIVE_ACTIVE(0), and the other is NEGATIVE_ACTIVE(1).
///@remark  Two trigger mode is supported by iMagic. One is LEVEL_TRIGGER(0), and the other is EDGE_TRIGGER(1).
///
S32 mpx_GpioIntConfig(U32 gpioNum, U32 triggerPolarity, U32 triggerMode);



///
///@ingroup	GPIO_INT
///@brief	  Register a service routine to the specific GPIO pin
///
///@param	  gpioNum             the GPIO number
///@param	  isr                 the service routine of the GPIO pin
///
///
///@retval	NO_ERR			               No Error
///@retval	ERR_GPIO_INVALID_NUMBER    Invalid GPIO number
///
///@remark	After the interrupt enable, the service rouinte would be executed if the signal matchs the interrupt configuration by mpx_GpioIntConfig().
///
S32 mpx_GpioIntRegister(U32 gpioNum, void (*isr)());



///
///@ingroup	GPIO_INT
///@brief	  Enable the specific GPIO interrupt.
///
///@param	  gpioNum             the GPIO number
///
///@retval	NO_ERR			               No Error
///@retval	ERR_GPIO_INVALID_NUMBER    Invalid GPIO number
///
S32 mpx_GpioIntEnable(U32 gpioNum);



///
///@ingroup	GPIO_INT
///@brief	  Disable the specific GPIO interrupt.
///
///@param	  gpioNum             the GPIO number
///
///@retval	NO_ERR			               No Error
///@retval	ERR_GPIO_INVALID_NUMBER    Invalid GPIO number
///
S32 mpx_GpioIntDisable(U32 gpioNum);

#endif //__HAL_GPIO_H

