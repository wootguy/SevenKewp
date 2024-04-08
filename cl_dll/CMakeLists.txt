set(WEAPON_HDR
	com_weapons.h
)
set(WEAPON_SRC
	hl/hl_weapons.cpp
	../dlls/wpn_shared/hl_wpn_glock.cpp
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

set(ALL_SRC	
	${MISC_HDR}
	${WEAPON_SRC} ${WEAPON_HDR}
	${HUD_SRC} ${HUD_HDR}
	${VGUI_SRC} ${VGUI_HDR}
	${INPUT_SRC} ${INPUT_HDR}
	${UTIL_SRC} ${UTIL_HDR}
	${GAME_SRC} ${GAME_HDR}
	${RENDER_SRC} ${RENDER_HDR}
	${ENTITY_SRC} ${ENTITY_HDR}
	
)

include_directories(../dlls)
include_directories(../dlls/env)
include_directories(.)
include_directories(../public)
include_directories(../common)
include_directories(../pm_shared)
include_directories(../engine)
include_directories(../utils/vgui/include)
include_directories(../game_shared)
include_directories(../external)

add_library(${CLIENT_DLL_NAME} SHARED ${ALL_SRC})

target_compile_definitions(${CLIENT_DLL_NAME} PRIVATE -DCLIENT_DLL -DCLIENT_WEAPONS -DHL_DLL)

if (USE_AUTOHOTKEY_SCRIPT)
	add_custom_command(TARGET ${CLIENT_DLL_NAME} POST_BUILD
		COMMAND start ${CMAKE_CURRENT_SOURCE_DIR}/../resize_console.ahk
	)
endif()

if(UNIX)

	add_definitions(-DLINUX)

	add_custom_command(TARGET ${CLIENT_DLL_NAME} POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/${GAMEDIR_NAME}/cl_dlls
	    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${CLIENT_DLL_NAME}> ${PROJECT_SOURCE_DIR}/${GAMEDIR_NAME}/cl_dlls/
	)

	target_link_libraries(${CLIENT_DLL_NAME} 
		dl
		pthread
		stdc++
		SDL2
		:vgui.so
	)

	set_target_properties(${CLIENT_DLL_NAME} PROPERTIES
		PREFIX ""
		COMPILE_FLAGS "-m32" LINK_FLAGS "-m32 -L ../../linux/release -L ../../linux"
	)

elseif(MSVC)
	
	add_custom_command(TARGET ${CLIENT_DLL_NAME} POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${CLIENT_DLL_NAME}> ${ENGINE_DIR}/${GAMEDIR_NAME}/cl_dlls/
	    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/${GAMEDIR_NAME}/cl_dlls
	    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${CLIENT_DLL_NAME}> ${PROJECT_SOURCE_DIR}/${GAMEDIR_NAME}/cl_dlls/
	)

	target_link_libraries(${CLIENT_DLL_NAME} 
		${CMAKE_CURRENT_SOURCE_DIR}/../utils/vgui/lib/win32_vc6/vgui.lib
		wsock32
		${CMAKE_CURRENT_SOURCE_DIR}/../lib/public/sdl2.lib
	)

	set_target_properties(${CLIENT_DLL_NAME} PROPERTIES
		VS_DEBUGGER_COMMAND "${ENGINE_DIR}/${ENGINE_EXE}"
		VS_DEBUGGER_WORKING_DIRECTORY "${ENGINE_DIR}"
		VS_DEBUGGER_COMMAND_ARGUMENTS "${GAME_ARGS}"
	)
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /W0") 

	source_group("Header Files\\Entities" FILES ${ENTITY_HDR})	
	source_group("Header Files\\Weapons" FILES ${WEAPON_HDR})	
	source_group("Header Files\\Util" FILES ${UTIL_HDR})	
	source_group("Header Files\\Game" FILES ${GAME_HDR})	
	source_group("Header Files\\HUD" FILES ${HUD_HDR})	
	source_group("Header Files\\VGUI" FILES ${VGUI_HDR})	
	source_group("Header Files\\Input" FILES ${INPUT_HDR})	
	source_group("Header Files\\Render" FILES ${RENDER_HDR})	
	
	source_group("Source Files\\Entities" FILES ${ENTITY_SRC})	
	source_group("Source Files\\Weapons" FILES ${WEAPON_SRC})	
	source_group("Source Files\\Util" FILES ${UTIL_SRC})	
	source_group("Source Files\\Game" FILES ${GAME_SRC})	
	source_group("Source Files\\HUD" FILES ${HUD_SRC})	
	source_group("Source Files\\VGUI" FILES ${VGUI_SRC})	
	source_group("Source Files\\Input" FILES ${INPUT_SRC})	
	source_group("Source Files\\Render" FILES ${RENDER_SRC})	

else()
	message(FATAL_ERROR "TODO: Mac support")
endif()
