#pragma once
#include "TextMenu.h"
#include "CPointEntity.h"

class CTriggerVote : public CPointEntity
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void VoteThink();
	void MenuCallback(TextMenu* menu, CBasePlayer* player, int itemNumber, TextMenuItem& item);

	string_t m_iszYesString;
	string_t m_iszNoString;
	TextMenu* m_textMenu;
	bool isActive;
	int m_yesVotes;
	int m_noVotes;
};