#include "CRuleEntity.h"

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