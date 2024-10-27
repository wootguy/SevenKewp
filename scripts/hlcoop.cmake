cmake_minimum_required(VERSION 3.2)

set(ROOT_FOLDER ${CMAKE_SOURCE_DIR}/SevenKewp)

set(MISC_HDR
	${ROOT_FOLDER}/engine/studio.h
	${ROOT_FOLDER}/common/Platform.h
	${ROOT_FOLDER}/dlls/cdll_dll.h
	${ROOT_FOLDER}/dlls/enginecallback.h
	${ROOT_FOLDER}/dlls/extdll.h
)

set(MONSTER_HDR
	${ROOT_FOLDER}/dlls/monster/activity.h
	${ROOT_FOLDER}/dlls/monster/activitymap.h
	${ROOT_FOLDER}/dlls/monster/bodyque.h
	${ROOT_FOLDER}/dlls/monster/CAGrunt.h
	${ROOT_FOLDER}/dlls/monster/CBaseGrunt.h
	${ROOT_FOLDER}/dlls/monster/CBaseGruntOp4.h
	${ROOT_FOLDER}/dlls/monster/CBaseMonster.h
	${ROOT_FOLDER}/dlls/monster/CBaseTurret.h
	${ROOT_FOLDER}/dlls/monster/CFlyingMonster.h
	${ROOT_FOLDER}/dlls/monster/CGargantua.h
	${ROOT_FOLDER}/dlls/monster/CHornet.h
	${ROOT_FOLDER}/dlls/monster/CMonsterMaker.h
	${ROOT_FOLDER}/dlls/monster/CTalkSquadMonster.h
	${ROOT_FOLDER}/dlls/monster/defaultai.h
	${ROOT_FOLDER}/dlls/monster/monsterevent.h
	${ROOT_FOLDER}/dlls/monster/monsters.h
	${ROOT_FOLDER}/dlls/monster/schedule.h
)

set(ENV_HDR
	${ROOT_FOLDER}/dlls/env/CAmbientGeneric.h
	${ROOT_FOLDER}/dlls/env/CBeam.h
	${ROOT_FOLDER}/dlls/env/CBloodSplat.h
	${ROOT_FOLDER}/dlls/env/CGibShooter.h
	${ROOT_FOLDER}/dlls/env/CLaser.h
	${ROOT_FOLDER}/dlls/env/CLight.h
	${ROOT_FOLDER}/dlls/env/CSoundEnt.h
	${ROOT_FOLDER}/dlls/env/CSprayCan.h
	${ROOT_FOLDER}/dlls/env/CSprite.h
	${ROOT_FOLDER}/dlls/env/decals.h
	${ROOT_FOLDER}/dlls/env/effects.h
	${ROOT_FOLDER}/dlls/env/explode.h
	${ROOT_FOLDER}/engine/shake.h
)

set(ITEM_HDR
	${ROOT_FOLDER}/dlls/item/CItem.h
)

set(TRIGGER_HDR
	${ROOT_FOLDER}/dlls/triggers/CBaseLogic.h
	${ROOT_FOLDER}/dlls/triggers/CBaseTrigger.h
	${ROOT_FOLDER}/dlls/triggers/CGamePlayerEquip.h
	${ROOT_FOLDER}/dlls/triggers/CFireAndDie.h
	${ROOT_FOLDER}/dlls/triggers/CRuleEntity.h
	${ROOT_FOLDER}/dlls/triggers/CTriggerMultiple.h
)

set(FUNC_HDR
	${ROOT_FOLDER}/dlls/func/CBaseButton.h
	${ROOT_FOLDER}/dlls/func/CBasePlatTrain.h
	${ROOT_FOLDER}/dlls/func/CBreakable.h
	${ROOT_FOLDER}/dlls/func/CBaseDoor.h
	${ROOT_FOLDER}/dlls/func/CFuncPlat.h
	${ROOT_FOLDER}/dlls/func/CFuncPlatRot.h
	${ROOT_FOLDER}/dlls/func/CFuncTank.h
	${ROOT_FOLDER}/dlls/func/CFuncTrackChange.h
	${ROOT_FOLDER}/dlls/func/CFuncTrackTrain.h
	${ROOT_FOLDER}/dlls/func/CFuncWall.h
	${ROOT_FOLDER}/dlls/func/CPlatTrigger.h
)

set(ENTITY_HDR
	${ROOT_FOLDER}/dlls/cbase.h
	${ROOT_FOLDER}/dlls/ent_globals.h
	${ROOT_FOLDER}/dlls/nodes.h
	${ROOT_FOLDER}/dlls/CBasePlayer.h
	${ROOT_FOLDER}/dlls/CBaseSpectator.h
	${ROOT_FOLDER}/dlls/CKeyValue.h
	${ROOT_FOLDER}/dlls/scripted.h
	${ROOT_FOLDER}/dlls/scriptevent.h
	${ROOT_FOLDER}/dlls/path/CPathCorner.h
	${ROOT_FOLDER}/dlls/path/CPathTrack.h
	${ROOT_FOLDER}/engine/progs.h
	${ROOT_FOLDER}/engine/progdefs.h
	${ROOT_FOLDER}/engine/edict.h
	${ROOT_FOLDER}/engine/customentity.h
	${ROOT_FOLDER}/common/const.h
)

set(WEAPON_HDR
	${ROOT_FOLDER}/dlls/weapon/CShockBeam.h
	${ROOT_FOLDER}/dlls/weapon/CGrapple.h
	${ROOT_FOLDER}/dlls/weapon/CGrappleTip.h
	${ROOT_FOLDER}/dlls/weapon/CSpore.h
	${ROOT_FOLDER}/dlls/weapon/CBasePlayerItem.h
	${ROOT_FOLDER}/dlls/weapon/CBasePlayerWeapon.h
	${ROOT_FOLDER}/dlls/weapon/CWeaponBox.h
	${ROOT_FOLDER}/dlls/weapon/CGlock.h
	${ROOT_FOLDER}/dlls/weapon/CCrowbar.h
	${ROOT_FOLDER}/dlls/weapon/CPython.h
	${ROOT_FOLDER}/dlls/weapon/CMP5.h
	${ROOT_FOLDER}/dlls/weapon/CCrossbow.h
	${ROOT_FOLDER}/dlls/weapon/CShotgun.h
	${ROOT_FOLDER}/dlls/weapon/CRpg.h
	${ROOT_FOLDER}/dlls/weapon/CGauss.h
	${ROOT_FOLDER}/dlls/weapon/CEgon.h
	${ROOT_FOLDER}/dlls/weapon/CHgun.h
	${ROOT_FOLDER}/dlls/weapon/CHandGrenade.h
	${ROOT_FOLDER}/dlls/weapon/CSatchel.h
	${ROOT_FOLDER}/dlls/weapon/CTripmine.h
	${ROOT_FOLDER}/dlls/weapon/CSqueak.h
	${ROOT_FOLDER}/dlls/weapon/weapons.h
)

set(AMMO_HDR
	${ROOT_FOLDER}/dlls/weapon/CBasePlayerAmmo.h
	${ROOT_FOLDER}/dlls/weapon/ammo.h
)

set(UTIL_HDR
	${ROOT_FOLDER}/dlls/animation.h
	${ROOT_FOLDER}/dlls/Bsp.h
	${ROOT_FOLDER}/dlls/bsptypes.h
	${ROOT_FOLDER}/dlls/bsplimits.h
	${ROOT_FOLDER}/dlls/plane.h
	${ROOT_FOLDER}/dlls/util.h
	${ROOT_FOLDER}/dlls/mstream.h
	${ROOT_FOLDER}/dlls/lagcomp.h
	${ROOT_FOLDER}/dlls/vector.h
	${ROOT_FOLDER}/game_shared/shared_util.h
)

set(GAME_HDR
	${ROOT_FOLDER}/dlls/client.h
	${ROOT_FOLDER}/engine/custom.h
	${ROOT_FOLDER}/engine/eiface.h
	${ROOT_FOLDER}/dlls/game.h
	${ROOT_FOLDER}/dlls/gamerules.h
	${ROOT_FOLDER}/dlls/saverestore.h
	${ROOT_FOLDER}/dlls/skill.h
	${ROOT_FOLDER}/dlls/teamplay_gamerules.h
	${ROOT_FOLDER}/dlls/user_messages.h
	${ROOT_FOLDER}/dlls/mod_api.h
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
function(hlcoop_sdk_init)	
	include_directories(${ROOT_FOLDER}/common)
	include_directories(${ROOT_FOLDER}/engine)
	include_directories(${ROOT_FOLDER}/game_shared)
	include_directories(${ROOT_FOLDER}/pm_shared)
	include_directories(${ROOT_FOLDER}/public)
	include_directories(${ROOT_FOLDER}/dlls/env)
	include_directories(${ROOT_FOLDER}/dlls/func)
	include_directories(${ROOT_FOLDER}/dlls/triggers)
	include_directories(${ROOT_FOLDER}/dlls/monster)
	include_directories(${ROOT_FOLDER}/dlls/item)
	include_directories(${ROOT_FOLDER}/dlls/path)
	include_directories(${ROOT_FOLDER}/dlls/weapon)
	include_directories(${ROOT_FOLDER}/dlls/)
	
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
		
		target_link_libraries(${PROJECT_NAME} PRIVATE ${ROOT_FOLDER}/build/dlls/sevenkewp.so)
		
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

		target_link_libraries(${PROJECT_NAME} PRIVATE ${ROOT_FOLDER}/build/dlls/Release/sevenkewp.lib)
	else()
		message(FATAL_ERROR "TODO: Mac support")
	endif()

	target_compile_definitions(${PROJECT_NAME} PRIVATE -DQUIVER -DVOXEL -DQUAKE2 -DVALVE_DLL -DCLIENT_WEAPONS -D_CRT_SECURE_NO_DEPRECATE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -DPLUGIN_BUILD PLUGIN_NAME="${PROJECT_NAME}")
	
endfunction()
