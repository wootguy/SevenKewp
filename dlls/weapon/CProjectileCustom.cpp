#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "CWeaponCustom.h"
#include "CProjectileCustom.h"
#include "Scheduler.h"
#include "te_effects.h"
#include "weapons.h"

void CProjectileCustom::Spawn() {
	bubbleDelay = 0.07f;
	thinkDelay = 0.05f;
	
	if (!damageType)
		damageType = DMG_GENERIC;

	pev->solid = SOLID_BBOX;

	SET_MODEL(edict(), STRING(pev->model));
	ResetSequenceInfo();

	SetThink(&CProjectileCustom::MoveThink);
	pev->nextthink = gpGlobals->time + thinkDelay;

	PlayMoveSound();
}

void CProjectileCustom::PlayMoveSound() {
	if (move_snd && !move_snd_playing) {
		EMIT_SOUND_DYN(edict(), CHAN_BODY, STRING(move_snd), 1.0f, ATTN_NORM, 0, 100);
		move_snd_playing = true;
	}
}

void CProjectileCustom::StopMoveSound() {
	if (move_snd && move_snd_playing) {
		STOP_SOUND(edict(), CHAN_BODY, STRING(move_snd));
		move_snd_playing = true;
	}
}

void CProjectileCustom::MoveThink() {
	if (pev->air_finished > 0 || (expire_time && gpGlobals->time > expire_time))
	{
		Die();
		Remove();
		return;
	}

	CustomMove();
}

void CProjectileCustom::CustomMove() {
	if (attached && attachTarget)
	{
		CBaseEntity* tar = attachTarget;

		// rotate position around target
		Vector newOri = attachStartOri + (tar->pev->origin - targetStartOri);
		newOri = UTIL_RotatePoint(newOri - tar->pev->origin, -tar->pev->angles) + tar->pev->origin;

		// rotate orientation around target
		Vector newDir = UTIL_RotatePoint(attachStartDir, -tar->pev->angles);
		g_engfuncs.pfnVecToAngles(newDir, pev->angles);

		pev->origin = newOri;

		// prevent sudden jerking due to movement lagging behind the Touch() event
		attachTime++;
		if (attachTime > 2) {
			pev->velocity = Vector(0, 0, 0);
			pev->movetype = MOVETYPE_FLY;
		}

		if (!tar->IsBSPModel() && !tar->IsAlive())
		{
			attached = false;
			attachTarget = NULL;
			if (flags & FL_WC_PROJ_IS_HOOK)
			{
				Remove();
				return;
			}
			pev->movetype = pev->gravity != 0 ? MOVETYPE_BOUNCE : MOVETYPE_BOUNCEMISSILE;
		}
	}
	else
	{
		if (attached)
		{
			attached = false;
			if (flags & FL_WC_PROJ_IS_HOOK)
			{
				Remove();
				return;
			}
			pev->movetype = pev->gravity != 0 ? MOVETYPE_BOUNCE : MOVETYPE_BOUNCEMISSILE;
		}
		if (!(flags & FL_WC_PROJ_NO_ORIENT))
			g_engfuncs.pfnVecToAngles(pev->velocity, pev->angles);
	}

	if (move_snd_playing && pev->velocity.Length() == 0)
		StopMoveSound();

	bool noBubbles = (flags & FL_WC_PROJ_NO_BUBBLES) != 0;
	bool inWater = g_engfuncs.pfnPointContents(pev->origin) == CONTENTS_WATER;
	if (!attached && !noBubbles && inWater)
	{
		if (nextBubbleTime < gpGlobals->time)
		{
			Vector pos = pev->origin;
			//float waterLevel = UTIL_WaterLevel(pos, pos.z, pos.z + 1024) - pos.z;
			//UTIL_BubbleTrail(pos, pos, "sprites/bubble.spr", waterLevel, 1, 16.0f);
			UTIL_BubbleTrail(pos, pos, 1);
			nextBubbleTime = gpGlobals->time + bubbleDelay;
		}
	}

	if (inWater && water_friction != 0)
	{
		float speed = pev->velocity.Length();
		if (speed > 0)
			pev->velocity = pev->velocity.Normalize(speed - speed * water_friction);
	}
	else if (!inWater && air_friction != 0)
	{
		float speed = pev->velocity.Length();
		if (speed > 0)
			pev->velocity = pev->velocity.Normalize(speed - speed * air_friction);
	}

	pev->nextthink = gpGlobals->time + thinkDelay;
}

void CProjectileCustom::Remove() {
	StopMoveSound();
	UTIL_Remove(this);
	if (spriteAttachment)
		UTIL_Remove(spriteAttachment);
}

void CProjectileCustom::DamageTarget(CBaseEntity* ent) {
	if (ent == NULL || ent->entindex() == 0)
		return;

	CBaseEntity* owner = CBaseEntity::Instance(pev->owner);

	TraceResult tr;
	Vector vecSrc = pev->origin;
	Vector vecAiming = pev->velocity.Normalize();
	Vector vecEnd = vecSrc + vecAiming * pev->velocity.Length() * 2;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, edict(), &tr);
	CBaseEntity* pHit = tr.pHit ? CBaseEntity::Instance(tr.pHit) : NULL;
	//te_beampoints(vecSrc, tr.vecEndPos);
	if (tr.flFraction >= 1.0 || pHit->entindex() != ent->entindex())
	{
		// This does a trace in the form of a box so there is a much higher chance of hitting something
		// From crowbar.cpp in the hlsdk:
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, edict(), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) && the object we hit
			// This is an approximation of the "best" intersection
			pHit = CBaseEntity::Instance(tr.pHit);
			if (pHit == NULL || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, edict());
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
			vecAiming = (vecEnd - vecSrc).Normalize();
		}
	}

	Vector oldVel = ent->pev->velocity;

	ClearMultiDamage();
	ent->TraceAttack(owner->pev, damage, vecAiming, &tr, damageType);
	ApplyMultiDamage(owner->pev, owner->pev);

	if ((damageType & DMG_LAUNCH) == 0) // prevent high damage from launching unless we ask for it
		ent->pev->velocity = oldVel;
}

void CProjectileCustom::Touch(CBaseEntity* pOther) {
	CustomTouch(pOther);
}

WeaponCustomProjectileAction CProjectileCustom::getImpactAction(CBaseEntity* pOther) {
	return pOther->IsBSPModel() ? world_event : monster_event;
}

void CProjectileCustom::Impact(CBaseEntity* pOther) {
	Remove();
}

void CProjectileCustom::Bounce(CBaseEntity* pOther) {
	
}

void CProjectileCustom::Attach(CBaseEntity* pOther) {
	attachTarget = pOther;
	attachStartOri = UTIL_UnwindPoint(pev->origin - pOther->pev->origin, -pOther->pev->angles) + pOther->pev->origin;
	attachStartDir = UTIL_UnwindPoint(pev->velocity.Normalize(), -pOther->pev->angles);
	targetStartOri = pOther->pev->origin;
	pev->solid = SOLID_NOT;
	pev->velocity = Vector(0, 0, 0);
	pev->avelocity = Vector(0, 0, 0);
	attached = true;
}

void CProjectileCustom::CustomTouch(CBaseEntity* pOther) {
	DamageTarget(pOther);

	switch (getImpactAction(pOther))
	{
	case WC_PROJ_ACT_IMPACT:
		Impact(pOther);
		break;
	case WC_PROJ_ACT_BOUNCE:
		Bounce(pOther);
		break;
	case WC_PROJ_ACT_ATTACH:
		Attach(pOther);
		break;
	}
}

LINK_ENTITY_TO_CLASS(custom_projectile, CProjectileCustom)