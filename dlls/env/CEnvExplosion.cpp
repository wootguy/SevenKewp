#include "extdll.h"
#include "util.h"
#include "decals.h"
#include "explode.h"
#include "weapons.h"

class CEnvExplosion : public CPointEntity
{
public:
	void Spawn();
	void EXPORT Smoke(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	const char* DisplayName() { return "Explosion"; }

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	int m_iMagnitude;// how large is the fireball? how much damage?
	int m_spriteScale; // what's the exact fireball sprite scale? 
	Vector m_effectOrigin; // where to play the explosion effects (offset from real origin so sprites look nice)
};

TYPEDESCRIPTION	CEnvExplosion::m_SaveData[] =
{
	DEFINE_FIELD(CEnvExplosion, m_iMagnitude, FIELD_INTEGER),
	DEFINE_FIELD(CEnvExplosion, m_spriteScale, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvExplosion, CPointEntity)
LINK_ENTITY_TO_CLASS(env_explosion, CEnvExplosion)

void CEnvExplosion::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "iMagnitude"))
	{
		m_iMagnitude = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CEnvExplosion::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;

	pev->movetype = MOVETYPE_NONE;
	/*
	if ( m_iMagnitude > 250 )
	{
		m_iMagnitude = 250;
	}
	*/

	float flSpriteScale;
	flSpriteScale = (m_iMagnitude - 50) * 0.6;

	/*
	if ( flSpriteScale > 50 )
	{
		flSpriteScale = 50;
	}
	*/
	if (flSpriteScale < 10)
	{
		flSpriteScale = 10;
	}

	m_spriteScale = (int)flSpriteScale;
}

void CEnvExplosion::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	TraceResult tr;

	pev->model = iStringNull;//invisible
	pev->solid = SOLID_NOT;// intangible

	Vector		vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector(0, 0, 8);

	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);

	m_effectOrigin = pev->origin;

	// Pull out of the wall a bit
	if (tr.flFraction != 1.0)
	{
		float dist = V_max(16, (m_iMagnitude - 24)) * 0.6;
		m_effectOrigin = tr.vecEndPos + (tr.vecPlaneNormal * dist);

		if (mp_explosionbug.value == 0) {
			// Move damage origin closer to sprite origin so explosions don't detonate inside
			// tiny puddles and do no damage. TODO: duplicated in CGrenade.
			Vector damageOrigin = tr.vecEndPos + (tr.vecPlaneNormal * V_min(32, dist));

			TraceResult tr;
			TRACE_LINE(pev->origin, damageOrigin, ignore_monsters, NULL, &tr);
			if (!tr.fStartSolid)
				pev->origin = tr.vecEndPos;
		}
	}

	if (mp_explosionbug.value) {
		pev->origin = m_effectOrigin;
	}

	// draw decal
	if (!(pev->spawnflags & SF_ENVEXPLOSION_NODECAL))
	{
		if (RANDOM_FLOAT(0, 1) < 0.5)
		{
			UTIL_DecalTrace(&tr, DECAL_SCORCH1);
		}
		else
		{
			UTIL_DecalTrace(&tr, DECAL_SCORCH2);
		}
	}

	// draw fireball
	if (!(pev->spawnflags & SF_ENVEXPLOSION_NOFIREBALL))
	{
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, m_effectOrigin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(m_effectOrigin.x);
		WRITE_COORD(m_effectOrigin.y);
		WRITE_COORD(m_effectOrigin.z);
		WRITE_SHORT(g_sModelIndexFireball);
		WRITE_BYTE((BYTE)m_spriteScale); // scale * 10
		WRITE_BYTE(15); // framerate
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();
	}
	else
	{
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(m_effectOrigin.x);
		WRITE_COORD(m_effectOrigin.y);
		WRITE_COORD(m_effectOrigin.z);
		WRITE_SHORT(g_sModelIndexFireball);
		WRITE_BYTE(0); // no sprite
		WRITE_BYTE(15); // framerate
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();
	}

	PLAY_DISTANT_SOUND(edict(), DISTANT_BOOM);

	// do damage
	if (!(pev->spawnflags & SF_ENVEXPLOSION_NODAMAGE))
	{
		entvars_t* ownerpev = pev->owner ? &pev->owner->v : pev;
		::RadiusDamage(m_effectOrigin, pev, ownerpev, m_iMagnitude, m_iMagnitude * 2.5, CLASS_NONE, DMG_BLAST);
	}

	SetThink(&CEnvExplosion::Smoke);
	pev->nextthink = gpGlobals->time + 0.3;

	// draw sparks
	if (!(pev->spawnflags & SF_ENVEXPLOSION_NOSPARKS))
	{
		int sparkCount = RANDOM_LONG(0, 3);

		for (int i = 0; i < sparkCount; i++)
		{
			Create("spark_shower", m_effectOrigin, tr.vecPlaneNormal, NULL);
		}
	}
}

void CEnvExplosion::Smoke(void)
{
	if (!(pev->spawnflags & SF_ENVEXPLOSION_NOSMOKE))
	{
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, m_effectOrigin);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(m_effectOrigin.x);
		WRITE_COORD(m_effectOrigin.y);
		WRITE_COORD(m_effectOrigin.z);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE((BYTE)m_spriteScale); // scale * 10
		WRITE_BYTE(12); // framerate
		MESSAGE_END();
	}

	if (!(pev->spawnflags & SF_ENVEXPLOSION_REPEATABLE))
	{
		UTIL_Remove(this);
	}
}


// HACKHACK -- create one of these and fake a keyvalue to get the right explosion setup
void ExplosionCreate(const Vector& center, const Vector& angles, edict_t* pOwner, int magnitude, BOOL doDamage)
{
	KeyValueData	kvd;
	char			buf[128];

	CBaseEntity* pExplosion = CBaseEntity::Create("env_explosion", center, angles, pOwner);
	snprintf(buf, 128, "%3d", magnitude);
	kvd.szKeyName = "iMagnitude";
	kvd.szValue = buf;
	pExplosion->KeyValue(&kvd);
	if (!doDamage)
		pExplosion->pev->spawnflags |= SF_ENVEXPLOSION_NODAMAGE;

	pExplosion->Spawn();
	pExplosion->Use(NULL, NULL, USE_TOGGLE, 0);
}
