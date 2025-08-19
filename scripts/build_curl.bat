@echo off

:: run from the location of this script
cd %~dp0
cd ..\external\curl\ || exit /b 1

if not exist build mkdir build
cd build

if exist external\Release\libcurl.lib (
	echo Curl is already built. Delete "external\curl\build\" if you want to rebuild it.
	echo.
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
    -DBUILD_STATIC_LIBS=ON ^
    -DCURL_BROTLI=OFF ^
    -DCURL_ZLIB=OFF ^
    -DCURL_ZSTD=OFF ^
    -DENABLE_CURL_MANUAL=OFF ^
    -DENABLE_THREADED_RESOLVER=OFF ^
    -DUSE_LIBIDN2=OFF ^
    -DUSE_NGHTTP2=OFF ^
    -DCURL_USE_LIBPSL=OFF ^
    -DBUILD_EXAMPLES=OFF ^
    -DCURL_USE_SCHANNEL=ON

cmake --build . --config Release
