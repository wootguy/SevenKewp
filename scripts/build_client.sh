#!/bin/bash
set -e

# run from the location of this script
cd "$(dirname "$0")"
cd ..

rm -rf build_client
mkdir build_client
cd build_client
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_CLIENT=ON -DBUILD_SERVER=OFF -DBUILD_PLUGINS=OFF -DUPDATE_PLUGINS=OFF -DINCREASED_SERVER_LIMITS=OFF ..
cmake --build .
