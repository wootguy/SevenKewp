#include "extdll.h"
#include "util.h"
#include "CRuleEntity.h"
#include "PluginManager.h"

#define SF_MOTD_ALL_PLAYERS 1	// open MOTD window for all players
#define SF_MOTD_LONG_MODE	2	// precache the file for clients to load from disk to increase max text length

#define MAX_MOTD_LENGTH 1536

class EXPORT CGameMotd : public CRulePointEntity
{
public:
	void	Spawn();
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	KeyValue(KeyValueData* pkvd);

	string_t m_title;
	string_t m_file;
};

LINK_ENTITY_TO_CLASS(game_motd, CGameMotd)


void CGameMotd::Spawn()
{
	if (pev->spawnflags & SF_MOTD_LONG_MODE) {
		PRECACHE_GENERIC(UTIL_VarArgs("maps/%s.txt", STRING(m_file)));
	}
}

void CGameMotd::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "motd_file"))
	{
		m_file = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "motd_title"))
	{
		m_title = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}


void CGameMotd::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	const char* motd_text = g_map_motd.c_str();

	bool allPlayers = pev->spawnflags & SF_MOTD_ALL_PLAYERS;
	bool longMode = pev->spawnflags & SF_MOTD_LONG_MODE;

	if (!longMode) {
		if (m_file) {
			int sz;
			char* motd_data = (char*)UTIL_LoadFile(UTIL_VarArgs("maps/%s.txt", STRING(m_file)), &sz);
			
			if (!motd_data) {
				EALERT(at_error, "Failed to open MOTD file 'maps/%s.txt'\n", STRING(m_file));
				return;
			}

			static char buffer[MAX_MOTD_LENGTH];
			strcpy_safe(buffer, motd_data, V_min(sz, MAX_MOTD_LENGTH));
			delete[] motd_data;
			motd_text = buffer;

			if (sz > MAX_MOTD_LENGTH) {
				EALERT(at_warning, "MOTD has too many characters (%d > %d). Consider enabling the long mode spawnflag.\n",
					sz, MAX_MOTD_LENGTH);
			}
		}
		else if (pev->message) {
			motd_text = STRING(pev->message);
		}
		else {
			motd_text = g_map_motd.c_str();
		}
	}

	if (!motd_text) {
		motd_text = "This map has no briefing.";
	}

	const char* mapName = STRING(gpGlobals->mapname);
	const char* title = m_title ? STRING(m_title) : mapName;

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

		if (!pPlayer) {
			continue;
		}

		if (!allPlayers && pPlayer != pActivator) {
			continue;
		}

		if (longMode) {
			if (m_file) {
				MESSAGE_BEGIN(MSG_ONE, gmsgVGUIMenu, NULL, pPlayer->edict());
				WRITE_BYTE(4);
				WRITE_STRING(STRING(m_file));
				MESSAGE_END();
			}
			else if (g_map_has_readme) {
				MESSAGE_BEGIN(MSG_ONE, gmsgVGUIMenu, NULL, pPlayer->edict());
				WRITE_BYTE(4);
				WRITE_STRING(STRING(gpGlobals->mapname));
				MESSAGE_END();
			}
			else if (g_map_has_readme2) {
				MESSAGE_BEGIN(MSG_ONE, gmsgVGUIMenu, NULL, pPlayer->edict());
				WRITE_BYTE(4);
				WRITE_STRING(UTIL_VarArgs("%s_readme", mapName));
				MESSAGE_END();
			}
		}
		else {
			MESSAGE_BEGIN(MSG_ONE, gmsgServerName, NULL, pPlayer->edict());
			WRITE_STRING(title);
			MESSAGE_END();

			UTIL_SendMotd(pPlayer, motd_text);

			MESSAGE_BEGIN(MSG_ONE, gmsgServerName, NULL, pPlayer->edict());
			WRITE_STRING(CVAR_GET_STRING("hostname"));
			MESSAGE_END();
		}
	}
}
