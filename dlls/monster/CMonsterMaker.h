#pragma once
// Monstermaker spawnflags
#define	SF_MONSTERMAKER_START_ON	1 // start active ( if has targetname )
#define	SF_MONSTERMAKER_CYCLIC		4 // drop one monster every time fired.
#define SF_MONSTERMAKER_MONSTERCLIP	8 // Children are blocked by monsterclip
#define SF_MONSTERMAKER_PRISONER	16 // Children are prisoners
#define SF_MONSTERMAKER_WAIT_SCRIPT	128 // Children wait for a scripted sequence

#define MAX_XENMAKER_BEAMS 64 // each beam will need a TE message so don't go too crazy...
#define XENMAKER_SOUND1 "debris/beamstart7.wav"
#define XENMAKER_SOUND2 "debris/beamstart2.wav"

enum blocked_spawn_modes {
	SPAWN_BLOCK_LEGACY, // fail the spawn if blocked
	SPAWN_BLOCK_WAIT, // wait for the blockage to clear, then spawn
	SPAWN_BLOCK_IGNORE // spawn even if blocked (xenmaker mode)
};

//=========================================================
// MonsterMaker - this ent creates monsters during the game.
//=========================================================
class EXPORT CMonsterMaker : public CBaseMonster
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }
	virtual	BOOL IsNormalMonster(void) { return FALSE; }
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	void ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void CyclicUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void MakerThink(void);
	void BlockedCyclicThink();
	void DeathNotice(entvars_t* pevChild);// monster maker children use this to tell the monster maker that they have died.
	void MakeMonster(void);
	void XenmakerEffect();
	virtual void Nerf(); // reduce monster count if there is no good reason to have multiple spawns
	bool NerfMonsterCounters(string_t target);
	int Classify(void);
	virtual BOOL HasTarget(string_t targetname);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	string_t m_iszMonsterClassname;// classname of the monster(s) that will be created.

	int	 m_cNumMonsters;// max number of monsters this ent can create


	int  m_cLiveChildren;// how many monsters made by this monster maker that are currently alive
	int	 m_iMaxLiveChildren;// max number of monsters that this maker may have out at one time.

	float m_flGround; // z coord of the ground under me, used to make sure no monsters are under the maker when it drops a new child

	BOOL m_fActive;
	BOOL m_fFadeChildren;// should we make the children fadeout?

	int m_blockedSpawnMode;

	// env_xenmaker effect settings
	float m_flBeamRadius;
	int m_iBeamAlpha;
	int m_iBeamCount;
	Vector m_vBeamColor;
	float m_flLightRadius;
	Vector m_vLightColor;
	float m_flStartSpriteFramerate;
	float m_flStartSpriteScale;
	int m_iStartSpriteAlpha;
	int m_xenSpriteIdx;
	int m_xenBeamSpriteIdx;
	float m_nextXenSound;
	int m_changeRenderMode;

	string_t m_xenmakerTemplate; // grab xenmaker settings from another entity

	string_t m_weaponModelV;
	string_t m_weaponModelP;
	string_t m_weaponModelW;
};