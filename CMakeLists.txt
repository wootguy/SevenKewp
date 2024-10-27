cmake_minimum_required(VERSION 3.6)

set(ENGINE_DIR "D:/Steam/steamapps/common/Half-Life" CACHE STRING "Path to your half-life-compatible engine folder")
set(ENGINE_EXE "hl.exe" CACHE STRING "Name of the executable that starts the half-life engine")
option(BUILD_SERVER "Disable this if you don't want to build the server library" ON)
option(BUILD_CLIENT "Enable this if you want to build the client library" OFF)
option(BUILD_PLUGINS "Build everything in the plugins/ folder" )

set(SERVER_DLL_NAME "sevenkewp")
set(CLIENT_DLL_NAME "client")
set(GAMEDIR_NAME "sevenkewp")
set(GAME_ARGS "-steam -console -game ${GAMEDIR_NAME} +developer 2 +map sc_test -sv_cheats 1 -maxplayers 32")

if (BUILD_PLUGINS)
	# all plugins build from the sevenkewp project
	set(MOD_DIR ${CMAKE_SOURCE_DIR})
else()
	# individual plugin build with sevenkewp as a submodule
	set(MOD_DIR ${CMAKE_SOURCE_DIR}/SevenKewp)
endif()

project(sevenkewp)

if (BUILD_SERVER)
	add_subdirectory(dlls) # server project
endif()

if (BUILD_CLIENT)
	add_subdirectory(cl_dll) # client project
endif()

if (BUILD_PLUGINS)
	set(PLUGIN_DIR "${CMAKE_CURRENT_LIST_DIR}/plugins")
	file(GLOB ALL_ITEMS "${PLUGIN_DIR}/*")
	message(STATUS "Note: Local plugin changes are discarded when built in BUILD_PLUGINS mode")

	foreach(ITEM ${ALL_ITEMS})
		if(IS_DIRECTORY ${ITEM})
			get_filename_component(REPO_NAME ${ITEM} NAME)
			add_subdirectory(${ITEM})
			
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
				message(STATUS "[${REPO_NAME}] ${GIT_PULL_OUTPUT}")
			endif()
		endif()
	endforeach()
endif()