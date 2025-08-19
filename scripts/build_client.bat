@echo off
cls

call build_curl.bat

:: run from the location of this script
cd %~dp0
cd ..

mkdir build_client
cd build_client
cmake -A Win32 -DBUILD_CLIENT=ON -DBUILD_SERVER=OFF -DBUILD_PLUGINS=OFF -DUPDATE_PLUGINS=OFF -DINCREASED_SERVER_LIMITS=OFF ..
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    @pause
    exit /b %ERRORLEVEL%
)

echo.

pause