#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "CBasePlayer.h"
#include "CBaseSpectator.h"
#include "client.h"
#include "env/CSoundEnt.h"
#include "gamerules.h"
#include "game.h"
#include "customentity.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"
#include "pm_shared.h"
#include "voice_gamemgr.h"

extern CVoiceGameMgr g_VoiceGameMgr;
extern int gmsgSayText;
extern cvar_t allow_spectators;
extern int g_teamplay;
extern float g_flWeaponCheat;

#if defined( _MSC_VER ) || defined( WIN32 )
typedef wchar_t	uchar16;
typedef unsigned int uchar32;
#else
typedef unsigned short uchar16;
typedef wchar_t uchar32;
#endif

//-----------------------------------------------------------------------------
// Purpose: determine if a uchar32 represents a valid Unicode code point
//-----------------------------------------------------------------------------
bool Q_IsValidUChar32(uchar32 uVal)
{
	// Values > 0x10FFFF are explicitly invalid; ditto for UTF-16 surrogate halves,
	// values ending in FFFE or FFFF, or values in the 0x00FDD0-0x00FDEF reserved range
	return (uVal < 0x110000u) && ((uVal - 0x00D800u) > 0x7FFu) && ((uVal & 0xFFFFu) < 0xFFFEu) && ((uVal - 0x00FDD0u) > 0x1Fu);
}

// Decode one character from a UTF-8 encoded string. Treats 6-byte CESU-8 sequences
// as a single character, as if they were a correctly-encoded 4-byte UTF-8 sequence.
int Q_UTF8ToUChar32(const char* pUTF8_, uchar32& uValueOut, bool& bErrorOut)
{
	const uint8* pUTF8 = (const uint8*)pUTF8_;

	int nBytes = 1;
	uint32 uValue = pUTF8[0];
	uint32 uMinValue = 0;

	// 0....... single byte
	if (uValue < 0x80)
		goto decodeFinishedNoCheck;

	// Expecting at least a two-byte sequence with 0xC0 <= first <= 0xF7 (110...... and 11110...)
	if ((uValue - 0xC0u) > 0x37u || (pUTF8[1] & 0xC0) != 0x80)
		goto decodeError;

	uValue = (uValue << 6) - (0xC0 << 6) + pUTF8[1] - 0x80;
	nBytes = 2;
	uMinValue = 0x80;

	// 110..... two-byte lead byte
	if (!(uValue & (0x20 << 6)))
		goto decodeFinished;

	// Expecting at least a three-byte sequence
	if ((pUTF8[2] & 0xC0) != 0x80)
		goto decodeError;

	uValue = (uValue << 6) - (0x20 << 12) + pUTF8[2] - 0x80;
	nBytes = 3;
	uMinValue = 0x800;

	// 1110.... three-byte lead byte
	if (!(uValue & (0x10 << 12)))
		goto decodeFinishedMaybeCESU8;

	// Expecting a four-byte sequence, longest permissible in UTF-8
	if ((pUTF8[3] & 0xC0) != 0x80)
		goto decodeError;

	uValue = (uValue << 6) - (0x10 << 18) + pUTF8[3] - 0x80;
	nBytes = 4;
	uMinValue = 0x10000;

	// 11110... four-byte lead byte. fall through to finished.

decodeFinished:
	if (uValue >= uMinValue && Q_IsValidUChar32(uValue))
	{
	decodeFinishedNoCheck:
		uValueOut = uValue;
		bErrorOut = false;
		return nBytes;
	}
decodeError:
	uValueOut = '?';
	bErrorOut = true;
	return nBytes;

decodeFinishedMaybeCESU8:
	// Do we have a full UTF-16 surrogate pair that's been UTF-8 encoded afterwards?
	// That is, do we have 0xD800-0xDBFF followed by 0xDC00-0xDFFF? If so, decode it all.
	if ((uValue - 0xD800u) < 0x400u && pUTF8[3] == 0xED && (uint8)(pUTF8[4] - 0xB0) < 0x10 && (pUTF8[5] & 0xC0) == 0x80)
	{
		uValue = 0x10000 + ((uValue - 0xD800u) << 10) + ((uint8)(pUTF8[4] - 0xB0) << 6) + pUTF8[5] - 0x80;
		nBytes = 6;
		uMinValue = 0x10000;
	}
	goto decodeFinished;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if UTF-8 string contains invalid sequences.
//-----------------------------------------------------------------------------
bool Q_UnicodeValidate(const char* pUTF8)
{
	bool bError = false;
	while (*pUTF8)
	{
		uchar32 uVal;
		// Our UTF-8 decoder silently fixes up 6-byte CESU-8 (improperly re-encoded UTF-16) sequences.
		// However, these are technically not valid UTF-8. So if we eat 6 bytes at once, it's an error.
		int nCharSize = Q_UTF8ToUChar32(pUTF8, uVal, bError);
		if (bError || nCharSize == 6)
			return false;
		pUTF8 += nCharSize;
	}
	return true;
}

//// HOST_SAY
// String comes in as
// say blah blah blah
// or as
// blah blah blah
//
void Host_Say(edict_t* pEntity, int teamonly)
{
	CBasePlayer* client;
	int		j;
	char* p;
	char	text[128];
	char    szTemp[256];
	const char* cpSay = "say";
	const char* cpSayTeam = "say_team";
	const char* pcmd = CMD_ARGV(0);

	// We can get a raw string now, without the "say " prepended
	if (CMD_ARGC() == 0)
		return;

	entvars_t* pev = &pEntity->v;
	CBasePlayer* player = GetClassPtr((CBasePlayer*)pev);

	//Not yet.
	if (player->m_flNextChatTime > gpGlobals->time)
		return;

	if (!stricmp(pcmd, cpSay) || !stricmp(pcmd, cpSayTeam))
	{
		if (CMD_ARGC() >= 2)
		{
			p = (char*)CMD_ARGS();
		}
		else
		{
			// say with a blank message, nothing to do
			return;
		}
	}
	else  // Raw text, need to prepend argv[0]
	{
		if (CMD_ARGC() >= 2)
		{
			snprintf(szTemp, 256, "%s %s", (char*)pcmd, (char*)CMD_ARGS());
		}
		else
		{
			// Just a one word command, use the first word...sigh
			snprintf(szTemp, 256, "%s", (char*)pcmd);
		}
		p = szTemp;
	}

	// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[strlen(p) - 1] = 0;
	}

	// make sure the text has content

	if (!p || !p[0] || !Q_UnicodeValidate(p))
		return;  // no character found, so say nothing

// turn on color set 2  (color on,  no sound)
	// turn on color set 2  (color on,  no sound)
	if (player->IsObserver() && (teamonly))
		snprintf(text, 128, "%c(SPEC) %s: ", 2, STRING(pEntity->v.netname));
	else if (teamonly)
		snprintf(text, 128, "%c(TEAM) %s: ", 2, STRING(pEntity->v.netname));
	else
		snprintf(text, 128, "%c%s: ", 2, STRING(pEntity->v.netname));

	j = sizeof(text) - 2 - strlen(text);  // -2 for /n and null terminator
	if ((int)strlen(p) > j)
		p[j] = 0;

	strcat_safe(text, p, 128);
	strcat_safe(text, "\n", 128);


	player->m_flNextChatTime = gpGlobals->time + CHAT_INTERVAL;

	// loop through all players
	// Start with the first player.
	// This may return the world in single player if the client types something between levels or during spawn
	// so check it, or it will infinite loop

	client = NULL;
	while (((client = (CBasePlayer*)UTIL_FindEntityByClassname(client, "player")) != NULL) && (!FNullEnt(client->edict())))
	{
		if (!client->pev)
			continue;

		if (client->edict() == pEntity)
			continue;

		if (!(client->IsNetClient()))	// Not a client ? (should never be true)
			continue;

		// can the receiver hear the sender? or has he muted him?
		if (g_VoiceGameMgr.PlayerHasBlockedPlayer(client, player))
			continue;

		/*
		if (!player->IsObserver() && teamonly && g_pGameRules->PlayerRelationship(client, CBaseEntity::Instance(pEntity)) != GR_TEAMMATE)
			continue;

		// Spectators can only talk to other specs
		if (player->IsObserver() && teamonly)
			if (!client->IsObserver())
				continue;
		*/

		MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, client->pev);
		WRITE_BYTE(ENTINDEX(pEntity));
		WRITE_STRING(text);
		MESSAGE_END();

	}

	// print to the sending client
	MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, &pEntity->v);
	WRITE_BYTE(ENTINDEX(pEntity));
	WRITE_STRING(text);
	MESSAGE_END();

	// echo to server console for listen servers, dedicated servers should have logs enabled
	if (!IS_DEDICATED_SERVER())
		g_engfuncs.pfnServerPrint(text);

	const char* temp;
	if (teamonly)
		temp = "say_team";
	else
		temp = "say";

	// team match?
	if (g_teamplay)
	{
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" %s \"%s\"\n",
			STRING(pEntity->v.netname),
			GETPLAYERUSERID(pEntity),
			GETPLAYERAUTHID(pEntity),
			g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pEntity), "model"),
			temp,
			p);
	}
	else
	{
		UTIL_LogPrintf("\"%s<%i><%s><%i>\" %s \"%s\"\n",
			STRING(pEntity->v.netname),
			GETPLAYERUSERID(pEntity),
			GETPLAYERAUTHID(pEntity),
			GETPLAYERUSERID(pEntity),
			temp,
			p);
	}
}

#define ABORT_IF_CHEATS_DISABLED(cheatName) \
if (CVAR_GET_FLOAT("sv_cheats") == 0) {\
	CLIENT_PRINTF(pEntity, print_center, UTIL_VarArgs(cheatName " N/A: Cheats disabled\n")); \
	return true; \
}

bool CheatCommand(edict_t* pEntity) {
	const char* pcmd = CMD_ARGV(0);

	entvars_t* pev = &pEntity->v;

	if (FStrEq(pcmd, "cl_noclip")) {
		ABORT_IF_CHEATS_DISABLED("No clip");
		pev->movetype = pev->movetype == MOVETYPE_NOCLIP ? MOVETYPE_WALK : MOVETYPE_NOCLIP;
		const char* newMode = pev->movetype == MOVETYPE_NOCLIP ? "ON" : "OFF";
		CLIENT_PRINTF(pEntity, print_center, UTIL_VarArgs("No clip is %s\n", newMode));
	}
	else if (FStrEq(pcmd, "godmode")) {
		ABORT_IF_CHEATS_DISABLED("God mode");
		pev->takedamage = pev->takedamage == DAMAGE_NO ? DAMAGE_YES : DAMAGE_NO;
		const char* newMode = pev->takedamage == DAMAGE_NO ? "ON" : "OFF";
		CLIENT_PRINTF(pEntity, print_center, UTIL_VarArgs("God mode is %s\n", newMode));
	}
	else if (FStrEq(pcmd, "cl_notarget")) {
		ABORT_IF_CHEATS_DISABLED("No target");
		pev->flags ^= FL_NOTARGET;
		const char* newMode = pev->flags & FL_NOTARGET ? "ON" : "OFF";
		CLIENT_PRINTF(pEntity, print_center, UTIL_VarArgs("No target is %s\n", newMode));
	}
	else if (FStrEq(pcmd, "trigger")) {
		ABORT_IF_CHEATS_DISABLED("Trigger");
		
		const char* target = CMD_ARGV(1);
		CBaseEntity* world = (CBaseEntity*)GET_PRIVATE(INDEXENT(0));
		int count = 0;

		string_t lastTriggerClass = 0;

		if (target[0] != '\0') {
			edict_t* pTarget = NULL;
			while (!FNullEnt(pTarget = FIND_ENTITY_BY_TARGETNAME(pTarget, target))) {
				CBaseEntity* ent = CBaseEntity::Instance(pTarget);
				if (ent) {
					ent->Use(world, world, USE_TOGGLE, 0);
					count++;
					lastTriggerClass = ent->pev->classname;
				}
			}
		}
		else {
			UTIL_MakeVectors(pev->v_angle + pev->punchangle);
			Vector aimDir = gpGlobals->v_forward;
			Vector vecSrc = pev->origin + pev->view_ofs;

			TraceResult tr;
			TRACE_LINE(vecSrc, vecSrc + aimDir * 4096, dont_ignore_monsters, pEntity, &tr);

			CBaseEntity* activator = CBaseEntity::Instance(pEntity);
			CBaseEntity* phit = CBaseEntity::Instance(tr.pHit);
			if (activator && phit && !FNullEnt(tr.pHit)) {
				phit->Use(activator, activator, USE_TOGGLE, 0);
				count++;
				lastTriggerClass = phit->pev->classname;
			}
		}

		if (count > 1) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/wpn_denyselect.wav", 0.4, ATTN_NORM);
			CLIENT_PRINTF(pEntity, print_center, UTIL_VarArgs("Triggered %d entities\n", count));
		}
		else if (count == 1) {
			CLIENT_PRINTF(pEntity, print_center, UTIL_VarArgs("Triggered a %s\n", STRING(lastTriggerClass)));
		}
		
		EMIT_SOUND(ENT(pev), CHAN_ITEM, count > 0 ? "common/wpn_select.wav" : "common/wpn_denyselect.wav", 0.4, ATTN_NORM);
	}
	else {
		return false;
	}

	return true;
}

// Use CMD_ARGV,  CMD_ARGV, and CMD_ARGC to get pointers the character string command.
void ClientCommand(edict_t* pEntity)
{
	const char* pcmd = CMD_ARGV(0);
	const char* pstr;

	// Is the client spawned yet?
	if (!pEntity->pvPrivateData)
		return;

	entvars_t* pev = &pEntity->v;

	if (CheatCommand(pEntity)) {
		return;
	}
	else if (FStrEq(pcmd, "say"))
	{
		Host_Say(pEntity, 0);
	}
	else if (FStrEq(pcmd, "say_team"))
	{
		Host_Say(pEntity, 1);
	}
	else if (FStrEq(pcmd, "fullupdate"))
	{
		GetClassPtr((CBasePlayer*)pev)->ForceClientDllUpdate();
	}
	else if (FStrEq(pcmd, "give"))
	{
		if (g_flWeaponCheat != 0.0)
		{
			int iszItem = ALLOC_STRING(CMD_ARGV(1));	// Make a copy of the classname
			GetClassPtr((CBasePlayer*)pev)->GiveNamedItem(STRING(iszItem));
		}
	}

	else if (FStrEq(pcmd, "drop"))
	{
		// player is dropping an item. 
		GetClassPtr((CBasePlayer*)pev)->DropPlayerItem((char*)CMD_ARGV(1));
	}
	else if (FStrEq(pcmd, "fov"))
	{
		if (g_flWeaponCheat && CMD_ARGC() > 1)
		{
			GetClassPtr((CBasePlayer*)pev)->m_iFOV = atoi(CMD_ARGV(1));
		}
		else
		{
			CLIENT_PRINTF(pEntity, print_console, UTIL_VarArgs("\"fov\" is \"%d\"\n", (int)GetClassPtr((CBasePlayer*)pev)->m_iFOV));
		}
	}
	else if (FStrEq(pcmd, "use"))
	{
		GetClassPtr((CBasePlayer*)pev)->SelectItem((char*)CMD_ARGV(1));
	}
	else if (((pstr = strstr(pcmd, MOD_SPRITE_FOLDER "weapon_")) != NULL) && (pstr == pcmd))
	{
		// custom weapon was selected (weapon name includes a folder path to force clients to load HUD files from there)
		const char* wepCname = pstr + strlen(MOD_SPRITE_FOLDER);
		GetClassPtr((CBasePlayer*)pev)->SelectItem(wepCname);
	}
	else if (((pstr = strstr(pcmd, "weapon_")) != NULL) && (pstr == pcmd))
	{
		GetClassPtr((CBasePlayer*)pev)->SelectItem(pcmd);
	}
	else if (FStrEq(pcmd, "lastinv"))
	{
		GetClassPtr((CBasePlayer*)pev)->SelectLastItem();
	}
	else if (FStrEq(pcmd, "spectate"))	// clients wants to become a spectator
	{
		// always allow proxies to become a spectator
		if ((pev->flags & FL_PROXY) || allow_spectators.value)
		{
			CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pev);

			edict_t* pentSpawnSpot = g_pGameRules->GetPlayerSpawnSpot(pPlayer);
			pPlayer->StartObserver(pev->origin, VARS(pentSpawnSpot)->angles);

			pPlayer->pev->origin = VARS(pentSpawnSpot)->origin + Vector(0, 0, 1);
			pPlayer->pev->v_angle = g_vecZero;
			pPlayer->pev->velocity = g_vecZero;
			pPlayer->pev->angles = VARS(pentSpawnSpot)->angles;
			pPlayer->pev->punchangle = g_vecZero;
			pPlayer->pev->fixangle = TRUE;

			// notify other clients of player switching to spectator mode
			UTIL_ClientPrintAll(print_chat, UTIL_VarArgs("%s switched to spectator mode\n",
				(pev->netname && STRING(pev->netname)[0] != 0) ? STRING(pev->netname) : "unconnected"));
		}
		else
			UTIL_ClientPrint(pEntity, print_console, "Spectator mode is disabled.\n");

	}
	else if (FStrEq(pcmd, "specmode"))	// new spectator mode
	{
		CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pev);

		if (pPlayer->IsObserver())
			pPlayer->Observer_SetMode(atoi(CMD_ARGV(1)));
	}
	else if (FStrEq(pcmd, "closemenus"))
	{
		// just ignore it
	}
	else if (FStrEq(pcmd, "follownext"))	// follow next player
	{
		CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pev);

		if (pPlayer->IsObserver())
			pPlayer->Observer_FindNextPlayer(atoi(CMD_ARGV(1)) ? true : false);
	}
	else if (g_pGameRules->ClientCommand(GetClassPtr((CBasePlayer*)pev), pcmd))
	{
		// MenuSelect returns true only if the command is properly handled,  so don't print a warning
	}
	else
	{
		// tell the user they entered an unknown command
		char command[128];

		// check the length of the command (prevents crash)
		// max total length is 192 ...and we're adding a string below ("Unknown command: %s\n")
		strncpy(command, pcmd, 127);
		command[127] = '\0';

		// tell the user they entered an unknown command
		UTIL_ClientPrint(pEntity, print_console, UTIL_VarArgs("Unknown command: %s\n", command));
	}
}
