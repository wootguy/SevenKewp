#!/bin/bash
set -e

# run from the location of this script
cd "$(dirname "$0")"

# make sure the game lib is up-to-date
sh build_game.sh

# plugin should have included the game repo as a submodule. So, there should be a cmake project above this one.
cd ../..
rm -rf build
mkdir build
cd build
cmake ..
cmake --build . --config Release
