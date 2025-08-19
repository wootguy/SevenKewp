#!/bin/bash
set -e  # exit on error

cd "$(dirname "$0")"
cd ../external/curl || { echo "Curl directory not found"; exit 1; }

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Check if already built
if [ -f ../build/libcurl.a ]; then
    echo "Curl is already built. Delete 'external/curl/build/' if you want to rebuild it."
    echo
fi

echo
echo "Building curl library"
echo

# Configure 32-bit static build
cmake .. \
    -DCMAKE_C_FLAGS="-m32" \
    -DCMAKE_CXX_FLAGS="-m32" \
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
    -DUSE_LIBIDN2=OFF \
    -DUSE_NGHTTP2=OFF \
    -DCURL_USE_LIBPSL=OFF \
    -DBUILD_EXAMPLES=OFF

# Build the library
cmake --build . --config Release
