@echo off
setlocal enabledelayedexpansion

set "CC_PATH="
set "CXX_PATH="
rem set "CC_PATH=C:/MinGW/bin/gcc.exe"
rem set "CXX_PATH=C:/MinGW/bin/g++.exe"

cd /d "%~dp0.."

set "EXTRA_ARGS="
if not "%CC_PATH%"=="" set "EXTRA_ARGS=%EXTRA_ARGS% -DCMAKE_C_COMPILER=%CC_PATH%"
if not "%CXX_PATH%"=="" set "EXTRA_ARGS=%EXTRA_ARGS% -DCMAKE_CXX_COMPILER=%CXX_PATH%"

echo === Configuring ===
cmake -B build -G "MinGW Makefiles" %EXTRA_ARGS%
if errorlevel 1 goto :error

echo === Building ===
cmake --build build
if errorlevel 1 goto :error

echo.
echo === Testing ===
echo.
ctest --test-dir build --output-on-failure

echo.
pause
exit /b 0

:error
echo.
echo Build 失敗，看上面的錯誤訊息。
pause
exit /b 1