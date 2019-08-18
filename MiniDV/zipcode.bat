echo off
echo This will rar *.c, *.h, *.a *.mak *.bin *.elf and makefile newer than date
echo example : 20050805

Path=C:\Program Files\WinRAR\;%PATH%
set date=
set /p date=Type the date of oldest updated file. (yyyymmddhhmmss) : 
Rar  a mp612_iPlay_%date%.rar -r -ed -ta%date% *.c *.h *.mak *.elf *.bin *.a makefile
pause
echo on