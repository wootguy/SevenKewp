/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "env/CSoundEnt.h"
#include "effects.h"
#include "customentity.h"
#include "explode.h"
#include "COsprey.h"

LINK_ENTITY_TO_CLASS( monster_osprey, COsprey )
LINK_ENTITY_TO_CLASS( monster_blkop_osprey, COsprey )

TYPEDESCRIPTION	COsprey::m_SaveData[] = 
{
	DEFINE_FIELD( COsprey, m_hGoalEnt, FIELD_EHANDLE ),
	DEFINE_FIELD( COsprey, m_vel1, FIELD_VECTOR ),
	DEFINE_FIELD( COsprey, m_vel2, FIELD_VECTOR ),
	DEFINE_FIELD( COsprey, m_pos1, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( COsprey, m_pos2, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( COsprey, m_ang1, FIELD_VECTOR ),
	DEFINE_FIELD( COsprey, m_ang2, FIELD_VECTOR ),

	DEFINE_FIELD( COsprey, m_startTime, FIELD_TIME ),
	DEFINE_FIELD( COsprey, m_dTime, FIELD_FLOAT ),
	DEFINE_FIELD( COsprey, m_velocity, FIELD_VECTOR ),

	DEFINE_FIELD( COsprey, m_flRotortilt, FIELD_FLOAT ),

	DEFINE_FIELD( COsprey, m_flRightHealth, FIELD_FLOAT ),
	DEFINE_FIELD( COsprey, m_flLeftHealth, FIELD_FLOAT ),

	DEFINE_FIELD( COsprey, m_iUnits, FIELD_INTEGER ),
	DEFINE_ARRAY( COsprey, m_hGrunt, FIELD_EHANDLE, OSPREY_MAX_CARRY),
	DEFINE_ARRAY( COsprey, m_vecOrigin, FIELD_POSITION_VECTOR, OSPREY_MAX_CARRY),
	DEFINE_ARRAY( COsprey, m_hRepel, FIELD_EHANDLE, 4 ),

	// DEFINE_FIELD( COsprey, m_iSoundState, FIELD_INTEGER ),
	// DEFINE_FIELD( COsprey, m_iSpriteTexture, FIELD_INTEGER ),
	// DEFINE_FIELD( COsprey, m_iPitch, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE( COsprey, CBaseMonster )


void COsprey :: Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	float zofs = 134;

	InitModel();

	if (!pev->skin && FClassnameIs(pev, "monster_blkop_osprey"))
		pev->skin = 2;

	SetSize(Vector( -480, -480, -100 + zofs), Vector(480, 480, 64 + zofs));

	Vector offset = Vector(0, 0, -zofs); // so what you see in the editor matches the game
	UTIL_SetOrigin( pev, pev->origin + offset );

	pev->flags |= FL_MONSTER;
	pev->takedamage		= DAMAGE_YES;
	
	SetHealth();
	m_flRightHealth = pev->health / 2;
	m_flLeftHealth = pev->health / 2;

	m_flFieldOfView = 0; // 180 degrees

	pev->sequence = 0;
	ResetSequenceInfo( );
	pev->frame = RANDOM_LONG(0,0xFF);

	InitBoneControllers();

	SetThink( &COsprey::FindAllThink );
	SetUse( &COsprey::CommandUse );

	if (!(pev->spawnflags & SF_OSPREY_WAITFORTRIGGER))
	{
		pev->nextthink = gpGlobals->time + 1.0;
	}

	m_pos2 = pev->origin;
	m_ang2 = pev->angles;
	m_vel2 = pev->velocity;

	m_overrideGruntCount = V_min(OSPREY_MAX_CARRY, m_overrideGruntCount);
}

void COsprey::KeyValue(KeyValueData* pkvd) {
	if (FStrEq(pkvd->szKeyName, "num"))
	{
		m_overrideGruntCount = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "grunttype"))
	{
		m_overrideGruntType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseMonster::KeyValue(pkvd);
	}
}

void COsprey::Precache( void )
{
	CBaseMonster::Precache();

	bool isBlkOps = FClassnameIs(pev, "monster_blkop_osprey");
	m_defaultModel = "models/osprey.mdl";
	replenishMonster = isBlkOps ? "monster_male_assassin" : "monster_human_grunt";

	UTIL_PrecacheOther(replenishMonster);

	PRECACHE_MODEL(GetModel());
	PRECACHE_MODEL("models/HVR.mdl");

	PRECACHE_SOUND("apache/ap_rotor2.wav");
	PRECACHE_SOUND("weapons/mortarhit.wav");

	m_iSpriteTexture = PRECACHE_MODEL( "sprites/rope.spr" );

	m_iExplode	= PRECACHE_MODEL( "sprites/fexplo.spr" );

	if (isBlkOps) {
		m_iTailGibs = PRECACHE_MODEL("models/blkop_tailgibs.mdl");
		m_iBodyGibs = PRECACHE_MODEL("models/blkop_bodygibs.mdl");
		m_iEngineGibs = PRECACHE_MODEL("models/blkop_enginegibs.mdl");
	} else {
		m_iTailGibs = PRECACHE_MODEL("models/osprey_tailgibs.mdl");
		m_iBodyGibs = PRECACHE_MODEL("models/osprey_bodygibs.mdl");
		m_iEngineGibs = PRECACHE_MODEL("models/osprey_enginegibs.mdl");
	}

	m_iGlassHit = PRECACHE_MODEL("sprites/xfire2.spr");
	m_iEngineHit = PRECACHE_MODEL("sprites/muz1.spr");
	m_iGlassGibs = PRECACHE_MODEL("models/chromegibs.mdl");
	m_iMechGibs = PRECACHE_MODEL("models/bigshrapnel.mdl");
}

void COsprey::CommandUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	pev->nextthink = gpGlobals->time + 0.1;
}

void COsprey :: FindAllThink( void )
{
	CBaseEntity *pEntity = NULL;

	bool isOspreyPlayerAlly = CBaseEntity::IRelationship(Classify(), CLASS_PLAYER) == R_AL;

	m_iUnits = 0;
	while (m_iUnits < OSPREY_MAX_CARRY && (pEntity = UTIL_FindEntityByClassname( pEntity, replenishMonster)) != NULL)
	{
		bool isUnitPlayerAlly = CBaseEntity::IRelationship(pEntity->Classify(), CLASS_PLAYER) == R_AL;
		if (pEntity->IsAlive() && isUnitPlayerAlly == isOspreyPlayerAlly)
		{
			m_hGrunt[m_iUnits]		= pEntity;
			m_vecOrigin[m_iUnits]	= pEntity->pev->origin;
			m_iUnits++;
		}
	}

	if (m_iUnits == 0)
	{
		m_iUnits = 4;
		ALERT( at_console, "osprey warning: no grunts to resupply (assuming 4)\n");
		//UTIL_Remove( this );
		//return;
	}

	if (m_overrideGruntCount > 0) {
		m_iUnits = m_overrideGruntCount;
	}

	SetThink( &COsprey::FlyThink );
	pev->nextthink = gpGlobals->time + 0.1;
	m_startTime = gpGlobals->time;
}


void COsprey :: DeployThink( void )
{
	UTIL_MakeAimVectors( pev->angles );

	Vector vecForward = gpGlobals->v_forward;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;

	Vector vecSrc;

	TraceResult tr;
	UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, -4096.0), ignore_monsters, ENT(pev), &tr);
	CSoundEnt::InsertSound ( bits_SOUND_DANGER, tr.vecEndPos, 400, 0.3 );

	vecSrc = pev->origin + vecForward *  32 + vecRight *  100 + vecUp * -96;
	m_hRepel[0] = MakeGrunt( vecSrc );

	vecSrc = pev->origin + vecForward * -64 + vecRight *  100 + vecUp * -96;
	m_hRepel[1] = MakeGrunt( vecSrc );

	vecSrc = pev->origin + vecForward *  32 + vecRight * -100 + vecUp * -96;
	m_hRepel[2] = MakeGrunt( vecSrc );

	vecSrc = pev->origin + vecForward * -64 + vecRight * -100 + vecUp * -96;
	m_hRepel[3] = MakeGrunt( vecSrc );

	SetThink( &COsprey::HoverThink );
	pev->nextthink = gpGlobals->time + 0.1;
}



BOOL COsprey :: HasDead( )
{
	for (int i = 0; i < m_iUnits; i++)
	{
		if (m_hGrunt[i] == NULL || !m_hGrunt[i]->IsAlive())
		{
			return TRUE;
		}
		else
		{
			m_vecOrigin[i] = m_hGrunt[i]->pev->origin;  // send them to where they died
		}
	}
	return FALSE;
}


CBaseMonster *COsprey :: MakeGrunt( Vector vecSrc )
{
	CBaseEntity *pEntity;
	CBaseMonster *pGrunt;

	TraceResult tr;
	UTIL_TraceLine( vecSrc, vecSrc + Vector( 0, 0, -4096.0), dont_ignore_monsters, ENT(pev), &tr);
	if ( tr.pHit && Instance( tr.pHit )->pev->solid != SOLID_BSP) 
		return NULL;

	Vector spawnAngles = pev->angles;
	spawnAngles.x = spawnAngles.z = 0;
	for (int i = 0; i < m_iUnits; i++)
	{
		if (m_hGrunt[i] == NULL || !m_hGrunt[i]->IsAlive())
		{
			if (m_hGrunt[i] != NULL && m_hGrunt[i]->pev->rendermode == kRenderNormal)
			{
				m_hGrunt[i]->SUB_StartFadeOut( );
			}
			
			const char* spawnGrunt = replenishMonster;
			bool invertAlly = false;
			StringMap keys;

			switch (m_overrideGruntType) {
			case OSPREY_GRUNT_OP4:
				spawnGrunt = "monster_human_grunt_ally";
				invertAlly = true;
				break;
			case OSPREY_GRUNT_RANDOM: {
				switch (RANDOM_LONG(0, 1)) {
				case 0:
					spawnGrunt = "monster_human_grunt_ally";
					invertAlly = true;
					break;
				default:
					spawnGrunt = "monster_human_grunt";
					break;
				}
				break;
			}
			case OSPREY_GRUNT_RANDOM_HWGRUNT: {
				switch (RANDOM_LONG(0, 4)) {
				case 0:
				case 1:
					spawnGrunt = "monster_human_grunt_ally";
					invertAlly = true;
					break;
				case 2:
				case 3:
					spawnGrunt = "monster_human_grunt";
					break;
				case 4:
					spawnGrunt = "monster_hwgrunt";
					break;
				}
				break;
			}
			case OSPREY_GRUNT_RANDOM_HWGRUNT_SNIPER: {
				switch (RANDOM_LONG(0, 5)) {
				case 0:
				case 1:
					spawnGrunt = "monster_human_grunt_ally";
					invertAlly = true;
					break;
				case 2:
				case 3:
					spawnGrunt = "monster_human_grunt";
					break;
				case 4:
					spawnGrunt = "monster_hwgrunt";
					break;
				case 5:
					spawnGrunt = "monster_human_grunt";
					keys.put("weapons", "130");
					break;
				}
			}
			case OSPREY_GRUNT_DEFAULT:
			default:
				break;
			}

			if ((bool)m_IsPlayerAlly != invertAlly) {
				keys.put("is_player_ally", "1");
			}

			pEntity = Create(spawnGrunt, vecSrc, spawnAngles, true, NULL, keys);
			pGrunt = pEntity->MyMonsterPointer( );
			pGrunt->pev->movetype = MOVETYPE_FLY;
			pGrunt->pev->velocity = Vector( 0, 0, RANDOM_FLOAT( -196, -128 ) );
			pGrunt->SetActivity( ACT_GLIDE );

			CBeam *pBeam = CBeam::BeamCreate( "sprites/rope.spr", 10 );
			pBeam->PointEntInit( vecSrc + Vector(0,0,112), pGrunt->entindex() );
			pBeam->SetFlags( BEAM_FSOLID );
			pBeam->SetColor( 255, 255, 255 );
			pBeam->SetThink( &CBeam::SUB_Remove );
			pBeam->pev->nextthink = gpGlobals->time + -4096.0 * tr.flFraction / pGrunt->pev->velocity.z + 0.5;

			// ALERT( at_console, "%d at %.0f %.0f %.0f\n", i, m_vecOrigin[i].x, m_vecOrigin[i].y, m_vecOrigin[i].z );  
			pGrunt->m_vecLastPosition = m_vecOrigin[i];
			m_hGrunt[i] = pGrunt;
			return pGrunt;
		}
	}
	// ALERT( at_console, "none dead\n");
	return NULL;
}


void COsprey :: HoverThink( void )
{
	int i;
	for (i = 0; i < 4; i++)
	{
		if (m_hRepel[i] != NULL && m_hRepel[i]->pev->health > 0 && !(m_hRepel[i]->pev->flags & FL_ONGROUND))
		{
			break;
		}
	}

	if (i == 4)
	{
		m_startTime = gpGlobals->time;
		SetThink( &COsprey::FlyThink );
	}

	pev->nextthink = gpGlobals->time + 0.1;
	UTIL_MakeAimVectors( pev->angles );
	Update();
	UpdateShockEffect();
}


void COsprey::UpdateGoal( )
{
	if (m_hGoalEnt)
	{
		m_pos1 = m_pos2;
		m_ang1 = m_ang2;
		m_vel1 = m_vel2;
		m_pos2 = m_hGoalEnt->pev->origin;
		m_ang2 = m_hGoalEnt->pev->angles;
		UTIL_MakeAimVectors( Vector( 0, m_ang2.y, 0 ) );
		m_vel2 = gpGlobals->v_forward * m_hGoalEnt->pev->speed;

		m_startTime = m_startTime + m_dTime;
		m_dTime = 2.0 * (m_pos1 - m_pos2).Length() / (m_vel1.Length() + m_hGoalEnt->pev->speed);

		if (m_ang1.y - m_ang2.y < -180)
		{
			m_ang1.y += 360;
		}
		else if (m_ang1.y - m_ang2.y > 180)
		{
			m_ang1.y -= 360;
		}
	}
	else
	{
		ALERT( at_console, "osprey missing target");
	}
}


void COsprey::FlyThink( void )
{
	StudioFrameAdvance( );
	UpdateShockEffect();
	pev->nextthink = gpGlobals->time + 0.1;

	if (!m_hGoalEnt && !FStringNull(pev->target) )// this monster has a target
	{
		m_hGoalEnt = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ) );
		UpdateGoal( );
	}

	if (m_hGoalEnt && gpGlobals->time > m_startTime + m_dTime)
	{
		if (m_hGoalEnt->pev->speed == 0)
		{
			SetThink( &COsprey::DeployThink );
		}
		do {
			m_hGoalEnt = UTIL_FindEntityByTargetname( NULL, STRING(m_hGoalEnt->pev->target ) );
		} while (m_hGoalEnt->pev->speed < 400 && !HasDead());
		UpdateGoal( );
	}

	Flight( );
	Update();
}


void COsprey::Flight( )
{
	float t = (gpGlobals->time - m_startTime);
	float scale = 1.0 / m_dTime;
	
	float f = UTIL_SplineFraction( t * scale, 1.0 );

	if (f > 0) {
		Vector pos = (m_pos1 + m_vel1 * t) * (1.0 - f) + (m_pos2 - m_vel2 * (m_dTime - t)) * f;
		Vector ang = (m_ang1) * (1.0 - f) + (m_ang2)*f;
		m_velocity = m_vel1 * (1.0 - f) + m_vel2 * f;
		UTIL_SetOrigin(pev, pos);
		pev->angles = ang;
	}

	
	UTIL_MakeAimVectors( pev->angles );
	float flSpeed = DotProduct( gpGlobals->v_forward, m_velocity );

	// float flSpeed = DotProduct( gpGlobals->v_forward, pev->velocity );

	float m_flIdealtilt = (160 - flSpeed) / 10.0;

	// ALERT( at_console, "%f %f\n", flSpeed, flIdealtilt );
	if (m_flRotortilt < m_flIdealtilt)
	{
		m_flRotortilt += 0.5;
		if (m_flRotortilt > 0)
			m_flRotortilt = 0;
	}
	if (m_flRotortilt > m_flIdealtilt)
	{
		m_flRotortilt -= 0.5;
		if (m_flRotortilt < -90)
			m_flRotortilt = -90;
	}
	SetBoneController( 0, m_flRotortilt );


	static int lastPitch[33];
	const int pitchShift = -20;

	// make rotor, engine sounds
	if (m_iSoundState == 0)
	{
		StartSound(edict(), CHAN_ITEM, "apache/ap_rotor2.wav", 1.0f, 0.3f, 0, 100 + pitchShift, g_vecZero, 0xffffffff);
		memset(lastPitch, 0, sizeof(int) * 33);
		m_iSoundState = SND_CHANGE_PITCH; // hack for going through level transitions
	}
	else
	{
		for (int i = 1; i < gpGlobals->maxClients; i++) {
			CBaseEntity* pPlayer = (CBaseEntity*)UTIL_PlayerByIndex(i);

			if (!pPlayer) {
				continue;
			}

			float dot = DotProduct(pev->velocity - pPlayer->pev->velocity, (pPlayer->pev->origin - pev->origin).Normalize());

			int pitch = (int)(100 + dot / 75.0) + pitchShift;

			if (pitch > 250)
				pitch = 250;
			if (pitch < 50)
				pitch = 50;

			pitch = (pitch / 2) * 2; // reduce the amount of network messages

			if (pitch == 100)
				pitch = 101; // SND_CHANGE_PITCH will not work for 100 pitch (random sounds will play)

			if (pitch == lastPitch[pPlayer->entindex()]) {
				continue;
			}

			StartSound(edict(), CHAN_ITEM, "apache/ap_rotor2.wav", 1.0f, 0.3f, SND_CHANGE_PITCH,
				pitch, g_vecZero, PLRBIT(pPlayer->edict()));
			lastPitch[pPlayer->entindex()] = pitch;
		}
	}

}


void COsprey::HitTouch( CBaseEntity *pOther )
{
	pev->nextthink = gpGlobals->time + 2.0;
}

void COsprey::Update()
{
	Look(4092); // Look around so AI triggers work.
	Listen(); // Listen for sounds so AI triggers work.

	ShowDamage();
	FCheckAITrigger();
}

int COsprey::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
/*	if (m_flRotortilt <= -90)
	{
		m_flRotortilt = 0;
	}
	else
	{
		m_flRotortilt -= 45;
	}
	SetBoneController( 0, m_flRotortilt );
	return 0;*/

	//Set enemy to last attacker.
	//Ospreys are not capable of fighting so they'll get angry at whatever shoots at them, not whatever looks like an enemy.
	m_hEnemy = Instance(pevAttacker);
	//It's on now!
	m_MonsterState = MONSTERSTATE_COMBAT;
	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}



void COsprey :: Killed( entvars_t *pevAttacker, int iGib )
{
	CBaseMonster::Killed(pevAttacker, GIB_NEVER); // for monstermaker death notice + death trigger

	pev->movetype = MOVETYPE_TOSS;
	pev->gravity = 0.3;
	pev->velocity = m_velocity;
	pev->avelocity = Vector( RANDOM_FLOAT( -20, 20 ), 0, RANDOM_FLOAT( -50, 50 ) );
	STOP_SOUND(ENT(pev), CHAN_ITEM, "apache/ap_rotor2.wav");

	UTIL_SetSize( pev, Vector( -32, -32, -64), Vector( 32, 32, 0) );
	SetThink( &COsprey::DyingThink );
	SetTouch( &COsprey::CrashTouch );
	pev->nextthink = gpGlobals->time + 0.1;
	pev->health = 0;
	pev->takedamage = DAMAGE_NO;

	m_startTime = gpGlobals->time + 4.0;
}

void COsprey::CrashTouch( CBaseEntity *pOther )
{
	// only crash if we hit something solid
	if ( pOther->pev->solid > SOLID_TRIGGER)
	{
		SetTouch( NULL );
		m_startTime = gpGlobals->time;
		pev->nextthink = gpGlobals->time;
		m_velocity = pev->velocity;
	}
}


void COsprey :: DyingThink( void )
{
	UpdateShockEffect();
	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	pev->avelocity = pev->avelocity * 1.02;

	// still falling?
	if (m_startTime > gpGlobals->time )
	{
		UTIL_MakeAimVectors( pev->angles );
		ShowDamage();

		Vector vecSpot = pev->origin + pev->velocity * 0.2;

		// random explosions
		Vector ori = vecSpot + Vector(RANDOM_FLOAT(-150, 150), RANDOM_FLOAT(-150, 150), RANDOM_FLOAT(-150, -50));
		UTIL_Explosion(ori, g_sModelIndexFireball, RANDOM_LONG(0, 29) + 30, 12, TE_EXPLFLAG_NONE);

		// lots of smoke
		ori = vecSpot + Vector(RANDOM_FLOAT(-150, 150), RANDOM_FLOAT(-150, 150), RANDOM_FLOAT(-150, -50));
		UTIL_Smoke(ori, g_sModelIndexSmoke, 100, 10);

		vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;

		int gibCount = UTIL_IsValidTempEntOrigin(vecSpot) ? 8 : 1;
		UTIL_BreakModel(vecSpot, Vector(800, 800, 132), pev->velocity, 50,
			m_iTailGibs, gibCount, 20, BREAK_METAL);

		// don't stop it we touch a entity
		pev->flags &= ~FL_ONGROUND;
		pev->nextthink = gpGlobals->time + 0.2;
		return;
	}
	else
	{
		Vector vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;

		UTIL_Sprite(vecSpot, m_iExplode, 250, 255);

		// blast circle
		UTIL_BeamCylinder(pev->origin, 2000, m_iSpriteTexture, 0, 0, 4, 32, 0, RGBA(255, 255, 192, 128), 0);

		EMIT_SOUND(ENT(pev), CHAN_STATIC, "weapons/mortarhit.wav", 1.0, 0.3);

		RadiusDamage( pev->origin, pev, pev, 300, CLASS_NONE, DMG_BLAST );

		// gibs
		vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
		UTIL_BreakModel(vecSpot, Vector(800, 800, 128), m_velocity, 40,
			m_iBodyGibs, 128, 200, BREAK_METAL);
		UTIL_Remove( this );
	}
}


void COsprey :: ShowDamage( void )
{
	if (m_flLeftHealth < 0)
	{
		Vector vecSrc = pev->origin + gpGlobals->v_right * -340;
		UTIL_Smoke(vecSrc, g_sModelIndexSmoke, RANDOM_LONG(0, 9) + 20, 12);
	}
	if (m_flRightHealth < 0)
	{
		Vector vecSrc = pev->origin + gpGlobals->v_right * 340;
		UTIL_Smoke(vecSrc, g_sModelIndexSmoke, RANDOM_LONG(0, 9) + 20, 12);
	}
}


void COsprey::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	// ALERT( at_console, "%d %.0f\n", ptr->iHitgroup, flDamage );
	bool isBlast = bitsDamageType & DMG_BLAST;
	bool engineExploded = false;

	if (bitsDamageType & DMG_BLAST)
	{
		flDamage *= 2;
	}

	// only so much per engine
	if (ptr->iHitgroup == 3)
	{
		if (m_flRightHealth < 0 && !isBlast) {
			UTIL_Ricochet(ptr->vecEndPos, 1.0f);
			return;
		}
		else {
			if (m_flRightHealth >= 0 && m_flRightHealth - flDamage < 0) {
				engineExploded = true;
			}
			m_flRightHealth -= flDamage;
		}
	}

	if (ptr->iHitgroup == 2)
	{
		if (m_flLeftHealth < 0 && !isBlast) {
			UTIL_Ricochet(ptr->vecEndPos, 1.0f);
			return;
		}
		else {
			if (m_flLeftHealth >= 0 && m_flLeftHealth - flDamage < 0) {
				engineExploded = true;
			}
			m_flLeftHealth -= flDamage;
		}
	}

	bool hitHard = flDamage >= 50;
	bool isShock = bitsDamageType & DMG_SHOCK;
	bool isEgon = bitsDamageType & DMG_ENERGYBEAM;
	bool hitWeakpoint = ptr->iHitgroup == 1 || ptr->iHitgroup == 2 || ptr->iHitgroup == 3;

	// hit hard, hits cockpit, hits engines
	if (hitHard || hitWeakpoint || isBlast || isEgon)
	{
		Vector dir = ptr->vecPlaneNormal;
		Vector pos = ptr->vecEndPos;
		int gibCount = 1;
		int gibModel = ptr->iHitgroup == 1 ? m_iGlassGibs : m_iMechGibs;

		if (isBlast) {
			gibModel = m_iBodyGibs;

			if (flDamage > 80) {
				gibCount = 8;
			}
			else if (flDamage > 30) {
				gibCount = 4;
			}
			else {
				gibCount = 2;
			}
		}
		else {
			gibCount = 1 + (int)(flDamage / 30.0);
		}

		if (engineExploded) {
			// engine destroyed
			ExplosionCreate(pos, g_vecZero, NULL, 100, true);
			gibCount = 16;
			gibModel = m_iBodyGibs;
		}
		else if (!isShock && !isEgon) {
			// cockpit
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pos);
			WRITE_BYTE(TE_STREAK_SPLASH);
			WRITE_COORD(pos.x);
			WRITE_COORD(pos.y);
			WRITE_COORD(pos.z);
			WRITE_COORD(dir.x);
			WRITE_COORD(dir.y);
			WRITE_COORD(dir.z);
			WRITE_BYTE(ptr->iHitgroup == 1 ? 0 : 5);
			WRITE_SHORT(flDamage >= 40 ? 16 : 8);
			WRITE_SHORT(768);
			WRITE_SHORT(256);
			MESSAGE_END();
			
			if (ptr->iHitgroup == 1 || ptr->iHitgroup == 2 || ptr->iHitgroup == 3) {
				Vector sprPos = pos + dir * 4;

				if (ptr->iHitgroup == 1) {
					UTIL_Explosion(sprPos, m_iGlassHit, 12, 80, 2 | 4 | 8);
				}
				else {
					UTIL_Explosion(sprPos, m_iEngineHit, 6 + (flDamage / 10), 50, 2 | 4 | 8);
				}
			}
		}

		if (isShock || isEgon) {
			gibCount = 0;
		}

		if (gibCount) {
			if (dir.Length() < 1.0f) {
				dir = (ptr->vecEndPos - pev->origin).Normalize();
				dir.z *= 0.2f;
				dir = dir.Normalize();
			}
			dir = dir * (isBlast ? 400 : 200);

			UTIL_BreakModel(pos, g_vecZero, dir, isBlast ? 30 : 15,
				gibModel, gibCount, 1, gibModel == m_iBodyGibs ? BREAK_METAL : 0);
		}

		// ALERT( at_console, "%.0f\n", flDamage );
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );
	}
	else
	{
		UTIL_Ricochet(ptr->vecEndPos, 1.0f);
	}
}





