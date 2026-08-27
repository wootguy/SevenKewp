#!/bin/bash
set -e

# this script automatically updates plugins to their latest version
# this will break builds that are not on the latest commit

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
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SERVER=ON -DBUILD_PLUGINS=ON -DUPDATE_PLUGINS=ON -DINCREASED_SERVER_LIMITS=OFF ..
cmake --build .
