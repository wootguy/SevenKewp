#!/bin/bash
set -e  # exit on error

# run from the location of this script
(
    cd "$(dirname "$0")"
    sh build_mbedtls.sh
)

cd "$(dirname "$0")"
cd ../external/curl || { echo "Curl directory not found"; exit 1; }

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Check if already built
if [ -f libcurl.a ]; then
    echo "Curl is already built. Delete 'external/curl/build/' if you want to rebuild it."
    echo
    exit 0
fi

echo
echo "Building curl library"
echo

# Configure 32-bit static build
cmake .. \
    -DCMAKE_C_FLAGS="-m32 -fPIC -ffunction-sections -fdata-sections" \
    -DBUILD_CURL_EXE=OFF \
    -DBUILD_LIBCURL_DOCS=OFF \
    -DBUILD_MISC_DOCS=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_STATIC_LIBS=ON \
    -DCURL_BROTLI=OFF \
    -DCURL_ZLIB=OFF \
    -DCURL_ZSTD=OFF \
    -DENABLE_CURL_MANUAL=OFF \
    -DENABLE_THREADED_RESOLVER=OFF \
    -DHTTP_ONLY=ON \
    -DCURL_DISABLE_PROXY=ON \
    -DCURL_DISABLE_WEBSOCKETS=ON \
    -DCURL_DISABLE_KERBEROS_AUTH=ON \
    -DCURL_DISABLE_ALTSVC=ON \
    -DCURL_DISABLE_HSTS=ON \
    -DENABLE_IPV6=OFF \
    -DCURL_DISABLE_NTLM=ON \
    -DCURL_DISABLE_NEGOTIATE_AUTH=ON \
    -DENABLE_UNIX_SOCKETS=OFF \
    -DUSE_LIBIDN2=OFF \
    -DUSE_NGHTTP2=OFF \
    -DCURL_USE_OPENSSL=OFF \
    -DCURL_USE_WOLFSSL=OFF \
    -DCURL_USE_MBEDTLS=ON \
    -DMBEDTLS_INCLUDE_DIR='../../mbedtls/include' \
    -DMBEDTLS_LIBRARY='../../mbedtls/build/library/mbedtls.a' \
    -DMBEDX509_LIBRARY='../../mbedtls/build/library/mbedx509.a' \
    -DMBEDCRYPTO_LIBRARY='../../mbedtls/build/library/mbedcrypto.a' \
    -DCURL_USE_LIBPSL=OFF \
    -DBUILD_EXAMPLES=OFF

# Build the library
cmake --build . --config Release
