# setup standard include directories and compile settings
function(hlcoop_setup_plugin OUTPUT_PATH)
	if(UNIX)
		set(DEBUG_WARN_FLAGS "-Wall -Wextra -Wpedantic -Werror=return-type -Wno-invalid-offsetof -Wno-class-memaccess -Wno-unused-parameter")
		
		set(OPT_FLAG "-O2")
		if (ASAN)
			set(ASAN_CFLAGS "-fsanitize=address -fno-omit-frame-pointer")
			set(ASAN_LFLAGS "-fsanitize=address")
			set(OPT_FLAG "-Og")
		endif()
		
		# Static linking libstd++ and libgcc so that the plugin can load on distros other than one it was compiled on.
		# -fvisibility=hidden fixes a weird bug where the metamod confuses game functions with plugin functions.
		# -g includes debug symbols which provides useful crash logs, but also inflates the .so file size a lot.
		# warnings are disabled in release mode (users don't care about that)
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -std=c++11 -fvisibility=hidden -static-libstdc++ -static-libgcc -g" PARENT_SCOPE)
		#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -static-libgcc -g" PARENT_SCOPE)
		
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -std=c++11 -fvisibility=hidden -fno-omit-frame-pointer -g ${ASAN_CFLAGS}" PARENT_SCOPE)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 ${DEBUG_WARN_FLAGS}" PARENT_SCOPE)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPT_FLAG} -w" PARENT_SCOPE)
		
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -g -fno-omit-frame-pointer ${ASAN_CFLAGS}" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 ${DEBUG_WARN_FLAGS}" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${OPT_FLAG} -w" PARENT_SCOPE)
		
		set(CMAKE_SHARED_LIBRARY_PREFIX "" PARENT_SCOPE)
		
	elseif(MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /we4715" PARENT_SCOPE) 
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP /we4715" PARENT_SCOPE) 
		
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /w" PARENT_SCOPE)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /w" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /W4" PARENT_SCOPE)
	else()
		message(FATAL_ERROR "TODO: Mac support")
	endif()

	target_compile_definitions(${PROJECT_NAME} PRIVATE -DQUIVER -DVOXEL -DQUAKE2 -DVALVE_DLL -DCLIENT_WEAPONS -D_CRT_SECURE_NO_DEPRECATE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DPLUGIN_BUILD -DHLCOOP_BUILD PLUGIN_NAME="${PROJECT_NAME}")
	
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR MSVC)
		target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/dlls/pch_plugin.h>")
	endif()
	
	target_link_libraries(${PROJECT_NAME} PRIVATE ${SERVER_DLL_NAME})
	
	if (INSTALL_BINARIES)
		set(PLUGIN_OUT_PATH "${SERVER_WORK_DIR}/valve/${OUTPUT_PATH}")	
	else()
		set(PLUGIN_OUT_PATH "${CMAKE_BINARY_DIR}/output/${OUTPUT_PATH}")
	endif()
	
	if (MSVC)
		set_target_properties(${PROJECT_NAME} PROPERTIES
			VS_DEBUGGER_COMMAND "hlds.exe"
			VS_DEBUGGER_WORKING_DIRECTORY "${SERVER_WORK_DIR}"
			VS_DEBUGGER_COMMAND_ARGUMENTS "${SERVER_ARGS}"
		)
		
		set_property(GLOBAL PROPERTY USE_FOLDERS ON)
		
		if(OUTPUT_PATH MATCHES "^plugins/maps/")
			set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Map Plugins")
		else()
			set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Server Plugins")
		endif()	
	endif()
	
	set_target_properties(${PROJECT_NAME} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUT_PATH}/$<0:>"
	)
	if(UNIX)
		set_target_properties(${PROJECT_NAME} PROPERTIES
			LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUT_PATH}/$<0:>"
		)
	endif()
	
endfunction()
