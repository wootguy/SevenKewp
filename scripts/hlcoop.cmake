cmake_minimum_required(VERSION 3.6)

set(MISC_HDR
	${MOD_DIR}/engine/studio.h
	${MOD_DIR}/common/Platform.h
	${MOD_DIR}/dlls/cdll_dll.h
	${MOD_DIR}/dlls/enginecallback.h
	${MOD_DIR}/dlls/extdll.h
)

set(MONSTER_HDR
	${MOD_DIR}/dlls/monster/activity.h
	${MOD_DIR}/dlls/monster/activitymap.h
	${MOD_DIR}/dlls/monster/bodyque.h
	${MOD_DIR}/dlls/monster/CAGrunt.h
	${MOD_DIR}/dlls/monster/CBaseGrunt.h
	${MOD_DIR}/dlls/monster/CBaseGruntOp4.h
	${MOD_DIR}/dlls/monster/CBaseMonster.h
	${MOD_DIR}/dlls/monster/CBaseTurret.h
	${MOD_DIR}/dlls/monster/CFlyingMonster.h
	${MOD_DIR}/dlls/monster/CGargantua.h
	${MOD_DIR}/dlls/monster/CHornet.h
	${MOD_DIR}/dlls/monster/CMonsterMaker.h
	${MOD_DIR}/dlls/monster/CTalkSquadMonster.h
	${MOD_DIR}/dlls/monster/defaultai.h
	${MOD_DIR}/dlls/monster/monsterevent.h
	${MOD_DIR}/dlls/monster/monsters.h
	${MOD_DIR}/dlls/monster/schedule.h
)

set(ENV_HDR
	${MOD_DIR}/dlls/env/CAmbientGeneric.h
	${MOD_DIR}/dlls/env/CBeam.h
	${MOD_DIR}/dlls/env/CBloodSplat.h
	${MOD_DIR}/dlls/env/CGibShooter.h
	${MOD_DIR}/dlls/env/CLaser.h
	${MOD_DIR}/dlls/env/CLight.h
	${MOD_DIR}/dlls/env/CSoundEnt.h
	${MOD_DIR}/dlls/env/CSprayCan.h
	${MOD_DIR}/dlls/env/CSprite.h
	${MOD_DIR}/dlls/env/decals.h
	${MOD_DIR}/dlls/env/effects.h
	${MOD_DIR}/dlls/env/explode.h
	${MOD_DIR}/engine/shake.h
)

set(ITEM_HDR
	${MOD_DIR}/dlls/item/CItem.h
)

set(TRIGGER_HDR
	${MOD_DIR}/dlls/triggers/CBaseLogic.h
	${MOD_DIR}/dlls/triggers/CBaseTrigger.h
	${MOD_DIR}/dlls/triggers/CGamePlayerEquip.h
	${MOD_DIR}/dlls/triggers/CFireAndDie.h
	${MOD_DIR}/dlls/triggers/CRuleEntity.h
	${MOD_DIR}/dlls/triggers/CTriggerMultiple.h
)

set(FUNC_HDR
	${MOD_DIR}/dlls/func/CBaseButton.h
	${MOD_DIR}/dlls/func/CBasePlatTrain.h
	${MOD_DIR}/dlls/func/CBreakable.h
	${MOD_DIR}/dlls/func/CBaseDoor.h
	${MOD_DIR}/dlls/func/CFuncPlat.h
	${MOD_DIR}/dlls/func/CFuncPlatRot.h
	${MOD_DIR}/dlls/func/CFuncTank.h
	${MOD_DIR}/dlls/func/CFuncTrackChange.h
	${MOD_DIR}/dlls/func/CFuncTrackTrain.h
	${MOD_DIR}/dlls/func/CFuncWall.h
	${MOD_DIR}/dlls/func/CPlatTrigger.h
)

set(ENTITY_HDR
	${MOD_DIR}/dlls/cbase.h
	${MOD_DIR}/dlls/ent_globals.h
	${MOD_DIR}/dlls/nodes.h
	${MOD_DIR}/dlls/CBasePlayer.h
	${MOD_DIR}/dlls/CBaseSpectator.h
	${MOD_DIR}/dlls/CKeyValue.h
	${MOD_DIR}/dlls/scripted.h
	${MOD_DIR}/dlls/scriptevent.h
	${MOD_DIR}/dlls/path/CPathCorner.h
	${MOD_DIR}/dlls/path/CPathTrack.h
	${MOD_DIR}/engine/progs.h
	${MOD_DIR}/engine/progdefs.h
	${MOD_DIR}/engine/edict.h
	${MOD_DIR}/engine/customentity.h
	${MOD_DIR}/common/const.h
)

set(WEAPON_HDR
	${MOD_DIR}/dlls/weapon/CShockBeam.h
	${MOD_DIR}/dlls/weapon/CGrapple.h
	${MOD_DIR}/dlls/weapon/CGrappleTip.h
	${MOD_DIR}/dlls/weapon/CSpore.h
	${MOD_DIR}/dlls/weapon/CBasePlayerItem.h
	${MOD_DIR}/dlls/weapon/CBasePlayerWeapon.h
	${MOD_DIR}/dlls/weapon/CWeaponBox.h
	${MOD_DIR}/dlls/weapon/CGlock.h
	${MOD_DIR}/dlls/weapon/CCrowbar.h
	${MOD_DIR}/dlls/weapon/CPython.h
	${MOD_DIR}/dlls/weapon/CMP5.h
	${MOD_DIR}/dlls/weapon/CCrossbow.h
	${MOD_DIR}/dlls/weapon/CShotgun.h
	${MOD_DIR}/dlls/weapon/CRpg.h
	${MOD_DIR}/dlls/weapon/CGauss.h
	${MOD_DIR}/dlls/weapon/CEgon.h
	${MOD_DIR}/dlls/weapon/CHgun.h
	${MOD_DIR}/dlls/weapon/CHandGrenade.h
	${MOD_DIR}/dlls/weapon/CSatchel.h
	${MOD_DIR}/dlls/weapon/CTripmine.h
	${MOD_DIR}/dlls/weapon/CSqueak.h
	${MOD_DIR}/dlls/weapon/weapons.h
)

set(AMMO_HDR
	${MOD_DIR}/dlls/weapon/CBasePlayerAmmo.h
	${MOD_DIR}/dlls/weapon/ammo.h
)

set(UTIL_HDR
	${MOD_DIR}/dlls/animation.h
	${MOD_DIR}/dlls/Bsp.h
	${MOD_DIR}/dlls/bsptypes.h
	${MOD_DIR}/dlls/bsplimits.h
	${MOD_DIR}/dlls/plane.h
	${MOD_DIR}/dlls/util.h
	${MOD_DIR}/dlls/mstream.h
	${MOD_DIR}/dlls/lagcomp.h
	${MOD_DIR}/dlls/vector.h
	${MOD_DIR}/game_shared/shared_util.h
)

set(GAME_HDR
	${MOD_DIR}/dlls/client.h
	${MOD_DIR}/engine/custom.h
	${MOD_DIR}/engine/eiface.h
	${MOD_DIR}/dlls/game.h
	${MOD_DIR}/dlls/gamerules.h
	${MOD_DIR}/dlls/saverestore.h
	${MOD_DIR}/dlls/skill.h
	${MOD_DIR}/dlls/teamplay_gamerules.h
	${MOD_DIR}/dlls/user_messages.h
	${MOD_DIR}/dlls/mod_api.h
)

set(HLCOOP_HEADERS
	${MONSTER_HDR}
	${ENTITY_HDR}
	${ENV_HDR}
	${ITEM_HDR}
	${PATH_HDR}
	${WEAPON_HDR}
	${AMMO_HDR}
	${GAME_HDR}
	${UTIL_HDR}
	${MISC_HDR}
)

# setup standard include directories and compile settings
function(hlcoop_setup_plugin OUTPUT_PATH)	
	include_directories(${MOD_DIR}/common)
	include_directories(${MOD_DIR}/engine)
	include_directories(${MOD_DIR}/game_shared)
	include_directories(${MOD_DIR}/pm_shared)
	include_directories(${MOD_DIR}/public)
	include_directories(${MOD_DIR}/dlls/env)
	include_directories(${MOD_DIR}/dlls/func)
	include_directories(${MOD_DIR}/dlls/triggers)
	include_directories(${MOD_DIR}/dlls/monster)
	include_directories(${MOD_DIR}/dlls/item)
	include_directories(${MOD_DIR}/dlls/path)
	include_directories(${MOD_DIR}/dlls/weapon)
	include_directories(${MOD_DIR}/dlls/)
	
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
		
		source_group("Header Files\\hlcoop\\Entities" FILES ${ENTITY_HDR})
		source_group("Header Files\\hlcoop\\Entities\\env" FILES ${ENV_HDR})
		source_group("Header Files\\hlcoop\\Entities\\func" FILES ${FUNC_HDR})
		source_group("Header Files\\hlcoop\\Entities\\item" FILES ${ITEM_HDR})
		source_group("Header Files\\hlcoop\\Entities\\monster" FILES ${MONSTER_HDR})
		source_group("Header Files\\hlcoop\\Entities\\path" FILES ${PATH_HDR})
		source_group("Header Files\\hlcoop\\Entities\\trigger" FILES ${TRIGGER_HDR})
		source_group("Header Files\\hlcoop\\Entities\\weapon" FILES ${WEAPON_HDR})
		source_group("Header Files\\hlcoop\\Entities\\weapon\\ammo" FILES ${AMMO_HDR})
		source_group("Header Files\\hlcoop\\Game" FILES ${GAME_HDR})
		source_group("Header Files\\hlcoop\\Util" FILES ${UTIL_HDR})
		source_group("Header Files\\hlcoop" FILES ${MISC_HDR})
	else()
		message(FATAL_ERROR "TODO: Mac support")
	endif()

	target_link_libraries(${PROJECT_NAME} PRIVATE sevenkewp)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DQUIVER -DVOXEL -DQUAKE2 -DVALVE_DLL -DCLIENT_WEAPONS -D_CRT_SECURE_NO_DEPRECATE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DPLUGIN_BUILD PLUGIN_NAME="${PROJECT_NAME}")
	
	if (BUILD_PLUGINS)
		set(PLUGIN_OUT_PATH "${CMAKE_SOURCE_DIR}/build/output/${OUTPUT_PATH}")
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
	endif()
	
endfunction()
