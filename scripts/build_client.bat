@echo off
cls

call build_curl.bat %1

set BUILD_DIR=build_client
set EXTRA_FLAGS=

if /i "%1"=="winxp" (
    :: enable flags for a compatible windows xp build
    set BUILD_DIR=build_client_winxp
    set EXTRA_FLAGS=-T v141_xp -DSTATIC_MSVC_RUNTIME=ON
)

if /i "%1"=="win7" (
    :: enable flags for a compatible windows xp build
    set BUILD_DIR=build_client_win7
    set EXTRA_FLAGS=-T v142 -DSTATIC_MSVC_RUNTIME=ON
)

:: run from the location of this script
cd %~dp0
cd ..

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%
cmake -A Win32 -DBUILD_CLIENT=ON -DENABLE_CURL=ON %EXTRA_FLAGS% ..
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    @pause
    exit /b %ERRORLEVEL%
)

echo.
