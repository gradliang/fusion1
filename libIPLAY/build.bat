@echo off
cd make

:MainMenu
echo ---------------------------------
echo     1.Make all
echo     2.Make clean all
echo     3.Make av
echo     4.Make xpg
echo     5.Make file IO lib
echo     0.Exit
echo ---------------------------------
set MenuIndex=
set /p MenuIndex=    Choice function : 

:RepeatAction
IF %MenuIndex%==0 EXIT
IF %MenuIndex%==1 GOTO MakeAll
IF %MenuIndex%==2 GOTO MakeClean
IF %MenuIndex%==3 GOTO MakeAV
IF %MenuIndex%==4 GOTO MakeXpg
IF %MenuIndex%==5 GOTO MakeFileIO
GOTO menu

:MakeAll
	SET menuStr=MakeAll
	make	
	GOTO done
	
:MakeClean
	SET menuStr="MakeClean"
	make clean	
	GOTO done

:MakeAV
	SET menuStr="MakeAV"
	make av
	GOTO done		
	
:MakeFileIO
	SET menuStr="MakeFileIO"
	make fileIO
	GOTO done	
	
:MakeXpg
	SET menuStr="MakeXpg"
	make xpg
	GOTO done	
	
:MakeOS
	SET menuStr="MakeOS"
	make os
	GOTO done				
	
:MakeDemux
	SET menuStr=Make Demux
	make demux
	GOTO done	

:done	
set boRepeat=
set /p boRepeat=  Repeat %menuStr% (y/n): 

IF %boRepeat%==n GOTO MainMenu
GOTO RepeatAction