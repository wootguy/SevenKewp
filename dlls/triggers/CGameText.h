#include "CRuleEntity.h"

//
// CGameText / game_text	-- NON-Localized HUD Message (use env_message to display a titles.txt message)
//	Flag: All players					SF_ENVTEXT_ALLPLAYERS
//
#define SF_ENVTEXT_ALLPLAYERS			0x0001
#define SF_NO_CONSOLE_ECHO				0x0002


class EXPORT CGameText : public CRulePointEntity
{
public:
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	KeyValue(KeyValueData* pkvd);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	inline	BOOL	MessageToAll(void) { return (pev->spawnflags & SF_ENVTEXT_ALLPLAYERS); }
	inline	void	MessageSet(const char* pMessage) { pev->message = ALLOC_STRING(pMessage); }
	inline	const char* MessageGet(void) { return STRING(pev->message); }

private:

	hudtextparms_t	m_textParms;
};