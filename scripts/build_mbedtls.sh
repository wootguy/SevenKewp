#!/bin/bash
set -e  # exit on error

cd "$(dirname "$0")"
cd ../external/mbedtls || { echo "mbedtls directory not found"; exit 1; }

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Check if already built
if [ -f library/mbedtls.a ]; then
    echo "mbedtls is already built. Delete 'external/mbedtls/build/' if you want to rebuild it."
    echo
    exit 0
fi

echo
echo "Building mbedtls library"
echo

# Configure 32-bit static build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="-m32 -fPIC" \
  -DBUILD_SHARED_LIBS=OFF \
  -DENABLE_PROGRAMS=OFF \
  -DENABLE_TESTING=OFF

# Build the library
cmake --build . --config Release
