
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
	CFurniture.cpp
	CMonsterMaker.cpp
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
set(MONSTER_SCRIPTED_SRC
	CCineAI.cpp
	CCineMonster.cpp
	CScriptedSentence.cpp
)

set(PLAYER_HDR
	client.h
	player.h
	player_util.h
	spectator.h
)
set(PLAYER_SRC
	client.cpp
	player.cpp
	player_util.cpp
	observer.cpp
	spectator.cpp
	../game_shared/voice_gamemgr.cpp
)

set(ENV_HDR
	env/CBeam.h
	env/CGibShooter.h
	env/CLaser.h
	env/CLight.h
	env/CSoundEnt.h
	env/CSprite.h
	env/explode.h
)
set(ENV_SRC
	env/CAmbientGeneric.cpp
	env/CBeam.cpp
	env/CBlood.cpp
	env/CBubbling.cpp
	env/CCorpse.cpp
	env/CDecal.cpp
	env/CEnvBeverage.cpp
	env/CEnvExplosion.cpp
	env/CEnvFunnel.cpp
	env/CEnvGlobal.cpp
	env/CEnvLight.cpp
	env/CEnvShooter.cpp
	env/CEnvSound.cpp
	env/CEnvSpark.cpp
	env/CFade.cpp
	env/CGibShooter.cpp
	env/CGlow.cpp
	env/CLaser.cpp
	env/CLight.cpp
	env/CLightning.cpp
	env/CMessage.cpp
	env/CShake.cpp
	env/CShower.cpp
	env/CSoundEnt.cpp
	env/CSpeaker.cpp
	env/CSprite.cpp
	env/CTestEffect.cpp
	env/decals.h
	env/effects.h
)

set(ITEM_HDR
	item/CItem.h
)
set(ITEM_SRC
	item/CAirtank.cpp
	item/CHealthKit.cpp
	item/CItem.cpp
	item/CItemAntidote.cpp
	item/CItemBattery.cpp
	item/CItemLongJump.cpp
	item/CItemSecurity.cpp
	item/CItemSoda.cpp
	item/CItemSuit.cpp
	item/CWorldItem.cpp
)

set(TRIGGER_HDR
	triggers/CBaseTrigger.h
	triggers/CFireAndDie.h
	triggers/CTriggerMultiple.h
)
set(TRIGGER_SRC
	triggers/CAutoTrigger.cpp
	triggers/CBaseTrigger.cpp
	triggers/CChangeLevel.cpp
	triggers/CFrictionModifier.cpp
	triggers/CFireAndDie.cpp
	triggers/CLadder.cpp
	triggers/CMultiManager.cpp
	triggers/CMultiSource.cpp
	triggers/CRenderFxManager.cpp
	triggers/CTriggerCamera.cpp
	triggers/CTargetCDAudio.cpp
	triggers/CTriggerCDAudio.cpp
	triggers/CTriggerChangeTarget.cpp
	triggers/CTriggerCounter.cpp
	triggers/CTriggerEndSection.cpp
	triggers/CTriggerGravity.cpp
	triggers/CTriggerHurt.cpp
	triggers/CTriggerMonsterJump.cpp
	triggers/CTriggerMultiple.cpp
	triggers/CTriggerPush.cpp
	triggers/CTriggerRelay.cpp
	triggers/CTriggerSave.cpp
	triggers/CTriggerTeleport.cpp
	triggers/CTriggerVolume.cpp
)

set(FUNC_HDR
	func/CBaseButton.h
	func/CBasePlatTrain.h
	func/CBreakable.h
	func/func_util.h
	func/CBaseDoor.h
	func/CFuncPlat.h
	func/CFuncPlatRot.h
	func/CFuncTank.h
	func/CFuncTrackChange.h
	func/CFuncTrackTrain.h
	func/CFuncWall.h
	func/CPlatTrigger.h
)
set(FUNC_SRC
	func/CBaseButton.cpp
	func/CBaseDoor.cpp
	func/CBasePlatTrain.cpp
	func/CBreakable.cpp
	func/CButtonTarget.cpp
	func/func_util.cpp
	func/CFuncConveyor.cpp
	func/CFuncIllusionary.cpp
	func/CFuncMonsterClip.cpp
	func/CFuncMortarField.cpp
	func/CFuncPendulum.cpp
	func/CFuncPlat.cpp
	func/CFuncPlatRot.cpp
	func/CFuncRotating.cpp
	func/CFuncTank.cpp
	func/CFuncTankControls.cpp
	func/CFuncTankGun.cpp
	func/CFuncTankLaser.cpp
	func/CFuncTankMortar.cpp
	func/CFuncTrackChange.cpp
	func/CFuncTrackTrain.cpp
	func/CFuncTrain.cpp
	func/CFuncTrainControls.cpp
	func/CFuncWall.cpp
	func/CFuncWallToggle.cpp
	func/CGunTarget.cpp
	func/CMomentaryDoor.cpp
	func/CMomentaryRotButton.cpp
	func/CPlatTrigger.cpp
	func/CPushable.cpp
	func/CRecharge.cpp
	func/CRotButton.cpp
	func/CRotDoor.cpp
)

set(ENTITY_HDR
	bodyque.h
	cbase.h
	ent_globals.h
	nodes.h
	scripted.h
	scriptevent.h
	../engine/progdefs.h
	../engine/edict.h
	../common/const.h
)
set(ENTITY_SRC
	cbase.cpp
	CBaseDelay.cpp
	CBaseDMStart.cpp
	CBaseToggle.cpp
	CGlobalState.cpp
	CNullEntity.cpp
	CPointEntity.cpp
	CWorld.cpp
	subs.cpp
)

set(PATH_HDR
	path/CPathCorner.h
	path/CPathTrack.h
)
set(PATH_SRC
	path/CPathCorner.cpp
	path/CPathTrack.cpp
	nodes.cpp
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
	mortar.cpp
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
	sound.cpp
	teamplay_gamerules.cpp
)

set(ALL_SRC
	${MONSTER_SRC} ${MONSTER_SHARE_SRC} ${MONSTER_SCRIPTED_SRC} ${MONSTER_HDR}
	${PLAYER_SRC} ${PLAYER_HDR}
	${ENTITY_SRC} ${ENTITY_HDR}
	${ENV_SRC} ${ENV_HDR}
	${ITEM_SRC} ${ITEM_HDR}
	${FUNC_HDR} ${FUNC_SRC}
	${PATH_HDR} ${PATH_SRC}
	${TRIGGER_HDR} ${TRIGGER_SRC}
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
include_directories(env)
include_directories(func)
include_directories(trigger)

add_library(${SERVER_DLL_NAME} SHARED ${ALL_SRC})


target_compile_definitions(${SERVER_DLL_NAME} PRIVATE -DQUIVER -DVOXEL -DQUAKE2 -DVALVE_DLL -DCLIENT_WEAPONS)

add_custom_command(TARGET ${SERVER_DLL_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${SERVER_DLL_NAME}> ${ENGINE_DIR}/${PROJECT_NAME}/dlls/
)

if(MSVC)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:${CMAKE_CURRENT_SOURCE_DIR}/hl.def") 
	
	set_target_properties(${SERVER_DLL_NAME} PROPERTIES
		VS_DEBUGGER_COMMAND "${ENGINE_DIR}/${ENGINE_EXE}"
		VS_DEBUGGER_WORKING_DIRECTORY "${ENGINE_DIR}"
		VS_DEBUGGER_COMMAND_ARGUMENTS "-steam -dev -console -game opencoop -sv_cheats 1 -maxplayers 2 +map c2a1"
	)
	
	
	source_group("Header Files\\Entities" FILES ${ENTITY_HDR})
	source_group("Header Files\\Entities\\env" FILES ${ENV_HDR})
	source_group("Header Files\\Entities\\func" FILES ${FUNC_HDR})
	source_group("Header Files\\Entities\\item" FILES ${ITEM_HDR})
	source_group("Header Files\\Entities\\monster" FILES ${MONSTER_HDR})
	source_group("Header Files\\Entities\\path" FILES ${PATH_HDR})
	source_group("Header Files\\Entities\\trigger" FILES ${TRIGGER_HDR})
	source_group("Header Files\\Entities\\weapon" FILES ${WEAPON_HDR})
	source_group("Header Files\\Player" FILES ${PLAYER_HDR})
	source_group("Header Files\\Game" FILES ${GAME_HDR})
	source_group("Header Files\\Util" FILES ${UTIL_HDR})
	
	source_group("Source Files\\Player" FILES ${PLAYER_SRC})
	source_group("Source Files\\Entities" FILES ${ENTITY_SRC})
	source_group("Source Files\\Entities\\env" FILES ${ENV_SRC})
	source_group("Source Files\\Entities\\func" FILES ${FUNC_SRC})
	source_group("Source Files\\Entities\\item" FILES ${ITEM_SRC})
	source_group("Source Files\\Entities\\monster" FILES ${MONSTER_SRC})
	source_group("Source Files\\Entities\\monster\\common" FILES ${MONSTER_SHARE_SRC})
	source_group("Source Files\\Entities\\monster\\scripted" FILES ${MONSTER_SCRIPTED_SRC})
	source_group("Source Files\\Entities\\path" FILES ${PATH_SRC})
	source_group("Source Files\\Entities\\trigger" FILES ${TRIGGER_SRC})
	source_group("Source Files\\Entities\\weapon" FILES ${WEAPON_SRC})
	source_group("Source Files\\Util" FILES ${UTIL_SRC})
	source_group("Source Files\\Game" FILES ${GAME_SRC})

else()
	message(FATAL_ERROR "TODO: Linux/mac support")
endif()
