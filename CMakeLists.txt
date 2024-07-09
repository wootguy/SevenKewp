cmake_minimum_required(VERSION 3.6)

set(ENGINE_DIR "D:/Steam/steamapps/common/Half-Life" CACHE STRING "Path to your half-life-compatible engine folder")
set(ENGINE_EXE "hl.exe" CACHE STRING "Name of the executable that starts the half-life engine")
option(USE_AUTOHOTKEY_SCRIPT "Disable this if you don't have AutoHotkey installed" )
option(BUILD_SERVER_ONLY "Disable this if you don't want to build the client library" )

set(SERVER_DLL_NAME "sevenkewp")
set(CLIENT_DLL_NAME "client")
set(GAMEDIR_NAME "sevenkewp")
set(GAME_ARGS "-steam -console -game ${GAMEDIR_NAME} +developer 2 +map sc_test -sv_cheats 1 -maxplayers 32")

project(sevenkewp)

add_subdirectory(dlls) # server project

if (NOT ${BUILD_SERVER_ONLY})
	add_subdirectory(cl_dll) # client project
endif()