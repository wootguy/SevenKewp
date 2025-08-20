@echo off

:: don't use mbedtls because it requires shipping certificates or disabling cert checks in curl
::call build_mbedtls.bat %1

set BUILD_DIR=build
set EXTRA_FLAGS=

if /i "%1"=="winxp" (
    :: enable flags for a compatible windows xp build
    set BUILD_DIR=build_static
    set EXTRA_FLAGS=-T v141_xp -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
)

if /i "%1"=="win7" (
    :: enable flags for a compatible windows xp build
    set BUILD_DIR=build_static
    set EXTRA_FLAGS=-T v142 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
)

:: run from the location of this script
cd %~dp0
cd ..\external\curl\ || exit /b 1

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

if exist lib\Release\libcurl.lib (
	echo Curl is already built. Delete "external\curl\%BUILD_DIR%\" if you want to rebuild it.
	echo.
	exit /b 0
)

echo.
echo Building curl library
echo.

cmake .. ^
    -G "Visual Studio 17 2022" ^
    -A Win32 ^
    -DBUILD_CURL_EXE=OFF ^
    -DBUILD_LIBCURL_DOCS=OFF ^
    -DBUILD_MISC_DOCS=OFF ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DBUILD_STATIC_CURL=ON ^
    -DBUILD_STATIC_LIBS=ON ^
    -DCURL_BROTLI=OFF ^
    -DCURL_ZLIB=OFF ^
    -DCURL_ZSTD=OFF ^
    -DENABLE_CURL_MANUAL=OFF ^
    -DENABLE_THREADED_RESOLVER=OFF ^
    -DHTTP_ONLY=ON ^
    -DENABLE_IPV6=OFF ^
    -DCURL_DISABLE_NTLM=ON ^
    -DCURL_USE_LIBPSL=OFF ^
    -DCURL_STATIC_CRT=OFF ^
    -DBUILD_EXAMPLES=OFF ^
    -DCURL_USE_SCHANNEL=ON ^
    -DCURL_USE_MBEDTLS=OFF ^
    -DUSE_NGHTTP2=OFF ^
    -DCURL_USE_LIBSSH2=OFF ^
    -DUSE_LIBIDN2=OFF ^
    %EXTRA_FLAGS%

cmake --build . --config Release
