#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "TextMenu.h"
#include "CTriggerVote.h"

// CTriggerVote / trigger_vote -- trigger something based on player votes

LINK_ENTITY_TO_CLASS(trigger_vote, CTriggerVote)

void CTriggerVote::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszYesString"))
	{
		m_iszYesString = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if (FStrEq(pkvd->szKeyName, "m_iszNoString"))
	{
		m_iszNoString = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CTriggerVote::VoteThink() {
	int totalVotes = m_yesVotes + m_noVotes;
	int percentGot = m_yesVotes ? (m_yesVotes / (float)totalVotes)*100 : 0;
	int percentNeeded = pev->health;
	const char* yesString = m_iszYesString ? STRING(m_iszYesString) : "Yes";

	if (totalVotes == 0) {
		ALERT(at_logged, "trigger_vote: Nobody voted.\n");
		UTIL_ClientPrintAll(print_chat, "Vote failed. Nobody voted.\n");

		if (pev->noise) {
			FireTargets(STRING(pev->noise), this, this, USE_ON, 0.0f);
		}
	}
	else if (percentGot >= percentNeeded) {
		UTIL_ClientPrintAll(print_chat, UTIL_VarArgs("Vote passed with %d%% votes for '%s'.\n",
			percentGot, yesString));
		ALERT(at_logged, "trigger_vote: Vote passed with %d percent votes for '%s'.\n",
			percentGot, yesString);

		if (pev->target) {
			FireTargets(STRING(pev->target), this, this, USE_ON, 0.0f);
		}
	}
	else {
		UTIL_ClientPrintAll(print_chat, UTIL_VarArgs("Vote failed. %d%% voted '%s' but %d%% is required.\n",
			percentGot, yesString, percentNeeded));
		ALERT(at_logged, "trigger_vote: Vote failed. %d percent voted '%s' but %d is required.\n",
			percentGot, yesString, percentNeeded, "%");

		if (pev->netname) {
			FireTargets(STRING(pev->netname), this, this, USE_ON, 0.0f);
		}
	}
	
	SetThink(NULL);
	pev->nextthink = 0;
}

void CTriggerVote::MenuCallback(TextMenu* menu, edict_t* player, int itemNumber, TextMenuItem& item) {
	if (item.data[0] == 'y') {
		m_yesVotes++;
	}
	if (item.data[0] == 'n') {
		m_noVotes++;
	}

	UTIL_ClientPrintAll(print_console, UTIL_VarArgs("'%s' voted '%s'\n", STRING(player->v.netname), item.displayText.c_str()));
	UTIL_ClientPrint(player, print_center, UTIL_VarArgs("You voted '%s'\n", item.displayText.c_str()));
	ALERT(at_logged, "trigger_vote: '%s' voted '%s'\n", STRING(player->v.netname), item.displayText.c_str());
}

void CTriggerVote::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	const char* voteMessage = STRING(pev->message);
	const char* yesString = m_iszYesString ? STRING(m_iszYesString) : "Yes";
	const char* noString = m_iszNoString ? STRING(m_iszNoString) : "No";
	m_yesVotes = 0;
	m_noVotes = 0;

	UTIL_ClientPrintAll(print_console, UTIL_VarArgs("Started vote '%s'\n", voteMessage));
	ALERT(at_logged, "trigger_vote: Started vote '%s'\n", voteMessage);

	m_textMenu = TextMenu::init((edict_t*)NULL, &CTriggerVote::MenuCallback, this);
	m_textMenu->SetTitle(voteMessage);
	m_textMenu->AddItem(yesString, "y");
	m_textMenu->AddItem(noString, "n");
	m_textMenu->Open(clampi(pev->frags, 0, 255), 0, NULL);

	SetThink(&CTriggerVote::VoteThink);
	pev->nextthink = gpGlobals->time + pev->frags;
}
