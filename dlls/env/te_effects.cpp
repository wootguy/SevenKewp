#include "extdll.h"
#include "util.h"
#include "te_effects.h"
#include "hlds_hooks.h"

RGB GetTeColor(uint8_t color) {
	// palette used by TE_* effects
	static uint32_t colors[] = {
		0x000000, 0x0F0F0F, 0x1F1F1F, 0x2F2F2F, 0x3F3F3F, 0x4B4B4B, 0x5B5B5B, 0x6B6B6B,
		0x7B7B7B, 0x8B8B8B, 0x9B9B9B, 0xABABAB, 0xBBBBBB, 0xCBCBCB, 0xDBDBDB, 0xEBEBEB,
		0x0F0B07, 0x170F0B, 0x1F170B, 0x271B0F, 0x2F2313, 0x372B17, 0x3F2F17, 0x4B371B,
		0x533B1B, 0x5B431F, 0x634B1F, 0x6B531F, 0x73571F, 0x7B5F23, 0x836723, 0x8F6F23,
		0x0B0B0F, 0x13131B, 0x1B1B27, 0x272733, 0x2F2F3F, 0x37374B, 0x3F3F57, 0x474767,
		0x4F4F73, 0x5B5B7F, 0x63638B, 0x6B6B97, 0x7373A3, 0x7B7BAF, 0x8383BB, 0x8B8BCB,
		0x000000, 0x070700, 0x0B0B00, 0x131300, 0x1B1B00, 0x232300, 0x2B2B07, 0x2F2F07,
		0x373707, 0x3F3F07, 0x474707, 0x4B4B0B, 0x53530B, 0x5B5B0B, 0x63630B, 0x6B6B0F,
		0x070000, 0x0F0000, 0x170000, 0x1F0000, 0x270000, 0x2F0000, 0x370000, 0x3F0000,
		0x470000, 0x4F0000, 0x570000, 0x5F0000, 0x670000, 0x6F0000, 0x770000, 0x7F0000,
		0x131300, 0x1B1B00, 0x232300, 0x2F2B00, 0x372F00, 0x433700, 0x4B3B07, 0x574307,
		0x5F4707, 0x6B4B0B, 0x77830F, 0x83731F, 0x8B5F1F, 0x977F27, 0xA3632F, 0xAF6723,
		0x231307, 0x2F170B, 0x3B1F0F, 0x4B2313, 0x572B17, 0x632F1F, 0x733723, 0x7F3B2B,
		0x8F4333, 0x9F4F33, 0xAF632F, 0xBF7737, 0xCF8F2B, 0xDFAB27, 0xEFCB1F, 0xFFF31B,
		0x0B0700, 0x1B1300, 0x2B230F, 0x372B13, 0x47331B, 0x533723, 0x633F2B, 0x6F4733,
		0x7F533F, 0x8B5F47, 0x9B6B53, 0xA77B5F, 0xB7876B, 0xC3977B, 0xD3A38B, 0xE3B39B,
		0xAB8BA3, 0x9F7F97, 0x938787, 0x8B677B, 0x7F5B6F, 0x775F63, 0x6B4B57, 0x5F3F4B,
		0x573747, 0x4B2F37, 0x43272F, 0x372327, 0x2B171B, 0x231313, 0x170B0B, 0x0F0707,
		0xBB739F, 0xAF6B8F, 0xA35F83, 0x97757B, 0x8B4F6F, 0x7F4B5F, 0x734B57, 0x6B3B4B,
		0x5F2F3F, 0x532737, 0x47232B, 0x3B1F27, 0x2F171B, 0x231313, 0x170B0B, 0x0F0707,
		0xDBC3BB, 0xCBB3A7, 0xBF9F9B, 0xAF978B, 0xA3877B, 0x977B6F, 0x876B5F, 0x7B6353,
		0x6B5747, 0x5F4B3B, 0x534033, 0x432F27, 0x372B1F, 0x271F17, 0x1B130F, 0x0F0B07,
		0x6F837B, 0x677B6F, 0x5F7367, 0x576B5F, 0x4F6357, 0x476B4F, 0x3F5747, 0x375B3F,
		0x2F4737, 0x2B3B2F, 0x234033, 0x1F372B, 0x172B23, 0x0F231F, 0x0B1313, 0x070B07,
		0xFFF31B, 0xEFE717, 0xDBCB13, 0xCBB70F, 0xBBA70F, 0xAB970B, 0x9B8307, 0x8B7307,
		0x7B6307, 0x6B5300, 0x5B4700, 0x4B3700, 0x3B2B00, 0x2B1F00, 0x1B0F00, 0x0B0700,
		0x0000FF, 0x0B0BEF, 0x1313DF, 0x1B1BCF, 0x2323BF, 0x2B2BAF, 0x2F2F9F, 0x2F2F8F,
		0x2F2F7F, 0x2F2F6F, 0x2F2F5F, 0x2B2B4F, 0x23233F, 0x1B1B2F, 0x13131F, 0x0B0B0F,
		0x2B0000, 0x3B0000, 0x4B0700, 0x5F0700, 0x6F0F00, 0x7F1707, 0x931F07, 0xA3270B,
		0xB7330F, 0xC34B1B, 0xCF632B, 0xDB7F3B, 0xE3974F, 0xE7AB5F, 0xEFCB77, 0xF7D38B,
		0xA77B3B, 0xB79737, 0xC7C737, 0xE7E757, 0x00FF00, 0xABE7FF, 0xD7FFFF, 0x670000,
		0x8B0000, 0xB30000, 0xD70000, 0xFF0000, 0xFFF393, 0xFFF7C7, 0xFFFFFF, 0x9F5B53
	};

	return colors[color];
}

const char* g_teRicochetSounds[5] = {
	"weapons/ric1.wav",
	"weapons/ric2.wav",
	"weapons/ric3.wav",
	"weapons/ric4.wav",
	"weapons/ric5.wav",
};

void UTIL_PrecacheLargeWorldEffects() {
	UTIL_PrecacheOther("te_user_tracer");
	UTIL_PrecacheOther("te_bloodsprite");
	UTIL_PrecacheOther("te_bloodsprite_drip");
	UTIL_PrecacheOther("te_decal");
	UTIL_PrecacheOther("te_explosion");
	UTIL_PrecacheOther("te_smoke");
	UTIL_PrecacheOther("te_sparks");
	UTIL_PrecacheOther("te_ricochet");

	int count = ARRAYSIZE(g_teRicochetSounds);
	if (soundvariety.value > 0) {
		count = V_min(soundvariety.value, count);
	}
	for (int i = 0; i < count; i++)
		PRECACHE_SOUND_ENT(NULL, (char*)(g_teRicochetSounds)[i]);
}

void UTIL_PlayRicochetSound(edict_t* ent) {
	EMIT_SOUND_DYN(ent, CHAN_WEAPON, RANDOM_SOUND_ARRAY(g_teRicochetSounds), 1.0f, ATTN_NORM, 0, 100);
}

void te_debug_box(Vector mins, Vector maxs, uint8_t life, RGBA c, int msgType, edict_t* dest) {
	Vector corners[8];

	// Generate all 8 corners of the box
	corners[0] = Vector(mins.x, mins.y, mins.z);
	corners[1] = Vector(maxs.x, mins.y, mins.z);
	corners[2] = Vector(maxs.x, maxs.y, mins.z);
	corners[3] = Vector(mins.x, maxs.y, mins.z);
	corners[4] = Vector(mins.x, mins.y, maxs.z);
	corners[5] = Vector(maxs.x, mins.y, maxs.z);
	corners[6] = Vector(maxs.x, maxs.y, maxs.z);
	corners[7] = Vector(mins.x, maxs.y, maxs.z);

	// Bottom edges
	te_debug_beam(corners[0], corners[1], life, c, msgType, dest);
	te_debug_beam(corners[1], corners[2], life, c, msgType, dest);
	te_debug_beam(corners[2], corners[3], life, c, msgType, dest);
	te_debug_beam(corners[3], corners[0], life, c, msgType, dest);

	// Top edges
	te_debug_beam(corners[4], corners[5], life, c, msgType, dest);
	te_debug_beam(corners[5], corners[6], life, c, msgType, dest);
	te_debug_beam(corners[6], corners[7], life, c, msgType, dest);
	te_debug_beam(corners[7], corners[4], life, c, msgType, dest);

	// Vertical edges
	te_debug_beam(corners[0], corners[4], life, c, msgType, dest);
	te_debug_beam(corners[1], corners[5], life, c, msgType, dest);
	te_debug_beam(corners[2], corners[6], life, c, msgType, dest);
	te_debug_beam(corners[3], corners[7], life, c, msgType, dest);
}

void te_debug_beam(Vector start, Vector end, uint8_t life, RGBA c, int msgType, edict_t* dest)
{
	UTIL_BeamPoints(start, end, MODEL_INDEX("sprites/laserbeam.spr"), 0, 0, life, 16, 0,
		c, 0, msgType, NULL, dest);
}

// This logic makes sure that a network message does not get sent to
// a client that can't handle a high entity index, if one is given.
// entindex is assumed to be the entity used for PVS/PAS tests, if using that message mode.
// Required variadic args:
//    int msgMode = the network message mode
//    const float* msgOrigin = origin used by the engine for some message send modes (PVS/PAS)
//    edict_t* targetEnt = the target entity of the network message (NULL for broadcasts)
// NOTE: This code is mostly duplicated in UTIL_BeamEnts, which checks multiple indexes
#define SAFE_MESSAGE_ENT_LOGIC(msg_func, entindex, ...) \
	if (entindex < MAX_LEGACY_CLIENT_ENTS) { \
		msg_func(entindex, __VA_ARGS__); \
		return; \
	} \
	\
	if (msgMode == MSG_ONE || msgMode == MSG_ONE_UNRELIABLE) { \
		if (UTIL_isSafeEntIndex(targetEnt, entindex, __FUNCTION__)) { \
			msg_func(entindex, __VA_ARGS__); \
		} \
		return; \
	} \
	\
	int originalMsgMode = msgMode; /* saved in case PVS/PAS was passed, for testing later */ \
	msgMode = GetIndividualNetMessageMode(msgMode); /* sending individual messages instead of a broadcast */\
	msgOrigin = NULL; /* don't send this to the engine because PVS/PAS testing will be done here */ \
	for (int i = 1; i <= gpGlobals->maxClients; i++) { \
		targetEnt = INDEXENT(i); \
	\
		if (TestMsgVis(targetEnt, entindex, originalMsgMode) && UTIL_isSafeEntIndex(targetEnt, entindex, __FUNCTION__)) { \
			msg_func(entindex, __VA_ARGS__); \
		} \
	}

// tests if a player would receive a message given a send mode
// and the entity which is emitting the message
bool TestMsgVis(edict_t* plr, int testEntIdx, int netMsgMode) {
	CBaseEntity* ent = CBaseEntity::Instance(INDEXENT(testEntIdx));

	if (!ent) {
		return false;
	}

	switch (netMsgMode) {
	case MSG_PVS:
	case MSG_PVS_R:
		return ent->InPVS(plr);
	case MSG_PAS:
	case MSG_PAS_R:
		return ent->InPAS(plr);
	default:
		return true;
	}
}

// converts a broadcast message mode to an individual mode
// while preserving the reliable/unreliable channel selection
int GetIndividualNetMessageMode(int msgMode) {
	switch (msgMode) {
	case MSG_ALL:
	case MSG_PVS_R:
	case MSG_PAS_R:
	case MSG_ONE:
		return MSG_ONE;
	default:
		return MSG_ONE_UNRELIABLE;
	}
}

void UTIL_BeamFollow_msg(int entindex, int modelIdx, int life, int width, RGBA color, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex);
	WRITE_SHORT(modelIdx);
	WRITE_BYTE(life);
	WRITE_BYTE(width);
	WRITE_BYTE(color.r);
	WRITE_BYTE(color.g);
	WRITE_BYTE(color.b);
	WRITE_BYTE(color.a);
	MESSAGE_END();
}
void UTIL_BeamFollow(int entindex, int modelIdx, int life, int width, RGBA color, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_BeamFollow_msg, entindex, modelIdx, life, width, color, msgMode, msgOrigin, targetEnt);
}

void UTIL_Fizz_msg(int entindex, int modelIdx, uint8_t density, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_FIZZ);
	WRITE_SHORT(entindex);
	WRITE_SHORT(modelIdx);
	WRITE_BYTE(density);
	MESSAGE_END();
}
void UTIL_Fizz(int entindex, int modelIdx, uint8_t density, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_Fizz_msg, entindex, modelIdx, density, msgMode, msgOrigin, targetEnt);
}

void UTIL_ELight_msg(int entindex, int attachment, Vector origin, float radius, RGBA color, int life, float decay, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(entindex + (attachment << 12));
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_COORD(radius);
	WRITE_BYTE(color.r);
	WRITE_BYTE(color.g);
	WRITE_BYTE(color.b);
	WRITE_BYTE(life);
	WRITE_COORD(decay);
	MESSAGE_END();
}
void UTIL_ELight(int entindex, int attachment, Vector origin, float radius, RGBA color, int life, float decay, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_ELight_msg, entindex, attachment, origin, radius, color, life, decay, msgMode, msgOrigin, targetEnt);
}

void UTIL_BeamEntPoint_msg(int entindex, int attachment, Vector point, int modelIdx, uint8_t frameStart,
	uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed,
	int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_BEAMENTPOINT);
	WRITE_SHORT(entindex + (attachment << 12));
	WRITE_COORD(point.x);
	WRITE_COORD(point.y);
	WRITE_COORD(point.z);
	WRITE_SHORT(modelIdx);
	WRITE_BYTE(frameStart);
	WRITE_BYTE(framerate);
	WRITE_BYTE(life);
	WRITE_BYTE(width);
	WRITE_BYTE(noise);
	WRITE_BYTE(color.r);
	WRITE_BYTE(color.g);
	WRITE_BYTE(color.b);
	WRITE_BYTE(color.a);
	WRITE_BYTE(speed);
	MESSAGE_END();
}
void UTIL_BeamEntPoint(int entindex, int attachment, Vector point, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	if (UTIL_IsValidTempEntOrigin(point)) {
		SAFE_MESSAGE_ENT_LOGIC(UTIL_BeamEntPoint_msg, entindex, attachment, point, modelIdx, frameStart, framerate, life, width, noise, color, speed, msgMode, msgOrigin, targetEnt);
	}
	else {
		if (entindex < MAX_LEGACY_CLIENT_ENTS) {
			CBeam* beam = CBeam::BeamCreate(INDEX_MODEL(modelIdx), width);
			beam->PointEntInit(point, entindex);
			beam->SetEndAttachment(attachment);
			beam->SetFrame(frameStart);
			beam->SetNoise(noise);
			beam->SetColor(color.r, color.g, color.b);
			beam->SetBrightness(color.a);
			beam->SetScrollRate(speed);
			beam->LiveForTime(life * 0.1f);
			// TODO: framerate
		}
		else {
			// should probably do this anyway because beams attached to entities have caused crashes before
			edict_t* ent = INDEXENT(entindex);
			Vector attachOri, attachAngles;
			GET_ATTACHMENT(ent, attachment, attachOri, attachAngles);
			Vector entOri = ent->v.origin + attachOri;
			UTIL_BeamPoints(entOri, point, modelIdx, frameStart, framerate, life,
				width, noise, color, speed, msgMode, msgOrigin, targetEnt);
		}
	}
}

void UTIL_KillBeam_msg(int entindex, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_KILLBEAM);
	WRITE_SHORT(entindex);
	MESSAGE_END();
}
void UTIL_KillBeam(int entindex, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_KillBeam_msg, entindex, msgMode, msgOrigin, targetEnt);
}

void UTIL_BeamEnts_msg(int entindex, int attachment, int entindex2, int attachment2, bool ringMode, int modelIdx, uint8_t frameStart,
	uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed,
	int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(ringMode ? TE_BEAMRING : TE_BEAMENTS);
	WRITE_SHORT(entindex + (attachment << 12));
	WRITE_SHORT(entindex2 + (attachment2 << 12));
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(frameStart);
	WRITE_BYTE(framerate);
	WRITE_BYTE(life);
	WRITE_BYTE(width);
	WRITE_BYTE(noise);
	WRITE_BYTE(color.r);
	WRITE_BYTE(color.g);
	WRITE_BYTE(color.b);
	WRITE_BYTE(color.a);
	WRITE_BYTE(speed);
	MESSAGE_END();
}
void UTIL_BeamEnts(int startEnt, int startAttachment, int endEnt, int endAttachment, bool ringMode, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	// NOTE: This code is mostly duplicated in the SAFE_MESSAGE_ENT_LOGIC macro
	if (startEnt < MAX_LEGACY_CLIENT_ENTS && endEnt < MAX_LEGACY_CLIENT_ENTS) {
		UTIL_BeamEnts_msg(startEnt, startAttachment, endEnt, endAttachment, ringMode, modelIdx, frameStart,
			framerate, life, width, noise, color, speed, msgMode, msgOrigin, targetEnt);
		return;
	}

	if (msgMode == MSG_ONE || msgMode == MSG_ONE_UNRELIABLE) {
		if (UTIL_isSafeEntIndex(targetEnt, startEnt, __FUNCTION__) && UTIL_isSafeEntIndex(targetEnt, endEnt, __FUNCTION__)) {
			UTIL_BeamEnts_msg(startEnt, startAttachment, endEnt, endAttachment, ringMode, modelIdx, frameStart,
				framerate, life, width, noise, color, speed, msgMode, msgOrigin, targetEnt);
		}
		return;
	}

	int originalMsgMode = msgMode; /* saved in case PVS/PAS was passed, for testing later */
	msgMode = GetIndividualNetMessageMode(msgMode); /* sending individual messages instead of a broadcast */
	msgOrigin = NULL; /* don't send this to the engine because PVS/PAS testing will be done here */
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		targetEnt = INDEXENT(i);

		bool isVisible = TestMsgVis(targetEnt, startEnt, originalMsgMode) || TestMsgVis(targetEnt, endEnt, originalMsgMode);
		bool isSafeIndex = UTIL_isSafeEntIndex(targetEnt, startEnt, __FUNCTION__) && UTIL_isSafeEntIndex(targetEnt, endEnt, __FUNCTION__);

		if (isVisible && isSafeIndex) {
			UTIL_BeamEnts_msg(startEnt, startAttachment, endEnt, endAttachment, ringMode, modelIdx, frameStart,
				framerate, life, width, noise, color, speed, msgMode, msgOrigin, targetEnt);
		}
	}
}

void UTIL_BeamPoints(Vector start, Vector end, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	if (UTIL_IsValidTempEntOrigin(start) && UTIL_IsValidTempEntOrigin(end)) {
		MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
		WRITE_BYTE(TE_BEAMPOINTS);
		WRITE_COORD_VECTOR(start);
		WRITE_COORD_VECTOR(end);
		WRITE_SHORT(modelIdx);
		WRITE_BYTE(frameStart);
		WRITE_BYTE(framerate);
		WRITE_BYTE(life);
		WRITE_BYTE(width);
		WRITE_BYTE(noise);
		WRITE_BYTE(color.r);
		WRITE_BYTE(color.g);
		WRITE_BYTE(color.b);
		WRITE_BYTE(color.a);
		WRITE_BYTE(speed);
		MESSAGE_END();
	}
	else {
		CBeam* beam = CBeam::BeamCreate(INDEX_MODEL(modelIdx), width);
		beam->PointsInit(start, end);
		beam->SetFrame(frameStart);
		beam->SetNoise(noise);
		beam->SetColor(color.r, color.g, color.b);
		beam->SetBrightness(color.a);
		beam->SetScrollRate(speed);
		beam->LiveForTime(life * 0.1f);
		// TODO: framerate
	}
}

void UTIL_BSPDecal_msg(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_BSPDECAL);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_SHORT(decalIdx);
	WRITE_SHORT(entindex);
	if (entindex)
		WRITE_SHORT(ENT(entindex)->v.modelindex);
	MESSAGE_END();
}
void UTIL_BSPDecal(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_BSPDecal_msg, entindex, origin, decalIdx, msgMode, msgOrigin, targetEnt);
}

void UTIL_PlayerDecal_msg(int entindex, int playernum, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_PLAYERDECAL);
	WRITE_BYTE(playernum);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_SHORT(entindex);
	WRITE_BYTE(decalIdx);
	MESSAGE_END();
}
void UTIL_PlayerDecal(int entindex, int playernum, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_PlayerDecal_msg, entindex, playernum, origin, decalIdx, msgMode, msgOrigin, targetEnt);
}

void UTIL_GunshotDecal_msg(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_GUNSHOTDECAL);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_SHORT(entindex);
	WRITE_BYTE(decalIdx);
	MESSAGE_END();
}
void UTIL_GunshotDecal(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_GunshotDecal_msg, entindex, origin, decalIdx, msgMode, msgOrigin, targetEnt);
}

void UTIL_Decal_msg(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	int mode = TE_DECAL;

	if (entindex) {
		mode = decalIdx > 255 ? TE_DECALHIGH : TE_DECAL;
	}
	else {
		mode = decalIdx > 255 ? TE_WORLDDECALHIGH : TE_WORLDDECAL;
	}

	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(mode);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_BYTE(decalIdx);
	if (entindex)
		WRITE_SHORT(entindex);
	MESSAGE_END();
}
void UTIL_Decal(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_Decal_msg, entindex, origin, decalIdx, msgMode, msgOrigin, targetEnt);
}

void UTIL_Tracer(Vector start, Vector end, int msgMode, edict_t* targetEnt) {
	if (UTIL_IsValidTempEntOrigin(start) && UTIL_IsValidTempEntOrigin(end)) {
		MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, start, targetEnt);
		WRITE_BYTE(TE_TRACER);
		WRITE_COORD(start.x);
		WRITE_COORD(start.y);
		WRITE_COORD(start.z);
		WRITE_COORD(end.x);
		WRITE_COORD(end.y);
		WRITE_COORD(end.z);
		MESSAGE_END();
	}
	else {
		CBaseEntity::Create("te_user_tracer", start, end, true);
	}
}

void UTIL_Explosion(Vector origin, int sprIndex, uint8_t scale, uint8_t framerate, uint8_t flags) {
	if (UTIL_IsValidTempEntOrigin(origin)) {
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(origin.x);
		WRITE_COORD(origin.y);
		WRITE_COORD(origin.z);
		WRITE_SHORT(sprIndex);
		WRITE_BYTE(scale);
		WRITE_BYTE(framerate);
		WRITE_BYTE(flags);
		MESSAGE_END();
	}
	else {
		CBaseEntity* ent = CBaseEntity::Create("te_explosion", origin, g_vecZero, false);
		ent->pev->scale = scale / 10.0f;
		ent->pev->framerate = framerate;
		ent->pev->spawnflags = flags;
		DispatchSpawn(ent->edict());
		SET_MODEL(ent->edict(), INDEX_MODEL(sprIndex));
	}
}

void UTIL_Smoke(Vector origin, int sprIndex, uint8_t scale, uint8_t framerate) {
	if (UTIL_IsValidTempEntOrigin(origin)) {
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, origin);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(origin.x);
		WRITE_COORD(origin.y);
		WRITE_COORD(origin.z);
		WRITE_SHORT(sprIndex);
		WRITE_BYTE(scale);
		WRITE_BYTE(framerate);
		MESSAGE_END();
	}
	else {
		StringMap keys = {
			{"model", INDEX_MODEL(sprIndex)}
		};
		CBaseEntity* ent = CBaseEntity::Create("te_smoke", origin, g_vecZero, false, NULL, keys);
		ent->pev->scale = scale / 10.0f;
		ent->pev->framerate = framerate;
		int brightness = RANDOM_LONG(1, 32);
		ent->pev->rendercolor = Vector(brightness, brightness, brightness);
		DispatchSpawn(ent->edict());
	}
}

void UTIL_BeamCylinder(Vector pos, float radius, int modelIdx, uint8_t startFrame, uint8_t frameRate,
	uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t scrollSpeed) {
	if (UTIL_IsValidTempEntOrigin(pos)) {
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pos);
		WRITE_BYTE(TE_BEAMCYLINDER);
		WRITE_COORD(pos.x);
		WRITE_COORD(pos.y);
		WRITE_COORD(pos.z);
		WRITE_COORD(pos.x);
		WRITE_COORD(pos.y);
		WRITE_COORD(pos.z + radius); // reach damage radius over .2 seconds
		WRITE_SHORT(modelIdx);
		WRITE_BYTE(startFrame);
		WRITE_BYTE(frameRate);
		WRITE_BYTE(life);
		WRITE_BYTE(width);
		WRITE_BYTE(noise);
		WRITE_BYTE(color.r);
		WRITE_BYTE(color.g);
		WRITE_BYTE(color.b);
		WRITE_BYTE(color.a);
		WRITE_BYTE(scrollSpeed);
		MESSAGE_END();
	}
	else {
		CTeBeamRing* ring = (CTeBeamRing*)CBaseEntity::Create("te_beamring", pos, g_vecZero, true);
		int ringWidth = V_min(255, (int)width * 10); // beam rings are thinner than beam cylinders
		ring->Expand(radius, modelIdx, startFrame, frameRate, life, ringWidth, noise, color, scrollSpeed,
			MSG_PVS, pos, NULL);
	}
}

void UTIL_BeamDisk(Vector pos, float radius, int modelIdx, uint8_t startFrame, uint8_t frameRate,
	uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t scrollSpeed) {
	if (UTIL_IsValidTempEntOrigin(pos)) {
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pos);
		WRITE_BYTE(TE_BEAMDISK);
		WRITE_COORD(pos.x);
		WRITE_COORD(pos.y);
		WRITE_COORD(pos.z);
		WRITE_COORD(pos.x);
		WRITE_COORD(pos.y);
		WRITE_COORD(pos.z + radius); // reach damage radius over .2 seconds
		WRITE_SHORT(modelIdx);
		WRITE_BYTE(startFrame);
		WRITE_BYTE(frameRate);
		WRITE_BYTE(life);
		WRITE_BYTE(width);
		WRITE_BYTE(noise);
		WRITE_BYTE(color.r);
		WRITE_BYTE(color.g);
		WRITE_BYTE(color.b);
		WRITE_BYTE(color.a);
		WRITE_BYTE(scrollSpeed);
		MESSAGE_END();
	}
	else {
		ALERT(at_console, "TODO: Beamdisk\n");
		CTeBeamRing* ring = (CTeBeamRing*)CBaseEntity::Create("te_beamring", pos, g_vecZero, true);
		ring->Expand(radius, modelIdx, startFrame, frameRate, life, width, noise, color, scrollSpeed,
			MSG_PVS, pos, NULL);
	}
}

void UTIL_BloodStream(const Vector& origin, const Vector& direction, int color, int amount)
{
	if (!UTIL_ShouldShowBlood(color))
		return;

	if (g_Language == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED)
		color = 0;


	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, origin);
	WRITE_BYTE(TE_BLOODSTREAM);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_COORD(direction.x);
	WRITE_COORD(direction.y);
	WRITE_COORD(direction.z);
	WRITE_BYTE(color);
	WRITE_BYTE(V_min(amount, 255));
	MESSAGE_END();
}

void UTIL_BloodDrips(const Vector& origin, const Vector& direction, int color, int amount)
{
	if (!UTIL_ShouldShowBlood(color))
		return;

	if (color == DONT_BLEED || amount == 0)
		return;

	if (g_Language == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED)
		color = 0;

	if (g_pGameRules->IsMultiplayer())
	{
		// scale up blood effect in multiplayer for better visibility
		amount *= 2;
	}

	if (amount > 255)
		amount = 255;

	int scale = V_min(V_max(3, amount / 10), 16);

	if (UTIL_IsValidTempEntOrigin(origin)) {
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, origin);
		WRITE_BYTE(TE_BLOODSPRITE);
		WRITE_COORD(origin.x);								// pos
		WRITE_COORD(origin.y);
		WRITE_COORD(origin.z);
		WRITE_SHORT(g_sModelIndexBloodSpray);				// initial sprite model
		WRITE_SHORT(g_sModelIndexBloodDrop);				// droplet sprite models
		WRITE_BYTE(color);								// color index into host_basepal
		WRITE_BYTE(scale);		// size
		MESSAGE_END();
	}
	else {
		CBaseEntity* ent = CBaseEntity::Create("te_bloodsprite", origin, g_vecZero, true);
		ent->pev->scale = scale / 30.0f;
		ent->pev->rendercolor = GetTeColor(color + RANDOM_LONG(0, 4)).ToVector();

		for (int i = 0; i < 2; i++) {
			CBaseEntity* drip = CBaseEntity::Create("te_bloodsprite_drip", origin, g_vecZero, true);
			drip->pev->rendercolor = GetTeColor(color + RANDOM_LONG(0, 4)).ToVector();
			drip->pev->scale = V_max(scale / 30.0f, 0.2f);
		}
	}
}

void UTIL_BloodDecalTrace(TraceResult* pTrace, int bloodColor)
{
	if (UTIL_ShouldShowBlood(bloodColor))
	{
		if (bloodColor == BLOOD_COLOR_RED)
			UTIL_DecalTrace(pTrace, DECAL_BLOOD1 + RANDOM_LONG(0, 5));
		else
			UTIL_DecalTrace(pTrace, DECAL_YBLOOD1 + RANDOM_LONG(0, 5));
	}
}


edict_t* UTIL_DecalTrace(TraceResult* pTrace, int decalNumber, int msgMode, edict_t* targetEnt)
{
	short entityIndex;
	int index;

	if (decalNumber < 0)
		return pTrace->pHit;

	index = gDecals[decalNumber].index;

	if (index < 0)
		return pTrace->pHit;

	if (pTrace->flFraction == 1.0)
		return pTrace->pHit;

	// Only decal BSP models
	if (pTrace->pHit)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(pTrace->pHit);
		if (pEntity && !pEntity->IsBSPModel())
			return pTrace->pHit;
		entityIndex = ENTINDEX(pTrace->pHit);
	}
	else
		entityIndex = 0;

	if (!UTIL_IsValidTempEntOrigin(pTrace->vecEndPos)) {
		// TODO: attach to moving objects and match transparency
		Vector ori = pTrace->vecEndPos + pTrace->vecPlaneNormal * 0.1f; // avoid z fighting
		Vector angles = UTIL_VecToSpriteAngles(pTrace->vecPlaneNormal);
		CBaseEntity* decal = CBaseEntity::Create("te_decal", ori, angles, false);
		decal->pev->skin = decalNumber;
		decal->pev->iuser1 = pTrace->pHit ? ENTINDEX(pTrace->pHit) : 0;
		decal->pev->v_angle = pTrace->vecPlaneNormal;
		DispatchSpawn(decal->edict());
		return decal->edict();
	}

	UTIL_Decal(entityIndex, pTrace->vecEndPos, index, msgMode, 0, targetEnt);
	return pTrace->pHit;
}

/*
==============
UTIL_PlayerDecalTrace

A player is trying to apply his custom decal for the spray can.
Tell connected clients to display it, or use the default spray can decal
if the custom can't be loaded.
==============
*/
void UTIL_PlayerDecalTrace(TraceResult* pTrace, int playernum, int decalNumber, BOOL bIsCustom)
{
	int index;

	if (!bIsCustom)
	{
		if (decalNumber < 0)
			return;

		index = gDecals[decalNumber].index;
		if (index < 0)
			return;
	}
	else
		index = decalNumber;

	if (pTrace->flFraction == 1.0)
		return;

	UTIL_PlayerDecal(ENTINDEX(pTrace->pHit), playernum, pTrace->vecEndPos, index);
}

edict_t* UTIL_GunshotDecalTrace(TraceResult* pTrace, int decalNumber, edict_t* emitter)
{
	if (decalNumber < 0)
		return pTrace->pHit;

	int index = gDecals[decalNumber].index;
	if (index < 0)
		return pTrace->pHit;

	if (pTrace->flFraction == 1.0)
		return pTrace->pHit;

	if (!UTIL_IsValidTempEntOrigin(pTrace->vecEndPos)) {
		return UTIL_DecalTrace(pTrace, decalNumber);
	}

	if (emitter) {
		for (int i = 1; i < gpGlobals->maxClients; i++) {
			CBasePlayer* plr = UTIL_PlayerByIndex(i);

			if (plr && plr->edict() != emitter) {
				UTIL_GunshotDecal(ENTINDEX(pTrace->pHit), pTrace->vecEndPos, index, MSG_ONE_UNRELIABLE, pTrace->vecEndPos, plr->edict());
			}
		}
	}
	else {
		UTIL_GunshotDecal(ENTINDEX(pTrace->pHit), pTrace->vecEndPos, index, MSG_PAS, pTrace->vecEndPos);
	}
	return pTrace->pHit;
}


void UTIL_Sparks(const Vector& position)
{
	if (UTIL_IsValidTempEntOrigin(position)) {
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
		WRITE_BYTE(TE_SPARKS);
		WRITE_COORD(position.x);
		WRITE_COORD(position.y);
		WRITE_COORD(position.z);
		MESSAGE_END();
	}
	else {
		CBaseEntity::Create("te_sparks", position, g_vecZero, true);
	}
}


void UTIL_Ricochet(const Vector& position, float scale)
{
	if (UTIL_IsValidTempEntOrigin(position)) {
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
		WRITE_BYTE(TE_ARMOR_RICOCHET);
		WRITE_COORD(position.x);
		WRITE_COORD(position.y);
		WRITE_COORD(position.z);
		WRITE_BYTE((int)(scale * 10));
		MESSAGE_END();
	}
	else {
		CBaseEntity::Create("te_ricochet", position, g_vecZero, true);
	}
}

void UTIL_Sprite(const Vector& position, int sprIndex, uint8_t scale, uint8_t opacity) {
	if (UTIL_IsValidTempEntOrigin(position)) {
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
		WRITE_BYTE(TE_SPRITE);
		WRITE_COORD(position.x);
		WRITE_COORD(position.y);
		WRITE_COORD(position.z);
		WRITE_SHORT(sprIndex);
		WRITE_BYTE(scale);
		WRITE_BYTE(opacity);
		MESSAGE_END();
	}
	else {
		CSprite* spr = CSprite::SpriteCreate(INDEX_MODEL(sprIndex), position, true);
		spr->pev->spawnflags = SF_SPRITE_ONCE_AND_REMOVE;
		spr->pev->rendermode = kRenderTransAdd;
		spr->pev->renderamt = opacity;
		spr->pev->scale = scale * 0.1f;
		spr->TurnOn();
	}
}


void UTIL_BreakModel(const Vector& pos, const Vector& size, const Vector& velocity, uint8_t noise,
	int modelIdx, uint8_t shards, uint8_t duration, uint8_t flags) {
	if (UTIL_IsValidTempEntOrigin(pos)) {
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pos);
		WRITE_BYTE(TE_BREAKMODEL);
		WRITE_COORD_VECTOR(pos);
		WRITE_COORD_VECTOR(size);
		WRITE_COORD_VECTOR(velocity);
		WRITE_BYTE(noise);
		WRITE_SHORT(modelIdx);	//model id#
		WRITE_BYTE(shards);
		WRITE_BYTE(duration);
		WRITE_BYTE(flags);
		MESSAGE_END();
	}
	else {
		const char* mdl = INDEX_MODEL(modelIdx);
		int bodyGroupCount = MODEL_FRAMES(MODEL_INDEX(mdl));

		int material = matNone;
		if (flags & 1) { material = matGlass; }
		if (flags & 2) { material = matMetal; }
		if (flags & 4) { material = matFlesh; }
		if (flags & 8) { material = matWood; }
		if (flags & 64) { material = matRocks; }

		shards = shards ? V_min(8, shards) : 8;
		float fadeStart = gpGlobals->time + (duration * 0.5f);
		float randVel = noise * 10;

		for (int i = 0; i < shards; i++) {
			CGib* pGib = GetClassPtr((CGib*)NULL);

			pGib->Spawn(mdl);
			pGib->m_cBloodDecals = 0;
			pGib->m_slideFriction = 0.5f;
			pGib->m_material = material;
			pGib->pev->friction = 0.5f;
			pGib->pev->gravity = 0.5f;
			pGib->pev->body = RANDOM_LONG(0, bodyGroupCount);
			pGib->pev->origin.x = pos.x + size.x * RANDOM_FLOAT(-0.5f, 0.5f);
			pGib->pev->origin.y = pos.y + size.y * RANDOM_FLOAT(-0.5f, 0.5f);
			pGib->pev->origin.z = pos.z + size.z * RANDOM_FLOAT(-0.5f, 0.5f);
			pGib->pev->velocity = velocity + Vector(RANDOM_FLOAT(-randVel, randVel), RANDOM_FLOAT(-randVel, randVel), RANDOM_FLOAT(0, randVel));
			pGib->pev->avelocity = Vector(RANDOM_FLOAT(-400, 400), 0, RANDOM_FLOAT(-400, 400));
			pGib->pev->angles = Vector(RANDOM_FLOAT(-180, 180), RANDOM_FLOAT(-180, 180), RANDOM_FLOAT(-180, 180));
			UTIL_SetSize(pGib->pev, Vector(0, 0, 0), Vector(0, 0, 0));
			pGib->LimitVelocity();

			if (flags & (1 | 32)) {
				pGib->pev->rendermode = kRenderTransTexture;
				pGib->pev->renderamt = 128;
			}
			if (flags & 16) {
				// TODO: particle trails
			}

			pGib->SetThink(&CGib::BreakThink);
			pGib->pev->nextthink = gpGlobals->time;
			pGib->m_lifeTime = fadeStart + RANDOM_FLOAT(0, 1);
		}
	}
}

void UTIL_SpriteSpray(Vector pos, Vector dir, int spriteIdx, uint8_t count, uint8_t speed, uint8_t noise, bool test) {
	if (UTIL_IsValidTempEntOrigin(pos) && !test) {
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pos);
		WRITE_BYTE(TE_SPRITE_SPRAY);
		WRITE_COORD_VECTOR(pos);
		WRITE_COORD_VECTOR(dir);
		WRITE_SHORT(spriteIdx);
		WRITE_BYTE(count);
		WRITE_BYTE(speed);
		WRITE_BYTE(noise);
		MESSAGE_END();
	}
	else if (count) {
		const char* mdl = INDEX_MODEL(spriteIdx);
		int frameCount = MODEL_FRAMES(MODEL_INDEX(mdl));

		count = V_min(8, count);
		float randVel = noise * 0.01f;
		
		for (int i = 0; i < count; i++) {
			CGib* pGib = GetClassPtr((CGib*)NULL);

			float randSpeed = speed * 2.0f * RANDOM_FLOAT(0.8f, 1.2f);

			pGib->Spawn(mdl);
			pGib->m_cBloodDecals = 0;
			pGib->m_lifeTime = gpGlobals->time + 0.5f;
			pGib->m_slideFriction = 0.5f;
			pGib->pev->friction = 0.5f;
			pGib->pev->gravity = 0.5f;
			pGib->pev->frame = RANDOM_LONG(0, frameCount);
			pGib->pev->origin = pos;
			pGib->pev->velocity = (dir + Vector(RANDOM_FLOAT(-randVel, randVel), RANDOM_FLOAT(-randVel, randVel), RANDOM_FLOAT(0, randVel*1.5f))) * randSpeed;
			UTIL_SetSize(pGib->pev, Vector(0, 0, 0), Vector(0, 0, 0));
			pGib->LimitVelocity();
			pGib->pev->flags |= FL_NOCLIP_MONSTERS | FL_NOCLIP_PLAYERS | FL_NOCLIP_PUSHABLES | FL_NOCLIP_TRACES;
			pGib->pev->solid = SOLID_NOT;
			pGib->pev->movetype = MOVETYPE_FLY;

			pGib->pev->renderamt = 255;
			pGib->pev->rendermode = kRenderTransTexture;

			pGib->SetThink(&CGib::SprayThink);
			pGib->SetTouch(&CGib::SprayTouch);
			pGib->pev->nextthink = gpGlobals->time;
		}
	}
}

void UTIL_Shrapnel(Vector pos, Vector dir, float flDamage, int bitsDamageType) {
	Vector sprPos = pos - Vector(0, 0, 10);
	bool isBlast = bitsDamageType & DMG_BLAST;
	int gibCount = V_min(128, flDamage / 10);

	if (!UTIL_IsValidTempEntOrigin(pos)) {
		gibCount = 1;
	}

	UTIL_Explosion(sprPos, g_sModelIndexShrapnelHit, V_min(8, RANDOM_LONG(3, 4) + (flDamage / 20)), 50, 2 | 4 | 8);

	UTIL_BreakModel(pos, g_vecZero, dir, isBlast ? 30 : 15, g_sModelIndexShrapnel, gibCount, 1, 0);

	// saving this in case it's useful for a similar effect. The sounds make more sense for small gibs
	// and they have higher gravity and less bounce. Much higher network usage for lots of gibs though.
	/*
	for (int i = 0; i < gibCount; i++) {
		EjectBrass(pos, dir * 200, RANDOM_LONG(0, 1.0f), MODEL_INDEX("models/shrapnel.mdl"), TE_BOUNCE_SHELL);
	}
	*/
}

extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model

void UTIL_Bubbles(Vector mins, Vector maxs, int count)
{
	Vector mid = (mins + maxs) * 0.5;

	float flHeight = UTIL_WaterLevel(mid, mid.z, mid.z + 1024);
	flHeight = flHeight - mins.z;

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, mid);
	WRITE_BYTE(TE_BUBBLES);
	WRITE_COORD(mins.x);	// mins
	WRITE_COORD(mins.y);
	WRITE_COORD(mins.z);
	WRITE_COORD(maxs.x);	// maxz
	WRITE_COORD(maxs.y);
	WRITE_COORD(maxs.z);
	WRITE_COORD(flHeight);			// height
	WRITE_SHORT(g_sModelIndexBubbles);
	WRITE_BYTE(count); // count
	WRITE_COORD(8); // speed
	MESSAGE_END();
}

void UTIL_BubbleTrail(Vector from, Vector to, int count)
{
	float flHeight = UTIL_WaterLevel(from, from.z, from.z + 256);
	flHeight = flHeight - from.z;

	if (flHeight < 8)
	{
		flHeight = UTIL_WaterLevel(to, to.z, to.z + 256);
		flHeight = flHeight - to.z;
		if (flHeight < 8)
			return;

		// UNDONE: do a ploink sound
		flHeight = flHeight + to.z - from.z;
	}

	if (count > 255)
		count = 255;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BUBBLETRAIL);
	WRITE_COORD(from.x);	// mins
	WRITE_COORD(from.y);
	WRITE_COORD(from.z);
	WRITE_COORD(to.x);	// maxz
	WRITE_COORD(to.y);
	WRITE_COORD(to.z);
	WRITE_COORD(flHeight);			// height
	WRITE_SHORT(g_sModelIndexBubbles);
	WRITE_BYTE(count); // count
	WRITE_COORD(8); // speed
	MESSAGE_END();
}

void UTIL_DLight(Vector pos, uint8_t radius, RGB color, uint8_t time, uint8_t decay) {
	if (UTIL_IsValidTempEntOrigin(pos)) {
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pos);
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD_VECTOR(pos);
		WRITE_BYTE(radius);
		WRITE_BYTE(color.r);
		WRITE_BYTE(color.g);
		WRITE_BYTE(color.b);
		WRITE_BYTE(time);
		WRITE_BYTE(decay);
		MESSAGE_END();
	}
	else if (radius > 10) {
		CBaseEntity* ent = CBaseEntity::Create("te_target", pos, g_vecZero, true);
		ent->pev->effects = radius > 100 ? EF_BRIGHTLIGHT : EF_DIMLIGHT;

		time -= decay * 0.5f;

		ent->SetThink(&CBaseEntity::SUB_Remove);
		ent->pev->nextthink = gpGlobals->time + time * 0.1f;
	}
}

void UTIL_TempSound(Vector pos, const char* sample, float volume, float attenuation, int flags, int pitch) {
	CBaseEntity* ent = CBaseEntity::Create("te_target", pos, g_vecZero, true);

	EMIT_SOUND_DYN(ent->edict(), CHAN_WEAPON, sample, volume, attenuation, flags, pitch);

	ent->SetThink(&CBaseEntity::SUB_Remove);
	ent->pev->nextthink = gpGlobals->time + 0.1f;
}

void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage)
{
	UTIL_BloodDrips(vecSpot, g_vecAttackDir, bloodColor, (int)flDamage);
}

int DamageDecal(CBaseEntity* pEntity, int bitsDamageType)
{
	if (!pEntity)
		return (DECAL_GUNSHOT1 + RANDOM_LONG(0, 4));

	return pEntity->DamageDecal(bitsDamageType);
}

void DecalGunshot(TraceResult* pTrace, int iBulletType, bool playTextureSound, Vector vecSrc, Vector vecEnd, edict_t* bulletEmitter)
{
	// Is the entity valid
	if (!UTIL_IsValidEntity(pTrace->pHit))
		return;

	// Sounds don't play out of bounds unless attached to an entity.
	// The lower level decal traces will return a new decal entity
	// which can be used for sound playback, if the trace hit an OOB surface
	edict_t* soundEmitter = pTrace->pHit;

	if (VARS(pTrace->pHit)->solid == SOLID_BSP || VARS(pTrace->pHit)->movetype == MOVETYPE_PUSHSTEP)
	{
		CBaseEntity* pEntity = NULL;
		// Decal the wall with a gunshot
		if (!FNullEnt(pTrace->pHit))
			pEntity = CBaseEntity::Instance(pTrace->pHit);

		switch (iBulletType)
		{
		case BULLET_PLAYER_9MM:
		case BULLET_MONSTER_9MM:
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_PLAYER_BUCKSHOT:
		case BULLET_PLAYER_357:
		case BULLET_PLAYER_556:
		default:
			// smoke and decal
			soundEmitter = UTIL_GunshotDecalTrace(pTrace, DamageDecal(pEntity, DMG_BULLET), bulletEmitter);
			break;
		case BULLET_MONSTER_12MM:
			// smoke and decal
			soundEmitter = UTIL_GunshotDecalTrace(pTrace, DamageDecal(pEntity, DMG_BULLET), bulletEmitter);
			break;
		case BULLET_PLAYER_CROWBAR:
			// wall decal
			soundEmitter = UTIL_DecalTrace(pTrace, DamageDecal(pEntity, DMG_CLUB));
			break;
		}

		if (iBulletType != BULLET_PLAYER_CROWBAR && !UTIL_IsValidTempEntOrigin(pTrace->vecEndPos)) {
			// the client side effect doesn't always play the ricochet sound
			if (RANDOM_LONG(0, 1) == 0)
				UTIL_PlayRicochetSound(soundEmitter);
		}
	}

	if (playTextureSound) {
		TEXTURETYPE_PlaySound(pTrace, vecSrc, vecEnd, iBulletType, soundEmitter, bulletEmitter);
	}
}

//
// EjectBrass - tosses a brass shell from passed origin at passed velocity
//
void EjectBrass(const Vector& vecOrigin, const Vector& vecVelocity, float rotation, int model, int soundtype)
{
	// FIX: when the player shoots, their gun isn't in the same position as it is on the model other players see.

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_MODEL);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_COORD(vecVelocity.x);
	WRITE_COORD(vecVelocity.y);
	WRITE_COORD(vecVelocity.z);
	WRITE_ANGLE(rotation);
	WRITE_SHORT(model);
	WRITE_BYTE(soundtype);
	WRITE_BYTE(25);// 2.5 seconds
	MESSAGE_END();
}