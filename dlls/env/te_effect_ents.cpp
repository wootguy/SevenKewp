#include "extdll.h"
#include "util.h"
#include "te_effects.h"
#include "hlds_hooks.h"

// This file is a collection of workarounds for TE_* effects that don't work past the +/-4096 boundary.
// Standard entities are used to somewhat accurately recreate the effects. These use drastically more
// network data but they work anywhere (within the limits of delta.lst). Not everything can be recreated
// but this is much better than nothing. Combat should mostly feel normal. Ideally new network messsages
// would be created to handle larger coordinates, but I want to maintain compatibility with vanilla HL.
// 
// TODO:
// - player sprays (impossible? Can't be applied to precached BSPs)
// - TE_BUBBLETRAIL (underwater bullets)
// - crosshairs shake a bit beyond 8192, and a lot near 32768 (how tf can origin affect that?)

#define TE_EXPLOSION_SPR "sprites/zerogxplode.spr"

#define TE_TRACER_SPR "sprites/white.spr"
#define TE_TRACER_LEN 512
#define TE_TRACER_SPEED 5500

#define TE_BLOODSPRITE_SPR1 "sprites/bloodspray.spr"
#define TE_BLOODSPRITE_SPR2 "sprites/blood.spr"

#define TE_DECAL_SPR "sprites/te_decal.spr"

#define TE_SPARK_SPR "sprites/richo1.spr"
#define TE_SPARK_SPR2 "sprites/wsplash3.spr"

const char* g_teExplosionSounds[] = {
	"weapons/explode3.wav",
	"weapons/explode4.wav",
	"weapons/explode5.wav"
};


// for comparing real TE effects to the entity versions
class CTempEffect : public CBaseEntity
{
public:
	void Spawn(void) {
		Precache();

		SetThink(&CTempEffect::test_think);
		pev->nextthink = gpGlobals->time + 1.0f;
	}

	void Precache(void) {
		UTIL_PrecacheOther("te_user_tracer");
		UTIL_PrecacheOther("te_bloodpsrite");
		UTIL_PrecacheOther("te_decal");
		UTIL_PrecacheOther("te_explosion");
		UTIL_PrecacheOther("te_smoke");
		UTIL_PrecacheOther("te_sparks");
		UTIL_PrecacheOther("te_ricochet");
	}

	void test_think() {
		/*
		Vector start = pev->origin;
		Vector end = start + Vector(0, -3000, 0);

		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TRACER);
		WRITE_COORD(start.x);
		WRITE_COORD(start.y);
		WRITE_COORD(start.z);
		WRITE_COORD(end.x);
		WRITE_COORD(end.y);
		WRITE_COORD(end.z);
		MESSAGE_END();

		start = pev->origin + Vector(0, 0, 256);
		end = start + Vector(0, -3000, 0);
		CBaseEntity::Create("te_user_tracer", start, end, true);
		*/

		/*
		float radius = 500;
		int w = 25;
		UTIL_BeamCylinder(pev->origin, radius, MODEL_INDEX("sprites/white.spr"), 0, 0, 8, w, 0, RGBA(0, 255, 0, 128), 0);

		CTeBeamRing* ring = (CTeBeamRing*)CBaseEntity::Create("te_beamring", pev->origin, g_vecZero, true);
		ring->Expand(radius, MODEL_INDEX("sprites/white.spr"), 0, 0, 8, w, 0, RGBA(255, 0, 0, 128), 0, MSG_PVS, pev->origin, NULL);
		*/

		Vector dir = Vector(2, 0, 0);
		uint8_t speed = 100;
		uint8_t noise = 0;
		UTIL_SpriteSpray(pev->origin, dir, MODEL_INDEX("sprites/mommaspout.spr"), 1, speed, noise, false);
		UTIL_SpriteSpray(pev->origin + Vector(0, 32, 0), dir, MODEL_INDEX("sprites/mommaspout.spr"), 1, speed, noise, true);

		pev->nextthink = gpGlobals->time + 1.0f;
	}
};


class CTeUserTracer : public CBeam
{
public:
	Vector endPos;
	Vector startPos;
	Vector dir;
	float len;

	void Spawn(void) {
		Precache();
		CBeam::Spawn();

		startPos = pev->origin;
		endPos = pev->angles;
		len = (endPos - startPos).Length();
		dir = (endPos - pev->origin).Normalize();

		BeamInit(TE_TRACER_SPR, 10);
		PointsInit(startPos + dir * TE_TRACER_LEN, startPos);
		SetEndAttachment(1);
		SetColor(204, 204, 102);
		SetBrightness(100);
		SetScrollRate(1);
		SetFlags(BEAM_FSHADEOUT);

		// interpolation doesn't work with beams.
		// it's too expensive to network 3 entities per tracer for attachments, so just deal with the stutter
		//pev->movetype = MOVETYPE_NOCLIP;
		//pev->velocity = Vector(0, 0, 0.1f); // smoothes movement

		SetThink(&CTeUserTracer::Think);
		pev->nextthink = gpGlobals->time + 0.0f;
	}

	void Precache(void) {
		PRECACHE_MODEL(TE_TRACER_SPR);
	}

	void Think() {
		pev->origin = pev->origin + dir * TE_TRACER_SPEED * gpGlobals->frametime;
		pev->angles = pev->angles + dir * TE_TRACER_SPEED * gpGlobals->frametime;

		float headDist = (pev->origin - startPos).Length();
		float tailDist = (pev->angles - startPos).Length();

		if (tailDist > len) {
			// tail reached the end
			UTIL_Remove(this);
		}
		if (headDist > len) {
			// stop at target and wait for tail to catch up
			pev->origin = endPos;
		}

		pev->nextthink = gpGlobals->time + 0.0f;
	}
};


class CTeBloodSprite : public CSprite
{
public:
	void Spawn(void) {
		Precache();

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;
		pev->framerate = 30;

		SET_MODEL(edict(), TE_BLOODSPRITE_SPR1);

		pev->angles = Vector(0, 0, RANDOM_LONG(-180, 180));

		m_lastTime = gpGlobals->time;
		m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;

		SetThink(&CTeBloodSprite::Think);
		pev->nextthink = gpGlobals->time + 0.02f;
	}

	void Precache(void) {
		PRECACHE_MODEL(TE_BLOODSPRITE_SPR1);
		PRECACHE_MODEL(TE_BLOODSPRITE_SPR2);
	}

	void Think() {
		pev->frame += pev->framerate * (gpGlobals->time - m_lastTime);
		if (pev->frame > m_maxFrame) {
			UTIL_Remove(this);
		}

		m_lastTime = gpGlobals->time;
		pev->nextthink = gpGlobals->time + 0.02f;
	}
};


class CTeBloodSpriteDrip : public CBaseEntity
{
public:
	void Spawn(void) {
		Precache();

		pev->solid = SOLID_TRIGGER;
		pev->movetype = MOVETYPE_BOUNCE;
		pev->friction = 1.0f;
		pev->gravity = 0.8f;
		pev->velocity = Vector(RANDOM_FLOAT(-1, 1) * 100, RANDOM_FLOAT(-1, 1) * 100, RANDOM_FLOAT(0, 1) * 100);
		pev->angles = Vector(0, 0, RANDOM_LONG(-180, 180));

		SET_MODEL(edict(), TE_BLOODSPRITE_SPR2);

		SetTouch(&CTeBloodSpriteDrip::DripTouch);

		UTIL_SetSize(pev, g_vecZero, g_vecZero);

		SetThink(&CTeBloodSpriteDrip::SUB_Remove);
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(1.0f, 2.0f);
	}

	void Precache(void) {
		PRECACHE_MODEL(TE_BLOODSPRITE_SPR2);
	}

	void DripTouch(CBaseEntity* other) {
		if (other->IsBSPModel()) {
			pev->velocity = g_vecZero;
			pev->avelocity = g_vecZero;
		}
	}
};


class CTeExplosion : public CSprite
{
public:
	float m_dlightEnd;
	EHANDLE h_dlight;

	void Spawn(void) {
		Precache();

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_FLY; // no interp so that it appears faster
		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 200;
		
		pev->velocity = Vector(0, 0, 8);

		SET_MODEL(edict(), TE_EXPLOSION_SPR);

		m_lastTime = gpGlobals->time;
		m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;

		SetThink(&CTeExplosion::Think);
		pev->nextthink = gpGlobals->time + 0.02f;

		pev->origin = pev->origin + Vector(0, 0, 10);

		if (pev->spawnflags & TE_EXPLFLAG_NOADDITIVE) {
			pev->rendermode = 0;
		}

		if (!(pev->spawnflags & TE_EXPLFLAG_NODLIGHTS)) {
			h_dlight = Create("te_target", pev->origin, g_vecZero);
			h_dlight->pev->effects |= EF_DIMLIGHT;
			m_dlightEnd = gpGlobals->time + 0.5f;
		}

		if (!(pev->spawnflags & TE_EXPLFLAG_NOSOUND)) {
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, g_teExplosionSounds[RANDOM_LONG(0, 2)], 1.0f, 0.4f, 0, 100);
		}
	}

	void Precache(void) {
		PRECACHE_MODEL(TE_EXPLOSION_SPR);
		PRECACHE_SOUND(g_teExplosionSounds[0]);
		PRECACHE_SOUND(g_teExplosionSounds[1]);
		PRECACHE_SOUND(g_teExplosionSounds[2]);
	}

	void Think() {
		pev->frame += pev->framerate * (gpGlobals->time - m_lastTime);
		if (pev->frame > m_maxFrame) {
			UTIL_Remove(this);
		}

		if (m_dlightEnd) {
			h_dlight->pev->origin.z += 4;

			if (gpGlobals->time > m_dlightEnd) {
				m_dlightEnd = 0;
				UTIL_Remove(h_dlight);
			}
		}

		m_lastTime = gpGlobals->time;
		pev->nextthink = gpGlobals->time + 0.02f;
	}
};


class CTeSmoke : public CSprite
{
public:
	void Spawn(void) {
		Precache();

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_FLY; // no interp so that it appears faster
		pev->framerate = 12;
		pev->rendermode = kRenderTransAlpha;
		pev->renderamt = 220;
		pev->velocity = Vector(0, 0, 30);

		SET_MODEL(edict(), STRING(pev->model));

		m_lastTime = gpGlobals->time;
		m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;

		SetThink(&CTeSmoke::Think);
		pev->nextthink = gpGlobals->time + 0.02f;
	}

	void Precache(void) {
		PRECACHE_MODEL(STRING(pev->model));
	}

	void Think() {
		pev->frame += pev->framerate * (gpGlobals->time - m_lastTime);
		
		if (pev->frame > m_maxFrame) {
			UTIL_Remove(this);
		}

		m_lastTime = gpGlobals->time;
		pev->nextthink = gpGlobals->time + 0.02f;
	}
};


class CTeDecal : public CBaseEntity
{
public:
	EHANDLE h_parent;
	Vector m_selfStartOrigin;
	Vector m_attachStartOrigin;
	Vector m_selfStartDir;
	float deathTime;

	void Spawn(void) {
		Precache();

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE; // appear fast
		pev->framerate = 0;
		pev->rendermode = kRenderTransAlpha;
		pev->renderamt = 255;
		pev->rendercolor = Vector(1, 1, 1);

		const char* spr = TE_DECAL_SPR;
		pev->frame = clampi(pev->skin, 0, DECAL_MOMMABIRTH);

		switch (pev->skin) {
		case DECAL_LAMBDA1:
		case DECAL_LAMBDA2:
		case DECAL_LAMBDA3:
		case DECAL_LAMBDA4:
		case DECAL_LAMBDA5:
		case DECAL_LAMBDA6:
			pev->rendercolor = Vector(255, 127, 0);
			break;
		case DECAL_BLOOD1:
		case DECAL_BLOOD2:
		case DECAL_BLOOD3:
		case DECAL_BLOOD4:
		case DECAL_BLOOD5:
		case DECAL_BLOOD6:
			pev->rendercolor = Vector(36, 0, 0);
			break;
		case DECAL_YBLOOD1:
		case DECAL_YBLOOD2:
		case DECAL_YBLOOD3:
		case DECAL_YBLOOD4:
		case DECAL_YBLOOD5:
		case DECAL_YBLOOD6:
			pev->rendercolor = Vector(204, 184, 97);
			break;
		case DECAL_GLASSBREAK1:
		case DECAL_GLASSBREAK2:
		case DECAL_GLASSBREAK3:
		case DECAL_BPROOF1:
		case DECAL_MOMMABIRTH:
		case DECAL_MOMMASPLAT:
			pev->rendercolor = Vector(255, 255, 255);
			break;
		case DECAL_SPIT1:
		case DECAL_SPIT2:
			pev->rendercolor = Vector(204, 228, 40);
			break;
		}

		SET_MODEL(edict(), spr);

		deathTime = gpGlobals->time + 10.0f + RANDOM_FLOAT(0, 1.0f);

		if (pev->iuser1) {
			h_parent = INDEXENT(pev->iuser1);
			m_attachStartOrigin = h_parent->pev->origin;
			//m_selfStartOrigin = pev->origin;
			//m_selfStartDir = pev->v_angle;

			m_selfStartOrigin = UTIL_UnwindPoint(pev->origin - h_parent->pev->origin, -h_parent->pev->angles) + h_parent->pev->origin;
			m_selfStartDir = UTIL_UnwindPoint(pev->v_angle, -h_parent->pev->angles);

			SetThink(&CTeDecal::AttachThink);
			pev->nextthink = gpGlobals->time + 0.05f;
		}
		else {
			SetThink(&CTeExplosion::SUB_FadeOut);
			pev->nextthink = deathTime;
		}
	}

	void AttachThink() {
		if (!h_parent) {
			UTIL_Remove(this);
			return;
		}

		pev->movetype = MOVETYPE_NOCLIP; // interpolate

		MAKE_VECTORS(h_parent->pev->angles);

		// rotate position around target
		Vector newOri = m_selfStartOrigin + (h_parent->pev->origin - m_attachStartOrigin);
		pev->origin = UTIL_RotatePoint(newOri - h_parent->pev->origin, -h_parent->pev->angles) + h_parent->pev->origin;

		// rotate orientation around target
		Vector newDir = UTIL_RotatePoint(m_selfStartDir, -h_parent->pev->angles);
		pev->angles = UTIL_VecToSpriteAngles(newDir);

		// TODO: this isn't enough for complex rotating parents
		pev->angles.z = -h_parent->pev->angles.y;

		if (gpGlobals->time > deathTime) {
			SUB_FadeOut();
		}

		pev->nextthink = gpGlobals->time + 0.05f;
	}

	void Precache(void) {
		PRECACHE_MODEL(TE_DECAL_SPR);
	}
};


class CTeSparks : public CSprite
{
public:
	void Spawn(void) {
		Precache();

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;
		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 200;
		pev->angles.z = RANDOM_FLOAT(-180, 180);

		SET_MODEL(edict(), TE_SPARK_SPR);

		SetThink(&CTeSparks::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.05f;

		CSprite* streaks = CSprite::SpriteCreate(TE_SPARK_SPR2, pev->origin + Vector(0,0,8), true);
		streaks->pev->rendermode = kRenderTransAdd;
		streaks->pev->renderamt = 200;
		streaks->pev->spawnflags = SF_SPRITE_ONCE_AND_REMOVE;
		streaks->pev->frame = RANDOM_LONG(0,1);
		streaks->pev->framerate = RANDOM_LONG(25, 35);
		streaks->pev->scale = RANDOM_FLOAT(0.8f, 1.2f);
		streaks->pev->movetype = MOVETYPE_NONE; // appears faster without interp
	}

	void Precache(void) {
		PRECACHE_MODEL(TE_SPARK_SPR);
		PRECACHE_MODEL(TE_SPARK_SPR2);
	}
};


class CTeRicochet : public CSprite
{
public:
	void Spawn(void) {
		Precache();

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;
		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 200;
		pev->angles.z = RANDOM_FLOAT(-180, 180);

		SET_MODEL(edict(), TE_SPARK_SPR);

		SetThink(&CTeRicochet::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.05f;

		UTIL_PlayRicochetSound(edict());
	}

	void Precache(void) {
		PRECACHE_MODEL(TE_SPARK_SPR);
		PRECACHE_SOUND_ARRAY(g_teRicochetSounds);
	}
};

// an invisible networked entity that you can attach effects to
class CTeTarget : public CBaseEntity
{
public:
	void Spawn(void) {
		Precache();

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;
		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 0;

		SET_MODEL(edict(), "models/player.mdl");
	}
};


void CTeBeamRing::Spawn(void) {
	Precache();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 0;
	SET_MODEL(edict(), "models/player.mdl");

	h_target = Create("te_target", pev->origin, g_vecZero, true);
}

void CTeBeamRing::Expand(float radius, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life,
	uint8_t width, uint8_t noise, RGBA color, uint8_t speed, int msgMode, const float* msgOrigin, edict_t* targetEnt) {

	UTIL_BeamEnts(entindex(), 0, h_target->entindex(), 0, true, modelIdx, frameStart, framerate, life,
		width, noise, color, speed, msgMode, msgOrigin, targetEnt);

	SetThink(&CTeBeamRing::ExpandThink);
	pev->nextthink = gpGlobals->time + 0.02f;
	m_lastExpand = gpGlobals->time;
	m_deathTime = gpGlobals->time + life * 0.1f;
	m_expandSpeed = radius;
}

void CTeBeamRing::ExpandThink() {
	if (!h_target || gpGlobals->time > m_deathTime) {
		UTIL_Remove(h_target);
		UTIL_Remove(this);
		return;
	}

	Vector expandDir = Vector(1, 0, 0);
	float expandAmt = m_expandSpeed * (gpGlobals->time - m_lastExpand);

	pev->origin = pev->origin + expandDir * -expandAmt;
	h_target->pev->origin = h_target->pev->origin + expandDir * expandAmt;

	m_lastExpand = gpGlobals->time;
	pev->nextthink = gpGlobals->time + 0.02f;
}


class CSplashWake : public CSprite
{
public:
	int life;
	float m_splashTime;

	void Spawn(void) {
		Precache();

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_FLY; // no interp so that it appears faster
		pev->rendermode = kRenderTransAlpha;

		if (!pev->scale)
			pev->scale = 1.0f;

		pev->angles = Vector(90, 0, RANDOM_LONG(0, 359));
		pev->avelocity.z = RANDOM_LONG(-5, 5) * 2;

		SET_MODEL(edict(), INDEX_MODEL(g_waterSplashWakeSpr));

		m_lastTime = gpGlobals->time;
		m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;

		SetThink(&CSplashWake::Think);

		pev->renderamt = 0;

		m_splashTime = gpGlobals->time;

		if (pev->iuser1) {
			m_splashTime += 0.1f + pev->iuser1 * 0.001f;
		}

		pev->nextthink = gpGlobals->time + 0.02f;
	}

	void Precache(void) {
	}

	void Think() {
		if (m_splashTime) {
			if (gpGlobals->time > m_splashTime) {
				m_splashTime = 0;

				float ratio = pev->scale;
				Vector offset = Vector(0, 0, (64 * ratio) - 16);
				int fps = 20 - ratio * 2;
				int pitch = 70 - ratio * 5;

				UTIL_Explosion(pev->origin + offset, g_waterSplashSpr, pev->scale * 10, fps, 1 | 2 | 4 | 8);

				EMIT_SOUND_DYN(edict(), CHAN_BODY, WATER_SPLASH2_SND_PATH, 1.0f, ATTN_NORM, 0, RANDOM_LONG(95, 105));
				EMIT_SOUND_DYN(edict(), CHAN_ITEM, RANDOM_SOUND_ARRAY(g_waterSplashSounds), 1.0f, ATTN_NORM, 0, pitch);

				pev->renderamt = 200;
			}
			else {
				pev->renderamt += 10;
			}
		}

		pev->scale += 0.01f;

		if (life++ >= 40)
			pev->renderamt -= 4;

		if (pev->renderamt <= 0) {
			UTIL_Remove(this);
		}

		m_lastTime = gpGlobals->time;
		pev->nextthink = gpGlobals->time + 0.02f;
	}
};

LINK_ENTITY_TO_CLASS(te_effect, CTempEffect)
LINK_ENTITY_TO_CLASS(te_user_tracer, CTeUserTracer)
LINK_ENTITY_TO_CLASS(te_bloodsprite, CTeBloodSprite)
LINK_ENTITY_TO_CLASS(te_bloodsprite_drip, CTeBloodSpriteDrip)
LINK_ENTITY_TO_CLASS(te_explosion, CTeExplosion)
LINK_ENTITY_TO_CLASS(te_smoke, CTeSmoke)
LINK_ENTITY_TO_CLASS(te_decal, CTeDecal)
LINK_ENTITY_TO_CLASS(te_sparks, CTeSparks)
LINK_ENTITY_TO_CLASS(te_ricochet, CTeRicochet)
LINK_ENTITY_TO_CLASS(te_beamring, CTeBeamRing)
LINK_ENTITY_TO_CLASS(te_target, CTeTarget)

LINK_ENTITY_TO_CLASS(splashwake, CSplashWake)
