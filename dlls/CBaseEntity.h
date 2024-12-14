#pragma once
#include "EHandle.h"
#include "saverestore.h"

class CBaseEntity;
class CBaseMonster;
class CBasePlayerWeapon;
class CTalkSquadMonster;
class CBaseToggle;
class CBaseAnimating;
class CItemInventory;

void* GET_PRIVATE(const edict_t* pent);

enum entindex_priority {
	// entity should have an index lower than 512 so that sprite attachments work
	// (skin is sent with 9 bits in delta.lst and this can't be changed for vanilla HL clients)
	ENTIDX_PRIORITY_HIGH,

	// entity is likely to be sent to clients (monsters, sprites, doors, etc.)
	ENTIDX_PRIORITY_NORMAL,

	// entity is server-side only and can have an index above the client limit
	// TODO: make a separate edict list for these, things like logic ents are basically a scripting engine
	ENTIDX_PRIORITY_LOW,
};

typedef void (CBaseEntity::* BASEPTR)(void);
typedef void (CBaseEntity::* ENTITYFUNCPTR)(CBaseEntity* pOther);
typedef void (CBaseEntity::* USEPTR)(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

// Ugly technique to override base member functions
// Normally it's illegal to cast a pointer to a member function of a derived class to a pointer to a 
// member function of a base class.  static_cast is a sleezy way around that problem.

#ifdef _DEBUG

#define SetThink( a ) ThinkSet( static_cast <void (CBaseEntity::*)(void)> (a), #a )
#define SetTouch( a ) TouchSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )
#define SetUse( a ) UseSet( static_cast <void (CBaseEntity::*)(	CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a), #a )
#define SetBlocked( a ) BlockedSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )

#else

#define SetThink( a ) m_pfnThink = static_cast <void (CBaseEntity::*)(void)> (a)
#define SetTouch( a ) m_pfnTouch = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)
#define SetUse( a ) m_pfnUse = static_cast <void (CBaseEntity::*)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a)
#define SetBlocked( a ) m_pfnBlocked = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)

#endif

//
// Converts a entvars_t * to a class pointer
// It will allocate the class and entity if necessary
//
template <class T> T* GetClassPtr(T* a)
{
	entvars_t* pev = (entvars_t*)a;

	// allocate entity if necessary
	bool allocate = pev == NULL;
	if (allocate)
		pev = VARS(CREATE_ENTITY());

	// get the private data
	a = (T*)GET_PRIVATE(ENT(pev));

	if (a == NULL)
	{
		// allocate private data 
		a = new(pev) T; // TODO: shouldn't this use malloc? What does the engine do?
		a->pev = pev;
	}

	// only relocate if a new entity was allocated, else EHANDLEs will break
	return allocate ? (T*)RelocateEntIdx(a) : a;
}

struct InventoryRules {
	string_t	item_name_required; 			// Inventory : Require these item(s)
	string_t	item_group_required;			// Inventory : Require an item from these group(s)
	int			item_group_required_num;		// Inventory : Number of item(s) from the required group(s) required(0 = all)
	string_t	item_name_canthave; 			// Inventory : Must not have these item(s)
	string_t	item_group_canthave;			// Inventory : Must not have an item in these group(s)
	int			item_group_canthave_num; 		// Inventory : Number of item(s) from the can't have group(s) (0 = all)
	bool		pass_ignore_use_triggers; 		// On pass : Ignore item's on use triggers?
	string_t	pass_drop_item_name; 			// On pass : Drop item(s)
	string_t	pass_drop_item_group; 			// On pass : Drop item(s) in these group(s)
	bool		pass_ignore_drop_triggers;		// On pass : Ignore item's on drop triggers?
	string_t	pass_return_item_name;			// On pass : Return item(s)
	string_t	pass_return_item_group; 		// On pass : Return item(s) in these group(s)
	bool		pass_ignore_return_triggers;	// On pass : Ignore item's on return triggers?
	string_t	pass_destroy_item_name; 		// On pass : Destroy item(s)
	string_t	pass_destroy_item_group;		// On pass : Destroy item(s) in these group(s)
	bool		pass_ignore_destroy_triggers; 	// On pass : Ignore item's on destroy triggers?
	string_t	target_on_fail; 				// Target : Inventory rules failed
};

//
// Base Entity.  All entity types derive from this
//
class EXPORT CBaseEntity
{
public:
	// Constructor.  Set engine to use C/C++ callback functions
	// pointers to engine data
	entvars_t* pev;		// Don't need to save/restore this pointer, the engine resets it

	// path corners
	EHANDLE m_hGoalEnt;// path corner we are heading towards
	EHANDLE m_hLink;// used for temporary link-list operations. 

	// initialization functions
	virtual void	Spawn(void) { return; }
	virtual void	Precache(void) { return; }
	virtual void	KeyValue(KeyValueData* pkvd);
	virtual CKeyValue GetKeyValue(const char* keyName);
	CKeyValue GetCustomKeyValue(const char* keyName);
	std::unordered_map<std::string, CKeyValue>* GetCustomKeyValues();
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	virtual int		ObjectCaps(void) { return FCAP_ACROSS_TRANSITION; }
	virtual void	Activate(void) {}
	virtual int		GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	virtual Vector	GetTargetOrigin() { return pev->origin; } // origin used for monster pathing and targetting

	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	virtual void	SetObjectCollisionBox(void);

	// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
	// still realize that they are teammates. (overridden for monsters that form groups)
	virtual int Classify(void) { return m_Classify; };
	virtual void DeathNotice(entvars_t* pevChild) {}// monster maker children use this to tell the monster maker that they have died.


	static	TYPEDESCRIPTION m_SaveData[];

	virtual void	TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	virtual int		TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual int		TakeHealth(float flHealth, int bitsDamageType, float healthcap = 0);
	virtual void	Killed(entvars_t* pevAttacker, int iGib);
	virtual int		BloodColor(void) { return DONT_BLEED; }
	virtual void	TraceBleed(float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	virtual BOOL    IsTriggered(CBaseEntity* pActivator) { return TRUE; }
	virtual CBaseMonster* MyMonsterPointer(void) { return NULL; }
	virtual CBasePlayer* MyPlayerPointer(void) { return NULL; }
	virtual CBasePlayerWeapon* GetWeaponPtr(void) { return NULL; };
	virtual CTalkSquadMonster* MyTalkSquadMonsterPointer(void) { return NULL; }
	virtual CBaseToggle* MyTogglePointer(void) { return NULL; }
	virtual CBaseAnimating* MyAnimatingPointer(void) { return NULL; }
	virtual CItemInventory* MyInventoryPointer(void) { return NULL; }
	virtual	int		GetToggleState(void) { return TS_AT_TOP; }
	virtual void	AddPoints(int score, BOOL bAllowNegativeScore) {}
	virtual void	AddPointsToTeam(int score, BOOL bAllowNegativeScore) {}
	virtual BOOL	AddPlayerItem(CBasePlayerItem* pItem) { return 0; }
	virtual BOOL	RemovePlayerItem(CBasePlayerItem* pItem) { return 0; }
	virtual int 	GiveAmmo(int iAmount, const char* szName, int iMax) { return -1; };
	virtual float	GetDelay(void) { return 0; }
	virtual int		IsMoving(void) { return pev->velocity != g_vecZero; }
	virtual void	OverrideReset(void) {}
	virtual int		DamageDecal(int bitsDamageType);
	// This is ONLY used by the node graph to test movement through a door
	virtual void	SetToggleState(int state) {}
	virtual void    StartSneaking(void) {}
	virtual void    StopSneaking(void) {}
	virtual BOOL	OnControls(entvars_t* otherPev) { return FALSE; }
	virtual BOOL    IsSneaking(void) { return FALSE; }
	virtual BOOL	IsAlive(void) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL	IsBSPModel(void) { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	virtual BOOL	ReflectGauss(void) { return (IsBSPModel() && !pev->takedamage); }
	virtual BOOL	HasTarget(string_t targetname) { return FStrEq(STRING(targetname), STRING(pev->target)); }
	virtual BOOL    IsInWorld(void);
	virtual	BOOL	IsMonster(void) { return FALSE; }
	virtual	BOOL	IsNormalMonster(void) { return FALSE; } // is this what you'd expect to be a monster? (not a monstermaker/grenade/etc.)
	virtual	BOOL	IsPlayer(void) { return FALSE; }
	virtual	BOOL	IsPlayerCorpse(void) { return FALSE; }
	virtual BOOL	IsNetClient(void) { return FALSE; }
	virtual BOOL	IsBreakable(void) { return FALSE; }
	virtual BOOL	IsMachine(void) { return FALSE; };
	virtual BOOL	IsWeather(void) { return FALSE; };
	virtual BOOL	IsBeam(void) { return FALSE; };
	virtual const char* TeamID(void) { return ""; }
	virtual const char* DisplayName() { return STRING(pev->classname); }
	virtual const char* GetDeathNoticeWeapon() { return STRING(pev->classname); };


	//	virtual void	SetActivator( CBaseEntity *pActivator ) {}
	virtual CBaseEntity* GetNextTarget(void);

	// fundamental callbacks
	void (CBaseEntity ::* m_pfnThink)(void);
	void (CBaseEntity ::* m_pfnTouch)(CBaseEntity* pOther);
	void (CBaseEntity ::* m_pfnUse)(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void (CBaseEntity ::* m_pfnBlocked)(CBaseEntity* pOther);

	virtual void Think(void) { if (m_pfnThink) (this->*m_pfnThink)(); };
	virtual void Touch(CBaseEntity* pOther) { if (m_pfnTouch) (this->*m_pfnTouch)(pOther); };
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
	{
		if (m_pfnUse)
			(this->*m_pfnUse)(pActivator, pCaller, useType, value);
	}
	virtual void Blocked(CBaseEntity* pOther) { if (m_pfnBlocked) (this->*m_pfnBlocked)(pOther); };

	// allow engine to allocate instance data
	void* operator new(size_t stAllocateBlock, entvars_t* pev)
	{
		return (void*)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
	};

	// don't use this.
#if _MSC_VER >= 1200 // only build this code if MSVC++ 6.0 or higher
	void operator delete(void* pMem, entvars_t* pev)
	{
		pev->flags |= FL_KILLME;
	};
#endif

	virtual void UpdateOnRemove(void);

	// common member functions
	void SUB_Remove(void);
	void SUB_DoNothing(void);
	void SUB_StartFadeOut(void);
	void SUB_FadeOut(void);
	void SUB_CallUseToggle(void) { this->Use(this, this, USE_TOGGLE, 0); }
	int			ShouldToggle(USE_TYPE useType, BOOL currentState);
	void		FireBullets(ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting, Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t* pevAttacker = NULL);
	Vector		FireBulletsPlayer(ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting, Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t* pevAttacker = NULL, int shared_rand = 0);

	virtual CBaseEntity* Respawn(void) { return NULL; }

	void SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value);
	void SUB_KillTarget(const char* target);
	// Do the bounding boxes of these two intersect?
	int		Intersects(CBaseEntity* pOther);
	void	MakeDormant(void);
	int		IsDormant(void);
	BOOL    IsLockedByMaster(void) { return FALSE; }

	static CBaseEntity* Instance(const edict_t* pent)
	{
		if (!pent)
			pent = ENT(0);
		CBaseEntity* pEnt = (CBaseEntity*)GET_PRIVATE(pent);
		return pEnt;
	}

	static CBaseEntity* Instance(entvars_t* pev) { return Instance(ENT(pev)); }
	static CBaseEntity* Instance(int eoffset) { return Instance(ENT(eoffset)); }

	CBaseMonster* GetMonsterPointer(entvars_t* pevMonster)
	{
		CBaseEntity* pEntity = Instance(pevMonster);
		if (pEntity)
			return pEntity->MyMonsterPointer();
		return NULL;
	}
	CBaseMonster* GetMonsterPointer(edict_t* pentMonster)
	{
		CBaseEntity* pEntity = Instance(pentMonster);
		if (pEntity)
			return pEntity->MyMonsterPointer();
		return NULL;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
#ifdef _DEBUG
	void FunctionCheck(void* pFunction, char* name)
	{
		//if (pFunction && !NAME_FOR_FUNCTION((uint32)pFunction) )
		//	ALERT( at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (uint32)pFunction );
	}

	BASEPTR	ThinkSet(BASEPTR func, char* name)
	{
		m_pfnThink = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnThink)))), name);
		return func;
	}
	ENTITYFUNCPTR TouchSet(ENTITYFUNCPTR func, char* name)
	{
		m_pfnTouch = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnTouch)))), name);
		return func;
	}
	USEPTR	UseSet(USEPTR func, char* name)
	{
		m_pfnUse = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnUse)))), name);
		return func;
	}
	ENTITYFUNCPTR	BlockedSet(ENTITYFUNCPTR func, char* name)
	{
		m_pfnBlocked = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnBlocked)))), name);
		return func;
	}

#endif


	// virtual functions used by a few classes

	// used by monsters that are created by the MonsterMaker
	virtual	void UpdateOwner(void) { return; };


	//
	static CBaseEntity* Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner = NULL, std::unordered_map<std::string, std::string> keys = std::unordered_map<std::string, std::string>());

	virtual BOOL FBecomeProne(void) { return FALSE; };
	edict_t* edict() { return ENT(pev); };
	EOFFSET eoffset() { return OFFSET(pev); };
	int	  entindex() { return ENTINDEX(edict()); };

	virtual Vector Center() { return (pev->absmax + pev->absmin) * 0.5; }; // center point of entity
	virtual Vector EyePosition() { return pev->origin + pev->view_ofs; };			// position of eyes
	virtual Vector EarPosition() { return pev->origin + pev->view_ofs; };			// position of ears
	virtual Vector BodyTarget(const Vector& posSrc) { return Center(); };		// position to shoot at

	virtual int Illumination() { return GETENTITYILLUM(ENT(pev)); };

	virtual	BOOL FVisible(CBaseEntity* pEntity);
	virtual	BOOL FVisible(const Vector& vecOrigin);

	virtual void SetClassify(int iNewClassify);
	virtual int IRelationship(CBaseEntity* pTarget);
	static int IRelationship(int attackerClass, int victimClass);
	bool ShouldBlockFriendlyFire(entvars_t* attacker);

	// can the player using this entity physically touch the ent with their hand?
	// or is there something in the way? (player use code assumes arms have noclip)
	bool CanReach(CBaseEntity* toucher);

	// true if this entity is in the PVS of the given player
	inline bool InPVS(edict_t* player) { return m_pvsPlayers & PLRBIT(player); }

	// true if the entity is in the PAS of the given player
	inline bool InPAS(edict_t* player) { return m_pasPlayers & PLRBIT(player); }

	// true if the entity is networked to the given player
	inline bool isVisibleTo(edict_t* player) { return m_netPlayers & PLRBIT(player); }

	// true if the entity was flagged to be hidden from the given player
	inline bool isHiddenFrom(edict_t* player) { return m_hidePlayers & PLRBIT(player); }

	// flag this entity to be visible/invisible to the given player
	inline void SetVisible(edict_t* player, bool visible) {
		if (visible) m_hidePlayers &= ~PLRBIT(player);
		else m_hidePlayers |= PLRBIT(player);
	}

	bool RunInventoryRules(CBaseEntity* ent); // returns false if entity inventory forbids activation

	virtual float GetDamageModifier() { return 1.0f; }

	// Smooths the movement of projectile models or sprites that use one of the following movetypes:
	//		NONE, STEP, WALK, FLY.
	// Call this in a think function which has a constant interval, and pass that interval as flInterval.
	// Angles cannot be controlled. In most cases you're better off using a different movetype.
	// (e.g. FLY -> BOUNCE with gravity=FLT_MIN and friction=1.0f)
	// EFLAG_SLERP only interpolates animated MDLs that are moved manually (not with velocity)
	// Benefits to using this over an interpolated movetype:
	// - more accurate position seen on the client (client predicts future location using velocity)
	// - projectiles render immediately, not ex_interp seconds after the entity is created
	//   (this also fixes a glitch when using attachments on the interpolated entity)
	void ParametricInterpolation(float flInterval);

	// used to override state sent to player
	virtual int AddToFullPack(struct entity_state_s* state, CBasePlayer* player) { return 1; }

	virtual Vector GetLookDirection();

	//We use this variables to store each ammo count.
	int ammo_9mm;
	int ammo_357;
	int ammo_bolts;
	int ammo_buckshot;
	int ammo_rockets;
	int ammo_uranium;
	int ammo_hornets;
	int ammo_argrens;
	//Special stuff for grenades and satchels.
	float m_flStartThrow;
	float m_flReleaseThrow;
	int m_chargeReady;
	int m_fInAttack;

	enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
	int m_fireState;
	int	m_Classify;		// Classify, to let mappers override the default

	InventoryRules	m_inventoryRules;
	bool m_isFadingOut; // is a corpse fading out

	uint32_t m_pasPlayers; // players in the audible set of this entity (invalid for invisible ents)
	uint32_t m_pvsPlayers; // players in the visible set of this entity (invalid for invisible ents)
	uint32_t m_netPlayers; // players this entity has been networked to (AddToFullPack returned 1)
	uint32_t m_hidePlayers; // players this entity will be hidden from (AddToFullPack)

private:
	bool TestInventoryRules(CBaseMonster* mon, std::unordered_set<CItemInventory*>& usedItems, const char** errorMsg);
};

inline void* GET_PRIVATE(const edict_t* pent)
{
	if (pent) {
		CBaseEntity* bent = (CBaseEntity*)pent->pvPrivateData;

		if (bent && bent->pev != &pent->v) {
			// TODO: pev was linked wrong somehow. mem corruption?
			ALERT(at_error, "Entity pev not linked to edict %d pev (%s)\n",
				ENTINDEX(pent), STRING(pent->v.classname));
			bent->pev = (entvars_t*)&pent->v;
			return NULL;
		}

		return bent;
	}
	return NULL;
}