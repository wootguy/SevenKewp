#!/bin/bash
set -e

# run from the location of this script
(
	cd "$(dirname "$0")"
	sh build_curl.sh
)

cd "$(dirname "$0")"
cd ..

rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SERVER=ON -DINCREASED_SERVER_LIMITS=OFF ..
cmake --build .
