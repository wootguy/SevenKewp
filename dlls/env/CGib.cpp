#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "env/CSoundEnt.h"
#include "decals.h"
#include "animation.h"
#include "weapons.h"
#include "CBreakable.h"

#define GERMAN_GIB_COUNT		4
#define	HUMAN_GIB_COUNT			6
#define ALIEN_GIB_COUNT			4


// HACKHACK -- The gib velocity equations don't work
void CGib::LimitVelocity(void)
{
	float length = pev->velocity.Length();

	// ceiling at 1500.  The gib velocity equation is not bounded properly.  Rather than tune it
	// in 3 separate places again, I'll just limit it here.
	if (length > 1500.0)
		pev->velocity = pev->velocity.Normalize() * 1500;		// This should really be sv_maxvelocity * 0.75 or something
}


void CGib::SpawnStickyGibs(entvars_t* pevVictim, Vector vecOrigin, int cGibs)
{
	int i;

	if (g_Language == LANGUAGE_GERMAN)
	{
		// no sticky gibs in germany right now!
		return;
	}

	for (i = 0; i < cGibs; i++)
	{
		CGib* pGib = GetClassPtr((CGib*)NULL);

		pGib->Spawn("models/stickygib.mdl");
		pGib->pev->body = RANDOM_LONG(0, 2);

		if (pevVictim)
		{
			pGib->pev->origin.x = vecOrigin.x + RANDOM_FLOAT(-3, 3);
			pGib->pev->origin.y = vecOrigin.y + RANDOM_FLOAT(-3, 3);
			pGib->pev->origin.z = vecOrigin.z + RANDOM_FLOAT(-3, 3);

			/*
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) );
			*/

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT(-0.15, 0.15);
			pGib->pev->velocity.y += RANDOM_FLOAT(-0.15, 0.15);
			pGib->pev->velocity.z += RANDOM_FLOAT(-0.15, 0.15);

			pGib->pev->velocity = pGib->pev->velocity * 900;

			pGib->pev->avelocity.x = RANDOM_FLOAT(250, 400);
			pGib->pev->avelocity.y = RANDOM_FLOAT(250, 400);

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();

			if (pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if (pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}


			pGib->pev->movetype = MOVETYPE_TOSS;
			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize(pGib->pev, Vector(0, 0, 0), Vector(0, 0, 0));
			pGib->SetTouch(&CGib::StickyGibTouch);
			pGib->SetThink(NULL);
		}
		pGib->LimitVelocity();
	}
}

void CGib::SpawnHeadGib(entvars_t* pevVictim)
{
	CGib* pGib = GetClassPtr((CGib*)NULL);

	if (g_Language == LANGUAGE_GERMAN)
	{
		pGib->Spawn("models/germangibs.mdl");// throw one head
		pGib->pev->body = 0;
	}
	else
	{
		pGib->Spawn("models/hgibs.mdl");// throw one head
		pGib->pev->body = 0;
	}

	if (pevVictim)
	{
		pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;

		int numPvsPlayers;
		edict_t* pvsPlayers = UTIL_ClientsInPVS(pGib->edict(), numPvsPlayers);

		// 5% chance head will be thrown at player's face (or your face in particular, in co-op).
		if (numPvsPlayers && RANDOM_LONG(0, 100) <= 5 * numPvsPlayers)
		{
			int pickPlayer = RANDOM_LONG(0, numPvsPlayers - 1);
			edict_t* plr = pvsPlayers;
			for (int i = 0; i < pickPlayer; i++) {
				plr = pvsPlayers->v.chain;
			}

			entvars_t* pevPlayer = VARS(plr);
			pGib->pev->velocity = ((pevPlayer->origin + pevPlayer->view_ofs) - pGib->pev->origin).Normalize() * 300;
			pGib->pev->velocity.z += 100;
		}
		else {
			pGib->pev->velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));
		}

		pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
		pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

		// copy owner's blood color
		pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();

		if (pevVictim->health > -50)
		{
			pGib->pev->velocity = pGib->pev->velocity * 0.7;
		}
		else if (pevVictim->health > -200)
		{
			pGib->pev->velocity = pGib->pev->velocity * 2;
		}
		else
		{
			pGib->pev->velocity = pGib->pev->velocity * 4;
		}
	}
	pGib->LimitVelocity();
}

void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* gibModel, int gibModelBodyGroups, int bodyGroupSkip) {
	int cSplat;

	for (cSplat = 0; cSplat < cGibs; cSplat++)
	{
		CGib* pGib = GetClassPtr((CGib*)NULL);

		pGib->Spawn(gibModel);
		pGib->pev->body = RANDOM_LONG(bodyGroupSkip, gibModelBodyGroups - 1);

		if (pevVictim)
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT(0, 1));
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT(0, 1));
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT(0, 1)) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT(-0.25, 0.25);
			pGib->pev->velocity.y += RANDOM_FLOAT(-0.25, 0.25);
			pGib->pev->velocity.z += RANDOM_FLOAT(-0.25, 0.25);

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT(300, 400);

			pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
			pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();

			if (pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if (pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize(pGib->pev, Vector(0, 0, 0), Vector(0, 0, 0));
		}
		pGib->LimitVelocity();
	}
}

void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int human)
{
	if (g_Language == LANGUAGE_GERMAN)
	{
		SpawnRandomGibs(pevVictim, cGibs, "models/germangibs.mdl", GERMAN_GIB_COUNT, 0);
	}
	else
	{
		if (human)
		{
			// human pieces
			// starts at one to avoid throwing random amounts of skulls (0th gib)
			SpawnRandomGibs(pevVictim, cGibs, "models/hgibs.mdl", GERMAN_GIB_COUNT, 1);
		}
		else
		{
			// aliens
			SpawnRandomGibs(pevVictim, cGibs, "models/agibs.mdl", ALIEN_GIB_COUNT, 1);
		}
	}
}


//=========================================================
// WaitTillLand - in order to emit their meaty scent from
// the proper location, gibs should wait until they stop 
// bouncing to emit their scent. That's what this function
// does.
//=========================================================
void CGib::WaitTillLand(void)
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if (pev->velocity == g_vecZero)
	{
		SetThink(&CGib::SUB_StartFadeOut);
		pev->nextthink = gpGlobals->time + m_lifeTime;

		// If you bleed, you stink!
		if (m_bloodColor != DONT_BLEED)
		{
			// ok, start stinkin!
			CSoundEnt::InsertSound(bits_SOUND_MEAT, pev->origin, 384, 25);
		}
	}
	else
	{
		// wait and check again in another half second.
		pev->nextthink = gpGlobals->time + 0.5;
	}
}

//
// Gib bounces on the ground or wall, sponges some blood down, too!
//
void CGib::BounceGibTouch(CBaseEntity* pOther)
{
	Vector	vecSpot;
	TraceResult	tr;

	//if ( RANDOM_LONG(0,1) )
	//	return;// don't bleed everytime

	if (pev->flags & FL_ONGROUND)
	{
		pev->velocity = pev->velocity * 0.9;
		pev->angles.x = 0;
		pev->angles.z = 0;
		pev->avelocity.x = 0;
		pev->avelocity.z = 0;
	}
	else
	{
		if (g_Language != LANGUAGE_GERMAN && m_cBloodDecals > 0 && m_bloodColor != DONT_BLEED)
		{
			vecSpot = pev->origin + Vector(0, 0, 8);//move up a bit, and trace down.
			UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -24), ignore_monsters, ENT(pev), &tr);

			UTIL_BloodDecalTrace(&tr, m_bloodColor);

			m_cBloodDecals--;
		}

		if (m_material != matNone && RANDOM_LONG(0, 2) == 0)
		{
			float volume;
			float zvel = fabs(pev->velocity.z);

			volume = 0.8 * V_min(1.0, ((float)zvel) / 450.0);

			CBreakable::MaterialSoundRandom(edict(), (Materials)m_material, volume);
		}
	}
}

//
// Sticky gib puts blood on the wall and stays put. 
//
void CGib::StickyGibTouch(CBaseEntity* pOther)
{
	Vector	vecSpot;
	TraceResult	tr;

	SetThink(&CGib::SUB_Remove);
	pev->nextthink = gpGlobals->time + 10;

	if (!FClassnameIs(pOther->pev, "worldspawn"))
	{
		pev->nextthink = gpGlobals->time;
		return;
	}

	UTIL_TraceLine(pev->origin, pev->origin + pev->velocity * 32, ignore_monsters, ENT(pev), &tr);

	UTIL_BloodDecalTrace(&tr, m_bloodColor);

	pev->velocity = tr.vecPlaneNormal * -1;
	pev->angles = UTIL_VecToAngles(pev->velocity);
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
}

//
// Throw a chunk
//
void CGib::Spawn(const char* szGibModel)
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->friction = 0.55; // deading the bounce a bit

	// sometimes an entity inherits the edict from a former piece of glass,
	// and will spawn using the same render FX or rendermode! bad!
	pev->renderamt = 255;
	pev->rendermode = kRenderNormal;
	pev->renderfx = kRenderFxNone;
	pev->solid = SOLID_SLIDEBOX;/// hopefully this will fix the VELOCITY TOO LOW crap
	pev->classname = MAKE_STRING("gib");

	SET_MODEL(ENT(pev), szGibModel);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pev->nextthink = gpGlobals->time + 4;
	m_lifeTime = 25;
	SetThink(&CGib::WaitTillLand);
	SetTouch(&CGib::BounceGibTouch);

	m_material = matNone;
	m_cBloodDecals = 5;// how many blood decals this gib can place (1 per bounce until none remain). 
}
