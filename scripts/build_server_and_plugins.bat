:: this script automatically updates plugins to their latest version
:: this will break builds that are not on the latest commit

@echo off
cls

call build_curl.bat

:: run from the location of this script
cd %~dp0
cd ..

mkdir build
cd build
cmake -A Win32 -DBUILD_SERVER=ON -DBUILD_PLUGINS=ON -DUPDATE_PLUGINS=ON -DINCREASED_SERVER_LIMITS=OFF ..
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    @pause
    exit /b %ERRORLEVEL%
)

echo.
