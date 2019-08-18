@echo off

:MainMenu
echo .
echo ---------------------------------
echo     1.Remove code for internal release
echo     2.Remove code for BSP release
echo     0.Exit
echo ---------------------------------
set MenuIndex=
set /p MenuIndex=    Choice function : 

:RepeatAction
IF %MenuIndex%==0 EXIT
IF %MenuIndex%==1 GOTO CleanForINT
IF %MenuIndex%==2 GOTO CleanForBSP
GOTO menu

:CleanForINT
	SET menuStr=:"CleanForINT"
	xcopy ..\libIPLAY ..\libIPLAY.BAK /s /e /i
	del ..\libIPLAY\libSrc\ar2524\*.c /s
	del ..\libIPLAY\libSrc\ar2524\*.s /s
 	del ..\libIPLAY\libSrc\ar2524\*.o /s
 	del ..\libIPLAY\libSrc\ar2524\*.lst /s
 	del ..\libIPLAY\libSrc\ar2524\*.bak /s
	del ..\libIPLAY\libSrc\lwip\*.c /s
	del ..\libIPLAY\libSrc\lwip\*.s /s
 	del ..\libIPLAY\libSrc\lwip\*.o /s
 	del ..\libIPLAY\libSrc\lwip\*.lst /s 
 	del ..\libIPLAY\libSrc\lwip\*.bak /s 	
	del ..\libIPLAY\libSrc\mac80211\*.c /s
	del ..\libIPLAY\libSrc\mac80211\*.s /s
 	del ..\libIPLAY\libSrc\mac80211\*.o /s
 	del ..\libIPLAY\libSrc\mac80211\*.lst /s
 	del ..\libIPLAY\libSrc\mac80211\*.bak /s 	
	del ..\libIPLAY\libSrc\os_linux\*.c /s
	del ..\libIPLAY\libSrc\os_linux\*.s /s
 	del ..\libIPLAY\libSrc\os_linux\*.o /s
 	del ..\libIPLAY\libSrc\os_linux\*.lst /s
 	del ..\libIPLAY\libSrc\os_linux\*.bak /s 
	del ..\libIPLAY\libSrc\rt73\*.c /s
	del ..\libIPLAY\libSrc\rt73\*.s /s
 	del ..\libIPLAY\libSrc\rt73\*.o /s
 	del ..\libIPLAY\libSrc\rt73\*.lst /s
 	del ..\libIPLAY\libSrc\rt73\*.bak /s 
	del ..\libIPLAY\libSrc\epub\*.c /s
	del ..\libIPLAY\libSrc\epub\*.s /s
 	del ..\libIPLAY\libSrc\epub\*.o /s
 	del ..\libIPLAY\libSrc\epub\*.lst /s
 	del ..\libIPLAY\libSrc\epub\*.bak /s 
	del ..\libIPLAY\libSrc\fontengine\*.c /s
	del ..\libIPLAY\libSrc\fontengine\*.s /s
 	del ..\libIPLAY\libSrc\fontengine\*.o /s
 	del ..\libIPLAY\libSrc\fontengine\*.lst /s
 	del ..\libIPLAY\libSrc\fontengine\*.bak /s 
	del ..\libIPLAY\libSrc\pdf\*.c /s
	del ..\libIPLAY\libSrc\pdf\*.s /s
 	del ..\libIPLAY\libSrc\pdf\*.o /s
 	del ..\libIPLAY\libSrc\pdf\*.lst /s
 	del ..\libIPLAY\libSrc\pdf\*.bak /s  	
	copy ..\libIPLAY.BAK\lib\indarch.o ..\libIPLAY\lib\
	echo Customer Release Done.
	echo .
	echo .
	echo Please modify corelib.h for internal release.
	echo .
	echo .
	notepad include\corelib.h
	cd main\make\
	config.bat
	make clean	
	GOTO MainMenu

:CleanForBSP
	SET menuStr=:"CleanForBSP"
	xcopy ..\libIPLAY ..\libIPLAY.BAK /s /e /i
	del ..\libIPLAY\*.c /s
	del ..\libIPLAY\*.s /s
 	del ..\libIPLAY\*.o /s
 	del ..\libIPLAY\*.lst /s
 	del ..\libIPLAY\*.bak /s
	copy ..\libIPLAY.BAK\lib\indarch.o ..\libIPLAY\lib\
	echo BSP Release Done.
	echo .
	echo .
	echo Please modify corelib.h for BSP release.
	echo .
	echo .
	notepad include\corelib.h
	cd main\make\
	config.bat
	make clean
	GOTO MainMenu
	
GOTO MainMenu

:done	
set boRepeat=
set /p boRepeat=  Repeat %menuStr% (y/n): 

IF %boRepeat%==n GOTO MainMenu
GOTO RepeatAction

GOTO MainMenu
