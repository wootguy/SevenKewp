@echo off
cls

:: run from the location of this script
cd %~dp0

:: make sure the game lib is up-to-date
call build_game.bat

:: plugin should have included the game repo as a submodule. So, there should be a cmake project above this one.
cd ..\..
mkdir build
cd build
cmake -A Win32 ..
cmake --build . --config Release
