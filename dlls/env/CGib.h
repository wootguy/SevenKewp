#pragma once
#include "CBaseEntity.h"

enum merged_gib_bodies {
	MERGE_MDL_AGIBS,
	MERGE_MDL_BIGSHRAPNEL,
	MERGE_MDL_BLKOP_BODYGIBS,
	MERGE_MDL_BLKOP_ENGINEGIBS,
	MERGE_MDL_BLKOP_TAILGIBS,
	MERGE_MDL_BM_LEG,
	MERGE_MDL_BM_SACK,
	MERGE_MDL_BM_SHELL,
	MERGE_MDL_CEILINGGIBS,
	MERGE_MDL_CHROMEGIBS,
	MERGE_MDL_CINDERGIBS,
	MERGE_MDL_COMPUTERGIBS,
	MERGE_MDL_FLESHGIBS,
	MERGE_MDL_GLASSGIBS,
	MERGE_MDL_HGIBS,
	MERGE_MDL_MECHGIBS,
	MERGE_MDL_METALPLATEGIBS_GREEN,
	MERGE_MDL_METALPLATEGIBS,
	MERGE_MDL_OSPREY_BODYGIBS,
	MERGE_MDL_OSPREY_ENGINEGIBS,
	MERGE_MDL_OSPREY_TAILGIBS,
	MERGE_MDL_PIT_DRONE_GIBS,
	MERGE_MDL_STROOPER_GIBS,
	MERGE_MDL_VGIBS,
	MERGE_MDL_WOODGIBS,
	MERGE_MDL_GIB_MODELS,
};

// offset and body count for gib models
struct GibInfo {
	const char* model;		// individual gib model
	uint8_t mergeOffset;	// body offset in merged gibs model
	uint8_t bodyCount;		// number of bodies for gib type
};

EXPORT extern GibInfo g_gibInfo[MERGE_MDL_GIB_MODELS];

//
// A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
//
class EXPORT CGib : public CBaseEntity
{
public:
	void Spawn(const char* szGibModel);
	void BounceGibTouch(CBaseEntity* pOther);
	void StickyGibTouch(CBaseEntity* pOther);
	void SprayTouch(CBaseEntity* pOther);
	void WaitTillLand(void);
	void StartFadeOut(void);
	void LimitVelocity(void);
	void BreakThink();
	void SprayThink();

	virtual int	ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	static	void SpawnHeadGib(entvars_t* pevVictim);
	static	void SpawnMonsterGibs(entvars_t* pevVictim, int cGibs, int human);
	static	void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* gibModel,
		int gibModelBodyGroups, int bodyGroupSkip, int bodyOffset);
	static	void SpawnRandomMergedGibs(entvars_t* pevVictim, int cGibs, int gibModel, int bodyGroupSkip);
	static  void SpawnStickyGibs(entvars_t* pevVictim, Vector vecOrigin, int cGibs);

	int		m_bloodColor;
	int		m_cBloodDecals;
	int		m_material;
	float	m_lifeTime;
	float	m_bornTime;
	float	m_slideFriction; // additional friction applied while on ground
	float	m_lastBounceSound;
};