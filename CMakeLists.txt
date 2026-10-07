cmake_minimum_required(VERSION 3.10)

option(BUILD_CLIENT "Enable this if you want to build the client library. Usually, I build either the client or the server, but not both. Then I can generate 2 separate solutions to debug both the client and server at the same time." OFF)
option(BUILD_SERVER "Enable this to build the server library. Usually, I build either the client or the server, but not both. Then I can generate 2 separate solutions to debug both the client and server at the same time." OFF)
option(INSTALL_BINARIES "Install binaries to your Half-Life client/server folder for debugging. Otherwise copy them to the output folder in this project.")
option(STATIC_MSVC_RUNTIME "Build using the static MSVC runtime. Public releases use this so that Visual Studio DLLs don't need to be included wth the client. If you're building for yourself, then leave this disabled.")
option(BUILD_CURL "Use the CURL library for server plugins and client updates. Leave this disabled to simplify the build process. The server and client can function without Curl. You will see an error if something needs it." ON)

set(SERVER_DLL_NAME "server")
set(CLIENT_DLL_NAME "client")

if (BUILD_SERVER)
	set(SERVER_WORK_DIR "C:/Games/hlds/steamapps/common/Half-Life/" CACHE STRING "Path to your dedicated server root folder. hlds.exe is found here.")
	set(SERVER_ARGS "+map c1a1 -dll dlls/server.dll -num_edicts 7727 -heapsize 131072 +maxplayers 32 +developer 1 +sv_cheats 1 -port 27016 +log on +servercfgfile \"\"" CACHE STRING "Dedicated server launch arguments for debugging. Port 27016 is used by default because the Half-Life Steam client will steal 27015 and prevent you starting the server.")
	option(BUILD_PLUGINS "Build everything in the plugins/ folder")
	option(UPDATE_PLUGINS "Discard local changes and update everything in the plugins/ folder from git. Useful for automated build scripts on public servers. Don't enable this for local development.")
	option(INCREASED_SERVER_LIMITS "Build a server library with an increased physent limit. This prevents clients getting stuck in areas with 600+ entities. ReHLDS must also be built with increased limits or else the server will crash instantly.")
	
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		PROPERTY VS_STARTUP_PROJECT ${SERVER_DLL_NAME}
	)
endif()

if (BUILD_CLIENT)
	if(WIN32)
		get_filename_component(STEAM_PATH
			"[HKEY_CURRENT_USER\\Software\\Valve\\Steam;SteamPath]"
			ABSOLUTE
		)
	elseif(UNIX)
		set(STEAM_PATH "$ENV{HOME}/.steam/steam")
	endif()

	set(CLIENT_WORK_DIR "${STEAM_PATH}/steamapps/common/Half-Life/" CACHE STRING "Path to your Steam Half-Life folder. hl.exe is found here. Used for setting up the debugger.")
	set(CLIENT_ARGS "-novid -console +developer 1 +connect 192.168.1.123:27016" CACHE STRING "Launch arguments for debugging.")
	set(CLIENT_WORK_DIR_XASH "C:/Games/Xash3D/" CACHE STRING "Path to your Xash3D root folder. xash3d.exe is found here. Used for setting up the debugger.")
	
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		PROPERTY VS_STARTUP_PROJECT ${CLIENT_DLL_NAME}
	)
endif()

mark_as_advanced(CMAKE_INSTALL_PREFIX)
mark_as_advanced(CMAKE_CONFIGURATION_TYPES)
mark_as_advanced(INCREASED_SERVER_LIMITS)
mark_as_advanced(STATIC_MSVC_RUNTIME)
mark_as_advanced(UPDATE_PLUGINS)

include_directories(${CMAKE_SOURCE_DIR}/common)
include_directories(${CMAKE_SOURCE_DIR}/engine)
include_directories(${CMAKE_SOURCE_DIR}/game_shared)
include_directories(${CMAKE_SOURCE_DIR}/pm_shared)
include_directories(${CMAKE_SOURCE_DIR}/public)
include_directories(${CMAKE_SOURCE_DIR}/external/rapidjson/include/)

if (BUILD_SERVER OR BUILD_CLIENT)
	if (BUILD_SERVER AND BUILD_CLIENT)
		project(sevenkewp)
	elseif (BUILD_SERVER)
		project(sevenkewp_server)
	elseif (BUILD_CLIENT)
		project(sevenkewp_client)
	endif()
endif()

if (MSVC)
	set(CMAKE_CONFIGURATION_TYPES "Debug;DebugXash;Release;RelWithDebInfo;Asan" CACHE STRING "" FORCE)
	set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_DEBUG} /fsanitize=address")
	set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} /fsanitize=address")
	set(CMAKE_SHARED_LINKER_FLAGS_ASAN "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG")
	
	set(CMAKE_C_FLAGS_DEBUGXASH "${CMAKE_C_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_DEBUGXASH "${CMAKE_CXX_FLAGS_DEBUG}")
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUGXASH "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
	
	if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
		message(WARNING "Half-Life is a 32-bit program. If you are not a developer adding 64-bit compatibility for sevenkewp, then delete the CMake cache and try again with the Win32 platform. Your server/client will crash otherwise.")
	endif()
endif()

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