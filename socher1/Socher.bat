@echo off
If exist \kika\socher_b\K.com goto Game

echo „‰‰—‰š„ Š…š š…‰„Œ ‰‰‡ —‡™„
echo               \kika\socher_b\
echo 1 ™—„ …† „‰‰—‰šŒ —‡™„ š—š’„Œ
echo                  2 ™—„ Œ…ˆ‰Œ
choice /c:12
if errorlevel 2 goto End
if errorlevel 1 goto Install
goto End


:Install
md \kika > NUL
md \kika\socher_b > NUL
xcopy *.* \kika\socher_b /h /y > NUL
cd \kika\socher_b > NUL
echo ! „Œ™…„ „—š„
pause
Socher.bat
goto End


:Game
cls

Kika.bat
:End
