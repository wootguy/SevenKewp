cmake_minimum_required(VERSION 3.6)

# setup standard include directories and compile settings
function(hlcoop_setup_plugin OUTPUT_PATH)
	if(UNIX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -g -O2 -fno-strict-aliasing -Wall -Wextra -Wpedantic -Wno-invalid-offsetof -Wno-class-memaccess -Wno-unused-parameter") 
		set(CMAKE_SHARED_LINKER_FLAGS "m32 -g") 
		
		# Static linking libstd++ and libgcc so that the plugin can load on distros other than one it was compiled on.
		# -fvisibility=hidden fixes a weird bug where the metamod confuses game functions with plugin functions.
		# -g includes debug symbols which provides useful crash logs, but also inflates the .so file size a lot.
		# warnings are disabled in release mode (users don't care about that)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -std=c++11 -fvisibility=hidden -static-libstdc++ -static-libgcc -g" PARENT_SCOPE)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0" PARENT_SCOPE)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -w -Wfatal-errors" PARENT_SCOPE)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -static-libgcc -g" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wall" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -w -Wfatal-errors" PARENT_SCOPE)
		
		set(CMAKE_SHARED_LIBRARY_PREFIX "" PARENT_SCOPE)
		
		set(CMAKE_SHARED_LIBRARY_PREFIX "" PARENT_SCOPE)
		
	elseif(MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /W4") 
		
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /w" PARENT_SCOPE)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /MTd" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT /w" PARENT_SCOPE)
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi /MTd" PARENT_SCOPE)
	else()
		message(FATAL_ERROR "TODO: Mac support")
	endif()

	target_compile_definitions(${PROJECT_NAME} PRIVATE -DQUIVER -DVOXEL -DQUAKE2 -DVALVE_DLL -DCLIENT_WEAPONS -D_CRT_SECURE_NO_DEPRECATE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DPLUGIN_BUILD PLUGIN_NAME="${PROJECT_NAME}")
	
	target_link_libraries(${PROJECT_NAME} PRIVATE ${SERVER_DLL_NAME})
	
	set(PLUGIN_OUT_PATH "${CMAKE_BINARY_DIR}/output/${OUTPUT_PATH}")
	set_target_properties(${PROJECT_NAME} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUT_PATH}
		RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PLUGIN_OUT_PATH}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PLUGIN_OUT_PATH}
	)
	
	if(UNIX)
		set_target_properties(${PROJECT_NAME} PROPERTIES
			LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUT_PATH}
			LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PLUGIN_OUT_PATH}
			LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PLUGIN_OUT_PATH}
		)
	endif()
	
endfunction()
