#define LOCAL_DEBUG_ENABLE  			0

#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "xpgFunc.h"
#include "ui.h"
#include "Peripheral.h"

static BYTE st_bBackLightOn=0;
SWORD PlatformInit()
{
    MP_DEBUG("%s", __func__);

#if AUTO_DEMO
    if (BootUpAutoDemoFlag && g_wAutoDemoFlag)
        BootUpAutoDemoFlag = 2;

    if (g_wAutoDemoFlag)
    {
        xpgAccuAutoDemoFlag ();
        AddTimerProc(g_wAutoDemoStartSecond << 2, xpgAutoDemoISR);
    }
#endif

#if (defined(WATCHDOG_GPIO)&&(WATCHDOG_GPIO!=GPIO_NULL))
	 SendWatchDog();
#endif

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    #if (BRIGHTNESS_CTRL == LCD_BRIGHTNESS_HW1)
    TimerPwmEnable(3, 240000, 0);
    #endif
#endif

#if defined(GPIO_GPIO_PWM_OUT)
	InitPwmGpio();
#endif

    return PASS;
}

void TurnOnBackLight(void)
{
    MP_DEBUG("%s", __func__);

    SetGPIOValue(GPIO_BL_CTRL, 1); // Platform_MP663_DV.h, #define GPIO_BL_CTRL GPIO_NULL
    st_bBackLightOn=1;
}


void TurnOffBackLight(void)
{
    MP_DEBUG("%s", __func__);

    SetGPIOValue(GPIO_BL_CTRL, 0); // Platform_MP663_DV.h, #define GPIO_BL_CTRL GPIO_NULL
    st_bBackLightOn=0;
}

BYTE CheckAndTurnOnBackLight(void)
{
	if (!st_bBackLightOn)
	{
		TurnOnBackLight();
		return 1;
	}
	return 0;
}

void TimerToBacklightOff(DWORD dwTime)
{
	if (dwTime)
		Ui_TimerProcAdd(dwTime, TurnOffBackLight);
	else
		Ui_TimerProcRemove(TurnOffBackLight);
}

