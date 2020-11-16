
set(MISC_HDR
	../common/Platform.h
	cdll_dll.h
	enginecallback.h
	extdll.h
)

set(MONSTER_HDR
	activity.h
	activitymap.h
	animation.h
	basemonster.h
	defaultai.h
	flyingmonster.h
	hornet.h
	monsterevent.h
	monsters.h
	schedule.h
	squadmonster.h
	talkmonster.h
)
set(MONSTER_SRC
	aflock.cpp
	agrunt.cpp
	apache.cpp
	barnacle.cpp
	barney.cpp
	bigmomma.cpp
	bloater.cpp
	bullsquid.cpp
	controller.cpp
	gargantua.cpp
	genericmonster.cpp
	gman.cpp
	h_cycler.cpp
	hassassin.cpp
	headcrab.cpp
	hgrunt.cpp
	hornet.cpp
	houndeye.cpp
	ichthyosaur.cpp
	islave.cpp
	leech.cpp
	nihilanth.cpp
	osprey.cpp
	rat.cpp
	roach.cpp
	scientist.cpp
	tentacle.cpp
	turret.cpp
	xen.cpp
	zombie.cpp
)
set(MONSTER_SHARE_SRC
	animating.cpp
	animation.cpp
	combat.cpp
	defaultai.cpp
	flyingmonster.cpp
	h_ai.cpp
	monsters.cpp
	monsterstate.cpp
	schedule.cpp
	squadmonster.cpp
	talkmonster.cpp
	tempmonster.cpp
)

set(PLAYER_HDR
	client.h
	player.h
	spectator.h
)
set(PLAYER_SRC
	client.cpp
	player.cpp
	observer.cpp
	spectator.cpp
	../game_shared/voice_gamemgr.cpp
)

set(ENTITY_HDR
	cbase.h
	decals.h
	doors.h
	effects.h
	explode.h
	func_break.h
	items.h
	nodes.h
	scripted.h
	scriptevent.h
	soundent.h
	trains.h
)
set(ENTITY_SRC
	bmodels.cpp
	buttons.cpp
	doors.cpp
	cbase.cpp
	effects.cpp
	explode.cpp
	func_break.cpp
	func_tank.cpp
	h_battery.cpp
	h_cine.cpp
	healthkit.cpp
	items.cpp
	lights.cpp
	monstermaker.cpp
	mortar.cpp
	nodes.cpp
	pathcorner.cpp
	airtank.cpp
	plats.cpp
	scripted.cpp
	sound.cpp
	soundent.cpp
	subs.cpp
	triggers.cpp
	world.cpp
)

set(WEAPON_HDR
	weapons.h
)
set(WEAPON_SRC
	crossbow.cpp
	crowbar.cpp
	egon.cpp
	gauss.cpp
	ggrenade.cpp
	handgrenade.cpp	
	hornetgun.cpp
	mp5.cpp
	python.cpp
	rpg.cpp
	satchel.cpp
	shotgun.cpp
	squeakgrenade.cpp
	tripmine.cpp
	weapons.cpp
	wpn_shared/hl_wpn_glock.cpp
)

set(UTIL_HDR
	plane.h
	util.h
	vector.h
)
set(UTIL_SRC
	plane.cpp
	util.cpp
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_math.cpp
	../pm_shared/pm_shared.cpp
)

set(GAME_HDR
	gamerules.h
	saverestore.h
	skill.h
	teamplay_gamerules.h
)
set(GAME_SRC
	game.cpp
	gamerules.cpp
	globals.cpp
	h_export.cpp
	maprules.cpp
	multiplay_gamerules.cpp
	singleplay_gamerules.cpp
	skill.cpp
	teamplay_gamerules.cpp
)

set(ALL_SRC
	${MONSTER_SRC} ${MONSTER_HDR}
	${MONSTER_SHARE_SRC}
	${PLAYER_SRC} ${PLAYER_HDR}
	${ENTITY_SRC} ${ENTITY_HDR}
	${WEAPON_SRC} ${WEAPON_HDR}
	${UTIL_SRC} ${UTIL_HDR}
	${GAME_SRC} ${GAME_HDR}
	${MISC_HDR}
)

include_directories(.)
include_directories(../common)
include_directories(../engine)
include_directories(../game_shared)
include_directories(../pm_shared)
include_directories(../public)

add_library(${SERVER_DLL_NAME} SHARED ${ALL_SRC})

add_custom_command(TARGET ${SERVER_DLL_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${SERVER_DLL_NAME}> ${ENGINE_DIR}/${PROJECT_NAME}/dlls/
)

if(MSVC)
	set_target_properties(${SERVER_DLL_NAME} PROPERTIES
		VS_DEBUGGER_COMMAND "${ENGINE_DIR}/${ENGINE_EXE}"
		VS_DEBUGGER_WORKING_DIRECTORY "${ENGINE_DIR}"
		VS_DEBUGGER_COMMAND_ARGUMENTS "-steam -dev -console -game opencoop -sv_cheats 1 -maxplayers 2 +map c2a1"
	)
	
	source_group("Header Files\\Monsters" FILES ${MONSTER_HDR})	
	source_group("Header Files\\Entities" FILES ${ENTITY_HDR})	
	source_group("Header Files\\Player" FILES ${PLAYER_HDR})	
	source_group("Header Files\\Game" FILES ${GAME_HDR})	
	source_group("Header Files\\Util" FILES ${UTIL_HDR})	
	source_group("Header Files\\Weapon" FILES ${WEAPON_HDR})	
	
	source_group("Source Files\\Monsters" FILES ${MONSTER_SRC})	
	source_group("Source Files\\Monsters\\common" FILES ${MONSTER_SHARE_SRC})	
	source_group("Source Files\\Player" FILES ${PLAYER_SRC})	
	source_group("Source Files\\Entities" FILES ${ENTITY_SRC})	
	source_group("Source Files\\Weapons" FILES ${WEAPON_SRC})	
	source_group("Source Files\\Util" FILES ${UTIL_SRC})	
	source_group("Source Files\\Game" FILES ${GAME_SRC})	

else()
	message(FATAL_ERROR "TODO: Linux/mac support")
endif()
