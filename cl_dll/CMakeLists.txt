project(${CLIENT_DLL_NAME})

set(WEAPON_HDR
	weapons/com_weapons.h
	weapons/ev_hldm.h
	weapons/eventscripts.h
)
set(SHARED_WEAPON_SRC
	../dlls/weapon/CGlock.cpp
	../dlls/weapon/CCrossbow.cpp
	../dlls/weapon/CCrowbar.cpp
	../dlls/weapon/CEgon.cpp
	../dlls/weapon/CGauss.cpp
	../dlls/weapon/CHandGrenade.cpp
	../dlls/weapon/CHgun.cpp
	../dlls/weapon/CMP5.cpp
	../dlls/weapon/CPython.cpp
	../dlls/weapon/CRpg.cpp
	../dlls/weapon/CSatchel.cpp
	../dlls/weapon/CShotgun.cpp
	../dlls/weapon/CSqueak.cpp
	../dlls/weapon/CTripmine.cpp
)
set(CUSTOM_WEAPON_SRC
	../dlls/weapon/custom/CWeaponCustom.cpp
	../dlls/weapon/custom/CWeaponEvents.cpp
	../dlls/weapon/custom/wc_net.cpp
	../dlls/weapon/custom/wc_schema.cpp
)
set(CLIENT_WEAPON_SRC
	weapons/com_weapons.cpp
	weapons/ev_hldm.cpp
	weapons/ev_common.cpp
	weapons/events.cpp
	weapons/hl_events.cpp
	weapons/hl_objects.cpp
	weapons/hl_baseentity.cpp
	weapons/hl_weapons.cpp
)

set(HUD_HDR
	hud/bench.h
	hud/hud.h
	hud/servers.h
	hud/servers_priv.h
	hud/wrect.h
	hud/player_info.h
	hud/global_consts.h
	../game_shared/voice_banmgr.h
	../game_shared/voice_status.h
)
set(HUD_SRC
	hud/benchtrace.cpp
	hud/hud.cpp
	hud/hud_msg.cpp
	hud/hud_redraw.cpp
	hud/servers.cpp
	hud/hud_update.cpp
	hud/voice_status.cpp
	../game_shared/voice_banmgr.cpp
)
set(HUD_COMPONENTS_HDR
	hud/components/hud_ammo.h
	hud/components/hud_ammohistory.h
	hud/components/hud_health.h
	hud/components/hud_spectator.h
)
set(HUD_COMPONENTS_SRC
	hud/components/hud_ammo.cpp
	hud/components/hud_ammo_secondary.cpp
	hud/components/hud_ammohistory.cpp
	hud/components/hud_battery.cpp
	hud/components/hud_death.cpp
	hud/components/hud_flashlight.cpp
	hud/components/hud_health.cpp
	hud/components/hud_conprint.cpp
	hud/components/hud_bench.cpp
	hud/components/hud_spectator.cpp
	hud/components/hud_menu.cpp
	hud/components/hud_nametags.cpp
	hud/components/hud_debug.cpp
	hud/components/hud_entstatus.cpp
	hud/components/hud_sprites.cpp
	hud/components/hud_message.cpp
	hud/components/hud_saytext.cpp
	hud/components/hud_clientstats.cpp
	hud/components/hud_statusbar.cpp
	hud/components/hud_text_message.cpp
	hud/components/hud_train.cpp
	hud/components/hud_web_updater.cpp
)

set(EFFECTS_HDR
	effects/effects.h
)
set(EFFECTS_SRC
	effects/fog.cpp
	effects/geiger.cpp
	effects/status_icons.cpp
	effects/effects.cpp
)

set(VGUI_HDR
	hud/vgui/vgui_ControlConfigPanel.h
	hud/vgui/vgui_int.h
	hud/vgui/vgui_SchemeManager.h
	hud/vgui/vgui_ScorePanel.h
	hud/vgui/vgui_ServerBrowser.h
	hud/vgui/vgui_SpectatorPanel.h
	../game_shared/vgui_scrollbar2.h
	../game_shared/vgui_slider2.h
)
set(VGUI_SRC
	hud/vgui/vgui_ClassMenu.cpp
	hud/vgui/vgui_ConsolePanel.cpp
	hud/vgui/vgui_ControlConfigPanel.cpp
	hud/vgui/vgui_CustomObjects.cpp
	hud/vgui/vgui_int.cpp
	hud/vgui/vgui_MOTDWindow.cpp
	hud/vgui/vgui_SchemeManager.cpp
	hud/vgui/vgui_ScorePanel.cpp
	hud/vgui/vgui_ServerBrowser.cpp
	hud/vgui/vgui_SpectatorPanel.cpp
	hud/vgui/vgui_TeamFortressViewport.cpp
	hud/vgui/vgui_teammenu.cpp
	
	../game_shared/vgui_checkbutton2.cpp
	../game_shared/vgui_grid.cpp
	../game_shared/vgui_helpers.cpp
	../game_shared/vgui_listbox.cpp
	../game_shared/vgui_loadtga.cpp
	../game_shared/vgui_scrollbar2.cpp
	../game_shared/vgui_slider2.cpp
)

set(INPUT_HDR
	input/camera.h
	input/in_defs.h
	input/kbutton.h
)
set(INPUT_SRC
	input/in_camera.cpp
	input/input.cpp
	input/inputw32.cpp
)

set(MISC_HDR
	cl_dll.h
	../common/Platform.h
)
set(UTIL_HDR
	util/cl_util.h
	util/interpolation.h
	../game_shared/prediction_files.h
	../common/parsemsg.h
	../game_shared/shared_util.h
	../game_shared/shared_effects.h
	../game_shared/rgb.h
	../game_shared/md5.h
	../game_shared/mstream.h
)
set(UTIL_SRC
	util/interpolation.cpp
	util/cl_util.cpp
	../game_shared/HashMap.cpp
	../game_shared/StringPool.cpp
	../common/parsemsg.cpp
	../public/interface.cpp
	../game_shared/shared_util.cpp
	../game_shared/prediction_files.cpp
	../game_shared/md5.cpp
	../game_shared/mstream.cpp
	../game_shared/miniz.c
)

set(RENDER_HDR
	rendering/GameStudioModelRenderer.h
	rendering/StudioModelRenderer.h
	rendering/tri.h
	rendering/gfx_util.h
	rendering/view.h
)
set(RENDER_SRC
	rendering/GameStudioModelRenderer.cpp
	rendering/studio_util.cpp
	rendering/StudioModelRenderer.cpp
	rendering/tri.cpp
	rendering/view.cpp
	rendering/gfx_util.cpp
)

set(MOVEMENT_HDR
	../pm_shared/pm_debug.h
	../pm_shared/pm_defs.h
	../pm_shared/pm_info.h
	../pm_shared/pm_materials.h
	../pm_shared/pm_movevars.h
	../pm_shared/pm_shared.h
)
set(MOVEMENT_SRC
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_math.cpp
	../pm_shared/pm_shared.cpp
)

set(HOOKS_HDR
	hooks/engine_pv.h
	hooks/demo.h
)
set(HOOKS_SRC
	hooks/entity.cpp
	hooks/cdll_int.cpp
	hooks/engine_pv.cpp
	hooks/demo.cpp
)

function(write_wep_source_file)	
	set(ENT_SRCS_EXT ${SHARED_WEAPON_SRC})
	list(LENGTH ENT_SRCS_EXT list_count)
	math(EXPR list_max_index ${list_count}-1)
	foreach(i RANGE ${list_max_index})
		list(GET ENT_SRCS_EXT ${i} x)
		
		set(WEP_INCLUDES "${WEP_INCLUDES} #include \"${x}\"\n")
	endforeach(i)
	
	set(OUT_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/weapons_shared.cpp)
	
	# Read the existing contents of the file if it exists
    if(EXISTS ${OUT_FILE_PATH})
        file(READ ${OUT_FILE_PATH} CURRENT_CONTENT)
    else()
        set(CURRENT_CONTENT "")
    endif()
	
    # Only write if the new content is different
    if(NOT "${CURRENT_CONTENT}" STREQUAL "${WEP_INCLUDES}")
        file(WRITE ${OUT_FILE_PATH} ${WEP_INCLUDES})
    endif()
endfunction()

write_wep_source_file()

# don't compile individual entity files
set_source_files_properties(${SHARED_WEAPON_SRC} PROPERTIES HEADER_FILE_ONLY TRUE)

set(ALL_SRC	
	${MISC_HDR}
	${SHARED_WEAPON_SRC} ${CLIENT_WEAPON_SRC} ${CUSTOM_WEAPON_SRC} ${WEAPON_HDR}
	${MOVEMENT_SRC} ${MOVEMENT_HDR}
	${HUD_SRC} ${HUD_HDR}
	${HUD_COMPONENTS_SRC} ${HUD_COMPONENTS_HDR}
	${EFFECTS_SRC} ${EFFECTS_HDR}
	${VGUI_SRC} ${VGUI_HDR}
	${INPUT_SRC} ${INPUT_HDR}
	${UTIL_SRC} ${UTIL_HDR}
	${RENDER_SRC} ${RENDER_HDR}
	${HOOKS_SRC} ${HOOKS_HDR}
	${CMAKE_CURRENT_BINARY_DIR}/weapons_shared.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/utils/vgui/include)
include_directories(${CMAKE_SOURCE_DIR}/external)
include_directories(${CMAKE_SOURCE_DIR}/external/curl/include/)

if (STATIC_MSVC_RUNTIME)
	# static runtime
	add_compile_options(/MT)
	add_definitions(-UHAVE___BUILTIN_ASSUME_ALIGNED) # probably only needed for v141_xp
endif()

add_library(${CLIENT_DLL_NAME} SHARED ${ALL_SRC})

target_compile_definitions(${CLIENT_DLL_NAME} PRIVATE -DCLIENT_DLL -DCLIENT_WEAPONS -DHL_DLL -DCURL_STATICLIB)

target_include_directories(${CLIENT_DLL_NAME} PRIVATE .)
target_include_directories(${CLIENT_DLL_NAME} PRIVATE hud)
target_include_directories(${CLIENT_DLL_NAME} PRIVATE hud/components)
target_include_directories(${CLIENT_DLL_NAME} PRIVATE hud/vgui)
target_include_directories(${CLIENT_DLL_NAME} PRIVATE weapons)
target_include_directories(${CLIENT_DLL_NAME} PRIVATE rendering)
target_include_directories(${CLIENT_DLL_NAME} PRIVATE effects)
target_include_directories(${CLIENT_DLL_NAME} PRIVATE input)
target_include_directories(${CLIENT_DLL_NAME} PRIVATE hooks)
target_include_directories(${CLIENT_DLL_NAME} PRIVATE util)

if(UNIX)
	add_definitions(-DLINUX)

	target_link_libraries(${CLIENT_DLL_NAME} 
		dl
		pthread
		stdc++
		${CMAKE_SOURCE_DIR}/linux/libSDL2-2.0.so.0
		:vgui.so
	)
	
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -static-libgcc -static-libstdc++ -Wl,--gc-sections")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
	
	if (ENABLE_CURL)
		target_compile_definitions(${CLIENT_DLL_NAME} PRIVATE -DENABLE_CURL)
		target_link_libraries(${CLIENT_DLL_NAME} 
			${CMAKE_SOURCE_DIR}/external/curl/build/lib/libcurl.a
			${CMAKE_SOURCE_DIR}/external/mbedtls/build/library/libmbedtls.a
			${CMAKE_SOURCE_DIR}/external/mbedtls/build/library/libmbedx509.a
			${CMAKE_SOURCE_DIR}/external/mbedtls/build/library/libmbedcrypto.a
		)
	endif()

	set_target_properties(${CLIENT_DLL_NAME} PROPERTIES
		PREFIX ""
		COMPILE_FLAGS "-m32 -Werror=return-type -Wno-format-truncation -Wno-unused-result" LINK_FLAGS "-m32 -L ../../linux/release -L ../../linux"
	)

elseif(MINGW)
	target_link_libraries(${CLIENT_DLL_NAME} wsock32 opengl32
		${CMAKE_SOURCE_DIR}/utils/vgui/lib/vgui.dll # this doesn't work. The C++ ABI is incompatible. Need a C wrapper lib.
		${CMAKE_SOURCE_DIR}/lib/public/sdl2.lib
	)
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -w")
	
	target_compile_definitions(${CLIENT_DLL_NAME} PRIVATE -DMINGW)
	
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)

elseif(MSVC)
	target_link_libraries(${CLIENT_DLL_NAME} wsock32 opengl32
		${CMAKE_SOURCE_DIR}/utils/vgui/lib/win32_vc6/vgui.lib
		${CMAKE_SOURCE_DIR}/lib/public/sdl2.lib
	)
	
	if (ENABLE_CURL)
		set(LIB_BUILD_PATH "build")
		if (STATIC_MSVC_RUNTIME)
			set(LIB_BUILD_PATH "build_static")
		endif()
	
		target_compile_definitions(${CLIENT_DLL_NAME} PRIVATE -DENABLE_CURL)
		target_link_libraries(${CLIENT_DLL_NAME} Ws2_32 Crypt32 Secur32 Wldap32 iphlpapi
			${CMAKE_SOURCE_DIR}/external/curl/${LIB_BUILD_PATH}/lib/Release/libcurl.lib
		)
	endif()

	set_target_properties(${CLIENT_DLL_NAME} PROPERTIES
		VS_DEBUGGER_COMMAND "${CLIENT_WORK_DIR}/${CLIENT_EXE}"
		VS_DEBUGGER_WORKING_DIRECTORY "${CLIENT_WORK_DIR}"
		VS_DEBUGGER_COMMAND_ARGUMENTS "${CLIENT_ARGS}"
	)
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /W0 /we4715") 

	source_group("Header Files\\Weapons" FILES ${WEAPON_HDR})	
	source_group("Header Files\\Util" FILES ${UTIL_HDR})	
	source_group("Header Files\\HUD" FILES ${HUD_HDR})	
	source_group("Header Files\\HUD\\VGUI" FILES ${VGUI_HDR})	
	source_group("Header Files\\HUD\\Components" FILES ${HUD_COMPONENTS_HDR})	
	source_group("Header Files\\Input" FILES ${INPUT_HDR})	
	source_group("Header Files\\Rendering" FILES ${RENDER_HDR})
	source_group("Header Files\\Hooks" FILES ${HOOKS_HDR})
	source_group("Header Files\\Effects" FILES ${EFFECTS_HDR})
	
	source_group("Source Files\\Weapons\\Default" FILES ${SHARED_WEAPON_SRC} ${CMAKE_CURRENT_BINARY_DIR}/weapons_shared.cpp)	
	source_group("Source Files\\Weapons\\Custom" FILES ${CUSTOM_WEAPON_SRC})	
	source_group("Source Files\\Weapons" FILES ${CLIENT_WEAPON_SRC})	
	source_group("Source Files\\Util" FILES ${UTIL_SRC})	
	source_group("Source Files\\Game" FILES ${GAME_SRC})	
	source_group("Source Files\\HUD" FILES ${HUD_SRC})	
	source_group("Source Files\\HUD\\Components" FILES ${HUD_COMPONENTS_SRC})	
	source_group("Source Files\\HUD\\VGUI" FILES ${VGUI_SRC})	
	source_group("Source Files\\Input" FILES ${INPUT_SRC})	
	source_group("Source Files\\Rendering" FILES ${RENDER_SRC})	
	source_group("Source Files\\Movement" FILES ${MOVEMENT_SRC})	
	source_group("Source Files\\Hooks" FILES ${HOOKS_SRC})	
	source_group("Source Files\\Effects" FILES ${EFFECTS_SRC})	

else()
	message(FATAL_ERROR "TODO: Mac support")
endif()

if (SETUP_IDE)
	set(MOD_OUTPUT_PATH "${CLIENT_WORK_DIR}/valve_addon/cl_dlls")
else()
	set(MOD_OUTPUT_PATH "${CMAKE_BINARY_DIR}/output/cl_dlls")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${MOD_OUTPUT_PATH}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${MOD_OUTPUT_PATH}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${MOD_OUTPUT_PATH}
)
if(UNIX)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${MOD_OUTPUT_PATH}
		LIBRARY_OUTPUT_DIRECTORY_DEBUG ${MOD_OUTPUT_PATH}
		LIBRARY_OUTPUT_DIRECTORY_RELEASE ${MOD_OUTPUT_PATH}
	)
endif()