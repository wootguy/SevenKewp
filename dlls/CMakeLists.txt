
set(MISC_HDR
	../engine/studio.h
	../common/Platform.h
	cdll_dll.h
	enginecallback.h
	extdll.h
)

set(MONSTER_HDR
	monster/activity.h
	monster/activitymap.h
	monster/bodyque.h
	monster/CAGrunt.h
	monster/CBaseGrunt.h
	monster/CBaseGruntAlly.h
	monster/CBaseMonster.h
	monster/CBaseTurret.h
	monster/CFlyingMonster.h
	monster/CGargantua.h
	monster/CHornet.h
	monster/CTalkSquadMonster.h
	monster/defaultai.h
	monster/monsterevent.h
	monster/monsters.h
	monster/schedule.h
)
set(MONSTER_SRC
	monster/CActAnimating.cpp
	monster/CAGrunt.cpp
	monster/CApache.cpp
	monster/CBabyGarg.cpp
	monster/CBabyVoltigore.cpp
	monster/CBarnacle.cpp
	monster/CBarney.cpp
	monster/CBaseAnimating.cpp
	monster/CBaseGrunt.cpp
	monster/CBaseGruntAlly.cpp
	monster/CBaseMonster.cpp
	monster/CBaseTurret.cpp
	monster/CBigMomma.cpp
	monster/CBloater.cpp
	monster/CBullsquid.cpp
	monster/CDeadHEV.cpp
	monster/CFlockingFlyerFlock.cpp
	monster/CFlyingMonster.cpp
	monster/CFurniture.cpp
	monster/CMiniTurret.cpp
	monster/CMonsterMaker.cpp
	monster/CController.cpp
	monster/CGargantua.cpp
	monster/CGenericMonster.cpp
	monster/CGMan.cpp
	monster/CGonome.cpp
	monster/CCycler.cpp
	monster/CHAssassin.cpp
	monster/CHeadCrab.cpp
	monster/CHGrunt.cpp
	monster/CHGruntAlly.cpp
	monster/CHornet.cpp
	monster/CHoundeye.cpp
	monster/CIchthyosaur.cpp
	monster/CISlave.cpp
	monster/CLeech.cpp
	monster/CMassn.cpp
	monster/CNihilanth.cpp
	monster/COFMedicAlly.cpp
	monster/COFTorchAlly.cpp
	monster/COsprey.cpp
	monster/COtis.cpp
	monster/CPitdrone.cpp
	monster/CRat.cpp
	monster/CRoach.cpp
	monster/CScientist.cpp
	monster/CSentry.cpp
	monster/CShockRoach.cpp
	monster/CShockTrooper.cpp
	monster/CTalkSquadMonster.cpp
	monster/CTentacle.cpp
	monster/CTor.cpp
	monster/CTurret.cpp
	monster/CVoltigore.cpp
	monster/CXenHair.cpp
	monster/CXenPLight.cpp
	monster/CXenSpore.cpp
	monster/CXenTree.cpp
	monster/CZombie.cpp
	monster/tempmonster.cpp
)
set(MONSTER_AI_SRC
	monster/defaultai.cpp
	monster/h_ai.cpp
)
set(MONSTER_SCRIPTED_SRC
	monster/CCineAI.cpp
	monster/CCineMonster.cpp
	monster/CScriptedSentence.cpp
)

set(ENV_HDR
	env/CBeam.h
	env/CBloodSplat.h
	env/CGibShooter.h
	env/CLaser.h
	env/CLight.h
	env/CSoundEnt.h
	env/CSprayCan.h
	env/CSprite.h
	env/decals.h
	env/effects.h
	env/explode.h
	../engine/shake.h
)
set(ENV_SRC
	env/CAmbientGeneric.cpp
	env/CBeam.cpp
	env/CBlood.cpp
	env/CBloodSplat.cpp
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
	env/CGib.cpp
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
	env/CSprayCan.cpp
	env/CSprite.cpp
	env/CTestEffect.cpp
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
	triggers/CRuleEntity.h
	triggers/CTriggerMultiple.h
)
set(TRIGGER_SRC
	triggers/CAutoTrigger.cpp
	triggers/CBaseTrigger.cpp
	triggers/CChangeLevel.cpp
	triggers/CFrictionModifier.cpp
	triggers/CFireAndDie.cpp
	triggers/CGameCounter.cpp
	triggers/CGameEnd.cpp
	triggers/CGamePlayerHurt.cpp
	triggers/CGamePlayerTeam.cpp
	triggers/CGamePlayerZone.cpp
	triggers/CGameScore.cpp
	triggers/CGameTeamMaster.cpp
	triggers/CGameText.cpp
	triggers/CInfoIntermission.cpp
	triggers/CLadder.cpp
	triggers/CMultiManager.cpp
	triggers/CMultiSource.cpp
	triggers/CRenderFxManager.cpp
	triggers/CRevertSaved.cpp
	triggers/CRuleEntity.cpp
	triggers/CStripWeapons.cpp
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
	cbase.h
	ent_globals.h
	nodes.h
	CBasePlayer.h
	CBaseSpectator.h
	scripted.h
	scriptevent.h
	../engine/progs.h
	../engine/progdefs.h
	../engine/edict.h
	../engine/customentity.h
	../common/const.h
)
set(ENTITY_SRC
	cbase.cpp
	CBaseEntity.cpp
	CBaseDelay.cpp
	CBaseDMStart.cpp
	CBasePlayer.cpp
	CBaseSpectator.cpp
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
	weapon/CShockBeam.h
	weapon/CSpore.h
	weapon/weapons.h
)
set(WEAPON_SRC
	weapon/CBasePlayerAmmo.cpp
	weapon/CBasePlayerItem.cpp
	weapon/CBasePlayerWeapon.cpp
	weapon/CCrossbow.cpp
	weapon/CCrowbar.cpp
	weapon/CEgon.cpp
	weapon/CGauss.cpp
	weapon/CGrenade.cpp
	weapon/CHandGrenade.cpp	
	weapon/CHgun.cpp
	weapon/CMortar.cpp
	weapon/CMP5.cpp
	weapon/CPython.cpp
	weapon/CRpg.cpp
	weapon/CSatchel.cpp
	weapon/CShockBeam.cpp
	weapon/CShotgun.cpp
	weapon/CSpore.cpp
	weapon/CSqueakGrenade.cpp
	weapon/CTripmine.cpp
	weapon/CWeaponBox.cpp
	weapon/weapons.cpp
	wpn_shared/hl_wpn_glock.cpp
	weapon/combat.cpp
)

set(UTIL_HDR
	plane.h
	player_util.h
	util.h
	vector.h
)
set(UTIL_SRC
	plane.cpp
	player_util.cpp
	util.cpp
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_math.cpp
	../pm_shared/pm_shared.cpp
)

set(GAME_HDR
	animation.h
	client.h
	../engine/custom.h
	../engine/eiface.h
	gamerules.h
	saverestore.h
	skill.h
	teamplay_gamerules.h
	user_messages.h
)
set(GAME_SRC
	animation.cpp
	client.cpp
	client_commands.cpp
	game.cpp
	gamerules.cpp
	globals.cpp
	h_export.cpp
	multiplay_gamerules.cpp
	singleplay_gamerules.cpp
	skill.cpp
	sound.cpp
	teamplay_gamerules.cpp
	user_messages.cpp
	../game_shared/voice_gamemgr.cpp
)

set(ALL_SRC
	${MONSTER_SRC} ${MONSTER_AI_SRC} ${MONSTER_SCRIPTED_SRC} ${MONSTER_HDR}
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

include_directories(../common)
include_directories(../engine)
include_directories(../game_shared)
include_directories(../pm_shared)
include_directories(../public)
include_directories(env)
include_directories(func)
include_directories(triggers)
include_directories(monster)
include_directories(item)
include_directories(path)
include_directories(weapon)
include_directories(.)

add_library(${SERVER_DLL_NAME} SHARED ${ALL_SRC})

target_include_directories(${SERVER_DLL_NAME} PRIVATE monster)

target_compile_definitions(${SERVER_DLL_NAME} PRIVATE -DQUIVER -DVOXEL -DQUAKE2 -DVALVE_DLL -DCLIENT_WEAPONS)

add_custom_command(TARGET ${SERVER_DLL_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${SERVER_DLL_NAME}> ${ENGINE_DIR}/${GAMEDIR_NAME}/dlls/
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/${GAMEDIR_NAME}/dlls
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${SERVER_DLL_NAME}> ${PROJECT_SOURCE_DIR}/${GAMEDIR_NAME}/dlls/
)

if(MSVC)
	if (USE_AUTOHOTKEY_SCRIPT)
		add_custom_command(TARGET ${SERVER_DLL_NAME} POST_BUILD
			COMMAND start ${CMAKE_CURRENT_SOURCE_DIR}/../resize_console.ahk
		)
	endif()


	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:${CMAKE_CURRENT_SOURCE_DIR}/hl.def") 
	
	set_target_properties(${SERVER_DLL_NAME} PROPERTIES
		VS_DEBUGGER_COMMAND "${ENGINE_DIR}/${ENGINE_EXE}"
		VS_DEBUGGER_WORKING_DIRECTORY "${ENGINE_DIR}"
		VS_DEBUGGER_COMMAND_ARGUMENTS "${GAME_ARGS}"
	)
	
	
	source_group("Header Files\\Entities" FILES ${ENTITY_HDR})
	source_group("Header Files\\Entities\\env" FILES ${ENV_HDR})
	source_group("Header Files\\Entities\\func" FILES ${FUNC_HDR})
	source_group("Header Files\\Entities\\item" FILES ${ITEM_HDR})
	source_group("Header Files\\Entities\\monster" FILES ${MONSTER_HDR})
	source_group("Header Files\\Entities\\path" FILES ${PATH_HDR})
	source_group("Header Files\\Entities\\trigger" FILES ${TRIGGER_HDR})
	source_group("Header Files\\Entities\\weapon" FILES ${WEAPON_HDR})
	source_group("Header Files\\Game" FILES ${GAME_HDR})
	source_group("Header Files\\Util" FILES ${UTIL_HDR})
	
	source_group("Source Files\\Entities" FILES ${ENTITY_SRC})
	source_group("Source Files\\Entities\\env" FILES ${ENV_SRC})
	source_group("Source Files\\Entities\\func" FILES ${FUNC_SRC})
	source_group("Source Files\\Entities\\item" FILES ${ITEM_SRC})
	source_group("Source Files\\Entities\\monster" FILES ${MONSTER_SRC})
	source_group("Source Files\\Entities\\monster\\ai" FILES ${MONSTER_AI_SRC})
	source_group("Source Files\\Entities\\monster\\scripted" FILES ${MONSTER_SCRIPTED_SRC})
	source_group("Source Files\\Entities\\path" FILES ${PATH_SRC})
	source_group("Source Files\\Entities\\trigger" FILES ${TRIGGER_SRC})
	source_group("Source Files\\Entities\\weapon" FILES ${WEAPON_SRC})
	source_group("Source Files\\Util" FILES ${UTIL_SRC})
	source_group("Source Files\\Game" FILES ${GAME_SRC})

else()
	message(FATAL_ERROR "TODO: Linux/mac support")
endif()
