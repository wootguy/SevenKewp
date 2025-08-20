@echo off

set BUILD_DIR=build
set EXTRA_FLAGS=

if /i "%1"=="winxp" (
    :: enable flags for a compatible windows xp build
    set BUILD_DIR=build_static
    set EXTRA_FLAGS=-T v141_xp -DMSVC_STATIC_RUNTIME=ON
)

:: run from the location of this script
cd %~dp0
cd ..\external\mbedtls\ || exit /b 1

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

if exist library\Release\mbedtls.lib (
	echo mbedtls is already built. Delete "external\mbedtls\%BUILD_DIR%\" if you want to rebuild it.
	echo.
	exit /b 0
)

echo.
echo Building mbedtls library
echo.

cmake .. ^
    -G "Visual Studio 17 2022" ^
    -A Win32 ^
    -DMSVC_STATIC_RUNTIME=ON ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DENABLE_PROGRAMS=OFF ^
    -DENABLE_TESTING=OFF ^
     %EXTRA_FLAGS%

cmake --build . --config Release

pause

