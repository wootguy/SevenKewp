#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CRuleEntity.h"

// CGameScore / game_score	-- award points to player / team 
//	Points +/- total
//	Flag: Allow negative scores					SF_SCORE_NEGATIVE
//	Flag: Award points to team in teamplay		SF_SCORE_TEAM

#define SF_SCORE_NEGATIVE			0x0001
#define SF_SCORE_TEAM				0x0002

class CGameScore : public CRulePointEntity
{
public:
	void	Spawn(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	KeyValue(KeyValueData* pkvd);

	inline	int		Points(void) { return pev->frags; }
	inline	BOOL	AllowNegativeScore(void) { return pev->spawnflags & SF_SCORE_NEGATIVE; }
	inline	BOOL	AwardToTeam(void) { return pev->spawnflags & SF_SCORE_TEAM; }

	inline	void	SetPoints(int points) { pev->frags = points; }

private:
};

LINK_ENTITY_TO_CLASS(game_score, CGameScore);


void CGameScore::Spawn(void)
{
	CRulePointEntity::Spawn();
}


void CGameScore::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "points"))
	{
		SetPoints(atoi(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}



void CGameScore::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	// Only players can use this
	if (pActivator->IsPlayer())
	{
		if (AwardToTeam())
		{
			pActivator->AddPointsToTeam(Points(), AllowNegativeScore());
		}
		else
		{
			pActivator->AddPoints(Points(), AllowNegativeScore());
		}
	}
}
