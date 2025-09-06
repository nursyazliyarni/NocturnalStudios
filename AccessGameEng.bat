@echo off
:menu
cls
echo =============================
echo   Build Helper Script
echo =============================
echo [1] Configure (cmake -S . -B build)
echo [2] Build Debug (cmake --build build --config Debug)
echo [3] Build Release (cmake --build build --config Release)
echo [4] Exit
echo.
set /p choice=Select option:

if "%choice%"=="1" goto configure
if "%choice%"=="2" goto build_debug
if "%choice%"=="3" goto build_release
if "%choice%"=="4" exit
goto menu

:configure
cmake -S . -B build
pause
goto menu

:build_debug
cmake --build build --config Debug
pause
goto menu

:build_release
cmake --build build --config Release
pause
goto menu
