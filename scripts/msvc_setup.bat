@echo off
cls

:: run from the location of this script
cd %~dp0
cd ..

mkdir msvc
cd msvc

mkdir server
cd server
cmake -A Win32 -DBUILD_CLIENT=ON -DBUILD_SERVER=ON -DBUILD_PLUGINS=ON ..

echo.