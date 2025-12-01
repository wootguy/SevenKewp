#pragma once
#include "extdll.h"
#include "util.h"
#include "shared_effects.h"
#include "rgb.h"
#include "CBaseEntity.h"

EXPORT extern DLL_GLOBAL short g_sModelIndexSmoke;

EXPORT RGB GetTeColor(uint8_t color);

EXPORT void UTIL_PlayRicochetSound(edict_t* ent);

EXPORT void UTIL_PrecacheLargeWorldEffects();

EXPORT void	UTIL_BloodStream(const Vector& origin, const Vector& direction, int color, int amount);
EXPORT void	UTIL_BloodDrips(const Vector& origin, const Vector& direction, int color, int amount);
EXPORT void	UTIL_Bubbles(Vector mins, Vector maxs, int count);
EXPORT void	UTIL_BubbleTrail(Vector from, Vector to, int count);
EXPORT void	UTIL_WaterSplash(Vector pos, bool bigSplash, bool playSound, float scale=1.0f, edict_t* skipEnt=NULL); // create a water splash if close enough to the surface
EXPORT void	UTIL_WaterSplashFootstep(int player_index); // create a water splash effect for footsteps
EXPORT void UTIL_BeamFollow(int entindex, int modelIdx, int life, int width, RGBA color, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);
EXPORT void UTIL_Fizz(int eidx, int modelIdx, uint8_t density, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);
EXPORT void UTIL_DLight(Vector pos, uint8_t radius, RGB color, uint8_t time, uint8_t decay);
EXPORT void UTIL_ELight(int entindex, int attachment, Vector origin, float radius, RGBA color, int life, float decay, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);
EXPORT void UTIL_BeamPoints(Vector start, Vector end, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);
EXPORT void UTIL_BeamEntPoint(int entindex, int attachment, Vector point, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);
EXPORT void UTIL_BeamEnts(int startEnt, int startAttachment, int endEnt, int endAttachment, bool ringMode, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);
EXPORT void UTIL_KillBeam(int entindex, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);
EXPORT void UTIL_BSPDecal(int entindex, Vector origin, int decalIdx, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);
EXPORT void UTIL_PlayerDecal(int entindex, int playernum, Vector origin, int decalIdx, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);
EXPORT void UTIL_GunshotDecal(int entindex, Vector origin, int decalIdx, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);

// Important: Don't send reliable decals to connecting clients or else they crash with:
// "Used decal #X without a name"
// MSG_ALL will send to connecting clients
EXPORT void UTIL_Decal(int entindex, Vector origin, int decalIdx, int msgMode = MSG_BROADCAST, const float* msgOrigin = NULL, edict_t* targetEnt = NULL);

EXPORT void	UTIL_BloodDecalTrace(TraceResult* pTrace, int bloodColor);
EXPORT edict_t* UTIL_DecalTrace(TraceResult* pTrace, int decalNumber, int msgMode=MSG_BROADCAST, edict_t* targetEnt=NULL);
EXPORT void	UTIL_PlayerDecalTrace(TraceResult* pTrace, int playernum, int decalNumber, BOOL bIsCustom);
// emitter = If set, impact effects will be sent to all HL clients except this player (otherwise all players)
EXPORT edict_t* UTIL_GunshotDecalTrace(TraceResult* pTrace, int decalNumber, edict_t* emitter=NULL);
EXPORT void	UTIL_Sparks(const Vector& position);
EXPORT void	UTIL_Ricochet(const Vector& position, float scale);
EXPORT void	UTIL_Sprite(const Vector& position, int sprIndex, uint8_t scale, uint8_t opacity);
EXPORT void	UTIL_BreakModel(const Vector& pos, const Vector& size, const Vector& velocity,
							uint8_t noise, int modelIdx, uint8_t shards, uint8_t duration, uint8_t flags);
EXPORT void UTIL_SpriteSpray(Vector pos, Vector dir, int spriteIdx, uint8_t count, uint8_t speed, uint8_t noise, bool test=false);
EXPORT void	UTIL_Shrapnel(Vector pos, Vector dir, float flDamage, int bitsDamageType);
EXPORT void UTIL_Tracer(Vector start, Vector end, int msgMode = MSG_PAS, edict_t* targetEnt = NULL);
EXPORT void UTIL_Explosion(Vector origin, int sprIndex, uint8_t scale, uint8_t framerate, uint8_t flags);
EXPORT void UTIL_Smoke(Vector origin, int sprIndex, uint8_t scale, uint8_t framerate);
EXPORT void UTIL_BeamCylinder(Vector pos, float radius, int modelIdx, uint8_t startFrame, uint8_t frameRate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t scrollSpeed);
EXPORT void UTIL_BeamTorus(Vector pos, float radius, int modelIdx, uint8_t startFrame, uint8_t frameRate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t scrollSpeed);
EXPORT void UTIL_BeamDisk(Vector pos, float radius, int modelIdx, uint8_t startFrame, uint8_t frameRate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t scrollSpeed);

EXPORT void EjectBrass(const Vector& vecOrigin, const Vector& vecVelocity, float rotation, int model, int soundtype, edict_t* target=NULL);
// emitter = If set, impact effects will be sent to all clients except this player (otherwise all players)
EXPORT void DecalGunshot(TraceResult* pTrace, int iBulletType, bool playTextureSound = false, Vector vecSrc = g_vecZero, Vector vecEnd = g_vecZero, edict_t* emitter=NULL);
EXPORT void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
EXPORT int DamageDecal(CBaseEntity* pEntity, int bitsDamageType);

// create a new entity which plays a sound and then kills itself. Not for use with looping sounds.
EXPORT void UTIL_TempSound(Vector pos, const char* sample, float volume=1.0f, float attenuation=ATTN_NORM, int flags=0, int pitch=100);

// advanced temporary sprite
EXPORT void UTIL_SpriteAdv(Vector pos, const SpriteAdvArgs& args, int msgMode=MSG_BROADCAST, const float* msgOrigin=NULL, edict_t* targetEnt=NULL);

// playSound: 1 = slow moving object, 2 = bullet, 3 = obnoxious player
// skipEnt: don't send message to this player (for predicted splashes)
EXPORT void UTIL_WaterSplashMsg(Vector pos, float scale, int playSound, edict_t* skipEnt=NULL);

class CTeBeamRing : public CBaseEntity
{
public:
	EHANDLE h_target;
	float m_lastExpand;
	float m_expandSpeed;
	float m_deathTime;

	void Spawn(void);
	void Expand(float radius, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life, uint8_t width,
		uint8_t noise, RGBA color, uint8_t speed, int msgMode, const float* msgOrigin, edict_t* targetEnt);
	void ExpandThink();
};