#include "CBasePlayerAmmo.h"

enum SporeAmmoAnim
{
	SPOREAMMO_IDLE = 0,
	SPOREAMMO_SPAWNUP,
	SPOREAMMO_SNATCHUP,
	SPOREAMMO_SPAWNDN,
	SPOREAMMO_SNATCHDN,
	SPOREAMMO_IDLE1,
	SPOREAMMO_IDLE2
};

enum SporeAmmoBody
{
	SPOREAMMOBODY_EMPTY = 0,
	SPOREAMMOBODY_FULL
};

class CSporeAmmo : public CBasePlayerAmmo
{
public:
	void Precache() override
	{
		PRECACHE_MODEL("models/spore_ammo.mdl");
		PRECACHE_SOUND("weapons/spore_ammo.wav");
	}

	void Spawn() override
	{
		Precache();

		SET_MODEL(ENT(pev), "models/spore_ammo.mdl");

		pev->movetype = MOVETYPE_FLY;

		UTIL_SetSize(pev, Vector(-16, -16, -16), Vector(16, 16, 16));

		pev->origin.z += 16;

		UTIL_SetOrigin(pev, pev->origin);

		pev->angles.x -= 90;

		pev->sequence = SPOREAMMO_SPAWNDN;

		pev->animtime = gpGlobals->time;

		pev->nextthink = gpGlobals->time + 4;

		pev->frame = 0;
		pev->framerate = 1;

		pev->health = 1;

		pev->body = SPOREAMMOBODY_FULL;

		pev->takedamage = DAMAGE_AIM;

		pev->solid = SOLID_BBOX;

		SetTouch(&CSporeAmmo::SporeTouch);
		SetThink(&CSporeAmmo::Idling);
	}

	BOOL TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType ) override
	{
		if (pev->body == SPOREAMMOBODY_EMPTY)
		{
			return false;
		}

		pev->body = SPOREAMMOBODY_EMPTY;

		pev->sequence = SPOREAMMO_SNATCHDN;

		pev->animtime = gpGlobals->time;
		pev->frame = 0;
		pev->nextthink = gpGlobals->time + 0.66;

		auto vecLaunchDir = pev->angles;

		vecLaunchDir.x -= 90;
		// Rotate it so spores that aren't rotated in Hammer point in the right direction.
		vecLaunchDir.y += 180;

		vecLaunchDir.x += RANDOM_FLOAT(-20, 20);
		vecLaunchDir.y += RANDOM_FLOAT(-20, 20);
		vecLaunchDir.z += RANDOM_FLOAT(-20, 20);

		auto pSpore = CSpore::CreateSpore(pev->origin, vecLaunchDir, this, CSpore::SporeType::GRENADE, false, true);

		UTIL_MakeVectors(vecLaunchDir);

		pSpore->pev->velocity = gpGlobals->v_forward * 800;

		return false;
	}

	BOOL AddAmmo(CBaseEntity* pOther)  override
	{
		//return pOther->GiveAmmo(AMMO_SPORE_GIVE, "spores", SPORE_MAX_CARRY, "weapons/spore_ammo.wav");
		return pOther->GiveAmmo(AMMO_SPORE_GIVE, "spores", SPORE_MAX_CARRY);
	}

	void Idling()
	{
		switch (pev->sequence)
		{
		case SPOREAMMO_SPAWNDN:
		{
			pev->sequence = SPOREAMMO_IDLE1;
			pev->animtime = gpGlobals->time;
			pev->frame = 0;
			break;
		}

		case SPOREAMMO_SNATCHDN:
		{
			pev->sequence = SPOREAMMO_IDLE;
			pev->animtime = gpGlobals->time;
			pev->frame = 0;
			pev->nextthink = gpGlobals->time + 10;
			break;
		}

		case SPOREAMMO_IDLE:
		{
			pev->body = SPOREAMMOBODY_FULL;
			pev->sequence = SPOREAMMO_SPAWNDN;
			pev->animtime = gpGlobals->time;
			pev->frame = 0;
			pev->nextthink = gpGlobals->time + 4;
			break;
		}

		default:
			break;
		}
	}

	void SporeTouch(CBaseEntity* pOther)
	{
		if (!pOther || !pOther->IsPlayer() || pev->body == SPOREAMMOBODY_EMPTY)
			return;

		if (AddAmmo(pOther))
		{
			pev->body = SPOREAMMOBODY_EMPTY;

			pev->sequence = SPOREAMMO_SNATCHDN;

			pev->animtime = gpGlobals->time;
			pev->frame = 0;
			pev->nextthink = gpGlobals->time + 0.66;

			EMIT_SOUND(edict(), CHAN_ITEM, "weapons/spore_ammo.wav", VOL_NORM, ATTN_NORM);
		}
	}
};

class CSporeClip : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/spore.mdl");
		CBasePlayerAmmo::Spawn();

		// don't sink into the ground (model origin is at the center, not the bottom)
		UTIL_SetSize(pev, Vector(-16, -16, -2), Vector(16, 16, 16));
	}

	void Precache(void)
	{
		PRECACHE_MODEL("models/spore.mdl");
		PRECACHE_SOUND("weapons/spore_ammo.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther)
	{
		if (pOther->GiveAmmo(AMMO_SPORE_GIVE, "spores", SPORE_MAX_CARRY) != -1)
		{
			EMIT_SOUND(edict(), CHAN_ITEM, "weapons/spore_ammo.wav", VOL_NORM, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(ammo_spore, CSporeAmmo);
LINK_ENTITY_TO_CLASS(ammo_sporeclip, CSporeClip);
