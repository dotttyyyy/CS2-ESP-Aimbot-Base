@echo off
echo.
echo ========================================
echo   BUILDING CS2 OFFLINE DEV TOOL
echo ========================================
echo.

cl /EHsc /std:c++17 src\main.cpp /link user32.lib kernel32.lib gdi32.lib gdiplus.lib

if %errorlevel% == 0 (
    echo.
    echo SUCCESS! CS2OfflineTool.exe is ready.
    echo Run as Administrator in offline CS2.
    echo.
) else (
    echo.
    echo FAILED. Install Visual Studio 2022.
    echo.
)
pause
