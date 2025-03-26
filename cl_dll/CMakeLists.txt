project(${CLIENT_DLL_NAME})

set(WEAPON_HDR
	com_weapons.h
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
set(CLIENT_WEAPON_SRC
	hl/hl_weapons.cpp
	com_weapons.cpp
)

set(HUD_HDR
	ammo.h
	ammohistory.h
	health.h
	hud.h
	hud_servers.h
	hud_servers_priv.h
	hud_spectator.h
	wrect.h
)
set(HUD_SRC
	ammo.cpp
	ammo_secondary.cpp
	ammohistory.cpp
	battery.cpp
	death.cpp
	flashlight.cpp
	geiger.cpp
	health.cpp
	hud.cpp
	hud_bench.cpp
	hud_benchtrace.cpp
	hud_msg.cpp
	hud_redraw.cpp
	hud_servers.cpp
	hud_spectator.cpp
	hud_update.cpp
	menu.cpp
	message.cpp
	saytext.cpp
	status_icons.cpp
	statusbar.cpp
	text_message.cpp
	train.cpp
)

set(VGUI_HDR
	vgui_ControlConfigPanel.h
	vgui_int.h
	vgui_SchemeManager.h
	vgui_ScorePanel.h
	vgui_ServerBrowser.h
	vgui_SpectatorPanel.h
	../game_shared/vgui_scrollbar2.h
	../game_shared/vgui_slider2.h
)
set(VGUI_SRC
	vgui_ClassMenu.cpp
	vgui_ConsolePanel.cpp
	vgui_ControlConfigPanel.cpp
	vgui_CustomObjects.cpp
	vgui_int.cpp
	vgui_MOTDWindow.cpp
	vgui_SchemeManager.cpp
	vgui_ScorePanel.cpp
	vgui_ServerBrowser.cpp
	vgui_SpectatorPanel.cpp
	vgui_TeamFortressViewport.cpp
	vgui_teammenu.cpp
	
	../game_shared/vgui_checkbutton2.cpp
	../game_shared/vgui_grid.cpp
	../game_shared/vgui_helpers.cpp
	../game_shared/vgui_listbox.cpp
	../game_shared/vgui_loadtga.cpp
	../game_shared/vgui_scrollbar2.cpp
	../game_shared/vgui_slider2.cpp
)

set(INPUT_HDR
	camera.h
	in_defs.h
	kbutton.h
)
set(INPUT_SRC
	in_camera.cpp
	input.cpp
	inputw32.cpp
)

set(MISC_HDR
	cl_dll.h
	../common/Platform.h
)
set(UTIL_HDR
	cl_util.h
	interpolation.h
	util_vector.h
	../common/parsemsg.h
	../pm_shared/pm_debug.h
	../pm_shared/pm_defs.h
	../pm_shared/pm_info.h
	../pm_shared/pm_materials.h
	../pm_shared/pm_movevars.h
	../pm_shared/pm_shared.h
)
set(UTIL_SRC
	cdll_int.cpp
	interpolation.cpp
	util.cpp
	../dlls/util/HashMap.cpp
	../common/parsemsg.cpp
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_math.cpp
	../pm_shared/pm_shared.cpp
	../public/interface.cpp
	../game_shared/shared_util.cpp
)

set(GAME_HDR
	demo.h
	ev_hldm.h
	view.h
	../game_shared/voice_banmgr.h
	../game_shared/voice_status.h
)
set(GAME_SRC
	ev_hldm.cpp
	demo.cpp
	events.cpp
	view.cpp
	voice_status.cpp
	../game_shared/voice_banmgr.cpp
)

set(RENDER_HDR
	GameStudioModelRenderer.h
	StudioModelRenderer.h
	tri.h
)
set(RENDER_SRC
	GameStudioModelRenderer.cpp
	studio_util.cpp
	StudioModelRenderer.cpp
	tri.cpp
)

set(ENTITY_HDR
	eventscripts.h
)
set(ENTITY_SRC
	entity.cpp
	ev_common.cpp
	hl/hl_events.cpp
	hl/hl_baseentity.cpp
	hl/hl_objects.cpp
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
	${SHARED_WEAPON_SRC} ${CLIENT_WEAPON_SRC} ${WEAPON_HDR}
	${HUD_SRC} ${HUD_HDR}
	${VGUI_SRC} ${VGUI_HDR}
	${INPUT_SRC} ${INPUT_HDR}
	${UTIL_SRC} ${UTIL_HDR}
	${GAME_SRC} ${GAME_HDR}
	${RENDER_SRC} ${RENDER_HDR}
	${ENTITY_SRC} ${ENTITY_HDR}
	${CMAKE_CURRENT_BINARY_DIR}/weapons_shared.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/utils/vgui/include)
include_directories(${CMAKE_SOURCE_DIR}/external)

add_library(${CLIENT_DLL_NAME} SHARED ${ALL_SRC})

target_compile_definitions(${CLIENT_DLL_NAME} PRIVATE -DCLIENT_DLL -DCLIENT_WEAPONS -DHL_DLL)

if(UNIX)
	add_definitions(-DLINUX)

	target_link_libraries(${CLIENT_DLL_NAME} 
		dl
		pthread
		stdc++
		SDL2
		:vgui.so
	)

	set_target_properties(${CLIENT_DLL_NAME} PROPERTIES
		PREFIX ""
		COMPILE_FLAGS "-m32 -Werror=return-type -Wno-format-truncation -Wno-unused-result" LINK_FLAGS "-m32 -L ../../linux/release -L ../../linux"
	)

elseif(MSVC)
	target_link_libraries(${CLIENT_DLL_NAME} 
		${CMAKE_CURRENT_SOURCE_DIR}/../utils/vgui/lib/win32_vc6/vgui.lib
		wsock32
		${CMAKE_CURRENT_SOURCE_DIR}/../lib/public/sdl2.lib
	)

	set_target_properties(${CLIENT_DLL_NAME} PROPERTIES
		VS_DEBUGGER_COMMAND "${CLIENT_WORK_DIR}/${CLIENT_EXE}"
		VS_DEBUGGER_WORKING_DIRECTORY "${CLIENT_WORK_DIR}"
		VS_DEBUGGER_COMMAND_ARGUMENTS "${CLIENT_ARGS}"
	)
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /W0 /we4715") 

	source_group("Header Files\\Entities" FILES ${ENTITY_HDR})	
	source_group("Header Files\\Weapons" FILES ${WEAPON_HDR})	
	source_group("Header Files\\Util" FILES ${UTIL_HDR})	
	source_group("Header Files\\Game" FILES ${GAME_HDR})	
	source_group("Header Files\\HUD" FILES ${HUD_HDR})	
	source_group("Header Files\\VGUI" FILES ${VGUI_HDR})	
	source_group("Header Files\\Input" FILES ${INPUT_HDR})	
	source_group("Header Files\\Render" FILES ${RENDER_HDR})	
	
	source_group("Source Files\\Entities" FILES ${ENTITY_SRC})	
	source_group("Source Files\\Weapons\\Shared" FILES ${SHARED_WEAPON_SRC} ${CMAKE_CURRENT_BINARY_DIR}/weapons_shared.cpp)	
	source_group("Source Files\\Weapons" FILES ${CLIENT_WEAPON_SRC})	
	source_group("Source Files\\Util" FILES ${UTIL_SRC})	
	source_group("Source Files\\Game" FILES ${GAME_SRC})	
	source_group("Source Files\\HUD" FILES ${HUD_SRC})	
	source_group("Source Files\\VGUI" FILES ${VGUI_SRC})	
	source_group("Source Files\\Input" FILES ${INPUT_SRC})	
	source_group("Source Files\\Render" FILES ${RENDER_SRC})	

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