cmake_minimum_required(VERSION 3.6)

set(ENGINE_DIR "D:/Steam/steamapps/common/Half-Life" CACHE STRING "Path to your half-life-compatible engine folder")
set(ENGINE_EXE "hl.exe" CACHE STRING "Name of the executable that starts the half-life engine")

set(SERVER_DLL_NAME "server")
set(CLIENT_DLL_NAME "client")

project(opencoop)

add_subdirectory(dlls) # server project
add_subdirectory(cl_dll) # client project
