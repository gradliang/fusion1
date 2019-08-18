
#ifndef UI_GPIO_H
#define UI_GPIO_H

#include "utiltypedef.h"
#include "ui.h"
/*
// Constant declarations
*/


/*
// Structure declarations
*/



/*
// Function prototype
*/
void Ui_SetKey(BYTE keyCode);
BYTE Ui_GetKey(void);
void Ui_UsbdDetectEvent(WHICH_OTG eWhichOtg);
void Ui_UsbdSetMode(WHICH_OTG eWhichOtg);

#endif  //UI_GPIO_H

