option(ASAN "Build with AddressSanitizer Enabled" OFF)

project(${SERVER_DLL_NAME})

set(MISC_HDR
	../engine/studio.h
	../common/Platform.h
	../game_shared/cdll_dll.h
	enginecallback.h
	extdll.h
)

set(MONSTER_HDR
	monster/activity.h
	monster/activitymap.h
	monster/bodyque.h
	monster/CAGrunt.h
	monster/CBaseGrunt.h
	monster/CBaseGruntOp4.h
	monster/CBaseMonster.h
	monster/CBaseTurret.h
	monster/CCineMonster.h
	monster/CFlyingMonster.h
	monster/CGargantua.h
	monster/CHornet.h
	monster/CMonsterMaker.h
	monster/CTalkSquadMonster.h
	monster/defaultai.h
	monster/monsterevent.h
	monster/monsters.h
	monster/schedule.h
	monster/nodes.h
)
set(MONSTER_SRC
	monster/CBaseMonster.cpp
	monster/CMonsterMaker.cpp
	monster/CTalkSquadMonster.cpp
	monster/defaultai.cpp
	monster/h_ai.cpp
	monster/CCineAI.cpp
	monster/CCineMonster.cpp
	monster/CScriptedSentence.cpp
	monster/nodes.cpp
)
set(MONSTER_MISC_SRC
	monster/CActAnimating.cpp
	monster/CBloater.cpp
	env/CCorpse.cpp
	monster/CDeadHEV.cpp
	monster/CFlockingFlyerFlock.cpp
	monster/CFurniture.cpp
	monster/CGenericMonster.cpp
	monster/CGMan.cpp
	monster/CCycler.cpp
	monster/CLeech.cpp
	monster/CRat.cpp
	monster/CRoach.cpp
	monster/CXenHair.cpp
	monster/CXenPLight.cpp
	monster/CXenSpore.cpp
	monster/CXenTree.cpp
	monster/tempmonster.cpp
)
set(MONSTER_MACHINE_SRC
	monster/CApache.cpp
	monster/CBaseTurret.cpp
	monster/CMiniTurret.cpp
	monster/COp4Mortar.cpp
	monster/COsprey.cpp
	monster/CRoboGrunt.cpp
	monster/CSentry.cpp
	monster/CTurret.cpp
)
set(MONSTER_HUMAN_SRC
	monster/CBarney.cpp
	monster/CBaseGrunt.cpp
	monster/CBaseGruntOp4.cpp
	monster/CBodyGuard.cpp
	monster/CHAssassin.cpp
	monster/CHGrunt.cpp
	monster/CHGruntOp4.cpp
	monster/CHGruntOp4Medic.cpp
	monster/CHGruntOp4Torch.cpp
	monster/CHWGrunt.cpp
	monster/CMassn.cpp
	monster/COtis.cpp
	monster/CPlayerMonster.cpp
	monster/CScientist.cpp
)
set(MONSTER_ALIEN_SRC
	monster/CAGrunt.cpp
	monster/CBabyGarg.cpp
	monster/CBabyVoltigore.cpp
	monster/CBarnacle.cpp
	monster/CBigMomma.cpp
	monster/CBullsquid.cpp
	monster/CChumtoad.cpp
	monster/CController.cpp
	monster/CFlyingMonster.cpp
	monster/CGargantua.cpp
	monster/CGonome.cpp
	monster/CHeadCrab.cpp
	monster/CHornet.cpp
	monster/CHoundeye.cpp
	monster/CIchthyosaur.cpp
	monster/CISlave.cpp
	monster/CKingpin.cpp
	monster/CNihilanth.cpp
	monster/CPitdrone.cpp
	monster/CShockRoach.cpp
	monster/CShockTrooper.cpp
	monster/CSqueakGrenade.cpp
	monster/CStukabat.cpp
	monster/CTentacle.cpp
	monster/CTor.cpp
	monster/CVoltigore.cpp
	monster/CZombie.cpp
)

set(ENV_HDR
	env/CAmbientGeneric.h
	env/CBeam.h
	env/CBloodSplat.h
	env/CGibShooter.h
	env/CLaser.h
	env/CLight.h
	env/CSoundEnt.h
	env/CSprayCan.h
	env/CSprite.h
	env/CGib.h
	env/decals.h
	env/effects.h
	env/explode.h
	env/te_effects.h
	../engine/shake.h
)
set(ENV_SRC
	env/CAmbientGeneric.cpp
	env/CBeam.cpp
	env/CBlood.cpp
	env/CBloodSplat.cpp
	env/CBubbling.cpp
	env/CDecal.cpp
	env/CEnvBeverage.cpp
	env/CEnvExplosion.cpp
	env/CEnvFunnel.cpp
	env/CEnvGlobal.cpp
	env/CEnvLight.cpp
	env/CEnvSentence.cpp
	env/CEnvShooter.cpp
	env/CEnvSound.cpp
	env/CEnvSpark.cpp
	env/CEnvWeather.cpp
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
	env/te_effects.cpp
	env/te_effect_ents.cpp
)

set(ITEM_HDR
	item/CItem.h
	item/CItemInventory.h
)
set(ITEM_SRC
	item/CAirtank.cpp
	item/CHealthKit.cpp
	item/CItem.cpp
	item/CItemAntidote.cpp
	item/CItemBattery.cpp
	item/CItemGeneric.cpp
	item/CItemInventory.cpp
	item/CItemLongJump.cpp
	item/CItemSecurity.cpp
	item/CItemSoda.cpp
	item/CItemSuit.cpp
	item/CWorldItem.cpp
)

set(TRIGGER_HDR
	triggers/CBaseLogic.h
	triggers/CBaseTrigger.h
	triggers/CGamePlayerEquip.h
	triggers/CFireAndDie.h
	triggers/CMultiSource.h
	triggers/CRuleEntity.h
	triggers/CTriggerMultiple.h
	triggers/CTriggerScript.h
	triggers/CTriggerVote.h
)
set(TRIGGER_SRC
	triggers/CBaseLogic.cpp
	triggers/CBaseTrigger.cpp
	triggers/CFireAndDie.cpp
	triggers/CRuleEntity.cpp
)
set(TRIGGER_OBSOLETE_SRC
	triggers/CRevertSaved.cpp
	triggers/CTriggerEndSection.cpp
	triggers/CTriggerSave.cpp
	triggers/CTriggerVolume.cpp
)
set(TRIGGER_POINT_SRC
	triggers/CAutoTrigger.cpp
	triggers/CGameCounter.cpp
	triggers/CGameEnd.cpp
	triggers/CGamePlayerCounter.cpp
	triggers/CGamePlayerEquip.cpp
	triggers/CGamePlayerHurt.cpp
	triggers/CGameScore.cpp
	triggers/CGameText.cpp
	triggers/CGlobalLightControl.cpp
	triggers/CMultiManager.cpp
	triggers/CMultiSource.cpp
	triggers/CRenderFxManager.cpp
	triggers/CStripWeapons.cpp
	triggers/CTriggerCamera.cpp
	triggers/CTargetCDAudio.cpp
	triggers/CTriggerChangeClass.cpp
	triggers/CTriggerChangeModel.cpp
	triggers/CTriggerChangeSky.cpp
	triggers/CTriggerChangeTarget.cpp
	triggers/CTriggerChangeValue.cpp
	triggers/CTriggerCondition.cpp
	triggers/CTriggerCreateEntity.cpp
	triggers/CTriggerEffect.cpp
	triggers/CTriggerEntityIterator.cpp
	triggers/CTriggerHurtRemote.cpp
	triggers/CTriggerRandom.cpp
	triggers/CTriggerRelay.cpp
	triggers/CTriggerRenamePlayer.cpp
	triggers/CTriggerRespawn.cpp
	triggers/CTriggerScript.cpp
	triggers/CTriggerSetCvar.cpp
	triggers/CTriggerSetOrigin.cpp
	triggers/CTriggerVote.cpp
)
set(TRIGGER_BRUSHED_SRC
	triggers/CChangeLevel.cpp
	triggers/CFrictionModifier.cpp
	triggers/CGamePlayerZone.cpp
	triggers/CPlayerRespawnZone.cpp
	triggers/CTriggerCDAudio.cpp
	triggers/CTriggerCounter.cpp
	triggers/CTriggerGravity.cpp
	triggers/CTriggerHurt.cpp
	triggers/CTriggerMonsterJump.cpp
	triggers/CTriggerMultiple.cpp
	triggers/CTriggerOnce.cpp
	triggers/CTriggerPush.cpp
	triggers/CTriggerTeleport.cpp
)

set(FUNC_HDR
	func/CBaseButton.h
	func/CBasePlatTrain.h
	func/CBreakable.h
	func/CBaseDoor.h
	func/CFuncConveyor.h
	func/CFuncPlat.h
	func/CFuncPlatRot.h
	func/CFuncTank.h
	func/CFuncTrackChange.h
	func/CFuncTrackTrain.h
	func/CFuncVehicle.h
	func/CFuncWall.h
	func/CPlatTrigger.h
)
set(FUNC_SRC
	func/CBaseButton.cpp
	func/CBaseDoor.cpp
	func/CBasePlatTrain.cpp
	func/CBreakable.cpp
	func/CButtonTarget.cpp
	func/CFuncClip.cpp
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
	func/CFuncTankRocket.cpp
	func/CFuncTrackChange.cpp
	func/CFuncTrackTrain.cpp
	func/CFuncTrain.cpp
	func/CFuncTrainControls.cpp
	func/CFuncVehicle.cpp
	func/CFuncWall.cpp
	func/CFuncWallToggle.cpp
	func/CGunTarget.cpp
	func/CLadder.cpp
	func/CMomentaryDoor.cpp
	func/CMomentaryRotButton.cpp
	func/CPlatTrigger.cpp
	func/CPushable.cpp
	func/CRecharge.cpp
	func/CRotButton.cpp
	func/CRotDoor.cpp
)

set(ENTITY_HDR
	CBaseAnimating.h
	CBaseDelay.h
	CBaseEntity.h
	CPointEntity.h
	CBaseToggle.h
	CWorld.h
	EHandle.h
	monster/scriptevent.h
	path/CPathCorner.h
	path/CPathTrack.h
	../engine/progs.h
	../engine/progdefs.h
	../engine/edict.h
	../engine/customentity.h
	../common/const.h
)
set(ENTITY_SRC
	CBaseAnimating.cpp
	CBaseEntity.cpp
	CBaseDelay.cpp
	CBaseToggle.cpp
	CGlobalState.cpp
	CNullEntity.cpp
	CPointEntity.cpp
	CWorld.cpp
	EHandle.cpp
	path/CPathCorner.cpp
	path/CPathTrack.cpp
)

set(WEAPON_HDR
	weapon/CShockBeam.h
	weapon/CGrapple.h
	weapon/CGrappleTip.h
	weapon/CSpore.h
	weapon/CBasePlayerItem.h
	weapon/CBasePlayerWeapon.h
	weapon/CWeaponBox.h
	weapon/CGlock.h
	weapon/CCrowbar.h
	weapon/CPython.h
	weapon/CMedkit.h
	weapon/CMP5.h
	weapon/CCrossbow.h
	weapon/CShotgun.h
	weapon/CRpg.h
	weapon/CGauss.h
	weapon/CEgon.h
	weapon/CHgun.h
	weapon/CHandGrenade.h
	weapon/CSatchel.h
	weapon/CTripmine.h
	weapon/CSqueak.h
	weapon/CDisplacer.h
	weapon/CDisplacerBall.h
	weapon/CPipewrench.h
	weapon/CShockRifle.h
	weapon/CSporeLauncher.h
	weapon/CWeaponInventory.h
	weapon/CKnife.h
	weapon/CM249.h
	weapon/CWeaponCustom.h
	weapon/weapons.h
)
set(WEAPON_SRC
	weapon/CBasePlayerItem.cpp
	weapon/CBasePlayerWeapon.cpp
	weapon/CCrossbow.cpp
	weapon/CCrowbar.cpp
	weapon/CEgon.cpp
	weapon/CGauss.cpp
	weapon/CGrenade.cpp
	weapon/CGrapple.cpp
	weapon/CGrappleTip.cpp
	weapon/CHandGrenade.cpp
	weapon/CHgun.cpp
	weapon/CMortar.cpp
	weapon/CMP5.cpp
	weapon/CPython.cpp
	weapon/CMedkit.cpp
	weapon/CRpg.cpp
	weapon/CSatchel.cpp
	weapon/CShockBeam.cpp
	weapon/CShotgun.cpp
	weapon/CSpore.cpp
	weapon/CSqueak.cpp
	weapon/CTripmine.cpp
	weapon/CWeaponBox.cpp
	weapon/CDisplacer.cpp
	weapon/CDisplacerBall.cpp
	weapon/CPipewrench.cpp
	weapon/CShockRifle.cpp
	weapon/CSporeLauncher.cpp
	weapon/weapons.cpp
	weapon/CGlock.cpp
	weapon/CWeaponInventory.cpp
	weapon/CKnife.cpp
	weapon/CM249.cpp
	weapon/CWeaponCustom.cpp
	weapon/combat.cpp
)
set(AMMO_HDR
	weapon/CBasePlayerAmmo.h
	weapon/ammo.h
)
set(AMMO_SRC
	weapon/CBasePlayerAmmo.cpp
	weapon/CAmmo556.cpp
	weapon/CCrossbowAmmo.cpp
	weapon/CGaussAmmo.cpp
	weapon/CGlockAmmo.cpp
	weapon/CMedkitAmmo.cpp
	weapon/CMP5AmmoBox.cpp
	weapon/CMP5AmmoClip.cpp
	weapon/CMP5AmmoGrenade.cpp
	weapon/CPythonAmmo.cpp
	weapon/CRpgAmmo.cpp
	weapon/CShotgunAmmo.cpp
	weapon/CSporeAmmo.cpp
)

set(UTIL_HDR
	util/animation.h
	util/Bsp.h
	util/bsptypes.h
	util/bsplimits.h
	util/CKeyValue.h
	util/CommandArgs.h
	util/debug.h
	util/eng_wrappers.h
	util/plane.h
	util/saverestore.h
	util/util.h
	util/mstream.h
	util/lagcomp.h
	util/vector.h
	util/Scheduler.h
	util/sentences.h
	util/TextMenu.h
	util/ThreadSafeInt.h
	util/ThreadSafeQueue.h
	util/wav.h
	util/HashMap.h
	util/StringPool.h
	util/md5.h
	../game_shared/shared_util.h
)
set(UTIL_SRC
	util/animation.cpp
	util/Bsp.cpp
	util/bsptypes.cpp
	util/CommandArgs.cpp
	util/debug.cpp
	util/eng_wrappers.cpp
	util/plane.cpp
	util/sound.cpp
	util/util.cpp
	util/mstream.cpp
	util/lagcomp.cpp
	util/saverestore.cpp
	util/Scheduler.cpp
	util/sentences.cpp
	util/subs.cpp
	util/TextMenu.cpp
	util/ThreadSafeInt.cpp
	util/ThreadSafeQueue.cpp
	util/wav.cpp
	util/HashMap.cpp
	util/StringPool.cpp
	util/md5.cpp
	../game_shared/shared_util.cpp
)
set(PLAYER_HDR
	../pm_shared/pm_debug.h
	../pm_shared/pm_defs.h
	../pm_shared/pm_info.h
	../pm_shared/pm_materials.h
	../pm_shared/pm_movevars.h
	../pm_shared/pm_shared.h
	player/CBasePlayer.h
	player/CBaseSpectator.h
)
set(PLAYER_SRC
	../pm_shared/pm_shared.cpp
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_math.cpp
	player/CBaseDMStart.cpp
	player/CBasePlayer.cpp
	player/CBaseSpectator.cpp
)
set(HOOKS_HDR
	../engine/custom.h
	../engine/eiface.h
	hooks/hlds_hooks.h
	hooks/PluginHooks.h
	hooks/PluginManager.h
	hooks/rehlds.h
)
set(HOOKS_SRC
	hooks/client_commands.cpp
	hooks/h_export.cpp
	hooks/hlds_hooks.cpp
	hooks/PluginManager.cpp
	hooks/rehlds.cpp
	hooks/rehlds_hooks.cpp
	../common/rehlds_interface.cpp
)

set(NET_HDR
	net/Socket.h
	net/IPV4.h
	net/Packet.h
)
set(NET_SRC
	net/IPV4.cpp
	net/Packet.cpp
)

if (UNIX)
	set(NET_SRC ${NET_SRC} net/network_unix.cpp net/Socket_unix.cpp)
else()
	set(NET_SRC ${NET_SRC} net/network_win.cpp net/Socket_win.cpp)
endif()

set(GAME_HDR
	game/game.h
	game/gamerules.h
	game/skill.h
	game/teamplay_gamerules.h
	game/user_messages.h
)
set(GAME_SRC
	game/game.cpp
	game/gamerules.cpp
	game/globals.cpp
	game/multiplay_gamerules.cpp
	game/singleplay_gamerules.cpp
	game/skill.cpp
	game/teamplay_gamerules.cpp
	../game_shared/voice_gamemgr.cpp
	game/user_messages.cpp
)

set(ALL_TRIGGER_MISC_SRC ${TRIGGER_SRC} ${TRIGGER_OBSOLETE_SRC})
set(ALL_TRIGGER_POINT_SRC ${TRIGGER_POINT_SRC})
set(ALL_TRIGGER_BRUSH_SRC ${TRIGGER_BRUSHED_SRC})
set(ALL_ENT_MISC_SRC ${ENTITY_SRC} ${PATH_SRC})

set(ENT_SRC
	${MONSTER_SRC}
	${MONSTER_ALIEN_SRC}
	${MONSTER_HUMAN_SRC}
	${MONSTER_MACHINE_SRC}
	${MONSTER_MISC_SRC}
	${ALL_TRIGGER_POINT_SRC}
	${ALL_TRIGGER_BRUSH_SRC}
	${ALL_TRIGGER_MISC_SRC}
	${WEAPON_SRC}
	${AMMO_SRC}
	${ENV_SRC}
	${ITEM_SRC}
	${FUNC_SRC}
	${ALL_ENT_MISC_SRC}
)

set(ENT_HDR
	${MONSTER_HDR}
	${ENTITY_HDR}
	${ENV_HDR}
	${ITEM_HDR}
	${PATH_HDR}
	${TRIGGER_HDR}
	${WEAPON_HDR}
	${AMMO_HDR}
)

# combine all entity source files into one monolothic file for a huge compile time speedup
function(write_ent_source_file IN_FILES_STRING OUT_NAME)
    # Split the input string into a list
    string(REPLACE ";" ";" IN_FILES_STRING "${IN_FILES_STRING}")
    string(REGEX REPLACE ";" ";" IN_SRCS_EXT "${IN_FILES_STRING}")
    list(LENGTH IN_SRCS_EXT list_count)
    math(EXPR list_max_index ${list_count}-1)
    foreach(i RANGE ${list_max_index})
        list(GET IN_SRCS_EXT ${i} x)
        
        set(OUT_FILE_INCLUDES "${OUT_FILE_INCLUDES} #include \"${x}\"\n")
    endforeach(i)
    
	set(OUT_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${OUT_NAME}")
	
	# Read the existing contents of the file if it exists
    if(EXISTS ${OUT_FILE_PATH})
        file(READ ${OUT_FILE_PATH} CURRENT_CONTENT)
    else()
        set(CURRENT_CONTENT "")
    endif()
	
    # Only write if the new content is different
    if(NOT "${CURRENT_CONTENT}" STREQUAL "${OUT_FILE_INCLUDES}")
        file(WRITE ${OUT_FILE_PATH} ${OUT_FILE_INCLUDES})
    endif()
endfunction()

write_ent_source_file("${MONSTER_SRC}" monsters_base.cpp)
write_ent_source_file("${MONSTER_ALIEN_SRC}" monsters_aliens.cpp)
write_ent_source_file("${MONSTER_HUMAN_SRC}" monsters_humans.cpp)
write_ent_source_file("${MONSTER_MACHINE_SRC}" monsters_machines.cpp)
write_ent_source_file("${MONSTER_MISC_SRC}" monsters_misc.cpp)
write_ent_source_file("${WEAPON_SRC}" all_weapons.cpp)
write_ent_source_file("${AMMO_SRC}" all_ammo.cpp)
write_ent_source_file("${ALL_TRIGGER_POINT_SRC}" triggers_point.cpp)
write_ent_source_file("${ALL_TRIGGER_BRUSH_SRC}" triggers_brush.cpp)
write_ent_source_file("${ALL_TRIGGER_MISC_SRC}" triggers_misc.cpp)
write_ent_source_file("${ALL_ENT_MISC_SRC}" all_ents_misc.cpp)
write_ent_source_file("${ENV_SRC}" all_ents_env.cpp)
write_ent_source_file("${ITEM_SRC}" all_ents_item.cpp)
write_ent_source_file("${FUNC_SRC}" all_ents_func.cpp)

set(GENERATED_SRCS 
	${CMAKE_CURRENT_BINARY_DIR}/monsters_base.cpp
	${CMAKE_CURRENT_BINARY_DIR}/monsters_aliens.cpp
	${CMAKE_CURRENT_BINARY_DIR}/monsters_humans.cpp
	${CMAKE_CURRENT_BINARY_DIR}/monsters_machines.cpp
	${CMAKE_CURRENT_BINARY_DIR}/monsters_misc.cpp
	${CMAKE_CURRENT_BINARY_DIR}/all_weapons.cpp
	${CMAKE_CURRENT_BINARY_DIR}/all_ammo.cpp
	${CMAKE_CURRENT_BINARY_DIR}/triggers_point.cpp
	${CMAKE_CURRENT_BINARY_DIR}/triggers_brush.cpp
	${CMAKE_CURRENT_BINARY_DIR}/triggers_misc.cpp
	${CMAKE_CURRENT_BINARY_DIR}/all_ents_misc.cpp
	${CMAKE_CURRENT_BINARY_DIR}/all_ents_env.cpp
	${CMAKE_CURRENT_BINARY_DIR}/all_ents_item.cpp
	${CMAKE_CURRENT_BINARY_DIR}/all_ents_func.cpp
)

set(ALL_SRC
	${GENERATED_SRCS}
	${ENT_SRC} ${ENT_HDR}
	${UTIL_SRC} ${UTIL_HDR}
	${NET_SRC} ${NET_HDR}
	${GAME_SRC} ${GAME_HDR}
	${PLAYER_SRC} ${PLAYER_HDR}
	${HOOKS_SRC} ${HOOKS_HDR}
	${MISC_HDR}
	env/fog_colors.c
)

# don't compile individual entity files
set_source_files_properties(${ENT_SRC} PROPERTIES HEADER_FILE_ONLY TRUE)

add_library(${SERVER_DLL_NAME} SHARED ${ALL_SRC})

target_include_directories(${SERVER_DLL_NAME} PRIVATE monster)

target_compile_definitions(${SERVER_DLL_NAME} PRIVATE -DQUIVER -DVOXEL -DQUAKE2 -DVALVE_DLL -DCLIENT_WEAPONS -D_CRT_SECURE_NO_DEPRECATE)

if (INCREASED_SERVER_LIMITS)
	target_compile_definitions(${SERVER_DLL_NAME} PRIVATE -DINCREASED_SERVER_LIMITS)
endif()

if(UNIX)
	set(OPT_FLAG "-O2")
	if (ASAN)
		set(ASAN_CFLAGS "-fsanitize=address -fno-omit-frame-pointer")
		set(ASAN_LFLAGS "-fsanitize=address")
		set(OPT_FLAG "-Og")
	endif()

	set_target_properties(${SERVER_DLL_NAME} PROPERTIES
		PREFIX ""
		COMPILE_FLAGS "-m32 -g -fno-strict-aliasing -Wall -Wextra -Wpedantic -Werror=return-type -Wno-invalid-offsetof -Wno-class-memaccess -Wno-unused-parameter -fno-omit-frame-pointer ${OPT_FLAG} ${ASAN_CFLAGS}"
		LINK_FLAGS "-m32 -g ${ASAN_LFLAGS}"
	)
	
elseif(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /W4 /we4715")
	
	set_target_properties(${SERVER_DLL_NAME} PROPERTIES
		VS_DEBUGGER_COMMAND "${SERVER_WORK_DIR}/${SERVER_EXE}"
		VS_DEBUGGER_WORKING_DIRECTORY "${SERVER_WORK_DIR}"
		VS_DEBUGGER_COMMAND_ARGUMENTS "${SERVER_ARGS}"
	)
	
	source_group("Header Files\\Entities" FILES ${ENTITY_HDR})
	source_group("Header Files\\Entities\\env" FILES ${ENV_HDR})
	source_group("Header Files\\Entities\\func" FILES ${FUNC_HDR})
	source_group("Header Files\\Entities\\item" FILES ${ITEM_HDR})
	source_group("Header Files\\Entities\\monster" FILES ${MONSTER_HDR})
	source_group("Header Files\\Entities\\path" FILES ${PATH_HDR})
	source_group("Header Files\\Entities\\player" FILES ${PLAYER_HDR})
	source_group("Header Files\\Entities\\trigger" FILES ${TRIGGER_HDR})
	source_group("Header Files\\Entities\\weapon" FILES ${WEAPON_HDR})
	source_group("Header Files\\Entities\\weapon\\ammo" FILES ${AMMO_HDR})
	source_group("Header Files\\Game" FILES ${GAME_HDR})
	source_group("Header Files\\Util" FILES ${UTIL_HDR})
	source_group("Header Files\\Network" FILES ${NET_HDR})
	source_group("Header Files\\Hooks" FILES ${HOOKS_HDR})
	
	source_group("Source Files\\Entities" FILES ${ENTITY_SRC})
	source_group("Source Files\\Generated" FILES  ${GENERATED_SRCS})
	source_group("Source Files\\Entities\\env" FILES ${ENV_SRC})
	source_group("Source Files\\Entities\\func" FILES ${FUNC_SRC})
	source_group("Source Files\\Entities\\item" FILES ${ITEM_SRC})
	source_group("Source Files\\Entities\\monster" FILES ${MONSTER_SRC})
	source_group("Source Files\\Entities\\monster\\aliens" FILES ${MONSTER_ALIEN_SRC})
	source_group("Source Files\\Entities\\monster\\humans" FILES ${MONSTER_HUMAN_SRC})
	source_group("Source Files\\Entities\\monster\\machines" FILES ${MONSTER_MACHINE_SRC})
	source_group("Source Files\\Entities\\monster\\misc" FILES ${MONSTER_MISC_SRC})
	source_group("Source Files\\Entities\\path" FILES ${PATH_SRC})
	source_group("Source Files\\Entities\\player" FILES ${PLAYER_SRC})
	source_group("Source Files\\Entities\\trigger" FILES ${TRIGGER_SRC})
	source_group("Source Files\\Entities\\trigger\\point" FILES ${TRIGGER_POINT_SRC})
	source_group("Source Files\\Entities\\trigger\\brush" FILES ${TRIGGER_BRUSHED_SRC})
	source_group("Source Files\\Entities\\trigger\\obsolete" FILES ${TRIGGER_OBSOLETE_SRC})
	source_group("Source Files\\Entities\\weapon" FILES ${WEAPON_SRC})
	source_group("Source Files\\Entities\\weapon\\ammo" FILES ${AMMO_SRC})
	source_group("Source Files\\Util" FILES ${UTIL_SRC})
	source_group("Source Files\\Network" FILES ${NET_SRC})
	source_group("Source Files\\Game" FILES ${GAME_SRC})
	source_group("Source Files\\Hooks" FILES ${HOOKS_SRC})
	
	# WinSock libraries for network utils
	target_link_libraries(${PROJECT_NAME} PRIVATE WS2_32 IPHLPAPI)

else()
	message(FATAL_ERROR "TODO: Mac support")
endif()

if (SETUP_IDE)
	set(MOD_OUTPUT_PATH "${SERVER_WORK_DIR}/valve/dlls")
else()
	set(MOD_OUTPUT_PATH "${CMAKE_BINARY_DIR}/output/dlls")
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