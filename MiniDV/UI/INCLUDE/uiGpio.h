
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
typedef struct
{
    BYTE keyCode;
    BYTE keyStatus;
    BYTE reserved[2];
} ST_KEY_INFO;


/*
// Function prototype
*/
void Ui_UsbdDetectEvent(WHICH_OTG eWhichOtg);
void Ui_UsbdSetMode(WHICH_OTG eWhichOtg);

void TurnOnBackLight(void);
void TurnOffBackLight(void);

#endif  //UI_GPIO_H

