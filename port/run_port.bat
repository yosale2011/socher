@echo off
rem Socher Hayam Win32 port launcher - runs the game from the asset
rem directory (socher1) so all .scr/.win/.sgn/.lin files are found.
cd /d %~dp0..\socher1
..\port\bin\socher.exe
