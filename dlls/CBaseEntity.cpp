#include	"extdll.h"
#include	"util.h"
#include	"saverestore.h"
#include	"hlds_hooks.h"
#include	"decals.h"
#include	"gamerules.h"
#include	"game.h"
#include	"weapons.h"
#include	"skill.h"
#include	"nodes.h"
#include "lagcomp.h"
#include "monsters.h"
#include "CItemInventory.h"
#include "CBasePlayer.h"
#include "explode.h"
#include "te_effects.h"

extern CGraph WorldGraph;
extern DLL_GLOBAL Vector		g_vecAttackDir;

std::vector<HashMap<CKeyValue>> g_customKeyValues;
CKeyValue g_emptyKeyValue;

CKeyValue GetEntvarsKeyvalue(entvars_t* pev, const char* keyName);

// Global Savedata for Delay
TYPEDESCRIPTION	CBaseEntity::m_SaveData[] =
{
	DEFINE_FIELD(CBaseEntity, m_hGoalEnt, FIELD_EHANDLE),

	DEFINE_FIELD(CBaseEntity, m_pfnThink, FIELD_FUNCTION),		// UNDONE: Build table of these!!!
	DEFINE_FIELD(CBaseEntity, m_pfnTouch, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseEntity, m_pfnUse, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseEntity, m_pfnBlocked, FIELD_FUNCTION),
};

const char* CBaseEntity::pSoundsWood[] =
{
	"debris/wood1.wav",
	"debris/wood2.wav",
	"debris/wood3.wav",
};

const char* CBaseEntity::pSoundsFlesh[] =
{
	"debris/flesh1.wav",
	"debris/flesh2.wav",
	"debris/flesh3.wav",
	"debris/flesh5.wav",
	"debris/flesh6.wav",
	"debris/flesh7.wav",
};

const char* CBaseEntity::pSoundsMetal[] =
{
	"debris/metal1.wav",
	"debris/metal2.wav",
	"debris/metal3.wav",
};

const char* CBaseEntity::pSoundsConcrete[] =
{
	"debris/concrete1.wav",
	"debris/concrete2.wav",
	"debris/concrete3.wav",
};


const char* CBaseEntity::pSoundsGlass[] =
{
	"debris/glass1.wav",
	"debris/glass2.wav",
	"debris/glass3.wav",
};

const char* CBaseEntity::pBustSoundsWood[] =
{
	"debris/bustcrate1.wav",
	"debris/bustcrate2.wav",
};

const char* CBaseEntity::pBustSoundsFlesh[] =
{
	"debris/bustflesh1.wav",
	"debris/bustflesh2.wav",
};

const char* CBaseEntity::pBustSoundsMetal[] =
{
	"debris/bustmetal1.wav",
	"debris/bustmetal2.wav",
};

const char* CBaseEntity::pBustSoundsConcrete[] =
{
	"debris/bustconcrete1.wav",
	"debris/bustconcrete2.wav",
};

const char* CBaseEntity::pBustSoundsGlass[] =
{
	"debris/bustglass1.wav",
	"debris/bustglass2.wav",
};


// give health
int CBaseEntity::TakeHealth(float flHealth, int bitsDamageType, float healthcap)
{
	if (!pev->takedamage)
		return 0;

	if (healthcap <= 0) {
		healthcap = pev->max_health;
	}

	// heal
	if (pev->health >= healthcap)
		return 0;

	pev->health += flHealth;

	if (pev->health > healthcap)
		pev->health = healthcap;

	return 1;
}

// inflict damage on this entity.  bitsDamageType indicates type of damage inflicted, ie: DMG_CRUSH
int CBaseEntity::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (!pev->takedamage)
		return 0;

	Vector vecTemp;

	if (m_breakFlags & FL_BREAK_IS_BREAKABLE) {
		if (m_breakFlags & FL_BREAK_TRIGGER_ONLY)
			return 0;
		if ((m_breakFlags & FL_BREAK_EXPLOSIVES_ONLY) && !(bitsDamageType & (DMG_BLAST | DMG_MORTAR)))
			return 0;
		if ((m_breakFlags & FL_BREAK_IMMUNE_TO_CLIENTS) && (pevAttacker->flags & FL_CLIENT))
			return 0;
		if (flDamage > 0 && ShouldBlockFriendlyFire(pevAttacker))
			return 0;
		if (!IsBreakable())
			return 0;

		// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
		// (that is, no actual entity projectile was involved in the attack so use the shooter's origin). 
		if (pevAttacker == pevInflictor) {
			vecTemp = (pev->absmin + (pev->size * 0.5)) - pevInflictor->origin;

			bool isPlayerMelee = (pevAttacker->flags & FL_CLIENT) && (bitsDamageType & DMG_CLUB);
			bool canInstantBreak = m_breakFlags & FL_BREAK_INSTANT;

			if (flDamage > 0 && isPlayerMelee && canInstantBreak) {
				bool isWrench = FStrEq(getActiveWeapon(pevAttacker), "weapon_pipewrench");
				bool isCrowbar = FStrEq(getActiveWeapon(pevAttacker), "weapon_crowbar");
				bool weakToWrench = m_breakWeapon == BREAK_INSTANT_WRENCH;
				bool weakToCrowbar = m_breakWeapon != BREAK_INSTANT_WRENCH;

				if ((weakToWrench && isWrench) || (weakToCrowbar && isCrowbar)) {
					flDamage = pev->health;
				}
			}
		}
		else // an actual missile was involved.
		{
			vecTemp = (pev->absmin + (pev->size * 0.5)) - pevInflictor->origin;
		}

		// Breakables take double damage from the crowbar
		if (bitsDamageType & DMG_CLUB)
			flDamage *= 2;

		// Boxes / glass / etc. don't take much poison damage, just the impact of the dart - consider that 10%
		if (bitsDamageType & DMG_POISON)
			flDamage *= 0.1;

		// this global is still used for glass and other non-monster killables, along with decals.
		g_vecAttackDir = vecTemp.Normalize();

		// Give points if this breakable triggers something (tanks), unless it's supposed to be invincible
		// and the mapper didn't know to use the "only trigger" flag. Points are halved to compensate for
		// many breakables being useless and taking no skill to destroy.
		// TODO: determine if a breakable is useless. Hard problem. Opening a shortcut is not useless.
		// Need the path finder and map solver done first.
		CBaseDelay* delay = MyDelayPointer();
		bool hasTriggers = pev->target || m_breakTrigger || (delay && delay->m_iszKillTarget);
		if (hasTriggers && pev->max_health <= 10000)
			GiveScorePoints(pevAttacker, flDamage * 0.5f);

		// do the damage
		pev->health = V_min(pev->max_health, pev->health - flDamage);

		if (pev->health <= 0)
		{
			Killed(pevAttacker, GIB_NORMAL);
			BreakableDie(Instance(pevAttacker));
			return 0;
		}

		// Make a shard noise each time func breakable is hit.
		// Don't play shard noise if cbreakable actually died.
		if (flDamage > 0)
			BreakableDamageSound();

		return 1;
	}

	// UNDONE: some entity types may be immune or resistant to some bitsDamageType

	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin). 
	if (pevAttacker == pevInflictor)
	{
		vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
	}
	else
		// an actual missile was involved.
	{
		vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
	}

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();

	// save damage based on the target's armor level

	// figure momentum add (don't let hurt brushes or other triggers move player)
	if ((!FNullEnt(pevInflictor)) && (pev->movetype == MOVETYPE_WALK || pev->movetype == MOVETYPE_STEP) && (pevAttacker->solid != SOLID_TRIGGER))
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();

		float flForce = flDamage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

		if (flForce > 1000.0)
			flForce = 1000.0;
		pev->velocity = pev->velocity + vecDir * flForce;
	}

	// do the damage
	pev->health -= flDamage;
	if (pev->health <= 0)
	{
		Killed(pevAttacker, GIB_NORMAL);

		if (IsMonster()) {
			g_pGameRules->DeathNotice((CBaseMonster*)this, pevAttacker, pevInflictor);
		}

		return 0;
	}

	return 1;
}

void CBaseEntity::Killed(entvars_t* pevAttacker, int iGib)
{
	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time;
}

CBaseEntity* CBaseEntity::GetNextTarget(void)
{
	if (FStringNull(pev->target))
		return NULL;
	return UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
}

void CBaseEntity::KeyValue(KeyValueData* pkvd) {
	// custom keyvalues (e.g. "$i_energy")
	if (strlen(pkvd->szKeyName) > 3 && pkvd->szKeyName[0] == '$' && pkvd->szKeyName[2] == '_') {
		CKeyValue value = g_emptyKeyValue;

		if (pkvd->szKeyName[1] == 'i') {
			value.keyType = KEY_TYPE_INT;
			value.iVal = atoi(pkvd->szValue);
		}
		else if (pkvd->szKeyName[1] == 'f') {
			value.keyType = KEY_TYPE_FLOAT;
			value.fVal = atof(pkvd->szValue);
		}
		else if (pkvd->szKeyName[1] == 'v') {
			value.keyType = KEY_TYPE_VECTOR;
			UTIL_StringToVector(value.vVal, pkvd->szValue);
		}
		else if (pkvd->szKeyName[1] == 's') {
			value.keyType = KEY_TYPE_STRING;
			value.sVal = ALLOC_STRING(pkvd->szValue);
		}
		else {
			pkvd->fHandled = FALSE;
			return;
		}

		HashMap<CKeyValue>* customKeys = GetCustomKeyValues();
		CKeyValue* val = customKeys->get(pkvd->szKeyName);

		if (val) {
			value.keyName = val->keyName; // reuse the key name to save string memory
		}
		else {
			value.keyName = STRING(ALLOC_STRING(pkvd->szKeyName));
		}

		customKeys->put(pkvd->szKeyName, value);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "classify"))
	{
		SetClassify(atoi(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_name_required"))
	{
		m_inventoryRules.item_name_required = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_group_required"))
	{
		m_inventoryRules.item_group_required = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_group_required_num"))
	{
		m_inventoryRules.item_group_required_num = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_name_canthave"))
	{
		m_inventoryRules.item_name_canthave = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_group_canthave"))
	{
		m_inventoryRules.item_group_canthave = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_group_canthave_num"))
	{
		m_inventoryRules.item_group_canthave_num = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pass_ignore_use_triggers"))
	{
		m_inventoryRules.pass_ignore_use_triggers = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pass_drop_item_name"))
	{
		m_inventoryRules.pass_drop_item_name = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pass_drop_item_group"))
	{
		m_inventoryRules.pass_drop_item_group = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pass_ignore_drop_triggers"))
	{
		m_inventoryRules.pass_ignore_drop_triggers = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pass_return_item_name"))
	{
		m_inventoryRules.pass_return_item_name = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pass_return_item_group"))
	{
		m_inventoryRules.pass_return_item_group = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pass_ignore_return_triggers"))
	{
		m_inventoryRules.pass_ignore_return_triggers = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pass_destroy_item_name"))
	{
		m_inventoryRules.pass_destroy_item_name = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pass_destroy_item_group"))
	{
		m_inventoryRules.pass_destroy_item_group = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pass_ignore_destroy_triggers"))
	{
		m_inventoryRules.pass_ignore_destroy_triggers = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_fail"))
	{
		m_inventoryRules.target_on_fail = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "displayname"))
	{
		m_displayName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "explosion"))
	{
		if (!stricmp(pkvd->szValue, "directed") || atoi(pkvd->szValue) == 1)
			m_breakFlags |= FL_BREAK_DIRECTIONAL_GIBS;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "material"))
	{
		int i = atoi(pkvd->szValue);

		// 0:glass, 1:metal, 2:flesh, 3:wood
		if ((i < 0) || (i >= matLastMaterial))
			m_breakMaterial = matWood;
		else
			m_breakMaterial = (Materials)i;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "gibmodel"))
	{
		m_breakModel = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "explodemagnitude"))
	{
		m_breakExplodeMag = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "weapon"))
	{
		m_breakWeapon = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "breakable"))
	{
		if (atoi(pkvd->szValue)) {
			m_breakFlags |= FL_BREAK_IS_BREAKABLE;
			pev->takedamage = DAMAGE_YES;
		}
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fireonbreak"))
	{
		m_breakTrigger = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "instantbreak"))
	{
		m_breakFlags |= atoi(pkvd->szValue) ? FL_BREAK_INSTANT : 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "onlytrigger"))
	{
		m_breakFlags |= atoi(pkvd->szValue) ? FL_BREAK_TRIGGER_ONLY : 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "explosivesonly"))
	{
		m_breakFlags |= atoi(pkvd->szValue) ? FL_BREAK_EXPLOSIVES_ONLY : 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "breakontrigger"))
	{
		m_breakFlags |= atoi(pkvd->szValue) ? FL_BREAK_CAN_TRIGGER : 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "repairable"))
	{
		m_breakFlags |= atoi(pkvd->szValue) ? FL_BREAK_REPAIRABLE : 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "immunetoclients"))
	{
		m_breakFlags |= atoi(pkvd->szValue) ? FL_BREAK_IMMUNE_TO_CLIENTS : 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "soundlist"))
	{
		m_soundReplacementKey = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		pkvd->fHandled = FALSE;
	}
}

CKeyValue CBaseEntity::GetCustomKeyValue(const char* keyName) {
	if (strlen(keyName) <3 || keyName[0] != '$' || keyName[2] != '_') { // don't add a space there
		return g_emptyKeyValue;
	}

	HashMap<CKeyValue>* customKeys = GetCustomKeyValues();
	CKeyValue* key = customKeys->get(keyName);

	if (key) {
		return *key;
	}
	
	ALERT(at_console, "%s (%s) has no custom key named %s\n",
		STRING(pev->targetname), STRING(pev->classname), keyName);
	return g_emptyKeyValue;
}

CKeyValue CBaseEntity::GetKeyValue(const char* keyName) {
	CKeyValue customKey = GetCustomKeyValue(keyName);
	if (customKey.keyType) {
		return customKey;
	}
	return GetEntvarsKeyvalue(pev, keyName);
}

HashMap<CKeyValue>* CBaseEntity::GetCustomKeyValues() {
	if (g_customKeyValues.empty())
		g_customKeyValues.resize(gpGlobals->maxEntities);

	return &g_customKeyValues[entindex()];
}

int CBaseEntity::Save(CSave& save)
{
	if (save.WriteEntVars("ENTVARS", pev))
		return save.WriteFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

	return 0;
}

int CBaseEntity::Restore(CRestore& restore)
{
	int status;

	status = restore.ReadEntVars("ENTVARS", pev);
	if (status)
		status = restore.ReadFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

	if (pev->modelindex != 0 && !FStringNull(pev->model))
	{
		Vector mins, maxs;
		mins = pev->mins;	// Set model is about to destroy these
		maxs = pev->maxs;


		PRECACHE_MODEL((char*)STRING(pev->model));
		SET_MODEL(ENT(pev), STRING(pev->model));
		UTIL_SetSize(pev, mins, maxs);	// Reset them
	}

	return status;
}

void CBaseEntity::SetObjectCollisionBox(void)
{
	::SetObjectCollisionBox(pev);
}

int	CBaseEntity::Intersects(CBaseEntity* pOther)
{
	if (pOther->pev->absmin.x > pev->absmax.x ||
		pOther->pev->absmin.y > pev->absmax.y ||
		pOther->pev->absmin.z > pev->absmax.z ||
		pOther->pev->absmax.x < pev->absmin.x ||
		pOther->pev->absmax.y < pev->absmin.y ||
		pOther->pev->absmax.z < pev->absmin.z)
		return 0;
	return 1;
}

void CBaseEntity::MakeDormant(void)
{
	SetBits(pev->flags, FL_DORMANT);

	// Don't touch
	pev->solid = SOLID_NOT;
	// Don't move
	pev->movetype = MOVETYPE_NONE;
	// Don't draw
	SetBits(pev->effects, EF_NODRAW);
	// Don't think
	pev->nextthink = 0;
	// Relink
	UTIL_SetOrigin(pev, pev->origin);
}

int CBaseEntity::IsDormant(void)
{
	return FBitSet(pev->flags, FL_DORMANT);
}

BOOL CBaseEntity::IsInWorld(void)
{
	// position 
	if (pev->origin.x > MAX_ENT_COORD) return FALSE;
	if (pev->origin.y > MAX_ENT_COORD) return FALSE;
	if (pev->origin.z > MAX_ENT_COORD) return FALSE;
	if (pev->origin.x < -MAX_ENT_COORD) return FALSE;
	if (pev->origin.y < -MAX_ENT_COORD) return FALSE;
	if (pev->origin.z < -MAX_ENT_COORD) return FALSE;
	// speed
	if (pev->velocity.x > 4096) return FALSE;
	if (pev->velocity.y > 4096) return FALSE;
	if (pev->velocity.z > 4096) return FALSE;
	if (pev->velocity.x < -4096) return FALSE;
	if (pev->velocity.y < -4096) return FALSE;
	if (pev->velocity.z < -4096) return FALSE;

	return TRUE;
}

int CBaseEntity::ShouldToggle(USE_TYPE useType, BOOL currentState)
{
	if (useType != USE_TOGGLE && useType != USE_SET)
	{
		if ((currentState && useType == USE_ON) || (!currentState && useType == USE_OFF))
			return 0;
	}
	return 1;
}

int	CBaseEntity::DamageDecal(int bitsDamageType)
{
	if (m_breakFlags & FL_BREAK_IS_BREAKABLE) {
		if (m_breakMaterial == matGlass)
			return DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2);

		if (m_breakMaterial == matUnbreakableGlass)
			return DECAL_BPROOF1;
	}

	if (pev->rendermode == kRenderTransAlpha)
		return -1;

	if (pev->rendermode != kRenderNormal)
		return DECAL_BPROOF1;

	return DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
}

// NOTE: szName must be a pointer to constant memory, e.g. "monster_class" because the entity
// will keep a pointer to it after this call.
CBaseEntity* CBaseEntity::Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles,
	bool spawn, edict_t* pentOwner, const StringMap& keys)
{
	edict_t* pent;
	CBaseEntity* pEntity;

	pent = CREATE_NAMED_ENTITY(ALLOC_STRING(szName)); // not using MAKE_STRING in case an unloaded plugin called this func
	if (FNullEnt(pent))
	{
		ALERT(at_console, UTIL_VarArgs("NULL Ent '%s' in Create!\n", szName));
		return NULL;
	}
	pEntity = Instance(pent);
	pEntity->pev->owner = pentOwner;
	pEntity->pev->origin = vecOrigin;
	pEntity->pev->angles = vecAngles;

	StringMap::iterator_t iter;
	while (keys.iterate(iter)) {
		KeyValueData dat;
		dat.fHandled = false;
		dat.szClassName = (char*)STRING(pEntity->pev->classname);
		dat.szKeyName = (char*)iter.key;
		dat.szValue = (char*)iter.value;
		DispatchKeyValue(pent, &dat);
	}

	if (spawn) {
		DispatchSpawnGame(pent);
		pEntity = CBaseEntity::Instance(pent); // may have been relocatd by spawn code
	}

	return pEntity;
}

//
// fade out - slowly fades a entity out, then removes it.
//
// DON'T USE ME FOR GIBS AND STUFF IN MULTIPLAYER! 
// SET A FUTURE THINK AND A RENDERMODE!!
void CBaseEntity::SUB_StartFadeOut(void)
{
	if (pev->rendermode == kRenderNormal)
	{
		pev->renderamt = 255;
		pev->rendermode = kRenderTransTexture;
	}

	m_isFadingOut = true;
	pev->solid = SOLID_NOT;
	pev->avelocity = g_vecZero;

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink(&CBaseEntity::SUB_FadeOut);
}

void CBaseEntity::SUB_FadeOut(void)
{
	if (pev->renderamt > 7)
	{
		pev->renderamt -= 7;
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		pev->renderamt = 0;
		pev->nextthink = gpGlobals->time + 0.2;
		SetThink(&CBaseEntity::SUB_Remove);
	}
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target
//=========================================================
BOOL CBaseEntity::FVisible(CBaseEntity* pEntity, bool fIgnoreGlass)
{
	TraceResult tr;
	Vector		vecLookerOrigin;
	Vector		vecTargetOrigin;

	if (FBitSet(pEntity->pev->flags, FL_NOTARGET))
		return FALSE;

	// don't look through water
	if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3)
		|| (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
		return FALSE;

	vecLookerOrigin = pev->origin + pev->view_ofs;//look through the caller's 'eyes'
	vecTargetOrigin = pEntity->IsBSPModel() ? pEntity->Center() : pEntity->EyePosition();

	IGNORE_GLASS ignore = fIgnoreGlass ? ignore_glass : dont_ignore_glass;
	UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore, ENT(pev)/*pentIgnore*/, &tr);

	if (tr.flFraction != 1.0 && tr.pHit != pEntity->edict())
	{
		return FALSE;// Line of sight is not established
	}
	else
	{
		return TRUE;// line of sight is valid.
	}
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target vector
//=========================================================
BOOL CBaseEntity::FVisible(const Vector& vecOrigin, bool fIgnoreGlass)
{
	TraceResult tr;
	Vector		vecLookerOrigin;

	vecLookerOrigin = EyePosition();//look through the caller's 'eyes'

	IGNORE_GLASS ignore = fIgnoreGlass ? ignore_glass : dont_ignore_glass;
	UTIL_TraceLine(vecLookerOrigin, vecOrigin, ignore_monsters, ignore, ENT(pev)/*pentIgnore*/, &tr);

	if (tr.flFraction != 1.0)
	{
		return FALSE;// Line of sight is not established
	}
	else
	{
		return TRUE;// line of sight is valid.
	}
}

/*
================
TraceAttack
================
*/
void CBaseEntity::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if (flDamage > 0 && ShouldBlockFriendlyFire(pevAttacker)) {
		return;
	}

	// random spark if this is a 'computer' object
	if ((m_breakFlags & FL_BREAK_IS_BREAKABLE) && RANDOM_LONG(0, 1))
	{
		switch (m_breakMaterial)
		{
		case matComputer:
		{
			UTIL_Sparks(ptr->vecEndPos);

			float flVolume = RANDOM_FLOAT(0.7, 1.0);//random volume range
			switch (RANDOM_LONG(0, 1))
			{
			case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark5.wav", flVolume, ATTN_NORM);	break;
			case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark6.wav", flVolume, ATTN_NORM);	break;
			}
		}
		break;

		case matUnbreakableGlass:
			UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(0.5, 1.5));
			break;
		default:
			break;
		}
	}

	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	if (pev->takedamage)
	{
		AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);

		int blood = BloodColor();

		if ((bitsDamageType & DMG_BLOOD) && blood != DONT_BLEED)
		{
			SpawnBlood(vecOrigin, blood, flDamage);// a little surface blood.
			TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
		}
	}
}

/*
//=========================================================
// TraceAttack
//=========================================================
void CBaseMonster::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	ALERT ( at_console, "%d\n", ptr->iHitgroup );


	if ( pev->takedamage )
	{
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );

		int blood = BloodColor();

		if ( blood != DONT_BLEED )
		{
			SpawnBlood(vecOrigin, blood, flDamage);// a little surface blood.
		}
	}
}
*/

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Monsters.
================
*/
void CBaseEntity::FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t* pevAttacker)
{
	static int tracerCount;
	int tracer;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;

	if (pevAttacker == NULL)
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for (ULONG iShot = 1; iShot <= cShots; iShot++)
	{
		// get circular gaussian spread
		float x, y;
		GetCircularGaussianSpread(x, y);

		Vector vecDir = vecDirShooting +
			x * vecSpread.x * vecRight +
			y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);

		tracer = 0;
		if (iTracerFreq != 0 && (tracerCount++ % iTracerFreq) == 0)
		{
			Vector vecTracerSrc;

			if (IsPlayer())
			{// adjust tracer position for player
				vecTracerSrc = vecSrc + Vector(0, 0, -4) + gpGlobals->v_right * 2 + gpGlobals->v_forward * 16;
			}
			else
			{
				vecTracerSrc = vecSrc;
			}

			if (iTracerFreq != 1)		// guns that always trace also always decal
				tracer = 1;
			switch (iBulletType)
			{
			case BULLET_MONSTER_MP5:
			case BULLET_MONSTER_9MM:
			case BULLET_MONSTER_12MM:
			case BULLET_PLAYER_556:
			default:
				UTIL_Tracer(vecTracerSrc, tr.vecEndPos);
				break;
			}
		}
		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (iDamage)
			{
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB));

				DecalGunshot(&tr, iBulletType, true, vecSrc, vecEnd);
			}
			else switch (iBulletType)
			{
			default:
			case BULLET_MONSTER_9MM:
				pEntity->TraceAttack(pevAttacker, gSkillData.sk_9mm_bullet, vecDir, &tr, DMG_BULLET);

				DecalGunshot(&tr, iBulletType, true, vecSrc, vecEnd);

				break;

			case BULLET_MONSTER_MP5:
				pEntity->TraceAttack(pevAttacker, gSkillData.sk_9mmAR_bullet, vecDir, &tr, DMG_BULLET);

				DecalGunshot(&tr, iBulletType, true, vecSrc, vecEnd);

				break;

			case BULLET_PLAYER_556:
				pEntity->TraceAttack(pevAttacker, gSkillData.sk_556_bullet, vecDir, &tr, DMG_BULLET);

				DecalGunshot(&tr, iBulletType, true, vecSrc, vecEnd);

				break;

			case BULLET_MONSTER_762:
				pEntity->TraceAttack(pevAttacker, gSkillData.sk_762_bullet, vecDir, &tr, DMG_BULLET);

				DecalGunshot(&tr, iBulletType, true, vecSrc, vecEnd);

				break;

			case BULLET_MONSTER_12MM:
				pEntity->TraceAttack(pevAttacker, gSkillData.sk_12mm_bullet, vecDir, &tr, DMG_BULLET);
				if (!tracer)
				{
					DecalGunshot(&tr, iBulletType, true, vecSrc, vecEnd);
				}
				break;

			case BULLET_NONE: // FIX 
				pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if (!FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					UTIL_DecalTrace(&tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2));
				}

				break;
			}
		}
		// make bullet trails
		UTIL_BubbleTrail(vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0);
	}
	ApplyMultiDamage(pev, pevAttacker);
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Players, uses the random seed generator to sync client and server side shots.
================
*/

/*
EHANDLE g_debugMonster;
EHANDLE g_debugCycler;
#include "basemonster.h"
*/

Vector CBaseEntity::FireBulletsPlayer(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t* pevAttacker, int shared_rand)
{
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x = 0, y = 0;

	if (pevAttacker == NULL)
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	iDamage = GetDamage(iDamage);

	for (ULONG iShot = 1; iShot <= cShots; iShot++)
	{
		//Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat(shared_rand + iShot, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + (1 + iShot), -0.5, 0.5);
		y = UTIL_SharedRandomFloat(shared_rand + (2 + iShot), -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + (3 + iShot), -0.5, 0.5);

		Vector vecDir = vecDirShooting +
			x * vecSpread.x * vecRight +
			y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);

		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
			
			// lag compensation debug code (shows a trace and monster state for misses)
			/*
			if (pEntity->IsMonster()) {
				hudtextparms_t params;
				params.r1 = 255;
				params.g1 = 255;
				params.b1 = 255;
				params.a1 = 255;
				params.channel = 1;
				params.fadeinTime = 0;
				params.fadeoutTime = 0.1f;
				params.x = -1;
				params.y = -1;
				params.effect = 0;

				UTIL_HudMessage(this, params, "X");
				g_debugMonster = pEntity;
			}
			else {
				hudtextparms_t params;
				params.r1 = 255;
				params.g1 = 255;
				params.b1 = 255;
				params.a1 = 255;
				params.channel = 2;
				params.fadeinTime = 0;
				params.fadeoutTime = 0.5f;
				params.x = -1;
				params.y = 0.55;
				params.effect = 0;

				UTIL_HudMessage(this, params, "MISS");
				
				if (g_debugMonster) {
					UTIL_Remove(g_debugCycler);
					CBaseEntity* debugMon = g_debugMonster;
					StringMap keys;
					keys["model"] = STRING(debugMon->pev->model);
					CBaseMonster* cycler = (CBaseMonster*)Create("cycler", debugMon->pev->origin, debugMon->pev->angles, 0, keys);
					cycler->pev->solid = SOLID_NOT;
					cycler->pev->sequence = debugMon->pev->sequence;
					cycler->pev->frame = debugMon->pev->frame;
					cycler->ResetSequenceInfo();
					cycler->pev->framerate = 0.0000001f;
					
					
					g_debugCycler = cycler;
					te_debug_beam(vecSrc, tr.vecEndPos, 1, RGBA(255, 0, 0), MSG_BROADCAST, NULL);
				}
			}
			*/

			if (iDamage)
			{
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB));

				DecalGunshot(&tr, iBulletType, true, vecSrc, vecEnd);
			}
			else switch (iBulletType)
			{
			default:
			case BULLET_PLAYER_9MM:
				pEntity->TraceAttack(pevAttacker, GetDamage(gSkillData.sk_plr_9mm_bullet), vecDir, &tr, DMG_BULLET);
				break;

			case BULLET_PLAYER_MP5:
				pEntity->TraceAttack(pevAttacker, GetDamage(gSkillData.sk_plr_9mmAR_bullet), vecDir, &tr, DMG_BULLET);
				break;

			case BULLET_PLAYER_BUCKSHOT:
				// make distance based!
				pEntity->TraceAttack(pevAttacker, GetDamage(gSkillData.sk_plr_buckshot), vecDir, &tr, DMG_BULLET);
				break;

			case BULLET_PLAYER_357:
				pEntity->TraceAttack(pevAttacker, GetDamage(gSkillData.sk_plr_357_bullet), vecDir, &tr, DMG_BULLET);
				break;

			case BULLET_NONE: // FIX 
				pEntity->TraceAttack(pevAttacker, GetDamage(50), vecDir, &tr, DMG_CLUB);
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if (!FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					UTIL_DecalTrace(&tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2));
				}

				break;
			}
		}
		// make bullet trails
		UTIL_BubbleTrail(vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0);
	}
	ApplyMultiDamage(pev, pevAttacker);

	return Vector(x * vecSpread.x, y * vecSpread.y, 0.0);
}

void CBaseEntity::TraceBleed(float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if (BloodColor() == DONT_BLEED)
		return;

	if (flDamage == 0)
		return;

	if (!(bitsDamageType & (DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_MORTAR)))
		return;

	// make blood decal on the wall! 
	TraceResult Bloodtr;
	Vector vecTraceDir;
	float flNoise;
	int cCount;
	int i;

	/*
		if ( !IsAlive() )
		{
			// dealing with a dead monster.
			if ( pev->max_health <= 0 )
			{
				// no blood decal for a monster that has already decalled its limit.
				return;
			}
			else
			{
				pev->max_health--;
			}
		}
	*/

	if (flDamage < 10)
	{
		flNoise = 0.1;
		cCount = 1;
	}
	else if (flDamage < 25)
	{
		flNoise = 0.2;
		cCount = 2;
	}
	else
	{
		flNoise = 0.3;
		cCount = 4;
	}

	for (i = 0; i < cCount; i++)
	{
		vecTraceDir = vecDir * -1;// trace in the opposite direction the shot came from (the direction the shot is going)

		vecTraceDir.x += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.y += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.z += RANDOM_FLOAT(-flNoise, flNoise);

		UTIL_TraceLine(ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * -172, ignore_monsters, ENT(pev), &Bloodtr);

		if (Bloodtr.flFraction != 1.0)
		{
			UTIL_BloodDecalTrace(&Bloodtr, BloodColor());
		}
	}
}


// This updates global tables that need to know about entities being removed
void CBaseEntity::UpdateOnRemove(void)
{
	int	i;

	if (FBitSet(pev->flags, FL_GRAPHED))
	{
		// this entity was a LinkEnt in the world node graph, so we must remove it from
		// the graph since we are removing it from the world.
		for (i = 0; i < WorldGraph.m_cLinks; i++)
		{
			if (WorldGraph.m_pLinkPool[i].m_pLinkEnt == pev)
			{
				// if this link has a link ent which is the same ent that is removing itself, remove it!
				WorldGraph.m_pLinkPool[i].m_pLinkEnt = NULL;
			}
		}
	}
	if (pev->globalname)
		gGlobalState.EntitySetState(pev->globalname, GLOBAL_DEAD);
}

// Convenient way to delay removing oneself
void CBaseEntity::SUB_Remove(void)
{
	UpdateOnRemove();
	if (pev->health > 0)
	{
		// this situation can screw up monsters who can't tell their entity pointers are invalid.
		pev->health = 0;
		ALERT(at_aiconsole, "SUB_Remove called on entity with health > 0\n");
	}

	REMOVE_ENTITY(ENT(pev));
}

// Convenient way to explicitly do nothing (passed to functions that require a method)
void CBaseEntity::SUB_DoNothing(void)
{
}

/*
==============================
SUB_UseTargets

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Removes all entities with a targetname that match self.killtarget,
and removes them, so some events can remove other triggers.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function (if they have one)

==============================
*/
void CBaseEntity::SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value)
{
	//
	// fire targets
	//
	if (!FStringNull(pev->target))
	{
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
	}
}

void CBaseEntity::SUB_KillTarget(const char* target)
{
	CBaseEntity* pentKillTarget = NULL;

	ALERT(at_aiconsole, "KillTarget: %s\n", target);
	pentKillTarget = UTIL_FindEntityByTargetname(NULL, target);
	while (pentKillTarget)
	{
		UTIL_Remove(pentKillTarget);

		ALERT(at_aiconsole, "killing %s\n", STRING(pentKillTarget->pev->classname));
		pentKillTarget = UTIL_FindEntityByTargetname(pentKillTarget, target);
	}
}

//=========================================================
// SetClassify - sets/changes the monster's classify and
// clears its current schedule to make it pick a new target
// according to its new class.
//=========================================================
void CBaseEntity::SetClassify(int iNewClassify)
{
	m_Classify = iNewClassify;
}

//=========================================================
// IRelationship - returns an integer that describes the 
// relationship between two types of monster.
//=========================================================
int CBaseEntity::IRelationship(CBaseEntity* pTarget)
{
	return IRelationship(Classify(), pTarget->Classify());
}

int CBaseEntity::IRelationship(int attackerClass, int victimClass) {
	const int classCnt = 16;

	//TODO: need to update the entries for military ally & race x
	static int iEnemy[classCnt][classCnt] =
	{				//   NONE	 MACH	 PLYR	 HPASS	 HMIL	 AMIL	 APASS	 AMONST	APREY	 APRED	 INSECT	PLRALY	PBWPN	ABWPN	RACEX	RACEXP
		/*NONE*/		{ R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO,	R_NO,	R_NO,	R_NO,	R_NO	},
		/*MACHINE*/		{ R_NO	,R_NO	,R_DL	,R_DL	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_DL,	R_DL,	R_DL,	R_DL,	R_DL	},
		/*PLAYER*/		{ R_NO	,R_DL	,R_AL	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_AL,	R_DL,	R_DL,	R_DL,	R_DL	},
		/*HUMANPASSIVE*/{ R_NO	,R_NO	,R_AL	,R_AL	,R_HT	,R_FR	,R_NO	,R_HT	,R_DL	,R_FR	,R_NO	,R_AL,	R_NO,	R_NO,	R_FR,	R_FR	},
		/*HUMANMILITAR*/{ R_NO	,R_NO	,R_HT	,R_DL	,R_AL	,R_HT	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_HT,	R_NO,	R_NO,	R_HT,	R_HT	},
		/*ALIENMILITAR*/{ R_NO	,R_DL	,R_HT	,R_DL	,R_HT	,R_AL	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_DL,	R_NO,	R_NO,	R_HT,	R_HT	},
		/*ALIENPASSIVE*/{ R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO,	R_NO,	R_NO,	R_NO,	R_NO	},
		/*ALIENMONSTER*/{ R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_DL,	R_NO,	R_NO,	R_NO,	R_NO	},
		/*ALIENPREY   */{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_FR	,R_NO	,R_DL,	R_NO,	R_NO,	R_NO,	R_NO	},
		/*ALIENPREDATO*/{ R_NO	,R_NO	,R_HT	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_HT	,R_DL	,R_NO	,R_HT,	R_NO,	R_NO,	R_DL,	R_DL	},
		/*INSECT*/		{ R_FR	,R_FR	,R_FR	,R_FR	,R_FR	,R_NO	,R_FR	,R_FR	,R_FR	,R_FR	,R_NO	,R_FR,	R_NO,	R_NO,	R_NO,	R_NO	},
		/*PLAYERALLY*/	{ R_NO	,R_DL	,R_AL	,R_AL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_NO,	R_NO,	R_DL,	R_DL	},
		/*PBIOWEAPON*/	{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_DL,	R_NO,	R_DL,	R_DL,	R_DL	},
		/*ABIOWEAPON*/	{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_AL	,R_NO	,R_DL	,R_DL	,R_NO	,R_NO	,R_DL,	R_DL,	R_NO,	R_DL,	R_DL	},
		/*RACEX*/		{ R_NO	,R_DL	,R_HT	,R_DL	,R_HT	,R_HT	,R_DL	,R_NO	,R_NO	,R_DL	,R_NO	,R_DL,	R_NO,	R_NO,	R_AL,	R_AL	},
		/*RACEX (pit)*/	{ R_NO	,R_DL	,R_HT	,R_DL	,R_HT	,R_HT	,R_DL	,R_NO	,R_NO	,R_DL	,R_NO	,R_DL,	R_NO,	R_NO,	R_AL,	R_AL	}
	};

	if (attackerClass < 0 || attackerClass >= classCnt || victimClass < 0 || victimClass >= classCnt) {
		return R_NO;
	}

	return iEnemy[attackerClass][victimClass];
}

bool CBaseEntity::ShouldBlockFriendlyFire(entvars_t* attacker) {
	// mp_npckill has priority over killnpc if not set to the default value
	int npcKillValue = (int)mp_npckill.value != 1 ? (int)mp_npckill.value : killnpc.value;

	if (!npcKillValue) {
		// disallow ally NPCs to be damaged by anything
		return IRelationship(Classify(), CLASS_PLAYER) == R_AL;
	}
	else if (npcKillValue == 2 && !FNullEnt(attacker)) {
		// disallow ally NPCs to be damaged by ally classes
		CBaseEntity* ent = CBaseEntity::Instance(attacker);
		return ent && IRelationship(Classify(), CLASS_PLAYER) == R_AL && IRelationship(ent) == R_AL;
	}

	return false;
}

bool CBaseEntity::CanReach(CBaseEntity* toucher) {
	TraceResult tr;
	TRACE_LINE(toucher->pev->origin + toucher->pev->view_ofs, pev->origin, dont_ignore_monsters, toucher->edict(), &tr);

	bool hitItemSurface = tr.pHit && tr.pHit == edict();
	bool enteredItemBox = boxesIntersect(pev->absmin, pev->absmax, tr.vecEndPos, tr.vecEndPos);
	
	return hitItemSurface || enteredItemBox;
}

bool CBaseEntity::TestInventoryRules(CBaseMonster* mon, std::unordered_set<CItemInventory*>& usedItems, const char** errorMsg) {
	InventoryRules& rules = m_inventoryRules;

	if (rules.item_name_required) {
		std::vector<std::string> names = splitString(STRING(rules.item_name_required), " ");
		for (const std::string& name : names) {
			CItemInventory* item = mon->GetInventoryItem(name.c_str());
			if (!item) {
				*errorMsg = "You lack a required item.";
				return false;
			}
			usedItems.insert(item);
		}
	}

	if (rules.item_group_required) {
		std::vector<CItemInventory*> items = mon->GetInventoryGroupItems(STRING(rules.item_group_required));

		int requireCount = rules.item_group_required_num;
		if (requireCount == 0) {
			requireCount = CountAllItemsInGroups(STRING(rules.item_group_required));
		}

		if ((int)items.size() < requireCount) {
			if (requireCount == 1)
				*errorMsg = "You lack a required item.";
			else
				*errorMsg = "You lack required items.";
			return false;
		}
	}

	if (rules.item_name_canthave) {
		std::vector<std::string> names = splitString(STRING(rules.item_name_canthave), " ");
		for (const std::string& name : names) {
			if (mon->GetInventoryItem(name.c_str())) {
				*errorMsg = "You are holding a forbidden item.";
				return false;
			}
		}
	}

	if (rules.item_group_canthave) {
		std::vector<CItemInventory*> items = mon->GetInventoryGroupItems(STRING(rules.item_group_canthave));

		int forbidCount = rules.item_group_canthave_num;
		if (forbidCount == 0) {
			forbidCount = CountAllItemsInGroups(STRING(rules.item_group_canthave));
		}

		if ((int)items.size() >= forbidCount) {
			*errorMsg = "You are holding forbidden items.";
			return false;
		}
	}

	return true;
}

bool CBaseEntity::HasInventoryRules() {
	InventoryRules& rules = m_inventoryRules;

	return rules.item_name_required || rules.item_group_required || rules.item_name_canthave
		|| rules.item_group_canthave;
}

bool CBaseEntity::RunInventoryRules(CBaseEntity* ent) {
	CBaseMonster* mon = ent ? ent->MyMonsterPointer() : NULL;

	if (!mon) {
		return true; // non-monsters entities don't have inventories to test
	}

	InventoryRules& rules = m_inventoryRules;
	std::unordered_set<CItemInventory*> usedItems;
	const char* errorMsg = "";

	if (!TestInventoryRules(mon, usedItems, &errorMsg)) {
		if (rules.target_on_fail) {
			FireTargets(STRING(rules.target_on_fail), ent, this, USE_TOGGLE, 0.0f);
		}

		if (mon->IsPlayer()) {
			CBasePlayer* plr = (CBasePlayer*)mon;
			plr->ShowInteractMessage(UTIL_VarArgs("%s: %s", DisplayName(), errorMsg));
		}

		return false;
	}

	if (!rules.pass_ignore_use_triggers) {
		for (CItemInventory* item : usedItems) {
			item->FireInvTargets(ent, item->m_target_on_use);
		}
	}

	if (rules.pass_drop_item_name) {
		std::vector<std::string> names = splitString(STRING(rules.pass_drop_item_name), " ");
		for (const std::string& name : names) {
			CItemInventory* item = mon->GetInventoryItem(name.c_str());
			if (item) {
				item->Detach(!rules.pass_ignore_drop_triggers);
			}
		}
	}

	if (rules.pass_drop_item_group) {
		std::vector<CItemInventory*> items = mon->GetInventoryGroupItems(STRING(rules.pass_drop_item_group));
		for (CItemInventory* item : items) {
			item->Detach(!rules.pass_ignore_drop_triggers);
		}
	}

	if (rules.pass_return_item_name) {
		std::vector<std::string> names = splitString(STRING(rules.pass_return_item_name), " ");
		for (const std::string& name : names) {
			CItemInventory* item = mon->GetInventoryItem(name.c_str());
			if (item) {
				item->Detach(!rules.pass_ignore_return_triggers);
				item->ReturnToSpawnPosition();
			}
		}
	}

	if (rules.pass_return_item_group) {
		std::vector<CItemInventory*> items = mon->GetInventoryGroupItems(STRING(rules.pass_return_item_group));
		for (CItemInventory* item : items) {
			item->Detach(!rules.pass_ignore_return_triggers);
			item->ReturnToSpawnPosition();
		}
	}

	if (rules.pass_destroy_item_name) {
		std::vector<std::string> names = splitString(STRING(rules.pass_destroy_item_name), " ");
		for (const std::string& name : names) {
			CItemInventory* item = mon->GetInventoryItem(name.c_str());
			if (item) {
				item->Detach(false);
				if (rules.pass_ignore_destroy_triggers)
					memset(&item->m_target_on_destroy, 0, sizeof(InvTriggerTargets));
				item->SetTouch(NULL);
				item->SetUse(NULL);
				UTIL_Remove(item);
			}
		}
	}

	if (rules.pass_destroy_item_group) {
		std::vector<CItemInventory*> items = mon->GetInventoryGroupItems(STRING(rules.pass_destroy_item_group));
		for (CItemInventory* item : items) {
			item->Detach(false);
			if (rules.pass_ignore_destroy_triggers)
				memset(&item->m_target_on_destroy, 0, sizeof(InvTriggerTargets));
			item->SetTouch(NULL);
			item->SetUse(NULL);
			UTIL_Remove(item);
		}
	}

	return true;
}

void CBaseEntity::ParametricInterpolation(float flInterval) {

	// A trace is done so that the client doesn't predict a projectile sliding across a wall
	// (try removing this and firing the crossbow up towards a wall, it will start angling 
	// upward as it approaches the impact point).
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + pev->velocity, ignore_monsters, NULL, &tr);

	float deltaTime = flInterval * (1.0f / flInterval) * tr.flFraction;
	Vector endpos = tr.vecEndPos;

	pev->startpos = pev->origin;
	pev->endpos = endpos;
	pev->starttime = gpGlobals->time;
	pev->impacttime = gpGlobals->time + deltaTime;

#if 0
	// Interpolate past position to current.
	// This should in theory create smoother movement due to clients not predicting
	// the wrong path if the velocity changes, but I can't tell any difference unless
	// the entity has the TE_BEAMFOLLOW effect
	pev->startpos = m_oldOrigin;
	pev->endpos = pev->origin;
	pev->starttime = gpGlobals->time;
	pev->impacttime = gpGlobals->time + flInterval;
#endif
}

Vector CBaseEntity::GetLookDirection() {
	Vector angles = pev->angles + pev->punchangle;
	angles.x *= -1;

	MAKE_VECTORS(angles);

	return gpGlobals->v_forward;
}

void CBaseEntity::GiveScorePoints(entvars_t* pevAttacker, float damageDealt) {
	CBaseEntity* baseEnt = CBaseEntity::Instance(pevAttacker);
	CBaseMonster* attackMon = baseEnt ? baseEnt->MyMonsterPointer() : NULL;

	if (pevAttacker == pev) {
		return;
	}

	// give points proportional to how much damage will be dealt, ignoring overkill damage
	if (attackMon && (pevAttacker->flags & FL_CLIENT) && pev->health > 0) {
		float damageAmt = damageDealt > 0 ? V_min(damageDealt, pev->health) : V_min(damageDealt, pev->max_health - pev->health);
		float multiplier = attackMon->IsPlayer() ? ((CBasePlayer*)attackMon)->m_scoreMultiplier : 1.0f;
		bool isFriendly = attackMon->IRelationship(this) == R_AL;

		if (isFriendly) {
			// always take full penalty for friendly fire
			multiplier = 1.0f;
		}

		pevAttacker->frags += damageAmt * (isFriendly ? -1 : 1) * mp_damage_points.value * multiplier;

		CBaseMonster* selfMon = MyMonsterPointer();
		if (selfMon) {
			selfMon->LogPlayerDamage(pevAttacker, damageAmt);
		}
	}
}

const char** CBaseEntity::MaterialSoundList(Materials precacheMaterial, int& soundCount)
{
	const char** pSoundList = NULL;

	switch (precacheMaterial)
	{
	case matWood:
		pSoundList = pSoundsWood;
		soundCount = ARRAYSIZE(pSoundsWood);
		break;
	case matFlesh:
		pSoundList = pSoundsFlesh;
		soundCount = ARRAYSIZE(pSoundsFlesh);
		break;
	case matComputer:
	case matUnbreakableGlass:
	case matGlass:
		pSoundList = pSoundsGlass;
		soundCount = ARRAYSIZE(pSoundsGlass);
		break;

	case matMetal:
		pSoundList = pSoundsMetal;
		soundCount = ARRAYSIZE(pSoundsMetal);
		break;

	case matCinderBlock:
	case matRocks:
		pSoundList = pSoundsConcrete;
		soundCount = ARRAYSIZE(pSoundsConcrete);
		break;


	case matCeilingTile:
	case matNone:
	default:
		soundCount = 0;
		break;
	}

	return pSoundList;
}

void CBaseEntity::MaterialSoundPrecache(Materials precacheMaterial)
{
	const char** pSoundList;
	int			i, soundCount = 0;

	pSoundList = MaterialSoundList(precacheMaterial, soundCount);

	for (i = 0; i < soundCount && i < soundvariety.value; i++) {
		PRECACHE_SOUND_ENT(NULL, (char*)pSoundList[i]);
	}
}

void CBaseEntity::MaterialSoundRandom(edict_t* pEdict, Materials soundMaterial, float volume)
{
	const char** pSoundList;
	int			soundCount = 0;

	pSoundList = MaterialSoundList(soundMaterial, soundCount);

	if (pSoundList) {
		int idx = RANDOM_LONG(0, V_min(soundCount, soundvariety.value) - 1);
		EMIT_SOUND(pEdict, CHAN_BODY, pSoundList[idx], volume, 1.0);
	}
}

void CBaseEntity::Spawn(void) {
	CBaseEntity::Precache();

	pev->max_health = pev->health;
}

void CBaseEntity::Precache(void) {
	if (m_soundReplacementKey && strlen(STRING(m_soundReplacementKey))) {
		const char* filePath = UTIL_VarArgs("sound/%s/%s",
			STRING(gpGlobals->mapname), STRING(m_soundReplacementKey));

		m_soundReplacementPath = ALLOC_STRING(loadReplacementFile(filePath).c_str());

		StringMap& soundReplacements =
			g_replacementFiles[STRING(m_soundReplacementPath)];

		StringMap::iterator_t iter;
		while (soundReplacements.iterate(iter)) {

			// sentences aren't precached by monster code, so precache the replacement here
			if (strlen(iter.key) && iter.key[0] == '!') {
				if (strlen(iter.value) && iter.value[0] == '!') {
					ALERT(at_console, "Monster sentence replacment not implemented.\n");
					continue;
				}

				PRECACHE_SOUND(iter.value);
			}
		}
	}

	if (m_breakFlags & FL_BREAK_IS_BREAKABLE) {
		const char* pGibName;

		switch (m_breakMaterial)
		{
		default:
		case matWood:
			pGibName = "models/woodgibs.mdl";

			PRECACHE_SOUND_ARRAY(pBustSoundsWood);
			break;
		case matFlesh:
			pGibName = "models/fleshgibs.mdl";

			PRECACHE_SOUND_ARRAY(pBustSoundsFlesh);
			break;
		case matComputer:
			PRECACHE_SOUND("buttons/spark5.wav");
			PRECACHE_SOUND("buttons/spark6.wav");
			pGibName = "models/computergibs.mdl";

			PRECACHE_SOUND_ARRAY(pBustSoundsMetal);
			break;

		case matUnbreakableGlass:
		case matGlass:
			pGibName = "models/glassgibs.mdl";

			PRECACHE_SOUND_ARRAY(pBustSoundsGlass);
			break;
		case matMetal:
			pGibName = "models/metalplategibs.mdl";

			PRECACHE_SOUND_ARRAY(pBustSoundsMetal);
			break;
		case matCinderBlock:
			pGibName = "models/cindergibs.mdl";

			PRECACHE_SOUND_ARRAY(pBustSoundsConcrete);
			break;
		case matRocks:
			pGibName = "models/rockgibs.mdl";

			PRECACHE_SOUND_ARRAY(pBustSoundsConcrete);
			break;
		case matCeilingTile:
			pGibName = "models/ceilinggibs.mdl";

			PRECACHE_SOUND("debris/bustceiling.wav");
			break;
		}

		MaterialSoundPrecache((Materials)m_breakMaterial);

		if (m_breakMaterial == matComputer) {
			MaterialSoundPrecache(matMetal);
		}

		if (m_breakModel)
			pGibName = STRING(m_breakModel);

		m_breakModel = ALLOC_STRING(pGibName);
		m_breakModelId = PRECACHE_MODEL((char*)pGibName);
	}
}

bool CBaseEntity::BreakableUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	bool playerUsable = ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE);
	bool isPlayerUse = playerUsable && pCaller && pCaller->IsPlayer();
	
	if ((m_breakFlags & FL_BREAK_CAN_TRIGGER) && IsBreakable() && !isPlayerUse)
	{
		//pev->angles.y = m_angle; // TODO: original HL code. what was this for? seems like it would break anything without an origin brush
		
		g_vecAttackDir = Vector(0,0,0);

		BreakableDie(pActivator);

		return true;
	}

	return false;
}

void CBaseEntity::BreakableDie(CBaseEntity* pActivator) {
	if (!(m_breakFlags & FL_BREAK_IS_BREAKABLE)) {
		return;
	}
	
	Vector vecSpot;// shard origin
	Vector vecVelocity;// shard velocity
	char cFlag = 0;
	int pitch;
	float fvol;

	pitch = 95 + RANDOM_LONG(0, 29);

	if (pitch > 97 && pitch < 103)
		pitch = 100;

	// The more negative pev->health, the louder
	// the sound should be.

	fvol = RANDOM_FLOAT(0.85, 1.0) + (fabs(pev->health) / 100.0);

	if (fvol > 1.0)
		fvol = 1.0;

	switch (m_breakMaterial)
	{
	case matGlass:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pBustSoundsGlass), fvol, ATTN_NORM, 0, pitch);
		cFlag = BREAK_GLASS;
		break;

	case matWood:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pBustSoundsWood), fvol, ATTN_NORM, 0, pitch);
		cFlag = BREAK_WOOD;
		break;

	case matComputer:
	case matMetal:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pBustSoundsMetal), fvol, ATTN_NORM, 0, pitch);
		cFlag = BREAK_METAL;
		break;

	case matFlesh:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pBustSoundsFlesh), fvol, ATTN_NORM, 0, pitch);
		cFlag = BREAK_FLESH;
		break;

	case matRocks:
	case matCinderBlock:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pBustSoundsConcrete), fvol, ATTN_NORM, 0, pitch);
		cFlag = BREAK_CONCRETE;
		break;

	case matCeilingTile:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustceiling.wav", fvol, ATTN_NORM, 0, pitch);
		break;

	case matNone:
	default:
		break;
	}

	vecVelocity = (m_breakFlags & FL_BREAK_DIRECTIONAL_GIBS) ? g_vecAttackDir * 150 : g_vecZero;
	vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
	UTIL_BreakModel(vecSpot, pev->size, vecVelocity, 10, m_breakModelId, 0, 25, cFlag);

	float size = pev->size.x;
	if (size < pev->size.y)
		size = pev->size.y;
	if (size < pev->size.z)
		size = pev->size.z;

	// !!! HACK  This should work!
	// Build a box above the entity that looks like an 8 pixel high sheet
	Vector mins = pev->absmin;
	Vector maxs = pev->absmax;
	mins.z = pev->absmax.z;
	maxs.z += 8;

	// BUGBUG -- can only find 256 entities on a breakable -- should be enough
	CBaseEntity* pList[256];
	int count = UTIL_EntitiesInBox(pList, 256, mins, maxs, FL_ONGROUND, false);
	if (count)
	{
		for (int i = 0; i < count; i++)
		{
			ClearBits(pList[i]->pev->flags, FL_ONGROUND);
			pList[i]->pev->groundentity = NULL;
		}
	}

	if (m_breakExplodeMag) {
		ExplosionCreate(Center(), pev->angles, edict(), m_breakExplodeMag, TRUE);
	}

	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = pev->ltime + 0.1f; // give time for damage sound (in case origin is out of bounds)
	pev->solid = SOLID_NOT;
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 0;

	// Don't fire something that could fire myself
	pev->targetname = 0;

	if (m_breakTrigger)
		FireTargets(STRING(m_breakTrigger), pActivator, this, USE_TOGGLE, 0.0f);
}

void CBaseEntity::BreakableDamageSound()
{
	int pitch;
	float fvol;
	int material = m_breakMaterial;

	//	if (RANDOM_LONG(0,1))
	//		return;

	if (RANDOM_LONG(0, 2))
		pitch = PITCH_NORM;
	else
		pitch = 95 + RANDOM_LONG(0, 34);

	fvol = RANDOM_FLOAT(0.75, 1.0);

	if (material == matComputer && RANDOM_LONG(0, 1))
		material = matMetal;

	int soundCount = 0;
	const char** dmgSoundList = MaterialSoundList((Materials)material, soundCount);

	if (dmgSoundList) {
		int idx = RANDOM_LONG(0, V_min(soundCount, soundvariety.value) - 1);
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, dmgSoundList[idx], fvol, ATTN_NORM, 0, pitch);
	}
}

const char* CBaseEntity::DisplayName() {
	if (m_displayName) {
		return STRING(m_displayName);
	}

	if (m_breakFlags & FL_BREAK_IS_BREAKABLE) {
		// May show "Explosives (explosives only)" in the HUD, but that's better than 
		// "Breakable (explosives only)" which is less informative.
		return m_breakExplodeMag ? "Explosives" : "Breakable";
	}

	return STRING(pev->classname);
}

bool CBaseEntity::IsDelaySpawned() {
	return !sv_precache_bspmodels->value && !g_can_set_bsp_models && pev->model && STRING(pev->model)[0] == '*';
}