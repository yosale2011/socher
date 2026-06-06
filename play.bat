@echo off
REM One-click launcher for Socher Hayam under DOSBox.
REM Mounts the game asset folder as both A: and B: (the game reads assets
REM from B:\ when it is run from A:\), then runs the original go.bat.

set "DOSBOX=C:\Program Files (x86)\DOSBox-0.74-3\DOSBox.exe"
set "GAMEDIR=%~dp0socher1"
set "CONF=%TEMP%\socher_dosbox.conf"

if not exist "%DOSBOX%" (
    echo DOSBox not found at "%DOSBOX%".
    echo Edit the DOSBOX path in this file to point to your DOSBox.exe.
    pause
    exit /b 1
)

if not exist "%GAMEDIR%" (
    echo Game asset folder not found at "%GAMEDIR%".
    pause
    exit /b 1
)

REM Generate a temporary DOSBox config with an [autoexec] section.
REM This avoids cmd quoting problems with paths passed via -c.
(
    echo [sdl]
    echo fullscreen=false
    echo output=opengl
    echo [render]
    echo scaler=normal3x forced
    echo [autoexec]
    echo mount a "%GAMEDIR%"
    echo mount b "%GAMEDIR%"
    echo a:
    echo go
) > "%CONF%"

"%DOSBOX%" -conf "%CONF%"
