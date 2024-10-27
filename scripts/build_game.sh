#!/bin/bash
set -e

# run from the location of this script
cd "$(dirname "$0")"
cd ..

rm -rf build
mkdir build
cd build
cmake -DBUILD_SERVER=ON ..
cmake --build . --config Release
