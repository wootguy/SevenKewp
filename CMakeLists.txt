cmake_minimum_required(VERSION 3.20)

option(BUILD_SERVER "Disable this if you don't want to build the server library" ON)
option(BUILD_CLIENT "Enable this if you want to build the client library" OFF)
option(BUILD_PLUGINS "Build everything in the plugins/ folder")
option(UPDATE_PLUGINS "Discard local changes and update everything in the plugins/ folder")
option(INCREASED_SERVER_LIMITS "Build a server library with increased limits (server crashes instantly if this does not match rehlds)")
option(SETUP_IDE "Install binaries to the game folder")

set(SERVER_WORK_DIR "C:/Games/hlds/steamapps/common/Half-Life" CACHE STRING "Path to your server root folder")
set(SERVER_ARGS "+map c1a1 -dll dlls/server.dll -num_edicts 2047 -heapsize 65536 +maxplayers 32 +developer 1 +sv_cheats 1 +log on -port 27016" CACHE STRING "server launch arguments")
set(SERVER_EXE "hlds.exe" CACHE STRING "Name of the executable that starts the server")

set(CLIENT_WORK_DIR "C:/Games/Steam/steamapps/common/Half-Life" CACHE STRING "Path to your client root folder")
set(CLIENT_ARGS "-novid" CACHE STRING "client launch arguments")
set(CLIENT_EXE "hl.exe" CACHE STRING "Name of the executable that starts the client")

set(SERVER_DLL_NAME "server")
set(CLIENT_DLL_NAME "client")

include_directories(${CMAKE_SOURCE_DIR}/common)
include_directories(${CMAKE_SOURCE_DIR}/engine)
include_directories(${CMAKE_SOURCE_DIR}/game_shared)
include_directories(${CMAKE_SOURCE_DIR}/pm_shared)
include_directories(${CMAKE_SOURCE_DIR}/public)
include_directories(${CMAKE_SOURCE_DIR}/dlls/env)
include_directories(${CMAKE_SOURCE_DIR}/dlls/func)
include_directories(${CMAKE_SOURCE_DIR}/dlls/triggers)
include_directories(${CMAKE_SOURCE_DIR}/dlls/monster)
include_directories(${CMAKE_SOURCE_DIR}/dlls/item)
include_directories(${CMAKE_SOURCE_DIR}/dlls/path)
include_directories(${CMAKE_SOURCE_DIR}/dlls/weapon)
include_directories(${CMAKE_SOURCE_DIR}/dlls/net)
include_directories(${CMAKE_SOURCE_DIR}/dlls/hooks)
include_directories(${CMAKE_SOURCE_DIR}/dlls/util)
include_directories(${CMAKE_SOURCE_DIR}/dlls/game)
include_directories(${CMAKE_SOURCE_DIR}/dlls/player)
include_directories(${CMAKE_SOURCE_DIR}/dlls/)

include_directories(${CMAKE_SOURCE_DIR}/rehlds)
include_directories(${CMAKE_SOURCE_DIR}/rehlds/rehlds/public)
include_directories(${CMAKE_SOURCE_DIR}/rehlds/rehlds/common)

project(sevenkewp)

if (BUILD_SERVER)
	add_subdirectory(dlls) # server project
endif()

if (BUILD_CLIENT)
	add_subdirectory(cl_dll) # client project
endif()

if (BUILD_PLUGINS)
	include(${CMAKE_SOURCE_DIR}/scripts/hlcoop_plugin.cmake)
	set(PLUGIN_DIR "${CMAKE_CURRENT_LIST_DIR}/plugins")
	file(GLOB ALL_ITEMS "${PLUGIN_DIR}/*")
	
	if (UPDATE_PLUGINS)
		message(STATUS "Note: Local plugin changes are discarded when built in UPDATE_PLUGINS mode")
	endif()
	
	foreach(ITEM ${ALL_ITEMS})
		if(IS_DIRECTORY ${ITEM})
			get_filename_component(REPO_NAME ${ITEM} NAME)
			add_subdirectory(${ITEM})
			
			if (UPDATE_PLUGINS)
				# Discard local changes and reset plugin to the latest commit, so that the builds work after force pushes
				execute_process(
					COMMAND git fetch
					WORKING_DIRECTORY ${ITEM}
					RESULT_VARIABLE GIT_PULL_RESULT
					ERROR_VARIABLE GIT_PULL_ERROR
					OUTPUT_VARIABLE GIT_PULL_OUTPUT
				)
				if(NOT GIT_PULL_RESULT EQUAL 0)
					message(FATAL_ERROR "Failed to fetch the latest changes in ${REPO_NAME}:\n${GIT_PULL_ERROR}")
				endif()
				
				execute_process(
					COMMAND git reset origin/master --hard
					WORKING_DIRECTORY ${ITEM}
					RESULT_VARIABLE GIT_PULL_RESULT
					ERROR_VARIABLE GIT_PULL_ERROR
					OUTPUT_VARIABLE GIT_PULL_OUTPUT
				)
				if(NOT GIT_PULL_RESULT EQUAL 0)
					message(FATAL_ERROR "Failed to reset changes in ${REPO_NAME}:\n${GIT_PULL_ERROR}")
				else()
					string(REPLACE "\n" "" GIT_PULL_OUTPUT "${GIT_PULL_OUTPUT}")
					message(STATUS "[${REPO_NAME}] ${GIT_PULL_OUTPUT}")
				endif()
			endif()
		endif()
	endforeach()
endif()